#!/usr/bin/env python3
"""Checagem cruzada com mingw-w64 i686 (Linux): rede de segurança complementar ao build MSVC.

O que faz:
  1. Gera shims de headers do sistema cujo nome difere só em maiúsculas/minúsculas
     (ex.: <Windows.h> -> <windows.h>), já que o mingw no Linux é case-sensitive.
  2. Compila todas as TUs de server/common, server/dbsrv, server/tmsrv e client/src.
  3. Linka DBSrv, TMSrv e um executável por fonte em tests/ (os do cliente com client/src/common) e executa os testes no wine (se disponível).
  4. Compara os avisos do GCC de uma lista curada (WARN_TRACKED: sombreamento, não inicializada,
     fora dos limites, memset/memcpy suspeitos, return ausente...) com a linha de base em
     tools/mingw_warning_baseline.txt. A checagem falha se um arquivo ganhar avisos de algum tipo;
     se perder, pede para atualizar a linha de base (--update-warning-baseline).

As TUs que hoje só compilam no MSVC ficam em tools/mingw_known_failures.txt. A checagem
falha se uma TU fora dessa lista não compilar e avisa quando uma TU da lista passar a
compilar (para que seja removida da lista).

Uso: python3 tools/mingw_check.py [--jobs N] [--no-wine] [--keep DIR] [--update-warning-baseline]
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


def gcc_major():
    out = subprocess.run([CXX, "-dumpversion"], capture_output=True, text=True).stdout.strip()
    m = re.match(r"\d+", out)
    return m.group(0) if m else "unknown"


# Um arquivo por versão principal do GCC: versões diferentes emitem conjuntos de avisos diferentes.
WARN_BASELINE = ROOT / "tools" / f"mingw_warning_baseline-gcc{gcc_major() if shutil.which(CXX) else 'unknown'}.txt"

# Avisos acompanhados (os demais ficam desligados). -O2 é necessário para os que dependem de
# análise de fluxo (maybe-uninitialized, array-bounds, stringop-overflow).
WARN_TRACKED = ["shadow", "uninitialized", "maybe-uninitialized", "array-bounds", "stringop-overflow",
                "restrict", "sizeof-pointer-memaccess", "memset-elt-size", "memset-transposed-args",
                "return-type", "delete-incomplete", "overflow"]
WARN_FLAGS = ["-O2", "-Wno-all", "-Wno-extra", "-Wno-permissive", "-Wno-write-strings", "-Wno-narrowing",
              "-Wno-conversion-null", "-Wno-int-to-pointer-cast", "-Wno-pointer-arith", "-Wno-multichar"]
WARN_FLAGS += [f"-W{w}" for w in WARN_TRACKED]
WARN_RX = re.compile(r"^(?P<path>[^:]+):(?P<line>\d+):(?P<col>\d+): warning: (?P<msg>.*) \[-W(?P<flag>[a-z0-9-]+)(?:=\d*)?\]$")

COMMON_FLAGS = [
    "-std=gnu++17", "-fpermissive", *WARN_FLAGS, "-fms-extensions",
    "-DWIN32", "-D_WINDOWS",
    # mingw-w64 mais antigos (ex.: Ubuntu 24.04) incluem <strsafe.h> a partir de <dshow.h>,
    # que redefine sprintf/strcpy/strcat como macros de deprecação.
    "-DSTRSAFE_NO_DEPRECATE",
    # Ponto flutuante em SSE2, como o MSVC Win32 (/arch:SSE2 é o padrão). O x87 do GCC i686 usa
    # precisão intermediária de 80 bits e muda resultados de BASE_GetCurrentScore conforme a otimização.
    "-msse2", "-mfpmath=sse",
    # time_t de 64 bits, como no MSVC Win32 (mantém as travas de ABI de ProtocolLayoutCheck.cpp).
    "-D__MINGW_USE_VC2005_COMPAT",
]

# Módulo -> (diretórios de include, flags extras, objetos entram no link?)
MODULES = {
    "server/common": (["server", "server/common"], [], True),
    "server/dbsrv": (["server", "server/common", "server/dbsrv"], [], True),
    "server/tmsrv": (["server", "server/common", "server/tmsrv"], [], True),
    "client/src": (["client/src"], ["-D_MBCS"], False),
    "tests/tmproject_characterization_tests.cpp": (["client/src"], ["-D_MBCS"], True),
    "tests": (["server", "server/common"], [], True),
}

# Testes do cliente: linkados com estes fontes do cliente (como em tests/CMakeLists.txt), não com w2pp_common.
CLIENT_TEST_SOURCES = [f"client/src/common/{n}.cpp" for n in ("Basedef", "BaseItem", "BaseCombat", "BaseWorld")] + \
    ["client/src/core/TMGlobal.cpp", "client/src/core/TMLog.cpp"]
CLIENT_TESTS = {"tmproject_characterization_tests"}

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
        if rel == mod or rel.startswith(mod + "/"):
            return mod
    return None


def compile_tu(src, work, shim_dir):
    rel = src.relative_to(ROOT).as_posix()
    incs, extra, want_obj = MODULES[module_of(rel)]
    obj = work / "obj" / (rel.replace("/", "_") + ".o")
    cmd = [CXX, *COMMON_FLAGS, *extra, f"-I{shim_dir}"] + [f"-I{ROOT / i}" for i in incs]
    # Todas as TUs geram objeto (necessário para os avisos de fluxo); só alguns módulos são linkados.
    cmd += ["-c", str(src), "-o", str(obj)]
    r = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
    errs = [l for l in r.stderr.splitlines() if " error: " in l or "fatal error" in l]
    warns = set()
    for l in r.stderr.splitlines():
        m = WARN_RX.match(l)
        if m and m.group("flag") in WARN_TRACKED:
            path = Path(m.group("path")).resolve()
            if ROOT in path.parents:
                warns.add((path.relative_to(ROOT).as_posix(), int(m.group("line")), m.group("flag"), m.group("msg")))
    return rel, r.returncode == 0, errs[:5], obj if want_obj or rel in CLIENT_TEST_SOURCES else None, warns


def check_warnings(warns, update):
    """Compara contagens de avisos por (tipo, arquivo) com a linha de base. Retorna 1 se piorou."""
    counts = {}
    for path, _, flag, _ in warns:
        counts[(flag, path)] = counts.get((flag, path), 0) + 1
    lines = [f"# Avisos do {CXX} {gcc_major()} acompanhados por tools/mingw_check.py: <quantidade> <tipo> <arquivo>.",
             "# Regrave com: python3 tools/mingw_check.py --update-warning-baseline"]
    lines += [f"{n} {flag} {path}" for (flag, path), n in sorted(counts.items(), key=lambda kv: (kv[0][1], kv[0][0]))]
    if update:
        WARN_BASELINE.write_text("\n".join(lines) + "\n")
        print(f"{WARN_BASELINE.name} regravada: {len(warns)} avisos em {len(counts)} pares (tipo, arquivo)")
        return 0
    if not WARN_BASELINE.exists():
        print(f"::warning::sem {WARN_BASELINE.name}; avisos não verificados. Conteúdo para gravar:")
        print("::group::" + WARN_BASELINE.name)
        print("\n".join(lines))
        print("::endgroup::")
        return 0
    base = {}
    if WARN_BASELINE.exists():
        for l in WARN_BASELINE.read_text().splitlines():
            if l.strip() and not l.startswith("#"):
                n, flag, path = l.split(" ", 2)
                base[(flag, path)] = int(n)
    status = 0
    for key in sorted(set(counts) | set(base), key=lambda k: (k[1], k[0])):
        flag, path = key
        have, had = counts.get(key, 0), base.get(key, 0)
        if have > had:
            status = 1
            print(f"::error file={path}::{have - had} aviso(s) novo(s) de -W{flag} (linha de base: {had}, agora: {have})")
            for p, line, f, msg in sorted(w for w in warns if w[0] == path and w[2] == flag):
                print(f"    {p}:{line}: {msg} [-W{f}]")
        elif have < had:
            print(f"::notice file={path}::-W{flag} caiu de {had} para {have}; atualize a linha de base (--update-warning-baseline)")
    print(f"avisos acompanhados: {len(warns)} (linha de base: {sum(base.values())})")
    return status


def run(cmd, **kw):
    shown = [Path(str(c)).name if str(c).startswith("/") else str(c) for c in cmd]
    print("+", " ".join(shown[:4]), "..." if len(shown) > 8 else "", " ".join(shown[-4:]) if len(shown) > 8 else " ".join(shown[4:]))
    return subprocess.run(cmd, **kw)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    ap.add_argument("--no-wine", action="store_true", help="nao executa os testes no wine")
    ap.add_argument("--keep", help="diretorio de trabalho a manter (padrao: temporario)")
    ap.add_argument("--update-warning-baseline", action="store_true",
                    help=f"regrava {WARN_BASELINE.name} com os avisos atuais")
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
        all_warns = set()
        for rel, ok, errs, obj, warns in ex.map(lambda s: compile_tu(s, work, shim_dir), sources):
            results[rel] = (ok, errs, obj)
            all_warns |= warns

    new_fail = [r for r, (ok, _, _) in results.items() if not ok and r not in known]
    fixed = [r for r, (ok, _, _) in results.items() if ok and r in known]
    stale = sorted(known - set(results))
    total_fail = sum(1 for ok, _, _ in results.values() if not ok)
    print(f"\nTUs: {len(results)} compiladas, {total_fail} falharam ({len(known)} conhecidas)")

    status = 0
    for r in new_fail:
        print(f"::error file={r}::TU nao compila no mingw (fora de {KNOWN.name})")
        for e in results[r][1]:
            print("    " + e.replace(str(ROOT) + "/", ""))
        status = 1
    for r in fixed:
        print(f"::warning file={r}::TU agora compila no mingw: remova de {KNOWN.name}")
    for r in stale:
        print(f"::warning::{r} esta em {KNOWN.name} mas nao existe mais")

    status |= check_warnings(all_warns, args.update_warning_baseline)

    # Link: w2pp_common como biblioteca estatica (igual ao CMake), DBSrv e testes.
    objs = lambda mod: [str(o) for r, (ok, _, o) in sorted(results.items()) if ok and o and r.startswith(mod + "/")]
    linked = ("server/common/", "server/dbsrv/", "server/tmsrv/", "tests/")
    if all(results[r][0] for r in results if r.startswith(linked) or r in CLIENT_TEST_SOURCES):
        lib = work / "libw2pp_common.a"
        lib.unlink(missing_ok=True)
        run([AR, "rcs", lib, *objs("server/common")], check=True)
        libs = ["-lws2_32", "-lwinmm"]
        if run([CXX, "-static", "-mwindows", *objs("server/dbsrv"), lib, "-o", work / "DBSrv.exe", *libs]).returncode:
            print("::error::falha ao linkar DBSrv")
            status = 1
        if run([CXX, "-static", "-mwindows", *objs("server/tmsrv"), lib, "-o", work / "TMSrv.exe", *libs]).returncode:
            print("::error::falha ao linkar TMSrv")
            status = 1
        # Um executável por fonte de teste (cada um tem seu main), como em tests/CMakeLists.txt.
        for obj in objs("tests"):
            name = Path(obj).name.replace("tests_", "", 1).rsplit(".cpp.o", 1)[0]
            exe = work / f"{name}.exe"
            if name in CLIENT_TESTS:
                deps = [str(results[r][2]) for r in CLIENT_TEST_SOURCES]
                cmd = [CXX, "-static", obj, *deps, "-o", exe, "-lwininet", "-ld3dx9"]
            else:
                cmd = [CXX, "-static", obj, lib, "-o", exe, *libs]
            if run(cmd).returncode:
                print(f"::error::falha ao linkar {name}")
                status = 1
            elif not args.no_wine and shutil.which("wine"):
                env = dict(os.environ, WINEDEBUG="-all")
                r = run(["wine", exe], env=env, timeout=300)
                if r.returncode:
                    print(f"::error::{name} falhou no wine")
                    status = 1
            elif not args.no_wine:
                print("::warning::wine nao encontrado; testes nao executados")
    else:
        print("::error::server/common, server/dbsrv, server/tmsrv, tests ou fontes dos testes do cliente com falha de compilacao; link ignorado")
        status = 1

    print("\nresultado:", "OK" if status == 0 else "FALHOU", f"(trabalho em {work})")
    if not args.keep and status == 0:
        shutil.rmtree(work, ignore_errors=True)
    return status


if __name__ == "__main__":
    sys.exit(main())
