-- B5: sessões do DBSrv, bloqueio pelo GM, cadastro pelo link do bot, contas de GM e o registro do
-- que eles fazem (docs/GM.md).

-- Conta carregada no DBSrv (AddAccountList), até sair da lista (RemoveAccountList). O DBSrv zera
-- todas quando sobe. O rollback pelo painel só vale para conta fora do jogo.
ALTER TABLE accounts ADD COLUMN online_since timestamptz;
-- Bloqueio pelo GM: o login devolve STATUS_BANNED até esta hora ('infinity' para sempre). Não é a
-- gravação da conta que muda isto, então o DBSrv não desfaz o bloqueio de quem está no jogo.
ALTER TABLE accounts ADD COLUMN banned_until timestamptz;
ALTER TABLE accounts ADD COLUMN ban_reason text;

-- Quem criou o convite ('gm:<nome>', 'discord:<id>' ou 'cli') e quando ele deixa de valer.
ALTER TABLE invites ADD COLUMN created_by text NOT NULL DEFAULT 'cli';
ALTER TABLE invites ADD COLUMN revoked_at timestamptz;
CREATE INDEX invites_open ON invites (discord_id) WHERE used_at IS NULL AND revoked_at IS NULL;

-- Links de uso único do /registrar. Só o sha256 do token fica no banco.
CREATE TABLE registrations (
    token_hash      bytea PRIMARY KEY,
    invite_code     text NOT NULL REFERENCES invites (code),
    created_at      timestamptz NOT NULL DEFAULT now(),
    expires_at      timestamptz NOT NULL,
    used_at         timestamptz
);

CREATE TABLE gm_users (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name            text NOT NULL UNIQUE,
    password_hash   text NOT NULL,
    totp_secret     bytea NOT NULL,
    -- Último passo de 30 s aceito: o mesmo código não entra duas vezes.
    totp_last_step  bigint NOT NULL DEFAULT 0,
    failed_logins   integer NOT NULL DEFAULT 0,
    locked_until    timestamptz,
    disabled        boolean NOT NULL DEFAULT false,
    created_at      timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE gm_sessions (
    token_hash      bytea PRIMARY KEY,
    gm_id           bigint NOT NULL REFERENCES gm_users (id) ON DELETE CASCADE,
    csrf            text NOT NULL,
    created_at      timestamptz NOT NULL DEFAULT now(),
    expires_at      timestamptz NOT NULL
);

CREATE TABLE gm_audit (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    at              timestamptz NOT NULL DEFAULT now(),
    actor           text NOT NULL,
    action          text NOT NULL,
    target          text,
    detail          jsonb NOT NULL DEFAULT '{}'
);

CREATE INDEX gm_audit_at ON gm_audit (at);
