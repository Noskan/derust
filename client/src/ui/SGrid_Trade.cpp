#include "core/pch.h"
#include "ui/SGrid.h"
#include "core/TMGlobal.h"
#include "ui/SControlContainer.h"
#include "render/TMMesh.h"
#include "scene/TMFieldScene.h"
#include "core/TMUtil.h"
#include "common/ItemEffect.h"

/// @file SGrid_Trade.cpp
/// @brief Operações comerciais da grade: compra, venda, troca, loja pessoal e automovimentação.

void SGridControl::BuyItem(int nCellX, int nCellY)
{
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{
			if (nCellX == 4 && nCellY == 7)
			{
				if (pItem->m_pItem->sIndex == 4998)
				{
					MSG_RepurchaseItems ReqUndoShop{};
					ReqUndoShop.Header.ID = pScene->m_pMyHuman->m_dwID;
					ReqUndoShop.Header.Type = MSG_RepurchaseItems_Opcode;

					auto pGridShop = pScene->m_pGridShop;
					ReqUndoShop.target = pScene->m_sShopTarget;
					SendOneMessage((char*)&ReqUndoShop, sizeof(ReqUndoShop));
				}
				else
				{
					MSG_REQShopList stReqShopList{};
					stReqShopList.Header.Type = MSG_REQShopList_Opcode;
					stReqShopList.Header.ID = pScene->m_pMyHuman->m_dwID;
					stReqShopList.TargetID = pScene->m_sShopTarget;

					pScene->m_pGridShop->m_dwMerchantID = pScene->m_sShopTarget;

					SendOneMessage((char*)&stReqShopList, sizeof(stReqShopList));

					pScene->m_dwNPCClickTime = g_pTimerManager->GetServerTime();
				}
				return;
			}

			int SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;
			int line = SourPos / 9;
			int pos = SourPos % 9;
			SourPos = SourPos % 9 + 27 * (SourPos / 9);
			int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
			if (nGridIndex > 7 || nGridIndex < 0)
				nGridIndex = 0;

			IVector2 vecGrid;
			int page = 0;
			auto pMyGrid = pScene->m_pGridInv;
			vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
			page = pScene->m_pGridInv->m_dwControlID - 67072;
			page *= 15;

			if (vecGrid.x == -1 && vecGrid.y == -1)
			{
				for (int i = 0; i < 4; ++i)
				{
					pMyGrid = pScene->m_pGridInvList[i];
					vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
					if (vecGrid.x > -1 && vecGrid.y > -1)
					{
						page = 15 * i;
						break;
					}
				}
			}
			if (page < 0 || page > 45)
				page = 0;

			MSG_Buy stBuy{};
			stBuy.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			stBuy.Header.Type = MSG_Buy_Opcode;
			stBuy.TargetID = m_dwMerchantID;
			if (pScene->m_bIsUndoShoplist)
				stBuy.TargetID = g_pCurrentScene->m_pMyHuman->m_dwID;

			stBuy.TargetCarryPos = SourPos;
			stBuy.MyCarryPos = page + vecGrid.x + 5 * vecGrid.y;

			if (vecGrid.x > -1 && vecGrid.y > -1)
			{
				if (pItem->m_pItem->sIndex != 3331 && pItem->m_pItem->sIndex == 4147)
				{
					memset(&pScene->m_stToto, 0, sizeof(pScene->m_stToto));
					memcpy(&pScene->m_stToto, &stBuy, sizeof(stBuy));
					pScene->m_stToto.Header.Type = 0x3CE;
					if (pScene->m_pTotoPanel)
					{
						if (pScene->m_pShopPanel->IsVisible())
							pScene->SetVisibleShop(0);

						if (pScene->m_pDescPanel->IsVisible())
							pScene->m_pDescPanel->SetVisible(0);

						pScene->m_pTotoPanel->SetVisible(1);
						pScene->m_pTotoNumber_Edit->SetText((char*)"");
						pScene->m_pTotoScoreA_Edit->SetText((char*)"");
						pScene->m_pTotoScoreB_Edit->SetText((char*)"");
						pScene->m_pControlContainer->SetFocusedControl(pScene->m_pTotoNumber_Edit);
					}
					return;
				}

				SendOneMessage((char*)&stBuy, 24);
			}
			else
			{
				auto pListBox = pScene->m_pChatList;
				auto ipNewItem = new SListBoxItem(g_pMessageStringTable[1],
					0xFFFFAAAA,
					0.0f,
					0.0f,
					280.0f,
					16.0f,
					0,
					0x77777777,
					1u,
					0);

				if (ipNewItem)
					pListBox->AddItem(ipNewItem);

				auto pSoundManager = g_pSoundManager;
				if (pSoundManager && pSoundManager->GetSoundData(33))
				{
					pSoundManager->GetSoundData(33)->Play();
				}
			}
		}
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLM)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
		{
			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[47], g_pItemList[pItem->m_pItem->sIndex].Name);			
			g_pCurrentScene->m_pMessageBox->SetMessage(szMsg, 4, 0);
			g_pCurrentScene->m_pMessageBox->SetVisible(1);
			g_pCurrentScene->m_pMessageBox->m_dwArg = m_dwMerchantID | (pItem->m_pItem->sIndex << 16);
		}
	}
}

int SGridControl::TradeItem(int nCellX, int nCellY)
{
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (pItem && pItem->m_GCObj.dwColor == 0xFFFFFFFF)
		{
			int Sourpage = 0;
			if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
			{
				Sourpage = 15 * (m_dwControlID - 67072);
				if (Sourpage < 0 || Sourpage > 45)
					Sourpage = 0;
			}

			int SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

			SGridControl* pGridMyItem[15];
			for (size_t i = 0; i < 15; ++i)
				pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 8448);

			bool bEmptyFind = false;
			for (size_t i = 0; i < 15; ++i)
			{
				if (!pGridMyItem[i]->GetItem(0, 0))
				{
					auto pstItem = new STRUCT_ITEM;
					memcpy(pstItem, pItem->m_pItem, sizeof(STRUCT_ITEM));

					auto newItem = new SGridControlItem(nullptr, pstItem, 0.0f, 0.0f);
					if (newItem)
						pGridMyItem[i]->AddItem(newItem, 0, 0);

					memcpy(&g_pObjectManager->m_stTrade.Item[i], pItem->m_pItem, sizeof(STRUCT_ITEM));

					g_pObjectManager->m_stTrade.CarryPos[i] = Sourpage + SourPos;
					pItem->m_GCObj.dwColor = 0xFFFF0000;
					bEmptyFind = true;
					break;
				}
			}
			if (!bEmptyFind)
				return 0;

			auto pMyCheck = (SButton*)pFScene->m_pControlContainer->FindControl(617);
			auto pOPCheck = (SButton*)pFScene->m_pControlContainer->FindControl(601);
			pMyCheck->m_bSelected = 0;
			pOPCheck->m_bSelected = 0;

			pFScene->m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
			g_pObjectManager->m_stTrade.MyCheck = pMyCheck->m_bSelected;
			g_pObjectManager->m_stTrade.Header.Type = 0x383;
			SendOneMessage((char*)&g_pObjectManager->m_stTrade, sizeof(g_pObjectManager->m_stTrade));
			return 1;
		}
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV2)
	{
		if (pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
		{
			int page = m_dwControlID - 67328;
			if (page >= 3)
				return 0;

			auto pInputGold = (SPanel*)pFScene->m_pControlContainer->FindControl(65885);
			auto pItem = pFScene->m_pCargoGridList[page]->GetItem(nCellX, nCellY);
			auto pRunAutoTrade = (SButton*)pFScene->m_pControlContainer->FindControl(667);

			if (!pRunAutoTrade->IsVisible())
				return 1;

			if (!pItem || !pItem->m_pItem)
				return 1;

			if (BASE_GetItemAbility(pItem->m_pItem, 111))
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[309], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
			if (pItem->m_pItem->sIndex == 508 || pItem->m_pItem->sIndex == 509 || pItem->m_pItem->sIndex == 522 || 
				pItem->m_pItem->sIndex >= 526 && pItem->m_pItem->sIndex <= 537 || pItem->m_pItem->sIndex == 747 || 
				pItem->m_pItem->sIndex == 4905)
			{
				pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[309], 2000);
				pFScene->m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}

			if (!pInputGold->IsVisible() && pItem->m_GCObj.dwColor == -1)
			{
				auto pText = (SText*)pFScene->m_pControlContainer->FindControl(65888);
				auto pEdit = (SEditableText*)pFScene->m_pControlContainer->FindControl(65889);

				pItem->m_GCObj.dwColor = 0xFFFF00FF;

				pFScene->m_nCoinMsgType = 4;
				pFScene->m_nLastAutoTradePos = 40 * page + 5 * pItem->m_nCellIndexY + pItem->m_nCellIndexX;

				pText->SetText(g_pMessageStringTable[142], 0);

				pFScene->m_pControlContainer->SetFocusedControl(pEdit);

				memset(pEdit->m_strComposeText, 0, sizeof(pEdit->m_strComposeText));
				pEdit->SetText((char*)"");
				pInputGold->SetVisible(1);
				pFScene->m_pChatSelectPanel->SetVisible(0);
			}
		}
		return 1;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV3)
	{
		int Sourpage = 0;
		if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
		{
			Sourpage = m_dwControlID - 67072;
			if (Sourpage < 0 || Sourpage > 3)
				Sourpage = 0;
		}

		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
			pFScene->m_MissionClass.ClickInvItem(pItem, pFScene->m_pGridInvList, Sourpage);
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV6)
	{
		int Sourpage = 0;
		if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
		{
			Sourpage = 15 * (m_dwControlID - 67072);
			if (Sourpage < 0 || Sourpage > 45)
				Sourpage = 0;
		}

		auto pItem = GetItem(nCellX, nCellY);

		if (!pItem || pItem->m_GCObj.dwColor != 0xFFFFFFFF)
			return 2;

		int SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;
		SourPos += Sourpage;

		SGridControl* pGridMyItem[3];
		for (size_t i = 0; i < 3; i++)
			pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(6436);

		float nY = 0.0f;
		float nX = 0.0f;
		bool bEmptyFind = false;
		for (int i = 0; i < 3; i++)
		{
			if (!pGridMyItem[i] || pGridMyItem[i]->GetItem(0, 0))
				continue;

			if (i == 0)
			{
				int target1trans = BASE_GetItemAbility(pItem->m_pItem, 112);
				int target1pos = g_pItemList[pItem->m_pItem->sIndex].nPos;
				int target1look = BASE_GetItemAbility(pItem->m_pItem, 18);
				int target1Level = BASE_GetItemAbility(pItem->m_pItem, 87);
				int target1Unique = g_pItemList[pItem->m_pItem->sIndex].nUnique;

				bool bOK = true;

				if (target1trans != 1)
					bOK = false;
				if (!bOK)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[273], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}

				if (target1Level != 6)
					bOK = false;
				if (!bOK)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[302], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}

				if (target1pos & 0x3F)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
			}
			if (i == 1)
			{
				int target2trans = BASE_GetItemAbility(pItem->m_pItem, 112);
				int target2grade = g_pItemList[pItem->m_pItem->sIndex].nUnique / 10;
				int target2pos = g_pItemList[pItem->m_pItem->sIndex].nPos;
				int target2look = BASE_GetItemAbility(pItem->m_pItem, 18);
				int target2Unique = g_pItemList[pItem->m_pItem->sIndex].nUnique;
				int target2Level = BASE_GetItemAbility(pItem->m_pItem, 87);

				bool bOK = true;

				if (target2grade < 4)
					bOK = false;
				if (!bOK)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}

				if (target2Level != 6)
					bOK = false;
				if (!bOK)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[302], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}

				auto pItem1 = pGridMyItem[0]->GetItem(0, 0);
				int target1grade = g_pItemList[pItem1->m_pItem->sIndex].nUnique % 10;
				int nPos = g_pItemList[pItem1->m_pItem->sIndex].nPos;
				int nClass = BASE_GetItemAbility(pItem1->m_pItem, 18);
				int nUnique = g_pItemList[pItem1->m_pItem->sIndex].nUnique;

				if (nUnique != target2Unique)
					bOK = nUnique == 45 && target2Unique == 48 && nPos == target2pos;
				if (nPos != target2pos)
					bOK = false;

				if (!bOK)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[276], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
			}
			if (i == 2)
			{
				nX = 13.0f;
				nY = 28.0f;
			}

			if (i == 0 || i == 1 || i == 2)
			{
				int nItemSanc = BASE_GetItemSanc(pItem->m_pItem);
				if (nItemSanc < 9 || !g_pItemList[pItem->m_pItem->sIndex].nPos)
				{
					pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[192], 2000);
					pFScene->m_pMessagePanel->SetVisible(1, 1);
					return 1;
				}
			}

			auto dst = new STRUCT_ITEM;
			memcpy(dst, pItem->m_pItem, sizeof(STRUCT_ITEM));

			auto newItem = new SGridControlItem(nullptr, dst, nX, nY);
			if (newItem)
				pGridMyItem[i]->AddItem(newItem, 0, 0);

			memcpy(&g_pObjectManager->m_stCombineItem4.Item[i], pItem->m_pItem, sizeof(STRUCT_ITEM));
			g_pObjectManager->m_stCombineItem4.CarryPos[i] = SourPos;
			pItem->m_GCObj.dwColor = 0xFFFF0000;
			bEmptyFind = true;
			break;
		}

		return bEmptyFind != 0;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEINV8)
	{
		int Sourpage = 0;
		if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
		{
			Sourpage = m_dwControlID - 67072;
			if (Sourpage < 0 || Sourpage > 3)
				Sourpage = 0;
		}

		auto pItem = GetItem(nCellX, nCellY);
		if (pItem)
			pFScene->m_ItemMixClass.ClickInvItem(pItem, pFScene->m_pGridInvList, Sourpage);
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEMY)
		return 1;
	if (m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (!pItem)
			return 1;

		SGridControl* pGridMyItem[13];
		for (size_t i = 0; i < 13; ++i)
			pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 65861);

		// TODO: review this code latter, possible buffer overflow.
		for (size_t i = 0; i < 13; ++i)
		{
			if (pGridMyItem[i] != this)
				continue;

			int Page = g_pObjectManager->m_stCombineItem.CarryPos[i] / 15;
			int CellIndexX = g_pObjectManager->m_stCombineItem.CarryPos[i] % 15 % 5;
			int CellIndexY = g_pObjectManager->m_stCombineItem.CarryPos[i] % 15 / 5;

			auto pPickupItem = pGridMyItem[i]->PickupItem(0, 0);
			auto pControlInv = pFScene->m_pGridInvList[Page];

			auto pItemInv = pControlInv->GetItem(CellIndexX, CellIndexY);
			pItemInv->m_GCObj.dwColor = 0xFFFFFFFF;

			g_pObjectManager->m_stCombineItem.CarryPos[i] = -1;

			memset(&g_pObjectManager->m_stCombineItem.Item[i], 0, sizeof(g_pObjectManager->m_stCombineItem.Item[i]));
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickupItem)
				g_pCursor->m_pAttachedItem = nullptr;

			SAFE_DELETE(pPickupItem);
			break;
		}
		return 1;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_ITEMMIX4)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (!pItem)
			return 1;

		SGridControl* pGridMyItem[3];
		for (size_t i = 0; i < 3; ++i)
			pGridMyItem[i] = (SGridControl*)pFScene->m_pControlContainer->FindControl(i + 6436);

		for (size_t i = 0; i < 3; ++i)
		{
			if (pGridMyItem[i] != this)
				continue;

			int Page = g_pObjectManager->m_stCombineItem4.CarryPos[i] / 15;
			int CellIndexX = g_pObjectManager->m_stCombineItem4.CarryPos[i] % 15 % 5;
			int CellIndexY = g_pObjectManager->m_stCombineItem4.CarryPos[i] % 15 / 5;

			auto pPickupItem = pGridMyItem[i]->PickupItem(0, 0);
			auto pControlInv = pFScene->m_pGridInvList[Page];

			auto pItemInv = pControlInv->GetItem(CellIndexX, CellIndexY);
			pItemInv->m_GCObj.dwColor = 0xFFFFFFFF;

			g_pObjectManager->m_stCombineItem4.CarryPos[i] = -1;

			memset(&g_pObjectManager->m_stCombineItem4.Item[i], 0, sizeof(g_pObjectManager->m_stCombineItem4.Item[i]));
			if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickupItem)
				g_pCursor->m_pAttachedItem = nullptr;

			SAFE_DELETE(pPickupItem);
		}

		return 1;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2)
	{
		auto pItem = GetItem(nCellX, nCellY);
		if (!pItem)
			return 1;

		for (size_t i = 0; i < 4; i++)
		{
			auto pGrid = pFScene->m_pGridInvList[i];
			int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);

			IVector2 vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);

			if (vecGrid.x > -1 && vecGrid.y > -1 || BASE_GetItemAbility(pItem->m_pItem, 38) == 2)
			{
				if (pItem && pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
				{
					pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[144], 646, 0);
					pFScene->m_pMessageBox->SetVisible(1);
					pFScene->m_pMessageBox->m_dwArg = m_dwControlID;
				}
				return 1;
			}
		}

		auto pListBox = pFScene->m_pChatList;

		auto pItem2 = new SListBoxItem(g_pMessageStringTable[1], 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0,
			0x77777777, 1, 0);
		if (pItem2)
			pListBox->AddItem(pItem2);

		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(33);
			if (pSoundData)
				pSoundData->Play(0, 0);
		}
		return 1;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP || m_eGridType == TMEGRIDTYPE::GRID_SKILLM ||
		m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXRESULT || m_eGridType == TMEGRIDTYPE::GRID_ITEMMIXNEED ||
		m_eGridType == TMEGRIDTYPE::GRID_MISSION_RESULT || m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEED ||
		m_eGridType == TMEGRIDTYPE::GRID_MISSION_NEEDLIST)
	{
		return 2;
	}

	auto pItem = SelectItem(nCellX, nCellY);
	if (pItem && IsPassiveSkill(pItem->m_pItem->sIndex) == 1)
		return 1;

	if (pItem && pItem->m_GCObj.dwColor != 0xFFFF0000)
		g_pCursor->AttachItem(pItem);
	
	return 1;
}

int SGridControl::SellItem(int nCellX, int nCellY, unsigned int dwFlags, unsigned int wParam)
{
	auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		SGridControl::m_pSellItem = g_pCursor->m_pAttachedItem;
		if (!SGridControl::m_pSellItem)
			return 1;
		if (pScene->m_bIsUndoShoplist)
			return 1;

		if (g_pEventTranslator->m_bCtrl)
		{
			short sDestType = CheckType(SGridControl::m_pSellItem->m_pGridControl->m_eItemType,
				SGridControl::m_pSellItem->m_pGridControl->m_eGridType);

			short sDestPos = CheckPos(SGridControl::m_pSellItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
				sDestPos = SGridControl::m_pSellItem->m_nCellIndexX + 5 * SGridControl::m_pSellItem->m_nCellIndexY;

			MSG_Sell stSell{};
			stSell.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			stSell.Header.Type = MSG_Sell_Opcode;
			stSell.TargetID = m_dwMerchantID;
			stSell.MyType = sDestType;
			stSell.MyPos = sDestPos;
			SendOneMessage((char*)&stSell, sizeof(stSell));
			SGridControl::m_pSellItem = 0;
		}
		else
		{
			char szMessage[128]{};
			sprintf(szMessage, g_pMessageStringTable[342], &g_pItemList[SGridControl::m_pSellItem->m_pItem->sIndex]);
			pScene->m_pMessageBox->SetMessage(szMessage, 890, g_pMessageStringTable[343]);
			pScene->m_pMessageBox->SetVisible(1);
			g_pCursor->m_pAttachedItem = 0;
		}

		m_dwEnableColor = 0;
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_DELETE)
	{
		SGridControl::m_pSellItem = g_pCursor->m_pAttachedItem;

		char szMessage[128]{};
		sprintf(szMessage, g_pItemList[SGridControl::m_pSellItem->m_pItem->sIndex].Name);
		pScene->m_pMessageBox->SetMessage(szMessage, 740, g_pMessageStringTable[18]);
		pScene->m_pMessageBox->SetVisible(1);
		g_pCursor->m_pAttachedItem = 0;
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT1 || 
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT2 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT3 ||
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT4 || 
			 m_eGridType == TMEGRIDTYPE::GRID_QUICKSLOAT5)
	{
		auto pReturnItem = g_pCursor->m_pAttachedItem;
		int itemcheck = 0;
		int itemidx = g_pCursor->m_pAttachedItem->m_pItem->sIndex;

		if (itemidx >= 400 && itemidx <= 409)
			itemcheck = 1;
		else if (itemidx >= 415 && itemidx <= 416)
			itemcheck = 1;
		else if (itemidx >= 428 && itemidx <= 435)
			itemcheck = 1;
		else if (itemidx >= 646 && itemidx <= 647)
			itemcheck = 1;
		else if (itemidx >= 680 && itemidx <= 691)
			itemcheck = 1;
		else if (itemidx >= 3310 && itemidx <= 3312)
			itemcheck = 1;
		else if (itemidx >= 3319 && itemidx <= 3323)
			itemcheck = 1;
		else if (itemidx >= 3368 && itemidx <= 3377)
			itemcheck = 1;
		else if (itemidx >= 3383 && itemidx <= 3384)
			itemcheck = 1;
		else if (itemidx == 3431 || itemidx == 4145 || itemidx == 1739 || itemidx == 3477)
			itemcheck = 1;
		else if (itemidx == 3472 || itemidx == 4097)
			itemcheck = 1;
		else if (itemidx >= 3200 && itemidx <= 3210)
			itemcheck = 1;
		if (itemidx == 3207)
			itemcheck = 0;

		if (!itemcheck)
		{
			g_pCursor->DetachItem();
			return 1;
		}

		if (BASE_GetItemAbility(pReturnItem->m_pItem, 33) >= 1)
			return 1;

		if (m_nNumItem >= 1)
		{
			for (int i = 0; i < m_nNumItem; ++i)
			{
				auto pItem = pScene->m_pQuick_Sloat[(int)m_eGridType - (int)TMEGRIDTYPE::GRID_QUICKSLOAT1]->PickupItem(i, 0);

				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pItem)
					g_pCursor->m_pAttachedItem = nullptr;

				SAFE_DELETE(pItem);
			}
		}

		auto pNewItem = new STRUCT_ITEM;
		if (g_pCursor->m_pAttachedItem)
		{
			memcpy(pNewItem, g_pCursor->m_pAttachedItem->m_pItem, sizeof(STRUCT_ITEM));

			auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
			pNewControlItem->m_nWidth = pNewControlItem->m_nWidth * 0.9f;
			pNewControlItem->m_nHeight = pNewControlItem->m_nHeight * 0.9f;
			pNewControlItem->m_GCObj.m_fWidth = pNewControlItem->m_GCObj.m_fWidth * 0.9f;
			pNewControlItem->m_GCObj.m_fHeight = pNewControlItem->m_GCObj.m_fHeight * 0.9f;

			pScene->m_pQuick_Sloat[(int)m_eGridType - (int)TMEGRIDTYPE::GRID_QUICKSLOAT1]->AddItem(pNewControlItem, nCellX, nCellY);
		}
		g_pCursor->DetachItem();
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		if (!IsSkill(g_pCursor->m_pAttachedItem->m_pItem->sIndex))
			return 1;

		int nSeg = 0;
		if (m_dwControlID == 65645)
			nSeg = 10;

		auto pItem = PickupItem(nCellX, nCellY);
		auto pNewItem = new STRUCT_ITEM;

		memcpy(pNewItem, g_pCursor->m_pAttachedItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0f, 0.0f);
		AddItem(pNewControlItem, nCellX, nCellY);

		if (g_pObjectManager->m_cSelectShortSkill - nSeg == nCellX)
			pNewControlItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pItem);

		auto pMobData = &g_pObjectManager->m_stMobData;
		g_pObjectManager->m_cShortSkill[nSeg + nCellX] = static_cast<char>(g_pItemList[pNewItem->sIndex].nIndexTexture);

		MSG_SetShortSkill stSetShortSkill{};
		stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
		stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;

		memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

		for (int j = 0; j < 20; ++j)
		{
			if (stSetShortSkill.Skill[j] >= 0 && stSetShortSkill.Skill[j] < 96)
			{
				stSetShortSkill.Skill[j] -= 24 * g_pObjectManager->m_stMobData.Class;
			}
			else if (stSetShortSkill.Skill[j] >= 105 && stSetShortSkill.Skill[j] < 153)
			{
				stSetShortSkill.Skill[j] -= 12 * g_pObjectManager->m_stMobData.Class;
			}
		}

		SendOneMessage((char*)&stSetShortSkill, sizeof(stSetShortSkill));

		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(31);
			if (pSoundData)
				pSoundData->Play();
		}

		pScene->UpdateScoreUI(0);
		pScene->UpdateSkillBelt();
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_CUBEBOX)
	{
		int nSeg = 0;
		if (m_dwControlID == 65645)
			nSeg = 10;

		if (!g_pCursor->m_pAttachedItem->m_pItem)
			return 0;

		auto pItem = PickupItem(nCellX, nCellY);

		pScene->UpdateScoreUI(0);
		pScene->UpdateSkillBelt();
	}
	else
	{
		auto pItem = GetItem(nCellX, nCellY);
		int nVolatile = BASE_GetItemAbility(g_pCursor->m_pAttachedItem->m_pItem, 38);
		int nDestVolatile = -1;
		int itemidx = -1;

		if (pItem)
		{
			nDestVolatile = BASE_GetItemAbility(pItem->m_pItem, 38);
			itemidx = pItem->m_pItem->sIndex;
		}

		short sDestType = CheckType(m_eItemType, m_eGridType);
		short sDestPos = CheckPos(m_eItemType);

		if ((((nVolatile >= 4 && nVolatile <= 6 || nVolatile == 9 || nVolatile == 15 || nVolatile == 16 || nVolatile >= 180 && nVolatile <= 183	|| 
			nVolatile >= 235 && nVolatile <= 238 || nVolatile >= 239 && nVolatile <= 240 || nVolatile >= 90 && nVolatile < 95 || nVolatile == 179 || 
			nVolatile == 186 || nVolatile == 196) && 
			!sDestType || nVolatile == 190 && sDestType == 1 && m_dwEnableColor == 0x330000FF) && 
			!nDestVolatile || nVolatile == 241 && sDestType == 1 && nDestVolatile == 16 || 
			(nVolatile == 4 || nVolatile == 5) && sDestType == 1 && m_dwEnableColor == 0x3300FF00 || 
			g_pCursor->m_pAttachedItem->m_pItem->sIndex == 3465 || (nVolatile == 4 || nVolatile == 5) && 
			(sDestType >= 1901 && sDestType <= 1910 || sDestType >= 1234 && sDestType <= 1237 || sDestType >= 1369 && sDestType <= 1372 || 
				sDestType >= 1519 && sDestType <= 1522 || sDestType >= 1669 && sDestType <= 1672 || sDestType == 1714)) && pItem)
		{
			unsigned int dwServerTime = g_pTimerManager->GetServerTime();

			if (pScene->m_dwUseItemTime && dwServerTime - pScene->m_dwUseItemTime < 200)
				return 1;

			short sSrcType = CheckType(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType,
				g_pCursor->m_pAttachedItem->m_pGridControl->m_eGridType);
			short sSrcPos = CheckPos(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType);

			if (sSrcPos == -1)
				sSrcPos = g_pCursor->m_pAttachedItem->m_nCellIndexX	+ 5 * g_pCursor->m_pAttachedItem->m_nCellIndexY;

			if (sDestPos == -1)
				sDestPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

			if (sSrcType == 2)
				return 0;

			if (nVolatile >= 239 && nVolatile <= 240)
			{
				_nCellX = nCellX;
				_nCellY = nCellY;
				_dwFlags = dwFlags;
				_wParam = wParam;
				pScene->m_pitemPassGrid = this;
				pScene->VisibleInputPass();
				return 1;
			}

			int Sourpage = 0;
			if (g_pCursor->m_pAttachedItem->m_pGridControl->m_dwControlID >= 67072 && 
				g_pCursor->m_pAttachedItem->m_pGridControl->m_dwControlID <= 67075)
			{
				Sourpage = 15 * (g_pCursor->m_pAttachedItem->m_pGridControl->m_dwControlID - 67072);
				if (Sourpage < 0 || Sourpage > 45)
					Sourpage = 0;
			}

			short sDestpage = 0;

			if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
			{
				sDestpage = 15 * (m_dwControlID - 67072);
				if (sDestpage < 0 || sDestpage > 45)
					sDestpage = 0;
			}

			if (Sourpage / 15 == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
				return 1;
			if (Sourpage / 15 == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
				return 1;

			MSG_UseItem stUseItem{};

			stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
			stUseItem.Header.Type = 883;
			stUseItem.SourType = sSrcType;
			stUseItem.SourPos = Sourpage + sSrcPos;
			stUseItem.DestType = sDestType;
			stUseItem.DestPos = sDestpage + sDestPos;
			stUseItem.ItemID = 0;
			stUseItem.GridX = 0;
			stUseItem.GridY = 0;
			SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

			pScene->m_dwUseItemTime = dwServerTime;
			int nCellTempX = g_pCursor->m_pAttachedItem->m_nCellIndexX;
			int nCellTempY = g_pCursor->m_pAttachedItem->m_nCellIndexY;
			auto pGrid = g_pCursor->m_pAttachedItem->m_pGridControl;
			auto pPickedItem = pGrid->GetItem(nCellTempX, nCellTempY);

			int nAmount = 0;

			if (pPickedItem)
			{
				nAmount = BASE_GetItemAmount(pPickedItem->m_pItem);

				if (pPickedItem->m_pItem->sIndex >= 2330 && pPickedItem->m_pItem->sIndex < 2390)
					nAmount = 0;
				if (nAmount <= 1)
				{
					pGrid->PickupItem(nCellTempX, nCellTempY);
					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
						g_pCursor->m_pAttachedItem = nullptr;

					SAFE_DELETE(pPickedItem);
				}
				else
				{
					BASE_SetItemAmount(pPickedItem->m_pItem, nAmount - 1);
					auto pGItem = pPickedItem;
					sprintf(pPickedItem->m_GCText.strString, "%2d", nAmount - 1);
					pGItem->m_GCText.pFont->SetText(pGItem->m_GCText.strString, pGItem->m_GCText.dwColor, 0);
				}
			}

			g_pCursor->DetachItem();
			if (nAmount <= 1)
			{				
				if (!sSrcType)
					memset(&g_pObjectManager->m_stMobData.Equip[sSrcPos], 0, sizeof(STRUCT_ITEM));
				else if (sSrcType == 1)
					memset(&g_pObjectManager->m_stMobData.Carry[sSrcPos], 0, sizeof(STRUCT_ITEM));
				else if (sSrcType == 2)
					memset(&g_pObjectManager->m_stItemCargo[sSrcPos], 0, sizeof(STRUCT_ITEM));
			}
		}
		else if (nVolatile == 190 && sDestType == 1 && m_dwEnableColor != 0x330000FF && pItem)
		{
			g_pCurrentScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[303], 3000);
			g_pCurrentScene->m_pMessagePanel->SetVisible(1, 1);
		}
		else if (CanChangeItem(g_pCursor->m_pAttachedItem, nCellX, nCellY, 0))
		{
			int page = m_dwControlID - 67072;
			if (page == 2 && g_pObjectManager->m_stMobData.Carry[60].sIndex != 3467)
				return 0;
			if (page == 3 && g_pObjectManager->m_stMobData.Carry[61].sIndex != 3467)
				return 0;

			SGridControl::m_pLastAttachedItem = g_pCursor->m_pAttachedItem;

			auto pos = g_pCursor->GetPos();
			g_pCurrentScene->OnMouseEvent(dwFlags, wParam, (int)pos.x, (int)pos.y);
		}
	}
	return 2;
}

int SGridControl::SellItem2()
{
	int nCellX = _nCellX;
	int nCellY = _nCellY;
	unsigned int dwFlags = _dwFlags;
	int wParam = _wParam;

	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
	{
		auto pScene = g_pCurrentScene;
		SGridControl::m_pSellItem = g_pCursor->m_pAttachedItem;
		if (!SGridControl::m_pSellItem)
			return 1;

		if (!g_pEventTranslator->m_bCtrl)
		{
			char szMessage[128]{};
			sprintf(szMessage, g_pMessageStringTable[342], g_pItemList[SGridControl::m_pSellItem->m_pItem->sIndex].Name);
			pScene->m_pMessageBox->SetMessage(szMessage, 890, g_pMessageStringTable[343]);
			pScene->m_pMessageBox->SetVisible(1);
			g_pCursor->m_pAttachedItem = 0;
		}
		else
		{
			short sDestType = CheckType(SGridControl::m_pSellItem->m_pGridControl->m_eItemType,
				SGridControl::m_pSellItem->m_pGridControl->m_eGridType);
			short sDestPos = CheckPos(SGridControl::m_pSellItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
				sDestPos = SGridControl::m_pSellItem->m_nCellIndexX + 5 * SGridControl::m_pSellItem->m_nCellIndexY;

			MSG_Sell stSell{};
			stSell.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			stSell.Header.Type = MSG_Sell_Opcode;
			stSell.TargetID = m_dwMerchantID;
			stSell.MyType = sDestType;
			stSell.MyPos = sDestPos;
			SendOneMessage((char*)&stSell, sizeof(stSell));
			SGridControl::m_pSellItem = 0;
		}
		m_dwEnableColor = 0;
		return 2;
	}
	if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
	{
		if (!IsSkill(g_pCursor->m_pAttachedItem->m_pItem->sIndex))
			return 1;

		int nSeg = 0;
		if (m_dwControlID == 65645)
			nSeg = 10;

		auto pReturnItem = PickupItem(nCellX, nCellY);

		auto pNewItem = new STRUCT_ITEM;
		memcpy(pNewItem, g_pCursor->m_pAttachedItem->m_pItem, sizeof(STRUCT_ITEM));
			
		auto pNewControlItem = new SGridControlItem(0, pNewItem, 0.0, 0.0);
		AddItem(pNewControlItem, nCellX, nCellY);

		if (g_pObjectManager->m_cSelectShortSkill - nSeg == nCellX)
			pNewControlItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pReturnItem);

		auto pMobData = &g_pObjectManager->m_stMobData;
		g_pObjectManager->m_cShortSkill[nSeg + nCellX] = static_cast<char>(g_pItemList[pNewItem->sIndex].nIndexTexture);

		MSG_SetShortSkill stSetShortSkill{};
		stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
		stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;
		memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));

		for (int i = 0; i < 20; ++i)
		{
			if (stSetShortSkill.Skill[i] >= 0 && stSetShortSkill.Skill[i] < 96)
			{
				stSetShortSkill.Skill[i] -= 24 * g_pObjectManager->m_stMobData.Class;
			}
			else if (stSetShortSkill.Skill[i] >= 105 && stSetShortSkill.Skill[i] < 153)
			{
				stSetShortSkill.Skill[i] -= 12 * g_pObjectManager->m_stMobData.Class;
			}
		}

		SendOneMessage((char*)&stSetShortSkill, sizeof(stSetShortSkill));

		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(31);
			if(pSoundData)
				pSoundData->Play(0, 0);
		}

		pFScene->UpdateScoreUI(0);
		pFScene->UpdateSkillBelt();
		return 2;
	}
	
	auto pItem = GetItem(nCellX, nCellY);
	int nVolatile = BASE_GetItemAbility(g_pCursor->m_pAttachedItem->m_pItem, 38);
	int nDestVolatile = -1;
	if (pItem)
		nDestVolatile = BASE_GetItemAbility(pItem->m_pItem, 38);

	short sDestType = CheckType(m_eItemType, m_eGridType);
	short sDestPos = CheckPos(m_eItemType);
	if (((nVolatile >= 4 && nVolatile <= 6
		|| nVolatile == 9
		|| nVolatile == 15
		|| nVolatile == 16
		|| nVolatile >= 180 && nVolatile <= 183
		|| nVolatile >= 235 && nVolatile <= 238
		|| nVolatile >= 239 && nVolatile <= 240
		|| nVolatile >= 90 && nVolatile < 95
		|| nVolatile == 179
		|| nVolatile == 186
		|| nVolatile == 196)
		&& !sDestType
		|| nVolatile == 190 && sDestType == 1 && m_dwEnableColor == 0x330000FF)
		&& !nDestVolatile
		&& pItem)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();
		if (pFScene->m_dwUseItemTime && dwServerTime - pFScene->m_dwUseItemTime < 200)
			return 1;

		short sSrcType = CheckType(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType,
			g_pCursor->m_pAttachedItem->m_pGridControl->m_eGridType);

		short sSrcPos = CheckPos(g_pCursor->m_pAttachedItem->m_pGridControl->m_eItemType);

		if (sSrcPos == -1)
			sSrcPos = g_pCursor->m_pAttachedItem->m_nCellIndexX	+ 5 * g_pCursor->m_pAttachedItem->m_nCellIndexY;

		if (sDestPos == -1)
			sDestPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;

		pFScene->m_dwUseItemTime = dwServerTime;
		int nCellTempX = g_pCursor->m_pAttachedItem->m_nCellIndexX;
		int nCellTempY = g_pCursor->m_pAttachedItem->m_nCellIndexY;
		auto pGrid = g_pCursor->m_pAttachedItem->m_pGridControl;
		auto pPickedItem = pGrid->GetItem(nCellTempX, nCellTempY);
		int nAmount = 0;

		if (pPickedItem)
		{
			nAmount = BASE_GetItemAmount(pPickedItem->m_pItem);
			if (pPickedItem->m_pItem->sIndex >= 2330 && pPickedItem->m_pItem->sIndex < 2390)
				nAmount = 0;
			if (nAmount <= 1)
			{
				pGrid->PickupItem(nCellTempX, nCellTempY);
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
					g_pCursor->m_pAttachedItem = 0;

				SAFE_DELETE(pPickedItem);
			}
			else
			{
				BASE_SetItemAmount(pPickedItem->m_pItem, nAmount - 1);
				auto pGItem = pPickedItem;
				sprintf(pPickedItem->m_GCText.strString, "%2d", nAmount - 1);
				pGItem->m_GCText.pFont->SetText(pGItem->m_GCText.strString, pGItem->m_GCText.dwColor, 0);
			}
		}

		g_pCursor->DetachItem();
		if (nAmount <= 1)
		{
			if (!sSrcType)
				memset(&g_pObjectManager->m_stMobData.Equip[sSrcPos], 0, sizeof(STRUCT_ITEM));
			else if (sSrcType == 1)
				memset(&g_pObjectManager->m_stMobData.Carry[sSrcPos], 0, sizeof(STRUCT_ITEM));
			else if (sSrcType == 2)
				memset(&g_pObjectManager->m_stItemCargo[sSrcPos], 0, sizeof(STRUCT_ITEM));
		}
	}
	else if (nVolatile == 190 && sDestType == 1 && m_dwEnableColor != 0x330000FF && pItem)
	{
		pFScene->m_pMessagePanel->SetMessage(g_pMessageStringTable[303], 3000);
		pFScene->m_pMessagePanel->SetVisible(1, 1);
	}
	else if (CanChangeItem(g_pCursor->m_pAttachedItem, nCellX, nCellY, 0))
	{
		SGridControl::m_pLastAttachedItem = g_pCursor->m_pAttachedItem;

		auto vecPos = g_pCursor->GetPos();
		pFScene->OnMouseEvent(dwFlags, wParam, (int)vecPos.x, (int)vecPos.y);
	}

	return 2;
}

void SGridControl::SwapItem(int nCellX, int nCellY, int nCellVWidth, int nCellVHeight, STRUCT_ITEM* pItem)
{
	auto pGeom = SGridControl::m_pLastAttachedItem->GetGeomControl();
	auto nWidth = (int)(pGeom->nWidth / (float)nCellVWidth);
	auto nHeight = (int)(pGeom->nHeight / (float)nCellVHeight);
	unsigned int nAtItemPos = 0;

	if (SGridControl::m_pLastAttachedItem)
	{
		memcpy(pItem, SGridControl::m_pLastAttachedItem->m_pItem, sizeof(STRUCT_ITEM));
		nAtItemPos = BASE_GetItemAbility(pItem, 17);
	}

	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	int page = pFScene->m_pGridInv->m_dwControlID - 67072;
	if ((page != 2 || g_pObjectManager->m_stMobData.Carry[60].sIndex == 3467) && 
		(page != 3 || g_pObjectManager->m_stMobData.Carry[61].sIndex == 3467))
	{
		if (m_eItemType == TMEITEMTYPE::ITEMTYPE_NONE)
		{
			short sDestType = CheckType(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType, 
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_eGridType);

			short sDestPos = CheckPos(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType);
			if (sDestPos == -1)
				sDestPos = SGridControl::m_pLastAttachedItem->m_nCellIndexX + 5 * SGridControl::m_pLastAttachedItem->m_nCellIndexY;

			int Sourpage = 0;
			int Destpage = 0;

			if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
			{
				Sourpage = 15 * (m_dwControlID - 67072);
				if (Sourpage < 0 || Sourpage > 45)
					Sourpage = 0;
			}
			if (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID >= 67072 && 
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID <= 67075)
			{
				Destpage = 15 * (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID - 67072);
				if (Destpage < 0 || Destpage > 45)
					Destpage = 0;
			}
			if (m_dwControlID >= 67328 && m_dwControlID <= 67330)
			{
				Sourpage = 40 * (m_dwControlID - 67328);
				if (Sourpage < 0 || Sourpage > 80)
					Sourpage = 0;
			}
			if (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID >= 67328 && 
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID <= 67330)
			{
				Destpage = 40 * (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID - 67328);
				if (Destpage < 0 || Destpage > 80)
					Destpage = 0;
			}

			MSG_SwapItem stSwapItem{};
			stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
			stSwapItem.Header.Type = MSG_SwapItem_Opcode;
			stSwapItem.SourType = static_cast<char>(CheckType(m_eItemType, m_eGridType));
			stSwapItem.SourPos = Sourpage + nCellX + 5 * nCellY;
			stSwapItem.DestType = static_cast<char>(sDestType);
			stSwapItem.DestPos = Destpage + sDestPos;
			stSwapItem.TargetID = TMFieldScene::m_dwCargoID;

			if (Destpage + sDestPos != stSwapItem.SourPos || 
				stSwapItem.SourType != stSwapItem.DestType)
			{
				SendOneMessage((char*)&stSwapItem, sizeof(stSwapItem));
			}
		}
		else if ((nAtItemPos & (int)m_eItemType) == (int)m_eItemType)
		{
			short sSrcType = CheckType(m_eItemType, m_eGridType);
			short sSrcPos = CheckPos(m_eItemType);

			short sDestType = CheckType(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType,
				SGridControl::m_pLastAttachedItem->m_pGridControl->m_eGridType);
			short sDestPos = CheckPos(SGridControl::m_pLastAttachedItem->m_pGridControl->m_eItemType);

			int nAX = SGridControl::m_pLastAttachedItem->m_nCellIndexX;
			int nAY = SGridControl::m_pLastAttachedItem->m_nCellIndexY;

			if (sSrcType || sDestType)
			{
				int Sourpage = 0;
				int Destpage = 0;

				if (sSrcType == 1)
				{
					Sourpage = 15 * (m_dwControlID - 67072);
					if (Sourpage < 0 || Sourpage > 45)
						Sourpage = 0;
				}
				if (sDestType == 1)
				{
					Destpage = 15 * (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID - 67072);
					if (Destpage < 0 || Destpage > 45)
						Destpage = 0;
				}
				if (sSrcType == 2)
				{
					Sourpage = 40 * (m_dwControlID - 67328);
					if (Sourpage < 0 || Sourpage > 80)
						Sourpage = 0;
				}
				if (sDestType == 2)
				{
					Destpage = 40 * (SGridControl::m_pLastAttachedItem->m_pGridControl->m_dwControlID - 67328);
					if (Destpage < 0 || Destpage > 80)
						Destpage = 0;
				}

				MSG_SwapItem stSwapItem{};
				stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
				stSwapItem.Header.Type = MSG_SwapItem_Opcode;
				stSwapItem.SourType = static_cast<char>(sSrcType);
				stSwapItem.SourPos = Sourpage + sSrcPos;
				stSwapItem.DestType = static_cast<char>(sDestType);
				stSwapItem.TargetID = TMFieldScene::m_dwCargoID;

				if (sDestType)
					stSwapItem.DestPos = Destpage + nAX + 5 * nAY;
				else
					stSwapItem.DestPos = static_cast<char>(sDestPos);

				if (stSwapItem.DestPos != stSwapItem.SourPos ||
					stSwapItem.SourType != stSwapItem.DestType)
				{
					SendOneMessage((char*)&stSwapItem, sizeof(stSwapItem));
				}
			}
		}

		SGridControl::m_pLastAttachedItem = 0;
		return;
	}
}

char SGridControl::AutoSellShowPrice(char* Price)
{
	if (m_eGridType != TMEGRIDTYPE::GRID_TRADEMY2 && m_eGridType != TMEGRIDTYPE::GRID_TRADEOP)
		return 0;

	int nPrice = m_nTradeMoney;

	char szPrice[128];
	char szPrice2[128];
	char szPrice3[128];
	char szPrice4[128];

	sprintf(szPrice2, "%d", nPrice % 10000);
	sprintf(szPrice3, g_pMessageStringTable[283], nPrice / 10000 % 10000);
	sprintf(szPrice4, g_pMessageStringTable[282], nPrice / 100000000);
	if (nPrice % 10000 <= 0)
	{
		if (nPrice / 10000 % 10000 <= 0)
		{
			if (nPrice / 100000000 <= 0)
				sprintf(szPrice, "");
			else
				sprintf(szPrice, "%s %s", szPrice4, g_pMessageStringTable[284]);
		}
		else if (nPrice / 100000000 <= 0)
		{
			sprintf(szPrice, "%s %s", szPrice3, g_pMessageStringTable[284]);
		}
		else
		{
			sprintf(szPrice, "%s %s %s", szPrice4, szPrice3, g_pMessageStringTable[284]);
		}
	}
	else if (nPrice / 10000 % 10000 <= 0)
	{
		if (nPrice / 100000000 <= 0)
			sprintf(szPrice, "%s %s", szPrice2, g_pMessageStringTable[284]);
		else
			sprintf(szPrice, "%s %s %s ", szPrice4, szPrice2, g_pMessageStringTable[284]);
	}
	else if (nPrice / 100000000 <= 0)
	{
		sprintf(szPrice, "%s %s%s", szPrice3, szPrice2, g_pMessageStringTable[284]);
	}
	else
	{
		sprintf(szPrice, "%s %s %s %s", szPrice4, szPrice3, szPrice2, g_pMessageStringTable[284]);
	}

	sprintf(Price, "%s", szPrice);
	return 1;
}

char SGridControl::automove(int nCellX, int nCellY)
{
	auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	auto pItem = SelectItem(nCellX, nCellY);

	if (pItem && pItem->m_GCObj.dwColor != 0xFFFF0000)
	{
		unsigned int NewItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
		for (int NewItemPosConv = 0; ; ++NewItemPosConv)
		{
			NewItemPos /= 2;
			if (!NewItemPos)
				break;
		}

		NewItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
		short sDestType = CheckType(pItem->m_pGridControl->m_eItemType, pItem->m_pGridControl->m_eGridType);
		short sDestPos = CheckPos(pItem->m_pGridControl->m_eItemType);

		int Destpage = 0;
		if (m_dwControlID >= 67072 && m_dwControlID <= 67075)
		{
			Destpage = 15 * (m_dwControlID - 67072);
			if (Destpage < 0 || Destpage > 45)
				Destpage = 0;
		}
		if (m_dwControlID >= 67328 && m_dwControlID <= 67330)
		{
			Destpage = 40 * (m_dwControlID - 67328);
			if (Destpage < 0 || Destpage > 80)
				Destpage = 0;
		}

		int Type;
		if (sDestType == 1)
			Type = 2;
		else
		{
			if (sDestType != 2)
				return 0;
			Type = 1;
		}

		if (sDestPos == -1)
			sDestPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;
		sDestPos += Destpage;

		int nAX = pItem->m_nCellIndexX;
		int nAY = pItem->m_nCellIndexY;
		auto pMobData = &g_pObjectManager->m_stMobData;

		IVector2 vecGrid;
		int page = 0;
		if (Type == 1)
		{
			auto pMyGrid = pScene->m_pGridInv;
			int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
			if (nGridIndex > 7 || nGridIndex < 0)
				nGridIndex = 0;

			vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
			page = pScene->m_pGridInv->m_dwControlID - 67072;
			page *= 15;

			if (vecGrid.x == -1)
			{
				for (int i = 0; i < 4; ++i)
				{
					pMyGrid = pScene->m_pGridInvList[i];
					nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
					if (nGridIndex > 7 || nGridIndex < 0)
						nGridIndex = 0;

					vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
					if (vecGrid.x != -1)
					{
						page = 15 * i;
						break;
					}
				}
			}
			if (page < 0 || page > 45)
				page = 0;
		}
		else if (Type != 2)
			return 0;
		else
		{
			auto pMyGrid = pScene->m_pCargoGrid;
			int nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
			if (nGridIndex > 7 || nGridIndex < 0)
				nGridIndex = 0;

			vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
			page = pScene->m_pCargoGrid->m_dwControlID - 67328;
			page *= 40;

			if (vecGrid.x == -1)
			{
				for (int j = 0; j < 3; ++j)
				{
					pMyGrid = pScene->m_pCargoGridList[j];
					nGridIndex = BASE_GetItemAbility(pItem->m_pItem, 33);
					if (nGridIndex > 7 || nGridIndex < 0)
						nGridIndex = 0;

					vecGrid = pMyGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0], g_pItemGridXY[nGridIndex][1]);
					if (vecGrid.x != -1)
					{
						page = 40 * j;
						break;
					}
				}
			}
			if (page < 0 || page > 80)
				page = 0;
		}

		if (vecGrid.x == -1)
			return 0;

		MSG_SwapItem stSwapItem{};
		stSwapItem.Header.ID = g_pObjectManager->m_dwCharID;
		stSwapItem.Header.Type = MSG_SwapItem_Opcode;
		stSwapItem.SourType = Type;
		stSwapItem.SourPos = page + vecGrid.x + 5 * vecGrid.y;
		stSwapItem.DestType = static_cast<char>(sDestType);
		stSwapItem.DestPos = static_cast<char>(sDestPos);
		stSwapItem.TargetID = TMFieldScene::m_dwCargoID;
		if ((unsigned char)sDestPos != (unsigned char)stSwapItem.SourPos 
			|| stSwapItem.SourType != stSwapItem.DestType)
		{
			SendOneMessage((char*)&stSwapItem, 20);
			pScene->m_pMouseOverHuman = 0;
		}
	}

	return 1;
}
