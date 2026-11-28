//! O site do `w2gm` (docs/GM.md): o cadastro pelo link do `/registrar` e o painel de GM.
//!
//! Fica atrás de um proxy HTTPS (o Caddy da VPS, `deploy/caddy`), ouvindo só em 127.0.0.1. Páginas
//! montadas no servidor, sem JavaScript; todo formulário do painel leva o token de CSRF da sessão.
//! O cookie da sessão é `__Host-` (só HTTPS, sem domínio), `HttpOnly` e `SameSite=Strict`.

use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

use axum::extract::{ConnectInfo, Form, FromRequestParts, Path, Query, State};
use axum::http::{HeaderMap, HeaderValue, StatusCode, header, request::Parts};
use axum::response::{IntoResponse, Redirect, Response};
use axum::routing::{get, post};
use axum::{Router, middleware};
use chrono::{DateTime, Utc};
use maud::{DOCTYPE, Markup, html};
use serde::Deserialize;
use sqlx::PgPool;

use w2db::admin::{self, Registered, Rollback};

use crate::gmauth::{self, Login};

pub const COOKIE: &str = "__Host-w2gm";
/// Tentativas de login de GM (e de cadastro) por IP a cada [`LIMIT_WINDOW`].
pub const LIMIT_TRIES: u32 = 10;
pub const LIMIT_WINDOW: Duration = Duration::from_secs(15 * 60);

pub struct AppState {
    pub pool: PgPool,
    limiter: Mutex<HashMap<String, (u32, Instant)>>,
    /// Segundos Unix agora (o teste do TOTP passa o seu relógio).
    pub clock: fn() -> u64,
}

impl AppState {
    pub fn new(pool: PgPool) -> Arc<Self> {
        Arc::new(AppState {
            pool,
            limiter: Mutex::new(HashMap::new()),
            clock: || SystemTime::now().duration_since(UNIX_EPOCH).map_or(0, |d| d.as_secs()),
        })
    }

    /// Conta uma tentativa de `who` (IP e o que tenta). `false` se passou do limite.
    fn allow(&self, who: String) -> bool {
        let mut m = self.limiter.lock().unwrap_or_else(|e| e.into_inner());
        let now = Instant::now();
        m.retain(|_, (_, t)| now.duration_since(*t) < LIMIT_WINDOW);
        let e = m.entry(who).or_insert((0, now));
        e.0 += 1;
        e.0 <= LIMIT_TRIES
    }
}

type St = Arc<AppState>;

pub fn router(state: St) -> Router {
    Router::new()
        .route("/healthz", get(|| async { "ok" }))
        .route("/static/gm.css", get(css))
        .route("/registrar/{token}", get(register_form).post(register_submit))
        .route("/gm/login", get(login_form).post(login_submit))
        .route("/gm/sair", post(logout))
        .route("/gm", get(home))
        .route("/gm/busca", get(search))
        .route("/gm/conta/{id}", get(account))
        .route("/gm/conta/{id}/rollback", post(rollback))
        .route("/gm/conta/{id}/bloqueio", post(ban))
        .route("/gm/item/{serial}", get(item))
        .route("/gm/convites", get(invites).post(invite_create))
        .route("/gm/convites/{code}/cancelar", post(invite_revoke))
        .route("/gm/registro", get(audit))
        .fallback(|| async { page_error(StatusCode::NOT_FOUND, "Página não encontrada.") })
        .layer(middleware::map_response(security_headers))
        .with_state(state)
}

async fn security_headers(mut res: Response) -> Response {
    let h = res.headers_mut();
    let set = |h: &mut HeaderMap, k: header::HeaderName, v: &'static str| {
        h.insert(k, HeaderValue::from_static(v));
    };
    set(
        h,
        header::CONTENT_SECURITY_POLICY,
        "default-src 'none'; style-src 'self'; form-action 'self'; frame-ancestors 'none'; base-uri 'none'",
    );
    // O link do cadastro leva o token na URL: nada de Referer para fora.
    set(h, header::REFERRER_POLICY, "no-referrer");
    set(h, header::X_CONTENT_TYPE_OPTIONS, "nosniff");
    set(h, header::X_FRAME_OPTIONS, "DENY");
    set(h, header::STRICT_TRANSPORT_SECURITY, "max-age=31536000");
    if !h.contains_key(header::CACHE_CONTROL) {
        set(h, header::CACHE_CONTROL, "no-store");
    }
    res
}

async fn css() -> Response {
    (
        [
            (header::CONTENT_TYPE, "text/css; charset=utf-8"),
            (header::CACHE_CONTROL, "max-age=3600"),
        ],
        include_str!("gm.css"),
    )
        .into_response()
}

// ---------------------------------------------------------------- pedaços comuns

/// O IP de quem pede. Atrás do Caddy (conexão de 127.0.0.1) é o último do `X-Forwarded-For`,
/// o que o Caddy pôs; os de antes vêm do cliente e não valem.
fn client_ip(parts: &Parts) -> String {
    let peer = parts.extensions.get::<ConnectInfo<SocketAddr>>().map(|c| c.0.ip());
    match peer {
        Some(ip) if ip.is_loopback() => parts
            .headers
            .get_all("x-forwarded-for")
            .iter()
            .filter_map(|v| v.to_str().ok())
            .flat_map(|v| v.split(','))
            .map(str::trim)
            .rfind(|s| !s.is_empty())
            .unwrap_or("127.0.0.1")
            .to_string(),
        Some(ip) => ip.to_string(),
        None => "?".to_string(),
    }
}

pub struct ClientIp(pub String);

impl<S: Send + Sync> FromRequestParts<S> for ClientIp {
    type Rejection = std::convert::Infallible;
    async fn from_request_parts(parts: &mut Parts, _: &S) -> Result<Self, Self::Rejection> {
        Ok(ClientIp(client_ip(parts)))
    }
}

fn cookie(headers: &HeaderMap, name: &str) -> Option<String> {
    headers
        .get_all(header::COOKIE)
        .iter()
        .filter_map(|v| v.to_str().ok())
        .flat_map(|v| v.split(';'))
        .filter_map(|kv| kv.trim().split_once('='))
        .find(|(k, _)| *k == name)
        .map(|(_, v)| v.to_string())
}

/// GM com sessão. Sem sessão, a página manda para o login.
pub struct Gm {
    pub name: String,
    pub csrf: String,
    token: String,
}

impl Gm {
    fn actor(&self) -> String {
        format!("gm:{}", self.name)
    }

    fn csrf_ok(&self, csrf: &str) -> bool {
        gmauth::same(&self.csrf, csrf)
    }
}

impl FromRequestParts<St> for Gm {
    type Rejection = Response;
    async fn from_request_parts(parts: &mut Parts, st: &St) -> Result<Self, Self::Rejection> {
        let login = || Redirect::to("/gm/login").into_response();
        let token = cookie(&parts.headers, COOKIE).ok_or_else(login)?;
        match admin::session_get(&st.pool, &token).await {
            Ok(Some(s)) => Ok(Gm {
                name: s.name,
                csrf: s.csrf,
                token,
            }),
            Ok(None) => Err(login()),
            Err(e) => Err(internal(e)),
        }
    }
}

fn stale_form() -> Response {
    page_error(
        StatusCode::FORBIDDEN,
        "Formulário vencido. Volte, recarregue a página e tente de novo.",
    )
}

fn internal(e: impl std::fmt::Display) -> Response {
    tracing::error!("{e}");
    page_error(
        StatusCode::INTERNAL_SERVER_ERROR,
        "Erro interno. O registro do w2gm tem o detalhe.",
    )
}

fn page_error(code: StatusCode, msg: &str) -> Response {
    (code, layout("Erro", None, html! { p.error { (msg) } })).into_response()
}

fn when(t: &DateTime<Utc>) -> String {
    t.format("%Y-%m-%d %H:%M UTC").to_string()
}

fn when_opt(t: &Option<DateTime<Utc>>) -> String {
    t.as_ref().map_or_else(|| "-".to_string(), when)
}

fn class_name(c: i16) -> &'static str {
    match c {
        0 => "TK",
        1 => "FM",
        2 => "BM",
        3 => "HT",
        _ => "?",
    }
}

fn container_name(c: i16) -> &'static str {
    match c {
        1 => "equipamento",
        2 => "inventário",
        3 => "baú",
        _ => "sem lugar",
    }
}

fn effects(e: &[i16; 6]) -> String {
    e.chunks(2)
        .filter(|p| p[0] != 0 || p[1] != 0)
        .map(|p| format!("{}:{}", p[0], p[1]))
        .collect::<Vec<_>>()
        .join(" ")
}

fn layout(title: &str, gm: Option<&Gm>, body: Markup) -> Markup {
    html! {
        (DOCTYPE)
        html lang="pt-BR" {
            head {
                meta charset="utf-8";
                meta name="viewport" content="width=device-width, initial-scale=1";
                meta name="robots" content="noindex";
                title { (title) " · derust" }
                link rel="stylesheet" href="/static/gm.css";
            }
            body {
                @if let Some(gm) = gm {
                    nav {
                        a href="/gm" { "Início" }
                        a href="/gm/convites" { "Convites" }
                        a href="/gm/registro" { "Registro" }
                        form.inline method="post" action="/gm/sair" {
                            input type="hidden" name="csrf" value=(gm.csrf);
                            button type="submit" { "Sair (" (gm.name) ")" }
                        }
                    }
                }
                main {
                    h1 { (title) }
                    (body)
                }
            }
        }
    }
}

#[derive(Deserialize)]
struct CsrfForm {
    csrf: String,
}

// ---------------------------------------------------------------- cadastro

#[derive(Deserialize)]
struct RegisterForm {
    name: String,
    password: String,
    password2: String,
}

fn register_page(token: &str, error: Option<&str>) -> Markup {
    layout(
        "Criar conta",
        None,
        html! {
            @if let Some(e) = error { p.error { (e) } }
            form method="post" action={ "/registrar/" (token) } {
                label { "Nome da conta" input name="name" required maxlength="15" pattern="[A-Za-z0-9]{1,15}" autocomplete="username"; }
                p.hint { "Até 15 letras e números, sem acento. É o nome que você digita para entrar no jogo." }
                label { "Senha" input type="password" name="password" required maxlength="15" autocomplete="new-password"; }
                label { "Repita a senha" input type="password" name="password2" required maxlength="15" autocomplete="new-password"; }
                p.hint { "Até 15 caracteres. A senha numérica é pedida no primeiro login, dentro do jogo." }
                button type="submit" { "Criar conta" }
            }
        },
    )
}

const BAD_LINK: &str = "Este link venceu ou já foi usado. Peça outro com /registrar no Discord.";

async fn register_form(State(st): State<St>, Path(token): Path<String>) -> Response {
    match admin::registration_discord(&st.pool, &token).await {
        Ok(Some(_)) => register_page(&token, None).into_response(),
        Ok(None) => page_error(StatusCode::NOT_FOUND, BAD_LINK),
        Err(e) => internal(e),
    }
}

async fn register_submit(
    State(st): State<St>,
    ClientIp(ip): ClientIp,
    Path(token): Path<String>,
    Form(f): Form<RegisterForm>,
) -> Response {
    if !st.allow(format!("register {ip}")) {
        return page_error(
            StatusCode::TOO_MANY_REQUESTS,
            "Muitas tentativas. Espere alguns minutos.",
        );
    }
    // O link é conferido antes do argon2id: sem link, nada de hash.
    match admin::registration_discord(&st.pool, &token).await {
        Ok(Some(_)) => {}
        Ok(None) => return page_error(StatusCode::NOT_FOUND, BAD_LINK),
        Err(e) => return internal(e),
    }
    if f.password != f.password2 {
        return register_page(&token, Some("As senhas não são iguais.")).into_response();
    }
    match admin::complete_registration(&st.pool, &token, f.name.as_bytes(), f.password.as_bytes()).await {
        Ok(Registered::Created) => layout(
            "Conta criada",
            None,
            html! {
                p { "A conta " strong { (f.name) } " está pronta. Entre no jogo com ela e com a senha que você escolheu." }
                p { "Este link não vale mais." }
            },
        )
        .into_response(),
        Ok(Registered::BadLink) => page_error(StatusCode::NOT_FOUND, BAD_LINK),
        Ok(Registered::InvalidName) => {
            register_page(&token, Some("Nome inválido: use de 1 a 15 letras e números, sem acento.")).into_response()
        }
        Ok(Registered::InvalidPassword) => {
            register_page(&token, Some("Senha inválida: de 1 a 15 caracteres.")).into_response()
        }
        Ok(Registered::NameTaken) => {
            register_page(&token, Some("Já existe uma conta com este nome. Escolha outro.")).into_response()
        }
        Err(e) => internal(e),
    }
}

// ---------------------------------------------------------------- login de GM

#[derive(Deserialize)]
struct LoginForm {
    name: String,
    password: String,
    code: String,
}

fn login_page(error: Option<&str>) -> Markup {
    layout(
        "Painel de GM",
        None,
        html! {
            @if let Some(e) = error { p.error { (e) } }
            form method="post" action="/gm/login" {
                label { "Nome" input name="name" required autocomplete="username"; }
                label { "Senha" input type="password" name="password" required autocomplete="current-password"; }
                label { "Código do autenticador" input name="code" required inputmode="numeric" pattern="[0-9]{6}" autocomplete="one-time-code"; }
                button type="submit" { "Entrar" }
            }
        },
    )
}

async fn login_form() -> Markup {
    login_page(None)
}

async fn login_submit(State(st): State<St>, ClientIp(ip): ClientIp, Form(f): Form<LoginForm>) -> Response {
    if !st.allow(format!("login {ip}")) {
        tracing::warn!(%ip, "login de GM: limite de tentativas");
        return (
            StatusCode::TOO_MANY_REQUESTS,
            login_page(Some("Muitas tentativas. Espere alguns minutos.")),
        )
            .into_response();
    }
    match gmauth::check_login(&st.pool, &f.name, &f.password, &f.code, (st.clock)()).await {
        Ok(Login::Ok(id)) => {
            let (Ok(token), Ok(csrf)) = (admin::new_token(), admin::new_token()) else {
                return internal("getrandom");
            };
            if let Err(e) = admin::session_create(&st.pool, id, &token, &csrf, gmauth::SESSION_HOURS).await {
                return internal(e);
            }
            tracing::info!(%ip, gm = %f.name, "login de GM");
            let cookie = format!(
                "{COOKIE}={token}; Path=/; Secure; HttpOnly; SameSite=Strict; Max-Age={}",
                gmauth::SESSION_HOURS * 3600
            );
            ([(header::SET_COOKIE, cookie)], Redirect::to("/gm")).into_response()
        }
        Ok(Login::Denied) => {
            tracing::warn!(%ip, gm = %f.name, "login de GM recusado");
            (
                StatusCode::UNAUTHORIZED,
                login_page(Some("Nome, senha ou código errado.")),
            )
                .into_response()
        }
        Ok(Login::Locked) => (
            StatusCode::UNAUTHORIZED,
            login_page(Some("Conta de GM bloqueada por erros seguidos. Espere 15 minutos.")),
        )
            .into_response(),
        Err(e) => internal(e),
    }
}

async fn logout(State(st): State<St>, gm: Gm, Form(f): Form<CsrfForm>) -> Response {
    if !gm.csrf_ok(&f.csrf) {
        return stale_form();
    }
    if let Err(e) = admin::session_delete(&st.pool, &gm.token).await {
        return internal(e);
    }
    let cookie = format!("{COOKIE}=; Path=/; Secure; HttpOnly; SameSite=Strict; Max-Age=0");
    ([(header::SET_COOKIE, cookie)], Redirect::to("/gm/login")).into_response()
}

// ---------------------------------------------------------------- painel

fn search_box(q: &str) -> Markup {
    html! {
        form method="get" action="/gm/busca" {
            label { "Conta, personagem (começo do nome) ou id do Discord"
                input name="q" value=(q) required autofocus; }
            button type="submit" { "Buscar" }
        }
    }
}

fn audit_table(rows: &[admin::AuditRow]) -> Markup {
    html! {
        table {
            tr { th { "Quando" } th { "Quem" } th { "O quê" } th { "Alvo" } th { "Detalhe" } }
            @for r in rows {
                tr {
                    td { (when(&r.at)) }
                    td { (r.actor) }
                    td { (r.action) }
                    td { (r.target.as_deref().unwrap_or("-")) }
                    td.detail { (r.detail.to_string()) }
                }
            }
        }
    }
}

async fn home(State(st): State<St>, gm: Gm) -> Response {
    let audit = match admin::recent_audit(&st.pool, 10).await {
        Ok(a) => a,
        Err(e) => return internal(e),
    };
    layout(
        "Painel de GM",
        Some(&gm),
        html! {
            (search_box(""))
            h2 { "Últimas ações" }
            (audit_table(&audit))
        },
    )
    .into_response()
}

#[derive(Deserialize)]
struct SearchQuery {
    #[serde(default)]
    q: String,
}

async fn search(State(st): State<St>, gm: Gm, Query(q): Query<SearchQuery>) -> Response {
    let hits = match admin::search(&st.pool, &q.q, 50).await {
        Ok(h) => h,
        Err(e) => return internal(e),
    };
    layout(
        "Busca",
        Some(&gm),
        html! {
            (search_box(&q.q))
            @if hits.is_empty() { p { "Nada encontrado." } }
            @else {
                table {
                    tr { th { "Conta" } th { "Personagens" } th { "Estado" } }
                    @for h in &hits {
                        tr {
                            td { a href={ "/gm/conta/" (h.id) } { (h.name) } }
                            td { (h.characters.join(", ")) }
                            td {
                                @if h.online { span.tag.on { "no jogo" } }
                                @if h.banned { span.tag.ban { "bloqueada" } }
                            }
                        }
                    }
                }
            }
        },
    )
    .into_response()
}

async fn account(State(st): State<St>, gm: Gm, Path(id): Path<i64>) -> Response {
    let v = match admin::account_view(&st.pool, id).await {
        Ok(Some(v)) => v,
        Ok(None) => return page_error(StatusCode::NOT_FOUND, "Conta inexistente."),
        Err(e) => return internal(e),
    };
    let i = &v.info;
    let ban = admin::ban_left(i.banned_until);
    let chars: HashMap<i16, &str> = v.characters.iter().map(|c| (c.slot, c.name.as_str())).collect();
    let place = |it: &admin::ItemRow| match it.char_slot {
        Some(s) => format!(
            "{} ({})",
            container_name(it.container),
            chars.get(&s).copied().unwrap_or("?")
        ),
        None => container_name(it.container).to_string(),
    };
    layout(
        &format!("Conta {}", i.name),
        Some(&gm),
        html! {
            table.info {
                tr { th { "Discord" } td { (i.discord_id.as_deref().unwrap_or("-")) } }
                tr { th { "Criada" } td { (when(&i.created_at)) } }
                tr { th { "Último login" } td { (when_opt(&i.last_login_at)) } }
                tr { th { "Última saída" } td { (when_opt(&i.last_logout_at)) } }
                tr { th { "No jogo" } td {
                    @if let Some(t) = &i.online_since { span.tag.on { "desde " (when(t)) } } @else { "não" }
                } }
                tr { th { "Bloqueio do GM" } td {
                    @match ban {
                        Some(0) => { span.tag.ban { "para sempre" } }
                        Some(_) => { span.tag.ban { "até " (when_opt(&i.banned_until)) } }
                        None => { "não" }
                    }
                    @if let (Some(_), Some(r)) = (ban, &i.ban_reason) { " (" (r) ")" }
                } }
                tr { th { "Bloqueio por senha errada" } td { (when_opt(&i.locked_until)) } }
                tr { th { "Versão" } td { (i.version) } }
                tr { th { "Gold do baú" } td { (i.cargo_coin) } }
            }

            h2 { "Bloqueio" }
            form method="post" action={ "/gm/conta/" (i.id) "/bloqueio" } {
                input type="hidden" name="csrf" value=(gm.csrf);
                label { "Dias (vazio: para sempre)" input name="days" inputmode="numeric" pattern="[0-9]{0,5}"; }
                label { "Motivo" input name="reason" maxlength="200"; }
                button type="submit" name="action" value="ban" { "Bloquear" }
                @if ban.is_some() { button type="submit" name="action" value="unban" { "Desbloquear" } }
                p.hint { "Vale no próximo login: quem está no jogo continua até sair." }
            }

            h2 { "Personagens" }
            table {
                tr { th { "Vaga" } th { "Nome" } th { "Classe" } th { "Nível" } th { "Gold" } th { "Posição" } }
                @for c in &v.characters {
                    tr { td { (c.slot) } td { (c.name) } td { (class_name(c.class)) } td { (c.level) } td { (c.coin) } td { (c.pos_x) " " (c.pos_y) } }
                }
            }

            h2 { "Itens" }
            table {
                tr { th { "Serial" } th { "Onde" } th { "Vaga" } th { "Item" } th { "Efeitos" } th {} }
                @for it in &v.items {
                    tr {
                        td { a href={ "/gm/item/" (it.serial) } { (it.serial) } }
                        td { (place(it)) }
                        td { (it.slot) }
                        td { (it.index) }
                        td { (effects(&it.effects)) }
                        td { @if let Some(o) = &it.contested_from { span.tag.ban { "disputado com " (o) } } }
                    }
                }
            }

            h2 { "Voltar um personagem" }
            @if i.online_since.is_some() {
                p.error { "A conta está no jogo. O rollback só vale com a conta fora (a gravação seguinte do jogo o desfaria)." }
            }
            form method="post" action={ "/gm/conta/" (i.id) "/rollback" } {
                input type="hidden" name="csrf" value=(gm.csrf);
                label { "Versão" select name="version" {
                    @for ver in &v.versions {
                        option value=(ver.version) { "v" (ver.version) " · " (when(&ver.saved_at)) }
                    }
                } }
                label { "Personagem" select name="slot" {
                    @for c in &v.characters { option value=(c.slot) { (c.slot) " · " (c.name) } }
                } }
                label.check { input type="checkbox" name="confirm" value="1" required; " Confirmo: o personagem volta a como estava na versão escolhida." }
                button type="submit" { "Voltar" }
                p.hint { "Dados, equipamento e inventário do personagem voltam. O baú e os outros personagens ficam, menos os itens que voltam para ele. Itens que hoje estão com outra conta não voltam." }
            }
            table {
                tr { th { "Versão" } th { "Gravada" } th { "Personagens (vaga, nome, nível)" } }
                @for ver in &v.versions {
                    tr {
                        td { (ver.version) }
                        td { (when(&ver.saved_at)) }
                        td { @for (s, n, l) in &ver.characters { (s) " " (n) " (" (l) ")  " } }
                    }
                }
            }

            h2 { "Convites" }
            (invite_table(&gm, &v.invites))
        },
    )
    .into_response()
}

#[derive(Deserialize)]
struct RollbackForm {
    csrf: String,
    version: i64,
    slot: u32,
    #[serde(default)]
    confirm: String,
}

async fn rollback(State(st): State<St>, gm: Gm, Path(id): Path<i64>, Form(f): Form<RollbackForm>) -> Response {
    if !gm.csrf_ok(&f.csrf) {
        return stale_form();
    }
    if f.confirm != "1" {
        return page_error(
            StatusCode::BAD_REQUEST,
            "Marque a confirmação para voltar o personagem.",
        );
    }
    let out = match admin::rollback_character(&st.pool, id, f.slot, f.version, &gm.actor()).await {
        Ok(o) => o,
        Err(e) => return internal(e),
    };
    let back = html! { p { a href={ "/gm/conta/" (id) } { "Voltar para a conta" } } };
    let msg = |m: &str| {
        (
            StatusCode::CONFLICT,
            layout("Rollback não feito", Some(&gm), html! { p.error { (m) } (back) }),
        )
            .into_response()
    };
    match out {
        Rollback::Online => msg("A conta está no jogo. Espere o jogador sair (ou tire-o do jogo) e tente de novo."),
        Rollback::NoVersion => msg("Esta versão não existe mais (o banco guarda as últimas 20)."),
        Rollback::NoCharacter => msg("O personagem não existe nesta versão, ou não existe mais na conta."),
        Rollback::NameTaken => msg("O nome que o personagem tinha nesta versão hoje é de outro personagem."),
        Rollback::Done(r) => {
            tracing::info!(gm = %gm.name, account = id, slot = f.slot, version = f.version, "rollback");
            layout(
                "Rollback feito",
                Some(&gm),
                html! {
                    p { "O personagem da vaga " (f.slot) " voltou para a versão " (f.version) ". A conta agora está na versão " (r.version) "." }
                    ul {
                        li { (r.restored) " item(ns) voltaram com o serial de antes." }
                        @if !r.recreated.is_empty() {
                            li { "Voltaram com serial novo (não existiam mais): "
                                @for (old, new) in &r.recreated { (old) " → " a href={ "/gm/item/" (new) } { (new) } "  " } }
                        }
                        @if !r.elsewhere.is_empty() {
                            li { "Não voltaram, estão com outra conta: "
                                @for (s, acc) in &r.elsewhere { a href={ "/gm/item/" (s) } { (s) } " (" (acc) ")  " } }
                        }
                        @if !r.removed.is_empty() {
                            li { "Saíram do personagem (ficam no histórico, sem lugar): "
                                @for s in &r.removed { a href={ "/gm/item/" (s) } { (s) } "  " } }
                        }
                    }
                    (back)
                },
            )
            .into_response()
        }
    }
}

#[derive(Deserialize)]
struct BanForm {
    csrf: String,
    action: String,
    #[serde(default)]
    days: String,
    #[serde(default)]
    reason: String,
}

async fn ban(State(st): State<St>, gm: Gm, Path(id): Path<i64>, Form(f): Form<BanForm>) -> Response {
    if !gm.csrf_ok(&f.csrf) {
        return stale_form();
    }
    let lift = f.action == "unban";
    let until = match f.days.trim() {
        "" => None,
        d => match d.parse::<i64>() {
            Ok(n) if (1..=36500).contains(&n) => Some(Utc::now() + chrono::Duration::days(n)),
            _ => {
                return page_error(
                    StatusCode::BAD_REQUEST,
                    "Dias: um número de 1 a 36500, ou vazio para sempre.",
                );
            }
        },
    };
    let reason: String = f.reason.trim().chars().take(200).collect();
    match admin::set_ban(&st.pool, id, until, lift, &reason, &gm.actor()).await {
        Ok(()) => Redirect::to(&format!("/gm/conta/{id}")).into_response(),
        Err(w2db::store::StoreError::Invalid(m)) => page_error(StatusCode::NOT_FOUND, &m),
        Err(e) => internal(e),
    }
}

async fn item(State(st): State<St>, gm: Gm, Path(serial): Path<i64>) -> Response {
    let (place, events) = match admin::item_history(&st.pool, serial).await {
        Ok(x) => x,
        Err(e) => return internal(e),
    };
    layout(
        &format!("Item {serial}"),
        Some(&gm),
        html! {
            @match &place {
                Some(p) => p {
                    "Agora: conta " a href={ "/gm/conta/" (p.account_id) } { (p.account) } ", "
                    (container_name(p.container))
                    @if let Some(c) = &p.character { " de " (c) }
                    @if let Some(s) = p.slot { ", vaga " (s) }
                    ", item " (p.index) "."
                },
                None => p { "O item não existe mais (destruído, ou o serial nunca existiu)." },
            }
            table {
                tr { th { "Quando" } th { "Evento" } th { "Conta" } th { "De" } th { "Item" } th { "Detalhe" } }
                @for e in &events {
                    tr {
                        td { (when(&e.at)) }
                        td { (e.kind) }
                        td { (e.account.as_deref().unwrap_or("-")) }
                        td { (e.from_account.as_deref().unwrap_or("-")) }
                        td { (e.item_index.map_or_else(|| "-".to_string(), |i| i.to_string())) }
                        td.detail { (e.detail.to_string()) }
                    }
                }
            }
        },
    )
    .into_response()
}

fn invite_table(gm: &Gm, rows: &[admin::InviteRow]) -> Markup {
    html! {
        @if rows.is_empty() { p { "Nenhum convite." } }
        @else {
            table {
                tr { th { "Código" } th { "Discord" } th { "Criado por" } th { "Criado" } th { "Estado" } th {} }
                @for r in rows {
                    tr {
                        td { code { (r.code) } }
                        td { (r.discord_id) }
                        td { (r.created_by) }
                        td { (when(&r.created_at)) }
                        td {
                            @if let Some(a) = &r.account { "usado: " (a) }
                            @else if r.revoked_at.is_some() { "cancelado" }
                            @else if r.used_at.is_some() { "usado" }
                            @else { "aberto" }
                        }
                        td {
                            @if r.used_at.is_none() && r.revoked_at.is_none() {
                                form.inline method="post" action={ "/gm/convites/" (r.code) "/cancelar" } {
                                    input type="hidden" name="csrf" value=(gm.csrf);
                                    button type="submit" { "Cancelar" }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

async fn invites(State(st): State<St>, gm: Gm) -> Response {
    let rows = match admin::list_invites(&st.pool, 200).await {
        Ok(r) => r,
        Err(e) => return internal(e),
    };
    layout(
        "Convites",
        Some(&gm),
        html! {
            form method="post" action="/gm/convites" {
                input type="hidden" name="csrf" value=(gm.csrf);
                label { "Id do Discord de quem vai receber" input name="discord_id" required inputmode="numeric" pattern="[0-9]{1,32}"; }
                button type="submit" { "Criar convite" }
                p.hint { "A pessoa usa /registrar no servidor do Discord para receber o link da conta. No Discord, o /convite faz o mesmo." }
            }
            (invite_table(&gm, &rows))
        },
    )
    .into_response()
}

#[derive(Deserialize)]
struct InviteForm {
    csrf: String,
    discord_id: String,
}

async fn invite_create(State(st): State<St>, gm: Gm, Form(f): Form<InviteForm>) -> Response {
    if !gm.csrf_ok(&f.csrf) {
        return stale_form();
    }
    match admin::create_invite(&st.pool, f.discord_id.trim(), &gm.actor()).await {
        Ok(_) => Redirect::to("/gm/convites").into_response(),
        Err(w2db::store::StoreError::Invalid(m)) => page_error(StatusCode::BAD_REQUEST, &m),
        Err(e) => internal(e),
    }
}

async fn invite_revoke(State(st): State<St>, gm: Gm, Path(code): Path<String>, Form(f): Form<CsrfForm>) -> Response {
    if !gm.csrf_ok(&f.csrf) {
        return stale_form();
    }
    match admin::revoke_invite(&st.pool, &code, &gm.actor()).await {
        Ok(true) => Redirect::to("/gm/convites").into_response(),
        Ok(false) => page_error(StatusCode::NOT_FOUND, "Convite inexistente, já usado ou já cancelado."),
        Err(e) => internal(e),
    }
}

async fn audit(State(st): State<St>, gm: Gm) -> Response {
    match admin::recent_audit(&st.pool, 200).await {
        Ok(rows) => layout("Registro", Some(&gm), audit_table(&rows)).into_response(),
        Err(e) => internal(e),
    }
}
