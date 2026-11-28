//! Saída da B2: conta em arquivo -> banco -> arquivo igual byte a byte, fora senha, senha numérica
//! e dados pessoais. Os arquivos de tests/data/account são os que o DBSrv grava no
//! tools/server_boot_smoke.py (contas sintéticas, com personagem criado e jogado).

use std::path::{Path, PathBuf};

use sqlx::PgPool;
use w2db::legacy::{self, testkit};
use w2db::{auth, store, transfer};

fn data_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("tests/data")
}

fn scratch(name: &str) -> PathBuf {
    let dir = std::env::temp_dir().join(format!("w2db-test-{}-{name}", std::process::id()));
    let _ = std::fs::remove_dir_all(&dir);
    std::fs::create_dir_all(&dir).unwrap();
    dir
}

fn assert_round_trip(src: &[u8], out: &[u8], what: &str) {
    let mask = testkit::round_tripped_mask();
    assert_eq!(out.len(), src.len());
    let diff: Vec<usize> = (0..src.len()).filter(|&i| mask[i] && src[i] != out[i]).collect();
    assert!(
        diff.is_empty(),
        "{what}: {} byte(s) diferentes, o primeiro em {}",
        diff.len(),
        diff[0]
    );
}

#[sqlx::test]
async fn dbsrv_files_round_trip(pool: PgPool) {
    let report = transfer::import_dir(&pool, &data_dir()).await.unwrap();
    assert_eq!((report.accounts, report.failed), (2, 0), "{report}");
    assert_eq!(report.characters, 2);

    let out = scratch("export");
    assert_eq!(transfer::export_dir(&pool, &out, None).await.unwrap(), 2);
    for name in ["DERUST", "DERUST2"] {
        let src = std::fs::read(data_dir().join("account/D").join(name)).unwrap();
        let got = std::fs::read(out.join("account/D").join(name)).unwrap();
        assert_round_trip(&src, &got, name);
    }
    // Índices de personagem como os do DBSrv.
    assert_eq!(&std::fs::read(out.join("char/H/HEROI")).unwrap()[..7], b"DERUST\0");

    // Senha e senha numérica viraram hash e conferem.
    let a = store::account_auth(&pool, b"DERUST").await.unwrap().unwrap();
    assert!(a.password_hash.starts_with("$argon2id$"));
    assert!(auth::verify(b"senha123", &a.password_hash));
    assert!(auth::verify(b"1234", a.numeric_hash.as_deref().unwrap()));

    // Nada de dados pessoais no banco.
    let dump: String = sqlx::query_scalar("SELECT string_agg(row_to_json(a)::text, '') FROM accounts a")
        .fetch_one(&pool)
        .await
        .unwrap();
    for personal in ["Teste", "exemplo.invalid", "5550000", "RuaTeste", "senha123"] {
        assert!(!dump.contains(personal), "{personal} foi para o banco");
    }

    // Importar de novo não duplica.
    let again = transfer::import_dir(&pool, &data_dir()).await.unwrap();
    assert_eq!((again.accounts, again.failed), (0, 2));
}

#[sqlx::test]
async fn random_accounts_round_trip(pool: PgPool) {
    let src_dir = scratch("random-src");
    let mut originals = Vec::new();
    for seed in 1..=12u64 {
        let mut file = testkit::random_account(seed.wrapping_mul(0x9E37_79B9_7F4A_7C15), (seed % 4) as usize);
        // Nomes únicos entre as contas (o gerador repete os nomes).
        let acc = format!("CONTA{seed}");
        file[..16].fill(0);
        file[..acc.len()].copy_from_slice(acc.as_bytes());
        for slot in 0..4 {
            let off = 224 + slot * 1024;
            if file[off] != 0 {
                let ch = format!("H{seed}x{slot}\u{e9}");
                file[off..off + 16].fill(0);
                file[off..off + ch.len()].copy_from_slice(ch.as_bytes());
            }
        }
        // Senha válida (1..15 bytes sem zero).
        file[16..32].fill(0);
        file[16..20].copy_from_slice(b"pass");
        let dir = src_dir.join("account").join(legacy::first_key(acc.as_bytes()).unwrap());
        std::fs::create_dir_all(&dir).unwrap();
        std::fs::write(dir.join(&acc), &file).unwrap();
        originals.push((acc, file));
    }
    let report = transfer::import_dir(&pool, &src_dir).await.unwrap();
    assert_eq!((report.accounts, report.failed), (12, 0), "{report}");

    let out = scratch("random-out");
    transfer::export_dir(&pool, &out, None).await.unwrap();
    for (acc, src) in &originals {
        let got = std::fs::read(out.join("account/C").join(acc)).unwrap();
        assert_round_trip(src, &got, acc);
    }
}

/// O proto que o codec do importador tira de cada arquivo de tests/data/account, gravado ao lado
/// (`<CONTA>.pb`). A conversão do DBSrv (server/dbsrv/persistence/W2DbConvert.cpp) confere contra
/// esses arquivos no ctest. Para regravar: W2DB_WRITE_GOLDEN=1 cargo test --test roundtrip golden.
#[test]
fn golden_protos() {
    use prost::Message;
    let dir = data_dir().join("account/D");
    for name in ["DERUST", "DERUST2"] {
        let (acc, _) = legacy::decode(&std::fs::read(dir.join(name)).unwrap()).unwrap();
        let bytes = acc.encode_to_vec();
        let golden = data_dir().join(format!("golden/{name}.pb"));
        if std::env::var_os("W2DB_WRITE_GOLDEN").is_some() {
            std::fs::write(&golden, &bytes).unwrap();
        }
        let want = std::fs::read(&golden).expect("falta o .pb (W2DB_WRITE_GOLDEN=1)");
        assert_eq!(
            w2db::pb::Account::decode(want.as_slice()).unwrap(),
            acc,
            "{name}.pb desatualizado"
        );
    }
}
