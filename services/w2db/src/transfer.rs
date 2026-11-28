//! Importação e exportação dos arquivos de conta do DBSrv (account/<letra>/<CONTA>).

use std::path::Path;

use anyhow::{Context, bail};
use sqlx::PgPool;

use crate::store::{self, SaveMode};
use crate::{auth, legacy};

/// Importa uma conta lida do arquivo. Devolve o número de personagens e de itens.
pub async fn import_file(pool: &PgPool, bytes: &[u8]) -> anyhow::Result<(usize, usize)> {
    let (acc, secrets) = legacy::decode(bytes)?;
    if !store::valid_name(&acc.name) {
        bail!("nome de conta inválido");
    }
    let password = auth::hash_async(secrets.password).await?;
    let numeric = match secrets.numeric {
        Some(n) => Some(auth::hash_async(n).await?),
        None => None,
    };
    let mut tx = pool.begin().await?;
    let Some(id) = store::create_account(&mut tx, &acc.name, &password, numeric.as_deref(), None).await? else {
        bail!("a conta já existe no banco");
    };
    match store::save_account(&mut tx, id, &acc, &[], "importação", false, SaveMode::Import).await? {
        store::SaveOutcome::Saved(_) => {}
        store::SaveOutcome::NameTaken => bail!("nome de personagem já existe em outra conta"),
        store::SaveOutcome::VersionConflict(_) => unreachable!("a importação não confere versão"),
    }
    tx.commit().await?;
    let items = acc.cargo.len()
        + acc
            .characters
            .iter()
            .map(|c| c.equip.len() + c.carry.len())
            .sum::<usize>();
    Ok((acc.characters.len(), items))
}

pub async fn import_dir(pool: &PgPool, run_dir: &Path) -> anyhow::Result<ImportReport> {
    let root = run_dir.join("account");
    let mut files = Vec::new();
    for dir in std::fs::read_dir(&root).with_context(|| format!("lendo {}", root.display()))? {
        let dir = dir?.path();
        if dir.is_dir() {
            for f in std::fs::read_dir(&dir)? {
                let f = f?.path();
                if f.is_file() {
                    files.push(f);
                }
            }
        }
    }
    files.sort();
    let (mut ok, mut chars, mut items, mut failed) = (0, 0, 0, 0);
    for f in &files {
        let bytes = std::fs::read(f)?;
        match import_file(pool, &bytes).await {
            Ok((c, i)) => {
                ok += 1;
                chars += c;
                items += i;
            }
            Err(e) => {
                failed += 1;
                eprintln!("{}: {e:#}", f.display());
            }
        }
    }
    Ok(ImportReport {
        accounts: ok,
        characters: chars,
        items,
        failed,
    })
}

pub struct ImportReport {
    pub accounts: usize,
    pub characters: usize,
    pub items: usize,
    pub failed: usize,
}

impl std::fmt::Display for ImportReport {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{} conta(s) importada(s), {} personagem(ns), {} item(ns); {} com erro",
            self.accounts, self.characters, self.items, self.failed
        )
    }
}

pub async fn export_dir(pool: &PgPool, out: &Path, only: Option<&str>) -> anyhow::Result<usize> {
    let names = match only {
        Some(n) => vec![n.to_string()],
        None => store::all_account_names(pool).await?,
    };
    for name in &names {
        let bytes = legacy::string_to_cp1252(name)?;
        let id = store::account_id(pool, &bytes)
            .await?
            .with_context(|| format!("conta {name} não existe"))?;
        let acc = store::load_account_pool(pool, id).await?;
        let file = legacy::encode(&acc)?;
        let upper = legacy::name_key(&acc.name);
        let dir = out.join("account").join(legacy::first_key(&upper)?);
        std::fs::create_dir_all(&dir)?;
        std::fs::write(dir.join(legacy::cp1252_to_string(&upper)), &file)?;
        // Índice de personagem do DBSrv (CFileDB::CreateCharacter): o nome da conta em 16 bytes.
        for c in &acc.characters {
            let key = legacy::name_key(&c.name);
            let dir = out.join("char").join(legacy::first_key(&key)?);
            std::fs::create_dir_all(&dir)?;
            let mut index = [0u8; 16];
            index[..acc.name.len()].copy_from_slice(&acc.name);
            std::fs::write(dir.join(legacy::cp1252_to_string(&key)), index)?;
        }
    }
    Ok(names.len())
}
