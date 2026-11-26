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
foram corrigidas (ver o fim do arquivo). Fica registrado:

| Onde | O que acontece | Quando dispara |
|---|---|---|
| `client/src/common/BaseWorld.cpp`, `BASE_IsInLowZone` | Fora do mapa chama `MessageBox`/`PostMessage` com `g_pApp->m_hWnd` e fecha o cliente. É tratamento de erro, não leitura fora dos limites | Coordenada fora de 0..4095 |

Outros comportamentos do cliente que o teste fixa como estão (não leem fora dos limites):

- **`BASE_CanCargo`**: `SourGrid` nunca é preenchida, então a função sempre devolve 1. A grade do baú é gravada com `CargoGrid[ty][9 * yy + tx + xx]`, deslocada em relação à linha.
- **`BASE_CanTrade`**: `OpponentTrade` não é usado (copia de uma lista local vazia).
- **`BASE_GetAttribute`**: a guarda confere `x <= 4096` duas vezes e nunca `y`. A máscara `& 1023` mantém o índice dentro da tabela.

O único `-Warray-bounds` que sobra é `client/src/render/MeshManager.cpp:329`, índice 7 (e 28)
em `short [6][60][56]`. Ver abaixo. Os outros avisos acompanhados pelo `mingw_check` são todos
`-Wshadow` (ocultação de nomes), que não indicam defeito por si.

Encontrado ao corrigir `CMob::SetSegment` e mantido: a guarda `if (RouteType <= 0 && RouteType > 4)`
nunca é verdadeira (deveria ser `||`), então o log "Wrong SetSegment" nunca sai. Trocar para `||`
faria a rota 0, que é válida e tratada no laço, sair cedo; a intenção não é clara.

### `MeshManager::m_sAnimationArray` menor do que o uso (não corrigido)

`m_sAnimationArray` é declarado com `MAX_ANI_CLASS = 6` classes, mas:

- o carregador (`MeshManager.cpp`, laço sobre o arquivo de animações) **grava** em
  `m_sAnimationArray[nCount]` com `nCount == 7` ou `28`, que vem do arquivo: são 60×56 `short`
  gravados além do fim, por cima de outras variáveis globais;
- os leitores (`TMHuman_Animation.cpp`, `TMHuman_Effects.cpp`) indexam
  `m_sAnimationArray[m_nSkinMeshType]` sem guarda, e o tipo de malha chega a 20, 21, 28, 29
  (montarias) e até 59 (`g_MobAniTable[60]`).

O mais provável é que a decompilação tenha errado o tamanho da primeira dimensão. Aumentá-la
tiraria a escrita fora dos limites, mas muda os índices de animação que os monstros e montarias
leem hoje (o que está depois do vetor na memória passaria a ser 0). Isso só pode ser validado
rodando o cliente com os arquivos do jogo, então fica para quando o teste no jogo for possível.

### Cor das malhas 737..739 na interface (não corrigido)

`RenderDevice_Geom.cpp:582` grava `pVertex[i].diffuse = ipControl->n3DObjIndex - 2964`, um
número pequeno negativo no lugar de uma cor, ao lado de um `dwColor[3]` que não é usado. É o
mesmo desenho do `TMItem::Render` corrigido abaixo, onde a intenção (`dwColor[índice - 737]`)
fica clara. Aqui não há leitura fora dos limites, e a cor exibida hoje é determinística; corrigir
muda o visual no inventário, então fica como decisão separada.

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
| `server/tmsrv/handlers/items/_MSG_Buy.cpp` | O GCC apontava `GuildTax`, `Price`, `Price2`, `CityTax`, `bPrice` e `Desconto` possivelmente não inicializados. Análise: (1) `GuildTax` no `goto LABEL_BUY1` é inalcançável, porque o único uso exige cidade e imposto válidos, exatamente a negação do `goto`; (2) na compra por doação, o `goto DonateBuy` pula as declarações e o `ItemLog` gravava `price:` com lixo da pilha (a mensagem de depuração com `FREEEXP == -2` também); (3) `g_pGuildZone[Village].CityTax` era lido antes de validar `Village`, além do fim da tabela fora das cidades, e o valor era descartado pelo `goto` | As seis variáveis são declaradas com 0 antes do `goto DonateBuy`, e `CityTax` só é lido com índice válido (0 fora das cidades, como em `SendShopList`). Preço, imposto e dinheiro do jogador não mudam em nenhum caminho; o log da compra por doação passa a gravar `price:0`. Os 6 avisos `-Wmaybe-uninitialized` saíram da linha de base |
| `server/tmsrv/net/GetFunc.cpp`, `GetCreateItem` | Item 3145 (estandarte da cidade): `g_pGuildZone[zone].Victory` e `.ChargeGuild` lidos sem validar `zone`. Fora das cidades `BASE_GetVillage` devolve 5, a leitura passava do fim da tabela e o valor entrava no índice e nos efeitos do item enviado ao cliente. Só dispara se o dado do jogo puser o 3145 fora de uma cidade | Com zona válida, nada muda. Fora das cidades `Victory` e `ChargeGuild` valem 0: estandarte sem vitória e sem guilda dona |
| `client/src/entities/TMHuman.cpp`, `TMHuman::InitObject` | Classe 16 (dragão esmeralda): os olhos 8 e 9 eram criados em `m_pEyeFire[8]`/`[9]`, além do fim do vetor de 7, e caíam em `m_pEyeFire2[1]`/`[2]`, o membro seguinte (que `RenderEffect_EmeraldDragon` atualiza com `m_vecTempPos[i + 7]` e as rotinas de limpeza apagam) | Grava direto em `m_pEyeFire2[i - 7]`: mesma memória, sem comportamento indefinido. O comportamento não muda |
| `client/src/entities/TMItem.cpp`, `TMItem::Render` | Malhas 737..739 no chão: `dwColor[nMeshIndex - 2980]` dava índice -2243..-2241 e lia antes do início de `dwColor[3]` (laranja, azul, verde) | `dwColor[nMeshIndex - 737]`, uma cor por malha. **Muda o visual**: antes a cor vinha de memória fora do vetor |
| `client/src/scene/TMFieldScene_Controls.cpp`, sussurro longo | `&stMsgWhisper.MobName[maxLen + 15]` passava do fim de `MobName[16]` para chegar a `String` | `&stMsgWhisper.String[maxLen - 1]`: mesmo endereço (sem preenchimento entre os dois vetores de `char`). O comportamento não muda |
| `server/dbsrv/core/Server.cpp`, `WSA_ACCEPTADMIN` | `for (i = 0; i <= MAX_ADMIN; i++)` lia `pAdmin[MAX_ADMIN]`, além do fim. No MSVC lia 4 bytes a mais; no GCC `-O2` o comportamento indefinido fez o compilador tirar o limite do laço, que varria a memória até cair: toda conexão na porta admin de um IP sem sessão derrubava o DBSrv. Achado pelo `server_boot_smoke.py`; o GCC não avisa | `i < MAX_ADMIN`. Só muda o caso em que os bytes após o array coincidissem com o IP (a conexão era recusada como "unknown attempt"; agora segue para a lista `pAdminIP`) |
| `server/tmsrv/systems/ScheduledEventSystem.cpp`, pista +3 | No bloco "grupo 2 ganhou", as runas iam para `Party[1]` e o prêmio 5134 para `Pista[3].Party[3].LeaderID`, além do fim de `Party[3]`: caía em `Pista[4].Party[0]`, o líder de outra pista (ou 0) | `Party[1]`, como nos outros cinco blocos. **Muda quem recebe o prêmio** |
| `server/tmsrv/handlers/items/UseItem_Quests.cpp`, bilhete do pesadelo | O bilhete solo pula com `goto lbl_a_noparty` a leitura do relógio, e `NigthTime` era calculado com `tm_min`/`tm_sec` sem valor | O relógio é lido antes do `goto`. O solo continua sem a checagem de horário; o tempo restante passa a ser o real. **Muda o tempo do bilhete solo** |
| `server/tmsrv/entities/CMob.cpp`, `SetSegment` | Rota 4 com `SegmentProgress == -1` saía sem valor de retorno e lia `SegmentListX[-1]`. O estado não acontece (direção 1 só nas rotas 2 e 3) | Retorno padrão 0 e progresso 0 nesse caminho. Caminhos alcançáveis iguais |
| `server/common/Basedef.cpp`, `BASE_GetEnglish` | `strncpy(temp, name, sizeof(name))` para um `temp` nunca usado | Cópia removida. Sem mudança |
| `server/tmsrv/net/GetFunc.cpp`, `server/tmsrv/systems/ItemSpawnSystem.cpp` (2×) | `Height = -204` num `unsigned char` (vira 52) | Conversão explícita `static_cast<unsigned char>(-204)`. Sem mudança |
| `client/src/render/CFrame.cpp`, osso 20, quadro 7 | `m_vecTempPos[10]` recebia um `vecOut3` nunca calculado (lixo da pilha) | O ramo não grava mais `[10]`: a origem do ponto se perdeu na decompilação e os leitores de `[10]` são efeitos de traje humanos (ossos 0/1) |
| `client/src/render/DXUtil.cpp`, `CArrayList` | `delete[]` de `void*` (alocado como `BYTE[]`) | `delete[] static_cast<BYTE*>(m_pData)`. Sem mudança |
| `client/src/gameplay/MrItemMix.cpp` | A cópia do item exigido no painel de mistura usava `sizeof(pstItem)` (4 de 8 bytes) e perdia os efeitos 1 e 2 | `sizeof(*pstItem)`. **Muda o item exibido** (passa a mostrar todos os efeitos) |
| `client/src/common/BaseItem.cpp`, `BASE_ClearItem` | `memset(item, 0, sizeof(item))` zerava 4 de 8 bytes | `sizeof(*item)`. O único chamador é `BASE_CanTrade`, sem chamadores; os hashes `ItemSancAndColor` e `EquipCarryTrade` da caracterização foram regravados |

As correções das funções `BASE_*` só mudam o resultado para entradas fora do domínio do jogo
(a única correção com efeito visível com dados válidos é a cor do `TMItem::Render`): os hashes de
caracterização (que só usam entradas válidas) não mudaram. `TestOutOfDomainInputs` em
`tests/w2pp_common_tests.cpp` e `CheckOutOfDomainInputs` em
`tests/tmproject_characterization_tests.cpp` conferem os valores definidos. No código antigo,
cinco das conferências do servidor falham, e a do `BASE_CanTrade` do cliente derruba o processo
com uma escrita fora da pilha.
