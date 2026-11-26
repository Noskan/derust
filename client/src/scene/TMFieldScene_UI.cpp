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
		return OnControl_CargoMoney(idwControlID, idwEvent);
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
		return OnControl_InvClose(idwControlID, idwEvent);
	if (idwControlID == B_CHAR_CLOSE)
		return OnControl_CharClose(idwControlID, idwEvent);
	if (idwControlID == B_SKILL_CLOSE)
	{
		SetVisibleSkill();

		if (g_nKeyType == 1)
			m_pControlContainer->SetFocusedControl(m_pEditChat);

		return 0;
	}
	if (idwControlID == B_CHAR)
		return OnControl_CharPanel(idwControlID, idwEvent);
	if (idwControlID == B_EQUIP)
	{
		auto pEquipBtn = static_cast<SButton*>(m_pControlContainer->FindControl(B_EQUIP));

		pEquipBtn->SetSelected(pEquipBtn->m_bSelected == 0);

		SetVisibleInventory();
		return 0;
	}
	if (idwControlID == B_CCMODE_SYSTEM)
		return OnControl_CCModeSystem(idwControlID, idwEvent);
	if (idwControlID == B_CCMODE_DLG_MOUNT)
		return OnControl_CCModeMount(idwControlID, idwEvent);
	if (idwControlID == B_CCMODE_DLG_HP)
		return OnControl_CCModeHP(idwControlID, idwEvent);
	if (idwControlID == B_CCMODE_DLG_MODE)
		return OnControl_CCModeMode(idwControlID, idwEvent);
	if (idwControlID == B_CHATLIST_SIZEUP)
	{
		OnKeyPlus(43, 0);
		return 0;
	}
	if (idwControlID == B_CHATLIST_LIGHT)
		return OnControl_ChatListLight(idwControlID, idwEvent);
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
		return OnControl_InvPage1(idwControlID, idwEvent);
	if (idwControlID == B_INV_PAGE2)
		return OnControl_InvPage2(idwControlID, idwEvent);
	if (idwControlID == B_INV_PAGE3)
		return OnControl_InvPage3(idwControlID, idwEvent);
	if (idwControlID == B_INV_PAGE4)
		return OnControl_InvPage4(idwControlID, idwEvent);
	if (idwControlID == B_CARGO_PAGE1)
		return OnControl_CargoPage1(idwControlID, idwEvent);
	if (idwControlID == B_CARGO_PAGE2)
		return OnControl_CargoPage2(idwControlID, idwEvent);
	if (idwControlID == B_CARGO_PAGE3)
		return OnControl_CargoPage3(idwControlID, idwEvent);
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
		return OnControl_GuildOnOff(idwControlID, idwEvent);
	if (idwControlID == TMB_RUNMODE)
	{
		SetRunMode();
		return 0;
	}
	if (idwControlID == B_SYS_QUIT)
		return OnControl_SysQuit(idwControlID, idwEvent);
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
		return OnControl_HelpButton1(idwControlID, idwEvent);
	if (idwControlID == TMB_HELP_BUTTON2)
		return OnControl_HelpButton2(idwControlID, idwEvent);
	if (idwControlID == TMB_HELP_BUTTON3)
		return OnControl_HelpButton3(idwControlID, idwEvent);
	if (idwControlID == TMB_HELP_BUTTON4)
		return OnControl_HelpButton4(idwControlID, idwEvent);
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
		return OnControl_CCPotion(idwControlID, idwEvent);
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
		return OnControl_ShortSkillToggle1(idwControlID, idwEvent);
	if (idwControlID == B_SHORTSKILL_TGL2)
		return OnControl_ShortSkillToggle2(idwControlID, idwEvent);
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
		return OnControl_ChatGeneralToggle(idwControlID, idwEvent);
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
		return OnControl_ServerListSelect(idwControlID, idwEvent, dwServerTime);
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
		return OnControl_PlayerMenu642(idwControlID, idwEvent);
	if (idwControlID == 863)
		return OnControl_PlayerMenu863(idwControlID, idwEvent);
	if (idwControlID == 912)
		return OnControl_PlayerMenu912(idwControlID, idwEvent);
	if (idwControlID >= 913 && idwControlID <= 915)
		return OnControl_PlayerMenu913(idwControlID, idwEvent);
	if (idwControlID == 816)
		return OnControl_PlayerMenu816(idwControlID, idwEvent);
	if (idwControlID == 817)
		return OnControl_PlayerMenu817(idwControlID, idwEvent);
	if (idwControlID == 818)
		return OnControl_PlayerMenu818(idwControlID, idwEvent);
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
		return OnControl_AddSpecial3(idwControlID, idwEvent, pMobData);
	if (idwControlID == 65763)
		return OnControl_AddSpecial4(idwControlID, idwEvent, pMobData);
	if (idwControlID == 475138)
		return OnControl_PartyListClick(idwControlID, idwEvent, dwServerTime, cktrans);
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
		return OnControl_AutoTrade667(idwControlID, idwEvent);

	return 0;
}
