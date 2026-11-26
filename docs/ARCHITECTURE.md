# Arquitetura do Monorepo `derust`

Este documento descreve a organização modular de **W2PP** (`server/`) e **TMProject** (`client/`), estabelecida na **Fase 1** da reestruturação, além das regras de fronteira, travas de ABI e o roteiro da **Fase 2**.

## 1. Visão Geral

O monorepo reúne o servidor e o cliente C++ do *With Your Destiny (WYD)* sob um único sistema de build **CMake** (MSVC Win32/x86, C++17), preservando os históricos completos dos repositórios originais.

> [!IMPORTANT]
> Nesta fase, `server/` e `client/` permanecem **codebases independentes**, cada qual com seu próprio módulo `common/`. Embora compartilhem conceitos de protocolo, estruturas fundamentais divergem em layout binário (por exemplo, `STRUCT_MOB` tem `816` bytes no servidor e `1024` bytes no cliente). A extração de um módulo `shared/protocol` fica reservada para uma etapa futura após a consolidação e auditoria das revisões de pacote.

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
9. **Próximo passo**: o que sobra nas seis funções são sequências de statements curtos e os blocos acima de 250 linhas que ficaram no lugar (o maior, em `PacketAttack_Attacker`, é a cadeia `if`/`else if` por skill). Decompô-los exige outra técnica, por exemplo transformar a cadeia por skill numa tabela de handlers.

### Diretrizes de Comentários e Higiene (Fase 2)
- Idioma: **Português (PT-BR)** nos novos comentários e documentação.
- Interfaces e tipos: comentários **Doxygen (`///`)** em classes, enums, structs de pacote (opcode, direção, campos relevantes) e funções públicas.
- Comentários inline: explicar o *por quê* e regras de domínio do WYD (magic numbers, IDs de itens/skills/quests, fórmulas, bitmasks e efeitos colaterais).
- Código legado:
  - Preservar integralmente os cabeçalhos de licença **GNU GPL v3**.
  - Substituir comentários corrompidos/mojibake (como textos em coreano quebrado) por descrições claras em PT-BR ao revisar cada arquivo.
  - Preservar código comentado suspeito/relevante marcando com `// LEGADO:` em vez de removê-lo sem confirmação.
  - Código novo extraído na Fase 2 deve seguir C++17 limpo e UTF-8.
