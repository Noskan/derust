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

/// @file TMFieldScene_Items.cpp
/// @brief Uso de itens no campo: poções (HP/MP/pet), alimentar montaria, tickets, fogos,
///        uso genérico de item, cápsulas, largar/pegar itens do chão e compra em loja.
///
/// Extraído de `TMFieldScene_Combat.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

void TMFieldScene::DropItem(unsigned int dwServerTime)
{
	auto pAttachItem = g_pCursor->m_pAttachedItem;
	if (!pAttachItem)
		return;

	static int dwLastDropTime = 0;
	if (dwServerTime - dwLastDropTime <= 1000)
		return;

	int nAX = pAttachItem->m_nCellIndexX;
	int nAY = pAttachItem->m_nCellIndexY;

	MSG_DropItem stDrop{};
	stDrop.Header.ID = m_pMyHuman->m_dwID;
	stDrop.Header.Type = MSG_DropItem_Opcode;
	stDrop.Rotate = 0;
	stDrop.SourType = pAttachItem->m_pGridControl->CheckType(pAttachItem->m_pGridControl->m_eItemType,
		pAttachItem->m_pGridControl->m_eGridType);

	if (stDrop.SourType)
	{
		if (stDrop.SourType == 1)
			stDrop.SourPos = nAX + 5 * nAY;
		else if (stDrop.SourType == 2)
			stDrop.SourPos = nAX + 5 * nAY;
		else
			stDrop.SourPos = pAttachItem->m_pGridControl->CheckPos(pAttachItem->m_pGridControl->m_eItemType);

		stDrop.GridX = (int)m_pMyHuman->m_vecPosition.x;
		stDrop.GridY = (int)m_pMyHuman->m_vecPosition.y;
		SendOneMessage((char*)&stDrop, sizeof(stDrop));
		dwLastDropTime = dwServerTime;
	}
}

int TMFieldScene::GetItem(TMItem* pItem)
{
	IVector2 vecGrid{};
	for (int i = 0; i < 4; ++i)
	{
		auto pGrid = m_pGridInvList[i];
		int nGridIndex = BASE_GetItemAbility(&pItem->m_stItem, 33);
		vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
		if (vecGrid.x > -1 && vecGrid.y > -1 || BASE_GetItemAbility(&pItem->m_stItem, 38) == 2)
		{
			m_pMyHuman->MoveGet(pItem);
			return 1;
		}
	}

	auto pListBox = m_pChatList;
	auto pBoxItem = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);

	pListBox->AddItem(pBoxItem);

	GetSoundAndPlay(33, 0, 0);
	m_dwGetItemTime = g_pTimerManager->GetServerTime();
	return 0;
}

void TMFieldScene::SendReqBuy(unsigned int dwControlID)
{
	MSG_ReqBuy stReqBuy{};
	stReqBuy.Header.ID = m_pMyHuman->m_dwID;
	stReqBuy.Header.Type = MSG_ReqBuy_Opcode;
	stReqBuy.TargetID = m_stAutoTrade.TargetID;
	stReqBuy.Pos = dwControlID - 653;
	stReqBuy.Price = m_stAutoTrade.TradeMoney[dwControlID - 653];
	stReqBuy.Tax = m_stAutoTrade.Tax;
	stReqBuy.item = m_stAutoTrade.Item[dwControlID - 653];

	SendOneMessage((char*)&stReqBuy, sizeof(stReqBuy));
}

void TMFieldScene::SetSanc()
{
	m_nMySanc = BASE_GetItemSanc(&g_pObjectManager->m_stMobData.Equip[4]);
}

int TMFieldScene::GetItemFromGround(unsigned int dwServerTime)
{
	if (g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill] == 31)
		return 0;

	auto pOverItem = m_pMouseOverItem;
	if (!pOverItem)
		return 0;
	if (dwServerTime <= m_dwGetItemTime + 1000)
		return 0;
	if (!pOverItem->m_bMouseOver)
		return 0;
	if (pOverItem->m_stItem.sIndex >= 1733 && pOverItem->m_stItem.sIndex <= 1736)
		return 1;
	if (pOverItem->m_stItem.sIndex >= 3145 && pOverItem->m_stItem.sIndex <= 3149)
		return 1;

	if (BASE_GetItemAbility(&pOverItem->m_stItem, 34) <= 0)
	{
		GetItem(pOverItem);
		return 1;
	}

	if (BASE_GetItemAbility(&pOverItem->m_stItem, 34) == 10 && pOverItem->m_stItem.sIndex >= 4100 && pOverItem->m_stItem.sIndex < 4200)
	{
		if (m_pGambleStore->m_bVisible == 1)
			return 1;

		if (pOverItem->m_stItem.sIndex == 4102)
		{
			SetVisibleGamble(1, 2);
			return 1;
		}
		if (pOverItem->m_stItem.sIndex == 4103)
		{
			SetVisibleGamble(1, 1);
			return 1;
		}
	}

	MSG_UpdateItem stUpdateItem{};
	stUpdateItem.Header.ID = m_pMyHuman->m_dwID;
	stUpdateItem.ItemID = pOverItem->m_dwID;
	stUpdateItem.Header.Type = MSG_UpdateItem_Opcode;
	stUpdateItem.State = 1;
	SendOneMessage((char*)&stUpdateItem, sizeof(stUpdateItem));

	m_dwGetItemTime = g_pTimerManager->GetServerTime();

	return 1;
}

char TMFieldScene::UseHPotion()
{
	SGridControl* pGridInv = nullptr;
	SGridControlItem* pItem = nullptr;

	bool bFind = false;
	int Page = 0;
	// TODO: do i need to say?
	for (int i = 0; i < 4; ++i)
	{
		pGridInv = m_pGridInvList[i];
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				pItem = pGridInv->GetItem(k, j);
				if (pItem && 
					(pItem->m_pItem->sIndex == 4097
						|| pItem->m_pItem->sIndex == 3431
						|| pItem->m_pItem->sIndex == 3322
						|| pItem->m_pItem->sIndex == 3477
						|| pItem->m_pItem->sIndex >= 400 && pItem->m_pItem->sIndex <= 404
						|| pItem->m_pItem->sIndex >= 428 && pItem->m_pItem->sIndex <= 431
						|| pItem->m_pItem->sIndex >= 680 && pItem->m_pItem->sIndex <= 685))
				{
					bFind = true;
					Page = 15 * i;
					break;
				}
			}
			if (bFind == 1)
				break;
		}
		if (bFind == 1)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) == 1)
	{
		if (m_pMyHuman->m_cCancel == 1)
			return 0;

		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

		int SourPosa = Page + SourPos;

		auto vec = m_pMyHuman->m_vecPosition;
		
		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPosa;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendOneMessage((char*)&stUseItem, sizeof(stUseItem));
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nAmount <= 1)
		{
			auto pPickedItem = m_pGridInvList[SourPosa / 15]->PickupItem(SourPosa % 15 % 5, SourPosa % 15 / 5);
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPosa], 0, sizeof(STRUCT_ITEM));

		GetSoundAndPlay(41, 0, 0);
	}

	UpdateScoreUI(16);
	return 1;
}

char TMFieldScene::UseMPotion()
{
	SGridControl* pGridInv = nullptr;
	SGridControlItem* pItem = nullptr;

	bool bFind = false;
	int Page = 0;
	// TODO: do i need to say?
	for (int i = 0; i < 4; ++i)
	{
		pGridInv = m_pGridInvList[i];
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				pItem = pGridInv->GetItem(k, j);
				if (pItem && (pItem->m_pItem->sIndex == 3323
					|| pItem->m_pItem->sIndex == 3472
					|| pItem->m_pItem->sIndex >= 405 && pItem->m_pItem->sIndex <= 409
					|| pItem->m_pItem->sIndex >= 432 && pItem->m_pItem->sIndex <= 435
					|| pItem->m_pItem->sIndex >= 686 && pItem->m_pItem->sIndex <= 691))
				{
					bFind = true;
					Page = 15 * i;
					break;
				}
			}
			if (bFind == 1)
				break;
		}
		if (bFind == 1)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) == 1)
	{
		if (m_pMyHuman->m_cCancel == 1)
			return 0;

		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

		int SourPosa = Page + SourPos;

		auto vec = m_pMyHuman->m_vecPosition;

		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPosa;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendOneMessage((char*)&stUseItem, sizeof(stUseItem));
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nAmount <= 1)
		{
			auto pPickedItem = m_pGridInvList[SourPosa / 15]->PickupItem(SourPosa % 15 % 5, SourPosa % 15 / 5);
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPosa], 0, sizeof(STRUCT_ITEM));

		GetSoundAndPlay(41, 0, 0);
	}

	UpdateScoreUI(16);
	return 1;
}

void TMFieldScene::UsePPotion()
{
	;
}

int TMFieldScene::IsFeedPotion(short sMountIndex, short sItemIndex)
{
	if ((sMountIndex == 2330 || sMountIndex == 2360) && sItemIndex == 2420)
		return 1;
	if ((sMountIndex == 2331 || sMountIndex == 2361) && (sItemIndex == 2421 || sItemIndex == 3368))
		return 1;
	if ((sMountIndex == 2332 || sMountIndex == 2362) && (sItemIndex == 2422 || sItemIndex == 3369))
		return 1;
	if ((sMountIndex == 2333 || sMountIndex == 2363) && (sItemIndex == 2423 || sItemIndex == 3370))
		return 1;
	if ((sMountIndex == 2334 || sMountIndex == 2364) && (sItemIndex == 2424 || sItemIndex == 3371))
		return 1;
	if ((sMountIndex == 2335 || sMountIndex == 2365) && (sItemIndex == 2425 || sItemIndex == 3372))
		return 1;
	if ((sMountIndex >= 2336 && sMountIndex <= 2345 || sMountIndex >= 2366 && sMountIndex <= 2375 || sMountIndex >= 2960 && sMountIndex < 3000) && 
		(sItemIndex == 2426 || sItemIndex == 3373))
	{
		return 1;
	}
	if ((sMountIndex == 2346 || sMountIndex == 2376) && (sItemIndex == 2436 || sItemIndex == 3383))
		return 1;
	if ((sMountIndex == 2347 || sMountIndex == 2377) && (sItemIndex == 2437 || sItemIndex == 3384))
		return 1;
	if ((sMountIndex == 2349 || sMountIndex == 2379) && (sItemIndex == 2427 || sItemIndex == 3374))
		return 1;
	if ((sMountIndex == 2350 || sMountIndex == 2380) && (sItemIndex == 2428 || sItemIndex == 3375))
		return 1;
	if ((sMountIndex >= 2351 && sMountIndex <= 2353 || sMountIndex >= 2381 && sMountIndex <= 2383) && (sItemIndex == 2429 || sItemIndex == 3376))
		return 1;
	if ((sMountIndex >= 2354 && sMountIndex <= 2356 || sMountIndex >= 2384 && sMountIndex <= 2386) && (sItemIndex == 2430 || sItemIndex == 3377))
		return 1;
	if ((sMountIndex == 2357 || sMountIndex == 2387) && (sItemIndex == 2426 || sItemIndex == 3373))
		return 1;
	if ((sMountIndex == 2358 || sMountIndex == 2388) && (sItemIndex == 2429 || sItemIndex == 3376))
		return 1;
	if ((sMountIndex == 2378 || sMountIndex == 2348) && (sItemIndex == 3465 || sItemIndex == 2438))
		return 1;
	if ((sMountIndex == 2389 || sMountIndex == 2359) && (sItemIndex == 3466 || sItemIndex == 2439))
		return 1;
	return 0;
}

char TMFieldScene::FeedMount()
{
	SGridControl* pGridInv = nullptr;
	SGridControlItem* pItem = nullptr;

	bool bFind = false;
	int Page = 0;
	// TODO: do i need to say?
	for (int i = 0; i < 4; ++i)
	{
		pGridInv = m_pGridInvList[i];
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				pItem = pGridInv->GetItem(k, j);
				if (pItem && IsFeedPotion(g_pObjectManager->m_stMobData.Equip[14].sIndex,
					pItem->m_pItem->sIndex))
				{
					bFind = true;
					Page = 15 * i;
					break;
				}
			}
			if (bFind == 1)
				break;
		}
		if (bFind == 1)
			break;
	}

	if (bFind != 1 || !pItem)
		return 0;

	if (BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE) == 15)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
			return 0;

		pGridInv->CheckType(pItem->m_pGridControl->m_eItemType,
			pItem->m_pGridControl->m_eGridType);

		int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

		int SourPosa = Page + SourPos;

		auto vec = m_pMyHuman->m_vecPosition;

		MSG_UseItem stUseItem{};
		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPosa;
		stUseItem.ItemID = 0;
		stUseItem.GridX = (int)vec.x;
		stUseItem.GridY = (int)vec.y;
		SendOneMessage((char*)&stUseItem, sizeof(stUseItem));
		m_dwUseItemTime = dwServerTime;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);
		if (nAmount <= 1)
		{
			auto pPickedItem = m_pGridInvList[SourPosa / 15]->PickupItem(SourPosa % 15 % 5, SourPosa % 15 / 5);
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = 0;

			SAFE_DELETE(pPickedItem);
			g_pCursor->DetachItem();
		}
		else
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
			sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		if (nAmount <= 1)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPosa], 0, sizeof(STRUCT_ITEM));
	}

	UpdateScoreUI(0);
	return 1;
}

void TMFieldScene::UseTicket(int nCellX, int nCellY)
{
	// O ticket é usado da página do inventário aberta (SGridControl, evento 38), como os outros itens.
	// A versão anterior procurava a grade única G_INV_GRID (65554), que não existe mais: o inventário
	// é paginado em m_pGridInvList.
	auto pGrid = m_pGridInv;
	if (!pGrid)
		return;

	auto pItem = pGrid->GetItem(nCellX, nCellY);
	if (!pItem)
		return;

	if (BASE_GetItemAbility(pItem->m_pItem, 38) != 14)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return;

	int SourPos = pGrid->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
	{
		// Mesma conta de SGridControl para os itens de uso (ui/SGrid_Input.cpp): 15 posições por página.
		int Page = 15 * (pGrid->m_dwControlID - 67072);
		if (Page < 0 || Page > 45)
			Page = 0;

		SourPos = Page + pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;
	}

	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)m_pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)m_pMyHuman->m_vecPosition.y;
	SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

	m_dwUseItemTime = dwServerTime;
	g_pEventTranslator->m_bRBtn = 1;

	int nAmount = BASE_GetItemAmount(pItem->m_pItem);
	if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
		nAmount = 0;

	if (nAmount > 1)
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
	}
	else
	{
		auto pPickedItem = pGrid->PickupItem(nCellX, nCellY);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;
		
		SAFE_DELETE(pPickedItem);
	}

	GetSoundAndPlay(47, 0, 0);

	UpdateScoreUI(0);
	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
}

void TMFieldScene::UseFireWork()
{
	if (!m_pFireWorkPanel)
		return;

	if (m_nFireWorkCellX < 0 || m_nFireWorkCellY < 0)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return;

	if (!m_pGridInv)
		return;

	auto pItem = m_pGridInv->GetItem(m_nFireWorkCellX, m_nFireWorkCellY);
	if (!pItem)
		return;

	int nItemSIndex = pItem->m_pItem->sIndex;
	auto pMyHuman = m_pMyHuman;

	int SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

	int nType = BASE_GetItemAbility(pItem->m_pItem, 38);

	MSG_UseItem2 stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem2_Opcode;
	stUseItem.SourType = 1;
	stUseItem.SourPos = SourPos;

	if (nType == 15)
	{
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
	}

	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)pMyHuman->m_vecPosition.y;
	for (int i = 0; i < 100; ++i)
	{
		if (m_pFireWorkButton[i]->m_bSelected)
			BASE_SetBit(stUseItem.Parm, i);
	}

	SendOneMessage((char*)&stUseItem, sizeof(stUseItem));
	m_nFireWorkCellX = -1;
	m_nFireWorkCellY = -1;
	m_dwUseItemTime = dwServerTime;

	int nAmount = BASE_GetItemAmount(pItem->m_pItem);
	if (nAmount > 1)
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
	}
	else
	{
		auto pPickedItem = m_pGridInv->PickupItem(pItem->m_nCellIndexX, pItem->m_nCellIndexY);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(pPickedItem);
	}

	int nSoundIndex = 41;
	if (nType >= 11 && nType <= 13)
		nSoundIndex = 54;

	if (nType != 19)
		GetSoundAndPlay(nSoundIndex, 0, 0);

	UpdateScoreUI(0);

	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
}

void TMFieldScene::UseItem(SGridControlItem* pItem, int nType, int nItemSIndex, int nCellX, int nCellY)
{
	if (pItem == nullptr || m_pGridInv == nullptr)
		return;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (!m_dwUseItemTime || (dwServerTime - m_dwUseItemTime) >= 200)
	{
		short SourPos = m_pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);

		if (SourPos == -1)
			SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

		int SourPage = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);

		MSG_UseItem stUseItem{};

		stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
		stUseItem.Header.Type = MSG_UseItem_Opcode;
		stUseItem.SourType = 1;
		stUseItem.SourPos = SourPage + SourPos;

		if (nType == 15)
		{
			stUseItem.DestType = 0;
			stUseItem.DestPos = 14;
		}

		stUseItem.ItemID = 0;
		stUseItem.GridX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
		stUseItem.GridY = static_cast<int>(m_pMyHuman->m_vecPosition.y);

		SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

		m_dwUseItemTime = dwServerTime;
		g_pEventTranslator->m_bRBtn = 1;

		int delAmountCnt = 1;

		if (pItem->m_pItem->sIndex == 4049)
			delAmountCnt = 10;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);

		if (nItemSIndex >= 2330 && nItemSIndex < 2390)
			nAmount = 0;

		if (nAmount > delAmountCnt)
		{
			BASE_SetItemAmount(pItem->m_pItem, nAmount - delAmountCnt);
			sprintf_s(pItem->m_GCText.strString, "%2d", nAmount - delAmountCnt);
			pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		}
		else
		{
			SGridControlItem* pPickedItem = m_pGridInvList[pItem->m_pGridControl->m_dwControlID - 67072]->PickupItem(nCellX, nCellY);

			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
				delete pPickedItem;
		}

		int nSoundIndex = 41;

		if (nType >= 11 && nType <= 13)
			nSoundIndex = 54;

		if (nType != 19 && g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}

		UpdateScoreUI(0);

		if (nAmount <= delAmountCnt)
			memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
	}
}

void TMFieldScene::SendCapsuleItem()
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	auto pEditID = (SEditableText*)m_pControlContainer->FindControl(65889);

	int len = strlen(pEditID->GetText());
	if (len >= 4)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[15], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return;
	}
	if (len > 12)
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[16], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return;
	}
	if (!BASE_CheckValidString(pEditID->GetText()))
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return;
	}

	char* szName = BASE_TransCurse(pEditID->GetText());

	for (int i = 0; i < len - 1; ++i)
	{
		if (szName[i] == -95 && szName[i + 1] == -95)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[17], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return;
		}
	}

	int nCellX = m_stCapsuleItem.GridX;
	int nCellY = m_stCapsuleItem.GridY;
	auto vec = m_pMyHuman->m_vecPosition;

	m_stCapsuleItem.GridX = (int)vec.x;
	m_stCapsuleItem.GridY = (int)vec.y;

	sprintf(m_stCapsuleItem.NewMobname, pEditID->GetText());
	SendOneMessage((char*)&m_stCapsuleItem, sizeof(m_stCapsuleItem));

	m_dwUseItemTime = dwServerTime;
	g_pEventTranslator->m_bRBtn = 1;
	
	auto pPickedItem = m_pGridInv->PickupItem(nCellX, nCellY);
	if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
		g_pCursor->m_pAttachedItem = nullptr;

	SAFE_DELETE(pPickedItem);

	UpdateScoreUI(0);
	memset(&g_pObjectManager->m_stMobData.Carry[m_stCapsuleItem.SourPos], 0, sizeof(STRUCT_ITEM));
}
