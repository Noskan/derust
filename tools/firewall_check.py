#!/usr/bin/env python3
"""Testa as regras do nftables da VPS (deploy/nftables/derust.nft) com conexões de verdade.

Num namespace de rede próprio (unshare, sem mexer na rede da máquina), monta um segundo namespace,
o "de fora", ligado por um par veth com IPv4 e IPv6, carrega as regras e abre ouvintes em todas as
portas dos serviços. Do lado de fora:
  - só 22, 80, 443 e 8281 conectam, em IPv4 e IPv6;
  - as locais (DBSrv 7514, admin do DBSrv 8895, Postgres 5432, Prometheus 9090, Alertmanager
    9093, node_exporter 9100, w2db 9730, w2gm 8730, Grafana 3000) e uma qualquer não conectam;
  - uma rajada de conexões novas na 8281 e na 22 passa do limite por IP e parte delas cai.
Por dentro (loopback), todas conectam. As regras também são carregadas duas vezes, como num
`systemctl reload nftables` com a tabela já no ar.

Uso: python3 tools/firewall_check.py [--rules ARQUIVO] [--nft CAMINHO]
Sem root, usa um namespace de usuário (unshare -rn); como root (o CI), só o de rede.
"""

import argparse
import json
import os
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OPEN = [22, 80, 443, 8281]
LOCAL = [7514, 8895, 5432, 9090, 9093, 9100, 9730, 8730, 3000, 12345]
INSIDE4, OUTSIDE4 = "10.99.0.1", "10.99.0.2"
INSIDE6, OUTSIDE6 = "fd00:99::1", "fd00:99::2"
# Do burst (20 na 8281, 10 na 22) mais uma folga: a rajada tem de passar do limite.
BURST = {8281: 40, 22: 25}

CLIENT = r"""
import json, socket, sys, time
targets, burst = json.loads(sys.argv[1]), json.loads(sys.argv[2])
def ok(host, port, timeout):
    fam = socket.AF_INET6 if ":" in host else socket.AF_INET
    s = socket.socket(fam, socket.SOCK_STREAM)
    s.settimeout(timeout)
    try:
        s.connect((host, port))
        return True
    except OSError:
        return False
    finally:
        s.close()
out = {"reach": {f"{h} {p}": ok(h, p, 1.0) for h, p in targets}, "burst": {}}
for host, port, n in burst:
    out["burst"][f"{host} {port}"] = sum(ok(host, port, 0.5) for _ in range(n))
print(json.dumps(out))
"""


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        raise SystemExit(f"{' '.join(map(str, cmd))}: {r.stderr.strip() or r.stdout.strip()}")
    return r.stdout


def listen(port):
    socks = []
    for fam, host in ((socket.AF_INET, "0.0.0.0"), (socket.AF_INET6, "::")):
        s = socket.socket(fam, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if fam == socket.AF_INET6:
            s.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 1)
        s.bind((host, port))
        s.listen(256)
        socks.append(s)
    return socks


def inner(rules, nft):
    """Já no namespace de rede próprio: este é o lado da VPS."""
    problems = []
    run(["ip", "link", "set", "lo", "up"])
    run(["ip", "link", "add", "fw0", "type", "veth", "peer", "name", "fw1"])
    outside = subprocess.Popen(["unshare", "-n", "sleep", "120"])
    try:
        time.sleep(0.3)
        pid = str(outside.pid)
        run(["ip", "link", "set", "fw1", "netns", pid])
        run(["ip", "addr", "add", f"{INSIDE4}/24", "dev", "fw0"])
        run(["ip", "addr", "add", f"{INSIDE6}/64", "dev", "fw0", "nodad"])
        run(["ip", "link", "set", "fw0", "up"])
        out_ns = ["nsenter", "-t", pid, "-n"]
        for c in (["ip", "link", "set", "lo", "up"], ["ip", "addr", "add", f"{OUTSIDE4}/24", "dev", "fw1"],
                  ["ip", "addr", "add", f"{OUTSIDE6}/64", "dev", "fw1", "nodad"], ["ip", "link", "set", "fw1", "up"]):
            run(out_ns + c)

        run([nft, "-c", "-f", str(rules)])
        run([nft, "-f", str(rules)])
        run([nft, "-f", str(rules)])
        tables = run([nft, "list", "tables"]).split()
        if tables.count("derust") != 1:
            problems.append(f"depois de carregar duas vezes: {' '.join(tables)}")

        socks = [s for p in OPEN + LOCAL for s in listen(p)]
        targets = [(h, p) for h in (INSIDE4, INSIDE6) for p in OPEN + LOCAL]
        burst = [(h, p, n) for h in (INSIDE4, INSIDE6) for p, n in BURST.items()]
        res = json.loads(run(out_ns + [sys.executable, "-I", "-c", CLIENT, json.dumps(targets), json.dumps(burst)]))
        for key, reached in res["reach"].items():
            port = int(key.split()[1])
            want = port in OPEN
            if reached != want:
                problems.append(f"de fora, {key}: {'conectou' if reached else 'não conectou'}, "
                                f"esperado {'conectar' if want else 'bloquear'}")
        for key, got in res["burst"].items():
            n = BURST[int(key.split()[1])]
            if not 0 < got < n:
                problems.append(f"rajada de {n} conexões em {key}: {got} passaram (o limite por IP não agiu)")
            else:
                print(f"rajada em {key}: {got} de {n} passaram")
        for p in OPEN + LOCAL:
            for host in ("127.0.0.1", "::1"):
                fam = socket.AF_INET6 if ":" in host else socket.AF_INET
                with socket.socket(fam) as s:
                    s.settimeout(1)
                    if s.connect_ex((host, p)) != 0:
                        problems.append(f"por dentro, {host} {p}: não conectou")
        for s in socks:
            s.close()
    finally:
        outside.kill()
        outside.wait()
    if problems:
        for p in problems:
            print(f"::error::{p}")
        print("resultado: FALHA")
        return 1
    print(f"de fora só {', '.join(map(str, OPEN))} (IPv4 e IPv6); {len(LOCAL)} portas locais bloqueadas")
    print("resultado: OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rules", default=str(ROOT / "deploy/nftables/derust.nft"))
    ap.add_argument("--nft", default=shutil.which("nft") or "/usr/sbin/nft")
    args = ap.parse_args()
    rules = Path(args.rules).resolve()
    if os.environ.get("DERUST_FW_INNER") == "1":
        return inner(rules, args.nft)
    for tool in ("unshare", "nsenter", "ip"):
        if not shutil.which(tool):
            raise SystemExit(f"falta o {tool}")
    if not Path(args.nft).exists():
        raise SystemExit(f"falta o nft ({args.nft})")
    ns = ["unshare", "-n"] if os.geteuid() == 0 else ["unshare", "-rn"]
    env = dict(os.environ, DERUST_FW_INNER="1")
    return subprocess.run(ns + [sys.executable, "-I", __file__, "--rules", str(rules), "--nft", args.nft],
                          env=env).returncode


if __name__ == "__main__":
    sys.exit(main())
