# derust

Monorepo unificando o servidor (**W2PP**) e o cliente (**TMProject**) do *With Your Destiny (WYD)*, preservando o histórico completo de commits de ambos os repositórios originais.

## Estrutura do Projeto

- [`server/`](server/README.md) — Servidor C++ importado de [Noskan/W2PP](https://github.com/Noskan/W2PP), organizado em módulos:
  - `server/common/` (`w2pp_common`) — Tipos base, pacotes, socket (`Basedef`, `CPSock`, `ItemEffect`) e trava de layout ABI.
  - `server/dbsrv/` (`DBSrv`) — Servidor de banco de dados em arquivos (`core/`, `persistence/`, `net/`, `resources/`).
  - `server/tmsrv/` (`TMSrv`) — Game server (`core/`, `entities/`, `net/`, `systems/`, `handlers/{account,combat,items,combine,trade,social,world}/`, `resources/`).
  - `server/tools/client_patch/` (`ClientPatch_v7662`) — DLL opcional de patch para cliente v7662 (`-DW2PP_BUILD_CLIENT_PATCH=ON`).
- [`client/`](client/README.md) — Cliente C++ DirectX 9 importado de [Noskan/tm-project](https://github.com/Noskan/tm-project), organizado em:
  - `client/src/` (`TMProject`) — `core/`, `common/`, `net/`, `crypto/` (`tm_crypto`), `render/`, `audio/`, `ui/`, `scene/`, `world/`, `entities/`, `skills/`, `effects/`, `gameplay/` e `resources/`.
  - `client/third_party/directx/` (`tm_directx`) — Headers e bibliotecas do DirectX 9 SDK.
  - `client/docs/` — Documentação auxiliar do cliente.
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — Mapa completo de módulos, diagrama de dependências, travas de ABI, regras de fronteira e roteiro da Fase 2.
- [`tools/check_boundaries.py`](tools/check_boundaries.py) — Verificador automático de fronteiras de dependência entre módulos.
- [`tests/`](tests/CMakeLists.txt) — Suíte de testes unitários CTest (`w2pp_common_tests`) para funções puras e layout binário do servidor.

## Build (Windows Win32 / MSVC)

```powershell
# Configurar (Visual Studio 2022 Win32)
cmake --preset win32-msvc

# Ou autodetectando a versão do Visual Studio instalada
cmake -S . -B build/win32-msvc -A Win32

# Compilar Debug ou Release
cmake --build build/win32-msvc --config Debug --parallel
cmake --build build/win32-msvc --config Release --parallel

# Executar testes unitários (CTest)
ctest --test-dir build/win32-msvc -C Debug --output-on-failure
```

Para verificar localmente as fronteiras de `#include`:

```bash
python3 tools/check_boundaries.py
```

Checagem cruzada com mingw-w64 i686 no Linux. Ela compila todas as TUs (`-O2`, ponto flutuante SSE2 como o MSVC), linka o DBSrv e o TMSrv, roda os testes de `tests/` no wine e compara os avisos do GCC acompanhados com a linha de base da versão do compilador (`tools/mingw_warning_baseline-gcc<N>.txt`). As TUs que ainda só compilam no MSVC ficam em [`tools/mingw_known_failures.txt`](tools/mingw_known_failures.txt).

```bash
sudo apt install g++-mingw-w64-i686 wine xvfb   # wine32 para os testes; xvfb para o teste de partida
python3 tools/mingw_check.py --keep build/mingw
python3 tools/server_boot_smoke.py build/mingw   # sobe DBSrv + TMSrv no wine com dados sintéticos
python3 tools/mingw_check.py --update-warning-baseline   # após remover ou justificar avisos
```

Testes (CTest no MSVC; no wine pelo `mingw_check`):
- [`w2pp_common_tests`](tests/w2pp_common_tests.cpp): casos unitários e travas de ABI.
- [`w2pp_characterization_tests`](tests/w2pp_characterization_tests.cpp): hashes de referência de ~40 funções `BASE_*` com dados sintéticos, sem arquivos do jogo. Os hashes foram gerados com o código da tag `pre-modularization` e conferem com o atual. Após uma mudança **intencional** de regra, regrave com `w2pp_characterization_tests --print`.
- [`tmproject_characterization_tests`](tests/tmproject_characterization_tests.cpp): o mesmo para ~55 funções `BASE_*` do cliente (`client/src/common`). Regrave com `tmproject_characterization_tests --print`.
- [`server_boot_smoke.py`](tools/server_boot_smoke.py): sobe o DBSrv e o TMSrv com uma árvore de dados sintética (sem arquivos do jogo): no CI, o build mingw no wine e os builds MSVC Debug/Release nativos no Windows. No Windows: `python tools/server_boot_smoke.py <dir com DBSrv.exe e TMSrv.exe>`. Confere que o TMSrv conecta no DBSrv, que as portas 7514/8895/8281 aceitam conexão e que os dois seguem no ar sem MessageBox nem exceção. Com o `LoginClient.exe` ([`tools/login_client`](tools/login_client/LoginClient.cpp), montado com o `CPSock` e as structs do cliente) no mesmo diretório, faz também o login de ponta a ponta: cria a conta pelo `ImportUser` do DBSrv, entra, define a senha numérica, cria e apaga personagens e entra no jogo, conferindo cada resposta no layout do cliente (o CI exige isso com `--require-login`). Não cobre a jogabilidade depois da entrada.

Bugs latentes do código legado encontrados por essas ferramentas, os mantidos por paridade e os já corrigidos (cada um com teste): [`docs/BUGS_LATENTES.md`](docs/BUGS_LATENTES.md).

## Status da Modularização

- **Fase 1 (concluída)**: Reorganização modular via `git mv`, migração para CMake + presets Win32, pipeline CI no GitHub Actions (`windows-latest` Debug/Release), verificador de fronteiras e travas de ABI em `ProtocolLayoutCheck.cpp`.
- **Fase 2 — Onda 1 (concluída)**: Limpeza de `ItemEffect.h` para PT-BR, `.clang-format`, documentação Doxygen/inline em PT-BR e fatiamento dos 4 maiores *god-files*: `server/common/Basedef.cpp` (`Basedef`/`BaseItem`/`BaseCombat`), `tmsrv/core/Server.cpp` (5 sistemas em `tmsrv/systems/`), `TMFieldScene.cpp` (5 TUs) e `TMHuman.cpp` (5 TUs).
- **Fase 2 — Onda 2 (concluída)**: Fatiamento de `_MSG_UseItem.cpp` (5 sub-handlers), `_MSG_Quest.cpp` (3 sub-handlers), `MobKilled.cpp` (Exp/Drops/PvP), `CFileDB.cpp` (pacotes de conta e mundo), `SGrid.cpp` (4 TUs) e `TMGround.cpp` (3 TUs), além da suíte CTest [`w2pp_common_tests`](tests/w2pp_common_tests.cpp) integrada ao CI.
- **Fase 2 — Onda 3 (concluída)**: Fatiamento de `dbsrv/core/Server.cpp` (Admin/Config/Logs), `ProcessSecMinTimer.cpp` (`ScheduledEventSystem` e `MobAISystem` em `tmsrv/systems/`), `RenderDevice.cpp` (Shaders/Rect/Geom), `client/src/common/Basedef.cpp` (`BaseItem`/`BaseCombat`/`BaseWorld`), `SControl.cpp` (Text/Buttons/List) e `TMScene.cpp` (Resources/Packets/Ground).
- **Fase 2 — Onda 4 (concluída)**: Segunda passada isolando funções gigantes em TUs próprias: `tmsrv/core/Server.cpp` (Window/Config/Logs), `server/common/BaseLoaders.cpp`, `TMFieldScene_*` (Panels/HUD/MsgBox/PacketAttack/PacketItems/Init/FrameMove/AutoSkill/Items) e `TMHuman_*` (FrameMove/Render).
- **Fase 3 (em andamento)**: Job de CI `mingw-check` (cross-compile mingw + testes no wine) e remoção das construções exclusivas do MSVC concluídos; decomposição das funções gigantes: 71 handlers de `OnControlEvent` em `TMFieldScene_Controls.cpp`, etapas nomeadas em `FrameMove`, `SkillUse`, `AutoSkillUse` e `OnPacketAttack` (inclusive blocos com retorno antecipado, via `bool Etapa(..., int& nReturnValue)`), e decomposição recursiva das maiores etapas e de `TMHuman::InitObject`/`FrameMove` (59 métodos; ~5.800 para ~2.100 linhas nas seis funções). Verificação sem rodar o jogo: testes de caracterização de `BASE_*` conferidos contra a `pre-modularization`, checagem de avisos do GCC com linha de base e tag `pre-phase3-decomposition` para `git bisect`. Ver [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).
- **Alinhamento com o cliente (em andamento)**: o servidor e o cliente vêm de versões diferentes do jogo; o alvo é a versão do cliente. O login já está no layout do cliente (`STRUCT_MOB` de 1024 bytes, pacotes 0x20D/0xFDE/0x20F/0x211/0x213/0x114 e `APP_VERSION` 1758), travado por `static_assert` em `server/common/ProtocolLayoutCheck.cpp` e pelo login de ponta a ponta no CI. O mapa do que difere (pacotes casados por opcode, structs medidas pelo compilador e regras `BASE_*` rodadas com as mesmas entradas nos dois lados) está em [`docs/DIVERGENCIAS.md`](docs/DIVERGENCIAS.md), gerado por `python3 tools/divergence_map.py`.

## Histórico e Licença

- O histórico completo de commits foi preservado via `git mv` (use `git log --follow -- <arquivo>`).
- Ambos os projetos originais são licenciados sob **GNU GPL v3** (veja [`server/license.txt`](server/license.txt)).
