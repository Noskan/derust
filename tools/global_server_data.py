#!/usr/bin/env python3
"""Monta a árvore de dados do servidor Linux a partir de uma instalação W2PP e do WYD Global.

Combina as definições de mundo de uma distribuição W2PP (BaseMob, BaseSummon, npc, NPCGener.txt,
Settings, gameconfig.txt, Language.txt, LevelItem.txt, InitItem.csv) com as tabelas e mapas do
WYD Global (ItemList.bin, SkillData.bin, HeightMap.dat, AttributeMap.dat, Lang/PT/strdef.bin).

Não lê nem copia contas (account/), personagens (char/), listas administrativas (Admin.txt,
Mac.txt), logs de terceiros nem binários (.exe/.dll) da origem; apenas os arquivos estáticos de
dados do jogo indicados explicitamente.

Uso:
  python3 tools/global_server_data.py build <servidor_w2pp> <wyd_global> <destino>
                                            --ip <ip> [--extra-ip <ip> ...]
                                            [--bin <dir> | --no-binaries] [--user <usuário>]
  python3 tools/global_server_data.py selftest
"""

import argparse
import getpass
import os
import re
import shutil
import struct
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import global_client_data as GC  # noqa: E402
import linux_deploy as LD        # noqa: E402
import w2_bin_csv as W           # noqa: E402

ROOT = Path(__file__).resolve().parent.parent

SIZEOF_MOB_816 = 816
SIZEOF_MOB_1024 = 1024
SIZEOF_MOB_1296 = 1296
SIZEOF_SCORE_7662 = 48

BASE_CLASSES = ("TK", "FM", "BM", "HT")
SETTINGS_FILES = ("SancRate.txt", "QuestsRate.txt", "CompRate.txt", "MobMerc.txt", "CastleQuest.txt")
MAX_SERVERGROUP = 10
HEIGHTMAP_SIZE = 4096 * 4096
ATTRIBUTE_MAP_SIZE = 1024 * 1024


def normalize_lf_bytes(raw):
    """Remove BOM UTF-8 e normaliza CRLF/CR para LF (leitores Linux comparam \\n)."""
    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    return raw.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def copy_text_lf(src, dst):
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(normalize_lf_bytes(src.read_bytes()))


def require_ci(base, rel, what):
    p = LD.find_case_insensitive(base, rel)
    if p is None:
        raise ValueError(f"{what} não encontrado em {base} ({rel})")
    return p


def load_valid_global_items(global_dir):
    """Retorna os índices 1..6499 preenchidos no ItemList do Global após aplicar extraitem.bin."""
    itemlist_path = require_ci(global_dir, "ItemList.bin", "ItemList.bin")
    items, _ = W.read_table(itemlist_path, W.G_ITEM_FMT, W.G_MAX_ITEMLIST, "ItemList.bin")
    extra = GC.find_ci(global_dir, "extraitem.bin")
    if extra:
        W.apply_extra(items, extra, rec=W.G_ITEM_FMT)
    valid = set()
    for idx in range(1, W.MAX_ITEMLIST):
        rec = items[idx]
        if rec[0].rstrip(b"\0") or rec[1] != 0:
            valid.add(idx)
    return valid


def convert_mob_to_1024(raw, label, valid_items, preserve_face, removed_items=None):
    """Converte STRUCT_MOB legado (816B ou 1296B com cauda zerada) para 1024B e saneia itens."""
    if len(raw) == SIZEOF_MOB_1024:
        out = bytearray(raw)
    else:
        if len(raw) == SIZEOF_MOB_1296:
            if raw[SIZEOF_MOB_816:] != b"\x00" * (SIZEOF_MOB_1296 - SIZEOF_MOB_816):
                raise ValueError(f"{label}: arquivo de 1296B tem dados além do byte 816")
            b = raw[:SIZEOF_MOB_816]
        elif len(raw) == SIZEOF_MOB_816:
            b = raw
        else:
            raise ValueError(f"{label}: tamanho inesperado {len(raw)} bytes (esperado 816, 1024 ou 1296)")

        out = bytearray(SIZEOF_MOB_1024)
        out[0:16] = b[0:16]     # MobName
        out[16] = b[16]         # Clan
        out[17] = b[17]         # Merchant
        out[18:20] = b[18:20]   # Guild
        out[20] = b[20]         # Class

        rsv_old = struct.unpack_from("<H", b, 22)[0]
        out[21] = rsv_old & 0xFF
        struct.pack_into("<H", out, 22, b[24])  # Quest (uint8 -> uint16)

        coin = struct.unpack_from("<i", b, 28)[0]
        struct.pack_into("<i", out, 24, coin)

        out[32:40] = b[32:40]   # Exp
        out[40:44] = b[40:44]   # HomeTownX, HomeTownY

        # BaseScore (44) e CurrentScore (92): int32 Level -> int16 Level; limpa padding 0xCC do MSVC.
        for score_off in (44, 92):
            lvl = struct.unpack_from("<i", b, score_off)[0]
            lvl = max(-32768, min(32767, lvl))
            struct.pack_into("<h", out, score_off, lvl)
            out[score_off + 4:score_off + 14] = b[score_off + 4:score_off + 14]
            out[score_off + 14] = 0 if b[score_off + 14] == 0xCC else b[score_off + 14]
            out[score_off + 15] = 0 if b[score_off + 15] == 0xCC else b[score_off + 15]
            out[score_off + 16:score_off + SIZEOF_SCORE_7662] = b[score_off + 16:score_off + SIZEOF_SCORE_7662]

        out[140:268] = b[140:268]   # Equip[16]
        out[268:780] = b[268:780]   # Carry[64]

        out[780:784] = b[780:784]   # LearnedSkill[0]
        magic_old = struct.unpack_from("<I", b, 784)[0]
        out[788:801] = b[788:801]   # ScoreBonus..GuildLevel
        out[801] = min(255, magic_old)
        out[802] = min(255, struct.unpack_from("<H", b, 802)[0])  # RegenHP
        out[803] = min(255, struct.unpack_from("<H", b, 804)[0])  # RegenMP
        out[804:808] = b[806:810]   # Resist[4]

    start_slot = 1 if preserve_face else 0
    for slot in range(start_slot, 80):
        off = 140 + slot * 8
        sidx = struct.unpack_from("<h", out, off)[0]
        if sidx != 0 and sidx not in valid_items:
            out[off:off + 8] = b"\x00" * 8
            if removed_items is not None:
                removed_items.setdefault(sidx, []).append((label, slot))

    return bytes(out)


def strip_npcgener_trailing_dots(raw_lf, npc_names):
    """Remove ponto final de nomes Win32 em Leader:/Follower: quando o arquivo sem ponto existir em npc/."""
    fixed = 0
    out_lines = []
    line_rx = re.compile(rb"^(\s*(?:Leader|Follower):\s*)(\S+)(.*)$")
    for line in raw_lf.split(b"\n"):
        m = line_rx.match(line)
        if m:
            name_bytes = m.group(2)
            name = name_bytes.decode("latin-1")
            if name.endswith(".") and name[:-1] in npc_names and name not in npc_names:
                line = m.group(1) + name[:-1].encode("latin-1") + m.group(3)
                fixed += 1
        out_lines.append(line)
    return b"\n".join(out_lines), fixed


def filter_level_item(raw_lf, valid_items):
    """Remove de LevelItem.txt linhas que referenciem índice de item ausente no ItemList do Global."""
    kept = []
    dropped = 0
    for line in raw_lf.split(b"\n"):
        s = line.strip()
        if s and not s.startswith(b"#"):
            parts = s.split()
            if len(parts) >= 4 and parts[3].isdigit():
                idx = int(parts[3])
                if idx != 0 and idx not in valid_items:
                    dropped += 1
                    continue
        kept.append(line)
    return b"\n".join(kept), dropped


def build(w2pp_dir, global_dir, dest_dir, ips, bin_dir=None, no_binaries=False, user=None):
    w2pp = Path(w2pp_dir)
    glob = Path(global_dir)
    dst = Path(dest_dir)

    if not w2pp.is_dir():
        raise ValueError(f"diretório do servidor W2PP inexistente: {w2pp}")
    if not glob.is_dir() or LD.find_case_insensitive(glob, "ItemList.bin") is None:
        raise ValueError(f"{glob} não parece uma instalação do WYD Global (falta ItemList.bin)")
    if dst.exists() and any(dst.iterdir()):
        raise ValueError(f"{dst} já existe e não está vazia")
    dst_res = dst.resolve()
    if dst_res.is_relative_to(w2pp.resolve()) or dst_res.is_relative_to(glob.resolve()):
        raise ValueError("a saída não pode ficar dentro das pastas de origem")
    if not ips:
        raise ValueError("informe ao menos um endereço IP em --ip")

    # Valida entradas antes de criar a árvore de destino.
    hmap_src = require_ci(glob, "HeightMap.dat", "HeightMap.dat")
    if hmap_src.stat().st_size != HEIGHTMAP_SIZE:
        raise ValueError(f"{hmap_src}: tamanho {hmap_src.stat().st_size} diferente de {HEIGHTMAP_SIZE}")
    attr_src = require_ci(glob, "Env/AttributeMap.dat", "Env/AttributeMap.dat")
    attr_bytes = attr_src.read_bytes()
    if len(attr_bytes) < ATTRIBUTE_MAP_SIZE:
        raise ValueError(f"{attr_src}: tamanho {len(attr_bytes)} menor que {ATTRIBUTE_MAP_SIZE}")
    strdef_src = require_ci(glob, "Lang/PT/strdef.bin", "Lang/PT/strdef.bin")
    basemob_srcs = {cls: require_ci(w2pp, f"DBSrv/run/BaseMob/{cls}", f"BaseMob/{cls}") for cls in BASE_CLASSES}
    basesummon_dir = require_ci(w2pp, "TMSrv/run/BaseSummon", "TMSrv/run/BaseSummon")
    npc_dir = require_ci(w2pp, "TMSrv/run/npc", "TMSrv/run/npc")

    for base, subs in LD.WRITE_DIRS.items():
        for s in [""] + subs:
            (dst / base / s).mkdir(parents=True, exist_ok=True)

    common = dst / "Common"
    db = dst / "DBSRV/Run"
    tm = dst / "TMSRV/Run"

    if not no_binaries:
        bins = Path(bin_dir).resolve() if bin_dir else ROOT / "build/linux"
        for exe, sub, run in (("DBSrv", "server/dbsrv", db), ("TMSrv", "server/tmsrv", tm)):
            src_bin = next((p for p in (bins / exe, bins / sub / exe) if p.is_file()), None)
            if not src_bin:
                raise ValueError(f"não achei {exe} em {bins} (use --bin ou --no-binaries)")
            shutil.copy2(src_bin, run / exe)
            os.chmod(run / exe, 0o755)

    # Endereços de rede
    ip_list = list(dict.fromkeys(ips))[:MAX_SERVERGROUP]
    (common / "serverlist.txt").write_text(
        "".join(f"{g} 0 {ip}\n{g} 1 {ip}\n" for g, ip in enumerate(ip_list)),
        encoding="ascii",
    )
    (tm / "localip.txt").write_text(ip_list[0] + "\n", encoding="ascii")

    # Tabelas ItemList e SkillData do Global -> CSV do servidor + ItemEffect.h
    effects_h = ROOT / "server/common/ItemEffect.h"
    shutil.copy2(effects_h, common / "ItemEffect.h")
    shutil.copy2(effects_h, tm / "ItemEffect.h")

    itemlist_bin = require_ci(glob, "ItemList.bin", "ItemList.bin")
    extra_bin = GC.find_ci(glob, "extraitem.bin")
    W.convert_itemlist(itemlist_bin, common / "itemlist.csv", extra=extra_bin, effects_path=effects_h, quiet=True)
    skilldata_bin = require_ci(glob, "SkillData.bin", "SkillData.bin")
    W.convert_skills(skilldata_bin, common / "SkillData.csv", quiet=True)
    valid_items = load_valid_global_items(glob)

    # Mapas e UI/strdef.bin
    shutil.copy2(hmap_src, tm / "heightmap.dat")
    attr_1m = attr_bytes[:ATTRIBUTE_MAP_SIZE]
    (db / "AttributeMap.dat").write_bytes(attr_1m)
    (tm / "AttributeMap.dat").write_bytes(attr_1m)

    strdef_bin = GC.strdef_for_client(strdef_src.read_bytes())
    for run_dir in (db, tm):
        (run_dir / "UI").mkdir(exist_ok=True)
        (run_dir / "UI/strdef.bin").write_bytes(strdef_bin)

    # BaseMob, BaseSummon e npc (816B/1296B -> 1024B + filtragem de itens ausentes no Global)
    removed_items = {}
    (db / "BaseMob").mkdir(exist_ok=True)
    for cls, src_path in basemob_srcs.items():
        (db / "BaseMob" / cls).write_bytes(
            convert_mob_to_1024(src_path.read_bytes(), f"BaseMob/{cls}", valid_items, False, removed_items)
        )

    (tm / "BaseSummon").mkdir(exist_ok=True)
    summon_count = 0
    for p in sorted(basesummon_dir.iterdir()):
        if not p.is_file():
            continue
        conv = convert_mob_to_1024(p.read_bytes(), f"BaseSummon/{p.name}", valid_items, True, removed_items)
        (tm / "BaseSummon" / p.name).write_bytes(conv)
        if p.name == "Dragao_Menor":
            (tm / "BaseSummon/Dragao_menor").write_bytes(conv)
        summon_count += 1

    (tm / "npc").mkdir(exist_ok=True)
    npc_names = set()
    npc_1296 = []
    for p in sorted(npc_dir.iterdir()):
        if not p.is_file():
            continue
        raw = p.read_bytes()
        if len(raw) == SIZEOF_MOB_1296:
            npc_1296.append(p.name)
        (tm / "npc" / p.name).write_bytes(
            convert_mob_to_1024(raw, f"npc/{p.name}", valid_items, True, removed_items)
        )
        npc_names.add(p.name)

    # NPCGener.new.txt (preferido) ou NPCGener.txt -> TMSRV/Run/NPCGener.txt
    npcgen_src = LD.find_case_insensitive(w2pp, "TMSrv/run/NPCGener.new.txt") or require_ci(
        w2pp, "TMSrv/run/NPCGener.txt", "NPCGener.txt"
    )
    npcgen_lf, fixed_dots = strip_npcgener_trailing_dots(normalize_lf_bytes(npcgen_src.read_bytes()), npc_names)
    (tm / "NPCGener.txt").write_bytes(npcgen_lf)

    # LevelItem.txt, InitItem.csv, Language.txt, gameconfig.txt e Settings/
    lvl_src = LD.find_case_insensitive(w2pp, "TMSrv/run/LevelItem.txt")
    dropped_lvl = 0
    if lvl_src:
        lvl_bytes, dropped_lvl = filter_level_item(normalize_lf_bytes(lvl_src.read_bytes()), valid_items)
        (tm / "LevelItem.txt").write_bytes(lvl_bytes)
    else:
        (tm / "LevelItem.txt").write_bytes(b"")

    init_item = LD.find_case_insensitive(w2pp, "TMSrv/run/InitItem.csv")
    if init_item:
        copy_text_lf(init_item, common / "InitItem.csv")
    else:
        (common / "InitItem.csv").write_bytes(b"")

    for src_rel, dst_path in (
        ("TMSrv/run/Language.txt", tm / "Language.txt"),
        ("TMSrv/run/gameconfig.txt", tm / "gameconfig.txt"),
    ):
        found = LD.find_case_insensitive(w2pp, src_rel)
        if found:
            copy_text_lf(found, dst_path)

    for cfg in SETTINGS_FILES:
        found = LD.find_case_insensitive(w2pp, f"Common/Settings/{cfg}")
        if found:
            copy_text_lf(found, common / "Settings" / cfg)
        else:
            (common / "Settings" / cfg).write_bytes(b"")

    # Arquivos locais de operação limpos (sem dados de terceiros)
    (db / "Config.txt").write_text("Sapphire 8\nLastCapsule 0\n", encoding="ascii")
    for empty_path in (common / "Guilds.txt", common / "GuildInfo", db / "Admin.txt", db / "Mac.txt"):
        empty_path.write_bytes(b"")

    units = LD.write_units(dst, user or getpass.getuser())

    # Confere que nenhum arquivo existente difere da caixa esperada pelo código C++.
    wrong = [
        (rel, LD.find_case_insensitive(dst, rel))
        for rel in sorted(LD.expected_files())
        if not (dst / rel).exists() and LD.find_case_insensitive(dst, rel) is not None
    ]
    if wrong:
        details = ", ".join(f"{rel} (existe {f.relative_to(dst).as_posix()})" for rel, f in wrong)
        raise ValueError(f"arquivos com caixa divergente na saída: {details}")

    print(
        f"Árvore montada em {dst}: 4 BaseMob, {summon_count} BaseSummon, {len(npc_names)} npc "
        f"(1296B: {len(npc_1296)}), {len(valid_items)} itens no ItemList"
    )
    print(
        f"Ajustes aplicados: {fixed_dots} nome(s) sem ponto em NPCGener.txt, "
        f"{len(removed_items)} ID(s) de item ausente removidos de slots, {dropped_lvl} linha(s) de LevelItem.txt"
    )
    print("Unidades systemd:", ", ".join(u.name for u in units))
    return 0


def make_synthetic_mob_816(name, *, clan=1, merchant=2, guild=7, cls=3, rsv=0x102, quest=9,
                           coin=123456, exp=5000000009, level=150000, ac=12, dmg=34,
                           str_v=10, int_v=20, dex_v=30, con_v=40, maxmp=0,
                           equip0=200, equip1=1, carry0=999, learned=0x11223344,
                           magic=300, regenhp=15, regenmp=300, resist=(10, 20, 30, 40),
                           dir_cc=False):
    b = bytearray(SIZEOF_MOB_816)
    enc_name = name.encode("latin-1")[:15]
    b[0:len(enc_name)] = enc_name
    b[16] = clan
    b[17] = merchant
    struct.pack_into("<H", b, 18, guild)
    b[20] = cls
    struct.pack_into("<H", b, 22, rsv)
    b[24] = quest
    struct.pack_into("<i", b, 28, coin)
    struct.pack_into("<q", b, 32, exp)
    for score_off in (44, 92):
        struct.pack_into("<iii", b, score_off, level, ac, dmg)
        b[score_off + 14] = 0xCC if dir_cc else 2
        b[score_off + 15] = 0xCC if dir_cc else 3
        struct.pack_into("<iiii", b, score_off + 16, 100, maxmp, 100, maxmp)
        struct.pack_into("<4h", b, score_off + 32, str_v, int_v, dex_v, con_v)
    struct.pack_into("<h", b, 140, equip0)
    struct.pack_into("<h", b, 148, equip1)
    struct.pack_into("<h", b, 268, carry0)
    struct.pack_into("<I", b, 780, learned)
    struct.pack_into("<I", b, 784, magic)
    struct.pack_into("<HH", b, 802, regenhp, regenmp)
    b[806:810] = bytes(resist)
    return bytes(b)


def selftest():
    failures = []

    def check(ok, what):
        if not ok:
            failures.append(what)

    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        w2pp = root / "w2pp"
        glob = root / "global"
        dst = root / "out"

        # Mini árvore W2PP sintética (com arquivos sensíveis falsos que NÃO podem vazar para dst).
        for sub in ("DBSrv/run/BaseMob", "DBSrv/run/account/A", "DBSrv/run/char/H", "DBSrv/run/Log",
                    "TMSrv/run/BaseSummon", "TMSrv/run/npc", "Common/Settings"):
            (w2pp / sub).mkdir(parents=True, exist_ok=True)
        for cls_idx, cls in enumerate(BASE_CLASSES):
            (w2pp / "DBSrv/run/BaseMob" / cls).write_bytes(
                make_synthetic_mob_816(cls, cls=cls_idx, coin=100 + cls_idx, carry0=999, dir_cc=True)
            )
        (w2pp / "DBSrv/run/account/A/TESTE").write_bytes(b"SENSITIVE_ACCOUNT")
        (w2pp / "DBSrv/run/char/H/HEROI").write_bytes(b"SENSITIVE_CHAR")
        (w2pp / "DBSrv/run/Admin.txt").write_bytes(b"SENSITIVE_ADMIN")
        (w2pp / "DBSrv/run/Mac.txt").write_bytes(b"SENSITIVE_MAC")
        (w2pp / "DBSrv/run/Log/old.txt").write_bytes(b"SENSITIVE_LOG")

        (w2pp / "TMSrv/run/BaseSummon/Dragao_Menor").write_bytes(
            make_synthetic_mob_816("Dragao", equip0=999, equip1=999, carry0=1)
        )
        (w2pp / "TMSrv/run/npc/Chefe_Treina").write_bytes(
            make_synthetic_mob_816("Chefe_Treina", maxmp=9000, equip0=999, equip1=999, carry0=1)
        )
        (w2pp / "TMSrv/run/npc/Balmus").write_bytes(
            make_synthetic_mob_816("Balmus", merchant=6) + b"\x00" * (SIZEOF_MOB_1296 - SIZEOF_MOB_816)
        )
        (w2pp / "TMSrv/run/NPCGener.new.txt").write_bytes(
            b"\xef\xbb\xbf#\t[   0]\r\n\tLeader:\tChefe_Treina.\r\n\tFollower:\tChefe_Treina.\r\n"
        )
        (w2pp / "TMSrv/run/LevelItem.txt").write_bytes(
            b"#\r\n1 10 0 1 0\r\n2 10 0 999 0\r\n"
        )
        (w2pp / "TMSrv/run/InitItem.csv").write_bytes(b"1,0\r\n")
        (w2pp / "TMSrv/run/Language.txt").write_bytes(b"1\tOla\r\n")
        (w2pp / "TMSrv/run/gameconfig.txt").write_bytes(b"Drop Item Event Settings:\r\n")
        (w2pp / "Common/Settings/SancRate.txt").write_bytes(b"PO 0 100\r\n\xc3\x82mago 0 50\r\n")

        # Mini instalação Global sintética.
        (glob / "Env").mkdir(parents=True)
        (glob / "Lang/PT").mkdir(parents=True)
        g_items = [[b""] + [0] * 51 for _ in range(W.G_MAX_ITEMLIST)]
        g_items[1][0] = b"Espada"
        g_items[1][1] = 1
        g_body = b"".join(W.G_ITEM_FMT.pack(*r) for r in g_items)
        (glob / "ItemList.bin").write_bytes(W.xor(g_body, 0x5A) + struct.pack("<i", 0x4823))

        g_extra = struct.pack("<h", 2) + W.G_ITEM_FMT.pack(*([b"Armadura", 2] + [0] * 50))
        (glob / "extraitem.bin").write_bytes(g_extra)

        g_spells = [[0] * 12 + [b"", b""] + [0] * 10 for _ in range(W.MAX_SPELL_LIST)]
        g_spells[1][0] = 10
        g_spells[1][14 + 8] = 2
        s_body = b"".join(W.G_SPELL_FMT.pack(*r) for r in g_spells)
        (glob / "SkillData.bin").write_bytes(W.xor(s_body, 0x5A) + struct.pack("<i", 0x29))

        (glob / "HeightMap.dat").write_bytes(b"\x01" * HEIGHTMAP_SIZE)
        (glob / "Env/AttributeMap.dat").write_bytes(b"\x02" * (ATTRIBUTE_MAP_SIZE + 4))
        (glob / "Lang/PT/strdef.bin").write_bytes(b"ok\x00" + b"\x00" * (GC.STRDEF_SIZE - 3))

        rc = build(w2pp, glob, dst, ["127.0.1.1"], no_binaries=True)
        check(rc == 0, f"build retornou {rc}")

        tk = (dst / "DBSRV/Run/BaseMob/TK").read_bytes()
        check(len(tk) == SIZEOF_MOB_1024, "BaseMob/TK tamanho 1024")
        check(tk[21] == 0x02, "Rsv mascarado para uint8")
        check(struct.unpack_from("<H", tk, 22)[0] == 9, "Quest em offset 22")
        check(struct.unpack_from("<i", tk, 24)[0] == 100, "Coin em offset 24")
        check(struct.unpack_from("<q", tk, 32)[0] == 5000000009, "Exp em offset 32")
        check(struct.unpack_from("<h", tk, 44)[0] == 32767, "Level truncado para int16")
        check(tk[44 + 14] == 0 and tk[44 + 15] == 0, "padding 0xCC limpo")
        check(struct.unpack_from("<h", tk, 140)[0] == 0, "BaseMob: Equip[0] ausente zerado")
        check(struct.unpack_from("<h", tk, 148)[0] == 1, "BaseMob: Equip[1] valido preservado")
        check(struct.unpack_from("<h", tk, 268)[0] == 0, "BaseMob: Carry[0] ausente zerado")
        check(struct.unpack_from("<I", tk, 780)[0] == 0x11223344 and struct.unpack_from("<I", tk, 784)[0] == 0,
              "LearnedSkill[0..1]")
        check(tk[801:808] == bytes([255, 15, 255, 10, 20, 30, 40]), "Magic/Regen/Resist em 801..807")

        npc = (dst / "TMSRV/Run/npc/Chefe_Treina").read_bytes()
        check(len(npc) == SIZEOF_MOB_1024, "npc 1024B")
        check(struct.unpack_from("<h", npc, 140)[0] == 999, "npc: Equip[0] preservado como rosto")
        check(struct.unpack_from("<h", npc, 148)[0] == 0, "npc: Equip[1] ausente zerado")
        check(struct.unpack_from("<h", npc, 268)[0] == 1, "npc: Carry[0] valido preservado")
        check(struct.unpack_from("<i", npc, 44 + 20)[0] == 9000, "npc: MaxMp preservado")
        check(len((dst / "TMSRV/Run/npc/Balmus").read_bytes()) == SIZEOF_MOB_1024, "npc 1296B -> 1024B")
        check((dst / "TMSRV/Run/BaseSummon/Dragao_menor").is_file(), "alias Dragao_menor criado")

        npcgen_out = (dst / "TMSRV/Run/NPCGener.txt").read_bytes()
        check(b"\r" not in npcgen_out and not npcgen_out.startswith(b"\xef\xbb\xbf"), "NPCGener LF sem BOM")
        check(b"Chefe_Treina." not in npcgen_out and b"Leader:\tChefe_Treina\n" in npcgen_out, "ponto final removido")

        lvl_out = (dst / "TMSRV/Run/LevelItem.txt").read_bytes()
        check(b"1 10 0 1 0" in lvl_out and b"999" not in lvl_out, "LevelItem filtrado pelo ItemList")
        check(b"\r" not in (dst / "TMSRV/Run/gameconfig.txt").read_bytes(), "gameconfig sem CR")
        check(b"\xc3\x82mago 0 50" in (dst / "Common/Settings/SancRate.txt").read_bytes(), "SancRate preserva Amago")
        check(b"2,Armadura," in (dst / "Common/itemlist.csv").read_bytes(),
              "extraitem aplicado em Common/itemlist.csv")
        check((dst / "TMSRV/Run/AttributeMap.dat").stat().st_size == ATTRIBUTE_MAP_SIZE, "AttributeMap 1 MB")
        check((dst / "TMSRV/Run/heightmap.dat").stat().st_size == HEIGHTMAP_SIZE, "heightmap 16 MB")
        check((dst / "DBSRV/Run/UI/strdef.bin").stat().st_size == GC.STRDEF_SIZE + 4, "strdef com soma")

        check(not (dst / "DBSRV/Run/account/A/TESTE").exists(), "não copia account/")
        check(not (dst / "DBSRV/Run/char/H/HEROI").exists(), "não copia char/")
        check((dst / "DBSRV/Run/Admin.txt").stat().st_size == 0, "Admin.txt zerado")
        check((dst / "DBSRV/Run/Mac.txt").stat().st_size == 0, "Mac.txt zerado")
        check(not (dst / "DBSRV/Run/Log/old.txt").exists(), "não copia Log/")

    for f in failures:
        print(f"FALHA {f}")
    if failures:
        return 1
    print("global_server_data: selftest OK")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build", help="monta a árvore do servidor Linux a partir de W2PP + WYD Global")
    b.add_argument("w2pp", help="raiz do servidor W2PP (somente leitura)")
    b.add_argument("glob", help="instalação do WYD Global (somente leitura)")
    b.add_argument("dest", help="diretório de destino, novo ou vazio")
    b.add_argument("--ip", required=True, help="IP principal (localip.txt e grupo 0)")
    b.add_argument("--extra-ip", action="append", default=[], help="IPs adicionais no serverlist.txt")
    b.add_argument("--bin", help="diretório com DBSrv e TMSrv compilados no Linux")
    b.add_argument("--no-binaries", action="store_true", help="não copia DBSrv/TMSrv")
    b.add_argument("--user", help="usuário para as unidades do systemd")
    sub.add_parser("selftest", help="confere a conversão e o isolamento com dados sintéticos")
    args = ap.parse_args()
    try:
        if args.cmd == "selftest":
            return selftest()
        return build(
            args.w2pp,
            args.glob,
            args.dest,
            [args.ip, *args.extra_ip],
            bin_dir=args.bin,
            no_binaries=args.no_binaries,
            user=args.user,
        )
    except (ValueError, OSError, W.FormatError) as e:
        print(f"erro: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
