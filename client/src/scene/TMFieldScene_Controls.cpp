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

/// @file TMFieldScene_Controls.cpp
/// @brief Handlers dos controles de UI do campo extraídos de `TMFieldScene::OnControlEvent`.
///
/// Fase 3: cada bloco `if (idwControlID == ...) { ...; return X; }` virou um método
/// `OnControl_*`, chamado como `return OnControl_*(...)`. Os corpos foram movidos sem
/// alteração; as locais de OnControlEvent que eles liam (`dwServerTime`, `pMobData`,
/// `cktrans`, constantes após o início da função) são passadas por valor com o mesmo nome.

/// Seleção de item na loja do Hell Gate (`TMB_HELLSTORE_SELECT_1..4`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_HellStoreSelect(unsigned int idwControlID, unsigned int idwEvent)
{
	switch (idwControlID)
	{
	case TMB_HELLSTORE_SELECT_1:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 21, 40);
		break;
	case TMB_HELLSTORE_SELECT_2:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 41, 60);
		break;
	case TMB_HELLSTORE_SELECT_3:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 61, 80);
		break;
	case TMB_HELLSTORE_SELECT_4:
		if (m_pHellStoreDesc)
			TMScene::LoadMsgText2(m_pHellStoreDesc, (char*)"UI\\hellStoredesc.txt", 81, 100);
		break;
	}

	m_nHellStoreValue = idwControlID;
	auto pHellgateStore = m_pHellgateStore;
	SButton* pHellSelect = nullptr;
	for (int i = 0; i < 4; ++i)
	{
		pHellSelect = (SButton*)m_pControlContainer->FindControl(i + TMB_HELLSTORE_SELECT_1);
		if (pHellSelect)
		{
			if (i + 6193 == idwControlID)
				pHellSelect->SetSelected(pHellgateStore->m_bVisible);
			else
				pHellSelect->SetSelected(0);
		}
	}
	return 1;
}

/// Setas de aposta do Gamble (`TMB_GAMBLE_LEFTX2..TMB_GAMBLE_RIGHT`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_GambleArrows(unsigned int idwControlID, unsigned int idwEvent)
{
	switch (idwControlID)
	{
	case TMB_GAMBLE_LEFTX2:
		m_nBet -= 10000;
		if (m_nBet < 1000)
			m_nBet = 1000;
		break;
	case TMB_GAMBLE_LEFT:
		m_nBet -= 1000;
		if (m_nBet < 1000)
			m_nBet = 1000;
		break;
	case TMB_GAMBLE_RIGHTX2:
		if (g_pObjectManager->m_stMobData.Coin - m_nBet < 10000)
			return 1;
		m_nBet += 10000;
		if (m_nBet > 100000)
			m_nBet = 100000;
		break;
	case TMB_GAMBLE_RIGHT:
		if (g_pObjectManager->m_stMobData.Coin - m_nBet < 1000)
			return 1;
		m_nBet += 1000;
		if (m_nBet > 100000)
			m_nBet = 100000;
		break;
	}

	auto pText = static_cast<SText*>(m_pControlContainer->FindControl(TMT_GAMBLE_BETCOUNT));
	
	char szText[128]{};
	sprintf(szText, "%6d", m_nBet);
	pText->SetText(szText, 0);

	sprintf(szText, "%10d", g_pObjectManager->m_stMobData.Coin - m_nBet);

	m_pMoney3->m_cComma = 1;
	m_pMoney3->SetText(szText, 0);
	return 1;
}

/// Início da aposta do Gamble (`TMB_GAMBLE_START`).
/// @return Valor devolvido por OnControlEvent para este controle.
int TMFieldScene::OnControl_GambleStart(unsigned int idwControlID, unsigned int idwEvent, STRUCT_MOB* pMobData)
{
	if (m_pReelPanel2->m_dwStopTime || m_pReelPanel2->m_bRoling == 1)
		return 1;
	if (m_pReelPanel->m_dwStopTime || m_pReelPanel->m_bRoling == 1)
		return 1;

	auto pGridInv = m_pGridInv;
	auto vec = pGridInv->CanAddItemInEmpty(1, 1);
	if (vec.x < 0 || vec.y < 0)
	{
		auto pChatList = m_pChatList;

		auto ipNewItem = new SListBoxItem(g_pMessageStringTable[1],
			0xFFFFAAAA,
			0.0,
			0.0,
			300.0f,
			16.0f,
			0,
			0x77777777,
			1u,
			0);

		if (ipNewItem)
			pChatList->AddItem(ipNewItem);

		GetSoundAndPlay(33, 0, 0);
		return 1;
	}

	if (pMobData->Coin - m_nBet >= 0)
	{
		MSG_STANDARDPARM2 stParm2{};
		stParm2.Header.ID = g_pObjectManager->m_dwCharID;
		stParm2.Header.Type = MSG_DoJackpotBet_Opcode;
		stParm2.Parm1 = 1;
		stParm2.Parm2 = m_nBet;
		SendOneMessage((char*)&stParm2, sizeof(stParm2));

		m_pReelPanel->m_dwBatCoin = m_nBet;
		m_pReelPanel2->m_dwBatCoin = m_nBet;

		m_pReelPanel->SetRoll(1, 0, 0, 0, 3000);
		m_pReelPanel2->SetRoll(1, 0, 0, 0, 3000);
		pMobData->Coin -= m_nBet;

		GetSoundAndPlay(338, 0, 0);

		UpdateScoreUI(0);
		return 1;
	}
	return 1;
}
