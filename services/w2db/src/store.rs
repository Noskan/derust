//! Acesso ao Postgres: conta inteira para dentro e para fora do banco, com os seriais dos itens.
//!
//! A gravação ([`save_account`]) é uma transação com a linha da conta travada:
//! 1. confere a versão esperada;
//! 2. grava os personagens (o que não é coluna vai para `characters.data`);
//! 3. põe cada item na vaga pelo serial (veja [`place_items`]) e registra os eventos;
//! 4. sobe a versão e guarda a conta gravada em `account_versions` (as últimas [`KEEP_VERSIONS`]).

use std::collections::{HashMap, HashSet};

use serde::{Deserialize, Serialize};
use sqlx::{PgPool, Postgres, Transaction};

use crate::legacy::{self, cp1252_to_string, name_key};
use crate::pb;

/// Versões guardadas por conta em `account_versions`.
pub const KEEP_VERSIONS: i64 = 20;

#[derive(Debug, thiserror::Error)]
pub enum StoreError {
    #[error(transparent)]
    Db(#[from] sqlx::Error),
    #[error("{0}")]
    Invalid(String),
    #[error(transparent)]
    Json(#[from] serde_json::Error),
}

impl From<legacy::LegacyError> for StoreError {
    fn from(e: legacy::LegacyError) -> Self {
        StoreError::Invalid(e.to_string())
    }
}

pub type Result<T> = std::result::Result<T, StoreError>;

/// O que vai em `characters.data`.
#[derive(Serialize, Deserialize, Default)]
#[serde(default)]
struct CharData {
    mob: pb::Mob,
    extra: pb::MobExtra,
    affects: Vec<pb::Affect>,
    short_skill: Vec<u8>,
}

/// Campos de autenticação da conta.
pub struct AccountAuth {
    pub id: i64,
    pub password_hash: String,
    pub numeric_hash: Option<String>,
    pub failed_logins: i32,
    pub locked_until: Option<chrono::DateTime<chrono::Utc>>,
    pub banned_until: Option<chrono::DateTime<chrono::Utc>>,
}

fn text(name: &[u8]) -> String {
    cp1252_to_string(name)
}

fn key(name: &[u8]) -> String {
    cp1252_to_string(&name_key(name))
}

/// Nome de conta ou personagem aceito pelo banco: 1..15 bytes, sem zero.
pub fn valid_name(name: &[u8]) -> bool {
    !name.is_empty() && name.len() < legacy::NAME_LENGTH && !name.contains(&0)
}

/// Senha de conta e senha numérica: 1..15 bytes sem zero (cabem no campo de 16 com terminador).
pub fn valid_secret(s: &[u8]) -> bool {
    !s.is_empty() && s.len() < legacy::ACCOUNTPASS_LENGTH && !s.contains(&0)
}

// ---------------------------------------------------------------- contas

pub async fn account_auth(pool: &PgPool, name: &[u8]) -> Result<Option<AccountAuth>> {
    let row = sqlx::query!(
        "SELECT id, password_hash, numeric_hash, failed_logins, locked_until, banned_until
         FROM accounts WHERE name_key = $1",
        key(name)
    )
    .fetch_optional(pool)
    .await?;
    Ok(row.map(|r| AccountAuth {
        id: r.id,
        password_hash: r.password_hash,
        numeric_hash: r.numeric_hash,
        failed_logins: r.failed_logins,
        locked_until: r.locked_until,
        banned_until: r.banned_until,
    }))
}

/// Cria a conta vazia. `false` se o nome já existe.
pub async fn create_account(
    tx: &mut Transaction<'_, Postgres>,
    name: &[u8],
    password_hash: &str,
    numeric_hash: Option<&str>,
    discord_id: Option<&str>,
) -> Result<Option<i64>> {
    let id = sqlx::query_scalar!(
        "INSERT INTO accounts (name, name_key, password_hash, numeric_hash, discord_id)
         VALUES ($1, $2, $3, $4, $5) ON CONFLICT (name_key) DO NOTHING RETURNING id",
        text(name),
        key(name),
        password_hash,
        numeric_hash,
        discord_id
    )
    .fetch_optional(&mut **tx)
    .await?;
    Ok(id)
}

pub async fn record_login_failure(pool: &PgPool, id: i64, max_failures: i32, lock_secs: i64) -> Result<()> {
    sqlx::query!(
        "UPDATE accounts SET
           failed_logins = CASE WHEN failed_logins + 1 >= $2 THEN 0 ELSE failed_logins + 1 END,
           locked_until = CASE WHEN failed_logins + 1 >= $2 THEN now() + make_interval(secs => $3) ELSE locked_until END
         WHERE id = $1",
        id,
        max_failures,
        lock_secs as f64
    )
    .execute(pool)
    .await?;
    Ok(())
}

pub async fn record_login_success(pool: &PgPool, id: i64) -> Result<()> {
    sqlx::query!(
        "UPDATE accounts SET failed_logins = 0, locked_until = NULL, last_login_at = now() WHERE id = $1",
        id
    )
    .execute(pool)
    .await?;
    Ok(())
}

pub async fn set_password(pool: &PgPool, id: i64, hash: &str) -> Result<()> {
    sqlx::query!(
        "UPDATE accounts SET password_hash = $2, updated_at = now() WHERE id = $1",
        id,
        hash
    )
    .execute(pool)
    .await?;
    Ok(())
}

pub async fn set_numeric(pool: &PgPool, id: i64, hash: &str) -> Result<()> {
    sqlx::query!(
        "UPDATE accounts SET numeric_hash = $2, updated_at = now() WHERE id = $1",
        id,
        hash
    )
    .execute(pool)
    .await?;
    Ok(())
}

pub async fn account_id(pool: &PgPool, name: &[u8]) -> Result<Option<i64>> {
    Ok(
        sqlx::query_scalar!("SELECT id FROM accounts WHERE name_key = $1", key(name))
            .fetch_optional(pool)
            .await?,
    )
}

pub async fn all_account_names(pool: &PgPool) -> Result<Vec<String>> {
    Ok(sqlx::query_scalar!("SELECT name FROM accounts ORDER BY name_key")
        .fetch_all(pool)
        .await?)
}

// ---------------------------------------------------------------- leitura da conta

fn item_from_row(index: i16, e: [i16; 6]) -> pb::Item {
    pb::Item {
        index: index as i32,
        effects: (0..3)
            .map(|i| pb::ItemEffect {
                effect: e[i * 2] as u32,
                value: e[i * 2 + 1] as u32,
            })
            .collect(),
    }
}

/// A conta inteira, com os itens nas vagas e os seriais.
pub async fn load_account(conn: &mut sqlx::PgConnection, id: i64) -> Result<pb::Account> {
    let a = sqlx::query!(
        "SELECT name, version, cargo_coin, donate, year, year_day, temp_key, empty_slots,
                numeric_hash IS NOT NULL AS \"numeric_set!\"
         FROM accounts WHERE id = $1",
        id
    )
    .fetch_one(&mut *conn)
    .await?;
    let chars = sqlx::query!(
        "SELECT id, slot, name, data FROM characters WHERE account_id = $1 ORDER BY slot",
        id
    )
    .fetch_all(&mut *conn)
    .await?;
    let items = sqlx::query!(
        "SELECT serial, character_id, container, slot AS \"slot!\", item_index, ef1, ev1, ef2, ev2, ef3, ev3
         FROM items WHERE account_id = $1 AND container <> 0 ORDER BY container, character_id, slot",
        id
    )
    .fetch_all(&mut *conn)
    .await?;

    let mut by_char: HashMap<i64, pb::Character> = HashMap::new();
    let mut order = Vec::new();
    for c in chars {
        let data: CharData = serde_json::from_value(c.data)?;
        order.push(c.id);
        by_char.insert(
            c.id,
            pb::Character {
                slot: c.slot as u32,
                name: legacy::string_to_cp1252(&c.name)?,
                mob: Some(data.mob),
                equip: Vec::new(),
                carry: Vec::new(),
                affects: data.affects,
                short_skill: data.short_skill,
                extra: Some(data.extra),
            },
        );
    }
    let mut cargo = Vec::new();
    for it in items {
        let stored = pb::StoredItem {
            serial: it.serial as u64,
            slot: it.slot as u32,
            item: Some(item_from_row(
                it.item_index,
                [it.ef1, it.ev1, it.ef2, it.ev2, it.ef3, it.ev3],
            )),
        };
        match (it.container, it.character_id.and_then(|c| by_char.get_mut(&c))) {
            (1, Some(c)) => c.equip.push(stored),
            (2, Some(c)) => c.carry.push(stored),
            (3, _) => cargo.push(stored),
            _ => return Err(StoreError::Invalid(format!("item {} em lugar inválido", it.serial))),
        }
    }
    Ok(pb::Account {
        name: legacy::string_to_cp1252(&a.name)?,
        version: a.version as u64,
        cargo_coin: a.cargo_coin,
        donate: a.donate,
        year: a.year,
        year_day: a.year_day,
        temp_key: a.temp_key,
        cargo,
        characters: order.into_iter().filter_map(|id| by_char.remove(&id)).collect(),
        empty_slots: serde_json::from_value(a.empty_slots)?,
        numeric_set: a.numeric_set,
    })
}

pub async fn load_account_pool(pool: &PgPool, id: i64) -> Result<pb::Account> {
    let mut conn = pool.acquire().await?;
    load_account(&mut conn, id).await
}

// ---------------------------------------------------------------- gravação

/// Como a gravação trata itens e personagens.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum SaveMode {
    /// Gravação do jogo: personagem que sumiu da conta é erro (apagar é com DeleteCharacter).
    Game,
    /// Importação de arquivo: todos os itens são novos (evento `import`).
    Import,
    /// CreateCharacter e DeleteCharacter: o conjunto de personagens muda.
    Characters,
}

pub enum SaveOutcome {
    Saved(pb::SaveAccountReply),
    VersionConflict(u64),
    NameTaken,
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
struct Loc {
    container: i16,
    char_slot: i16,
    slot: i16,
}

struct Placement<'a> {
    loc: Loc,
    serial: u64,
    item: &'a pb::Item,
}

static EMPTY_ITEM: std::sync::LazyLock<pb::Item> = std::sync::LazyLock::new(pb::Item::default);

fn push_placements<'a>(
    out: &mut Vec<Placement<'a>>,
    container: i16,
    char_slot: i16,
    max: usize,
    items: &'a [pb::StoredItem],
    what: &str,
) -> Result<()> {
    for it in items {
        if it.slot as usize >= max {
            return Err(StoreError::Invalid(format!(
                "{what}: vaga {} fora de 0..{max}",
                it.slot
            )));
        }
        let item = it.item.as_ref().unwrap_or(&EMPTY_ITEM);
        if item.effects.len() > 3 {
            return Err(StoreError::Invalid(format!(
                "{what}: item com {} efeitos",
                item.effects.len()
            )));
        }
        out.push(Placement {
            loc: Loc {
                container,
                char_slot,
                slot: it.slot as i16,
            },
            serial: it.serial,
            item,
        });
    }
    Ok(())
}

fn placements(acc: &pb::Account) -> Result<Vec<Placement<'_>>> {
    let mut out = Vec::new();
    for c in &acc.characters {
        push_placements(&mut out, 1, c.slot as i16, legacy::MAX_EQUIP, &c.equip, "equip")?;
        push_placements(&mut out, 2, c.slot as i16, legacy::MAX_CARRY, &c.carry, "carry")?;
    }
    push_placements(&mut out, 3, -1, legacy::MAX_CARGO, &acc.cargo, "cargo")?;
    let mut seen = HashSet::new();
    for p in &out {
        if !seen.insert(p.loc) {
            return Err(StoreError::Invalid(format!("vaga repetida {:?}", p.loc)));
        }
    }
    Ok(out)
}

fn effects(item: &pb::Item) -> [i16; 6] {
    let mut e = [0i16; 6];
    for (i, ef) in item.effects.iter().take(3).enumerate() {
        e[i * 2] = (ef.effect & 0xFF) as i16;
        e[i * 2 + 1] = (ef.value & 0xFF) as i16;
    }
    e
}

fn container_pb(c: i16) -> i32 {
    match c {
        1 => pb::Container::Equip as i32,
        2 => pb::Container::Carry as i32,
        _ => pb::Container::Cargo as i32,
    }
}

struct Event {
    serial: i64,
    kind: &'static str,
    from_account: Option<i64>,
    item_index: i16,
    detail: serde_json::Value,
}

/// Grava a conta `id`. `acc.version` é a versão esperada (ignorada no modo `Import`).
pub async fn save_account(
    tx: &mut Transaction<'_, Postgres>,
    id: i64,
    acc: &pb::Account,
    destroyed: &[u64],
    reason: &str,
    logout: bool,
    mode: SaveMode,
) -> Result<SaveOutcome> {
    let current = sqlx::query_scalar!("SELECT version FROM accounts WHERE id = $1 FOR UPDATE", id)
        .fetch_one(&mut **tx)
        .await?;
    if mode != SaveMode::Import && acc.version as i64 != current {
        return Ok(SaveOutcome::VersionConflict(current as u64));
    }

    // Personagens.
    let mut slots = HashSet::new();
    for c in &acc.characters {
        if c.slot as usize >= legacy::MOB_PER_ACCOUNT || !slots.insert(c.slot) {
            return Err(StoreError::Invalid(format!(
                "vaga de personagem {} inválida ou repetida",
                c.slot
            )));
        }
        if !valid_name(&c.name) {
            return Err(StoreError::Invalid("nome de personagem inválido".into()));
        }
    }
    let existing = sqlx::query!("SELECT id, slot FROM characters WHERE account_id = $1", id)
        .fetch_all(&mut **tx)
        .await?;
    for e in &existing {
        if !slots.contains(&(e.slot as u32)) {
            if mode == SaveMode::Game {
                return Err(StoreError::Invalid(format!(
                    "o personagem da vaga {} sumiu da conta",
                    e.slot
                )));
            }
            sqlx::query!("DELETE FROM characters WHERE id = $1", e.id)
                .execute(&mut **tx)
                .await?;
        }
    }
    let mut char_ids: HashMap<i16, i64> = HashMap::new();
    for c in &acc.characters {
        let mob = c.mob.clone().unwrap_or_default();
        let extra = c.extra.clone().unwrap_or_default();
        let score = mob.base_score.clone().unwrap_or_default();
        let (class, level, exp, coin, guild, guild_level, x, y) = (
            mob.class as i16,
            score.level,
            mob.exp,
            mob.coin,
            mob.guild as i32,
            mob.guild_level as i16,
            mob.home_town_x,
            mob.home_town_y,
        );
        let class_master = extra.class_master as i16;
        let data = serde_json::to_value(CharData {
            mob,
            extra,
            affects: c.affects.clone(),
            short_skill: c.short_skill.clone(),
        })?;
        let row = sqlx::query_scalar!(
            "INSERT INTO characters (account_id, slot, name, name_key, class, class_master, level, exp, coin,
                                     guild, guild_level, pos_x, pos_y, data)
             VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)
             ON CONFLICT (account_id, slot) DO UPDATE SET
               name = EXCLUDED.name, name_key = EXCLUDED.name_key, class = EXCLUDED.class,
               class_master = EXCLUDED.class_master, level = EXCLUDED.level, exp = EXCLUDED.exp,
               coin = EXCLUDED.coin, guild = EXCLUDED.guild, guild_level = EXCLUDED.guild_level,
               pos_x = EXCLUDED.pos_x, pos_y = EXCLUDED.pos_y, data = EXCLUDED.data, updated_at = now()
             RETURNING id",
            id,
            c.slot as i16,
            text(&c.name),
            key(&c.name),
            class,
            class_master,
            level,
            exp,
            coin,
            guild,
            guild_level,
            x,
            y,
            data
        )
        .fetch_one(&mut **tx)
        .await;
        match row {
            Ok(cid) => {
                char_ids.insert(c.slot as i16, cid);
            }
            Err(sqlx::Error::Database(e)) if e.constraint() == Some("characters_name_key_key") => {
                return Ok(SaveOutcome::NameTaken);
            }
            Err(e) => return Err(e.into()),
        }
    }

    let (assigned, dupes) = place_items(tx, id, acc, destroyed, reason, mode, &char_ids).await?;

    let empty_slots = serde_json::to_value(&acc.empty_slots)?;
    let version = sqlx::query_scalar!(
        "UPDATE accounts SET cargo_coin = $2, donate = $3, year = $4, year_day = $5, temp_key = $6,
                empty_slots = $7, version = version + 1, updated_at = now(),
                last_logout_at = CASE WHEN $8 THEN now() ELSE last_logout_at END
         WHERE id = $1 RETURNING version",
        id,
        acc.cargo_coin,
        acc.donate,
        acc.year,
        acc.year_day,
        &acc.temp_key,
        empty_slots,
        logout
    )
    .fetch_one(&mut **tx)
    .await?;

    // A conta como ficou, para desfazer depois.
    let saved = load_account(tx, id).await?;
    sqlx::query!(
        "INSERT INTO account_versions (account_id, version, snapshot) VALUES ($1, $2, $3)",
        id,
        version,
        serde_json::to_value(&saved)?
    )
    .execute(&mut **tx)
    .await?;
    sqlx::query!(
        "DELETE FROM account_versions WHERE account_id = $1 AND version <= $2",
        id,
        version - KEEP_VERSIONS
    )
    .execute(&mut **tx)
    .await?;

    Ok(SaveOutcome::Saved(pb::SaveAccountReply {
        status: pb::Status::Ok as i32,
        version: version as u64,
        assigned,
        dupe_suspects: dupes,
    }))
}

/// Põe os itens da conta nas vagas pelo serial:
/// - serial 0: item novo, ganha serial (`create`, ou `import` na importação);
/// - serial desta conta: vai para a vaga (`move` se mudou de lugar);
/// - serial de outra conta, sem lugar lá (saiu numa troca, por exemplo): muda de dono (`transfer`);
/// - serial numa vaga viva de outra conta: também muda de dono (`contest`), e a conta que o perdeu
///   fica marcada em `contested_from`. É a troca em que o outro lado ainda não gravou (nas trocas
///   de mão dupla, um dos lados sempre grava primeiro). A marca sai quando aquela conta grava sem o
///   serial;
/// - serial que veio de uma conta marcada (ela grava de novo com o item que perdeu), de um item já
///   disputado, repetido nesta gravação ou que não existe no banco: suspeita de duplicação. O item
///   fica, com serial novo, e o evento `dupe_suspect` guarda o serial original;
/// - serial desta conta que não veio e não está em `destroyed`: fica sem lugar (`remove`);
/// - serial em `destroyed`: apagado (`destroy`).
async fn place_items(
    tx: &mut Transaction<'_, Postgres>,
    id: i64,
    acc: &pb::Account,
    destroyed: &[u64],
    reason: &str,
    mode: SaveMode,
    char_ids: &HashMap<i16, i64>,
) -> Result<(Vec<pb::AssignedSerial>, Vec<u64>)> {
    let wanted = placements(acc)?;
    let mut events: Vec<Event> = Vec::new();

    // Itens vivos desta conta, com o lugar de antes.
    let before = sqlx::query!(
        "SELECT i.serial, i.container, i.slot, i.item_index, c.slot AS \"char_slot?\"
         FROM items i LEFT JOIN characters c ON c.id = i.character_id
         WHERE i.account_id = $1",
        id
    )
    .fetch_all(&mut **tx)
    .await?;
    let mut own: HashMap<i64, (Option<Loc>, i16)> = before
        .iter()
        .map(|r| {
            let loc = (r.container != 0).then(|| Loc {
                container: r.container,
                char_slot: if r.container == 3 {
                    -1
                } else {
                    r.char_slot.unwrap_or(-1)
                },
                slot: r.slot.unwrap_or(0),
            });
            (r.serial, (loc, r.item_index))
        })
        .collect();

    // Destruídos de propósito.
    let destroyed: Vec<i64> = destroyed
        .iter()
        .map(|&s| s as i64)
        .filter(|s| own.contains_key(s))
        .collect();
    if !destroyed.is_empty() {
        sqlx::query!("DELETE FROM items WHERE serial = ANY($1)", &destroyed)
            .execute(&mut **tx)
            .await?;
        for s in &destroyed {
            let (_, idx) = own.remove(s).unwrap();
            events.push(Event {
                serial: *s,
                kind: "destroy",
                from_account: None,
                item_index: idx,
                detail: serde_json::json!({ "reason": reason }),
            });
        }
    }

    // Libera as vagas: tudo desta conta fica sem lugar e volta para a vaga certa abaixo.
    sqlx::query!(
        "UPDATE items SET container = 0, slot = NULL, character_id = NULL WHERE account_id = $1 AND container <> 0",
        id
    )
    .execute(&mut **tx)
    .await?;

    // Seriais de outras contas pedidos nesta gravação.
    let foreign: Vec<i64> = wanted
        .iter()
        .map(|p| p.serial as i64)
        .filter(|s| *s != 0 && !own.contains_key(s))
        .collect();
    let others: HashMap<i64, (i64, i16, Option<i64>)> = sqlx::query!(
        "SELECT serial, account_id, container, contested_from FROM items WHERE serial = ANY($1) FOR UPDATE",
        &foreign
    )
    .fetch_all(&mut **tx)
    .await?
    .into_iter()
    .map(|r| (r.serial, (r.account_id, r.container, r.contested_from)))
    .collect();
    // Serial que mudou de dono nesta gravação -> nova marca de disputa (None: sem disputa).
    let mut contest: Vec<(i64, Option<i64>)> = Vec::new();

    let mut used: HashSet<i64> = HashSet::new();
    let mut moves: Vec<(&Placement<'_>, i64)> = Vec::new();
    let mut inserts: Vec<(&Placement<'_>, Option<(i64, &'static str)>)> = Vec::new();
    for p in &wanted {
        let s = p.serial as i64;
        if s == 0 || mode == SaveMode::Import {
            inserts.push((p, None));
        } else if used.contains(&s) {
            inserts.push((p, Some((s, "repetido na gravação"))));
        } else if let Some((old, idx)) = own.get(&s) {
            used.insert(s);
            if *old != Some(p.loc) {
                events.push(Event {
                    serial: s,
                    kind: "move",
                    from_account: None,
                    item_index: *idx,
                    detail: serde_json::json!({ "from": old.map(|l| [l.container, l.char_slot, l.slot]), "to": [p.loc.container, p.loc.char_slot, p.loc.slot] }),
                });
            }
            moves.push((p, s));
        } else if let Some(&(other, container, contested)) = others.get(&s) {
            if container == 0 {
                used.insert(s);
                contest.push((s, None));
                events.push(Event {
                    serial: s,
                    kind: "transfer",
                    from_account: Some(other),
                    item_index: p.item.index as i16,
                    detail: serde_json::json!({ "reason": reason }),
                });
                moves.push((p, s));
            } else if contested.is_none() {
                used.insert(s);
                contest.push((s, Some(other)));
                events.push(Event {
                    serial: s,
                    kind: "contest",
                    from_account: Some(other),
                    item_index: p.item.index as i16,
                    detail: serde_json::json!({ "reason": reason }),
                });
                moves.push((p, s));
            } else if contested == Some(id) {
                inserts.push((p, Some((s, "a conta grava de novo um item que passou para outra"))));
            } else {
                inserts.push((p, Some((s, "item já disputado por outra conta"))));
            }
        } else {
            inserts.push((p, Some((s, "serial inexistente"))));
        }
    }

    // Itens que ficam (desta conta ou recebidos): vaga, dono e conteúdo.
    if !moves.is_empty() {
        let mut serial = Vec::new();
        let mut character = Vec::new();
        let mut container = Vec::new();
        let mut slot = Vec::new();
        let mut index = Vec::new();
        let mut ef: [Vec<i16>; 6] = Default::default();
        for (p, s) in &moves {
            serial.push(*s);
            character.push(if p.loc.container == 3 {
                None
            } else {
                char_ids.get(&p.loc.char_slot).copied()
            });
            container.push(p.loc.container);
            slot.push(p.loc.slot);
            index.push(p.item.index as i16);
            for (i, v) in effects(p.item).into_iter().enumerate() {
                ef[i].push(v);
            }
        }
        sqlx::query!(
            "UPDATE items SET account_id = $1, character_id = u.character_id, container = u.container, slot = u.slot,
                    item_index = u.item_index, ef1 = u.ef1, ev1 = u.ev1, ef2 = u.ef2, ev2 = u.ev2, ef3 = u.ef3,
                    ev3 = u.ev3, updated_at = now()
             FROM UNNEST($2::bigint[], $3::bigint[], $4::smallint[], $5::smallint[], $6::smallint[], $7::smallint[],
                         $8::smallint[], $9::smallint[], $10::smallint[], $11::smallint[], $12::smallint[])
                  AS u(serial, character_id, container, slot, item_index, ef1, ev1, ef2, ev2, ef3, ev3)
             WHERE items.serial = u.serial",
            id,
            &serial,
            &character as &[Option<i64>],
            &container,
            &slot,
            &index,
            &ef[0],
            &ef[1],
            &ef[2],
            &ef[3],
            &ef[4],
            &ef[5]
        )
        .execute(&mut **tx)
        .await?;
    }

    if !contest.is_empty() {
        let serial: Vec<i64> = contest.iter().map(|c| c.0).collect();
        let from: Vec<Option<i64>> = contest.iter().map(|c| c.1).collect();
        sqlx::query!(
            "UPDATE items SET contested_from = u.contested_from
             FROM UNNEST($1::bigint[], $2::bigint[]) AS u(serial, contested_from)
             WHERE items.serial = u.serial",
            &serial,
            &from as &[Option<i64>]
        )
        .execute(&mut **tx)
        .await?;
    }
    // Esta conta gravou sem os itens que perdeu numa disputa: confirma a perda.
    let kept: Vec<i64> = wanted.iter().map(|p| p.serial as i64).filter(|s| *s != 0).collect();
    sqlx::query!(
        "UPDATE items SET contested_from = NULL WHERE contested_from = $1 AND NOT (serial = ANY($2))",
        id,
        &kept
    )
    .execute(&mut **tx)
    .await?;

    // Itens novos.
    let mut assigned = Vec::new();
    let mut dupes = Vec::new();
    if !inserts.is_empty() {
        let mut character = Vec::new();
        let mut container = Vec::new();
        let mut slot = Vec::new();
        let mut index = Vec::new();
        let mut ef: [Vec<i16>; 6] = Default::default();
        for (p, _) in &inserts {
            character.push(if p.loc.container == 3 {
                None
            } else {
                char_ids.get(&p.loc.char_slot).copied()
            });
            container.push(p.loc.container);
            slot.push(p.loc.slot);
            index.push(p.item.index as i16);
            for (i, v) in effects(p.item).into_iter().enumerate() {
                ef[i].push(v);
            }
        }
        let rows = sqlx::query!(
            "INSERT INTO items (account_id, character_id, container, slot, item_index, ef1, ev1, ef2, ev2, ef3, ev3)
             SELECT $1, * FROM UNNEST($2::bigint[], $3::smallint[], $4::smallint[], $5::smallint[], $6::smallint[],
                                      $7::smallint[], $8::smallint[], $9::smallint[], $10::smallint[], $11::smallint[])
             RETURNING serial, character_id, container, slot AS \"slot!\"",
            id,
            &character as &[Option<i64>],
            &container,
            &slot,
            &index,
            &ef[0],
            &ef[1],
            &ef[2],
            &ef[3],
            &ef[4],
            &ef[5]
        )
        .fetch_all(&mut **tx)
        .await?;
        let char_slot: HashMap<i64, i16> = char_ids.iter().map(|(s, c)| (*c, *s)).collect();
        let by_loc: HashMap<Loc, i64> = rows
            .iter()
            .map(|r| {
                let cs = if r.container == 3 {
                    -1
                } else {
                    r.character_id.and_then(|c| char_slot.get(&c).copied()).unwrap_or(-1)
                };
                (
                    Loc {
                        container: r.container,
                        char_slot: cs,
                        slot: r.slot,
                    },
                    r.serial,
                )
            })
            .collect();
        for (p, why) in &inserts {
            let serial = by_loc[&p.loc];
            assigned.push(pb::AssignedSerial {
                container: container_pb(p.loc.container),
                character_slot: p.loc.char_slot.max(0) as u32,
                slot: p.loc.slot as u32,
                serial: serial as u64,
            });
            match why {
                None => events.push(Event {
                    serial,
                    kind: if mode == SaveMode::Import { "import" } else { "create" },
                    from_account: None,
                    item_index: p.item.index as i16,
                    detail: serde_json::json!({ "reason": reason }),
                }),
                Some((original, what)) => {
                    dupes.push(serial as u64);
                    let other = others.get(original).map(|o| o.0);
                    events.push(Event {
                        serial,
                        kind: "dupe_suspect",
                        from_account: other,
                        item_index: p.item.index as i16,
                        detail: serde_json::json!({ "original_serial": original, "why": what, "reason": reason }),
                    });
                }
            }
        }
    }

    // Desta conta, não vieram e não foram destruídos: ficam sem lugar.
    for (s, (old, idx)) in &own {
        if !used.contains(s) && old.is_some() {
            events.push(Event {
                serial: *s,
                kind: "remove",
                from_account: None,
                item_index: *idx,
                detail: serde_json::json!({ "reason": reason }),
            });
        }
    }

    if !events.is_empty() {
        let serial: Vec<i64> = events.iter().map(|e| e.serial).collect();
        let kind: Vec<String> = events.iter().map(|e| e.kind.to_string()).collect();
        let from: Vec<Option<i64>> = events.iter().map(|e| e.from_account).collect();
        let index: Vec<i16> = events.iter().map(|e| e.item_index).collect();
        let detail: Vec<serde_json::Value> = events.into_iter().map(|e| e.detail).collect();
        sqlx::query!(
            "INSERT INTO item_events (serial, kind, account_id, from_account_id, item_index, detail)
             SELECT u.serial, u.kind, $1, u.from_account, u.item_index, u.detail
             FROM UNNEST($2::bigint[], $3::text[], $4::bigint[], $5::smallint[], $6::jsonb[])
                  AS u(serial, kind, from_account, item_index, detail)",
            id,
            &serial,
            &kind,
            &from as &[Option<i64>],
            &index,
            &detail
        )
        .execute(&mut **tx)
        .await?;
    }

    Ok((assigned, dupes))
}

// ---------------------------------------------------------------- personagens

pub async fn find_character(pool: &PgPool, name: &[u8]) -> Result<Option<(String, i16)>> {
    let r = sqlx::query!(
        "SELECT a.name, c.slot FROM characters c JOIN accounts a ON a.id = c.account_id WHERE c.name_key = $1",
        key(name)
    )
    .fetch_optional(pool)
    .await?;
    Ok(r.map(|r| (r.name, r.slot)))
}

/// Apaga os itens do personagem (evento `delete_character`) antes de a gravação tirar a vaga.
pub async fn destroy_character_items(tx: &mut Transaction<'_, Postgres>, id: i64, slot: u32) -> Result<()> {
    sqlx::query!(
        "WITH gone AS (
           DELETE FROM items i USING characters c
           WHERE c.account_id = $1 AND c.slot = $2 AND i.character_id = c.id
           RETURNING i.serial, i.item_index)
         INSERT INTO item_events (serial, kind, account_id, item_index)
         SELECT serial, 'delete_character', $1, item_index FROM gone",
        id,
        slot as i16
    )
    .execute(&mut **tx)
    .await?;
    Ok(())
}

// ---------------------------------------------------------------- comunidade

pub async fn create_invite(pool: &PgPool, code: &str, discord_id: &str) -> Result<()> {
    sqlx::query!(
        "INSERT INTO invites (code, discord_id) VALUES ($1, $2)",
        code,
        discord_id
    )
    .execute(pool)
    .await?;
    Ok(())
}

/// Marca o convite como usado. Devolve o id do Discord, ou `None` se o código não vale.
pub async fn take_invite(tx: &mut Transaction<'_, Postgres>, code: &str) -> Result<Option<String>> {
    Ok(sqlx::query_scalar!(
        "UPDATE invites SET used_at = now() WHERE code = $1 AND used_at IS NULL RETURNING discord_id",
        code
    )
    .fetch_optional(&mut **tx)
    .await?)
}

pub async fn bind_invite(tx: &mut Transaction<'_, Postgres>, code: &str, account: i64) -> Result<()> {
    sqlx::query!("UPDATE invites SET account_id = $2 WHERE code = $1", code, account)
        .execute(&mut **tx)
        .await?;
    Ok(())
}

/// Relatos de bug por personagem na última hora (além do intervalo entre relatos no TMSrv).
pub const BUG_LIMIT_PER_HOUR: i64 = 5;

/// Grava o relato do `/bug`. `None` se o personagem já mandou [`BUG_LIMIT_PER_HOUR`] relatos na
/// última hora.
pub async fn report_bug(pool: &PgPool, r: &pb::ReportBugRequest) -> Result<Option<i64>> {
    let account = account_id(pool, &r.account_name).await?;
    let name = text(&r.character_name);
    let mut tx = pool.begin().await?;
    // Dois relatos do mesmo personagem ao mesmo tempo não passam juntos do limite.
    sqlx::query("SELECT pg_advisory_xact_lock(hashtext('bug_reports'), hashtext($1))")
        .bind(&name)
        .execute(&mut *tx)
        .await?;
    let recent = sqlx::query_scalar!(
        r#"SELECT count(*) AS "n!" FROM bug_reports
           WHERE character_name = $1 AND created_at > now() - interval '1 hour'"#,
        name
    )
    .fetch_one(&mut *tx)
    .await?;
    if recent >= BUG_LIMIT_PER_HOUR {
        return Ok(None);
    }
    let id = sqlx::query_scalar!(
        "INSERT INTO bug_reports (account_id, character_name, server, pos_x, pos_y, text)
         VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
        account,
        name,
        r.server,
        r.pos_x,
        r.pos_y,
        cp1252_to_string(&r.text)
    )
    .fetch_one(&mut *tx)
    .await?;
    tx.commit().await?;
    Ok(Some(id))
}

/// Um relato de bug na fila do Discord.
#[derive(Debug, Clone)]
pub struct BugPost {
    pub id: i64,
    pub account: Option<String>,
    pub character: String,
    pub server: i32,
    pub x: i32,
    pub y: i32,
    pub text: String,
}

/// Tentativas de postar um relato antes de desistir dele (fica no banco, fora da fila).
pub const BUG_POST_ATTEMPTS: i32 = 10;

/// Os relatos ainda não postados, do mais antigo.
pub async fn pending_bug_posts(pool: &PgPool, limit: i64) -> Result<Vec<BugPost>> {
    let rows = sqlx::query!(
        "SELECT b.id, a.name AS \"account?\", b.character_name, b.server, b.pos_x, b.pos_y, b.text
         FROM bug_reports b LEFT JOIN accounts a ON a.id = b.account_id
         WHERE b.posted_at IS NULL AND b.post_attempts < $1
         ORDER BY b.id LIMIT $2",
        BUG_POST_ATTEMPTS,
        limit
    )
    .fetch_all(pool)
    .await?;
    Ok(rows
        .into_iter()
        .map(|r| BugPost {
            id: r.id,
            account: r.account,
            character: r.character_name,
            server: r.server,
            x: r.pos_x,
            y: r.pos_y,
            text: r.text,
        })
        .collect())
}

/// Relatos na fila do Discord.
pub async fn bug_queue_len(pool: &PgPool) -> Result<i64> {
    Ok(sqlx::query_scalar!(
        r#"SELECT count(*) AS "n!" FROM bug_reports WHERE posted_at IS NULL AND post_attempts < $1"#,
        BUG_POST_ATTEMPTS
    )
    .fetch_one(pool)
    .await?)
}

pub async fn mark_bug_posted(pool: &PgPool, id: i64) -> Result<()> {
    sqlx::query!("UPDATE bug_reports SET posted_at = now() WHERE id = $1", id)
        .execute(pool)
        .await?;
    Ok(())
}

pub async fn mark_bug_post_failed(pool: &PgPool, id: i64) -> Result<()> {
    sqlx::query!(
        "UPDATE bug_reports SET post_attempts = post_attempts + 1 WHERE id = $1",
        id
    )
    .execute(pool)
    .await?;
    Ok(())
}
