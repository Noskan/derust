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

/// @file TMFieldScene_HUD.cpp
/// @brief HUD do campo: atributos/EXP do personagem (`UpdateScoreUI`), barra de skills,
///        câmera, modos de chat/PK/corrida, quadro de avisos, fogos de artifício, tempo de quest
///        e utilitários de texto do chat.
///
/// Extraído de `TMFieldScene_UI.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

/// Atualiza os textos e barras de HUD (HP, MP, EXP, atributos, buffs e status do personagem).
void TMFieldScene::UpdateScoreUI(unsigned int unFlag)
{
	auto pMobData = &g_pObjectManager->m_stMobData;
	if (g_pObjectManager->m_stMobData.Equip[13].sIndex == 769)
	{
		int sanc = BASE_GetItemSanc(&pMobData->Equip[13]);
		int hpprogress = 0;

		if (sanc > 6)
			hpprogress = 545;
		else if (sanc > 3)
			hpprogress = 546;
		else if (sanc > 0)
			hpprogress = 547;
		else
			hpprogress = 538;
		m_pHPBar->m_GCProgress.nTextureSetIndex = hpprogress;
		m_pHPBar->Update();
	}
	else
	{
		m_pHPBar->m_GCProgress.nTextureSetIndex = 538;
		m_pHPBar->Update();
	}

	if (pMobData->CurrentScore.Hp > pMobData->CurrentScore.MaxHp)
		pMobData->CurrentScore.Hp = pMobData->CurrentScore.MaxHp;

	if (m_pHPBar)
		m_pHPBar->SetMaxProgress(pMobData->CurrentScore.MaxHp);

	if (m_pHPBar)
		m_pHPBar->SetCurrentProgress(pMobData->CurrentScore.Hp);

	char szStr[128]{};
	sprintf(szStr, "%d", pMobData->CurrentScore.Hp);
	if (m_pCHP)
		m_pCHP->SetText(szStr, 0);

	// HP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.Hp);
	if (m_pCIHP)
		m_pCIHP->SetText(szStr, 0);
	if (m_pMPBar)
		m_pMPBar->SetMaxProgress(pMobData->CurrentScore.MaxMp);
	if (m_pMPBar)
		m_pMPBar->SetCurrentProgress(pMobData->CurrentScore.Mp);

	// MP on "C"
	sprintf(szStr, "%d", pMobData->CurrentScore.Mp);
	if (m_pCMP)
		m_pCMP->SetText(szStr, 0);
	if (m_pCIMP)
		m_pCIMP->SetText(szStr, 0);

	if (m_pMainCharName)
		m_pMainCharName->SetText(pMobData->MobName, 0);

	if (!(unFlag & 0x10))
	{
		sprintf(szStr, "%I64d", pMobData->Exp);
		if (m_pCIEXP)
		{
			m_pCIEXP->m_cComma = 1;
			m_pCIEXP->SetText(szStr, 0);
		}

		long long nLevelCount = 0;
		int ckind = m_pMyHuman->Is2stClass();
		int CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			nLevelCount = (g_pNextLevel_G2[CurLevel + 1] + g_pNextLevel_G2[CurLevel]) / 10;
		else
			nLevelCount = (g_pNextLevel[CurLevel + 1] + g_pNextLevel[CurLevel]) / 10;

		nLevelCount = static_cast<int>((((float)g_pObjectManager->m_nFakeExp * 100.0f) / (float)nLevelCount));
		sprintf(szStr, g_pMessageStringTable[304], g_pObjectManager->m_nFakeExp, nLevelCount);
		if (m_pGridCharFace)
		{
			m_pGridCharFace->m_GCPanel.nTextureSetIndex = 541;
			m_pGridCharFace->m_GCPanel.nTextureIndex = ckind + 3 * pMobData->Class;
		}
		if (m_pCIFakeExp)
		{
			m_pCIFakeExp->SetText(szStr, 0);
			if (nLevelCount < 80)
				m_pCIFakeExp->SetTextColor(0xFFFFFFFF);
			else
				m_pCIFakeExp->SetTextColor(0xFFFF0000);
		}
		if (m_pExpHold)
		{
			if (g_pObjectManager->m_nFakeExp <= 0)
				m_pExpHold->SetVisible(0);
			else
				m_pExpHold->SetVisible(1);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Level + 1);
		m_Level = pMobData->CurrentScore.Level + 1;
		if (m_pCLevel)
			m_pCLevel->SetText(szStr, 0);
		if (m_pMainInfo2_Lv)
			m_pMainInfo2_Lv->SetText(szStr, 0);

		ckind = m_pMyHuman->Is2stClass();
		CurLevel = pMobData->CurrentScore.Level;
		if (ckind == 2)
			sprintf(szStr, "%I64d", g_pNextLevel_G2[CurLevel + 1]);
		else
			sprintf(szStr, "%I64d", g_pNextLevel[CurLevel + 1]);

		char strEXP[256]{};
		if (m_pCIEXPE)
		{
			m_pCIEXPE->m_cComma = 1;
			m_pCIEXPE->SetText(szStr, 0);
		}

		sprintf(szStr, "%d", pMobData->ScoreBonus);
		if (m_pScBonus)
			m_pScBonus->SetText(szStr, 0);

		sprintf(szStr, "%d", pMobData->SpecialBonus);
		if (m_pSpBonus)
			m_pSpBonus->SetText(szStr, 0);
		if (m_pSkBonus)
			m_pSkBonus->SetText(szStr, 0);

		char szMoeny[64]{};
		char szRMoeny[64]{};
		sprintf(szMoeny, "%10d", pMobData->Coin);
		sprintf(szRMoeny, "%10d", g_pObjectManager->m_RMBCount);
		m_Coin = pMobData->Coin;
		m_RMB = g_pObjectManager->m_RMBCount;
		if (m_pMoney1)
		{
			m_pMoney1->m_cComma = 1;
			m_pMoney1->SetText(szMoeny, 0);
		}
		if (m_pMoney4)
		{
			m_pMoney4->m_cComma = 1;
			m_pMoney4->SetText(szRMoeny, 0);
		}
		if (m_pMoney2)
		{
			m_pMoney2->m_cComma = 1;
			m_pMoney2->SetText(szMoeny, 0);
		}

		sprintf(szMoeny, "%10d", pMobData->Coin - m_nBet);
		if (m_pMoney3)
		{
			m_pMoney3->m_cComma = 1;
			m_pMoney3->SetText(szMoeny, 0);
		}

		char szNameTemp[128]{};
		sprintf(szNameTemp, "[%s]:%d", pMobData->MobName, strlen(pMobData->MobName));
		if (m_pCIName)
			m_pCIName->SetText(szNameTemp, 1);

		static const char* szClass[4] = {
			g_pMessageStringTable[121],
			g_pMessageStringTable[122],
			g_pMessageStringTable[123],
			g_pMessageStringTable[124]
		};
			
		char szValue[256]{};
		sprintf(szValue, "%d", m_pMyHuman->m_nTotalKill);
		m_pMyHuman->m_pKillLabel->SetText(szValue, 0);
		m_pCIGuild->SetText(szValue, 0);
		m_pCIGuild->SetTextColor(0xFFAAAAAA);
		if (pMobData->Equip[0].sIndex >= 40)
		{
			m_pCIClass->SetText((char*)"Monster", 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}
		else if (pMobData->Equip[0].sIndex % 10 >= 6)
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)szClass[pMobData->Equip[0].sIndex / 10], 0);
		}
		else
		{
			m_pCIClass->SetText((char*)szClass[pMobData->Class], 0);
			m_pCIClass2->SetText((char*)" ", 0);
		}

		sprintf(szStr, "%d", pMobData->CurrentScore.Str);
		if (m_pCIStr)
			m_pCIStr->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Int);
		if (m_pCIInt)
			m_pCIInt->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Dex);
		if (m_pCIDex)
			m_pCIDex->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->CurrentScore.Con);
		if (m_pCICon)
			m_pCICon->SetText(szStr, 0);

		int nMaxLevel = 0;
		if (m_pMyHuman->Is2stClass() == 2)
		{
			nMaxLevel = 200;
		}
		else
		{
			nMaxLevel = 3 * (pMobData->CurrentScore.Level + 1) / 2;
			if (nMaxLevel > 200)
				nMaxLevel = 200;
		}

		int nMax3 = nMaxLevel;
		int nMax2 = nMaxLevel;
		int nMax1 = nMaxLevel;
		if (!pMobData->Class && IsValidSkill(205) == 1)
			nMaxLevel = 280;
		else if (pMobData->Class == 2 && IsValidSkill(233) == 1)
			nMaxLevel = 230;
		if (pMobData->Class == 3 && IsValidSkill(238) == 1)
			nMax1 = 400;
		else if (IsValidSkill(200) == 1)
			nMax1 = 320;
		else if (IsValidSkill(31) == 1)
			nMax1 = 255;
		if (IsValidSkill(204) == 1)
			nMax2 = 320;
		else if (IsValidSkill(39) == 1)
			nMax2 = 255;
		if (IsValidSkill(208) == 1)
			nMax3 = 320;
		else if (IsValidSkill(47) == 1)
			nMax3 = 255;

		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[0], nMaxLevel);
		if (m_pCISpecial1)
			m_pCISpecial1->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[1], nMax1);
		if (m_pCISpecial2)
			m_pCISpecial2->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[2], nMax2);
		if (m_pCISpecial3)
			m_pCISpecial3->SetText(szStr, 0);
		sprintf(szStr, "%3d/%3d", pMobData->CurrentScore.Special[3], nMax3);
		if (m_pCISpecial4)
			m_pCISpecial4->SetText(szStr, 0);

		long long nMax = 0;
		long long nCur = 0;
		if (ckind == 2)
		{
			nMax = (g_pNextLevel_G2[CurLevel + 1] - g_pNextLevel_G2[CurLevel]);
			nCur = pMobData->Exp - g_pNextLevel_G2[CurLevel];
		}
		else
		{
			nMax = g_pNextLevel[CurLevel + 1] -g_pNextLevel[CurLevel];
			nCur = pMobData->Exp - g_pNextLevel[CurLevel];
		}

		int nLamp = static_cast<int>(nMax / 4);
		int nGrid = 4;

		if (nCur >= nMax / 4)
		{
			if (nCur < static_cast<long long>(2 * static_cast<long long>(nLamp)))
			{
				nGrid = 2;
			}
			else
			{				
				if (nCur < static_cast<long long>(3 * static_cast<long long>(nLamp)))
					nGrid = 3;
			}
		}
		else
		{
			nGrid = 1;
		}

		for (int l = 0; l < 3; ++l)
		{
			if (m_pExpLamp[l])
				m_pExpLamp[l]->m_GCPanel.nTextureIndex = l < nGrid - 1;
		}

		nCur -= (static_cast<long long>(nGrid) - 1) * static_cast<long long>(nLamp);
		nMax = static_cast<long long>((double)nLamp * 0.1);

		for (int n = 0; n < 10; ++n)
		{
			if (nCur > nMax)
			{
				nCur -= nMax;
				if (m_pExpProgress[n])
				{
					m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
					m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nMax));
				}
			}
			else if (m_pExpProgress[n])
			{
				m_pExpProgress[n]->SetMaxProgress(static_cast<int>(nMax));
				m_pExpProgress[n]->SetCurrentProgress(static_cast<int>(nCur));
				nCur = 0;
			}
		}

		int face = pMobData->Equip[0].sIndex;
		int cls = 0;
		if (face < 40)
		{
			if (face == 32)
			{
				cls = 2;
			}
			else
			{
				int mod = face % 10;
				int div = face / 10;
				if (face % 10 < 6 || mod > 9)
					cls = div;
				else
					cls = mod - 6;
			}
			pMobData->Class = cls;
		}

		if (!(unFlag & 1))
		{
			int sIndex = 0;
			int ItemIndex = 0;
			// Skill list on "S"
			for (int i = 0; i < 24; ++i)
			{
				unsigned int dwBit = 1 << i;

				if (!m_pSkillSecGrid[i])
					continue;

				if ((dwBit & pMobData->LearnedSkill[0]) != dwBit)
				{
					m_pSkillSecGrid[i]->Empty();
					continue;
				}

				auto pSkillItem = m_pSkillSecGrid[i]->GetItem(0, 0);
				sIndex = 24 * pMobData->Class + i + 5000;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;

					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}
				}

				m_pSkillSecGrid[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if(ipCtrlItem)
					m_pSkillSecGrid[i]->AddItemInEmpty(ipCtrlItem);
			}
			// Skill bar 2
			for (int i = 0; i < 12; ++i)
			{
				if (!m_pSkillSecGrid2)
					continue;

				if (((1 << i) & pMobData->LearnedSkill[1]) != 1 << i)
				{
					m_pSkillSecGrid2[i]->Empty();
					continue;
				}

				SGridControlItem* pSkillItem = m_pSkillSecGrid2[i]->GetItem(0, 0);
				sIndex = 12 * pMobData->Class + i + 5400;

				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = nullptr;
						continue;
					}
				}

				m_pSkillSecGrid2[i]->Empty();

				auto pStructItem = new STRUCT_ITEM;
				memset(pStructItem, 0, sizeof(STRUCT_ITEM));

				pStructItem->sIndex = sIndex;

				auto ipCtrlItem = new SGridControlItem(0, pStructItem, 0.0f, 0.0f);
				if (ipCtrlItem)
					m_pSkillSecGrid2[i]->AddItemInEmpty(ipCtrlItem);
			}
			// Skill bar 1
			for (int i = 0; i < 8; ++i)
			{
				unsigned int dwBit = 1 << (i + 24);
				if (!m_pGridSkillBelt)
					continue;

				auto pSkillItem = m_pGridSkillBelt->GetItem(i % 4, i / 4);
				sIndex = i + 5096;
				if (pSkillItem)
				{
					auto pstOldItem = pSkillItem->GetItem();
					ItemIndex = pstOldItem->sIndex;
					if (ItemIndex > 0 && sIndex == ItemIndex)
					{
						pSkillItem = 0;
						continue;
					}				
				}

				auto pOldItem = m_pGridSkillBelt->PickupItem(i % 4, i / 4);
				if ((dwBit & pMobData->LearnedSkill[0]) == dwBit)
				{
					auto pItem = new STRUCT_ITEM;
					memset(pItem, 0, sizeof(STRUCT_ITEM));
					pItem->sIndex = i + 5096;

					m_pGridSkillBelt->AddItem(new SGridControlItem(0, pItem, 0.0f, 0.0f), i % 4, i / 4);
				}
				if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pOldItem)
					g_pCursor->m_pAttachedItem = 0;

				//TODO: check this strange bOld
				bool bOld = false;
				if (bOld != 1 && pOldItem)
				{
					SAFE_DELETE(pOldItem);
				}
			}
		}
		if (m_pSLPanel1 && !pMobData->Class)
			m_pSLPanel1->SetVisible(0);
		else if (m_pSLPanel1)
		{
			m_pSLPanel1->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel1->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel2 && !pMobData->Class)
			m_pSLPanel2->SetVisible(0);
		else if (m_pSLPanel2)
		{
			m_pSLPanel2->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel2->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSLPanel3 && !pMobData->Class)
			m_pSLPanel3->SetVisible(0);
		else if (m_pSLPanel3)
		{
			m_pSLPanel3->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSLPanel3->m_GCPanel.nTextureIndex = 2;
		}
		if (m_pSkillCover && !pMobData->Class)
			m_pSkillCover->SetVisible( 0);
		else if (m_pSkillCover)
		{
			m_pSkillCover->SetVisible(1);
			if (pMobData->Class == 1)
				m_pSkillCover->m_GCPanel.nTextureIndex = 0;
			if (pMobData->Class == 2)
				m_pSkillCover->m_GCPanel.nTextureIndex = 1;
			if (pMobData->Class == 3)
				m_pSkillCover->m_GCPanel.nTextureIndex = 2;
		}

		switch (pMobData->Class)
		{
		case 0:
			m_pCISp1Caption->SetText(g_pMessageStringTable[242], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[243], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[244], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[245], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[107], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[108], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[109], 0);
			break;
		case 1:
			m_pCISp1Caption->SetText(g_pMessageStringTable[246], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[247], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[248], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[249], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[110], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[111], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[112], 0);
			break;
		case 2:
			m_pCISp1Caption->SetText(g_pMessageStringTable[250], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[251], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[252], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[253], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[113], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[114], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[115], 0);
			break;
		case 3:
			m_pCISp1Caption->SetText(g_pMessageStringTable[254], 0);
			m_pCISp2Caption->SetText(g_pMessageStringTable[255], 0);
			m_pCISp3Caption->SetText(g_pMessageStringTable[256], 0);
			m_pCISp4Caption->SetText(g_pMessageStringTable[257], 0);
			m_pSkillSec1->SetText(g_pMessageStringTable[133], 0);
			m_pSkillSec2->SetText(g_pMessageStringTable[134], 0);
			m_pSkillSec3->SetText(g_pMessageStringTable[135], 0);
			break;
		}

		int nRate = pMobData->CurrentScore.Special[0];
		int nDamageValue = pMobData->CurrentScore.Damage;
		if (nRate > 100)
			nRate = 100;

		sprintf(szStr, "%d", nDamageValue);
		if (m_pDamage)
			m_pDamage->SetText(szStr, 0);

		int cSkillIndex = g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill];
		if (cSkillIndex >= 0 && cSkillIndex < 248)
		{
			SetMyHumanMagic();
			int nIdx = g_pObjectManager->m_stMobData.Equip[6].sIndex;
			int nWeather = g_nWeather;
			if (g_nWeather == 3)
				nWeather = 2;

			if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
				nWeather = 2;

			int nSkillDam = BASE_GetSkillDamage(cSkillIndex, pMobData, nWeather, GetWeaponDamage(),
				g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);
			if (nSkillDam >= 0)
			{
				sprintf(szStr, "%d", nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
			else
			{
				sprintf(szStr, "%d", -nSkillDam);
				if (m_pSkillDam)
					m_pSkillDam->SetText(szStr, 0);
				if (m_pSkillDam)
					m_pSkillDam->SetTextColor(0xFFBBBBFF);
			}
		}
		else
		{
			sprintf(szStr, "%d", 0);
			if (m_pSkillDam)
				m_pSkillDam->SetText(szStr, 0);
		}

		if (!m_pMyHuman->m_cOnlyMove)
			m_pMyHuman->SetSpeed(m_bMountDead);

		int nSpeedValue = (int)m_pMyHuman->m_fMaxSpeed;

		sprintf(szStr, "%d", nSpeedValue);
		if (m_pSpeed)
			m_pSpeed->SetText(szStr, 0);

		int nACValue = pMobData->CurrentScore.Ac;
		sprintf(szStr, "%d", nACValue);
		if (m_pDefence)
			m_pDefence->SetText(szStr, 0);

		int nDef = BASE_GetMobAbility(pMobData, 53) + (BASE_GetMobAbility(pMobData, 3) + pMobData->BaseScore.Ac);
		sprintf(szStr, "%d", nDef);
		int nSpeedClass = 0;

		if (m_pMyHuman->m_nClass == 26 && !m_pMyHuman->m_stLookInfo.FaceMesh)
		{
			nSpeedClass = 10;
			if (pMobData->LearnedSkill[0] & 0x20000)
				nSpeedClass = 20;
		}
		else if (m_pMyHuman->m_nClass == 26 && m_pMyHuman->m_stLookInfo.FaceMesh == 1)
			nSpeedClass = 0;
		else if (m_pMyHuman->m_nClass == 33)
		{
			nSpeedClass = 20;
			if (pMobData->LearnedSkill[0] & 0x200000)
				nSpeedClass = 40;
		}
		else if(m_pMyHuman->m_nClass == 40)
			nSpeedClass = 20;
		else if (m_pMyHuman->m_nClass == 63)
			nSpeedClass = 30;
		
		int nAttSpeedValue = (pMobData->CurrentScore.Special[2] / 10 + 10)
			* (m_pMyHuman->m_cSpeedUp - m_pMyHuman->m_cSpeedDown)
			+ pMobData->CurrentScore.Dex / 5
			+ nSpeedClass
			+ BASE_GetMobAbility(pMobData, 26)
			+ 100;

		if (m_pMyHuman->m_cFreeze == 1)
			nAttSpeedValue -= 30;

		sprintf(szStr, "%d%%", nAttSpeedValue);
		if (m_pAttackSpeed)
			m_pAttackSpeed->SetText(szStr, 0);
		int nCriticalValue = pMobData->Critical;
		nCriticalValue *= 4;

		sprintf(szStr, "%d.%d%%", nCriticalValue / 10, nCriticalValue % 10);
		if (m_pCritical)
			m_pCritical->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[2]);
		if (m_pRegist1)
			m_pRegist1->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[3]);
		if (m_pRegist2)
			m_pRegist2->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[0]);
		if (m_pRegist3)
			m_pRegist3->SetText(szStr, 0);
		sprintf(szStr, "%d", pMobData->Resist[1]);
		if (m_pRegist4)
			m_pRegist4->SetText(szStr, 0);

		for (int i = 0; i < 32; ++i)
		{
			int nType = (signed int)m_pMyHuman->m_usAffect[i] >> 8;
			if (m_pAffectL[i])
			{
				if (nType >= 0 && nType < 50)
					m_pAffectL[i]->SetText(g_pAffectTable[nType], 0);
				else
					m_pAffectL[i]->SetText((char*)" ", 0);
			}
		}

		char szVal[32]{};

		for (int i = 0; i < 32; ++i)
		{
			if (m_pMyHuman->m_stAffect[i].Type > 0)
			{
				if (m_pMyHuman->m_stAffect[i].Time < 1000000)
				{
					sprintf(szVal, "%5d", m_pMyHuman->m_stAffect[i].Time);					
				}
				else
				{
					int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
					int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
					int Day = m_pMyHuman->m_stAffect[i].Time % 10000;
					int nYearDay = 365;
					if (!(Year % 4))
						nYearDay = 366;
					int nDafaultDay = 0;
					switch (add)
					{
					case 4:
						nDafaultDay = 7;
						break;
					case 5:
						nDafaultDay = 15;
						break;
					case 6:
						nDafaultDay = 30;
						break;
					}

					if (add)
					{
						int freeDay = nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - Day);
						sprintf(szVal, g_pMessageStringTable[291], freeDay);
					}
				}
				if (m_pAffectL[i] && !i)
				{
					int sTime = m_pMyHuman->m_stAffect[0].Time;
					if (sTime >= 7375)
					{
						m_pAffect[i]->SetTextColor(0xFFFFFFFF);
					}
					else
					{
						for (int nJ = 0; nJ < 7; ++nJ)
						{
							if (sTime < g_sTimeTable[nJ])
							{
								m_pAffect[i]->SetTextColor(g_dwFoodColor[nJ]);
								break;
							}
						}
					}
				}
				if (m_pMiniPanel)
				{
					if (m_pMyHuman->m_stAffect[i].Type < 50)
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[m_pMyHuman->m_stAffect[i].Type];
					else
						m_pAffectIcon[i]->m_GCPanel.nTextureIndex = 0;
					m_pAffectIcon[i]->m_GCPanel.nLayer = 29;
				}
			}	
			else
			{
				if (m_pAffectL[i])
					m_pAffectL[i]->SetVisible(0);
				if (m_pAffectIcon[i] && m_pMiniPanel)
				{
					m_pAffectIcon[i]->SetVisible(0);
					m_dwAffectBlinkTime[i] = 0;
				}
			}
		}
		if (m_pCargoCoin)
		{
			sprintf(szVal, "%10d", g_pObjectManager->m_nCargoCoin);
			m_pCargoCoin->m_cComma = 1;
			m_pCargoCoin->SetText(szVal, 0);
			if (m_pMyCargoCoin)
				m_pMyCargoCoin->SetText(szVal, 0);
		}

		if (!m_pMyHuman->m_pMantua)
			m_pKingDomFlag->SetVisible(0);
		else if (m_pMyHuman->m_pMantua->m_Look.Skin0 == 19)
			m_pKingDomFlag->SetVisible(0);
		else
		{
			char szTemp[256]{};
			m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->m_pMantua->m_Look.Skin0;
			if ((m_pMyHuman->m_sHelmIndex == 3503 || m_pMyHuman->m_sHelmIndex == 3504 || 
				m_pMyHuman->m_sHelmIndex == 3505 || m_pMyHuman->m_sHelmIndex == 3506) && 
				m_pMyHuman->m_pMantua->m_Look.Skin0 == 2)
			{
				m_pKingDomFlag->m_GCPanel.nTextureIndex = 33;
			}

			m_pKingDomFlag->m_GCPanel.nTextureIndex = m_pMyHuman->UnSetCitizenMantle(m_pKingDomFlag->m_GCPanel.nTextureIndex);
			m_pFlagDescText[0]->SetText(g_pItemList[pMobData->Equip[15].sIndex].Name, 0);

			sprintf(szTemp, "%s %d", g_pMessageStringTable[80], BASE_GetItemAbility(&pMobData->Equip[15], 3));
			m_pFlagDescText[1]->SetText(szTemp, 0);

			sprintf(szTemp, "%s %d", g_pMessageStringTable[81], BASE_GetItemAbility(&pMobData->Equip[15], 4));
			m_pFlagDescText[2]->SetText(szTemp, 0);
			m_pKingDomFlag->SetVisible(1);
		}

		if (m_pMyHuman->m_cMount == 1 && 
			(m_pMyHuman->m_nMountSkinMeshType == 31	|| m_pMyHuman->m_nMountSkinMeshType == 40 || 
			 m_pMyHuman->m_nMountSkinMeshType == 20 && m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
		{
			if (m_pBtnMountRun)
			{
				m_pBtnMountRun->m_bEnable = 1;
				m_pBtnMountRun->m_bSelected = g_bRunning;
			}
		}
		else if (m_pBtnMountRun)
		{
			m_pBtnMountRun->m_bEnable = 0;
		}

		SetPosPKRun();
	}
}

void TMFieldScene::UpdateSkillBelt()
{
	auto pSkillBelt2 = m_pGridSkillBelt2;
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem2 = pSkillBelt2->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 248)
		{
			auto pStructItem2 = new STRUCT_ITEM;
			memset(pStructItem2, 0, sizeof(STRUCT_ITEM));

			pStructItem2->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i] + 5295;
			
			auto pSkillItem = new SGridControlItem(0, pStructItem2, 0.0f, 0.0f);

			if (pSkillItem)
				pSkillBelt2->AddSkillItem(pSkillItem, i, 0);

			if (g_pObjectManager->m_cSelectShortSkill == i)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem2)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pReturnItem2);
	}

	auto pSkillBelt3 = m_pGridSkillBelt3;
	for (int i = 0; i < 10; ++i)
	{
		auto pReturnItem3 = pSkillBelt3->PickupItem(i, 0);
		if ((unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 248)
		{
			auto pStructItem3 = new STRUCT_ITEM;
			memset(pStructItem3, 0, sizeof(STRUCT_ITEM));

			pStructItem3->sIndex = (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] < 105 ? (unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5000 :
				(unsigned char)g_pObjectManager->m_cShortSkill[i + 10] + 5295;

			auto pSkillItem = new SGridControlItem(0, pStructItem3, 0.0f, 0.0f);

			if (pSkillItem)
				pSkillBelt3->AddSkillItem(pSkillItem, i, 0);

			if (g_pObjectManager->m_cSelectShortSkill == i + 10)
				pSkillItem->m_GCObj.nTextureSetIndex = 200;
		}
		if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pReturnItem3)
			g_pCursor->m_pAttachedItem = nullptr;

		SAFE_DELETE(pReturnItem3);
	}
}

void TMFieldScene::UpdateMyHuman()
{
	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
	
	SANC_INFO stSancInfo{};
	memcpy(&stSancInfo, &m_pMyHuman->m_stOldSancInfo, sizeof(stSancInfo));

	m_pMyHuman->SetPacketMOBItem(&g_pObjectManager->m_stMobData);

	if ((unsigned char)stSancInfo.Sanc0 > 0 && pMobData->Equip[0].sIndex != 32)
		memcpy(&m_pMyHuman->m_stSancInfo, &stSancInfo, sizeof(stSancInfo));

	float fCon = (float)m_pMyHuman->m_stScore.Con;
	m_pMyHuman->SetCharHeight(fCon);
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);

	int nWeaponTypeL = BASE_GetItemAbility(&pMobData->Equip[6], 21);
	if (nWeaponTypeL == 41)
	{
		m_pMyHuman->m_stLookInfo.RightMesh = m_pMyHuman->m_stLookInfo.LeftMesh;
		m_pMyHuman->m_stLookInfo.RightSkin = m_pMyHuman->m_stLookInfo.LeftSkin;
		m_pMyHuman->m_stSancInfo.Sanc6 = m_pMyHuman->m_stSancInfo.Sanc7;
		m_pMyHuman->m_stSancInfo.Legend6 = m_pMyHuman->m_stSancInfo.Legend7;
	}

	m_pMyHuman->InitObject();
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, m_pMyHuman->m_fAngle, 0.0f);

	m_pMyHuman->CheckAffect();
	SetSanc();
}

void TMFieldScene::SetMyHumanExp(long long unExp, int nFakeExp)
{
	auto pMobData = &g_pObjectManager->m_stMobData;

	long long nExp = unExp - pMobData->Exp;
	int nFExp = g_pObjectManager->m_nFakeExp - nFakeExp;

	if (nExp == 0 && nFExp == 0)
		return;

	pMobData->Exp = unExp;
	g_pObjectManager->m_nFakeExp = nFakeExp;

	memcpy(&m_pMyHuman->m_stScore, &pMobData->CurrentScore, sizeof(pMobData->CurrentScore));

	UpdateScoreUI(0);

	int nTX = 0;
	int nTY = 0;
	if (nExp >= 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		sprintf(szStr, "Exp +%d", static_cast<unsigned int>(nExp));

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 0.5f, 0, 1, 1200, 0, 1));

		sprintf(szStr, g_pMessageStringTable[148], nExp);
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
	if (nFExp > 0 && BASE_Get3DTo2DPos(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y, &nTX, &nTY))
	{
		char szStr[128]{};
		sprintf(szStr, "Cp -%d", nFExp);

		m_pExtraContainer->AddChild(new TMFont3(szStr, nTX, nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f), 0xFFFF8866, 1.0f, 0, 1, 1500, 0, 1));

		sprintf(szStr, g_pMessageStringTable[305], nFExp);
		if (!m_bShowExp)
		{
			if (m_pChatListnotice)
				m_pChatListnotice->AddItem(new SListBoxItem(szStr, 0xFFCCAAFF, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
		}
	}
}

void TMFieldScene::SetAutoOption(int nIndex, char* szString)
{
}

void TMFieldScene::SetAutoSkillNum(int nCount)
{
	m_nAutoSkillNum = nCount;

	if (m_pAutoSkillPanel)
	{
		for (int i = 1; i <= 10; ++i)
		{
			if (i > nCount)
				m_pAutoSkillPanelChild[i - 1]->SetVisible(0);
			else
				m_pAutoSkillPanelChild[i - 1]->SetVisible(1);
		}
	}
}

void TMFieldScene::SetCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	if (pCamera->m_dwSetTime)
		return;

	m_nCameraMode += 1;
	if (m_nCameraMode > 2)
		m_nCameraMode = 0;

	if (m_bIsDungeon == 1 && m_nCameraMode == 1)
		m_nCameraMode = 2;

	if (m_nCameraMode == 0)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 && 
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
		pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 1)
	{
		TMVector2 Pos = g_pObjectManager->m_pCamera->GetFocusedObject()->m_vecPosition;
		if ((int)Pos.x >> 7 > 26 && (int)Pos.x >> 7 < 31 && 
			(int)Pos.y >> 7 > 20 && (int)Pos.y >> 7 < 25)
		{
			pCamera->m_fVerticalAngle = -0.64114136f;
			pCamera->m_fHorizonAngle = 1.013417f;
		}
		else
		{
			pCamera->m_fVerticalAngle = -0.76999825f;
			pCamera->m_fHorizonAngle = 0.80142671f;
		}

		pCamera->m_nQuaterView = 0;
		pCamera->m_fSightLength = 11.0f;
		pCamera->m_fWantLength = 11.0f;
		pCamera->m_fCamHeight = 0.33f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}
	else if (m_nCameraMode == 2)
	{
		pCamera->m_nQuaterView = 0;
		pCamera->m_fVerticalAngle = 0.1f;
		pCamera->m_fSightLength = 3.5f;
		pCamera->m_fWantLength = 3.5f;
		pCamera->m_fCamHeight = 0.33f;
		pCamera->m_fHorizonAngle = 3.1415927f;
		g_pDevice->m_bFog = 1;
		m_pSky->m_bVisible = 0;
		m_bQuater = 0;
	}

	auto pBtnCamera = (SButton*)m_pControlContainer->FindControl(308);
	if (pBtnCamera)
		pBtnCamera->SetSelected(m_bQuater);
}

void TMFieldScene::InitCameraView()
{
	auto pCamera = g_pObjectManager->m_pCamera;
	pCamera->m_nQuaterView = 0;
	pCamera->m_fVerticalAngle = -0.78539819f;
	pCamera->m_fSightLength = pCamera->m_fMaxCamLen;
	pCamera->m_fWantLength = pCamera->m_fMaxCamLen;
	pCamera->m_fCamHeight = 0.33000001f;
	pCamera->m_fHorizonAngle = 0.78539819f;
	g_pDevice->m_bFog = 1;
	m_pSky->m_bVisible = 0;
	m_bQuater = 0;
	m_bTab = 0;
	pCamera->EarthQuake(10);
}

void TMFieldScene::SetPK()
{
	TMFieldScene::m_bPK = TMFieldScene::m_bPK == 0;

	m_PkButton->SetSelected(TMFieldScene::m_bPK == 0);
	m_PkButton->Update();

	auto pBtnPK = (SButton*)m_pControlContainer->FindControl(306);
	MSG_STANDARDPARM stParam{};
	stParam.Header.ID = g_pObjectManager->m_dwCharID;
	stParam.Header.Type = MSG_SetPKMode_Opcode;
	stParam.Parm = TMFieldScene::m_bPK;
	SendOneMessage((char*)&stParam, sizeof(stParam));

	if (pBtnPK)
		pBtnPK->SetSelected(TMFieldScene::m_bPK);
}

void TMFieldScene::SetAutoTarget()
{
	m_cAutoAttack = m_cAutoAttack == 0;

	auto pBtnAuto = static_cast<SButton*>(m_pControlContainer->FindControl(312u));

	if (m_cAutoAttack == 0)
		m_pTargetHuman = nullptr;

	if (pBtnAuto)
		pBtnAuto->SetSelected(m_cAutoAttack);

	m_pAutoSkillPanel->SetVisible(m_cAutoAttack);
}

void TMFieldScene::SetWhisper(char cOn)
{
	m_cWhisper = cOn;

	m_pChatWhisper->m_bSelected = m_pChatWhisper->m_bSelected == 0;
	m_pChatWhisper_C->m_bSelected = m_pChatWhisper->m_bSelected == 0;

	m_pChatWhisper->Update();
	m_pChatWhisper_C->Update();
}

void TMFieldScene::SetPartyChat(char cOn)
{
	m_pChatParty->m_bSelected = m_pChatParty->m_bSelected == 0;
	m_pChatParty_C->m_bSelected = m_pChatParty->m_bSelected == 0;

	m_pChatParty->Update();
	m_pChatParty_C->Update();
}

void TMFieldScene::SetGuildChat(char cOn)
{
	m_pChatGuild->m_bSelected = m_pChatGuild->m_bSelected == 0;
	m_pChatGuild_C->m_bSelected = m_pChatGuild->m_bSelected == 0;

	m_pChatGuild->Update();
	m_pChatGuild_C->Update();
}

void TMFieldScene::SetKingDomChat(char cOn)
{
	m_pKingDomGuild->m_bSelected = m_pKingDomGuild->m_bSelected == 0;
	m_pKingDomGuild->Update();
}

void TMFieldScene::SetMyHumanMagic()
{
	;
}

void TMFieldScene::InitBoard()
{
	if (m_pGMsgPanel)
	{
		SListBoxBoardItem* pBoardItem = new SListBoxBoardItem((char*)"0",
			g_pMessageStringTable[176],
			g_pMessageStringTable[177],
			g_pMessageStringTable[178],
			g_pMessageStringTable[179],
			g_pMessageStringTable[180],
			0x5588FFFFu,
			1);

		pBoardItem->SetControlID(824);
		pBoardItem->SetPos(4.0f, 28.0f);
		m_pGMsgListPanel->AddChild(pBoardItem);

		m_pButtonBox = new SButtonBox(4.0f, 320.0f, 500.0f, 16.0f, 1, 10, 1, 1, 10, 1, 10);
		m_pButtonBox->SetControlID(826);
		m_pButtonBox->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

		m_pGMsgListPanel->AddChild(m_pButtonBox);
		m_pGMsgPanel->SetVisible(0);
	}

	m_pHelpPanel = (SPanel*)m_pControlContainer->FindControl(864);
	if (m_pHelpPanel)
	{
		m_pHelpText = (SText*)m_pControlContainer->FindControl(865);
		m_pHelpButton[0] = (SButton*)m_pControlContainer->FindControl(867);
		m_pHelpList[0] = (SListBox*)m_pControlContainer->FindControl(868);
		m_pHelpButton[1] = (SButton*)m_pControlContainer->FindControl(869);
		m_pHelpList[1] = (SListBox*)m_pControlContainer->FindControl(870);
		m_pHelpButton[2] = (SButton*)m_pControlContainer->FindControl(871);
		m_pHelpList[2] = (SListBox*)m_pControlContainer->FindControl(872);
		m_pHelpButton[3] = (SButton*)m_pControlContainer->FindControl(873);
		m_pHelpList[3] = (SListBox*)m_pControlContainer->FindControl(874);
		m_pHelpMemo = (SButton*)m_pControlContainer->FindControl(875);
		m_pHelpSummon = (SButton*)m_pControlContainer->FindControl(878);

		m_pHelpSummon->m_cAlwaysAlt = 1;
		m_pHelpSummon->m_cBlink = 1;

		m_pHelpSummon->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_pAltText->SetPos(-26.0f * RenderDevice::m_fWidthRatio,
			-17.0f * RenderDevice::m_fHeightRatio);

		m_pHelpSummon->m_cAlwaysAlt = 0;
		m_pHelpMemo->m_nPosX = (float)(665.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpMemo->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(105.0f * RenderDevice::m_fHeightRatio);
		m_pHelpMemo->m_cBlink = 1;
		m_pHelpSummon->m_nPosX = (float)(616.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pHelpSummon->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(90.0f * RenderDevice::m_fHeightRatio);

		LoadMsgText(m_pHelpList[0], (char*)"UI\\interface.txt");
		LoadMsgText(m_pHelpList[1], (char*)"UI\\command.txt");
		LoadMsgText(m_pHelpList[2], (char*)"UI\\etc.txt");

		if (m_pHelpList[1])
			m_pHelpList[1]->SetVisible(0);
		if (m_pHelpList[2])
			m_pHelpList[2]->SetVisible(0);
		if (m_pHelpList[3])
			m_pHelpList[3]->SetVisible(0);
		if (m_pHelpMemo)
			m_pHelpMemo->SetVisible(0);
		if (m_pHelpSummon)
			m_pHelpSummon->SetVisible(0);

		m_pHelpPanel->SetVisible(0);
		m_pHelpPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pHelpPanel->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pHelpPanel->m_nHeight * 0.6f));

		m_pHelpInterface = (SPanel*)m_pControlContainer->FindControl(6067);
		for (int i = 0; i < 3; ++i)
		{
			m_pHelpInterfacePanel[i] = (SPanel*)m_pControlContainer->FindControl(i + 6064);
			m_pHelpInterfaceList[i] = (SListBox*)m_pControlContainer->FindControl(i + 6074);
			if (i > 0)
				m_pHelpInterfacePanel[i]->SetVisible(0);
		}
		LoadMsgText(m_pHelpInterfaceList[0], (char*)"UI\\interface1.txt");
		LoadMsgText(m_pHelpInterfaceList[1], (char*)"UI\\interface2.txt");
		LoadMsgText(m_pHelpInterfaceList[2], (char*)"UI\\interface3.txt");
		m_nCurrInterfacePanelIndex = 0;
	}	

	m_pQuestPanel = (SPanel*)m_pControlContainer->FindControl(1054256);
	if (m_pQuestPanel)
	{
		m_pQuestButton[0] = (SButton*)m_pControlContainer->FindControl(1054259);
		m_pQuestList[0] = (SListBox*)m_pControlContainer->FindControl(1054260);
		m_pQuestContentList[0] = (SListBox*)m_pControlContainer->FindControl(1054261);
		m_pQuestButton[1] = (SButton*)m_pControlContainer->FindControl(1054262);
		m_pQuestList[1] = (SListBox*)m_pControlContainer->FindControl(1054263);
		m_pQuestContentList[1] = (SListBox*)m_pControlContainer->FindControl(1054264);
		m_pQuestButton[2] = (SButton*)m_pControlContainer->FindControl(1054265);
		m_pQuestList[2] = (SListBox*)m_pControlContainer->FindControl(1054266);
		m_pQuestContentList[2] = (SListBox*)m_pControlContainer->FindControl(1054267);
		m_pQuestButton[3] = (SButton*)m_pControlContainer->FindControl(1054268);
		m_pQuestList[3] = (SListBox*)m_pControlContainer->FindControl(1054269);
		m_pQuestContentList[3] = (SListBox*)m_pControlContainer->FindControl(1054271);

		if (m_pQuestList[0])
			m_pQuestList[0]->SetVisible(0);
		if (m_pQuestList[1])
			m_pQuestList[1]->SetVisible(0);
		if (m_pQuestList[2])
			m_pQuestList[2]->SetVisible(0);
		if (m_pQuestList[3])
			m_pQuestList[3]->SetVisible(0);

		if (m_pQuestContentList[0])
			m_pQuestContentList[0]->SetVisible(0);
		if (m_pQuestContentList[1])
			m_pQuestContentList[1]->SetVisible(0);
		if (m_pQuestContentList[2])
			m_pQuestContentList[2]->SetVisible(0);
		if (m_pQuestContentList[3])
			m_pQuestContentList[3]->SetVisible(0);

		m_pQuestPanel->SetVisible(0);

		m_pQuestList[0]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[1]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[2]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
		m_pQuestList[3]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

		m_pQuestPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuestPanel->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pQuestPanel->m_nHeight * 0.6f));

		memset(m_pLevelQuest, 0, sizeof(m_pLevelQuest));

		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects.txt", 97);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects2.txt", 98);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects3.txt", 99);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestSubjects4.txt", 101);
		LoadMsgLevel(m_pLevelQuest, "UI\\QuestMessage.txt", 100);

		m_pQuestMemo = (SButton*)m_pControlContainer->FindControl(1054273);
		m_pQuestMemo->m_cAlwaysAlt = 1;
		m_pQuestMemo->m_cBlink = 1;

		m_pQuestMemo->m_pAltText->SetPos(-20.0f, -10.0f);

		m_pQuestMemo->m_nPosX = (float)(700.0f * RenderDevice::m_fWidthRatio)
			+ m_pMainInfo1->m_nPosX;
		m_pQuestMemo->m_nPosY = m_pMainInfo1->m_nPosY
			- (float)(105.0f * RenderDevice::m_fHeightRatio);

		if (!g_pObjectManager->m_stMobData.CurrentScore.Level)
		{
			const STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
			LoadMsgText3(m_pQuestList[0], 
				(char*)"UI\\QuestSubjects.txt",	
				g_pObjectManager->m_stMobData.CurrentScore.Level + 1, 
				g_pObjectManager->m_stMobData.Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[1],
				(char*)"UI\\QuestSubjects2.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[2],
				(char*)"UI\\QuestSubjects3.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);
			LoadMsgText3(m_pQuestList[3],
				(char*)"UI\\QuestSubjects4.txt",
				pMobData->CurrentScore.Level + 1,
				pMobData->Equip[0].sIndex % 10);

			m_pQuestMemo->SetVisible(1);

			char szStr[128]{};
			unsigned int dwCol = 0xFFAAAAFF;
			if (m_pLevelQuest[pMobData->CurrentScore.Level] == 100)
				dwCol = LoadMsgText4(					
					szStr,
					sizeof szStr,
					(char*)"UI\\QuestMessage.txt",
					pMobData->CurrentScore.Level + 1,
					pMobData->Equip[0].sIndex % 10);
			else
				sprintf(szStr, g_pMessageStringTable[307]);

			SListBoxItem* pChatItem = new SListBoxItem(szStr, dwCol, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);

			if (pChatItem && m_pChatList)
				m_pChatList->AddItem(pChatItem);
		}
		else if (m_pQuestMemo)
		{
			m_pQuestMemo->SetVisible(0);
		}
	}

	m_pPotalPanel = (SPanel*)m_pControlContainer->FindControl(12544);
	m_pPotalList = (SListBox*)m_pControlContainer->FindControl(12545);
	m_pPotalText = (SText*)m_pControlContainer->FindControl(12549);
	m_pPotalText1 = (SText*)m_pControlContainer->FindControl(12550);
	m_pPotalText2 = (SText*)m_pControlContainer->FindControl(12551);
	m_pPotalText3 = (SText*)m_pControlContainer->FindControl(12552);

	m_pPotalPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuestPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pQuestPanel->m_nHeight * 0.6f));

	if (m_pPotalList)
	{
		m_pPotalList->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);
	}

	if (m_pPotalPanel)
		m_pPotalPanel->SetVisible(0);

	for (int j = 0; j < 4; ++j)
	{
		m_pQuestList[j]->SetSize(200.0f, m_pQuestList[j]->m_nHeight);
		m_pQuestContentList[j]->SetSize(300.0f, m_pQuestContentList[j]->m_nHeight);
	}
}

int TMFieldScene::LoadMsgText(SListBox* pListBox, char* szFileName)
{
	FILE* fp = nullptr;
	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;
	if (!pListBox)
		return 0;

	char szCol[7]{};

	char szTemp[256]{};
	char szText[256]{};

	unsigned int dwCol = 0;

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		strncpy(szCol, szTemp, 6);
		sscanf(szCol, "%x", &dwCol);

		auto szRet = strstr(szTemp, "\n");
		if (szRet)
			szRet[0] = 0;

		if (szTemp[6] == ' ')
		{
			sprintf(szText, "%s", &szTemp[6]);
			pListBox->AddItem(new SListBoxItem(szText, dwCol | 0xFF000000, 0.0f, 0.0f, 400.0f, 16.0f, 0, 0x77777777, 1u, 0));
		}
	}
	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

void TMFieldScene::SetPosPKRun()
{
	;
}

void TMFieldScene::SetRunMode()
{
	if (m_pMyHuman->m_cMount == 1 && 
		(m_pMyHuman->m_nMountSkinMeshType == 31 || m_pMyHuman->m_nMountSkinMeshType == 40 || m_pMyHuman->m_nMountSkinMeshType == 20 && 
		 m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
	{
		g_bRunning = g_bRunning == 0;
		UpdateScoreUI(0);
	}
}

void TMFieldScene::UpdateFireWorkButton(int nIndex)
{
	if (nIndex >= 0 && nIndex <= 99 && m_pFireWorkPanel)
	{
		if (m_pFireWorkButton[nIndex])
		{
			m_pFireWorkButton[nIndex]->m_bSelected = m_pFireWorkButton[nIndex]->m_bSelected == 0;
			m_pFireWorkButton[nIndex]->Update();
		}
	}
}

void TMFieldScene::ClearFireWork()
{
	if (m_pFireWorkPanel)
	{
		for (int i = 0; i < 100; ++i)
		{
			m_pFireWorkButton[i]->m_bSelected = 0;
			m_pFireWorkButton[i]->Update();
		}
	}
}

void TMFieldScene::DrawCustomFireWork(int nIndex)
{
	// This func is funny haha xD
}

void TMFieldScene::TotoSelect()
{
}

void TMFieldScene::TotoBuy()
{
}

void TMFieldScene::TotoClose()
{
}

void TMFieldScene::SetQuestStatus(bool bStart)
{
	if (bStart == 1)
		m_dwQuestStartTime = timeGetTime();
	else
		m_dwQuestStartTime = 0;

	m_pQuestRemainTime->SetVisible(bStart);
}

void TMFieldScene::UpdateQuestTime()
{
	if (m_dwQuestStartTime)
	{
		int nLeftSecond = 900 - (timeGetTime() - m_dwQuestStartTime) / 1000;

		char strText[128]{};
		sprintf(strText, "[%02d:%02d]", nLeftSecond / 60, nLeftSecond % 60);

		m_pQuestRemainTime->SetText(strText, 0);
		if (!m_pQuestRemainTime->IsVisible())
			m_pQuestRemainTime->SetVisible(1);
	}
}

void TMFieldScene::SetButtonTextXY(SButton* pButton)
{
	if (m_pSystemPanel && pButton)
	{
		TMVector2 vecXY{};
		auto vec = pButton->GetPos();

		int nLen = strlen(pButton->m_GCPanel.pFont->m_szString);
		if (RenderDevice::m_fWidthRatio == 0.80000001f)
		{
			vecXY.x = 43.0f - ((float)(nLen - 1) * 3.0f);
			vecXY.y = 2.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.28f)
		{
			vecXY.x = 66.0f - ((float)(nLen - 1) * 4.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 1.6f)
		{
			vecXY.x = 75.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}
		else if (RenderDevice::m_fWidthRatio == 2.0f)
		{
			vecXY.x = 90.0f - ((float)(nLen - 1) * 5.0f);
			vecXY.y = 5.0f;
		}

		pButton->m_GCPanel.pFont->m_nPosX = (int)((m_pSystemPanel->m_nPosX + vec.x) + vecXY.x);
		pButton->m_GCPanel.pFont->m_nPosY = (int)((m_pSystemPanel->m_nPosY + vec.y) + vecXY.y);
	}
}

void TMFieldScene::SysMsgChat(char* str)
{
	auto pEdit = m_pEditChat;
	auto pChatList = m_pChatListnotice;

	pChatList->AddItem(new SListBoxItem(str, 0xFFFFAAAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0));
	pEdit->SetText((char*)"");

	m_pControlContainer->SetFocusedControl(nullptr);
	if (g_pObjectManager->m_bTvControl != 1)
		pEdit->SetText((char*)"");
}

void TMFieldScene::GetTimeString(char* szVal, int sTime, int nTime, int i)
{
	if (nTime >= 1000000)
	{
		int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
		int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
		int nYearDay = 365;
		if (!(Year % 4))
			nYearDay = 366;

		int nDafaultDay = 0;
		switch (add)
		{
		case 4:
			nDafaultDay = 7;
			break;
		case 5:
			nDafaultDay = 15;
			break;
		case 6:
			nDafaultDay = 30;
			break;
		}

		if (add)
			sprintf(szVal, g_pMessageStringTable[291], nDafaultDay - nYearDay * (m_nYear - Year) - (m_nDays - m_pMyHuman->m_stAffect[i].Time % 10000));
	}
	else if (sTime > 86400)
		sprintf(szVal, g_pMessageStringTable[291], sTime / 86400);
	else if (sTime > 3600)
		sprintf(szVal, g_pMessageStringTable[292], sTime / 3600);
	else if (sTime <= 600)
		sprintf(szVal, "%5d", sTime);
	else
		sprintf(szVal, g_pMessageStringTable[293], sTime / 60);
}

int TMFieldScene::StrByteCheck(char* szString)
{
	int value = 0;
	int byteCheck = 0;
	for (size_t i = 0; i < strlen(szString); ++i)
	{
		if (szString[i] >= 'A' && szString[i] <= 'z')
			++value;
		else if (byteCheck == 1)
		{
			++value;
			byteCheck = 0;
		}
		else
			byteCheck = 1;
	}

	return value;
}

void TMFieldScene::NewCCMode()
{
	;
}

void TMFieldScene::InsertInChatList(SListBox* pChatList, STRUCT_MOB *pMobData, SEditableText* pEditChat, unsigned int dwColor, int colorId, unsigned int startId)
{
	MSG_MessageWhisper stMsgWhisper{};
	stMsgWhisper.Header.ID = g_pObjectManager->m_dwCharID;
	stMsgWhisper.Header.Type = MSG_MessageWhisper_Opcode;
	stMsgWhisper.Color = colorId;

	sprintf(stMsgWhisper.MobName, "");
	sprintf(stMsgWhisper.String, "%s", pEditChat->GetText());
	BASE_TransCurse(stMsgWhisper.String);

	pEditChat->SetText((char*)"");

	SendOneMessage((char*)&stMsgWhisper, sizeof(stMsgWhisper));

	int len = strlen(stMsgWhisper.String) + strlen(pMobData->MobName);
	const size_t maxLen = 40;
	if (len <= maxLen)
	{
		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", pMobData->MobName, &stMsgWhisper.String[startId]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
	else
	{
		char dest[128]{};
		char dest2[128]{};
		if (IsClearString(stMsgWhisper.String, maxLen - 1))
		{
			strncpy(dest, stMsgWhisper.String, maxLen);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen]);
		}
		else
		{
			strncpy(dest, stMsgWhisper.String, maxLen - 1);
			sprintf(dest2, "%s", &stMsgWhisper.String[maxLen - 1]);
		}

		char istrText[128]{};
		sprintf(istrText, "[%s]> %s", g_pObjectManager->m_stMobData.MobName, &dest[maxLen]);

		auto ipNewItem = new SListBoxItem(istrText, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);

		auto ipNewItem2 = new SListBoxItem(dest2, dwColor, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (strlen(stMsgWhisper.String) > maxLen && ipNewItem && pChatList)
			pChatList->AddItem(ipNewItem);
	}
}
