# Pacotes de um lado só: triagem

Triagem manual, opcode por opcode, dos pacotes que o [`DIVERGENCIAS.md`](DIVERGENCIAS.md) lista como
usados por um lado só (39 direções só do cliente e 47 só do servidor, no relatório gerado antes de
`tools/divergence_map.py` aceitar opcodes decimais). Cada linha foi conferida no código. Diferente do
relatório, este arquivo é escrito à mão: ao mudar um destes pacotes, atualize a linha.

## Resumo

| | A: recurso de um lado só | B: o outro lado tem, de outro jeito | C: sem uso ou entre servidores | D: outro |
|---|---|---|---|---|
| Só do cliente (39) | 19 | 6 | 11 | 3 |
| Só do servidor (47, mais o 0x194) | 3 | 19 | 12 | 14 |

- **Recursos do cliente que faltam no servidor (A, 19)**: quiz aleatório, preço dinâmico dos itens
  412/413, saldo e loja de cash, log de erro do cliente, jackpot, saída com atraso (`SysQuit`),
  convocação, fogos premium, contador da quest 12, recompra de itens vendidos ao NPC, painel de 4
  linhas, voo por rota e o chat de grupo por índice. Implementar cada um é trabalho de jogabilidade.
- **Quase todo B é limite da análise de texto**: opcodes em decimal (`Type = 652;`), `Type` atribuído
  num membro declarado em outro arquivo, `case` sem cast. Os decimais nos envios já são reconhecidos;
  com isso o relatório passou a casar 98 direções (eram 90).
- **Corrigido a partir desta triagem**:
  - O TMSrv repassava o resultado da transferência de personagem (0xFAA) como `(char*)&m`, sendo `m` já
    um ponteiro; o cliente recebia lixo da pilha.
  - A extração (0x2D4) passou a ler `MSG_Sell.MyPos`, o campo que o cliente preenche, em vez de um
    `int` que só dava certo pelo preenchimento zerado.
- **Resolvido depois da triagem**:
  - Cápsula: o DBSrv responde 0xDC3 no layout de `MSG_CAPSULEINFO` do cliente (rosto, nível, atributos
    e maestrias; as 9 skills e os bits de quest ficam 0, sem correspondência no personagem salvo). O
    0xD1F com `STRUCT_SEALOFSOUL` saiu.
  - O TMSrv repassa ao DBSrv o pedido de transferência de personagem (0xFAA), no modo de transferência.
  - Placar do RvR (0x3BB): o servidor manda os pontos crus, como o cliente decodifica (o vermelho em
    módulo 256, porque o cliente só lê o byte baixo).
  - O log de erro do cliente (0x2BC) vai para o log do servidor.
- **Ainda em aberto**:
  - Os outros recursos A do cliente (quiz, jackpot, loja de cash, preço dinâmico, saída com atraso,
    convocação, fogos premium, contador da quest 12, recompra, painel de 4 linhas, voo por rota, chat
    de grupo por índice): cada um é desenho de jogo no servidor.
  - Os 8 combinadores marcados D (Lindy, Shany, Odin, Odin2, Ehre, Alquimia, Ailyn, Agatha) recebem do
    cliente o opcode que vem do `Mixlist.bin`, que não está no repositório. No `Mixlist.bin` do WYD
    Global (patch 749) os 8 opcodes aparecem nas receitas, mas o registro de lá tem 2664 bytes contra
    os 2656 do `STRUCT_RESULT_ITEMLIST` do cliente: é outra versão, e o arquivo da nossa continua faltando.

## Só do cliente

| Opcode | Direção | Cat. | O que é | Evidência |
|---|---|---|---|---|
| `0x105` | s→c | A | Chat de grupo de um membro fora da visão, com o texto pela tabela de mensagens. O handler reconstruído monta o texto e imprime outro campo. | `client/src/scene/TMFieldScene_Packets.cpp:256` |
| `0x106` | s→c | C | "MessageChat_Param": os dois handlers só fazem `return 0`. | `client/src/scene/TMFieldScene_Packets.cpp:305`, `client/src/entities/TMHuman_Packets.cpp:1668` |
| `0x19C` | s→c | B | Fecha o painel de carga quando o servidor recusa uma troca com a carga (sinal 412). | `client/src/scene/TMFieldScene_Packets.cpp:1660`; `server/tmsrv/handlers/trade/_MSG_TradingItem.cpp:50` |
| `0x1BF` | s→c | C | "RESULTGAMBLE": handler vazio. | `client/src/scene/TMFieldScene_PacketItems.cpp:975` |
| `0x1C1` | s→c | C | "REQArray": handler vazio. | `client/src/scene/TMFieldScene_Packets.cpp:1774` |
| `0x1C2` | s→c | A | Preço dinâmico dos itens 412/413. | `client/src/scene/TMFieldScene_PacketItems.cpp:980` |
| `0x1C5` | s→c | C | "Nuke": o handler só faz `return 1`. | `client/src/scene/TMFieldScene_Packets.cpp:1779` |
| `0x1C6` | s→c | A | Quiz aleatório: pergunta e 4 respostas. | `client/src/scene/TMFieldScene_Packets.cpp:1785` |
| `0x1CF` | s→c | A | Saldo de cash no HUD. | `client/src/entities/TMHuman_Packets.cpp:1881` |
| `0x1D0` | s→c | A | Loja de cash (reaproveita a lista da loja comum). | `client/src/scene/TMFieldScene_Packets.cpp:119` |
| `0x1D6` | s→c | C | Nomes de guilda na seleção de personagem: gravados, nunca lidos. | `client/src/scene/TMSelectCharScene.cpp:1556` |
| `0x2BC` | c→s | A → resolvido | Log de erro crítico do cliente (falha ao carregar mapa ou malha); o servidor passou a registrá-lo. | `client/src/scene/TMScene.cpp:798` |
| `0x2BE` | c→s | A | Aposta no jackpot. | `client/src/scene/TMFieldScene_Controls.cpp:187` |
| `0x2C7` | c→s | A | Resposta do quiz (par do 0x1C6). | `client/src/scene/TMFieldScene_UI.cpp:562` |
| `0x2C8` | s→c | C | "AutoKick": handler vazio. | `client/src/scene/TMFieldScene_Packets.cpp:1809` |
| `0x384` | s→c | B | Fim da troca: o servidor manda pelo sinal 900. | `client/src/entities/TMHuman_Packets.cpp:1984`; `server/tmsrv/core/Server.cpp:1208` |
| `0x3A0` | c→s | B | Ping; o servidor só atualiza o último recebimento. | `client/src/scene/TMFieldScene.cpp:82`; `server/tmsrv/net/ProcessClientMessage.cpp:59` |
| `0x3AE` | s→c | A | O servidor autoriza a saída com atraso e o cliente fecha. | `client/src/scene/TMFieldScene_Packets.cpp:1846` |
| `0x3AE` | c→s | A | Pedido de saída (sair, trocar de servidor ou personagem). | `client/src/scene/TMFieldScene_Controls.cpp:1561` |
| `0x3AF` | s→c | D | Só o gold; o servidor manda o gold dentro de 0x337 e 0x185. | `client/src/entities/TMHuman_Packets.cpp:1869`; `server/tmsrv/net/SendFunc.cpp:1218` |
| `0x3B0` | s→c | B | Mobs restantes (salas secretas, pista de runas). | `client/src/scene/TMFieldScene_Packets.cpp:1693`; `server/tmsrv/core/ProcessSecMinTimer.cpp:246` |
| `0x3B2` | s→c | A | Pedido de convocação, que o alvo aceita num painel. | `client/src/scene/TMFieldScene_Packets.cpp:471` |
| `0x3B3` | s→c | A | Cancela a convocação. | `client/src/scene/TMFieldScene_Packets.cpp:481` |
| `0x3BB` | s→c | B | Placar do RvR; o servidor passou a mandar os pontos sem o +1024. | `client/src/scene/TMFieldScene_Packets.cpp:1763`; `server/tmsrv/core/ProcessSecMinTimer.cpp:258` |
| `0x3C9` | c→s | A | Fogos premium com padrão escolhido. | `client/src/scene/TMFieldScene_Items.cpp:613` |
| `0x3CA` | s→c | C | "PremiumFireWork": handler vazio. | `client/src/entities/TMHuman_Packets.cpp:628` |
| `0x3CF` | s→c | A | Início do contador de mobs da quest 12. | `client/src/scene/TMFieldScene_Packets.cpp:1814` |
| `0x3D0` | s→c | A | Progresso do contador da quest 12. | `client/src/scene/TMFieldScene_Packets.cpp:1834` |
| `0x3E8` | s→c | A | Lista de recompra dos itens vendidos ao NPC. | `client/src/scene/TMFieldScene_PacketItems.cpp:1019` |
| `0x3E8` | c→s | A | Pedido da lista de recompra. | `client/src/ui/SGrid_Trade.cpp:27` |
| `0x7B1` | s→c | A | Painel de 4 linhas (quiz ou aviso de evento). | `client/src/scene/TMFieldScene_Packets.cpp:435` |
| `0x7DB` | s→c | C | Tempo de jogo do controle anti-vício: gravado, nunca lido. | `client/src/scene/TMScene_Packets.cpp:469` |
| `0xAD9` | c→s | A | Início e fim do voo por rota. | `client/src/scene/TMFieldScene.cpp:423` |
| `0xBFF` | s→c | C | Desafio de "Encode": os três handlers de destino estão vazios. | `client/src/scene/TMScene_Packets.cpp:74` |
| `0xDC3` | s→c | D → resolvido | Dados da cápsula no tooltip; o DBSrv passou a responder neste opcode e layout. | `client/src/scene/TMFieldScene_PacketItems.cpp:987`; `server/dbsrv/persistence/CFileDB_AccountPackets.cpp` |
| `0xED7` | c→s | C | Declarar guerra de servidor; o painel nunca é aberto. | `client/src/scene/TMFieldScene_Controls.cpp:993` |
| `0xED8` | c→s | C | Recusar guerra de servidor; o painel nunca é aberto. | `client/src/scene/TMFieldScene_Controls.cpp:1003` |
| `0xFAA` | s→c | B | Resultado da transferência de personagem, gerado no DBSrv e repassado pelo TMSrv (repasse corrigido). | `client/src/scene/TMSelectCharScene.cpp:1465`; `server/tmsrv/net/ProcessDBMessage.cpp:213` |
| `0xFAA` | c→s | D → resolvido | Pedido de transferência; o TMSrv passou a repassá-lo ao DBSrv (`handlers/account/_MSG_ReqTransper.cpp`). | `client/src/scene/TMSelectCharScene.cpp:694`; `server/dbsrv/persistence/CFileDB_WorldPackets.cpp:60` |

Os handlers vazios (C) podem ser lacuna da reconstrução, e não recurso morto no jogo original.

## Só do servidor

| Opcode | Direção | Nome no servidor | Cat. | O que é | Evidência |
|---|---|---|---|---|---|
| `0x101` | s→c | `_MSG_MessagePanel` | B | Texto no painel; o cliente trata num `if` com outros tipos. | `client/src/scene/TMScene_Packets.cpp:111` |
| `0x102` | s→c | `_MSG_MessageBoxOk` | D | Sem chamador no servidor. | `server/tmsrv/net/SendFunc.cpp:179` |
| `0x117` | s→c | `_MSG_NewAccountFail` | D | Só responde a um pedido que ninguém envia. | `server/tmsrv/net/ProcessDBMessage.cpp:481` |
| `0x119` | s→c | `_MSG_CharacterLoginFail` | B | Falha ao entrar com o personagem (`case` sem cast). | `client/src/scene/TMSelectCharScene.cpp:1454` |
| `0x11A` | s→c | `_MSG_NewCharacterFail` | B | Falha ao criar personagem. | `client/src/scene/TMSelectCharScene.cpp:1410` |
| `0x11B` | s→c | `_MSG_DeleteCharacterFail` | B | Falha ao apagar personagem. | `client/src/scene/TMSelectCharScene.cpp:1425` |
| `0x11C` | s→c | `_MSG_AlreadyPlaying` | B | A conta já está em jogo. | `client/src/scene/TMSelectServerScene.cpp:853` |
| `0x11D` | s→c | `_MSG_StillPlaying` | B | A conta ainda está conectada. | `client/src/scene/TMSelectServerScene.cpp:853` |
| `0x166` | s→c | `_MSG_PKInfo` | A | Estado PK de um jogador; o cliente não trata. | `server/tmsrv/net/SendFunc.cpp:1741` |
| `0x194` | s→c | (sinal 404) | B | Conta sem permissão para entrar; o cliente testa `Type == 0x194` sem cast. Apareceu na lista com os decimais. | `client/src/scene/TMScene_Packets.cpp:66`; `server/tmsrv/handlers/account/_MSG_CharacterLogin.cpp:41` |
| `0x20D` | s→c | `_MSG_AccountLogin` | C | Repasse do login ao DBSrv. | `server/tmsrv/handlers/account/_MSG_AccountLogin.cpp:92` |
| `0x20F` | s→c | `_MSG_CreateCharacter` | C | Repasse ao DBSrv. | `server/tmsrv/handlers/account/_MSG_CreateCharacter.cpp:43` |
| `0x211` | s→c | `_MSG_DeleteCharacter` | C | Repasse ao DBSrv. | `server/tmsrv/handlers/account/_MSG_DeleteCharacter.cpp:33` |
| `0x213` | s→c | `_MSG_CharacterLogin` | C | Repasse ao DBSrv. | `server/tmsrv/handlers/account/_MSG_CharacterLogin.cpp:81` |
| `0x2CD` | s→c | `_MSG_CapsuleInfo` | D | Não sai para o cliente: o handler troca o tipo e manda ao DBSrv. | `server/tmsrv/handlers/items/_MSG_CapsuleInfo.cpp:28` |
| `0x3A5` | s→c | `_MSG_GuildBoard` | A | Quadro da guilda ao usar o item; o cliente não trata. | `server/tmsrv/handlers/items/UseItem_Scrolls.cpp:381` |
| `0x3AB` | s→c | `_MSG_AcceptParty` | D | Não vai à rede: o servidor injeta o pacote em si mesmo ao trocar o líder do grupo. | `server/tmsrv/systems/WorldEventSystem.cpp:1565` |
| `0x7A9` | s→c | `_MSG_TransperCharacter` | B | Aviso de modo de transferência; o cliente liga `g_bMoveServer`. | `client/src/scene/TMSelectCharScene.cpp:1460` |
| `0xE0E` | s→c | `_MSG_War` | C | Declaração de guerra enviada ao DBSrv. | `server/tmsrv/handlers/social/_MSG_War.cpp:40` |
| `0xE12` | s→c | `_MSG_GuildAlly` | C | Aliança enviada ao DBSrv. | `server/tmsrv/handlers/social/_MSG_GuildAlly.cpp:40` |
| `0xE13` | s→c | `_MSG_GuildInfo` | C | Dados da guilda enviados ao DBSrv. | `server/tmsrv/systems/WorldEventSystem.cpp:125` |
| `0xFDF` | s→c | `_MSG_AccountSecureFail` | B | Senha numérica errada; o cliente reabre o diálogo. | `client/src/scene/TMSelectCharScene.cpp:1392` |
| `0x4501` | s→c | `_MSG_GrindRankingData` | A | Ranking de EXP próprio do W2PP. | `server/tmsrv/net/ProcessDBMessage.cpp:1287` |
| `0x28C` | c→s | `_MSG_Deprivate` | B | Expulsar da guilda (decimal 652). | `client/src/scene/TMFieldScene_MsgBox.cpp:201` |
| `0x28F` | c→s | `_MSG_ChallangeConfirm` | B | Confirmar desafio (655). | `client/src/scene/TMFieldScene_MsgBox.cpp:349` |
| `0x290` | c→s | `_MSG_ReqTeleport` | B | Pedido de teleporte (656). | `client/src/scene/TMFieldScene_MsgBox.cpp:244` |
| `0x2C3` | c→s | `_MSG_CombineItemLindy` | D | Opcode vindo do `Mixlist.bin` (fora do repositório). | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x2C4` | c→s | `_MSG_CombineItemShany` | D | Idem. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x2D2` | c→s | `_MSG_CombineItemOdin` | D | Idem. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x2D3` | c→s | `_MSG_CombineItemEhre` | D | Idem. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x2D4` | c→s | `_MSG_CombineItemExtracao` | B | Extração (724, `MSG_Sell`); o servidor passou a ler `MyPos`. | `client/src/scene/TMFieldScene_MsgBox.cpp:492` |
| `0x2E1` | c→s | `_MSG_CombineItemAlquimia` | D | Opcode do `Mixlist.bin`. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x2E2` | c→s | `_MSG_CombineItemOdin2` | D | Idem. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x338` | c→s | `_MSG_CNFMobKill` | D | Falso positivo: o servidor ajusta o próprio pacote de saída. | `server/tmsrv/net/SendFunc.cpp:919` |
| `0x364` | c→s | `_MSG_CreateMob` | D | Falso positivo, mesmo caso. | `server/tmsrv/net/SendFunc.cpp:892` |
| `0x397` | c→s | `_MSG_SendAutoTrade` | B | Abrir a loja pessoal (919, num membro declarado no header). | `client/src/scene/TMFieldScene_Controls.cpp:2142` |
| `0x3A6` | c→s | `_MSG_CombineItem` | B | Compositor; `Type` num membro do ObjectManager. | `client/src/scene/TMFieldScene_Panels.cpp:473` |
| `0x3B5` | c→s | `_MSG_CombineItemAilyn` | D | Opcode do `Mixlist.bin`. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x3BA` | c→s | `_MSG_CombineItemAgatha` | D | Idem. | `client/src/gameplay/MrItemMix.cpp:893` |
| `0x3C0` | c→s | `_MSG_CombineItemTiny` | B | Combinação do Tiny; `Type` num membro do ObjectManager. | `client/src/scene/TMFieldScene_Panels.cpp:519` |
| `0x3CC` | c→s | `_MSG_PutoutSeal` | B | Tirar o personagem da cápsula (972). | `client/src/scene/TMFieldScene_Panels.cpp:1466` |
| `0xD1D` | c→s | `_MSG_MagicTrumpet` | C | Trombeta vinda do DBSrv, retransmitida a todos. | `server/tmsrv/net/ProcessDBMessage.cpp:136` |
| `0xD1F` | c→s | `_MSG_CNFDBCapsuleInfo` | C → removido | Resposta da cápsula vinda do DBSrv; trocada pelo 0xDC3. | `server/tmsrv/net/ProcessDBMessage.cpp:1247` |
| `0xE0E` | c→s | `_MSG_War` | B | Declarar guerra no painel da guilda (3598). | `client/src/scene/TMFieldScene_MsgBox.cpp:212` |
| `0xE12` | c→s | `_MSG_GuildAlly` | B | Aliança (3602). | `client/src/scene/TMFieldScene_MsgBox.cpp:224` |
| `0xE13` | c→s | `_MSG_GuildInfo` | C | Só lido do DBSrv. | `server/tmsrv/net/ProcessDBMessage.cpp:99` |
| `0xFDF` | c→s | `_MSG_AccountSecureFail` | C | Lido do DBSrv e reenviado ao cliente. | `server/tmsrv/net/ProcessDBMessage.cpp:547` |
| `0x4501` | c→s | `_MSG_GrindRankingData` | C | Lido do DBSrv (ranking). | `server/tmsrv/net/ProcessDBMessage.cpp:1278` |
