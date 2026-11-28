//! O bot do Discord (docs/GM.md): `/convite` (só quem tem o cargo de GM) cria um convite para um
//! membro, e `/registrar` devolve ao membro convidado um link de uso único para criar a conta.
//!
//! As respostas são efêmeras (só quem chamou vê) e sem menções. A lógica fica em
//! [`invite_reply`] e [`register_reply`], testadas sem o Discord; os comandos só as chamam.

use poise::serenity_prelude as serenity;
use sqlx::PgPool;

use w2db::admin::{self, Registration};

pub struct Data {
    pub pool: PgPool,
    pub public_url: String,
    pub gm_role: serenity::RoleId,
}

type Error = Box<dyn std::error::Error + Send + Sync>;
type Context<'a> = poise::Context<'a, Data, Error>;

/// O que o `/convite` responde. `is_gm`: quem chamou tem o cargo de GM.
pub async fn invite_reply(pool: &PgPool, actor: u64, is_gm: bool, member: u64, member_is_bot: bool) -> String {
    if !is_gm {
        return "Só quem tem o cargo de GM cria convites.".into();
    }
    if member_is_bot {
        return "Bots não recebem convite.".into();
    }
    match admin::create_invite(pool, &member.to_string(), &format!("discord:{actor}")).await {
        Ok(_) => format!("Convite criado para <@{member}>. Agora é só a pessoa usar /registrar aqui no servidor."),
        Err(e) => {
            tracing::error!("convite: {e}");
            "Não consegui criar o convite agora. Tente de novo em instantes.".into()
        }
    }
}

/// O que o `/registrar` responde ao membro `user`.
pub async fn register_reply(pool: &PgPool, public_url: &str, user: u64) -> String {
    match admin::start_registration(pool, &user.to_string()).await {
        Ok(Registration::Link(token)) => format!(
            "Seu link para criar a conta (vale {} minutos e só funciona uma vez; não passe para ninguém):\n{}/registrar/{}",
            admin::REGISTRATION_MINUTES,
            public_url.trim_end_matches('/'),
            token
        ),
        Ok(Registration::NoInvite) => {
            "Você ainda não tem convite (ou já criou a conta com ele). Peça um a um GM.".into()
        }
        Err(e) => {
            tracing::error!("registrar: {e}");
            "Não consegui gerar o link agora. Tente de novo em instantes.".into()
        }
    }
}

async fn reply(ctx: Context<'_>, text: String) -> Result<(), Error> {
    ctx.send(
        poise::CreateReply::default()
            .content(text)
            .ephemeral(true)
            .allowed_mentions(serenity::CreateAllowedMentions::new()),
    )
    .await?;
    Ok(())
}

/// Cria um convite para um membro do servidor (só GMs).
#[poise::command(slash_command, guild_only, ephemeral)]
async fn convite(
    ctx: Context<'_>,
    #[description = "Quem vai receber o convite"] membro: serenity::User,
) -> Result<(), Error> {
    let is_gm = match ctx.author_member().await {
        Some(m) => m.roles.contains(&ctx.data().gm_role),
        None => false,
    };
    let text = invite_reply(
        &ctx.data().pool,
        ctx.author().id.get(),
        is_gm,
        membro.id.get(),
        membro.bot,
    )
    .await;
    reply(ctx, text).await
}

/// Recebe o link para criar a sua conta do jogo (precisa de convite).
#[poise::command(slash_command, guild_only, ephemeral)]
async fn registrar(ctx: Context<'_>) -> Result<(), Error> {
    let text = register_reply(&ctx.data().pool, &ctx.data().public_url, ctx.author().id.get()).await;
    reply(ctx, text).await
}

/// Conecta ao Discord e atende os comandos até o processo acabar. Os comandos são registrados só
/// no servidor `guild` (aparecem na hora, ao contrário dos globais).
pub async fn run(token: String, guild: serenity::GuildId, data: Data) -> anyhow::Result<()> {
    let framework = poise::Framework::builder()
        .options(poise::FrameworkOptions {
            commands: vec![convite(), registrar()],
            on_error: |error| {
                Box::pin(async move {
                    match error {
                        poise::FrameworkError::Command { error, ctx, .. } => {
                            tracing::error!("comando /{}: {error}", ctx.command().name);
                        }
                        other => {
                            if let Err(e) = poise::builtins::on_error(other).await {
                                tracing::error!("bot: {e}");
                            }
                        }
                    }
                })
            },
            ..Default::default()
        })
        .setup(move |ctx, ready, framework| {
            Box::pin(async move {
                tracing::info!("bot do Discord conectado como {}", ready.user.name);
                poise::builtins::register_in_guild(ctx, &framework.options().commands, guild).await?;
                Ok(data)
            })
        })
        .build();
    let mut client = serenity::ClientBuilder::new(token, serenity::GatewayIntents::GUILDS)
        .framework(framework)
        .await?;
    client.start().await?;
    Ok(())
}
