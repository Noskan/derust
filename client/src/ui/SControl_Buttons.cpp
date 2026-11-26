#include "core/pch.h"
#include "core/EventTranslator.h"
#include "ui/SGrid.h"
#include "ui/SControl.h"
#include "core/TMGlobal.h"
#include "common/Basedef.h"
#include "ui/SControlContainer.h"
#include "scene/TMFieldScene.h"

/// @file SControl_Buttons.cpp
/// @brief Controles de interação: `SButton`, `SButtonBox`, `SCheckBox`, `SProgressBar` e `SScrollBar`.
///
/// Extraído de `SControl.cpp` na Fase 2 (Onda 3) sem alteração de comportamento.

SButton::SButton(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, int bSound, char* istrText)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, idwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_bMouseOver = 0;
	m_bSelected = 0;
	m_cAlwaysAlt = 0;
	m_cBlink = 0;
	m_pAltText = 0;
	m_dwColor = idwColor;
	m_bSound = bSound;
	m_dwOldTime = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_BUTTON;

	if (istrText != nullptr)
	{
		BASE_UnderBarToSpace(istrText);
		if (!strcmp(istrText, " "))
			istrText = nullptr;
	}

	if (istrText != nullptr)
	{
		if (inTextureSetIndex >= -2 && inTextureSetIndex < 0)
		{
			m_GCPanel.pFont = nullptr;
			m_GCPanel.pFont = new TMFont2();

			sprintf(m_GCPanel.strString, "%s", istrText);

			if (m_GCPanel.pFont != nullptr)
				m_GCPanel.pFont->SetText(m_GCPanel.strString, 0xFFFFFFFF, 0);
		}

		m_pAltText = new SText(-1, istrText, 0xFFFFFFFF, inWidth - 10.0f, 0.0f, (float)strlen(istrText) * 8.0f, (float)RenderDevice::m_nFontSize,
			0, 0xFF333333, 1u, 0);
		m_pAltText->SetVisible(0);
		m_pAltText->SetAlwaysOnTop(1);

		if (inTextureSetIndex == -1 || inTextureSetIndex == -2)
		{
			m_nPosX = m_nPosX
				- (float)((float)(RenderDevice::m_nFontSize - 12) * (float)((float)strlen(istrText) * 0.3f));
			m_nPosY = m_nPosY - (float)((float)(RenderDevice::m_nFontSize - 12) * 0.6f);
		}
	}

	m_GrayType = 0;
	Update();
}

SButton::~SButton()
{
	SAFE_DELETE(m_GCPanel.pFont);
	SAFE_DELETE(m_pAltText);
}

void SButton::SetText(char* istrText)
{
	if (m_pAltText != nullptr)
		m_pAltText->SetText(istrText, 0);

	sprintf(m_GCPanel.strString, "%s", istrText);
	m_GCPanel.pFont->SetText(m_GCPanel.strString, 0xFFFFFFFF, 0);
}

int SButton::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
	if (m_pAltText != nullptr)
	{
		if (m_bOver == 1)
		{
			if (!m_pAltText->IsVisible())
				m_pAltText->SetVisible(1);
			if (m_GCPanel.nTextureSetIndex > -3 && m_GCPanel.nTextureSetIndex < 3)
			{
				m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_TEXT_FOCUS;
				if (m_GCPanel.pFont != nullptr)
					m_GCPanel.pFont->m_dwShadeColor = 0x66FFFFFF;
			}
		}
		else
		{
			m_pAltText->SetVisible(0);
			m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
			if (m_GCPanel.pFont != nullptr)
				m_GCPanel.pFont->m_dwShadeColor = 0xFF000000;
		}
	}

	switch (dwFlags)
	{
	case WM_MOUSEMOVE:
		if (m_bMouseOver == 1 && m_pEventListener != nullptr)
			m_pEventListener->OnControlEvent(m_dwControlID, 2);
		break;
	case WM_LBUTTONDOWN:
		if (m_bVisible == 0)
			return 1;

		if (m_bOver == 1)
		{
			m_bPressed = 1;
			m_bFocused = 1;
			Update();
			return 1;
		}
		break;
	case WM_LBUTTONUP:
		if (m_bVisible == 0)
			return 1;

		if (m_bEnable == 1 && m_bOver == 1 && m_bPressed == 1 && m_pEventListener != nullptr)
		{
			unsigned int dwServerTime = g_pTimerManager->GetServerTime();
			if (dwServerTime > m_dwOldTime + 500)
			{
				if (m_bSound == 1 && g_pSoundManager != nullptr)
				{
					if (g_pSoundManager->GetSoundData(53))
						g_pSoundManager->GetSoundData(53)->Play(0, 0);
				}

				Update();
				m_pEventListener->OnControlEvent(m_dwControlID, 0);
				m_dwOldTime = g_pTimerManager->GetServerTime();
				m_bPressed = 0;
			}
			return 1;			
		}
		m_bPressed = 0;
		break;
	default:
		return 0;
	}

	if (m_cAlwaysAlt != 0 && m_pAltText->IsVisible())
		m_pAltText->SetVisible(1);

	Update();
	return 0;
}

void SButton::SetSelected(int bSelected)
{
	m_bSelected = bSelected;
}

void SButton::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	if (m_dwControlID == 4628)
		m_GCPanel.dwColor = 0xAA000077;

	if (m_cBlink != 0)
	{
		unsigned int dwServerTime = g_pTimerManager->GetServerTime();
		if ((dwServerTime / 750) % 2)
			m_GCPanel.nTextureIndex = 1;
		else
			m_GCPanel.nTextureIndex = 0;
	}

	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
	if (m_pAltText != nullptr && 
		m_pAltText->IsVisible()	&& 
		(m_GCPanel.nTextureSetIndex > 0 || m_GCPanel.nTextureSetIndex < -2))
	{
		ivParentPos = ivParentPos + TMVector2(m_nPosX, m_nPosY);

		if (((ivParentPos.x + m_pAltText->m_nPosX) + m_pAltText->m_nWidth) > (float)g_pDevice->m_dwScreenWidth)
			ivParentPos.x = ((float)g_pDevice->m_dwScreenWidth - (m_pAltText->m_nPosX * 2.0f)) - m_pAltText->m_nWidth;
		
		m_pAltText->FrameMove2(pDrawList, ivParentPos, 28, nFlag);
	}
}

void SButton::Update()
{
	if (m_bEnable == 0)
	{
		m_GCPanel.nTextureIndex = 4;
		if (m_GrayType != 0)
			m_GCPanel.dwColor = 0xFF7F7F7F;
	}
	else if (m_bSelected == 1)
	{
		m_GCPanel.nTextureIndex = 3;
	}
	else if (m_bOver == 1)
	{
		if (m_bPressed == 1)
			m_GCPanel.nTextureIndex = 2;
		else
			m_GCPanel.nTextureIndex = 1;
	}
	else
	{
		m_GCPanel.nTextureIndex = 0;
		if (m_GCPanel.pFont != nullptr)
			m_GCPanel.dwColor = 0x1010101;
		else
			m_GCPanel.dwColor = m_dwColor;
	}
}

SButtonBox::SButtonBox(float inPosX, float inPosY, float inWidth, float inHeight, int nStartCount, int nEndCount, int nCurrnetPage, int nPrevPage, int nNextPage, int nStartPage, int nEndPage)
	: SControl(inPosX, inPosY, inWidth, inHeight)
{

}

SButtonBox::~SButtonBox()
{
}

void SButtonBox::SetEventListener(IEventListener* ipEventListener)
{
}

void SButtonBox::SetButtonBox(int nStartCount, int nEndCount, int nCurrnetPage, int nPrevPage, int nNextPage, int nStartPage, int nEndPage)
{
}

SCheckBox::SCheckBox(unsigned int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int dwColor)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, dwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_bValue = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_CHECKBOX;
	m_dwSelectedColor = -16777216;
	m_dwUnSelectedColor = dwColor;
}

SCheckBox::~SCheckBox()
{
}

void SCheckBox::SetValue(int ibValue)
{
	m_bValue = ibValue;
	Update();
}

int SCheckBox::GetValue()
{
	return m_bValue;
}

int SCheckBox::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (!m_bSelectEnable)
		return 0;

	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (dwFlags == 512)
		return 0;

	if (dwFlags != 513)
		return SPanel::OnMouseEvent(dwFlags, wParam, nX, nY);

	m_bValue = m_bValue == 0;
	m_bFocused = 1;
	Update();

	if (m_pEventListener)
		m_pEventListener->OnControlEvent(m_dwControlID, 0);

	return 1;
}

void SCheckBox::Update()
{
	if (m_bValue == 1)
	{
		m_GCPanel.nTextureIndex = 1;
		m_GCPanel.dwColor = m_dwSelectedColor;
	}
	else
	{
		m_GCPanel.nTextureIndex = 0;
		m_GCPanel.dwColor = m_dwUnSelectedColor;
	}
}

SProgressBar::SProgressBar(int inTextureSetIndex, int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight, unsigned int idwProgressColor, unsigned int idwColor, unsigned int dwStyle)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, idwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_nCurrent = inCurrent;
	m_nMax = inMax;
	m_GCProgress = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, inTextureSetIndex, inX, inY, 0.0f, inHeight, 0, idwProgressColor);
	m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
	m_GCPanel.nTextureIndex = 1;
	m_dwStyle = dwStyle;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR;

	if (RenderDevice::m_nBright > 58)
	{
		int nR = WYDCOLOR_RED(idwProgressColor) - (RenderDevice::m_nBright - 40);
		int nG = WYDCOLOR_GREEN(idwProgressColor) - (RenderDevice::m_nBright - 40);
		int nB = WYDCOLOR_BLUE(idwProgressColor) - (RenderDevice::m_nBright - 40);

		if (nR < 0)
			nR = 0;
		if (nG < 0)
			nG = 0;
		if (nB < 0)
			nB = 0;

		m_GCProgress.dwColor = nB | (nG << 8) | idwProgressColor & 0xFF000000 | (nR << 16);
	}

	int nTextureSetIndex = m_GCProgress.nTextureSetIndex;
	if (nTextureSetIndex < -2)
		nTextureSetIndex = -m_GCProgress.nTextureSetIndex;

	auto pUISet = g_pTextureManager->GetUITextureSet(nTextureSetIndex);
	if (pUISet != nullptr)
	{
		m_InitHeight = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nHeight;
		m_InitStartY = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nStartY;
		m_InitWidth = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nWidth;
		m_InitStartX = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nStartX;
	}

	Update();
}

SProgressBar::~SProgressBar()
{
	ResetBar();

	SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;

	if (pControlContainer != nullptr && m_GCProgress.nLayer >= 0)
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCProgress, m_GCProgress.nLayer);
}

void SProgressBar::SetCurrentProgress(int inCurrent)
{
	if (m_nCurrent != inCurrent)
	{
		m_nCurrent = inCurrent;
		if (inCurrent < 0)
			m_nCurrent = 0;

		Update();
	}
}

void SProgressBar::SetMaxProgress(int inMax)
{
	if (m_nMax != inMax)
	{
		m_nMax = inMax;
		Update();
	}
}

int SProgressBar::GetCurrentProgress()
{
	return m_nCurrent;
}

int SProgressBar::GetMaxProgress()
{
	return m_nMax;
}

void SProgressBar::Update()
{
	if (m_nCurrent > m_nMax)
		m_nCurrent = m_nMax;
	if (m_nMax <= 0)
		m_nMax = 1;

	if (m_dwStyle == 1)
		m_nProgressWidth = (float)((float)m_nCurrent * m_nWidth) / (float)m_nMax;	
	else if (m_dwStyle == 2)
		m_nProgressWidth = (float)((float)m_nCurrent * m_nWidth) / (float)m_nMax;
	else
		m_nProgressHeight = (float)((float)m_nCurrent * m_nHeight) / (float)m_nMax;
}

void SProgressBar::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	float fWidthRatio = (float)g_pDevice->m_dwScreenWidth / 800.0f;
	float fHeightRatio = (float)g_pDevice->m_dwScreenHeight / 600.0f;

	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);

	if (m_dwStyle == 1)
	{
		m_GCProgress.nPosX = (float)(ivParentPos.x + m_nPosX) + 2.0f;
		m_GCProgress.nPosY = (float)(ivParentPos.y + m_nPosY) + 2.0f;
		m_GCProgress.nWidth = m_nProgressWidth;
		m_GCProgress.nHeight = m_nHeight - 4.0f;
		m_GCProgress.nLayer = inParentLayer;
	}
	else if (m_dwStyle == 2)
	{
		int nTextureSetIndex = m_GCProgress.nTextureSetIndex;
		if (nTextureSetIndex < -2)
			nTextureSetIndex = -m_GCProgress.nTextureSetIndex;

		auto pUISet = g_pTextureManager->GetUITextureSet(nTextureSetIndex);
		if (pUISet != nullptr)
			pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nWidth = (int)(m_nProgressWidth / fWidthRatio);

		m_GCProgress.nPosX = (float)(ivParentPos.x + m_nPosX) + 2.0f;
		m_GCProgress.nPosY = (float)(ivParentPos.y + m_nPosY) + 2.0f;
		m_GCProgress.nWidth = m_nProgressWidth;
		m_GCProgress.nHeight = m_nHeight - 4.0f;
		m_GCProgress.nLayer = inParentLayer;
	}
	else
	{
		int nIndex = m_GCProgress.nTextureSetIndex;
		if (nIndex < -2)
			nIndex = -m_GCProgress.nTextureSetIndex;

		auto pUISet = g_pTextureManager->GetUITextureSet(nIndex);
		if (pUISet != nullptr)
		{
			pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nHeight = (int)(m_nProgressHeight / fHeightRatio);
			pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nStartY = (int)(((m_nHeight - m_nProgressHeight) / fHeightRatio) + m_InitStartY);
		}

		m_GCProgress.nPosX = ivParentPos.x + m_nPosX;
		m_GCProgress.nPosY = ((ivParentPos.y + m_nPosY) + m_nHeight) - m_nProgressHeight;
		m_GCProgress.nWidth = m_nWidth;
		m_GCProgress.nHeight = m_nProgressHeight;
		m_GCProgress.nLayer = inParentLayer;
	}

	AddRenderControlItem(pDrawList, &m_GCProgress, inParentLayer);
}

void SProgressBar::ResetBar()
{
	int nTextureSetIndex = 0;
	if (m_GCProgress.nTextureSetIndex < -2)
		nTextureSetIndex = -m_GCProgress.nTextureSetIndex;
}

SScrollBar::SScrollBar(int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight, unsigned int dwStyle, unsigned int idwBarColor, unsigned int idwColor, int bChat)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_dwStyle = dwStyle;
	m_nCurrent = inCurrent;
	m_nMax = inMax;
	m_nBtnSize = 17.0f;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_SCROLLBAR;

	if (m_dwStyle == 0)
	{
		m_nBarSize = inWidth;
		float nStartXPos = 0;

		if (bChat)
			nStartXPos = -5;

		m_pUpPanel = new SPanel(423, (float)nStartXPos, 0.0f, 10.0f, m_nBtnSize, 0xAAAA00FF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pDownPanel = new SPanel(424, (float)nStartXPos, ((m_nHeight - m_nBtnSize) + 7.0f) / 1.0f, 10.0f, m_nBtnSize, 0xAAAA00FF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_nScrollLength = (int)(float)(m_nHeight - (m_nBtnSize * 2.0));
		m_pBar = new SPanel(425, (float)nStartXPos + 1.0f, 0.0f, 10.0f - 2.0f, 17.0f, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pBackground1 = new SPanel(426, (float)nStartXPos, 0.0f, 10.0f, inHeight, 0xFFFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pBackground1->GetGeomControl()->eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
	}
	else
	{
		m_pUpPanel = new SPanel(-2, 0.0, 0.0, m_nBtnSize, m_nHeight, 0x44444444, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pDownPanel = new SPanel(-2, m_nWidth - m_nBtnSize, 0.0f, m_nBtnSize, m_nHeight, 0x44444444, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_nScrollLength = (int)(m_nWidth - (m_nBtnSize * 2.0f));
		m_pBar = new SPanel(-2, 0.0f, 0.0f, m_nBarSize, m_nBarSize, 0, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
		m_pBackground1 = new SPanel(-2, 0.0f, 0.0f, m_nWidth, inHeight, 0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

		if(m_pBackground1 != nullptr)
			m_pBackground1->GetGeomControl()->nTextureIndex = 0;
	}

	if (m_pUpPanel != nullptr)
	{
		m_pUpPanel->SetControlID(0);
		AddChild(m_pUpPanel);
	}
	if (m_pDownPanel != nullptr)
	{
		m_pDownPanel->SetControlID(1);
		AddChild(m_pDownPanel);
	}
	if (m_pBar != nullptr)
	{
		m_pBar->SetControlID(2);
		AddChild(m_pBar);
	}
	if (m_pBackground1 != nullptr)
	{
		m_pBackground1->SetControlID(3);
		AddChild(m_pBackground1);
	}

	Update();
}

SScrollBar::~SScrollBar()
{
}

void SScrollBar::SetCurrentPos(int inCurrent)
{
	m_nCurrent = inCurrent;
	Update();
}

int SScrollBar::GetCurrentPos()
{
	return m_nCurrent;
}

void SScrollBar::SetMaxValue(int inMax)
{
	m_nMax = inMax;
	Update();
}

int SScrollBar::GetMaxValue()
{
	return m_nMax;
}

void SScrollBar::SetSize(float nWidth, float nHeight)
{
	SControl::SetSize(nWidth, nHeight);
	if (m_dwStyle == 0)
	{
		m_pDownPanel->SetPos(m_pDownPanel->m_nPosX, (float)(m_nHeight - m_nBtnSize) / 1.0f);
		m_nScrollLength = (int)(nHeight - (m_nBtnSize * 2.0f));
		Update();
	}
}

void SScrollBar::Up()
{
	if (m_nCurrent > 5)
	{
		m_nCurrent -= 5;
	}
	else if (m_nCurrent > 0)
	{
		m_nCurrent = 0;
	}

	Update();
}

void SScrollBar::Down()
{
	if (m_nCurrent < m_nMax - 5)
	{
		m_nCurrent += 5;
	}
	else if (m_nCurrent < m_nMax)
	{
		m_nCurrent = m_nMax;
	}

	Update();
}

void SScrollBar::Update()
{
	if (m_nCurrent < 0)
		m_nCurrent = 0;
	if (m_nCurrent > m_nMax)
		m_nCurrent = m_nMax;

	if (m_nMax > 0)
	{
		m_nScrollPos = (int)((((float)m_nScrollLength - m_nBarSize) * (float)m_nCurrent) / (float)m_nMax);
		if (m_dwStyle == 0)
			m_pBar->SetPos(m_pBar->m_nPosX, (float)m_nScrollPos + m_nBtnSize);
		else
			m_pBar->SetPos((float)m_nScrollPos + m_nBtnSize, 0.0f);

		if (m_pEventListener != nullptr)
			m_pEventListener->OnControlEvent(m_dwControlID, m_nCurrent);
	}
}

int SScrollBar::OnControlEvent(DWORD idwControlID, DWORD idwEvent)
{
	return 1;
}

void SScrollBar::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	;
}

int SScrollBar::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{ 
	if (m_bSelectEnable == 0)
		return 0;

	int bInUp = m_pUpPanel->PtInControl((int)((float)nX - m_nPosX), (int)((float)nY - m_nPosY));
	int bInDown = m_pDownPanel->PtInControl((int)((float)nX - m_nPosX), (int)((float)nY - m_nPosY));
	int bInBar = PointInRect(nX, nY, (float)(m_nPosX + m_nWidth) - 15.0f, m_nPosY + m_nBtnSize, 15.0f, m_nHeight - (float)(2.0f * m_nBtnSize));

	if (bInUp && dwFlags == WM_LBUTTONUP)
		Up();
	if (bInDown && dwFlags == WM_LBUTTONUP)
		Down();

	if (bInBar && dwFlags == WM_MOUSEMOVE && wParam & 1)
	{
		if (m_dwStyle != 0)
			m_nScrollPos = static_cast<int>((float)nX - (float)(m_nPosX + m_nBtnSize));
		else
			m_nScrollPos = static_cast<int>((float)nY - (float)(m_nPosY + m_nBtnSize));

		m_nCurrent = (int)((float)(m_nScrollPos * m_nMax) / (float)((float)m_nScrollLength - m_nBarSize));
		if (m_nCurrent > 0 && m_nCurrent < m_nMax)
			Update();
	}

	return 0;
}

void SScrollBar::upbarSetPos(float x, float y)
{
	m_pUpPanel->SetPos(x, y);
}

void SScrollBar::downbarSetPos(float x, float y)
{
	m_pDownPanel->SetPos(x, y);
}

void SScrollBar::upbarSetsize(float x, float y)
{
	m_pUpPanel->SetRealSize(x, y);
}

void SScrollBar::downbarSetsize(float x, float y)
{
	m_pDownPanel->SetRealSize(x, y);
}

void SScrollBar::upbarSetvisible(bool bSet)
{
	;
}

void SScrollBar::downbarSetvisible(bool bSet)
{
	;
}

void SScrollBar::scrollbarSetvisible(bool bSet)
{
	m_pBar->SetVisible(bSet);
}

void SScrollBar::scrollbarbackSetvisible(bool bSet)
{
	m_pBackground1->SetVisible(bSet);
}
