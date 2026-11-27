#include "core/pch.h"
#include "ui/SControl.h"
#include "entities/TMItem.h"
#include "scene/TMScene.h"
#include "skills/TMSkillMagicShield.h"
#include "effects/TMEffectBillBoard.h"
#include "effects/TMEffectBillBoard2.h"
#include "effects/TMEffectCharge.h"
#include "effects/TMEffectMesh.h"
#include "effects/TMEffectMeshRotate.h"
#include "effects/TMEffectSkinMesh.h"
#include "entities/TMButterFly.h"
#include "entities/TMShade.h"
#include "entities/TMHuman.h"
#include "core/TMGlobal.h"
#include "ui/SControlContainer.h"
#include "effects/TMEffectSWSwing.h"
#include "scene/ObjectManager.h"
#include "render/TMCamera.h"
#include "render/TMMesh.h"
#include "world/TMGround.h"
#include "scene/TMFieldScene.h"
#include "effects/TMEffectParticle.h"
#include "core/TMLog.h"
#include "core/TMUtil.h"
#include "world/TMSky.h"
#include "scene/TMObjectContainer.h"
#include "render/TMLight.h"
#include "skills/TMSkillExplosion2.h"
#include "ui/SGrid.h"
#include "effects/TMEffectStart.h"
#include "skills/TMSkillTownPortal.h"
#include "effects/TMFireEffect.h"
#include "effects/TMEffectSpark.h"
#include "effects/TMEffectLevelUp.h"
#include "common/ItemEffect.h"
#include "render/TMFont3.h"
#include "skills/TMSkillJudgement.h"
#include "skills/TMSkillSlowSlash.h"
#include "skills/TMSkillHaste.h"
#include "skills/TMSkillSpeedUp.h"
#include "skills/TMSkillDoubleSwing.h"
#include "skills/TMSkillFreezeBlade.h"
#include "skills/TMSkillBash.h"
#include "skills/TMSkillMagicArrow.h"
#include "skills/TMSkillCure.h"
#include "skills/TMSkillHeal.h"
#include "skills/TMSkillFire.h"
#include "skills/TMSkillIceSpear.h"
#include "skills/TMSkillPoison.h"
#include "skills/TMSkillSpChange.h"
#include "entities/TMArrow.h"
#include "skills/TMSkillThunderBolt.h"
#include "effects/TMEffectDust.h"
#include "effects/TMEffectFirework.h"

/// @file TMHuman_Packets.cpp
/// @brief Handlers de pacotes de rede (OnPacket*) e montagem de equipamentos/cores recebidos do servidor.

/// Despacha pacotes de entidade recebidos para os handlers OnPacket* de TMHuman.
int TMHuman::OnPacketEvent(unsigned int dwCode, char* buf)
{
    if (m_dwDelayDel)
        return 0;

    if (buf == nullptr)
        return 0;

    MSG_STANDARD* pStandard = (MSG_STANDARD*)buf;

    if (pStandard->ID != m_dwID)
        return 0;

    if (pStandard->Type == MSG_Action_Opcode || pStandard->Type == MSG_Action_Stop_Opcode || pStandard->Type == MSG_Action2_Opcode)
    {
        MSG_Action* pAction = (MSG_Action*)buf;
        if (pAction->TargetX < 0 || pAction->TargetX > 5000 ||
            pAction->TargetY < 0 || pAction->TargetY > 5000)
        {
            LOG_WRITELOG("\nError Position [X:%d Y:%d] MSG Type : 0x%X\n", pAction->TargetX, pAction->TargetY, pAction->Header.Type);
        }

        if (pAction->Effect == 0 || pAction->Effect == 2)
            return OnPacketMove(pAction);
        else if (pAction->Effect == 7)
            return OnPacketChaosCube(pAction);
        else if (pAction->Effect >= 1)
            return OnPacketIllusion(pStandard);
        else
            return 1;
    }

    switch (pStandard->Type)
    {
    case 0x3CA:
        return OnPacketPremiumFireWork((MSG_STANDARD*)buf);
        break;
    case MSG_Motion_Opcode:
        return OnPacketFireWork(reinterpret_cast<MSG_Motion*>(buf));
        break;
    case MSG_RemoveMob_Opcode:
        return OnPacketRemoveMob((MSG_STANDARD*)buf);
        break;
    case MSG_SendItem_Opcode:
        return OnPacketSendItem((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateEquip_Opcode:
        return OnPacketUpdateEquip((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateAffect_Opcode:
        return OnPacketUpdateAffect((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateScore_Opcode:
        return OnPacketUpdateScore((MSG_STANDARD*)buf);
        break;
    case MSG_SetHpMp_Opcode:
        return OnPacketSetHpMp(reinterpret_cast<MSG_SetHpMp*>(buf));
        break;
    case MSG_SetHpDam_Opcode:
        return OnPacketSetHpDam((MSG_STANDARD*)buf);
        break;
    case MSG_MessageChat_Opcode:
        return OnPacketMessageChat((MSG_STANDARD*)buf);
        break;
    case 0x105:
        return OnPacketMessageChat_Index((MSG_STANDARD*)buf);
        break;
    case 0x106:
        return OnPacketMessageChat_Param((MSG_STANDARD*)buf);
        break;
    case MSG_MessageWhisper_Opcode:
        return OnPacketMessageWhisper(reinterpret_cast<MSG_MessageWhisper*>(buf));
        break;
    case MSG_UpdateEtc_Opcode:
        return OnPacketUpdateEtc((MSG_STANDARD*)buf);
        break;
    case 0x3AF:
        return OnPacketUpdateCoin(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case 0x1CF:
        return OnPacketUpdateRMB(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case MSG_Trade_Opcode:
        return OnPacketTrade(reinterpret_cast<MSG_Trade*>(buf));
        break;
    case MSG_CloseTrade_Opcode:
        return OnPacketQuitTrade((MSG_STANDARD*)buf);
        break;
    case MSG_UpdateCarry_Opcode:
        return OnPacketCarry(reinterpret_cast<MSG_Carry*>(buf));
        break;
    case MSG_CNFCheck_Opcode:
        return OnPacketCNFCheck((MSG_STANDARD*)buf);
        break;
    case MSG_SetClan_Opcode:
        return OnPacketSetClan(reinterpret_cast<MSG_STANDARDPARM*>(buf));
        break;
    case MSG_ReqRanking_Opcode:
        return OnPacketReqRanking(reinterpret_cast<MSG_STANDARDPARM2*>(buf));
        break;
    case MSG_SendCastleState2_Opcode:
        return OnPacketVisualEffect((MSG_STANDARD*)buf);
        break;
    default:
        break;    
    }

    return 0;
}

int TMHuman::OnPacketMove(MSG_Action* pAction)
{
    if (pAction == nullptr)
        return 0;

    // NOTE: there's a strange code in the beginnig, that not make sense...
    // and is not used aparently.

    if (m_cDie == 1)
        return 1;
    if (pAction->Effect == 2)
        m_bSliding = 1;

    m_fMaxSpeed = (float)pAction->Speed;
    if (pAction->Speed < 1)
        m_fMaxSpeed = 1.0f;

    char szBuffer[48]{};

    int nStartRouteIndex = m_nLastRouteIndex;
    if (m_fProgressRate > 0.5f)
        nStartRouteIndex = m_nLastRouteIndex + 1;

    int nX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
    int nY = (int)m_vecRouteBuffer[nStartRouteIndex].y;

    if (strlen(pAction->Route) == 0)
        m_cOnlyMove = 1;

    int tX = 0;
    int tY = 0;

    if (nX == pAction->PosX && nY == pAction->PosY && m_cOnlyMove != 1)
    {
        memcpy(m_cRouteBuffer, pAction->Route, 24);

        m_vecTargetPos.x = pAction->TargetX;
        m_vecTargetPos.y = pAction->TargetY;

        m_cSameHeight = 0;
        GenerateRouteTable(pAction->PosX, pAction->PosY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
    }
    else
    {
        if (std::abs(pAction->PosX - nX) > 33 || std::abs(pAction->PosY - nY) > 33)
        {
            nX = pAction->PosX;
            nY = pAction->PosY;
        }

        int tX = pAction->TargetX;
        int tY = pAction->TargetY;

        int bRoute = 0;
        char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
        BASE_GetRoute(nX, nY, &tX, &tY, szBuffer, 12, pHeightMapData, 8);

        if (strlen(szBuffer) == 0)
            return 1;

        if (tX == pAction->TargetX && tY == pAction->TargetY)
        {
            memcpy(m_cRouteBuffer, szBuffer, 48);
            m_vecTargetPos.x = pAction->TargetX;
            m_vecTargetPos.y = pAction->TargetY;
            m_cSameHeight = 0;

            GenerateRouteTable(nX, nY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
        }
        else if (g_pCurrentScene)
        {
            if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && g_pCurrentScene->m_pMyHuman != this)
            {
                int nStartX = nX;
                int nStartY = nY;
                nX = pAction->PosX;
                nY = pAction->PosY;
                tX = pAction->TargetX;
                tY = pAction->TargetY;
                char* pHeight = (char*)g_pCurrentScene->m_HeightMapData;
                BASE_GetRoute(nX, nY, &tX, &tY, szBuffer, 12, pHeight, 8);

                if (strlen(szBuffer))
                {
                    if (tX == pAction->TargetX && tY == pAction->TargetY)
                    {
                        memcpy(m_cRouteBuffer, szBuffer, 48);

                        m_vecTargetPos.x = pAction->TargetX;
                        m_vecTargetPos.y = pAction->TargetY;
                        m_cSameHeight = 0;

                        GenerateRouteTable(nX, nY, m_cRouteBuffer, m_vecRouteBuffer, &m_nMaxRouteIndex);
                        ChangeRouteBuffer(nStartX, nStartY, m_vecRouteBuffer, &m_nMaxRouteIndex);
                    }
                }
            }
        }
    }

    if (m_cOnlyMove)
    {
        if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this)
        {
            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
            pFScene->m_stMoveStop.LastX = pAction->PosX;
            pFScene->m_stMoveStop.LastY = pAction->PosY;
            pFScene->m_stMoveStop.NextX = pAction->TargetX;
            pFScene->m_stMoveStop.NextY = pAction->TargetY;
        }
    }

    MoveTo(m_vecRouteBuffer[1]);
    m_fMoveToAngle = m_fAngle;
    m_dwStartMoveTime = g_pTimerManager->GetServerTime();

    return 1;
}

int TMHuman::OnPacketChaosCube(MSG_Action* pAction)
{
    m_cSameHeight = 1;
    m_vecStartPos.x = (int)m_vecPosition.x;
    m_vecStartPos.y = (int)m_vecPosition.y;
    m_vecTargetPos.x = pAction->TargetX;
    m_vecTargetPos.y = pAction->TargetY;

    for (int i = 0; i < 48; ++i)
    {
        m_vecRouteBuffer[i].x = (float)pAction->TargetX + 0.5f;
        m_vecRouteBuffer[i].y = (float)pAction->TargetY + 0.5f;
    }

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            pScene->m_vecMyNext.x = pAction->TargetX;
            pScene->m_vecMyNext.y = pAction->TargetY;
            pScene->m_stMoveStop.NextX = pAction->TargetX;
            pScene->m_stMoveStop.NextY = pAction->TargetY;
        }

        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }

    m_dwStartMoveTime = g_pTimerManager->GetServerTime();
    return 1;

}

int TMHuman::OnPacketIllusion(MSG_STANDARD* pStd)
{
    auto pAction = (MSG_Action*)pStd;
    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (!m_cHide && pStd->Type == MSG_Action2_Opcode)
    {
        g_pObjectManager->m_stMobData.CurrentScore.Mp -= g_pSpell[73].ManaSpent;
        bool bExpand = false;
        if (m_nClass == 4 || m_nClass == 8)
            bExpand = true;

        auto pEffect = new TMEffectSkinMesh(m_nSkinMeshType, TMVector3(0.0f, 0.0f, 0.0f), TMVector3(0.0f, 0.0f, 0.0f), 0, nullptr);

        if (m_cMount > 0 && m_pMount)
        {
            pEffect->m_nSkinMeshType = m_nMountSkinMeshType;
            memcpy(&pEffect->m_stLookInfo, &m_stMountLook, sizeof(m_stMountLook));
            pEffect->m_nSkinMeshType2 = m_nSkinMeshType;
            memcpy(&pEffect->m_stLookInfo2, &m_stLookInfo, sizeof(m_stLookInfo));
        }
        else
        {
            memcpy(&pEffect->m_stLookInfo, &m_stLookInfo, sizeof(m_stLookInfo));
        }

        pEffect->m_StartColor.r = 1.0f;
        pEffect->m_StartColor.g = 1.0f;
        pEffect->m_StartColor.b = 1.0f;
        pEffect->m_fScale = m_fScale;
        pEffect->InitObject(bExpand);
        pEffect->m_nFade = 1;
        pEffect->m_dwLifeTime = 3000;
        pEffect->InitPosition(m_vecPosition.x, m_fHeight + 0.1f, m_vecPosition.y);

        if (m_cMount > 0 && m_pMount)
        {
            if (pEffect->m_pSkinMesh)
                pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)m_eMotion]]);
            if (pEffect->m_pSkinMesh2)
                pEffect->m_pSkinMesh2->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion + 28]]);
        }
        else
        {
            pEffect->m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)m_eMotion]]);
        }

        pEffect->m_fStartAngle = m_fAngle;
        pEffect->m_fAngle = pEffect->m_fStartAngle;
        if (m_nWeaponTypeL != 101)
            pEffect->m_fAngle = m_fAngle + D3DXToRadian(360);
        pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pEffect->m_nMotionType = 0;

        g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
    }
    else if (!m_cHide && pAction->Effect != 5)
    {
        int nType = 1;
        if (pAction->Effect == 6)
            nType = 2;

        auto pPortal = new TMSkillTownPortal(TMVector3(m_vecPosition.x,
            m_fHeight + 0.050000001f, m_vecPosition.y), nType);

        g_pCurrentScene->m_pEffectContainer->AddChild(pPortal);
        _dwAttackDelay = g_pTimerManager->GetRealTime();
    }
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pCamera = g_pObjectManager->m_pCamera;
        bool bDestIsDungeon = false;
        bool bNowIsDungeon = false;

        if ((int)pAction->TargetY >> 7 > 25)
        {
            bDestIsDungeon = true;
            pScene->m_bIsDungeon = 1;
        }
        else
            pScene->m_bIsDungeon = 0;

        if ((int)m_vecPosition.y >> 7 > 25)
            bNowIsDungeon = true;

        if (bDestIsDungeon == 1 && !bNowIsDungeon)
        {
            pCamera->m_fMaxCamLen = 11.0f;
            if (pCamera->m_fSightLength > 11.0f)
                pCamera->m_fSightLength = 11.0f;
        }
        else if (bNowIsDungeon == 1 && !bDestIsDungeon)
        {
            pCamera->m_fMaxCamLen = 15.0f;
            pCamera->m_fSightLength = pCamera->m_fSightLength + 4.0f;
            if (pCamera->m_fSightLength > 15.0f)
                pCamera->m_fSightLength = 15.0f;
        }
        if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            int nNowX = (int)m_vecPosition.x;
            int nNowY = (int)m_vecPosition.y;
            bool bQuestEnd = false;
            bool bQuestStart = false;

            bQuestStart = nNowX >> 7 == 25
                && nNowY >> 7 == 13
                && (int)pAction->TargetX >> 7 >= 26
                && (int)pAction->TargetX >> 7 <= 30
                && (int)pAction->TargetY >> 7 >= 8
                && (int)pAction->TargetY >> 7 <= 12;

            if (bQuestStart)
                pScene->SetQuestStatus(1);
            else if (bQuestEnd == 1)
                pScene->SetQuestStatus(0);

            pScene->m_vecMyNext.x = pAction->TargetX;
            pScene->m_vecMyNext.y = pAction->TargetY;
            pScene->m_stMoveStop.NextX = pAction->TargetX;
            pScene->m_stMoveStop.NextY = pAction->TargetY;
        }
        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }

    TMVector2 vecPosition{ (float)pAction->TargetX + 0.5f, (float)pAction->TargetY + 0.5f };

    InitPosition(vecPosition.x, static_cast<float>(pScene->GroundGetMask(vecPosition)), vecPosition.y);

    if (pScene->m_pMyHuman != this)
    {
        m_LastSendTargetPos.x = pAction->TargetX;
        m_LastSendTargetPos.y = pAction->TargetY;
    }
    else if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        pScene->m_vecMyNext.x = pAction->TargetX;
        pScene->m_vecMyNext.y = pAction->TargetY;
        pScene->m_stMoveStop.NextX = pAction->TargetX;
        pScene->m_stMoveStop.NextY = pAction->TargetY;
    }

    m_cDie = 0;
    if (!m_cHide && pAction->Effect != 5 && pStd->Type != 872)
    {
        if (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26)
        {
            auto pEffect = new TMEffectStart(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                1, nullptr);

            pScene->m_pEffectContainer->AddChild(pEffect);
        }
        else
        {
            auto pEffect = new TMEffectStart(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                0, nullptr);

            pScene->m_pEffectContainer->AddChild(pEffect);
        }

        if (g_pObjectManager->m_pCamera->m_pFocusedObject == this)
            GetSoundAndPlay(151, 0, 0);

        int nType = 1;
        if (pAction->Effect == 6)
            nType = 2;

        if (pAction->Effect == 6 && m_nClass == 62 && m_stLookInfo.FaceMesh == 2)
        {
            InitPosition(m_vecPosition.x, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) - 2.0f, m_vecPosition.y);
            TMVector3 vecPos{ m_vecPosition.x, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.2f, m_vecPosition.y };

            auto pJudgement = new TMSkillJudgement(vecPos, 4, 0.1f);

            pScene->m_pEffectContainer->AddChild(pJudgement);
        }
        else
        {
            auto pPartal = new TMSkillTownPortal(
                TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                nType);

            pScene->m_pEffectContainer->AddChild(pPartal);
        }
    }

    if (pScene->m_pMyHuman == this)
    {
        auto pCamera = g_pObjectManager->m_pCamera;
        auto fHAngle = pCamera->m_fHorizonAngle;
        
        pScene->Warp();
        if (pStd->Type == MSG_Action2_Opcode)
            pCamera->m_fHorizonAngle = fHAngle;
        if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
            pScene->m_bLastMyAttr = BASE_GetAttr(pAction->TargetX, pAction->TargetY);

        InitPosition(vecPosition.x,
            (float)pScene->GroundGetMask(vecPosition) * 0.1f,
            vecPosition.y);
    }
    if (pStd->Type == MSG_Action2_Opcode)
    {
        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 0);
        if (g_pObjectManager->m_pCamera->m_pFocusedObject == this)
            GetSoundAndPlay(175, 0, 0);
    }
    else
    {
        SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);
        if (!m_cHide)
        {
            auto pLevelUp = new TMEffectLevelUp(TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y), 0);
            pScene->m_pEffectContainer->AddChild(pLevelUp);

            if (!m_cSummons)
            {
                auto pPortal = new TMSkillTownPortal(
                    TMVector3((float)pAction->TargetX + 0.5f, ((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, (float)pAction->TargetY + 0.5f),
                    1);

                pScene->m_pEffectContainer->AddChild(pPortal);
            }
        }
    }

    m_bIgnoreHeight = 0;    
    if (pScene && pScene->m_pMyHuman == this && pScene->m_bAirMove == 1)
        pScene->AirMove_End();

    return 1;
}

int TMHuman::OnPacketFireWork(MSG_Motion* pStd)
{
	if (pStd->Motion == 100)
	{
		auto pFireWork = new TMEffectFireWork({ m_vecPosition.x, m_fHeight + 5.0f, m_vecPosition.y }, pStd->Parm);
		if (g_pCurrentScene->m_pEffectContainer)
			g_pCurrentScene->m_pEffectContainer->AddChild(pFireWork);
		return 1;
	}
	else if (pStd->Parm == 1)
	{
		if (m_nClass == 36 || m_nClass == 37)
			return 1;
		if (m_nClass == 39)
		{
			if (!(rand() % 2))
			{
				pStd->Motion = 16;
				if (m_bVisible == 1)
					GetSoundAndPlay(267, 0, 0);
			}

			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			return 1;
		}
		if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
		{
			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			return 1;
		}
		if (m_nSkinMeshType == 0 || m_nSkinMeshType == 1 || m_nSkinMeshType == 21 || m_nSkinMeshType == 3)
		{
			SetMotion((ECHAR_MOTION)pStd->Motion, pStd->Direction);
			if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15 && m_bVisible == 1)
				GetSoundAndPlay(300, 0, 0);

			return 1;
		}

		return 1;
	}

	if (pStd->Parm == 2)
		m_cDie = 0;
	if (pStd->Parm == 3)
	{
		auto pLevelUp = new TMEffectLevelUp({ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 0);
		if (g_pCurrentScene->m_pEffectContainer)
			g_pCurrentScene->m_pEffectContainer->AddChild(pLevelUp);
	}
	if (pStd->Motion < 256)
	{
		if (g_pObjectManager->m_dwCharID == pStd->Header.ID)
			m_SendeMotion = ECHAR_MOTION::ECMOTION_NONE;

		auto eMotion = pStd->Motion;
		if ((m_dwID > 0 && m_dwID < 1000) &&
			(m_nSkinMeshType == 3 || m_nSkinMeshType == 8 || m_nSkinMeshType == 7 || m_nSkinMeshType == 25 || m_nSkinMeshType == 28) &&
			((int)eMotion > 14 && (int)eMotion < 25 || eMotion == 13))
		{
			eMotion -= 14;
		}

		SetMotion((ECHAR_MOTION)eMotion, pStd->Direction);
	}

	return 1;
}

int TMHuman::OnPacketPremiumFireWork(MSG_STANDARD* pStd)
{
	return 0;
}

int TMHuman::OnPacketRemoveMob(MSG_STANDARD* pStd)
{
    auto pRemoveMob = (MSG_RemoveMob*)pStd;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        if (m_stScore.Hp <= 0 && !m_sFamCount)
            Die();

        return 1;
    }

    auto pScene = g_pCurrentScene;
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene && pFScene->m_pPGTPanel->IsVisible() == 1 && pFScene->m_pPGTOver && m_dwID == pFScene->m_pPGTOver->m_dwID)
        pFScene->m_pPGTPanel->SetVisible(0);

    if (pFScene && pFScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        if (m_dwID == pFScene->m_pGridShop->m_dwMerchantID && pFScene->m_pShopPanel->m_bVisible == 1)
            pFScene->SetVisibleShop(0);
        if (m_dwID == pFScene->m_dwHellStoreID && pFScene->m_pHellgateStore->m_bVisible == 1)
            pFScene->SetVisibleHellGateStore(0);

        if (m_dwID == pFScene->m_stAutoTrade.TargetID)
        {
            auto pPanel = (SPanel*)pFScene->m_pControlContainer->FindControl(646);
            if (pPanel)
            {
                if (pPanel->IsVisible() == 1)
                    pFScene->SetVisibleAutoTrade(0, 0);
            }
        }

        auto pPartyList = pFScene->m_pPartyList;
        if (pPartyList && pPartyList->m_nNumItem < 2 && m_bParty == 1)
        {
            pPartyList->Empty();
            m_bParty = 0;

            SAFE_DELETE(m_pInMiniMap);
            pFScene->m_pMyHuman->m_bParty = 0;
        }
        else if (pPartyList)
        {
            for (int i = 0; i < pPartyList->m_nNumItem; ++i)
            {
                auto pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[i];
                if (pPartyItem->m_dwCharID == m_dwID)
                {
                    auto pNode = g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
                    if (pNode)
                    {
                        pNode->m_bParty = 0;
                        SAFE_DELETE(m_pInMiniMap);
                    }
                    if (pPartyItem->m_nState == 2)
                    {
                        pPartyItem->m_nState = 4;
                        pPartyItem->m_GCText.dwColor = 0xFF777777;
                        pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
                    }
                    else if (!pPartyItem->m_nState)
                    {
                        pPartyItem->m_nState = 3;
                        pPartyItem->m_GCText.dwColor = 0xFF777777;
                        pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
                    }
                    break;
                }
            }
        }
    }
    if (m_nWillDie == -1)
    {
        if (!pRemoveMob->RemoveType)
        {
            if (g_pCurrentScene->m_pMyHuman != this)
                m_nWillDie = pRemoveMob->RemoveType;

            return 1;
        }
        if (pRemoveMob->RemoveType == 1 || m_nSkinMeshType == 37)
        {
            unsigned int dwServerTime = g_pTimerManager->GetServerTime();
            m_nWillDie = pRemoveMob->RemoveType;
            if (!m_dwDeadTime)
                m_dwDeadTime = dwServerTime;
            m_stScore.Hp = 0;
            Die();
            return 1;
        }
        if (pRemoveMob->RemoveType == 2)
        {
            m_nWillDie = pRemoveMob->RemoveType;

            TMVector3 vecStart{ m_vecPosition.x, (float)((float)pScene->GroundGetMask(m_vecPosition) * 0.1f) + 0.05f, m_vecPosition.y };
            if (!m_cHide)
            {
                if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26)
                {
                    auto pEffect = new TMEffectStart(vecStart, 0, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);

                    auto pEffect2 = new TMEffectBillBoard2(1, 2000, 0.5f, 0.5f, 0.5f, 0.002f, 0);
                    pEffect2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect2->m_vecPosition = vecStart;
                    pScene->m_pEffectContainer->AddChild(pEffect2);
                }
                else if (m_nSkinMeshType == 21 || m_nSkinMeshType == 22 || m_nSkinMeshType == 23 || 
                    m_nSkinMeshType == 24 || m_nSkinMeshType == 2 || m_nSkinMeshType == 3 || m_nSkinMeshType == 4)
                {
                    auto pEffect = new TMEffectStart(vecStart, 1, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);
                }
                else if (m_nSkinMeshType == 20)
                {
                    auto pEffect = new TMEffectStart(vecStart, 1, 0);
                    pScene->m_pEffectContainer->AddChild(pEffect);
                }
            }

            if (!m_dwDeadTime)
                m_dwDeadTime = g_pTimerManager->GetServerTime() + 1000;
            
            if (static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject) == this)
            {
                GetSoundAndPlay(4, 0, 0);
            }

            return 1;
        }

        if (m_nWillDie == 3)
        {
            if (g_pCurrentScene->m_pMyHuman != this)
                DelayDelete();

            return 1;
        }

        int nType = 1;

        if (m_dwID <= 0 || m_dwID >= 1000)
            nType = 3;

        if (!m_cHide)
        {
            auto pPortal = new TMSkillTownPortal(TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y), nType);
            pScene->m_pEffectContainer->AddChild(pPortal);
        }

        if (pScene->m_pMyHuman != this)
            DelayDelete();
        return 1;
    }

    if (m_nWillDie == 1)
    {
        if (m_nWillDie == 1 && !m_dwDeadTime)
        {
            m_cDie = 0;
            Die();
            if (!m_dwDeadTime)
                m_dwDeadTime = g_pTimerManager->GetServerTime() + 1000;
        }
        return 1;
    }

    if (g_pCurrentScene->m_pMyHuman != this)
        DelayDelete();

	return 1;
}

int TMHuman::OnPacketSendItem(MSG_STANDARD* pStd)
{
    auto pSendItem = reinterpret_cast<MSG_SendItem*>(pStd);

    TMFieldScene* pFScene{};

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene)
        pFScene->Bag_View();

    auto pMobData = &g_pObjectManager->m_stMobData;

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        if (pSendItem->DestType == 0)
        {
            if (pSendItem->DestPos == 6 && BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[6], EF_WTYPE) == 41)
            {
                m_stLookInfo.RightMesh = 0;
                m_stLookInfo.RightSkin = 0;
            }

            memcpy(&pMobData->Equip[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            if (pSendItem->DestPos)
                memcpy(&g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            SGridControl* pGridEquip[16]{};

            pGridEquip[1] = pFScene->m_pGridHelm;
            pGridEquip[2] = pFScene->m_pGridCoat;
            pGridEquip[3] = pFScene->m_pGridPants;
            pGridEquip[4] = pFScene->m_pGridGloves;
            pGridEquip[5] = pFScene->m_pGridBoots;
            pGridEquip[6] = pFScene->m_pGridLeft;
            pGridEquip[7] = pFScene->m_pGridRight;
            pGridEquip[8] = pFScene->m_pGridRing;
            pGridEquip[9] = pFScene->m_pGridNecklace;
            pGridEquip[10] = pFScene->m_pGridOrb;
            pGridEquip[11] = pFScene->m_pGridCabuncle;
            pGridEquip[12] = pFScene->m_pGridGuild;
            pGridEquip[13] = pFScene->m_pGridEvent;
            pGridEquip[14] = pFScene->m_pGridDRing;
            pGridEquip[15] = pFScene->m_pGridMantua;

            if (pSendItem->DestPos > 0 && pSendItem->DestPos < 16)
            {
                if (pGridEquip[pSendItem->DestPos] != nullptr)
                {
                    SGridControlItem* pItem = pGridEquip[pSendItem->DestPos]->PickupItem(0, 0);

                    if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pItem)
                        g_pCursor->m_pAttachedItem = nullptr;

                    delete pItem;
                }

                if (pSendItem->Item.sIndex > 0)
                {
                    auto pstItem = new STRUCT_ITEM();

                    if (pstItem)
                    {
                        memcpy(pstItem, &pMobData->Equip[pSendItem->DestPos], sizeof(STRUCT_ITEM));

                        auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                        if (pItem)
                        {
                            if (pGridEquip[pSendItem->DestPos])
                            {
                                pGridEquip[pSendItem->DestPos]->Empty();
                                pGridEquip[pSendItem->DestPos]->AddItem(pItem, 0, 0);
                            }
                        }
                    }
                }
            }
        }
        else if (pSendItem->DestType == 1)
        {
            memcpy(&pMobData->Carry[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));
            memcpy(g_pObjectManager->m_stMobData.Carry, pMobData->Carry, sizeof(g_pObjectManager->m_stMobData.Carry));

            int Page = pSendItem->DestPos / 15;
            int CellIndexX = pSendItem->DestPos % 15 % 5;
            int CellIndexY = pSendItem->DestPos % 15 / 5;

            if (Page >= 0 && Page <= 3)
            {
                SGridControl* pGrid = pFScene->m_pGridInvList[Page];

                SGridControlItem* pOldGridItem = pGrid->PickupAtItem(CellIndexX, CellIndexY);

                if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pOldGridItem)
                    g_pCursor->m_pAttachedItem = nullptr;

                delete pOldGridItem;

                if (pSendItem->Item.sIndex > 0)
                {
                    auto pstItem = new STRUCT_ITEM();

                    if (pstItem)
                    {
                        memcpy(pstItem, &pSendItem->Item, sizeof(STRUCT_ITEM));

                        auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                        if (pItem)
                            pGrid->AddItem(pItem, CellIndexX, CellIndexY);
                    }
                }
            }
        }
        else if (pSendItem->DestType == 2)
        {
            memcpy(&g_pObjectManager->m_stItemCargo[pSendItem->DestPos], &pSendItem->Item, sizeof(STRUCT_ITEM));

            int CellIndexX = pSendItem->DestPos % 40 % 5;
            int CellIndexY = pSendItem->DestPos % 40 / 5;

            auto pGrid = pFScene->m_pCargoGridList[pSendItem->DestPos / 40];

            // TODO:
            // Check if this is correct
            pGrid->PickupAtItem(CellIndexX, CellIndexY);

            if (pSendItem->Item.sIndex > 0)
            {
                auto pstItem = new STRUCT_ITEM();

                if (pstItem)
                {
                    memcpy(pstItem, &pSendItem->Item, sizeof(STRUCT_ITEM));

                    auto pItem = new SGridControlItem(0, pstItem, 0.0f, 0.0f);

                    if (pItem)
                        pGrid->AddItem(pItem, CellIndexX, CellIndexY);
                }
            }
        }
    }

    SetPacketMOBItem(pMobData);
    SetCharHeight(static_cast<float>(m_stScore.Con));
    SetRace(pMobData->Equip[0].sIndex);

    if (m_nWeaponTypeL == 41)
    {
        m_stLookInfo.RightMesh = m_stLookInfo.LeftMesh;
        m_stLookInfo.RightSkin = m_stLookInfo.LeftSkin;
        m_stSancInfo.Sanc6 = m_stSancInfo.Sanc7;
        m_stSancInfo.Legend6 = m_stSancInfo.Legend7;
    }

    InitObject();
    CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
    InitAngle(0, m_fAngle, 0);

    if (m_cMount == 1)
    {
        int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);

        m_pMountHPBar->SetCurrentProgress(nMountHP);

        if (pFScene)
        {
            pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);

            char szMHP[32]{};
            sprintf_s(szMHP, "%d", nMountHP);

            pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
        }
    }

    if (pFScene)
        pFScene->UpdateScoreUI(0);

    SGridControl::m_sLastMouseOverIndex = -1;
    return 1;
}

int TMHuman::OnPacketUpdateEquip(MSG_STANDARD* pStd)
{
    auto pEquip = reinterpret_cast<MSG_UpdateEquip*>(pStd);
    STRUCT_ITEM item{};
    item.sIndex = pEquip->sEquip[0] & 0xFFF;

    TMEffectParticle* pParticle = nullptr;
    if (m_stLookInfo.FaceMesh != g_pItemList[item.sIndex].nIndexMesh || BASE_GetItemAbility(&item, EF_CLASS) != m_nClass)
        pParticle = new TMEffectParticle(TMVector3{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y }, 1, 3, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3{}, 1000u);

    if (pParticle)
        g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

    if (g_pSoundManager && g_pSoundManager->GetSoundData(158))
        g_pSoundManager->GetSoundData(158)->Play();

    auto pLightMap = new TMShade(4, 7, 1.0f);

    if (pLightMap)
    {
        pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pLightMap->SetPosition(m_vecPosition);
        pLightMap->m_dwLifeTime = 3000;
        pLightMap->SetColor(0xAAAAAAAA);
        g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);
    }

    SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

    if (!m_cHide)
    {
        auto pLevelUp = new TMEffectLevelUp(TMVector3{ m_vecPosition.x, m_fHeight, m_vecPosition.y }, 0);

        if (pLevelUp)
            g_pCurrentScene->m_pEffectContainer->AddChild(pLevelUp);
    }

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pMobData = &g_pObjectManager->m_stMobData;
        g_pObjectManager->m_stMobData.Equip[0].sIndex = pEquip->sEquip[0] & 0xFFF;

        if (m_cMount == 1)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);
            m_pMountHPBar->SetCurrentProgress(nMountHP);
            auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
            if (g_pCurrentScene)
            {
                pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);
                char szMHP[32] = { 0 };
                sprintf_s(szMHP, "%d", nMountHP);

                pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
            }
        }
    }

    SetPacketEquipItem(pEquip->sEquip);

    if (pEquip->sEquip[14] & 0xFFF && ((pEquip->sEquip[14] & 0xFFF) < 3980) || (pEquip->sEquip[14] & 0xFFF) >= 3999)
        SetMountCostume(pEquip->Equip2[14]);

    SetColorItem(pEquip->Equip2);
    float fCon = static_cast<float>(m_stScore.Con);

    SetCharHeight(fCon);
    SetRace(pEquip->sEquip[0] & 0xFFF);

    STRUCT_ITEM itemL{};
    itemL.sIndex = pEquip->sEquip[6] & 0xFFF;

    int nWeaponTypeL = BASE_GetItemAbility(&itemL, EF_WTYPE);
    if (nWeaponTypeL == 41)
    {
        m_stLookInfo.RightMesh = m_stLookInfo.LeftMesh;
        m_stLookInfo.RightSkin = m_stLookInfo.LeftSkin;

        m_stSancInfo.Sanc6 = m_stSancInfo.Sanc7;
        m_stSancInfo.Legend6 = m_stSancInfo.Legend7;
    }

    InitObject();

    CheckWeapon(pEquip->sEquip[6] & 0xFFF, pEquip->sEquip[7] & 0xFFF);
    InitAngle(0.0f, m_fAngle, 0.0f);

    TMFieldScene* pScene = nullptr;
    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pScene)
        pScene->UpdateScoreUI(0);

    SGridControl::m_sLastMouseOverIndex = -1;
	return 1;
}

int TMHuman::OnPacketUpdateAffect(MSG_STANDARD* pStd)
{
    auto pUpdateAffect = reinterpret_cast<MSG_UpdateAffect*>(pStd);

    TMFieldScene* pFScene{};

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        m_DilpunchJewel = 0;
        m_MoonlightJewel = 0;
        m_BloodJewel = 0;
        m_JewelGlasses = 0;
        m_RedJewel = 0;

        unsigned int dwServerTime = g_pTimerManager->GetServerTime();

        for (int i = 0; i < 32; ++i)
        {
            if ((8 * pUpdateAffect->Affect[i].Time - 8 - (dwServerTime - pFScene->m_dwStartAffectTime[i]) / 1000) / 8 != pUpdateAffect->Affect[i].Time
                || m_stAffect[i].Type != pUpdateAffect->Affect[i].Type)
            {
                memcpy(&m_stAffect[i], &pUpdateAffect->Affect[i], sizeof(m_stAffect[i]));
                pFScene->m_dwStartAffectTime[i] = g_pTimerManager->GetServerTime();
            }

            if (pUpdateAffect->Affect[i].Type == 8)
            {
                if (pUpdateAffect->Affect[i].Value & 0x1)
                    m_DilpunchJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x2)
                    m_MoonlightJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x4)
                    m_JewelGlasses = 1;
                if (pUpdateAffect->Affect[i].Value & 0x8)
                    m_BloodJewel = 1;
                if (pUpdateAffect->Affect[i].Value & 0x10)
                    m_RedJewel = 1;
            }
        }

        if (!pFScene->m_nYear && !pFScene->m_nDays || pFScene->m_dwEventTime && dwServerTime > (pFScene->m_dwEventTime + 3600000))
        {
            pFScene->m_dwEventTime = dwServerTime;
            MSG_MessageWhisper stWhisper{};
            stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
            stWhisper.Header.Type = MSG_MessageWhisper_Opcode;
            sprintf_s(stWhisper.MobName, "day");
            SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
        }

        if (pFScene)
            pFScene->UpdateScoreUI(1u);
    }
    return 1;
}

int TMHuman::OnPacketUpdateScore(MSG_STANDARD* pStd)
{
    auto pUpdateScore = reinterpret_cast<MSG_UpdateScore*>(pStd);

    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        static_cast<TMFieldScene*>(g_pCurrentScene)->Bag_View();

    m_cHide = m_dwID < 1000 == 1 && pUpdateScore->Score.Merchant & 1;
    
    if (m_dwID >= 1000 && pUpdateScore->ReqHp == 1)
    {
        TMHuman* pHuman = g_pObjectManager->GetHumanByID(m_dwID);

        if (pHuman != nullptr)
        {
            pHuman->m_BigHp = pHuman->m_usHP;
            pHuman->m_MaxBigHp = pHuman->m_usHP;
        }
    }

    TMFieldScene* pFScene{};

    if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (pFScene == nullptr)
        return 1;

    SListBox* pPartyList = pFScene->m_pPartyList;

    if (pPartyList != nullptr)
    {
        for (int i = 0; i < pPartyList->m_nNumItem; ++i)
        {
            auto pPartyItem = static_cast<SListBoxPartyItem*>(pPartyList->m_pItemList[i]);
            if (pPartyItem->m_dwCharID == m_dwID)
            {
                char szVal[32]{};

                sprintf_s(szVal, "%d", pUpdateScore->Score.Level + 1);

                pPartyItem->m_pLevelText->SetText(szVal, 0);

                if (pUpdateScore->Score.Hp > pUpdateScore->Score.MaxHp)
                    pUpdateScore->Score.Hp = pUpdateScore->Score.MaxHp;

                pPartyItem->m_pHpProgress->SetMaxProgress(pUpdateScore->Score.MaxHp);
                pPartyItem->m_pHpProgress->SetCurrentProgress(pUpdateScore->Score.Hp);
                break;
            }
        }
    }

    memcpy(&m_stScore, &pUpdateScore->Score, sizeof(m_stScore));

    if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this)
    {
        if (g_pObjectManager->m_stMobData.CurrentScore.Level < pUpdateScore->Score.Level && Is2stClass() == 2)
        {
            auto pMobData = &g_pObjectManager->m_stMobData;
            if (pFScene->m_pLevelQuest[pMobData->CurrentScore.Level + 1])
            {
                if (g_nBattleMaster % 10 >= 6 && pUpdateScore->Score.Level >= 350 || g_nBattleMaster % 10 < 6 && pUpdateScore->Score.Level < 350)
                {
                    pFScene->m_pQuestMemo->SetVisible(1);

                    DWORD dwCol = 0xFFAAAAFF;

                    char szStr[128]{};

                    if (pFScene->m_pLevelQuest[pMobData->CurrentScore.Level + 1] == 100)
                        dwCol = pFScene->LoadMsgText4(
                            szStr,
                            sizeof szStr,
                            "UI\\QuestMessage.txt",
                            pMobData->CurrentScore.Level + 2,
                            pMobData->Equip[0].sIndex % 10);
                    else
                        sprintf_s(szStr, g_pMessageStringTable[307]);

                    if (dwCol != 0xFF000000)
                    {
                        pFScene->m_pMessagePanel->SetMessage(szStr, 3000);

                        auto pItem = new SListBoxItem(szStr, dwCol, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);
                        
                        if (pItem)
                            pFScene->m_pChatListnotice->AddItem(pItem);
                    }
                }
            }
        }
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &pUpdateScore->Score, sizeof(STRUCT_SCORE));
        m_sGuildLevel = static_cast<unsigned char>(g_pObjectManager->m_stMobData.GuildLevel);
        g_pObjectManager->m_stMobData.Magic = static_cast<char>(pUpdateScore->Magic);
    }

    unsigned short usGuild = pUpdateScore->Guild;

    m_usGuild = usGuild;

    if (usGuild)
    {
        m_stGuildMark.bHideGuildmark = 0;
        m_stGuildMark.nGuild = usGuild & 0xFFF;
        m_stGuildMark.nSubGuild = BASE_GetSubGuild(m_sGuildLevel);
        m_stGuildMark.nGuildChannel = ((int)usGuild >> 12) & 0xF;
        m_stGuildMark.sGuildIndex = m_sGuildLevel;
        
        if (pFScene && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    else
    {
        m_stGuildMark.bHideGuildmark = 1;
        m_stGuildMark.pGuildMark->SetVisible(0);
    }

    SetCharHeight(static_cast<float>(m_stScore.Con));

    if (m_nClass == 40)
        m_fScale *= (((float)m_stScore.Special[3] * 0.003f) + 1.0f);

    if (pFScene && g_pCurrentScene->m_pMyHuman == this)
    {
        DWORD dwServerTime = g_pTimerManager->GetServerTime();

        for (int l = 0; l < 32; ++l)
        {
            if ((((m_usAffect[l] & 0xFF) - 1) * 8
                + 4
                - (dwServerTime - pFScene->m_dwStartAffectTime[l]) / 1000)
                / 8 != (pUpdateScore->Affect[l] & 0xFF)
                || (int)m_usAffect[l] >> 8 != (int)pUpdateScore->Affect[l] >> 8)
            {
                memcpy(&m_usAffect[l], &pUpdateScore->Affect[l], sizeof(m_usAffect[l]));
                pFScene->m_dwStartAffectTime[l] = dwServerTime;
            }
        }
    }
    else
    {
        memcpy(m_usAffect, pUpdateScore->Affect, sizeof(m_usAffect));
    }

    if (this == g_pCurrentScene->m_pMyHuman)
    {
        g_pObjectManager->m_stMobData.Critical = pUpdateScore->Critical;
        g_pObjectManager->m_stMobData.SaveMana = pUpdateScore->SaveMana;
    }

    char oldShaow = m_cShadow;
    CheckAffect();
    UpdateScore(pUpdateScore->GuildLevel);

    if (this == g_pCurrentScene->m_pMyHuman && pFScene)
    {
        if (!m_cOnlyMove)
            SetSpeed(pFScene->m_bMountDead);
    }
    else if (!m_cOnlyMove)
    {
        SetSpeed(0);
    }

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pMobData = &g_pObjectManager->m_stMobData;

        memcpy(pMobData->Resist, pUpdateScore->Resist, sizeof(pMobData->Resist));

        g_pObjectManager->m_stSelCharData.Guild[g_pObjectManager->m_cCharacterSlot] = usGuild;

        if (!usGuild)
            g_pObjectManager->m_usWarGuild = -1;

        memcpy(&pMobData->CurrentScore, &pUpdateScore->Score, sizeof(pMobData->CurrentScore));

        if (m_cMount == 1)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], EF_MOUNTHP);
            m_pMountHPBar->SetCurrentProgress(nMountHP);
            
            if (pFScene)
            {
                pFScene->m_pMHPBar->SetCurrentProgress(nMountHP);

                if (nMountHP < 0)
                    nMountHP = 0;

                char szMHP[32]{};
                sprintf(szMHP, "%d", nMountHP);
                pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
            }
        }

        if (pFScene)
        {
            pFScene->SetSanc();
            pFScene->m_nReqHP = pUpdateScore->ReqHp;
            pFScene->m_nReqMP = pUpdateScore->ReqMp;
        }

        if (pFScene)
            pFScene->UpdateScoreUI(0);
    }

    if (m_cMount && oldShaow != m_cShadow)
    {
        D3DXVECTOR3 m_vOldAngle{ m_pMount->m_vAngle };

        if (m_cShadow == 1)
        {
            memset(&m_stMountSanc, 0, sizeof(m_stMountSanc));
        }
        else
        {
            m_stMountSanc.Sanc0 = m_stOldMountSanc.Sanc0;
            m_stMountSanc.Sanc4 = m_stOldMountSanc.Sanc4;
            m_stMountSanc.Legend0 = m_stOldMountSanc.Legend0;
            m_stMountSanc.Legend4 = m_stOldMountSanc.Legend4;
        }
        UpdateMount();
        m_pMount->SetAngle(m_vOldAngle);
    }

    m_c8thSkill = pUpdateScore->LearnedSkill;

    if (oldShaow == 1 && !m_cShadow)
    {
        m_stSancInfo.Sanc0 = m_stOldSancInfo.Sanc0;
        m_stSancInfo.Sanc4 = m_stOldSancInfo.Sanc4;
        m_stSancInfo.Legend0 = m_stOldSancInfo.Legend0;
        m_stSancInfo.Legend4 = m_stOldSancInfo.Legend4;
        
        m_stColorInfo.Sanc0 = m_stOldColorInfo.Sanc0;
        m_stColorInfo.Sanc4 = m_stOldColorInfo.Sanc4;
        m_stColorInfo.Legend0 = m_stOldColorInfo.Legend0;
        m_stColorInfo.Legend4 = m_stOldColorInfo.Legend4;

        InitObject();

        if (g_pCurrentScene->m_pMyHuman == this)
        {
            CheckWeapon(
                g_pObjectManager->m_stMobData.Equip[6].sIndex, 
                g_pObjectManager->m_stMobData.Equip[7].sIndex);

            InitAngle(0, m_fAngle, 0);
        }
    }

    return 1;
}

int TMHuman::OnPacketSetHpMp(MSG_SetHpMp* pStd)
{
    if (g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    m_stScore.Hp = pStd->Hp;
    m_stScore.Mp = pStd->Mp;
    
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->m_cAutoAttack == 1 && pFScene->m_pTargetHuman == this && pStd->Hp <= 0)
        pFScene->m_pTargetHuman = 0;
    if (m_stScore.Hp >= m_stScore.MaxHp)
        m_stScore.Hp = m_stScore.MaxHp;

    m_pProgressBar->SetCurrentProgress(m_stScore.Hp);
    SetGuildBattleHPBar(m_stScore.Hp);
    SetGuildBattleLifeCount();

    if (pFScene->m_pMyHuman == this && !pFScene->m_bAirMove)
    {
        pFScene->m_nReqHP = pStd->ReqHp;
        pFScene->m_nReqMP = pStd->ReqMp;
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &m_stScore, sizeof(m_stScore));
        if (pFScene->m_pHPBar)
            pFScene->m_pHPBar->SetCurrentProgress(m_stScore.Hp);
        if (pFScene->m_pMPBar)
            pFScene->m_pMPBar->SetCurrentProgress(m_stScore.Mp);

        if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
        if (pFScene->m_pMainCharName)
            pFScene->m_pMainCharName->SetText(m_szName, 0);
        if (pFScene->m_pCurrentHPText)
        {
            char szHP[32]{};
            sprintf(szHP, "%d", m_stScore.Hp);
            if (pFScene->m_pCurrentHPText)
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
        }
        if (pFScene->m_pMaxHPText)
        {
            char szHP[32]{};
            sprintf(szHP, "/ %d", m_stScore.MaxHp);
            if (pFScene->m_pMaxHPText)
                pFScene->m_pMaxHPText->SetText(szHP, 0);
        }
        if (pFScene->m_pCurrentMPText)
        {
            char szMP[32]{};
            sprintf(szMP, "%d", m_stScore.Mp);
            if (pFScene->m_pCurrentMPText)
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
        }
        if (pFScene->m_pMaxMPText)
        {
            char szMP[32]{};
            sprintf(szMP, "/ %d", m_stScore.MaxMp);
            if (pFScene->m_pMaxMPText)
                pFScene->m_pMaxMPText->SetText(szMP, 0);
        }
        if (pFScene->m_pCurrentMHPText && m_pMountHPBar)
        {
            int nHP = m_pMountHPBar->GetCurrentProgress();
            if (!m_cMount)
                nHP = 0;

            char szMHP[32]{};
            sprintf(szMHP, "%d", nHP);
            pFScene->m_pCurrentMHPText->SetText(szMHP, 0);
        }
        if (pFScene->m_pMaxMHPText && m_pMountHPBar)
        {
            int nMaxHP = m_pMountHPBar->GetMaxProgress();
            if (!m_cMount)
                nMaxHP = 0;

            char szMHP[32]{};
            sprintf(szMHP, "%d", nMaxHP);
            pFScene->m_pMaxMHPText->SetText(szMHP, 0);
        }
    }
    if (m_stScore.Hp > 0 && (m_cDie == 1 || m_eMotion == ECHAR_MOTION::ECMOTION_DEAD))
    {
        m_cDie = 0;
        SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);
    }

    return 1;
}

int TMHuman::OnPacketSetHpDam(MSG_STANDARD* pStd)
{
    auto pSetHpDam = (MSG_SetHpDam*)pStd;
    if (g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    m_stScore.Hp = pSetHpDam->Hp;
    m_pProgressBar->SetCurrentProgress(pSetHpDam->Hp);
    SetGuildBattleHPBar(m_stScore.Hp);
    SetGuildBattleLifeCount();

    TMFont3* pFont = nullptr;
    unsigned int dwColor = 0xFFFFFF00;

    int nTX = 0;
    int nTY = 0;
    if (BASE_Get3DTo2DPos(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y, &nTX, &nTY))
    {
        char szVal[128]{};

        if (pSetHpDam->Dam < 0)
        {
            if (g_pCurrentScene->m_pMyHuman == this)
            {
                dwColor = 0xFFFF0000;

                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0,
                    500,
                    1,
                    1500,
                    0,
                    4);
            }
            else
            {
                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0,
                    500,
                    1,
                    1500,
                    0,
                    3);
            }
        }
        else
        {
            dwColor = 0xFF5555FF;
            if (pSetHpDam->Dam)
            {
                sprintf(szVal, "+ %d", pSetHpDam->Dam);
                pFont = new TMFont3(szVal,
                    nTX,
                    nTY + (int)(RenderDevice::m_fHeightRatio * 80.0f),
                    dwColor,
                    2.0f,
                    500,
                    1,
                    1500,
                    0,
                    2);
            }
        }
    }

    auto pScene = g_pCurrentScene;

    BASE_Get3DTo2DPos(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y, &nTX, &nTY);
    if (pFont)
        pScene->m_pExtraContainer->AddChild(pFont);
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &m_stScore, sizeof(m_stScore));
        auto pCurrentHPText = (SText*)pScene->m_pControlContainer->FindControl(65614);
        auto pHPBar = (SProgressBar*)pScene->m_pControlContainer->FindControl(65621);
        pHPBar->SetCurrentProgress(m_stScore.Hp);
        if (pCurrentHPText)
        {
            char szHP[32]{};
            sprintf(szHP, "%d", m_stScore.Hp);
            pCurrentHPText->SetText(szHP, 0);
        }
    }

    return 1;
}

int TMHuman::OnPacketMessageChat(MSG_STANDARD* pStd)
{
    auto pMsgChat = reinterpret_cast<MSG_MessageChat*>(pStd);

    pMsgChat->String[127] = 0;
    pMsgChat->String[126] = 0;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pScene->m_pChatGeneral)
        return 0;
    if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && !pScene->m_pChatGeneral->m_bSelected)
        return 1;

    auto pChatList = pScene->m_pChatList;
    int nIndex = 0;

    if (pMsgChat->String[0] == '*')
    {
        nIndex = 1;
        m_dwChatDelayTime = 10000;
    }
    else
        m_dwChatDelayTime = 3000;

    char szMsg[128]{};

    if (strlen(pMsgChat->String) + strlen(m_szName) <= 50)
    {
        sprintf(szMsg, "[%s]> %s", m_szName, &pMsgChat->String[nIndex]);

        auto ipNewItem = new SListBoxItem(szMsg, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0);

        if (ipNewItem && pChatList)
            pChatList->AddItem(ipNewItem);
    }
    else
    {
        char szMsg3[128]{};
        char szMsg2[128]{};
        if (IsClearString(pMsgChat->String, 39))
        {
            strncpy(szMsg3, pMsgChat->String, 40);
            sprintf(szMsg2, "%s", &pMsgChat->String[40]);
        }
        else
        {
            strncpy(szMsg3, pMsgChat->String, 39);
            sprintf(szMsg2, "%s", &pMsgChat->String[39]);
        }

        sprintf(szMsg, "[%s]> %s", m_szName, &szMsg3[nIndex]);

        auto ipNewItem = new SListBoxItem(szMsg, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);

        if (ipNewItem && pChatList)
            pChatList->AddItem(ipNewItem);

        auto ipNewItem2 = new SListBoxItem(szMsg2, 0xFFFFFFFF, 0.0, 0.0, 280.0f, 16.0f, 0, 0x77777777, 1u, 0);

        if (strlen(pMsgChat->String) > 40 && ipNewItem2 && pChatList)
            pChatList->AddItem(ipNewItem2);
    }

    sprintf(szMsg, "[%s]> %s", m_szName, &pMsgChat->String[nIndex]);
    pScene->m_dwChatTime = g_pTimerManager->GetServerTime();

    if (pMsgChat->String[0] != '=' && pMsgChat->String[0] != '-' && pMsgChat->String[0] != '@')
        SetChatMessage(&pMsgChat->String[nIndex]);

    return 1;
}

int TMHuman::OnPacketMessageChat_Index(MSG_STANDARD* pStd)
{
	return 0;
}

int TMHuman::OnPacketMessageChat_Param(MSG_STANDARD* pStd)
{
	return 0;
}

int TMHuman::OnPacketMessageWhisper(MSG_MessageWhisper* pMsg)
{
    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    
    SListBox* pChatList = pScene->m_pChatList;

    pMsg->MobName[15] = 0;
    pMsg->String[127] = 0;
    pMsg->String[126] = 0;

    int nIndex = 0;
    unsigned int dwColor = 0xFFFFFF00;
    int bDrawText = 1;

    char szMsg[128]{};

    if (pMsg->String[0] == '-' && pMsg->Color == 3)
    {
        if (!pScene->m_pChatGuild->m_bSelected)
            bDrawText = 0;

        dwColor = 0xFFAAFFFF;

        nIndex = 1;
        if (pMsg->String[1] == '-')
        {
            dwColor = 0xFF00FFFF;
            nIndex = 2;
        }

        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->Color == 7)
    {
        dwColor = 0xFFBBBBBB;
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, pMsg->String);
    }
    else if (pMsg->String[0] == '=')
    {
        if (pScene->m_pPartyList->m_nNumItem > 1)
        {
            dwColor = 0xFFFF99FF;
            nIndex = 1;
        }
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->String[0] == '@')
    {
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
        sprintf_s(szMsg, "[%s]> %s", pMsg->MobName, &pMsg->String[nIndex]);
    }
    else if (pMsg->String[0] != '!')
    {
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && !pScene->m_pChatWhisper->m_bSelected)
            bDrawText = 0;

        sprintf_s(szMsg, "[%s] : %s> %s", pMsg->MobName, m_szName, pMsg->String);

        _SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(" ", 0xFFFFFFFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        sprintf_s(szMsg, g_pMessageStringTable[226], pMsg->MobName);

        char szTime[128]{};
        sprintf_s(szTime, "%s [%02d:%02d:%02d]", szMsg, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(szTime, 0xFFFFFFFF, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        sprintf_s(szMsg, "%s", &pMsg->String[1]);

        if (pScene->m_pHelpList[3] != nullptr)
            pScene->m_pHelpList[3]->AddItem(new SListBoxItem(szMsg, 0xFFFFFFCC, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        if (pScene->m_pHelpMemo)
            pScene->m_pHelpMemo->SetVisible(1);
    }

    if (!bDrawText)
        return 1;

    if (strlen(pMsg->MobName) + strlen(szMsg) <= 55)
    {
        if (pChatList)
            pChatList->AddItem(new SListBoxItem(szMsg, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));
    }
    else
    {
        char szMsg2[128]{};
        char szMsg3[128]{};

        if (IsClearString(szMsg, 54))
        {
            strncpy(szMsg3, szMsg, 55);
            sprintf(szMsg2, "%s", &szMsg[55]);
        }
        else
        {
            strncpy(szMsg3, szMsg, 54);
            sprintf(szMsg2, "%s", &szMsg[54]);
        }

        if (pChatList)
            pChatList->AddItem(new SListBoxItem(szMsg3, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));

        if (strlen(szMsg) > 55)
        {
            if (pChatList)
                pChatList->AddItem(new SListBoxItem(szMsg2, dwColor, 0.0f, 0.0f, 280.0f, 16.0f, 0, 0x77777777u, 1u, 0));
        }
    }
    return 1;
}

int TMHuman::OnPacketUpdateEtc(MSG_STANDARD* pStd)
{
    auto pUpdateEtc = reinterpret_cast<MSG_UpdateEtc*>(pStd);

    if (g_pCurrentScene->m_pMyHuman == this)
    {
        if (g_pObjectManager->m_stMobData.LearnedSkill != pUpdateEtc->LearnedSkill)
        {
            if (g_pSoundManager)
            {
                auto pSoundData = g_pSoundManager->GetSoundData(31);

                if (pSoundData)
                    pSoundData->Play(0, 0);
            }

            bool bChange{ false };

            for (int i = 0; i < 20; ++i)
            {
                int nBit = (unsigned char)g_pObjectManager->m_cShortSkill[i] - 24 * g_pObjectManager->m_stMobData.Class;
                
                if ((unsigned char)g_pObjectManager->m_cShortSkill[i] >= 96)
                    nBit = g_pObjectManager->m_cShortSkill[i] - 72;

                if (((1 << nBit) & pUpdateEtc->LearnedSkill[0]) != 1 << nBit)
                {
                    if ((unsigned char)g_pObjectManager->m_cShortSkill[i] < 24)
                    {
                        g_pObjectManager->m_cShortSkill[i] = -1;
                        bChange = true;
                    }
                }
            }

            if (bChange)
            {
                MSG_SetShortSkill stSetShortSkill{};
                stSetShortSkill.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
                stSetShortSkill.Header.Type = MSG_SetShortSkill_Opcode;
                
                memcpy(stSetShortSkill.Skill, g_pObjectManager->m_cShortSkill, sizeof(stSetShortSkill.Skill));
                
                for (int j = 0; j < 20; ++j)
                {
                    if (stSetShortSkill.Skill[j] >= 0 && stSetShortSkill.Skill[j] < 96)
                        stSetShortSkill.Skill[j] -= 24 * g_pObjectManager->m_stMobData.Class;

                    else if (stSetShortSkill.Skill[j] >= 105 && stSetShortSkill.Skill[j] < 153)
                        stSetShortSkill.Skill[j] -= 12 * g_pObjectManager->m_stMobData.Class;
                }
                SendOneMessage((char*)&stSetShortSkill, sizeof(stSetShortSkill));
            }
        }
        g_pObjectManager->m_stMobData.LearnedSkill[0] = pUpdateEtc->LearnedSkill[0];
        g_pObjectManager->m_stMobData.LearnedSkill[1] = pUpdateEtc->LearnedSkill[1];
        g_pObjectManager->m_stMobData.Exp = pUpdateEtc->Exp;
        g_pObjectManager->m_stMobData.ScoreBonus = pUpdateEtc->ScoreBonus;
        g_pObjectManager->m_stMobData.SpecialBonus = pUpdateEtc->SpecialBonus;
        g_pObjectManager->m_stMobData.SkillBonus = pUpdateEtc->SkillBonus;
        g_pObjectManager->m_stMobData.Coin = pUpdateEtc->Coin;
        g_pObjectManager->m_nFakeExp = pUpdateEtc->FakeExp;
        g_pObjectManager->m_stSelCharData.Coin[g_pObjectManager->m_cCharacterSlot] = pUpdateEtc->Coin;

        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }
    return 1;
}

int TMHuman::OnPacketUpdateCoin(MSG_STANDARDPARM* pStd)
{
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_stMobData.Coin = pStd->Parm;
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }

    return 1;
}

int TMHuman::OnPacketUpdateRMB(MSG_STANDARDPARM* pStd)
{
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_RMBCount = pStd->Parm;
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            static_cast<TMFieldScene*>(g_pCurrentScene)->UpdateScoreUI(0);
    }

    return 1;
}

int TMHuman::OnPacketTrade(MSG_Trade* pStd)
{
    if (g_pCurrentScene->m_pMyHuman != this || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return 1;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    auto pOpCheckButton = (SButton*)g_pCurrentScene->m_pControlContainer->FindControl(601);
    pOpCheckButton->SetSelected((unsigned char)pStd->MyCheck);


    if (g_pObjectManager->m_stTrade.OpponentID || !pStd->OpponentID)
    {
        bool bChanged = false;

        SGridControl* pGridOp[15];
        for (int i = 0; i < 15; ++i)
        {
            pGridOp[i] = (SGridControl*)pScene->m_pControlContainer->FindControl(i + 8192);
            auto pPickedItem = pGridOp[i]->PickupItem(0, 0);
            if (pPickedItem && memcmp(pPickedItem->m_pItem, &pStd->Item[i], sizeof(STRUCT_ITEM)))
                bChanged = 1;
            if (!pPickedItem && pStd->Item[i].sIndex > 0)
                bChanged = 1;
            if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickedItem)
                g_pCursor->m_pAttachedItem = 0;
            
            SAFE_DELETE(pPickedItem);

            if (pStd->Item[i].sIndex > 0)
            {
                auto pstItem = new STRUCT_ITEM;
                if (pstItem)
                {
                    memcpy(pstItem, &pStd->Item[i], sizeof(STRUCT_ITEM));
                    pGridOp[i]->AddItem(new SGridControlItem(0, pstItem, 0.0f, 0.0f), 0, 0);
                }
            }
        }

        auto pOPGold = (SText*)pScene->m_pControlContainer->FindControl(603);

        char szGold[128]{};
        sprintf(szGold, "%10d", pStd->TradeMoney);
        if (bChanged == 1 || strcmp(szGold, pOPGold->GetText()))
        {
            auto pMyCheck = (SButton*)pScene->m_pControlContainer->FindControl(617);
            auto pOtherCheck = (SButton*)pScene->m_pControlContainer->FindControl(601);
            pMyCheck->m_bSelected = 0;
            pOtherCheck->m_bSelected = 0;

            pScene->m_dwLastCheckTime = g_pApp->m_pTimerManager->GetServerTime();
            g_pObjectManager->m_stTrade.MyCheck = pMyCheck->m_bSelected;
        }

        pOPGold->SetText(szGold, 0);
        if (!pScene->m_pControlContainer->FindControl(576)->IsVisible())
        {
            auto pTextMyName = (SText*)pScene->m_pControlContainer->FindControl(618);
            auto pTextOPName = (SText*)pScene->m_pControlContainer->FindControl(602);
            auto pNode = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->OpponentID);
            if (pNode)
            {
                char szMyName[128]{};
                sprintf(szMyName, "[%s]:%d", m_szName, strlen(m_szName));
                char szOPName[128]{};
                sprintf(szOPName, "[%s]:%d", pNode->m_szName, strlen(pNode->m_szName));
                pTextMyName->SetText(szMyName, 1);
                pTextOPName->SetText(szOPName, 1);
                pScene->SetVisibleTrade(1);
            }
        }
        return 1;
    }

    auto pOpp = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->OpponentID);
    if (pOpp)
    {
        char szMessage[128]{};
        sprintf(szMessage, g_pMessageStringTable[64], pOpp->m_szName, pOpp->m_stScore.Level + 1);
        pScene->m_pMessageBox->SetMessage(szMessage, 601, g_pMessageStringTable[28]);
        pScene->m_pMessageBox->m_dwArg = pStd->OpponentID;
        pScene->m_pMessageBox->SetVisible(1);
        g_pCursor->DetachItem();
        memcpy(&g_pObjectManager->m_stTrade, pStd, sizeof(g_pObjectManager->m_stTrade));
        g_pObjectManager->m_stTrade.OpponentID = 0;
        g_pObjectManager->m_stTrade.TradeMoney = 0;
    }

    return 1;
}

int TMHuman::OnPacketQuitTrade(MSG_STANDARD* pStd)
{
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        g_pObjectManager->m_stTrade.OpponentID = 0;
        g_pObjectManager->m_stTrade.MyCheck = 0;
        SGridControl::m_sLastMouseOverIndex = -1;
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
            auto pTradePanel = g_pCurrentScene->m_pControlContainer->FindControl(576);
            auto pATradePanel = pScene->m_pControlContainer->FindControl(646);
            if (pTradePanel && pTradePanel->IsVisible() == 1)
                pScene->SetVisibleTrade(0);
            if (pATradePanel && pATradePanel->IsVisible() == 1)
                pScene->SetVisibleAutoTrade(0, 0);
        }
    }

    return 1;
}

int TMHuman::OnPacketCarry(MSG_Carry* pStd)
{
    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        pScene->Bag_View();

    if (pScene->m_pMyHuman == this)
    {
        //pScene->m_pControlContainer->FindControl(65554u);
        for (int nCarryIndex = 0; nCarryIndex < 63; ++nCarryIndex)
        {
            if (nCarryIndex / 15 < 4)
            {
                auto pPickupItem = pScene->m_pGridInvList[nCarryIndex / 15]->PickupAtItem(nCarryIndex % 15 % 5, nCarryIndex % 15 / 5);
                if (g_pCursor->m_pAttachedItem && g_pCursor->m_pAttachedItem == pPickupItem)
                    g_pCursor->m_pAttachedItem = 0;
                
                SAFE_DELETE(pPickupItem);
            }
        }

        pScene->m_pGridInvList[0]->Empty();
        pScene->m_pGridInvList[1]->Empty();
        pScene->m_pGridInvList[2]->Empty();
        pScene->m_pGridInvList[3]->Empty();
        memcpy(g_pObjectManager->m_stMobData.Carry, pStd->Carry, sizeof(pStd->Carry));

        for (int nCarryIndex = 0; nCarryIndex < 63; ++nCarryIndex)
        {
            if (pStd->Carry[nCarryIndex].sIndex > 40)
            {
                auto pItemCarry = new STRUCT_ITEM;
                if (pItemCarry)
                {
                    memset(pItemCarry, 0, sizeof(STRUCT_ITEM));
                    memcpy(pItemCarry, &pStd->Carry[nCarryIndex], sizeof(STRUCT_ITEM));                   
                    if (nCarryIndex / 15 < 4)
                        pScene->m_pGridInvList[nCarryIndex / 15]->AddItem(new SGridControlItem(0, pItemCarry, 0.0f, 0.0f), nCarryIndex % 15 % 5, nCarryIndex % 15 / 5);
                }
            }
        }

        g_pObjectManager->m_stMobData.Coin = pStd->Coin;
        g_pObjectManager->m_stTrade.OpponentID = 0;
        g_pObjectManager->m_stTrade.MyCheck = 0;
        if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            pScene->SetVisibleTrade(0);
            pScene->SetVisibleInventory();
        }

        pScene->UpdateScoreUI(0);
    }

	return 1;
}

int TMHuman::OnPacketCNFCheck(MSG_STANDARD* pStd)
{
    auto pMyCheckButton = (SButton*)g_pCurrentScene->m_pControlContainer->FindControl(617);
    pMyCheckButton->m_bSelected = 1;
	return 1;
}

int TMHuman::OnPacketSetClan(MSG_STANDARDPARM* pStd)
{
    if (pStd->Parm == 4)
    {
        m_pNameLabel->m_GCBorder.dwColor = 0x5500AA00;
        m_pNameLabel->m_cBorder = 1;
        m_cSummons = 1;
    }
    else
    {
       m_cSummons = 0;
    }

    return 1;
}

int TMHuman::OnPacketReqRanking(MSG_STANDARDPARM2* pStd)
{
    auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pStd->Parm1);
    if (pHuman)
    {
        const static char szVS[4][16] = {
            "1 : 1",
            "5 : 5",
            "10 : 10",
            "All : All"
        };

        char szTemp[128]{};
        sprintf(szTemp, g_pMessageStringTable[153], pHuman->m_szName, szVS[pStd->Parm2 % 4]);

        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pScene->m_pMessageBox->SetMessage(szTemp, 927, g_pMessageStringTable[154]);
        pScene->m_pMessageBox->SetVisible(1);
        pScene->m_pMessageBox->m_dwArg = pStd->Parm1;
    }

    return 1;
}

int TMHuman::OnPacketVisualEffect(MSG_STANDARD* pStd)
{
    auto pParam = reinterpret_cast<MSG_STANDARDPARM*>(pStd);
    if (pParam->Parm == 1)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        unsigned dwLightColor = 0xFFFFFFFF;
        for (int i = -1; i < 2; ++i)
        {
            auto pEffect = new TMEffectMesh(506, dwLightColor, 0.0f, 0);
            if (pEffect)
            {
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->m_cShine = 1;
                pEffect->m_dwCycleTime = 3000;
                pEffect->m_dwLifeTime = 8000;
                pEffect->m_fScaleH = 1.0f;
                pEffect->m_fScaleV = 2.5f;
                pEffect->m_vecPosition = TMVector3(((float)i * 0.2f) + m_vecPosition.x,
                    m_fHeight,
                    ((float)i * 0.2f) + m_vecPosition.y);

                pScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        auto pLightShade = new TMShade(3, 118, 1.0);
        if (pLightShade)
        {
            pLightShade->m_dwLifeTime = 8000;
            pLightShade->SetColor(dwLightColor);
            pLightShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
            pLightShade->SetPosition(TMVector2(m_vecPosition.x, m_vecPosition.y));
            pScene->m_pEffectContainer->AddChild(pLightShade);
        }
        GetSoundAndPlay(1, 0, 0);
    }

    return 1;
}

/// Decodifica equipamentos visuais e montaria recebidos em MSG_CreateMob.
void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)
{
    if (!m_dwDelayDel)
    {
        m_sHeadIndex = pMobData->Equip[0].sIndex;
        m_sHelmIndex = pMobData->Equip[1].sIndex;
        m_citizen = static_cast<unsigned char>(pMobData->Equip[0].stEffect[2].cValue);
        m_cLegend = static_cast<char>(g_pItemList[pMobData->Equip[0].sIndex].nGrade);
        m_stLookInfo.FaceMesh = g_pItemList[pMobData->Equip[0].sIndex].nIndexMesh;
        m_stLookInfo.FaceSkin = g_pItemList[pMobData->Equip[0].sIndex].nIndexTexture;

        if (pMobData->Equip[1].sIndex >= 3500 && (pMobData->Equip[1].sIndex <= 3502 || pMobData->Equip[1].sIndex == 3507))
        {
            m_stLookInfo.HelmMesh = 0;
            m_stLookInfo.HelmSkin = 0;
        }
        else
        {
            m_stLookInfo.HelmMesh = g_pItemList[pMobData->Equip[1].sIndex].nIndexMesh;
            m_stLookInfo.HelmSkin = g_pItemList[pMobData->Equip[1].sIndex].nIndexTexture;
        }

        m_stLookInfo.CoatMesh = g_pItemList[pMobData->Equip[2].sIndex].nIndexMesh;
        m_stLookInfo.CoatSkin = g_pItemList[pMobData->Equip[2].sIndex].nIndexTexture;
        m_stLookInfo.PantsMesh = g_pItemList[pMobData->Equip[3].sIndex].nIndexMesh;
        m_stLookInfo.PantsSkin = g_pItemList[pMobData->Equip[3].sIndex].nIndexTexture;
        m_stLookInfo.GlovesMesh = g_pItemList[pMobData->Equip[4].sIndex].nIndexMesh;
        m_stLookInfo.GlovesSkin = g_pItemList[pMobData->Equip[4].sIndex].nIndexTexture;
        m_stLookInfo.BootsMesh = g_pItemList[pMobData->Equip[5].sIndex].nIndexMesh;
        m_stLookInfo.BootsSkin = g_pItemList[pMobData->Equip[5].sIndex].nIndexTexture;
        m_stLookInfo.LeftMesh = g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh;
        m_stLookInfo.LeftSkin = g_pItemList[pMobData->Equip[6].sIndex].nIndexTexture;
        m_stLookInfo.RightMesh = g_pItemList[pMobData->Equip[7].sIndex].nIndexMesh;
        m_stLookInfo.RightSkin = g_pItemList[pMobData->Equip[7].sIndex].nIndexTexture;

        if (pMobData->Equip[15].sIndex > 0)
        {
            m_sMantuaIndex = pMobData->Equip[15].sIndex;
            m_wMantuaSkin = g_pItemList[pMobData->Equip[15].sIndex].nIndexTexture;
            SetMantua(m_wMantuaSkin);
            m_ucMantuaLegend = static_cast<char>(g_pItemList[m_sMantuaIndex].nGrade);
            m_ucMantuaSanc = m_nTotalKill / 1000;
            if ((unsigned char)m_ucMantuaSanc > 9)
                m_ucMantuaSanc = 9;
        }
        else
            m_cMantua = 0;
        
        m_sFamiliar = pMobData->Equip[13].sIndex;
        m_sCostume = pMobData->Equip[12].sIndex;
        m_stSancInfo.Sanc0 = BASE_GetItemSanc(pMobData->Equip);
        m_stSancInfo.Sanc1 = BASE_GetItemSanc(&pMobData->Equip[1]);
        m_stSancInfo.Sanc2 = BASE_GetItemSanc(&pMobData->Equip[2]);
        m_stSancInfo.Sanc3 = BASE_GetItemSanc(&pMobData->Equip[3]);
        m_stSancInfo.Sanc4 = BASE_GetItemSanc(&pMobData->Equip[4]);
        m_stSancInfo.Sanc5 = BASE_GetItemSanc(&pMobData->Equip[5]);
        m_stSancInfo.Sanc7 = BASE_GetItemSanc(&pMobData->Equip[6]);
        m_stSancInfo.Sanc6 = BASE_GetItemSanc(&pMobData->Equip[7]);
        m_stColorInfo.Sanc0 = BASE_GetItemColorEffect(pMobData->Equip);
        m_stColorInfo.Sanc1 = BASE_GetItemColorEffect(&pMobData->Equip[1]);
        m_stColorInfo.Sanc2 = BASE_GetItemColorEffect(&pMobData->Equip[2]);
        m_stColorInfo.Sanc3 = BASE_GetItemColorEffect(&pMobData->Equip[3]);
        m_stColorInfo.Sanc4 = BASE_GetItemColorEffect(&pMobData->Equip[4]);
        m_stColorInfo.Sanc5 = BASE_GetItemColorEffect(&pMobData->Equip[5]);
        m_stColorInfo.Sanc7 = BASE_GetItemColorEffect(&pMobData->Equip[6]);
        m_stColorInfo.Sanc6 = BASE_GetItemColorEffect(&pMobData->Equip[7]);
        m_sFamCount = 0;

        if (m_sFamiliar == 769)
            m_sFamCount = BASE_GetItemSanc(&pMobData->Equip[13]) + 1;

        SetGuildBattleLifeCount();
        m_cDamageRate = 1;
        if (pMobData->Equip[13].sIndex == 786)
        {
            int nDmgSanc = BASE_GetItemSanc(&pMobData->Equip[13]);
            if (nDmgSanc >= 2 || pMobData->Equip[13].sIndex == 786)
                m_cDamageRate = nDmgSanc;
            else
                m_cDamageRate = 2;
        }

        if (pMobData->Equip[11].sIndex == 786)
            m_cDamageRate += 16 * BASE_GetItemSanc(&pMobData->Equip[11]);

        bool bSvadilfari = false;
        int tempIndex = pMobData->Equip[14].sIndex;

        if (pMobData->Equip[14].sIndex == 2383 || pMobData->Equip[14].sIndex == 2382)
            pMobData->Equip[14].sIndex = 2387;
        else if (pMobData->Equip[14].sIndex == 2387)
            bSvadilfari = true;

        if (tempIndex >= 2360 && tempIndex < 2390
            || tempIndex >= 3980 && tempIndex < 3999
            || tempIndex >= 2960 && tempIndex < 3000)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], 80);
            if (nMountHP > 0
                || pMobData->Equip[14].sIndex >= 2960 && pMobData->Equip[14].sIndex < 2999
                || pMobData->Equip[14].sIndex >= 3980 && pMobData->Equip[14].sIndex < 3999)
            {
                m_cLastMount = m_cMount;
                m_cMount = 1;
                int sIndex = pMobData->Equip[14].sIndex - 2045;
                int _nEquipIdx = pMobData->Equip[14].sIndex;

                if (_nEquipIdx == 2389)
                    sIndex = 346;
                else if (_nEquipIdx == 2378)
                    sIndex = 333;
                else if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388)
                    sIndex = 336;
                else if (_nEquipIdx >= 3980 && _nEquipIdx <= 3982)
                    sIndex = _nEquipIdx - 3638;
                else if (_nEquipIdx >= 3983 && _nEquipIdx <= 3985)
                    sIndex = _nEquipIdx - 3641;
                else if (_nEquipIdx >= 3986 && _nEquipIdx <= 3988)
                    sIndex = _nEquipIdx - 3644;
                else if(_nEquipIdx == 3989)
                    sIndex = 345;
                else if (_nEquipIdx == 3990)
                    sIndex = 334;
                else if (_nEquipIdx == 3991)
                    sIndex = 335;
                else if (_nEquipIdx == 3992)
                    sIndex = 318;
                else  if (_nEquipIdx == 3993 || _nEquipIdx == 3994)
                    sIndex = 200;
                else if (_nEquipIdx >= 2960 && _nEquipIdx <= 2961)
                    sIndex = _nEquipIdx - 2616;
                 
                STRUCT_ITEM item{};

                item.sIndex = sIndex;
                int nMountSanc = BASE_GetItemAbility(&pMobData->Equip[14], 81) / 10;
                int nClass = BASE_GetItemAbility(&item, 18);
                m_nMountSkinMeshType = BASE_DefineSkinMeshType(nClass);
                m_stMountLook.Mesh0 = g_pItemList[sIndex].nIndexMesh;
                m_stMountLook.Mesh1 = m_stMountLook.Mesh0;
                m_stMountLook.Skin0 = g_pItemList[sIndex].nIndexTexture;
                m_stMountLook.Skin1 = m_stMountLook.Skin0;
                m_sMountIndex = sIndex - 315;
                if (_nEquipIdx == 3993)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 12;
                    m_stMountSanc.Sanc1 = 12;
                }
                else if (_nEquipIdx == 3994)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 12;
                    m_stMountSanc.Sanc1 = 12;
                    m_stMountLook.Skin0 = 8;
                    m_stMountLook.Skin1 = 8;
                }
                else if (sIndex >= 321 && sIndex <= 325)
                {
                    m_stMountLook.Mesh2 = sIndex - 320;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else if (sIndex >= 326 && sIndex <= 330)
                {
                    m_stMountLook.Mesh2 = sIndex - 325;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else if (sIndex == 334 || sIndex == 335)
                {
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
                else if (sIndex >= 336 && sIndex <= 338)
                {
                    if (bSvadilfari)
                    {
                        m_stMountLook.Mesh0 = 10;
                        m_stMountLook.Mesh1 = 10;
                        m_stMountLook.Mesh2 = 11;
                        m_stMountSanc.Sanc2 = nMountSanc;
                        m_stMountSanc.Sanc0 = 0;
                        m_stMountSanc.Sanc1 = 0;
                    }
                    else
                    {
                        m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                        m_stMountSanc.Sanc2 = nMountSanc;
                        m_stMountSanc.Sanc0 = nMountSanc;
                        m_stMountSanc.Sanc1 = nMountSanc;
                    }
                }
                else if (sIndex >= 339 && sIndex <= 341)
                {
                    m_stMountLook.Mesh2 = sIndex + m_stMountLook.Mesh0 - 339;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
                else if (sIndex >= 342 && sIndex <= 345)
                {
                    switch (sIndex)
                    {
                    case 344:
                        m_stMountLook.Mesh2 = 10;
                        break;
                    case 345:
                        m_stMountLook.Mesh2 = 11;
                        break;
                    case 343:
                        m_stMountLook.Mesh2 = 11;
                        break;
                    default:
                        m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                        break;
                    }
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    if (sIndex == 345)
                        m_stMountSanc.Sanc2 = 7;
                }
                else
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }

                if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388 && !bSvadilfari)
                    m_stMountLook.Mesh2 = _nEquipIdx - 2379;

                m_fMountScale = BASE_GetMountScale(m_nMountSkinMeshType, m_stMountLook.Mesh0);
                if (sIndex == 333)
                    m_fMountScale = 1.1f;

                if (!m_cLastMount && m_nMountSkinMeshType == 31)
                {
                    auto pSoundManager = g_pSoundManager;

                    if (pSoundManager)
                    {
                        auto pSoundData = pSoundManager->GetSoundData(276);
                        if (pSoundData && !pSoundData->IsSoundPlaying())
                            pSoundData->Play();
                    }                    
                    m_cLastMount = m_cMount;
                }

                int nMountMaxHPIndex = sIndex - 315;
                if (sIndex - 315 < 0)
                    nMountMaxHPIndex = 0;
                switch (_nEquipIdx)
                {
                case 2378:
                    nMountMaxHPIndex = 18;
                    break;
                case 2379:
                    nMountMaxHPIndex = 19;
                    break;
                case 2380:
                    nMountMaxHPIndex = 21;
                    break;
                case 2381:
                    nMountMaxHPIndex = 20;
                    break;
                case 2382:
                case 2383:
                case 2384:
                case 2385:
                case 2386:
                    break;
                case 2387:
                    nMountMaxHPIndex = 20;
                    break;
                case 2388:
                    nMountMaxHPIndex = 21;
                    break;
                case 2389:
                    nMountMaxHPIndex = 22;
                    break;
                }
                m_pMountHPBar->SetMaxProgress(g_nMountHPTable[nMountMaxHPIndex]);
                m_pMountHPBar->SetCurrentProgress(nMountHP);

                TMFieldScene* pFSCene = (TMFieldScene*)g_pCurrentScene;
                if (pFSCene)
                {
                    pFSCene->m_pMHPBar->SetMaxProgress(g_nMountHPTable[nMountMaxHPIndex]);
                    pFSCene->m_pMHPBar->SetCurrentProgress(nMountHP);

                    char szMHP[32]{};
                    sprintf(szMHP, "%d", g_nMountHPTable[nMountMaxHPIndex]);
                    pFSCene->m_pMaxMHPText->SetText(szMHP, 0);

                    sprintf(szMHP, "%d", nMountHP);
                    pFSCene->m_pCurrentMHPText->SetText(szMHP, 0);
                }

                if (_nEquipIdx && (tempIndex < 3980 || tempIndex >= 3999))
                    SetMountCostume((unsigned char)pMobData->Equip[14].stEffect[2].cValue);
            }
            else
                m_cMount = 0;
        }
        else
        {
            m_cMount = 0;
            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
            if (pFScene && !pFScene->m_bAirMove)
            {
                char buffer[4];
                sprintf(buffer, "%d", 0);

                pFScene->m_pMaxMHPText->SetText(buffer, 0);
                sprintf(buffer, "%d", 0);
                pFScene->m_pCurrentMHPText->SetText(buffer, 0);
            }

            if(pFScene)
                pFScene->m_pMHPBar->SetCurrentProgress(0);
        }

        pMobData->Equip[14].sIndex = tempIndex;
        m_stSancInfo.Legend0 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[0].sIndex].nGrade);
        m_stSancInfo.Legend1 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[1].sIndex].nGrade);
        m_stSancInfo.Legend2 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[2].sIndex].nGrade);
        m_stSancInfo.Legend3 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[3].sIndex].nGrade);
        m_stSancInfo.Legend4 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[4].sIndex].nGrade);
        m_stSancInfo.Legend5 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[5].sIndex].nGrade);
        m_stSancInfo.Legend7 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[6].sIndex].nGrade);
        m_stSancInfo.Legend6 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[7].sIndex].nGrade);

        if ((unsigned char)m_stSancInfo.Legend0 <= 4
            && (unsigned char)m_stSancInfo.Sanc0 > 9)
        {
            m_stSancInfo.Legend0 = BASE_GetItemTenColor(pMobData->Equip) + 4;
        }
        else if (m_stSancInfo.Legend0 == 4 && (unsigned char)m_stSancInfo.Sanc0 > 9)
        {
            m_stSancInfo.Legend0 = BASE_GetItemTenColor(pMobData->Equip);
        }
        if ((unsigned char)m_stSancInfo.Legend1 <= 4
            && (unsigned char)m_stSancInfo.Sanc1 > 9)
        {
            m_stSancInfo.Legend1 = BASE_GetItemTenColor(&pMobData->Equip[1]) + 4;
        }
        else if (m_stSancInfo.Legend1 == 4 && (unsigned char)m_stSancInfo.Sanc1 > 9)
        {
            m_stSancInfo.Legend1 = BASE_GetItemTenColor(&pMobData->Equip[1]);
        }

        if ((unsigned char)m_stSancInfo.Legend2 <= 4
            && (unsigned char)m_stSancInfo.Sanc2 > 9)
        {
            m_stSancInfo.Legend2 = BASE_GetItemTenColor(&pMobData->Equip[2]) + 4;
        }
        else if (m_stSancInfo.Legend2 == 4 && (unsigned char)m_stSancInfo.Sanc2 > 9)
        {
            m_stSancInfo.Legend2 = BASE_GetItemTenColor(&pMobData->Equip[2]);
        }
        if ((unsigned char)m_stSancInfo.Legend3 <= 4
            && (unsigned char)m_stSancInfo.Sanc3 > 9)
        {
            m_stSancInfo.Legend3 = BASE_GetItemTenColor(&pMobData->Equip[3]) + 4;
        }
        else if (m_stSancInfo.Legend3 == 4 && (unsigned char)m_stSancInfo.Sanc3 > 9)
        {
            m_stSancInfo.Legend3 = BASE_GetItemTenColor(&pMobData->Equip[3]);
        }
        if ((unsigned char)m_stSancInfo.Legend4 <= 4
            && (unsigned char)m_stSancInfo.Sanc4 > 9)
        {
            m_stSancInfo.Legend4 = BASE_GetItemTenColor(&pMobData->Equip[4]) + 4;
        }
        else if (m_stSancInfo.Legend4 == 4 && (unsigned char)m_stSancInfo.Sanc4 > 9)
        {
            m_stSancInfo.Legend4 = BASE_GetItemTenColor(&pMobData->Equip[4]);
        }
        if ((unsigned char)m_stSancInfo.Legend5 <= 4
            && (unsigned char)m_stSancInfo.Sanc5 > 9)
        {
            m_stSancInfo.Legend5 = BASE_GetItemTenColor(&pMobData->Equip[5]) + 4;
        }
        else if (m_stSancInfo.Legend5 == 4 && (unsigned char)m_stSancInfo.Sanc5 > 9)
        {
            m_stSancInfo.Legend5 = BASE_GetItemTenColor(&pMobData->Equip[5]);
        }
        if ((unsigned char)m_stSancInfo.Legend7 <= 4
            && (unsigned char)m_stSancInfo.Sanc7 > 9)
        {
            m_stSancInfo.Legend7 = BASE_GetItemTenColor(&pMobData->Equip[6]) + 4;
        }
        else if (m_stSancInfo.Legend7 == 4 && (unsigned char)m_stSancInfo.Sanc7 > 9)
        {
            m_stSancInfo.Legend7 = BASE_GetItemTenColor(&pMobData->Equip[6]);
        }
        if ((unsigned char)m_stSancInfo.Legend6 <= 4
            && (unsigned char)m_stSancInfo.Sanc6 > 9)
        {
            m_stSancInfo.Legend6 = BASE_GetItemTenColor(&pMobData->Equip[7]) + 4;
        }
        else if (m_stSancInfo.Legend6 == 4 && (unsigned char)m_stSancInfo.Sanc6 > 9)
        {
            m_stSancInfo.Legend6 = BASE_GetItemTenColor(&pMobData->Equip[7]);
        }

        TMFieldScene* pFScene = nullptr;
        if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
            pFScene = (TMFieldScene*)g_pCurrentScene;

        if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this && pFScene)
        {
            int nValue = BASE_GetStaticItemAbility(&pMobData->Equip[14], 80);
            if (pMobData->Equip[14].sIndex < 3980 || pMobData->Equip[14].sIndex >= 3999)
                pFScene->m_bMountDead = nValue <= 0;
            else
                pFScene->m_bMountDead = 0;
        }

        m_stOldSancInfo = m_stSancInfo;
        m_stOldColorInfo = m_stColorInfo;
        m_stOldMountSanc = m_stMountSanc;
    }
}

void TMHuman::SetPacketEquipItem(unsigned short* sEquip)
{
    if (!m_dwDelayDel)
    {
        m_sHeadIndex = sEquip[0] & 0xFFF;
        m_sHelmIndex = sEquip[1] & 0xFFF;
        m_cLegend = static_cast<char>(g_pItemList[sEquip[0] & 0xFFF].nGrade);
        m_stLookInfo.FaceMesh = g_pItemList[sEquip[0] & 0xFFF].nIndexMesh;
        m_stLookInfo.FaceSkin = g_pItemList[sEquip[0] & 0xFFF].nIndexTexture;
        if ((sEquip[1] & 0xFFF) < 3500 || (sEquip[1] & 0xFFF) > 3502 && (sEquip[1] & 0xFFF) != 3507)
        {
            m_stLookInfo.HelmMesh = g_pItemList[sEquip[1] & 0xFFF].nIndexMesh;
            m_stLookInfo.HelmSkin = g_pItemList[sEquip[1] & 0xFFF].nIndexTexture;
        }
        m_stLookInfo.CoatMesh = g_pItemList[sEquip[2] & 0xFFF].nIndexMesh;
        m_stLookInfo.CoatSkin = g_pItemList[sEquip[2] & 0xFFF].nIndexTexture;
        m_stLookInfo.PantsMesh = g_pItemList[sEquip[3] & 0xFFF].nIndexMesh;
        m_stLookInfo.PantsSkin = g_pItemList[sEquip[3] & 0xFFF].nIndexTexture;
        m_stLookInfo.GlovesMesh = g_pItemList[sEquip[4] & 0xFFF].nIndexMesh;
        m_stLookInfo.GlovesSkin = g_pItemList[sEquip[4] & 0xFFF].nIndexTexture;
        m_stLookInfo.BootsMesh = g_pItemList[sEquip[5] & 0xFFF].nIndexMesh;
        m_stLookInfo.BootsSkin = g_pItemList[sEquip[5] & 0xFFF].nIndexTexture;
        m_stLookInfo.LeftMesh = g_pItemList[sEquip[6] & 0xFFF].nIndexMesh;
        m_stLookInfo.LeftSkin = g_pItemList[sEquip[6] & 0xFFF].nIndexTexture;
        m_stLookInfo.RightMesh = g_pItemList[sEquip[7] & 0xFFF].nIndexMesh;
        m_stLookInfo.RightSkin = g_pItemList[sEquip[7] & 0xFFF].nIndexTexture;
        if ((sEquip[15] & 0xFFF) > 0)
        {
            m_sMantuaIndex = sEquip[15] & 0xFFF;
            m_wMantuaSkin = g_pItemList[sEquip[15] & 0xFFF].nIndexTexture;
            SetMantua(m_wMantuaSkin);
            m_ucMantuaLegend = static_cast<char>(g_pItemList[sEquip[15] & 0xFFF].nGrade);
            m_ucMantuaSanc = m_nTotalKill / 1000;
            if ((unsigned char)m_ucMantuaSanc > 12)
                m_ucMantuaSanc = 12;
        }
        else
        {
            m_cMantua = 0;
        }

        if ((int)sEquip[12] < 4150 || (int)sEquip[12] > 4200)
            m_sCostume = 0;
        else
            m_sCostume = sEquip[12];

        m_sFamiliar = sEquip[13] & 0xFFF;
        m_stSancInfo.Sanc0 = (int)sEquip[0] >> 12;
        m_stSancInfo.Sanc1 = (int)sEquip[1] >> 12;
        m_stSancInfo.Sanc2 = (int)sEquip[2] >> 12;
        m_stSancInfo.Sanc3 = (int)sEquip[3] >> 12;
        m_stSancInfo.Sanc4 = (int)sEquip[4] >> 12;
        m_stSancInfo.Sanc5 = (int)sEquip[5] >> 12;
        m_stSancInfo.Sanc7 = (int)sEquip[6] >> 12;
        m_stSancInfo.Sanc6 = (int)sEquip[7] >> 12;

        if (g_pCurrentScene->m_pMyHuman == this)
            g_pObjectManager->m_stMobData.Equip[0].stEffect[0].cValue = m_stSancInfo.Sanc0;

        m_sFamCount = 0;
        if (m_sFamiliar == 769)
            m_sFamCount = ((int)sEquip[13] >> 12) + 1;

        SetGuildBattleLifeCount();
        m_cDamageRate = 1;
        if ((sEquip[13] & 0xFFF) == 786)
        {
            int nDmgSanc = (int)sEquip[13] >> 12;
            if (nDmgSanc >= 2 || (sEquip[13] & 0xFFF) == 786)
                m_cDamageRate = nDmgSanc;
            else
                m_cDamageRate = 2;
        }
        if ((sEquip[11] & 0xFFF) == 786)
            m_cDamageRate += 16 * ((int)sEquip[11] >> 12);

        int nMountIndex = sEquip[14] & 0xFFF;
        bool bSvadilfari = false;
        if (nMountIndex == 2383 || nMountIndex == 2382)
        {
            nMountIndex = 2387;
        }
        else if (nMountIndex == 2387)
        {
            bSvadilfari = 1;
        }

        if (nMountIndex >= 2360 && nMountIndex < 2390
            || nMountIndex >= 3980 && nMountIndex < 3999
            || nMountIndex >= 2960 && nMountIndex < 3000)
        {
            m_cMount = 1;
            int sIndex = nMountIndex - 2045;
            if (nMountIndex == 2389)
                sIndex = 346;
            else if (nMountIndex == 2378)
                sIndex = 333;
            else if (nMountIndex >= 2387 && nMountIndex <= 2388)
                sIndex = 336;
            else if (nMountIndex >= 3980 && nMountIndex <= 3982)
                sIndex = nMountIndex - 3638;
            else if (nMountIndex >= 3983 && nMountIndex <= 3985)
                sIndex = nMountIndex - 3641;
            else if (nMountIndex >= 3986 && nMountIndex <= 3988)
                sIndex = nMountIndex - 3644;
            else if (nMountIndex == 3989)
                sIndex = 345;
            else if (nMountIndex == 3990)
                sIndex = 334;
            else if (nMountIndex == 3991)
                sIndex = 335;
            else if (nMountIndex == 3992)
                sIndex = 318;
            else  if (nMountIndex == 3993 || nMountIndex == 3994)
                sIndex = 200;
            else if (nMountIndex >= 2960 && nMountIndex <= 2961)
                sIndex = nMountIndex - 2616;

            STRUCT_ITEM item{};

            item.sIndex = sIndex;
            m_sMountIndex = sIndex - 315;

            int nMountSanc = (int)sEquip[14] >> 12;
            int nClass = BASE_GetItemAbility(&item, 18);

            m_nMountSkinMeshType = BASE_DefineSkinMeshType(nClass);
            m_stMountLook.Mesh0 = g_pItemList[sIndex].nIndexMesh;
            m_stMountLook.Mesh1 = m_stMountLook.Mesh0;
            m_stMountLook.Skin0 = g_pItemList[sIndex].nIndexTexture;
            m_stMountLook.Skin1 = m_stMountLook.Skin0;
            m_sMountIndex = sIndex - 315;
            if (nMountIndex == 3993)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 12;
                m_stMountSanc.Sanc1 = 12;
            }
            else if (nMountIndex == 3994)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 12;
                m_stMountSanc.Sanc1 = 12;
                m_stMountLook.Skin0 = 8;
                m_stMountLook.Skin1 = 8;
            }
            else if (sIndex >= 321 && sIndex <= 325)
            {
                m_stMountLook.Mesh2 = sIndex - 320;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
            }
            else if (sIndex >= 326 && sIndex <= 330)
            {
                m_stMountLook.Mesh2 = sIndex - 325;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
            }
            else if (sIndex == 334 || sIndex == 335)
            {
                m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }
            else if (sIndex >= 336 && sIndex <= 338)
            {
                if (bSvadilfari)
                {
                    m_stMountLook.Mesh0 = 10;
                    m_stMountLook.Mesh1 = 10;
                    m_stMountLook.Mesh2 = 11;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else
                {
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
            }
            else if (sIndex >= 339 && sIndex <= 341)
            {
                m_stMountLook.Mesh2 = sIndex + m_stMountLook.Mesh0 - 339;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }
            else if (sIndex >= 342 && sIndex <= 345)
            {
                switch (sIndex)
                {
                case 344:
                    m_stMountLook.Mesh2 = 10;
                    break;
                case 345:
                    m_stMountLook.Mesh2 = 11;
                    break;
                case 343:
                    m_stMountLook.Mesh2 = 11;
                    break;
                default:
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    break;
                }
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
                m_stMountSanc.Sanc2 = 0;
                if (sIndex == 345)
                    m_stMountSanc.Sanc2 = 7;
            }
            else
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }

            if (nMountIndex >= 2387 && nMountIndex <= 2388 && !bSvadilfari)
                m_stMountLook.Mesh2 = nMountIndex - 2379;

            m_fMountScale = BASE_GetMountScale(m_nMountSkinMeshType, m_stMountLook.Mesh0);
            if (sIndex == 333)
                m_fMountScale = 1.0f;
        }
        else
            m_cMount = 0;

        m_stSancInfo.Legend0 = static_cast<unsigned char>(g_pItemList[sEquip[0] & 0xFFF].nGrade);
        m_stSancInfo.Legend1 = static_cast<unsigned char>(g_pItemList[sEquip[1] & 0xFFF].nGrade);
        m_stSancInfo.Legend2 = static_cast<unsigned char>(g_pItemList[sEquip[2] & 0xFFF].nGrade);
        m_stSancInfo.Legend3 = static_cast<unsigned char>(g_pItemList[sEquip[3] & 0xFFF].nGrade);
        m_stSancInfo.Legend4 = static_cast<unsigned char>(g_pItemList[sEquip[4] & 0xFFF].nGrade);
        m_stSancInfo.Legend5 = static_cast<unsigned char>(g_pItemList[sEquip[5] & 0xFFF].nGrade);
        m_stSancInfo.Legend7 = static_cast<unsigned char>(g_pItemList[sEquip[6] & 0xFFF].nGrade);
        m_stSancInfo.Legend6 = static_cast<unsigned char>(g_pItemList[sEquip[7] & 0xFFF].nGrade);

        m_stOldSancInfo = m_stSancInfo;
        m_stOldColorInfo = m_stColorInfo;
        m_stOldMountSanc = m_stMountSanc;
    }
}

void TMHuman::SetColorItem(char* sEquip2)
{
    char sEquipType = 0;
    char sEquipTypea = 0;
    char sEquipTypeb = 0; 
    char sEquipTypec = 0;
    char sEquipTyped = 0;  
    char sEquipTypee = 0; 
    char sEquipTypef = 0;
    char sEquipTypeg = 0;
    if (!m_dwDelayDel)
    {
        if (m_stSancInfo.Sanc0 > 9)
        {
            m_stColorInfo.Sanc0 = *sEquip2 & 0xF;
            if (m_stColorInfo.Sanc0)
                m_stColorInfo.Sanc0 += 115;
        }
        else
        {
            m_stColorInfo.Sanc0 = *sEquip2;
        }
        if (m_stSancInfo.Sanc1 > 9)
        {
            m_stColorInfo.Sanc1 = sEquip2[1] & 0xF;
            if (m_stColorInfo.Sanc1)
                m_stColorInfo.Sanc1 += 115;
        }
        else
        {
            m_stColorInfo.Sanc1 = sEquip2[1];
        }
        if (m_stSancInfo.Sanc2 > 9)
        {
            m_stColorInfo.Sanc2 = sEquip2[2] & 0xF;
            if (m_stColorInfo.Sanc2)
                m_stColorInfo.Sanc2 += 115;
        }
        else
        {
            m_stColorInfo.Sanc2 = sEquip2[2];
        }
        if (m_stSancInfo.Sanc3 > 9)
        {
            m_stColorInfo.Sanc3 = sEquip2[3] & 0xF;
            if (m_stColorInfo.Sanc3)
                m_stColorInfo.Sanc3 += 115;
        }
        else
        {
            m_stColorInfo.Sanc3 = sEquip2[3];
        }
        if (m_stSancInfo.Sanc4 > 9)
        {
            m_stColorInfo.Sanc4 = sEquip2[4] & 0xF;
            if (m_stColorInfo.Sanc4)
                m_stColorInfo.Sanc4 += 115;
        }
        else
        {
            m_stColorInfo.Sanc4 = sEquip2[4];
        }
        if (m_stSancInfo.Sanc5 > 9)
        {
            m_stColorInfo.Sanc5 = sEquip2[5] & 0xF;
            if (m_stColorInfo.Sanc5)
                m_stColorInfo.Sanc5 += 115;
        }
        else
        {
            m_stColorInfo.Sanc5 = sEquip2[5];
        }
        if (m_stSancInfo.Sanc7 > 9)
        {
            m_stColorInfo.Sanc7 = sEquip2[6] & 0xF;
            if (m_stColorInfo.Sanc7)
                m_stColorInfo.Sanc7 += 115;
        }
        else
        {
            m_stColorInfo.Sanc7 = sEquip2[6];
        }
        if (m_stSancInfo.Sanc6 > 9)
        {
            m_stColorInfo.Sanc6 = sEquip2[7] & 0xF;
            if (m_stColorInfo.Sanc6)
                m_stColorInfo.Sanc6 += 115;
        }
        else
        {
            m_stColorInfo.Sanc6 = sEquip2[7];
        }
        sEquipType = *sEquip2 >> 4;
        if (!sEquipType || m_stSancInfo.Legend0 > 4 || m_stSancInfo.Sanc0 <= 9)
        {
            if (sEquipType && m_stSancInfo.Legend0 == 4 && m_stSancInfo.Sanc0 > 9)
                m_stSancInfo.Legend0 = sEquipType + 4;
        }
        else
        {
            m_stSancInfo.Legend0 = sEquipType + 8;
        }
        sEquipTypea = sEquip2[1] >> 4;
        if (!sEquipTypea || m_stSancInfo.Legend1 > 4 || m_stSancInfo.Sanc1 <= 9)
        {
            if (sEquipTypea && m_stSancInfo.Legend1 == 4 && m_stSancInfo.Sanc1 > 9)
                m_stSancInfo.Legend1 = sEquipTypea + 4;
        }
        else
        {
            m_stSancInfo.Legend1 = sEquipTypea + 8;
        }
        sEquipTypeb = sEquip2[2] >> 4;
        if (!sEquipTypeb || m_stSancInfo.Legend2 > 4 || m_stSancInfo.Sanc2 <= 9)
        {
            if (sEquipTypeb && m_stSancInfo.Legend2 == 4 && m_stSancInfo.Sanc2 > 9)
                m_stSancInfo.Legend2 = sEquipTypeb + 4;
        }
        else
        {
            m_stSancInfo.Legend2 = sEquipTypeb + 8;
        }
        sEquipTypec = sEquip2[3] >> 4;
        if (!sEquipTypec || m_stSancInfo.Legend3 > 4 || m_stSancInfo.Sanc3 <= 9)
        {
            if (sEquipTypec && m_stSancInfo.Legend3 == 4 && m_stSancInfo.Sanc3 > 9)
                m_stSancInfo.Legend3 = sEquipTypec + 4;
        }
        else
        {
            m_stSancInfo.Legend3 = sEquipTypec + 8;
        }
        sEquipTyped = sEquip2[4] >> 4;
        if (!sEquipTyped || m_stSancInfo.Legend4 > 4 || m_stSancInfo.Sanc4 <= 9)
        {
            if (sEquipTyped && m_stSancInfo.Legend4 == 4 && m_stSancInfo.Sanc4 > 9)
                m_stSancInfo.Legend4 = sEquipTyped + 4;
        }
        else
        {
            m_stSancInfo.Legend4 = sEquipTyped + 8;
        }
        sEquipTypee = sEquip2[5] >> 4;
        if (!sEquipTypee || m_stSancInfo.Legend5 > 4 || m_stSancInfo.Sanc5 <= 9)
        {
            if (sEquipTypee && m_stSancInfo.Legend5 == 4 && m_stSancInfo.Sanc5 > 9)
                m_stSancInfo.Legend5 = sEquipTypee + 4;
        }
        else
        {
            m_stSancInfo.Legend5 = sEquipTypee + 8;
        }
        sEquipTypef = sEquip2[6] >> 4;
        if (m_stSancInfo.Legend7 > 4 || m_stSancInfo.Sanc7 <= 9)
        {
            if (sEquipTypef && m_stSancInfo.Legend7 == 4 && m_stSancInfo.Sanc7 > 9)
                m_stSancInfo.Legend7 = sEquipTypef + 4;
        }
        else
        {
            m_stSancInfo.Legend7 = sEquipTypef + 8;
        }
        sEquipTypeg = sEquip2[7] >> 4;
        int result = m_stSancInfo.Legend6;
        if (result > 4 || m_stSancInfo.Sanc6 <= 9)
        {
            if (sEquipTypeg)
            {
                if (m_stSancInfo.Legend6 == 4)
                {
                    result = m_stSancInfo.Sanc6;
                    if (result > 9)
                    {
                        result = sEquipTypeg;
                        m_stSancInfo.Legend6 += sEquipTypeg + 4;
                    }
                }
            }
        }
        else
        {
            result = sEquipTypeg + 8;
            m_stSancInfo.Legend6 = result;
        }
    }
}
