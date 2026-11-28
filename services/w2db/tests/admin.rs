//! As operações do painel de GM e do bot (`w2db::admin`) com o jogo do outro lado: sessões do
//! DBSrv, bloqueio no login, rollback de personagem, cadastro pelo link e a busca.

use std::path::PathBuf;

use hyper_util::rt::TokioIo;
use sqlx::PgPool;
use tokio::net::{UnixListener, UnixStream};
use tokio_stream::wrappers::UnixListenerStream;
use tonic::transport::{Channel, Endpoint};
use w2db::admin::{self, Registered, Registration, Rollback};
use w2db::pb::{self, Status, w2_db_client::W2DbClient, w2_db_server::W2DbServer};
use w2db::service::Service;

type Client = W2DbClient<Channel>;

async fn start(pool: PgPool) -> (Client, PathBuf) {
    let dir = std::env::temp_dir().join(format!(
        "w2db-admin-{}-{}",
        std::process::id(),
        getrandom::u64().unwrap()
    ));
    std::fs::create_dir_all(&dir).unwrap();
    let sock = dir.join("w2db.sock");
    let listener = UnixListener::bind(&sock).unwrap();
    tokio::spawn(
        tonic::transport::Server::builder()
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
    (W2DbClient::new(channel), dir)
}

fn item(index: i32) -> pb::Item {
    pb::Item {
        index,
        effects: vec![pb::ItemEffect::default(); 3],
    }
}

fn stored(serial: u64, slot: u32, index: i32) -> pb::StoredItem {
    pb::StoredItem {
        serial,
        slot,
        item: Some(item(index)),
    }
}

fn character(slot: u32, name: &str) -> pb::Character {
    pb::Character {
        slot,
        name: name.as_bytes().to_vec(),
        mob: Some(pb::Mob {
            class: 1,
            coin: 500,
            base_score: Some(pb::Score {
                level: 7,
                ..Default::default()
            }),
            ..Default::default()
        }),
        equip: vec![stored(0, 0, 1100)],
        carry: vec![stored(0, 3, 400)],
        short_skill: vec![0xFF; 16],
        extra: Some(pb::MobExtra::default()),
        ..Default::default()
    }
}

async fn create(c: &mut Client, name: &str, pass: &str) {
    let r = c
        .create_account(pb::CreateAccountRequest {
            name: name.as_bytes().to_vec(),
            password: pass.as_bytes().to_vec(),
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(r.status, Status::Ok as i32, "criar {name}");
}

async fn login(c: &mut Client, name: &str, pass: &str) -> pb::AccountLoginReply {
    c.account_login(pb::AccountLoginRequest {
        name: name.as_bytes().to_vec(),
        password: pass.as_bytes().to_vec(),
        ip: "10.0.0.1".into(),
    })
    .await
    .unwrap()
    .into_inner()
}

async fn account(c: &mut Client, name: &str) -> pb::Account {
    c.load_account(pb::LoadAccountRequest {
        name: name.as_bytes().to_vec(),
    })
    .await
    .unwrap()
    .into_inner()
    .account
    .unwrap()
}

async fn save(c: &mut Client, acc: &pb::Account, destroyed: Vec<u64>) -> pb::SaveAccountReply {
    let r = c
        .save_account(pb::SaveAccountRequest {
            account: Some(acc.clone()),
            destroyed,
            reason: "teste".into(),
            logout: false,
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(r.status, Status::Ok as i32, "{r:?}");
    r
}

async fn new_char(c: &mut Client, acc: &str, ch: pb::Character) {
    let version = account(c, acc).await.version;
    let r = c
        .create_character(pb::CreateCharacterRequest {
            account_name: acc.as_bytes().to_vec(),
            expected_version: version,
            character: Some(ch),
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(r.status, Status::Ok as i32);
}

async fn session(c: &mut Client, name: &str, online: bool) -> i32 {
    c.set_session(pb::SetSessionRequest {
        name: name.as_bytes().to_vec(),
        online,
    })
    .await
    .unwrap()
    .into_inner()
    .status
}

async fn id_of(pool: &PgPool, name: &str) -> i64 {
    sqlx::query_scalar("SELECT id FROM accounts WHERE name = $1")
        .bind(name)
        .fetch_one(pool)
        .await
        .unwrap()
}

async fn events(pool: &PgPool, serial: u64) -> Vec<String> {
    sqlx::query_scalar("SELECT kind FROM item_events WHERE serial = $1 ORDER BY id")
        .bind(serial as i64)
        .fetch_all(pool)
        .await
        .unwrap()
}

#[sqlx::test]
async fn sessions_and_ban(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone()).await;
    create(&mut c, "Ana", "senha").await;
    let ana = id_of(&pool, "Ana").await;
    let online = |pool: PgPool| async move {
        sqlx::query_scalar::<_, bool>("SELECT online_since IS NOT NULL FROM accounts WHERE name = 'Ana'")
            .fetch_one(&pool)
            .await
            .unwrap()
    };

    assert_eq!(
        session(&mut c, "ana", true).await,
        Status::Ok as i32,
        "o nome vale sem caixa"
    );
    assert!(online(pool.clone()).await);
    assert_eq!(session(&mut c, "ANA", false).await, Status::Ok as i32);
    assert!(!online(pool.clone()).await);
    assert_eq!(session(&mut c, "Ninguem", true).await, Status::NoAccount as i32);
    session(&mut c, "ANA", true).await;
    c.reset_sessions(pb::ResetSessionsRequest {}).await.unwrap();
    assert!(!online(pool.clone()).await, "o DBSrv subiu: ninguém no jogo");

    // Bloqueio por um dia: o login certo responde BANNED com o tempo; o errado segue errado.
    let tomorrow = chrono::Utc::now() + chrono::Duration::days(1);
    admin::set_ban(&pool, ana, Some(tomorrow), false, "teste", "gm:teste")
        .await
        .unwrap();
    let r = login(&mut c, "ANA", "senha").await;
    assert_eq!(r.status, Status::Banned as i32);
    assert!(r.account.is_none());
    assert!((86_000..=86_400).contains(&r.lock_seconds), "{}", r.lock_seconds);
    assert_eq!(
        login(&mut c, "ANA", "errada").await.status,
        Status::WrongPassword as i32
    );

    // Para sempre: tempo 0.
    admin::set_ban(&pool, ana, None, false, "", "gm:teste").await.unwrap();
    let r = login(&mut c, "ANA", "senha").await;
    assert_eq!((r.status, r.lock_seconds), (Status::Banned as i32, 0));

    admin::set_ban(&pool, ana, None, true, "", "gm:teste").await.unwrap();
    assert_eq!(login(&mut c, "ANA", "senha").await.status, Status::Ok as i32);

    // Bloqueio vencido não vale.
    sqlx::query("UPDATE accounts SET banned_until = now() - interval '1 minute' WHERE id = $1")
        .bind(ana)
        .execute(&pool)
        .await
        .unwrap();
    assert_eq!(login(&mut c, "ANA", "senha").await.status, Status::Ok as i32);

    let actions: Vec<String> = sqlx::query_scalar("SELECT action FROM gm_audit WHERE target = 'Ana' ORDER BY id")
        .fetch_all(&pool)
        .await
        .unwrap();
    assert_eq!(actions, ["ban", "ban", "unban"]);
}

#[sqlx::test]
async fn rollback_character(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone()).await;
    create(&mut c, "Ana", "a").await;
    create(&mut c, "Beto", "b").await;
    new_char(&mut c, "ANA", character(0, "Ana1")).await;
    new_char(&mut c, "ANA", character(1, "Ana2")).await;
    new_char(&mut c, "BETO", character(0, "Beto1")).await;
    let ana = id_of(&pool, "Ana").await;

    // A versão boa: espada (A) no equipamento, poção (B) e um item a mais (C) no inventário.
    let mut acc = account(&mut c, "ANA").await;
    acc.characters[0].carry.push(stored(0, 5, 500));
    let good = save(&mut c, &acc, vec![]).await.version;
    let acc = account(&mut c, "ANA").await;
    let a = acc.characters[0].equip[0].serial;
    let b = acc.characters[0].carry.iter().find(|i| i.slot == 3).unwrap().serial;
    let cc = acc.characters[0].carry.iter().find(|i| i.slot == 5).unwrap().serial;

    // Depois: C consumido, B guardado no baú, A foi para o Beto, D novo, gold mudou.
    let mut acc = account(&mut c, "ANA").await;
    let ch = &mut acc.characters[0];
    ch.equip.clear();
    ch.carry = vec![stored(0, 7, 600)];
    ch.mob.as_mut().unwrap().coin = 999;
    acc.cargo.push(stored(b, 0, 400));
    save(&mut c, &acc, vec![cc]).await;
    let mut beto = account(&mut c, "BETO").await;
    beto.characters[0].carry.push(stored(a, 9, 1100));
    save(&mut c, &beto, vec![]).await;
    let d = account(&mut c, "ANA").await.characters[0].carry[0].serial;

    // No jogo, não.
    session(&mut c, "ANA", true).await;
    assert!(matches!(
        admin::rollback_character(&pool, ana, 0, good as i64, "gm:teste")
            .await
            .unwrap(),
        Rollback::Online
    ));
    session(&mut c, "ANA", false).await;
    assert!(matches!(
        admin::rollback_character(&pool, ana, 0, 999, "gm:teste").await.unwrap(),
        Rollback::NoVersion
    ));
    assert!(matches!(
        admin::rollback_character(&pool, ana, 3, good as i64, "gm:teste")
            .await
            .unwrap(),
        Rollback::NoCharacter
    ));

    let Rollback::Done(r) = admin::rollback_character(&pool, ana, 0, good as i64, "gm:teste")
        .await
        .unwrap()
    else {
        panic!("rollback recusado");
    };
    assert_eq!(r.restored, 1, "{r:?}");
    assert_eq!(r.elsewhere, [(a, "Beto".to_string())]);
    assert_eq!(r.removed, [d]);
    assert_eq!(r.recreated.len(), 1);
    assert_eq!(r.recreated[0].0, cc);
    let new_c = r.recreated[0].1;

    let acc = account(&mut c, "ANA").await;
    assert_eq!(acc.version, r.version);
    let ch = &acc.characters[0];
    assert_eq!(ch.mob.as_ref().unwrap().coin, 500, "os dados do personagem voltaram");
    assert!(ch.equip.is_empty(), "a espada está com o Beto e fica com ele");
    let carry: Vec<(u32, u64, i32)> = ch
        .carry
        .iter()
        .map(|i| (i.slot, i.serial, i.item.as_ref().unwrap().index))
        .collect();
    assert_eq!(carry, [(3, b, 400), (5, new_c, 500)]);
    assert!(acc.cargo.is_empty(), "a poção saiu do baú e voltou ao inventário");
    assert_eq!(acc.characters[1].carry.len(), 1, "o outro personagem não mudou");
    assert_eq!(
        account(&mut c, "BETO").await.characters[0]
            .carry
            .iter()
            .filter(|i| i.serial == a)
            .count(),
        1
    );

    assert_eq!(events(&pool, new_c).await, ["create", "restore"]);
    assert_eq!(events(&pool, b).await.last().unwrap(), "move");
    assert_eq!(events(&pool, d).await.last().unwrap(), "remove");
    let (place, history) = admin::item_history(&pool, cc as i64).await.unwrap();
    assert!(place.is_none(), "o C original foi destruído");
    let kinds: Vec<&str> = history.iter().map(|e| e.kind.as_str()).collect();
    assert_eq!(
        kinds,
        ["create", "destroy", "restore"],
        "o histórico do serial antigo leva ao novo"
    );

    // O jogo grava em seguida sem disputa nem duplicação.
    let acc = account(&mut c, "ANA").await;
    assert!(save(&mut c, &acc, vec![]).await.dupe_suspects.is_empty());
    let audit: i64 = sqlx::query_scalar("SELECT count(*) FROM gm_audit WHERE action = 'rollback'")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(audit, 1);
}

#[sqlx::test]
async fn registration_by_link(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone()).await;
    assert!(matches!(
        admin::start_registration(&pool, "111").await.unwrap(),
        Registration::NoInvite
    ));
    assert!(
        admin::create_invite(&pool, "abc", "gm:teste").await.is_err(),
        "id do Discord só com dígitos"
    );
    admin::create_invite(&pool, "111", "gm:teste").await.unwrap();
    let Registration::Link(token) = admin::start_registration(&pool, "111").await.unwrap() else {
        panic!("sem link");
    };
    assert_eq!(
        admin::registration_discord(&pool, &token).await.unwrap().as_deref(),
        Some("111")
    );
    assert_eq!(admin::registration_discord(&pool, "outro").await.unwrap(), None);

    for bad in ["", "com espaço", "acentuação", "nomegrandedemais"] {
        assert_eq!(
            admin::complete_registration(&pool, &token, bad.as_bytes(), b"senha")
                .await
                .unwrap(),
            Registered::InvalidName,
            "{bad}"
        );
    }
    assert_eq!(
        admin::complete_registration(&pool, &token, b"Nova", b"").await.unwrap(),
        Registered::InvalidPassword
    );
    assert_eq!(
        admin::complete_registration(&pool, &token, b"Nova", b"senha1")
            .await
            .unwrap(),
        Registered::Created
    );
    assert_eq!(
        admin::complete_registration(&pool, &token, b"Outra", b"senha1")
            .await
            .unwrap(),
        Registered::BadLink,
        "o link é de uso único"
    );
    assert_eq!(login(&mut c, "NOVA", "senha1").await.status, Status::Ok as i32);
    let discord: Option<String> = sqlx::query_scalar("SELECT discord_id FROM accounts WHERE name = 'Nova'")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(discord.as_deref(), Some("111"));
    assert!(
        matches!(
            admin::start_registration(&pool, "111").await.unwrap(),
            Registration::NoInvite
        ),
        "o convite foi usado"
    );

    // Nome já usado: nada se perde, o link continua valendo.
    admin::create_invite(&pool, "222", "gm:teste").await.unwrap();
    let Registration::Link(t2) = admin::start_registration(&pool, "222").await.unwrap() else {
        panic!("sem link");
    };
    assert_eq!(
        admin::complete_registration(&pool, &t2, b"NOVA", b"x").await.unwrap(),
        Registered::NameTaken
    );
    assert!(admin::registration_discord(&pool, &t2).await.unwrap().is_some());

    // Convite cancelado: o link para de valer.
    let code: String = sqlx::query_scalar("SELECT code FROM invites WHERE discord_id = '222'")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert!(admin::revoke_invite(&pool, &code, "gm:teste").await.unwrap());
    assert!(!admin::revoke_invite(&pool, &code, "gm:teste").await.unwrap());
    assert_eq!(admin::registration_discord(&pool, &t2).await.unwrap(), None);
    assert_eq!(
        admin::complete_registration(&pool, &t2, b"Terceira", b"x")
            .await
            .unwrap(),
        Registered::BadLink
    );

    // Link vencido.
    admin::create_invite(&pool, "333", "gm:teste").await.unwrap();
    let Registration::Link(t3) = admin::start_registration(&pool, "333").await.unwrap() else {
        panic!("sem link");
    };
    sqlx::query("UPDATE registrations SET expires_at = now() - interval '1 second'")
        .execute(&pool)
        .await
        .unwrap();
    assert_eq!(
        admin::complete_registration(&pool, &t3, b"Quarta", b"x").await.unwrap(),
        Registered::BadLink
    );
    let hashes: i64 = sqlx::query_scalar("SELECT count(*) FROM registrations WHERE length(token_hash) = 32")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(hashes, 3, "só o sha256 do token fica no banco");
}

#[sqlx::test]
async fn search_and_view(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone()).await;
    create(&mut c, "Ana", "a").await;
    create(&mut c, "Anibal", "a").await;
    create(&mut c, "Beto", "b").await;
    new_char(&mut c, "BETO", character(0, "Zeca")).await;
    sqlx::query("UPDATE accounts SET discord_id = '42' WHERE name = 'Beto'")
        .execute(&pool)
        .await
        .unwrap();

    let names = |hits: Vec<admin::AccountHit>| hits.into_iter().map(|h| h.name).collect::<Vec<_>>();
    assert_eq!(names(admin::search(&pool, "an", 50).await.unwrap()), ["Ana", "Anibal"]);
    assert_eq!(
        names(admin::search(&pool, "zec", 50).await.unwrap()),
        ["Beto"],
        "pelo personagem"
    );
    assert_eq!(
        names(admin::search(&pool, "42", 50).await.unwrap()),
        ["Beto"],
        "pelo Discord"
    );
    assert!(admin::search(&pool, "%", 50).await.unwrap().is_empty(), "o % é literal");
    assert!(admin::search(&pool, "  ", 50).await.unwrap().is_empty());

    let beto = id_of(&pool, "Beto").await;
    let v = admin::account_view(&pool, beto).await.unwrap().unwrap();
    assert_eq!(v.characters.len(), 1);
    assert_eq!(v.items.len(), 2);
    assert!(!v.versions.is_empty());
    assert_eq!(v.versions[0].characters, [(0, "Zeca".to_string(), 7)]);
    assert!(admin::account_view(&pool, 999_999).await.unwrap().is_none());
}
