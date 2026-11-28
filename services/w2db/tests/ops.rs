//! Operação (docs/OPS.md): `/metrics`, os relatos do `/bug` no Discord (com um webhook falso) e o
//! `w2db check`.

use std::convert::Infallible;
use std::sync::{Arc, Mutex};

use bytes::Bytes;
use http_body_util::{BodyExt, Full};
use hyper::body::Incoming;
use hyper::service::service_fn;
use hyper_util::rt::TokioIo;
use sqlx::PgPool;
use tokio::net::{TcpListener, UnixListener, UnixStream};
use tokio_stream::wrappers::UnixListenerStream;
use tonic::transport::{Channel, Endpoint};
use w2db::pb::{self, Status, w2_db_client::W2DbClient, w2_db_server::W2DbServer};
use w2db::{check, discord, metrics, service::Service, store};

type Client = W2DbClient<Channel>;

/// O serviço como no `serve`, com a camada das métricas.
async fn start(pool: PgPool) -> Client {
    let dir = std::env::temp_dir().join(format!("w2db-ops-{}-{}", std::process::id(), getrandom::u64().unwrap()));
    std::fs::create_dir_all(&dir).unwrap();
    let sock = dir.join("w2db.sock");
    let listener = UnixListener::bind(&sock).unwrap();
    tokio::spawn(
        tonic::transport::Server::builder()
            .layer(metrics::RpcLayer)
            .add_service(W2DbServer::new(Service::new(pool, false)))
            .serve_with_incoming(UnixListenerStream::new(listener)),
    );
    let channel = Endpoint::try_from("http://w2db.local")
        .unwrap()
        .connect_with_connector(tower::service_fn(move |_| {
            let path = sock.clone();
            async move { Ok::<_, std::io::Error>(TokioIo::new(UnixStream::connect(path).await?)) }
        }))
        .await
        .unwrap();
    W2DbClient::new(channel)
}

async fn create(c: &mut Client, name: &str) {
    let r = c
        .create_account(pb::CreateAccountRequest {
            name: name.as_bytes().to_vec(),
            password: b"senha".to_vec(),
            bonus: true,
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(r.status, Status::Ok as i32);
}

fn value(text: &str, series: &str) -> Option<f64> {
    text.lines()
        .find_map(|l| l.strip_prefix(series).and_then(|v| v.strip_prefix(' ')))
        .and_then(|v| v.trim().parse().ok())
}

#[sqlx::test]
async fn metrics_endpoint(pool: PgPool) {
    let mut c = start(pool.clone()).await;
    let listener = metrics::bind("127.0.0.1:0".parse().unwrap()).await.unwrap();
    let addr = listener.local_addr().unwrap();
    tokio::spawn(metrics::serve(listener, pool.clone()));

    c.health(pb::HealthRequest {}).await.unwrap();
    create(&mut c, "Metrica").await;
    let wrong = c
        .account_login(pb::AccountLoginRequest {
            name: b"Metrica".to_vec(),
            password: b"errada".to_vec(),
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(wrong.status, Status::WrongPassword as i32);
    // Erro de gRPC (não um status do w2db): conta como erro do método.
    assert!(
        c.create_invite(pb::CreateInviteRequest {
            discord_id: String::new()
        })
        .await
        .is_err()
    );

    let base = format!("http://{addr}");
    let rsp = reqwest::get(format!("{base}/metrics")).await.unwrap();
    assert!(rsp.status().is_success());
    assert!(
        rsp.headers()[reqwest::header::CONTENT_TYPE]
            .to_str()
            .unwrap()
            .starts_with("text/plain; version=0.0.4")
    );
    let text = rsp.text().await.unwrap();
    assert_eq!(
        value(&text, "w2db_rpc_requests_total{method=\"Health\"}"),
        Some(1.0),
        "{text}"
    );
    assert_eq!(value(&text, "w2db_rpc_errors_total{method=\"Health\"}"), Some(0.0));
    assert_eq!(
        value(&text, "w2db_rpc_errors_total{method=\"CreateInvite\"}"),
        Some(1.0)
    );
    assert_eq!(
        value(&text, "w2db_rpc_duration_seconds_count{method=\"AccountLogin\"}"),
        Some(1.0)
    );
    assert_eq!(
        value(&text, "w2db_login_failures_total{reason=\"wrong_password\"}"),
        Some(1.0)
    );
    assert_eq!(value(&text, "w2db_database_up"), Some(1.0));
    assert_eq!(value(&text, "w2db_discord_queue"), Some(0.0));
    assert!(value(&text, "w2db_wal_archive_failures_total").is_some());
    // Cada família tem HELP e TYPE uma vez.
    let types = text
        .lines()
        .filter(|l| l.starts_with("# TYPE w2db_rpc_requests_total "))
        .count();
    assert_eq!(types, 1);

    let missing = reqwest::get(format!("{base}/outro")).await.unwrap();
    assert_eq!(missing.status(), reqwest::StatusCode::NOT_FOUND);
}

/// Webhook falso: guarda os corpos recebidos e responde com `status`.
async fn fake_webhook(status: u16) -> (String, Arc<Mutex<Vec<serde_json::Value>>>) {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let got = Arc::new(Mutex::new(Vec::new()));
    let seen = got.clone();
    tokio::spawn(async move {
        loop {
            let (stream, _) = listener.accept().await.unwrap();
            let seen = seen.clone();
            tokio::spawn(async move {
                let svc = service_fn(move |req: http::Request<Incoming>| {
                    let seen = seen.clone();
                    async move {
                        let body = req.into_body().collect().await.unwrap().to_bytes();
                        seen.lock().unwrap().push(serde_json::from_slice(&body).unwrap());
                        Ok::<_, Infallible>(
                            http::Response::builder()
                                .status(status)
                                .body(Full::new(Bytes::new()))
                                .unwrap(),
                        )
                    }
                });
                let _ = hyper::server::conn::http1::Builder::new()
                    .serve_connection(TokioIo::new(stream), svc)
                    .await;
            });
        }
    });
    (format!("http://{addr}/api/webhooks/1/segredo"), got)
}

fn bug(name: &str, text: &[u8]) -> pb::ReportBugRequest {
    pb::ReportBugRequest {
        account_name: b"NINGUEM".to_vec(),
        character_name: name.as_bytes().to_vec(),
        server: 0,
        pos_x: 2100,
        pos_y: 2101,
        text: text.to_vec(),
    }
}

#[sqlx::test]
async fn bug_reports_go_to_discord(pool: PgPool) {
    store::report_bug(&pool, &bug("Fulano", b"@everyone caiu do mapa"))
        .await
        .unwrap();
    store::report_bug(&pool, &bug("Beltrano", b"loja n\xe3o abre"))
        .await
        .unwrap();

    // Webhook fora do ar: o relato fica na fila com a tentativa contada.
    let (down, _) = fake_webhook(500).await;
    let w = discord::Webhook::new(down).unwrap();
    assert_eq!(w.post_pending(&pool).await.unwrap(), 0);
    assert_eq!(store::bug_queue_len(&pool).await.unwrap(), 2);
    let attempts: Vec<i32> = sqlx::query_scalar("SELECT post_attempts FROM bug_reports ORDER BY id")
        .fetch_all(&pool)
        .await
        .unwrap();
    assert_eq!(attempts, vec![1, 1]);

    let (up, got) = fake_webhook(204).await;
    let w = discord::Webhook::new(up).unwrap();
    assert_eq!(w.post_pending(&pool).await.unwrap(), 2);
    assert_eq!(w.post_pending(&pool).await.unwrap(), 0, "o que foi postado sai da fila");
    assert_eq!(store::bug_queue_len(&pool).await.unwrap(), 0);
    let got = got.lock().unwrap().clone();
    assert_eq!(got.len(), 2);
    let first = got[0]["content"].as_str().unwrap();
    assert!(first.contains("Fulano"), "{first}");
    assert!(first.contains("\\@everyone caiu do mapa"), "{first}");
    assert_eq!(got[0]["allowed_mentions"]["parse"], serde_json::json!([]));
    assert!(got[1]["content"].as_str().unwrap().contains("loja não abre"));

    // Quem desiste depois de BUG_POST_ATTEMPTS fica no banco, fora da fila.
    let id = store::report_bug(&pool, &bug("Ciclano", b"x")).await.unwrap().unwrap();
    sqlx::query("UPDATE bug_reports SET post_attempts = $2 WHERE id = $1")
        .bind(id)
        .bind(store::BUG_POST_ATTEMPTS)
        .execute(&pool)
        .await
        .unwrap();
    assert_eq!(store::bug_queue_len(&pool).await.unwrap(), 0);
}

#[sqlx::test]
async fn check_finds_what_constraints_do_not(pool: PgPool) {
    let mut c = start(pool.clone()).await;
    create(&mut c, "Conferida").await;
    create(&mut c, "Outra").await;

    let r = check::run(&pool).await.unwrap();
    assert!(r.problems.is_empty(), "{r}");
    assert!(r.ok.iter().any(|l| l.contains("2 conta(s)")), "{r}");
    assert!(r.to_string().ends_with("resultado: OK"));

    // Item sem histórico e conta com a última versão guardada errada.
    let serial: i64 = sqlx::query_scalar("SELECT min(serial) FROM items")
        .fetch_one(&pool)
        .await
        .unwrap();
    sqlx::query("DELETE FROM item_events WHERE serial = $1")
        .bind(serial)
        .execute(&pool)
        .await
        .unwrap();
    sqlx::query("UPDATE accounts SET version = version + 5 WHERE name_key = 'OUTRA'")
        .execute(&pool)
        .await
        .unwrap();
    let r = check::run(&pool).await.unwrap();
    assert_eq!(r.problems.len(), 2, "{r}");
    assert!(
        r.problems
            .iter()
            .any(|p| p.contains("sem evento") && p.contains(&serial.to_string()))
    );
    assert!(r.problems.iter().any(|p| p.contains("última versão")));
    assert!(r.to_string().ends_with("resultado: 2 problema(s)"));
}
