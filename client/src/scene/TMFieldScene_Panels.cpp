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

/// @file TMFieldScene_Panels.cpp
/// @brief Abertura/fechamento e estado dos painéis do campo: inventário, ficha, loja, cargo,
///        troca, combinações (Ailyn/Tiny/HellGate/Gamble), portais, minimapa, skills, venda
///        automática, sacola, voo (AirMove), mix/missão e diálogos de entrada de texto.
///
/// Extraído de `TMFieldScene_UI.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

void TMFieldScene::SetVisibleInventory()
{
	SGridControl::m_sLastMouseOverIndex = -1;

	auto pCargoPanel = m_pCargoPanel;
	auto pPanel = m_pInvenPanel;
	auto pCPanel = m_pCPanel;
	auto pSkillPanel = m_pSkillPanel;
	auto pSkillMPanel = m_pSkillMPanel;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pItemMixPanel = m_pItemMixPanel;
	auto pItemMixPanel4 = m_pItemMixPanel4;
	auto pHellgateStore = m_pHellgateStore;
	auto pGambleStore = m_pGambleStore;

	if (m_pGambleStore->IsVisible() == 1)
		pPanel->m_bVisible = 0;

	if (pGambleStore->IsVisible() == 1)
		pCPanel->m_bVisible = 0;

	int bInv = pPanel->m_bVisible == 0;

	if (pATradePanel && pATradePanel->IsVisible() == 1)
		SetVisibleAutoTrade(0, 0);

	if (pGambleStore && pGambleStore->IsVisible() == 1)
		SetVisibleGamble(0, 0);

	pPanel->SetVisible(bInv == 0);

	if (!bInv)
	{
		ClearCombine();
		ClearCombine4();
		ClearMixPannel();

		m_ItemMixClass.m_pMixPanel->SetVisible(0);

		if (m_ItemMixClass.m_pMixPanel)
			m_ItemMixClass.m_pMixPanel->SetVisible(0);

		ClearMissionPannel();

		m_MissionClass.m_pMissionPanel->SetVisible(0);

		if (m_MissionClass.m_pMissionPanel)
			m_MissionClass.m_pMissionPanel->SetVisible(0);

		SetVisibleShop(0);

		pCargoPanel->SetVisible(0);
		pItemMixPanel->SetVisible(0);
		pItemMixPanel4->SetVisible(0);
		pHellgateStore->SetVisible(0);

		SetGridState();

		m_pInputGoldPanel->SetVisible(0);

		if (pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		g_pCursor->DetachItem();
	}

	pPanel->SetVisible(bInv);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleCharInfo()
{
	SGridControl::m_sLastMouseOverIndex = -1;
	
	auto pCPanel = m_pCPanel;
	auto pCargoPanel = m_pCargoPanel;
	auto pCargoPanel1 = m_pCargoPanel1;
	auto pSkillMPanel = m_pSkillMPanel;
	auto pSkillPanel = m_pSkillPanel;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pHellgateStore = m_pHellgateStore;
	auto pGambleStore = m_pGambleStore;

	bool bInv = m_pInvenPanel->m_bVisible;
	bool bVisible = pCPanel->m_bVisible == 0;

	pCPanel->SetVisible(bVisible);
	pCPanel->SetVisible(bVisible);

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(51);
		if (pSoundData)
		{
			pSoundData->Play();
		}
	}
}

void TMFieldScene::SetVisibleShop(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;

	if (bShow)
	{
		g_pCursor->DetachItem();

		m_pCPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);

		if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_SELL;

		SetEquipGridState(0);

		m_pSkillPanel->SetVisible(0);
	}
	else
	{
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;

		SetEquipGridState(1);

		g_pObjectManager->m_RMBShopOpen = 0;
		m_bEventCouponOpen = 0;
	}

	m_pShopPanel->SetVisible(bShow);

	if (bShow)
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f, RenderDevice::m_fHeightRatio * 35.0f);

	m_pInvenPanel->SetVisible(bShow);

	UpdateScoreUI(0);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleCargo(int bShow)
{
	if (m_pGambleStore->IsVisible() == 1)
		bShow = 0;

	SGridControl::m_sLastMouseOverIndex = -1;
	
	m_pCargoPanel->SetVisible(bShow);
	m_pCargoPanel1->SetVisible(bShow);

	if (bShow)
	{
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f, RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f, RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 287.0f, RenderDevice::m_fHeightRatio * 35.0f);
	}

	m_pInvenPanel->SetVisible(bShow);
	m_pSkillPanel->SetVisible(bShow == 0);
	m_pShopPanel->SetVisible(bShow == 0);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleCargo1(int bShow)
{
	if (m_pGambleStore1->IsVisible() == 1)
		bShow = 0;

	SGridControl::m_sLastMouseOverIndex = -1;
	auto pTradePanel = m_pTradePanel;
	auto pATradePanel = m_pAutoTrade;
	auto pShopPanel = m_pShopPanel;
	auto pPanel = m_pInvenPanel;
	auto pCPanel = m_pCPanel;
	auto pHellgateStore = m_pHellgateStore;
	m_pCargoPanel1->SetVisible(bShow);

	if (bShow)
	{
		pPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
		m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
	}

	pPanel->SetVisible(bShow);
	m_pSkillPanel->SetVisible(bShow == 0);
	pShopPanel->SetVisible(bShow == 0);

	GetSoundAndPlay(51, 0, 0);
}

void TMFieldScene::SetVisibleTrade(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;

	auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));
	auto pBtnInv = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));
	auto pMyGold = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_MYGOLD));
	auto pOPGold = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_OPGOLD));

	pOPGold->m_cComma = 1;
	pMyGold->m_cComma = 1;

	bool bSendQuit = false;

	if (m_pTradePanel->IsVisible() == 1 && !bShow)
		bSendQuit = true;

	if (bShow == 1)
	{
		g_pCursor->DetachItem();
		m_pGambleStore->SetVisible(0);
		
		if (m_pTradePanel->IsVisible() == 1 && m_pAutoTrade)
			SetVisibleAutoTrade(0, 0);

		m_pSystemPanel->SetVisible(0);
		m_pCPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);
		m_pTradePanel->SetVisible(bShow);
		m_pInvenPanel->SetVisible(bShow);
		m_pSkillPanel->SetVisible(0);
		m_pSkillMPanel->SetVisible(0);
		m_pShopPanel->SetVisible(0);
		m_pHellgateStore->SetVisible(0);
		pBtnInv->SetSelected(0);
		pBtnChar->SetSelected(0);
		if (m_pSkillPanel->IsVisible() == 1)
			m_pSkillPanel->SetVisible(0);
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			SGridControl* pGridInv = m_pGridInvList[i];

			for (int nY = 0; nY < 3; ++nY)
			{
				for (int nX = 0; nX < 5; ++nX)
				{
					SGridControlItem* pItem = pGridInv->GetItem(nX, nY);

					if (pItem && pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
		pMyGold->SetText((char*)"         0", 0);
		pOPGold->SetText((char*)"         0", 0);
		m_pTradePanel->SetVisible(bShow);
		m_pInvenPanel->SetVisible(0);
		m_pInputGoldPanel->SetVisible(0);
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}

	if (!bShow && g_pObjectManager->m_stTrade.OpponentID > 0)
	{
		g_pObjectManager->m_stTrade.OpponentID = 0;
		g_pObjectManager->m_stTrade.MyCheck = 0;
		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

		if (bSendQuit)
		{
			MSG_STANDARD stStandard{};

			stStandard.Type = MSG_CloseTrade_Opcode;
			stStandard.ID = m_pMyHuman->m_dwID;
			SendOneMessage((char*)&stStandard, sizeof(stStandard));
		}
	}

	if (bShow == 0)
	{
		memset(&g_pObjectManager->m_stTrade, 0, sizeof(g_pObjectManager->m_stTrade));

		for (int j = 0; j < 15; ++j)
			g_pObjectManager->m_stTrade.CarryPos[j] = -1;
	}

	if (bShow == 1)
	{
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV;
		SetEquipGridState(0);
		g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;
		g_pObjectManager->m_stTrade.Header.ID = m_pMyHuman->m_dwID;
	}
	else
	{
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		SetEquipGridState(1);
		memset(&g_pObjectManager->m_stTrade, 0, sizeof(g_pObjectManager->m_stTrade));

		for (int k = 0; k < 15; ++k)
			g_pObjectManager->m_stTrade.CarryPos[k] = -1;

		SGridControl* pGridOp[15]{};
		SGridControl* pGridMy[15]{};

		for (int l = 0; l < 15; ++l)
		{
			SGridControlItem* pPickedItem{};

			pGridOp[l] = static_cast<SGridControl*>(m_pControlContainer->FindControl(l + TMG_TRADE_OP1));

			if (pGridOp[l])
				pPickedItem = pGridOp[l]->PickupItem(0, 0);

			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
			{
				delete pPickedItem;

				pPickedItem = nullptr;
			}

			pGridMy[l] = static_cast<SGridControl*>(m_pControlContainer->FindControl(l + TMG_TRADE_MY1));

			if (pGridMy[l])
				pPickedItem = pGridMy[l]->PickupItem(0, 0);

			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
				g_pCursor->m_pAttachedItem = nullptr;

			if (pPickedItem)
				delete pPickedItem;
		}

		auto pOpCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_TRADE_OPCHECK));
		auto pMyCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_TRADE_MYCHECK));
		
		pOpCheckButton->m_bSelected = 0;
		pMyCheckButton->m_bSelected = 0;

		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

		if (m_pInvenPanel->IsVisible() == 1)
			SetVisibleInventory();
	}
}

void TMFieldScene::ClearCombine()
{
	for (int i = 0; i < 3; ++i)
	{
		auto pGridInv = m_pGridInvList[i];
		for (int nY = 0; nY < 3; ++nY)
		{
			for (int nX = 0; nX < 5; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	memset(&g_pObjectManager->m_stCombineItem, 0, sizeof(g_pObjectManager->m_stCombineItem));
	g_pObjectManager->m_stCombineItem.Header.ID = m_pMyHuman->m_dwID;
	g_pObjectManager->m_stCombineItem.Header.Type = MSG_CombineItem_Opcode;
	for (int i = 0; i < 8; ++i)
		g_pObjectManager->m_stCombineItem.CarryPos[i] = -1;

	SGridControl* pGridMix[8]{};
	for (int i = 0; i < 8; ++i)
	{
		SGridControlItem* pPickedItem = nullptr;
		pGridMix[i] = (SGridControl*)m_pControlContainer->FindControl(i + 65861);
		if (pGridMix[i])
			pPickedItem = pGridMix[i]->PickupItem(0, 0);

		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;
		
		SAFE_DELETE(pPickedItem);
	}

	m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	SetEquipGridState(1);
}

void TMFieldScene::ClearCombine4()
{
	for (int i = 0; i < 3; ++i)
	{
		auto pGridInv = m_pGridInvList[i];
		for (int nY = 0; nY < 3; ++nY)
		{
			for (int nX = 0; nX < 5; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	memset(&g_pObjectManager->m_stCombineItem4, 0, sizeof(g_pObjectManager->m_stCombineItem4));
	g_pObjectManager->m_stCombineItem4.Header.ID = m_pMyHuman->m_dwID;
	g_pObjectManager->m_stCombineItem4.Header.Type = MSG_CombineItemTiny_Opcode;
	for (int i = 0; i < 8; ++i)
		g_pObjectManager->m_stCombineItem4.CarryPos[i] = -1;

	SGridControl* pGridMix[8]{};
	for (int i = 0; i < 3; ++i)
	{
		SGridControlItem* pPickedItem = nullptr;
		if (m_pGridItemMix4[i])
			pPickedItem = m_pGridItemMix4[i]->PickupItem(0, 0);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;

		SAFE_DELETE(pPickedItem);
	}

	m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	SetEquipGridState(1);
}

void TMFieldScene::DoCombine()
{
	if (g_nCombineMode == 1)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (!g_pObjectManager->m_stCombineItem.Item[i].sIndex)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[274], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return;
			}
		}
	}

	bool bFind = false;
	for (int i = 0; i < 8; ++i)
	{
		if (g_pObjectManager->m_stCombineItem.CarryPos[i] != -1)
		{
			bFind = true;
			break;
		}
	}

	if (bFind)
		SendOneMessage((char*)&g_pObjectManager->m_stCombineItem, sizeof(g_pObjectManager->m_stCombineItem));
}

void TMFieldScene::DoCombine4()
{
	if (g_pObjectManager->m_stCombineItem4.Item[0].sIndex && g_pObjectManager->m_stCombineItem4.Item[1].sIndex)
	{
		bool bFind = false;
		for (int i = 0; i < 8; ++i)
		{
			if (g_pObjectManager->m_stCombineItem4.CarryPos[i] != -1)
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
			SendOneMessage((char*)&g_pObjectManager->m_stCombineItem4, sizeof(g_pObjectManager->m_stCombineItem4));
	}
	else
	{
		m_pMessagePanel->SetMessage(g_pMessageStringTable[274], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
}

void TMFieldScene::SetVisibleMixItem(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;

	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}

		m_pItemMixPanel->SetVisible(1);
		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		TMFieldScene::SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_pItemMixPanel->SetVisible(0);
		ClearCombine();
	}
}

void TMFieldScene::SetVisibleMixItemTiini(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;

	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}
		m_pItemMixPanel4->SetVisible(1);
		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV6;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV6;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV6;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV6;
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_pItemMixPanel4->SetVisible(0);
		ClearCombine4();
	}
}

void TMFieldScene::SetVisibleHellGateStore(int bShow)
{
	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);
	if (bShow == 1)
	{
		if (m_pInvenPanel && !m_pInvenPanel->m_bVisible)
			SetVisibleInventory();
		if (m_pHellStoreDesc)
			LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 0, 20);

		m_pHellgateStore->SetVisible(1);
		g_pDevice->m_nWidthShift = 0;
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_SELL;
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_pHellgateStore->SetVisible(0);
		m_dwHellStoreID = 0;
		m_nHellStoreValue = 0;
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		SetEquipGridState(1);
	}
	if (!bShow)
	{
		for (int i = 0; i < 4; ++i)
		{
			auto pBtnHellStore = (SButton*)m_pControlContainer->FindControl(i + 6193);
			if (pBtnHellStore)
				pBtnHellStore->SetSelected(0);
		}
	}
}

void TMFieldScene::SetVisibleGamble(int bShow, char cType)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (bShow || m_pGambleStore->m_bVisible != 1 || !m_pReelPanel2->m_dwStopTime && m_pReelPanel2->m_bRoling != 1)
	{
		if (m_pInputGoldPanel->IsVisible() == 1)
			SetInVisibleInputCoin();
		if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
			SetVisibleSkill();
		if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
			SetVisibleCharInfo();
		if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
			SetVisibleCargo(0);
		if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
			SetVisibleCargo(0);
		if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
			SetVisibleAutoTrade(0, 0);
		if (bShow == 1 && m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();
		if (bShow == 1 && m_pShopPanel && m_pShopPanel->m_bVisible == 1)
			SetVisibleShop(0);
		auto pGambleStore = m_pGambleStore;
		if (bShow == 1)
		{
			if (pGambleStore)
			{
				pGambleStore->SetVisible(1);
				if (cType == 1)
				{
					m_pReelPanel->SetVisible(1);
					m_pReelPanel2->SetVisible(0);
				}
				else if (cType == 2)
				{
					m_pReelPanel->SetVisible(0);
					m_pReelPanel2->SetVisible(1);
				}
			}

			m_pGridHelm->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
			SetEquipGridState(0);
		}
		else
		{
			if (pGambleStore)
				pGambleStore->SetVisible(0);
			g_pDevice->m_nWidthShift = 0;
		}
	}
}

void TMFieldScene::SetVisiblePotal(int bShow, int nPos)
{
	if (m_bAirmove_ShowUI == 1)
		AirMove_ShowUI(0);

	auto pPotalPanel = m_pPotalPanel;
	if (!pPotalPanel)
		return;

	char strTmp1[32]{};
	strcpy(strTmp1, g_pMessageStringTable[382]);
	char strTmp2[32]{};
	strcpy(strTmp2, g_pMessageStringTable[383]);
	char strTmp3[32]{};
	strcpy(strTmp3, g_pMessageStringTable[384]);
	if (m_pPotalText1)
		m_pPotalText1->SetText(strTmp1, 0);
	if (m_pPotalText2)
		m_pPotalText2->SetText(strTmp2, 0);
	if (m_pPotalText3)
		m_pPotalText3->SetText(strTmp3, 0);

	pPotalPanel->SetVisible(bShow);
	char szPotalPos[64]{};
	sprintf(szPotalPos, "UI\\PotalPos.txt");
	if (bShow == 1)
		LoadMsgText2(m_pPotalList, szPotalPos, 10 * nPos, 10 * (nPos + 1) - 1);
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(0);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(0);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);
	if (bShow == 1 && m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
		SetVisibleInventory();
	if (bShow == 1 && m_pShopPanel && m_pShopPanel->m_bVisible == 1)
		SetVisibleShop(0);
	if (bShow)
	{
		m_pSkillPanel->m_bVisible = 0;

		char szStr[128]{};
		sprintf(szStr, "%s", g_pMessageStringTable[310]);
		TMScene::LoadMsgText2(m_pPotalList, szPotalPos, 10 * nPos, 10 * (nPos + 1) - 1);
		switch (nPos)
		{
		case 0:
			sprintf(szStr, "%s", g_pMessageStringTable[310]);
			break;
		case 1:
			sprintf(szStr, "%s", g_pMessageStringTable[311]);
			break;
		case 2:
			sprintf(szStr, "%s", g_pMessageStringTable[312]);
			break;
		case 3:
			sprintf(szStr, "%s", g_pMessageStringTable[313]);
			break;
		case 4:
			sprintf(szStr, "%s", g_pMessageStringTable[314]);
			break;
		case 5:
			sprintf(szStr, "%s", g_pMessageStringTable[315]);
			break;
		}

		m_pPotalText->SetText(szStr, 0);
		m_pPotalText->SetTextColor(0xFFFFFFFF);
	}
	else
	{
		memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
	}
}

void TMFieldScene::SetVisibleMiniMap()
{
	if (m_pMiniMapPanel == nullptr)
		return;

	SGridControl::m_sLastMouseOverIndex = -1;

	int bVisible = m_pMiniMapPanel->m_bVisible;

	if (bVisible)
	{
		for (int j = 0; j < 256; ++j)
		{
			m_pInMiniMapPosPanel[j]->SetVisible(0);
			m_pInMiniMapPosText[j]->SetVisible(0);
		}
	}
	else
	{
		for (int i = 0; i < 256; ++i)
		{
			m_pInMiniMapPosPanel[i]->SetVisible(0);
			m_pInMiniMapPosText[i]->SetVisible(0);
		}
	}

	if (bVisible == 1)
	{
		auto pBGPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12841u));

		if (pBGPanel)
			pBGPanel->SetVisible(0);

		if (m_pMiniMapServerPanel)
			m_pMiniMapServerPanel->SetVisible(0);

		if (m_pMiniMapZoomOut)
			m_pMiniMapZoomOut->SetVisible(1);

		if (m_pMiniMapZoomIn)
			m_pMiniMapZoomIn->SetVisible(0);

		if (TMGround::m_fMiniMapScale >= 1.0f)
		{
			if (m_pMiniMapPanel)
			{
				m_pMiniMapPanel->SetPos((float)g_pDevice->m_dwScreenWidth - m_pMiniMapPanel->m_nWidth, 0);
				m_pMiniMapPanel->SetVisible(0);
			}
		}
		else
		{
			TMGround::m_fMiniMapScale = 1.5f;

			float fHeight = BASE_ScreenResize(400.0f);

			if (m_pMiniMapPanel)
			{
				m_pMiniMapPanel->SetSize(fHeight, fHeight);

				m_pMiniMapPanel->SetPos(
					((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pMiniMapPanel->m_nWidth * 0.5f),
					(((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pMiniMapPanel->m_nHeight * 0.5f)) - 15.0f);
			}

			if (m_pMiniMapDir)
				m_pMiniMapDir->SetPos(fHeight * 0.5f, fHeight * 0.5f);

			if (m_pPositionText)
				m_pPositionText->SetPos(10.0f, 0);

			if (m_pMiniMapZoomIn)
				m_pMiniMapZoomIn->SetPos(fHeight - 134.0f, fHeight + 5.0f);

			if (m_pMiniMapZoomOut)
				m_pMiniMapZoomOut->SetPos(fHeight - 40.0f, 0);

			if (m_pMiniMapServerPanel)
				m_pMiniMapServerPanel->SetPos(fHeight - 274.0f, fHeight);

			if (m_pMiniMapZoomIn)
				m_pMiniMapZoomIn->m_bSelected = 1;

			if (m_pMiniMapZoomOut)
				m_pMiniMapZoomOut->m_bSelected = 0;
		}
	}
	else
	{
		TMGround::m_fMiniMapScale = 0.60000002f;

		auto pBGPanel = static_cast<SPanel*>(m_pControlContainer->FindControl(12841u));

		if (pBGPanel)
			pBGPanel->SetVisible(1);

		if (m_pMiniMapZoomOut)
			m_pMiniMapZoomOut->SetVisible(0);

		if (m_pMiniMapZoomIn)
		{
			m_pMiniMapZoomIn->SetVisible(1);

			m_pMiniMapZoomIn->SetPos(BASE_ScreenResize(118.0f), -BASE_ScreenResize(18.0f));
		}

		if (m_pMiniMapPanel)
		{
			m_pMiniMapPanel->SetSize(BASE_ScreenResize(137.0f), BASE_ScreenResize(137.0f));

			m_pMiniMapPanel->SetPos(((float)g_pDevice->m_dwScreenWidth - m_pMiniMapPanel->m_nWidth) - 2.0f, 33.0f);
		}

		m_pMiniMapServerPanel->SetVisible(1);

		if (m_pMiniMapServerPanel)
		{
			m_pMiniMapServerPanel->SetPos(0, BASE_ScreenResize(130.0f));
		}

		if (m_pMiniMapPanel)
			m_pMiniMapPanel->SetVisible(bVisible == 0);

		if (m_pMiniMapDir)
		{
			m_pMiniMapDir->SetPos(BASE_ScreenResize(68.0f), BASE_ScreenResize(68.0f));
		}

		if (m_pPositionText)
		{
			m_pPositionText->SetPos(BASE_ScreenResize(13.0f), BASE_ScreenResize(120.0f));
		}

		if (m_pMiniMapZoomIn && m_pMiniMapZoomOut)
		{
			m_pMiniMapZoomOut->SetPos(BASE_ScreenResize(119.0f), BASE_ScreenResize(142.0f));
			m_pMiniMapZoomOut->m_bSelected = 1;
		}
	}

	bVisible = m_pMiniMapPanel->m_bVisible;

	if (bVisible == 1 && m_pGround)
		m_pGround->RestoreDeviceObjects();

	auto pBtnMinimap = static_cast<SButton*>(m_pControlContainer->FindControl(296u));

	if (pBtnMinimap)
		pBtnMinimap->SetSelected(m_pMiniMapPanel->m_bVisible);

	m_pPositionText->SetVisible(bVisible);

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleParty()
{
	if (m_pPartyPanel)
		m_pPartyPanel->SetVisible(m_pPartyPanel->m_bVisible == 0);
}

void TMFieldScene::SetVisibleSkillMaster()
{
	SGridControl::m_sLastMouseOverIndex = -1;

	int bVisible = m_pSkillMPanel->IsVisible() == 0;

	m_pSkillPanel->SetVisible(m_pSkillMPanel->m_bVisible == 0);

	if (bVisible == 1)
	{
		m_pSystemPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);

		if (m_pTradePanel && m_pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);

		m_pInvenPanel->SetVisible(0);
		m_pCPanel->SetVisible(0);
		m_pShopPanel->SetVisible(0);
		m_pSkillMPanel->SetVisible(1);
		m_pSkillPanel->SetVisible(1);
		m_pHellgateStore->SetVisible(0);
		m_pGambleStore->SetVisible(0);
		m_pSkillPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f, RenderDevice::m_fHeightRatio * 35.0f);
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}
}

void TMFieldScene::SetVisibleSkill()
{
	SGridControl::m_sLastMouseOverIndex = -1;

	int bVisible = m_pSkillPanel->m_bVisible == 0;

	m_pSkillPanel->SetVisible(bVisible);

	if (bVisible == 1)
	{
		m_pSkillPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f, RenderDevice::m_fHeightRatio * 35.0f);
	}
	else
	{
		m_pShopPanel->SetVisible(0);
		m_pCargoPanel->SetVisible(0);
		m_pHellgateStore->SetVisible(0);
		m_pGambleStore->SetVisible(0);
		m_pSkillMPanel->SetVisible(0);
		m_pDescPanel->SetVisible(0);
		g_pCursor->DetachItem();
	}

	if (g_pSoundManager)
	{
		auto pSoundData = g_pSoundManager->GetSoundData(51);

		if (pSoundData)
			pSoundData->Play(0, 0);
	}

	if (m_pCargoPanel->IsVisible())
		m_pCargoPanel->SetVisible(0);
}

void TMFieldScene::SetVisibleServerWar()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	auto pText = (SText*)m_pControlContainer->FindControl(65888);

	m_nCoinMsgType = 9;
	pText->SetText(g_pMessageStringTable[374], 0);
	m_pControlContainer->SetFocusedControl(pEdit);
	pInputGoldPanel->SetVisible(1);
}

void TMFieldScene::SetVisibleRefuseServerWar()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	auto pText = (SText*)m_pControlContainer->FindControl(65888);

	m_nCoinMsgType = 10;
	pText->SetText(g_pMessageStringTable[375], 0);
	m_pControlContainer->SetFocusedControl(pEdit);
	pInputGoldPanel->SetVisible(1);
}

void TMFieldScene::SetInVisibleInputCoin()
{
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889u);
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;

	m_pControlContainer->SetFocusedControl(nullptr);

	pInputGoldPanel->SetVisible(0);
	pEdit->SetText((char*)"");

	if (m_nLastAutoTradePos >= 0)
	{
		m_pCargoGridList[m_nLastAutoTradePos / 40]->GetAtItem(m_nLastAutoTradePos % 40 % 5,	m_nLastAutoTradePos % 40 / 5)->m_GCObj.dwColor = 0xFFFFFFFF;

		m_nLastAutoTradePos = -1;
	}
	if (g_nKeyType == 1)
		m_pControlContainer->SetFocusedControl(m_pEditChat);
}

void TMFieldScene::SetGridState()
{
	m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;

	SetEquipGridState(1);
}

void TMFieldScene::SetEquipGridState(int bDefault)
{
	if (bDefault == 1)
	{
		m_pGridHelm->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridCoat->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridPants->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridGloves->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridBoots->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridRight->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridLeft->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridGuild->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridEvent->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridRing->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridNecklace->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridOrb->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridCabuncle->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
		m_pGridDRing->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	}
	else
	{
		m_pGridHelm->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridCoat->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridPants->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridGloves->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridBoots->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridRight->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridLeft->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridGuild->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridEvent->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridRing->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridNecklace->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridOrb->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridCabuncle->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
		m_pGridDRing->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
	}

	m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
}

void TMFieldScene::SetVisibleNameLabel()
{
	m_bShowNameLabel = m_bShowNameLabel == 0;

	auto pBtnName = static_cast<SButton*>(m_pControlContainer->FindControl(307u));

	if (pBtnName)
		pBtnName->SetSelected(m_bShowNameLabel == 0);
}

void TMFieldScene::SetVisibleAutoTrade(int bShow, int bCargo)
{
	SGridControl::m_sLastMouseOverIndex = -1;

	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();

	if (m_pAutoTrade)
	{
		auto pBtnCloseAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_CLOSE));
		auto pBtnAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(B_AUTOTRADEBTN));
		auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));
		auto pBtnInv = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));
		auto pMyCargoCoin = static_cast<SText*>(m_pControlContainer->FindControl(TMT_ATRADE_COIN));
		auto pMyCargoCoinB = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_COIN));
		
		if (pBtnCloseAutoTrade)
			pBtnCloseAutoTrade->SetVisible(0);

		bool bSendQuit = false;

		if (pBtnAutoTrade)
			pBtnAutoTrade->SetSelected(bShow);

		if (m_pAutoTrade)
		{
			if (m_pAutoTrade->IsVisible() == 1 && !bShow && !m_pInvenPanel->IsVisible())
				bSendQuit = true;

			m_pAutoTrade->SetVisible(bShow);

			m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 254.0f, RenderDevice::m_fHeightRatio * 35.0f);
		}
		if (bShow == 1)
		{
			g_pCursor->DetachItem();
			m_pSystemPanel->SetVisible(0);
			m_pCPanel->SetVisible(0);
			m_pSkillPanel->SetVisible(0);
			m_pSkillMPanel->SetVisible(0);
			pBtnInv->SetSelected(0);
			pBtnChar->SetSelected(0);
			SetVisibleTrade(0);

			auto pRunAutoTrade = static_cast<SButton*>(m_pControlContainer->FindControl(TMB_ATRADE_RUN));

			if (bCargo == 1)
			{
				m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f, RenderDevice::m_fHeightRatio * 35.0f);
				m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 514.0f, RenderDevice::m_fHeightRatio * 35.0f);
				m_pCargoPanel->SetVisible(1);
				m_pCargoPanel1->SetVisible(1);
				m_pInvenPanel->SetVisible(0);

				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(1);

				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(1);

				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(1);

				for (int i = 0; i < 10; ++i)
				{
					if (m_pGridAutoTrade[i])
						m_pGridAutoTrade[i]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
				}
			}
			else
			{
				m_pCargoPanel->SetVisible(0);
				m_pCargoPanel1->SetVisible(0);

				if (pMyCargoCoin)
					pMyCargoCoin->SetVisible(0);

				if (pMyCargoCoinB)
					pMyCargoCoinB->SetVisible(0);

				m_pInvenPanel->SetVisible(1);

				if (pRunAutoTrade)
					pRunAutoTrade->SetVisible(0);

				for (int j = 0; j < 11; ++j)
				{
					if (m_pGridAutoTrade[j])
						m_pGridAutoTrade[j]->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY2;
				}
				m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
				m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
				m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
				m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
				SetEquipGridState(0);
			}
			m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
			m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
			m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV2;
		}
		else
		{
			sprintf(m_pMyHuman->m_TradeDesc, "");
			m_pMyHuman->m_pAutoTradeDesc->SetText(m_pMyHuman->m_TradeDesc, 0);

			if (m_pInvenPanel->IsVisible() == 1)
			{
				m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
				m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
				m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
				m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
				SetEquipGridState(1);
			}

			for (int k = 0; k < 10; ++k)
			{
				m_stAutoTrade.Header.ID = m_pMyHuman->m_dwID;
				m_stAutoTrade.TargetID = 0;

				SGridControlItem* pItem = m_pGridAutoTrade[k]->PickupAtItem(0, 0);

				int page = m_stAutoTrade.CarryPos[k] / 40;

				SGridControlItem* pSrcItem = m_pCargoGridList[page]->GetAtItem(
					m_stAutoTrade.CarryPos[k] % 40 % 5,
					m_stAutoTrade.CarryPos[k] % 40 / 5);

				if (pSrcItem)
					pSrcItem->m_GCObj.dwColor = 0xFFFFFFFF;

				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pItem);
			}
			m_pInvenPanel->SetVisible(0);
			m_pCargoPanel->SetVisible(0);
			m_pCargoPanel1->SetVisible(0);
			g_pDevice->m_nWidthShift = 0;
			m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
			m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
			m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;

			memset(&m_stAutoTrade, 0, sizeof(m_stAutoTrade));

			if (bSendQuit)
			{
				MSG_STANDARD stStandard{};
				
				stStandard.Type = 0x384;
				stStandard.ID = m_pMyHuman->m_dwID;
				SendOneMessage((char*)&stStandard, sizeof(stStandard));
			}
		}
	}
}

void TMFieldScene::SetVisibleKhepraPortal(bool bVisible)
{
	if (m_pKhepraPortal && m_pKhepraPortalEff1 && m_pKhepraPortalEff2)
	{
		if (bVisible == 1)
		{
			m_pKhepraPortal->m_fHeight = -8.3000002f;
			m_pKhepraPortalEff1->m_vecPosition.y = -4.73f;
			m_pKhepraPortalEff2->m_vecPosition.y = -7.8000002f;
		}
		else
		{
			m_pKhepraPortal->m_fHeight = -20.0f;
			m_pKhepraPortalEff1->m_vecPosition.y = -20.0f;
			m_pKhepraPortalEff2->m_vecPosition.y = -20.0f;
		}
	}
}

void TMFieldScene::VisibleInputTradeName()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(65888u);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889u);
	if (pText && pEdit)
	{
		m_nCoinMsgType = 3;
		pText->SetText(g_pMessageStringTable[141], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);
		auto pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574u);
		if (pInputBG2)
			pInputBG2->SetVisible(1);
		pEdit->m_nMaxStringLen = 20;
		m_pChatSelectPanel->SetVisible(0);
	}
}

void TMFieldScene::VisibleInputPass()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(65888);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	if (pText && pEdit)
	{
		m_nCoinMsgType = 11;
		pText->SetText(g_pMessageStringTable[405], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);

		auto pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);
		if (pInputBG2)
			pInputBG2->SetVisible(1);

		pEdit->m_nMaxStringLen = 12;
	}
}

void TMFieldScene::VisibleInputGuildName()
{
	auto pInputGoldPanel = (SControl*)m_pInputGoldPanel;
	auto pText = (SText*)m_pControlContainer->FindControl(65888);
	auto pEdit = (SEditableText*)m_pControlContainer->FindControl(65889);
	if (pText && pEdit)
	{
		m_nCoinMsgType = 8;
		pText->SetText(g_pMessageStringTable[363], 0);
		m_pControlContainer->SetFocusedControl(pEdit);
		pInputGoldPanel->SetVisible(1);

		auto pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);
		if (pInputBG2)
			pInputBG2->SetVisible(1);

		pEdit->m_nMaxStringLen = 20;
	}
}

void TMFieldScene::VisibleInputCharName(SGridControlItem* pItem, int nCellX, int nCellY)
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
		if (SourPage < 0 || SourPage > 45)
			SourPage = 0;

		memset(&m_stCapsuleItem, 0, sizeof(m_stCapsuleItem));
		m_stCapsuleItem.Header.ID = g_pObjectManager->m_dwCharID;
		m_stCapsuleItem.Header.Type = 972;
		m_stCapsuleItem.SourType = 1;
		m_stCapsuleItem.SourPos = SourPage + SourPos;
		m_stCapsuleItem.ItemID = 0;
		m_stCapsuleItem.GridX = nCellX;
		m_stCapsuleItem.GridY = nCellY;

		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(T_INPUT_GOLD));
		auto pEdit = static_cast<SEditableText*>(m_pControlContainer->FindControl(E_INPUT_GOLD));

		if (pText && pEdit)
		{
			m_nCoinMsgType = 6;
			pText->SetText(g_pMessageStringTable[349], 0);
			m_pControlContainer->SetFocusedControl(pEdit);
			m_pInputGoldPanel->SetVisible(1);

			auto pInputBG2 = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_INPUT_BG2));

			if (pInputBG2)
				pInputBG2->SetVisible(1);

			pEdit->m_nMaxStringLen = 16;
		}
	}
}

void TMFieldScene::Bag_View()
{
	char str[128]{};
	if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
	{
		if (m_bJPNBag[2])
			m_pInvPageBtn3->SetTextureSetIndex(527);
		else
			m_pInvPageBtn3->SetTextureSetIndex(528);

		auto pEffect = g_pObjectManager->m_stMobData.Carry[60].stEffect;

		char szStrDay[128]{};
		char szStrMonth[128]{};

		int nMonth = 0;
		int nDate = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (pEffect[i].cEffect == EF_DATE)
			{
				nDate = (unsigned char)pEffect[i].cValue;
			}
			else if (pEffect[i].cEffect == 110)
			{
				nMonth = (unsigned char)pEffect[i].cValue;
			}
		}

		if (nDate)
			sprintf(szStrDay, g_pMessageStringTable[291], nDate);
		else
			sprintf(szStrDay, (char*)"");
		if (nMonth)
			sprintf(szStrMonth, g_pMessageStringTable[296], nMonth);
		else
			sprintf(szStrMonth, (char*)"");
		if (nDate)
			sprintf(str, "%s%s", szStrMonth, szStrDay);
		else
			sprintf(str, (char*)"");

		m_pJPNBag_Day1->SetText(str, 0);
		m_pJPNBag_Day1->SetTextColor(0xFFFFFF00);
	}
	else if (m_bJPNBag[2])
	{
		m_pInvPageBtn3->SetTextureSetIndex(548);
	}
	else
	{
		m_pInvPageBtn3->SetTextureSetIndex(549);
	}

	if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
	{
		if (m_bJPNBag[3])
			m_pInvPageBtn3->SetTextureSetIndex(527);
		else
			m_pInvPageBtn3->SetTextureSetIndex(528);

		auto pEffect = g_pObjectManager->m_stMobData.Carry[61].stEffect;

		char szStrDay[128]{};
		char szStrMonth[128]{};

		int nMonth = 0;
		int nDate = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (pEffect[i].cEffect == EF_DATE)
			{
				nDate = (unsigned char)pEffect[i].cValue;
			}
			else if (pEffect[i].cEffect == 110)
			{
				nMonth = (unsigned char)pEffect[i].cValue;
			}
		}

		if (nDate)
			sprintf(szStrDay, g_pMessageStringTable[291], nDate);
		else
			sprintf(szStrDay, (char*)"");
		if (nMonth)
			sprintf(szStrMonth, g_pMessageStringTable[296], nMonth);
		else
			sprintf(szStrMonth, (char*)"");
		if (nDate)
			sprintf(str, "%s%s", szStrMonth, szStrDay);
		else
			sprintf(str, (char*)"");

		m_pJPNBag_Day2->SetText(str, 0);
		m_pJPNBag_Day2->SetTextColor(0xFFFFFF00);
	}
	else if (m_bJPNBag[3])
	{
		m_pInvPageBtn4->SetTextureSetIndex(548);
	}
	else
	{
		m_pInvPageBtn4->SetTextureSetIndex(549);
	}
}

int TMFieldScene::AirMove_ShowUI(bool bShow)
{
	auto pPotalPanel = m_pPotalPanel;
	if (!pPotalPanel)
		return 0;
	if (!m_pPotalList)
		return 0;

	pPotalPanel->SetVisible(bShow);
	if (bShow == 1)
	{
		if (m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
			SetVisibleSkill();
		if (m_pCPanel && m_pCPanel->m_bVisible == 1)
			SetVisibleCharInfo();
		if (m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
			SetVisibleCargo(0);
		if (m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
			SetVisibleAutoTrade(0, 0);
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();
		if (m_pShopPanel && m_pShopPanel->m_bVisible == 1)
			SetVisibleShop(0);

		char szStr[128]{};
		sprintf(szStr, "%s", g_pMessageStringTable[378]);
		m_pPotalText->SetText(szStr, 0);
		m_pPotalText->SetTextColor(0xFFFFFFFF);
		if (m_pPotalText1)
			m_pPotalText1->SetText(g_pMessageStringTable[380], 0);
		if (m_pPotalText2)
			m_pPotalText2->SetText(g_pMessageStringTable[379], 0);
		if (m_pPotalText3)
			m_pPotalText3->SetText(g_pMessageStringTable[381], 0);

		LoadMsgText3(m_pQuestList[0], (char*)"UI\\QuestSubjects.txt", 400, 0);
		LoadMsgText3(m_pQuestList[1], (char*)"UI\\QuestSubjects2.txt", 400, 0);
		LoadMsgText3(m_pQuestList[2], (char*)"UI\\QuestSubjects3.txt", 400, 0);
		LoadMsgText3(m_pQuestList[3], (char*)"UI\\QuestSubjects4.txt", 400, 0);

		char strAirMoveList[10][256]{};
		if (!m_pQuestList[0] || !m_pQuestList[1])
			return 0;

		int i = 0;
		char strAirMovePlaceName[10][64]{};
		strncpy(strAirMovePlaceName[0], m_pQuestList[0]->m_pItemList[0]->GetText() + 1, 18);
		strncpy(strAirMovePlaceName[1], m_pQuestList[0]->m_pItemList[1]->GetText() + 1, 18);
		strncpy(strAirMovePlaceName[2], m_pQuestList[0]->m_pItemList[6]->GetText() + 1, 18);
		sprintf(strAirMovePlaceName[3], g_pMessageStringTable[212]);
		sprintf(strAirMovePlaceName[4], g_pMessageStringTable[218]);

		for (int i = 0; i < 5; ++i)
		{
			while (1)
			{
				if (strlen(strAirMovePlaceName[i]) >= 18)
					break;

				strcat(strAirMovePlaceName[i], " ");
			}
		}

		for (i = 0; i < 5; ++i)
			sprintf(strAirMoveList[i], "FFFFFF    %04d %04d  %s               %d", g_pAirMoveList[i].nX, g_pAirMoveList[i].nY, strAirMovePlaceName[i], 0);

		int nCount = 0;
		m_pPotalList->Empty();
		for (int j = 0; j < 5; ++j)
		{
			char szTemp[256]{};
			sprintf(szTemp, "%s", strAirMoveList[j]);

			char szCol[7]{};
			strncpy(szCol, szTemp, 6u);

			unsigned int dwCol = 0;
			sscanf(szCol, "%x", &dwCol);
			
			auto szRet = strstr(szTemp, "\n");
			if (szRet)
				szRet[0] = 0;

			char szText[256]{};
			sprintf(szText, "%s", &szTemp[6]);
	
			auto pItem = new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				m_pPotalList->m_nWidth,
				16.0f,
				0,
				0x77777777,
				1,
				0);

			m_pPotalList->AddItem(pItem);
			if (++nCount > 100)
				break;
		}

		if (m_pPotalList->m_pScrollBar)
			m_pPotalList->m_pScrollBar->SetCurrentPos(0);
	}

	m_bAirmove_ShowUI = bShow;
	if (!bShow)
	{
		auto pCharPanel = m_pCPanel;
		auto pInvPanel = m_pInvenPanel;
		auto pSkillPanel = m_pSkillPanel;
		auto pPartyPanel = m_pPartyPanel;
		auto pTradePanel = m_pTradePanel;
		auto pAutoTradePanel = m_pAutoTrade;
		auto pShopPanel = m_pShopPanel;
		auto pCargoPanel = m_pCargoPanel;
		auto pCargoPanel1 = m_pCargoPanel1;
		auto pMinimapPanel = m_pMiniMapPanel;
		auto pInputGoldPanel = m_pInputGoldPanel;
		auto pPGTPanel = m_pPGTPanel;
		auto pSystemPanel = m_pSystemPanel;
		auto pGambleStore = m_pGambleStore;
		auto pServerPanel = m_pServerPanel;
		auto pPotalPanel = m_pPotalPanel;
		if (g_bActiveWB == 1)
		{
			g_pApp->SwitchWebBrowserState(0);
			return 0;
		}

		if (pAutoTradePanel && pAutoTradePanel->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		else if (pPGTPanel->IsVisible() == 1)
			pPGTPanel->SetVisible(0);
		else if (m_pQuestPanel->IsVisible() == 1)
		{
			m_pQuestPanel->SetVisible(0);
			m_pQuestBtn->SetSelected(0);
			GetSoundAndPlay(51, 0, 0);
		}
		else if (m_pFireWorkPanel && m_pFireWorkPanel->IsVisible() == 1)
			m_pFireWorkPanel->SetVisible(0);
		else if (m_pTotoPanel && m_pTotoPanel->IsVisible() == 1)
			m_pTotoPanel->SetVisible(0);
		else if (pInvPanel->IsVisible() == 1)
			OnControlEvent(65562, 0);
		else if (pSkillPanel->IsVisible() == 1)
			OnControlEvent(65568, 0);
		else if (pCharPanel->IsVisible() == 1)
			OnControlEvent(65769, 0);
		else if (pTradePanel->IsVisible() == 1)
			SetVisibleTrade(0);
		else if (pPartyPanel->IsVisible() == 1)
			SetVisibleParty();
		else if (pShopPanel->IsVisible()== 1)
			SetVisibleShop(0);
		else if (pCargoPanel->IsVisible() == 1)
			SetVisibleCargo(0);
		else if (pCargoPanel1->IsVisible() == 1)
			SetVisibleCargo(0);
		else if (pGambleStore->IsVisible() == 1)
			SetVisibleGamble(0, 0);
		else if (pInputGoldPanel->IsVisible() == 1)
			pInputGoldPanel->SetVisible(0);
		else if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)
		{
			m_pMsgPanel->SetVisible(0);
			m_pControlContainer->SetFocusedControl(0);
		}
		else if (m_pHelpPanel && m_pHelpPanel->IsVisible() == 1)
		{
			m_pHelpPanel->SetVisible(0);
			m_pHelpBtn->SetSelected(0);
			GetSoundAndPlay(51, 0, 0);
		}
		else if (pServerPanel && pServerPanel->IsVisible() == 1)
			pServerPanel->SetVisible(0);
		else if (pPotalPanel && pPotalPanel->IsVisible() == 1)
			pPotalPanel->SetVisible(0);
		else if (m_pMessageBox && m_pMessageBox->IsVisible() == 1)
			m_pMessageBox->SetVisible(0);
	}

	return 1;
}

void TMFieldScene::SetVisibleMixPanel(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}

		m_ItemMixClass.m_pMixPanel->SetVisible(1);
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV8;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV8;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV8;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV8;
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_ItemMixClass.m_pMixPanel->SetVisible(0);
		ClearMixPannel();
	}
}

void TMFieldScene::ClearMixPannel()
{
	for (int i = 0; i < 4; ++i)
	{
		auto pGridInv = m_pGridInvList[i];
		for (int nY = 0; nY < 3; ++nY)
		{
			for (int nX = 0; nX < 5; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	m_ItemMixClass.ClearGridList();
	m_ItemMixClass.m_stCombineItem.Header.ID = m_pMyHuman->m_dwID;
	m_ItemMixClass.m_stCombineItem.Header.Type = MSG_CombineItemTiny_Opcode;

	m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	SetEquipGridState(1);
}

void TMFieldScene::SetVisibleMissionPanel(int bShow)
{
	SGridControl::m_sLastMouseOverIndex = -1;
	if (m_pInputGoldPanel->IsVisible() == 1)
		SetInVisibleInputCoin();
	if (bShow == 1 && m_pSkillPanel && m_pSkillPanel->m_bVisible == 1)
		SetVisibleSkill();
	if (bShow == 1 && m_pCPanel && m_pCPanel->m_bVisible == 1)
		SetVisibleCharInfo();
	if (bShow == 1 && m_pCargoPanel && m_pCargoPanel->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pCargoPanel1 && m_pCargoPanel1->m_bVisible == 1)
		SetVisibleCargo(1);
	if (bShow == 1 && m_pAutoTrade && m_pAutoTrade->m_bVisible == 1)
		SetVisibleAutoTrade(0, 0);

	if (bShow == 1)
	{
		if (m_pInvenPanel)
		{
			if (!m_pInvenPanel->m_bVisible)
				SetVisibleInventory();
		}

		m_MissionClass.m_pMissionPanel->SetVisible(1);
		m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

		g_pDevice->m_nWidthShift = 0;
		g_pCursor->DetachItem();
		m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_TRADEINV3;
		SetEquipGridState(0);
	}
	else
	{
		if (m_pInvenPanel && m_pInvenPanel->m_bVisible == 1)
			SetVisibleInventory();

		g_pDevice->m_nWidthShift = 0;
		m_MissionClass.m_pMissionPanel->SetVisible(0);
		ClearMissionPannel();
	}
}

void TMFieldScene::ClearMissionPannel()
{
	for (int i = 0; i < 4; ++i)
	{
		auto pGridInv = m_pGridInvList[i];
		for (int nY = 0; nY < 3; ++nY)
		{
			for (int nX = 0; nX < 5; ++nX)
			{
				auto pItem = pGridInv->GetItem(nX, nY);
				if (pItem)
				{
					if (pItem->m_GCObj.dwColor == 0xFFFF0000)
						pItem->m_GCObj.dwColor = 0xFFFFFFFF;
				}
			}
		}
	}

	m_MissionClass.ClearGridList();
	m_MissionClass.m_stCombineItem.Header.ID = m_pMyHuman->m_dwID;
	m_MissionClass.m_stCombineItem.Header.Type = MSG_CombineItemTiny_Opcode;
	m_pGridInvList[0]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[1]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[2]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	m_pGridInvList[3]->m_eGridType = TMEGRIDTYPE::GRID_DEFAULT;
	SetEquipGridState(1);
}
