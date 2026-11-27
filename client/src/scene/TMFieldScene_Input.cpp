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

/// @file TMFieldScene_Input.cpp
/// @brief Tratamento de teclado, atalhos (OnKey*), mouse e interacao com NPCs em campo.

/// Processa entrada de caracteres digitados no chat e campos de texto.
int TMFieldScene::OnCharEvent(char iCharCode, int lParam)
{
	if (m_bAirMove == 1)
		return 0;

	DWORD dwServerTime = g_pTimerManager->GetServerTime();

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

	if (m_pMessageBox->IsVisible() == 1
		&& (m_pMessageBox->m_dwMessage == 601 || m_pMessageBox->m_dwMessage == 927)
		&& iCharCode == 13)
	{
		return 1;
	}

	if (g_nKeyType == 1)
	{
		if (iCharCode == 27)
		{
			OnESC();
			return 1;
		}

		if (TMScene::OnCharEvent(iCharCode, lParam) == 1)
			return 1;
	}
	else
	{
		if (TMScene::OnCharEvent(iCharCode, lParam) == 1)
			return 1;

		if (iCharCode == 27)
		{
			OnESC();
			return 1;
		}
	}

	if (m_bCriticalError == 1)
		return 1;

	if (OnKeyTotoTab(iCharCode, lParam))
		return 1;

	if (OnKeyCamView(iCharCode, lParam))
		return 1;

	if (g_nKeyType)
		return 0;

	if (UseQuickSloat(iCharCode))
		return 1;

	if (OnKeyTotoEnter(iCharCode, lParam))
		return 1;

	if (OnKeyDebug(iCharCode, lParam))
		return 1;

	if (OnKeySkill(iCharCode, lParam))
		return 1;

	if (OnKeyDash(iCharCode, lParam))
		return 1;

	if (OnKeyPlus(iCharCode, lParam))
		return 1;

	if (OnKeyPK(iCharCode, lParam))
		return 1;

	if (OnKeyName(iCharCode, lParam))
		return 1;

	if (OnKeyAutoTarget(iCharCode, lParam))
		return 1;

	if (OnKeyHelp(iCharCode, lParam))
		return 1;

	if (OnKeyRun(iCharCode, lParam))
		return 1;

	if (OnKeyFeedMount(iCharCode, lParam))
		return 1;

	if (g_pObjectManager->m_stMobData.CurrentScore.Hp > 0)
	{
		if (OnKeyHPotion(iCharCode, lParam))
			return 1;

		if (OnKeyMPotion(iCharCode, lParam))
			return 1;

		if (OnKeyPPotion(iCharCode, lParam))
			return 1;

		if (OnKeySkillPage(iCharCode, lParam))
			return 1;

		if (OnKeyQuestLog(iCharCode, lParam))
			return 1;

		if (OnKeyReverse(iCharCode, lParam))
			return 1;

		if (OnKeyAutoRun(iCharCode, lParam))
			return 1;

		if (OnKeyGuildOnOff(iCharCode, lParam))
			return 1;

		if (OnKeyShortSkill(iCharCode, lParam))
			return 1;
	}

	if (OnKeyVisibleSkill(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleInven(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleCharInfo(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleMinimap(iCharCode, lParam))
		return 1;

	if (OnKeyVisibleParty(iCharCode, lParam))
		return 1;

	if (OnKeyReturn(iCharCode, lParam))
		return 1;
	
	return 0;
}

/// Roteia teclas de atalho (habilidades, inventario, mapa, PK, auto-caca) na cena de campo.
int TMFieldScene::OnKeyDownEvent(unsigned int iKeyCode)
{
	DWORD dwServerTime = g_pTimerManager->GetServerTime();

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

	if (TMScene::OnKeyDownEvent(iKeyCode) == 1)
		return 1;

	if (iKeyCode == VK_INSERT)
	{
		MSG_MessageWhisper stWhisper{};

		stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
		stWhisper.Header.Type = MSG_MessageWhisper_Opcode;

		sprintf_s(stWhisper.MobName, "time");

		SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
	}

	if (iKeyCode >= VK_F1 && iKeyCode <= VK_F4)
	{
		if (!m_pInvenPanel->IsVisible())
			SetVisibleInventory();

		switch (iKeyCode)
		{
		case 112:
			m_pGridInvList[0]->SetVisible(0);
			m_pGridInvList[1]->SetVisible(0);
			m_pGridInvList[2]->SetVisible(0);
			m_pGridInvList[3]->SetVisible(0);
			m_pGridInv = m_pGridInvList[0];
			m_pGridInv->SetVisible(1);
			m_pInvPageBtn1->SetTextureSetIndex(527);
			m_pInvPageBtn2->SetTextureSetIndex(528);
			m_pInvPageBtn3->SetTextureSetIndex(528);
			m_pInvPageBtn4->SetTextureSetIndex(528);
			break;
		case 113:
			m_pGridInvList[0]->SetVisible(0);
			m_pGridInvList[1]->SetVisible(0);
			m_pGridInvList[2]->SetVisible(0);
			m_pGridInvList[3]->SetVisible(0);
			m_pGridInv = m_pGridInvList[1];
			m_pGridInv->SetVisible(1);
			m_pInvPageBtn1->SetTextureSetIndex(528);
			m_pInvPageBtn2->SetTextureSetIndex(527);
			m_pInvPageBtn3->SetTextureSetIndex(528);
			m_pInvPageBtn4->SetTextureSetIndex(528);
			break;
		case 114:
			m_pGridInvList[0]->SetVisible(0);
			m_pGridInvList[1]->SetVisible(0);
			m_pGridInvList[2]->SetVisible(0);
			m_pGridInvList[3]->SetVisible(0);
			m_pGridInv = m_pGridInvList[2];
			m_pGridInv->SetVisible(1);
			m_pInvPageBtn1->SetTextureSetIndex(528);
			m_pInvPageBtn2->SetTextureSetIndex(528);
			m_pInvPageBtn3->SetTextureSetIndex(527);
			m_pInvPageBtn4->SetTextureSetIndex(528);
			break;
		case 115:
			m_pGridInvList[0]->SetVisible(0);
			m_pGridInvList[1]->SetVisible(0);
			m_pGridInvList[2]->SetVisible(0);
			m_pGridInvList[3]->SetVisible(0);
			m_pGridInv = m_pGridInvList[3];
			m_pGridInv->SetVisible(1);
			m_pInvPageBtn1->SetTextureSetIndex(528);
			m_pInvPageBtn2->SetTextureSetIndex(528);
			m_pInvPageBtn3->SetTextureSetIndex(528);
			m_pInvPageBtn4->SetTextureSetIndex(527);
			break;
		}

		// Fix the "lock" missing on change inv
		Bag_View();
	}

	if (iKeyCode == VK_F11)
	{
		if (dwServerTime < m_dwKeyTime + 500 || m_bAirMove == 1)
			return 1;

		if (m_pMyHuman && m_pMyHuman->m_fProgressRate > 0.0f && m_pMyHuman->m_fProgressRate < 0.89999998f)
			return 1;

		int page{};

		SGridControl* pGridInv{};
		SGridControlItem* pItem{};
		int nX{};
		int nY{};
		int bFind{};

		for (int i = 0; i < 4; ++i)
		{
			pGridInv = m_pGridInvList[i];

			for (nY = 0; nY < 3; ++nY)
			{
				for (nX = 0; nX < 5; ++nX)
				{
					pItem = pGridInv->GetItem(nX, nY);

					if (pItem && BASE_GetItemAbility(pItem->m_pItem, EF_VOLATILE) == 11)
					{
						bFind = 1;
						page = 15 * i;
						break;
					}
				}
				if (bFind == 1)
					break;
			}
			if (bFind == 1)
				break;
		}

		if (bFind == 1 && pItem)
		{
			if (BASE_GetItemAbility(pItem->m_pItem, 38) == 11)
			{
				short SourPos = nX + 5 * nY;
				m_dwGetItemTime = g_pTimerManager->GetServerTime();
				m_dwLastTeleport = m_dwGetItemTime;
				m_cLastTeleport = 1;

				MSG_STANDARDPARM stDelayStart{};

				stDelayStart.Header.ID = m_pMyHuman->m_dwID;
				stDelayStart.Header.Type = MSG_DelayStart_Opcode;
				stDelayStart.Parm = 1;
				SendOneMessage((char*)&stDelayStart, sizeof(MSG_STANDARDPARM));

				m_stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
				m_stUseItem.Header.Type = MSG_UseItem_Opcode;
				m_stUseItem.SourType = 1;
				m_stUseItem.SourPos = page + SourPos;
				m_stUseItem.ItemID = 0;
				m_stUseItem.GridX = static_cast<unsigned short>(m_pMyHuman->m_vecPosition.x);
				m_stUseItem.GridY = static_cast<unsigned short>(m_pMyHuman->m_vecPosition.y);

				int nAmount = BASE_GetItemAmount(pItem->m_pItem);

				if (nAmount > 1)
				{
					BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);

					sprintf_s(pItem->m_GCText.strString, "%2d", nAmount - 1);

					pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
				}
				else
				{
					auto pPickedItem = pGridInv->PickupItem(nX, nY);

					if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
						g_pCursor->m_pAttachedItem = nullptr;

					if (pPickedItem)
						delete pPickedItem;
				}

				if (nAmount <= 1)
					memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

				if (g_pSoundManager)
				{
					auto pSoundData = g_pSoundManager->GetSoundData(54);

					if (pSoundData)
						pSoundData->Play(0, 0);
				}
			}

			UpdateScoreUI(0);

			m_dwKeyTime = dwServerTime;
		}

		return 1;
	}

	if (iKeyCode >= VK_NUMPAD0 && iKeyCode <= VK_NUMPAD9)
	{
		return OnKeyNumPad(iKeyCode);
	}

	if (iKeyCode == VK_PRIOR)
	{
		SListBox* pChatList{};

		if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)
			pChatList = m_pMsgList;
		else
			pChatList = m_pChatList;

		if (pChatList && pChatList->IsVisible() == 1 && pChatList->m_pScrollBar)
			pChatList->m_pScrollBar->Up();

		return 1;
	}

	if (iKeyCode == VK_NEXT)
	{
		SListBox* pChatList{};

		if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)
			pChatList = m_pMsgList;
		else
			pChatList = m_pChatList;

		if (pChatList && pChatList->IsVisible() == 1 && pChatList->m_pScrollBar)
			pChatList->m_pScrollBar->Down();

		return 1;
	}

	return 0;
}

/// Processa cliques, arraste de itens nos grids e selecao de alvos no mundo 3D.
int TMFieldScene::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	DWORD dwServerTime = g_pTimerManager->GetServerTime();

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

	if (dwServerTime < m_dwNPCClickTime + 1000)
		return 1;

	if (m_bAirMove == 1)
		return 0;

	if (TMScene::OnMouseEvent(dwFlags, wParam, nX, nY) == 1)
		return 1;

	if (g_bActiveWB == 1)
		return 1;

	if (m_bCriticalError == 1)
		return 1;

	for (int nModIndex = 0; nModIndex < 8; ++nModIndex)
	{
		if (m_pControlContainer->m_pModalControl[nModIndex] &&
			m_pControlContainer->m_pModalControl[nModIndex]->IsVisible())
		{
			if (m_pControlContainer->m_pModalControl[nModIndex] != static_cast<SControl*>(m_pMessageBox))
				return 1;

			if (m_pMessageBox->m_dwMessage != 601 && m_pMessageBox->m_dwMessage != 927)
				return 1;
		}
	}

	if (nX != m_nLastMousePosX || nY != m_nLastMousePosY || !g_pEventTranslator->button[0] && !g_pEventTranslator->button[1])
	{
		m_nLastMousePosX = nX;
		m_nLastMousePosY = nY;
		m_dwLastMousePosTime = dwServerTime;
	}

	if (m_pMessageBox->m_bVisible == 1 && 
		m_pMessageBox->m_dwMessage == 99 &&
		g_pObjectManager->m_stMobData.CurrentScore.Hp > 0 &&
		!m_pMyHuman->m_cDie)
	{
		m_pMessageBox->SetVisible(0);
	}

	auto pMobData = &g_pObjectManager->m_stMobData;

	if (m_cResurrect ||
		pMobData->Equip[13].sIndex == 769 ||
		g_pObjectManager->m_stMobData.CurrentScore.Hp > 0 && m_pMyHuman->m_cDie != 1 ||
		dwFlags != 516 ||
		wParam & 8 ||
		nX <= 0 ||
		nY <= 0 ||
		nX >= static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
		nY >= static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
	{
		if ((g_pObjectManager->m_stMobData.CurrentScore.Hp <= 0 || m_pMyHuman->m_cDie == 1)
			&& dwFlags == 513
			&& !m_pMyHuman->m_sFamCount)
		{
			POINT pt{};
			pt.x = static_cast<int>(m_pMyHuman->m_vecPosition.x);
			pt.y = static_cast<int>(m_pMyHuman->m_vecPosition.y);

			if (!m_pMessageBox->IsVisible() && (!m_pMyHuman->IsInTown() || PtInRect(&rectTownInCastle, pt) == 1))
			{
				m_pMessageBox->SetMessage(g_pMessageStringTable[27], 11u, 0);
				m_pMessageBox->SetVisible(1);
				return 1;
			}
		}
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_DEAD)
			return 1;

		if (wParam & 4 && dwFlags == 516)
		{
			if (m_pMouseOverHuman)
			{
				if (m_pMouseOverHuman->m_dwID > 0 && m_pMouseOverHuman->m_dwID < 1000)
				{
					char szStrTemp[128]{};
					sprintf_s(szStrTemp, "/%s ", m_pMouseOverHuman->m_szName);

					m_pEditChat->SetText(szStrTemp);

					m_dwOldAttackTime = dwServerTime;
					return 1;
				}
			}
		}

		auto vec = GroundGetPickPos();

		auto pPanel = m_pInvenPanel;

		if (m_pMyHuman->m_bSliding == 1)
			return 1;

		if (m_pMyHuman->m_cOnlyMove == 1 
			&& (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || 
				m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND02 ||
				m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_LEVELUP))
		{
			m_pMyHuman->m_cOnlyMove = 0;
			m_pMyHuman->SetSpeed(m_bMountDead);
		}

		if (m_pMyHuman->m_cOnlyMove == 1)
			return 1;

		if (m_pMyHuman->m_cSameHeight == 1)
			return 1;

		if (dwFlags == 512)
			MouseMove(nX, nY);

		if (dwFlags == 513)
		{
			if (m_cLastFlagLButtonUp == 1)
				m_dwLastMouseDownTime = dwServerTime;

			m_cLastFlagLButtonUp = 0;
		}

		if (g_pObjectManager->m_stMobData.CurrentScore.Hp > 0)
		{
			int SWidth = g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift;
			int SHeight = g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift;

			if (nX > 0 && nY > 0 && nX < SWidth && nY < SHeight)
			{
				if (dwFlags == 517 && wParam & 8)
				{
					if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
						return 1;

					if (m_pMouseOverHuman && m_pMouseOverHuman->m_dwEdgeColor != 0x8800FF00)
						PGTVisible(dwServerTime);

					return 1;
				}

				if (dwFlags == 516 && !(wParam & 8))
				{
					if (wParam & 4)
						return SkillUse(nX, nY, vec, dwServerTime, 0, 0);
					else
						return SkillUse(nX, nY, vec, dwServerTime, 1, 0);
				}

				if (dwFlags == 514)
				{
					m_cLastFlagLButtonUp = 1;
					m_bMoveing = 0;

					if (m_pMyHuman->m_cHide == 1)
						return 1;

					if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
						return 1;

					return MouseClick_NPC(nX, nY, vec, dwServerTime);
				}

				if (dwFlags == 513 && !m_pMyHuman->m_cHide && (!m_bMoveing || wParam & 4))
				{
					int nRet = MobAttack(wParam, vec, dwServerTime);

					if (!nRet)
						nRet = CheckMerchant(m_pMouseOverHuman);

					float fHeight = static_cast<float>(GroundGetMask(TMVector2{ vec.x, vec.z })) * 0.1f;

					TMVector3 vecTar{ vec.x, fHeight + 0.30000001f, vec.z };

					m_pTarget1->m_vecPosition = vecTar;
					m_pTarget2->m_vecPosition = vecTar;
					m_pTargetBill->m_vecPosition = vecTar;

					m_pTargetBill->m_vecPosition.y -= 0.2f;

					if (nRet)
						return nRet;
				}

				if (dwFlags != 513 || wParam & 4 || g_pCursor->m_pAttachedItem || m_pMyHuman->m_cCantMove)
				{
					if (dwFlags == 513 && pPanel->IsVisible() == 1 && g_pCursor->m_pAttachedItem)
					{
						if (g_pCursor->m_pAttachedItem->m_pItem->sIndex >= 5000 && g_pCursor->m_pAttachedItem->m_pItem->sIndex < 5096)
							return 1;

						DropItem(dwServerTime);
						return 1;
					}
				}
				else if (m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_SEATING && m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_PUNISHING)
				{
					if (static_cast<float>(nX) >= (m_pChatBack->GetPos().x + 10.0f) &&
						abs(static_cast<int>(vec.x - m_pMyHuman->m_vecPosition.x)) < 15 &&
						abs(static_cast<int>(vec.y - m_pMyHuman->m_vecPosition.y)) > 15)
					{
						m_bMoveing = 1;

						float fHeight = static_cast<float>(GroundGetMask(TMVector2{ vec.x, vec.z })) * 0.1f;

						TMVector3 vecTar{ vec.x, fHeight + 0.30000001f, vec.z };

						m_pTarget1->m_vecPosition = vecTar;
						m_pTarget2->m_vecPosition = vecTar;
						m_pTargetBill->m_vecPosition = vecTar;
						m_pTargetBill->m_vecPosition.y -= 0.2f;
						
						MobMove(vec, dwServerTime);
					}
				}
				else
				{
					if (m_pMyHuman->m_SendeMotion != ECHAR_MOTION::ECMOTION_NONE)
						return 1;

					MSG_Motion stMotion{};

					stMotion.Header.ID = g_pObjectManager->m_dwCharID;
					stMotion.Header.Type = MSG_Motion_Opcode;

					if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
						stMotion.Motion = 25;
					else
						stMotion.Motion = 27;

					stMotion.Direction = 0.0f;

					m_pMyHuman->m_SendeMotion = static_cast<ECHAR_MOTION>(stMotion.Motion);

					SendOneMessage((char*)&stMotion, sizeof(stMotion));

					m_dwKeyTime = dwServerTime;
				}
			}
		}
		return 0;
	}

	for (int i = 0; i < 20; ++i)
	{
		char cSkillIndex = g_pObjectManager->m_cShortSkill[i];

		if (cSkillIndex == 99)
		{
			g_pObjectManager->m_cSelectShortSkill = cSkillIndex;
			
			UpdateSkillBelt();

			m_pMessageBox->SetMessage(g_pMessageStringTable[227], 99u, 0);
			m_pMessageBox->SetVisible(1);
			return 1;
		}
	}

	return 1;
}

int TMFieldScene::OnAccel(int nMsg)
{
	if (g_nKeyType != 1)
		return 1;

	switch (nMsg)
	{
	case 40075:
		return OnKeySkill(33, 0);
	case 40076:
		return OnKeySkill(64, 0);
	case 40077:
		return OnKeySkill(35, 0);
	case 40078:
		return OnKeySkill(36, 0);
	case 40079:
		return OnKeySkill(37, 0);
	case 40080:
		return OnKeySkill(94, 0);
	case 40081:
		return OnKeySkill(38, 0);
	case 40082:
		return OnKeySkill(42, 0);
	case 40083:
		return OnKeySkill(40, 0);
	case 40074:
		return OnKeySkill(41, 0);
	case 40048:
		return OnKeyDash(45, 0);
	case 40047:
		return OnKeyPlus(43, 0);
	case 40092:
		SetPK();
		return 1;
	case 40094:
		return OnKeyName(110, 0);
	case 40099:
		return OnKeyAutoTarget(116, 0);
	case 40090:
		return OnKeyHelp(104, 0);
	case 40097:
		return OnKeyRun(114, 0);
	case 40100:
		return OnKeyFeedMount(118, 0);
	case 40098:
		return OnKeyVisibleSkill(115, 0);
	case 40091:
		return OnKeyVisibleInven(105, 0);
	case 40085:
		return OnKeyVisibleInven(105, 0);
	case 40086:
		return OnKeyVisibleCharInfo(99, 0);
	case 40093:
		return OnKeyVisibleMinimap(109, 0);
	case 40095:
		return OnKeyVisibleParty(112, 0);
	}
	if (g_pObjectManager->m_stMobData.CurrentScore.Hp > 0)
	{
		switch (nMsg)
		{
		case 40103:
			return OnKeySkillPage(122, 0);
		case 40102:
			return OnKeyQuestLog(120, 0);
		case 40027:
			return OnKeyReverse(91, 0);
		case 40028:
			return OnKeyAutoRun(93, 0);
		case 40029:
			return OnKeyGuildOnOff(39, 0);
		case 40065:
			return OnKeyShortSkill(49, 0);
		case 40066:
			return OnKeyShortSkill(50, 0);
		case 40067:
			return OnKeyShortSkill(51, 0);
		case 40068:
			return OnKeyShortSkill(52, 0);
		case 40069:
			return OnKeyShortSkill(53, 0);
		case 40070:
			return OnKeyShortSkill(54, 0);
		case 40071:
			return OnKeyShortSkill(55, 0);
		case 40072:
			return OnKeyShortSkill(56, 0);
		case 40073:
			return OnKeyShortSkill(57, 0);
		case 40064:
			return OnKeyShortSkill(48, 0);
		case 40104:
			return OnKeyNumPad(96);
		case 40105:
			return OnKeyNumPad(97);
		case 40106:
			return OnKeyNumPad(98);
		case 40107:
			return OnKeyNumPad(99);
		case 40108:
			return OnKeyNumPad(100);
		case 40109:
			return OnKeyNumPad(101);
		case 40110:
			return OnKeyNumPad(102);
		case 40111:
			return OnKeyNumPad(103);
		case 40112:
			return OnKeyNumPad(104);
		case 40113:
			return OnKeyNumPad(105);
		}
	}

	return 0;
}

void TMFieldScene::MouseMove(int nX, int nY)
{
	if (m_pMouseOverHuman && 
		m_pMouseOverHuman->m_pSkinMesh &&
		nX > 0 &&
		nY > 0 &&
		nX < static_cast<int>(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) &&
		nY < static_cast<int>(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
	{
		if (m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.r >= 0.99900001f &&
			m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.g <= 0.001f &&
			m_pMouseOverHuman->m_pSkinMesh->m_materials.Emissive.b <= 0.001f)
		{
			g_pCursor->m_GCPanel.nTextureIndex = 1;
		}
		else if (g_pCursor->m_GCPanel.nTextureIndex == 1)
		{
			g_pCursor->m_GCPanel.nTextureIndex = 0;
		}
	}
	else if (g_pCursor->m_GCPanel.nTextureIndex == 1)
	{
		g_pCursor->m_GCPanel.nTextureIndex = 0;
	}
}

int TMFieldScene::MouseClick_NPC(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	auto pOver = m_pMouseOverHuman;

	if (m_pAutoTrade->m_bVisible)
		return 1;

	m_bTeleportMsg = 0;

	if (!pOver || pOver->m_bMouseOver != 1)
	{
		if (dwServerTime - m_dwLastMouseDownTime > 1000 &&
			m_dwLastMouseDownTime && !m_pMyHuman->m_cLastMoveStop && m_pMyHuman->m_stScore.Hp > 0)
		{
			MobStop(vec);
		}
		return 1;
	}

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 1 &&
		(int)m_pMyHuman->m_vecPosition.x >> 7 < 11 &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 < 5)
		return 1;

	if (dwServerTime - m_dwNPCClickTime < 1000)
		return 1;

	if (pOver->m_TradeDesc[0])
	{
		MSG_STANDARDPARM stQuest{};

		stQuest.Header.Type = MSG_ReqTradeList_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm = pOver->m_dwID;
		SendOneMessage((char*)&stQuest, sizeof(stQuest));
		m_dwNPCClickTime = dwServerTime;
		return 1;
	}

	if (pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 51 &&
		((int)m_pMyHuman->m_vecPosition.x >> 7 == 13 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 14) &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 == 28)
		return 1;

	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 1)
	{
		if (!m_pShopPanel->IsVisible())
		{
			if (pOver->m_sHeadIndex == 58 && pOver->m_sHelmIndex == 1110)
				m_nIsMP = 1;
			else if (pOver->m_sHeadIndex == 59 && pOver->m_sHelmIndex == 1257)
				m_nIsMP = 2;

			if (pOver->m_sHeadIndex != 59 || pOver->m_sHelmIndex != 1260)
				m_nIsMP = 0;
			else
				m_bEventCouponClick = 1;

			MSG_REQShopList stReqShopList{};

			stReqShopList.Header.Type = MSG_REQShopList_Opcode;
			stReqShopList.Header.ID = m_pMyHuman->m_dwID;
			stReqShopList.TargetID = pOver->m_dwID;

			m_pGridShop->m_dwMerchantID = pOver->m_dwID;

			SendOneMessage((char*)&stReqShopList, sizeof(stReqShopList));

			m_dwNPCClickTime = dwServerTime;
			m_sShopTarget = pOver->m_dwID;
			m_bIsUndoShoplist = 0;
		}
		return 1;
	}

	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 3)
	{
		if (!m_pShopPanel->IsVisible())
		{
			if (pOver->m_sHeadIndex == 58 && pOver->m_sHelmIndex == 1110)
				m_nIsMP = 1;
			else if (pOver->m_sHeadIndex == 59 && pOver->m_sHelmIndex == 1257)
				m_nIsMP = 2;

			if (pOver->m_sHeadIndex != 59 || pOver->m_sHelmIndex != 1260)
				m_nIsMP = 0;
			else
				m_bEventCouponClick = 1;

			MSG_REQShopList stReqShopList{};

			stReqShopList.Header.Type = MSG_REQShopList_Opcode;
			stReqShopList.Header.ID = m_pMyHuman->m_dwID;
			stReqShopList.TargetID = pOver->m_dwID;

			m_pGridShop->m_dwMerchantID = pOver->m_dwID;

			SendOneMessage((char*)&stReqShopList, sizeof(stReqShopList));

			m_dwNPCClickTime = dwServerTime;
			m_sShopTarget = pOver->m_dwID;
			m_bIsUndoShoplist = 0;
		}
		return 1;
	}
	
	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 2 && pOver->m_sHeadIndex != 51)
	{
		if (!m_pCargoPanel->IsVisible())
		{
			SetVisibleCargo(1);
			TMFieldScene::m_dwCargoID = pOver->m_dwID;
			m_dwNPCClickTime = dwServerTime;
		}
		return 1;
	}
	
	if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 3)
	{
		MouseClick_SkillMasterNPC(dwServerTime, pOver);
		return 1;
	}
	
	if (pOver->m_dwID < 1000 ||
		pOver->m_vecTargetPos.x < 2148 ||
		pOver->m_vecTargetPos.x > 2156 ||
		pOver->m_vecTargetPos.y < 2067 ||
		pOver->m_vecTargetPos.y > 2076 ||
		pOver->m_sHeadIndex != 51 ||
		m_pGround->m_vecOffsetIndex.x != 16 ||
		m_pGround->m_vecOffsetIndex.y != 16)
	{
		
		if (pOver->m_dwID >= 1000 &&
			pOver->m_sHeadIndex == 67 &&
			m_pGround->m_vecOffsetIndex.x == 16 &&
			m_pGround->m_vecOffsetIndex.y == 16)
		{
			m_MissionClass.ResultItemListSet();
			SetVisibleMissionPanel(m_MissionClass.m_pMissionPanel->m_bVisible == 0);
			return 1;
		}

		if (MouseClick_MixNPC(pOver))
			return 1;

		if (pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 57)
		{
			MouseClick_PremiumNPC(pOver);
			return 1;
		}

		if (pOver->m_dwID >= 1000 && 
			pOver->m_sHeadIndex == 63 &&
			(pOver->m_stScore.Merchant & 0xF) == 7 && 
			m_pGround->m_vecOffsetIndex.x == 16 &&
			m_pGround->m_vecOffsetIndex.y == 16)
		{
			AirMove_ShowUI(1);
			return 1;
		}
		
		if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 4 || (pOver->m_stScore.Merchant & 0xF) >= 8 && (pOver->m_stScore.Merchant & 0xF) <= 15)
		{
			MouseClick_QuestNPC(dwServerTime, pOver);
			return 1;
		}
		
		if (pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) >= 6 && (pOver->m_stScore.Merchant & 0xF) <= 8)
		{
			MSG_STANDARDPARM stPacket{};

			stPacket.Header.Type = MSG_Challange_Opcode;
			stPacket.Header.ID = m_pMyHuman->m_dwID;
			m_dwTID = pOver->m_dwID;
			stPacket.Parm = m_dwTID;

			SendOneMessage((char*)&stPacket, sizeof(stPacket));

			m_dwNPCClickTime = dwServerTime;
			return 1;
		}
		return 1;
	}

	if (!m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[437], pOver->m_sHeadIndex, 0);
		m_pMessageBox->m_dwArg = pOver->m_dwID;
		m_pMessageBox->SetVisible(1);
	}

	return 1;
}

int TMFieldScene::CheckMerchant(TMHuman* pOver)
{
	if (m_pAutoTrade->m_bVisible)
		return 1;

	if (!pOver || pOver->m_bMouseOver != 1)
		return 0;

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 1 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 11 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 5)
		return 1;

	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && 
		pOver->m_sHeadIndex == 51 && 
		((int)m_pMyHuman->m_vecPosition.x >> 7 == 13 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 14) && (int)m_pMyHuman->m_vecPosition.y >> 7 == 28)
	{
		return 1;
	}

	if (pOver->m_TradeDesc[0])
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 1)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 2)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 3)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 67 && m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 13)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 67 && m_pGround->m_vecOffsetIndex.x == 28 && m_pGround->m_vecOffsetIndex.y == 24)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 54)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 55)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 56)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 68)
		return 1;
	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && pOver->m_sHeadIndex == 57)
		return 1;


	if (((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) == 4) ||
		(pOver->m_stScore.Merchant & 0xF) >= 8 && (pOver->m_stScore.Merchant & 0xF) <= 15)
	{
		if ((pOver->m_stScore.Merchant & 0xF) == 15)
		{
			if (!m_pMyHuman->IsInTown())
			{
				if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua != m_pMyHuman->m_cMantua && 
					m_pMyHuman->m_cMantua != 4 && m_pMyHuman->m_cMantua != 3)
				{
					return 1;
				}

				if (m_pMyHuman->m_pMantua && 
					((int)m_pMyHuman->m_pMantua->m_Look.Skin0 < 2 || 
					((int)m_pMyHuman->m_pMantua->m_Look.Skin0 >= 8 && (int)m_pMyHuman->m_pMantua->m_Look.Skin0 <= 14)))
				{
					if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742
						&& (g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760 || g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763))
					{
						return 1;
					}
				}
			}
		}

		if ((pOver->m_stScore.Merchant & 0xF) == 13)
			return 1;
		if ((pOver->m_stScore.Merchant & 0xF) == 14)
			return 1;

		return 1;
	}

	if ((pOver->m_dwID < 0 || pOver->m_dwID >= 1000) && (pOver->m_stScore.Merchant & 0xF) >= 6 && (pOver->m_stScore.Merchant & 0xF) <= 8)
		return 1;

	return 0;
}

int TMFieldScene::MouseLButtonDown(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	// This function is just that
	return 1;
}

void TMFieldScene::OnESC()
{
	if (g_bActiveWB == 1)
	{
		g_pApp->SwitchWebBrowserState(0);
	}
	else if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
	{
		SetVisibleAutoTrade(0, 0);
	}
	else if (m_pPGTPanel->IsVisible() == 1)
	{
		m_pPGTPanel->SetVisible(0);
	}
	else if (m_pQuestPanel->IsVisible() == 1)
	{
		m_pQuestPanel->SetVisible(0);
		m_pQuestBtn->SetSelected(0);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(51);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	else if (m_pFireWorkPanel && m_pFireWorkPanel->IsVisible() == 1)
	{
		m_pFireWorkPanel->SetVisible(0);
	}
	else if (m_pTotoPanel && m_pTotoPanel->IsVisible() == 1)
	{
		m_pTotoPanel->SetVisible(0);
	}
	else if (m_pInvenPanel->IsVisible() == 1)
	{
		OnControlEvent(65562u, 0);
	}
	else if (m_pSkillPanel->IsVisible() == 1)
	{
		OnControlEvent(65568u, 0);
	}
	else if (m_pCPanel->IsVisible() == 1)
	{
		OnControlEvent(65769u, 0);
	}
	else if (m_pTradePanel->IsVisible() == 1)
	{
		SetVisibleTrade(0);
	}
	else if (m_pPartyPanel->IsVisible() == 1)
	{
		SetVisibleParty();
	}
	else if (m_pShopPanel->IsVisible() == 1)
	{
		SetVisibleShop(0);
	}
	else if (m_pCargoPanel->IsVisible() == 1)
	{
		SetVisibleCargo(0);
	}
	else if (m_pGambleStore->IsVisible() == 1)
	{
		SetVisibleGamble(0, 0);
	}
	else if (m_pInputGoldPanel->IsVisible() == 1)
	{
		m_pInputGoldPanel->SetVisible(0);
	}
	else if (m_pMsgPanel && m_pMsgPanel->IsVisible() == 1)
	{
		m_pMsgPanel->SetVisible(0);
		m_pControlContainer->SetFocusedControl(0);
	}
	else if (m_pHelpPanel && m_pHelpPanel->IsVisible() == 1)
	{
		m_pHelpPanel->SetVisible(0);
		m_pHelpBtn->SetSelected(0);

		if (g_pSoundManager)
		{
			auto pSoundData = g_pSoundManager->GetSoundData(51);

			if (pSoundData)
				pSoundData->Play(0, 0);
		}
	}
	else if (m_pServerPanel && m_pServerPanel->IsVisible() == 1)
	{
		m_pServerPanel->SetVisible(0);
	}
	else if (m_pPotalPanel && m_pPotalPanel->IsVisible() == 1)
	{
		m_pPotalPanel->SetVisible(0);
	}
	else if (m_pMessageBox && m_pMessageBox->IsVisible() == 1)
	{
		m_pMessageBox->SetVisible(0);
	}
	else if (!m_pSystemPanel->IsVisible())
	{
		m_pSystemPanel->SetVisible(1);
	}
	else if (m_pSystemPanel->IsVisible() == 1)
	{
		m_pSystemPanel->SetVisible(0);
	}
}

char TMFieldScene::UseQuickSloat(char key)
{
	SGridControlItem* pItemFind = nullptr;
	SGridControl* pGridSloat = nullptr;
	if (key == 'Q' || key == 'q')
	{
		pItemFind = m_pQuick_Sloat[0]->GetAtItem(0, 0);
		pGridSloat = m_pQuick_Sloat[0];
	}
	else if (key == 'W' || key == 'w')
	{
		pItemFind = m_pQuick_Sloat[1]->GetAtItem(0, 0);
		pGridSloat = m_pQuick_Sloat[1];
	}
	else if (key == 'E' || key == 'e')
	{
		pItemFind = m_pQuick_Sloat[2]->GetAtItem(0, 0);
		pGridSloat = m_pQuick_Sloat[2];
	}
	else if (key == 'R' || key == 'r')
	{
		pItemFind = m_pQuick_Sloat[3]->GetAtItem(0, 0);
		pGridSloat = m_pQuick_Sloat[3];
	}
	else if(key == 'T' || key == 't')
	{
		pItemFind = m_pQuick_Sloat[4]->GetAtItem(0, 0);
		pGridSloat = m_pQuick_Sloat[4];
	}

	if (!pItemFind)
		return 0;

	int page = 0;

	SGridControlItem* pItem = nullptr;
	SGridControl* pGridInv = nullptr;
	bool bFind = false;
	for (int i = 0; i < 4; ++i)
	{
		pGridInv = m_pGridInvList[i];
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				pItem = pGridInv->GetItem(k, j);
				if (pItem && pItem->m_pItem->sIndex == pItemFind->m_pItem->sIndex)
				{
					bFind = true;
					page = 15 * i;
					break;
				}
			}
			if (bFind == true)
				break;
		}
		if (bFind == true)
			break;
	}
	if (bFind != true || !pItem)
		return 0;

	int nType = BASE_GetItemAbility(pItem->m_pItem, 38);
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwUseItemTime && dwServerTime - m_dwUseItemTime < 200)
		return 0;

	int SourType = pGridInv->CheckType(pItem->m_pGridControl->m_eItemType, pItem->m_pGridControl->m_eGridType);
	int SourPos = pGridInv->CheckPos(pItem->m_pGridControl->m_eItemType);
	if (SourPos == -1)
		SourPos = pItem->m_nCellIndexX + 5 * pItem->m_nCellIndexY;
	SourPos = page + SourPos;

	MSG_UseItem stUseItem{};
	stUseItem.Header.ID = g_pObjectManager->m_dwCharID;
	stUseItem.Header.Type = MSG_UseItem_Opcode;
	stUseItem.SourType = SourType;
	stUseItem.SourPos = SourPos;
	stUseItem.ItemID = 0;
	stUseItem.GridX = (int)m_pMyHuman->m_vecPosition.x;
	stUseItem.GridY = (int)m_pMyHuman->m_vecPosition.y;

	if (nType == 15)
	{
		stUseItem.DestType = 0;
		stUseItem.DestPos = 14;
	}

	SendOneMessage((char*)&stUseItem, sizeof(stUseItem));

	m_dwUseItemTime = dwServerTime;
	int nAmount = BASE_GetItemAmount(pItem->m_pItem);

	if (nAmount > 1)
	{
		BASE_SetItemAmount(pItem->m_pItem, nAmount - 1);
		sprintf(pItem->m_GCText.strString, "%2d", nAmount - 1);
		pItem->m_GCText.pFont->SetText(pItem->m_GCText.strString, pItem->m_GCText.dwColor, 0);
		sprintf(pItemFind->m_GCText.strString, "%2d", nAmount - 1);
		pItemFind->m_GCText.pFont->SetText(pItemFind->m_GCText.strString, pItemFind->m_GCText.dwColor, 0);
	}
	else
	{
		auto pPickedItem = m_pGridInvList[stUseItem.SourPos / 15]->PickupItem(stUseItem.SourPos % 15 % 5, stUseItem.SourPos % 15 / 5);
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
			g_pCursor->m_pAttachedItem = 0;
		
		SAFE_DELETE(pPickedItem);

		g_pCursor->DetachItem();
		bool bFind = false;
		for (int i = 0; i < 4; ++i)
		{
			auto pGridInv = m_pGridInvList[i];
			for (int l = 0; l < 3; ++l)
			{
				for (int m = 0; m < 5; ++m)
				{
					auto pItem = pGridInv->GetItem(m, l);
					if (pItem && pItem->m_pItem->sIndex == pItemFind->m_pItem->sIndex)
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
		if (!bFind)
		{
			auto pReturnItem = pGridSloat->PickupItem(0, 0);
			SAFE_DELETE(pReturnItem);
		}
	}
	if (nAmount <= 1)
		memset(&g_pObjectManager->m_stMobData.Carry[SourPos], 0, sizeof(STRUCT_ITEM));

	GetSoundAndPlay(41, 0, 0);
	UpdateScoreUI(16);

	return 1;
}

int TMFieldScene::OnKeyDebug(char iCharCode, int lParam)
{
	// Just that
	return 0;
}

int TMFieldScene::OnKeySkill(char iCharCode, int lParam)
{
	if (!m_pSkillPanel->IsVisible())
		return 0;

	int nBase = 0;
	float fWRatio = RenderDevice::m_fWidthRatio;
	float fHRatio = RenderDevice::m_fHeightRatio;
	if (m_pGridSkillBelt3->IsVisible() == 1)
		nBase = 10;

	int CodeIndex = -1;
	switch (iCharCode)
	{
	case '!':
		CodeIndex = 0;
		break;
	case '@':
		CodeIndex = 1;
		break;
	case '#':
		CodeIndex = 2;
		break;
	case '$':
		CodeIndex = 3;
		break;
	case '%':
		CodeIndex = 4;
		break;
	case '^':
		CodeIndex = 5;
		break;
	case '&':
		CodeIndex = 6;
		break;
	case '*':
		CodeIndex = 7;
		break;
	case '(':
		CodeIndex = 8;
		break;
	case ')':
		CodeIndex = 9;
		break;
	}
	if (CodeIndex == -1)
		return 0;

	for (int i = 0; i < 24; ++i)
	{
		auto pGrid = m_pSkillSecGrid[i];
		if (PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, pGrid->m_GCPanel.nPosX, pGrid->m_GCPanel.nPosY, pGrid->m_GCPanel.nWidth * fWRatio, pGrid->m_GCPanel.nHeight * fHRatio) == 1)
		{
			auto pGridItem = pGrid->GetItem(0, 0);
			SetShortSkill(CodeIndex + nBase, pGridItem);
			break;
		}
	}
	for (int i = 0; i < 12; ++i)
	{
		auto pGrid = m_pSkillSecGrid2[i];
		if (PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, pGrid->m_GCPanel.nPosX, pGrid->m_GCPanel.nPosY, pGrid->m_GCPanel.nWidth * fWRatio, pGrid->m_GCPanel.nHeight * fHRatio) == 1)
		{
			auto pGridItem = pGrid->GetItem(0, 0);
			SetShortSkill(CodeIndex + nBase, pGridItem);
			break;
		}
	}
	if (PointInRect((int)g_pCursor->m_nPosX, (int)g_pCursor->m_nPosY, m_pGridSkillBelt->m_GCPanel.nPosX, m_pGridSkillBelt->m_GCPanel.nPosY, 
		m_pGridSkillBelt->m_GCPanel.nWidth * fWRatio, m_pGridSkillBelt->m_GCPanel.nHeight * fHRatio) == 1)
	{
		auto pItem = m_pGridSkillBelt->GetAtItem((int)((8 * (int)(g_pCursor->m_nPosX - m_pGridSkillBelt->m_GCPanel.nPosX)) / (m_pGridSkillBelt->m_GCPanel.nWidth * fWRatio)),
			(int)((int)(g_pCursor->m_nPosY - m_pGridSkillBelt->m_GCPanel.nPosY)	/ (m_pGridSkillBelt->m_GCPanel.nHeight * fHRatio)));

		if (pItem)
			SetShortSkill(nBase, pItem);
	}

	return 1;
}

int TMFieldScene::OnKeyDash(char iCharCode, int lParam)
{
	if (iCharCode != '-' && iCharCode != '_')
		return 0;

	if (m_cAutoAttack == 1)
	{
		int nCount = m_nAutoSkillNum + 1;

		if (nCount > 10)
			nCount = 1;

		SetAutoSkillNum(nCount);
	}
	return 1;
}

int TMFieldScene::OnKeyPlus(char iCharCode, int lParam)
{
	if (iCharCode != '=' && iCharCode != '+')
		return 0;

	++m_nChatListSize;
	m_nChatListSize %= 4;
	if (!m_pChatList)
		return 0;

	auto pChatList = m_pChatList;

	pChatList->SetSize(pChatList->m_nWidth, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	pChatList->m_pScrollBar->SetSize(pChatList->m_pScrollBar->m_nWidth, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	pChatList->m_pScrollBar->m_pBackground1->SetSize(pChatList->m_pScrollBar->m_nWidth, (float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);

	m_pChatBack->SetSize(BASE_ScreenResize(10.0) + pChatList->m_nWidth, pChatList->m_nHeight);

	pChatList->m_pScrollBar->SetMaxValue(1000);
	auto pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672u);

	if (pChatPanel)
	{
		m_pChatList->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - pChatList->m_nHeight) - 4.0f);
		m_pChatBack->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - pChatList->m_nHeight) - 4.0f);
	}

	if (m_nChatListSize == 3)
		m_pChatBack->SetVisible(0);
	else
		m_pChatBack->SetVisible(1);

	pChatList->m_nVisibleCount = 10 * m_nChatListSize + 5;
	pChatList->m_pScrollBar->Down();

	if (m_nChatListSize == 3)
		pChatList->SetVisible(0);
	else if (!pChatList->m_bVisible)
		pChatList->SetVisible(1);

	GetSoundAndPlay(51, 0, 0);

	return 1;
}

int TMFieldScene::OnKeyPK(char iCharCode, int lParam)
{
	if (iCharCode != 'k' && iCharCode != 'K')
		return 0;

	SetPK();
	return 1;
}

int TMFieldScene::OnKeyName(char iCharCode, int lParam)
{
	if (g_bEvent == 1)
		return 0;

	if (iCharCode != 'n' && iCharCode != 'N')
		return 0;

	SetVisibleNameLabel();
	return 1;
}

int TMFieldScene::OnKeyAutoTarget(char iCharCode, int lParam)
{
	if (iCharCode != 'Y' && iCharCode != 'y')
		return 0;

	SetAutoTarget();
	return 1;
}

int TMFieldScene::OnKeyAuto(char iCharCode, int lParam)
{
	// Just that
	return 0;
}

int TMFieldScene::OnKeyHelp(char iCharCode, int lParam)
{
	if (iCharCode != 'h' && iCharCode != 'H')
		return 0;

	if (m_pHelpPanel)
	{
		int bVisible = m_pHelpPanel->IsVisible();
		m_pHelpPanel->SetVisible(bVisible == 0);
		m_pHelpBtn->SetSelected(bVisible == 0);
		GetSoundAndPlay(51, 0, 0);
	}

	return 1;
}

int TMFieldScene::OnKeyRun(char iCharCode, int lParam)
{
	if (iCharCode != 'r' && iCharCode != 'R')
		return 0;

	SetRunMode();
	return 1;
}

int TMFieldScene::OnKeyFeedMount(char iCharCode, int lParam)
{
	if (iCharCode != 'v' && iCharCode != 'V')
		return 0;

	return FeedMount();
}

int TMFieldScene::OnKeyHPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'q' && iCharCode != 'Q')
		return 0;

	return UseHPotion();
}

int TMFieldScene::OnKeyMPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'w' && iCharCode != 'W')
		return 0;

	return UseMPotion();
}

int TMFieldScene::OnKeyPPotion(char iCharCode, int lParam)
{
	if (iCharCode != 'e' && iCharCode != 'E')
		return 0;

	TMFieldScene::UsePPotion();
	return 1;
}

int TMFieldScene::OnKeySkillPage(char iCharCode, int lParam)
{
	if (iCharCode != 'z' && iCharCode != 'Z')
		return 0;

	if (m_pGridSkillBelt2->m_bVisible)
		OnControlEvent(65647, 0);
	else
		OnControlEvent(65646u, 0);

	return 1;
}

int TMFieldScene::OnKeyQuestLog(char iCharCode, int lParam)
{
	if (iCharCode != 'x' && iCharCode != 'X')
		return 0;

	OnControlEvent(65793, 0);
	return 1;
}

int TMFieldScene::OnKeyReverse(char iCharCode, int lParam)
{
	// Just that
	return 0;
}

int TMFieldScene::OnKeyAutoRun(char iCharCode, int lParam)
{
	if (m_pMyHuman->m_cCantMove)
		return 0;

	if (iCharCode != ']' && iCharCode != '}')
		return 0;

	if (m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 31
		|| m_pGround->m_vecOffsetIndex.x == 14 && m_pGround->m_vecOffsetIndex.y == 30
		|| m_pGround->m_vecOffsetIndex.x == 15 && m_pGround->m_vecOffsetIndex.y == 31
		|| m_pGround->m_vecOffsetIndex.x == 9 && m_pGround->m_vecOffsetIndex.y == 28
		|| m_pGround->m_vecOffsetIndex.x == 8 && m_pGround->m_vecOffsetIndex.y == 27
		|| m_pGround->m_vecOffsetIndex.x == 10 && m_pGround->m_vecOffsetIndex.y == 27)
	{
		return 1;
	}

	m_bAutoRun = m_bAutoRun == 0;
	if (m_pAutoRunBtn)
		m_pAutoRunBtn->SetSelected(m_bAutoRun);

	return 1;
}

int TMFieldScene::OnKeyGuildOnOff(char iCharCode, int lParam)
{
	if (iCharCode != '\'' && iCharCode != '"')
		return 0;

	OnControlEvent(299, 0);
	return 1;
}

int TMFieldScene::OnKeyShortSkill(char iCharCode, int lParam)
{
	if ((iCharCode < '0' || iCharCode > '9') && iCharCode != '!' && iCharCode != '@' && 
		iCharCode != '#' && iCharCode != '$' && iCharCode != '%' && iCharCode != '¨' && 
		iCharCode != '&' && iCharCode != '*' && iCharCode != '(' && iCharCode != ')')
	{
		return 0;
	}

	if (m_bNumPad == 1)
	{
		m_bNumPad = 0;
		return 1;
	}

	if (iCharCode >= '0' && iCharCode <= '9')
	{
		g_pObjectManager->m_cSelectShortSkill = iCharCode - '1';
		if (iCharCode == 48)
			g_pObjectManager->m_cSelectShortSkill = 9;
	}
	else
	{
		switch (iCharCode)
		{
		case '!':
			g_pObjectManager->m_cSelectShortSkill = 0;
			break;
		case '@':
			g_pObjectManager->m_cSelectShortSkill = 1;
			break;
		case '#':
			g_pObjectManager->m_cSelectShortSkill = 2;
			break;
		case '$':
			g_pObjectManager->m_cSelectShortSkill = 3;
			break;
		case '%':
			g_pObjectManager->m_cSelectShortSkill = 4;
			break;
		case '¨':
			g_pObjectManager->m_cSelectShortSkill = 5;
			break;
		case '&':
			g_pObjectManager->m_cSelectShortSkill = 6;
			break;
		case '*':
			g_pObjectManager->m_cSelectShortSkill = 7;
			break;
		case '(':
			g_pObjectManager->m_cSelectShortSkill = 8;
			break;
		case ')':
			g_pObjectManager->m_cSelectShortSkill = 9;
			break;
		}
	}
	if (m_pGridSkillBelt3->IsVisible())
		g_pObjectManager->m_cSelectShortSkill += 10;

	if (g_pObjectManager->m_cSelectShortSkill < 10)
	{
		auto pBeltGrid2 = m_pGridSkillBelt2;
		for (int i = 0; i < 10; ++i)
		{
			auto ipCtrlItem2 = pBeltGrid2->GetItem(i, 0);
			if (ipCtrlItem2)
			{
				if (i == g_pObjectManager->m_cSelectShortSkill)
					ipCtrlItem2->m_GCObj.nTextureSetIndex = 200;
				else
					ipCtrlItem2->m_GCObj.nTextureSetIndex = 199;
			}
		}
	}
	else
	{
		auto pBeltGrid3 = m_pGridSkillBelt3;
		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem3 = pBeltGrid3->GetItem(j, 0);
			if (j + 10 == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem3)
				ipCtrlItem3->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem3)
				ipCtrlItem3->m_GCObj.nTextureSetIndex = 199;
		}
	}

	int cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if (cSkillIndex >= 105)
		cSkillIndex += 95;
	if (cSkillIndex >= 0 && cSkillIndex < 248)
	{
		SetMyHumanMagic();

		int nIdx = g_pObjectManager->m_stMobData.Equip[6].sIndex;
		auto pMobData = &g_pObjectManager->m_stMobData;
		int nWeather = g_nWeather;
		if (g_nWeather == 3)
			nWeather = 2;

		if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
			(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			nWeather = 2;
		}

		int nSkillDam = BASE_GetSkillDamage(cSkillIndex, pMobData, nWeather, GetWeaponDamage(), 
			g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);
		auto pSkillDam = (SText*)m_pControlContainer->FindControl(65735);

		char szStr[128]{};
		if (nSkillDam < 0)
		{
			sprintf(szStr, "%d", -nSkillDam);
			pSkillDam->SetText(szStr, 0);
			pSkillDam->SetTextColor(0xFFAAFFAA);
		}
		else
		{
			sprintf(szStr, "%d", nSkillDam);
			pSkillDam->SetText(szStr, 0);
			pSkillDam->SetTextColor(0xFFBBBBFF);
		}
	}
	else
	{
		auto pText = static_cast<SText*>(m_pControlContainer->FindControl(65735));
		
		char szStr[128]{};
		sprintf(szStr, "%d", 0);
		pText->SetText(szStr, 0);
	}

	return 1;
}

int TMFieldScene::OnKeyVisibleSkill(char iCharCode, int lParam)
{
	if (iCharCode != 's' && iCharCode != 'S')
		return 0;

	if (m_pAutoTrade->IsVisible())
		return 1;

	if (m_pShopPanel->IsVisible() != 1)
		SetVisibleSkill();

	return 1;
}

int TMFieldScene::OnKeyCamView(char iCharCode, int lParam)
{
	if (iCharCode != 9)
		return 0;

	SetCameraView();
	return 1;
}

int TMFieldScene::OnKeyVisibleInven(char iCharCode, int lParam)
{
	if (iCharCode != 'i' && iCharCode != 'I' && iCharCode != 'g' && iCharCode != 'G')
		return 0;

	if (m_pAutoTrade->IsVisible())
		return 1;

	SetVisibleInventory();

	auto pPanel = m_pInvenPanel;
	auto pBtnEquip = static_cast<SButton*>(m_pControlContainer->FindControl(65791u));

	if (pBtnEquip)
		pBtnEquip->SetSelected(pPanel->m_bVisible);
		
	return 1;
}

int TMFieldScene::OnKeyVisibleCharInfo(char iCharCode, int lParam)
{
	if (iCharCode != 'c' && iCharCode != 'C')
		return 0;
	if (m_pAutoTrade->IsVisible())
		return 1;
	if (m_pShopPanel->IsVisible())
		return 1;
	if (m_pTradePanel->IsVisible())
		return 1;

	SetVisibleCharInfo();

	auto pPanel = m_pCPanel;
	auto pBtnChar = static_cast<SButton*>(m_pControlContainer->FindControl(65790));
	if (pBtnChar)
		pBtnChar->SetSelected(pPanel->m_bVisible);

	return 1;
}

int TMFieldScene::OnKeyVisibleMinimap(char iCharCode, int lParam)
{
	if (iCharCode != 'm' && iCharCode != 'M')
		return 0;

	SetVisibleMiniMap();
	return 1;
}

int TMFieldScene::OnKeyVisibleParty(char iCharCode, int lParam)
{
	if (iCharCode != 'p' && iCharCode != 'P')
		return 0;

	SetVisibleParty();
	return 1;
}

int TMFieldScene::OnKeyReturn(char iCharCode, int lParam)
{
	if (!m_pEditChatPanel)
		return 0;
	if (!m_pEditChat)
		return 0;
	if (iCharCode != 13)
		return 0;

	auto pEdit = m_pEditChat;
	if (pEdit->IsFocused())
	{
		m_pEditChatPanel->SetVisible(0);
		m_pChatPanel->SetVisible(1);
		m_pControlContainer->SetFocusedControl(0);
		return 1;
	}

	m_pEditChatPanel->SetVisible(1);
	m_pChatPanel->SetVisible(0);
	m_pControlContainer->SetFocusedControl(pEdit);
	auto Button = (SButton*)m_pControlContainer->FindControl(90130u);
	auto Button1 = (SButton*)m_pControlContainer->FindControl(90114u);
	auto Button2 = (SButton*)m_pControlContainer->FindControl(90129u);
	auto Button3 = (SButton*)m_pControlContainer->FindControl(90131u);
	auto Button4 = (SButton*)m_pControlContainer->FindControl(90132u);
	auto Button5 = (SButton*)m_pControlContainer->FindControl(90133u);
	auto Button6 = (SButton*)m_pControlContainer->FindControl(90134u);
	auto Button7 = (SButton*)m_pControlContainer->FindControl(90135u);
	auto Button8 = (SButton*)m_pControlContainer->FindControl(90136u);

	if (!strcmp(Button1->m_GCPanel.strString, Button2->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button3->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"=");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button4->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"-");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button5->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"--");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button6->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"@@");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button7->m_GCPanel.strString))
	{
		pEdit->SetText((char*)"@");
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button8->m_GCPanel.strString))
	{
		char temp[32]{};
		sprintf(temp, "/%s ", g_pMessageStringTable[389]);

		pEdit->SetText(temp);
	}
	else if (!strcmp(Button1->m_GCPanel.strString, Button->m_GCPanel.strString))
	{
		char temp[32]{};
		sprintf(temp, "/%s ", m_cWhisperName);
		pEdit->SetText(temp);
	}

	return 1;
}

int TMFieldScene::OnKeyNumPad(unsigned int iKeyCode)
{
	if (iKeyCode < VK_NUMPAD0 || iKeyCode > VK_NUMPAD9)
		return 0;

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (m_pControlContainer->m_pFocusControl &&
		m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT && !g_nKeyType)
	{
		return 1;
	}

	if (m_pMyHuman->m_cMount)
		return 1;

	if (m_pMyHuman->m_cDie == 1)
		return 1;

	if (dwServerTime < m_dwKeyTime + 500)
		return 1;

	if (m_pMyHuman->m_SendeMotion != ECHAR_MOTION::ECMOTION_NONE)
		return 1;

	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEAT ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISH ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND03 ||
		m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNEND)
	{
		return 1;
	}

	MSG_Motion stMotion{};
	
	stMotion.Header.ID = g_pObjectManager->m_dwCharID;
	stMotion.Header.Type = MSG_Motion_Opcode;
	stMotion.Motion = iKeyCode - 81;

	if (iKeyCode == VK_NUMPAD9)
	{
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
		{
			stMotion.Motion = 25;
		}
		else
		{
			if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISHING)
				return 1;

			stMotion.Motion = 13;
		}
	}
	else if (iKeyCode == VK_NUMPAD0)
	{
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_PUNISHING)
		{
			stMotion.Motion = 27;
		}
		else
		{
			if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_SEATING)
				return 1;

			stMotion.Motion = 15;
		}
	}

	m_pMyHuman->m_SendeMotion = static_cast<ECHAR_MOTION>(stMotion.Motion);

	stMotion.Direction = 0.0f;

	SendOneMessage((char*)&stMotion, sizeof(stMotion));

	m_dwKeyTime = dwServerTime;
	m_bNumPad = 1;
	return 1;
}

int TMFieldScene::OnKeyTotoTab(char iCharCode, int lParam)
{
	return 0;
}

int TMFieldScene::OnKeyTotoEnter(char iCharCode, int lParam)
{
	return 0;
}

int TMFieldScene::MouseClick_MixNPC(TMHuman* pOver)
{
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 67 && 
		m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 13)
	{
		m_ItemMixClass.ResultItemListSet(67, 13, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}

	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 67 && 
		m_pGround->m_vecOffsetIndex.x == 28 && m_pGround->m_vecOffsetIndex.y == 24 && pOver->m_dwID != 1033)
	{
		m_ItemMixClass.ResultItemListSet(67, 28, 24);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 54 && 
		m_pGround->m_vecOffsetIndex.x == 19 && m_pGround->m_vecOffsetIndex.y == 13)
	{
		m_ItemMixClass.ResultItemListSet(54, 19, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 54 && 
		m_pGround->m_vecOffsetIndex.x == 25 && m_pGround->m_vecOffsetIndex.y == 13)
	{
		g_nCombineMode = 1;
		if (m_pItemMixPanel)
			SetVisibleMixItem(m_pItemMixPanel->m_bVisible == 0);

		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 55)
	{
		m_ItemMixClass.ResultItemListSet(55, 19, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 56)
	{
		m_ItemMixClass.ResultItemListSet(56, 19, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 66 && 
		m_pGround->m_vecOffsetIndex.x == 25 && m_pGround->m_vecOffsetIndex.y == 13)
	{
		m_ItemMixClass.ResultItemListSet(66, 25, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 68 && 
		m_pGround->m_vecOffsetIndex.x == 19 && m_pGround->m_vecOffsetIndex.y == 15)
	{
		g_nCombineMode = 1;
		m_ItemMixClass.ResultItemListSet(68, 19, 15);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 68)
	{		
		SetVisibleMixItemTiini(m_pItemMixPanel4->m_bVisible == 0);
		return 1;
	}
	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 67 && 
		(pOver->m_stScore.Merchant & 0xF) == 8 && 
		m_pGround->m_vecOffsetIndex.x == 25 && m_pGround->m_vecOffsetIndex.y == 13)
	{
		m_ItemMixClass.ResultItemListSet(67, 25, 13);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	}

	return 0;
}

void TMFieldScene::MouseClick_PremiumNPC(TMHuman* pOver)
{
}

int TMFieldScene::MouseClick_SkillMasterNPC(unsigned int dwServerTime, TMHuman* pOver)
{
	if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742 && !m_pMessageBox->IsVisible())
	{
		m_pMessageBox->SetMessage(g_pMessageStringTable[152], 1742, 0);
		m_pMessageBox->m_dwArg = pOver->m_dwID;
		m_pMessageBox->SetVisible(1);

		MSG_STANDARDPARM2 stQuest{};

		stQuest.Header.Type = MSG_Quest_Opcode;
		stQuest.Header.ID = m_pMyHuman->m_dwID;
		stQuest.Parm1 = pOver->m_dwID;
		stQuest.Parm2 = 0;
		SendOneMessage((char*)&stQuest, sizeof(stQuest));

		m_dwNPCClickTime = dwServerTime;
		return 1;
	}

	if (!m_pShopPanel->IsVisible())
	{
		MSG_REQShopList stReqShopList{};

		stReqShopList.Header.Type = MSG_REQShopList_Opcode;
		stReqShopList.Header.ID = m_pMyHuman->m_dwID;
		stReqShopList.TargetID = pOver->m_dwID;

		m_pGridSkillMaster->m_dwMerchantID = pOver->m_dwID;

		SendOneMessage((char*)&stReqShopList, sizeof(stReqShopList));
		m_dwNPCClickTime = dwServerTime;
	}
	return 1;
}

int TMFieldScene::MouseClick_QuestNPC(unsigned int dwServerTime, TMHuman* pOver)
{
	if ((pOver->m_stScore.Merchant & 0xF) == 15 && !m_pMyHuman->IsInTown())
	{
		if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua != m_pMyHuman->m_cMantua && m_pMyHuman->m_cMantua != 3)
			return 1;

		if (m_pMyHuman->m_pMantua && 
			((int)m_pMyHuman->m_pMantua->m_Look.Skin0 < 2 || (int)m_pMyHuman->m_pMantua->m_Look.Skin0 >= 8	&& (int)m_pMyHuman->m_pMantua->m_Look.Skin0 <= 14))
		{
			if (g_pObjectManager->m_stMobData.Equip[10].sIndex == 1742 && (g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760 || 
				g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763))
			{
				m_pMessagePanel->SetMessage(g_pMessageStringTable[241], 4000);
				m_pMessagePanel->SetVisible(1, 1);
				return 1;
			}
			if (g_pObjectManager->m_stMobData.Equip[10].sIndex != 1742 || g_pObjectManager->m_stMobData.Equip[11].sIndex < 1760	|| 
				g_pObjectManager->m_stMobData.Equip[11].sIndex > 1763)
			{
				return 1;
			}
		}
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 13)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[131], 13, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}

		return 1;
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 14)
	{
		char cLifeStone = 0;
		char cSapha = 0;
		for (int i = 0; i < 63; ++i)
		{
			if (g_pObjectManager->m_stMobData.Carry[i].sIndex == 1740 && g_pObjectManager->m_stMobData.Carry[i + 1].sIndex == 1741)
				cLifeStone = 1;
			if (g_pObjectManager->m_stMobData.Carry[i].sIndex == 697)
				++cSapha;
			if (cLifeStone == 1 && cSapha >= 20 && m_pMyHuman->m_stScore.Level >= 299)
			{
				if (!m_pMessageBox->IsVisible())
				{
					m_pMessageBox->SetMessage(g_pMessageStringTable[233], 233, 0);
					m_pMessageBox->m_dwArg = pOver->m_dwID;
					m_pMessageBox->SetVisible(1);
				}
				return 1;
			}
		}
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 15 || (pOver->m_stScore.Merchant & 0xF) == 10)
	{
		if (!m_pMessageBox->IsVisible())
		{
			if (pOver->m_sHeadIndex == 51 && _locationCheck(pOver->m_vecPosition, 16, 16))
				m_pMessageBox->SetMessage(g_pMessageStringTable[404], pOver->m_stScore.Merchant & 0xF, 0);
			else
				m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_stScore.Merchant & 0xF, 0);

			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}

		return 1;
	}
	if ((pOver->m_stScore.Merchant & 0xF) == 4 && pOver->m_sHeadIndex == 271)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[348], 271, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}

	if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && (pOver->m_stScore.Merchant & 0xF) == 9 && pOver->m_sHeadIndex == 51)
	{
		m_pInputGoldPanel->SetVisible(1);
		auto pText = (SText*)m_pControlContainer->FindControl(65888);
		if (pText)
		{
			m_nCoinMsgType = 7;
			pText->SetText(g_pMessageStringTable[136], 0);
			auto pEdit = (SEditableText*)g_pCurrentScene->m_pControlContainer->FindControl(65889);
			m_pControlContainer->SetFocusedControl((SControl*)pEdit);
		}
	}
	else if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 58 && (pOver->m_stScore.Merchant & 0xF) == 11)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_sHeadIndex, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}
	else if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && pOver->m_sHeadIndex == 58 && pOver->m_stScore.Merchant == 76)
	{
		if (!m_pMessageBox->IsVisible())
		{
			m_pMessageBox->SetMessage(g_pMessageStringTable[152], pOver->m_sHeadIndex, 0);
			m_pMessageBox->m_dwArg = pOver->m_dwID;
			m_pMessageBox->SetVisible(1);
		}
		return 1;
	}

	MSG_STANDARDPARM2 stQuest{};
	stQuest.Header.Type = MSG_Quest_Opcode;
	stQuest.Header.ID = m_pMyHuman->m_dwID;
	stQuest.Parm1 = pOver->m_dwID;
	stQuest.Parm2 = 0;
	SendOneMessage((char*)&stQuest, sizeof(stQuest));
	m_dwNPCClickTime = dwServerTime;

	return 1;
}
