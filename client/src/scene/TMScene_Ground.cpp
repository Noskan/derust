#include "core/pch.h"
#include "ui/SControlContainer.h"
#include "ui/SControl.h"
#include "ui/SGrid.h"
#include "ui/UIBinary.h"
#include "world/TMGround.h"
#include "world/TMSky.h"
#include "world/TMSun.h"
#include "render/TMLight.h"
#include "entities/TMHuman.h"
#include "scene/TMObject.h"
#include "render/TMCamera.h"
#include "scene/TMObjectContainer.h"
#include "entities/TMItem.h"
#include "core/TMGlobal.h"
#include "core/TMLog.h"
#include "scene/TMScene.h"
#include "scene/TMFieldScene.h"
#include "effects/TMEffectFirework.h"
#include "world/TMSnow.h"
#include "world/TMRain.h"

/// @file TMScene_Ground.cpp
/// @brief Consultas ao terreno da cena: anexação de blocos de chão (`GroundNewAttach`),
///        picking, tipo de tile, máscara, altura, cor e água.
///
/// Extraído de `TMScene.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

char heightMapData[128][128]{};

int TMScene::GroundNewAttach(EDirection eDir)
{
	memset(heightMapData, 0, sizeof(heightMapData));
	if (!m_pGround)
		return 0;

	int x{};
	int y{};

	if (eDir == EDirection::EDIR_LEFT)
	{
		if (m_pGround->m_pLeftGround || m_pGround->m_cLeftEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x - 1;
		y = m_pGround->m_vecOffsetIndex.y;
	}
	else if (eDir == EDirection::EDIR_RIGHT)
	{
		if (m_pGround->m_pRightGround || m_pGround->m_cRightEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x + 1;
		y = m_pGround->m_vecOffsetIndex.y;
	}
	else if (eDir == EDirection::EDIR_UP)
	{
		if (m_pGround->m_pUpGround || m_pGround->m_cUpEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x;
		y = m_pGround->m_vecOffsetIndex.y - 1;
	}
	else if (eDir == EDirection::EDIR_DOWN)
	{
		if (m_pGround->m_pDownGround || m_pGround->m_cDownEnable != 1)
			return 0;

		x = m_pGround->m_vecOffsetIndex.x;
		y = m_pGround->m_vecOffsetIndex.y + 1;
	}

	char fileNameTrn[128]{};
	char fileNameDat[128]{};

	sprintf_s(fileNameTrn, "Env\\Field%02d%02d.trn", x, y);
	sprintf_s(fileNameDat, "Env\\Field%02d%02d.dat", x, y);

	int gId = (m_nCurrentGroundIndex + 1) % 2;

	if (m_pGroundList[gId])
	{
		if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x + 1 &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y + 1)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x - 1 &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], &m_HeightMapData[i][128], 128);
		}
		else if (m_pGroundList[gId]->m_vecOffsetIndex.x == m_pGround->m_vecOffsetIndex.x &&
			m_pGroundList[gId]->m_vecOffsetIndex.y == m_pGround->m_vecOffsetIndex.y - 1)
		{
			for (int i = 0; i < 128; ++i)
				memcpy(heightMapData[i], m_HeightMapData[i + 128], 128);
		}
	}
	else
	{
		for (int i = 0; i < 128; ++i)
			memcpy(heightMapData[i], m_HeightMapData[i], 128);
	}
	
	auto pGround = new TMGround();

	if (!pGround->LoadTileMap(fileNameTrn))
	{
		delete pGround;
		return 0;
	}

	if (m_pGroundList[gId])
	{
		delete m_pGroundList[gId];

		m_pGroundList[gId] = nullptr;
	}

	if (m_pObjectContainerList[gId])
	{
		delete m_pObjectContainerList[gId];

		m_pObjectContainerList[gId] = nullptr;
	}

	m_pGroundList[gId] = pGround;

	m_pGroundObjectContainer->AddChild(m_pGroundList[gId]);

	if (m_pGroundList[m_nCurrentGroundIndex])
		m_pGroundList[m_nCurrentGroundIndex]->Attach(m_pGroundList[gId]);

	m_pObjectContainerList[gId] = new TMObjectContainer(m_pGroundList[gId]);

	if (!m_pObjectContainerList[gId]->Load(fileNameDat))
	{
		if (m_pObjectContainerList[gId])
		{
			delete m_pObjectContainerList[gId];
			
			m_pObjectContainerList[gId] = nullptr;
		}
		
		LOG_WRITELOG("DataFile Not Found : %s\r\n", fileNameDat);
		
		if (!m_bCriticalError)
			LogMsgCriticalError(12, 0, 0, 0, 0);

		m_bCriticalError = 1;
		return 0;
	}

	if (m_pObjectContainerList[m_nCurrentGroundIndex])
	{
		if (eDir == EDirection::EDIR_DOWN)
			m_pObjectContainerList[m_nCurrentGroundIndex]->SetPrevNode(m_pObjectContainerList[gId]);
		else
			m_pObjectContainerList[m_nCurrentGroundIndex]->SetNextNode(m_pObjectContainerList[gId]);
	}

	g_pTextureManager->ReleaseNotUsingTexture();
	
	memset(m_HeightMapData, 0, 4);

	switch (eDir)
	{
	case EDirection::EDIR_LEFT:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(m_HeightMapData[i], m_pGround->m_pLeftGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i][128], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_pLeftGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_pLeftGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_RIGHT:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i][128], m_pGround->m_pRightGround->m_pMaskData[i], 128);
			memcpy(m_HeightMapData[i], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_UP:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i], m_pGround->m_pUpGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i + 128], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_pUpGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_pUpGround->m_vecOffset.y;
		break;
	case EDirection::EDIR_DOWN:
		for (int i = 0; i < 128; ++i)
		{
			memcpy(&m_HeightMapData[i + 128], m_pGround->m_pDownGround->m_pMaskData[i], 128);
			memcpy(&m_HeightMapData[i], heightMapData[i], 128);
		}
		g_HeightPosX = (int)m_pGround->m_vecOffset.x;
		g_HeightPosY = (int)m_pGround->m_vecOffset.y;
		break;
	}

	BASE_ApplyAttribute((char*)m_HeightMapData, 256);

	memcpy(m_GateMapData, m_HeightMapData, sizeof(m_HeightMapData));

	SaveHeightMap(fileNameTrn);

	m_pGround->SetMiniMapData();

	m_nAdjustTime = 0;
	m_dwInitTime = g_pTimerManager->GetServerTime();

	return 1;
}

D3DXVECTOR3 TMScene::GroundGetPickPos()
{
	D3DXVECTOR3 vPickPos{ 0.0f, -10000.0f, 0.0f };
	D3DXVECTOR3 vPickTempPos{ 0.0f, -10000.0f, 0.0f };
	D3DXVECTOR3 vFocusePos{ 0.0f, -9000.0f, 0.0f };

	auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

	if (pFocusedObject)
	{
		vFocusePos.x = pFocusedObject->m_vecPosition.x;
		vFocusePos.y = pFocusedObject->m_fHeight;
		vFocusePos.z = pFocusedObject->m_vecPosition.x;
	}

	if (!m_pGround)
		return vPickPos;

	vPickTempPos = m_pGround->GetPickPos();

	if (fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
		vPickPos = vPickTempPos;

	if (vPickPos.y >= -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f)
		return vPickPos;

	if (m_pGround->m_pLeftGround)
	{
		vPickTempPos = m_pGround->m_pLeftGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y) || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f)
				return vPickPos;
		}
	}

	if (m_pGround->m_pRightGround)
	{
		vPickTempPos = m_pGround->m_pRightGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y < 4.0f))
				return vPickPos;
		}
	}

	if (m_pGround->m_pUpGround)
	{
		vPickTempPos = m_pGround->m_pUpGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0f && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
				return vPickPos;
		}
	}

	if (m_pGround->m_pDownGround)
	{
		vPickTempPos = m_pGround->m_pDownGround->GetPickPos();

		if (fabsf(vFocusePos.y - vPickPos.y) > fabsf(vFocusePos.y - vPickTempPos.y))
		{
			vPickPos = vPickTempPos;

			if (vPickTempPos.y > -5000.0 && fabsf(vFocusePos.y - vPickPos.y) < 2.0f || fabsf(vFocusePos.y - vPickTempPos.y) < 4.0f)
				return vPickPos;

		}
	}

	return vPickPos;
}

int TMScene::GroundGetTileType(TMVector2 vecPosition)
{
	int nTileType{};

	if (m_pGround)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) && 
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pLeftGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pRightGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pUpGround->GetTileType(vecPosition);
		}
		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			nTileType = m_pGround->m_pDownGround->GetTileType(vecPosition);
		}
	}

	return nTileType;
}

int TMScene::GroundGetMask(TMVector2 vecPosition)
{
	int nXIndex = (int)vecPosition.x - g_HeightPosX;
	int nYIndex = (int)vecPosition.y - g_HeightPosY;

	if (nXIndex < 0)
		nXIndex = 0;

	if (nYIndex < 0)
		nYIndex = 0;

	if (nXIndex > 256)
		nXIndex = 255;

	if (nYIndex > 256)
		nYIndex = 255;

	int value = m_HeightMapData[nYIndex][nXIndex];
	return m_HeightMapData[nYIndex][nXIndex];
}

int TMScene::GroundGetMask(IVector2 vecPosition)
{
	int nXIndex = vecPosition.x - g_HeightPosX;
	int nYIndex = vecPosition.y - g_HeightPosY;

	if (nXIndex < 0)
		nXIndex = 0;

	if (nYIndex < 0)
		nYIndex = 0;

	if (nXIndex > 256)
		nXIndex = 255;

	if (nYIndex > 256)
		nYIndex = 255;

	return m_HeightMapData[nYIndex][nXIndex];
}

float TMScene::GroundGetHeight(TMVector2 vecPosition)
{
	float fHeight{ -10000.0f };

	if (m_pGround != nullptr)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pLeftGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pRightGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pUpGround->GetHeight(vecPosition);
		}
		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			fHeight = m_pGround->m_pDownGround->GetHeight(vecPosition);
		}
	}

	return fHeight;
}

D3DCOLORVALUE TMScene::GroundGetColor(TMVector2 vecPosition)
{
	D3DCOLORVALUE color{ 1.0f, 1.0f, 1.0f, 1.0f };

	if (m_pGround != nullptr)
	{
		if (vecPosition.x >= m_pGround->m_vecOffset.x &&
			vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
			vecPosition.y >= m_pGround->m_vecOffset.y &&
			vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pLeftGround
			&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pLeftGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pRightGround
			&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pRightGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pUpGround
			&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pUpGround->GetColor(vecPosition);
		}

		else if (m_pGround->m_pDownGround
			&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
			&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
			&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
			&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
		{
			color = m_pGround->m_pDownGround->GetColor(vecPosition);
		}
	}

	return color;
}

void TMScene::GroundSetColor(TMVector2 vecPosition, unsigned int dwColor)
{
	if (!m_pGround)
		return;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && vecPosition.x < (m_pGround->m_vecOffset.x + 128.0f) &&
		vecPosition.y >= m_pGround->m_vecOffset.y && vecPosition.y < (m_pGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pLeftGround 
		&& vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pLeftGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pRightGround
		&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pRightGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pRightGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pRightGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pUpGround
		&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pUpGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pUpGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pUpGround->SetColor(vecPosition, dwColor);
	}
	else if (m_pGround->m_pDownGround
		&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x
		&& vecPosition.x < (m_pGround->m_pDownGround->m_vecOffset.x + 128.0f)
		&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y
		&& vecPosition.y < (m_pGround->m_pDownGround->m_vecOffset.y + 128.0f))
	{
		m_pGround->m_pDownGround->SetColor(vecPosition, dwColor);
	}
}

int TMScene::GroundIsInWater(TMVector2 vecPosition, float fHeight, float* pfWaterHeight)
{
	if (!m_pGround)
		return 0;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround && 
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround
		&& vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround
		&& vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}
	if (m_pGround->m_pDownGround
		&& vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x && (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) > vecPosition.x
		&& vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y && (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pDownGround->IsInWater(vecPosition, fHeight, pfWaterHeight);
	}

	return 0;
}

int TMScene::GroundIsInWater2(TMVector2 vecPosition, float* pfWaterHeight)
{
	if (!m_pGround)
		return 0;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround && 
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround && 
		vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround && 
		vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
	}
	if (!m_pGround->m_pDownGround || 
		vecPosition.x < m_pGround->m_pDownGround->m_vecOffset.x || (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) <= vecPosition.x || 
		vecPosition.y < m_pGround->m_pDownGround->m_vecOffset.y || (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) <= vecPosition.y)
	{
		return 0;
	}

	return m_pGround->m_pDownGround->IsInWater(vecPosition, (float)GroundGetMask(vecPosition) * 0.1f, pfWaterHeight);
}

float TMScene::GroundGetWaterHeight(TMVector2 vecPosition, float* pfWaterHeight)
{
	if (!m_pGround)
		return -100.0f;

	if (vecPosition.x >= m_pGround->m_vecOffset.x && (float)(m_pGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_vecOffset.y && (float)(m_pGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pLeftGround && 
		vecPosition.x >= m_pGround->m_pLeftGround->m_vecOffset.x && (float)(m_pGround->m_pLeftGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pLeftGround->m_vecOffset.y && (float)(m_pGround->m_pLeftGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pLeftGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pRightGround && 
		vecPosition.x >= m_pGround->m_pRightGround->m_vecOffset.x && (float)(m_pGround->m_pRightGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pRightGround->m_vecOffset.y && (float)(m_pGround->m_pRightGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pRightGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pUpGround && 
		vecPosition.x >= m_pGround->m_pUpGround->m_vecOffset.x && (float)(m_pGround->m_pUpGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pUpGround->m_vecOffset.y && (float)(m_pGround->m_pUpGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
		return m_pGround->m_pUpGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}
	if (m_pGround->m_pDownGround && 
		vecPosition.x >= m_pGround->m_pDownGround->m_vecOffset.x && (float)(m_pGround->m_pDownGround->m_vecOffset.x + 128.0f) > vecPosition.x && 
		vecPosition.y >= m_pGround->m_pDownGround->m_vecOffset.y && (float)(m_pGround->m_pDownGround->m_vecOffset.y + 128.0f) > vecPosition.y)
	{
	    return m_pGround->m_pDownGround->GetWaterHeight(vecPosition, pfWaterHeight);
	}

	return -100.0f;
}

int TMScene::GetMask2(TMVector2 vecPosition)
{
	int nMaskX = (int)(vecPosition.x - (float)g_HeightPosX);
	int nMaskY = (int)(vecPosition.y - (float)g_HeightPosY);

	if (nMaskX >= 0 && nMaskY >= 0 && nMaskX < 256 && nMaskY < 256)
		return m_GateMapData[nMaskY][nMaskX];

	return -10000;
}
