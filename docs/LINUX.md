# Servidores no Linux

O DBSrv e o TMSrv compilam e rodam só no Linux x86_64. O cliente (TMProject, DirectX 9) continua só no Windows. O código dos servidores veio do Windows; o que era da janela e do Winsock virou o laço de eventos e as chamadas POSIX descritos em [Como os servidores rodam no Linux](#como-os-servidores-rodam-no-linux), sem mudar o comportamento do jogo.

## Build

O DBSrv fala com o serviço de persistência (`docs/W2DB.md`) por gRPC. No Debian ou no Ubuntu:

```bash
sudo apt install libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc
```

Sem esses pacotes, o CMake avisa e monta o DBSrv só com os arquivos (`DERUST_W2DB=OFF`).

```bash
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
```

Os binários ficam em `build/linux/server/dbsrv/DBSrv` e `build/linux/server/tmsrv/TMSrv`. Fora do Windows o cliente é desligado (`DERUST_BUILD_CLIENT` vale `OFF`), e no Windows os servidores (`DERUST_BUILD_SERVER` vale `OFF` e ligar dá erro).

Com AddressSanitizer e UBSan, como no CI:

```bash
cmake -S . -B build/linux-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/linux-asan --parallel
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/linux-asan --output-on-failure
```

O teste de jogo aceita os binários Linux. Coloque `DBSrv`, `TMSrv` e o `LoginClient.exe` do build mingw (`tools/mingw_check.py`) num diretório e rode:

```bash
python3 tools/server_boot_smoke.py <diretório> --require-login
```

O LoginClient roda no wine. O teste fica sendo um cliente Windows jogando contra servidores Linux. A árvore que o teste monta é a mesma da produção (próxima seção).

Para o mesmo teste com as contas no Postgres, use `--w2db services/target/debug/w2db`. O Postgres é o do sistema: o teste roda `initdb` no próprio diretório. Com `--forced-dupe`, o teste também força uma duplicação e exige que ela seja acusada. Isso precisa de um TMSrv montado com `-DDERUST_TEST_HOOKS=ON`, que nunca vai para a produção. Detalhes em `docs/W2DB.md`.

Avisos: `tools/linux_warning_check.py` compila o Release do zero e compara os avisos com a linha de base. Os tipos acompanhados são os mesmos da checagem mingw. Há uma linha de base por versão do gcc, `tools/linux_warning_baseline-gcc<N>.txt`, e a comparação vale por tipo e por arquivo. O CI usa esse mesmo build para os testes e para o artefato. Depois de remover ou justificar avisos:

```bash
python3 tools/linux_warning_check.py --build-dir build/linux --update-warning-baseline
```

## Implantação e systemd

`tools/linux_deploy.py` monta a árvore da produção e gera as unidades do systemd:

```bash
python3 tools/linux_deploy.py /srv/w2 --bin build/linux --user w2   # ou --bin com o artefato derust-linux-x86_64 do CI
```

O script:
- cria `Common/`, `DBSRV/Run/` e `TMSRV/Run/`;
- cria os diretórios em que os servidores gravam e que não criam sozinhos: `Log`, `DayLog`, `chat_log`, `item_log`, `account/<inicial>`, `char/<inicial>`, `capsule`, `npc`, e `Import*`, `Error` e `status` em `Common`;
- copia os binários para os `Run`;
- confere a caixa dos arquivos de dados contra os nomes que o código abre, tirados das próprias fontes. `--fix-case` renomeia arquivos e diretórios para a caixa certa, e `--check-only` só confere;
- grava `systemd/w2-dbsrv.service` e `systemd/w2-tmsrv.service` com os caminhos da árvore;
- com `--w2db <binário>`, copia o w2db para `W2DB/` e grava `w2-w2db.service` e um modelo do `/etc/derust/w2db.env`. O DBSrv passa a usar o socket do w2db (`W2DB_SOCKET`);
- com `--w2gm <binário>` (junto do `--w2db`), copia o painel de GM para `W2GM/` e grava `w2-w2gm.service`, sem privilégios, e um modelo do `/etc/derust/w2gm.env` (`docs/GM.md`);
- grava as unidades de operação (`docs/OPS.md`): `w2-notify@.service` (aviso no Discord quando uma unidade falha), `w2-backup@.service` e os timers do backup. Copia os scripts delas para `bin/` e cria `metrics/`, a pasta do `W2_METRICS_DIR` (`--metrics-dir` para outra).

Ele não copia os dados do jogo nem cria arquivos de configuração (`localip.txt`, `serverlist.txt`, `gameconfig.txt`...): isso fica com quem opera. A lista dos arquivos que o código abre e que ainda faltam sai no fim.

Para instalar as unidades:

```bash
sudo cp /srv/w2/systemd/w2-*.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now w2-dbsrv w2-tmsrv
journalctl -u w2-tmsrv -f        # alertas e eco dos comandos
```

Como as unidades se comportam:
- `systemctl stop` manda `SIGTERM`, que o servidor recebe como o pedido de encerramento: grava e sai, com até 60 s de prazo.
- `Restart=always` reinicia em 5 s sempre que o processo sai sem um `stop`. Isso cobre:
  - uma queda;
  - os comandos `reboot` e `exit` do FIFO de controle (para desligar de vez, use `systemctl stop`);
  - o TMSrv que sai quando não alcança o DBSrv no boot. A unidade dele vem depois da do DBSrv (`After=`), mas o DBSrv pode ainda não estar escutando.
- `LimitNOFILE=8192`: o padrão de 1024 descritores fica curto para os 1000 jogadores (`MAX_USER`), mais os arquivos e as conexões entre servidores.
- Numa queda em laço, o intervalo entre reinícios dobra até 5 min (`RestartSteps=`, `RestartMaxDelaySec=`).
- Cada queda chama `w2-notify@.service` (`OnFailure=`), que avisa no Discord com o motivo e o fim do journal. `LimitCORE=infinity` deixa o core dump para o `systemd-coredump` (`docs/OPS.md`).

## Como os servidores rodam no Linux

Os servidores rodam só no Linux. O código veio do Windows, e até a branch `refactor/linux-only-servers` uma camada `server/compat` emulava o Win32 por baixo dele. Hoje as fontes usam direto as chamadas do Linux, por três arquivos de `server/common`:

- **`EventLoop.h`: laço de eventos.** Faz o papel do laço de mensagens da janela do Windows.
  - Espera com `poll()` pelos sockets registrados com `W2LoopWatch`, pelo FIFO de controle e pelos timers de `W2LoopSetTimer`, no relógio monotônico.
  - Cada evento vai para o tratador de cada servidor, `OnServerEvent`, cujos casos são os do antigo `MainWndProc`.
  - A ordem é a do Windows: a fila esvazia antes da próxima espera, e depois de `W2LoopQuit` o que já estava na fila ainda é entregue.
  - Os sockets usam os canais `NET_*` de `CPSock.h` (`NET_READ`, `NET_ACCEPT`, `NET_READDB`...), os antigos `WSA_*`.
- **Sockets.**
  - `W2LoopWatch` deixa o socket não bloqueante, como o `WSAAsyncSelect` fazia.
  - `CPSock::StartListen` liga `SO_REUSEADDR`. Sem ela, o servidor não sobe por cerca de 60 s depois de derrubado, enquanto houver conexões em TIME_WAIT na porta.
  - Quando um socket de dados fica legível, o laço espia um byte (`MSG_PEEK`): dados viram `W2_NET_READ`, e 0 bytes ou erro viram um único `W2_NET_CLOSE`, depois dos dados que restavam. O Winsock entregava nessa mesma ordem. `CPSock::Receive` não trata `recv() == 0` como fechamento, porque o fechamento chega como `W2_NET_CLOSE`.
- **`System.h`: arquivos.**
  - `W2FindFirst`/`W2FindNext` listam diretórios com `glob()`. `*.*` casa nomes sem ponto, e `.` e `..` vêm primeiro: os laços que pulam a primeira entrada e o limite de 10 entradas do `ImportItem` contam com isso, como no `FindFirstFile`.
  - `W2MoveFile` falha se o destino existir, como o `MoveFile` (o `rename()` substituiria o destino).
  - `BASE_InitModuleDir` muda para o diretório do executável, lido de `/proc/self/exe`.
- **`Platform.h`: o que também compila no Windows.** Fica portável porque a checagem de divergências (`tools/divergence_map.py`) compila as regras de `server/common` com mingw e roda no wine. Tem `W2TickCount`, `W2SleepMs`, `W2StrUpper`, `W2Rand`, `ServerAlert` e os tipos `BOOL`/`TRUE`/`FALSE` que o código usa.
- **Estado.** Fica em estáticos locais. Construtores globais, como o `RankingSystem` do DBSrv, listam arquivos antes de `main`.
- **Texto.** O `CharNext` do original avançava um byte no código de página de um Windows ocidental (1252), e o código agora faz isso direto.

O que deixa o comportamento igual ao do Windows:
- **Números aleatórios:** `W2Rand`, em `common/Platform.h`, usa o algoritmo do `rand()` do CRT da Microsoft, com a mesma semente. Chances de drop, refinação e dano são as mesmas.
- **Arquivos e pacotes:** os campos têm largura fixa. `ProtocolLayoutCheck.cpp` trava os tamanhos e offsets também no Linux x86_64. Os arquivos de conta e os `.bin` são os mesmos nos dois sistemas.
- **Ponto flutuante:** é SSE2, como no MSVC Win32. `-ffp-contract=off` impede o GCC de fundir multiplicação e soma.
- **Testes de caracterização:** os hashes conferem no Linux, inclusive as fórmulas de ponto flutuante.

## O que muda para quem opera

- **Diretórios.**
  - Os servidores abrem `../../Common/...`, e um abre arquivos do outro: `../../TMSRV/Run/...` e `../../DBSRV/Run/...`. A árvore esperada é `<raiz>/Common`, `<raiz>/DBSRV/Run` e `<raiz>/TMSRV/Run`, com essa caixa.
  - Cada servidor muda para o diretório do próprio executável ao iniciar (`BASE_InitModuleDir`), então o binário fica dentro do seu `Run`.
- **Nomes de arquivo.** No Linux a caixa conta. Os arquivos precisam ter exatamente o nome que o código abre, por exemplo `localip.txt`, `serverlist.txt`, `itemlist.csv`, `ItemList.bin`, `ExtraItem.bin`, `SkillData.csv` e `Guilds.txt`. Uma árvore copiada do Windows pode ter outra caixa. A lista completa sai de:

  ```bash
  grep -rhoE '"[^"]*\.(txt|csv|bin|dat|h)"' server --include=*.cpp | sort -u
  ```

- **Mensagens.** O que no Windows era uma `MessageBox` é uma linha `[título] texto` no stderr (`ServerAlert`). Guarde o stderr num log, por exemplo pelo journal do systemd.
- **Encerramento.** `SIGTERM`, `SIGINT` e o comando `exit` seguem o caminho de fechar a janela no Windows. As perguntas de confirmação de lá ("Você realmente deseja desligar o server?") saem como alerta no stderr: o sinal já é a resposta de quem opera. Os servidores gravam guildas, configuração e logs antes de sair.
- **Painel e menu.** O painel da janela não existe. Ele só mostrava o que o TMSrv grava no `gameconfig.txt` e, no DBSrv, os servidores e admins conectados. Os comandos do menu vão pelo FIFO de controle (próxima seção).
- **`gameconfig.txt`.** O TMSrv o regrava a cada mudança de evento feita por um GM e quando não o encontra no boot (`WriteGameConfig`). Até esta branch, no Linux, a gravação era pulada sem aviso: a função antiga desenhava o painel antes de gravar e saía quando a camada não dava onde desenhar. O arquivo gravado agora é idêntico, byte a byte, ao do build Windows.

## Comandos do menu

Cada servidor cria, no próprio diretório, um FIFO `control.fifo` com permissão 0600: só o usuário que roda o servidor escreve nele. A variável `W2_CONTROL` troca o caminho. Cada linha escrita no FIFO é o nome de um item do menu da janela, sem o `&`, sem espaços e sem diferença de maiúsculas, ou o id dele. A linha vira o comando que o clique no menu mandaria, então o código que roda é o mesmo do Windows.

```bash
echo saveall > TMSRV/Run/control.fifo
echo help    > TMSRV/Run/control.fifo   # lista os comandos no stderr
```

| Servidor | Comando | O que faz |
|---|---|---|
| TMSrv | `saveall` | grava todos os personagens no DBSrv (`SaveAll`) |
| TMSrv | `readgameconfig` | relê o `gameconfig.txt` (`ReadConfig`) |
| TMSrv | `readguild` | relê as guildas (`CReadFiles::ReadGuild`) |
| TMSrv | `readguildname` | relê os nomes de guilda (`BASE_InitializeGuildName`) |
| TMSrv | `reloadmob` | relê o `NPCGener.txt` (`ReadNPCGenerator`) |
| TMSrv | `reboot` | começa a contagem de desligamento (`ServerDown = 1`) |
| TMSrv | `exit` | encerra, como `SIGTERM` |
| DBSrv | `reloadmobbase` | relê os personagens-base de `BaseMob/` |
| DBSrv | `exit` | encerra, como `SIGTERM` |

O servidor ecoa cada comando no stderr (`controle: SaveAll (9003)`). Um nome desconhecido também aparece no stderr, junto com a lista. Com o servidor parado, ninguém lê o FIFO e o `echo` fica esperando. Use `timeout 5 sh -c 'echo saveall > control.fifo'` num script.

Cada servidor registra os seus comandos no início do `main` (`RegisterCommands`, com `W2LoopAddCommand`), com os nomes dos itens do menu do Windows.

## Página de status dos canais

A lista de canais do cliente mostra a lotação de cada canal e marca os que estão fora do ar. Os números vêm de uma página que o DBSrv grava a cada 30 s e que o cliente baixa por HTTP:

- **O arquivo.** `Common/status/serv<NN>.htm`, com `NN` o índice do grupo do DBSrv no `serverlist.txt`. A variável `W2_STATUS_DIR` troca o diretório. Apesar do nome, é texto puro: uma linha por canal, `-1` para canal sem TMSrv e a lotação para os outros. A lotação é a do jogo mais um terço, mais de 0 a 3, como no original, e o cliente escreve "FULL" acima de 600.
- **A gravação.** O DBSrv grava num `.part` ao lado e troca o nome no fim, então o servidor web nunca entrega a página pela metade. Sem o diretório, a gravação é pulada sem aviso, e a lista de canais aparece toda fora do ar. O login continua funcionando.
- **A URL.** O cliente lê a URL da coluna 0 do grupo no `serverlist.bin` dele, por exemplo `http://jogo.exemplo/status/serv00.htm`. Essa lista é a do cliente, não o `serverlist.txt` do servidor.
- **O servidor web.** O DBSrv não serve a página: no Windows quem a servia era o Apache do WampServer (o caminho original era `C:/wamp/www`). No Linux, um nginx basta:

  ```nginx
  server {
      listen 80;
      location /status/ {
          alias /srv/w2/Common/status/;
          default_type text/plain;
          add_header Cache-Control no-store;
      }
  }
  ```

  Os arquivos saem com permissão 0644, legíveis pelo usuário do nginx.

O histórico de lotação vai para `Common/data<NN>.csv`: a cada 15 minutos, uma linha com a data e a hora, o pico de cada canal no período e o total.

## Borda de rede do TMSrv

O TMSrv recusa, por IP (`server/tmsrv/net/NetGuard.h`):
- a sexta conexão simultânea;
- mais de 10 conexões novas em 10 s;
- mais de 5 pedidos de login de conta em 60 s. O jogador recebe "Muitas tentativas de login" e a conexão fecha.

A conexão que não termina o login da conta em 30 s, contados do aceite, é fechada no `CheckIdle`. Isso inclui a que não manda o `INITCODE`.

O despacho (`ProcessClientMessage`) descarta o pacote menor do que a estrutura do seu tipo. Os tamanhos estão em `ClientMessageMinSize`, e o descarte grava `err,packet short` no log. As recusas da borda gravam `err,netguard ...`. Atrás de NAT, até 5 jogadores dividem um IP.

## Fuzzing

Dois alvos do libFuzzer, em `tests/fuzz`, compilados com clang, ASan e UBSan:
- `fuzz_sock_read`: bytes crus no `CPSock::ReadMessage`;
- `fuzz_client_message`: o TMSrv inteiro (`tmsrv_core`, o servidor sem o `main`) com a árvore sintética do teste de jogo. Cada entrada leva um jogador ao estágio escolhido (conectado, seleção de personagem ou em jogo, com o DBSrv imitado) e manda até 16 pacotes no `ProcessClientMessage`.

```bash
cmake -S . -B build/fuzz -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DDERUST_BUILD_FUZZERS=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/fuzz --parallel --target fuzz_sock_read fuzz_client_message
python3 tools/fuzz_tmsrv.py build/fuzz --regress                    # entradas de regressão
python3 tools/fuzz_tmsrv.py build/fuzz --time 3600 --work build/fz  # rodada longa
```

Os resultados de `tools/fuzz_tmsrv.py`:
- o corpus fica em `<work>/corpus/<alvo>`;
- a entrada que derruba o alvo fica em `<work>/crash/<alvo>/`;
- a saída inteira fica em `<work>/log/<alvo>.txt`. Algumas quedas dependem do estado acumulado do mundo e não se repetem com a entrada sozinha.

Cada bug corrigido ganha uma entrada em `tests/fuzz/regress/<alvo>`. O CI reexecuta essas entradas e roda 60 s por alvo (job `fuzz`).

## Pendências

- Nenhuma do port. Os `sprintf` que o gcc do Ubuntu apontava com `-Wformat-overflow` foram corrigidos (`docs/ARCHITECTURE.md`, item 27).
