#include "core/pch.h"
#include "world/TMSea.h"
#include "scene/TMScene.h"
#include "core/TMGlobal.h"
#include "entities/TMHuman.h"
#include "core/TMLog.h"
#include "world/TMGround.h"
#include "render/TMCamera.h"
#include "effects/TMEffectBillBoard.h"
#include "skills/TMSkillFire.h"
#include "core/TMUtil.h"

/// @file TMGround.cpp
/// @brief Ciclo de vida, carregamento de tilemap, consultas de altura/máscara/água e conectividade de terreno.

int TMGround::m_bFirst = 1;

float TMGround::m_fMiniMapScale = 0.60f;

TMGround::TMGround()
    : TreeNode(0)
{
    m_vecOffsetIndex = IVector2{};
    m_vecEffset = TMVector2{};

    m_pLeftGround = 0;
    m_pRightGround = 0;
    m_pUpGround = 0;
    m_pDownGround = 0;
    m_nSeaIndex = 0;
    m_bVisible = 1;
    m_bDungeon = 0;
    m_bWire = 0;
    m_dwLastEffectTime = 0;
    m_dwServertime = 0;
    m_cLeftEnable = 0;
    m_cRightEnable = 0;
    m_cUpEnable = 0;
    m_cDownEnable = 0;
    m_fEffHeight = 2.0f;
    m_dwEffStart = 0;

    // this code is to supress warnings
    memset(&m_MapName, 0, sizeof m_MapName);
    memset(&m_TileMapData, 0, sizeof m_TileMapData);

    for (int i = 0; i < 4; ++i)
    {
        m_vertex[i].diffuse = 0x00FFFFFF;
        m_vertexVoodoo[i].diffuse = 0x00FFFFFF;
    }

    for (int i = 0; i < 128; ++i)
    {
        memset(&m_pMaskData[i], 0, sizeof m_pMaskData[i]);
        memset(&m_pVAttrData[i], 0, sizeof m_pVAttrData[i]);
    }

    for (int i = 0; i < 10; ++i)
        m_pSeaList[i] = 0;

    if (TMGround::m_bFirst == 1)
    {
        FILE* pFile = nullptr;
        fopen_s(&pFile, "cdata.bin", "rb");

        if (pFile)
        {
            fread(&TMGround::m_nCheckSum, sizeof m_nCheckSum, 1u, pFile);

            int nCDataCheckSum = 0;
            for (int k = 0; k < 64; ++k)
                for (int j = 0; j < 32; ++j)
                    nCDataCheckSum += 8 * k + 4 * m_nCheckSum[j][j] * 16 * j;

            if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && nCDataCheckSum != 5855606140)
            {
                fclose(pFile);

                LOG_WRITELOG("DataFile Error\r\n");
                MessageBoxA(g_pApp->m_hWnd, "DataFile Error.", "File Error", 0);
                PostMessageA(g_pApp->m_hWnd, WM_CLOSE, 0, 0);

                return;
            }

            fclose(pFile);
            m_bFirst = 0;
        }
        else
        {
            LOG_WRITELOG("DataFile NotFound");

            if (!g_pCurrentScene->m_bCriticalError)
                g_pCurrentScene->LogMsgCriticalError(4, 0, 0, 0, 0);

            g_pCurrentScene->m_bCriticalError = 1;
            return;
        }
    }

    SetPos(0, 0);
    return;
}

TMGround::~TMGround()
{
}

void TMGround::SetPos(int nX, int nY)
{
    m_vecOffsetIndex.x = nX;
    m_vecOffsetIndex.y = nY;

    m_vecOffset.x = (m_vecOffsetIndex.x * 2.0f) * 64.0f;
    m_vecOffset.y = (m_vecOffsetIndex.y * 2.0f) * 64.0f;

    if (nY <= 25)
    {
        if (nX >= 8 && nY <= 12 && nY >= 11 && nY <= 14)
        {
            m_bDungeon = 3;
            RenderDevice::m_bDungeon = 3;
        }
        else if (nX > 1 && nX < 11 && nY < 5)
        {
            m_bDungeon = 4;
            RenderDevice::m_bDungeon = 4;
        }
        else if (nX >= 26 && nX <= 30 && nY >= 8 && nY <= 12)
        {
            m_bDungeon = 5;
            RenderDevice::m_bDungeon = 5;
        }
        else
        {
            m_bDungeon = 0;
            RenderDevice::m_bDungeon = 0;
        }
    }
    else
    {
        if (nX < 16 && nX > 8 && nY > 25)
        {
            m_bDungeon = 2;
            RenderDevice::m_bDungeon = 2;
        }
        else
        {
            m_bDungeon = 1;
            RenderDevice::m_bDungeon = 1;
        }
    }
}

int TMGround::Attach(TMGround* pGround)
{
    if (!pGround)
        return 0;

    m_pLeftGround = 0;
    m_pRightGround = 0;
    m_pUpGround = 0;
    m_pDownGround = 0;

    if (pGround->m_vecOffsetIndex.x == m_vecOffsetIndex.x + 1)
    {
        m_pRightGround = pGround;
        m_pRightGround->m_pLeftGround = this;

        for (int i = 0; i < 64; ++i)
        {
            pGround->m_TileMapData[64 * i].cHeight = m_TileMapData[(i << 6) + 63].cHeight;
            pGround->m_TileMapData[64 * i].dwColor = m_TileMapData[(i << 6) + 63].dwColor;

            pGround->m_TileNormalVector[64 * i] = m_TileNormalVector[64 * i + 63];
        }

        m_nMiniMapPos = 0;
        m_pRightGround->m_nMiniMapPos = 1;
        return 1;
    }
    if (pGround->m_vecOffsetIndex.x == m_vecOffsetIndex.x - 1)
    {
        m_pLeftGround = pGround;
        m_pLeftGround->m_pRightGround = this;

        for (int i = 0; i < 64; ++i)
        {
            m_TileMapData[64 * i].cHeight = pGround->m_TileMapData[(i << 6) + 63].cHeight;
            m_TileMapData[64 * i].dwColor = pGround->m_TileMapData[(i << 6) + 63].dwColor;

           m_TileNormalVector[64 * i] = pGround->m_TileNormalVector[64 * i + 63];
        }

        m_nMiniMapPos = 1;
        m_pLeftGround->m_nMiniMapPos = 0;
        return 1;
    }
    if (pGround->m_vecOffsetIndex.y == m_vecOffsetIndex.y + 1)
    {
        m_pDownGround = pGround;
        m_pDownGround->m_pUpGround = this;

        for (int i = 0; i < 64; ++i)
        {
            pGround->m_TileMapData[i].cHeight = m_TileMapData[i + 4032].cHeight;
            pGround->m_TileMapData[i].dwColor = m_TileMapData[i + 4032].dwColor;

            pGround->m_TileNormalVector[i] = m_TileNormalVector[i + 4032];
        }

        m_nMiniMapPos = 0;
        m_pDownGround->m_nMiniMapPos = 2;
        return 1;
    }
    if (pGround->m_vecOffsetIndex.y == m_vecOffsetIndex.y - 1)
    {
        m_pUpGround = pGround;
        m_pUpGround->m_pDownGround = this;

        for (int i = 0; i < 64; ++i)
        {
            m_TileMapData[i].cHeight = pGround->m_TileMapData[i + 4032].cHeight;
            m_TileMapData[i].dwColor = pGround->m_TileMapData[i + 4032].dwColor;

            m_TileNormalVector[i] = pGround->m_TileNormalVector[i + 4032];
        }

        m_nMiniMapPos = 2;
        m_pUpGround->m_nMiniMapPos = 0;
        return 1;
    }

    return 0;
}

int TMGround::LoadTileMap(const char* szFileName)
{
    FILE* fp = nullptr;
    fopen_s(&fp, szFileName, "rb");

    if (fp)
    {
        int byNameLen = 0;
        fread(&byNameLen, 1u, 1u, fp);

       //  added to supress warning
        if (byNameLen > 128)
            byNameLen = 128;

        fread(m_MapName, 1u, byNameLen, fp);

        m_MapName[byNameLen] = '\0';

        int bPosX = 0;
        int bPosY = 0;
        fread(&bPosX, 1, 1, fp);
        fread(&bPosY, 1, 1, fp);

        SetPos(bPosX, bPosY);
        SetAttatchEnable(bPosX, bPosY);

        fread(m_TileMapData, 12, 4096, fp);
        fclose(fp);

        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            int nCheckSum = 0;
            int nCheckSize = 49152;
            auto pCheck = (char*)m_TileMapData;

            for (int i = 0; i < nCheckSize; nCheckSum += pCheck[i++])
                ;

            if (m_nCheckSum[bPosY][bPosX] != nCheckSum + bPosX * bPosY)
            {
                LOG_WRITELOG("CheckSum Error: %d,%d m_nCheckSum=%d nCheckSum= %d\r\n",
                    bPosX, bPosY, m_nCheckSum[bPosY][bPosX], nCheckSum + bPosX * bPosY);

                if (!g_pCurrentScene->m_bCriticalError)
                    g_pCurrentScene->LogMsgCriticalError(5, 0, 0, 0, 0);

                g_pCurrentScene->m_bCriticalError = 1;
                return 0;
            }
        }

        for (int nY = 1; nY < 63; ++nY)
        {
            for (int nX = 1; nX < 63; ++nX)
            {
                m_TileNormalVector[nX + (nY << 6)] = GetNormalInGround(nX, nY);
            }
        }

        for (int nNoIndex = 0; nNoIndex < 64; ++nNoIndex)
        {
            m_TileNormalVector[64 * nNoIndex] = m_TileNormalVector[64 * nNoIndex + 1];
            m_TileNormalVector[(nNoIndex << 6) + 63] = m_TileNormalVector[64 * nNoIndex + 62];
        }

        for (int nNoIndex = 0; nNoIndex < 64; ++nNoIndex)
        {
            m_TileNormalVector[nNoIndex] = m_TileNormalVector[nNoIndex + 64];
            m_TileNormalVector[nNoIndex + 4032] = m_TileNormalVector[nNoIndex + 3968];
        }

        memset(&m_materials, 0, sizeof m_materials);
        m_materials.Diffuse.r = 1.0f;
        m_materials.Diffuse.g = 1.0f;
        m_materials.Diffuse.b = 1.0f;

        m_materials.Specular = m_materials.Diffuse;
        m_materials.Power = 0.0f;

        m_materials.Emissive.r = 0.3f;
        m_materials.Emissive.g = 0.3f;
        m_materials.Emissive.b = 0.3f;

        if (g_pDevice->m_bVoodoo == 1)
        {
            m_materials.Emissive.r = 0.2f;
            m_materials.Emissive.g = 0.2f;
            m_materials.Emissive.b = 0.2f;
        }

        for (int nY = 0; nY < 64; ++nY)
            m_TileMapData[64 * nY].cHeight = m_TileMapData[(nY << 6) + 1].cHeight;

        for (int j = 0; j < 64; ++j)
            m_TileMapData[j].cHeight = m_TileMapData[j + 64].cHeight;

        for (int nY = 0; nY < 64; ++nY)
        {
            for (int j = 0; j < 64; ++j)
            {
                float f1 = 0.0f;
                float f2 = 0.0f;
                float f3 = 0.0f;
                float f4 = 0.0f;

                f1 = m_TileMapData[j + (nY << 6)].cHeight;
                if (nY < 62)
                    f3 = m_TileMapData[j + ((nY + 1) << 6)].cHeight;
                else
                    f3 = m_TileMapData[j + (nY << 6)].cHeight;

                if (j < 62)
                {
                    f2 = m_TileMapData[j + (nY << 6) + 1].cHeight;

                    if (nY >= 62)
                        f4 = m_TileMapData[j + (nY << 6) + 1].cHeight;
                    else
                        f4 = m_TileMapData[j + ((nY + 1) << 6) + 1].cHeight;
                }
                else
                {
                    f2 = m_TileMapData[j + (nY << 6)].cHeight;

                    if (nY >= 62)
                        f4 = m_TileMapData[j + (nY << 6)].cHeight;
                    else
                        f4 = m_TileMapData[j + ((nY + 1) << 6)].cHeight;
                }

                float fCenter = (((f1 + f2) + f3) + f4) / 4.0f;
                m_pMaskData[2 * nY][2 * j] = static_cast<char>((f1 + fCenter) / 2.0f);
                m_pMaskData[2 * nY][2 * j + 1] = static_cast<char>((f2 + fCenter) / 2.0f);
                m_pMaskData[2 * nY + 1][2 * j] = static_cast<char>((f3 + fCenter) / 2.0f);
                m_pMaskData[2 * nY + 1][2 * j + 1] = static_cast<char>((f4 + fCenter) / 2.0f);
            }
        }

        if (!m_cUpEnable)
        {
            for (int X = 0; X < 128; ++X)
            {
                for (int Y = 0; Y < 15; ++Y)
                    m_pMaskData[Y][X] = 127;
            }
        }

        if (!m_cDownEnable)
        {
            for (int k = 0; k < 128; ++k)
            {
                for (int l = 114; l < 128; ++l)
                    m_pMaskData[l][k] = 127;
            }
        }
        
        if (!m_cLeftEnable)
        {
            for (int m = 0; m < 128; ++m)
            {
                for (int n = 0; n < 15; ++n)
                    m_pMaskData[m][n] = 127;
            }
        }

        if (!m_cRightEnable)
        {
            for (int ii = 0; ii < 128; ++ii)
            {
                for (int jj = 114; jj < 128; ++jj)
                    m_pMaskData[ii][jj] = 127;
            }
        }

        if (m_cLeftEnable == 1 && m_cDownEnable == 1)
        {
            for (int kk = 0; kk < 16; ++kk)
            {
                for (int ll = 113; ll < 128; ++ll)
                    m_pMaskData[ll][kk] = 127;
            }
        }
        
        if (m_cLeftEnable == 1 && m_cUpEnable == 1)
        {
            for (int mm = 0; mm < 16; ++mm)
            {
                for (int nn = 0; nn < 16; ++nn)
                    m_pMaskData[nn][mm] = 127;
            }
        }
        
        if (m_cRightEnable == 1 && m_cDownEnable == 1)
        {
            for (int i1 = 113; i1 < 128; ++i1)
            {
                for (int i2 = 113; i2 < 128; ++i2)
                    m_pMaskData[i2][i1] = 127;
            }
        }
        
        if (m_cRightEnable == 1 && m_cUpEnable == 1)
        {
            for (int i3 = 113; i3 < 128; ++i3)
            {
                for (int i4 = 0; i4 < 16; ++i4)
                    m_pMaskData[i4][i3] = 127;
            }
        }

        return true;
    }
    else
        LOG_WRITELOG(">> Fail to Load TileMap : %s\r\n", szFileName);

    return 0;
}

float TMGround::GetHeight(TMVector2 vecPosition)
{
    float fHeight = 1.0f;

    int nX = static_cast<int>((vecPosition.x - m_vecOffset.x) / 2.0f);
    int nY = static_cast<int>((vecPosition.y - m_vecOffset.y) / 2.0f);

    if (nX < 0 || nY < 0 || nX > 64 || nY > 64)
        return -10000.0f;

    D3DXVECTOR3 vPickPos{ 0.0, -10000.0f, 0.0f };
    D3DXVECTOR3 vPickRayDir{ 0.0f, -1.0f, 0.0f };
    D3DXVECTOR3 vPickRayOrig{ vecPosition.x, 100.0f, vecPosition.y };
    D3DXVECTOR3 v0{};
    D3DXVECTOR3 v2{};
    D3DXVECTOR3 v6{};
    D3DXVECTOR3 v8{};

    if (nX < 63 && nY < 63 && nX >= 0 && nY >= 0)
    {
        v0 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + (nY << 6)].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + (nY << 6) + 1].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + ((nY + 1) << 6)].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + ((nY + 1) << 6) + 1].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);
    }
    if (nX == 63)
    {
        v0 = D3DXVECTOR3((float)((float)63 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[(nY << 6) + 63].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)64 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[(nY << 6) + 63].cHeight * 0.1f,
            (float)((float)nY * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)63 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[((nY + 1) << 6) + 63].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)64 * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[((nY + 1) << 6) + 63].cHeight * 0.1f,
            (float)((float)(nY + 1) * 2.0f) + m_vecOffset.y);
    }
    else if (nY == 63)
    {
        v0 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4032].cHeight * 0.1f,
            (float)((float)63 * 2.0f) + m_vecOffset.y);

        v2 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4033].cHeight * 0.1f,
            (float)((float)63 * 2.0f) + m_vecOffset.y);

        v6 = D3DXVECTOR3((float)((float)nX * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4032].cHeight * 0.1f,
            (float)((float)64 * 2.0f) + m_vecOffset.y);

        v8 = D3DXVECTOR3((float)((float)(nX + 1) * 2.0f) + m_vecOffset.x,
            (float)m_TileMapData[nX + 4033].cHeight * 0.1f,
            (float)((float)64 * 2.0f) + m_vecOffset.y);
    }
   
    float fU = 0.0;
    float fV = 0.0;
    float fDis = 0.0f;

    if (D3DXIntersectTri(&v0, &v2, &v6, &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDis) == 1)
        return (100.0f - fDis);
    
    if (D3DXIntersectTri(&v8, &v6, &v2, &vPickRayOrig, &vPickRayDir, &fU, &fV, &fDis) == 1)
        return (100.0f - fDis);

    if (nX < 0 || nY < 0 || nX > 63 || nY > 63)
        return -10000.0f;

    if (nX >= 0 && nX < 63 && nY >= 0 && nY < 63)
    {
        fHeight += m_TileMapData[nX + (nY << 6)].cHeight;
        fHeight += m_TileMapData[nX + (nY << 6) + 1].cHeight;
        fHeight += m_TileMapData[nX + ((nY + 1) << 6)].cHeight;
        fHeight += m_TileMapData[nX + ((nY + 1) << 6) + 1].cHeight;
    }
    else if (nX == 63)
    {
        fHeight += m_TileMapData[(nY << 6) + 63].cHeight;
        fHeight += m_TileMapData[(nY << 6) + 63].cHeight;
        fHeight += m_TileMapData[((nY + 1) << 6) + 63].cHeight;
        fHeight += m_TileMapData[((nY + 1) << 6) + 63].cHeight;
    }
    else if (nY == 63)
    {
        fHeight += m_TileMapData[nX + 4032].cHeight;
        fHeight += m_TileMapData[nX + 4033].cHeight;
        fHeight += m_TileMapData[nX + 4032].cHeight;
        fHeight += m_TileMapData[nX + 4033].cHeight;
    }
	
    return (fHeight * 0.1f) / 4.0f;
}

int TMGround::GetMask(TMVector2 vecPosition)
{
    int nMaskX = (int)(vecPosition.x - m_vecOffset.x);
    int nMaskY = (int)(vecPosition.y - m_vecOffset.y);

    if (nMaskX >= 0 && nMaskY >= 0 && nMaskX < 128 && nMaskY < 128)
        return m_pMaskData[nMaskY][nMaskX];

    return -10000;
}

D3DCOLORVALUE TMGround::GetColor(TMVector2 vecPosition)
{
    int nX = static_cast<int>((vecPosition.x - m_vecOffset.x) / 2);
    int nY = static_cast<int>((vecPosition.y - m_vecOffset.y) / 2);

    int dwColor[4]{ 0 };
    if (nX >= 0 && nX < 63 && nY >= 0 && nY < 63)
    {
        dwColor[0] = m_TileMapData[nX + (nY << 6)].dwColor;
        dwColor[1] = m_TileMapData[nX + (nY << 6) + 1].dwColor;
        dwColor[2] = m_TileMapData[nX + ((nY + 1) << 6)].dwColor;
        dwColor[3] = m_TileMapData[nX + ((nY + 1) << 6) + 1].dwColor;
    }
    else if (nX == 63)
    {
        dwColor[0] = m_TileMapData[(nY << 6) + 63].dwColor;
        dwColor[1] = m_TileMapData[(nY << 6) + 63].dwColor;
        dwColor[2] = m_TileMapData[((nY + 1) << 6) + 63].dwColor;
        dwColor[3] = m_TileMapData[((nY + 1) << 6) + 63].dwColor;
    }
    else if (nX == 63)
    {
        dwColor[0] = m_TileMapData[nX + 4032].dwColor;
        dwColor[1] = m_TileMapData[nX + 4033].dwColor;
        dwColor[2] = m_TileMapData[nX + 4032].dwColor;
        dwColor[3] = m_TileMapData[nX + 4033].dwColor;
    }

    D3DCOLORVALUE color[4]{};
    for (int i = 0; i < 4; ++i)
    {
        color[i].r = ((0xFF0000 & dwColor[i]) >> 16) / 256.0f;
        color[i].g = ((dwColor[i] & 0xFF00) >> 8) / 256.0f;
        color[i].b = (dwColor[i] & 0xFF) / 256.0f;
    }

    float fDX = ((float)nX * 2.0f) - (vecPosition.x - m_vecOffset.x);
    float fDY = ((float)nY * 2.0f) - (vecPosition.y - m_vecOffset.y);

    D3DCOLORVALUE result{};

    result.g = (((((fDX + fDY) * color[3].g) + (((4.0f - fDX) - fDY) * color[0].g)) + (((fDX + 2.0f) - fDY) * color[1].g)) + (((2.0f - fDX) + fDY) * color[2].g)) / 12.0f;
    result.b = (((((fDX + fDY) * color[3].b) + (((4.0f - fDX) - fDY) * color[0].b)) + (((fDX + 2.0f) - fDY) * color[1].b)) + (((2.0f - fDX) + fDY) * color[2].b)) / 12.0f;
    result.r = (((((fDX + fDY) * color[3].r) + (((4.0f - fDX) - fDY) * color[0].r)) + (((fDX + 2.0f) - fDY) * color[1].r)) + (((2.0f - fDX) + fDY) * color[2].r)) / 12.0f;
    result.a = 1.0f;

	return result;
}

int TMGround::GetTileType(TMVector2 vecPosition)
{
    int nX = static_cast<int>(vecPosition.x - m_vecOffset.x);
    int nY = static_cast<int>(vecPosition.y - m_vecOffset.y);

    if (m_pVAttrData[nY][nX] == 1)
        return 1;

    int nIndex = m_TileMapData[nX / 2 + (nY / 2 << 6)].sTileIndex + TILE_TEXTURE_BASE;

    if (nIndex >= 14 && nIndex <= 17 && nIndex >= 38 && nIndex <= 77 && nIndex >= 86 && nIndex <= 101 && nIndex >= 130 && nIndex <= 149)
        return 0;

    if (nIndex >= 186 && nIndex <= 193)
        return 11;

    if (nIndex >= 202 && nIndex <= 205)
        return 8;

    if (nIndex < 230 || nIndex > 231)
        return 3;

    return 9;
}

void TMGround::SetColor(TMVector2 vecPosition, unsigned int dwColor)
{
    int nX = static_cast<int>(vecPosition.x - m_vecOffset.x) / 2;
    int nY = static_cast<int>(vecPosition.y - m_vecOffset.y) / 2;

    if (nX >= 0 && nX <= 63 && nY >= 0 && nY <= 63)
        m_TileMapData[nX + (nY << 6)].dwColor = dwColor;
}

TMVector3 TMGround::GetNormalInGround(int nX, int nY)
{
    TMVector3 vRetNormal(0.0f, 1.0f, 0.0f);
    TMVector3 vNormal[4]{};
    TMVector3 vAround[4]{};

    float fCurHeight = (float)m_TileMapData[nX + (nY << 6)].cHeight;

    if (nX > 0 && nX < 64 && nY > 0 && nY < 64)
    {
        vAround[0] = TMVector3(-1.0f, (float)m_TileMapData[nX + (nY << 6) - 1].cHeight, 0.0f);
        vAround[1] = TMVector3(0.0f, (float)m_TileMapData[nX + ((nY + 1) << 6)].cHeight, 1.0f);
        vAround[2] = TMVector3(1.0f, (float)m_TileMapData[nX + (nY << 6) + 1].cHeight, 0.0f);
        vAround[3] = TMVector3(0.0f, (float)m_TileMapData[nX + ((nY - 1) << 6)].cHeight, -1.0f);

        vNormal[0] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[0], vAround[1]);
        vNormal[1] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[1], vAround[2]);
        vNormal[2] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[2], vAround[3]);
        vNormal[3] = ComputeNormalVector(TMVector3(0.0f, fCurHeight, 0.0f), vAround[3], vAround[0]);

        vRetNormal = (vNormal[0] + vNormal[1] + vNormal[2] + vNormal[3]) / 4.0f;
    }

    return vRetNormal;
}

void TMGround::SetAttatchEnable(int nX, int nY)
{
    if (nX == 1 && nY == 1)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 16 && nY == 15)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 16 && nY == 16)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 16)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 16)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 18 && nY == 17)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 16)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 15)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 20 && nY == 16)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 20 && nY == 15)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 14)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 19 && nY == 12)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 20 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 12)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 13)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 12)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 11)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 10)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 9)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 15 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 16 && nY == 12)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 15 && nY == 12)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 14 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 28)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 31)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 14 && nY == 30)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 15 && nY == 31)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 12 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 9 && nY == 13)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 8 && nY == 13)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 14)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 13 && nY == 14)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 13 && nY == 12)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 1 && nY == 31)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 1 && nY == 29)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 2 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 3 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 4 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 5 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 3 && nY == 30)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 3 && nY == 31)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 5 && nY == 31)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 7 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 6 && nY == 30)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 7 && nY == 29)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 8 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 11)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 9 && nY == 31)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 10 && nY == 29)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 11 && nY == 29)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 9 && nY == 28)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 8 && nY == 27)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 10 && nY == 27)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 8 && nY == 2)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 9 && nY == 1)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 10 && nY == 2)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 13 && nY == 28)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 14 && nY == 28)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 31)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 18 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 31)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 17 && nY == 30)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 18 && nY == 30)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX == 19 && nY == 30)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 17 && nY == 28)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 31 && nY == 31)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 25 && nY == 13)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 8)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 9)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 10)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 11)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 26 && nY == 12)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX == 27 && nY == 11)
    {
        m_cLeftEnable = 2;
        m_cRightEnable = 2;
        m_cUpEnable = 2;
        m_cDownEnable = 2;
    }
    else if (nX < 26)
    {
        if (nX == 8 && nY == 16)
        {
            m_cLeftEnable = 0;
            m_cRightEnable = 1;
            m_cUpEnable = 1;
            m_cDownEnable = 0;
        }
        else if (nX == 9 && nY == 16)
        {
            m_cLeftEnable = 1;
            m_cRightEnable = 0;
            m_cUpEnable = 1;
            m_cDownEnable = 0;
        }
        else if (nX == 8 && nY == 15)
        {
            m_cLeftEnable = 0;
            m_cRightEnable = 1;
            m_cUpEnable = 0;
            m_cDownEnable = 1;
        }
        else if (nX == 9 && nY == 15)
        {
            m_cLeftEnable = 1;
            m_cRightEnable = 0;
            m_cUpEnable = 0;
            m_cDownEnable = 1;
        }
    }
    else if (nX == 28 && nY == 24)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 28 && nY == 23)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 27 && nY == 23)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 0;
    }
    else if (nX == 29 && nY == 23)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 2;
        m_cUpEnable = 1;
        m_cDownEnable = 2;
    }
    else if (nX == 27 && nY == 22)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 28 && nY == 22)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 1;
        m_cDownEnable = 1;
    }
    else if (nX == 28 && nY == 21)
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 29 && nY == 22)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 1;
        m_cUpEnable = 0;
        m_cDownEnable = 1;
    }
    else if (nX == 30 && nY == 22)
    {
        m_cLeftEnable = 1;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
    else if (nX != 29 || nY != 27)
    {
        if (nX == 28 && nY == 28)
        {
            m_cLeftEnable = 0;
            m_cRightEnable = 0;
            m_cUpEnable = 0;
            m_cDownEnable = 0;
        }
        else if (nX == 30 && nY == 28)
        {
            m_cLeftEnable = 0;
            m_cRightEnable = 0;
            m_cUpEnable = 0;
            m_cDownEnable = 0;
        }
    }
    else
    {
        m_cLeftEnable = 0;
        m_cRightEnable = 0;
        m_cUpEnable = 0;
        m_cDownEnable = 0;
    }
}

int TMGround::IsInWater(TMVector2 vecPosition, float fHeight, float* pfWaterHeight)
{
    int i = 0;
    for (i = 0; ; ++i)
    {
        if (i >= 10)
            return 0;

        if (m_pSeaList[i])
        {
            POINT ptPos{};
            ptPos.x = static_cast<LONG>(vecPosition.x);
            ptPos.y = static_cast<LONG>(vecPosition.y);

            if (PtInRect(&m_pSeaList[i]->m_rectRange, ptPos) == 1)
                break;
        }
    }

    if (m_pSeaList[i]->m_fHeight <= fHeight)
        return 0;

    *pfWaterHeight = m_pSeaList[i]->m_fHeight;
	return 1;
}

float TMGround::GetWaterHeight(TMVector2 vecPosition, float* pfWaterHeight)
{
    for (int i = 0; i < 10; ++i)
    {
        if (m_pSeaList[i])
        {
            float fHeight = m_pSeaList[i]->GetHeight(vecPosition.x, vecPosition.y);

            if (fHeight > -100.0f)
            {
                *pfWaterHeight = fHeight + m_pSeaList[i]->m_fHeight;

                return *pfWaterHeight;
            }
        }
    }

    return -100.0f;
}
