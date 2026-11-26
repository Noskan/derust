# Bugs latentes conhecidos

Comportamentos do código legado encontrados pelos testes de caracterização
([`tests/w2pp_characterization_tests.cpp`](../tests/w2pp_characterization_tests.cpp)) e pela
checagem de avisos do GCC ([`tools/mingw_check.py`](../tools/mingw_check.py)).

**Nenhum foi corrigido.** A modularização mantém paridade de comportamento: todos já existiam
na tag `pre-modularization`. Corrigir qualquer um deles muda o comportamento do jogo, então cada
correção deve ser uma decisão explícita, num commit próprio, de preferência acompanhada de um
teste no jogo.

## Leitura fora dos limites (resultado depende do layout de memória)

| Onde | O que acontece | Quando dispara |
|---|---|---|
| `server/common/BaseItem.cpp:845`, `BASE_GetGrowthRate` | Lê `g_pSancRate[2][cEffect / 10]`, mas a tabela tem 12 colunas | Montaria (2360..2389) com `stEffect[1].cEffect` ≥ 120 |
| `server/common/BaseItem.cpp:1128` e `:1196`, `BASE_CanCarry`/`BASE_CanCargo` | Lê `g_pItemGrid[grid]` (8 entradas) com `grid = EF_GRID` do item, sem validar | Item com `EF_GRID` ≥ 8 (dado de item corrompido ou adulterado) |
| `server/common/BaseItem.cpp:1243`, `BASE_CanCargo` | A grade é gravada com `linha * CARGOGRIDX` (9), mas a conferência lê `linha * CARGOGRIDY` (14). Para linha ≥ 9 a leitura passa do fim de `CargoGrid[128]`, na pilha | Itens nas linhas de baixo do baú |
| `server/common/BaseCombat.cpp:1753`, `BASE_GetCurrentScore` | `mount <= 2390` aceita 2390, e `g_pMountBonus[mount - 2360]` tem 30 linhas | Item 2390 no slot de montaria |

O GCC (`-Warray-bounds`) aponta ainda, no cliente:

| Onde | Aviso |
|---|---|
| `client/src/entities/TMHuman.cpp:1145..1164` | Índice 8 em `TMEffectBillBoard* [7]` |
| `client/src/entities/TMItem.cpp:811` | Índice -2241 em `const unsigned int [3]` |
| `client/src/render/MeshManager.cpp:329` | Índice 7 em `short [6][60][56]` |
| `client/src/scene/TMFieldScene_Controls.cpp:571` | Índice 70 em `char [16]`. Já existia em `TMFieldScene_UI.cpp` antes da Fase 3 |
| `server/tmsrv/systems/ScheduledEventSystem.cpp:668` | Índice 3 em `STRUCT_PISTA::<struct> [3]` |

## Grade com a célula errada ou memória não inicializada

| Onde | O que acontece |
|---|---|
| `server/common/BaseItem.cpp:1179`, `BASE_CanCarry` | Mesma troca: grava com `linha * CARRYGRIDX` (9) e confere com `linha * CARRYGRIDY` (7). A leitura fica dentro do buffer, mas olha a célula errada |
| `server/common/BaseItem.cpp:1134` e `:1202` | `memset(CarryGrid, 0, MAX_CARRY-1)` e `memset(CargoGrid, 0, MAX_CARGO-2)` deixam 1 e 2 bytes finais com lixo da pilha |
| `server/tmsrv/handlers/items/_MSG_Buy.cpp:273..307` | O GCC aponta `GuildTax`, `Price`, `Price2`, `CityTax`, `bPrice` e `Desconto` possivelmente não inicializados nos caminhos `goto LABEL_BUY1`/`goto DonateBuy`. É o mesmo comportamento que o MSVC já tinha antes da separação declaração/atribuição. Falta confirmar se as condições de guarda impedem o uso |
| `server/tmsrv/handlers/items/UseItem_Quests.cpp:514`, `server/tmsrv/entities/CMob.cpp:595`, `client/src/render/CFrame.cpp:311` | O GCC aponta `-Wmaybe-uninitialized` (`when.tm_min`/`tm_sec`, `iterator`, `vecOut3`) |

## Outros avisos relevantes

| Onde | Aviso |
|---|---|
| `server/common/Basedef.cpp:1355` | `strncpy(..., sizeof(origem))`: o tamanho é o da origem, não o do destino |
| `client/src/common/BaseItem.cpp:571`, `client/src/gameplay/MrItemMix.cpp:193` | `memset`/`memcpy` com `sizeof` do próprio ponteiro |
| `client/src/render/DXUtil.cpp:937`, `:965` | `delete` de `void*` (comportamento indefinido) |
| `server/tmsrv/net/GetFunc.cpp:1213`, `server/tmsrv/systems/ItemSpawnSystem.cpp:1307`, `:1806` | `-204` atribuído a `unsigned char` (vira 52) |
