//! O serviço gRPC (`w2db.v1.W2Db`) sobre o [`store`].

use sqlx::PgPool;
use tonic::{Request, Response, Status as GrpcStatus};

use crate::pb::{self, Status, w2_db_server::W2Db};
use crate::store::{self, SaveMode, SaveOutcome, StoreError};
use crate::{auth, legacy};

pub struct Service {
    pool: PgPool,
    /// Conta nova só com convite (o teste fechado).
    require_invite: bool,
}

impl Service {
    pub fn new(pool: PgPool, require_invite: bool) -> Self {
        Service { pool, require_invite }
    }
}

type Rsp<T> = Result<Response<T>, GrpcStatus>;

fn internal(e: impl std::fmt::Display) -> GrpcStatus {
    tracing::error!("{e}");
    GrpcStatus::internal("erro interno do w2db")
}

fn store_err(e: StoreError) -> GrpcStatus {
    match e {
        StoreError::Invalid(m) => GrpcStatus::invalid_argument(m),
        e => internal(e),
    }
}

fn status(s: Status) -> Response<pb::StatusReply> {
    Response::new(pb::StatusReply { status: s as i32 })
}

fn save_reply(o: SaveOutcome) -> pb::SaveAccountReply {
    match o {
        SaveOutcome::Saved(r) => r,
        SaveOutcome::VersionConflict(v) => pb::SaveAccountReply {
            status: Status::VersionConflict as i32,
            version: v,
            ..Default::default()
        },
        SaveOutcome::NameTaken => pb::SaveAccountReply {
            status: Status::NameTaken as i32,
            ..Default::default()
        },
    }
}

fn simple_save(s: Status) -> Response<pb::SaveAccountReply> {
    Response::new(pb::SaveAccountReply {
        status: s as i32,
        ..Default::default()
    })
}

/// Senha de conta e senha numérica: 1..15 bytes sem zero (cabem no campo de 16 com terminador).
fn valid_secret(s: &[u8]) -> bool {
    !s.is_empty() && s.len() < legacy::ACCOUNTPASS_LENGTH && !s.contains(&0)
}

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

enum Auth {
    Ok(store::AccountAuth),
    Fail(Status, u32),
}

impl Service {
    /// Confere bloqueio e segredo (senha ou senha numérica) e conta a tentativa.
    async fn check_secret(&self, name: &[u8], secret: &[u8], numeric: bool) -> Result<Auth, GrpcStatus> {
        let Some(a) = store::account_auth(&self.pool, name).await.map_err(store_err)? else {
            auth::verify_async(secret.to_vec(), auth::dummy_hash().to_string()).await;
            return Ok(Auth::Fail(Status::NoAccount, 0));
        };
        if let Some(until) = a.locked_until {
            let left = (until - chrono::Utc::now()).num_seconds();
            if left > 0 {
                return Ok(Auth::Fail(Status::Locked, left as u32));
            }
        }
        let hash = if numeric {
            match &a.numeric_hash {
                Some(h) => h.clone(),
                None => return Ok(Auth::Fail(Status::NumericNotSet, 0)),
            }
        } else {
            a.password_hash.clone()
        };
        if !auth::verify_async(secret.to_vec(), hash).await {
            store::record_login_failure(&self.pool, a.id, auth::MAX_FAILURES, auth::LOCK_SECONDS)
                .await
                .map_err(store_err)?;
            return Ok(Auth::Fail(Status::WrongPassword, 0));
        }
        store::record_login_success(&self.pool, a.id).await.map_err(store_err)?;
        Ok(Auth::Ok(a))
    }

    async fn id_of(&self, name: &[u8]) -> Result<Option<i64>, GrpcStatus> {
        store::account_id(&self.pool, name).await.map_err(store_err)
    }
}

#[tonic::async_trait]
impl W2Db for Service {
    async fn health(&self, _: Request<pb::HealthRequest>) -> Rsp<pb::HealthReply> {
        let ok = sqlx::query_scalar!("SELECT 1 AS \"one!\"")
            .fetch_one(&self.pool)
            .await
            .is_ok();
        Ok(Response::new(pb::HealthReply {
            database_ok: ok,
            version: env!("CARGO_PKG_VERSION").to_string(),
        }))
    }

    async fn create_account(&self, req: Request<pb::CreateAccountRequest>) -> Rsp<pb::CreateAccountReply> {
        let r = req.into_inner();
        let reply = |s: Status| Ok(Response::new(pb::CreateAccountReply { status: s as i32 }));
        if !store::valid_name(&r.name) || !valid_secret(&r.password) || legacy::first_key(&r.name).is_err() {
            return reply(Status::Invalid);
        }
        if self.require_invite && r.invite_code.is_empty() {
            return reply(Status::InviteRequired);
        }
        let hash = auth::hash_async(r.password.clone()).await.map_err(internal)?;
        let mut tx = self.pool.begin().await.map_err(internal)?;
        let discord = if r.invite_code.is_empty() {
            None
        } else {
            match store::take_invite(&mut tx, &r.invite_code).await.map_err(store_err)? {
                Some(d) => Some(d),
                None => return reply(Status::BadInvite),
            }
        };
        let Some(id) = store::create_account(&mut tx, &r.name, &hash, None, discord.as_deref())
            .await
            .map_err(store_err)?
        else {
            return reply(Status::AccountExists);
        };
        if !r.invite_code.is_empty() {
            store::bind_invite(&mut tx, &r.invite_code, id)
                .await
                .map_err(store_err)?;
        }
        let acc = new_account(&r.name, r.bonus);
        store::save_account(&mut tx, id, &acc, &[], "nova conta", false, SaveMode::Characters)
            .await
            .map_err(store_err)?;
        tx.commit().await.map_err(internal)?;
        reply(Status::Ok)
    }

    async fn account_login(&self, req: Request<pb::AccountLoginRequest>) -> Rsp<pb::AccountLoginReply> {
        let r = req.into_inner();
        match self.check_secret(&r.name, &r.password, false).await? {
            Auth::Fail(s, lock) => {
                if s == Status::WrongPassword || s == Status::Locked {
                    tracing::info!(ip = %r.ip, account = %legacy::cp1252_to_string(&r.name), "login recusado: {s:?}");
                }
                Ok(Response::new(pb::AccountLoginReply {
                    status: s as i32,
                    account: None,
                    lock_seconds: lock,
                }))
            }
            Auth::Ok(a) => {
                let account = store::load_account_pool(&self.pool, a.id).await.map_err(store_err)?;
                Ok(Response::new(pb::AccountLoginReply {
                    status: Status::Ok as i32,
                    account: Some(account),
                    lock_seconds: 0,
                }))
            }
        }
    }

    async fn load_account(&self, req: Request<pb::LoadAccountRequest>) -> Rsp<pb::LoadAccountReply> {
        let r = req.into_inner();
        let Some(id) = self.id_of(&r.name).await? else {
            return Ok(Response::new(pb::LoadAccountReply {
                status: Status::NoAccount as i32,
                account: None,
            }));
        };
        let account = store::load_account_pool(&self.pool, id).await.map_err(store_err)?;
        Ok(Response::new(pb::LoadAccountReply {
            status: Status::Ok as i32,
            account: Some(account),
        }))
    }

    async fn set_password(&self, req: Request<pb::SetPasswordRequest>) -> Rsp<pb::StatusReply> {
        let r = req.into_inner();
        if !valid_secret(&r.password) {
            return Ok(status(Status::Invalid));
        }
        let Some(id) = self.id_of(&r.name).await? else {
            return Ok(status(Status::NoAccount));
        };
        let hash = auth::hash_async(r.password).await.map_err(internal)?;
        store::set_password(&self.pool, id, &hash).await.map_err(store_err)?;
        Ok(status(Status::Ok))
    }

    async fn set_numeric_password(&self, req: Request<pb::SetNumericPasswordRequest>) -> Rsp<pb::StatusReply> {
        let r = req.into_inner();
        if r.numeric.is_empty() || r.numeric.len() > 6 || r.numeric.contains(&0) {
            return Ok(status(Status::Invalid));
        }
        let Some(id) = self.id_of(&r.name).await? else {
            return Ok(status(Status::NoAccount));
        };
        let hash = auth::hash_async(r.numeric).await.map_err(internal)?;
        store::set_numeric(&self.pool, id, &hash).await.map_err(store_err)?;
        Ok(status(Status::Ok))
    }

    async fn check_numeric_password(&self, req: Request<pb::CheckNumericPasswordRequest>) -> Rsp<pb::StatusReply> {
        let r = req.into_inner();
        Ok(status(match self.check_secret(&r.name, &r.numeric, true).await? {
            Auth::Ok(_) => Status::Ok,
            Auth::Fail(s, _) => s,
        }))
    }

    async fn save_account(&self, req: Request<pb::SaveAccountRequest>) -> Rsp<pb::SaveAccountReply> {
        let r = req.into_inner();
        let acc = r.account.unwrap_or_default();
        let Some(id) = self.id_of(&acc.name).await? else {
            return Ok(simple_save(Status::NoAccount));
        };
        let mut tx = self.pool.begin().await.map_err(internal)?;
        let out = store::save_account(&mut tx, id, &acc, &r.destroyed, &r.reason, r.logout, SaveMode::Game)
            .await
            .map_err(store_err)?;
        if matches!(out, SaveOutcome::Saved(_)) {
            tx.commit().await.map_err(internal)?;
        }
        Ok(Response::new(save_reply(out)))
    }

    async fn create_character(&self, req: Request<pb::CreateCharacterRequest>) -> Rsp<pb::SaveAccountReply> {
        let r = req.into_inner();
        let ch = r.character.unwrap_or_default();
        let Some(id) = self.id_of(&r.account_name).await? else {
            return Ok(simple_save(Status::NoAccount));
        };
        let mut tx = self.pool.begin().await.map_err(internal)?;
        let current = sqlx::query_scalar!("SELECT version FROM accounts WHERE id = $1 FOR UPDATE", id)
            .fetch_one(&mut *tx)
            .await
            .map_err(internal)?;
        if current as u64 != r.expected_version {
            return Ok(Response::new(save_reply(SaveOutcome::VersionConflict(current as u64))));
        }
        let mut acc = store::load_account(&mut tx, id).await.map_err(store_err)?;
        if ch.slot as usize >= legacy::MOB_PER_ACCOUNT {
            return Ok(simple_save(Status::Invalid));
        }
        if acc.characters.iter().any(|c| c.slot == ch.slot) {
            return Ok(simple_save(Status::SlotTaken));
        }
        acc.empty_slots.retain(|e| e.slot != ch.slot);
        acc.characters.push(ch);
        acc.characters.sort_by_key(|c| c.slot);
        let out = store::save_account(&mut tx, id, &acc, &[], "novo personagem", false, SaveMode::Characters)
            .await
            .map_err(store_err)?;
        if matches!(out, SaveOutcome::Saved(_)) {
            tx.commit().await.map_err(internal)?;
        }
        Ok(Response::new(save_reply(out)))
    }

    async fn delete_character(&self, req: Request<pb::DeleteCharacterRequest>) -> Rsp<pb::SaveAccountReply> {
        let r = req.into_inner();
        let Some(id) = self.id_of(&r.account_name).await? else {
            return Ok(simple_save(Status::NoAccount));
        };
        let mut tx = self.pool.begin().await.map_err(internal)?;
        let current = sqlx::query_scalar!("SELECT version FROM accounts WHERE id = $1 FOR UPDATE", id)
            .fetch_one(&mut *tx)
            .await
            .map_err(internal)?;
        if current as u64 != r.expected_version {
            return Ok(Response::new(save_reply(SaveOutcome::VersionConflict(current as u64))));
        }
        let mut acc = store::load_account(&mut tx, id).await.map_err(store_err)?;
        if !acc.characters.iter().any(|c| c.slot == r.slot) {
            return Ok(simple_save(Status::SlotEmpty));
        }
        store::destroy_character_items(&mut tx, id, r.slot)
            .await
            .map_err(store_err)?;
        acc.characters.retain(|c| c.slot != r.slot);
        if let Some(mut residue) = r.residue {
            residue.slot = r.slot;
            if !(residue.mob.is_empty()
                && residue.short_skill.is_empty()
                && residue.affects.is_empty()
                && residue.extra.is_empty())
            {
                acc.empty_slots.push(residue);
            }
        }
        let out = store::save_account(
            &mut tx,
            id,
            &acc,
            &[],
            "personagem apagado",
            false,
            SaveMode::Characters,
        )
        .await
        .map_err(store_err)?;
        if matches!(out, SaveOutcome::Saved(_)) {
            tx.commit().await.map_err(internal)?;
        }
        Ok(Response::new(save_reply(out)))
    }

    async fn find_character(&self, req: Request<pb::FindCharacterRequest>) -> Rsp<pb::FindCharacterReply> {
        let r = req.into_inner();
        Ok(Response::new(
            match store::find_character(&self.pool, &r.name).await.map_err(store_err)? {
                Some((account, slot)) => pb::FindCharacterReply {
                    status: Status::Ok as i32,
                    account_name: legacy::string_to_cp1252(&account).map_err(internal)?,
                    slot: slot as u32,
                },
                None => pb::FindCharacterReply {
                    status: Status::NoAccount as i32,
                    ..Default::default()
                },
            },
        ))
    }

    async fn create_invite(&self, req: Request<pb::CreateInviteRequest>) -> Rsp<pb::CreateInviteReply> {
        let r = req.into_inner();
        if r.discord_id.is_empty() {
            return Err(GrpcStatus::invalid_argument("discord_id vazio"));
        }
        let code = auth::invite_code().map_err(internal)?;
        store::create_invite(&self.pool, &code, &r.discord_id)
            .await
            .map_err(store_err)?;
        Ok(Response::new(pb::CreateInviteReply { code }))
    }

    async fn report_bug(&self, req: Request<pb::ReportBugRequest>) -> Rsp<pb::ReportBugReply> {
        let r = req.into_inner();
        if r.text.trim().is_empty() || r.text.len() > 2000 {
            return Ok(Response::new(pb::ReportBugReply {
                status: Status::Invalid as i32,
                id: 0,
            }));
        }
        let id = store::report_bug(&self.pool, &r).await.map_err(store_err)?;
        Ok(Response::new(pb::ReportBugReply {
            status: Status::Ok as i32,
            id,
        }))
    }
}
