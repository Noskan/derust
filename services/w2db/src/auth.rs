//! Senhas e senhas numéricas em argon2id (parâmetros padrão do crate: m = 19 MiB, t = 2, p = 1,
//! a recomendação da OWASP). O hash é lento de propósito: rode em `spawn_blocking`.

use argon2::Argon2;
use argon2::password_hash::{PasswordHasher, PasswordVerifier, phc::PasswordHash};

/// Senhas erradas seguidas até bloquear a conta.
pub const MAX_FAILURES: i32 = 5;
/// Tempo de bloqueio depois de [`MAX_FAILURES`] senhas erradas.
pub const LOCK_SECONDS: i64 = 15 * 60;

pub fn hash(secret: &[u8]) -> anyhow::Result<String> {
    Ok(Argon2::default()
        .hash_password(secret)
        .map_err(|e| anyhow::anyhow!("argon2: {e}"))?
        .to_string())
}

pub fn verify(secret: &[u8], hash: &str) -> bool {
    match PasswordHash::new(hash) {
        Ok(parsed) => Argon2::default().verify_password(secret, &parsed).is_ok(),
        Err(_) => false,
    }
}

/// Hash de uma senha qualquer, para a conta inexistente gastar o mesmo tempo que a errada.
pub fn dummy_hash() -> &'static str {
    static DUMMY: std::sync::LazyLock<String> =
        std::sync::LazyLock::new(|| hash(b"w2db-dummy-password").expect("argon2"));
    &DUMMY
}

pub async fn hash_async(secret: Vec<u8>) -> anyhow::Result<String> {
    tokio::task::spawn_blocking(move || hash(&secret)).await?
}

pub async fn verify_async(secret: Vec<u8>, hash: String) -> bool {
    tokio::task::spawn_blocking(move || verify(&secret, &hash))
        .await
        .unwrap_or(false)
}

/// Código de convite: 16 caracteres de um alfabeto sem ambiguidade (sem 0/O, 1/I/L).
pub fn invite_code() -> anyhow::Result<String> {
    const ALPHABET: &[u8] = b"ABCDEFGHJKMNPQRSTUVWXYZ23456789";
    let mut raw = [0u8; 16];
    getrandom::fill(&mut raw).map_err(|e| anyhow::anyhow!("getrandom: {e}"))?;
    Ok(raw
        .iter()
        .map(|b| ALPHABET[*b as usize % ALPHABET.len()] as char)
        .collect())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn hash_and_verify() {
        let h = hash(b"senha123").unwrap();
        assert!(h.starts_with("$argon2id$v=19$m=19456,t=2,p=1$"), "{h}");
        assert!(verify(b"senha123", &h));
        assert!(!verify(b"senha124", &h));
        assert!(!verify(b"senha123", "lixo"));
        assert_ne!(hash(b"senha123").unwrap(), h, "o sal tem que mudar");
    }

    #[test]
    fn invite_codes() {
        let a = invite_code().unwrap();
        assert_eq!(a.len(), 16);
        assert_ne!(a, invite_code().unwrap());
    }
}
