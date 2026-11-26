# Bugs latentes conhecidos

Comportamentos do código legado encontrados pelos testes de caracterização
([`tests/w2pp_characterization_tests.cpp`](../tests/w2pp_characterization_tests.cpp) e
[`tests/tmproject_characterization_tests.cpp`](../tests/tmproject_characterization_tests.cpp)) e pela
checagem de avisos do GCC ([`tools/mingw_check.py`](../tools/mingw_check.py)).

A modularização mantém paridade de comportamento: todos já existiam na tag `pre-modularization`.
Corrigir qualquer um deles muda o comportamento do jogo, então cada correção é uma decisão
explícita, num commit próprio, com um teste que mostra o defeito. Os corrigidos estão no fim.

## Funções `BASE_*` e avisos de limites

As leituras fora dos limites das funções `BASE_*` encontradas pelos testes de caracterização
foram corrigidas (ver o fim do arquivo). Fica registrado, no cliente:

| Onde | O que acontece | Quando dispara |
|---|---|---|
| `client/src/common/BaseWorld.cpp`, `BASE_IsInLowZone` | Fora do mapa chama `MessageBox`/`PostMessage` com `g_pApp->m_hWnd` e fecha o cliente. É tratamento de erro, não leitura fora dos limites | Coordenada fora de 0..4095 |

Outros comportamentos do cliente que o teste fixa como estão (não leem fora dos limites):

- **`BASE_CanCargo`**: `SourGrid` nunca é preenchida, então a função sempre devolve 1. A grade do baú é gravada com `CargoGrid[ty][9 * yy + tx + xx]`, deslocada em relação à linha.
- **`BASE_CanTrade`**: `OpponentTrade` não é usado (copia de uma lista local vazia).
- **`BASE_ClearItem`**: `memset(item, 0, sizeof(item))` zera só o tamanho do ponteiro (4 bytes em Win32). O item de 8 bytes fica com `stEffect[1]` e `stEffect[2]` intactos.
- **`BASE_GetAttribute`**: a guarda confere `x <= 4096` duas vezes e nunca `y`. A máscara `& 1023` mantém o índice dentro da tabela.

O GCC (`-Warray-bounds`) aponta ainda, no cliente:

| Onde | Aviso |
|---|---|
| `client/src/entities/TMHuman.cpp:1145..1164` | Índice 8 em `TMEffectBillBoard* [7]` |
| `client/src/entities/TMItem.cpp:811` | Índice -2241 em `const unsigned int [3]` |
| `client/src/render/MeshManager.cpp:329` | Índice 7 em `short [6][60][56]` |
| `client/src/scene/TMFieldScene_Controls.cpp:571` | Índice 70 em `char [16]`. Já existia em `TMFieldScene_UI.cpp` antes da Fase 3 |
| `server/tmsrv/systems/ScheduledEventSystem.cpp:668` | Índice 3 em `STRUCT_PISTA::<struct> [3]` |

## Memória possivelmente não inicializada

| Onde | O que acontece |
|---|---|
| `server/tmsrv/handlers/items/_MSG_Buy.cpp:273..307` | O GCC aponta `GuildTax`, `Price`, `Price2`, `CityTax`, `bPrice` e `Desconto` possivelmente não inicializados nos caminhos `goto LABEL_BUY1`/`goto DonateBuy`. É o mesmo comportamento que o MSVC já tinha antes da separação declaração/atribuição. Falta confirmar se as condições de guarda impedem o uso |
| `server/tmsrv/handlers/items/UseItem_Quests.cpp:514`, `server/tmsrv/entities/CMob.cpp:595`, `client/src/render/CFrame.cpp:311` | O GCC aponta `-Wmaybe-uninitialized` (`when.tm_min`/`tm_sec`, `iterator`, `vecOut3`) |

## Outros avisos relevantes

| Onde | Aviso |
|---|---|
| `server/common/Basedef.cpp:1355` | `strncpy(..., sizeof(origem))`: o tamanho é o da origem, não o do destino |
| `client/src/common/BaseItem.cpp:571`, `client/src/gameplay/MrItemMix.cpp:193` | `memset`/`memcpy` com `sizeof` do próprio ponteiro |
| `client/src/render/DXUtil.cpp:937`, `:965` | `delete` de `void*` (comportamento indefinido) |
| `server/tmsrv/net/GetFunc.cpp:1213`, `server/tmsrv/systems/ItemSpawnSystem.cpp:1307`, `:1806` | `-204` atribuído a `unsigned char` (vira 52) |

## Corrigidos

| Onde | Defeito | Correção |
|---|---|---|
| `server/common/BaseItem.cpp`, `BASE_CanCarry` e `BASE_CanCargo` | A grade de ocupação era gravada com `linha * CARRYGRIDX`/`CARGOGRIDX` (9 colunas), mas conferida com `linha * CARRYGRIDY`/`CARGOGRIDY` (7 e 14). A conferência olhava outra célula. Os chamadores (`_MSG_TradingItem`, `ProcessDBMessage`, `CReadFiles` no DBSrv) percorrem os slots com `DestX = i % 9`, `DestY = i / 9` e gravam o item no primeiro slot em que a função devolve TRUE, sem conferir `sIndex`. Com o slot 9 ocupado e a célula 14 livre, o baú devolvia TRUE e **sobrescrevia o item do slot 9**. No baú, a partir da linha 9 a leitura ainda passava do fim de `CargoGrid[128]`, na pilha | A conferência passa a usar a mesma largura da gravação (`CARRYGRIDX`/`CARGOGRIDX`). Os `memset` de 63 e 126 bytes cobrem exatamente as grades 9×7 e 9×14, então não precisaram mudar. Teste: `TestCarryCargoGrid` em `tests/w2pp_common_tests.cpp`, que falha no código antigo. O hash `CanEquipCarry` da caracterização foi regravado |
| `server/common/BaseItem.cpp` (`BASE_GetItemAbilityNosanc`, `GetStaticItemAbility`, `GetBonusItemAbility`, `GetBonusItemAbilityNosanc`), `server/common/BaseCombat.cpp` (`BASE_GetCurrentScore`), `client/src/common/BaseItem.cpp` (`BASE_GetItemAbility`, `GetStaticItemAbility`, `GetBonusItemAbility`, `GetBonusItemAbilityNosanc`) | A guarda era `idx > MAX_ITEMLIST` (ou `> 6500`), então `sIndex == 6500` lia `g_pItemList[6500]`, uma posição além do fim | A guarda passa a ser `>= MAX_ITEMLIST`, como nas outras funções. Item 6500 vale como item inexistente (habilidade 0) |
| `server/common/BaseItem.cpp`, `BASE_GetGrowthRate` | Lia `g_pSancRate[2][cEffect / 10]` com 12 colunas: montaria de nível ≥ 120 lia além do fim. O único chamador (`UseItem_Scrolls`) já recusa nível ≥ 120 antes | Nível ≥ 120 devolve taxa 0 (não cresce mais) |
| `server/common/BaseItem.cpp` (`BASE_CanCarry`, `BASE_CanCargo`) e `client/src/common/BaseItem.cpp` (`BASE_CanCargo`) | Liam `g_pItemGrid[grid]` (8 formatos) com `grid = EF_GRID` do item, sem validar | `EF_GRID` fora de 0..7 (dado corrompido) vale como item de uma célula. Não devolve FALSE, para o DBSrv não deixar de carregar um item do baú |
| `server/common/BaseCombat.cpp`, `BASE_GetCurrentScore` | `mount <= 2390` aceitava 2390 e lia `g_pMountBonus[30]` (a tabela tem 30 linhas, 2360..2389) | `mount < 2390`: o item 2390 não é montaria e não muda a velocidade |
| `client/src/common/BaseItem.cpp`, `BASE_CanTrade` | `pos = (unsigned char)MyTrade[i]` e `if (pos != -1)`: a comparação era sempre verdadeira, e uma entrada -1 (255) **gravava** em `Dest[255]`, fora do inventário de 64 itens. A função não tem chamadores no cliente | `if (pos < MAX_CARRY)`: -1 marca slot vazio |
| `client/src/common/BaseWorld.cpp`, `BASE_GetAttr` | `g_pAttribute[nY / 4 % 1024][nX / 4 % 1024]`: com coordenada negativa o resto era negativo e lia antes do início da tabela | Coordenada negativa devolve 0, como `BASE_GetAttribute` fora do mapa |

Essas correções só mudam o resultado para entradas fora do domínio do jogo: os hashes de
caracterização (que só usam entradas válidas) não mudaram. `TestOutOfDomainInputs` em
`tests/w2pp_common_tests.cpp` e `CheckOutOfDomainInputs` em
`tests/tmproject_characterization_tests.cpp` conferem os valores definidos. No código antigo,
cinco das conferências do servidor falham, e a do `BASE_CanTrade` do cliente derruba o processo
com uma escrita fora da pilha.
