# Fase 2.5: antes de subir na VPS

O teste fechado tem três passos: dados (1), loop básico (2) e servidor no ar (3). Os dois
primeiros estão feitos. A fase 2.5 entra antes do passo 3 porque, no ar, o servidor guarda dados
de pessoas reais e recebe pacotes da internet. As decisões abaixo são de 2026-11-28; **toda a
fase precisa estar na `main`, com o CI verde, antes do passo 3**.

## Decisões

| Tema | Decisão |
|---|---|
| Acesso | IP público com as portas do jogo abertas (sem VPN). Fuzzing, hash de senha, limite de login e firewall são obrigatórios |
| Banco | Postgres. Colunas para conta e para o que se consulta do personagem, uma linha por item; o resto do personagem (score, skills, afetos, quests, alma) em `jsonb` |
| Itens | Serial (`bigint`) só no banco e no servidor, com registro de cada movimentação. O protocolo do cliente não muda |
| Acesso ao banco | Serviço separado em Rust (tokio, tonic, sqlx, argon2). O DBSrv fala com ele por gRPC num Unix socket |
| Login | Senha e senha numérica em argon2id; limite de tentativas por IP (TMSrv) e por conta (serviço) |
| Backup | pgBackRest: backup completo semanal, diferencial diário e WAL contínuo fora da VPS, com restauração testada |
| Extras | Métricas e alertas no Discord, contas por convite (bot do Discord), comando `/bug`, painel web de GM |
| Migração | O teste começa com o banco vazio. O importador arquivo→Postgres existe e é testado só com contas sintéticas |

Premissas do plano (podem ser revistas): guildas, cápsulas, ranking e safira também vão para o
Postgres; a conversão entre os `STRUCT_*` e o proto fica no C++ (o Rust não conhece o layout
legado); a senha é definida por um link HTTPS de uso único, não pelo Discord; Prometheus e
Grafana ficam na própria VPS.

Dados pessoais: os campos de nome real, SSN, e-mail, telefone e endereço de `STRUCT_ACCOUNTINFO`
não são gravados. Fica o nome da conta e o id do Discord do convite.

## Ponto de partida

- O DBSrv grava a conta inteira (`STRUCT_ACCOUNTFILE`, 8640 bytes, com os 4 personagens e o
  baú) por cima do arquivo (`CFileDB::DBWriteAccount`), sem arquivo temporário. Senha em texto
  puro, comparada com `strcmp`.
- `ProcessClientMessage` confere só o `ID` do pacote e despacha para 83 handlers; não há
  conferência central de tamanho por tipo. `CPSock::ReadMessage` só limita o tamanho total.
- Cerca de 100 pontos do TMSrv gravam direto em `Equip`, `Carry` ou `Cargo`: é o custo do serial.
- TMSrv e DBSrv são só executáveis; o fuzzing precisa do TMSrv como biblioteca.

## Branches

### B1 `feat/fuzz-and-network-edge`

1. `tmsrv_core` (biblioteca OBJECT sem o `main`) e build clang + libFuzzer.
2. Alvos: `fuzz_sock_read` (bytes crus no `CPSock::ReadMessage`), `fuzz_client_message`
   (mundo sintético em memória, jogador fictício em jogo, sequência de pacotes no
   `ProcessClientMessage`, envio de rede trocado por stub) e `fuzz_db_message` (o
   `ProcessMessage` do DBSrv; prioridade menor, a porta 7514 fica local).
3. Corpus inicial gravado de uma sessão do `LoginClient`.
4. Tamanho mínimo por tipo de pacote no despacho, a partir de `shared/protocol` (o cliente é a
   referência). Pacote curto é descartado e registrado.
5. Borda de rede no TMSrv: conexões simultâneas e aceites por IP, janela de tentativas de login
   por IP, tempo limite para conexão sem `INITCODE`.
6. CI com 60 s por alvo; rodadas longas fora do CI.

Saída: 24 h de CPU por alvo sem queda nova; cada bug achado com correção, linha em
`BUGS_LATENTES.md` e entrada de regressão no corpus.

Estado (2026-11-28): itens 1, 2 (sem o `fuzz_db_message`), 4, 5 e 6 feitos; uso em
`docs/LINUX.md` (Borda de rede do TMSrv, Fuzzing). Os 6 bugs achados e corrigidos estão em
`BUGS_LATENTES.md`. Mudanças em relação ao plano:
- As sementes saem da tabela de tamanhos (um pacote de cada tipo em cada estágio do jogador),
  não de uma gravação do `LoginClient`: cobrem todos os tipos, inclusive os que a sessão
  gravada não manda.
- O `fuzz_db_message` fica para depois. A porta 7514 só escuta localmente.
- As 24 h por alvo são rodadas manuais (`tools/fuzz_tmsrv.py --time`), antes do portão do
  passo 3. Até agora: 2M+ execuções do `fuzz_sock_read` e cerca de 1,7M do
  `fuzz_client_message` sem queda depois das correções.

### B2 `feat/persistence-service`

1. Workspace Rust em `services/`, crate `w2db`.
2. Schema (migrações sqlx): `accounts`, `characters` (com `version` para concorrência
   otimista), `items` (o serial), `item_events`, `character_versions` (últimas N versões, para
   rollback por conta sem PITR), `invites`, `bug_reports`, guildas, cápsulas, ranking, safira.
3. Contrato `.proto` versionado: `AccountLogin`, `CreateAccount`, `CreateCharacter`,
   `DeleteCharacter`, `SaveCharacter` (com eventos de item e versão esperada), `SavingQuit`,
   cápsulas, guildas, `ReportBug`, `Health`.
4. argon2id e bloqueio temporário de conta no serviço.
5. CI: Postgres como container de serviço, `sqlx prepare` (modo offline), testes de integração,
   `cargo clippy -D warnings`.

Saída: conta sintética arquivo→banco→arquivo igual byte a byte, fora senha e dados pessoais.

Estado (2026-11-28): feito, com a saída conferida (`docs/W2DB.md`). Mudanças em relação ao plano:
- A versão para concorrência otimista fica na conta, não no personagem, e `character_versions`
  virou `account_versions`. O baú e o gold do baú são da conta, e o DBSrv grava a conta inteira:
  com versão por personagem, duas gravações da mesma conta poderiam se cruzar no baú.
- O Rust conhece o layout do arquivo, mas só no importador e no exportador (`legacy.rs`; o
  importador já é o `w2db import`). Em jogo, a conversão continua no C++ (B3). Os dois lados
  conferem contra `shared/protocol/account_layout.txt`, que o próprio C++ gera.
- `SavingQuit` é o `SaveAccount` com `logout = true`.
- Guildas, cápsulas, ranking e safira ficam para a B3. Cada um tem arquivo e fluxo próprios no
  DBSrv, e a forma das RPCs depende de como a B3 ligar o `CFileDB` a eles. Os itens dentro das
  cápsulas também precisam de serial.

### B3 `feat/dbsrv-on-postgres` (depois da B2)

1. Cliente gRPC assíncrono no DBSrv, com a fila de conclusão no laço de eventos. Login e criação
   de personagem viram máquina de estados; o cache `pAccountList` continua.
2. `CFileDB` passa a usar o serviço; o importador vira `w2db import`.
3. Serial no TMSrv: vetores paralelos de serial (equip/carry por mob, baú, chão) e funções
   centrais de criar, mover e destruir item, que emitem os eventos. As mensagens internas
   TMSrv↔DBSrv levam seriais e eventos.
4. Conferência em cada gravação: seriais batem com as vagas ocupadas; divergência vai para o log e
   para alerta.
5. `server_boot_smoke.py` sobe Postgres e serviço e confere uma troca A↔B pelo serial no banco.

Saída: smoke completo com ASan/UBSan e Postgres no CI; dupe forçado detectado.

### B4 `feat/ops` (backup e métricas depois da B3)

1. pgBackRest para um bucket fora da VPS e script de restauração em outra instância com as
   conferências do banco (perda máxima de cerca de 1 minuto).
2. Métricas: TMSrv e DBSrv gravam arquivo para o coletor textfile do node_exporter (online, tick,
   pacotes/s, logins falhos, divergências de serial); o serviço expõe `/metrics`; Prometheus e
   Grafana na VPS.
3. Alertas: Alertmanager e `OnFailure=` do systemd para webhook do Discord; `systemd-coredump`.
4. `/bug`: comando de chat como o `/day`, gravado em `bug_reports` e postado no Discord, com
   limite por personagem.

### B5 `feat/community-gm` (depois da B2 e B3)

1. Bot de convite (Rust, serenity/poise): `/convite` gera código; `/registrar` devolve um link
   HTTPS de uso único para definir a senha.
2. Painel de GM (Rust, axum, páginas no servidor com htmx), só HTTPS, login de GM com TOTP: busca
   de conta e personagem, inventário com seriais, histórico de item, rollback de personagem fora
   do jogo, bloqueio de conta, convites.

## Portão do passo 3

B1 a B5 na `main` com o CI verde, e versionados para o passo 3: regras nftables (abertas só 8281,
80/443 e 22 com chave; 7514, 8895, Postgres e gRPC locais), units do systemd com usuário sem
privilégios e o roteiro de restauração.
