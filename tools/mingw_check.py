#!/usr/bin/env python3
"""Checagem cruzada com mingw-w64 i686 (Linux): rede de segurança complementar ao build MSVC.

O que faz:
  1. Gera shims de headers do sistema cujo nome difere só em maiúsculas/minúsculas
     (ex.: <Windows.h> -> <windows.h>), já que o mingw no Linux é case-sensitive.
  2. Compila todas as TUs de server/common, server/dbsrv, server/tmsrv e client/src.
  3. Linka DBSrv e w2pp_common_tests e executa os testes no wine (se disponível).

As TUs que hoje só compilam no MSVC ficam em tools/mingw_known_failures.txt. A checagem
falha se uma TU fora dessa lista não compilar e avisa quando uma TU da lista passar a
compilar (para que seja removida da lista).

Enquanto a Fase 3 não remove as construções exclusivas do MSVC, as linhas
`#pragma region`/`#pragma endregion` são descartadas antes de compilar (o GCC não as
aceita entre `}` e `else`).

Uso: python3 tools/mingw_check.py [--jobs N] [--no-wine] [--keep DIR]
"""

import argparse
import concurrent.futures as cf
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CXX = "i686-w64-mingw32-g++"
AR = "i686-w64-mingw32-ar"
KNOWN = ROOT / "tools" / "mingw_known_failures.txt"

COMMON_FLAGS = [
    "-std=gnu++17", "-fpermissive", "-w", "-fms-extensions",
    "-DWIN32", "-D_WINDOWS",
    # time_t de 64 bits, como no MSVC Win32 (mantém as travas de ABI de ProtocolLayoutCheck.cpp).
    "-D__MINGW_USE_VC2005_COMPAT",
]

# Módulo -> (diretórios de include, flags extras, compilar objeto?)
MODULES = {
    "server/common": (["server", "server/common"], [], True),
    "server/dbsrv": (["server", "server/common", "server/dbsrv"], [], True),
    "server/tmsrv": (["server", "server/common", "server/tmsrv"], [], False),
    "client/src": (["client/src"], ["-D_MBCS"], False),
    "tests": (["server", "server/common"], [], True),
}

PRAGMA_REGION = re.compile(rb"^[ \t]*#[ \t]*pragma[ \t]+(?:end)?region\b.*$", re.M)
SYS_INCLUDE = re.compile(rb'^[ \t]*#[ \t]*include[ \t]*<([^>]+)>', re.M)


def sysroot_include():
    out = subprocess.run([CXX, "-print-sysroot"], capture_output=True, text=True).stdout.strip()
    cands = [Path(out) / "include"] if out else []
    cands += [Path("/usr/i686-w64-mingw32/include"), Path("/usr/i686-w64-mingw32/sys-root/mingw/include")]
    for c in cands:
        if (c / "windows.h").exists():
            return c
    sys.exit("mingw-w64 i686 nao encontrado (instale mingw-w64 / g++-mingw-w64-i686)")


def make_shims(sources, shim_dir, inc_root):
    lower = {p.name.lower(): p.name for p in inc_root.iterdir() if p.is_file()}
    wanted = set()
    for s in sources:
        for m in SYS_INCLUDE.finditer(s.read_bytes()):
            wanted.add(m.group(1).decode("latin1"))
    made = []
    for h in sorted(wanted):
        if "/" in h or (inc_root / h).exists():
            continue
        real = lower.get(h.lower())
        if real and real != h:
            (shim_dir / h).write_text(f"#include <{real}>\n")
            made.append(f"{h} -> {real}")
    return made


def module_of(rel):
    for mod in MODULES:
        if rel.startswith(mod + "/"):
            return mod
    return None


def compile_tu(src, work, shim_dir):
    rel = src.relative_to(ROOT).as_posix()
    mod = module_of(rel)
    incs, extra, want_obj = MODULES[mod]
    flt = work / "src" / rel
    flt.parent.mkdir(parents=True, exist_ok=True)
    flt.write_bytes(PRAGMA_REGION.sub(b"", src.read_bytes()))
    obj = work / "obj" / (rel.replace("/", "_") + ".o")
    cmd = [CXX, *COMMON_FLAGS, *extra, f"-I{shim_dir}", "-iquote", str(src.parent)]
    cmd += [f"-I{ROOT / i}" for i in incs]
    cmd += ["-c", str(flt), "-o", str(obj)] if want_obj else ["-fsyntax-only", str(flt)]
    r = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
    errs = [l for l in r.stderr.splitlines() if " error: " in l or "fatal error" in l]
    return rel, r.returncode == 0, errs[:5], obj if want_obj else None


def run(cmd, **kw):
    shown = [Path(str(c)).name if str(c).startswith("/") else str(c) for c in cmd]
    print("+", " ".join(shown[:4]), "..." if len(shown) > 8 else "", " ".join(shown[-4:]) if len(shown) > 8 else " ".join(shown[4:]))
    return subprocess.run(cmd, **kw)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    ap.add_argument("--no-wine", action="store_true", help="nao executa os testes no wine")
    ap.add_argument("--keep", help="diretorio de trabalho a manter (padrao: temporario)")
    args = ap.parse_args()
    sys.stdout.reconfigure(line_buffering=True)

    if not shutil.which(CXX):
        sys.exit(f"{CXX} nao encontrado no PATH")
    inc_root = sysroot_include()

    known = set()
    if KNOWN.exists():
        known = {l.split("#")[0].strip() for l in KNOWN.read_text().splitlines()} - {""}

    sources = []
    for mod in MODULES:
        sources += sorted((ROOT / mod).rglob("*.cpp"))
    work = Path(args.keep) if args.keep else Path(tempfile.mkdtemp(prefix="mingw_check_"))
    (work / "obj").mkdir(parents=True, exist_ok=True)
    shim_dir = work / "shims"
    shim_dir.mkdir(exist_ok=True)
    scan = [f for mod in MODULES for f in (ROOT / mod).rglob("*") if f.suffix.lower() in (".h", ".hpp", ".cpp")]
    print("shims:", ", ".join(make_shims(scan, shim_dir, inc_root)) or "nenhum")

    results = {}
    with cf.ThreadPoolExecutor(args.jobs) as ex:
        for rel, ok, errs, obj in ex.map(lambda s: compile_tu(s, work, shim_dir), sources):
            results[rel] = (ok, errs, obj)

    new_fail = [r for r, (ok, _, _) in results.items() if not ok and r not in known]
    fixed = [r for r, (ok, _, _) in results.items() if ok and r in known]
    stale = sorted(known - set(results))
    total_fail = sum(1 for ok, _, _ in results.values() if not ok)
    print(f"\nTUs: {len(results)} compiladas, {total_fail} falharam ({len(known)} conhecidas)")

    status = 0
    for r in new_fail:
        print(f"::error file={r}::TU nao compila no mingw (fora de {KNOWN.name})")
        for e in results[r][1]:
            print("    " + e.replace(str(work / "src") + "/", ""))
        status = 1
    for r in fixed:
        print(f"::warning file={r}::TU agora compila no mingw: remova de {KNOWN.name}")
    for r in stale:
        print(f"::warning::{r} esta em {KNOWN.name} mas nao existe mais")

    # Link: w2pp_common como biblioteca estatica (igual ao CMake), DBSrv e testes.
    objs = lambda mod: [str(o) for r, (ok, _, o) in sorted(results.items()) if ok and o and r.startswith(mod + "/")]
    if all(results[r][0] for r in results if r.startswith(("server/common/", "server/dbsrv/", "tests/"))):
        lib = work / "libw2pp_common.a"
        lib.unlink(missing_ok=True)
        run([AR, "rcs", lib, *objs("server/common")], check=True)
        libs = ["-lws2_32", "-lwinmm"]
        if run([CXX, "-static", "-mwindows", *objs("server/dbsrv"), lib, "-o", work / "DBSrv.exe", *libs]).returncode:
            print("::error::falha ao linkar DBSrv")
            status = 1
        tests = work / "w2pp_common_tests.exe"
        if run([CXX, "-static", *objs("tests"), lib, "-o", tests, *libs]).returncode:
            print("::error::falha ao linkar w2pp_common_tests")
            status = 1
        elif not args.no_wine and shutil.which("wine"):
            env = dict(os.environ, WINEDEBUG="-all")
            r = run(["wine", tests], env=env, timeout=300)
            if r.returncode:
                print("::error::w2pp_common_tests falhou no wine")
                status = 1
        elif not args.no_wine:
            print("::warning::wine nao encontrado; testes nao executados")
    else:
        print("::error::server/common, server/dbsrv ou tests com falha de compilacao; link ignorado")
        status = 1

    print("\nresultado:", "OK" if status == 0 else "FALHOU", f"(trabalho em {work})")
    if not args.keep and status == 0:
        shutil.rmtree(work, ignore_errors=True)
    return status


if __name__ == "__main__":
    sys.exit(main())
