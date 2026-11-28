# Painel de GM, convites e cadastro

O `w2gm` (`services/w2gm`, Rust) é a parte da comunidade do teste fechado:
- o **bot do Discord**, com `/convite` e `/registrar`;
- a **página de cadastro**, aberta pelo link de uso único do `/registrar`;
- o **painel de GM**: busca de conta e personagem, inventário com seriais, histórico de item, rollback de personagem, bloqueio de conta e convites.

Ele fala direto com o Postgres do w2db, pela biblioteca do w2db (`w2db::admin`). O DBSrv continua falando só com o `w2db serve`. O `w2gm` roda como uma unit à parte (`w2-w2gm.service`): se ele cair, o jogo continua.

## Como funciona

### Convite e cadastro

1. Um GM usa `/convite membro:@pessoa` no servidor do Discord. Só quem tem o cargo de GM (`W2GM_DISCORD_GM_ROLE`) consegue. O convite fica ligado ao id do Discord da pessoa. O painel também cria convites, pelo id do Discord.
2. A pessoa usa `/registrar` e recebe, só para ela (resposta efêmera), um link `https://<site>/registrar/<token>`. O link vale 30 minutos e uma vez só. O banco guarda só o sha256 do token.
3. Na página, ela escolhe o nome da conta (até 15 letras e números) e a senha (até 15 caracteres). A conta nasce como a do `ImportUser`, sem os itens de bônus. A senha numérica é pedida no primeiro login, dentro do jogo.
4. O convite fica usado e ligado à conta (`invites.account_id`, `accounts.discord_id`). Para uma segunda conta, a pessoa precisa de outro convite.

Um convite aberto pode ser cancelado no painel. Cancelado, o link que já saiu para de valer.

Com `W2DB_REQUIRE_INVITE=true` no w2db, o `ImportUser` do DBSrv deixa de criar contas, e o cadastro pelo link fica sendo o único caminho.

### Contas de GM

São separadas das contas do jogo, e o login é nome, senha e código TOTP:

```bash
read -rs P; printf '%s\n' "$P" | sudo -u <usuário> sh -c \
  'set -a; . /etc/derust/w2gm.env; exec /srv/derust/W2GM/w2gm gm-add <nome>'
```

O comando imprime um endereço `otpauth://` (e o segredo em base32) para o aplicativo autenticador: Aegis, FreeOTP, Google Authenticator. O mesmo comando com um nome que já existe troca a senha e o segredo e derruba as sessões. `w2gm gm-disable <nome>` desativa a conta e derruba as sessões dela.

- A senha de GM tem pelo menos 12 caracteres e vai em argon2id, como a do jogo.
- O mesmo código TOTP não entra duas vezes (`gm_users.totp_last_step`).
- Cinco erros seguidos (senha ou código) bloqueiam a conta de GM por 15 minutos.
- Por IP, são 10 tentativas de login (e 10 de cadastro) a cada 15 minutos.
- A sessão dura 12 horas.

### O painel

- **Busca:** pelo começo do nome da conta ou de um personagem, sem diferença de caixa, ou pelo id do Discord.
- **Conta:** Discord, datas de criação, login e saída, se está no jogo, bloqueios, personagens, todos os itens com serial e onde estão, as versões guardadas e os convites.
- **Item:** onde o serial está e o histórico dele (`item_events`). Isso inclui a suspeita de duplicação que cita o serial e o rollback que o recriou.
- **Bloqueio:** por N dias ou para sempre, com motivo. O login certo passa a responder `STATUS_BANNED`, e o DBSrv manda ao cliente a mensagem de conta bloqueada (`_MSG_DBAccountLoginFail_Block`), a mesma do bloqueio por data. A senha errada continua contando como errada. **O bloqueio vale no próximo login:** quem está no jogo continua até sair. Para tirar alguém do jogo na hora, um personagem de GM usa `/gm kick <personagem>` no jogo (`ProcessImple`).
- **Rollback de personagem:** veja abaixo.
- **Convites:** criar, listar e cancelar.
- **Registro:** cada ação de GM, de convite e de cadastro fica em `gm_audit`, com quem fez, o alvo e o detalhe.

### Rollback de personagem

O banco guarda as últimas 20 versões de cada conta (`account_versions`). O rollback volta **um personagem** para como ele estava numa dessas versões: os dados (nível, gold, posição, skills, quests), o equipamento e o inventário. O baú e os outros personagens ficam como estão.

Para cada item da versão escolhida:
- **o serial ainda é desta conta:** o item volta para a vaga de antes e sai de onde está hoje (do baú, de outro personagem ou de outra vaga);
- **o serial hoje é de outra conta:** o item não volta. O relatório diz qual é e com quem está, e o GM decide;
- **o item não existe mais** (consumido, refinado, vendido): volta com serial novo, e o evento `restore` liga o serial novo ao antigo.

Os itens que o personagem tem hoje e não tinha na versão ficam sem lugar (`remove`), no histórico.

**Só com a conta fora do jogo.** Se a conta estiver carregada no DBSrv, a gravação seguinte do jogo desfaria o rollback, e o painel recusa. Para isso, o DBSrv avisa o w2db quando uma conta entra na lista dele e quando sai (`SetSession`, `accounts.online_since`). Ao subir, ele zera todas as marcas (`ResetSessions`): com o DBSrv fora do ar, as marcas antigas ficam até ele voltar.

O rollback é uma gravação normal da conta (`save_account`): a versão sobe, e o próprio rollback vira uma versão que também pode ser desfeita.

## Instalação na VPS

Depois do w2db (`docs/W2DB.md`, `docs/OPS.md`):

1. **Binário e unit:** `python3 tools/linux_deploy.py /srv/derust --w2db <w2db> --w2gm <w2gm> --user <usuário>`. Ele copia o binário para `W2GM/` e grava `systemd/w2-w2gm.service` e `systemd/w2gm.env.example`.
2. **Configuração:** `/etc/derust/w2gm.env` a partir do modelo, com modo 0600:
   - `DATABASE_URL`, o mesmo do w2db;
   - `W2GM_PUBLIC_URL`, o endereço HTTPS do site;
   - para o bot, `W2GM_DISCORD_TOKEN_FILE`, `W2GM_DISCORD_GUILD` e `W2GM_DISCORD_GM_ROLE`.
3. **Bot do Discord:**
   - Crie uma aplicação no portal de desenvolvedores do Discord e grave o token do bot num arquivo com modo 0600 (`/etc/derust/discord-bot-token`).
   - Convide o bot para o servidor com os escopos `bot` e `applications.commands`. Ele não precisa de nenhuma permissão de canal nem de intents privilegiadas.
   - Os comandos são registrados só nesse servidor (`W2GM_DISCORD_GUILD`). Os ids do servidor e do cargo aparecem no Discord com o modo de desenvolvedor ligado.
4. **HTTPS:** instale o Caddy e copie `deploy/caddy/Caddyfile` para `/etc/caddy/Caddyfile`, trocando o domínio. O DNS precisa apontar para a VPS, e as portas 80 e 443 precisam estar abertas: o Caddy tira e renova o certificado sozinho. O `w2gm` ouve só em `127.0.0.1:8730`.
5. `sudo systemctl enable --now w2-w2gm`. Sem o token, só o site roda. Com o token errado, o processo sai, o systemd reinicia em intervalos crescentes e o `OnFailure=` avisa no Discord.

O primeiro GM é criado com `w2gm gm-add` na própria VPS.

## Segurança

- **Atrás do proxy:** o `w2gm` só fala HTTP, em 127.0.0.1. O HTTPS é do Caddy, que também redireciona o HTTP. O IP do cliente, usado no limite de tentativas, é o último do `X-Forwarded-For` e só vale quando a conexão vem de 127.0.0.1. O Caddy põe esse valor e descarta o que o cliente mandou.
- **Cookie:** o cookie da sessão é `__Host-w2gm`, com `Secure`, `HttpOnly`, `SameSite=Strict` e sem domínio. O banco guarda só o sha256 do token (`gm_sessions`).
- **CSRF:** todo formulário do painel leva o token de CSRF da sessão, comparado sem atalho.
- **Páginas:** são montadas no servidor (maud, que escapa todo texto), sem JavaScript. Os cabeçalhos:
  - `Content-Security-Policy: default-src 'none'; style-src 'self'; form-action 'self'; frame-ancestors 'none'`;
  - `Referrer-Policy: no-referrer`, para o token do cadastro não vazar pelo Referer;
  - `X-Frame-Options: DENY`, `X-Content-Type-Options: nosniff`, `Cache-Control: no-store` e HSTS.
- **Unit:** a unit do `w2gm` roda sem privilégios e sem gravar no disco: `NoNewPrivileges`, `ProtectSystem=strict`, filtro de chamadas de sistema `@system-service` e `MemoryDenyWriteExecute`.
- **Bot:** as respostas são efêmeras e saem sem menções (`allowed_mentions` vazio).
- **Contas de GM:** argon2id, TOTP de uso único e bloqueio depois de cinco erros seguidos.

## Testes

```bash
cd services
export DATABASE_URL=postgres://w2@localhost/w2db
cargo test -p w2db --test admin
cargo test -p w2gm
```

- **`services/w2db/tests/admin.rs`**, com o serviço gRPC do outro lado:
  - sessões: marca, desmarca e `ResetSessions`;
  - bloqueio no login: por um dia, para sempre, desbloqueio e bloqueio vencido;
  - rollback com cada caso de item: volta com o serial, volta com serial novo, ficou com outra conta e saiu do personagem. Também a recusa com a conta no jogo e o jogo gravando depois sem suspeita de duplicação;
  - cadastro pelo link: nomes inválidos, uso único, nome já usado sem perder o link, convite cancelado e link vencido;
  - a busca.
- **`services/w2gm/tests/web.rs`**, o site de ponta a ponta:
  - login com TOTP: senha errada, código errado, nome inexistente e o mesmo código duas vezes;
  - o cookie e o CSRF;
  - sair, o bloqueio por erros e a conta de GM desativada;
  - o cadastro pelo link do `/registrar`, as respostas do bot, os cabeçalhos de segurança, o HTML escapado e o rollback recusado com a conta no jogo.
- **Smoke** (`tools/server_boot_smoke.py --w2db`):
  - as sessões do DBSrv de verdade: `ResetSessions` na subida, `SetSession` na entrada e na saída de cada conta, e nenhuma conta marcada no fim;
  - o LoginClient no modo `banned` contra uma conta bloqueada: recebe a mensagem de bloqueio, sem o `0x10A`.
- **Job `ops` do CI:**
  - gera a `w2-w2gm.service` e confere todas as units com `systemd-analyze verify`;
  - `tools/ops_check.py` valida o Caddyfile com `caddy validate`.
