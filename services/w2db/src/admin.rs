//! Operações do painel de GM e do bot do Discord (`w2gm`, docs/GM.md): sessões do DBSrv, criação
//! de conta, bloqueio, busca, histórico de item, rollback de personagem, convites, cadastro pelo
//! link de uso único, contas de GM e o registro do que cada um fez (`gm_audit`).

use std::collections::{HashMap, HashSet};

use chrono::{DateTime, Datelike, Utc};
use sha2::{Digest, Sha256};
use sqlx::{PgPool, Postgres, Transaction};

use crate::auth;
use crate::legacy::{self, cp1252_to_string, name_key};
use crate::pb::{self, Status};
use crate::store::{self, Result, SaveMode, SaveOutcome, StoreError};

/// Quanto vale o link do `/registrar`.
pub const REGISTRATION_MINUTES: i64 = 30;
/// Bloqueio "para sempre": a data no banco (o chrono não lê o `infinity` do Postgres).
pub const BAN_FOREVER_YEAR: i32 = 9999;

fn key(name: &[u8]) -> String {
    cp1252_to_string(&name_key(name))
}

/// sha256 de um token de link ou de sessão: só o hash fica no banco.
pub fn token_hash(token: &str) -> Vec<u8> {
    Sha256::digest(token.as_bytes()).to_vec()
}

/// Token aleatório de 256 bits em base64url, para links e sessões.
pub fn new_token() -> anyhow::Result<String> {
    let mut raw = [0u8; 32];
    getrandom::fill(&mut raw).map_err(|e| anyhow::anyhow!("getrandom: {e}"))?;
    Ok(data_encoding::BASE64URL_NOPAD.encode(&raw))
}

// ---------------------------------------------------------------- auditoria

/// Registra uma ação. `actor` é `gm:<nome>`, `discord:<id>` ou `cli`.
pub async fn audit<'e, E>(
    db: E,
    actor: &str,
    action: &str,
    target: Option<&str>,
    detail: serde_json::Value,
) -> Result<()>
where
    E: sqlx::PgExecutor<'e>,
{
    sqlx::query!(
        "INSERT INTO gm_audit (actor, action, target, detail) VALUES ($1, $2, $3, $4)",
        actor,
        action,
        target,
        detail
    )
    .execute(db)
    .await?;
    Ok(())
}

pub struct AuditRow {
    pub at: DateTime<Utc>,
    pub actor: String,
    pub action: String,
    pub target: Option<String>,
    pub detail: serde_json::Value,
}

pub async fn recent_audit(pool: &PgPool, limit: i64) -> Result<Vec<AuditRow>> {
    Ok(sqlx::query_as!(
        AuditRow,
        "SELECT at, actor, action, target, detail FROM gm_audit ORDER BY id DESC LIMIT $1",
        limit
    )
    .fetch_all(pool)
    .await?)
}

// ---------------------------------------------------------------- sessões do DBSrv

/// A conta entrou (`online`) ou saiu da lista do DBSrv. `false` se a conta não existe.
pub async fn set_session(pool: &PgPool, name: &[u8], online: bool) -> Result<bool> {
    let r = sqlx::query!(
        "UPDATE accounts SET online_since = CASE WHEN $2 THEN now() ELSE NULL END WHERE name_key = $1",
        key(name),
        online
    )
    .execute(pool)
    .await?;
    Ok(r.rows_affected() > 0)
}

/// O DBSrv subiu: nenhuma conta está no jogo.
pub async fn reset_sessions(pool: &PgPool) -> Result<u64> {
    Ok(
        sqlx::query!("UPDATE accounts SET online_since = NULL WHERE online_since IS NOT NULL")
            .execute(pool)
            .await?
            .rows_affected(),
    )
}

// ---------------------------------------------------------------- criação de conta

/// A conta nova do DBSrv (`CFileDB::AddAccount`): barra de skills da vaga 0 em -1 e, com bônus,
/// 50 itens 401 e 50 itens 406 no baú.
fn new_account(name: &[u8], bonus: bool) -> pb::Account {
    let item = |index| pb::Item {
        index,
        effects: vec![pb::ItemEffect::default(); 3],
    };
    pb::Account {
        name: name.to_vec(),
        version: 1,
        empty_slots: vec![pb::EmptySlot {
            slot: 0,
            short_skill: vec![0xFF; 8],
            ..Default::default()
        }],
        cargo: if bonus {
            (0..100)
                .map(|i| pb::StoredItem {
                    serial: 0,
                    slot: i,
                    item: Some(item(if i < 50 { 401 } else { 406 })),
                })
                .collect()
        } else {
            Vec::new()
        },
        ..Default::default()
    }
}

/// Cria a conta, com o convite quando há um (ele fica usado e ligado à conta). Devolve
/// `Ok`, `Invalid`, `BadInvite` ou `AccountExists`; num erro nada fica gravado.
pub async fn create_account_in(
    tx: &mut Transaction<'_, Postgres>,
    name: &[u8],
    password_hash: &str,
    bonus: bool,
    invite: Option<&str>,
) -> Result<Status> {
    if !store::valid_name(name) || legacy::first_key(name).is_err() {
        return Ok(Status::Invalid);
    }
    let discord = match invite {
        None => None,
        Some(code) => match store::take_invite(tx, code).await? {
            Some(d) => Some(d),
            None => return Ok(Status::BadInvite),
        },
    };
    let Some(id) = store::create_account(tx, name, password_hash, None, discord.as_deref()).await? else {
        return Ok(Status::AccountExists);
    };
    if let Some(code) = invite {
        store::bind_invite(tx, code, id).await?;
    }
    store::save_account(
        tx,
        id,
        &new_account(name, bonus),
        &[],
        "nova conta",
        false,
        SaveMode::Characters,
    )
    .await?;
    Ok(Status::Ok)
}

// ---------------------------------------------------------------- bloqueio

/// Bloqueio pelo GM ainda valendo: segundos que faltam (`Some(0)` é para sempre).
pub fn ban_left(banned_until: Option<DateTime<Utc>>) -> Option<u32> {
    let until = banned_until?;
    if until.year() >= BAN_FOREVER_YEAR {
        return Some(0);
    }
    let left = (until - Utc::now()).num_seconds();
    (left > 0).then(|| left.min(u32::MAX as i64) as u32)
}

/// Bloqueia a conta até `until` (`None`: para sempre) ou desbloqueia (`lift`). Vale no próximo
/// login: quem está no jogo continua até sair.
pub async fn set_ban(
    pool: &PgPool,
    account: i64,
    until: Option<DateTime<Utc>>,
    lift: bool,
    reason: &str,
    actor: &str,
) -> Result<()> {
    let until = if lift {
        None
    } else {
        Some(until.unwrap_or_else(|| {
            chrono::NaiveDate::from_ymd_opt(BAN_FOREVER_YEAR, 12, 31)
                .unwrap()
                .and_hms_opt(0, 0, 0)
                .unwrap()
                .and_utc()
        }))
    };
    let mut tx = pool.begin().await?;
    let name = sqlx::query_scalar!(
        "UPDATE accounts SET banned_until = $2, ban_reason = $3 WHERE id = $1 RETURNING name",
        account,
        until,
        (!lift).then_some(reason)
    )
    .fetch_optional(&mut *tx)
    .await?
    .ok_or_else(|| StoreError::Invalid("conta inexistente".into()))?;
    audit(
        &mut *tx,
        actor,
        if lift { "unban" } else { "ban" },
        Some(&name),
        serde_json::json!({ "until": until.map(|u| u.to_rfc3339()), "reason": reason }),
    )
    .await?;
    tx.commit().await?;
    Ok(())
}

// ---------------------------------------------------------------- consulta

pub struct AccountHit {
    pub id: i64,
    pub name: String,
    pub characters: Vec<String>,
    pub online: bool,
    pub banned: bool,
}

fn like_prefix(q: &str) -> String {
    let mut s = String::new();
    for c in q.chars() {
        if matches!(c, '%' | '_' | '\\') {
            s.push('\\');
        }
        s.push(c.to_ascii_uppercase());
    }
    s.push('%');
    s
}

/// Contas cujo nome, ou o nome de um personagem, começa com `q` (sem diferença de caixa), ou com
/// este id do Discord.
pub async fn search(pool: &PgPool, q: &str, limit: i64) -> Result<Vec<AccountHit>> {
    let q = q.trim();
    if q.is_empty() {
        return Ok(Vec::new());
    }
    let rows = sqlx::query!(
        r#"SELECT a.id, a.name, a.online_since IS NOT NULL AS "online!",
                  COALESCE(a.banned_until > now(), false) AS "banned!",
                  COALESCE(array_agg(c.name ORDER BY c.slot) FILTER (WHERE c.name IS NOT NULL), '{}') AS "characters!"
           FROM accounts a LEFT JOIN characters c ON c.account_id = a.id
           WHERE a.name_key LIKE $1 OR a.discord_id = $2
              OR a.id IN (SELECT account_id FROM characters WHERE name_key LIKE $1)
           GROUP BY a.id ORDER BY a.name_key LIMIT $3"#,
        like_prefix(q),
        q,
        limit
    )
    .fetch_all(pool)
    .await?;
    Ok(rows
        .into_iter()
        .map(|r| AccountHit {
            id: r.id,
            name: r.name,
            characters: r.characters,
            online: r.online,
            banned: r.banned,
        })
        .collect())
}

pub struct AccountInfo {
    pub id: i64,
    pub name: String,
    pub discord_id: Option<String>,
    pub version: i64,
    pub cargo_coin: i32,
    pub created_at: DateTime<Utc>,
    pub last_login_at: Option<DateTime<Utc>>,
    pub last_logout_at: Option<DateTime<Utc>>,
    pub online_since: Option<DateTime<Utc>>,
    pub banned_until: Option<DateTime<Utc>>,
    pub ban_reason: Option<String>,
    pub locked_until: Option<DateTime<Utc>>,
}

pub struct CharacterRow {
    pub slot: i16,
    pub name: String,
    pub class: i16,
    pub level: i32,
    pub coin: i32,
    pub pos_x: i32,
    pub pos_y: i32,
}

pub struct ItemRow {
    pub serial: i64,
    /// 1 equipamento, 2 inventário, 3 baú.
    pub container: i16,
    pub char_slot: Option<i16>,
    pub slot: i16,
    pub index: i16,
    pub effects: [i16; 6],
    pub contested_from: Option<String>,
}

pub struct VersionRow {
    pub version: i64,
    pub saved_at: DateTime<Utc>,
    /// Personagens da versão (vaga, nome, nível), para escolher para onde voltar.
    pub characters: Vec<(u32, String, i32)>,
}

pub struct AccountView {
    pub info: AccountInfo,
    pub characters: Vec<CharacterRow>,
    pub items: Vec<ItemRow>,
    pub versions: Vec<VersionRow>,
    pub invites: Vec<InviteRow>,
}

pub async fn account_view(pool: &PgPool, id: i64) -> Result<Option<AccountView>> {
    let Some(info) = sqlx::query_as!(
        AccountInfo,
        "SELECT id, name, discord_id, version, cargo_coin, created_at, last_login_at, last_logout_at,
                online_since, banned_until, ban_reason, locked_until
         FROM accounts WHERE id = $1",
        id
    )
    .fetch_optional(pool)
    .await?
    else {
        return Ok(None);
    };
    let characters = sqlx::query_as!(
        CharacterRow,
        "SELECT slot, name, class, level, coin, pos_x, pos_y FROM characters WHERE account_id = $1 ORDER BY slot",
        id
    )
    .fetch_all(pool)
    .await?;
    let items = sqlx::query!(
        r#"SELECT i.serial, i.container, c.slot AS "char_slot?", i.slot AS "slot!", i.item_index,
                  i.ef1, i.ev1, i.ef2, i.ev2, i.ef3, i.ev3, o.name AS "contested_from?"
           FROM items i LEFT JOIN characters c ON c.id = i.character_id
                        LEFT JOIN accounts o ON o.id = i.contested_from
           WHERE i.account_id = $1 AND i.container <> 0
           ORDER BY i.container, c.slot, i.slot"#,
        id
    )
    .fetch_all(pool)
    .await?
    .into_iter()
    .map(|r| ItemRow {
        serial: r.serial,
        container: r.container,
        char_slot: r.char_slot,
        slot: r.slot,
        index: r.item_index,
        effects: [r.ef1, r.ev1, r.ef2, r.ev2, r.ef3, r.ev3],
        contested_from: r.contested_from,
    })
    .collect();
    let mut versions = Vec::new();
    for r in sqlx::query!(
        "SELECT version, saved_at, snapshot FROM account_versions WHERE account_id = $1 ORDER BY version DESC",
        id
    )
    .fetch_all(pool)
    .await?
    {
        let snap: pb::Account = serde_json::from_value(r.snapshot)?;
        versions.push(VersionRow {
            version: r.version,
            saved_at: r.saved_at,
            characters: snap
                .characters
                .iter()
                .map(|c| {
                    let level = c
                        .mob
                        .as_ref()
                        .and_then(|m| m.base_score.as_ref())
                        .map_or(0, |s| s.level);
                    (c.slot, cp1252_to_string(&c.name), level)
                })
                .collect(),
        });
    }
    let invites = sqlx::query_as!(
        InviteRow,
        "SELECT i.code, i.discord_id, i.created_by, i.created_at, i.used_at, i.revoked_at, a.name AS \"account?\"
         FROM invites i LEFT JOIN accounts a ON a.id = i.account_id
         WHERE i.account_id = $1 OR ($2::text IS NOT NULL AND i.discord_id = $2)
         ORDER BY i.created_at DESC",
        id,
        info.discord_id
    )
    .fetch_all(pool)
    .await?;
    Ok(Some(AccountView {
        info,
        characters,
        items,
        versions,
        invites,
    }))
}

pub struct ItemEventRow {
    pub at: DateTime<Utc>,
    pub kind: String,
    pub account: Option<String>,
    pub from_account: Option<String>,
    pub item_index: Option<i16>,
    pub detail: serde_json::Value,
}

pub struct ItemPlace {
    pub account_id: i64,
    pub account: String,
    pub container: i16,
    pub character: Option<String>,
    pub slot: Option<i16>,
    pub index: i16,
}

/// Onde o item está agora (`None` se foi apagado) e o histórico dele, do mais antigo ao mais novo.
/// Os eventos `dupe_suspect` e `restore` que citam o serial no `detail` entram também.
pub async fn item_history(pool: &PgPool, serial: i64) -> Result<(Option<ItemPlace>, Vec<ItemEventRow>)> {
    let place = sqlx::query_as!(
        ItemPlace,
        r#"SELECT i.account_id, a.name AS account, i.container, c.name AS "character?", i.slot, i.item_index AS index
           FROM items i JOIN accounts a ON a.id = i.account_id LEFT JOIN characters c ON c.id = i.character_id
           WHERE i.serial = $1"#,
        serial
    )
    .fetch_optional(pool)
    .await?;
    let events = sqlx::query_as!(
        ItemEventRow,
        r#"SELECT e.at, e.kind, a.name AS "account?", f.name AS "from_account?", e.item_index, e.detail
           FROM item_events e LEFT JOIN accounts a ON a.id = e.account_id LEFT JOIN accounts f ON f.id = e.from_account_id
           WHERE e.serial = $1 OR e.detail->>'original_serial' = $2 OR e.detail->>'old_serial' = $2
           ORDER BY e.id"#,
        serial,
        serial.to_string()
    )
    .fetch_all(pool)
    .await?;
    Ok((place, events))
}

// ---------------------------------------------------------------- rollback

#[derive(Debug, Default)]
pub struct RollbackReport {
    /// A versão nova da conta.
    pub version: u64,
    /// Itens que voltaram com o serial de antes.
    pub restored: usize,
    /// Itens que não existiam mais (consumidos, refinados, vendidos): voltam com serial novo
    /// (serial antigo, serial novo).
    pub recreated: Vec<(u64, u64)>,
    /// Itens que hoje estão com outra conta e não voltam (serial, conta).
    pub elsewhere: Vec<(u64, String)>,
    /// Itens que o personagem tinha e a versão não: ficam sem lugar, no histórico (`remove`).
    pub removed: Vec<u64>,
}

#[derive(Debug)]
pub enum Rollback {
    Done(RollbackReport),
    /// A conta está no jogo (no DBSrv): a gravação seguinte desfaria o rollback.
    Online,
    NoVersion,
    /// O personagem não existe na versão ou não existe mais na conta.
    NoCharacter,
    /// O nome do personagem na versão é hoje de outro personagem.
    NameTaken,
}

/// Volta o personagem da vaga `slot` para como ele estava na versão `version` da conta: dados,
/// equipamento e inventário. O baú e os outros personagens ficam como estão, menos os itens que
/// voltam para este personagem (saem de onde estão). Itens que estão hoje com outra conta não
/// voltam: o relatório diz quais e com quem. Só com a conta fora do jogo.
pub async fn rollback_character(pool: &PgPool, account: i64, slot: u32, version: i64, actor: &str) -> Result<Rollback> {
    let mut tx = pool.begin().await?;
    let Some(row) = sqlx::query!(
        "SELECT name, online_since FROM accounts WHERE id = $1 FOR UPDATE",
        account
    )
    .fetch_optional(&mut *tx)
    .await?
    else {
        return Err(StoreError::Invalid("conta inexistente".into()));
    };
    if row.online_since.is_some() {
        return Ok(Rollback::Online);
    }
    let Some(snapshot) = sqlx::query_scalar!(
        "SELECT snapshot FROM account_versions WHERE account_id = $1 AND version = $2",
        account,
        version
    )
    .fetch_optional(&mut *tx)
    .await?
    else {
        return Ok(Rollback::NoVersion);
    };
    let snapshot: pb::Account = serde_json::from_value(snapshot)?;
    let Some(mut old) = snapshot.characters.into_iter().find(|c| c.slot == slot) else {
        return Ok(Rollback::NoCharacter);
    };
    let mut acc = store::load_account(&mut tx, account).await?;
    let Some(pos) = acc.characters.iter().position(|c| c.slot == slot) else {
        return Ok(Rollback::NoCharacter);
    };

    // De quem é hoje cada serial da versão.
    let serials: Vec<i64> = old
        .equip
        .iter()
        .chain(&old.carry)
        .map(|i| i.serial as i64)
        .filter(|s| *s != 0)
        .collect();
    let owners: HashMap<i64, (i64, String)> = sqlx::query!(
        "SELECT i.serial, i.account_id, a.name FROM items i JOIN accounts a ON a.id = i.account_id
         WHERE i.serial = ANY($1) FOR UPDATE OF i",
        &serials
    )
    .fetch_all(&mut *tx)
    .await?
    .into_iter()
    .map(|r| (r.serial, (r.account_id, r.name)))
    .collect();

    let mut report = RollbackReport::default();
    // (container, vaga) -> serial antigo dos itens que voltam com serial novo.
    let mut recreate: HashMap<(i32, u32), u64> = HashMap::new();
    for (container, list) in [
        (pb::Container::Equip, &mut old.equip),
        (pb::Container::Carry, &mut old.carry),
    ] {
        list.retain_mut(|it| match owners.get(&(it.serial as i64)) {
            Some((owner, _)) if *owner == account => true,
            Some((_, other)) => {
                report.elsewhere.push((it.serial, other.clone()));
                false
            }
            None => {
                if it.serial != 0 {
                    recreate.insert((container as i32, it.slot), it.serial);
                }
                it.serial = 0;
                true
            }
        });
    }
    let back: HashSet<u64> = old
        .equip
        .iter()
        .chain(&old.carry)
        .map(|i| i.serial)
        .filter(|s| *s != 0)
        .collect();
    report.restored = back.len();
    report.removed = acc.characters[pos]
        .equip
        .iter()
        .chain(&acc.characters[pos].carry)
        .map(|i| i.serial)
        .filter(|s| !back.contains(s))
        .collect();
    // Os itens que voltam saem de onde estão hoje (baú, outro personagem, outra vaga).
    for c in acc.characters.iter_mut() {
        c.equip.retain(|i| !back.contains(&i.serial));
        c.carry.retain(|i| !back.contains(&i.serial));
    }
    acc.cargo.retain(|i| !back.contains(&i.serial));
    acc.characters[pos] = old;

    let reason = format!("rollback para a versão {version} ({actor})");
    let saved = match store::save_account(&mut tx, account, &acc, &[], &reason, false, SaveMode::Game).await? {
        SaveOutcome::Saved(r) => r,
        SaveOutcome::NameTaken => return Ok(Rollback::NameTaken),
        SaveOutcome::VersionConflict(_) => {
            return Err(StoreError::Invalid("conflito de versão com a conta travada".into()));
        }
    };
    report.version = saved.version;
    for a in &saved.assigned {
        if a.character_slot == slot
            && let Some(&old_serial) = recreate.get(&(a.container, a.slot))
        {
            report.recreated.push((old_serial, a.serial));
            sqlx::query!(
                "INSERT INTO item_events (serial, kind, account_id, item_index, detail)
                 SELECT serial, 'restore', account_id, item_index, $2 FROM items WHERE serial = $1",
                a.serial as i64,
                serde_json::json!({ "old_serial": old_serial.to_string(), "reason": reason })
            )
            .execute(&mut *tx)
            .await?;
        }
    }
    audit(
        &mut *tx,
        actor,
        "rollback",
        Some(&row.name),
        serde_json::json!({
            "slot": slot, "to_version": version, "new_version": report.version,
            "restored": report.restored, "recreated": report.recreated,
            "elsewhere": report.elsewhere, "removed": report.removed,
        }),
    )
    .await?;
    tx.commit().await?;
    Ok(Rollback::Done(report))
}

// ---------------------------------------------------------------- convites e cadastro

pub struct InviteRow {
    pub code: String,
    pub discord_id: String,
    pub created_by: String,
    pub created_at: DateTime<Utc>,
    pub used_at: Option<DateTime<Utc>>,
    pub revoked_at: Option<DateTime<Utc>>,
    pub account: Option<String>,
}

/// Convite para um id do Discord. Devolve o código.
pub async fn create_invite(pool: &PgPool, discord_id: &str, actor: &str) -> Result<String> {
    if discord_id.is_empty() || discord_id.len() > 32 || !discord_id.bytes().all(|b| b.is_ascii_digit()) {
        return Err(StoreError::Invalid("id do Discord inválido".into()));
    }
    let code = auth::invite_code().map_err(|e| StoreError::Invalid(e.to_string()))?;
    let mut tx = pool.begin().await?;
    sqlx::query!(
        "INSERT INTO invites (code, discord_id, created_by) VALUES ($1, $2, $3)",
        code,
        discord_id,
        actor
    )
    .execute(&mut *tx)
    .await?;
    audit(
        &mut *tx,
        actor,
        "invite",
        Some(discord_id),
        serde_json::json!({ "code": code }),
    )
    .await?;
    tx.commit().await?;
    Ok(code)
}

pub async fn list_invites(pool: &PgPool, limit: i64) -> Result<Vec<InviteRow>> {
    Ok(sqlx::query_as!(
        InviteRow,
        "SELECT i.code, i.discord_id, i.created_by, i.created_at, i.used_at, i.revoked_at, a.name AS \"account?\"
         FROM invites i LEFT JOIN accounts a ON a.id = i.account_id
         ORDER BY i.created_at DESC LIMIT $1",
        limit
    )
    .fetch_all(pool)
    .await?)
}

/// Cancela um convite ainda não usado. `false` se ele não existe ou já foi usado ou cancelado.
pub async fn revoke_invite(pool: &PgPool, code: &str, actor: &str) -> Result<bool> {
    let mut tx = pool.begin().await?;
    let who = sqlx::query_scalar!(
        "UPDATE invites SET revoked_at = now() WHERE code = $1 AND used_at IS NULL AND revoked_at IS NULL
         RETURNING discord_id",
        code
    )
    .fetch_optional(&mut *tx)
    .await?;
    let Some(who) = who else {
        return Ok(false);
    };
    audit(
        &mut *tx,
        actor,
        "revoke_invite",
        Some(&who),
        serde_json::json!({ "code": code }),
    )
    .await?;
    tx.commit().await?;
    Ok(true)
}

pub enum Registration {
    /// O token do link (vai na URL; o banco só guarda o hash).
    Link(String),
    /// Não há convite aberto para este id do Discord.
    NoInvite,
}

/// O `/registrar` do bot: um link de uso único, válido por [`REGISTRATION_MINUTES`], para o
/// convite aberto mais antigo deste id do Discord.
pub async fn start_registration(pool: &PgPool, discord_id: &str) -> Result<Registration> {
    let Some(code) = sqlx::query_scalar!(
        "SELECT code FROM invites WHERE discord_id = $1 AND used_at IS NULL AND revoked_at IS NULL
         ORDER BY created_at LIMIT 1",
        discord_id
    )
    .fetch_optional(pool)
    .await?
    else {
        return Ok(Registration::NoInvite);
    };
    let token = new_token().map_err(|e| StoreError::Invalid(e.to_string()))?;
    sqlx::query!(
        "INSERT INTO registrations (token_hash, invite_code, expires_at)
         VALUES ($1, $2, now() + make_interval(mins => $3))",
        token_hash(&token),
        code,
        REGISTRATION_MINUTES as i32
    )
    .execute(pool)
    .await?;
    Ok(Registration::Link(token))
}

/// O link ainda vale: o id do Discord do convite.
pub async fn registration_discord(pool: &PgPool, token: &str) -> Result<Option<String>> {
    Ok(sqlx::query_scalar!(
        "SELECT i.discord_id FROM registrations r JOIN invites i ON i.code = r.invite_code
         WHERE r.token_hash = $1 AND r.used_at IS NULL AND r.expires_at > now()
           AND i.used_at IS NULL AND i.revoked_at IS NULL",
        token_hash(token)
    )
    .fetch_optional(pool)
    .await?)
}

#[derive(Debug, PartialEq, Eq)]
pub enum Registered {
    Created,
    /// Link vencido, usado ou de convite cancelado.
    BadLink,
    InvalidName,
    InvalidPassword,
    NameTaken,
}

/// Cria a conta pelo link do `/registrar`. O link e o convite ficam usados.
pub async fn complete_registration(pool: &PgPool, token: &str, name: &[u8], password: &[u8]) -> Result<Registered> {
    if !store::valid_name(name) || legacy::first_key(name).is_err() || !name.iter().all(|b| b.is_ascii_alphanumeric()) {
        return Ok(Registered::InvalidName);
    }
    if !store::valid_secret(password) {
        return Ok(Registered::InvalidPassword);
    }
    let hash = auth::hash_async(password.to_vec())
        .await
        .map_err(|e| StoreError::Invalid(e.to_string()))?;
    let mut tx = pool.begin().await?;
    let Some(code) = sqlx::query_scalar!(
        "UPDATE registrations SET used_at = now()
         WHERE token_hash = $1 AND used_at IS NULL AND expires_at > now() RETURNING invite_code",
        token_hash(token)
    )
    .fetch_optional(&mut *tx)
    .await?
    else {
        return Ok(Registered::BadLink);
    };
    let revoked = sqlx::query_scalar!("SELECT revoked_at FROM invites WHERE code = $1", code)
        .fetch_one(&mut *tx)
        .await?;
    if revoked.is_some() {
        return Ok(Registered::BadLink);
    }
    let out = match create_account_in(&mut tx, name, &hash, false, Some(&code)).await? {
        Status::Ok => Registered::Created,
        Status::BadInvite => Registered::BadLink,
        Status::AccountExists => Registered::NameTaken,
        _ => Registered::InvalidName,
    };
    if out != Registered::Created {
        return Ok(out);
    }
    let discord = sqlx::query_scalar!("SELECT discord_id FROM invites WHERE code = $1", code)
        .fetch_one(&mut *tx)
        .await?;
    audit(
        &mut *tx,
        &format!("discord:{discord}"),
        "register",
        Some(&cp1252_to_string(name)),
        serde_json::json!({ "invite": code }),
    )
    .await?;
    tx.commit().await?;
    Ok(Registered::Created)
}

// ---------------------------------------------------------------- contas de GM

pub struct GmUser {
    pub id: i64,
    pub name: String,
    pub password_hash: String,
    pub totp_secret: Vec<u8>,
    pub totp_last_step: i64,
    pub locked_until: Option<DateTime<Utc>>,
    pub disabled: bool,
}

pub fn valid_gm_name(name: &str) -> bool {
    (3..=32).contains(&name.len())
        && name
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b == b'_' || b == b'-')
}

/// Cria a conta de GM (ou troca a senha e o segredo do TOTP de uma que existe, e a reativa).
pub async fn gm_upsert(pool: &PgPool, name: &str, password_hash: &str, totp_secret: &[u8]) -> Result<()> {
    if !valid_gm_name(name) {
        return Err(StoreError::Invalid("nome de GM: 3 a 32 letras, números, _ ou -".into()));
    }
    let mut tx = pool.begin().await?;
    let id = sqlx::query_scalar!(
        "INSERT INTO gm_users (name, password_hash, totp_secret) VALUES ($1, $2, $3)
         ON CONFLICT (name) DO UPDATE SET password_hash = EXCLUDED.password_hash,
           totp_secret = EXCLUDED.totp_secret, totp_last_step = 0, failed_logins = 0, locked_until = NULL,
           disabled = false
         RETURNING id",
        name,
        password_hash,
        totp_secret
    )
    .fetch_one(&mut *tx)
    .await?;
    sqlx::query!("DELETE FROM gm_sessions WHERE gm_id = $1", id)
        .execute(&mut *tx)
        .await?;
    audit(&mut *tx, "cli", "gm_upsert", Some(name), serde_json::json!({})).await?;
    tx.commit().await?;
    Ok(())
}

/// Desativa a conta de GM e derruba as sessões dela. `false` se ela não existe.
pub async fn gm_disable(pool: &PgPool, name: &str) -> Result<bool> {
    let mut tx = pool.begin().await?;
    let Some(id) = sqlx::query_scalar!("UPDATE gm_users SET disabled = true WHERE name = $1 RETURNING id", name)
        .fetch_optional(&mut *tx)
        .await?
    else {
        return Ok(false);
    };
    sqlx::query!("DELETE FROM gm_sessions WHERE gm_id = $1", id)
        .execute(&mut *tx)
        .await?;
    audit(&mut *tx, "cli", "gm_disable", Some(name), serde_json::json!({})).await?;
    tx.commit().await?;
    Ok(true)
}

pub async fn gm_by_name(pool: &PgPool, name: &str) -> Result<Option<GmUser>> {
    Ok(sqlx::query_as!(
        GmUser,
        "SELECT id, name, password_hash, totp_secret, totp_last_step, locked_until, disabled
         FROM gm_users WHERE name = $1",
        name
    )
    .fetch_optional(pool)
    .await?)
}

/// Senha ou código errado: depois de [`auth::MAX_FAILURES`] seguidos, a conta de GM fica
/// bloqueada por [`auth::LOCK_SECONDS`].
pub async fn gm_login_failed(pool: &PgPool, id: i64) -> Result<()> {
    sqlx::query!(
        "UPDATE gm_users SET
           failed_logins = CASE WHEN failed_logins + 1 >= $2 THEN 0 ELSE failed_logins + 1 END,
           locked_until = CASE WHEN failed_logins + 1 >= $2 THEN now() + make_interval(secs => $3) ELSE locked_until END
         WHERE id = $1",
        id,
        auth::MAX_FAILURES,
        auth::LOCK_SECONDS as f64
    )
    .execute(pool)
    .await?;
    Ok(())
}

/// Login de GM aceito no passo `step` do TOTP. `false` se o passo já foi usado (o mesmo código
/// duas vezes) ou a conta foi desativada no meio-tempo.
pub async fn gm_login_ok(pool: &PgPool, id: i64, step: i64) -> Result<bool> {
    let r = sqlx::query!(
        "UPDATE gm_users SET totp_last_step = $2, failed_logins = 0, locked_until = NULL
         WHERE id = $1 AND totp_last_step < $2 AND NOT disabled",
        id,
        step
    )
    .execute(pool)
    .await?;
    Ok(r.rows_affected() == 1)
}

pub struct GmSession {
    pub gm_id: i64,
    pub name: String,
    pub csrf: String,
}

pub async fn session_create(pool: &PgPool, gm_id: i64, token: &str, csrf: &str, hours: i32) -> Result<()> {
    sqlx::query!(
        "INSERT INTO gm_sessions (token_hash, gm_id, csrf, expires_at) VALUES ($1, $2, $3, now() + make_interval(hours => $4))",
        token_hash(token),
        gm_id,
        csrf,
        hours
    )
    .execute(pool)
    .await?;
    sqlx::query!("DELETE FROM gm_sessions WHERE expires_at < now()")
        .execute(pool)
        .await?;
    Ok(())
}

pub async fn session_get(pool: &PgPool, token: &str) -> Result<Option<GmSession>> {
    Ok(sqlx::query_as!(
        GmSession,
        "SELECT s.gm_id, g.name, s.csrf FROM gm_sessions s JOIN gm_users g ON g.id = s.gm_id
         WHERE s.token_hash = $1 AND s.expires_at > now() AND NOT g.disabled",
        token_hash(token)
    )
    .fetch_optional(pool)
    .await?)
}

pub async fn session_delete(pool: &PgPool, token: &str) -> Result<()> {
    sqlx::query!("DELETE FROM gm_sessions WHERE token_hash = $1", token_hash(token))
        .execute(pool)
        .await?;
    Ok(())
}
