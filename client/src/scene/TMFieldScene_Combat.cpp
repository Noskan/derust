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

/// @file TMFieldScene_Combat.cpp
/// @brief Uso de habilidades, auto-skill, ataque/movimento de combate e uso de itens/pocoes.

/// Executa habilidade ativa sobre alvo/posicao validando alcance, mana, cooldown e animacao.
int TMFieldScene::SkillUse(int nX, int nY, D3DXVECTOR3 vec, unsigned int dwServerTime, int bMoving, TMHuman* pTarget)
{
	if (m_pPotalPanel->m_bVisible == 1)
		return 0;
	if (g_pEventTranslator->m_bAlt == 1)
		return 0;
	if (m_pMyHuman->m_cHide == 1)
		return 0;
	if (m_pMyHuman->m_cCantAttk)
		return 0;
	m_pTargetItem = 0;
	if (m_pMyHuman->m_bSkillBlack == 1)
		return 0;

	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	TMHuman* pOver = pTarget ? pTarget : m_pMouseOverHuman;

	if (pOver && pOver->m_cShadow == 1 && pOver->m_nClass == 66 && !m_pMyHuman->m_JewelGlasses)
		return 0;

	m_pTargetHuman = nullptr;

	char cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
	if ((unsigned char)cSkillIndex > 104)
		cSkillIndex += 95;

	if (pOver && cSkillIndex == 6 && pOver->m_nClass == 66)
		return 0;

	if (m_pMyHuman->m_sCostume >= 4150 && m_pMyHuman->m_sCostume < 4200 && (cSkillIndex == 64 || cSkillIndex == 66 || cSkillIndex == 68 || cSkillIndex == 70 || cSkillIndex == 71))
		return 0;

	int nCannonIndex = -1;

	if (!IsValidClassSkill((unsigned char)cSkillIndex))
		return 0;

	int ItemType = BASE_GetItemAbility(&pMobData->Equip[6], 21);
	if (cSkillIndex == 79 && ItemType != 101)
		return 0;
	if (cSkillIndex == 75 && ItemType != 101)
		return 0;
	if (cSkillIndex == 92 && ItemType != 41)
		return 0;

	switch (cSkillIndex)
	{
	case 99:
		return 0;
	case 83:
		return 0;
	case 84:
		m_ItemMixClass.ResultItemListSet(0, 0, 0);
		SetVisibleMixPanel(m_ItemMixClass.m_pMixPanel->m_bVisible == 0);
		return 1;
	case 97:
		bool bFind = false;
		for (int i = 0; i < 100; ++i)
		{
			auto pItem = (TMCannon*)g_pObjectManager->GetItemByID(i + 15001);
			if (pItem && pItem->m_stItem.sIndex == 746 && 
				m_pMyHuman->m_vecPosition.x == pItem->m_vecBasePosition.x && 
				m_pMyHuman->m_vecPosition.y == pItem->m_vecBasePosition.y)
			{
				bFind = 1;
				nCannonIndex = i + 15001;
				break;
			}
		}
		if (!bFind)
			return GetItemFromGround(dwServerTime);
		break;
	}

	if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK && m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
		return 0;
	if ((unsigned char)cSkillIndex >= 248)
		return GetItemFromGround(dwServerTime);
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return GetItemFromGround(dwServerTime);
	if (g_pSpell[(unsigned char)cSkillIndex].Passive == 1)
		return GetItemFromGround(dwServerTime);

	if (pOver)
	{
		if (g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && pOver && pOver->m_bParty == 1)
			return GetItemFromGround(dwServerTime);
		if (g_pSpell[(unsigned __int8)cSkillIndex].Aggressive == 1 && pOver && m_pMyHuman->m_usGuild && g_pObjectManager->m_usAllyGuild && 
			(m_pMyHuman->m_usGuild == pOver->m_usGuild || g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
		{
			return GetItemFromGround(dwServerTime);
		}
	}

	{
		int nReturnValue = 0;
		if (SkillUse_CastleWarCheck(dwServerTime, pOver, cSkillIndex, nReturnValue))
			return nReturnValue;
	}

	int Delay = g_pSpell[(unsigned char)cSkillIndex].Delay;
	if (m_nMySanc >= 9 && Delay >= 2)
		--Delay;
	if (m_pMyHuman->m_DilpunchJewel == 1)
		--Delay;
	if (Delay < 1)
		Delay = 1;

	if (pOver && pOver->m_nClass == 66 && pOver->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
		return 0;
	if (dwServerTime < m_dwSkillLastTime[(unsigned char)cSkillIndex] + 1000 * Delay)
		return 0;

	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 && 
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return 0;

	{
		int nReturnValue = 0;
		if (SkillUse_CastSelfOrArea(vec, dwServerTime, bMoving, nSpecForce, pMobData, pOver, cSkillIndex, nCannonIndex, nReturnValue))
			return nReturnValue;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 2 && !pOver)
	{
		pOver = m_pMyHuman;
		pOver->m_bMouseOver = 1;
	}
	if (!pOver || pOver->m_bMouseOver != 1 && !pTarget || m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
	{
		m_pMouseOverHuman = 0;
		return GetItemFromGround(dwServerTime);
	}

	if (m_pMyHuman->m_cHide == 1)
		return 1;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 1;

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

	if (pOver->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone())
		return 1;
	if ((m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		return 1;

	if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1)
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& !pOver->IsInPKZone())
	{
		return 1;
	}
	if ((pOver->m_dwID && pOver->m_dwID < 1000 || pOver->m_bParty == 1)
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& !m_pMyHuman->IsInPKZone())
	{
		return 1;
	}
	if (pOver->IsInPKZone() == 1
		&& !m_pMyHuman->IsInPKZone()
		&& !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		return 1;
	}
	if (!pOver->IsInPKZone()
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (pOver->m_cSummons == 1 || pOver->m_bParty == 1))
	{
		return 1;
	}
	if (!TMFieldScene::m_bPK
		&& pOver->m_cSummons == 1
		&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
		&& pOver->m_usGuild)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua > 0
			&& m_pMyHuman->m_cMantua == pOver->m_cMantua
			&& !pOver->IsInCastleZone())
		{
			return 1;
		}
	}
	if (!TMFieldScene::m_bPK)
	{
		if (pOver->m_usGuild)
		{
			if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
			{
				if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && pOver->IsInPKZone() == 1)
				{
					if (!g_bCastleWar)
						return 1;
					if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
						return 1;
				}
			}
		}
	}

	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
	{
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cMantua > 0 && pOver->m_cMantua != 4) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 1;
		}
	}
	if (!g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons)
			return 1;
	}
	if (pOver
		&& pOver->m_usGuild
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& (m_pMyHuman->m_usGuild == pOver->m_usGuild
			|| g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
	{
		return 1;
	}
	if (pOver && !TMFieldScene::m_bPK && pOver->m_cSummons == 1 && !pOver->m_usGuild)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 1;
	}

	if (pOver->m_dwID <= 0 && pOver->m_dwID >= 1000 && !pOver->m_bParty && !pOver->m_cSummons && pOver->IsMerchant()
		|| pOver->m_TradeDesc[0]
		|| dwServerTime <= m_dwOldAttackTime + 1000)
	{
		return GetItemFromGround(dwServerTime);
	}
	if ((pOver->m_stScore.Reserved & 0xF) == 15 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if (!m_pMyHuman->m_cMantua)
			return 1;
		if (pOver->m_cMantua > 0
			&& m_pMyHuman->m_cMantua > 0
			&& pOver->m_cMantua == m_pMyHuman->m_cMantua)
		{
			return 1;
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
	int nMobAttackRange = nSpecForce + g_pSpell[(unsigned __int8)cSkillIndex].Range;
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

	{
		int nReturnValue = 0;
		if (SkillUse_ApproachTarget(vec, dwServerTime, bMoving, nSpecForce, pOver, cSkillIndex, nTX, nTY, nDistance, nMobAttackRange, nDX, nDY, nReturnValue))
			return nReturnValue;
	}

	if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
	{
		if (nDistance > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
			return 1;
	}

	if (!TMFieldScene::m_bPK && !m_pMyHuman->m_cMantua)
	{
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000
			&& pOver->IsInPKZone() == 1
			&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
			&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{
			return 1;
		}
	}

	if (!TMFieldScene::m_bPK
		&& g_bCastleWar > 0
		&& m_pMyHuman->m_cMantua > 0
		&& pOver->m_cMantua > 0
		&& m_pMyHuman->m_cMantua == pOver->m_cMantua
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		return 1;
	}

	if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild && 
		pOver->m_dwID > 0 && pOver->m_dwID < 1000 && g_pSpell[(unsigned char)cSkillIndex].Aggressive)
	{
		if (!g_bCastleWar)
			return 1;
		if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
			return 1;
	}

	
	if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
	{
		int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
			((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
		if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_cSummons == 1) && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !isInPos)
		{
			return 1;
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
		return 1;

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

/// Durante a guerra de castelo, bloqueia skill agressiva em aliado da mesma capa (pega item do chão).
/// @note Extraído de `TMFieldScene::SkillUse` (Fase 3); locais da função passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::SkillUse_CastleWarCheck(unsigned int& dwServerTime, TMHuman*& pOver, char& cSkillIndex, int& nReturnValue)
{
if (g_bCastleWar)
{
	if (!TMFieldScene::m_bPK && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
	{
		if (pOver)
		{
			if (pOver->m_cMantua == m_pMyHuman->m_cMantua)
			{
				if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000) && pOver->IsInPKZone() == 1)
					{ nReturnValue = TMFieldScene::GetItemFromGround(dwServerTime); return true; }
			}
		}
	}
	if (!TMFieldScene::m_bPK
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& pOver
		&& pOver->m_cSummons == 1
		&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
		&& pOver->m_usGuild
		&& !pOver->IsInCastleZone())
	{
		{ nReturnValue = GetItemFromGround(dwServerTime); return true; }
	}
}
else
{
	if (!TMFieldScene::m_bPK && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && m_pMyHuman->m_cMantua > 0)
	{
		if (pOver)
		{
			if (!pOver->m_cMantua)
			{
				if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000) && pOver->IsInPKZone() == 1)
					{ nReturnValue = GetItemFromGround(dwServerTime); return true; }
			}
		}
	}
	if (!TMFieldScene::m_bPK
		&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
		&& pOver
		&& pOver->m_cSummons == 1
		&& g_pObjectManager->m_usWarGuild != pOver->m_usGuild
		&& pOver->m_usGuild)
	{
		{ nReturnValue = TMFieldScene::GetItemFromGround(dwServerTime); return true; }
	}
}
	return false;
}

/// Uso de skill sem alvo / em área (`TargetType` 0, 3, 4, 5, 6): mana, validações e envio.
/// @note Extraído de `TMFieldScene::SkillUse` (Fase 3); locais da função passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::SkillUse_CastSelfOrArea(D3DXVECTOR3& vec, unsigned int& dwServerTime, int& bMoving, int& nSpecForce, STRUCT_MOB*& pMobData, TMHuman*& pOver, char& cSkillIndex, int& nCannonIndex, int& nReturnValue)
{
if (dwServerTime > m_dwOldAttackTime + 1000 &&
	(g_pSpell[(unsigned char)cSkillIndex].TargetType == 0 || 
		g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 || 
		g_pSpell[(unsigned char)cSkillIndex].TargetType == 4 || 
		g_pSpell[(unsigned char)cSkillIndex].TargetType == 5 ||
		g_pSpell[(unsigned char)cSkillIndex].TargetType == 6))
{
	int nSpecial = m_pMyHuman->m_stScore.Level;
	if ((unsigned char)cSkillIndex < 96)
		nSpecial = g_pObjectManager->m_stMobData.CurrentScore.Special[((unsigned char)cSkillIndex - 24 * (unsigned char)g_pObjectManager->m_stMobData.Class) / 8 + 1];

	auto pChatList = m_pChatList;
	if (BASE_GetManaSpent((unsigned char)cSkillIndex, (unsigned char)g_pObjectManager->m_stMobData.SaveMana, nSpecial) > g_pObjectManager->m_stMobData.CurrentScore.Mp)
	{
		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[30],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1,
			0);

		pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		{ nReturnValue = 0; return true; }
	}
	if (cSkillIndex == 85 && (100 * m_pMyHuman->m_stScore.Special[2]) > g_pObjectManager->m_stMobData.Coin)
	{
		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[155],
			0xFFFFAAAA,
			0.0f,
			0.0f,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1,
			0);

		pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		{ nReturnValue = 0; return true; }
	}
	if (m_pMyHuman->IsInTown() == 1	&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
		{ nReturnValue = 0; return true; }

	MSG_Attack stAttack{};
	stAttack.Header.Type = MSG_Attack_Multi_Opcode;
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
			{ nReturnValue = 1; return true; }

		float fMyAngle = atan2f(vec.x - m_pMyHuman->m_vecPosition.x, vec.z - m_pMyHuman->m_vecPosition.y);

		int nMastery = m_pMyHuman->m_stScore.Special[3] / 75;
		if (nMastery > 3)
			nMastery = 3;

		int nMX = (int)m_pMyHuman->m_vecPosition.x;
		int nMY = (int)m_pMyHuman->m_vecPosition.y;
		int nMobCount = 0;

		SkillUse_CollectSelfAreaTargets(cSkillIndex, stAttack, nTargetIndex, pNode, fMyAngle, nMastery, nMobCount);
	}
	else if (g_pSpell[(unsigned char)cSkillIndex].TargetType == 3 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 4 || g_pSpell[(unsigned char)cSkillIndex].TargetType == 6)
	{
		stAttack.FlagLocal = 0;
		int nTargetIndex = 0;
		int nCritical = (unsigned char)g_pObjectManager->m_stMobData.Critical;

		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		if (!pNode)
			{ nReturnValue = 1; return true; }

		int nSX = (int)vec.x;
		int nSY = (int)vec.z;
		if (!(int)vec.x && !nSY && pOver)
		{
			nSX = (signed int)pOver->m_vecPosition.x;
			nSY = (signed int)pOver->m_vecPosition.y;
		}

		int nMobCount = 0;

		while (pNode != nullptr && nMobCount < 1000)
		{
			++nMobCount;
			int x2 = (int)pNode->m_vecPosition.x;
			int y2 = (int)pNode->m_vecPosition.y;

			if (SkillUse_AreaCenterOnTarget(pOver, cSkillIndex, nSX, nSY, nReturnValue))
				return true;
			if (cSkillIndex == 95 && pNode != pOver)
			{
				if (!pOver)
					{ nReturnValue = 0; return true; }

				pNode = (TMHuman*)pNode->m_pNextLink;
				nSX = (int)pOver->m_vecPosition.x;
				nSY = (int)pOver->m_vecPosition.y;
				continue;
			}

			if (pNode && pNode->m_usGuild && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 &&
				(m_pMyHuman->m_usGuild == pNode->m_usGuild || g_pObjectManager->m_usAllyGuild == pNode->m_usGuild))
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				nMX = m_stMoveStop.NextX;
				nMY = m_stMoveStop.NextY;
			}

			int nDistanceFromMe = BASE_GetDistance(nMX, nMY, nSX, nSY);
			if (cSkillIndex == 0 && nDistanceFromMe == 2 && (nSX == (int)m_pMyHuman->m_vecPosition.x || nSY == (int)m_pMyHuman->m_vecPosition.y))
			{
				nDistanceFromMe = 1;
			}
			if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
			{
				nDistanceFromMe -= 12;
				if (nDistanceFromMe < 0)
					nDistanceFromMe = 0;
			}
			if (cSkillIndex == 0 || cSkillIndex == 35 || cSkillIndex == 16 || cSkillIndex == 7 || cSkillIndex == 17 ||
				cSkillIndex == 23 || cSkillIndex == 39 || cSkillIndex == 51 || cSkillIndex == 55 || cSkillIndex == 79 || cSkillIndex == 97)
			{
				if (nDistanceFromMe > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range && g_pSpell[(unsigned char)cSkillIndex].Range != -1)
				{
					if (cSkillIndex == 35 || cSkillIndex == 39 || cSkillIndex == 97)
						{ nReturnValue = 0; return true; }

					int tx = nSX;
					int ty = nSY;
					bool bHit = true;

					BASE_GetHitPosition((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y, &tx, &ty, (char*)m_HeightMapData, 8);

					if (tx != nSX || ty != nSY)
						bHit = false;

					if (!bMoving)
						{ nReturnValue = 0; return true; }

					if (nDistanceFromMe < nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
						{ nReturnValue = 0; return true; }

					if (!bHit)
						{ nReturnValue = 0; return true; }

					if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
					{
						unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
						if (dwMod > 2)
							dwMod -= 2;

						if (g_pEventTranslator->button[0])
						{
							if (dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
								{ nReturnValue = 1; return true; }
						}
					}

					if (m_pGround == nullptr)
						{ nReturnValue = 0; return true; }

					if (vec.y < -5000.0f && pOver)
					{
						vec.x = pOver->m_vecPosition.x;
						vec.z = pOver->m_vecPosition.y;
						vec.y = pOver->m_fHeight;
					}

					m_pMyHuman->SetSpeed(m_bMountDead);
					if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
						{ nReturnValue = 1; return true; }

					m_pTargetItem = nullptr;
					m_pMyHuman->m_pMoveSkillTargetHuman = pOver;
					m_dwLastSetTargetHuman = dwServerTime;
					if (m_pMyHuman->m_cCantMove)
						{ nReturnValue = 0; return true; }

					int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
					int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;
					if (m_stMoveStop.NextX)
					{
						nMoveSX = m_stMoveStop.NextX;
						nMoveSY = m_stMoveStop.NextY;
					}

					if (!pOver)
						{ nReturnValue = 0; return true; }

					tx = (int)pOver->m_vecPosition.x;
					ty = (int)pOver->m_vecPosition.y;

					int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, tx, ty);
					int PlusX = 1;
					int PlusY = 1;
					if (nMoveSX > tx)
						PlusX = -1;
					if (nMoveSY > ty)
						PlusY = -1;

					int nBreak = 0;
					while (nMoveDistance2 > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
					{
						if (nBreak > 500)
							{ nReturnValue = 1; return true; }

						if (nMoveSX != tx)
							nMoveSX += PlusX;
						if (nMoveSY != ty)
							nMoveSY += PlusY;

						nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, tx, ty);
						++nBreak;
					}

					m_vecMyNext.x = nMoveSX;
					m_vecMyNext.y = nMoveSY;
					{ nReturnValue = 0; return true; }
				}
				if (cSkillIndex == 97)
				{
					if (nDistanceFromMe < 4)
						{ nReturnValue = 1; return true; }

					auto pCannon = (TMItem*)g_pObjectManager->GetItemByID(nCannonIndex);
					float fAngle = atan2f((float)(nSX - (int)m_pMyHuman->m_vecPosition.x), (float)(nSY - (int)m_pMyHuman->m_vecPosition.y));

					if (fAngle < 0.0f)
						fAngle = fAngle + D3DXToRadian(360);
					if (pCannon->m_fAngle - D3DXToRadian(90) > fAngle)
						{ nReturnValue = 1; return true; }
					if (fAngle > pCannon->m_fAngle + D3DXToRadian(90))
						{ nReturnValue = 1; return true; }
				}
				if (cSkillIndex == 35)
				{
					if ((pOver->m_dwID > 0 && pOver->m_dwID < 1000 || pOver->m_bParty == 1) &&
						g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && !pOver->IsInPKZone())
					{
						{ nReturnValue = 1; return true; }
					}
				}
				if (g_pSpell[(unsigned char)cSkillIndex].Range == -1)
				{
					if (nDistanceFromMe > nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27))
						{ nReturnValue = 1; return true; }
				}

				int sx = (int)m_pMyHuman->m_vecPosition.x;
				int sy = (int)m_pMyHuman->m_vecPosition.y;
				int nDTX = nSX;
				int nDTY = nSY;
				BASE_GetHitPosition(sx, sy, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
				if (pOver && pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
				{
					nDTX = nSX;
					nDTY = nSY;
				}

				if (nDTX != nSX || nDTY != nSY)
					{ nReturnValue = 1; return true; }
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

			if (pNode->IsInPKZone() == 1
				&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1
				&& m_pMyHuman)
			{
				if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
				{
					if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
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
				}

				if (TMFieldScene::m_bPK == 1
					&& g_bCastleWar > 0
					&& m_pMyHuman->m_cMantua > 0
					&& pOver
					&& pOver->m_cMantua > 0
					&& m_pMyHuman->m_cMantua == pOver->m_cMantua
					&& g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
				{
					int isInPos = (int)m_pMyHuman->m_vecPosition.x >> 7 != 8 && (int)m_pMyHuman->m_vecPosition.x >> 7 != 9 || 
								  (int)m_pMyHuman->m_vecPosition.y >> 7 != 15 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 16 ? 0 : 1;

					if (isInPos)
						{ nReturnValue = 1; return true; }
				}

				if (!TMFieldScene::m_bPK
					&& pNode->m_cSummons == 1
					&& g_pObjectManager->m_usWarGuild != pNode->m_usGuild
					&& pNode->m_usGuild)
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

				if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && !pNode->m_usGuild)
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

				if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
				{
					if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}
				}

				if (!TMFieldScene::m_bPK
					&& g_bCastleWar > 0
					&& m_pMyHuman->m_cMantua == 3)
				{
					if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					if (pNode->m_cMantua > 0 && pNode->m_cMantua != 4)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}
				}

				if (pNode->m_bParty == 1)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (!TMFieldScene::m_bPK)
				{
					if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
					{
						if (!pNode->m_cMantua && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
					}
				}
			}

			if ((pNode->m_dwID <= 0 || pNode->m_dwID >= 1000) && !pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
			{
				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				continue;
			}

			if (pNode->m_cDie == 1)
			{
				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
				continue;
			}
			if (pNode->IsInPKZone() == 1 && !m_pMyHuman->IsInPKZone() && !g_pSpell[(unsigned char)cSkillIndex].Aggressive)
			{
				pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
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
	else if (cSkillIndex == 29 || cSkillIndex == 44)
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
	}
	else
	{
		if (cSkillIndex == 42)
		{
			if (m_pMyHuman->m_bParty == 1)
				m_pMessagePanel->SetMessage(g_pMessageStringTable[31], 1000);
			else
				m_pMessagePanel->SetMessage(g_pMessageStringTable[32], 1000);

			m_pMessagePanel->SetVisible(1, 1);
			{ nReturnValue = 1; return true; }
		}
		if (cSkillIndex == 73)
		{
			if (g_pObjectManager->m_stMobData.CurrentScore.Mp < g_pSpell[73].ManaSpent)
				{ nReturnValue = 1; return true; }

			if (dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000)
			{
				int targetx = (signed int)vec.x;
				int targety = (int)vec.z;
				int x = (int)m_pMyHuman->m_vecPosition.x;
				int y = (int)m_pMyHuman->m_vecPosition.y;

				char cRouteBuffer[48]{};
				BASE_GetRoute(x, y, &targetx, &targety, cRouteBuffer, 8, (char*)m_HeightMapData, 8);
				if (!strlen(cRouteBuffer))
					{ nReturnValue = 1; return true; }

				MSG_Action stAction{};
				stAction.Header.ID = m_pMyHuman->m_dwID;
				stAction.PosX = x;
				stAction.PosY = y;
				stAction.Effect = 6;
				stAction.Header.Type = MSG_Action2_Opcode;
				stAction.Speed = g_nMyHumanSpeed;
				stAction.TargetX = targetx;
				stAction.TargetY = targety;
				m_stMoveStop.LastX = x;
				m_stMoveStop.LastY = stAction.PosY;
				m_stMoveStop.NextX = stAction.TargetX;
				m_stMoveStop.NextY = stAction.TargetY;
				SendOneMessage((char*)&stAction, sizeof(stAction));
				IncSkillSel();
				m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
				m_pMyHuman->m_dwOldMovePacketTime = dwServerTime;
			}
			{ nReturnValue = 1; return true; }
		}

		stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].Damage = -1;
		stAttack.TargetX = (int)m_pMyHuman->m_vecPosition.x;
		stAttack.TargetY = (int)m_pMyHuman->m_vecPosition.y;

		if (m_stMoveStop.NextX)
		{
			stAttack.TargetX = m_stMoveStop.NextX;
			stAttack.TargetY = m_stMoveStop.NextY;
		}
		if (cSkillIndex == 56
			|| cSkillIndex == 57
			|| cSkillIndex == 58
			|| cSkillIndex == 59
			|| cSkillIndex == 60
			|| cSkillIndex == 61
			|| cSkillIndex == 62
			|| cSkillIndex == 63
			|| (unsigned char)cSkillIndex == 200
			|| (unsigned char)cSkillIndex == 216)
		{
			m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
			m_pMyHuman->m_dwOldMovePacketTime = dwServerTime;
		}
	}
	
	if (cSkillIndex == 98)
	{
		int nTX = (int)vec.x;
		int nTY = (int)vec.z;
		int nMX = (int)m_pMyHuman->m_vecPosition.x;
		int nMY = (int)m_pMyHuman->m_vecPosition.y;
		if (m_stMoveStop.NextX)
		{
			nMX = m_stMoveStop.NextX;
			nMY = m_stMoveStop.NextY;
		}

		int nDistance = BASE_GetDistance(nMX, nMY, nTX, nTY);
		int nMobAttackRange = nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range;
		int nDX = nTX;
		int nDY = nTY;
		BASE_GetHitPosition(nMX, nMY, &nDX, &nDY, (char*)m_HeightMapData, 8);
		if (nMobAttackRange == -1)
			nMobAttackRange = 1;

		if (BASE_GetItemAbility(&pMobData->Equip[6], 21) < 100)
		{
			if (nSpecForce + BASE_GetMobAbility(pMobData, 27) > nMobAttackRange)
				nMobAttackRange = nSpecForce + BASE_GetMobAbility(pMobData, 27);
		}

		BASE_GetDistance(nMX, nMY, nDX, nDY);
		if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
			{ nReturnValue = 1; return true; }

		if (BASE_GetDistance(nMX, nMY, (int)vec.x, (int)vec.z) > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
			{ nReturnValue = 1; return true; }
		stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].Damage = -1;
		stAttack.TargetX = (int)vec.x;
		stAttack.TargetY = (int)vec.z;
		int bValue = g_pAttribute[(int)stAttack.TargetY >> 2][(int)stAttack.TargetX >> 2];
		if (bValue & 1 && !(bValue & 0x40))
			{ nReturnValue = 1; return true; }
	}
	if (cSkillIndex == 102)
	{
		int nCls = m_pMyHuman->m_sHeadIndex % 10;
		if (!IsValidSkill(31) && !IsValidSkill(39) && !IsValidSkill(47))
		{
			m_pMessagePanel->SetMessage(g_pMessageStringTable[357], 1000);
			m_pMessagePanel->SetVisible(1, 1);
			{ nReturnValue = 1; return true; }
		}
	}
	if ((int)stAttack.Dam[0].TargetID > 0 || cSkillIndex == 97 || cSkillIndex == 35 || cSkillIndex == 51)
	{
		for (int l = g_pSpell[(unsigned char)cSkillIndex].MaxTarget; l < 13; ++l)
		{
			stAttack.Dam[l].TargetID = 0;
			stAttack.Dam[l].Damage = 0;
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

		MSG_Attack stAttackLocal{};
		memcpy(&stAttackLocal, &stAttack, nSize);
		stAttackLocal.Header.ID = m_dwID;
		stAttackLocal.FlagLocal = 1;
		if (nSpecForce)
			stAttackLocal.DoubleCritical |= 8;

		OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
		SendOneMessage((char*)&stAttack, nSize);
		IncSkillSel();
		m_dwOldAttackTime = dwServerTime;
		m_dwSkillLastTime[(unsigned char)cSkillIndex] = dwServerTime;
	}
	if (cSkillIndex != 85)
		{ nReturnValue = 1; return true; }			
}
	return false;
}

/// `TargetType` 5: percorre os personagens próximos e preenche `stAttack.Dam` com os alvos dentro do alcance.
/// @note Extraído de `TMFieldScene::SkillUse_CastSelfOrArea` (Fase 3); locais passadas por referência.
void TMFieldScene::SkillUse_CollectSelfAreaTargets(char& cSkillIndex, MSG_Attack& stAttack, int& nTargetIndex, TMHuman*& pNode, float& fMyAngle, int& nMastery, int& nMobCount)
{
	while (pNode != nullptr && nMobCount <= 1000)
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
			float fNodeAngle = atan2f(pNode->m_vecPosition.x - m_pMyHuman->m_vecPosition.x,	pNode->m_vecPosition.y - m_pMyHuman->m_vecPosition.y);
			if (fabsf(fNodeAngle - fMyAngle) > 0.7853982f)
			{
				int nDTX = nDX;
				int nDTY = nDY;
				BASE_GetHitPosition(x1, y1, &nDTX, &nDTY, (char*)m_HeightMapData, 8);
				if (pNode->m_nClass == 56 && !pNode->m_stLookInfo.FaceMesh)
				{
					nDTX = nDX;
					nDTY = nDY;
				}

				if (nDTX != nDX && nDTY != nDY)
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (!pNode->IsInPKZone() && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1 && 
					(pNode->m_dwID > 0 && pNode->m_dwID < 1000))
				{
					pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
					continue;
				}

				if (pNode->IsInPKZone() == 1 && g_pSpell[(unsigned char)cSkillIndex].Aggressive == 1)
				{
					if (pNode->m_bParty == 1)
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}

					if (!TMFieldScene::m_bPK)
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000 && g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}

							if (m_pMyHuman->m_cMantua && pNode->m_cMantua == m_pMyHuman->m_cMantua)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}
						}

						if (pNode->m_cSummons == 1 && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}

							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}
						}

						if (pNode->m_cSummons == 1 && !pNode->m_usGuild)
						{
							if (!g_bCastleWar)
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}

							if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua == m_pMyHuman->m_cMantua && !pNode->IsInCastleZone())
							{
								pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
								continue;
							}
						}
					}

					if (m_pMyHuman->m_cMantua > 0 && pNode->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && 
						!TMFieldScene::m_bPK)
					{
						if (!g_bCastleWar)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}

						if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
					}

					if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
					{
						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
							continue;
						}
					}

					if (pNode->m_dwID <= 0 || pNode->m_dwID >= 1000 && 
						!pNode->m_bParty && !pNode->m_cSummons && pNode->IsMerchant())
					{
						pNode = static_cast<TMHuman*>(pNode->m_pNextLink);
						continue;
					}
				}

				if (pNode->m_cDie == 1)
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
	};
}

/// Skills de área centradas no alvo (0, 16, 7, 17, 23, 35, 39, 51, 55): o centro da área é a posição de `pOver`.
/// @note Extraído de `TMFieldScene::SkillUse_CastSelfOrArea` (Fase 3); locais passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::SkillUse_AreaCenterOnTarget(TMHuman*& pOver, char& cSkillIndex, int& nSX, int& nSY, int& nReturnValue)
{
	if (cSkillIndex == 0 || cSkillIndex == 16 || cSkillIndex == 7 || cSkillIndex == 17 || cSkillIndex == 23 ||
		cSkillIndex == 35 || cSkillIndex == 39 || cSkillIndex == 51 || cSkillIndex == 55)
	{
		if (!pOver)
			{ nReturnValue = 1; return true; }

		nSX = (int)pOver->m_vecPosition.x;
		nSY = (int)pOver->m_vecPosition.y;

		if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
		{
			int nMX = (int)m_pMyHuman->m_vecPosition.x;
			int nMY = (int)m_pMyHuman->m_vecPosition.y;
			if (m_stMoveStop.NextX)
			{
				nMX = m_stMoveStop.NextX;
				nMY = m_stMoveStop.NextY;
			}

			int nDistanceFromMe = BASE_GetDistance(nMX, nMY, (int)pOver->m_vecPosition.x, (int)pOver->m_vecPosition.y) - 12;
			if (nDistanceFromMe < 1)
			{
				nSX = (signed int)pOver->m_vecPosition.x;
				nSY = (signed int)pOver->m_vecPosition.y;
			}
		}
	}
	return false;
}

/// Alvo fora de alcance: aproxima o personagem antes de usar a skill.
/// @note Extraído de `TMFieldScene::SkillUse` (Fase 3); locais da função passadas por referência.
/// @return true se o bloco original executou `return` (valor em `nReturnValue`); false para seguir.
bool TMFieldScene::SkillUse_ApproachTarget(D3DXVECTOR3& vec, unsigned int& dwServerTime, int& bMoving, int& nSpecForce, TMHuman*& pOver, char& cSkillIndex, int& nTX, int& nTY, int& nDistance, int& nMobAttackRange, int& nDX, int& nDY, int& nReturnValue)
{
if (nDistance > nMobAttackRange || nDX != nTX || nDY != nTY)
{
	if (bMoving == 1 && nDistance >= nMobAttackRange && nDX == nTX && nDY == nTY)
	{
		if ((signed int)m_pMyHuman->m_eMotion >= 4 && (signed int)m_pMyHuman->m_eMotion <= 9)
		{
			unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
			if (dwMod > 2)
				dwMod -= 2;
			if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
			{
				{ nReturnValue = 1; return true; }
			}
		}

		if (!m_pGround)
			{ nReturnValue = 0; return true; }
		if (vec.y < -5000.0f && pOver)
		{
			vec.x = pOver->m_vecPosition.x;
			vec.z = pOver->m_vecPosition.y;
			vec.y = pOver->m_fHeight;
		}

		m_pMyHuman->SetSpeed(m_bMountDead);
		if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
			{ nReturnValue = 1; return true; }

		m_pTargetItem = nullptr;
		m_pMyHuman->m_pMoveSkillTargetHuman = pOver;
		m_dwLastSetTargetHuman = dwServerTime;
		if (m_pMyHuman->m_cCantMove)
			{ nReturnValue = 0; return true; }

		int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
		int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;
		if (m_stMoveStop.NextX)
		{
			nMoveSX = m_stMoveStop.NextX;
			nMoveSY = m_stMoveStop.NextY;
		}

		if (!pOver)
			{ nReturnValue = 0; return true; }

		nTX = (int)pOver->m_vecPosition.x;
		nTY = (int)pOver->m_vecPosition.y;

		int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
		int PlusX = 1;
		int PlusY = 1;
		if (nMoveSX > nTX)
			PlusX = -1;
		if (nMoveSY > nTY)
			PlusY = -1;

		int nBreak = 0;
		while (nMoveDistance2 > nSpecForce + g_pSpell[(unsigned char)cSkillIndex].Range)
		{
			if (nBreak > 500)
				{ nReturnValue = 1; return true; }

			if (nMoveSX != nTX)
				nMoveSX += PlusX;
			if (nMoveSY != nTY)
				nMoveSY += PlusY;

			nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
			++nBreak;
		}

		m_vecMyNext.x = nMoveSX;
		m_vecMyNext.y = nMoveSY;
	}
	{ nReturnValue = GetItemFromGround(dwServerTime); return true; }
}
	return false;
}

/// Monta e envia o ataque em área das skills 12, 16 e 28 (alvos ao redor de `nDX`/`nDY`).
/// @note Extraído de `TMFieldScene::SkillUse` (Fase 3); locais da função passadas por referência.
///       `AutoSkillUse` tinha uma cópia idêntica do bloco e também chama este método.
void TMFieldScene::SendAreaAttack(TMHuman*& pTarget, TMHuman*& pOver, char& cSkillIndex, int& nDX, int& nDY, MSG_Attack& stAttack, int& nTargetIndex)
{
if (cSkillIndex == 16 || cSkillIndex == 12 || cSkillIndex == 28)
{
	nDX = (int)pTarget->m_vecPosition.x - (int)m_pMyHuman->m_vecPosition.x;
	nDY = (int)pTarget->m_vecPosition.y - (int)m_pMyHuman->m_vecPosition.y;
	if (nDX > 0)
		nDX = 1;
	else if (nDX < 0)
		nDX = -1;
	if (nDY > 0)
		nDY = 1;
	else if (nDY < 0)
		nDY = -1;

	int TX = nDX + (int)pTarget->m_vecPosition.x;
	int TY = nDY + (int)pTarget->m_vecPosition.y;
	TMHuman* pNode = (TMHuman*)m_pHumanContainer->m_pDown;

	while (pNode->m_pNextLink)
	{
		if (pNode == m_pMyHuman	|| pNode == pTarget
			|| (int)pNode->m_vecPosition.x != TX
			|| (int)pNode->m_vecPosition.y != TY)
		{
			pNode = (TMHuman*)pNode->m_pNextLink;
			continue;
		}

		if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
		{
			if (!pNode->IsInPKZone() || !TMFieldScene::m_bPK || pNode->m_bParty == 1)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (!TMFieldScene::m_bPK
				&& m_pMyHuman->m_cMantua > 0
				&& m_pMyHuman->m_cMantua == pNode->m_cMantua
				&& g_bCastleWar > 0)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (!TMFieldScene::m_bPK
				&& g_bCastleWar > 0
				&& m_pMyHuman->m_cMantua == 3
				&& (pNode->m_dwID > 0 && pNode->m_dwID < 1000 || (pNode->m_cMantua > 0 && pNode->m_cMantua != 4)))
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pNode->IsInCastleZone())
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
		}
		else if (!TMFieldScene::m_bPK
			&& pNode->m_cSummons == 1
			&& (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild || !pNode->m_usGuild))
		{
			if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
		}
		else if (pOver && !TMFieldScene::m_bPK && pNode->m_cSummons == 1 && pNode->m_usGuild)
		{
			if (!pNode->IsInCastleZone() && g_bCastleWar > 0 || !g_bCastleWar)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}
		}

		stAttack.Dam[nTargetIndex].TargetID = pNode->m_dwID;
		stAttack.Dam[nTargetIndex++].Damage = -1;
		break;
	}
}
}

/// Inicia ataque basico contra o alvo selecionado ou move ate alcance valido.
int TMFieldScene::MobAttack(unsigned int wParam, D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	auto pOver = m_pMouseOverHuman;
	int nSpecForce = 0;
	auto pMobData = &g_pObjectManager->m_stMobData;

	if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
		nSpecForce = 1;

	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	if (pOver)
	{
		if (pOver->m_nClass == 66 && pOver->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
			return 0;
		if (pOver->m_bParty == 1)
			return 0;
		if (pOver->m_usGuild && (m_pMyHuman->m_usGuild == pOver->m_usGuild || g_pObjectManager->m_usAllyGuild == pOver->m_usGuild))
			return 0;
		if (pOver->m_dwID > 0 && pOver->m_dwID < 1000 && (!pOver->IsInPKZone() || !m_pMyHuman->IsInPKZone()))
			return 0;
		if (m_pMyHuman->m_cCantAttk)
			return 0;

		if (!TMFieldScene::m_bPK && pOver->m_cSummons == 1)
		{
			bool SameMantua = false;
			if (m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
				SameMantua = true;

			if (pOver->m_usGuild && g_pObjectManager->m_usWarGuild != pOver->m_usGuild && !g_bCastleWar)
			{
				if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
					return 0;

				if (m_pMyHuman->m_cMantua == pOver->m_cMantua && !pOver->IsInCastleZone())
					return 0;
			}

			if (!pOver->m_usGuild)
			{
				if (!g_bCastleWar)
					return 1;
				if (SameMantua)
					return 0;
			}
		}
	}
	if (pOver && pOver->m_bMouseOver == 1)
	{
		int IsTargetUser = pOver->m_dwID > 0 && pOver->m_dwID < 1000;

		if (pOver->m_dwID >= 1000)
		{
			if (pOver->IsMerchant())
				return 0;

			if ((pOver->m_stScore.Reserved & 0xF) == 15)
			{
				if (!m_pMyHuman->m_cMantua || m_pMyHuman->m_cMantua == 3)
					return 0;
				if (pOver->m_cMantua > 0 && m_pMyHuman->m_cMantua > 0 && pOver->m_cMantua == m_pMyHuman->m_cMantua)
					return 0;
			}
		}

		if (pOver->m_TradeDesc[0])
			return 0;
		if (m_pMyHuman->IsInTown() == 1 || pOver->IsInTown() == 1)
			return 0;

		if (!IsTargetUser && !pOver->m_bParty && !pOver->m_cSummons || pOver->IsInPKZone())
		{
			if (!m_pMyHuman->m_cMantua)
			{
				if (IsTargetUser && pOver->IsInPKZone() && !TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
					return 0;
			}
			if (!TMFieldScene::m_bPK)
			{
				if (IsTargetUser)
				{
					if (g_bCastleWar == 1 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
						return 0;
					if (!g_bCastleWar)
					{
						if (g_pObjectManager->m_usWarGuild != pOver->m_usGuild)
							return 0;
						if (m_pMyHuman->m_cMantua == pOver->m_cMantua)
							return 0;
					}
				}
			}

			if (!TMFieldScene::m_bPK && pOver->m_cSummons == 1 && g_pObjectManager->m_usWarGuild != pOver->m_usGuild && pOver->m_usGuild && !pOver->IsInCastleZone())
				return 0;
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua)
				return 0;

			int isInPos = ((int)m_pMyHuman->m_vecPosition.x >> 7 == 8 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 9) &&
				((int)m_pMyHuman->m_vecPosition.y >> 7 == 15 || (int)m_pMyHuman->m_vecPosition.y >> 7 == 16);
			if (TMFieldScene::m_bPK == 1 && g_bCastleWar > 0 && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pOver->m_cMantua && !isInPos)
				return 0;
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3)
			{
				if (IsTargetUser || pOver->m_cMantua > 0 && pOver->m_cMantua != 4)
					return 0;
			}
			if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && !pOver->IsInCastleZone())
			{
				if (IsTargetUser && !isInPos)
					return 0;
			}

			unsigned int dwAddTime = 0;
			if (dwServerTime <= m_dwOldAttackTime + 1000)
				return 1;

			int nSX = (int)m_pMyHuman->m_vecPosition.x;
			int nSY = (int)m_pMyHuman->m_vecPosition.y;
			int nDistance = BASE_GetDistance(nSX, nSY, nSX, nSY);
			int nTX = (int)pOver->m_vecPosition.x;
			int nTY = (int)pOver->m_vecPosition.y;
			nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
			if (nDistance > 1 && m_pMyHuman->m_cCantAttk == 1)
				return 0;

			int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
			BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)m_HeightMapData, 8);

			if (pOver->m_nClass == 56 && !pOver->m_stLookInfo.FaceMesh)
			{
				if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
					(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
					return 0;

				nDistance -= 12;
				if (nDistance < 0)
					nDistance = 0;

				nTX = (int)pOver->m_vecPosition.x;
				nTY = (int)pOver->m_vecPosition.y;
			}

			static int nMotionIndex = 0;
			++nMotionIndex;
			nMotionIndex %= 3;

			if (pOver)
				m_pMyHuman->m_nAttackDestID = pOver->m_dwID;

			if (nDistance <= nMobAttackRange && nTX == (signed int)pOver->m_vecPosition.x && nTY == (signed int)pOver->m_vecPosition.y)
			{
				MSG_Attack stAttack{};
				stAttack.Header.Type = MSG_Attack_One_Opcode;
				stAttack.Header.ID = m_pMyHuman->m_dwID;
				stAttack.AttackerID = m_pMyHuman->m_dwID;
				stAttack.PosX = m_stMoveStop.NextX;
				stAttack.PosY = m_stMoveStop.NextY;
				stAttack.CurrentMp = -1;
				stAttack.SkillIndex = -1;
				stAttack.SkillParm = 0;
				stAttack.Motion = nMotionIndex + 4;

				if (m_pMyHuman->m_nClass == 33)
				{
					if (nDistance < 2)
					{
						stAttack.Motion = 4;
					}
					else
					{
						stAttack.Motion = 5;
						stAttack.SkillIndex = 105;
						stAttack.SkillParm = 2;
					}
				}
				if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
				{
					stAttack.SkillIndex = 151;
					stAttack.SkillParm = 0;
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 102)
				{
					stAttack.SkillIndex = 152;
					if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 871)
						stAttack.SkillParm = 0;
					else if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 872)
						stAttack.SkillParm = 1;
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 103)
				{
					stAttack.SkillIndex = 153;
					switch (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh)
					{
					case 873:
						stAttack.SkillParm = 0;
						break;
					case 874:
						stAttack.SkillParm = 1;
						break;
					case 875:
						stAttack.SkillParm = 2;
						break;
					case 876:
						stAttack.SkillParm = 3;
						break;
					case 877:
						stAttack.SkillParm = 4;
						break;
					case 892:
						stAttack.SkillParm = 5;
						break;
					case 907:
						stAttack.SkillParm = 6;
						break;
					case 908:
						stAttack.SkillParm = 7;
						break;
					case 909:
						stAttack.SkillParm = 8;
						break;
					case 37:
						stAttack.SkillParm = 9;
						break;
					case 767:
						stAttack.SkillParm = 10;
						break;
					case 2814:
						stAttack.SkillParm = 11;
						break;
					}
				}
				else if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 104)
				{
					stAttack.SkillIndex = 104;
					if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 878)
						stAttack.SkillParm = 0;
					else if (g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh == 879)
						stAttack.SkillParm = 1;
				}

				if (m_cAutoAttack == 1)
					m_pTargetHuman = pOver;
				stAttack.Dam[0].TargetID = pOver->m_dwID;
				auto pTarget = g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
				int nCritical = (unsigned char)pMobData->Critical;
				stAttack.Dam[0].Damage = -2;
				stAttack.Progress = TMFieldScene::m_usProgress;

				BASE_GetDoubleCritical(pMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);
				stAttack.TargetX = (signed int)pOver->m_vecPosition.x;
				stAttack.TargetY = (signed int)pOver->m_vecPosition.y;
				if (pTarget->m_nClass == 66 && pTarget->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
					return 0;

				int nSize = sizeof(MSG_AttackOne);
				if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
				{
					stAttack.Header.Type = 926;
					nSize = sizeof(MSG_AttackTwo);
				}
				if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
				{
					int nDX = (int)pOver->m_vecPosition.x - (int)m_pMyHuman->m_vecPosition.x;
					int nDY = (int)pOver->m_vecPosition.y - (int)m_pMyHuman->m_vecPosition.y;
					if (nDX > 0)
						nDX = 1;
					else if (nDX < 0)
						nDX = -1;
					if (nDY > 0)
						nDY = 1;
					else if (nDY < 0)
						nDY = -1;

					int TX = nDX + (int)pOver->m_vecPosition.x;
					int TY = nDY + (int)pOver->m_vecPosition.y;

					auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;

					while (pNode->m_pNextLink != nullptr)
					{
						if (pNode == m_pMyHuman || pNode == pOver || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
						{
							pNode = (TMHuman*)pNode->m_pNextLink;
							continue;
						}

						if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
						{
							if (!pNode->IsInPKZone() && pNode->m_bParty == 1)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_pObjectManager->m_usWarGuild != pNode->m_usGuild && !g_bCastleWar)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && m_pMyHuman->m_cMantua > 0 && m_pMyHuman->m_cMantua == pNode->m_cMantua && g_bCastleWar > 0)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && m_pMyHuman->m_cMantua == 3
								&& ((pNode->m_dwID > 0 && pNode->m_dwID < 1000) || pNode->m_cMantua > 0 && pNode->m_cMantua != 4))
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
							if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && pNode->IsInCastleZone())
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}
						else
						{
							if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild)
								|| !pNode->m_usGuild)
							{
								pNode = (TMHuman*)pNode->m_pNextLink;
								continue;
							}
						}

						stAttack.Dam[1].TargetID = pNode->m_dwID;
						stAttack.Dam[1].Damage = -2;
						break;
					}

					if (pMobData->LearnedSkill[0] & 0x200000 || stAttack.Dam[1].Damage == -2)
					{
						stAttack.Header.Type = MSG_Attack_Two_Opcode;
						nSize = sizeof(MSG_AttackTwo);
					}
					else
					{
						stAttack.Header.Type = MSG_Attack_One_Opcode;
						nSize = sizeof(MSG_AttackOne);
					}
				}

				if (m_pMyHuman->m_bMoveing && (stAttack.SkillIndex == -1 || stAttack.SkillIndex == 104 || stAttack.SkillIndex > 150))
				{
					m_stMoveStop.Header.ID = m_pMyHuman->m_dwID;
					m_stMoveStop.Header.Type = 0x2CB;
					m_stMoveStop.CurrentX = stAttack.PosX;
					m_stMoveStop.CurrentY = stAttack.PosY;
					SendOneMessage((char*)&m_stMoveStop, sizeof(m_stMoveStop));
				}

				SendOneMessage((char*)&stAttack, nSize);

				MSG_Attack stAttackLocal{};
				memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
				stAttackLocal.Header.ID = m_dwID;
				stAttackLocal.FlagLocal = 1;
				if (nSpecForce)
					stAttackLocal.DoubleCritical |= 8;
				stAttackLocal.Progress = stAttack.Progress;
				OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
				m_dwOldAttackTime = dwServerTime;
			}
			else if (nDistance >= nMobAttackRange && !(wParam & 4))
			{
				if (m_pMyHuman->m_cCantMove)
					return 0;
				if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
				{
					unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
					if (dwMod > 2)
						dwMod -= 2;

					if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
						return 1;
				}

				if (m_pGround)
				{
					if (vec.y < -5000.0f && pOver)
					{
						vec.x = pOver->m_vecPosition.x;
						vec.z = pOver->m_vecPosition.y;
						vec.y = pOver->m_fHeight;
					}

					m_pMyHuman->SetSpeed(m_bMountDead);
					if (vec.x == m_pMyHuman->m_vecPosition.x && vec.z == m_pMyHuman->m_vecPosition.y)
						return 1;

					m_pTargetItem = 0;
					m_pMyHuman->m_pMoveTargetHuman = pOver;
					m_dwLastSetTargetHuman = dwServerTime;
					int nMoveSX = (int)m_pMyHuman->m_vecPosition.x;
					int nMoveSY = (int)m_pMyHuman->m_vecPosition.y;

					if (m_stMoveStop.NextX)
					{
						nMoveSX = m_stMoveStop.NextX;
						nMoveSY = m_stMoveStop.NextY;
					}
					if (pOver)
					{
						int nTX = (int)pOver->m_vecPosition.x;
						int nTY = (int)pOver->m_vecPosition.y;
						int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
						int PlusX = 1;
						int PlusY = 1;
						if (nMoveSX > nTX)
							PlusX = -1;
						if (nMoveSY > nTY)
							PlusY = -1;

						int nBreak = 0;
						while (nMoveDistance2 > nMobAttackRange)
						{
							if (nBreak > 500)
								return 1;
							if (nMoveSX != nTX)
								nMoveSX += PlusX;
							if (nMoveSY != nTY)
								nMoveSY += PlusY;
							nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
							++nBreak;
						}

						m_vecMyNext.x = nMoveSX;
						m_vecMyNext.y = nMoveSY;
					}
				}
			}
		}

		return 1;
	}

	m_pMouseOverHuman = nullptr;
	return GetItemFromGround(dwServerTime) == 1;
}

int TMFieldScene::MobMove(D3DXVECTOR3 vec, unsigned int dwServerTime)
{
	if (m_pMyHuman->m_bSliding == 1)
		return 0;
	if (dwServerTime < m_dwGetItemTime + 1000)
		return 0;
	if (m_pCargoPanel->IsVisible() == 1)
		return 1;
	if (m_pCargoPanel1->IsVisible() == 1)
		return 1;
	if (!g_pApp->m_binactive)
		return 1;
	if (dwServerTime < m_pMyHuman->m_dwOldMovePacketTime + 200)
		return 0;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 0;

	if ((m_pMyHuman->m_pMoveTargetHuman || m_pMyHuman->m_pMoveSkillTargetHuman)
		&& dwServerTime < m_dwLastSetTargetHuman + 1000)
	{
		return 0;
	}

	if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
	{
		unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
		if (dwMod > 2)
			dwMod -= 2;

		if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
			return 0;
	}

	if (m_pGround)
	{
		if (vec.y < -5000.0f)
			return 0;

		STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
		m_pMyHuman->SetSpeed(m_bMountDead);

		if (vec.x == m_pMyHuman->m_vecPosition.x &&
			vec.z == m_pMyHuman->m_vecPosition.y)
			return 1;

		TMVector2 vecC = TMVector2(vec.x, vec.z);
		TMVector2 vecD = vecC - m_pMyHuman->m_vecPosition;

		m_vecMyNext.x = (int)vec.x;
		m_vecMyNext.y = (int)vec.z;
		if (m_pMyHuman->m_fProgressRate >= 0.89999998f || m_pMyHuman->m_fProgressRate == 0.0f)
			m_pMyHuman->GetRoute(m_vecMyNext, 0, 0);

		m_pTargetHuman = 0;
		m_pMyHuman->m_pMoveTargetHuman = 0;
		m_pMyHuman->m_pMoveSkillTargetHuman = 0;
		m_pMouseOverHuman = 0;
		m_pTargetItem = 0;
	}

	return 1;
}

int TMFieldScene::MobMove2(TMVector2 vec, unsigned int dwServerTime)
{
	if (m_pMyHuman->m_bSliding == 1)
		return 1;
	if (dwServerTime < m_dwGetItemTime + 1000)
		return 1;
	if (m_stAutoTrade.TargetID == m_pMyHuman->m_dwID)
		return 1;

	if (!m_pMyHuman->m_cMount && m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_NONE && m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 && 
		m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_STAND02	&& m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_WALK && 
		m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_RUN && m_pMyHuman->m_eMotion != ECHAR_MOTION::ECMOTION_LEVELUP)
	{
		return 1;
	}
	if ((m_pMyHuman->m_pMoveTargetHuman || m_pMyHuman->m_pMoveSkillTargetHuman)
		&& dwServerTime < m_dwLastSetTargetHuman + 1000)
	{
		return 1;
	}

	if ((int)m_pMyHuman->m_eMotion >= 4 && (int)m_pMyHuman->m_eMotion <= 9)
	{
		unsigned int dwMod = MeshManager::m_BoneAnimationList[m_pMyHuman->m_nSkinMeshType].numAniCut[m_pMyHuman->m_pSkinMesh->m_nAniIndex];
		if (dwMod > 2)
			dwMod -= 2;

		if (g_pEventTranslator->button[0] && dwServerTime < m_pMyHuman->m_dwStartAnimationTime + 4 * dwMod * m_pMyHuman->m_pSkinMesh->m_dwFPS)
			return 1;
	}
	if (m_pGround)
	{
		STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
		m_pMyHuman->SetSpeed(m_bMountDead);

		if (vec.x == m_pMyHuman->m_vecPosition.x && vec.y == m_pMyHuman->m_vecPosition.y)
			return 1;

		TMVector2 vecD = vec - m_pMyHuman->m_vecPosition;
		m_vecMyNext.x = (int)vec.x;
		m_vecMyNext.y = (int)vec.y;

		if (m_pMyHuman->m_fProgressRate >= 0.89999998f || m_pMyHuman->m_fProgressRate == 0.0f)
			m_pMyHuman->GetRoute(m_vecMyNext, 0, 0);

		m_pTargetHuman = 0;
		m_pMyHuman->m_pMoveSkillTargetHuman = 0;
		m_pMyHuman->m_pMoveTargetHuman = 0;
		m_pMouseOverHuman = 0;
		m_pTargetItem = 0;
	}

	return 1;
}

void TMFieldScene::IncSkillSel()
{
	int nMax = 10;
	if (m_pGridSkillBelt3->m_bVisible)
		nMax = 20;

	int nBase = nMax - m_nAutoSkillNum;

	if (m_cAutoAttack == 1)
	{
		int nBack = g_pObjectManager->m_cSelectShortSkill;
		if (g_pObjectManager->m_cSelectShortSkill < nBase)
			g_pObjectManager->m_cSelectShortSkill = nBase;
		else if (++g_pObjectManager->m_cSelectShortSkill >= nMax)
			g_pObjectManager->m_cSelectShortSkill = nBase;

		auto pBeltGrid = m_pGridSkillBelt2;

		if (nMax == 20)
			pBeltGrid = m_pGridSkillBelt3;

		auto pItem = pBeltGrid->GetItem(g_pObjectManager->m_cSelectShortSkill - 10 * m_pGridSkillBelt3->m_bVisible, 0);

		if (!pItem)
		{
			int nStart = g_pObjectManager->m_cSelectShortSkill + 1;
			if (nStart >= nMax)
				nStart = nBase;
			for (int i = nStart; i < nMax; ++i)
			{
				pItem = pBeltGrid->GetItem(i - 10 * m_pGridSkillBelt3->m_bVisible, 0);
				if (pItem)
				{
					g_pObjectManager->m_cSelectShortSkill = i;
					break;
				}
			}
		}
		if (!pItem)
		{
			g_pObjectManager->m_cSelectShortSkill = nBack;
			if (!pBeltGrid->GetItem(g_pObjectManager->m_cSelectShortSkill - 10 * m_pGridSkillBelt3->m_bVisible, 0))
				return;
		}

		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem = m_pGridSkillBelt2->GetItem(j, 0);
			if (j == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 199;
		}

		for (int j = 0; j < 10; ++j)
		{
			auto ipCtrlItem = m_pGridSkillBelt3->GetItem(j, 0);
			if (j + 10 == g_pObjectManager->m_cSelectShortSkill && ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 200;
			else if (ipCtrlItem)
				ipCtrlItem->m_GCObj.nTextureSetIndex = 199;
		}
	}
}

void TMFieldScene::SetShortSkill(int nIndex, SGridControlItem* pGridItem)
{
	if (!pGridItem || IsPassiveSkill(pGridItem->m_pItem->sIndex))
		return;

	if (nIndex < 10)
	{
		auto pBelt2 = m_pGridSkillBelt2;
		auto pReturnItem2 = pBelt2->PickupItem(nIndex, 0);

		auto pNewItem2 = new STRUCT_ITEM;
		memcpy(pNewItem2, pGridItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewGridItem = new SGridControlItem(nullptr, pNewItem2, 0.0f, 0.0f);
		pBelt2->AddItem(pNewGridItem, nIndex, 0);

		if (g_pObjectManager->m_cSelectShortSkill == nIndex)
			pNewGridItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();
		
		SAFE_DELETE(pReturnItem2);
	}
	else
	{
		auto pBelt3 = m_pGridSkillBelt3;
		auto pReturnItem3 = pBelt3->PickupItem(nIndex - 10, 0);
		auto pNewItem3 = new STRUCT_ITEM;
		memcpy(pNewItem3, pGridItem->m_pItem, sizeof(STRUCT_ITEM));

		auto pNewGridItem = new SGridControlItem(nullptr, pNewItem3, 0.0f, 0.0f);
		pBelt3->AddItem(pNewGridItem, nIndex - 10, 0);

		if (g_pObjectManager->m_cSelectShortSkill == nIndex)
			pNewGridItem->m_GCObj.nTextureSetIndex = 200;

		g_pCursor->DetachItem();

		SAFE_DELETE(pReturnItem3);
	}

	g_pObjectManager->m_cShortSkill[nIndex] = static_cast<char>(g_pItemList[pGridItem->m_pItem->sIndex].nIndexTexture);
	MSG_SetShortSkill stSetShortSkill{};
	stSetShortSkill.Header.ID = m_pMyHuman->m_dwID;
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

	GetSoundAndPlay(31, 0, 0);
}

void TMFieldScene::SetSkillColor(TMHuman* pAttacker, char cSkillIndex)
{
	if (!pAttacker)
		return;

	unsigned int dwIndex = 221;
	if (pAttacker->m_dwID > 0 && pAttacker->m_dwID < 1000)
	{
		const static unsigned int dwTextureIndex[MAX_SPELL_LIST] = 
		{ 
			1, 5, 3, 5, 1, 5, 5, 5, 4, 0, 5, 0, 0, 5, 
			5, 5, 2, 2, 4, 1, 3, 2, 4, 5, 1, 5, 3, 1, 
			5, 5, 5, 5, 3, 1, 1, 3, 1, 0, 0, 5, 2, 0, 
			5, 1, 1, 5, 1, 5, 0, 0, 1, 1, 0, 1, 5, 5, 
			1, 0, 3, 0, 0, 0, 5, 5, 2, 5, 2, 5, 0, 5, 
			5, 5, 0, 2, 1, 0, 2, 1, 5, 5, 5, 2, 3, 2, 
			3, 5, 5, 5, 0, 5, 1, 2, 5, 5, 5, 5, 1, 5, 
			5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		if (cSkillIndex >= 0 && cSkillIndex < 248)
			dwIndex = dwTextureIndex[cSkillIndex] + 216;
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = dwIndex;
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_bHide = cSkillIndex == 75;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = dwIndex;
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_bHide = cSkillIndex == 75;
			}
		}
		return;
	}

	switch (DS_SOUND_MANAGER::m_nMusicIndex)
	{
	case 2:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 222;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 222;
		}
		break;
	case 3:
	case 6:
	case 8:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 221;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 221;
		}
		break;
	case 4:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 223;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 223;
		}
		break;
	case 5:
	case 7:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 224;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 224;
				if (pAttacker->m_sHeadIndex == 287)
				{
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[0])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 226;
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[1])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 226;
				}
				if (pAttacker->m_sHeadIndex == 283)
				{
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[0])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 227;
					if (!pAttacker->m_pSkinMesh->m_pSwingEffect[1])
						return;
					pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 227;
				}
				if (pAttacker->m_sHeadIndex == 175)
				{
					if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
					{
						pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 228;
						if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
							pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 228;
					}
				}
			}
		}
		break;
	case 9:
		if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
		{
			pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 225;
			if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
			{
				pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 225;
				if (pAttacker->m_sHeadIndex == 175)
				{
					if (pAttacker->m_pSkinMesh->m_pSwingEffect[0])
					{
						pAttacker->m_pSkinMesh->m_pSwingEffect[0]->m_dwSWTextureIndex = 228;
						if (pAttacker->m_pSkinMesh->m_pSwingEffect[1])
							pAttacker->m_pSkinMesh->m_pSwingEffect[1]->m_dwSWTextureIndex = 228;
					}
				}
			}
		}
		break;
	}
}

int TMFieldScene::GetWeaponDamage()
{
	auto pMobData = &g_pObjectManager->m_stMobData;
	int w1 = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[6], 2);
	int w2 = BASE_GetItemAbility(&pMobData->Equip[7], 2);
	int t1 = g_pItemList[pMobData->Equip[6].sIndex].nUnique;
	int t2 = g_pItemList[pMobData->Equip[7].sIndex].nUnique;

	int nWeaponDamage = 0;
	if (t1 == 47 && t2 == 45)
		w2 = 0;
	if (w1 <= w2)
		nWeaponDamage = w2 + w1 / 3;
	else
		nWeaponDamage = w1 + w2 / 3;

	int idx1 = pMobData->Equip[6].sIndex;
	int nPos1 = g_pItemList[idx1].nPos;

	if ((idx1 >= 0 || idx1 < 6500) && (nPos1 == 64 || nPos1 == 192) && t1 != 44 && t2 != 47 && BASE_GetItemSanc(&pMobData->Equip[6]) >= 9)
	{
		int nu = g_pItemList[idx1].nUnique;
		if (nu != 47 && nu != 44)
			nWeaponDamage += 40;
	}

	int idx2 = pMobData->Equip[7].sIndex;
	int nPos2 = g_pItemList[idx2].nPos;
	if ((idx2 >= 0 || idx2 < 6500) && (nPos2 == 64 || nPos2 == 192) && t1 != 44 && t2 != 47 && BASE_GetItemSanc(&pMobData->Equip[7]) >= 9)
	{
		int nu = g_pItemList[idx2].nUnique;
		if (nu != 47 && nu != 44)
			nWeaponDamage += 40;
	}

	return nWeaponDamage;
}

void TMFieldScene::MobStop(D3DXVECTOR3 vec)
{
	int nTX = static_cast<int>(vec.x);
	int nTY = static_cast<int>(vec.z);
	int nX = static_cast<int>(m_pMyHuman->m_vecPosition.x);
	int nY = static_cast<int>(m_pMyHuman->m_vecPosition.y);

	if (nTX >= nX + 1)
		++nX;

	else if (nTX <= nX - 1)
		--nX;

	if (nTY >= nY + 1)
		++nY;

	else if (nTY <= nY - 1)
		--nY;

	int nSX = m_pMyHuman->m_LastSendTargetPos.x;
	int nSY = m_pMyHuman->m_LastSendTargetPos.y;

	char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;

	char cRouteBuffer[48]{};

	int nRet = BASE_GetRoute(nSX, nSY, &nX, &nY, cRouteBuffer, 12, pHeightMapData, 8);
	if (!nRet)
	{
		nX = nSX;
		nY = nSY;
	}

	int nStart = GroundGetMask(TMVector2{ (float)nSX, (float)nSY });
	int nEnd = GroundGetMask(TMVector2{ (float)nX, (float)nY });

	int nHeight{};

	if ((nEnd - nStart) <= 0)
		nHeight = nStart - nEnd;
	else
		nHeight = nEnd - nStart;
	
	if (nHeight > 30)
	{
		nRet = BASE_GetRoute(nSX, nSY, &nX, &nY, cRouteBuffer, 6, pHeightMapData, 8);

		if (!nRet)
		{
			nX = nSX;
			nY = nSY;
		}
	}
	if (nRet && g_bLastStop != MSG_Action_Stop_Opcode && !m_pMyHuman->m_cLastMoveStop)
	{
		m_vecMyNext.x = nX;
		m_vecMyNext.y = nY;
		m_pMyHuman->m_vecTargetPos.x = nX;
		m_pMyHuman->m_vecTargetPos.y = nY;
		
		m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

		MSG_Action stAction{};

		stAction.Header.ID = m_pMyHuman->m_dwID;
		stAction.PosX = nSX;
		stAction.PosY = nSY;
		stAction.Effect = 0;
		stAction.Header.Type = MSG_Action_Stop_Opcode;
		stAction.Speed = g_nMyHumanSpeed;
		stAction.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
		stAction.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

		for (int j = 0; j < 23; ++j)
			stAction.Route[j] = 0;

		m_stMoveStop.LastX = stAction.PosX;
		m_stMoveStop.LastY = stAction.PosY;
		m_stMoveStop.NextX = stAction.TargetX;
		m_stMoveStop.NextY = stAction.TargetY;

		SendOneMessage((char*)&stAction, sizeof(stAction));

		m_pMyHuman->OnPacketEvent(MSG_Action_Stop_Opcode, (char*)&stAction);

		g_bLastStop = stAction.Header.Type;
	}

	m_pMyHuman->m_cLastMoveStop = 0;
	m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
}
