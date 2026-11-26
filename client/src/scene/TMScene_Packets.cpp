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

/// @file TMScene_Packets.cpp
/// @brief Tratamento de pacotes de rede comum a todas as cenas (`TMScene::OnPacketEvent`).
///
/// Extraído de `TMScene.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

int TMScene::OnPacketEvent(unsigned int dwCode, char* pSBuffer)
{
	if (g_pCurrentScene != this)
		return 0;

	auto pStd = reinterpret_cast<MSG_STANDARD*>(pSBuffer);
	auto dwServerTime = g_pTimerManager->GetServerTime();

	if (!pSBuffer && (!m_dwDelayDisconnectTime || m_dwDelayDisconnectTime + 15000 < dwServerTime))
	{
		if (!m_pMessagePanel->IsVisible())
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}

		if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);

		m_dwDelayDisconnectTime = 0;
		return 1;
	}

	if (!pStd)
	{
		if(!m_pMessagePanel->IsVisible())
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}

		if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);

		m_dwDelayDisconnectTime = 0;
		return 1;
	}

	if (pStd->Type == 0x194)
	{
		g_pObjectManager->m_bBilling = 1;
		m_pMessageBox->SetMessage(g_pMessageStringTable[132], B_CREATE_ID, nullptr);
		m_pMessageBox->SetVisible(1);
		return 1;
	}

	if (pStd->Type == MSG_Encode_Opcode)
	{
		MSG_Encode* pEncode = reinterpret_cast<MSG_Encode*>(pStd);

		int EncodeByte1 = *(DWORD*)&pEncode->Parm[40];
		int EncodeByte2 = *(DWORD*)&pEncode->Parm[41];

		int type = (3 * (EncodeByte1 / 7) + 7 * (EncodeByte2 / 3) + 220) % 3;
		if (type)
		{
			if (type == 1)
				pStd->Type = 0x13BD;
			else if (type == 2)
				pStd->Type = 0x7BE;
		}
		else
			pStd->Type = 0xFBC;
		return 1;
	}

	if (pStd->Type == 0xFBC)
	{
		// need to decompile here... and other encode packets
		return 1;
	}

	if (pStd->Type == 0x13BD)
	{
		// need to decompile here... and other encode packets
		return 1;
	}

	if (pStd->Type == 0x7BE)
	{
		// need to decompile here... and other encode packets
		return 1;
	}
	if (!pStd->ID && (pStd->Type == MSG_MessagePanel_Opcode || pStd->Type == 0x102 || pStd->Type == 0x104 || pStd->Type == 0x105 || pStd->Type == 0x106))
	{
		char szStr[128] = { 0 };
		if (pStd->Type != MSG_MessagePanel_Opcode)
		{
			if (pStd->Type == 0x105)
			{
				auto pMessageChat = reinterpret_cast<MSG_MessageChat*>(pStd);
				if (!pMessageChat->String[0])
				{
					char str[128]{};
					char num[5]{};

					int index = *reinterpret_cast<short*>(&pMessageChat->String[2]) + 1000;
					g_pMessageStringTable[index][127] = 0;
					g_pMessageStringTable[index][126] = 0;

					strcpy(str, g_pMessageStringTable[index]);
					if (strlen(str) < 1)
						strcpy(str, _itoa(*reinterpret_cast<short*>(&pMessageChat->String[2]), num, 10));

					if (index == 465 || index == 466 || index == 485 || index == 484)
						m_pMessagePanel->SetMessage(str, 600000);
					else
						m_pMessagePanel->SetMessage(str, 4000);

					m_pMessagePanel->SetVisible(1, 1);
				}
			}
			else if (pStd->Type == 0x106)
			{
				auto pMessageChat = reinterpret_cast<MSG_MessageChat*>(pStd);
				if (!pMessageChat->String[0])
				{
					char szStr[128]{};
					char szParse[6][128]{};
					int Param = 0;
					strcpy(szStr, &pMessageChat->String[4]);

					for (int k = 0; k < 128; ++k)
					{
						if (szStr[k] == ',')
						{
							szStr[k] = ' ';
							++Param;
						}
						if (!szStr[k])
							break;
					}

					sscanf(szStr, "%s %s %s %s %s %s", szParse[0], szParse[1], szParse[2], szParse[3], szParse[4], szParse[5]);


					int index = *reinterpret_cast<short*>(&pMessageChat->String[2]) + 1000;

					memset(szStr, 0, sizeof(szStr));
					sprintf(szStr, g_pMessageStringTable[index], szParse[0], szParse[1], szParse[2], szParse[3], szParse[4], szParse[5]);

					char buffer[4]{};

					if (strlen(szStr) < 1)
						strcpy(szStr, _itoa(*reinterpret_cast<short*>(&pMessageChat->String[2]), buffer, 10));

					if (index == 465 || index == 466 || index == 485 || index == 484)
						m_pMessagePanel->SetMessage(szStr, 600000);
					else
						m_pMessagePanel->SetMessage(szStr, 4000);

					m_pMessagePanel->SetVisible(1, 1);
				}
			}

			return 1;
		}

		auto pMsgPanel = reinterpret_cast<MSG_MessagePanel*>(pStd);
		pMsgPanel->String[127] = 0;
		pMsgPanel->String[126] = 0;

		if (m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR && pMsgPanel->String[0] == '^')
		{
			char szMsg[128]{ 0 };
			sprintf_s(szMsg, "%s", &pMsgPanel->String[1]);

			m_pMessageBox2->SetMessage(szMsg, 0, nullptr);
			m_pMessageBox2->SetVisible(1);
		}
		else if (pMsgPanel->String[0] == '^')
		{
			char szMsg[128]{ 0 };
			sprintf_s(szMsg, "%s", &pMsgPanel->String[1]);

			m_pMessagePanel->SetMessage(szMsg, 7000);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else if (pMsgPanel->String[0] == '!' && pMsgPanel->String[1] == '#')
		{
			for (int i = 2; i < 6; i++)
			{
				if (pMsgPanel->String[i] < '0' || pMsgPanel->String[i] > '9')
					pMsgPanel->String[i] = '0';
			}

			TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
			pFScene->m_nYear = static_cast<unsigned short>(pMsgPanel->String[3] - 48)
				+ 10 * static_cast<unsigned short>(pMsgPanel->String[2] - 48);
			pFScene->m_nDays = static_cast<unsigned short>(pMsgPanel->String[6] - 48)
				+ 10 * static_cast<unsigned short>(pMsgPanel->String[5] - 48)
				+ 100 * static_cast<unsigned short>(pMsgPanel->String[4] - 48);

			return 1;
		}
		else if (pMsgPanel->String[1] == '!' && pMsgPanel->String[2] == '!' && pMsgPanel->String[3] == '!')
		{
			auto pFocusedObject = static_cast<TMObject*>(m_pMyHuman);

			if ((int)pFocusedObject->m_vecPosition.x >> 7 == 31 && (int)pFocusedObject->m_vecPosition.y >> 7 == 31)
			{
				for (int nType = 0; nType < 5; ++nType)
				{
					for (int j = 0; j < 5; ++j)
					{
						auto pFireWork = new TMEffectFireWork({ (pFocusedObject->m_vecPosition.x) - 10.0f + (5.0f * nType), 7.0f, ((pFocusedObject->m_vecPosition.y - 10.0f) + 5.0f * j) + 8.0f }, nType);

						g_pCurrentScene->AddChild(pFireWork);
					}
				}
			}

			if (pFocusedObject->IsInTown() == 1 || (int)pFocusedObject->m_vecPosition.x >> 7 != 31 || (int)pFocusedObject->m_vecPosition.y >> 7 != 31)
			{
				m_pMessagePanel->SetMessage(pMsgPanel->String, 4000);
				m_pMessagePanel->SetVisible(1, 1);
			}
		}
		else if (pMsgPanel->String[1] == '!' && pMsgPanel->String[2] == '!' && pMsgPanel->String[3] == '#')
		{
			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && pMsgPanel->String[4] == 'E')
			{
				auto pKilled = static_cast<TMHuman*>(g_pObjectManager->GetHumanByID(static_cast<TMFieldScene*>(g_pCurrentScene)->m_dwKhepraID));

				if (pKilled)
				{
					pKilled->m_stScore.Hp = 0;
					pKilled->Die();
				}

				static_cast<TMFieldScene*>(g_pCurrentScene)->m_dwKhepraID = 0;

				for (int iSt = 0; iSt < 5; ++iSt)
					pMsgPanel->String[iSt] = '_';
			}
		}
		else if (pMsgPanel->String[0] == '!' || pMsgPanel->String[1] == '!')
		{
			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				for (int n = 2; n < 8; ++n)
				{
					if (pMsgPanel->String[n] < '0' || pMsgPanel->String[n] > '9')
						pMsgPanel->String[n] = '0';
				}

				TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
				pFScene->m_NightmareTime.wHour = pMsgPanel->String[3] - 48 + 10 * (pMsgPanel->String[2] - 48);
				pFScene->m_NightmareTime.wMonth = pMsgPanel->String[5] - 48 + 10 * (pMsgPanel->String[4] - 48);
				pFScene->m_NightmareTime.wSecond = pMsgPanel->String[7] - 48 + 10 * (pMsgPanel->String[6] - 48);
				pFScene->m_dwLastNightmareTime = g_pTimerManager->GetServerTime();
				return 1;
			}
		}
		else if (pMsgPanel->String[0] == '!')
		{
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);

			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

				auto pItem3 = new SListBoxItem(" ", 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
				pFScene->m_pHelpList[3]->AddItem(pItem3);

				char szTime[128]{ 0 };
				char _Buffer[128]{ 0 };

				sprintf_s(szTime, "SMS [%02d:%02d:%02d]", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
				auto pItem = new SListBoxItem(szTime, 0xFFBBFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
				pFScene->m_pHelpList[3]->AddItem(pItem);

				sprintf_s(_Buffer, "%s", &pMsgPanel->String[1]);

				auto pItem2 = new SListBoxItem(_Buffer, 0xFFBBFFCC, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777u, 1u, 0);
				pFScene->m_pHelpList[3]->AddItem(pItem2);

				if (pFScene->m_pHelpMemo)
					pFScene->m_pHelpMemo->SetVisible(1);
			}
			else
			{
				for (int l = 0; l < 98; ++l)
				{
					if (!g_pObjectManager->m_stMemo[l].szString[0] && !g_pObjectManager->m_stMemo[l + 1].szString[0])
					{
						g_pObjectManager->m_stMemo[l].dwColor = -1;
						sprintf_s(g_pObjectManager->m_stMemo[l].szString, "");
						g_pObjectManager->m_stMemo[l + 1].dwColor = 0xFFBBFFFF;
						sprintf_s(g_pObjectManager->m_stMemo[l + 1].szString, "SMS [%02d:%02d:%02d]", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

						g_pObjectManager->m_stMemo[l + 2].dwColor = 0xFFBBFFCC;
						sprintf_s(g_pObjectManager->m_stMemo[l + 2].szString, "%s", &pMsgPanel->String[1]);
					}
				}
			}

			m_pMessagePanel->SetMessage(&pMsgPanel->String[1], 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else if (pMsgPanel->String[0] == '2' && pMsgPanel->String[1] == '0' && pMsgPanel->String[2] == '0')
		{
			char Msg[128]{ 0 };
			if (m_pMyHuman)
				sprintf_s(Msg, "%s %d %d %d", pMsgPanel->String, g_pObjectManager->m_nServerIndex, static_cast<int>(m_pMyHuman->m_vecPosition.x), 
					static_cast<int>(m_pMyHuman->m_vecPosition.y));
			else
				sprintf_s(Msg, "%s %d", pMsgPanel->String, g_pObjectManager->m_nServerIndex);

			m_pMessagePanel->SetMessage(Msg, 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}
		else
		{
			m_pMessagePanel->SetMessage(pMsgPanel->String, 4000u);
			m_pMessagePanel->SetVisible(1, 1);
		}

		SListBox* pChatList = static_cast<SListBox*>(m_pControlContainer->FindControl(65667));

		int len = strlen(pMsgPanel->String);
		int size = 50;
		if (len > size)
		{
			char szMsg2[128]{};
			char szMsg3[128]{};

			if (IsClearString(pMsgPanel->String, size - 1))
			{
				strncpy(szMsg3, pMsgPanel->String, size);
				sprintf(szMsg2, "%s", &pMsgPanel->String[size]);
			}
			else
			{
				strncpy(szMsg3, pMsgPanel->String, size - 1);
				sprintf(szMsg2, "%s", &pMsgPanel->String[size - 1]);
			}

			SListBoxItem* ipNewItem = new SListBoxItem(szMsg3, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);

			SListBoxItem* ipNewItem2 = new SListBoxItem(szMsg2, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0, 0, 0x77777777, 1u, 0);
			if (ipNewItem2 && pChatList)
				pChatList->AddItem(ipNewItem2);
		}
		else
		{
			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
				if (!strcmp(pMsgPanel->String, "Whisper : Off"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[448], g_pMessageStringTable[447]);
					pFScene->SetWhisper(0);
				}
				if (!strcmp(pMsgPanel->String, "Whisper : On"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[448], g_pMessageStringTable[446]);
					pFScene->SetWhisper(1);
				}
				if (!strcmp(pMsgPanel->String, "Citizen Chatting : Off"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[449], g_pMessageStringTable[447]);
					pFScene->SetPartyChat(0);
				}
				if (!strcmp(pMsgPanel->String, "Citizen Chatting : On"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[449], g_pMessageStringTable[446]);
					pFScene->SetPartyChat(1);
				}
				if (!strcmp(pMsgPanel->String, "Guild Chatting : Off"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[450], g_pMessageStringTable[447]);
					pFScene->SetGuildChat(0);
				}
				if (!strcmp(pMsgPanel->String, "Guild Chatting : On"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[450], g_pMessageStringTable[446]);
					pFScene->SetGuildChat(1);
				}
				if (!strcmp(pMsgPanel->String, "Kingdom Chatting : On"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[451], g_pMessageStringTable[446]);
					pFScene->SetKingDomChat(1);
				}
				if (!strcmp(pMsgPanel->String, "Kingdom Chatting : Off"))
				{
					sprintf(pMsgPanel->String, "%s%s", g_pMessageStringTable[451], g_pMessageStringTable[447]);
					pFScene->SetKingDomChat(0);
				}
			}

			SListBoxItem* ipNewItem = new SListBoxItem(pMsgPanel->String, 0xFFCCAAFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);
		}

		SControl* pLoginOK = m_pControlContainer->FindControl(65873);
		if (pLoginOK)
			pLoginOK->SetEnable(1);
		return 1;
	}
	if (pStd->Type == MSG_MessageShout_Opcode)
	{
		MSG_MessageWhisper* pShoutMessage = reinterpret_cast<MSG_MessageWhisper*>(pStd);
		pShoutMessage->String[111] = 0;
		pShoutMessage->String[110] = 0;

		TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
		if (!pFScene->m_pChatGeneral || pFScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD || pFScene->m_pCHP->m_bSelectEnable)
		{
			int nIndex = 0;
			unsigned int dwColor = 0xFF00CD00;

			SListBox* pChatList = pFScene->m_pChatList;

			int nLen = strlen(pShoutMessage->MobName);

			SListBoxItem* ipNewItem = new SListBoxItem(pShoutMessage->MobName,
				dwColor,
				0.0f,
				0.0f,
				(float)nLen * 6.6999998f,
				16.0f,
				0,
				0xFFFFFF00,
				1u,
				0);

			ipNewItem->m_bBGColor = 0;
			if (ipNewItem && pChatList)
				pChatList->AddItem(ipNewItem);

			pFScene->m_dwChatTime = g_pTimerManager->GetServerTime();
			return 1;
		}
		else
			return 1;
	}
	if (pStd->Type == 0x7DB)
	{
		MSG_STANDARDPARM* m = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

		g_pApp->china_bWrite = 1;
		g_pApp->china_Playtime = m->Parm / 60;
		LOG_WRITELOG("get start Time : %d \r\n", g_pApp->china_Playtime);
		return 1;
	}

	if (m_pControlContainer && m_pControlContainer->OnPacketEvent(dwCode, pSBuffer) == 1)
		return 1;

	TreeNode::OnPacketEvent(dwCode, pSBuffer);
	return 0;
}
