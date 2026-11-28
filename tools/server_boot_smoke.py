#!/usr/bin/env python3
"""Teste de partida dos servidores Linux (DBSrv + TMSrv), com dados sintéticos.

Não usa dados do jogo: monta uma árvore mínima com arquivos gerados aqui (listas vazias,
mapas zerados, um itemlist.csv com alguns itens) e o `server/common/ItemEffect.h` do
repositório, que o servidor lê em tempo de execução para montar a tabela de efeitos.

O que confere:
  1. O DBSrv sobe e escuta na porta DB_PORT (7514).
  2. O TMSrv carrega os dados, conecta no DBSrv e escuta na GAME_PORT (8281). O TMSrv só
     chega ao `StartListen` depois de conectar no DBSrv, então a porta aberta prova a conexão.
  3. Uma conexão TCP de fora é aceita e fechada nas duas portas.
  4. Os dois continuam vivos por alguns segundos, com os timers (ProcessSecTimer/MinTimer)
     rodando.
  5. Nenhum dos dois abre MessageBox nem sofre exceção não tratada. Toda caixa de erro do
     boot é uma MessageBox.

Com o LoginClient.exe (tools/login_client, montado com os fontes do cliente) no mesmo diretório,
confere também o login e uma sessão curta de jogo de ponta a ponta:
  6. Duas contas são criadas pelo caminho do próprio DBSrv (arquivos em Common/ImportUser, que o
     ProcessSecTimer importa e apaga).
  7. O LoginClient entra na primeira conta, define a senha numérica, cria um personagem, cria e
     apaga outro e entra no jogo; depois entra com a segunda conta perto do primeiro. Os valores
     do personagem vêm do BaseMob sintético (synthetic_base_mob), gravado no layout do cliente.
  8. Com os dois no jogo: cada um vê o outro, chat, movimento (inclusive para uma casa ocupada),
     soltar e pegar item, troca de posição com e sem bolsa, uso de poção, e saída e volta do
     personagem com o inventário gravado pelo DBSrv. Cada resposta é conferida no layout do
     cliente (ver o cabeçalho de tools/login_client/LoginClient.cpp).

Não cobre combate, troca entre jogadores, lojas nem NPCs (o mundo sintético não tem mobs).

Antes, um autoteste sobe o DBSrv sem o SkillData.csv e exige que a MessageBox seja vista:
garante que o detector funciona na plataforma em uso (desligue com --no-self-test).

Os servidores são os binários ELF do build CMake no Linux, executados direto. Os comandos do
menu da janela vão pelo FIFO de controle (`control.fifo`), conferindo o eco de cada um. Um
alerta (ServerAlert, a antiga MessageBox) escreve "[título] texto" no stderr, que vai para
<dir>/stderr.log; o encerramento é por SIGTERM, que o laço de eventos entrega como o pedido de
fechar a janela. O LoginClient.exe roda no wine (32 bits, num prefixo próprio, dentro do diretório de trabalho, e
num Xvfb próprio): é um cliente Windows contra servidores Linux.

Com --w2db (o binário de services/w2db), as contas ficam no Postgres em vez de ./account: o teste
sobe um cluster Postgres próprio (initdb no diretório de trabalho, só Unix socket) e o `w2db serve`,
e aponta o DBSrv para ele (w2db.txt). Confere também, no banco:
  9. As contas e os personagens criados pelo caminho do jogo, sem senha em claro, e os itens de
     cada um com serial e evento de criação.
 10. A troca de mão dupla do LoginClient pelo serial: a espada criada em cada conta está na outra.
Com --forced-dupe (TMSrv montado com DERUST_TEST_HOOKS), o LoginClient força uma duplicação numa
segunda troca (/w2dupe: A dá a armadura e fica com uma cópia), e o teste exige que o w2db acuse o
serial repetido (evento dupe_suspect) e que o DBSrv avise o operador.

Operação (docs/OPS.md), conferida em todo teste com o LoginClient:
 11. O /bug do LoginClient (passo 17) chega ao log do TMSrv e do DBSrv; com --w2db, a linha de
     bug_reports e o post no webhook falso do Discord (um servidor HTTP do teste), sem menção.
 12. As métricas que o TMSrv e o DBSrv gravam em W2_METRICS_DIR (w2_tmsrv.prom, w2_dbsrv.prom),
     e, com --w2db, o /metrics do w2db (a suspeita de duplicação contada, com --forced-dupe).
 13. Com --w2db, o `w2db check` no banco do teste no fim.

Uso: python3 tools/server_boot_smoke.py <dir com DBSrv e TMSrv> [--keep DIR] [--settle S]
       [--login-client EXE] [--require-login] [--w2db EXE [--pg-bin DIR] [--forced-dupe]]
"""

import argparse
import http.server
import json
import os
import re
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import linux_deploy  # noqa: E402  (WRITE_DIRS: a árvore da produção)
import w2_bin_csv  # noqa: E402
DATA = ROOT / "tests" / "data" / "server_boot"

DB_PORT = 7514      # server/common/Basedef.h
ADMIN_PORT = 8895   # server/common/Basedef.h
GAME_PORT = 8281    # server/common/Basedef.h
SIZEOF_STRUCT_MOB = 1024  # Win32, layout do cliente; o servidor lê até isso, um arquivo menor também passaria
BASE_CLASSES = ("TK", "FM", "BM", "HT")  # ordem de g_pBaseSet no DBSrv

# Contas e personagens do login sintético: A faz o login completo; B entra perto de A para o
# teste de jogo. As duas usam a mesma senha e a mesma senha numérica.
LOGIN_ACCOUNT, LOGIN_PASSWORD, LOGIN_NUMERIC = "derust", "senha123", "1234"
LOGIN_CHAR, LOGIN_CLASS = "Heroi", 1
LOGIN_ACCOUNT_B, LOGIN_CHAR_B, LOGIN_CLASS_B = "derust2", "Parceiro", 2
# O /bug do passo 17 do LoginClient (kBugText em tools/login_client/LoginClient.cpp).
BUG_TEXT = "smoke: a porta do castelo nao abre @everyone"

# Itens sintéticos no formato de BASE_ReadItemListFile:
# índice, nome, malha.textura, nível.for.int.des.con, único, preço, pos, extra, grau, (efeito, valor)*
ITEMLIST_CSV = (
    "1,EspadaTeste,1.0,1.0.0.0.0,0,100,64,0,0,EF_DAMAGE,10,EF_GRID,0\n"
    "2,ArmaduraTeste,2.0,1.0.0.0.0,0,100,2,0,0,EF_AC,5\n"
    "412,PocaoTeste,3.0,.,0,10,0,0,0,EF_VOLATILE,1,EF_HP,50\n"
    "3467,BolsaTeste,4.0,.,0,10,0,0,0\n"
    # Casos difíceis do leitor, só para conferir tools/w2_bin_csv.py contra o ItemList.bin que o
    # próprio servidor grava (check_bin_csv): negativos, valores que não cabem num short, tokens
    # que o sscanf corta no meio, efeito que não existe (vira 128) e tab no lugar de vírgula.
    "6001,Negativo,-5.-7,-1.2.-3.4.-5,-1,-100,-2,-3,-4,EF_DAMAGE,-9\n"
    "6002,Prefixo,7x.3,1.2,5,12abc,1,0,0\n"
    "6003,EfeitoDesconhecido,1.0,.,0,1,0,0,0,EF_NAO_EXISTE,5,EF_HP,3\n"
    "6004\tTab,70000.1,0.0.0.0.0,0,1,0,0,0,EF_AC,40000\n"
)


def synthetic_base_mob(cls):
    """BaseMob da classe `cls` no layout do cliente (offsets de STRUCT_MOB conferidos em
    server/common/ProtocolLayoutCheck.cpp). Os valores são distintos por campo e por classe, para
    o LoginClient (ExpectedFor) saber que cada um chegou no lugar certo; mantenha os dois iguais."""
    mob = bytearray(SIZEOF_STRUCT_MOB)
    struct.pack_into("<B", mob, 20, cls)                  # Class
    struct.pack_into("<i", mob, 24, 1000 + cls)           # Coin
    struct.pack_into("<q", mob, 32, 5000000000 + cls)     # Exp (passa de 32 bits)
    # O DBSrv copia CurrentScore sobre BaseScore ao carregar o BaseMob (Server.cpp), e a lista de
    # personagens mostra CurrentScore: grava o mesmo STRUCT_SCORE nos dois.
    for score in (44, 92):                                # BaseScore, CurrentScore
        struct.pack_into("<i", mob, score, 1)             # Level
        struct.pack_into("<4h", mob, score + 32, 10 + cls, 20 + cls, 30 + cls, 40 + cls)  # Str/Int/Dex/Con
    struct.pack_into("<h", mob, 140 + 8, 2)               # Equip[1].sIndex: ArmaduraTeste
    struct.pack_into("<h", mob, 268, 412)                 # Carry[0].sIndex: PocaoTeste
    # Carry[1] e Carry[2]: espada e armadura para as trocas, com o grau (EF_SANC) da classe, para
    # os itens de A e de B serem diferentes.
    struct.pack_into("<h6B", mob, 268 + 1 * 8, 1, 43, cls, 0, 0, 0, 0)
    struct.pack_into("<h6B", mob, 268 + 2 * 8, 2, 43, cls, 0, 0, 0, 0)
    # Carry[61]: bolsa (libera Carry[45..59]), válida até 31/12/2099 (dia, mês, ano - 2000 nos
    # valores dos três efeitos, como BASE_CheckItemDate lê). Carry[60] fica sem bolsa.
    struct.pack_into("<h6B", mob, 268 + 61 * 8, 3467, 0, 31, 0, 12, 0, 99)
    struct.pack_into("<I", mob, 780, 0x00010203 + cls)    # LearnedSkill
    return bytes(mob)


def synthetic_npc_mob(name, *, level=1, hp=10, maxmp=0, merchant=0, exp=100):
    """NPC/monstro sintético de 1024 bytes para TMSRV/Run/npc/."""
    mob = bytearray(SIZEOF_STRUCT_MOB)
    enc = name.encode("latin-1")[:15]
    mob[0:len(enc)] = enc
    mob[16] = 2          # Clan (monstro não agressivo, como Porco)
    mob[17] = merchant   # Merchant
    struct.pack_into("<q", mob, 32, exp)
    for score in (44, 92):
        struct.pack_into("<h2xii4B4i", mob, score, level, 0, 0, merchant, 1, 0, 0, hp, maxmp, hp, maxmp)
    # Equip[0].sIndex: rosto de monstro (face = 202 / 10 >= 4). Com 0, BASE_GetCurrentScore trata o
    # NPC como personagem e recalcula MaxHp a partir de Con = 0.
    struct.pack_into("<h", mob, 140, 202)
    return bytes(mob)


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
    """Monta a árvore da produção, <work>/{Common,DBSRV/Run,TMSRV/Run} (os servidores abrem
    ../../Common e ../../<outro>/Run), com os diretórios de gravação de tools/linux_deploy.py.
    Sem exe_dir, não copia os servidores (tools/fuzz_tmsrv.py usa só os dados)."""
    common = work / "Common"
    db = work / "DBSRV" / "Run"
    tm = work / "TMSRV" / "Run"
    for d in (common, db, tm):
        d.mkdir(parents=True)  # work é <trabalho>/selftest ou <trabalho>/boot
    # Log, DayLog, account/<inicial>, char/<inicial>, Common/ImportUser...: os servidores gravam
    # neles mas não os criam.
    for base, subs in linux_deploy.WRITE_DIRS.items():
        for s in subs:
            (work / base / s).mkdir(parents=True, exist_ok=True)

    ips = local_ips()[:10]  # MAX_SERVERGROUP
    # Um grupo por endereço: índice 0 é o DBSrv, índice 1 o TMSrv.
    (common / "serverlist.txt").write_text("".join(f"{g} 0 {ip}\n{g} 1 {ip}\n" for g, ip in enumerate(ips)))
    shutil.copy(ROOT / "server/common/ItemEffect.h", common / "ItemEffect.h")
    (common / "itemlist.csv").write_text(ITEMLIST_CSV)
    for name in ("SkillData.csv", "InitItem.csv", "Guilds.txt", "GuildInfo",
                 "Settings/QuestsRate.txt", "Settings/CompRate.txt",
                 "Settings/MobMerc.txt", "Settings/CastleQuest.txt"):
        zero_file(common / name, 0)
    # UTF-8 + CP1252 de Âmago para conferir CReadFiles::ReadSancRate sem sobrescrever PO.
    (common / "Settings/SancRate.txt").write_bytes(b"PO 0 100\n\xc3\x82mago 0 50\n\xc2mago 1 40\n")

    if exe_dir is not None:
        shutil.copy(exe_dir / "DBSrv", db)
    (db / "Config.txt").write_text("Sapphire 8\nLastCapsule 0\n")
    zero_file(db / "AttributeMap.dat", 1024 * 1024)
    (db / "BaseMob").mkdir()
    for i, cls in enumerate(BASE_CLASSES):
        (db / "BaseMob" / cls).write_bytes(synthetic_base_mob(i))

    if exe_dir is not None:
        shutil.copy(exe_dir / "TMSrv", tm)
    # O gameconfig.txt é o que o próprio TMSrv grava quando não o encontra (DrawConfig).
    # Com DropItem 1, para o teste de jogo soltar item no chão.
    config = (DATA / "gameconfig.txt").read_text()
    assert config.count("DropItem 0") == 1
    (tm / "gameconfig.txt").write_text(config.replace("DropItem 0", "DropItem 1"))
    (tm / "localip.txt").write_text(ips[0] + "\n")  # o nome que o TMSrv abre (no Linux a caixa conta)
    zero_file(tm / "AttributeMap.dat", 1024 * 1024)
    zero_file(tm / "heightmap.dat", 4096 * 4096)  # MAX_GRIDY * MAX_GRIDX
    (tm / "LevelItem.txt").write_bytes(b"")
    (tm / "npc" / "AlvoTeste").write_bytes(synthetic_npc_mob("AlvoTeste", level=1, hp=8, exp=100, merchant=0))
    (tm / "npc" / "AuraTeste").write_bytes(synthetic_npc_mob("AuraTeste", level=5, hp=50, maxmp=9000, merchant=0))
    (tm / "npc" / "SegTeste").write_bytes(synthetic_npc_mob("SegTeste", level=2, hp=20, merchant=0))
    (tm / "npc" / "ImpostoTeste").write_bytes(synthetic_npc_mob("ImpostoTeste", level=5, hp=50, merchant=6))
    npcgen = (
        "#\n" * 8
        + "#\t[8]\n\tMinuteGenerate:\t1\n\tMaxNumMob:\t1\n\tMinGroup:\t1\n\tMaxGroup:\t1\n"
          "\tRouteType:\t6\n\tStartX:\t2105\n\tStartY:\t2042\n\tLeader:\tAlvoTeste\n"
        + "#\t[9]\n\tMinuteGenerate:\t1\n\tMaxNumMob:\t1\n\tMinGroup:\t6\n\tMaxGroup:\t6\n"
          "\tFormation:\t2\n\tRouteType:\t6\n\tStartX:\t1000\n\tStartY:\t1000\n"
          "\tStartRange:\t2\n\tLeader:\tAuraTeste\n\tFollower:\tSegTeste\n"
        + "#\t[10]\n\tMinuteGenerate:\t1\n\tMaxNumMob:\t1\n\tMinGroup:\t1\n\tMaxGroup:\t1\n"
          "\tRouteType:\t3\n\tStartX:\t1050\n\tStartY:\t1000\n\tSegment1X:\t1051\n\tSegment1Y:\t1000\n\tLeader:\tSegTeste\n"
        + "#\t[11]\n\tMinuteGenerate:\t1\n\tMaxNumMob:\t1\n\tMinGroup:\t1\n\tMaxGroup:\t1\n"
          "\tRouteType:\t1\n\tStartX:\t1080\n\tStartY:\t1000\n\tSegment1X:\t1081\n\tSegment1Y:\t1000\n\tLeader:\tSegTeste\n"
        + "#\t[12]\n\tMinuteGenerate:\t1\n\tMaxNumMob:\t1\n\tMinGroup:\t1\n\tMaxGroup:\t1\n"
          "\tRouteType:\t6\n\tStartX:\t1110\n\tStartY:\t1000\n\tLeader:\tImpostoTeste\n"
    )
    (tm / "NPCGener.txt").write_text(npcgen, encoding="ascii")
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


def server_log(directory):
    """Conteúdo do log do servidor (.\\Log\\*.txt), em latin-1."""
    return "".join(f.read_text("latin-1") for f in sorted((directory / "Log").glob("*.txt")))


def port_open(port, host="127.0.0.1"):
    try:
        with socket.create_connection((host, port), timeout=1):
            return True
    except OSError:
        return False


class WineHost:
    """wine 32 bits num prefixo próprio, num Xvfb próprio, para o LoginClient.exe."""

    def __init__(self, work):
        for tool in ("wine", "wineserver", "wineboot", "Xvfb"):
            if not shutil.which(tool):
                raise SystemExit(f"falta {tool} no PATH")
        self.xvfb, display = start_xvfb()
        self.env = dict(os.environ, WINEPREFIX=str(work / "wineprefix"), DISPLAY=display,
                        WINEDLLOVERRIDES="mscoree,mshtml=")
        self.env.pop("WAYLAND_DISPLAY", None)
        quiet = dict(self.env, WINEDEBUG="-all")
        subprocess.run(["wineboot", "-i"], env=quiet, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=300)
        subprocess.run(["wineserver", "-w"], env=quiet, timeout=120)

    def run(self, exe, args, cwd, timeout):
        quiet = dict(self.env, WINEDEBUG="-all")
        return subprocess.run(["wine", str(exe), *args], cwd=cwd, env=quiet, capture_output=True,
                              text=True, errors="replace", timeout=timeout)

    def close(self):
        subprocess.run(["wineserver", "-k"], env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.xvfb.send_signal(signal.SIGTERM)
        self.xvfb.wait(timeout=20)


class LinuxHost:
    """Linux nativo: DBSrv e TMSrv do build CMake. ServerAlert (common/Platform.cpp) escreve
    "[título] texto" no stderr do processo, gravado em <dir>/stderr.log. Um SIGTERM segue o
    caminho de fechar a janela no Windows (grava e encerra). O LoginClient.exe é um programa
    Windows e roda no wine."""

    def __init__(self, work, need_wine):
        self.wine = WineHost(work) if need_wine else None
        # W2_METRICS_DIR dos servidores (common/Metrics.h), definido no boot.
        self.metrics_dir = None

    def start(self, name, cwd):
        log = cwd / "stderr.log"
        env = dict(os.environ)
        if self.metrics_dir is not None:
            env["W2_METRICS_DIR"] = str(self.metrics_dir)
        proc = subprocess.Popen([str(cwd / name)], cwd=cwd, stdout=subprocess.DEVNULL, stderr=open(log, "wb"), env=env)
        return Server(self, name, proc, log)

    # Alertas que o teste provoca de propósito (a duplicação forçada): não contam como problema, e o
    # teste confere à parte que vieram.
    expected_alerts = []

    def problems(self, server):
        text = server.log.read_bytes().decode("utf-8", "replace") if server.log.exists() else ""
        out = [f'{server.name}: MessageBox "{l}"' for l in text.splitlines()
               if l.startswith("[") and not any(re.search(p, l) for p in self.expected_alerts)]
        code = server.proc.poll()
        if code is not None and code < 0:
            out.append(f"{server.name}: terminou pelo sinal {-code}")
        return out

    # Comandos do menu da janela pelo FIFO de controle (common/EventLoop.h): os que não derrubam o
    # servidor, com o texto que cada um ecoa no stderr. Reboot e Exit encerram e ficam de fora;
    # o encerramento já é conferido pelo SIGTERM.
    CONTROL = {
        "TMSrv": [("readgameconfig", "ReadGameConfig"), ("readguild", "ReadGuild"),
                  ("readguildname", "ReadGuildName"), ("reloadmob", "ReloadMob"), ("saveall", "SaveAll")],
        "DBSrv": [("reloadmobbase", "Reload MobBase")],
    }

    def control(self, servers, timeout=15):
        """Manda os comandos pelo FIFO de cada servidor e espera o eco "controle: <texto> (<id>)".
        Confere também o `help` e a recusa de um comando que não existe."""
        problems = []
        for s in servers:
            fifo = s.log.parent / "control.fifo"
            if not fifo.exists():
                problems.append(f"{s.name}: não criou {fifo.name}")
                continue
            cmds = self.CONTROL.get(s.name, [])
            with open(fifo, "w") as f:
                f.write("help\nnao_existe\n" + "".join(c + "\n" for c, _ in cmds))
            want = ["controle: comandos:", 'controle: comando desconhecido "nao_existe"'] + \
                   [f"controle: {label} (" for _, label in cmds]
            end = time.time() + timeout
            while True:
                text = s.log.read_bytes().decode("utf-8", "replace")
                missing = [w for w in want if w not in text]
                if not missing or time.time() > end:
                    break
                time.sleep(0.5)
            problems += [f"{s.name}: o FIFO de controle não ecoou \"{m}\"" for m in missing]
            if not missing:
                print(f"{s.name}: comandos do menu pelo FIFO de controle: {', '.join(c for c, _ in cmds)}")
        return problems

    def run(self, exe, args, cwd, timeout):
        if Path(exe).suffix.lower() == ".exe":
            return self.wine.run(exe, args, cwd, timeout)
        return subprocess.run([str(exe), *args], cwd=cwd, capture_output=True, text=True,
                              errors="replace", timeout=timeout)

    def stop(self, servers, graceful=False):
        """graceful: SIGTERM, o pedido de encerramento do laço de eventos; os dois servidores gravam
        (guildas, configuração, logs) e saem com código 0."""
        problems = []
        for s in reversed(servers):
            if graceful and s.proc.poll() is None:
                s.proc.send_signal(signal.SIGTERM)
                try:
                    code = s.proc.wait(timeout=30)
                    if code != 0:
                        problems.append(f"{s.name}: saiu com código {code} após SIGTERM")
                except subprocess.TimeoutExpired:
                    problems.append(f"{s.name}: não terminou 30s após SIGTERM")
            if s.proc.poll() is None:
                s.proc.kill()
            s.proc.wait(timeout=20)
        return problems

    def close(self):
        if self.wine:
            self.wine.close()


class Server:
    def __init__(self, host, name, proc, log):
        self.host, self.name, self.proc, self.log = host, name, proc, log

    def problems(self):
        msgs = self.host.problems(self)
        code = self.proc.poll()
        if code is not None:
            msgs.append(f"{self.name}: processo terminou (código {code} / 0x{code & 0xFFFFFFFF:08X})")
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


def find_pg_bin(given):
    """Diretório com initdb, pg_ctl e psql: o dado, o do pg_config ou o maior /usr/lib/postgresql/N/bin."""
    if given:
        return Path(given)
    if shutil.which("initdb"):
        return Path(shutil.which("initdb")).parent
    try:
        out = subprocess.run(["pg_config", "--bindir"], capture_output=True, text=True, check=True).stdout.strip()
        if (Path(out) / "initdb").exists():
            return Path(out)
    except (OSError, subprocess.CalledProcessError):
        pass
    found = sorted(Path("/usr/lib/postgresql").glob("*/bin/initdb"), key=lambda p: int(p.parent.parent.name))
    if not found:
        raise SystemExit("não achei o initdb do Postgres (--pg-bin)")
    return found[-1].parent


class FakeWebhook:
    """Servidor HTTP local no lugar do webhook do Discord: guarda o JSON de cada POST."""

    def __init__(self):
        posts = self.posts = []

        class Handler(http.server.BaseHTTPRequestHandler):
            def do_POST(self):
                body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
                posts.append((self.path, json.loads(body)))
                self.send_response(204)
                self.end_headers()

            def log_message(self, *a):
                pass

        self.server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        self.url = f"http://127.0.0.1:{self.server.server_address[1]}/api/webhooks/1/teste"
        threading.Thread(target=self.server.serve_forever, daemon=True).start()

    def close(self):
        self.server.shutdown()


def parse_metrics(text):
    """{série com rótulos: valor} do formato de texto do Prometheus (sem as linhas #)."""
    out = {}
    for line in text.splitlines():
        if not line or line.startswith("#"):
            continue
        name, _, value = line.rpartition(" ")
        out[name] = float(value)
    return out


def check_metrics_files(metrics_dir, w2db, dupe, quiet=False):
    """Os arquivos .prom do TMSrv e do DBSrv: formato e os contadores que o teste de jogo move."""
    problems = []
    values = {}
    for name in ("w2_tmsrv", "w2_dbsrv"):
        path = metrics_dir / f"{name}.prom"
        if not path.exists():
            problems.append(f"{name}.prom não foi gravado em W2_METRICS_DIR")
            continue
        text = path.read_text()
        families = re.findall(r"^# TYPE (\S+) (counter|gauge|summary)$", text, re.M)
        if len(families) != len(set(families)):
            problems.append(f"{name}.prom: família repetida")
        for line in text.splitlines():
            if line and not line.startswith("#") and not re.match(r'^[a-z_][a-z0-9_]*(\{[a-z_]+="[^"]*"\})? -?[0-9.e+]+$', line):
                problems.append(f"{name}.prom: linha fora do formato: {line!r}")
        values.update(parse_metrics(text))
        age = time.time() - values.get(f"{name}_last_write_timestamp_seconds", 0)
        if age > 60:
            problems.append(f"{name}.prom: gravado há {age:.0f}s (o intervalo é de 15 s)")
    if problems:
        return problems
    want = {
        "w2_tmsrv_packets_total": lambda v: v > 20,
        "w2_tmsrv_ticks_total": lambda v: v > 10,
        "w2_tmsrv_tick_seconds_total": lambda v: v > 0,
        "w2_tmsrv_bug_reports_total": lambda v: v == 1,
        "w2_tmsrv_dbsrv_connected": lambda v: v == 1,
        "w2_dbsrv_tmsrv_connected": lambda v: v == 1,
        "w2_dbsrv_saves_total": lambda v: v > 0,
        "w2_dbsrv_w2db_failures_total": lambda v: v == 0,
        "w2_dbsrv_w2db_pending_total": lambda v: v == 0,
    }
    if w2db:
        want["w2_dbsrv_bug_reports_total"] = lambda v: v == 1
        want["w2_dbsrv_dupe_suspects_total"] = (lambda v: v == 1) if dupe else (lambda v: v == 0)
    for series, ok in want.items():
        if series not in values or not ok(values[series]):
            problems.append(f"métricas: {series} = {values.get(series)}")
    if not problems and not quiet:
        print(f"métricas em arquivo: {values['w2_tmsrv_packets_total']:.0f} pacotes, "
              f"{values['w2_tmsrv_ticks_total']:.0f} ticks (pico {values['w2_tmsrv_tick_seconds_max'] * 1000:.1f} ms), "
              f"{values['w2_dbsrv_saves_total']:.0f} gravações, 1 /bug")
    return problems


def check_bug_logs(tm_log, db_log):
    """O /bug nos logs (gravados no encerramento)."""
    problems = []
    if f"etc,bug report {BUG_TEXT}" not in tm_log:
        problems.append("TMSrv: o /bug não está no log")
    if f"{LOGIN_CHAR}" not in db_log or BUG_TEXT not in db_log:
        problems.append("DBSrv: o /bug não chegou (log)")
    return problems


def check_w2db_ops(db, dupe, timeout=20):
    """O /bug no banco e no Discord falso, e o /metrics do w2db."""
    problems = []
    rows = db.sql("SELECT character_name, text, posted_at IS NOT NULL FROM bug_reports")
    if [(r[0], r[1]) for r in rows] != [(LOGIN_CHAR, BUG_TEXT)]:
        return [f"w2db: bug_reports {rows}, esperado o relato de {LOGIN_CHAR}"]
    end = time.time() + timeout
    while not db.webhook.posts and time.time() < end:
        time.sleep(0.2)
    if len(db.webhook.posts) != 1:
        problems.append(f"Discord falso: {len(db.webhook.posts)} post(s), esperado 1")
    else:
        body = db.webhook.posts[0][1]
        content = body.get("content", "")
        if LOGIN_CHAR not in content or "\\@everyone" not in content or body.get("allowed_mentions") != {"parse": []}:
            problems.append(f"Discord falso: post sem o relato escapado ou com menções: {body}")
    m = db.metrics()
    want = {
        "w2db_bug_reports_total": 1,
        "w2db_discord_posts_total": 1,
        "w2db_dupe_suspects_total": 1 if dupe else 0,
        "w2db_database_up": 1,
        'w2db_rpc_errors_total{method="SaveAccount"}': 0,
    }
    for series, v in want.items():
        if m.get(series) != v:
            problems.append(f"/metrics do w2db: {series} = {m.get(series)}, esperado {v}")
    if m.get('w2db_rpc_requests_total{method="SaveAccount"}', 0) == 0:
        problems.append("/metrics do w2db: nenhuma gravação contada")
    if not problems:
        saves = m['w2db_rpc_requests_total{method="SaveAccount"}']
        print(f"w2db: /bug no banco e no Discord falso (escapado, sem menção); /metrics com {saves:.0f} gravações")
    return problems


class W2Db:
    """Postgres próprio (só Unix socket, sem fsync) e o `w2db serve` em cima dele. Os sockets ficam
    num diretório curto do sistema: o caminho de um Unix socket tem limite de ~107 bytes."""

    def __init__(self, work, exe, pg_bin, timeout=60):
        self.work, self.exe, self.pg_bin = work, Path(exe), pg_bin
        work.mkdir(parents=True, exist_ok=True)
        self.sock = Path(tempfile.mkdtemp(prefix="w2db_"))
        self.data = work / "pg"
        self.proc = None
        quiet = {"stdout": subprocess.DEVNULL, "stderr": subprocess.STDOUT}
        subprocess.run([str(pg_bin / "initdb"), "-D", str(self.data), "-U", "w2", "--auth=trust", "-E", "UTF8",
                        "--locale=C", "--no-sync"], check=True, **quiet)
        opts = f"-c listen_addresses='' -k {self.sock} -c fsync=off -c full_page_writes=off"
        subprocess.run([str(pg_bin / "pg_ctl"), "-D", str(self.data), "-l", str(work / "pg.log"), "-o", opts, "-w",
                        "start"], check=True, **quiet)
        self.url = f"postgres://w2@localhost/postgres?host={self.sock}"
        self.sql("CREATE DATABASE w2db", db="postgres")
        self.url = f"postgres://w2@localhost/w2db?host={self.sock}"
        self.socket = self.sock / "w2db.sock"
        # Webhook falso do Discord para os relatos do /bug, e o /metrics numa porta livre.
        self.webhook = FakeWebhook()
        hook_file = work / "discord-webhook"
        hook_file.write_text(self.webhook.url + "\n")
        with socket.socket() as s:
            s.bind(("127.0.0.1", 0))
            self.metrics_url = f"http://127.0.0.1:{s.getsockname()[1]}/metrics"
        env = dict(os.environ, DATABASE_URL=self.url, RUST_LOG="info", NO_COLOR="1")
        log = open(work / "w2db.log", "wb")
        self.proc = subprocess.Popen([str(self.exe), "serve", "--socket", str(self.socket), "--metrics-addr",
                                      self.metrics_url.split("/")[2], "--discord-webhook-file", str(hook_file),
                                      "--discord-poll-secs", "1"], env=env, stdout=log, stderr=subprocess.STDOUT)
        end = time.time() + timeout
        while not self.socket.exists():
            if self.proc.poll() is not None or time.time() > end:
                raise SystemExit(f"w2db não subiu (log em {work / 'w2db.log'}):\n{self.log()}")
            time.sleep(0.2)
        print(f"w2db no ar em {self.socket} (Postgres em {self.data})")

    def sql(self, query, db="w2db"):
        """Linhas da consulta, cada uma uma tupla de textos."""
        r = subprocess.run([str(self.pg_bin / "psql"), "-h", str(self.sock), "-U", "w2", "-d", db, "-X", "-q", "-tA",
                            "-F", "\t", "-v", "ON_ERROR_STOP=1", "-c", query], capture_output=True, text=True)
        if r.returncode != 0:
            raise RuntimeError(f"psql: {r.stderr.strip()}")
        return [tuple(l.split("\t")) for l in r.stdout.splitlines() if l]

    def log(self):
        try:
            return (self.work / "w2db.log").read_text(errors="replace")
        except OSError:
            return ""

    def problems(self):
        out = []
        if self.proc is not None and self.proc.poll() is not None:
            out.append(f"w2db: saiu com código {self.proc.returncode}")
        for line in re.sub(r"\x1b\[[0-9;]*m", "", self.log()).splitlines():
            if " ERROR " in line or "panicked" in line:
                out.append(f"w2db: {line.strip()}")
        return out

    def metrics(self):
        """{série: valor} do /metrics do w2db."""
        text = urllib.request.urlopen(self.metrics_url, timeout=5).read().decode()
        return parse_metrics(text)

    def check(self):
        """`w2db check` no banco do teste."""
        r = subprocess.run([str(self.exe), "check"], capture_output=True, text=True,
                           env=dict(os.environ, DATABASE_URL=self.url, NO_COLOR="1", RUST_LOG="warn"))
        if r.returncode != 0:
            return [f"w2db check: {l}" for l in r.stdout.splitlines() if l.startswith("PROBLEMA")] or \
                [f"w2db check saiu com {r.returncode}: {r.stderr.strip()}"]
        print("w2db check: " + r.stdout.strip().splitlines()[-1])
        return []

    def close(self):
        self.webhook.close()
        if self.proc is not None and self.proc.poll() is None:
            self.proc.send_signal(signal.SIGTERM)
            try:
                self.proc.wait(10)
            except subprocess.TimeoutExpired:
                self.proc.kill()
        subprocess.run([str(self.pg_bin / "pg_ctl"), "-D", str(self.data), "-m", "fast", "-w", "stop"],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        shutil.rmtree(self.sock, ignore_errors=True)


def check_w2db(db):
    """Contas, personagens e itens que o jogo gravou no w2db."""
    problems = []
    accounts = {r[0]: r for r in db.sql(
        "SELECT name_key, password_hash LIKE '$argon2id$%', numeric_hash IS NOT NULL, version FROM accounts")}
    for account in (LOGIN_ACCOUNT, LOGIN_ACCOUNT_B):
        row = accounts.get(account.upper())
        if row is None:
            problems.append(f"w2db: a conta {account.upper()} não está no banco")
        elif row[1] != "t":
            problems.append(f"w2db: a senha da conta {account.upper()} não está em argon2id")
    if LOGIN_ACCOUNT.upper() in accounts and accounts[LOGIN_ACCOUNT.upper()][2] != "t":
        problems.append(f"w2db: a senha numérica de {LOGIN_ACCOUNT.upper()} não foi gravada")
    chars = {r[0]: r for r in db.sql(
        "SELECT c.name, a.name_key, c.class, (SELECT count(*) FROM items i WHERE i.character_id = c.id) "
        "FROM characters c JOIN accounts a ON a.id = c.account_id")}
    for name, account, cls in ((LOGIN_CHAR, LOGIN_ACCOUNT, LOGIN_CLASS), (LOGIN_CHAR_B, LOGIN_ACCOUNT_B, LOGIN_CLASS_B)):
        row = chars.get(name)
        if row is None:
            problems.append(f"w2db: o personagem {name} não está no banco")
            continue
        if row[1] != account.upper() or int(row[2]) != cls:
            problems.append(f"w2db: personagem {name} com conta/classe {row[1]}/{row[2]}, esperado {account.upper()}/{cls}")
        if int(row[3]) == 0:
            problems.append(f"w2db: o personagem {name} está sem itens no banco")
    if len(chars) != 2:
        problems.append(f"w2db: esperava 2 personagens (o apagado sai do banco), há {sorted(chars)}")
    orphans = db.sql("SELECT i.serial FROM items i WHERE NOT EXISTS (SELECT 1 FROM item_events e WHERE "
                     "e.serial = i.serial AND e.kind IN ('create', 'import', 'dupe_suspect'))")
    if orphans:
        problems.append(f"w2db: itens sem evento de criação: {[r[0] for r in orphans]}")
    if not problems:
        n = db.sql("SELECT count(*), count(DISTINCT kind) FROM item_events")[0]
        print(f"w2db: 2 contas e 2 personagens no banco, itens com serial ({n[0]} eventos, {n[1]} tipos)")
    return problems


ITEM_HOME = ("SELECT i.serial, a.name_key, (SELECT ca.name_key FROM item_events e JOIN accounts ca ON ca.id = "
             "e.account_id WHERE e.serial = i.serial AND e.kind = 'create'), i.ev1, "
             "(SELECT string_agg(e.kind, ',' ORDER BY e.id) FROM item_events e WHERE e.serial = i.serial) "
             "FROM items i JOIN accounts a ON a.id = i.account_id WHERE i.container <> 0 AND i.item_index = {}")


def check_w2db_trade(db):
    """A espada de cada conta (EF_SANC = classe) foi na troca de mão dupla para a outra, com o
    serial que nasceu na conta de origem."""
    problems = []
    rows = db.sql(ITEM_HOME.format(1))
    want = {(LOGIN_ACCOUNT.upper(), LOGIN_ACCOUNT_B.upper(), LOGIN_CLASS),
            (LOGIN_ACCOUNT_B.upper(), LOGIN_ACCOUNT.upper(), LOGIN_CLASS_B)}
    got = {(r[2], r[1], int(r[3])) for r in rows}
    if got != want:
        problems.append(f"w2db: espadas depois da troca (criada em, está em, grau): {sorted(got)}, esperado "
                        f"{sorted(want)}; linhas {rows}")
    else:
        hist = "; ".join(f"{r[0]}: {r[4]}" for r in rows)
        print(f"w2db: troca de mão dupla pelo serial: cada espada está na outra conta ({hist})")
    return problems


def check_w2db_dupe(db, servers, timeout=20):
    """A duplicação forçada: a armadura de A foi para B com o serial dela, e a cópia que ficou com A
    virou suspeita de duplicação (serial novo, evento dupe_suspect com o original), com aviso do
    DBSrv ao operador."""
    end = time.time() + timeout
    dupes = []
    while time.time() < end and not dupes:
        dupes = db.sql("SELECT e.serial, a.name_key, e.item_index, e.detail->>'original_serial' FROM item_events e "
                       "JOIN accounts a ON a.id = e.account_id WHERE e.kind = 'dupe_suspect'")
        time.sleep(0.5)
    problems = []
    armor = {r[0]: r for r in db.sql(ITEM_HOME.format(2))}
    if len(dupes) != 1:
        return [f"w2db: esperava uma suspeita de duplicação (a armadura forçada), há {dupes}"]
    serial, account, index, original = dupes[0]
    if account != LOGIN_ACCOUNT.upper() or int(index) != 2:
        problems.append(f"w2db: suspeita de duplicação em {account} item {index}, esperado {LOGIN_ACCOUNT.upper()} item 2")
    home = armor.get(original)
    if home is None or home[1] != LOGIN_ACCOUNT_B.upper() or home[2] != LOGIN_ACCOUNT.upper():
        problems.append(f"w2db: o serial original {original} da armadura duplicada não está em "
                        f"{LOGIN_ACCOUNT_B.upper()} vindo de {LOGIN_ACCOUNT.upper()}: {home}")
    db_server = next(s for s in servers if s.name == "DBSrv")
    text = db_server.log.read_bytes().decode("utf-8", "replace") if db_server.log.exists() else ""
    if not any(l.startswith("[w2db]") and "dupe suspect" in l for l in text.splitlines()):
        problems.append("DBSrv: não avisou o operador da suspeita de duplicação (alerta [w2db] no stderr)")
    if not problems:
        print(f"w2db: duplicação forçada acusada: a cópia de A ganhou o serial {serial} (original {original}, "
              f"que foi para {LOGIN_ACCOUNT_B.upper()}), e o DBSrv avisou o operador")
    return problems


def prepare(work, exe_dir):
    db_dir, tm_dir, game_host = build_tree(work, exe_dir)
    return db_dir, tm_dir, game_host


def self_test(host, work, exe_dir, timeout):
    """Confere o próprio detector: sem o SkillData.csv o DBSrv abre uma MessageBox antes de
    escutar em qualquer porta. Se o detector não a vir, o OK do teste principal não vale nada."""
    db_dir, _, _ = prepare(work, exe_dir)
    (work / "Common" / "SkillData.csv").unlink()
    server = host.start("DBSrv", db_dir)
    try:
        end = time.time() + timeout
        seen = []
        while time.time() < end and not seen:
            seen = server.problems()
            time.sleep(0.5)
    finally:
        host.stop([server])
    if any("MessageBox" in p and "SkillData.csv" in p for p in seen):
        print("autoteste: a MessageBox do SkillData.csv ausente foi detectada")
        return []
    return [f"autoteste: esperava a MessageBox do SkillData.csv ausente, veio {seen or 'nada'}"]


def import_accounts(common):
    """Arquivos de CReadFiles::ImportUser, um por conta: conta, senha, nome, SSN1, SSN2 (não
    nulos), e-mail, telefone, endereço e bônus, um por linha."""
    paths = []
    for account in (LOGIN_ACCOUNT, LOGIN_ACCOUNT_B):
        lines = [account, LOGIN_PASSWORD, "Teste", "111", "222", "teste@exemplo.invalid",
                 "5550000", "RuaTeste", "0"]
        path = common / "ImportUser" / account
        path.write_text("".join(l + "\n" for l in lines))
        paths.append(path)
    return paths


def login(host, work, db_dir, game_host, client, import_files, timeout, db=None, dupe=False):
    """Espera o DBSrv importar as contas e roda o LoginClient contra o TMSrv."""
    end = time.time() + timeout
    while any(f.exists() for f in import_files) and time.time() < end:
        time.sleep(0.5)
    for f in import_files:
        if f.exists():
            return [f"DBSrv: não importou a conta de {f} em {timeout:.0f}s"]
        if db is not None:
            if not db.sql(f"SELECT 1 FROM accounts WHERE name_key = '{f.name.upper()}'"):
                return [f"DBSrv: importou a conta {f.name} mas ela não está no w2db"]
            print(f"conta \"{f.name}\" criada pelo ImportUser no w2db")
            continue
        account = db_dir / "account" / f.name[0].upper() / f.name.upper()
        if not account.exists():
            return [f"DBSrv: importou a conta mas não gravou {account}"]
        print(f"conta \"{f.name}\" criada pelo ImportUser ({account.stat().st_size} bytes)")
    cwd = work / "login"
    cwd.mkdir()
    args = [game_host, str(GAME_PORT), LOGIN_ACCOUNT, LOGIN_PASSWORD, LOGIN_NUMERIC, LOGIN_CHAR, str(LOGIN_CLASS),
            LOGIN_ACCOUNT_B, LOGIN_CHAR_B, str(LOGIN_CLASS_B)] + (["dupe"] if dupe else [])
    try:
        r = host.run(client, args, cwd, timeout)
    except subprocess.TimeoutExpired:
        return [f"LoginClient: não terminou em {timeout:.0f}s"]
    for line in r.stdout.splitlines():
        print(f"  {line}")
    if r.returncode != 0:
        problems = [l for l in r.stdout.splitlines() if l.startswith("FALHA")]
        return [f"LoginClient: {p}" for p in problems] or [f"LoginClient: saiu com código {r.returncode}"]
    return []


def check_bin_csv(common):
    """Confere tools/w2_bin_csv.py contra o servidor: o ItemList.bin (e o ExtraItem.bin) que o
    servidor gravou a partir do ITEMLIST_CSV tem que ser exatamente o que o leitor imitado pelo
    script monta do mesmo csv, e o csv que o script tira do binário tem que voltar igual."""
    raw, _ = w2_bin_csv.read_table(common / "ItemList.bin", w2_bin_csv.ITEM_FMT, w2_bin_csv.MAX_ITEMLIST,
                                   "ItemList.bin")
    w2_bin_csv.apply_extra(raw, common / "ExtraItem.bin")
    effects, _ = w2_bin_csv.read_effect_names(common / "ItemEffect.h")
    mirror = w2_bin_csv.server_read_itemlist(ITEMLIST_CSV.encode(), effects)
    problems = []
    for idx, rec in enumerate(raw):
        rec = w2_bin_csv.item_record(rec)
        want = mirror.get(idx)
        if want is None:
            if not w2_bin_csv.is_empty(rec):
                problems.append(f"w2_bin_csv: o servidor gravou o item {idx}, que o leitor imitado não monta")
        elif want != rec:
            diff = [f"{f}: servidor {a!r}, script {b!r}" for f, a, b in zip(w2_bin_csv.ITEM_FIELDS, rec, want) if a != b]
            problems.append(f"w2_bin_csv: item {idx} difere do servidor ({'; '.join(diff)})")
    out = common.parent / "itemlist.from_bin.csv"
    diffs, _ = w2_bin_csv.convert_itemlist(common / "ItemList.bin", out, common / "ExtraItem.bin",
                                           common / "ItemEffect.h", quiet=True)
    # O 6003 tem o efeito 128, que não tem nome: é a única coisa que o csv não representa.
    problems += [f"w2_bin_csv: {d}" for d in diffs if not d.startswith("item 6003:")]
    if not problems:
        print(f"w2_bin_csv: o ItemList.bin do servidor confere com o leitor imitado ({len(mirror)} itens)")
    return problems


def check_status_page(common, db_started, timeout=45):
    """A página de status do DBSrv (Common/status/serv<NN>.htm), que ele grava a cada 30 s: uma
    linha por canal (MAX_SERVER), com a lotação do canal 1, onde está o TMSrv, e -1 nos outros."""
    deadline = db_started + timeout
    pages = []
    while time.time() < deadline:
        pages = sorted((common / "status").glob("serv*.htm"))
        if pages:
            break
        time.sleep(1)
    if not pages:
        return [f"DBSrv: não gravou a página de status em {common / 'status'} em {timeout}s"]
    lines = pages[0].read_text().split("\n")
    if lines[-1] == "":
        lines.pop()
    try:
        counts = [int(l) for l in lines]
    except ValueError:
        return [f"DBSrv: página de status com linha que não é número: {lines!r}"]
    if len(counts) != 10 or counts[0] < 0 or any(c != -1 for c in counts[1:]):
        return [f"DBSrv: página de status inesperada ({pages[0].name}): {counts}"]
    if list((common / "status").glob("*.part")):
        return ["DBSrv: sobrou um .part da página de status"]
    print(f"página de status: {pages[0].name} com o canal 1 no ar e os outros 9 em -1")
    return []


def boot(host, work, exe_dir, args):
    db_dir, tm_dir, game_host = prepare(work, exe_dir)
    import_files = import_accounts(work / "Common") if args.login_client else None
    servers = []
    graceful = False  # só encerra com WM_QUIT se os dois chegaram ao fim do tempo no ar
    db = None
    host.metrics_dir = work / "metrics"
    host.metrics_dir.mkdir(parents=True, exist_ok=True)
    try:
        if args.w2db:
            db = W2Db(work / "w2db", args.w2db, find_pg_bin(args.pg_bin))
            (db_dir / "w2db.txt").write_text(f"{db.socket}\n")
        t0 = db_started = time.time()
        servers.append(host.start("DBSrv", db_dir))
        problems = wait_port(DB_PORT, "127.0.0.1", servers, args.timeout)
        if problems:
            return problems
        print(f"DBSrv escutando na {DB_PORT} ({time.time() - t0:.1f}s)")
        t0 = time.time()
        servers.append(host.start("TMSrv", tm_dir))
        problems = wait_port(GAME_PORT, game_host, servers, args.timeout)
        if problems:
            return problems
        print(f"TMSrv conectado ao DBSrv e escutando em {game_host}:{GAME_PORT} ({time.time() - t0:.1f}s)")
        for h, port in (("127.0.0.1", DB_PORT), ("127.0.0.1", ADMIN_PORT), (game_host, GAME_PORT)):
            if not port_open(port, h):
                problems.append(f"conexão de teste em {h}:{port} recusada")
        if args.login_client and not problems:
            problems += login(host, work, db_dir, game_host, args.login_client, import_files, args.timeout, db,
                              args.forced_dupe)
            problems += [p for s in servers for p in s.problems()]
            if not problems:
                print("login e jogo de ponta a ponta conferidos")
            if db is not None and not problems:
                problems += check_w2db(db)
                problems += check_w2db_trade(db)
                if args.forced_dupe:
                    problems += check_w2db_dupe(db, servers)
        if hasattr(host, "control") and not problems:
            problems += host.control(servers)
            problems += [p for s in servers for p in s.problems()]
        if args.login_client and not problems and db is not None:
            problems += check_w2db_ops(db, args.forced_dupe)
        time.sleep(args.settle)
        if not problems:
            problems += check_status_page(work / "Common", db_started)
        if args.login_client and not problems:
            # Gravadas a cada 15 s: espera uma gravação depois do /bug.
            end = time.time() + 20
            while True:
                found = check_metrics_files(host.metrics_dir, db is not None, args.forced_dupe, quiet=True)
                if not found or time.time() > end:
                    break
                time.sleep(1)
            problems += check_metrics_files(host.metrics_dir, db is not None, args.forced_dupe)
        problems += [p for s in servers for p in s.problems()]
        if db is not None:
            problems += db.problems()
            if not problems:
                problems += db.check()
        graceful = not problems
    finally:
        stop_problems = host.stop(servers, graceful=graceful)
        if db is not None:
            stop_problems += db.problems()
            db.close()
    problems += stop_problems
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
    if not problems and args.login_client:
        problems += check_bug_logs(tm_log, db_log)
    if not problems:
        problems += check_bin_csv(work / "Common")
    if not problems:
        print(f"os dois seguem no ar após {args.settle:.0f}s, sem MessageBox")
    return problems


def main():
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    ap = argparse.ArgumentParser()
    ap.add_argument("exe_dir", help="diretório com DBSrv e TMSrv (build CMake no Linux)")
    ap.add_argument("--keep", help="diretório de trabalho a manter (padrão: temporário)")
    ap.add_argument("--settle", type=float, default=15, help="segundos com os dois no ar (padrão 15)")
    ap.add_argument("--timeout", type=float, default=90, help="espera máxima por cada porta (padrão 90)")
    ap.add_argument("--no-self-test", action="store_true", help="pula o autoteste do detector de MessageBox")
    ap.add_argument("--login-client", help="LoginClient.exe (padrão: o do diretório dos servidores, se houver)")
    ap.add_argument("--require-login", action="store_true", help="falha se não houver LoginClient.exe")
    ap.add_argument("--w2db", help="binário do w2db (services/w2db): contas no Postgres em vez de ./account")
    ap.add_argument("--pg-bin", help="diretório com initdb, pg_ctl e psql (padrão: procura)")
    ap.add_argument("--forced-dupe", action="store_true",
                    help="com --w2db: força uma duplicação numa troca (TMSrv com DERUST_TEST_HOOKS) e exige que seja acusada")
    args = ap.parse_args()
    if args.forced_dupe and not args.w2db:
        raise SystemExit("--forced-dupe precisa de --w2db")
    if args.w2db:
        args.w2db = Path(args.w2db).resolve()
        if not args.w2db.is_file():
            raise SystemExit(f"falta {args.w2db}")

    exe_dir = Path(args.exe_dir).resolve()
    for exe in ("DBSrv", "TMSrv"):
        if not (exe_dir / exe).is_file():
            raise SystemExit(f"falta {exe_dir / exe}")
    client = Path(args.login_client).resolve() if args.login_client else exe_dir / "LoginClient.exe"
    if client.exists():
        args.login_client = client
    elif args.require_login or args.login_client:
        raise SystemExit(f"falta {client} (--require-login)")
    else:
        args.login_client = None
        print(f"sem {client.name}: o login de ponta a ponta não será conferido")
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

    need_wine = bool(args.login_client) and Path(args.login_client).suffix.lower() == ".exe"
    host = LinuxHost(work, need_wine)
    if args.forced_dupe:
        host.expected_alerts.append(r"^\[w2db\] err,w2db 1 dupe suspect\(s\) in " + LOGIN_ACCOUNT.upper() + ",")
    print("plataforma: Linux nativo" + (" (LoginClient no wine + Xvfb)" if need_wine else ""))
    try:
        problems = [] if args.no_self_test else self_test(host, work / "selftest", exe_dir, args.timeout)
        if not problems:
            problems = boot(host, work / "boot", exe_dir, args)
    finally:
        host.close()

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
