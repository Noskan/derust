#!/usr/bin/env python3
"""Teste de partida dos servidores (DBSrv + TMSrv), no wine ou no Windows, com dados sintéticos.

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

Plataformas:
  - Linux nativo (o diretório tem DBSrv e TMSrv ELF, do build CMake no Linux): executa os
    binários direto, e manda os comandos do menu da janela pelo FIFO de controle
    (`control.fifo`), conferindo o eco de cada um. A MessageBox da camada server/compat escreve "[título] texto" no stderr,
    que vai para <dir>/stderr.log; o encerramento é por SIGTERM, que a camada transforma em
    WM_CLOSE. O LoginClient.exe continua rodando no wine: é um cliente Windows contra
    servidores Linux.
  - Linux com os .exe: wine (32 bits) e Xvfb, porque as janelas dos servidores pedem um display e o driver
    `null` do wine não serve para isso em todas as versões. Usa um prefixo do wine próprio,
    dentro do diretório de trabalho, para ligar o canal relay sem mexer no do usuário.
  - Windows: executa os .exe direto (é como o CI testa os builds MSVC). As MessageBox são lidas
    das janelas de diálogo do processo; exceções encerram o processo (sem janela do WER).

Uso: python3 tools/server_boot_smoke.py <dir com DBSrv e TMSrv (.exe ou ELF)> [--keep DIR] [--settle S]
       [--login-client EXE] [--require-login]
"""

import argparse
import os
import re
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
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
    # Carry[61]: bolsa (libera Carry[45..59]), válida até 31/12/2099 (dia, mês, ano - 2000 nos
    # valores dos três efeitos, como BASE_CheckItemDate lê). Carry[60] fica sem bolsa.
    struct.pack_into("<h6B", mob, 268 + 61 * 8, 3467, 0, 31, 0, 12, 0, 99)
    struct.pack_into("<I", mob, 780, 0x00010203 + cls)    # LearnedSkill
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


def build_tree(work, exe_dir, suffix):
    """Monta a árvore da produção, <work>/{Common,DBSRV/Run,TMSRV/Run} (os servidores abrem
    ../../Common e ../../<outro>/Run), com os diretórios de gravação de tools/linux_deploy.py."""
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
                 "Settings/SancRate.txt", "Settings/QuestsRate.txt", "Settings/CompRate.txt",
                 "Settings/MobMerc.txt", "Settings/CastleQuest.txt"):
        zero_file(common / name, 0)

    shutil.copy(exe_dir / ("DBSrv" + suffix), db)
    (db / "Config.txt").write_text("Sapphire 8\nLastCapsule 0\n")
    zero_file(db / "AttributeMap.dat", 1024 * 1024)
    (db / "BaseMob").mkdir()
    for i, cls in enumerate(BASE_CLASSES):
        (db / "BaseMob" / cls).write_bytes(synthetic_base_mob(i))

    shutil.copy(exe_dir / ("TMSrv" + suffix), tm)
    # O gameconfig.txt é o que o próprio TMSrv grava quando não o encontra (DrawConfig).
    # Com DropItem 1, para o teste de jogo soltar item no chão.
    config = (DATA / "gameconfig.txt").read_text()
    assert config.count("DropItem 0") == 1
    (tm / "gameconfig.txt").write_text(config.replace("DropItem 0", "DropItem 1"))
    (tm / "localip.txt").write_text(ips[0] + "\n")  # o nome que o TMSrv abre (no Linux a caixa conta)
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


class WineHost:
    """Linux: wine 32 bits num prefixo próprio, num Xvfb próprio. As MessageBox saem pelo canal
    relay (só MessageBoxA/W) no stderr do wine, que vai para <dir>/wine.log."""

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
        subprocess.run(["wine", "reg", "add", r"HKCU\Software\Wine\Debug", "/v", "RelayInclude",
                        "/d", "user32.MessageBoxA;user32.MessageBoxW", "/f"],
                       env=quiet, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True, timeout=120)
        subprocess.run(["wineserver", "-w"], env=quiet, timeout=120)
        self.relay = dict(self.env, WINEDEBUG="-all,+relay")

    def start(self, name, cwd):
        log = cwd / "wine.log"
        proc = subprocess.Popen(["wine", name + ".exe"], cwd=cwd, env=self.relay,
                                stdout=subprocess.DEVNULL, stderr=open(log, "wb"))
        return Server(self, name, proc, log)

    def problems(self, server):
        return wine_log_problems(server.name, server.log)

    def run(self, exe, args, cwd, timeout):
        quiet = dict(self.env, WINEDEBUG="-all")
        return subprocess.run(["wine", str(exe), *args], cwd=cwd, env=quiet, capture_output=True,
                              text=True, errors="replace", timeout=timeout)

    def stop(self, servers, graceful=False):
        # O msvcrt do wine já deixa o log em disco; matar o prefixo basta.
        subprocess.run(["wineserver", "-k"], env=self.env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        for s in servers:
            try:
                s.proc.wait(timeout=20)
            except subprocess.TimeoutExpired:
                s.proc.kill()
        return []

    def close(self):
        self.xvfb.send_signal(signal.SIGTERM)
        self.xvfb.wait(timeout=20)


class NativeHost:
    """Windows: executa os .exe direto. Uma MessageBox (e a caixa de asserção do CRT de Debug)
    é uma janela de diálogo de classe #32770 do processo; o texto está no controle 0xFFFF.
    SetErrorMode(SEM_NOGPFAULTERRORBOX), herdado pelos filhos, faz uma exceção não tratada
    encerrar o processo (com o código da exceção) em vez de abrir a janela do WER."""

    def __init__(self, work):
        import ctypes
        from ctypes import wintypes
        self.ctypes, self.wintypes = ctypes, wintypes
        ctypes.windll.kernel32.SetErrorMode(0x0001 | 0x0002 | 0x8000)  # FAILCRITICAL | NOGPFAULTERRORBOX | NOOPENFILEERRORBOX
        u = ctypes.WinDLL("user32", use_last_error=True)
        self.WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
        u.EnumWindows.argtypes = [self.WNDENUMPROC, wintypes.LPARAM]
        u.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
        u.GetClassNameW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
        u.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
        u.GetDlgItemTextW.argtypes = [wintypes.HWND, ctypes.c_int, wintypes.LPWSTR, ctypes.c_int]
        u.GetWindowThreadProcessId.restype = wintypes.DWORD
        u.PostThreadMessageW.argtypes = [wintypes.DWORD, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
        self.user32 = u

    def start(self, name, cwd):
        proc = subprocess.Popen([str(cwd / (name + ".exe"))], cwd=cwd,
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return Server(self, name, proc, None)

    def windows(self, pid):
        """(hwnd, thread id, classe) das janelas de topo do processo."""
        ctypes, wintypes, u = self.ctypes, self.wintypes, self.user32
        out = []

        def cb(hwnd, _):
            owner = wintypes.DWORD()
            tid = u.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
            if owner.value == pid:
                cls = ctypes.create_unicode_buffer(64)
                u.GetClassNameW(hwnd, cls, 64)
                out.append((hwnd, tid, cls.value))
            return True

        u.EnumWindows(self.WNDENUMPROC(cb), 0)
        return out

    def dialogs(self, pid):
        ctypes, wintypes, u = self.ctypes, self.wintypes, self.user32
        found = []

        def cb(hwnd, _):
            owner = wintypes.DWORD()
            u.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
            if owner.value == pid:
                cls = ctypes.create_unicode_buffer(64)
                u.GetClassNameW(hwnd, cls, 64)
                if cls.value == "#32770":
                    cap = ctypes.create_unicode_buffer(512)
                    txt = ctypes.create_unicode_buffer(4096)
                    u.GetWindowTextW(hwnd, cap, 512)
                    u.GetDlgItemTextW(hwnd, 0xFFFF, txt, 4096)
                    found.append(" / ".join(s for s in (txt.value.replace("\r\n", " ").strip(), cap.value) if s))
            return True

        u.EnumWindows(self.WNDENUMPROC(cb), 0)
        return found

    def problems(self, server):
        return [f'{server.name}: MessageBox "{d}"' for d in self.dialogs(server.proc.pid)]

    def run(self, exe, args, cwd, timeout):
        return subprocess.run([str(exe), *args], cwd=cwd, capture_output=True, text=True,
                              errors="replace", timeout=timeout)

    def stop(self, servers, graceful=False):
        """graceful: WM_QUIT na thread das janelas. O GetMessage do WinMain devolve 0 e a saída
        normal do CRT grava os logs (o Log() não faz fflush; matando o processo eles se perdem).
        O WM_CLOSE não serve: os dois servidores pedem confirmação numa MessageBox."""
        problems = []
        for s in reversed(servers):
            if graceful and s.proc.poll() is None:
                tids = {tid for _, tid, cls in self.windows(s.proc.pid) if cls != "#32770"}
                for tid in tids:
                    self.user32.PostThreadMessageW(tid, 0x0012, 0, 0)  # WM_QUIT
                try:
                    code = s.proc.wait(timeout=30)
                    if code != 0:
                        problems.append(f"{s.name}: saiu com código {code} / 0x{code & 0xFFFFFFFF:08X} após WM_QUIT")
                except subprocess.TimeoutExpired:
                    problems.append(f"{s.name}: não terminou 30s após WM_QUIT")
            if s.proc.poll() is None:
                s.proc.kill()
            s.proc.wait(timeout=20)
        return problems

    def close(self):
        pass


class LinuxHost:
    """Linux nativo: DBSrv e TMSrv do build CMake. A MessageBox de server/compat/w2_win32.cpp
    escreve "[título] texto" no stderr do processo, gravado em <dir>/stderr.log. Um SIGTERM
    vira WM_CLOSE, o mesmo caminho de fechar a janela no Windows (grava e encerra). O
    LoginClient.exe é um programa Windows e roda no wine."""

    def __init__(self, work, need_wine):
        self.wine = WineHost(work) if need_wine else None

    def start(self, name, cwd):
        log = cwd / "stderr.log"
        proc = subprocess.Popen([str(cwd / name)], cwd=cwd, stdout=subprocess.DEVNULL, stderr=open(log, "wb"))
        return Server(self, name, proc, log)

    def problems(self, server):
        text = server.log.read_bytes().decode("utf-8", "replace") if server.log.exists() else ""
        out = [f'{server.name}: MessageBox "{l}"' for l in text.splitlines() if l.startswith("[")]
        code = server.proc.poll()
        if code is not None and code < 0:
            out.append(f"{server.name}: terminou pelo sinal {-code}")
        return out

    # Comandos do menu da janela pelo FIFO de controle (server/compat): os que não derrubam o
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
        """graceful: SIGTERM, que a camada entrega como WM_CLOSE; os dois servidores gravam
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


def prepare(work, exe_dir):
    db_dir, tm_dir, game_host = build_tree(work, exe_dir, exe_suffix(exe_dir))
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


def login(host, work, db_dir, game_host, client, import_files, timeout):
    """Espera o DBSrv importar as contas e roda o LoginClient contra o TMSrv."""
    end = time.time() + timeout
    while any(f.exists() for f in import_files) and time.time() < end:
        time.sleep(0.5)
    for f in import_files:
        if f.exists():
            return [f"DBSrv: não importou a conta de {f} em {timeout:.0f}s"]
        account = db_dir / "account" / f.name[0].upper() / f.name.upper()
        if not account.exists():
            return [f"DBSrv: importou a conta mas não gravou {account}"]
        print(f"conta \"{f.name}\" criada pelo ImportUser ({account.stat().st_size} bytes)")
    cwd = work / "login"
    cwd.mkdir()
    args = [game_host, str(GAME_PORT), LOGIN_ACCOUNT, LOGIN_PASSWORD, LOGIN_NUMERIC, LOGIN_CHAR, str(LOGIN_CLASS),
            LOGIN_ACCOUNT_B, LOGIN_CHAR_B, str(LOGIN_CLASS_B)]
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


def boot(host, work, exe_dir, args):
    db_dir, tm_dir, game_host = prepare(work, exe_dir)
    import_files = import_accounts(work / "Common") if args.login_client else None
    servers = []
    graceful = False  # só encerra com WM_QUIT se os dois chegaram ao fim do tempo no ar
    try:
        t0 = time.time()
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
            problems += login(host, work, db_dir, game_host, args.login_client, import_files, args.timeout)
            problems += [p for s in servers for p in s.problems()]
            if not problems:
                print("login e jogo de ponta a ponta conferidos")
        if hasattr(host, "control") and not problems:
            problems += host.control(servers)
            problems += [p for s in servers for p in s.problems()]
        time.sleep(args.settle)
        problems += [p for s in servers for p in s.problems()]
        graceful = not problems
    finally:
        stop_problems = host.stop(servers, graceful=graceful)
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
    if not problems:
        problems += check_bin_csv(work / "Common")
    if not problems:
        print(f"os dois seguem no ar após {args.settle:.0f}s, sem MessageBox")
    return problems


def exe_suffix(exe_dir):
    """".exe" para os binários Windows, "" para os do build Linux."""
    return "" if os.name != "nt" and (exe_dir / "DBSrv").is_file() and (exe_dir / "TMSrv").is_file() else ".exe"


def main():
    # O console do Windows (cp1252) estraga os acentos das mensagens.
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    ap = argparse.ArgumentParser()
    ap.add_argument("exe_dir", help="diretório com DBSrv e TMSrv (.exe, ou ELF do build Linux)")
    ap.add_argument("--keep", help="diretório de trabalho a manter (padrão: temporário)")
    ap.add_argument("--settle", type=float, default=15, help="segundos com os dois no ar (padrão 15)")
    ap.add_argument("--timeout", type=float, default=90, help="espera máxima por cada porta (padrão 90)")
    ap.add_argument("--no-self-test", action="store_true", help="pula o autoteste do detector de MessageBox")
    ap.add_argument("--login-client", help="LoginClient.exe (padrão: o do diretório dos servidores, se houver)")
    ap.add_argument("--require-login", action="store_true", help="falha se não houver LoginClient.exe")
    args = ap.parse_args()

    exe_dir = Path(args.exe_dir).resolve()
    suffix = exe_suffix(exe_dir)
    for exe in ("DBSrv" + suffix, "TMSrv" + suffix):
        if not (exe_dir / exe).exists():
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

    if os.name == "nt":
        host, platform = NativeHost(work), "Windows nativo"
    elif suffix == "":
        need_wine = bool(args.login_client) and Path(args.login_client).suffix.lower() == ".exe"
        host = LinuxHost(work, need_wine)
        platform = "Linux nativo" + (" (LoginClient no wine + Xvfb)" if need_wine else "")
    else:
        host, platform = WineHost(work), "wine + Xvfb"
    print(f"plataforma: {platform}")
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
