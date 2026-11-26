#include "core/pch.h"
#include "scene/TMFieldScene.h"
#include "core/TMGlobal.h"
#include "core/TMLog.h"
#include "audio/dsutil.h"
#include "audio/DirShow.h"
#include "ui/SControlContainer.h"
#include "ui/SGrid.h"
#include "gameplay/Mission.h"
#include "gameplay/MrItemMix.h"
#include "world/TMGround.h"
#include "entities/TMHuman.h"
#include "scene/TMObjectContainer.h"
#include "render/TMCamera.h"
#include "world/TMSun.h"
#include "world/TMSky.h"
#include "world/TMSnow.h"
#include "world/TMRain.h"
#include "render/TMSkinMesh.h"
#include "effects/TMEffectMesh.h"
#include "effects/TMEffectBillBoard2.h"
#include "effects/TMEffectBillBoard.h"
#include "core/TMUtil.h"
#include "world/TMHouse.h"
#include "effects/TMEffectBillBoard4.h"
#include "skills/TMSkillMagicArrow.h"
#include "effects/TMEffectStart.h"
#include "skills/TMSkillJudgement.h"
#include "skills/TMSkillTownPortal.h"
#include "effects/TMEffectLevelUp.h"
#include "effects/TMEffectSkinMesh.h"
#include "common/ItemEffect.h"
#include "effects/TMEffectSWSwing.h"
#include "effects/TMEffectSpark.h"
#include "skills/TMSkillHolyTouch.h"
#include "effects/TMEffectParticle.h"
#include "skills/TMSkillMeteorStorm.h"
#include "skills/TMSkillThunderBolt.h"
#include "skills/TMSkillSlowSlash.h"
#include "skills/TMSkillMagicShield.h"
#include "skills/TMSkillFreezeBlade.h"
#include "entities/TMArrow.h"
#include "entities/TMShade.h"
#include "skills/TMSkillPoison.h"
#include "render/TMFont3.h"
#include "skills/TMSkillHeavenDust.h"
#include "skills/TMSkillFlash.h"
#include "entities/TMItem.h"
#include "entities/TMCannon.h"
#include "effects/TMEffectCharge.h"
#include "skills/TMSkillExplosion2.h"
#include "effects/TMEffectDust.h"
#include "world/TMGate.h"
#include <WinInet.h>

/// @file TMFieldScene_PacketItems.cpp
/// @brief Pacotes de itens e comércio: criar/largar/pegar/atualizar itens, troca de slots,
///        lojas (NPC e RMB), compra/venda/recompra, cargo, venda automática, combinação e cápsulas.
///
/// Extraído de `TMFieldScene_Packets.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

int TMFieldScene::OnPacketItemSold(MSG_STANDARDPARM2* pStd)
{
	auto pPanel = this->m_pAutoTrade;
	if (pPanel && pPanel->IsVisible() == 1 && pStd->Parm1 == m_stAutoTrade.TargetID)
	{
		auto pItem = m_pGridAutoTrade[pStd->Parm2]->PickupAtItem(0, 0);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pItem)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pItem);
	}

	return 1;
}

int TMFieldScene::OnPacketUpdateCargoCoin(MSG_STANDARDPARM* pStd)
{
	g_pObjectManager->m_nCargoCoin = pStd->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketCreateItem(MSG_CreateItem* pMsg)
{
	auto pOldItem = (TMItem*)g_pObjectManager->GetItemByID(pMsg->ItemID);
	
	if (BASE_GetItemAbility(&pMsg->Item, 34) > 0)
	{
		TMGate* pItem = nullptr;

		if (pOldItem)
			pItem = static_cast<TMGate*>(pOldItem);
		else
		{
			pItem = new TMGate();
		}

		if (!pItem)
			return 1;

		pItem->InitItem(pMsg->Item);
		pItem->InitGate(pMsg->Item);

		pItem->m_dwID = pMsg->ItemID;
		pItem->m_nMaskIndex = 0;
		pItem->InitObject();
		pItem->InitAngle(0.0f, ((float)pMsg->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

		float fX = (float)pMsg->GridX + 0.5f;
		float fY = (float)pMsg->GridY + 0.5f;

		float fHeight = m_pGround->GetHeight(TMVector2(fX, fY));

		if (fHeight < -500.0f)
		{
			auto pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
			if (pOtherGround)
				fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
			else
			{
				TMScene::FrameMove(0);
				pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
				if (pOtherGround)
					fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
			}
		}

		pItem->InitPosition(fX, fHeight, fY);
		pItem->m_sAuth = 1;

		int nMaskIndex = BASE_GetItemAbility(&pMsg->Item, 34);
		BASE_UpdateItem2(nMaskIndex, 1, (unsigned char)pMsg->State, pMsg->GridX, pMsg->GridY, (char*)m_HeightMapData,
			(int)(pItem->m_fAngle / D3DXToRadian(90)), pMsg->Height);

		pItem->SetState((EGATE_STATE)pMsg->State);
		if (!pOldItem)
			m_pItemContainer->AddChild(pItem);

		return 1;
	}

	TMItem* pItem = nullptr;
	if (pOldItem)
		pItem = pOldItem;
	else if (g_pItemList[pMsg->Item.sIndex].nIndexMesh == 1607)
	{
		pItem = new TMCannon();
		pItem->m_dwObjType = 1607;
		pMsg->Rotate = 1;
	}
	else
	{
		pItem = new TMItem();
	}

	if (!pItem)
		return 1;

	pItem->InitItem(pMsg->Item);
	pItem->m_dwID = pMsg->ItemID;
	pItem->m_nMaskIndex = 0;
	pItem->InitObject();
	pItem->InitAngle(0.0f, ((float)pMsg->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

	float fX = (float)pMsg->GridX + 0.5f;
	float fY = (float)pMsg->GridY + 0.5f;

	float fHeight = GroundGetMask(TMVector2(fX, fY)) * 0.1f;
	pItem->InitPosition(fX, fHeight + 0.1f, fY);

	if (!pOldItem)
		m_pItemContainer->AddChild(pItem);

	if (pMsg->Create == 1)
	{
		if (BASE_GetItemAbility(&pMsg->Item, 38) == 2)
			GetSoundAndPlay(44, 0, 0);
		else if (pMsg->Item.sIndex == 412 || pMsg->Item.sIndex == 413 || pMsg->Item.sIndex == 4141 ||
			pMsg->Item.sIndex == 419 || pMsg->Item.sIndex == 420)
		{
			GetSoundAndPlay(48, 0, 0);
		}
		else if (pMsg->Item.sIndex == 747)
		{
			GetSoundAndPlay(306, 0, 0);
		}
		else
			GetSoundAndPlay(45, 0, 0);
	}

	return 1;
}

int TMFieldScene::OnPacketCNFDropItem(MSG_CNFDropItem* pMsg)
{
	SGridControlItem* pGridItem = nullptr;
	if (pMsg->SourType == 0)
	{
		SGridControl* pGridList[16]{};
		pGridList[0] = nullptr;
		pGridList[1] = m_pGridHelm;
		pGridList[2] = m_pGridCoat;
		pGridList[3] = m_pGridPants;
		pGridList[4] = m_pGridGloves;
		pGridList[5] = m_pGridBoots;
		pGridList[6] = m_pGridLeft;
		pGridList[7] = m_pGridRight;
		pGridList[8] = m_pGridRing;
		pGridList[9] = m_pGridNecklace;
		pGridList[10] = m_pGridOrb;
		pGridList[11] = m_pGridCabuncle;
		pGridList[12] = m_pGridGuild;
		pGridList[13] = m_pGridEvent;
		pGridList[14] = m_pGridDRing;
		pGridList[15] = m_pGridMantua;

		if (pGridList[pMsg->SourPos])
			pGridItem = pGridList[pMsg->SourPos]->PickupItem(0, 0);

		memset(&g_pObjectManager->m_stMobData.Equip[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pMsg->SourType == 1)
	{
		auto pGridItem = m_pGridInv->PickupAtItem(pMsg->SourPos % 5, pMsg->SourPos / 5);
		memset(&g_pObjectManager->m_stMobData.Carry[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pMsg->SourType == 2)
	{
		pGridItem = static_cast<SGridControl*>(m_pControlContainer->FindControl(65690))->PickupAtItem(pMsg->SourPos % 5, pMsg->SourPos / 5);
		memset(&g_pObjectManager->m_stItemCargo[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}

	g_pCursor->DetachItem();
	SAFE_DELETE(pGridItem);

	m_pMyHuman->m_sFamiliar = g_pObjectManager->m_stMobData.Equip[13].sIndex;
	GetSoundAndPlay(45, 0, 0);

	UpdateScoreUI(0);
	UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketCNFGetItem(MSG_CNFGetItem* pMsg)
{
	auto pGrid = m_pGridInv;
	auto pStructItem = new STRUCT_ITEM;

	memcpy(pStructItem, &pMsg->Item, sizeof(pMsg->Item));
	if (BASE_GetItemAbility((STRUCT_ITEM*)pStructItem, 38) == 2)
	{
		int coin = (unsigned char)BASE_GetItemAbility(pStructItem, 36) << 8;
		int tempb = BASE_GetItemAbility(pStructItem, 37);

		g_pObjectManager->m_stMobData.Coin += coin + tempb;

		char szMoney[64]{};
		sprintf(szMoney, "%10d", g_pObjectManager->m_stMobData.Coin);
		m_pMoney1->m_cComma = 2;
		m_pMoney1->SetText(szMoney, 0);
		m_pMoney2->m_cComma = 2;
		m_pMoney2->SetText(szMoney, 0);

		sprintf(szMoney, "%10d", g_pObjectManager->m_stMobData.Coin - m_nBet);
		m_pMoney3->m_cComma = 2;
		m_pMoney3->SetText(szMoney, 0);
	}
	else
	{
		pGrid->AddItem(new SGridControlItem(0, pStructItem, 0.0f, 0.0f), pMsg->DestPos % 5, pMsg->DestPos / 5);
		memcpy(&g_pObjectManager->m_stMobData.Carry[pMsg->DestPos], pStructItem, sizeof(STRUCT_ITEM));
	}

	GetSoundAndPlay(45, 0, 0);
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketUpdateItem(MSG_UpdateItem* pMsg)
{
	auto pItem = (TMGate*)g_pObjectManager->GetItemByID(pMsg->ItemID);
	if (pItem && BASE_GetItemAbility(&pItem->m_stItem, 34) > 0)
	{
		STRUCT_ITEM stItem{};
		stItem.sIndex = pItem->m_stItem.sIndex;
		
		int nMaskIndex = BASE_GetItemAbility(&stItem, 34);
		int nState = static_cast<int>(pItem->m_eState);
		if (nState > static_cast<int>(EGATE_STATE::EGATE_LOCKED))
			nState -= static_cast<int>(EGATE_STATE::EGATE_LOCKED);

		pItem->m_sAuth = 1;
		BASE_UpdateItem2(nMaskIndex, nState, pMsg->State, (int)pItem->m_vecPosition.x, (int)pItem->m_vecPosition.y,	(char*)m_HeightMapData,
			(int)(pItem->m_fAngle / D3DXToRadian(90)), pMsg->Height);

		pItem->SetState((EGATE_STATE)(pMsg->State + static_cast<int>(EGATE_STATE::EGATE_LOCKED)));
	}

	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketRemoveItem(MSG_STANDARDPARM* pStd)
{
	m_pMouseOverItem = nullptr;
	g_pObjectManager->DeleteObject(pStd->Parm);
	return 1;
}

int TMFieldScene::OnPacketAutoTrade(MSG_STANDARD* pStd)
{
	auto pAutoTrade = reinterpret_cast<MSG_AutoTrade*>(pStd);

	auto pTitle = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_TITLE));
	auto pName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_ID));

	pAutoTrade->Desc[23] = 0;
	pAutoTrade->Desc[22] = 0;

	if (pTitle)
		pTitle->SetText(pAutoTrade->Desc, 0);

	auto pHuman = static_cast<TMHuman*>(g_pObjectManager->GetHumanByID(pAutoTrade->TargetID));

	memcpy(&m_stAutoTrade, pAutoTrade, sizeof(m_stAutoTrade));

	if (pHuman)
		pName->SetText(pHuman->m_szName, 1);

	for (int i = 0; i < 10; ++i)
	{
		SGridControl* pGrid = m_pGridAutoTrade[i];

		pGrid->m_nTradeMoney = pAutoTrade->TradeMoney[i];

		SGridControlItem* pItem = pGrid->PickupAtItem(0, 0);

		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pItem)
			g_pCursor->m_pAttachedItem = nullptr;

		if (pItem)
			delete pItem;

		if (pAutoTrade->Item[i].sIndex > 0)
		{
			auto pstItem = new STRUCT_ITEM();

			if (pstItem)
			{
				memcpy(pstItem, &pAutoTrade->Item[i], sizeof(STRUCT_ITEM));

				auto ipNewItem = new SGridControlItem(pGrid, pstItem, 0.0f, 0.0f);

				if (ipNewItem)
					pGrid->AddItem(ipNewItem, 0, 0);
			}
		}
	}

	auto pButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_RUN));

	if (pButton)
		pButton->SetVisible(0);

	if (pAutoTrade->TargetID == m_pMyHuman->m_dwID)
	{
		sprintf(m_pMyHuman->m_TradeDesc, pAutoTrade->Desc);

		m_pMyHuman->m_pAutoTradeDesc->SetText(m_pMyHuman->m_TradeDesc, 0);
	}
	else
	{
		SetVisibleAutoTrade(1, 0);
	}

	return 1;
}

/// Atualiza grids locais (Equip, Carry, Cargo) apos movimentacao de item confirmada pelo servidor.
int TMFieldScene::OnPacketSwapItem(MSG_STANDARD* pStd)
{
	MSG_SwapItem* pSwapItem = reinterpret_cast<MSG_SwapItem*>(pStd);

	SGridControl* pSrcGrid = nullptr;
	SGridControl* pDestGrid = nullptr;
	SGridControlItem* pSrcItem = nullptr;
	SGridControlItem* pDestItem = nullptr;

	SGridControl* pGridSrc[16]{};

	if (!pSwapItem->SourType)
	{
		pGridSrc[0] = m_pGridInv;
		pGridSrc[1] = m_pGridHelm;
		pGridSrc[2] = m_pGridCoat;
		pGridSrc[3] = m_pGridPants;
		pGridSrc[4] = m_pGridGloves;
		pGridSrc[5] = m_pGridBoots;
		pGridSrc[6] = m_pGridLeft;
		pGridSrc[7] = m_pGridRight;
		pGridSrc[8] = m_pGridRing;
		pGridSrc[9] = m_pGridNecklace;
		pGridSrc[10] = m_pGridOrb;
		pGridSrc[11] = m_pGridCabuncle;
		pGridSrc[12] = m_pGridGuild;
		pGridSrc[13] = m_pGridEvent;
		pGridSrc[14] = m_pGridDRing;
		pGridSrc[15] = m_pGridMantua;
		pSrcGrid = pGridSrc[pSwapItem->SourPos];
		pSrcItem = pSrcGrid->PickupItem(0, 0);

		memset(&g_pObjectManager->m_stMobData.Equip[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pSwapItem->SourType == 1)
	{
		pSrcGrid = m_pGridInvList[pSwapItem->SourPos / 15];
		pSrcItem = pSrcGrid->PickupAtItem(pSwapItem->SourPos % 15 % 5, pSwapItem->SourPos % 15 / 5);
		memset(&g_pObjectManager->m_stMobData.Carry[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pSwapItem->SourType == 2)
	{
		pSrcGrid = m_pCargoGridList[pSwapItem->SourPos / 40];
		pSrcItem = pSrcGrid->PickupAtItem(pSwapItem->SourPos % 40 % 5, pSwapItem->SourPos % 40 / 5);
		memset(&g_pObjectManager->m_stItemCargo[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
	}

	SGridControl* pGridDest[16]{};
	if (!pSwapItem->DestType)
	{
		pGridDest[0] = m_pGridInv;
		pGridDest[1] = m_pGridHelm;
		pGridDest[2] = m_pGridCoat;
		pGridDest[3] = m_pGridPants;
		pGridDest[4] = m_pGridGloves;
		pGridDest[5] = m_pGridBoots;
		pGridDest[6] = m_pGridLeft;
		pGridDest[7] = m_pGridRight;
		pGridDest[8] = m_pGridRing;
		pGridDest[9] = m_pGridNecklace;
		pGridDest[10] = m_pGridOrb;
		pGridDest[11] = m_pGridCabuncle;
		pGridDest[12] = m_pGridGuild;
		pGridDest[13] = m_pGridEvent;
		pGridDest[14] = m_pGridDRing;
		pGridDest[15] = m_pGridMantua;

		pDestGrid = pGridDest[pSwapItem->DestPos];
		pDestItem = pDestGrid->PickupItem(0, 0);

		memset(&g_pObjectManager->m_stMobData.Equip[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pSwapItem->DestType == 1)
	{
		pDestGrid = m_pGridInvList[pSwapItem->DestPos / 15];
		pDestItem = pDestGrid->PickupAtItem(pSwapItem->DestPos % 15 % 5, pSwapItem->DestPos % 15 / 5);
		memset(&g_pObjectManager->m_stMobData.Carry[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pSwapItem->DestType == 2)
	{
		pDestGrid = m_pCargoGridList[pSwapItem->DestPos / 40];
		pDestItem = pDestGrid->PickupAtItem(pSwapItem->DestPos % 40 % 5,
			 pSwapItem->DestPos % 40 / 5);
		memset(&g_pObjectManager->m_stItemCargo[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
	}

	if (pDestItem)
	{
		sizeof(STRUCT_MOB);
		if (!pSwapItem->SourType)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{
				pSrcGrid->AddItem(pDestItem, 0, 0);
				if (pDestItem)
					memcpy(&g_pObjectManager->m_stMobData.Equip[pSwapItem->SourPos], pDestItem->m_pItem, sizeof(STRUCT_ITEM));				
				else
					memset(&g_pObjectManager->m_stMobData.Equip[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pDestItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pDestItem);
			}
		}
		else if (pSwapItem->SourType == 1)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{				
				pSrcGrid->AddItem(pDestItem, pSwapItem->SourPos % 15 % 5, pSwapItem->SourPos % 15 / 5);
				if (pDestItem)
					memcpy(&g_pObjectManager->m_stMobData.Carry[pSwapItem->SourPos], pDestItem->m_pItem, sizeof(STRUCT_ITEM));
				else
					memset(&g_pObjectManager->m_stMobData.Carry[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pDestItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pDestItem);
			}
		}
		else if (pSwapItem->SourType == 2)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{				
				pSrcGrid->AddItem(pDestItem, pSwapItem->SourPos % 40 % 5, pSwapItem->SourPos % 40 / 5);
				if (pDestItem)
					memcpy(&g_pObjectManager->m_stItemCargo[pSwapItem->SourPos], pDestItem->m_pItem, sizeof(STRUCT_ITEM));
				else
					memset(&g_pObjectManager->m_stItemCargo[pSwapItem->SourPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pDestItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pDestItem);
			}
		}
	}
	if (pSrcItem)
	{
		if (!pSwapItem->DestType)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				pDestGrid->AddItem(pSrcItem, 0, 0);
				if (pSrcItem)
					memcpy(&g_pObjectManager->m_stMobData.Equip[pSwapItem->DestPos], pSrcItem->m_pItem, sizeof(STRUCT_ITEM));
				else
					memset(&g_pObjectManager->m_stMobData.Equip[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pSrcItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pSrcItem);
			}
		}
		else if (pSwapItem->DestType == 1)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				pDestGrid->AddItem(pSrcItem, pSwapItem->DestPos % 15 % 5, pSwapItem->DestPos % 15 / 5);
				if (pSrcItem)
					memcpy(&g_pObjectManager->m_stMobData.Carry[pSwapItem->DestPos], pSrcItem->m_pItem, sizeof(STRUCT_ITEM));
				else
					memset(&g_pObjectManager->m_stMobData.Carry[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pSrcItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pSrcItem);
			}
		}
		else if (pSwapItem->DestType == 2)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				pDestGrid->AddItem(pSrcItem, pSwapItem->DestPos % 40 % 5, pSwapItem->DestPos % 40 / 5);
				if (pSrcItem)
					memcpy(&g_pObjectManager->m_stItemCargo[pSwapItem->DestPos], pSrcItem->m_pItem, sizeof(STRUCT_ITEM));
				else
					memset(&g_pObjectManager->m_stItemCargo[pSwapItem->DestPos], 0, sizeof(STRUCT_ITEM));
			}
			else
			{
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pSrcItem)
					g_pCursor->m_pAttachedItem = 0;

				SAFE_DELETE(pSrcItem);
			}
		}
	}

	auto pMobData = &g_pObjectManager->m_stMobData;
	if (m_pMyHuman->m_sFamiliar != g_pObjectManager->m_stMobData.Equip[13].sIndex && m_pMyHuman->m_pFamiliar)
	{
		g_pObjectManager->DeleteObject(m_pMyHuman->m_pFamiliar);
		m_pMyHuman->m_pFamiliar = 0;
	}

	m_pMyHuman->m_sFamiliar = pMobData->Equip[13].sIndex;
	if (!pMobData->Guild)
		g_pObjectManager->m_usWarGuild = -1;
	if (pSwapItem->SourPos == 15 && pSwapItem->DestPos != 15 && m_pMyHuman)
		m_pMyHuman->m_sMountIndex = 0;

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(31);
		if (pSoundData)
			pSoundData->Play();
	}

	g_pCursor->DetachItem();
	SGridControl::m_pLastAttachedItem = 0;
	UpdateScoreUI(0);
	UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketShopList(MSG_STANDARD* pStd)
{
	auto pShopList = reinterpret_cast<MSG_ShopList*>(pStd);

	if (pShopList->ShopType == 1)
	{
		if (m_bEventCouponClick == 1)
		{
			m_bEventCouponClick = 0;
			m_bEventCouponOpen = 1;
		}

		m_pGridShop->Empty();

		for (int i = 0; i < 27; ++i)
		{
			auto pItemList = new STRUCT_ITEM;
			memcpy(pItemList, &pShopList->List[i], sizeof(STRUCT_ITEM));

			if (pShopList->List[i].sIndex <= 0)
			{
				delete pItemList;
				continue;
			}

			auto pItem = new SGridControlItem(0, pItemList, 0.0f, 0.0f);
			m_pGridShop->AddItem(pItem, i % 5, i / 5);

			int nAmount = BASE_GetItemAmount(pItemList);
			if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount > 0)
			{
				sprintf_s(pItem->m_GCText.strString, "%2d", nAmount);

				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}
		}

		auto pREQItem = new STRUCT_ITEM;
		if (pREQItem)
		{
			pREQItem->sIndex = 4998;
			
			auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);

			pItem->m_GCObj.nTextureIndex = 8;

			if (pItem)
				m_pGridShop->AddItem(pItem, 4, 7);
		}
		g_pObjectManager->m_nTax = pShopList->Tax;
		SetVisibleShop(1);
	}
	else if (pShopList->ShopType == 3)
	{
		m_pGridSkillMaster->Empty();

		switch ((pShopList->List[0].sIndex - 5000) / 24)
		{
		case 0:
			m_pSkillMSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pSkillMSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pSkillMSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pSkillMSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		for (int j = 0; j < 27; ++j)
		{
			if (pShopList->List[j].sIndex == 5027)
			{
				std::swap(pShopList->List[j], pShopList->List[j + 1]);
				std::swap(pShopList->List[j + 1], pShopList->List[j + 2]);
				break;
			}
		}

		for (int k = 0; k < 27; ++k)
		{
			auto dst = new STRUCT_ITEM;
			memcpy(dst, &pShopList->List[k], sizeof(STRUCT_ITEM));

			if (pShopList->List[k].sIndex <= 0)
			{
				delete dst;
				continue;
			}

			auto pItem = new SGridControlItem(0, dst, 0.0f, 0.0f);
			if (pItem)
				m_pGridSkillMaster->AddItem(pItem, k % 9 % 4, k / 9 + (k - k / 9) / 4);
		}

		if (!m_pSkillMPanel->IsVisible())
			SetVisibleSkillMaster();
	}

	return 1;
}

int TMFieldScene::OnPacketRMBShopList(MSG_RMBShopList* pMsg)
{
	if (pMsg->ShopType == 1)
	{
		if (m_bEventCouponClick == 1)
		{
			m_bEventCouponClick = 0;
			m_bEventCouponOpen = 1;
		}

		auto pGrid = m_pGridShop;
		pGrid->Empty();

		for (int i = 0; i < 39; ++i)
		{
			auto pItemList = new STRUCT_ITEM;			
			memcpy(pItemList, &pMsg->List[i], sizeof(STRUCT_ITEM));

			if (pMsg->List[i].sIndex <= 0)
			{
				delete pItemList;
				continue;
			}

			auto pItem = new SGridControlItem(0, pItemList, 0.0f, 0.0f);

			pGrid->AddItem(pItem, i % 5, i / 5);
			int nAmount = BASE_GetItemAmount(pItemList);
			if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount > 0)
			{
				sprintf(pItem->m_GCText.strString, "%2d", nAmount);
				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}
		}

		auto pREQItem = new STRUCT_ITEM;
		memset(pREQItem, 0, sizeof(STRUCT_ITEM));
		pREQItem->sIndex = 4998;

		auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);
		pItem->m_GCObj.nTextureIndex = 8;
		pGrid->AddItem(pItem, 4, 7);

		g_pObjectManager->m_nTax = pMsg->Tax;
		SetVisibleShop(1);
	}
	else if (pMsg->ShopType == 3)
	{		
		switch ((pMsg->List[0].sIndex - 5000) / 24)
		{
		case 0:
			m_pSkillMSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pSkillMSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pSkillMSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pSkillMSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillMSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillMSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		for (int j = 0; j < 27; ++j)
		{
			if (pMsg->List[j].sIndex == 5027)
			{
				std::swap(pMsg->List[j], pMsg->List[j + 1]);
				std::swap(pMsg->List[j + 1], pMsg->List[j + 2]);
				break;
			}
		}

		auto pGridSkillMaster = m_pGridSkillMaster;
		pGridSkillMaster->Empty();
		for (int k = 0; k < 27; ++k)
		{
			auto pNewItem = new STRUCT_ITEM;
			memcpy(pNewItem, &pMsg->List[k], sizeof(STRUCT_ITEM));

			if (pMsg->List[k].sIndex <= 0)
			{
				delete pNewItem;
				continue;
			}

			auto pItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
			pGridSkillMaster->AddItem(pItem, k % 9 % 4, k / 9 + (k - k / 9) / 4);
		}

		if (!m_pSkillMPanel->IsVisible())
			SetVisibleSkillMaster();
	}

	return 1;
}

int TMFieldScene::OnPacketBuy(MSG_STANDARD* pStd)
{
	// Just that
	return 1;
}

int TMFieldScene::OnPacketSell(MSG_STANDARD* pStd)
{
	auto pSell = reinterpret_cast<MSG_Sell*>(pStd);

	if (m_pGridHellStore->m_dwMerchantID == pSell->TargetID || 
		m_pGridShop->m_dwMerchantID == pSell->TargetID || !pSell->TargetID &&
		g_pObjectManager->m_stMobData.Class == 3 &&
		g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x1000)
	{
		SGridControlItem* pDestItem{};

		if (pSell->MyType == 0)
		{
			SGridControl* pGridDest[16]{};

			pGridDest[0] = m_pGridInv;
			pGridDest[1] = m_pGridHelm;
			pGridDest[2] = m_pGridCoat;
			pGridDest[3] = m_pGridPants;
			pGridDest[4] = m_pGridGloves;
			pGridDest[5] = m_pGridBoots;
			pGridDest[6] = m_pGridLeft;
			pGridDest[7] = m_pGridRight;
			pGridDest[8] = m_pGridRing;
			pGridDest[9] = m_pGridNecklace;
			pGridDest[10] = m_pGridOrb;
			pGridDest[11] = m_pGridCabuncle;
			pGridDest[12] = m_pGridGuild;
			pGridDest[13] = m_pGridEvent;
			pGridDest[14] = m_pGridDRing;
			pGridDest[15] = m_pGridMantua;
			pDestItem = pGridDest[pSell->MyPos]->PickupItem(0, 0);

			int nPrice = 0;

			if (pDestItem->m_pItem->sIndex > 0 && pDestItem->m_pItem->sIndex < MAX_ITEMLIST)
				nPrice = g_pItemList[pDestItem->m_pItem->sIndex].nPrice;

			nPrice = static_cast<int>((float)nPrice * 0.25f);

			if (nPrice >= 5001 && nPrice <= 10000)
			{
				nPrice = 2 * nPrice / 3;
			}
			else if (nPrice > 10000)
			{
				nPrice /= 2;
			}

			memset(&g_pObjectManager->m_stMobData.Equip[pSell->MyPos], 0, sizeof(STRUCT_ITEM));
			g_pObjectManager->m_stMobData.Coin += nPrice;
		}
		else if (pSell->MyType == 1)
		{
			pDestItem = m_pGridInvList[pSell->MyPos / 15]->PickupAtItem(
				pSell->MyPos % 15 % 5,
				pSell->MyPos % 15 / 5);

			int nPrice = 0;

			if (pDestItem->m_pItem->sIndex > 0 && pDestItem->m_pItem->sIndex < MAX_ITEMLIST)
				nPrice = g_pItemList[pDestItem->m_pItem->sIndex].nPrice;

			nPrice = static_cast<int>((float)nPrice * 0.25f);

			if (nPrice >= 5001 && nPrice <= 10000)
			{
				nPrice = 2 * nPrice / 3;
			}
			else if (nPrice > 10000)
			{
				nPrice /= 2;
			}

			memset(&g_pObjectManager->m_stMobData.Carry[pSell->MyPos], 0, sizeof(STRUCT_ITEM));
			g_pObjectManager->m_stMobData.Coin += nPrice;
		}

		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pDestItem)
			g_pCursor->m_pAttachedItem = nullptr;

		if (pDestItem)
			delete pDestItem;
		
		UpdateScoreUI(0);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(31);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}

		g_pCursor->DetachItem();
	}

	UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketDeposit(MSG_STANDARD* pStd)
{
	auto pDeposit = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

	g_pObjectManager->m_nCargoCoin += pDeposit->Parm;
	g_pObjectManager->m_stMobData.Coin -= pDeposit->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketWithdraw(MSG_STANDARD* pStd)
{
	auto pWithdraw = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

	g_pObjectManager->m_nCargoCoin -= pWithdraw->Parm;
	g_pObjectManager->m_stMobData.Coin += pWithdraw->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketCloseShop(MSG_STANDARD* pStd)
{
	SetVisibleShop(0);
	return 1;
}

int TMFieldScene::OnPacketCombineComplete(MSG_STANDARD* pStd)
{
	SetVisibleMixItem(0);
	SetVisibleMixItemTiini(0);
	SetVisibleMixPanel(0);
	return 1;
}

int TMFieldScene::OnPacketRESULTGAMBLE(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketItemPrice(MSG_STANDARDPARM2* pStd)
{
	g_pItemList[412].nPrice = pStd->Parm1;
	g_pItemList[413].nPrice = pStd->Parm2;
	return 1;
}

int TMFieldScene::OnPacketCapsuleInfo(MSG_CAPSULEINFO* pStd)
{
	bool bFind = false;
	for (int i = 0; i < 12; ++i)
	{
		if (g_pObjectManager->m_stCapsuleInfo[i].CIndex == pStd->CIndex)
		{
			g_pObjectManager->m_stCapsuleInfo[i] = *pStd;
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		for (int i = 0; i < 12; ++i)
		{
			if (!g_pObjectManager->m_stCapsuleInfo[i].CIndex)
			{
				g_pObjectManager->m_stCapsuleInfo[i] = *pStd;
				bFind = true;
				break;
			}
		}
	}

	if (!bFind)
		g_pObjectManager->m_stCapsuleInfo[0] = *pStd;

	SGridControl::m_bNeedUpdate = 1;
	return 1;
}

int TMFieldScene::OnPacketUndoSellItem(MSG_RepurchaseItems* pMsg)
{
	m_bIsUndoShoplist = 1;
	memset(m_stRepurcharse, 0, sizeof(m_stRepurcharse));

	auto pGrid = m_pGridShop;
	pGrid->Empty();

	for (int i = 0; i < 10; ++i)
	{
		auto pItemList = new STRUCT_ITEM;
		memcpy(pItemList, &pMsg->Repurcharse[i].stItem, sizeof(STRUCT_ITEM));

		if (pMsg->Repurcharse[i].stItem.sIndex <= 0)
		{
			delete pItemList;
			continue;
		}

		auto pItem = new SGridControlItem(0, pItemList, 0.0f, 0.0f);
		memcpy(&m_stRepurcharse[i], &pMsg->Repurcharse[i], sizeof(pMsg->Repurcharse[i]));

		pGrid->AddItem(pItem, i % 5, i / 5);
		int nAmount = BASE_GetItemAmount(pItemList);

		if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
			nAmount = 0;
		if (nAmount > 0)
		{
			sprintf(pItem->m_GCText.strString, "%2d", nAmount);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
	}

	auto pREQItem = new STRUCT_ITEM;
	memset(pREQItem, 0, sizeof(STRUCT_ITEM));
	pREQItem->sIndex = 4999;

	auto pItem = new SGridControlItem(0, pREQItem, 0.0f, 0.0f);
	pItem->m_GCObj.nTextureIndex = 9;

	pGrid->AddItem(pItem, 4, 7);


	SetVisibleShop(1);
	return 1;
}
