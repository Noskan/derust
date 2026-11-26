#include "core/pch.h"
#include "ui/SGrid.h"
#include "core/TMGlobal.h"
#include "ui/SControlContainer.h"
#include "render/TMMesh.h"
#include "scene/TMFieldScene.h"
#include "core/TMUtil.h"
#include "common/ItemEffect.h"

/// @file SGrid_Input.cpp
/// @brief Eventos de entrada na grade (mouse, teclado, clique direito e uso rápido de itens).

int SGridControl::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (!m_bSelectEnable)
		return 0;
	if (g_pObjectManager->m_stMobData.CurrentScore.Hp <= 0)
		return 0;
	if (!m_bEnable)
		return 0;

	int nCellVWidth = (int)(m_nWidth / (float)m_nColumnGridCount);
	int nCellVHeight = (int)(m_nHeight / (float)m_nRowGridCount);
	int nCellX = (int)(((float)nX - m_nPosX) / (float)nCellVWidth);
	int nCellY = (int)(((float)nY - m_nPosY) / (float)nCellVHeight);
	int bPtInRect = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	bool bClick = false;

	if (!bPtInRect)
	{
		for (int i = 0; i < m_nNumItem; ++i)
			m_pItemList[i]->m_bOver = 0;
	}

	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (dwFlags == 513)
	{
		STRUCT_ITEM item{};
		unsigned int nItemPos = 0;
		if (g_pCursor->m_pAttachedItem)
		{
			if (g_pCursor->m_pAttachedItem->m_nCellIndexX < 0 || g_pCursor->m_pAttachedItem->m_nCellIndexX > 16	|| 
				g_pCursor->m_pAttachedItem->m_nCellIndexY < 0 || g_pCursor->m_pAttachedItem->m_nCellIndexY > 16)
			{
				return 0;
			}

			memcpy(&item, g_pCursor->m_pAttachedItem->m_pItem, sizeof(item));
			nItemPos = BASE_GetItemAbility(&item, 17);
		}

		bool bClick = true;
		if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_HAND && bPtInRect)
		{
			if (g_pTimerManager->GetServerTime() < m_dwLastBuyTime + 500)
				return 0;

			if (m_eGridType == TMEGRIDTYPE::GRID_TRADENONE || m_eGridType == TMEGRIDTYPE::GRID_TRADEMY || 
				m_eGridType == TMEGRIDTYPE::GRID_TRADEOP || m_eGridType == TMEGRIDTYPE::GRID_TRADEINV || 
				m_eGridType == TMEGRIDTYPE::GRID_TRADEINV2 || m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2
				|| m_eGridType == TMEGRIDTYPE::GRID_TRADEINV3)
			{
				return 0;
			}

			BuyItem(nCellX, nCellY);
			m_dwLastBuyTime = g_pTimerManager->GetServerTime();
		}
	}
	else if (dwFlags == 514)
	{
		if (g_pEventTranslator->m_bCtrl)
		{
			if (pFScene->m_pCargoPanel->m_bVisible)
			{
				g_pCursor->m_pAttachedItem = nullptr;
				automove(nCellX, nCellY);
			}
			return 0;
		}

		STRUCT_ITEM dst{};
		unsigned int nItemPos = 0;
		if (g_pCursor->m_pAttachedItem)
		{
			memcpy(&dst, g_pCursor->m_pAttachedItem->m_pItem, sizeof(dst));
			nItemPos = BASE_GetItemAbility(&dst, 17);
		}
		if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_HAND)
		{
			if (bPtInRect)
			{
				if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
				{
					g_pCurrentScene->m_pControlContainer->SetFocusedControl(nullptr);
					if (nCellX >= 9)
						pFScene->OnKeyShortSkill(48, 0);
					else
						pFScene->OnKeyShortSkill(nCellX + 49, 0);
					return 1;
				}

				if (m_eGridType == TMEGRIDTYPE::GRID_TRADENONE || m_eGridType == TMEGRIDTYPE::GRID_TRADEOP)
					return 1;

				int nRet = TradeItem(nCellX, nCellY);
				if (nRet != 2)
					return nRet;

				if (m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXRESULT && m_pLastMouseOverItem && 
					SGridControl::m_pLastMouseOverItem->m_pItem	&& 
					SGridControl::m_pLastMouseOverItem->m_pItem->sIndex > 0	&& 
					SGridControl::m_pLastMouseOverItem->m_pItem->sIndex < 11500)
				{
					pFScene->m_ItemMixClass.Set_NeedItemList(SGridControl::m_pLastMouseOverItem->m_pItem->sIndex);
					pFScene->m_ItemMixClass.CheckInv(pFScene->m_pGridInvList);
				}

				if (m_eGridType == TMEGRIDTYPE::GRID_MISSION_RESULT && SGridControl::m_pLastMouseOverItem && 
					SGridControl::m_pLastMouseOverItem->m_pItem && 
					SGridControl::m_pLastMouseOverItem->m_pItem->sIndex > 0	&&
					SGridControl::m_pLastMouseOverItem->m_pItem->sIndex < 11600)
				{
					pFScene->m_MissionClass.Set_NeedItemList(SGridControl::m_pLastMouseOverItem->m_pItem->sIndex);
					pFScene->m_MissionClass.CheckInv(pFScene->m_pGridInvList);
				}
				g_pCursor->m_pAttachedItem = nullptr;
			}
		}
		else if (bPtInRect)
		{
			if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_PICKUP && !SGridControl::m_pLastAttachedItem)
			{
				if (g_pCursor->m_pAttachedItem)
				{
					int nRet = SellItem(nCellX, nCellY, 514, wParam);
					if (nRet != 2)
						return nRet;
				}
			}
		}
		if (g_pCursor->m_pAttachedItem)
		{
			memcpy(&dst, g_pCursor->m_pAttachedItem->m_pItem, sizeof(dst));
			nItemPos = BASE_GetItemAbility(&dst, 17);
		}
		if (bPtInRect && g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_PICKUP && 
			SGridControl::m_pLastAttachedItem && g_pCursor->m_pAttachedItem)
		{
			int page = pFScene->m_pGridInv->m_dwControlID - 67072;
			if (page == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
				return 0;
			if (page == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
				return 0;

			SwapItem(nCellX, nCellY, nCellVWidth, nCellVHeight, &dst);
		}
	}
	if (!bClick && dwFlags == 512)
	{
		int nRet = MouseOver(nCellX, nCellY, bPtInRect);
		if (nRet != 2)
			return nRet;
	}
	else if (dwFlags == 516)
	{
		RButton(nCellX, nCellY, bPtInRect);
	}
	else if (dwFlags == 513 && g_pEventTranslator->m_bShift)
	{
		auto pItem = SelectItem(nCellX, nCellY);	
		if (!pItem)
			return 0;

		int nAmount = BASE_GetItemAmount(pItem->m_pItem);
		if (nAmount <= 1)
			return 0;
		
		bool itemcheck = 0;

		if (pItem->m_pItem->sIndex == 412)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex == 413)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex == 415)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex == 419)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex == 420)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex == 4049)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex >= 2390 && pItem->m_pItem->sIndex <= 2419)
			itemcheck = true;
		else if (pItem->m_pItem->sIndex >= 3200 && pItem->m_pItem->sIndex <= 3220)
			itemcheck = true;

		if (!itemcheck)
			return 0;

		SGridControl::m_pSellItem = pItem;
		auto pText = (SText*)pFScene->m_pControlContainer->FindControl(65888);
		auto pEdit = (SEditableText*)pFScene->m_pControlContainer->FindControl(65889);
		auto pInputGold = (SPanel*)pFScene->m_pControlContainer->FindControl(65885);
		pItem->m_GCObj.dwColor = 0xFFFF00FF;

		pFScene->m_nCoinMsgType = 12;
		pFScene->m_nLastAutoTradePos = -1;
		pText->SetText(g_pMessageStringTable[408], 0); 
		
		pFScene->m_pControlContainer->SetFocusedControl(pEdit);

		memset(pEdit->m_strComposeText, 0, sizeof(pEdit->m_strComposeText));
		pEdit->SetText((char*)"");
		pEdit->m_bEncrypt = 1;
		pInputGold->SetVisible(1);

		SGridControl::m_pSellItem = pItem;
	}
	else if (!bClick && dwFlags == 517 && bPtInRect && g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_HAND && 
		(m_eGridType == TMEGRIDTYPE::GRID_CARGO || m_eGridType == TMEGRIDTYPE::GRID_DEFAULT))
	{
		int page = m_dwControlID - 67072;
		if (page == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
			return 0;
		if (page == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
			return 0;

		if (!pFScene || !pFScene->m_pMyHuman)
			return 0;

		auto pMyHuman = pFScene->m_pMyHuman;
		auto pItem = SelectItem(nCellX, nCellY);

		if (m_eGridType == TMEGRIDTYPE::GRID_DEFAULT && 
			(pMyHuman->m_sFamiliar == 3914 || pMyHuman->m_sFamiliar == 3915)
			&& g_pEventTranslator->m_bShift == 1)
		{
			if (pItem)
			{
				if (pItem->m_pGridControl->m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE)
				{
					pFScene->m_pGridShop->m_dwMerchantID = 0;
					SGridControl::m_pSellItem = pItem;

					char szMessage[128];
					sprintf(szMessage, g_pMessageStringTable[342], &g_pItemList[pItem->m_pItem->sIndex]);
					pFScene->m_pMessageBox->SetMessage(szMessage, 890, g_pMessageStringTable[343]);
					pFScene->m_pMessageBox->SetVisible(1);
					return 1;
				}
			}
		}
		if (!g_pEventTranslator->m_bCtrl)
			return 0;

		if (pItem)
		{
			if (pItem->m_GCObj.dwColor != 0xFFFF0000)
			{
				unsigned int NewItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
				int NewItemPosConv;
				for (NewItemPosConv = 0; ; ++NewItemPosConv)
				{
					NewItemPos /= 2;
					if (!NewItemPos)
						break;
				}

				NewItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
				if (NewItemPos <= 1)
					return 0;

				int sDestType = CheckType(pItem->m_pGridControl->m_eItemType, pItem->m_pGridControl->m_eGridType);
				int sDestPos = CheckPos(pItem->m_pGridControl->m_eItemType);
				int nAX = pItem->m_nCellIndexX;
				int nAY = pItem->m_nCellIndexY;
				auto pMobData = &g_pObjectManager->m_stMobData;

				if (sDestPos == -1)
				{
					if (!BASE_CanEquip(pItem->m_pItem, &pMobData->CurrentScore, sDestPos, pMobData->Equip[0].sIndex, pMobData->Equip, 
						g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex, pMobData->HasSoulSkill()))
						return 0;

					if (NewItemPos >= 64 && NewItemPos <= 192)
					{
						STRUCT_ITEM itemL{};
						STRUCT_ITEM itemR{};

						itemL.sIndex = g_pCurrentScene->m_pMyHuman->m_sLeftIndex;
						itemR.sIndex = g_pCurrentScene->m_pMyHuman->m_sRightIndex;
						unsigned int nWeaponLPos = BASE_GetItemAbility(&itemL, 17);
						unsigned int nWeaponRPos = BASE_GetItemAbility(&itemR, 17);


						if (nWeaponLPos == 64 && nWeaponRPos == 128 && NewItemPos != 128)
							NewItemPosConv = 6;
						if (nWeaponLPos == 64 && NewItemPos == 192)
							NewItemPosConv = 6;
						if (nWeaponLPos == 192 && nWeaponRPos == 192 && NewItemPos != 128 && NewItemPos != 192)
							return 0;
						if (nWeaponRPos == 128 && NewItemPosConv == 7)
							NewItemPosConv = 6;

						if (!BASE_CanEquip(pItem->m_pItem, &pMobData->CurrentScore,	6, pMobData->Equip[0].sIndex, pMobData->Equip,
							g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex, pMobData->HasSoulSkill()) &&
							NewItemPos != 128)
						{
							return 0;
						}
					}

					MSG_SwapItem stSwapItem{};
					stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
					stSwapItem.Header.Type = MSG_SwapItem_Opcode;
					stSwapItem.SourType = 0;
					stSwapItem.SourPos = NewItemPosConv;
					stSwapItem.DestType = sDestType;
					stSwapItem.TargetID = TMFieldScene::m_dwCargoID;

					if (sDestType)
					{
						int page = m_dwControlID - 67072;
						if (page == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
							return 0;
						if (page == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
							return 0;
						stSwapItem.DestPos = 15 * page + nAX + 5 * nAY;
					}
					else
					{
						stSwapItem.DestPos = sDestPos;
					}

					if (stSwapItem.DestPos != stSwapItem.SourPos || stSwapItem.SourType != stSwapItem.DestType)
					{
						SendOneMessage((char*)&stSwapItem, 20);
						pFScene->m_pMouseOverHuman = 0;
					}
					return 0;
				}

				int SourPage = 0;
				IVector2 vecGrid{};

				auto pMyGrid = pFScene->m_pGridInv;
				int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
				if (nGridIndex > 7 || nGridIndex < 0)
					nGridIndex = 0;

				vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
				SourPage = pFScene->m_pGridInv->m_dwControlID - 67072;
				SourPage *= 15;

				int j = 0;
				if (vecGrid.x == -1)
				{
					for (j = 0; j < 4; ++j)
					{
						pMyGrid = pFScene->m_pGridInvList[j];
						nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
						if (nGridIndex > 7 || nGridIndex < 0)
							nGridIndex = 0;

						vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
						if (vecGrid.x != -1)
						{
							SourPage = 15 * j;
							break;
						}
					}
				}

				if (j == 3)
					return 0;
				if (SourPage < 0 || SourPage > 45)
					SourPage = 0;
				if (SourPage / 15 == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
					return 0;
				if (SourPage / 15 == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
					return 0;

				MSG_SwapItem Msg{};
				Msg.Header.ID = g_pObjectManager->m_dwCharID;
				Msg.Header.Type = MSG_SwapItem_Opcode;
				Msg.SourType = 1;
				Msg.SourPos = SourPage + vecGrid.x + 5 * vecGrid.y;
				Msg.DestType = sDestType;
				Msg.DestPos = sDestPos;
				Msg.TargetID = TMFieldScene::m_dwCargoID; 
				
				if ((unsigned char)sDestPos != (unsigned char)Msg.SourPos || Msg.SourType != Msg.DestType)
				{
					SendOneMessage((char*)&Msg, 20);
					pFScene->m_pMouseOverHuman = nullptr;
				}
			}
		}
	}

	return 0;
}

int SGridControl::OnKeyDownEvent(unsigned int iKeyCode)
{
	if (!m_bEnable)
		return 0;

	if (iKeyCode == '.' && m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
		if (g_pCurrentScene->m_pControlContainer->m_pFocusControl)
			return 0;

		int nDelIndex = -1;
		for (int i = 0; i < 10; ++i)
		{
			auto pItem = GetItem(i, 0);
			if (pItem && pItem->m_GCObj.nTextureSetIndex == 200)
			{
				nDelIndex = i;
				auto pReturnItem = PickupItem(i, 0);
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem)
					g_pCursor->m_pAttachedItem = 0;

				if (pReturnItem)
					delete pReturnItem;

				break;
			}
			if (pItem && pItem->m_GCObj.nTextureSetIndex == 2)
			{
				nDelIndex = i;
				auto pReturnItem = PickupItem(i, 0);
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem)
					g_pCursor->m_pAttachedItem = 0;

				if (pReturnItem)
					delete pReturnItem;
				break;
			}
		}

		if (pScene->m_pGridSkillBelt3->IsVisible() == 1)
			nDelIndex += 10;
		if (nDelIndex >= 0)
			g_pObjectManager->m_cShortSkill[nDelIndex] = -1;

		MSG_SetShortSkill stSetShortSkill{};
		stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
		stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;
		memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

		for (int ia = 0; ia < 20; ++ia)
		{
			if (stSetShortSkill.Skill[ia] >= 0 && stSetShortSkill.Skill[ia] < 96)
			{
				stSetShortSkill.Skill[ia] -= 24 * g_pObjectManager->m_stMobData.Class;
			}
			else if (stSetShortSkill.Skill[ia] >= 105 && stSetShortSkill.Skill[ia] < 153)
			{
				stSetShortSkill.Skill[ia] -= 12 * g_pObjectManager->m_stMobData.Class;
			}
		}

		SendOneMessage((char*)&stSetShortSkill, sizeof(stSetShortSkill));
		pScene->UpdateScoreUI(0);
		pScene->UpdateSkillBelt();
	}

	return 0;
}

void SGridControl::RButton(int nCellX, int nCellY, int bPtInRect)
{
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	if (!pFScene || !pFScene->m_pMyHuman)
		return;

	auto pMyHuman = pFScene->m_pMyHuman;

	if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_HAND && bPtInRect == 1 &&
		m_dwControlID >= G_SKILL_SEC1_1 && m_dwControlID <= G_SKILL_SEC3_8 &&
		pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_WALK &&
		pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_RUN)
	{
		auto pItem = GetAtItem(0, 0);
		if (!pItem)
			return;

		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		int cSkillIndex = pItem->m_pItem->sIndex - 0x1388;
		if (cSkillIndex >= 0 && cSkillIndex < 248 &&
			IsValidClassSkill(cSkillIndex) &&
			(!g_pSpell[cSkillIndex].TargetType || g_pSpell[cSkillIndex].TargetType == 2) &&
			g_pSpell[cSkillIndex].Aggressive != 1 &&
			(g_pSpell[cSkillIndex].AffectType > 0 || g_pSpell[cSkillIndex].TickType > 0) &&
			g_pSpell[cSkillIndex].Passive != 1)
		{
			int Delay = g_pSpell[cSkillIndex].Delay;
			if (pFScene->m_nMySanc >= 9 && Delay >= 2)
				--Delay;
			if (pMyHuman->m_DilpunchJewel == 1)
				--Delay;
			if (Delay < 1)
				Delay = 1;

			if (dwServerTime < pFScene->m_dwSkillLastTime[cSkillIndex] + 1000 * Delay ||
				dwServerTime < pFScene->m_dwSkillLastTime[cSkillIndex] + 1000 ||
				dwServerTime < pFScene->m_dwOldAttackTime + 1000)
				return;

			int nSpecial = pMyHuman->m_stScore.Level;
			if (cSkillIndex < 96)
				nSpecial = g_pObjectManager->m_stMobData.CurrentScore.Special[
					(cSkillIndex - 24 * g_pObjectManager->m_stMobData.Class) / 8 + 1];

			if (BASE_GetManaSpent(cSkillIndex, g_pObjectManager->m_stMobData.SaveMana, nSpecial) <=
				g_pObjectManager->m_stMobData.CurrentScore.Mp)
			{
				MSG_Attack stAttack{};

				stAttack.Header.Type = MSG_Attack_Multi_Opcode;
				stAttack.Header.ID = pMyHuman->m_dwID;
				stAttack.AttackerID = pMyHuman->m_dwID;
				stAttack.PosX = (int)pMyHuman->m_vecPosition.x;
				stAttack.PosY = (int)pMyHuman->m_vecPosition.y;
				stAttack.TargetX = (int)pMyHuman->m_vecPosition.x;
				stAttack.TargetY = (int)pMyHuman->m_vecPosition.y;
				stAttack.CurrentMp = -1;
				stAttack.SkillIndex = cSkillIndex;
				stAttack.SkillParm = 0;
				stAttack.Motion = -1;
				stAttack.FlagLocal = 0;
				stAttack.Dam[0].Damage = -1;
				stAttack.Dam[0].TargetID = pMyHuman->m_dwID;

				int nSize = sizeof(MSG_Attack);

				if (g_pSpell[cSkillIndex].MaxTarget != 1)
				{
					if (g_pSpell[cSkillIndex].MaxTarget == 2)
					{
						stAttack.Header.Type = MSG_Attack_Two_Opcode;
						nSize = sizeof(MSG_AttackTwo);
					}
					SendOneMessage((char*)&stAttack, nSize);
				}
				else
				{
					stAttack.Header.Type = MSG_Attack_One_Opcode;
					nSize = sizeof(MSG_AttackOne);
					SendOneMessage((char*)&stAttack, 72);
				}

				MSG_Attack stAttackLocal{};
				memcpy(&stAttackLocal, &stAttack, nSize);

				stAttackLocal.Header.ID = m_dwID;
				stAttackLocal.FlagLocal = 1;
				pFScene->OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
				pFScene->m_dwOldAttackTime = dwServerTime;
				pFScene->m_pTargetHuman = 0;
				pFScene->m_dwSkillLastTime[cSkillIndex] = dwServerTime;
			}
			else
			{
				auto pChatList = pFScene->m_pChatList;
				pChatList->AddItem(new SListBoxItem(g_pMessageStringTable[30], 0xFFFFAAAA, 0.0f, 0.0f, 280.0f,
					16.0f, 0, 0x77777777, 1, 0));

				auto pSoundManager = g_pSoundManager;
				if (pSoundManager)
				{
					auto pSoundData = pSoundManager->GetSoundData(33);
					if (pSoundData)
						pSoundData->Play(0, 0);
				}
			}
		}
		return;
	}
	if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_PICKUP && g_pCursor->m_pAttachedItem)
	{
		g_pCursor->DetachItem();
		g_pEventTranslator->m_bRBtn = 1;
		return;
	}

	if (g_pCursor->GetStyle() == ECursorStyle::TMC_CURSOR_HAND && m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE &&
		(m_eGridType == TMEGRIDTYPE::GRID_DEFAULT || m_eGridType == TMEGRIDTYPE::GRID_SELL))
	{
		int page = pFScene->m_pGridInv->m_dwControlID - 67072;
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();

		if ((pFScene->m_dwUseItemTime && dwServerTime - pFScene->m_dwUseItemTime < 200) ||
			(page == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467) ||
			(page == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467))
			return;

		if (!bPtInRect)
			return;

		if (g_pEventTranslator->m_bRBtn)
			return;

		auto pItem = GetItem(nCellX, nCellY);
		if (!pItem)
			return;

		int nType = BASE_GetItemAbility(pItem->m_pItem, 38);
		int nItemSIndex = pItem->m_pItem->sIndex;

		int skillId = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
		if (skillId >= 105)
			skillId += 95;

		if (skillId == 83 && pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			if (pFScene->m_dwOldAttackTime + 1000 < g_pTimerManager->GetServerTime())
			{
				char szMessage[128]{};
				sprintf(szMessage, g_pMessageStringTable[149], g_pItemList[pItem->m_pItem->sIndex].Name);
				pFScene->m_pMessageBox->SetMessage(szMessage, 84, g_pMessageStringTable[150]);
				pFScene->m_pMessageBox->SetVisible(1);

				short sDestType = CheckType(pItem->m_pGridControl->m_eItemType,
					pItem->m_pGridControl->m_eGridType);
				short sDestPos = CheckPos(pItem->m_pGridControl->m_eItemType);

				int Page = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);
				if (Page < 0 || Page > 45)
					Page = 0;
				if (sDestPos == -1)
					sDestPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

				pFScene->m_sDestType = sDestType;
				pFScene->m_sDestPos = Page + sDestPos;
			}
			return;
		}
		if (nItemSIndex == 3207)
		{
			if (!pFScene->m_pCargoPanel->IsVisible())
				pFScene->SetVisibleCargo(1);
		}
		if (nItemSIndex >= 3468 && nItemSIndex <= 3471)
		{
			int Page = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);
			if (Page < 0 || Page > 45)
				Page = 0;

			if (pMyHuman->m_cCancel != 1 &&
				(!pFScene->m_dwUseItemTime || dwServerTime - pFScene->m_dwUseItemTime >= 200))
			{
				short SourType = CheckType(pItem->m_pGridControl->m_eItemType,
					pItem->m_pGridControl->m_eGridType);
				short SourPos = CheckPos(pItem->m_pGridControl->m_eItemType);
				if (SourPos == -1)
					SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

				SourPos = Page + SourPos;

				auto vec = pMyHuman->m_vecPosition;

				MSG_UseItem stUseItem{};
				stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				stUseItem.Header.Type = MSG_UseItem_Opcode;
				stUseItem.SourType = 1;
				stUseItem.SourPos = SourPos;
				stUseItem.ItemID = 0;
				stUseItem.GridX = (int)vec.x;
				stUseItem.GridY = (int)vec.y;
				SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

				int nAmount = BASE_GetItemAmount(pItem->m_pItem);
				if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
					nAmount = 0;
				if (nAmount <= 1)
				{
					auto pPickedItem = PickupItem(nCellX, nCellY);
					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
						g_pCursor->m_pAttachedItem = nullptr;
					SAFE_DELETE(pPickedItem);
				}
				else
				{
					BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
					sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
					pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
				}

				if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
					pFScene->UpdateScoreUI(0);
				if (nAmount <= 1)
					memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

				pFScene->m_dwUseItemTime = dwServerTime;
			}
			return;
		}
		if (nItemSIndex == 3467)
		{
			int Page = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);
			if (Page < 0 || Page > 45)
				Page = 0;

			if (pMyHuman->m_cCancel != 1 &&
				(!pFScene->m_dwUseItemTime || dwServerTime - pFScene->m_dwUseItemTime >= 200))
			{
				short SourType = CheckType(pItem->m_pGridControl->m_eItemType,
					pItem->m_pGridControl->m_eGridType);
				short SourPos = CheckPos(pItem->m_pGridControl->m_eItemType);
				if (SourPos == -1)
					SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

				SourPos = Page + SourPos;

				auto vec = pMyHuman->m_vecPosition;

				MSG_UseItem stUseItem{};
				stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				stUseItem.Header.Type = MSG_UseItem_Opcode;
				stUseItem.SourType = 1;
				stUseItem.SourPos = SourPos;
				stUseItem.ItemID = 0;
				stUseItem.GridX = (int)vec.x;
				stUseItem.GridY = (int)vec.y;
				SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

				pFScene->m_dwUseItemTime = dwServerTime;

				if (g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467 ||
					g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
				{
					MSG_STANDARDPARM2 stDeleteItem{};
					stDeleteItem.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
					stDeleteItem.Header.Type = MSG_DeleteItem_Opcode;
					stDeleteItem.Parm1 = SourPos;
					stDeleteItem.Parm2 = 3467;
					SendOneMessage((char*)&stDeleteItem, 20);
				}
			}
			return;
		}
		if (nType == 14)
		{
			pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[175], 38, 0);
			pFScene->m_pMessageBox->m_dwArg = nCellY | (nCellX << 16);
			pFScene->m_pMessageBox->SetVisible(1);
			return;
		}
		if (nType == 11 || nType == 13)
		{
			short SourType = CheckType(pItem->m_pGridControl->m_eItemType,
				pItem->m_pGridControl->m_eGridType);
			short SourPos = CheckPos(pItem->m_pGridControl->m_eItemType);
			if (SourPos == -1)
				SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

			int Page = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);
			if (Page < 0 || Page > 45)
				Page = 0;

			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
			{
				if (pMyHuman &&
					pMyHuman->m_fProgressRate < 0.89999998f && pMyHuman->m_fProgressRate > 0.0f)
				{
					return;
				}

				pFScene->m_dwGetItemTime = g_pTimerManager->GetServerTime();
				pFScene->m_dwLastTeleport = pFScene->m_dwGetItemTime;
				pFScene->m_cLastTeleport = 1;
				memset(&pFScene->m_stUseItem, 0, sizeof(pFScene->m_stUseItem));
				pFScene->m_stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				pFScene->m_stUseItem.Header.Type = MSG_UseItem_Opcode;
				pFScene->m_stUseItem.SourType = 1;
				pFScene->m_stUseItem.SourPos = Page + SourPos;
				pFScene->m_stUseItem.ItemID = 0;
				pFScene->m_stUseItem.GridX = (int)pMyHuman->m_vecPosition.x;
				pFScene->m_stUseItem.GridY = (int)pMyHuman->m_vecPosition.y;

				MSG_STANDARDPARM stDelayStart{};
				stDelayStart.Header.ID = pMyHuman->m_dwID;
				stDelayStart.Header.Type = MSG_DelayStart_Opcode;
				stDelayStart.Parm = 1;
				SendOneMessage((char*)&stDelayStart, sizeof(stDelayStart));
			}

			g_pEventTranslator->m_bRBtn = 1;

			int nAmount = BASE_GetItemAmount(pItem->m_pItem);
			if (pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount <= 1)
			{
				auto pPickedItem = PickupItem(nCellX, nCellY);
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
					g_pCursor->m_pAttachedItem = nullptr;
				SAFE_DELETE(pPickedItem);
			}
			else
			{
				BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
				sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
				pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
			}

			int nSoundIndex = 41;
			if (nType >= 11 && nType <= 13)
				nSoundIndex = 54;

			auto pSoundManager = g_pSoundManager;
			if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(nSoundIndex);
				if (pSoundData)
					pSoundData->Play(0, 0);
			}

			if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
				pFScene->UpdateScoreUI(0);
			if (nAmount <= 1)
				memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));
		}
		if (nItemSIndex == 3336 && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[286], 3000);
			pFScene->m_pMessagePanel->SetVisible(1, 1);
			return;
		}
		if (nItemSIndex == 3442)
		{
			if (!pFScene->m_dwUseItemTime || dwServerTime - pFScene->m_dwUseItemTime >= 200)
			{
				pFScene->m_pFireWorkPanel->SetVisible(1);
				for (int i = 0; i < 100; ++i)
					pFScene->m_pFireWorkButton[i]->Update();

				pFScene->m_nFireWorkCellX = nCellX;
				pFScene->m_nFireWorkCellY = nCellY;
				pFScene->m_dwUseItemTime = dwServerTime;
				g_pEventTranslator->m_bRBtn = 1;
			}
			return;
		}
		if (nType == 195)
		{
			pFScene->SetVisiblePotal(1, pItem->m_pItem->sIndex - 3432);

			short SourType = CheckType(pItem->m_pGridControl->m_eItemType,
				pItem->m_pGridControl->m_eGridType);
			short SourPos = CheckPos(pItem->m_pGridControl->m_eItemType);
			if (SourPos == -1)
			{
				int Page = 15 * (pItem->m_pGridControl->m_dwControlID - 67072);
				if (Page < 0 || Page > 45)
					Page = 0;

				SourPos = Page + 5 * pItem->m_nCellIndexY + pItem->m_nCellIndexX;
			}
			auto vec = pMyHuman->m_vecPosition;

			memset(&pFScene->m_stPotalItem, 0, sizeof(pFScene->m_stPotalItem));
			pFScene->m_stPotalItem.Header.ID = g_pObjectManager->m_dwCharID;
			pFScene->m_stPotalItem.Header.Type = MSG_UseItem_Opcode;
			pFScene->m_stPotalItem.SourType = 1;
			pFScene->m_stPotalItem.SourPos = SourPos;
			pFScene->m_stPotalItem.ItemID = 0;
			pFScene->m_stPotalItem.GridX = static_cast<unsigned short>(vec.x);
			pFScene->m_stPotalItem.GridY = static_cast<unsigned short>(vec.y);
			return;
		}
		if (nType == 187 && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{

			pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[300], 883, 0);
			pFScene->m_pMessageBox->m_dwArg = nCellY | (nCellX << 16);
			pFScene->m_pMessageBox->SetVisible(1);
			return;
		}
		if (Check_ItemRightClick(nType, nItemSIndex) && (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD ||
			pMyHuman->m_cCancel != 1 || nType != 1))
		{
			if (nType >= 70 && nType < 90)
			{
				if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
				{
					if (pFScene->m_pGridDRing->m_pItemList[0])
					{
						pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[285], 3000);
						pFScene->m_pMessagePanel->SetVisible(1, 1);
						return;
					}
				}
			}
			else if (nType == 206 && pItem->m_pItem->stEffect[0].cEffect == 59)
			{
				if (g_pCurrentScene)
					pFScene->VisibleInputCharName(pItem, nCellX, nCellY);
				return;
			}
			else if (nType == 206)
			{
				pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[350], 883, 0);
				pFScene->m_pMessageBox->m_dwArg = nCellY | (nCellX << 16);
				pFScene->m_pMessageBox->SetVisible(1);
				return;
			}
			else if (nType == 211)
			{
				pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[356], 883, 0);
				pFScene->m_pMessageBox->m_dwArg = nCellY | (nCellX << 16);
				pFScene->m_pMessageBox->SetVisible(1);
				return;
			}

			if (pMyHuman)
			{
				if (nItemSIndex == 4030)
				{
					if (!pFScene->m_dwUseItemTime || dwServerTime - pFScene->m_dwUseItemTime >= 200)
					{
						MSG_STANDARDPARM stParam{};
						stParam.Header.Type = MSG_UseDeclarationOfWar_Opcode;
						stParam.Header.Size = sizeof(stParam);
						stParam.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
						stParam.Parm = 0;
						SendOneMessage((char*)&stParam, sizeof(stParam));
						pFScene->m_dwUseItemTime = dwServerTime;
					}
				}
				else if (nItemSIndex != 4906 && (nItemSIndex < 4132 || nItemSIndex > 4139))
				{
					if (nItemSIndex == 4907)
						pFScene->m_pRPSGamePanel->SetVisible(1);

					pFScene->UseItem(pItem, nType, nItemSIndex, nCellX, nCellY);
				}
			}
		}
	}
}

int SGridControl::Check_ItemRightClick(int nType, int nItemSIndex)
{
	switch (nType)
	{
	case 1:
		return 1;
	case 12:
		return 1;
	case 6:
		return 1;
	case 7:
		return 1;
	case 8:
		return 1;
	case 10:
		return 1;
	case 15:
		return 1;
	case 18:
		return 1;
	case 140:
		return 1;
	case 170:
		return 1;
	case 171:
		return 1;
	case 172:
		return 1;
	case 200:
		return 1;
	case 201:
		return 1;
	case 202:
		return 1;
	case 173:
		return 1;
	case 174:
		return 1;
	case 175:
		return 1;
	case 176:
		return 1;
	case 177:
		return 1;
	case 178:
		return 1;
	case 203:
		return 1;
	case 204:
		return 1;
	case 205:
		return 1;
	case 188:
		return 1;
	case 189:
		return 1;
	case 191:
		return 1;
	case 192:
		return 1;
	case 193:
		return 1;
	case 194:
		return 1;
	case 197:
		return 1;
	case 198:
		return 1;
	case 206:
		return 1;
	case 210:
		return 1;
	case 208:
		return 1;
	}

	if (nType >= 19 && nType <= 28)
		return 1;
	if (nType == 30)
		return 1;
	if (nType >= 31 && nType <= 36)
		return 1;
	if (nType >= 40 && nType <= 58)
		return 1;
	if (nType >= 60 && nType <= 69)
		return 1;
	if (nType >= 70 && nType < 90)
		return 1;
	if (nType >= 131 && nType < 139)
		return 1;
	if (nType >= 161 && nType < 169)
		return 1;
	if (nType >= 184 && nType < 186)
		return 1;
	
	switch (nItemSIndex)
	{
	case 4146:
		return 1;
	case 4147:
		return 1;
	case 5338:
		return 1;
	case 3451:
	case 3452:
		return 1;
	case 3453:
	case 3454:
		return 1;
	case 5137:
		return 1;
	case 5453:
		return 1;
	case 5454:
		return 1;
	case 646:
		return 1;
	case 647:
		return 1;
	case 3378:
		return 1;
	case 4030:
		return 1;
	case 4031:
		return 1;
	case 4014:
		return 1;
	case 3020:
		return 1;
	case 1773:
		return 1;
	case 4148:
		return 1;
	case 4044:
		return 1;
	case 4045:
		return 1;
	case 4046:
		return 1;
	case 4047:
		return 1;
	case 415:
		return 1;
	case 679:
		return 1;
	case 3478:
		return 1;
	case 241:
		return 1;
	case 3473:
		return 1;
	case 3475:
		return 1;
	case 473:
		return 1;
	case 4149:
		return 1;
	case 489:
		return 1;
	case 3479:
		return 1;
	case 3480:
		return 1;
	case 3210:
		return 1;
	}

	if (nItemSIndex >= 3021 && nItemSIndex <= 3026)
		return 1;
	if (nItemSIndex >= 3445 && nItemSIndex <= 3448)
		return 1;	
	if (nItemSIndex >= 3200 && nItemSIndex < 3300)
		return 1;
	if (nItemSIndex >= 3457 && nItemSIndex <= 3459)
		return 1;
	if (nItemSIndex == 4048)
		return 1;
	if (nItemSIndex == 4049)
		return 1;
	if (nItemSIndex >= 1777 && nItemSIndex <= 1779)
		return 1;	
	if (nItemSIndex >= 4900 && nItemSIndex <= 4910)
		return 1;
	if (nItemSIndex >= 4911 && nItemSIndex <= 4915)
		return 1;

	return 0;
}
