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

/// @file RenderDevice_Rect.cpp
/// @brief Primitivas 2D do RenderDevice: retângulos texturizados/coloridos, barras de
///        progresso, dano flutuante e sprites rotacionados (família `RenderRect*`).
///
/// Extraído de `RenderDevice.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

void RenderDevice::RenderRect(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, IDirect3DTexture9* pTexture, float fScaleX, float fScaleY)
{
	RECT srcRect{};
	srcRect.left = (int)iStartX;
	srcRect.top = (int)iStartY;
	srcRect.right = (int)(iStartX + iCX);
	srcRect.bottom = (int)(iStartY + iCY);

	D3DXVECTOR2 rotCenter((float)(iCX / 2.0) + iStartX, (float)(iCY / 2.0) + iStartY);
	D3DXVECTOR2 destPoint(iDestX, iDestY);
	D3DXVECTOR2 scaleVec(fScaleX, fScaleY);
	
	if (pTexture != nullptr && m_pSprite != nullptr)
	{		
		m_pSprite->Begin();
		m_pSprite->Draw(pTexture, &srcRect, &scaleVec, &rotCenter, 0, &destPoint, 0xFFFFFFFF);
		m_pSprite->End();
	}
}

void RenderDevice::RenderRectC(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, IDirect3DTexture9* pTexture, DWORD dwColor, float fScaleX, float fScaleY)
{
	m_CtrlVertex[0].position.x = iDestX;
	m_CtrlVertex[0].position.y = iDestY;
	m_CtrlVertex[0].tu = iStartX / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[0].tv = iStartY / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[1].position.x = (float)(iCX * fScaleX) + iDestX;
	m_CtrlVertex[1].position.y = iDestY;
	m_CtrlVertex[1].tu = (float)((float)(iStartX + iCX) + 0.5f) / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[1].tv = (float)(iStartY + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[2].position.x = (float)(iCX * fScaleX) + iDestX;
	m_CtrlVertex[2].position.y = (float)(iCY * fScaleY) + iDestY;
	m_CtrlVertex[2].tu = (float)((float)(iStartX + iCX) + 0.5f) / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[2].tv = (float)((float)(iStartY + iCY) + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[3].position.x = iDestX;
	m_CtrlVertex[3].position.y = (float)(iCY * fScaleY) + iDestY;
	m_CtrlVertex[3].tu = iStartX / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[3].tv = (float)((float)(iStartY + iCY) + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	SetTexture(0, pTexture);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectCoord(float iDestX, float iDestY, float iCX, float iCY, IDirect3DTexture9* pTexture, DWORD dwColor, float fU, float fV)
{
	m_CtrlVertex[0].position.x = iDestX;
	m_CtrlVertex[0].position.y = iDestY;
	m_CtrlVertex[0].tu = 0.0;
	m_CtrlVertex[0].tv = 0.0;
	m_CtrlVertex[1].position.x = iDestX + iCX;
	m_CtrlVertex[1].position.y = iDestY;
	m_CtrlVertex[1].tu = fU;
	m_CtrlVertex[1].tv = 0.0;
	m_CtrlVertex[2].position.x = iDestX + iCX;
	m_CtrlVertex[2].position.y = iDestY + iCY;
	m_CtrlVertex[2].tu = fU;
	m_CtrlVertex[2].tv = fV;
	m_CtrlVertex[3].position.x = iDestX;
	m_CtrlVertex[3].position.y = iDestY + iCY;
	m_CtrlVertex[3].tu = 0.0;
	m_CtrlVertex[3].tv = fV;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	SetTexture(0, pTexture);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectNoTex(float iX, float iY, float iCX, float iCY, DWORD dwColor, int bTrans)
{
	m_CtrlVertex[0].position.x = iX;
	m_CtrlVertex[0].position.y = iY;
	m_CtrlVertex[1].position.x = iX + iCX;
	m_CtrlVertex[1].position.y = iY;
	m_CtrlVertex[2].position.x = iX + iCX;
	m_CtrlVertex[2].position.y = iY + iCY;
	m_CtrlVertex[3].position.x = iX;
	m_CtrlVertex[3].position.y = iY + iCY;

	for (int j = 0; j < 4; ++j)
	{
		m_CtrlVertex[j].diffuse = dwColor;
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

	SetTexture(0, nullptr);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(		
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000u);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDDu);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDu);
	}
}

void RenderDevice::RenderRectTex(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 512.0f;
	float fTexHeight = 512.0f;

	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}
	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertex[0].position.x = iDestX;
		m_CtrlVertex[0].position.y = iDestY;
		m_CtrlVertex[1].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertex[1].position.y = iDestY;
		m_CtrlVertex[2].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertex[2].position.y = (float)(DestCY * fScale) + iDestY;
		m_CtrlVertex[3].position.x = iDestX;
		m_CtrlVertex[3].position.y = (float)(DestCY * fScale) + iDestY;
	}
	else
	{
		float fRX = (float)(DestCX * fScale) / 2.0f;
		float fRY = (float)(DestCY * fScale) / 2.0f;
		float fCenterX = (float)((float)((float)(DestCX * fScale) + iDestX) + iDestX) / 2.0f;
		float fCenterY = (float)((float)((float)(DestCY * fScale) + iDestY) + iDestY) / 2.0f;
		float fSin = sinf(fAngle);
		float fCos = cosf(fAngle);
		m_CtrlVertex[0].position.x = (float)((float)(-fRX * fCos)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertex[0].position.y = (float)((float)(-fRX * fSin)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[1].position.x = (float)((float)(fCos * fRX)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertex[1].position.y = (float)((float)(fSin * fRX)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[2].position.x = (float)((float)(fCos * fRX) - (float)(fSin * fRY)) + fCenterX;
		m_CtrlVertex[2].position.y = (float)((float)(fSin * fRX) + (float)(fCos * fRY)) + fCenterY;
		m_CtrlVertex[3].position.x = (float)((float)(-fRX * fCos)
			- (float)(fSin * fRY))
			+ fCenterX;
		m_CtrlVertex[3].position.y = (float)((float)(-fRX * fSin)
			+ (float)(fCos * fRY))
			+ fCenterY;
	}

	float nEndX = iStartX + iCX;
	float nEndY = iStartY + iCY;
	float fEndX = 0.0f;
	float fEndY = 0.0f;

	if (m_iVGAID == 1)
	{
		fEndX = (float)(signed int)(float)((float)(nEndX / fTexWidth) * 10000.0f) * 0.000099999997f;
		fEndY = (float)(signed int)(float)((float)(nEndY / fTexHeight) * 10000.0f) * 0.000099999997f;
	}
	else
	{
		fEndX = nEndX / fTexWidth;
		fEndY = nEndY / fTexHeight;
	}

	m_CtrlVertex[0].tu = (float)(iStartX / fTexWidth) + (float)(0.5f / fTexWidth);
	m_CtrlVertex[0].tv = (float)(iStartY / fTexHeight) + (float)(0.5f / fTexHeight);
	m_CtrlVertex[1].tu = fEndX - (float)(0.5f / fTexWidth);
	m_CtrlVertex[1].tv = (float)(iStartY / fTexHeight) + (float)(0.5f / fTexHeight);
	m_CtrlVertex[2].tu = fEndX - (float)(0.5f / fTexWidth);
	m_CtrlVertex[2].tv = fEndY - (float)(0.5f / fTexHeight);
	m_CtrlVertex[3].tu = (float)(iStartX / fTexWidth) + (float)(0.5f / fTexWidth);
	m_CtrlVertex[3].tv = fEndY - (float)(0.5f / fTexHeight);

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

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

	SetTexture(0, pTexture);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectTex2C(float iStartX, float iStartY, float iCX, float iCY, float iStartX2, float iStartY2, float iCX2, float iCY2, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}
	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertexC2[0].position.x = iDestX;
		m_CtrlVertexC2[0].position.y = iDestY;
		m_CtrlVertexC2[1].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertexC2[1].position.y = iDestY;
		m_CtrlVertexC2[2].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertexC2[2].position.y = (float)(DestCY * fScale) + iDestY;
		m_CtrlVertexC2[3].position.x = iDestX;
		m_CtrlVertexC2[3].position.y = (float)(DestCY * fScale) + iDestY;
	}
	else
	{
		float fRX = (float)(DestCX * fScale) / 2.0f;
		float fRY = (float)(DestCY * fScale) / 2.0f;
		float fCenterX = (float)((float)((float)(DestCX * fScale) + iDestX) + iDestX) / 2.0f;
		float fCenterY = (float)((float)((float)(DestCY * fScale) + iDestY) + iDestY) / 2.0f;
		float fSin = sinf(fAngle);
		float fCos = cosf(fAngle);
		m_CtrlVertexC2[0].position.x = (float)((float)(-fRX * fCos)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertexC2[0].position.y = (float)((float)(-fRX * fSin)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertexC2[1].position.x = (float)((float)(fCos * fRX)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertexC2[1].position.y = (float)((float)(fSin * fRX)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[2].position.x = (float)((float)(fCos * fRX) - (float)(fSin * fRY)) + fCenterX;
		m_CtrlVertex[2].position.y = (float)((float)(fSin * fRX) + (float)(fCos * fRY)) + fCenterY;
		m_CtrlVertexC2[3].position.x = (float)((float)(-fRX * fCos)
			- (float)(fSin * fRY))
			+ fCenterX;
		m_CtrlVertexC2[3].position.y = (float)((float)(-fRX * fSin)
			+ (float)(fCos * fRY))
			+ fCenterY;
	}

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertexC2[0].tu1 = iStartX / fTexWidth;
	m_CtrlVertexC2[0].tv1 = iStartY / fTexHeight;
	m_CtrlVertexC2[1].tu1 = fEndX;
	m_CtrlVertexC2[1].tv1 = iStartY / fTexHeight;
	m_CtrlVertexC2[2].tu1 = fEndX;
	m_CtrlVertexC2[2].tv1 = fEndY;
	m_CtrlVertexC2[3].tu1 = iStartX / fTexWidth;
	m_CtrlVertexC2[3].tv1 = fEndY;

	if (pTexture2)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	float fEndX2 = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY2 = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertexC2[0].tu2 = iStartX2 / fTexWidth;
	m_CtrlVertexC2[0].tv2 = iStartY2 / fTexHeight;
	m_CtrlVertexC2[1].tu2 = fEndX2;
	m_CtrlVertexC2[1].tv2 = iStartY2 / fTexHeight;
	m_CtrlVertexC2[2].tu2 = fEndX2;
	m_CtrlVertexC2[2].tv2 = fEndY2;
	m_CtrlVertexC2[3].tu2 = iStartX2 / fTexWidth;
	m_CtrlVertexC2[3].tv2 = fEndY2;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertexC2[j].diffuse = dwColor;

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

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTex2(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_CtrlVertex2[0].position.x = iDestX;
	m_CtrlVertex2[0].position.y = iDestY;
	m_CtrlVertex2[1].position.x = (float)(DestCX * fScale) + iDestX;
	m_CtrlVertex2[1].position.y = iDestY;
	m_CtrlVertex2[2].position.x = (float)(DestCX * fScale) + iDestX;
	m_CtrlVertex2[2].position.y = (float)(DestCY * fScale) + iDestY;
	m_CtrlVertex2[3].position.x = iDestX;
	m_CtrlVertex2[3].position.y = (float)(DestCY * fScale) + iDestY;

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f)	* 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertex2[0].tu2 = iStartX / fTexWidth;
	m_CtrlVertex2[0].tv2 = iStartY / fTexHeight;
	m_CtrlVertex2[1].tu2 = fEndX;
	m_CtrlVertex2[1].tv2 = iStartY / fTexHeight;
	m_CtrlVertex2[2].tu2 = fEndX;
	m_CtrlVertex2[2].tv2 = fEndY;
	m_CtrlVertex2[3].tu2 = iStartX / fTexWidth;
	m_CtrlVertex2[3].tv2 = fEndY;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex2[j].diffuse = dwColor;

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

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTex2M(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_MiniMapVertex2[0].position.x = iDestX;
	m_MiniMapVertex2[0].position.y = iDestY;
	m_MiniMapVertex2[1].position.x = (float)(DestCX * fScale) + iDestX;
	m_MiniMapVertex2[1].position.y = iDestY;
	m_MiniMapVertex2[2].position.x = (float)(DestCX * fScale) + iDestX;
	m_MiniMapVertex2[2].position.y = (float)(DestCY * fScale) + iDestY;
	m_MiniMapVertex2[3].position.x = iDestX;
	m_MiniMapVertex2[3].position.y = (float)(DestCY * fScale) + iDestY;

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_MiniMapVertex2[0].tu2 = iStartX / fTexWidth;
	m_MiniMapVertex2[0].tv2 = iStartY / fTexHeight;
	m_MiniMapVertex2[1].tu2 = fEndX;
	m_MiniMapVertex2[1].tv2 = iStartY / fTexHeight;
	m_MiniMapVertex2[2].tu2 = fEndX;
	m_MiniMapVertex2[2].tv2 = fEndY;
	m_MiniMapVertex2[3].tu2 = iStartX / fTexWidth;
	m_MiniMapVertex2[3].tv2 = fEndY;

	for (int j = 0; j < 4; ++j)
		m_MiniMapVertex2[j].diffuse = dwColor;

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

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTexDamage(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_CtrlVertex[0].position.x = iDestX - (float)(DestCX * (float)(fScale * 0.5));
	m_CtrlVertex[0].position.y = iDestY - (float)(DestCY * (float)(fScale * 0.5));
	m_CtrlVertex[1].position.x = (float)(DestCX * (float)(fScale * 0.5)) + iDestX;
	m_CtrlVertex[1].position.y = iDestY - (float)(DestCY * (float)(fScale * 0.5));
	m_CtrlVertex[2].position.x = (float)(DestCX * (float)(fScale * 0.5)) + iDestX;
	m_CtrlVertex[2].position.y = (float)(DestCY * (float)(fScale * 0.5)) + iDestY;
	m_CtrlVertex[3].position.x = iDestX - (float)(DestCX * (float)(fScale * 0.5));
	m_CtrlVertex[3].position.y = (float)(DestCY * (float)(fScale * 0.5)) + iDestY;

	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertex[0].tu = iStartX / fTexWidth;
		m_CtrlVertex[0].tv = iStartY / fTexHeight;
		m_CtrlVertex[1].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[1].tv = iStartY / fTexHeight;
		m_CtrlVertex[2].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[2].tv = (float)(iStartY + iCY) / fTexHeight;
		m_CtrlVertex[3].tu = iStartX / fTexWidth;
		m_CtrlVertex[3].tv = (float)(iStartY + iCY) / fTexHeight;
	}
	else
	{
		m_CtrlVertex[3].tu = iStartX / fTexWidth;
		m_CtrlVertex[3].tv = (float)((float)(iCY / 2.0) + iStartY) / fTexHeight;
		m_CtrlVertex[0].tu = (float)((float)(iCX / 2.0) + iStartX) / fTexWidth;
		m_CtrlVertex[0].tv = iStartY / fTexHeight;
		m_CtrlVertex[1].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[1].tv = (float)((float)(iCY / 2.0) + iStartY) / fTexHeight;
		m_CtrlVertex[2].tu = (float)((float)(iCX / 2.0) + iStartX) / fTexWidth;
		m_CtrlVertex[2].tv = (float)(iStartY + iCY) / fTexHeight;
	}

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

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

	SetTexture(0, pTexture);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);

	if (m_dwBitCount == 32)
		SetRenderState(D3DRS_ALPHAREF, 0xAA);
	else
		SetRenderState(D3DRS_ALPHAREF, 0xFF);
}

void RenderDevice::RenderRectProgress2(float iX, float iY, float iCX, float iCY, float fProgress, DWORD dwColor)
{
	if (fProgress > 0.0)
	{
		if (m_iVGAID == 1)
		{
			if (m_dwBitCount == 32)
				SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				SetRenderState(D3DRS_ALPHAREF, 0xF000u);
		}
		else if (m_dwBitCount == 32)
		{
			SetRenderState(D3DRS_ALPHAREF, 0xDDu);
		}
		else
		{
			SetRenderState(D3DRS_ALPHAREF, 0xDu);
		}

		SetTexture(0, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);

		m_pd3dDevice->SetFVF(324);
		float fSX = (float)(iCX / 2.0f) + iX;
		float fSY = (float)(iCY / 2.0f) + iY;
		float fWidth = iCX / 2.0f;
		float fHeight = iCY / 2.0f;
		for (int i = 0; i < 10; ++i)
			m_CtrlProgressVertex[i].diffuse = dwColor;

		if (fProgress <= 0.125f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = (float)(fWidth * (float)(fProgress / 0.125f)) + fSX;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				1u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.25f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight)
				+ (float)(fHeight * (float)((float)(fProgress - 0.125f) / 0.125f));
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				2u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.375f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = (float)(fHeight * (float)((float)(fProgress - 0.25f) / 0.125f)) + fSY;
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				3u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.5f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = fSY + fHeight;
			m_CtrlProgressVertex[5].position.x = (float)(fSX + fWidth)
				- (float)(fWidth * (float)((float)(fProgress - 0.375f) / 0.125f));
			m_CtrlProgressVertex[5].position.y = fSY + fHeight;
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				4u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.625f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = fSY + fHeight;
			m_CtrlProgressVertex[5].position.x = fSX;
			m_CtrlProgressVertex[5].position.y = fSY + fHeight;
			m_CtrlProgressVertex[6].position.x = fSX - (float)(fWidth * (float)((float)(fProgress - 0.5f) / 0.125f));
			m_CtrlProgressVertex[6].position.y = fSY + fHeight;
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				5u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.75f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = fSY + fHeight;
			m_CtrlProgressVertex[5].position.x = fSX;
			m_CtrlProgressVertex[5].position.y = fSY + fHeight;
			m_CtrlProgressVertex[6].position.x = fSX - fWidth;
			m_CtrlProgressVertex[6].position.y = fSY + fHeight;
			m_CtrlProgressVertex[7].position.x = fSX - fWidth;
			m_CtrlProgressVertex[7].position.y = (float)(fSY + fHeight)
				- (float)(fHeight * (float)((float)(fProgress - 0.625f) / 0.125f));
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				6u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 0.875f)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = fSY + fHeight;
			m_CtrlProgressVertex[5].position.x = fSX;
			m_CtrlProgressVertex[5].position.y = fSY + fHeight;
			m_CtrlProgressVertex[6].position.x = fSX - fWidth;
			m_CtrlProgressVertex[6].position.y = fSY + fHeight;
			m_CtrlProgressVertex[7].position.x = fSX - fWidth;
			m_CtrlProgressVertex[7].position.y = fSY;
			m_CtrlProgressVertex[8].position.x = fSX - fWidth;
			m_CtrlProgressVertex[8].position.y = fSY - (float)(fHeight * (float)((float)(fProgress - 0.75f) / 0.125f));
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				7u,
				m_CtrlProgressVertex,
				28u);
		}
		else if (fProgress <= 1.0)
		{
			m_CtrlProgressVertex[0].position.x = fSX;
			m_CtrlProgressVertex[0].position.y = fSY;
			m_CtrlProgressVertex[1].position.x = fSX;
			m_CtrlProgressVertex[1].position.y = fSY - fHeight;
			m_CtrlProgressVertex[2].position.x = fSX + fWidth;
			m_CtrlProgressVertex[2].position.y = fSY - fHeight;
			m_CtrlProgressVertex[3].position.x = fSX + fWidth;
			m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
			m_CtrlProgressVertex[4].position.x = fSX + fWidth;
			m_CtrlProgressVertex[4].position.y = fSY + fHeight;
			m_CtrlProgressVertex[5].position.x = fSX;
			m_CtrlProgressVertex[5].position.y = fSY + fHeight;
			m_CtrlProgressVertex[6].position.x = fSX - fWidth;
			m_CtrlProgressVertex[6].position.y = fSY + fHeight;
			m_CtrlProgressVertex[7].position.x = fSX - fWidth;
			m_CtrlProgressVertex[7].position.y = fSY;
			m_CtrlProgressVertex[8].position.x = fSX - fWidth;
			m_CtrlProgressVertex[8].position.y = fSY - fHeight;
			m_CtrlProgressVertex[9].position.x = (float)(fSX - fWidth)
				+ (float)(fWidth * (float)((float)(fProgress - 0.875f) / 0.125f));
			m_CtrlProgressVertex[9].position.y = fSY - fHeight;
			m_pd3dDevice->DrawPrimitiveUP(
				D3DPT_TRIANGLEFAN,
				8u,
				m_CtrlProgressVertex,
				28u);
		}
	}
}

void RenderDevice::RenderRectRot(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float nCenX, float nCenY, float fAngle, IDirect3DTexture9* pTexture, float fScaleX, float fScaleY)
{
	RECT srcRect{};
	srcRect.left = (int)iStartX;
	srcRect.top = (int)iStartY;
	srcRect.right = (int)(iStartX + iCX);
	srcRect.bottom = (int)(iStartY + iCY);

	D3DXVECTOR2 rotCenter(nCenX, nCenY);
	D3DXVECTOR2 destPoint(iDestX, iDestY);
	D3DXVECTOR2 scaleVec(fScaleX, fScaleY);

	if (pTexture != nullptr && m_pSprite != nullptr)
	{
		m_pSprite->Begin();
		m_pSprite->Draw(pTexture, &srcRect, &scaleVec, &rotCenter, -fAngle, &destPoint, 0xFFFFFFFF);
		m_pSprite->End();
	}
}
