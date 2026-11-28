#!/usr/bin/env python3
"""Roda os alvos de fuzzing do servidor (tests/fuzz) com libFuzzer.

O fuzz_client_message carrega o TMSrv inteiro a partir da árvore sintética do
tools/server_boot_smoke.py (só dados, nenhum arquivo de fora do repositório). Como o TMSrv, ele
muda para o diretório do próprio executável ao iniciar, então o binário é copiado para TMSRV/Run
da árvore. O fuzz_sock_read não lê arquivos.

Uso:
  python3 tools/fuzz_tmsrv.py <build-fuzz> [--target client|sock|all] [--time SEGUNDOS]
                              [--work DIR] [--jobs N] [--regress]

<build-fuzz> é um build com -DDERUST_BUILD_FUZZERS=ON (ver tests/fuzz/CMakeLists.txt).

Sem --regress, cada alvo roda --time segundos a partir das sementes geradas mais o corpus de
regressão versionado (tests/fuzz/regress/<alvo>), e o corpus que ele cresce fica em
<work>/corpus/<alvo>. Uma queda deixa a entrada em <work>/crash/<alvo>/ e sai com código 1.
Com --regress, só reexecuta cada arquivo de tests/fuzz/regress/<alvo> (rápido, para o CI).
"""

import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import server_boot_smoke  # noqa: E402  (build_tree: a árvore sintética)

TARGETS = {"client": "fuzz_client_message", "sock": "fuzz_sock_read"}
REGRESS = ROOT / "tests" / "fuzz" / "regress"
INITCODE = 0x1F11F311
ENV = dict(os.environ, ASAN_OPTIONS="detect_leaks=0:abort_on_error=1", UBSAN_OPTIONS="print_stacktrace=1")


def sock_seeds(out):
    """Fluxos de bytes como chegam do cliente: INITCODE e pacotes com cabeçalho de 12 bytes."""
    out.mkdir(parents=True, exist_ok=True)
    header = lambda size, typ: struct.pack("<hBBhhI", size, 0, 0, typ, 1, 0)
    seeds = {
        "init-only": struct.pack("<I", INITCODE),
        "one-header": struct.pack("<I", INITCODE) + header(12, 0x3A0),
        "two-packets": struct.pack("<I", INITCODE) + header(16, 0x36C) + b"\0" * 4 + header(12, 0x3A0),
        "big": struct.pack("<I", INITCODE) + header(120, 0x20D) + b"A" * 108,
    }
    for name, data in seeds.items():
        (out / name).write_bytes(data)


def prepare(target, build, work):
    exe = build / "tests" / "fuzz" / TARGETS[target]
    if not exe.is_file():
        raise SystemExit(f"falta {exe} (configure com -DDERUST_BUILD_FUZZERS=ON)")
    seeds = work / "seeds" / target
    if target == "client":
        tree = work / "tree"
        if not tree.exists():
            server_boot_smoke.build_tree(tree, None)
        run_dir = tree / "TMSRV" / "Run"
        shutil.copy(exe, run_dir / exe.name)
        exe = run_dir / exe.name
        seeds.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(exe)], cwd=run_dir, env=dict(ENV, W2_FUZZ_WRITE_SEEDS=str(seeds)), check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    else:
        run_dir = work
        sock_seeds(seeds)
    return exe, run_dir, seeds


def fuzz(target, build, work, seconds, jobs):
    exe, run_dir, seeds = prepare(target, build, work)
    corpus = work / "corpus" / target
    crash = work / "crash" / target
    corpus.mkdir(parents=True, exist_ok=True)
    crash.mkdir(parents=True, exist_ok=True)
    regress = REGRESS / target
    dirs = [str(corpus), str(seeds)] + ([str(regress)] if regress.is_dir() else [])
    cmd = [str(exe), f"-max_total_time={seconds}", "-timeout=25", "-rss_limit_mb=4096",
           "-max_len=9000", "-print_final_stats=1", f"-artifact_prefix={crash}/"]
    if jobs > 1:
        cmd += [f"-fork={jobs}", "-ignore_crashes=0"]
    print(f"== {target}: {seconds}s, corpus em {corpus}", flush=True)
    r = subprocess.run(cmd + dirs, cwd=run_dir, env=ENV, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       text=True, errors="replace")
    # O TMSrv escreve os ServerAlert ("[título] texto") no stderr; o resto é do libFuzzer. A saída
    # inteira fica em <work>/log/<alvo>.txt: a queda pode depender do estado do mundo acumulado e
    # não se repetir com a entrada sozinha.
    log = work / "log" / f"{target}.txt"
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(r.stdout)
    lines = [l for l in r.stdout.splitlines() if not l.startswith("[")]
    print("\n".join(lines[-40:]))
    found = sorted(crash.iterdir())
    if r.returncode != 0 or found:
        print(f"FALHA: {target} saiu com {r.returncode}; entradas em {crash}: {[p.name for p in found]}")
        return False
    return True


def regress(target, build, work):
    regress_dir = REGRESS / target
    files = sorted(regress_dir.iterdir()) if regress_dir.is_dir() else []
    if not files:
        print(f"== {target}: sem entradas de regressão")
        return True
    exe, run_dir, _ = prepare(target, build, work)
    r = subprocess.run([str(exe), "-runs=1", *map(str, files)], cwd=run_dir, env=ENV,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace")
    ok = r.returncode == 0
    print(f"== {target}: {len(files)} entrada(s) de regressão: {'OK' if ok else 'FALHA'}")
    if not ok:
        print("\n".join(l for l in r.stdout.splitlines() if not l.startswith("["))[-6000:])
    return ok


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("build", help="build com -DDERUST_BUILD_FUZZERS=ON")
    ap.add_argument("--target", choices=[*TARGETS, "all"], default="all")
    ap.add_argument("--time", type=int, default=60, help="segundos por alvo (padrão 60)")
    ap.add_argument("--work", help="diretório de trabalho (padrão: temporário, apagado no fim)")
    ap.add_argument("--jobs", type=int, default=1, help="processos do libFuzzer (-fork)")
    ap.add_argument("--regress", action="store_true", help="só reexecuta tests/fuzz/regress")
    args = ap.parse_args()

    build = Path(args.build).resolve()
    targets = list(TARGETS) if args.target == "all" else [args.target]
    tmp = None
    if args.work:
        work = Path(args.work).resolve()
        work.mkdir(parents=True, exist_ok=True)
    else:
        tmp = tempfile.mkdtemp(prefix="fuzz_tmsrv_")
        work = Path(tmp)
    ok = False
    try:
        ok = all([regress(t, build, work) if args.regress else fuzz(t, build, work, args.time, args.jobs)
                  for t in targets])
    finally:
        if tmp and ok:
            shutil.rmtree(tmp, ignore_errors=True)
    print("resultado:", "OK" if ok else f"FALHA (trabalho em {work})")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
