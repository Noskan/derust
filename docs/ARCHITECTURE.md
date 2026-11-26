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

### Próximos alvos (Onda 4)
- **Cliente**: as TUs já fatiadas que seguem grandes — `TMFieldScene_UI.cpp` (~7,8 mil linhas), `TMHuman.cpp` (~5,2 mil), `TMFieldScene_Packets.cpp` (~5,1 mil), `TMFieldScene.cpp` (~5,0 mil) e `TMFieldScene_Combat.cpp` (~4,4 mil).
- **Servidor**: `tmsrv/core/Server.cpp` (~2,9 mil), `server/common/Basedef.cpp` (~2,8 mil) e `BaseCombat.cpp` (~2,4 mil), além da promoção gradual de subsistemas desacoplados a static libs CMake com testes adicionais.

### Diretrizes de Comentários e Higiene (Fase 2)
- Idioma: **Português (PT-BR)** nos novos comentários e documentação.
- Interfaces e tipos: comentários **Doxygen (`///`)** em classes, enums, structs de pacote (opcode, direção, campos relevantes) e funções públicas.
- Comentários inline: explicar o *por quê* e regras de domínio do WYD (magic numbers, IDs de itens/skills/quests, fórmulas, bitmasks e efeitos colaterais).
- Código legado:
  - Preservar integralmente os cabeçalhos de licença **GNU GPL v3**.
  - Substituir comentários corrompidos/mojibake (como textos em coreano quebrado) por descrições claras em PT-BR ao revisar cada arquivo.
  - Preservar código comentado suspeito/relevante marcando com `// LEGADO:` em vez de removê-lo sem confirmação.
  - Código novo extraído na Fase 2 deve seguir C++17 limpo e UTF-8.
