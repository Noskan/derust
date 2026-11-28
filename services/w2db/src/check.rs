//! `w2db check`: conferências do banco, para depois de uma restauração do backup (docs/OPS.md).
//!
//! Só lê. As restrições do esquema o Postgres já garante; aqui fica o que elas não dizem: as
//! migrações aplicadas são as deste binário, cada item está numa vaga que existe e de um
//! personagem da própria conta, todo item tem evento no histórico, a última versão guardada de
//! cada conta é a atual, e cada conta sai do banco e cabe no arquivo do DBSrv (o caminho de um
//! login).

use std::fmt::Write as _;

use sqlx::PgPool;

use crate::{legacy, store};

#[derive(Debug, Default)]
pub struct Report {
    /// Uma linha por conferência que passou, com o que viu.
    pub ok: Vec<String>,
    /// Uma linha por problema.
    pub problems: Vec<String>,
}

impl std::fmt::Display for Report {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut s = String::new();
        for l in &self.ok {
            let _ = writeln!(s, "ok: {l}");
        }
        for l in &self.problems {
            let _ = writeln!(s, "PROBLEMA: {l}");
        }
        if self.problems.is_empty() {
            s.push_str("resultado: OK");
        } else {
            let _ = write!(s, "resultado: {} problema(s)", self.problems.len());
        }
        f.write_str(&s)
    }
}

/// Até quantos exemplos de cada problema vão para o relatório.
const SAMPLES: i64 = 5;

fn samples(v: &[i64]) -> String {
    v.iter().map(|s| s.to_string()).collect::<Vec<_>>().join(", ")
}

pub async fn run(pool: &PgPool) -> anyhow::Result<Report> {
    let mut r = Report::default();

    // Migrações: as aplicadas com sucesso são exatamente as embutidas neste binário.
    let applied: Vec<(i64, bool)> = sqlx::query_as("SELECT version, success FROM _sqlx_migrations ORDER BY version")
        .fetch_all(pool)
        .await?;
    let embedded: Vec<i64> = sqlx::migrate!().iter().map(|m| m.version).collect();
    let ok_applied: Vec<i64> = applied.iter().filter(|a| a.1).map(|a| a.0).collect();
    if ok_applied == embedded && applied.iter().all(|a| a.1) {
        r.ok.push(format!("migrações {:?}", embedded));
    } else {
        r.problems
            .push(format!("migrações no banco {:?}, neste w2db {:?}", applied, embedded));
    }

    let (accounts, characters, items, events): (i64, i64, i64, i64) = sqlx::query_as(
        "SELECT (SELECT count(*) FROM accounts), (SELECT count(*) FROM characters),
                (SELECT count(*) FROM items), (SELECT count(*) FROM item_events)",
    )
    .fetch_one(pool)
    .await?;
    let (last_save, last_event): (Option<String>, Option<String>) =
        sqlx::query_as("SELECT (SELECT max(updated_at)::text FROM accounts), (SELECT max(at)::text FROM item_events)")
            .fetch_one(pool)
            .await?;
    r.ok.push(format!(
        "{accounts} conta(s), {characters} personagem(ns), {items} item(ns), {events} evento(s); \
         última gravação {}, último evento {}",
        last_save.as_deref().unwrap_or("-"),
        last_event.as_deref().unwrap_or("-")
    ));

    // Vaga dentro do tamanho do contêiner.
    let bad: Vec<i64> = sqlx::query_scalar(
        "SELECT serial FROM items
         WHERE (container = 1 AND slot NOT BETWEEN 0 AND $1) OR (container = 2 AND slot NOT BETWEEN 0 AND $2)
            OR (container = 3 AND slot NOT BETWEEN 0 AND $3)
         ORDER BY serial LIMIT $4",
    )
    .bind(legacy::MAX_EQUIP as i32 - 1)
    .bind(legacy::MAX_CARRY as i32 - 1)
    .bind(legacy::MAX_CARGO as i32 - 1)
    .bind(SAMPLES)
    .fetch_all(pool)
    .await?;
    push(&mut r, bad, "itens em vaga fora do contêiner", "vagas dos itens");

    // Equipamento e inventário de personagem da própria conta.
    let bad: Vec<i64> = sqlx::query_scalar(
        "SELECT i.serial FROM items i JOIN characters c ON c.id = i.character_id
         WHERE i.account_id <> c.account_id ORDER BY i.serial LIMIT $1",
    )
    .bind(SAMPLES)
    .fetch_all(pool)
    .await?;
    push(&mut r, bad, "itens com personagem de outra conta", "dono dos itens");

    // Conta que perdeu o item numa troca não é a própria dona.
    let bad: Vec<i64> =
        sqlx::query_scalar("SELECT serial FROM items WHERE contested_from = account_id ORDER BY serial LIMIT $1")
            .bind(SAMPLES)
            .fetch_all(pool)
            .await?;
    push(&mut r, bad, "itens contestados pela própria conta", "itens contestados");

    // Todo item tem pelo menos um evento (create, import, transfer...).
    let bad: Vec<i64> = sqlx::query_scalar(
        "SELECT i.serial FROM items i
         WHERE NOT EXISTS (SELECT 1 FROM item_events e WHERE e.serial = i.serial)
         ORDER BY i.serial LIMIT $1",
    )
    .bind(SAMPLES)
    .fetch_all(pool)
    .await?;
    push(&mut r, bad, "itens sem evento no histórico", "histórico dos itens");

    // A última versão guardada é a atual (toda gravação sobe a versão e guarda a conta).
    let bad: Vec<i64> = sqlx::query_scalar(
        "SELECT a.id FROM accounts a
         JOIN (SELECT account_id, max(version) AS v FROM account_versions GROUP BY account_id) s
           ON s.account_id = a.id
         WHERE s.v <> a.version ORDER BY a.id LIMIT $1",
    )
    .bind(SAMPLES)
    .fetch_all(pool)
    .await?;
    push(
        &mut r,
        bad,
        "contas com a última versão guardada diferente da atual (ids)",
        "versões das contas",
    );

    // Cada conta sai do banco como no login e cabe no arquivo do DBSrv.
    let ids: Vec<i64> = sqlx::query_scalar("SELECT id FROM accounts ORDER BY id")
        .fetch_all(pool)
        .await?;
    let mut bad = Vec::new();
    for id in &ids {
        let fine = match store::load_account_pool(pool, *id).await {
            Ok(acc) => legacy::encode(&acc).is_ok(),
            Err(_) => false,
        };
        if !fine {
            bad.push(*id);
        }
    }
    let n = bad.len();
    bad.truncate(SAMPLES as usize);
    if n == 0 {
        r.ok.push(format!(
            "as {} conta(s) carregam e cabem no arquivo do DBSrv",
            ids.len()
        ));
    } else {
        r.problems.push(format!(
            "{n} conta(s) não carregam ou não cabem no arquivo (ids {})",
            samples(&bad)
        ));
    }
    Ok(r)
}

fn push(r: &mut Report, bad: Vec<i64>, problem: &str, what: &str) {
    if bad.is_empty() {
        r.ok.push(what.to_string());
    } else {
        r.problems.push(format!("{problem}: {}", samples(&bad)));
    }
}
