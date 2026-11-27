#!/usr/bin/env python3
"""Converte os .bin do cliente (ItemList.bin, ExtraItem.bin, SkillData.bin) nos .csv do servidor.

O servidor monta as tabelas a partir de Common/itemlist.csv e Common/SkillData.csv
(BASE_ReadItemListFile e BASE_InitializeSkill, em server/common/BaseLoaders.cpp). O cliente lê
as mesmas tabelas em binário (BASE_ReadItemList e BASE_ReadSkillBin, em
client/src/common/Basedef.cpp). Este script faz o caminho do binário para o csv, que é o que
falta para montar um servidor a partir de um cliente.

Formatos (layout do cliente, conferido por server/common/ProtocolLayoutCheck.cpp):
  ItemList.bin   6500 STRUCT_ITEMLIST de 140 bytes, com XOR 0x5A, mais 4 bytes de rodapé.
                 O cliente em Release só aceita o rodapé 0x1343B16.
  ExtraItem.bin  registros de 142 bytes sem XOR: índice (short) e um STRUCT_ITEMLIST. O cliente
                 e o servidor aplicam cada um por cima do ItemList (índices 1..6499).
  SkillData.bin  248 STRUCT_SPELL de 96 bytes, com XOR 0x5A, mais 4 bytes de rodapé (não conferido).

Também lê o layout do WYD Global atual (ItemList 14500 x 172, extraitem 2 + 172, SkillData
248 x 104), escolhido pelo tamanho do arquivo, e o leva ao do servidor. Não vão para o csv: os
itens de índice 6500 em diante, os bits 16+ do nPos, os 28 bytes finais do item e os dois int
finais da skill. O resumo do que ficou de fora sai no relatório.

Os nomes de efeito do itemlist.csv vêm do ItemEffect.h que o servidor lê em tempo de execução
(`#define EF_NOME valor`, lido como em BASE_InitializeEffectName). O padrão é o do repositório,
server/common/ItemEffect.h; passe --effects com o da sua árvore se for outro.

Conferência: depois de escrever o csv, o script o relê imitando o leitor do servidor (o mesmo
sscanf, a troca de vírgula por espaço, o corte de nome e o /4 do AffectTime) e compara campo a
campo com o binário. Tudo o que o servidor leria diferente é listado. Exemplos: um nome com
espaço, nIndexVisualEffect (o leitor zera), um efeito sem nome no ItemEffect.h, o ForceDamage
da skill (o csv não tem a coluna).
Saída: 0 se o servidor leria exatamente o binário, 1 se há diferenças (o csv é gravado assim
mesmo), 2 em erro de formato.

Uso:
  python3 tools/w2_bin_csv.py itemlist <ItemList.bin> -o <itemlist.csv> [--extra <ExtraItem.bin> | --no-extra]
                                [--effects <ItemEffect.h>]
  python3 tools/w2_bin_csv.py skilldata <SkillData.bin> -o <SkillData.csv>
  python3 tools/w2_bin_csv.py selftest

Por padrão o ExtraItem.bin ao lado do ItemList.bin é aplicado, como o cliente faz.
"""

import argparse
import random
import struct
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

MAX_ITEMLIST = 6500
MAX_SPELL_LIST = 248
MAX_EFFECTINDEX = 128
MAX_STATICEFFECT = 12
ITEMNAME_LENGTH = 64
ITEMLIST_CLIENT_TRAILER = 0x1343B16  # o único rodapé que o cliente em Release aceita

# STRUCT_ITEMLIST: Name[64], 8 shorts (malha, textura, efeito visual, 5 requisitos),
# 12 x {short sEffect, short sValue}, int nPrice, 4 shorts (nUnique, nPos, nExtra, nGrade).
ITEM_FMT = struct.Struct("<64s8h24hi4h")
# STRUCT_SPELL: 12 ints, Act1[8], Act2[8], 8 ints.
SPELL_FMT = struct.Struct("<12i8s8s8i")
assert ITEM_FMT.size == 140 and SPELL_FMT.size == 96

# Layout do WYD Global (cliente atual da Raid Hut, medido no patch 749). Os primeiros 132 bytes
# do item são os nossos (nome, malha, requisitos, 12 efeitos, preço). Depois vêm nUnique e nPos
# como int (o nPos usa os bits 16 e 17, posições que o nosso short não tem), nExtra e nGrade, e 28
# bytes sem equivalente aqui. Slots de efeito vazios são (255, 0). A skill é a nossa com dois
# int a mais no fim (o primeiro costuma ser o índice + 1).
G_MAX_ITEMLIST = 14500
G_ITEM_FMT = struct.Struct("<64s8h24hiiihh14h")
G_SPELL_FMT = struct.Struct("<12i8s8s10i")
assert G_ITEM_FMT.size == 172 and G_SPELL_FMT.size == 104
G_EMPTY_EFFECT = 255

ITEM_FIELDS = ["Name", "nIndexMesh", "nIndexTexture", "nIndexVisualEffect", "nReqLvl", "nReqStr",
               "nReqInt", "nReqDex", "nReqCon"] + \
              [f"stEffect[{j}].{f}" for j in range(MAX_STATICEFFECT) for f in ("sEffect", "sValue")] + \
              ["nPrice", "nUnique", "nPos", "nExtra", "nGrade"]
SPELL_INTS = ["SkillPoint", "TargetType", "ManaSpent", "Delay", "Range", "InstanceType", "InstanceValue",
              "TickType", "TickValue", "AffectType", "AffectValue", "AffectTime"]
SPELL_TAIL = ["InstanceAttribute", "TickAttribute", "Aggressive", "MaxTarget", "bParty", "AffectResist",
              "Passive", "ForceDamage"]
SPELL_FIELDS = SPELL_INTS + [f"Act1[{i}]" for i in range(8)] + [f"Act2[{i}]" for i in range(8)] + SPELL_TAIL

C_SPACE = b" \t\n\v\f\r"


class FormatError(Exception):
    pass


# ---------------------------------------------------------------------------------------------
# Leitura dos binários

def xor(data, key):
    return bytes(b ^ key for b in data)


def detect_layout(path, layouts):
    """Escolhe o layout pelo tamanho do arquivo: [(nome, Struct, quantidade)]."""
    size = Path(path).stat().st_size
    for name, rec, count in layouts:
        if size in (rec.size * count, rec.size * count + 4):
            return name, rec, count
    return layouts[0]  # read_table explica o erro com o layout do cliente


def global_item_to_client(raw, lost):
    """Registro do Global (172 bytes) -> o do cliente (140), contando o que fica de fora em `lost`."""
    name, shorts, fx = raw[0], list(raw[1:9]), list(raw[9:33])
    price, unique, pos, extra, grade = raw[33:38]
    tail = raw[38:]
    for j in range(MAX_STATICEFFECT):
        if fx[2 * j] == G_EMPTY_EFFECT and fx[2 * j + 1] == 0:
            fx[2 * j] = 0
    if s16(unique) != unique:
        lost["nUnique fora de short"] = lost.get("nUnique fora de short", 0) + 1
    if pos & ~0xFFFF:
        lost["nPos com os bits 16+ (posições que o servidor não tem)"] = \
            lost.get("nPos com os bits 16+ (posições que o servidor não tem)", 0) + 1
    for k, v in enumerate(tail):
        if v:
            key = f"campo extra do Global no byte {144 + 2 * k}"
            lost[key] = lost.get(key, 0) + 1
    return [name] + shorts + fx + [price, s16(unique), s16(pos), extra, grade]


def global_spell_to_client(raw, idx, lost):
    tail = raw[14:]
    if tail[8] != idx + 1 or tail[9]:
        lost["int extra da skill do Global (fora do esperado: índice + 1, 0)"] = \
            lost.get("int extra da skill do Global (fora do esperado: índice + 1, 0)", 0) + 1
    return list(raw[:14]) + list(tail[:8])


def read_table(path, rec, count, what):
    data = Path(path).read_bytes()
    size = rec.size * count
    if len(data) not in (size, size + 4):
        hint = ""
        for extra in (0, 4):
            body = len(data) - extra
            if body > 0 and body % rec.size == 0:
                hint = f"; {body // rec.size} registros de {rec.size} bytes" + (" mais 4 de rodapé" if extra else "")
                break
        raise FormatError(f"{path}: {len(data)} bytes; o {what} do cliente tem {size + 4} "
                          f"({count} x {rec.size} + 4 de rodapé){hint}. Outra versão do jogo usa outro "
                          "layout, e converter assim daria lixo.")
    body = xor(data[:size], 0x5A)
    trailer = struct.unpack("<i", data[size:size + 4])[0] if len(data) == size + 4 else None
    return [list(rec.unpack_from(body, i * rec.size)) for i in range(count)], trailer


def apply_extra(items, path, rec=ITEM_FMT):
    """Aplica o ExtraItem.bin como o cliente e o servidor: blocos de 2 + registro, sem XOR."""
    data = Path(path).read_bytes()
    step = 2 + rec.size
    applied = 0
    for off in range(0, len(data) - step + 1, step):
        idx = struct.unpack_from("<h", data, off)[0]
        if 0 < idx < len(items):
            items[idx] = list(rec.unpack_from(data, off + 2))
            applied += 1
    return applied, len(data) % step


def c_atoi(s):
    i, n = 0, len(s)
    while i < n and s[i] in C_SPACE:
        i += 1
    sign = 1
    if i < n and s[i] in b"+-":
        sign = -1 if s[i] == ord("-") else 1
        i += 1
    v = 0
    while i < n and 48 <= s[i] <= 57:
        v = v * 10 + s[i] - 48
        i += 1
    return sign * v


def read_effect_names(path):
    """EffectNameTable como BASE_InitializeEffectName monta: `#define NOME valor`, o primeiro vence."""
    table = [b""] * MAX_EFFECTINDEX
    notes = []
    for line in Path(path).read_bytes().split(b"\n"):
        if line[:1] == b"\r":
            continue
        tok = line.split()
        if not tok:
            continue
        if tok[0] == b"#include":
            raise FormatError(f"{path}: `#include` derruba o servidor (BASE_InitializeEffectName chama a si "
                              "mesma sem fim); junte os arquivos num só")
        if tok[0] != b"#define" or len(tok) < 3:
            continue
        val = c_atoi(tok[2])
        if val < 0 or val >= MAX_EFFECTINDEX:
            notes.append(f"ItemEffect.h: {tok[1].decode('latin-1')} = {val} fora de 0..{MAX_EFFECTINDEX - 1}, ignorado")
            continue
        if table[val]:
            notes.append(f"ItemEffect.h: valor {val} repetido ({tok[1].decode('latin-1')}); vale "
                         f"{table[val].decode('latin-1')}")
            continue
        if len(tok[1]) >= 24:
            notes.append(f"ItemEffect.h: {tok[1].decode('latin-1')} tem 24+ caracteres e o servidor "
                         "não o reconhece (a tabela guarda 24 bytes)")
            continue
        table[val] = tok[1]
    return table, notes


# ---------------------------------------------------------------------------------------------
# Imitação do sscanf da glibc/msvcrt para os formatos dos leitores do servidor

class Scan:
    def __init__(self, s):
        self.s, self.i = s, 0

    def ws(self):
        while self.i < len(self.s) and self.s[self.i] in C_SPACE:
            self.i += 1

    def d(self):
        self.ws()
        j = self.i
        if j < len(self.s) and self.s[j] in b"+-":
            j += 1
        k = j
        while k < len(self.s) and 48 <= self.s[k] <= 57:
            k += 1
        if k == j:
            return None
        v = int(self.s[self.i:k])
        self.i = k
        return v

    def str(self):
        self.ws()
        j = self.i
        while j < len(self.s) and self.s[j] not in C_SPACE:
            j += 1
        if j == self.i:
            return None
        v = self.s[self.i:j]
        self.i = j
        return v

    def lit(self, c):
        if self.i < len(self.s) and self.s[self.i] == c:
            self.i += 1
            return True
        return False


def sscanf(s, fmt):
    """fmt: sequência de 'd', 's' e literais de um byte (como '.'). Para na primeira falha."""
    sc, out = Scan(s), []
    for f in fmt:
        if f == "d":
            v = sc.d()
        elif f == "s":
            v = sc.str()
        else:
            if not sc.lit(ord(f)):
                break
            continue
        if v is None:
            break
        out.append(v)
    return out


def s16(v):
    return (v + 0x8000) % 0x10000 - 0x8000


def s32(v):
    return (v + 0x80000000) % 0x100000000 - 0x80000000


def s8(v):
    return (v + 0x80) % 0x100 - 0x80


def fgets_lines(data):
    """Linhas como o fgets(str, 1024, fp) dos leitores: no máximo 1023 bytes por leitura."""
    out = []
    for raw in data.split(b"\n"):
        while len(raw) > 1022:
            out.append(raw[:1023])
            raw = raw[1023:]
        out.append(raw + b"\n")
    if out and out[-1] == b"\n":
        out.pop()
    return out


def server_read_itemlist(csv_bytes, effects):
    """BASE_ReadItemListFile (sem o Build): devolve {índice: registro} como o servidor monta."""
    items = {}
    for line in fgets_lines(csv_bytes):
        s = line.replace(b",", b" ")
        v = sscanf(s, "dsssddddd" + "sd" * MAX_STATICEFFECT)
        if not v:
            continue
        index = v[0]
        if index >= MAX_ITEMLIST or index < 0:
            continue  # o servidor mostra uma MessageBox (ou escreve fora da tabela, se negativo)
        name = v[1] if len(v) > 1 else b""
        temp1 = v[2] if len(v) > 2 else b""
        temp2 = v[3] if len(v) > 3 else b""
        unique, price, pos, extra, grade = (v[4:9] + [0] * 5)[:5]
        if len(name) >= ITEMNAME_LENGTH - 1:
            name = name[:ITEMNAME_LENGTH - 2]
        rec = items.setdefault(index, [b""] + [0] * (len(ITEM_FIELDS) - 1))
        rec[0] = name
        if temp1[:1] == b".":
            temp1 = b" " + temp1[1:]
        if temp2[:1] == b".":
            temp2 = b" " + temp2[1:]
        mt = (sscanf(temp1, "d.d") + [0, 0])[:2]
        rq = (sscanf(temp2, "d.d.d.d.d") + [0] * 5)[:5]
        rec[1], rec[2] = s16(mt[0]), s16(mt[1])
        rec[3] = 0  # nIndexVisualEffect
        rec[4:9] = [s16(x) for x in rq]
        pairs = v[9:]
        for j in range(MAX_STATICEFFECT):
            nm = pairs[2 * j] if 2 * j < len(pairs) else b""
            if not nm:
                continue
            val = pairs[2 * j + 1] if 2 * j + 1 < len(pairs) else 0
            code = next((k for k in range(MAX_EFFECTINDEX) if effects[k] == nm), MAX_EFFECTINDEX)
            rec[9 + 2 * j] = s16(code)
            rec[10 + 2 * j] = s16(val)
        rec[33] = s32(price)
        rec[34], rec[35], rec[36], rec[37] = s16(unique), s16(pos), s16(extra), s16(grade)
    return items


def server_read_skills(csv_bytes):
    """BASE_InitializeSkill: devolve {índice: registro} como o servidor monta."""
    spells = {}
    for line in fgets_lines(csv_bytes):
        s = line.split(b"\0", 1)[0].replace(b",", b" ")
        v = sscanf(s, "d" + "d" * 12 + "ss" + "d" * 7)
        if not v:
            continue
        index = v[0]
        rec = [0] * len(SPELL_FIELDS)
        nums = v[1:13]
        for i, x in enumerate(nums):
            rec[i] = s32(x)
        rest = v[13:]
        act1 = rest[0] if len(rest) > 0 else b""
        act2 = rest[1] if len(rest) > 1 else b""
        tail = rest[2:]
        for i, x in enumerate(tail):
            rec[28 + i] = s32(x)
        # AffectTime / 4, divisão do C (trunca para zero)
        t = rec[11]
        rec[11] = int(t / 4) if t >= 0 else -((-t) // 4)
        for base, act in ((12, act1), (20, act2)):
            for i, x in enumerate(sscanf(act, "d.d.d.d.d.d")):
                rec[base + i] = s8(x)
        if 0 <= index < MAX_SPELL_LIST:
            spells[index] = rec
    return spells


# ---------------------------------------------------------------------------------------------
# Escrita dos csv

def item_record(raw):
    """Registro desempacotado -> lista comparável (nome até o NUL, Act como signed char)."""
    rec = list(raw)
    rec[0] = rec[0].split(b"\0", 1)[0]
    return rec


def spell_record(raw):
    rec = list(raw[:12])
    rec += [s8(b) for b in raw[12]] + [s8(b) for b in raw[13]]
    rec += list(raw[14:])
    return rec


def is_empty(rec):
    return all(not x for x in rec)


def itemlist_to_csv(items, effects):
    """[(índice, registro)] -> (bytes do csv, avisos)."""
    code_of = {}
    for k in range(MAX_EFFECTINDEX):
        if effects[k] and effects[k] not in code_of:
            code_of[effects[k]] = k
    lines, warn = [], []
    for idx, rec in items:
        if is_empty(rec):
            continue
        name = rec[0]
        if not name:
            warn.append(f"item {idx}: sem nome; o leitor do servidor não aceita linha sem nome, item fora do csv")
            continue
        clean = bytes(b"_"[0] if (b in C_SPACE or b == 44) else b for b in name)
        if clean != name:
            warn.append(f"item {idx}: nome {name.decode('latin-1')!r} com espaço ou vírgula, gravado como "
                        f"{clean.decode('latin-1')}")
        cols = [str(idx).encode(), clean,
                f"{rec[1]}.{rec[2]}".encode(),
                ".".join(str(x) for x in rec[4:9]).encode(),
                str(rec[34]).encode(), str(rec[33]).encode(), str(rec[35]).encode(),
                str(rec[36]).encode(), str(rec[37]).encode()]
        for j in range(MAX_STATICEFFECT):
            code, val = rec[9 + 2 * j], rec[10 + 2 * j]
            if code == 0 and val == 0:
                continue
            nm = effects[code] if 0 <= code < MAX_EFFECTINDEX else b""
            if not nm or code_of.get(nm) != code:
                warn.append(f"item {idx}: efeito {code} (valor {val}) sem nome no ItemEffect.h, fora do csv")
                continue
            cols += [nm, str(val).encode()]
        lines.append(b",".join(cols) + b"\n")
    return b"".join(lines), warn


def act_str(rec, base):
    return ".".join(str(x) for x in rec[base:base + 6]).encode()


def skills_to_csv(spells):
    lines, warn = [], []
    for idx, rec in spells:
        if is_empty(rec):
            continue
        t = rec[11] * 4
        if s32(t) != t:
            warn.append(f"skill {idx}: AffectTime {rec[11]} x 4 não cabe num int")
        cols = [idx] + rec[0:11] + [t]
        line = ",".join(str(x) for x in cols).encode()
        line += b"," + act_str(rec, 12) + b"," + act_str(rec, 20) + b","
        line += ",".join(str(x) for x in rec[28:35]).encode()
        lines.append(line + b"\n")
    return b"".join(lines), warn


def compare(kind, fields, source, parsed, normalize=None):
    """Diferenças entre o binário e o que o servidor leria do csv."""
    diffs = []
    for idx, rec in source:
        want = rec if not normalize else normalize(rec)
        got = parsed.get(idx)
        if got is None:
            if not is_empty(want):
                diffs.append(f"{kind} {idx}: o servidor não leria este registro")
            continue
        for f, a, b in zip(fields, want, got):
            if a != b:
                show = (lambda x: x.decode("latin-1") if isinstance(x, bytes) else x)
                diffs.append(f"{kind} {idx}: {f} {show(a)!r} no binário, o servidor leria {show(b)!r}")
    extra = set(parsed) - {i for i, _ in source}
    diffs += [f"{kind} {i}: o servidor leria um registro a mais" for i in sorted(extra)]
    return diffs


def effect_multiset(rec):
    return sorted((rec[9 + 2 * j], rec[10 + 2 * j]) for j in range(MAX_STATICEFFECT)
                  if rec[9 + 2 * j] or rec[10 + 2 * j])


def compare_items(source, parsed):
    """Como compare(), mas os efeitos valem como conjunto: o csv só guarda os pares não vazios, em
    ordem, e o jogo percorre os 12. A mudança de posição é contada à parte."""
    diffs, moved = [], 0
    nofx = [f for f in ITEM_FIELDS if not f.startswith("stEffect")]
    keep = [i for i, f in enumerate(ITEM_FIELDS) if not f.startswith("stEffect")]
    for idx, rec in source:
        got = parsed.get(idx)
        if got is None:
            if not is_empty(rec):
                diffs.append(f"item {idx}: o servidor não leria este item")
            continue
        for f, i in zip(nofx, keep):
            if rec[i] != got[i]:
                a, b = rec[i], got[i]
                a = a.decode("latin-1") if isinstance(a, bytes) else a
                b = b.decode("latin-1") if isinstance(b, bytes) else b
                diffs.append(f"item {idx}: {f} {a!r} no binário, o servidor leria {b!r}")
        if effect_multiset(rec) != effect_multiset(got):
            diffs.append(f"item {idx}: efeitos {effect_multiset(rec)} no binário, o servidor leria "
                         f"{effect_multiset(got)}")
        elif rec[9:33] != got[9:33]:
            moved += 1
    diffs += [f"item {i}: o servidor leria um item a mais" for i in sorted(set(parsed) - {i for i, _ in source})]
    return diffs, moved


# ---------------------------------------------------------------------------------------------
# Comandos

def convert_itemlist(bin_path, out, extra=None, effects_path=None, quiet=False):
    layout, rec, count = detect_layout(bin_path, [("cliente", ITEM_FMT, MAX_ITEMLIST),
                                                  ("global", G_ITEM_FMT, G_MAX_ITEMLIST)])
    raw, trailer = read_table(bin_path, rec, count, "ItemList.bin")
    log = [] if layout == "cliente" else [
        f"layout do WYD Global ({count} x {rec.size} bytes), convertido para o do servidor ({MAX_ITEMLIST} x {ITEM_FMT.size})"]
    if trailer is None:
        log.append("ItemList.bin sem os 4 bytes de rodapé")
    elif trailer != ITEMLIST_CLIENT_TRAILER:
        log.append(f"rodapé do ItemList.bin = {trailer:#x}; o cliente em Release só aceita {ITEMLIST_CLIENT_TRAILER:#x}")
    if extra:
        applied, rest = apply_extra(raw, extra, rec)
        log.append(f"ExtraItem.bin: {applied} item(ns) aplicado(s) por cima do ItemList"
                   + (f" ({rest} byte(s) no fim ignorados)" if rest else ""))
    effects, notes = read_effect_names(effects_path or ROOT / "server/common/ItemEffect.h")
    log += notes
    if layout == "global":
        lost = {}
        beyond = [i for i in range(MAX_ITEMLIST, count) if any(raw[i][1:]) or raw[i][0].strip(b"\0")]
        if beyond:
            log.append(f"{len(beyond)} item(ns) com índice {MAX_ITEMLIST}+ (o servidor vai até {MAX_ITEMLIST - 1}), "
                       f"fora do csv: {beyond[:10]}{'...' if len(beyond) > 10 else ''}")
        raw = [global_item_to_client(r, lost) for r in raw[:MAX_ITEMLIST]]
        log += [f"do Global, sem lugar no servidor: {k} em {v} item(ns)" for k, v in sorted(lost.items())]
    source = [(i, item_record(r)) for i, r in enumerate(raw)]
    unnamed = {}
    for _, r in source:
        for j in range(MAX_STATICEFFECT):
            code, val = r[9 + 2 * j], r[10 + 2 * j]
            if (code or val) and not (0 <= code < MAX_EFFECTINDEX and effects[code]):
                unnamed[code] = unnamed.get(code, 0) + 1
    if unnamed:
        log.append("efeitos sem nome no ItemEffect.h (código: itens): "
                   + ", ".join(f"{c}: {n}" for c, n in sorted(unnamed.items())))
    csv, warn = itemlist_to_csv(source, effects)
    Path(out).write_bytes(csv)
    parsed = server_read_itemlist(csv, effects)
    # nome: o que vem depois do NUL não conta; nIndexVisualEffect o leitor sempre zera
    diffs, moved = compare_items([(i, r) for i, r in source if not is_empty(r)], parsed)
    n = sum(1 for _, r in source if not is_empty(r))
    if moved:
        log.append(f"{moved} item(ns) com espaço vazio entre efeitos: no csv os efeitos ficam juntos, na mesma ordem")
    return report(f"itemlist: {len(parsed)} de {n} item(ns) gravados em {out}", log, warn, diffs, quiet)


def convert_skills(bin_path, out, quiet=False):
    layout, rec, count = detect_layout(bin_path, [("cliente", SPELL_FMT, MAX_SPELL_LIST),
                                                  ("global", G_SPELL_FMT, MAX_SPELL_LIST)])
    raw, trailer = read_table(bin_path, rec, count, "SkillData.bin")
    log = [] if trailer is not None else ["SkillData.bin sem os 4 bytes de rodapé"]
    if layout == "global":
        lost = {}
        raw = [global_spell_to_client(r, i, lost) for i, r in enumerate(raw)]
        log.append(f"layout do WYD Global ({count} x {rec.size} bytes), convertido para o do servidor "
                   f"({MAX_SPELL_LIST} x {SPELL_FMT.size})")
        log += [f"{k}: {v} skill(s)" for k, v in sorted(lost.items())]
    source = [(i, spell_record(r)) for i, r in enumerate(raw)]
    csv, warn = skills_to_csv(source)
    Path(out).write_bytes(csv)
    parsed = server_read_skills(csv)
    diffs = compare("skill", SPELL_FIELDS, [(i, r) for i, r in source if not is_empty(r)], parsed)
    n = sum(1 for _, r in source if not is_empty(r))
    return report(f"skilldata: {len(parsed)} de {n} skill(s) gravadas em {out}", log, warn, diffs, quiet)


def report(head, log, warn, diffs, quiet):
    if not quiet:
        print(head)
        for line in log + warn[:50]:
            print(f"  {line}")
        if len(warn) > 50:
            print(f"  ... e mais {len(warn) - 50} aviso(s)")
        if diffs:
            print(f"  {len(diffs)} campo(s) que o servidor leria diferente do binário:")
            for d in diffs[:200]:
                print(f"    {d}")
            if len(diffs) > 200:
                print(f"    ... e mais {len(diffs) - 200}")
        print("resultado: " + ("OK (o servidor lê exatamente o binário)" if not diffs else
                               f"DIFERENÇAS ({len(diffs)})"))
    return diffs, warn


# ---------------------------------------------------------------------------------------------
# Autoteste com dados sintéticos

def pack_items(recs, trailer=ITEMLIST_CLIENT_TRAILER):
    body = b"".join(ITEM_FMT.pack(*r) for r in recs)
    return xor(body, 0x5A) + struct.pack("<i", trailer)


def pack_spells(recs):
    body = b"".join(SPELL_FMT.pack(*r) for r in recs)
    return xor(body, 0x5A) + struct.pack("<i", 12345)


def selftest():
    rnd = random.Random(7)
    effects, _ = read_effect_names(ROOT / "server/common/ItemEffect.h")
    named = [k for k in range(1, MAX_EFFECTINDEX) if effects[k]]
    problems = []
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)

        # 1. Itens que o csv representa: tem que voltar igual, campo a campo.
        recs = [[b""] + [0] * 37 for _ in range(MAX_ITEMLIST)]
        for idx in rnd.sample(range(MAX_ITEMLIST), 400):
            name = bytes(rnd.choice(b"ABCxyz_019\xe7\xe3") for _ in range(rnd.randint(1, 62)))
            r = [name + b"\0" * (64 - len(name))]
            r += [rnd.randint(-32768, 32767), rnd.randint(-32768, 32767), 0]
            r += [rnd.randint(-32768, 32767) for _ in range(5)]
            nfx = rnd.randint(0, MAX_STATICEFFECT)
            for j in range(MAX_STATICEFFECT):
                r += [rnd.choice(named), rnd.randint(-32768, 32767)] if j < nfx else [0, 0]
            r += [rnd.randint(-2 ** 31, 2 ** 31 - 1)] + [rnd.randint(-32768, 32767) for _ in range(4)]
            recs[idx] = r
        (tmp / "ItemList.bin").write_bytes(pack_items(recs))
        diffs, warn = convert_itemlist(tmp / "ItemList.bin", tmp / "itemlist.csv", quiet=True)
        if diffs or warn:
            problems.append(f"itens representáveis: {len(diffs)} diferença(s), {len(warn)} aviso(s): "
                            f"{(diffs + warn)[:3]}")

        # 2. ExtraItem.bin por cima, sem XOR.
        extra = struct.pack("<h", 5) + ITEM_FMT.pack(b"Extra" + b"\0" * 59, *([1] * 2 + [0] + [2] * 5 +
                                                                              [0] * 24 + [77] + [3] * 4))
        (tmp / "ExtraItem.bin").write_bytes(extra + struct.pack("<h", 0) + b"\0" * 140 + b"xx")
        convert_itemlist(tmp / "ItemList.bin", tmp / "itemlist.csv", extra=tmp / "ExtraItem.bin", quiet=True)
        line = [l for l in (tmp / "itemlist.csv").read_bytes().split(b"\n") if l.startswith(b"5,")]
        if line != [b"5,Extra,1.1,2.2.2.2.2,3,77,3,3,3"]:
            problems.append(f"ExtraItem.bin: item 5 saiu {line}")

        # 3. O que o csv não representa tem que aparecer como diferença ou aviso.
        bad = [[b""] + [0] * 37 for _ in range(MAX_ITEMLIST)]
        bad[10] = [b"Com espaco" + b"\0" * 54, 1, 0, 0] + [0] * 5 + [0] * 24 + [5, 0, 0, 0, 0]
        bad[11] = [b"Visual" + b"\0" * 58, 1, 0, 9] + [0] * 5 + [0] * 24 + [5, 0, 0, 0, 0]
        bad[12] = [b"SemNomeFx" + b"\0" * 55, 1, 0, 0] + [0] * 5 + [0, 7] + [0] * 22 + [5, 0, 0, 0, 0]
        bad[13] = [b"\0" * 64, 1, 0, 0] + [0] * 5 + [0] * 24 + [5, 0, 0, 0, 0]
        bad[14] = [b"N" * 63 + b"\0", 1, 0, 0] + [0] * 5 + [0] * 24 + [5, 0, 0, 0, 0]
        (tmp / "bad.bin").write_bytes(pack_items(bad, trailer=1))
        diffs, warn = convert_itemlist(tmp / "bad.bin", tmp / "bad.csv", extra=None, quiet=True)
        for idx, what in ((10, "Name"), (11, "nIndexVisualEffect"), (12, "efeitos"), (13, "não leria"),
                          (14, "Name")):
            if not any(d.startswith(f"item {idx}:") and what in d for d in diffs):
                problems.append(f"item {idx}: esperava a diferença em {what}, veio {[d for d in diffs if f'item {idx}:' in d]}")

        # 4. Skills: todos os campos do csv voltam; ForceDamage e Act[6..7] aparecem como diferença.
        spells = [[0] * 12 + [b"\0" * 8, b"\0" * 8] + [0] * 8 for _ in range(MAX_SPELL_LIST)]
        for idx in rnd.sample(range(MAX_SPELL_LIST), 120):
            r = [rnd.randint(-2 ** 29, 2 ** 29) for _ in range(12)]
            r += [bytes(rnd.randint(0, 255) for _ in range(6)) + b"\0\0" for _ in range(2)]
            r += [rnd.randint(-2 ** 31, 2 ** 31 - 1) for _ in range(7)] + [0]
            spells[idx] = r
        (tmp / "SkillData.bin").write_bytes(pack_spells(spells))
        diffs, warn = convert_skills(tmp / "SkillData.bin", tmp / "SkillData.csv", quiet=True)
        if diffs or warn:
            problems.append(f"skills representáveis: {len(diffs)} diferença(s): {(diffs + warn)[:3]}")
        spells[3] = [1] * 12 + [b"\1" * 8, b"\0" * 8] + [0] * 7 + [50]
        (tmp / "SkillData.bin").write_bytes(pack_spells(spells))
        diffs, _ = convert_skills(tmp / "SkillData.bin", tmp / "SkillData.csv", quiet=True)
        want = {"Act1[6]", "Act1[7]", "ForceDamage"}
        got = {d.split(": ")[1].split(" ")[0] for d in diffs if d.startswith("skill 3:")}
        if got != want:
            problems.append(f"skill 3: esperava diferenças em {sorted(want)}, veio {sorted(got)}")

        # 5. Layout do WYD Global: 14500 x 172 e 248 x 104, levados ao do servidor.
        g = [[b"\0" * 64] + [0] * 8 + [G_EMPTY_EFFECT, 0] * 12 + [0] * 19 for _ in range(G_MAX_ITEMLIST)]
        fx = [named[0], 7] + [G_EMPTY_EFFECT, 0] * 11
        g[20] = [b"Global" + b"\0" * 58, 3, 1, 0, 10, 0, 0, 0, 0] + fx + [500, 41, 4, 9, 2] + [0] * 14
        g[21] = [b"Bit16" + b"\0" * 59, 3, 0, 0, 0, 0, 0, 0, 0] + [G_EMPTY_EFFECT, 0] * 12 + [1, 0, 0x10002, 0, 0] + [5] + [0] * 13
        g[7000] = [b"Alto" + b"\0" * 60, 1] + [0] * 7 + [G_EMPTY_EFFECT, 0] * 12 + [1, 0, 0, 0, 0] + [0] * 14
        body = b"".join(G_ITEM_FMT.pack(*r) for r in g)
        (tmp / "G_ItemList.bin").write_bytes(xor(body, 0x5A) + struct.pack("<i", 0x4823))
        diffs, _ = convert_itemlist(tmp / "G_ItemList.bin", tmp / "g.csv", quiet=True)
        lines = (tmp / "g.csv").read_bytes().split(b"\n")
        want = [b"20,Global,3.1,10.0.0.0.0,41,500,4,9,2," + effects[named[0]] + b",7", b"21,Bit16,3.0,0.0.0.0.0,0,1,2,0,0", b""]
        if lines != want or diffs:
            problems.append(f"layout do Global: csv {lines}, diferenças {diffs[:3]}")
        gs = [[0] * 12 + [b"\0" * 8, b"\0" * 8] + [0] * 8 + [i + 1, 0] for i in range(MAX_SPELL_LIST)]
        gs[4] = [1] * 12 + [b"\1" * 6 + b"\0\0", b"\2" * 6 + b"\0\0"] + [3] * 7 + [0, 5, 0]
        body = b"".join(G_SPELL_FMT.pack(*r) for r in gs)
        (tmp / "G_SkillData.bin").write_bytes(xor(body, 0x5A) + struct.pack("<i", 0x29))
        diffs, _ = convert_skills(tmp / "G_SkillData.bin", tmp / "gs.csv", quiet=True)
        lines = (tmp / "gs.csv").read_bytes().split(b"\n")
        if diffs or lines[0] != b"4,1,1,1,1,1,1,1,1,1,1,1,4,1.1.1.1.1.1,2.2.2.2.2.2,3,3,3,3,3,3,3":
            problems.append(f"skill do Global: {lines[:1]}, diferenças {diffs[:3]}")

        # 6. Tamanho de outra versão: erro claro, sem csv.
        (tmp / "outro.bin").write_bytes(b"\0" * (7000 * 140 + 4))
        try:
            convert_itemlist(tmp / "outro.bin", tmp / "outro.csv", quiet=True)
            problems.append("ItemList.bin de 7000 itens foi aceito")
        except FormatError as e:
            if "7000 registros" not in str(e):
                problems.append(f"erro do tamanho errado sem a dica: {e}")

    for p in problems:
        print(f"::error::{p}")
    print("autoteste: " + ("OK" if not problems else f"FALHOU ({len(problems)})"))
    return 1 if problems else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    a = sub.add_parser("itemlist", help="ItemList.bin (+ ExtraItem.bin) -> itemlist.csv")
    a.add_argument("bin")
    a.add_argument("-o", "--output", required=True)
    g = a.add_mutually_exclusive_group()
    g.add_argument("--extra", help="ExtraItem.bin (padrão: o que estiver ao lado do ItemList.bin)")
    g.add_argument("--no-extra", action="store_true", help="não aplica o ExtraItem.bin")
    a.add_argument("--effects", help="ItemEffect.h (padrão: server/common/ItemEffect.h)")
    s = sub.add_parser("skilldata", help="SkillData.bin -> SkillData.csv")
    s.add_argument("bin")
    s.add_argument("-o", "--output", required=True)
    sub.add_parser("selftest", help="confere o conversor com dados sintéticos")
    args = ap.parse_args()

    try:
        if args.cmd == "selftest":
            return selftest()
        if not Path(args.bin).is_file():
            raise FormatError(f"{args.bin}: arquivo não encontrado")
        if args.cmd == "itemlist":
            extra = None
            if not args.no_extra:
                extra = args.extra or next((p for p in Path(args.bin).resolve().parent.iterdir()
                                            if p.name.lower() == "extraitem.bin"), None)
            diffs, _ = convert_itemlist(args.bin, args.output, extra, args.effects)
        else:
            diffs, _ = convert_skills(args.bin, args.output)
    except (FormatError, OSError) as e:
        print(f"erro: {e}", file=sys.stderr)
        return 2
    return 1 if diffs else 0


if __name__ == "__main__":
    sys.exit(main())
