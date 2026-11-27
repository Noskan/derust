#!/usr/bin/env python3
"""Avisos do gcc no build nativo do Linux (servidores e testes), com linha de base.

Compila o projeto com CMake num diretório novo, no Release que vai para a hospedagem, com os
mesmos avisos acompanhados da checagem mingw (WARN_TRACKED de tools/mingw_check.py:
sombreamento, não inicializada, fora dos limites, memset/memcpy suspeitos, return ausente...).
Compara as contagens por (tipo, arquivo) com tools/linux_warning_baseline-gcc<N>.txt, um arquivo
por versão principal do gcc. Falha se um arquivo ganhar avisos de algum tipo; se perder, pede
para regravar a linha de base.

Os avisos ficam só no build: as flags -W não mudam o código gerado, e o diretório de build
(--build-dir) serve para rodar os testes e publicar os binários.

Uso: python3 tools/linux_warning_check.py [--build-dir DIR] [--jobs N] [--update-warning-baseline]
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mingw_check as M  # noqa: E402

ROOT = M.ROOT
CXX = os.environ.get("CXX", "g++")


def gcc_major():
    out = subprocess.run([CXX, "-dumpversion"], capture_output=True, text=True).stdout.strip()
    m = re.match(r"\d+", out)
    return m.group(0) if m else "unknown"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build-dir", help="diretório do build (padrão: temporário, apagado no fim)")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    ap.add_argument("--update-warning-baseline", action="store_true", help="regrava a linha de base com os avisos atuais")
    args = ap.parse_args()
    sys.stdout.reconfigure(line_buffering=True)

    baseline = ROOT / "tools" / f"linux_warning_baseline-gcc{gcc_major()}.txt"
    header = [f"# Avisos do {CXX} {gcc_major()} (build Linux nativo) acompanhados por tools/linux_warning_check.py: <quantidade> <tipo> <arquivo>.",
              "# Regrave com: python3 tools/linux_warning_check.py --update-warning-baseline"]

    build = Path(args.build_dir).resolve() if args.build_dir else Path(tempfile.mkdtemp(prefix="linux_warn_"))
    if build.exists():
        shutil.rmtree(build)  # build do zero: um objeto já compilado não reemitiria os avisos
    flags = " ".join(f for f in M.WARN_FLAGS if f != "-O2")  # o nível de otimização é o do Release
    r = subprocess.run(["cmake", "-S", str(ROOT), "-B", str(build), "-DCMAKE_BUILD_TYPE=Release",
                        f"-DCMAKE_CXX_COMPILER={CXX}", f"-DCMAKE_CXX_FLAGS={flags}"],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout[-3000:], r.stderr[-3000:])
        sys.exit("cmake não configurou")
    r = subprocess.run(["cmake", "--build", str(build), "--parallel", str(args.jobs)],
                       capture_output=True, text=True, errors="replace")
    out = r.stdout + r.stderr
    if r.returncode != 0:
        errs = [l for l in out.splitlines() if " error" in l or "error:" in l][:20]
        print("\n".join(errs) or out[-3000:])
        sys.exit("o build nativo falhou")

    warns = set()
    for line in out.splitlines():
        m = M.WARN_RX.match(line.strip())
        if not m:
            continue
        path = Path(m.group("path"))
        if not path.is_absolute():
            path = (build / path).resolve()
        if ROOT in path.parents:
            warns.add((path.relative_to(ROOT).as_posix(), int(m.group("line")), m.group("flag"), m.group("msg")))

    status = M.check_warnings(warns, args.update_warning_baseline, baseline=baseline, header=header)
    if not args.build_dir:
        shutil.rmtree(build, ignore_errors=True)
    print(f"resultado: {'OK' if status == 0 else 'FALHOU'}")
    return status


if __name__ == "__main__":
    sys.exit(main())
