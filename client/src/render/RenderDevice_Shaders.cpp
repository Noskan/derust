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

/// @file RenderDevice_Shaders.cpp
/// @brief Vertex/pixel shaders (declarações de vértice e bytecode embutido) e os
///        blocos de estados de render (`SetRenderStateBlock`) do RenderDevice.
///
/// Extraído de `RenderDevice.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

int RenderDevice::InitVertexShader()
{
	D3DVERTEXELEMENT9 VertexDecl1[5];
	D3DVERTEXELEMENT9 VertexDecl2[6];
	D3DVERTEXELEMENT9 VertexDecl3[6];
	D3DVERTEXELEMENT9 VertexDecl4[6];

	VertexDecl1[0].Stream = 0;
	VertexDecl1[0].Offset = 0;
	VertexDecl1[0].Type = 2;
	VertexDecl1[0].Method = 0;
	VertexDecl1[0].Usage = 0;
	VertexDecl1[0].UsageIndex = 0;
	VertexDecl1[1].Stream = 0;
	VertexDecl1[1].Offset = 12;
	VertexDecl1[1].Type = 4;
	VertexDecl1[1].Method = 0;
	VertexDecl1[1].Usage = 2;
	VertexDecl1[1].UsageIndex = 0;
	VertexDecl1[2].Stream = 0;
	VertexDecl1[2].Offset = 16;
	VertexDecl1[2].Type = 2;
	VertexDecl1[2].Method = 0;
	VertexDecl1[2].Usage = 3;
	VertexDecl1[2].UsageIndex = 0;
	VertexDecl1[3].Stream = 0;
	VertexDecl1[3].Offset = 28;
	VertexDecl1[3].Type = 1;
	VertexDecl1[3].Method = 0;
	VertexDecl1[3].Usage = 5;
	VertexDecl1[3].UsageIndex = 0;
	VertexDecl1[4].Stream = 255;
	VertexDecl1[4].Offset = 0;
	VertexDecl1[4].Type = 17;
	VertexDecl1[4].Method = 0;
	VertexDecl1[4].Usage = 0;
	VertexDecl1[4].UsageIndex = 0;
	VertexDecl2[0].Stream = 0;
	VertexDecl2[0].Offset = 0;
	VertexDecl2[0].Type = 2;
	VertexDecl2[0].Method = 0;
	VertexDecl2[0].Usage = 0;
	VertexDecl2[0].UsageIndex = 0;
	VertexDecl2[1].Stream = 0;
	VertexDecl2[1].Offset = 12;
	VertexDecl2[1].Type = 0;
	VertexDecl2[1].Method = 0;
	VertexDecl2[1].Usage = 1;
	VertexDecl2[1].UsageIndex = 0;
	VertexDecl2[2].Stream = 0;
	VertexDecl2[2].Offset = 16;
	VertexDecl2[2].Type = 4;
	VertexDecl2[2].Method = 0;
	VertexDecl2[2].Usage = 2;
	VertexDecl2[2].UsageIndex = 0;
	VertexDecl2[3].Stream = 0;
	VertexDecl2[3].Offset = 20;
	VertexDecl2[3].Type = 2;
	VertexDecl2[3].Method = 0;
	VertexDecl2[3].Usage = 3;
	VertexDecl2[3].UsageIndex = 0;
	VertexDecl2[4].Stream = 0;
	VertexDecl2[4].Offset = 32;
	VertexDecl2[4].Type = 1;
	VertexDecl2[4].Method = 0;
	VertexDecl2[4].Usage = 5;
	VertexDecl2[4].UsageIndex = 0;
	VertexDecl2[5].Stream = 255;
	VertexDecl2[5].Offset = 0;
	VertexDecl2[5].Type = 17;
	VertexDecl2[5].Method = 0;
	VertexDecl2[5].Usage = 0;
	VertexDecl2[5].UsageIndex = 0;
	VertexDecl3[0].Stream = 0;
	VertexDecl3[0].Offset = 0;
	VertexDecl3[0].Type = 2;
	VertexDecl3[0].Method = 0;
	VertexDecl3[0].Usage = 0;
	VertexDecl3[0].UsageIndex = 0;
	VertexDecl3[1].Stream = 0;
	VertexDecl3[1].Offset = 12;
	VertexDecl3[1].Type = 1;
	VertexDecl3[1].Method = 0;
	VertexDecl3[1].Usage = 1;
	VertexDecl3[1].UsageIndex = 0;
	VertexDecl3[2].Stream = 0;
	VertexDecl3[2].Offset = 20;
	VertexDecl3[2].Type = 4;
	VertexDecl3[2].Method = 0;
	VertexDecl3[2].Usage = 2;
	VertexDecl3[2].UsageIndex = 0;
	VertexDecl3[3].Stream = 0;
	VertexDecl3[3].Offset = 24;
	VertexDecl3[3].Type = 2;
	VertexDecl3[3].Method = 0;
	VertexDecl3[3].Usage = 3;
	VertexDecl3[3].UsageIndex = 0;
	VertexDecl3[4].Stream = 0;
	VertexDecl3[4].Offset = 36;
	VertexDecl3[4].Type = 1;
	VertexDecl3[4].Method = 0;
	VertexDecl3[4].Usage = 5;
	VertexDecl3[4].UsageIndex = 0;
	VertexDecl3[5].Stream = 255;
	VertexDecl3[5].Offset = 0;
	VertexDecl3[5].Type = 17;
	VertexDecl3[5].Method = 0;
	VertexDecl3[5].Usage = 0;
	VertexDecl3[5].UsageIndex = 0;
	VertexDecl4[0].Stream = 0;
	VertexDecl4[0].Offset = 0;
	VertexDecl4[0].Type = 2;
	VertexDecl4[0].Method = 0;
	VertexDecl4[0].Usage = 0;
	VertexDecl4[0].UsageIndex = 0;
	VertexDecl4[1].Stream = 0;
	VertexDecl4[1].Offset = 12;
	VertexDecl4[1].Type = 2;
	VertexDecl4[1].Method = 0;
	VertexDecl4[1].Usage = 1;
	VertexDecl4[1].UsageIndex = 0;
	VertexDecl4[2].Stream = 0;
	VertexDecl4[2].Offset = 24;
	VertexDecl4[2].Type = 4;
	VertexDecl4[2].Method = 0;
	VertexDecl4[2].Usage = 2;
	VertexDecl4[2].UsageIndex = 0;
	VertexDecl4[3].Stream = 0;
	VertexDecl4[3].Offset = 28;
	VertexDecl4[3].Type = 2;
	VertexDecl4[3].Method = 0;
	VertexDecl4[3].Usage = 3;
	VertexDecl4[3].UsageIndex = 0;
	VertexDecl4[4].Stream = 0;
	VertexDecl4[4].Offset = 40;
	VertexDecl4[4].Type = 1;
	VertexDecl4[4].Method = 0;
	VertexDecl4[4].Usage = 5;
	VertexDecl4[4].UsageIndex = 0;
	VertexDecl4[5].Stream = 255;
	VertexDecl4[5].Offset = 0;
	VertexDecl4[5].Type = 17;
	VertexDecl4[5].Method = 0;
	VertexDecl4[5].Usage = 0;
	VertexDecl4[5].UsageIndex = 0;

	LPD3DVERTEXELEMENT9 VertexDecl[4];
	VertexDecl[0] = VertexDecl1;
	VertexDecl[1] = VertexDecl2;
	VertexDecl[2] = VertexDecl3;
	VertexDecl[3] = VertexDecl4;

	m_bUseSW = 16;
	D3DCAPS9 d3dCaps{};
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);

	if (d3dCaps.VertexShaderVersion >= 0xFFFE0101)
	{
		m_bUseSW = 0;
		m_dwBufferUsage = 8;
		if (m_pVertexShader[0] == nullptr)
			LOG_WRITELOG("VertexShader HW Accel Enabled\r\n");
	}
	else if (m_pVertexShader[0] == nullptr)
	{
		LOG_WRITELOG("VertexShader SW Mode \r\n");
	}

	if (m_pVertexShader[0] == nullptr)
	{
		for (int i = 0; i < 8; ++i)
		{
			LPD3DXBUFFER pCode = nullptr;
			char szBinFile[128];
			sprintf_s(szBinFile, ShaderSkinMesh_Path, i + 1);

			int handle = _open(szBinFile, _O_BINARY);
			if (handle == -1)
			{
				LOG_WRITELOG("Read VertexShader %d Error.. \r\n", i + 1);
				return 0;
			}

			int nLength = _filelength(handle);
			D3DXCreateBuffer(nLength, &pCode);
			_read(handle, pCode->GetBufferPointer(), pCode->GetBufferSize());
			_close(handle);

			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDecl[i % 4], &m_pVertexDeclaration[i])))
				return 0;

			if (FAILED(m_pd3dDevice->CreateVertexShader((const DWORD*)pCode->GetBufferPointer(), &m_pVertexShader[i])))
				return 0;

			pCode->Release();
		}

		D3DXMATRIX mat;
		D3DXMatrixTranspose(&mat, &m_matProj);
		m_pd3dDevice->SetVertexShaderConstantF(2, (const float*)&mat, 4);
	}

	D3DVERTEXELEMENT9 DeclWater[4];
	DeclWater[0].Stream = 0;
	DeclWater[0].Offset = 0;
	DeclWater[0].Type = 2;
	DeclWater[0].Method = 0;
	DeclWater[0].Usage = 0;
	DeclWater[0].UsageIndex = 0;
	DeclWater[1].Stream = 0;
	DeclWater[1].Offset = 12;
	DeclWater[1].Type = 4;
	DeclWater[1].Method = 0;
	DeclWater[1].Usage = 10;
	DeclWater[1].UsageIndex = 0;
	DeclWater[2].Stream = 0;
	DeclWater[2].Offset = 16;
	DeclWater[2].Type = 1;
	DeclWater[2].Method = 0;
	DeclWater[2].Usage = 5;
	DeclWater[2].UsageIndex = 0;
	DeclWater[3].Stream = 255;
	DeclWater[3].Offset = 0;
	DeclWater[3].Type = 17;
	DeclWater[3].Method = 0;
	DeclWater[3].Usage = 0;
	DeclWater[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclEquip[4];
	DeclEquip[0].Stream = 0;
	DeclEquip[0].Offset = 0;
	DeclEquip[0].Type = 2;
	DeclEquip[0].Method = 0;
	DeclEquip[0].Usage = 0;
	DeclEquip[0].UsageIndex = 0;
	DeclEquip[1].Stream = 0;
	DeclEquip[1].Offset = 12;
	DeclEquip[1].Type = 2;
	DeclEquip[1].Method = 0;
	DeclEquip[1].Usage = 3;
	DeclEquip[1].UsageIndex = 0;
	DeclEquip[2].Stream = 0;
	DeclEquip[2].Offset = 24;
	DeclEquip[2].Type = 1;
	DeclEquip[2].Method = 0;
	DeclEquip[2].Usage = 5;
	DeclEquip[2].UsageIndex = 0;
	DeclEquip[3].Stream = 255;
	DeclEquip[3].Offset = 0;
	DeclEquip[3].Type = 17;
	DeclEquip[3].Method = 0;
	DeclEquip[3].Usage = 0;
	DeclEquip[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclEquipShadow[2];
	DeclEquipShadow[0].Stream = 0;
	DeclEquipShadow[0].Offset = 0;
	DeclEquipShadow[0].Type = 2;
	DeclEquipShadow[0].Method = 0;
	DeclEquipShadow[0].Usage = 0;
	DeclEquipShadow[0].UsageIndex = 0;
	DeclEquipShadow[1].Stream = 255;
	DeclEquipShadow[1].Offset = 0;
	DeclEquipShadow[1].Type = 17;
	DeclEquipShadow[1].Method = 0;
	DeclEquipShadow[1].Usage = 0;
	DeclEquipShadow[1].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclBlur[3];
	DeclBlur[0].Stream = 0;
	DeclBlur[0].Offset = 0;
	DeclBlur[0].Type = 2;
	DeclBlur[0].Method = 0;
	DeclBlur[0].Usage = 0;
	DeclBlur[0].UsageIndex = 0;
	DeclBlur[1].Stream = 0;
	DeclBlur[1].Offset = 12;
	DeclBlur[1].Type = 1;
	DeclBlur[1].Method = 0;
	DeclBlur[1].Usage = 5;
	DeclBlur[1].UsageIndex = 0;
	DeclBlur[2].Stream = 255;
	DeclBlur[2].Offset = 0;
	DeclBlur[2].Type = 17;
	DeclBlur[2].Method = 0;
	DeclBlur[2].Usage = 0;
	DeclBlur[2].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclMesh[4];
	DeclMesh[0].Stream = 0;
	DeclMesh[0].Offset = 0;
	DeclMesh[0].Type = 2;
	DeclMesh[0].Method = 0;
	DeclMesh[0].Usage = 0;
	DeclMesh[0].UsageIndex = 0;
	DeclMesh[1].Stream = 0;
	DeclMesh[1].Offset = 12;
	DeclMesh[1].Type = 4;
	DeclMesh[1].Method = 0;
	DeclMesh[1].Usage = 10;
	DeclMesh[1].UsageIndex = 0;
	DeclMesh[2].Stream = 0;
	DeclMesh[2].Offset = 16;
	DeclMesh[2].Type = 1;
	DeclMesh[2].Method = 0;
	DeclMesh[2].Usage = 5;
	DeclMesh[2].UsageIndex = 0;
	DeclMesh[3].Stream = 255;
	DeclMesh[3].Offset = 0;
	DeclMesh[3].Type = 17;
	DeclMesh[3].Method = 0;
	DeclMesh[3].Usage = 0;
	DeclMesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclSwing[6];
	DeclSwing[0].Stream = 0;
	DeclSwing[0].Offset = 0;
	DeclSwing[0].Type = 2;
	DeclSwing[0].Method = 0;
	DeclSwing[0].Usage = 0;
	DeclSwing[0].UsageIndex = 0;
	DeclSwing[1].Stream = 0;
	DeclSwing[1].Offset = 12;
	DeclSwing[1].Type = 4;
	DeclSwing[1].Method = 0;
	DeclSwing[1].Usage = 10;
	DeclSwing[1].UsageIndex = 0;
	DeclSwing[2].Stream = 0;
	DeclSwing[2].Offset = 16;
	DeclSwing[2].Type = 1;
	DeclSwing[2].Method = 0;
	DeclSwing[2].Usage = 5;
	DeclSwing[2].UsageIndex = 0;
	DeclSwing[3].Stream = 0;
	DeclSwing[3].Offset = 24;
	DeclSwing[3].Type = 1;
	DeclSwing[3].Method = 0;
	DeclSwing[3].Usage = 5;
	DeclSwing[3].UsageIndex = 1;
	DeclSwing[4].Stream = 0;
	DeclSwing[4].Offset = 32;
	DeclSwing[4].Type = 1;
	DeclSwing[4].Method = 0;
	DeclSwing[4].Usage = 5;
	DeclSwing[4].UsageIndex = 2;
	DeclSwing[5].Stream = 255;
	DeclSwing[5].Offset = 0;
	DeclSwing[5].Type = 17;
	DeclSwing[5].Method = 0;
	DeclSwing[5].Usage = 0;
	DeclSwing[5].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclEfMesh[3];
	DeclEfMesh[0].Stream = 0;
	DeclEfMesh[0].Offset = 0;
	DeclEfMesh[0].Type = 2;
	DeclEfMesh[0].Method = 0;
	DeclEfMesh[0].Usage = 0;
	DeclEfMesh[0].UsageIndex = 0;
	DeclEfMesh[1].Stream = 0;
	DeclEfMesh[1].Offset = 12;
	DeclEfMesh[1].Type = 1;
	DeclEfMesh[1].Method = 0;
	DeclEfMesh[1].Usage = 5;
	DeclEfMesh[1].UsageIndex = 0;
	DeclEfMesh[2].Stream = 255;
	DeclEfMesh[2].Offset = 0;
	DeclEfMesh[2].Type = 17;
	DeclEfMesh[2].Method = 0;
	DeclEfMesh[2].Usage = 0;
	DeclEfMesh[2].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclBumpEquip[5];
	DeclBumpEquip[0].Stream = 0;
	DeclBumpEquip[0].Offset = 0;
	DeclBumpEquip[0].Type = 2;
	DeclBumpEquip[0].Method = 0;
	DeclBumpEquip[0].Usage = 0;
	DeclBumpEquip[0].UsageIndex = 0;
	DeclBumpEquip[1].Stream = 0;
	DeclBumpEquip[1].Offset = 12;
	DeclBumpEquip[1].Type = 2;
	DeclBumpEquip[1].Method = 0;
	DeclBumpEquip[1].Usage = 3;
	DeclBumpEquip[1].UsageIndex = 0;
	DeclBumpEquip[2].Stream = 0;
	DeclBumpEquip[2].Offset = 24;
	DeclBumpEquip[2].Type = 1;
	DeclBumpEquip[2].Method = 0;
	DeclBumpEquip[2].Usage = 5;
	DeclBumpEquip[2].UsageIndex = 0;
	DeclBumpEquip[3].Stream = 0;
	DeclBumpEquip[3].Offset = 32;
	DeclBumpEquip[3].Type = 2;
	DeclBumpEquip[3].Method = 0;
	DeclBumpEquip[3].Usage = 6;
	DeclBumpEquip[3].UsageIndex = 0;
	DeclBumpEquip[4].Stream = 255;
	DeclBumpEquip[4].Offset = 0;
	DeclBumpEquip[4].Type = 17;
	DeclBumpEquip[4].Method = 0;
	DeclBumpEquip[4].Usage = 0;
	DeclBumpEquip[4].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclEnvMesh[4];
	DeclEnvMesh[0].Stream = 0;
	DeclEnvMesh[0].Offset = 0;
	DeclEnvMesh[0].Type = 2;
	DeclEnvMesh[0].Method = 0;
	DeclEnvMesh[0].Usage = 0;
	DeclEnvMesh[0].UsageIndex = 0;
	DeclEnvMesh[1].Stream = 0;
	DeclEnvMesh[1].Offset = 12;
	DeclEnvMesh[1].Type = 2;
	DeclEnvMesh[1].Method = 0;
	DeclEnvMesh[1].Usage = 3;
	DeclEnvMesh[1].UsageIndex = 0;
	DeclEnvMesh[2].Stream = 0;
	DeclEnvMesh[2].Offset = 24;
	DeclEnvMesh[2].Type = 1;
	DeclEnvMesh[2].Method = 0;
	DeclEnvMesh[2].Usage = 5;
	DeclEnvMesh[2].UsageIndex = 0;
	DeclEnvMesh[3].Stream = 255;
	DeclEnvMesh[3].Offset = 0;
	DeclEnvMesh[3].Type = 17;
	DeclEnvMesh[3].Method = 0;
	DeclEnvMesh[3].Usage = 0;
	DeclEnvMesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclBumpMesh[4];
	DeclBumpMesh[0].Stream = 0;
	DeclBumpMesh[0].Offset = 0;
	DeclBumpMesh[0].Type = 2;
	DeclBumpMesh[0].Method = 0;
	DeclBumpMesh[0].Usage = 0;
	DeclBumpMesh[0].UsageIndex = 0;
	DeclBumpMesh[1].Stream = 0;
	DeclBumpMesh[1].Offset = 12;
	DeclBumpMesh[1].Type = 2;
	DeclBumpMesh[1].Method = 0;
	DeclBumpMesh[1].Usage = 3;
	DeclBumpMesh[1].UsageIndex = 0;
	DeclBumpMesh[2].Stream = 0;
	DeclBumpMesh[2].Offset = 24;
	DeclBumpMesh[2].Type = 1;
	DeclBumpMesh[2].Method = 0;
	DeclBumpMesh[2].Usage = 5;
	DeclBumpMesh[2].UsageIndex = 0;
	DeclBumpMesh[3].Stream = 255;
	DeclBumpMesh[3].Offset = 0;
	DeclBumpMesh[3].Type = 17;
	DeclBumpMesh[3].Method = 0;
	DeclBumpMesh[3].Usage = 0;
	DeclBumpMesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclUV2Mesh[4];
	DeclUV2Mesh[0].Stream = 0;
	DeclUV2Mesh[0].Offset = 0;
	DeclUV2Mesh[0].Type = 2;
	DeclUV2Mesh[0].Method = 0;
	DeclUV2Mesh[0].Usage = 0;
	DeclUV2Mesh[0].UsageIndex = 0;
	DeclUV2Mesh[1].Stream = 0;
	DeclUV2Mesh[1].Offset = 12;
	DeclUV2Mesh[1].Type = 1;
	DeclUV2Mesh[1].Method = 0;
	DeclUV2Mesh[1].Usage = 5;
	DeclUV2Mesh[1].UsageIndex = 0;
	DeclUV2Mesh[2].Stream = 0;
	DeclUV2Mesh[2].Offset = 20;
	DeclUV2Mesh[2].Type = 1;
	DeclUV2Mesh[2].Method = 0;
	DeclUV2Mesh[2].Usage = 5;
	DeclUV2Mesh[2].UsageIndex = 1;
	DeclUV2Mesh[3].Stream = 255;
	DeclUV2Mesh[3].Offset = 0;
	DeclUV2Mesh[3].Type = 17;
	DeclUV2Mesh[3].Method = 0;
	DeclUV2Mesh[3].Usage = 0;
	DeclUV2Mesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclWaterFallMesh[5];
	DeclWaterFallMesh[0].Stream = 0;
	DeclWaterFallMesh[0].Offset = 0;
	DeclWaterFallMesh[0].Type = 2;
	DeclWaterFallMesh[0].Method = 0;
	DeclWaterFallMesh[0].Usage = 0;
	DeclWaterFallMesh[0].UsageIndex = 0;
	DeclWaterFallMesh[1].Stream = 0;
	DeclWaterFallMesh[1].Offset = 12;
	DeclWaterFallMesh[1].Type = 2;
	DeclWaterFallMesh[1].Method = 0;
	DeclWaterFallMesh[1].Usage = 3;
	DeclWaterFallMesh[1].UsageIndex = 0;
	DeclWaterFallMesh[2].Stream = 0;
	DeclWaterFallMesh[2].Offset = 24;
	DeclWaterFallMesh[2].Type = 4;
	DeclWaterFallMesh[2].Method = 0;
	DeclWaterFallMesh[2].Usage = 10;
	DeclWaterFallMesh[2].UsageIndex = 0;
	DeclWaterFallMesh[3].Stream = 0;
	DeclWaterFallMesh[3].Offset = 28;
	DeclWaterFallMesh[3].Type = 1;
	DeclWaterFallMesh[3].Method = 0;
	DeclWaterFallMesh[3].Usage = 5;
	DeclWaterFallMesh[3].UsageIndex = 0;
	DeclWaterFallMesh[4].Stream = 255;
	DeclWaterFallMesh[4].Offset = 0;
	DeclWaterFallMesh[4].Type = 17;
	DeclWaterFallMesh[4].Method = 0;
	DeclWaterFallMesh[4].Usage = 0;
	DeclWaterFallMesh[4].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclLODMesh[3];
	DeclLODMesh[0].Stream = 0;
	DeclLODMesh[0].Offset = 0;
	DeclLODMesh[0].Type = 2;
	DeclLODMesh[0].Method = 0;
	DeclLODMesh[0].Usage = 0;
	DeclLODMesh[0].UsageIndex = 0;
	DeclLODMesh[1].Stream = 0;
	DeclLODMesh[1].Offset = 12;
	DeclLODMesh[1].Type = 1;
	DeclLODMesh[1].Method = 0;
	DeclLODMesh[1].Usage = 5;
	DeclLODMesh[1].UsageIndex = 0;
	DeclLODMesh[2].Stream = 255;
	DeclLODMesh[2].Offset = 0;
	DeclLODMesh[2].Type = 17;
	DeclLODMesh[2].Method = 0;
	DeclLODMesh[2].Usage = 0;
	DeclLODMesh[2].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclNormalMesh[4];
	DeclNormalMesh[0].Stream = 0;
	DeclNormalMesh[0].Offset = 0;
	DeclNormalMesh[0].Type = 2;
	DeclNormalMesh[0].Method = 0;
	DeclNormalMesh[0].Usage = 0;
	DeclNormalMesh[0].UsageIndex = 0;
	DeclNormalMesh[1].Stream = 0;
	DeclNormalMesh[1].Offset = 12;
	DeclNormalMesh[1].Type = 2;
	DeclNormalMesh[1].Method = 0;
	DeclNormalMesh[1].Usage = 3;
	DeclNormalMesh[1].UsageIndex = 0;
	DeclNormalMesh[2].Stream = 0;
	DeclNormalMesh[2].Offset = 24;
	DeclNormalMesh[2].Type = 1;
	DeclNormalMesh[2].Method = 0;
	DeclNormalMesh[2].Usage = 5;
	DeclNormalMesh[2].UsageIndex = 0;
	DeclNormalMesh[3].Stream = 255;
	DeclNormalMesh[3].Offset = 0;
	DeclNormalMesh[3].Type = 17;
	DeclNormalMesh[3].Method = 0;
	DeclNormalMesh[3].Usage = 0;
	DeclNormalMesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclSpecNormalMesh[4];
	DeclSpecNormalMesh[0].Stream = 0;
	DeclSpecNormalMesh[0].Offset = 0;
	DeclSpecNormalMesh[0].Type = 2;
	DeclSpecNormalMesh[0].Method = 0;
	DeclSpecNormalMesh[0].Usage = 0;
	DeclSpecNormalMesh[0].UsageIndex = 0;
	DeclSpecNormalMesh[1].Stream = 0;
	DeclSpecNormalMesh[1].Offset = 12;
	DeclSpecNormalMesh[1].Type = 2;
	DeclSpecNormalMesh[1].Method = 0;
	DeclSpecNormalMesh[1].Usage = 3;
	DeclSpecNormalMesh[1].UsageIndex = 0;
	DeclSpecNormalMesh[2].Stream = 0;
	DeclSpecNormalMesh[2].Offset = 24;
	DeclSpecNormalMesh[2].Type = 1;
	DeclSpecNormalMesh[2].Method = 0;
	DeclSpecNormalMesh[2].Usage = 5;
	DeclSpecNormalMesh[2].UsageIndex = 0;
	DeclSpecNormalMesh[3].Stream = 255;
	DeclSpecNormalMesh[3].Offset = 0;
	DeclSpecNormalMesh[3].Type = 17;
	DeclSpecNormalMesh[3].Method = 0;
	DeclSpecNormalMesh[3].Usage = 0;
	DeclSpecNormalMesh[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclSpecWaterFall2Mesh[5];
	DeclSpecWaterFall2Mesh[0].Stream = 0;
	DeclSpecWaterFall2Mesh[0].Offset = 0;
	DeclSpecWaterFall2Mesh[0].Type = 2;
	DeclSpecWaterFall2Mesh[0].Method = 0;
	DeclSpecWaterFall2Mesh[0].Usage = 0;
	DeclSpecWaterFall2Mesh[0].UsageIndex = 0;
	DeclSpecWaterFall2Mesh[1].Stream = 0;
	DeclSpecWaterFall2Mesh[1].Offset = 12;
	DeclSpecWaterFall2Mesh[1].Type = 4;
	DeclSpecWaterFall2Mesh[1].Method = 0;
	DeclSpecWaterFall2Mesh[1].Usage = 10;
	DeclSpecWaterFall2Mesh[1].UsageIndex = 0;
	DeclSpecWaterFall2Mesh[2].Stream = 0;
	DeclSpecWaterFall2Mesh[2].Offset = 16;
	DeclSpecWaterFall2Mesh[2].Type = 1;
	DeclSpecWaterFall2Mesh[2].Method = 0;
	DeclSpecWaterFall2Mesh[2].Usage = 5;
	DeclSpecWaterFall2Mesh[2].UsageIndex = 0;
	DeclSpecWaterFall2Mesh[3].Stream = 0;
	DeclSpecWaterFall2Mesh[3].Offset = 24;
	DeclSpecWaterFall2Mesh[3].Type = 1;
	DeclSpecWaterFall2Mesh[3].Method = 0;
	DeclSpecWaterFall2Mesh[3].Usage = 5;
	DeclSpecWaterFall2Mesh[3].UsageIndex = 1;
	DeclSpecWaterFall2Mesh[4].Stream = 255;
	DeclSpecWaterFall2Mesh[4].Offset = 0;
	DeclSpecWaterFall2Mesh[4].Type = 17;
	DeclSpecWaterFall2Mesh[4].Method = 0;
	DeclSpecWaterFall2Mesh[4].Usage = 0;
	DeclSpecWaterFall2Mesh[4].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclSpecWater[5];
	DeclSpecWater[0].Stream = 0;
	DeclSpecWater[0].Offset = 0;
	DeclSpecWater[0].Type = 2;
	DeclSpecWater[0].Method = 0;
	DeclSpecWater[0].Usage = 0;
	DeclSpecWater[0].UsageIndex = 0;
	DeclSpecWater[1].Stream = 0;
	DeclSpecWater[1].Offset = 12;
	DeclSpecWater[1].Type = 2;
	DeclSpecWater[1].Method = 0;
	DeclSpecWater[1].Usage = 3;
	DeclSpecWater[1].UsageIndex = 0;
	DeclSpecWater[2].Stream = 0;
	DeclSpecWater[2].Offset = 24;
	DeclSpecWater[2].Type = 1;
	DeclSpecWater[2].Method = 0;
	DeclSpecWater[2].Usage = 5;
	DeclSpecWater[2].UsageIndex = 0;
	DeclSpecWater[3].Stream = 0;
	DeclSpecWater[3].Offset = 32;
	DeclSpecWater[3].Type = 1;
	DeclSpecWater[3].Method = 0;
	DeclSpecWater[3].Usage = 5;
	DeclSpecWater[3].UsageIndex = 1;
	DeclSpecWater[4].Stream = 255;
	DeclSpecWater[4].Offset = 0;
	DeclSpecWater[4].Type = 17;
	DeclSpecWater[4].Method = 0;
	DeclSpecWater[4].Usage = 0;
	DeclSpecWater[4].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclChannel2Alpha[4];
	DeclChannel2Alpha[0].Stream = 0;
	DeclChannel2Alpha[0].Offset = 0;
	DeclChannel2Alpha[0].Type = 2;
	DeclChannel2Alpha[0].Method = 0;
	DeclChannel2Alpha[0].Usage = 0;
	DeclChannel2Alpha[0].UsageIndex = 0;
	DeclChannel2Alpha[1].Stream = 0;
	DeclChannel2Alpha[1].Offset = 12;
	DeclChannel2Alpha[1].Type = 1;
	DeclChannel2Alpha[1].Method = 0;
	DeclChannel2Alpha[1].Usage = 5;
	DeclChannel2Alpha[1].UsageIndex = 0;
	DeclChannel2Alpha[2].Stream = 0;
	DeclChannel2Alpha[2].Offset = 20;
	DeclChannel2Alpha[2].Type = 1;
	DeclChannel2Alpha[2].Method = 0;
	DeclChannel2Alpha[2].Usage = 5;
	DeclChannel2Alpha[2].UsageIndex = 1;
	DeclChannel2Alpha[3].Stream = 255;
	DeclChannel2Alpha[3].Offset = 0;
	DeclChannel2Alpha[3].Type = 17;
	DeclChannel2Alpha[3].Method = 0;
	DeclChannel2Alpha[3].Usage = 0;
	DeclChannel2Alpha[3].UsageIndex = 0;

	D3DVERTEXELEMENT9 DeclRefectWater[5];
	DeclRefectWater[0].Stream = 0;
	DeclRefectWater[0].Offset = 0;
	DeclRefectWater[0].Type = 2;
	DeclRefectWater[0].Method = 0;
	DeclRefectWater[0].Usage = 0;
	DeclRefectWater[0].UsageIndex = 0;
	DeclRefectWater[1].Stream = 0;
	DeclRefectWater[1].Offset = 12;
	DeclRefectWater[1].Type = 4;
	DeclRefectWater[1].Method = 0;
	DeclRefectWater[1].Usage = 10;
	DeclRefectWater[1].UsageIndex = 0;
	DeclRefectWater[2].Stream = 0;
	DeclRefectWater[2].Offset = 16;
	DeclRefectWater[2].Type = 1;
	DeclRefectWater[2].Method = 0;
	DeclRefectWater[2].Usage = 5;
	DeclRefectWater[2].UsageIndex = 0;
	DeclRefectWater[3].Stream = 0;
	DeclRefectWater[3].Offset = 24;
	DeclRefectWater[3].Type = 1;
	DeclRefectWater[3].Method = 0;
	DeclRefectWater[3].Usage = 5;
	DeclRefectWater[3].UsageIndex = 1;
	DeclRefectWater[4].Stream = 255;
	DeclRefectWater[4].Offset = 0;
	DeclRefectWater[4].Type = 17;
	DeclRefectWater[4].Method = 0;
	DeclRefectWater[4].Usage = 0;
	DeclRefectWater[4].UsageIndex = 0;

	LPD3DVERTEXELEMENT9 VertexDeclEff[19];
	VertexDeclEff[0] = DeclWater;
	VertexDeclEff[1] = DeclEquip;
	VertexDeclEff[2] = DeclEquipShadow;
	VertexDeclEff[3] = DeclBlur;
	VertexDeclEff[4] = DeclMesh;
	VertexDeclEff[5] = DeclSwing;
	VertexDeclEff[6] = DeclEfMesh;
	VertexDeclEff[7] = DeclBumpEquip;
	VertexDeclEff[8] = DeclEnvMesh;
	VertexDeclEff[9] = DeclBumpMesh;
	VertexDeclEff[10] = DeclUV2Mesh;
	VertexDeclEff[11] = DeclWaterFallMesh;
	VertexDeclEff[12] = DeclLODMesh;
	VertexDeclEff[13] = DeclNormalMesh;
	VertexDeclEff[14] = DeclSpecNormalMesh;
	VertexDeclEff[15] = DeclSpecWaterFall2Mesh;
	VertexDeclEff[16] = DeclSpecWater;
	VertexDeclEff[17] = DeclChannel2Alpha;
	VertexDeclEff[18] = DeclRefectWater;

	if (m_pVSEffect[0] == nullptr)
	{
		for (int j = 0; j < 4; ++j)
		{
			LPD3DXBUFFER pCode = nullptr;
			char szFileName[128];
			sprintf_s(szFileName, ShaderVertexShader_Path, j + 1);

			int fh = _open(szFileName, _O_BINARY);
			if (fh == -1)
			{
				LOG_WRITELOG("Read VertexShader %d Error.. \r\n", j + 1);
				return 0;
			}

			int nLength = _filelength(fh);
			D3DXCreateBuffer(nLength, &pCode);
			_read(fh, pCode->GetBufferPointer(), pCode->GetBufferSize());
			_close(fh);

			if (FAILED(m_pd3dDevice->CreateVertexDeclaration(VertexDeclEff[j], &m_pVDEffect[j])))
				return 0;

			if (FAILED(m_pd3dDevice->CreateVertexShader((const DWORD*)pCode->GetBufferPointer(), &m_pVSEffect[j])))
				return 0;

			pCode->Release();
		}
	}

	return 1;
}

int RenderDevice::InitPixelShader()
{
	D3DCAPS9 d3dCaps{};
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);
	if (d3dCaps.PixelShaderVersion >= 0xFFFF0101)
	{
		if (m_pPSEffect[0] == nullptr)
		{
			for (int i = 0; i < 6; ++i)
			{
				LPD3DXBUFFER pCode = nullptr;
				char szBinFile[128];
				sprintf_s(szBinFile, ShaderPixelShader_Path, i + 1);

				int handle = _open(szBinFile, _O_BINARY);
				if (handle == -1)
				{
					LOG_WRITELOG("Read PixelShader %d Error.. \r\n", i + 1);
					return 0;
				}

				int nLength = _filelength(handle);
				D3DXCreateBuffer(nLength, &pCode);
				_read(handle, pCode->GetBufferPointer(), pCode->GetBufferSize());
				_close(handle);

				if (FAILED(m_pd3dDevice->CreatePixelShader((const DWORD*)pCode->GetBufferPointer(), &m_pPSEffect[i])))
					return 0;

				pCode->Release();
			}
		}

		return 1;
	}
	else
	{
		m_bSupportPS11 = 0;
		m_nShadowTextureSize = 64;
		return 1;
	}

	return 1;
}

int RenderDevice::SetRenderStateBlock(int nIndex)
{
	if (nIndex == 0)
	{
		SetRenderState(D3DRS_ZENABLE, 1u);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetRenderState(D3DRS_LIGHTING, 0);
		SetRenderState(D3DRS_SPECULARENABLE, 1u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_SHADEMODE, 2u);
		SetRenderState(D3DRS_AMBIENT, 0x0AAAAAA);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 7u);
		SetRenderState(D3DRS_COLORVERTEX, 1u);
		SetRenderState(D3DRS_DITHERENABLE, 1u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 1u);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 1u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_pd3dDevice->SetFVF(322);
	}
	else if (nIndex == 1)
	{
		m_light[0].Diffuse = m_colorLight;
		m_light[0].Specular = m_colorLight;
		m_light[1].Diffuse = m_colorBackLight;
		m_light[1].Specular = m_colorBackLight;

		if (g_pCurrentScene != nullptr && g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
		{
			if (static_cast<TMFieldScene*>(g_pCurrentScene)->m_bShowBoss == 1)
			{
				m_light[0].Diffuse.a = 0.0;
				m_light[0].Diffuse.b = 0.0;
				m_light[0].Diffuse.g = 0.0;
				m_light[0].Diffuse.r = 0.0;
				m_light[0].Specular.a = 0.0;
				m_light[0].Specular.b = 0.0;
				m_light[0].Specular.g = 0.0;
				m_light[0].Specular.r = 0.0;
			}
			static_cast<TMFieldScene*>(g_pCurrentScene)->m_bShowBoss = 0;
		}

		m_light[1].Diffuse = m_colorBackLight;
		m_light[1].Specular = m_colorBackLight;
		m_light[0].Direction = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		m_light[1].Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

		for (int i = 0; i < 2; ++i)
		{
			if (FAILED(m_pd3dDevice->SetLight(i, &m_light[i])))
				return 0;

			if (FAILED(m_pd3dDevice->LightEnable(i, 1)))
				return 0;
		}

		D3DXVECTOR4 vLightDir;
		TMCamera* pCamera = g_pObjectManager->m_pCamera;

		if (RenderDevice::m_bDungeon != 0 && RenderDevice::m_bDungeon != 4)
		{
			vLightDir = D3DXVECTOR4(
				pCamera->m_vecCamDir.x - m_light[0].Direction.x, 
				0.5f, 
				(pCamera->m_vecCamDir.z + m_light[0].Direction.z) - 0.3f, 
				0.0f);
		}
		else
		{
			vLightDir = D3DXVECTOR4(
				-m_light[0].Direction.x,
				0.5f,
				m_light[0].Direction.z,
				0.0f);
		}

		m_pd3dDevice->SetVertexShaderConstantF(1, (const float*)&vLightDir, 1);

		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			SetRenderState(D3DRS_ALPHAREF, 0xD);
		}

		SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1);
		SetRenderState(D3DRS_ALPHAFUNC, 7);
		SetRenderState(D3DRS_AMBIENT, 0x33FFFFFFu);
		SetRenderState(D3DRS_ZENABLE, 1);
		SetRenderState(D3DRS_ZFUNC, 4);
		SetRenderState(D3DRS_ZWRITEENABLE, 1);
		SetRenderState(D3DRS_LIGHTING, 1);
		SetRenderState(D3DRS_SPECULARENABLE, 0);
		SetRenderState(D3DRS_CULLMODE, 3);
		SetRenderState(D3DRS_SHADEMODE, 2);
		SetRenderState(D3DRS_COLORVERTEX, 1);
		SetRenderState(D3DRS_NORMALIZENORMALS, 1);
		SetRenderState(D3DRS_DITHERENABLE, 1);
		SetRenderState(D3DRS_SRCBLEND, 2);
		SetRenderState(D3DRS_DESTBLEND, 6);
		SetRenderState(D3DRS_FOGENABLE, m_bFog);
		SetRenderState(D3DRS_RANGEFOGENABLE, 0);

		m_dwActualClearColor = m_dwClearColor;
		float fstart = (float)(m_fFogStart + g_pApp->m_pObjectManager->m_pCamera->m_fSightLength) - 8.0f;
		float fend = (float)(m_fFogEnd + g_pApp->m_pObjectManager->m_pCamera->m_fSightLength) - 15.0f;

		SetRenderState(D3DRS_FOGCOLOR, m_dwClearColor);
		SetRenderState(D3DRS_FOGVERTEXMODE, 3);
		SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fstart));
		SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fend));
		SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 1);
		SetRenderState(D3DRS_SPECULARMATERIALSOURCE, 2);
		SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 1);
		SetRenderState(D3DRS_SPECULARMATERIALSOURCE, 1);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 2u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(1u, D3DTSS_COLORARG2, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(1u, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(1u, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(1u, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(1u, D3DSAMP_MIPFILTER, 0);
		SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);
		SetTextureStageState(2u, D3DTSS_COLOROP, 1u);
	}
	else if (nIndex == 2)
	{
		SetRenderState(D3DRS_ZENABLE, 0);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
		SetRenderState(D3DRS_ALPHAREF, 8u);
		SetRenderState(D3DRS_ALPHAFUNC, 7u);
		SetRenderState(D3DRS_FILLMODE, 3u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_STENCILENABLE, 0);
		SetRenderState(D3DRS_CLIPPING, 1u);
		SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, 0);
		SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, 0);
		SetRenderState(D3DRS_VERTEXBLEND, 0);
		SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
		SetSamplerState(0, D3DSAMP_MINFILTER, 1u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 1u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 1u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
	}
	else if (nIndex == 3)
	{
		SetRenderState(D3DRS_ZENABLE, 0);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
		SetRenderState(D3DRS_ALPHAREF, 8u);
		SetRenderState(D3DRS_ALPHAFUNC, 7u);
		SetRenderState(D3DRS_FILLMODE, 3u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_STENCILENABLE, 0);
		SetRenderState(D3DRS_CLIPPING, 1u);
		SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, 0);
		SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, 0);
		SetRenderState(D3DRS_VERTEXBLEND, 0);
		SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 2u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
	}

	return 1;
}
