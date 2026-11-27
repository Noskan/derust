#!/usr/bin/env python3
"""Monta, a partir de uma instalação do WYD Global, os arquivos que o nosso cliente (TMProject) lê
de outro jeito.

Modelo híbrido (ver docs/DADOS_GLOBAL.md): o protocolo e as tabelas que o servidor também usa
(ItemList, SkillData) seguem o 7662. Os recursos visuais do Global (listas de textura de 528
bytes, MeshList até 3148, object.bin com 4096 máscaras) o cliente lê direto e não passam por aqui.

O resultado é uma camada: só os arquivos que mudam, nos caminhos relativos em que o cliente os
procura. Copie a instalação do Global para uma pasta própria e copie a camada por cima. A
instalação original não é alterada.

  ItemList.bin, SkillData.bin  layout do Global (14500 x 172 e 248 x 104) levado ao 7662 (6500 x 140
                               e 248 x 96), com XOR 0x5A e o rodapé que o cliente em Release aceita.
                               O extraitem.bin do Global é aplicado antes, como o cliente do Global
                               faz; nenhum ExtraItem.bin é gravado (o do Global tem registros de 174
                               bytes). Leitores e conversão: tools/w2_bin_csv.py.
  Env/AttributeMap.dat         o do Global (1 MB) com a soma BASE_GetSum no fim, que o cliente confere.
  UI/strdef.bin                o Lang/PT/strdef.bin (texto puro) com XOR 0x5A e soma BASE_GetSum dos
                               bytes cifrados (BASE_ReadMessageBin). O UI/strdef.bin do Global é coreano.
  UI/*.txt, Mixhelp.dat        os textos de Lang/PT/, nos caminhos em que o cliente os abre.
  cdata.bin                    TMGround::m_nCheckSum[64][32] recalculado dos Field*.trn e Field*.dat do
                               Env (o do Global confere 99 de 120 .dat e nenhum .trn). Entradas sem
                               arquivo ficam como no cdata.bin do Global.
  sn.bin, serverlist.bin       um grupo de servidor real com um canal no IP dado e o grupo virtual
                               "recomendado", com a URL da página de status do DBSrv.

Uso:
  python3 tools/global_client_data.py build <instalação do Global> <saída> --status-url <url> --ip <ip>
                                              [--name Global] [--recommended Recomend]
  python3 tools/global_client_data.py selftest
"""

import argparse
import os
import re
import shutil
import struct
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import w2_bin_csv as W  # noqa: E402

REPO = Path(__file__).resolve().parent.parent
BASEDEF_CPP = REPO / "client" / "src" / "common" / "Basedef.cpp"

ATTRIBUTE_MAP_SIZE = 1024 * 1024
STRDEF_SIZE = 2000 * 128
CDATA_ENTRIES = 64 * 32
TILE_BYTES = 49152
MAX_GROUND_OBJECTS = 4096
SN_GROUPS = 11
SN_NAME = 9
SERVERLIST_GROUPS = 10
SERVERLIST_SERVERS = 11
SERVERLIST_FIELD = 64


def i32(v):
    return (v + 2**31) % 2**32 - 2**31


def s8(b):
    return b - 256 if b > 127 else b


def tdiv(a, b):
    """Divisão inteira truncada para zero, como em C."""
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q


def base_getsum(data):
    """BASE_GetSum do cliente (client/src/common/Basedef.cpp), com char com sinal.

    O `else` pertence só ao `if (mod == 5)`: todo byte fora da posição 5 soma também p / 3."""
    s = 0
    for i, u in enumerate(data):
        p = s8(u)
        m = i % 7
        if m == 0:
            s += tdiv(p, 2)
        if m == 1:
            s += p ^ 0xFF
        if m == 2:
            s += 3 * p
        if m == 3:
            s += 2 * p
        if m == 4:
            s -= tdiv(p, 7)
        if m == 5:
            s -= p
        else:
            s += tdiv(p, 3)
    return i32(s)


def attribute_map_with_sum(data):
    if len(data) != ATTRIBUTE_MAP_SIZE:
        raise ValueError(f"AttributeMap.dat com {len(data)} bytes; o esperado é {ATTRIBUTE_MAP_SIZE}")
    return data + struct.pack("<i", base_getsum(data))


def strdef_for_client(plain):
    if len(plain) != STRDEF_SIZE:
        raise ValueError(f"strdef.bin com {len(plain)} bytes; o esperado é {STRDEF_SIZE} (2000 x 128)")
    enc = W.xor(plain, 0x5A)
    return enc + struct.pack("<i", base_getsum(enc))


def ground_object_sum(data):
    """Soma de um Field*.dat como TMObjectContainer::Load: tipo + (int)x + (int)y + (int)altura +
    máscara de cada objeto. Registros de 28 bytes, mais 8 nos tipos 501-506, 511-518 e 520-599."""
    s = 0
    pos = 0
    for _ in range(MAX_GROUND_OBJECTS):
        if pos + 28 > len(data):
            break
        typ, fx, fy, fh, _angle, _tex, mask = struct.unpack_from("<I4f2i", data, pos)
        pos += 28
        s += typ + int(fx) + int(fy) + int(fh) + mask
        if 501 <= typ <= 506 or 511 <= typ <= 518 or 520 <= typ <= 599:
            pos += 8
    return s


def tile_map_sum(data):
    """Soma de um Field*.trn como TMGround::LoadTileMap. Devolve (x, y, soma) com x e y do arquivo."""
    n = min(data[0], 128)
    if len(data) < 3 + n + TILE_BYTES:
        raise ValueError(f"mapa com {len(data)} bytes, curto demais")
    px, py = data[1 + n], data[2 + n]
    tiles = data[3 + n:3 + n + TILE_BYTES]
    return px, py, i32(sum(s8(b) for b in tiles) + px * py)


def make_cdata(env_dir, base):
    """Recalcula o cdata.bin a partir dos mapas de `env_dir`. Devolve (bytes, relatório)."""
    if len(base) < CDATA_ENTRIES * 4:
        raise ValueError(f"cdata.bin base com {len(base)} bytes; o esperado é {CDATA_ENTRIES * 4}")
    table = list(struct.unpack_from(f"<{CDATA_ENTRIES}i", base))
    stats = {"trn": [0, 0], "dat": [0, 0]}
    for name in sorted(os.listdir(env_dir)):
        m = re.fullmatch(r"field(\d\d)(\d\d)\.(trn|dat)", name, re.I)
        if not m:
            continue
        x, y, ext = int(m.group(1)), int(m.group(2)), m.group(3).lower()
        data = (Path(env_dir) / name).read_bytes()
        if ext == "trn":
            px, py, s = tile_map_sum(data)
            idx = py * 32 + px
        else:
            s = i32(ground_object_sum(data) + x * y)
            idx = (32 + y) * 32 + x
        if not 0 <= idx < CDATA_ENTRIES or (ext == "trn" and idx >= 32 * 32):
            continue
        stats[ext][table[idx] == s] += 1
        table[idx] = s
    return struct.pack(f"<{CDATA_ENTRIES}i", *table), stats


def make_sn(real_name, recommended_name):
    """sn.bin lido em NewApp: g_szServerNameList[11][9] e g_nServerCountList[11] (int).

    TMSelectServerScene::InitializeUI mostra o nome i + 1 para a entrada i, e a entrada com a
    maior contagem vira o grupo virtual "recomendado" (m_nAdmitGroup). O grupo real 0 tem
    contagem 1 e o recomendado, 2, como no sn.bin do Global."""
    real = real_name.encode("latin-1")[:SN_NAME - 1]
    recom = recommended_name.encode("latin-1")[:SN_NAME - 1]
    names = [real, real, recom] + [b""] * (SN_GROUPS - 3)
    counts = [1, 2] + [0] * (SN_GROUPS - 2)
    return b"".join(n.ljust(SN_NAME, b"\0") for n in names) + struct.pack(f"<{SN_GROUPS}i", *counts)


def serverlist_key(basedef_path=BASEDEF_CPP):
    """Chave de 64 bytes de BASE_InitializeServerList (`char szList[65]` em Basedef.cpp)."""
    src = Path(basedef_path).read_bytes()
    m = re.search(rb'char szList\[65\] = \{ "(.*?)"\}', src)
    if not m:
        raise ValueError(f"szList não encontrado em {basedef_path}")
    key = m.group(1).decode("utf-8").encode("latin-1")
    if len(key) != SERVERLIST_FIELD:
        raise ValueError(f"chave do serverlist com {len(key)} bytes; o esperado é {SERVERLIST_FIELD}")
    return key


def make_serverlist(url, ip, key):
    """serverlist.bin: 10 grupos x 11 campos de 64 bytes, cada byte somado a key[63 - i].

    Grupo 0: URL da página de status e um canal no IP dado. Grupo 1 (o "recomendado" do sn.bin):
    só a URL, porque o cliente preenche os canais dele."""
    rows = [[b""] * SERVERLIST_SERVERS for _ in range(SERVERLIST_GROUPS)]
    rows[0][0] = url.encode("latin-1")
    rows[0][1] = ip.encode("latin-1")
    rows[1][0] = url.encode("latin-1")
    out = bytearray()
    for row in rows:
        for field in row:
            if len(field) >= SERVERLIST_FIELD:
                raise ValueError(f"campo do serverlist longo demais: {field!r}")
            plain = field.ljust(SERVERLIST_FIELD, b"\0")
            out += bytes((plain[i] + key[63 - i]) & 0xFF for i in range(SERVERLIST_FIELD))
    return bytes(out)


def decode_serverlist(data, key):
    fields = []
    for k in range(0, len(data), SERVERLIST_FIELD):
        chunk = data[k:k + SERVERLIST_FIELD]
        fields.append(bytes((chunk[i] - key[63 - i]) & 0xFF for i in range(SERVERLIST_FIELD)).split(b"\0")[0])
    return [fields[g * SERVERLIST_SERVERS:(g + 1) * SERVERLIST_SERVERS] for g in range(SERVERLIST_GROUPS)]


def find_ci(folder, name):
    """Arquivo `name` em `folder` sem diferenciar maiúsculas (a instalação vem do Windows)."""
    folder = Path(folder)
    if not folder.is_dir():
        return None
    return next((p for p in folder.iterdir() if p.name.lower() == name.lower()), None)


def convert_tables(src, dst):
    items, trailer = W.read_table(src / "ItemList.bin", W.G_ITEM_FMT, W.G_MAX_ITEMLIST, "ItemList.bin")
    extra = find_ci(src, "extraitem.bin")
    applied = 0
    if extra:
        applied, _rest = W.apply_extra(items, extra, rec=W.G_ITEM_FMT)
    lost = {}
    beyond = sum(1 for r in items[W.MAX_ITEMLIST:] if r[0].rstrip(b"\0"))
    out = [W.global_item_to_client(r, lost) for r in items[:W.MAX_ITEMLIST]]
    body = b"".join(W.ITEM_FMT.pack(*r) for r in out)
    (dst / "ItemList.bin").write_bytes(W.xor(body, 0x5A) + struct.pack("<i", W.ITEMLIST_CLIENT_TRAILER))
    print(f"ItemList.bin: {len(out)} itens; extraitem aplicado em {applied}; {beyond} itens com nome de "
          f"{W.MAX_ITEMLIST} em diante ficaram de fora; rodapé original {trailer:#x}")
    for k, v in sorted(lost.items()):
        print(f"  perdido: {k} em {v} item(ns)")

    spells, strailer = W.read_table(src / "SkillData.bin", W.G_SPELL_FMT, W.MAX_SPELL_LIST, "SkillData.bin")
    slost = {}
    sout = [W.global_spell_to_client(r, i, slost) for i, r in enumerate(spells)]
    sbody = b"".join(W.SPELL_FMT.pack(*r) for r in sout)
    (dst / "SkillData.bin").write_bytes(W.xor(sbody, 0x5A) + struct.pack("<i", strailer or 0))
    print(f"SkillData.bin: {len(sout)} skills")
    for k, v in sorted(slost.items()):
        print(f"  perdido: {k} em {v} skill(s)")


def build(src, dst, status_url, ip, name, recommended):
    src, dst = Path(src), Path(dst)
    if not (src / "ItemList.bin").is_file():
        raise ValueError(f"{src} não parece uma instalação do WYD Global (falta ItemList.bin)")
    if dst.exists() and any(dst.iterdir()):
        raise ValueError(f"{dst} já existe e não está vazia")
    if dst.resolve().is_relative_to(src.resolve()):
        raise ValueError("a saída não pode ficar dentro da instalação do Global")
    (dst / "UI").mkdir(parents=True, exist_ok=True)
    (dst / "Env").mkdir(exist_ok=True)

    convert_tables(src, dst)

    env = find_ci(src, "Env")
    attr = find_ci(env, "AttributeMap.dat")
    (dst / "Env" / "AttributeMap.dat").write_bytes(attribute_map_with_sum(attr.read_bytes()))
    print("Env/AttributeMap.dat: soma acrescentada")

    pt = find_ci(find_ci(src, "Lang"), "PT")
    (dst / "UI" / "strdef.bin").write_bytes(strdef_for_client(find_ci(pt, "strdef.bin").read_bytes()))
    texts = sorted(p for p in pt.iterdir() if p.suffix.lower() == ".txt")
    for p in texts:
        shutil.copyfile(p, dst / "UI" / p.name)
    mixhelp = find_ci(pt, "Mixhelp.dat")
    if mixhelp:
        shutil.copyfile(mixhelp, dst / "Mixhelp.dat")
    print(f"UI/strdef.bin e {len(texts)} textos de Lang/PT")

    base = find_ci(src, "cdata.bin")
    cdata, stats = make_cdata(env, base.read_bytes())
    (dst / "cdata.bin").write_bytes(cdata)
    print(f"cdata.bin: entradas que o do Global já tinha certas / recalculadas: "
          f".trn {stats['trn'][1]}/{sum(stats['trn'])}, .dat {stats['dat'][1]}/{sum(stats['dat'])}")

    (dst / "sn.bin").write_bytes(make_sn(name, recommended))
    (dst / "serverlist.bin").write_bytes(make_serverlist(status_url, ip, serverlist_key()))
    print(f"sn.bin e serverlist.bin: grupo {name!r}, canal 1 em {ip}, status em {status_url}")
    return 0


def selftest():
    failures = []

    def check(ok, what):
        if not ok:
            failures.append(what)

    # BASE_GetSum: valores calculados à mão a partir do código C.
    check(base_getsum(b"\x02") == 1, "getsum [2]")                     # 2/2 + 2/3
    check(base_getsum(b"\x00\x00") == 255, "getsum [0,0]")              # 0 ^ 0xFF na posição 1
    check(base_getsum(b"\x00\x00\x00\x00\x00\x07") == 248, "getsum posição 5 sem o /3")
    check(base_getsum(b"\xf7") == -7, "getsum com sinal e divisão truncada")  # -9/2 + -9/3
    check(base_getsum(b"\x00" * 9) == 510, "getsum volta à posição 1 depois de 7")

    plain = bytes((i * 7) & 0xFF for i in range(STRDEF_SIZE))
    out = strdef_for_client(plain)
    check(len(out) == STRDEF_SIZE + 4, "strdef tamanho")
    check(W.xor(out[:-4], 0x5A) == plain, "strdef XOR")
    check(struct.unpack("<i", out[-4:])[0] == base_getsum(out[:-4]), "strdef soma dos bytes cifrados")

    attr = bytes(i & 0x7F for i in range(ATTRIBUTE_MAP_SIZE))
    out = attribute_map_with_sum(attr)
    check(out[:-4] == attr and struct.unpack("<i", out[-4:])[0] == base_getsum(attr), "AttributeMap soma")
    try:
        attribute_map_with_sum(attr[:-1])
        check(False, "AttributeMap curto aceito")
    except ValueError:
        pass

    with tempfile.TemporaryDirectory() as tmp:
        env = Path(tmp)
        name = b"abc"
        tiles = b"\x01" * (TILE_BYTES - 1) + b"\xff"
        (env / "field0205.trn").write_bytes(bytes([len(name)]) + name + bytes([2, 5]) + tiles)
        rec1 = struct.pack("<I4f2i", 501, 10.7, 20.2, -1.5, 0.0, 0, 3) + b"\x55" * 8
        rec2 = struct.pack("<I4f2i", 7, 1.0, 2.0, 3.0, 0.0, 0, 4)
        (env / "Field0304.DAT").write_bytes(rec1 + rec2)
        (env / "outro.trn").write_bytes(b"ignorado")
        base = struct.pack(f"<{CDATA_ENTRIES}i", *range(CDATA_ENTRIES))
        cdata, stats = make_cdata(env, base)
        table = struct.unpack(f"<{CDATA_ENTRIES}i", cdata)
        check(table[5 * 32 + 2] == 49150 + 10, "cdata .trn: soma com sinal + x*y")
        # 501 + 10 + 20 + int(-1.5) + 3, com 8 bytes extras, e 7 + 1 + 2 + 3 + 4, mais 3*4.
        check(table[(32 + 4) * 32 + 3] == 533 + 17 + 12, "cdata .dat: registro estendido e truncamento")
        check(table[0] == 0 and table[CDATA_ENTRIES - 1] == CDATA_ENTRIES - 1, "cdata mantém a base")
        check(stats == {"trn": [1, 0], "dat": [1, 0]}, "cdata relatório")

    sn = make_sn("Global", "Recomendado")
    check(len(sn) == SN_GROUPS * SN_NAME + SN_GROUPS * 4, "sn tamanho")
    check(sn[:9] == b"Global\0\0\0" and sn[18:27] == b"Recomend\0", "sn nomes (8 caracteres)")
    check(struct.unpack_from(f"<{SN_GROUPS}i", sn, SN_GROUPS * SN_NAME)[:3] == (1, 2, 0), "sn contagens")

    key = bytes((i * 37 + 11) & 0xFF for i in range(SERVERLIST_FIELD))
    data = make_serverlist("http://10.0.0.1/serv00.htm", "10.0.0.2", key)
    check(len(data) == SERVERLIST_GROUPS * SERVERLIST_SERVERS * SERVERLIST_FIELD, "serverlist tamanho")
    rows = decode_serverlist(data, key)
    check(rows[0][0] == b"http://10.0.0.1/serv00.htm" and rows[0][1] == b"10.0.0.2", "serverlist grupo 0")
    check(rows[1][0] == b"http://10.0.0.1/serv00.htm" and rows[1][1] == b"", "serverlist grupo recomendado")
    check(all(f == b"" for g in rows[2:] for f in g), "serverlist resto vazio")
    check(len(serverlist_key()) == SERVERLIST_FIELD, "chave do serverlist lida de Basedef.cpp")

    for f in failures:
        print(f"FALHA {f}")
    if failures:
        return 1
    print("global_client_data: selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build", help="grava a camada de arquivos convertidos")
    b.add_argument("src", help="instalação do WYD Global (só leitura)")
    b.add_argument("dst", help="pasta de saída, nova ou vazia")
    b.add_argument("--status-url", required=True, help="URL da página de status do DBSrv (serv00.htm)")
    b.add_argument("--ip", required=True, help="IP do TMSrv (canal 1)")
    b.add_argument("--name", default="Global", help="nome do grupo de servidor (até 8 caracteres)")
    b.add_argument("--recommended", default="Recomend", help="nome do grupo recomendado (até 8 caracteres)")
    sub.add_parser("selftest", help="confere as conversões com dados sintéticos")
    args = ap.parse_args()
    try:
        if args.cmd == "selftest":
            return selftest()
        return build(args.src, args.dst, args.status_url, args.ip, args.name, args.recommended)
    except (ValueError, OSError) as e:
        print(f"erro: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
