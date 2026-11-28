//! O site e o bot do w2gm contra um Postgres de teste: login de GM com TOTP, sessão e CSRF,
//! cadastro pelo link do /registrar e os cabeçalhos de segurança.

use axum::Router;
use axum::body::Body;
use axum::http::{Request, StatusCode, header};
use http_body_util::BodyExt;
use sqlx::PgPool;
use tower::ServiceExt;

use w2gm::{bot, gmauth, web};

fn app(pool: &PgPool) -> Router {
    web::router(web::AppState::new(pool.clone()))
}

struct Reply {
    status: StatusCode,
    headers: axum::http::HeaderMap,
    body: String,
}

async fn send(app: &Router, req: Request<Body>) -> Reply {
    let res = app.clone().oneshot(req).await.unwrap();
    let status = res.status();
    let headers = res.headers().clone();
    let body = String::from_utf8(res.into_body().collect().await.unwrap().to_bytes().to_vec()).unwrap();
    Reply { status, headers, body }
}

fn get(uri: &str, cookie: Option<&str>) -> Request<Body> {
    let mut r = Request::get(uri);
    if let Some(c) = cookie {
        r = r.header(header::COOKIE, c);
    }
    r.body(Body::empty()).unwrap()
}

fn post(uri: &str, cookie: Option<&str>, form: &[(&str, &str)]) -> Request<Body> {
    let body: Vec<String> = form.iter().map(|(k, v)| format!("{k}={}", urlencode(v))).collect();
    let mut r = Request::post(uri).header(header::CONTENT_TYPE, "application/x-www-form-urlencoded");
    if let Some(c) = cookie {
        r = r.header(header::COOKIE, c);
    }
    r.body(Body::from(body.join("&"))).unwrap()
}

fn urlencode(s: &str) -> String {
    s.bytes()
        .map(|b| match b {
            b'A'..=b'Z' | b'a'..=b'z' | b'0'..=b'9' | b'-' | b'_' | b'.' => (b as char).to_string(),
            _ => format!("%{b:02X}"),
        })
        .collect()
}

fn now() -> u64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .unwrap()
        .as_secs()
}

fn code(secret: &[u8], at: u64) -> String {
    gmauth::totp(secret).unwrap().generate(at).to_string()
}

/// O valor `csrf` do primeiro formulário da página.
fn csrf(body: &str) -> String {
    let at = body.find("name=\"csrf\" value=\"").expect("página sem csrf") + 19;
    body[at..at + body[at..].find('"').unwrap()].to_string()
}

/// Entra como GM e devolve o cabeçalho Cookie da sessão.
async fn gm_session(app: &Router, pool: &PgPool) -> String {
    let secret = gmauth::add_gm(pool, "chefe", "senha-comprida-1").await.unwrap();
    let r = send(
        app,
        post(
            "/gm/login",
            None,
            &[
                ("name", "chefe"),
                ("password", "senha-comprida-1"),
                ("code", &code(&secret, now())),
            ],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::SEE_OTHER, "{}", r.body);
    let set = r.headers[header::SET_COOKIE].to_str().unwrap();
    assert!(set.starts_with("__Host-w2gm="), "{set}");
    for attr in ["Secure", "HttpOnly", "SameSite=Strict", "Path=/"] {
        assert!(set.contains(attr), "{set} sem {attr}");
    }
    set.split(';').next().unwrap().to_string()
}

#[sqlx::test(migrations = "../w2db/migrations")]
async fn gm_login_totp_and_csrf(pool: PgPool) {
    let app = app(&pool);

    // Sem sessão, o painel manda para o login.
    let r = send(&app, get("/gm", None)).await;
    assert_eq!(r.status, StatusCode::SEE_OTHER);
    assert_eq!(r.headers[header::LOCATION], "/gm/login");

    assert!(
        gmauth::add_gm(&pool, "curto", "123").await.is_err(),
        "senha de GM curta"
    );
    let secret = gmauth::add_gm(&pool, "chefe", "senha-comprida-1").await.unwrap();
    let t = now();

    // Senha certa e código errado; senha errada e código certo; nome que não existe.
    let wrong_code = format!("{:06}", (code(&secret, t).parse::<u32>().unwrap() + 1) % 1_000_000);
    for form in [
        [
            ("name", "chefe"),
            ("password", "senha-comprida-1"),
            ("code", wrong_code.as_str()),
        ],
        [
            ("name", "chefe"),
            ("password", "errada-errada"),
            ("code", &code(&secret, t)),
        ],
        [
            ("name", "ninguem"),
            ("password", "senha-comprida-1"),
            ("code", &code(&secret, t)),
        ],
    ] {
        let r = send(&app, post("/gm/login", None, &form)).await;
        assert_eq!(r.status, StatusCode::UNAUTHORIZED);
        assert!(r.body.contains("Nome, senha ou código errado."));
        assert!(!r.headers.contains_key(header::SET_COOKIE));
    }

    // O código certo entra uma vez só.
    let ok = code(&secret, t);
    let form = [
        ("name", "chefe"),
        ("password", "senha-comprida-1"),
        ("code", ok.as_str()),
    ];
    let r = send(&app, post("/gm/login", None, &form)).await;
    assert_eq!(r.status, StatusCode::SEE_OTHER);
    let cookie = r.headers[header::SET_COOKIE]
        .to_str()
        .unwrap()
        .split(';')
        .next()
        .unwrap()
        .to_string();
    let r = send(&app, post("/gm/login", None, &form)).await;
    assert_eq!(r.status, StatusCode::UNAUTHORIZED, "o mesmo código duas vezes");

    let r = send(&app, get("/gm", Some(&cookie))).await;
    assert_eq!(r.status, StatusCode::OK);
    assert!(r.body.contains("Painel de GM") && r.body.contains("Sair (chefe)"));

    // POST sem o token da sessão é recusado; com ele, passa.
    let r = send(&app, get("/gm/convites", Some(&cookie))).await;
    let token = csrf(&r.body);
    let r = send(
        &app,
        post(
            "/gm/convites",
            Some(&cookie),
            &[("csrf", "outro"), ("discord_id", "777")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::FORBIDDEN);
    let r = send(
        &app,
        post(
            "/gm/convites",
            Some(&cookie),
            &[("csrf", &token), ("discord_id", "777")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::SEE_OTHER);
    let r = send(&app, get("/gm/convites", Some(&cookie))).await;
    assert!(r.body.contains("777") && r.body.contains("gm:chefe") && r.body.contains("aberto"));
    let r = send(&app, get("/gm/registro", Some(&cookie))).await;
    assert!(r.body.contains("invite"));

    // Sair derruba a sessão.
    let r = send(&app, post("/gm/sair", Some(&cookie), &[("csrf", &token)])).await;
    assert_eq!(r.status, StatusCode::SEE_OTHER);
    assert_eq!(
        send(&app, get("/gm", Some(&cookie))).await.status,
        StatusCode::SEE_OTHER
    );

    // Erros seguidos bloqueiam a conta de GM, mesmo com o código certo depois.
    sqlx::query("UPDATE gm_users SET totp_last_step = 0")
        .execute(&pool)
        .await
        .unwrap();
    for _ in 0..5 {
        gmauth::check_login(&pool, "chefe", "errada", "000000", t)
            .await
            .unwrap();
    }
    assert_eq!(
        gmauth::check_login(&pool, "chefe", "senha-comprida-1", &code(&secret, t), t)
            .await
            .unwrap(),
        gmauth::Login::Locked
    );

    // Conta de GM desativada: as sessões caem.
    let cookie = {
        sqlx::query("UPDATE gm_users SET locked_until = NULL, totp_last_step = 0")
            .execute(&pool)
            .await
            .unwrap();
        let r = send(&app, post("/gm/login", None, &form)).await;
        r.headers[header::SET_COOKIE]
            .to_str()
            .unwrap()
            .split(';')
            .next()
            .unwrap()
            .to_string()
    };
    assert_eq!(send(&app, get("/gm", Some(&cookie))).await.status, StatusCode::OK);
    assert!(w2db::admin::gm_disable(&pool, "chefe").await.unwrap());
    assert_eq!(
        send(&app, get("/gm", Some(&cookie))).await.status,
        StatusCode::SEE_OTHER
    );
}

#[sqlx::test(migrations = "../w2db/migrations")]
async fn registration_through_the_bot_link(pool: PgPool) {
    let app = app(&pool);

    assert!(
        bot::invite_reply(&pool, 1, false, 500, false)
            .await
            .contains("Só quem tem o cargo")
    );
    assert!(bot::invite_reply(&pool, 1, true, 500, true).await.contains("Bots não"));
    assert!(
        bot::register_reply(&pool, "https://gm.exemplo", 500)
            .await
            .contains("ainda não tem convite")
    );
    assert!(
        bot::invite_reply(&pool, 1, true, 500, false)
            .await
            .contains("Convite criado para <@500>")
    );

    let text = bot::register_reply(&pool, "https://gm.exemplo/", 500).await;
    let link = text.lines().last().unwrap();
    let path = link.strip_prefix("https://gm.exemplo").expect(link);
    assert!(path.starts_with("/registrar/") && path.len() > 40, "{path}");

    let r = send(&app, get(path, None)).await;
    assert_eq!(r.status, StatusCode::OK);
    assert!(r.body.contains("Criar conta"));
    let r = send(
        &app,
        post(
            path,
            None,
            &[("name", "Lia"), ("password", "abc123"), ("password2", "abc124")],
        ),
    )
    .await;
    assert!(r.body.contains("As senhas não são iguais."));
    let r = send(
        &app,
        post(
            path,
            None,
            &[("name", "Lia"), ("password", "abc123"), ("password2", "abc123")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::OK, "{}", r.body);
    assert!(r.body.contains("Conta criada"));
    let r = send(&app, get(path, None)).await;
    assert_eq!(r.status, StatusCode::NOT_FOUND, "o link é de uso único");
    let r = send(
        &app,
        post(
            path,
            None,
            &[("name", "Lia2"), ("password", "abc123"), ("password2", "abc123")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::NOT_FOUND);

    let a = w2db::store::account_auth(&pool, b"LIA")
        .await
        .unwrap()
        .expect("conta criada");
    assert!(w2db::auth::verify(b"abc123", &a.password_hash));
    assert!(
        bot::register_reply(&pool, "https://gm.exemplo", 500)
            .await
            .contains("ainda não tem convite")
    );
    let r = send(&app, get("/registrar/inventado", None)).await;
    assert_eq!(r.status, StatusCode::NOT_FOUND);
}

#[sqlx::test(migrations = "../w2db/migrations")]
async fn security_headers_and_pages(pool: PgPool) {
    let app = app(&pool);
    let r = send(&app, get("/gm/login", None)).await;
    assert_eq!(r.status, StatusCode::OK);
    for (h, v) in [
        ("content-security-policy", "default-src 'none'"),
        ("referrer-policy", "no-referrer"),
        ("x-frame-options", "DENY"),
        ("x-content-type-options", "nosniff"),
        ("cache-control", "no-store"),
        ("strict-transport-security", "max-age="),
    ] {
        assert!(r.headers[h].to_str().unwrap().contains(v), "{h}");
    }
    assert!(!r.body.contains("<script"), "sem JavaScript");
    let r = send(&app, get("/static/gm.css", None)).await;
    assert!(
        r.headers[header::CONTENT_TYPE]
            .to_str()
            .unwrap()
            .starts_with("text/css")
    );
    assert_eq!(send(&app, get("/nada", None)).await.status, StatusCode::NOT_FOUND);

    // O painel com uma conta: busca, conta, item; HTML do jogador sai escapado.
    let cookie = gm_session(&app, &pool).await;
    w2db::admin::create_invite(&pool, "900", "cli").await.unwrap();
    let w2db::admin::Registration::Link(t) = w2db::admin::start_registration(&pool, "900").await.unwrap() else {
        panic!()
    };
    w2db::admin::complete_registration(&pool, &t, b"Rui", b"x")
        .await
        .unwrap();
    sqlx::query("UPDATE accounts SET ban_reason = '<b>oi</b>', banned_until = now() + interval '1 day'")
        .execute(&pool)
        .await
        .unwrap();
    let r = send(&app, get("/gm/busca?q=ru", Some(&cookie))).await;
    assert!(
        r.body.contains(">Rui</a>") && r.body.contains("bloqueada"),
        "{}",
        r.body
    );
    let id: i64 = sqlx::query_scalar("SELECT id FROM accounts WHERE name = 'Rui'")
        .fetch_one(&pool)
        .await
        .unwrap();
    let r = send(&app, get(&format!("/gm/conta/{id}"), Some(&cookie))).await;
    assert_eq!(r.status, StatusCode::OK);
    assert!(r.body.contains("&lt;b&gt;oi&lt;/b&gt;") && !r.body.contains("<b>oi</b>"));
    let token = csrf(&r.body);
    let r = send(
        &app,
        post(
            &format!("/gm/conta/{id}/bloqueio"),
            Some(&cookie),
            &[("csrf", &token), ("action", "unban")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::SEE_OTHER);
    let banned: bool = sqlx::query_scalar("SELECT banned_until IS NOT NULL FROM accounts WHERE id = $1")
        .bind(id)
        .fetch_one(&pool)
        .await
        .unwrap();
    assert!(!banned);

    // Rollback com a conta no jogo é recusado, sem confirmação também.
    sqlx::query("UPDATE accounts SET online_since = now()")
        .execute(&pool)
        .await
        .unwrap();
    let version: i64 = sqlx::query_scalar("SELECT max(version) FROM account_versions")
        .fetch_one(&pool)
        .await
        .unwrap();
    let v = version.to_string();
    let r = send(
        &app,
        post(
            &format!("/gm/conta/{id}/rollback"),
            Some(&cookie),
            &[("csrf", &token), ("version", &v), ("slot", "0")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::BAD_REQUEST);
    let r = send(
        &app,
        post(
            &format!("/gm/conta/{id}/rollback"),
            Some(&cookie),
            &[("csrf", &token), ("version", &v), ("slot", "0"), ("confirm", "1")],
        ),
    )
    .await;
    assert_eq!(r.status, StatusCode::CONFLICT);
    assert!(r.body.contains("está no jogo"));
    let r = send(&app, get("/gm/item/123456", Some(&cookie))).await;
    assert!(r.body.contains("não existe mais"));
}
