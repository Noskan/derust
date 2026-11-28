# Operação: backup, métricas, alertas e `/bug`

O que mantém o servidor do teste fechado operável (fase 2.5, B4 em `docs/FASE_2_5.md`). Tudo roda
na própria VPS, menos o repositório do backup, que fica num bucket fora dela. Os arquivos de
configuração estão em `deploy/`. As unidades do systemd saem de `tools/linux_deploy.py`, com os
caminhos da árvore.

| Peça | O que faz | Onde |
|---|---|---|
| TMSrv, DBSrv | gravam métricas em arquivo a cada 15 s | `$W2_METRICS_DIR/w2_tmsrv.prom`, `w2_dbsrv.prom` |
| w2db | `/metrics` e os relatos do `/bug` no Discord | `127.0.0.1:9730`, `W2DB_DISCORD_WEBHOOK_FILE` |
| node_exporter | a máquina, as units (`--collector.systemd`) e os `.prom` | `127.0.0.1:9100` |
| Prometheus | coleta e avalia os alertas | `deploy/prometheus/` |
| Alertmanager | manda os alertas para o Discord | `deploy/alertmanager/alertmanager.yml` |
| Grafana | o painel `derust` | `deploy/grafana/` |
| `w2-notify@.service` | aviso no Discord quando uma unit falha | `deploy/bin/w2-notify-discord` |
| pgBackRest | backup completo semanal, diferencial diário, WAL contínuo | `deploy/pgbackrest/`, `deploy/postgres/` |
| `tools/w2db_restore.py` | restauração numa instância separada, conferida | |

## Métricas

**TMSrv e DBSrv:**
- **Como gravam:** cada um tem uma tabela fixa de métricas (`core/ServerMetrics.cpp`) e a grava no
  formato de texto do Prometheus (`common/Metrics.h`). Grava num temporário e renomeia, para o
  coletor nunca ler um arquivo pela metade.
- **Sem `W2_METRICS_DIR`:** nada é gravado.
- **Além da tabela:** cada arquivo leva `<prefixo>_start_time_seconds` e
  `<prefixo>_last_write_timestamp_seconds`. O alerta de servidor parado ou travado olha esta
  última.

| Métrica | |
|---|---|
| `w2_tmsrv_users_online`, `w2_tmsrv_connections` | jogadores no mundo, conexões abertas |
| `w2_tmsrv_tick_seconds_max`, `_total`, `w2_tmsrv_ticks_total` | duração do `ProcessSecTimer`: o pico desde a última gravação e a soma (média = razão das taxas) |
| `w2_tmsrv_packets_total`, `w2_tmsrv_packets_rejected_total{reason}` | pacotes de cliente lidos e recusados (`read`: leitura; `short`: menor que a estrutura) |
| `w2_tmsrv_connections_refused_total`, `w2_tmsrv_logins_rate_limited_total` | recusas da borda de rede por IP (`net/NetGuard.cpp`) |
| `w2_tmsrv_bug_reports_total`, `w2_tmsrv_dbsrv_connected` | `/bug` enviados; conexão com o DBSrv |
| `w2_dbsrv_accounts_online`, `w2_dbsrv_tmsrv_connected` | contas conectadas; TMSrv conectados |
| `w2_dbsrv_login_failures_total{reason}` | `password`, `no_account`, `locked` (w2db), `blocked` (data), `numeric` |
| `w2_dbsrv_saves_total` | gravações de conta |
| `w2_dbsrv_w2db_failures_total`, `w2_dbsrv_w2db_pending_total` | chamadas ao w2db que falharam; contas que foram para `./w2db_pending` |
| `w2_dbsrv_dupe_suspects_total` | divergências de serial (suspeitas de duplicação) |

**w2db (`/metrics`, `--metrics-addr` ou `W2DB_METRICS_ADDR`):**
- `w2db_rpc_requests_total`, `w2db_rpc_errors_total` e `w2db_rpc_duration_seconds` por método.
- `w2db_login_failures_total{reason}`, `w2db_dupe_suspects_total` e
  `w2db_version_conflicts_total`.
- O `/bug`: `w2db_bug_reports_total`, `w2db_bug_reports_limited_total`, `w2db_discord_posts_total`,
  `w2db_discord_post_failures_total` e `w2db_discord_queue`.
- Lido do banco na hora da coleta: `w2db_database_up` e o arquivamento do WAL (`pg_stat_archiver`):
  `w2db_wal_archived_total`, `w2db_wal_archive_failures_total` e os horários do último arquivado e
  da última falha.

**Backup:** o `ExecStartPost=` da `w2-backup@<tipo>.service` grava
`w2_backup_last_success_timestamp_seconds{type}`. Esse passo só roda depois de um backup que
terminou bem.

## Alertas

As regras ficam em `deploy/prometheus/rules/derust.yml`, e os testes delas em `derust_test.yml`.

| Grupo | Alertas |
|---|---|
| Servidores | TMSrv ou DBSrv sem gravar métricas há 90 s, arquivo `.prom` ausente, unit `w2-*` em `failed`, tick acima de 250 ms, TMSrv sem DBSrv, muitas conexões recusadas, muitos logins recusados |
| Dados | w2db fora, erros do w2db, falha do DBSrv ao falar com o w2db, gravação pendente, **suspeita de duplicação**, relatos parados na fila do Discord |
| Backup | arquivamento do WAL falhando, nenhum backup em 26 h, nenhum completo em 8 dias, nenhum registro de backup completo |
| Máquina | disco com menos de 10% livre, memória com menos de 10% |

O Alertmanager manda todos para um canal do Discord: os críticos repetem a cada hora, os outros a
cada 4 h, e o aviso de resolvido também vai. Com o w2db fora, os alertas que são consequência
dele ficam em silêncio (`inhibit_rules`).

**Quedas:** toda unit do derust tem `OnFailure=w2-notify@%n.service`. Com o `RestartMode=normal`
padrão, cada queda passa pelo estado `failed`, então cada uma gera um aviso no Discord antes do
reinício. O aviso traz:
- o motivo: código de saída, sinal ou core dump;
- os reinícios;
- as últimas linhas do journal;
- com core dump, o resumo do `coredumpctl`. Instale o `systemd-coredump`; as units têm
  `LimitCORE=infinity`.

Numa queda em laço, a mesma unit só avisa de novo depois de 5 min, e o aviso seguinte diz quantas
quedas ficaram sem aviso. O intervalo entre reinícios também dobra até 5 min (`RestartSteps=`).

**Webhooks:** são dois, um por canal. O endereço é segredo (quem o tem posta no canal), então fica
em arquivo de modo 0600 e nunca no log.
- **Alertas:** `/etc/prometheus/discord-alerts-webhook` para o Alertmanager e
  `/etc/derust/discord-alerts-webhook` para o `w2-notify`.
- **Relatos:** `/etc/derust/discord-bugs-webhook` para o w2db (`W2DB_DISCORD_WEBHOOK_FILE`).

## `/bug`

O jogador digita `/bug o que aconteceu` (até 127 caracteres) e o relato segue este caminho:
- **TMSrv:** responde "Relato enviado. Obrigado!", limita a 1 relato por minuto por conexão e manda
  a conta, o personagem, o canal e a posição ao DBSrv (`_MSG_DBBugReport`, 0x828).
- **DBSrv:** registra no log e grava no w2db (`ReportBug`).
- **w2db:** limita a 5 relatos por hora por personagem e grava em `bug_reports`.
- **Discord:** uma tarefa do w2db posta a fila a cada 10 s. O texto do jogador sai escapado, sem
  menção a ninguém (`allowed_mentions` vazio) e sem prévia de link. O relato que o Discord recusa
  volta para a fila; depois de 10 tentativas fica só no banco.

Sem o w2db, o relato fica só no log do DBSrv.

## Backup

O pgBackRest guarda num bucket S3 compatível fora da VPS, com criptografia (`aes-256-cbc`):
- **Completo:** um por semana, no domingo às 04:00 (`w2-backup-full.timer`).
- **Diferencial:** nos outros dias (`w2-backup-diff.timer`).
- **WAL:** contínuo, pelo `archive_command`. O `archive_timeout = 60` fecha o segmento a cada
  minuto, então a perda máxima numa restauração é de cerca de 1 minuto.
- **Retenção:** 4 backups completos, com os diferenciais e o WAL entre eles.

Para instalar (Debian, Postgres 17):

1. `deploy/postgres/derust.conf` em `/etc/postgresql/17/main/conf.d/`; reinicie o Postgres.
2. `deploy/pgbackrest/pgbackrest.conf` em `/etc/pgbackrest/pgbackrest.conf`, com o bucket, o
   endpoint e a região. As chaves e a `repo1-cipher-pass` ficam em
   `/etc/pgbackrest/conf.d/secrets.conf` (modo 0600, dono `postgres`). **Guarde a
   `repo1-cipher-pass` também fora da VPS:** sem ela o backup não abre.
3. `sudo -u postgres pgbackrest --stanza=w2db stanza-create` e
   `sudo -u postgres pgbackrest --stanza=w2db check`.
4. Ative os timers: `systemctl enable --now w2-backup-full.timer w2-backup-diff.timer`. Rode o
   primeiro completo à mão: `systemctl start w2-backup@full.service`.

## Restauração

O roteiro roda numa instância separada: outra máquina com o mesmo `pgbackrest.conf` e os segredos,
ou a própria VPS, num diretório novo. Ele não mexe no banco em produção.

```
python3 tools/w2db_restore.py restore --w2db /caminho/do/w2db [--target '2026-12-01 21:30:00-03'] [--keep]
```

O script segue estes passos:
1. Restaura o último estado, ou o instante de `--target`, num diretório novo.
2. Cria lá um `postgresql.conf` mínimo; no Debian a configuração fica fora do backup.
3. Sobe o Postgres só num socket privado e com `archive_mode=off`, para a instância restaurada
   não mandar WAL de volta ao repositório.
4. Espera o fim da recuperação e roda o `w2db check`. As conferências do `check` estão logo abaixo.
5. Desliga a instância. Com `--keep`, deixa a instância no ar e diz como pará-la.

O `w2db check` só lê, e sai com 1 se achar problema. Ele confere:
- que as migrações são as do binário;
- que as vagas dos itens existem;
- que o personagem do item é da própria conta;
- que não há contestação pela própria conta;
- que todo item tem evento no histórico;
- que a última versão guardada é a atual;
- que cada conta carrega e cabe no arquivo do DBSrv.

Ele também mostra a última gravação e o último evento, para conferir contra o instante pedido.

Para voltar o jogo a um backup:
1. Pare o TMSrv, o DBSrv e o w2db.
2. Restaure com o `pgbackrest restore` padrão no diretório do Postgres, com o Postgres parado
   (`--delta`, e `--type=time --target=...` para um instante).
3. Suba o Postgres e rode o `w2db check`.
4. Suba os serviços.

Para desfazer uma conta só, sem restaurar o banco, use as últimas 20 versões guardadas em
`account_versions`.

## Instalação

Na ordem, depois do Postgres e do pgBackRest acima:

1. `python3 tools/linux_deploy.py /srv/derust --w2db <w2db> --user <usuário>`. Ele monta a árvore
   e gera as units em `/srv/derust/systemd`. O pacote `prometheus-node-exporter` precisa ler
   `/srv/derust/metrics`, então a árvore não pode ficar numa pasta pessoal fechada.
2. `/etc/derust/w2db.env` a partir de `systemd/w2db.env.example` (modo 0600). Coloque também os
   webhooks do Discord em `/etc/derust/`.
3. Copie as units e ative-as. O script imprime o comando.
4. **node_exporter:** em `/etc/default/prometheus-node-exporter`, `ARGS="--web.listen-address=127.0.0.1:9100 --collector.systemd --collector.textfile.directory=/srv/derust/metrics"`.
5. **Prometheus:** `deploy/prometheus/prometheus.yml` em `/etc/prometheus/` e as regras em
   `/etc/prometheus/rules/`. Faça-o ouvir só em `127.0.0.1:9090`, com
   `--web.listen-address=127.0.0.1:9090` em `/etc/default/prometheus`.
6. **Alertmanager:** `deploy/alertmanager/alertmanager.yml` em `/etc/prometheus/`, com
   `--web.listen-address=127.0.0.1:9093` e `--cluster.listen-address=`.
7. **Grafana:** os arquivos de `deploy/grafana/provisioning/` em `/etc/grafana/provisioning/` e o
   painel em `/var/lib/grafana/dashboards/derust/`. O acesso é pelo HTTPS do proxy, como no painel
   de GM.

Só as portas do jogo, 80/443 e 22 ficam abertas; as regras do nftables são do portão do passo 3.

## Testes

- **Rust** (`services/w2db/tests/ops.rs`): o `/metrics`, os relatos num webhook falso (inclusive
  com o webhook fora do ar) e o `w2db check`, que tem de achar dois problemas plantados.
- **C++** (`tests/w2pp_common_tests.cpp`): o formato exato do arquivo `.prom` e a troca atômica.
- **Smoke** (`tools/server_boot_smoke.py`):
  - o passo 17 do LoginClient: `/bug` sem texto, com texto e logo em seguida;
  - o relato nos logs; com `--w2db`, no banco e no webhook falso, escapado;
  - os dois `.prom` e os contadores que o jogo move;
  - o `/metrics` do w2db (a duplicação forçada contada);
  - o `w2db check` no fim.
- **Job `ops` do CI** (Debian 13):
  - `tools/ops_check.py`: toda métrica das regras e do painel existe no código, mais o
    `promtool check config`, o `promtool test rules` e o `amtool check-config`;
  - `tools/w2db_restore.py selftest`, de ponta a ponta com o pgBackRest num repositório local:
    contas sintéticas, backup completo e diferencial, restauração até um instante (o que veio
    depois dele não está lá), restauração até o fim, `w2db check` nas duas, e o arquivamento
    visto no `/metrics`;
  - `systemd-analyze verify` em todas as units geradas.
