#!/usr/bin/env python3
"""Teste de partida dos servidores (DBSrv + TMSrv) no wine, com dados sintéticos.

Não usa dados do jogo: monta uma árvore mínima com arquivos gerados aqui (listas vazias,
mapas zerados, um itemlist.csv com três itens) e o `server/common/ItemEffect.h` do
repositório, que o servidor lê em tempo de execução para montar a tabela de efeitos.

O que confere:
  1. O DBSrv sobe e escuta na porta DB_PORT (7514).
  2. O TMSrv carrega os dados, conecta no DBSrv e escuta na GAME_PORT (8281). O TMSrv só
     chega ao `StartListen` depois de conectar no DBSrv, então a porta aberta prova a conexão.
  3. Uma conexão TCP de fora é aceita e fechada nas duas portas.
  4. Os dois continuam vivos por alguns segundos, com os timers (ProcessSecTimer/MinTimer)
     rodando.
  5. Nenhum dos dois abre MessageBox. Toda caixa de erro do boot é uma MessageBox; o texto
     é capturado pelo canal `relay` do wine (só MessageBoxA/W, para não pesar).

Cobre partida, carregamento e laço de timers, não jogabilidade (não há cliente nem conta).

Precisa de wine (32 bits) e Xvfb: as janelas dos servidores pedem um display, e o driver
`null` do wine não serve para isso em todas as versões. Usa um prefixo do wine próprio,
dentro do diretório de trabalho, para poder ligar o canal relay sem mexer no do usuário.

Uso: python3 tools/server_boot_smoke.py <dir com DBSrv.exe e TMSrv.exe> [--keep DIR] [--settle S]
"""

import argparse
import os
import re
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "tests" / "data" / "server_boot"

DB_PORT = 7514      # server/common/Basedef.h
ADMIN_PORT = 8895   # server/common/Basedef.h
GAME_PORT = 8281    # server/common/Basedef.h
SIZEOF_STRUCT_MOB = 816  # Win32; o servidor lê até isso, um arquivo menor também passaria

# Três itens sintéticos no formato de BASE_ReadItemListFile:
# índice, nome, malha.textura, nível.for.int.des.con, único, preço, pos, extra, grau, (efeito, valor)*
ITEMLIST_CSV = (
    "1,EspadaTeste,1.0,1.0.0.0.0,0,100,64,0,0,EF_DAMAGE,10,EF_GRID,0\n"
    "2,ArmaduraTeste,2.0,1.0.0.0.0,0,100,2,0,0,EF_AC,5\n"
    "412,PocaoTeste,3.0,.,0,10,0,0,0,EF_VOLATILE,1\n"
)


def zero_file(path, size):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "wb") as f:
        f.truncate(size)


def summon_names():
    """Nomes que CNPCSummon::Initialize lê de ./BaseSummon, tirados do próprio fonte."""
    src = (ROOT / "server/tmsrv/entities/CNPCGene.cpp").read_text("latin-1")
    body = src[src.index("void CNPCSummon::Initialize()"):]
    body = body[:body.index("\n}\n")]
    names = re.findall(r'strcpy\(\(char\*\)Mob\[\d+\]\.MobName, "([^"]+)"\)', body)
    if not names:
        raise SystemExit("não achei os nomes de CNPCSummon::Initialize em CNPCGene.cpp")
    return names


def local_ips():
    """Endereços IPv4 do host na ordem do getaddrinfo, que é o que DBSrv e TMSrv comparam
    com o serverlist.txt (DBSrv: getaddrinfo(gethostname()); TMSrv: LocalIP.txt)."""
    try:
        infos = socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET)
    except socket.gaierror:
        infos = []
    ips = []
    for info in infos:
        ip = info[4][0]
        if ip not in ips:
            ips.append(ip)
    return ips or ["127.0.0.1"]


def build_tree(work, exe_dir):
    """Monta <work>/Common e <work>/srv/{DBSrv,TMSrv}: os servidores abrem ../../Common."""
    common = work / "Common"
    db = work / "srv" / "DBSrv"
    tm = work / "srv" / "TMSrv"
    for d in (common, db, tm):
        d.mkdir(parents=True)

    ips = local_ips()[:10]  # MAX_SERVERGROUP
    # Um grupo por endereço: índice 0 é o DBSrv, índice 1 o TMSrv.
    (common / "serverlist.txt").write_text("".join(f"{g} 0 {ip}\n{g} 1 {ip}\n" for g, ip in enumerate(ips)))
    shutil.copy(ROOT / "server/common/ItemEffect.h", common / "ItemEffect.h")
    (common / "itemlist.csv").write_text(ITEMLIST_CSV)
    for name in ("SkillData.csv", "InitItem.csv", "Guilds.txt", "GuildInfo",
                 "Settings/SancRate.txt", "Settings/QuestsRate.txt", "Settings/CompRate.txt",
                 "Settings/MobMerc.txt", "Settings/CastleQuest.txt"):
        zero_file(common / name, 0)

    shutil.copy(exe_dir / "DBSrv.exe", db)
    (db / "Config.txt").write_text("Sapphire 8\nLastCapsule 0\n")
    zero_file(db / "AttributeMap.dat", 1024 * 1024)
    for cls in ("TK", "FM", "BM", "HT"):
        zero_file(db / "BaseMob" / cls, SIZEOF_STRUCT_MOB)

    shutil.copy(exe_dir / "TMSrv.exe", tm)
    # O gameconfig.txt é o que o próprio TMSrv grava quando não o encontra (DrawConfig).
    shutil.copy(DATA / "gameconfig.txt", tm / "gameconfig.txt")
    (tm / "LocalIP.txt").write_text(ips[0] + "\n")
    zero_file(tm / "AttributeMap.dat", 1024 * 1024)
    zero_file(tm / "heightmap.dat", 4096 * 4096)  # MAX_GRIDY * MAX_GRIDX
    for name in ("NPCGener.txt", "LevelItem.txt"):
        (tm / name).write_bytes(b"")
    for name in summon_names():
        zero_file(tm / "BaseSummon" / name, SIZEOF_STRUCT_MOB)
    return db, tm, ips[0]


def start_xvfb():
    r, w = os.pipe()
    proc = subprocess.Popen(["Xvfb", "-displayfd", str(w), "-screen", "0", "640x480x24", "-nolisten", "tcp"],
                            pass_fds=(w,), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    os.close(w)
    with os.fdopen(r) as f:
        num = f.readline().strip()
    if not num:
        proc.kill()
        raise SystemExit("Xvfb não informou o display")
    return proc, ":" + num


def unescape(s):
    """Desfaz os escapes \\xNN do relay; os textos do fonte são UTF-8 (alguns, CP949 antigo)."""
    raw = re.sub(r"\\x([0-9a-fA-F]{2})", lambda m: chr(int(m.group(1), 16)), s).encode("latin-1", "replace")
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError:
        return raw.decode("latin-1")


def wine_log_problems(name, log):
    """MessageBox (texto pelo relay; o wine escapa bytes fora do ASCII) e exceções não tratadas.
    Numa exceção o wine abre o depurador e o processo fica pendurado em vez de terminar, então
    "processo vivo" sozinho não prova nada: a linha "wine: Unhandled ..." é que denuncia."""
    if not log.exists():
        return []
    out = []
    for line in log.read_text("latin-1").splitlines():
        if "Call user32.MessageBox" in line:
            strs = [unescape(s) for s in re.findall(r'"((?:[^"\\]|\\.)*)"', line)]
            out.append(f'{name}: MessageBox "{" / ".join(strs) if strs else line.strip()}"')
        elif line.startswith("wine: Unhandled"):
            out.append(f"{name}: {line.strip()}")
    return out


def server_log(directory):
    """Conteúdo do log do servidor (.\\Log\\*.txt), em latin-1."""
    return "".join(f.read_text("latin-1") for f in sorted((directory / "Log").glob("*.txt")))


def port_open(port, host="127.0.0.1"):
    try:
        with socket.create_connection((host, port), timeout=1):
            return True
    except OSError:
        return False


class Server:
    def __init__(self, name, cwd, env):
        self.name = name
        self.log = cwd / "wine.log"
        self.proc = subprocess.Popen(["wine", name + ".exe"], cwd=cwd, env=env,
                                     stdout=subprocess.DEVNULL, stderr=open(self.log, "wb"))

    def problems(self):
        msgs = wine_log_problems(self.name, self.log)
        if self.proc.poll() is not None:
            msgs.append(f"{self.name}: processo terminou (código {self.proc.returncode})")
        return msgs


def wait_port(port, host, servers, timeout):
    end = time.time() + timeout
    while time.time() < end:
        probs = [p for s in servers for p in s.problems()]
        if probs:
            return probs
        if port_open(port, host):
            return []
        time.sleep(0.5)
    return [f"porta {port} não abriu em {timeout}s"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("exe_dir", help="diretório com DBSrv.exe e TMSrv.exe")
    ap.add_argument("--keep", help="diretório de trabalho a manter (padrão: temporário)")
    ap.add_argument("--settle", type=float, default=15, help="segundos com os dois no ar (padrão 15)")
    ap.add_argument("--timeout", type=float, default=90, help="espera máxima por cada porta (padrão 90)")
    args = ap.parse_args()

    exe_dir = Path(args.exe_dir).resolve()
    for exe in ("DBSrv.exe", "TMSrv.exe"):
        if not (exe_dir / exe).exists():
            raise SystemExit(f"falta {exe_dir / exe}")
    for tool in ("wine", "wineserver", "Xvfb"):
        if not shutil.which(tool):
            raise SystemExit(f"falta {tool} no PATH")
    game_host = local_ips()[0]  # o TMSrv escuta só no IP do LocalIP.txt
    busy = [p for p in (DB_PORT, ADMIN_PORT) if port_open(p)] + [GAME_PORT] * port_open(GAME_PORT, game_host)
    if busy:
        raise SystemExit(f"portas já em uso neste host: {busy}")

    if args.keep:
        work = Path(args.keep).resolve()
        if work.exists():
            shutil.rmtree(work)
        work.mkdir(parents=True)
    else:
        work = Path(tempfile.mkdtemp(prefix="server_boot_"))

    db_dir, tm_dir, game_host = build_tree(work, exe_dir)
    for d in (db_dir, tm_dir):
        (d / "Log").mkdir()  # StartLog grava em .\\Log\\, que os servidores não criam

    xvfb, display = start_xvfb()
    env = dict(os.environ, WINEPREFIX=str(work / "wineprefix"), DISPLAY=display, WINEDLLOVERRIDES="mscoree,mshtml=")
    env.pop("WAYLAND_DISPLAY", None)
    servers = []
    problems = []
    try:
        quiet = dict(env, WINEDEBUG="-all")
        subprocess.run(["wineboot", "-i"], env=quiet, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=300)
        subprocess.run(["wine", "reg", "add", r"HKCU\Software\Wine\Debug", "/v", "RelayInclude",
                        "/d", "user32.MessageBoxA;user32.MessageBoxW", "/f"],
                       env=quiet, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True, timeout=120)
        subprocess.run(["wineserver", "-w"], env=quiet, timeout=120)
        relay = dict(env, WINEDEBUG="-all,+relay")

        t0 = time.time()
        servers.append(Server("DBSrv", db_dir, relay))
        problems = wait_port(DB_PORT, "127.0.0.1", servers, args.timeout)
        if not problems:
            print(f"DBSrv escutando na {DB_PORT} ({time.time() - t0:.1f}s)")
            t0 = time.time()
            servers.append(Server("TMSrv", tm_dir, relay))
            problems = wait_port(GAME_PORT, game_host, servers, args.timeout)
        if not problems:
            print(f"TMSrv conectado ao DBSrv e escutando em {game_host}:{GAME_PORT} ({time.time() - t0:.1f}s)")
            for host, port in (("127.0.0.1", DB_PORT), ("127.0.0.1", ADMIN_PORT), (game_host, GAME_PORT)):
                if not port_open(port, host):
                    problems.append(f"conexão de teste em {host}:{port} recusada")
            time.sleep(args.settle)
            problems += [p for s in servers for p in s.problems()]
            # O TMSrv só grava "sys,start server" depois de ler o heightmap e o BASE_InitializeBaseDef;
            # sem o heightmap, o Reboot() só registra o erro e volta, e o boot pareceria bom.
            tm_log, db_log = server_log(tm_dir), server_log(db_dir)
            if "sys,start server" not in tm_log:
                problems.append("TMSrv: o log não tem \"sys,start server\" (Reboot não carregou a base)")
            # O DBSrv fecha conexões de IPs fora do serverlist; o TMSrv registraria a queda.
            if "err wsareaddb" in tm_log:
                problems.append("TMSrv: perdeu a conexão com o DBSrv (\"err wsareaddb\")")
            if f"err wsa_accept request from  {game_host}" in db_log:
                problems.append(f"DBSrv: recusou a conexão do TMSrv ({game_host})")
            if not problems:
                print(f"os dois seguem no ar após {args.settle:.0f}s, sem MessageBox")
    finally:
        subprocess.run(["wineserver", "-k"], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for s in servers:
            try:
                s.proc.wait(timeout=20)
            except subprocess.TimeoutExpired:
                s.proc.kill()
        xvfb.send_signal(signal.SIGTERM)
        xvfb.wait(timeout=20)

    if problems:
        for p in problems:
            print(f"::error::{p}")
        print(f"resultado: FALHA (trabalho em {work})")
        return 1
    if not args.keep:
        shutil.rmtree(work, ignore_errors=True)
    print("resultado: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
