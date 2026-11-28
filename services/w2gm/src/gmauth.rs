//! Login de GM: nome, senha (argon2id, como as contas do jogo) e código TOTP (RFC 6238: SHA-1,
//! 6 dígitos, passos de 30 s, um passo de folga). O mesmo código não entra duas vezes
//! (`gm_users.totp_last_step`), e 5 erros seguidos bloqueiam a conta de GM por 15 minutos.

use sqlx::PgPool;
use totp_rs::{Builder, Totp};

use w2db::admin;
use w2db::auth;

/// Horas de uma sessão do painel.
pub const SESSION_HOURS: i32 = 12;
/// Bytes do segredo do TOTP (160 bits, o recomendado pela RFC 4226).
pub const SECRET_BYTES: usize = 20;

pub fn totp(secret: &[u8]) -> anyhow::Result<Totp> {
    Builder::new()
        .with_secret(secret.to_vec())
        .build()
        .map_err(|e| anyhow::anyhow!("totp: {e:?}"))
}

pub fn new_secret() -> anyhow::Result<Vec<u8>> {
    let mut s = vec![0u8; SECRET_BYTES];
    getrandom::fill(&mut s).map_err(|e| anyhow::anyhow!("getrandom: {e}"))?;
    Ok(s)
}

/// O endereço `otpauth://` para o aplicativo autenticador (FreeOTP, Aegis, Google Authenticator).
pub fn otpauth_uri(name: &str, secret: &[u8]) -> String {
    let b32 = data_encoding::BASE32_NOPAD.encode(secret);
    format!("otpauth://totp/derust:{name}?secret={b32}&issuer=derust&algorithm=SHA1&digits=6&period=30")
}

/// Cria (ou recria) a conta de GM. Devolve o segredo do TOTP.
pub async fn add_gm(pool: &PgPool, name: &str, password: &str) -> anyhow::Result<Vec<u8>> {
    if password.len() < 12 {
        anyhow::bail!("a senha de GM precisa de pelo menos 12 caracteres");
    }
    let hash = auth::hash_async(password.as_bytes().to_vec()).await?;
    let secret = new_secret()?;
    admin::gm_upsert(pool, name, &hash, &secret).await?;
    Ok(secret)
}

#[derive(Debug, PartialEq, Eq)]
pub enum Login {
    /// O id da conta de GM.
    Ok(i64),
    /// Nome, senha ou código errado (a resposta não diz qual).
    Denied,
    /// Bloqueada por erros seguidos.
    Locked,
}

/// Confere nome, senha e código. `now`: segundos Unix (o teste passa o seu).
pub async fn check_login(pool: &PgPool, name: &str, password: &str, code: &str, now: u64) -> anyhow::Result<Login> {
    let Some(gm) = admin::gm_by_name(pool, name).await? else {
        // O mesmo tempo da senha errada, para a resposta não dizer quais nomes existem.
        auth::verify_async(password.as_bytes().to_vec(), auth::dummy_hash().to_string()).await;
        return Ok(Login::Denied);
    };
    if gm.disabled {
        auth::verify_async(password.as_bytes().to_vec(), auth::dummy_hash().to_string()).await;
        return Ok(Login::Denied);
    }
    if gm.locked_until.is_some_and(|t| t > chrono::Utc::now()) {
        return Ok(Login::Locked);
    }
    let password_ok = auth::verify_async(password.as_bytes().to_vec(), gm.password_hash.clone()).await;
    let step = totp(&gm.totp_secret)?.check(code.trim(), now);
    match step {
        Some(step) if password_ok && admin::gm_login_ok(pool, gm.id, step as i64).await? => Ok(Login::Ok(gm.id)),
        _ => {
            admin::gm_login_failed(pool, gm.id).await?;
            Ok(Login::Denied)
        }
    }
}

/// Comparação sem atalho (o token de CSRF).
pub fn same(a: &str, b: &str) -> bool {
    a.len() == b.len() && a.bytes().zip(b.bytes()).fold(0u8, |acc, (x, y)| acc | (x ^ y)) == 0
}
