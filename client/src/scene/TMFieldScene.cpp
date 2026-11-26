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
