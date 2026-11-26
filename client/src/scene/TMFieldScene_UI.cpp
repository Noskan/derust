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

/// @file TMFieldScene_UI.cpp
/// @brief Eventos de controles/HUD, janelas, inventario, chat, guildmarks e paineis de composicao.

/// Processa eventos de clique e interacao nos paineis, botoes e grids da interface.
int TMFieldScene::OnControlEvent(unsigned int idwControlID, unsigned int idwEvent)
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (dwServerTime < g_dwStartQuitGameTime + 6000)
		return 1;
	if (dwServerTime < m_dwLastLogout + 6000)
		return 1;
	if (dwServerTime < m_dwLastSelServer + 6000)
		return 1;
	if (dwServerTime < m_dwLastTown + 6000)
		return 1;
	if (dwServerTime < m_dwLastResurrect + 6000)
		return 1;
	if (dwServerTime < m_dwLastTeleport + 6000)
		return 1;
	if (dwServerTime < m_dwLastRelo + 6000)
		return 1;
	if (dwServerTime < m_dwLastWhisper + 6000)
		return 1;

	auto pMobData = &g_pObjectManager->m_stMobData;
	int cktrans = 0;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		cktrans = 1;

	if (idwControlID == TMB_IME_BUTTON)
	{
		if (m_pControlContainer->m_pFocusControl
			&& m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT && m_pControlContainer->m_pFocusControl->m_bVisible)
		{
			if (g_pEventTranslator->IsNative())
				g_pEventTranslator->SetIMEAlphaNumeric();
			else
				g_pEventTranslator->SetIMENative();
		}
	}
	else if (idwControlID == E_CHAT && idwEvent == 8)
		return 1;
	if (idwControlID == TMB_HELLSTORE_OK)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[144], m_nHellStoreValue, 0);
			m_pMessageBox->m_dwArg = m_dwHellStoreID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}
	if (idwControlID >= TMB_HELLSTORE_SELECT_1 && idwControlID <= TMB_HELLSTORE_SELECT_4)
		return OnControl_HellStoreSelect(idwControlID, idwEvent);
	if (idwControlID >= TMB_GAMBLE_LEFTX2 && idwControlID <= TMB_GAMBLE_RIGHT)
		return OnControl_GambleArrows(idwControlID, idwEvent);
	if (idwControlID == TMB_GAMBLE_START)
		return OnControl_GambleStart(idwControlID, idwEvent, pMobData);
	if (idwControlID == TMB_GAMBLE_QUIT)
	{
		SetVisibleGamble(0, 0);
		return 0;
	}
	if (idwControlID == E_CHAT && !idwEvent)
		return OnControl_ChatInput(idwControlID, idwEvent, dwServerTime, pMobData);
	if (idwControlID == E_CHAT && (idwEvent == 2 || idwEvent == 3))
	{
		char szText[128]{};
		sprintf_s(szText, m_szLastChatList[m_sChatIndex]);

		m_pEditChat->SetText(szText);
		++m_sChatIndex;
		m_sChatIndex %= 5;
		return 0;
	}
	if (idwControlID == E_CHAT && (idwEvent == 4 || idwEvent == 5))
	{
		char szWhisperList[19]{};
		sprintf(szWhisperList, "/%s ", m_szWhisperList[m_sWhisperIndex]);

		m_pEditChat->SetText(szWhisperList);
		++m_sWhisperIndex;
		m_sWhisperIndex %= 5;
		return 0;
	}
	if (idwControlID == E_CHAT && idwEvent == 6)
	{
		m_pEditChat->SetText((char*)"");
		return 0;
	}
	if (idwControlID == B_MONEY)
		return OnControl_Money(idwControlID, idwEvent);
	if (idwControlID == B_CARGO_MONEY)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;

		if (!m_pAutoTrade || m_pAutoTrade->IsVisible() != 1)
		{
			auto pText = static_cast<SText*>(m_pControlContainer->FindControl(T_INPUT_GOLD));

			if (m_pCargoPanel->IsVisible() == 1)
			{
				m_nCoinMsgType = 2;
				pText->SetText(g_pMessageStringTable[138], 0);
				m_pControlContainer->SetFocusedControl(m_pControlContainer->FindControl(E_INPUT_GOLD));
				m_pInputGoldPanel->SetVisible(1);
				m_pChatSelectPanel->SetVisible(0);
			}
		}
		return 1;
	}
	if (idwControlID == B_IG_OK)
		return OnControl_InputOk(idwControlID, idwEvent);
	if (idwControlID == E_INPUT_GOLD)
	{
		if (m_pMyHuman->m_cDie == 1 || m_nCoinMsgType == 5)
			return 1;

		OnControlEvent(B_IG_OK, 0);
		return 1;
	}
	if (idwControlID == B_IG_CANCEL)
	{
		SetInVisibleInputCoin();
		return 0;
	}
	if (idwControlID == B_INV_CLOSE)
	{
		SetVisibleInventory();

		auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

		pEquipBtn->SetSelected(m_pInvenPanel->m_bVisible);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_CHAR_CLOSE)
	{
		SetVisibleCharInfo();

		auto pCharBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));

		pCharBtn->SetSelected(m_pCPanel->m_bVisible);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_SKILL_CLOSE)
	{
		SetVisibleSkill();

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_CHAR)
	{
		auto pATradePanel = static_cast<SPanel*>(m_pControlContainer->FindControl(TMP_ATRADE_PANEL));

		if (pATradePanel && pATradePanel->IsVisible())
			return 1;

		if (m_pShopPanel->IsVisible() == 1)
			return 1;

		if (m_pTradePanel->IsVisible() != 1)
		{
			auto pCharBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));

			pCharBtn->SetSelected(pCharBtn->m_bSelected == 0);

			SetVisibleCharInfo();

			return 0;
		}

		return 1;
	}
	if (idwControlID == B_EQUIP)
	{
		auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

		pEquipBtn->SetSelected(pEquipBtn->m_bSelected == 0);

		SetVisibleInventory();
		return 0;
	}
	if (idwControlID == B_CCMODE_SYSTEM)
	{
		if (m_pccmode->IsVisible())
		{
			m_pccmode->SetVisible(0);
		}
		else
		{
			m_pccmode->SetVisible(1);

			char szText[128]{};

			sprintf_s(szText, g_pMessageStringTable[476], g_GameAuto_hpValue, g_GameAuto_mountValue);

			auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

			if (pChatItem)
				m_pChatList->AddItem(pChatItem);
		}

		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_MOUNT)
	{
		g_GameAuto_mountValue += 10;

		if (g_GameAuto_mountValue > 90)
			g_GameAuto_mountValue = 30;

		char szText[128]{};

		if (g_GameAuto_mountValue)
			sprintf_s(szText, g_pMessageStringTable[478], g_GameAuto_mountValue);
		else
			sprintf_s(szText, g_pMessageStringTable[477]);

		auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pChatItem)
			m_pChatList->AddItem(pChatItem);

		sprintf_s(szText, "%d%%", g_GameAuto_mountValue);

		m_pCCModeMountSte->SetText(szText, 0);

		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_HP)
	{
		g_GameAuto_hpValue += 10;

		if (g_GameAuto_hpValue > 90)
			g_GameAuto_hpValue = 0;

		char szText[128]{};

		if (g_GameAuto_hpValue)
			sprintf_s(szText, g_pMessageStringTable[480], g_GameAuto_hpValue);
		else
			sprintf_s(szText, g_pMessageStringTable[479]);

		auto pChatItem = new SListBoxItem(szText, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pChatItem)
			m_pChatList->AddItem(pChatItem);

		sprintf_s(szText, "%d%%", g_GameAuto_hpValue);

		m_pCCModeHpSte->SetText(szText, 0);
		return 0;
	}
	if (idwControlID == B_CCMODE_DLG_MODE)
	{
		if (++g_GameAuto > 3)
			g_GameAuto = 0;

		switch (g_GameAuto)
		{
		case 0:
			m_pMGameAutoBtn->SetTextureSetIndex(458);
			m_pMGameAutoBtn->m_pAltText->SetText(g_UIString[229], 0);
			break;
		case 1:
			m_pMGameAutoBtn->SetTextureSetIndex(455);
			m_pMGameAutoBtn->m_pAltText->SetText(g_UIString[226], 0);
			break;
		case 2:
			m_pMGameAutoBtn->SetTextureSetIndex(456);
			m_pMGameAutoBtn->m_pAltText->SetText(g_UIString[227], 0);
			break;
		case 3:
			m_pMGameAutoBtn->SetTextureSetIndex(459);
			m_pMGameAutoBtn->m_pAltText->SetText(g_UIString[230], 0);
			break;
		}

		return 0;
	}
	if (idwControlID == B_CHATLIST_SIZEUP)
	{
		OnKeyPlus(43, 0);
		return 0;
	}
	if (idwControlID == B_CHATLIST_LIGHT)
	{
		static short Color = 0;
		switch (Color)
		{
		case 0:
			m_pChatBack->m_GCPanel.dwColor = 0x66000000;
			Color = 1;
			break;
		case 1:
			m_pChatBack->m_GCPanel.dwColor = 0x88000000;
			Color = 2;
			break;
		case 2:
			m_pChatBack->m_GCPanel.dwColor = 0xAA000000;
			Color = 3;
			break;
		case 3:
			m_pChatBack->m_GCPanel.dwColor = 0xFF000000;
			Color = 4;
			break;
		case 4:
			m_pChatBack->m_GCPanel.dwColor = 0;
			Color = 0;
			break;
		}
		return 0;
	}
	if (idwControlID == B_CHAT_SELECT)
	{
		if (m_pChatListPanel->IsVisible())
			m_pChatListPanel->SetVisible(0);
		else
			m_pChatListPanel->SetVisible(1);

		return 0;
	}
	if (idwControlID >= B_CHAT_SELECT_NOMAL && idwControlID <= B_CHAT_SELECT_SHOUT)
		return OnControl_ChatSelect(idwControlID, idwEvent);
	if (idwControlID == B_INV_PAGE1)
	{
		m_bJPNBag[0] = 1;
		m_bJPNBag[3] = 0;
		m_bJPNBag[2] = 0;
		m_bJPNBag[1] = 0;
		m_pGridInvList[0]->SetVisible(0);
		m_pGridInvList[1]->SetVisible(0);
		m_pGridInvList[2]->SetVisible(0);
		m_pGridInvList[3]->SetVisible(0);
		m_pGridInv = m_pGridInvList[0];
		m_pGridInv->SetVisible(1);
		m_pInvPageBtn1->SetTextureSetIndex(527);
		m_pInvPageBtn2->SetTextureSetIndex(528);

		if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
			m_pInvPageBtn3->SetTextureSetIndex(528);
		else
			m_pInvPageBtn3->SetTextureSetIndex(549);

		if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
			m_pInvPageBtn4->SetTextureSetIndex(528);
		else
			m_pInvPageBtn4->SetTextureSetIndex(549);

		return 0;
	}
	if (idwControlID == B_INV_PAGE2)
	{
		m_bJPNBag[1] = 1;
		m_bJPNBag[3] = 0;
		m_bJPNBag[2] = 0;
		m_bJPNBag[0] = 0;
		m_pGridInvList[0]->SetVisible(0);
		m_pGridInvList[1]->SetVisible(0);
		m_pGridInvList[2]->SetVisible(0);
		m_pGridInvList[3]->SetVisible(0);
		m_pGridInv = m_pGridInvList[1];
		m_pGridInv->SetVisible(1);
		m_pInvPageBtn1->SetTextureSetIndex(528);
		m_pInvPageBtn2->SetTextureSetIndex(527);

		if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
			m_pInvPageBtn3->SetTextureSetIndex(528);
		else
			m_pInvPageBtn3->SetTextureSetIndex(549);

		if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
			m_pInvPageBtn4->SetTextureSetIndex(528);
		else
			m_pInvPageBtn4->SetTextureSetIndex(549);

		return 0;
	}
	if (idwControlID == B_INV_PAGE3)
	{
		m_bJPNBag[2] = 1;
		m_bJPNBag[3] = 0;
		m_bJPNBag[1] = 0;
		m_bJPNBag[0] = 0;
		m_pGridInvList[0]->SetVisible(0);
		m_pGridInvList[1]->SetVisible(0);
		m_pGridInvList[2]->SetVisible(0);
		m_pGridInvList[3]->SetVisible(0);
		m_pGridInv = m_pGridInvList[2];
		m_pGridInv->SetVisible(1);
		m_pInvPageBtn1->SetTextureSetIndex(528);
		m_pInvPageBtn2->SetTextureSetIndex(528);

		if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
			m_pInvPageBtn3->SetTextureSetIndex(527);
		else
			m_pInvPageBtn3->SetTextureSetIndex(548);

		if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
			m_pInvPageBtn4->SetTextureSetIndex(528);
		else
			m_pInvPageBtn4->SetTextureSetIndex(549);

		return 0;
	}
	if (idwControlID == B_INV_PAGE4)
	{
		m_bJPNBag[3] = 1;
		m_bJPNBag[2] = 0;
		m_bJPNBag[1] = 0;
		m_bJPNBag[0] = 0;
		m_pGridInvList[0]->SetVisible(0);
		m_pGridInvList[1]->SetVisible(0);
		m_pGridInvList[2]->SetVisible(0);
		m_pGridInvList[3]->SetVisible(0);
		m_pGridInv = m_pGridInvList[3];
		m_pGridInv->SetVisible(1);
		m_pInvPageBtn1->SetTextureSetIndex(528);
		m_pInvPageBtn2->SetTextureSetIndex(528);

		if (g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467)
			m_pInvPageBtn3->SetTextureSetIndex(528);
		else
			m_pInvPageBtn3->SetTextureSetIndex(549);

		if (g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467)
			m_pInvPageBtn4->SetTextureSetIndex(527);
		else
			m_pInvPageBtn4->SetTextureSetIndex(548);

		return 0;
	}
	if (idwControlID == B_CARGO_PAGE1)
	{
		m_pCargoGridList[0]->SetVisible(0);
		m_pCargoGridList[1]->SetVisible(0);
		m_pCargoGridList[2]->SetVisible(0);
		m_pCargoGrid = m_pCargoGridList[0];
		m_pCargoGrid->SetVisible(1);
		m_pStorePageBtn1->SetTextureSetIndex(532);
		m_pStorePageBtn2->SetTextureSetIndex(533);
		m_pStorePageBtn3->SetTextureSetIndex(535);
		return 0;
	}
	if (idwControlID == B_CARGO_PAGE2)
	{
		m_pCargoGridList[0]->SetVisible(0);
		m_pCargoGridList[1]->SetVisible(0);
		m_pCargoGridList[2]->SetVisible(0);
		m_pCargoGrid = m_pCargoGridList[1];
		m_pCargoGrid->SetVisible(1);
		m_pStorePageBtn1->SetTextureSetIndex(531);
		m_pStorePageBtn2->SetTextureSetIndex(534);
		m_pStorePageBtn3->SetTextureSetIndex(535);
		return 0;
	}
	if (idwControlID == B_CARGO_PAGE3)
	{
		m_pCargoGridList[0]->SetVisible(0);
		m_pCargoGridList[1]->SetVisible(0);
		m_pCargoGridList[2]->SetVisible(0);
		m_pCargoGrid = m_pCargoGridList[2];
		m_pCargoGrid->SetVisible(1);
		m_pStorePageBtn1->SetTextureSetIndex(531);
		m_pStorePageBtn2->SetTextureSetIndex(533);
		m_pStorePageBtn3->SetTextureSetIndex(536);
		return 0;
	}
	if (idwControlID == P_CCMODE_DLG_PONT)
		return OnControl_CCModeDialog(idwControlID, idwEvent);
	if (idwControlID == TMB_MINIMAP)
	{
		SetVisibleMiniMap();
		return 0;
	}
	if (idwControlID == B_SYSTEM)
	{
		m_pSystemPanel->SetVisible(1);
		g_pCursor->DetachItem();
		return 1;
	}
	if (idwControlID == TMB_CAMERABTN)
	{
		SetCameraView();
		return 1;
	}
	if (idwControlID == TMB_PKBTN)
	{
		SetPK();
		return 1;
	}
	if (idwControlID == TMB_NAMEBTN)
	{
		SetVisibleNameLabel();
		return 1;
	}
	if (idwControlID == TMB_AUTOTARGETBTN)
	{
		SetAutoTarget();
		return 1;
	}
	if (idwControlID == TMB_HPOTION)
	{
		UseHPotion();
		return 1;
	}
	if (idwControlID == TMB_MPOTION)
	{
		UseMPotion();
		return 1;
	}
	if (idwControlID == TMB_PPOTION)
	{
		UsePPotion();
		return 1;
	}
	if (idwControlID == TMB_PARTY_AUTOOK)
	{
		m_bAutoParty = m_bAutoParty == 0;
		return 1;
	}
	if (idwControlID == TMB_MSG_OK)
	{
		m_pMsgPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(0);
		return 1;
	}
	if (idwControlID == B_AUTOTRADEBTN)
	{
		if (!m_pMyHuman->IsInTown())
			return 1;

		if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
			SetVisibleAutoTrade(0, 0);
		else
			VisibleInputTradeName();
		return 1;
	}
	if (idwControlID == B_CHAT_WHISPER)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		sprintf(stMsgChat.String, "whisper");
		SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));
		return 1;
	}
	if (idwControlID == B_CHAT_PARTY)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		sprintf(stMsgChat.String, "partychat");
		SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));
		return 1;
	}
	if (idwControlID == B_CHAT_KINGDOM)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		sprintf(stMsgChat.String, "kingdomchat");
		SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));
		return 1;
	}
	if (idwControlID == B_CHAT_GUILD)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;
		sprintf(stMsgChat.String, "guildchat");
		SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));
		return 1;
	}
	if (idwControlID == TMB_GUILDONOFF)
	{
		MSG_MessageChat stMsgChat{};
		stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgChat.Header.Type = MSG_MessageChat_Opcode;

		if (m_pBtnGuildOnOff)
		{
			if(m_pBtnGuildOnOff->m_bSelected == 1)
				sprintf(stMsgChat.String, "guildon");
			else
				sprintf(stMsgChat.String, "guildoff");
		}
		else
		{
			m_cGuildOnOff = m_cGuildOnOff == 0;
			if (m_cGuildOnOff == 1)
				sprintf(stMsgChat.String, "guildon");
			else
				sprintf(stMsgChat.String, "guildoff");
		}
		SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));
		return 1;
	}
	if (idwControlID == TMB_RUNMODE)
	{
		SetRunMode();
		return 0;
	}
	if (idwControlID == B_SYS_QUIT)
	{
		SetRunMode();
		if (g_dwStartQuitGameTime)
			return 1;

		g_dwStartQuitGameTime = g_pTimerManager->GetServerTime();

		MSG_STANDARDPARM stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		g_pSocketManager->SendOneMessage((char*)&stParm, sizeof(stParm));
		return 0;
	}
	if (idwControlID == P_MINIBTNPANEL_BTN)
	{
		if (m_pMiniPanel->m_bVisible)
			m_pMiniPanel->SetVisible(0);
		else
			m_pMiniPanel->SetVisible(1);
		if (m_pMiniBtn)
			m_pMiniBtn->m_bSelected = m_pMiniPanel->m_bVisible;
		return 0;
	}
	if (idwControlID == B_SYS_SERVER)
	{
		if (!m_pServerPanel)
		{
			m_dwLastSelServer = g_pTimerManager->GetServerTime();
			MSG_STANDARDPARM stParm{};
			stParm.Header.ID = m_pMyHuman->m_dwID;
			stParm.Header.Type = MSG_SysQuit_Opcode;
			g_pSocketManager->SendOneMessage((char*)&stParm, sizeof(stParm));
			return 1;
		}

		m_pSystemPanel->SetVisible(0);

		m_pMessagePanel->SetMessage(g_pMessageStringTable[23], 0);
		m_pMessagePanel->SetVisible(1, 0);

		auto serverGroup = 0; // v445
		for (int nn = 0; nn < MAX_SERVERNUMBER; ++nn)
		{
			if (!g_pServerList[nn][0][0])
			{
				serverGroup = nn - 1;
				break;
			}
		}

		int nDay[10] = { 0 }; // v678;

		_SYSTEMTIME time{};
		GetLocalTime(&time);
		int wDay = time.wDay % 10;
		if (!wDay)
			wDay = 10;

		for (int i = 0; i < MAX_SERVERGROUP; ++i)
		{
			for (int k = 1; k < MAX_SERVERNUMBER; ++k)
				if (g_pServerList[i][k][0] != 0)
					++nDay[i];

			if (nDay[i])
				nDay[i] = !(time.wDay % nDay[i]) ? nDay[i] : time.wDay % nDay[i];
		}

		auto currentServerGroupIndex = g_pObjectManager->m_nServerGroupIndex; // v438

		char szUserCount[1024] = { 0 };
		int nUserCount[MAX_SERVERNUMBER] = { 0 };
		int nUserCount2[MAX_SERVERNUMBER] = { 0 };
		if (currentServerGroupIndex == serverGroup)
		{
			for (int i = serverGroup; i < MAX_SERVERGROUP; ++i)
				g_pServerList[i][0][0] = 0;

			for (int i = 0; i < serverGroup; ++i)
			{
				memset(nUserCount2, -1, sizeof nUserCount2);
				BASE_GetHttpRequest(g_pServerList[i][0], szUserCount, sizeof szUserCount);

				sscanf_s(szUserCount, "%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n",
					&nUserCount2[0], &nUserCount2[1], &nUserCount2[2], &nUserCount2[3], &nUserCount2[4], &nUserCount2[5],
					&nUserCount2[6], &nUserCount2[7], &nUserCount2[8], &nUserCount2[9], &nUserCount2[10]);

				// 
				nUserCount[nDay[serverGroup- i]] = nUserCount2[nDay[serverGroup - i]];
				sprintf_s(g_pServerList[currentServerGroupIndex][i + 1], "%s", g_pServerList[serverGroup - i - 1][nDay[serverGroup - i] + 1]);
			}
		}
		else
		{
			BASE_GetHttpRequest(g_pServerList[currentServerGroupIndex][0], szUserCount, sizeof szUserCount);
			sscanf_s(szUserCount, "%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n%d\\n",
				&nUserCount[0], &nUserCount[1], &nUserCount[2], &nUserCount[3], &nUserCount[4], &nUserCount[5],
				&nUserCount[6], &nUserCount[7], &nUserCount[8], &nUserCount[9]);
		}

		m_pMessagePanel->SetVisible(0, 1);

		SListBox* pServerList = m_pServerList;
		if (pServerList)
		{
			pServerList->Empty();
			for (int num = 1;; ++num)
			{
				if (num >= MAX_SERVERNUMBER)
				{
					pServerList->SetVisible(1);

					if (m_pServerPanel)
						m_pServerPanel->SetVisible(1);
					break;
				}

				if (g_pServerList[currentServerGroupIndex][num][0])
				{
					char iStrText[32] = { 0 }; // original = 14
					if (serverGroup == currentServerGroupIndex)
					{
						if (currentServerGroupIndex - num < 0)
							continue;

						if (g_szServerName[currentServerGroupIndex - num][nDay[currentServerGroupIndex - num]][0])
							sprintf_s(iStrText, "%s-%s", g_szServerNameList[currentServerGroupIndex - num], g_szServerName[currentServerGroupIndex - num][nDay[currentServerGroupIndex - num] - 1]);
						else
						{
							sprintf_s(iStrText, "%s-%d", g_szServerNameList[currentServerGroupIndex - num], nDay[currentServerGroupIndex - num]);
							if (nUserCount[num] > 500)
							{
								int len = strlen(iStrText);

								if (len < 14)
								{
									for (int n1 = len; n1 < len; ++n1)
										iStrText[n1] = ' ';
								}

								iStrText[14] = 0;
								strcat(iStrText, "FULL");
							}
						}
					}
					else if (g_szServerNameList[currentServerGroupIndex][0])
					{
						if (g_szServerName[currentServerGroupIndex][num - 1][0])
							sprintf_s(iStrText, "%s-%s", g_szServerNameList[currentServerGroupIndex], g_szServerName[currentServerGroupIndex][num - 1]);
						else
						{
							sprintf_s(iStrText, "%s-%d", g_szServerNameList[currentServerGroupIndex], num);

							if (nUserCount[num] > 600)
							{
								int len = strlen(iStrText);

								if (len < 14)
								{
									for (int n1 = len; n1 < len; ++n1)
										iStrText[n1] = ' ';
								}

								iStrText[14] = 0;
								strcat(iStrText, "FULL");
							}
						}
					}
					else
						sprintf_s(iStrText, g_pMessageStringTable[68], num + 1, num);

					int nCount = nUserCount[num];
					if (nCount < 0)
						nCount = 0;

					int nTextureSet = -1;
					if (nDay[currentServerGroupIndex] == num)
						nTextureSet = -2;

					if (currentServerGroupIndex == serverGroup)
						nTextureSet = -2;

					// -1??
					auto server = new SListBoxServerItem(nTextureSet, iStrText, 0xFFFFFFFF, 0.0f, 0.0f, static_cast<float>(g_nChannelWidth), 16.0f, nCount, 0, 0, num);

					if (nUserCount[num] < 0)
						server->m_cConnected = 0;
					pServerList->AddItem(server);
				}
				else if (serverGroup == currentServerGroupIndex && num < serverGroup)
				{
					char iStrTexr[14] = { 0 };
					sprintf_s(iStrTexr, g_pMessageStringTable[70]);

					auto server = new SListBoxServerItem(6, iStrTexr, 0xFFFFFFFF, 0.0f, 0.0f, static_cast<float>(g_nChannelWidth), 16.0f, nUserCount2[num], 0, 0, 0);
					if (nUserCount[num] < 0)
						server->m_cConnected = 0;

					pServerList->AddItem(server);
				}
			}
		}
	}

	if (idwControlID == B_QUEST_BUTTON)
		return OnControl_QuestButton1(idwControlID, idwEvent);
	if (idwControlID == B_QUEST_BUTTON2)
		return OnControl_QuestButton2(idwControlID, idwEvent);
	if (idwControlID == B_QUEST_BUTTON3)
		return OnControl_QuestButton3(idwControlID, idwEvent);
	if (idwControlID == B_QUEST_BUTTON4)
		return OnControl_QuestButton4(idwControlID, idwEvent);
	if (idwControlID == B_QUEST_MEMO)
		return OnControl_QuestMemo(idwControlID, idwEvent, pMobData);
	if (idwControlID == TMB_HELP_BUTTON1)
	{
		if (m_pHelpInterface)
			m_pHelpInterface->SetVisible(1);
		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON2)
	{
		if (m_pHelpInterface)
			m_pHelpInterface->SetVisible(0);
		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(1);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON3)
	{
		if (m_pHelpInterface)
			m_pHelpInterface->SetVisible(0);
		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(1);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON4)
	{
		if (m_pHelpInterface)
			m_pHelpInterface->SetVisible(0);
		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(1);
		return 1;
	}
	if (idwControlID == TMB_HELP_BUTTON5)
	{
		m_pHelpPanel->SetVisible(0);
		return 1;
	}
	if (idwControlID == TMB_HELP_MEMO)
	{
		if (m_pHelpPanel)
		{
			m_pHelpPanel->SetVisible(1);
			m_pHelpBtn->SetSelected(1);
			OnControlEvent(873, 0);
		}
		return 1;
	}
	if (idwControlID == TMB_HELP_SUMMON)
	{
		char szStr[128]{};
		sprintf(szStr, g_pMessageStringTable[228], m_szSummoner);

		m_pMessageBox->SetMessage(szStr, 228u, g_pMessageStringTable[229]);
		m_pMessageBox->SetVisible(1);
		m_pHelpSummon->SetVisible(0);

		return 0;
	}
	if (idwControlID == TMB_HELP_OK)
	{
		if (m_pHelpPanel)
		{
			m_pHelpPanel->SetVisible(0);
			m_pHelpBtn->SetSelected(0);
		}

		return 0;
	}
	if (idwControlID == B_QUEST_QUIT)
	{
		if (m_pQuestPanel)
		{
			m_pQuestPanel->SetVisible(0);
			m_pQuestBtn->SetSelected(0);
		}

		return 0;
	}
	if (idwControlID == B_CCATTACK)
		return OnControl_CCAttack(idwControlID, idwEvent);
	if (idwControlID == B_CCPOTION)
	{
		if (++m_AutoHpMp >= 4)
			m_AutoHpMp = 0;
		if (m_AutoHpMp)
		{
			switch (m_AutoHpMp)
			{
			case 1:
				m_pSGameAutoBtn->SetTextureSetIndex(461);
				break;
			case 2:
				m_pSGameAutoBtn->SetTextureSetIndex(460);
				break;
			case 3:
				m_pSGameAutoBtn->SetTextureSetIndex(466);
				break;
			}
		}
		else
		{
			m_pSGameAutoBtn->SetTextureSetIndex(462);
		}
		return 1;
	}
	if (idwControlID == B_CCMOVE)
		return OnControl_CCMove(idwControlID, idwEvent);
	if (idwControlID == B_ITEMMIX_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_ITEMMIX_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == TMB_ITEMMIX4_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], TMB_ITEMMIX4_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_ITEM_MIX_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_ITEM_MIX_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_MISSION_RUN)
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[319], B_MISSION_RUN, 0);
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == B_SHORTSKILL_TGL1)
	{
		m_pGridSkillBelt2->SetVisible(1);
		m_pGridSkillBelt3->SetVisible(0);

		m_pShortSkillTglBtn1->SetSelected(1);
		m_pShortSkillTglBtn2->SetSelected(0);

		GetSoundAndPlay(53, 0, 0);
		
		m_pControlContainer->SetFocusedControl(0);
		m_pShortSkill_Txt->SetText((char*)"1", 0);
		OnKeyShortSkill(49, 0);
		m_bSkillBeltSwitch = 0;

		return 0;
	}
	if (idwControlID == B_SHORTSKILL_TGL2)
	{
		m_pGridSkillBelt2->SetVisible(0);
		m_pGridSkillBelt3->SetVisible(1);

		m_pShortSkillTglBtn1->SetSelected(0);
		m_pShortSkillTglBtn2->SetSelected(1);

		GetSoundAndPlay(53, 0, 0);

		m_pControlContainer->SetFocusedControl(0);
		m_pShortSkill_Txt->SetText((char*)"2", 0);
		OnKeyShortSkill(49, 0);
		m_bSkillBeltSwitch = 1;

		return 0;
	}
	if (idwControlID == B_HELP)
	{
		m_pControlContainer->SetFocusedControl(0);
		OnKeyHelp(104, 0);

		return 0;
	}
	if (idwControlID == B_COMMUNITY)
	{
		g_pApp->SwitchWebBoard();

		return 0;
	}
	if (idwControlID == B_QUESTLOG)
		return OnControl_QuestLog(idwControlID, idwEvent, pMobData);
	if (idwControlID == 1054260)
	{
		TMScene::LoadMsgText2(
			m_pQuestContentList[0],
			(char*)"UI\\QuestContents.txt",
			20 * idwEvent,
			20 * (idwEvent + 1) - 1);

		return 0;
	}
	if (idwControlID == 1054263)
	{
		TMScene::LoadMsgText2(
			m_pQuestContentList[1],
			(char*)"UI\\QuestContents2.txt",
			20 * idwEvent,
			20 * (idwEvent + 1) - 1);

		return 0;
	}
	if (idwControlID == 1054266)
	{
		TMScene::LoadMsgText2(
			m_pQuestContentList[2],
			(char*)"UI\\QuestContents3.txt",
			20 * idwEvent,
			20 * (idwEvent + 1) - 1);
		return 0;
	}
	if (idwControlID == 1054269)
	{
		TMScene::LoadMsgText2(
			m_pQuestContentList[3],
			(char*)"UI\\QuestContents4.txt",
			20 * idwEvent,
			20 * (idwEvent + 1) - 1);
		return 0;
	}
	if (idwControlID == 5696)
	{
		if (m_nChatListSize == 3)
			m_nChatListSize = 3;
		else
			m_nChatListSize = 2;
				
		OnKeyPlus(43, 0);
		return 0;
	}
	if (idwControlID == 65677)
	{
		m_pChatGeneral->m_bSelected = m_pChatGeneral->m_bSelected == 0;
		m_pChatGeneral_C->m_bSelected = m_pChatGeneral->m_bSelected == 0;
		m_pChatGeneral->Update();
		m_pChatGeneral_C->Update();

		char szText[128]{};
		if (m_pChatGeneral->m_bSelected)
			sprintf(szText, "%s%s", g_pMessageStringTable[452], g_pMessageStringTable[446]);
		else
			sprintf(szText, "%s%s", g_pMessageStringTable[452], g_pMessageStringTable[447]);

		auto ipNewItem = new SListBoxItem(szText, 0xFFCCAAFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		auto pChatList = (SListBox*)m_pControlContainer->FindControl(65667);;
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		return 0;
	}
	if (idwControlID == 65785)
	{
		SetPK();
		return 0;
	}
	if (idwControlID == 5742)
	{
		if (m_pPartyPanel && m_pPartyBtn)
		{
			m_pPartyPanel->SetVisible(m_pPartyPanel->m_bVisible == 0);
			m_pPartyBtn->m_bSelected = m_pPartyBtn->m_bSelected == 0;
		}
		return 0;
	}
	if (idwControlID == 6068)
	{
		if (m_nCurrInterfacePanelIndex > 0)
		{
			auto pHelpPanel = m_pHelpInterfacePanel[m_nCurrInterfacePanelIndex--];
			pHelpPanel->SetVisible(0);
			m_pHelpInterfacePanel[m_nCurrInterfacePanelIndex]->SetVisible(1);
		}
		return 0;
	}
	if (idwControlID == 6069)
	{
		if (m_nCurrInterfacePanelIndex < 2)
		{
			auto pHelpPanel = m_pHelpInterfacePanel[m_nCurrInterfacePanelIndex++];
			pHelpPanel->SetVisible(0);
			m_pHelpInterfacePanel[m_nCurrInterfacePanelIndex]->SetVisible(1);
		}
		return 0;
	}
	if (idwControlID >= 897 && idwControlID <= 900)
	{
		m_pQuizBG->SetVisible(0);
		MSG_STANDARDPARM stParm{};
		stParm.Header.ID = g_pObjectManager->m_dwCharID;
		stParm.Header.Type = 0x2C7;
		stParm.Parm = idwControlID - 897;
		SendOneMessage((char*)&stParm, sizeof(stParm));
		return 1;
	}
	if (idwControlID >= 8706 && idwControlID <= 8806)
	{
		UpdateFireWorkButton(idwControlID - 8706);
		return 1;
	}
	if (idwControlID == 8810)
	{
		ClearFireWork();
		return 1;
	}
	if (idwControlID == 8807)
	{
		if (m_pFireWorkPanel)
		{
			m_pFireWorkPanel->SetVisible(0);
			m_nFireWorkCellX = -1;
			m_nFireWorkCellY = -1;
		}
		return 1;
	}
	if (idwControlID == 8809)
	{
		if (m_pFireWorkPanel)
		{
			m_pFireWorkPanel->SetVisible(0);
			UseFireWork();
		}
		return 1;
	}
	if (idwControlID >= 8811 && idwControlID <= 8815)
	{
		DrawCustomFireWork(idwControlID - 8811);
		return 1;
	}
	if (idwControlID == 8964)
	{
		TotoSelect();
		return 1;
	}
	if (idwControlID == 8966)
	{
		TotoClose();
		return 1;
	}
	if (idwControlID == 8978)
	{
		TotoBuy();
		return 1;
	}
	if (idwControlID == 12291)
	{
		m_dwLastSelServer = g_pTimerManager->GetServerTime();
		MSG_STANDARDPARM stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = 0x3AE;
		stParm.Parm = 0;
		SendOneMessage((char*)&stParm, sizeof(stParm));
		return 1;
	}
	if (idwControlID == 12289)
	{
		SListBoxServerItem* pItem = (SListBoxServerItem*)m_pServerList->GetItem(idwEvent);
		if (pItem->m_nCurrent < 500)
		{
			m_nServerMove = idwEvent + 1;
			m_dwLastTeleport = dwServerTime;
			m_cLastTeleport = 1;
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[25], 4000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
	if (idwControlID == 12545)
	{
		if (m_bAirmove_ShowUI == 1)
			return 1;
		if (m_stPotalItem.Header.ID)
			m_stPotalItem.ItemID = idwEvent + 1;
		else
			SetVisiblePotal(0, 0);
		return 1;
	}
	if (idwControlID == 12546)
		return OnControl_AirMoveStart(idwControlID, idwEvent);
	if (idwControlID == 12547)
	{
		if (m_bAirmove_ShowUI == 1)
			AirMove_ShowUI(0);

		memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
		SetVisiblePotal(0, 0);
		return 1;
	}
	if (idwControlID == 65881)
	{
		m_dwLastLogout = g_pTimerManager->GetServerTime();
		MSG_STANDARDPARM stParm{};
		stParm.Header.ID = m_pMyHuman->m_dwID;
		stParm.Header.Type = MSG_SysQuit_Opcode;
		stParm.Parm = 0;
		SendOneMessage((char*)&stParm, sizeof(stParm));
		return 1;
	}
	if (idwControlID == 65883)
	{
		m_pSystemPanel->SetVisible(0);
		return 1;
	}
	if (idwControlID == 641)
		return OnControl_PlayerMenu641(idwControlID, idwEvent);
	if (idwControlID == 643)
		return OnControl_PlayerMenu643(idwControlID, idwEvent);
	if (idwControlID == 642)
	{
		m_pBtnPGTGuild->SetVisible(0);
		m_pBtnPGTParty->SetVisible(0);
		m_pBtnPGTTrade->SetVisible(0);
		m_pBtnPGTChallenge->SetVisible(0);
		m_pBtnPGT1_V_1->SetVisible(0);
		m_pBtnPGT5_V_5->SetVisible(0);
		m_pBtnPGT10_V_10->SetVisible(0);
		m_pBtnPGTAll_V_All->SetVisible(0);
		m_pBtnPGTGuildDrop->SetVisible(1);
		m_pBtnPGTGuildWar->SetVisible(1);
		m_pBtnPGTGuildWar->SetVisible(0);
		m_pBtnPGTGuildAlly->SetVisible(1);

		if (m_pMyHuman && (m_pMyHuman->m_sGuildLevel >= 3 && m_pMyHuman->m_sGuildLevel <= 8 || m_pMyHuman->m_sGuildLevel == 9) && 
			(!m_pPGTOver->m_usGuild	|| m_pPGTOver->m_usGuild == m_pMyHuman->m_usGuild && (!m_pPGTOver->m_sGuildLevel || m_pPGTOver->m_sGuildLevel == 1)) && 
			m_pPGTOver->m_sGuildLevel != 9)
		{
			m_pBtnPGTGuildInvite->SetVisible(1);
		}
		m_pBtnPGTGICommon->SetVisible(0);
		m_pBtnPGTGIChief1->SetVisible(0);
		m_pBtnPGTGIChief2->SetVisible(0);
		m_pBtnPGTGIChief3->SetVisible(0);
		return 0;
	}
	if (idwControlID == 863)
	{
		m_pBtnPGTGuildDrop->SetVisible(0);
		m_pBtnPGTGuildWar->SetVisible(0);
		m_pBtnPGTGuildAlly->SetVisible(0);
		m_pBtnPGTGuildInvite->SetVisible(0);
		if (!m_pMyHuman)
			return 0;
		if (!m_pPGTOver)
			return 0;
		if (!m_pPGTOver->m_usGuild)
			m_pBtnPGTGICommon->SetVisible(0);

		m_pBtnPGTGICommon->SetVisible(1);
		if (m_pMyHuman->m_sGuildLevel == 9 && (m_pPGTOver->m_sGuildLevel < 3 || m_pPGTOver->m_sGuildLevel > 8) && 
			m_pMyHuman->m_usGuild == m_pPGTOver->m_usGuild)
		{
			m_pBtnPGTGIChief1->SetVisible(1);
			m_pBtnPGTGIChief2->SetVisible(1);
			m_pBtnPGTGIChief3->SetVisible(1);
		}
		return 0;
	}
	if (idwControlID == 912)
		return OnControl_PlayerMenu912(idwControlID, idwEvent);
	if (idwControlID >= 913 && idwControlID <= 915)
		return OnControl_PlayerMenu913(idwControlID, idwEvent);
	if (idwControlID == 816)
	{
		m_pPGTOver = 0;
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		m_pMessageBox->SetMessage(g_pMessageStringTable[36], 816u, 0);
		m_pMessageBox->m_dwArg = m_dwOpID;
		m_pMessageBox->SetVisible(1);
		return 1;
	}
	if (idwControlID == 817)
	{
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		if (m_pPGTOver)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[158], 817u, 0);
			m_pMessageBox->m_dwArg = m_pPGTOver->m_usGuild;
			m_pMessageBox->SetVisible(1);
			m_pPGTOver = 0;
		}
		return 1;
	}
	if (idwControlID == 818)
	{
		m_pPGTOver = 0;

		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}

		m_pMessageBox->SetMessage(g_pMessageStringTable[159], 818u, 0);
		m_pMessageBox->m_dwArg = m_dwOpID;
		m_pMessageBox->SetVisible(1);

		return 1;
	}
	if (idwControlID == 862)
	{
		if (m_pMessageBox->IsVisible())
		{
			m_pPGTPanel->SetVisible(0);
			return 0;
		}
		if (m_pPGTOver)
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[221], 862, 0);
			m_pMessageBox->m_dwArg = m_pPGTOver->m_usGuild;
			m_pMessageBox->SetVisible(1);
			m_pPGTOver = 0;
		}
		return 1;
	}
	if (idwControlID == 644)
	{
		m_pPGTOver = 0;
		m_pPGTPanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 668)
	{
		SetVisibleAutoTrade(0, 0);
		return 1;
	}
	if (idwControlID == 620)
	{
		if (m_pPGTPanel)
		{
			m_pBtnPGTParty->SetVisible(0);
			m_pBtnPGTGuild->SetVisible(0);
			m_pBtnPGTTrade->SetVisible(0);
			m_pBtnPGTChallenge->SetVisible(0);
			m_pBtnPGT1_V_1->SetVisible(1);
			m_pBtnPGT5_V_5->SetVisible(1);
			m_pBtnPGT10_V_10->SetVisible(1);
			m_pBtnPGTAll_V_All->SetVisible(1);
			m_pPGTPanel->SetVisible(1);
			m_pBtnPGTGICommon->SetVisible(0);
			m_pBtnPGTGIChief1->SetVisible(0);
			m_pBtnPGTGIChief2->SetVisible(0);
			m_pBtnPGTGIChief3->SetVisible(0);
		}
		return 0;
	}
	if (idwControlID == 639 || idwControlID == 621 || idwControlID == 622 || idwControlID == 623)
		return OnControl_PlayerMenu639(idwControlID, idwEvent);
	if (idwControlID == 1617)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "rock");
		SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 1618)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "paper");
		SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 1619)
	{
		MSG_MessageWhisper stMsgWhisper{};
		stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf(stMsgWhisper.MobName, "_RPS_");
		sprintf(stMsgWhisper.String, "scissor");
		SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
		m_pRPSGamePanel->SetVisible(0);
		return 0;
	}
	if (idwControlID == 4617)
	{
		if (!idwEvent)
			return TMFieldScene::OnMsgBoxEvent(4617, 0, dwServerTime);

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == 65716)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->ScoreBonus <= 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 0;
		SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		return 1;
	}
	if (idwControlID == 65719)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->ScoreBonus <= 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 1;
		SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		return 1;
	}
	if (idwControlID == 65722)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->ScoreBonus <= 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 2;
		SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		return 1;
	}
	if (idwControlID == 65725)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->ScoreBonus <= 0)
			return 0;

		MSG_ApplyBonus stApplyBonus{};
		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 0;
		stApplyBonus.Detail = 3;
		SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		return 1;
	}
	if (idwControlID == 65754)
		return OnControl_AddSpecial1(idwControlID, idwEvent, pMobData);
	if (idwControlID == 65757)
		return OnControl_AddSpecial2(idwControlID, idwEvent, pMobData);
	if (idwControlID == 65760)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->SpecialBonus <= 0)
			return 0;

		int totalSpecial = 0;
		if (m_pMyHuman->Is2stClass() == 2)
			totalSpecial = 200;
		else
			totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
		if (IsValidSkill(204) == 1)
		{
			totalSpecial = 320;
		}
		else if (IsValidSkill(39) == 1)
		{
			totalSpecial = 255;
		}
		else if (totalSpecial > 200)
		{
			totalSpecial = 200;
		}

		if (pMobData->CurrentScore.Special[2] < totalSpecial)
		{
			MSG_ApplyBonus stApplyBonus{};
			stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
			stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
			stApplyBonus.BonusType = 1;
			stApplyBonus.Detail = 2;
			SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
	if (idwControlID == 65763)
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->SpecialBonus <= 0)
			return 0;

		int totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
		if (m_pMyHuman->Is2stClass() == 2)
			totalSpecial = 200;
		if (IsValidSkill(208) == 1)
		{
			totalSpecial = 320;
		}
		else if (IsValidSkill(47) == 1)
		{
			totalSpecial = 255;
		}
		else if (totalSpecial > 200)
		{
			totalSpecial = 200;
		}

		if (pMobData->CurrentScore.Special[3] < totalSpecial)
		{
			MSG_ApplyBonus stApplyBonus{};
			stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
			stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
			stApplyBonus.BonusType = 1;
			stApplyBonus.Detail = 3;
			SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
	if (idwControlID == 475138)
	{
		auto pPartyList = m_pPartyList;
		auto pPartyItem = (SListBoxPartyItem*)pPartyList->GetItem(idwEvent);

		if (pPartyItem && pPartyItem->m_nState == 1 && !pPartyList->m_bRButton)
		{
			MSG_CNFParty2 stCNFParty2{};
			stCNFParty2.Header.ID = m_pMyHuman->m_dwID;
			stCNFParty2.Header.Type = MSG_CNFParty2_Opcode;
			stCNFParty2.LeaderID = pPartyItem->m_dwCharID;
			
			sprintf(stCNFParty2.LeaderName, pPartyItem->GetText());

			auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
			if (pHuman)
				pHuman->m_bParty = 1;

			SendOneMessage((char*)&stCNFParty2, sizeof(stCNFParty2));
		}
		else if (pPartyItem	&& g_pEventTranslator->m_bCtrl == 1	&& pPartyItem->m_bSelectEnable == 1	&& !pPartyList->m_bRButton)
		{
			int charId = pPartyItem->m_dwCharID;

			if (charId >= 0 && charId < 1000)
			{
				char szText[128]{};
				sprintf(szText, pPartyItem->GetText());
				m_pMessageBox->SetMessage(szText, 50001u, 0);
				m_pMessageBox->m_dwArg = pPartyItem->m_dwCharID;
				m_pMessageBox->SetVisible(1);
			}
		}
		else if (pPartyItem && pPartyList->m_bRButton == 1)
		{
			int skillId = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
			int delay = g_pSpell[skillId].Delay;
			if (m_nMySanc >= 9 && delay >= 2)
				--delay;
			if (m_pMyHuman->m_DilpunchJewel == 1)
				--delay;
			if (delay < 1)
				delay = 1;
			if (dwServerTime < m_dwSkillLastTime[skillId] + 1000 * delay)
				return 1;

			int Special = m_pMyHuman->m_stScore.Level;
			int classId = skillId - 24 * g_pObjectManager->m_stMobData.Class;
			if (skillId < 96)
				Special = g_pObjectManager->m_stMobData.CurrentScore.Special[classId / 8 + 1];

			if (BASE_GetManaSpent(skillId, g_pObjectManager->m_stMobData.SaveMana, Special) > g_pObjectManager->m_stMobData.CurrentScore.Mp)
			{
				auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
					0xFFFFAAAA,
					0.0,
					0.0,
					300.0f,
					16.0f,
					0,
					0x77777777,
					1,
					0);

				auto pChatList = m_pChatList;

				if (pChatList && ipNewItem)
					pChatList->AddItem(ipNewItem);

				GetSoundAndPlay(33, 0, 0);

				return 1;
			}

			if (dwServerTime > m_dwOldAttackTime + 1000	&& skillId == 42 && pPartyItem->m_dwCharID == m_pMyHuman->m_dwID)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[40], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			if (dwServerTime > m_dwOldAttackTime + 1000
				&& (skillId == 25
					|| skillId == 27
					|| skillId == 42
					|| skillId == 43
					|| skillId == 44
					|| skillId == 45
					|| skillId == 13))
			{
				if (skillId != 42)
				{
					auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);

					if (!pHuman)
						return 1;
					if (!pHuman->m_bParty)
						return 1;

					int x1 = (int)m_pMyHuman->m_vecPosition.x;
					int y1 = (int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						x1 = m_stMoveStop.NextX;
						y1 = m_stMoveStop.NextY;
					}

					int x2 = (int)pHuman->m_vecPosition.x;
					int y2 = (int)pHuman->m_vecPosition.y;

					int distance = BASE_GetDistance(x1, y1, x2, y2);
					int range = cktrans + g_pSpell[skillId].Range;

					int ty = y2;
					int tx = x2;

					BASE_GetHitPosition(x1, y1, &tx, &ty, (char*)m_HeightMapData, 8);
					if (distance > range || tx != x2 || ty != y2)
						return 1;

					int my_att = g_pAttribute[y1 / 4][x1 / 4];
					int other_att = g_pAttribute[y2 / 4][x2 / 4];
					if (!(my_att & 0x40))
					{
						if (other_att & 0x40)
							return 1;
					}
				}

				MSG_Attack stAttack{};
				stAttack.Header.Type = MSG_Attack_Multi_Opcode;
				stAttack.Header.ID = m_pMyHuman->m_dwID;
				stAttack.AttackerID = m_pMyHuman->m_dwID;
				stAttack.PosX = (int)m_pMyHuman->m_vecPosition.x;
				stAttack.PosY = (int)m_pMyHuman->m_vecPosition.y;
				stAttack.CurrentMp = -1;
				stAttack.SkillIndex = skillId;
				stAttack.SkillParm = 0;
				stAttack.Motion = -1;
				stAttack.Dam[0].TargetID = pPartyItem->m_dwCharID;
				stAttack.Dam[0].Damage = -1;
				stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
				stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					stAttack.PosX = m_stMoveStop.NextX;
					stAttack.TargetX = stAttack.PosX;
					stAttack.PosY = m_stMoveStop.NextY;
					stAttack.TargetY = stAttack.PosY;
				}
				SendOneMessage((char*)&stAttack, sizeof(stAttack));

				MSG_Attack localAttack{};
				memcpy(&localAttack, (char*)&stAttack, sizeof(localAttack));

				localAttack.Header.ID = m_dwID;
				localAttack.FlagLocal = 1;
				if (cktrans)
					localAttack.DoubleCritical |= 8u;

				OnPacketEvent(MSG_Attack_Multi_Opcode, (char*)&localAttack);
				m_dwOldAttackTime = dwServerTime;
				m_dwSkillLastTime[skillId] = dwServerTime;
			}
		}
		return 1;
	}
	if (idwControlID == 475139)
	{
		if (!m_pChatListPanel->IsVisible())
		{
			MSG_STANDARDPARM stParm{};
			stParm.Header.Type = 0x37E;
			stParm.Header.ID = m_pMyHuman->m_dwID;
			stParm.Parm = 0;

			SendOneMessage((char*)&stParm, sizeof(stParm));

			m_pPartyPanel->SetVisible(0);
		}
		return 1;
	}
	if (idwControlID == 617)
	{
		if (m_dwLastCheckTime + 2000 <= g_pApp->m_pTimerManager->GetServerTime())
		{
			auto pButton = (SButton*)m_pControlContainer->FindControl(617u);
			pButton->m_bSelected = pButton->m_bSelected == 0;
			g_pObjectManager->m_stTrade.MyCheck = pButton->m_bSelected;

			MSG_Trade stTrade{};

			memcpy(&stTrade, &g_pObjectManager->m_stTrade, sizeof(stTrade));
			stTrade.Header.Type = 0x383;
			SendOneMessage((char*)&stTrade, sizeof(stTrade));

			m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[41], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
		return 1;
	}
	if (idwControlID == 667)
	{
		int validItem = 1;
		for (int i = 0; i < 10; ++i)
		{
			if (m_stAutoTrade.Item[i].sIndex > 0)
				validItem = 0;
		}
		if (validItem == 1)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[145], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 1;
		}

		auto pButtonRun = (SButton*)m_pControlContainer->FindControl(667u);
		m_stAutoTrade.Header.Type = 919;
		m_stAutoTrade.TargetID = m_pMyHuman->m_dwID;
		SendOneMessage((char*)&m_stAutoTrade, sizeof(m_stAutoTrade));

		auto pButtonCancel = (SButton*)m_pControlContainer->FindControl(668u);
		if (pButtonRun)
			pButtonRun->SetVisible(0);
		if (pButtonCancel)
			pButtonCancel->SetVisible(1);

		auto pAutoTrade =  m_pAutoTrade;
		if (pAutoTrade && pAutoTrade->IsVisible() == 1)
		{
			m_pCargoPanel->SetVisible(0);
			m_pCargoPanel1->SetVisible(0);

			pAutoTrade->SetRealPos((float)g_pDevice->m_dwScreenWidth - pAutoTrade->m_nWidth, 0.0f);
		}

		return 1;
	}

	return 0;
}
