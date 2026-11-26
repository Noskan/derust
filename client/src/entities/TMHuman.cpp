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

/// @file TMHuman.cpp
/// @brief Ciclo de vida, renderizacao base, FrameMove, rotulos (labels) e estado da entidade TMHuman.

TMVector2 TMHuman::m_vecPickSize[100] = {
  { 0.40000001f, 2.0f },
  { 0.40000001f, 2.0f },
  { 0.5f, 2.0f },
  { 0.40000001f, 2.0f },
  { 0.5f, 2.0f },
  { 0.40000001f, 2.5f },
  { 0.69999999f, 1.0f },
  { 1.0f, 2.0f },
  { 0.69999999f, 2.0f },
  { 0.69999999f, 3.2f },
  { 0.69999999f, 2.0f },
  { 1.0f, 2.5f },
  { 0.40000001f, 2.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.2f, 2.2f },
  { 0.44999999f, 0.89999998f },
  { 0.40000001f, 1.3f },
  { 2.5f, 1.5f },
  { 0.40000001f, 1.0f },
  { 0.69999999f, 1.0f },
  { 0.60000002f, 2.5f },
  { 0.80000001f, 1.0f },
  { 0.69999999f, 1.0f },
  { 0.69999999f, 1.0f },
  { 1.4f, 1.2f },
  { 1.8f, 1.5f },
  { 0.5f, 0.2f },
  { 1.0f, 2.5f },
  { 2.0f, 2.0f },
  { 1.8f, 1.8f },
  { 2.0f, 1.8f },
  { 1.0f, 1.0f },
  { 1.0f, 1.8f },
  { 1.2f, 2.2f },
  { 0.80000001f, 1.0f },
  { 2.9000001f, 2.5f },
  { 1.4f, 3.0f },
  { 1.4f, 3.2f },
  { 0.40000001f, 3.5f },
  { 0.40000001f, 3.5f },
  { 0.40000001f, 2.0999999f },
  { 0.40000001f, 2.8f },
  { 0.40000001f, 2.5f },
  { 0.40000001f, 2.0f },
  { 0.40000001f, 2.0f },
  { 0.40000001f, 2.0f },
  { 0.40000001f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 1.0f, 1.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f },
  { 0.0f, 0.0f }
};

DWORD TMHuman::m_dwNameColor[9] =
{
  0xFF000000,
  0xFFFF0000,
  0xFFFFAA66,
  0xFFFFFF00,
  0xFFFFFF99,
  0xFF99FF99,
  0xFF99FFFF,
  0xFFFFFFFF,
  0xFFFFF6FF
};

TMHuman::TMHuman(TMScene* pParentScene)
{
	m_pParentScene = pParentScene;
	++g_nMobCount;
    m_vecStartPos.x = 0;
    m_vecStartPos.y = 0;
    m_sFamCount = 0;
    m_pSkinMesh = nullptr;
    m_pShade = nullptr;
    m_pMoveTargetHuman = nullptr;
    m_pMoveSkillTargetHuman = nullptr;
    m_pNameLabel = nullptr;
    m_pKillLabel = nullptr;
    m_pAutoTradePanel = nullptr;
    m_pAutoTradeDesc = nullptr;
    m_pChatMsg = nullptr;
    m_pProgressBar = nullptr;
    m_pInMiniMap = nullptr;
    m_pBlood = nullptr;
    m_pAurora = nullptr;
    m_pSkillAmp = nullptr;
    m_pbomb = nullptr;
    m_pProtector = nullptr;
    m_pFamiliar = nullptr;
    m_pLifeDrain = nullptr;
    m_pShadow = nullptr;
    m_pHuntersVision = nullptr;
    m_pOverExp = nullptr;
    m_pBraveOverExp = nullptr;
    m_pMantua = nullptr;
    m_pMountHPBar = nullptr;
    m_pEleStream = nullptr;
    m_pEleStream2 = nullptr;
    m_pRescue = nullptr;
    m_pMagicShield = nullptr;
    m_pCancelation = nullptr;
    m_pChargeEnergy = nullptr;
    m_sDelayDel = 0;
    m_cHide = 0;
    m_usGuild = 0;
    m_nMotionCount = 1;
    m_nClass = 1;
    m_sHeadIndex = 0;
    m_sHelmIndex = 0;
    m_cMantua = 0;
    m_cPowerUp = 0;
    m_sMountIndex = -1;
    m_sGuildLevel = 0;
    m_nLegType = 1;
    m_nWeaponTypeL = 0;
    m_nWeaponTypeR = 0;
    m_cSameHeight = 0;
    m_fMountScale = 1.0f;
    m_pMount = 0;
    m_cMount = 0;
    m_cClone = 0;
    m_cLastMount = 0;
    m_nMountSkinMeshType = 0;

    memset(&m_stMountLook, 0, sizeof(m_stMountLook));
    memset(&m_stMountSanc, 0, sizeof(m_stMountSanc));

    m_wMantuaSkin = 0;
    m_ucMantuaSanc = 0;
    m_ucMantuaLegend = 0;
    m_sFamiliar = 0;
    m_sCostume = 0;
    m_nCurrentKill = 0;
    m_ucChaosLevel = 75;
    m_nTotalKill = 0;
    m_cLegend = 0;
    m_cOnlyMove = 0;
    m_cSummons = 0;
    m_cLastMoveStop = 0;
    m_cHasShield = 0;
    m_bSliding = 0;
    m_wAttackerID = 0;
    m_dwEarthQuakeTime = 0;
    m_dwBreathStartTime = 0;
    m_dwBreathLifeTime = 0;
    m_dwLiquidTime = 0;
    m_dwPunishedTime = 0;
    m_cDamageRate = 1;
    m_dwEdgeColor = 0xBBFFFFFF;

    memset(&m_usTargetID, 0, sizeof(m_usTargetID));
    memset(&m_TradeDesc, 0, sizeof(m_TradeDesc));

    m_fCurrAng = 0.0f;
    m_dwForcedRotMaxTime = 0;
    m_dwForcedRotCurTime = 0;
    m_cMotionLoopCnt = 0;
    m_bForcedRotation = 0;
    m_bCritical = 0;
    m_pCriticalArmor = nullptr;

    for (int i = 0; i < 2; ++i)
        m_pSoul[i] = nullptr;

    for (int i = 0; i < 7; ++i)
    {
        m_pEyeFire[i] = nullptr;
        m_pEyeFire2[i] = nullptr;
    }
    for (int i = 0; i < 7; ++i)
        m_pRotateBone[i] = nullptr;

    for (int i = 0; i < 4; ++i)
        m_pFly[i] = nullptr;

    for (int i = 0; i < 5; ++i)
        m_pImmunity[i] = nullptr;

    m_pLightenStorm[0] = nullptr;
    m_pLightenStorm[1] = nullptr;

    Init();

    m_pNameLabel = nullptr;
    m_pKillLabel = nullptr;
    m_pAutoTradeDesc = nullptr;
    m_pAutoTradePanel = nullptr;
    m_pChatMsg = nullptr;
    m_stGuildMark.pGuildMark = 0;
    m_pProgressBar = nullptr;
    m_pMountHPBar = nullptr;
    m_pNickNameLabel = nullptr;
    m_stGuildMark.pGuildMark = 0;
    m_pTitleProgressBar = nullptr;
    m_pTitleNameLabel = nullptr;

    if (g_pCurrentScene != nullptr)
    {
        m_pNameLabel = new SText(-1, "NoName", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000, 1, 0);
        m_pAutoTradeDesc = new SText(-1, "", 0xFFFFFFFF, 0.0, 650.0f, 143.0f, 50.0f, 0, 0xFFFFFFFF, 1, 0);
        m_pAutoTradePanel = new SPanel(512, -10.0f, 635.0f, 141.0f, 43.0f, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
        m_pAutoTradePanel->m_bSelectEnable = 0;
        m_pChatMsg = new SText(-2, "", 0xFFFFFFFF, 0.0f, 650.0f, 256.0f, 64.0f, 1, 0x77000000, 1, 0);
        m_pChatMsg->m_Font.m_bMultiLine = 1;
        m_stGuildMark.pGuildMark = new SPanel(-2, 0.0f, 0.0f, 12.0f, 16.0f, 0x77777777u, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
        m_pProgressBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 72.0f, 7.0f, 0xFF9EF048, 0xFF333333, 1u);
        m_pMountHPBar = new SProgressBar(-2, 30, 30, 0.0f, 0.0f, 72.0f, 7.0f, 0xFFFFAA00, 0xFF333333, 1);

        m_pChatMsg->SetVisible(0);
        m_pNameLabel->m_GCBorder.nTextureSetIndex = -12;
        m_pNameLabel->SetVisible(0);
        
        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pAutoTradeDesc->SetVisible(0);

        m_pNickNameLabel = new SText(-1, "", 0xFFFFFFAA, 0.0f, 650.0f, 128.0f, 16.0f, 0, 0x55AA0000, 1, 0);
        m_pNickNameLabel->m_GCBorder.nTextureSetIndex = -2;

        m_pNickNameLabel->SetVisible(0);
        m_pAutoTradePanel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        m_pMountHPBar->SetVisible(0);

        float nYp = 29.0f * RenderDevice::m_fHeightRatio;
        m_pTitleProgressBar = new SProgressBar(-1, 30, 0, 0.0f, nYp, 200.0f, 18.0f, 0xFF800000, 0x40333333u, 1);
        m_pTitleProgressBar->m_nPosX = (float)g_pDevice->m_dwScreenWidth * 0.55000001f;
        m_pTitleProgressBar->SetVisible(0);

        g_pCurrentScene->m_pControlContainer->AddItem(m_pTitleProgressBar);

        m_pTitleNameLabel = new SText(-1, "NoName", 0xFFFFFFAA, 0.0f, 0.0f, 200.0f, 16.0f, 0, 0x55AA0000, 1, 1);
        m_pTitleNameLabel->SetVisible(0);
        m_pTitleProgressBar->AddChild(m_pTitleNameLabel);

        m_pKillLabel = new SText(-1, "", 0xFFFFFFAA, 140.0f, -12.0f, 128.0f, 16.0f, 0, 0x55AA0000, 1u, 0);
        m_pKillLabel->SetVisible(0);
        m_pTitleProgressBar->AddChild(m_pKillLabel);

        g_pCurrentScene->m_pControlContainer->AddItem(m_pNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_stGuildMark.pGuildMark);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pAutoTradeDesc);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pAutoTradePanel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pNickNameLabel);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pChatMsg);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pProgressBar);
        g_pCurrentScene->m_pControlContainer->AddItem(m_pMountHPBar);
    }

    m_dwAttackEffectTime = 0;
    m_dwDelayDel = 0;
    m_BigHp = 0;
    m_MaxBigHp = 0;
    m_usHP = 0;
    m_nDoubleCount = 0;
    m_bPunchEffect = 0;
    m_dwPunchEffectTime = 0;
    m_nAttackDestID = 0;
    m_cAvatar = 0;
    m_dwAvatarEffTime = 0;
    m_bStartAvatarEffect = 0;
    m_sMantuaIndex = 0;
    m_c8thSkill = 0;
    m_stGuildMark.nSubGuild = 0;
    m_stGuildMark.nGuildChannel = 0;
    m_stGuildMark.nGuild = -1;

    memset(&m_stGuildMark.strMarkFileName, 0, sizeof(m_stGuildMark.strMarkFileName));
    m_stGuildMark.nMarkIndex = 0;
    m_stGuildMark.bHideGuildmark = 0;
    m_stGuildMark.bLoadedGuildmark = 0;
    m_bUsed = 0;
    m_bIgnoreHeight = 0;
    m_vecAirMove.x = 0.0f;
    m_vecAirMove.y = 0.0f;
}

TMHuman::~TMHuman()
{
    TMScene* pScene = m_pParentScene;
    --g_nMobCount;
    if (m_sDelayDel == 0)
        ++g_nUnDelMobCount;

    SAFE_DELETE(m_pChatMsg);
    SAFE_DELETE(m_pNameLabel);
    SAFE_DELETE(m_stGuildMark.pGuildMark);
    SAFE_DELETE(m_pAutoTradeDesc);
    SAFE_DELETE(m_pAutoTradePanel);
    SAFE_DELETE(m_pNickNameLabel);
    SAFE_DELETE(m_pProgressBar);
    SAFE_DELETE(m_pMountHPBar);
    SAFE_DELETE(m_pInMiniMap);
    SAFE_DELETE(m_pTitleProgressBar);
    SAFE_DELETE(m_pSkinMesh);
    SAFE_DELETE(m_pMantua);
    SAFE_DELETE(m_pMount);
}

/// Inicializa malha esqueletica (skin mesh), animacoes, barra de HP e objetos visuais do personagem/NPC.
int TMHuman::InitObject()
{
    UpdateScore(0);

    if (m_pSkinMesh != nullptr)
    {
        m_pSkinMesh->m_pOwner = nullptr;

        if (m_pSkinMesh->m_pSwingEffect[0])
        {
            m_pSkinMesh->m_pSwingEffect[0]->m_pParentSkin = nullptr;
            if (m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant)
            {
                g_pObjectManager->DeleteObject(m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant);
                m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant = nullptr;
            }
        }
        if (m_pSkinMesh->m_pSwingEffect[1])
        {
            m_pSkinMesh->m_pSwingEffect[1]->m_pParentSkin = nullptr;
            if (m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant)
            {
                g_pObjectManager->DeleteObject(m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant);
                m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant = nullptr;
            }
        }
    }

    int nCos = 0;
    if (m_sCostume >= 4150 && m_sCostume < 4200 || m_sCostume >= 6301 && m_sCostume <= 6400 && m_nClass != 29)
    {
        m_stLookInfo.CoatMesh = g_pItemList[m_sCostume].nIndexMesh;
        m_stLookInfo.PantsMesh = m_stLookInfo.CoatMesh;
        m_stLookInfo.GlovesMesh = m_stLookInfo.PantsMesh;
        m_stLookInfo.BootsMesh = m_stLookInfo.GlovesMesh;
        m_stLookInfo.CoatSkin = g_pItemList[m_sCostume].nIndexTexture;
        m_stLookInfo.PantsSkin = m_stLookInfo.CoatSkin;
        m_stLookInfo.GlovesSkin = m_stLookInfo.PantsSkin;
        m_stLookInfo.BootsSkin = m_stLookInfo.GlovesSkin;
        nCos = SetHumanCostume();
    }
    else if ((m_sHelmIndex == 3503 || m_sHelmIndex == 3504 || m_sHelmIndex == 3505 || m_sHelmIndex == 3506) && !nCos)
    {
        nCos = 100;
        m_stLookInfo.HelmMesh = 0;
        m_stLookInfo.HelmSkin = 0;
        m_stLookInfo.CoatSkin = 0;
        m_stLookInfo.PantsSkin = 0;
        m_stLookInfo.GlovesSkin = 0;
        m_stLookInfo.BootsSkin = 0;
        if (m_sHeadIndex == 6 || m_sHeadIndex == 7 || m_sHeadIndex == 8 || m_sHeadIndex == 9)
        {
            m_stLookInfo.CoatMesh = 95;
            m_stLookInfo.PantsMesh = 95;
            m_stLookInfo.GlovesMesh = 95;
            m_stLookInfo.BootsMesh = 95;
        }
        if (m_sHeadIndex == 16 || m_sHeadIndex == 17 || m_sHeadIndex == 18 || m_sHeadIndex == 19)
        {
            m_stLookInfo.CoatMesh = 97;
            m_stLookInfo.PantsMesh = 97;
            m_stLookInfo.GlovesMesh = 97;
            m_stLookInfo.BootsMesh = 97;
        }
        if (m_sHeadIndex == 26 || m_sHeadIndex == 27 || m_sHeadIndex == 28 || m_sHeadIndex == 29)
        {
            m_stLookInfo.CoatMesh = 76;
            m_stLookInfo.PantsMesh = 76;
            m_stLookInfo.GlovesMesh = 76;
            m_stLookInfo.BootsMesh = 76;
        }
        if (m_sHeadIndex == 36 || m_sHeadIndex == 37 || m_sHeadIndex == 38 || m_sHeadIndex == 39)
        {
            m_stLookInfo.CoatMesh = 78;
            m_stLookInfo.PantsMesh = 78;
            m_stLookInfo.GlovesMesh = 78;
            m_stLookInfo.BootsMesh = 78;
        }
        if (m_sHeadIndex == 22 || m_sHeadIndex == 23 || m_sHeadIndex == 24 || m_sHeadIndex == 25 || m_nClass == 63)
        {
            m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
            m_stLookInfo.CoatMesh = m_stLookInfo.HelmMesh;
            m_stLookInfo.PantsMesh = m_stLookInfo.CoatMesh;
            m_stLookInfo.GlovesMesh = m_stLookInfo.PantsMesh;
            m_stLookInfo.BootsMesh = m_stLookInfo.GlovesMesh;
            m_stLookInfo.HelmSkin = m_stLookInfo.FaceSkin;
            m_stLookInfo.CoatSkin = m_stLookInfo.HelmSkin;
            m_stLookInfo.PantsSkin = m_stLookInfo.CoatSkin;
            m_stLookInfo.GlovesSkin = m_stLookInfo.PantsSkin;
            m_stLookInfo.BootsSkin = m_stLookInfo.GlovesSkin;
            m_stSancInfo.Sanc1 = m_stSancInfo.Sanc0;
            m_stSancInfo.Sanc2 = m_stSancInfo.Sanc1;
            m_stSancInfo.Sanc3 = m_stSancInfo.Sanc2;
            m_stSancInfo.Sanc4 = m_stSancInfo.Sanc3;
            m_stSancInfo.Sanc5 = m_stSancInfo.Sanc4;
            m_stSancInfo.Legend1 = m_stSancInfo.Legend0;
            m_stSancInfo.Legend2 = m_stSancInfo.Legend1;
            m_stSancInfo.Legend3 = m_stSancInfo.Legend2;
            m_stSancInfo.Legend4 = m_stSancInfo.Legend3;
            m_stSancInfo.Legend5 = m_stSancInfo.Legend4;
            m_stColorInfo.Sanc1 = m_stColorInfo.Sanc0;
            m_stColorInfo.Sanc2 = m_stColorInfo.Sanc1;
            m_stColorInfo.Sanc3 = m_stColorInfo.Sanc2;
            m_stColorInfo.Sanc4 = m_stColorInfo.Sanc3;
            m_stColorInfo.Sanc5 = m_stColorInfo.Sanc4;
        }
        else
            memset(&m_stSancInfo, 0, sizeof(m_stSancInfo));
    }

    SAFE_DELETE(m_pSkinMesh);

    if (!nCos && 
              (m_nSkinMeshType == 20
            || m_nSkinMeshType == 39
            || m_nSkinMeshType == 21
            || m_nSkinMeshType == 22
            || m_nSkinMeshType == 23
            || m_nSkinMeshType == 24
            || m_nSkinMeshType == 40
            || m_nSkinMeshType == 3
            || m_nSkinMeshType == 4
            || m_nSkinMeshType == 25
            || m_nSkinMeshType == 28
            || m_nSkinMeshType == 29
            || m_nSkinMeshType == 2
            || m_nSkinMeshType == 6
            || m_nSkinMeshType == 7
            || m_nSkinMeshType == 8
            || m_nSkinMeshType == 30
            || m_nSkinMeshType == 31
            || m_nSkinMeshType == 33
            || m_nSkinMeshType == 36
            || m_nSkinMeshType == 12
            || m_nSkinMeshType == 43
            || m_nSkinMeshType == 10
            || m_nSkinMeshType == 5
            || m_nSkinMeshType == 45
            || m_nSkinMeshType == 46
            || m_nSkinMeshType == 47
            || m_nSkinMeshType == 53
            || m_nSkinMeshType == 54
            || m_nSkinMeshType == 55
            || m_nSkinMeshType == 56
            || m_nSkinMeshType == 57))
    {
        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
        m_stLookInfo.HelmSkin = m_stLookInfo.FaceSkin;
        m_stSancInfo.Sanc1 = m_stSancInfo.Sanc0;
        m_stSancInfo.Legend1 = m_stSancInfo.Legend0;
        m_stColorInfo.Sanc1 = m_stColorInfo.Sanc0;
        if (m_nSkinMeshType == 45)
        {
            m_stSancInfo.Legend7 = 0;
            m_stSancInfo.Legend6 = 0;
            m_stSancInfo.Legend5 = 0;
            m_stSancInfo.Legend4 = 0;
            m_stSancInfo.Legend3 = 0;
            m_stSancInfo.Legend2 = 0;
            m_stSancInfo.Legend1 = 0;
            m_stSancInfo.Legend0 = 0;
            m_stSancInfo.Sanc7 = 0;
            m_stSancInfo.Sanc6 = 0;
            m_stSancInfo.Sanc5 = 0;
            m_stSancInfo.Sanc4 = 0;
            m_stSancInfo.Sanc3 = 0;
            m_stSancInfo.Sanc2 = 0;
            m_stSancInfo.Sanc1 = 0;
            m_stSancInfo.Sanc0 = 0;
            m_stSancInfo.Legend2 = 2;
            m_stSancInfo.Sanc2 = 8;
        }
        if (m_nSkinMeshType == 46)
        {
            m_stSancInfo.Legend7 = 0;
            m_stSancInfo.Legend6 = 0;
            m_stSancInfo.Legend5 = 0;
            m_stSancInfo.Legend4 = 0;
            m_stSancInfo.Legend3 = 0;
            m_stSancInfo.Legend2 = 0;
            m_stSancInfo.Legend1 = 0;
            m_stSancInfo.Legend0 = 0;
            m_stSancInfo.Sanc7 = 0;
            m_stSancInfo.Sanc6 = 0;
            m_stSancInfo.Sanc5 = 0;
            m_stSancInfo.Sanc4 = 0;
            m_stSancInfo.Sanc3 = 0;
            m_stSancInfo.Sanc2 = 0;
            m_stSancInfo.Sanc1 = 0;
            m_stSancInfo.Sanc0 = 0;
            m_stSancInfo.Legend2 = 2;
            m_stSancInfo.Sanc2 = 13;
        }
        if (m_nSkinMeshType == 53)
        {
            m_stSancInfo.Legend7 = 0;
            m_stSancInfo.Legend6 = 0;
            m_stSancInfo.Legend5 = 0;
            m_stSancInfo.Legend4 = 0;
            m_stSancInfo.Legend3 = 0;
            m_stSancInfo.Legend2 = 0;
            m_stSancInfo.Legend1 = 0;
            m_stSancInfo.Legend0 = 0;
            m_stSancInfo.Sanc7 = 0;
            m_stSancInfo.Sanc6 = 0;
            m_stSancInfo.Sanc5 = 0;
            m_stSancInfo.Sanc4 = 0;
            m_stSancInfo.Sanc3 = 0;
            m_stSancInfo.Sanc2 = 0;
            m_stSancInfo.Sanc1 = 0;
            m_stSancInfo.Sanc0 = 0;
            m_stSancInfo.Sanc0 = 13;
            m_stSancInfo.Sanc2 = 13;
            m_stSancInfo.Sanc1 = 13;
            m_fScale = 2.0f;
        }
        if (m_nSkinMeshType == 56 || m_nSkinMeshType == 57)
        {
            m_stSancInfo.Legend7 = 0;
            m_stSancInfo.Legend6 = 0;
            m_stSancInfo.Legend5 = 0;
            m_stSancInfo.Legend4 = 0;
            m_stSancInfo.Legend3 = 0;
            m_stSancInfo.Legend2 = 0;
            m_stSancInfo.Legend1 = 0;
            m_stSancInfo.Legend0 = 0;
            m_stSancInfo.Sanc7 = 0;
            m_stSancInfo.Sanc6 = 0;
            m_stSancInfo.Sanc5 = 0;
            m_stSancInfo.Sanc4 = 0;
            m_stSancInfo.Sanc3 = 0;
            m_stSancInfo.Sanc2 = 0;
            m_stSancInfo.Sanc1 = 0;
            m_stSancInfo.Sanc0 = 0;
            m_stSancInfo.Sanc4 = 13;
            m_stSancInfo.Sanc3 = 13;
            m_stSancInfo.Sanc2 = 13;
            m_stSancInfo.Sanc1 = 13;
            m_stSancInfo.Sanc0 = 13;
        }
        if (m_nSkinMeshType == 55)
        {
            m_stSancInfo.Legend7 = 0;
            m_stSancInfo.Legend6 = 0;
            m_stSancInfo.Legend5 = 0;
            m_stSancInfo.Legend4 = 0;
            m_stSancInfo.Legend3 = 0;
            m_stSancInfo.Legend2 = 0;
            m_stSancInfo.Legend1 = 0;
            m_stSancInfo.Legend0 = 0;
            m_stSancInfo.Sanc7 = 0;
            m_stSancInfo.Sanc6 = 0;
            m_stSancInfo.Sanc5 = 0;
            m_stSancInfo.Sanc4 = 0;
            m_stSancInfo.Sanc3 = 0;
            m_stSancInfo.Sanc2 = 0;
            m_stSancInfo.Sanc1 = 0;
            m_stSancInfo.Sanc0 = 0;
            m_stSancInfo.Sanc1 = 13;
            m_stSancInfo.Sanc0 = 13;

            TMHuman::m_vecPickSize[55].y = 0.449f;
            TMHuman::m_vecPickSize[55].x = 0.449f;
        }
    }

    if (m_nClass == 40)
    {
        m_stLookInfo.LeftMesh = 0;
        m_stLookInfo.RightMesh = 0;
    }
    if (m_nSkinMeshType == 26 || m_nSkinMeshType == 35)
    {
        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
        m_stLookInfo.CoatMesh = m_stLookInfo.HelmMesh;
        m_stLookInfo.HelmSkin = m_stLookInfo.FaceSkin;
        m_stLookInfo.CoatSkin = m_stLookInfo.HelmSkin;
        m_stSancInfo.Sanc1 = m_stSancInfo.Sanc0;
        m_stSancInfo.Sanc2 = m_stSancInfo.Sanc1;
        m_stSancInfo.Legend1 = m_stSancInfo.Legend0;
        m_stSancInfo.Legend2 = m_stSancInfo.Legend1;
        m_stColorInfo.Sanc1 = m_stColorInfo.Sanc0;
        m_stColorInfo.Sanc2 = m_stColorInfo.Sanc1;
    }
    else if (m_nSkinMeshType == 11 || m_nSkinMeshType == 37 || m_nSkinMeshType == 44)
    {
        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
        m_stLookInfo.CoatMesh = m_stLookInfo.HelmMesh;
        m_stLookInfo.PantsMesh = m_stLookInfo.CoatMesh;
        m_stLookInfo.GlovesMesh = m_stLookInfo.PantsMesh;
        m_stLookInfo.HelmSkin = m_stLookInfo.FaceSkin;
        m_stLookInfo.CoatSkin = m_stLookInfo.HelmSkin;
        m_stLookInfo.PantsSkin = m_stLookInfo.CoatSkin;
        m_stLookInfo.GlovesSkin = m_stLookInfo.PantsSkin;
        m_stSancInfo.Sanc1 = m_stSancInfo.Sanc0;
        m_stSancInfo.Sanc2 = m_stSancInfo.Sanc1;
        m_stSancInfo.Sanc3 = m_stSancInfo.Sanc2;
        m_stSancInfo.Sanc4 = m_stSancInfo.Sanc3;
        m_stSancInfo.Legend1 = m_stSancInfo.Legend0;
        m_stSancInfo.Legend2 = m_stSancInfo.Legend1;
        m_stSancInfo.Legend3 = m_stSancInfo.Legend2;
        m_stSancInfo.Legend4 = m_stSancInfo.Legend3;
        m_stColorInfo.Sanc1 = m_stColorInfo.Sanc0;
        m_stColorInfo.Sanc2 = m_stColorInfo.Sanc1;
        m_stColorInfo.Sanc3 = m_stColorInfo.Sanc2;
        m_stColorInfo.Sanc4 = m_stColorInfo.Sanc3;
    }
    if (m_nClass == 39 || m_nClass == 40 || m_nClass == 63)
    {
        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
        m_stLookInfo.CoatMesh = m_stLookInfo.HelmMesh;
        m_stLookInfo.PantsMesh = m_stLookInfo.CoatMesh;
        m_stLookInfo.GlovesMesh = m_stLookInfo.PantsMesh;
        m_stLookInfo.BootsMesh = m_stLookInfo.GlovesMesh;
        m_stLookInfo.HelmSkin = m_stLookInfo.FaceSkin;
        m_stLookInfo.CoatSkin = m_stLookInfo.HelmSkin;
        m_stLookInfo.PantsSkin = m_stLookInfo.CoatSkin;
        m_stLookInfo.GlovesSkin = m_stLookInfo.PantsSkin;
        m_stLookInfo.BootsSkin = m_stLookInfo.GlovesSkin;
        m_stSancInfo.Sanc1 = m_stSancInfo.Sanc0;
        m_stSancInfo.Sanc2 = m_stSancInfo.Sanc1;
        m_stSancInfo.Sanc3 = m_stSancInfo.Sanc2;
        m_stSancInfo.Sanc4 = m_stSancInfo.Sanc3;
        m_stSancInfo.Sanc5 = m_stSancInfo.Sanc4;
        m_stSancInfo.Legend1 = m_stSancInfo.Legend0;
        m_stSancInfo.Legend2 = m_stSancInfo.Legend1;
        m_stSancInfo.Legend3 = m_stSancInfo.Legend2;
        m_stSancInfo.Legend4 = m_stSancInfo.Legend3;
        m_stSancInfo.Legend5 = m_stSancInfo.Legend4;
        m_stColorInfo.Sanc1 = m_stColorInfo.Sanc0;
        m_stColorInfo.Sanc2 = m_stColorInfo.Sanc1;
        m_stColorInfo.Sanc3 = m_stColorInfo.Sanc2;
        m_stColorInfo.Sanc4 = m_stColorInfo.Sanc3;
        m_stColorInfo.Sanc5 = m_stColorInfo.Sanc4;
    }
    else if (m_nClass == 62 && m_stLookInfo.FaceMesh == 2)
    {
        m_stSancInfo.Sanc0 = 4;
        m_stSancInfo.Sanc1 = 4;
    }

    int bExpand = 0;
    if (m_nClass == 4
        || m_nClass == 8
        || m_nClass == 36
        || m_nClass == 39
        || m_nClass == 40
        || m_nClass == 60
        || m_nClass == 63)
    {
        bExpand = 1;
    }

    if (m_stLookInfo.FaceMesh == 40)
    {
        if (!m_stLookInfo.CoatMesh)
            m_stLookInfo.CoatMesh = 40;
        if (!m_stLookInfo.PantsMesh)
            m_stLookInfo.PantsMesh = 40;
        if (!m_stLookInfo.GlovesMesh)
            m_stLookInfo.GlovesMesh = 40;
        if (!m_stLookInfo.BootsMesh)
            m_stLookInfo.BootsMesh = 40;
        m_stLookInfo.HelmMesh = 40;
    }
    else if (m_stLookInfo.FaceMesh == 80 || m_stLookInfo.FaceMesh == 64)
    {
        if (!m_stLookInfo.CoatMesh)
            m_stLookInfo.CoatMesh = 40;
        if (!m_stLookInfo.PantsMesh)
            m_stLookInfo.PantsMesh = 40;
        if (!m_stLookInfo.GlovesMesh)
            m_stLookInfo.GlovesMesh = 40;
        if (!m_stLookInfo.BootsMesh)
            m_stLookInfo.BootsMesh = 40;
        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
    }

    if (m_nClass != 1
        && m_nClass != 2
        && m_nClass != 4
        && m_nClass != 8
        && nCos
        && nCos != 100)
    {
        bExpand = 1;
        m_nSkinMeshType = 0;
    }

    if (m_cShadow == 1)
    {
        memset(&m_stSancInfo, 0, sizeof(m_stSancInfo));
        memset(&m_stMountSanc, 0, sizeof(m_stMountSanc));
    }

    if (m_nClass == 26 || m_nClass == 33 || m_nClass == 40 || m_nClass == 63)
        m_pSkinMesh = new TMSkinMesh((LOOK_INFO*)&m_stLookInfo, &m_stSancInfo, m_nSkinMeshType, bExpand, &m_stColorInfo, 1, nCos, 1);
    else
        m_pSkinMesh = new TMSkinMesh((LOOK_INFO*)&m_stLookInfo, &m_stSancInfo, m_nSkinMeshType, bExpand, &m_stColorInfo, 1, nCos, 0);

    if (m_pSkinMesh != nullptr)
    {
        m_pSkinMesh->m_pOwner = this;
        m_pSkinMesh->RestoreDeviceObjects();

        m_pSkinMesh->m_dwFPS = 40;

        if (m_nSkinMeshType == 31 && m_fScale < 1.0)
        {
            float fGrade = 2.0f - m_fScale;
            m_pSkinMesh->m_vScale.x = m_fScale;
            m_pSkinMesh->m_vScale.y = m_fScale / fGrade;
            m_pSkinMesh->m_vScale.z = m_fScale;
        }
        else
        {
            m_pSkinMesh->m_vScale.x = m_fScale;
            m_pSkinMesh->m_vScale.y = m_fScale;
            m_pSkinMesh->m_vScale.z = m_fScale;
        }      
    }

    SetHandEffect(m_nHandEffect);
    SAFE_DELETE(m_pMantua);

    if (m_cMantua > 0
        && (!m_nSkinMeshType
            || m_nSkinMeshType == 1
            || m_nSkinMeshType == 8
            || m_nSkinMeshType == 3
            || m_nSkinMeshType == 2))
    {
        LOOK_INFO stLook{};
        SANC_INFO stSanc{};

        if (!nCos)
        {
            stLook.Skin0 = m_wMantuaSkin;
            stSanc.Sanc0 = m_ucMantuaSanc;
            stSanc.Legend0 = m_ucMantuaLegend;

            m_pMantua = new TMSkinMesh(&stLook, &stSanc, 85, 0, 0, 1, 0, 0);
        }
        else if (nCos == 100)
        {
            if (m_sMantuaIndex == 545 || m_sMantuaIndex == 3197 || m_sMantuaIndex >= 1766 && m_sMantuaIndex <= 1768)
                stLook.Skin0 = 0;
            if (m_sMantuaIndex == 546 || m_sMantuaIndex == 3198 || m_sMantuaIndex == 1770 || m_sMantuaIndex == 1769)
                stLook.Skin0 = 1;
            if (m_sMantuaIndex == 548 || m_sMantuaIndex == 3199 || m_sMantuaIndex >= 572 && m_sMantuaIndex <= 574)
                stLook.Skin0 = 2;

            stSanc.Sanc0 = static_cast<char>(m_citizen);
            stSanc.Legend0 = m_ucMantuaLegend;

            m_pMantua = new TMSkinMesh(&stLook, &stSanc, 85, 0, 0, 1, nCos, 0);
        }

        if (m_pMantua != nullptr)
        {
            m_pMantua->m_pOwner = 0;
            m_pMantua->RestoreDeviceObjects();
            m_pMantua->m_dwFPS = 40;
            m_pMantua->m_vScale.x = m_fScale;
            m_pMantua->m_vScale.y = m_fScale;
            m_pMantua->m_vScale.z = m_fScale;
            m_pMantua->SetVecMantua(1, m_nMountSkinMeshType);
        }
    }

    UpdateMount();
    if (m_stScore.Hp <= 0)
    {
        m_nWillDie = 4;
        SetAnimation(ECHAR_MOTION::ECMOTION_DEAD, 1);
    }
    else
        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);

    for (int i = 0; i < 7; ++i)
    {
        if (m_pEyeFire[i])
        {
            g_pObjectManager->DeleteObject(m_pEyeFire[i]);
            m_pEyeFire[i] = nullptr;
        }
        if (m_pEyeFire2[i])
        {
            g_pObjectManager->DeleteObject(m_pEyeFire2[i]);
            m_pEyeFire2[i] = nullptr;
        }
    }
    for (int i = 0; i < 7; ++i)
    {
        if (m_pRotateBone[i])
        {
            g_pObjectManager->DeleteObject(m_pRotateBone[i]);
            m_pRotateBone[i] = nullptr;
        }
    }

    if (m_nClass == 36 || m_nClass == 37)
    {
        int nSkelType = 0;
        if (m_nClass == 37)
            nSkelType = 3;
        if (m_stLookInfo.HelmMesh == 11)
            nSkelType = 1;
        if (m_stLookInfo.HelmMesh == 10)
            nSkelType = 2;

        unsigned int dwColor[4]{};
        dwColor[0] = 0xFF005555;
        dwColor[1] = 0xFF885500;
        dwColor[2] = 0xFF550000;
        dwColor[3] = 0xFF005500;

        if (nSkelType == 2 && m_stLookInfo.LeftMesh != 930)
        {
            int nMeshIndex[7]{};
            nMeshIndex[0] = 3;
            nMeshIndex[1] = 6;
            nMeshIndex[2] = 4;
            nMeshIndex[3] = 7;
            nMeshIndex[4] = 5;
            nMeshIndex[5] = 6;
            nMeshIndex[6] = 7;

            for (int j = 0; j < 7; ++j)
            {
                m_pRotateBone[j] = nullptr;
                m_pRotateBone[j] = new TMEffectMeshRotate(TMVector3(m_vecPosition.x, (float)(m_fHeight + 0.80000001f) + (float)((float)j * 0.2f), m_vecPosition.y), 0, this, 1, 0);

                if (m_pRotateBone[j])
                {
                    m_pRotateBone[j]->m_dwStartTime = 150 * j;
                    m_pRotateBone[j]->m_nMeshIndex = nMeshIndex[j];
                    g_pCurrentScene->m_pEffectContainer->AddChild(m_pRotateBone[j]);
                }
            }
        }

        int nPosIndex[7]{};
        nPosIndex[0] = 8;
        nPosIndex[1] = 9;
        nPosIndex[2] = 1;
        nPosIndex[3] = 6;
        nPosIndex[4] = 7;
        nPosIndex[5] = 2;
        nPosIndex[6] = 3;

        float fSizeX[4][7] = {
            { 0.2f, 0.8f, 0.2f, 0.6f, 0.8f, 0.6f, 0.8f },
            { 0.2f, 0.8f, 0.2f, 0.6f, 0.8f, 0.6f, 0.8f },
            { 0.2f, 0.8f, 0.2f, 0.6f, 0.8f, 0.6f, 0.8f },
            { 0.2f, 0.8f, 0.2f, 0.6f, 0.8f, 0.6f, 0.8f },
        };

        float fSizeY[4][7] = {
            { 0.3f, 1.0f, 0.3f, 0.8f, 1.0f, 0.8f, 1.0f },
            { 0.3f, 1.0f, 0.3f, 0.8f, 1.0f, 0.8f, 1.0f },
            { 0.3f, 1.0f, 0.3f, 0.8f, 1.0f, 0.8f, 1.0f },
            { 0.3f, 1.0f, 0.3f, 0.8f, 1.0f, 0.8f, 1.0f },
        };

        for (int k = 0; k < 7; ++k)
        {
            if (k != 2 || nSkelType != 1 && nSkelType != 2)
            {
                if (k >= 5 && nSkelType != 2)
                    break;

                m_pEyeFire[k] = new TMEffectBillBoard(101,
                    0,
                    fSizeX[nSkelType][k] * m_fScale,
                    fSizeY[nSkelType][k] * m_fScale,
                    fSizeX[nSkelType][k] * m_fScale,
                    0.0,
                    8,
                    80);

                if (m_pEyeFire[k])
                {
                    m_pEyeFire[k]->SetColor(dwColor[nSkelType]);
                    m_pEyeFire[k]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    m_pEyeFire[k]->m_nFade = 0;
                    m_pEyeFire[k]->m_vecPosition = m_vecTempPos[nPosIndex[k]];
                    m_pEyeFire[k]->m_bFrameMove = 0;
                    g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[k]);
                }
            }
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (m_pFly[i])
        {
            g_pObjectManager->DeleteObject(m_pFly[i]);
            m_pFly[i] = nullptr;
        }
    }
    if (m_nClass == 21 && m_stLookInfo.FaceMesh == 10
     || m_nClass == 28 && m_stLookInfo.FaceMesh == 2
     || m_nClass == 25 && m_stLookInfo.FaceMesh == 3 && m_stLookInfo.FaceSkin == 8
     || m_nClass == 25 && m_stLookInfo.FaceMesh == 12)
    {
        for (int l = 0; l < 4; ++l)
        {
            if (m_pFly[l])
            {
                g_pObjectManager->DeleteObject(m_pFly[l]);
                m_pFly[l] = nullptr;
            }

            m_pFly[l] = new TMButterFly(6, 3, this);
            if (m_pFly[l])
            {
                m_pFly[l]->InitObject();
                m_pFly[l]->InitAngle(0.0f,
                    (float)((float)l * 3.1415927f) / 6.0f,
                    0.0f);
                m_pFly[l]->InitPosition(
                    (float)((float)l * 0.2f) + m_vecPosition.x,
                    (float)(m_fHeight + 1.5f) + (float)((float)l * 0.2f),
                    (float)((float)l * 0.2f) + m_vecPosition.y);

                m_pFly[l]->m_fParticleH = m_pFly[l]->m_fParticleH * 0.5f;
                m_pFly[l]->m_fParticleV = m_pFly[l]->m_fParticleV * 0.5f;
                m_pFly[l]->m_fCircleSpeed = (float)l + 8.0f;

                g_pCurrentScene->m_pEffectContainer->AddChild(m_pFly[l]);
            }
        }
    }

    if (m_nClass == 34 || m_nClass == 23 || m_nClass == 21 && m_stLookInfo.FaceMesh == 10)
    {
        m_cDodge = 1;
        int nCount = 1;
        int nTexIndex = 71;
        int dwColor = 0x0FFFFAAFF;
        if (m_nClass == 23)
        {
            nCount = 2;
            m_cDodge = 0;
            dwColor = 0x0FF33FF66;
        }
        else if (m_nClass == 21)
        {
            nTexIndex = 60;
            dwColor = 0x0FFEE8800;
        }

        for (int m = 0; m < nCount; ++m)
        {
            if (m_pEyeFire[m])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[m]);
                m_pEyeFire[m] = nullptr;
            }

            m_pEyeFire[m] = new TMEffectBillBoard(nTexIndex, 0, 1.0f, 1.0f, 1.0f, 0.0f, 1, 80);

            if (m_pEyeFire[m])
            {
                m_pEyeFire[m]->SetColor(dwColor);
                m_pEyeFire[m]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[m]->m_nFade = 0;
                m_pEyeFire[m]->m_vecPosition = m_vecTempPos[0];
                m_pEyeFire[m]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[m]);
            }
        }
    }
    if (m_nClass == 29 && m_stLookInfo.FaceMesh == 1)
    {
        for (int n = 0; n < 1; ++n)
        {
            if (m_pEyeFire[n])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[n]);
                m_pEyeFire[n] = 0;
            }

            m_pEyeFire[n] = new TMEffectBillBoard(71, 0, 1.0f, 1.0f, 1.0f, 0.0f, 1, 80);

            if (m_pEyeFire[n])
            {
                m_pEyeFire[n]->SetColor(0x0FFFF00FF);
                m_pEyeFire[n]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[n]->m_nFade = 0;
                m_pEyeFire[n]->m_vecPosition = m_vecTempPos[0];
                m_pEyeFire[n]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[n]);
            }
        }
    }
    if (m_nClass == 32 && m_stLookInfo.FaceMesh == 2)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_pEyeFire[i])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[i]);
                m_pEyeFire[i] = 0;
            }

            m_pEyeFire[i] = new TMEffectBillBoard(11,
                0,
                1.2f * m_fScale,
                1.8f * m_fScale,
                1.2f * m_fScale,
                0.0,
                8,
                80);

            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(0x0FFAA8800);
                m_pEyeFire[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[i]->m_nFade = 0;
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[i]);
            }
        }
    }
    if (m_nClass == 16 && !m_stLookInfo.FaceMesh && m_stLookInfo.FaceSkin == 1 && m_cLegend != 4)
    {
        float fSize = 0.4f;
        for (int i = 8; i < 10; ++i)
        {
            if (m_pEyeFire[i])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[i]);
                m_pEyeFire[i] = 0;
            }

            m_pEyeFire[i] = new TMEffectBillBoard(
                101,
                0,
                fSize * m_fScale,
                fSize * m_fScale,
                fSize * m_fScale,
                0.0,
                8,
                80);

            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(0x0FF88FFAA);
                m_pEyeFire[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[i]->m_nFade = 0;
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[i]);
            }
        }
    }
    if (m_nClass == 39)
    {
        int nTextureIndex[4]{};
        nTextureIndex[0] = 56;
        nTextureIndex[1] = 56;
        nTextureIndex[2] = 101;
        nTextureIndex[3] = 101;

        int nTexCount[4]{};
        nTexCount[0] = 1;
        nTexCount[1] = 1;
        nTexCount[2] = 8;
        nTexCount[3] = 8;

        for (int i = 0; i < 4; ++i)
        {
            if (m_pEyeFire[i])
            {
                g_pObjectManager->DeleteObject(m_pEyeFire[i]);
                m_pEyeFire[i] = 0;
            }

            m_pEyeFire[i] = new TMEffectBillBoard(
                nTextureIndex[i],
                0,
                1.0f * m_fScale,
                1.0f * m_fScale,
                1.0f * m_fScale,
                0.0,
                nTexCount[i],
                80);

            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(0x0FFFF5500);
                m_pEyeFire[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[i]->m_nFade = 0;
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[0];
                m_pEyeFire[i]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[i]);
            }
        }
    }
    if (m_nClass == 38 && m_cMantua > 0 && !m_pEyeFire[1])
    {
        int nPosIndex[7]{};
        nPosIndex[0] = 8;
        nPosIndex[1] = 9;
        nPosIndex[2] = 1;
        nPosIndex[3] = 6;
        nPosIndex[4] = 7;
        nPosIndex[5] = 2;
        nPosIndex[6] = 3;

        float fSizeX[7]{};
        fSizeX[0] = 0.2f;
        fSizeX[1] = 0.2f;
        fSizeX[2] = 0.8f;
        fSizeX[3] = 0.6f;
        fSizeX[4] = 0.6f;
        fSizeX[5] = 0.8f;
        fSizeX[6] = 0.8f;

        float fSizeY[7]{};
        fSizeY[0] = 0.3f;
        fSizeY[1] = 0.3f;
        fSizeY[2] = 1.0f;
        fSizeY[3] = 0.8f;
        fSizeY[4] = 0.8f;
        fSizeY[5] = 1.0f;
        fSizeY[6] = 1.0f;

        unsigned int dwColor = 0xFF005588;
        if (m_stLookInfo.LeftMesh == 930)
            dwColor = 0xFF008855;

        for (int i = 1; i < 7; ++i)
        {
            m_pEyeFire[i] = new TMEffectBillBoard(123,
                0,
                fSizeX[i] * m_fScale,
                fSizeY[i] * m_fScale,
                fSizeX[i] * m_fScale,
                0.0f,
                1,
                80);

            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(dwColor);
                m_pEyeFire[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[i]->m_nFade = 2;
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[nPosIndex[i]];
                m_pEyeFire[i]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[i]);
            }
        }
    }
    if (m_nClass == 33 && m_stLookInfo.FaceMesh == 1 && RenderDevice::m_bDungeon == 2 && !m_pEyeFire[0])
    {
        unsigned int dwColor = 0xFF005588;

        for (int i = 0; i < 7; ++i)
        {
            m_pEyeFire[i] = new TMEffectBillBoard(
                101,
                0,
                2.0f * m_fScale,
                3.0f * m_fScale,
                2.0f * m_fScale,
                0.0f,
                8,
                80);

            if (m_pEyeFire[i])
            {
                m_pEyeFire[i]->SetColor(0xFFFF5500);
                m_pEyeFire[i]->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                m_pEyeFire[i]->m_nFade = 2;
                m_pEyeFire[i]->m_vecPosition = m_vecTempPos[i];
                m_pEyeFire[i]->m_bFrameMove = 0;
                g_pCurrentScene->m_pEffectContainer->AddChild(m_pEyeFire[i]);
            }
        }
    }

    if (m_pShade)
    {
        g_pObjectManager->DeleteObject(m_pShade);
        m_pShade = 0;
    }

    if (!g_bHideEffect && !g_pDevice->m_bSavage && !g_pDevice->m_bIntel)
    {
        switch (m_nSkinMeshType)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            if (m_cMount <= 0)
                m_pShade = new TMShade(3, 4, m_fScale);
            else
                m_pShade = new TMShade(5, 4, m_fScale);
            break;
        case 6:
        case 8:
        case 9:
        case 10:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 21:
        case 22:
        case 23:
        case 25:
        case 26:
        case 27:
        case 28:
        case 30:
        case 31:
        case 33:
        case 40:
        case 41:
        case 43:
            m_pShade = new TMShade(3, 4, m_fScale);
            break;
        case 7:
        case 29:
            m_pShade = new TMShade(4, 4, m_fScale);
            break;
        case 11:
        case 35:
        case 42:
            m_pShade = new TMShade(6, 4, m_fScale);
            break;
        case 12:
        case 24:
        case 32:
            m_pShade = new TMShade(2, 4, m_fScale);
            break;
        case 20:
            if (!m_stLookInfo.HelmMesh)
                m_pShade = new TMShade(6, 4, m_fScale);
            break;
        case 34:
        case 36:
        case 44:
            m_pShade = new TMShade(8, 4, m_fScale);
            break;
        case 37:
            m_pShade = new TMShade(2, 203, m_fScale);
            break;
        case 38:
            m_pShade = new TMShade(5, 4, m_fScale);
            break;
        case 39:
            m_pShade = new TMShade(6, 4, m_fScale);
            break;
        }

        if (m_pShade)
        {
            m_pShade->SetPosition(TMVector2(0.0f, 0.0f));
            g_pCurrentScene->m_pShadeContainer->AddChild(m_pShade);
        }
    }

    GetLegType();

    if (m_pEleStream)
    {
        g_pObjectManager->DeleteObject(m_pEleStream);
        m_pEleStream = 0;
    }
    if (m_pEleStream2)
    {
        g_pObjectManager->DeleteObject(m_pEleStream2);
        m_pEleStream2 = 0;
    }
    if (m_pRescue)
    {
        g_pObjectManager->DeleteObject(m_pRescue);
        m_pRescue = 0;
    }
    if (m_pMagicShield)
    {
        g_pObjectManager->DeleteObject(m_pMagicShield);
        m_pMagicShield = 0;
    }
    if (m_pCancelation)
    {
        g_pObjectManager->DeleteObject(m_pCancelation);
        m_pCancelation = 0;
    }

    if (m_nClass <= 8)
    {
        m_pMagicShield = new TMSkillMagicShield(this, 0);
        m_pRescue = new TMSkillMagicShield(this, 1);
        m_pEleStream = new TMSkillMagicShield(this, 2);
        m_pEleStream2 = new TMSkillMagicShield(this, 3);
        m_pCancelation = new TMSkillMagicShield(this, 4);
    }

    if (m_pRescue)
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pRescue);
    if (m_pMagicShield)
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pMagicShield);
    if (m_pCancelation)
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pCancelation);
    if (m_pEleStream)
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pEleStream);
    if (m_pEleStream2)
        g_pCurrentScene->m_pEffectContainer->AddChild(m_pEleStream2);

    return 1;
}

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

/// Atualiza estado por quadro: interpolacao de movimento, animacao, combate e buffs.
int TMHuman::FrameMove(unsigned int dwServerTime)
{
    if (m_pAutoTradeDesc)
        m_pAutoTradeDesc->SetVisible(0);
    if (m_pAutoTradePanel)
        m_pAutoTradePanel->SetVisible(0);

    int a = 10;

    if (m_dwDelayDel && m_cDeleted != 1)
    {
        if (m_dwDelayDel + 10000 < g_pTimerManager->GetServerTime())
        {
            m_dwDelayDel = 0;
            g_pObjectManager->DeleteObject(this);
        }
        return 1;
    }

    if (m_cDeleted == 1)
        return 1;

    dwServerTime = g_pTimerManager->GetServerTime();

    if (dwServerTime > m_dwPunchEffectTime + 100)
        m_bPunchEffect = 0;
    if (m_bVisible == 1)
    {
        IsMouseOver();
        if (m_dwStartChatMsgTime)
        {
            if (dwServerTime - m_dwStartChatMsgTime < m_dwChatDelayTime)
            {
                if (!m_pChatMsg->m_bVisible)
                    m_pChatMsg->SetVisible(1);
                if (!m_pNameLabel->m_bVisible)
                    m_pNameLabel->SetVisible(1);
            }
        }
    }
    if (m_pInMiniMap)
    {
        TMObject* pFocusedObject = g_pObjectManager->m_pCamera->m_pFocusedObject;
        float temp = 86.0f;
        if (TMGround::m_fMiniMapScale < 1.0f)
            temp = 74.0f;

        if (pFocusedObject)
        {
            float fX = pFocusedObject->m_vecPosition.x - m_vecPosition.x;
            float fY = pFocusedObject->m_vecPosition.y - m_vecPosition.y;
            float fX2 = (((-(float)((0.70710701f * fX) + (-0.70710701f * fY)) * 2.0f) + temp)
                + TMGround::m_fMiniMapScale * 120.0f);
            float fY2 = (((((0.70710701f * fX) + (0.70710701f * fY)) * 2.0f) + temp)
                + TMGround::m_fMiniMapScale * 120.0f);

            m_pInMiniMap->SetPos(fX2, fY2);
        }
    }

    if (!m_bVisible)
    {
        m_pNameLabel->SetVisible(0);
        m_pProgressBar->SetVisible(0);
        if (m_stGuildMark.pGuildMark)
            m_stGuildMark.pGuildMark->SetVisible(0);

        m_pChatMsg->SetVisible(0);
        m_pNickNameLabel->SetVisible(0);
        if (!m_nWillDie)
        {
            DelayDelete();
            return 1;
        }
    }

    if (m_dwStartChatMsgTime && dwServerTime - m_dwStartChatMsgTime > m_dwChatDelayTime)
    {
        m_pChatMsg->SetVisible(0);
        m_dwStartChatMsgTime = 0;
        m_dwChatDelayTime = 3000;
    }

    TMScene* pScene = g_pCurrentScene;
    float fDieHeight = 0.0f;
    unsigned int dwWillDieTime = 10000;
    if (m_nClass == 56 && !m_stLookInfo.FaceMesh)
        dwWillDieTime = 600000;
    if (m_dwDeadTime && dwServerTime > dwWillDieTime + m_dwDeadTime && (m_nWillDie == 1 || m_nWillDie == 2))
    {
        m_dwDeadTime = 0;
        DelayDelete();
        return 1;
    }

    if (m_dwDeadTime && (m_nWillDie == 1 || m_nWillDie == 2) && m_cDie == 1)
    {
        float fRatio = 1.0f;
        if (m_nClass == 44)
            fRatio = 4.0f;
        if (m_nClass != 56 || m_stLookInfo.FaceMesh)
        {
            fDieHeight = (((((float)(m_dwDeadTime + 10000 - dwServerTime) / 10000.0f) - 1.0f) * m_fScale)
                * TMHuman::m_vecPickSize[m_nSkinMeshType].y)
                * fRatio;
        }
        else
            fDieHeight = 0.0f;

        if (m_nClass == 68)
            fDieHeight = 0.0;
    }

    if (m_stPunchEvent.dwTime && dwServerTime > m_stPunchEvent.dwTime + 200)
    {
        Punched(m_stPunchEvent.nDamage, m_stPunchEvent.vecFrom);
        memset(&m_stPunchEvent, 0, sizeof(m_stPunchEvent));
        m_stPunchEvent.dwTime = 0;
    }

    if (m_dwEarthQuakeTime && dwServerTime - m_dwEarthQuakeTime > 0x3E8)
    {
        m_dwEarthQuakeTime = 0;
        if (m_nClass == 56)
            g_pObjectManager->GetCamera()->EarthQuake(1);
        else if (m_nClass == 32)
        {
            g_pObjectManager->GetCamera()->EarthQuake(1);

            int nTexIndex = 0;
            float fWaterHeight = 0.0f;
            int nSoundIndex = 142;

            TMVector2 vec((float)(cosf(m_fAngle - 3.1415927f) * 1.5f) + m_vecPosition.x,
                m_vecPosition.y - (float)(sinf(m_fAngle - 3.1415927f) * 1.5f));

            if (pScene->GroundIsInWater(vec, m_fHeight, &fWaterHeight) == 1)
            {
                nTexIndex = 151;
                nSoundIndex = 10;
            }

            auto pSoundManager = g_pSoundManager;
            if (pSoundManager)
            {
                auto pSoundData = pSoundManager->GetSoundData(nSoundIndex);
                if (pSoundData && !pSoundData->IsSoundPlaying())
                {
                    pSoundData->Play();
                }
            }

            if (nTexIndex == 0)
            {
                TMShade* pCrater = new TMShade(2, 117, 1.0f);

                if (pCrater)
                {
                    pCrater->m_bFI = 0;
                    pCrater->m_dwLifeTime = 5000;
                    pCrater->SetColor(0xAAAAAAAA);
                    pCrater->SetPosition(vec);

                    g_pCurrentScene->m_pShadeContainer->AddChild(pCrater);
                }
            }

            if (!g_pDevice->m_bSavage && !g_pDevice->m_bIntel)
            {
                for (int i = 0; i < 3; ++i)
                {
                    TMEffectBillBoard* pBillEffect = new TMEffectBillBoard(nTexIndex,
                        2000,
                        0.80000001f,
                        0.80000001f,
                        0.80000001f,
                        0.001f,
                        1,
                        80);

                    if (pBillEffect)
                    {
                        pBillEffect->m_bStickGround = 1;
                        pBillEffect->m_vecPosition = TMVector3((float)((float)(i * (rand() % 3 - 1)) * 0.40000001f) + vec.x, m_fHeight,
                            (float)((float)(i * (rand() % 3 - 1)) * 0.40000001f) + vec.y);
                        g_pCurrentScene->m_pShadeContainer->AddChild(pBillEffect);
                        m_dwLastDustTime = dwServerTime;
                    }
                }
            }
        }
    }
    if (m_nSkinMeshType == 20 && !m_stLookInfo.HelmMesh && !(dwServerTime % 5000 / 100))
    {
        int nSoundIndex = g_MobAniTable[m_nSkinMeshType].dwSoundTable[1];
        auto pSoundManager = g_pSoundManager;
        if (pSoundManager)
        {
            auto pSoundData = pSoundManager->GetSoundData(nSoundIndex);
            if (pSoundData && !pSoundData->IsSoundPlaying())
            {
                pSoundData->Play();
            }
        }
    }

    unsigned int dwUnitTime = 1000;
    if (m_fMaxSpeed > 0.0f)
        dwUnitTime = (unsigned int)(1000.0f / m_fMaxSpeed);

    unsigned int dwElapsedStartTime = dwServerTime - m_dwStartMoveTime;
    if (!dwUnitTime)
    {
        pScene->m_pMessagePanel->SetMessage("Crashed Character Unit Time", 0);
        pScene->m_pMessagePanel->SetVisible(1, 1);
        return 1;
    }

    int nRouteIndex = (int)(dwElapsedStartTime / dwUnitTime);
    float fProgressRate = (float)(dwElapsedStartTime - (nRouteIndex * dwUnitTime)) / (float)dwUnitTime;
    if (nRouteIndex < m_nMaxRouteIndex)
        m_bMoveing = 1;
    else
    {
        nRouteIndex = m_nMaxRouteIndex;
        fProgressRate = 1.0f;
        m_bMoveing = 0;
    }

    if (m_nMaxRouteIndex + 1 < 48 && 
        (m_vecRouteBuffer[m_nMaxRouteIndex].x != m_vecRouteBuffer[m_nMaxRouteIndex + 1].x || 
         m_vecRouteBuffer[m_nMaxRouteIndex].y != m_vecRouteBuffer[m_nMaxRouteIndex + 1].y))
    {
        ++m_nMaxRouteIndex;
    }
    if (nRouteIndex != m_nLastRouteIndex)
    {
        MoveTo(m_vecRouteBuffer[(nRouteIndex + 1) % 48]);
        m_nLastRouteIndex = nRouteIndex % 48;
    }

    float fElapsedAngleToTime = (float)(dwServerTime - m_dwMoveToTime) * 0.0049999999f;
    if (fElapsedAngleToTime > 1.0f)
        fElapsedAngleToTime = 1.0f;

    float fDAngle = 0.0f;
    if (!m_bForcedRotation)
    {
        fDAngle = m_fWantAngle - m_fMoveToAngle;
    }
    else
    {
        m_fWantAngle = m_fCurrAng;
        m_fCurrAng = m_fCurrAng + 0.15000001f;
        if (m_fCurrAng > 360.0f)
            m_fCurrAng = 0.0f;

        fDAngle = m_fWantAngle - m_fMoveToAngle;
        if (m_dwForcedRotCurTime >= m_dwForcedRotMaxTime)
        {
            m_bForcedRotation = 0;
            m_fCurrAng = 0.0f;
            m_cMotionLoopCnt = 0;
            SetAnimation(ECHAR_MOTION::ECMOTION_STAND01, 0);
        }
        else
        {
            m_dwForcedRotCurTime = g_pTimerManager->GetServerTime();

            if (!m_cMotionLoopCnt && m_dwForcedRotMaxTime - m_dwForcedRotCurTime < 6000
              || m_cMotionLoopCnt == 1 && m_dwForcedRotMaxTime - m_dwForcedRotCurTime < 4000
              || m_cMotionLoopCnt == 2 && m_dwForcedRotMaxTime - m_dwForcedRotCurTime < 2000)
            {
                ++m_cMotionLoopCnt;
                SetAnimation(m_eMotion, 10);
            }
            m_fWantAngle = m_fCurrAng;
        }
    }    

    if (fabsf(m_fAngle - m_fWantAngle) > 0.017453292f)
    {
        if (m_bForcedRotation)
            m_fAngle = fElapsedAngleToTime * fDAngle;
        else
            m_fAngle = (fElapsedAngleToTime * fDAngle) + m_fMoveToAngle;

        SetAngle(0.0f, m_fAngle, 0.0f);
    }

    m_fProgressRate = fProgressRate;
    TMVector2 vecCurrent = (m_vecRouteBuffer[nRouteIndex] * (1.0f - fProgressRate)) + (m_vecRouteBuffer[nRouteIndex + 1] * fProgressRate);

    if (m_cSameHeight == 1)
    {
        fProgressRate = (float)(dwServerTime - m_dwStartMoveTime) / 2000.0f;
        vecCurrent.x = (float)((float)((float)m_vecStartPos.x * (float)(1.0f - fProgressRate))
            + (float)((float)m_vecTargetPos.x * fProgressRate))
            + 0.5f;
        vecCurrent.y = (float)((float)((float)m_vecStartPos.y * (float)(1.0f - fProgressRate))
            + (float)((float)m_vecTargetPos.y * fProgressRate))
            + 0.5f;
    }

    if (fProgressRate > 1.0f)
        m_cSameHeight = 0;

    m_vecPosition = vecCurrent;
    FrameMoveEffect(dwServerTime);

    if (pScene && pScene->m_pGround)
    {
        float fCurrent = (float)pScene->GroundGetMask(m_vecRouteBuffer[nRouteIndex]) * 0.1f;
        float fNext = (float)pScene->GroundGetMask(m_vecRouteBuffer[nRouteIndex + 1]) * 0.1f;
        if (m_cSameHeight == 1)
        {
            fCurrent = (float)pScene->GroundGetMask(m_vecStartPos) * 0.1f;
            fNext = (float)pScene->GroundGetMask(m_vecTargetPos) * 0.1f;
        }

        m_fWantHeight = ((1.0f - fProgressRate) * fCurrent) + (float)(fNext * fProgressRate);
        if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR)
        {
            if (m_vecPosition.y >= 2060.0f)
                m_fWantHeight = 4.09f;
            else
                m_fWantHeight = 0.1f;
        }

        if (m_dwID < 0 || m_dwID > 1000)
            m_fWantHeight = m_fWantHeight + fDieHeight;

        if (m_nSkinMeshType == 20)
        {
            if (!m_stLookInfo.HelmMesh && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                m_fWantHeight = m_fWantHeight + 1.2f;
            else if (m_stLookInfo.HelmMesh == 2 && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                m_fWantHeight = m_fWantHeight + 1.0f;
        }
        else if (m_nSkinMeshType == 24)
        {
            if (m_eMotion == ECHAR_MOTION::ECMOTION_RUN || m_eMotion == ECHAR_MOTION::ECMOTION_WALK && m_bParty == 1)
                m_fWantHeight = m_fWantHeight + 1.2f;
            else if (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02)
            {
                if (m_fMaxSpeed > 2.0f || m_bParty == 1)
                    m_fWantHeight = m_fWantHeight + 1.2f;
            }
            else if ((int)m_eMotion >= 4 && (int)m_eMotion <= 9 && (m_fMaxSpeed > 2.0f || m_bParty == 1))
                m_fWantHeight = m_fWantHeight + 1.1f;
        }
    }

    if (!m_bIgnoreHeight)
        m_fHeight = GetMyHeight();

    if (m_bIgnoreHeight)
        SetPosition(m_vecPosition.x + m_vecAirMove.x, m_fHeight, m_vecPosition.y + m_vecAirMove.y);
    else
        SetPosition(m_vecPosition.x, m_fHeight, m_vecPosition.y);

    TMHuman* pFocused = nullptr;
    if(pScene)
        pFocused = g_pCurrentScene->m_pMyHuman;

    if (pFocused != this && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        TMFieldScene* pFieldScene = (TMFieldScene*)g_pCurrentScene;
        SListBox* pPartyList = pFieldScene->m_pPartyList;

        TMVector2 vecD;
        if (pPartyList)
        {
            for (int j = 0; j < pPartyList->m_nNumItem; ++j)
            {
                SListBoxPartyItem* pPartyItem = (SListBoxPartyItem*)pPartyList->m_pItemList[j];
                if (pPartyItem->m_dwCharID == m_dwID)
                {
                    vecD = pFocused->m_vecPosition - m_vecPosition;
                    break;
                }
            }
        }
    }

    TMVector2 vecNow = TMVector2(m_vecRouteBuffer[nRouteIndex].x, m_vecRouteBuffer[nRouteIndex].y);
    TMVector2 vecNext = TMVector2(m_vecRouteBuffer[nRouteIndex + 1].x, m_vecRouteBuffer[nRouteIndex + 1].y);
    
    if (m_cSameHeight != 2)
    {
        vecNow.x = m_vecRouteBuffer[nRouteIndex].x;
        vecNow.y = m_vecRouteBuffer[nRouteIndex].y;
        vecNext.x = m_vecRouteBuffer[nRouteIndex + 1].x;
        vecNext.y = m_vecRouteBuffer[nRouteIndex + 1].y;
    }

    if (vecNow.x == vecNext.x && vecNow.y == vecNext.y)
    {
        if (m_stScore.Hp > 0 && (m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_eMotion == ECHAR_MOTION::ECMOTION_RUN))
        {
            SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);

            if (m_fMaxSpeed <= 10.0f)
            {
                for (int l = nRouteIndex + 1; l < 48; ++l)
                    m_vecRouteBuffer[l] = m_vecRouteBuffer[nRouteIndex];
            }
            else
            {
                for (int nIndex = 0; nIndex < 48; ++nIndex)
                    m_vecRouteBuffer[nIndex] = m_vecRouteBuffer[nRouteIndex];
            }

            m_cOnlyMove = 0;
            SetSpeed(0);

            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
            if (g_pCurrentScene->m_pMyHuman == this && m_pMoveTargetHuman)
            {
                if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                {
                    if (pFScene->m_cAutoAttack == 1)
                        pFScene->m_pTargetHuman = m_pMoveTargetHuman;
                }
                MoveAttack(m_pMoveTargetHuman);
                m_pMoveTargetHuman = 0;
                return 1;
            }
            if (g_pCurrentScene->m_pMyHuman == this && m_pMoveSkillTargetHuman)
            {
                if (g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
                {
                    pFScene->SkillUse((int)m_pMoveSkillTargetHuman->m_vecPosition.x, (int)m_pMoveSkillTargetHuman->m_vecPosition.y,
                        pScene->GroundGetPickPos(), dwServerTime, 1, m_pMoveSkillTargetHuman);
                }
                m_pMoveSkillTargetHuman = 0;
                return 1;
            }
        }
        else if (m_stScore.Hp <= 0 && m_eMotion != ECHAR_MOTION::ECMOTION_DIE && m_nWillDie == 4)
            SetAnimation(ECHAR_MOTION::ECMOTION_DEAD, 1);

        else if (m_stScore.Hp <= 0 && m_eMotion != ECHAR_MOTION::ECMOTION_DIE && m_eMotion != ECHAR_MOTION::ECMOTION_DEAD
            && m_nWillDie == 1)
        {
            SetAnimation(ECHAR_MOTION::ECMOTION_DEAD, 0);
        }
        else if (m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02)
        {
            if (m_cOnlyMove == 1 && m_bSliding == 1)
            {               
                if (m_fMaxSpeed > 6.0f)
                {
                    for (int k = 0; k < 48; ++k)
                        m_vecRouteBuffer[k] = m_vecRouteBuffer[nRouteIndex];
                }
            }

            m_pMoveTargetHuman = 0;
            m_pMoveSkillTargetHuman = 0;
            if (m_cOnlyMove == 1)
                SetSpeed(0);

            m_cOnlyMove = 0;
            m_bSliding = 0;
        }
    }
    else if ((m_eMotion != ECHAR_MOTION::ECMOTION_WALK || m_eMotion != ECHAR_MOTION::ECMOTION_RUN || ((int)m_eMotion < 4 && (int)m_eMotion > 9))
        && !m_bSliding)
    {
        int nWalk = 2;
        if (m_nSkinMeshType == 31 && m_fScale > 0.69999999f || m_cMount == 1 && m_nMountSkinMeshType == 31 && m_fMountScale > 0.69999999f)
            nWalk = 3;
        if (m_nMountSkinMeshType == 40 || m_nMountSkinMeshType == 20 || m_nMountSkinMeshType == 39)
            nWalk = 3;

        if ((float)nWalk < m_fMaxSpeed)
            SetAnimation(ECHAR_MOTION::ECMOTION_RUN, 1);
        else
            SetAnimation(ECHAR_MOTION::ECMOTION_WALK, 1);
    }

    if ((m_eMotion == ECHAR_MOTION::ECMOTION_NONE || m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_eMotion == ECHAR_MOTION::ECMOTION_STAND02) 
        && !m_nWillDie)
    {
        DelayDelete();
        return 1;
    }

    int nWalkSndIndex = g_pCurrentScene->GroundGetTileType(m_vecPosition);
    if (pScene && pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        if (nWalkSndIndex == 8)
            nWalkSndIndex = 8;
        else if (nWalkSndIndex)
        {
            if ((int)m_fHeight != (int)pScene->m_pGround->GetHeight(m_vecPosition))
                nWalkSndIndex = 0;
        }
    }
    if (m_pShade && !m_cHide)
        m_pShade->m_bShow = nWalkSndIndex != 1;
    if (nWalkSndIndex == 11)
        nWalkSndIndex = 1;

    if ((m_nClass != 45 || !m_cHide) && !m_cShadow)
    {
        int nDust = 1;
        if (pScene && pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
        {
            int isInPos = ((int)m_vecPosition.x >> 7 <= 26 || (int)m_vecPosition.x >> 7 >= 31 ||
                (int)m_vecPosition.y >> 7 <= 20 || (int)m_vecPosition.y >> 7 >= 25);
            if (g_nWeather == 1 || !isInPos)
                nDust = 0;
        }

        if (m_cAvatar == 1 && !m_cDie)
        {
            int nTextureIndex = 0;
            if (nWalkSndIndex == 8)
                nTextureIndex = 193;

            float fSpeed = 0.0392f;
            fSpeed = (float)(m_fScale * TMHuman::m_vecPickSize[m_nSkinMeshType].x) * 0.0392f;
            if (!m_cMount && m_nSkinMeshType != 40 || m_cMount && m_nMountSkinMeshType != 40)
            {
                TMEffectBillBoard* pChild = new TMEffectBillBoard(10, 450, 0.1f, 0.1f, 0.1f, fSpeed, 1, 80);

                if (pChild)
                {
                    pChild->m_bStickGround = 1;
                    pChild->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pChild->m_vecPosition = TMVector3(m_vecPosition.x,
                        m_fHeight - 0.89999998f,
                        m_vecPosition.y);

                    g_pCurrentScene->m_pEffectContainer->AddChild(pChild);
                    m_dwLastDustTime = dwServerTime;
                }

                pChild->SetColor(0x30101010);
            }
        }
        else if (nDust && (!g_pDevice->m_bSavage && !g_pDevice->m_bIntel && m_eMotion == ECHAR_MOTION::ECMOTION_RUN && !m_cHide
                || (m_nClass == 22 || m_nClass == 20) && m_eMotion == ECHAR_MOTION::ECMOTION_WALK) && 
            dwServerTime - m_dwLastDustTime > (unsigned int)(1000.0f / m_fMaxSpeed))
        {
            int nTextureIndex = 0;
            if (nWalkSndIndex == 8)
                nTextureIndex = 193;

            float fVelocity = 0.0012f;
            fVelocity = (float)(m_fScale * TMHuman::m_vecPickSize[m_nSkinMeshType].x) * 0.0012000001f;
            if (!m_cMount && m_nSkinMeshType != 40 || m_cMount && m_nMountSkinMeshType != 40)
            {
                TMEffectBillBoard* pChild = new TMEffectBillBoard(nTextureIndex, 2000, 0.5f, 0.5f, 0.5f, fVelocity, 1, 80);

                if (pChild)
                {
                    pChild->m_bStickGround = 1;
                    pChild->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pChild->SetColor(0x0FFFFFFFF);
                    pChild->m_vecPosition = TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y);

                    g_pCurrentScene->m_pEffectContainer->AddChild(pChild);
                    m_dwLastDustTime = dwServerTime;
                }
            }
        }
    }    
    if (m_nClass != 45 && !m_cHide)
    {
        unsigned dwFootTerm = (unsigned int)((1000.0f * m_fScale) / m_fMaxSpeed);
        if (dwFootTerm > 230 && dwFootTerm < 350)
            dwFootTerm = 230;
        if (dwFootTerm > 340)
            dwFootTerm = 340;
        if (m_nLegType == 3)
            dwFootTerm = 200;
        if (m_nSkinMeshType == 11)
            dwFootTerm *= 2;
        if (m_nSkinMeshType == 39)
            dwFootTerm *= 2;

        if (dwServerTime - m_dwFootMastTime > dwFootTerm && m_nLegType > 0 && 
            (m_eMotion == ECHAR_MOTION::ECMOTION_WALK || m_eMotion == ECHAR_MOTION::ECMOTION_RUN))
        {
            int isInPos = (int)m_vecPosition.x >> 7 <= 26 || (int)m_vecPosition.x >> 7 >= 31 || 
                (int)m_vecPosition.y >> 7 <= 20 || (int)m_vecPosition.y >> 7 >= 25;

            if (nWalkSndIndex == 9 || (nWalkSndIndex == 8 && !isInPos))
            {
                unsigned int dwCol = 0xFFFF8866;
                if (nWalkSndIndex == 9)
                    dwCol = 0x990000FF;

                int nScale = 2;
                int nFootType = 194;
                if (m_cMount > 0)
                    nFootType = 195;
                switch (m_nLegType)
                {
                case 2:
                    nFootType = 195;
                    if (m_fScale > 1.3f)
                        nScale = 3;
                    break;
                case 3:
                    nFootType = 196;
                    nScale = 3;
                    dwCol = 0xFFFFAA88;
                    break;
                case 4:
                    nFootType = 197;
                    nScale = 4;
                    break;
                case 5:
                    nFootType = 198;
                    nScale = 4;
                    break;
                }

                if (nFootType == 194 && m_fScale > 1.5f)
                    nScale = 3;
                if (m_nSkinMeshType == 39)
                    nScale = 4;

                TMShade* pFootMark = new TMShade(nScale, nFootType, 1.0f);
                if (pFootMark)
                {
                    pFootMark->m_nFade = 1;
                    pFootMark->m_bFI = 0;
                    pFootMark->m_dwLifeTime = 3000;
                    pFootMark->m_fAngle = m_fAngle - D3DXToRadian(90);
                    pFootMark->SetColor(dwCol);
                    pFootMark->SetPosition(m_vecPosition);

                    g_pCurrentScene->m_pShadeContainer->AddChild(pFootMark);
                    m_dwFootMastTime = dwServerTime;
                }
            }
        }
    }
    if (m_nSkinMeshType == 20 && (m_stLookInfo.HelmMesh == 2 || m_stLookInfo.HelmMesh == 4 && m_stLookInfo.HelmSkin == 1))
    {
        unsigned int dwTime = 1000;
        int nParts = 2;
        unsigned int dwColor = 0xFFFF0000;

        if (m_stLookInfo.HelmMesh == 2 && m_fScale < 0.60000002f)
        {
            dwTime = 1200;
            nParts = 6;
        }
        else if (m_stLookInfo.HelmMesh == 4)
        {
            nParts = 4;
            dwColor = 0x0FFFF9900;
            if (m_fScale < 0.60000002f)
            {
                dwTime = 800;
                nParts = 6;
            }
        }
        if (dwServerTime - m_dwLastDFire > dwTime)
        {
            for (int m = 1; m < 10 - nParts; ++m)
            {
                TMEffectBillBoard* pFire = new TMEffectBillBoard(44,
                    2000,
                    m_fScale * 4.0f,
                    m_fScale * 4.0f,
                    m_fScale * 4.0f,
                    0.00050000002f,
                    1,
                    80);

                if (pFire)
                {
                    pFire->SetColor(dwColor);
                    pFire->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                    pFire->m_nFade = 1;
                    pFire->m_vecPosition = m_vecTempPos[m];
                    g_pCurrentScene->m_pEffectContainer->AddChild(pFire);
                }
            }
            m_dwLastDFire = dwServerTime;
        }
    }
    if (m_pSkinMesh)
    {
        if (!m_bVisible && m_bDoubleAttack == 1)
        {
            unsigned int dwFPS = m_pSkinMesh->m_dwFPS;
            int nSkinMeshType = m_nSkinMeshType;
            int nAniIndex = m_pSkinMesh->m_nAniIndex;
            unsigned int dwMod = MeshManager::m_BoneAnimationList[nSkinMeshType].numAniCut[nAniIndex];

            if (dwServerTime > m_dwStartAnimationTime + 4 * dwMod * dwFPS)
            {
                if (m_nLoop == 0)
                {
                    if (m_eMotion != ECHAR_MOTION::ECMOTION_DIE && m_bDoubleAttack == 1)
                        m_bDoubleAttack = 0;
                }
                else if (m_nLoop == 1)
                {
                    if ((int)m_eMotion < 4 || (int)m_eMotion > 9)
                        m_bDoubleAttack = 0;
                    else if (!g_pEventTranslator->button[0] && dwServerTime > m_dwStartAnimationTime + 4 * dwMod * m_pSkinMesh->m_dwFPS)
                    {
                        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
                        m_bDoubleAttack = 0;
                    }
                }
            }
        }
        else if (m_bVisible == 1)
        {
            SetColorMaterial();
            unsigned int dwFPS = m_pSkinMesh->m_dwFPS;
            int nSkinMeshType = m_nSkinMeshType;
            int nAniIndex = m_pSkinMesh->m_nAniIndex;

            if (m_cMount)
            {
                if (m_pMount)
                {
                    m_pMount->FrameMove(dwServerTime);
                    if (m_eMotion == ECHAR_MOTION::ECMOTION_DIE)
                    {
                        nSkinMeshType = m_nMountSkinMeshType;
                        dwFPS = m_pMount->m_dwFPS;
                        nAniIndex = m_pMount->m_nAniIndex;
                    }
                }
            }

            m_pSkinMesh->FrameMove(dwServerTime);
            if (m_cMantua > 0 && m_pMantua)
                m_pMantua->FrameMove(dwServerTime);

            unsigned int dwMod = MeshManager::m_BoneAnimationList[nSkinMeshType].numAniCut[nAniIndex];
            if (dwMod > 2)
                dwMod -= 2;

            if ((m_dwID > 0 && m_dwID < 1000) && m_bDoubleAttack == 1 && m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
                m_bDoubleAttack = 0;

            if (dwServerTime > m_dwStartAnimationTime + 4 * dwMod * dwFPS)
            {
                if (m_nLoop == 0)
                {
                    if (m_eMotion == ECHAR_MOTION::ECMOTION_DIE)
                    {
                        SetAnimation(ECHAR_MOTION::ECMOTION_DEAD, 1);
                        if (m_nClass == 64 && m_sHeadIndex == 397)
                        {
                            m_cHide = 1;
                            TMEffectParticle* pParticle = new TMEffectParticle(TMVector3(m_vecPosition.x, m_fHeight, m_vecPosition.y),
                                0,
                                8,
                                10.0f,
                                0xFFFF3333,
                                1,
                                56,
                                1.0f,
                                1,
                                TMVector3(0.0f, 0.0f, 0.0f), 1000);

                            if (pParticle)
                                g_pCurrentScene->m_pEffectContainer->AddChild(pParticle);
                        }
                        if (g_pCurrentScene->m_pMyHuman == this)
                        {
                            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
                            if (!IsInTown())
                            {
                                pFScene->m_pMessageBox->SetMessage(g_pMessageStringTable[27], 11u, 0);
                                pFScene->m_pMessageBox->SetVisible(1);
                            }
                        }
                        else
                        {
                            if (m_dwID >= 0 && m_dwID < 1000)
                            {
                                if (!m_dwDeadTime)
                                    m_dwDeadTime = dwServerTime;
                                if (!m_cClone)
                                    m_nWillDie = 1;
                            }
                        }
                        m_bDoubleAttack = 0;
                    }
                    else if (m_eMotion == ECHAR_MOTION::ECMOTION_SEAT)
                        SetAnimation(ECHAR_MOTION::ECMOTION_SEATING, 1);
                    else if (m_eMotion == ECHAR_MOTION::ECMOTION_PUNISH)
                        SetAnimation(ECHAR_MOTION::ECMOTION_PUNISHING, 1);
                    else if (m_bDoubleAttack == 1)
                    {
                        if (m_nClass == 33 && m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK01)
                        {
                            m_pSkinMesh->m_nAniIndex = 0;
                            SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK01, 0);
                        }
                        else if (m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK01)
                        {
                            m_pSkinMesh->m_nAniIndex = 0;
                            SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK02, 0);
                        }
                        else if (m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK02)
                        {
                            m_pSkinMesh->m_nAniIndex = 0;
                            SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK03, 0);
                        }
                        else if (m_eMotion == ECHAR_MOTION::ECMOTION_ATTACK03)
                        {
                            m_pSkinMesh->m_nAniIndex = 0;
                            SetAnimation(ECHAR_MOTION::ECMOTION_ATTACK01, 0);
                        }
                        m_bDoubleAttack = 0;
                    }
                    else if (m_bSkill != 1 || m_nMotionIndex < 0 || m_nMotionIndex >= 3 || m_eMotionBuffer[m_nMotionIndex + 1] == ECHAR_MOTION::ECMOTION_NONE)
                    {
                        if (m_eMotion == ECHAR_MOTION::ECMOTION_HOLYTOUCH || m_eMotion == ECHAR_MOTION::ECMOTION_RELAX)
                        {
                            float fEffectLen = 1.0f;
                            if (m_cMount == 1)
                                fEffectLen = 1.5f;

                            if (m_bSwordShadow[0] == 1 && m_pSkinMesh->m_pSwingEffect[0])
                            {
                                if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                                    m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                                else
                                    m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * fEffectLen;

                                m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = dwServerTime;
                            }
                            if (m_bSwordShadow[1] == 1 && m_pSkinMesh->m_pSwingEffect[1])
                            {
                                m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                                m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = dwServerTime;
                            }
                        }
                        m_cPunish = 0;
                        m_bSkill = 0;
                        m_nMotionIndex = -1;
                        for (int n = 0; n < 4; ++n)
                            m_eMotionBuffer[n] = ECHAR_MOTION::ECMOTION_NONE;

                        if (m_pSkinMesh->m_pSwingEffect[0])
                            m_pSkinMesh->m_pSwingEffect[0]->m_cFireEffect = 0;
                        if (m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[1]->m_cFireEffect = 0;
                        if (m_pSkinMesh->m_pSwingEffect[0])
                            m_pSkinMesh->m_pSwingEffect[0]->m_cGoldPiece = 0;
                        if (m_pSkinMesh->m_pSwingEffect[1])
                            m_pSkinMesh->m_pSwingEffect[1]->m_cGoldPiece = 0;

                        SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
                        m_bDoubleAttack = 0;
                    }
                    else
                    {
                        SetAnimation(m_eMotionBuffer[++m_nMotionIndex], 0);
                    }
                }
                else if (m_nLoop == 1)
                {
                    if ((int)m_eMotion >= 4 && (int)m_eMotion <= 6)
                    {
                        float fEffectLen = 1.0f;
                        if (m_cMount == 1)
                            fEffectLen = 1.5;

                        if (m_bSwordShadow[0] == 1 && m_pSkinMesh->m_pSwingEffect[0])
                        {
                            if (m_nWeaponTypeL == 41 && m_pSkinMesh->m_pSwingEffect[1])
                                m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                            else
                                m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = m_fSowrdLength[0] * fEffectLen;
                            m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = dwServerTime;
                        }
                        if (m_bSwordShadow[1] == 1 && m_pSkinMesh->m_pSwingEffect[1])
                        {
                            m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                            m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = dwServerTime;
                        }
                        if (!g_pEventTranslator->button[0] && dwServerTime > m_dwStartAnimationTime + 4 * dwMod * m_pSkinMesh->m_dwFPS)
                        {
                            SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
                            m_bDoubleAttack = 0;
                        }
                    }
                    else if ((int)m_eMotion >= 7 && (int)m_eMotion <= 9)
                    {
                        float fEffectLen = 1.0f;
                        if (m_cMount == 1)
                            fEffectLen = 1.5f;
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
                                m_pSkinMesh->m_pSwingEffect[0]->m_dwStartTime = dwServerTime;
                        }
                        if (m_bSwordShadow[1] == 1)
                        {
                            if (m_pSkinMesh->m_pSwingEffect[1])
                                m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = m_fSowrdLength[1] * fEffectLen;
                            if (m_pSkinMesh->m_pSwingEffect[1])
                                m_pSkinMesh->m_pSwingEffect[1]->m_dwStartTime = dwServerTime;
                        }
                        if (!g_pEventTranslator->button[1] && dwServerTime > m_dwStartAnimationTime + 4 * dwMod * m_pSkinMesh->m_dwFPS)
                        {
                            SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
                            m_bDoubleAttack = 0;
                        }
                    }
                    else
                    {
                        m_bDoubleAttack = 0;
                    }
                }
            }
            AnimationFrame(nWalkSndIndex);

            if (m_pShade)
            {
                if (m_cMount && m_pMount)
                    m_pShade->SetPosition(TMVector2(m_pMount->m_vPosition.x, m_pMount->m_vPosition.z));
                else if(m_pSkinMesh)
                    m_pShade->SetPosition(TMVector2(m_pSkinMesh->m_vPosition.x, m_pSkinMesh->m_vPosition.z));
            }
        }
    }

    return 1;
}

void TMHuman::InitPosition(float fX, float fY, float fZ)
{
    if (m_dwDelayDel != 0)
        return;

    SetPosition(fX, fY, fZ);
    m_fWantHeight = fY;

    m_vecMoveToPos = m_vecPosition;
    m_vecFromPos = m_vecPosition;

    m_vecTargetPos.x = (int)m_vecPosition.x;
    m_vecTargetPos.y = (int)m_vecPosition.y;

    m_vecDPosition = TMVector2(0.0f, 0.0f);

    for (int i = 0; i < 48; i++)
        m_vecRouteBuffer[i] = m_vecPosition;

    m_LastSendTargetPos = m_vecTargetPos;
}

void TMHuman::InitAngle(float fYaw, float fPitch, float fRoll)
{
    if (m_dwDelayDel != 0)
        return;

    if (m_nClass == 44)
        fPitch = D3DXToRadian(180);

    TMObject::InitAngle(fYaw, fPitch, fRoll);
    m_fWantAngle = m_fAngle;
    m_fMoveToAngle = m_fAngle;
}

void TMHuman::SetAngle(float fYaw, float fPitch, float fRoll)
{
    if (m_dwDelayDel != 0)
        return;

    m_fAngle = fPitch;
    if (m_cMount == 0)
    {
        if (m_pSkinMesh)
            m_pSkinMesh->SetAngle(fYaw, -fPitch, fRoll);
    }
    else
    {
        if (m_pMount)
            m_pMount->SetAngle(fYaw, -fPitch + D3DXToRadian(360), fRoll);
        if (m_pSkinMesh)
            m_pSkinMesh->SetAngle(0.0f, 0.0f, 0.0f);
    }
}

void TMHuman::SetPosition(float fX, float fY, float fZ)
{
    if (m_dwDelayDel != 0)
        return;

    m_vecPosition.x = fX;
    m_vecPosition.y = fZ;
    m_fHeight = fY;

    float fHSize = (TMHuman::m_vecPickSize[m_nSkinMeshType].x * m_fScale) / 2.0f;
    if (m_cMount)
        fHSize = (TMHuman::m_vecPickSize[m_nMountSkinMeshType].x * m_fScale) / 2.0f;

    float fDX = cosf(m_fAngle) * fHSize;
    float fDY = sinf(m_fAngle) * fHSize;

    if (m_cMount == 0)
    {
        if (m_nClass == 44)
            fDX = fDX + 0.5f;
        if (m_pSkinMesh)
            m_pSkinMesh->SetPosition(fX + fDX, fY, fZ - fDY);
    }
    else
    {
        if (m_pMount)
            m_pMount->SetPosition(fX + fDX, fY, fZ - fDY);
        if (m_pSkinMesh)
            m_pSkinMesh->SetPosition(0.0f, 0.0f, 0.0f);
    }
}

void TMHuman::RestoreDeviceObjects()
{
    if (m_dwDelayDel)
        return;

    if (m_stLookInfo.FaceMesh == 40)
    {
        if (!m_stLookInfo.CoatMesh)
            m_stLookInfo.CoatMesh = 40;
        if (!m_stLookInfo.PantsMesh)
            m_stLookInfo.PantsMesh = 40;
        if (!m_stLookInfo.GlovesMesh)
            m_stLookInfo.GlovesMesh = 40;
        if (!m_stLookInfo.BootsMesh)
            m_stLookInfo.BootsMesh = 40;

        m_stLookInfo.HelmMesh = 40;
    }
    else if (m_stLookInfo.FaceMesh == 80 || m_stLookInfo.FaceMesh == 64)
    {
        if (!m_stLookInfo.CoatMesh)
            m_stLookInfo.CoatMesh = 40;
        if (!m_stLookInfo.PantsMesh)
            m_stLookInfo.PantsMesh = 40;
        if (!m_stLookInfo.GlovesMesh)
            m_stLookInfo.GlovesMesh = 40;
        if (!m_stLookInfo.BootsMesh)
            m_stLookInfo.BootsMesh = 40;

        m_stLookInfo.HelmMesh = m_stLookInfo.FaceMesh;
    }
    if (!m_pSkinMesh)
    {
        bool bExpand = false;
        if (m_nClass == 4 || m_nClass == 8 || m_nClass == 36 || m_nClass == 39 || m_nClass == 40 || m_nClass == 60)
            bExpand = true;

        m_pSkinMesh = new TMSkinMesh((LOOK_INFO*)&m_stLookInfo,
            &m_stSancInfo,
            m_nSkinMeshType,
            bExpand,
            &m_stColorInfo,
            1,
            0,
            0);

        if (!m_pSkinMesh)
            return;
    }
    if (m_pSkinMesh)
    {
        m_pSkinMesh->m_pOwner = this;
        m_pSkinMesh->RestoreDeviceObjects();
        m_pSkinMesh->m_dwFPS = 40;

        if (m_nSkinMeshType == 31 && m_fScale < 1.0f)
        {
            float fGrade = 2.0f - m_fScale;
            m_pSkinMesh->m_vScale.x = m_fScale;
            m_pSkinMesh->m_vScale.y = m_fScale / fGrade;
            m_pSkinMesh->m_vScale.z = m_fScale;
        }
        else
        {
            m_pSkinMesh->m_vScale.x = m_fScale;
            m_pSkinMesh->m_vScale.y = m_fScale;
            m_pSkinMesh->m_vScale.z = m_fScale;
        }
    }

    if (m_nSkinMeshType == 31 && m_fScale < 1.0f)
    {
        float fGrade = 2.0f - m_fScale;
        m_pSkinMesh->m_vScale.x = m_fScale;
        m_pSkinMesh->m_vScale.y = m_fScale / fGrade;
        m_pSkinMesh->m_vScale.z = m_fScale;
    }
    else
    {
        m_pSkinMesh->m_vScale.x = m_fScale;
        m_pSkinMesh->m_vScale.y = m_fScale;
        m_pSkinMesh->m_vScale.z = m_fScale;
    }

    if (m_cMantua > 0 && !m_nSkinMeshType || m_nSkinMeshType == 1 || m_nSkinMeshType == 8 || m_nSkinMeshType == 3 || m_nSkinMeshType == 2)
    {
        if (!m_pMantua)
        {
            LOOK_INFO stLook{};
            stLook.Skin0 = m_wMantuaSkin;

            SANC_INFO stSanc{};
            stSanc.Sanc0 = m_ucMantuaSanc;
            stSanc.Legend0 = m_ucMantuaLegend;

            m_pMantua = new TMSkinMesh(&stLook, &stSanc, 85, 0, 0, 1, 0, 0);

            m_pMantua->m_pOwner = 0;
            m_pMantua->m_dwFPS = 40;
            m_pMantua->m_vScale.x = m_fScale;
            m_pMantua->m_vScale.y = m_fScale;
            m_pMantua->m_vScale.z = m_fScale;
            m_pMantua->SetVecMantua(1, m_nMountSkinMeshType);
        }
        m_pMantua->RestoreDeviceObjects();
    }

    UpdateMount();
}

void TMHuman::InvalidateDeviceObjects()
{
    if (m_dwDelayDel != 0)
        return;

    if (m_pSkinMesh)
        m_pSkinMesh->InvalidateDeviceObjects();
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

int TMHuman::OnCharEvent(char iCharCode, int lParam)
{
    if (m_dwDelayDel)
        return 0;

    return TreeNode::OnCharEvent(iCharCode, lParam);
}

int TMHuman::IsMerchant()
{
    if ((m_stScore.Reserved & 0xF) >= 1 && (m_stScore.Reserved & 0xF) <= 14)
        return 1;

    if (m_sHeadIndex == 54 || m_sHeadIndex == 55 || m_sHeadIndex == 56 || m_sHeadIndex == 57 || m_sHeadIndex == 51 || m_sHeadIndex == 68 || m_sHeadIndex == 67)
        return 1;
    
    return 0;
}

void TMHuman::Init()
{
    SAFE_DELETE(m_pInMiniMap);

    m_nWillDie = -1;
    m_bCNFMobKill = 0;
    m_cDie = 0;
    m_bMouseOver = 0;
    m_bParty = 0;
    m_nHandEffect = 0;
    m_nSkillIndex = -1;
    m_bDoubleAttack = 0;
    m_bSkill = 0;
    m_cPoison = 0;
    m_cHaste = 0;
    m_cAssert = 0;
    m_cFreeze = 0;
    m_cPunish = 0;
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
    m_cProtector = 0;
    m_cShadow = 0;
    m_cElimental = 0;
    m_cDodge = 0;
    m_cHuntersVision = 0;
    m_bSkillBlack = 0;
    m_cOverExp = 0;
    m_DilpunchJewel = 0;
    m_MoonlightJewel = 0;
    m_JewelGlasses = 0;
    m_BloodJewel = 0;
    m_RedJewel = 0;
    m_cGodCos = 0;
    m_cLifeDrain = 0;
    m_cEnchant = 0;
    m_cManaControl = 0;
    m_cArmorClass = 0;
    m_cImmunity = 0;
    m_cCoinArmor = 0;
    m_cCriticalArmor = 0;
    m_cSoul = 0;
    SetAvatar(0);

    m_dwObjType = 3;
    m_eMotion = ECHAR_MOTION::ECMOTION_NONE;
    m_SendeMotion = ECHAR_MOTION::ECMOTION_NONE;

    for (int i = 0; i < 4; ++i)
        m_eMotionBuffer[i] = ECHAR_MOTION::ECMOTION_NONE;

    m_nMotionIndex = -1;
    m_fMaxSpeed = 2.0f;
    m_nWeaponTypeIndex = 0;

    m_vecPosition = TMVector2(0.0f, 0.0f);
    m_vecAttTargetPos = TMVector2(0.0f, 0.0f);
    m_vecOldFire = TMVector3(0.0f, 0.0f, 0.0f);
    m_LastSendTargetPos = IVector2(0, 0);

    m_bSelected = 0;
    m_vecTargetPos.x = -1;
    m_vecTargetPos.y = -1;
    m_dwOldMovePacketTime = 0;
    m_dwLastMagicShield = 0;
    m_dwLastCancelTime = 0;
    m_dwLastSpeedUp = 0;
    m_dwMoveToTime = 0;
    m_dwStartMoveTime = 0;
    m_dwDeadTime = 0;
    m_dwStartDie = 0;
    m_dwLastWaste = 0;
    m_dwLastbomb = 0;
    m_dwLastbombCheck = 0;
    m_dwLastDFire = 0;
    m_dwLastDustTime = 0;
    m_dwFootMastTime = 0;
    m_dwDodgeTime = 0;
    m_dwStartAnimationTime = 0;
    m_dwLastDummyTime = 0;
    m_dwWaterTime = 0;
    m_dwLastHaste = 0;
    m_dwElimental = 0;
    m_dwStartChatMsgTime = 0;
    m_dwLastPlayPunchedTime = 0;
    m_dwChatDelayTime = 3000;
    m_dwGolemDustTime = 0;
    m_nLoop = 1;
    m_nMaxRouteIndex = 1;
    m_fProgressRate = 0.0f;
    m_nLastRouteIndex = 47;
    m_sLeftIndex = 0;
    m_sRightIndex = 0;
    m_bSwordShadow[0] = 0;
    m_bSwordShadow[1] = 0;
    m_fSowrdLength[0] = 0.0;
    m_fSowrdLength[1] = 0.0;
    m_sAttackLR = -1;
    m_sPunchLR = -1;
    m_nDoubleCount = 0;

    memset(&m_usAffect, 0, sizeof(m_usAffect));
    memset(&m_stAffect, 0, sizeof(m_stAffect));
    memset(&m_cRouteBuffer, 0, sizeof(m_cRouteBuffer));
    memset(&m_stLookInfo, 0, sizeof(m_stLookInfo));
    memset(&m_stSancInfo, 0, sizeof(m_stSancInfo));
    memset(&m_stColorInfo, 0, sizeof(m_stColorInfo));
    memset(m_szName, 0, sizeof(m_szName));
    memset(m_szNickName, 0, sizeof(m_szNickName));
    memset(&m_stScore, 0, sizeof(m_stScore));
    memset(&m_stPunchEvent, 0, sizeof(m_stPunchEvent));
    memset(&m_stEffectEvent, 0, sizeof(m_stEffectEvent));
    memset(&m_dsBufferParams, 0, sizeof(m_dsBufferParams));

    m_dsBufferParams.dwSize = 64;
    m_dsBufferParams.flMinDistance = 0.05f;
    m_dsBufferParams.flMaxDistance = 14.0f;
    m_dsBufferParams.vConeOrientation.x = 1.0f;
    m_dsBufferParams.vConeOrientation.z = 1.0f;

    if (m_pChatMsg)
        m_pChatMsg->SetVisible(0);
}

void TMHuman::UpdateScore(int nGuildLevel)
{
    if (!m_dwDelayDel)
    {
        if (!m_MaxBigHp)
        {
            if (m_pProgressBar)
            {
                m_pProgressBar->SetMaxProgress(m_stScore.MaxHp);
                m_pProgressBar->SetCurrentProgress(m_stScore.Hp);
                m_pTitleProgressBar->SetMaxProgress(m_stScore.MaxHp);
                m_pTitleProgressBar->SetCurrentProgress(m_stScore.Hp);
            }

            SetGuildBattleHPBar(m_stScore.Hp);
        }
        else
        {
            m_pProgressBar->SetMaxProgress(m_MaxBigHp);
            m_pProgressBar->SetCurrentProgress(m_BigHp);
            m_pTitleProgressBar->SetMaxProgress(m_MaxBigHp);
            m_pTitleProgressBar->SetCurrentProgress(m_BigHp);

            SetGuildBattleHPBar(m_BigHp);
        }

        SetGuildBattleLifeCount();

        TMFieldScene* pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
        if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && pScene->m_pMyHuman == this)
        {
            auto pHPBar = pScene->m_pHPBar;
            auto pMPBar = pScene->m_pMPBar;
            auto pCurrentHPText = pScene->m_pCurrentHPText;
            auto pMaxHPText = pScene->m_pMaxHPText;
            auto pCurrentMPText = pScene->m_pCurrentMPText;
            auto pMaxMPText = pScene->m_pMaxMPText;
            auto pCurrentMHPText = pScene->m_pCurrentMHPText;
            auto pMaxMHPText = pScene->m_pMaxMHPText;

            memcpy(&g_pObjectManager->m_stMobData.CurrentScore, &m_stScore, sizeof m_stScore);
            if (pCurrentHPText)
            {
                if (m_stScore.Hp > m_stScore.MaxHp)
                    m_stScore.Hp = m_stScore.MaxHp;

                char szHP[32] = { 0 };
                sprintf_s(szHP, "%d", m_stScore.Hp);
                pCurrentHPText->SetText(szHP, 0);
            }
            if (pMaxHPText)
            {
                char _Buffer[32] = { 0 };
                sprintf_s(_Buffer, "/ %d", m_stScore.MaxHp);
                pMaxHPText->SetText(_Buffer, 0);
            }
            if (pCurrentMPText)
            {
                char szMP[32] = { 0 };
                sprintf_s(szMP, "%d", m_stScore.Mp);
                pCurrentMPText->SetText(szMP, 0);
            }
            if (pMaxMPText)
            {
                char szMP[32] = { 0 };
                sprintf_s(szMP, "/ %d", m_stScore.MaxMp);
                pMaxMPText->SetText(szMP, 0);
            }
            if (pHPBar)
            {
                pHPBar->SetMaxProgress(m_stScore.MaxHp);
                pHPBar->SetCurrentProgress(m_stScore.Hp);
            }

            if (pMPBar)
            {
                pMPBar->SetMaxProgress(m_stScore.MaxMp);
                pMPBar->SetCurrentProgress(m_stScore.Mp);
                pMPBar->SetVisible(1);
            }
        }

        auto pDest = strchr(m_szName, '^');
        if (pDest && !IsClearString2(m_szName, pDest - m_szName))
            pDest = nullptr;

        if(pDest == nullptr)
        {
            if (m_pNameLabel)
                m_pNameLabel->SetText(m_szName, 1);

            if (m_pTitleNameLabel)
                m_pTitleNameLabel->SetText(m_szName, 2);

            if (m_pNickNameLabel)
            {
                m_pNickNameLabel->SetText(m_szNickName, 1);
                m_pNickNameLabel->SetTextColor(0xFFCCCCCC);
            }

            if (pScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
            {
                if ((m_dwID >= 0 && m_dwID < 1000) && (m_nCurrentKill || m_nTotalKill > 0))
                {
                    bool isInPos = (int)m_vecPosition.x >> 7 <= 16 || (int)m_vecPosition.x >> 7 >= 20 || 
                        (int)m_vecPosition.y >> 7 <= 29 ? 0 : 1;

                    if (isInPos == 1 ? pScene->m_pMyHuman == this : 1)
                    {
                        char szTemp[128] = { 0 };
                        sprintf_s(szTemp, "%d", m_nCurrentKill);

                        if (m_pKillLabel)
                            m_pKillLabel->SetText(szTemp, 0);
                    }
                }
            }

            // this && was added to prevent null dereferencing
            if (m_ucChaosLevel < 10 && m_pNameLabel)
            {
                m_pNameLabel->m_cBorder = 1;
                m_pNameLabel->SetTextColor(0xFF000000);
            }
            else
            {
                if (m_ucChaosLevel > 150)
                    m_ucChaosLevel = 150;
                else if (m_ucChaosLevel < 10)
                    m_ucChaosLevel = 5;

                int nIndex = (m_ucChaosLevel - 5) / 20;
                float fPos = (float)(m_ucChaosLevel - (20 * nIndex + 5)) / 20.0f;

                unsigned int dwR = (unsigned int)(((float)(((unsigned int)0xFF0000 & TMHuman::m_dwNameColor[nIndex]) >> 16) * (1.0f - fPos)) + 
                    ((float)(((unsigned int)0xFF0000 & TMHuman::m_dwNameColor[nIndex + 1]) >> 16) * fPos)) << 16;
                unsigned int dwG = (unsigned int)(((float)((TMHuman::m_dwNameColor[nIndex] & 0xFF00) >> 8) * (1.0f - fPos)) + 
                    ((float)((TMHuman::m_dwNameColor[nIndex + 1] & 0xFF00) >> 8) * fPos)) << 8;
                unsigned int dwB = (unsigned int)(((float)(TMHuman::m_dwNameColor[nIndex] & 0xFF) * (1.0f - fPos)) + 
                    ((float)(TMHuman::m_dwNameColor[nIndex + 1] & 0xFF) * fPos));

                m_pNameLabel->SetTextColor(dwB | dwG | dwR | 0xFF000000);

                // this && was added to prevent null dereferencing
                if (m_dwID > 0 && m_dwID < 1000 && m_pNameLabel)
                    m_pNameLabel->m_cBorder = 0;
            }

            m_pNameLabel->SetSize(strlen(m_szName) * 6.0f + 18.0f, 16.0f);

            if (m_pNameLabel && m_pNameLabel->m_cBorder)
                m_cSummons = 0;
        }
        else
        {
            char szMyMob[64] = { 0 };
            memcpy(szMyMob, m_szName, pDest - m_szName);

            m_cSummons = 1;
            m_pNameLabel->SetText(szMyMob, 0);

            if (m_pTitleNameLabel)
                m_pTitleNameLabel->SetText(szMyMob, 0);

            m_pNameLabel->SetSize(strlen(szMyMob) * 6.0f + 18.0f, 16.0f);
        }

        if (m_pSkinMesh)
        {
            m_pSkinMesh->m_vScale.x = m_fScale;
            m_pSkinMesh->m_vScale.y = m_fScale;
            m_pSkinMesh->m_vScale.z = m_fScale;
        }
    }
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
            (m_sHeadIndex != 271 || !(m_stScore.Reserved & 0xF)) && 
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
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Reserved & 0xF)
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
            (m_sHeadIndex != 271 || !(m_stScore.Reserved & 0xF)) &&
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
                        !m_stLookInfo.FaceMesh || m_sHeadIndex == 271 && m_stScore.Reserved & 0xF)
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

void TMHuman::DelayDelete()
{
    m_dwDelayDel = g_pTimerManager->GetServerTime();
    g_pObjectManager->DisconnectEffectFromMob(this);

    auto pScene = static_cast<TMFieldScene*>(m_pParentScene);
    if (pScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
    {
        m_sDelayDel = 1;

        if (pScene->m_pTargetHuman == this)
            pScene->m_pTargetHuman = nullptr;

        if (pScene->m_pMyHuman && pScene->m_pMyHuman->m_pMoveTargetHuman == this)
            pScene->m_pMyHuman->m_pMoveTargetHuman = nullptr;
        
        if (pScene->m_pMyHuman && pScene->m_pMyHuman->m_pMoveSkillTargetHuman == this)
            pScene->m_pMyHuman->m_pMoveSkillTargetHuman = nullptr;

        for (int j = 0; j < 13; ++j)
        {
            if (m_usTargetID[j])
            {
                auto pTarget = static_cast<TMHuman*>(g_pObjectManager->GetHumanByID(m_usTargetID[j]));
                if (pTarget)
                {
                    if (pTarget->m_wAttackerID == m_dwID)
                        pTarget->m_wAttackerID = 0;
                }
            }
        }

        if (pScene->m_pPGTOver == this)
            pScene->m_pPGTOver = nullptr;
        if (pScene->m_pMyHuman == this)
            pScene->m_pMyHuman = nullptr;
    }

    if (g_pCurrentScene->m_pMouseOverHuman == this)
        g_pCurrentScene->m_pMouseOverHuman = nullptr;

    if (static_cast<TMHuman*>(g_pObjectManager->m_pCamera->m_pFocusedObject) == this)
        g_pObjectManager->m_pCamera->m_pFocusedObject = nullptr;

    if (m_pSkinMesh)
    {
        m_pSkinMesh->m_pOwner = nullptr;

        if (m_pSkinMesh->m_pSwingEffect[0])
        {
            m_pSkinMesh->m_pSwingEffect[0]->m_pParentSkin = nullptr;

            if (m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant)
            {
                g_pObjectManager->DeleteObject(m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant);
                m_pSkinMesh->m_pSwingEffect[0]->m_pEnchant = nullptr;
            }
        }

        if (m_pSkinMesh->m_pSwingEffect[1])
        {
            m_pSkinMesh->m_pSwingEffect[1]->m_pParentSkin = nullptr;

            if (m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant)
            {
                g_pObjectManager->DeleteObject(m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant);
                m_pSkinMesh->m_pSwingEffect[1]->m_pEnchant = nullptr;
            }
        }
    }

    if (m_pLifeDrain)
    {
        g_pObjectManager->DeleteObject(m_pLifeDrain);
        m_pLifeDrain = nullptr;
    }
    if (m_pHuntersVision)
    {
        g_pObjectManager->DeleteObject(m_pHuntersVision);
        m_pHuntersVision = nullptr;
    }
    if (m_pOverExp)
    {
        g_pObjectManager->DeleteObject(m_pOverExp);
        m_pOverExp = nullptr;
    }
    if (m_pBraveOverExp)
    {
        g_pObjectManager->DeleteObject(m_pBraveOverExp);
        m_pBraveOverExp = nullptr;
    }
    if (m_pProtector)
    {
        g_pObjectManager->DeleteObject(m_pProtector);
        m_pProtector = nullptr;
    }
    if (m_pFamiliar)
    {
        g_pObjectManager->DeleteObject(m_pFamiliar);
        m_pFamiliar = nullptr;
    }
    if (m_pShade)
    {
        g_pObjectManager->DeleteObject(m_pShade);
        m_pShade = nullptr;
    }
    if (m_pAurora)
    {
        g_pObjectManager->DeleteObject(m_pAurora);
        m_pAurora = nullptr;
    }
    if (m_pSkillAmp)
    {
        g_pObjectManager->DeleteObject(m_pSkillAmp);
        m_pSkillAmp = nullptr;
    }
    if (m_pbomb)
    {
        g_pObjectManager->DeleteObject(m_pbomb);
        m_pbomb = nullptr;
    }
    if (m_pShadow)
    {
        g_pObjectManager->DeleteObject(m_pShadow);
        m_pShadow = nullptr;
    }
    if (m_pCriticalArmor)
    {
        g_pObjectManager->DeleteObject(m_pCriticalArmor);
        m_pCriticalArmor = nullptr;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (m_pSoul[i])
        {
            g_pObjectManager->DeleteObject(m_pSoul[i]);
            m_pSoul[i] = nullptr;
        }
    }
    for (int i = 0; i < 7; ++i)
    {
        if (m_pRotateBone[i])
        {
            g_pObjectManager->DeleteObject(m_pRotateBone[i]);
            m_pRotateBone[i] = nullptr;
        }
    }
    for (int i = 0; i < 7; ++i)
    {
        if (m_pEyeFire[i])
        {
            g_pObjectManager->DeleteObject(m_pEyeFire[i]);
            m_pEyeFire[i] = nullptr;
        }
        if (m_pEyeFire2[i])
        {
            g_pObjectManager->DeleteObject(m_pEyeFire2[i]);
            m_pEyeFire2[i] = nullptr;
        }
    }
    for (int i = 0; i < 4; ++i)
    {
        if (m_pFly[i])
        {
            g_pObjectManager->DeleteObject(m_pFly[i]);
            m_pFly[i] = nullptr;
        }
    }
    for (int i = 0; i < 5; ++i)
    {
        if (m_pImmunity[i])
        {
            g_pObjectManager->DeleteObject(m_pImmunity[i]);
            m_pImmunity[i] = nullptr;
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        if (m_pLightenStorm[i])
        {
            g_pObjectManager->DeleteObject(m_pLightenStorm[i]);
            m_pLightenStorm[i] = nullptr;
        }
    }
    if (m_pEleStream)
    {
        g_pObjectManager->DeleteObject(m_pEleStream);
        m_pEleStream = nullptr;
    }
    if (m_pEleStream2)
    {
        g_pObjectManager->DeleteObject(m_pEleStream2);
        m_pEleStream2 = nullptr;
    }
    if (m_pRescue)
    {
        g_pObjectManager->DeleteObject(m_pRescue);
        m_pRescue = nullptr;
    }
    if (m_pMagicShield)
    {
        g_pObjectManager->DeleteObject(m_pMagicShield);
        m_pMagicShield = nullptr;
    }
    if (m_pCancelation)
    {
        g_pObjectManager->DeleteObject(m_pCancelation);
        m_pCancelation = nullptr;
    }

    if (m_pChatMsg)
        m_pChatMsg->SetVisible(0);

    int result = 0;
    if (m_pNameLabel)
        m_pNameLabel->SetVisible(0);
    if (m_pKillLabel)
        m_pKillLabel->SetVisible(0);
    if (m_stGuildMark.pGuildMark)
        m_stGuildMark.pGuildMark->SetVisible(0);
    if (m_pAutoTradeDesc)
        m_pAutoTradeDesc->SetVisible(0);
    if (m_pAutoTradePanel)
        m_pAutoTradePanel->SetVisible(0);
    if (m_pNickNameLabel)
        m_pNickNameLabel->SetVisible(0);
    if (m_pProgressBar)
        m_pProgressBar->SetVisible(0);
    if (m_pMountHPBar)
        m_pMountHPBar->SetVisible(0);
    if (m_pInMiniMap)
        m_pInMiniMap->SetVisible(0);

    m_dwID = -1;
    m_bParty = 0;
}

void TMHuman::SetCharHeight(float fCon)
{
    float fRatio = 0.0f;
    if (m_dwID >= 0 && m_dwID < 1000 || g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR)
    {
        fRatio = 4000.0f;
        if (fCon > 500.0f && m_stScore.Level < 500)
            fCon = 500.0f;
    }
    else
    {
        fRatio = 2000.0;
    }

    m_fScale = ((fCon / fRatio) + 1.0f) * 0.89999998f;
}

float TMHuman::GetMyHeight()
{
    if (g_pCurrentScene == nullptr)
        return 0.0f;

    if (m_nClass == 45 && !m_stLookInfo.LeftMesh)
    {
        m_fWantHeight = 0.0;
        return 0.0f;
    }
    else if (m_nClass == 50 && g_pCurrentScene->m_pGround)
    {
        m_fWantHeight = (float)g_pCurrentScene->GetMask2(m_vecPosition) * 0.1f;
        return m_fWantHeight;
    }

    else if (m_nSkinMeshType == 40
        || m_nSkinMeshType == 24
        || m_nSkinMeshType == 20
        || m_nSkinMeshType == 39
        || m_nSkinMeshType == 8
        || m_cMount && (m_nMountSkinMeshType == 40
            || m_nMountSkinMeshType == 20
            || m_nMountSkinMeshType == 39
            || m_nMountSkinMeshType == 45
            || m_nMountSkinMeshType == 46
            || m_nMountSkinMeshType == 47))
    {
        if (fabsf(m_fHeight - m_fWantHeight) <= 0.1f)
            return m_fWantHeight;
        else
            return (float)(m_fWantHeight
                - (float)((float)(m_fWantHeight - m_fHeight) * 0.89999998f));
    }

    else if (m_nSkinMeshType == 5 && m_stLookInfo.FaceMesh != 1)
    {      
        if (fabsf(m_fHeight - m_fWantHeight) <= 0.1f)
            return m_fWantHeight;
        else
            return (float)(m_fWantHeight
                - (float)((float)(m_fWantHeight - m_fHeight) * 0.89999998f));
    }

    return m_fWantHeight;
}

void TMHuman::SetGuildBattleHPColor()
{
    // It's an empty function... yeah! 
}

void TMHuman::SetGuildBattleHPBar(int nHP)
{
    // It's an empty function... yeah! 
}

void TMHuman::SetGuildBattleLifeCount()
{
    // It's an empty function... yeah! 
}

int TMHuman::Is2stClass()
{
    int mantua = g_pObjectManager->m_stMobData.Equip[15].sIndex;
    
    if (!g_pObjectManager->m_stMobData.Equip[0].stEffect[1].cValue)
        return 0;

    if (g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x40000000)
        return 2;

    return 1;
}

int TMHuman::IAmkhepra()
{
    return m_nClass == 56 && !m_stLookInfo.FaceMesh;
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

int TMHuman::StrByteCheck(const char* szString)
{
    int value = 0;
    bool byteCheck = false;

    int len = strlen(szString);
    for (int i = 0;i < len ; ++i)
    {
        if (szString[i] >= 'A' && szString[i] <= 'z')
            ++value;
        else if (byteCheck)
        {
            ++value;
            byteCheck = false;
        }
        else
            byteCheck = true;
    }

    return value;
}

bool TMHuman::_locationCheck(TMVector2 vec2, int mapX, int mapY)
{
    return mapY == (int)(vec2.y * 0.0078125f) && mapX == (int)(vec2.x * 0.0078125f);
}

bool _locationCheck(float posx, float posy, int mapX, int mapY)
{
    return (int)(posx * 0.0078125f) == mapX && (int)(posy * 0.0078125f) == mapY;
}

bool _locationCheck(TMVector2 vec2, int mapX, int mapY)
{
    return (int)(vec2.x * 0.0078125f) == mapX && (int)(vec2.y * 0.0078125f) == mapY;
}
