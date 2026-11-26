#include "core/pch.h"
#include "ui/SGrid.h"
#include "core/TMGlobal.h"
#include "ui/SControlContainer.h"
#include "render/TMMesh.h"
#include "scene/TMFieldScene.h"
#include "core/TMUtil.h"
#include "common/ItemEffect.h"

/// @file SGrid.cpp
/// @brief Gerenciamento estrutural da grade de itens (layout de células, CRUD e SGridControlItem).

SGridControlItem* SGridControl::m_pLastMouseOverItem;

SGridControlItem* SGridControl::m_pLastAttachedItem;

SGridControlItem* SGridControl::m_pSellItem;

int SGridControl::m_bNeedUpdate = 1;

char* SGridControl::m_szParamString[49] = {
	g_pMessageStringTable[73],
	g_pMessageStringTable[74],
	g_pMessageStringTable[75],
	g_pMessageStringTable[76],
	g_pMessageStringTable[77],
	g_pMessageStringTable[78],
	g_pMessageStringTable[79],
	g_pMessageStringTable[80],
	g_pMessageStringTable[81],
	g_pMessageStringTable[82],
	g_pMessageStringTable[83],
	g_pMessageStringTable[84],
	g_pMessageStringTable[85],
	g_pMessageStringTable[86],
	g_pMessageStringTable[87],
	g_pMessageStringTable[88],
	g_pMessageStringTable[89],
	g_pMessageStringTable[174],
	g_pMessageStringTable[90],
	g_pMessageStringTable[91],
	g_pMessageStringTable[92],
	g_pMessageStringTable[93],
	g_pMessageStringTable[94],
	g_pMessageStringTable[95],
	g_pMessageStringTable[96],
	g_pMessageStringTable[97],
	g_pMessageStringTable[98],
	g_pMessageStringTable[99],
	g_pMessageStringTable[140],
	g_pMessageStringTable[100],
	g_pMessageStringTable[101],
	g_pMessageStringTable[102],
	g_pMessageStringTable[103],
	g_pMessageStringTable[104],
	g_pMessageStringTable[105],
	g_pMessageStringTable[128],
	g_pMessageStringTable[129],
	g_pMessageStringTable[130],
	g_pMessageStringTable[80],
	g_pMessageStringTable[79],
	g_pMessageStringTable[104],
	g_pMessageStringTable[79],
	g_pMessageStringTable[162],
	g_pMessageStringTable[163],
	g_pMessageStringTable[164],
	g_pMessageStringTable[165],
	g_pMessageStringTable[166],
	g_pMessageStringTable[169],
	g_pMessageStringTable[171]
};

SGridControl::SGridControl(unsigned int inTextureSetIndex, int inRowGridCount, int inColumnGridCount, float inX, float inY, float inWidth, float inHeight, TMEITEMTYPE type)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_eItemType = type;
	m_nRowGridCount = inRowGridCount;
	m_nColumnGridCount = inColumnGridCount;
	m_bDrawGrid = 0;
	m_nNumItem = 0;
	m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_dwMerchantID = 0;
	m_dwLastBuyTime = 0;
	m_GCEnable = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, -2, 0.0f, 0.0f, inWidth, inHeight, 0, 0xFFFF0000);
	m_dwEnableColor = 0;

	m_vecPickupedPos = IVector2(0, 0);
	m_vecPickupedSize = IVector2(0, 0);

	m_pbFilled = new int[m_nColumnGridCount * m_nRowGridCount];
	m_GCGrid = new GeomControl[inColumnGridCount * inRowGridCount];

	if (m_GCGrid)
	{
		for (int i = 0; i < inColumnGridCount * inRowGridCount; ++i)
		{
			m_GCGrid[i].nTextureSetIndex = -489;
			m_GCGrid[i].nTextureIndex = i;
			m_GCGrid[i].nWidth = inWidth / (float)m_nColumnGridCount;
			m_GCGrid[i].nHeight = inHeight / (float)m_nRowGridCount;
			m_GCGrid[i].dwColor = -1;
			m_GCGrid[i].eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
		}
	}

	memset(m_pbFilled, 0, m_nColumnGridCount * sizeof(int) * m_nRowGridCount);
	memset(m_pItemList, 0, sizeof(m_pItemList));
	m_nTradeMoney = 0;
	m_dwLastSortTime = 0;
}

SGridControl::~SGridControl()
{
	Empty();

	auto pControlContainer = g_pCurrentScene->m_pControlContainer;
	if (pControlContainer)
	{
		if (m_bDrawGrid == 1)
		{
			for (int nY = 0; nY < m_nRowGridCount; ++nY)
			{
				for (int nX = 0; nX < m_nColumnGridCount; ++nX)
				{
					if (!m_pbFilled[nX + m_nColumnGridCount * nY]
						&& m_GCGrid[nX + m_nColumnGridCount * nY].nLayer >= 0)
					{
						RemoveRenderControlItem(
							pControlContainer->m_pDrawControl,
							&m_GCGrid[nX + m_nColumnGridCount * nY],
							m_GCGrid[nX + m_nColumnGridCount * nY].nLayer);
					}
				}
			}
		}
		if (m_GCEnable.nLayer >= 0)
			RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCEnable, m_GCEnable.nLayer);
	}

	SAFE_DELETE_ARRAY(m_pbFilled);
	SAFE_DELETE_ARRAY(m_GCGrid);
}

void SGridControl::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	m_GCPanel.nPosX = ivParentPos.x + m_nPosX;
	m_GCPanel.nPosY = ivParentPos.y + m_nPosY;
	if (m_GCPanel.nTextureSetIndex >= 0)
		SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);

	if (m_bDrawGrid == 1)
	{
		for (int nY = 0; nY < m_nRowGridCount; ++nY)
		{
			for (int nX = 0; nX < m_nColumnGridCount; ++nX)
			{
				if (m_pbFilled[nX + m_nColumnGridCount * nY])
					continue;

				m_GCGrid[nX + m_nColumnGridCount * nY].nPosX = (float)(ivParentPos.x + m_nPosX)
					+ (float)((float)nX
						* m_GCGrid[nX + m_nColumnGridCount * nY].nWidth);
				m_GCGrid[nX + m_nColumnGridCount * nY].nPosY = (float)(ivParentPos.y + m_nPosY)
					+ (float)((float)nY
						* m_GCGrid[nX + m_nColumnGridCount * nY].nHeight);
				m_GCGrid[nX + m_nColumnGridCount * nY].nLayer = inParentLayer;
				AddRenderControlItem(pDrawList, &m_GCGrid[nX + m_nColumnGridCount * nY], inParentLayer);
			}
		}
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pGridCurrent = m_pItemList[i];
			if (!pGridCurrent)
				continue;

			TMVector2 vecPos = TMVector2((ivParentPos.x + m_nPosX) + (((float)pGridCurrent->m_nCellIndexX * m_nWidth) / (float)m_nColumnGridCount),
				(ivParentPos.y + m_nPosY) + (((float)pGridCurrent->m_nCellIndexY * m_nHeight) / (float)m_nRowGridCount));

			pGridCurrent->FrameMove2(pDrawList, vecPos, inParentLayer, 0);

			if (pGridCurrent->m_fTimer <= 0.0f || pGridCurrent->m_fTimer >= 1.0f)
				continue;

			pGridCurrent->m_GCEnable.nPosX = ivParentPos.x + m_nPosX + (float)((float)pGridCurrent->m_nCellIndexX * BASE_ScreenResize(m_GCGrid->nWidth));
			pGridCurrent->m_GCEnable.nPosY = ivParentPos.y + m_nPosY + (float)((float)pGridCurrent->m_nCellIndexY * BASE_ScreenResize(m_GCGrid->nHeight));
			pGridCurrent->m_GCEnable.nWidth = (float)((float)(24 * pGridCurrent->m_nCellWidth)
				* RenderDevice::m_fWidthRatio)
				* (float)(1.0f - pGridCurrent->m_fTimer);

			if (pGridCurrent->m_GCEnable.nWidth > 0.0099999998f && pGridCurrent->m_GCEnable.nWidth < 2.0f)
				pGridCurrent->m_GCEnable.nWidth = 2.0f;

			pGridCurrent->m_GCEnable.nHeight = (float)(24 * pGridCurrent->m_nCellHeight)
				* RenderDevice::m_fHeightRatio;

			pGridCurrent->m_GCEnable.nLayer = inParentLayer;
			pGridCurrent->m_GCEnable.dwColor = 0xAA000000;
			AddRenderControlItem(pDrawList, &pGridCurrent->m_GCEnable, inParentLayer);
		}
		return;
	}
	if (m_eItemType != TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType == TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType == TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType == TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType == TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		if (!m_pItemList[0])
			return;

		if (m_eGridType == TMEGRIDTYPE::GRID_TRADEOP || m_eGridType == TMEGRIDTYPE::GRID_TRADEMY ||
			m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2)
		{
			m_pItemList[0]->m_GCEnable.nPosX = ivParentPos.x + m_nPosX;
			m_pItemList[0]->m_GCEnable.nPosY = ivParentPos.y + m_nPosY;
			m_pItemList[0]->m_GCEnable.nWidth = m_nWidth - 4.0f;
			m_pItemList[0]->m_GCEnable.nHeight = m_nHeight - 3.0f;
			m_pItemList[0]->m_GCEnable.nLayer = inParentLayer;
			m_pItemList[0]->m_GCEnable.dwColor = 0x0FF000000;
			AddRenderControlItem(pDrawList, &m_pItemList[0]->m_GCEnable, inParentLayer);
		}
		TMVector2 vecPos = TMVector2((ivParentPos.x + m_nPosX)
			+ ((float)(m_nWidth - m_pItemList[0]->m_GCObj.nWidth) / 2.0f),
			(ivParentPos.y + m_nPosY)
			+ ((float)(m_nHeight - m_pItemList[0]->m_GCObj.nHeight) / 2.0f));

		m_pItemList[0]->FrameMove2(pDrawList, vecPos, inParentLayer, 0);
		return;
	}

	for (int j = 0; j < m_nNumItem; ++j)
	{
		auto pGridCurrent = m_pItemList[j];
		if (!pGridCurrent)
			continue;

		TMVector2 vecPos = TMVector2((ivParentPos.x + m_nPosX)
			+ ((float)((float)pGridCurrent->m_nCellIndexX * m_nWidth) / (float)m_nColumnGridCount),
			(float)(ivParentPos.y + m_nPosY)
			+ ((float)((float)pGridCurrent->m_nCellIndexY * m_nHeight) / (float)m_nRowGridCount));

		pGridCurrent->FrameMove2(pDrawList, vecPos, inParentLayer, 0);
		if (pGridCurrent->m_GCObj.dwColor != 0xFFFF0000)
			continue;

		pGridCurrent->m_GCEnable.nPosX = (ivParentPos.x + m_nPosX) + (float)((float)pGridCurrent->m_nCellIndexX * BASE_ScreenResize(m_GCGrid->nWidth));
		pGridCurrent->m_GCEnable.nPosY = (ivParentPos.y + m_nPosY) + (float)((float)pGridCurrent->m_nCellIndexY * BASE_ScreenResize(m_GCGrid->nHeight));

		if (m_bDrawGrid)
		{
			pGridCurrent->m_GCEnable.nWidth = BASE_ScreenResize((float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellWidth));
			pGridCurrent->m_GCEnable.nHeight = BASE_ScreenResize((float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellHeight));
		}
		else
		{
			pGridCurrent->m_GCEnable.nWidth = (float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellWidth);
			pGridCurrent->m_GCEnable.nHeight = (float)(SControl::m_nGridCellSize * pGridCurrent->m_nCellHeight);
		}

		pGridCurrent->m_GCEnable.nLayer = inParentLayer;
		pGridCurrent->m_GCEnable.dwColor = 0x33FF0000;
		AddRenderControlItem(pDrawList, &pGridCurrent->m_GCEnable, inParentLayer);
	}
}

int SGridControl::CanItAdd(int* bFilledBuffer, int inCellIndexX, int inCellIndexY, int inCellWidth, int inCellHeight)
{
	if (!bFilledBuffer)
		return 0;
	if (inCellWidth + inCellIndexX > m_nColumnGridCount)
		return 0;
	if (inCellHeight + inCellIndexY > m_nRowGridCount)
		return 0;
	for (int nY = 0; nY < inCellHeight; ++nY)
	{
		for (int nX = 0; nX < inCellWidth; ++nX)
		{
			if (bFilledBuffer[nX + inCellIndexX + m_nColumnGridCount * (nY + inCellIndexY)] == 1)
				return 0;
		}
	}

	return 1;
}

int SGridControl::AddItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	for (int nY = 0; nY < ipNewItem->m_nCellHeight; ++nY)
	{
		for (int nX = 0; nX < ipNewItem->m_nCellWidth; ++nX)
		{
			if (nX + inCellIndexX < m_nColumnGridCount && nY + inCellIndexY < m_nRowGridCount)
				m_pbFilled[nX + inCellIndexX + m_nColumnGridCount * (nY + inCellIndexY)] = 1;
		}
	}

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	m_pItemList[m_nNumItem++] = ipNewItem;

	TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipNewItem->m_GCObj.n3DObjIndex, 0, 180000);
	if (pMesh)
	{
		float fLen = pMesh->m_fMaxZ;
		if (fLen > ((float)ipNewItem->m_nCellHeight * 0.3f))
			ipNewItem->m_GCObj.fScale = ((float)ipNewItem->m_nCellHeight * 0.3f) / fLen;
	}

	return 1;
}

int SGridControl::AddSkillItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	for (int nY = 0; nY < ipNewItem->m_nCellHeight; ++nY)
	{
		for (int nX = 0; nX < ipNewItem->m_nCellWidth; ++nX)
		{
			if (nX + inCellIndexX < m_nColumnGridCount && nY + inCellIndexY < m_nRowGridCount)
				m_pbFilled[nX + inCellIndexX + m_nColumnGridCount * (nY + inCellIndexY)] = 1;
		}
	}

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	ipNewItem->m_nWidth = 24.0f * RenderDevice::m_fWidthRatio;
	ipNewItem->m_nHeight = 24.0f * RenderDevice::m_fHeightRatio;
	m_pItemList[m_nNumItem++] = ipNewItem;

	TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipNewItem->m_GCObj.n3DObjIndex, 0, 180000);
	if (pMesh)
	{
		float fLen = pMesh->m_fMaxZ;
		if (fLen > ((float)ipNewItem->m_nCellHeight * 0.3f))
			ipNewItem->m_GCObj.fScale = ((float)ipNewItem->m_nCellHeight * 0.3f) / fLen;
	}

	return 1;
}

int SGridControl::SetItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY)
{
	for (int nY = 0; nY < ipNewItem->m_nCellHeight; ++nY)
	{
		for (int nX = 0; nX < ipNewItem->m_nCellWidth; ++nX)
		{
			if (nX + inCellIndexX < m_nColumnGridCount && nY + inCellIndexY < m_nRowGridCount)
				m_pbFilled[nX + inCellIndexX + m_nColumnGridCount * (nY + inCellIndexY)] = 1;
		}
	}

	ipNewItem->SetGridControl(this);
	ipNewItem->m_nCellIndexX = inCellIndexX;
	ipNewItem->m_nCellIndexY = inCellIndexY;
	m_pItemList[m_nNumItem++] = ipNewItem;

	TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipNewItem->m_GCObj.n3DObjIndex, 0, 180000);
	if (pMesh)
	{
		float fLen = pMesh->m_fMaxZ;
		if (fLen > ((float)ipNewItem->m_nCellHeight * 0.3f))
			ipNewItem->m_GCObj.fScale = ((float)ipNewItem->m_nCellHeight * 0.3f) / fLen;
	}

	return 1;
}

IVector2 SGridControl::AddItemInEmpty(SGridControlItem* ipNewItem)
{
	IVector2 vec{ -1, -1 };
	for (int nY = 0; nY <= m_nRowGridCount - ipNewItem->m_nCellHeight; ++nY)
	{
		for (int nX = 0; nX <= m_nColumnGridCount - ipNewItem->m_nCellWidth; ++nX)
		{
			if (AddItem(ipNewItem, nX, nY) == 1)
			{
				vec.x = nX;
				vec.y = nY;				
				return vec;
			}
		}
	}

	return vec;
}

IVector2 SGridControl::CanAddItemInEmpty(int nWidth, int nHeight)
{
	IVector2 vec{ -1, -1 };

	for (int nX = 0; nX <= m_nColumnGridCount - nWidth; ++nX)
	{
		for (int nY = 0; nY <= m_nRowGridCount - nHeight; ++nY)
		{
			if (CanItAdd(m_pbFilled, nX, nY, nWidth, nHeight) == 1)
			{
				vec.x = nX;
				vec.y = nY;				
				return vec;
			}
		}
	}

	return vec;
}

int SGridControl::CanChangeItem(SGridControlItem* ipNewItem, int inCellIndexX, int inCellIndexY, int bOnlyCheck)
{
	if (IsSkill(ipNewItem->m_pItem->sIndex) == 1)
		return 0;

	auto pMobData = &g_pObjectManager->m_stMobData;
	short sType = CheckType(m_eItemType, m_eGridType);
	short sPos = CheckPos(m_eItemType);

	if (!sType)
	{
		unsigned int nItemType = BASE_GetItemAbility(ipNewItem->m_pItem, 17);
		if (m_dwControlID == 65556)
		{
			if (nItemType == 128)
			{
				return BASE_CanEquip(
					ipNewItem->m_pItem,
					&pMobData->CurrentScore,
					sPos,
					pMobData->Equip[0].sIndex,
					pMobData->Equip,
					g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
					pMobData->HasSoulSkill());
			}
			if (nItemType == 64 || nItemType == 192)
			{
				if (pMobData->Equip[6].sIndex <= 0)
				{
					if (BASE_CanEquip(
						ipNewItem->m_pItem,
						&pMobData->CurrentScore,
						6,
						pMobData->Equip[0].sIndex,
						pMobData->Equip,
						g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
						pMobData->HasSoulSkill())
						&& !bOnlyCheck)
					{
						short sDestType = CheckType(ipNewItem->m_pGridControl->m_eItemType, ipNewItem->m_pGridControl->m_eGridType);
						short sDestPos = CheckPos(ipNewItem->m_pGridControl->m_eItemType);
						if (sDestPos == -1)
							sDestPos = ipNewItem->m_nCellIndexX + 5 * ipNewItem->m_nCellIndexY;

						MSG_SwapItem stSwapItem{};
						stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
						stSwapItem.Header.Type = MSG_SwapItem_Opcode;
						stSwapItem.SourType = 0;
						stSwapItem.SourPos = 6;
						stSwapItem.DestType = static_cast<char>(sDestType);
						stSwapItem.DestPos = static_cast<char>(sDestPos);
						stSwapItem.TargetID = TMFieldScene::m_dwCargoID;
						SendOneMessage((char*)&stSwapItem, 20);
					}

					return 0;
				}

				short sDestType = CheckType(ipNewItem->m_pGridControl->m_eItemType, ipNewItem->m_pGridControl->m_eGridType);
				short sDestPos = CheckPos(ipNewItem->m_pGridControl->m_eItemType);
				if (sDestType == 0 && sDestPos == 6)
					return 0;
			}
		}

		return BASE_CanEquip(
			ipNewItem->m_pItem,
			&pMobData->CurrentScore,
			sPos,
			pMobData->Equip[0].sIndex,
			pMobData->Equip,
			g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex,
			pMobData->HasSoulSkill());
	}
	else if (sType == 1)
		return 1;
	else
	{
		int page = 40 * (m_dwControlID - 67328);
		int nPos = ipNewItem->m_nCellIndexX + 5 * ipNewItem->m_nCellIndexY;

		STRUCT_ITEM stCargo[128]{};
		memcpy(stCargo, g_pObjectManager->m_stItemCargo, sizeof(stCargo));
		if (ipNewItem->m_pGridControl->m_eGridType == TMEGRIDTYPE::GRID_CARGO)
			memset(&stCargo[nPos], 0, sizeof(STRUCT_ITEM));

		return 1;
	}

	return 0;
}

SGridControlItem* SGridControl::PickupItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItem = nullptr;
	int nIndex = -1;

	for (int i = 0; i < m_nNumItem; ++i)
	{
		pItem = m_pItemList[i];
		if (pItem && pItem->PtInItem(inCellIndexX, inCellIndexY) == 1)
		{
			nIndex = i;
			break;
		}
		pItem = nullptr;
	}
	if (!pItem)
		return nullptr;

	for (int nY = pItem->m_nCellIndexY; nY < pItem->m_nCellHeight + pItem->m_nCellIndexY; ++nY)
	{
		for (int nX = pItem->m_nCellIndexX; nX < pItem->m_nCellWidth + pItem->m_nCellIndexX; ++nX)
		{
			if (nX < m_nColumnGridCount && nY < m_nRowGridCount)
				m_pbFilled[nX + m_nColumnGridCount * nY] = 0;
		}
	}
	if (nIndex != -1 && m_nNumItem > nIndex && nIndex >= 0)
	{
		for (int j = nIndex + 1; j < m_nNumItem; ++j)
			m_pItemList[j - 1] = m_pItemList[j];

		m_pItemList[m_nNumItem - 1] = nullptr;
		m_nNumItem--;
	}

	return pItem;
}

SGridControlItem* SGridControl::PickupAtItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItem = nullptr;
	int nIndex = -1;
	for (int i = 0; i < m_nNumItem; ++i)
	{
		pItem = m_pItemList[i];
		if (pItem && pItem->PtAtItem(inCellIndexX, inCellIndexY) == 1)
		{
			nIndex = i;
			break;
		}
		pItem = nullptr;
	}
	if (!pItem)
		return nullptr;

	for (int nY = pItem->m_nCellIndexY; nY < pItem->m_nCellHeight + pItem->m_nCellIndexY; ++nY)
	{
		for (int nX = pItem->m_nCellIndexX; nX < pItem->m_nCellWidth + pItem->m_nCellIndexX; ++nX)
		{
			if (nX < m_nColumnGridCount && nY < m_nRowGridCount)
				m_pbFilled[nX + m_nColumnGridCount * nY] = 0;
		}
	}
	if (nIndex != -1 && m_nNumItem > nIndex && nIndex >= 0)
	{
		for (int j = nIndex + 1; j < m_nNumItem; ++j)
			m_pItemList[j - 1] = m_pItemList[j];

		m_pItemList[m_nNumItem - 1] = nullptr;
		m_nNumItem--;
	}

	return pItem;
}

SGridControlItem* SGridControl::PickupItem(SGridControlItem* ipItem)
{
	for (int i = 0; i < this->m_nNumItem; ++i)
	{
		auto pItem = m_pItemList[i];
		if (pItem == ipItem)
			return PickupItem(pItem->m_nCellIndexX, pItem->m_nCellIndexY);
	}

	return nullptr;
}

SGridControlItem* SGridControl::SelectItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtInItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->SelectThis(1);
				pItemReturn = pItem;
			}
			else
				pItem->SelectThis(0);
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->SelectThis(1);
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtInItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->m_bOver = 1;
				pItemReturn = pItem;
			}
			else
				pItem->m_bOver = 0;
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->m_bOver = 1;;
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetAtItem(int inCellIndexX, int inCellIndexY)
{
	SGridControlItem* pItemReturn = nullptr;
	if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADENONE
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEOP
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY
		&& m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIX4
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXRESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_ITEMMIXNEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_RESULT
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEED
		&& m_eGridType != TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		for (int i = 0; i < m_nNumItem; ++i)
		{
			auto pItem = m_pItemList[i];
			if (m_pItemList[i]->PtAtItem(inCellIndexX, inCellIndexY) == 1)
			{
				pItem->m_bOver = 1;
				pItemReturn = pItem;
			}
			else
				pItem->m_bOver = 0;
		}
	}
	else if (m_pItemList[0])
	{
		m_pItemList[0]->m_bOver = 1;;
		pItemReturn = m_pItemList[0];
	}

	return pItemReturn;
}

SGridControlItem* SGridControl::GetItem(int nCount)
{
	return m_pItemList[nCount];
}

void SGridControl::Empty()
{
	memset(m_pbFilled, 0, m_nColumnGridCount * sizeof(int) * m_nRowGridCount);
	for (int i = 0; i < m_nNumItem; ++i)
	{
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == m_pItemList[i])
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(m_pItemList[i]);		
	}

	m_nNumItem = 0;
}

short SGridControl::CheckType(TMEITEMTYPE eType, TMEGRIDTYPE eGridType)
{
	if (eType != TMEITEMTYPE::ITEMTYPE_NONE)
		return 0;

	if (eGridType == TMEGRIDTYPE::GRID_CARGO)
		return 2;

	return 1;
}

short SGridControl::CheckPos(TMEITEMTYPE eType)
{
	switch (eType)
	{
	case TMEITEMTYPE::ITEMTYPE_HELM:
		return 1;
	case TMEITEMTYPE::ITEMTYPE_COAT:
		return 2;
	case TMEITEMTYPE::ITEMTYPE_PANTS:
		return 3;
	case TMEITEMTYPE::ITEMTYPE_GLOVES:
		return 4;
	case TMEITEMTYPE::ITEMTYPE_BOOTS:
		return 5;
	case TMEITEMTYPE::ITEMTYPE_RIGHT:
		return 7;
	case TMEITEMTYPE::ITEMTYPE_LEFT:
		return 6;
	case TMEITEMTYPE::ITEMTYPE_RING:
		return 8;
	case TMEITEMTYPE::ITEMTYPE_NECKLACE:
		return 9;
	case TMEITEMTYPE::ITEMTYPE_ORB:
		return 10;
	case TMEITEMTYPE::ITEMTYPE_CABUNCLE:
		return 11;
	case TMEITEMTYPE::ITEMTYPE_GUILD:
		return 12;
	case TMEITEMTYPE::ITEMTYPE_EVENT:
		return 13;
	case TMEITEMTYPE::ITEMTYPE_MOUNT:
		return 14;
	case TMEITEMTYPE::ITEMTYPE_MANTUA:
		return 15;
		break;
	}

	return -1;
}

SGridControlItem::SGridControlItem(SGridControl* pParent, STRUCT_ITEM* pItem, float inX, float inY)
	: S3DObj(0, inX, inY, 35.0f, 35.0f)
{
	m_GCEnable = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, -2, 0.0f, 0.0f, 1.0f, 1.0f, 0, 0x33FF0000);
	m_GCText = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0xFFFFFFFF);

	m_pItem = pItem;

	if (m_pItem && pItem->sIndex >= 0 && pItem->sIndex <= 11600)
	{
		m_GCObj.n3DObjIndex = g_pItemList[pItem->sIndex].nIndexMesh;
		m_pGridControl = pParent;
		m_GCText.strString[0] = 0;
		m_GCText.pFont = &m_Font;
		m_bSelect = 0;
		m_fTimer = 1.0f;

		int nSizeIndex = BASE_GetItemAbility(pItem, 33);
		int nType = BASE_GetItemAbility(pItem, 38);
		if (nSizeIndex > 7 || nSizeIndex < 0)
			nSizeIndex = 0;

		m_nCellWidth = g_pItemGridXY[nSizeIndex][0];
		m_nCellHeight = g_pItemGridXY[nSizeIndex][1];
		m_nWidth = BASE_ScreenResize((float)SControl::m_nGridCellSize);
		m_nHeight = BASE_ScreenResize((float)SControl::m_nGridCellSize);

		if (m_pItem->sIndex >= 5000 && m_pItem->sIndex <= 5102
			|| m_pItem->sIndex >= 5400 && m_pItem->sIndex <= 5447)
		{
			m_GCObj.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
			m_GCObj.nTextureSetIndex = 199;
			m_GCObj.nTextureIndex = g_pItemList[m_pItem->sIndex].nIndexTexture;
			m_nWidth = (float)m_nCellWidth * (23.0f * RenderDevice::m_fWidthRatio);
			m_nHeight = (float)m_nCellHeight * (23.0f * RenderDevice::m_fHeightRatio);
			m_GCObj.m_fWidth = (float)m_nCellWidth * (23.0f * RenderDevice::m_fWidthRatio);
			m_GCObj.m_fHeight = (float)m_nCellHeight * (23.0f * RenderDevice::m_fHeightRatio);
			return;
		}

		m_GCObj.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
		m_GCObj.nTextureSetIndex = 526;
		m_GCObj.nTextureIndex = g_itemicon[m_pItem->sIndex] - 1;
		if (m_pItem->sIndex > 6500 && m_pItem->sIndex < 11500)
			m_GCObj.nTextureIndex = g_pItemMixHelp[m_pItem->sIndex].Icon - 1;
		if (m_pItem->sIndex >= 11500 && m_pItem->sIndex <= 11600)
		{
			int INDEX = m_pItem->sIndex - 11500;
			m_GCObj.nTextureIndex = INDEX + 900;
			if (INDEX >= 50)
				m_GCObj.nTextureIndex = 13;
		}

		m_nWidth = 35.0f * RenderDevice::m_fWidthRatio;
		m_nHeight = 35.0f * RenderDevice::m_fHeightRatio;
		m_GCObj.m_fWidth = 35.0f * RenderDevice::m_fWidthRatio;
		m_GCObj.m_fHeight = 35.0f * RenderDevice::m_fHeightRatio;
		int nAmount = BASE_GetItemAmount(pItem);
		if (pItem->sIndex >= 2330 && pItem->sIndex < 2390)
			nAmount = 0;

		m_GCObj.pFont = &m_Font;

		if (nAmount > 0)
		{
			sprintf(m_GCObj.strString, "%2d", nAmount);
			m_GCObj.pFont->SetText(m_GCObj.strString, m_GCObj.dwColor, 0);
		}

		m_GCObj.sLegend = g_pItemList[pItem->sIndex].nGrade;
		int sMultiTexture = BASE_GetItemSanc(pItem);

		if (sMultiTexture > 0 && BASE_GetItemAbility(pItem, 17) > 0)
		{
			sprintf(m_GCObj.strString, "+%d", sMultiTexture);
			if (sMultiTexture < 1 || sMultiTexture > 9)
				m_GCObj.pFont->SetText(m_GCObj.strString, 0xFFFFFF55, 0);
			else
				m_GCObj.pFont->SetText(m_GCObj.strString, 0xFFFFFFFF, 0);
		}
		if (sMultiTexture > 12)
			sMultiTexture = 12;

		if ((g_pItemList[pItem->sIndex].nUnique == 51 || m_GCObj.sLegend) && m_GCObj.sLegend <= 4 && sMultiTexture > 9)
			m_GCObj.sLegend = (unsigned char)BASE_GetItemTenColor(pItem) + 4;

		if (sMultiTexture > 9)
			sMultiTexture = 9;

		if (BASE_GetItemAbility(pItem, 17) > 0)
			m_GCObj.sSanc = sMultiTexture;

		switch (BASE_GetItemColorEffect(pItem))
		{
		case 116:
			m_GCObj.sLegend = 116;
			break;
		case 117:
			m_GCObj.sLegend = 117;
			break;
		case 118:
			m_GCObj.sLegend = 118;
			break;
		case 119:
			m_GCObj.sLegend = 119;
			break;
		case 120:
			m_GCObj.sLegend = 120;
			break;
		case 121:
			m_GCObj.sLegend = 121;
			break;
		case 122:
			m_GCObj.sLegend = 122;
			break;
		case 123:
			m_GCObj.sLegend = 123;
			break;
		case 124:
			m_GCObj.sLegend = 124;
			break;
		case 125:
			m_GCObj.sLegend = 125;
			break;
		}
		if (pItem->sIndex >= 2330 && pItem->sIndex < 2390)
			m_GCObj.sLegend = 0;
	}
}

SGridControlItem::~SGridControlItem()
{
	if (m_pItem && m_pItem->sIndex <= 11600 && m_pItem->sIndex >= 0)
	{
		auto pControlContainer = g_pCurrentScene->m_pControlContainer;
		if (g_pItemList[m_pItem->sIndex].nIndexMesh < 0)
		{
			if (pControlContainer && m_GCObj.nLayer >= 0)
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCObj, m_GCObj.nLayer);
		}
		else if (pControlContainer)
		{
			if (m_GCText.nLayer >= 0)
			{
				if (strlen(m_GCText.strString))
					RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCText, m_GCText.nLayer);
			}
		}
		if (pControlContainer && m_GCEnable.nLayer >= 0)
			RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCEnable, m_GCEnable.nLayer);

		if (m_pItem)
		{
			delete m_pItem;
			m_pItem = nullptr;
		}
	}
}

void SGridControlItem::SelectThis(int bSelect)
{
	m_bSelect = bSelect;
}

int SGridControlItem::IsSelect()
{
	return m_bSelect;
}

SGridControl* SGridControlItem::GetGridControl()
{
	return m_pGridControl;
}

void SGridControlItem::SetGridControl(SGridControl* pGridControl)
{
	m_pGridControl = pGridControl;
}

STRUCT_ITEM* SGridControlItem::GetItem()
{
	return m_pItem;
}

void SGridControlItem::FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag)
{
	if (m_pItem && m_pItem->sIndex <= 11600 && m_pItem->sIndex >= 0)
	{
		m_GCObj.nPosX = ivItemPos.x + m_nPosX;
		m_GCObj.nPosY = ivItemPos.y + m_nPosY;
		m_GCObj.nWidth = m_nWidth;
		m_GCObj.nHeight = m_nHeight;
		m_GCObj.nLayer = inParentLayer;
		AddRenderControlItem(pDrawList, &m_GCObj, inParentLayer);
	}
}

int SGridControlItem::PtInItem(int inPosX, int inPosY)
{
	return inPosX >= m_nCellIndexX
		&& inPosY >= m_nCellIndexY
		&& inPosX < m_nCellWidth + m_nCellIndexX
		&& inPosY < m_nCellHeight + m_nCellIndexY;
}

int SGridControlItem::PtAtItem(int inPosX, int inPosY)
{
	return inPosX == m_nCellIndexX && inPosY == m_nCellIndexY;
}
