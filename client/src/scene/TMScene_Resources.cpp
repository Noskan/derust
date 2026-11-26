#include "core/pch.h"
#include "ui/SControlContainer.h"
#include "ui/SControl.h"
#include "ui/SGrid.h"
#include "ui/UIBinary.h"
#include "world/TMGround.h"
#include "world/TMSky.h"
#include "world/TMSun.h"
#include "render/TMLight.h"
#include "entities/TMHuman.h"
#include "scene/TMObject.h"
#include "render/TMCamera.h"
#include "scene/TMObjectContainer.h"
#include "entities/TMItem.h"
#include "core/TMGlobal.h"
#include "core/TMLog.h"
#include "scene/TMScene.h"
#include "scene/TMFieldScene.h"
#include "effects/TMEffectFirework.h"
#include "world/TMSnow.h"
#include "world/TMRain.h"

/// @file TMScene_Resources.cpp
/// @brief Carga de recursos da cena: layout de UI binário (`ReadRCBin`) e textos de
///        mensagens/quests (`LoadMsgText*`, `LoadMsgLevel`).
///
/// Extraído de `TMScene.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

int TMScene::ReadRCBin(char* szBinFileName)
{
	FILE* fpBinary = nullptr;
	
	fopen_s(&fpBinary, szBinFileName, "rb");

	if (!fpBinary)
		return 0;

	CONTROL_TYPE nControlType{};

	while (fread(&nControlType, 4, 1, fpBinary))
	{
		switch (nControlType)
		{
		case CONTROL_TYPE::CTRL_TYPE_PANEL:
		{
			BinPanel binPanelData;

			if (!fread(&binPanelData, sizeof(binPanelData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pPanel = new SPanel(
				binPanelData.nTextureSetIndex,
				(float)binPanelData.nStartX,
				(float)binPanelData.nStartY,
				(float)binPanelData.nWidth,
				(float)binPanelData.nHeight,
				binPanelData.nColor,
				static_cast<RENDERCTRLTYPE>(binPanelData.nFillType));

			if (!pPanel)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binPanelData.nID, GetSceneType());
				return 0;
			}

			pPanel->SetControlID(binPanelData.nID);
			pPanel->SetCenterPos(binPanelData.nID,
				(float)binPanelData.nStartX,
				(float)binPanelData.nStartY,
				(float)binPanelData.nWidth,
				(float)binPanelData.nHeight);

			if (binPanelData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binPanelData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pPanel));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pPanel));
			}

			pPanel->m_bPickable = binPanelData.nPickable;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_GRID:
		{
			BinGrid binGridData;

			if (!fread(&binGridData, sizeof(binGridData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pGrid = new SGridControl(
				binGridData.nTextureSetIndex,
				binGridData.nRowCount,
				binGridData.nColumnCount,
				(float)binGridData.nStartX,
				(float)binGridData.nStartY,
				(float)binGridData.nWidth,
				(float)binGridData.nHeight,
				static_cast<TMEITEMTYPE>(binGridData.nType));

			if (!pGrid)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binGridData.nID, GetSceneType());
				return 0;
			}

			pGrid->SetControlID(binGridData.nID);

			pGrid->SetCenterPos(binGridData.nID,
				(float)binGridData.nStartX,
				(float)binGridData.nStartY,
				(float)binGridData.nWidth,
				(float)binGridData.nHeight);

			if (m_pControlContainer)
				pGrid->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pGrid->SetEventListener(nullptr);

			if (binGridData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binGridData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pGrid));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pGrid));
			}
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_3DOBJ:
		{
			Bin3DObj bin3DObjData;

			if (!fread(&bin3DObjData, sizeof(bin3DObjData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto p3DObj = new S3DObj(bin3DObjData.n3DObjIndex,
				(float)bin3DObjData.nStartX,
				(float)bin3DObjData.nStartY,
				(float)bin3DObjData.nWidth,
				(float)bin3DObjData.nHeight);

			if (!p3DObj)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", bin3DObjData.nID, GetSceneType());
				return 0;
			}

			p3DObj->SetControlID(bin3DObjData.nID);
			p3DObj->SetCenterPos(bin3DObjData.nID,
				(float)bin3DObjData.nStartX,
				(float)bin3DObjData.nStartY,
				(float)bin3DObjData.nWidth,
				(float)bin3DObjData.nHeight);

			if (bin3DObjData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(bin3DObjData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(p3DObj));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(p3DObj));
			}
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_BUTTON:
		{
			BinButton binButtonData;

			if (!fread(&binButtonData, sizeof(binButtonData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			char strbuf[128]{};

			sprintf_s(strbuf, "%s", g_UIString[binButtonData.nStringIndex]);

			auto pButton = new SButton(
				binButtonData.nTextureSetIndex,
				(float)binButtonData.nStartX,
				(float)binButtonData.nStartY,
				(float)binButtonData.nWidth,
				(float)binButtonData.nHeight,
				binButtonData.nColor,
				binButtonData.nSound,
				strbuf);

			if (!pButton)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binButtonData.nID, GetSceneType());
				return 0;
			}

			pButton->SetControlID(binButtonData.nID);

			pButton->SetCenterPos(binButtonData.nID,
				(float)binButtonData.nStartX,
				(float)binButtonData.nStartY,
				(float)binButtonData.nWidth,
				(float)binButtonData.nHeight);

			if (binButtonData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binButtonData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pButton));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pButton));
			}

			if (m_pControlContainer)
				pButton->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pButton->SetEventListener(nullptr);
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_TEXT:
		{
			BinText binTextData;

			if (!fread(&binTextData, sizeof(binTextData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			char strbuf[128]{};

			sprintf_s(strbuf, "%s", g_UIString[binTextData.nStringIndex]);

			auto pText = new SText(
				binTextData.nTextureSetIndex,
				strbuf,
				binTextData.nFontColor,
				(float)binTextData.nStartX,
				(float)binTextData.nStartY,
				(float)binTextData.nWidth,
				(float)binTextData.nHeight,
				binTextData.nBorder,
				binTextData.nBorderColor,
				binTextData.nTextType,
				binTextData.nAlignType);

			if (!pText)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binTextData.nID, GetSceneType());
				return 0;
			}

			pText->SetControlID(binTextData.nID);

			pText->SetCenterPos(binTextData.nID,
				(float)binTextData.nStartX,
				(float)binTextData.nStartY,
				(float)binTextData.nWidth,
				(float)binTextData.nHeight);

			if (binTextData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binTextData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pText));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pText));
			}
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT:
		{
			BinEdit binEditData;

			if (!fread(&binEditData, sizeof(binEditData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pEdit = new SEditableText(
				binEditData.nTextureSetIndex,
				binEditData.szString,
				binEditData.nMaxStringLength,
				binEditData.nPassword,
				binEditData.nFontColor,
				(float)binEditData.nStartX,
				(float)binEditData.nStartY,
				(float)binEditData.nWidth,
				(float)binEditData.nHeight,
				binEditData.nBorder,
				binEditData.nBorderColor,
				binEditData.nTextType,
				binEditData.nAlignType);

			if (!pEdit)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binEditData.nID, GetSceneType());
				return 0;
			}

			pEdit->SetControlID(binEditData.nID);

			pEdit->SetCenterPos(binEditData.nID,
				(float)binEditData.nStartX,
				(float)binEditData.nStartY,
				(float)binEditData.nWidth,
				(float)binEditData.nHeight);

			if (binEditData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binEditData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pEdit));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pEdit));
			}

			if (m_pControlContainer)
				pEdit->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pEdit->SetEventListener(nullptr);
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR:
		{
			BinProgress binProgressData;

			if (!fread(&binProgressData, sizeof(binProgressData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pProgress = new SProgressBar(
				binProgressData.nTextureSetIndex,
				binProgressData.nCurrent,
				binProgressData.nMaxValue,
				(float)binProgressData.nStartX,
				(float)binProgressData.nStartY,
				(float)binProgressData.nWidth,
				(float)binProgressData.nHeight,
				binProgressData.nProgressColor,
				binProgressData.nColor,
				binProgressData.nStyle);

			if (!pProgress)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binProgressData.nID, GetSceneType());
				return 0;
			}

			pProgress->SetControlID(binProgressData.nID);

			pProgress->SetCenterPos(binProgressData.nID,
				(float)binProgressData.nStartX,
				(float)binProgressData.nStartY,
				(float)binProgressData.nWidth,
				(float)binProgressData.nHeight);

			if (binProgressData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binProgressData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pProgress));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pProgress));
			}

			if (m_pControlContainer)
				pProgress->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pProgress->SetEventListener(nullptr);
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_CHECKBOX:
		{
			BinCheckBox binCheckBoxData;

			if (!fread(&binCheckBoxData, sizeof(binCheckBoxData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pCheckBox = new SCheckBox(
				binCheckBoxData.nTextureSetIndex,
				(float)binCheckBoxData.nStartX,
				(float)binCheckBoxData.nStartY,
				(float)binCheckBoxData.nWidth,
				(float)binCheckBoxData.nHeight,
				binCheckBoxData.nColor);

			if (!pCheckBox)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binCheckBoxData.nID, GetSceneType());
				return 0;
			}

			pCheckBox->SetControlID(binCheckBoxData.nID);

			pCheckBox->SetCenterPos(binCheckBoxData.nID,
				(float)binCheckBoxData.nStartX,
				(float)binCheckBoxData.nStartY,
				(float)binCheckBoxData.nWidth,
				(float)binCheckBoxData.nHeight);

			if (binCheckBoxData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binCheckBoxData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pCheckBox));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pCheckBox));
			}

			if (m_pControlContainer)
				pCheckBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pCheckBox->SetEventListener(nullptr);
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_LISTBOX:
		{
			BinListBox binListBoxData;

			if (!fread(&binListBoxData, sizeof(binListBoxData), 1, fpBinary))
			{
				LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
				fclose(fpBinary);
				return 0;
			}

			auto pListBox = new SListBox(
				binListBoxData.nTextureSetIndex,
				binListBoxData.nMaxCount,
				binListBoxData.nVisibleCount,
				(float)binListBoxData.nStartX,
				(float)binListBoxData.nStartY,
				(float)binListBoxData.nWidth,
				(float)binListBoxData.nHeight,
				binListBoxData.nColor,
				static_cast<RENDERCTRLTYPE>(binListBoxData.nFillType),
				binListBoxData.nSelect,
				binListBoxData.nScroll,
				0);

			if (!pListBox)
			{
				LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binListBoxData.nID, GetSceneType());
				return 0;
			}

			pListBox->SetControlID(binListBoxData.nID);

			pListBox->SetCenterPos(binListBoxData.nID,
				(float)binListBoxData.nStartX,
				(float)binListBoxData.nStartY,
				(float)binListBoxData.nWidth,
				(float)binListBoxData.nHeight);

			if (binListBoxData.nParentID)
			{
				auto pParent = m_pControlContainer->FindControl(binListBoxData.nParentID);

				if (pParent)
					pParent->AddChild(static_cast<TreeNode*>(pListBox));
			}
			else
			{
				m_pControlContainer->AddItem(static_cast<SControl*>(pListBox));
			}

			if (m_pControlContainer)
				pListBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
			else
				pListBox->SetEventListener(nullptr);
		}
		break;
		default:
			LOG_WRITELOG("Not Support Control Type [%d]\r\n", nControlType);
			return 0;
		}
	}

	fclose(fpBinary);
	return 1;
}

int TMScene::LoadMsgText(SListBox* pListBox, const char* szFileName)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[7]{};

		strncpy(szCol, szTemp, 6);

		DWORD dwCol{};

		sscanf(szCol, "%x", &dwCol);

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[6] == 32)
		{
			sprintf_s(szText, "%s", &szTemp[6]);
			
			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

int TMScene::LoadMsgText2(SListBox* pListBox, const char* szFileName, int nStartLine, int nEndLine)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	int nCount = 0;

	pListBox->Empty();

	for (int i = 0; i < 560 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[7]{};
		
		strncpy(szCol, szTemp, 6);

		DWORD dwCol{};

		sscanf(szCol, "%x", &dwCol);

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[6] == 32 && i >= nStartLine && i <= nEndLine)
		{
			sprintf_s(szText, "%s", &szTemp[6]);

			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));

			if (++nCount > 100)
				break;
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

int TMScene::LoadMsgText3(SListBox* pListBox, const char* szFileName, int nLv, int ntrans)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	int nLvLimit = 300;

	pListBox->Empty();

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};
		
		strncpy(szCol, szTemp, 10);

		DWORD dwCol{};

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		if (nLvLimit > nLv)
			dwCol = 0xFF777777;

		if (nLvLimit > 350 && ntrans < 6)
			dwCol = 0xFF777777;

		if (g_pCurrentScene)
		{
			if (m_pMyHuman->Is2stClass() == 2)
				dwCol = 0xFF777777;

			else if (nLvLimit > nLv)
				dwCol = 0xFF777777;
		}

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[10] == 32)
		{
			sprintf(szText, "%s", &szTemp[10]);
			
			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

unsigned int TMScene::LoadMsgText4(char* pStr, int dwStrSize, const char* szFileName, int nLv, int ntrans)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	int nLvLimit = 300;
	DWORD dwCol = 0;

	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};

		strncpy(szCol, szTemp, 10);

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		if (nLvLimit == nLv)
		{
			if (dwCol == 0xFFAAAA && ntrans >= 6)
			{
				dwCol = 0;
			}
			else if (nLvLimit <= 350 || ntrans >= 6)
			{
				char* szRet = strstr(szTemp, "\n");

				if (szRet)
					*szRet = 0;

				if (szTemp[10] == 32)
					sprintf_s(pStr, dwStrSize, "%s", &szTemp[10]);
			}
			else
			{
				dwCol = 0;
			}
			
			break;
		}
	}

	fclose(fp);

	return dwCol | 0xFF000000;
}

int TMScene::LoadMsgLevel(char* LevelQuest, const char* szFileName, char cType)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};

		strncpy(szCol, szTemp, 10);

		int nLvLimit{};
		DWORD dwCol{};

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		LevelQuest[nLvLimit - 1] = cType;
	}
	fclose(fp);
	return 1;
}
