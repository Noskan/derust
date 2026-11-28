#pragma once

#include "scene/TreeNode.h"
#include "common/Structures.h"

/// Tile do Field*.trn no layout do WYD Global (12 bytes), conferido no desenho do chão do WYD.exe
/// do Global: as coordenadas vêm nos bytes 1 e 2 e os índices de textura são `short` nos bytes 4 e 6.
/// O 7662 guardava índice e coordenada em bytes alternados; ler os mapas do Global com aquele
/// layout tomava a coordenada pelo índice e deixava um quarto do chão sem textura.
struct FileTileInfo
{
    char cHeight;
    unsigned char byTileCoord;      // TMGround::TileCoordList
    unsigned char byBackTileCoord;  // TMGround::BackTileCoordList
    char cReserved;
    short sTileIndex;               // textura sTileIndex + TILE_TEXTURE_BASE
    short sBackTileIndex;           // textura sBackTileIndex + BACK_TILE_TEXTURE_BASE
    unsigned int dwColor;
};
static_assert(sizeof(FileTileInfo) == 12, "Field*.trn: 4096 tiles de 12 bytes");

/// Primeira textura de tile e de fundo na lista do Env (EnvTextureList3.bin). Com a lista de 2048
/// entradas do Global os fundos começam em 1024; no 7662 (512 entradas) começavam em 256.
constexpr int TILE_TEXTURE_BASE = 10;
constexpr int BACK_TILE_TEXTURE_BASE = 1024;

class TMSea;
/// @brief Bloco 128x128 de terreno, tilemap, picking, água e renderização no cliente.
/// @details Implementação dividida entre [TMGround.cpp](TMGround.cpp), [TMGround_Render.cpp](TMGround_Render.cpp)
///          e [TMGround_Tables.cpp](TMGround_Tables.cpp).
class TMGround : public TreeNode
{
public:
    TMGround();
    ~TMGround();

    void RestoreDeviceObjects() override;
    void SetPos(int nX, int nY);
    int Attach(TMGround* pGround);
    /// @brief Carrega dados de altura, textura e máscaras do mapa de tiles.
    int LoadTileMap(const char* szFileName);
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    /// @brief Calcula a interseção 3D do cursor sobre o terreno.
    D3DXVECTOR3 GetPickPos();
    /// @brief Interpola a altura do terreno na posição mundial informada.
    float GetHeight(TMVector2 vecPosition);
    int GetMask(TMVector2 vecPosition);
    D3DCOLORVALUE GetColor(TMVector2 vecPosition);
    int GetTileType(TMVector2 vecPosition);
    void SetColor(TMVector2 vecPosition, unsigned int dwColor);
    TMVector3 GetNormalInGround(int nX, int nY);
    int SetMiniMapData();
    void SetAttatchEnable(int nX, int nY);
    int IsInWater(TMVector2 vecPosition, float fHeight, float* pfWaterHeight);
    float GetWaterHeight(TMVector2 vecPosition, float* pfWaterHeight);

public:
    static float TileCoordList[8][4][2];
    static float BackTileCoordList[32][4][2];
    /// Coordenadas de textura dos 4 vértices de um tile. O Global indexa TileCoordList sem conferir,
    /// e o Field0102.trn traz coordenadas 44..46 e 100..101: de 8 a 39 a leitura cai em
    /// BackTileCoordList, que vem logo depois na memória (mesmo resultado aqui); acima disso o
    /// Global lê memória fora das tabelas, e este cliente usa a coordenada 0.
    static const float (*GetTileCoord(unsigned char byCoord))[2];
    static int m_nCheckSum[64][32];
    static int m_bFirst;
    static float m_fMiniMapScale;

public:
    char m_MapName[128];
    unsigned int m_dwLastEffectTime;
    unsigned int m_dwServertime;
    TMGround* m_pLeftGround;
    TMGround* m_pRightGround;
    TMGround* m_pUpGround;
    TMGround* m_pDownGround;
    int m_bVisible;
    int m_bWire;
    int m_bDungeon;
    int m_nMiniMapPos;
    FileTileInfo m_TileMapData[4096];
    char m_pMaskData[128][128];
    char m_pVAttrData[128][128];
    TMVector3 m_TileNormalVector[4096];
    D3DMATERIAL9 m_materials;
    RDLNVERTEX2 m_vertex[4];
    RDLVERTEX m_vertexVoodoo[4];
    int m_nOptTexture[512];
    IVector2 m_vecOffsetIndex;
    TMVector2 m_vecOffset;
    char m_cLeftEnable;
    char m_cRightEnable;
    char m_cUpEnable;
    char m_cDownEnable;
    TMSea* m_pSeaList[10];
    int m_nSeaIndex;
    float m_fEffHeight;
    TMVector2 m_vecEffset;
    unsigned int m_dwEffStart;
};