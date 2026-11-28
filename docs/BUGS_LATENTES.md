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

Encontrado ao conferir os limites de `SetAffect` (`server/tmsrv/systems/AffectSystem.cpp`) e
mantido: a guarda `if (pMob[conn].MOB.Merchant == 1 || conn > MAX_USER)` recusa o `SetAffect` em
todo monstro e NPC (só `pMob[MAX_USER]`, o primeiro, passa), enquanto `SetTick` usa `&&` e só
recusa o mercador. Trocar para `&&` passaria a aplicar buffs e debuffs em monstros, o que muda o
combate; fica como decisão separada.

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

### Rodapé do `ItemList.bin` gravado pelo servidor (não corrigido)

O `BASE_ReadItemList` do cliente em Release só aceita o `ItemList.bin` cujos 4 últimos bytes
valem `0x1343B16`. O `BASE_WriteItemList` do servidor, que regrava o `Common/ItemList.bin` a
partir do `itemlist.csv` em todo boot, põe ali um `W2Rand()`. O checksum de verdade vai para o
`../ItemCSum.h`. Por isso um `ItemList.bin` gerado pelo servidor não serve para o cliente sem
corrigir o rodapé. O `tools/w2_bin_csv.py` mostra o rodapé que encontra. Corrigir significa
decidir qual lado está certo: o cliente reconstruído pode ter trocado o checksum calculado
(`sum`, hoje "Not being used") pelo rodapé lido. Essa decisão fica para quando houver um patch de
cliente gerado a partir do servidor.

Evidência posterior: o `ItemList.bin` e o `SkillData.bin` oficiais do WYD Global (patch 749)
têm rodapé `0x4823` e `0x29`. São o segundo e o primeiro valor do `rand()` do MSVC com a semente
padrão, o mesmo que o servidor grava. Então a ferramenta oficial também põe um `rand()` ali. Isso
reforça que a checagem contra `0x1343B16` do cliente reconstruído não é a regra original, mas o
Global é de uma versão mais nova que a do cliente-alvo, então a evidência não é conclusiva.

### Conferência do `cdata.bin` que nunca passa (não corrigido)

O construtor de `TMGround` (`client/src/world/TMGround.cpp`) soma o `cdata.bin` num `int` e, na
cena de campo, compara com `5855606140`, constante maior que 32 bits: a comparação nunca é
igual e, se valesse, mostraria "DataFile Error" e fecharia o cliente. Hoje ela não dispara porque
a conferência só roda no primeiro `TMGround` criado (`m_bFirst`), e as cenas de seleção de
servidor e de personagem criam um antes do campo. A constante provavelmente perdeu os bits altos
(ou ganhou) na reconstrução; o valor de 32 bits original não é conhecido, e o `cdata.bin`
regenerado pela `tools/global_client_data.py` não teria a soma do original de qualquer jeito.
Fica registrado para quem mudar a ordem das cenas.

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
| `server/tmsrv/entities/CMob.cpp`, `SetSegment`; `StandingByProcessor`; `server/tmsrv/systems/MobAISystem.cpp`, `ProcessNpcMovementTick` | Rota 4 com `SegmentProgress == -1` saía sem valor de retorno e lia `SegmentListX[-1]`. Rota 3 voltando ao início (`SegmentProgress == -1`) e rota 1 no fim (`SegmentProgress == 5`) saíam do laço com `break` sem acertar o progresso e liam `SegmentListX[-1]` e `[5]` (UBSan no `server_boot_smoke.py`). A guarda do `ProcessNpcMovementTick` era `Progress > MAX_SEGMENT` e aceitava 5 | Retorno padrão 0; as rotas 3 e 1 recomeçam do segmento 0 (a 3 também com direção 0); `SetSegment` e `StandingByProcessor` conferem o progresso antes de indexar; a guarda passa a `>= MAX_SEGMENT`. **Muda a rota**: os NPCs das rotas 1 e 3 recomeçam do primeiro segmento em vez de ler fora do vetor |
| `server/tmsrv/systems/MobSpawnSystem.cpp`, `GenerateMob` | Seguidores em formação liam `g_pFormation[i][j][Formation]` com a tabela declarada `[5][12][2]` (formação, seguidor, eixo): índices trocados, leitura além do fim com 6 seguidores, e o mesmo deslocamento em X e Y. O mercador 6 (imposto) gravava `GuildImpostoID[level]` e lia `g_pGuildZone[level]` com o nível do NPC, sem conferir | `g_pFormation[Formation][i][0]` para X e `[1]` para Y, com formação fora de 0..4 tratada como 0; nível fora de 0..4 não registra o NPC de imposto. **Muda a posição dos seguidores** em formação |
| `server/tmsrv/systems/AffectSystem.cpp`, `SetAffect` e `SetTick` | `pMob[conn]` e `g_pSpell[skillnum]` sem conferir o índice. `SetTick` recusava o mercador só com `mob > MAX_USER` | Índice fora de `1..MAX_MOB-1` ou skill fora de `0..MAX_SPELL_LIST-1` devolve FALSE; `SetTick` passa a `mob >= MAX_USER`, que inclui o primeiro NPC. A guarda do `SetAffect` que recusa todo NPC ficou como está (acima) |
| `server/tmsrv/net/SendFunc.cpp` (`SendClientMessage`, `SendClientMessageOk`, `SendChat`, `SendSay`), `server/tmsrv/systems/imple.cpp` (`notice`, `chiefnotice`) | `memcpy(..., Message, MESSAGE_LENGTH)` copiava 96 bytes de qualquer texto, lendo além do fim das literais curtas (ASan, `global-buffer-overflow`, no `/day`) | `strncpy` limitado a `MESSAGE_LENGTH - 1` (e ponteiro nulo ignorado). O texto enviado não muda; os bytes depois do terminador passam a ser zero |
| `server/tmsrv/core/CReadFiles.cpp`, `ReadSancRate` | A linha `ÂMAGO` gravava em `g_pSancRate[0]`, sobrescrevendo as taxas de `PO`, e só era reconhecida na codificação em que o fonte foi salvo | Grava em `g_pSancRate[2]` e aceita `Âmago` em UTF-8, CP1252 ou sem acento. **Muda as taxas**: `PO` deixa de ser sobrescrita e `Âmago` passa a valer |
| `server/common/Basedef.cpp`, `BASE_GetEnglish` | `strncpy(temp, name, sizeof(name))` para um `temp` nunca usado | Cópia removida. Sem mudança |
| `server/tmsrv/net/GetFunc.cpp`, `server/tmsrv/systems/ItemSpawnSystem.cpp` (2×) | `Height = -204` num `unsigned char` (vira 52) | Conversão explícita `static_cast<unsigned char>(-204)`. Sem mudança |
| `client/src/render/CFrame.cpp`, osso 20, quadro 7 | `m_vecTempPos[10]` recebia um `vecOut3` nunca calculado (lixo da pilha) | O ramo não grava mais `[10]`: a origem do ponto se perdeu na decompilação e os leitores de `[10]` são efeitos de traje humanos (ossos 0/1) |
| `client/src/render/DXUtil.cpp`, `CArrayList` | `delete[]` de `void*` (alocado como `BYTE[]`) | `delete[] static_cast<BYTE*>(m_pData)`. Sem mudança |
| `client/src/gameplay/MrItemMix.cpp` | A cópia do item exigido no painel de mistura usava `sizeof(pstItem)` (4 de 8 bytes) e perdia os efeitos 1 e 2 | `sizeof(*pstItem)`. **Muda o item exibido** (passa a mostrar todos os efeitos) |
| `client/src/common/BaseItem.cpp`, `BASE_ClearItem` | `memset(item, 0, sizeof(item))` zerava 4 de 8 bytes | `sizeof(*item)`. O único chamador é `BASE_CanTrade`, sem chamadores; os hashes `ItemSancAndColor` e `EquipCarryTrade` da caracterização foram regravados |
| `client/src/render/TextureManager.cpp` (`GetUITexture`, `GetEffectTexture`, `GetModelTexture`, `GetEnvTexture`, `GetDynamicTexture`, marcas de guilda, conjuntos de UI) | Guardas `nIndex > MAX_*` aceitavam `nIndex == MAX_*` e liam uma posição além do fim | `nIndex >= MAX_*`. O índice igual ao limite vale como textura inexistente |
| `client/src/render/MeshManager.cpp`, `InitMeshManager` | Gravava `m_stCommonMesh[nIndex]` sem conferir o índice lido do `MeshList.txt`; o do WYD Global vai até 3148, além das 3048 entradas | Índice fora da tabela vai para o log e é ignorado; a tabela passou a 4096 (docs/DADOS_GLOBAL.md) |
| `client/src/scene/ObjectManager.cpp`, `InitCurseList` | A contagem lida do `curse.bin` ia direto para o `new[]` e para o laço: com o arquivo do Global, `bad_array_new_length` derrubava o cliente | Contagem que não cabe no tamanho do arquivo deixa a lista vazia (com log) |
| `client/src/scene/TMSelectServerScene.cpp`, botão Conectar | `g_nServerCountList[nIndexN - selecionado - 1]` usava a contagem do grupo no lugar do total de grupos: com um grupo real selecionado lia `g_nServerCountList[-1]`, e o Conectar recusava com "Selecione o canal e o servidor" | Mesmo mapeamento do clique na lista de grupos (`nMaxGroupN - selecionado - 1`), com índice conferido. **Muda o comportamento**: o Conectar passa a funcionar nos grupos reais |
| `client/src/scene/TMSelectServerScene.cpp` (3×) e `TMFieldScene_UI.cpp` (2×), página de status | `sscanf_s` com `"%d\\n..."` (barra invertida literal no formato) só lia o primeiro número; os outros canais ficavam com o valor inicial | `BASE_ParseUserCount` lê a página inteira e põe a linha k (canal k + 1, como o DBSrv grava) em `nUserCount[k + 1]`. Teste: `CheckUserCountPage` em `tests/tmproject_characterization_tests.cpp`. **Muda o comportamento**: canais ausentes da página aparecem indisponíveis |
| `client/src/audio/DirShow.cpp`, `CleanGraph` | Contava os filtros com `while (SUCCEEDED(Skip(1)))`, que não termina (`Skip` devolve `S_FALSE`, que é sucesso, no fim da lista), e liberava cada filtro em laço até a contagem zerar. Com música ligada, a entrada no campo derrubava o cliente (medição com os dados do Global) | Recolhe os filtros com `Next` até não vir `S_OK` (no máximo 32) e dá um `Release` por filtro, o da referência que o `Next` entregou |
| `client/src/render/CMesh.cpp`, armas 2889..2901, 2906..2909 e 2912..2923 | Usava `pMesh->m_pVB` sem conferir: a malha que não carregou (textura fora da lista) tem buffers nulos e o `Lock` derrubava o cliente na seleção de personagem | Malha nula, sem buffers ou com textura fora da lista sai pelo mesmo `return 1` das falhas do dispositivo |
| `client/src/scene/TMScene.cpp`, `LogMsgCriticalError` | Lia `m_pMyHuman` sem conferir; o erro de dados críticos do `TMGround` pode vir antes de o personagem existir, e o relatório derrubava o cliente | Sem personagem, o relatório vai com ID e posição 0 |
| `client/src/scene/TMFieldScene_Items.cpp`, `UseTicket` | Procurava a grade única `G_INV_GRID` (65554), que o inventário paginado não tem mais: confirmar o uso de um ticket derrubava o cliente. A posição enviada também ignorava a página | Usa a página aberta (`m_pGridInv`) e soma a página à posição, como `SGridControl` faz nos outros itens de uso |
| `client/src/scene/TMFieldScene_PacketItems.cpp`, `OnPacketCNFDropItem` | Baú: procurava a grade única `G_CARGO_GRID` (65690) e derrubava o cliente ao largar um item do baú. Inventário: tirava a célula `SourPos % 5, SourPos / 5` da página aberta (o item de outra página continuava na grade) e guardava o ponteiro numa variável local, sem liberar o item | Página e célula como em `OnPacketSwapItem` (15 posições por página no inventário, 40 no baú), com índices conferidos; o item tirado é liberado como no ramo do equipamento. O `-Wshadow` do arquivo saiu da linha de base |
| `client/src/scene/TMFieldScene_PacketItems.cpp`, `OnPacketCNFGetItem` | Ao pegar ouro do chão, escrevia em `m_pMoney2` (texto 1041) sem conferir; o layout do WYD Global não tem esse controle | Confere o ponteiro, como nos outros textos de dinheiro |
| `server/tmsrv/net/ProcessClientMessage.cpp`, despacho | O despacho só exigia o cabeçalho de 12 bytes: um pacote mais curto do que a estrutura do seu tipo chegava ao handler, que lia os campos além do fim do pacote (o resto do buffer de recepção) | `ClientMessageMinSize` dá o `sizeof` da estrutura que o cliente envia em cada tipo (o cliente é a referência; os tamanhos do mingw i686 batem com os do servidor). Pacote curto é descartado e registrado (`err,packet short`). O teste de jogo com o `LoginClient` não registra nenhum |
| `server/tmsrv/handlers/trade/_MSG_TradingItem.cpp`, teleporte pelo NPC | A guarda `WarpID <= MAX_USER && WarpID >= MAX_MOB` nunca era verdadeira: o `WarpID` do pacote indexava `pMob[WarpID]` sem limite (achado pelo `fuzz_client_message`) | `WarpID < MAX_USER \|\| WarpID >= MAX_MOB` recusa, como a guarda pretendia. Entrada de regressão: `tests/fuzz/regress/client/tradingitem-warpid` |
| `server/tmsrv/core/Server_Logs.cpp` (`Log`, `ChatLog`, `ItemLog`), `server/dbsrv/core/Server_Logs.cpp` (`Log`, `Temp`), `server/dbsrv/core/CReadFiles.cpp` | `fprintf(fp, LogTemp)`: o texto do log vira formato, e o nome da conta (antes da autenticação) ou o chat com `%n`/`%s` lia e gravava pela pilha. O `sprintf` em `LogTemp[1024]` também não tinha limite (achado pelo `fuzz_client_message`) | `fprintf(fp, "%s", ...)` e `snprintf` limitado ao que sobra de `LogTemp`. O texto gravado não muda; linhas acima de 1023 bytes são cortadas. Entrada de regressão: `tests/fuzz/regress/client/log-format-string` |
| `server/tmsrv/handlers/account/_MSG_AccountLogin.cpp` | `sscanf(m->AccountName, "%s", pUser[conn].AccountName)` com o campo do pacote sem terminador copiava além dos 16 bytes do destino, antes da autenticação | Termina o campo em `ACCOUNTNAME_LENGTH - 1` e limpa o destino antes. Conta válida não muda |
| `server/tmsrv/handlers/combat/_MSG_Attack.cpp`, skill 79 | O laço copiava 6 alvos do pacote, mas `MSG_AttackOne` (1 alvo) e `MSG_AttackTwo` (2) são menores: a cópia lia e gravava além do fim do pacote | Copia só os alvos que cabem no tamanho recebido (1, 2 ou 6). O laço de dano já não lia os outros |
| `server/tmsrv/entities/CNPCGene.cpp`, `ReadNPCGenerator`; `server/dbsrv/persistence/CFileDB_AccountPackets.cpp` (criação e troca de nome, 3×) | `'Í'` e `'í'` num fonte UTF-8 viram constantes de dois bytes (`0xC38D`, `0xC3AD`) que nenhum `char` iguala: as comparações nunca valiam | Hex escapes `'\xCD'` e `'\xED'`, os bytes CP1252 do fonte original. **Muda o comportamento**: o DBSrv volta a recusar nomes com `\xED\xED`, e a regravação do `NPCGener` volta a pular as linhas que começam com `\xCD` |

As correções das funções `BASE_*` só mudam o resultado para entradas fora do domínio do jogo
(a única correção com efeito visível com dados válidos é a cor do `TMItem::Render`): os hashes de
caracterização (que só usam entradas válidas) não mudaram. `TestOutOfDomainInputs` em
`tests/w2pp_common_tests.cpp` e `CheckOutOfDomainInputs` em
`tests/tmproject_characterization_tests.cpp` conferem os valores definidos. No código antigo,
cinco das conferências do servidor falham, e a do `BASE_CanTrade` do cliente derruba o processo
com uma escrita fora da pilha.
