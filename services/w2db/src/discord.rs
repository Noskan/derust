//! Relatos do `/bug` no Discord (docs/OPS.md): a fila é `bug_reports` sem `posted_at`.
//!
//! Uma tarefa olha a fila de tempos em tempos e posta cada relato no webhook do canal. O que o
//! jogador escreveu não menciona ninguém (`allowed_mentions` vazio) e não vira prévia de link. O
//! endereço do webhook é segredo (quem o tem posta no canal): vem de um arquivo e não vai para o
//! log, nem nos erros do reqwest.

use std::path::Path;
use std::time::Duration;

use anyhow::Context;
use serde_json::json;
use sqlx::PgPool;

use crate::metrics::{METRICS, Metrics};
use crate::store::{self, BugPost};

/// Intervalo entre as olhadas na fila.
pub const POLL: Duration = Duration::from_secs(10);
/// Relatos por olhada (o webhook aceita cerca de 5 por 2 segundos).
const BATCH: i64 = 5;
/// Tamanho máximo do conteúdo de uma mensagem do Discord.
const MAX_CONTENT: usize = 2000;

pub struct Webhook {
    client: reqwest::Client,
    url: String,
}

/// Lê o endereço do webhook (primeira linha do arquivo).
pub fn read_url(path: &Path) -> anyhow::Result<String> {
    let text = std::fs::read_to_string(path).with_context(|| format!("lendo {}", path.display()))?;
    let url = text.lines().next().unwrap_or_default().trim().to_string();
    anyhow::ensure!(
        url.starts_with("https://") || url.starts_with("http://"),
        "{} não tem um endereço http(s) na primeira linha",
        path.display()
    );
    Ok(url)
}

/// Escapa o que o Discord leria como formatação ou citação no texto do jogador.
fn escape(s: &str) -> String {
    let mut o = String::with_capacity(s.len());
    for c in s.chars() {
        if c.is_control() {
            o.push(' ');
            continue;
        }
        if matches!(
            c,
            '\\' | '*' | '_' | '~' | '`' | '|' | '>' | '#' | '-' | '[' | ']' | '(' | ')' | '<' | '@' | ':'
        ) {
            o.push('\\');
        }
        o.push(c);
    }
    o
}

/// O corpo da mensagem do relato.
pub fn message(b: &BugPost) -> serde_json::Value {
    let account = b.account.as_deref().unwrap_or("?");
    let mut content = format!(
        "**Bug #{}**: {} (conta {}), servidor {}, posição {} {}\n",
        b.id,
        escape(&b.character),
        escape(account),
        b.server,
        b.x,
        b.y
    );
    let room = MAX_CONTENT.saturating_sub(content.chars().count() + 2);
    let text: String = escape(&b.text).chars().take(room).collect();
    content.push_str("> ");
    content.push_str(&text);
    json!({
        "content": content,
        "allowed_mentions": { "parse": [] },
        // SUPPRESS_EMBEDS: link no texto não vira prévia.
        "flags": 4,
    })
}

impl Webhook {
    pub fn new(url: String) -> anyhow::Result<Self> {
        let client = reqwest::Client::builder()
            .timeout(Duration::from_secs(15))
            .redirect(reqwest::redirect::Policy::none())
            .build()?;
        Ok(Webhook { client, url })
    }

    /// Posta o que está na fila, até [`BATCH`] relatos. Devolve quantos foram.
    pub async fn post_pending(&self, pool: &PgPool) -> anyhow::Result<usize> {
        let mut sent = 0;
        for b in store::pending_bug_posts(pool, BATCH).await? {
            let r = self.client.post(&self.url).json(&message(&b)).send().await;
            match r {
                Ok(rsp) if rsp.status().is_success() => {
                    store::mark_bug_posted(pool, b.id).await?;
                    Metrics::inc(&METRICS.discord_posts);
                    sent += 1;
                }
                Ok(rsp) if rsp.status() == reqwest::StatusCode::TOO_MANY_REQUESTS => {
                    // Limite do Discord: espera o que ele pedir e tenta na próxima olhada.
                    Metrics::inc(&METRICS.discord_post_failures);
                    let wait = rsp
                        .headers()
                        .get("retry-after")
                        .and_then(|v| v.to_str().ok())
                        .and_then(|v| v.parse::<f64>().ok())
                        .unwrap_or(5.0)
                        .clamp(0.0, 60.0);
                    tracing::warn!("discord: limite de envio, esperando {wait:.1} s");
                    tokio::time::sleep(Duration::from_secs_f64(wait)).await;
                    break;
                }
                Ok(rsp) => {
                    Metrics::inc(&METRICS.discord_post_failures);
                    store::mark_bug_post_failed(pool, b.id).await?;
                    tracing::warn!("discord: relato #{} recusado com {}", b.id, rsp.status());
                }
                Err(e) => {
                    Metrics::inc(&METRICS.discord_post_failures);
                    store::mark_bug_post_failed(pool, b.id).await?;
                    tracing::warn!("discord: relato #{} não foi: {}", b.id, e.without_url());
                    break;
                }
            }
        }
        Ok(sent)
    }

    /// Posta a fila para sempre (a tarefa do `serve`), olhando a cada `poll`.
    pub async fn run(self, pool: PgPool, poll: Duration) {
        loop {
            if let Err(e) = self.post_pending(&pool).await {
                tracing::warn!("discord: {e}");
            }
            tokio::time::sleep(poll).await;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn player_text_does_not_mention_or_format() {
        let b = BugPost {
            id: 7,
            account: Some("CONTA".into()),
            character: "Fulano_".into(),
            server: 1,
            x: 2100,
            y: 2101,
            text: "@everyone <@123> **x** https://a.b\nlinha".into(),
        };
        let m = message(&b);
        let c = m["content"].as_str().unwrap();
        assert!(c.starts_with("**Bug #7**: Fulano\\_ (conta CONTA), servidor 1, posição 2100 2101\n> "));
        assert!(c.contains("\\@everyone \\<\\@123\\> \\*\\*x\\*\\* https\\://a.b linha"));
        assert_eq!(m["allowed_mentions"]["parse"].as_array().unwrap().len(), 0);
        assert_eq!(m["flags"], 4);
    }

    #[test]
    fn long_text_fits_one_message() {
        let b = BugPost {
            id: 1,
            account: None,
            character: "A".into(),
            server: 0,
            x: 0,
            y: 0,
            text: "*".repeat(5000),
        };
        assert!(message(&b)["content"].as_str().unwrap().chars().count() <= MAX_CONTENT);
    }
}
