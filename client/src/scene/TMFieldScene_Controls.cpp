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

/// @file TMFieldScene_Controls.cpp
/// @brief Handlers dos controles de UI do campo extraídos de `TMFieldScene::OnControlEvent`.
///
/// Fase 3: cada bloco `if (idwControlID == ...) { ...; return X; }` virou um método
/// `OnControl_*`, chamado como `return OnControl_*(...)`. Os corpos foram movidos sem
/// alteração; as locais de OnControlEvent que eles liam (`dwServerTime`, `pMobData`,
/// `cktrans`, constantes após o início da função) são passadas por valor com o mesmo nome.

/// Seleção de item na loja do Hell Gate (`TMB_HELLSTORE_SELECT_1..4`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HellStoreSelect(unsigned int idwControlID, unsigned int idwEvent)
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

/// Setas de aposta do Gamble (`TMB_GAMBLE_LEFTX2..TMB_GAMBLE_RIGHT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_GambleArrows(unsigned int idwControlID, unsigned int idwEvent)
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

/// Início da aposta do Gamble (`TMB_GAMBLE_START`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_GambleStart(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Envio da linha de chat (`E_CHAT`): comandos `/`, sussurro, party, guilda, reino e chat normal.
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ChatInput(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime, STRUCT_MOB* pMobData)
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

/// Botão de gold do inventário (`B_MONEY`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_Money(unsigned int idwControlID, unsigned int idwEvent)
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
