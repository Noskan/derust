#include "core/pch.h"
#include "core/EventTranslator.h"
#include "ui/SGrid.h"
#include "ui/SControl.h"
#include "core/TMGlobal.h"
#include "common/Basedef.h"
#include "ui/SControlContainer.h"
#include "scene/TMFieldScene.h"

/// @file SControl_Text.cpp
/// @brief Controles de texto: `SText` (rótulos) e `SEditableText` (campos editáveis com IME).
///
/// Extraído de `SControl.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

SText::SText(int inTextureSetIndex, const char* istrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_GCText = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, inTextureSetIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0, idwFontColor);
	m_GCText2 = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, inTextureSetIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0, idwFontColor);
	m_GCText3 = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, inTextureSetIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0, idwFontColor);
	m_GCText4 = GeomControl(RENDERCTRLTYPE::RENDER_TEXT, inTextureSetIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0, idwFontColor);

	m_dwAlignType = dwAlignType;
	m_dwTextType = dwType;
	m_cBorder = ibBorder;

	m_GCBorder = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, inTextureSetIndex, 0.0f, 0.0f, 0.0f, 0.0f, 0, idwBorderColor);

	m_cComma = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_TEXT;

	SetType(m_dwTextType);

	m_GCText.strString[0] = 0;
	m_GCText.pFont = &m_Font;
	m_GCText2.strString[0] = 0;
	m_GCText2.pFont = &m_Font2;
	m_GCText3.strString[0] = 0;
	m_GCText3.pFont = &m_Font3;
	m_GCText4.strString[0] = 0;
	m_GCText4.pFont = &m_Font4;

	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace((char*)istrText);

		if (!strcmp(istrText, " "))
			istrText = nullptr;
	}
	if (istrText != nullptr)
	{
		if (strlen(istrText) != 0)
		{
			strcpy(m_GCText.strString, istrText);
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}
	}
}

SText::~SText()
{
	if (g_pCurrentScene != nullptr)
	{
		SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;
		if (pControlContainer != nullptr)
		{
			if (m_GCBorder.nLayer >= 0)
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCBorder, m_GCBorder.nLayer);
			if (m_GCText.nLayer >= 0)
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCText, m_GCText.nLayer);

			if (m_cComma == 2)
			{
				if (m_GCText2.nLayer >= 0)
					RemoveRenderControlItem(
						pControlContainer->m_pDrawControl,
						&m_GCText2,
						m_GCText2.nLayer);

				if (m_GCText3.nLayer >= 0)
					RemoveRenderControlItem(
						pControlContainer->m_pDrawControl,
						&m_GCText3,
						m_GCText3.nLayer);

				if (m_GCText4.nLayer >= 0)
					RemoveRenderControlItem(
						pControlContainer->m_pDrawControl,
						&m_GCText4,
						m_GCText4.nLayer);
			}
		}
	}
}

void SText::SetText(char* istrText, int bCheckZero)
{
	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace(istrText);
		if (!strcmp(istrText, " "))
			istrText = 0;
	}

	size_t count = 0;

	if (istrText == nullptr)
	{
		sprintf(m_GCText.strString, "");
		m_Font.SetText(m_GCText.strString, m_GCText.dwColor, bCheckZero);

		return;
	}
	else if (!strcmp(m_GCText.strString, istrText) && !m_cComma)
		return;

	if (strlen(istrText) != 0 && (m_cComma == 2 || m_cComma == 1))
	{
		for (size_t i = 0; i < strlen(istrText); ++i)
		{
			if (!isdigit(istrText[i]) && istrText[i] != ' ')
				++count;
		}

		if (!count && atoi(istrText))
		{
			char str[100]{};
			for (size_t i = 0; i < strlen(istrText); ++i)
			{
				if (istrText[i] != ' ')
					continue;

				++count;
				for (size_t j = i; j < strlen(istrText); ++j)
				{
					istrText[j] = istrText[j + 1];
				}
				i -= 1;
			}

			size_t i = 0;
			int j = 0;
			while (1)
			{
				if (i >= strlen(istrText))
					break;

				if (((strlen(istrText) - i) % 3) || i == 0)
					str[j++] = istrText[i];
				else
				{
					str[j] = ',';
					str[j + 1] = istrText[i];
					j += 2;
				}
				++i;
			}

			strcpy(istrText, str);

			if (count > 0)
			{
				char temp1[128]{};
				char temp2[128]{};
				strcpy(temp1, istrText);

				for (i = 0; i < count; ++i)
					temp2[i] = ' ';

				strcat(temp2, temp1);
				strcpy(istrText, temp2);
			}
		}
	}

	if (strlen(istrText) > 255)
		istrText[255] = 0;

	strcpy(m_GCText.strString, istrText);
	m_Font.SetText(m_GCText.strString, m_GCText.dwColor, bCheckZero);

	if (m_cComma == 2)
	{
		int nStartPos = strlen(istrText);
		memset(m_GCText.strString, 0, sizeof(m_GCText.strString));
		memset(m_GCText2.strString, 0, sizeof(m_GCText2.strString));
		memset(m_GCText3.strString, 0, sizeof(m_GCText3.strString));
		memset(m_GCText4.strString, 0, sizeof(m_GCText4.strString));

		int i = 0;
		int count = 0;
		while (i < nStartPos)
		{
			if (istrText[i] == ',')
				++count;

			if (i >= 255)
				break;

			if (count != 0)
				m_GCText.strString[i] = ' ';
			else
				m_GCText.strString[i] = istrText[i];
			if (count == 1)
				m_GCText2.strString[i] = istrText[i];
			else
				m_GCText2.strString[i] = ' ';
			if (count == 2)
				m_GCText3.strString[i] = istrText[i];
			else
				m_GCText3.strString[i] = ' ';
			if (count <= 2)
				m_GCText4.strString[i] = ' ';
			else
				m_GCText4.strString[i] = istrText[i];

			++i;
		}

		m_Font.SetText(m_GCText.strString, 0xFFDDFFDD, bCheckZero);
		m_Font2.SetText(m_GCText2.strString, 0xFF88FF88, bCheckZero);
		m_Font3.SetText(m_GCText3.strString, 0xFFFFFF66, bCheckZero);
		m_Font4.SetText(m_GCText4.strString, 0xFFFFFFFF, bCheckZero);
	}
}

void SText::SetTextColor(unsigned int dwFontColor)
{
	if (RenderDevice::m_nBright > 58)
	{
		int nR = WYDCOLOR_RED(dwFontColor) - (RenderDevice::m_nBright - 40);
		int nG = WYDCOLOR_GREEN(dwFontColor) - (RenderDevice::m_nBright - 40);
		int nB = WYDCOLOR_BLUE(dwFontColor) - (RenderDevice::m_nBright - 40);

		if (nR < 0)
			nR = 0;
		if (nG < 0)
			nG = 0;
		if (nB < 0)
			nB = 0;

		dwFontColor = nB | (nG << 8) | dwFontColor & 0xFF000000 | (nR << 16);
	}

	m_GCText.dwColor = dwFontColor;
	m_GCText.pFont->m_dwColor = dwFontColor;
}

char* SText::GetText()
{
	return m_GCText.strString;
}

void SText::SetType(unsigned int dwType)
{
	m_dwTextType = dwType;
	if (m_dwTextType == 0)
	{
		m_GCText.eRenderType = RENDERCTRLTYPE::RENDER_TEXT;
		m_GCText2.eRenderType = RENDERCTRLTYPE::RENDER_TEXT;
		m_GCText3.eRenderType = RENDERCTRLTYPE::RENDER_TEXT;
		m_GCText4.eRenderType = RENDERCTRLTYPE::RENDER_TEXT;
	}
	else if (m_dwTextType == 1)
	{
		m_GCText.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText2.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText3.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText4.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
	}
	else if (m_dwTextType == 2)
	{
		m_GCText.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText2.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText3.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCText4.eRenderType = RENDERCTRLTYPE::RENDER_SHADOW;
		m_GCBorder.nTextureSetIndex = -2;
		m_GCText2.nTextureSetIndex = -2;
		m_GCText3.nTextureSetIndex = -2;
		m_GCText4.nTextureSetIndex = -2;
	}
}

void SText::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	SetType(m_dwTextType);
	if (m_cBorder == 1)
	{
		m_GCBorder.nPosX = ivParentPos.x + m_nPosX;
		m_GCBorder.nPosY = ivParentPos.y + m_nPosY;
		m_GCBorder.nWidth = m_nWidth;
		m_GCBorder.nHeight = m_nHeight;
		m_GCBorder.nLayer = inParentLayer;

		if ((float)(m_GCBorder.nPosX + m_GCBorder.nWidth) < 0.0f || 
			(float)(m_GCBorder.nPosY + m_GCBorder.nHeight) < 0.0f ||
			m_GCBorder.nPosX > (float)(800.0f * RenderDevice::m_fWidthRatio) ||
			m_GCBorder.nPosY > (float)(600.0f * RenderDevice::m_fHeightRatio))
		{
			return;
		}

		AddRenderControlItem(pDrawList, &m_GCBorder, inParentLayer);
	}

	float fWidthRatio = RenderDevice::m_fWidthRatio;
	float fHeightRatio = RenderDevice::m_fHeightRatio;

	if (m_dwAlignType == 0)
	{
		m_GCText.nPosX = (float)(ivParentPos.x + m_nPosX) + (8.0f * fWidthRatio);
		if (m_cComma == 2)
		{
			m_GCText2.nPosX = (float)(ivParentPos.x + m_nPosX) + (8.0f * fWidthRatio);
			m_GCText3.nPosX = (float)(ivParentPos.x + m_nPosX) + (8.0f * fWidthRatio);
			m_GCText4.nPosX = (float)(ivParentPos.x + m_nPosX) + (8.0f * fWidthRatio);
		}
	}
	else if (m_dwAlignType == 1)
	{				
		// This code is from chinese cliente version.
		//int len = strlen(m_GCText.strString);
		//m_GCText.nPosX = (float)(ivParentPos.x + m_nPosX)
		//	+ (float)((float)(m_nWidth - (float)((float)(6 * len) * fHeightRatio)) / 2.0f);
				
		// This code is from WYD BR version.
		SIZE size;
		GetTextExtentPoint32(g_pDevice->m_hDC, m_GCText.strString, strlen(m_GCText.strString), &size);
		m_GCText.nPosX = (float)(ivParentPos.x + m_nPosX) + ((m_nWidth - (float)size.cx) / 2.0f);

		if (m_cComma == 2)
		{
			SIZE size2;
			GetTextExtentPoint32(g_pDevice->m_hDC, m_GCText2.strString, strlen(m_GCText2.strString), &size2);
			SIZE size3;
			GetTextExtentPoint32(g_pDevice->m_hDC, m_GCText3.strString, strlen(m_GCText3.strString), &size3);
			SIZE size4;
			GetTextExtentPoint32(g_pDevice->m_hDC, m_GCText4.strString, strlen(m_GCText4.strString), &size4);

			auto fullSize = size2.cx + size3.cx + size4.cx;
			m_GCText.nPosX = (float)(ivParentPos.x + m_nPosX) + ((m_nWidth - (float)fullSize) / 2.0f);
			m_GCText2.nPosX = (float)(size2.cx / 2) + m_GCText.nPosX;
			m_GCText3.nPosX = (float)(size3.cx / 2) + m_GCText2.nPosX;
			m_GCText4.nPosX = (float)(size4.cx / 2) + m_GCText3.nPosX;
		}
	}
	else if (m_dwAlignType == 2)
	{
		int len = strlen(m_GCText.strString);
		m_GCText.nPosX = (float)((float)(ivParentPos.x + m_nPosX) + m_nWidth)
			- (float)((float)(8 * len + 8) * fWidthRatio);

		if (m_cComma == 2)
		{
			m_GCText2.nPosX = (float)((float)(ivParentPos.x + m_nPosX) + m_nWidth)
				- (float)((float)(8 * len + 8) * fWidthRatio);
			m_GCText3.nPosX = (float)((float)(ivParentPos.x + m_nPosX) + m_nWidth)
				- (float)((float)(8 * len + 8) * fWidthRatio);
			m_GCText4.nPosX = (float)((float)(ivParentPos.x + m_nPosX) + m_nWidth)
				- (float)((float)(8 * len + 8) * fWidthRatio);
		}
	}
	else if (m_dwAlignType == 3)
	{
		m_GCText.nPosX = (float)(ivParentPos.x + m_nPosX)
			+ (float)(2.0f * RenderDevice::m_fWidthRatio);

		if (m_cComma == 2)
		{
			m_GCText2.nPosX = (float)(ivParentPos.x + m_nPosX) + (2.0f * fWidthRatio);
			m_GCText3.nPosX = (float)(ivParentPos.x + m_nPosX) + (2.0f * fWidthRatio);
			m_GCText4.nPosX = (float)(ivParentPos.x + m_nPosX) + (2.0f * fWidthRatio);
		}
	}
	else if (m_dwAlignType == 4)
	{
		m_GCText.nPosX = (float)((float)(ivParentPos.x + m_nPosX)
			+ (float)(8.0f * RenderDevice::m_fWidthRatio))
			+ (float)104;
		if (m_cComma == 2)
		{
			m_GCText2.nPosX = (float)(ivParentPos.x + m_nPosX + (8.0f * fWidthRatio))
				+ (float)104;
			m_GCText3.nPosX = (float)(ivParentPos.x + m_nPosX + (8.0f * fWidthRatio))
				+ (float)104;
			m_GCText4.nPosX = (float)(ivParentPos.x + m_nPosX + (8.0f * fWidthRatio))
				+ (float)104;
		}
	}

	m_GCText.nPosY = (float)(ivParentPos.y + m_nPosY)
		+ (float)((float)((float)(m_nHeight - (float)(16.0f * fHeightRatio)) / 2.0f) + 2.0f);
	m_GCText.nWidth = m_nWidth;
	m_GCText.nHeight = m_nHeight;
	m_GCText.nLayer = inParentLayer;

	if ((float)(m_GCText.nPosX + m_GCText.nWidth) >= 0.0f && 
		(float)(m_GCText.nPosY + m_GCText.nHeight) >= 0.0f &&
		m_GCText.nPosX <= (float)(800.0f * RenderDevice::m_fWidthRatio) && 
		m_GCText.nPosY <= (float)(600.0f * RenderDevice::m_fHeightRatio))
	{		
		AddRenderControlItem(pDrawList, &m_GCText, inParentLayer);

		if (m_cComma == 2)
		{
			m_GCText2.nPosY = ((ivParentPos.y + m_nPosY) +
				(((m_nHeight - (16.0f * fHeightRatio)) / 2.0f) + 2.0f));
			m_GCText2.nWidth = m_nWidth;
			m_GCText2.nHeight = m_nHeight;
			m_GCText2.nLayer = inParentLayer; 
			AddRenderControlItem(pDrawList, &m_GCText2, inParentLayer);

			m_GCText3.nPosY = ((ivParentPos.y + m_nPosY)
				+ (((m_nHeight - (16.0f * fHeightRatio)) / 2.0f)
					+ 2.0f));
			m_GCText3.nWidth = m_nWidth;
			m_GCText3.nHeight = m_nHeight;
			m_GCText3.nLayer = inParentLayer;
			AddRenderControlItem(pDrawList, &m_GCText3, inParentLayer);

			m_GCText4.nPosY = ((ivParentPos.y + m_nPosY)
				+ (((m_nHeight - (16.0f * fHeightRatio)) / 2.0f)
					+ 2.0f));
			m_GCText4.nWidth = m_nWidth;
			m_GCText4.nHeight = m_nHeight;
			m_GCText4.nLayer = inParentLayer;
			AddRenderControlItem(pDrawList, &m_GCText4, inParentLayer);
		}		
	}
}

int SText::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	Update();
	return 0;
}

SEditableText::SEditableText(int inTextureSetIndex, const char* istrText, size_t inMaxStringLen, int ibPasswd, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType)
	: SText(inTextureSetIndex,
		0,
		idwFontColor,
		inX,
		inY,
		inWidth,
		inHeight,
		ibBorder,
		idwBorderColor,
		dwType,
		dwAlignType)
{
	m_cTempChar = 0;
	m_cReserved = 0;
	m_nMaxStringLen = inMaxStringLen;
	m_nCursorVisible = 0;
	m_bPasswd = ibPasswd;
	m_bEncrypt = 0;
	m_bKorean = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT;
	m_nMaxStringLen = m_nMaxStringLen;

	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace(const_cast<char*>(istrText));
		if (!strcmp(istrText, " "))
			istrText = nullptr;
	}

	int nCopyLen = 0;
	if (istrText != nullptr)
	{
		nCopyLen = strlen(istrText) <= inMaxStringLen ? strlen(istrText) : inMaxStringLen;
		strncpy(m_strText, istrText, nCopyLen);
	}
	else
		nCopyLen = 0;

	m_strText[nCopyLen] = 0;
	m_strComposeText[0] = 0;
}

SEditableText::~SEditableText()
{
}

void SEditableText::SetText(char* istrText)
{
	if (istrText[0] == 0)
	{
		memset(m_strText, 0, sizeof(m_strText));
		Update();
		return;
	}

	int nCopyLen = strlen(istrText) <= m_nMaxStringLen ? strlen(istrText) : m_nMaxStringLen;

	strncpy(m_strText, istrText, nCopyLen);
	m_strText[nCopyLen] = 0;
	Update();
}

char* SEditableText::GetText()
{
	return m_strText;
}

int SEditableText::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	int bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (dwFlags != WM_LBUTTONDOWN)
		return SText::OnMouseEvent(dwFlags, wParam, nX, nY);

	if (bOver != 1)
		return 0;

	m_bFocused = 1;
	return 1;
}

int SEditableText::OnCharEvent(char iCharCode, int lParam)
{
	if (m_bEnable == 0)
		return 0;

	if (m_bFocused != 1)
		return 0;

	if (iCharCode == VK_LEFT)
		return 0;
	if (iCharCode == VK_ESCAPE)
	{
		if (g_nKeyType == 0)
		{
			SetText((char*)"");
			m_bFocused = 0;
			g_pCurrentScene->m_pControlContainer->SetFocusedControl(nullptr);
		}
		return 1;
	}
	if (iCharCode == VK_RETURN)
	{
		if (m_pEventListener != nullptr)
			return m_pEventListener->OnControlEvent(m_dwControlID, 0);

		return 0;
	}
	if (iCharCode == VK_TAB)
	{
		if (m_pEventListener != nullptr)
			return m_pEventListener->OnControlEvent(m_dwControlID, 1);

		return 0;
	}

	SButton* Button = nullptr;
	SButton* Button2 = nullptr;

	int nTextLen = strlen(m_strText);
	if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && 
		iCharCode != VK_BACK &&
		nTextLen <= 1 && 
		(iCharCode == '=' || iCharCode == '-' || iCharCode == '@'))
	{		
		if (nTextLen == 0)
		{
			switch (iCharCode)
			{
			case '-':
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_GUILD));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "-");
				break;
			case '=':
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_PARTY));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "=");
				break;
			case '@':
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_KINGDOM));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "@");
				break;
			}

			if (Button != nullptr)
			{
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
		}
		else if (iCharCode == '-' && m_strText[0] == '-')
		{
			Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_GUILD2));
			strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "--");
			Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
			Button2->SetText(Button->m_GCPanel.strString);
		}
		else if (iCharCode == '@' && m_strText[0] == '@')
		{
			Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_CITY));
			strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "@@");
			Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
			Button2->SetText(Button->m_GCPanel.strString);
		}
	}
	if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD && iCharCode != VK_BACK)
	{
		char temp[64]{};
		sprintf(temp, "/%s", g_pMessageStringTable[389]);
		int length = strlen(temp);

		if (m_strText[0] == '/' && !strncmp(m_strText, temp, length))
		{
			Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_SHOUT));
			strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "");
			Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
			Button2->SetText(Button->m_GCPanel.strString);
		}
		else if (m_strText[0] == '/' && strncmp(m_strText, temp, length))
		{
			Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_WHISPER));
			strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "");
			Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
			Button2->SetText(Button->m_GCPanel.strString);
		}
	}
	if (iCharCode == VK_BACK)
	{
		if (strlen(m_strComposeText))
			return 1;

		int nTextLen = strlen(m_strText);
		if (nTextLen == 0)
		{
			if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_NOMAL));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}

			return m_pEventListener->OnControlEvent(m_dwControlID, 7);
		}

		LPSTR szPrevText = CharPrev(m_strText, &m_strText[nTextLen]);

		int nLen = szPrevText - m_strText;
		m_strText[nLen] = 0;
		Update();

		if (g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
		{
			if (!strcmp(m_strText, "="))
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_PARTY));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "=");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
			else if (!strcmp(m_strText, "-"))
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_GUILD));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "-");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
			else if (!strcmp(m_strText, "--"))
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_GUILD2));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "--");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
			else if (!strcmp(m_strText, "@"))
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_KINGDOM));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "@");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
			else if (!strcmp(m_strText, "@@"))
			{
				Button = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT_CITY));
				strcpy(static_cast<TMFieldScene*>(g_pCurrentScene)->m_cChatType, "@@");
				Button2 = static_cast<SButton*>(g_pCurrentScene->m_pControlContainer->FindControl(B_CHAT_SELECT));
				Button2->SetText(Button->m_GCPanel.strString);
			}
		}
		return 1;
	}
	else if (m_nMaxStringLen > strlen(m_strText))
	{
		if (m_cTempChar)
			m_cTempChar = 0;

		LPSTR lpCurrent = &m_strText[strlen(m_strText)];
		lpCurrent[0] = iCharCode;
		lpCurrent++;
		lpCurrent[0] = 0;
		lpCurrent++;
		Update();

		return 1;	
	}
	else
	{
		int nTextLen = strlen(m_strText);
		if (nTextLen == 0)
			return 1;

		if (!IsClearString(m_strText, nTextLen - 1))
		{
			m_strText[nTextLen - 1] = 0;
			m_strComposeText[0] = 0;
		}

		Update();
		m_cTempChar = iCharCode;
		return 1;
	}

	return 0;
}

int SEditableText::OnChangeIME()
{
	// No need for now
	return 0;
}

int SEditableText::OnIMEEvent(char* ipComposeString)
{
	// No need for now
	return 0;
}

int SEditableText::OnKeyDownEvent(unsigned int iKeyCode)
{
	if (m_bEnable == 0)
		return 0;
	if (m_bFocused != 1)
		return 0;

	if (iKeyCode == VK_PRIOR)
		return m_pEventListener->OnControlEvent(m_dwControlID, 2);
	if (iKeyCode == VK_UP)
		return m_pEventListener->OnControlEvent(m_dwControlID, 3);
	if (iKeyCode == VK_NEXT)
		return m_pEventListener->OnControlEvent(m_dwControlID, 4);
	if (iKeyCode == VK_DOWN)
		return m_pEventListener->OnControlEvent(m_dwControlID, 5);
	if(iKeyCode == VK_DELETE)
		return m_pEventListener->OnControlEvent(m_dwControlID, 6);

	return 0;
}

int SEditableText::OnKeyUpEvent(unsigned int iKeyCode)
{
	return m_bFocused == 1;
}

void SEditableText::Update()
{
	int nStringLen = strlen(m_strText);
	int nComposeStringLen = strlen(m_strComposeText);

	if (m_bPasswd == 0)
	{
		strcpy(m_GCText.strString, m_strText);
	}
	else
	{
		for (int nIndex = 0; nIndex < nComposeStringLen + nStringLen; ++nIndex)
			m_GCText.strString[nIndex] = '*';

		m_GCText.strString[nStringLen] = 0;
	}

	m_GCText.pFont->SetText(m_GCText.strString, 0xFFFFFFFF, 0);
}

void SEditableText::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	int nStringLen = strlen(m_strText);
	if (m_bFocused == 1)
	{
		++m_nCursorVisible;
		m_nCursorVisible %= 20;

		if (m_nCursorVisible == 10)
		{
			m_GCText.strString[nStringLen] = '|';
			m_GCText.strString[nStringLen + 1] = 0;
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}
		else if (m_nCursorVisible == 0)
		{
			m_GCText.strString[nStringLen] = 0;
			m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
		}
	}
	else if (m_GCText.strString[nStringLen] == '|')
	{
		m_GCText.strString[nStringLen] = 0;
		m_GCText.pFont->SetText(m_GCText.strString, m_GCText.dwColor, 0);
	}
	else 
		m_GCText.strString[nStringLen] = 0;

	SText::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
}

void SEditableText::SetFocused(int bFocused)
{
	m_bFocused = bFocused;
}

int SEditableText::IsIMENative()
{
	return 0;
}
