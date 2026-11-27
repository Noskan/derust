# Servidores no Linux

O DBSrv e o TMSrv compilam e rodam nativos no Linux x86_64. O cliente (TMProject, DirectX 9) continua só no Windows. As fontes são as mesmas dos dois lados: no Linux, a camada `server/compat` emula o pequeno subconjunto do Win32 que os servidores usam, e o código do jogo não muda.

## Build

```bash
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
ctest --test-dir build/linux --output-on-failure
```

Os binários ficam em `build/linux/server/dbsrv/DBSrv` e `build/linux/server/tmsrv/TMSrv`. Fora do Windows o cliente é desligado (`DERUST_BUILD_CLIENT` vale `OFF`).

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

O LoginClient roda no wine. O teste fica sendo um cliente Windows jogando contra servidores Linux.

## Como a camada funciona

`server/compat/posix/` tem os cabeçalhos que as fontes incluem (`Windows.h`, `windows.h`, `io.h`, `winsock2.h`, `mbstring.h` e outros). Todos apontam para `server/compat/w2_win32.h`. No Windows esse diretório não entra no build.

- **Laço de mensagens.**
  - `RegisterClass` guarda o `MainWndProc`, e `CreateWindow` devolve uma janela fictícia.
  - `GetMessage` espera com `poll()` pelos sockets registrados com `WSAAsyncSelect` e pelos timers de `SetTimer`. Cada evento vira a mensagem que o Windows entregaria: `WSA_*` com o socket em `wParam` e o evento em `lParam`, ou `WM_TIMER`. As mensagens são despachadas ao mesmo `MainWndProc`.
- **Sockets.**
  - `WSAAsyncSelect` deixa o socket não bloqueante, como no Winsock.
  - Quando um socket de dados fica legível, o laço espia um byte (`MSG_PEEK`): 0 bytes vira `FD_CLOSE` e dados viram `FD_READ`. O Winsock entrega nessa mesma ordem. `CPSock::Receive` não trata `recv() == 0` como fechamento, porque no Windows o fechamento chega como `FD_CLOSE`.
- **Arquivos.**
  - `_open`/`_read`/`_write`, `_findfirst` e `FindFirstFile` funcionam sobre POSIX.
  - `*.*` casa nomes sem ponto, e `.` e `..` vêm primeiro. Os laços `while (FindNextFile(...))` pulam a primeira entrada contando com isso.
  - Os handles de busca são números pequenos, porque as fontes guardam o retorno de `_findfirst` num `int`.
- **Estado da camada.** Fica em estáticos locais. Construtores globais, como o `RankingSystem` do DBSrv, chamam a camada antes de `main`.
- **Texto.** `_mbschr` e `CharNext` seguem o código de página de um byte (1252) de um Windows ocidental, que é o padrão do CRT ao iniciar.

Fora da camada, o que já deixava o comportamento igual:
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

- **Mensagens.** Toda `MessageBox` vira uma linha `[título] texto` no stderr. Guarde o stderr num log, por exemplo pelo journal do systemd.
- **Encerramento.** `SIGTERM` e `SIGINT` chegam como `WM_CLOSE`, o mesmo caminho de fechar a janela no Windows. As perguntas de confirmação recebem a resposta afirmativa, e os servidores gravam guildas, configuração e logs antes de sair.
- **Painel.** O painel da janela não existe. Os comandos do menu (no TMSrv: recarregar configuração, guildas e mobs, salvar tudo, reiniciar; no DBSrv: desligar) ainda não têm equivalente no Linux.

## Pendências

- Os comandos do menu da janela, para quem opera o servidor no Linux.
- Os avisos do gcc nativo ainda não têm linha de base. Hoje são 41 e nenhum é bug: casts de `HMENU` para o menu, que não existe no Linux, `int*` no lugar de `socklen_t*`, que tem o mesmo tamanho, e `NULL` comparado com inteiros.
