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
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Reserved >> 4);
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

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Reserved & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->IsMerchant() || (pHuman->m_stScore.Reserved & 0xF) == 15))
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
		unsigned char nDir = ((unsigned char)pHuman->m_stScore.Reserved >> 4);
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

		pHuman->m_cHide = (pHuman->m_dwID >= 0 && pHuman->m_dwID < 1000) && pHuman->m_stScore.Reserved & 1;
		if ((pHuman->m_dwID < 0 || pHuman->m_dwID > 1000) && (pHuman->m_stScore.Reserved & 0xF) >= 1 && (pHuman->m_stScore.Reserved & 0xF) <= 15)
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

int TMFieldScene::OnPacketWeather(MSG_STANDARDPARM* pStd)
{
	SetWeather(pStd->Parm);
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

int TMFieldScene::OnPacketReqChallange(MSG_STANDARD* pStd)
{
	if (!m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[407], 60, 0);
		m_pMessageBox->SetVisible(1);
	}

	return 1;
}

int TMFieldScene::OnPacketCloseShop(MSG_STANDARD* pStd)
{
	SetVisibleShop(0);
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

int TMFieldScene::OnPacketCombineComplete(MSG_STANDARD* pStd)
{
	SetVisibleMixItem(0);
	SetVisibleMixItemTiini(0);
	SetVisibleMixPanel(0);
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

int TMFieldScene::OnPacketRESULTGAMBLE(MSG_STANDARD* pStd)
{
	return 0;
}

int TMFieldScene::OnPacketREQArray(MSG_STANDARD* pStd)
{
	return 0;
}

/// Processa resultados de ataque, dano, skill em area e efeitos visuais recebidos do servidor.
int TMFieldScene::OnPacketAttack(MSG_STANDARD* pStd)
{
	auto pAttack = reinterpret_cast<MSG_Attack*>(pStd);

	auto pAttacker = (TMHuman*)g_pObjectManager->GetHumanByID(pAttack->AttackerID);
	auto pTarget = (TMHuman*)g_pObjectManager->GetHumanByID(pAttack->Dam[0].TargetID);

	bool bomb = false;
	if (pAttack->SkillIndex == 104)
	{
		pAttack->SkillIndex = 39;
		bomb = true;
	}

	TMVector2 vecAttackerPos{};
	int nClass = 0;
	float fHeight = 1.5f;

	if (pAttacker)
	{
		if (pAttack->DoubleCritical & 8)
		{
			unsigned int dwStartTime = g_pTimerManager->GetServerTime();
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
			{
				if (pAttacker->m_nSkinMeshType == 3 || pAttacker->m_nSkinMeshType == 8 || pAttacker->m_nClass == 40)
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_cSForce = 5;
				else if(pAttacker->m_nWeaponTypeL == 101)
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_cSForce = 2;
				else if (pAttacker->m_nWeaponTypeL == 41)
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_cSForce = 3;
				else if (pAttacker->m_nWeaponTypeL == 103)
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_cSForce = 4;
				else
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_cSForce = 1;

				pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = dwStartTime;
			}

			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				if (pAttacker->m_nSkinMeshType == 3 || pAttacker->m_nSkinMeshType == 8 || pAttacker->m_nClass == 40)
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_cSForce = 5;
				else if (pAttacker->m_nWeaponTypeL == 101)
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_cSForce = 2;
				else if (pAttacker->m_nWeaponTypeL == 41)
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_cSForce = 3;
				else if (pAttacker->m_nWeaponTypeL == 103)
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_cSForce = 4;
				else
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_cSForce = 1;

				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = dwStartTime;
			}
		}
		if (pAttacker->m_nClass == 32 && pAttack->Motion == 4 && pTarget == m_pMyHuman)
			pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime();

		for (int i = 0; i < 13; ++i)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;
			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			pAttacker->m_usTargetID[i] = pAttack->Dam[i].TargetID;
		}

		SetSkillColor(pAttacker, static_cast<char>(pAttack->SkillIndex));

		vecAttackerPos = pAttacker->m_vecPosition;
		if (pAttacker->m_nSkinMeshType == 20 && pAttacker->m_stLookInfo.HelmMesh == 0)
			nClass = 1;

		if ((pAttacker != m_pMyHuman || !pAttack->FlagLocal && pAttacker == m_pMyHuman)	&& 
			pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104)
		{
			pAttacker->m_stScore.Mp = pAttack->CurrentMp;
			if (pAttacker == m_pMyHuman)
			{
				g_pObjectManager->m_stMobData.CurrentScore.Mp = pAttack->CurrentMp;
				auto pMPBar = (SProgressBar*)m_pControlContainer->FindControl(1170);
				auto pCurrentMPText = (SText*)m_pControlContainer->FindControl(65616);

				if (pMPBar)
					pMPBar->SetCurrentProgress(pAttacker->m_stScore.Mp);
				if (pCurrentMPText)
				{
					char szText[128]{};
					sprintf(szText, "%d", pAttacker->m_stScore.Mp);
					pCurrentMPText->SetText(szText, 0);
				}
			}
		}
		// Maybe the skills names are wrong rsrs.
		if (pAttacker != m_pMyHuman || pAttack->FlagLocal == 1 && pAttacker == m_pMyHuman || !pAttack->FlagLocal && 
			pAttacker == m_pMyHuman && (unsigned char)pAttack->Motion == 254)
		{
			if (pAttack->SkillIndex == 4) // Possuído
			{
				pAttacker->m_cPunish = 1;
				pAttacker->m_dwPunishedTime = g_pTimerManager->GetServerTime();
			}

			float fAngle = pAttacker->m_fWantAngle;

			if (!pTarget)
				pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY), static_cast<char>(pAttack->SkillIndex));
			else if ((unsigned char)pAttack->Motion != 254)
			{
				if (pAttacker->m_sHeadIndex <= 50)
					pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, pTarget, pAttack->SkillIndex);
				else
					pAttacker->Attack((ECHAR_MOTION)pAttack->Motion, pTarget, *(unsigned char*)&pAttack->SkillIndex);

				fHeight = pTarget->m_fHeight + 1.5f;

				if (pAttacker != pTarget)
					fAngle = atan2f(pTarget->m_vecPosition.x - pAttacker->m_vecPosition.x, pTarget->m_vecPosition.y - pAttacker->m_vecPosition.y) + D3DXToRadian(90);
			}

			if (pAttack->SkillIndex == 98) // Canhão Superior
				fAngle = atan2f((float)pAttack->TargetX - pAttacker->m_vecPosition.x, (float)pAttack->TargetY - pAttacker->m_vecPosition.y) + D3DXToRadian(90);
			if (pAttack->DoubleCritical & 1)
				pAttacker->m_bDoubleAttack = 1;
			if (pAttacker->m_nClass != 44)
				pAttacker->SetWantAngle(fAngle);

			if (pTarget && pTarget != pAttacker && pTarget != m_pMyHuman && (pTarget->m_nClass != 56 || pTarget->m_stLookInfo.FaceMesh)	&&
				pAttack->SkillIndex != 27)
			{
				pTarget->SetWantAngle(fAngle + D3DXToRadian(180));
			}

			if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 || 
				pAttack->SkillIndex >= 151 && pAttack->SkillIndex <= 155 || 
				pAttack->SkillIndex == 104 || pAttack->SkillIndex == 105 || pAttack->SkillIndex == 111)
			{
				pAttacker->m_stEffectEvent.sEffectIndex = pAttack->SkillIndex;
				pAttacker->m_stEffectEvent.sEffectLevel = (unsigned char)pAttack->SkillParm;
				if ((unsigned char)pAttack->Motion == 254)
					pAttacker->m_stEffectEvent.sEffectLevel = 1;

				if (!pTarget)
				{
					float iY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
					pAttacker->m_stEffectEvent.vecTo = TMVector3((float)pAttack->TargetX, iY, (float)pAttack->TargetY);
					pAttacker->m_stEffectEvent.pTarget = 0;
				}
				else
				{
					pAttacker->m_stEffectEvent.pTarget = pTarget;
					if (pTarget && pAttacker && pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
					{
						TMVector3 Len{ pTarget->m_vecPosition.x - pAttacker->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y - pAttacker->m_vecPosition.y };
						pAttacker->m_stEffectEvent.vecTo = TMVector3((float)(Len.x / 2.0f) + pAttacker->m_vecPosition.x, pTarget->m_fHeight, (float)(Len.z / 2.0f) + pAttacker->m_vecPosition.y);
					}
					else
						pAttacker->m_stEffectEvent.vecTo = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
				}

				if (pAttack->SkillIndex >= 151 && pAttack->SkillIndex <= 153 || 
					pAttack->SkillIndex == 104 || pAttack->SkillIndex == 105)
				{
					pAttacker->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 200;
				}
				else
				{
					pAttacker->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 500;
				}
			}
			if (pAttacker->m_nClass == 62 && pAttacker->m_stLookInfo.FaceMesh == 2 && pAttack->SkillIndex == 108 && pTarget)
			{
				TMVector3 vecStart{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
				TMVector3 vecDest{ pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y };

				vecDest.y += 1.0f;
				vecStart.y += 1.0f;

				auto pMagic = new TMSkillMagicArrow(vecStart, vecDest, 5, nullptr);
				if (pMagic && m_pEffectContainer)
					m_pEffectContainer->AddChild(pMagic);
			}
			else if (pAttack->SkillIndex == 6) // Aura da Vida
			{
				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
				
				auto pEffect = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFF5555FF, 0xFF222299, 1000, 1.0f, 5, 0.0f);
				if (pEffect && m_pEffectContainer)
					m_pEffectContainer->AddChild(pEffect);
			}
			else if (pAttack->SkillIndex == 3) // Perseguição
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					vecPos.y -= 0.5f;

					auto pHoly = new TMSkillHolyTouch(vecPos, 1);

					if (pHoly && m_pEffectContainer)
						m_pEffectContainer->AddChild(pHoly);
				}
			}
			else if (pAttack->SkillIndex == 5) // Fanatismo
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					vecPos.y -= 0.5f;

					auto pEffect = new TMEffectStart(vecPos, 2, nullptr);
					if (pEffect && m_pEffectContainer)
						m_pEffectContainer->AddChild(pEffect);

					GetSoundAndPlay(151, 0, 0);
				}
			}
			else if (pAttack->SkillIndex == 45) // Arma Mágica
			{
				float fY = (float)pAttack->TargetY + 0.5f;
				TMVector3 vecTarget{ (float)pAttack->TargetX + 0.5f, (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, fY)) * 0.1f, fY };

				if (pTarget)
					vecTarget = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 1.0f, pTarget->m_vecPosition.y);

				auto pParticle = new TMEffectParticle(vecTarget, 0, 20, 0.1f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);
				if (pParticle && m_pEffectContainer)
					m_pEffectContainer->AddChild(pParticle);

				GetSoundAndPlay(158, 0, 0);
			}
			else if (pAttack->SkillIndex == 34 && (unsigned char)pAttack->Motion == 254) // Relampago
			{
				if (pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFAAEEFF);
						m_pEffectContainer->AddChild(pEffect);
					}
				}
			}
			else if (pAttack->SkillIndex == 32 && (unsigned char)pAttack->Motion == 254) // Renascimento
			{
				if (pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFFFAA00);
						m_pEffectContainer->AddChild(pEffect);
					}
				}
			}
			else if (pAttack->SkillIndex == 36) // Tempestade de Meteoros
			{
				if ((unsigned char)pAttack->Motion == 254 && pAttacker && pAttacker->m_pFamiliar)
				{
					auto pEffect = new TMEffectBillBoard(1, 700, 0.1f, 0.1f, 0.1f, 0.002f, 1, 80);
					if (pEffect)
					{
						pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pEffect->m_vecStartPos = TMVector3(pAttacker->m_pFamiliar->m_vecPosition.x, pAttacker->m_pFamiliar->m_fHeight + 0.1f, pAttacker->m_pFamiliar->m_vecPosition.y);
						pEffect->SetColor(0xFFAAEEFF);
						m_pEffectContainer->AddChild(pEffect);
					}
				}

				TMVector3 vecPos{};
				if (pTarget)
					vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
				else
				{
					float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
					vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fY, (float)pAttack->TargetY + 0.5f);
				}

				for (int i = 0; i < 6; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(TMVector3(0.0f, 0.0f, 0.0f), TMVector3(((float)(i % 3) * 0.3f) + vecPos.x,
						vecPos.y, ((float)((i + 3) % 5) * 0.3f) + vecPos.z), 1, 0);

					if (pMeteor && m_pEffectContainer)
						m_pEffectContainer->AddChild(pMeteor);
				}
			}
			// Nevoa Venenosa, Choque Divino, Ataque de Fogo, Toque Sagrado
			else if (pAttack->SkillIndex == 41 || pAttack->SkillIndex == 29	|| pAttack->SkillIndex == 33 || pAttack->SkillIndex == 2)
			{
				for (int i = 0; i < 13; i++)
				{
					if ((unsigned char)pAttack->Motion == 254)
					{
						if (i >= 4)
							break;
					}
					else if (g_pSpell[pAttack->SkillIndex].MaxTarget <= i)
						break;

					auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pTargetHuman && pTargetHuman != pAttacker)
					{
						if (pAttack->SkillIndex == 33)
						{
							if (!pAttack->FlagLocal || pAttack->FlagLocal == 1 && pAttacker && pAttacker == m_pMyHuman)
							{
								int nValue = 0;
								if ((unsigned char)pAttack->Motion == 254)
									nValue = 1;
								if ((unsigned char)pAttack->Motion == 253)
									nValue = 2;
								if ((unsigned char)pAttack->Motion == 252)
								{
									auto vecPos = TMVector3((float)pAttack->PosX + 0.5f, 1.0f, (float)pAttack->PosY + 0.5f);
									if (pAttacker)
										vecPos = TMVector3(pAttacker->m_vecPosition.x, (float)(pAttacker->m_fScale * 5.5999999f) + pAttacker->m_fHeight, pAttacker->m_vecPosition.y);

									auto pEffect = new TMEffectSpark(vecPos, pTargetHuman, TMVector3(0.0f, 0.0f, 0.0f),
										0xFF5555FF, 0xFF222299, 1000, 1.0f, 5, 0.0f);

									if (pEffect && m_pEffectContainer)
										m_pEffectContainer->AddChild(pEffect);
								}
								else
								{
									auto pThunder = new TMSkillThunderBolt(TMVector3(pTargetHuman->m_vecPosition.x,
										pTargetHuman->m_fHeight,
										pTargetHuman->m_vecPosition.y), nValue);

									if (pThunder && m_pEffectContainer)
										m_pEffectContainer->AddChild(pThunder);
								}
							}
						}
						else
						{						
							pTargetHuman->m_stEffectEvent.sEffectIndex = pAttack->SkillIndex;
							pTargetHuman->m_stEffectEvent.sEffectLevel = (unsigned char)pAttack->SkillParm;
							pTargetHuman->m_stEffectEvent.pTarget = pTargetHuman;

							if (pTargetHuman && pAttacker && pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh)
							{
								TMVector3 Len{ pTargetHuman->m_vecPosition.x - pAttacker->m_vecPosition.x, pTargetHuman->m_fHeight,
									pTargetHuman->m_vecPosition.y - pAttacker->m_vecPosition.y };

								pAttacker->m_stEffectEvent.vecTo = TMVector3((float)(Len.x / 2.0) + pAttacker->m_vecPosition.x,
									pTargetHuman->m_fHeight,
									(float)(Len.z / 2.0) + pAttacker->m_vecPosition.y);
							}
							else
							{
								pTargetHuman->m_stEffectEvent.vecTo = TMVector3(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight, pTargetHuman->m_vecPosition.y);
							}

							pTargetHuman->m_stEffectEvent.dwTime = g_pTimerManager->GetServerTime() + 500;
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 52) // Enfraquecer
			{
				if ((unsigned char)pAttack->Motion == 254)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					if (pTarget)
					{
						TMVector3 vecTarget{ pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y };

						auto pSlowSlash = new TMSkillSlowSlash(vecPos, vecTarget, 2, pTarget);
						if (pSlowSlash)
							m_pEffectContainer->AddChild(pSlowSlash);

						unsigned int dwServerTime = g_pTimerManager->GetServerTime();

						auto pLevelUp = new TMEffectLevelUp(vecTarget, 2);
						if (pLevelUp)
							m_pEffectContainer->AddChild(pLevelUp);

						if (pTarget->m_pEleStream2)
							pTarget->m_pEleStream2->StartVisible(dwServerTime);

						GetSoundAndPlay(156, 0, 0);
					}
				}
				else
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };
					TMVector3 vecTarget;
					if (pTarget)
						vecTarget = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight, pTarget->m_vecPosition.y);
					else
					{
						float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
						vecTarget = TMVector3((float)pAttack->TargetX + 0.5f, fY, (float)pAttack->TargetY + 0.5f);
					}
					auto result = ((vecTarget - vecPos) / 7.0f);
					vecTarget = vecPos + result;

					auto pFreeze = new TMSkillFreezeBlade(vecTarget, 2, 0, 0);
					if (pFreeze)
						pFreeze->m_vecNextD = TMVector2(result.x, result.z);

					if (m_pEffectContainer && pFreeze)
						m_pEffectContainer->AddChild(pFreeze);
				}
			}
			// Recuperar, Exterminar
			else if (pAttack->SkillIndex == 30 || pAttack->SkillIndex == 23)
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pAttacker->m_vecPosition.y };
					int nType = 0;
					if (pAttack->SkillIndex == 23)
					{
						nType = 2;

						if (pTarget)
							vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 0.5f, pTarget->m_vecPosition.y);
						else
							vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fHeight + 0.5f, (float)pAttack->TargetY + 0.5f);
					}

					auto pJudgement = new TMSkillJudgement(vecPos, nType, 0.1f);
					if (pJudgement)
						m_pEffectContainer->AddChild(pJudgement);
				}
			}
			else if (pAttack->SkillIndex == 55) // Aura Bestial
			{
				TMVector3 vecPos;
				if (pTarget)
				{
					vecPos = TMVector3(pTarget->m_vecPosition.x, pTarget->m_fHeight + 0.2f, pTarget->m_vecPosition.y);

					auto pJudgement = new TMSkillJudgement(vecPos, 3, 0.1f);
					if (pJudgement)
						m_pEffectContainer->AddChild(pJudgement);
				}
				else
				{
					float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f)) * 0.1f;
					vecPos = TMVector3((float)pAttack->TargetX + 0.5f, fY + 0.2f, (float)pAttack->TargetY + 0.5f);
				}

				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					auto pOwner = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pOwner)
					{
						auto pEffect = new TMEffectStart(TMVector3(pOwner->m_vecPosition.x, pOwner->m_fHeight + 0.5f, pOwner->m_vecPosition.y), 5, pOwner);

						if (pEffect)
						{
							pEffect->m_dwLifeTime = 2000;
							m_pEffectContainer->AddChild(pEffect);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 76) // Imunidade
			{
				GetSoundAndPlay(34, 0, 0);
			}
			else if (pAttack->SkillIndex == 77) // Meditação
			{
				GetSoundAndPlay(36, 0, 0);
			}
			else if (pAttack->SkillIndex == 87) // Escudo Dourado
			{
				if (!g_bHideEffect)
				{
					for (int i = 0; i < 10; i++)
					{
						int nRand = rand() % 5;

						auto pBill = new TMEffectBillBoard(
							56,
							150 * i + 1000,
							((float)(i % 2) * 0.1f) + 0.2f,
							((float)(i % 2) * 0.1f) + 0.2f,
							((float)(i % 2) * 0.1f) + 0.2f,
							0.0f,
							1,
							80);

						if (pBill)
						{
							pBill->m_fParticleH = (float)((float)(i % 3) * 0.15f) + 1.1f;
							pBill->m_fParticleV = -2.0f;
							pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
							pBill->m_nParticleType = 3;

							pBill->m_vecStartPos = TMVector3((pAttacker->m_vecPosition.x + 0.5f) + ((float)(nRand - 3) * 0.1f),
								((pAttacker->m_fHeight + 0.5f) + ((float)(nRand - 3) * 0.1f)) + 2.0f,
								(pAttacker->m_vecPosition.y + 0.5f) + ((float)(nRand - 3) * 0.1f));

							pBill->SetColor(0xFF4444FF);

							m_pEffectContainer->AddChild(pBill);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 86) // Explosão Etérea
			{
				if (pAttacker)
				{
					TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pAttacker->m_vecPosition.y };
					TMVector2 vecSum{};

					int nCount = 0;
					for (int i = 0; i < 13; i++)
					{
						if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
							break;

						if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
							break;

						auto pOwner = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
						if (pOwner)
						{
							TMVector3 vecDest{ pOwner->m_vecPosition.x, pAttacker->m_fHeight + 0.5f, pOwner->m_vecPosition.y };

							vecSum.x += pOwner->m_vecPosition.x;
							vecSum.y += pOwner->m_vecPosition.y;
							++nCount;

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}

					TMVector2 vecDNormal{};
					if (nCount <= 0)
						vecDNormal = TMVector2((float)pAttack->TargetX + 0.5f, (float)pAttack->TargetY + 0.5f);
					else
						vecDNormal = vecSum / static_cast<const float>(nCount);

					float fAng = atan2f(vecDNormal.x - pAttacker->m_vecPosition.x, -vecDNormal.y + pAttacker->m_vecPosition.y);
					fAng -= D3DXToRadian(90);

					if (fAng < 0.0f)
						fAng += D3DXToRadian(360);
					if (fAng > D3DXToRadian(360))
						fAng -= D3DXToRadian(360);

					int nIndex = (int)((fAng * 12.0f) / D3DXToRadian(360));
					int nMin = nIndex - 3;
					int nMax = nIndex + 3;

					if (nMin < 0 || nMax > 12)
					{
						int nLow = 0;
						int nHigh = 0;

						if (nMin >= 0)
						{
							nLow = nMin;
							nHigh = nMax - 12;
						}
						else
						{
							nLow = nMin + 12;
							nHigh = nMax;
						}

						for (int i = 0; i < nHigh; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
						for (int i = nLow; i < 12; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}
					else
					{
						for (int i = nMin; i < nMax; i++)
						{
							TMVector2 vecCDest{ vecDNormal.x + m_vecKnifePos[i].x, vecDNormal.y + m_vecKnifePos[i].y };
							TMVector3 vecDest{ vecCDest.x, pAttacker->m_fHeight + 0.5f, vecCDest.y };

							auto pArrow = new TMArrow(vecPos, vecDest, 2, 152, 0, 0, 0);

							m_pEffectContainer->AddChild(pArrow);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 99) // Muro de Espinhos
			{
				if (pTarget)
				{
					bool bExpand = false;
					if (pTarget->m_nClass == 4 || pTarget->m_nClass == 8)
						bExpand = 1;

					auto pEffectSkinMesh = new TMEffectSkinMesh(
						pTarget->m_nSkinMeshType,
						TMVector3(0.0f, 0.0f, 0.0f), 
						TMVector3(0.0f, 0.0f, 0.0f), 
						0,
						nullptr);

					if (pTarget && pTarget->m_cMount > 0 && pTarget->m_pMount)
					{
						pEffectSkinMesh->m_nSkinMeshType = pTarget->m_nMountSkinMeshType;
						memcpy(&pEffectSkinMesh->m_stLookInfo, &pTarget->m_stMountLook, sizeof(pTarget->m_stMountLook));
						pEffectSkinMesh->m_nSkinMeshType2 = pTarget->m_nSkinMeshType;
						memcpy(&pEffectSkinMesh->m_stLookInfo2, &pTarget->m_stLookInfo, sizeof(pTarget->m_stLookInfo));
					}
					else
					{
						memcpy(&pEffectSkinMesh->m_stLookInfo, &pTarget->m_stLookInfo, sizeof(pTarget->m_stLookInfo));
					}

					pEffectSkinMesh->m_StartColor.r = 0.5f;
					pEffectSkinMesh->m_StartColor.g = 0.5f;
					pEffectSkinMesh->m_StartColor.b = 0.5f;
					pEffectSkinMesh->InitObject(bExpand);
					pEffectSkinMesh->m_nFade = 1;
					pEffectSkinMesh->m_dwLifeTime = 3000;
					pEffectSkinMesh->InitPosition(pTarget->m_vecPosition.x, pTarget->m_fHeight + 6.0f, pTarget->m_vecPosition.y);

					if (pTarget->m_cMount > 0 && pTarget->m_pMount)
					{
						if (pEffectSkinMesh->m_pSkinMesh)
							pEffectSkinMesh->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType].dwAniTable[0]);
						if (pEffectSkinMesh->m_pSkinMesh2)
							pEffectSkinMesh->m_pSkinMesh2->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType2].dwAniTable[24]);
					}
					else
					{
						pEffectSkinMesh->m_pSkinMesh->SetAnimation(g_MobAniTable[pEffectSkinMesh->m_nSkinMeshType].dwAniTable[0]);
					}

					pEffectSkinMesh->m_fStartAngle = pTarget->m_fAngle;
					pEffectSkinMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
					pEffectSkinMesh->m_nMotionType = 8;

					m_pEffectContainer->AddChild(pEffectSkinMesh);
				}

				unsigned int dwColor = 0xAAAAAAAA;

				for(int i = -1; i < 2; i++)
				{
					auto pEffectMesh = new TMEffectMesh(506, dwColor, 0.0f, 0);

					pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
					pEffectMesh->m_cShine = 1;
					pEffectMesh->m_dwCycleTime = 3000;
					pEffectMesh->m_dwLifeTime = 3000;
					pEffectMesh->m_fScaleH = 1.5f;
					pEffectMesh->m_fScaleV = 3.0f;
					pEffectMesh->m_vecPosition = TMVector3(((float)i * 0.2f) + pTarget->m_vecPosition.x, pTarget->m_fHeight - 0.5f, ((float)i * 0.2f) + pTarget->m_vecPosition.y);

					m_pEffectContainer->AddChild(pEffectMesh);
				}

				auto pShade = new TMShade(4, 118, 1.0f);

				pShade->m_dwLifeTime = 8000;
				pShade->SetColor(dwColor);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pTarget->m_vecPosition.x, pTarget->m_vecPosition.y));
				
				m_pEffectContainer->AddChild(pShade);

				GetSoundAndPlay(1, 0, 0);
			}
			else if (pAttack->SkillIndex == 100) // Ressureição
			{
				GetSoundAndPlay(156, 0, 0);
			}
			else if (pAttack->SkillIndex == 106 || pAttack->SkillIndex == 108)
			{
				int nSkill = 0;
				if (pAttack->SkillIndex == 108)
					nSkill = 1;

				if (pTarget && pAttacker)
					pAttacker->Fire(pTarget, nSkill);
			}
			else if (pAttack->SkillIndex == 107)
			{
				if (pTarget && pAttacker)
				{
					fHeight = ((float)(TMHuman::m_vecPickSize[pAttacker->m_nSkinMeshType].y	* pAttacker->m_fScale) * 0.69999999f) + pAttacker->m_fHeight;
					if(pAttacker->m_nSkinMeshType == 8)
						fHeight = (float)(1.0f * pAttacker->m_fScale) + fHeight;

					TMVector3 vecPos{ pAttacker->m_vecPosition.x, fHeight, pAttacker->m_vecPosition.y };

					auto pEffectSpark = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFFFFAA00, 0xFF554411, 1000, 1.0f, 5, 0.0f);

					m_pEffectContainer->AddChild(pEffectSpark);
				}
			}
			else if (pAttack->SkillIndex == 109)
			{
				if (pTarget && pAttacker)
				{
					fHeight = ((float)(TMHuman::m_vecPickSize[pAttacker->m_nSkinMeshType].y * pAttacker->m_fScale) * 0.69999999f) + pAttacker->m_fHeight;
					if (pAttacker->m_nSkinMeshType == 42)
						fHeight = (float)(1.0f * pAttacker->m_fScale) + fHeight;

					TMVector3 vecPos{ pAttacker->m_vecPosition.x, fHeight, pAttacker->m_vecPosition.y };

					auto pEffectSpark = new TMEffectSpark(vecPos, pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFF550000, 0xAA663311, 1000, 1.0f, 5, 0.0f);

					m_pEffectContainer->AddChild(pEffectSpark);
				}
			}
			else if (pAttack->SkillIndex == 110) // Skill Kefra
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				bool bMyAttack = false;
				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;

					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);

					if (pMultiTarget)
					{
						if (pMultiTarget == m_pMyHuman)
							bMyAttack = true;

						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 1.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart, vecTarget, 3, nullptr);
						pMeteor->m_dwStartTime += rand() % 1000;
						
						m_pEffectContainer->AddChild(pMeteor);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						auto pMeteor = new TMSkillMeteorStorm(
							TMVector3(((float)rndx + vecStart.x) - 9.0f, vecStart.y, ((float)rndy + vecStart.z) - 9.0f), 
							TMVector3(((float)rndx + vecTarget.x) - 9.0f, vecTarget.y, ((float)rndy + vecTarget.z) - 9.0f), 
							3, nullptr);

						pMeteor->m_dwStartTime += rand() % 1000;

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				if (pAttacker->m_dwEarthQuakeTime + 1700 < g_pTimerManager->GetServerTime())
				{
					pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime() - 1000;

					for (int i = -3; i < 3; i++)
					{
						for (int j = -3; i < 3; i++)
						{
							auto pBill = new TMEffectBillBoard(193, 4000, 2.0f, 2.0f, 2.0f, 0.001f, 1, 80);

							pBill->m_bStickGround = i % 2;
							pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x, pAttacker->m_fHeight, ((float)j * 2.0f) + pAttacker->m_vecPosition.y);
							pBill->SetColor(0xFF777799);

							m_pEffectContainer->AddChild(pBill);
						}
					}

					auto pGround = m_pGround;
					if (pGround)
					{
						pGround->m_dwEffStart = g_pTimerManager->GetServerTime();
						pGround->m_vecEffset = pAttacker->m_vecPosition;
					}
				}

				int nDistance = BASE_GetDistance((int)pAttacker->m_vecPosition.x, (int)pAttacker->m_vecPosition.y, 
					(int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);

				unsigned int Now = g_pTimerManager->GetServerTime();

				unsigned int dwKhepraDelay = 2000;
				if (!bMyAttack)
					dwKhepraDelay = 1000;
				if (nDistance < 18)
				{
					if (dwKhepraDelay + m_pMyHuman->m_dwOldMovePacketTime < Now && !m_pMyHuman->m_cDie &&
						m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_DEAD && 
						m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_DIE)
					{
						int nRan = 4;
						int nLength = 5;

						if (!bMyAttack)
						{
							nRan = 8;
							nLength = 9;
						}

						if (rand() % 10 < nRan)
						{
							m_pTargetHuman = nullptr;
							if (!g_bRunning)
								SetRunMode();

							int x = (int)m_pMyHuman->m_vecPosition.x;
							int y = (int)m_pMyHuman->m_vecPosition.y;

							int nMoveX = 0;
							int nMoveY = 0;
							if (x <= 2362)
								nMoveX = -nLength;
							else if (x >= 2370)
								nMoveX = nLength;
							if (y <= 3927)
								nMoveY = -nLength;
							else if (y >= 3935)
								nMoveY = nLength;

							int targetx = (int)((float)nMoveX + m_pMyHuman->m_vecPosition.x);
							int targety = (int)((float)nMoveY + m_pMyHuman->m_vecPosition.y);
							if (targetx < 2341)
								targetx = 2341;
							else if (targetx > 2391)
								targetx = 2391;
							if (targety < 3907)
								targetx = 3907;
							else if (targety > 3952)
								targety = 3952;

							char Route[48]{};
							BASE_GetRoute(x, y, &targetx, &targety, Route, 12, (char*)m_HeightMapData, 8);
							if (!strlen(Route))
								return 1;

							MSG_Action Msg{};
							Msg.Header.ID = m_pMyHuman->m_dwID;
							Msg.Header.Type = MSG_Action_Opcode;
							Msg.PosX = targetx;
							Msg.PosY = targety;
							Msg.Effect = 2;
							Msg.TargetX = targetx;
							Msg.TargetY = targety;

							g_bLastStop = MSG_Action_Opcode;

							m_stMoveStop.LastX = targetx;
							m_stMoveStop.LastY = targety;
							m_stMoveStop.NextX = targetx;
							m_stMoveStop.NextY = targety;

							SendOneMessage((char*)&Msg, sizeof(Msg));

							m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
							m_pMyHuman->OnPacketEvent(MSG_Action_Opcode, (char*)&Msg);
						}
					}
				}
			}
			else if (pAttack->SkillIndex == 111)
			{
				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };

				auto pShade = new TMShade(70, 118, 1.0f);

				pShade->SetColor(0xAA660000);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pAttacker->m_vecPosition.x, pAttacker->m_vecPosition.y));
				pShade->m_dwLifeTime = 3200;
				m_pEffectContainer->AddChild(pShade);

				TMVector3 vecTarget{};
				TMVector3 vecTarget2{};
				TMVector3 vecD{};
				TMVector3 vecPos2{};
				D3DXVECTOR3 vecDNormal{};

				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x, pMultiTarget->m_fHeight, pMultiTarget->m_vecPosition.y);
						vecTarget2 = vecTarget;

						auto pFreeze = new TMSkillFreezeBlade(vecTarget2, 8, 1, 0);
						pFreeze->m_vecNextD = TMVector2(vecD.x, vecD.z);

						m_pEffectContainer->AddChild(pFreeze);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						vecTarget2 = TMVector3(((float)rndx + vecTarget.x) - 9.0f, vecTarget.y, ((float)rndy + vecTarget.z) - 9.0f);
						
						auto pFreeze = new TMSkillFreezeBlade(vecTarget2, 8, 1, 0);
						pFreeze->m_vecNextD = TMVector2(vecD.x, vecD.z);

						m_pEffectContainer->AddChild(pFreeze);
					}
				}
			}
			else if (pAttacker->m_nClass == 56 && !pAttacker->m_stLookInfo.FaceMesh)
			{
				TMVector3 vecSpTarget = pAttacker->m_vecTempPos[2];
				TMVector3 vecSpStart = pAttacker->m_vecTempPos[1];
				TMVector3 vecStart{};
				TMVector3 vecTarget{};
				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					int rndx = rand() % 18;
					int rndy = rand() % 18;

					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						TMVector3 vecStart = pAttacker->m_vecTempPos[i % 2 + 1];
						TMVector3 vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight + 1.0f,
							pMultiTarget->m_vecPosition.y);

						auto pEffectSpark = new TMEffectSpark(vecStart,
							pMultiTarget,
							TMVector3(0.0f, 0.0f, 0.0f),
							0xFFFF0000,
							0xFF222299,
							1000,
							1.0f,
							5,
							0.0);

						pEffectSpark->m_fRange = 0.2f;
						m_pEffectContainer->AddChild(pEffectSpark);
					}
					else if (vecTarget.x != 0.0f && vecTarget.y != 0.0f && vecTarget.z != 0.0f)
					{
						vecStart = pAttacker->m_vecTempPos[i % 2 + 1];

						auto pEffectSpark = new TMEffectSpark(vecStart,
							nullptr,
							TMVector3(((float)rndx + vecTarget.x) - 9.0f,
								vecTarget.y,
								(float)((float)rndy + vecTarget.z) - 9.0f),
							0xFFFF0000,
							0xFF222299,
							1000,
							1.0f,
							5,
							0.0);

						pEffectSpark->m_fRange = 0.4f;
						m_pEffectContainer->AddChild(pEffectSpark);
					}
				}
			}
			else if (pAttack->SkillIndex == 113)
			{
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);
				unsigned int dwColor = 0xFF33FF66;

				TMVector3 vecPos{ pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y };

				auto pShade = new TMShade(70, 118, 1.0f);
				pShade->SetColor(0xAA660000);
				pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
				pShade->SetPosition(TMVector2(pAttacker->m_vecPosition.x, pAttacker->m_vecPosition.y));
				pShade->m_dwLifeTime = 3200;
				m_pEffectContainer->AddChild(pShade);

				TMVector3 vecTarget{};
				TMVector3 vecD{};
				TMVector3 vecPos2{};
				D3DXVECTOR3 vecDNormal{};

				for (int i = 0; i < 13; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight,
							pMultiTarget->m_vecPosition.y);

						auto pFreeze = new TMSkillFreezeBlade(vecTarget, 8, 1, 0);
						m_pEffectContainer->AddChild(pFreeze);

						auto pPoison = new TMSkillPoison(vecTarget, dwColor, 10, 1, 0);
						m_pEffectContainer->AddChild(pPoison);
					}
				}
			}
			else if (pAttack->SkillIndex == 114)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};

				bool bMyAttack = false;
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);

				for (int i = 0; i < 13; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);
						vecTarget = vecStart;
						vecStart.y = 1.0f;

						auto pJudgement = new TMSkillJudgement(vecStart, 2, 0.1f);

						m_pEffectContainer->AddChild(pJudgement);

						TMVector3 vec{};
						for (int j = 0; j < 4; j++)
						{
							vec = TMVector3(((float)(2 * (j % 2)) + vecTarget.x) - 1.0f,
								vecTarget.y,
								((float)(2 * (j / 2)) + vecTarget.z) - 1.0f);

							auto pFreeze = new TMSkillFreezeBlade(
								vec, 1, 0, 0
							);

							m_pEffectContainer->AddChild(pFreeze);
						}

						TMVector3 vecDest{ pMultiTarget->m_vecPosition.x,
								  pMultiTarget->m_fHeight + 0.5f,
								  pMultiTarget->m_vecPosition.y };

						vecStart = TMVector3(vecDest.x + 3.0f, vecDest.y + 5.0f, vecDest.z - 3.0f);

						auto pArrow = new TMArrow(vecStart, vecDest, 0, 10001, 0, 0, 0);
						pArrow->m_nColor = 0xFFFFFF00;

						m_pEffectContainer->AddChild(pArrow);
					}
				}
			}
			else if (pAttack->SkillIndex == 115)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				if (pAttacker->m_nClass != 73)
					pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);

				for (int i = 0; i < 13; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 10.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart,
							vecTarget,
							3,
							nullptr);

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				int ran = rand() % 10;

				if (ran >= 6)
				{
					if (pAttacker->m_dwEarthQuakeTime + 4000 < g_pTimerManager->GetServerTime())
					{
						pAttacker->m_dwEarthQuakeTime = g_pTimerManager->GetServerTime() - 2000;

						for (int i = -3; i < 3; i++)
						{
							for (int j = -3; j < 3; j++)
							{
								auto pBill = new TMEffectBillBoard(193,
									4000,
									2.0f,
									2.0f,
									2.0f,
									0.001f,
									1,
									80);

								pBill->m_bStickGround = i % 2;
								pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x,
									pAttacker->m_fHeight,
									(float)((float)j * 2.0f) + pAttacker->m_vecPosition.y);
								pBill->SetColor(0xFF777799);

								m_pEffectContainer->AddChild(pBill);
							}
						}

						auto pGround = m_pGround;
						if (pGround)
						{
							pGround->m_dwEffStart = g_pTimerManager->GetServerTime();
							pGround->m_vecOffset = pAttacker->m_vecPosition;
						}

						auto pCamera = g_pObjectManager->GetCamera();
						pCamera->EarthQuake(11);
						
						GetSoundAndPlay(160, 0, 0);
					}
				}
			}
			else if (pAttack->SkillIndex == 116)
			{
				TMVector3 vecTarget{};
				TMVector3 vecStart{};
				pAttacker->SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK04, 0);
				
				for (int i = 0; i < 13; i++)
				{
					auto pMultiTarget = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pMultiTarget)
					{
						vecTarget = TMVector3(pMultiTarget->m_vecPosition.x,
							pMultiTarget->m_fHeight - 5.0f,
							pMultiTarget->m_vecPosition.y);

						vecStart = vecTarget;
						vecStart.y = 10.0f;

						auto pMeteor = new TMSkillMeteorStorm(vecStart, 
							vecTarget,
							3, 
							0);

						m_pEffectContainer->AddChild(pMeteor);
					}
				}

				for (int i = -3; i < 3; i++)
				{
					for (int j = -3; j < 3; j++)
					{
						auto pBill = new TMEffectBillBoard(193,
							4000,
							2.0f,
							2.0f,
							2.0f,
							0.001f,
							1,
							80);

						pBill->m_bStickGround = i % 2;
						pBill->m_vecPosition = TMVector3(((float)i * 2.0f) + pAttacker->m_vecPosition.x,
							pAttacker->m_fHeight,
							(float)((float)j * 2.0f) + pAttacker->m_vecPosition.y);
						pBill->SetColor(0xFF777799);

						m_pEffectContainer->AddChild(pBill);
					}
				}
			}
		}
	}
	else
	{
		bool bFind = false;
		for (int i = 0; i < 13; i++)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;

			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			if (pAttack->Dam[i].TargetID = m_pMyHuman->m_dwID)
			{
				bFind = true;
				break;
			}
		}

		if (bFind || pAttack->SkillIndex > 55 && pAttack->SkillIndex > 63)
		{
			TMVector2 vecAttackerPos{ (float)pAttack->PosX + 0.5f, (float)pAttack->PosY + 0.5f };

			MSG_STANDARDPARM stParm{};
			stParm.Header.ID = g_pObjectManager->m_dwCharID;
			stParm.Header.Type = 0x369;
			stParm.Parm = pAttack->AttackerID;
			SendOneMessage((char*)&stParm, sizeof(stParm));
		}
	}

	TMVector3 vecStart{};
	TMVector3 vecTarget{};

	if (pAttacker)
		vecStart = TMVector3(pAttacker->m_vecPosition.x, pAttacker->m_fHeight + 1.0f, pAttacker->m_vecPosition.y);
	else
	{
		float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
		vecStart = TMVector3(vecAttackerPos.x, fY + 1.0f, vecAttackerPos.y);
	}

	unsigned int dwDelay = 0;
	if (g_pSpell[pAttack->SkillIndex].TargetType != 3 && g_pSpell[pAttack->SkillIndex].TargetType != 4 && 
		g_pSpell[pAttack->SkillIndex].TargetType != 5 && g_pSpell[pAttack->SkillIndex].TargetType != 6)
	{
		for (int i = 0; i < 13; i++)
		{
			if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
				break;

			if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
				break;

			auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
			if (pTargetHuman)
			{
				int nDamageRate = pTargetHuman->m_cDamageRate;
				if (nDamageRate == 0)
					nDamageRate = 1;

				if (m_pMyHuman)
				{
					int nDistance = BASE_GetDistance((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y,
						(int)pTargetHuman->m_vecPosition.x,	(int)pTargetHuman->m_vecPosition.y);

					if (nDistance > 20)
					{
						OutputDebugString(">>> damage font error distance : %d\n");
						return 0;
					}
				}
				if (pAttacker != m_pMyHuman || pAttack->FlagLocal == 0 && pAttacker == m_pMyHuman)
				{
					if (pAttack->Dam[i].Damage == -3 || pAttack->Dam[i].Damage == -4)
					{
						int nTX = 0;
						int nTY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY))
						{
							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								char szStr[128]{};
								sprintf(szStr, "miss");

								auto pFont = new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFFFFFF, 2.0f, dwDelay, 1, 1500, 0, 4);
								m_pExtraContainer->AddChild(pFont);
							}
						}
					}
					else if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 && 
						g_pSpell[pAttack->SkillIndex].InstanceType == 6 && 
						(pAttack->Dam[i].Damage >= 0 || pAttack->Dam[i].Damage <= -6))
					{
						if (!pAttack->FlagLocal)
						{
							if (!pTargetHuman->m_MaxBigHp)
							{
								int HealDam = pAttack->Dam[i].Damage / nDamageRate;
								int Dam = pTargetHuman->m_stScore.Hp - HealDam;

								if (Dam > pTargetHuman->m_stScore.MaxHp)
									Dam = pTargetHuman->m_stScore.MaxHp;
								if (Dam <= 0)
									pTargetHuman->m_stScore.Hp = 0;
								else
									pTargetHuman->m_stScore.Hp = Dam;
							}
							else
							{
								int Dam = pAttack->Dam[i].Damage;
								if (pTargetHuman->m_BigHp == Dam)
									pTargetHuman->m_BigHp = 0;
								else
									pTargetHuman->m_BigHp -= Dam;
								if (!nDamageRate)
									nDamageRate = 1;

								pTargetHuman->m_stScore.Hp = (short)(pTargetHuman->m_BigHp / nDamageRate);
							}

							if (pTargetHuman == m_pMyHuman)
							{
								int HealDam = pAttack->Dam[i].Damage / nDamageRate;
								int Dam = HealDam + m_nReqHP;
								// TODO: change to define
								if (Dam > 100000)
									Dam = 100000;
								if (Dam <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP = Dam;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}
						int nTX = 0;
						int nTY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y,	&nTX, &nTY))
						{
							char szStr[128]{};
							if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
								(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
								&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
							{
								sprintf(szStr, "?");
							}
							else
								sprintf(szStr, "+ %d", -pAttack->Dam[i].Damage / ((pAttack->DoubleCritical & 1) + 1));

							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								auto pFont = new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFF5555FF, 2.0f, dwDelay, 1, 1500, 0, 2);
								m_pExtraContainer->AddChild(pFont);
							}
						}
						if (pAttack->AttackerID == m_pMyHuman->m_dwID)
							SetMyHumanExp(pAttack->CurrentExp, pAttack->FakeExp);
					}
					else
					{
						if (!pAttack->FlagLocal)
						{
							int nValue = pAttack->Dam[i].Damage;
							if (nValue > 1000000)
								nValue = 0;
							if (nValue >= 0)
							{
								if (!pTargetHuman->m_MaxBigHp)
								{
									nValue /= nDamageRate;
									int hp = pTargetHuman->m_stScore.Hp - nValue;
									if (hp < 0)
										hp = 0;
									pTargetHuman->m_stScore.Hp = hp;
								}
								else
								{
									pTargetHuman->m_BigHp -= nValue;
									pTargetHuman->m_stScore.Hp = (short)(pTargetHuman->m_BigHp / nDamageRate);
								}
							}
							if (pTargetHuman->m_stScore.Hp < 0)
								pTargetHuman->m_stScore.Hp = 0;
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage > 0)
									m_nReqHP -= nValue;
								else
									m_nReqHP = 0;

								if ((int)pAttack->AttackerID > 1000)
								{
									if (pAttack->ReqMp > 0)
										m_nReqMP -= pAttack->ReqMp;

									pTargetHuman->m_stScore.Mp = m_nReqMP;
								}

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						pTargetHuman->m_wAttackerID = pAttack->AttackerID;

						int bInScreen = 0;
						int nTX = 0;
						int nTY = 0;
						if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pTargetHuman && pAttacker)
							bInScreen = BASE_Get3DTo2DPos(
								(float)(pTargetHuman->m_vecPosition.x * 0.5f) + (float)(pAttacker->m_vecPosition.x * 0.5f),
								pTargetHuman->m_fHeight - 1.0f,
								(float)(pTargetHuman->m_vecPosition.y * 0.5f) + (float)(pAttacker->m_vecPosition.y * 0.5f),
								&nTX,
								&nTY);
						else
							bInScreen = BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY);

						if (bInScreen)
						{
							if (!pTargetHuman || !pAttacker)
								return 1;

							for (int bViewHalf = 0; bViewHalf < (pAttack->DoubleCritical & 1) + 1; bViewHalf++)
							{
								int nFlank = 0;
								if (pAttack->DoubleCritical & 4)
									nFlank = pAttack->Dam[1].Damage;

								int nValue = (pAttack->Dam[i].Damage - nFlank) / ((pAttack->DoubleCritical & 1) + 1);
								if (nValue > 1000000)
									nValue = 0;

								if (nValue > 0)
								{
									char szStr[128]{};
									if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
										((int)pTargetHuman->m_vecPosition.x >> 7 != 18 || (int)pTargetHuman->m_vecPosition.y >> 7 != 30) && 
										(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
										&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
									{
										sprintf(szStr, "?");
									}
									else if (!(pAttack->DoubleCritical & 4) || bViewHalf)
									{
										sprintf(szStr, "%d", nValue);
									}
									else if (pAttack->Dam[1].Damage > 0)
									{
										sprintf(szStr, "%d + %d", nValue, nFlank);
									}

									bool bDrawFront = false;
									if (g_bHideEffect)
									{
										if (pAttacker == m_pMyHuman)
											bDrawFront = true;
										if (pTargetHuman == m_pMyHuman)
											bDrawFront = true;
									}
									else
									{
										bDrawFront = true;
									}

									if (pAttack->TargetX == 1129 && pAttack->TargetY == 1707 || 
										pAttack->TargetX == 1116 && pAttack->TargetY == 1707 ||
										pAttack->TargetX == 1094 && pAttack->TargetY == 1690)
									{
										bDrawFront = false;
									}

									if (bDrawFront)
									{
										TMFont3* pFont = nullptr;
										unsigned int dwColor = 0xFFFFFFFF;
										float fSize = 1.0f;
										if (i > 0 && pAttack->Dam[i].TargetID == pAttack->Dam[i - 1].TargetID)
										{
											m_pMyHuman->m_bCritical = 0;
											if (!(pAttack->DoubleCritical & 2))
											{
												if (pTargetHuman == m_pMyHuman)
												{
													if (bInScreen)
													{
														pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
															(int)(RenderDevice::m_fHeightRatio * 80.0f) + 
															(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) - 
																((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)), 
															dwColor,
															fSize, 
															dwDelay,
															1,
															1200,
															bViewHalf, 
															4);
													}
												}
												else if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
															((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														1,
														1200,
														bViewHalf,
														3);
												}
											}
											else
											{
												m_pMyHuman->m_bCritical = 1;
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX + 20 - 10 * bViewHalf,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
															((float)(40 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														9,
														300 * (pAttack->DoubleCritical & 2) + 1200,
														bViewHalf,
														5);
												}
											}
										}
										else
										{
											m_pMyHuman->m_bCritical = 0;
											if (pAttack->DoubleCritical & 2)
											{
												m_pMyHuman->m_bCritical = 1;
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * RenderDevice::m_fHeightRatio)) -
															((float)(40 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														9,
														550 * (pAttack->DoubleCritical & 2) + 1000 * bViewHalf + 1000,
														bViewHalf,
														5);
												}
											}
											else if (pTargetHuman == m_pMyHuman)
											{
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX,
														(int)(RenderDevice::m_fHeightRatio * 40.0f) +
														(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
															((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)),
														dwColor,
														fSize,
														dwDelay,
														1,
														1000 * bViewHalf + 1000,
														bViewHalf,
														4);
												}
											}
											else if (bInScreen)
											{
												pFont = new TMFont3(szStr, nTX,
													(int)(RenderDevice::m_fHeightRatio * 40.0f) +
													(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) -
														((float)(20 * bViewHalf) * RenderDevice::m_fHeightRatio)),
													dwColor,
													fSize,
													dwDelay,
													1,
													1000 * bViewHalf + 1000,
													bViewHalf,
													3);
											}
										}

										if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pAttacker != m_pMyHuman)
										{
											if (pFont)
											{
												pFont->m_fScale = 0.5f;
												if (pFont->m_nType == 5)
													pFont->m_nType = 6;
											}
										}

										m_pExtraContainer->AddChild(pFont);
									}

									if (pTargetHuman == m_pMyHuman && pAttacker)
										sprintf(m_szLastAttackerName, "%s", pAttacker->m_szName);

									if (pTargetHuman->m_cCriticalArmor == 1)
									{
										auto pEffectMesh = new TMEffectMesh(2838, 0xFF999999, pTargetHuman->m_fAngle, 4);

										pEffectMesh->m_nTextureIndex = 413;
										pEffectMesh->m_dwLifeTime = 500;
										pEffectMesh->m_dwCycleTime = 500;
																				
										if (pTargetHuman->m_cMount == 1)
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecSkinPos.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_vecSkinPos.y)
												- 0.30000001f,
												pTargetHuman->m_vecSkinPos.z);
										}
										else
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecPosition.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_fHeight)
												+ 0.30000001f,
												pTargetHuman->m_vecPosition.y);
										}

										pEffectMesh->m_fScaleH = 2.5f;
										pEffectMesh->m_fScaleV = 2.5f;
										pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
										pEffectMesh->m_cShine = 0;

										m_pEffectContainer->AddChild(pEffectMesh);
									}
								}
							}
						}
					}

					if (pTargetHuman->m_stScore.Hp < 0 && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.Hp = 0;
						m_nReqHP = 0;
					}
					if (pTargetHuman->m_stScore.Hp > pTargetHuman->m_stScore.MaxHp && !pAttack->FlagLocal)
						pTargetHuman->m_stScore.Hp = pTargetHuman->m_stScore.MaxHp;
					if (pTargetHuman == m_pMyHuman && !pAttack->FlagLocal)
						g_pObjectManager->m_stMobData.CurrentScore.Hp = pTargetHuman->m_stScore.Hp;

					pTargetHuman->UpdateScore(0);

					auto pPartyList = m_pPartyList;
					if (pPartyList)
					{
						for (int i = 0; i < pPartyList->m_nNumItem; i++)
						{
							auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
							if (pPartyItem->m_dwCharID == pTargetHuman->m_dwID)
							{
								pPartyItem->m_pHpProgress->SetMaxProgress(pTargetHuman->m_stScore.MaxHp);
								pPartyItem->m_pHpProgress->SetCurrentProgress(pTargetHuman->m_stScore.Hp);
								break;
							}
						}
					}
				}
				if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104 && pAttack->SkillIndex != 29)
				{
					if (g_pSpell[pAttack->SkillIndex].InstanceType == 8)
						return 1;
				}

				if (pAttacker && pAttacker->m_nSkinMeshType == 11)
					nClass = 4;

				static const unsigned int dwDelayTable[36] = 
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0, 1000, 1000, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 700, 700, 700,
					1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
				};

				if ((unsigned char)pAttack->Motion > 9)
					pAttack->Motion = 9;

				int nMotion = (unsigned char)pAttack->Motion - 4;
				if (pAttack->SkillIndex == 151)
					nClass = 5;
				if (pAttacker != m_pMyHuman || !pAttack->FlagLocal && pAttacker == m_pMyHuman)
				{
					pTargetHuman->m_stPunchEvent.nDamage = pAttack->Dam[i].Damage / nDamageRate;
					pTargetHuman->m_stPunchEvent.vecFrom = vecAttackerPos;
					pTargetHuman->m_stPunchEvent.dwTime = 0 + dwDelayTable[6 * nClass + nMotion] + g_pTimerManager->GetServerTime();
				}
			}
		}
	}
	else
	{
		float fY = (float)GroundGetMask(TMVector2((float)pAttack->TargetX, (float)pAttack->TargetY)) * 0.1f;
		vecTarget = TMVector3((float)pAttack->TargetX, fY, (float)pAttack->TargetY);

		if (pAttacker != m_pMyHuman || !pAttack->FlagLocal && pAttacker == m_pMyHuman)
		{
			for (int i = 0; i < 13; i++)
			{
				if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
					break;

				if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
					break;

				auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
				if (pTargetHuman)
				{
					int nDamageRate = pTargetHuman->m_cDamageRate;
					if (nDamageRate == 0)
						nDamageRate = 1;

					if (pAttack->Dam[i].Damage == -3 || pAttack->Dam[i].Damage == -4)
					{
						int pX = 0;
						int pY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &pX, &pY))
						{
							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								char szStr[128]{};
								sprintf(szStr, "miss");
								auto pFont = new TMFont3(szStr, pX, pY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFFFFFF, 2.0f, dwDelay, 1, 1500, 0, 4);
								m_pExtraContainer->AddChild(pFont);
							}
						}
					}
					else if (pAttack->SkillIndex >= 0 && pAttack->SkillIndex < 104
						&& g_pSpell[pAttack->SkillIndex].InstanceType == 6
						&& (pAttack->Dam[i].Damage >= 0 || pAttack->Dam[i].Damage <= -6))
					{
						if (!pAttack->FlagLocal)
						{
							if (pTargetHuman->m_stScore.Hp - pAttack->Dam[i].Damage / nDamageRate <= 0)
								pTargetHuman->m_stScore.Hp = 0;
							else
								pTargetHuman->m_stScore.Hp -= pAttack->Dam[i].Damage / nDamageRate;
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP -= pAttack->Dam[i].Damage / nDamageRate;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						int nStartX = 0;
						int nStartY = 0;
						if (BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nStartX, &nStartY))
						{
							char szStr[128]{};
							if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
								(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
								&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
							{
								sprintf(szStr, "?");
							}
							else
								sprintf(szStr, "+ %d", -pAttack->Dam[i].Damage / ((pAttack->DoubleCritical & 1) + 1));

							bool bDrawFront = false;
							if (g_bHideEffect)
							{
								if (pAttacker == m_pMyHuman)
									bDrawFront = true;
								if (pTargetHuman == m_pMyHuman)
									bDrawFront = true;
							}
							else
							{
								bDrawFront = true;
							}

							if (bDrawFront)
							{
								auto pFont = new TMFont3(szStr, nStartX, nStartY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFF5555FF, 2.0f, dwDelay, 1, 1500, 0, 2);
								m_pExtraContainer->AddChild(pFont);
							}
						}
						if (pAttack->AttackerID == m_pMyHuman->m_dwID)
							SetMyHumanExp(pAttack->CurrentExp, pAttack->FakeExp);
					}
					else
					{
						if (!pAttack->FlagLocal)
						{
							if (!pTargetHuman->m_MaxBigHp)
							{
								if (pTargetHuman->m_stScore.Hp - pAttack->Dam[i].Damage / nDamageRate <= 0)
									pTargetHuman->m_stScore.Hp = 0;
								else
									pTargetHuman->m_stScore.Hp -= pAttack->Dam[i].Damage / nDamageRate;
							}
							else
							{
								if (pTargetHuman->m_BigHp == pAttack->Dam[i].Damage)
									pTargetHuman->m_BigHp = 0;
								else
									pTargetHuman->m_BigHp -= pAttack->Dam[i].Damage;
								pTargetHuman->m_stScore.Hp = (short)pTargetHuman->m_BigHp;
							}
							if (pTargetHuman == m_pMyHuman)
							{
								if (m_nReqHP - pAttack->Dam[i].Damage <= 0)
									m_nReqHP = 0;
								else
									m_nReqHP -= pAttack->Dam[i].Damage / nDamageRate;

								memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
							}
						}

						pTargetHuman->m_wAttackerID = pAttack->AttackerID;

						int bInScreen = 0;
						int nTX = 0;
						int nTY = 0;
						if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pTargetHuman && pAttacker)
							bInScreen = BASE_Get3DTo2DPos(
								(float)(pTargetHuman->m_vecPosition.x * 0.5f) + (float)(pAttacker->m_vecPosition.x * 0.5f),
								pTargetHuman->m_fHeight - 1.0f,
								(float)(pTargetHuman->m_vecPosition.y * 0.5f) + (float)(pAttacker->m_vecPosition.y * 0.5f),
								&nTX,
								&nTY);
						else
							bInScreen = BASE_Get3DTo2DPos(pTargetHuman->m_vecPosition.x, pTargetHuman->m_fHeight + 1.0f, pTargetHuman->m_vecPosition.y, &nTX, &nTY);

						if (bInScreen)
						{
							if (!pTargetHuman || !pAttacker)
								return 1;

							for (int bViewHalf = 0; bViewHalf < (pAttack->DoubleCritical & 1) + 1; bViewHalf++)
							{
								int nValue = pAttack->Dam[i].Damage / ((pAttack->DoubleCritical & 1) + 1);
								
								if (nValue > 0)
								{
									char szStr[128]{};
									if (((int)pTargetHuman->m_vecPosition.x >> 7 > 16 && (int)pTargetHuman->m_vecPosition.x >> 7 < 20 && (int)pTargetHuman->m_vecPosition.y >> 7 > 29) &&
										((int)pTargetHuman->m_vecPosition.x >> 7 != 18 || (int)pTargetHuman->m_vecPosition.y >> 7 != 30) &&
										(pAttack->AttackerID != m_pMyHuman->m_dwID || pTargetHuman->m_dwID < 1000)
										&& (pTargetHuman->m_dwID != m_pMyHuman->m_dwID || (int)pAttack->AttackerID < 1000))
									{
										sprintf(szStr, "?");
									}
									else 
									{
										sprintf(szStr, "%d", nValue);
									}
									
									if (pAttack->SkillIndex == 79)
										dwDelay += 100 * i;

									bool bDrawFront = false;
									if (g_bHideEffect)
									{
										if (pAttacker == m_pMyHuman)
											bDrawFront = true;
										if (pTargetHuman == m_pMyHuman)
											bDrawFront = true;
									}
									else
									{
										bDrawFront = true;
									}

									TMFont3* pFont = nullptr;
									unsigned int dwColor = 0xFFFFFFFF;
									float fSize = 1.0f;
									if (bDrawFront)
									{
										if (!(pAttack->DoubleCritical & 2))
										{
											if (pTargetHuman == m_pMyHuman)
											{
												if (bInScreen)
												{
													pFont = new TMFont3(szStr, nTX - 10 * i,
														(int)(RenderDevice::m_fHeightRatio * 80.0f) +
														(int)(((float)nTY - (float)(20.0f * i)) * RenderDevice::m_fHeightRatio),
														dwColor,
														fSize,
														dwDelay,
														1,
														1200,
														0,
														4);
												}
											}
											else if (bInScreen)
											{
												pFont = new TMFont3(szStr, nTX - 10 * i,
													(int)(RenderDevice::m_fHeightRatio * 80.0f) +
													(int)(((float)nTY - (float)(20.0f * i)) * RenderDevice::m_fHeightRatio),
													dwColor,
													fSize,
													dwDelay,
													1,
													1200,
													0,
													3);
											}
										}
										else if (bInScreen)
										{
											pFont = new TMFont3(szStr, nTX - 10 * i,
												(int)(RenderDevice::m_fHeightRatio * 80.0f) +
												(int)(((float)nTY - (float)(10.0f * RenderDevice::m_fHeightRatio)) - ((float)(40 * i) * RenderDevice::m_fHeightRatio) + 
													(float)(RenderDevice::m_fHeightRatio * 80.0f)),
												dwColor,
												fSize,
												dwDelay,
												1,
												1200,
												0,
												5);
										}

										if (pTargetHuman->m_nClass == 56 && !pTargetHuman->m_stLookInfo.FaceMesh && pAttacker != m_pMyHuman)
										{
											if (pFont)
											{
												pFont->m_fScale = 0.5f;
												if (pFont->m_nType == 5)
													pFont->m_nType = 6;
											}
										}

										m_pExtraContainer->AddChild(pFont);										
									}

									if (pTargetHuman == m_pMyHuman && pAttacker)
										sprintf(m_szLastAttackerName, "%s", pAttacker->m_szName);

									if (pTargetHuman->m_cCriticalArmor == 1)
									{
										auto pEffectMesh = new TMEffectMesh(2838, 0xFF999999, pTargetHuman->m_fAngle, 4);

										pEffectMesh->m_nTextureIndex = 413;
										pEffectMesh->m_dwLifeTime = 500;
										pEffectMesh->m_dwCycleTime = 500;

										if (pTargetHuman->m_cMount == 1)
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecSkinPos.x,
												(((TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_vecSkinPos.y)
												- 0.30000001f,
												pTargetHuman->m_vecSkinPos.z);
										}
										else
										{
											pEffectMesh->m_vecPosition = TMVector3(pTargetHuman->m_vecPosition.x,
												(float)((float)((float)(TMHuman::m_vecPickSize[pTargetHuman->m_nSkinMeshType].y
													* pTargetHuman->m_fScale)
													/ 2.0f)
													+ pTargetHuman->m_fHeight)
												+ 0.30000001f,
												pTargetHuman->m_vecPosition.y);
										}

										pEffectMesh->m_fScaleH = 2.5f;
										pEffectMesh->m_fScaleV = 2.5f;
										pEffectMesh->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
										pEffectMesh->m_cShine = 0;

										m_pEffectContainer->AddChild(pEffectMesh);
									}
								}
							}
						}
					}
					if (pTargetHuman->m_stScore.Hp < 0 && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.Hp = 0;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));

						m_nReqHP = 0;
					}
					if (pTargetHuman->m_stScore.Hp > pTargetHuman->m_stScore.MaxHp && !pAttack->FlagLocal)
					{
						pTargetHuman->m_stScore.Hp = pTargetHuman->m_stScore.MaxHp;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
					}
					if (pTargetHuman == m_pMyHuman && !pAttack->FlagLocal)
					{
						g_pObjectManager->m_stMobData.CurrentScore.Hp = pTargetHuman->m_stScore.Hp;
						if (pTargetHuman == m_pMyHuman)
							memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pTargetHuman->m_stScore, sizeof(pTargetHuman->m_stScore));
					}
					pTargetHuman->UpdateScore(0);
				}
			}
		}
		if (pAttacker != m_pMyHuman || (pAttack->FlagLocal == 1 && pAttacker == m_pMyHuman))
		{
			if (pAttack->SkillIndex == 0)
			{
				auto pHeavensDust = new TMSkillHeavensDust(vecTarget, 0);

				m_pEffectContainer->AddChild(pHeavensDust);
			}
			else if (pAttack->SkillIndex == 1)
			{
				TMVector3 vecPos{vecStart.x, vecStart.y - 0.5f, vecStart.z};

				auto pHollyTouch = new TMSkillHolyTouch(vecPos, 0);

				m_pEffectContainer->AddChild(pHollyTouch);
			}
			else if (pAttack->SkillIndex == 26)
			{
				TMVector3 vecPos{ vecStart.x, vecStart.y - 0.5f, vecStart.z };

				auto pFlash = new TMSkillFlash(vecPos, 0);

				m_pEffectContainer->AddChild(pFlash);

				if (!pAttacker)
					return 1;
				if (m_pMyHuman->IsInTown() == 1)
					return 1;
				if (pAttacker->m_usGuild && m_pMyHuman->m_usGuild && (m_pMyHuman->m_usGuild == pAttacker->m_usGuild || g_pObjectManager->m_usAllyGuild == pAttacker->m_usGuild))
					return 1;

				bool bFound = false;
				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Dam[i].TargetID == m_pMyHuman->m_dwID)
					{
						bFound = true;
						break;
					}
				}

				if (bFound && pAttacker != m_pMyHuman && !pAttacker->m_bParty)
				{
					float fFlashTerm = 0.0f;
					if (m_pMyHuman->m_stScore.Level <= 0)
						fFlashTerm = (((float)m_pMyHuman->m_stScore.Level * 4000.0f)
							* (float)pAttacker->m_stScore.Special[1])
						/ 100.0f;
					else
						fFlashTerm = (((float)(pAttacker->m_stScore.Level / m_pMyHuman->m_stScore.Level)
							* 4000.0f)
							* (float)pAttacker->m_stScore.Special[1])
						/ 100.0f;
					if (fFlashTerm < 2000.0)
						fFlashTerm = 2000.0;
					if (fFlashTerm > 4000.0)
						fFlashTerm = 4000.0;

					m_fFlashTerm = fFlashTerm;
					m_dwStartFlashTime = g_pTimerManager->GetServerTime();
				}
			}
			else if (pAttack->SkillIndex == 7)
			{
				for (int i = 0; i < 13; i++)
				{
					if (pAttack->Header.Type == MSG_Attack_One_Opcode && i >= 1)
						break;

					if (pAttack->Header.Type == MSG_Attack_Two_Opcode && i >= 2)
						break;

					auto pTargetHuman = g_pObjectManager->GetHumanByID(pAttack->Dam[i].TargetID);
					if (pTargetHuman)
					{
						float fTarget = 0.0f;
						if (pAttacker)
							fTarget = pAttacker->m_fHeight;

						vecStart = TMVector3(pTargetHuman->m_vecPosition.x,
							fTarget + 0.5f,
							pTargetHuman->m_vecPosition.y);

						vecTarget = TMVector3(vecStart.x + 3.0f, vecStart.y + 5.0f, vecStart.z - 3.0f);

						auto pArrow = new TMArrow(vecTarget, vecStart, 0, 10001, 0, 0, 0);

						m_pEffectContainer->AddChild(pArrow);
					}
				}
			}
			else if (pAttack->SkillIndex == 35)
			{
				vecTarget.y += 1.0f;

				auto pMeteor = new TMSkillMeteorStorm(TMVector3(0.0f, 0.0f, 0.0f), vecTarget, 0, nullptr);

				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 39 && !bomb)
			{
				vecTarget.y += 1.0f;

				TMVector3 vecSt{};

				for (int i = 0; i < 4; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3((float)(vecTarget.x - 1.8f) + ((float)(i % 2) * 3.5999999f),
						vecTarget.y,
						(float)(vecTarget.z - 1.8f) + ((float)(i / 2) * 3.5999999f)), 4, nullptr);

					pMeteor->m_dwStartTime += 200 * i;
					m_pEffectContainer->AddChild(pMeteor);
				}	

				auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3(vecTarget.x, vecTarget.y, vecTarget.z), 4, nullptr);

				pMeteor->m_dwStartTime += 270;
				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 39 && bomb)
			{
				vecTarget.y += 2.0f;

				TMVector3 vecSt{};

				for (int i = 0; i < 4; i++)
				{
					auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3((float)(vecTarget.x - 0.89999998f) + ((float)(i % 2) * 1.8f),
						vecTarget.y,
						(float)(vecTarget.z - 0.89999998f) + ((float)(i / 2) * 1.8f)), 4, nullptr);

					m_pEffectContainer->AddChild(pMeteor);
				}

				vecTarget.y += 0.30000001f;

				auto pMeteor = new TMSkillMeteorStorm(vecSt, TMVector3(vecTarget.x, vecTarget.y, vecTarget.z), 10, nullptr);
				m_pEffectContainer->AddChild(pMeteor);
			}
			else if (pAttack->SkillIndex == 97)
			{
				vecTarget.y += 1.0f;
				for (int i = 0; i < 100; i++)
				{
					auto pItem = (TMCannon*)g_pObjectManager->GetItemByID(i + 15001);
					if (pItem)
					{
						if (pItem->m_stItem.sIndex == 746)
						{
							if (vecStart.x == pItem->m_vecBasePosition.x && vecStart.z == pItem->m_vecBasePosition.y)
							{
								pItem->m_cFire = 1;
								vecStart.x = (float)(pItem->m_fCosF * pItem->m_fCannonLen) + vecStart.x;
								vecStart.z = vecStart.z - (float)(pItem->m_fSinF * pItem->m_fCannonLen);
								break;
							}
						}
					}
				}

				float fx = vecTarget.x - m_pMyHuman->m_vecPosition.x;
				float fy = vecTarget.z - m_pMyHuman->m_vecPosition.y;

				auto pMeteor = new TMSkillMeteorStorm(vecStart, vecTarget, 2, 0);

				pMeteor->m_fDestLength = sqrt((float)(fx * fx) + (float)(fy * fy));

				m_pEffectContainer->AddChild(pMeteor);
				return 1;
			}
		}
	}

	if (!pAttack->FlagLocal)
	{
		if (pAttacker == m_pMyHuman)
		{
			m_nReqMP = pAttack->ReqMp;
			if (m_nReqHP < static_cast<unsigned short>(m_pMyHuman->m_stScore.Hp))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.Hp);
			if (m_nReqMP < static_cast<unsigned int>(m_pMyHuman->m_stScore.Mp))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.Mp);
			if (m_nReqHP > static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHp))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHp);
			if (m_nReqMP > static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMp))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMp);
			UpdateScoreUI(0);
		}
		else if (pTarget == m_pMyHuman)
		{
			if (m_nReqHP < static_cast<unsigned short>(m_pMyHuman->m_stScore.Hp))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.Hp);
			if (m_nReqMP < static_cast<unsigned int>(m_pMyHuman->m_stScore.Mp))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.Mp);
			if (m_nReqHP > static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHp))
				m_nReqHP = static_cast<unsigned short>(m_pMyHuman->m_stScore.MaxHp);
			if (m_nReqMP > static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMp))
				m_nReqMP = static_cast<unsigned int>(m_pMyHuman->m_stScore.MaxMp);
			UpdateScoreUI(16);
		}
	}

	return 1;
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
