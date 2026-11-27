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

/// @file TMHuman_Render.cpp
/// @brief Renderização do personagem e de seus rótulos: `Render`, picking (`IsMouseOver`),
///        posicionamento de nome/guilda/chat (`LabelPosition*`), minimapa e controles de UI.
///
/// Extraído de `TMHuman.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

/// Renderiza o modelo 3D, sombra, montaria e efeitos associados do personagem.
int TMHuman::Render()
{
    if (m_dwDelayDel)
        return 0;
    if (m_cDeleted == 1)
        return 0;
    if (!m_pSkinMesh)
        return 0;

    if (!IsVisible() || m_cHide == 1 || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
    {
        for (int i = 0; i < 7; ++i)
        {
            if (m_pRotateBone[i])
                m_pRotateBone[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 7; ++i)
        {
            if (m_pEyeFire[i])
                m_pEyeFire[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 4; ++i)
        {
            if (m_pFly[i])
                m_pFly[i]->m_bVisible = 0;
        }
        for (int i = 0; i < 2; ++i)
        {
            if (m_pSoul[i])
            {
                m_pSoul[i]->m_bVisible = 0;
                m_pSoul[i]->m_pBillBoard->m_bShow = 0;
            }
        }

        HideLabel();
        return 1;
    }
    else if (m_nClass == 45)
    {
        return 1;
    }

    int nCommon = m_stLookInfo.LeftMesh;
    TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nCommon, 0, 3_min);

    if (!pMesh || nCommon != 2888 && nCommon != 2889)
    {
        for (int j = 0; j < 7; ++j)
        {
            if (m_pRotateBone[j])
                m_pRotateBone[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 7; ++j)
        {
            if (m_pEyeFire[j])
                m_pEyeFire[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 4; ++j)
        {
            if (m_pFly[j])
                m_pFly[j]->m_bVisible = 1;
        }
        for (int j = 0; j < 2; ++j)
        {
            if (m_pSoul[j])
                m_pSoul[j]->m_bVisible = 1;
        }

        if (g_pDevice->m_dwBitCount == 32)
            g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xAAu);
        else
            g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFFu);

        if (m_cHide == 1 && (TMHuman*)g_pObjectManager->m_pCamera->m_pFocusedObject != this
            || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this  && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        {
            return 1;
        }

        if (m_cHide == 1 || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 9u);
            m_pSkinMesh->m_cDefaultAlpha = 0;
        }
        else if (g_bHideSkillBuffEffect == 1 && m_cCoinArmor == 1)
        {
            D3DCOLORVALUE color{};
            color.r = 1.0f;
            color.g = 0.69f;
            color.b = 0.0;
            color.a = 1.0f;
            m_pSkinMesh->m_materials.Diffuse = color;

            m_pSkinMesh->m_materials.Specular.r = 1.0f;
            m_pSkinMesh->m_materials.Specular.g = 1.0f;
            m_pSkinMesh->m_materials.Specular.b = 1.0f;
            m_pSkinMesh->m_materials.Emissive.r = 1.0f;
            m_pSkinMesh->m_materials.Emissive.g = 0.69f;
            m_pSkinMesh->m_materials.Emissive.b = 0.0;
        }
        else
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
            m_pSkinMesh->m_cDefaultAlpha = 1;
        }

        if (m_nClass == 32 && m_stLookInfo.FaceSkin == 2)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 8u);
        }
        else if (m_nClass == 59 || m_cShadow == 1)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
            g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
            g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
            g_pDevice->SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
        }
        else
        {
            g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 7u);
        }

        if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && m_pSkinMesh && m_bPunchEffect == 1)
        {
            m_pSkinMesh->m_materials.Emissive.r = 0.69f;
            m_pSkinMesh->m_materials.Emissive.g = 0.69f;
            m_pSkinMesh->m_materials.Emissive.b = 0.89f;
        }

        if (!m_cMount || !m_pMount)
        {
            m_pSkinMesh->Render(0.0f, 1.0f, 0.0f);
            m_vecSkinPos = TMVector3(m_pSkinMesh->m_vPosition.x, m_pSkinMesh->m_vPosition.y, m_pSkinMesh->m_vPosition.z);
        }
        else
        {
            m_pMount->Render(0.0f, 1.0f, 0.0f);
            m_pSkinMesh->m_BaseMatrix = m_pMount->m_OutMatrix;

            TMVector3 vecPosScale = TMVector3(
                -0.37f * m_fMountScale,
                1.0f / m_fMountScale,
                0.2f * m_fMountScale);

            switch (m_nMountSkinMeshType)
            {
            case 25:
                vecPosScale.x = -0.34999999f * m_fMountScale;
                vecPosScale.y = 1.0f / m_fMountScale;
                vecPosScale.z = -0.1f * m_fMountScale;
                break;
            case 28:
                vecPosScale.x = -0.2f * m_fMountScale;
                vecPosScale.y = 1.0f / m_fMountScale;
                vecPosScale.z = 0.1f * m_fMountScale;
                break;
            case 29:
                if (m_pMount->m_Look.Mesh0 != 1 && m_pMount->m_Look.Mesh0 != 4)
                {
                    vecPosScale.x = -0.37f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.60000002f * m_fMountScale;
                }
                else
                {
                    vecPosScale.x = -0.1f * m_fMountScale;
                    vecPosScale.z = -0.60000002f * m_fMountScale;
                }
                break;
            case 20:
                if (m_pMount->m_Look.Mesh0 == 7)
                {
                    vecPosScale.x = 0.5f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.1f * m_fMountScale;
                }
                else
                {
                    vecPosScale.x = 0.25f * m_fMountScale;
                    vecPosScale.y = 1.0f / m_fMountScale;
                    vecPosScale.z = -0.1f * m_fMountScale;
                }
                break;
            default:       
                break;
            }

            float fTemp = 0.0f;
            if (m_nClass == 40)
            {
                if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 != 7)
                {
                    vecPosScale.y = m_fScale / 1.3f;
                    vecPosScale.x = vecPosScale.x / 1.45f;
                }
                else if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 == 7)
                {
                    vecPosScale.y = m_fScale * 1.7f;
                }
                else
                {
                    vecPosScale.y = m_fScale;
                    vecPosScale.x = vecPosScale.x + 0.050000001f;
                }
            }

            if (m_nMountSkinMeshType == 29 && m_stMountLook.Mesh1 == 5)
            {
                vecPosScale.x = -0.2f;
                vecPosScale.y = 1.0f / m_fMountScale;
            }
            else if (m_nMountSkinMeshType == 48)
            {
                vecPosScale.x = vecPosScale.x - 0.80000001f;
                vecPosScale.z = vecPosScale.z + 0.2f;
            }
            else if (m_nMountSkinMeshType == 49 || m_nMountSkinMeshType == 52)
            {
                vecPosScale.x = vecPosScale.x - 0.2f;
                vecPosScale.z = vecPosScale.z - 0.2f;
            }
            else if (m_nMountSkinMeshType == 50)
            {
                vecPosScale.x = vecPosScale.x - 0.5f;
                vecPosScale.z = vecPosScale.z - 0.2f;
            }

            m_pSkinMesh->Render(vecPosScale.x, vecPosScale.y, vecPosScale.z);
            D3DXVECTOR4 vecOut;
            D3DXVECTOR3 vecIn = D3DXVECTOR3(vecPosScale.z / m_fMountScale,
                vecPosScale.x / m_fMountScale,
                0.0f);

            D3DXVec3Transform(&vecOut, &vecIn, &m_pSkinMesh->m_BaseMatrix);
            m_vecSkinPos.x = vecOut.x;
            m_vecSkinPos.y = vecOut.y - (float)(0.40000001 * m_fScale);
            m_vecSkinPos.z = vecOut.z;
        }

        if (m_cHide == 1
            || m_cCoinArmor == 1
            || m_cShadow == 1
            && g_pCurrentScene->m_pMyHuman != this
            && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
        {
            g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
            g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4u);
        }

        if (m_cMantua > 0 && m_pMantua)
        {
            float fScale = 1.0f;
            int nClass = 0;
            switch (m_nClass)
            {
            case 1:
                nClass = 0;
                break;
            case 2:
                nClass = 1;
                break;
            case 4:
                nClass = 2;
                break;
            case 8:
                nClass = 3;
                break;
            case 38:
                nClass = 1;
                break;
            case 25:
                nClass = 5;
                break;
            case 26:
                nClass = 6;
                break;
            case 33:
                nClass = 7;
                break;
            case 40:
                nClass = 8;
                break;
            default:
                nClass = 0;
                break;
            }

            if (m_sHeadIndex == 22 || m_sHeadIndex == 23)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.07f, 1.2f, -0.2f);
            }
            else if (m_sHeadIndex == 24)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.0f, 1.2f, 0.0f);
            }
            else if (m_sHeadIndex == 25)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.01f, 1.0f, 0.05f);
            }
            else if (m_sHeadIndex == 26)
            {
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(0.07f, 1.2f, 0.05f);
            }
            else
            {
                if (m_nSkinMeshType == 1)
                    fScale = 0.9f;

                int nMesh = m_stLookInfo.CoatMesh;
                float fLen = 0.0f;
                if (nMesh < 40)
                    fLen = fMantuaList[nClass][nMesh];
                else
                    fLen = fMantuaList[nClass][nMesh - 40]; // TODO: confirm this later

                float fLenUp = 0.0;
                if (m_sMantuaIndex >= 3197 && m_sMantuaIndex <= 3199)
                {
                    fLen = 0.11f;
                    fLenUp = 0.05f;
                }
                if (m_sMantuaIndex == 573 || m_sMantuaIndex == 1767 || m_sMantuaIndex == 1770)
                {
                    fLen = 0.15f;
                    fLenUp = 0.05f;
                }
                m_pMantua->m_BaseMatrix = m_pSkinMesh->m_OutMatrix;
                m_pMantua->Render(fLen, fScale, fLenUp);
            }
        }
        RenderEffect();
        if (m_bVisible == 1)
            LabelPosition();
        return 1;
    }

    if (nCommon == 2888)
    {
        pMesh->m_fScaleH = 3.0f;
        pMesh->m_fScaleV = 3.0f;
    }
    if (nCommon == 2889)
    {
        pMesh->m_fScaleH = 1.0f;
        pMesh->m_fScaleV = 1.0f;
    }

    if ((int)m_vecPosition.x == 1224 && (int)m_vecPosition.y == 1975
     || (int)m_vecPosition.x == 1225 && (int)m_vecPosition.y == 1976)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, -0.08726646f, 0, 0, 0, 0);       
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, -0.08726646f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1216 && (int)m_vecPosition.y == 1968
     || (int)m_vecPosition.x == 1217 && (int)m_vecPosition.y == 1969)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.2359877f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.2359877f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1207 && (int)m_vecPosition.y == 1988
        || (int)m_vecPosition.x == 1206 && (int)m_vecPosition.y == 1987)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.5850534f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 5.5850534f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1189 && (int)m_vecPosition.y == 1955
        || (int)m_vecPosition.x == 1188 && (int)m_vecPosition.y == 1954)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 4.8869219f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 4.8869219f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1240 && (int)m_vecPosition.y == 2006
        || (int)m_vecPosition.x == 1239 && (int)m_vecPosition.y == 2005)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1082 && (int)m_vecPosition.y == 2126
        || (int)m_vecPosition.x == 1081 && (int)m_vecPosition.y == 2125)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.9198622f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.9198622f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1074 && (int)m_vecPosition.y == 2119
        || (int)m_vecPosition.x == 1073 && (int)m_vecPosition.y == 2118)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.9670596f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.9670596f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1108 && (int)m_vecPosition.y == 2142
        || (int)m_vecPosition.x == 1107 && (int)m_vecPosition.y == 2141)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.7453293f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 1.7453293f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1095 && (int)m_vecPosition.y == 2107
        || (int)m_vecPosition.x == 1094 && (int)m_vecPosition.y == 2106)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.4434609f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 2.4434609f, 0, 0, 0, 0);
    }
    else if ((int)m_vecPosition.x == 1060 && (int)m_vecPosition.y == 2090
        || (int)m_vecPosition.x == 1059 && (int)m_vecPosition.y == 2089)
    {
        if (m_pTitleProgressBar->IsVisible())
            pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 3.3161256f, 0, 0, 0, 0);
        else
            pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 3.3161256f, 0, 0, 0, 0);
    }
    else if (m_pTitleProgressBar->IsVisible())
    {
        pMesh->RenderPick(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }
    else
    {
        pMesh->Render(m_vecPosition.x, m_fHeight, m_vecPosition.y, 0, 0, 0, 0, 0);
    }

    LabelPosition2();
    return 1;
}

/// Testa intersecao do cursor do mouse com a caixa de selecao da entidade no mundo 3D.
int TMHuman::IsMouseOver()
{
    if (m_dwDelayDel)
        return 0;

    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELECT_SERVER)
    {
        m_bMouseOver = 0;
        return 0;
    }
    if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_DEMO)
    {
        m_bMouseOver = 0;
        return 0;
    }
    if (m_cHide || m_cShadow == 1 && g_pCurrentScene->m_pMyHuman != this && !g_pCurrentScene->m_pMyHuman->m_JewelGlasses)
    {
        m_bMouseOver = 0;
        return 0;
    }

    if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD || g_pObjectManager->m_cShortSkill[g_pObjectManager->m_cSelectShortSkill] != 31)
    {
        if (m_eMotion == ECHAR_MOTION::ECMOTION_DEAD || m_eMotion == ECHAR_MOTION::ECMOTION_DIE || m_cDie == 1)
        {
            if (g_pCurrentScene->m_pMouseOverHuman == this)
                g_pCurrentScene->m_pMouseOverHuman = nullptr;

            m_bMouseOver = 0;
            return 0;
        }
    }
    else
    {
        if (m_eMotion != ECHAR_MOTION::ECMOTION_DEAD && m_eMotion != ECHAR_MOTION::ECMOTION_DIE && m_cDie != 1)
        {
            if (g_pCurrentScene->m_pMouseOverHuman == this)
                g_pCurrentScene->m_pMouseOverHuman = nullptr;

            m_bMouseOver = 0;
            return 0;
        }
    }

    D3DXVECTOR3 vPickRayDir{};
    D3DXVECTOR3 vPickRayOrig{};
    g_pDevice->GetPickRayVector(&vPickRayOrig, &vPickRayDir);

    TMVector3 vecCam = g_pObjectManager->m_pCamera->m_cameraPos;

    TMHuman* pOldOverHuman = g_pCurrentScene->m_pMouseOverHuman;
    TMHuman* pFocusedObject = g_pCurrentScene->m_pMyHuman;

    bool bMouseOver = 0;

    int nMeshType = m_nSkinMeshType;
    if (m_cMount > 0)
        nMeshType = m_nMountSkinMeshType;

    int nCommon = m_stLookInfo.LeftMesh;
    TMMesh* pMesh = g_pMeshManager->GetCommonMesh(nCommon, 0, 3_min);


    if (m_pNameLabel && m_pNameLabel->IsOver())
    {
        if (!pFocusedObject)
        {
            g_pCurrentScene->m_pMouseOverHuman = this;
            m_bMouseOver = 1;
        }

        bMouseOver = 1;
    }

    D3DXVECTOR3 v0{};
    D3DXVECTOR3 v1{};
    D3DXVECTOR3 v2{};
    D3DXVECTOR3 v3{};

    float fRadius = (TMHuman::m_vecPickSize[nMeshType].x * m_fScale) + 0.5f;

    if (pMesh && (nCommon == 2888 || nCommon == 2889))
    {
        if (nCommon == 2888)
        {
            fRadius = 3.0f;
        }
        else if (nCommon == 2889)
        {
            fRadius = 4.0f;
        }

        if (!bMouseOver)
        {
            v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
            v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
            v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
            v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

            if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
            {
                if (!pFocusedObject)
                {
                    g_pCurrentScene->m_pMouseOverHuman = this;
                    m_bMouseOver = 1;
                }
                bMouseOver = 1;
            }
            if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
            {
                if (!pFocusedObject)
                {
                    g_pCurrentScene->m_pMouseOverHuman = this;
                    m_bMouseOver = 1;
                }
                bMouseOver = 1;
            }
        }
    }
    if (IAmkhepra() == 1)
    {
        fRadius -= 0.2f;

        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y);
        v1 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y);
        v2 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y);

        if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (!bMouseOver)
    {
        fRadius = (TMHuman::m_vecPickSize[nMeshType].x * m_fScale) * 1.0f;
        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }

        D3DXVECTOR3 v4{};
        D3DXVECTOR3 v5{};
        D3DXVECTOR3 v6{};
        D3DXVECTOR3 v7{};

        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y - fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v4 = D3DXVECTOR3(m_vecPosition.x - fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v5 = D3DXVECTOR3(m_vecPosition.x + fRadius, (TMHuman::m_vecPickSize[nMeshType].y * m_fScale) + m_fHeight, m_vecPosition.y + fRadius);
        v6 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
        v7 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (!bMouseOver && D3DXIntersectTri(&v4, &v0, &v2, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v6, &v4, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v0, &v1, &v3, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v3, &v2, &v0, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v1, &v5, &v7, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v7, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v5, &v4, &v6, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v6, &v7, &v5, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (!bMouseOver && m_nClass == 56 && !m_stLookInfo.FaceMesh)
    {
        v0 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v1 = D3DXVECTOR3(m_vecPosition.x - fRadius, m_fHeight, m_vecPosition.y + fRadius);
        v2 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y - fRadius);
        v3 = D3DXVECTOR3(m_vecPosition.x + fRadius, m_fHeight, m_vecPosition.y + fRadius);

        if (!bMouseOver && D3DXIntersectTri(&v0, &v2, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
        if (!bMouseOver && D3DXIntersectTri(&v2, &v3, &v1, &vPickRayOrig, &vPickRayDir, 0, 0, 0) == 1)
        {
            if (!pFocusedObject)
            {
                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
            bMouseOver = 1;
        }
    }
    if (g_pObjectManager->m_pCamera && pFocusedObject != this)
    {
        if (bMouseOver == 1)
        {
            TMVector2 vec2{ vecCam.x, vecCam.z };
            if (!pOldOverHuman || vec2.DistanceFrom(pOldOverHuman->m_vecPosition) > vec2.DistanceFrom(m_vecPosition))
            {
                if (g_pCurrentScene->m_pMouseOverHuman)
                    g_pCurrentScene->m_pMouseOverHuman->m_bMouseOver = 0;

                g_pCurrentScene->m_pMouseOverHuman = this;
                m_bMouseOver = 1;
            }
        }
        else if (g_pCurrentScene->m_pMouseOverHuman == this)
        {
            g_pCurrentScene->m_pMouseOverHuman = 0;
            m_bMouseOver = 1;
        }
    }
    if (bMouseOver == 1)
    {
        float fDis = TMVector2(vecCam.x, vecCam.z).DistanceFrom(m_vecPosition);

        if (fDis < 1.5f)
        {
            bMouseOver = 0;
            m_bMouseOver = 0;
            g_pCurrentScene->m_pMouseOverHuman = pOldOverHuman;
        }
        else if (pOldOverHuman)
        {
            if (pFocusedObject != this)
            {
                if (m_dwID >= 0 && m_dwID < 1000)
                {
                    if ((pOldOverHuman->m_dwID < 0 || pOldOverHuman->m_dwID > 1000) && pOldOverHuman->m_cSummons == 1)
                    {
                        if (g_pCurrentScene->m_pMouseOverHuman)
                            g_pCurrentScene->m_pMouseOverHuman->m_bMouseOver = 0;

                        m_bMouseOver = 1;
                        g_pCurrentScene->m_pMouseOverHuman = this;
                    }
                }
            }
        }
    }
    if (!m_bMouseOver && bMouseOver == 1)
    {
        TMScene* pScene = g_pCurrentScene;
        if (pScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        {
            auto pSoundManager = g_pSoundManager;
            if (pSoundManager)
            {
                auto pSoundData = pSoundManager->GetSoundData(52);
                if (pSoundData->IsSoundPlaying())
                {
                    pSoundData->Play();
                }
            }
        }        
    }

    m_bMouseOver = bMouseOver;
    return bMouseOver;
}

void TMHuman::LabelPosition()
{
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pFScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return;

    if (m_dwDelayDel)
        return;

    if (m_cHide == 1 || m_cShadow == 1 && pFScene->m_pMyHuman != this &&
        !pFScene->m_pMyHuman->m_JewelGlasses)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible( 0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pTitleProgressBar->SetVisible(0);
        m_pTitleNameLabel->SetVisible(0);
        return;
    }

    if (pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
    {
        if (m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 && 
            m_sMountIndex != 29 && m_sMountIndex != 30 && !pFScene->m_bAirMove)
        {
            pFScene->m_pMHPBar->SetVisible(1);
            pFScene->m_pMHPBarT->SetVisible(1);
        }
        else
        {
            pFScene->m_pMHPBar->SetVisible(0);
            pFScene->m_pMHPBarT->SetVisible(0);
        }
    }

    m_pTitleNameLabel->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    bool bTargetMob = false;
    if (pFScene->m_pMyHuman && m_dwID == pFScene->m_pMyHuman->m_nAttackDestID && !m_cDie)
    {
        bTargetMob = 1;
        m_pTitleProgressBar->SetVisible(1);
    }
    if (pFScene->m_pMouseOverHuman != this && 
        (m_sHeadIndex == 216 || m_sHeadIndex == 226 || m_sHeadIndex == 298))
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
        return;
    }

    if (g_nUpdateGuildName > 0)
    {
        if (m_dwID >= 0 && m_dwID < 1000 && (int)m_usGuild > 0)
            UpdateGuildName();
    }

    if (m_pNameLabel)
    {
        if (g_bEvent == 1)
            return;

        if (pFScene->m_pMouseOverHuman != this
            && (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26 && 
                (m_nClass != 33 || m_stLookInfo.FaceMesh) || 
                (m_dwID < 0 || m_dwID >= 1000)) && 
            !IsMerchant() && 
            m_bParty != 1 && 
            (int)m_usGuild <= 0 && 
            (m_dwID < 0 || m_dwID >= 1000) &&
            (m_sHeadIndex != 271 || !(m_stScore.Merchant & 0xF)) && 
            bTargetMob != 1)
        {
            m_pNameLabel->SetVisible(0);
            m_pKillLabel->SetVisible(0);
            m_pProgressBar->SetVisible(0);
            m_pMountHPBar->SetVisible(0);
            if (m_stGuildMark.pGuildMark)
                m_stGuildMark.pGuildMark->SetVisible(0);
            m_pAutoTradeDesc->SetVisible(0);
            m_pAutoTradePanel->SetVisible(0);
            m_pNickNameLabel->SetVisible(0);
            m_pTitleProgressBar->SetVisible( 0);
            m_pTitleNameLabel->SetVisible(0);
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
            {
                m_pTitleProgressBar->SetVisible(1);
                m_pTitleNameLabel->SetVisible(1);
            }          
        }
        else
        {
            D3DXVECTOR3 vTemp;
            D3DXVECTOR3 vPosTransformed;
            D3DXVECTOR3 vecPos;

            vecPos.x = m_vecPosition.x;
            vecPos.z = m_vecPosition.y;
            vecPos.y = ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight) + 0.2f;
            D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
            D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);
            if (vPosTransformed.z < 0.0f || vPosTransformed.z >= 1.0f)
            {
                m_pNameLabel->SetVisible(0);
                m_pKillLabel->SetVisible(0);
                m_pProgressBar->SetVisible(0);
                m_pMountHPBar->SetVisible(0);

                if (m_stGuildMark.pGuildMark)
                    m_stGuildMark.pGuildMark->SetVisible(0);

                m_pAutoTradeDesc->SetVisible(0);
                m_pAutoTradePanel->SetVisible(0);
                m_pNickNameLabel->SetVisible( 0);
                m_pChatMsg->SetVisible(0);
                m_pTitleProgressBar->SetVisible(0);
                m_pTitleNameLabel->SetVisible(0);
                if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                {
                    m_pTitleProgressBar->SetVisible(1);
                    m_pTitleNameLabel->SetVisible(1);
                }
            }
            else
            {
                int vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f);
                int vPosInY = 0;
                if (m_cMount)
                    vPosInY = (int)((-vPosTransformed.y + 0.76f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    + (int)(g_pObjectManager->m_pCamera->m_fSightLength * 3.5f);
                else
                    vPosInY = (int)((-vPosTransformed.y + 1.16f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    - 3 * (int)g_pObjectManager->m_pCamera->m_fSightLength;

                if (vPosInX <= 0 || vPosInX >= (int)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) || 
                    vPosInY <= 0 || vPosInY >= (int)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
                {
                    m_pNameLabel->SetVisible(0);
                    m_pKillLabel->SetVisible(0);
                    m_pProgressBar->SetVisible(0);
                    m_pMountHPBar->SetVisible(0);
                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetVisible(0);
                    m_pAutoTradeDesc->SetVisible(0);
                    m_pAutoTradePanel->SetVisible(0);
                    m_pNickNameLabel->SetVisible(0);
                    m_pChatMsg->SetVisible(0);
                    m_pTitleProgressBar->SetVisible(0);
                    m_pTitleNameLabel->SetVisible(0);
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                }
                else
                {
                    if (!m_ucChaosLevel)
                    {                   
                        float fProgress = (float)(g_pTimerManager->GetServerTime() % 2000);
                        fProgress = sinf((float)(fProgress * D3DXToRadian(180)) / 2000.0f);
                        m_pNameLabel->SetTextColor(((unsigned int)(float)(fProgress * 255.0f) << 16) | 0xFF000000);
                    }

                    short sLevel = pFScene->m_pMyHuman->m_stScore.Level;
                    if (pFScene->m_pMyHuman->Is2stClass() == 2)
                        sLevel += 400;

                    if (sLevel - 40 >= m_stScore.Level && 
                        m_sHeadIndex > 40 && m_stScore.Level < 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF737373;
                    }
                    if (sLevel - 40 < m_stScore.Level && 
                        m_sHeadIndex > 40 || m_stScore.Level >= 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                    }
                    m_pNameLabel->SetVisible(1);
                    if (m_pTitleProgressBar->m_bVisible == 1)
                        m_pTitleNameLabel->SetVisible(1);

                    if (m_stGuildMark.pGuildMark && m_stGuildMark.nGuild != -1 && 
                        m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex >= 0)
                    {
                        int nMark = m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex;
                        if (nMark < 0 || nMark > 64 || m_stGuildMark.nGuild == (g_pTextureManager->m_stGuildMark[nMark].nGuild & 0xFFFF) && 
                            g_pTextureManager->m_stGuildMark[nMark].nGuild >> 16 == m_stGuildMark.nGuildChannel)
                        {
                            if (!m_stGuildMark.bHideGuildmark)
                                m_stGuildMark.pGuildMark->SetVisible(1);
                        }
                        else
                        {
                            m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex = -1;
                            m_stGuildMark.pGuildMark->SetVisible(0);
                        }
                    }
                    m_pNickNameLabel->SetVisible(1);
                    if (m_cDie == 1)
                        m_pNameLabel->SetVisible(0);

                    if (m_dwID >= 0 && m_dwID < 1000 && (m_nCurrentKill > 0 || (int)m_nTotalKill > 0))
                    {
                        m_pKillLabel->SetVisible(1);
                        if ((int)m_vecPosition.x >> 7 > 16 && (int)m_vecPosition.x >> 7 < 20 && (int)m_vecPosition.y >> 7 > 29 &&
                            pFScene->m_pMyHuman != this)
                        {
                            m_pKillLabel->SetVisible(0);
                        }

                        if ((int)m_vecPosition.x >> 7 == 17 && (int)m_vecPosition.y >> 7 == 28)
                            m_pKillLabel->SetVisible(0);
                    }

                    if (m_TradeDesc[0])
                    {
                        m_pAutoTradeDesc->SetVisible(1);
                        m_pAutoTradePanel->SetVisible(1);
                    }
                    else
                    {
                        m_pAutoTradeDesc->SetVisible(0);
                        m_pAutoTradePanel->SetVisible(0);
                    }
                    if (g_pCurrentScene->m_pMyHuman != this)
                    {
                        if (m_nClass != 56 || m_stLookInfo.FaceMesh)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                        }
                    }

                    vPosInY = (int)((float)vPosInY + (RenderDevice::m_fHeightRatio * 13.0f));
                    float fWidthRatio = RenderDevice::m_fWidthRatio;
                    if (m_cMount)
                        vPosInY = vPosInY + (int)((RenderDevice::m_fHeightRatio * 2.0f) * 4.0f);
                    else
                        vPosInY = vPosInY - (int)((RenderDevice::m_fHeightRatio * 2.0f) * 16.0f);

                    if (1.0 == RenderDevice::m_fHeightRatio)
                    {
                        if (m_cMount)
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 8.0f);
                        else
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 10.0f);
                    }
                    else if (RenderDevice::m_fHeightRatio >= 1.7)
                    {
                        if (m_cMount)
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 15.0f);
                        else
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 6.0f);
                    }
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                    else
                    {
                        if (m_cMount)
                            m_pProgressBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f), 
                                ((float)vPosInY - BASE_ScreenResize(10.0f)) - 15.0f);
                        else
                            m_pProgressBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f), 
                                (float)vPosInY - BASE_ScreenResize(10.0f));

                        m_pMountHPBar->SetRealPos((float)vPosInX - BASE_ScreenResize(35.0f), 
                            ((float)vPosInY - BASE_ScreenResize(16.0f)) - 14.0f);

                        m_pProgressBar->SetSize(BASE_ScreenResize(72.0f), BASE_ScreenResize(2.0f) + 6.0f);
                        m_pMountHPBar->SetSize(BASE_ScreenResize(72.0f), BASE_ScreenResize(2.0f) + 5.0f);

                        m_pMountHPBar->Update();
                        m_pProgressBar->Update();
                        m_pProgressBar->m_GCProgress.nWidth = m_pProgressBar->m_nProgressWidth;
                        m_pProgressBar->m_GCProgress.nHeight = m_pProgressBar->m_nHeight - 4.0f;
                        m_pMountHPBar->m_GCProgress.nWidth = m_pMountHPBar->m_nProgressWidth;
                        m_pMountHPBar->m_GCProgress.nHeight = m_pMountHPBar->m_nHeight - 4.0f;
                    }

                    int nLen2 = strlen(m_pNameLabel->GetText());
                    int nLen3 = strlen(m_TradeDesc);
                    int nLen4 = strlen(m_pNickNameLabel->GetText());

                    m_pChatMsg->SetRealPos((float)vPosInX - (float)(m_pChatMsg->m_nWidth / 2.0f), (float)vPosInY - 60.0f);
                    m_pAutoTradeDesc->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f), (float)vPosInY - (float)(3.0f * RenderDevice::m_fHeightRatio));
                    m_pAutoTradePanel->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f), (float)vPosInY - (float)(3.0f * RenderDevice::m_fHeightRatio));

                    float nPosY = (float)vPosInY;
                    if (m_cMount && !m_pAutoTradeDesc->IsVisible())
                        nPosY = nPosY - 20.0f;

                    fWidthRatio = RenderDevice::m_fWidthRatio;
                    m_pNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen2 + 2)) * RenderDevice::m_fWidthRatio) / 2.0f), 
                        nPosY);

                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetRealPos(((float)vPosInX - (((float)(6 * (nLen2 + 2)) * fWidthRatio) / 2.0f)) - 10.0f, 
                            nPosY + 2.0f);

                    m_pNickNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen4 + 2)) * fWidthRatio) / 2.0f), 
                        nPosY - (float)(28.0f * fWidthRatio));

                    if ((int)m_vecPosition.x >> 7 > 1 && (int)m_vecPosition.x >> 7 < 11 && (int)m_vecPosition.y >> 7 < 5)
                    {
                        m_pProgressBar->SetVisible(1);
                        m_pMountHPBar->SetVisible(0);
                        m_pTitleProgressBar->SetVisible(0);
                        m_pTitleNameLabel->SetVisible(0);
                    }

                    else if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26 || m_nClass == 33 && 
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Merchant & 0xF)
                    {
                        if (_locationCheck(m_vecPosition, 14, 28) && m_sHeadIndex == 51)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                            m_pTitleProgressBar->SetVisible(0);
                            m_pTitleNameLabel->SetVisible(0);
                        }
                    }

                    auto pFocused = g_pCurrentScene->m_pMyHuman;
                    if (pFocused)
                    {
                        if (!IsMerchant() && m_sHeadIndex != 57 && !m_pAutoTradeDesc->IsVisible())
                        {
                            m_pProgressBar->SetVisible(1);
                            if (g_pCurrentScene->m_pMyHuman != this && g_pCurrentScene->m_pMouseOverHuman == this)
                            {
                                if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                                    m_pProgressBar->SetVisible(0);

                                m_pTitleNameLabel->SetVisible(1);
                                m_pTitleProgressBar->SetVisible(1);
                            }
                        }

                        if (pFocused == this && m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 && m_sMountIndex != 29 && m_sMountIndex != 30)
                            m_pMountHPBar->SetVisible(1);
                        else
                            m_pMountHPBar->SetVisible(0);

                        if (pFocused == this&& m_cMount == 1)
                        {
                            int MountIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex;
                            if (MountIndex >= 3980 && MountIndex <= 3999)
                                m_pMountHPBar->SetVisible(0);
                        }

                        m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;

                        if (g_bCastleWar)
                        {
                            if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                        }
                        else if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                        {
                            if (m_dwID < 0 || m_dwID >= 1000 && !TMFieldScene::m_bPK)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                        }
                        if (pFocused != this && m_citizen == pFocused->m_citizen && m_sHeadIndex < 40)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                        }
                        if (IsInCastleZone2() && !g_bCastleWar)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        }
                        if (pFocused == this || m_bParty == 1 || m_usGuild && 
                            (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF006400;
                        }
                    }
                }
            }
        }
    }
    if (IsInTown() == 1 && m_nClass == 16)
        m_pProgressBar->SetVisible(0);

    if (m_pAutoTradeDesc->IsVisible() == 1)
    {
        m_pProgressBar->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_stGuildMark.pGuildMark->SetVisible(0);
    }
    else if (!m_stGuildMark.bLoadedGuildmark)
    {
        if (!strlen(m_TradeDesc) && m_stGuildMark.sGuildIndex && m_stGuildMark.nGuild != -1 && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    if (m_bIgnoreHeight == 1)
        m_pMountHPBar->SetVisible(0);

    if (!pFScene->m_bShowNameLabel && pFScene->m_pMouseOverHuman != this && !bTargetMob)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);
        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);
        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
    }
}

void TMHuman::LabelPosition2()
{
    auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

    if (!pFScene || g_pCurrentScene->GetSceneType() != ESCENE_TYPE::ESCENE_FIELD)
        return;

    if (m_dwDelayDel)
        return;

    if (m_cHide == 1 || m_cShadow == 1 && pFScene->m_pMyHuman != this &&
        !pFScene->m_pMyHuman->m_JewelGlasses)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pTitleProgressBar->SetVisible(0);
        m_pTitleNameLabel->SetVisible(0);
        return;
    }

    if (pFScene->m_pMHPBar && pFScene->m_pMHPBarT)
    {
        if (m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 &&
            m_sMountIndex != 29 && m_sMountIndex != 30 && !pFScene->m_bAirMove)
        {
            pFScene->m_pMHPBar->SetVisible(1);
            pFScene->m_pMHPBarT->SetVisible(1);
        }
        else
        {
            pFScene->m_pMHPBar->SetVisible(0);
            pFScene->m_pMHPBarT->SetVisible(0);
        }
    }

    m_pTitleNameLabel->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    bool bTargetMob = false;
    if (pFScene->m_pMyHuman && m_dwID == pFScene->m_pMyHuman->m_nAttackDestID && !m_cDie)
    {
        bTargetMob = 1;
        m_pTitleProgressBar->SetVisible(1);
    }
    if (pFScene->m_pMouseOverHuman != this &&
        (m_sHeadIndex == 216 || m_sHeadIndex == 226 || m_sHeadIndex == 298))
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pKillLabel->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
        return;
    }

    if (g_nUpdateGuildName > 0)
    {
        if (m_dwID >= 0 && m_dwID < 1000 && (int)m_usGuild > 0)
            UpdateGuildName();
    }

    if (m_pNameLabel)
    {
        if (g_bEvent == 1)
            return;

        if (pFScene->m_pMouseOverHuman != this
            && (m_nClass != 1 && m_nClass != 2 && m_nClass != 4 && m_nClass != 8 && m_nClass != 26 &&
                (m_nClass != 33 || m_stLookInfo.FaceMesh) ||
                (m_dwID < 0 || m_dwID >= 1000)) &&
            !IsMerchant() &&
            m_bParty != 1 &&
            (int)m_usGuild <= 0 &&
            (m_dwID < 0 || m_dwID >= 1000) &&
            (m_sHeadIndex != 271 || !(m_stScore.Merchant & 0xF)) &&
            bTargetMob != 1)
        {
            m_pNameLabel->SetVisible(0);
            m_pKillLabel->SetVisible(0);
            m_pProgressBar->SetVisible(0);
            m_pMountHPBar->SetVisible(0);
            if (m_stGuildMark.pGuildMark)
                m_stGuildMark.pGuildMark->SetVisible(0);
            m_pAutoTradeDesc->SetVisible(0);
            m_pAutoTradePanel->SetVisible(0);
            m_pNickNameLabel->SetVisible(0);
            m_pTitleProgressBar->SetVisible(0);
            m_pTitleNameLabel->SetVisible(0);
            if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
            {
                m_pTitleProgressBar->SetVisible(1);
                m_pTitleNameLabel->SetVisible(1);
            }
        }
        else
        {
            D3DXVECTOR3 vTemp;
            D3DXVECTOR3 vPosTransformed;
            D3DXVECTOR3 vecPos;

            vecPos.x = m_vecPosition.x;
            vecPos.z = m_vecPosition.y;
            vecPos.y = ((TMHuman::m_vecPickSize[m_nSkinMeshType].y * m_fScale) + m_fHeight) + 0.2f;
            D3DXVec3TransformCoord(&vTemp, &vecPos, &g_pDevice->m_matView);
            D3DXVec3TransformCoord(&vPosTransformed, &vTemp, &g_pDevice->m_matProj);
            if (vPosTransformed.z < 0.0f || vPosTransformed.z >= 1.0f)
            {
                m_pNameLabel->SetVisible(0);
                m_pKillLabel->SetVisible(0);
                m_pProgressBar->SetVisible(0);
                m_pMountHPBar->SetVisible(0);

                if (m_stGuildMark.pGuildMark)
                    m_stGuildMark.pGuildMark->SetVisible(0);

                m_pAutoTradeDesc->SetVisible(0);
                m_pAutoTradePanel->SetVisible(0);
                m_pNickNameLabel->SetVisible(0);
                m_pChatMsg->SetVisible(0);
                m_pTitleProgressBar->SetVisible(0);
                m_pTitleNameLabel->SetVisible(0);
                if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                {
                    m_pTitleProgressBar->SetVisible(1);
                    m_pTitleNameLabel->SetVisible(1);
                }
            }
            else
            {
                int vPosInX = (int)(((vPosTransformed.x + 1.0f) * (float)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift)) / 2.0f);
                int vPosInY = 0;
                if (m_cMount)
                    vPosInY = (int)((-vPosTransformed.y + 0.76f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    + (int)(g_pObjectManager->m_pCamera->m_fSightLength * 3.5f);
                else
                    vPosInY = (int)((-vPosTransformed.y + 1.16f) * (float)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift - 1) / 2.0f)
                    - 3 * (int)g_pObjectManager->m_pCamera->m_fSightLength;

                if (vPosInX <= 0 || vPosInX >= (int)(g_pDevice->m_dwScreenWidth - g_pDevice->m_nWidthShift) ||
                    vPosInY <= 0 || vPosInY >= (int)(g_pDevice->m_dwScreenHeight - g_pDevice->m_nHeightShift))
                {
                    m_pNameLabel->SetVisible(0);
                    m_pKillLabel->SetVisible(0);
                    m_pProgressBar->SetVisible(0);
                    m_pMountHPBar->SetVisible(0);
                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetVisible(0);
                    m_pAutoTradeDesc->SetVisible(0);
                    m_pAutoTradePanel->SetVisible(0);
                    m_pNickNameLabel->SetVisible(0);
                    m_pChatMsg->SetVisible(0);
                    m_pTitleProgressBar->SetVisible(0);
                    m_pTitleNameLabel->SetVisible(0);
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh && m_cDie != 1)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                }
                else
                {
                    if (!m_ucChaosLevel)
                    {
                        float fProgress = (float)(g_pTimerManager->GetServerTime() % 2000);
                        fProgress = sinf((float)(fProgress * D3DXToRadian(180)) / 2000.0f);
                        m_pNameLabel->SetTextColor(((unsigned int)(float)(fProgress * 255.0f) << 16) | 0xFF000000);
                    }

                    short sLevel = pFScene->m_pMyHuman->m_stScore.Level;
                    if (pFScene->m_pMyHuman->Is2stClass() == 2)
                        sLevel += 400;

                    if (sLevel - 40 >= m_stScore.Level &&
                        m_sHeadIndex > 40 && m_stScore.Level < 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF737373;
                    }
                    if (sLevel - 40 < m_stScore.Level &&
                        m_sHeadIndex > 40 || m_stScore.Level >= 350 && m_sHeadIndex > 40)
                    {
                        m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                    }
                    m_pNameLabel->SetVisible(1);
                    if (m_pTitleProgressBar->m_bVisible == 1)
                        m_pTitleNameLabel->SetVisible(1);

                    if (m_stGuildMark.pGuildMark && m_stGuildMark.nGuild != -1 &&
                        m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex >= 0)
                    {
                        int nMark = m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex;
                        if (nMark < 0 || nMark > 64 || m_stGuildMark.nGuild == (g_pTextureManager->m_stGuildMark[nMark].nGuild & 0xFFFF) &&
                            g_pTextureManager->m_stGuildMark[nMark].nGuild >> 16 == m_stGuildMark.nGuildChannel)
                        {
                            if (!m_stGuildMark.bHideGuildmark)
                                m_stGuildMark.pGuildMark->SetVisible(1);
                        }
                        else
                        {
                            m_stGuildMark.pGuildMark->m_GCPanel.nMarkIndex = -1;
                            m_stGuildMark.pGuildMark->SetVisible(0);
                        }
                    }
                    m_pNickNameLabel->SetVisible(1);
                    if (m_cDie == 1)
                        m_pNameLabel->SetVisible(0);

                    if (m_dwID >= 0 && m_dwID < 1000 && (m_nCurrentKill > 0 || (int)m_nTotalKill > 0))
                    {
                        m_pKillLabel->SetVisible(1);
                        if ((int)m_vecPosition.x >> 7 > 16 && (int)m_vecPosition.x >> 7 < 20 && (int)m_vecPosition.y >> 7 > 29 &&
                            pFScene->m_pMyHuman != this)
                        {
                            m_pKillLabel->SetVisible(0);
                        }

                        if ((int)m_vecPosition.x >> 7 == 17 && (int)m_vecPosition.y >> 7 == 28)
                            m_pKillLabel->SetVisible(0);
                    }

                    if (m_TradeDesc[0])
                    {
                        m_pAutoTradeDesc->SetVisible(1);
                        m_pAutoTradePanel->SetVisible(1);
                    }
                    else
                    {
                        m_pAutoTradeDesc->SetVisible(0);
                        m_pAutoTradePanel->SetVisible(0);
                    }
                    if (g_pCurrentScene->m_pMyHuman != this)
                    {
                        if (m_nClass != 56 || m_stLookInfo.FaceMesh)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                        }
                    }

                    vPosInY = (int)((float)vPosInY + (RenderDevice::m_fHeightRatio * 13.0f));
                    float fWidthRatio = RenderDevice::m_fWidthRatio;
                    if (m_cMount)
                        vPosInY = vPosInY + (int)((RenderDevice::m_fHeightRatio * 2.0f) * 4.0f);
                    else
                        vPosInY = vPosInY - (int)((RenderDevice::m_fHeightRatio * 2.0f) * 16.0f);

                    if (1.0 == RenderDevice::m_fHeightRatio)
                    {
                        if (m_cMount)
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 8.0f);
                        else
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 10.0f);
                    }
                    else if (RenderDevice::m_fHeightRatio >= 1.7)
                    {
                        if (m_cMount)
                            vPosInY += (int)(RenderDevice::m_fHeightRatio * 15.0f);
                        else
                            vPosInY -= (int)(RenderDevice::m_fHeightRatio * 6.0f);
                    }
                    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                    {
                        m_pTitleProgressBar->SetVisible(1);
                        m_pTitleNameLabel->SetVisible(1);
                    }
                    else
                    {
                        if (m_cMount)
                            m_pProgressBar->SetRealPos((float)vPosInX - 34.0f,
                                ((float)vPosInY - 10.0f) + 5.0f);
                        else
                            m_pProgressBar->SetRealPos((float)vPosInX - 34.0f,
                                (float)vPosInY - 25.0f);

                        m_pMountHPBar->SetRealPos((float)vPosInX - 34.0f,
                            ((float)vPosInY - 30.0f));

                        m_pProgressBar->SetSize(72.0f, 7.0f);
                        m_pMountHPBar->SetSize(72.0f, 7.0f);

                        m_pMountHPBar->Update();
                        m_pProgressBar->Update();
                        m_pProgressBar->m_GCProgress.nWidth = m_pProgressBar->m_nProgressWidth;
                        m_pProgressBar->m_GCProgress.nHeight = m_pProgressBar->m_nHeight - 4.0f;
                        m_pMountHPBar->m_GCProgress.nWidth = m_pMountHPBar->m_nProgressWidth;
                        m_pMountHPBar->m_GCProgress.nHeight = m_pMountHPBar->m_nHeight - 4.0f;
                    }

                    int nLen2 = strlen(m_pNameLabel->GetText());
                    int nLen3 = strlen(m_TradeDesc);
                    int nLen4 = strlen(m_pNickNameLabel->GetText());

                    m_pChatMsg->SetRealPos((float)vPosInX - (float)(m_pChatMsg->m_nWidth / 2.0f), (float)vPosInY - 60.0f);
                    m_pAutoTradeDesc->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f), (float)vPosInY - (float)(3.0f * RenderDevice::m_fHeightRatio));
                    m_pAutoTradePanel->SetRealPos((float)vPosInX - ((150.0f * fWidthRatio) / 2.0f), (float)vPosInY - (float)(3.0f * RenderDevice::m_fHeightRatio));

                    float nPosY = (float)vPosInY;
                    if (m_cMount && !m_pAutoTradeDesc->IsVisible())
                        nPosY = nPosY - 20.0f;

                    fWidthRatio = RenderDevice::m_fWidthRatio;
                    m_pNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen2 + 2)) * RenderDevice::m_fWidthRatio) / 2.0f),
                        nPosY);

                    if (m_stGuildMark.pGuildMark)
                        m_stGuildMark.pGuildMark->SetRealPos(((float)vPosInX - (((float)(6 * (nLen2 + 2)) * fWidthRatio) / 2.0f)) - 10.0f,
                            nPosY + 2.0f);

                    m_pNickNameLabel->SetRealPos((float)vPosInX - (((float)(6 * (nLen4 + 2)) * fWidthRatio) / 2.0f),
                        nPosY - (float)(28.0f * fWidthRatio));

                    if ((int)m_vecPosition.x >> 7 > 1 && (int)m_vecPosition.x >> 7 < 11 && (int)m_vecPosition.y >> 7 < 5)
                    {
                        m_pProgressBar->SetVisible(1);
                        m_pMountHPBar->SetVisible(0);
                        m_pTitleProgressBar->SetVisible(0);
                        m_pTitleNameLabel->SetVisible(0);
                    }

                    else if (m_nClass == 1 || m_nClass == 2 || m_nClass == 4 || m_nClass == 8 || m_nClass == 26 || m_nClass == 33 &&
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Merchant & 0xF)
                    {
                        if (_locationCheck(m_vecPosition, 14, 28) && m_sHeadIndex == 51)
                        {
                            m_pProgressBar->SetVisible(1);
                            m_pMountHPBar->SetVisible(0);
                        }
                        else
                        {
                            m_pProgressBar->SetVisible(0);
                            m_pMountHPBar->SetVisible(0);
                            m_pTitleProgressBar->SetVisible(0);
                            m_pTitleNameLabel->SetVisible(0);
                        }
                    }

                    auto pFocused = g_pCurrentScene->m_pMyHuman;
                    if (pFocused)
                    {
                        if ((m_bParty == 1
                            || IsInPKZone() == 1
                            || pFocused->m_cMantua > 0 && m_cMantua > 0
                            || pFocused == this && m_cMount == 1) && 
                            !IsMerchant() && m_sHeadIndex != 57 && !m_pAutoTradeDesc->IsVisible())
                        {
                            m_pProgressBar->SetVisible(1);
                            if (g_pCurrentScene->m_pMyHuman != this && g_pCurrentScene->m_pMouseOverHuman == this)
                            {
                                if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
                                    m_pProgressBar->SetVisible(0);

                                m_pTitleNameLabel->SetVisible(1);
                                m_pTitleProgressBar->SetVisible(1);
                            }
                        }

                        if (pFocused == this && m_cMount == 1 && m_sMountIndex != 27 && m_sMountIndex != 28 && m_sMountIndex != 29 && m_sMountIndex != 30)
                            m_pMountHPBar->SetVisible(1);
                        else
                            m_pMountHPBar->SetVisible(0);

                        if (pFocused == this && m_cMount == 1)
                        {
                            int MountIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex;
                            if (MountIndex >= 3980 && MountIndex <= 3999)
                                m_pMountHPBar->SetVisible(0);
                        }

                        m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;

                        if (g_bCastleWar)
                        {
                            if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                        }
                        else if (pFocused->m_cMantua > 0 && m_cMantua > 0 && pFocused->m_cMantua == m_cMantua)
                        {
                            if (m_dwID < 0 || m_dwID >= 1000 && !TMFieldScene::m_bPK)
                                m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                        }
                        if (pFocused != this && m_citizen == pFocused->m_citizen && m_sHeadIndex < 40)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF00FF00;
                        }
                        if (IsInCastleZone2() && !g_bCastleWar)
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFFFF0000;
                        }
                        if (pFocused == this || m_bParty == 1 || m_usGuild &&
                            (m_usGuild == pFocused->m_usGuild || g_pObjectManager->m_usAllyGuild == m_usGuild))
                        {
                            m_pProgressBar->m_GCProgress.dwColor = 0xFF1E821E;
                            m_pTitleProgressBar->m_GCProgress.dwColor = 0xFF006400;
                        }
                    }
                }
            }
        }
    }
    if (m_pAutoTradeDesc->IsVisible() == 1)
    { 
        m_pKillLabel->SetVisible(0);
        m_stGuildMark.pGuildMark->SetVisible(0);
    }
    else if (!m_stGuildMark.bLoadedGuildmark)
    {
        if (!strlen(m_TradeDesc) && m_stGuildMark.sGuildIndex && m_stGuildMark.nGuild != -1 && !m_pAutoTradeDesc->IsVisible())
            pFScene->Guildmark_Create(&m_stGuildMark);
    }
    if (m_bIgnoreHeight == 1)
        m_pMountHPBar->SetVisible(0);
}

void TMHuman::HideLabel()
{
    m_pProgressBar->SetVisible(0);
    m_pMountHPBar->SetVisible(0);
    m_pNameLabel->SetVisible(0);
    if (m_stGuildMark.pGuildMark)
        m_stGuildMark.pGuildMark->SetVisible(0);

    m_pNickNameLabel->SetVisible(0);
    m_pAutoTradeDesc->SetVisible(0);
    m_pAutoTradePanel->SetVisible(0);
    m_pKillLabel->SetVisible(0);
    m_pChatMsg->SetVisible(0);
    m_pTitleProgressBar->SetVisible(0);
    m_pTitleNameLabel->SetVisible(0);
}

void TMHuman::SetChatMessage(const char* szString)
{
    if (!m_dwDelayDel)
    {
        int nHeight = 35;
        m_dwStartChatMsgTime = g_pTimerManager->GetServerTime();

        char temp[256]{ 0 };
        sprintf_s(temp, "%s", m_pNameLabel->GetText());

        GetChatLen(temp, &nHeight);
        sprintf_s(temp, "%s", szString);

        m_pChatMsg->SetText(temp, 0);

        float width = (float)GetChatLen(temp, &nHeight) * RenderDevice::m_fWidthRatio;
        m_pChatMsg->SetSize(width, (float)nHeight * RenderDevice::m_fHeightRatio);
    }
}

int TMHuman::GetChatLen(const char* szString, int* pHeight)
{
    if (m_dwDelayDel)
        return 0;

    int len = strlen(szString);
    int nLen = 0;
    if (len >= 41)
        nLen = 256;
    else
        nLen = 6 * len;

    if (len >= 41)
    {
        nLen = (int)((float)nLen * 1.0f);
        *pHeight = 50;
    }
    else
    {
        nLen = (int)((float)nLen * 1.0f) + 20;
        *pHeight = 40;
    }

	return nLen;
}

void TMHuman::SetInMiniMap(unsigned int dwCol)
{
    if (!m_pInMiniMap)
    {
        if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

            if (g_pCurrentScene->m_pMyHuman != this)
            {
                m_pInMiniMap = new SPanel(-2, 0.0f, 0.0f, 4.0f, 4.0f, dwCol, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

                if (m_pInMiniMap)
                {
                    m_pInMiniMap->m_bSelectEnable = 0;
                    pFScene->m_pMiniMapPanel->AddChild(m_pInMiniMap);
                }
            }
        }
    }
    else
        m_pInMiniMap->m_GCPanel.dwColor = dwCol;
}

void TMHuman::UpdateGuildName()
{
    if (!m_dwDelayDel)
    {
        if (!m_usGuild)
        {
            m_stGuildMark.bHideGuildmark = 1;
            m_stGuildMark.pGuildMark->SetVisible(0);
        }
        else
        {
            m_stGuildMark.nSubGuild = BASE_GetSubGuild(m_sGuildLevel);
            m_stGuildMark.nGuild = m_usGuild % 0xFFF;
            m_stGuildMark.nGuildChannel = (m_usGuild >> 12) & 0xF;
            m_stGuildMark.sGuildIndex = m_sGuildLevel;

            TMFieldScene* pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
            if (g_pCurrentScene)
            {
                if (!m_pAutoTradeDesc->IsVisible())
                    pFScene->Guildmark_Create(&m_stGuildMark);
            }

        }
    }
}

void TMHuman::CreateControl()
{
    DestroyControl();

    m_pNameLabel = new SText(-1, "MoName", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pKillLabel = new SText(-1, "", 0xFFFFFFAA, 0.0f, 650.0f, 128.0, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pAutoTradeDesc = new SText(-1, "", 0xFFFFFFFF, 0.0f, 650.0f, 143.0f, 50.0f, 0, 0xFFFFFFFF, 1u, 0);
    m_pAutoTradePanel = new SPanel(512, -10.0f, 635.0f, 141.0f, 43.0f, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
    m_pAutoTradePanel->m_bSelectEnable = 0;

    m_pChatMsg = new SText(-2, "", 0xFFFFFFFF, 0.0, 650.0, (float)256, 64.0, 1, 0x77000000u, 1u, 0);
    m_pChatMsg->m_Font.m_bMultiLine = 1;

    m_stGuildMark.pGuildMark = nullptr;

    m_stGuildMark.pGuildMark = new SPanel(-2, 0.0, 0.0, 12.0, 16.0, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
    m_pProgressBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 60.0f, 7.0f, 0xFFFF0000, 0xFF333333, 1u);
    m_pMountHPBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 60.0f, 7.0f, 0xFFFFAA00, 0xFF333333, 1u);

    m_pChatMsg->SetVisible(0);
    m_pNameLabel->m_GCBorder.nTextureSetIndex = -2;
    m_pNameLabel->SetVisible(0);
    m_pKillLabel->SetVisible(0);

    if (m_stGuildMark.pGuildMark)
        m_stGuildMark.pGuildMark->SetVisible(0);

    m_pAutoTradeDesc->SetVisible(0);

    m_pNickNameLabel = new SText(-1, "", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000u, 1u, 0);
    m_pNickNameLabel->m_GCBorder.nTextureSetIndex = -2;
    m_pNickNameLabel->SetVisible(0);
    m_pAutoTradePanel->SetVisible(0);
    m_pMountHPBar->SetVisible(0);

    if (g_pCurrentScene)
    {
        g_pCurrentScene->m_pControlContainer->AddItem(m_pNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pKillLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_stGuildMark.pGuildMark);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pAutoTradePanel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pNickNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pChatMsg);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pProgressBar);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pMountHPBar);
    }
}

void TMHuman::DestroyControl()
{
    SAFE_DELETE(m_pChatMsg);
    SAFE_DELETE(m_pNameLabel);
    SAFE_DELETE(m_pKillLabel);
    SAFE_DELETE(m_stGuildMark.pGuildMark);
    SAFE_DELETE(m_pAutoTradeDesc);
    SAFE_DELETE(m_pAutoTradePanel);
    SAFE_DELETE(m_pNickNameLabel);
    SAFE_DELETE(m_pProgressBar);
    SAFE_DELETE(m_pMountHPBar);
    SAFE_DELETE(m_pInMiniMap);
    SAFE_DELETE(m_pSkinMesh);
    SAFE_DELETE(m_pMantua);
    SAFE_DELETE(m_pMount);
    SAFE_DELETE(m_pMount);
}
