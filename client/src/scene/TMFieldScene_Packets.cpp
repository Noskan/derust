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

/// @file TMFieldScene_Packets.cpp
/// @brief Despacho e handlers de pacotes de rede (OnPacket*) recebidos durante o jogo.

/// Despacha pacotes de jogo recebidos do TMSrv para o respectivo handler OnPacket*.
int TMFieldScene::OnPacketEvent(unsigned int dwCode, char* buf)
{
	if (TMScene::OnPacketEvent(dwCode, buf) == 1)
		return 1;

	auto pStd = (MSG_STANDARD*)buf;

	switch (pStd->Type)
	{
	case MSG_CreateMob_Opcode:
	case MSG_CreateMobTrade_Opcode:
		return OnPacketCreateMob(pStd);
	case MSG_Action_Opcode:
	case MSG_Action_Stop_Opcode:
		return OnPacketAction(pStd);
	case 0x1C1:
		return OnPacketREQArray(pStd);
	case 0x333:
		return OnPacketMessageChat(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x105:
		return OnPacketMessageChat_Index(reinterpret_cast<MSG_MessageChat*>(pStd));
	case 0x106:
		return OnPacketMessageChat_Param(pStd);
	case 0x334:
		return OnPacketMessageWhisper(reinterpret_cast<MSG_MessageWhisper*>(pStd));
	case 0x7B1:
		return OnPacketLongMessagePanel(reinterpret_cast<MSG_LongMessagePanel*>(pStd));
	case 0x3B2:
		return OnPacketReqSummon(reinterpret_cast<MSG_ReqSummon*>(pStd));
	case 0x3B3:
		return OnPacketCancelSummon(pStd);
	case 0x3A3:
		return OnPacketSoundEffect(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case 0x116:
		return OnPacketCNFCharacterLogout(pStd);
	case 0x52A:
		return OnPacketCNFRemoveServer(reinterpret_cast<MSG_CNFRemoveServer*>(pStd));
	case 0x10A:
		return OnPacketCNFAccountLogin(reinterpret_cast<MSG_CNFRemoveServerLogin*>(pStd));
	case 0x114:
		return OnPacketCNFCharacterLogin(reinterpret_cast<MSG_CNFCharacterLogin*>(pStd));
	case 0x3E8:
		return OnPacketUndoSellItem(reinterpret_cast<MSG_RepurchaseItems*>(pStd));
	case 0x39B:
		return OnPacketItemSold(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
	case 0x339:
		return OnPacketUpdateCargoCoin(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case 0x18B:
		return OnPacketWeather(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case 0x26E:
		return OnPacketCreateItem(reinterpret_cast<MSG_CreateItem*>(pStd));
	case 0x175:
		return OnPacketCNFDropItem(reinterpret_cast<MSG_CNFDropItem*>(pStd));
	case 0x171:
		return OnPacketCNFGetItem(reinterpret_cast<MSG_CNFGetItem*>(pStd));
	case 0x374:
		return OnPacketUpdateItem(reinterpret_cast<MSG_UpdateItem*>(pStd));
	case 0x16F:
		return OnPacketRemoveItem(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
	case 0x1D0:
		g_pObjectManager->m_RMBShopOpen = 1;
		return OnPacketShopList(pStd);
		break;
	case 0x379:
		return OnPacketBuy(pStd);
		break;
	case 0x338:
		return OnPacketCNFMobKill(reinterpret_cast<MSG_CNFMobKill*>(pStd));
		break;
	case 0x37F:
		return OnPacketREQParty(reinterpret_cast<MSG_REQParty*>(pStd));
		break;
	case 0x37D:
		return OnPacketAddParty(reinterpret_cast<MSG_AddParty*>(pStd));
		break;
	case 0x37E:
		return OnPacketRemoveParty(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x292:
		return OnPacketSetHpMode(reinterpret_cast<MSG_SetHpMode*>(pStd));
		break;
	case 0x18D:
		return OnPacketReqChallange(pStd);
		break;
	case 0x378:
		return OnPacketSetShortSkill(reinterpret_cast<MSG_SetShortSkill*>(pStd));
		break;
	case 0x19C:
		return OnPacketClearMenu(pStd);
		break;
	case 0x3A7:
		return OnPacketCombineComplete(pStd);
		break;
	case 0x3AC:
		return OnPacketCastleState(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3A1:
		return OnPacketStartTime(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3B0:
		return OnPacketRemainCount(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3A8:
		return OnPacketWarInfo(reinterpret_cast<MSG_STANDARDPARM3*>(pStd));
		break;
	case 0x3A4:
		return OnPacketGuildDisable(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3A2:
		return OnPacketEnvEffect(pStd);
		break;
	case 0x3BB:
		return OnPacketRemainNPCCount(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x1BF:
		return OnPacketRESULTGAMBLE(pStd);
		break;
	case MSG_AutoTrade_Opcode:
		return OnPacketAutoTrade(pStd);
	case MSG_SwapItem_Opcode:
		return OnPacketSwapItem(pStd);
	case MSG_ShopList_Opcode:
		return OnPacketShopList(pStd);
	case MSG_Sell_Opcode:
		return OnPacketSell(pStd);
	case MSG_Deposit_Opcode:
		return OnPacketDeposit(pStd);
	case MSG_Withdraw_Opcode:
		return OnPacketWithdraw(pStd);
	case MSG_CloseShop_Opcode:
		return OnPacketCloseShop(pStd);
	case MSG_Attack_Multi_Opcode:
	case MSG_Attack_One_Opcode:
	case MSG_Attack_Two_Opcode:
		return OnPacketAttack(pStd);
	case 0x1C5:
		return OnPacketNuke(pStd);
		break;
	case 0x1C6:
		return OnPacketRandomQuiz(reinterpret_cast<MSG_RandomQuiz*>(pStd));
		break;
	case 0x2C8:
		return OnPacketAutoKick(pStd);
		break;
	case 0x1C2:
		return OnPacketItemPrice(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
		break;
	case 0xDC3:
		return OnPacketCapsuleInfo(reinterpret_cast<MSG_CAPSULEINFO*>(pStd));
		break;
	case 0x3CF:
		return OnPacketRunQuest12Start(reinterpret_cast<MSG_STANDARDPARM*>(pStd));
		break;
	case 0x3D0:
		return OnPacketRunQuest12Count(reinterpret_cast<MSG_STANDARDPARM2*>(pStd));
		break;
	case 0x3AE:
		return OnPacketDelayQuit((MSG_STANDARDPARM*)pStd);
		break;
	}

	return 0;
}

int TMFieldScene::OnPacketMessageChat(MSG_MessageChat* pStd)
{	
	if (m_pPartyList->m_nNumItem <= 0)
		return 0;

	if (!g_pObjectManager->GetHumanByID(pStd->Header.ID))
	{
		auto pPartyList = m_pPartyList;
		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
			if (pPartyItem->m_dwCharID == pStd->Header.ID)
			{
				pStd->String[127] = 0;
				pStd->String[126] = 0;

				auto pChatList = m_pChatList;

				char szMsg[128]{};
				sprintf(szMsg, "[%s]> %s", pPartyItem->GetText(), pStd->String);

				pChatList->AddItem(new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

				m_dwChatTime = g_pTimerManager->GetServerTime();
				return 0;
			}
		}
	}

	return 0;
}

int TMFieldScene::OnPacketMessageChat_Index(MSG_MessageChat* pStd)
{
	if (pStd->String[0] != 1)
		return 0;

	char str[128]{};
	char num[5]{};

	g_pMessageStringTable[*(short*)&pStd->String[2] + 1000][127] = 0;
	g_pMessageStringTable[*(short*)&pStd->String[2] + 1000][126] = 0;

	strcpy(str, g_pMessageStringTable[*(short*)&pStd->String[2] + 1000]);

	if (strlen(str) < 1)
	{
		_itoa(*(short*)&pStd->String[2], num, 10);
		strcpy(str, num);
	}

	if (m_pPartyList->m_nNumItem <= 0)
		return 0;

	if (!g_pObjectManager->GetHumanByID(pStd->Header.ID))
	{
		auto pPartyList = m_pPartyList;
		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
			if (pPartyItem->m_dwCharID == pStd->Header.ID)
			{
				pStd->String[127] = 0;
				pStd->String[126] = 0;

				auto pChatList = m_pChatList;

				char szMsg[128]{};
				sprintf(szMsg, "[%s]> %s", pPartyItem->GetText(), pStd->String);

				pChatList->AddItem(new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

				m_dwChatTime = g_pTimerManager->GetServerTime();
				return 0;
			}
		}
	}

	return 0;
}

int TMFieldScene::OnPacketMessageChat_Param(MSG_STANDARD* pStd)
{
	return 0;
}

/// Trata mensagens privadas (whisper) e respostas de comandos de chat.
int TMFieldScene::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)
{
	if (g_pObjectManager->GetHumanByID(pMsg->Header.ID))
		return 0;

	auto pChatList = m_pChatList;
	pMsg->MobName[15] = 0;
	pMsg->String[127] = 0;
	pMsg->String[126] = 0;

	int nIndex = 0;
	unsigned int dwColor = 0xFFFFFF00;
	bool bDrawText = true;
	
	char szMsg[128]{};
	if (pMsg->String[0] == '-' && pMsg->Color == 3)
	{
		if (m_pChatGuild && !m_pChatGuild->m_bSelected)
			bDrawText = false;

		dwColor = 0xFFAAFFFF;
		nIndex = 1;
		if (pMsg->String[1] == '-')
		{
			dwColor = 0xFF00FFFF;
			nIndex = 2;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);	
	}
	else if (pMsg->Color == 7)
	{
		dwColor = 0xFFBBBBBB;
		nIndex = 0;
		sprintf(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);		
	}
	else if (pMsg->String[0] == '=')
	{
		if (m_pPartyList->m_nNumItem > 1)
		{
			dwColor = 0xFFFF99FF;
			nIndex = 1;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);		
	}
	else if (pMsg->String[0] == '@')
	{
		if (m_pChatParty && !m_pChatParty->m_bSelected)
			bDrawText = false;

		if (pMsg->String[1] == '@')
		{
			dwColor = 0xF0F60AFF;
			nIndex = 2;
		}
		else
		{
			dwColor = 0xFF00AAFF;
			nIndex = 1;
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
	}
	else if (pMsg->String[0] != '!')
	{
		SYSTEMTIME sysTime{};
		GetLocalTime(&sysTime);

		m_pHelpList[3]->AddItem(new SListBoxItem((char*)" ", 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

		sprintf(szMsg, g_pMessageStringTable[226], pMsg->MobName);
		char szTime[128]{};
		sprintf(szTime, "%s [%02d:%02d:%02d]", szMsg, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

		sprintf(szMsg, "%s", &pMsg->String[1]);
		m_pHelpList[3]->AddItem(new SListBoxItem(szMsg, 0xFFFFFFCC, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

		if (m_pHelpMemo)
			m_pHelpMemo->SetVisible(1);

		return 1;
	}

	if (!bDrawText)
		return 1;

	if (strlen(pMsg->MobName) + strlen(pMsg->String) <= 43)
	{
		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);

		if(pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0));
	}
	else
	{
		char szMsg2[128]{};
		char szMsg3[128]{};

		if (IsClearString(pMsg->String, 42))
		{
			strncpy(szMsg3, pMsg->String, 43);
			sprintf(szMsg2, "%s", &pMsg->String[43]);
		}
		else
		{
			strncpy(szMsg3, pMsg->String, 42);
			sprintf(szMsg2, "%s", &pMsg->String[42]);
		}

		sprintf(szMsg, "[%s]> %s", pMsg->MobName, &szMsg3[nIndex]);

		if(pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1, 0));

		if (strlen(pMsg->String) > 43 && pChatList)
			pChatList->AddItem(new SListBoxItem(szMsg2, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777, 1, 0));
	}

	m_dwChatTime = g_pTimerManager->GetServerTime();
	return 1;
}

int TMFieldScene::OnPacketLongMessagePanel(MSG_LongMessagePanel* pMsg)
{
	pMsg->Line[0][127] = 0;
	pMsg->Line[1][127] = 0;
	pMsg->Line[2][127] = 0;
	pMsg->Line[3][127] = 0;

	if (pMsg->Parm1 == 10)
	{
		for (int i = 0; i < 4; ++i)
			strcpy(m_szEventTextTemp[i], pMsg->Line[i]);
		m_dwEventStartTime = 0;

		return 1;
	}
	if (m_pQuizPanel)
	{
		if (!pMsg->Parm1 && m_pQuizCaption)
			m_pQuizCaption->SetText(g_pMessageStringTable[260], 0);
		else if (pMsg->Parm1 == 1 && m_pQuizCaption)
			m_pQuizCaption->SetText(g_pMessageStringTable[259], 0);

		for (int j = 0; j < 4; ++j)
		{
			m_pChatListnotice->AddItem(new SListBoxItem(pMsg->Line[j], 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
			m_pQuizText[j]->SetText(pMsg->Line[j], 0);
		}

		m_dwQuizStart = g_pTimerManager->GetServerTime();
		m_pQuizPanel->SetVisible(1);
		
		GetSoundAndPlay(33, 0, 0);
	}
	return 1;
}

int TMFieldScene::OnPacketReqSummon(MSG_ReqSummon* pStd)
{
	pStd->Name[15] = 0;

	m_pHelpSummon->SetVisible(1);

	sprintf(m_szSummoner, pStd->Name);
	return 1;
}

int TMFieldScene::OnPacketCancelSummon(MSG_STANDARD* pStd)
{
	m_pHelpSummon->SetVisible(0);
	return 1;
}

int TMFieldScene::OnPacketAction(MSG_STANDARD* pStd)
{
	if (g_pObjectManager->GetHumanByID(pStd->ID))
		return 0;
	
	MSG_REQMobByID stReqMobById{};

	stReqMobById.Header.ID = g_pObjectManager->m_dwCharID;
	stReqMobById.Header.Type = MSG_REQMobByID_Opcode;
	stReqMobById.MobID = pStd->ID;
	SendOneMessage((char*)&stReqMobById, sizeof(stReqMobById));
	return 1;
}

int TMFieldScene::OnPacketSoundEffect(MSG_STANDARDPARM* pStd)
{
	int nSoundIndex = pStd->Parm;

	GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
	return 1;
}

/// Cria ou atualiza um personagem/NPC/monstro visivel no campo a partir do pacote do servidor.
int TMFieldScene::OnPacketCreateMob(MSG_STANDARD* pStd)
{
	if (m_pHumanContainer == nullptr)
		return 1;

	auto pCreateMob = (MSG_CreateMob*)pStd;
	auto pCreateMobTrade = (MSG_CreateMobTrade*)pStd;

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pCreateMob->MobID);

	std::string qwe{ pCreateMob->MobName };

	STRUCT_AFFECT tempAffect[32]{};

	TMHuman* pHuman = nullptr;
	if (pCreateMob->MobID == m_pMyHuman->m_dwID)
	{
		for (int l = 0; l < 32; ++l)
			memcpy(&tempAffect[l], &m_pMyHuman->m_stAffect[l], sizeof(STRUCT_AFFECT));

		g_nTempArray[2] = (int)&m_pMyHuman->m_vecPosition;
		g_nTempArray2[2] = (int)&m_pMyHuman->m_vecPosition;
		if (!m_stMoveStop.NextX)
		{
			m_stMoveStop.NextX = pCreateMob->PosX;
			m_stMoveStop.NextY = pCreateMob->PosY;
		}
	}
	if (pStd->Type == MSG_CreateMobTrade_Opcode && pCreateMobTrade->Desc[0])
	{
		int len = strlen(pCreateMobTrade->Desc);
		if (len > 0)
		{
			for (int i = 1; i < 16; ++i)
			{
				pCreateMob->Equip[i] &= 0xFFF;
				pCreateMob->Equip2[i] = 0;
			}

			for (int i = 1; i < 32; ++i)
				pCreateMob->Affect[i] = 0;

			pCreateMob->Equip[0] = 230;
			pCreateMob->Equip[6] = 0;
			pCreateMob->Equip[7] = 0;
			pCreateMob->Equip[14] = 0;
			pCreateMob->Score.Con = 15000;
		}
	}
	if (!pNode)
	{
		pHuman = new TMHuman(this);
		if (pHuman == nullptr)
			return 1;

		pHuman->m_dwID = pCreateMob->MobID;
		pHuman->m_usGuild = pCreateMob->Guild;

		if (pCreateMob->MobID > 0 && pCreateMob->MobID < 1000)
		{
			char cCurrent = pCreateMob->MobName[13];
			short* sTotal = (short*)&pCreateMob->MobName[14];
			pHuman->m_nCurrentKill = (unsigned char)cCurrent;
			pHuman->m_nTotalKill = *sTotal;
			pHuman->m_ucChaosLevel = pCreateMob->MobName[12];
			pCreateMob->MobName[12] = 0;
			pCreateMob->MobName[15] = 0;
			pCreateMob->Nick[25] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
			sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
			pHuman->m_citizen = pCreateMob->Server;
		}
		else
		{
			pHuman->m_nCurrentKill = 0;
			pHuman->m_nTotalKill = 0;
			pHuman->m_ucChaosLevel = 75;
			pCreateMob->MobName[15] = 0;
			pCreateMob->Nick[25] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
			sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
			pHuman->m_citizen = 0;
		}

		memcpy(pHuman->m_usAffect, pCreateMob->Affect, sizeof(pHuman->m_usAffect));

		pHuman->SetPacketEquipItem(pCreateMob->Equip);

		if ((pCreateMob->Equip[14] & 0xFFF) && pCreateMob->Equip2[14] && 
			((pCreateMob->Equip2[14] & 0xFFF)< 3980 || (pCreateMob->Equip2[14] & 0xFFF) >= 3999))
		{
			pHuman->SetMountCostume((unsigned char)pCreateMob->Equip2[14]);
		}

		pHuman->SetColorItem(pCreateMob->Equip2);
		pHuman->m_fMaxSpeed = 2.0f;

		memcpy(&pHuman->m_stScore, &pCreateMob->Score, sizeof(pHuman->m_stScore));

		float fCon = (float)pHuman->m_stScore.Con;
		pHuman->SetCharHeight(fCon);
		if (pCreateMob->Equip[0] < 40)
			pHuman->m_fScale = pHuman->m_fScale * 0.89999998f;

		pHuman->SetRace(pCreateMob->Equip[0] & 0xFFF);

		STRUCT_ITEM tepFace{};
		memcpy(&tepFace, &pCreateMob->Equip[0], sizeof(tepFace));
		tepFace.sIndex = pCreateMob->Equip[0];

		STRUCT_ITEM itemL{};
		itemL.sIndex = pCreateMob->Equip[6] & 0xFFF;
		int nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);

		if (nWeaponTypeL == 41)
		{
			pHuman->m_stLookInfo.RightMesh = pHuman->m_stLookInfo.LeftMesh;
			pHuman->m_stLookInfo.RightSkin = pHuman->m_stLookInfo.LeftSkin;
			pHuman->m_stSancInfo.Sanc6 = pHuman->m_stSancInfo.Sanc7;
			pHuman->m_stSancInfo.Legend6 = pHuman->m_stSancInfo.Legend7;
		}

		pHuman->InitObject(); 
		
		if (pHuman == m_pMyHuman)
		{
			for (int k = 0; k < 32; ++k)
				memcpy(&m_pMyHuman->m_stAffect[k], &tempAffect[k], sizeof(STRUCT_AFFECT));
		}

		pHuman->CheckAffect();
		pHuman->CheckWeapon(pCreateMob->Equip[6] & 0xFFF, pCreateMob->Equip[7] & 0xFFF);

		pHuman->m_sGuildLevel = (unsigned char)pCreateMob->GuildLevel;
		unsigned short usGuild = pCreateMob->Guild;

		if (usGuild)
		{
			int nSubGuild = BASE_GetSubGuild(pHuman->m_sGuildLevel);
			pHuman->m_stGuildMark.nGuild = usGuild & 0xFFF;
			pHuman->m_stGuildMark.nSubGuild = nSubGuild;
			pHuman->m_stGuildMark.nGuildChannel = ((signed int)usGuild >> 12) & 0xF;

			switch (pHuman->m_sGuildLevel)
			{
			case 0:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 1:
				pHuman->m_stGuildMark.sGuildIndex = 535;
				break;
			case 2:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 3:
				pHuman->m_stGuildMark.sGuildIndex = 526;
				break;
			case 4:
				pHuman->m_stGuildMark.sGuildIndex = 527;
				break;
			case 5:
				pHuman->m_stGuildMark.sGuildIndex = 528;
				break;
			case 6:
				pHuman->m_stGuildMark.sGuildIndex = 529;
				break;
			case 7:
				pHuman->m_stGuildMark.sGuildIndex = 530;
				break;
			case 8:
				pHuman->m_stGuildMark.sGuildIndex = 531;
				break;
			case 9:
				pHuman->m_stGuildMark.sGuildIndex = 509;
				break;
			}

			if (pStd->Type != MSG_CreateMobTrade_Opcode && !pHuman->m_pAutoTradeDesc->IsVisible())
				Guildmark_Create(&pHuman->m_stGuildMark);
		}

		float fAngle = 0.0f;
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Merchant >> 4);
		if (nDir == 6)
			fAngle = D3DXToRadian(180);
		if (nDir == 9)
			fAngle = D3DXToRadian(135);
		if (nDir == 8)
			fAngle = D3DXToRadian(90);
		if (nDir == 7)
			fAngle = D3DXToRadian(45);
		if (nDir == 4)
			fAngle = D3DXToRadian(0);
		if (nDir == 1)
			fAngle = D3DXToRadian(315);
		if (nDir == 2)
			fAngle = D3DXToRadian(270);
		if (nDir == 3)
			fAngle = D3DXToRadian(225);

		pHuman->InitAngle(0.0f, fAngle, 0.0f);

		TMVector2 vecPosition{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };
				
		pHuman->InitPosition(vecPosition.x, GroundGetMask(vecPosition) * 0.1f, vecPosition.y);

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Merchant & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->IsMerchant() || (pHuman->m_stScore.Merchant & 0xF) == 15))
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		int nItemCode = pCreateMob->Equip[13] & 0xFFF;
		int nHP = 0;
		if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
		{
			int nHp = 0;
			int nSanc = (int)pCreateMob->Equip[13] >> 12;
			if (nSanc < 2)
				nSanc = 2;
			if (nItemCode == 1936)
			{
				nSanc *= 10;
			}
			else if (nItemCode == 1937)
			{
				nSanc *= 1000;
			}
			
			int tmp = pCreateMob->Score.Hp * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;

			pHuman->m_BigHp = tmp;
			tmp = pCreateMob->Score.MaxHp * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;
			pHuman->m_MaxBigHp = tmp;
			pHuman->m_usHP = pHuman->m_MaxBigHp;
		}

		if (pHuman->m_nClass == 56 && !pHuman->m_stLookInfo.FaceMesh)
			m_dwKhepraID = pHuman->m_dwID;

		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) &&
			(int)vecPosition.x >> 7 >= 28 && (int)vecPosition.x >> 7 <= 30 && 
			(int)vecPosition.y >> 7 >= 27 && (int)vecPosition.y >> 7 <= 28)
		{
			pHuman->SetInMiniMap(0xAAFF0000);
		}

		m_pHumanContainer->AddChild(pHuman);
	}
	else if (pNode->m_nWillDie)
	{
		unsigned int dwOldMoveTime = pNode->m_dwOldMovePacketTime;
		pNode->m_nWillDie = -1;
		pNode->m_dwDeadTime = 0;

		pHuman = pNode;
		pHuman->Init();

		pHuman->m_dwID = pCreateMob->MobID;
		pHuman->m_usGuild = pCreateMob->Guild;

		if (pCreateMob->MobID > 0 && pCreateMob->MobID < 1000)
		{
			char cCurrent = pCreateMob->MobName[13];
			short* sTotal = (short*)&pCreateMob->MobName[14];
			pHuman->m_nCurrentKill = (unsigned char)cCurrent;
			pHuman->m_nTotalKill = *sTotal;
			pHuman->m_ucChaosLevel = pCreateMob->MobName[12];
			pCreateMob->MobName[12] = 0;
			pCreateMob->MobName[15] = 0;
			pCreateMob->MobName[14] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);
		}
		else
		{
			pHuman->m_nCurrentKill = 0;
			pHuman->m_nTotalKill = 0;
			pHuman->m_ucChaosLevel = 75;
			pCreateMob->MobName[15] = 0;
			sprintf(pHuman->m_szName, "%s", pCreateMob->MobName);

			int nItemCode = pCreateMob->Equip[13] & 0xFFF;
			int nHP = 0;
			if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
			{
				int nHp = 0;
				int nSanc = (int)pCreateMob->Equip[13] >> 12;
				if (nSanc < 2)
					nSanc = 2;
				if (nItemCode == 1936)
				{
					nSanc *= 10;
				}
				else if (nItemCode == 1937)
				{
					nSanc *= 1000;
				}

				int tmp = pCreateMob->Score.Hp * nSanc;
				if (tmp > 2000000000)
					tmp = 2000000000;

				pHuman->m_BigHp = tmp;
				tmp = pCreateMob->Score.MaxHp * nSanc;
				if (tmp > 2000000000)
					tmp = 2000000000;
				pHuman->m_MaxBigHp = tmp;
				pHuman->m_usHP = pHuman->m_MaxBigHp;
			}
		}

		pCreateMob->Nick[25] = 0;

		sprintf(pHuman->m_szNickName, "%s", pCreateMob->Nick);
		memcpy(pHuman->m_usAffect, pCreateMob->Affect, sizeof(pHuman->m_usAffect));

		pHuman->SetPacketEquipItem(pCreateMob->Equip);

		if ((pCreateMob->Equip[14] & 0xFFF) && pCreateMob->Equip2[14] &&
			((pCreateMob->Equip2[14] & 0xFFF) < 3980 || (pCreateMob->Equip2[14] & 0xFFF) >= 3999))
		{
			pHuman->SetMountCostume((unsigned char)pCreateMob->Equip2[14]);
		}

		pHuman->SetColorItem(pCreateMob->Equip2);
		pHuman->m_fMaxSpeed = 2.0f;

		memcpy(&pHuman->m_stScore, &pCreateMob->Score, sizeof(pHuman->m_stScore));

		float fCon = (float)pHuman->m_stScore.Con;
		pHuman->SetCharHeight(fCon);

		int SkinMeshType = pHuman->m_nSkinMeshType;
		bool bWasMobTrade = false;
		if (pHuman->m_nClass == 29)
			bWasMobTrade = true;

		pHuman->SetRace(pCreateMob->Equip[0] & 0xFFF);

		if (pStd->Type != MSG_CreateMobTrade_Opcode && !bWasMobTrade)
			pHuman->m_nSkinMeshType = SkinMeshType;

		STRUCT_ITEM itemL{};
		itemL.sIndex = pCreateMob->Equip[6] & 0xFFF;
		int nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);

		if (nWeaponTypeL == 41)
		{
			pHuman->m_stLookInfo.RightMesh = pHuman->m_stLookInfo.LeftMesh;
			pHuman->m_stLookInfo.RightSkin = pHuman->m_stLookInfo.LeftSkin;
			pHuman->m_stSancInfo.Sanc6 = pHuman->m_stSancInfo.Sanc7;
			pHuman->m_stSancInfo.Legend6 = pHuman->m_stSancInfo.Legend7;
		}
		if (pHuman == m_pMyHuman)
		{
			for (int k = 0; k < 32; ++k)
				memcpy(&m_pMyHuman->m_stAffect[k], &tempAffect[k], sizeof(STRUCT_AFFECT));
		}


		pHuman->CheckAffect();
		pHuman->CheckWeapon(pCreateMob->Equip[6] & 0xFFF, pCreateMob->Equip[7] & 0xFFF);
		pHuman->InitObject();

		float fAngle = 0.0f;
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Merchant >> 4);
		if (nDir == 6)
			fAngle = D3DXToRadian(180);
		if (nDir == 9)
			fAngle = D3DXToRadian(135);
		if (nDir == 8)
			fAngle = D3DXToRadian(90);
		if (nDir == 7)
			fAngle = D3DXToRadian(45);
		if (nDir == 4)
			fAngle = D3DXToRadian(0);
		if (nDir == 1)
			fAngle = D3DXToRadian(315);
		if (nDir == 2)
			fAngle = D3DXToRadian(270);
		if (nDir == 3)
			fAngle = D3DXToRadian(225);

		pHuman->InitAngle(0.0f, fAngle, 0.0f);

		TMVector2 vecPosition{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };

		pHuman->InitPosition(vecPosition.x, GroundGetMask(vecPosition) * 0.1f, vecPosition.y);

		pHuman->m_sGuildLevel = (unsigned char)pCreateMob->GuildLevel;
		unsigned short usGuild = pCreateMob->Guild;

		if (usGuild)
		{
			int nSubGuild = BASE_GetSubGuild(pHuman->m_sGuildLevel);
			pHuman->m_stGuildMark.nGuild = usGuild & 0xFFF;
			pHuman->m_stGuildMark.nSubGuild = nSubGuild;
			pHuman->m_stGuildMark.nGuildChannel = ((signed int)usGuild >> 12) & 0xF;

			switch (pHuman->m_sGuildLevel)
			{
			case 0:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 1:
				pHuman->m_stGuildMark.sGuildIndex = 535;
				break;
			case 2:
				pHuman->m_stGuildMark.sGuildIndex = 508;
				break;
			case 3:
				pHuman->m_stGuildMark.sGuildIndex = 526;
				break;
			case 4:
				pHuman->m_stGuildMark.sGuildIndex = 527;
				break;
			case 5:
				pHuman->m_stGuildMark.sGuildIndex = 528;
				break;
			case 6:
				pHuman->m_stGuildMark.sGuildIndex = 529;
				break;
			case 7:
				pHuman->m_stGuildMark.sGuildIndex = 530;
				break;
			case 8:
				pHuman->m_stGuildMark.sGuildIndex = 531;
				break;
			case 9:
				pHuman->m_stGuildMark.sGuildIndex = 509;
				break;
			}

			if (pStd->Type != MSG_CreateMobTrade_Opcode && !pHuman->m_pAutoTradeDesc->IsVisible())
				Guildmark_Create(&pHuman->m_stGuildMark);
		}

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Merchant & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->m_stScore.Merchant & 0xF) >= 1 && (pHuman->m_stScore.Merchant & 0xF) <= 15)
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && pHuman->m_sHeadIndex == 54)
			pHuman->m_pNameLabel->SetTextColor(0xFFAAFFAA);

		if (pHuman == m_pMyHuman)
			UpdateScoreUI(0);

		pNode->m_dwOldMovePacketTime = dwOldMoveTime;
	}
	else
	{
		pNode->m_nWillDie = -1;
		pNode->m_dwDeadTime = 0;

		int nItemCode = pCreateMob->Equip[13] & 0xFFF;
		int nHP = 0;
		if (nItemCode == 786 || nItemCode == 1936 || nItemCode == 1937)
		{
			int nHp = 0;
			int nSanc = (int)pCreateMob->Equip[13] >> 12;
			if (nSanc < 2)
				nSanc = 2;
			if (nItemCode == 1936)
			{
				nSanc *= 10;
			}
			else if (nItemCode == 1937)
			{
				nSanc *= 1000;
			}

			int tmp = pCreateMob->Score.Hp * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;

			pNode->m_BigHp = tmp;
			tmp = pCreateMob->Score.MaxHp * nSanc;
			if (tmp > 2000000000)
				tmp = 2000000000;
			pNode->m_MaxBigHp = tmp;
			pNode->m_usHP = pNode->m_MaxBigHp;
			pNode->UpdateScore(0);
		}
	}
	if (pHuman)
	{
		pHuman->m_pNameLabel->m_GCBorder.dwColor = 0x55AA0000;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && !pHuman->m_stScore.Ac)
		{
			pHuman->m_pNameLabel->m_GCBorder.dwColor = 0x5500AA00;
			pHuman->m_pNameLabel->m_cBorder = 1;
			pHuman->m_cSummons = 1;
		}
		if (pStd->Type == MSG_CreateMobTrade_Opcode)
		{
			pCreateMobTrade->Desc[22] = 0;
			pCreateMobTrade->Desc[21] = 0;

			sprintf(pHuman->m_TradeDesc, pCreateMobTrade->Desc);
			pHuman->m_pAutoTradeDesc->SetText(pHuman->m_TradeDesc, 0);
		}
		else
		{
			memset(pHuman->m_TradeDesc, 0, sizeof(pHuman->m_TradeDesc));
			pHuman->m_pAutoTradeDesc->SetText((char*)"", 0);

			auto pPanel = m_pAutoTrade;
			if (pPanel && pPanel->IsVisible() == 1 && pHuman->m_dwID == m_stAutoTrade.TargetID)
			{
				SetVisibleAutoTrade(0, 0);
			}
		}
	} 
	
	if (pHuman && !pHuman->m_cHide)
	{
		if (!pHuman->m_cHide && ((pCreateMob->CreateType & 0x7FFF) == 2 || (pCreateMob->CreateType & 0x7FFF) == 3))
		{
			TMVector3 vecEffectPos = TMVector3((float)pCreateMob->PosX + 0.5f, 
				GroundGetMask(pHuman->m_vecPosition) * 0.1f + 0.05000000f,
				(float)pCreateMob->PosY + 0.5f);

			if (!pHuman->m_nSkinMeshType || pHuman->m_nSkinMeshType == 1)
			{
				auto pEffect = new TMEffectStart(vecEffectPos, 0, nullptr);

				if (pEffect && m_pEffectContainer)
					m_pEffectContainer->AddChild(pEffect);
			}
			else if ((pCreateMob->CreateType & 0x7FFF) != 3 && pHuman->m_nSkinMeshType != 35 && pHuman->m_nSkinMeshType != 36)
			{
				auto pChild = new TMEffectStart(vecEffectPos, 1, nullptr);

				if (pChild && m_pEffectContainer)
					m_pEffectContainer->AddChild(pChild);

				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && m_pMyHuman == pHuman)
				{
					auto pSoundData = pSoundManager->GetSoundData(151);
					pSoundData->Play();
				}
			}
			if ((pCreateMob->CreateType & 0x7FFF) == 3)
			{
				if (pHuman->m_nClass == 62 && pHuman->m_stLookInfo.FaceMesh == 2)
				{
					TMVector2 effectPos{ (float)pCreateMob->PosX + 0.5f, (float)pCreateMob->PosY + 0.5f };
					pHuman->InitPosition(effectPos.x,
						(GroundGetMask(effectPos) * 0.1f) - 2.0f,
						effectPos.y);

					TMVector3 vecPos = TMVector3(effectPos.x, (GroundGetMask(effectPos) * 0.1f) + 0.2f, effectPos.y);
					auto pJudgement = new TMSkillJudgement(vecPos, 4, 0.1f);

					if (pJudgement && m_pEffectContainer)
						m_pEffectContainer->AddChild(pJudgement);
				}
				else
				{
					int nType = 1;
					if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000))
						nType = 3;

					auto pPortal = new TMSkillTownPortal(vecEffectPos, nType);

					if (pPortal && m_pEffectContainer)
						m_pEffectContainer->AddChild(pPortal);
				}
			}
			pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

			if (pHuman->m_nSkinMeshType == 35 || pHuman->m_nSkinMeshType == 36)
			{
				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && m_pMyHuman == pHuman)
				{
					auto pSoundData = pSoundManager->GetSoundData(303);
					pSoundData->Play();
				}

				if (!g_bHideEffect)
				{
					for (int m = -3; m < 3; ++m)
					{
						auto pBillEffect = new TMEffectBillBoard(193, 4000, 1.0f, 1.0f, 1.0f, 0.001f, 1, 80);

						if (pBillEffect)
						{
							pBillEffect->m_bStickGround = m % 2;
							pBillEffect->m_vecPosition = TMVector3(((float)m * 0.5f) + pHuman->m_vecPosition.x,
								pHuman->m_fHeight,
								((float)m * 0.5f) + pHuman->m_vecPosition.y);

							m_pEffectContainer->AddChild(pBillEffect);
						}

						auto pBillEffect2 = new TMEffectBillBoard(193, 4000, 1.0f, 1.0f, 1.0f, 0.001f, 1, 80);
						if (pBillEffect2)
						{
							pBillEffect2->m_bStickGround = m % 2;
							pBillEffect2->m_vecPosition = TMVector3(pHuman->m_vecPosition.x - ((float)m * 0.5f),
								pHuman->m_fHeight,
								pHuman->m_vecPosition.y - ((float)m * 0.5f));

							m_pEffectContainer->AddChild(pBillEffect2);
						}
					}
				}
			}
			else
			{
				auto pLevelUp = new TMEffectLevelUp(vecEffectPos, 0);

				if (pLevelUp)
					m_pEffectContainer->AddChild(pLevelUp);
			}
		}
		if (!pHuman->m_nSkinMeshType || pHuman->m_nSkinMeshType == 1)
		{
			if ((pCreateMob->CreateType & 0xF0) == 16)
				pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_PUNISHING, 1);
			else if ((pCreateMob->CreateType & 0xF0) == 32)
				pHuman->SetAnimation(ECHAR_MOTION::ECMOTION_SEATING, 1);
		}
	}
	if (pHuman == m_pMyHuman)
	{
		m_vecMyNext.x = (int)m_pMyHuman->m_vecPosition.x;
		m_vecMyNext.y = (int)m_pMyHuman->m_vecPosition.y;
		Bag_View();
	}

	auto pPartyList = m_pPartyList;
	if (pPartyList)
	{
		for (int n = 0; n < pPartyList->m_nNumItem; ++n)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[n];
			if (pPartyItem->m_dwCharID == pCreateMob->MobID)
			{
				auto pPartyHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
				if (pPartyHuman && pPartyItem->m_nState != 1)
				{
					pPartyHuman->m_bParty = 1;
					pPartyHuman->SetInMiniMap(0xAAFFFF00);
				}
				if (pPartyItem->m_nState == 4)
				{
					pPartyItem->m_nState = 2;
					pPartyItem->m_GCText.dwColor = 0x0FFAAAAFF;
					pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,
						pPartyItem->m_GCText.dwColor,
						0);
				}
				else if (pPartyItem->m_nState == 3)
				{
					pPartyItem->m_nState = 0;
					pPartyItem->m_GCText.dwColor = 0x0FFFFFFFF;
					pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,
						pPartyItem->m_GCText.dwColor,
						0);
				}
				break;
			}
		}
	}

	if (pHuman && 
		(_locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 8, 15) || _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 8, 16)	|| 
		 _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 9, 15) || _locationCheck((float)pCreateMob->PosX, (float)pCreateMob->PosY, 9, 16)))
	{
		if (pHuman->m_cMantua == 1)
			pHuman->SetInMiniMap(0xAA0000FF);
		if (pHuman->m_cMantua == 2)
			pHuman->SetInMiniMap(0xAAFF0000);
	}

	return 1;
}

int TMFieldScene::OnPacketCNFCharacterLogout(MSG_STANDARD* pStd)
{
	if (pStd->ID == g_pObjectManager->m_dwCharID)
	{
		g_pDevice->m_nWidthShift = 0;

		int nSlot = g_pObjectManager->m_cCharacterSlot;
		auto pSelChar = &g_pObjectManager->m_stSelCharData;

		memcpy(&g_pObjectManager->m_stSelCharData.Score[nSlot], &m_pMyHuman->m_stScore, sizeof(m_pMyHuman->m_stScore));
		memcpy(pSelChar->Equip[nSlot], g_pObjectManager->m_stMobData.Equip, sizeof(g_pObjectManager->m_stMobData.Equip));

		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTCHAR_STATE);
	}

	return 1;
}

int TMFieldScene::OnPacketCNFRemoveServer(MSG_CNFRemoveServer* pStd)
{
	if (pStd->Header.ID != g_pObjectManager->m_dwCharID || g_pSocketManager->Sock)
	{
		if (pStd->Header.ID == g_pObjectManager->m_dwCharID && g_pSocketManager->Sock)
		{
			memcpy(&m_stRemoveServer, pStd, sizeof(m_stRemoveServer));
			m_bMsgRemoveServer = 1;
		}
		return 1;
	}

	m_pMessagePanel->SetMessage(g_pMessageStringTable[7], 0);
	m_pMessagePanel->SetVisible(1, 0);

	g_bMoveServer = 0;
	int nServer = 0;

	sscanf(pStd->TID, "*%d", &nServer);
	g_pObjectManager->m_nServerIndex = nServer;
	CheckPKNonePK(g_pObjectManager->m_nServerIndex);
	sprintf(g_pApp->m_szServerIP, "%s", g_pServerList[g_pObjectManager->m_nServerGroupIndex][nServer]);

	if (g_pSocketManager->ConnectServer(g_pApp->m_szServerIP, TM_CONNECTION_PORT, 0, 1124))
	{
		MSG_AccountLogin stAccountLogin{};
		stAccountLogin.Header.ID = 0;
		stAccountLogin.Header.Type = MSG_AccountLogin_Opcode;
		stAccountLogin.Version = 1758;
		stAccountLogin.Force = 1;

		ULONG dwSize = 0;
		IP_ADAPTER_INFO stInfo{};
		GetAdaptersInfo(&stInfo, &dwSize);
		if (dwSize)
		{
			PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
			GetAdaptersInfo(pInfo, &dwSize);

			char* sour = pInfo->AdapterName;
			int tpos = 0;
			int grid = 0;
			char temp[256]{};
			for (size_t i = 0; i < strlen(pInfo->AdapterName); ++i)
			{
				if (sour[i] != '{' && sour[i] != '}' && sour[i] != '-')
				{
					temp[tpos++] = sour[i];
					if (!(++grid % 8))
						temp[tpos++] = 32;
				}
			}

			temp[tpos] = 0;
			sscanf(temp, "%x %x %x %x",	stAccountLogin.Mac,	&stAccountLogin.Mac[1],	&stAccountLogin.Mac[2],	&stAccountLogin.Mac[3]);
			free(pInfo);
		}

		strncpy(stAccountLogin.AccountName, pStd->AccountName, sizeof(pStd->AccountName));
		strncpy(stAccountLogin.TID, pStd->TID, sizeof(pStd->TID));
		sprintf(stAccountLogin.AccountPass, "");
		SendOneMessage((char*)&stAccountLogin, sizeof(stAccountLogin));
		return 1;
	}

	m_pMessagePanel->SetMessage(g_pMessageStringTable[8], 4000);
	m_pMessagePanel->SetVisible(1, 1);
	if (m_eSceneType != ESCENE_TYPE::ESCENE_LOGIN)
		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
	return 1;
}

int TMFieldScene::OnPacketCNFAccountLogin(MSG_CNFRemoveServerLogin* pStd)
{
	memcpy(&g_pObjectManager->m_stSelCharData, &pStd->SelChar, sizeof(pStd->SelChar));
	memcpy(g_pObjectManager->m_stItemCargo, pStd->Cargo, sizeof(pStd->Cargo));
	g_pObjectManager->m_nCargoCoin = pStd->Coin;
	memset(g_pObjectManager->m_stMemo, 0, sizeof(g_pObjectManager->m_stMemo));

	/*for (int i = 0; i < 16; ++i)
		g_pSocketManager->SendQueue[i] = *((unsigned char*)&pStd->Tick + i + 4);*/

	g_pSocketManager->SendCount = 0;
	g_pSocketManager->RecvCount = 0;
	return 1;
}

int TMFieldScene::OnPacketCNFCharacterLogin(MSG_CNFCharacterLogin* pStd)
{
	m_pMessagePanel->SetVisible(0, 1);
	g_pTimerManager->SetServerTime(pStd->Header.Tick);
	g_pObjectManager->m_dwCharID = pStd->ClientID;
	memcpy(&g_pObjectManager->m_stMobData, &pStd->MOB, sizeof(pStd->MOB));
	g_pObjectManager->m_nFakeExp = pStd->Ext1.Data[0];
	g_pObjectManager->m_stMobData.HomeTownX = pStd->PosX;
	g_pObjectManager->m_stMobData.HomeTownY = pStd->PosY;
	memcpy(g_pObjectManager->m_cShortSkill, g_pObjectManager->m_stMobData.ShortSkill, sizeof(g_pObjectManager->m_stMobData.ShortSkill));

	memcpy(&g_pObjectManager->m_cShortSkill[4], pStd->ShortSkill, sizeof(pStd->ShortSkill));
	for (int i = 0; i < 20; ++i)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
			g_pObjectManager->m_cShortSkill[i] += 24 * g_pObjectManager->m_stMobData.Class;
	}

	g_nWeather = pStd->Weather;

	m_pHPBar->ResetBar();
	m_pMPBar->ResetBar();
	if (m_pMHPBar)
		m_pMHPBar->ResetBar();
	if (m_pMHPBarT)
		m_pMHPBarT->ResetBar();

	g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_FIELD2_STATE);
	return 1;
}

int TMFieldScene::OnPacketWeather(MSG_STANDARDPARM* pStd)
{
	SetWeather(pStd->Parm);
	return 1;
}

int TMFieldScene::OnPacketCNFMobKill(MSG_CNFMobKill* pStd)
{
	auto pAttacker = g_pObjectManager->GetHumanByID(pStd->Killer);
	if (pAttacker)
	{
		if (pAttacker->m_bParty == 1 || m_pMyHuman == pAttacker)
		{
			SetMyHumanExp(pStd->Exp, pStd->FakeExp);
			if (m_cAutoAttack == 1 && m_pMyHuman == pAttacker)
				m_pTargetHuman = 0;
		}

		m_pEffectContainer->AddChild(new TMEffectCharge(pAttacker, 0, 0xFFFFFFFF));
	}

	auto pKilled = g_pObjectManager->GetHumanByID(pStd->KilledMob);
	if (pKilled)
	{
		pKilled->m_stScore.Hp = 0;
		pKilled->Die();
	}

	if (m_pMyHuman == pKilled)
	{
		bool bFind = false;
		for (int i = 0; i < 4; ++i)
		{
			auto pGridInv = m_pGridInvList[i];
			for (int nY = 0; nY < 3; ++nY)
			{
				for (int nX = 0; nX < 5; ++nX)
				{
					auto pItem = pGridInv->GetItem(nX, nY);
					if (pItem && pItem->m_pItem->sIndex == 3463)
					{
						bFind = true;
						break;
					}
				}
				if (bFind == true)
					break;
			}

			if (bFind == true)
				break;
		}

		if (bFind)
		{
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);

			char szTime[128]{};

			sprintf(szTime,	"[%02d:%02d:%02d] Killer[%s] ",	sysTime.wHour, sysTime.wMinute,	sysTime.wSecond, pAttacker->m_szName);

			m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

			if (m_pHelpMemo)
				m_pHelpMemo->SetVisible(1);
		}
	}

	return 1;
}

int TMFieldScene::OnPacketREQParty(MSG_REQParty* pStd)
{
	auto pPartyList = m_pPartyList;
	pStd->Leader.Name[15] = 0;

	auto pPartyItem = new SListBoxPartyItem(pStd->Leader.Name,
		0xFFFFFFFF,
		0.0f,
		0.0f,
		104.0f,
		20.0f,
		pStd->Leader.ID,
		pStd->Leader.Class,
		pStd->Leader.Level,
		pStd->Leader.Hp,
		pStd->Leader.MaxHp);

	if (!pStd->Leader.PartyIndex)
		pPartyItem->m_nState = 1;
	if (pPartyList->m_nNumItem > 0)
		pPartyList->Empty();

	pPartyList->AddItem(pPartyItem);

	if (m_pPartyPanel)
		m_pPartyPanel->SetVisible(1);
	if (m_pPartyBtn)
		m_pPartyBtn->m_bSelected = 0;

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Leader.ID);
	if (pNode)
		pNode->SetInMiniMap(0xAAFFFF00);
	if (!m_pPartyPanel->IsVisible())
		SetVisibleParty();

	auto pChatList = m_pChatList;

	char szMsg[128]{};
	sprintf(szMsg, g_pMessageStringTable[62], pStd->Leader.Name);

	pChatList->AddItem(new SListBoxItem(szMsg, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));

	if (!m_bAutoParty)
	{
		sprintf(szMsg, g_pMessageStringTable[63], pStd->Leader.Name);

		pChatList->AddItem(new SListBoxItem(szMsg, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		m_pPartyAutoButton->SetVisible(0);
		m_pPartyAutoText->SetVisible(0);

		auto partyback = (SPanel*)m_pControlContainer->FindControl(7602196);
		if (partyback)
			partyback->SetVisible(0);
	}
	else
	{
		MSG_CNFParty2 stCnfParty{};

		stCnfParty.Header.ID = m_pMyHuman->m_dwID;
		stCnfParty.Header.Type = MSG_CNFParty2_Opcode;
		stCnfParty.LeaderID = pStd->Leader.ID;
		sprintf(stCnfParty.LeaderName, pStd->Leader.Name);
		
		if (pNode)
			pNode->m_bParty = 1;

		SendOneMessage((char*)&stCnfParty, sizeof(stCnfParty));
	}

	m_dwChatTime = g_pTimerManager->GetServerTime();

	GetSoundAndPlay(33, 0, 0);

	return 1;
}

int TMFieldScene::OnPacketAddParty(MSG_AddParty* pStd)
{
	auto pPartyList = m_pPartyList;
	pStd->Party.Name[15] = 0;
	for (int i = 0; i < pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
		if (pPartyItem->m_dwCharID == pStd->Party.ID)
		{
			m_pPartyList->DeleteItem(pPartyItem);
			break;
		}
	}

	unsigned int dwColor = 0xFFFFFFFF;
	if (!pStd->Party.PartyIndex)
		dwColor = 0xFFAAAAFF;

	auto pPartyItem = new SListBoxPartyItem(pStd->Party.Name,
		dwColor,
		0.0f,
		0.0f,
		114.0f,
		20.0f,
		pStd->Party.ID,
		pStd->Party.Class,
		pStd->Party.Level,
		pStd->Party.Hp,
		pStd->Party.MaxHp);

	if (!pStd->Party.PartyIndex)
		pPartyItem->m_nState = 2;

	pPartyList->AddItem(pPartyItem);

	auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Party.ID);
	if (pNode)
	{
		pNode->m_bParty = 1;
		pNode->SetInMiniMap(0xAAFFFF00);
	}
	if (!m_pPartyPanel->IsVisible())
	{
		if (pStd->Party.ID > 0 && pStd->Party.ID < 1000)
			SetVisibleParty();
	}

	return 1;
}

int TMFieldScene::OnPacketRemoveParty(MSG_STANDARDPARM* pStd)
{
	if (!m_pPartyList)
		return 0;

	auto pPartyList = m_pPartyList;

	if (!pStd->Parm)
	{
		m_pMyHuman->m_bParty = 0;

		for (int i = 0; i < pPartyList->m_nNumItem; ++i)
		{			
			auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(static_cast<SListBoxPartyItem*>(pPartyList->m_pItemList[i])->m_dwCharID);
			if (pNode)
			{
				pNode->m_bParty = 0;
				if (pNode)
				{
					if (_locationCheck(pNode->m_vecFromPos, 8, 15) || _locationCheck(pNode->m_vecFromPos, 8, 16) || 
						_locationCheck(pNode->m_vecFromPos, 9, 15) || _locationCheck(pNode->m_vecFromPos, 9, 16))
					{
						if (pNode->m_cMantua == 1)
							pNode->SetInMiniMap(0xAA0000FF);
						if (pNode->m_cMantua == 2)
							pNode->SetInMiniMap(0xAAFF0000);
					}
					else if (pNode->m_pInMiniMap)
						SAFE_DELETE(pNode->m_pInMiniMap);
				}
			}
		}

		pPartyList->Empty();
	}
	else
	{
		for (int inItemIndex = 0; inItemIndex < pPartyList->m_nNumItem; ++inItemIndex)
		{
			auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[inItemIndex];
			if (pPartyItem->m_dwCharID == pStd->Parm)
			{
				auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
				if (pNode)
				{
					pNode->m_bParty = 0;
					if (_locationCheck(pNode->m_vecFromPos, 8, 15) || _locationCheck(pNode->m_vecFromPos, 8, 16) ||
						_locationCheck(pNode->m_vecFromPos, 9, 15) || _locationCheck(pNode->m_vecFromPos, 9, 16))
					{
						if (pNode->m_cMantua == 1)
							pNode->SetInMiniMap(0xAA0000FF);
						if (pNode->m_cMantua == 2)
							pNode->SetInMiniMap(0xAAFF0000);
					}
					else if (pNode->m_pInMiniMap)
						SAFE_DELETE(pNode->m_pInMiniMap);
				}

				pPartyList->DeleteItem(inItemIndex);
				break;
			}
		}
	}

	if (pPartyList->m_nNumItem == 1)
	{
		pPartyList->Empty();
		m_pMyHuman->m_bParty = 0;
	}

	return 1;
}

int TMFieldScene::OnPacketSetHpMode(MSG_SetHpMode* pStd)
{
	if (pStd->Mode / 10 == 1)
	{
		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTCHAR_STATE);
		return 1;
	}
	else if (pStd->Mode != 22)
	{
		if (!m_pMessagePanel->m_bVisible)
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[13], 2000);
			m_pMessagePanel->SetVisible(1, 1);
		}

		g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
		return 1;
	}
	else if (pStd->Mode == 22)
	{
		if (m_pMyHuman->m_stScore.Level < 1000)
			m_pMyHuman->Die();

		return 1;
	}
	
	return 1;
}

int TMFieldScene::OnPacketReqChallange(MSG_STANDARD* pStd)
{
	if (!m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[407], 60, 0);
		m_pMessageBox->SetVisible(1);
	}

	return 1;
}

int TMFieldScene::OnPacketSetShortSkill(MSG_SetShortSkill* pStd)
{	
	memcpy(g_pObjectManager->m_cShortSkill, pStd->Skill, sizeof(pStd->Skill));
	for (int i = 0; i < 20; ++i)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 105 || (unsigned char)g_pObjectManager->m_cShortSkill[i] >= 117)
		{
			if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
				g_pObjectManager->m_cShortSkill[i] += 24 * g_pObjectManager->m_stMobData.Class;
		}
		else
		{
			g_pObjectManager->m_cShortSkill[i] += 12 * g_pObjectManager->m_stMobData.Class;
		}
	}

	UpdateSkillBelt();
	return 1;
}

int TMFieldScene::OnPacketClearMenu(MSG_STANDARD* pStd)
{
	SetVisibleCargo(0);
	return 1;
}

int TMFieldScene::OnPacketCastleState(MSG_STANDARDPARM* pStd)
{
	g_bCastleWar2 = pStd->Parm;

	DS_SOUND_MANAGER::m_nCastleIndex = -1;
	if (!g_bCastleWar2)
	{
		DS_SOUND_MANAGER::m_nMusicIndex = -1;
		DS_SOUND_MANAGER::m_nCastleIndex = -1;
	}

	return 1;
}

int TMFieldScene::OnPacketStartTime(MSG_STANDARDPARM* pStd)
{
	m_nLastTime = pStd->Parm;

	m_dwStartRankTime = g_pTimerManager->GetServerTime();

	char szTimer[128]{};
	sprintf(szTimer, "%d", m_dwStartRankTime);
	m_pRankTimeText->SetText(szTimer, 0);
	m_bRankTimeOn = 1;
	return 1;
}

int TMFieldScene::OnPacketRemainCount(MSG_STANDARDPARM* pStd)
{
	char szText[128]{};
	sprintf(szText, "%s %d", g_pMessageStringTable[230], pStd->Parm);
	m_dwRemainTime = g_pTimerManager->GetServerTime();
	m_pRemainText->SetText(szText, 0);
	m_pRemainText->SetVisible(1);
	return 1;
}

int TMFieldScene::OnPacketWarInfo(MSG_STANDARDPARM3* pStd)
{
	switch (pStd->Header.Size)
	{
	case sizeof(MSG_STANDARDPARM):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		break;
	case sizeof(MSG_STANDARDPARM2):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		m_cWarClan = pStd->Parm2;
		break;
	case sizeof(MSG_STANDARDPARM3):
		g_pObjectManager->m_usWarGuild = pStd->Parm1;
		if (!pStd->Parm1)
			g_pObjectManager->m_usWarGuild = -1;
		m_cWarClan = pStd->Parm2;
		g_pObjectManager->m_usAllyGuild = pStd->Parm3;
		break;
	}

	return 1;
}

int TMFieldScene::OnPacketGuildDisable(MSG_STANDARDPARM* pStd)
{
	if (m_pBtnGuildOnOff)
		m_pBtnGuildOnOff->SetSelected(pStd->Parm == 1 ? 1 : 0);

	return 1;
}

int TMFieldScene::OnPacketEnvEffect(MSG_STANDARD* pStd)
{
	auto pEnvEffect = reinterpret_cast<MSG_EnvEffect*>(pStd);
	if (pEnvEffect->x1 > pEnvEffect->x2 || pEnvEffect->y1 > pEnvEffect->y2)
		return 1;

	for (int nY = pEnvEffect->y1; nY < pEnvEffect->y2; nY += 3)
	{
		for (int nX = pEnvEffect->x1 + 1; nX < pEnvEffect->x2; nX += 4)
		{
			unsigned int dwColor = 0x44444444;

			TMVector2 vec{ (float)nX, (float)nY + 1.0f };
			float fHeight = (float)GroundGetMask(vec) * 0.1f;
			if (pEnvEffect->Effect == 32)
			{		
				auto pExplosion = new TMSkillExplosion2(TMVector3(vec.x, fHeight, vec.y), 0, 1.5f, 210, dwColor);
				m_pEffectContainer->AddChild(pExplosion);
			}
		}
	}

	return 1;
}

int TMFieldScene::OnPacketRemainNPCCount(MSG_STANDARDPARM* pStd)
{
	char szText[128]{};
	sprintf(szText, "%d / %d", pStd->Parm & 0xFF, pStd->Parm >> 16);

	m_dwRemainTime = g_pTimerManager->GetServerTime();
	m_pRemainText->SetText(szText, 0);
	m_pRemainText->SetVisible(1);
	return 1;
}

int TMFieldScene::OnPacketREQArray(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketNuke(MSG_STANDARD* pStd)
{
	// just that
	return 1;
}

int TMFieldScene::OnPacketRandomQuiz(MSG_RandomQuiz* pStd)
{
	pStd->Question[127] = 0;
	pStd->Answer[0][31] = 0;
	pStd->Answer[1][31] = 0;
	pStd->Answer[2][31] = 0;
	pStd->Answer[3][31] = 0;

	if (m_pQuizQuestion)
		m_pQuizQuestion->SetText(pStd->Question, 0);
	if (m_pQuizButton[0])
		m_pQuizButton[0]->SetText(pStd->Answer[0]);
	if (m_pQuizButton[1])
		m_pQuizButton[1]->SetText(pStd->Answer[1]);
	if (m_pQuizButton[2])
		m_pQuizButton[2]->SetText(pStd->Answer[2]);
	if (m_pQuizButton[3])
		m_pQuizButton[3]->SetText(pStd->Answer[3]);
	if (m_pQuizBG)
		m_pQuizBG->SetVisible(1);

	return 1;
}

int TMFieldScene::OnPacketAutoKick(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketRunQuest12Start(MSG_STANDARDPARM* pStd)
{
	if (pStd->Parm)
	{
		m_nQuest12MaxMobs = pStd->Parm;

		char szText[128]{};
		sprintf(szText, "0 / %d", m_nQuest12MaxMobs);
		m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 130.0f,
			30.0f * RenderDevice::m_fHeightRatio);

		m_pRemainText->SetText(szText, 0);
		m_pRemainText->SetVisible(1);
	}
	else
		m_pRemainText->SetVisible(0);

	return 1;
}

int TMFieldScene::OnPacketRunQuest12Count(MSG_STANDARDPARM2* pStd)
{
	char szText[128]{};
	sprintf(szText, "%d / %d", pStd->Parm1, pStd->Parm2);

	m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 130.0f,
		30.0f * RenderDevice::m_fHeightRatio);

	m_pRemainText->SetText(szText, 0);
	return 1;
}

int TMFieldScene::OnPacketDelayQuit(MSG_STANDARDPARM* pStd)
{
	PostMessage(g_pApp->m_hWnd, 16, 0, 0);
	return 0;
}
