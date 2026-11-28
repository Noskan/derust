//! w2db: o serviço (serve), as migrações e o importador/exportador dos arquivos do DBSrv.

use std::os::unix::fs::PermissionsExt;
use std::path::{Path, PathBuf};

use anyhow::{Context, bail};
use clap::{Parser, Subcommand};
use sqlx::PgPool;
use sqlx::postgres::PgPoolOptions;
use tokio::net::UnixListener;
use tokio_stream::wrappers::UnixListenerStream;

use w2db::pb::w2_db_server::W2DbServer;
use w2db::store;
use w2db::{auth, service, transfer};

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
    /// Roda o serviço gRPC num Unix socket (aplica as migrações antes).
    Serve {
        #[arg(long, env = "W2DB_SOCKET", default_value = "/run/w2db/w2db.sock")]
        socket: PathBuf,
        /// Conta nova só com código de convite.
        #[arg(long)]
        require_invite: bool,
    },
    /// Aplica as migrações.
    Migrate,
    /// Importa as contas de um DBSRV/Run (os arquivos de account/<letra>/<CONTA>).
    Import { run_dir: PathBuf },
    /// Grava as contas do banco no formato do DBSrv (account/ e char/ dentro de out_dir).
    Export {
        out_dir: PathBuf,
        /// Só esta conta.
        #[arg(long)]
        account: Option<String>,
    },
    /// Cria um código de convite para um id do Discord.
    Invite { discord_id: String },
}

async fn connect(url: Option<String>) -> anyhow::Result<PgPool> {
    let url = url.context("falta --database-url ou DATABASE_URL")?;
    Ok(PgPoolOptions::new().max_connections(16).connect(&url).await?)
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    tracing_subscriber::fmt()
        .with_env_filter(tracing_subscriber::EnvFilter::try_from_default_env().unwrap_or_else(|_| "info".into()))
        .init();
    let cli = Cli::parse();
    let pool = connect(cli.database_url).await?;
    match cli.cmd {
        Cmd::Serve { socket, require_invite } => {
            sqlx::migrate!().run(&pool).await?;
            serve(pool, &socket, require_invite).await
        }
        Cmd::Migrate => {
            sqlx::migrate!().run(&pool).await?;
            println!("migrações aplicadas");
            Ok(())
        }
        Cmd::Import { run_dir } => {
            let r = transfer::import_dir(&pool, &run_dir).await?;
            println!("{r}");
            if r.failed > 0 {
                bail!("{} conta(s) não importada(s)", r.failed);
            }
            Ok(())
        }
        Cmd::Export { out_dir, account } => {
            let n = transfer::export_dir(&pool, &out_dir, account.as_deref()).await?;
            println!("{n} conta(s) exportada(s) para {}", out_dir.display());
            Ok(())
        }
        Cmd::Invite { discord_id } => {
            let code = auth::invite_code()?;
            store::create_invite(&pool, &code, &discord_id).await?;
            println!("{code}");
            Ok(())
        }
    }
}

async fn serve(pool: PgPool, socket: &Path, require_invite: bool) -> anyhow::Result<()> {
    if socket.exists() {
        std::fs::remove_file(socket).with_context(|| format!("removendo {}", socket.display()))?;
    }
    let listener = UnixListener::bind(socket).with_context(|| format!("abrindo {}", socket.display()))?;
    // Dono e grupo (o usuário dos servidores do jogo) leem e escrevem; os outros não.
    std::fs::set_permissions(socket, std::fs::Permissions::from_mode(0o660))?;
    tracing::info!("w2db no ar em {}", socket.display());
    let shutdown = async {
        let mut term = tokio::signal::unix::signal(tokio::signal::unix::SignalKind::terminate()).expect("SIGTERM");
        tokio::select! {
            _ = tokio::signal::ctrl_c() => {}
            _ = term.recv() => {}
        }
        tracing::info!("encerrando");
    };
    tonic::transport::Server::builder()
        .add_service(W2DbServer::new(service::Service::new(pool, require_invite)))
        .serve_with_incoming_shutdown(UnixListenerStream::new(listener), shutdown)
        .await?;
    let _ = std::fs::remove_file(socket);
    Ok(())
}
