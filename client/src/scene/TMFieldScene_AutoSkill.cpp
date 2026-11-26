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

/// @file TMFieldScene_AutoSkill.cpp
/// @brief Uso automático de skills (`AutoSkillUse`), usado pelo modo de caça automática.
///
/// Extraído de `TMFieldScene_Combat.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

/// Executa habilidade automatica em modo de auto-combate/macro.
int TMFieldScene::AutoSkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)
{
	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_pPotalPanel->m_bVisible == 1)
		return 0;
	if (g_pEventTranslator->m_bAlt == 1)
		return 0;
	if (pTarget->IsMerchant())
		return 0;
	if (pTarget->m_cSummons == 1)
		return 0;
	if (pTarget->m_cDie == 1)
		return 0;
	if (m_pMyHuman->IsInTown() == 1)
		return 0;
	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK && m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
		return 0;
	if (m_pMyHuman->m_cCantAttk)
		return 0;
	if (pTarget->m_nClass == 44 && pTarget->m_sHeadIndex == 219)
		return 0;
	if (m_pMyHuman->m_bSkillBlack == 1)
		return 0;

	dwServerTime = g_pApp->m_pTimerManager->GetServerTime();
	m_pTargetItem = 0;
	
	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	auto pOver = pTarget;
	if (!pTarget)
		return 0;
	if (pOver->m_cShadow == 1 && pOver->m_nClass == 66 && !m_pMyHuman->m_JewelGlasses)
		return 0;

	char cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if ((unsigned char)cSkillIndex > 104)
		cSkillIndex += 95;
	if (cSkillIndex == 86)
	{
		if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000)
			return 0;
	}
	if (cSkillIndex == 6 && pOver->m_nClass == 66)
		return 0;

	if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000 == 1 && cSkillIndex != 27 && cSkillIndex != 29 && cSkillIndex != 44)
		return 0;
	if (m_pMyHuman->m_sCostume >= 4150 && m_pMyHuman->m_sCostume < 4200 && (cSkillIndex == 64 || cSkillIndex == 66 || cSkillIndex == 68 || cSkillIndex == 70 || cSkillIndex == 71))
		return 0;
	if (!IsValidClassSkill((unsigned char)cSkillIndex))
		return 0;

	int Delay = g_pSpell[(unsigned char)cSkillIndex].Delay;
	if (m_nMySanc >= 9 && Delay >= 2)
		--Delay;
	if (m_pMyHuman->m_DilpunchJewel == 1)
		--Delay;
	if (Delay < 1)
		Delay = 1;

	if (dwServerTime < m_dwOldAttackTime + 1000)
		return 0;
	if (dwServerTime < m_dwSkillLastTime[(unsigned char)cSkillIndex] + 1000 * Delay)
		return 0;

	int TarType = g_pSpell[(unsigned __int8)cSkillIndex].TargetType;
	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 && 
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return 0;

	if (cSkillIndex == 79 && BASE_GetItemAbility(&pMobData->Equip[6], 21) != 101)
		return 0;

	{
		int nReturnValue = 0;
		if (AutoSkillUse_CastSelfOrArea(vec, dwServerTime, nSpecForce, pOver, cSkillIndex, TarType, nReturnValue))
			return nReturnValue;
	}

	if (!pOver || pOver->m_bMouseOver != 1 && !pTarget)
	{
		m_pMouseOverHuman = 0;
		GetItemFromGround(dwServerTime);
		return 0;
	}

	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	int Special = m_pMyHuman->m_stScore.Level;
	if ((unsigned char)cSkillIndex < 96)
		Special = g_pObjectManager->m_stMobData.CurrentScore.Special[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

	if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.SaveMana, Special) > g_pObjectManager->m_stMobData.CurrentScore.Mp)
	{
		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1u,
			0);

		m_pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		return 1;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if ((m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1))
			return 0;

		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1))
		{
			if (!pOver->IsInPKZone())
				return 0;

			if (!m_pMyHuman->IsInPKZone())
				return 0;
		}
	}

	if (pOver->IsInPKZone() == 1
		&& !m_pMyHuman->IsInPKZone()
		&& !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		return 0;
	}
	if (!pOver->IsInPKZone()
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (pOver->m_cSummons == 1 || pOver->m_bParty == 1))
	{
		return 0;
	}

	{
		int nReturnValue = 0;
		if (AutoSkillUse_PKCheck(pOver, cSkillIndex, nReturnValue))
			return nReturnValue;
	}

	if (!g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (pOver->m_dwID <= 0 || pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons)
			return 0;
	}

	if (pOver
		&& pOver->m_usGuild
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (m_pMyHuman->m_usGuild == pOver->m_usGuild
			|| g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
	{
		return 0;
	}

	if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons && pOver->IsMerchant())
		return 0;
	if (pOver->m_TradeDesc[0])
		return 0;
	if ((pOver->m_stScore.Reserved & 0xF) == 15 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if (!m_pMyHuman->m_cMantua)
			return 0;
		if (pOver->m_cMantua > 0
			&& m_pMyHuman->m_cMantua > 0
			&& pOver->m_cMantua == m_pMyHuman->m_cMantua)
		{
			return 0;
		}
	}


	int nTX = (int)pOver->m_vecPosition.x;
	int nTY = (int)pOver->m_vecPosition.y;
	int nSX = (int)m_pMyHuman->m_vecPosition.x;
	int nSY = (int)m_pMyHuman->m_vecPosition.y;
	if (m_stMoveStop.NextX)
	{
		nSX = m_stMoveStop.NextX;
		nSY = m_stMoveStop.NextY;
	}

	int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
	int nMobAttackRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
	int nDX = nTX;
	int nDY = nTY;
	BASE_GetHitPosition(nSX, nSY, &nDX, &nDY, (char*)m_HeightMapData, 8);
	if (nMobAttackRange == -1)
		nMobAttackRange = 1;

	if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
	{
		nDistance -= 12;
		if (nDistance < 0)
			nDistance = 0;
		nDX = (signed int)pOver->m_vecPosition.x;
		nDY = (signed int)pOver->m_vecPosition.y;
	}
	if (BASE_GetItemAbility(&pMobData->Equip[6], 21) < 100)
	{
		if (nSpecForce + BASE_GetMobAbility(pMobData, 27) > nMobAttackRange)
			nMobAttackRange = nSpecForce + BASE_GetMobAbility(pMobData, 27);
	}

	if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
	{
		GetItemFromGround(dwServerTime);
		return 0;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		if (nDistance > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
			return 0;
	}

	if (!TMFieldScene::m_bPK && !m_pMyHuman->m_cMantua)
	{
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000
			&& pOver->IsInPKZone() == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 0;
		}
	}

	if (!TMFieldScene::m_bPK
		&& g_bCastleWar > 0
		&& m_pMyHuman->m_cMantua > 0
		&& pOver->m_cMantua > 0
		&& m_pMyHuman->m_cMantua == pOver->m_cMantua
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		return 0;
	}

	if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild &&
		pOver->m_dwID > 0 && pOver->m_dwID < 1000 && g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (!g_bCastleWar)
			return 0;
		if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 0;
	}

	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
	{
		int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
			((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cSummons == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !isInPos)
		{
			return 0;
		}
	}

	MSG_Attack stAttack{};

	stAttack.Header.Type = MSG_Attack_Multi_Opcode;
	stAttack.Header.ID = m_pMyHuman->m_dwID;
	stAttack.AttackerID = m_pMyHuman->m_dwID;
	stAttack.PosX = m_stMoveStop.NextX;
	stAttack.PosY = m_stMoveStop.NextY;
	stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
	stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;
	stAttack.CurrentMp = -1;
	stAttack.SkillIndex = (unsigned char)cSkillIndex;
	stAttack.SkillParm = 0;
	stAttack.Motion = -1;
	stAttack.FlagLocal = 0;
	stAttack.Dam[0].TargetID = pOver->m_dwID;

	pTarget = (TMHuman*)g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
	if (!pTarget)
		return 0;

	int nTargetIndex = 0;
	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
		stAttack.Dam[nTargetIndex].Damage = -2;
		stAttack.Progress = TMFieldScene::m_usProgress;
		++nTargetIndex;
	}
	stAttack.Dam[nTargetIndex].TargetID = pTarget->m_dwID;
	stAttack.Dam[nTargetIndex++].Damage = -1;
	SendAreaAttack(pTarget, pOver, cSkillIndex, nDX, nDY, stAttack, nTargetIndex);

	stAttack.TargetX = (int)pOver->m_vecPosition.x;
	stAttack.TargetY = (int)pOver->m_vecPosition.y;
	if (cSkillIndex)
	{
		stAttack.PosX = m_stMoveStop.NextX;
		stAttack.PosY = m_stMoveStop.NextY;
		stAttack.TargetX = m_stMoveStop.NextX;
		stAttack.TargetY = m_stMoveStop.NextY;
	}

	int Size = sizeof(MSG_Attack);
	if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget == 1)
	{
		stAttack.Header.Type = MSG_Attack_One_Opcode;
		Size = sizeof(MSG_AttackOne);
	}
	if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget == 2)
	{
		stAttack.Header.Type = MSG_Attack_Two_Opcode;
		Size = sizeof(MSG_AttackTwo);
	}
	SendOneMessage((char*)&stAttack, Size);
	IncSkillSel();

	MSG_Attack stLocalAttack{};
	memcpy(&stLocalAttack, &stAttack, sizeof(stLocalAttack));

	stLocalAttack.Header.ID = m_dwID;
	stLocalAttack.FlagLocal = 1;

	if (nSpecForce)
		stLocalAttack.DoubleCritical |= 8u;

	OnPacketEvent(stAttack.Header.Type, (char*)&stLocalAttack);
	m_dwOldAttackTime = dwServerTime;
	m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
	m_pMyHuman->m_pMoveSkillTargetHuman = 0;

	return 1;
}

/// Uso automático de skill sem alvo / em área (`TargetType` 0, 3, 4, 5, 6).
/// @note Extraído de `TMFieldScene::AutoSkillUse` (Fase 3); locais da função passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::AutoSkillUse_CastSelfOrArea(D3DXVECTOR3& vec, unsigned int& dwServerTime, int& nSpecForce, TMHuman*& pOver, char& cSkillIndex, int& TarType, int& nReturnValue)
{
if (!TarType || TarType == 3 || TarType == 4 || TarType == 5 || TarType == 6)
{
	int nSpecial = m_pMyHuman->m_stScore.Level;
	if ((unsigned char)cSkillIndex < 96)
		nSpecial = g_pObjectManager->m_stMobData.CurrentScore.Special[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

	if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.SaveMana, nSpecial) > g_pObjectManager->m_stMobData.CurrentScore.Mp)
	{
		auto pChatList = m_pChatList;

		pChatList->AddItem(new SListBoxItem(g_pMessageStringTable[30],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1,
			0));

		GetSoundAndPlay(33, 0, 0);
		{ nReturnValue = 0; return true; }
	}

	MSG_Attack stAttack{};
	stAttack.Header.Type = 871;
	stAttack.Header.ID = m_pMyHuman->m_dwID;
	stAttack.AttackerID = m_pMyHuman->m_dwID;
	stAttack.PosX = (int)m_pMyHuman->m_vecPosition.x;
	stAttack.PosY = (int)m_pMyHuman->m_vecPosition.y;
	stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
	stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;
	if (m_stMoveStop.NextX)
	{
		stAttack.PosX = m_stMoveStop.NextX;
		stAttack.TargetX = stAttack.PosX;
		stAttack.PosY = m_stMoveStop.NextY;
		stAttack.TargetY = stAttack.PosY;
	}
	stAttack.CurrentMp = -1;
	stAttack.SkillIndex = (unsigned char)cSkillIndex;
	stAttack.SkillParm = 0;
	stAttack.Motion = -1;

	if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 5)
	{
		stAttack.FlagLocal = 0;
		int nTargetIndex = 0;
		int nCritical = (unsigned char)g_pObjectManager->m_stMobData.Critical;

		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		if (!pNode)
			{ nReturnValue = 0; return true; }

		float fMyAngle = atan2f(vec.x - m_pMyHuman->m_vecPosition.x,
			vec.z - m_pMyHuman->m_vecPosition.y);

		int nMastery = m_pMyHuman->m_stScore.Special[3] / 75;
		if (nMastery > 3)
			nMastery = 3;

		int nMX = (int)m_pMyHuman->m_vecPosition.x;
		int nMY = (int)m_pMyHuman->m_vecPosition.y;
		int nMobCount = 0;

		while (pNode && nMobCount <= 1000)
		{
			++nMobCount;

			if (pNode == m_pMyHuman)
			{
				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				continue;
			}

			int nDX = (int)pNode->m_vecPosition.x;
			int nDY = (int)pNode->m_vecPosition.y;
			int x1 = (int)m_pMyHuman->m_vecPosition.x;
			int y1 = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				x1 = m_stMoveStop.NextX;
				y1 = m_stMoveStop.NextY;
			}

			int nDistanceFromMe = BASE_GetDistance(x1, y1, nDX, nDY);
			if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
			{
				nDistanceFromMe -= 12;
				if (nDistanceFromMe < 0)
					nDistanceFromMe = 0;
			}

			if (nDistanceFromMe <= nMastery + 3)
			{
				float fNodeAngle = atan2f(pNode->m_vecPosition.x - m_pMyHuman->m_vecPosition.x,
					pNode->m_vecPosition.y - m_pMyHuman->m_vecPosition.y);
			
				if (fabsf(fNodeAngle - fMyAngle) >= 0.78539819f)
				{
					int nDTX = nDX;
					int nDTY = nDY;
					BASE_GetHitPosition(x1, y1, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
					if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
					{
						nDTX = nDX;
						nDTY = nDY;
					}

					if (nDTX != nDX || nDTY != nDY)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}
					if (pNode->m_cDie == 1)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					if (!TMFieldScene::m_bPK && pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
					stAttack.Dam[nTargetIndex++].Damage = -1;
					stAttack.TargetX = nDX;
					stAttack.TargetY = nDY;
				}
			}

			pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
			if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget <= nTargetIndex || nTargetIndex >= 13)
				break;
		}
	}
	else if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 4 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 6)
	{
		stAttack.FlagLocal = 0;
		int nTargetIndex = 0;
		int nCritical = (unsigned char)g_pObjectManager->m_stMobData.Critical;

		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		if (!pNode)
			{ nReturnValue = 0; return true; }

		int nSX = (int)vec.x;
		int nSY = (int)vec.z;
		if (!(int)vec.x && !nSY && pOver)
		{
			nSX = (int)pOver->m_vecPosition.x;
			nSY = (int)pOver->m_vecPosition.y;
		}

		int nMobCount = 0;
		while (pNode && nMobCount <= 1000)
		{
			++nMobCount;

			int x2 = (int)pNode->m_vecPosition.x;
			int y2 = (int)pNode->m_vecPosition.y;

			if (cSkillIndex == 0|| cSkillIndex == 16 || cSkillIndex == 7 || cSkillIndex == 17 || cSkillIndex == 23 || 
				cSkillIndex == 35 || cSkillIndex == 39 || cSkillIndex == 51	|| cSkillIndex == 55)
			{
				if (!pOver)
					{ nReturnValue = 0; return true; }

				int nSX = (int)pOver->m_vecPosition.x;
				int nSY = (int)pOver->m_vecPosition.y;
				if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
				{
					int nDTX = (int)m_pMyHuman->m_vecPosition.x;
					int nDTY = (int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						nDTX = m_stMoveStop.NextX;
						nDTY = m_stMoveStop.NextY;
					}
					int nDistanceFromMe = BASE_GetDistance(
						nDTX,
						nDTY,
						(int)pOver->m_vecPosition.x,
						(int)pOver->m_vecPosition.y)
						- 12;
					if (nDistanceFromMe < 1)
					{
						nSX = (int)pOver->m_vecPosition.x;
						nSY = (int)pOver->m_vecPosition.y;
					}
				}
			}

			if (cSkillIndex == 79 && pNode != pOver)
			{
				if (!pOver)
					{ nReturnValue = 0; return true; }

				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				nSX = (int)pOver->m_vecPosition.x;
				nSY = (int)pOver->m_vecPosition.y;
				continue;
			}

			if (pNode && pNode->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && 
				(m_pMyHuman->m_usGuild == pNode->m_usGuild || g_pObjectManager->m_usAllyGuild == pNode->m_usGuild))
			{
				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				continue;
			}

			int x1 = (int)m_pMyHuman->m_vecPosition.x;
			int y1 = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				x1 = m_stMoveStop.NextX;
				y1 = m_stMoveStop.NextY;
			}

			int nDistanceFromMe = BASE_GetDistance(x1, y1, nSX, nSY);

			if (!cSkillIndex && nDistanceFromMe == 2 && (nSX == (int)m_pMyHuman->m_vecPosition.x || nSY == (int)m_pMyHuman->m_vecPosition.y))
				nDistanceFromMe = 1;
			if (pOver)
			{
				if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
				{
					nDistanceFromMe -= 12;
					if (nDistanceFromMe < 0)
						nDistanceFromMe = 0;
				}
			}
			if (cSkillIndex == 0 || cSkillIndex == 35 || cSkillIndex == 16 || cSkillIndex == 7 || cSkillIndex == 17 || 
				cSkillIndex == 23 || cSkillIndex == 39 || cSkillIndex == 51 || cSkillIndex == 55 || cSkillIndex == 95)
			{
				if (nDistanceFromMe > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range && g_pSpell[(unsigned char)cSkillIndex].Range != -1)
					{ nReturnValue = 0; return true; }
				if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
				{
					if (nDistanceFromMe > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
						{ nReturnValue = 0; return true; }
				}

				int sx = (int)m_pMyHuman->m_vecPosition.x;
				int sy = (int)m_pMyHuman->m_vecPosition.y;
				int tx = nSX;
				int ty = nSY;
				BASE_GetHitPosition(sx, sy, &tx, &ty, (char*)m_HeightMapData, 8);
				if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
				{
					tx = nSX;
					ty = nSY;
				}
				if (tx != nSX || ty != nSY)
					{ nReturnValue = 0; return true; }
			}
			else
			{
				nSX = (int)m_pMyHuman->m_vecPosition.x;
				nSY = (int)m_pMyHuman->m_vecPosition.y;
				if (m_stMoveStop.NextX)
				{
					nSX = m_stMoveStop.NextX;
					nSY = m_stMoveStop.NextY;
				}
			}

			int nDistance = BASE_GetDistance(nSX, nSY, x2, y2);
			int nTX = x2;
			int nTY = y2;
			BASE_GetHitPosition2(nSX, nSY, &nTX, &nTY, (char*)m_HeightMapData, 8);
			int nGridDistance = -1;
			switch (g_pSpell[(unsigned char)cSkillIndex].TargetType)
			{
			case 3:
				nGridDistance = 1;
				break;
			case 4:
				nGridDistance = 2;
				break;
			case 6:
				nGridDistance = 3;
				break;
			}

			if (pNode->IsInPKZone() && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			{
				if (pNode->m_bParty == 1)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (!TMFieldScene::m_bPK)
				{
					if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
						if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !pNode->m_cMantua && !g_bCastleWar)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}

					if (pNode->m_cSummons == 1)
					{							
						if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							} 
							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
						if (!pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}

							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
					}
					if (g_bCastleWar > 0 && !pNode->IsInCastleZone() && pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						pNode = (TMHuman*)pNode->m_pNextLink;
						continue;
					}

					if (g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
					{
						if ((pNode->m_dwID > 0 && pNode->m_dwID < 1000) || 
							(pNode->m_cMantua > 0 && pNode->m_cMantua != 4))
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}
					}
				}

				if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && !TMFieldScene::m_bPK)
				{
					if (!g_bCastleWar)
					{
						pNode = (TMHuman*)pNode->m_pNextLink;
						continue;
					}

					if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000)
					{
						pNode = (TMHuman*)pNode->m_pNextLink;
						continue;
					}
				}
			}
			if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && !pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (pNode->m_cDie == 1)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (pNode->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone() && !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (nDistance <= nGridDistance
				&& nTX == x2
				&& nTY == y2
				&& pNode != m_pMyHuman
				&& (!pNode->IsInTown() || !g_pSpell[(unsigned char)cSkillIndex].Aggressive))
			{
				if ((pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && !pNode->m_bParty && !pNode->m_cSummons) ||
					(pNode->m_dwID > 0 && pNode->m_dwID < 1000 && (!g_pSpell[(unsigned char)cSkillIndex].Aggressive || pNode->IsInPKZone())))
				{
					if (g_pSpell[(unsigned char)cSkillIndex].Range == -1 && !nTargetIndex)
					{
						stAttack.Dam[0].TargetID = pNode->m_dwID;
						stAttack.Dam[0].Damage = -2;
						stAttack.Progress = TMFieldScene::m_usProgress;
						nTargetIndex = 1;
					}

					stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
					stAttack.Dam[nTargetIndex].Damage = -1;
					if (pOver
						&& pOver == pNode
						&& nTargetIndex
						&& (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3
							|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 4
							|| g_pSpell[(unsigned char)cSkillIndex].TargetType == 6))
					{
						unsigned int dwID = stAttack.Dam[0].TargetID;
						int nDamage = stAttack.Dam[0].Damage;
						stAttack.Dam[0].TargetID = stAttack.Dam[nTargetIndex].TargetID;
						stAttack.Dam[0].Damage = stAttack.Dam[nTargetIndex].Damage;
						stAttack.Dam[nTargetIndex].TargetID = dwID;
						stAttack.Dam[nTargetIndex].Damage = nDamage;
					}

					++nTargetIndex;
				}
			}

			pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
			if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 && nTargetIndex > 7
				|| g_pSpell[(unsigned char)cSkillIndex].MaxTarget <= nTargetIndex
				|| nTargetIndex >= 13)
			{
				break;
			}
		}

		stAttack.TargetX = nSX;
		stAttack.TargetY = nSY;
	}

	if ((int)stAttack.Dam[0].TargetID > 0 || cSkillIndex == 97 || cSkillIndex == 35 || cSkillIndex == 51)
	{
		for (int i = g_pSpell[(unsigned char)cSkillIndex].MaxTarget; i < 13; ++i)
		{
			stAttack.Dam[i].TargetID = 0;
			stAttack.Dam[i].Damage = 0;
		}

		int nSize = sizeof(MSG_Attack);
		if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget == 1)
		{
			stAttack.Header.Type = MSG_Attack_One_Opcode;
			nSize = sizeof(MSG_AttackOne);
		}
		if (g_pSpell[(unsigned char)cSkillIndex].MaxTarget == 2)
		{
			stAttack.Header.Type = MSG_Attack_Two_Opcode;
			nSize = sizeof(MSG_AttackTwo);
		}

		SendOneMessage((char*)&stAttack, nSize);
		IncSkillSel();

		MSG_Attack stAttackLocal{};
		memcpy(&stAttackLocal, &stAttack, nSize);
		stAttackLocal.Header.ID = m_dwID;
		stAttackLocal.FlagLocal = 1;
		if (nSpecForce)
			stAttackLocal.DoubleCritical |= 8;

		OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
		m_dwOldAttackTime = dwServerTime;
		m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
		{ nReturnValue = 1; return true; }
	}
	if (cSkillIndex == 29)
	{
		if (m_pMyHuman->m_bParty == 1)
		{
			int nIndex = 0;
			int nIndexCount = 0;

			stIDDis stPartys[12]{};

			auto pPartyList = m_pPartyList;
			for (int k = 0; k < pPartyList->m_nNumItem; ++k)
			{
				auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[k];
				auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);

				if (pHuman && (m_pMyHuman->IsInPKZone() || pHuman->IsInPKZone() != 1))
				{
					int nMX = (signed int)m_pMyHuman->m_vecPosition.x;
					int nMY = (signed int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						nMX = m_stMoveStop.NextX;
						nMY = m_stMoveStop.NextY;
					}

					int nDistanceFromMe = BASE_GetDistance(nMX, nMY, (int)pHuman->m_vecPosition.x, (int)pHuman->m_vecPosition.y);
					if (nDistanceFromMe < nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
					{
						stPartys[nIndex].dwID = pPartyItem->m_dwCharID;
						stPartys[nIndex++].nLen = nDistanceFromMe;
					}
				}

				nIndexCount = nIndex;
				if (nIndex >= 12)
					nIndexCount = 12;

				int nMaxIndex = 0;

				for (int nIndex = 0; nIndex < nIndexCount; ++nIndex)
				{
					nMaxIndex = nIndex;

					stIDDis stMax{};
					stMax.dwID = stPartys[nIndex].dwID;
					stMax.nLen = stPartys[nIndex].nLen;
					for (int j = nIndex + 1; j < nIndexCount; ++j)
					{
						if (stMax.nLen > stPartys[j].nLen)
						{
							stMax.dwID = stPartys[j].dwID;
							stMax.nLen = stPartys[j].nLen;
							nMaxIndex = j;
						}
					}

					stPartys[nMaxIndex].dwID = stPartys[nIndex].dwID;
					stPartys[nMaxIndex].nLen = stPartys[nIndex].nLen;
					stPartys[nIndex].dwID = stMax.dwID;
					stPartys[nIndex].nLen = stMax.nLen;
				}
				int nTargetI = 0;
				for (nIndex = 0; nIndex < nIndexCount && g_pSpell[(unsigned char)cSkillIndex].MaxTarget > nTargetI && nTargetI < 13; ++nIndex)
				{
					stAttack.Dam[nTargetI].TargetID = stPartys[nIndex].dwID;
					stAttack.Dam[nTargetI++].Damage = -1;
				}
			}
		}
		else
		{
			stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
			stAttack.Dam[0].Damage = -1;
		}

		SendOneMessage((char*)&stAttack, sizeof(stAttack));

		MSG_Attack stAttackLocal{};
		memcpy(&stAttackLocal, &stAttack, sizeof(MSG_Attack));
		stAttackLocal.Header.ID = m_dwID;
		stAttackLocal.FlagLocal = 1;
		if (nSpecForce)
			stAttackLocal.DoubleCritical |= 8;

		OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
		m_dwOldAttackTime = dwServerTime;
		m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
		{ nReturnValue = 1; return true; }
	}
	if (cSkillIndex == 27)
	{
		int nMX = (int)m_pMyHuman->m_vecPosition.x;
		int nMY = (int)m_pMyHuman->m_vecPosition.y;
		if (m_stMoveStop.NextX)
		{
			nMX = m_stMoveStop.NextX;
			nMY = m_stMoveStop.NextY;
		}

		int nTX = (int)pOver->m_vecPosition.x;
		int nTY = (int)pOver->m_vecPosition.y;
		int nDistance = BASE_GetDistance(nMX, nMY, nTX, nTY);
		int nMobRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
		int nDX = nTX;
		int nDY = nTY;

		BASE_GetHitPosition(nMX, nMY, &nDX, &nDY, (char*)m_HeightMapData, 8);
		if (nDistance > nMobRange || nDX != nTX || nDY != nTY)
			{ nReturnValue = 1; return true; }

		int bMyValue = g_pAttribute[nMY / 4][nMX / 4];
		int bNodeValue = g_pAttribute[nTY / 4][nTX / 4];
		if (!(bMyValue & 0x40) && bNodeValue & 0x40)
			{ nReturnValue = 1; return true; }

		MSG_Attack Msg{};
		Msg.Header.Type = MSG_Attack_Multi_Opcode;
		Msg.Header.ID = m_pMyHuman->m_dwID;
		Msg.AttackerID = m_pMyHuman->m_dwID;
		Msg.PosX = (int)m_pMyHuman->m_vecPosition.x;
		Msg.PosY = (int)m_pMyHuman->m_vecPosition.y;
		Msg.CurrentMp = -1;
		Msg.SkillIndex = (unsigned char)cSkillIndex;
		Msg.SkillParm = 0;
		Msg.Motion = -1;
		Msg.Dam[0].TargetID = pOver->m_dwID;
		Msg.Dam[0].Damage = -1;
		Msg.TargetX = (int)m_pMyHuman->m_vecPosition.x;
		Msg.TargetY = (int)m_pMyHuman->m_vecPosition.y;
		if (m_stMoveStop.NextX)
		{
			Msg.PosX = m_stMoveStop.NextX;
			Msg.TargetX = Msg.PosX;
			Msg.PosY = m_stMoveStop.NextY;
			Msg.TargetY = Msg.PosY;
		}

		SendOneMessage((char*)&Msg, sizeof(Msg));

		MSG_Attack stAttackLocal{};
		memcpy(&stAttackLocal, &stAttack, sizeof(MSG_Attack));
		stAttackLocal.Header.ID = m_dwID;
		stAttackLocal.FlagLocal = 1;
		if (nSpecForce)
			stAttackLocal.DoubleCritical |= 8;

		OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
		m_dwOldAttackTime = dwServerTime;
		m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
		{ nReturnValue = 1; return true; }
	}
	if (cSkillIndex != 85)
		{ nReturnValue = 0; return true; }
}
	return false;
}

/// Validações de PK/guerra de guilda antes da skill automática em jogador.
/// @note Extraído de `TMFieldScene::AutoSkillUse` (Fase 3); locais da função passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::AutoSkillUse_PKCheck(TMHuman*& pOver, char& cSkillIndex, int& nReturnValue)
{
if (!TMFieldScene::m_bPK)
{
	if (pOver->m_usGuild)
	{
		if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && pOver->IsInPKZone() == 1)
			{
				if (!g_bCastleWar)
					{ nReturnValue = 0; return true; }
				if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
					{ nReturnValue = 0; return true; }
			}
		}
	}
	if (pOver->m_cSummons == 1)
	{
		if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && pOver->m_usGuild)
		{
			if (!g_bCastleWar)
				{ nReturnValue = 0; return true; }
			if (m_pMyHuman->m_cMantua > 0
				&& m_pMyHuman->m_cMantua == pOver->m_cMantua
				&& !pOver->IsInCastleZone())
			{
				{ nReturnValue = 0; return true; }
			}
		}
		if (!pOver->m_usGuild)
		{
			if (!g_bCastleWar)
				{ nReturnValue = 0; return true; }
			if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
				{ nReturnValue = 0; return true; }
		}
	}
	if (g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
	{
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cMantua > 0 && pOver->m_cMantua != 4)
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			{ nReturnValue = 0; return true; }
		}
	}
}
	return false;
}
