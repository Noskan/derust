# Dados do WYD Global no cliente

O teste fechado usa os dados do WYD Global (patch 749, instalação em PT). O cliente-alvo é o
7662, então os dois não leem tudo do mesmo jeito. A medição do passo 1 levou o cliente até o
mundo com esses dados, e dela saiu a decisão de **modelo híbrido** (2026-11-27):

- **Fica no 7662:** o protocolo e as tabelas que o servidor também usa (`ItemList`, `SkillData`,
  mobs). Os dados do Global cabem nelas sem perda de conteúdo jogável: nenhum item com nome tem
  índice 6500 ou maior.
- **O cliente lê o formato do Global direto:** os recursos visuais, que só o cliente usa.
  Convertê-los para os formatos do 7662 perdia, na medição, cerca de 641 texturas de modelo, 61
  de efeito e 140 de cenário.
- **Conversão de uma vez:** o que sobra (tabelas compartilhadas, textos PT, somas de conferência,
  lista de servidores) sai de [`tools/global_client_data.py`](../tools/global_client_data.py).

## O que o cliente passou a ler do Global

| Arquivo | Global | 7662 (antes) | No cliente |
|---|---|---|---|
| `UI/UITextureListN.bin`, `Effect/EffectTextureList.bin`, `Mesh/MeshTextureList.bin`, `Env/EnvTextureList3.bin` | registros de 528 bytes: nome em 0..509, caractere de alfa no byte 510 | o próprio `stTextureListInfo` (264 bytes) gravado no arquivo | `ReadTextureListFile` em `render/TextureManager.cpp` lê cada registro e copia nome e alfa |
| Quantidade de texturas (UI, efeito, modelo, cenário) | 512, 600, 3048, 2048 registros | 512, 512, 2048, 512 | `MAX_*_TEXTURE` em `render/TextureManager.h` |
| `Mesh/MeshList.txt` | índices até 3148 | 3048 entradas, sem conferir o índice | `MAX_COMMON_MESH` 4096; índice fora da tabela é registrado no log e ignorado |
| `object.bin` | 4096 máscaras, mesma cifra e mesma soma | 2048 | `MAX_OBJECT_MASK` 4096; `TMObject::RegisterMask` recusa índice fora da tabela |
| `curse.bin` | outro formato, sem contagem | contagem + pares de `stCurse` | contagem que não cabe no arquivo deixa a lista vazia em vez de derrubar o cliente |
| `Env/Field*.trn` (tiles de 12 bytes) | altura, coordenada, coordenada do fundo, reservado, índice do tile (`short`), índice do fundo (`short`), cor; fundos a partir da textura 1024 | índice e coordenada em bytes alternados; fundos a partir da 256 | `FileTileInfo` em `world/TMGround.h`, conferido no desenho do chão do `WYD.exe` do Global |
| `UI/FieldScene2.bin` | dois painéis a mais (90631 "Battlefield" e a barra 90624), visíveis no arquivo | — | a cena de campo os esconde ao carregar |

`Mesh/BoneAni4.txt` (até o índice 87, limite 100) e `Mesh/ValidIndex.bin` (exatamente
100 x 186 x 4 bytes) já cabiam e não mudaram.

O `TextureManager` do `WYD.exe` do Global (construtor e leitores) confirma os quatro limites e o
registro de 528 bytes: a lista do cenário é lida como `0x108000` bytes (2048 x 528).

Com o layout de tile do Global, só 520 dos 507.904 tiles dos mapas (0,1%) e 26 fundos apontam para
textura que a instalação não traz. Lidos no layout do 7662, eram 124.532 (os "trechos de chão
brancos" da primeira medição). As tabelas de coordenadas de tile do Global são iguais às do cliente.

O layout de campo do Global não tem 28 dos controles que a cena procura (por exemplo as grades
únicas `G_INV_GRID` e `G_CARGO_GRID`, já substituídas pelas páginas, e o texto de dinheiro 1041).
Os usos que não conferiam o ponteiro foram corrigidos (docs/BUGS_LATENTES.md); os demais já
conferiam.

## O que a ferramenta converte

```
python3 tools/global_client_data.py build "<instalação do Global>" <saída> \
    --status-url http://<ip>:<porta>/serv00.htm --ip <ip do TMSrv>
```

A saída é uma camada com os arquivos que mudam, nos caminhos em que o cliente os procura. Copie a
instalação do Global para uma pasta própria, copie a camada por cima e ponha o `TMProject.exe` na
raiz. A instalação original não é alterada; a ferramenta recusa uma saída dentro dela.

| Arquivo da camada | De onde vem |
|---|---|
| `ItemList.bin`, `SkillData.bin` | layout do Global (14500 x 172 e 248 x 104) levado ao do 7662 por `tools/w2_bin_csv.py`, com o `extraitem.bin` do Global aplicado antes. O relatório lista os campos do Global que não existem no 7662 |
| `Env/AttributeMap.dat` | o do Global com a soma `BASE_GetSum` no fim, que o cliente confere |
| `UI/strdef.bin` | o `Lang/PT/strdef.bin` (texto puro) com XOR 0x5A e soma. O `UI/strdef.bin` do Global é coreano |
| `UI/*.txt`, `Mixhelp.dat` | os textos de `Lang/PT/` |
| `cdata.bin` | as somas de `TMGround::m_nCheckSum` recalculadas dos `Field*.trn` e `Field*.dat`. O `cdata.bin` do Global confere 99 dos 120 `.dat` e nenhum dos 124 `.trn` com as regras deste cliente; sem a correção o campo para com "CheckSum Error" |
| `sn.bin`, `serverlist.bin` | um grupo com um canal no IP dado e o grupo virtual "recomendado", ambos com a URL da página de status do DBSrv |

`python3 tools/global_client_data.py selftest` confere as conversões com dados sintéticos (roda
no CI).

## Árvore do servidor Linux

```
python3 tools/global_server_data.py build <servidor W2PP> "<instalação do Global>" <destino> \
    --ip <ip do TMSrv> [--extra-ip <ip> ...] [--bin <dir com DBSrv e TMSrv> | --no-binaries]
```

Monta a árvore que o `tools/linux_deploy.py` espera, juntando as duas origens (as duas só são
lidas; o destino tem de ser novo ou vazio):

| Da distribuição W2PP | Do WYD Global |
|---|---|
| `BaseMob`, `BaseSummon`, `npc/`, `NPCGener.txt`, `Settings/`, `gameconfig.txt`, `Language.txt`, `LevelItem.txt`, `InitItem.csv` | `ItemList.bin` e `SkillData.bin` (no layout do 7662, como na camada do cliente), `HeightMap.dat`, `AttributeMap.dat`, `Lang/PT/strdef.bin` |

Os `STRUCT_MOB` de 816 ou 1296 bytes viram os de 1024, e os itens que não existem no ItemList do
Global saem dos mobs e do `LevelItem.txt`. Os textos passam a LF, sem BOM. Os nomes de
`Leader:`/`Follower:` terminados em ponto (aceitos no Windows) perdem o ponto quando o arquivo
sem ponto existe em `npc/`. Contas, personagens, listas administrativas, logs e binários da
origem não são copiados. `--bin` põe os servidores e gera as unidades do systemd (`--user`).
`python3 tools/global_server_data.py selftest` confere a conversão e o isolamento com dados
sintéticos.

## Página de status dos canais

O DBSrv grava `serv00.htm` (`CReadFiles::UpdateConnection`) com uma linha por TMSrv: a linha k é
a contagem do canal k + 1, ou -1 se o canal não está conectado. O cliente lê a página com
`BASE_ParseUserCount`, que põe a linha k em `nUserCount[k + 1]`, a posição que as cenas usam
para o canal. O formato antigo (`"%d\\n..."` no `sscanf_s`, com a barra invertida literal) só lia
o primeiro número.

## Notas da medição

- **Cliques automatizados:** o botão (`SButton`) só vê o clique se o botão do mouse estiver
  pressionado quando o quadro consulta o DirectInput (`EventTranslator`). Um clique instantâneo
  do `xdotool` se perde; um clique de 1,5 s funciona. Nas listas, o clique instantâneo funcionou
  na medição. Para testes, prefira o atalho `W2_AUTOLOGIN` (`client/src/core/TMAutoLogin.h`).
- **Resultado:** com os arquivos visuais crus do Global, a camada da ferramenta e a música ligada,
  o cliente entra no campo, anda e segue estável (sem "CheckSum Error" nem falha de textura no log).
- **Painel "Battlefield" e textos "Classe":** eram os painéis só do Global (tabela acima). Os
  textos "Classe" são o valor padrão (UIString 137) que o cliente do Global troca em tempo de jogo.
- **"Invalide Item Class 0" no log:** vinha do personagem de teste, criado com o `BaseMob`
  sintético do `tools/server_boot_smoke.py`, que não tem rosto (`Equip[0]`); a classe sai do
  `EF_CLASS` do rosto. Os rostos do ItemList do Global têm `EF_CLASS` certo.
- **Segunda medição, mundo povoado (2026-11-27):** com o servidor montado pela
  `tools/global_server_data.py` e o cliente com a camada, o personagem entra no Campo de Treino
  com o rosto do `BaseMob` real, entre NPCs e monstros. O chão sai sem as placas brancas (layout
  de tile do Global), a HUD sai sem o "Battlefield" e sem a barra "Classe", e o personagem anda.
  O "Invalide Item Class 0" aparece só 3 vezes, ao entrar no campo. O loop básico (spawn,
  combate, abate, EXP e gravação do personagem) passou com o DBSrv e o TMSrv compilados com ASan
  e UBSan; os defeitos que eles acharam estão em docs/BUGS_LATENTES.md.
