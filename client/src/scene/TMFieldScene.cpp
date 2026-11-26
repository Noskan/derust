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

/// @file TMFieldScene.cpp
/// @brief Ciclo de vida principal, inicializacao, loop FrameMove, clima e automacao da cena de campo.

RECT TMFieldScene::m_rectWarning[7] =
{
  { 2255, 1535, 2263, 1538 },
  { 150, 3788, 156, 3793 },
  { 150, 3711, 156, 3772 },
  { 154, 3773, 157, 3780 },
  { 1075, 1708, 1078, 1713 },
  { 1171, 4076, 1177, 4079 },
  { 2362, 4041, 2369, 4044 }
};

int TMFieldScene::m_bPK = 0;
unsigned short TMFieldScene::m_usProgress = 0;
unsigned int TMFieldScene::m_dwCargoID = 0;

TMFieldScene::TMFieldScene()
	: TMScene()
{
	g_nTempArray[1] = (int)g_pObjectManager->m_stMobData.MobName;
	g_nTempArray2[1] = (int)g_pObjectManager->m_stMobData.MobName;

	LOG_WRITELOG(">> New Field Scene\r\n");
	m_eSceneType = ESCENE_TYPE::ESCENE_FIELD;
	m_dwID = static_cast<unsigned int>(m_eSceneType);
	m_bInitGate = 1;

	g_bLastStop = 0;
	g_bCastleWar = 0;
	g_bCastleWar2 = 0;

	m_dwLastGuildNameCheckTime = 0;
	m_dwLastWolfSound = 0;
	m_nMySanc = 0;
	m_cResurrect = 0;
	m_dwDustTerm = 2000;
	m_cLastFlagLButtonUp = 1;
	m_dwInTownTime = 0;
	m_dwFieldTime = 0;
	m_dwLastMouseDownTime = 0;
	m_dwLastPappusTime = 0;
	m_dwLastDustTime = 0;
	m_dwLastRemain = 0;
	m_dwLastLogout = 0;
	m_dwLastSelServer = 0;
	m_dwLastTeleport = 0;
	m_dwLastTown = 0;
	m_dwLastResurrect = 0;
	m_dwLastRelo = 0;
	m_dwStartFlashTime = 0;
	m_dwOldAttackTime = 0;
	m_dwWeatherTime = 0;
	m_dwLastDeadTime = 0;
	m_nWTime = 0;
	m_dwKeyTime = 0;
	m_dwPGTTime = 0;
	m_dwGetItemTime = 0;
	m_dwUseItemTime = 0;
	m_dwNPCClickTime = 0;
	m_dwLastCheckTime = 0;
	m_dwLastSetTargetHuman = 0;
	m_dwRemainTime = 0;
	m_dwQuizStart = 0;
	m_nChatListSize = 0;
	m_nReqHP = 0;
	m_nReqMP = 0;
	m_nAutoSkillNum = 4;
	m_nLastAutoTradePos = -1;
	m_sDestType = -1;
	m_sDestPos = -1;
	m_dwLastMousePosTime = 0;
	m_nLastMousePosX = 0;
	m_nLastMousePosY = 0;
	m_bSandWind = 1;
	m_cLastTown = 0;
	m_cLastTeleport = 0;
	m_cLastRelo = 0;
	m_cLastWhisper = 0;
	m_cGuildOnOff = 1;
	m_bMoveing = 0;
	m_nDustCount = 0;
	m_pSysMsgList = 0;
	m_dwLastNightmareTime = 0;
	m_dwNightmareTime = 0;

	g_GameAuto = 0;

	m_dwAttackDelay = 0;
	m_dwLastWhisper = 0;

	memset(&m_stLastWhisper, 0, sizeof(m_stLastWhisper));
	memset(m_dwStartAffectTime, 0, sizeof(m_dwStartAffectTime));
	memset(&m_stAutoTrade, 0, sizeof(m_stAutoTrade));
	memset(&m_stUseItem, 0, sizeof(m_stUseItem));
	memset(&m_stCapsuleItem, 0, sizeof(m_stCapsuleItem));
	memset(&m_stPotalItem, 0, sizeof(m_stPotalItem));
	memset(m_szSummoner, 0, sizeof(m_szSummoner));
	memset(m_szSummoner2, 0, sizeof(m_szSummoner2));
	memset(m_pLottoNumber, 0, sizeof(m_pLottoNumber));

	m_bNumPad = 0;
	m_bMountDead = 0;
	m_pTarget1 = nullptr;
	m_pTarget2 = nullptr;
	m_pTargetBill = nullptr;
	m_pHelpMemo = nullptr;
	m_pHelpSummon = nullptr;
	m_pMyHuman = nullptr;
	m_pTargetItem = nullptr;
	m_pTargetHuman = nullptr;
	m_pAutoSkillPanel = nullptr;
	m_pPGTOver = nullptr;
	m_pTradePanel = nullptr;
	m_pLottoPanel = nullptr;
	m_pLottoCost = nullptr;
	m_pHellgateStore = nullptr;
	m_pHellStoreDesc = nullptr;
	m_pGridHellStore = nullptr;
	m_pGambleStore = nullptr;
	m_pQuestPanel = nullptr;
	m_pQuestQuitBtn = nullptr;
	m_pQuestMemo = nullptr;
	m_pCargoGrid = nullptr;
	m_pGridHelm = nullptr;
	m_pGridCoat = nullptr;
	m_pGridPants = nullptr;
	m_pGridGloves = nullptr;
	m_pGridBoots = nullptr;
	m_pGridRight = nullptr;
	m_pGridLeft = nullptr;
	m_pGridGuild = nullptr;
	m_pGridEvent = nullptr;
	m_pGridRing = nullptr;
	m_pGridNecklace = nullptr;
	m_pGridOrb = nullptr;
	m_pGridCabuncle = nullptr;
	m_pGridDRing = nullptr;
	m_pGridMantua = nullptr;
	m_pEventPanel = nullptr;
	m_dwLastAutoAttackTime = 0;
	m_nCoinMsgType = 0;
	m_dwOpID = 0;
	m_dwChatTime = 0;
	m_fFlashTerm = 2000.0f;
	m_dwLastCheckAutoMouse = 0;
	m_bLastMyAttr = 0;
	m_nLastPotal = -1;
	m_dwTID = 0;
	m_bWarning = 0;
	m_bQuater = 1;
	m_cWarClan = -1;
	m_cAutoAttack = 0;
	g_pDevice->m_bFog = 1;

	TMFieldScene::m_bPK = 0;

	m_bShowNameLabel = 1;
	m_cWhisper = 0;
	m_cPartyChat = 0;
	m_cGuildChat = 0;
	m_bTab = 0;

	TMFieldScene::m_dwCargoID = 0;

	m_dwHellStoreID = 0;
	m_nHellStoreValue = 0;
	m_nFireWorkCellX = -1;
	m_nFireWorkCellY = -1;
	m_nTotoNum = 0;

	memset(&m_stToto, 0, sizeof(m_stToto));

	m_sDay = 0;

	memset(m_dwLastChatTime, 0, sizeof(m_dwLastChatTime));
	memset(m_pGridAutoTrade, 0, sizeof(m_pGridAutoTrade));
	memset(m_pGridItemMix, 0, sizeof(m_pGridItemMix));
	memset(m_pGridMixResult, 0, sizeof(m_pGridMixResult));

	m_pChatList = nullptr;
	m_pChatPanel = nullptr;
	m_pPositionText = nullptr;
	m_pFadePanel = nullptr;
	m_pEditChat = nullptr;
	m_pInputBG2 = nullptr;
	m_pInvenPanel = nullptr;
	m_pAutoTrade = nullptr;
	m_pItemMixPanel = nullptr;
	m_pItemMixPanel4 = nullptr;
	m_pSystemPanel = nullptr;
	m_pPGTPanel = nullptr;
	m_pInputGoldPanel = nullptr;
	m_pCPanel = nullptr;
	m_pSkillPanel = nullptr;
	m_pKingDomFlag = nullptr;
	m_pFlagDesc = nullptr;
	m_pFlagDescText[0] = nullptr;
	m_pFlagDescText[1] = nullptr;
	m_pFlagDescText[2] = nullptr;
	m_pRankTimeText = nullptr;
	m_pRemainText = nullptr;
	m_nLastTime = 0;
	m_dwStartRankTime = 0;
	m_pMsgPanel = nullptr;
	m_pMsgList = nullptr;
	m_pMsgText = nullptr;
	m_nVillage = -1;
	m_pQuizPanel = nullptr;
	m_pQuizCaption = nullptr;

	memset(m_pQuizText, 0, sizeof(m_pQuizText));
	m_pQuizBG = 0;
	m_pQuizQuestion = 0;
	memset(m_pQuizButton, 0, sizeof(m_pQuizButton));

	m_pQuestBtn = 0;
	m_pAutoRunBtn = 0;
	m_pHelpBtn = 0;
	m_pMGameAutoBtn = 0;
	m_pSGameAutoBtn = 0;
	m_pAutoTarget = 0;
	m_pCCPotionBtn = 0;
	m_pCCFeedBtn = 0;
	m_AutoStartPointX = 0;
	m_AutoStartPointY = 0;
	m_AutoPostionUse = 2;
	m_AutoHpMp = 1;
	m_pSetType = 0;
	m_pHelpPanel = 0;
	m_pHelpText = 0;

	memset(m_pHelpButton, 0, sizeof(m_pHelpButton));
	memset(m_pHelpList, 0, sizeof(m_pHelpList));

	m_pFireWorkPanel = 0;
	memset(m_pFireWorkButton, 0, sizeof(m_pFireWorkButton));
	m_pFireWorkOKButton = 0;
	m_pFireWorkQuitButton = 0;

	m_pTotoPanel = 0;
	m_pTotoSelect_Btn = 0;
	m_pTotoBuy_Btn = 0;
	m_pTotoQuit_Btn = 0;
	m_pTotoNumber_Edit = 0;
	m_pTotoTime_Txt = 0;
	m_pTotoTeamA_Txt = 0;
	m_pTotoTeamB_Txt = 0;
	m_pTotoScoreA_Edit = 0;
	m_pTotoScoreB_Edit = 0;

	memset(m_szAutoCaption, 0, sizeof(m_szAutoCaption));
	memset(m_szAutoClass, 0, sizeof(m_szAutoClass));
	memset(m_szAutoFolder, 0, sizeof(m_szAutoFolder));
	memset(m_szAutoFolder2, 0, sizeof(m_szAutoFolder2));
	memset(m_szAutoProcess, 0, sizeof(m_szAutoProcess));

	m_szAutoFolder[0] = 107;
	m_szAutoFolder[1] = 48;
	m_szAutoFolder[2] = 101;
	m_szAutoFolder[3] = 48;
	m_szAutoFolder[4] = 98;
	m_szAutoFolder[5] = 48;
	m_szAutoFolder[6] = 105;
	m_szAutoFolder[7] = 48;
	m_szAutoFolder[8] = 97;
	m_szAutoFolder[9] = 48;
	m_szAutoFolder[10] = 117;
	m_szAutoFolder[11] = 116;
	m_szAutoFolder[12] = 111;
	m_szAutoFolder[13] = 92;
	m_szAutoFolder[14] = 107;
	m_szAutoFolder[15] = 101;
	m_szAutoFolder[16] = 98;
	m_szAutoFolder[17] = 105;
	m_szAutoFolder[18] = 97;
	m_szAutoFolder[19] = 117;
	m_szAutoFolder[20] = 116;
	m_szAutoFolder[21] = 111;
	m_szAutoFolder[22] = 46;
	m_szAutoFolder[23] = 101;
	m_szAutoFolder[24] = 120;
	m_szAutoFolder[25] = 101;
	m_szAutoFolder2[0] = 65;
	m_szAutoFolder2[1] = 99;
	m_szAutoFolder2[2] = 116;
	m_szAutoFolder2[3] = 105;
	m_szAutoFolder2[4] = 118;
	m_szAutoFolder2[5] = 101;
	m_szAutoFolder2[6] = 74;
	m_szAutoFolder2[7] = 111;
	m_szAutoFolder2[8] = 121;
	m_szAutoFolder2[9] = 46;
	m_szAutoFolder2[10] = 111;
	m_szAutoFolder2[11] = 99;
	m_szAutoFolder2[12] = 120;

	memset(m_pQuestButton, 0, sizeof(m_pQuestButton));
	memset(m_pQuestList, 0, sizeof(m_pQuestList));
	memset(m_pQuestContentList, 0, sizeof(m_pQuestContentList));

	m_pPartyPanel = 0;
	m_pBtnPGTParty = 0;
	m_pBtnPGTGuild = 0;
	m_pBtnPGTTrade = 0;
	m_pBtnPGTChallenge = 0;
	m_pBtnPGT1_V_1 = 0;
	m_pBtnPGT5_V_5 = 0;
	m_pBtnPGT10_V_10 = 0;
	m_pBtnPGTAll_V_All = 0;
	m_pBtnPGTGuildDrop = 0;
	m_pBtnPGTGuildWar = 0;
	m_pBtnPGTGuildAlly = 0;
	m_pBtnPGTGuildInvite = 0;
	m_pBtnPGTGICommon = 0;
	m_pBtnPGTGIChief1 = 0;
	m_pBtnPGTGIChief2 = 0;
	m_pBtnPGTGIChief3 = 0;
	m_pHPBar = 0;
	m_pMPBar = 0;
	m_pMHPBar = 0;
	m_pMHPBarT = 0;
	m_pCurrentHPText = 0;
	m_pMaxHPText = 0;
	m_pCurrentMPText = 0;
	m_pMaxMPText = 0;
	m_pCurrentMHPText = 0;
	m_pMaxMHPText = 0;
	m_pInfoText = 0;

	memset(m_szLastAttackerName, 0, sizeof(m_szLastAttackerName));

	for (int i = 0; i < 12; ++i)
	{
		float fAngle = (((float)i * 3.1415927f) * 2.0f) / 12.0f;
		m_vecKnifePos[i] = TMVector2(cosf(fAngle) * 10.0f, sinf(fAngle) * 10.0f);
	}
	for (int i = 0; i < 24; ++i)
		m_pSkillSecGrid[i] = 0;
	for (int i = 0; i < 12; ++i)
		m_pSkillSecGrid2[i] = 0;

	m_pGridSkillBelt = 0;
	m_pGridSkillBelt2 = 0;
	m_pGridSkillBelt3 = 0;
	m_pShortSkillTglBtn1 = 0;
	m_pShortSkillTglBtn2 = 0;
	g_pGBPanel = 0;
	m_nCameraLoop = 0;

	TMFieldScene::m_usProgress = 0;

	m_nYear = 0;
	m_nDays = 0;
	m_nBet = 1000;
	m_dwEventTime = 0;
	m_dwLastClbuttonTime = 0;
	m_bClbutton = 0;
	m_dwKhepraID = 0;

	memset(&m_stRemoveServer, 0, sizeof(m_stRemoveServer));

	m_nServerMove = 0;
	m_pPotalPanel = 0;
	m_pPotalList = 0;
	m_pPotalText = 0;
	g_nTempArray[2] = (int)m_bShowNameLabel + 28;
	g_nTempArray2[2] = (int)m_bShowNameLabel + 28;
	g_pObjectManager->m_cSelectShortSkill = 0;

	m_pKhepraPortal = 0;
	m_pKhepraPortalEff1 = 0;
	m_pKhepraPortalEff2 = 0;
	m_dwKhepraDieTime = 0;
	m_nKhepraDieFlag = 0;
	m_nQuest12MaxMobs = 0;
	m_pQuestRemainTime = 0;
	m_nCameraMode = 0;
	m_bCantAttk = 0;
	m_dwCantMoveTime = 0;
	m_bTeleportMsg = 0;
	m_dwLastSecProcessTime = 0;
	m_nGuildMarkCount = 0;
	m_hInternetSession = 0;
	m_dwLastGetGuildmarkTime = 0;
	m_bIsDungeon = 0;
	m_nMouseRDownX = 0;
	m_nMouseRDownY = 0;
	m_dwDeleteURLTime = 0;
	m_bAirMove = 0;
	m_nOldMountSkinMeshType = -1;
	m_nAirMove_State = 0;
	m_dwAirMove_TickTime = 0;
	m_bAirMove_Wing = 0;
	m_vecAirMove_Dest.x = 0.0;
	m_vecAirMove_Dest.y = 0.0;
	m_fAirMove_Speed = 0.6f;
	m_bAirmove_ShowUI = 0;
	m_nAirMove_Index = 0;
	m_nAirMove_RouteIndex = 0;
	m_eOldMotion = ECHAR_MOTION::ECMOTION_NONE;
	m_pPotalText1 = 0;
	m_pPotalText2 = 0;
	m_pPotalText3 = 0;

	for (int i = 0; i < 32; ++i)
	{
		m_pAffectIcon[i] = 0;
		m_pTargetAffectIcon[i] = 0;
		m_dwAffectBlinkTime[i] = 0;
	}

	for (int i = 0; i < 13; ++i)
	{
		for (int j = 0; j < 32; ++j)
			m_pPartyAffectIcon[i][j] = 0;
	}

	m_pPartyAffectText = 0;
	m_pPartyAutoButton = 0;
	m_pPartyAutoText = 0;
	m_bAutoParty = 0;
	m_pDailyQuestButton = 0;
	m_pAffectDesc = 0;
	m_pChatGeneral_C = 0;
	m_pChatParty_C = 0;
	m_pChatWhisper_C = 0;
	m_pChatGuild_C = 0;
	m_pMiniBtn = 0;
	m_pMiniPanel = 0;
	m_pMainInfo2 = 0;
	m_pShortSkillPanel = 0;
	m_pMainInfo1 = 0;
	m_pMainInfo1_BG = 0;
	m_nIsMP = 0;
	m_bShowExp = 0;
	m_bTempCastlewar = 0;

	for (int i = 0; i < 4; ++i)
		m_bJPNBag[i] = 0;

	strcpy(m_cChatType, "");
	strcpy(m_cChatSelect, "");
	strcpy(m_cWhisperName, "");

	m_bShowBoss = 0;
	m_Coin = 0;
	m_bIsUndoShoplist = 0;
	m_sShopTarget = 0;
	m_bEventCouponClick = 0;
	m_bEventCouponOpen = 0;
}

TMFieldScene::~TMFieldScene()
{
	g_nTempArray[1] = 0;
	g_nTempArray[2] = 0;

	if (g_bActiveWB == 1)
	{
		g_pApp->RenderScene();
		g_pApp->SwitchWebBrowserState(0);
	}

	if (m_pHelpList[3])
	{
		memset(g_pObjectManager->m_stMemo, 0, sizeof(g_pObjectManager->m_stMemo));
		for (int i = 0; i < m_pHelpList[3]->m_nNumItem; ++i)
		{
			g_pObjectManager->m_stMemo[i].dwColor = m_pHelpList[3]->m_pItemList[i]->m_GCText.dwColor;
			sprintf(g_pObjectManager->m_stMemo[i].szString, "%s", m_pHelpList[3]->m_pItemList[i]->m_GCText.strString);
		}
	}

	g_pDevice->m_nWidthShift = 0;

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(107);
		if (pSoundData && pSoundData->IsSoundPlaying())
			pSoundData->Stop();

		pSoundData = pSoundManager->GetSoundData(6);
		if (pSoundData && pSoundData->IsSoundPlaying())
			pSoundData->Stop();
	}

	if (g_pCurrentScene
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_SELECT_SERVER
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD
		&& g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_DEMO
		&& g_pApp->m_pBGMManager)
	{
		g_pApp->m_pBGMManager->StopBGM();
	}

	if (m_pMyHuman)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!strcmp(m_pMyHuman->m_szName, g_pObjectManager->m_stSelCharData.MobName[j]))
			{
				g_pObjectManager->m_stSelCharData.Exp[j] = g_pObjectManager->m_stMobData.Exp;
				break;
			}
		}
	}
}

void SetMinimapPos()
{
	FILE* fpMinimap = nullptr;
	fopen_s(&fpMinimap, "minimap.dat", "rt");

	memset(g_MinimapPos, 0, sizeof(g_MinimapPos));

	if (fpMinimap)
	{
		int Color = 0; 
		for (int index = 0; index < 256; ++index)
		{
			if (fscanf(fpMinimap, "%d %d %d %d %d %s", &g_MinimapPos[index].nX, &g_MinimapPos[index].nY, &Color, &g_MinimapPos[index].nCX, &g_MinimapPos[index].nCY, g_MinimapPos[index].szTarget) == -1)
				break;

			switch (Color)
			{
			case 1:
				g_MinimapPos[index].dwColor = 0xFFFFFFFF;
				break;
			case 2:
				g_MinimapPos[index].dwColor = 0xFF44AA44;
				break;
			case 3:
				g_MinimapPos[index].dwColor = 0xFF5555FF;
				break;
			case 4:
				g_MinimapPos[index].dwColor = 0xFFAA00FF;
				break;
			}
		}

		fclose(fpMinimap);
	}
}

/// Inicializa controles de HUD, minimapa, efeitos de clima, camera e estado inicial da cena de campo.
int TMFieldScene::InitializeScene()
{
	LOG_WRITELOG(">> Init Field Scene::Start\r\n");
	LoadRC("UI\\FieldScene2.txt");
	g_pDevice->m_nHeightShift = 0;

	m_pQuizBG = new SPanel(-9, BASE_ScreenResize(275.0f) / 2.0f, 100.0f, 525.0f, 54.0f, 0x44FFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	if (m_pQuizBG)
	{
		m_pQuizBG->SetControlID(896);
		m_pQuizBG->m_bSelectEnable = 0;
		m_pControlContainer->AddItem(m_pQuizBG);

		float fScale = (float)g_pDevice->m_dwScreenHeight * 0.016f;
		m_pQuizBG->SetPos((float)g_pDevice->m_dwScreenWidth * 0.5f - m_pQuizBG->m_nWidth * 0.5f, fScale);
	}

	m_pQuizQuestion = new SText(-2, "Question", 0xFFFFFFFF, 5.0f, 104.0f, 510.0f, 16.0f, 0, 0x77777777, 1, 0);
	if (m_pQuizQuestion)
		m_pQuizBG->AddChild(m_pQuizQuestion);

	m_pQuizButton[0] = new SButton(-2, 5.0f, 128.0f, 125.0f, 20.0f, 0x77777777, 1, (char*)"Answer1");

	if (m_pQuizButton[0])
	{
		m_pQuizButton[0]->SetControlID(897);
		if (m_pControlContainer)
			m_pQuizButton[0]->SetEventListener(m_pControlContainer);
		else
			m_pQuizButton[0]->SetEventListener(nullptr);

		m_pQuizBG->AddChild(m_pQuizButton[0]);
	}

	m_pQuizButton[1] = new SButton(-2, 135.0f, 128.0f, 125.0f, 20.0f, 0x77777777, 1, (char*)"Answer2");
	if (m_pQuizButton[1])
	{
		m_pQuizButton[1]->SetControlID(898);
		if (m_pControlContainer)
			m_pQuizButton[1]->SetEventListener(m_pControlContainer);
		else
			m_pQuizButton[1]->SetEventListener(nullptr);

		m_pQuizBG->AddChild(m_pQuizButton[1]);
	}

	m_pQuizButton[2] = new SButton(-2, 265.0f, 128.0f, 125.0f, 20.0f, 0x77777777, 1, (char*)"Answer3");
	if (m_pQuizButton[2])
	{
		m_pQuizButton[2]->SetControlID(899);
		if (m_pControlContainer)
			m_pQuizButton[2]->SetEventListener(m_pControlContainer);
		else
			m_pQuizButton[2]->SetEventListener(nullptr);

		m_pQuizBG->AddChild(m_pQuizButton[2]);
	}

	m_pQuizButton[3] = new SButton(-2, 395.0f, 128.0f, 125.f, 20.0f, 0x77777777, 1, (char*)"Answer4");
	if (m_pQuizButton[3])
	{
		m_pQuizButton[3]->SetControlID(900);
		if (m_pControlContainer)
			m_pQuizButton[3]->SetEventListener(m_pControlContainer);
		else
			m_pQuizButton[3]->SetEventListener(nullptr);

		m_pQuizBG->AddChild(m_pQuizButton[3]);
	}

	if (m_pQuizBG)
		m_pQuizBG->SetVisible(0);

	m_pChatList = (SListBox*)m_pControlContainer->FindControl(65667);
	m_pChatListnotice = (SListBox*)m_pControlContainer->FindControl(65944);
	m_pChatBack = (SPanel*)m_pControlContainer->FindControl(65943);
	m_pChatGeneral = (SButton*)m_pControlContainer->FindControl(65677);
	m_pChatParty = (SButton*)m_pControlContainer->FindControl(65678);
	m_pChatWhisper = (SButton*)m_pControlContainer->FindControl(65679);
	m_pChatGuild = (SButton*)m_pControlContainer->FindControl(65680);
	m_pChatGeneral_C = (SButton*)m_pControlContainer->FindControl(65673);
	m_pChatParty_C = (SButton*)m_pControlContainer->FindControl(65674);
	m_pChatWhisper_C = (SButton*)m_pControlContainer->FindControl(65675);
	m_pChatGuild_C = (SButton*)m_pControlContainer->FindControl(65676);
	m_pKingdomText = (SText*)m_pControlContainer->FindControl(131072);
	m_pKingDomGuild = (SButton*)m_pControlContainer->FindControl(131074);
	m_pKingDomGuild_C = (SButton*)m_pControlContainer->FindControl(131073);
	m_pQuestBtn = (SButton*)m_pControlContainer->FindControl(65793);
	m_pHelpBtn = (SButton*)m_pControlContainer->FindControl(65795);
	m_pccmode = (SPanel*)m_pControlContainer->FindControl(66817);

	m_pccmode->SetVisible(0);

	m_pGridCharFace = (SPanel*)m_pControlContainer->FindControl(69636);
	m_pKingDomGuild_C->m_bSelected = 1;
	m_pKingDomGuild_C->SetVisible(0);
	m_pChatGeneral_C->m_bSelected = 0;
	m_pChatParty_C->m_bSelected = 0;
	m_pChatWhisper_C->m_bSelected = 0;
	m_pChatGuild_C->m_bSelected = 0;
	m_pChatGeneral->m_bSelected = 1;
	m_pChatParty->m_bSelected = 1;
	m_pChatWhisper->m_bSelected = 1;
	m_pChatGuild->m_bSelected = 1;
	m_pKingDomGuild->m_bSelected = 0;
	m_pChatList->m_pScrollBar->SetVisible(1);
	m_pChatListnotice->m_nPosX = 490.0f * RenderDevice::m_fWidthRatio;
	m_pChatListnotice->m_nPosX = 440.0f * RenderDevice::m_fHeightRatio;
	m_pChatListnotice->m_pScrollBar->SetVisible(0);

	m_pAutoRunBtn = (SButton*)m_pControlContainer->FindControl(316);
	m_pSysMsgList = (SListBox*)m_pControlContainer->FindControl(5695);
	m_PkButton = (SButton*)m_pControlContainer->FindControl(65786);

	m_PkButton->SetSelected(TMFieldScene::m_bPK == 0);
	m_pPGTText = (SText*)m_pControlContainer->FindControl(645);

	char szStr[128]{};
	sprintf(szStr, " %s", g_pMessageStringTable[157]);

	m_pAutoSkillPanel = (SPanel*)m_pControlContainer->FindControl(65648);
	m_pAutoSkillPanelChild[0] = (SPanel*)m_pControlContainer->FindControl(65775);
	m_pAutoSkillPanelChild[1] = (SPanel*)m_pControlContainer->FindControl(65776);
	m_pAutoSkillPanelChild[2] = (SPanel*)m_pControlContainer->FindControl(65777);
	m_pAutoSkillPanelChild[3] = (SPanel*)m_pControlContainer->FindControl(65778);
	m_pAutoSkillPanelChild[4] = (SPanel*)m_pControlContainer->FindControl(65779);
	m_pAutoSkillPanelChild[5] = (SPanel*)m_pControlContainer->FindControl(65780);
	m_pAutoSkillPanelChild[6] = (SPanel*)m_pControlContainer->FindControl(65781);
	m_pAutoSkillPanelChild[7] = (SPanel*)m_pControlContainer->FindControl(65782);
	m_pAutoSkillPanelChild[8] = (SPanel*)m_pControlContainer->FindControl(65783);
	m_pAutoSkillPanelChild[9] = (SPanel*)m_pControlContainer->FindControl(65784);

	if (m_pAutoSkillPanel)
		m_pAutoSkillPanel->SetVisible(0);

	SetAutoSkillNum(m_nAutoSkillNum);

	memset(&g_pObjectManager->m_stTrade, 0, sizeof(g_pObjectManager->m_stTrade));
	memset(&g_pObjectManager->m_stCombineItem, 0, sizeof(g_pObjectManager->m_stCombineItem));
	memset(&g_pObjectManager->m_stCombineItem4, 0, sizeof(g_pObjectManager->m_stCombineItem4));
	g_pObjectManager->m_stCombineItem.Header.ID = g_pObjectManager->m_dwCharID;
	g_pObjectManager->m_stCombineItem.Header.Type = 0x3A6;
	g_pObjectManager->m_stCombineItem4.Header.ID = g_pObjectManager->m_dwCharID;
	g_pObjectManager->m_stCombineItem4.Header.Type = 0x3C0;

	for (int i = 0; i < 15; ++i)
		g_pObjectManager->m_stTrade.CarryPos[i] = -1;
	for (int i = 0; i < 8; ++i)
	{
		g_pObjectManager->m_stCombineItem.CarryPos[i] = -1;
		g_pObjectManager->m_stCombineItem4.CarryPos[i] = -1;
	}

	m_pPositionText = (SText*)m_pControlContainer->FindControl(771);
	if (m_pPositionText)
		m_pPositionText->SetVisible(0);

	m_pFadePanel = (SPanel*)m_pControlContainer->FindControl(65566);
	if (m_pFadePanel)
		m_pFadePanel->m_bSelectEnable = 0;

	SPanel* pUnderBGPanel = (SPanel*)m_pControlContainer->FindControl(625);
	if (pUnderBGPanel)
		pUnderBGPanel->m_bSelectEnable = 0;

	m_pEditChat = (SEditableText*)m_pControlContainer->FindControl(65671);
	m_pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);

	if (m_pInputBG2)
		m_pInputBG2->SetVisible(0);
	if (m_pEditChat)
		m_pEditChat->SetVisible(1);

	m_pEditChat = (SEditableText*)m_pControlContainer->FindControl(65671);
	m_pInputBG2 = (SPanel*)m_pControlContainer->FindControl(574);

	if (m_pInputBG2)
		m_pInputBG2->SetVisible(0);
	if (m_pEditChat)
		m_pEditChat->SetVisible(1);

	memset(m_dwSkillLastTime, 0, sizeof(m_dwSkillLastTime));

	m_sWhisperIndex = 0;
	m_sChatIndex = 0;

	memset(m_szLastChatList, 0, sizeof(m_szLastChatList));
	memset(m_szWhisperList, 0, sizeof(m_szWhisperList));

	m_pFadePanel->SetSize((float)g_pDevice->m_dwScreenWidth, (float)g_pDevice->m_dwScreenHeight);

	m_pKingDomFlag = (SPanel*)m_pControlContainer->FindControl(65768);
	if (m_pKingDomFlag)
		m_pKingDomFlag->SetVisible(0);

	m_pFlagDesc = (SPanel*)m_pControlContainer->FindControl(65771);
	if (m_pFlagDesc)
		m_pFlagDesc->SetVisible(0);

	m_pFlagDescText[0] = (SText*)m_pControlContainer->FindControl(65772);
	m_pFlagDescText[1] = (SText*)m_pControlContainer->FindControl(65773);
	m_pFlagDescText[2] = (SText*)m_pControlContainer->FindControl(65774);

	if (m_pKingDomFlag)
		m_pKingDomFlag->m_pDescPanel = m_pFlagDesc;

	m_pRankTimeText = new SText(-2,
		"00 : 00",
		0xFF00FF00,
		(((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) / 2.0f) - 42.0f,
		30.0f,
		200.0f,
		16.0f,
		0,
		0x77777777,
		1,
		0);

	if (m_pRankTimeText)
	{
		m_pRankTimeText->m_Font.m_fSize = 2.0f;
		m_pRankTimeText->SetVisible(0);
		m_bRankTimeOn = 0;
	}

	char szTempLeft[128]{};
	sprintf(szTempLeft, "%s 100", g_pMessageStringTable[230]);

	m_pRemainText = new SText(-2,
		szTempLeft,
		0xFFFFAA00,
		((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 210.0f,
		30.0f,
		100.0f,
		16.0f,
		0,
		0x77777777,
		1,
		0);

	if (m_pRemainText)
	{
		m_pRemainText->m_Font.m_fSize = 2.0f;
		m_pRemainText->SetVisible(0);
	}

	m_pQuestRemainTime = new SText(-2,
		"",
		0xDDFFFF33,
		(float)((float)g_pDevice->m_dwScreenWidth / RenderDevice::m_fWidthRatio) - 140.0f,
		5.0f * RenderDevice::m_fHeightRatio,
		100.0f,
		16.0f,
		0,
		0x77777777,
		1,
		0);

	if (m_pQuestRemainTime)
	{
		m_pQuestRemainTime->m_Font.m_fSize = 1.0f;
		m_pQuestRemainTime->SetVisible(1);
	}

	if (m_pRankTimeText)
		m_pControlContainer->AddItem(m_pRankTimeText);
	if (m_pRemainText)
		m_pControlContainer->AddItem(m_pRemainText);
	if (m_pQuestRemainTime)
		m_pControlContainer->AddItem(m_pQuestRemainTime);

	m_pRankTimeText->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - 50.0f, 
		(3.0f * RenderDevice::m_fHeightRatio) + 55.0f);

	m_pRemainText->SetPos((float)g_pDevice->m_dwScreenWidth - 200.0f,
		30.0f * RenderDevice::m_fHeightRatio);

	if (g_pDevice->m_dwScreenWidth < 800)
	{
		m_pQuestRemainTime->SetPos((float)g_pDevice->m_dwScreenWidth + 35.0f,
			5.0f * RenderDevice::m_fHeightRatio);
	}
	else
	{
		m_pQuestRemainTime->SetPos(276.0f * RenderDevice::m_fWidthRatio,
			525.0f * RenderDevice::m_fHeightRatio);
	}

	m_pMoney1 = (SText*)m_pControlContainer->FindControl(65564u);
	m_pMoney4 = (SText*)m_pControlContainer->FindControl(94221u);
	m_pSkBonus = (SText*)m_pControlContainer->FindControl(65600u);
	m_pCHP = (SText*)m_pControlContainer->FindControl(65614u);
	m_pCMP = (SText*)m_pControlContainer->FindControl(65616u);
	m_pExpLamp[0] = (SPanel*)m_pControlContainer->FindControl(65637u);
	m_pExpLamp[1] = (SPanel*)m_pControlContainer->FindControl(65638u);
	m_pExpLamp[2] = (SPanel*)m_pControlContainer->FindControl(65639u);
	m_pExpProgress[0] = (SProgressBar*)m_pControlContainer->FindControl(65655u);
	m_pExpProgress[1] = (SProgressBar*)m_pControlContainer->FindControl(65656u);
	m_pExpProgress[2] = (SProgressBar*)m_pControlContainer->FindControl(65657u);
	m_pExpProgress[3] = (SProgressBar*)m_pControlContainer->FindControl(65658u);
	m_pExpProgress[4] = (SProgressBar*)m_pControlContainer->FindControl(65659u);
	m_pExpProgress[5] = (SProgressBar*)m_pControlContainer->FindControl(65660u);
	m_pExpProgress[6] = (SProgressBar*)m_pControlContainer->FindControl(65661u);
	m_pExpProgress[7] = (SProgressBar*)m_pControlContainer->FindControl(65662u);
	m_pExpProgress[8] = (SProgressBar*)m_pControlContainer->FindControl(65663u);
	m_pExpProgress[9] = (SProgressBar*)m_pControlContainer->FindControl(65664u);
	m_pCCModeHpSte = (SText*)m_pControlContainer->FindControl(66823u);
	m_pCCModeMountSte = (SText*)m_pControlContainer->FindControl(66824u);

	char chtmp[128]{};
	sprintf(chtmp, "%d", g_GameAuto_hpValue);
	m_pCCModeHpSte->SetText(chtmp, 0);
	sprintf(chtmp, "%d", g_GameAuto_mountValue);
	m_pCCModeMountSte->SetText(chtmp, 0);

	m_pCIName = (SText*)m_pControlContainer->FindControl(65703u);
	m_pCIClass = (SText*)m_pControlContainer->FindControl(65707u);
	m_pCIClass2 = (SText*)m_pControlContainer->FindControl(65708u);
	m_pCIHP = (SText*)m_pControlContainer->FindControl(65729u);
	m_pCIMP = (SText*)m_pControlContainer->FindControl(65731u);
	m_pCIEXP = (SText*)m_pControlContainer->FindControl(65710u);
	m_pCIEXPE = (SText*)m_pControlContainer->FindControl(65711u);
	m_pCLevel = (SText*)m_pControlContainer->FindControl(65713u);
	m_pScBonus = (SText*)m_pControlContainer->FindControl(65727u);
	m_pCIStr = (SText*)m_pControlContainer->FindControl(65715u);
	m_pCIInt = (SText*)m_pControlContainer->FindControl(65718u);
	m_pCIDex = (SText*)m_pControlContainer->FindControl(65721u);
	m_pCICon = (SText*)m_pControlContainer->FindControl(65724u);
	m_pCIFakeExp = (SText*)m_pControlContainer->FindControl(65767u);
	m_pCISpecial1 = (SText*)m_pControlContainer->FindControl(65753u);
	m_pCISpecial2 = (SText*)m_pControlContainer->FindControl(65756u);
	m_pCISpecial3 = (SText*)m_pControlContainer->FindControl(65759u);
	m_pCISpecial4 = (SText*)m_pControlContainer->FindControl(65762u);
	m_pSpBonus = (SText*)m_pControlContainer->FindControl(65765u);
	m_pMoney2 = (SText*)m_pControlContainer->FindControl(1041u);
	m_pMoney3 = (SText*)m_pControlContainer->FindControl(6408u);

	for (int i = 0; i < 24; ++i)
		m_pSkillSecGrid[i] = (SGridControl*)m_pControlContainer->FindControl(i + 65574);

	for (int i = 0; i < 12; ++i)
		m_pSkillSecGrid2[i] = (SGridControl*)m_pControlContainer->FindControl(i + 94208);

	m_pDamage = (SText*)m_pControlContainer->FindControl(65733u);
	m_pSkillDam = (SText*)m_pControlContainer->FindControl(65735u);
	m_pSpeed = (SText*)m_pControlContainer->FindControl(65737u);
	m_pDefence = (SText*)m_pControlContainer->FindControl(65739u);
	m_pAttackSpeed = (SText*)m_pControlContainer->FindControl(65741u);
	m_pCritical = (SText*)m_pControlContainer->FindControl(65743u);
	m_pRegist1 = (SText*)m_pControlContainer->FindControl(65745u);
	m_pRegist2 = (SText*)m_pControlContainer->FindControl(65747u);
	m_pRegist3 = (SText*)m_pControlContainer->FindControl(65749u);
	m_pRegist4 = (SText*)m_pControlContainer->FindControl(65751u);
	m_pSLPanel1 = (SPanel*)m_pControlContainer->FindControl(1923u);
	m_pSLPanel2 = (SPanel*)m_pControlContainer->FindControl(1924u);
	m_pSLPanel3 = (SPanel*)m_pControlContainer->FindControl(1925u);
	m_pSkillCover = (SPanel*)m_pControlContainer->FindControl(1157u);
	m_pMainInfo1 = (SPanel*)m_pControlContainer->FindControl(65628u);
	m_pMainInfo1_BG = (SPanel*)m_pControlContainer->FindControl(65641u);

	m_pMainInfo1_BG->SetPos(((float)g_pDevice->m_dwScreenWidth - m_pMainInfo1_BG->m_nWidth) / 2.0f,
		m_pMainInfo1_BG->m_nPosX);

	m_pMainInfo1->SetStickLeft();
	m_pMainInfo1->SetStickBottom();

	SPanel* pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672);
	m_pChatPanel = (SPanel*)m_pControlContainer->FindControl(65672);
	m_pChatList->m_nPosX = BASE_ScreenResize(10.0f);
	m_pChatList->m_nPosY = (pChatPanel->m_nPosY - m_pChatList->m_nHeight) - BASE_ScreenResize(10.0f);

	m_pChatBack->m_nPosX = BASE_ScreenResize(10.0f);
	m_pChatBack->m_nPosY = m_pChatList->m_nPosY - BASE_ScreenResize(10.0f);

	m_pChatBack->SetSize(BASE_ScreenResize(10.0f) + m_pChatList->m_nWidth,
		BASE_ScreenResize(20.0f) + m_pChatList->m_nHeight);
	m_pChatList->SetSize(m_pChatList->m_nWidth,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	m_pChatBack->SetSize(BASE_ScreenResize(10.0f) + m_pChatList->m_nWidth,
		m_pChatList->m_nHeight);
	m_pChatList->m_pScrollBar->SetSize(m_pChatList->m_pScrollBar->m_nWidth,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	m_pChatList->m_pScrollBar->m_pBackground1->SetSize(m_pChatList->m_pScrollBar->m_nWidth,
		(float)(140 * m_nChatListSize + 60) * RenderDevice::m_fHeightRatio);
	m_pChatList->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - m_pChatList->m_nHeight) - 4.0f);
	m_pChatBack->SetPos(m_pChatList->m_nPosX, (float)(pChatPanel->m_nPosY - m_pChatList->m_nHeight) - 4.0f);

	m_pChatSelectPanel = (SPanel*)m_pControlContainer->FindControl(90113);
	m_pChatListPanel = (SPanel*)m_pControlContainer->FindControl(90128);
	m_pChatType = (SButton*)m_pControlContainer->FindControl(90114);

	SButton* Button = (SButton*)m_pControlContainer->FindControl(90129);
	m_pChatType->SetText(Button->m_GCPanel.strString);
	m_pChatListPanel->SetVisible(0);
	m_pChatSelectPanel->SetVisible(0);
	m_pChatList->m_pScrollBar->m_pBackground1->SetVisible(0);
	m_pChatListnotice->SetVisible(1);
	m_pMainInfo2 = (SPanel*)m_pControlContainer->FindControl(65610);

	if (m_pMainInfo2)
	{
		m_pMainInfo2->SetAutoSize();
		m_pMainInfo2->SetStickLeft();
		m_pMainInfo2->SetStickTop();
		m_pMainInfo2_Name = (SText*)m_pControlContainer->FindControl(65611);
		m_pMainInfo2->SetVisible(0);
	}

	m_pMainInfo2_Lv = (SText*)m_pControlContainer->FindControl(65613);
	m_pEditChatPanel = (SPanel*)m_pControlContainer->FindControl(65670);

	if (m_pEditChatPanel)
	{
		m_pEditChatPanel->m_nPosY = m_pControlContainer->FindControl(65672)->m_nPosY - (float)(2.0f * RenderDevice::m_fHeightRatio);
		m_pEditChatPanel->m_nPosX = m_pChatSelectPanel->m_nPosX	+ m_pChatSelectPanel->m_nWidth;

		if (g_nKeyType != 1)
			g_nKeyType = 0;
		if (!g_nKeyType)
			m_pEditChatPanel->SetVisible(0);

		m_pChatPanel->SetVisible(1);
	}

	m_pMiniBtn = (SButton*)m_pControlContainer->FindControl(65787);
	m_pMiniPanel = (SPanel*)m_pControlContainer->FindControl(65788);

	if (m_pMiniPanel)
		m_pMiniPanel->SetVisible(0);

	m_pShortSkillPanel = (SPanel*)m_pControlContainer->FindControl(65642);
	m_pShortSkill_Txt = (SText*)m_pControlContainer->FindControl(65643);
	m_pCISp1Caption = (SText*)m_pControlContainer->FindControl(65755);
	m_pCISp2Caption = (SText*)m_pControlContainer->FindControl(65758);
	m_pCISp3Caption = (SText*)m_pControlContainer->FindControl(65761);
	m_pCISp4Caption = (SText*)m_pControlContainer->FindControl(65764);
	m_pCIGuild = (SText*)m_pControlContainer->FindControl(65705);
	m_pSkillSec1 = (SText*)m_pControlContainer->FindControl(65570);
	m_pSkillSec2 = (SText*)m_pControlContainer->FindControl(65571);
	m_pSkillSec3 = (SText*)m_pControlContainer->FindControl(65572);
	m_pSkillMSec1 = (SText*)m_pControlContainer->FindControl(65604);
	m_pSkillMSec2 = (SText*)m_pControlContainer->FindControl(65605);
	m_pSkillMSec3 = (SText*)m_pControlContainer->FindControl(65606);

	for (int id = 0; id < 32; ++id)
	{
		m_pAffectL[id] = (SText*)m_pControlContainer->FindControl(id + 1328);
		m_pAffect[id] = (SText*)m_pControlContainer->FindControl(id + 90432);
	}

	m_pCargoCoin = (SText*)m_pControlContainer->FindControl(65689);
	m_pMyCargoCoin = (SText*)m_pControlContainer->FindControl(812);

	SText* pMyGold = (SText*)m_pControlContainer->FindControl(619);
	SText* pOPGold = (SText*)m_pControlContainer->FindControl(603);
	pMyGold->m_cComma = 2;
	pOPGold->m_cComma = 2;
	pMyGold->SetText((char*)"         0", 0);
	pOPGold->SetText((char*)"         0", 0);
	m_pInvenPanel = (SPanel*)m_pControlContainer->FindControl(589832);

	SPanel* pPanel1 = (SPanel*)m_pControlContainer->FindControl(65565);

	m_pInvenPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pInvenPanel)
	{
		m_pInvenPanel->SetVisible(0);
		pPanel1->m_bSelectEnable = 0;
	}

	m_pAutoTrade = (SPanel*)m_pControlContainer->FindControl(646);

	if (m_pAutoTrade)
	{
		m_pAutoTrade->SetVisible(0);
		for (int k = 0; k < 10; ++k)
		{
			m_pGridAutoTrade[k] = (SGridControl*)m_pControlContainer->FindControl(k + 653);
			if (m_pGridAutoTrade[k])
				m_pGridAutoTrade[k]->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
		}

		m_pAutoTrade->SetPos(RenderDevice::m_fWidthRatio * 254.0f,
			RenderDevice::m_fHeightRatio * 35.0f);
	}

	m_pItemMixPanel4 = (SPanel*)m_pControlContainer->FindControl(6432);
	m_pMix4Desc = (SListBox*)m_pControlContainer->FindControl(6439);
	m_pItemMixPanel = (SPanel*)m_pControlContainer->FindControl(65857);

	m_ItemMixClass.Read_MixListFile();
	m_ItemMixClass.TakeItResource(m_pControlContainer, g_pObjectManager->m_dwCharID);
	m_MissionClass.TakeItResource(m_pControlContainer, g_pObjectManager->m_dwCharID);

	if (m_pItemMixPanel4)
		m_pItemMixPanel4->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

	if(m_pItemMixPanel)
		m_pItemMixPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
			RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pMix4Desc)
		LoadMsgText(m_pMix4Desc, (char*)"UI\\mix4desc.txt");

	if (m_pItemMixPanel)
	{
		m_pItemMixPanel->SetVisible(0);
		for (int l = 0; l < 8; ++l)
		{
			m_pGridItemMix[l] = (SGridControl*)m_pControlContainer->FindControl(l + 65861);
			if (m_pGridItemMix[l])
				m_pGridItemMix[l]->m_eGridType = TMEGRIDTYPE::GRID_ITEMMIX;
		}
	}

	if (m_pItemMixPanel4)
	{
		m_pItemMixPanel4->SetVisible(0);
		for (int m = 0; m < 3; ++m)
		{
			m_pGridItemMix4[m] = (SGridControl*)m_pControlContainer->FindControl(m + 6436);
			if (m_pGridItemMix4[m])
				m_pGridItemMix4[m]->m_eGridType = TMEGRIDTYPE::GRID_ITEMMIX4;
		}
	}

	m_pHellgateStore = (SPanel*)m_pControlContainer->FindControl(6185);
	m_pHellStoreDesc = (SListBox*)m_pControlContainer->FindControl(6201);
	m_pGridHellStore = (SGridControl*)m_pControlContainer->FindControl(6208);
	m_pGridHellStore->m_bDrawGrid = 0;
	m_pGridHellStore->m_eGridType = TMEGRIDTYPE::GRID_SHOP;

	if (m_pHellgateStore)
		m_pHellgateStore->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pHellgateStore->m_nWidth * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pHellgateStore->m_nHeight * 0.5f));

	if (m_pHellgateStore)
		m_pHellgateStore->SetVisible(0);

	m_pGambleStore = (SPanel*)m_pControlContainer->FindControl(6400);
	if (m_pGambleStore)
		m_pGambleStore->SetVisible(0);

	m_pDescPanel = m_pControlContainer->FindControl(258);
	m_pDescPanel->m_bSelectEnable = 0;
	m_pDescPanel->SetVisible(0);
	m_pDescNameText = (SText*)m_pControlContainer->FindControl(772);
	m_pParamText[0] = (SText*)m_pControlContainer->FindControl(773);
	m_pParamText[1] = (SText*)m_pControlContainer->FindControl(774);
	m_pParamText[2] = (SText*)m_pControlContainer->FindControl(775);
	m_pParamText[3] = (SText*)m_pControlContainer->FindControl(776);
	m_pParamText[4] = (SText*)m_pControlContainer->FindControl(777);
	m_pParamText[5] = (SText*)m_pControlContainer->FindControl(784);
	m_pParamText[6] = (SText*)m_pControlContainer->FindControl(794);
	m_pParamText[7] = (SText*)m_pControlContainer->FindControl(795);
	m_pParamText[8] = (SText*)m_pControlContainer->FindControl(796);
	m_pParamText[9] = (SText*)m_pControlContainer->FindControl(797);
	m_pParamText[10] = (SText*)m_pControlContainer->FindControl(798);
	m_pParamText[11] = (SText*)m_pControlContainer->FindControl(799);
	m_pParamText[12] = (SText*)m_pControlContainer->FindControl(800);
	m_pParamText[13] = (SText*)m_pControlContainer->FindControl(801);
	m_pSystemPanel = (SPanel*)m_pControlContainer->FindControl(65879);

	if (m_pSystemPanel)
	{
		m_pSystemPanel->SetVisible(0);
		m_pSystemPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[1] = m_pSystemPanel;
	}
	m_pPGTPanel = (SPanel*)m_pControlContainer->FindControl(640);
	if (m_pPGTPanel)
	{
		m_pPGTPanel->SetVisible(0);
		m_pPGTPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[2] = m_pPGTPanel;
	}
	m_pInputGoldPanel = (SPanel*)m_pControlContainer->FindControl(65885);
	if (m_pInputGoldPanel)
	{
		m_pInputGoldPanel->SetVisible(0);
		m_pInputGoldPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[3] = m_pInputGoldPanel;
	}

	m_pBtnGuildOnOff = (SButton*)m_pControlContainer->FindControl(299);
	m_pBtnMountRun = (SButton*)m_pControlContainer->FindControl(298);
	m_pBtnPGTParty = (SButton*)m_pControlContainer->FindControl(641);
	m_pBtnPGTGuild = (SButton*)m_pControlContainer->FindControl(642);
	m_pBtnPGTTrade = (SButton*)m_pControlContainer->FindControl(643);
	m_pBtnPGTChallenge = (SButton*)m_pControlContainer->FindControl(620);
	m_pBtnPGT1_V_1 = (SButton*)m_pControlContainer->FindControl(639);
	m_pBtnPGT5_V_5 = (SButton*)m_pControlContainer->FindControl(621);
	m_pBtnPGT10_V_10 = (SButton*)m_pControlContainer->FindControl(622);
	m_pBtnPGTAll_V_All = (SButton*)m_pControlContainer->FindControl(623);
	m_pBtnPGTGuildDrop = (SButton*)m_pControlContainer->FindControl(816);
	m_pBtnPGTGuildWar = (SButton*)m_pControlContainer->FindControl(817);
	m_pBtnPGTGuildAlly = (SButton*)m_pControlContainer->FindControl(862);
	m_pBtnPGTGuildInvite = (SButton*)m_pControlContainer->FindControl(863);
	m_pBtnPGTGICommon = (SButton*)m_pControlContainer->FindControl(912);
	m_pBtnPGTGIChief1 = (SButton*)m_pControlContainer->FindControl(913);
	m_pBtnPGTGIChief2 = (SButton*)m_pControlContainer->FindControl(914);
	m_pBtnPGTGIChief3 = (SButton*)m_pControlContainer->FindControl(915);
	SControl* pServer = m_pControlContainer->FindControl(65880);
	SControl* pChar = m_pControlContainer->FindControl(65881);
	SControl* pQuit = m_pControlContainer->FindControl(65882);
	SControl* pCancel = m_pControlContainer->FindControl(5883);
	m_pCPanel = (SPanel*)m_pControlContainer->FindControl(65696);

	SPanel* pCPanel1 = (SPanel*)m_pControlContainer->FindControl(65770);
	m_pCPanel->SetPos(RenderDevice::m_fWidthRatio * 60.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	m_pSystemPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pSystemPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pSystemPanel->m_nHeight * 0.5f));

	m_pPGTPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pPGTPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pPGTPanel->m_nHeight * 0.5f));

	m_pInputGoldPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pInputGoldPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pInputGoldPanel->m_nHeight * 0.5f));

	if (m_pCPanel)
	{
		m_pCPanel->SetVisible(0);
		pCPanel1->m_bSelectEnable = 0;
	}

	m_pShopPanel = (SPanel*)m_pControlContainer->FindControl(65692);

	SPanel *pShopPanel1 = (SPanel*)m_pControlContainer->FindControl(65695);

	m_pShopPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pShopPanel)
	{
		m_pShopPanel->SetVisible(0);
		pShopPanel1->m_bSelectEnable = 0;
	}
	m_pCargoPanel = (SPanel*)m_pControlContainer->FindControl(65686);
	m_pCargoPanel1 = (SPanel*)m_pControlContainer->FindControl(65692);

	SPanel* pCargoPanel1 = (SPanel*)m_pControlContainer->FindControl(65691);
	m_pCargoPanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
		RenderDevice::m_fHeightRatio * 35.0f);
	m_pCargoPanel1->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pCargoPanel)
	{
		m_pCargoPanel->SetVisible(0);
		m_pCargoPanel1->SetVisible(0);
		pCargoPanel1->m_bSelectEnable = 0;
	}

	SPanel* pTradePanel = (SPanel*)m_pControlContainer->FindControl(576);
	SPanel* pTradePanel1 = (SPanel*)m_pControlContainer->FindControl(577);

	pTradePanel->SetPos(RenderDevice::m_fWidthRatio * 287.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	m_pTradePanel = pTradePanel;

	if (pTradePanel)
	{
		pTradePanel->SetVisible(0);
		pTradePanel1->m_bSelectEnable = 0;
	}

	m_pLottoPanel = (SPanel*)m_pControlContainer->FindControl(2048);

	if (m_pLottoPanel)
	{
		m_pLottoPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - ((m_pLottoPanel->m_nWidth + 40.0f) * 0.5f),
			((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pLottoPanel->m_nHeight * 0.5f));

		m_pLottoCost = (SText*)m_pControlContainer->FindControl(2051);
		for (int nLotto = 0; nLotto < 6; ++nLotto)
			m_pLottoNumber[nLotto] = (SText*)m_pControlContainer->FindControl(nLotto + 2053);

		m_pLottoPanel->m_bPickable = 1;
		m_pLottoPanel->m_bSelectEnable = 1;
		m_pLottoPanel->SetVisible(0);
	}

	m_pSkillMPanel = (SPanel*)m_pControlContainer->FindControl(65602);
	SPanel* pSkillMPanel1 = (SPanel*)m_pControlContainer->FindControl(65609);
	m_pSkillMDesc = (SListBox*)m_pControlContainer->FindControl(65608);

	m_pSkillMPanel->SetPos(RenderDevice::m_fWidthRatio * 60.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	if (m_pSkillMPanel)
	{
		m_pSkillMPanel->SetVisible(0);
		pSkillMPanel1->m_bSelectEnable = 0;
	}

	m_pSkillPanel = (SPanel*)m_pControlContainer->FindControl(65567);

	SPanel* pSkillPanel1 = (SPanel*)m_pControlContainer->FindControl(65601);
	SText* pSkillBonus = (SText*)m_pControlContainer->FindControl(65600);

	m_pSkillPanel->SetPos(RenderDevice::m_fWidthRatio * 514.0f,
		RenderDevice::m_fHeightRatio * 35.0f);

	SPanel* pSkillTitlePanel = (SPanel*)m_pControlContainer->FindControl(1912);
	SPanel* pSkillGridPanel = (SPanel*)m_pControlContainer->FindControl(1910);
	SPanel* pSkillGridPanel2 = (SPanel*)m_pControlContainer->FindControl(1911);
	if (m_pSkillPanel)
	{
		m_pSkillPanel->SetVisible(0);
		if (pSkillTitlePanel)
			pSkillTitlePanel->m_bSelectEnable = 0;
		if (pSkillGridPanel)
			pSkillGridPanel->m_bSelectEnable = 0;
		if (pSkillGridPanel2)
			pSkillGridPanel2->m_bSelectEnable = 0;
		pSkillBonus->m_bSelectEnable = 0;
		pSkillPanel1->m_bSelectEnable = 0;
	}

	m_pInvPageBtn1 = (SButton*)m_pControlContainer->FindControl(67076);
	m_pInvPageBtn2 = (SButton*)m_pControlContainer->FindControl(67077);
	m_pInvPageBtn3 = (SButton*)m_pControlContainer->FindControl(67078);
	m_pInvPageBtn4 = (SButton*)m_pControlContainer->FindControl(67079);
	m_pJPNBag_Day1 = (SText*)m_pControlContainer->FindControl(77834);
	m_pJPNBag_Day2 = (SText*)m_pControlContainer->FindControl(77835);
	m_pStorePageBtn1 = (SButton*)m_pControlContainer->FindControl(67332);
	m_pStorePageBtn2 = (SButton*)m_pControlContainer->FindControl(67333);
	m_pStorePageBtn3 = (SButton*)m_pControlContainer->FindControl(67334);
	m_pMGameAutoBtn = (SButton*)m_pControlContainer->FindControl(66818);
	m_pSetType = (SButton*)m_pControlContainer->FindControl(66821);
	m_pHPBar = (SProgressBar*)m_pControlContainer->FindControl(65621);
	m_pMPBar = (SProgressBar*)m_pControlContainer->FindControl(65623);
	m_pMHPBar = (SProgressBar*)m_pControlContainer->FindControl(65625);
	m_pMHPBarT = (SProgressBar*)m_pControlContainer->FindControl(65626);
	m_pMainCharName = (SText*)m_pControlContainer->FindControl(69635);
	m_pCurrentHPText = (SText*)m_pControlContainer->FindControl(65614);
	m_pMaxHPText = (SText*)m_pControlContainer->FindControl(65615);
	m_pCurrentMPText = (SText*)m_pControlContainer->FindControl(65616);
	m_pMaxMPText = (SText*)m_pControlContainer->FindControl(65617);
	m_pCurrentMHPText = (SText*)m_pControlContainer->FindControl(65618);
	m_pMaxMHPText = (SText*)m_pControlContainer->FindControl(65619);
	m_pExpHold = (SPanel*)m_pControlContainer->FindControl(65636);
	m_pInfoText = (SText*)m_pControlContainer->FindControl(770);

	if (m_pExpHold)
		m_pExpHold->SetVisible(0);
	m_pQuizPanel = (SPanel*)m_pControlContainer->FindControl(880);
	m_pQuizText[0] = (SText*)m_pControlContainer->FindControl(881);
	m_pQuizText[1] = (SText*)m_pControlContainer->FindControl(882);
	m_pQuizText[2] = (SText*)m_pControlContainer->FindControl(883);
	m_pQuizText[3] = (SText*)m_pControlContainer->FindControl(884);

	if (m_pQuizPanel)
		m_pQuizPanel->m_bSelectEnable = 0;
	if (m_pQuizPanel)
		m_pQuizPanel->SetVisible(0);

	m_pQuizCaption = (SText*)m_pControlContainer->FindControl(6108);

	m_pQuizPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pQuizPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (200.0f * RenderDevice::m_fHeightRatio));

	m_pEventText = (SText*)m_pControlContainer->FindControl(65669);
	m_pEventPanel = (SPanel*)m_pControlContainer->FindControl(65668);

	m_pEventText->SetPos(300.0f, 300.0f);

	m_dwEventStartTime = g_pTimerManager->GetServerTime();
	m_nCurrEventTextIndex = 0;

	memset(m_szEventTextTemp, 0, sizeof(m_szEventTextTemp));

	STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
	int nTownX = (int)g_pObjectManager->m_stMobData.HomeTownX >> 7;
	int nTownY = (int)g_pObjectManager->m_stMobData.HomeTownY >> 7;
	char szMapPath[128]{};
	char szDataPath[128]{};
	sprintf(szMapPath, "env\\Field%02d%02d.trn", nTownX, nTownY);
	sprintf(szDataPath, "env\\Field%02d%02d.dat", nTownX, nTownY);

	m_pGroundList[0] = new TMGround();
	if (!m_pGroundList[0]->LoadTileMap(szMapPath))
	{
		if (!m_bCriticalError)
			LogMsgCriticalError(2, 0, 0, 0, 0);

		m_bCriticalError = 1;
	}

	m_pGround = m_pGroundList[0];
	m_pGround->SetMiniMapData();

	m_pObjectContainerList[0] = new TMObjectContainer(m_pGround);

	if (m_pObjectContainerList[0])
		m_pGroundObjectContainer->AddChild(m_pObjectContainerList[0]);

	SetMinimapPos();

	m_pMyHuman = new TMHuman(this);
	if (m_pMyHuman)
		m_pMyHuman->m_dwID = g_pObjectManager->m_dwCharID;

	m_pMyHuman->m_nTotalKill = ((unsigned char)pMobData->MobName[15] << 8) + (unsigned char)pMobData->MobName[14];
	m_pMyHuman->m_nCurrentKill = m_pMyHuman->m_nTotalKill;
	m_pMyHuman->m_ucChaosLevel = pMobData->MobName[12];
	pMobData->MobName[12] = 0;
	pMobData->MobName[15] = 0;

	sprintf(m_pMyHuman->m_szName, "%s", pMobData->MobName);

	m_pMyHuman->SetPacketMOBItem(pMobData);
	m_pMyHuman->SetSpeed(m_bMountDead);
	if (m_pMyHuman->m_cMount == 1 && 
		(m_pMyHuman->m_nMountSkinMeshType == 31 || m_pMyHuman->m_nMountSkinMeshType == 40 || m_pMyHuman->m_nMountSkinMeshType == 20 && 
		 m_pMyHuman->m_stMountLook.Mesh0 != 7 || m_pMyHuman->m_nMountSkinMeshType == 39))
	{
		m_pMyHuman->m_fMaxSpeed = 3.0f;
	}

	unsigned short usGuild = g_pObjectManager->m_stSelCharData.Guild[g_pObjectManager->m_cCharacterSlot];
	m_pMyHuman->m_sGuildLevel = (unsigned char)pMobData->GuildLevel;
	m_pMyHuman->m_usGuild = usGuild;

	SGridControl* pGridTradeOp1 = (SGridControl*)m_pControlContainer->FindControl(8192);
	SGridControl* pGridTradeOp2 = (SGridControl*)m_pControlContainer->FindControl(8193);
	SGridControl* pGridTradeOp3 = (SGridControl*)m_pControlContainer->FindControl(8194);
	SGridControl* pGridTradeOp4 = (SGridControl*)m_pControlContainer->FindControl(8195);
	SGridControl* pGridTradeOp5 = (SGridControl*)m_pControlContainer->FindControl(8196);
	SGridControl* pGridTradeOp6 = (SGridControl*)m_pControlContainer->FindControl(8197);
	SGridControl* pGridTradeOp7 = (SGridControl*)m_pControlContainer->FindControl(8198);
	SGridControl* pGridTradeOp8 = (SGridControl*)m_pControlContainer->FindControl(8199);
	SGridControl* pGridTradeOp9 = (SGridControl*)m_pControlContainer->FindControl(8200);
	SGridControl* pGridTradeOp10 = (SGridControl*)m_pControlContainer->FindControl(8201);

	pGridTradeOp1->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp2->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp3->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp4->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp5->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp6->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp7->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp8->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp9->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;
	pGridTradeOp10->m_eGridType = TMEGRIDTYPE::GRID_TRADEOP;

	SGridControl* pGridTradeMy1 = (SGridControl*)m_pControlContainer->FindControl(8448);
	SGridControl* pGridTradeMy2 = (SGridControl*)m_pControlContainer->FindControl(8449);
	SGridControl* pGridTradeMy3 = (SGridControl*)m_pControlContainer->FindControl(8450);
	SGridControl* pGridTradeMy4 = (SGridControl*)m_pControlContainer->FindControl(8451);
	SGridControl* pGridTradeMy5 = (SGridControl*)m_pControlContainer->FindControl(8452);
	SGridControl* pGridTradeMy6 = (SGridControl*)m_pControlContainer->FindControl(8453);
	SGridControl* pGridTradeMy7 = (SGridControl*)m_pControlContainer->FindControl(8454);
	SGridControl* pGridTradeMy8 = (SGridControl*)m_pControlContainer->FindControl(8455);
	SGridControl* pGridTradeMy9 = (SGridControl*)m_pControlContainer->FindControl(8456);
	SGridControl* pGridTradeMy10 = (SGridControl*)m_pControlContainer->FindControl(8457);

	pGridTradeMy1->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy2->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy3->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy4->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy5->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy6->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy7->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy8->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy9->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;
	pGridTradeMy10->m_eGridType = TMEGRIDTYPE::GRID_TRADEMY;

	SControl* pOpCheckButton = m_pControlContainer->FindControl(601);
	SControl* pMyCheckButton = m_pControlContainer->FindControl(617);

	m_pCargoGridList[0] = (SGridControl*)m_pControlContainer->FindControl(67328);
	m_pCargoGridList[1] = (SGridControl*)m_pControlContainer->FindControl(67329);
	m_pCargoGridList[2] = (SGridControl*)m_pControlContainer->FindControl(67330);
	m_pCargoGrid = m_pCargoGridList[0];
	m_pCargoGridList[0]->SetVisible(0);
	m_pCargoGridList[1]->SetVisible(0);
	m_pCargoGridList[2]->SetVisible(0);
	m_pCargoGrid->SetVisible(1);
	m_pCargoGridList[0]->m_bDrawGrid = 1;
	m_pCargoGridList[0]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pCargoGridList[1]->m_bDrawGrid = 1;
	m_pCargoGridList[1]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pCargoGridList[2]->m_bDrawGrid = 1;
	m_pCargoGridList[2]->m_eGridType = TMEGRIDTYPE::GRID_CARGO;

	m_pGridInvList[0] = (SGridControl*)m_pControlContainer->FindControl(67072);
	m_pGridInvList[1] = (SGridControl*)m_pControlContainer->FindControl(67073);
	m_pGridInvList[2] = (SGridControl*)m_pControlContainer->FindControl(67074);
	m_pGridInvList[3] = (SGridControl*)m_pControlContainer->FindControl(67075);
	m_pGridInvList[0]->SetVisible(0);
	m_pGridInvList[1]->SetVisible(0);
	m_pGridInvList[2]->SetVisible(0);
	m_pGridInvList[3]->SetVisible(0);
	m_pGridInv = m_pGridInvList[0];
	m_pGridInv->SetVisible(1);

	m_pQuicInvDelete = (SGridControl*)m_pControlContainer->FindControl(67080);
	m_pQuicInvDelete->m_eGridType = TMEGRIDTYPE::GRID_DELETE;
	m_pGridHelm = (SGridControl*)m_pControlContainer->FindControl(65555);
	m_pGridCoat = (SGridControl*)m_pControlContainer->FindControl(65558);
	m_pGridPants = (SGridControl*)m_pControlContainer->FindControl(65559);
	m_pGridGloves = (SGridControl*)m_pControlContainer->FindControl(65560);
	m_pGridBoots = (SGridControl*)m_pControlContainer->FindControl(65561);
	m_pGridRight = (SGridControl*)m_pControlContainer->FindControl(65556);
	m_pGridLeft = (SGridControl*)m_pControlContainer->FindControl(65557);
	m_pGridGuild = (SGridControl*)m_pControlContainer->FindControl(65548);
	m_pGridEvent = (SGridControl*)m_pControlContainer->FindControl(65549);
	m_pGridRing = (SGridControl*)m_pControlContainer->FindControl(65550);
	m_pGridNecklace = (SGridControl*)m_pControlContainer->FindControl(65551);
	m_pGridOrb = (SGridControl*)m_pControlContainer->FindControl(65552);
	m_pGridCabuncle = (SGridControl*)m_pControlContainer->FindControl(65553);
	m_pGridDRing = (SGridControl*)m_pControlContainer->FindControl(65546);
	m_pGridMantua = (SGridControl*)m_pControlContainer->FindControl(65547);
	m_pGridSkillMaster = (SGridControl*)m_pControlContainer->FindControl(65607);
	m_pGridShop = (SGridControl*)m_pControlContainer->FindControl(65694);

	m_pGridShop->m_bDrawGrid = 1;
	m_pGridShop->m_eGridType = TMEGRIDTYPE::GRID_SHOP;
	m_pGridMantua->m_eGridType = TMEGRIDTYPE::GRID_TRADENONE;
	m_pGridInvList[0]->m_bDrawGrid = 1;
	m_pGridInvList[1]->m_bDrawGrid = 1;
	m_pGridInvList[2]->m_bDrawGrid = 1;
	m_pGridInvList[3]->m_bDrawGrid = 1;
	m_pGridSkillMaster->m_bDrawGrid = 0;
	m_pGridSkillMaster->m_eGridType = TMEGRIDTYPE::GRID_SKILLM;
	m_pCargoGrid->m_bDrawGrid = 1;
	m_pCargoGrid->m_eGridType = TMEGRIDTYPE::GRID_CARGO;
	m_pGridSkillBelt = (SGridControl*)m_pControlContainer->FindControl(65598);
	m_pGridSkillBelt2 = (SGridControl*)m_pControlContainer->FindControl(65644);
	m_pGridSkillBelt2->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	m_pGridSkillBelt3 = (SGridControl*)m_pControlContainer->FindControl(65645);
	m_pGridSkillBelt3->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;
	m_pGridSkillBelt3->SetVisible(0);
	m_pShortSkillTglBtn1 = (SButton*)m_pControlContainer->FindControl(65646);
	m_pShortSkillTglBtn2 = (SButton*)m_pControlContainer->FindControl(65647);
	m_pQuick_Sloat[0] = (SGridControl*)m_pControlContainer->FindControl(66560);
	m_pQuick_Sloat[0]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT1;
	m_pQuick_Sloat[1] = (SGridControl*)m_pControlContainer->FindControl(66561);
	m_pQuick_Sloat[1]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT2;
	m_pQuick_Sloat[2] = (SGridControl*)m_pControlContainer->FindControl(66562);
	m_pQuick_Sloat[2]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT3;
	m_pQuick_Sloat[3] = (SGridControl*)m_pControlContainer->FindControl(66563);
	m_pQuick_Sloat[3]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT4;
	m_pQuick_Sloat[4] = (SGridControl*)m_pControlContainer->FindControl(66564);
	m_pQuick_Sloat[4]->m_eGridType = TMEGRIDTYPE::GRID_QUICKSLOAT5;
	m_pShortSkillTglBtn1->SetSelected(1);
	OnControlEvent(65646, 0);
	m_bSkillBeltSwitch = 0;

	int nClass = BASE_GetItemAbility(&pMobData->Equip[0], 18);

	if (pMobData->Equip[1].sIndex > 40 && nClass != 21)
	{
		STRUCT_ITEM* pItemHelm = new STRUCT_ITEM;
		memcpy(pItemHelm, &pMobData->Equip[1], 8);
				
		m_pGridHelm->AddItem(new SGridControlItem(nullptr, pItemHelm, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[2].sIndex > 40)
	{
		STRUCT_ITEM* pItemCoat = new STRUCT_ITEM;
		memcpy(pItemCoat, &pMobData->Equip[2], 8);

		m_pGridCoat->AddItem(new SGridControlItem(nullptr, pItemCoat, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[3].sIndex > 40)
	{
		STRUCT_ITEM* pItemPants = new STRUCT_ITEM;
		memcpy(pItemPants, &pMobData->Equip[3], 8);

		m_pGridPants->AddItem(new SGridControlItem(nullptr, pItemPants, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[4].sIndex > 40)
	{
		STRUCT_ITEM* pItemGloves = new STRUCT_ITEM;
		memcpy(pItemGloves, &pMobData->Equip[4], 8);

		m_pGridGloves->AddItem(new SGridControlItem(nullptr, pItemGloves, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[5].sIndex > 40)
	{
		STRUCT_ITEM* pItemBoots = new STRUCT_ITEM;
		memcpy(pItemBoots, &pMobData->Equip[5], 8);

		m_pGridBoots->AddItem(new SGridControlItem(nullptr, pItemBoots, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[7].sIndex > 40)
	{
		STRUCT_ITEM* pItemRight = new STRUCT_ITEM;
		memcpy(pItemRight, &pMobData->Equip[7], 8);

		m_pGridRight->AddItem(new SGridControlItem(nullptr, pItemRight, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[6].sIndex > 40)
	{
		STRUCT_ITEM* pItemLeft = new STRUCT_ITEM;
		memcpy(pItemLeft, &pMobData->Equip[6], 8);

		m_pGridLeft->AddItem(new SGridControlItem(nullptr, pItemLeft, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[12].sIndex > 40)
	{
		STRUCT_ITEM* pItemGuild = new STRUCT_ITEM;
		memcpy(pItemGuild, &pMobData->Equip[12], 8);

		m_pGridGuild->AddItem(new SGridControlItem(nullptr, pItemGuild, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[13].sIndex > 40)
	{
		STRUCT_ITEM* pItemEvent = new STRUCT_ITEM;
		memcpy(pItemEvent, &pMobData->Equip[13], 8);

		m_pGridEvent->AddItem(new SGridControlItem(nullptr, pItemEvent, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[14].sIndex > 40)
	{
		STRUCT_ITEM* pItemDRing = new STRUCT_ITEM;
		memcpy(pItemDRing, &pMobData->Equip[14], 8);

		m_pGridDRing->AddItem(new SGridControlItem(nullptr, pItemDRing, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[15].sIndex > 40)
	{
		STRUCT_ITEM* pItemMantua = new STRUCT_ITEM;
		memcpy(pItemMantua, &pMobData->Equip[15], 8);

		m_pGridMantua->AddItem(new SGridControlItem(nullptr, pItemMantua, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[8].sIndex > 40)
	{
		STRUCT_ITEM* pItemRing = new STRUCT_ITEM;
		memcpy(pItemRing, &pMobData->Equip[8], 8);

		m_pGridRing->AddItem(new SGridControlItem(nullptr, pItemRing, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[9].sIndex > 40)
	{
		STRUCT_ITEM* pItemNecklace = new STRUCT_ITEM;
		memcpy(pItemNecklace, &pMobData->Equip[9], 8);

		m_pGridNecklace->AddItem(new SGridControlItem(nullptr, pItemNecklace, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[10].sIndex > 40)
	{
		STRUCT_ITEM* pItemOrb = new STRUCT_ITEM;
		memcpy(pItemOrb, &pMobData->Equip[10], 8);

		m_pGridOrb->AddItem(new SGridControlItem(nullptr, pItemOrb, 0.0f, 0.0f), 0, 0);
	}
	if (pMobData->Equip[11].sIndex > 40)
	{
		STRUCT_ITEM* pItemCabuncle = new STRUCT_ITEM;
		memcpy(pItemCabuncle, &pMobData->Equip[11], 8);

		m_pGridCabuncle->AddItem(new SGridControlItem(nullptr, pItemCabuncle, 0.0f, 0.0f), 0, 0);
	}

	for (int nCarryIndex = 0; nCarryIndex < 64; ++nCarryIndex)
	{
		if (pMobData->Carry[nCarryIndex].sIndex > 40)
		{
			STRUCT_ITEM* pItemCarry = new STRUCT_ITEM;
			memcpy(pItemCarry, &pMobData->Carry[nCarryIndex], 8);
			
			int Page = nCarryIndex / 15;
			if (nCarryIndex / 15 > -1 && Page < 4)
			{
				m_pGridInvList[Page]->AddItem(new SGridControlItem(0, pItemCarry, 0.0f, 0.0f),
					nCarryIndex % 15 % 5,
					nCarryIndex % 15 / 5);
			}
			else
				delete pItemCarry;
		}
	}

	STRUCT_ITEM* pCargo = g_pObjectManager->m_stItemCargo;
	for (int nCargoIndex = 0; nCargoIndex < 120; ++nCargoIndex)
	{
		if (g_pObjectManager->m_stItemCargo[nCargoIndex].sIndex)
		{
			STRUCT_ITEM* pItemCargo = new STRUCT_ITEM;
			memcpy(pItemCargo, &pCargo[nCargoIndex], 8);
			
			int Page = nCargoIndex / 40;
			if (nCargoIndex / 40 > -1 && Page < 3)
			{
				m_pCargoGridList[Page]->AddItem(new SGridControlItem(0, pItemCargo, 0.0f, 0.0f),
					nCargoIndex % 40 % 5,
					nCargoIndex % 40 / 5);
			}
			else
				delete pItemCargo;
		}
	}

	m_pMyHuman->m_stScore = pMobData->CurrentScore;

	m_pMyHuman->SetCharHeight((float)m_pMyHuman->m_stScore.Con);
	m_pMyHuman->SetRace(pMobData->Equip[0].sIndex);

	if (BASE_GetItemAbility(&pMobData->Equip[6], 21) == 41)
	{
		m_pMyHuman->m_stLookInfo.RightMesh = m_pMyHuman->m_stLookInfo.LeftMesh;
		m_pMyHuman->m_stLookInfo.RightSkin = m_pMyHuman->m_stLookInfo.LeftSkin;
		m_pMyHuman->m_stSancInfo.Sanc6 = m_pMyHuman->m_stSancInfo.Sanc7;
		m_pMyHuman->m_stSancInfo.Legend6 = m_pMyHuman->m_stSancInfo.Legend7;
	}

	m_pMyHuman->InitObject(); 
	m_pMyHuman->CheckWeapon(pMobData->Equip[6].sIndex, pMobData->Equip[7].sIndex);
	m_pMyHuman->InitAngle(0.0f, 0.39269909f, 0.0f);
	m_pMyHuman->InitPosition((float)pMobData->HomeTownX + 0.5f, 0, (float)pMobData->HomeTownY + 0.5f);

	g_pObjectManager->m_pCamera->SetFocusedObject(m_pMyHuman);
	g_pObjectManager->m_pCamera->m_nQuaterView = 0;
	m_bLastMyAttr = BASE_GetAttr(pMobData->HomeTownX, pMobData->HomeTownY);
	UpdateScoreUI(0);
	UpdateSkillBelt();
	g_pObjectManager->m_cSelectShortSkill = 0;

	for (int ie = 0; ie < 10; ++ie)
	{
		if ((unsigned char)g_pObjectManager->m_cShortSkill[ie] < 248)
			OnKeyShortSkill(ie + 49, 0);
	}

	if (m_pMyHuman->Is2stClass() == 2 && g_pObjectManager->m_stMobData.CurrentScore.Level >= 79)
		g_pSpell[102].Delay = 1000;

	if (!m_pObjectContainerList[0]->Load(szDataPath))
	{
		LOG_WRITELOG("DataFile Not Found\r\n");
		if (!m_bCriticalError)
			LogMsgCriticalError(3, 0, 0, 0, 0);

		m_bCriticalError = 1;
	}

	for (int nY = 0; nY < 128; ++nY)
		memcpy(m_HeightMapData[nY], m_pGround->m_pMaskData[nY], 128);

	g_HeightPosX = (int)m_pGround->m_vecOffset.x;
	g_HeightPosY = (int)m_pGround->m_vecOffset.y;
	BASE_ApplyAttribute((char*)m_HeightMapData, 256);

	memcpy(m_GateMapData, m_HeightMapData, sizeof(m_HeightMapData));

	m_pItemContainer = new TreeNode(0);
	m_pGroundObjectContainer->AddChild(m_pGroundList[0]);
	m_pSun = new TMSun();

	if (m_pSun)
	{
		m_pSun->InitObject();
		m_pEffectContainer->AddChild(m_pSun);
	}

	m_pSky = new TMSky();	
	m_pSky->m_bVisible = 0;

	AddChild(m_pSky);
	AddChild(m_pItemContainer);

	SetSanc();

	m_pMyHuman->m_cHide = (m_pMyHuman->m_dwID >= 0 && m_pMyHuman->m_dwID < 1000) == 1 && m_pMyHuman->m_stScore.Reserved & 1;
	m_pHumanContainer->AddChild(m_pMyHuman);

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(102);
		if (pSoundData && !pSoundData->IsSoundPlaying())
			pSoundData->Play();
	}

	m_pRain = new TMRain();
	m_pRain->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pRain);

	m_pSnow = new TMSnow(1.0f);
	m_pSnow->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pSnow);

	m_pSnow2 = new TMSnow(2.0f);
	m_pSnow2->m_bVisible = 0;
	m_pEffectContainer->AddChild(m_pSnow2);

	m_pTarget1 = new TMEffectMesh(316, 0xFF111188, 0.0f, 0);
	m_pTarget1->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
	m_pEffectContainer->AddChild(m_pTarget1);

	m_pTarget2 = new TMEffectMesh(317, 0xFFFF0000, 0.0f, 0);
	m_pEffectContainer->AddChild(m_pTarget2);

	m_pTargetBill = new TMEffectBillBoard2(232, 0, 1.0f, 1.0f, 1.0f, 0.0005f, 0);
	m_pTargetBill->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
	m_pTargetBill->SetColor(0xFF9999AA);
	m_pEffectContainer->AddChild(m_pTargetBill);

	g_pDevice->m_colorLight.r = 1.0f;
	g_pDevice->m_colorLight.g = 1.0f;
	g_pDevice->m_colorLight.b = 1.0f;
	g_pDevice->m_colorBackLight.r = 0.0;
	g_pDevice->m_colorBackLight.g = 0.2f;
	g_pDevice->m_colorBackLight.b = 0.3f;

	m_pMiniMapPanel = (SPanel*)m_pControlContainer->FindControl(289);
	m_pMiniMapZoomIn = (SButton*)m_pControlContainer->FindControl(5714);
	m_pMiniMapZoomOut = (SButton*)m_pControlContainer->FindControl(5715);
	if (m_pMainInfo2_Name)
		m_pMainInfo2_Name->SetText(m_pMyHuman->m_szName, 1);

	m_pMiniMapServerPanel = (SPanel*)m_pControlContainer->FindControl(6136);
	m_pMiniMapServerText = (SText*)m_pControlContainer->FindControl(6137);
	m_pMiniMapDir = (SPanel*)m_pControlContainer->FindControl(291);

	SButton* pMiniMapBtn = (SButton*)m_pControlContainer->FindControl(296);
	if (m_pMiniMapPanel)
	{
		m_pMiniMapPanel->GetGeomControl()->fAngle = -0.78539819f;
		m_pMiniMapPanel->m_bSelectEnable = 0;

		if (m_pMiniMapDir)
			m_pMiniMapDir->m_bSelectEnable = 0;

		m_pMiniMapPanel->SetVisible(0);

		if (pMiniMapBtn)
			pMiniMapBtn->SetSelected(0);
		m_pMiniMapPanel->m_GCPanel.dwColor = 0x80FFFFFF;
	}

	for (int ig = 0; ig < 256; ++ig)
	{
		m_pInMiniMapPosPanel[ig] = new SPanel(-2, 0.0f, 0.0f, 4.0f, 4.0f, g_MinimapPos[ig].dwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

		if (m_pInMiniMapPosPanel[ig])
		{
			m_pInMiniMapPosPanel[ig]->m_bSelectEnable = 0;
			m_pInMiniMapPosPanel[ig]->SetVisible(0);
			m_pMiniMapPanel->AddChild(m_pInMiniMapPosPanel[ig]);
		}

		m_pInMiniMapPosText[ig] = new SText(-2,
			g_MinimapPos[ig].szTarget,
			g_MinimapPos[ig].dwColor,
			0.0f,
			0.0f,
			8.0f,
			12.0f,
			0,
			0x77777777u,
			1u,
			0);

		if (m_pInMiniMapPosText[ig])
		{
			m_pInMiniMapPosText[ig]->m_bSelectEnable = 0;
			m_pInMiniMapPosText[ig]->SetVisible(0);
			m_pMiniMapPanel->AddChild(m_pInMiniMapPosText[ig]);
		}
	}

	SPanel* pPartyPanel = (SPanel*)m_pControlContainer->FindControl(475136);
	SText* pPartyTitle = (SText*)m_pControlContainer->FindControl(475137);
	m_pPartyList = (SListBox*)m_pControlContainer->FindControl(475138);
	m_pPartyPanel = (SPanel*)pPartyPanel;
	m_pPartyBtn = (SButton*)m_pControlContainer->FindControl(5742);

	if (pPartyPanel)
	{
		pPartyPanel->SetStickLeft();
		pPartyPanel->SetStickTop();
		pPartyPanel->m_nPosY = pPartyPanel->m_nPosY + 135.0f;
	}

	if (m_pPartyBtn)
		m_pPartyBtn->SetPos(0.0f, ((float)g_pDevice->m_dwScreenHeight - pPartyPanel->m_nHeight) - 165.0f);

	if (m_pPartyBtn)
		m_pPartyBtn->m_pAltText->SetPos(m_pPartyBtn->m_pAltText->m_nPosX + 5.0f, (pPartyPanel->m_nHeight * 0.5f) - 13.0f);

	OnControlEvent(5742, 0);

	m_pMsgPanel = (SPanel*)m_pControlContainer->FindControl(669);
	m_pMsgList = (SListBox*)m_pControlContainer->FindControl(670);
	m_pMsgText = (SText*)m_pControlContainer->FindControl(638);
	m_pGMsgPanel = (SPanel*)m_pControlContainer->FindControl(819);
	m_pGMsgText = (SText*)m_pControlContainer->FindControl(820);
	m_pGMsgListPanel = (SPanel*)m_pControlContainer->FindControl(823);
	m_pGMsgViewPanel = (SPanel*)m_pControlContainer->FindControl(828);
	m_pGMsgWritePanel = (SPanel*)m_pControlContainer->FindControl(838);
	m_pGMsgList = (SListBox*)m_pControlContainer->FindControl(825);
	m_pGMsgRContext = (SListBox*)m_pControlContainer->FindControl(829);
	m_pGMsgReplyList = (SListBox*)m_pControlContainer->FindControl(830);
	m_pGMsgWContext = (SListBox*)m_pControlContainer->FindControl(840);
	m_pGMsgEditRelpy = (SEditableText*)m_pControlContainer->FindControl(837);
	m_pGMsgEditTitle = (SEditableText*)m_pControlContainer->FindControl(839);
	m_pGMsgTitile = (SText*)m_pControlContainer->FindControl(836);
	m_pBtnBoardModify = (SButton*)m_pControlContainer->FindControl(832);
	m_pBtnBoardDel = (SButton*)m_pControlContainer->FindControl(834);
	m_pBtnBoardSaveGM = (SButton*)m_pControlContainer->FindControl(842);
	m_pBtnSaveReply = (SButton*)m_pControlContainer->FindControl(831);
	m_pBtnWrite = (SButton*)m_pControlContainer->FindControl(827);

	m_pRPSGamePanel = (SPanel*)m_pControlContainer->FindControl(1616);
	m_pRPSGamePanel->SetVisible(0);
	m_pRPSGamePanel->m_bPickable = 0;
	m_pRPSGameRock = (SButton*)m_pControlContainer->FindControl(1617);
	m_pRPSGamePaper = (SButton*)m_pControlContainer->FindControl(1618);
	m_pRPSGameScissor = (SButton*)m_pControlContainer->FindControl(1619u);

	InitBoard();

	if (!LoadMsgText(m_pMsgList, (char*)"notice.txt"))
		m_pMsgPanel->SetVisible(0);

	m_pMsgPanel->SetPos(((float)g_pDevice->m_dwScreenWidth * 0.5f) - (m_pMsgPanel->m_nWidth * 0.5f),
		((float)g_pDevice->m_dwScreenHeight * 0.5f) - (m_pMsgPanel->m_nHeight * 0.5f));

	m_pMsgPanel->SetVisible(0);

	SetWeather(g_nWeather);

	if (m_pHelpList[3])
	{
		int n;
		for (n = 0; n < 99 && (g_pObjectManager->m_stMemo[n].szString[0] || g_pObjectManager->m_stMemo[n + 1].szString[0]); ++n)
		{
			SListBoxItem* pItem = new SListBoxItem(g_pObjectManager->m_stMemo[n].szString,
				g_pObjectManager->m_stMemo[n].dwColor,
				0.0f,
				0.0f,
				300.0f,
				16.0f,
				0,
				0x77777777u,
				1u,
				0);

			m_pHelpList[3]->AddItem(pItem);
		}

		if (n > 0 && m_pHelpMemo)
			m_pHelpMemo->SetVisible(1);
	}

	m_pReelPanel = new SReelPanel(330, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);
	m_pReelPanel2 = new SReelPanel(340, 21.0f, 79.0f, 62.0f, 62.0f, 1.0f);

	m_pGambleStore->AddChild(m_pReelPanel);
	m_pGambleStore->AddChild(m_pReelPanel2);

	if (!strcmp(g_TempName, g_pObjectManager->m_stMobData.MobName))
	{
		if (strcmp(g_TempNick, " "))
		{
			MSG_MessageWhisper stWhisper{};
			stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
			stWhisper.Header.Type = MSG_MessageWhisper_Opcode;

			sprintf(stWhisper.MobName, "tab");
			sprintf(stWhisper.String, g_TempNick);
			SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
		}
	}

	m_pServerPanel = (SPanel*)m_pControlContainer->FindControl(12288);
	m_pServerList = (SListBox*)m_pControlContainer->FindControl(12289);

	m_pServerPanel->SetPos((float)(g_pDevice->m_dwScreenWidth >> 1) - (m_pServerPanel->m_nWidth / 2.0f),
		(float)(g_pDevice->m_dwScreenHeight >> 1) - (m_pServerPanel->m_nHeight / 2.0f));
	m_pServerPanel->SetVisible(0);

	if (g_nKeyType == 1)
		m_pControlContainer->SetFocusedControl(m_pEditChat);

	InitCameraView();
	SetCameraView();

	m_pAlphaNative->SetPos((float)(m_pEditChat->m_nWidth + m_pChatPanel->m_nWidth) - 40.0f,
		(float)(m_pChatPanel->m_nPosY + m_pEditChat->m_nPosY) - 2.0f);

	m_pFireWorkPanel = (SPanel*)m_pControlContainer->FindControl(8705);

	if (m_pFireWorkPanel)
	{
		for (int jj = 0; jj < 100; ++jj)
		{
			m_pFireWorkButton[jj] = new SButton(375,
				(float)(25 * (jj % 10)) + 3.0f,
				(float)(25 * (jj / 10)) + 3.0f,
				24.0f,
				24.0f,
				0x77777777u,
				1,
				(char*)"");

			m_pFireWorkButton[jj]->SetControlID(jj + 8706);
			m_pFireWorkButton[jj]->SetEventListener(m_pControlContainer ? m_pControlContainer : nullptr);

			m_pFireWorkPanel->AddChild(m_pFireWorkButton[jj]);
		}

		m_pFireWorkPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[5] = m_pFireWorkPanel;
		m_pFireWorkPanel->SetVisible(0);

		m_pFireWorkPanel->SetPos((float)(g_pDevice->m_dwScreenWidth) - (m_pFireWorkPanel->m_nWidth / 2.0f),
			(float)(g_pDevice->m_dwScreenHeight) - (m_pFireWorkPanel->m_nHeight / 2.0f));
	}

	m_pTotoPanel = (SPanel*)m_pControlContainer->FindControl(8961);

	if (m_pTotoPanel)
	{
		m_pTotoSelect_Btn = (SButton*)m_pControlContainer->FindControl(8964);
		m_pTotoBuy_Btn = (SButton*)m_pControlContainer->FindControl(8978);
		m_pTotoQuit_Btn = (SButton*)m_pControlContainer->FindControl(8966);
		m_pTotoNumber_Edit = (SEditableText*)m_pControlContainer->FindControl(8963);
		m_pTotoNumber_Edit->m_nMaxStringLen = 3;
		m_pTotoTime_Txt = (SText*)m_pControlContainer->FindControl(8968);
		m_pTotoTeamA_Txt = (SText*)m_pControlContainer->FindControl(8971);
		m_pTotoTeamB_Txt = (SText*)m_pControlContainer->FindControl(8972);
		m_pTotoScoreA_Edit = (SEditableText*)m_pControlContainer->FindControl(8973);
		m_pTotoScoreA_Edit->m_nMaxStringLen = 3;
		m_pTotoScoreB_Edit = (SEditableText*)m_pControlContainer->FindControl(8974);
		m_pTotoScoreB_Edit->m_nMaxStringLen = 3;
		m_pTotoPanel->m_bModal = 1;
		m_pControlContainer->m_pModalControl[6] = (SControl*)m_pTotoPanel;
		m_pTotoPanel->SetVisible(0);

		m_pTotoPanel->SetPos((float)(g_pDevice->m_dwScreenWidth) - (m_pTotoPanel->m_nWidth / 2.0f),
			(float)(g_pDevice->m_dwScreenHeight) - (m_pTotoPanel->m_nHeight / 2.0f));
	}

	BASE_ReadTOTOList((char*)"UI\\TOTOGame.csv");

	SetAutoTarget();

	memset(&m_stMoveStop, 0, sizeof(m_stMoveStop));

	m_dwLastCheckAutoMouse = g_pTimerManager->GetServerTime();
	m_dwLastCheckPlayTime = g_pTimerManager->GetServerTime();
	m_nPlayTime = 0;

	g_pTextureManager->Release_GuildMarkList();
	g_pObjectManager->m_pCamera->m_fMaxCamLen = 15.0f;
	if (m_pPartyPanel)
		m_pPartyPanel->SetVisible(0);

	for (int ih = 0; ih < 32; ++ih)
	{
		m_pAffectIcon[ih] = (SPanel*)m_pControlContainer->FindControl(ih + 90400);
		m_pTargetAffectIcon[ih] = (SPanel*)m_pControlContainer->FindControl(ih + 90369);
	}

	m_pAffectDesc = (SText*)m_pControlContainer->FindControl(65798);
	m_pAffectDescList[0] = (SText*)m_pControlContainer->FindControl(773);
	m_pAffectDescList[1] = (SText*)m_pControlContainer->FindControl(774);
	m_pAffectDescList[2] = (SText*)m_pControlContainer->FindControl(775);
	m_pAffectDescList[3] = (SText*)m_pControlContainer->FindControl(776);
	m_pAffectDescList[4] = (SText*)m_pControlContainer->FindControl(777);
	m_pAffectDescList[5] = (SText*)m_pControlContainer->FindControl(784);
	m_pAffectDescList[6] = (SText*)m_pControlContainer->FindControl(794);
	m_pAffectDescList[7] = (SText*)m_pControlContainer->FindControl(795);
	m_pAffectDescList[8] = (SText*)m_pControlContainer->FindControl(796);
	m_pAffectDescList[9] = (SText*)m_pControlContainer->FindControl(797);
	m_pAffectDescList[10] = (SText*)m_pControlContainer->FindControl(798);
	m_pAffectDescList[11] = (SText*)m_pControlContainer->FindControl(799);

	for (int j = 0; j < 13; ++j)
	{
		for (int ii = 0; ii < 32; ++ii)
			m_pPartyAffectIcon[j][ii] = (SPanel*)m_pControlContainer->FindControl(32 * j + ii + 475152);
	}

	m_pPartyAffectText = (SText*)m_pControlContainer->FindControl(7602193);
	m_pPartyAutoButton = (SButton*)m_pControlContainer->FindControl(7602194);
	m_pPartyAutoText = (SText*)m_pControlContainer->FindControl(7602195);
	
	SListBox* temp1 = (SListBox*)m_pControlContainer->FindControl(1054276u);
	SListBox* temp2 = (SListBox*)m_pControlContainer->FindControl(1054277u);

	m_pDailyQuestButton = (SButton*)m_pControlContainer->FindControl(1054278);

	if (temp1)
		temp1->SetVisible(0);
	if (temp2)
		temp2->SetVisible(0);

	if (m_pDailyQuestButton)
		m_pDailyQuestButton->SetVisible(0);

	LOG_WRITELOG(">> Init Field Scene::End\r\n");
	g_bEffectFirst = 1;
	return 1;
}

/// Atualiza o loop de quadro da cena: camera, entidades, efeitos, auto-ataque e timers.
int TMFieldScene::FrameMove(unsigned int dwServerTime)
{
	if (g_bEffectFirst == 1)
	{
		UpdateMyHuman();
		g_bEffectFirst = 0;
	}

	dwServerTime = g_pTimerManager->GetServerTime();

	TMScene::FrameMove(dwServerTime);

	if (TimeDelay(dwServerTime) == 1)
		return 1;

	//if (m_dwDeleteURLTime && dwServerTime > m_dwDeleteURLTime + 1000)
	//{
	//	if (DelTempFiles(g_pMessageStringTable[269]) == 1)
	//		m_dwDeleteURLTime = 0;
	//	else
	//		m_dwDeleteURLTime = dwServerTime;
	//}

	AirMove_Main(dwServerTime);
	Affect_Main(dwServerTime);

	if ((m_pGround->m_vecOffsetIndex.x != 8 || m_pGround->m_vecOffsetIndex.y != 15) && 
		(m_pGround->m_vecOffsetIndex.x != 8 || m_pGround->m_vecOffsetIndex.y != 16) && 
		(m_pGround->m_vecOffsetIndex.x != 9 || m_pGround->m_vecOffsetIndex.y != 15) && 
		(m_pGround->m_vecOffsetIndex.x != 9 || m_pGround->m_vecOffsetIndex.y != 16))
	{
		if (m_bTempCastlewar == 1)
		{
			g_bCastleWar = 0;
			g_bCastleWar2 = 0;
		}
	}
	else if (!g_bCastleWar)
	{
		g_bCastleWar = 1;
		g_bCastleWar2 = 1;
		m_bTempCastlewar = 1;
	}
	if (!m_nAdjustTime && dwServerTime > m_dwInitTime + 3000)
		m_nAdjustTime = 1;

	GameAuto();

	int azran = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
	if (azran != 1 || 
		m_pMyHuman->m_vecPosition.x < 2604.0f || m_pMyHuman->m_vecPosition.x > 2651.0f || 
		m_pMyHuman->m_vecPosition.y < 1708.0f || m_pMyHuman->m_vecPosition.y > 1744.0f)
	{
		if (g_bEvent == 1)
		{
			g_bEvent = 0;
			m_bShowNameLabel = 1;
		}
	}
	else if (!g_bEvent)
	{
		g_bEvent = 1;
		m_bShowNameLabel = 0;
	}
	if (m_dwLastDeadTime && dwServerTime - m_dwLastDeadTime > 180000 && m_pMyHuman->m_cDie == 1 && 
		(int)m_pMyHuman->m_vecPosition.x >> 7 != 1 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 1)
	{
		MSG_STANDARD stStandard;
		stStandard.ID = g_pObjectManager->m_dwCharID;
		stStandard.Type = 0x289;
		SendOneMessage((char*)&stStandard, 12);
		m_dwLastDeadTime = 0;
	}
	if (dwServerTime - m_dwQuizStart > 5000 && m_pQuizPanel && m_pQuizPanel->m_bVisible)
		m_pQuizPanel->SetVisible(0);

	bool bInTown = m_pMyHuman->IsInTown();
	unsigned int WaitTime = 180000;

	if (dwServerTime - m_dwEventStartTime > WaitTime)
	{
		++m_nCurrEventTextIndex;
		m_nCurrEventTextIndex %= 4;
		m_dwEventStartTime = dwServerTime;

		auto pChatList = m_pChatListnotice;
		for (int k = 3; k >= 0; --k)
		{
			if (pChatList == nullptr)
				break;

			if (strlen(m_szEventTextTemp[k]))
			{
				auto pChatItem = new SListBoxItem(m_szEventTextTemp[k],
					0xFFFFF6A6,
					0.0f,
					0.0f,
					300.0f,
					16.0f,
					0,
					0x77777777,
					1,
					0);

				if (pChatItem)
					pChatList->AddItem(pChatItem);
			}
		}
	}

	auto pCPanel = m_pCPanel;

	if (pCPanel && pCPanel->IsVisible() == 1)
	{
		int btPressType = 0;
		auto pOpSTRButton = (SButton*)m_pControlContainer->FindControl(65716);
		auto pOpINTButton = (SButton*)m_pControlContainer->FindControl(65719);
		auto pOpDEXButton = (SButton*)m_pControlContainer->FindControl(65722);
		auto pOpCONButton = (SButton*)m_pControlContainer->FindControl(65725);
		if (pOpSTRButton->IsOver() == 1)
			btPressType = 65716;
		if (pOpINTButton->IsOver() == 1)
			btPressType = 65719;
		if (pOpDEXButton->IsOver() == 1)
			btPressType = 65722;
		if (pOpCONButton->IsOver() == 1)
			btPressType = 65725;

		auto pOpSP1Button = (SButton*)m_pControlContainer->FindControl(65754);
		auto pOpSP2Button = (SButton*)m_pControlContainer->FindControl(65757);
		auto pOpSP3Button = (SButton*)m_pControlContainer->FindControl(65760);
		auto pOpSP4Button = (SButton*)m_pControlContainer->FindControl(65763);
		if (pOpSP1Button->IsOver() == 1)
			btPressType = 65754;
		if (pOpSP2Button->IsOver() == 1)
			btPressType = 65757;
		if (pOpSP3Button->IsOver() == 1)
			btPressType = 65760;
		if (pOpSP4Button->IsOver() == 1)
			btPressType = 65763;

		if (!btPressType || !g_pEventTranslator->button[0])
		{
			m_bClbutton = 0;
			m_dwLastClbuttonTime = 0;
		}
		else if (m_bClbutton)
		{
			if (m_dwLastClbuttonTime + 100 < dwServerTime)
			{
				OnControlEvent(btPressType, 0);
				m_dwLastClbuttonTime = dwServerTime;
			}
		}
		else if (m_dwLastClbuttonTime)
		{
			if (m_dwLastClbuttonTime + 1500 < dwServerTime)
			{
				m_dwLastClbuttonTime = dwServerTime;
				m_bClbutton = 1;
			}
		}
		else
		{
			m_dwLastClbuttonTime = dwServerTime;
		}
	}

	if (dwServerTime - TMHouse::m_dwVisibleWaterFall > 1000)
	{
		auto pSoundManager = g_pSoundManager;
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(6);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}

	if (m_bCriticalError == 1 && m_pMessagePanel)
	{
		m_pMessagePanel->SetMessage("Critical Data Error In Client", 0);
		m_pMessagePanel->SetVisible(1, 0);
		return 1;
	}

	if (m_pMessagePanel && m_pMessageBox->m_dwMessage == 11 && m_pMessageBox->IsVisible() == 1 && 
		g_pObjectManager->m_stMobData.CurrentScore.Hp > 0)
	{
		m_pMessageBox->SetVisible(0);
	}

	if (m_pMiniMapDir)
		m_pMiniMapDir->GetGeomControl()->fAngle = m_pMyHuman->m_fAngle - 2.3561945f;

	if (m_pMiniMapPanel)
	{
		if (m_pMiniMapPanel->m_bVisible)
		{
			float fX = 0.0f;
			float fY = 0.0f;
			float fX2 = 0.0f;
			float fY2 = 0.0f;

			auto pFocused = g_pObjectManager->m_pCamera->m_pFocusedObject;
			if (pFocused)
			{
				int nCullSize = 400;
				if (TMGround::m_fMiniMapScale < 1.0)
					nCullSize = 130;

				int PlusX = 60;
				int PlusY = 10;
				for (int l = 0; l < 256; ++l)
				{
					fX = pFocused->m_vecPosition.x - (float)g_MinimapPos[l].nX;
					fY = pFocused->m_vecPosition.y - (float)g_MinimapPos[l].nY;

					// TODO: confirm this "-" latter.
					fX2 = ((-((0.70710701f * fX) + (-0.70710701f * fY))	* 2.0f) + 78.0f)
						+ ((signed int)TMGround::m_fMiniMapScale * 128.0f);
					fY2 = ((((0.70710701f * fX) + (0.70710701f * fY)) * 2.0f) + 78.0f)
						+ ((signed int)TMGround::m_fMiniMapScale * 128.0f);

					if (fX2 >= 0.0 && fX2 <= (float)nCullSize && fY2 >= 0.0 && fY2 <= (float)nCullSize)
						m_pInMiniMapPosPanel[l]->SetVisible(1);
					else
						m_pInMiniMapPosPanel[l]->SetVisible(0);

					if (((float)g_MinimapPos[l].nCX + fX2) >= 0.0f && ((float)g_MinimapPos[l].nCX + fX2) <= (float)(nCullSize - PlusX) && 
						((float)g_MinimapPos[l].nCY + fY2) >= 0.0f && ((float)g_MinimapPos[l].nCY + fY2) <= (float)(nCullSize - PlusY))
					{
						if (TMGround::m_fMiniMapScale >= 1.0f)
							m_pInMiniMapPosText[l]->SetVisible(1);
						else
							m_pInMiniMapPosText[l]->SetVisible(0);
					}
					else
					{
						m_pInMiniMapPosText[l]->SetVisible(0);
					}

					m_pInMiniMapPosPanel[l]->SetPos(BASE_ScreenResize(fX2), BASE_ScreenResize(fY2));
					if (TMGround::m_fMiniMapScale >= 1.0f)
					{
						m_pInMiniMapPosText[l]->SetPos(BASE_ScreenResize((float)g_MinimapPos[l].nCX + fX2), 
							BASE_ScreenResize((float)g_MinimapPos[l].nCY + fY2));
					}
				}
			}
		}
	}
	if (m_pTarget1 && m_pTarget2 && m_pTargetBill)
	{
		if (m_pTarget1->m_bShow == 1)
		{
			m_pTarget1->m_fAngle = ((float)(dwServerTime % 1000) * 3.1415927f) / 500.0f;
			m_pTarget2->m_fAngle = m_pTarget1->m_fAngle;

			m_pTargetBill->m_vecScale.x = (float)(sinf(m_pTarget1->m_fAngle * 2.0f) * 0.1f) + 1.0f;
			m_pTargetBill->m_vecScale.y = m_pTargetBill->m_vecScale.x;
			m_pTargetBill->m_vecScale.z = m_pTargetBill->m_vecScale.x;
		}
		if ((m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_WALK|| m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_RUN)
			&& !m_bAutoRun && !m_pMyHuman->m_pMoveTargetHuman && !m_pMyHuman->m_pMoveSkillTargetHuman && !m_pTargetItem)
		{
			m_pTarget1->m_bShow = 1;
			m_pTarget2->m_bShow = 1;
			m_pTargetBill->m_bShow = 1;
		}
		else
		{
			m_pTarget1->m_bShow = 0;
			m_pTarget2->m_bShow = 0;
			m_pTargetBill->m_bShow = 0;
		}
	}

	bool bWarn = 0;
	for (int nZone = 0; nZone < 7; ++nZone)
	{
		POINT pt;
		pt.x = (int)m_pMyHuman->m_vecPosition.x;
		pt.y = (int)m_pMyHuman->m_vecPosition.y;
		if (PtInRect(&TMFieldScene::m_rectWarning[nZone], pt) == 1)
		{
			bWarn = 1;
			break;
		}
	}

	if (!m_bWarning && bWarn == 1)
	{
		m_bWarning = 1;
		m_pMessagePanel->SetMessage(g_pMessageStringTable[42], 2000);
		m_pMessagePanel->SetVisible(1, 1);
	}
	else if (!bWarn)
		m_bWarning = 0;

	int nDesert = (int)m_pMyHuman->m_vecPosition.x >> 7 >= 8 && (int)m_pMyHuman->m_vecPosition.x >> 7 <= 12 && 
		(int)m_pMyHuman->m_vecPosition.y >> 7 >= 11&& (int)m_pMyHuman->m_vecPosition.y >> 7 <= 14;

	if (!RenderDevice::m_bDungeon && nDesert == 1)
	{
		RenderDevice::m_bDungeon = 3;
	}
	else if (RenderDevice::m_bDungeon == 3 && !nDesert)
	{
		RenderDevice::m_bDungeon = 0;
	}
	if (RenderDevice::m_bDungeon == 4)
	{
		if (dwServerTime - m_dwRemainTime > 900000)
			m_pRemainText->SetVisible(0);
	}
	else if (RenderDevice::m_bDungeon != 5 && dwServerTime - m_dwRemainTime > 6000)
	{
		m_pRemainText->SetVisible(0);
	}
	if (RenderDevice::m_bDungeon == 5)
	{
		UpdateQuestTime();
	}
	else if (m_pQuestRemainTime->IsVisible() == 1)
	{
		SetQuestStatus(0);
	}

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 6 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 28 && m_pPositionText)
		m_pPositionText->SetVisible(0);

	if ((int)m_pMyHuman->m_vecPosition.x >> 7 != 31 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 31 && m_pGambleStore->m_bVisible == 1)
		SetVisibleGamble(0, 0);

	if (!g_bCastleWar2)
	{
		STRUCT_MOB* pMobData = &g_pObjectManager->m_stMobData;
		if (bInTown == 1)
		{
			POINT pt;
			pt.x = (int)m_pMyHuman->m_vecPosition.x;
			pt.y = (int)m_pMyHuman->m_vecPosition.y;

			int nMusicIndex = 0;
			if (PtInRect(&rectTownInCastle, pt) != 1)
			{
				int city = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
				if (city == 2)
					city = 0;

				nMusicIndex = 2 * city + 1;
			}
			else
				nMusicIndex = 8;

			if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
			{
				m_dwInTownTime = dwServerTime;
				if (g_pApp->m_pBGMManager)
				{
					SAFE_DELETE(g_pApp->m_pBGMManager);

					g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
					g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
					g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
				}

				DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				
				MSG_STANDARDPARM dst{};
				dst.Header.ID = g_pObjectManager->m_dwCharID;
				dst.Header.Type = 0x291;
				dst.Parm = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);

				if (dst.Parm < 4)
				{
					SendOneMessage((char*)&dst, 16);
					g_pObjectManager->m_stSelCharData.HomeTownX[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.x;
					g_pObjectManager->m_stSelCharData.HomeTownY[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.y;
				}
			}
		}
		else if((int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			if ((int)m_pMyHuman->m_vecPosition.x <= 1664 || (int)m_pMyHuman->m_vecPosition.x >= 1792 || 
				(int)m_pMyHuman->m_vecPosition.y <= 1536 || (int)m_pMyHuman->m_vecPosition.y >= 1920)
			{
				if (RenderDevice::m_bDungeon == 3)
				{
					int nMusicIndex = 9;
					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}		
				else if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26	&& (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
						 (int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
				{
					int nMusicIndex = 9;
					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}
				else
				{
					POINT pt;
					pt.x = (int)m_pMyHuman->m_vecPosition.x;
					pt.y = (int)m_pMyHuman->m_vecPosition.y;

					int m;
					for (m = 0; m < 2; ++m)
					{
						if (PtInRect(&g_rectField[m], pt) != 1)
							break;
					}

					if (m == 2)
						m = 1;

					int nMusicIndex = 2 * m + 2;
					if (RenderDevice::m_bDungeon == 4 && m_pRemainText->m_bVisible == 1)
						nMusicIndex = 5;

					if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
					{
						m_dwFieldTime = dwServerTime;
						if (g_pApp->m_pBGMManager)
						{
							SAFE_DELETE(g_pApp->m_pBGMManager);

							g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
							g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
							g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
						}
						DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
					}
					if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
						SetVisibleAutoTrade(0, 0);
				}
			}
			else
			{
				int nMusicIndex = 6;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}

				if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
					SetVisibleAutoTrade(0, 0);
			}
		}
		else
		{
			if ((int)m_pMyHuman->m_vecPosition.x >> 7 < 16 && (int)m_pMyHuman->m_vecPosition.x >> 7 > 8 &&
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 25)
			{
				int nMusicIndex = 7;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}
			else if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 18 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 30)
			{
				int nMusicIndex = 12;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
				if (m_pMyHuman && m_pMyHuman->m_vecPosition.y >= 3918.0)
					SetVisibleKhepraPortal(m_dwKhepraID == 0);
				else
					SetVisibleKhepraPortal(0);

				FrameMove_KhepraDieEffect(dwServerTime);
			}
			else if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 16 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 > 29)
			{
				int nMusicIndex = 11;
				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}
			else
			{
				int nMusicIndex = 5;

				if ((int)m_pMyHuman->m_vecPosition.x >> 7 != 31 && (int)m_pMyHuman->m_vecPosition.y >> 7 != 31)
				{
					if (m_pGambleStore->m_bVisible == 1)
						SetVisibleGamble(0, 0);
				}
				else
					nMusicIndex = 6;

				if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
				{
					m_dwFieldTime = dwServerTime;
					if (g_pApp->m_pBGMManager)
					{
						SAFE_DELETE(g_pApp->m_pBGMManager);

						g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
						g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
						g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
					}
					DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
				}
			}

			if (m_pAutoTrade && m_pAutoTrade->IsVisible() == 1)
				SetVisibleAutoTrade(0, 0);
		}
	}
	else
	{
		if (bInTown == 1)
		{
			int nVillage = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
			if (m_nVillage != nVillage)
			{
				MSG_STANDARDPARM stParam{};
				stParam.Header.ID = g_pObjectManager->m_dwCharID;
				stParam.Header.Type = 0x291;
				stParam.Parm = BASE_GetVillage((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
				m_nVillage = nVillage;

				if (stParam.Parm < 4)
				{
					SendOneMessage((char*)&stParam, 16);
					g_pObjectManager->m_stSelCharData.HomeTownX[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.x;
					g_pObjectManager->m_stSelCharData.HomeTownY[g_pObjectManager->m_cCharacterSlot] = (int)m_pMyHuman->m_vecPosition.y;
				}
			}
		}
		else
		{
			m_nVillage = -1;
		}

		int nMusicIndex = 10;
		if (DS_SOUND_MANAGER::m_nMusicIndex != nMusicIndex)
		{
			m_dwFieldTime = dwServerTime;
			if (g_pApp->m_pBGMManager)
			{
				SAFE_DELETE(g_pApp->m_pBGMManager);

				g_pApp->m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * g_pApp->m_nMusic - 3000);
				g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
				g_pApp->m_pBGMManager->PlayMusic(nMusicIndex);
			}
			DS_SOUND_MANAGER::m_nMusicIndex = nMusicIndex;
		}
	}
	if (m_pPositionText)
	{
		if (m_pPositionText->IsVisible() == 1)
		{
			char szPos[64]{};
			sprintf(szPos, "X: %4d  Y: %4d", (int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
			m_pPositionText->SetText(szPos, 0);

			char szServer[64]{};
			int nServerGroupIndex = nServerGroupIndex = g_pObjectManager->m_nServerGroupIndex;
			sprintf(szServer, "%s-%d", g_szServerNameList[nServerGroupIndex], g_pObjectManager->m_nServerIndex);
			if (m_pMiniMapServerText)
				m_pMiniMapServerText->SetText(szServer, 0);
		}
	}

	for (int i = 0; i < 32; ++i)
	{
		if ((unsigned char)m_pMyHuman->m_stAffect[i].Type > 0)
		{
			char szVal[128]{};
			short sTime = 8 * m_pMyHuman->m_stAffect[i].Time - (dwServerTime - m_dwStartAffectTime[i]) / 1000;
			if (sTime <= 0)
			{
				if (m_pAffectL[i])
				{
					if (!i)
						m_pAffect[0]->SetTextColor(0xFF000000);

					m_pAffect[i]->SetText((char*)"    0", 0);
					m_pAffectL[i]->SetVisible(0);
				}

				continue;
			}

			if (m_pMyHuman->m_stAffect[i].Time >= 1000000)
			{
				int add = m_pMyHuman->m_stAffect[i].Time / 1000000;
				int Year = m_pMyHuman->m_stAffect[i].Time % 1000000 / 10000;
				int Day = m_pMyHuman->m_stAffect[i].Time % 10000;
				int nYearDay = 365;
				if (!(Year % 4))
					nYearDay = 366;

				int nDefaultDay = 0;
				switch (add)
				{
				case 4:
					nDefaultDay = 7;
					break;
				case 5:
					nDefaultDay = 15;
					break;
				case 6:
					nDefaultDay = 30;
					break;
				}
				if (add)
				{
					int freeDay = nDefaultDay - nYearDay * (m_nYear - Year) - (m_nDays - Day);
					sprintf(szVal, "%dD", freeDay);
				}
			}
			else if (sTime > 86400)
			{
				int nDay = sTime / 86400;
				sprintf(szVal, "%ddD", sTime / 86400);
			}
			else if (sTime > 3600)
			{
				int nHour = sTime / 3600;
				sprintf(szVal, "%dH", sTime / 3600);
			}
			else if (sTime > 600)
			{
				int nMin = sTime / 60;
				sprintf(szVal, "%dM", sTime / 60);
			}
			else
			{
				sprintf(szVal, "%5d", sTime);
			}

			if (m_pAffectL[i])
			{
				if (!i)
				{
					if (sTime < 7375)
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
					else
						m_pAffect[i]->SetTextColor(0xFFFFFFFF);
				}
				m_pAffect[i]->SetText(szVal, 0);
			}

			if (m_pAffectL[i])
				m_pAffectL[i]->SetVisible(0);
			if (m_pMiniPanel)
			{
				if ((unsigned char)m_pMyHuman->m_stAffect[i].Type < 50)
					m_pAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[i].Type];
				else
					m_pAffectIcon[i]->m_GCPanel.nTextureIndex = 0;
				m_pAffectIcon[i]->m_GCPanel.nLayer = 29;
			}
		}
		else
		{
			if (m_pAffectL[i])
				m_pAffectL[i]->SetVisible(0);
			m_dwAffectBlinkTime[i] = 0;
		}
	}
	if (g_pObjectManager->m_stMobData.CurrentScore.Hp > 0 && m_bAutoRun && !g_pCursor->m_pAttachedItem)
	{
		TMVector2 vec = m_pMyHuman->m_vecPosition;
		float fDir = 1.0f;

		if (m_bReverse)
			fDir = -1.0f;

		vec.x = (((fDir * g_pObjectManager->m_pCamera->m_vecCamDir.x)
			* m_pMyHuman->m_fMaxSpeed)
			* 1.5f)
			+ vec.x;
		vec.y = (((fDir * g_pObjectManager->m_pCamera->m_vecCamDir.z)
			* m_pMyHuman->m_fMaxSpeed)
			* 1.5f)
			+ vec.y;

		if (!BASE_IsInLowZone((int)vec.x, (int)vec.y))
			MobMove2(vec, dwServerTime);
	}
	if (!m_pMyHuman->m_cLastMoveStop)
	{
		if (m_pMyHuman->m_fMaxSpeed >= 5.0f)
		{
			if (m_pMyHuman->m_fProgressRate >= 0.8f || m_pMyHuman->m_fProgressRate == 0.0f)
			{
				if ((int)m_pMyHuman->m_vecPosition.x == m_vecMyNext.x && (int)m_pMyHuman->m_vecPosition.y == m_vecMyNext.y && 
					(m_pMyHuman->m_LastSendTargetPos.x != m_vecMyNext.x || m_pMyHuman->m_LastSendTargetPos.y != m_vecMyNext.y) && 
					dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
				{
					m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

					MSG_Action Msg{};
					Msg.Header.ID = m_pMyHuman->m_dwID;
					Msg.PosX = m_stMoveStop.NextX;
					Msg.PosY = m_stMoveStop.NextY;
					Msg.Effect = 0;
					Msg.Header.Type = MSG_Action_Opcode;
					Msg.Speed = g_nMyHumanSpeed;
					Msg.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
					Msg.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

					for (int n = 0; n < 23; ++n)
						Msg.Route[n] = 0;

					g_bLastStop = Msg.Header.Type;
					m_stMoveStop.LastX = Msg.PosX;
					m_stMoveStop.LastY = Msg.PosY;
					m_stMoveStop.NextX = Msg.TargetX;
					m_stMoveStop.NextY = Msg.TargetY;
					SendOneMessage((char*)&Msg, 52);
					m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
				}
				else if (m_vecMyNext.x != (int)m_pMyHuman->m_vecPosition.x || m_vecMyNext.y != (int)m_pMyHuman->m_vecPosition.y)
				{
					m_pMyHuman->GetRoute(m_vecMyNext, 0, m_pMyHuman->m_cLastMoveStop);
				}
			}
		}
		else if (m_pMyHuman->m_fProgressRate >= 0.90f || m_pMyHuman->m_fProgressRate == 0.0f)
		{
			int nPosX = (int)m_pMyHuman->m_vecPosition.x;
			int nPosY = (int)m_pMyHuman->m_vecPosition.y;
			if (nPosX == m_vecMyNext.x && nPosY == m_vecMyNext.y && (m_pMyHuman->m_LastSendTargetPos.x != m_vecMyNext.x	|| 
				m_pMyHuman->m_LastSendTargetPos.y != m_vecMyNext.y) && dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
			{
				m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

				MSG_Action stAction{};
				stAction.Header.ID = m_pMyHuman->m_dwID;
				stAction.PosX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.PosY = m_pMyHuman->m_LastSendTargetPos.y;
				stAction.Effect = 0;
				stAction.Header.Type = MSG_Action_Opcode;
				stAction.Speed = g_nMyHumanSpeed;
				stAction.TargetX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.TargetY = m_pMyHuman->m_LastSendTargetPos.y;

				for (int j = 0; j < 23; ++j)
					stAction.Route[j] = 0;

				g_bLastStop = stAction.Header.Type;
				m_stMoveStop.LastX = stAction.PosX;
				m_stMoveStop.LastY = stAction.PosY;
				m_stMoveStop.NextX = stAction.TargetX;
				m_stMoveStop.NextY = stAction.TargetY;
				SendOneMessage((char*)&stAction, 52);
				m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
			}
			else if (m_vecMyNext.x != (int)m_pMyHuman->m_vecPosition.x || m_vecMyNext.y != (int)m_pMyHuman->m_vecPosition.y)
				m_pMyHuman->GetRoute(m_vecMyNext, 0, m_pMyHuman->m_cLastMoveStop);
		}
	}
	if (m_pTargetItem)
		m_pMyHuman->MoveGet(m_pTargetItem);

	if (m_dwStartFlashTime)
	{
		if (m_fFlashTerm <= 0.0f)
			m_fFlashTerm = 1.0f;

		float fProgress = (float)(dwServerTime - m_dwStartFlashTime) / m_fFlashTerm;
		auto pFadePanel = m_pFadePanel;

		if (fProgress < 0.1f)
		{
			float fSin = sinf((float)(fProgress * 5.0f) * 3.1415927f);

			unsigned int dwAlpha = (unsigned int)(float)(fabsf(fSin) * 0.0f);
			pFadePanel->m_GCPanel.dwColor = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | (dwAlpha << 24);
		}
		else if (fProgress < 0.90f)
			pFadePanel->m_GCPanel.dwColor = 0xDD000000;
		else if (fProgress < 1.0f)
		{
			float fSin = sinf((((fProgress - 0.90f) * 5.0f) + 0.5f) * 3.1415927f);

			unsigned int dwAlpha = (unsigned int)(float)(fabsf(fSin) * 0.0f);
			pFadePanel->m_GCPanel.dwColor = dwAlpha | (dwAlpha << 8) | (dwAlpha << 16) | (dwAlpha << 24);
		}
		else
		{
			pFadePanel->m_GCPanel.dwColor = 0;
			m_dwStartFlashTime = 0;
		}

		m_pTargetHuman = 0;
	}

	auto pSoundManager = g_pSoundManager;
	if (m_pGround->m_bDungeon == 0|| m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4)
	{
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(341);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}
	else
	{
		if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(107);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}

		if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 18 && (int)m_pMyHuman->m_vecPosition.y >> 7 == 30)
		{
			if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(341);
				if (pSoundData && !pSoundData->IsSoundPlaying())
					pSoundData->Play();
			}
		}
		else if (pSoundManager)
		{
			auto pSoundData = pSoundManager->GetSoundData(341);
			if (pSoundData && pSoundData->IsSoundPlaying())
				pSoundData->Stop();
		}
	}
	if (g_nWeather == 2 || g_nWeather == 3)
	{
		if ((m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4) && m_pSnow->m_bVisible == 1)
		{
			m_pSnow->m_bVisible = 0;
			m_pSnow2->m_bVisible = 0;
			m_pRain->m_bVisible = 1;
		}
		else if (!m_pGround->m_bDungeon && !m_pSnow->m_bVisible)
		{
			m_pSnow->m_bVisible = 1;
			if (g_nWeather == 3)
				m_pSnow2->m_bVisible = 1;
			m_pRain->m_bVisible = 0;
		}
	}
	if (!g_nWeather && (!m_pGround->m_bDungeon || m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4))
	{
		int nTime = (dwServerTime / 600000) % 12;
		if (m_nWTime)
			nTime = m_nWTime;

		if ((m_pGround->m_bDungeon == 3 || m_pGround->m_bDungeon == 4) && m_pSnow->m_bVisible == 1)
		{
			m_pSnow->m_bVisible = 0;
			m_pSnow2->m_bVisible = 0;
			m_pRain->m_bVisible = 1;
		}
		if (nTime <= 7)
		{
			if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
				(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
			{
				if (m_pSky->m_nState != 1 && m_pSky->m_nState != 11)
				{
					m_pSky->SetWeatherState(11);
					m_pRain->m_bVisible = 0;
					m_pSnow->m_bVisible = 1;
					m_pSnow2->m_bVisible = 0;
				}
			}
			else if (m_pSky->m_nState && m_pSky->m_nState != 10)
			{
				m_pSky->SetWeatherState(10);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime >= 8 && nTime <= 9)
		{
			if (m_pSky->m_nState != 2 && m_pSky->m_nState != 12)
			{
				m_pSky->SetWeatherState(12);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime >= 10 && nTime <= 11)
		{
			if (m_pSky->m_nState != 3 && m_pSky->m_nState != 13)
			{
				m_pSky->SetWeatherState(13);
				m_pRain->m_bVisible = 0;
				m_pSnow->m_bVisible = 0;
				m_pSnow2->m_bVisible = 0;
			}
		}
		else if (nTime == 12)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 12;
				m_pSky->SetWeatherState(11);
			}
			if (!m_pRain->m_bVisible)
				m_pRain->m_bVisible = 1;
			if (m_pSnow->m_bVisible == 1)
				m_pSnow->m_bVisible = 0;
			if (m_pSnow2->m_bVisible == 1)
				m_pSnow2->m_bVisible = 0;
		}
		else if (nTime == 13)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 13;
				m_pSky->SetWeatherState(11);
			}
			if (m_pRain->m_bVisible == 1)
				m_pRain->m_bVisible = 0;
			if (!m_pSnow->m_bVisible)
				m_pSnow->m_bVisible = 1;
			if (m_pSnow2->m_bVisible == 1)
				m_pSnow2->m_bVisible = 0;
		}
		else if (nTime == 14)
		{
			if (m_pSky->m_nState != 1)
			{
				g_nWeather = 14;
				m_pSky->SetWeatherState(11);
			}
			if (m_pRain->m_bVisible == 1)
				m_pRain->m_bVisible = 0;
			if (!m_pSnow->m_bVisible)
				m_pSnow->m_bVisible = 1;
			if (!m_pSnow2->m_bVisible)
				m_pSnow2->m_bVisible = 1;
		}
		if (dwServerTime - m_dwWeatherTime > 1000)
		{
			if (!m_pGround->m_bDungeon)
			{
				int nRand = rand() % 7;
				int nTexture = 95;

				if (((int)m_pMyHuman->m_vecPosition.x >> 7 == 29 || (int)m_pMyHuman->m_vecPosition.x >> 7 == 30) && (int)m_pMyHuman->m_vecPosition.y >> 7 == 22)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(114);
						if (pSoundData && !pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}

					nTexture = 2;
				}
				else if (pSoundManager)
				{
					auto pSoundData = pSoundManager->GetSoundData(114);
					if (pSoundData && pSoundData->IsSoundPlaying())
						pSoundData->Stop();
				}

				if ((int)m_pMyHuman->m_vecPosition.x >> 7 == 28	&& (int)m_pMyHuman->m_vecPosition.y >> 7 == 24 || 
					(int)m_pMyHuman->m_vecPosition.x >> 7 == 19	&& (int)m_pMyHuman->m_vecPosition.y >> 7 == 12)
				{
					int nSoundRand = rand() % 7;
					if (nSoundRand >= 0 && nSoundRand <= 2)
					{
						if (pSoundManager)
						{
							auto pSoundData = pSoundManager->GetSoundData(367 + nSoundRand);
							if (pSoundData && !pSoundData->IsSoundPlaying())
								pSoundData->Play();
						}
					}					
				}

				if (!g_bHideEffect)
				{
					auto pPappus = new TMEffectBillBoard(nTexture, 20000, 0.2f, 0.2f, 0.2f, 0.0f, 1, 80);
					if (pPappus)
					{
						pPappus->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							((float)nRand * 0.2f) + m_pMyHuman->m_fHeight,
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pPappus->m_vecPosition.y += 1.0f;
						pPappus->m_vecStartPos = pPappus->m_vecPosition;
						pPappus->m_fCircleSpeed = ((float)nRand * 0.1f) + 1.0f;
						nRand = rand() % 7;
						pPappus->m_fParticleH = ((float)nRand * 0.5f) + 7.0f;
						nRand = rand() % 7;
						pPappus->m_fParticleV = ((float)nRand * 0.1f) + 1.0f;
						pPappus->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pPappus->m_nParticleType = nRand % 3 + 3;
						pPappus->SetColor(0xAAAAAAAA);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pPappus);
					}
				}				
			}
			if (m_pSky->m_nState == 3 || m_pSky->m_nState == 13)
			{
				if (pSoundManager)
				{
					auto pSoundData = pSoundManager->GetSoundData(107);
					if (pSoundData && !pSoundData->IsSoundPlaying())
						pSoundData->Play();
				}

				if (((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
					 (int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25) == 0 &&
					!g_bHideEffect)
				{
					int nRand = rand() % 7;
					auto pGlow = new TMEffectBillBoard(56, 20000, 0.2f, 0.2f, 0.2f, 0.0f, 1, 80);
					if (pGlow)
					{
						pGlow->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							(float)(m_pMyHuman->m_fHeight + 1.5f) + ((float)nRand * 0.2f),
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pGlow->m_vecStartPos = pGlow->m_vecPosition;
						pGlow->m_fCircleSpeed = (float)((float)nRand * 0.1f) + 1.5f;
						pGlow->m_fParticleH = (float)((float)nRand * 0.5f) + 5.0f;
						pGlow->m_fParticleV = (float)((float)nRand * 0.05f) + 0.2f;
						pGlow->m_nParticleType = nRand % 3 + 6;
						pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pGlow->SetColor(0x0FFFFFF00);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pGlow);
					}

					pGlow = new TMEffectBillBoard(60, 20000, 0.07f, 0.07f, 0.07f, 0.0f, 1, 80);
					if (pGlow)
					{
						pGlow->m_vecPosition = TMVector3((float)(m_pMyHuman->m_vecPosition.x + 2.0f) - (float)nRand,
							(float)(m_pMyHuman->m_fHeight + 1.5f) + ((float)nRand * 0.2f),
							(float)(m_pMyHuman->m_vecPosition.y + 2.0f) - (float)nRand);

						pGlow->m_vecStartPos = pGlow->m_vecPosition;
						pGlow->m_fCircleSpeed = ((float)nRand * 0.1f) + 1.5f;
						pGlow->m_fParticleH = ((float)nRand * 0.5f) + 5.0f;
						pGlow->m_fParticleV = ((float)nRand * 0.05f) + 0.2f;
						pGlow->m_nParticleType = nRand % 3 + 6;
						pGlow->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
						pGlow->SetColor(0x0FFFFFF00);
						if (m_pEffectContainer)
							m_pEffectContainer->AddChild(pGlow);
					}
				}
				if ((RenderDevice::m_bDungeon == 3 || RenderDevice::m_bDungeon == 4) && dwServerTime - m_dwLastWolfSound > 25000)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(111);
						if (pSoundData && !pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}

					m_dwLastWolfSound = dwServerTime;
				}
			}
			else if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(107);
				if (pSoundData && !pSoundData->IsSoundPlaying())
					pSoundData->Stop();
			}

			if (RenderDevice::m_bDungeon == 3)
			{
				if (dwServerTime - m_dwLastDustTime > m_dwDustTerm && m_bSandWind == 1 && g_bCastleWar == 2 && !m_pSky->m_nState)
				{
					if (pSoundManager)
					{
						auto pSoundData = pSoundManager->GetSoundData(304);
						if (pSoundData && pSoundData->IsSoundPlaying())
							pSoundData->Play();
					}
					if (!g_bHideEffect)
					{
						for (int ii = 0; ii < 5; ++ii)
						{
							float fWidth = (float)(50 * (rand() % 5)) + 300.0f;
							auto pCamera = g_pObjectManager->m_pCamera;
							
							auto pBillEffect = new TMEffectBillBoard4(193, 800 * ii + 3000, fWidth, fWidth, 1.0f, 1, 80);
							if (pBillEffect)
							{
								pBillEffect->SetPosition(100 * ii - 900, 100 * ii);
								pBillEffect->SetParticle(1600.0, 0.0);
								m_pEffectContainer->AddChild(pBillEffect);
							}
						}
					}
					++m_nDustCount;
					m_nDustCount %= 10;
					if (m_nDustCount)
						m_dwDustTerm = 1000;
					else
						m_dwDustTerm = 3000 * (rand() % 10 + 5);

					m_dwLastDustTime = dwServerTime;
				}		
				if (dwServerTime - m_dwLastPappusTime > 2000)
				{
					for (int jj = 0; jj < 8; ++jj)
					{
						int nRand = rand() % 5;
						int nRand2 = rand() % 5;

						TMVector3 vecStart = TMVector3(
							((float)(m_pMyHuman->m_vecPosition.x - 16.0f) + (float)jj) + (float)(4 * nRand),
							m_pMyHuman->m_fHeight + 8.0f,
							((float)(m_pMyHuman->m_vecPosition.y - 12.0f) + (float)jj) + (float)(2 * nRand2));

						TMVector3 vecEnd = TMVector3(
							(float)((float)(m_pMyHuman->m_vecPosition.x + 4.0f) + (float)jj) + (float)(2 * nRand),
							m_pMyHuman->m_fHeight + 0.5f,
							(float)((float)(m_pMyHuman->m_vecPosition.y + 4.0f) + (float)jj) + (float)nRand2
						);

						auto pStorm = new TMSkillMagicArrow(vecStart, vecEnd, 3, 0);
						if (pStorm)
							m_pEffectContainer->AddChild(pStorm);
					}
					m_dwLastPappusTime = dwServerTime;
				}
			}
			m_dwWeatherTime = g_pTimerManager->GetServerTime();
		}
	}
	if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 && (int)m_pMyHuman->m_vecPosition.x >> 7 < 31 && 
		(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 && (int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
	{
		int bValue = BASE_GetAttr((int)m_pMyHuman->m_vecPosition.x,	(int)m_pMyHuman->m_vecPosition.y);
		if (!(bValue & 8) && dwServerTime - m_dwLastDustTime > m_dwDustTerm	&& m_bSandWind == 1
			&& !g_bCastleWar2 && m_pSky->m_nState == 1)
		{
			if (pSoundManager)
			{
				auto pSoundData = pSoundManager->GetSoundData(304);
				if (pSoundData && pSoundData->IsSoundPlaying())
					pSoundData->Play();
			}

			for (int kk = 0; kk < 5; ++kk)
			{
				float fScaleX = (float)(50 * (rand() % 5)) + 300.0f;
				auto pCamera = g_pObjectManager->m_pCamera;

				auto pBillEffect = new TMEffectBillBoard4(422, 800 * kk + 3000, fScaleX, fScaleX, 1.0f, 1, 80);
				if (pBillEffect)
				{
					pBillEffect->SetPosition(100 * kk - 900, 100 * kk);
					pBillEffect->SetParticle(1600.0, 0.0);
					m_pEffectContainer->AddChild(pBillEffect);
				}
			}

			++m_nDustCount;
			m_nDustCount %= 10;
			if (m_nDustCount)
				m_dwDustTerm = 1000;
			else
				m_dwDustTerm = 3000 * (rand() % 10 + 5);
			m_dwLastDustTime = dwServerTime;
		}
	}

	auto pGridSkillBelt2 = m_pGridSkillBelt2;
	auto pGridSkillBelt3 = m_pGridSkillBelt3;
	int nSkill2 = -1;
	int nSkill3 = -1;

	for (size_t nShort = 0; nShort < 20; ++nShort)
	{
		char cSkillIndex = g_pObjectManager->m_cShortSkill[nShort];
		if ((unsigned char)cSkillIndex >= 105)
			cSkillIndex += 95;

		unsigned int dwSub = dwServerTime - m_dwSkillLastTime[(unsigned char)cSkillIndex];
		if (nShort >= 10)
			++nSkill3;
		else
			++nSkill2;

		if (dwSub)
		{
			float fTime = 0.0f;
			int Delay = g_pSpell[(unsigned char)cSkillIndex].Delay;
			if (m_nMySanc >= 9 && Delay >= 2)
				--Delay;
			if (m_pMyHuman->m_DilpunchJewel == 1)
				--Delay;
			if (Delay < 1)
				Delay = 1;

			if (m_pMyHuman->Is2stClass() == 2 && cSkillIndex == 102	&& g_pObjectManager->m_stMobData.CurrentScore.Level >= 79)
				Delay = 1000;

			if (Delay > 0)
				fTime = (float)dwSub / ((float)Delay * 1000.0f);
			else
				fTime = (float)dwSub / 1000.0f;

			if (fTime > 0.0f && fTime < 1.0f)
			{
				auto pItem2 = pGridSkillBelt2->GetItem(nSkill2, 0);
				if (nShort >= 10)
					pItem2 = pGridSkillBelt3->GetItem(nSkill3, 0);
				
				if (pItem2)
				{
					int idx = (unsigned char)cSkillIndex < 200 ? (unsigned char)cSkillIndex + 5000 : (unsigned char)cSkillIndex + 5200;
					if (pItem2->m_pItem->sIndex == idx)
						pItem2->m_fTimer = fTime;
				}
			}
		}
	}
	if (dwServerTime - m_dwChatTime > 10000)
	{
		char szMsg[128]{};
		sprintf(szMsg, "");
		auto pItem = new SListBoxItem(szMsg, 0xFFAAFFAA, 0.0f, 0.0f, 300.0f, 16.0f, 0, 0x77777777, 1, 0);
		if (pItem)
		{
			m_pChatList->AddItem(pItem);
			m_dwChatTime = dwServerTime;
		}
	}
	if (m_pRankTimeText && m_bRankTimeOn)
	{
		if (m_pGround->m_vecOffsetIndex.x == 1 && m_pGround->m_vecOffsetIndex.y == 31
			|| m_pGround->m_vecOffsetIndex.x == 6 && m_pGround->m_vecOffsetIndex.y == 28
			|| m_pGround->m_vecOffsetIndex.x == 9 && m_pGround->m_vecOffsetIndex.y == 28
			|| m_pGround->m_vecOffsetIndex.x == 8 && m_pGround->m_vecOffsetIndex.y == 27
			|| m_pGround->m_vecOffsetIndex.x == 10 && m_pGround->m_vecOffsetIndex.y == 27
			|| m_pGround->m_vecOffsetIndex.x == 10 && m_pGround->m_vecOffsetIndex.y == 11
			|| m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 31
			|| m_pGround->m_vecOffsetIndex.x == 14 && m_pGround->m_vecOffsetIndex.y == 30
			|| m_pGround->m_vecOffsetIndex.x == 15 && m_pGround->m_vecOffsetIndex.y == 31
			|| m_pGround->m_vecOffsetIndex.x == 8 && m_pGround->m_vecOffsetIndex.y == 2
			|| m_pGround->m_vecOffsetIndex.x == 9 && m_pGround->m_vecOffsetIndex.y == 1
			|| m_pGround->m_vecOffsetIndex.x == 10 && m_pGround->m_vecOffsetIndex.y == 2
			|| m_pGround->m_vecOffsetIndex.x == 13 && m_pGround->m_vecOffsetIndex.y == 28
			|| m_pGround->m_vecOffsetIndex.x == 14 && m_pGround->m_vecOffsetIndex.y == 28
			|| m_pGround->m_vecOffsetIndex.x == 1 && m_pGround->m_vecOffsetIndex.y == 1)
		{
			unsigned int dwElapsedRankTime = dwServerTime - m_dwStartRankTime;
			int nElapsedTime = dwElapsedRankTime / 1000;
			int nRemainTime = m_nLastTime - dwElapsedRankTime / 1000;
			if (nRemainTime > 0)
			{
				char szTime[128]{};
				int nMinute = nRemainTime / 60;
				int nSec = nRemainTime % 60;
				sprintf(szTime, "%02d : %02d", nRemainTime / 60, nRemainTime % 60);
				m_pRankTimeText->SetText(szTime, 0);
				if (nMinute < 100 && nSec < 100)
					m_pRankTimeText->SetVisible(1);
			}
			else
				m_pRankTimeText->SetVisible(0);
		}
		else
		{
			m_pRankTimeText->SetVisible(0);
			m_bRankTimeOn = 0;
		}
	}
	if (m_cAutoAttack == 1 && m_pTargetHuman && !m_pTargetHuman->m_cDie && dwServerTime - m_dwLastAutoAttackTime > 200)
	{
		if (m_pMyHuman->m_usGuild != m_pTargetHuman->m_usGuild && g_pObjectManager->m_usAllyGuild != m_pTargetHuman->m_usGuild ||
			!m_pMyHuman->m_usGuild || !m_pTargetHuman->m_usGuild)
		{
			m_pMyHuman->MoveAttack(m_pTargetHuman);
			if (m_pTargetHuman->m_nClass == 66 && m_pTargetHuman->m_cShadow == 1 && !m_pMyHuman->m_JewelGlasses)
				m_pTargetHuman = 0;

			m_dwLastAutoAttackTime = dwServerTime;
		}
	}
	else if (m_cAutoAttack == 1 && m_pTargetHuman && m_pTargetHuman->m_cDie == 1)
		m_pTargetHuman = 0;

	if (m_pMyHuman)
	{
		auto bAttr = BASE_GetAttr((int)m_pMyHuman->m_vecPosition.x, (int)m_pMyHuman->m_vecPosition.y);
		int nPosIndex = -1;
		int x = (int)m_pMyHuman->m_vecPosition.x & 0xFFFC;
		int y = (int)m_pMyHuman->m_vecPosition.y & 0xFFFC;

		for (int ll = 0; ll < 37; ++ll)
		{
			if (g_TeleportTable[ll].nX == x && g_TeleportTable[ll].nY == y)
			{
				nPosIndex = ll;
				break;
			}
		}
		if (m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND01 || m_pMyHuman->m_eMotion == ECHAR_MOTION::ECMOTION_STAND02)
		{
			int bLastAttr = m_bLastMyAttr;
			m_bLastMyAttr = bAttr;

			if (bAttr & 0x10)
			{
				if (!m_bTeleportMsg)
				{
					m_bTeleportMsg = 1;
					if (m_nLastPotal != -1)
					{
						int GridX = g_TeleportTable[nPosIndex].nX / 64;
						int GridY = g_TeleportTable[nPosIndex].nY / 64;
						int LstGridX = g_TeleportTable[m_nLastPotal].nX / 64;
						int LstGridY = g_TeleportTable[m_nLastPotal].nY / 64;
						if ((GridX != LstGridX || GridY != LstGridY) && bLastAttr == bAttr)
						{
							m_nLastPotal = nPosIndex;
							return 1;
						}
					}
					if ((int)m_pMyHuman->m_vecPosition.x > 1963 && (int)m_pMyHuman->m_vecPosition.x < 1970 &&
						(int)m_pMyHuman->m_vecPosition.y > 1770 && (int)m_pMyHuman->m_vecPosition.y < 1777)
					{
						m_pMessagePanel->SetMessage(g_pMessageStringTable[160], 3000);
						m_pMessagePanel->SetVisible(1, 1);
					}
					else
					{
						if ((int)m_pMyHuman->m_vecPosition.x >= 2370 && (int)m_pMyHuman->m_vecPosition.x <= 2411 &&
							(int)m_pMyHuman->m_vecPosition.y >= 1728 && (int)m_pMyHuman->m_vecPosition.y <= 1759)
						{
							m_pMessagePanel->SetMessage(g_pMessageStringTable[258], 3000);
							m_pMessagePanel->SetVisible(1, 1);
							return 1;
						}
						if (!m_pMessageBox->IsVisible())
						{
							if (m_bAirMove == 1)
								return 1;

							if (m_pMyHuman->m_vecPosition.x > 1366.5f && m_pMyHuman->m_vecPosition.x < 3366.5f &&
								m_pMyHuman->m_vecPosition.y > 2926.5f && m_pMyHuman->m_vecPosition.y < 4926.5f
								&& m_dwKhepraID)
							{
								return 1;
							}

							m_nLastPotal = nPosIndex;
							if (nPosIndex >= 0)
							{
								char szGoto[256]{};
								sprintf(szGoto, g_pMessageStringTable[208], g_TeleportTable[nPosIndex].szTarget);
								int nShowPrice = 0;
								if (!m_cWarClan && m_pMyHuman->m_cMantua && m_pMyHuman->m_cMantua != 3)
									nShowPrice = 1;
								if (m_cWarClan == 7 && m_pMyHuman->m_cMantua != 1)
									nShowPrice = 1;
								if (m_cWarClan == 8 && m_pMyHuman->m_cMantua != 2)
									nShowPrice = 1;
								if (m_cWarClan == -1)
									nShowPrice = 1;

								if (g_TeleportTable[nPosIndex].nPrice > 0 && nShowPrice == 1)
								{
									char szPrice[256]{};
									sprintf(szPrice, g_pMessageStringTable[207], g_TeleportTable[nPosIndex].nPrice);
									m_pMessageBox->SetMessage(szGoto, 16, szPrice);
								}
								else
								{
									m_pMessageBox->SetMessage(szGoto, 16, 0);
								}
							}
							m_pMessageBox->SetVisible(1);
						}
					}
				}
			}
		}

		if (!(bAttr & 0x10)	&& m_pMessageBox->IsVisible() == 1 && m_pMessageBox->m_dwMessage == 16)
			m_pMessageBox->SetVisible(0);
	}

	int nViewgridx = 33;
	for (int i = 0; i < 100; ++i)
	{
		if (i == 53)
			nViewgridx = 20;

		if (fabsf(static_cast<float>(g_pInitItem[i].PosX) - m_pMyHuman->m_vecPosition.x) < nViewgridx - 1)
		{
			if (fabsf(static_cast<float>(g_pInitItem[i].PosY) - m_pMyHuman->m_vecPosition.y) < nViewgridx - 1)
			{
				CreateGate(i, m_bInitGate);
			}
		}
	}

	if (m_bInitGate == 1)
		m_bInitGate = 0;

	FindAuto();

	for (int i = 0; i < m_pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)m_pPartyList->m_pItemList[i];
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
		if (pHuman)
		{
			if (pHuman->m_cCancel == 1)
			{
				pPartyItem->m_GCText.dwColor = 0xFFFF0000;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString,	pPartyItem->m_GCText.dwColor, 0);
			}
			else if (pPartyItem->m_nState == 2)
			{
				pPartyItem->m_GCText.dwColor = 0xFFAAAAFF;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
			}
			else if (!pPartyItem->m_nState)
			{
				pPartyItem->m_GCText.dwColor = 0xFFFFFFFF;
				pPartyItem->m_GCText.pFont->SetText(pPartyItem->m_GCText.strString, pPartyItem->m_GCText.dwColor, 0);
			}
		}
	}

	return 1;
}

void TMFieldScene::PGTVisible(unsigned int dwServerTime)
{
	auto pOver = m_pMouseOverHuman;
	if (pOver && (pOver->m_dwID <= 0 || pOver->m_dwID > 1000))
		return;

	if (!pOver || pOver->m_bMouseOver != 1 || dwServerTime < m_dwPGTTime + 500)
		return;

	m_dwOpID = pOver->m_dwID;
	auto pPGTText = m_pPGTText;

	char szStr[128]{};
	sprintf(szStr, g_pMessageStringTable[60], pOver->m_szName);
	pPGTText->SetText((char*)"", 0);

	RECT rt;
	rt.left = 2564;
	rt.top = 1689;
	rt.right = 2579;
	rt.bottom = 1711;

	POINT pt;
	pt.x = (int)m_pMyHuman->m_vecPosition.x;
	pt.y = (int)m_pMyHuman->m_vecPosition.y;
	PtInRect(&rt, pt);

	m_pBtnPGTGuild->SetVisible(1);
	m_pBtnPGTParty->SetVisible(1);
	m_pBtnPGTTrade->SetVisible(1);
	m_pBtnPGTGuildDrop->SetVisible(0);
	m_pBtnPGTGuildWar->SetVisible(0);
	m_pBtnPGTGuildAlly->SetVisible(0);
	m_pBtnPGTGuildInvite->SetVisible(0);
	m_pBtnPGT1_V_1->SetVisible(0);
	m_pBtnPGT5_V_5->SetVisible(0);
	m_pBtnPGT10_V_10->SetVisible(0);
	m_pBtnPGTAll_V_All->SetVisible(0);
	m_pBtnPGTChallenge->SetVisible(1);
	m_pBtnPGTGICommon->SetVisible(0);
	m_pBtnPGTGIChief1->SetVisible(0);
	m_pBtnPGTGIChief2->SetVisible(0);
	m_pBtnPGTGIChief3->SetVisible(0);
	m_pPGTOver = pOver;
	m_pPGTPanel->SetVisible(1);
	m_dwPGTTime = g_pTimerManager->GetServerTime();
}

int TMFieldScene::TimeDelay(unsigned int dwServerTime)
{
	if (g_dwStartQuitGameTime)
	{
		if (dwServerTime > g_dwStartQuitGameTime + 5000)
		{
			PostMessage(g_pApp->m_hWnd, 16, 0, 0);
			return 1;
		}

		unsigned int dwRemain = (g_dwStartQuitGameTime + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[224], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
		}
	}
	if (m_dwLastLogout)
	{
		if (dwServerTime > m_dwLastLogout + 5000)
		{
			MSG_STANDARD stStandard{};
			stStandard.ID = g_pObjectManager->m_dwCharID;
			stStandard.Type = 0x215;
			SendOneMessage((char*)&stStandard, sizeof(stStandard));
			m_dwLastLogout = 0;
			return 1;
		}

		unsigned int dwRemain = (m_dwLastLogout + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[225], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
		}
	}
	if (m_dwLastSelServer)
	{
		if (dwServerTime > m_dwLastSelServer + 5000)
		{
			m_dwLastSelServer = 0;
			g_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
			return 1;
		}

		unsigned int dwRemain = (m_dwLastSelServer + 5000 - dwServerTime) / 1000;
		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[224], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);
			if (g_bActiveWB == 1)
				g_pApp->SwitchWebBrowserState(0);
		}
	}
	if (m_dwLastTown)
	{
		if (dwServerTime > m_dwLastTown + 6000 && !m_cLastTown)
			m_dwLastTown = 0;
		else if (dwServerTime > m_dwLastTown + 5000 && m_cLastTown == 1)
		{
			MSG_STANDARD stRecall{};
			stRecall.ID = m_pMyHuman->m_dwID;
			stRecall.Type = MSG_Recall_Opcode;
			SendOneMessage((char*)&stRecall, sizeof(stRecall));

			m_cResurrect = 0;
			m_pMyHuman->m_cDie = 0;
			m_pMyHuman->SetAnimation(ECHAR_MOTION::ECMOTION_LEVELUP, 0);

			if (!m_pMyHuman->m_cHide)
			{
				auto pLevelUp = new TMEffectLevelUp(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight, m_pMyHuman->m_vecPosition.y), 0);
				if (pLevelUp)
					m_pEffectContainer->AddChild(pLevelUp);
			}

			m_cLastTown = 0;
		}
		else
		{
			unsigned int dwRemain = (m_dwLastSelServer + 5000 - dwServerTime) / 1000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				auto pPortal = new TMSkillTownPortal(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 0.05f, m_pMyHuman->m_vecPosition.y), 1);
				if (pPortal)
					m_pEffectContainer->AddChild(pPortal);
			}
		}
	}
	if (m_dwLastResurrect)
	{
		MSG_AttackOne stAttack{};
		stAttack.Header.Type = MSG_Attack_One_Opcode;
		stAttack.Header.ID = m_pMyHuman->m_dwID;
		stAttack.AttackerID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].TargetID = m_pMyHuman->m_dwID;
		stAttack.Dam[0].Damage = -1;
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
		stAttack.SkillIndex = 99;
		stAttack.SkillParm = 0;
		stAttack.Motion = -1;

		if (dwServerTime > m_dwLastResurrect + 6000 && !m_cResurrect)
			m_dwLastResurrect = 0;
		else if (dwServerTime > m_dwLastResurrect + 5000 && m_cResurrect == 1)
		{
			m_cResurrect = 0;
			stAttack.FlagLocal = 0;
			SendOneMessage((char*)&stAttack, sizeof(stAttack));
		}
		else
		{
			unsigned int dwRemain = (m_dwLastResurrect + 5000 - dwServerTime) / 2000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;
				stAttack.FlagLocal = 1;

				OnPacketEvent(stAttack.Header.Type, (char*)&stAttack);
			}
		}
	}
	if (m_dwLastTeleport)
	{
		if (dwServerTime > m_dwLastTeleport + 6000 && !m_cLastTeleport)
			m_dwLastTeleport = 0;
		else if (dwServerTime > m_dwLastTeleport + 5000 && m_cLastTeleport == 1)
		{
			if (m_nServerMove)
			{
				m_dwDelayDisconnectTime = dwServerTime;

				MSG_MessageWhisper stWhisper{};
				stWhisper.Header.ID = g_pObjectManager->m_dwCharID;
				stWhisper.Header.Type = MSG_MessageWhisper_Opcode;
				sprintf(stWhisper.MobName, "srv");
				sprintf(stWhisper.String, "%d", m_nServerMove);
				SendOneMessage((char*)&stWhisper, sizeof(stWhisper));
				m_nServerMove = 0;
			}
			else
			{
				SendOneMessage((char*)&m_stUseItem, sizeof(m_stUseItem));
				memset(&m_stUseItem, 0, sizeof(m_stUseItem));
				m_dwUseItemTime = dwServerTime;
			}
			m_cLastTeleport = 0;
		}
		else
		{
			unsigned int dwRemain = (m_dwLastTeleport + 5000 - dwServerTime) / 1000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				auto pPortal = new TMSkillTownPortal(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 0.05f, m_pMyHuman->m_vecPosition.y), 1);
				if (pPortal)
					m_pEffectContainer->AddChild(pPortal);
			}
		}
	}
	if (m_dwLastRelo)
	{
		if (dwServerTime > m_dwLastRelo + 6000 && !m_cLastRelo)
			m_dwLastRelo = 0;
		else if (dwServerTime > m_dwLastRelo + 5000 && m_cLastRelo == 1)
		{
			MSG_MessageWhisper stMsgImp{};
			stMsgImp.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgImp.Header.Type = MSG_MessageWhisper_Opcode;
			sprintf(stMsgImp.MobName, "relo");
			sprintf(stMsgImp.String, "%s", m_szSummoner);
			SendOneMessage((char*)&stMsgImp, sizeof(stMsgImp));
			m_cLastRelo = 0;
		}
		else if (dwServerTime > m_dwLastRelo + 5000 && m_cLastRelo == 2)
		{
			MSG_MessageWhisper stMsgImp{};
			stMsgImp.Header.ID = g_pObjectManager->m_dwCharID;
			stMsgImp.Header.Type = MSG_MessageWhisper_Opcode;
			sprintf(stMsgImp.MobName, "relo");
			sprintf(stMsgImp.String, "%s", m_szSummoner2);
			SendOneMessage((char*)&stMsgImp, sizeof(stMsgImp));
			m_cLastRelo = 0;
		}
		else
		{
			unsigned int dwRemain = (m_dwLastRelo + 5000 - dwServerTime) / 1000;
			if (m_dwLastRemain != dwRemain)
			{
				m_bAutoRun = 0;
				m_dwLastRemain = dwRemain;

				auto pPortal = new TMSkillTownPortal(TMVector3(m_pMyHuman->m_vecPosition.x,	m_pMyHuman->m_fHeight + 0.05f, m_pMyHuman->m_vecPosition.y), 1);
				if (pPortal)
					m_pEffectContainer->AddChild(pPortal);
			}
		}
	}

	if (m_dwLastWhisper)
	{
		if (dwServerTime > m_dwLastWhisper + 6000 && !m_cLastWhisper)
		{
			m_dwLastWhisper = 0;
			return 0;
		}
		if (dwServerTime > m_dwLastWhisper + 5000 && m_cLastWhisper == 1)
		{
			SendOneMessage((char*)&m_stLastWhisper, sizeof(m_stLastWhisper));
			memset((char*)&m_stLastWhisper, 0, sizeof(m_stLastWhisper));
			m_dwLastWhisper = 0;
			return 1;
		}

		unsigned int dwRemain = (m_dwLastWhisper + 5000 - dwServerTime) / 1000;

		if (m_dwLastRemain != dwRemain)
		{
			m_bAutoRun = 0;
			m_dwLastRemain = dwRemain;

			char szMsg[128]{};
			sprintf(szMsg, g_pMessageStringTable[223], m_dwLastRemain + 1);

			m_pMessagePanel->SetMessage(szMsg, 2000);
			m_pMessagePanel->SetVisible(1, 1);		
		}
	}

	return 0;
}

void TMFieldScene::FrameMove_KhepraDieEffect(unsigned int dwServerTime)
{
	if (m_nKhepraDieFlag < 1 || m_dwKhepraDieTime + 300 >= dwServerTime)
		return;

	int RndX = rand() % 9;
	int RndZ = rand() % 9;
	float X = (float)RndX + 2362.0f;
	float Y = -6.8000002f;
	float Z = (float)RndZ + 3927.0f;

	auto pThunder1 = new TMSkillThunderBolt(TMVector3(X, Y, Z), 3);
	m_pEffectContainer->AddChild(pThunder1);
	auto pDust1 = new TMEffectDust(TMVector3(X, Y, Z), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust1);
	auto pDust2 = new TMEffectDust(TMVector3(X - 0.3f, Y, Z - 0.3f), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust2);
	auto pDust3 = new TMEffectDust(TMVector3(X + 0.3f, Y, Z + 0.3f), 50.0f, 0);
	m_pEffectContainer->AddChild(pDust3);

	m_dwKhepraDieTime += 100 * (rand() % 10 + 1) + 300;
	m_nKhepraDieFlag++;

	if (m_nKhepraDieFlag >= 10)
	{
		auto pPoison = new TMSkillPoison(TMVector3(2365.0f, -9.8000002f, 3930.0f), 0xFFCC6666, 25, 1, 0);
		m_pEffectContainer->AddChild(pPoison);

		m_nKhepraDieFlag = 0; 
		m_dwKhepraDieTime = 0;
	}
}

void TMFieldScene::FindAuto()
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (dwServerTime - LastSendTime > 250000)
	{
		MSG_STANDARD stStandard{};
		stStandard.ID = g_pObjectManager->m_dwCharID;
		stStandard.Type = MSG_Ping_Opcode;
		SendOneMessage((char*)&stStandard, sizeof(stStandard));
	}

	// TODO: china stuffs here, not needed for now
}

int TMFieldScene::FindProcess(unsigned int processID)
{
	return 0;
}

void TMFieldScene::CreateGate(int nZoneIndex, int bInit)
{
	auto pOldItem = (TMItem*)g_pObjectManager->GetItemByID(nZoneIndex + 15001);
	if (pOldItem)
		return;

	int a = nZoneIndex == 53 ? 0 : 0;
	
	MSG_CreateItem stCreateItem{};
	stCreateItem.ItemID = nZoneIndex + 15001;
	stCreateItem.Item.sIndex = g_pInitItem[nZoneIndex].sIndex;
	stCreateItem.GridX = g_pInitItem[nZoneIndex].PosX;
	stCreateItem.GridY = g_pInitItem[nZoneIndex].PosY;
	stCreateItem.Rotate = static_cast<char>(g_pInitItem[nZoneIndex].Rotate);
	stCreateItem.Height = 16;
	stCreateItem.State = 2;
	auto pCreateItem = &stCreateItem;

	if (BASE_GetItemAbility(&stCreateItem.Item, 34) > 0)
	{
		TMGate* pItem = nullptr;
		if (stCreateItem.Item.sIndex >= 0 && stCreateItem.Item.sIndex <= 6500)
		{
			auto pItem = new TMGate();

			pItem->InitItem(pCreateItem->Item);
			pItem->InitGate(pCreateItem->Item);

			pItem->m_dwID = pCreateItem->ItemID;
			pItem->m_nMaskIndex = 0;
			pItem->InitObject();
			pItem->InitAngle(0.0f, ((float)pCreateItem->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

			float fX = (float)pCreateItem->GridX + 0.5f;
			float fY = (float)pCreateItem->GridY + 0.5f;

			float fHeight = m_pGround->GetHeight(TMVector2(fX, fY));

			if (fHeight < -500.0f)
			{
				auto pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
				if (pOtherGround)
					fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
				else
				{
					TMScene::FrameMove(0);
					pOtherGround = m_pGroundList[(m_nCurrentGroundIndex + 1) % 2];
					if (pOtherGround)
						fHeight = pOtherGround->GetHeight(TMVector2(fX, fY));
				}
			}

			pItem->InitPosition(fX, fHeight, fY);
			int nMaskIndex = BASE_GetItemAbility(&pCreateItem->Item, 34);
			BASE_UpdateItem2(nMaskIndex, 1, (unsigned char)pCreateItem->State, pCreateItem->GridX, pCreateItem->GridY, (char*)m_HeightMapData, 
				(int)(pItem->m_fAngle / D3DXToRadian(90)), pCreateItem->Height);

			pItem->SetState((EGATE_STATE)pCreateItem->State);
			if (!pOldItem)
				m_pItemContainer->AddChild(pItem);
		}

		return;
	}

	TMItem* pItem = nullptr;
	if (pOldItem)
		pItem = pOldItem;
	else if (g_pItemList[pCreateItem->Item.sIndex].nIndexMesh == 1607)
	{
		pItem = new TMCannon();
		pItem->m_dwObjType = 1607;
	}
	else
	{
		pItem = new TMItem();
	}

	if (!pItem)
		return;

	pItem->InitItem(pCreateItem->Item);
	pItem->m_dwID = pCreateItem->ItemID;
	pItem->m_nMaskIndex = 0;
	pItem->InitObject();
	pItem->InitAngle(0.0f, ((float)pCreateItem->Rotate * D3DXToRadian(180)) / 2.0f, 0.0f);

	float fX = (float)pCreateItem->GridX + 0.5f;
	float fY = (float)pCreateItem->GridY + 0.5f;

	float fHeight = GroundGetMask(TMVector2(fX, fY)) * 0.1f;
	pItem->InitPosition(fX, fHeight + 0.1f, fY);

	if (!pOldItem)
		m_pItemContainer->AddChild(pItem);

	if (pCreateItem->Create == 1)
	{
		if (BASE_GetItemAbility(&pCreateItem->Item, 38) == 2)
			GetSoundAndPlay(44, 0, 0);
		else if (pCreateItem->Item.sIndex == 412 || pCreateItem->Item.sIndex == 413 || pCreateItem->Item.sIndex == 4141 || 
			pCreateItem->Item.sIndex == 419 || pCreateItem->Item.sIndex == 420)
		{
			GetSoundAndPlay(48, 0, 0);
		}
		else
			GetSoundAndPlay(45, 0, 0);
	}
}

void TMFieldScene::SetWeather(int nWeather)
{
	g_nWeather = nWeather;

	if (((int)m_pMyHuman->m_vecPosition.x >> 7 == 29 || 
		(int)m_pMyHuman->m_vecPosition.x >> 7 == 30) &&
		(int)m_pMyHuman->m_vecPosition.y >> 7 == 22 == 1)
	{
		nWeather = 0;
	}
	else if ((nWeather == 2 || nWeather == 3) && (RenderDevice::m_bDungeon == 3 || RenderDevice::m_bDungeon == 4))
	{
		nWeather = 1;
	}
	else
	{
		if ((int)m_pMyHuman->m_vecPosition.x >> 7 > 26 &&
			(int)m_pMyHuman->m_vecPosition.x >> 7 < 31 &&
			(int)m_pMyHuman->m_vecPosition.y >> 7 > 20 &&
			(int)m_pMyHuman->m_vecPosition.y >> 7 < 25)
		{
			if (nWeather == 1)
				nWeather = 3;
			else
				nWeather = 2;
		}
	}

	switch (nWeather)
	{
	case 1:
		m_pRain->m_bVisible = 1;
		m_pSnow->m_bVisible = 0;
		m_pSnow2->m_bVisible = 0;
		m_pSky->SetWeatherState(11);
		break;
	case 2:
		m_pRain->m_bVisible = 0;
		m_pSnow->m_bVisible = 1;
		m_pSnow2->m_bVisible = 0;
		m_pSky->SetWeatherState(11);
		break;
	case 3:
		m_pRain->m_bVisible = 0;
		m_pSnow->m_bVisible = 1;
		m_pSnow2->m_bVisible = 1;
		m_pSky->SetWeatherState(11);
		break;
	}
}

/// Atualiza deslocamento aereo (voo entre cidades/zonas) a cada quadro.
void TMFieldScene::AirMove_Main(unsigned int dwServerTime)
{
	// TODO: change the state to enum
	if (m_nAirMove_State && m_pMyHuman)
	{
		if (m_bAirMove)
			m_pMyHuman->m_cHide = 0;

		switch (m_nAirMove_State)
		{
		case 1:
		{
			AirMove_Start(0);
		}
		break;
		case 2:
		{
			if (dwServerTime > m_dwAirMove_TickTime + 2800)
			{
				m_nAirMove_State = 3;
				m_dwAirMove_TickTime = dwServerTime;
				return;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 15000.0f)) / 6.0f);
			if (fVal >= 0.3f)
				fVal = 0.3f;
			if (fVal < 0.0f)
				fVal = 0.0f;

			if (m_pMyHuman->m_pMount)
				m_pMyHuman->m_pMount->m_dwFPS = 15;
			m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight + fVal;
		}
		break;
		case 3:
		{
			int nRnd = rand() % 300;
			if (dwServerTime >= m_dwAirMove_TickTime + nRnd + 1000)
			{
				m_dwAirMove_TickTime = dwServerTime;
				m_bAirMove_Wing = m_bAirMove_Wing == 0;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 10000.0f)) / 6.0f) + ((float)nRnd / 8000.0f);
			if (m_bAirMove_Wing || m_pMyHuman->m_fHeight >= 14.0)
			{
				if (m_pMyHuman->m_fHeight > 8.0)
					m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight - fVal;
			}
			else
				m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight + fVal;

			float AddY = 0.0f;
			float AddX = 0.0f;
			float MyX = m_pMyHuman->m_vecPosition.x;
			float MyY = m_pMyHuman->m_vecPosition.y;
			if (MyX >= (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX)
				AddX = -m_fAirMove_Speed;
			else if ((float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX >= MyX)
				AddX = +m_fAirMove_Speed;
			if (MyY >= (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY)
				AddY = -m_fAirMove_Speed;
			else if ((float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY >= MyY)
				AddY = +m_fAirMove_Speed;

			float DiffX = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX - (float)(MyX + AddX);
			float DiffY = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY - (float)(MyY + AddY);
			if (DiffX <= 0.5f && DiffX >= -0.5f)
				AddX = 0.0f;
			if (DiffY <= 0.5f && DiffY >= -0.5f)
				AddY = 0.0f;

			if (fabsf(AddX) == m_fAirMove_Speed && fabsf(AddY) == m_fAirMove_Speed)
			{
				AddX = AddX / 2.0f;
				AddY = AddY / 2.0f;
			}
			if (AddX != 0.0f || AddY != 0.0f)
			{
				m_pMyHuman->m_vecAirMove.x = m_pMyHuman->m_vecAirMove.x + AddX;
				m_pMyHuman->m_vecAirMove.y = m_pMyHuman->m_vecAirMove.y + AddY;

				float dPosX = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX - m_pMyHuman->m_vecPosition.x;
				float dPosY = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY - m_pMyHuman->m_vecPosition.y;

				int nRouteIndex = m_nAirMove_RouteIndex;
				if (nRouteIndex > 9)
					nRouteIndex = 9;

				if (fabsf(dPosX) + fabsf(dPosY) < 70.0f
					&& (nRouteIndex == 9 || g_pAirMoveRoute[m_nAirMove_Index][nRouteIndex + 1].nX == 0 && g_pAirMoveRoute[m_nAirMove_Index][nRouteIndex + 1].nY == 0))
				{
					if (m_fAirMove_Speed >= 0.2f)
						m_fAirMove_Speed = m_fAirMove_Speed - sinf((m_fAirMove_Speed * 0.0023f) * D3DXToRadian(180));
				}
				else if (m_fAirMove_Speed <= 0.69999999f)
					m_fAirMove_Speed = sinf((m_fAirMove_Speed * 0.0049999999f) * D3DXToRadian(180)) + m_fAirMove_Speed;

				if (m_pMyHuman->m_pMount)
					m_pMyHuman->m_pMount->m_dwFPS = 15 - (int)(m_fAirMove_Speed * 2.0f);

				m_pMyHuman->m_fWantAngle = atan2f(dPosX, dPosY) + D3DXToRadian(90);
			}
			else if (g_pAirMoveRoute[m_nAirMove_Index][++m_nAirMove_RouteIndex].nX || g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY)
			{
				m_vecAirMove_Dest.x = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nX;
				m_vecAirMove_Dest.y = (float)g_pAirMoveRoute[m_nAirMove_Index][m_nAirMove_RouteIndex].nY;
				m_dwAirMove_TickTime = dwServerTime;
			}
			else
			{
				m_nAirMove_State = 4;
				m_dwAirMove_TickTime = dwServerTime;
			}
		}
		break;
		case 4:
		{
			float fHeight = m_pMyHuman->m_fWantHeight;
			float fDiff = m_pMyHuman->m_fHeight - fHeight;
			if (dwServerTime > m_dwAirMove_TickTime + 2800)
			{
				m_pMyHuman->m_fHeight = fHeight;
				m_nAirMove_State = 5;
				m_dwAirMove_TickTime = dwServerTime;
				return;
			}

			float fVal = sinf((D3DXToRadian(180) * ((float)(dwServerTime - m_dwAirMove_TickTime) / 15000.0f)) / 6.0f);
			if (m_pMyHuman->m_pMount)
				m_pMyHuman->m_pMount->m_dwFPS = 15;
			if ((float)(m_pMyHuman->m_fHeight - fVal) > 0.0f)
				m_pMyHuman->m_fHeight = m_pMyHuman->m_fHeight - fVal;
		}
		break;
		case 5:
		{
			AirMove_End();
		}
		break;
		}
	}
}

void TMFieldScene::AirMove_Start(int nIndex)
{
	m_nAirMove_State = 2;
	m_bAirMove = 1;
	m_eOldMotion = m_pMyHuman->m_eMotion;
	m_nOldMountSkinMeshType = m_pMyHuman->m_nMountSkinMeshType;

	m_pMyHuman->UpdateMount();
	m_pMyHuman->m_bIgnoreHeight = 1;
	m_dwAirMove_TickTime = g_pTimerManager->GetServerTime();
	m_pMyHuman->m_vecAirMove.x = 0.0f;
	m_pMyHuman->m_vecAirMove.y = 0.0f;

	auto pParticle = new TMEffectParticle(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y), 
		1, 10, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);

	m_pEffectContainer->AddChild(pParticle);

	m_vecAirMove_Dest.x = (float)g_pAirMoveRoute[nIndex][0].nX;
	m_vecAirMove_Dest.y = (float)g_pAirMoveRoute[nIndex][0].nY;

	MSG_STANDARDPARM2 stAirmoveStart{};
	stAirmoveStart.Header.Type = MSG_AirMove_Start_Opcode;
	stAirmoveStart.Header.ID = m_pMyHuman->m_dwID;
	stAirmoveStart.Parm1 = nIndex;
	stAirmoveStart.Parm2 = 1;
	SendOneMessage((char*)&stAirmoveStart, sizeof(stAirmoveStart));
	m_nAirMove_Index = nIndex;
	m_nAirMove_RouteIndex = 0;
	m_fAirMove_Speed = 0.2f;
}

void TMFieldScene::AirMove_End()
{
	if (this->m_pMyHuman)
	{
		m_pMyHuman->m_vecPosition.x = m_pMyHuman->m_vecPosition.x + m_pMyHuman->m_vecAirMove.x;
		m_pMyHuman->m_vecPosition.y = m_pMyHuman->m_vecPosition.y + m_pMyHuman->m_vecAirMove.y;
		m_nAirMove_State = -1;
		m_bAirMove = 0;

		if (m_nOldMountSkinMeshType <= 0)
		{
			SAFE_DELETE(m_pMyHuman->m_pMount);
			m_pMyHuman->m_cMount = 0;
			m_pMyHuman->m_nMountSkinMeshType = 0;
		}
		else
		{
			m_pMyHuman->m_nSkinMeshType = m_nOldMountSkinMeshType;
			m_nOldMountSkinMeshType = -1;
			m_pMyHuman->UpdateMount();
		}

		UpdateMyHuman();
		m_pMyHuman->m_bIgnoreHeight = 0;
		m_dwAirMove_TickTime = 0;
		m_pMyHuman->SetAnimation(m_eOldMotion, 1);

		auto pParticle = new TMEffectParticle(TMVector3(m_pMyHuman->m_vecPosition.x, m_pMyHuman->m_fHeight + 1.0f, m_pMyHuman->m_vecPosition.y),
			1, 10, 3.0f, 0, 1, 56, 1.0f, 1, TMVector3(0.0f, 0.0f, 0.0f), 1000);
		m_pEffectContainer->AddChild(pParticle);

		if (m_nAirMove_Index < 0 || m_nAirMove_Index > 10)
			m_nAirMove_Index = 0;


		MSG_STANDARDPARM2 stAirmoveStart{};
		stAirmoveStart.Header.Type = MSG_AirMove_Start_Opcode;
		stAirmoveStart.Header.ID = m_pMyHuman->m_dwID;
		stAirmoveStart.Parm1 = m_nAirMove_Index;
		stAirmoveStart.Parm2 = 2;
		SendOneMessage((char*)&stAirmoveStart, sizeof(stAirmoveStart));
	}
}

/// Atualiza icones, temporizadores e efeitos locais de buffs/debuffs ativos.
int TMFieldScene::Affect_Main(unsigned int dwServerTime)
{
	if (!m_pMyHuman || !m_pMiniPanel)
		return 0;

	int i = 0;
	int nAvailCount = 0;
	m_pAffectDesc->SetVisible(0);
	m_pAffectDesc->SetText((char*)"", 0);
	bool bomb = false;
	for (int i = 0; i < 32; ++i)
	{
		if ((unsigned char)m_pMyHuman->m_stAffect[i].Type <= 50)
		{
			if (!m_pAffectIcon[i]->IsVisible())
				m_pAffect[i]->SetVisible(0);

			int sTime = 8 * m_pMyHuman->m_stAffect[i].Time - (dwServerTime - m_dwStartAffectTime[i]) / 1000;

			char szVal[128]{};
			GetTimeString(szVal, sTime, m_pMyHuman->m_stAffect[i].Time, i);
			int nTime = m_pMyHuman->m_stAffect[i].Time;

			if (m_pAffectIcon[i] && i && (unsigned char)m_pMyHuman->m_stAffect[i].Type > 0 || !i && nTime > 0)
			{
				m_pAffectIcon[i]->m_nPosX = BASE_ScreenResize(30.0f);
				m_pAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f) + m_pAffectIcon[i]->m_nWidth) * (float)(nAvailCount + 1))
					+ m_pAffectIcon[i]->m_nPosX;
				m_pAffectIcon[i]->m_nPosY = BASE_ScreenResize(80.0f);

				if (m_pAffectIcon[i]->m_bOver == 1)
				{
					if (m_pAffectL[i])
					{
						m_pAffect[i]->SetVisible(0);
						m_pAffectL[i]->SetVisible(0);
					}

					char strAffectString[128]{};
					if (m_pMyHuman->m_stAffect[i].Type == 8)
					{
						auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
						int efvalue = m_pMyHuman->m_stAffect[i].Value;
						int CheckBit = 0;
						int Cnt = 0;
						for (int k = 0; k < 10; ++k)
						{
							CheckBit = 1 << k;
							if ((1 << k) & efvalue)
							{
								sprintf(strAffectString, "%s %s", strAffectString, g_pAffectSubTable[k]);
								++Cnt;
							}
						}

						int slen = strlen(strAffectString) + 4;
						sprintf(strAffectString, "%s : %s", strAffectString, szVal);
						m_pAffectDesc->SetText(strAffectString, 0);
						m_pAffectDesc->m_nPosX = m_pAffectIcon[i]->m_nPosX;
						if ((float)(m_pAffectDesc->m_nWidth + 60.0f) + m_pAffectDesc->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
							m_pAffectDesc->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffectDesc->m_nWidth + 60.0f);

						m_pAffectDesc->m_nPosY = m_pAffectIcon[i]->m_nPosY + 40.0f;
						m_pAffectDesc->SetVisible(1);
					}
					else
					{
						sprintf(strAffectString, "%s : %s", g_pAffectTable[(unsigned char)m_pMyHuman->m_stAffect[i].Type], szVal);
						m_pAffectDesc->SetText(strAffectString, 0);
						m_pAffectDesc->m_nPosX = m_pAffectIcon[i]->m_nPosX;
						if ((float)(m_pAffectDesc->m_nWidth + 60.0f) + m_pAffectDesc->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
							m_pAffectDesc->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffectDesc->m_nWidth + 60.0f);
						m_pAffectDesc->m_nPosY = m_pAffectIcon[i]->m_nPosY + 40.0f;
						m_pAffectDesc->SetVisible(1);					
					}
				}
				else
				{
					int len = strlen(m_pAffect[i]->m_Font.m_szString);
					int test = 1;
					for (int l = 0; l < len; ++l)
					{
						if (!isdigit(m_pAffect[i]->m_Font.m_szString[l]) && m_pAffect[i]->m_Font.m_szString[l] != ' ')
							test = 0;
					}

					m_pAffect[i]->m_nPosX = m_pAffectIcon[i]->m_nPosX + (test ? -8.0f : 3.0f);
					if ((float)(m_pAffect[i]->m_nWidth + 60.0f) + m_pAffect[i]->m_nPosX > (float)g_pDevice->m_dwScreenWidth)
						m_pAffect[i]->m_nPosX = (float)g_pDevice->m_dwScreenWidth - (float)(m_pAffect[i]->m_nWidth + 60.0f);

					m_pAffect[i]->m_nPosY = m_pAffectIcon[i]->m_nPosY + 24.0f;
					m_pAffect[i]->SetVisible(1);
				}

				++nAvailCount;
				if (!m_dwAffectBlinkTime[i] && sTime <= 10 && sTime > 1)
					m_dwAffectBlinkTime[i] = dwServerTime;
				else if (!m_dwAffectBlinkTime[i] || sTime >= 10)
					m_pAffectIcon[i]->SetVisible(1);
				else
				{
					unsigned int dwDelay = 3 * nTime * nTime + 200;
					if (dwServerTime - m_dwAffectBlinkTime[i] >= dwDelay)
					{
						m_pAffectIcon[i]->SetVisible(m_pAffectIcon[i]->m_bVisible == 0);
						m_dwAffectBlinkTime[i] = dwServerTime;
					}
				}
				if (nTime >= 10)
				{
					m_pAffectIcon[i]->SetVisible(1);
					m_dwAffectBlinkTime[i] = 0;
				}
			}
		}
	}

	nAvailCount = 0;
	int YnAvailCount = 0;
	int index = 0;

	if (m_pMyHuman->m_pTitleProgressBar)
	{
		for (int i = 0; i < 32; ++i)
		{
			m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = -1;
			index = (int)m_TargetAffect[i] >> 8;
			if (index == 0)
			{
				m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = -1;
			}
			else
			{
				m_pTargetAffectIcon[i]->m_GCPanel.nTextureIndex = g_AffectSkillType[index];
				++nAvailCount;

				auto vecTitleProgressBar = m_pMyHuman->m_pTitleProgressBar->GetPos();
				m_pTargetAffectIcon[i]->m_nPosX = vecTitleProgressBar.x
					- (float)(m_pTargetAffectIcon[i]->m_nWidth * 2.0f);
				m_pTargetAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f) + m_pTargetAffectIcon[i]->m_nWidth) * (float)(nAvailCount + 1))
					+ m_pTargetAffectIcon[i]->m_nPosX;				
				m_pTargetAffectIcon[i]->m_nPosY = ((float)(m_pMyHuman->m_pTitleProgressBar->m_nHeight * 2.0f) / 3.0f) + vecTitleProgressBar.y;

				if (nAvailCount > 9)
				{
					m_pTargetAffectIcon[i]->m_nPosY = m_pTargetAffectIcon[i]->m_nPosY + m_pTargetAffectIcon[i]->m_nHeight;					
					m_pTargetAffectIcon[i]->m_nPosX = vecTitleProgressBar.x - (float)(m_pTargetAffectIcon[i]->m_nWidth * 2.0f);
					m_pTargetAffectIcon[i]->m_nPosX = ((float)(BASE_ScreenResize(1.0f)
						+ m_pTargetAffectIcon[i]->m_nWidth)
						* (float)(++YnAvailCount + 1))
						+ m_pTargetAffectIcon[i]->m_nPosX;
				}
			}

			m_pTargetAffectIcon[i]->m_GCPanel.nLayer = 29;
		}
	}

	int j = 0;
	index = 0;
	m_pPartyAffectText->SetVisible(0);
	auto PartyExit = (SButton*)m_pControlContainer->FindControl(475139);
	if (PartyExit)
	{
		PartyExit->SetVisible(1);
		float size = (((float)(m_pPartyList->m_nNumItem - m_pPartyList->m_nStartItemIndex)
			* m_pPartyList->m_nHeight)
			/ (float)m_pPartyList->m_nVisibleCount)
			+ 30.0f;

		
		auto vecPartyExitPos = PartyExit->GetPos();
		PartyExit->SetPos(vecPartyExitPos.x, size);

		m_pPartyAutoButton->SetVisible(0);
		m_pPartyAutoText->SetVisible(0);

		auto partyback = (SPanel*)m_pControlContainer->FindControl(7602196);
		if (partyback)
			partyback->SetVisible(0);
		if (m_pPartyList->m_nNumItem >= 1)
		{
			m_pPartyPanel->SetSize(m_pPartyPanel->m_nWidth,	(float)m_pPartyList->m_nNumItem * 20.0f);
		}
		else
		{
			PartyExit->SetVisible(0);
			m_pPartyAutoButton->SetVisible(1);
			m_pPartyAutoText->SetVisible(1);
			if (partyback)
				partyback->SetVisible(1);
			if (m_bAutoParty)
				m_pPartyAutoButton->m_GCPanel.bVisible = 1;
			else
				m_pPartyAutoButton->m_GCPanel.bVisible = 0;

			m_pPartyPanel->SetSize(m_pPartyPanel->m_nWidth, 50.0f);
		}
	}
	for (i = 0; i < 13; ++i)
	{
		for (j = 0; j < 32; ++j)
		{
			m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = -1;
			m_pPartyAffectIcon[i][j]->m_GCPanel.nLayer = 29;
		}
	}

	for (i = 0; i < m_pPartyList->m_nNumItem; ++i)
	{
		auto pPartyItem = (SListBoxPartyItem*)m_pPartyList->m_pItemList[i];
		auto pHuman = (TMHuman*)g_pObjectManager->GetHumanByID(pPartyItem->m_dwCharID);
		nAvailCount = 0;
		YnAvailCount = 0;

		for (j = 0; j < 32; ++j)
		{
			if (pHuman)
			{
				index = (signed int)pHuman->m_usAffect[j] >> 8;

				if (!index)
				{
					m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = -1;
				}
				else
				{
					m_pPartyAffectIcon[i][j]->m_GCPanel.nTextureIndex = g_AffectSkillType[index];
					++nAvailCount;

					auto vecPartyListPos = m_pPartyList->GetPos();
					m_pPartyAffectIcon[i][j]->m_nPosX = (float)(vecPartyListPos.x + m_pPartyList->m_nWidth)
						- 20.0f;
					m_pPartyAffectIcon[i][j]->m_nPosX = ((BASE_ScreenResize(1.0f) + m_pPartyAffectIcon[i][j]->m_nWidth)
						* (float)(nAvailCount + 1))
						+ m_pPartyAffectIcon[i][j]->m_nPosX;

					m_pPartyAffectIcon[i][j]->m_nPosY = (float)((float)((float)(i
						- m_pPartyList->m_nStartItemIndex)
						* m_pPartyList->m_nHeight)
						/ (float)m_pPartyList->m_nVisibleCount)
						+ 12.0f;

					if (nAvailCount > 10)
					{
						m_pPartyAffectIcon[i][j]->m_nPosX = (float)(vecPartyListPos.x + m_pPartyList->m_nWidth)
							- 20.0f;
						m_pPartyAffectIcon[i][j]->m_nPosX = ((BASE_ScreenResize(1.0f)
							+ m_pPartyAffectIcon[i][j]->m_nWidth)
							* (float)(++YnAvailCount + 1))
							+ m_pPartyAffectIcon[i][j]->m_nPosX;
						m_pPartyAffectIcon[i][j]->m_nPosY = (((float)(i
							- m_pPartyList->m_nStartItemIndex)
							* m_pPartyList->m_nHeight)
							/ (float)m_pPartyList->m_nVisibleCount)
							+ 22.0f;
					}
					if (m_pPartyAffectIcon[i][j]->IsOver() == 1)
					{
						m_pPartyAffectText->SetRealPos(m_pPartyAffectIcon[i][j]->m_nPosX - m_pPartyAffectIcon[i][j]->m_nWidth,
							m_pPartyAffectIcon[i][j]->m_nPosY - (float)(m_pPartyAffectIcon[i][j]->m_nHeight * 1.5f));

						m_pPartyAffectText->SetText(g_pAffectTable[index], 0);
						m_pPartyAffectText->SetVisible(1);
					}
				}
			}
		}
	}

	return 1;
}

/// Executa ciclo de automacao/macro de combate e uso de pocoes.
void TMFieldScene::GameAuto()
{
	if (!g_GameAuto)
		return;

	if (g_pCurrentScene->m_eSceneType != ESCENE_TYPE::ESCENE_FIELD)
	{
		g_GameAuto = 0;
		return;
	}

	if (m_pMyHuman->m_cHide == 1)
		return;

	unsigned int dwServerTime = g_pApp->m_pTimerManager->GetServerTime();
	if (m_dwAttackDelay)
	{
		if (dwServerTime - m_dwAttackDelay > 2000)
			m_dwAttackDelay = 0;
		m_pAutoTarget = nullptr;
		return;
	}
	if (m_dwLastLogout || m_dwLastTown || m_dwLastTeleport || m_pMyHuman->m_dwDelayDel)
	{
		m_dwAttackDelay = dwServerTime;
		m_pAutoTarget = nullptr;
		return;
	}

	int CharHp = g_pObjectManager->m_stMobData.CurrentScore.Hp;
	if (m_pMyHuman->m_cDie)
	{
		m_pAutoTarget = nullptr;
		return;
	}

	int nSX = (int)m_pMyHuman->m_vecPosition.x;
	int nSY = (int)m_pMyHuman->m_vecPosition.y;
	int CharMaxHp = g_pObjectManager->m_stMobData.CurrentScore.MaxHp;
	int CharMp = g_pObjectManager->m_stMobData.CurrentScore.Mp;
	int CharMaxMp = g_pObjectManager->m_stMobData.CurrentScore.MaxMp;
	int nMountHP = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[14], 80);
	int nMountFeed = BASE_GetItemAbility(&g_pObjectManager->m_stMobData.Equip[14], 82);
	int sIndex = g_pObjectManager->m_stMobData.Equip[14].sIndex - 2045;
	int _nEquipIdx = g_pObjectManager->m_stMobData.Equip[14].sIndex;
	if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388)
		sIndex = 336;
	else if (_nEquipIdx >= 3980 && _nEquipIdx <= 3982)
		sIndex = _nEquipIdx - 3638;
	else if (_nEquipIdx >= 3983 && _nEquipIdx <= 3985)
		sIndex = _nEquipIdx - 3641;
	else if (_nEquipIdx >= 3986 && _nEquipIdx <= 3988)
		sIndex = _nEquipIdx - 3644;

	int nMountMaxHPIndex = sIndex - 315;
	if (sIndex - 315 < 0)
		nMountMaxHPIndex = 0;

	switch (g_pObjectManager->m_stMobData.Equip[14].sIndex - 2378)
	{
	case 0:
		nMountMaxHPIndex = 18;
		break;
	case 1:
		nMountMaxHPIndex = 19;
		break;
	case 2:
		nMountMaxHPIndex = 21;
		break;
	case 3:
		nMountMaxHPIndex = 20;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		break;
	case 9:
		nMountMaxHPIndex = 20;
		break;
	case 10:
		nMountMaxHPIndex = 21;
		break;
	case 11:
		nMountMaxHPIndex = 22;
		break;
	}

	int nMountMaxHp = g_nMountHPTable[nMountMaxHPIndex];
	int CheckMountHp = 0;

	if (g_GameAuto_mountValue)
		CheckMountHp = g_GameAuto_mountValue * nMountMaxHp / 100;
	else
		CheckMountHp = 0;

	CharMaxHp = g_GameAuto_hpValue * CharMaxHp / 100;
	CharMaxMp = g_GameAuto_hpValue * CharMaxMp / 100;

	if (!(dwServerTime % 3))
	{
		if (nMountHP > 0 && nMountHP < CheckMountHp && FeedMount())
			return;

		if (CharHp < CharMaxHp && UseHPotion())
			return;

		if (m_AutoHpMp != 3 && CharMp < CharMaxMp && UseMPotion())
			return;

		if (nMountFeed > 0 && nMountFeed < 6 && FeedMount())
			return;
	}

	if ((int)m_pMyHuman->m_eMotion > 1)
		return;

	if (m_bSkillBeltSwitch && m_bSkillBeltSwitch != 1)
		m_bSkillBeltSwitch = 0;

	for (int i = 10 * m_bSkillBeltSwitch; i < 10 * m_bSkillBeltSwitch + 10; ++i)
	{
		int idxSkill = (char)g_pObjectManager->m_cShortSkill[i];
		if (idxSkill == -1)
			continue;

		if (idxSkill == 3 || idxSkill == 5 || idxSkill == 53 || idxSkill == 54 || idxSkill == 9 || idxSkill == 11 || idxSkill == 37 ||
			idxSkill == 41 || idxSkill == 43 || idxSkill == 44 || idxSkill == 45 || idxSkill == 46 || idxSkill == 64 || idxSkill == 66 ||
			idxSkill == 68 || idxSkill == 70 || idxSkill == 71 || idxSkill == 87 || idxSkill == 75 || idxSkill == 76 || idxSkill == 77 ||
			idxSkill == 81 || idxSkill == 85 || idxSkill == 89 || idxSkill == 92)
		{
			int useSkill = 1;
			for (int j = 0; j < 32; ++j)
			{
				if ((unsigned char)m_pMyHuman->m_stAffect[j].Type <= 0)
					continue;

				if (idxSkill == 64 || idxSkill == 66 || idxSkill == 68 || idxSkill == 70 || idxSkill == 71)
				{
					if (g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[j].Type] == 71 && m_pMyHuman->m_stAffect[j].Time > 3)
					{
						useSkill = 0;
						break;
					}
				}
				else if (g_AffectSkillType[(unsigned char)m_pMyHuman->m_stAffect[j].Type] == idxSkill && m_pMyHuman->m_stAffect[j].Time > 3)
				{
					useSkill = 0;
					break;
				}
			}
			if (useSkill)
			{
				int DelayTime = 0;
				if (idxSkill == 3 || idxSkill == 5 || idxSkill == 53 || idxSkill == 54 || idxSkill == 74 || idxSkill == 76 || idxSkill == 77)
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Special[1];
				else if (idxSkill == 9 || idxSkill == 11 || idxSkill == 13 || idxSkill == 15 || idxSkill == 37 || idxSkill == 86 || idxSkill == 87)
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Special[2];
				else if (idxSkill == 41 || idxSkill == 43 || idxSkill == 44 || idxSkill == 45 || idxSkill == 46 || idxSkill == 64 || idxSkill == 66 ||
					idxSkill == 68 || idxSkill == 70 || idxSkill == 71 || idxSkill == 89 || idxSkill == 90)
				{
					DelayTime = 100 * g_pSpell[idxSkill].AffectTime * g_pObjectManager->m_stMobData.CurrentScore.Special[3];
				}

				if (DelayTime + m_dwSkillLastTime[idxSkill] <= dwServerTime)
				{
					g_pObjectManager->m_cSelectShortSkill = i;
					if (SkillUse(nSX, nSY, GroundGetPickPos(), dwServerTime, 1, 0) == 1)
						return;
				}
			}
			continue;
		}
		if ((idxSkill == 56 || idxSkill == 57 || idxSkill == 58 || idxSkill == 59 || idxSkill == 60 || idxSkill == 61 || idxSkill == 62 || idxSkill == 63) &&
			m_dwSkillLastTime[idxSkill] + 80000 <= dwServerTime)
		{
			g_pObjectManager->m_cSelectShortSkill = i;
			SkillUse(nSX, nSY, GroundGetPickPos(), dwServerTime, 1, 0);
			return;
		}
	}

	if (g_GameAuto == 3)
		return;

	if ((int)m_pMyHuman->m_vecPosition.x >= 2362 && (int)m_pMyHuman->m_vecPosition.x <= 2370 &&
		(int)m_pMyHuman->m_vecPosition.y >= 3927 && (int)m_pMyHuman->m_vecPosition.y <= 3935)
		return;

	if (m_AutoPostionUse == 1 && !m_pAutoTarget)
	{
		if ((nSX != m_AutoStartPointX || nSY != m_AutoStartPointY) && BASE_GetDistance(nSX, nSY, m_AutoStartPointX, m_AutoStartPointY) <= 10)
		{
			if (dwServerTime - m_pMyHuman->m_dwOldMovePacketTime > 1000 && !m_pMyHuman->m_cDie)
			{
				m_pMyHuman->m_LastSendTargetPos = m_vecMyNext;

				MSG_Action stAction{};
				stAction.Header.ID = m_pMyHuman->m_dwID;
				stAction.PosX = m_pMyHuman->m_LastSendTargetPos.x;
				stAction.PosY = m_pMyHuman->m_LastSendTargetPos.y;
				stAction.Effect = 0;
				stAction.Header.Type = MSG_Action_Opcode;
				stAction.Speed = g_nMyHumanSpeed;
				stAction.TargetX = m_AutoStartPointX;
				stAction.TargetY = m_AutoStartPointY;

				for (int k = 0; k < 23; ++k)
					stAction.Route[k] = 0;

				g_bLastStop = stAction.Header.Type;
				m_stMoveStop.LastX = stAction.PosX;
				m_stMoveStop.LastY = stAction.PosY;
				m_stMoveStop.NextX = stAction.TargetX;
				m_stMoveStop.NextY = stAction.TargetY;
				SendOneMessage((char*)&stAction, sizeof(stAction));
				m_pMyHuman->OnPacketEvent(MSG_Action_Opcode, (char*)&stAction);
				m_pMyHuman->m_dwOldMovePacketTime = g_pTimerManager->GetServerTime();
				return;
			}
		}
	}

	if (m_pMyHuman->IsInTown() == 1)
		return;

	if (g_GameAuto == 1)
	{
		if (!m_pAutoTarget)
		{
			m_pAutoTarget = nullptr;

			auto pTarget = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pTarget && pTarget->m_pNextLink)
			{
				if (pTarget == m_pMyHuman)
					pTarget = (TMHuman*)pTarget->m_pNextLink;
				else
				{
					if (m_pMyHuman->MAutoAttack(pTarget, 0) == 1)
					{
						m_pAutoTarget = pTarget;
						return;
					}

					pTarget = (TMHuman*)pTarget->m_pNextLink;
				}
			}

			if (m_pAutoTarget || m_AutoPostionUse == 2)
				return;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pNode && pNode->m_pNextLink)
			{
				if (pNode == m_pMyHuman)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nTX = (int)pNode->m_vecPosition.x;
				int nTY = (int)pNode->m_vecPosition.y;

				if (BASE_GetDistance(nSX, nSY, nTX, nTY) > 8)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nSpecForce = 0;
				if (g_pObjectManager->m_stMobData.LearnedSkill[0] & 0x20000000)
					nSpecForce = 1;

				int nMobAttackRange = nSpecForce + BASE_GetMobAbility(&g_pObjectManager->m_stMobData, 27);
				BASE_GetHitPosition(nSX, nSY, &nTX, &nTY, (char*)g_pCurrentScene->m_HeightMapData, 8);

				if (nTX != (int)pNode->m_vecPosition.x || nTY != (int)pNode->m_vecPosition.y)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int attack = 0;
				if (pNode)
					attack = m_pMyHuman->MAutoAttack(pNode, 1);
				if (attack == 1)
				{
					m_pAutoTarget = pNode;
					return;
				}
				if (attack == 2)
					return;

				pNode = (TMHuman*)pNode->m_pNextLink;
			}

			return;
		}

		int rnt = 0;
		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		while (pNode && pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode == m_pAutoTarget)
			{
				rnt = m_pMyHuman->MAutoAttack(m_pAutoTarget, m_AutoPostionUse == 2 ? 0 : 1);
				if (m_pAutoTarget->m_cShadow == 1 && m_pAutoTarget->m_nClass == 66)
					m_pAutoTarget = nullptr;
			}

			pNode = (TMHuman*)pNode->m_pNextLink;
			if (!rnt)
				m_pAutoTarget = nullptr;
		}

		return;
	}

	if (g_GameAuto == 2)
	{
		if (m_pMyHuman->m_bSkillBlack == 1)
			return;

		if (!m_pAutoTarget)
		{
			m_pAutoTarget = nullptr;

			auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
			while (pNode && pNode->m_pNextLink)
			{
				if (pNode == m_pMyHuman)
				{
					pNode = (TMHuman*)pNode->m_pNextLink;
					continue;
				}

				int nX = (int)pNode->m_vecPosition.x;
				int nY = (int)pNode->m_vecPosition.y;

				if (AutoSkillUse(nX, nY, D3DXVECTOR3(pNode->m_vecPosition.x, pNode->m_fHeight, pNode->m_vecPosition.y), 0, 0, pNode) == 1)
				{
					m_pAutoTarget = pNode;
					return;
				}

				pNode = (TMHuman*)pNode->m_pNextLink;
			}

			return;
		}

		int CheckAtt = 0;
		auto pNode = (TMHuman*)m_pHumanContainer->m_pDown;
		while (pNode && pNode->m_pNextLink)
		{
			if (pNode == m_pMyHuman)
			{
				pNode = (TMHuman*)pNode->m_pNextLink;
				continue;
			}

			if (pNode == m_pAutoTarget)
			{
				int itx = (int)m_pAutoTarget->m_vecPosition.x;
				int ity = (int)m_pAutoTarget->m_vecPosition.y;

				if (!AutoSkillUse(itx, ity, D3DXVECTOR3(m_pAutoTarget->m_vecPosition.x, m_pAutoTarget->m_fHeight, m_pAutoTarget->m_vecPosition.y), 0, 0, m_pAutoTarget))
				{
					m_pAutoTarget = 0;
					CheckAtt = 1;
					break;
				}
			}

			pNode = (TMHuman*)pNode->m_pNextLink;
		}

		if (!CheckAtt)
			m_pAutoTarget = 0;
	}
}
