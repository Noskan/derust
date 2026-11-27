# Arquitetura do Monorepo `derust`

Este documento descreve a organização modular de **W2PP** (`server/`) e **TMProject** (`client/`), estabelecida na **Fase 1** da reestruturação, além das regras de fronteira, travas de ABI e o roteiro da **Fase 2**.

## 1. Visão Geral

O monorepo reúne o servidor e o cliente C++ do *With Your Destiny (WYD)* sob um único sistema de build **CMake** (MSVC Win32/x86, C++17), preservando os históricos completos dos repositórios originais.

> [!IMPORTANT]
> `server/` e `client/` continuam com o próprio módulo `common/` cada um, mas os layouts de pacote e de struct já foram alinhados ao cliente (itens 11 a 14 abaixo). O que tem a mesma definição nos dois lados (nome, layout e tipo de cada campo) fica em `shared/protocol/` e é incluído pelos dois `common/Basedef.h`: hoje `STRUCT_ITEM`, `STRUCT_TREASURE`, `STRUCT_DAM`, `STRUCT_SPELL` e `STRUCT_AFFECT` (itens 15 e 16). `shared/` só pode incluir o próprio `shared/` (`tools/check_boundaries.py`).

---

## 2. Mapa de Módulos

### 2.1 Servidor — `server/` (W2PP)

| Diretório | Target CMake | Responsabilidade |
| --- | --- | --- |
| `server/common/` | `w2pp_common` (STATIC) | Tipos base, pacotes, criptografia/socket (`Basedef`, `CPSock`, `ItemEffect`) e trava de layout ABI (`ProtocolLayoutCheck.cpp`). |
| `server/dbsrv/core/` | `DBSrv` (WIN32) | Ponto de entrada (`Server`), protocolo admin (`Server_Admin`), configuração (`Server_Config`), logs (`Server_Logs`), leitura de arquivos de dados (`CReadFiles`), `stdafx.h`. |
| `server/dbsrv/persistence/` | `DBSrv` | Persistência de contas/personagens em disco (`CFileDB`) e ranking (`CRanking`). |
| `server/dbsrv/net/` | `DBSrv` | Sessões conectadas de GameServers no DBSrv (`CUser`). |
| `server/dbsrv/resources/` | `DBSrv` | Script de recursos Win32 (`DBSrv.rc`, `resource.h`, `icon1.ico`). |
| `server/tmsrv/core/` | `TMSrv` (WIN32) | Loop principal (`Server`), timers de segundo/minuto (`ProcessSecMinTimer`), leitura de dados (`CReadFiles`), strings (`Language.h`). |
| `server/tmsrv/entities/` | `TMSrv` | Entidades e geradores de mundo (`CMob`, `CItem`, `CUser`, `CNPCGene`). |
| `server/tmsrv/net/` | `TMSrv` | Roteamento cliente/DB (`ProcessClientMessage`, `ProcessDBMessage`) e helpers de envio/consulta (`SendFunc`, `GetFunc`). |
| `server/tmsrv/systems/` | `TMSrv` | Sistemas de mundo/eventos (`MobKilled`, `MobAISystem`, `ScheduledEventSystem`, `CCastleZakum`, `CWarTower`, comandos `imple`). |
| `server/tmsrv/handlers/account/` | `TMSrv` | Login, senha numérica, criação/remoção e seleção de personagem (`_MSG_AccountLogin`, `_MSG_CharacterLogin`, etc.). |
| `server/tmsrv/handlers/combat/` | `TMSrv` | Ataque, movimento, ações, distribuição de pontos, barra de skills, PK (`_MSG_Attack`, `_MSG_Action`, etc.). |
| `server/tmsrv/handlers/items/` | `TMSrv` | Uso, compra/venda NPC, drop/pickup, baú, divisão e cápsula (`_MSG_UseItem`, `_MSG_Buy`, `_MSG_Deposit`, etc.). |
| `server/tmsrv/handlers/combine/` | `TMSrv` | Composições de NPCs (`_MSG_CombineItem*`: Agatha, Ailyn, Alquimia, Ehre, Extração, Lindy, Odin, Shany, Tiny). |
| `server/tmsrv/handlers/trade/` | `TMSrv` | Troca direta e loja pessoal (`_MSG_Trade`, `_MSG_TradingItem`, `_MSG_SendAutoTrade`, `_MSG_ReqBuy`, etc.). |
| `server/tmsrv/handlers/social/` | `TMSrv` | Chat, sussurro, grupo, guilda, guerra e duelo (`_MSG_MessageChat`, `_MSG_InviteGuild`, `_MSG_War`, etc.). |
| `server/tmsrv/handlers/world/` | `TMSrv` | Quests, cidade, teleporte, ranking e visibilidade (`_MSG_Quest`, `_MSG_ReqTeleport`, etc.). |
| `server/tmsrv/resources/` | `TMSrv` | Script de recursos Win32 (`TMSrv.rc`, `resource.h`, `icon1.ico`). |
| `server/tools/client_patch/` | `ClientPatch_v7662` (SHARED, opcional) | DLL legada de patch em memória para executável v7662 (`W2PP_BUILD_CLIENT_PATCH=OFF` por padrão). |

### 2.2 Cliente — `client/` (TMProject)

| Diretório | Target CMake | Responsabilidade |
| --- | --- | --- |
| `client/third_party/directx/` | `tm_directx` (INTERFACE) | Headers e `.lib`s do DirectX 9 SDK (anexados após o Windows SDK via `tm_directx.props` para evitar conflito com `basetsd.h`/`multimon.h`). |
| `client/src/core/` | `TMProject` (WIN32) | Ciclo da aplicação (`NewApp`, `TMProject`), PCH (`pch.h`), globais (`TMGlobal`, `TMPaths`), utilitários/log/timer/input/stacktrace. |
| `client/src/common/` | `TMProject` | Protocolo, definições base (`Basedef`, `ItemEffect`, `Structures`, `Enums`, `NewClientB`) e trava de layout ABI (`ProtocolLayoutCheck.cpp`). |
| `client/src/net/` | `TMProject` | Cliente de socket e enfileiramento de pacotes (`CPSock`). |
| `client/src/crypto/` | `tm_crypto` (INTERFACE) | Primitivas criptográficas header-only (`AES`, `Base64`, `desbase64`, `md5`, `MD5Checksum`). |
| `client/src/render/` | `TMProject` | Dispositivo Direct3D9 (`D3DDevice`, `RenderDevice`), texturas, malhas/animações, fontes, câmera e luz. |
| `client/src/audio/` | `TMProject` | Áudio DirectSound e reprodução de vídeo DirectShow (`dsutil`, `DirShow`, `TMVideoWnd`). |
| `client/src/ui/` | `TMProject` | Controles de interface (`SControl`, `SControlContainer`, `SGrid`, `ResourceControl.h`, `UIBinary.h`). |
| `client/src/scene/` | `TMProject` | Árvore de cena, gerenciador de objetos e cenas (`TMScene`, `TMDemoScene`, `TMSelectServerScene`, `TMSelectCharScene`, `TMFieldScene`). |
| `client/src/world/` | `TMProject` | Terreno, água, céu, clima e estruturas estáticas (`TMGround`, `TMSea`, `TMSky`, `TMHouse`, `TMGate`, etc.). |
| `client/src/entities/` | `TMProject` | Personagens, itens no chão, projéteis e criaturas (`TMHuman`, `TMItem`, `TMDrop`, `TMBike`, etc.). |
| `client/src/skills/` | `TMProject` | Efeitos visuais e comportamento das 24 classes `TMSkill*`. |
| `client/src/effects/` | `TMProject` | Partículas, billboards e efeitos visuais `TMEffect*` e `TMFireEffect`. |
| `client/src/gameplay/` | `TMProject` | Missões, composição de itens e tabelas de quest (`Mission`, `MrItemMix`, `Quest.h`). |
| `client/src/resources/` | `TMProject` | Recursos Win32 (`TMProject.rc`, `Resource.h`, ícones e cursores). |
| `client/docs/` | — | Notas históricas do cliente (`Files Name.txt`). |

| [`tools/check_boundaries.py`](../tools/check_boundaries.py) | — | Verificador automático de fronteiras de dependência entre módulos. |
| [`tests/`](../tests/CMakeLists.txt) | `w2pp_common_tests` (CTest) | Suíte de testes unitários para funções puras e layout binário de `w2pp_common`. |

---

## 3. Dependências e Fronteiras

```mermaid
flowchart TD
    subgraph Server ["server/ (W2PP)"]
        w2pp_common["server/common (w2pp_common)"]
        dbsrv["server/dbsrv (DBSrv)"]
        tmsrv["server/tmsrv (TMSrv)"]
        patch["server/tools/client_patch (opcional)"]
        dbsrv --> w2pp_common
        tmsrv --> w2pp_common
    end

    subgraph Client ["client/ (TMProject)"]
        tm_crypto["client/src/crypto (tm_crypto)"]
        tm_common["client/src/common"]
        tm_core["client/src/core + net"]
        tm_engine["render / audio / ui / scene / world / entities / skills / effects / gameplay"]
        tm_dx["client/third_party/directx (tm_directx)"]
        tm_engine --> tm_core
        tm_engine --> tm_common
        tm_engine --> tm_dx
        tm_core --> tm_common
        tm_core --> tm_crypto
    end
```

O script [`tools/check_boundaries.py`](../tools/check_boundaries.py) é executado localmente e no job `check-boundaries` do CI para impedir regressões arquiteturais:
1. Proibido `#include` com barra invertida (`\`) ou caminho relativo com `..` em `server/` e `client/src/`.
2. `server/common/` não pode incluir headers de `dbsrv/`, `tmsrv/` ou `tools/`.
3. `server/dbsrv/` e `server/tmsrv/` não podem incluir headers um do outro.
4. `client/src/crypto/` não pode depender de nenhum outro módulo de `client/src/`.
5. `client/src/common/` não pode incluir headers de camadas superiores (`render/`, `audio/`, `ui/`, `scene/`, `world/`, `entities/`, `skills/`, `effects/`, `gameplay/`).

---

## 4. Build e CI (Windows Win32 / MSVC)

### Pré-requisitos
- Windows 10/11 com Visual Studio 2022 (toolchain C++ Desktop x86) e CMake >= 3.24.
- Arquitetura-alvo obrigatória: **Win32 (`x86`)**, compatível com as bibliotecas estáticas do DirectX 9 em `client/third_party/directx/Lib/`.

### Comandos de Build

```powershell
# Configurar via preset (VS 2022 Win32)
cmake --preset win32-msvc

# Ou com autodetecção da versão do Visual Studio instalada + ClientPatch opcional
cmake -S . -B build/win32-msvc -A Win32 -DW2PP_BUILD_CLIENT_PATCH=ON

# Compilar Debug ou Release
cmake --build build/win32-msvc --config Debug --parallel
cmake --build build/win32-msvc --config Release --parallel

# Executar testes unitários via CTest
ctest --test-dir build/win32-msvc -C Debug --output-on-failure
```

Opções CMake disponíveis na raiz:
- `DERUST_BUILD_SERVER` (`ON` por padrão)
- `DERUST_BUILD_CLIENT` (`ON` por padrão)
- `DERUST_BUILD_TESTS` (`ON` por padrão)
- `W2PP_BUILD_CLIENT_PATCH` (`OFF` por padrão)

No GitHub Actions ([`.github/workflows/build.yml`](../.github/workflows/build.yml)):
- `check-boundaries` (`ubuntu-latest`): executa `python3 tools/check_boundaries.py`.
- `build-windows` (`windows-latest`, matriz `Debug` e `Release`): compila `w2pp_common`, `DBSrv`, `TMSrv`, `ClientPatch_v7662`, `TMProject` e `w2pp_common_tests`, executa `ctest` e publica os binários executáveis/DLLs como artifacts.

---

## 5. Travas de ABI e Protocolo

Os arquivos [`server/common/ProtocolLayoutCheck.cpp`](../server/common/ProtocolLayoutCheck.cpp) e [`client/src/common/ProtocolLayoutCheck.cpp`](../client/src/common/ProtocolLayoutCheck.cpp) validam em tempo de compilação (`static_assert`) o tamanho binário exato em Win32 das estruturas críticas de rede e persistência:

- **Servidor (`w2pp_common`)**: `MSG_STANDARD` (12 B), `STRUCT_ITEM` (8 B), `STRUCT_SCORE` (48 B), `STRUCT_MOB` (816 B), `STRUCT_MOBEXTRA` (536 B), `STRUCT_RANKING` (52 B), `STRUCT_ACCOUNTFILE` (7800 B).
- **Cliente (`TMProject`)**: `MSG_STANDARD` (12 B), `STRUCT_ITEM` (8 B), `STRUCT_SCORE` (48 B), `STRUCT_MOB` (1024 B).

Qualquer refatoração futura que altere alinhamento ou tamanho desses tipos quebra o build imediatamente.

---

## 6. Progresso e Roteiro da Fase 2 (Quebra de God-Files e Comentários)

Na **Fase 2**, os arquivos monolíticos são fatiados por coesão (preservando 100% de paridade de comportamento), acompanhados de [`.clang-format`](../.clang-format) e documentação em PT-BR:

### Onda 1 (concluída na branch `refactor/modularize-phase2`)
- **`ItemEffect.h` (`server/common/` e `client/src/common/`)**: comentários coreanos/mojibake convertidos para PT-BR preservando 100% das constantes `EF_*`.
- **`server/common/Basedef.cpp` (6.814 → 2.844 linhas)**: dividido em `Basedef.cpp`, [`BaseItem.cpp`](../server/common/BaseItem.cpp) (1.649L) e [`BaseCombat.cpp`](../server/common/BaseCombat.cpp) (2.436L), com [`BasedefInternal.h`](../server/common/BasedefInternal.h) e documentação Doxygen em [`server/common/Basedef.h`](../server/common/Basedef.h).
- **`server/tmsrv/core/Server.cpp` (9.449 → 2.912 linhas)**: extraídos 5 sistemas em `server/tmsrv/systems/`: `MountSystem.cpp` (167L), `AffectSystem.cpp` (1.204L), `WorldEventSystem.cpp` (2.251L), `ItemSpawnSystem.cpp` (1.811L) e `MobSpawnSystem.cpp` (1.377L).
- **`client/src/scene/TMFieldScene.cpp` (24.476 → 5.045 linhas)**: dividido em `TMFieldScene.cpp`, `TMFieldScene_UI.cpp` (7.753L), `TMFieldScene_Packets.cpp` (5.128L), `TMFieldScene_Combat.cpp` (4.431L) e `TMFieldScene_Input.cpp` (2.368L).
- **`client/src/entities/TMHuman.cpp` (16.323 → 5.152 linhas)**: dividido em `TMHuman.cpp`, `TMHuman_Effects.cpp` (3.147L), `TMHuman_Animation.cpp` (3.126L), `TMHuman_Packets.cpp` (3.021L) e `TMHuman_Combat.cpp` (2.125L).

### Onda 2 (concluída na branch `refactor/modularize-phase2-wave2`)
- **Servidor — `_MSG_UseItem.cpp` (5.726 → 90 linhas)**: dividido via [`UseItemInternal.h`](../server/tmsrv/handlers/items/UseItemInternal.h) em 5 sub-handlers coesos por faixa de `EF_VOLATILE`: `UseItem_Refine.cpp` (1.231L), `UseItem_Scrolls.cpp` (962L), `UseItem_Quests.cpp` (1.461L), `UseItem_Mithril.cpp` (1.182L) e `UseItem_Extractions.cpp` (955L).
- **Servidor — `_MSG_Quest.cpp` (2.712 → 181 linhas)**: dividido via [`QuestInternal.h`](../server/tmsrv/handlers/world/QuestInternal.h) em `Quest_Progression.cpp` (913L), `Quest_Kingdom.cpp` (1.117L) e `Quest_Special.cpp` (610L).
- **Servidor — `MobKilled.cpp` (2.469 → 239 linhas)**: dividido via [`MobKilledInternal.h`](../server/tmsrv/systems/MobKilledInternal.h) em `MobKilled_Exp.cpp` (446L), `MobKilled_Drops.cpp` (1.417L) e `MobKilled_PvP.cpp` (434L).
- **Servidor — `CFileDB.cpp` (2.688 → 831 linhas)**: despacho de pacotes em `CFileDB::ProcessMessage` extraído em `CFileDB_AccountPackets.cpp` (1.507L) e `CFileDB_WorldPackets.cpp` (484L).
- **Cliente — `SGrid.cpp` (5.332 → 986 linhas)**: dividido em `SGrid.cpp`, `SGrid_Trade.cpp` (1.457L), `SGrid_Input.cpp` (1.137L) e `SGrid_Tooltip.cpp` (1.792L), com documentação Doxygen em [`SGrid.h`](../client/src/ui/SGrid.h).
- **Cliente — `TMGround.cpp` (4.502 → 1.420 linhas)**: dividido em `TMGround.cpp`, `TMGround_Tables.cpp` (2.242L) e `TMGround_Render.cpp` (872L), com documentação Doxygen em [`TMGround.h`](../client/src/world/TMGround.h).
- **Testes automatizados (CTest)**: adicionada a suíte [`tests/w2pp_common_tests.cpp`](../tests/w2pp_common_tests.cpp) executada automaticamente pelo CI Windows em `Debug` e `Release`.

### Onda 3 (concluída na branch `refactor/modularize-phase2-wave3`)
- **Servidor — `dbsrv/core/Server.cpp` (2.343 → 1.131 linhas)**: funções inteiras movidas para `Server_Admin.cpp` (918L, protocolo administrativo e bloqueio de contas), `Server_Config.cpp` (170L, configuração e transferência de guildas) e `Server_Logs.cpp` (267L, logs e relatórios diários).
- **Servidor — `tmsrv/core/ProcessSecMinTimer.cpp` (2.315 → 946 linhas)**: os 4 maiores blocos de `ProcessSecTimer()` viraram funções declaradas em [`SecTimerInternal.h`](../server/tmsrv/systems/SecTimerInternal.h): `ScheduledEventSystem.cpp` (787L: `ProcessRuneQuestPortals`, `ProcessScheduledWorldEvents`) e `MobAISystem.cpp` (726L: `ProcessNpcMovementTick`, `ProcessMobCombatTick`). Os blocos não têm `return`, `break`/`continue` escapando nem `goto` entre blocos; as antigas variáveis locais (`bb`, `initial`, `Sec4`, `Sec16`) viraram parâmetros de mesmo nome.
- **Cliente — `render/RenderDevice.cpp` (3.486 → 912 linhas)**: `RenderDevice_Shaders.cpp` (1.053L), `RenderDevice_Rect.cpp` (908L) e `RenderDevice_Geom.cpp` (678L).
- **Cliente — `common/Basedef.cpp` (3.388 → 638 linhas)**: `BaseItem.cpp` (1.590L), `BaseCombat.cpp` (444L) e `BaseWorld.cpp` (751L), espelhando a divisão de `server/common/`.
- **Cliente — `ui/SControl.cpp` (2.994 → 630 linhas)**: `SControl_Text.cpp` (824L), `SControl_Buttons.cpp` (684L) e `SControl_List.cpp` (895L).
- **Cliente — `scene/TMScene.cpp` (2.699 → 822 linhas)**: `TMScene_Resources.cpp` (800L), `TMScene_Packets.cpp` (484L) e `TMScene_Ground.cpp` (673L).
- Os headers [`RenderDevice.h`](../client/src/render/RenderDevice.h), [`client/src/common/Basedef.h`](../client/src/common/Basedef.h), [`SControl.h`](../client/src/ui/SControl.h) e [`TMScene.h`](../client/src/scene/TMScene.h) documentam (Doxygen, PT-BR) onde cada parte da implementação vive.

### Onda 4 (concluída na branch `refactor/modularize-phase2-wave4`)
Segunda passada nas TUs que ainda passavam de ~2,5 mil linhas, isolando as funções gigantes em TUs próprias (definições inteiras movidas sem alteração, bytes preservados):
- **Servidor — `tmsrv/core/Server.cpp` (2.913 → 1.423 linhas)**: `Server_Window.cpp` (951L: `WinMain`, `MainWndProc`, `DrawConfig`), `Server_Config.cpp` (477L: `ReadConfig`, `ReadLevelItemConfig`) e `Server_Logs.cpp` (198L).
- **Servidor — `server/common/Basedef.cpp` (2.845 → 1.504 linhas)**: `BaseLoaders.cpp` (1.382L) com os 20 carregadores/gravadores de tabelas (itens, skills, mensagens, nomes, itens iniciais, guildas, servidores, taxa de acerto, atributos de mapa).
- **Cliente — `TMFieldScene_UI.cpp` (7.754 → 3.574 linhas, agora só `OnControlEvent`)**: `TMFieldScene_Panels.cpp` (1.940L, `SetVisible*`), `TMFieldScene_HUD.cpp` (1.695L, `UpdateScoreUI`/chat/câmera) e `TMFieldScene_MsgBox.cpp` (726L, `OnMsgBoxEvent`/`Guildmark_*`).
- **Cliente — `TMFieldScene_Packets.cpp` (5.129 → 1.850 linhas)**: `TMFieldScene_PacketAttack.cpp` (2.333L, `OnPacketAttack`) e `TMFieldScene_PacketItems.cpp` (1.065L, itens e lojas).
- **Cliente — `TMFieldScene.cpp` (5.046 → 1.140 linhas)**: `TMFieldScene_Init.cpp` (2.098L, construtor/`InitializeScene`) e `TMFieldScene_FrameMove.cpp` (1.927L).
- **Cliente — `TMFieldScene_Combat.cpp` (4.432 → 2.582 linhas)**: `TMFieldScene_AutoSkill.cpp` (1.159L) e `TMFieldScene_Items.cpp` (809L, poções/itens).
- **Cliente — `TMHuman.cpp` (5.153 → 2.101 linhas)**: `TMHuman_FrameMove.cpp` (1.245L) e `TMHuman_Render.cpp` (1.927L, `Render`/rótulos).
- **Codificação**: 7 comentários CP1252 de `TMFieldScene_PacketAttack.cpp` convertidos para UTF-8. Os únicos bytes não-UTF-8 restantes (`DirShow.cpp`, `NewApp.cpp`, `TMFieldScene_Input.cpp`) estão em literais de string/char (tabelas CP949 de IME) e são mantidos de propósito, pois o build MBCS depende deles.

### Próxima fase (Fase 3): decomposição das funções gigantes
Depois da Onda 4, o tamanho restante está concentrado em poucas funções, e dividir arquivos deixa de ajudar:

| Função | Linhas |
|---|---|
| `TMFieldScene::OnControlEvent` | ~3.500 → ~680 (71 handlers `OnControl_*` em `TMFieldScene_Controls.cpp` + `OnControl_SysServer`) |
| `TMFieldScene::OnPacketAttack` | ~2.270 → ~70 (`PacketAttack_Attacker` ~1.230, `PacketAttack_ApplyTargets` ~990) |
| `TMFieldScene::SkillUse` | ~1.570 → ~430 (`SendAreaAttack`, `SkillUse_CastleWarCheck`, `SkillUse_CastSelfOrArea` ~950, `SkillUse_ApproachTarget`) |
| `TMFieldScene::FrameMove` | ~1.510 → ~475 (9 etapas `FrameMove_*` extraídas) |
| `TMFieldScene::InitializeScene` | ~1.500 (sequência plana, sem blocos grandes) |
| `TMFieldScene::AutoSkillUse` | ~1.100 → ~340 (`AutoSkillUse_CastSelfOrArea` ~640, `AutoSkillUse_PKCheck`) |
| `TMHuman::InitObject`, `TMHuman::FrameMove` | ~1.000 cada |

Decompô-las exige reescrever fluxo de controle (`return` dentro de blocos que podem cair para o próximo `if`, locais compartilhadas), o que deixa de ser movimentação de código. Rede de segurança (Fase 3, passos 1 e 2 — concluídos na branch `refactor/phase3-mingw-ci`):
1. **Job `mingw-check` no CI** (`ubuntu-24.04`): [`tools/mingw_check.py`](../tools/mingw_check.py) compila todas as 256 TUs do servidor, do cliente e dos testes com mingw-w64 i686, linka o DBSrv e roda `w2pp_common_tests` no wine. Só as TUs listadas em [`tools/mingw_known_failures.txt`](../tools/mingw_known_failures.txt) podem falhar; uma falha nova quebra o job e uma TU consertada gera aviso para sair da lista.
2. **Construções exclusivas do MSVC removidas**: `goto` que saltava `int x = expr;` (a declaração foi separada da atribuição, o que preserva o comportamento), `#pragma region` entre `}` e `else` ou dentro de inicializadores (virou comentário `// region`), `sizeof TIPO` sem parênteses e `std::exception(const char*)` (trocado por `std::runtime_error`). Hoje restam 3 TUs na lista, todas por APIs do DirectX SDK legado (`D3DXMATRIXA16`, `D3D9b_SDK_VERSION` e o `ID3DXSprite` antigo).
3. **Decomposição de `OnControlEvent` (primeira leva, branch `refactor/phase3-oncontrolevent`)**: 28 blocos com 30+ linhas viraram métodos `OnControl_*` em `TMFieldScene_Controls.cpp`, um por commit. A função caiu de ~3.500 para ~1.560 linhas. Só entraram blocos que:
   - terminam sempre em `return` no nível superior do corpo (não caem para o próximo `if`);
   - não têm `else` acoplado nem `goto`/rótulos;
   - leem apenas as locais `dwServerTime`, `pMobData` e `cktrans`, constantes depois do prólogo, que são passadas por valor com o mesmo nome.

   O ponto de chamada mantém a condição original e vira `return OnControl_X(...)`. Cada extração foi conferida reinserindo o corpo no lugar (texto idêntico), e cada commit intermediário compila no mingw.
4. **Segunda leva (branch `refactor/phase3-giant-functions`)**:
   - Mais 43 handlers de `OnControlEvent` (blocos de 12+ linhas, mesmo critério). A função fica com ~860 linhas.
   - Extração `void` de statements de nível 1 sem `return` nem `goto` em `FrameMove` (8 etapas `FrameMove_*`: painel, minimapa, guerra de castelo, timers de afeto, parada de movimento, clima/sons, barras de skill, tempo de ranking) e em `SkillUse` (`SendAreaAttack`). Como o bloco é filho direto do corpo da função, todo `break`/`continue` dentro dele mira um laço interno. As locais e os parâmetros que ele usa passam **por referência**: é o mesmo objeto, então as escritas continuam visíveis depois da chamada. Cada extração foi conferida reinserindo o corpo no lugar, e a lista de parâmetros foi cruzada com todas as declarações de nível 1 anteriores ao bloco.
   - `AutoSkillUse` tinha uma cópia idêntica do bloco de ataque em área e passou a chamar `SendAreaAttack`.
5. **Terceira leva (branch `refactor/phase3-early-return`)**: 9 blocos com `return` no meio de um fluxo que cai adiante viraram `bool Nome(..., int& nReturnValue)`. A transformação é mecânica:
   - cada `return X;` do bloco vira `{ nReturnValue = X; return true; }`. X é avaliado no mesmo ponto e com os mesmos objetos, e a conversão para `int` é a mesma;
   - o fim do bloco ganha `return false;`, que significa "não retornou, continue";
   - o ponto de chamada vira `{ int nReturnValue = 0; if (Nome(...)) return nReturnValue; }`.

   Ficam de fora blocos com `goto`, lambda (cujo `return` não é da função) ou pré-processador. A verificação desfaz a reescrita e reinsere o corpo (texto idêntico), e cruza os parâmetros com todas as declarações de nível 1 anteriores ao bloco. Com isso, `OnPacketAttack`, `SkillUse` e `AutoSkillUse` passam a ler como uma sequência de etapas nomeadas.
6. **Verificação sem rodar o jogo (branch `refactor/phase3-verification`)**. Como o repositório não tem os arquivos do jogo, a rede de segurança ganhou três peças:
   - **Tag `pre-phase3-decomposition`** (`df54ba9`), o último commit antes da decomposição. Como cada extração está num commit próprio que compila, um `git bisect` entre essa tag e a `main` acha o commit responsável quando o teste no jogo for possível.
   - **`w2pp_characterization_tests`**: ~40 funções `BASE_*` (habilidades de item, sanc, códigos visuais, `CanEquip`/`CanCarry`/`CanCargo`, pontuações, `GetCurrentScore`, fórmulas de combate, mapa, strings) recebem milhares de entradas de um LCG próprio. As tabelas `g_pItemList`/`g_pSpell` são preenchidas com dados sintéticos. O hash das saídas foi gerado compilando o mesmo teste contra `server/Code/Basedef.cpp` da tag `pre-modularization` e confere com o código atual no mingw (x87 e SSE2, `-O0` a `-O2`) e no MSVC Debug/Release.
   - **Avisos do GCC com linha de base**: `-Wshadow`, `-W(maybe-)uninitialized`, `-Warray-bounds`, `-Wstringop-overflow`, `-Wsizeof-pointer-memaccess` etc., contados por arquivo. Um aviso novo reprova o job. O conjunto de `-Wshadow` nos arquivos da Fase 3 é idêntico ao da tag `pre-phase3-decomposition`, ou seja, as extrações não introduziram ocultação de nomes.

   - **`tmproject_characterization_tests`** (branch `refactor/phase3-client-characterization`): o mesmo método para ~55 funções `BASE_*` do cliente (habilidades e cores de item, `CanEquip`/`CanEquip_RecvRes`, inventário, baú e troca, fórmulas de combate, rota e posição de acerto no mapa de alturas, atributos do mapa, strings e lista de palavrões). Compila só `client/src/common/Base*.cpp` com as globais de `TMGlobal.cpp`/`TMLog.cpp`, sem o restante do cliente. Os hashes foram gerados contra `client/Projects/TMProject/Basedef.cpp` da tag `pre-modularization` (antes da divisão em quatro arquivos) e conferem com o código atual, com `-O0`/x87 e `-O2`/SSE2.

   Os comportamentos suspeitos encontrados (leituras fora dos limites, grade do baú conferindo a célula errada) estão em [`BUGS_LATENTES.md`](BUGS_LATENTES.md). Foram mantidos por paridade e depois corrigidos, cada um com teste: a grade do baú do servidor na branch `fix/cargo-carry-grid-stride`, e as leituras fora dos limites que só disparam com dado inválido na branch `fix/latent-out-of-bounds`, sem mudar nenhum hash de caracterização.
7. **Teste de partida dos servidores (branch `test/server-boot-smoke`)**: [`tools/server_boot_smoke.py`](../tools/server_boot_smoke.py), passo próprio no job `mingw-check`. Monta uma árvore mínima gerada pelo script:
   - `serverlist.txt` com os IPs do host;
   - `itemlist.csv` com três itens;
   - listas vazias e mapas zerados;
   - os nomes de `BaseSummon` tirados do próprio `CNPCGene.cpp`;
   - o `server/common/ItemEffect.h` do repositório;
   - o `gameconfig.txt` padrão que o TMSrv grava ([`tests/data/server_boot/`](../tests/data/server_boot/)).

   Sobe o DBSrv e o TMSrv num prefixo do wine e num Xvfb próprios (job `mingw-check`) e, nativos, os binários MSVC Debug e Release (job `build-windows`, branch `test/server-boot-windows`). No Windows, as MessageBox (e as asserções do CRT de Debug) são lidas das janelas de diálogo do processo, e `SEM_NOGPFAULTERRORBOX` faz uma exceção encerrar o processo em vez de abrir o WER; os servidores saem por `WM_QUIT`, porque o `Log()` não faz `fflush` e matar o processo perderia o log. Antes de cada partida, um autoteste sobe o DBSrv sem o `SkillData.csv` e exige que a caixa seja vista. Falha em qualquer MessageBox (capturada pelo canal `relay`), exceção não tratada (o wine abre o depurador e o processo fica pendurado, então "vivo" não basta), porta que não abre, queda da conexão TMSrv–DBSrv ou falta de `sys,start server` no log. Na primeira execução, achou um crash do DBSrv no accept da porta admin (ver [`BUGS_LATENTES.md`](BUGS_LATENTES.md)).
8. **Quarta leva, decomposição recursiva (branch `refactor/phase3-giant-functions-2`)**: 59 métodos novos, um commit por extração.

   | Função | Antes | Depois |
   |---|---|---|
   | `PacketAttack_Attacker` | 1.231 | 575 |
   | `PacketAttack_ApplyTargets` | 985 | 281 |
   | `SkillUse_CastSelfOrArea` | 953 | 405 |
   | `AutoSkillUse_CastSelfOrArea` | 642 | 295 |
   | `TMHuman::InitObject` | 1.027 | 260 |
   | `TMHuman::FrameMove` | 945 | 291 |

   As quatro etapas da cena eram um único `if` envolvendo o corpo inteiro, então a extração desce pelos blocos aninhados. Um bloco filho de 25 a 250 linhas vira método; um bloco maior fica no lugar e os filhos dele são extraídos. O plano é calculado uma vez sobre o código original, para que nenhum candidato contenha outro. As regras novas em relação às levas anteriores:
   - passam por referência as locais de todos os blocos que envolvem o candidato, não só as de nível 1, e as variáveis de cabeçalho de `for`. Locais `auto` só cruzam a fronteira quando o tipo é inferível (`new T`, `(T*)`, `static_cast<T*>`, `GetHumanByID`);
   - `break`/`continue` têm de mirar um laço ou `switch` interno ao candidato;
   - dentro de uma etapa `bool ...(..., int& nReturnValue)`, um sub-bloco com retorno mantém o mesmo protocolo (a chamada vira `if (Sub(...)) return true;`), sem reescrever nenhum `return`.

   Verificação: ida e volta em cada passo; a TU compila no mingw em cada commit; os cabeçalhos só ganharam declarações; toda linha removida reaparece entre as adicionadas; avisos idênticos (78). Contra a troca silenciosa de uma local por um membro ou global de mesmo nome: toda local que sombreia algo gera `-Wshadow`, e as 8 dessas funções são reconhecidas pelo analisador; qualquer outra local não reconhecida só causaria erro de compilação.
9. **Quinta leva, os blocos acima de 250 linhas (branch `refactor/phase3-skill-chain`)**: 25 commits, um por extração (e um que só escreve dois tipos que eram `auto`).

   | Função | Antes | Depois | Maior bloco antes | Maior bloco depois |
   |---|---|---|---|---|
   | `PacketAttack_Attacker` | 575 | 79 | 571 | 75 |
   | `PacketAttack_ApplyTargets` | 281 | 20 | 277 | 16 |
   | `SkillUse_CastSelfOrArea` | 405 | 186 | 401 | 182 |
   | `AutoSkillUse_CastSelfOrArea` | 295 | 61 | 291 | 57 |
   | `MobAttack` | 442 | 219 | 389 | 198 |

   - **Cadeia por skill**: em vez de uma tabela de handlers, o corpo de cada ramo com 12 linhas ou mais virou um método (`PacketAttack_Skill36Effect`, `PacketAttack_Skill34FairyEffect`…), e o ramo ficou com a mesma condição, no mesmo lugar, chamando o método entre chaves. Uma tabela por número de skill mudaria a ordem de avaliação: alguns ramos testam a classe e o rosto (classe 62, classe 56) ou a animação (`Motion` 254) antes do número da skill. Os ramos curtos e o da skill 110 (que repassa o `return` de uma etapa anterior) ficaram no lugar. A cadeia, agora em `PacketAttack_AnimateAttacker`, caiu de 457 para 157 linhas.
   - **Repasse do protocolo**: um bloco que só retorna por `if (Etapa(..., nReturnValue)) return true;` passou a contar como dentro do protocolo, o que liberou os blocos de `PacketAttack_ApplyTargets`, `SkillUse_CastSelfOrArea` e `AutoSkillUse_CastSelfOrArea`. O método extraído devolve `true` nesse mesmo caminho, com o `nReturnValue` já preenchido pela etapa interna.
   - **`MobAttack`**, que ficou fora das levas anteriores, perdeu quatro blocos (`MobAttack_RejectTarget`, `MobAttack_WeaponSkill`, `MobAttack_Class3Rule`, `MobAttack_ApproachTarget`), com os `return` reescritos no protocolo `nReturnValue`.

   Verificação: a mesma das levas anteriores (ida e volta em cada passo, a TU compila no mingw em cada commit, avisos idênticos, 77). As únicas linhas removidas sem par entre as adicionadas são os dois `auto` e os `return 0`/`return 1` reescritos. Nenhum bloco das seis funções passa de 250 linhas; o que sobra nelas e em `SkillUse` (429 linhas) e `AutoSkillUse` (341) são sequências de statements curtos.
10. **Mapa de divergências servidor x cliente**: o servidor e o cliente vêm de versões diferentes do jogo, com cópias próprias das structs, dos opcodes e das regras `BASE_*`; o alvo escolhido é a versão do cliente. `tools/divergence_map.py` mede a diferença sem mudar código e gera [`DIVERGENCIAS.md`](DIVERGENCIAS.md): layout de cada struct via `sizeof`/`offsetof` no mingw (executado no wine), pacotes casados pelo valor do opcode e pela struct que cada lado envia ou lê, e as funções `BASE_*` em comum compiladas contra cada lado (`tools/divergence/base_compare.cpp`) com as mesmas tabelas sintéticas e as mesmas entradas. É a base para decidir a ordem do alinhamento: primeiro o protocolo (um cabeçalho de pacotes único, com `static_assert` de tamanho), depois as regras, função por função. O relatório cita `arquivo:linha`, então fica desatualizado com refatorações; regenere antes de usar.
11. **Login no layout do cliente (branch `refactor/login-client-protocol`)**: primeiro passo do alinhamento. A `STRUCT_MOB` do servidor passou ao layout do cliente (1024 bytes; os nomes do servidor ficaram, os offsets e larguras seguem o cliente, e os campos que encolheram, Magic e RegenHP/RegenMP, já eram limitados pelo servidor a 1 byte). Também passaram ao layout do cliente o `MSG_CNFClientCharacterLogin` (0x114, 1712 bytes), o `MSG_AccountLogin` (senha de 16, 120 bytes), o `MSG_DeleteCharacter`, o `MSG_CharacterLogin` e o `MSG_AccountSecure`, e `APP_VERSION` virou 1758. Não houve camada de tradução: não havia contas a preservar. Cada campo do login tem `static_assert` de offset e tamanho em `server/common/ProtocolLayoutCheck.cpp`, e o teste de partida faz o login de ponta a ponta com um cliente sintético montado com os fontes do cliente (`tools/login_client`). Os hashes de caracterização da `STRUCT_MOB` passaram a ser calculados campo a campo, para não dependerem do layout.
12. **Pacotes de jogo no layout do cliente (branch `refactor/packets-client-layout`)**: segundo passo. Chat, sussurro e trombeta passaram a ter o texto de 128 bytes do cliente (o cliente grava `String[126]`/`[127]`, e os pacotes curtos do servidor faziam ele ler o pacote seguinte do buffer). A party segue a `PARTY` do cliente (`STRUCT_PARTY`); os ataques (0x367/0x39D/0x39E) seguem o cliente campo a campo, o que corrige `SkillParm`/`FlagLocal` trocados e o `ReqMp` no offset errado; `MSG_UpdateItem`, `MSG_SetHpDam` (dano saturado em 2 bytes), `MSG_CreateMob` (cidadania no lugar do `Hold`), `MSG_UpdateScore`, `MSG_CreateItem`, `MSG_DecayItem`, `MSG_TradingItem`, `MSG_SetShortSkill` e `MSG_REQShopList` também. Cada campo tem `static_assert` em `ProtocolLayoutCheck.cpp`. O que sobra em [`DIVERGENCIAS.md`](DIVERGENCIAS.md) nessas tabelas foi conferido e vem anotado no relatório (um deles é bug do próprio cliente, no 0x363).
13. **Regras `BASE_*` do cliente no servidor (branch `refactor/base-rules-client`)**: terceiro passo. Vale a regra do cliente; onde o código do cliente tinha erro evidente da reconstrução, o cliente foi corrigido antes (`CanTrade` e `CanCargo` sempre aceitavam, `GetVillage` só comparava `x`, o dano das duas mãos de `GetMobAbility` era código morto, `GetItemAbilityNosanc` comparava o índice do item). As 29 funções em comum dão a mesma saída nos dois lados para todas as entradas de `tools/divergence/base_compare.cpp`. Mudanças de jogo: multiplicadores de refinação +11..+16 do cliente, velocidade até 7, fórmulas de dano de skill, crítico por `g_pHitRate`, dano de duas armas, equipar pelo rosto, troca pelas bolsas de `Carry[60]`/`Carry[61]`, rota, máscaras de chão dos portões (antes zeradas), nomes sem lista de reservados. `BASE_GetItemSanc` devolve +10..+16 como 10..16 e `REF_11..REF_15` passaram a 11..15. As funções `BASE_*` só de um lado (por exemplo, `BASE_GetCurrentScore` do servidor) não têm com o que comparar; as duas outras pendências deste item foram resolvidas no item 15.
14. **Structs aninhadas e dados de jogo no layout do cliente (branch `refactor/nested-structs-client`)**: o `tools/divergence_map.py` passou a comparar os campos dentro das structs aninhadas dos pacotes, o que mostrou duas diferenças que a comparação por campo de primeiro nível escondia. `STRUCT_AFFECT` tinha os mesmos bytes nos dois lados, mas com os nomes do meio trocados (o `Level` do servidor era o `Value` do cliente); o servidor passou aos nomes do cliente, uso a uso, guiado pelo compilador. `STRUCT_SCORE.Level` tinha 4 bytes no servidor e 2 no cliente; passou a 2. `STRUCT_SPELL` ganhou o layout de 96 bytes do cliente (`Act1`, `Act2`, `ForceDamage`), e o carregador do `SkillData.csv` guarda as duas colunas de movimento. O `SkillData.bin` passou a ter o mesmo formato no item 15. `ItemList.bin` tem o mesmo formato nos dois (6500 × 140 bytes). Correções que saíram junto: o teste do Remédio/Elixir da Coragem lia o campo de 1 byte e nunca deixava renovar o efeito, e o cliente tirava os movimentos de skill da posição errada num dos laços (precedência do `?:`).
15. **Pendências das regras, cabeçalho comum e triagem (branch `refactor/rules-pending-client`)**: o inventário inteiro segue as posições liberadas por bolsa do cliente (`BASE_CarryUsable` e `BASE_NextCarry` no lugar de `MaxCarry`, que saiu do `CMob`); o teste de "não refina" (126) olha só o efeito fixo do ItemList, porque num item 126 é a última tintura do cliente; `g_pSpell` tem as 248 entradas do cliente (`MAX_SPELL_LIST`), então o `SkillData.bin` é o mesmo arquivo, mas o combate continua aceitando só as 103 primeiras (`MAX_SKILLINDEX`), já que o servidor não tem código para as outras; e o módulo `shared/protocol` (biblioteca INTERFACE `w2_shared`) recebeu as structs idênticas. `STRUCT_AFFECT` ficou fora: os nomes e offsets batem, mas o sinal dos campos não, e o sinal muda contas de um lado ou do outro. A triagem dos pacotes de um lado só está em [`PACOTES_DE_UM_LADO.md`](PACOTES_DE_UM_LADO.md); dela saíram duas correções (repasse do 0xFAA, leitura do 0x2D4) e o reconhecimento de opcodes decimais no analisador. Em aberto: os recursos do cliente que o servidor não tem, a resposta da cápsula (0xDC3 contra 0xD1F, com dados que não se traduzem um a um) e o repasse do pedido de transferência.
16. **Pontos em aberto da triagem (branch `refactor/open-items-client`)**: o TMSrv repassa ao DBSrv o pedido de transferência de personagem (0xFAA); o placar do RvR (0x3BB) vai sem o +1024 de outro cliente; a resposta da cápsula passou a ser o 0xDC3 no layout do cliente, preenchido pelo DBSrv com rosto, nível, atributos e maestrias (as skills e a quest ficam 0, sem correspondência no personagem salvo); o log de erro do cliente (0x2BC) vai para o log do servidor; e `STRUCT_AFFECT` foi para `shared/protocol` com os tipos sem sinal do servidor, depois de conferir que nenhum uso do cliente muda para os valores que o servidor envia (o item 15 tinha concluído o contrário sem olhar os usos). Em aberto: os recursos do cliente que são desenho de jogo no servidor (lista em [`PACOTES_DE_UM_LADO.md`](PACOTES_DE_UM_LADO.md)).
17. **Teste de jogo de ponta a ponta (branch `test/gameplay-smoke`)**: o `LoginClient` passou a entrar no jogo com dois personagens e a conferir, no layout do cliente, criação de mob, chat, movimento, soltar e pegar item, troca de posição com bolsa, uso de poção e a volta do personagem com o inventário gravado pelo DBSrv. O teste achou três bugs do servidor, corrigidos: o movimento para uma casa ocupada devolvia `sizeof(m)` bytes (o ponteiro, 4 bytes), que o `CPSock` do cliente rejeita; a confirmação de pegar item (0x171) ia com o item zerado, e o cliente põe esse item no inventário e soma dele o ouro; e a troca de posição (0x376) só conferia a bolsa na casa onde o item é solto, embora leve o item de lá para a casa de origem. No 0x376, os nomes do cliente (`Sour*`, `Dest*`) são o contrário do que os campos fazem; o servidor está certo e o cabeçalho do cliente ganhou a nota. Fora do teste: combate, troca entre jogadores, lojas e NPCs, que pedem mobs no mundo sintético.
18. **Nomes do cliente e mais structs no cabeçalho comum (branch `refactor/client-names`)**: o servidor passou aos nomes de campo do cliente em 15 pacotes (`MSG_SendItem`, `MSG_AccountLogin`, `MSG_UpdateScore`, `MSG_UpdateEtc`, `MSG_CreateMob` e outros) e em `STRUCT_MOB`, `STRUCT_SELCHAR`, `STRUCT_ITEMLIST`, `STRUCT_INITITEM` e `STRUCT_BEASTBONUS`, com a troca guiada pelo compilador (cada erro "has no member" aponta um uso). Onde o nome do cliente era um marcador e o do servidor dizia o que o campo é, ficou o do servidor: `STRUCT_SCORE.Reserved` do cliente virou `Merchant` (tipo de comerciante e direção). `LearnedSkill` e `SecLearnedSkill` do servidor viraram `unsigned int LearnedSkill[2]`, como no cliente; as duas divisões por `MAX_USER` (NPCs) mantêm o sinal com um cast. Depois, `STRUCT_SCORE`, `STRUCT_MOB`, `STRUCT_SELCHAR`, `STRUCT_ITEMLIST` (com `STRUCT_STATICEFFECT`), `STRUCT_INITITEM` e `STRUCT_BEASTBONUS` foram para `shared/protocol/GameStructs.h` com os tipos do servidor, depois de conferir os usos do cliente em que o sinal pesa: para os valores que o servidor envia nenhum muda, e `Critical`/`SaveMana`, que o cliente já lia como `unsigned char` em quase todo uso, passam a ser lidos assim em todos. Fora: `STRUCT_ACCOUNTFILE` (o arquivo de conta tem outro formato em cada lado) e `STRUCT_GUILDZONE` (campos diferentes no meio). Os pacotes continuam em cada lado, porque o cabeçalho é `_MSG` no servidor e `MSG_STANDARD Header` no cliente.
19. **Opcodes comuns e regras com outro nome (branch `refactor/remaining-items`)**: `shared/protocol/Opcodes.h` reúne os 111 opcodes que o cliente envia ou lê e que têm nome em algum lado, com os nomes do cliente; o cliente deixou as suas definições e os literais nos `switch` e envios, e o servidor mantém os `_MSG_X` (com as flags de roteamento do DBSrv), conferidos um a um em `ProtocolLayoutCheck.cpp`. Os 23 opcodes que só o cliente usa e nunca nomeou seguem literais. Pacotes: o 0x3B4 vai como `MSG_STANDARDPARM2`, que o cliente lê; o 0x363 era erro da reconstrução do cliente, que tirava a cidadania do dono da loja de `Desc[0]`; o 0x10A fica como está, porque o layout maior só vale depois da troca de servidor (0x52A), que o servidor não implementa. Regras: a revisão das 52 funções `BASE_*` só do servidor achou três que o cliente decodifica de outro jeito, agora com a regra do cliente: a refinação da montaria nunca ia (condição `>= 2360 && < 2360`), as tinturas iam sempre como +12 com a cor errada para os outros jogadores, e a subguilda vinha da medalha em vez do `GuildLevel` (`BASE_GetSubGuild`, agora comparada pelo mapa). Em aberto, só do servidor: `BASE_CheckItemDate` compara o mês 0..11 com o mês gravado 1..12, `BASE_SetItemDate(…, 30)` grava o dia 32 no dia 31, e as faixas de itens com validade diferem das que o tooltip do cliente mostra. `BASE_IsInLowZone` do cliente (bit 0x80 do mapa) só interrompe a corrida automática, enquanto o servidor recolhe só acima de `FREEEXP`; não é regra de protocolo.
20. **Servidores no Linux (branch `port/linux-prep`)**: o DBSrv e o TMSrv compilam e rodam nativos no Linux x86_64, para a hospedagem. Primeiro, o que mudaria o comportamento: os campos do arquivo de conta com tamanho de plataforma (`long`, `time_t`) ganharam largura fixa, seis contas de ponteiro passaram a usar `char*` em vez de `int`, os caminhos usam `/` e um nome só para o `ExtraItem.bin`, e o `rand()` virou `W2Rand` (`common/Platform.h`), com o algoritmo do CRT da Microsoft, porque a glibc gera outra sequência. Depois, a camada `server/compat`, só no build Linux, emula o subconjunto Win32 que os servidores usam: o laço de mensagens com `poll()` entrega ao mesmo `MainWndProc` as mensagens de socket e de timer que o Windows entregaria, a `MessageBox` vai para o stderr e `SIGTERM` vira `WM_CLOSE`. As travas de `ProtocolLayoutCheck.cpp` valem também no Linux x86_64, e os testes de caracterização dão os mesmos hashes. O CI ganhou o build Release nativo e, no job mingw, o build com ASan/UBSan, que roda os testes e o teste de jogo com o `LoginClient.exe` no wine contra os servidores nativos. Os sanitizers acharam três casos, corrigidos: o `FindClose` de um handle inválido, a leitura desalinhada do cabeçalho em `CPSock::ReadMessage` e um deslocamento maior que 31 em `BASE_CanEquip`, que agora usa a máscara de 5 bits que o x86 aplicava. Ver [`LINUX.md`](LINUX.md).
21. **Comandos do menu no Linux (branch `port/linux-control`)**: o `AppendMenu` da camada `server/compat` registra os itens do menu de cada servidor, e um FIFO `control.fifo` (0600) no diretório do servidor entra no mesmo `poll()` do laço. Cada linha com o nome de um item vira o `WM_COMMAND` do clique: `saveall`, `readgameconfig`, `readguild`, `readguildname`, `reloadmob`, `reboot` e `exit` no TMSrv, e `reloadmobbase` e `exit` no DBSrv. O código do jogo não mudou. O teste de jogo nativo manda os comandos que não derrubam o servidor e confere o eco de cada um. Junto, o `WM_CREATE` passou a rodar dentro do `CreateWindow`, como no Windows, em vez de ir para a fila.
22. **Hospedagem no Linux (branch `port/linux-hosting`)**: `tools/linux_deploy.py` monta a árvore da produção (`Common`, `DBSRV/Run`, `TMSRV/Run`) com os diretórios em que os servidores gravam sem criar, confere a caixa dos arquivos de dados contra os nomes tirados das fontes (`--fix-case` renomeia) e gera as unidades `w2-dbsrv` e `w2-tmsrv` do systemd (`SIGTERM` para gravar e sair, `Restart=always`, `LimitNOFILE=8192`). O teste de jogo passou a usar essa mesma árvore, e o CI confere as unidades com `systemd-analyze verify`. `tools/linux_warning_check.py` compara os avisos do build nativo com uma linha de base por versão do gcc, com os mesmos tipos acompanhados do mingw. A primeira passada achou oito `sprintf` que escreviam no próprio destino, que é comportamento indefinido; agora eles anexam a partir do fim.
23. **Conversão dos dados do cliente (branch `tools/bin-csv`)**: `tools/w2_bin_csv.py` converte o `ItemList.bin` (com o `ExtraItem.bin` aplicado por cima, como faz o cliente) no `itemlist.csv` e o `SkillData.bin` no `SkillData.csv`, no layout do cliente (6500 × 140 e 248 × 96 bytes, XOR 0x5A). Um arquivo de outro tamanho, de outra versão do jogo, é recusado com o número de registros que ele teria. O script relê o csv que gravou imitando `BASE_ReadItemListFile` e `BASE_InitializeSkill` (o mesmo `sscanf`, o corte do nome, o `/4` do `AffectTime`) e lista o que o servidor leria diferente. O autoteste com dados sintéticos roda no CI. O teste de jogo traz itens com casos difíceis para o leitor (negativos, overflow de `short`, tokens cortados pelo `sscanf`, efeito inexistente, tab) e exige que o leitor imitado monte exatamente o `ItemList.bin` que o servidor grava, no build nativo, no mingw e no MSVC. Dessa conferência saiu o problema do rodapé do `ItemList.bin` (`docs/BUGS_LATENTES.md`).
24. **Dados do WYD Global (branch `tools/global-layout`)**: o cliente atual do WYD Global (patch 749) usa outro layout: `ItemList.bin` de 14500 × 172 bytes, `extraitem.bin` de 2 + 172 e `SkillData.bin` de 248 × 104. Os primeiros 132 bytes do item são os nossos. Depois vêm `nUnique` e `nPos` como `int` (o `nPos` usa os bits 16 e 17), `nExtra`, `nGrade` e 28 bytes sem equivalente; os slots de efeito vazios são (255, 0). A skill é a nossa com dois `int` a mais. O layout foi deduzido dos dados: as posições de equipamento, os preços e os efeitos conhecidos batem, e o último `int` do item é o próprio índice. `tools/w2_bin_csv.py` o reconhece pelo tamanho e converte para o layout do servidor, com um resumo do que não cabe. No patch 749 isso é o efeito 93, que não tem nome em nenhum dos `ItemEffect.h` e aparece com valor 1 em 1957 itens, os bits altos do `nPos` em 68 itens e o `ForceDamage` de 37 skills.
25. **Pendências do alinhamento (branch `refactor/finish-alignment`)**: as duas regras de data que só o servidor tem e que o item 19 deixou em aberto foram corrigidas. `BASE_SetItemDate` passou a somar os dias pelo `mktime` (antes, com 30 dias no dia 31, gravava o dia 32, e com menos de 30 dias o mês não avançava, o que dava uma data já vencida), e `BASE_CheckItemDate` compara a data inteira (antes, no caso "mês seguinte, dia menor", comparava o `tm_mon` de 0 a 11 com o mês gravado de 1 a 12, e o item só vencia um mês depois). As duas ganharam uma variante com a data de hoje como parâmetro (`BASE_SetItemDateAt`, `BASE_CheckItemDateAt`), testada com datas fixas em `w2pp_common_tests`. As faixas de itens com validade ficam como estão: o tooltip do cliente mostra a data de faixas maiores (3980–3999, 4150–4189, 6301–6400…), mas só quando o item tem os efeitos de data, e o servidor só grava a data nas faixas que ele mesmo faz vencer; ampliar a expiração faria vencer na hora os itens sem data. `STRUCT_GUILDZONE` e `STRUCT_POSITION` foram para `shared/protocol` com os nomes do cliente (os bytes e a tabela `g_pGuildZone` já eram iguais; os nomes antigos do servidor ficaram nos comentários). `STRUCT_ACCOUNTFILE` continua diferente de propósito: o cliente só a declara, sem uso, e o arquivo de conta é do DBSrv; o motivo ficou no mapa (`STRUCT_NOTES` em `tools/divergence_map.py`). Os 8 combinadores com opcode do `Mixlist.bin` foram conferidos no arquivo do WYD Global (patch 749): os opcodes do servidor estão todos lá, mas o registro do Global tem 2664 bytes contra 2656 no nosso cliente, então o `Mixlist.bin` da nossa versão continua sendo a prova que falta. Em aberto: a troca de servidor (0x52A) e os recursos do cliente que são desenho de jogo.

### Diretrizes de Comentários e Higiene (Fase 2)
- Idioma: **Português (PT-BR)** nos novos comentários e documentação.
- Interfaces e tipos: comentários **Doxygen (`///`)** em classes, enums, structs de pacote (opcode, direção, campos relevantes) e funções públicas.
- Comentários inline: explicar o *por quê* e regras de domínio do WYD (magic numbers, IDs de itens/skills/quests, fórmulas, bitmasks e efeitos colaterais).
- Código legado:
  - Preservar integralmente os cabeçalhos de licença **GNU GPL v3**.
  - Substituir comentários corrompidos/mojibake (como textos em coreano quebrado) por descrições claras em PT-BR ao revisar cada arquivo.
  - Preservar código comentado suspeito/relevante marcando com `// LEGADO:` em vez de removê-lo sem confirmação.
  - Código novo extraído na Fase 2 deve seguir C++17 limpo e UTF-8.
