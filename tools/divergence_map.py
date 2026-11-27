#!/usr/bin/env python3
"""Mapa de divergências entre o servidor (W2PP) e o cliente (TMProject).

Os dois lados vêm de versões diferentes do jogo e cada um tem a sua cópia das structs, dos
opcodes e das regras `BASE_*`. Este script mede o quanto divergem, sem mudar código:

  1. Layout: tamanho de cada struct e offset/tamanho de cada campo, medidos pelo compilador
     (mingw i686, flags do tools/mingw_check.py) e executados no wine.
  2. Protocolo: para cada opcode, que struct cada lado envia (`x.Type = OPCODE` e os helpers
     `SendClientSignal*`) e que struct cada lado lê no `case OPCODE:` que o trata (o cast do
     próprio case ou da função que ele chama). Os pacotes são casados pelo valor do opcode,
     não pelo nome da struct.
  3. Regras: as funções `BASE_*` em comum recebem as mesmas entradas nos dois lados
     (tools/divergence/base_compare.cpp, compilado uma vez para cada lado) e as saídas são
     comparadas chamada a chamada.

Gera docs/DIVERGENCIAS.md. Uso: python3 tools/divergence_map.py [--keep DIR] [--out ARQ]
"""

import argparse
import concurrent.futures as cf
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from collections import OrderedDict, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import mingw_check as M  # noqa: E402

SIDES = OrderedDict(
    server=dict(
        label="servidor",
        headers=["shared/protocol/GameStructs.h", "server/common/Basedef.h"],
        includes=["common/Basedef.h"],
        incs=["shared", "server", "server/common"], extra=[],
        sources=["server/tmsrv", "server/common"],
        opcode=r"_MSG_\w+|0x[0-9A-Fa-f]+",
        rules_sources=sorted(str(p.relative_to(ROOT)) for p in (ROOT / "server/common").glob("*.cpp")),
        rules_libs=["-lws2_32", "-lwinmm"],
        common_cpp=sorted((ROOT / "server/common").glob("*.cpp")),
    ),
    client=dict(
        label="cliente",
        headers=["shared/protocol/Opcodes.h", "shared/protocol/GameStructs.h", "client/src/common/Basedef.h", "client/src/common/Structures.h"],
        includes=["core/pch.h", "common/Basedef.h", "common/Structures.h"],
        incs=["shared", "client/src"], extra=["-D_MBCS"],
        sources=["client/src"],
        opcode=r"MSG_\w+_Opcode|0x[0-9A-Fa-f]+",
        rules_sources=M.CLIENT_TEST_SOURCES,
        rules_libs=["-lwininet", "-ld3dx9"],
        common_cpp=[ROOT / f"client/src/common/{n}.cpp" for n in ("Basedef", "BaseItem", "BaseCombat", "BaseWorld")],
    ),
)

# Helpers do TMSrv que montam e enviam um pacote de struct fixa.
SERVER_SIGNAL_HELPERS = {
    "SendClientSignal": "MSG_STANDARD",
    "SendClientSignalParm": "MSG_STANDARDPARM",
    "SendClientSignalParm2": "MSG_STANDARDPARM2",
    "SendClientSignalParm3": "MSG_STANDARDPARM3",
    "SendClientSignalShortParm2": "MSG_STANDARDSHORTPARM2",
}

# Parâmetros que só um lado tem e o valor neutro usado em base_compare.cpp.
RULE_NOTES = {
    "BASE_GetMobAbility": "STRUCT_MOB preenchida campo a campo pelo nome",
    "BASE_GetMaxAbility": "STRUCT_MOB preenchida campo a campo pelo nome",
    "BASE_GetDoubleCritical": "STRUCT_MOB preenchida campo a campo pelo nome",
}

# Pacotes que seguem na tabela mas já foram conferidos à mão: por que ficam como estão.
PACKET_NOTES = {
    0x10A: "o cliente só lê `MSG_CNFRemoveServerLogin` já em campo, depois da troca de servidor (0x52A), que o servidor não implementa; na tela de login lê `MSG_CNFAccountLogin`, que bate",
    0x363: "o handler do cliente faz cast para `MSG_CreateMob` e para `MSG_CreateMobTrade`; o mapa só vê o primeiro. Na loja (0x363), o cliente lê a cidadania de `MSG_CreateMobTrade::Server` (byte 252), onde o servidor a envia; antes lia o byte 228 (`Desc[0]`), um erro da reconstrução corrigido",
}


# Structs que seguem na tabela mas já foram conferidas à mão: por que ficam como estão.
STRUCT_NOTES = {
    "STRUCT_ACCOUNTFILE": "o arquivo de conta só existe no DBSrv; o cliente declara a struct (e cinco variantes antigas) sem usá-la em lugar nenhum. O layout do servidor guarda `affect` e `mobExtra`, que o personagem salvo precisa, e mudá-lo invalidaria as contas gravadas",
}

def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), t, flags=re.S)
    return re.sub(r"//[^\n]*", "", t)


def read(p):
    return Path(p).read_text("utf-8", "replace")


# ------------------------------------------------------------------ headers
def structs_in(text):
    """nome -> lista de (campo, declaração, bitfield), só campos de nível 1."""
    t = strip_comments(text)
    out = {}
    for m in re.finditer(r"(?:^|\n)\s*(?:typedef\s+)?struct\s+(\w+)\s*(?::[^{;]*)?\{", t):
        name = m.group(1)
        i = j = m.end()
        d = 1
        while d and j < len(t):
            d += {"{": 1, "}": -1}.get(t[j], 0)
            j += 1
        fields, depth, cur = [], 0, ""
        for ch in t[i:j - 1]:
            if ch in "{}":
                depth += 1 if ch == "{" else -1
                cur = ""
                continue
            if depth:
                continue
            if ch != ";":
                cur += ch
                continue
            decl = re.sub(r"^(public|private|protected)\s*:\s*", "", " ".join(cur.split()))
            cur = ""
            if not decl or "(" in decl or decl.split()[0] in ("static", "friend", "using", "typedef", "enum"):
                continue
            if decl == "_MSG":   # macro do servidor com os campos de MSG_STANDARD
                fields.append(("Header", "_MSG", False))
                continue
            parts = decl.split(",")
            mt = re.match(r"^(.*?)([\*&\s]*)(\w+)\s*((?:\[[^\]]*\])*)\s*(:\s*\d+)?\s*(=.*)?$", parts[0].strip())
            if not mt:
                continue
            base = mt.group(1).strip()
            for k, p in enumerate(parts):
                mp = re.match(r"^([\*&\s]*)(\w+)\s*((?:\[[^\]]*\])*)\s*(:\s*\d+)?", p.strip() if k else p.strip()[len(base):].strip())
                if mp:
                    fields.append((mp.group(2), f"{base}{mp.group(1).strip()} {mp.group(2)}{mp.group(3)}", bool(mp.group(4))))
        out.setdefault(name, fields)
    return out


def opcode_names(side, text):
    t = strip_comments(text)
    rx = r"const\s+short\s+(_MSG_\w+)\s*=" if side == "server" else r"constexpr\s+auto\s+(MSG_\w+_Opcode)\s*="
    return list(OrderedDict.fromkeys(re.findall(rx, t)))


# ------------------------------------------------------------------ compilação
def compile_flags(side, work):
    cfg = SIDES[side]
    return [M.CXX, *M.COMMON_FLAGS, "-w", *cfg["extra"], f"-I{work / 'shims'}"] + [f"-I{ROOT / i}" for i in cfg["incs"]]


def measure_layout(side, structs, opcodes, work):
    cfg = SIDES[side]
    body = []
    for name, fields in structs.items():
        body.append(f'printf("S {name} %u\\n", (unsigned)sizeof({name}));')
        for f, decl, bit in fields:
            if bit or decl == "_MSG":
                continue
            body.append(f'printf("F {name} {f} %u %u\\n", (unsigned)offsetof({name}, {f}), (unsigned)sizeof((({name}*)0)->{f}));')
    for op in opcodes:
        body.append(f'printf("O {op} %d\\n", (int)(unsigned short)({op}));')
    head = [f'#include "{h}"' for h in cfg["includes"]] + ["#include <cstdio>", "#include <cstddef>", "int main() {"]
    src, exe = work / f"layout_{side}.cpp", work / f"layout_{side}.exe"
    src.write_text("\n".join(head + body + ["return 0; }"]) + "\n", "utf-8")
    r = subprocess.run(compile_flags(side, work) + [str(src), "-o", str(exe), "-static"],
                       capture_output=True, text=True, errors="replace", env=dict(os.environ, LC_ALL="C"))
    if r.returncode:
        sys.exit(f"layout do {side} não compila:\n{r.stderr[-3000:]}")
    out = subprocess.run(["wine", str(exe)], capture_output=True, text=True, env=dict(os.environ, WINEDEBUG="-all"),
                         timeout=300).stdout.replace("\r", "")
    sizes, fields, ops = {}, {}, {}
    for l in out.splitlines():
        p = l.split()
        if p and p[0] == "S":
            sizes[p[1]] = int(p[2])
        elif p and p[0] == "F":
            fields.setdefault(p[1], {})[p[2]] = (int(p[3]), int(p[4]))
        elif p and p[0] == "O":
            ops[p[1]] = int(p[2])
    for name, fl in structs.items():
        if any(d == "_MSG" for _, d, _ in fl):
            fields.setdefault(name, {})["Header"] = (0, 12)
    return sizes, fields, ops


def flatten(structs, sizes, fields):
    """Campos-folha de cada struct, descendo nas structs aninhadas: `Affect[].Level` é o campo Level do
    primeiro elemento de Affect (os outros elementos repetem o mesmo layout). Campos de tipo que não é
    struct medida (inteiros, arrays de char, unions) ficam como estão. Devolve também, por struct, as
    posições dos blocos de bytes (`char x[N]`): ali quem envia grava bytes crus, sem campos."""
    decls = {n: {f: d for f, d, _ in fl} for n, fl in structs.items()}
    memo = {}

    def leaves(name, depth=0):
        if name in memo:
            return memo[name]
        out, blocks = {}, []
        for f, pos in fields.get(name, {}).items():
            d = decls.get(name, {}).get(f, "")
            m = re.match(r"^(?:const\s+|struct\s+)*(\w+)\s+\w+((?:\[[^\]]*\])*)$", d)
            sub = m and m.group(1)
            # O cabeçalho fica inteiro: o servidor o declara pela macro _MSG, sem campos próprios.
            if sub and sub in fields and sub not in (name, "MSG_STANDARD") and depth < 6:
                arr = "[]" * len(re.findall(r"\[", m.group(2)))
                sub_out, sub_blocks = leaves(sub, depth + 1)
                for sf, (so, ss) in sub_out.items():
                    out[f"{f}{arr}.{sf}"] = (pos[0] + so, ss)
                blocks += [(pos[0] + bo, bs) for bo, bs in sub_blocks]
            else:
                out[f] = tuple(pos)
                if re.match(r"^(?:unsigned\s+|signed\s+)?char\s+\w+\[", d):
                    blocks.append(tuple(pos))
        memo[name] = (out, blocks)
        return memo[name]

    res = {n: leaves(n) for n in fields}
    return {n: r[0] for n, r in res.items()}, {n: r[1] for n, r in res.items() if r[1]}


def build_rules(side, work):
    cfg = SIDES[side]
    flags = compile_flags(side, work)
    objdir = work / f"obj_{side}"
    objdir.mkdir(exist_ok=True)

    def cc(src):
        obj = objdir / (Path(src).name + ".o")
        r = subprocess.run(flags + ["-c", str(ROOT / src), "-o", str(obj)], capture_output=True, text=True, errors="replace")
        if r.returncode:
            sys.exit(f"{src} não compila ({side}):\n{r.stderr[-2000:]}")
        return obj

    with cf.ThreadPoolExecutor(os.cpu_count() or 2) as ex:
        objs = list(ex.map(cc, cfg["rules_sources"]))
    lib = work / f"lib{side}.a"
    lib.unlink(missing_ok=True)
    subprocess.run([M.AR, "rcs", str(lib), *map(str, objs)], check=True)
    exe = work / f"base_compare_{side}.exe"
    r = subprocess.run(flags + [f"-DDIV_{side.upper()}", str(ROOT / "tools/divergence/base_compare.cpp"), str(lib),
                                "-o", str(exe), "-static", *cfg["rules_libs"]], capture_output=True, text=True, errors="replace")
    if r.returncode:
        sys.exit(f"base_compare ({side}) não linka:\n{r.stderr[-3000:]}")
    out = work / f"rules_{side}.tsv"
    subprocess.run(["wine", str(exe), str(out)], check=True, env=dict(os.environ, WINEDEBUG="-all"), timeout=1800)
    return out


def compare_rules(a_path, b_path):
    a = a_path.read_text("latin1").splitlines()
    b = b_path.read_text("latin1").splitlines()
    if len(a) != len(b):
        sys.exit("as duas execuções de base_compare geraram quantidades diferentes de linhas")
    st = OrderedDict()
    for x, y in zip(a, b):
        fa, ia, oa = x.split("\t")
        fb, ib, ob = y.split("\t")
        if (fa, ia) != (fb, ib):
            sys.exit(f"entradas desalinhadas: {x[:80]} | {y[:80]}")
        s = st.setdefault(fa, dict(eq=0, ne=0, ex=None))
        if oa == ob:
            s["eq"] += 1
        else:
            s["ne"] += 1
            if s["ex"] is None:
                s["ex"] = (ia, oa, ob)
    return st


# ------------------------------------------------------------------ funções BASE_* de cada lado
def function_bodies(files):
    out = {}
    for f in files:
        t = strip_comments(read(f))
        for m in re.finditer(r"^[A-Za-z_][\w\s\*&:]*?\b(BASE_\w+)\s*\(([^)]*)\)\s*\{", t, re.M):
            out[m.group(1)] = dict(params=" ".join(m.group(2).split()), file=Path(f).relative_to(ROOT).as_posix())
    return out


# ------------------------------------------------------------------ protocolo
def source_files(side):
    for d in SIDES[side]["sources"]:
        yield from sorted((ROOT / d).rglob("*.cpp"))


CAST_RX = re.compile(r"\(\s*(?:struct\s+)?(\w+)\s*\*\s*\)|\b(?:reinterpret|static)_cast\s*<\s*(?:struct\s+)?(\w+)\s*\*\s*>")


def first_cast(text, known):
    """Primeira struct conhecida num cast; MSG_STANDARD só se não houver outra (é o cabeçalho genérico)."""
    found = [m.group(1) or m.group(2) for m in CAST_RX.finditer(text)]
    found = [f for f in found if f in known]
    specific = [f for f in found if f != "MSG_STANDARD"]
    return specific[0] if specific else (found[0] if found else None)


def enclosing_class(text, pos):
    m = None
    for m in re.finditer(r"\b(\w+)::\w+\s*\(", text[max(0, pos - 60000):pos]):
        pass
    return m.group(1) if m else None


def packet_usage(side, opvalue, known):
    """valor do opcode -> {'send': {struct: {locais}}, 'recv': {struct: {locais}}}."""
    tok = SIDES[side]["opcode"]

    def value(t):
        if t.lower().startswith("0x"):
            return int(t, 16)
        if t.isdigit():   # decimal (`Header.Type = 724;`): só valores de pacote, acima de 0xFF
            return int(t) if int(t) > 0xFF else None
        return opvalue.get(t)

    # Nos envios também vale o decimal: o cliente monta vários pacotes com `Type = 652;`.
    tok_send = tok + r"|[1-9][0-9]{2,4}"

    texts = {p: strip_comments(read(p)) for p in source_files(side)}
    # Definições de função por nome, para seguir `case X: Exec_X(conn, pMsg);` / `return OnPacketY(pStd);`.
    defs = defaultdict(list)   # nome -> [(classe ou None, arquivo, posição do corpo, parâmetros)]
    for p, t in texts.items():
        for m in re.finditer(r"^[\w \t\*&:<>]*?\b(?:(\w+)::)?(\w+)\s*\(([^;{)]*)\)\s*(?:const\s*)?\{", t, re.M):
            j, d = m.end(), 1
            while d and j < len(t):
                d += {"{": 1, "}": -1}.get(t[j], 0)
                j += 1
            defs[m.group(2)].append((m.group(1), t[m.end():j], m.group(3)))
    use = defaultdict(lambda: dict(send=defaultdict(set), recv=defaultdict(set)))

    for p, t in texts.items():
        rel = p.relative_to(ROOT).as_posix()

        def where(pos):
            return f"{rel}:{t.count(chr(10), 0, pos) + 1}"

        for m in re.finditer(r"\b(\w+)\s*(?:\.|->)\s*(?:Header\s*\.\s*)?Type\s*=\s*(" + tok_send + r")\s*;", t):
            v = value(m.group(2))
            if v is None:
                continue
            var = m.group(1)
            before = t[max(0, m.start() - 12000):m.start()]
            decls = [d for d in re.finditer(r"\b(?:struct\s+)?(\w+)\s*\*?\s*\b" + re.escape(var) + r"\s*(?:[;=\[{(,)])", before)
                     if d.group(1) in known]
            if decls:
                use[v]["send"][decls[-1].group(1)].add(where(m.start()))
        if side == "server":
            for m in re.finditer(r"\b(SendClientSignal\w*)\s*\([^,()]+,[^,()]+,\s*(" + tok_send + r")\s*[,)]", t):
                st = SERVER_SIGNAL_HELPERS.get(m.group(1))
                v = value(m.group(2))
                if st and v is not None:
                    use[v]["send"][st].add(where(m.start()))
        # `if (pStd->Type == OPCODE)` seguido de um cast (TMSelectServerScene, por exemplo).
        for m in re.finditer(r"\b(?:pStd|packet|pMsg|pStandard|std|m|msg|pHeader)\s*(?:\.|->)\s*(?:Header\s*\.\s*)?Type\s*==\s*("
                             + tok + r")\b", t):
            v = value(m.group(1))
            if v is None:
                continue
            seg = t[m.end():m.end() + 800]
            nxt = re.search(r"Type\s*==", seg)
            st = first_cast(seg[:nxt.start()] if nxt else seg, known)
            if st:
                use[v]["recv"][st].add(where(m.start()))
        for m in re.finditer(r"\bcase\s+(" + tok + r")\s*:", t):
            v = value(m.group(1))
            if v is None:
                continue
            # Labels seguidos (`case A: case B: corpo`) compartilham o corpo do último.
            body_start = m.end()
            while True:
                nxt = re.match(r"\s*case\s+[\w:]+\s*:", t[body_start:])
                if not nxt:
                    break
                body_start += nxt.end()
            seg = t[body_start:body_start + 1500]
            stop = re.search(r"\bcase\s+[\w:]+\s*:|\bdefault\s*:|\bbreak\s*;|\breturn\b[^;]*;", seg)
            cut = seg[:stop.end() if stop else len(seg)]
            st = first_cast(cut, known) or ("MSG_STANDARD" if re.search(r"\b(?:pStd|pStandard|std)\s*->", cut) else None)
            call = re.search(r"\b(\w+)\s*\(\s*(?:[\w>\-]+\s*,\s*)?(?:\(\s*\w+\s*\*\s*\)\s*|\w+_cast\s*<[^>]*>\s*\(\s*)?"
                             r"(?:pStd|pMsg|msg|Msg|buf|pStandard|pBuf)\b", cut)
            handler_body = cut
            if (not st or st == "MSG_STANDARD") and call and call.group(1) in defs:
                cands = defs[call.group(1)]
                cls = enclosing_class(t, m.start())
                cands = [c for c in cands if c[0] == cls] or cands
                _, handler_body, params = cands[0]
                inner = first_cast(handler_body, known)
                ptype = next((x for x in re.findall(r"(\w+)\s*\*", params) if x in known), None)
                # Sem cast específico, o handler só lê o cabeçalho (ou nada além dele).
                st = next((c for c in (inner, ptype) if c and c != "MSG_STANDARD"), None) or inner or ptype or st or "MSG_STANDARD"
            if st:
                use[v]["recv"][st].add(where(m.start()))
                # Eco: o TMSrv costuma devolver o próprio pacote recebido sem reatribuir o Type.
                if side == "server" and st != "MSG_STANDARD":
                    for cv in re.findall(r"(\w+)\s*=\s*\(\s*(?:struct\s+)?" + st + r"\s*\*\s*\)", handler_body):
                        if re.search(r"\(\s*(?:char|MSG_STANDARD)\s*\*\s*\)\s*" + re.escape(cv) + r"\b", handler_body):
                            use[v]["send"][st].add(where(m.start()) + " (eco)")
                            break
    return use


# ------------------------------------------------------------------ relatório
def struct_field_diff(name, fs, fc):
    """Campos que mudam de offset/tamanho, que mudam de nome no mesmo offset ou que só existem de um lado."""
    a, b = fs.get(name, {}), fc.get(name, {})
    if name == "MSG_STANDARD":   # o servidor declara o cabeçalho pela macro _MSG; ver a nota do relatório
        return []
    a_by_pos = {v: k for k, v in a.items()}
    b_by_pos = {v: k for k, v in b.items()}
    out = []
    renamed_b = set()
    for f, v in sorted(a.items(), key=lambda kv: kv[1]):
        if f in b:
            if b[f] != v:
                out.append(f"`{f}` {v[0]}+{v[1]} → {b[f][0]}+{b[f][1]}")
        elif v in b_by_pos and b_by_pos[v] not in a:
            out.append(f"`{f}` → `{b_by_pos[v]}`")
            renamed_b.add(b_by_pos[v])
        else:
            out.append(f"`{f}` só no servidor ({v[0]}+{v[1]})")
    for f, v in sorted(b.items(), key=lambda kv: kv[1]):
        if f not in a and f not in renamed_b:
            out.append(f"`{f}` só no cliente ({v[0]}+{v[1]})")
    return out


def code_list(items, limit=None):
    items = list(items)
    shown = items if limit is None else items[:limit]
    s = ", ".join(f"`{x}`" for x in shown)
    return s + (f" e mais {len(items) - limit}" if limit is not None and len(items) > limit else "")


def write_report(out_path, L, rules, bodies, usage, only):
    S, C = L["server"], L["client"]
    lines = []
    w = lines.append

    def positions(side_layout, name):
        if name == "MSG_STANDARD":   # o servidor declara o cabeçalho pela macro _MSG
            return {"Header": (0, 12)}
        return side_layout["fields"].get(name, {})

    def leaves(side_layout, name):
        if name == "MSG_STANDARD":
            return {"Header": (0, 12)}
        return side_layout["flat"].get(name, {})

    def misaligned(sl, snd_name, rl, rcv_name):
        """Campos de quem lê (descendo nas structs aninhadas) sem um campo do mesmo offset e tamanho em quem
        envia, ou cujo nome existe em quem envia em outro offset (dois campos trocados de lugar)."""
        snd = leaves(sl, snd_name)
        have = {tuple(v) for v in snd.values()}
        starts = {v[0]: v[0] + v[1] for v in snd.values()}
        blocks = sl["blocks"].get(snd_name, [])

        def covered(pos):
            o, n = pos
            if tuple(pos) in have:
                return True
            # Dentro de um bloco de bytes de quem envia: bytes crus, sem campos para comparar.
            if any(b[0] <= o and o + n <= b[0] + b[1] for b in blocks):
                return True
            # Coberto por campos seguidos de quem envia (um array dividido em campos, como LearnedSkill).
            while o in starts and starts[o] <= pos[0] + n:
                o = starts[o]
                if o == pos[0] + n:
                    return True
            return False

        return [f for f, pos in sorted(leaves(rl, rcv_name).items(), key=lambda kv: kv[1])
                if not covered(pos) or (f in snd and tuple(snd[f]) != tuple(pos))]

    pairs = []
    for v in sorted(set(usage["server"]) | set(usage["client"])):
        su, cu = usage["server"].get(v), usage["client"].get(v)
        for direction, snd, rcv, sl, rl in (("servidor → cliente", su and su["send"], cu and cu["recv"], S, C),
                                            ("cliente → servidor", cu and cu["send"], su and su["recv"], C, S)):
            if not snd and not rcv:
                continue
            ss = sorted({sl["sizes"][x] for x in snd}) if snd else []
            rs = sorted({rl["sizes"][x] for x in rcv}) if rcv else []
            bad_fields, renamed = {}, []
            if snd and rcv:
                for r in sorted(rcv):
                    # Compara com a variante enviada de mesmo tamanho, se houver; senão com a maior.
                    same = [s for s in snd if sl["sizes"][s] == rl["sizes"][r]]
                    s = sorted(same or snd, key=lambda x: -sl["sizes"][x])[0]
                    if rl["sizes"][r] > sl["sizes"][s]:
                        continue
                    mis = misaligned(sl, s, rl, r)
                    if mis:
                        bad_fields[(s, r)] = mis
                    elif s == r and struct_field_diff(r, {r: positions(S, r)}, {r: positions(C, r)}):
                        renamed.append(r)
                if max(rs) > max(ss):
                    status = "lê além"
                elif bad_fields:
                    status = "desalinhado"
                elif renamed:
                    status = "nomes"
                else:
                    status = "ok"
            else:
                status = "só envia" if snd else "só lê"
            pairs.append(dict(v=v, dir=direction, snd=snd or {}, rcv=rcv or {}, ss=ss, rs=rs, status=status,
                              bad=bad_fields, renamed=sorted(set(renamed)), sl=sl, rl=rl))
    proto_ok = [p for p in pairs if p["status"] == "ok"]
    proto_bad = [p for p in pairs if p["status"] == "lê além"]
    proto_mis = [p for p in pairs if p["status"] == "desalinhado"]
    proto_names = [p for p in pairs if p["status"] == "nomes"]
    cli_only = [p for p in pairs if (p["dir"] == "cliente → servidor" and p["status"] == "só envia") or
                (p["dir"] == "servidor → cliente" and p["status"] == "só lê")]
    srv_names = defaultdict(list)
    for n, v in S["ops"].items():
        srv_names[v].append(n)

    def is_db(v):
        """Tráfego TMSrv <-> DBSrv/NPServer, que não passa pelo cliente."""
        names = srv_names.get(v, [])
        if names and all(n.startswith(("_MSG_DB", "_MSG_NP")) for n in names):
            return True
        return bool(v & 0xC00) and not v & 0x300

    srv_only_all = [p for p in pairs if p not in cli_only and p["status"] in ("só envia", "só lê")]
    srv_only = [p for p in srv_only_all if not is_db(p["v"])]

    game_structs = [n for n in sorted(set(S["sizes"]) & set(C["sizes"])) if not n.startswith("MSG_")]
    game_diff = [n for n in game_structs if S["sizes"][n] != C["sizes"][n] or struct_field_diff(n, S["fields"], C["fields"])]
    rule_bad = [f for f, s in rules.items() if s["ne"]]

    w("# Divergências entre servidor e cliente")
    w("")
    w("Gerado por `tools/divergence_map.py`; não edite à mão. O servidor (W2PP, `server/`) e o cliente")
    w("(TMProject, `client/`) vêm de versões diferentes do jogo, e cada um tem a sua cópia das structs, dos")
    w("opcodes e das regras `BASE_*`. O alvo escolhido é a versão do cliente: cada linha abaixo é algo que o")
    w("servidor teria de mudar para falar com este cliente, ou uma decisão a tomar.")
    w("")
    w("## Resumo")
    w("")
    w(f"- **Pacotes**, casados pelo valor do opcode e pela struct que cada lado envia ou lê (uma \"direção\" é "
      f"um opcode num sentido): **{len(proto_bad)} direções em que quem lê espera mais bytes do que o outro lado "
      f"envia** e **{len(proto_mis)} em que os campos lidos não estão nos offsets enviados**. Outras {len(proto_names)} "
      f"batem em offsets mas trocam nomes de campos, e {len(proto_ok)} batem por inteiro. Em {len(cli_only)}, o cliente "
      f"envia ou espera um pacote que o servidor não lê ou não envia; em {len(srv_only)}, é o servidor que envia ou lê "
      f"algo que o cliente não usa (fora {len(srv_only_all) - len(srv_only)} do tráfego com o DBSrv).")
    w(f"- **Structs de jogo e de arquivo** de mesmo nome: {len(game_structs)}, das quais {len(game_diff)} {'difere' if len(game_diff) == 1 else 'diferem'}. "
      f"`STRUCT_MOB` tem {S['sizes'].get('STRUCT_MOB')} bytes no servidor e {C['sizes'].get('STRUCT_MOB')} no cliente, "
      f"e vai dentro dos pacotes de login.")
    w(f"- **Regras `BASE_*`** comparadas com as mesmas entradas: {len(rules)}. **{len(rule_bad)} divergem** em ao "
      f"menos uma entrada; {len(rules) - len(rule_bad)} concordam em todas.")
    w(f"- **Só de um lado**: {len(only['server'])} funções `BASE_*` só no servidor e {len(only['client'])} só no cliente.")
    w("")
    w("## Como foi medido")
    w("")
    w("- **Layout**: `sizeof` e `offsetof` de cada struct e campo, compilados com o mingw i686 (os flags do")
    w("  `tools/mingw_check.py`, que reproduzem o layout do MSVC Win32) e executados no wine. Nada é estimado")
    w("  a partir do texto.")
    w("- **Pacotes**: um opcode é casado pelo valor (`_MSG_X` no servidor; `MSG_X_Opcode` ou literal no")
    w("  cliente; num envio, também o decimal acima de 255). A struct enviada vem de `var.Type = OPCODE` (com a declaração de `var`) e dos helpers")
    w("  `SendClientSignal*` do TMSrv. A struct lida vem do primeiro cast dentro do `case OPCODE:` ou da")
    w("  função que ele chama, ou do cast depois de um `if (p->Type == OPCODE)`. É análise de texto: um pacote montado de outro jeito (por exemplo, `memcpy`")
    w("  num buffer cru) não aparece, e o primeiro cast pode não ser a struct inteira do pacote. Quando o")
    w("  TMSrv devolve o pacote que recebeu (`SendOneMessage((char*)m, ...)` com o `m` do cast), o envio")
    w("  conta para o mesmo opcode e aparece marcado \"(eco)\". Um handler sem cast conta como leitura do")
    w("  cabeçalho (`MSG_STANDARD`). Cada linha")
    w("  traz o local no código para conferir.")
    w("- **Campos**: a comparação desce nas structs aninhadas (`MOB.CurrentScore.Level`; num array de structs,")
    w("  o primeiro elemento, `Affect[].Value`, já que os outros repetem o layout). Um campo de quem lê bate")
    w("  se do outro lado há um campo no mesmo offset e com o mesmo tamanho, se cai dentro de um bloco de")
    w("  bytes (`char x[N]`, que o outro lado preenche cru ou zera) ou se é coberto exatamente por campos")
    w("  seguidos (um array dividido em campos). Um nome que existe dos dois lados em offsets diferentes conta")
    w("  como desalinhado, mesmo que os offsets batam com outros campos: são dois campos trocados de lugar.")
    w("- **Regras**: `tools/divergence/base_compare.cpp` é compilado uma vez contra cada lado. As tabelas que o")
    w("  jogo carrega de arquivo (`g_pItemList`, `g_pSpell`, `g_pAttribute`, mapa de alturas) recebem os mesmos")
    w("  valores sintéticos, campo a campo, mesmo onde o nome ou o offset muda, e as entradas ficam no domínio")
    w("  que o jogo garante. Onde as assinaturas diferem, o parâmetro que só um lado tem recebe um valor neutro,")
    w("  indicado na tabela. As entradas são conferidas linha a linha (mesma função, mesma entrada nos dois")
    w("  lados) antes de comparar as saídas. O código do cliente é uma reconstrução com outro texto, então")
    w("  \"concorda\" quer dizer mesma saída, não mesmo código.")
    w("")

    def pair_table(rows, extra=None):
        w("| Opcode | Direção | Envia | Lê | Onde |" + (" Campos |" if extra else ""))
        w("|---|---|---|---|---|" + ("---|" if extra else ""))
        for p in rows:
            snd = ", ".join(f"`{x}` ({p['sl']['sizes'][x]})" for x in sorted(p["snd"]))
            rcv = ", ".join(f"`{x}` ({p['rl']['sizes'][x]})" for x in sorted(p["rcv"]))
            where = sorted(set().union(*p["snd"].values()))[:1] + sorted(set().union(*p["rcv"].values()))[:1]
            cell = f" {extra(p)} |" if extra else ""
            w(f"| `0x{p['v']:X}` | {p['dir']} | {snd} | {rcv} | {'<br>'.join(f'`{x}`' for x in where)} |{cell}")
        w("")
        notes = sorted({p["v"] for p in rows if p["v"] in PACKET_NOTES})
        for v in notes:
            w(f"- `0x{v:X}`: {PACKET_NOTES[v]}.")
        if notes:
            w("")

    def mis_cell(p):
        parts = []
        for (s, r), mis in sorted(p["bad"].items()):
            shown = ", ".join(f"`{f}`" for f in mis[:6]) + (f" e mais {len(mis) - 6}" if len(mis) > 6 else "")
            parts.append(f"`{r}`: {shown}")
        return "<br>".join(parts)

    w("## Pacotes em que quem lê espera mais do que recebe")
    w("")
    w("Tamanho em bytes da struct que um lado envia e da que o outro lê para o mesmo opcode. Aqui quem lê")
    w("espera mais bytes do que chegam: ou o pacote é recusado, ou os campos do fim saem do buffer.")
    w("")
    pair_table(proto_bad)

    w("## Pacotes com campos em outros offsets")
    w("")
    w("Quem lê cabe no que foi enviado, mas alguns dos campos que ele lê (também dentro das structs")
    w("aninhadas) não têm, do outro lado, um campo no mesmo offset e com o mesmo tamanho, ou têm o mesmo")
    w("nome em outro offset: o valor chega trocado ou cortado. A última coluna lista esses campos da struct")
    w("de quem lê.")
    w("")
    pair_table(proto_mis, mis_cell)

    w("<details><summary>Pacotes com os mesmos offsets e campos renomeados</summary>")
    w("")
    w("Mesma struct dos dois lados e mesmos offsets, mas com campos de outro nome. Em geral é só nomenclatura;")
    w("às vezes é outro significado no mesmo lugar (`Hold` virou `FakeExp`, por exemplo).")
    w("")
    pair_table(proto_names, lambda p: "<br>".join("; ".join(struct_field_diff(n, {n: positions(S, n)}, {n: positions(C, n)}))
                                                  for n in p["renamed"]))
    w("</details>")
    w("")

    w("## Pacotes que só o cliente usa")
    w("")
    w("O cliente envia e o servidor não tem um `case` que leia, ou o cliente tem um `case` para um opcode que")
    w("o servidor nunca envia. Parte pode ser recurso novo da versão do cliente; parte pode ser um envio que")
    w("a análise de texto não reconhece.")
    w("")
    w("A coluna \"No servidor\" mostra o nome do opcode quando o servidor o declara: nesses casos ele conhece o")
    w("valor, mas a análise não achou quem envie ou leia.")
    w("")
    w("| Opcode | Direção | Struct no cliente | Onde | No servidor |")
    w("|---|---|---|---|---|")
    for p in cli_only:
        m = p["snd"] if p["dir"] == "cliente → servidor" else p["rcv"]
        sts = ", ".join(f"`{t}` ({C['sizes'][t]})" for t in sorted(m))
        w(f"| `0x{p['v']:X}` | {p['dir']} | {sts} | `{sorted(set().union(*m.values()))[0]}` | "
          f"{code_list(sorted(srv_names.get(p['v'], []))) or '—'} |")
    w("")

    w("<details><summary>Pacotes que batem</summary>")
    w("")
    w("Quem lê cabe no que foi enviado e todos os campos que ele lê estão nos mesmos offsets.")
    w("")
    pair_table(proto_ok)
    w("")
    w("</details>")
    w("")

    w("## Structs de jogo e de arquivo")
    w("")
    w("Structs de mesmo nome fora dos pacotes. Quando mudam, mudam também os pacotes que as carregam e os")
    w("arquivos que as gravam (contas, `.bin`). `A → B` é um campo que mudou de nome no mesmo offset;")
    w("`offset+tamanho` em bytes.")
    w("")
    w("| Struct | Servidor | Cliente | Diferenças |")
    w("|---|---|---|---|")
    for n in game_diff:
        w(f"| `{n}` | {S['sizes'][n]} | {C['sizes'][n]} | {'; '.join(struct_field_diff(n, S['fields'], C['fields'])) or '—'} |")
    w("")
    for n in game_diff:
        if n in STRUCT_NOTES:
            w(f"- `{n}`: {STRUCT_NOTES[n]}.")
    if any(n in STRUCT_NOTES for n in game_diff):
        w("")
    w(f"Idênticas (mesmo tamanho, mesmos campos nos mesmos offsets): {code_list(n for n in game_structs if n not in game_diff)}.")
    w("")

    w("## Regras `BASE_*`")
    w("")
    w("Chamadas com a mesma entrada nos dois lados. O exemplo é a primeira entrada em que as saídas diferem.")
    w("")
    w("| Função | Chamadas | Divergem | Exemplo: entrada → servidor / cliente | Observação |")
    w("|---|---|---|---|---|")
    short = lambda x: x if len(x) <= 44 else x[:41] + "..."
    for f, s in sorted(rules.items(), key=lambda fs: (-fs[1]["ne"] / max(1, fs[1]["eq"] + fs[1]["ne"]), fs[0])):
        total = s["eq"] + s["ne"]
        ex = f"`{short(s['ex'][0])}` → `{short(s['ex'][1])}` / `{short(s['ex'][2])}`" if s["ex"] else ""
        pct = f"{100 * s['ne'] / total:.1f}%" if s["ne"] else "0"
        w(f"| `{f}` | {total} | {pct} | {ex} | {RULE_NOTES.get(f, '')} |")
    w("")
    skipped = sorted((set(bodies["server"]) & set(bodies["client"])) - set(rules))
    w("Em comum, mas fora da comparação (leem arquivo, rede ou relógio, ou os parâmetros não têm")
    w(f"correspondência): {code_list(skipped)}.")
    w("")

    w("## Só de um lado")
    w("")
    w(f"- Funções `BASE_*` só no cliente ({len(only['client'])}): {code_list(only['client'])}.")
    w(f"- Funções `BASE_*` só no servidor ({len(only['server'])}): {code_list(only['server'])}.")
    srv_sends = [p for p in srv_only if p["dir"] == "servidor → cliente"]
    srv_reads = [p for p in srv_only if p["dir"] == "cliente → servidor"]
    w(f"- Opcodes que o servidor envia e o cliente não trata ({len(srv_sends)}): "
      f"{', '.join(f'`0x{p['v']:X}`' for p in srv_sends)}.")
    w(f"- Opcodes que o servidor lê e o cliente não envia ({len(srv_reads)}): "
      f"{', '.join(f'`0x{p['v']:X}`' for p in srv_reads)}.")
    w("")
    out_path.write_text("\n".join(lines) + "\n", "utf-8")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--keep", help="diretório de trabalho a manter")
    ap.add_argument("--out", default=str(ROOT / "docs" / "DIVERGENCIAS.md"))
    args = ap.parse_args()
    sys.stdout.reconfigure(line_buffering=True)
    work = Path(args.keep).resolve() if args.keep else Path(tempfile.mkdtemp(prefix="divergence_"))
    if args.keep and work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True, exist_ok=True)
    (work / "shims").mkdir()
    scan = [f for d in ("server/common", "client/src", "tools/divergence") for f in (ROOT / d).rglob("*")
            if f.suffix.lower() in (".h", ".cpp")]
    M.make_shims(scan, work / "shims", M.sysroot_include())

    L = {}
    for side, cfg in SIDES.items():
        text = "\n".join(read(ROOT / h) for h in cfg["headers"])
        structs = structs_in(text)
        sizes, fields, ops = measure_layout(side, structs, opcode_names(side, text), work)
        flat, blocks = flatten(structs, sizes, fields)
        L[side] = dict(sizes=sizes, fields=fields, ops=ops, flat=flat, blocks=blocks)
        print(f"{side}: {len(sizes)} structs, {sum(len(v) for v in fields.values())} campos, {len(ops)} opcodes medidos")

    usage = {side: packet_usage(side, L[side]["ops"], set(L[side]["sizes"])) for side in SIDES}
    for side in SIDES:
        print(f"{side}: {len(usage[side])} opcodes com envio ou leitura identificados")

    outs = {side: build_rules(side, work) for side in SIDES}
    rules = compare_rules(outs["server"], outs["client"])
    print(f"regras: {len(rules)} funções, {sum(1 for s in rules.values() if s['ne'])} divergem")

    bodies = {side: function_bodies(SIDES[side]["common_cpp"]) for side in SIDES}
    only = {"server": sorted(set(bodies["server"]) - set(bodies["client"])),
            "client": sorted(set(bodies["client"]) - set(bodies["server"]))}
    (work / "data.json").write_text(json.dumps(dict(
        layout=L, rules=rules,
        usage={s: {f"0x{v:X}": {k: {t: sorted(x) for t, x in d.items()} for k, d in u.items()} for v, u in usage[s].items()}
               for s in usage}), indent=1, ensure_ascii=False), "utf-8")
    out = Path(args.out).resolve()
    write_report(out, L, rules, bodies, usage, only)
    print(f"relatório: {out.relative_to(ROOT) if out.is_relative_to(ROOT) else out}")
    if not args.keep:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    main()
