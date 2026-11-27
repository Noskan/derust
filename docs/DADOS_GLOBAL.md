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

`Mesh/BoneAni4.txt` (até o índice 87, limite 100) e `Mesh/ValidIndex.bin` (exatamente
100 x 186 x 4 bytes) já cabiam e não mudaram.

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
- **Ainda não investigado:** o painel "Battlefield" aberto ao entrar no campo, que não fecha no
  botão Fechar; textos "Classe" no lugar de valores; trechos de chão brancos; e as linhas
  "Invalide Item Class 0" no log.
