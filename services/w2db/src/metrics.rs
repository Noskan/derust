//! Métricas do w2db no formato de texto do Prometheus (`/metrics`, docs/OPS.md).
//!
//! Contadores do processo ([`METRICS`]), uma camada do tower que mede cada RPC pelo caminho
//! (`/w2db.v1.W2Db/<Método>`) e, na hora da coleta, o que se lê do banco: se ele responde, a fila
//! de relatos do Discord e o arquivamento do WAL (`pg_stat_archiver`, o lado contínuo do backup).

use std::collections::BTreeMap;
use std::convert::Infallible;
use std::fmt::Write as _;
use std::future::Future;
use std::net::SocketAddr;
use std::pin::Pin;
use std::sync::Mutex;
use std::sync::atomic::{AtomicU64, Ordering::Relaxed};
use std::task::{Context, Poll};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

use bytes::Bytes;
use http_body_util::Full;
use hyper::body::Incoming;
use hyper::service::service_fn;
use hyper_util::rt::{TokioIo, TokioTimer};
use sqlx::PgPool;
use tokio::net::TcpListener;

use crate::store;

/// Métodos distintos guardados (o caminho vem do DBSrv; isto só limita um cliente estranho).
const MAX_METHODS: usize = 64;

#[derive(Default, Clone, Copy)]
struct Rpc {
    requests: u64,
    errors: u64,
    seconds: f64,
}

pub struct Metrics {
    rpc: Mutex<BTreeMap<String, Rpc>>,
    pub login_wrong_password: AtomicU64,
    pub login_locked: AtomicU64,
    pub login_no_account: AtomicU64,
    pub numeric_wrong: AtomicU64,
    pub dupe_suspects: AtomicU64,
    pub version_conflicts: AtomicU64,
    pub bug_reports: AtomicU64,
    pub bug_reports_limited: AtomicU64,
    pub discord_posts: AtomicU64,
    pub discord_post_failures: AtomicU64,
    start: AtomicU64,
}

pub static METRICS: Metrics = Metrics {
    rpc: Mutex::new(BTreeMap::new()),
    login_wrong_password: AtomicU64::new(0),
    login_locked: AtomicU64::new(0),
    login_no_account: AtomicU64::new(0),
    numeric_wrong: AtomicU64::new(0),
    dupe_suspects: AtomicU64::new(0),
    version_conflicts: AtomicU64::new(0),
    bug_reports: AtomicU64::new(0),
    bug_reports_limited: AtomicU64::new(0),
    discord_posts: AtomicU64::new(0),
    discord_post_failures: AtomicU64::new(0),
    start: AtomicU64::new(0),
};

fn now_secs() -> f64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs_f64()
}

impl Metrics {
    pub fn inc(c: &AtomicU64) {
        c.fetch_add(1, Relaxed);
    }

    pub fn add(c: &AtomicU64, n: u64) {
        c.fetch_add(n, Relaxed);
    }

    /// Marca o início do serviço (`w2db_start_time_seconds`).
    pub fn started(&self) {
        self.start.store(now_secs() as u64, Relaxed);
    }

    fn rpc(&self, method: &str, took: Duration, ok: bool) {
        let mut m = self.rpc.lock().unwrap_or_else(|e| e.into_inner());
        if !m.contains_key(method) && m.len() >= MAX_METHODS {
            return;
        }
        let r = m.entry(method.to_string()).or_default();
        r.requests += 1;
        if !ok {
            r.errors += 1;
        }
        r.seconds += took.as_secs_f64();
    }

    /// O texto da coleta. Os valores do banco ficam de fora (com `w2db_database_up 0`) se ele não
    /// responder.
    pub async fn render(&self, pool: &PgPool) -> String {
        let mut o = String::new();
        let family = |o: &mut String, name: &str, kind: &str, help: &str| {
            let _ = writeln!(o, "# HELP {name} {help}\n# TYPE {name} {kind}");
        };
        let v = |c: &AtomicU64| c.load(Relaxed);

        family(&mut o, "w2db_build_info", "gauge", "Versão do w2db.");
        let _ = writeln!(o, "w2db_build_info{{version=\"{}\"}} 1", env!("CARGO_PKG_VERSION"));
        family(&mut o, "w2db_start_time_seconds", "gauge", "Início do serviço (Unix).");
        let _ = writeln!(o, "w2db_start_time_seconds {}", v(&self.start));

        let rpc = self.rpc.lock().unwrap_or_else(|e| e.into_inner()).clone();
        family(
            &mut o,
            "w2db_rpc_requests_total",
            "counter",
            "Chamadas gRPC por método.",
        );
        for (m, r) in &rpc {
            let _ = writeln!(o, "w2db_rpc_requests_total{{method=\"{m}\"}} {}", r.requests);
        }
        family(
            &mut o,
            "w2db_rpc_errors_total",
            "counter",
            "Chamadas gRPC que voltaram com erro (status gRPC diferente de OK).",
        );
        for (m, r) in &rpc {
            let _ = writeln!(o, "w2db_rpc_errors_total{{method=\"{m}\"}} {}", r.errors);
        }
        family(
            &mut o,
            "w2db_rpc_duration_seconds",
            "summary",
            "Tempo das chamadas gRPC por método.",
        );
        for (m, r) in &rpc {
            let _ = writeln!(o, "w2db_rpc_duration_seconds_sum{{method=\"{m}\"}} {}", r.seconds);
            let _ = writeln!(o, "w2db_rpc_duration_seconds_count{{method=\"{m}\"}} {}", r.requests);
        }

        family(
            &mut o,
            "w2db_login_failures_total",
            "counter",
            "Logins e senhas numéricas recusados, pelo motivo.",
        );
        for (reason, c) in [
            ("wrong_password", &self.login_wrong_password),
            ("locked", &self.login_locked),
            ("no_account", &self.login_no_account),
            ("wrong_numeric", &self.numeric_wrong),
        ] {
            let _ = writeln!(o, "w2db_login_failures_total{{reason=\"{reason}\"}} {}", v(c));
        }
        for (name, help, c) in [
            (
                "w2db_dupe_suspects_total",
                "Itens gravados como suspeita de duplicação (evento dupe_suspect).",
                &self.dupe_suspects,
            ),
            (
                "w2db_version_conflicts_total",
                "Gravações recusadas por versão da conta desatualizada.",
                &self.version_conflicts,
            ),
            ("w2db_bug_reports_total", "Relatos do /bug gravados.", &self.bug_reports),
            (
                "w2db_bug_reports_limited_total",
                "Relatos do /bug recusados pelo limite por personagem.",
                &self.bug_reports_limited,
            ),
            (
                "w2db_discord_posts_total",
                "Relatos postados no Discord.",
                &self.discord_posts,
            ),
            (
                "w2db_discord_post_failures_total",
                "Tentativas de postar no Discord que falharam.",
                &self.discord_post_failures,
            ),
        ] {
            family(&mut o, name, "counter", help);
            let _ = writeln!(o, "{name} {}", v(c));
        }

        let up = sqlx::query_scalar::<_, i32>("SELECT 1").fetch_one(pool).await.is_ok();
        family(&mut o, "w2db_database_up", "gauge", "O Postgres respondeu na coleta.");
        let _ = writeln!(o, "w2db_database_up {}", up as i32);
        if !up {
            return o;
        }
        if let Ok(n) = store::bug_queue_len(pool).await {
            family(
                &mut o,
                "w2db_discord_queue",
                "gauge",
                "Relatos do /bug esperando para ir ao Discord.",
            );
            let _ = writeln!(o, "w2db_discord_queue {n}");
        }
        // O WAL vai para o repositório do pgBackRest pelo archive_command; falha aqui é backup parado.
        let archiver = sqlx::query_as::<_, (i64, i64, Option<f64>, Option<f64>)>(
            "SELECT archived_count, failed_count,
                    extract(epoch FROM last_archived_time)::float8,
                    extract(epoch FROM last_failed_time)::float8
             FROM pg_stat_archiver",
        )
        .fetch_one(pool)
        .await;
        if let Ok((archived, failed, last_ok, last_fail)) = archiver {
            family(
                &mut o,
                "w2db_wal_archived_total",
                "counter",
                "Segmentos de WAL arquivados (pg_stat_archiver).",
            );
            let _ = writeln!(o, "w2db_wal_archived_total {archived}");
            family(
                &mut o,
                "w2db_wal_archive_failures_total",
                "counter",
                "Falhas do archive_command (pg_stat_archiver).",
            );
            let _ = writeln!(o, "w2db_wal_archive_failures_total {failed}");
            family(
                &mut o,
                "w2db_wal_last_archived_timestamp_seconds",
                "gauge",
                "Último WAL arquivado (Unix; 0 se nenhum).",
            );
            let _ = writeln!(o, "w2db_wal_last_archived_timestamp_seconds {}", last_ok.unwrap_or(0.0));
            family(
                &mut o,
                "w2db_wal_last_failed_timestamp_seconds",
                "gauge",
                "Última falha do arquivamento (Unix; 0 se nenhuma).",
            );
            let _ = writeln!(o, "w2db_wal_last_failed_timestamp_seconds {}", last_fail.unwrap_or(0.0));
        }
        o
    }
}

/// Camada que conta e cronometra cada RPC.
#[derive(Clone, Copy, Default)]
pub struct RpcLayer;

impl<S> tower::Layer<S> for RpcLayer {
    type Service = RpcService<S>;
    fn layer(&self, inner: S) -> Self::Service {
        RpcService(inner)
    }
}

#[derive(Clone)]
pub struct RpcService<S>(S);

impl<S, B, RB> tower::Service<http::Request<B>> for RpcService<S>
where
    S: tower::Service<http::Request<B>, Response = http::Response<RB>>,
    S::Future: Send + 'static,
{
    type Response = S::Response;
    type Error = S::Error;
    type Future = Pin<Box<dyn Future<Output = Result<Self::Response, Self::Error>> + Send>>;

    fn poll_ready(&mut self, cx: &mut Context<'_>) -> Poll<Result<(), Self::Error>> {
        self.0.poll_ready(cx)
    }

    fn call(&mut self, req: http::Request<B>) -> Self::Future {
        let method = req.uri().path().rsplit('/').next().unwrap_or_default().to_string();
        let start = Instant::now();
        let fut = self.0.call(req);
        Box::pin(async move {
            let r = fut.await;
            // O tonic manda o erro só nos cabeçalhos (grpc-status); o OK vai nos trailers.
            let ok = match &r {
                Ok(rsp) => rsp.headers().get("grpc-status").is_none_or(|s| s == "0"),
                Err(_) => false,
            };
            METRICS.rpc(&method, start.elapsed(), ok);
            r
        })
    }
}

/// Abre o endereço do `/metrics`.
pub async fn bind(addr: SocketAddr) -> anyhow::Result<TcpListener> {
    let l = TcpListener::bind(addr).await?;
    tracing::info!("métricas em http://{}/metrics", l.local_addr()?);
    Ok(l)
}

/// Serve `GET /metrics` (só HTTP/1, sem TLS: o endereço é local, para o Prometheus da mesma
/// máquina).
pub async fn serve(listener: TcpListener, pool: PgPool) {
    loop {
        let (stream, _) = match listener.accept().await {
            Ok(s) => s,
            Err(e) => {
                tracing::warn!("métricas: accept: {e}");
                tokio::time::sleep(Duration::from_millis(100)).await;
                continue;
            }
        };
        let pool = pool.clone();
        tokio::spawn(async move {
            let svc = service_fn(move |req: http::Request<Incoming>| {
                let pool = pool.clone();
                async move { Ok::<_, Infallible>(respond(&req, &pool).await) }
            });
            let _ = hyper::server::conn::http1::Builder::new()
                .timer(TokioTimer::new())
                .header_read_timeout(Duration::from_secs(10))
                .serve_connection(TokioIo::new(stream), svc)
                .await;
        });
    }
}

async fn respond(req: &http::Request<Incoming>, pool: &PgPool) -> http::Response<Full<Bytes>> {
    let rsp = http::Response::builder();
    if req.method() != http::Method::GET || req.uri().path() != "/metrics" {
        return rsp
            .status(http::StatusCode::NOT_FOUND)
            .body(Full::new(Bytes::from_static(b"not found\n")))
            .unwrap_or_default();
    }
    rsp.header(http::header::CONTENT_TYPE, "text/plain; version=0.0.4; charset=utf-8")
        .body(Full::new(Bytes::from(METRICS.render(pool).await)))
        .unwrap_or_default()
}
