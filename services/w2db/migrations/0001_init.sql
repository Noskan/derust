-- Esquema inicial do w2db (docs/W2DB.md).
--
-- Unidade de gravação: a conta (personagens, baú e itens juntos, como o arquivo do DBSrv), com
-- versão para concorrência otimista. Cada item tem um serial; cada mudança de dono, de lugar,
-- criação e destruição vai para item_events.

CREATE TABLE accounts (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name            text NOT NULL,
    -- Maiúsculas ASCII do nome (W2StrUpper): unicidade e busca.
    name_key        text NOT NULL UNIQUE,
    password_hash   text NOT NULL,
    numeric_hash    text,
    discord_id      text,
    cargo_coin      integer NOT NULL DEFAULT 0,
    donate          integer NOT NULL DEFAULT 0,
    year            integer NOT NULL DEFAULT 0,
    year_day        integer NOT NULL DEFAULT 0,
    temp_key        bytea NOT NULL DEFAULT '\x',
    -- Bytes das vagas vazias (pb.EmptySlot), para a conta voltar igual ao arquivo.
    empty_slots     jsonb NOT NULL DEFAULT '[]',
    version         bigint NOT NULL DEFAULT 1,
    failed_logins   integer NOT NULL DEFAULT 0,
    locked_until    timestamptz,
    created_at      timestamptz NOT NULL DEFAULT now(),
    updated_at      timestamptz NOT NULL DEFAULT now(),
    last_login_at   timestamptz,
    last_logout_at  timestamptz
);

CREATE TABLE characters (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    account_id      bigint NOT NULL REFERENCES accounts (id) ON DELETE CASCADE,
    slot            smallint NOT NULL CHECK (slot BETWEEN 0 AND 3),
    name            text NOT NULL,
    name_key        text NOT NULL UNIQUE,
    -- Colunas para consulta (ranking, painel de GM); a fonte é o data.
    class           smallint NOT NULL,
    class_master    smallint NOT NULL,
    level           integer NOT NULL,
    exp             bigint NOT NULL,
    coin            integer NOT NULL,
    guild           integer NOT NULL,
    guild_level     smallint NOT NULL,
    pos_x           integer NOT NULL,
    pos_y           integer NOT NULL,
    -- O resto do personagem: {mob, extra, affects, short_skill} (pb.Mob, pb.MobExtra...).
    data            jsonb NOT NULL,
    created_at      timestamptz NOT NULL DEFAULT now(),
    updated_at      timestamptz NOT NULL DEFAULT now(),
    UNIQUE (account_id, slot)
);

CREATE INDEX characters_guild ON characters (guild) WHERE guild <> 0;

-- container: 0 sem lugar (saiu da conta sem destruição registrada), 1 equipamento,
-- 2 inventário, 3 baú (os valores de pb.Container).
CREATE TABLE items (
    serial          bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    account_id      bigint NOT NULL REFERENCES accounts (id) ON DELETE CASCADE,
    character_id    bigint REFERENCES characters (id) ON DELETE SET NULL,
    container       smallint NOT NULL CHECK (container BETWEEN 0 AND 3),
    slot            smallint,
    item_index      smallint NOT NULL,
    ef1             smallint NOT NULL,
    ev1             smallint NOT NULL,
    ef2             smallint NOT NULL,
    ev2             smallint NOT NULL,
    ef3             smallint NOT NULL,
    ev3             smallint NOT NULL,
    created_at      timestamptz NOT NULL DEFAULT now(),
    updated_at      timestamptz NOT NULL DEFAULT now(),
    CHECK ((container = 0) = (slot IS NULL)),
    CHECK ((container IN (1, 2)) = (character_id IS NOT NULL))
);

CREATE UNIQUE INDEX items_char_slot ON items (character_id, container, slot) WHERE container IN (1, 2);
CREATE UNIQUE INDEX items_cargo_slot ON items (account_id, slot) WHERE container = 3;
CREATE INDEX items_account ON items (account_id);

-- kind: create, move, transfer, remove, destroy, dupe_suspect, import, delete_character.
CREATE TABLE item_events (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    at              timestamptz NOT NULL DEFAULT now(),
    serial          bigint NOT NULL,
    kind            text NOT NULL,
    account_id      bigint,
    from_account_id bigint,
    item_index      smallint,
    detail          jsonb NOT NULL DEFAULT '{}'
);

CREATE INDEX item_events_serial ON item_events (serial);
CREATE INDEX item_events_account ON item_events (account_id, at);

-- As últimas versões de cada conta (pb.Account em JSON, com os seriais), para desfazer uma conta
-- sem restaurar o banco inteiro.
CREATE TABLE account_versions (
    account_id      bigint NOT NULL REFERENCES accounts (id) ON DELETE CASCADE,
    version         bigint NOT NULL,
    saved_at        timestamptz NOT NULL DEFAULT now(),
    snapshot        jsonb NOT NULL,
    PRIMARY KEY (account_id, version)
);

CREATE TABLE invites (
    code            text PRIMARY KEY,
    discord_id      text NOT NULL,
    created_at      timestamptz NOT NULL DEFAULT now(),
    used_at         timestamptz,
    account_id      bigint REFERENCES accounts (id)
);

CREATE TABLE bug_reports (
    id              bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    account_id      bigint REFERENCES accounts (id),
    character_name  text NOT NULL,
    server          integer NOT NULL,
    pos_x           integer NOT NULL,
    pos_y           integer NOT NULL,
    text            text NOT NULL,
    status          text NOT NULL DEFAULT 'open',
    created_at      timestamptz NOT NULL DEFAULT now()
);
