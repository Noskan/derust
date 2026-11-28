# w2db: serviço de persistência

O `w2db` (`services/w2db`, Rust) guarda contas, personagens e itens num Postgres. O DBSrv fala com ele por gRPC num Unix socket ([No DBSrv](#no-dbsrv)). Sem um w2db configurado, o DBSrv continua nos arquivos (`./account`, `./char`).

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
| `item_events` | O histórico de cada serial: `create`, `import`, `move`, `transfer`, `contest`, `remove`, `destroy`, `dupe_suspect` e `delete_character` |
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
   - **serial numa vaga viva de outra conta:** também muda de dono (`contest`), e a conta que perdeu o item fica marcada (`items.contested_from`). É a troca em que o outro lado ainda não gravou: no fim de uma troca o TMSrv grava os dois jogadores, um depois do outro, e numa troca de mão dupla o primeiro sempre grava com o item que ainda está no outro. A marca sai quando a conta marcada grava sem o serial.
   - **serial de um item que esta conta perdeu numa disputa, de um item já disputado, repetido na mesma gravação ou inexistente:** suspeita de duplicação. O item fica com serial novo, que vai em `dupe_suspects`, e o evento `dupe_suspect` guarda o serial original. O jogador não perde nada e o caso fica registrado. Numa duplicação de verdade, as duas contas têm o item: a primeira a gravar leva o serial, e a segunda dispara a suspeita.
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

## No DBSrv

O DBSrv usa o w2db quando há um socket configurado: a variável `W2DB_SOCKET` ou a primeira linha de `w2db.txt` no diretório do DBSrv. Com o w2db configurado e fora do ar (`Health`), o DBSrv não sobe, para não cair nos arquivos sem querer. Sem o gRPC de C++ no build (`DERUST_W2DB=OFF`, ou `libgrpc++-dev` ausente), o DBSrv é montado só com os arquivos e também não sobe se houver um w2db configurado.

O cliente fica em `server/dbsrv/persistence/W2DbClient.cpp`. A conversão `STRUCT_ACCOUNTFILE` ↔ `w2db.v1.Account` fica em `W2DbConvert.cpp` e produz exatamente o proto do codec do importador: o `ctest` `w2db_convert` confere as contas de teste contra os `.pb` que o Rust gera delas (`tests/data/golden`, teste `golden_protos`).

As primitivas do `CFileDB` mudam de lugar, e os pacotes do TMSrv continuam os mesmos:
- `DBReadAccount` e `DBWriteAccount` viram `LoadAccount` e `SaveAccount`. A conta vem sem senha.
- O login confere a senha no w2db (`AccountLogin`), com o bloqueio dele. A senha digitada fica em `AccountPass`, na memória, porque apagar personagem compara com ela.
- A senha numérica é conferida no w2db (`CheckNumericPassword`). Quando o TMSrv grava uma senha numérica nova, o DBSrv a manda para `SetNumericPassword`.
- `AddAccount` (o `ImportUser`) chama `CreateAccount`, e `UpdateAccount` chama `SetPassword`. Os dados pessoais são descartados.
- Criar personagem: o nome é conferido com `FindCharacter`, e o personagem nasce na gravação da conta. Se outro levar o nome antes, a gravação volta `STATUS_NAME_TAKEN`, a criação falha e a conta em memória volta a ser a do banco.
- Apagar personagem: `DeleteCharacter`, que leva os itens dele junto.
- `SavingQuit` grava com `logout = true`. A cópia para `S:/export` do original não existe com o w2db.

As chamadas são síncronas, com prazo (5 s; 15 s nas que calculam argon2id), na thread do laço de eventos. O w2db roda na mesma máquina, a gravação leva poucos milissegundos e o login algumas dezenas. Para o teste fechado (até 30 jogadores) isso não aparece, e a memória do DBSrv fica igual ao banco depois de cada chamada.

Se uma gravação falhar (w2db fora do ar, prazo estourado ou conflito de versão duas vezes seguidas), o DBSrv grava o arquivo da conta em `./w2db_pending/<CONTA>`, registra no log e avisa o operador. Num conflito de versão, o DBSrv relê a conta do banco e grava de novo, uma vez.

### Seriais

Os seriais ficam no DBSrv. Para cada conta conectada, ele guarda a versão, os seriais e os itens da última gravação. A cada gravação os seriais são deduzidos, nesta ordem:
1. mesma vaga e mesmo item;
2. item recebido numa troca, com o serial que tinha na conta de origem (abaixo);
3. mesma vaga e mesmo índice: o item mudou na vaga (refinação, quantidade);
4. mesmo item em outra vaga: o item mudou de lugar.

O resto vai com serial 0, que para o w2db é item novo.

Entre contas, o TMSrv avisa as trocas. No fim de uma troca, cada item que mudou de mão entra na próxima gravação de quem recebeu (`STRUCT_ITEMHANDOFF` em `MSG_DBSaveMob` e `MSG_SavingQuit`), com a conta e a vaga do Carry de onde saiu. O DBSrv acha o serial:
- na vaga de quem deu, se ele ainda não gravou depois da troca;
- senão, entre os itens que saíram das últimas gravações de quem deu.

O item recebido grava com esse serial, e o w2db faz a `contest` ou a `transfer`. Se quem deu ainda tiver o item, que é o que acontece num bug de duplicação, a gravação dele leva o mesmo serial, e o w2db acusa a suspeita de duplicação. O DBSrv registra no log e avisa o operador (`[w2db] err,w2db N dupe suspect(s) ...` no stderr).

Ficam com serial novo, e o item antigo fica sem lugar (`remove`):
- item que muda de conta sem troca: chão, loja pessoal (`_MSG_ReqBuy`), correio;
- item recebido que o TMSrv junta com outro na troca.

O histórico do item se perde nesses casos, mas nada se duplica. O DBSrv registra no log o item de troca que não achou.

### O que ainda não passa pelo w2db

- **Cápsulas:** guardar e tirar personagem da cápsula é recusado com o w2db.
- **Ranking e DayLog:** varrem `./account` e ficam vazios.
- **Guildas e safira:** seguem nos arquivos de sempre, que não são de conta.

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
  - troca pelo serial (inclusive a de mão dupla, sem falso alarme), suspeita de duplicação e destruição;
  - limite de versões guardadas;
  - apagar personagem;
  - convites e `/bug`.

O teste de jogo (`tools/server_boot_smoke.py --w2db services/target/debug/w2db --forced-dupe`) faz o caminho inteiro com o DBSrv no w2db:
- sobe um Postgres próprio e o `w2db serve`;
- cria as contas pelo `ImportUser`;
- joga com o `LoginClient`;
- confere no banco as contas, os personagens e os itens com serial, e a troca de mão dupla pelo serial: a espada criada em cada conta está na outra. Uma espada vai por `contest` e a outra por `remove` e `transfer`.

Com `--forced-dupe`, uma segunda troca é feita com o gancho de teste `/w2dupe`. Esse gancho só existe com `DERUST_TEST_HOOKS=ON` e deixa uma cópia do item com quem deu. O teste exige o `dupe_suspect` no banco e o aviso do DBSrv. O CI roda esse teste com ASan/UBSan.

As consultas são conferidas na compilação contra o esquema. Sem banco, a conferência usa a cópia em `services/.sqlx` (`SQLX_OFFLINE=true`, como no CI). Depois de mudar uma consulta ou uma migração, rode `cargo sqlx prepare --workspace -- --all-targets` e inclua o `.sqlx` no commit.

Para um Postgres local descartável, sem mexer no do sistema:

```bash
P=/usr/lib/postgresql/17/bin; D=build/scratch/pg
$P/initdb -D $D/data -U w2 --auth=trust && mkdir -p $D/sock
$P/pg_ctl -D $D/data -l $D/pg.log -o "-c listen_addresses='' -k $PWD/$D/sock" start
createdb -h $PWD/$D/sock -U w2 w2db
export DATABASE_URL="postgres://w2@localhost/w2db?host=$PWD/$D/sock"
```
