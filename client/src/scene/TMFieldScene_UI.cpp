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
	{
		switch (idwControlID)
		{
		case TMB_HELLSTORE_SELECT_1:
			if (m_pHellStoreDesc)
				TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 21, 40);
			break;
		case TMB_HELLSTORE_SELECT_2:
			if (m_pHellStoreDesc)
				TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 41, 60);
			break;
		case TMB_HELLSTORE_SELECT_3:
			if (m_pHellStoreDesc)
				TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 61, 80);
			break;
		case TMB_HELLSTORE_SELECT_4:
			if (m_pHellStoreDesc)
				TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 81, 100);
			break;
		}

		m_nHellStoreValue = idwControlID;
		auto pHellgateStore = m_pHellgateStore;
		SButton* pHellSelect = nullptr;
		for (int i = 0; i < 4; ++i)
		{
			pHellSelect = (SButton*)m_pControlContainer->FindControl(i + TMB_HELLSTORE_SELECT_1);
			if (pHellSelect)
			{
				if (i + 6193 == idwControlID)
					pHellSelect->SetSelected(pHellgateStore->m_bVisible);
				else
					pHellSelect->SetSelected(0);
			}
		}
		return 1;
	}
	if (idwControlID >= TMB_GAMBLE_LEFTX2 && idwControlID <= TMB_GAMBLE_RIGHT)
	{
		switch (idwControlID)
		{
		case TMB_GAMBLE_LEFTX2:
			m_nBet -= 10000;
			if (m_nBet < 1000)
				m_nBet = 1000;
			break;
		case TMB_GAMBLE_LEFT:
			m_nBet -= 1000;
			if (m_nBet < 1000)
				m_nBet = 1000;
			break;
		case TMB_GAMBLE_RIGHTX2:
			if (g_pObjectManager->m_stMobData.Coin - m_nBet < 10000)
				return 1;
			m_nBet += 10000;
			if (m_nBet > 100000)
				m_nBet = 100000;
			break;
		case TMB_GAMBLE_RIGHT:
			if (g_pObjectManager->m_stMobData.Coin - m_nBet < 1000)
				return 1;
			m_nBet += 1000;
			if (m_nBet > 100000)
				m_nBet = 100000;
			break;
		}

		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_GAMBLE_BETCOUNT));
		
		char szText[128]{};
		sprintf(szText, "%6d", m_nBet);
		pText->SetText(szText, 0);

		sprintf(szText, "%10d", g_pObjectManager->m_stMobData.Coin - m_nBet);

		m_pMoney3->m_cComma = 1;
		m_pMoney3->SetText(szText, 0);
		return 1;
	}
	if (idwControlID == TMB_GAMBLE_START)
	{
		if (m_pReelPanel2->m_dwStopTime || m_pReelPanel2->m_bRoling == 1)
			return 1;
		if (m_pReelPanel->m_dwStopTime || m_pReelPanel->m_bRoling == 1)
			return 1;

		auto pGridInv = m_pGridInv;
		auto vec = pGridInv->CanAddItemInEmpty(1, 1);
		if (vec.x < 0 || vec.y < 0)
		{
			auto pChatList = m_pChatList;

			auto ipNewItem = new SListBoxItem(g_pMessageStringTable[1],
				0xFFFFAAAA,
				0.0,
				0.0,
				300.0f,
				16.0f,
				0,
				0x77777777,
				1u,
				0);

			if (ipNewItem)
				pChatList->AddItem(ipNewItem);

			GetSoundAndPlay(33, 0, 0);
			return 1;
		}

		if (pMobData->Coin - m_nBet >= 0)
		{
			MSG_STANDARDPARM2 stParm2{};
			stParm2.Header.ID = g_pObjectManager->m_dwCharID;
			stParm2.Header.Type = MSG_DoJackpotBet_Opcode;
			stParm2.Parm1 = 1;
			stParm2.Parm2 = m_nBet;
			SendOneMessage((char*)&stParm2, sizeof(stParm2));

			m_pReelPanel->m_dwBatCoin = m_nBet;
			m_pReelPanel2->m_dwBatCoin = m_nBet;

			m_pReelPanel->SetRoll(1, 0, 0, 0, 3000);
			m_pReelPanel2->SetRoll(1, 0, 0, 0, 3000);
			pMobData->Coin -= m_nBet;

			GetSoundAndPlay(338, 0, 0);

			UpdateScoreUI(0);
			return 1;
		}
		return 1;
	}
	if (idwControlID == TMB_GAMBLE_QUIT)
	{
		SetVisibleGamble(0, 0);
		return 0;
	}
	if (idwControlID == E_CHAT && !idwEvent)
	{
		auto pEditChat = m_pEditChat;
		if (!strlen(pEditChat->GetText()))
		{
			if (!g_nKeyType)
			{
				m_pEditChatPanel->SetVisible(0);
				m_pChatPanel->SetVisible(1);
				m_pControlContainer->SetFocusedControl(0);
			}
			return 1;
		}

		unsigned int dwLastChatTime = m_dwLastChatTime[3];
		m_dwLastChatTime[3] = m_dwLastChatTime[2];
		m_dwLastChatTime[2] = m_dwLastChatTime[1];
		m_dwLastChatTime[1] = m_dwLastChatTime[0];
		m_dwLastChatTime[0] = dwServerTime;

		char istrText[128]{};

		if (dwServerTime - dwLastChatTime < 4000)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[33], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}

		if (m_szLastChatList[0][0])
		{
			if (strcmp(m_szLastChatList[0], pEditChat->GetText()))
			{
				for (int j = 4; j > 0; --j)
					memcpy(m_szLastChatList[j], m_szLastChatList[j - 1], 128);

				sprintf(m_szLastChatList[0], "%s", pEditChat->GetText());
			}
		}
		else
		{
			for (int k = 0; k < 5; ++k)
			{
				sprintf(m_szLastChatList[k], "%s", pEditChat->GetText());
			}
		}

		m_sChatIndex = 0;

		if (!strcmp(pEditChat->GetText(), g_pMessageStringTable[191]))
		{
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "/help"))
		{
			OnKeyHelp(104, 0);
			pEditChat->SetText((char*)"");
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "effects"))
		{
			if (g_bHideEffect)
			{
				g_bHideEffect = 0;
				g_bHideSkillBuffEffect = 0;
				g_bHideSkillBuffEffect2 = 0;
			}
			else
			{
				g_bHideEffect = 1;
				g_bHideSkillBuffEffect = 1;
				g_bHideSkillBuffEffect2 = 1;
			}

			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "fps"))
		{
			g_pDevice->m_bDrawFPS = g_pDevice->m_bDrawFPS == 0;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			UpdateScoreUI(0);
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "effect"))
		{
			g_pDevice->m_bShowEffects = g_pDevice->m_bShowEffects == 0;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "effectold"))
		{
			g_bHideSkillBuffEffect = !g_bHideSkillBuffEffect;
			pEditChat->SetText((char*)"");
			m_pControlContainer->SetFocusedControl(0);
			return 1;
		}
		else if (!strcmp(pEditChat->GetText(), "exp"))
		{
			m_bShowExp = m_bShowExp == 0;
			char str[128]{};
			if (m_bShowExp)
				sprintf(str, "Show Exp : OFF");
			else
				sprintf(str, "Show Exp : ON");

			SysMsgChat(str);
			return 1;
		}

		unsigned int idwFontColor = 0xFFFFAAAA;

		char Chat[128]{};
		sprintf(Chat, "%s", pEditChat->GetText());
		if (!BASE_CheckChatValid(Chat))
			pEditChat->SetText(g_pMessageStringTable[1521]);

		auto pPartyList = m_pPartyList;
		auto pChatList = m_pChatList;
		int idx = 0;
		switch (Chat[0])
		{
		case '-':
		{
			idwFontColor = 0xFFAAFFFF;
			idx = 1;
			int nStartIndex = 1;
			if (Chat[1] == '-')
			{
				idwFontColor = 0xFF00FFFF;
				nStartIndex = 2;
				idx = 2;
			}

			InsertInChatList(pChatList, pMobData, pEditChat, idwFontColor, 3, nStartIndex);
		}
		break;
		case '=':
		{
			if (pPartyList->m_nNumItem <= 0)
			{
				pEditChat->SetText((char*)"");
				return 0;
			}
			idx = 2;
			idwFontColor = 0xFFFF99FF;

			InsertInChatList(pChatList, pMobData, pEditChat, idwFontColor, 1, 1);
		}
		break;
		case '@':
		{
			idwFontColor = 0xFFAAFFFF;

			int nStartIndex = 1;
			if (Chat[1] == '@')
			{
				idwFontColor = 0xF0F60AFF;
				idx = 3;
				nStartIndex = 2;
			}
			else
			{
				idwFontColor = 0xFF00AAFF;
				idx = 2;
				nStartIndex = 1;
			}

			InsertInChatList(pChatList, pMobData, pEditChat, idwFontColor, 3, nStartIndex);

		}
		break;
		case '/':
		{
			idx = 3;
			char str1[128]{};
			sscanf(Chat, "/%s", str1);

			if (!strcmp(str1, "relo") || !strcmp(str1, g_pMessageStringTable[234]) || !strcmp(str1, "Relocate") || !strcmp(str1, "relocate"))
			{
				if (!m_pAutoTrade->IsVisible())
				{
					sscanf(Chat, "/%s %s", str1, m_szSummoner2);

					m_cLastRelo = 2;
					m_dwLastRelo = g_pTimerManager->GetServerTime();

					pEditChat->SetText((char*)"");

					m_pControlContainer->SetFocusedControl(0);
					m_pEditChatPanel->SetVisible(0);
					m_pChatPanel->SetVisible(0);
				}
				return 1;
			}

			idwFontColor = 0xFFFFFF00;

			MSG_MessageWhisper stMsgWhisper{};
			stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;

			sprintf(stMsgWhisper.MobName, "%s", str1);
			strcpy(m_cWhisperName, str1);

			if (strlen(str1) >= 16)
			{
				str1[15] = 0;
				str1[14] = 0;
			}
			if (m_szWhisperList[0][0])
			{
				if (strcmp(m_szWhisperList[0], str1))
				{
					for (int l = 4; l > 0; --l)
						memcpy(m_szWhisperList[l], m_szWhisperList[l - 1], 16);

					sprintf(m_szWhisperList[0], "%s", str1);
				}
			}
			else
			{
				for (int m = 0; m < 5; ++m)
					sprintf(m_szWhisperList[m], "%s", str1);
			}

			m_sWhisperIndex = 0;
			auto pChatText = pEditChat->GetText();
			char* str = &pChatText[strlen(str1) + 1];
			if (str[0])
			{
				sprintf(stMsgWhisper.String, "%s", &pChatText[strlen(str1) + 2]);
			}

			BASE_TransCurse(stMsgWhisper.String);

			if (!strcmp(str1, "summonguild"))
			{
				if (m_pMyHuman->m_fProgressRate < 0.89999998f && m_pMyHuman->m_fProgressRate > 0.0f)
					return 1;

				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, "king") || !strcmp(str1, "kingdom") || !strcmp(str1, "King") || !strcmp(str1, "Kingdom"))
			{
				if (m_pMyHuman->m_fProgressRate < 0.89999998f && m_pMyHuman->m_fProgressRate > 0.0f)
					return 1;

				m_stLastWhisper = stMsgWhisper;
				m_cLastWhisper = 1;
				m_dwLastWhisper = g_pTimerManager->GetServerTime();
			}
			else if (!strcmp(str1, g_pMessageStringTable[389]))
			{
				sprintf(stMsgWhisper.MobName, "%s", "spk");
				strcpy(m_cWhisperName, g_pMessageStringTable[389]);
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, g_pMessageStringTable[386]))
			{
				if (m_pMyHuman->m_sGuildLevel)
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
				if (!CheckGuildName(stMsgWhisper.String, 0))
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
				sprintf(stMsgWhisper.MobName, "%s", "create");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, g_pMessageStringTable[387]))
			{
				if (m_pMyHuman->m_sGuildLevel != 9 && (m_pMyHuman->m_sGuildLevel < 3 || m_pMyHuman->m_sGuildLevel > 8))
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[373], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
				sprintf(stMsgWhisper.MobName, "%s", "handover");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, g_pMessageStringTable[391]))
			{
				sprintf(stMsgWhisper.MobName, "%s", "getout");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, g_pMessageStringTable[390]))
			{
				sprintf(stMsgWhisper.MobName, "%s", "war");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, "srv"))
			{
				return 1;
			}
			else if (!strcmp(str1, g_pMessageStringTable[496]))
			{
				sprintf(stMsgWhisper.MobName, "%s", "item_lock");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, g_pMessageStringTable[497]))
			{
				sprintf(stMsgWhisper.MobName, "%s", "item_unlock");
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else if (!strcmp(str1, "tab"))
			{
				if (m_pMyHuman->m_fProgressRate < 0.89999998f && m_pMyHuman->m_fProgressRate > 0.0f)
					return 1;

				if (!strcmp(m_pMyHuman->m_szNickName, stMsgWhisper.String))
					return 1;

				sprintf(g_TempNick, "%s", stMsgWhisper.String);

				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));
			}
			else
				SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));

			if (!strcmp(str1, "r") || !(strcmp(str1, "re")))
			{
				sprintf(str1, g_pMessageStringTable[61]);
				sprintf(istrText, "[%s] [%s]> %s", g_pObjectManager->m_stMobData.MobName, str1, stMsgWhisper.String);
			}
			else
			{
				sprintf(istrText, "[%s] : %s> %s", g_pObjectManager->m_stMobData.MobName, str1, stMsgWhisper.String);
			}

			int len = strlen(istrText) + strlen(pMobData->MobName);
			const size_t maxLen = 55;
			if (len <= maxLen)
			{
				auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);
			}
			else
			{
				char dest[128]{};
				char dest2[128]{};
				if (IsClearString(stMsgWhisper.String, maxLen - 1))
				{
					strncpy(dest, stMsgWhisper.String, maxLen);
					sprintf(dest2, "%s", &stMsgWhisper.String[maxLen]);
				}
				else
				{
					strncpy(dest, stMsgWhisper.String, maxLen - 1);
					sprintf(dest2, "%s", &stMsgWhisper.MobName[maxLen + 15]);
				}

				auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);

				auto ipNewItem2 = new SListBoxItem(dest2, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (strlen(istrText) > maxLen && ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);
			}
		}
		break;
		default:
		{
			idx = 0;
			MSG_MessageChat stMsgChat{};
			stMsgChat.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgChat.Header.Type = MSG_MessageChat_Opcode;

			sprintf(stMsgChat.String, "%s", pEditChat->GetText());

			BASE_TransCurse(stMsgChat.String);

			pEditChat->SetText((char*)"");

			SendOneMessage((char*)&stMsgChat, sizeof(stMsgChat));

			size_t len = strlen(stMsgChat.String) + strlen(pMobData->MobName);
			const size_t maxLen = 40;
			if (len <= maxLen)
			{
				sprintf(istrText, "[%s]> %s", pMobData->MobName, stMsgChat.String);

				auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);
			}
			else
			{
				char dest[128]{};
				char dest2[128]{};
				if (IsClearString(stMsgChat.String, maxLen - 1))
				{
					strncpy(dest, stMsgChat.String, maxLen);
					dest[maxLen] = 0;
					dest[maxLen + 1] = 0;
					sprintf(dest2, "%s", &stMsgChat.String[maxLen]);
				}
				else
				{
					strncpy(dest, stMsgChat.String, maxLen - 1);
					dest2[maxLen - 1] = 0;
					sprintf(dest2, "%s", &stMsgChat.String[maxLen - 1]);
				}

				sprintf(istrText, "[%s]> %s", g_pObjectManager->m_stMobData.MobName, dest);

				auto ipNewItem = new SListBoxItem(istrText, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);

				auto ipNewItem2 = new SListBoxItem(dest2, idwFontColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
				if (strlen(stMsgChat.String) > maxLen && ipNewItem && pChatList)
					pChatList->AddItem(ipNewItem);
			}

			sprintf(istrText, "%s", stMsgChat.String);
			if (stMsgChat.String[0] == '*')
			{
				m_pMyHuman->m_dwChatDelayTime = 10000;
				sprintf(istrText, "%s", &stMsgChat.String[1]);
			}
			else
			{
				m_pMyHuman->m_dwChatDelayTime = 3000;
			}
		}
		break;
		}

		m_dwChatTime = g_pTimerManager->GetServerTime();
		if (strncmp(Chat, "+set ", 4) && !idx)
			m_pMyHuman->SetChatMessage(istrText);
		return 0;
	}
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
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;

		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(T_INPUT_GOLD));
		auto pEdit = m_pControlContainer->FindControl(E_INPUT_GOLD);

		if (m_pCargoPanel1->IsVisible() == 1)
		{
			m_nCoinMsgType = 0;
			pText->SetText(g_pMessageStringTable[136], 0);
			m_pControlContainer->SetFocusedControl(pEdit);
			m_pInputGoldPanel->SetVisible(1);
		}
		if (m_pCargoPanel->IsVisible() == 1)
		{
			m_nCoinMsgType = 0;
			pText->SetText(g_pMessageStringTable[136], 0);
			m_pControlContainer->SetFocusedControl(pEdit);
			m_pInputGoldPanel->SetVisible(1);
		}
		else if (m_pTradePanel->IsVisible() == 1)
		{
			if (g_pObjectManager->m_stTrade.TradeMoney > 0)
				return 1;

			m_nCoinMsgType = 1;
			pText->SetText(g_pMessageStringTable[137], 0);
			m_pControlContainer->SetFocusedControl(pEdit);
			m_pInputGoldPanel->SetVisible(1);
		}
		else
		{
			m_pInputGoldPanel->SetVisible(0);
		}

		m_pChatSelectPanel->SetVisible(0);
		return 0;
	}
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
	{
		auto pInputText = static_cast<SEditableText*>(m_pControlContainer->FindControl(E_INPUT_GOLD));

		char* inputText = pInputText->GetText();

		int inputTextLen = strlen(inputText);

		if (strcmp(inputText, "all"))
		{
			bool bFind = false;

			for (int n = 0; n < inputTextLen; ++n)
			{
				if (inputText[n] < '0' || inputText[n] > '9')
				{
					bFind = true;
					break;
				}
			}

			for (int n = 0; n < inputTextLen; ++n)
			{
				if (inputText[n] == '%')
					inputText[n] = '!';
			}

			long long nInputValue = _atoi64(pInputText->GetText());

			if (strlen(pInputText->GetText()) <= 0)
			{
				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}

			if (m_nCoinMsgType != 12 &&
				m_nCoinMsgType != 11 &&
				m_nCoinMsgType != 8 &&
				m_nCoinMsgType != 3 &&
				m_nCoinMsgType != 6)
			{
				if (bFind == true || nInputValue < 0 || nInputValue > g_pObjectManager->m_stMobData.Coin && !m_nCoinMsgType)
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[34], 1000);
					m_pMessagePanel->SetVisible(1, 1);
					m_pControlContainer->SetFocusedControl(pInputText);
					m_pChatSelectPanel->SetVisible(0);
					return 1;
				}
			}

			if (bFind == true && m_nCoinMsgType == 12)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[409], 1000);
				m_pMessagePanel->SetVisible(1, 1);
				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}

			if (m_nCoinMsgType == 4 && nInputValue > 1999999999)
			{
				char istrMessage[128]{};

				sprintf_s(istrMessage, g_pMessageStringTable[143], 2000000000);

				m_pMessagePanel->SetMessage(istrMessage, 1000);
				m_pMessagePanel->SetVisible(1, 1);

				m_pControlContainer->SetFocusedControl(pInputText);
				return 1;
			}

			switch (m_nCoinMsgType)
			{
			case 2:
			{
				MSG_STANDARDPARM stWithdraw{};

				stWithdraw.Header.Type = MSG_Withdraw_Opcode;
				stWithdraw.Header.ID = m_pMyHuman->m_dwID;
				stWithdraw.Parm = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stWithdraw, sizeof(stWithdraw));
			}
			break;
			case 0:
			{
				MSG_STANDARDPARM stDeposit{};

				stDeposit.Header.Type = MSG_Deposit_Opcode;
				stDeposit.Header.ID = m_pMyHuman->m_dwID;
				stDeposit.Parm = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stDeposit, sizeof(stDeposit));
			}
			break;
			case 1:
			{
				auto pMyCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(617u));
				auto pOpCheckButton = static_cast<SButton*>(m_pControlContainer->FindControl(601u));

				pMyCheckButton->m_bSelected = 0;
				pOpCheckButton->m_bSelected = 0;

				m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();

				g_pObjectManager->m_stTrade.MyCheck = pMyCheckButton->m_bSelected;
				g_pObjectManager->m_stTrade.TradeMoney = static_cast<int>(nInputValue);
				g_pObjectManager->m_stTrade.Header.Type = MSG_Trade_Opcode;

				SendOneMessage((char*)&g_pObjectManager->m_stTrade, sizeof(g_pObjectManager->m_stTrade));

				auto pMyGold = static_cast<SText*>(m_pControlContainer->FindControl(619u));

				char szText[11]{};

				sprintf_s(szText, "%10lld", nInputValue);

				pMyGold->SetText(szText, 0);
			}
			break;
			case 3:
			{
				auto pATradeTitle = (SText*)m_pControlContainer->FindControl(TMT_ATRADE_TITLE);
				auto pATradeName = (SText*)m_pControlContainer->FindControl(TMT_ATRADE_ID);

				if (pATradeTitle)
				{
					pATradeTitle->SetText(pInputText->GetText(), 0);
				}

				pATradeName->SetText(m_pMyHuman->m_szName, 1);

				m_stAutoTrade.TargetID = m_pMyHuman->m_dwID;

				sprintf_s(m_stAutoTrade.Desc, "%s", pInputText->GetText());

				if (m_pInputBG2)
					m_pInputBG2->SetVisible(0);

				pInputText->m_nMaxStringLen = 10;

				for (int i = 0; i < 10; ++i)
				{
					SGridControlItem* pAutoTradeItem = m_pGridAutoTrade[i]->PickupAtItem(0, 0);

					auto pCargoItem = m_pCargoGrid->GetAtItem(m_stAutoTrade.CarryPos[i] % 5, m_stAutoTrade.CarryPos[i] / 5);

					if (pCargoItem)
						pCargoItem->m_GCObj.dwColor = -1;

					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pAutoTradeItem)
						g_pCursor->m_pAttachedItem = nullptr;

					if (pAutoTradeItem)
					{
						delete pAutoTradeItem;

						pAutoTradeItem = nullptr;
					}
				}
				SetVisibleAutoTrade(1, 1);
			}
			break;
			case 11:
			{
				sprintf_s(m_stPass, "%s", pInputText->GetText());

				m_pitemPassGrid->SellItem2();

				memset(m_stPass, 0, sizeof(m_stPass));
			}
			break;
			case 4:
			{
				if (m_nLastAutoTradePos >= 0)
				{
					for (int j = 0; j < 10; ++j)
					{
						SGridControl* pParent = m_pGridAutoTrade[j];
						SGridControlItem* pAutoTradeItem = pParent->GetAtItem(0, 0);

						SGridControl* pCargoGrid = m_pCargoGridList[m_nLastAutoTradePos / 40];
						SGridControlItem* pCargoItem = pCargoGrid->GetAtItem(m_nLastAutoTradePos % 40 % 5, m_nLastAutoTradePos % 40 / 5);

						if (!pAutoTradeItem)
						{
							if (!pCargoItem)
								return 1;

							auto pItem = new STRUCT_ITEM;

							memcpy(pItem, pCargoItem->m_pItem, sizeof(STRUCT_ITEM));

							pParent->AddItem(new SGridControlItem(pParent, pItem, 0.0f, 0.0f), 0, 0);

							pCargoItem->m_GCObj.dwColor = 0xFFFF0000;

							m_stAutoTrade.CarryPos[j] = m_nLastAutoTradePos;
							m_stAutoTrade.TradeMoney[j] = static_cast<int>(nInputValue);

							memcpy(&m_stAutoTrade.Item[j], pItem, sizeof(STRUCT_ITEM));

							pParent->m_nTradeMoney = static_cast<int>(nInputValue);
							break;
						}

						if (pAutoTradeItem && j == 11)
						{
							m_pMessagePanel->SetMessage(g_pMessageStringTable[1], 2000);
							m_pMessagePanel->SetVisible(1, 1);

							pCargoItem->m_GCObj.dwColor = 0xFFFFFFFF;
						}
					}
					m_nLastAutoTradePos = -1;
				}
			}
			break;
			case 6:
				SendCapsuleItem();
				break;
			case 7:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = 977;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stPacket, sizeof(stPacket));
			}
			break;
			case 8:
			{
				char str2[128]{};

				sprintf_s(str2, "%s %s", m_pPGTOver->m_szName, inputText);

				MSG_MessageWhisper stMessageWhisper{};

				stMessageWhisper.Header.ID = g_pObjectManager->m_dwCharID;
				stMessageWhisper.Header.Type = MSG_MessageWhisper_Opcode;
				sprintf_s(stMessageWhisper.MobName, "subcreate");
				sprintf_s(stMessageWhisper.String, "%s", str2);

				if (strlen(str2) >= 16)
				{
					str2[15] = 0;
					str2[14] = 0;
				}

				if (!CheckGuildName(stMessageWhisper.String, 1))
				{
					m_pMessagePanel->SetMessage(g_pMessageStringTable[370], 2000);
					m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
				if (m_szWhisperList[0][0])
				{
					if (strcmp((char*)m_szWhisperList, str2) != 0)
					{
						for (int i = 4; i > 0; --i)
							memcpy(m_szWhisperList[i], m_szWhisperList[i - 1], 16);

						sprintf_s(m_szWhisperList[0], "%s", str2);
					}
				}
				else
				{
					for (int i = 0; i < 5; ++i)
						sprintf_s(m_szWhisperList[i], "%s", str2);
				}
				m_sWhisperIndex = 0;

				char* text = pInputText->GetText();

				if (text[strlen(str2) + 1])
				{
					sprintf(stMessageWhisper.String, "%s", &text[strlen(str2) + 2]);
				}

				BASE_TransCurse(stMessageWhisper.String);
				pInputText->SetText((char*)"");
				SendOneMessage((char*)&stMessageWhisper, sizeof(stMessageWhisper));
			}
			break;
			case 9:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = 0xED7;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stPacket, sizeof(stPacket));
			}
			break;
			case 10:
			{
				MSG_STANDARDPARM stPacket{};

				stPacket.Header.Type = 0xED8;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stPacket, sizeof(stPacket));
			}
			break;
			case 12:
			{
				int nItemAmount = BASE_GetItemAmount(SGridControl::m_pSellItem->m_pItem);

				if (nInputValue >= nItemAmount)
				{
					m_pControlContainer->SetFocusedControl(nullptr);
					m_pInputGoldPanel->SetVisible(0);
					pInputText->SetText((char*)"");
					return 0;
				}

				if (m_pGridInv->CheckType(
					SGridControl::m_pSellItem->m_pGridControl->m_eItemType,
					SGridControl::m_pSellItem->m_pGridControl->m_eGridType) != 1)
				{
					return 1;
				}

				int pos = SGridControl::m_pSellItem->m_nCellIndexX + 5 * SGridControl::m_pSellItem->m_nCellIndexY;
				int page = 15 * (SGridControl::m_pSellItem->m_pGridControl->m_dwControlID - 67072);

				if (page < 0 || page > 45)
					page = 0;

				pos += page;

				MSG_STANDARDPARM3 stPacket{};

				stPacket.Header.Type = MSG_SplitItem_Opcode;
				stPacket.Header.ID = m_pMyHuman->m_dwID;
				stPacket.Parm1 = pos;
				stPacket.Parm2 = SGridControl::m_pSellItem->m_pItem->sIndex;
				stPacket.Parm3 = static_cast<int>(nInputValue);
				SendOneMessage((char*)&stPacket, sizeof(stPacket));
			}
			break;
			}

			m_pControlContainer->SetFocusedControl(0);
			m_pInputGoldPanel->SetVisible(0);

			pInputText->SetText((char*)"");

			if (g_nKeyType == 1)
				m_pControlContainer->SetFocusedControl(m_pEditChat);
		}
		else
		{
			char szText[11]{};

			if (!m_nCoinMsgType || m_nCoinMsgType == 1 || m_nCoinMsgType == 7)
			{
				sprintf_s(szText, "%d", g_pObjectManager->m_stMobData.Coin);

				pInputText->SetText(szText);
			}
			else if (m_nCoinMsgType == 2)
			{
				sprintf_s(szText, "%d", g_pObjectManager->m_nCargoCoin);

				pInputText->SetText(szText);
			}
		}

		return 1;
	}
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
	{
		auto pBtn = static_cast<SButton*>(m_pControlContainer->FindControl(idwControlID));

		switch (idwControlID)
		{
		case B_CHAT_SELECT_NOMAL:
			strcpy(m_cChatType, "");
			break;
		case B_CHAT_SELECT_WHISPER:
			sprintf(m_cChatType, "/%s ", m_cWhisperName);
			break;
		case B_CHAT_SELECT_PARTY:
			strcpy(m_cChatType, "=");
			break;
		case B_CHAT_SELECT_GUILD:
			strcpy(m_cChatType, "-");
			break;
		case B_CHAT_SELECT_GUILD2:
			strcpy(m_cChatType, "--");
			break;
		case B_CHAT_SELECT_CITY:
			strcpy(m_cChatType, "@@");
			break;
		case B_CHAT_SELECT_KINGDOM:
			strcpy(m_cChatType, "@");
			break;
		case B_CHAT_SELECT_SHOUT:
			char src[128]{};
			sprintf(src, "/%s ", g_pMessageStringTable[389]);
			strcpy(m_cChatType, src);
			break;
		}

		strcpy(m_cChatSelect, pBtn->m_GCPanel.strString);

		auto pChatSelectBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAT_SELECT));

		pChatSelectBtn->SetText(pBtn->m_GCPanel.strString);

		m_pChatListPanel->SetVisible(0);

		m_pEditChat->SetText(m_cChatType);

		return 0;
	}
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
	{
		int posX = 0;
		int posY = 0;

		if (++m_AutoPostionUse >= 3)
			m_AutoPostionUse = 0;

		if (m_AutoPostionUse == 0)
		{
			m_pSetType->SetTextureSetIndex(463);
			m_pSetType->m_pAltText->SetText(g_UIString[232], 0);
		}
		else if (m_AutoPostionUse == 1)
		{
			m_pSetType->SetTextureSetIndex(464);

			posX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
			posY = static_cast<int>(m_pMyHuman->m_vecPosition.y);

			m_pSetType->m_pAltText->SetText(g_UIString[233], 0);
		}
		else if (m_AutoPostionUse == 2)
		{
			m_pSetType->SetTextureSetIndex(465);
			m_pSetType->m_pAltText->SetText((char*)"", 0);
		}

		m_AutoStartPointX = posX;
		m_AutoStartPointY = posY;

		return 0;
	}
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
	{
		if (m_pQuestList[0])
		{
			m_pQuestList[0]->SetVisible(1);
			m_pQuestList[0]->m_bSelectEnable = 1;
		}
		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(1);
		if (m_pQuestList[1])
		{
			m_pQuestList[1]->SetVisible(0);
			m_pQuestList[1]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(0);
		if (m_pQuestList[2])
		{
			m_pQuestList[2]->SetVisible(0);
			m_pQuestList[2]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(0);
		if (m_pQuestList[3])
		{
			m_pQuestList[3]->SetVisible(0);
			m_pQuestList[3]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(0);

		m_pQuestMemo->SetVisible(0);

		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON2)
	{
		if (m_pQuestList[0])
		{
			m_pQuestList[0]->SetVisible(0);
			m_pQuestList[0]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(0);
		if (m_pQuestList[1])
		{
			m_pQuestList[1]->SetVisible(1);
			m_pQuestList[1]->m_bSelectEnable = 1;
		}
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(1);
		if (m_pQuestList[2])
		{
			m_pQuestList[2]->SetVisible(0);
			m_pQuestList[2]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(0);
		if (m_pQuestList[3])
		{
			m_pQuestList[3]->SetVisible(0);
			m_pQuestList[3]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(0);

		m_pQuestMemo->SetVisible(0);
		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON3)
	{
		if (m_pQuestList[0])
		{
			m_pQuestList[0]->SetVisible(0);
			m_pQuestList[0]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(0);
		if (m_pQuestList[1])
		{
			m_pQuestList[1]->SetVisible(0);
			m_pQuestList[1]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(0);
		if (m_pQuestList[2])
		{
			m_pQuestList[2]->SetVisible(1);
			m_pQuestList[2]->m_bSelectEnable = 1;
		}
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(1);
		if (m_pQuestList[3])
		{
			m_pQuestList[3]->SetVisible(0);
			m_pQuestList[3]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(0);

		m_pQuestMemo->SetVisible(0);
		return 1;
	}
	if (idwControlID == B_QUEST_BUTTON4)
	{
		if (m_pQuestList[0])
		{
			m_pQuestList[0]->SetVisible(0);
			m_pQuestList[0]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(0);
		if (m_pQuestList[1])
		{
			m_pQuestList[1]->SetVisible(0);
			m_pQuestList[1]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(0);
		if (m_pQuestList[2])
		{
			m_pQuestList[2]->SetVisible(0);
			m_pQuestList[2]->m_bSelectEnable = 0;
		}
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(0);
		if (m_pQuestList[3])
		{
			m_pQuestList[3]->SetVisible(1);
			m_pQuestList[3]->m_bSelectEnable = 1;
		}
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(1);
		m_pQuestMemo->SetVisible(0);
		return 1;
	}
	if (idwControlID == B_QUEST_MEMO)
	{
		if (m_pQuestPanel)
		{
			m_pQuestPanel->SetVisible(1);
			m_pQuestMemo->SetVisible(0);

			TMScene::LoadMsgText3(
				m_pQuestList[0],
				(char*)"UI\\QuestSubjects.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[1],
				(char*)"UI\\QuestSubjects2.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[2],
				(char*)"UI\\QuestSubjects3.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[3],
				(char*)"UI\\QuestSubjects4.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);

			switch (m_pLevelQuest[pMobData->CurrentScore.Level])
			{
			case 97:
				OnControlEvent(1054259, 0);
				break;
			case 98:
				OnControlEvent(1054262, 0);
				break;
			case 99:
				OnControlEvent(1054265, 0);
				break;
			case 101:
				OnControlEvent(1054268, 0);
				break;
			}		
		}	
		return 1;
	}
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
	{
		if (++g_GameAuto >= 4)
			g_GameAuto = 0;
		if (m_pMyHuman)
			m_pMyHuman->_dwAttackDelay = 0;
		if (g_GameAuto)
		{
			m_pSGameAutoBtn->SetVisible(1);
			m_pSetType->SetVisible(1);
		}
		else
		{
			m_pSGameAutoBtn->SetVisible(0);
			m_pSetType->SetVisible(0);
		}

		switch (g_GameAuto)
		{
		case 0:
			m_pMGameAutoBtn->SetTextureSetIndex(458);
			m_pMGameAutoBtn->m_pAltText->SetText(
				g_UIString[229],
				0);
			break;
		case 1:
			m_pMGameAutoBtn->SetTextureSetIndex(455);
			m_pMGameAutoBtn->m_pAltText->SetText(
				g_UIString[226],
				0);
			break;
		case 2:
			m_pMGameAutoBtn->SetTextureSetIndex(456);
			m_pMGameAutoBtn->m_pAltText->SetText(
				g_UIString[227],
				0);
			break;
		case 3:
			m_pMGameAutoBtn->SetTextureSetIndex(459);
			m_pMGameAutoBtn->m_pAltText->SetText(
				g_UIString[230],
				0);
			break;
		}
		return 1;
	}
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
	{
		if (++m_AutoPostionUse >= 3)
			m_AutoPostionUse = 0;

		int nAutoPostionUse = m_AutoPostionUse;

		int startX = 0;
		int startY = 0;
		if (!nAutoPostionUse)
		{
			m_pSetType->SetTextureSetIndex(463);
			m_pSetType->m_pAltText->SetText(g_UIString[233], 0);
		}
		else if (nAutoPostionUse == 1)
		{
			m_pSetType->SetTextureSetIndex(464);
			startX = (int)m_pMyHuman->m_vecPosition.x;
			startY = (int)m_pMyHuman->m_vecPosition.y;
			m_pSetType->m_pAltText->SetText(g_UIString[234], 0);
		}
		else if (nAutoPostionUse == 2)
		{
			m_pSetType->SetTextureSetIndex(465);
			m_pSetType->m_pAltText->SetText(g_UIString[232], 0);
		}

		m_AutoStartPointX = startX;
		m_AutoStartPointY = startY;
		return 1;
	}
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
	{
		int nIsVisible = m_pQuestPanel->IsVisible();
		m_pQuestPanel->SetVisible(nIsVisible == 0);
		m_pQuestBtn->SetSelected(nIsVisible == 0);

		if (!nIsVisible)
		{
			TMScene::LoadMsgText3(
				m_pQuestList[0],
				(char*)"UI\\QuestSubjects.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[1],
				(char*)"UI\\QuestSubjects2.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[2],
				(char*)"UI\\QuestSubjects3.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			TMScene::LoadMsgText3(
				m_pQuestList[3],
				(char*)"UI\\QuestSubjects4.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
		}

		// Open the mortal quests list as default
		OnControlEvent(B_QUEST_BUTTON, 0);
		GetSoundAndPlay(51, 0, 0);
		return 0;
	}
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
	{
		if (m_bAirmove_ShowUI == 1)
		{
			auto pItem = m_pPotalList->GetSelectedIndex();
			
			if (pItem + 1 != 0 && !m_bAirMove)
				AirMove_Start(pItem);

			AirMove_ShowUI(0);
			return 1;
		}

		if (!m_stPotalItem.ItemID)
		{
			m_stPotalItem.ItemID = m_pPotalList->GetSelectedIndex() + 1;
		}
		if (m_stPotalItem.Header.ID)
		{
			int nSourPage = m_stPotalItem.SourPos / 15;
			int nSourRes = m_stPotalItem.SourPos % 15 % 5;
			int nSourDiv = m_stPotalItem.SourPos % 15 / 5;
			auto pGridInvList = m_pGridInvList[nSourPage];
			auto pItem = pGridInvList->GetItem(nSourRes, nSourDiv);

			if (pItem)
			{
				int amount = BASE_GetItemAmount(pItem->m_pItem);
				if (amount > 1)
				{
					BASE_SetItemAmount(pItem->m_pItem, amount - 1);
					sprintf(pItem->m_GCText.strString, "%2d", amount - 1);
					pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
				}
				else
				{
					auto pPickItem = pGridInvList->PickupItem(nSourRes, nSourDiv);

					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickItem)
						g_pCursor->m_pAttachedItem = nullptr;
					
					SAFE_DELETE(pPickItem);
				}

				if (amount <= 1)
				{
					if (!m_stPotalItem.SourType)
					{
						memset(&g_pObjectManager->m_stMobData.Equip[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
					}
					else if (m_stPotalItem.SourType == 1)
					{
						memset(&g_pObjectManager->m_stMobData.Carry[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
					}
					else if (m_stPotalItem.SourType == 2)
					{
						memset(&g_pObjectManager->m_stItemCargo[m_stPotalItem.SourPos], 0, sizeof(STRUCT_ITEM));
					}
				}

				m_dwGetItemTime = g_pTimerManager->GetServerTime();
				m_dwLastTeleport = m_dwGetItemTime;
				m_cLastTeleport = 1;
				
				auto vec = m_pMyHuman->m_vecPosition;

				memset(&m_stUseItem, 0, sizeof(m_stUseItem));
				m_stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				m_stUseItem.Header.Type = m_stPotalItem.Header.Type;
				m_stUseItem.SourType = m_stPotalItem.SourType;
				m_stUseItem.SourPos = m_stPotalItem.SourPos;
				m_stUseItem.ItemID = m_stPotalItem.ItemID;
				m_stUseItem.GridX = (int)vec.x;
				m_stUseItem.GridY = (int)vec.y;

				memset((char*)&m_stPotalItem, 0, 0x24u);

				MSG_STANDARDPARM stParm{};
				stParm.Header.ID = m_pMyHuman->m_dwID;
				stParm.Header.Type = MSG_SysQuit_Opcode;
				stParm.Parm = 1;
				SendOneMessage((char*)&stParm, sizeof(stParm));
			}
		}

		SetVisiblePotal(0, 0);
		return 1;
	}
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
	{
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(m_dwOpID);
		if (!pHuman)
		{
			m_pPGTOver = 0;
			return 1;
		}
		else if (pHuman->m_bParty)
		{
			m_pPGTOver = 0;
			return 1;
		}
		else if (m_pMyHuman->m_cDie == 1)
		{
			m_pPGTOver = 0;
			return 1;
		}

		if (!m_pPartyList->m_pItemList[0] || static_cast<SListBoxPartyItem*>(m_pPartyList->m_pItemList[0])->m_nState != 1)
		{
			MSG_REQParty stReqParty{};
			stReqParty.Header.Type = MSG_REQParty_Opcode;
			stReqParty.Header.ID = m_pMyHuman->m_dwID;
			stReqParty.Leader.Class = m_pMyHuman->m_nSkinMeshType - 1;
			stReqParty.Leader.PartyIndex = 0;
			stReqParty.Leader.Level = m_pMyHuman->m_stScore.Level;
			stReqParty.Leader.Hp = m_pMyHuman->m_stScore.Hp;
			stReqParty.Leader.MaxHp = m_pMyHuman->m_stScore.MaxHp;
			stReqParty.Leader.ID = m_pMyHuman->m_dwID;
			sprintf(stReqParty.Leader.Name, "%s", m_pMyHuman->m_szName);
			stReqParty.TargetID = m_dwOpID;
			SendOneMessage((char*)&stReqParty, sizeof(stReqParty));
			m_dwOpID = 0;
			m_pPGTPanel->SetVisible(0);
			m_pPGTOver = 0;
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[156], 2000);
		m_pPGTOver = 0;
		return 1;
	}
	if (idwControlID == 643)
	{
		if (!m_pPGTOver	|| 
			(float)BASE_GetDistance(
				(int)m_pMyHuman->m_vecPosition.x,
				(int)m_pMyHuman->m_vecPosition.y,
				(int)m_pPGTOver->m_vecPosition.x,
				(int)m_pPGTOver->m_vecPosition.y) > ((float)(g_pObjectManager->m_pCamera->m_fMaxCamLen - 11.0) + 6.0))
		{
			return 1;
		}

		m_pPGTOver = 0;
		if (m_pMyHuman->m_cDie == 1)
			return 1;

		RECT rc; 
		
		rc.left = 2601;
		rc.top = 1702;
		rc.right = 2652;
		rc.bottom = 1750;

		POINT pt;
		pt.x = (int)m_pMyHuman->m_vecPosition.x;
		pt.y = (int)m_pMyHuman->m_vecPosition.y;
		if (PtInRect(&rc, pt) == 1)
			return 1;

		auto pTradePanel = m_pTradePanel;
		if (!g_pObjectManager->m_stTrade.OpponentID || !pTradePanel || pTradePanel->IsVisible() != 1)
		{
			g_pObjectManager->m_stTrade.Header.Type = 0x383;
			g_pObjectManager->m_stTrade.Header.ID = m_pMyHuman->m_dwID;
			g_pObjectManager->m_stTrade.OpponentID = m_dwOpID;
			SendOneMessage((char*)&g_pObjectManager->m_stTrade, 156);

			m_dwOpID = 0;
			m_pPGTPanel->SetVisible(0);
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[35], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return 1;
	}
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
	{
		if (!m_pPGTOver)
			return 0;

		if (!g_pCurrentScene)
			return 0;
		if (m_pPGTOver->m_usGuild)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[364], 2000);
			m_pMessagePanel->SetVisible(1, 1);
			return 0;
		}

		if (g_pObjectManager->m_stMobData.Coin >= 1000000)
		{
			MSG_STANDARDPARM2 stParm2{};
			stParm2.Header.Type = MSG_InviteGuild_Opcode;
			stParm2.Header.ID = m_pMyHuman->m_dwID;
			stParm2.Parm1 = m_pPGTOver->m_dwID;
			
			SendOneMessage((char*)&stParm2, sizeof(stParm2));
			m_pPGTPanel->SetVisible(0);
			m_pPGTOver = 0;
			return 0;
		}

		m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
		m_pMessagePanel->SetVisible(1, 1);
		return 0;
	}
	if (idwControlID >= 913 && idwControlID <= 915)
	{
		if (!g_pCurrentScene)
			return 0;

		if (m_pPGTOver != nullptr)
		{
			if (!m_pPGTOver->m_usGuild)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[367], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (m_pPGTOver->m_usGuild != m_pMyHuman->m_usGuild)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[365], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (m_pPGTOver->m_sGuildLevel >= 3 && m_pPGTOver->m_sGuildLevel <= 8)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[366], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
			if (g_pObjectManager->m_stMobData.Coin < 50000000)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[155], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 0;
			}
		}

		VisibleInputGuildName();
		m_pPGTPanel->SetVisible(0);
		return 0;
	}
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
	{
		if (m_pPGTOver)
		{
			MSG_STANDARDPARM2 stParm2{};
			stParm2.Header.Type = 0x39F;
			stParm2.Header.ID = m_pMyHuman->m_dwID;;
			stParm2.Parm1 = m_pPGTOver->m_dwID;

			switch (idwControlID)
			{
			case 0x27Fu:
				stParm2.Parm2 = 0;
				break;
			case 0x26Du:
				stParm2.Parm2 = 1;
				break;
			case 0x26Eu:
				stParm2.Parm2 = 2;
				break;
			case 0x26Fu:
				stParm2.Parm2 = 3;
				break;
			}

			SendOneMessage((char*)&stParm2, sizeof(stParm2));

			m_pPGTOver = 0;
			if (m_pPGTPanel)
				m_pPGTPanel->SetVisible(0);
		}
		return 0;
	}
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
	{
		if (m_pMyHuman->m_cDie == 1)
			return 1;
		if (pMobData->SpecialBonus <= 0)
			return 0;

		int totalSpecial = 0;
		if (m_pMyHuman->Is2stClass() != 2)
		{
			totalSpecial = 3 * (pMobData->CurrentScore.Level + 1) / 2;
		}
		else if (!pMobData->Class && IsValidSkill(205) == 1)
		{
			totalSpecial = 280;
		}
		else if (pMobData->Class != 2 || IsValidSkill(233) != 1)
		{
			totalSpecial = 200;
		}
		else
		{
			totalSpecial = 230;
		}

		if (pMobData->CurrentScore.Special[0] < totalSpecial)
		{
			MSG_ApplyBonus stApplyBonus{};
			stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
			stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
			stApplyBonus.BonusType = 1;
			stApplyBonus.Detail = 0;
			SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
	if (idwControlID == 65757)
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
		if (pMobData->Class == 3 && IsValidSkill(238) == 1)
		{
			totalSpecial = 400;
		}
		else if (IsValidSkill(200) == 1)
		{
			totalSpecial = 320;
		}
		else if (IsValidSkill(31) == 1)
		{
			totalSpecial = 255;
		}
		else if (totalSpecial > 200)
		{
			totalSpecial = 200;
		}

		if (pMobData->CurrentScore.Special[1] < totalSpecial)
		{
			MSG_ApplyBonus stApplyBonus{};
			stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
			stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
			stApplyBonus.BonusType = 1;
			stApplyBonus.Detail = 1;
			SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[39], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		return 1;
	}
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

/// Atualiza os textos e barras de HUD (HP, MP, EXP, atributos, buffs e status do personagem).
void TMFieldScene::UpdateScoreUI(unsigned int unFlag)
{
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (g_pObjectManager->m_stMobData.Equip[13].sIndex == 769)
	{
		int sanc = BASE_GetItemSanc(&pMobData->Equip[13]);
		int hpprogress = 0;

		if (sanc > 6)
			hpprogress = 545;
		else if (sanc > 3)
			hpprogress = 546;
		else if (sanc > 0)
			hpprogress = 547;
		else
			hpprogress = 538;
		m_pHPBar->m_GCProgress.nTextureSetIndex = hpprogress;
		m_pHPBar->Update();
	}
	else
	{
		m_pHPBar->m_GCProgress.nTextureSetIndex = 538;
		m_pHPBar->Update();
	}

	if (pMobData->CurrentScore.Hp > pMobData->CurrentScore.MaxHp)
		pMobData->CurrentScore.Hp = pMobData->CurrentScore.MaxHp;

	if (m_pHPBar)
		m_pHPBar->SetMaxProgress(pMobData->CurrentScore.MaxHp);

	if (m_pHPBar)
		m_pHPBar->SetCurrentProgress(pMobData->CurrentScore.Hp);

	char szStr[128]{};
	sprintf(szStr, "%d", pMobData->CurrentScore.Hp);
	if (m_pCHP)
		m_pCHP->SetText(szStr, 0);

	// HP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.Hp);
	if (m_pCIHP)
		m_pCIHP->SetText(szStr, 0);
	if (m_pMPBar)
		m_pMPBar->SetMaxProgress(pMobData->CurrentScore.MaxMp);
	if (m_pMPBar)
		m_pMPBar->SetCurrentProgress(pMobData->CurrentScore.Mp);

	// MP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.Mp);
	if (m_pCMP)
		m_pCMP->SetText(szStr, 0);
	if (m_pCIMP)
		m_pCIMP->SetText(szStr, 0);

	if (m_pMainCharName)
		m_pMainCharName->SetText(pMobData->MobName, 0);

	if (!(unFlag & 0x10))
	{
		sprintf(szStr, "%I64d", pMobData->Exp);
		if (m_pCIEXP)
		{
			m_pCIEXP->m_cComma = 1;
			m_pCIEXP->SetText(szStr, 0);
		}

		long long nLevelCount = 0;
		int ckind = m_pMyHuman->Is2stClass();
		int CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			nLevelCount = (g_pNextLevel_G2[CurLevel + 1] + g_pNextLevel_G2[CurLevel]) / 10;
		else
			nLevelCount = (g_pNextLevel[CurLevel + 1] + g_pNextLevel[CurLevel]) / 10;

		nLevelCount = static_cast<int>((((float)g_pObjectManager->m_nFakeExp * 100.0f) / (float)nLevelCount));
		sprintf(szStr, g_pMessageStringTable[304], g_pObjectManager->m_nFakeExp, nLevelCount);
		if (m_pGridCharFace)
		{
			m_pGridCharFace->m_GCPanel.nTextureSetIndex = 541;
			m_pGridCharFace->m_GCPanel.nTextureIndex = ckind + 3 * pMobData->Class;
		}
		if (m_pCIFakeExp)
		{
			m_pCIFakeExp->SetText(szStr, 0);
			if (nLevelCount < 80)
				m_pCIFakeExp->SetTextColor(0xFFFFFFFF);
			else
				m_pCIFakeExp->SetTextColor(0xFFFF0000);
		}
		if (m_pExpHold)
		{
			if (g_pObjectManager->m_nFakeExp <= 0)
				m_pExpHold->SetVisible(0);
			else
				m_pExpHold->SetVisible(1);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Level + 1);
		m_Level = pMobData->CurrentScore.Level + 1;
		if (m_pCLevel)
			m_pCLevel->SetText(szStr, 0);
		if (m_pMainInfo2_Lv)
			m_pMainInfo2_Lv->SetText(szStr, 0);

		ckind = m_pMyHuman->Is2stClass();
		CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			sprintf(szStr, "%I64d", g_pNextLevel_G2[CurLevel + 1]);
		else
			sprintf(szStr, "%I64d", g_pNextLevel[CurLevel + 1]);

		char strEXP[256]{};
		if (m_pCIEXPE)
		{
			m_pCIEXPE->m_cComma = 1;
			m_pCIEXPE->SetText(szStr, 0);
		}

		sprintf(szStr, "%d", pMobData->ScoreBonus);
		if (m_pScBonus)
			m_pScBonus->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->SpecialBonus);
		if (m_pSpBonus)
			m_pSpBonus->SetText(szStr, 0);
		if (m_pSkBonus)
			m_pSkBonus->SetText(szStr, 0);

		char szMoeny[64]{};
		char szRMoeny[64]{};
		sprintf(szMoeny, "%10d", pMobData->Coin);
		sprintf(szRMoeny, "%10d", g_pObjectManager->m_RMBCount);
		m_Coin = pMobData->Coin;
		m_RMB = g_pObjectManager->m_RMBCount;
		if (m_pMoney1)
		{
			m_pMoney1->m_cComma = 1;
			m_pMoney1->SetText(szMoeny, 0);
		}
		if (m_pMoney4)
		{
			m_pMoney4->m_cComma = 1;
			m_pMoney4->SetText(szRMoeny, 0);
		}
		if (m_pMoney2)
		{
			m_pMoney2->m_cComma = 1;
			m_pMoney2->SetText(szMoeny, 0);
		}

		sprintf(szMoeny, "%10d", pMobData->Coin - m_nBet);
		if (m_pMoney3)
		{
			m_pMoney3->m_cComma = 1;
			m_pMoney3->SetText(szMoeny, 0);
		}

		char szNameTemp[128]{};
		sprintf(szNameTemp, "[%s]:%d", pMobData->MobName, strlen(pMobData->MobName));
		if (m_pCIName)
			m_pCIName->SetText(szNameTemp, 1);

		static const char* szClass[4] = {
			g_pMessageStringTable[121],
			g_pMessageStringTable[122],
			g_pMessageStringTable[123],
			g_pMessageStringTable[124]
		};
			
		char szValue[256]{};
		sprintf(szValue, "%d", m_pMyHuman->m_nTotalKill);
		m_pMyHuman->m_pKillLabel->SetText(szValue, 0);
		m_pCIGuild->SetText(szValue, 0);
		m_pCIGuild->SetTextColor(0xFFAAAAAA);
		if (pMobData->Equip[0].sIndex >= 40)
		{
			m_pCIClass->SetText((char*)"Monster", 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}
		else if (pMobData->Equip[0].sIndex % 10 >= 6)
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)szClass[pMobData->Equip[0].sIndex / 10], 0);
		}
		else
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Str);
		if (m_pCIStr)
			m_pCIStr->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Int);
		if (m_pCIInt)
			m_pCIInt->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Dex);
		if (m_pCIDex)
			m_pCIDex->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Con);
		if (m_pCICon)
			m_pCICon->SetText(szStr, 0);

		int nMaxLevel = 0;
		if (m_pMyHuman->Is2stClass() == 2)
		{
			nMaxLevel = 200;
		}
		else
		{
			nMaxLevel = 3 * (pMobData->CurrentScore.Level + 1) / 2;
			if (nMaxLevel > 200)
				nMaxLevel = 200;
		}

		int nMax3 = nMaxLevel;
		int nMax2 = nMaxLevel;
		int nMax1 = nMaxLevel;
		if (!pMobData->Class && IsValidSkill(205) == 1)
			nMaxLevel = 280;
		else if (pMobData->Class == 2 && IsValidSkill(233) == 1)
			nMaxLevel = 230;
		if (pMobData->Class == 3 && IsValidSkill(238) == 1)
			nMax1 = 400;
		else if (IsValidSkill(200) == 1)
			nMax1 = 320;
		else if (IsValidSkill(31) == 1)
			nMax1 = 255;
		if (IsValidSkill(204) == 1)
			nMax2 = 320;
		else if (IsValidSkill(39) == 1)
			nMax2 = 255;
		if (IsValidSkill(208) == 1)
			nMax3 = 320;
		else if (IsValidSkill(47) == 1)
			nMax3 = 255;

		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[0], nMaxLevel);
		if (m_pCISpecial1)
			m_pCISpecial1->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[1], nMax1);
		if (m_pCISpecial2)
			m_pCISpecial2->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[2], nMax2);
		if (m_pCISpecial3)
			m_pCISpecial3->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[3], nMax3);
		if (m_pCISpecial4)
			m_pCISpecial4->SetText(szStr, 0);

		long long nMax = 0;
		long long nCur = 0;
		if (ckind == 2)
		{
			nMax = (g_pNextLevel_G2[CurLevel + 1] - g_pNextLevel_G2[CurLevel]);
			nCur = pMobData->Exp - g_pNextLevel_G2[CurLevel];
		}
		else
		{
			nMax = g_pNextLevel[CurLevel + 1] -g_pNextLevel[CurLevel];
			nCur = pMobData->Exp - g_pNextLevel[CurLevel];
		}

		int nLamp = static_cast<int>(nMax / 4);
		int nGrid = 4;

		if (nCur >= nMax / 4)
		{
			if (nCur < static_cast<long long>(2 * static_cast<long long>(nLamp)))
			{
				nGrid = 2;
			}
			else
			{				
				if (nCur < static_cast<long long>(3 * static_cast<long long>(nLamp)))
					nGrid = 3;
			}
		}
		else
		{
			nGrid = 1;
		}

		for (int l = 0; l < 3; ++l)
		{
			if (m_pExpLamp[l])
				m_pExpLamp[l]->m_GCPanel.nTextureIndex = l < nGrid - 1;
		}

		nCur -= (static_cast<long long>(nGrid) - 1) * static_cast<long long>(nLamp);
		nMax = static_cast<long long>((double)nLamp * 0.1);

		for (int n = 0; n < 10; ++n)
		{
			if (nCur > nMax)
			{
				nCur -= nMax;
				if (m_pExpProgress[n])
				{
					m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
					m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nMax));
				}
			}
			else if (m_pExpProgress[n])
			{
				m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
				m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nCur));
				nCur = 0;
			}
		}

		int face = pMobData->Equip[0].sIndex;
		int cls = 0;
		if (face < 40)
		{
			if (face == 32)
			{
				cls = 2;
			}
			else
			{
				int mod = face % 10;
				int div = face / 10;
				if (face % 10 < 6 || mod > 9)
					cls = div;
				else
					cls = mod - 6;
			}
			pMobData->Class = cls;
		}

		if (!(unFlag & 1))
		{
			int sIndex = 0;
			int ItemIndex = 0;
			// Skill list on "S"
			for (int i = 0; i < 24; ++i)
			{
				unsigned int dwBit = 1 << i;

				if (!m_pSkillSecGrid[i])
					continue;

				if ((dwBit & pMobData->LearnedSkill[0]) != dwBit)
				{
					m_pSkillSecGrid[i]->Empty();
					continue;
				}

				auto pSkillItem = m_pSkillSecGrid[i]->GetItem(0, 0);
				sIndex = 24 * pMobData->Class + i + 5000;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;

					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}
				}

				m_pSkillSecGrid[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if(ipCtrlItem)
					m_pSkillSecGrid[i]->AddItemInEmpty(ipCtrlItem);
			}
			// Skill bar 2
			for (int i = 0; i < 12; ++i)
			{
				if (!m_pSkillSecGrid2)
					continue;

				if (((1 << i) & pMobData->LearnedSkill[1]) != 1 << i)
				{
					m_pSkillSecGrid2[i]->Empty();
					continue;
				}

				SGridControlItem* pSkillItem = m_pSkillSecGrid2[i]->GetItem(0, 0);
				sIndex = 12 * pMobData->Class + i + 5400;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = nullptr;
						continue;
					}
				}

				m_pSkillSecGrid2[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if (ipCtrlItem)
					m_pSkillSecGrid2[i]->AddItemInEmpty(ipCtrlItem);
			}
			// Skill bar 1
			for (int i = 0; i < 8; ++i)
			{
				unsigned int dwBit = 1 << (i + 24);
				if (!m_pGridSkillBelt)
					continue;

				auto pSkillItem = m_pGridSkillBelt->GetItem(i % 4, i / 4);
				sIndex = i + 5096;
				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}				
				}

				auto pOldItem = m_pGridSkillBelt->PickupItem(i % 4, i / 4);
				if ((dwBit & pMobData->LearnedSkill[0]) == dwBit)
				{
					auto pItem = new STRUCT_ITEM;
					memset(pItem, 0, sizeof(STRUCT_ITEM));
					pItem->sIndex = i + 5096;

					m_pGridSkillBelt->AddItem(new SGridControlItem(0, pItem, 0.0f, 0.0f), i % 4, i / 4);
				}
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pOldItem)
					g_pCursor->m_pAttachedItem = 0;

				//TODO: check this strange bOld
				bool bOld = false;
				if (bOld != 1 && pOldItem)
				{
					SAFE_DELETE(pOldItem);
				}
			}
		}
		if (m_pSLPanel1 && !pMobData->Class)
			m_pSLPanel1->SetVisible(0);
		else if (m_pSLPanel1)
		{
			m_pSLPanel1->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel2 && !pMobData->Class)
			m_pSLPanel2->SetVisible(0);
		else if (m_pSLPanel2)
		{
			m_pSLPanel2->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel3 && !pMobData->Class)
			m_pSLPanel3->SetVisible(0);
		else if (m_pSLPanel3)
		{
			m_pSLPanel3->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSkillCover && !pMobData->Class)
			m_pSkillCover->SetVisible( 0);
		else if (m_pSkillCover)
		{
			m_pSkillCover->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSkillCover->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSkillCover->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSkillCover->m_GCPanel.nTextureIndex = 2;
		}

		switch (pMobData->Class)
		{
		case 0:
			m_pCISp1Caption->SetText(g_pMessageStringTable[242], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[243], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[244], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[245], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pCISp1Caption->SetText(g_pMessageStringTable[246], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[247], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[248], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[249], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pCISp1Caption->SetText(g_pMessageStringTable[250], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[251], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[252], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[253], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pCISp1Caption->SetText(g_pMessageStringTable[254], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[255], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[256], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[257], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		int nRate = pMobData->CurrentScore.Special[0];
		int nDamageValue = pMobData->CurrentScore.Damage;
		if (nRate > 100)
			nRate = 100;

		sprintf(szStr, "%d", nDamageValue);
		if (m_pDamage)
			m_pDamage->SetText(szStr, 0);

		int cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
		if (cSkillIndex >= 0 && cSkillIndex < 248)
		{
			SetMyHumanMagic();
			int nIdx = g_pObjectManager->m_stMobData.Equip[6].sIndex;
			int nWeather = g_nWeather;
			if (g_nWeather == 3)
				nWeather = 2;

			if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
				nWeather = 2;

			int nSkillDam = BASE_GetSkillDamage(cSkillIndex, pMobData, nWeather, GetWeaponDamage(),
				g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);
			if (nSkillDam >= 0)
			{
				sprintf(szStr, "%d", nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
			else
			{
				sprintf(szStr, "%d", -nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
		}
		else
		{
			sprintf(szStr, "%d", 0);
			if (m_pSkillDam)
				m_pSkillDam->SetText(szStr, 0);
		}

		if (!m_pMyHuman->m_cOnlyMove)
			m_pMyHuman->SetSpeed(m_bMountDead);

		int nSpeedValue = (int)m_pMyHuman->m_fMaxSpeed;

		sprintf(szStr, "%d", nSpeedValue);
		if (m_pSpeed)
			m_pSpeed->SetText(szStr, 0);

		int nACValue = pMobData->CurrentScore.Ac;
		sprintf(szStr, "%d", nACValue);
		if (m_pDefence)
			m_pDefence->SetText(szStr, 0);

		int nDef = BASE_GetMobAbility(pMobData, 53) + (BASE_GetMobAbility(pMobData, 3) + pMobData->BaseScore.Ac);
		sprintf(szStr, "%d", nDef);
		int nSpeedClass = 0;

		if (m_pMyHuman->m_nClass == 26 && !m_pMyHuman->m_stLookInfo.FaceMesh)
		{
			nSpeedClass = 10;
			if (pMobData->LearnedSkill[0] & 0x20000)
				nSpeedClass = 20;
		}
		else if (m_pMyHuman->m_nClass == 26 && m_pMyHuman->m_stLookInfo.FaceMesh == 1)
			nSpeedClass = 0;
		else if (m_pMyHuman->m_nClass == 33)
		{
			nSpeedClass = 20;
			if (pMobData->LearnedSkill[0] & 0x200000)
				nSpeedClass = 40;
		}
		else if(m_pMyHuman->m_nClass == 40)
			nSpeedClass = 20;
		else if (m_pMyHuman->m_nClass == 63)
			nSpeedClass = 30;
		
		int nAttSpeedValue = (pMobData->CurrentScore.Special[2] / 10 + 10)
			* (m_pMyHuman->m_cSpeedUp - m_pMyHuman->m_cSpeedDown)
			+ pMobData->CurrentScore.Dex / 5
			+ nSpeedClass
			+ BASE_GetMobAbility(pMobData, 26)
			+ 100;

		if (m_pMyHuman->m_cFreeze == 1)
			nAttSpeedValue -= 30;

		sprintf(szStr, "%d%%", nAttSpeedValue);
		if (m_pAttackSpeed)
			m_pAttackSpeed->SetText(szStr, 0);
		int nCriticalValue = pMobData->Critical;
		nCriticalValue *= 4;

		sprintf(szStr, "%d.%d%%", nCriticalValue / 10, nCriticalValue % 10);
		if (m_pCritical)
			m_pCritical->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[2]);
		if (m_pRegist1)
			m_pRegist1->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[3]);
		if (m_pRegist2)
			m_pRegist2->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[0]);
		if (m_pRegist3)
			m_pRegist3->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[1]);
		if (m_pRegist4)
			m_pRegist4->SetText(szStr, 0);

		for (int i = 0; i < 32; ++i)
		{
			int nType = (signed int)m_pMyHuman->m_usAffect[i] >> 8;
			if (m_pAffectL[i])
			{
				if (nType >= 0 && nType < 50)
					m_pAffectL[i]->SetText(g_pAffectTable[nType], 0);
				else
					m_pAffectL[i]->SetText((char*)" ", 0);
			}
		}

		char szVal[32]{};

		for (int i = 0; i < 32; ++i)
		{
			if (m_pMyHuman->m_stAffect[i].Type > 0)
			{
				if (m_pMyHuman->m_stAffect[i].Time < 1000000)
				{
					sprintf(szVal, "%5d", m_pMyHuman->m_stAffect[i].Time);					
				}
				else
				{
					int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
					int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
					int Day = m_pMyHuman->m_stAffect[i].Time % 10000;
					int nYearDay = 365;
					if (!(Year % 4))
						nYearDay = 366;
					int nDafaultDay = 0;
					switch (add)
					{
					case 4:
						nDafaultDay = 7;
						break;
					case 5:
						nDafaultDay = 15;
						break;
					case 6:
						nDafaultDay = 30;
						break;
					}

					if (add)
					{
						int freeDay = nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - Day);
						sprintf(szVal, g_pMessageStringTable[291], freeDay);
					}
				}
				if (m_pAffectL[i] && !i)
				{
					int sTime = m_pMyHuman->m_stAffect[0].Time;
					if (sTime >= 7375)
					{
						m_pAffect[i]->SetTextColor(0xFFFFFFFF);
					}
					else
					{
						for (int nJ = 0; nJ < 7; ++nJ)
						{
							if (sTime < g_sTimeTable[nJ])
							{
								m_pAffect[i]->SetTextColor(g_dwFoodColor[nJ]);
								break;
							}
						}
					}
				}
				if (m_pMiniPanel)
				{
					if (m_pMyHuman->m_stAffect[i].Type < 50)
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[m_pMyHuman->m_stAffect[i].Type];
					else
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = 0;
					m_pAffectIcon[i]->m_GCPanel.nLayer = 29;
				}
			}	
			else
			{
				if (m_pAffectL[i])
					m_pAffectL[i]->SetVisible(0);
				if (m_pAffectIcon[i] && m_pMiniPanel)
				{
					m_pAffectIcon[i]->SetVisible(0);
					m_dwAffectBlinkTime[i] = 0;
				}
			}
		}
		if (m_pCargoCoin)
		{
			sprintf(szVal, "%10d", g_pObjectManager->m_nCargoCoin);
			m_pCargoCoin->m_cComma = 1;
			m_pCargoCoin->SetText(szVal, 0);
			if (m_pMyCargoCoin)
				m_pMyCargoCoin->SetText(szVal, 0);
		}

		if (!m_pMyHuman->m_pMantua)
			m_pKingDomFlag->SetVisible(0);
		else if (m_pMyHuman->m_pMantua->m_Look.Skin0 == 19)
			m_pKingDomFlag->SetVisible(0);
		else
		{
			char szTemp[256]{};
			m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->m_pMantua->m_Look.Skin0;
			if ((m_pMyHuman->m_sHelmIndex == 3503 || m_pMyHuman->m_sHelmIndex == 3504 || 
				m_pMyHuman->m_sHelmIndex == 3505 || m_pMyHuman->m_sHelmIndex == 3506) && 
				m_pMyHuman->m_pMantua->m_Look.Skin0 == 2)
			{
				m_pKingDomFlag->m_GCPanel.nTextureIndex = 33;
			}

			m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->UnSetCitizenMantle(m_pKingDomFlag->m_GCPanel.nTextureIndex);
			m_pFlagDescText[0]->SetText(g_pItemList[pMobData->Equip[15].sIndex].Name, 0);

			sprintf(szTemp, "%s %d", g_pMessageStringTable[80], BASE_GetItemAbility(&pMobData->Equip[15], 3));
			m_pFlagDescText[1]->SetText(szTemp, 0);

			sprintf(szTemp, "%s %d", g_pMessageStringTable[81], BASE_GetItemAbility(&pMobData->Equip[15], 4));
			m_pFlagDescText[2]->SetText(szTemp, 0);
			m_pKingDomFlag->SetVisible(1);
		}

		if (m_pMyHuman->m_cMount == 1 && 
			(m_pMyHuman->m_nMountSkinMeshType == 31	|| m_pMyHuman->m_nMountSkinMeshType == 40 || 
			 m_pMyHuman->m_nMountSkinMeshType == 20 && m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
		{
			if (m_pBtnMountRun)
			{
				m_pBtnMountRun->m_bEnable = 1;
				m_pBtnMountRun->m_bSelected = g_bRunning;
			}
		}
		else if (m_pBtnMountRun)
		{
			m_pBtnMountRun->m_bEnable = 0;
		}

		SetPosPKRun();
	}
}

void TMFieldScene::UpdateSkillBelt()
{
	auto pSkillBelt2 = m_pGridSkillBelt2;
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem2 = pSkillBelt2->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 248)
		{
			auto pStructItem2 = new STRUCT_ITEM;
			memset(pStructItem2, 0, sizeof(STRUCT_ITEM));

			pStructItem2->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i] + 5295;
			
			auto pSkillItem = new SGridControlItem(0, pStructItem2, 0.0f, 0.0f);

			if (pSkillItem)
				pSkillBelt2->AddSkillItem(pSkillItem, i, 0);

			if (g_pObjectManager->m_cSelectShortSkill == i)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem2)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pReturnItem2);
	}

	auto pSkillBelt3 = m_pGridSkillBelt3;
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem3 = pSkillBelt3->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 248)
		{
			auto pStructItem3 = new STRUCT_ITEM;
			memset(pStructItem3, 0, sizeof(STRUCT_ITEM));

			pStructItem3->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5295;

			auto pSkillItem = new SGridControlItem(0, pStructItem3, 0.0f, 0.0f);

			if (pSkillItem)
				pSkillBelt3->AddSkillItem(pSkillItem, i, 0);

			if (g_pObjectManager->m_cSelectShortSkill == i + 10)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem3)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pReturnItem3);
	}
}

void TMFieldScene::UpdateMyHuman()
{
	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
	
	SANC_INFO stSancInfo{};
	memcpy(&stSancInfo, &m_pMyHuman->m_stOldSancInfo, sizeof(stSancInfo));

	m_pMyHuman->SetPacketMOBItem(&g_pObjectManager->m_stMobData);

	if ((unsigned char)stSancInfo.Sanc0 > 0 && pMobData->Equip[0].sIndex != 32)
		memcpy(&m_pMyHuman->m_stSancInfo, &stSancInfo, sizeof(stSancInfo));

	float fCon = (float)m_pMyHuman->m_stScore.Con;
	m_pMyHuman->SetCharHeight(fCon);
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);

	int nWeaponTypeL = BASE_GetItemAbility(&pMobData->Equip[6], 21);
	if (nWeaponTypeL == 41)
	{
		m_pMyHuman->m_stLookInfo.RightMesh = m_pMyHuman->m_stLookInfo.LeftMesh;
		m_pMyHuman->m_stLookInfo.RightSkin = m_pMyHuman->m_stLookInfo.LeftSkin;
		m_pMyHuman->m_stSancInfo.Sanc6 = m_pMyHuman->m_stSancInfo.Sanc7;
		m_pMyHuman->m_stSancInfo.Legend6 = m_pMyHuman->m_stSancInfo.Legend7;
	}

	m_pMyHuman->InitObject();
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, m_pMyHuman->m_fAngle, 0.0f);

	m_pMyHuman->CheckAffect();
	SetSanc();
}

void TMFieldScene::SetMyHumanExp(long long unExp, int nFakeExp)
{
	auto pMobData = &g_pObjectManager->m_stMobData;

	long long nExp = unExp - pMobData->Exp;
	int nFExp = g_pObjectManager->m_nFakeExp - nFakeExp;

	if (nExp == 0 && nFExp == 0)
		return;

	pMobData->Exp = unExp;
	g_pObjectManager->m_nFakeExp = nFakeExp;

	memcpy(&m_pMyHuman->m_stScore, &pMobData->CurrentScore, sizeof(pMobData->CurrentScore));

	UpdateScoreUI(0);

	int nTX = 0;
	int nTY = 0;
	if (nExp >= 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		sprintf(szStr, "Exp +%d", static_cast<unsigned int>(nExp));

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 0.5f, 0, 1, 1200, 0, 1));

		sprintf(szStr, g_pMessageStringTable[148], nExp);
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
	if (nFExp > 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		sprintf(szStr, "Cp -%d", nFExp);

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 1.0f, 0, 1, 1500, 0, 1));

		sprintf(szStr, g_pMessageStringTable[305], nFExp);
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
}

void TMFieldScene::SetAutoOption(int nIndex, char* szString)
{
}

void TMFieldScene::SetAutoSkillNum(int nCount)
{
	m_nAutoSkillNum = nCount;

	if (m_pAutoSkillPanel)
	{
		for (int i = 1; i <= 10; ++i)
		{
			if (i > nCount)
				m_pAutoSkillPanelChild[i - 1]->SetVisible(0);
			else
				m_pAutoSkillPanelChild[i - 1]->SetVisible(1);
		}
	}
}

void TMFieldScene::SetCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	if (pCamera->m_dwSetTime)
		return;

	m_nCameraMode += 1;
	if (m_nCameraMode > 2)
		m_nCameraMode = 0;

	if (m_bIsDungeon == 1 && m_nCameraMode == 1)
		m_nCameraMode = 2;

	if (m_nCameraMode == 0)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 && 
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
		pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 1)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 && 
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = 11.0f;
		pCamera->m_fWantLength = 11.0f;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 2)
	{
		pCamera->m_nQuaterView = 0;
		pCamera->m_fVerticalAngle = 0.1f;
		pCamera->m_fSightLength = 3.5f;
		pCamera->m_fWantLength = 3.5f;
		pCamera->m_fCamHeight = 0.33f;
		pCamera->m_fHorizonAngle = 3.1415927f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}

	auto pBtnCamera = (SButton*)m_pControlContainer->FindControl(308);
	if (pBtnCamera)
		pBtnCamera->SetSelected(m_bQuater);
}

void TMFieldScene::InitCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	pCamera->m_nQuaterView = 0;
	pCamera->m_fVerticalAngle = -0.78539819f;
	pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
	pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
	pCamera->m_fCamHeight = 0.33000001f;
	pCamera->m_fHorizonAngle = 0.78539819f;
	g_pDevice->m_bFog = 1;
	m_pSky->m_bVisible = 0;
	m_bQuater = 0;
	m_bTab = 0;
	pCamera->EarthQuake(10);
}

void TMFieldScene::SetPK()
{
	TMFieldScene::m_bPK = TMFieldScene::m_bPK == 0;

	m_PkButton->SetSelected(TMFieldScene::m_bPK == 0);
	m_PkButton->Update();

	auto pBtnPK = (SButton*)m_pControlContainer->FindControl(306);
	MSG_STANDARDPARM stParam{};
	stParam.Header.ID = g_pObjectManager->m_dwCharID;
	stParam.Header.Type = MSG_SetPKMode_Opcode;
	stParam.Parm = TMFieldScene::m_bPK;
	SendOneMessage((char*)&stParam, sizeof(stParam));

	if (pBtnPK)
		pBtnPK->SetSelected(TMFieldScene::m_bPK);
}

void TMFieldScene::SetVisibleNameLabel()
{
	m_bShowNameLabel = m_bShowNameLabel == 0;

	auto pBtnName = static_cast<SButton*>(m_pControlContainer->FindControl(307u));

	if (pBtnName)
		pBtnName->SetSelected(m_bShowNameLabel == 0);
}

void TMFieldScene::SetAutoTarget()
{
	m_cAutoAttack = m_cAutoAttack == 0;

	auto pBtnAuto = static_cast<SButton*>(m_pControlContainer->FindControl(312u));

	if (m_cAutoAttack == 0)
		m_pTargetHuman = nullptr;

	if (pBtnAuto)
		pBtnAuto->SetSelected(m_cAutoAttack);

	m_pAutoSkillPanel->SetVisible(m_cAutoAttack);
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

void TMFieldScene::SetWhisper(char cOn)
{
	m_cWhisper = cOn;

	m_pChatWhisper->m_bSelected = m_pChatWhisper->m_bSelected == 0;
	m_pChatWhisper_C->m_bSelected = m_pChatWhisper->m_bSelected == 0;

	m_pChatWhisper->Update();
	m_pChatWhisper_C->Update();
}

void TMFieldScene::SetPartyChat(char cOn)
{
	m_pChatParty->m_bSelected = m_pChatParty->m_bSelected == 0;
	m_pChatParty_C->m_bSelected = m_pChatParty->m_bSelected == 0;

	m_pChatParty->Update();
	m_pChatParty_C->Update();
}

void TMFieldScene::SetGuildChat(char cOn)
{
	m_pChatGuild->m_bSelected = m_pChatGuild->m_bSelected == 0;
	m_pChatGuild_C->m_bSelected = m_pChatGuild->m_bSelected == 0;

	m_pChatGuild->Update();
	m_pChatGuild_C->Update();
}

void TMFieldScene::SetKingDomChat(char cOn)
{
	m_pKingDomGuild->m_bSelected = m_pKingDomGuild->m_bSelected == 0;
	m_pKingDomGuild->Update();
}

void TMFieldScene::SetMyHumanMagic()
{
	;
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

void TMFieldScene::InitBoard()
{
	if (m_pGMsgPanel)
	{
		SListBoxBoardItem* pBoardItem = new SListBoxBoardItem((char*)"0",
			g_pMessageStringTable[176],
			g_pMessageStringTable[177],
			g_pMessageStringTable[178],
			g_pMessageStringTable[179],
			g_pMessageStringTable[180],
			0x5588FFFFu,
			1);

		pBoardItem->SetControlID(824);
		pBoardItem->SetPos(4.0f, 28.0f);
		m_pGMsgListPanel->AddChild(pBoardItem);

		m_pButtonBox = new SButtonBox(4.0f, 320.0f, 500.0f, 16.0f, 1, 10, 1, 1, 10, 1, 10);
		m_pButtonBox->SetControlID(826);
		m_pButtonBox->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

		m_pGMsgListPanel->AddChild(m_pButtonBox);
		m_pGMsgPanel->SetVisible(0);
	}

	m_pHelpPanel = (SPanel*)m_pControlContainer->FindControl(864);
	if (m_pHelpPanel)
	{
		m_pHelpText = (SText*)m_pControlContainer->FindControl(865);
		m_pHelpButton[0] = (SButton*)m_pControlContainer->FindControl(867);
		m_pHelpList[0] = (SListBox*)m_pControlContainer->FindControl(868);
		m_pHelpButton[1] = (SButton*)m_pControlContainer->FindControl(869);
		m_pHelpList[1] = (SListBox*)m_pControlContainer->FindControl(870);
		m_pHelpButton[2] = (SButton*)m_pControlContainer->FindControl(871);
		m_pHelpList[2] = (SListBox*)m_pControlContainer->FindControl(872);
		m_pHelpButton[3] = (SButton*)m_pControlContainer->FindControl(873);
		m_pHelpList[3] = (SListBox*)m_pControlContainer->FindControl(874);
		m_pHelpMemo = (SButton*)m_pControlContainer->FindControl(875);
		m_pHelpSummon = (SButton*)m_pControlContainer->FindControl(878);

		m_pHelpSummon->m_cAlwaysAlt = 1;
		m_pHelpSummon->m_cBlink = 1;

		m_pHelpSummon->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);

		m_pHelpSummon->m_cAlwaysAlt = 0;
		m_pHelpMemo->m_nPosX = (float)(665.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpMemo->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(105.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_cBlink = 1;
		m_pHelpSummon->m_nPosX = (float)(616.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpSummon->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(90.0f * RenderDevice::m_fHeightRatio);

		LoadMsgText(m_pHelpList[0], (char*)"UI\\interface.txt");
		LoadMsgText(m_pHelpList[1], (char*)"UI\\command.txt");
		LoadMsgText(m_pHelpList[2], (char*)"UI\\etc.txt");

		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);
		if (m_pHelpMemo)
			m_pHelpMemo->SetVisible(0);
		if (m_pHelpSummon)
			m_pHelpSummon->SetVisible(0);

		m_pHelpPanel->SetVisible(0);
		m_pHelpPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pHelpPanel->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pHelpPanel->m_nHeight * 0.6f));

		m_pHelpInterface = (SPanel*)m_pControlContainer->FindControl(6067);
		for (int i = 0; i < 3; ++i)
		{
			m_pHelpInterfacePanel[i] = (SPanel*)m_pControlContainer->FindControl(i + 6064);
			m_pHelpInterfaceList[i] = (SListBox*)m_pControlContainer->FindControl(i + 6074);
			if (i > 0)
				m_pHelpInterfacePanel[i]->SetVisible(0);
		}
		LoadMsgText(m_pHelpInterfaceList[0], (char*)"UI\\interface1.txt");
		LoadMsgText(m_pHelpInterfaceList[1], (char*)"UI\\interface2.txt");
		LoadMsgText(m_pHelpInterfaceList[2], (char*)"UI\\interface3.txt");
		m_nCurrInterfacePanelIndex = 0;
	}	

	m_pQuestPanel = (SPanel*)m_pControlContainer->FindControl(1054256);
	if (m_pQuestPanel)
	{
		m_pQuestButton[0] = (SButton*)m_pControlContainer->FindControl(1054259);
		m_pQuestList[0] = (SListBox*)m_pControlContainer->FindControl(1054260);
		m_pQuestContentList[0] = (SListBox*)m_pControlContainer->FindControl(1054261);
		m_pQuestButton[1] = (SButton*)m_pControlContainer->FindControl(1054262);
		m_pQuestList[1] = (SListBox*)m_pControlContainer->FindControl(1054263);
		m_pQuestContentList[1] = (SListBox*)m_pControlContainer->FindControl(1054264);
		m_pQuestButton[2] = (SButton*)m_pControlContainer->FindControl(1054265);
		m_pQuestList[2] = (SListBox*)m_pControlContainer->FindControl(1054266);
		m_pQuestContentList[2] = (SListBox*)m_pControlContainer->FindControl(1054267);
		m_pQuestButton[3] = (SButton*)m_pControlContainer->FindControl(1054268);
		m_pQuestList[3] = (SListBox*)m_pControlContainer->FindControl(1054269);
		m_pQuestContentList[3] = (SListBox*)m_pControlContainer->FindControl(1054271);

		if (m_pQuestList[0])
			m_pQuestList[0]->SetVisible(0);
		if (m_pQuestList[1])
			m_pQuestList[1]->SetVisible(0);
		if (m_pQuestList[2])
			m_pQuestList[2]->SetVisible(0);
		if (m_pQuestList[3])
			m_pQuestList[3]->SetVisible(0);

		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(0);
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(0);
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(0);
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(0);

		m_pQuestPanel->SetVisible(0);

		m_pQuestList[0]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[1]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[2]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[3]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

		m_pQuestPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuestPanel->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pQuestPanel->m_nHeight * 0.6f));

		memset(m_pLevelQuest, 0, sizeof(m_pLevelQuest));

		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects.txt", 97);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects2.txt", 98);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects3.txt", 99);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects4.txt", 101);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestMessage.txt", 100);

		m_pQuestMemo = (SButton*)m_pControlContainer->FindControl(1054273);
		m_pQuestMemo->m_cAlwaysAlt = 1;
		m_pQuestMemo->m_cBlink = 1;

		m_pQuestMemo->m_pAltText->SetPos(-20.0f, -10.0f);

		m_pQuestMemo->m_nPosX = (float)(700.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pQuestMemo->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(105.0f * RenderDevice::m_fHeightRatio);

		if (!g_pObjectManager->m_stMobData.CurrentScore.Level)
		{
			const STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
			LoadMsgText3(m_pQuestList[0], 
				(char*)"UI\\QuestSubjects.txt",	
				g_pObjectManager->m_stMobData.CurrentScore.Level + 1, 
				g_pObjectManager->m_stMobData.Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[1],
				(char*)"UI\\QuestSubjects2.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[2],
				(char*)"UI\\QuestSubjects3.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[3],
				(char*)"UI\\QuestSubjects4.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);

			m_pQuestMemo->SetVisible(1);

			char szStr[128]{};
			unsigned int dwCol = 0xFFAAAAFF;
			if (m_pLevelQuest[pMobData->CurrentScore.Level] == 100)
				dwCol = LoadMsgText4(					
					szStr,
					sizeof szStr,
					(char*)"UI\\QuestMessage.txt",
					pMobData->CurrentScore.Level + 1,
					pMobData->Equip[0].sIndex % 10);
			else
				sprintf(szStr, g_pMessageStringTable[307]);

			SListBoxItem* pChatItem = new SListBoxItem(szStr, dwCol, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);

			if (pChatItem && m_pChatList)
				m_pChatList->AddItem(pChatItem);
		}
		else if (m_pQuestMemo)
		{
			m_pQuestMemo->SetVisible(0);
		}
	}

	m_pPotalPanel = (SPanel*)m_pControlContainer->FindControl(12544);
	m_pPotalList = (SListBox*)m_pControlContainer->FindControl(12545);
	m_pPotalText = (SText*)m_pControlContainer->FindControl(12549);
	m_pPotalText1 = (SText*)m_pControlContainer->FindControl(12550);
	m_pPotalText2 = (SText*)m_pControlContainer->FindControl(12551);
	m_pPotalText3 = (SText*)m_pControlContainer->FindControl(12552);

	m_pPotalPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuestPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pQuestPanel->m_nHeight * 0.6f));

	if (m_pPotalList)
	{
		m_pPotalList->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
	}

	if (m_pPotalPanel)
		m_pPotalPanel->SetVisible(0);

	for (int j = 0; j < 4; ++j)
	{
		m_pQuestList[j]->SetSize(200.0f, m_pQuestList[j]->m_nHeight);
		m_pQuestContentList[j]->SetSize(300.0f, m_pQuestContentList[j]->m_nHeight);
	}
}

int TMFieldScene::LoadMsgText(SListBox* pListBox, char* szFileName)
{
	FILE* fp = nullptr;
	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;
	if (!pListBox)
		return 0;

	char szCol[7]{};

	char szTemp[256]{};
	char szText[256]{};

	unsigned int dwCol = 0;

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		strncpy(szCol, szTemp, 6);
		sscanf(szCol, "%x", &dwCol);

		auto szRet = strstr(szTemp, "\n");
		if (szRet)
			szRet[0] = 0;

		if (szTemp[6] == ' ')
		{
			sprintf(szText, "%s", &szTemp[6]);
			pListBox->AddItem(new SListBoxItem(szText, dwCol | 0xFF000000, 0.0f, 0.0f, 400.0f, 16.0f, 0, 0x77777777, 1u, 0));
		}
	}
	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

void TMFieldScene::SetPosPKRun()
{
	;
}

void TMFieldScene::SetRunMode()
{
	if (m_pMyHuman->m_cMount == 1 && 
		(m_pMyHuman->m_nMountSkinMeshType == 31 || m_pMyHuman->m_nMountSkinMeshType == 40 || m_pMyHuman->m_nMountSkinMeshType == 20 && 
		 m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
	{
		g_bRunning = g_bRunning == 0;
		UpdateScoreUI(0);
	}
}

void TMFieldScene::UpdateFireWorkButton(int nIndex)
{
	if (nIndex >= 0 && nIndex <= 99 && m_pFireWorkPanel)
	{
		if (m_pFireWorkButton[nIndex])
		{
			m_pFireWorkButton[nIndex]->m_bSelected = m_pFireWorkButton[nIndex]->m_bSelected == 0;
			m_pFireWorkButton[nIndex]->Update();
		}
	}
}

void TMFieldScene::ClearFireWork()
{
	if (m_pFireWorkPanel)
	{
		for (int i = 0; i < 100; ++i)
		{
			m_pFireWorkButton[i]->m_bSelected = 0;
			m_pFireWorkButton[i]->Update();
		}
	}
}

void TMFieldScene::DrawCustomFireWork(int nIndex)
{
	// This func is funny haha xD
}

void TMFieldScene::TotoSelect()
{
}

void TMFieldScene::TotoBuy()
{
}

void TMFieldScene::TotoClose()
{
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

void TMFieldScene::SetQuestStatus(bool bStart)
{
	if (bStart == 1)
		m_dwQuestStartTime = timeGetTime();
	else
		m_dwQuestStartTime = 0;

	m_pQuestRemainTime->SetVisible(bStart);
}

void TMFieldScene::UpdateQuestTime()
{
	if (m_dwQuestStartTime)
	{
		int nLeftSecond = 900 - (timeGetTime() - m_dwQuestStartTime) / 1000;

		char strText[128]{};
		sprintf(strText, "[%02d:%02d]", nLeftSecond / 60, nLeftSecond % 60);

		m_pQuestRemainTime->SetText(strText, 0);
		if (!m_pQuestRemainTime->IsVisible())
			m_pQuestRemainTime->SetVisible(1);
	}
}

void TMFieldScene::SetButtonTextXY(SButton* pButton)
{
	if (m_pSystemPanel && pButton)
	{
		TMVector2 vecXY{};
		auto vec = pButton->GetPos();

		int nLen = strlen(pButton->m_GCPanel.pFont->m_szString);
		if (RenderDevice::m_fWidthRatio == 0.80000001f)
		{
			vecXY.x = 43.0f - ((float)(nLen - 1) * 3.0f);
			vecXY.y = 2.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.28f)
		{
			vecXY.x = 66.0f - ((float)(nLen - 1) * 4.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.6f)
		{
			vecXY.x = 75.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 2.0f)
		{
			vecXY.x = 90.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}

		pButton->m_GCPanel.pFont->m_nPosX = (int)((m_pSystemPanel->m_nPosX + vec.x) + vecXY.x);
		pButton->m_GCPanel.pFont->m_nPosY = (int)((m_pSystemPanel->m_nPosY + vec.y) + vecXY.y);
	}
}

/// Trata respostas das caixas de dialogo modais (confirmacoes de compra, troca, guilda, etc.).
int TMFieldScene::OnMsgBoxEvent(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime)
{
	switch (m_pMessageBox->m_dwMessage)
	{
	case 601:
	{
		g_pObjectManager->m_stTrade.OpponentID = m_pMessageBox->m_dwArg;

		MSG_Trade stTrade{};
		memcpy(&stTrade, &g_pObjectManager->m_stTrade, sizeof(stTrade));
		stTrade.Header.ID = m_pMyHuman->m_dwID;
		stTrade.Header.Type = MSG_Trade_Opcode;
		SendOneMessage((char*)&stTrade, sizeof(stTrade));
		if (m_pTradePanel && !m_pTradePanel->IsVisible())
		{
			auto pNode = g_pObjectManager->GetHumanByID(m_pMessageBox->m_dwArg);
			if (!pNode)
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[37], 2000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			auto pTextMyName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_MYNAME));
			auto pTextOPName = static_cast<SText*>(m_pControlContainer->FindControl(TMT_TRADE_OPNAME));

			char szMyName[128]{};
			char szOPName[128]{};
			sprintf_s(szMyName, "[%s]:%d", m_pMyHuman->m_szName, strlen(m_pMyHuman->m_szName));
			sprintf_s(szOPName, "[%s]:%d", pNode->m_szName, strlen(pNode->m_szName));
			pTextMyName->SetText(szMyName, 1);
			pTextOPName->SetText(szOPName, 1);
			SetVisibleTrade(1);
		}
	}
	break;
	case 11:
	{
		m_dwLastTown = g_pTimerManager->GetServerTime();
		m_cLastTown = 1;
		m_pMyHuman->m_bCNFMobKill = 0;

		MSG_STANDARDPARM stDelayStart{};
		stDelayStart.Header.ID = m_pMyHuman->m_dwID;
		stDelayStart.Header.Type = 942;
		stDelayStart.Parm = 2;
		SendOneMessage((char*)&stDelayStart, sizeof(stDelayStart));
	}
	break;
	case 4:
	{
		MSG_ApplyBonus stApplyBonus{};

		stApplyBonus.Header.ID = m_pMyHuman->m_dwID;
		stApplyBonus.Header.Type = MSG_ApplyBonus_Opcode;
		stApplyBonus.BonusType = 2;
		stApplyBonus.Detail = m_pMessageBox->m_dwArg >> 16;
		stApplyBonus.TargetID = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stApplyBonus, sizeof(stApplyBonus));
	}
	break;
	case 6193:
	case 6194:
	case 6195:
	case 6196:
	{
		char Value = 0;
		int GridX = 0;
		int GridY = 0;
		if (m_pMessageBox->m_dwMessage == 6193 || m_pMessageBox->m_dwMessage == 6196)
		{
			GridX = 2;
			GridY = 4;
			if (m_pMessageBox->m_dwMessage == 6193)
				Value = 1;
			else
				Value = 4;
		}
		else if (m_pMessageBox->m_dwMessage == 6194)
		{
			GridX = 2;
			GridY = 3;
			Value = 2;
		}
		if (m_pMessageBox->m_dwMessage == 6195)
		{
			GridX = 1;
			GridY = 1;
			Value = 3;
		}

		for (int i = 0; i < 4; ++i)
		{
			SGridControl* pMyGrid = static_cast<TMFieldScene*>(g_pCurrentScene)->m_pGridInvList[i];

			IVector2 vecGrid = pMyGrid->CanAddItemInEmpty(GridX, GridY);

			if (vecGrid.x > -1 && vecGrid.y > -1)
			{
				MSG_HellBuy stHellBuy{};
				
				stHellBuy.Header.ID = m_pMyHuman->m_dwID;
				stHellBuy.Header.Type = 701;
				stHellBuy.TargetID = m_pMessageBox->m_dwArg;
				stHellBuy.TargetCarryPos = static_cast<unsigned char>(Value);
				stHellBuy.MyCarryPos = vecGrid.x + 5 * vecGrid.y;

				SendOneMessage((char*)&stHellBuy, sizeof(stHellBuy));

				if (g_pSoundManager)
				{
					auto pSoundData = g_pSoundManager->GetSoundData(336);

					if (pSoundData)
						pSoundData->Play(0, 0);
				}
				return 1;
			}
		}
	
		auto pItem = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pItem)
			m_pChatList->AddItem(pItem);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(33);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	break;
	case 816:
	{
		MSG_STANDARDPARM stGuildDep{};
		stGuildDep.Header.ID = m_pMyHuman->m_dwID;
		stGuildDep.Header.Type = 652;
		stGuildDep.Parm = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stGuildDep, sizeof(stGuildDep));
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 817:
	{
		MSG_STANDARDPARM2 stParam{};

		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Header.Type = 3598;
		stParam.Parm1 = m_pMyHuman->m_usGuild;
		stParam.Parm2 = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stParam, sizeof(stParam));
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 862:
	{
		MSG_STANDARDPARM2 stParam{};

		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Header.Type = 3602;
		stParam.Parm1 = m_pMyHuman->m_usGuild;
		stParam.Parm2 = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stParam, sizeof(stParam));
		m_pPGTPanel->SetVisible(0);
	}
	break;
	case 818:
		m_pPGTPanel->SetVisible(0);
		break;
	case 16:
	{
		m_dwGetItemTime = g_pTimerManager->GetServerTime();
		int nX = (int)m_pMyHuman->m_vecPosition.x;
		int nY = (int)m_pMyHuman->m_vecPosition.y;
		char bAttr = BASE_GetAttr(nX, nY);
		if (bAttr & 0x10)
		{
			MSG_STANDARDPARM stParam{};
			stParam.Header.ID = g_pObjectManager->m_dwCharID;
			stParam.Header.Type = 656;
			stParam.Parm = 0;
			SendOneMessage((char*)&stParam, sizeof(stParam));
		}
		else
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[38], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		m_pMessageBox->SetVisible(0);
		return 1;
	}
	break;
	case 927:
	{
		MSG_STANDARDPARM2 stQuest{};

		stQuest.Header.Type = 927;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm1 = m_pMessageBox->m_dwArg;
		stQuest.Parm2 = 4;
		SendOneMessage((char*)&stQuest, sizeof(stQuest));
		m_pPGTOver = nullptr;
	}
	break;
	case 883:
	{
		int nCellX = m_pMessageBox->m_dwArg >> 16;
		int nCellY = m_pMessageBox->m_dwArg & 0xFFFF;

		SGridControlItem* pItem = m_pGridInv->GetItem(nCellX, nCellY);

		if (pItem)
		{
			int nType = BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE);

			UseItem(pItem, nType, pItem->m_pItem->sIndex, nCellX, nCellY);
		}
	}
	break;
	case 13:
	{
		MSG_STANDARDPARM2 stParam{};

		stParam.Header.Type = MSG_Quest_Opcode;
		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Parm1 = m_pMessageBox->m_dwArg;
		stParam.Parm2 = 0;

		SendOneMessage((char*)&stParam, sizeof(stParam));
	}
	break;
	case 15:
	case 10:
	case 1742:
	case 233:
	case 58:
	{
		MSG_STANDARDPARM2 stParam{};

		stParam.Header.Type = MSG_Quest_Opcode;
		stParam.Header.ID = m_pMyHuman->m_dwID;
		stParam.Parm1 = m_pMessageBox->m_dwArg;
		stParam.Parm2 = 1;

		SendOneMessage((char*)&stParam, sizeof(stParam));
	}
	break;
	case 646:
		SendReqBuy(m_pMessageBox->m_dwArg);
		break;
	case 38:
	{
		int nCellX = (m_pMessageBox->m_dwArg & 0xFFFF0000) >> 16;
		int nCellY = m_pMessageBox->m_dwArg & 0xFFFF;
		UseTicket(nCellX, nCellY);
	}
	break;
	case 99:
		m_cResurrect = 1;
		m_dwLastResurrect = g_pTimerManager->GetServerTime();
		return 1;
	case 228:
		m_cLastRelo = 1;
		m_dwLastRelo = g_pTimerManager->GetServerTime();
		break;
	case 50001:
	{
		MSG_STANDARDPARM stRemoveParty{};

		stRemoveParty.Header.Type = 894;
		stRemoveParty.Header.ID = m_pMyHuman->m_dwID;
		stRemoveParty.Parm = m_pMessageBox->m_dwArg;
		SendOneMessage((char*)&stRemoveParty, sizeof(stRemoveParty));
		break;
	}
	case 60:
	{
		MSG_STANDARDPARM2 stParam{};

		stParam.Header.ID = g_pObjectManager->m_dwCharID;
		stParam.Header.Type = 655;
		stParam.Parm1 = m_dwTID;
		stParam.Parm2 = 0;
		SendOneMessage((char*)&stParam, sizeof(stParam));
	}
	break;
	case 65859:
		DoCombine();
		break;
	case 6434:
		DoCombine4();
		break;
	case 81923:
		m_ItemMixClass.DoCombine(m_pMessagePanel, m_pGridInvList, m_Coin);
		break;
	case 86019:
		m_MissionClass.DoCombine(m_pMessagePanel, m_pGridInvList, m_pMyHuman->Is2stClass(), m_Level, m_Coin);
		SetVisibleMissionPanel(0);
		break;
	case 51:
	{
		MSG_STANDARDPARM2 stQuest{};

		stQuest.Header.Type = MSG_Quest_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm1 = m_pMessageBox->m_dwArg;
		stQuest.Parm2 = 0;
		SendOneMessage((char*)&stQuest, sizeof(stQuest));
		m_dwNPCClickTime = dwServerTime;
	}
	break;
	case 740:
	{
		SGridControlItem* pSellItem = SGridControl::m_pSellItem;

		short sDestType = m_pGridInv->CheckType(
			pSellItem->m_pGridControl->m_eItemType,
			pSellItem->m_pGridControl->m_eGridType);

		if (sDestType != 1)
			return 1;

		short sDestPos = pSellItem->m_nCellIndexX + 5 * pSellItem->m_nCellIndexY;

		// TODO:
		// Check if this is correct
		pSellItem->m_pGridControl->PickupAtItem(pSellItem->m_nCellIndexX, pSellItem->m_nCellIndexY);

		int DestPage = 15 * (pSellItem->m_pGridControl->m_dwControlID - 67072);
		if (DestPage < 0 || DestPage > 45)
			DestPage = 0;

		MSG_STANDARDPARM2 stDeleteItem{};

		stDeleteItem.Header.ID = m_pMyHuman->m_dwID;
		stDeleteItem.Header.Type = MSG_DeleteItem_Opcode;
		stDeleteItem.Parm1 = DestPage + sDestPos;
		stDeleteItem.Parm2 = pSellItem->m_pItem->sIndex;
		SendOneMessage((char*)&stDeleteItem, sizeof(stDeleteItem));
		SGridControl::m_pSellItem = nullptr;
	}
	break;
	case 890:
	{
		if (m_pGridInv == nullptr || m_pGridShop == nullptr)
			return 1;

		SGridControlItem* pSellItem = SGridControl::m_pSellItem;

		if (pSellItem)
		{
			short sDestType = m_pGridInv->CheckType(
				pSellItem->m_pGridControl->m_eItemType,
				pSellItem->m_pGridControl->m_eGridType);

			short sDestPos = m_pGridInv->CheckPos(pSellItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
				sDestPos = pSellItem->m_nCellIndexX + 5 * pSellItem->m_nCellIndexY;

			int DestPage = 15 * (pSellItem->m_pGridControl->m_dwControlID - 67072);
			if (DestPage < 0 || DestPage > 45)
				DestPage = 0;

			MSG_Sell stSell{};

			stSell.Header.ID = m_pMyHuman->m_dwID;
			stSell.Header.Type = MSG_Sell_Opcode;
			stSell.TargetID = m_pGridShop->m_dwMerchantID;
			stSell.MyType = sDestType;
			stSell.MyPos = DestPage + sDestPos;

			SendOneMessage((char*)&stSell, sizeof(stSell));
		}
		SGridControl::m_pSellItem = nullptr;
	}
	break;
	case 271:
	case 88:
	{
		MSG_STANDARDPARM2 stQuest{};

		stQuest.Header.Type = MSG_Quest_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm1 = m_pMessageBox->m_dwArg;
		stQuest.Parm2 = 0;

		SendOneMessage((char*)&stQuest, sizeof(stQuest));
	}
	break;
	case 84:
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if ((dwServerTime - m_dwOldAttackTime) < 1000)
			return 1;

		int nSkill = 83 - 24 * g_pObjectManager->m_stMobData.Class;

		if (BASE_GetManaSpent(
			83,
			(unsigned char)g_pObjectManager->m_stMobData.SaveMana,
			g_pObjectManager->m_stMobData.CurrentScore.Special[nSkill / 8 + 1]) <= g_pObjectManager->m_stMobData.CurrentScore.Mp)
		{
			MSG_AttackOne stAttack{};
			
			stAttack.Header.Type = MSG_Attack_One_Opcode;
			stAttack.Header.ID = m_pMyHuman->m_dwID;
			stAttack.AttackerID = m_pMyHuman->m_dwID;
			stAttack.PosX = m_stMoveStop.NextX;
			stAttack.TargetX = stAttack.PosX;
			stAttack.PosY = m_stMoveStop.NextY;
			stAttack.TargetY = stAttack.PosY;
			stAttack.CurrentMp = -1;
			stAttack.SkillIndex = 83;
			stAttack.SkillParm = 0;
			stAttack.Motion = -1;
			SendOneMessage((char*)&stAttack, sizeof(stAttack));
			m_dwOldAttackTime = dwServerTime;
			m_dwSkillLastTime[83] = dwServerTime;

			MSG_Sell stSell{};

			stSell.Header.ID = m_pMyHuman->m_dwID;
			stSell.Header.Type = 724;
			stSell.MyType = m_sDestType;
			stSell.MyPos = m_sDestPos;

			SendOneMessage((char*)&stSell, sizeof(stSell));

			m_sDestType = -1;
			m_sDestPos = -1;

			if (m_pMyHuman->m_pSkinMesh->m_pSwingEffect[0])
				m_pMyHuman->m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 1;
			if (m_pMyHuman->m_pSkinMesh->m_pSwingEffect[1])
				m_pMyHuman->m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 1;
			break;
		}

		auto pItem = new SListBoxItem(g_pMessageStringTable[30], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);

		if (pItem)
			m_pChatList->AddItem(pItem);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(33);
			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	break;
	default:
		break;
	}

	return 1;
}

int TMFieldScene::Guildmark_Create(stGuildMarkInfo* pMark)
{
	if (!pMark || !pMark->pGuildMark)
		return 0;

	int nFindMarkIndex = Guildmark_Find_ArrayIndex(pMark->nGuild + (pMark->nGuildChannel << 16));

	pMark->bLoadedGuildmark = 1;
	if (nFindMarkIndex != -1)
	{
		Guildmark_Link(pMark->pGuildMark, nFindMarkIndex, pMark->sGuildIndex);
		return 1;
	}

	pMark->nMarkIndex = Guildmark_Find_EmptyArrayIndex();
	if (pMark->nMarkIndex == -1)
		pMark->nMarkIndex = Guildmark_DeleteIdleGuildmark();

	if (pMark->nMarkIndex != -1)
	{
		char strFileName[64]{};

		int nChief = 0;
		if (pMark->sGuildIndex == 526 || pMark->sGuildIndex == 529 || pMark->sGuildIndex == 532 || pMark->sGuildIndex == 535)
			nChief = 1;
		if (pMark->sGuildIndex == 527 || pMark->sGuildIndex == 530 || pMark->sGuildIndex == 533 || pMark->sGuildIndex == 536)
			nChief = 2;
		if (pMark->sGuildIndex == 528 || pMark->sGuildIndex == 531 || pMark->sGuildIndex == 534 || pMark->sGuildIndex == 537)
			nChief = 3;


		Guildmark_MakeFileName(strFileName, pMark->nGuild, nChief, pMark->nGuildChannel);
		strcpy(pMark->strMarkFileName, strFileName);
		g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = pMark->nGuild + (pMark->nGuildChannel << 16);
		
		CreateThread(NULL, 0, Guildmark_Download, pMark, 0, NULL);
		return 1;
	}

	return 1;
}

void TMFieldScene::Guildmark_MakeFileName(char* szStr, int nGuild, int nChief, int nChannel)
{
	if (szStr)
		sprintf(szStr, "%c%02d%02d%04d.bmp", 'c', g_pObjectManager->m_nServerGroupIndex, nChannel, nGuild);
}

int TMFieldScene::Guildmark_Find_ArrayIndex(int nGuild)
{
	for (int i = 0; i < 64; ++i)
	{
		if (g_pTextureManager->m_stGuildMark[i].nGuild == nGuild)
			return i;
	}

	return -1;
}

int TMFieldScene::Guildmark_Find_EmptyArrayIndex()
{
	for (int i = 0; i < 64; ++i)
	{
		if (g_pTextureManager->m_stGuildMark[i].nGuild == -1)
			return i;
	}

	return -1;
}

int TMFieldScene::Guildmark_DeleteIdleGuildmark()
{
	int nMostOld = 0;
	unsigned int dwMostOldTime = 0;
	for (int i = 0; i < 64; ++i)
	{
		if (!g_pTextureManager->m_stGuildMark[i].nGuild)
			continue;

		if (!g_pTextureManager->m_stGuildMark[i].dwLastRenderTime)
		{
			nMostOld = i;
			break;
		}
		if (!dwMostOldTime)
		{
			nMostOld = i;
			dwMostOldTime = g_pTextureManager->m_stGuildMark[i].dwLastRenderTime;
		}
		else if (g_pTextureManager->m_stGuildMark[i].dwLastRenderTime < dwMostOldTime)
		{
			nMostOld = i;
			dwMostOldTime = g_pTextureManager->m_stGuildMark[i].dwLastRenderTime;
		}
	}

	g_pTextureManager->m_stGuildMark[nMostOld].nGuild = -1;

	SAFE_RELEASE(g_pTextureManager->m_stGuildMark[nMostOld].pTexture);

	g_pTextureManager->m_stGuildMark[nMostOld].dwLastRenderTime = 0;
	--m_nGuildMarkCount;
	return nMostOld;
}

int TMFieldScene::Guildmark_IsCorrectBMP(char* szMarkBuffer)
{
	if (!szMarkBuffer)
		return 0;

	BITMAPFILEHEADER header;
	memcpy(&header, szMarkBuffer, sizeof(BITMAPFILEHEADER));

	if (header.bfType != 19778)
		return 0;
	if (header.bfSize != 630 && header.bfSize != 632)
		return 0;

	BITMAPINFO info;
	memcpy(&info, &szMarkBuffer[14], header.bfOffBits - 14);
	if (info.bmiHeader.biWidth == 16 && info.bmiHeader.biHeight == 12)
		return info.bmiHeader.biBitCount == 24;
	
	return 0;
}

void TMFieldScene::Guildmark_Link(SPanel* pPanel, int nMarkIndex, int nGuildIndex)
{
	if (pPanel)
	{
		pPanel->m_GCPanel.nMarkIndex = nMarkIndex;
		if (nGuildIndex == 509 || nGuildIndex == 9)
			pPanel->m_GCPanel.nMarkLayout = 1;
		else if ((nGuildIndex < 526 || nGuildIndex > 531) && (nGuildIndex < 3 || nGuildIndex > 8))
			pPanel->m_GCPanel.nMarkLayout = 3;
		else
			pPanel->m_GCPanel.nMarkLayout = 2;
	}
}

void TMFieldScene::SysMsgChat(char* str)
{
	auto pEdit = m_pEditChat;
	auto pChatList = m_pChatListnotice;

	pChatList->AddItem(new SListBoxItem(str, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
	pEdit->SetText((char*)"");

	m_pControlContainer->SetFocusedControl(nullptr);
	if (g_pObjectManager->m_bTvControl != 1)
		pEdit->SetText((char*)"");
}

void TMFieldScene::GetTimeString(char* szVal, int sTime, int nTime, int i)
{
	if (nTime >= 1000000)
	{
		int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
		int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
		int nYearDay = 365;
		if (!(Year % 4))
			nYearDay = 366;

		int nDafaultDay = 0;
		switch (add)
		{
		case 4:
			nDafaultDay = 7;
			break;
		case 5:
			nDafaultDay = 15;
			break;
		case 6:
			nDafaultDay = 30;
			break;
		}

		if (add)
			sprintf(szVal, g_pMessageStringTable[291], nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - m_pMyHuman->m_stAffect[i].Time % 10000));
	}
	else if (sTime > 86400)
		sprintf(szVal, g_pMessageStringTable[291], sTime / 86400);
	else if (sTime > 3600)
		sprintf(szVal, g_pMessageStringTable[292], sTime / 3600);
	else if (sTime <= 600)
		sprintf(szVal, "%5d", sTime);
	else
		sprintf(szVal, g_pMessageStringTable[293], sTime / 60);
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

int TMFieldScene::StrByteCheck(char* szString)
{
	int value = 0;
	int byteCheck = 0;
	for (size_t i = 0; i < strlen(szString); ++i)
	{
		if (szString[i] >= 'A' && szString[i] <= 'z')
			++value;
		else if (byteCheck == 1)
		{
			++value;
			byteCheck = 0;
		}
		else
			byteCheck = 1;
	}

	return value;
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

void TMFieldScene::NewCCMode()
{
	;
}

void TMFieldScene::InsertInChatList(SListBox* pChatList, STRUCT_MOB *pMobData, SEditableText* pEditChat, unsigned int dwColor, int colorId, unsigned int startId)
{
	MSG_MessageWhisper stMsgWhisper{};
	stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
	stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;
	stMsgWhisper.Color = colorId;

	sprintf(stMsgWhisper.MobName, "");
	sprintf(stMsgWhisper.String, "%s", pEditChat->GetText());
	BASE_TransCurse(stMsgWhisper.String);

	pEditChat->SetText((char*)"");

	SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));

	int len = strlen(stMsgWhisper.String) + strlen(pMobData->MobName);
	const size_t maxLen = 40;
	if (len <= maxLen)
	{
		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", pMobData->MobName, &stMsgWhisper.String[startId]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
	else
	{
		char dest[128]{};
		char dest2[128]{};
		if (IsClearString(stMsgWhisper.String, maxLen - 1))
		{
			strncpy(dest, stMsgWhisper.String, maxLen);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen]);
		}
		else
		{
			strncpy(dest, stMsgWhisper.String, maxLen - 1);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen - 1]);
		}

		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", g_pObjectManager->m_stMobData.MobName, &dest[maxLen]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		auto ipNewItem2 = new SListBoxItem(dest2, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (strlen(stMsgWhisper.String) > maxLen && ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
}

/// Thread worker que baixa a imagem BMP da guildmark via HTTP.
DWORD WINAPI Guildmark_Download(void* pArg)
{
	// TODO: we have to find a better way to download the guildmark
	// currently we have a great treat of data race...
	auto pMark = (stGuildMarkInfo*)pArg;
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	if (!g_pCurrentScene || !pMark || !pMark->strMarkFileName[0] || !pMark->pGuildMark)
	{
		g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = -1;
		return 0;
	}

	pFScene->m_dwLastGetGuildmarkTime = timeGetTime();

	char strMarkBuffer[632]{};
	char strURL[64]{};

	strcpy(strURL, g_pMessageStringTable[377]);
	strcat(strURL, pMark->strMarkFileName);

	if (!pFScene->m_hInternetSession)
	{
		pFScene->m_hInternetSession = InternetOpen("MS", 0, 0, 0, 0);
		if (!pFScene->m_hInternetSession)
			return 0;
	}

	auto m_hHttpFile = InternetOpenUrl(pFScene->m_hInternetSession, strURL, 0, 0, 0x4000000, 0);
	
	DWORD dwBytesRead = 0;
	if (m_hHttpFile)
	{
		char szData[1024]{};
		InternetReadFile(m_hHttpFile, szData, 632, &dwBytesRead);
		memcpy(strMarkBuffer, szData, dwBytesRead);
		InternetCloseHandle(m_hHttpFile);

		int bIsCorrectBMP = pFScene->Guildmark_IsCorrectBMP(strMarkBuffer);
		if (bIsCorrectBMP == 1 && g_pTextureManager->LoadGuildTexture(pMark->nMarkIndex, strMarkBuffer) == 1)
		{
			++pFScene->m_nGuildMarkCount;
			g_pTextureManager->m_stGuildMark[pMark->nMarkIndex].nGuild = pMark->nGuild + (pMark->nGuildChannel << 16);
			pMark->pGuildMark->m_GCPanel.nMarkIndex = pMark->nMarkIndex;
			if (pMark->sGuildIndex == 509)
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 1;
			else if (pMark->sGuildIndex >= 526 && pMark->sGuildIndex <= 531)
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 2;
			else
				pMark->pGuildMark->m_GCPanel.nMarkLayout = 3;
		}		
	}

	if(m_hHttpFile != nullptr)
		InternetCloseHandle(m_hHttpFile);

	return 1;
}
