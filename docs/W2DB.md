# w2db: serviço de persistência

O `w2db` (`services/w2db`, Rust) guarda contas, personagens e itens num Postgres. O DBSrv fala com ele por gRPC num Unix socket. Na B2 o serviço existe e está testado. Quem liga o DBSrv a ele é a B3 (`docs/FASE_2_5.md`); até lá o DBSrv continua nos arquivos.

## Peças

| Arquivo | O que é |
|---|---|
| `proto/w2db/v1/w2db.proto` | O contrato. As mensagens de personagem espelham `STRUCT_MOB`, `STRUCT_MOBEXTRA`, `STRUCT_AFFECT` e `STRUCT_ITEM`, e cada item leva um serial |
| `migrations/` | O esquema (sqlx). As migrações rodam no `serve` e no `migrate` |
| `src/store.rs` | Leitura e gravação da conta inteira, com as regras dos seriais |
| `src/service.rs` | O serviço gRPC |
| `src/auth.rs` | argon2id, bloqueio por senha errada e códigos de convite |
| `src/legacy.rs` | O arquivo de conta do DBSrv (`STRUCT_ACCOUNTFILE`) de e para o proto. Só o importador e o exportador usam |
| `src/transfer.rs` | `w2db import` e `w2db export` |

## Comandos

```bash
cd services
cargo build --release
export DATABASE_URL=postgres://w2@localhost/w2db
target/release/w2db migrate
target/release/w2db serve --socket /run/w2db/w2db.sock [--require-invite]
target/release/w2db import /srv/w2/DBSRV/Run      # lê account/<letra>/<CONTA>
target/release/w2db export /tmp/saida [--account NOME]
target/release/w2db invite <id do Discord>        # imprime o código
```

O socket é criado com permissão 0660: o usuário dos servidores do jogo precisa estar no grupo do dono do socket. O `SIGTERM` encerra o serviço e apaga o socket.

## Esquema

| Tabela | Conteúdo |
|---|---|
| `accounts` | Nome, hashes da senha e da senha numérica, `cargo_coin`, `donate`, `temp_key`, `version`, bloqueio por senha errada, id do Discord |
| `characters` | Uma linha por personagem. Colunas para consulta: classe, `class_master`, nível, exp, gold, guilda e posição. O resto vai em `data` (jsonb): `{mob, extra, affects, short_skill}` |
| `items` | Uma linha por item, com o `serial` como chave. Guarda a conta, o personagem, o `container` (0 sem lugar, 1 equipamento, 2 inventário, 3 baú), a vaga, o índice e os 3 pares efeito/valor |
| `item_events` | O histórico de cada serial: `create`, `import`, `move`, `transfer`, `remove`, `destroy`, `dupe_suspect` e `delete_character` |
| `account_versions` | As últimas 20 versões de cada conta (a conta em JSON, com os seriais), para desfazer uma conta sem restaurar o banco |
| `invites`, `bug_reports` | Convites de uso único e relatos do `/bug` |

Os dados pessoais de `STRUCT_ACCOUNTINFO` não entram no banco: nome real, SSN, e-mail, telefone e endereço.

Os nomes de conta e de personagem são únicos sem diferença de maiúsculas. A regra é a mesma do `W2StrUpper`: só as letras ASCII mudam de caixa. No banco os textos ficam em UTF-8, convertidos do CP1252 do jogo. Os 5 bytes que o CP1252 não define viram os controles C1 de mesmo valor, então a conversão de volta é exata.

## Gravação

A unidade de gravação é a conta inteira, como o arquivo do DBSrv: o baú e o gold do baú são de todos os personagens. Toda gravação acontece numa transação com a linha da conta travada:

1. A versão enviada tem que ser a do banco. Se não for, a resposta é `STATUS_VERSION_CONFLICT` e nada é gravado.
2. Os personagens são gravados. Um personagem só some com o `DeleteCharacter`: se faltar numa gravação comum, o pedido é recusado.
3. Cada item vai para a sua vaga pelo serial:
   - **serial 0:** item novo. Ganha serial (evento `create`), e o serial volta na resposta (`assigned`).
   - **serial desta conta:** vai para a vaga pedida (`move`, se mudou de lugar).
   - **serial de outra conta, que lá está sem lugar:** muda de dono (`transfer`). É a troca: a conta A gravou sem o item e a conta B grava com ele.
   - **serial que está numa vaga viva de outra conta, repetido na mesma gravação ou inexistente:** suspeita de duplicação. O item fica com serial novo, que vai em `dupe_suspects`, e o evento `dupe_suspect` guarda o serial original. O jogador não perde nada e o caso fica registrado.
   - **serial desta conta que não veio na gravação:** fica sem lugar (`remove`). Ele espera uma `transfer`.
   - **serial em `destroyed`:** é apagado (`destroy`). É o caso de consumo, refinação e venda ao NPC.
4. A versão sobe, e a conta gravada vai para `account_versions`.

`SaveAccount` com `logout = true` é o `SavingQuit`.

## Login

- Senhas e senhas numéricas são guardadas em argon2id, com os parâmetros padrão do crate (m = 19 MiB, t = 2, p = 1). O hash roda fora das threads do tokio.
- Depois de 5 erros seguidos (senha ou senha numérica), a conta fica bloqueada por 15 minutos. O bloqueio responde `STATUS_LOCKED` com o tempo restante.
- A conta inexistente também gasta o tempo de um hash, para a resposta não revelar quais contas existem.
- O TMSrv limita as tentativas por IP (`NetGuard`, B1). O serviço limita por conta.

## Importação e exportação

O `w2db import` lê os arquivos do DBSrv e cria cada conta numa transação:
- a senha e a senha numérica viram hash;
- cada item ganha serial (evento `import`);
- uma conta que já existe no banco não é importada de novo.

O `w2db export` grava `account/<letra>/<CONTA>` e os índices `char/<letra>/<NOME>` como o DBSrv. O arquivo exportado é igual ao original byte a byte, com três exceções:
- a senha sai zerada;
- os dados pessoais saem zerados;
- a senha numérica sai zerada (com o primeiro byte em -1 quando a conta não tem uma).

Os bytes de alinhamento saem zerados, como o DBSrv os grava. Os bytes das vagas vazias que não são zero (o DBSrv deixa a barra de skills da vaga 0 em -1) ficam em `accounts.empty_slots` e voltam iguais.

O codec do arquivo (`legacy.rs`) e o C++ conferem contra a mesma tabela de layout, `shared/protocol/account_layout.txt`:
- o `ctest` (`account_layout`, `tests/account_layout_dump.cpp`) compara o `offsetof` de cada campo com a tabela;
- o teste `layout_matches_cpp` compara os campos que o codec lê com a mesma tabela.

Se a estrutura mudar, os dois testes falham.

## Testes

Precisam de um Postgres. O `sqlx::test` cria um banco por teste, a partir do usuário de `DATABASE_URL`, que precisa poder criar bancos.

```bash
cd services
export DATABASE_URL=postgres://w2@localhost/w2db
cargo test
cargo clippy --all-targets -- -D warnings
cargo sqlx prepare --workspace --check -- --all-targets   # .sqlx em dia com o esquema
```

O que os testes cobrem:
- **`tests/roundtrip.rs`:** os arquivos de conta que o DBSrv grava no `tools/server_boot_smoke.py` (`tests/data/account`, contas sintéticas) e 12 contas com bytes aleatórios em todo campo passam por banco e exportação e voltam iguais. Os dados pessoais não aparecem no banco.
- **`tests/grpc.rs`:** o serviço por um Unix socket:
  - login e bloqueio por senha errada;
  - senha numérica e bônus da conta nova;
  - personagens e nomes únicos;
  - conflito de versão;
  - troca pelo serial, suspeita de duplicação e destruição;
  - limite de versões guardadas;
  - apagar personagem;
  - convites e `/bug`.

As consultas são conferidas na compilação contra o esquema. Sem banco, a conferência usa a cópia em `services/.sqlx` (`SQLX_OFFLINE=true`, como no CI). Depois de mudar uma consulta ou uma migração, rode `cargo sqlx prepare --workspace -- --all-targets` e inclua o `.sqlx` no commit.

Para um Postgres local descartável, sem mexer no do sistema:

```bash
P=/usr/lib/postgresql/17/bin; D=build/scratch/pg
$P/initdb -D $D/data -U w2 --auth=trust && mkdir -p $D/sock
$P/pg_ctl -D $D/data -l $D/pg.log -o "-c listen_addresses='' -k $PWD/$D/sock" start
createdb -h $PWD/$D/sock -U w2 w2db
export DATABASE_URL="postgres://w2@localhost/w2db?host=$PWD/$D/sock"
```
