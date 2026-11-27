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
  4. Grava em <destino>/systemd as unidades w2-dbsrv.service e w2-tmsrv.service, com os
     caminhos desta árvore. A instalação no sistema fica com quem opera (o script imprime
     os comandos).

Não copia dados do jogo nem cria arquivos de configuração: isso é de quem opera.

Uso: python3 tools/linux_deploy.py <destino> [--bin DIR] [--no-binaries] [--fix-case]
       [--user USUÁRIO] [--check-only]
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


UNIT = """# Gerado por tools/linux_deploy.py. Instale com:
#   sudo cp {path} /etc/systemd/system/ && sudo systemctl daemon-reload
#   sudo systemctl enable --now {name}
[Unit]
Description={desc}
After=network-online.target{after}
Wants=network-online.target{wants}

[Service]
Type=simple
User={user}
WorkingDirectory={run}
ExecStart={run}/{exe}
# SIGTERM é o pedido de encerramento, como fechar a janela no Windows: grava (guildas,
# configuração, logs) e sai.
KillSignal=SIGTERM
TimeoutStopSec=60
# Reinicia sempre que o processo sair sem um `systemctl stop`: queda, o comando `reboot` ou `exit`
# do FIFO de controle, e o TMSrv que sai quando não alcança o DBSrv no boot.
Restart=always
RestartSec=5
# Um descritor por jogador (MAX_USER 1000) mais arquivos e as conexões entre servidores.
LimitNOFILE=8192
# As MessageBox e o eco dos comandos de control.fifo saem no stderr: journalctl -u {name}
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
"""


def write_units(dest, user):
    sysd = dest / "systemd"
    sysd.mkdir(exist_ok=True)
    units = [
        ("w2-dbsrv.service", "W2PP DBSrv (banco de dados)", dest / "DBSRV/Run", "DBSrv", "", ""),
        ("w2-tmsrv.service", "W2PP TMSrv (servidor de jogo)", dest / "TMSRV/Run", "TMSrv",
         " w2-dbsrv.service", " w2-dbsrv.service"),
    ]
    paths = []
    for name, desc, run, exe, after, wants in units:
        path = sysd / name
        path.write_text(UNIT.format(path=path, name=name, desc=desc, run=run, exe=exe, user=user,
                                    after=after, wants=wants))
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
    args = ap.parse_args()

    dest = Path(args.dest).resolve()
    if not args.check_only:
        for base, subs in WRITE_DIRS.items():
            for s in [""] + subs:
                (dest / base / s).mkdir(parents=True, exist_ok=True)
        print(f"árvore em {dest}: Common, DBSRV/Run, TMSRV/Run e os diretórios de gravação")

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
        units = write_units(dest, args.user)
        print("unidades do systemd:")
        for u in units:
            print(f"  {u}")
        print("instale com: sudo cp " + " ".join(str(u) for u in units) + " /etc/systemd/system/ && "
              "sudo systemctl daemon-reload && sudo systemctl enable --now w2-dbsrv w2-tmsrv")

    pending = wrong if not (args.fix_case and not args.check_only) else []
    if pending:
        print(f"resultado: {len(pending)} arquivo(s) com a caixa diferente (rode com --fix-case para renomear)")
        return 1
    print("resultado: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
