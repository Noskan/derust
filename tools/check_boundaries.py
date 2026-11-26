#!/usr/bin/env python3
"""Verifica fronteiras de dependencia e estilo de #include em server/ e client/src/."""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
INCLUDE_RE = re.compile(rb'^(?:\xef\xbb\xbf)?[ \t]*#[ \t]*include[ \t]*"([^"]+)"', re.MULTILINE)

SOURCE_EXTENSIONS = {".h", ".hpp", ".cpp", ".cc", ".c"}

CLIENT_HIGHER_MODULES = {
    "render",
    "audio",
    "ui",
    "scene",
    "world",
    "entities",
    "skills",
    "effects",
    "gameplay",
}


def scan_files(base: pathlib.Path):
    for p in sorted(base.rglob("*")):
        if p.is_file() and p.suffix.lower() in SOURCE_EXTENSIONS:
            yield p


def check_repo() -> list[str]:
    errors: list[str] = []

    for area in ("server", "client/src"):
        base = ROOT / area
        if not base.exists():
            errors.append(f"Diretorio esperado nao encontrado: {area}")
            continue

        for path in scan_files(base):
            rel = path.relative_to(ROOT).as_posix()
            raw = path.read_bytes()
            for m in INCLUDE_RE.finditer(raw):
                inc = m.group(1).decode("latin1")
                line_no = raw[: m.start()].count(b"\n") + 1

                if "\\" in inc:
                    errors.append(f"{rel}:{line_no}: include com barra invertida: \"{inc}\"")
                if ".." in inc:
                    errors.append(f"{rel}:{line_no}: include relativo com '..': \"{inc}\"")

                top_mod = inc.split("/", 1)[0] if "/" in inc else ""

                if rel.startswith("server/common/"):
                    if top_mod in {"dbsrv", "tmsrv", "tools"}:
                        errors.append(f"{rel}:{line_no}: server/common nao pode depender de {top_mod}/: \"{inc}\"")
                elif rel.startswith("server/dbsrv/"):
                    if top_mod == "tmsrv":
                        errors.append(f"{rel}:{line_no}: server/dbsrv nao pode depender de tmsrv/: \"{inc}\"")
                elif rel.startswith("server/tmsrv/"):
                    if top_mod == "dbsrv":
                        errors.append(f"{rel}:{line_no}: server/tmsrv nao pode depender de dbsrv/: \"{inc}\"")
                elif rel.startswith("client/src/crypto/"):
                    if top_mod and top_mod != "crypto":
                        errors.append(f"{rel}:{line_no}: client/src/crypto nao pode depender de {top_mod}/: \"{inc}\"")
                elif rel.startswith("client/src/common/"):
                    if top_mod in CLIENT_HIGHER_MODULES:
                        errors.append(f"{rel}:{line_no}: client/src/common nao pode depender de {top_mod}/: \"{inc}\"")

    return errors


def main() -> int:
    errors = check_repo()
    if errors:
        print("Violacoes de fronteira detectadas:", file=sys.stderr)
        for err in errors:
            print(f"  - {err}", file=sys.stderr)
        return 1
    print("check_boundaries: OK (0 violacoes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
