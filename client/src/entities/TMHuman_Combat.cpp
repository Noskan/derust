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

/// @file TMHuman_Combat.cpp
/// @brief Movimentacao, calculo de rota (GetRoute), ataques, dano recebido (Punched) e affects locais.

void TMHuman::MoveTo(TMVector2 vecPos)
{
    if (!m_dwDelayDel && (vecPos.x != m_vecPosition.x || vecPos.y != m_vecPosition.y))
    {
        TMVector2 dPosition = vecPos - m_vecPosition;
        m_vecMoveToPos = vecPos;

        m_fWantAngle = (float)atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

        if (m_fAngle < 0.0f)
            m_fAngle = m_fAngle + D3DXToRadian(360);
        else if (m_fAngle > D3DXToRadian(360))
            m_fAngle = m_fAngle - D3DXToRadian(360);

        m_fMoveToAngle = m_fAngle;
        if ((float)(m_fWantAngle - m_fMoveToAngle) < 0.0f && (float)(m_fWantAngle - m_fMoveToAngle) < -D3DXToRadian(180))
            m_fWantAngle = m_fWantAngle + D3DXToRadian(360);
        else if ((float)(m_fWantAngle - m_fMoveToAngle) > 0.0f && (float)(m_fWantAngle - m_fMoveToAngle) > D3DXToRadian(180))
            m_fWantAngle = m_fWantAngle - D3DXToRadian(360);
              
        m_vecFromPos = m_vecPosition;
        m_vecDPosition = m_vecMoveToPos - m_vecPosition;
        float fDistance = m_vecDPosition.DistanceFrom(TMVector2(0.0f, 0.0f));
        m_dwMoveToTime = g_pTimerManager->GetServerTime();
    }
}

/// Move em direcao ao alvo e executa ataque ao atingir distancia valida.
void TMHuman::MoveAttack(TMHuman* pTarget)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    if ((pTarget->m_dwID < 0 || pTarget->m_dwID >= 1000) && pTarget->IsMerchant())
        return;

    auto pFocused = g_pCurrentScene->m_pMyHuman;
    if ((m_stScore.Reserved & 0xF) == 15 && m_cMantua > 0 && pFocused->m_cMantua > 0 && m_cMantua == pFocused->m_cMantua)
        return;

    if (m_cCantAttk)
        return;

    if (pTarget->m_nClass == 66 && pTarget->m_cShadow == 1 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        return;

    if (IsInTown() == 1 || pTarget->IsInTown() == 1)
        return;

    if ((int)m_vecPosition.x >= 2362 && (int)m_vecPosition.x <= 2370 && (int)m_vecPosition.y >= 3927 && (int)m_vecPosition.y <= 3935)
        return;

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
        return;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pScene->m_dwLastTeleport != 0 || pScene->m_dwLastTown != 0 || pScene->m_dwLastLogout != 0 || g_dwStartQuitGameTime != 0)
        return;

    if (!TMFieldScene::m_bPK && pTarget->m_cSummons == 1 && g_pObjectManager->m_usWarGuild == pTarget->m_usGuild && pTarget->m_usGuild && g_bCastleWar)
    {
        // TODO: check this later
        int isInPos = (int)m_vecPosition.x >> 7 <= 16 || (int)m_vecPosition.x >> 7 >= 20 || (int)m_vecPosition.y >> 7 <= 29;
        if (isInPos || (m_cMantua > 0 && pTarget->m_cMantua == m_cMantua))
            return;
    }

    if (!TMFieldScene::m_bPK)
    {
        if (!g_bCastleWar && ((int)m_vecPosition.x >> 7 <= 16 || (int)m_vecPosition.x >> 7 >= 20 || (int)m_vecPosition.y >> 7 <= 29))
        {
            if (pTarget->m_usGuild && g_pObjectManager->m_usWarGuild != pTarget->m_usGuild)
                return;

            if (pScene->m_cAutoAttack == 1)
            {
                if (g_pObjectManager->m_usWarGuild > 0 && g_pObjectManager->m_usWarGuild != pTarget->m_usGuild)
                {
                    if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000)
                        return;
                }
            }
        }
        else if (m_cMantua > 0 && pTarget->m_cMantua == m_cMantua)
        {
            return;
        }
    }

    unsigned dwServerTime = g_pTimerManager->GetServerTime();
    pScene->m_pTargetHuman = pTarget;

    int nSpecForce = 0;
    if (g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x20000000)
    {
        nSpecForce = 1;
    }

    if (dwServerTime <= pScene->m_dwOldAttackTime + 1000)
        return;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;
    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);

    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);
    if (pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
    {
        nDistance -= 12;
        if (nDistance < 0)
            nDistance = 0;

        nTX = (int)pTarget->m_vecPosition.x;
        nTY = (int)pTarget->m_vecPosition.y;
    }

    auto pMobData = &g_pObjectManager->m_stMobData;
    static int nMotionIndex = 0;
    ++nMotionIndex;
    nMotionIndex %= 3;

    if (pTarget->m_cShadow == 1 && pTarget->m_nClass == 66 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        return;

    if (nDistance <= nMobAttackRange && nTX == (int)pTarget->m_vecPosition.x && nTY == (int)pTarget->m_vecPosition.y)
    {
        MSG_Attack stAttack{};
        stAttack.Header.Type = MSG_Attack_One_Opcode;
        stAttack.Header.ID = m_dwID;
        stAttack.AttackerID = m_dwID;
        stAttack.PosX = (int)m_vecPosition.x;
        stAttack.PosY = (int)m_vecPosition.y;

        if (pScene->m_stMoveStop.NextX)
        {
            stAttack.PosX = pScene->m_stMoveStop.NextX;
            stAttack.PosY = pScene->m_stMoveStop.NextY;
        }

        stAttack.CurrentMp = -1;
        stAttack.SkillIndex = -1;
        stAttack.SkillParm = 0;
        stAttack.Motion = nMotionIndex + 4;

        if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
            stAttack.SkillIndex = 151;
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

        stAttack.Dam[0].TargetID = pTarget->m_dwID;

        auto pHumanTarget = g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
        if (pHumanTarget)
        {
            int nCritical = (unsigned char)g_pObjectManager->m_stMobData.Critical;
            stAttack.Dam[0].Damage = -2;
            stAttack.Progress = TMFieldScene::m_usProgress;

            BASE_GetDoubleCritical(&g_pObjectManager->m_stMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);

            stAttack.TargetX = (int)pHumanTarget->m_vecPosition.x;
            stAttack.TargetY = (int)pHumanTarget->m_vecPosition.y;

            int nSize = sizeof(MSG_Attack);
            if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
            {
                stAttack.Header.Type = MSG_Attack_Two_Opcode;
                nSize = sizeof(MSG_AttackTwo);
            }
            if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
            {
                int nDX = (int)(pHumanTarget->m_vecPosition.x - pScene->m_pMyHuman->m_vecPosition.x);
                int nDY = (int)(pHumanTarget->m_vecPosition.y - pScene->m_pMyHuman->m_vecPosition.y);
                if (nDX > 0)
                    nDX = 1;
                else if (nDX < 0)
                    nDX = -1;
                if (nDY > 0)
                    nDY = 1;
                else if (nDY < 0)
                    nDY = -1;

                int TX = nDX + (int)pHumanTarget->m_vecPosition.x;
                int TY = nDY + (int)pHumanTarget->m_vecPosition.y;

                auto pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown;

                while (pNode->m_pNextLink != nullptr)
                {
                    if (pNode == pScene->m_pMyHuman || pNode == pHumanTarget || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
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
                        if (!TMFieldScene::m_bPK && pScene->m_pMyHuman->m_cMantua > 0 && pScene->m_pMyHuman->m_cMantua == pNode->m_cMantua && g_bCastleWar > 0)
                        {
                            pNode = (TMHuman*)pNode->m_pNextLink;
                            continue;
                        }
                        if (!TMFieldScene::m_bPK && g_bCastleWar > 0 && pScene->m_pMyHuman->m_cMantua == 3
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

            SendOneMessage((char*)&stAttack, nSize);

            MSG_Attack stAttackLocal{};
            memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
            stAttackLocal.Header.ID = m_dwID;
            stAttackLocal.FlagLocal = 1;
            if (nSpecForce)
                stAttackLocal.DoubleCritical |= 4;

            stAttackLocal.Progress = stAttack.Progress;
            pScene->OnPacketEvent(stAttack.Header.Type, (char*)&stAttackLocal);
            pScene->m_dwOldAttackTime = dwServerTime;
            return;
        }
    }
    else if (pScene->m_cAutoAttack == 1 &&
        (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02) && !m_cCantMove &&
        (int)GetKeyState(16) >> 8 <= 0)
    {
        int nMoveSX = (int)m_vecPosition.x;
        int nMoveSY = (int)m_vecPosition.y;

        nTX = (int)pTarget->m_vecPosition.x;
        nTY = (int)pTarget->m_vecPosition.y;

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
                return;

            if (nMoveSX != nTX)
                nMoveSX += PlusX;

            if (nMoveSY != nTY)
                nMoveSY += PlusY;

            nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
            ++nBreak;
        }

        pScene->m_vecMyNext.x = nMoveSX;
        pScene->m_vecMyNext.y = nMoveSY;
        GetRoute(pScene->m_vecMyNext, 0, 0);
    }
}

void TMHuman::MoveGet(TMItem* pTarget)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
        return;

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    pScene->m_pTargetItem = pTarget;

    if (dwServerTime <= pScene->m_dwGetItemTime + 1000)
        return;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;

    if (pScene->m_stMoveStop.NextX)
    {
        nSX = pScene->m_stMoveStop.NextX;
        nSY = pScene->m_stMoveStop.NextY;
    }

    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);

    if ((pTarget->m_stItem.sIndex == 1727 && m_stScore.Level < 1000) || pTarget->m_stItem.sIndex == 359 || (pTarget->m_stItem.sIndex >= 1733 && pTarget->m_stItem.sIndex <= 1736))
        return;

    if (nDistance > 1 ||
        nTX != (int)pTarget->m_vecPosition.x || nTY != (int)pTarget->m_vecPosition.y || m_LastSendTargetPos.x != (int)m_vecPosition.x || m_LastSendTargetPos.y != (int)m_vecPosition.y)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02)
        {
            short sVal = GetKeyState(16);
            if ((sVal >> 8) <= 0)
            {
                pScene->m_vecMyNext.x = (int)pTarget->m_vecPosition.x;
                pScene->m_vecMyNext.y = (int)pTarget->m_vecPosition.y;
                GetRoute(pScene->m_vecMyNext, 0, 0);
            }
        }

        return;
    }

    auto pGrid = pScene->m_pGridInv;
    int nGridIndex = BASE_GetItemAbility(&pTarget->m_stItem, 33);
    if (nGridIndex > 7 || nGridIndex < 0)
        nGridIndex = 0;

    auto vecGrid = pGrid->CanAddItemInEmpty(g_pItemGridXY[nGridIndex][0],
        g_pItemGridXY[nGridIndex][1]);

    if (vecGrid.x <= -1 || vecGrid.y <= -1 && BASE_GetItemAbility(&pTarget->m_stItem, 38) != 2)
        return;

    if (pTarget->m_stItem.sIndex == 773 || pTarget->m_stItem.sIndex == 746)
        return;

    MSG_GetItem stGetItem{};
    stGetItem.Header.ID = m_dwID;
    stGetItem.Header.Type = MSG_GetItem_Opcode;
    stGetItem.ItemID = pTarget->m_dwID;
    stGetItem.DestType = 1;
    stGetItem.DestPos = vecGrid.x + 5 * vecGrid.y;
    stGetItem.GridX = (int)pTarget->m_vecPosition.x;
    stGetItem.GridY = (int)pTarget->m_vecPosition.y;
    SendOneMessage((char*)&stGetItem, sizeof(stGetItem));

    pScene->m_pTargetItem = 0;
    pScene->m_dwOldAttackTime = dwServerTime;
    pScene->m_dwGetItemTime = dwServerTime;
}

void TMHuman::Attack(ECHAR_MOTION eMotion, TMVector2 vecTarget, char cSkillIndex)
{
    if (m_dwDelayDel)
        return;

    m_vecAttTargetPos = TMVector2(0.0f, 0.0f);
    m_fTargetHeight = 0.5f;

    auto dPosition = vecTarget - m_vecPosition;

    if (m_nClass != 44)
        m_fWantAngle = atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

    if (cSkillIndex >= 0 && cSkillIndex < 248)
    {
        m_bSkill = 1;
        m_nMotionIndex = 0;
        // A reconstrução tinha `Act1[i + m_cMount ? 3 : 0]`, que é `(i + m_cMount) ? 3 : 0`: todo
        // movimento depois do primeiro vinha da posição 3. O índice é i, deslocado 3 na montaria, como
        // no outro laço deste arquivo.
        for (int i = 0; i < 4; ++i)
        {
            if (m_nSkinMeshType == 1)
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act2[i + (m_cMount ? 3 : 0)] - 1);
            else
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act1[i + (m_cMount ? 3 : 0)] - 1);
        }

        m_eMotionBuffer[3] = ECHAR_MOTION::ECMOTION_NONE;
        SetAnimation(m_eMotionBuffer[m_nMotionIndex], 0);
        return;
    }

    if (eMotion != m_eMotion)
        SetAnimation(eMotion, 0);

    m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.2f) + (float)(vecTarget.x * 0.80000001f),
        (float)(m_vecPosition.y * 0.2f) + (float)(vecTarget.y * 0.80000001f));
}

void TMHuman::Attack(ECHAR_MOTION eMotion, TMHuman* pTarget, short cSkillIndex)
{
    if (m_dwDelayDel)
        return;

    if (!pTarget)
        return;

    auto dPosition = pTarget->m_vecPosition - m_vecPosition;
    m_fTargetHeight = 0.5f;

    float fW = TMHuman::m_vecPickSize[pTarget->m_nSkinMeshType].x;
    float fH = TMHuman::m_vecPickSize[pTarget->m_nSkinMeshType].y;

    m_fTargetHeight = (float)(sqrtf((float)(fW * fW) + (float)(fH * fH)) * pTarget->m_fScale) * 0.30000001f;

    if (m_fTargetHeight > 2.0f)
        m_fTargetHeight = 2.0f;
    if (pTarget != this && m_nClass != 44)
        m_fWantAngle = atan2f(dPosition.x, dPosition.y) + D3DXToRadian(90);

    m_nSkillIndex = cSkillIndex;

    if (cSkillIndex >= 0 && cSkillIndex <= 103 || cSkillIndex >= 200 && cSkillIndex < 248)
    {
        m_bSkill = 1;
        m_nMotionIndex = 0;
        m_nMotionCount = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (m_nSkinMeshType == 1)
            {
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act2[i + (m_cMount ? 3 : 0)] - 1);
                if ((int)m_eMotionBuffer[i] > 0)
                    ++m_nMotionCount;
            }
            else
            {
                m_eMotionBuffer[i] = (ECHAR_MOTION)(g_pSpell[cSkillIndex].Act1[i + (m_cMount ? 3 : 0)] - 1);
                if ((int)m_eMotionBuffer[i] > 0)
                    ++m_nMotionCount;
            }
        }

        m_eMotionBuffer[3] = ECHAR_MOTION::ECMOTION_NONE;
        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
        SetAnimation(m_eMotionBuffer[m_nMotionIndex], 0);
        m_nSkillIndex = -1;
        return;
    }

    if (eMotion != m_eMotion)
    {
        if (g_pCurrentScene->m_pMyHuman == this)
            SetAnimation(eMotion, 0);
        else if (m_nClass == 66 && 
            m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK04 && m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK05 && m_eMotion != ECHAR_MOTION::ECMOTION_ATTACK06 ||
            m_nClass != 66)
        {
            SetAnimation(eMotion, 0);
        }
        m_nSkillIndex = -1;
    }

    if (pTarget->m_nClass == 56 && !pTarget->m_stLookInfo.FaceMesh)
    {
        m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.60000002f) + (float)(pTarget->m_vecPosition.x * 0.40000001f),
            (float)(m_vecPosition.y * 0.60000002f) + (float)(pTarget->m_vecPosition.y * 0.40000001f));
    }
    else
    {
        m_vecAttTargetPos = TMVector2((float)(m_vecPosition.x * 0.2f) + (float)(pTarget->m_vecPosition.x * 0.80000001f),
            (float)(m_vecPosition.y * 0.2f) + (float)(pTarget->m_vecPosition.y * 0.80000001f));
    }
}

void TMHuman::Punched(int nDamage, TMVector2 vecFrom)
{
    if (m_dwDelayDel)
        return;

    if (m_eMotion != ECHAR_MOTION::ECMOTION_STRIKE)
    {
        int nBlood = GetBloodColor();
        if (m_eMotion != ECHAR_MOTION::ECMOTION_DIE)
        {
            float Tvalue = 0.2f;
            int Trand = 10;
            if (m_nClass == 66)
            {
                Tvalue = 0.8f;
                Trand = 30;
            }

            float fDam = 0.0f;
            if (m_stScore.Hp <= 0)
                fDam = (float)nDamage;
            else
                fDam = (float)nDamage / (float)m_stScore.Hp;

            if ((fDam > Tvalue || !(rand() % Trand) && (int)fDam > 0) && m_stScore.Hp > 0)
            {
                if (g_pCurrentScene->m_pMyHuman == this)
                {
                    unsigned int dwTime = g_pTimerManager->GetServerTime();
                    if (((int)m_eMotion < 4 || (int)m_eMotion > 9 || !(rand() % 5)) && dwTime - m_dwStartAnimationTime > 0x258)
                    {
                        TMHuman::SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
                    }
                }
                else
                {
                    TMHuman::SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
                }
            }
            if (fDam > 0.1f)
            {
                float fSize = (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + 1.01f;

                auto pDamageEffect = new TMEffectBillBoard(119,
                    500,
                    1.5f * fSize,
                    1.5f * fSize,
                    1.5f * fSize,
                    0.002f,
                    1,
                    80);

                pDamageEffect->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y);

                if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
                    pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                if (nBlood == 56)
                    pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

                g_pCurrentScene->AddChild(pDamageEffect);
            }
        }
    }

    if (m_stScore.Hp < 0)
        m_stScore.Hp = 0;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            if (pFScene->m_pMainCharName)
                pFScene->m_pMainCharName->SetText(m_szName, 0);
            if (pFScene->m_pCurrentHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "%d", m_stScore.Hp);
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pMaxHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "/ %d", m_stScore.MaxHp);
                pFScene->m_pMaxHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pCurrentMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "%d", m_stScore.Mp);
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
            }
            if (pFScene->m_pMaxMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "/ %d", m_stScore.MaxMp);
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
            if (pFScene->m_pHPBar)
                pFScene->m_pHPBar->SetMaxProgress(m_stScore.MaxHp);
            if (pFScene->m_pMPBar)
            {
                pFScene->m_pMPBar->SetMaxProgress(m_stScore.MaxMp);
                pFScene->m_pMPBar->SetCurrentProgress(m_stScore.Mp);
            }
            if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            {
                pFScene->m_pMHPBar->SetMaxProgress(m_pMountHPBar->GetMaxProgress());
                pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
            }
        }
        return;
    }

    if (m_MaxBigHp)
        m_pProgressBar->SetCurrentProgress(m_BigHp);
    else
        m_pProgressBar->SetCurrentProgress(m_stScore.Hp);

    SetGuildBattleHPBar(m_stScore.Hp);
    SetGuildBattleLifeCount();

    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        if (pFScene->m_pInfoText)
            pFScene->m_pInfoText->SetText(m_szName, 1);
    }
}

void TMHuman::Punched(int nDamage, TMHuman* pFrom)
{
    if (m_dwDelayDel || !pFrom)
        return;

    float fDam = 0.0f;
    if (m_stScore.Hp <= 0)
        fDam = (float)nDamage;
    else
        fDam = (float)nDamage / (float)m_stScore.Hp;

    int nBlood = GetBloodColor();
    if ((fDam > 0.2f || !(rand() % 10) && (int)fDam > 0) && m_stScore.Hp > 0)
    {
        if (g_pCurrentScene->m_pMyHuman == this)
        {
            unsigned int dwTime = g_pTimerManager->GetServerTime();
            if (((int)m_eMotion < 4 || (int)m_eMotion > 9 || !(rand() % 5)) && dwTime - m_dwStartAnimationTime > 600)
            {
                SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
            }
        }
        else
        {
            SetAnimation(ECHAR_MOTION::ECMOTION_STRIKE, 0);
        }
    }
    if (fDam > 0.1f)
    {
        float fSize = (float)(TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + 0.0099999998f;

        auto pDamageEffect = new TMEffectBillBoard(nBlood,
            600,
            0.1f * fSize,
            0.1f * fSize,
            0.1f * fSize,
            0.0049999999f,
            1,
            80);

        pDamageEffect->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y);

        if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
            pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        if (nBlood == 56)
            pDamageEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;

        g_pCurrentScene->AddChild(pDamageEffect);
    }

    if (m_stScore.Hp < 0)
        m_stScore.Hp = 0;
    if (g_pCurrentScene->m_pMyHuman == this)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
        {
            if (pFScene->m_pMainCharName)
                pFScene->m_pMainCharName->SetText(m_szName, 0);
            if (pFScene->m_pCurrentHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "%d", m_stScore.Hp);
                pFScene->m_pCurrentHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pMaxHPText)
            {
                char szHP[32]{};
                sprintf(szHP, "/ %d", m_stScore.MaxHp);
                pFScene->m_pMaxHPText->SetText(szHP, 0);
            }
            if (pFScene->m_pCurrentMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "%d", m_stScore.Mp);
                pFScene->m_pCurrentMPText->SetText(szMP, 0);
            }
            if (pFScene->m_pMaxMPText)
            {
                char szMP[32]{};
                sprintf(szMP, "/ %d", m_stScore.MaxMp);
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
            if (pFScene->m_pHPBar)
                pFScene->m_pHPBar->SetMaxProgress(m_stScore.MaxHp);
            if (pFScene->m_pMPBar)
            {
                pFScene->m_pMPBar->SetMaxProgress(m_stScore.MaxMp);
                pFScene->m_pMPBar->SetCurrentProgress(m_stScore.Mp);
            }
            if (m_pMountHPBar && pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
            {
                pFScene->m_pMHPBar->SetMaxProgress(m_pMountHPBar->GetMaxProgress());
                pFScene->m_pMHPBar->SetCurrentProgress(m_pMountHPBar->GetCurrentProgress());
            }
        }
        return;
    }
   
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    if (pFScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        m_pProgressBar->SetCurrentProgress(m_stScore.Hp);
        SetGuildBattleHPBar(m_stScore.Hp);
        SetGuildBattleLifeCount();

        if (pFScene->m_pInfoText)
            pFScene->m_pInfoText->SetText(m_szName, 1);
    }
}

void TMHuman::Fire(TMObject* pTarget, int nSkill)
{
    if (m_dwDelayDel)
        return;

    if (m_vecTempPos[0].x == 0.0f && m_vecTempPos[0].y == 0.0f && m_vecTempPos[0].z == 0.0f)
    {
        m_vecTempPos[0].x = m_vecPosition.x;
        m_vecTempPos[0].y = m_fHeight + 1.5f;
        m_vecTempPos[0].z = m_vecPosition.y;
    }

    if (nSkill == 0)
    {
        if (m_nSkinMeshType == 20 && (!m_stLookInfo.HelmMesh || m_stLookInfo.HelmMesh == 2) || m_nClass == 35)
        {
            auto pBreathFire = new TMFireEffect(m_vecTempPos[0], pTarget, 42);

            m_dwBreathStartTime = pBreathFire->m_dwCreateTime;
            m_dwBreathLifeTime = pBreathFire->m_dwLifeTime;

            g_pCurrentScene->AddChild(pBreathFire);
        }
    }
    else if (nSkill == 1)
    {
        auto pEffect = new TMEffectSpark(m_vecTempPos[0], pTarget, TMVector3(0.0f, 0.0f, 0.0f), 0xFFFF3300, 0xFF551100, 1000, 1.0f, 5, 0.0);
        pEffect->m_fRange = 0.5f;

        g_pCurrentScene->AddChild(pEffect);
    }
}

void TMHuman::Die()
{
    if (m_dwDelayDel)
        return;

    if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pScene->m_pTargetHuman = 0;
    }

    if ((int)m_wAttackerID > 0)
    {
        auto pAttacker = g_pObjectManager->GetHumanByID(m_wAttackerID);
        if (pAttacker)
        {
            if (pAttacker->m_cLifeDrain == 1)
            {
                // Dead code aparently
                /*
                 TMVector3 vecStart{ m_vecPosition.x, m_fHeight + 1.0f, m_vecPosition.y };
                 TMEffectBillBoard* pFire = nullptr;
                 */
            }
        }
    }

    if (m_cDie == 1)
        return;

    int nStartRouteIndex = m_nLastRouteIndex;
    if (m_fProgressRate > 0.5f)
        nStartRouteIndex = m_nLastRouteIndex + 1;

    for (int i = nStartRouteIndex + 1; i < 48; ++i)
        m_vecRouteBuffer[i] = m_vecRouteBuffer[nStartRouteIndex];

    SetAnimation(ECHAR_MOTION::ECMOTION_DIE, 0);

    if (m_nClass == 44)
    {
        TMVector3 vecPos{ m_vecPosition.x, m_fHeight + 2.0f, m_vecPosition.y };
        auto pParticle = new TMEffectParticle(vecPos, 4, 12, 0.05f, 0xFFFFAA00, 0, 56, 1.0, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);

        g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

        unsigned int dwColor = 0x44444444;
        auto pExplosion = new TMSkillExplosion2(vecPos, 0, 1.0f, 210, dwColor);

        g_pCurrentScene->m_pEffectContainer->AddChild(pExplosion);

        auto pBill = new TMEffectBillBoard(59, 2500, 0.2f, 0.2f, 0.2f, 0.003f, 1, 80);
        pBill->m_vecStartPos = pBill->m_vecPosition = vecPos;
        pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_DEFAULT;
        if (g_pDevice->m_bSavage == 1 || g_pDevice->m_bIntel == 1)
            pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
        pBill->SetColor(0xFFFFFFFF);

        g_pCurrentScene->m_pEffectContainer->AddChild(pBill);

        GetSoundAndPlay(309, 0, 0);
    }

    m_cPoison = 0;
    m_cHaste = 0;
    m_cAssert = 0;
    m_cFreeze = 0;
    m_cSlowSlash = 0;
    m_cSpeedUp = 0;
    m_cSpeedDown = 0;
    m_cShield = 0;
    m_cCancel = 0;
    m_cAurora = 0;
    m_cWeapon = 0;
    m_cSKillAmp = 0;
    m_cLighten = 0;
    m_cWaste = 0;

    SetAvatar(0);
    if (m_pInMiniMap)
        m_pInMiniMap->SetVisible(0);

    if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pFScene->m_dwLastDeadTime = g_pTimerManager->GetServerTime();
    }

    m_cSummons = 0;
    m_cDie = 1;
    m_vecTempPos[0].x = m_vecPosition.x;
    m_vecTempPos[0].y = m_fHeight + 1.5f;
    m_vecTempPos[0].z = m_vecPosition.y;

    g_pObjectManager->DeleteObject(m_pShade);
    m_pShade = nullptr;

    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
        StartKhepraDieEffect();
}

void TMHuman::Stand()
{
    if (m_dwDelayDel)
        return;

    char szBuffer[48]{};
    int nStartRouteIndex = m_nLastRouteIndex + 1;
    if (m_fProgressRate > 0.5f)
        nStartRouteIndex = m_nLastRouteIndex + 2;

    int nX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
    int nY = (int)m_vecRouteBuffer[nStartRouteIndex].y;
    for (int i = nStartRouteIndex; i < 48; ++i)
    {
        m_vecRouteBuffer[i].x = (float)nX;
        m_vecRouteBuffer[i].y = (float)nY;
    }
}

void TMHuman::OnlyMove(int nX, int nY, int nLocal)
{
    if (m_dwDelayDel)
        return;

    if (m_vecTargetPos.x == nX && m_vecTargetPos.y == nY)
        return;

    MSG_Action stAction{};
    stAction.Header.ID = m_dwID;
    stAction.PosX = nX;
    stAction.PosY = nY;
    stAction.Effect = 0;
    stAction.Header.Type = MSG_Action_Opcode;
    if (g_pCurrentScene->m_pMyHuman == this)
        stAction.Speed = g_nMyHumanSpeed;
    else
        stAction.Speed = (int)m_fMaxSpeed;

    stAction.TargetX = nX;
    stAction.TargetY = nY;

    if (!nLocal || nLocal == 2)
    {
        auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        pFScene->m_stMoveStop.LastX = stAction.PosX;
        pFScene->m_stMoveStop.LastY = stAction.PosY;
        pFScene->m_stMoveStop.NextX = stAction.TargetX;
        pFScene->m_stMoveStop.NextY = stAction.TargetY;
        SendOneMessage((char*)&stAction, sizeof(stAction));
    }
    if (!nLocal || nLocal == 1)
        OnPacketEvent(876, (char*)&stAction);
}

int TMHuman::IsGoMore()
{	
    if (m_dwDelayDel)
        return 0;

    if (m_vecRouteBuffer[m_nLastRouteIndex].x != m_vecRouteBuffer[m_nLastRouteIndex + 1].x || 
        m_vecRouteBuffer[m_nLastRouteIndex].y != m_vecRouteBuffer[m_nLastRouteIndex + 1].y)
    {
        return 1;
    }

    return 0;
}

void TMHuman::SetWantAngle(float fAngle)
{
    if (m_dwDelayDel)
        return;
    if (m_nClass == 44)
        fAngle = D3DXToRadian(180);

    m_fWantAngle = fAngle;
    m_fMoveToAngle = m_fAngle;
}

/// Calcula caminho ate a celula destino evitando obstaculos no mapa de altura.
void TMHuman::GetRoute(IVector2 vecTarget, int nCount, int bStop)
{
    if (!m_dwDelayDel && (vecTarget.x != m_LastSendTargetPos.x || vecTarget.y != m_LastSendTargetPos.y))
    {
        int nStartRouteIndex = m_nLastRouteIndex;
        if (m_fProgressRate > 0.5f)
            nStartRouteIndex = m_nLastRouteIndex + 1;
        int nSX = (int)m_vecRouteBuffer[nStartRouteIndex].x;
        int nSY = (int)m_vecRouteBuffer[nStartRouteIndex].y;
        unsigned int dwDealyTime = 1000;
        unsigned int dwServerTime = g_pTimerManager->GetServerTime();

        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 4)
            dwDealyTime = 500;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 5)
            dwDealyTime = 500;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 6)
            dwDealyTime = 100;
        if (this == g_pCurrentScene->m_pMyHuman && (int)m_fMaxSpeed == 7)
            dwDealyTime = 100;

        unsigned int dwTime = g_pTimerManager->GetServerTime();
        TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;

        if ((this != g_pCurrentScene->m_pMyHuman || g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD || 
            (dwTime >= g_dwStartQuitGameTime + 6000 
            && dwTime >= pFScene->m_dwLastLogout + 6000
            && dwTime >= pFScene->m_dwLastSelServer + 6000
            && dwTime >= pFScene->m_dwLastTown + 6000
            && dwTime >= pFScene->m_dwLastTeleport + 6000)))
        {
            if (dwServerTime - m_dwOldMovePacketTime > dwDealyTime || bStop)
            {
                char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
                int nTX = vecTarget.x;
                int nTY = vecTarget.y;

                char cRouteBuffer[48]{};

                TMScene* pScene = g_pCurrentScene;
                float fHeight = (float)pScene->GroundGetMask(m_vecPosition);
                int nMaxRoute = 12;

                if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
                    nMaxRoute = 16;

                BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute, pHeightMapData, 8);

                int nStart2 = pScene->GroundGetMask(TMVector2((float)m_LastSendTargetPos.x, (float)m_LastSendTargetPos.y));
                int nStart = pScene->GroundGetMask(TMVector2((float)nSX, (float)nSY));
                int nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y));

                int nHeight = abs(nEnd - nStart);
                int nHeight2 = abs(nEnd - nStart2);

                if (nHeight2 <= 30 || !m_LastSendTargetPos.x || !m_LastSendTargetPos.y)
                {
                    if (nHeight > 30)
                    {
                        memset(cRouteBuffer, 0, sizeof(cRouteBuffer));
                        BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute / 2, pHeightMapData, 8);

                        nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y));
                        nHeight = abs(nEnd - nStart);
                        if (nHeight > 30)
                        {
                            memset(cRouteBuffer, 0, sizeof(cRouteBuffer));
                            BASE_GetRoute(nSX, nSY, &vecTarget.x, &vecTarget.y, cRouteBuffer, nMaxRoute / 4, pHeightMapData, 8);

                            nEnd = pScene->GroundGetMask(TMVector2((float)vecTarget.x, (float)vecTarget.y)); 
                            nHeight = abs(nEnd - nStart);
                        }
                    }
                    if ((int)m_vecPosition.x == vecTarget.x
                     && (int)m_vecPosition.y == vecTarget.y)
                    {
                        if (bStop && (m_LastSendTargetPos.x != vecTarget.x || m_LastSendTargetPos.y != vecTarget.y))
                        {
                            m_cLastMoveStop = 1;
                            TMHuman* pObj = g_pCurrentScene->m_pMyHuman;

                            if (pObj == this
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO)
                            {
                                m_LastSendTargetPos.x = vecTarget.x;
                                m_LastSendTargetPos.y = vecTarget.y;

                                MSG_Action dst{};

                                dst.Header.ID = m_dwID;
                                dst.PosX = nSX;
                                dst.PosY = nSY;
                                dst.Effect = 0;
                                dst.Header.Type = MSG_Action_Opcode;
                                dst.Speed = g_nMyHumanSpeed;
                                dst.TargetX = vecTarget.x;
                                dst.TargetY = vecTarget.y;

                                g_pCurrentScene->OnPacketEvent(MSG_Action_Opcode, (char*)&dst);

                                if (bStop != 2)
                                {
                                    pFScene->m_stMoveStop.LastX = dst.PosX;
                                    pFScene->m_stMoveStop.LastY = dst.PosY;
                                    pFScene->m_stMoveStop.NextX = dst.TargetX;
                                    pFScene->m_stMoveStop.NextY = dst.TargetY;
                                    SendOneMessage((char*)&dst, 52);
                                    g_bLastStop = dst.Header.Type;
                                }

                                for (int i = 0; i < 48; ++i)
                                {
                                    m_vecRouteBuffer[i].x = (float)nSX + 0.5f;
                                    m_vecRouteBuffer[i].y = (float)nSY + 0.5f;
                                }

                                m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
                            }
                        }
                    }
                    else if (cRouteBuffer[0] != 0)
                    {
                        TMVector2 vecRouteTable[48]{};
                        int nRouteLen;
                        GenerateRouteTable(nSX, nSY, cRouteBuffer, vecRouteTable, &nRouteLen);

                        bool bFind = false;
                        int nRouteIndex = nRouteLen;
                        if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO)
                        {
                            for (nRouteIndex = nRouteLen; nRouteIndex > 0; --nRouteIndex)
                            {
                                bFind = false;
                                for (TMHuman* pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown; pNode && pNode->m_pNextLink; pNode = (TMHuman*)pNode->m_pNextLink)
                                {
                                    int nX = (int)pNode->m_vecRouteBuffer[47].x;
                                    int nY = (int)pNode->m_vecRouteBuffer[47].y;

                                    if ((int)vecRouteTable[nRouteIndex].x == nX && (int)vecRouteTable[nRouteIndex].y == nY && !pNode->m_cDie)
                                    {
                                        bFind = true;
                                        break;
                                    }
                                }

                                if (!bFind)
                                    break;
                            }
                        }

                        vecTarget.x = (int)vecRouteTable[nRouteIndex].x;
                        vecTarget.y = (int)vecRouteTable[nRouteIndex].y;

                        for (int k = nRouteIndex + 1; k < 48; ++k)
                        {
                            vecRouteTable[k].x = 0.0f;
                            vecRouteTable[k].y = 0.0f;
                        }

                        if (g_pCurrentScene->m_pMyHuman == this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                            pFScene->m_vecMyNext = vecTarget;

                        if ((vecTarget.x != nSX || vecTarget.y != nSY) && (vecTarget.x != m_vecTargetPos.x || vecTarget.y != m_vecTargetPos.y || bStop))
                        {
                            memset(m_cRouteBuffer, 0, sizeof(m_cRouteBuffer));
                            memcpy(m_cRouteBuffer, cRouteBuffer, nRouteIndex);
                            
                            MSG_Action stAction{};
                            stAction.Header.ID = m_dwID;
                            stAction.PosX = nSX;
                            stAction.PosY = nSY;
                            stAction.Effect = 0;

                            if (bStop)
                                stAction.Header.Type = MSG_Action_Stop_Opcode;
                            else
                                stAction.Header.Type = MSG_Action_Opcode;

                            if (g_pCurrentScene->m_pMyHuman == this)
                                stAction.Speed = g_nMyHumanSpeed;
                            else
                                stAction.Speed = (int)m_fMaxSpeed;

                            stAction.TargetX = vecTarget.x;
                            stAction.TargetY = vecTarget.y;

                            for (int j = 0; j < 23; ++j)
                                stAction.Route[j] = m_cRouteBuffer[j % 24];

                            if (m_LastSendTargetPos.x != vecTarget.x || m_LastSendTargetPos.y != vecTarget.y)
                            {
                                if (m_cLastMoveStop == bStop && bStop == 1)
                                    return;

                                m_cLastMoveStop = bStop;
                                if (g_pCurrentScene->m_pMyHuman == this
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
                                    && g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO
                                    && dwServerTime - m_dwOldMovePacketTime > 1000
                                    && bStop != 2)
                                {
                                    pFScene->m_stMoveStop.LastX = stAction.PosX;
                                    pFScene->m_stMoveStop.LastY = stAction.PosY;
                                    pFScene->m_stMoveStop.NextX = stAction.TargetX;
                                    pFScene->m_stMoveStop.NextY = stAction.TargetY;
                                    SendOneMessage((char*)&stAction, sizeof(stAction));
                                    g_bLastStop = stAction.Header.Type;

                                    m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
                                    m_LastSendTargetPos.x = stAction.TargetX;
                                    m_LastSendTargetPos.y = stAction.TargetY;
                                }
                            }

                            OnPacketEvent(MSG_Action_Opcode, (char*)&stAction);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void TMHuman::GenerateRouteTable(int nSX, int nSY, char* pRouteBuffer, TMVector2* pRouteTable, int* pMaxRouteIndex)
{
    if (m_dwDelayDel != 0)
        return;

    TMVector2 vecCurrent = TMVector2((float)nSX + 0.5f, (float)nSY + 0.5f);
    pRouteTable[0] = vecCurrent;

    if (pMaxRouteIndex)
        *pMaxRouteIndex = strlen(pRouteBuffer) + 2;

    for (int i = 1; i < 48; ++i)
    {
        pRouteTable[i] = vecCurrent;
        switch (pRouteBuffer[i - 1])
        {
        case '6':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            break;
        case '4':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            break;
        case '8':
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        case '2':
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '3':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '1':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            pRouteTable[i].y = pRouteTable[i].y - 1.0f;
            break;
        case '9':
            pRouteTable[i].x = pRouteTable[i].x + 1.0f;
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        case '7':
            pRouteTable[i].x = pRouteTable[i].x - 1.0f;
            pRouteTable[i].y = pRouteTable[i].y + 1.0f;
            break;
        }
        vecCurrent = pRouteTable[i];
    }

    //std::cout << "Current " << vecCurrent.x << " " << vecCurrent.y << '\n';
}

int TMHuman::StraightRouteTable(int nSX, int nSY, int nTargetX, int nTargetY, TMVector2* pRouteTable, int* pMaxRouteIndex, int distance, char* pHeight, int MH)
{
    if (m_dwDelayDel)
        return 0;

    if (nSX == nTargetX && nSY == nTargetY)
        return 0;

    TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;

    // This is probably e ifndef of debug
    int a;
    if (g_pCurrentScene->m_pMyHuman == this)
        a = 0;

    int nDis = BASE_GetDistance(nSX, nSY, nTargetX, nTargetY);

    TMVector2 vecCurrent;
    // TODO: verify this vecposition.y 
    if ((int)m_vecPosition.y == nTargetX && (int)m_vecPosition.y == nTargetY)
        return 0;

    vecCurrent.x = m_vecPosition.x;
    vecCurrent.y = m_vecPosition.y;

    D3DXVECTOR2 NorVec2;
    auto pV = D3DXVECTOR2((float)(nTargetX - nSX), (float)(nTargetY - nSY));
    D3DXVec2Normalize(&NorVec2, &pV);

    auto pV2 = D3DXVECTOR2((float)((float)nTargetX + 0.5f) - vecCurrent.x,
        (float)((float)nTargetY + 0.5f) - vecCurrent.y);
    float TargetLen = D3DXVec2Length(&pV2);

    pRouteTable[0] = vecCurrent;

    float NowLen = 0.0f;
    int nMax = 48;
    int nSXa = (int)pRouteTable->x;
    int nSYa = (int)pRouteTable->y;

    int nPlusX = 1;
    if (nTargetX - nSXa < 0)
        nPlusX = -1;

    int nPlusY = 1;
    if (nTargetY - nSYa < 0)
        nPlusY = -1;

    int nMinX = nSXa <= nTargetX ? nSXa : nTargetX;
    int nMinY = nSYa <= nTargetY ? nSYa : nTargetY;
    int nMaxX = nSXa <= nTargetX ? nTargetX : nSXa;
    int nMaxY = nSYa <= nTargetY ? nTargetY : nSYa;

    int Cul = pHeight[nSXa + g_HeightWidth * (nSYa - g_HeightPosY) - g_HeightPosX];

    for (int nY = nMinY; nY < nMaxY; ++nY)
    {
        for (int nX = nMinX; nX < nMaxX; ++nX)
        {
            int nH = pHeight[nX + g_HeightWidth * (nY - g_HeightPosY) - g_HeightPosX];
            if (abs(nH - Cul) > MH)
                return 0;
        }
    }

    for (int i = 1; i < 48; ++i)
    {
        pRouteTable[i] = vecCurrent;
        if (nMax > i)
        {
            float fy = (float)nDis;
            float value = TargetLen;

            pRouteTable[i] += ((TMVector2(NorVec2.x, NorVec2.y) * value) / fy);

            int nSXb = (int)pRouteTable[i].x;
            int nSYb = (int)pRouteTable[i].y;

            if (nSXb != (int)pRouteTable[i - 1].x || pRouteTable[i].y != pRouteTable[i - 1].y)
            {
                Cul = pHeight[nSXb + g_HeightWidth * (nSYb - g_HeightPosY) - g_HeightPosX];
                if ((int)pRouteTable[0].x != nSXb - nPlusX)
                {
                    int CulX = pHeight[nSXb + g_HeightWidth * (nSYb - g_HeightPosY) - nPlusX - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulX))
                        return 0;
                }
                if ((int)pRouteTable[0].y != nSYb - nPlusY)
                {
                    int CulY = pHeight[nSXb + g_HeightWidth * (nSYb - nPlusY - g_HeightPosY) - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulY))
                        return 0;
                }
                if ((int)pRouteTable[0].y != nSYb - nPlusY && (int)pRouteTable[0].x != nSXb - nPlusX)
                {
                    int CulXY = pHeight[nSXb + g_HeightWidth * (nSYb - nPlusY - g_HeightPosY) - nPlusX - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulXY))
                        return 0;
                }

                if (i)
                {
                    int CulBack = pHeight[(int)pRouteTable[i - 1].x
                        + g_HeightWidth * ((int)pRouteTable[i - 1].y - g_HeightPosY)
                        - g_HeightPosX];
                    if (MH - 2 <= abs(Cul - CulBack))
                        return 0;
                }
            }

            if (i >= nDis)
            {
                nMax = i;
                pRouteTable[i].x = (float)nTargetX + 0.5f;
                pRouteTable[i].y = (float)nTargetY + 0.5f;
            }
        }

        vecCurrent = pRouteTable[i];
    }

    if (pMaxRouteIndex)
        *pMaxRouteIndex = nMax;

    m_cSameHeight = 2;
    return 1;
}

int TMHuman::ChangeRouteBuffer(int nSX, int nSY, TMVector2* pRouteTable, int* pMaxRouteIndex)
{	
    char* pHeightMapData = (char*)g_pCurrentScene->m_HeightMapData;
    int nRoutCount = 0;            

    int i;
    for (i = 46; i >= 0; --i)
    {
        if (pRouteTable[i].x != pRouteTable[i + 1].x || 
            pRouteTable[i].y != pRouteTable[i + 1].y)
        {
            int tX = (int)pRouteTable[i].x;
            int tY = (int)pRouteTable[i].y;
            
            char szBuffer[48]{};

            BASE_GetRoute(nSX, nSY, &tX, &tY, szBuffer, 12, pHeightMapData, 8);
            if (tX == (int)pRouteTable[i].x && tY == (int)pRouteTable[i].y)
            {
                nRoutCount = i;
                break;
            }
        }
    }

    if (i < 0 || i > 46)
        return 0;

    if (nRoutCount < 0 || nRoutCount > 46)
        return 0;

    TMVector2 vecRouteBuffer[48]{};
    TMVector2 vecCurrent;
    int nMaxRouteIndex;
    char szBuffer[48]{};
    GenerateRouteTable(nSX, nSY, szBuffer, vecRouteBuffer, &nMaxRouteIndex);

    if (nMaxRouteIndex - 2 < 0)
        return 0;

    vecCurrent = vecRouteBuffer[nMaxRouteIndex - 2];
    for (i = nMaxRouteIndex - 2; i < 48; ++i)
    {
        vecRouteBuffer[i] = vecCurrent;
        if (i + nRoutCount - (nMaxRouteIndex - 2) < 48)
           vecRouteBuffer[i] = pRouteTable[i + nRoutCount - (nMaxRouteIndex - 2)];

        vecCurrent = vecRouteBuffer[i];
    }

    memcpy(pRouteTable, vecRouteBuffer, sizeof(vecRouteBuffer));
    return 1;
}

/// Atualiza estado visual e timers dos buffs/debuffs (affects) da entidade.
void TMHuman::CheckAffect()
{
    if (m_dwDelayDel)
        return;

    if (!m_pSkinMesh)
        return;

    m_cPoison = 0;
    m_cHaste = 0;
    m_cAssert = 0;
    m_cFreeze = 0;
    m_cSlowSlash = 0;
    m_cPowerUp = 0;
    m_cSpeedUp = 0;
    m_cSpeedDown = 0;
    m_cShield = 0;
    m_cCancel = 0;
    m_cAurora = 0;
    m_cWeapon = 0;
    m_cSKillAmp = 0;
    m_cLighten = 0;
    m_cWaste = 0;
    m_cProtector = 0;
    m_cEnchant = 0;
    m_cShadow = 0;
    m_cLifeDrain = 0;
    if (m_nClass != 34 && (m_nClass != 21 || m_stLookInfo.FaceMesh != 10))
        m_cDodge = 0;
    m_cHuntersVision = 0;
    m_cOverExp = 0;
    m_cGodCos = 0;
    m_cManaControl = 0;
    m_cImmunity = 0;
    m_cArmorClass = 0;
    m_cCoinArmor = 0;
    m_cElimental = 0;
    m_cCantMove = 0;
    m_cCriticalArmor = 0;
    m_cSoul = 0;
    m_bSkillBlack = 0;
    m_cCantMove = 0;
    m_cCantAttk = 0;
    m_bShield2 = 0;
    SetAvatar(0);

    for (int i = 0; i < 32; ++i)
    {
        switch (m_usAffect[i] >> 8)
        {
        case 1:
            m_cFreeze = 1;
            break;
        case 2:
            m_cHaste = 1;
            break;
        case 3:
            m_cSlowSlash = 1;
            break;
        case 4:
            m_cPowerUp = 1;
            break;
        case 5:
            m_bSkillBlack = 1;
            break;
        case 6:
            m_bShield2 = 1;
            break;
        case 7:
            m_cSpeedDown = 1;
            break;
        case 8:
            if (m_stAffect[i].Value & 1)
                m_DilpunchJewel = 1;
            break;
        case 9:
            m_cWeapon = 1;
            break;
        case 10:
            m_cWaste = 1;
            break;
        case 11:
            m_cShield = 1;
            break;
        case 12:
        case 14:
        case 16:
        case 33:
        case 34:
        case 35:
        case 36:
        case 38:
        case 41:
        case 42:
        case 43:
            continue;
        case 13:
            m_cAssert = 1;
            break;
        case 15:
            m_cSKillAmp = 1;
            break;
        case 17:
            m_cAurora = 1;
            break;
        case 18:
            m_cManaControl = 1;
            break;
        case 19:
            m_cImmunity = 1;
            break;
        case 20:
            m_cPoison = 1;
            break;
        case 21:
            m_cArmorClass = 1;
            break;
        case 22:
            m_cLighten = 1;
            break;
        case 23:
            m_cElimental = 1;
            break;
        case 24:
            m_cCriticalArmor = 1;
            break;
        case 25:
            m_cProtector = 1;
            break;
        case 26:
            m_cDodge = 1;
            break;
        case 27:
            m_cEnchant = 1;
            break;
        case 28:
            m_cShadow = 1;
            break;
        case 29:
            TMHuman::SetAvatar(1);
            break;
        case 30:
            m_cHuntersVision = 1;
            break;
        case 31:
            m_cCoinArmor = 1;
            break;
        case 32:
            m_cCancel = 1;
            break;
        case 37:
            m_cSoul = 1;
            break;
        case 39:
            m_cOverExp = 1;
            break;
        case 40:
            m_cCantMove = 1;
            m_cCantAttk = 1;
            m_bSkillBlack = 1;
            break;
        case 44:
            m_cCantMove = 1;
            break;
        case 45:
            m_cCantMove = 1;
            m_cCantAttk = 1;
            break;
        }
    }

    if (m_pSkinMesh->m_pSwingEffect[0])
        m_pSkinMesh->m_pSwingEffect[0]->m_cArmorClass = m_cArmorClass;
    if (m_pSkinMesh->m_pSwingEffect[1])
        m_pSkinMesh->m_pSwingEffect[1]->m_cArmorClass = m_cArmorClass;
    if (m_pSkinMesh->m_pSwingEffect[0])
        m_pSkinMesh->m_pSwingEffect[0]->m_cAssert = m_cAssert;
    if (m_pSkinMesh->m_pSwingEffect[1])
        m_pSkinMesh->m_pSwingEffect[1]->m_cAssert = m_cAssert;
    if (m_pSkinMesh->m_pSwingEffect[0])
        m_pSkinMesh->m_pSwingEffect[0]->m_bEnchant = m_cEnchant;
    if (m_pSkinMesh->m_pSwingEffect[1])
        m_pSkinMesh->m_pSwingEffect[1]->m_bEnchant = m_cEnchant;
    if (m_cSpeedUp != 1 || m_cSpeedDown)
    {
        if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 0;
        if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 0;
    }
    else
    {
        if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 1;
        if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 1;
    }
    if (m_cWeapon == 1)
    {
        if (m_pSkinMesh->m_pSwingEffect[0] && m_sRightIndex && !m_cHasShield)
            m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 1;
        else if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 0;
        if (m_pSkinMesh->m_pSwingEffect[1] && m_sLeftIndex)
            m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 1;
        else if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 0;
    }
    else
    {
        if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 0;
        if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 0;
    }

    TMFieldScene* pFScene = nullptr;
    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (g_pCurrentScene->m_pMyHuman == this && pFScene)
    {
        pFScene->UpdateScoreUI(0);
        if (!m_cOnlyMove)
            SetSpeed(pFScene->m_bMountDead);
    }
    else if (!m_cOnlyMove)
    {
        SetSpeed(0);
    }
}

void TMHuman::SetSpeed(int bMountDead)
{
    m_fMaxSpeed = (float)BASE_GetSpeed(&m_stScore);
    if (g_pCurrentScene->m_pMyHuman == this)
        g_nMyHumanSpeed = (int)m_fMaxSpeed;
}

/// Controla ataque automatico continuo contra o alvo em foco.
int TMHuman::MAutoAttack(TMHuman* pTarget, int mode)
{
    if (_dwAttackDelay)
    {
        if (g_pTimerManager->GetServerTime() - _dwAttackDelay > 2000)
            _dwAttackDelay = 0;
        return 0;
    }
    if (!pTarget)
        return 0;
    if (pTarget->m_nClass == 44 && pTarget->m_sHeadIndex == 219)
        return 0;
    if (m_cMantua && pTarget->m_cMantua == m_cMantua)
        return 0;
    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
    {
        g_GameAuto = 0;
        return 0;
    }
    if (pTarget->m_cDie == 1)
        return 0;
    if (pTarget->m_cHide == 1)
        return 0;
    if (m_dwDelayDel)
        return 0;
    if (!pTarget)
        return 0;
    if (pTarget->m_dwID > 0 && pTarget->m_dwID < 1000 == 1)
        return 0;
    if (pTarget->IsMerchant())
        return 0;
    if (pTarget->m_cSummons == 1)
        return 0;
    if ((m_stScore.Reserved & 0xF) == 15)
    {
        auto pFocused = g_pCurrentScene->m_pMyHuman;
        if (m_cMantua > 0 && pFocused->m_cMantua > 0 && m_cMantua == pFocused->m_cMantua)
            return 0;
    }

    auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    auto pMobData = &g_pObjectManager->m_stMobData;
    int nSpecForce = 0;
    if (pMobData && pMobData->LearnedSkill[0] & 0x20000000)
        nSpecForce = 1;
    if (dwServerTime < pScene->m_dwOldAttackTime + 1000)
        return 0;

    int nSX = (int)m_vecPosition.x;
    int nSY = (int)m_vecPosition.y;
    int nTX = (int)pTarget->m_vecPosition.x;
    int nTY = (int)pTarget->m_vecPosition.y;
    int nDistance = BASE_GetDistance(nSX, nSY, nTX, nTY);
    int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
    BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)pScene->m_HeightMapData, 8);
    m_cMount;

    static int nMotionIndex = 0;
    ++nMotionIndex;
    nMotionIndex %= 3;

    if (nDistance < 0 || nDistance > 15)
        return 0;
    if (!nTX || !nTY)
        return 0;

    if (nDistance <= nMobAttackRange && 
        nTX == (int)pTarget->m_vecPosition.x && nTY == (int)pTarget->m_vecPosition.y && 
        m_LastSendTargetPos.x == (int)m_vecPosition.x && m_LastSendTargetPos.y == (int)m_vecPosition.y)
    {
        if (pTarget->m_cShadow == 1 && pTarget->m_nClass == 66 && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
            return 0;

        MSG_Attack stAttack{};
        stAttack.Header.Type = MSG_Attack_One_Opcode;
        stAttack.Header.ID = m_dwID;
        stAttack.AttackerID = m_dwID;
        stAttack.PosX = (int)m_vecPosition.x;
        stAttack.PosY = (int)m_vecPosition.y;
        if (pScene->m_stMoveStop.NextX)
        {
            stAttack.PosX = pScene->m_stMoveStop.NextX;
            stAttack.PosY = pScene->m_stMoveStop.NextY;
        }
        stAttack.CurrentMp = -1;
        stAttack.SkillIndex = -1;
        stAttack.SkillParm = 0;
        stAttack.Motion = nMotionIndex + 4;

        if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 101)
            stAttack.SkillIndex = 151;
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
        stAttack.Dam[0].TargetID = pTarget->m_dwID;

        auto pTargetHuman = (TMHuman*)g_pObjectManager->GetHumanByID(stAttack.Dam[0].TargetID);
        if (!pTargetHuman)
            return 0;

        int nCritical = (unsigned char)g_pObjectManager->m_stMobData.Critical;
        stAttack.Dam[0].Damage = -2;
        stAttack.Progress = TMFieldScene::m_usProgress;
        BASE_GetDoubleCritical(&g_pObjectManager->m_stMobData, 0, &TMFieldScene::m_usProgress, &stAttack.DoubleCritical);
        stAttack.TargetX = (int)pTargetHuman->m_vecPosition.x;
        stAttack.TargetY = (int)pTargetHuman->m_vecPosition.y;
        
        int nSize = sizeof(MSG_Attack);
        if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x200000)
        {
            stAttack.Header.Type = MSG_Attack_Two_Opcode;
            nSize = sizeof(MSG_AttackTwo);
        }

        if (pMobData->Class == 3 && pMobData->LearnedSkill[0] & 0x40)
        {
            int nDX = (int)(pTargetHuman->m_vecPosition.x - pScene->m_pMyHuman->m_vecPosition.x);
            int nDY = (int)(pTargetHuman->m_vecPosition.y - pScene->m_pMyHuman->m_vecPosition.y);
            if (nDX > 0)
                nDX = 1;
            else if (nDX < 0)
                nDX = -1;
            if (nDY > 0)
                nDY = 1;
            else if (nDY < 0)
                nDY = -1;

            int TX = nDX + (int)pTargetHuman->m_vecPosition.x;
            int TY = nDY + (int)pTargetHuman->m_vecPosition.y;
            auto pNode = (TMHuman*)pScene->m_pHumanContainer->m_pDown;

            while (pNode->m_pNextLink)
            {
                if (pNode == pScene->m_pMyHuman || pNode == pTargetHuman || (int)pNode->m_vecPosition.x != TX || (int)pNode->m_vecPosition.y != TY)
                {
                    pNode = (TMHuman*)pNode->m_pNextLink;
                    continue;
                }

                if (pNode->m_dwID > 0 && pNode->m_dwID < 1000)
                {
                    if (!pNode->IsInPKZone() || pNode->m_bParty == 1)
                    {
                        pNode = (TMHuman*)pNode->m_pNextLink;
                        continue;
                    }

                    if (!TMFieldScene::m_bPK)
                    {
                        if (!g_bCastleWar)
                        {
                            if (g_pObjectManager->m_usWarGuild != pNode->m_usGuild)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                        }
                        else
                        {
                            if (m_cMantua > 0 && m_cMantua == pNode->m_cMantua)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                            if (m_cMantua == 3 && (pNode->m_dwID > 0 && pNode->m_dwID < 1000) || pNode->m_cMantua > 0 && pNode->m_cMantua != 4)
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }

                            if (!pNode->IsInCastleZone())
                            {
                                pNode = (TMHuman*)pNode->m_pNextLink;
                                continue;
                            }
                        }
                    }
                }
                else
                {
                    if (!TMFieldScene::m_bPK && pNode->m_cSummons == 1 && (g_pObjectManager->m_usWarGuild != pNode->m_usGuild && pNode->m_usGuild) || !pNode->m_usGuild)
                    {
                        pNode = (TMHuman*)pNode->m_pNextLink;
                        continue;
                    }
                } 
                
                stAttack.Dam[1].TargetID = pNode->m_dwID;
                stAttack.Dam[1].Damage = -2;
                break;
            }    

            if (!(pMobData->LearnedSkill[0] & 0x200000) && stAttack.Dam[1].Damage != -2)
            {
                stAttack.Header.Type = MSG_Attack_One_Opcode;
                nSize = sizeof(MSG_AttackOne);
            }
            else
            {
                stAttack.Header.Type = MSG_Attack_Two_Opcode;
                nSize = sizeof(MSG_AttackTwo);
            }
        }

        SendOneMessage((char*)&stAttack, nSize);

        MSG_Attack stAttackLocal{};
        memcpy((char*)&stAttackLocal, (char*)&stAttack, nSize);
        stAttackLocal.Header.ID = pScene->m_dwID;
        stAttackLocal.FlagLocal = 1;
        if (pMobData->LearnedSkill[0] & 0x20000000)
            stAttackLocal.DoubleCritical |= 4;
        pScene->OnPacketEvent(MSG_Attack_One_Opcode, (char*)&stAttackLocal);
        pScene->m_dwOldAttackTime = dwServerTime;
        return 1;
    }

    if (!mode)
        return 0;
    if ((GetKeyState(VK_SHIFT) >> 8) > 0)
        return 0;

    int nMoveSX = (int)m_vecPosition.x;
    int nMoveSY = (int)m_vecPosition.y;

    nTX = (int)pTarget->m_vecPosition.x;
    nTY = (int)pTarget->m_vecPosition.y;

    int nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);

    int PlusX = 1;
    int PlusY = 1;
    if (nSX > nTX)
        PlusX = -1;
    if (nSY > nTY)
        PlusY = -1;

    while (nMoveDistance2 > nMobAttackRange)
    {
        if (nMoveSX != nTX)
            nMoveSX += PlusX;
        if (nMoveSY != nTY)
            nMoveSY += PlusY;
        nMoveDistance2 = BASE_GetDistance(nMoveSX, nMoveSY, nTX, nTY);
    }

    pScene->m_vecMyNext.x = nMoveSX;
    pScene->m_vecMyNext.y = nMoveSY;
    GetRoute(pScene->m_vecMyNext, 0, 0);
    return 2;
}
