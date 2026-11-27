# Divergências entre servidor e cliente

Gerado por `tools/divergence_map.py`; não edite à mão. O servidor (W2PP, `server/`) e o cliente
(TMProject, `client/`) vêm de versões diferentes do jogo, e cada um tem a sua cópia das structs, dos
opcodes e das regras `BASE_*`. O alvo escolhido é a versão do cliente: cada linha abaixo é algo que o
servidor teria de mudar para falar com este cliente, ou uma decisão a tomar.

## Resumo

- **Pacotes**, casados pelo valor do opcode e pela struct que cada lado envia ou lê (uma "direção" é um opcode num sentido): **2 direções em que quem lê espera mais bytes do que o outro lado envia** e **1 em que os campos lidos não estão nos offsets enviados**. Outras 3 batem em offsets mas trocam nomes de campos, e 114 batem por inteiro. Em 36, o cliente envia ou espera um pacote que o servidor não lê ou não envia; em 42, é o servidor que envia ou lê algo que o cliente não usa (fora 53 do tráfego com o DBSrv).
- **Structs de jogo e de arquivo** de mesmo nome: 14, das quais 2 diferem. `STRUCT_MOB` tem 1024 bytes no servidor e 1024 no cliente, e vai dentro dos pacotes de login.
- **Regras `BASE_*`** comparadas com as mesmas entradas: 29. **0 divergem** em ao menos uma entrada; 29 concordam em todas.
- **Só de um lado**: 52 funções `BASE_*` só no servidor e 26 só no cliente.

## Como foi medido

- **Layout**: `sizeof` e `offsetof` de cada struct e campo, compilados com o mingw i686 (os flags do
  `tools/mingw_check.py`, que reproduzem o layout do MSVC Win32) e executados no wine. Nada é estimado
  a partir do texto.
- **Pacotes**: um opcode é casado pelo valor (`_MSG_X` no servidor; `MSG_X_Opcode` ou literal no
  cliente; num envio, também o decimal acima de 255). A struct enviada vem de `var.Type = OPCODE` (com a declaração de `var`) e dos helpers
  `SendClientSignal*` do TMSrv. A struct lida vem do primeiro cast dentro do `case OPCODE:` ou da
  função que ele chama, ou do cast depois de um `if (p->Type == OPCODE)`. É análise de texto: um pacote montado de outro jeito (por exemplo, `memcpy`
  num buffer cru) não aparece, e o primeiro cast pode não ser a struct inteira do pacote. Quando o
  TMSrv devolve o pacote que recebeu (`SendOneMessage((char*)m, ...)` com o `m` do cast), o envio
  conta para o mesmo opcode e aparece marcado "(eco)". Um handler sem cast conta como leitura do
  cabeçalho (`MSG_STANDARD`). Cada linha
  traz o local no código para conferir.
- **Campos**: a comparação desce nas structs aninhadas (`MOB.CurrentScore.Level`; num array de structs,
  o primeiro elemento, `Affect[].Value`, já que os outros repetem o layout). Um campo de quem lê bate
  se do outro lado há um campo no mesmo offset e com o mesmo tamanho, se cai dentro de um bloco de
  bytes (`char x[N]`, que o outro lado preenche cru ou zera) ou se é coberto exatamente por campos
  seguidos (um array dividido em campos). Um nome que existe dos dois lados em offsets diferentes conta
  como desalinhado, mesmo que os offsets batam com outros campos: são dois campos trocados de lugar.
- **Regras**: `tools/divergence/base_compare.cpp` é compilado uma vez contra cada lado. As tabelas que o
  jogo carrega de arquivo (`g_pItemList`, `g_pSpell`, `g_pAttribute`, mapa de alturas) recebem os mesmos
  valores sintéticos, campo a campo, mesmo onde o nome ou o offset muda, e as entradas ficam no domínio
  que o jogo garante. Onde as assinaturas diferem, o parâmetro que só um lado tem recebe um valor neutro,
  indicado na tabela. As entradas são conferidas linha a linha (mesma função, mesma entrada nos dois
  lados) antes de comparar as saídas. O código do cliente é uma reconstrução com outro texto, então
  "concorda" quer dizer mesma saída, não mesmo código.

## Pacotes em que quem lê espera mais do que recebe

Tamanho em bytes da struct que um lado envia e da que o outro lê para o mesmo opcode. Aqui quem lê
espera mais bytes do que chegam: ou o pacote é recusado, ou os campos do fim saem do buffer.

| Opcode | Direção | Envia | Lê | Onde |
|---|---|---|---|---|
| `0x10A` | servidor → cliente | `MSG_DBCNFAccountLogin` (1928) | `MSG_CNFAccountLogin` (1928), `MSG_CNFRemoveServerLogin` (3624) | `server/tmsrv/net/ProcessDBMessage.cpp:398`<br>`client/src/scene/TMFieldScene_Packets.cpp:97` |
| `0x3B4` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM2` (20) | `server/tmsrv/handlers/items/UseItem_Quests.cpp:881`<br>`client/src/scene/TMSelectCharScene.cpp:1349` |

- `0x10A`: o cliente só lê `MSG_CNFRemoveServerLogin` já em campo, depois da troca de servidor (0x52A), que o servidor não implementa; na tela de login lê `MSG_CNFAccountLogin`, que bate.
- `0x3B4`: o cliente lê só `Parm1`, que o servidor envia.

## Pacotes com campos em outros offsets

Quem lê cabe no que foi enviado, mas alguns dos campos que ele lê (também dentro das structs
aninhadas) não têm, do outro lado, um campo no mesmo offset e com o mesmo tamanho, ou têm o mesmo
nome em outro offset: o valor chega trocado ou cortado. A última coluna lista esses campos da struct
de quem lê.

| Opcode | Direção | Envia | Lê | Onde | Campos |
|---|---|---|---|---|---|
| `0x363` | servidor → cliente | `MSG_CreateMobTrade` (256) | `MSG_CreateMob` (232) | `server/tmsrv/net/GetFunc.cpp:1074`<br>`client/src/scene/TMFieldScene_Packets.cpp:70` | `MSG_CreateMob`: `Server` |

- `0x363`: bug do cliente: na loja (0x363) ele lê a cidadania pelo layout de `MSG_CreateMob`, no byte 228, que é `Desc[0]`; o servidor envia `Desc` ali, como o cliente exige, e a cidadania em 252.

<details><summary>Pacotes com os mesmos offsets e campos renomeados</summary>

Mesma struct dos dois lados e mesmos offsets, mas com campos de outro nome. Em geral é só nomenclatura;
às vezes é outro significado no mesmo lugar (`Hold` virou `FakeExp`, por exemplo).

| Opcode | Direção | Envia | Lê | Onde | Campos |
|---|---|---|---|---|---|
| `0x336` | servidor → cliente | `MSG_UpdateScore` (152) | `MSG_UpdateScore` (152) | `server/tmsrv/net/SendFunc.cpp:1140`<br>`client/src/entities/TMHuman_Packets.cpp:113` | `RegenHP` só no servidor (134+1); `RegenMP` só no servidor (135+1) |
| `0x337` | servidor → cliente | `MSG_UpdateEtc` (48) | `MSG_UpdateEtc` (48) | `server/tmsrv/net/SendFunc.cpp:1204`<br>`client/src/entities/TMHuman_Packets.cpp:134` | `Magic` só no servidor (38+2) |
| `0x364` | servidor → cliente | `MSG_CreateMob` (232) | `MSG_CreateMob` (232) | `server/tmsrv/net/GetFunc.cpp:951`<br>`client/src/scene/TMFieldScene_Packets.cpp:69` | `Unknow` só no servidor (133+3) |

</details>

## Pacotes que só o cliente usa

O cliente envia e o servidor não tem um `case` que leia, ou o cliente tem um `case` para um opcode que
o servidor nunca envia. Parte pode ser recurso novo da versão do cliente; parte pode ser um envio que
a análise de texto não reconhece.

A coluna "No servidor" mostra o nome do opcode quando o servidor o declara: nesses casos ele conhece o
valor, mas a análise não achou quem envie ou leia.

| Opcode | Direção | Struct no cliente | Onde | No servidor |
|---|---|---|---|---|
| `0x105` | servidor → cliente | `MSG_MessageChat` (140), `MSG_STANDARD` (12) | `client/src/entities/TMHuman_Packets.cpp:125` | — |
| `0x106` | servidor → cliente | `MSG_MessageChat` (140), `MSG_STANDARD` (12) | `client/src/entities/TMHuman_Packets.cpp:128` | — |
| `0x1BF` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Packets.cpp:174` | — |
| `0x1C1` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Packets.cpp:75` | — |
| `0x1C2` | servidor → cliente | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Packets.cpp:204` | — |
| `0x1C5` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Packets.cpp:195` | — |
| `0x1C6` | servidor → cliente | `MSG_RandomQuiz` (268) | `client/src/scene/TMFieldScene_Packets.cpp:198` | — |
| `0x1CF` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/entities/TMHuman_Packets.cpp:140` | — |
| `0x1D0` | servidor → cliente | `MSG_ShopList` (236) | `client/src/scene/TMFieldScene_Packets.cpp:119` | — |
| `0x1D6` | servidor → cliente | `MSG_INITGULDNAME` (28) | `client/src/scene/TMSelectCharScene.cpp:1556` | — |
| `0x2BD` | cliente → servidor | `MSG_HellBuy` (24) | `client/src/scene/TMFieldScene_MsgBox.cpp:165` | — |
| `0x2BE` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Controls.cpp:187` | — |
| `0x2C7` | cliente → servidor | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_UI.cpp:562` | — |
| `0x2C8` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Packets.cpp:201` | — |
| `0x3A0` | cliente → servidor | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene.cpp:82` | `_MSG_Ping` |
| `0x3AE` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Packets.cpp:216` | — |
| `0x3AE` | cliente → servidor | `MSG_STANDARDPARM` (16) | `client/src/core/NewApp.cpp:1279` | — |
| `0x3AF` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/entities/TMHuman_Packets.cpp:137` | — |
| `0x3B0` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Packets.cpp:159` | `_MSG_MobLeft` |
| `0x3B2` | servidor → cliente | `MSG_ReqSummon` (32) | `client/src/scene/TMFieldScene_Packets.cpp:87` | — |
| `0x3B3` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Packets.cpp:89` | — |
| `0x3BB` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Packets.cpp:171` | `_MSG_MobCount` |
| `0x3C9` | cliente → servidor | `MSG_UseItem2` (52) | `client/src/scene/TMFieldScene_Items.cpp:613` | — |
| `0x3CA` | servidor → cliente | `MSG_STANDARD` (12) | `client/src/entities/TMHuman_Packets.cpp:95` | — |
| `0x3CF` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Packets.cpp:210` | — |
| `0x3D0` | servidor → cliente | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Packets.cpp:213` | — |
| `0x3D1` | cliente → servidor | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:929` | — |
| `0x3E8` | servidor → cliente | `MSG_RepurchaseItems` (176) | `client/src/scene/TMFieldScene_Packets.cpp:101` | — |
| `0x3E8` | cliente → servidor | `MSG_RepurchaseItems` (176) | `client/src/ui/SGrid_Trade.cpp:27` | — |
| `0x7B1` | servidor → cliente | `MSG_LongMessagePanel` (528) | `client/src/scene/TMFieldScene_Packets.cpp:85` | — |
| `0x7DB` | servidor → cliente | `MSG_STANDARDPARM` (16) | `client/src/scene/TMScene_Packets.cpp:469` | — |
| `0xAD9` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene.cpp:423` | — |
| `0xBFF` | servidor → cliente | `MSG_Encode` (180) | `client/src/scene/TMScene_Packets.cpp:74` | — |
| `0xDC3` | servidor → cliente | `MSG_CAPSULEINFO` (56) | `client/src/scene/TMFieldScene_Packets.cpp:207` | `_MSG_CNFCapsuleInfo` |
| `0xED7` | cliente → servidor | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:993` | — |
| `0xED8` | cliente → servidor | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:1003` | — |

<details><summary>Pacotes que batem</summary>

Quem lê cabe no que foi enviado e todos os campos que ele lê estão nos mesmos offsets.

| Opcode | Direção | Envia | Lê | Onde |
|---|---|---|---|---|
| `0x110` | servidor → cliente | `MSG_CNFNewCharacter` (856) | `MSG_CNFNewCharacter` (856) | `server/tmsrv/net/ProcessDBMessage.cpp:583`<br>`client/src/scene/TMSelectCharScene.cpp:1400` |
| `0x112` | servidor → cliente | `MSG_CNFDeleteCharacter` (856) | `MSG_CNFDeleteCharacter` (856) | `server/tmsrv/net/ProcessDBMessage.cpp:599`<br>`client/src/scene/TMSelectCharScene.cpp:1416` |
| `0x114` | servidor → cliente | `MSG_CNFClientCharacterLogin` (1712) | `MSG_CNFCharacterLogin` (1712) | `server/tmsrv/net/ProcessDBMessage.cpp:781`<br>`client/src/scene/TMFieldScene_Packets.cpp:99` |
| `0x116` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/core/Server.cpp:1077`<br>`client/src/scene/TMFieldScene_Packets.cpp:93` |
| `0x165` | servidor → cliente | `MSG_RemoveMob` (16) | `MSG_RemoveMob` (16) | `server/tmsrv/net/SendFunc.cpp:499`<br>`client/src/entities/TMHuman_Packets.cpp:101` |
| `0x16F` | servidor → cliente | `MSG_DecayItem` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/handlers/items/_MSG_GetItem.cpp:113`<br>`client/src/scene/TMFieldScene_Packets.cpp:117` |
| `0x171` | servidor → cliente | `MSG_CNFGetItem` (28) | `MSG_CNFGetItem` (28) | `server/tmsrv/handlers/items/_MSG_GetItem.cpp:237`<br>`client/src/scene/TMFieldScene_Packets.cpp:113` |
| `0x175` | servidor → cliente | `MSG_CNFDropItem` (28) | `MSG_CNFDropItem` (28) | `server/tmsrv/handlers/items/_MSG_DropItem.cpp:130`<br>`client/src/scene/TMFieldScene_Packets.cpp:111` |
| `0x17C` | servidor → cliente | `MSG_ShopList` (236) | `MSG_ShopList` (236) | `server/tmsrv/net/SendFunc.cpp:1377`<br>`client/src/scene/TMFieldScene_Packets.cpp:181` |
| `0x181` | servidor → cliente | `MSG_SetHpMp` (28) | `MSG_SetHpMp` (28) | `server/tmsrv/net/SendFunc.cpp:1596`<br>`client/src/entities/TMHuman_Packets.cpp:116` |
| `0x182` | servidor → cliente | `MSG_SendItem` (24) | `MSG_SendItem` (24) | `server/tmsrv/net/SendFunc.cpp:1068`<br>`client/src/entities/TMHuman_Packets.cpp:104` |
| `0x185` | servidor → cliente | `MSG_UpdateCarry` (528) | `MSG_Carry` (528) | `server/tmsrv/net/SendFunc.cpp:1542`<br>`client/src/entities/TMHuman_Packets.cpp:149` |
| `0x18A` | servidor → cliente | `MSG_SetHpDam` (20) | `MSG_SetHpDam` (20) | `server/tmsrv/systems/AffectSystem.cpp:1097`<br>`client/src/entities/TMHuman_Packets.cpp:119` |
| `0x18B` | servidor → cliente | `MSG_UpdateWeather` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/SendFunc.cpp:1558`<br>`client/src/scene/TMFieldScene_Packets.cpp:107` |
| `0x18D` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/handlers/social/_MSG_Challange.cpp:116`<br>`client/src/scene/TMFieldScene_Packets.cpp:141` |
| `0x193` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/handlers/world/Quest_Kingdom.cpp:567`<br>`client/src/entities/TMHuman_Packets.cpp:155` |
| `0x196` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/handlers/items/_MSG_Buy.cpp:62`<br>`client/src/scene/TMFieldScene_Packets.cpp:189` |
| `0x19C` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/handlers/trade/_MSG_TradingItem.cpp:50`<br>`client/src/scene/TMFieldScene_Packets.cpp:147` |
| `0x20D` | cliente → servidor | `MSG_AccountLogin` (120) | `MSG_AccountLogin` (120) | `client/src/scene/TMFieldScene_Packets.cpp:1238`<br>`server/tmsrv/net/ProcessClientMessage.cpp:68` |
| `0x20F` | cliente → servidor | `MSG_NewCharacter` (36) | `MSG_CreateCharacter` (36) | `client/src/scene/TMSelectCharScene.cpp:576`<br>`server/tmsrv/net/ProcessClientMessage.cpp:92` |
| `0x211` | cliente → servidor | `MSG_DeleteCharacter` (48) | `MSG_DeleteCharacter` (48) | `client/src/scene/TMSelectCharScene.cpp:716`<br>`server/tmsrv/net/ProcessClientMessage.cpp:80` |
| `0x213` | cliente → servidor | `MSG_CharacterLogin` (36) | `MSG_CharacterLogin` (36) | `client/src/scene/TMSelectCharScene.cpp:511`<br>`server/tmsrv/net/ProcessClientMessage.cpp:72` |
| `0x215` | cliente → servidor | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_FrameMove.cpp:1725`<br>`server/tmsrv/net/ProcessClientMessage.cpp:76` |
| `0x26E` | servidor → cliente | `MSG_CreateItem` (32) | `MSG_CreateItem` (32) | `server/tmsrv/core/ProcessSecMinTimer.cpp:811`<br>`client/src/scene/TMFieldScene_Packets.cpp:109` |
| `0x270` | cliente → servidor | `MSG_GetItem` (28) | `MSG_GetItem` (28) | `client/src/entities/TMHuman_Combat.cpp:498`<br>`server/tmsrv/net/ProcessClientMessage.cpp:210` |
| `0x272` | cliente → servidor | `MSG_DropItem` (32) | `MSG_DropItem` (32) | `client/src/scene/TMFieldScene_Items.cpp:77`<br>`server/tmsrv/net/ProcessClientMessage.cpp:206` |
| `0x277` | cliente → servidor | `MSG_ApplyBonus` (20) | `MSG_ApplyBonus` (20) | `client/src/scene/TMFieldScene_Controls.cpp:1807`<br>`server/tmsrv/net/ProcessClientMessage.cpp:222` |
| `0x27B` | cliente → servidor | `MSG_REQShopList` (16) | `MSG_REQShopList` (16) | `client/src/scene/TMFieldScene_Input.cpp:2231`<br>`server/tmsrv/net/ProcessClientMessage.cpp:144` |
| `0x289` | cliente → servidor | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_FrameMove.cpp:132`<br>`server/tmsrv/net/ProcessClientMessage.cpp:124` |
| `0x28B` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Input.cpp:2217`<br>`server/tmsrv/net/ProcessClientMessage.cpp:192` |
| `0x28C` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_MsgBox.cpp:201`<br>`server/tmsrv/net/ProcessClientMessage.cpp:128` |
| `0x28E` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Input.cpp:1074`<br>`server/tmsrv/net/ProcessClientMessage.cpp:132` |
| `0x28F` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:349`<br>`server/tmsrv/net/ProcessClientMessage.cpp:136` |
| `0x290` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_MsgBox.cpp:244`<br>`server/tmsrv/net/ProcessClientMessage.cpp:140` |
| `0x291` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_FrameMove.cpp:1025`<br>`server/tmsrv/net/ProcessClientMessage.cpp:176` |
| `0x292` | servidor → cliente | `MSG_SetHpMode` (20) | `MSG_SetHpMode` (20) | `server/tmsrv/net/SendFunc.cpp:1628`<br>`client/src/scene/TMFieldScene_Packets.cpp:138` |
| `0x2BC` | cliente → servidor | `MSG_MessageLog` (140) | `MSG_MessageLog` (140) | `client/src/scene/TMScene.cpp:802`<br>`server/tmsrv/net/ProcessClientMessage.cpp:88` |
| `0x2CD` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/ui/SGrid_Tooltip.cpp:305`<br>`server/tmsrv/net/ProcessClientMessage.cpp:311` |
| `0x2D4` | cliente → servidor | `MSG_Sell` (20) | `MSG_Sell` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:492`<br>`server/tmsrv/net/ProcessClientMessage.cpp:299` |
| `0x2E4` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_DeleteItem` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:404`<br>`server/tmsrv/net/ProcessClientMessage.cpp:279` |
| `0x2E5` | cliente → servidor | `MSG_STANDARDPARM3` (24) | `MSG_SplitItem` (24) | `client/src/scene/TMFieldScene_Controls.cpp:1038`<br>`server/tmsrv/net/ProcessClientMessage.cpp:287` |
| `0x333` | servidor → cliente | `MSG_MessageChat` (140) | `MSG_MessageChat` (140) | `server/tmsrv/net/SendFunc.cpp:1644`<br>`client/src/entities/TMHuman_Packets.cpp:122` |
| `0x333` | cliente → servidor | `MSG_MessageChat` (140) | `MSG_MessageChat` (140) | `client/src/scene/TMFieldScene_Controls.cpp:2557`<br>`server/tmsrv/net/ProcessClientMessage.cpp:100` |
| `0x334` | servidor → cliente | `MSG_MessageWhisper` (160) | `MSG_MessageWhisper` (160) | `server/tmsrv/net/ProcessClientMessage.cpp:172 (eco)`<br>`client/src/entities/TMHuman_Packets.cpp:131` |
| `0x334` | cliente → servidor | `MSG_MessageWhisper` (160) | `MSG_MessageWhisper` (160) | `client/src/entities/TMHuman_Packets.cpp:1141`<br>`server/tmsrv/net/ProcessClientMessage.cpp:172` |
| `0x338` | servidor → cliente | `MSG_CNFMobKill` (32) | `MSG_CNFMobKill` (32) | `server/tmsrv/systems/MobKilled.cpp:157`<br>`client/src/scene/TMFieldScene_Packets.cpp:126` |
| `0x339` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/SendFunc.cpp:1241`<br>`client/src/scene/TMFieldScene_Packets.cpp:105` |
| `0x366` | servidor → cliente | `MSG_Action` (52) | `MSG_STANDARD` (12) | `server/tmsrv/net/ProcessClientMessage.cpp:105 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:73` |
| `0x366` | cliente → servidor | `MSG_Action` (52) | `MSG_Action` (52) | `client/src/entities/TMHuman_Combat.cpp:1297`<br>`server/tmsrv/net/ProcessClientMessage.cpp:105` |
| `0x367` | servidor → cliente | `MSG_Attack` (168) | `MSG_Attack` (168) | `server/tmsrv/net/GetFunc.cpp:1584`<br>`client/src/scene/TMFieldScene_Packets.cpp:191` |
| `0x367` | cliente → servidor | `MSG_Attack` (168) | `MSG_Attack` (168) | `client/src/scene/TMFieldScene_AutoSkill.cpp:1076`<br>`server/tmsrv/net/ProcessClientMessage.cpp:200` |
| `0x368` | servidor → cliente | `MSG_Action` (52) | `MSG_Action` (52) | `server/tmsrv/handlers/combat/_MSG_Action.cpp:168`<br>`client/src/entities/TMHuman_Packets.cpp:74` |
| `0x368` | cliente → servidor | `MSG_Action` (52) | `MSG_Action` (52) | `client/src/scene/TMFieldScene_Combat.cpp:1465`<br>`server/tmsrv/net/ProcessClientMessage.cpp:106` |
| `0x369` | cliente → servidor | `MSG_REQMobByID` (16), `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_PacketAttack.cpp:708`<br>`server/tmsrv/net/ProcessClientMessage.cpp:120` |
| `0x36A` | servidor → cliente | `MSG_Motion` (20) | `MSG_Motion` (20) | `server/tmsrv/net/ProcessClientMessage.cpp:110 (eco)`<br>`client/src/entities/TMHuman_Packets.cpp:98` |
| `0x36A` | cliente → servidor | `MSG_Motion` (20) | `MSG_Motion` (20) | `client/src/scene/TMFieldScene_Input.cpp:2073`<br>`server/tmsrv/net/ProcessClientMessage.cpp:110` |
| `0x36B` | servidor → cliente | `MSG_UpdateEquip` (60) | `MSG_UpdateEquip` (60) | `server/tmsrv/net/SendFunc.cpp:1088`<br>`client/src/entities/TMHuman_Packets.cpp:107` |
| `0x36C` | servidor → cliente | `MSG_Action` (52) | `MSG_STANDARD` (12) | `server/tmsrv/net/GetFunc.cpp:1257`<br>`client/src/scene/TMFieldScene_Packets.cpp:72` |
| `0x36C` | cliente → servidor | `MSG_Action` (52) | `MSG_Action` (52) | `client/src/entities/TMHuman_Combat.cpp:1074`<br>`server/tmsrv/net/ProcessClientMessage.cpp:104` |
| `0x373` | cliente → servidor | `MSG_UseItem` (36) | `MSG_UseItem` (36) | `client/src/scene/TMFieldScene_Input.cpp:1371`<br>`server/tmsrv/net/ProcessClientMessage.cpp:218` |
| `0x374` | servidor → cliente | `MSG_UpdateItem` (20) | `MSG_UpdateItem` (20) | `server/tmsrv/net/ProcessClientMessage.cpp:188 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:115` |
| `0x374` | cliente → servidor | `MSG_UpdateItem` (20) | `MSG_UpdateItem` (20) | `client/src/scene/TMFieldScene_Items.cpp:185`<br>`server/tmsrv/net/ProcessClientMessage.cpp:188` |
| `0x376` | servidor → cliente | `MSG_TradingItem` (20) | `MSG_SwapItem` (20) | `server/tmsrv/net/ProcessClientMessage.cpp:168 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:179` |
| `0x376` | cliente → servidor | `MSG_SwapItem` (20) | `MSG_TradingItem` (20) | `client/src/ui/SGrid.cpp:464`<br>`server/tmsrv/net/ProcessClientMessage.cpp:168` |
| `0x378` | servidor → cliente | `MSG_SetShortSkill` (32) | `MSG_SetShortSkill` (32) | `server/tmsrv/handlers/items/UseItem_Quests.cpp:962`<br>`client/src/scene/TMFieldScene_Packets.cpp:144` |
| `0x378` | cliente → servidor | `MSG_SetShortSkill` (32) | `MSG_SetShortSkill` (32) | `client/src/entities/TMHuman_Packets.cpp:1838`<br>`server/tmsrv/net/ProcessClientMessage.cpp:196` |
| `0x379` | servidor → cliente | `MSG_Buy` (24) | `MSG_STANDARD` (12) | `server/tmsrv/net/ProcessClientMessage.cpp:234 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:123` |
| `0x379` | cliente → servidor | `MSG_Buy` (24) | `MSG_Buy` (24) | `client/src/ui/SGrid_Trade.cpp:82`<br>`server/tmsrv/net/ProcessClientMessage.cpp:234` |
| `0x37A` | servidor → cliente | `MSG_Sell` (20) | `MSG_Sell` (20) | `server/tmsrv/net/ProcessClientMessage.cpp:238 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:183` |
| `0x37A` | cliente → servidor | `MSG_Sell` (20) | `MSG_Sell` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:435`<br>`server/tmsrv/net/ProcessClientMessage.cpp:238` |
| `0x37D` | servidor → cliente | `MSG_CNFAddParty` (40) | `MSG_AddParty` (40) | `server/tmsrv/net/SendFunc.cpp:1481`<br>`client/src/scene/TMFieldScene_Packets.cpp:132` |
| `0x37E` | servidor → cliente | `MSG_RemoveParty` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/SendFunc.cpp:1517`<br>`client/src/scene/TMFieldScene_Packets.cpp:135` |
| `0x37E` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:3042`<br>`server/tmsrv/net/ProcessClientMessage.cpp:156` |
| `0x37F` | servidor → cliente | `MSG_SendReqParty` (44) | `MSG_REQParty` (44) | `server/tmsrv/net/SendFunc.cpp:1447`<br>`client/src/scene/TMFieldScene_Packets.cpp:129` |
| `0x37F` | cliente → servidor | `MSG_REQParty` (44) | `MSG_SendReqParty` (44) | `client/src/scene/TMFieldScene_Controls.cpp:1595`<br>`server/tmsrv/net/ProcessClientMessage.cpp:160` |
| `0x383` | servidor → cliente | `MSG_Trade` (156) | `MSG_Trade` (156) | `server/tmsrv/net/ProcessClientMessage.cpp:242 (eco)`<br>`client/src/entities/TMHuman_Packets.cpp:143` |
| `0x383` | cliente → servidor | `MSG_Trade` (156) | `MSG_Trade` (156) | `client/src/scene/TMFieldScene_Controls.cpp:3066`<br>`server/tmsrv/net/ProcessClientMessage.cpp:242` |
| `0x384` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/core/Server.cpp:1208`<br>`client/src/entities/TMHuman_Packets.cpp:146` |
| `0x384` | cliente → servidor | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `client/src/scene/TMFieldScene_Panels.cpp:1361`<br>`server/tmsrv/net/ProcessClientMessage.cpp:214` |
| `0x386` | servidor → cliente | `MSG_STANDARD` (12) | `MSG_STANDARD` (12) | `server/tmsrv/handlers/trade/_MSG_Trade.cpp:242`<br>`client/src/entities/TMHuman_Packets.cpp:152` |
| `0x387` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/ProcessClientMessage.cpp:152 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:187` |
| `0x387` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:783`<br>`server/tmsrv/net/ProcessClientMessage.cpp:152` |
| `0x388` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/ProcessClientMessage.cpp:148 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:185` |
| `0x388` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Controls.cpp:793`<br>`server/tmsrv/net/ProcessClientMessage.cpp:148` |
| `0x397` | servidor → cliente | `MSG_SendAutoTrade` (196) | `MSG_AutoTrade` (196) | `server/tmsrv/net/SendFunc.cpp:552`<br>`client/src/scene/TMFieldScene_Packets.cpp:177` |
| `0x398` | cliente → servidor | `MSG_ReqBuy` (36) | `MSG_ReqBuy` (36) | `client/src/scene/TMFieldScene_Items.cpp:127`<br>`server/tmsrv/net/ProcessClientMessage.cpp:230` |
| `0x399` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_HUD.cpp:1125`<br>`server/tmsrv/net/ProcessClientMessage.cpp:180` |
| `0x39A` | cliente → servidor | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `client/src/scene/TMFieldScene_Input.cpp:933`<br>`server/tmsrv/net/ProcessClientMessage.cpp:184` |
| `0x39B` | servidor → cliente | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `server/tmsrv/handlers/trade/_MSG_ReqBuy.cpp:185`<br>`client/src/scene/TMFieldScene_Packets.cpp:103` |
| `0x39D` | servidor → cliente | `MSG_Attack` (168), `MSG_AttackOne` (72) | `MSG_Attack` (168) | `server/tmsrv/net/GetFunc.cpp:1300`<br>`client/src/scene/TMFieldScene_Packets.cpp:192` |
| `0x39D` | cliente → servidor | `MSG_Attack` (168), `MSG_AttackOne` (72) | `MSG_Attack` (168) | `client/src/entities/TMHuman_Combat.cpp:1896`<br>`server/tmsrv/net/ProcessClientMessage.cpp:201` |
| `0x39E` | servidor → cliente | `MSG_Attack` (168) | `MSG_Attack` (168) | `server/tmsrv/net/ProcessClientMessage.cpp:202 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:193` |
| `0x39E` | cliente → servidor | `MSG_Attack` (168) | `MSG_Attack` (168) | `client/src/entities/TMHuman_Combat.cpp:1988`<br>`server/tmsrv/net/ProcessClientMessage.cpp:202` |
| `0x39F` | servidor → cliente | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `server/tmsrv/net/ProcessClientMessage.cpp:250 (eco)`<br>`client/src/entities/TMHuman_Packets.cpp:158` |
| `0x39F` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Controls.cpp:1747`<br>`server/tmsrv/net/ProcessClientMessage.cpp:250` |
| `0x3A1` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/core/ProcessSecMinTimer.cpp:439`<br>`client/src/scene/TMFieldScene_Packets.cpp:156` |
| `0x3A2` | servidor → cliente | `MSG_EnvEffect` (24) | `MSG_EnvEffect` (24) | `server/tmsrv/net/SendFunc.cpp:353`<br>`client/src/scene/TMFieldScene_Packets.cpp:168` |
| `0x3A3` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/handlers/items/UseItem_Scrolls.cpp:230`<br>`client/src/scene/TMFieldScene_Packets.cpp:91` |
| `0x3A4` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/handlers/social/_MSG_MessageChat.cpp:45`<br>`client/src/scene/TMFieldScene_Packets.cpp:165` |
| `0x3A7` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARD` (12) | `server/tmsrv/handlers/combine/_MSG_CombineItem.cpp:109`<br>`client/src/scene/TMFieldScene_Packets.cpp:150` |
| `0x3A8` | servidor → cliente | `MSG_STANDARDPARM3` (24) | `MSG_STANDARDPARM3` (24) | `server/tmsrv/net/SendFunc.cpp:1429`<br>`client/src/scene/TMFieldScene_Packets.cpp:162` |
| `0x3AB` | cliente → servidor | `MSG_CNFParty2` (32) | `MSG_AcceptParty` (32) | `client/src/scene/TMFieldScene_Controls.cpp:1965`<br>`server/tmsrv/net/ProcessClientMessage.cpp:164` |
| `0x3AC` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/net/ProcessDBMessage.cpp:1004`<br>`client/src/scene/TMFieldScene_Packets.cpp:153` |
| `0x3AD` | servidor → cliente | `MSG_STANDARDPARM` (16) | `MSG_STANDARDPARM` (16) | `server/tmsrv/systems/AffectSystem.cpp:73`<br>`client/src/entities/TMHuman_Packets.cpp:161` |
| `0x3B9` | servidor → cliente | `MSG_SendAffect` (268) | `MSG_UpdateAffect` (268) | `server/tmsrv/net/SendFunc.cpp:1770`<br>`client/src/entities/TMHuman_Packets.cpp:110` |
| `0x3D5` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_Controls.cpp:1685`<br>`server/tmsrv/net/ProcessClientMessage.cpp:283` |
| `0x52A` | servidor → cliente | `MSG_DBCNFServerChange` (80) | `MSG_CNFRemoveServer` (80) | `server/tmsrv/net/ProcessDBMessage.cpp:1311 (eco)`<br>`client/src/scene/TMFieldScene_Packets.cpp:95` |
| `0xD1D` | servidor → cliente | `MSG_MagicTrumpet` (160) | `MSG_MessageWhisper` (160) | `server/tmsrv/handlers/social/_MSG_MessageWhisper.cpp:132`<br>`client/src/scene/TMScene_Packets.cpp:432` |
| `0xE0E` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:212`<br>`server/tmsrv/net/ProcessClientMessage.cpp:307` |
| `0xE12` | cliente → servidor | `MSG_STANDARDPARM2` (20) | `MSG_GuildAlly` (20), `MSG_STANDARDPARM2` (20) | `client/src/scene/TMFieldScene_MsgBox.cpp:224`<br>`server/tmsrv/net/ProcessClientMessage.cpp:303` |
| `0xFAA` | servidor → cliente | `MSG_ReqTransper` (52) | `MSG_ReqTransper` (52) | `server/tmsrv/net/ProcessClientMessage.cpp:84 (eco)`<br>`client/src/scene/TMSelectCharScene.cpp:1465` |
| `0xFAA` | cliente → servidor | `MSG_ReqTransper` (52) | `MSG_ReqTransper` (52) | `client/src/scene/TMSelectCharScene.cpp:694`<br>`server/tmsrv/net/ProcessClientMessage.cpp:84` |
| `0xFDE` | servidor → cliente | `MSG_AccountSecure` (32), `MSG_STANDARD` (12) | `MSG_CHARPASSWORD` (32) | `server/tmsrv/net/ProcessClientMessage.cpp:96 (eco)`<br>`client/src/scene/TMSelectCharScene.cpp:1370` |
| `0xFDE` | cliente → servidor | `MSG_CHARPASSWORD` (32) | `MSG_AccountSecure` (32), `MSG_STANDARD` (12) | `client/src/scene/TMSelectCharScene.cpp:415`<br>`server/tmsrv/net/ProcessClientMessage.cpp:96` |


</details>

## Structs de jogo e de arquivo

Structs de mesmo nome fora dos pacotes. Quando mudam, mudam também os pacotes que as carregam e os
arquivos que as gravam (contas, `.bin`). `A → B` é um campo que mudou de nome no mesmo offset;
`offset+tamanho` em bytes.

| Struct | Servidor | Cliente | Diferenças |
|---|---|---|---|
| `STRUCT_ACCOUNTFILE` | 8640 | 8000 | `Info` só no servidor (0+220); `Char` 224+4096 → 216+4096; `Cargo` 4320+1024 → 4312+1024; `Coin` 5344+4 → 5336+4; `ShortSkill` 5348+64 → 5340+64; `affect` só no servidor (5412+1024); `mobExtra` só no servidor (6440+2144); `Donate` só no servidor (8584+4); `TempKey` só no servidor (8588+52); `Account` só no cliente (0+212); `Ext1` só no cliente (5404+1152); `Ext2` só no cliente (6560+1440) |
| `STRUCT_GUILDZONE` | 84 | 84 | `GuildSpawnX` → `GuildStartX`; `GuildSpawnY` → `GuildStartY`; `CitySpawnX` só no servidor (16+4); `CitySpawnY` só no servidor (20+4); `CityLimitX1` → `vx1`; `CityLimitY1` → `vy1`; `CityLimitX2` → `vx2`; `CityLimitY2` → `vy2`; `WarAreaX1` → `ax1`; `WarAreaY1` → `ay1`; `WarAreaX2` → `ax2`; `WarAreaY2` → `ay2`; `ChargeWarSpawnX` → `hx`; `ChargeWarSpawnY` → `hy`; `ChallangeWarSpawnX` → `gx`; `ChallangeWarSpawnY` → `gy`; `CityTax` → `Tax`; `Clan` → `ChargeClan`; `Victory` → `ChargeCount`; `Start` só no cliente (16+8) |

Idênticas (mesmo tamanho, mesmos campos nos mesmos offsets): `STRUCT_AFFECT`, `STRUCT_BEASTBONUS`, `STRUCT_DAM`, `STRUCT_INITITEM`, `STRUCT_ITEM`, `STRUCT_ITEMLIST`, `STRUCT_MOB`, `STRUCT_SCORE`, `STRUCT_SELCHAR`, `STRUCT_SPELL`, `STRUCT_STATICEFFECT`, `STRUCT_TREASURE`.

## Regras `BASE_*`

Chamadas com a mesma entrada nos dois lados. O exemplo é a primeira entrada em que as saídas diferem.

| Função | Chamadas | Divergem | Exemplo: entrada → servidor / cliente | Observação |
|---|---|---|---|---|
| `BASE_ApplyAttribute` | 16 | 0 |  |  |
| `BASE_CanCargo` | 3000 | 0 |  |  |
| `BASE_CanEquip` | 3000 | 0 |  |  |
| `BASE_CanTrade` | 3000 | 0 |  |  |
| `BASE_CheckValidString` | 12000 | 0 |  |  |
| `BASE_ClearItem` | 12000 | 0 |  |  |
| `BASE_GetBonusItemAbility` | 384000 | 0 |  |  |
| `BASE_GetBonusItemAbilityNosanc` | 384000 | 0 |  |  |
| `BASE_GetDistance` | 12000 | 0 |  |  |
| `BASE_GetDoubleCritical` | 3000 | 0 |  | STRUCT_MOB preenchida campo a campo pelo nome |
| `BASE_GetItemAbility` | 384000 | 0 |  |  |
| `BASE_GetItemAbilityNosanc` | 384000 | 0 |  |  |
| `BASE_GetItemAmount` | 12000 | 0 |  |  |
| `BASE_GetItemSanc` | 12000 | 0 |  |  |
| `BASE_GetItemSancSuccess` | 12000 | 0 |  |  |
| `BASE_GetManaSpent` | 3000 | 0 |  |  |
| `BASE_GetMaxAbility` | 384000 | 0 |  | STRUCT_MOB preenchida campo a campo pelo nome |
| `BASE_GetMobAbility` | 384000 | 0 |  | STRUCT_MOB preenchida campo a campo pelo nome |
| `BASE_GetRoute` | 12000 | 0 |  |  |
| `BASE_GetSkillDamage` | 3000 | 0 |  |  |
| `BASE_GetSpeed` | 3000 | 0 |  |  |
| `BASE_GetStaticItemAbility` | 384000 | 0 |  |  |
| `BASE_GetSum` | 12000 | 0 |  |  |
| `BASE_GetSum2` | 12000 | 0 |  |  |
| `BASE_GetVillage` | 12000 | 0 |  |  |
| `BASE_SetItemAmount` | 12000 | 0 |  |  |
| `BASE_SortTradeItem` | 3000 | 0 |  |  |
| `BASE_UnderBarToSpace` | 12000 | 0 |  |  |
| `BASE_UpdateItem2` | 3000 | 0 |  |  |

Em comum, mas fora da comparação (leem arquivo, rede ou relógio, ou os parâmetros não têm
correspondência): `BASE_CanCarry`, `BASE_ChangeOrAddEffectValue`, `BASE_GetHttpRequest`, `BASE_GetSancEffValue`, `BASE_GetWeekNumber`, `BASE_HasSancAdd`, `BASE_InitModuleDir`, `BASE_InitializeAttribute`, `BASE_InitializeBaseDef`, `BASE_InitializeHitRate`, `BASE_InitializeServerList`, `BASE_ReadInitItem`, `BASE_ReadItemList`, `BASE_ReadMessageBin`, `BASE_ReadSkillBin`, `BASE_RemoveEffect`.

## Só de um lado

- Funções `BASE_*` só no cliente (26): `BASE_CanEquip_RecvRes`, `BASE_CanRefine`, `BASE_CheckChatValid`, `BASE_DefineSkinMeshType`, `BASE_Get3DTo2DPos`, `BASE_GetAttr`, `BASE_GetAttribute`, `BASE_GetColorCount`, `BASE_GetEffectValue`, `BASE_GetHitPosition`, `BASE_GetHitPosition2`, `BASE_GetItemColor`, `BASE_GetItemColorEffect`, `BASE_GetItemTenColor`, `BASE_GetMeshIndex`, `BASE_GetMountScale`, `BASE_GetOptionColor`, `BASE_GetSubGuild`, `BASE_InitEffectString`, `BASE_InitialItemRePrice`, `BASE_IsInLowZone`, `BASE_ReadItemPrice`, `BASE_ReadTOTOList`, `BASE_ScreenResize`, `BASE_SetBit`, `BASE_TransCurse`.
- Funções `BASE_*` só no servidor (52): `BASE_CarryUsable`, `BASE_CheckFairyDate`, `BASE_CheckHangul`, `BASE_CheckItemDate`, `BASE_CheckPacket`, `BASE_ClearMob`, `BASE_ClearMobExtra`, `BASE_GetAccuracyRate`, `BASE_GetArena`, `BASE_GetBonusScorePoint`, `BASE_GetBonusSkillPoint`, `BASE_GetClientGuildName`, `BASE_GetCurrentScore`, `BASE_GetDamage`, `BASE_GetDamageRate`, `BASE_GetDestByAction`, `BASE_GetEnglish`, `BASE_GetFirstKey`, `BASE_GetGrowthRate`, `BASE_GetGuild`, `BASE_GetGuildName`, `BASE_GetHitRate`, `BASE_GetHpMp`, `BASE_GetIndex`, `BASE_GetItemCode`, `BASE_GetItemGem`, `BASE_GetKorFirst`, `BASE_GetLanguage`, `BASE_GetMobCheckSum`, `BASE_GetSubGuildByMedal`, `BASE_GetSuccessRate`, `BASE_InitializeClientGuildName`, `BASE_InitializeEffectName`, `BASE_InitializeGuildName`, `BASE_InitializeInitItem`, `BASE_InitializeItemList`, `BASE_InitializeMessage`, `BASE_InitializeMobname`, `BASE_InitializeSkill`, `BASE_NeedLog`, `BASE_NextCarry`, `BASE_ReadItemListFile`, `BASE_SetItemDate`, `BASE_SetItemSanc`, `BASE_SpaceToUnderBar`, `BASE_UpdateItem`, `BASE_VisualAnctCode`, `BASE_VisualItemCode`, `BASE_WriteInitItem`, `BASE_WriteItemList`, `BASE_WriteMessageBin`, `BASE_WriteSkillBin`.
- Opcodes que o servidor envia e o cliente não trata (23): `0x101`, `0x102`, `0x117`, `0x119`, `0x11A`, `0x11B`, `0x11C`, `0x11D`, `0x166`, `0x194`, `0x20D`, `0x20F`, `0x211`, `0x213`, `0x2CD`, `0x3A5`, `0x3AB`, `0x7A9`, `0xE0E`, `0xE12`, `0xE13`, `0xFDF`, `0x4501`.
- Opcodes que o servidor lê e o cliente não envia (19): `0x2C3`, `0x2C4`, `0x2D2`, `0x2D3`, `0x2E1`, `0x2E2`, `0x338`, `0x364`, `0x397`, `0x3A6`, `0x3B5`, `0x3BA`, `0x3C0`, `0x3CC`, `0xD1D`, `0xDC3`, `0xE13`, `0xFDF`, `0x4501`.

