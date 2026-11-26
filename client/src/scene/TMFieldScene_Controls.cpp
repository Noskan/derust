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

/// Confirmação da caixa de entrada de texto/valor (`B_IG_OK`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InputOk(unsigned int idwControlID, unsigned int idwEvent)
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

/// Seleção do canal de chat (`B_CHAT_SELECT_NOMAL..B_CHAT_SELECT_SHOUT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ChatSelect(unsigned int idwControlID, unsigned int idwEvent)
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

/// Diálogo do modo CC (`P_CCMODE_DLG_PONT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCModeDialog(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 1 do painel de quest (`B_QUEST_BUTTON`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestButton1(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 2 do painel de quest (`B_QUEST_BUTTON2`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestButton2(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 3 do painel de quest (`B_QUEST_BUTTON3`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestButton3(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 4 do painel de quest (`B_QUEST_BUTTON4`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestButton4(unsigned int idwControlID, unsigned int idwEvent)
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

/// Memo de quest (`B_QUEST_MEMO`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestMemo(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Comando de ataque do modo CC (`B_CCATTACK`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCAttack(unsigned int idwControlID, unsigned int idwEvent)
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

/// Comando de movimento do modo CC (`B_CCMOVE`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCMove(unsigned int idwControlID, unsigned int idwEvent)
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

/// Abertura do log de quests (`B_QUESTLOG`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_QuestLog(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Início do voo pelo painel de portais (controle 12546).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AirMoveStart(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo (`m_pPGTOver`/`m_dwOpID`), controle 641.
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu641(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 643 (exige distância).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu643(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 912 (guilda).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu912(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controles 913..915 (guilda).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu913(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controles 639/621/622/623 (pacote 0x39F).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu639(unsigned int idwControlID, unsigned int idwEvent)
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

/// Distribuição de ponto especial 1 (controle 65754, `SpecialBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddSpecial1(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto especial 2 (controle 65757, `SpecialBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddSpecial2(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto especial 3 (controle 65760, `SpecialBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddSpecial3(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto especial 4 (controle 65763, `SpecialBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddSpecial4(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Clique na lista de party (controle 475138; `idwEvent` = índice do item).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PartyListClick(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime, int cktrans)
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

/// Validação/abertura da venda automática (controle 667).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AutoTrade667(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão de gold do baú (`B_CARGO_MONEY`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CargoMoney(unsigned int idwControlID, unsigned int idwEvent)
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

/// Fechar inventário (`B_INV_CLOSE`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InvClose(unsigned int idwControlID, unsigned int idwEvent)
{
	SetVisibleInventory();

	auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

	pEquipBtn->SetSelected(m_pInvenPanel->m_bVisible);

	if (g_nKeyType == 1)
		m_pControlContainer->SetFocusedControl(m_pEditChat);

	return 0;
}

/// Fechar janela do personagem (`B_CHAR_CLOSE`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CharClose(unsigned int idwControlID, unsigned int idwEvent)
{
	SetVisibleCharInfo();

	auto pCharBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_CHAR));

	pCharBtn->SetSelected(m_pCPanel->m_bVisible);

	if (g_nKeyType == 1)
		m_pControlContainer->SetFocusedControl(m_pEditChat);

	return 0;
}

/// Abrir/fechar janela do personagem (`B_CHAR`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CharPanel(unsigned int idwControlID, unsigned int idwEvent)
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

/// Sistema do modo CC (`B_CCMODE_SYSTEM`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCModeSystem(unsigned int idwControlID, unsigned int idwEvent)
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

/// Diálogo do modo CC: montaria (`B_CCMODE_DLG_MOUNT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCModeMount(unsigned int idwControlID, unsigned int idwEvent)
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

/// Diálogo do modo CC: HP (`B_CCMODE_DLG_HP`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCModeHP(unsigned int idwControlID, unsigned int idwEvent)
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

/// Diálogo do modo CC: modo (`B_CCMODE_DLG_MODE`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCModeMode(unsigned int idwControlID, unsigned int idwEvent)
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

/// Realce da lista de chat (`B_CHATLIST_LIGHT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ChatListLight(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 1 do inventário (`B_INV_PAGE1`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InvPage1(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 2 do inventário (`B_INV_PAGE2`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InvPage2(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 3 do inventário (`B_INV_PAGE3`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InvPage3(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 4 do inventário (`B_INV_PAGE4`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_InvPage4(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 1 do baú (`B_CARGO_PAGE1`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CargoPage1(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 2 do baú (`B_CARGO_PAGE2`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CargoPage2(unsigned int idwControlID, unsigned int idwEvent)
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

/// Página 3 do baú (`B_CARGO_PAGE3`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CargoPage3(unsigned int idwControlID, unsigned int idwEvent)
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

/// Liga/desliga exibição de guilda (`TMB_GUILDONOFF`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_GuildOnOff(unsigned int idwControlID, unsigned int idwEvent)
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

/// Sair do jogo pelo menu do sistema (`B_SYS_QUIT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_SysQuit(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 1 da ajuda (`TMB_HELP_BUTTON1`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HelpButton1(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 2 da ajuda (`TMB_HELP_BUTTON2`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HelpButton2(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 3 da ajuda (`TMB_HELP_BUTTON3`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HelpButton3(unsigned int idwControlID, unsigned int idwEvent)
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

/// Botão 4 da ajuda (`TMB_HELP_BUTTON4`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HelpButton4(unsigned int idwControlID, unsigned int idwEvent)
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

/// Poção do modo CC (`B_CCPOTION`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_CCPotion(unsigned int idwControlID, unsigned int idwEvent)
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

/// Alterna barra de atalhos de skill 1 (`B_SHORTSKILL_TGL1`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ShortSkillToggle1(unsigned int idwControlID, unsigned int idwEvent)
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

/// Alterna barra de atalhos de skill 2 (`B_SHORTSKILL_TGL2`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ShortSkillToggle2(unsigned int idwControlID, unsigned int idwEvent)
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

/// Alterna o filtro de chat geral (controle 65677).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ChatGeneralToggle(unsigned int idwControlID, unsigned int idwEvent)
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

/// Seleção de servidor na lista para troca de canal (controle 12289).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ServerListSelect(unsigned int idwControlID, unsigned int idwEvent, unsigned int dwServerTime)
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

/// Menu de contexto do jogador alvo, controle 642 (esconde os botões do menu).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu642(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 863 (submenu de guilda).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu863(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 816 (mensagem 36).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu816(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 817 (mensagem 158).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu817(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 818.
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu818(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 862 (mensagem 221).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu862(unsigned int idwControlID, unsigned int idwEvent)
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

/// Menu de contexto do jogador alvo, controle 620.
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_PlayerMenu620(unsigned int idwControlID, unsigned int idwEvent)
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

/// Pedra/papel/tesoura: pedra (controle 1617, sussurro `_RPS_`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_RpsRock(unsigned int idwControlID, unsigned int idwEvent)
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

/// Pedra/papel/tesoura: controle 1618 (sussurro `_RPS_`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_RpsPaper(unsigned int idwControlID, unsigned int idwEvent)
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

/// Pedra/papel/tesoura: controle 1619 (sussurro `_RPS_`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_RpsScissors(unsigned int idwControlID, unsigned int idwEvent)
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

/// Distribuição de ponto de atributo 1 (controle 65716, `ScoreBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddScore1(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto de atributo 2 (controle 65719, `ScoreBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddScore2(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto de atributo 3 (controle 65722, `ScoreBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddScore3(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Distribuição de ponto de atributo 4 (controle 65725, `ScoreBonus`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_AddScore4(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
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

/// Abre a lista de chat e pede o pacote 0x37E (controle 475139).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_ChatListOpen(unsigned int idwControlID, unsigned int idwEvent)
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

/// Confirmação da troca (controle 617, `MyCheck`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_TradeCheck(unsigned int idwControlID, unsigned int idwEvent)
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
