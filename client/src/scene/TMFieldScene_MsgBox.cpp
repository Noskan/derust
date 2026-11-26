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

/// @file TMFieldScene_MsgBox.cpp
/// @brief Respostas das caixas de mensagem do campo (`OnMsgBoxEvent`) e cache/download das
///        marcas de guilda (`Guildmark_*`).
///
/// Extraído de `TMFieldScene_UI.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

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
