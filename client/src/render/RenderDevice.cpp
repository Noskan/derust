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

int RenderDevice::m_nBright = 50;
DWORD RenderDevice::m_dwCurrScreenX = 1024;
DWORD RenderDevice::m_dwCurrScreenY = 768;
DWORD RenderDevice::m_dwCurrBpp = 32;
DWORD RenderDevice::m_dwCurrRefreshRate = 60;
int RenderDevice::m_nFontSize = 12;
int RenderDevice::m_nLargeFontSize = 40;
int RenderDevice::m_nFontTextureSize = 512;
int RenderDevice::m_nFontTextureSizeY = 64;
float RenderDevice::m_fWidthRatio = 1.0f;
float RenderDevice::m_fHeightRatio = 1.0f;
float RenderDevice::m_fFOVY = 0.25f;
int RenderDevice::m_bCameraRot = 1;
int RenderDevice::m_bDungeon = 0;

RenderDevice::RenderDevice(DWORD dwScreenWidth, DWORD dwScreenHeight, DWORD dwBitCount, int bFullScreen)
{
	m_pOldRenderTarget = nullptr;
	m_pOldDepthStencil = nullptr;
	m_pSprite = nullptr;
	m_pFont = nullptr;
	m_pDXFont = nullptr;
	m_pDXFontLarge = nullptr;
	m_pTextureManager = nullptr;
	m_pMeshManager = nullptr;

	m_dwBufferUsage = 24;
	m_d3dEnumeration.AppUsesDepthBuffer = 1;
	m_d3dEnumeration.AppUsesMixedVP = 1;
	m_dwClearColor = 0;
	m_dwScreenWidth = dwScreenWidth;
	m_dwScreenHeight = dwScreenHeight;
	m_dwBitCount = dwBitCount;
	m_bFull = bFullScreen;
	m_nWidthShift = 0;
	m_nHeightShift = 0;
	m_bLoadMeshManager = 0;
	m_fFPS = 0.0f;
	m_fFogStart = 60.0f;
	m_fFogEnd = 156.0f;
	m_dwStartTime = 0;
	m_bFog = 1;
	m_bShowEffects = 1;
	m_bSupportPS11 = 0;
	m_bSupportVS20 = 0;

	RenderDevice::m_fWidthRatio = (float)m_dwScreenWidth / 800.0f;
	RenderDevice::m_fHeightRatio = (float)m_dwScreenHeight / 600.0f;

	m_bSavage = 0;

	RenderDevice::m_nLargeFontSize = (int)((float)RenderDevice::m_nLargeFontSize * RenderDevice::m_fWidthRatio);

	memset(m_dwRenderStateList, -1, sizeof(m_dwRenderStateList));
	memset(&m_hbmBitmap, 0, sizeof(m_hbmBitmap));
	memset(&m_hDC, 0, sizeof(m_hDC));
	memset(&m_hFont, 0, sizeof(m_hFont));
	memset(&m_bmi, 0, sizeof(m_bmi));
	memset(&m_pBitmapBits, 0, sizeof(m_pBitmapBits));

	for (int nStage = 0; nStage < 8; ++nStage)
	{
		memset(m_dwTextureStageStateList[nStage], 0, sizeof(m_dwTextureStageStateList[nStage]));
		m_pTexture[nStage] = nullptr;
	}

	for (int nStagea = 0; nStagea < 8; ++nStagea)
	{
		memset(m_dwSamplerStateList[nStagea], -1, sizeof(m_dwSamplerStateList[nStagea]));
	}

	for (int i = 0; i < 2; ++i)
	{
		memset(&m_light[i], 0, sizeof(m_light[i]));
		m_light[i].Type = D3DLIGHTTYPE::D3DLIGHT_DIRECTIONAL;
		m_light[i].Ambient.r = 0.0f;
		m_light[i].Ambient.g = 0.0f;
		m_light[i].Ambient.b = 0.0f;

		if (i == 0)
		{
			m_light[0].Direction = D3DXVECTOR3(-10.0f, 10.0f, -6.0f);
			
			D3DXVECTOR3 vecDir(m_light[0].Direction.x, m_light[0].Direction.y, m_light[0].Direction.z);
			D3DXVec3Normalize((D3DXVECTOR3*)&m_light[0].Direction, &vecDir);
		}
		else if (i == 1)
		{
			m_light[1].Direction = D3DXVECTOR3(10.0f, -14.0f, 6.0f);

			D3DXVECTOR3 vecDir(m_light[1].Direction.x, m_light[1].Direction.y, m_light[1].Direction.z);
			D3DXVec3Normalize((D3DXVECTOR3*)&m_light[1].Direction, &vecDir);
		}
	}

	for (int ia = 0; ia < 8; ++ia)
	{
		m_pVertexShader[ia] = nullptr;
		m_pVertexDeclaration[ia] = nullptr;
	}

	for (int ib = 0; ib < 4; ++ib)
	{
		m_pVSEffect[ib] = nullptr;
		m_pVDEffect[ib] = nullptr;
	}

	for (int ic = 0; ic < 6; ++ic)
	{
		m_pPSEffect[ic] = nullptr;
	}

	m_colorLight.r = 1.0f;
	m_colorLight.g = 1.0f;
	m_colorLight.b = 1.0f;
	m_colorBackLight.r = 0.69f;
	m_colorBackLight.g = 0.69f;
	m_colorBackLight.b = 0.69f;

	m_vPickRayDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_vPickRayOrig = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	// TODO: add a throw here if the g_pDevice is not null
	g_pDevice = this;
}

RenderDevice::~RenderDevice()
{
	Finalize();
	g_pDevice = nullptr;
}

int RenderDevice::Initialize(HWND hWnd)
{
	if (D3DDevice::Initialize(hWnd) != S_OK)
		return 0;

	if (m_bFull)
		ToggleFullscreen();

	if (m_pTextureManager == nullptr)
	{
		m_pTextureManager = new TextureManager();

		if (!m_pTextureManager->InitTextureManager())
		{
			LOG_WRITELOG("Initialize Texture Manager Failed\r\n");
			return 0;
		}

		if (m_pMeshManager != nullptr)
			m_pMeshManager->RestoreDeviceObjects();
		if (m_pTextureManager != nullptr)
			m_pTextureManager->RestoreRenderTargetTexture();
	}

	if (m_pFont == nullptr)
	{
		m_pFont = new TMFont2();
	}

	if (m_pSprite == nullptr)
	{
		if (FAILED(D3DXCreateSprite(m_pd3dDevice, &m_pSprite)))
		{
			LOG_WRITELOG("Error Create Sprite\r\n");
			return 0;
		}
	}

	if (!InitVertexShader())
	{
		LOG_WRITELOG("Error VertexShader \r\n");
		return 0;
	}

	if (!InitPixelShader())
	{
		LOG_WRITELOG("Error Pixel Shader \r\n");
		return 0;
	}

	if (!InitializeRenderingState())
	{
		LOG_WRITELOG("Error Rendering State\r\n");
		return 0;
	}

	m_MiniMapVertex2[0].position = m_CtrlVertex[0].position = m_CtrlVertex2[0].position = m_CtrlVertexC2[0].position = TMVector3(0.0f, 0.0f, 0.01f);

	m_CtrlVertex[0].rhw = 1.0f;
	m_CtrlVertex2[0].rhw = 1.0f;
	m_CtrlVertexC2[0].rhw = 1.0f;
	m_MiniMapVertex2[0].rhw = 1.0f;
	m_CtrlVertex[0].diffuse = 0xAAAAAAAA;
	m_CtrlVertex2[0].diffuse = 0xAAAAAAAA;
	m_CtrlVertexC2[0].diffuse = 0xAAAAAAAA;
	m_MiniMapVertex2[0].diffuse = 0xAAAAAAAA;
	m_CtrlVertex[0].tu = 0.01f;
	m_CtrlVertex2[0].tu1 = 0.01f;
	m_CtrlVertexC2[0].tu1 = 0.01f;
	m_MiniMapVertex2[0].tu1 = 0.01f;
	m_CtrlVertex[0].tv = 0.01f;
	m_CtrlVertex2[0].tv1 = 0.01f;
	m_CtrlVertexC2[0].tv1 = 0.01f;
	m_MiniMapVertex2[0].tv1 = 0.01f;

	m_MiniMapVertex2[1].position = m_CtrlVertex[1].position = m_CtrlVertex2[1].position = m_CtrlVertexC2[1].position = TMVector3(0.0f, 0.0f, 0.01f);

	m_CtrlVertex[1].rhw = 1.0f;
	m_CtrlVertex2[1].rhw = 1.0f;
	m_CtrlVertexC2[1].rhw = 1.0f;
	m_MiniMapVertex2[1].rhw = 1.0f;
	m_CtrlVertex[1].diffuse = 0xAAAAAAAA;
	m_CtrlVertex2[1].diffuse = 0xAAAAAAAA;
	m_CtrlVertexC2[1].diffuse = 0xAAAAAAAA;
	m_MiniMapVertex2[1].diffuse = 0xAAAAAAAA;
	m_CtrlVertex[1].tu = 0.99f;
	m_CtrlVertex2[1].tu1 = 0.99f;
	m_CtrlVertexC2[1].tu1 = 0.99f;
	m_MiniMapVertex2[1].tu1 = 0.99f;
	m_CtrlVertex[1].tv = 0.01f;
	m_CtrlVertex2[1].tv1 = 0.01f;
	m_CtrlVertexC2[1].tv1 = 0.01f;
	m_MiniMapVertex2[1].tv1 = 0.01f;

	m_MiniMapVertex2[2].position = m_CtrlVertex[2].position = m_CtrlVertex2[2].position = m_CtrlVertexC2[2].position = TMVector3(0.0f, 0.0f, 0.01f);

	m_CtrlVertex[2].rhw = 1.0f;
	m_CtrlVertex2[2].rhw = 1.0f;
	m_CtrlVertexC2[2].rhw = 1.0f;
	m_MiniMapVertex2[2].rhw = 1.0f;
	m_CtrlVertex[2].diffuse = 0xAAAAAAAA;
	m_CtrlVertex2[2].diffuse = 0xAAAAAAAA;
	m_CtrlVertexC2[2].diffuse = 0xAAAAAAAA;
	m_MiniMapVertex2[2].diffuse = 0xAAAAAAAA;
	m_CtrlVertex[2].tu = 0.99f;
	m_CtrlVertex2[2].tu1 = 0.99f;
	m_CtrlVertexC2[2].tu1 = 0.99f;
	m_MiniMapVertex2[2].tu1 = 0.99f;
	m_CtrlVertex[2].tv = 0.99f;
	m_CtrlVertex2[2].tv1 = 0.99f;
	m_CtrlVertexC2[2].tv1 = 0.99f;
	m_MiniMapVertex2[2].tv1 = 0.99f;

	m_MiniMapVertex2[3].position = m_CtrlVertex[3].position = m_CtrlVertex2[3].position = m_CtrlVertexC2[3].position = TMVector3(0.0f, 0.0f, 0.01f);

	m_CtrlVertex[3].rhw = 1.0f;
	m_CtrlVertex2[3].rhw = 1.0f;
	m_CtrlVertexC2[3].rhw = 1.0f;
	m_MiniMapVertex2[3].rhw = 1.0f;
	m_CtrlVertex[3].diffuse = 0xAAAAAAAA;
	m_CtrlVertex2[3].diffuse = 0xAAAAAAAA;
	m_CtrlVertexC2[3].diffuse = 0xAAAAAAAA;
	m_MiniMapVertex2[3].diffuse = 0xAAAAAAAA;
	m_CtrlVertex[3].tu = 0.01f;
	m_CtrlVertex2[3].tu1 = 0.01f;
	m_CtrlVertexC2[3].tu1 = 0.01f;
	m_MiniMapVertex2[3].tu1 = 0.01f;
	m_CtrlVertex[3].tv = 0.99f;
	m_CtrlVertex2[3].tv1 = 0.99f;
	m_CtrlVertexC2[3].tv1 = 0.99f;
	m_MiniMapVertex2[3].tv1 = 0.99f;

	SetGamma();

	return 1;
}

void RenderDevice::Finalize()
{
	RenderDevice::m_nBright = 50;

	D3DGAMMARAMP gamma{};
	m_pd3dDevice->GetGammaRamp(0, &gamma);

	for (int i = 0; i < 256; i++)
	{
		int nVal = static_cast<int>(((((float)RenderDevice::m_nBright * 0.02f) * (float)i) * 256.0f));
		if (nVal > 0xFFFF)
			nVal = 0xFFFF;

		gamma.red[i] = nVal;
		gamma.green[i] = nVal;
		gamma.blue[i] = nVal;
	}

	m_pd3dDevice->SetGammaRamp(0, 0, &gamma);

	if (m_d3dpp.Windowed)
	{
		HDC hdc = GetDC(g_pApp->m_hWnd);
		SetDeviceGammaRamp(hdc, &gamma);
		ReleaseDC(g_pApp->m_hWnd, hdc);
	}

	SAFE_DELETE(m_pTextureManager);
	SAFE_DELETE(m_pMeshManager);
	SAFE_RELEASE(m_pSprite);

	for (int ia = 0; ia < 8; ++ia)
	{
		SAFE_RELEASE(m_pVertexShader[ia]);
		SAFE_RELEASE(m_pVertexDeclaration[ia]);
	}

	for (int ib = 0; ib < 4; ++ib)
	{
		SAFE_RELEASE(m_pVSEffect[ib]);
		SAFE_RELEASE(m_pVDEffect[ib]);
	}

	for (int ic = 0; ic < 6; ++ic)
	{
		SAFE_RELEASE(m_pPSEffect[ic]);
	}

	SAFE_DELETE(m_pFont);
	SAFE_DELETE_ARRAY(TMFont2::m_pBuffer);

	Cleanup3DEnvironment();
}

void RenderDevice::SetViewPort(int nStartX, int nStartY, int nWidth, int nHeight)
{
	m_viewport.X = nStartX;
	m_viewport.Y = nStartY;
	m_viewport.Width = nWidth;
	m_viewport.Height = nHeight;
	m_viewport.MinZ = 0.0f;
	m_viewport.MaxZ = 1.0f;

	if (FAILED(m_pd3dDevice->SetViewport(&m_viewport)))
	{
		LOG_WRITEERROR(0x10000006);
	}

	SetProjectionMatrix();
}

int RenderDevice::Lock(int bClear)
{
	if (m_bDeviceLost)
	{
		HRESULT hr = m_pd3dDevice->TestCooperativeLevel();
		if (FAILED(hr))
		{
			if (hr == D3DERR_DEVICELOST)
				return 0;
			if (hr != D3DERR_DEVICENOTRESET)
				return hr;
			
			if (m_bWindowed)
			{
				D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
				m_pD3D->GetAdapterDisplayMode(pAdapterInfo->AdapterOrdinal, &m_d3dSettings.Windowed_DisplayMode);
				m_d3dpp.BackBufferFormat = m_d3dSettings.Windowed_DisplayMode.Format;
			}

			return Reset3DEnvironment();
		}

		m_bDeviceLost = 0;
	}

	if (bClear)
	{
		if (g_pCurrentScene != nullptr && g_pCurrentScene->m_pSky != nullptr)
		{
			if (RenderDevice::m_bDungeon != 0 && RenderDevice::m_bDungeon != 3 && RenderDevice::m_bDungeon != 4)
				m_dwActualClearColor = 0;
			else if (g_pCurrentScene->m_pSky->m_nState == 0)
				m_dwActualClearColor = 0xFF335599;
			else if (g_pCurrentScene->m_pSky->m_nState == 1)
				m_dwActualClearColor = 0xFF333333;
			else if (g_pCurrentScene->m_pSky->m_nState == 2)
				m_dwActualClearColor = 0xFF441100;
			else if (g_pCurrentScene->m_pSky->m_nState == 3)
				m_dwActualClearColor = 0xFF222222;
		}

		if (FAILED(m_pd3dDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, m_dwActualClearColor, 1.0f, 0)))
			return 0;
	}

	HRESULT result = m_pd3dDevice->BeginScene();

	return result == D3DERR_INVALIDCALL ? 0 : result >= 0;
}

int RenderDevice::Unlock(int bEnd)
{
	if (bEnd == 1 && m_pFont != nullptr)
	{
		float fTime = timeGetTime() * 0.001f;

		static DWORD dwFrames = 0;
		++dwFrames;
		static char szString[256];

		static float fLastTime = 0.0f;
		if ((fTime - fLastTime) > 2.0f)
		{
			m_fFPS = dwFrames / (fTime - fLastTime);
			fLastTime = fTime;
			dwFrames = 0;
			
#ifdef _DEBUG
			sprintf(szString, "FPS: %4.3f | Objects: %d | EN: %d ET: %d", m_fFPS, g_objectnumber, g_effectnumber, g_totaleffect);
#endif
		}

		m_bShowEffects = 1;
		if (g_bDebugMsg == 1)
		{
			m_pFont->SetText(szString, 0xFFFFFFFF, 0);
			m_pFont->Render(10, m_dwScreenHeight - 15, 0);
		}
	}

	if (FAILED(m_pd3dDevice->EndScene()))
		return 0;

	if (bEnd == 1)
	{
		if (m_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST)
			m_bDeviceLost = 1;
	}

	return 1;
}

void RenderDevice::SetLight()
{
	for (int i = 0; i < 2; i++)
	{
		if(FAILED(m_pd3dDevice->SetLight(i, &m_light[i])))
			return;

		if (FAILED(m_pd3dDevice->LightEnable(i, 1)))
			return;
	}
}

void RenderDevice::SetWindowedFullScreen()
{
	HDC hDC = CreateDCA("DISPLAY", 0, 0, nullptr);
	m_dwCurrScreenX = GetDeviceCaps(hDC, HORZRES);
	m_dwCurrScreenY = GetDeviceCaps(hDC, VERTRES);
	m_dwCurrBpp = GetDeviceCaps(hDC, BITSPIXEL);
	m_dwCurrRefreshRate = GetDeviceCaps(hDC, VREFRESH);

	DeleteDC(hDC);

	if (!ChangeDisplay(m_dwScreenWidth, m_dwScreenHeight, m_dwBitCount, RenderDevice::m_dwCurrRefreshRate))
		ChangeDisplay(m_dwScreenWidth, m_dwScreenHeight, m_dwBitCount, 0);
}

void RenderDevice::RestoreWindowMode()
{
	ChangeDisplay(RenderDevice::m_dwCurrScreenX,
		RenderDevice::m_dwCurrScreenY,
		RenderDevice::m_dwCurrBpp,
		RenderDevice::m_dwCurrRefreshRate);
}

int RenderDevice::ChangeDisplay(DWORD x, DWORD y, DWORD bpp, DWORD ref)
{
	DEVMODE mode;
	GetCurrentDisplayMode(&mode);

	mode.dmPelsWidth = x;
	mode.dmPelsHeight = y;
	mode.dmBitsPerPel = bpp;
	if (ref)
		mode.dmDisplayFrequency = ref;

	int lResult = ChangeDisplaySettings(&mode, 0);
	return lResult == 0 || lResult == 1;
}

HRESULT RenderDevice::GetCurrentDisplayMode(PDEVMODE devMode)
{
	HDC hDC = CreateDCA("DISPLAY", 0, 0, 0);
	int iOrgX = GetDeviceCaps(hDC, HORZRES);
	int iOrgY = GetDeviceCaps(hDC, VERTRES);
	int iOrgBpp = GetDeviceCaps(hDC, BITSPIXEL);
	GetDeviceCaps(hDC, VREFRESH);
	DeleteDC(hDC);

	int iModeNum = 0;
	for (int bResult = EnumDisplaySettings(0, 0, devMode); bResult; bResult = EnumDisplaySettings(0, iModeNum, devMode))
	{
		if (devMode->dmPelsWidth == iOrgX && devMode->dmPelsHeight == iOrgY && devMode->dmBitsPerPel == iOrgBpp)
			return 1;

		++iModeNum;
	}

	return 0;
}

HRESULT RenderDevice::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT Format)
{
	return (dwBehavior & 0x40) ? E_FAIL : 0;
}

HRESULT RenderDevice::RestoreDeviceObjects()
{
	D3DCAPS9 d3dCaps{};
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);
	if (d3dCaps.PixelShaderVersion >= 0xFFFF0101)
	{
		m_bSupportPS11 = 1;
		m_nShadowTextureSize = 128;
	}
	else
	{
		m_bSupportPS11 = 0;
		m_nShadowTextureSize = 64;

		if (g_nReflection > 0)
			g_nReflection = 0;
		if (g_nUseBlur > 0)
			g_nUseBlur = 0;
	}

	m_bSupportPS12 = d3dCaps.PixelShaderVersion >= 0xFFFF0102;
	m_bSupportVS20 = d3dCaps.VertexShaderVersion >= 0xFFFE0200;

	SetViewPort(0, 0, m_dwScreenWidth, m_dwScreenHeight);

	for (int i = 0; i < 10; ++i)
		m_pd3dDevice->SetTexture(i, nullptr);

	if (m_pSprite == nullptr)
	{
		if (FAILED(D3DXCreateSprite(m_pd3dDevice, &m_pSprite)))
			return 0;
	}

	if (g_pApp->m_pObjectManager != nullptr)
		g_pApp->m_pObjectManager->RestoreDeviceObjects();

	if (m_hbmBitmap == nullptr)
	{
		memset(&m_bmi, 0, sizeof(m_bmi));
		m_bmi.bmiHeader.biSize = 40;
		m_bmi.bmiHeader.biWidth = RenderDevice::m_nFontTextureSize;
		m_bmi.bmiHeader.biHeight = -RenderDevice::m_nFontTextureSize;
		m_bmi.bmiHeader.biPlanes = 1;
		m_bmi.bmiHeader.biCompression = 0;
		m_bmi.bmiHeader.biBitCount = 32;
		m_hDC = CreateCompatibleDC(0);
		m_hbmBitmap = CreateDIBSection(m_hDC, &m_bmi, 0, (void**)&m_pBitmapBits, 0,	0);

		sprintf_s(g_szFontName, "Tahoma");
		FILE* fpFont = nullptr;
		fopen_s(&fpFont, FontConfig_Path, "rt");

		if (fpFont != nullptr)
		{
			char szTemp[256]{};
			fgets(szTemp, 256, fpFont);
			sscanf(szTemp, "%s", g_szFontName);
			fgets(szTemp, 256, fpFont);
			sscanf(szTemp, "%d", &g_nFontBold);

			if (g_nFontBold <= 0)
				g_nFontBold = 500;

			fclose(fpFont);
		}

		m_hFont = CreateFont(RenderDevice::m_nFontSize, 0, 0, 0, g_nFontBold, 0, 0, 0, 1, 4, 0, 4, 2, g_szFontName);
		SelectObject(m_hDC, m_hbmBitmap);
		SelectObject(m_hDC, m_hFont);
		SetTextColor(m_hDC, (COLORREF)0xFFFFFF);
		SetBkColor(m_hDC, 0);
	}

	HRESULT hr = m_pd3dDevice->GetRenderTarget(0, &m_pOldRenderTarget);
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->GetDepthStencilSurface(&m_pOldDepthStencil);
	if (FAILED(hr))
		return hr;

	if (m_pDXFont != nullptr)
		m_pDXFont->OnResetDevice();
	if (m_pDXFontLarge != nullptr)
		m_pDXFontLarge->OnResetDevice();

	InitializeRenderingState();
	InitVertexShader();
	// Not used
	//InitPixelShader();

	if (d3dCaps.MaxTextureWidth > 256)
	{
		TextureManager::DYNAMIC_TEXTURE_WIDTH = 512;
		TextureManager::DYNAMIC_TEXTURE_HEIGHT = 512;
	}

	return 0;
}

HRESULT RenderDevice::InvalidateDeviceObjects()
{
	if (m_pTextureManager != nullptr)
		m_pTextureManager->ReleaseTexture();
	if (m_pMeshManager != nullptr)
		m_pMeshManager->ReleaseMesh();
	if (g_pObjectManager != nullptr)
		g_pObjectManager->InvalidateDeviceObjects();

	for (int iInfl = 0; iInfl < 8; ++iInfl)
	{
		SAFE_RELEASE(m_pVertexShader[iInfl]);
		SAFE_RELEASE(m_pVertexDeclaration[iInfl]);
	}
	for (int i = 0; i < 4; ++i)
	{
		SAFE_RELEASE(m_pVSEffect[i]);
	}
	for (int ia = 0; ia < 6; ++ia)
	{
		SAFE_RELEASE(m_pPSEffect[ia]);
	}

	if (m_hbmBitmap != nullptr)
	{
		DeleteObject(m_hbmBitmap);
		DeleteDC(m_hDC);
		DeleteObject(m_hFont);
		m_hbmBitmap = nullptr;
		m_hDC = nullptr;
		m_hFont = nullptr;
	}
	
	SAFE_RELEASE(m_pOldRenderTarget);
	SAFE_RELEASE(m_pOldDepthStencil);

	if (m_pDXFont != nullptr)
		m_pDXFont->OnLostDevice();
	if (m_pDXFontLarge != nullptr)
		m_pDXFontLarge->OnLostDevice();
	
	SAFE_RELEASE(m_pSprite);

	return 0;
}

void RenderDevice::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	if ((int)State >= 0 && (int)State < 256 && m_dwRenderStateList[State] != Value)
	{
		m_dwRenderStateList[State] = Value;
		m_pd3dDevice->SetRenderState(State, Value);
	}
}

void RenderDevice::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE type, DWORD Value)
{
	if (dwStage >= 8)
		return;

	if (type >= 0 && type < 29 && m_dwTextureStageStateList[dwStage][type] != Value)
	{
		m_dwTextureStageStateList[dwStage][type] = Value;
		m_pd3dDevice->SetTextureStageState(dwStage, type, Value);
	}
}

void RenderDevice::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	if (Stage >= 8)
		return;

	if (m_pTexture[Stage] != pTexture)
	{
		m_pTexture[Stage] = pTexture;
		m_pd3dDevice->SetTexture(Stage, pTexture);
	}
}

void RenderDevice::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE State, DWORD Value)
{
	if (dwStage >= 8)
		return;

	if (State >= 0 && State < 14 && m_dwSamplerStateList[dwStage][State] != Value)
	{
		m_dwSamplerStateList[dwStage][State] = Value;
		m_pd3dDevice->SetSamplerState(dwStage, State, Value);
	}
}

void RenderDevice::SetGamma()
{
	if (RenderDevice::m_nBright != 50)
	{
		static D3DGAMMARAMP gamma;
		memset(&gamma, 0, sizeof(gamma));

		m_pd3dDevice->GetGammaRamp(1, &gamma);

		for (int i = 0; i < 256; i++)
		{
			int nVal = static_cast<int>(((((float)RenderDevice::m_nBright * 0.02f) * (float)i) * 256.0f));
			if (nVal > 0xFFFF)
				nVal = 0xFFFF;

			gamma.red[i] = nVal;
			gamma.green[i] = nVal;
			gamma.blue[i] = nVal;
		}

		m_pd3dDevice->SetGammaRamp(0, 0, &gamma);

		if (m_d3dpp.Windowed)
		{
			HDC hdc = GetDC(g_pApp->m_hWnd);
			SetDeviceGammaRamp(hdc, &gamma);
			ReleaseDC(g_pApp->m_hWnd, hdc);
		}
	}
}

int RenderDevice::InitMeshManager()
{
	if (m_pMeshManager != nullptr)
		return 1;

	m_pMeshManager = new MeshManager();
	if (!m_pMeshManager->InitMeshManager())
	{
		LOG_WRITELOG("Initialize Mesh Manager Failed\r\n");
		return 0;
	}

	m_pMeshManager->RestoreDeviceObjects();
	m_bLoadMeshManager = 1;

	return 1;
}

int RenderDevice::InitializeRenderingState()
{
	D3DXMatrixIdentity(&m_matWorld);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_matWorld);

	memset(m_dwRenderStateList, 0, sizeof(m_dwRenderStateList));

	for (int nState = 0; nState < 256; ++nState)
	{
		m_dwRenderStateList[nState] = -1;
	}

	for (int nStage = 0; nStage < 8; ++nStage)
	{
		for (int i = 0; i < 14; ++i)
		{
			m_dwSamplerStateList[nStage][i] = -1;
		}
	}

	for (int nStage = 0; nStage < 8; ++nStage)
	{
		for (int j = 0; j < 29; ++j)
			m_dwTextureStageStateList[nStage][j] = -1;
	}

	return 1;
}

int RenderDevice::SetViewVector(TMVector3 ivCamera, TMVector3 ivLookat)
{
	D3DXVECTOR3 upVector(0.0f, 1.0f, 0.0f);
	m_vCamera = D3DXVECTOR3(ivCamera.x, ivCamera.y, ivCamera.z);
	m_vLookatPos = D3DXVECTOR3(ivLookat.x, ivLookat.y, ivLookat.z);

	D3DXMatrixLookAtLH(&m_matView, &m_vCamera, &m_vLookatPos, &upVector);

	return SUCCEEDED(m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView));
}

HRESULT RenderDevice::SetProjectionMatrix()
{
	D3DXMatrixPerspectiveFovLH(&m_matProj, 
		RenderDevice::m_fFOVY * D3DXToRadian(180),
		(float)((float)(m_dwScreenWidth - m_nWidthShift) / (float)(m_dwScreenHeight - m_nHeightShift)) * 1.0f,
		g_ClipNear * 1.4f,
		g_ClipFar);

	HRESULT hr = m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	D3DXMATRIX mat;
	D3DXMatrixTranspose(&mat, &m_matProj);

	m_pd3dDevice->SetVertexShaderConstantF(2, (const float*)&mat, 4);

	return SUCCEEDED(hr) ? 0 : hr;
}

int RenderDevice::SetMatrixForUI()
{
	D3DXMATRIX matUIProjection;
	D3DXMatrixPerspectiveFovLH(&matUIProjection, 0.1f, (float)((float)m_d3dsdBackBuffer.Width / (float)m_d3dsdBackBuffer.Height) * 0.94f, 10.0f, 100.0f);

	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matUIProjection);

	D3DXMATRIX matView;
	D3DXVECTOR3 upVector(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 vecCam(0.0f, 0.0f, 50.0f);
	D3DXVECTOR3 vecLookAt(0.0f, 0.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &vecCam, &vecLookAt, &upVector);

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	return 1;
}

void RenderDevice::GetPickRayVector(D3DXVECTOR3* pRickRayOrig, D3DXVECTOR3* pPickRayDir)
{
	D3DXVECTOR3 v;
	v.x = (((2.0f * g_pCursor->m_nPosX) / (float)(m_dwScreenWidth - m_nWidthShift)) - 1.0f) / m_matProj.m[0][0];
	v.y = -(((2.0f * g_pCursor->m_nPosY) / (float)(m_dwScreenHeight - m_nHeightShift)) - 1.0f) / m_matProj.m[1][1];
	v.z = 1.0f;

	D3DXMATRIX matViewInv;
	D3DXMatrixInverse(&matViewInv, 0, &m_matView);

	pPickRayDir->x = (float)((v.x * matViewInv.m[0][0]) + (v.y * matViewInv.m[1][0]) + (v.z * matViewInv.m[2][0]));
	pPickRayDir->y = (float)((v.x * matViewInv.m[0][1]) + (v.y * matViewInv.m[1][1]) + (v.z * matViewInv.m[2][1]));
	pPickRayDir->z = (float)((v.x * matViewInv.m[0][2]) + (v.y * matViewInv.m[1][2]) + (v.z * matViewInv.m[2][2]));

	pRickRayOrig->x = matViewInv.m[3][0];
	pRickRayOrig->y = matViewInv.m[3][1];
	pRickRayOrig->z = matViewInv.m[3][2];
}

void RenderDevice::LogRenderState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, RenderStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 256; i++)
	{
		fprintf(fp, "Index:%d Val=%d\n", i, m_dwRenderStateList[i]);
	}

	fclose(fp);
}

void RenderDevice::LogSamplerState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, SamplerStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 14; ++j)
		{
			fprintf(fp, "Stage:%d State:%d  Val=%d\n", i, j, m_dwSamplerStateList[i][j]);
		}
	}

	fclose(fp);
}

void RenderDevice::LogTextureStageState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, TextureStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 29; ++j)
		{
			fprintf(fp, "Stage:%d State:%d  Val=%d\n", i, j, m_dwTextureStageStateList[i][j]);
		}
	}

	fclose(fp);
}
