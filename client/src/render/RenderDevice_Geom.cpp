#include "core/pch.h"
#include "render/GeomObject.h"
#include "render/TextureManager.h"
#include "render/MeshManager.h"
#include "render/TMFont2.h"
#include "render/RenderDevice.h"
#include "core/TMGlobal.h"
#include "core/TMLog.h"
#include "world/TMSky.h"
#include "scene/TMFieldScene.h"
#include "render/TMCamera.h"
#include "render/TMMesh.h"
#include "common/Basedef.h"
#include <io.h>
#include <fcntl.h>
#include <winnt.h>

/// @file RenderDevice_Geom.cpp
/// @brief Renderização dos controles de UI baseados em geometria (`GeomControl`):
///        imagens recortadas por conjunto de textura, controles e fundos.
///
/// Extraído de `RenderDevice.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

void RenderDevice::RenderGeomRectImage(GeomControl* ipControl)
{
	if (ipControl->nTextureSetIndex >= 0)
	{
		ControlTextureSet* pUISet = g_pTextureManager->GetUITextureSet(ipControl->nTextureSetIndex);

		if (pUISet == nullptr)
			return;

		if (pUISet->nCount < ipControl->nTextureIndex)
			return;

		if (pUISet->pTextureCoord == nullptr)
			return;

		LPDIRECT3DTEXTURE9 pTexture = g_pTextureManager->GetUITexture(pUISet->pTextureCoord[ipControl->nTextureIndex].nTextureIndex, 2000);
		if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE)
		{
			RenderRect(
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + (float)ipControl->nPosX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + (float)ipControl->nPosY,
				pTexture,
				1.0f,
				1.0f);
		}
		else if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
		{
			float fScaleX = ipControl->nWidth / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth;
			float fScaleY = ipControl->nHeight / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight;

			if (ipControl->bClip == 1)
			{
				RenderRectRot(					
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY + ipControl->fTop,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
					ipControl->fBottom - ipControl->fTop,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
					(float)((float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY) + ipControl->fTop,
					ipControl->nWidth / 2.0f,
					ipControl->nHeight / 2.0f,
					ipControl->fAngle,
					pTexture,
					fScaleX,
					fScaleY);
				return;
			}

			SetRenderStateBlock(0);

			if (ipControl->sSanc > 0)
			{
				float fLayoutScaleX = BASE_ScreenResize(2.0f) + ipControl->nWidth;
				float fLayoutScaleY = BASE_ScreenResize(2.0f) + ipControl->nHeight;
				float fLayoutPosX = BASE_ScreenResize(1.0f);
				float fLayoutPosY = BASE_ScreenResize(1.0f);
				float lev = (float)((float)ipControl->sSanc - 1.0f) * 35.0f;
				LPDIRECT3DTEXTURE9 pEfTexture = g_pTextureManager->GetUITexture(338, 2000);
				if (pEfTexture != nullptr)
				{
					RenderRectRot(						
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						fScaleX,
						fScaleY);
				}
			}
			if (ipControl->sLegend > 0)
			{
				float fLayoutScaleX = BASE_ScreenResize(2.0f) + ipControl->nWidth;
				float fLayoutScaleY = BASE_ScreenResize(2.0f) + ipControl->nHeight;
				float fLayoutPosX = BASE_ScreenResize(1.0f);
				float fLayoutPosY = BASE_ScreenResize(1.0f);
				float fStartX = 0.0f;
				float fStartY = 0.0f;

				LPDIRECT3DTEXTURE9 pEfTexture = g_pTextureManager->GetUITexture(338, 2000);
				switch (ipControl->sLegend)
				{
				case 5:
				case 9:
					fStartX = 315.0f;
					break;
				case 6:
				case 10:
					fStartX = 350.0f;
					break;
				case 7:
				case 11:
					fStartX = 385.0f;
					break;
				case 8:
				case 12:
					fStartX = 420.0f;
					break;
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
				case 30:
				case 31:
				case 32:
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
				case 38:
				case 39:
				case 40:
				case 41:
				case 42:
				case 43:
				case 44:
				case 45:
				case 46:
				case 47:
				case 48:
				case 49:
				case 50:
				case 51:
				case 52:
				case 53:
				case 54:
				case 55:
				case 56:
				case 57:
				case 58:
				case 59:
				case 60:
				case 61:
				case 62:
				case 63:
				case 64:
				case 65:
				case 66:
				case 67:
				case 68:
				case 69:
				case 70:
				case 71:
				case 72:
				case 73:
				case 74:
				case 75:
				case 76:
				case 77:
				case 78:
				case 79:
				case 80:
				case 81:
				case 82:
				case 83:
				case 84:
				case 85:
				case 86:
				case 87:
				case 88:
				case 89:
				case 90:
				case 91:
				case 92:
				case 93:
				case 94:
				case 95:
				case 96:
				case 97:
				case 98:
				case 99:
				case 100:
				case 101:
				case 102:
				case 103:
				case 104:
				case 105:
				case 106:
				case 107:
				case 108:
				case 109:
				case 110:
				case 111:
				case 112:
				case 113:
				case 114:
				case 115:
					pEfTexture = nullptr;
					break;
				case 116:
					fStartX = 0.0;
					fStartY = 35.0f;
					break;
				case 117:
					fStartX = 35.0f;
					fStartY = 35.0f;
					break;
				case 118:
					fStartX = 70.0f;
					fStartY = 35.0f;
					break;
				case 119:
					fStartX = 105.0f;
					fStartY = 35.0f;
					break;
				case 120:
					fStartX = 140.0f;
					fStartY = 35.0f;
					break;
				case 121:
					fStartX = 175.0f;
					fStartY = 35.0f;
					break;
				case 122:
					fStartX = 210.0f;
					fStartY = 35.0f;
					break;
				case 123:
					fStartX = 245.0f;
					fStartY = 35.0f;
					break;
				case 124:
					fStartX = 280.0f;
					fStartY = 35.0f;
					break;
				case 125:
					fStartX = 315.0f;
					fStartY = 35.0f;
					break;
				}

				if (pEfTexture != nullptr)
				{
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						fScaleX,
						fScaleY);
				}				
			}
			RenderDevice::RenderRectRot(
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
				ipControl->nWidth / 2.0f,
				ipControl->nHeight / 2.0f,
				ipControl->fAngle,
				pTexture,
				fScaleX,
				fScaleY);
		}	
		else if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE_TILE)
		{
			RenderRectCoord(			
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
				ipControl->nWidth,
				ipControl->nHeight,
				pTexture,
				ipControl->dwColor,
				ipControl->nWidth / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
				ipControl->nHeight / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight);
		}
	}
	else if (ipControl->nTextureSetIndex < -2)
	{
		ControlTextureSet* pUISet = g_pTextureManager->GetUITextureSet(-ipControl->nTextureSetIndex);
		if (pUISet != nullptr)
		{
			if (pUISet->pTextureCoord == nullptr)
				return;

			LPDIRECT3DTEXTURE9 pTexture = g_pTextureManager->GetUITexture(pUISet->pTextureCoord[ipControl->nTextureIndex].nTextureIndex,
				 2000);

			if (pTexture != nullptr)
			{
				RenderRectTex(
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
					ipControl->nWidth,
					ipControl->nHeight,
					pTexture,
					ipControl->dwColor,
					1,
					ipControl->fAngle,
					1.0);
			}

			return;
		}		
	}
	else
	{
		int nMarkIndex = ipControl->nMarkIndex;
		if (nMarkIndex >= 0 && nMarkIndex <= 64)
		{
			if (g_pTextureManager->m_stGuildMark[nMarkIndex].pTexture)
			{
				float iX = ipControl->nPosX - 2.0f;
				float iY = ipControl->nPosY - 2.0f;
				float iCX = 2.0f + 18.0f;
				float iCY = 2.0f + 14.0f;
				float fWidthRatio = RenderDevice::m_fWidthRatio;
				float fHeightRatio = RenderDevice::m_fHeightRatio;
				if (RenderDevice::m_fWidthRatio != 1.0f)
				{
					iCX = RenderDevice::m_fWidthRatio * 18.5f;
					iCY = RenderDevice::m_fHeightRatio * 14.5f;
				}
				if (RenderDevice::m_fHeightRatio == 0.8f)
				{
					iX = iX - 0.0f;
					iY = iY + 1.0f;
				}
				else if (RenderDevice::m_fHeightRatio == 1.0f)
				{
					iX = iX + 0.0f;
					iY = iY + 0.0f;
				}
				else if (RenderDevice::m_fWidthRatio == 1.6f)
				{
					iX = iX - 1.0f;
					iY = iY + 0.0f;
				}
				else if (RenderDevice::m_fWidthRatio == 2.0f)
				{
					iX = iX - 1.0f;
					iY = iY + 0.0f;
				}
				if (ipControl->nMarkLayout == 1)
				{
					RenderRectNoTex(
						iX - (float)((float)(16.0f * RenderDevice::m_fWidthRatio) - 16.0f),
						iY - (float)((float)(12.0f * RenderDevice::m_fHeightRatio) - 12.0f),
						iCX,
						iCY,
						0xFFFFD700,
						1);
				}
				else if (ipControl->nMarkLayout == 2)
				{
					RenderRectNoTex(
						iX - (float)((float)(16.0f * RenderDevice::m_fWidthRatio) - 16.0f),
						iY - (float)((float)(12.0f * RenderDevice::m_fHeightRatio) - 12.0f),
						iCX,
						iCY,
						0xFFC0C0C0,
						1);
				}

				RenderRect(
					0.0,
					0.0,
					16.0,
					12.0,
					ipControl->nPosX - (float)((float)(16.0f * fWidthRatio) - 16.0f),
					ipControl->nPosY - (float)((float)(12.0f * fHeightRatio) - 12.0f),
					g_pTextureManager->m_stGuildMark[nMarkIndex].pTexture,
					fWidthRatio,
					fHeightRatio);
				g_pTextureManager->m_stGuildMark[nMarkIndex].dwLastRenderTime = timeGetTime();
			}
		}
		else
		{
			int bTrans = 0;
			if (ipControl->nTextureSetIndex == -2)
				bTrans = 1;
			RenderRectNoTex(
				ipControl->nPosX,
				ipControl->nPosY,
				ipControl->nWidth,
				ipControl->nHeight,
				ipControl->dwColor,
				bTrans);
		}
	}
}

void RenderDevice::RenderGeomControl(GeomControl* ipControl)
{
	if (ipControl != nullptr && ipControl->bVisible == 1)
	{
		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
		}

		SetRenderStateBlock(0);

		switch (ipControl->eRenderType)
		{
		case RENDERCTRLTYPE::RENDER_TEXT:
		case RENDERCTRLTYPE::RENDER_SHADOW:
			if (ipControl->pFont)
			{
				int nUp = 0;
				if (m_dwScreenWidth == 640)
					nUp = -1;
				ipControl->pFont->Render(
					(int)ipControl->nPosX,
					(int)ipControl->nPosY + nUp,
					(int)ipControl->eRenderType);
			}
			break;
		case RENDERCTRLTYPE::RENDER_IMAGE:
		case RENDERCTRLTYPE::RENDER_IMAGE_TILE:
		case RENDERCTRLTYPE::RENDER_IMAGE_STRETCH:
		case RENDERCTRLTYPE::RENDER_TEXT_FOCUS:
			RenderGeomRectImage(ipControl);
			if (ipControl->strString[0])
			{
				int nLength = strlen(ipControl->strString);
				if (nLength > 0 && nLength < 64 && ipControl->pFont)
				{
					int nUp = 0;
					if (m_dwScreenWidth == 640)
						nUp = -1;
					if (ipControl->nTextureSetIndex == 526)
					{
						ipControl->pFont->Render(
							(int)(float)((float)(ipControl->nWidth - (float)(7 * nLength)) + ipControl->nPosX),
							nUp + (int)(float)((float)(ipControl->nHeight - 12.0) + ipControl->nPosY),
							(int)ipControl->eRenderType);
					}
					else
					{
						ipControl->pFont->Render(
							(int)(float)((float)((float)(ipControl->nWidth - (float)(6 * nLength)) / 2.0f) + ipControl->nPosX),
							nUp + (int)(float)((float)((float)(ipControl->nHeight - 12.0f) / 2.0f) + ipControl->nPosY),
							(int)ipControl->eRenderType);
					}
				}
			}
			break;
		case RENDERCTRLTYPE::RENDER_3DOBJ:
			if (ipControl->n3DObjIndex < 737 || ipControl->n3DObjIndex > 739)
			{
				SetRenderStateBlock(1);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 8);
				SetRenderState(D3DRS_FOGENABLE, 0);
				SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
				SetRenderState(D3DRS_ALPHATESTENABLE, 0);

				TMMesh *pMesh = g_pMeshManager->GetCommonMesh(ipControl->n3DObjIndex, 0, 3_min);
				float fScale = 1.0f / RenderDevice::m_fWidthRatio;
				if (ipControl->dwBGColor == -1)
					fScale = fScale * 0.80000001f;

				fScale = fScale * 0.89999998f;

				if (ipControl->n3DObjIndex >= 937 && ipControl->n3DObjIndex <= 946
					|| ipControl->n3DObjIndex >= 300 && ipControl->n3DObjIndex <= 303)
				{
					fScale = 0.5f;
				}

				fScale = BASE_ScreenResize(fScale);

				if (pMesh)
				{
					pMesh->RenderForUI((int)ipControl->nPosX, (int)ipControl->nPosY, ipControl->fAngle, ipControl->fScale * fScale,
						ipControl->dwColor, ipControl->nTextureIndex, ipControl->nTextureSetIndex, ipControl->sLegend);
				}
				if (ipControl->dwBGColor)
				{
					int nTexIndex = 123;
					if (ipControl->dwBGColor == 0xFFFFFFAA)
						nTexIndex = 56;
					if (ipControl->dwBGColor == 0xFFFFFFFF)
						nTexIndex = 273;
					if (ipControl->dwBGColor == 0xFF8800BB)
						nTexIndex = 122;
					if (ipControl->dwBGColor == 0xFF444488)
						nTexIndex = 122;
					if (ipControl->dwBGColor == 0xFFFF0000)
						nTexIndex = 56;

					RenderGeomControlBG(ipControl, ipControl->dwBGColor, nTexIndex);
				}

				SetRenderState(D3DRS_FOGENABLE, m_bFog);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 4);				
			}
			else
			{
				unsigned int dwColor[3] = { 0xFFFF4400, 0xFF3366FF, 0xFF00FF00 };
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 8u);
				g_pDevice->SetRenderState(D3DRS_LIGHTING, 0);
				g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
				g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
				g_pDevice->SetRenderState(D3DRS_SRCBLEND, 5u);
				g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
				SetRenderState(D3DRS_FOGENABLE, 0);

				float fScale = 1.0f / RenderDevice::m_fWidthRatio;
				TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipControl->n3DObjIndex, 1, 3_min);

				if (pMesh)
				{
					D3DVERTEXBUFFER_DESC vDesc;
					if (SUCCEEDED(pMesh->m_pVB->GetDesc(&vDesc)))
					{
						RDLVERTEX* pVertex;
						pMesh->m_pVB->Lock(0, 0, (void**)&pVertex, 0);

						int nCount = vDesc.Size / 24;
						for (int i = 0; i < nCount; ++i)
							pVertex[i].diffuse = ipControl->n3DObjIndex - 2964;

						pMesh->m_pVB->Unlock();

						pMesh->m_nTextureIndex[0] = 204;

						pMesh->RenderForUI((int)ipControl->nPosX, (int)ipControl->nPosY, ipControl->fAngle, ipControl->fScale * fScale,
							ipControl->dwColor, ipControl->nTextureIndex, ipControl->nTextureSetIndex, ipControl->sLegend);
					}
				}

				SetRenderState(D3DRS_FOGENABLE, m_bFog);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 4);
			}
			break;	
		}

		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
		}
		return;
	}
}

void RenderDevice::RenderGeomControlBG(GeomControl* ipControl, DWORD dwColor, int nTextureIndex)
{
	if (m_dwStartTime == 0)
		m_dwStartTime = g_pTimerManager->GetServerTime();

	float fCurrRatio = (float)((g_pTimerManager->GetServerTime() - m_dwStartTime) % 2000);
	fCurrRatio = (((fCurrRatio * 0.0005f) - 0.5f) * 2.0f) * D3DXToRadian(180);
	fCurrRatio = (fCurrRatio * 0.34f) + 0.64f;

	unsigned int dwCurrColor = WYD_RGBA((unsigned int)((float)WYDCOLOR_RED(dwColor) * fCurrRatio), (unsigned int)((float)WYDCOLOR_GREEN(dwColor) * fCurrRatio),
		(unsigned int)((float)WYDCOLOR_BLUE(dwColor) * fCurrRatio), (unsigned int)((float)WYDCOLOR_ALPHA(dwColor) * fCurrRatio));

	RDTLVERTEX v[4]{};
	for (int i = 0; i < 4; ++i)
	{
		v[i].rhw = 1.0f;
		v[i].diffuse = dwCurrColor;
	}

	float fX = ipControl->nPosX;
	float fY = ipControl->nPosY;
	float fHalfWidth = (float)(ipControl->m_fWidth * 0.5f) * ipControl->fScale;
	float fHalfHeight = (float)(ipControl->m_fHeight * 0.5f) * ipControl->fScale;

	if (dwColor == -1)
	{
		fHalfWidth = fHalfWidth * 2.0f;
		fHalfHeight = fHalfHeight * 2.0f;
	}

	v[0].position.x = fX - fHalfWidth;
	v[0].position.y = fY - fHalfHeight;
	v[0].position.z = 0.1f;
	v[0].tu = 0.0f;
	v[0].tv = 0.0f;
	v[1].position.x = fX + fHalfWidth;
	v[1].position.y = fY - fHalfHeight;
	v[1].position.z = 0.1f;
	v[1].tu = 0.1f;
	v[1].tv = 0.0f;
	v[2].position.x = fX - fHalfWidth;
	v[2].position.y = fY + fHalfHeight;
	v[2].position.z = 0.1f;
	v[2].tu = 0.0f;
	v[2].tv = 1.0f;
	v[3].position.x = fX + fHalfWidth;
	v[3].position.y = fY + fHalfHeight;
	v[3].position.z = 0.1f;
	v[3].tu = 1.0f;
	v[3].tv = 1.0f;

	SetTexture(0, g_pTextureManager->GetEffectTexture(nTextureIndex, 5000));
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	SetRenderState(D3DRS_SRCBLEND, 5);
	SetRenderState(D3DRS_DESTBLEND, 2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, 28);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
}
