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

/// @file TMHuman_Animation.cpp
/// @brief Animacoes, troca de armas/montarias/capas/trajes, materiais e sons de combate.

void TMHuman::SetRace(short sIndex)
{
    if (m_dwDelayDel != 0)
        return;

    STRUCT_ITEM item{};
    item.sIndex = sIndex;
    m_nClass = BASE_GetItemAbility(&item, 18);
    m_nSkinMeshType = BASE_DefineSkinMeshType(m_nClass);

    if(sIndex == 25)
        m_fScale = (((float)m_stScore.Special[3] * 0.003f) + 1.0f) * m_fScale;
}

/// Seleciona e aplica a animacao correspondente ao estado/arma do personagem.
void TMHuman::SetAnimation(ECHAR_MOTION eMotion, int nLoop)
{
    if (!m_dwDelayDel && eMotion != ECHAR_MOTION::ECMOTION_NONE && m_pSkinMesh)
    {
        if (eMotion == ECHAR_MOTION::ECMOTION_LEVELUP && (m_nClass == 34 || m_nClass == 38))
            eMotion = ECHAR_MOTION::ECMOTION_STAND02;

        if ((eMotion == ECHAR_MOTION::ECMOTION_HOLYTOUCH || eMotion == ECHAR_MOTION::ECMOTION_RELAX)
            && (m_nSkinMeshType == 3 || m_nSkinMeshType == 7 || m_nSkinMeshType == 25 || m_nSkinMeshType == 28))
        {
            eMotion = ECHAR_MOTION::ECMOTION_LEVELUP;
        }
        if (m_nSkinMeshType == 8 && eMotion == ECHAR_MOTION::ECMOTION_HOLYTOUCH)
        {
            eMotion = ECHAR_MOTION::ECMOTION_ATTACK01;
            nLoop = 0;
        }
        if (IsInTown() == 1 && eMotion == ECHAR_MOTION::ECMOTION_STAND02 && !m_nWeaponTypeIndex)
            eMotion = ECHAR_MOTION::ECMOTION_STAND01;

        if (eMotion == ECHAR_MOTION::ECMOTION_STAND01 || eMotion == ECHAR_MOTION::ECMOTION_STAND02)
        {
            m_bSliding = 0;
            if (m_cMantua > 0 && m_pMantua)
            {
                if (m_cMount <= 0)
                    m_pMantua->SetAnimation(0);
                else
                    m_pMantua->SetAnimation(3);
            }
        }

        if (m_nSkinMeshType == 31 && (int)eMotion >= 4 && (int)eMotion <= 6)
            *(int*)eMotion += 3;
        if (m_nSkinMeshType == 21 && (int)m_stLookInfo.FaceMesh > 1 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
            eMotion = ECHAR_MOTION::ECMOTION_RUN;
        if (m_nSkinMeshType == 24 && m_bParty == 1 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
            eMotion = ECHAR_MOTION::ECMOTION_RUN;
        if (m_nSkinMeshType == 20 && m_stLookInfo.FaceMesh && m_stLookInfo.HelmMesh != 2 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
            eMotion = ECHAR_MOTION::ECMOTION_WALK;

        if (m_nSkinMeshType == 20 && m_stLookInfo.FaceMesh == 4 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
        {
            m_pSkinMesh->m_dwFPS = (int)(float)(34.0f - m_fMaxSpeed) / 2;
            m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);
        }
        if (m_nSkinMeshType == 20 && (eMotion == ECHAR_MOTION::ECMOTION_WALK || eMotion == ECHAR_MOTION::ECMOTION_RUN) 
            && m_stLookInfo.FaceMesh == 7)
            m_pSkinMesh->m_dwFPS = 6;

        if (m_nSkinMeshType == 0 || m_nSkinMeshType == 1 || m_nSkinMeshType == 2 || m_nSkinMeshType == 3
         || m_nSkinMeshType == 4 || m_nSkinMeshType == 5)
        {
            if (m_cMount == 1)
            {
                if (m_pMount)
                {
                    if (m_nMountSkinMeshType == 20 && (eMotion == ECHAR_MOTION::ECMOTION_WALK || eMotion == ECHAR_MOTION::ECMOTION_RUN) && m_pMount->m_Look.Mesh0 == 7)
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[2]);
                    else if ((signed int)eMotion >= 4 && (signed int)eMotion <= 9)
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[1]);
                    else
                        m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)eMotion]);
                }
                if (m_sHeadIndex < 40 && (m_sHeadIndex % 10 == 1 || m_sHeadIndex % 10 > 5))
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion + 28]]);
                }
                else if (m_nSkinMeshType == 3)
                    m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28]);
                else
                    m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28]]);
            }
            else
            {
                int bExt = 0;
                if (m_sHeadIndex < 40 && (m_sHeadIndex % 10 == 1 || m_sHeadIndex % 10 > 5))
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    if (!m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion]])
                        && !g_MobAniTableEx[nClass][m_nSkinMeshType].dwAniTable[(int)eMotion])
                    {
                        m_eMotion = eMotion;
                        return;
                    }
                }
                else if (!m_pSkinMesh->SetAnimation(MeshManager::m_sAnimationArray[m_nSkinMeshType][m_nWeaponTypeIndex][g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion]])
                    && !g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion])
                {
                    m_eMotion = eMotion;
                    return;
                }
            }

            m_eMotion = eMotion;

            if (eMotion == ECHAR_MOTION::ECMOTION_RUN)
            {
                m_pSkinMesh->m_dwFPS = (int)(27.0f - m_fMaxSpeed) / 2;
                m_pSkinMesh->m_dwFPS = (unsigned int)((float)m_pSkinMesh->m_dwFPS * m_fScale);
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(2);
                    else
                        m_pMantua->SetAnimation(3);
                }

                if (m_cMount == 1 && m_pMount)
                {
                    if (m_nMountSkinMeshType != 40)
                        m_pMount->m_dwFPS = (int)(float)(27.0f - m_fMaxSpeed) / 2;

                    switch (m_nMountSkinMeshType)
                    {
                    case 20:
                        m_pMount->m_dwFPS = 20;
                        break;
                    case 39:
                        m_pMount->m_dwFPS = 20;
                        break;
                    case 48:
                        m_pMount->m_dwFPS = 3;
                        break;
                    case 49:
                        m_pMount->m_dwFPS = 6;
                        break;
                    case 52:
                        m_pMount->m_dwFPS = 6;
                        break;
                    case 50:
                        m_pMount->m_dwFPS = 6;
                        break;
                    }
                }

                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
            }            
            else if (eMotion == ECHAR_MOTION::ECMOTION_WALK)
            {
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(1);
                    else
                        m_pMantua->SetAnimation(3);
                }

                if (m_nSkinMeshType == 2)
                    m_pSkinMesh->m_dwFPS = (signed int)(float)(40.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;
                else
                    m_pSkinMesh->m_dwFPS = (signed int)(float)(36.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);

                if (m_cMount == 1 && m_pMount)
                {
                    m_pMount->m_dwFPS = (signed int)(float)(32.0f - (float)(m_fMaxSpeed * 3.0f)) / 2;
                    m_pMount->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fMountScale);
                }
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;
            }
            else
            {
                if (m_cMantua > 0 && m_pMantua)
                {
                    if (m_cMount <= 0)
                        m_pMantua->SetAnimation(1);
                    else
                        m_pMantua->SetAnimation(3);
                }

                unsigned int dwSpeedTemp = 0;
                if (m_sHeadIndex >= 40 || m_sHeadIndex % 10 != 1 && m_sHeadIndex % 10 <= 5)
                    dwSpeedTemp = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
                else
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    dwSpeedTemp = g_MobAniTableEx[nClass][m_nSkinMeshType].dwSpeed[(int)eMotion];
                }
                              
                m_pSkinMesh->m_dwFPS = (unsigned int)((float)dwSpeedTemp * 1.0f);
                if (m_cMount == 1 && m_pMount)                   
                    m_pMount->m_dwFPS = (unsigned int)((float)g_MobAniTable[m_nMountSkinMeshType].dwSpeed[(int)eMotion] * 1.0f);
            }

            if ((int)eMotion >= 4 && (int)eMotion <= 9)
            {
                float fEffectLen = 1.0f;
                if (m_cMount == 1)
                    fEffectLen = 1.2f;

                unsigned int dwSpeedTemp = 0;
                if (m_sHeadIndex >= 40 || m_sHeadIndex % 10 != 1 && m_sHeadIndex % 10 <= 5)
                    dwSpeedTemp = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
                else
                {
                    int nClass = 0;
                    if (m_sHeadIndex % 10 == 1)
                        nClass = m_sHeadIndex / 10;
                    else
                        nClass = m_sHeadIndex % 10 - 6;
                    if (nClass > 3)
                        nClass = 0;
                    dwSpeedTemp = g_MobAniTableEx[nClass][m_nSkinMeshType].dwSpeed[(int)eMotion];
                }

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)dwSpeedTemp * 1.0f);
                if (m_bSwordShadow[0] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[0])
                    {
                        if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                        else
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * fEffectLen;
                    }
                    if (m_pSkinMesh->m_pSwingEffect[0])
                        m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
                if (m_bSwordShadow[1] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }

                float fSpeed = 1.0f;
                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.0f);
                if (m_cFreeze == 1)
                    m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * 1.15f);
                if (m_nMotionCount > 1)
                    m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.2f);
                else if (m_bDoubleAttack == 1)
                {
                    if (m_nWeaponTypeL == 101)
                        m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 2.0f);
                    else
                        m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS / 1.7f);
                }
            }
            else
            {
                m_nMotionCount = 0;
            }
            if (eMotion == ECHAR_MOTION::ECMOTION_LEVELUP)
            {
                if (m_nSkinMeshType == 2 && m_nWeaponTypeIndex == 4)
                     m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * 1.6f);

                if ((TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject == this
                    && m_cMount == 1
                    && m_nMountSkinMeshType == 31
                    && g_pSoundManager
                    && g_pSoundManager->GetSoundData(279))
                {
                    g_pSoundManager->GetSoundData(279)->Play();
                }
                if ((TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject == this)
                {
                    int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[14];
                    if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                        nSoundIndex = 225;
                    if (g_pSoundManager && g_pSoundManager->GetSoundData(nSoundIndex))
                        g_pSoundManager->GetSoundData(nSoundIndex)->Play();
                }
                else if (g_pSoundManager != nullptr && g_pSoundManager->GetSoundData(158))
                {
                    g_pSoundManager->GetSoundData(158)->Play();
                }
            }
        }    
        else
        {
            int nBaseValue = 30;
            if (m_nSkinMeshType == 31)
                nBaseValue = 38;
            else if (m_nSkinMeshType == 39)
                nBaseValue = 40;
            else if (m_nSkinMeshType == 38)
                nBaseValue = 38;
            else if (m_nSkinMeshType == 30)
                nBaseValue = 40;
            else if (m_nSkinMeshType == 21 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nBaseValue = 24;
            else if (m_nSkinMeshType == 24 && eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nBaseValue = 20;
            else if (m_nSkinMeshType == 22)
                nBaseValue = 26;
            else if (m_nSkinMeshType == 21)
                nBaseValue = 28;
            else if (m_nSkinMeshType == 29 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                nBaseValue = 36;
            else if (m_nSkinMeshType == 7)
                nBaseValue = 34;
            else if (m_nSkinMeshType == 28)
                nBaseValue = 31;
            else if (m_nSkinMeshType == 2)
                nBaseValue = 26;
            else if (m_nSkinMeshType == 11)
                nBaseValue = 50;
            else if (m_nSkinMeshType == 35)
                nBaseValue = 28;
            else if (m_nSkinMeshType == 44)
                nBaseValue = 50;

            if ((signed int)eMotion >= 4 && (signed int)eMotion <= 9)
            {
                if (m_bSwordShadow[0] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[0])
                    {
                        if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * 1.0f;
                        else
                            m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * 1.0f;
                    }
                    if (m_pSkinMesh->m_pSwingEffect[0])
                        m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
                if (m_bSwordShadow[1] == 1)
                {
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * 1.0f;
                    if (m_pSkinMesh->m_pSwingEffect[1])
                        m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = g_pTimerManager->GetServerTime();
                }
            }
            if (m_nSkinMeshType == 37 && eMotion == ECHAR_MOTION::ECMOTION_LEVELUP && g_pSoundManager && g_pSoundManager->GetSoundData(294))
                g_pSoundManager->GetSoundData(294)->Play();

            if (eMotion == ECHAR_MOTION::ECMOTION_RUN || eMotion == ECHAR_MOTION::ECMOTION_WALK)
            {
                m_pSkinMesh->m_dwFPS = (int)((float)nBaseValue - (float)(m_fMaxSpeed * 3.0f)) / 2;
                if (m_nSkinMeshType == 40)
                    m_pSkinMesh->m_dwFPS = 20;
                if (m_nSkinMeshType == 20 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    m_pSkinMesh->m_dwFPS = 20;
                if (m_nSkinMeshType == 39 && eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    m_pSkinMesh->m_dwFPS = 20;

                m_pSkinMesh->m_dwFPS = (unsigned int)(float)((float)m_pSkinMesh->m_dwFPS * m_fScale);
            }
            if (m_cMount == 1)
            {
                if (m_pMount)
                {
                    m_pMount->m_dwFPS = m_pSkinMesh->m_dwFPS;
                    m_pMount->SetAnimation(g_MobAniTable[m_nMountSkinMeshType].dwAniTable[(int)eMotion]);
                }

                m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion + 28 * m_cMount]);
            }
            else
            {
                if (m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion]) == 0)
                {
                    if (!g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion])
                        m_eMotion = eMotion;               
                    return;
                }
            }

            m_eMotion = eMotion;
            m_pSkinMesh->m_dwFPS = g_MobAniTable[m_nSkinMeshType].dwSpeed[(int)eMotion];
            if (m_cMount == 1 && m_pMount)
                m_pMount->m_dwFPS = g_MobAniTable[m_nMountSkinMeshType].dwSpeed[(int)eMotion];
        }

        m_nLoop = nLoop;
        m_dwStartAnimationTime = g_pTimerManager->GetServerTime();

        if (m_cPunish == 1)
            m_dwLastDummyTime = g_pTimerManager->GetServerTime();
    }
}

void TMHuman::SetColorMaterial()
{
    if (m_dwDelayDel)
        return;

    TMScene* pScene = g_pCurrentScene;
    if (!m_pSkinMesh)
        return;

    float fAlpha = 1.0f;

    if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
    {
        TMHuman *pFocused = (TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject;

        D3DCOLORVALUE color = pScene->GroundGetColor(m_vecPosition);
        bool bLight = false;
        TMSky* pSky = g_pCurrentScene->m_pSky;

        if (pSky->m_nState && pSky->m_nState != 2 && pSky->m_nState != 12)
        {
            for (int i = 0; i < 2; ++i)
            {
                TMObjectContainer* pContainer = g_pCurrentScene->m_pObjectContainerList[i];
                if (pContainer == nullptr)
                    continue;

                int nLightIndex = pContainer->m_nLightIndex;
                for (int j = 0; j < nLightIndex; ++j)
                {
                    if (pContainer->m_pLightContainer[j]->m_bEnable != 1)
                        continue;

                    float fLen = pContainer->m_pLightContainer[j]->m_vecPosition.DistanceFrom(m_vecPosition);

                    if (fLen >= 5.0f)
                        continue;

                    color.r = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.r)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.r * 0.2f);
                    color.g = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.g)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.g * 0.2f);
                    color.b = (((5.0f - fLen) * pContainer->m_pLightContainer[j]->m_Light.Ambient.b)
                        / 3.0f)
                        + (g_pDevice->m_colorLight.b * 0.2f);
                    bLight = 1;
                    break;
                }
            }
        }

        if (!bLight)
        {
            color.r = (color.r * 0.5f) + (g_pDevice->m_colorLight.r * 0.5f);
            color.g = (color.g * 0.5f) + (g_pDevice->m_colorLight.g * 0.5f);
            color.b = (color.b * 0.5f) + (g_pDevice->m_colorLight.b * 0.5f);
            color.a = fAlpha;
        }

        m_pSkinMesh->m_materials.Specular.a = fAlpha;
        m_pSkinMesh->m_materials.Emissive.a = fAlpha;
        m_pSkinMesh->m_materials.Diffuse = color;
        m_pSkinMesh->m_materials.Specular.r = 1.0f;
        m_pSkinMesh->m_materials.Specular.g = 1.0f;
        m_pSkinMesh->m_materials.Specular.b = 1.0f;

        if (g_pCurrentScene->m_pMouseOverHuman == this && pFocused != this)
        {
            if ((m_dwID >= 0 && m_dwID < 1000) || (m_stScore.Reserved & 0xF) != 15)
            {
                if (m_dwID < 0 || m_dwID >= 1000 && IsMerchant())
                    m_dwEdgeColor = 0x8800FF00;
                else if (m_TradeDesc[0])
                    m_dwEdgeColor = 0x8800FF00;
                else if (m_bParty == 1)
                    m_dwEdgeColor = 0x88FF00FF;
                else if (m_usGuild && (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                    m_dwEdgeColor = 0x8800FFFF;
                else
                {
                    int bTown = IsInTown();
                    if (bTown || m_pProgressBar->m_GCProgress.dwColor != 0xFFFF0000)
                        m_dwEdgeColor = 0x88FFFFFF;
                    else
                        m_dwEdgeColor = 0x88FF0000;
                }
            }
            else
            {
                m_dwEdgeColor = 0x8800FF00;
            }

            if ((m_stScore.Reserved & 0xF) == 15 && pFocused->m_cMantua && pFocused->m_cMantua != 3 && m_pMantua && pFocused->m_pMantua
                && m_cMantua != pFocused->m_cMantua)
            {
                m_dwEdgeColor = 0x88FF0000;
            }
        }
        else
        {
            if (!g_bHideSkillBuffEffect && m_cHaste == 1)
            {
                unsigned int dwServerTime = g_pTimerManager->GetServerTime();
                if (m_eMotion == ECHAR_MOTION::ECMOTION_RUN && dwServerTime - m_dwLastHaste > (unsigned int)(600.0f / m_fMaxSpeed))
                {
                    TMEffectBillBoard2* pBill2 = new TMEffectBillBoard2(43, 1000, 0.5f, 0.5f, 0.5f, 0.00050000002f, 0);

                    if (pBill2)
                    {
                        pBill2->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight + 0.2f, m_vecPosition.y);
                        pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                        m_dwLastHaste = dwServerTime;
                    }
                }
            }
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
            {
                color.a = fAlpha;
                color.r = 0.6f;
                color.g = 0.4f;
                color.b = 0.4f;
                m_pSkinMesh->m_materials.Emissive.r = (g_pDevice->m_colorBackLight.r * 0.4f)
                    + (0.6f * 0.5f);
                m_pSkinMesh->m_materials.Emissive.g = (g_pDevice->m_colorBackLight.g * 0.4f)
                    + (color.r * 0.5f);
                m_pSkinMesh->m_materials.Emissive.b = (g_pDevice->m_colorBackLight.b * 0.4f)
                    + (color.r * 0.5f);
                return;
            }
            if (m_cCancel == 1)
            {
                color.a = fAlpha;
                color.r = (color.r * 0.4f) + 0.2f;
                color.g = color.g * 0.4f;
                color.b = color.b * 0.4f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if (m_cFreeze == 1)
            {
                color.a = fAlpha;
                color.r = 0.0f;
                color.g = 0.4f;
                color.b = 0.9f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if (m_cPoison == 1)
            {
                color.a = fAlpha;
                color.r = 0.0f;
                color.g = 0.3f;
                color.b = 0.0f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;

                color.g = 0.8f;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else if(m_cSlowSlash == 1)
            {
                float fVel = (float)((g_pTimerManager->GetServerTime() + (300 * m_dwID)) % 1000) / 1000.0f;
                float fR = (sinf((fVel * 3.1415927f) * 2.0f) * 0.5f) + 0.5f;
                color.a = fAlpha;
                color.r = fR / 2.0f;
                color.g = fR / 2.0f;
                color.b = 0.0f;

                m_pSkinMesh->m_materials.Emissive = color;
                m_pSkinMesh->m_materials.Specular = color;
                m_pSkinMesh->m_materials.Diffuse = color;

                if (m_cMount > 0 && m_pMount)
                    memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
            }
            else
            {
                m_pSkinMesh->m_materials.Emissive.r = (g_pDevice->m_colorBackLight.r * 0.3f) + (color.r * 0.4f);
                m_pSkinMesh->m_materials.Emissive.g = (g_pDevice->m_colorBackLight.g * 0.3f) + (color.r * 0.4f);
                m_pSkinMesh->m_materials.Emissive.b = (g_pDevice->m_colorBackLight.b * 0.3f) + (color.r * 0.4f);

                if (m_nClass == 32 && m_stLookInfo.FaceMesh == 2)
                {
                    m_pSkinMesh->m_materials.Emissive.r = 0.65f;
                    m_pSkinMesh->m_materials.Emissive.g = 0.0;
                    m_pSkinMesh->m_materials.Emissive.b = 0.0;
                }
            }
        }
    }
    else
    {
        if (m_bSelected)
        {
            m_pSkinMesh->m_materials.Diffuse.r = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.g = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.b = 1.0f;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 1.0f;
            m_pSkinMesh->m_materials.Emissive.g = 1.0f;
            m_pSkinMesh->m_materials.Emissive.b = 1.0f;
        }
        else if (m_bMouseOver == 1)
        {
            m_pSkinMesh->m_materials.Diffuse.r = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.g = 1.0f;
            m_pSkinMesh->m_materials.Diffuse.b = 1.0f;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 0.3f;
            m_pSkinMesh->m_materials.Emissive.g = 0.5f;
            m_pSkinMesh->m_materials.Emissive.b = 0.0;
        }
        else
        {
            D3DCOLORVALUE color{};
            color.r = (g_pDevice->m_colorLight.r * 0.30000001f) + 0.2f;
            color.g = (g_pDevice->m_colorLight.g * 0.30000001f) + 0.2f;
            color.b = (g_pDevice->m_colorLight.b * 0.30000001f) + 0.2f;

            m_pSkinMesh->m_materials.Diffuse = color;
            m_pSkinMesh->m_materials.Emissive = color;
            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
        }
    }

    if (m_cMantua > 0 && m_pMantua)
        memcpy(&m_pMantua->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMantua->m_materials));
    if (m_cMount > 0 && m_pMount)
        memcpy(&m_pMount->m_materials, &m_pSkinMesh->m_materials, sizeof(m_pMount->m_materials));
}

/// Avanca quadros da animacao atual disparando eventos de passo, golpe e som.
void TMHuman::AnimationFrame(int nWalkSndIndex)
{
    if (m_dwDelayDel || !m_pSkinMesh)
        return;

    if (m_pShade)
    {
        if (m_cHide == 1)
            m_pShade->m_bShow = 0;
        if (m_cShadow == 1)
            m_pShade->m_bShow = 0;
        if (m_nClass == 45)
            m_pShade->m_bShow = 0;
    }

    if (m_nClass != 45 && (m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_eMotion == ECHAR_MOTION::ECMOTION_RUN))
    {
        float fWaterHeight = 0.0f;

        if (g_pCurrentScene->GroundIsInWater(m_vecPosition, m_fHeight, &fWaterHeight) == 1)
        {
            nWalkSndIndex = 4;
            unsigned int dwServerTime = g_pTimerManager->GetServerTime();
            unsigned int nWaterTime = 80;
            if (m_eMotion == ECHAR_MOTION::ECMOTION_WALK)
                nWaterTime = 120;
            if ((dwServerTime - m_dwWaterTime) > nWaterTime)
            {
                float fSpeed = (m_fScale * TMHuman::m_vecPickSize[m_nSkinMeshType].x) * 0.0020000001f;

                auto pEffect = new TMEffectBillBoard2(10, 700, 0.5f, 0.5f, 0.5f, fSpeed, 0);
                if (pEffect)
                {
                    pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pEffect->m_vecPosition.x = m_vecPosition.x;
                    pEffect->m_vecPosition.z = m_vecPosition.y;
                    pEffect->m_vecPosition.y = fWaterHeight + 0.25f;
                    g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
                }
                m_dwWaterTime = dwServerTime;
            }
        }

        unsigned int dwOffset = m_pSkinMesh->m_dwOffset;
        int nSkinMeshType = m_nSkinMeshType;
        int nHeadMesh = m_stLookInfo.HelmMesh;
        if (m_cMount == 1 && m_pMount)
        {
            dwOffset = m_pMount->m_dwOffset;
            nSkinMeshType = m_nMountSkinMeshType;
            nHeadMesh = m_stMountLook.Mesh0;
        }

        if (nWalkSndIndex == 8 &&
            (int)m_vecPosition.x >> 7 > 26 &&
            (int)m_vecPosition.x >> 7 < 31 &&
            (int)m_vecPosition.y >> 7 > 20 &&
            (int)m_vecPosition.y >> 7 < 25)
        {
            nWalkSndIndex = 83;
        }

        if (nWalkSndIndex == 9 &&
            (int)m_vecPosition.x >> 7 > 26 &&
            (int)m_vecPosition.x >> 7 < 31 &&
            (int)m_vecPosition.y >> 7 > 20 &&
            (int)m_vecPosition.y >> 7 < 25)
        {
            nWalkSndIndex = 82;
        }

        if (m_cHide || nWalkSndIndex == 4 || nSkinMeshType != 31 || this != static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
        {
            if (!m_cHide)
            {
                if (dwOffset >= 2)
                {
                    if (dwOffset >= 7 && dwOffset < 9)
                    {
                        if (g_pSoundManager == nullptr)
                            return;

                        auto pSoundData = g_pSoundManager->GetSoundData(2 * nWalkSndIndex + 183);

                        if (pSoundData == nullptr || pSoundData->IsSoundPlaying())
                            return;

                        if (nSkinMeshType == 20 && (!nHeadMesh || nHeadMesh == 2) && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                        }
                        else if (nSkinMeshType != 39 || m_eMotion != ECHAR_MOTION::ECMOTION_RUN)
                        {
                            if (m_nClass == 33)
                            {
                                GetSoundAndPlayIfNot(148, 0, 0);
                            }
                            else if (nSkinMeshType == 40)
                            {
                                GetSoundAndPlayIfNot(g_MobAniTable[40].dwSoundTable[3], 0, 0);
                            }
                            else if (this == static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
                            {
                                if (nWalkSndIndex == 9)
                                    nWalkSndIndex = 8;

                                pSoundData->Play(0, 0);
                            }
                        }
                        else
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                        }
                    }
                }
                else if (g_pSoundManager != nullptr)
                {
                    auto pSoundData = g_pSoundManager->GetSoundData(2 * nWalkSndIndex + 182);

                    if (pSoundData == nullptr || pSoundData->IsSoundPlaying())
                        return;

                    if (nSkinMeshType == 20 && (!nHeadMesh || nHeadMesh == 2) && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                    {
                        GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                    }
                    else if (nSkinMeshType != 39 || m_eMotion != ECHAR_MOTION::ECMOTION_RUN)
                    {
                        if (m_nClass == 33)
                        {
                            GetSoundAndPlayIfNot(148, 0, 0);
                        }
                        else if (nSkinMeshType == 40)
                        {
                            GetSoundAndPlayIfNot(g_MobAniTable[40].dwSoundTable[3], 0, 0);
                        }
                        else if (this == static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject))
                        {
                            if (nWalkSndIndex == 9)
                                nWalkSndIndex = 8;

                            pSoundData->Play(0, 0);
                        }
                    }
                    else
                    {
                        GetSoundAndPlayIfNot(g_MobAniTable[nSkinMeshType].dwSoundTable[3], 0, 0);
                    }
                }
            }
        }
        else
        {
            if (dwOffset < 2)
            {
                int nSoundIndex = 192;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 184;
                    break;
                case 8:
                    nSoundIndex = 198;
                    break;
                case 82:
                    nSoundIndex = 354;
                    break;
                case 83:
                    nSoundIndex = 350;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 5 && dwOffset < 7)
            {
                int nSoundIndex = 193;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 185;
                    break;
                case 8:
                    nSoundIndex = 199;
                    break;
                case 82:
                    nSoundIndex = 355;
                    break;
                case 83:
                    nSoundIndex = 351;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 11 && dwOffset < 13)
            {
                int nSoundIndex = 194;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 196;
                    break;
                case 8:
                    nSoundIndex = 180;
                    break;
                case 82:
                    nSoundIndex = 356;
                    break;
                case 83:
                    nSoundIndex = 352;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }

            if (dwOffset >= 13 && dwOffset < 15)
            {
                int nSoundIndex = 195;
                switch (nWalkSndIndex)
                {
                case 1:
                    nSoundIndex = 197;
                    break;
                case 8:
                    nSoundIndex = 181;
                    break;
                case 82:
                    nSoundIndex = 357;
                    break;
                case 83:
                    nSoundIndex = 353;
                    break;
                }
                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }
        return;
    }

    if ((int)m_eMotion < 4 || (int)m_eMotion > 9)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_STRIKE)
        {
            if (m_pSkinMesh->m_dwOffset < 2)
            {
                int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[10];
                if (m_nSkinMeshType == 2 && m_nClass == 25)
                    nSoundIndex += 10;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                    nSoundIndex = 223;
                else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 21 && m_stLookInfo.FaceMesh >= 2)
                    nSoundIndex += 171;
                else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                    nSoundIndex = 208;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                    nSoundIndex = 261;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                    nSoundIndex = 261;
                else if (m_nClass == 35)
                    nSoundIndex -= 89;
                else if (m_nClass == 34)
                    nSoundIndex = 258;
                else if (m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex = 265;
                else if (m_nClass == 60)
                    nSoundIndex = 364;
                if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15)
                    nSoundIndex = 312;

                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }
        else if (m_eMotion == ECHAR_MOTION::ECMOTION_DIE && m_pSkinMesh->m_dwOffset >= 5 && m_pSkinMesh->m_dwOffset < 7)
        {
            int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[11];
            if (m_nSkinMeshType == 2 && m_nClass == 25)
                nSoundIndex += 10;
            else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                nSoundIndex = 224;
            else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                nSoundIndex += 4;
            else if (m_nSkinMeshType == 21 && (signed int)m_stLookInfo.FaceMesh >= 2)
                nSoundIndex += 171;
            else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                nSoundIndex += 4;
            else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                nSoundIndex = 209;
            else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                nSoundIndex = 262;
            else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                nSoundIndex = 263;
            else if (m_nClass == 34)
                nSoundIndex = 259;
            else if (m_nClass == 35)
                nSoundIndex -= 89;
            else if (m_nClass == 36 || m_nClass == 37)
                nSoundIndex = 264;
            else if (m_nClass == 39 || m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                nSoundIndex = 268;
            else if (m_nClass == 60)
                nSoundIndex = 365;
            if (m_nClass == 4 && m_stLookInfo.FaceMesh == 15)
                nSoundIndex = 299;

            GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
        }
    }
    else
    {
        if (m_pSkinMesh->m_dwOffset >= 6 && m_pSkinMesh->m_dwOffset < 8)
        {
            if (m_nSkinMeshType == 4 || m_nSkinMeshType == 2)
                PlayAttackSound(m_eMotion, 0);
            if (!m_nSkinMeshType || m_nSkinMeshType == 1)
            {
                if (m_nClass != 60)
                    PlayAttackSound(m_eMotion, 0);
                else
                    GetSoundAndPlayIfNot(363, 0, 0);
            }
            else
            {
                int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[static_cast<int>(m_eMotion)];
                if (m_nSkinMeshType == 2 && m_nClass == 25)
                    nSoundIndex += 10;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 4)
                    nSoundIndex = 226;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 5 || m_stLookInfo.FaceMesh == 6))
                    nSoundIndex = 222;
                else if (m_nSkinMeshType == 4 && m_stLookInfo.FaceMesh == 3)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 21 && (signed int)m_stLookInfo.FaceMesh >= 2)
                    nSoundIndex += 171;
                else if (m_nSkinMeshType == 3 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex += 4;
                else if (m_nSkinMeshType == 20 && (m_stLookInfo.FaceMesh == 4 || m_stLookInfo.FaceMesh == 7) && m_fScale < 0.60000002f)
                    nSoundIndex = 208;
                else if (m_nSkinMeshType == 2 && (m_stLookInfo.FaceMesh == 7 || m_stLookInfo.FaceMesh == 9))
                    nSoundIndex = 201;
                else if (m_nSkinMeshType == 2 && m_stLookInfo.FaceMesh == 8)
                    nSoundIndex = 201;
                else if (m_nClass == 35)
                    nSoundIndex -= 89;
                else if (m_nClass == 30 && m_stLookInfo.FaceMesh == 4 || m_nClass == 33 && m_stLookInfo.FaceMesh == 1)
                    nSoundIndex = 265;

                GetSoundAndPlayIfNot(nSoundIndex, 0, 0);
            }
        }

        if (m_pSkinMesh->m_dwOffset >= 0xD && m_pSkinMesh->m_dwOffset < 0xF && (!m_nSkinMeshType ||
            m_nSkinMeshType == 1 ||
            m_nSkinMeshType == 3 ||
            m_nSkinMeshType == 4 ||
            m_nSkinMeshType == 2))
        {
            PlayAttackSound(m_eMotion, 1);
        }

        if (m_pSkinMesh->m_dwOffset >= 8 && m_pSkinMesh->m_dwOffset < 11)
        {
            if (m_nClass == 68 && (m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02 || m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK05))
            {
                for (int i = -3; i < 3; ++i)
                {
                    auto pBillEffect = new TMEffectBillBoard(193, 700, 0.5f, 1.0f, 0.5f, 0.0049999999f, 1, 80);;

                    if (pBillEffect != nullptr)
                    {
                        pBillEffect->m_bStickGround = i % 2;
                        pBillEffect->m_vecPosition = TMVector3{ ((float)i * 0.5f) + m_vecPosition.x, m_fHeight, ((float)i * 0.02f) + m_vecPosition.y };
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBillEffect);
                    }

                    auto pBillEffect2 = new TMEffectBillBoard(193, 700, 0.5f, 1.0f, 0.5f, 0.0049999999f, 1, 80);

                    if (pBillEffect2 != nullptr)
                    {
                        pBillEffect2->m_bStickGround = i % 2;
                        pBillEffect2->m_vecPosition = TMVector3{ m_vecPosition.x - ((float)i * 0.5f), m_fHeight, m_vecPosition.y - ((float)i * 0.02f) };
                        g_pCurrentScene->m_pEffectContainer->AddChild(pBillEffect2);
                    }
                }
            }
        }

        if (m_nClass == 2 || (int)m_eMotion < 7)
        {
            STRUCT_ITEM itemL{};
            itemL.sIndex = m_sLeftIndex;

            int nWeaponTypeL = m_nWeaponTypeL;
            if (nWeaponTypeL == 1 && g_pItemList[m_sLeftIndex].nReqLvl > 90)
                nWeaponTypeL = 2;

            PlayPunchedSound(nWeaponTypeL, 0);

            if (!g_pCurrentScene->m_pMyHuman || BASE_GetDistance(
                (int)g_pCurrentScene->m_pMyHuman->m_vecPosition.x,
                (int)g_pCurrentScene->m_pMyHuman->m_vecPosition.y,
                (int)m_vecAttTargetPos.x,
                (int)m_vecAttTargetPos.y) <= 20)
            {
                if (nWeaponTypeL != 101 && nWeaponTypeL != 102 && nWeaponTypeL != 103)
                {
                    float fWeaponLen = 1.0f;
                    float fLevel = (float)m_stScore.Level / 300.0f;
                    if (fLevel > 1.0f)
                        fLevel = 1.0f;
                    TMVector3 vec{ m_vecAttTargetPos.x, (m_fHeight + 0.89999998f) + (0.30000001f * fLevel), m_vecAttTargetPos.y };

                    if (m_dwAttackEffectTime && g_pTimerManager->GetServerTime() > (m_dwAttackEffectTime + 100))
                    {
                        D3DXVECTOR3 vecAxis{ 0.0f, 0.0f, -1.0f };
                        D3DXVECTOR3 vecMyToTarget{ m_vecAttTargetPos.x - m_vecPosition.x, 0.0f, m_vecAttTargetPos.y - m_vecPosition.y };
                        D3DXVec3Normalize(&vecMyToTarget, &vecMyToTarget);
                        D3DXVECTOR3 vecCross;
                        D3DXVec3Cross(&vecCross, &vecAxis, &vecMyToTarget);

                        float fDot = -D3DXVec3Dot(&vecAxis, &vecMyToTarget);
                        float fAngle = ((fDot * 0.5f) + 0.5f) * 3.1415927f;

                        if (vecCross.y < 0.0f)
                            fAngle = -fAngle;

                        unsigned int dwCenterColor = 0x80FFFFFF;
                        unsigned int dwOtherColor = 0x52A9E5;
                        unsigned int dwLightColor = 0x334388;

                        switch (m_stSancInfo.Legend7)
                        {
                        case 8:
                            dwCenterColor = 0x80FFCCCC;
                            dwOtherColor = 0xE57777;
                            dwLightColor = 0x883333;
                            break;
                        case 7:
                            dwCenterColor = 0x80FFCCFF;
                            dwOtherColor = 0xCC88CC;
                            dwLightColor = 0x884388;
                            break;
                        case 6:
                            dwCenterColor = 0x80CCFFCC;
                            dwOtherColor = 0x88E588;
                            dwLightColor = 0x338843;
                            break;
                        case 5:
                            dwCenterColor = 0x80CCCCFF;
                            dwOtherColor = 0x5253E5;
                            dwLightColor = 0x222288;
                            break;
                        }

                        TMFieldScene* pScene{};
                        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                            pScene = static_cast<TMFieldScene*>(g_pCurrentScene);

                        auto pMeshEffect = new TMEffectMesh(531, dwCenterColor, m_fAngle, 4);
                        if (pMeshEffect)
                        {
                            if (m_fTargetHeight > 1.5f && pScene && this != pScene->m_pMyHuman)
                            {
                                pMeshEffect->m_fScaleH = 1.8f * m_fTargetHeight;
                                pMeshEffect->m_fScaleV = 1.8f * m_fTargetHeight;
                                pMeshEffect->m_dwLifeTime = 200;
                            }
                            else
                            {
                                pMeshEffect->m_fScaleH = 2.0f * m_fTargetHeight;
                                pMeshEffect->m_fScaleV = 2.0f * m_fTargetHeight;
                                pMeshEffect->m_dwLifeTime = 200;
                            }
                            pMeshEffect->m_nTextureIndex = 229;
                            pMeshEffect->m_dwCycleTime = 200;
                            pMeshEffect->m_vecPosition = vec;
                            pMeshEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pMeshEffect->m_cShine = 0;
                            g_pCurrentScene->m_pEffectContainer->AddChild(pMeshEffect);
                        }

                        if (m_fTargetHeight <= 1.5f || m_nClass != 56)
                        {
                            auto pLightMap = new TMShade(7, 118, 1.0f);
                            if (pLightMap)
                            {
                                if (m_bCritical)
                                    pLightMap->SetColor(0x883333);
                                else
                                    pLightMap->SetColor(dwLightColor);

                                pLightMap->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                pLightMap->SetPosition(TMVector2{ (vecMyToTarget.x * 0.5f) + vec.x, (vecMyToTarget.z * 0.5f) + vec.z });
                                pLightMap->m_dwLifeTime = 250;
                                g_pCurrentScene->m_pEffectContainer->AddChild(pLightMap);
                            }
                        }

                        auto vecDir = TMVector3{ vecMyToTarget.x, vecMyToTarget.y, vecMyToTarget.z };

                        if (!g_bHideEffect)
                        {
                            TMHuman* pAttackDest = g_pObjectManager->GetHumanByID(m_nAttackDestID);
                            if (pAttackDest)
                            {
                                pAttackDest->m_bPunchEffect = 1;
                                pAttackDest->m_dwPunchEffectTime = g_pTimerManager->GetServerTime();
                            }

                            TMEffectParticle* pParticle{};

                            if (m_fTargetHeight <= 1.5f || m_nClass != 56)
                            {
                                if (m_cAvatar == 1)
                                {
                                    if (m_bCritical)
                                    {
                                        pParticle = new TMEffectParticle(vec + (vecDir * 0.30000001f), 5, 20, 1.0f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                                    }
                                }
                                else if (m_bCritical)
                                {
                                    pParticle = new TMEffectParticle(vec + (vecDir * 0.30000001f), 5, 10, 0.80000001f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                                }

                                if (pParticle)
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);
                            }

                            if (m_bCritical)
                            {
                                TMEffectBillBoard* pBill1{};

                                TMVector3 vecTargetPos = vec;

                                if (m_fTargetHeight <= 1.5f)
                                {
                                    pBill1 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }
                                else
                                {
                                    pBill1 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }

                                if (pBill1)
                                {
                                    pBill1->m_bLookCam = 0;
                                    pBill1->m_vecPosition = vecTargetPos + (vecDir * 0.5f);
                                    pBill1->m_vecRotAxis = vecDir;
                                    pBill1->m_fAxisAngle = (m_fAngle + 1.5707964f) + 0.050000001f;
                                    pBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pBill1->m_nParticleType = 15;
                                    pBill1->m_fScaleVelX = 0.003f;
                                    pBill1->m_fScaleVelY = 0.003f;
                                    pBill1->m_fScaleVelZ = 0.003f;
                                    pBill1->SetColor(0x883333);
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill1);
                                }

                                TMEffectBillBoard* pBill2{};

                                if (m_fTargetHeight <= 1.5f)
                                {
                                    pBill2 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        4.0f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);
                                }
                                else
                                {
                                    pBill2 = new TMEffectBillBoard(
                                        230,
                                        300,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        1.3f * m_fTargetHeight,
                                        0.00050000002f,
                                        1,
                                        80);;
                                }

                                if (pBill2)
                                {
                                    pBill2->m_bLookCam = 0;
                                    pBill2->m_vecPosition = vecTargetPos + (vecDir * 0.5f);
                                    pBill2->m_vecRotAxis = vecDir;
                                    pBill2->m_fAxisAngle = (m_fAngle + 1.5707964f) - 0.050000001f;
                                    pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                                    pBill2->m_nParticleType = 15;
                                    pBill2->m_fScaleVelX = 0.0049999999f;
                                    pBill2->m_fScaleVelY = 0.0049999999f;
                                    pBill2->m_fScaleVelZ = 0.0049999999f;
                                    pBill2->SetColor(0x883333);
                                    g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                                }
                            }
                        }
                    }
                    m_dwAttackEffectTime = g_pTimerManager->GetServerTime();
                }
            }
        }

        if (m_pSkinMesh->m_dwOffset >= 0xF && m_pSkinMesh->m_dwOffset < 0x11 && (m_nClass == 2 || (int)m_eMotion < 7))
        {
            STRUCT_ITEM itemL{};
            STRUCT_ITEM itemR{};
            itemL.sIndex = m_sLeftIndex;
            itemR.sIndex = m_sRightIndex;
            int nWeaponTypeR = m_nWeaponTypeR;
            int nWeaponPosL = BASE_GetItemAbility(&itemL, EF_POS);
            int nWeaponPosR = BASE_GetItemAbility(&itemR, EF_POS);

            if (nWeaponPosR != 0 && nWeaponPosR != 128 || !nWeaponPosR && !nWeaponPosL)
            {
                if (nWeaponTypeR == 1 && g_pItemList[m_sRightIndex].nReqLvl > 90)
                    nWeaponTypeR = 2;
                PlayPunchedSound(nWeaponTypeR, 1);

                float fLevel = (float)m_stScore.Level / 300.0f;
                if (fLevel > 1.0f)
                    fLevel = 1.0f;

                unsigned int dwColor = 0x80FFFFFF;
                unsigned int dwOtherColor = 0x52A9E5;
                unsigned int dwLightColor = 0x334388;
                switch (m_stSancInfo.Legend7)
                {
                case 8:
                    dwColor = 0x80FFCCCC;
                    dwOtherColor = 0xE57777;
                    dwLightColor = 0x883333;
                    break;
                case 7:
                    dwColor = 0x80FFCCFF;
                    dwOtherColor = 0xCC88CC;
                    dwLightColor = 0x884388;
                    break;
                case 6:
                    dwColor = 0x80CCFFCC;
                    dwOtherColor = 0x88E588;
                    dwLightColor = 0x338843;
                    break;
                case 5:
                    dwColor = 0x80CCCCFF;
                    dwOtherColor = 0x5253E5;
                    dwLightColor = 0x222288;
                    break;
                }

                TMVector3 other{ m_vecAttTargetPos.x, (m_fHeight + 0.89999998f) + (0.30000001f * fLevel), m_vecAttTargetPos.y };

                if (m_dwAttackEffectTime && g_pTimerManager->GetServerTime() > (m_dwAttackEffectTime + 100))
                {
                    D3DXVECTOR3 pV1{ 0.0f, 0.0f, -1.0f };
                    D3DXVECTOR3 pV2{
                        m_vecAttTargetPos.x - m_vecPosition.x,
                        0.0f,
                        m_vecAttTargetPos.y - m_vecPosition.y };

                    D3DXVec3Normalize(&pV2, &pV2);
                    D3DXVECTOR3 pOut;
                    D3DXVec3Cross(&pOut, &pV1, &pV2);

                    auto pChild = new TMEffectMesh(531, dwColor, m_fAngle, 4);
                    if (pChild)
                    {
                        pChild->m_nTextureIndex = 229;
                        pChild->m_dwLifeTime = 150;
                        pChild->m_dwCycleTime = 500;
                        pChild->m_vecPosition = other;
                        pChild->m_fScaleH = 1.3f * m_fTargetHeight;
                        pChild->m_fScaleV = 1.3f * m_fTargetHeight;
                        pChild->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                        pChild->m_cShine = 0;
                        g_pCurrentScene->m_pEffectContainer->AddChild(pChild);

                        auto pShade = new TMShade(7, 118, 1.0f);
                        if (pShade)
                        {
                            if (m_bCritical)
                                pShade->SetColor(0x883333);
                            else
                                pShade->SetColor(dwLightColor);

                            pShade->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pShade->SetPosition(TMVector2{ (pV2.x * 0.5f) + other.x, (pV2.z * 0.5f) + other.z });
                            pShade->m_dwLifeTime = 500;
                            g_pCurrentScene->m_pEffectContainer->AddChild(pShade);
                        }
                    }

                    if (!g_bHideEffect)
                    {
                        TMVector3 vecDir{ pV2.x, pV2.y, pV2.z };

                        TMEffectParticle* pParticle{};

                        if (m_bCritical)
                        {
                            pParticle = new TMEffectParticle(other + (vecDir * 0.30000001f), 5, 10, 1.3f, 0x883333, 0, 231, 1.0f, 1, vecDir, 300);
                        }
                        else
                        {
                            pParticle = new TMEffectParticle(other + (vecDir * 0.30000001f), 5, 5, 0.1f, 0xFFEEAA, 0, 231, 1.0f, 1, vecDir, 800);
                        }

                        if (pParticle)
                            g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);

                        auto pBill1 = new TMEffectBillBoard(
                            230,
                            300,
                            4.0f * m_fTargetHeight,
                            4.0f * m_fTargetHeight,
                            4.5f * m_fTargetHeight,
                            0.00050000002f,
                            1,
                            80);

                        if (pBill1)
                        {
                            pBill1->m_bLookCam = 0;

                            pBill1->m_vecPosition = other + (vecDir * 0.5f);
                            pBill1->m_vecRotAxis = vecDir;
                            pBill1->m_fAxisAngle = (m_fAngle + 1.5707964f) + 0.050000001f;
                            pBill1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pBill1->m_nParticleType = 14;
                            pBill1->m_fScaleVelX = 0.0049999999f;
                            pBill1->m_fScaleVelY = 0.0049999999f;
                            pBill1->m_fScaleVelZ = 0.0049999999f;
                            if (m_bCritical)
                                pBill1->SetColor(0x883333);
                            else
                                pBill1->SetColor(dwLightColor);
                            g_pCurrentScene->m_pEffectContainer->AddChild(pBill1);
                        }

                        auto pBill2 = new TMEffectBillBoard(
                            230,
                            300,
                            4.0f * m_fTargetHeight,
                            4.0f * m_fTargetHeight,
                            4.5f * m_fTargetHeight,
                            0.00050000002f,
                            1,
                            80);

                        if (pBill2)
                        {
                            pBill2->m_bLookCam = 0;

                            pBill2->m_vecPosition = other + (vecDir * 0.5f);
                            pBill2->m_vecRotAxis = vecDir;
                            pBill2->m_fAxisAngle = (m_fAngle + 1.5707964f) - 0.050000001f;
                            pBill2->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                            pBill2->m_nParticleType = 15;
                            pBill2->m_fScaleVelX = 0.0049999999f;
                            pBill2->m_fScaleVelY = 0.0049999999f;
                            pBill2->m_fScaleVelZ = 0.0049999999f;
                            if (m_bCritical)
                                pBill2->SetColor(0x883333);
                            else
                                pBill2->SetColor(dwLightColor);
                            g_pCurrentScene->m_pEffectContainer->AddChild(pBill2);
                        }
                    }
                }

                m_dwAttackEffectTime = g_pTimerManager->GetServerTime();
            }
        }

        if (m_nSkinMeshType == 20 && (!m_stLookInfo.HelmMesh || m_stLookInfo.HelmMesh == 2) && !(m_pSkinMesh->m_dwOffset % 5) &&
            m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02 &&
            (m_vecOldFire.x != m_vecTempPos[0].x || m_vecOldFire.y != m_vecTempPos[0].y || m_vecOldFire.z != m_vecTempPos[0].z))
        {
            auto pBill = new TMEffectBillBoard(44, 2000, 0.5f, 0.5f, 0.5f, 0.00050000002f, 1, 80);
            if (pBill)
            {
                pBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pBill->m_vecPosition = m_vecTempPos[0];
                g_pCurrentScene->m_pEffectContainer->AddChild(pBill);
            }
            m_vecOldFire = m_vecTempPos[0];
        }
    }
}

void TMHuman::SetWeaponType(int nWeaponType)
{
    if (m_dwDelayDel != 0)
        return;

    m_nWeaponTypeIndex = nWeaponType;
}

/// Atualiza visual e tipo das armas equipadas nas maos esquerda e direita.
void TMHuman::CheckWeapon(short sIndexL, short sIndexR)
{
    if (m_dwDelayDel != 0)
        return;

    m_cHasShield = 0;

    STRUCT_ITEM itemL{};
    STRUCT_ITEM itemR{};

    m_sLeftIndex = sIndexL;
    itemL.sIndex = sIndexL;
    m_sRightIndex = sIndexR;
    itemR.sIndex = sIndexR;
    m_nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);
    m_nWeaponTypeR = BASE_GetItemAbility(&itemR, 21);
    unsigned int nWeaponPosL = BASE_GetItemAbility(&itemL, 17);
	unsigned int nWeaponPosR = BASE_GetItemAbility(&itemR, 17);

    if (itemR.sIndex < 0 || itemL.sIndex < 0 || itemR.sIndex >= 6500 || itemL.sIndex >= 6500)
    {
        LOG_WRITELOG("Check Weapon : L = %d R = %d", itemL.sIndex, itemR.sIndex);
        return;
    }
    if (g_pItemList[itemR.sIndex].nIndexMesh >= 0 && g_pItemList[itemL.sIndex].nIndexMesh >= 0)
    {
        TMMesh* pMesh1 = g_pMeshManager->GetCommonMesh(g_pItemList[itemR.sIndex].nIndexMesh, 0, 3_min);
        TMMesh* pMesh2 = g_pMeshManager->GetCommonMesh(g_pItemList[itemL.sIndex].nIndexMesh, 0, 3_min);

        if (!pMesh1 || !pMesh2)
        {
            LOG_WRITELOG("NULL Mesh Check Weapon : L = %d R = %d", itemL.sIndex, itemR.sIndex);
            return;
        }

        m_fSowrdLength[0] = pMesh1->m_fMaxZ;
        m_fSowrdLength[1] = pMesh2->m_fMaxZ;
        m_bSwordShadow[0] = 0;
        m_bSwordShadow[1] = 0;

        if (m_pSkinMesh)
        {
            if (m_nClass == 26 || m_nClass == 33 || m_nClass == 40)
            {
                SetWeaponType(0);
                if (m_sRightIndex)
                    m_bSwordShadow[0] = 1;
                if (m_sLeftIndex)
                    m_bSwordShadow[1] = 1;
                if (m_pSkinMesh->m_pSwingEffect[0])
                {
                    m_fSowrdLength[0] = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_fWeaponLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect = 16 * g_pItemList[itemR.sIndex].nGrade;
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect += m_stSancInfo.Sanc6;
                }
                if (m_pSkinMesh->m_pSwingEffect[1])
                {
                    m_fSowrdLength[1] = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_fWeaponLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect = 16 * g_pItemList[itemL.sIndex].nGrade;
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect += m_stSancInfo.Sanc7;
                }
                return;
            }

            if (m_pSkinMesh->m_pSwingEffect[0])
            {
                m_pSkinMesh->m_pSwingEffect[0]->m_fWeaponLength = m_fSowrdLength[0] - 0.1f;
                m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect = 16 * g_pItemList[itemR.sIndex].nGrade;
                m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect += m_stSancInfo.Sanc6;
            }
            if (m_pSkinMesh->m_pSwingEffect[1])
            {
                m_pSkinMesh->m_pSwingEffect[1]->m_fWeaponLength = m_fSowrdLength[1] - 0.1f;
                m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect = 16 * g_pItemList[itemL.sIndex].nGrade;
                m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect += m_stSancInfo.Sanc7;
            }
            if (m_cWeapon == 1)
            {
                if (m_pSkinMesh->m_pSwingEffect[0] && m_sRightIndex)
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 1;
                if (m_pSkinMesh->m_pSwingEffect[1] && m_sLeftIndex)
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 1;
            }
            else
            {
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 0;
            }

            if (m_nClass == 36)
            {
                if (m_nWeaponTypeL == 1 || m_nWeaponTypeL == 61 || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(11);
                }
                else if (m_nWeaponTypeL == 11)
                {
                    SetWeaponType(15);
                }
                else if (m_nWeaponTypeL == 13)
                {
                    SetWeaponType(12);
                }
                else if (m_nWeaponTypeL == 21 || m_nWeaponTypeL == 22 || m_nWeaponTypeL == 23)
                {
                    SetWeaponType(13);
                }
                else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                {
                    SetWeaponType(14);
                }
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 37)
            {
                SetWeaponType(11);
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 60)
            {
                if (m_nWeaponTypeL == 41 && !nWeaponPosR)
                {
                    m_pSkinMesh->m_cRotate[1] = 1;
                    SetWeaponType(0);
                    m_bSwordShadow[0] = 1;
                }
                else if (m_nWeaponTypeL == 11 && nWeaponPosR == 128)
                {
                    SetWeaponType(1);
                    m_bSwordShadow[0] = 1;
                }
                else if (m_nWeaponTypeL == 11 && nWeaponPosR == 192)
                {
                    SetWeaponType(4);
                    m_bSwordShadow[0] = 1;
                    m_bSwordShadow[1] = 1;
                }
                else
                {
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                    SetWeaponType(0);
                }
                return;
            }
            if (m_nClass == 61)
            {
                SetWeaponType(1);
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 62)
            {
                if (m_nWeaponTypeL == 1
                    || m_nWeaponTypeL == 11
                    || m_nWeaponTypeL == 61
                    || m_nWeaponTypeL == 2
                    || m_nWeaponTypeL == 12
                    || m_nWeaponTypeL == 62
                    || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(1);
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                }
                else
                {
                    SetWeaponType(0);
                }
                return;
            }

            if (!m_nSkinMeshType)
            {
                if (!m_cMount)
                {
                    if (!m_nWeaponTypeL && nWeaponPosR == 128)
                    {
                        SetWeaponType(2);
                    }
                    else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                    {
                        SetWeaponType(4);
                        m_bSwordShadow[0] = 1;
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 1
                        || m_nWeaponTypeL == 11
                        || m_nWeaponTypeL == 61
                        || m_nWeaponTypeL == 31)
                    {
                        if (!m_nWeaponTypeR)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 128)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 196)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 2
                        || m_nWeaponTypeL == 12
                        || m_nWeaponTypeL == 62)
                    {
                        if (nWeaponPosR == 128)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (!m_nWeaponTypeR)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 196)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                    {
                        SetWeaponType(6);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 13)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(7);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 21
                        || m_nWeaponTypeL == 22
                        || m_nWeaponTypeL == 23)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(8);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(10);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 104)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(9);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 101)
                    {
                        SetWeaponType(3);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 32 || m_nWeaponTypeL == 33)
                    {
                        SetWeaponType(5);
                        m_bSwordShadow[1] = 1;
                    }
                    else
                    {
                        if (m_nWeaponTypeL == 41)
                        {
                            m_bSwordShadow[1] = 1;
                            m_bSwordShadow[0] = 1;
                        }
                        SetWeaponType(0);
                    }
                }
                else if (!m_nWeaponTypeL && nWeaponPosR == 128)
                {
                    SetWeaponType(3);
                }
                else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                {
                    SetWeaponType(2);
                    m_bSwordShadow[0] = 1;
                    m_bSwordShadow[1] = 1;
                }
                else if (m_nWeaponTypeL == 1
                    || m_nWeaponTypeL == 11
                    || m_nWeaponTypeL == 61
                    || m_nWeaponTypeL == 2
                    || m_nWeaponTypeL == 12
                    || m_nWeaponTypeL == 62
                    || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(1);
                    if (!m_nWeaponTypeR)
                    {
                        m_bSwordShadow[1] = 1;
                    }
                    else if (nWeaponPosR == 128)
                    {
                        m_bSwordShadow[1] = 1;
                    }
                }
                else if (m_nWeaponTypeL == 21
                    || m_nWeaponTypeL == 22
                    || m_nWeaponTypeL == 23
                    || m_nWeaponTypeL == 13
                    || m_nWeaponTypeL == 3
                    || m_nWeaponTypeL == 63
                    || m_nWeaponTypeL == 32
                    || m_nWeaponTypeL == 33)
                {
                    if (nWeaponPosL == 64)
                    {
                        SetWeaponType(4);
                        m_bSwordShadow[1] = 1;
                    }
                }
                else if (m_nWeaponTypeL == 101)
                {
                    SetWeaponType(5);
                    m_bSwordShadow[1] = 1;
                }
                else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                {
                    if (nWeaponPosL == 64)
                    {
                        SetWeaponType(1);
                        m_bSwordShadow[1] = 1;
                    }
                }
                else
                {
                    if (m_nWeaponTypeL == 41)
                    {
                        m_bSwordShadow[1] = 1;
                        m_bSwordShadow[0] = 1;
                    }
                    SetWeaponType(0);
                }
                if (m_nWeaponTypeL == 41)
                {
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                }               
            }
            else
            {
                switch (m_nSkinMeshType)
                {
                case 1:
                    if (!m_cMount)
                    {
                        if (!m_nWeaponTypeL && nWeaponPosR == 128)
                        {
                            SetWeaponType(2);
                        }
                        else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                        {
                            SetWeaponType(10);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 61
                            || m_nWeaponTypeL == 2
                            || m_nWeaponTypeL == 12
                            || m_nWeaponTypeL == 62
                            || m_nWeaponTypeL == 31)
                        {
                            if (!m_nWeaponTypeR)
                            {
                                SetWeaponType(1);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 128)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 196)
                            {
                                SetWeaponType(4);
                                m_bSwordShadow[0] = 1;
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 21
                            || m_nWeaponTypeL == 22
                            || m_nWeaponTypeL == 23)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 31)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 13)
                        {
                            SetWeaponType(7);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 32 || m_nWeaponTypeL == 33)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(9);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 101)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(6);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else
                        {
                            if (m_nWeaponTypeL == 41)
                            {
                                m_bSwordShadow[1] = 1;
                                m_bSwordShadow[0] = 1;
                            }
                            SetWeaponType(0);
                        }
                    }
                    else if (!m_nWeaponTypeL && nWeaponPosR == 128)
                    {
                        SetWeaponType(3);
                    }
                    else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                    {
                        SetWeaponType(2);
                        m_bSwordShadow[0] = 1;
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 1
                        || m_nWeaponTypeL == 11
                        || m_nWeaponTypeL == 61
                        || m_nWeaponTypeL == 2
                        || m_nWeaponTypeL == 12
                        || m_nWeaponTypeL == 62
                        || m_nWeaponTypeL == 31)
                    {
                        SetWeaponType(1);
                        if (!m_nWeaponTypeR)
                        {
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 128)
                        {
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 21
                        || m_nWeaponTypeL == 22
                        || m_nWeaponTypeL == 23
                        || m_nWeaponTypeL == 13
                        || m_nWeaponTypeL == 3
                        || m_nWeaponTypeL == 63
                        || m_nWeaponTypeL == 32
                        || m_nWeaponTypeL == 33)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 101)
                    {
                        SetWeaponType(5);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else
                    {
                        if (m_nWeaponTypeL == 41)
                        {
                            m_bSwordShadow[1] = 1;
                            m_bSwordShadow[0] = 1;
                        }
                        SetWeaponType(0);
                    }
                    break;
                    case 2:
                        if (!m_cMount)
                        {
                            if (m_nWeaponTypeL == 101)
                            {
                                SetWeaponType(1);
                                m_bSwordShadow[1] = 0;
                            }
                            else if (m_nWeaponTypeL == 12)
                            {
                                SetWeaponType(2);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 128 || m_nWeaponTypeL == 1 || m_nWeaponTypeL == 11)
                            {
                                SetWeaponType(5);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (m_nWeaponTypeL == 11
                                || m_nWeaponTypeL == 12
                                || m_nWeaponTypeL == 13
                                || m_nWeaponTypeL == 21)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (m_nWeaponTypeL == 31
                                || m_nWeaponTypeL == 32
                                || m_nWeaponTypeL == 33)
                            {
                                SetWeaponType(4);
                                m_bSwordShadow[1] = 1;
                            }
                            else
                            {
                                SetWeaponType(0);
                                if (nWeaponPosL)
                                    m_bSwordShadow[1] = 1;
                                else
                                    m_bSwordShadow[1] = 0;
                            }
                        }
                        else if (m_nWeaponTypeL == 101)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 0;
                        }
                        else if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 12
                            || m_nWeaponTypeL == 13
                            || m_nWeaponTypeL == 31
                            || m_nWeaponTypeL == 32
                            || m_nWeaponTypeL == 33)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else
                        {
                            SetWeaponType(0);
                            if (nWeaponPosL)
                                m_bSwordShadow[1] = 1;
                            else
                                m_bSwordShadow[1] = 0;
                        }
                        break;
                    case 3:
                        if (m_cMount)
                        {
                            SetWeaponType(0);
                        }
                        else if (m_stLookInfo.FaceMesh)
                        {
                            if (m_stLookInfo.FaceMesh == 1)
                                SetWeaponType(1);
                        }
                        else
                        {
                            SetWeaponType(0);
                        }
                        break;
                    case 4:
                        if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 61)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 21
                            || m_nWeaponTypeL == 22
                            || m_nWeaponTypeL == 23
                            || m_nWeaponTypeL == 13)
                        {
                            SetWeaponType(2);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 0;
                        }
                        else
                        {
                            SetWeaponType(0);
                            if (nWeaponPosL)
                                m_bSwordShadow[1] = 1;
                            else
                                m_bSwordShadow[1] = 0;
                        }
                        break;
                }
            }
            if (m_nWeaponTypeL == 41 && m_pSkinMesh)
            {
                m_pSkinMesh->m_cRotate[1] = 1;
            }
            else if (m_pSkinMesh)
            {
                m_pSkinMesh->m_cRotate[1] = 0;
            }
            if (m_nSkinMeshType == 11)
            {
                m_pSkinMesh->m_cRotate[1] = 1;
            }
            else if (m_nSkinMeshType == 10)
            {
                m_pSkinMesh->m_cRotate[0] = 1;
                SetWeaponType(0);
                if (m_sRightIndex)
                    m_bSwordShadow[0] = 1;
                if (m_sLeftIndex)
                    m_bSwordShadow[1] = 1;
            }
            if ((int)m_eMotion < 14)
                SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
        }
    }
}

void TMHuman::PlayAttackSound(ECHAR_MOTION eMotion, int nLR)
{
    if (m_dwDelayDel)
        return;

    STRUCT_ITEM itemL{};
    STRUCT_ITEM itemR{};

    itemL.sIndex = m_sLeftIndex;
    itemR.sIndex = m_sRightIndex;
    BASE_GetItemAbility(&itemL, 17);
    
	unsigned int nWeaponPosR = BASE_GetItemAbility(&itemR, 17);
    int nSoundIndex = 121;

    if (m_nSkinMeshType == 3 || m_nClass == 40)
    {
        if (!nLR && m_nWeaponTypeL || nLR == 1 && nWeaponPosR && nWeaponPosR != 128)
        {
            if (g_pSoundManager)
            {
                auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
                if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sAttackLR != nLR))
                {
                    pSoundData->Play(0, 0);
                }
            }
        }
        m_sAttackLR = nLR;
        return;
    }

    if (m_nWeaponTypeL == 1 && !nLR || m_nWeaponTypeR == 1 && nLR == 1)
    {
        if (g_pItemList[m_sLeftIndex].nReqLvl > 90 && !nLR || g_pItemList[m_sRightIndex].nReqLvl > 90 && !nLR)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 124;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 125;
        }
        else if (!((signed int)eMotion % 2))
            nSoundIndex = 121;
        else if ((signed int)eMotion % 2 == 1)
            nSoundIndex = 122;
    }
    else if (m_nWeaponTypeL == 2 && !nLR || m_nWeaponTypeR == 2 && nLR == 1)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 124;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 125;
    }
    else if (m_nWeaponTypeL == 3 && !nLR || m_nWeaponTypeR == 3 && nLR == 1)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 126;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 127;
    }
    else if (!nLR
        && (m_nWeaponTypeL == 11
            || m_nWeaponTypeL == 12
            || m_nWeaponTypeL == 31
            || m_nWeaponTypeL == 32
            || m_nWeaponTypeL == 61
            || m_nWeaponTypeL == 62)
        || nLR == 1
        && (m_nWeaponTypeR == 11
            || m_nWeaponTypeR == 12
            || m_nWeaponTypeR == 31
            || m_nWeaponTypeR == 32
            || m_nWeaponTypeR == 61
            || m_nWeaponTypeR == 62))
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 128;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 129;
    }
    else if (m_nWeaponTypeL == 13 || m_nWeaponTypeL == 33 || m_nWeaponTypeL == 63)
    {
        if (!((int)eMotion % 2))
            nSoundIndex = 131;
        else if ((int)eMotion % 2 == 1)
            nSoundIndex = 132;
    }
    else
    {
        if (m_nWeaponTypeL == 101)
            return;
        if (m_nWeaponTypeL == 41)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 135;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 136;
        }
        else if (m_nWeaponTypeL == 21 || m_nWeaponTypeL == 22 || m_nWeaponTypeL == 23)
        {
            if (!((int)eMotion % 2))
                nSoundIndex = 137;
            else if ((int)eMotion % 2 == 1)
                nSoundIndex = 138;
        }
        else if (m_nWeaponTypeL == 102)
            nSoundIndex = 139;
        else if (m_nWeaponTypeL != 103 && m_nWeaponTypeL != 104)
            nSoundIndex = 121;
        else
            nSoundIndex = 140;
    }

    if (!nLR && m_nWeaponTypeL || nLR == 1 && nWeaponPosR && nWeaponPosR != 128)
    {
        if (g_pSoundManager)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sAttackLR != nLR))
            {
                pSoundData->Play(0, 0);
            }
        }
    }

    m_sAttackLR = nLR;
}

void TMHuman::PlayPunchedSound(int nType, int nLR)
{
    if (m_dwDelayDel)
        return;

    int nSoundIndex = 27;
    unsigned int dwServerTime = g_pTimerManager->GetServerTime();
    if (m_nSkinMeshType == 3 || m_nClass == 40)
    {
        if (g_pSoundManager)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sPunchLR != nLR || dwServerTime > m_dwLastPlayPunchedTime + 400))
            {
                pSoundData->Play(0, 0);
                m_dwLastPlayPunchedTime = dwServerTime;
            }
        }

        m_sPunchLR = nLR;
        return;
    }
    
    if (nType == 1)
        nSoundIndex = 21;
    if (nType == 2)
        nSoundIndex = 22;
    if (nType == 3)
        nSoundIndex = 23;
    if (nType == 11 || nType == 31 || nType == 61)
        nSoundIndex = 25;
    if (nType == 12 || nType == 32 || nType == 62)
        nSoundIndex = 26;
    if (nType == 13 || nType == 33 || nType == 63)
        nSoundIndex = 27;
    if (nType == 21 || nType == 22 || nType == 23 || nType == 41)
        nSoundIndex = 28;
    if (nType > 100 && nType < 105)
        nSoundIndex = 24;

    if (g_pSoundManager)
    {
        if (nSoundIndex != 24)
        {
            auto pSoundData = g_pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && (!pSoundData->IsSoundPlaying() || m_sPunchLR != nLR || dwServerTime > m_dwLastPlayPunchedTime + 400))
            {
                pSoundData->Play(0, 0);
                m_dwLastPlayPunchedTime = dwServerTime;
            }
        }
        m_sPunchLR = nLR;
    }
}

void TMHuman::SetMotion(ECHAR_MOTION eMotion, float fAngle)
{
    if (m_dwDelayDel)
        return;

    SetAnimation(eMotion, 0);
}

void TMHuman::SetHandEffect(int nHandEffect)
{
    if (m_dwDelayDel)
        return;

    m_nHandEffect = nHandEffect;
    if (m_pSkinMesh)
    {
        if (m_bSwordShadow[0] == 1 && m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = m_nHandEffect;
        else if (m_pSkinMesh->m_pSwingEffect[0])
            m_pSkinMesh->m_pSwingEffect[0]->m_nHandEffect = 0;
        if (m_bSwordShadow[1] == 1 && m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = m_nHandEffect;
        else if (m_pSkinMesh->m_pSwingEffect[1])
            m_pSkinMesh->m_pSwingEffect[1]->m_nHandEffect = 0;
    }
}

void TMHuman::GetLegType()
{
    if (!m_dwDelayDel)
    {
        int nSkinMeshType = m_nSkinMeshType;
        if (m_cMount == 1)
            nSkinMeshType = m_nMountSkinMeshType;

        if (nSkinMeshType)
        {
            switch (nSkinMeshType)
            {
            case 1:
                m_nLegType = 1;
                break;
            case 11:
                m_nLegType = 4;
                break;
            case 20:
                m_nLegType = 0;
                break;
            case 21:
                m_nLegType = 2;
                break;
            case 22:
                m_nLegType = 1;
                break;
            case 23:
                m_nLegType = 0;
                break;
            case 24:
                m_nLegType = 0;
                break;
            case 2:
                m_nLegType = 1;
                break;
            case 25:
                m_nLegType = 2;
                break;
            case 26:
                m_nLegType = 3;
                break;
            case 27:
                m_nLegType = 3;
                break;
            case 3:
                m_nLegType = 1;
                break;
            case 28:
                m_nLegType = 2;
                break;
            case 29:
                m_nLegType = 2;
                break;
            case 6:
                m_nLegType = 1;
                break;
            case 4:
                m_nLegType = 2;
                break;
            case 32:
                m_nLegType = 0;
                break;
            case 7:
                m_nLegType = 2;
                break;
            case 8:
                m_nLegType = 0;
                break;
            case 69:
                m_nLegType = 0;
                break;
            case 30:
                m_nLegType = 2;
                break;
            case 31:
                m_nLegType = 2;
                break;
            case 36:
                m_nLegType = 3;
                break;
            case 35:
                m_nLegType = 5;
                break;
            case 34:
                m_nLegType = 3;
                break;
            case 38:
                m_nLegType = 2;
                break;
            case 39:
                m_nLegType = 2;
                break;
            case 40:
                m_nLegType = 0;
                break;
            case 12:
                m_nLegType = 0;
                break;
            case 43:
                m_nLegType = 0;
                break;
            case 42:
                m_nLegType = 4;
                break;
            case 10:
                m_nLegType = 1;
                break;
            case 44:
                m_nLegType = 0;
                break;
            case 5:
                m_nLegType = m_stMountLook.Mesh0 == 1;
                break;
            }
        }
        else
        {
            m_nLegType = 1;
        }
    }
}

int TMHuman::GetBloodColor()
{
    int nBlood = 0;
    if (m_nClass <= 8)
        nBlood = 0;

    if (m_nClass == 25 && m_stLookInfo.FaceMesh == 3 && m_stLookInfo.FaceSkin == 8 || m_nClass == 25 && m_stLookInfo.FaceMesh == 12)
        nBlood = 89;

    if (m_nClass == 28 && m_stLookInfo.FaceMesh == 2)
        nBlood = 89;
    if (m_nClass == 16 && m_stLookInfo.FaceMesh == 6)
        nBlood = 89;

    switch (m_nSkinMeshType)
    {
    case 26:
        nBlood = 89;
        break;
    case 35:
        nBlood = 89;
        break;
    case 36:
        nBlood = 89;
        break;
    case 11:
        nBlood = 89;
        break;
    }

    if (m_nClass == 44 || m_nClass == 45)
        nBlood = 56;

    return nBlood;
}

void TMHuman::SetAvatar(char cAvatar)
{
    m_cAvatar = cAvatar;
}

void TMHuman::UpdateMount()
{
    TMFieldScene* pScene = (TMFieldScene*)g_pCurrentScene;
    if (pScene->m_bAirMove == 1 && g_pCurrentScene->m_pMyHuman == this)
    {
        m_cMount = 1;
        m_nMountSkinMeshType = 40;
        SetAnimation(ECHAR_MOTION::ECMOTION_SEATING, 1);

        memset(&m_stMountLook, 0, sizeof(m_stMountLook));
        if (m_pMantua)
        {
            m_pMantua->SetVecMantua(1, 40);
            m_pMantua->SetAnimation(3);
        }
    }

    SAFE_DELETE(m_pMount);

    if (m_cMount > 0)
    {
        if (m_pMount == nullptr)
        {
            m_pMount = new TMSkinMesh(&m_stMountLook,
                &m_stMountSanc,
                m_nMountSkinMeshType,
                0,
                0,
                1,
                0,
                1);

            if (m_pMount)
            {
                m_pMount->m_pOwner = this;
                if (m_pMount->m_nBoneAniIndex == 50)
                    m_pMount->m_dwFPS = 7;
                else
                    m_pMount->m_dwFPS = 40;
                if (m_nClass == 40)
                {
                    m_pMount->m_vScale.x = m_fMountScale;
                    m_pMount->m_vScale.y = m_fMountScale;
                    m_pMount->m_vScale.z = m_fMountScale;
                }
                else
                {
                    m_pMount->m_vScale.x = m_fScale * m_fMountScale;
                    m_pMount->m_vScale.y = m_fScale * m_fMountScale;
                    m_pMount->m_vScale.z = m_fScale * m_fMountScale;
                }

                m_pMount->m_bBaseMat = 0;
                if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 == 7)
                {
                    m_pSkinMesh->SetVecMantua(4, m_nMountSkinMeshType);
                }
                else if (m_nMountSkinMeshType == 20)
                {
                    m_pSkinMesh->SetVecMantua(3, m_nMountSkinMeshType);
                }
                else
                {
                    m_pSkinMesh->SetVecMantua(2, m_nMountSkinMeshType);
                }
            }
        }

        if (m_pMount)
            m_pMount->RestoreDeviceObjects();

        return;
    }

    if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && g_pCurrentScene->m_pMyHuman == this)
    {
        pScene->SetPosPKRun();
        SetSpeed(pScene->m_bMountDead);
    }
}

void TMHuman::SetMantua(int nTexture)
{
    if (!nTexture
        || nTexture == 2
        || nTexture == 8
        || nTexture == 9
        || nTexture == 11
        || nTexture == 24
        || nTexture == 27
        || nTexture == 31
        || nTexture == 34)
    {
        m_cMantua = 1;
    }
    else if (nTexture == 1
        || nTexture == 3
        || nTexture == 12
        || nTexture == 13
        || nTexture == 14
        || nTexture == 25
        || nTexture == 28
        || nTexture == 30
        || nTexture == 32
        || nTexture == 35)
    {
        m_cMantua = 2;
    }
    else if (nTexture == 6
        || nTexture == 7
        || nTexture == 15
        || nTexture == 16
        || nTexture == 17
        || nTexture == 19
        || nTexture == 26
        || nTexture == 29
        || nTexture == 33
        || nTexture == 36)
    {
        m_cMantua = 3;
    }
    else
    {
        m_cMantua = 4;
    }
}

int TMHuman::SetCitizenMantle(int BaseSkin)
{
    if (m_citizen < 0 || m_citizen > 10)
        m_citizen = 0;
    if (!m_citizen)
        return BaseSkin;

    switch (BaseSkin)
    {
    case 34:
    case 35:
    case 36:
        return BaseSkin;
    case 19:
        return m_citizen + 39;
    case 3:
        return m_citizen + 49;
    case 2:
        return m_citizen + 59;
    case 6:
        return m_citizen + 69;
    case 1:
        return m_citizen + 79;
    case 0:
        return m_citizen + 89;
    case 7:
        return m_citizen + 99;
    case 25:
        return m_citizen + 109;
    case 24:
        return m_citizen + 119;
    case 26:
        return m_citizen + 129;
    case 28:
        return m_citizen + 139;
    case 27:
        return m_citizen + 149;
    case 29:
        return m_citizen + 159;
    case 32:
        return m_citizen + 169;
    case 31:
        return m_citizen + 179;
    case 33:
        BaseSkin = m_citizen + 189;
        break;
    }

    return BaseSkin;
}

int TMHuman::UnSetCitizenMantle(int BaseSkin)
{
    int mantle = BaseSkin / 10;
    if (BaseSkin / 10 == 4)
        return 19;

    switch (mantle)
    {
    case 5:
        return 3;
    case 6:
        return 2;
    case 7:
        return 6;
    case 8:
        return 1;
    case 9:
        return 0;
    case 10:
        return 7;
    case 11:
        return 25;
    case 12:
        return 24;
    case 13:
        return 26;
    case 14:
        return 28;
    case 15:
        return 27;
    case 16:
        return 29;
    case 17:
        return 32;
    case 18:
        return 31;
    case 19:
        BaseSkin = 33;
        break;
    }
    return BaseSkin;
}

void TMHuman::SetMountCostume(unsigned int index)
{
    if (index >= 11)
    {
        m_stMountLook.Mesh2 = 0;
        m_stMountLook.Mesh1 = 0;
        m_stMountLook.Mesh0 = 0;
        m_stMountLook.Skin2 = 0;
        m_stMountLook.Skin1 = 0;
        m_stMountLook.Skin0 = 0;
        memset(&m_stMountSanc.Sanc0, 0, sizeof m_stMountSanc);
        memset(&m_stOldMountSanc.Sanc0, 0, sizeof m_stOldMountSanc);

        switch (index)
        {
        case 11:
            m_nMountSkinMeshType = 29;
            m_stMountLook.Mesh0 = 5;
            m_stMountLook.Mesh1 = 5;
            m_fMountScale = 1.25f;
            break;
        case 12:
        case 13:
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 8;
            m_stMountLook.Mesh1 = 8;
            if (index == 12)
            {
                m_stMountLook.Skin0 = 1;
                m_stMountLook.Skin1 = 1;
            }
            else
            {
                m_stMountSanc.Sanc2 = 12;
                m_stMountSanc.Sanc1 = 12;
                m_stMountSanc.Sanc0 = 12;
                m_stOldMountSanc.Sanc2 = 12;
                m_stOldMountSanc.Sanc1 = 12;
                m_stOldMountSanc.Sanc0 = 12;
            }

            m_fMountScale = 1.0f;
            break;
        case 14:
            m_nMountSkinMeshType = 48;
            m_fMountScale = 0.80f;
            break;
        case 15:
        case 16:
            m_stMountLook.Skin0 = 1;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69f;
            break;
        case 17:
        case 18:
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 14;
            m_stMountLook.Mesh1 = 14;
            if (index == 18)
            {
                m_stMountLook.Skin0 = 1;
                m_stMountLook.Skin1 = 1;
            }

            m_fMountScale = 0.89f;
            m_stMountSanc.Sanc2 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc0 = 13;
            m_stOldMountSanc.Sanc2 = 13;
            m_stOldMountSanc.Sanc1 = 13;
            m_stOldMountSanc.Sanc0 = 13;
            break;
        case 19:
            m_nMountSkinMeshType = 50;
            m_stMountSanc.Sanc2 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc0 = 13;
            m_stOldMountSanc.Sanc2 = 13;
            m_stOldMountSanc.Sanc1 = 13;
            m_stOldMountSanc.Sanc0 = 13;
            break;
        case 20:
            m_stMountLook.Skin0 = 1;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Mesh0 = 1;
            m_stMountLook.Mesh1 = 1;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69f;
            break;
        case 21:
            m_nMountSkinMeshType = 51;
            m_fMountScale = 1.0f;
            break;
        }
    }
}

/// Aplica malhas e texturas de traje/costume especial ao personagem.
int TMHuman::SetHumanCostume()
{
    int nCos = 0;
    m_nSkinMeshType = 0;
    memset(&m_stColorInfo, 0, sizeof(m_stColorInfo));

    if (m_sCostume <= 6301)
    {
        if (m_sCostume != 6301)
        {
            switch (m_sCostume)
            {
            case 4150:
                nCos = 16;
                m_nSkinMeshType = 0;
                break;
            case 4151:
                nCos = -1;
                m_nSkinMeshType = 0;
                break;
            case 4152:
                nCos = 1;
                m_nSkinMeshType = 1;
                break;
            case 4153:
                nCos = 2;
                m_nSkinMeshType = 0;
                break;
            case 4154:
                nCos = 3;
                m_nSkinMeshType = 1;
                break;
            case 4155:
                nCos = 4;
                m_nSkinMeshType = 0;
                break;
            case 4156:
                nCos = 5;
                m_nSkinMeshType = 1;
                break;
            case 4157:
                nCos = 7;
                m_nSkinMeshType = 1;
                break;
            case 4158:
                nCos = 6;
                m_nSkinMeshType = 0;
                break;
            case 4159:
                nCos = 8;
                m_nSkinMeshType = 1;
                break;
            case 4160:
                nCos = 9;
                m_nSkinMeshType = 1;
                break;
            case 4161:
                nCos = 10;
                m_nSkinMeshType = 1;
                break;
            case 4162:
                nCos = 11;
                m_nSkinMeshType = 1;
                break;
            case 4163:
                nCos = 13;
                m_nSkinMeshType = 1;
                break;
            case 4164:
                nCos = 12;
                m_nSkinMeshType = 1;
                break;
            case 4165:
                nCos = 14;
                m_nSkinMeshType = 0;
                break;
            case 4166:
                nCos = 15;
                m_nSkinMeshType = 1;
                break;
            case 4167:
                nCos = 17;
                m_nSkinMeshType = 1;
                break;
            case 4168:
                nCos = 18;
                m_nSkinMeshType = 1;
                break;
            case 4169:
                nCos = 21;
                m_nSkinMeshType = 0;
                break;
            case 4170:
                nCos = 22;
                m_nSkinMeshType = 0;
                break;
            case 4171:
                nCos = 19;
                m_nSkinMeshType = 0;
                break;
            case 4172:
                nCos = 20;
                m_nSkinMeshType = 0;
                break;
            case 4173:
                nCos = 23;
                m_nSkinMeshType = 0;
                break;
            case 4174:
                nCos = 24;
                m_nSkinMeshType = 0;
                break;
            case 4175:
                nCos = 25;
                m_nSkinMeshType = 1;
                break;
            case 4176:
                nCos = 26;
                m_nSkinMeshType = 1;
                break;
            case 4177:
                nCos = 27;
                m_nSkinMeshType = 1;
                break;
            case 4178:
                nCos = 28;
                m_nSkinMeshType = 0;
                break;
            case 4179:
                nCos = 29;
                m_nSkinMeshType = 1;
                break;
            case 4180:
                nCos = 30;
                m_nSkinMeshType = 1;
                break;
            case 4181:
                nCos = 31;
                m_nSkinMeshType = 1;
                break;
            case 4182:
                nCos = 32;
                m_nSkinMeshType = 1;
                break;
            case 4183:
                nCos = 33;
                m_nSkinMeshType = 1;
                break;
            }
        }
        else
        {
            nCos = 16;
            m_nSkinMeshType = 0;
        }
    }
    else if (m_sCostume == 6400)
    {
        nCos = 34;
        m_nSkinMeshType = 1;
    }

    m_stLookInfo.FaceMesh = 0;
    m_stLookInfo.FaceSkin = 0;
    m_stLookInfo.HelmMesh = 0;
    m_stLookInfo.CoatMesh = 0;
    m_stLookInfo.PantsMesh = 0;
    m_stLookInfo.GlovesMesh = 0;
    m_stLookInfo.BootsMesh = 0;
    m_stLookInfo.CoatSkin = 0;
    m_stLookInfo.CoatSkin = 0;
    m_stLookInfo.PantsSkin = 0;
    m_stLookInfo.GlovesSkin = 0;
    m_stLookInfo.BootsSkin = 0;

    if (m_sCostume == 4150)
    {
        memset(&m_stSancInfo.Sanc0, 9, 6u);
        memset(&m_stSancInfo.Legend0, 0, 6u);
    }
    else if ((m_sCostume < 4169 || m_sCostume > 4174) && (m_sCostume < 4176 || m_sCostume > 4179)  && m_sCostume != 4183 && m_sCostume != 4151)
    {
        if (m_sCostume == 6301)
            memset(&m_stSancInfo.Sanc0, 9, 6u);
        else
            memset(&m_stSancInfo.Sanc0, 0, 6u);

        memset(&m_stSancInfo.Legend0, 0, 6u);
    }
    else
    {
        memset(&m_stSancInfo.Sanc0, 13, 6u);
        memset(&m_stSancInfo.Legend0, 0, 6u);
    }

    return nCos;
}
