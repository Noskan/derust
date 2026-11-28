//! w2gm: o painel de GM e o bot do Discord (docs/GM.md).

use std::io::BufRead;
use std::net::SocketAddr;
use std::path::{Path, PathBuf};

use anyhow::{Context, bail};
use clap::{Parser, Subcommand};
use poise::serenity_prelude as serenity;
use sqlx::PgPool;
use sqlx::postgres::PgPoolOptions;

use w2gm::{bot, gmauth, web};

/// A migração do w2db com as tabelas do painel (0004_community_gm.sql).
const NEEDS_MIGRATION: i64 = 4;

#[derive(Parser)]
#[command(version, about)]
struct Cli {
    /// postgres://... (ou a variável DATABASE_URL)
    #[arg(long, env = "DATABASE_URL", global = true, hide_env_values = true)]
    database_url: Option<String>,
    #[command(subcommand)]
    cmd: Cmd,
}

#[derive(Subcommand)]
enum Cmd {
    /// Serve o painel e o cadastro (atrás do proxy HTTPS) e, com o token, roda o bot do Discord.
    Serve {
        #[arg(long, env = "W2GM_LISTEN", default_value = "127.0.0.1:8730")]
        listen: SocketAddr,
        /// Endereço público do site, para os links do /registrar (https://...).
        #[arg(long, env = "W2GM_PUBLIC_URL")]
        public_url: String,
        /// Arquivo com o token do bot (primeira linha). Sem ele, só o site.
        #[arg(long, env = "W2GM_DISCORD_TOKEN_FILE")]
        discord_token_file: Option<PathBuf>,
        /// Id do servidor do Discord onde os comandos são registrados.
        #[arg(long, env = "W2GM_DISCORD_GUILD")]
        discord_guild: Option<u64>,
        /// Id do cargo que pode usar o /convite.
        #[arg(long, env = "W2GM_DISCORD_GM_ROLE")]
        discord_gm_role: Option<u64>,
    },
    /// Cria a conta de GM (ou troca senha e TOTP de uma que existe). A senha vem na primeira linha
    /// da entrada; imprime o endereço otpauth:// para o aplicativo autenticador.
    GmAdd { name: String },
    /// Desativa a conta de GM e derruba as sessões dela.
    GmDisable { name: String },
}

async fn connect(url: Option<String>) -> anyhow::Result<PgPool> {
    let url = url.context("falta DATABASE_URL (ou --database-url)")?;
    let pool = PgPoolOptions::new()
        .max_connections(5)
        .connect(&url)
        .await
        .context("conectando ao Postgres")?;
    let applied: Option<i64> = sqlx::query_scalar("SELECT max(version) FROM _sqlx_migrations WHERE success")
        .fetch_one(&pool)
        .await
        .context("lendo as migrações (rode w2db migrate)")?;
    if applied.unwrap_or(0) < NEEDS_MIGRATION {
        bail!("o banco está sem a migração {NEEDS_MIGRATION} do w2db: rode w2db migrate (ou suba o w2db)");
    }
    Ok(pool)
}

fn first_line(path: &Path) -> anyhow::Result<String> {
    let text = std::fs::read_to_string(path).with_context(|| format!("lendo {}", path.display()))?;
    let line = text.lines().next().unwrap_or("").trim().to_string();
    if line.is_empty() {
        bail!("{} está vazio", path.display());
    }
    Ok(line)
}

async fn shutdown() {
    let mut term = tokio::signal::unix::signal(tokio::signal::unix::SignalKind::terminate()).expect("SIGTERM");
    tokio::select! {
        _ = term.recv() => {}
        _ = tokio::signal::ctrl_c() => {}
    }
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(
            tracing_subscriber::EnvFilter::try_from_default_env()
                .unwrap_or_else(|_| "info,serenity=warn,tracing::span=warn".into()),
        )
        .init();
    let cli = Cli::parse();
    match cli.cmd {
        Cmd::Serve {
            listen,
            public_url,
            discord_token_file,
            discord_guild,
            discord_gm_role,
        } => {
            if !public_url.starts_with("https://") && !public_url.starts_with("http://127.0.0.1") {
                bail!("--public-url precisa ser https:// (os links do cadastro levam o token)");
            }
            let pool = connect(cli.database_url).await?;
            let listener = tokio::net::TcpListener::bind(listen)
                .await
                .with_context(|| format!("abrindo {listen}"))?;
            tracing::info!("w2gm em http://{listen} (público: {public_url})");

            let bot = match discord_token_file {
                Some(file) => {
                    let (Some(guild), Some(role)) = (discord_guild, discord_gm_role) else {
                        bail!("o bot precisa de --discord-guild e --discord-gm-role");
                    };
                    let token = first_line(&file)?;
                    let data = bot::Data {
                        pool: pool.clone(),
                        public_url: public_url.clone(),
                        gm_role: serenity::RoleId::new(role),
                    };
                    Some(tokio::spawn(bot::run(token, serenity::GuildId::new(guild), data)))
                }
                None => {
                    tracing::info!("sem --discord-token-file: o bot do Discord não roda");
                    None
                }
            };

            let app = web::router(web::AppState::new(pool));
            let server = axum::serve(listener, app.into_make_service_with_connect_info::<SocketAddr>())
                .with_graceful_shutdown(shutdown());
            match bot {
                // O bot caiu de vez (token errado, por exemplo): o processo sai e o systemd avisa.
                Some(bot) => tokio::select! {
                    r = server => r?,
                    r = bot => {
                        r??;
                        bail!("o bot do Discord parou");
                    }
                },
                None => server.await?,
            }
            Ok(())
        }
        Cmd::GmAdd { name } => {
            let pool = connect(cli.database_url).await?;
            let mut password = String::new();
            std::io::stdin().lock().read_line(&mut password)?;
            let password = password.trim_end_matches(['\r', '\n']);
            let secret = gmauth::add_gm(&pool, &name, password).await?;
            println!("Conta de GM {name} pronta. Cadastre no aplicativo autenticador:");
            println!("{}", gmauth::otpauth_uri(&name, &secret));
            println!(
                "(ou o segredo {} à mão: TOTP, SHA-1, 6 dígitos, 30 s)",
                data_encoding::BASE32_NOPAD.encode(&secret)
            );
            Ok(())
        }
        Cmd::GmDisable { name } => {
            let pool = connect(cli.database_url).await?;
            if !w2db::admin::gm_disable(&pool, &name).await? {
                bail!("não existe conta de GM {name}");
            }
            println!("Conta de GM {name} desativada.");
            Ok(())
        }
    }
}
