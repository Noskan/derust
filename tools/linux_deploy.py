#!/usr/bin/env python3
"""Monta (ou confere) a árvore de um servidor Linux e gera as unidades do systemd.

A árvore é a que os servidores esperam, com essa caixa (ver docs/LINUX.md):

    <destino>/Common/        dados comuns (itemlist.csv, SkillData.csv, serverlist.txt, ...)
    <destino>/DBSRV/Run/     DBSrv, contas (account/<inicial>), personagens (char/<inicial>), logs
    <destino>/TMSRV/Run/     TMSrv, mapas, NPCGener.txt, gameconfig.txt, logs

O que faz:
  1. Cria os diretórios em que os servidores gravam e que não criam sozinhos: Log, DayLog,
     chat_log, item_log, account/A..Z+etc, char/A..Z+etc, capsule, npc e, em Common, Import*
     e Error.
  2. Copia DBSrv e TMSrv (do build Linux ou do artefato derust-linux-x86_64 do CI) para os
     diretórios Run, a menos que --no-binaries.
  3. Confere a caixa dos arquivos de dados: os nomes vêm das próprias fontes (os literais
     passados a fopen/_open e as constantes de caminho). No Linux "LocalIP.txt" não é
     "localip.txt"; uma árvore copiada do Windows costuma ter esse problema. Com --fix-case,
     renomeia para o nome que o código abre.
  4. Grava em <destino>/systemd as unidades w2-dbsrv.service e w2-tmsrv.service (e a
     w2-w2db.service com --w2db, e a w2-w2gm.service do painel de GM com --w2gm), com os caminhos desta árvore, mais as de operação
     (docs/OPS.md): w2-notify@.service (aviso no Discord quando uma unidade falha),
     w2-backup@.service e os timers do backup. Copia os scripts delas (deploy/bin) para
     <destino>/bin e cria a pasta das métricas. A instalação no sistema fica com quem opera
     (o script imprime os comandos).

Não copia dados do jogo nem cria arquivos de configuração: isso é de quem opera.

Uso: python3 tools/linux_deploy.py <destino> [--bin DIR] [--no-binaries] [--fix-case]
       [--user USUÁRIO] [--check-only] [--w2db BIN [--w2gm BIN]] [--metrics-dir DIR] [--pg-user USUÁRIO]
"""

import argparse
import getpass
import os
import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

INITIALS = [chr(c) for c in range(ord("A"), ord("Z") + 1)] + ["etc"]
WRITE_DIRS = {
    # status: a página de status do DBSrv (serv<NN>.htm), que um servidor web publica.
    "Common": ["ImportUser", "ImportItem", "ImportDonate", "ImportDonateError", "Error", "status"],
    "DBSRV/Run": ["Log", "DayLog", "capsule"] + [f"account/{i}" for i in INITIALS] + [f"char/{i}" for i in INITIALS],
    "TMSRV/Run": ["Log", "chat_log", "item_log", "npc"],
}
SERVER_DIR = {"server/dbsrv": ["DBSRV/Run"], "server/tmsrv": ["TMSRV/Run"], "server/common": ["DBSRV/Run", "TMSRV/Run"]}

# Literais de caminho nas fontes: argumentos de fopen/_open e de BASE_ReadItemListFile, e as
# constantes `... = "../..."` / `... = "./..."` (CReadFiles::*_PATH) e strcpy(x, "./...").
PATH_RX = [
    re.compile(r'\b(?:fopen|_open)\s*\(\s*"([^"%*]+)"'),
    re.compile(r'\bBASE_ReadItemListFile\s*\(\s*"([^"%*]+)"'),
    re.compile(r'=\s*"((?:\.\./|\./)[^"%*]+)"'),
    re.compile(r'\bstrcpy\s*\(\s*\w+\s*,\s*"((?:\.\./|\./)[^"%*]+)"'),
]


def expected_files():
    """{caminho relativo ao destino: [fontes que o abrem]} para os arquivos que o código lê ou grava."""
    out = {}
    for mod, run_dirs in SERVER_DIR.items():
        for src in sorted((ROOT / mod).rglob("*.cpp")):
            text = src.read_text("utf-8", errors="replace")
            for rx in PATH_RX:
                for m in rx.finditer(text):
                    lit = m.group(1)
                    if lit.endswith("/") or " " in lit:
                        continue
                    if lit.startswith("../../"):
                        targets = [lit[len("../../"):]]
                    elif lit.startswith("../"):
                        continue  # cabeçalhos de checksum gravados fora da árvore (SkillCSum.h)
                    else:
                        name = lit[2:] if lit.startswith("./") else lit
                        targets = [f"{d}/{name}" for d in run_dirs]
                    for t in targets:
                        top = t.split("/", 2)
                        if top[0] == "Common" or "/".join(top[:2]) in ("DBSRV/Run", "TMSRV/Run"):
                            out.setdefault(t, set()).add(src.relative_to(ROOT).as_posix())
    return out


def find_case_insensitive(dest, rel):
    """Caminho existente que difere de `rel` só na caixa, ou None."""
    cur = dest
    for part in rel.split("/"):
        exact = cur / part
        if exact.exists():
            cur = exact
            continue
        if not cur.is_dir():
            return None
        match = [p for p in cur.iterdir() if p.name.lower() == part.lower()]
        if not match:
            return None
        cur = match[0]
    return cur


def fix_case(dest, rel):
    """Renomeia, componente a componente, o caminho existente para a caixa de `rel`: um diretório
    com a caixa errada é renomeado inteiro (com tudo o que tem dentro), não só o arquivo."""
    cur = dest
    renamed = []
    for part in rel.split("/"):
        exact = cur / part
        if not exact.exists():
            match = [p for p in cur.iterdir() if p.name.lower() == part.lower()]
            match[0].rename(exact)
            renamed.append(f"{match[0].relative_to(dest).as_posix()} -> {exact.relative_to(dest).as_posix()}")
        cur = exact
    return renamed


# Isolamento de todas as unidades que ficam no ar (docs/OPS.md): usuário sem privilégios, sem
# capability nenhuma, o sistema todo só para leitura (menos ReadWritePaths= e o RuntimeDirectory=),
# /home só para leitura, /tmp e /dev próprios, só sockets IPv4, IPv6 e Unix, e as chamadas de
# sistema de um serviço comum. Uma chamada fora do filtro falha com EPERM em vez de matar o
# processo. tools/server_boot_smoke.py --sandbox roda o jogo com estas mesmas propriedades.
HARDENING = [
    "NoNewPrivileges=yes",
    "CapabilityBoundingSet=",
    "ProtectSystem=strict",
    "ProtectHome=read-only",
    "PrivateTmp=yes",
    "PrivateDevices=yes",
    "ProtectKernelTunables=yes",
    "ProtectKernelModules=yes",
    "ProtectKernelLogs=yes",
    "ProtectControlGroups=yes",
    "ProtectClock=yes",
    "ProtectHostname=yes",
    "ProtectProc=invisible",
    "RestrictAddressFamilies=AF_INET AF_INET6 AF_UNIX",
    "RestrictNamespaces=yes",
    "RestrictRealtime=yes",
    "RestrictSUIDSGID=yes",
    "LockPersonality=yes",
    "RemoveIPC=yes",
    "MemoryDenyWriteExecute=yes",
    "SystemCallArchitectures=native",
    "SystemCallFilter=@system-service",
    "SystemCallFilter=~@privileged @resources",
    "SystemCallErrorNumber=EPERM",
]


def server_rw_paths(exe, dest, metrics):
    """O que DBSrv e TMSrv gravam: o próprio Run (contas, personagens, logs, control.fifo), o
    Common (Import*, Error, a página de status) e a pasta das métricas."""
    run = dest / ("DBSRV/Run" if exe == "DBSrv" else "TMSRV/Run")
    return [run, dest / "Common", Path(metrics)]


def hardening(rw_paths=()):
    lines = list(HARDENING)
    if rw_paths:
        lines.append("ReadWritePaths=" + " ".join(str(p) for p in rw_paths))
    return "\n".join(lines) + "\n"


# Comum às unidades dos processos que ficam no ar.
SERVICE_COMMON = """Restart=always
RestartSec=5
# Queda em laço: o intervalo dobra a cada reinício até 5 min.
RestartSteps=5
RestartMaxDelaySec=300
# Core dump para o systemd-coredump (coredumpctl), que o aviso do Discord resume.
LimitCORE=infinity
StandardOutput=journal
StandardError=journal
"""

UNIT = """# Gerado por tools/linux_deploy.py. Instale com:
#   sudo cp {path} /etc/systemd/system/ && sudo systemctl daemon-reload
#   sudo systemctl enable --now {name}
[Unit]
Description={desc}
After=network-online.target{after}
Wants=network-online.target{wants}
# Cada queda (Restart= passa pelo estado failed) vira um aviso no Discord (docs/OPS.md).
OnFailure=w2-notify@%n.service

[Service]
Type=simple
User={user}
WorkingDirectory={run}
ExecStart={exec}
{env}{hardening}# SIGTERM é o pedido de encerramento, como fechar a janela no Windows: grava (guildas,
# configuração, logs) e sai.
KillSignal=SIGTERM
TimeoutStopSec=60
# Reinicia sempre que o processo sair sem um `systemctl stop`: queda, o comando `reboot` ou `exit`
# do FIFO de controle, e o TMSrv que sai quando não alcança o DBSrv no boot.
{common}# Um descritor por jogador (MAX_USER 1000) mais arquivos e as conexões entre servidores.
LimitNOFILE=8192
# As MessageBox e o eco dos comandos de control.fifo saem no stderr: journalctl -u {name}

[Install]
WantedBy=multi-user.target
"""

W2DB_UNIT = """# Gerado por tools/linux_deploy.py (docs/W2DB.md, docs/OPS.md).
# /etc/derust/w2db.env (modo 0600) tem pelo menos DATABASE_URL; o modelo é o que o script imprime.
[Unit]
Description=w2db (contas do DBSrv no Postgres)
After=network-online.target postgresql.service
Wants=postgresql.service
OnFailure=w2-notify@%n.service

[Service]
Type=simple
User={user}
WorkingDirectory={run}
EnvironmentFile=/etc/derust/w2db.env
Environment=NO_COLOR=1
ExecStart={run}/w2db serve --socket /run/w2db/w2db.sock
# O socket fica em /run/w2db, só para o usuário dos servidores.
RuntimeDirectory=w2db
RuntimeDirectoryMode=0750
# Só fala com o Postgres, o DBSrv e o Discord: não grava nada além do socket.
{hardening}{common}
[Install]
WantedBy=multi-user.target
"""

W2DB_ENV = """# /etc/derust/w2db.env (sudo install -m 0600 -o {user} ...): lido pela w2-w2db.service.
DATABASE_URL=postgres:///w2db?host=/var/run/postgresql
# /metrics para o Prometheus (só local).
W2DB_METRICS_ADDR=127.0.0.1:9730
# Relatos do /bug no Discord: o arquivo tem o endereço do webhook do canal (modo 0600).
#W2DB_DISCORD_WEBHOOK_FILE=/etc/derust/discord-bugs-webhook
# Conta nova só com convite (o teste fechado).
#W2DB_REQUIRE_INVITE=true
"""

W2GM_UNIT = """# Gerado por tools/linux_deploy.py (docs/GM.md): o painel de GM, o cadastro pelo link e o bot.
# Ouve só em 127.0.0.1; o Caddy (deploy/caddy/Caddyfile) põe o HTTPS na frente.
# /etc/derust/w2gm.env (modo 0600) tem DATABASE_URL e W2GM_PUBLIC_URL; o modelo é o que o script
# imprime.
[Unit]
Description=w2gm (painel de GM e bot do Discord)
After=network-online.target postgresql.service w2-w2db.service
Wants=network-online.target w2-w2db.service
OnFailure=w2-notify@%n.service

[Service]
Type=simple
User={user}
WorkingDirectory={run}
EnvironmentFile=/etc/derust/w2gm.env
Environment=NO_COLOR=1
ExecStart={run}/w2gm serve
# É a parte que a internet alcança (pelo Caddy): não grava nada no disco.
{hardening}{common}
[Install]
WantedBy=multi-user.target
"""

W2GM_ENV = """# /etc/derust/w2gm.env (sudo install -m 0600 -o {user} ...): lido pela w2-w2gm.service.
DATABASE_URL=postgres:///w2db?host=/var/run/postgresql
W2GM_LISTEN=127.0.0.1:8730
# O endereço do Caddy (deploy/caddy/Caddyfile): vai nos links do /registrar.
W2GM_PUBLIC_URL=https://gm.exemplo.invalid
# O bot do Discord (docs/GM.md). Sem o token, só o site roda.
#W2GM_DISCORD_TOKEN_FILE=/etc/derust/discord-bot-token
#W2GM_DISCORD_GUILD=<id do servidor>
#W2GM_DISCORD_GM_ROLE=<id do cargo que cria convites>
"""

NOTIFY_UNIT = """# Gerado por tools/linux_deploy.py: o OnFailure= das unidades do derust (docs/OPS.md).
# O endereço do webhook fica em /etc/derust/discord-alerts-webhook (dono {user}, modo 0600).
[Unit]
Description=Aviso no Discord da falha de %i

[Service]
Type=oneshot
User={user}
# Para ler o journal e o coredumpctl da unit que falhou.
SupplementaryGroups=systemd-journal
StateDirectory=w2-notify
Environment=W2_DISCORD_WEBHOOK_FILE=/etc/derust/discord-alerts-webhook
ExecStart={bin}/w2-notify-discord %i
"""

BACKUP_UNIT = """# Gerado por tools/linux_deploy.py: backup do w2db com o pgBackRest (docs/OPS.md), pelos
# timers w2-backup-full.timer e w2-backup-diff.timer.
[Unit]
Description=Backup do w2db (pgBackRest, %i)
After=postgresql.service
OnFailure=w2-notify@%n.service

[Service]
Type=oneshot
User={pg_user}
ExecStart=/usr/bin/pgbackrest --stanza=w2db --type=%i backup
# Só depois de um backup bom (com + roda como root, para gravar na pasta das métricas).
ExecStartPost=+{bin}/w2-backup-metric %i {metrics}
"""

TIMER = """# Gerado por tools/linux_deploy.py (docs/OPS.md).
[Unit]
Description={desc}

[Timer]
OnCalendar={when}
# Se a máquina estava desligada na hora, roda ao ligar.
Persistent=true
RandomizedDelaySec=10min
Unit=w2-backup@{type}.service

[Install]
WantedBy=timers.target
"""


def write_units(dest, user, w2db=False, metrics=None, pg_user="postgres", w2gm=False):
    if metrics is None:
        metrics = dest / "metrics"
        metrics.mkdir(exist_ok=True)
    sysd = dest / "systemd"
    sysd.mkdir(exist_ok=True)
    bindir = dest / "bin"
    env_metrics = f"Environment=W2_METRICS_DIR={metrics}\n"
    dbsrv_env = env_metrics + ("Environment=W2DB_SOCKET=/run/w2db/w2db.sock\n" if w2db else "")
    dbsrv_dep = " w2-w2db.service" if w2db else ""
    units = [
        ("w2-dbsrv.service", "W2PP DBSrv (banco de dados)", dest / "DBSRV/Run", "DBSrv", dbsrv_dep, dbsrv_dep,
         dbsrv_env),
        ("w2-tmsrv.service", "W2PP TMSrv (servidor de jogo)", dest / "TMSRV/Run", "TMSrv",
         " w2-dbsrv.service", " w2-dbsrv.service", env_metrics),
    ]
    files = {}
    for name, desc, run, exe, after, wants, env in units:
        files[name] = UNIT.format(path=sysd / name, name=name, desc=desc, run=run, exec=f"{run}/{exe}", user=user,
                                  after=after, wants=wants, env=env, common=SERVICE_COMMON,
                                  hardening=hardening(server_rw_paths(exe, dest, metrics)))
    if w2db:
        files["w2-w2db.service"] = W2DB_UNIT.format(user=user, run=dest / "W2DB", common=SERVICE_COMMON,
                                                    hardening=hardening())
        files["w2db.env.example"] = W2DB_ENV.format(user=user)
    if w2gm:
        files["w2-w2gm.service"] = W2GM_UNIT.format(user=user, run=dest / "W2GM", common=SERVICE_COMMON,
                                                    hardening=hardening())
        files["w2gm.env.example"] = W2GM_ENV.format(user=user)
    files["w2-notify@.service"] = NOTIFY_UNIT.format(user=user, bin=bindir)
    files["w2-backup@.service"] = BACKUP_UNIT.format(pg_user=pg_user, bin=bindir, metrics=metrics)
    files["w2-backup-full.timer"] = TIMER.format(desc="Backup completo semanal do w2db", when="Sun *-*-* 04:00:00",
                                                 type="full")
    files["w2-backup-diff.timer"] = TIMER.format(desc="Backup diferencial diário do w2db",
                                                 when="Mon..Sat *-*-* 04:00:00", type="diff")
    paths = []
    for name, text in files.items():
        path = sysd / name
        path.write_text(text)
        paths.append(path)
    return paths


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dest", help="raiz da árvore (Common, DBSRV/Run, TMSRV/Run)")
    ap.add_argument("--bin", help="diretório com DBSrv e TMSrv (padrão: build/linux do repositório)")
    ap.add_argument("--no-binaries", action="store_true", help="não copia os binários")
    ap.add_argument("--fix-case", action="store_true", help="renomeia os arquivos de dados para a caixa que o código abre")
    ap.add_argument("--user", default=getpass.getuser(), help="usuário das unidades do systemd (padrão: o atual)")
    ap.add_argument("--check-only", action="store_true", help="só confere a caixa dos arquivos; não cria nem copia nada")
    ap.add_argument("--w2db", metavar="BIN", help="binário do w2db: copia para <destino>/W2DB e gera a w2-w2db.service "
                    "(o DBSrv passa a usar o w2db)")
    ap.add_argument("--w2gm", metavar="BIN", help="binário do w2gm: copia para <destino>/W2GM e gera a w2-w2gm.service "
                    "(painel de GM e bot; precisa do --w2db)")
    ap.add_argument("--metrics-dir", help="pasta dos arquivos .prom para o node_exporter (padrão: <destino>/metrics)")
    ap.add_argument("--pg-user", default="postgres", help="usuário do Postgres, que roda o pgBackRest (padrão: postgres)")
    args = ap.parse_args()

    dest = Path(args.dest).resolve()
    if args.w2gm and not args.w2db:
        sys.exit("--w2gm precisa do --w2db (o painel lê o banco do w2db)")
    if not args.check_only:
        for base, subs in WRITE_DIRS.items():
            for s in [""] + subs:
                (dest / base / s).mkdir(parents=True, exist_ok=True)
        metrics = Path(args.metrics_dir).resolve() if args.metrics_dir else dest / "metrics"
        metrics.mkdir(parents=True, exist_ok=True)
        os.chmod(metrics, 0o755)
        print(f"árvore em {dest}: Common, DBSRV/Run, TMSRV/Run e os diretórios de gravação; métricas em {metrics}")

        # Os scripts das unidades (aviso no Discord, registro do backup).
        (dest / "bin").mkdir(exist_ok=True)
        for script in sorted(p for p in (ROOT / "deploy/bin").iterdir() if p.is_file()):
            shutil.copy2(script, dest / "bin" / script.name)
            os.chmod(dest / "bin" / script.name, 0o755)

        if args.w2db:
            (dest / "W2DB").mkdir(exist_ok=True)
            shutil.copy2(args.w2db, dest / "W2DB/w2db")
            os.chmod(dest / "W2DB/w2db", 0o755)
            print(f"w2db: {args.w2db} -> {dest / 'W2DB/w2db'}")
        if args.w2gm:
            (dest / "W2GM").mkdir(exist_ok=True)
            shutil.copy2(args.w2gm, dest / "W2GM/w2gm")
            os.chmod(dest / "W2GM/w2gm", 0o755)
            print(f"w2gm: {args.w2gm} -> {dest / 'W2GM/w2gm'}")

        if not args.no_binaries:
            bins = Path(args.bin).resolve() if args.bin else ROOT / "build/linux"
            for exe, sub, run in (("DBSrv", "server/dbsrv", "DBSRV/Run"), ("TMSrv", "server/tmsrv", "TMSRV/Run")):
                src = next((p for p in (bins / exe, bins / sub / exe) if p.is_file()), None)
                if not src:
                    sys.exit(f"não achei {exe} em {bins} (use --bin ou --no-binaries)")
                shutil.copy2(src, dest / run / exe)
                os.chmod(dest / run / exe, 0o755)
                print(f"{exe}: {src} -> {dest / run / exe}")

    wrong, missing = [], []
    for rel, srcs in sorted(expected_files().items()):
        if (dest / rel).exists():
            continue
        found = find_case_insensitive(dest, rel)
        if found:
            wrong.append((rel, found))
        else:
            missing.append((rel, sorted(srcs)))

    for rel, found in wrong:
        if args.fix_case and not args.check_only:
            if (dest / rel).exists():
                continue  # já resolvido ao renomear um diretório acima
            for r in fix_case(dest, rel):
                print(f"renomeado: {r}")
        else:
            print(f"caixa diferente: o código abre {rel}, existe {found.relative_to(dest).as_posix()}")
    if missing:
        print(f"{len(missing)} arquivo(s) que o código abre não existem (nem todos são obrigatórios; "
              "os de log e os gerados pelos servidores aparecem aqui antes do primeiro boot):")
        for rel, srcs in missing:
            print(f"  {rel}  ({', '.join(srcs)})")

    if not args.check_only:
        units = write_units(dest, args.user, bool(args.w2db), metrics, args.pg_user, bool(args.w2gm))
        print(f"unidades do systemd em {dest / 'systemd'}:")
        for u in units:
            print(f"  {u.name}")
        names = [u.name for u in units if u.suffix in (".service", ".timer")]
        services = "w2-w2db w2-dbsrv w2-tmsrv" if args.w2db else "w2-dbsrv w2-tmsrv"
        if args.w2gm:
            services += " w2-w2gm"
        print("instale com: sudo cp " + " ".join(str(dest / "systemd" / n) for n in names) + " /etc/systemd/system/ && "
              f"sudo systemctl daemon-reload && sudo systemctl enable --now {services} "
              "w2-backup-full.timer w2-backup-diff.timer")
        print(f"node_exporter: --collector.textfile.directory={metrics} --collector.systemd (docs/OPS.md)")
        if args.w2db:
            print(f"w2db: copie {dest / 'systemd/w2db.env.example'} para /etc/derust/w2db.env (modo 0600) e ajuste")
        if args.w2gm:
            print(f"w2gm: copie {dest / 'systemd/w2gm.env.example'} para /etc/derust/w2gm.env (modo 0600) e ajuste; "
                  f"o HTTPS é o deploy/caddy/Caddyfile")

    pending = wrong if not (args.fix_case and not args.check_only) else []
    if pending:
        print(f"resultado: {len(pending)} arquivo(s) com a caixa diferente (rode com --fix-case para renomear)")
        return 1
    print("resultado: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
