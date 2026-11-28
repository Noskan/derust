//! Arquivo de conta do DBSrv (`STRUCT_ACCOUNTFILE`, 8640 bytes) <-> [`pb::Account`].
//!
//! Só o importador e o exportador usam este módulo: em jogo, o DBSrv converte as estruturas para
//! o contrato no C++. Os offsets são os de `shared/protocol/account_layout.txt`, que o C++ gera e
//! confere (`tests/account_layout_dump.cpp`); o teste `layout_matches_cpp` confere a tabela de
//! campos daqui contra o mesmo arquivo.
//!
//! Volta igual ao arquivo, byte a byte, tudo exceto a senha, a senha numérica (o banco guarda só os
//! hashes) e os dados pessoais (nome real, SSN, e-mail, telefone e endereço, que não são gravados).
//! Os bytes de alinhamento voltam zerados, como o DBSrv os grava.

use crate::pb;

pub const ACCOUNT_FILE_SIZE: usize = 8640;
pub const MOB_SIZE: usize = 1024;
pub const MOBEXTRA_SIZE: usize = 536;
pub const MOB_PER_ACCOUNT: usize = 4;
pub const MAX_EQUIP: usize = 16;
pub const MAX_CARRY: usize = 64;
pub const MAX_CARGO: usize = 128;
pub const MAX_AFFECT: usize = 32;
pub const NAME_LENGTH: usize = 16;
pub const ACCOUNTPASS_LENGTH: usize = 16;

const OFF_CHAR: usize = 224;
const OFF_CARGO: usize = 4320;
const OFF_COIN: usize = 5344;
const OFF_SHORTSKILL: usize = 5348;
const OFF_AFFECT: usize = 5412;
const OFF_MOBEXTRA: usize = 6440;
const OFF_DONATE: usize = 8584;
const OFF_TEMPKEY: usize = 8588;

const AFFECT_BYTES: usize = MAX_AFFECT * 8;
const SHORTSKILL_BYTES: usize = 16;

/// Campos do arquivo que não voltam iguais (senha, senha numérica e dados pessoais): offset e
/// tamanho dentro do arquivo.
pub const NOT_ROUND_TRIPPED: &[(&str, usize, usize)] = &[
    ("AccountPass", 16, 16),
    ("RealName", 32, 24),
    ("SSN1", 56, 4),
    ("SSN2", 60, 4),
    ("Email", 64, 48),
    ("Telephone", 112, 16),
    ("Address", 128, 78),
    ("NumericToken", 206, 6),
];

#[derive(Debug, thiserror::Error)]
pub enum LegacyError {
    #[error("o arquivo de conta tem {0} bytes, esperava {ACCOUNT_FILE_SIZE}")]
    Size(usize),
    #[error("{0}")]
    Invalid(String),
}

/// O que vem do arquivo além da conta: os segredos em texto puro, para o importador gerar os
/// hashes.
pub struct Secrets {
    pub password: Vec<u8>,
    /// `None` quando a conta não tem senha numérica (o DBSrv grava -1 no primeiro byte).
    pub numeric: Option<Vec<u8>>,
}

// ---------------------------------------------------------------- leitura e escrita

/// Leitor sequencial que registra cada campo lido (nome, offset, tamanho), para o teste de layout.
struct Rd<'a> {
    b: &'a [u8],
    base: usize,
    off: usize,
    trace: Option<&'a mut Vec<(String, usize, usize)>>,
}

impl<'a> Rd<'a> {
    fn new(b: &'a [u8]) -> Self {
        Rd {
            b,
            base: 0,
            off: 0,
            trace: None,
        }
    }
    fn at(&mut self, name: &str, off: usize, len: usize) -> &'a [u8] {
        if let Some(t) = self.trace.as_deref_mut() {
            t.push((name.to_string(), off, len));
        }
        self.off = off + len;
        &self.b[self.base + off..self.base + off + len]
    }
    fn bytes(&mut self, name: &str, off: usize, len: usize) -> Vec<u8> {
        self.at(name, off, len).to_vec()
    }
    fn u8(&mut self, name: &str, off: usize) -> u32 {
        self.at(name, off, 1)[0] as u32
    }
    fn i8(&mut self, name: &str, off: usize) -> i32 {
        self.at(name, off, 1)[0] as i8 as i32
    }
    fn u16(&mut self, name: &str, off: usize) -> u32 {
        u16::from_le_bytes(self.at(name, off, 2).try_into().unwrap()) as u32
    }
    fn i16(&mut self, name: &str, off: usize) -> i32 {
        i16::from_le_bytes(self.at(name, off, 2).try_into().unwrap()) as i32
    }
    fn u32(&mut self, name: &str, off: usize) -> u32 {
        u32::from_le_bytes(self.at(name, off, 4).try_into().unwrap())
    }
    fn i32(&mut self, name: &str, off: usize) -> i32 {
        i32::from_le_bytes(self.at(name, off, 4).try_into().unwrap())
    }
    fn i64(&mut self, name: &str, off: usize) -> i64 {
        i64::from_le_bytes(self.at(name, off, 8).try_into().unwrap())
    }
    /// Sub-leitor para uma estrutura em `off`; os campos dela ficam registrados com o prefixo.
    fn sub<R>(&mut self, off: usize, f: impl FnOnce(&mut Rd<'_>) -> R) -> R {
        let mut inner = Vec::new();
        let r = {
            let mut rd = Rd {
                b: self.b,
                base: self.base + off,
                off: 0,
                trace: self.trace.is_some().then_some(&mut inner),
            };
            f(&mut rd)
        };
        if let Some(t) = self.trace.as_deref_mut() {
            t.extend(inner);
        }
        r
    }
}

/// Escritor com o mesmo endereçamento do leitor.
struct Wr<'a> {
    b: &'a mut [u8],
    base: usize,
}

impl Wr<'_> {
    fn put(&mut self, off: usize, v: &[u8]) {
        let s = self.base + off;
        self.b[s..s + v.len()].copy_from_slice(v);
    }
    /// Bytes de tamanho fixo: copia o que houver (até `len`) e deixa o resto zerado.
    fn bytes(&mut self, off: usize, len: usize, v: &[u8]) {
        let n = v.len().min(len);
        self.put(off, &v[..n]);
    }
    fn u8(&mut self, off: usize, v: u32) {
        self.put(off, &[v as u8]);
    }
    fn i8(&mut self, off: usize, v: i32) {
        self.put(off, &[v as i8 as u8]);
    }
    fn u16(&mut self, off: usize, v: u32) {
        self.put(off, &(v as u16).to_le_bytes());
    }
    fn i16(&mut self, off: usize, v: i32) {
        self.put(off, &(v as i16).to_le_bytes());
    }
    fn u32(&mut self, off: usize, v: u32) {
        self.put(off, &v.to_le_bytes());
    }
    fn i32(&mut self, off: usize, v: i32) {
        self.put(off, &v.to_le_bytes());
    }
    fn i64(&mut self, off: usize, v: i64) {
        self.put(off, &v.to_le_bytes());
    }
    fn sub(&mut self, off: usize) -> Wr<'_> {
        Wr {
            b: self.b,
            base: self.base + off,
        }
    }
}

/// Texto de tamanho fixo até o primeiro zero.
fn cstr(b: &[u8]) -> Vec<u8> {
    b.iter().take_while(|&&c| c != 0).copied().collect()
}

/// Bytes reservados: vazio quando são todos zero.
fn reserved(b: Vec<u8>) -> Vec<u8> {
    if b.iter().all(|&c| c == 0) { Vec::new() } else { b }
}

/// Tira os elementos zerados do fim de um vetor de tamanho fixo.
fn trim_tail<T: PartialEq + Default>(mut v: Vec<T>) -> Vec<T> {
    let zero = T::default();
    while v.last() == Some(&zero) {
        v.pop();
    }
    v
}

// ---------------------------------------------------------------- estruturas

fn rd_item(r: &mut Rd<'_>) -> pb::Item {
    let index = r.i16("sIndex", 0);
    let e = r.at("stEffect", 2, 6);
    pb::Item {
        index,
        effects: (0..3)
            .map(|i| pb::ItemEffect {
                effect: e[i * 2] as u32,
                value: e[i * 2 + 1] as u32,
            })
            .collect(),
    }
}

fn wr_item(w: &mut Wr<'_>, it: &pb::Item) {
    w.i16(0, it.index);
    for (i, e) in it.effects.iter().take(3).enumerate() {
        w.u8(2 + i * 2, e.effect);
        w.u8(3 + i * 2, e.value);
    }
}

fn item_is_empty(b: &[u8]) -> bool {
    b.iter().all(|&c| c == 0)
}

/// Itens de um vetor: só as vagas com algum byte diferente de zero (serial 0: ainda sem serial).
fn rd_items(r: &mut Rd<'_>, name: &str, off: usize, count: usize) -> Vec<pb::StoredItem> {
    let all = r.at(name, off, count * 8);
    let mut out = Vec::new();
    for slot in 0..count {
        if item_is_empty(&all[slot * 8..slot * 8 + 8]) {
            continue;
        }
        let item = r.sub(off + slot * 8, |r| {
            let mut quiet = Rd {
                b: r.b,
                base: r.base,
                off: 0,
                trace: None,
            };
            rd_item(&mut quiet)
        });
        out.push(pb::StoredItem {
            serial: 0,
            slot: slot as u32,
            item: Some(item),
        });
    }
    out
}

fn wr_items(w: &mut Wr<'_>, off: usize, count: usize, items: &[pb::StoredItem], what: &str) -> Result<(), LegacyError> {
    for it in items {
        let slot = it.slot as usize;
        if slot >= count {
            return Err(LegacyError::Invalid(format!("{what}: vaga {slot} fora de 0..{count}")));
        }
        if let Some(item) = &it.item {
            wr_item(&mut w.sub(off + slot * 8), item);
        }
    }
    Ok(())
}

fn rd_score(r: &mut Rd<'_>) -> pb::Score {
    pb::Score {
        level: r.i16("Level", 0),
        ac: r.i32("Ac", 4),
        damage: r.i32("Damage", 8),
        merchant: r.u8("Merchant", 12),
        attack_run: r.u8("AttackRun", 13),
        direction: r.u8("Direction", 14),
        chaos_rate: r.u8("ChaosRate", 15),
        max_hp: r.i32("MaxHp", 16),
        max_mp: r.i32("MaxMp", 20),
        hp: r.i32("Hp", 24),
        mp: r.i32("Mp", 28),
        str: r.i16("Str", 32),
        int: r.i16("Int", 34),
        dex: r.i16("Dex", 36),
        con: r.i16("Con", 38),
        special: {
            let b = r.at("Special", 40, 8);
            (0..4)
                .map(|i| i16::from_le_bytes([b[i * 2], b[i * 2 + 1]]) as i32)
                .collect()
        },
    }
}

fn wr_score(w: &mut Wr<'_>, s: &pb::Score) {
    w.i16(0, s.level);
    w.i32(4, s.ac);
    w.i32(8, s.damage);
    w.u8(12, s.merchant);
    w.u8(13, s.attack_run);
    w.u8(14, s.direction);
    w.u8(15, s.chaos_rate);
    w.i32(16, s.max_hp);
    w.i32(20, s.max_mp);
    w.i32(24, s.hp);
    w.i32(28, s.mp);
    w.i16(32, s.str);
    w.i16(34, s.int);
    w.i16(36, s.dex);
    w.i16(38, s.con);
    for (i, v) in s.special.iter().take(4).enumerate() {
        w.i16(40 + i * 2, *v);
    }
}

/// `STRUCT_MOB`: nome, o resto do mob, equipamento e inventário.
fn rd_mob(r: &mut Rd<'_>) -> (Vec<u8>, pb::Mob, Vec<pb::StoredItem>, Vec<pb::StoredItem>) {
    let name = cstr(r.at("MobName", 0, 16));
    let clan = r.i8("Clan", 16);
    let merchant = r.u8("Merchant", 17);
    let guild = r.u16("Guild", 18);
    let class = r.u8("Class", 20);
    let rsv = r.u8("Rsv", 21);
    let quest = r.u16("Quest", 22);
    let coin = r.i32("Coin", 24);
    let exp = r.i64("Exp", 32);
    let home_town_x = r.i16("HomeTownX", 40);
    let home_town_y = r.i16("HomeTownY", 42);
    r.at("BaseScore", 44, 48);
    let base_score = r.sub(44, |r| quiet(r, rd_score));
    r.at("CurrentScore", 92, 48);
    let current_score = r.sub(92, |r| quiet(r, rd_score));
    let equip = rd_items(r, "Equip", 140, MAX_EQUIP);
    let carry = rd_items(r, "Carry", 268, MAX_CARRY);
    let ls = r.at("LearnedSkill", 780, 8);
    let learned_skill = trim_tail(vec![
        u32::from_le_bytes(ls[0..4].try_into().unwrap()),
        u32::from_le_bytes(ls[4..8].try_into().unwrap()),
    ]);
    let mob = pb::Mob {
        clan,
        merchant,
        guild,
        class,
        rsv,
        quest,
        coin,
        exp,
        home_town_x,
        home_town_y,
        base_score: Some(base_score),
        current_score: Some(current_score),
        learned_skill,
        score_bonus: r.u16("ScoreBonus", 788),
        special_bonus: r.u16("SpecialBonus", 790),
        skill_bonus: r.u16("SkillBonus", 792),
        critical: r.u8("Critical", 794),
        save_mana: r.u8("SaveMana", 795),
        short_skill: r.bytes("ShortSkill", 796, 4),
        guild_level: r.u8("GuildLevel", 800),
        magic: r.u8("Magic", 801),
        regen_hp: r.u8("RegenHP", 802),
        regen_mp: r.u8("RegenMP", 803),
        resist: r.bytes("Resist", 804, 4),
        reserved_dummy: reserved(r.bytes("dummy", 808, 212)),
        current_kill: r.u16("CurrentKill", 1020),
        total_kill: r.u16("TotalKill", 1022),
    };
    (name, mob, equip, carry)
}

/// Lê uma estrutura sem registrar os campos internos (o campo inteiro já foi registrado).
fn quiet<R>(r: &mut Rd<'_>, f: impl FnOnce(&mut Rd<'_>) -> R) -> R {
    let mut q = Rd {
        b: r.b,
        base: r.base,
        off: 0,
        trace: None,
    };
    f(&mut q)
}

fn wr_mob(w: &mut Wr<'_>, c: &pb::Character) -> Result<(), LegacyError> {
    let m = c.mob.clone().unwrap_or_default();
    w.bytes(0, NAME_LENGTH, &c.name);
    w.i8(16, m.clan);
    w.u8(17, m.merchant);
    w.u16(18, m.guild);
    w.u8(20, m.class);
    w.u8(21, m.rsv);
    w.u16(22, m.quest);
    w.i32(24, m.coin);
    w.i64(32, m.exp);
    w.i16(40, m.home_town_x);
    w.i16(42, m.home_town_y);
    wr_score(&mut w.sub(44), &m.base_score.unwrap_or_default());
    wr_score(&mut w.sub(92), &m.current_score.unwrap_or_default());
    wr_items(w, 140, MAX_EQUIP, &c.equip, "equip")?;
    wr_items(w, 268, MAX_CARRY, &c.carry, "carry")?;
    for (i, v) in m.learned_skill.iter().take(2).enumerate() {
        w.u32(780 + i * 4, *v);
    }
    w.u16(788, m.score_bonus);
    w.u16(790, m.special_bonus);
    w.u16(792, m.skill_bonus);
    w.u8(794, m.critical);
    w.u8(795, m.save_mana);
    w.bytes(796, 4, &m.short_skill);
    w.u8(800, m.guild_level);
    w.u8(801, m.magic);
    w.u8(802, m.regen_hp);
    w.u8(803, m.regen_mp);
    w.bytes(804, 4, &m.resist);
    w.bytes(808, 212, &m.reserved_dummy);
    w.u16(1020, m.current_kill);
    w.u16(1022, m.total_kill);
    Ok(())
}

fn rd_save_celestial(r: &mut Rd<'_>, i: usize, off: usize) -> pb::SaveCelestial {
    let p = |f: &str| format!("SaveCelestial[{i}].{f}");
    r.sub(0, |r| pb::SaveCelestial {
        class: r.i32(&p("Class"), off),
        exp: r.i64(&p("Exp"), off + 8),
        spx: r.i16(&p("SPX"), off + 16),
        spy: r.i16(&p("SPY"), off + 18),
        base_score: {
            r.at(&p("BaseScore"), off + 20, 48);
            Some(r.sub(off + 20, |r| quiet(r, rd_score)))
        },
        learned_skill: r.i32(&p("LearnedSkill"), off + 68),
        score_bonus: r.u16(&p("ScoreBonus"), off + 72),
        special_bonus: r.u16(&p("SpecialBonus"), off + 74),
        skill_bonus: r.u16(&p("SkillBonus"), off + 76),
        skill_bar1: r.bytes(&p("SkillBar1"), off + 78, 4),
        skill_bar2: r.bytes(&p("SkillBar2"), off + 82, 16),
        soul: r.i8(&p("Soul"), off + 98),
        reserved: reserved(r.bytes(&p("EMPTY"), off + 99, 30)),
    })
}

fn wr_save_celestial(w: &mut Wr<'_>, off: usize, s: &pb::SaveCelestial) {
    w.i32(off, s.class);
    w.i64(off + 8, s.exp);
    w.i16(off + 16, s.spx);
    w.i16(off + 18, s.spy);
    wr_score(&mut w.sub(off + 20), &s.base_score.clone().unwrap_or_default());
    w.i32(off + 68, s.learned_skill);
    w.u16(off + 72, s.score_bonus);
    w.u16(off + 74, s.special_bonus);
    w.u16(off + 76, s.skill_bonus);
    w.bytes(off + 78, 4, &s.skill_bar1);
    w.bytes(off + 82, 16, &s.skill_bar2);
    w.i8(off + 98, s.soul);
    w.bytes(off + 99, 30, &s.reserved);
}

const SAVE_CELESTIAL_OFF: [usize; 2] = [160, 296];

fn rd_extra(r: &mut Rd<'_>) -> pb::MobExtra {
    pb::MobExtra {
        class_master: r.i16("ClassMaster", 0),
        citizen: r.i8("Citizen", 2),
        fame: r.i32("Fame", 4),
        soul: r.i8("Soul", 8),
        mortal_face: r.i16("MortalFace", 10),
        mortal_newbie: r.i8("QuestInfo.Mortal.Newbie", 12),
        mortal_terra_mistica: r.i8("QuestInfo.Mortal.TerraMistica", 13),
        mortal_molar_gargula: r.i8("QuestInfo.Mortal.MolarGargula", 14),
        mortal_pilula_orc: r.i8("QuestInfo.Mortal.PilulaOrc", 15),
        mortal_reserved: reserved(r.bytes("QuestInfo.Mortal.EMPTY", 16, 30)),
        arch_mortal_slot: r.i8("QuestInfo.Arch.MortalSlot", 46),
        arch_mortal_level: r.i8("QuestInfo.Arch.MortalLevel", 47),
        arch_level355: r.i8("QuestInfo.Arch.Level355", 48),
        arch_level370: r.i8("QuestInfo.Arch.Level370", 49),
        arch_cristal: r.i8("QuestInfo.Arch.Cristal", 50),
        arch_reserved: reserved(r.bytes("QuestInfo.Arch.EMPTY", 51, 30)),
        celestial_arch_level: r.i16("QuestInfo.Celestial.ArchLevel", 82),
        celestial_celestial_level: r.i16("QuestInfo.Celestial.CelestialLevel", 84),
        celestial_sub_celestial_level: r.i16("QuestInfo.Celestial.SubCelestialLevel", 86),
        celestial_lv40: r.i8("QuestInfo.Celestial.Lv40", 88),
        celestial_lv90: r.i8("QuestInfo.Celestial.Lv90", 89),
        celestial_add120: r.i8("QuestInfo.Celestial.Add120", 90),
        celestial_add150: r.i8("QuestInfo.Celestial.Add150", 91),
        celestial_add180: r.i8("QuestInfo.Celestial.Add180", 92),
        celestial_add200: r.i8("QuestInfo.Celestial.Add200", 93),
        celestial_arcana: r.i8("QuestInfo.Celestial.Arcana", 94),
        celestial_reset: r.i8("QuestInfo.Celestial.Reset", 95),
        celestial_reserved: reserved(r.bytes("QuestInfo.Celestial.EMPTY", 96, 30)),
        circle: r.i8("QuestInfo.Circle", 126),
        quest_reserved: reserved(r.bytes("QuestInfo.EMPTY", 127, 30)),
        save_celestial: trim_tail(
            SAVE_CELESTIAL_OFF
                .iter()
                .enumerate()
                .map(|(i, &off)| rd_save_celestial(r, i, off))
                .collect(),
        ),
        last_nt: r.i64("LastNT", 432),
        nt: r.i32("NT", 440),
        kefra_ticket: r.i32("KefraTicket", 444),
        divine_end: r.i64("DivineEnd", 448),
        hold: r.u32("Hold", 456),
        day_log_exp: r.i64("DayLog.Exp", 464),
        day_log_year_day: r.i32("DayLog.YearDay", 472),
        donate_last_time: r.i64("DonateInfo.LastTime", 480),
        donate_count: r.i32("DonateInfo.Count", 488),
        reserved: reserved(r.bytes("EMPTY", 496, 36)),
    }
}

fn wr_extra(w: &mut Wr<'_>, e: &pb::MobExtra) {
    w.i16(0, e.class_master);
    w.i8(2, e.citizen);
    w.i32(4, e.fame);
    w.i8(8, e.soul);
    w.i16(10, e.mortal_face);
    w.i8(12, e.mortal_newbie);
    w.i8(13, e.mortal_terra_mistica);
    w.i8(14, e.mortal_molar_gargula);
    w.i8(15, e.mortal_pilula_orc);
    w.bytes(16, 30, &e.mortal_reserved);
    w.i8(46, e.arch_mortal_slot);
    w.i8(47, e.arch_mortal_level);
    w.i8(48, e.arch_level355);
    w.i8(49, e.arch_level370);
    w.i8(50, e.arch_cristal);
    w.bytes(51, 30, &e.arch_reserved);
    w.i16(82, e.celestial_arch_level);
    w.i16(84, e.celestial_celestial_level);
    w.i16(86, e.celestial_sub_celestial_level);
    w.i8(88, e.celestial_lv40);
    w.i8(89, e.celestial_lv90);
    w.i8(90, e.celestial_add120);
    w.i8(91, e.celestial_add150);
    w.i8(92, e.celestial_add180);
    w.i8(93, e.celestial_add200);
    w.i8(94, e.celestial_arcana);
    w.i8(95, e.celestial_reset);
    w.bytes(96, 30, &e.celestial_reserved);
    w.i8(126, e.circle);
    w.bytes(127, 30, &e.quest_reserved);
    for (s, &off) in e.save_celestial.iter().zip(SAVE_CELESTIAL_OFF.iter()) {
        wr_save_celestial(w, off, s);
    }
    w.i64(432, e.last_nt);
    w.i32(440, e.nt);
    w.i32(444, e.kefra_ticket);
    w.i64(448, e.divine_end);
    w.u32(456, e.hold);
    w.i64(464, e.day_log_exp);
    w.i32(472, e.day_log_year_day);
    w.i64(480, e.donate_last_time);
    w.i32(488, e.donate_count);
    w.bytes(496, 36, &e.reserved);
}

fn rd_affects(b: &[u8]) -> Vec<pb::Affect> {
    trim_tail(
        (0..MAX_AFFECT)
            .map(|i| {
                let a = &b[i * 8..i * 8 + 8];
                pb::Affect {
                    r#type: a[0] as u32,
                    level: a[1] as u32,
                    value: u16::from_le_bytes([a[2], a[3]]) as u32,
                    time: u32::from_le_bytes(a[4..8].try_into().unwrap()),
                }
            })
            .collect(),
    )
}

fn wr_affects(w: &mut Wr<'_>, affects: &[pb::Affect]) {
    for (i, a) in affects.iter().take(MAX_AFFECT).enumerate() {
        let o = i * 8;
        w.u8(o, a.r#type);
        w.u8(o + 1, a.level);
        w.u16(o + 2, a.value);
        w.u32(o + 4, a.time);
    }
}

// ---------------------------------------------------------------- conta

/// Lê o arquivo de conta. Os itens vêm com serial 0.
pub fn decode(b: &[u8]) -> Result<(pb::Account, Secrets), LegacyError> {
    decode_traced(b, None)
}

fn decode_traced(
    b: &[u8],
    trace: Option<&mut Vec<(String, usize, usize)>>,
) -> Result<(pb::Account, Secrets), LegacyError> {
    if b.len() != ACCOUNT_FILE_SIZE {
        return Err(LegacyError::Size(b.len()));
    }
    let mut r = Rd::new(b);
    r.trace = trace;

    let name = cstr(r.at("Info.AccountName", 0, 16));
    let password = cstr(r.at("Info.AccountPass", 16, ACCOUNTPASS_LENGTH));
    r.at("Info.RealName", 32, 24);
    r.at("Info.SSN1", 56, 4);
    r.at("Info.SSN2", 60, 4);
    r.at("Info.Email", 64, 48);
    r.at("Info.Telephone", 112, 16);
    r.at("Info.Address", 128, 78);
    let token = r.bytes("Info.NumericToken", 206, 6);
    let year = r.i32("Info.Year", 212);
    let year_day = r.i32("Info.YearDay", 216);

    let mut characters = Vec::new();
    let mut empty_slots = Vec::new();
    for slot in 0..MOB_PER_ACCOUNT {
        let mob_off = OFF_CHAR + slot * MOB_SIZE;
        let ss_off = OFF_SHORTSKILL + slot * SHORTSKILL_BYTES;
        let af_off = OFF_AFFECT + slot * AFFECT_BYTES;
        let ex_off = OFF_MOBEXTRA + slot * MOBEXTRA_SIZE;
        let tag = |f: &str| format!("Char[{slot}].{f}");

        let (cname, mob, equip, carry) = {
            let mut inner = Vec::new();
            let out = {
                let mut sr = Rd {
                    b,
                    base: mob_off,
                    off: 0,
                    trace: r.trace.is_some().then_some(&mut inner),
                };
                rd_mob(&mut sr)
            };
            if let Some(t) = r.trace.as_deref_mut() {
                t.extend(inner.into_iter().map(|(n, o, l)| (tag(&n), mob_off + o, l)));
            }
            out
        };
        let short_skill = r.bytes(&format!("ShortSkill[{slot}]"), ss_off, SHORTSKILL_BYTES);
        let affects_raw = r.at(&format!("affect[{slot}]"), af_off, AFFECT_BYTES);
        let extra = {
            let mut inner = Vec::new();
            let out = {
                let mut sr = Rd {
                    b,
                    base: ex_off,
                    off: 0,
                    trace: r.trace.is_some().then_some(&mut inner),
                };
                rd_extra(&mut sr)
            };
            if let Some(t) = r.trace.as_deref_mut() {
                t.extend(
                    inner
                        .into_iter()
                        .map(|(n, o, l)| (format!("mobExtra[{slot}].{n}"), ex_off + o, l)),
                );
            }
            out
        };

        if cname.is_empty() {
            // Vaga vazia: guarda os bytes que não forem zero, para voltar igual.
            let mob_raw = reserved(b[mob_off..mob_off + MOB_SIZE].to_vec());
            let ss = reserved(short_skill);
            let af = reserved(affects_raw.to_vec());
            let ex = reserved(b[ex_off..ex_off + MOBEXTRA_SIZE].to_vec());
            if !(mob_raw.is_empty() && ss.is_empty() && af.is_empty() && ex.is_empty()) {
                empty_slots.push(pb::EmptySlot {
                    slot: slot as u32,
                    mob: mob_raw,
                    short_skill: ss,
                    affects: af,
                    extra: ex,
                });
            }
            continue;
        }
        characters.push(pb::Character {
            slot: slot as u32,
            name: cname,
            mob: Some(mob),
            equip,
            carry,
            affects: rd_affects(affects_raw),
            short_skill,
            extra: Some(extra),
        });
    }

    let cargo = rd_items(&mut r, "Cargo", OFF_CARGO, MAX_CARGO);
    let cargo_coin = r.i32("Coin", OFF_COIN);
    let donate = r.i32("Donate", OFF_DONATE);
    let temp_key = reserved(r.bytes("TempKey", OFF_TEMPKEY, 52));

    let numeric = (token[0] != 0xFF && token[0] != 0).then(|| cstr(&token));

    let account = pb::Account {
        name,
        version: 0,
        cargo_coin,
        donate,
        year,
        year_day,
        temp_key,
        cargo,
        characters,
        empty_slots,
        numeric_set: numeric.is_some(),
    };
    Ok((account, Secrets { password, numeric }))
}

/// Grava o arquivo de conta. Senha e dados pessoais ficam zerados; sem senha numérica, o primeiro
/// byte dela vai em -1, como na conta nova do DBSrv (com senha numérica, os bytes ficam zerados:
/// o banco só tem o hash).
pub fn encode(a: &pb::Account) -> Result<Vec<u8>, LegacyError> {
    let mut b = vec![0u8; ACCOUNT_FILE_SIZE];
    let mut w = Wr { b: &mut b, base: 0 };

    if a.name.len() >= 16 {
        return Err(LegacyError::Invalid("nome da conta com 16 bytes ou mais".into()));
    }
    w.bytes(0, 16, &a.name);
    if !a.numeric_set {
        w.u8(206, 0xFF);
    }
    w.i32(212, a.year);
    w.i32(216, a.year_day);

    for c in &a.characters {
        let slot = c.slot as usize;
        if slot >= MOB_PER_ACCOUNT {
            return Err(LegacyError::Invalid(format!("personagem na vaga {slot}")));
        }
        if c.name.len() >= NAME_LENGTH {
            return Err(LegacyError::Invalid("nome do personagem com 16 bytes ou mais".into()));
        }
        wr_mob(&mut w.sub(OFF_CHAR + slot * MOB_SIZE), c)?;
        w.bytes(
            OFF_SHORTSKILL + slot * SHORTSKILL_BYTES,
            SHORTSKILL_BYTES,
            &c.short_skill,
        );
        wr_affects(&mut w.sub(OFF_AFFECT + slot * AFFECT_BYTES), &c.affects);
        wr_extra(
            &mut w.sub(OFF_MOBEXTRA + slot * MOBEXTRA_SIZE),
            &c.extra.clone().unwrap_or_default(),
        );
    }
    for e in &a.empty_slots {
        let slot = e.slot as usize;
        if slot >= MOB_PER_ACCOUNT || a.characters.iter().any(|c| c.slot as usize == slot) {
            return Err(LegacyError::Invalid(format!("vaga vazia {slot} inválida")));
        }
        w.bytes(OFF_CHAR + slot * MOB_SIZE, MOB_SIZE, &e.mob);
        w.bytes(
            OFF_SHORTSKILL + slot * SHORTSKILL_BYTES,
            SHORTSKILL_BYTES,
            &e.short_skill,
        );
        w.bytes(OFF_AFFECT + slot * AFFECT_BYTES, AFFECT_BYTES, &e.affects);
        w.bytes(OFF_MOBEXTRA + slot * MOBEXTRA_SIZE, MOBEXTRA_SIZE, &e.extra);
    }

    wr_items(&mut w, OFF_CARGO, MAX_CARGO, &a.cargo, "cargo")?;
    w.i32(OFF_COIN, a.cargo_coin);
    w.i32(OFF_DONATE, a.donate);
    w.bytes(OFF_TEMPKEY, 52, &a.temp_key);
    Ok(b)
}

/// Diretório do arquivo dentro de `account/` e `char/`, como `BASE_GetFirstKey`: a primeira letra
/// ASCII, ou `etc`. Nomes que começam com byte >= 0x80 (o ramo coreano do original) não são
/// aceitos.
pub fn first_key(name: &[u8]) -> Result<String, LegacyError> {
    match name.first() {
        Some(c) if c.is_ascii_alphabetic() => Ok((*c as char).to_string()),
        Some(c) if *c >= 0x80 => Err(LegacyError::Invalid("nome começando com byte >= 0x80".into())),
        _ => Ok("etc".to_string()),
    }
}

/// Chave de unicidade de nomes (conta e personagem): maiúsculas ASCII, como `W2StrUpper` no
/// locale C.
pub fn name_key(name: &[u8]) -> Vec<u8> {
    name.to_ascii_uppercase()
}

// ---------------------------------------------------------------- CP1252

const CP1252_HIGH: [char; 32] = [
    '\u{20AC}', '\u{0081}', '\u{201A}', '\u{0192}', '\u{201E}', '\u{2026}', '\u{2020}', '\u{2021}', '\u{02C6}',
    '\u{2030}', '\u{0160}', '\u{2039}', '\u{0152}', '\u{008D}', '\u{017D}', '\u{008F}', '\u{0090}', '\u{2018}',
    '\u{2019}', '\u{201C}', '\u{201D}', '\u{2022}', '\u{2013}', '\u{2014}', '\u{02DC}', '\u{2122}', '\u{0161}',
    '\u{203A}', '\u{0153}', '\u{009D}', '\u{017E}', '\u{0178}',
];

/// Texto do jogo (CP1252) para o banco. Os 5 bytes sem caractere no CP1252 viram os controles
/// C1 de mesmo valor, então a volta é exata.
pub fn cp1252_to_string(b: &[u8]) -> String {
    b.iter()
        .map(|&c| match c {
            0x80..=0x9F => CP1252_HIGH[(c - 0x80) as usize],
            _ => c as char,
        })
        .collect()
}

pub fn string_to_cp1252(s: &str) -> Result<Vec<u8>, LegacyError> {
    s.chars()
        .map(|ch| {
            let u = ch as u32;
            if u < 0x80 || (0xA0..=0xFF).contains(&u) {
                return Ok(u as u8);
            }
            CP1252_HIGH
                .iter()
                .position(|&h| h == ch)
                .map(|i| 0x80 + i as u8)
                .ok_or_else(|| LegacyError::Invalid(format!("caractere {ch:?} fora do CP1252")))
        })
        .collect()
}

#[doc(hidden)]
pub mod testkit {
    //! Contas sintéticas e a máscara do que volta igual, para os testes (também os de integração).
    use super::*;

    const LAYOUT: &str = include_str!("../../../shared/protocol/account_layout.txt");

    /// Tabela do C++: (estrutura, campo) -> (offset, tamanho).
    pub fn cpp_layout() -> Vec<(String, String, usize, usize)> {
        let mut cur = String::new();
        let mut out = Vec::new();
        for line in LAYOUT.lines() {
            let p: Vec<&str> = line.split_whitespace().collect();
            if p[0] == "struct" {
                cur = p[1].to_string();
            } else {
                out.push((
                    cur.clone(),
                    p[0].to_string(),
                    p[1].parse().unwrap(),
                    p[2].parse().unwrap(),
                ));
            }
        }
        out
    }

    pub fn sizes() -> std::collections::HashMap<String, usize> {
        LAYOUT
            .lines()
            .filter_map(|l| l.strip_prefix("struct "))
            .map(|l| {
                let mut p = l.split_whitespace();
                (p.next().unwrap().to_string(), p.next().unwrap().parse().unwrap())
            })
            .collect()
    }

    /// Bytes de padding (fora de qualquer campo) de uma estrutura, pelo layout do C++.
    pub fn padding(st: &str) -> Vec<usize> {
        let size = sizes()[st];
        let mut covered = vec![false; size];
        for (s, _, o, l) in cpp_layout() {
            if s == st {
                covered[o..o + l].iter_mut().for_each(|c| *c = true);
            }
        }
        (0..size).filter(|&i| !covered[i]).collect()
    }

    /// Posições do arquivo que precisam voltar iguais: tudo menos padding e os campos que não
    /// voltam.
    pub fn round_tripped_mask() -> Vec<bool> {
        let mut m = vec![true; ACCOUNT_FILE_SIZE];
        for i in padding("STRUCT_ACCOUNTFILE") {
            m[i] = false;
        }
        for slot in 0..MOB_PER_ACCOUNT {
            for i in padding("STRUCT_MOB") {
                m[OFF_CHAR + slot * MOB_SIZE + i] = false;
            }
            for off in [44, 92] {
                for i in padding("STRUCT_SCORE") {
                    m[OFF_CHAR + slot * MOB_SIZE + off + i] = false;
                }
            }
            for i in padding("STRUCT_MOBEXTRA") {
                m[OFF_MOBEXTRA + slot * MOBEXTRA_SIZE + i] = false;
            }
            for off in SAVE_CELESTIAL_OFF {
                for i in padding("STRUCT_SCORE") {
                    m[OFF_MOBEXTRA + slot * MOBEXTRA_SIZE + off + 20 + i] = false;
                }
            }
        }
        for &(_, o, l) in NOT_ROUND_TRIPPED {
            m[o..o + l].iter_mut().for_each(|c| *c = false);
        }
        m
    }

    /// Conta com bytes pseudoaleatórios em todo campo (padding zerado), com nomes válidos.
    pub fn random_account(seed: u64, empty_slot: usize) -> Vec<u8> {
        let mask = round_tripped_mask();
        let mut x = seed;
        let mut b: Vec<u8> = (0..ACCOUNT_FILE_SIZE)
            .map(|i| {
                x ^= x << 13;
                x ^= x >> 7;
                x ^= x << 17;
                if mask[i] { x as u8 } else { 0 }
            })
            .collect();
        let put_name = |b: &mut Vec<u8>, off: usize, n: &[u8]| {
            b[off..off + 16].fill(0);
            b[off..off + n.len()].copy_from_slice(n);
        };
        put_name(&mut b, 0, b"CONTA");
        for slot in 0..MOB_PER_ACCOUNT {
            put_name(
                &mut b,
                OFF_CHAR + slot * MOB_SIZE,
                format!("Heroi{slot}\u{e9}").as_bytes(),
            );
        }
        // Uma vaga vazia com resíduo (o nome zerado, o resto como veio).
        b[OFF_CHAR + empty_slot * MOB_SIZE..OFF_CHAR + empty_slot * MOB_SIZE + 16].fill(0);
        b
    }
}

#[cfg(test)]
mod tests {
    use super::testkit::*;
    use super::*;

    /// Cada campo que o codec lê tem o offset e o tamanho do C++.
    #[test]
    fn layout_matches_cpp() {
        let s = sizes();
        assert_eq!(s["STRUCT_ACCOUNTFILE"], ACCOUNT_FILE_SIZE);
        assert_eq!(s["STRUCT_MOB"], MOB_SIZE);
        assert_eq!(s["STRUCT_MOBEXTRA"], MOBEXTRA_SIZE);
        assert_eq!(s["STRUCT_AFFECT"] * MAX_AFFECT, AFFECT_BYTES);

        let mut trace = Vec::new();
        decode_traced(&vec![0u8; ACCOUNT_FILE_SIZE], Some(&mut trace)).unwrap();
        let got: std::collections::HashMap<String, (usize, usize)> =
            trace.into_iter().map(|(n, o, l)| (n, (o, l))).collect();

        let cpp = cpp_layout();
        let field = |st: &str, f: &str| {
            cpp.iter()
                .find(|(s, n, _, _)| s == st && n == f)
                .map(|(_, _, o, l)| (*o, *l))
                .unwrap()
        };
        let mut expected: Vec<(String, (usize, usize))> = Vec::new();
        for (st, f, o, l) in &cpp {
            match st.as_str() {
                "STRUCT_ACCOUNTINFO" => expected.push((format!("Info.{f}"), (*o, *l))),
                "STRUCT_ACCOUNTFILE" => {
                    let (base, _) = field("STRUCT_ACCOUNTFILE", f);
                    match f.as_str() {
                        "Info" | "Char" | "mobExtra" => {}
                        "ShortSkill" | "affect" => {
                            let each = l / MOB_PER_ACCOUNT;
                            for slot in 0..MOB_PER_ACCOUNT {
                                expected.push((format!("{f}[{slot}]"), (base + slot * each, each)));
                            }
                        }
                        _ => expected.push((f.clone(), (*o, *l))),
                    }
                }
                "STRUCT_MOB" => {
                    let (base, _) = field("STRUCT_ACCOUNTFILE", "Char");
                    for slot in 0..MOB_PER_ACCOUNT {
                        expected.push((format!("Char[{slot}].{f}"), (base + slot * MOB_SIZE + o, *l)));
                    }
                }
                "STRUCT_MOBEXTRA" => {
                    let (base, _) = field("STRUCT_ACCOUNTFILE", "mobExtra");
                    for slot in 0..MOB_PER_ACCOUNT {
                        expected.push((format!("mobExtra[{slot}].{f}"), (base + slot * MOBEXTRA_SIZE + o, *l)));
                    }
                }
                _ => {}
            }
        }
        for (name, want) in &expected {
            assert_eq!(got.get(name), Some(want), "campo {name}");
        }
        assert_eq!(got.len(), expected.len(), "o codec lê campos que o C++ não lista");

        // Estruturas internas, campo a campo.
        let score: Vec<_> = cpp.iter().filter(|(s, ..)| s == "STRUCT_SCORE").collect();
        let mut t = Vec::new();
        quiet_trace(&mut t, rd_score);
        assert_eq!(
            t,
            score.iter().map(|(_, n, o, l)| (n.clone(), *o, *l)).collect::<Vec<_>>()
        );
        let item: Vec<_> = cpp.iter().filter(|(s, ..)| s == "STRUCT_ITEM").collect();
        let mut t = Vec::new();
        quiet_trace(&mut t, rd_item);
        assert_eq!(
            t,
            item.iter().map(|(_, n, o, l)| (n.clone(), *o, *l)).collect::<Vec<_>>()
        );
        assert_eq!(field("STRUCT_AFFECT", "Type"), (0, 1));
        assert_eq!(field("STRUCT_AFFECT", "Level"), (1, 1));
        assert_eq!(field("STRUCT_AFFECT", "Value"), (2, 2));
        assert_eq!(field("STRUCT_AFFECT", "Time"), (4, 4));
    }

    fn quiet_trace<R>(t: &mut Vec<(String, usize, usize)>, f: impl FnOnce(&mut Rd<'_>) -> R) {
        let b = [0u8; 64];
        let mut r = Rd {
            b: &b,
            base: 0,
            off: 0,
            trace: Some(t),
        };
        f(&mut r);
    }

    #[test]
    fn round_trip_random_accounts() {
        let mask = round_tripped_mask();
        for seed in 1..=64u64 {
            let src = random_account(seed.wrapping_mul(0x9E37_79B9_7F4A_7C15), (seed % 4) as usize);
            let (acc, secrets) = decode(&src).unwrap();
            assert_eq!(acc.characters.len(), 3);
            assert_eq!(acc.empty_slots.len(), 1);
            assert_eq!(secrets.password, cstr(&src[16..32]));
            let out = encode(&acc).unwrap();
            for i in 0..ACCOUNT_FILE_SIZE {
                if mask[i] {
                    assert_eq!(out[i], src[i], "semente {seed}: byte {i} mudou");
                } else if !NOT_ROUND_TRIPPED.iter().any(|&(_, o, l)| (o..o + l).contains(&i)) {
                    assert_eq!(out[i], 0, "semente {seed}: padding {i} não ficou zerado");
                }
            }
            // E o proto passa pelo JSON do banco sem perda.
            let json = serde_json::to_value(&acc).unwrap();
            let back: pb::Account = serde_json::from_value(json).unwrap();
            assert_eq!(back, acc);
        }
    }

    #[test]
    fn new_account_marks_numeric_unset() {
        let mut src = vec![0u8; ACCOUNT_FILE_SIZE];
        src[..4].copy_from_slice(b"NOVA");
        src[16..20].copy_from_slice(b"1234");
        src[206] = 0xFF;
        src[OFF_SHORTSKILL..OFF_SHORTSKILL + 8].fill(0xFF);
        let (acc, secrets) = decode(&src).unwrap();
        assert!(secrets.numeric.is_none());
        assert!(!acc.numeric_set);
        assert_eq!(acc.empty_slots.len(), 1);
        let out = encode(&acc).unwrap();
        let mut want = src.clone();
        want[16..32].fill(0);
        assert_eq!(out, want);
    }

    #[test]
    fn cp1252_round_trip() {
        let all: Vec<u8> = (1..=255).collect();
        let s = cp1252_to_string(&all);
        assert_eq!(string_to_cp1252(&s).unwrap(), all);
        assert_eq!(cp1252_to_string(b"Jo\xe3o \x80"), "João €");
        assert!(string_to_cp1252("\u{4e2d}").is_err());
    }

    #[test]
    fn first_key_like_dbsrv() {
        assert_eq!(first_key(b"derust").unwrap(), "d");
        assert_eq!(first_key(b"9abc").unwrap(), "etc");
        assert!(first_key(b"\xb0\xa1").is_err());
    }
}
