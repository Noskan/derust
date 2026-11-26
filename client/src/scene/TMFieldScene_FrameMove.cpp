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

/// @file TMFieldScene_FrameMove.cpp
/// @brief Atualização por quadro da cena de campo (`FrameMove`) e temporizadores associados
///        (`TimeDelay`, `PGTVisible`, efeito de morte do Khepra).
///
/// Extraído de `TMFieldScene.cpp` na Fase 2 (Onda 4) sem alteração de comportamento.

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

	FrameMove_CPanel(dwServerTime, pCPanel);

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

/// Atualiza o painel `m_pCPanel` quando visível.
/// @note Extraído de `TMFieldScene::FrameMove` (Fase 3); locais da função passadas por referência.
void TMFieldScene::FrameMove_CPanel(unsigned int& dwServerTime, SPanel*& pCPanel)
{
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
