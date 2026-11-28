//! O serviço de ponta a ponta: servidor gRPC num Unix socket, cliente tonic, Postgres de teste.

use std::path::PathBuf;

use hyper_util::rt::TokioIo;
use sqlx::PgPool;
use tokio::net::{UnixListener, UnixStream};
use tokio_stream::wrappers::UnixListenerStream;
use tonic::transport::{Channel, Endpoint};
use w2db::pb::{self, Status, w2_db_client::W2DbClient, w2_db_server::W2DbServer};
use w2db::service::Service;

type Client = W2DbClient<Channel>;

async fn start(pool: PgPool, require_invite: bool) -> (Client, PathBuf) {
    let dir = std::env::temp_dir().join(format!("w2db-grpc-{}-{}", std::process::id(), rand_suffix()));
    std::fs::create_dir_all(&dir).unwrap();
    let sock = dir.join("w2db.sock");
    let listener = UnixListener::bind(&sock).unwrap();
    tokio::spawn(
        tonic::transport::Server::builder()
            .add_service(W2DbServer::new(Service::new(pool, require_invite)))
            .serve_with_incoming(UnixListenerStream::new(listener)),
    );
    let path = sock.clone();
    let channel = Endpoint::try_from("http://w2db.local")
        .unwrap()
        .connect_with_connector(tower::service_fn(move |_| {
            let path = path.clone();
            async move { Ok::<_, std::io::Error>(TokioIo::new(UnixStream::connect(path).await?)) }
        }))
        .await
        .unwrap();
    (W2DbClient::new(channel), dir)
}

fn rand_suffix() -> u64 {
    getrandom::u64().unwrap()
}

fn item(index: i32, ef: u32, ev: u32) -> pb::Item {
    pb::Item {
        index,
        effects: vec![
            pb::ItemEffect { effect: ef, value: ev },
            pb::ItemEffect::default(),
            pb::ItemEffect::default(),
        ],
    }
}

fn stored(serial: u64, slot: u32, it: pb::Item) -> pb::StoredItem {
    pb::StoredItem {
        serial,
        slot,
        item: Some(it),
    }
}

fn character(slot: u32, name: &str) -> pb::Character {
    pb::Character {
        slot,
        name: name.as_bytes().to_vec(),
        mob: Some(pb::Mob {
            class: 1,
            coin: 500,
            exp: 1234,
            home_town_x: 2100,
            home_town_y: 2100,
            base_score: Some(pb::Score {
                level: 7,
                special: vec![1, 2, 3, 4],
                ..Default::default()
            }),
            ..Default::default()
        }),
        equip: vec![stored(0, 0, item(1100, 0, 0))],
        carry: vec![stored(0, 3, item(400, 61, 5))],
        affects: vec![pb::Affect {
            r#type: 8,
            level: 1,
            value: 3,
            time: 99,
        }],
        short_skill: vec![0xFF; 16],
        extra: Some(pb::MobExtra {
            class_master: 2,
            fame: 10,
            ..Default::default()
        }),
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
    c.save_account(pb::SaveAccountRequest {
        account: Some(acc.clone()),
        destroyed,
        reason: "teste".into(),
        logout: false,
    })
    .await
    .unwrap()
    .into_inner()
}

async fn new_char(c: &mut Client, acc: &str, ch: pb::Character) -> pb::SaveAccountReply {
    let version = account(c, acc).await.version;
    c.create_character(pb::CreateCharacterRequest {
        account_name: acc.as_bytes().to_vec(),
        expected_version: version,
        character: Some(ch),
    })
    .await
    .unwrap()
    .into_inner()
}

async fn events(pool: &PgPool, serial: u64) -> Vec<String> {
    sqlx::query_scalar("SELECT kind FROM item_events WHERE serial = $1 ORDER BY id")
        .bind(serial as i64)
        .fetch_all(pool)
        .await
        .unwrap()
}

#[sqlx::test]
async fn accounts_and_login(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone(), false).await;
    assert!(c.health(pb::HealthRequest {}).await.unwrap().into_inner().database_ok);

    create(&mut c, "Alice", "segredo").await;
    let dup = c
        .create_account(pb::CreateAccountRequest {
            name: b"ALICE".to_vec(),
            password: b"x".to_vec(),
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(
        dup.status,
        Status::AccountExists as i32,
        "nome sem diferença de maiúsculas"
    );

    let ok = login(&mut c, "alice", "segredo").await;
    assert_eq!(ok.status, Status::Ok as i32);
    let acc = ok.account.unwrap();
    assert_eq!(acc.version, 2, "a conta nova já foi gravada uma vez");
    assert_eq!(
        acc.empty_slots[0].short_skill,
        vec![0xFF; 8],
        "como o AddAccount do DBSrv"
    );
    assert!(!acc.numeric_set);

    assert_eq!(login(&mut c, "nobody", "x").await.status, Status::NoAccount as i32);
    for _ in 0..w2db::auth::MAX_FAILURES {
        assert_eq!(
            login(&mut c, "alice", "errada").await.status,
            Status::WrongPassword as i32
        );
    }
    let locked = login(&mut c, "alice", "segredo").await;
    assert_eq!(
        locked.status,
        Status::Locked as i32,
        "bloqueada depois de 5 erros, mesmo com a senha certa"
    );
    assert!(locked.lock_seconds > 800);

    // Senha numérica.
    let st = c
        .check_numeric_password(pb::CheckNumericPasswordRequest {
            name: b"BOB".to_vec(),
            numeric: b"1".to_vec(),
        })
        .await
        .unwrap()
        .into_inner()
        .status;
    assert_eq!(st, Status::NoAccount as i32);
    create(&mut c, "Bob", "b0b").await;
    let check = |n: &'static [u8]| pb::CheckNumericPasswordRequest {
        name: b"BOB".to_vec(),
        numeric: n.to_vec(),
    };
    assert_eq!(
        c.check_numeric_password(check(b"1234"))
            .await
            .unwrap()
            .into_inner()
            .status,
        Status::NumericNotSet as i32
    );
    let set = pb::SetNumericPasswordRequest {
        name: b"BOB".to_vec(),
        numeric: b"1234".to_vec(),
    };
    assert_eq!(
        c.set_numeric_password(set).await.unwrap().into_inner().status,
        Status::Ok as i32
    );
    assert_eq!(
        c.check_numeric_password(check(b"1234"))
            .await
            .unwrap()
            .into_inner()
            .status,
        Status::Ok as i32
    );
    assert_eq!(
        c.check_numeric_password(check(b"4321"))
            .await
            .unwrap()
            .into_inner()
            .status,
        Status::WrongPassword as i32
    );
    assert!(account(&mut c, "BOB").await.numeric_set);

    let bonus = c
        .create_account(pb::CreateAccountRequest {
            name: b"Rico".to_vec(),
            password: b"x".to_vec(),
            bonus: true,
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(bonus.status, Status::Ok as i32);
    let rico = account(&mut c, "RICO").await;
    assert_eq!(rico.cargo.len(), 100);
    assert!(rico.cargo.iter().all(|i| i.serial != 0));
}

#[sqlx::test]
async fn characters_items_and_versions(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone(), false).await;
    create(&mut c, "Ana", "a").await;
    create(&mut c, "Beto", "b").await;

    let r = new_char(&mut c, "ANA", character(0, "Guerreira")).await;
    assert_eq!(r.status, Status::Ok as i32);
    assert_eq!(r.assigned.len(), 2, "equip e carry ganharam serial");
    let taken = new_char(&mut c, "BETO", character(1, "GUERREIRA")).await;
    assert_eq!(
        taken.status,
        Status::NameTaken as i32,
        "nome de personagem único sem diferença de maiúsculas"
    );
    assert_eq!(
        new_char(&mut c, "BETO", character(1, "Mago")).await.status,
        Status::Ok as i32
    );
    assert_eq!(
        new_char(&mut c, "ANA", character(0, "Outra")).await.status,
        Status::SlotTaken as i32
    );

    let found = c
        .find_character(pb::FindCharacterRequest {
            name: b"guerreira".to_vec(),
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!((found.account_name.as_slice(), found.slot), (b"Ana".as_slice(), 0));

    // Os dados voltam como foram.
    let mut ana = account(&mut c, "ANA").await;
    let g = &ana.characters[0];
    let mut want = character(0, "Guerreira");
    want.equip[0].serial = g.equip[0].serial;
    want.carry[0].serial = g.carry[0].serial;
    assert_eq!(*g, want);
    let level: i32 = sqlx::query_scalar("SELECT level FROM characters WHERE name_key = 'GUERREIRA'")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(level, 7);

    // Mudar de vaga mantém o serial e registra o movimento.
    let potion = ana.characters[0].carry[0].serial;
    ana.characters[0].carry[0].slot = 10;
    let r = save(&mut c, &ana, vec![]).await;
    assert_eq!(r.status, Status::Ok as i32);
    assert!(r.assigned.is_empty());
    assert_eq!(events(&pool, potion).await, ["create", "move"]);

    // Versão velha é recusada.
    let stale = save(&mut c, &ana, vec![]).await;
    assert_eq!(stale.status, Status::VersionConflict as i32);
    assert_eq!(stale.version, r.version);

    // Troca: o item sai da Ana (sem lugar) e aparece no Beto pelo mesmo serial.
    let mut ana = account(&mut c, "ANA").await;
    let mut beto = account(&mut c, "BETO").await;
    let sword = ana.characters[0].equip[0].clone();
    ana.characters[0].equip.clear();
    assert_eq!(save(&mut c, &ana, vec![]).await.status, Status::Ok as i32);
    beto.characters[0].carry.push(pb::StoredItem {
        slot: 20,
        ..sword.clone()
    });
    let r = save(&mut c, &beto, vec![]).await;
    assert_eq!(r.status, Status::Ok as i32);
    assert!(r.dupe_suspects.is_empty());
    assert_eq!(events(&pool, sword.serial).await, ["create", "remove", "transfer"]);
    let beto = account(&mut c, "BETO").await;
    assert!(beto.characters[0].carry.iter().any(|i| i.serial == sword.serial));

    // Duplicação: a Ana grava de novo com a espada que agora é do Beto. A primeira gravação leva a
    // espada para a Ana (pode ser o lado que grava primeiro numa troca); quando o Beto grava de
    // novo com ela, é a suspeita.
    let mut ana = account(&mut c, "ANA").await;
    ana.characters[0].equip.push(sword.clone());
    let r = save(&mut c, &ana, vec![]).await;
    assert_eq!(r.status, Status::Ok as i32);
    assert!(r.dupe_suspects.is_empty());
    let beto = account(&mut c, "BETO").await;
    assert!(
        beto.characters[0].carry.iter().all(|i| i.serial != sword.serial),
        "a espada passou para a Ana no banco"
    );
    let mut beto_stale = beto.clone();
    beto_stale.characters[0].carry.push(pb::StoredItem {
        slot: 20,
        ..sword.clone()
    });
    let r = save(&mut c, &beto_stale, vec![]).await;
    assert_eq!(r.status, Status::Ok as i32);
    assert_eq!(r.dupe_suspects.len(), 1);
    let copy = r.dupe_suspects[0];
    assert_ne!(copy, sword.serial);
    assert_eq!(events(&pool, copy).await, ["dupe_suspect"]);
    assert_eq!(
        events(&pool, sword.serial).await,
        ["create", "remove", "transfer", "contest"]
    );
    let original: i64 =
        sqlx::query_scalar("SELECT (detail->>'original_serial')::bigint FROM item_events WHERE serial = $1")
            .bind(copy as i64)
            .fetch_one(&pool)
            .await
            .unwrap();
    assert_eq!(original as u64, sword.serial);

    // Destruição de propósito.
    let mut ana = account(&mut c, "ANA").await;
    let potion = ana.characters[0].carry[0].serial;
    ana.characters[0].carry.clear();
    assert_eq!(save(&mut c, &ana, vec![potion]).await.status, Status::Ok as i32);
    assert_eq!(events(&pool, potion).await.last().unwrap(), "destroy");
    let gone: i64 = sqlx::query_scalar("SELECT count(*) FROM items WHERE serial = $1")
        .bind(potion as i64)
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(gone, 0);

    // Só as últimas 20 versões ficam.
    for _ in 0..25 {
        let ana = account(&mut c, "ANA").await;
        assert_eq!(save(&mut c, &ana, vec![]).await.status, Status::Ok as i32);
    }
    let kept: i64 = sqlx::query_scalar(
        "SELECT count(*) FROM account_versions v JOIN accounts a ON a.id = v.account_id WHERE a.name_key = 'ANA'",
    )
    .fetch_one(&pool)
    .await
    .unwrap();
    assert_eq!(kept, w2db::store::KEEP_VERSIONS);

    // Apagar personagem apaga os itens dele, com evento.
    let ana = account(&mut c, "ANA").await;
    let serials: Vec<u64> = ana.characters[0].equip.iter().map(|i| i.serial).collect();
    let r = c
        .delete_character(pb::DeleteCharacterRequest {
            account_name: b"ANA".to_vec(),
            expected_version: ana.version,
            slot: 0,
            residue: None,
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(r.status, Status::Ok as i32);
    assert!(account(&mut c, "ANA").await.characters.is_empty());
    for s in serials {
        assert_eq!(events(&pool, s).await.last().unwrap(), "delete_character");
    }

    // Personagem não pode sumir numa gravação comum.
    let mut beto = account(&mut c, "BETO").await;
    beto.characters.clear();
    let err = c
        .save_account(pb::SaveAccountRequest {
            account: Some(beto),
            ..Default::default()
        })
        .await
        .unwrap_err();
    assert_eq!(err.code(), tonic::Code::InvalidArgument);
}

#[sqlx::test]
async fn invites_and_bugs(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone(), true).await;
    let no_invite = c
        .create_account(pb::CreateAccountRequest {
            name: b"Zed".to_vec(),
            password: b"z".to_vec(),
            ..Default::default()
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(no_invite.status, Status::InviteRequired as i32);
    let code = c
        .create_invite(pb::CreateInviteRequest {
            discord_id: "123".into(),
        })
        .await
        .unwrap()
        .into_inner()
        .code;
    let req = |code: &str| pb::CreateAccountRequest {
        name: b"Zed".to_vec(),
        password: b"z".to_vec(),
        invite_code: code.into(),
        ..Default::default()
    };
    assert_eq!(
        c.create_account(req("ERRADO")).await.unwrap().into_inner().status,
        Status::BadInvite as i32
    );
    assert_eq!(
        c.create_account(req(&code)).await.unwrap().into_inner().status,
        Status::Ok as i32
    );
    let again = pb::CreateAccountRequest {
        name: b"Zed2".to_vec(),
        ..req(&code)
    };
    assert_eq!(
        c.create_account(again).await.unwrap().into_inner().status,
        Status::BadInvite as i32,
        "convite de uso único"
    );
    let discord: Option<String> = sqlx::query_scalar("SELECT discord_id FROM accounts WHERE name_key = 'ZED'")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(discord.as_deref(), Some("123"));

    let bug = c
        .report_bug(pb::ReportBugRequest {
            account_name: b"ZED".to_vec(),
            character_name: b"Ninguem".to_vec(),
            server: 1,
            pos_x: 2100,
            pos_y: 2100,
            text: "porta não abre".into(),
        })
        .await
        .unwrap()
        .into_inner();
    assert_eq!(bug.status, Status::Ok as i32);
    assert!(bug.id > 0);
}

#[sqlx::test]
async fn two_way_trade_is_not_a_dupe(pool: PgPool) {
    let (mut c, _dir) = start(pool.clone(), false).await;
    create(&mut c, "Caio", "c").await;
    create(&mut c, "Duda", "d").await;
    assert_eq!(
        new_char(&mut c, "CAIO", character(0, "Caio1")).await.status,
        Status::Ok as i32
    );
    assert_eq!(
        new_char(&mut c, "DUDA", character(0, "Duda1")).await.status,
        Status::Ok as i32
    );
    let mut caio = account(&mut c, "CAIO").await;
    let mut duda = account(&mut c, "DUDA").await;
    let s = caio.characters[0].carry[0].clone();
    let t = duda.characters[0].carry[0].clone();

    // A troca no TMSrv: cada um fica com o item do outro; o Caio grava primeiro.
    caio.characters[0].carry = vec![pb::StoredItem { slot: 3, ..t.clone() }];
    duda.characters[0].carry = vec![pb::StoredItem { slot: 3, ..s.clone() }];
    let r = save(&mut c, &caio, vec![]).await;
    assert!(r.dupe_suspects.is_empty() && r.assigned.is_empty());
    let r = save(&mut c, &duda, vec![]).await;
    assert!(r.dupe_suspects.is_empty() && r.assigned.is_empty(), "{r:?}");
    assert_eq!(events(&pool, t.serial).await, ["create", "contest"]);
    assert_eq!(events(&pool, s.serial).await, ["create", "remove", "transfer"]);
    let marks: i64 = sqlx::query_scalar("SELECT count(*) FROM items WHERE contested_from IS NOT NULL")
        .fetch_one(&pool)
        .await
        .unwrap();
    assert_eq!(marks, 0, "a Duda gravou sem o item que perdeu: a disputa se resolveu");

    // Gravações seguintes não geram nada.
    let caio = account(&mut c, "CAIO").await;
    assert!(save(&mut c, &caio, vec![]).await.dupe_suspects.is_empty());
    let duda = account(&mut c, "DUDA").await;
    assert!(save(&mut c, &duda, vec![]).await.dupe_suspects.is_empty());
}
