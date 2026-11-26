#include "core/pch.h"
#include "core/EventTranslator.h"
#include "ui/SGrid.h"
#include "ui/SControl.h"
#include "core/TMGlobal.h"
#include "common/Basedef.h"
#include "ui/SControlContainer.h"
#include "scene/TMFieldScene.h"

unsigned int SControl::m_dwStaticID{ 0 };
int SControl::m_nGridCellSize{ 35 };
int SCursor::m_nCursorType{ 0 };
HCURSOR SCursor::m_hCursor1{};
HCURSOR SCursor::m_hCursor2{};

SControl::SControl(float inPosX, float inPosY, float inWidth, float inHeight) 
	: TreeNode(0)
{
	m_bAlwaysOnTop = 0;
	m_bVisible = 1;
	m_bEnable = 1;
	m_bFocused = 0;
	m_bOver = 0;
	m_bDeleteThisObject = 0;
	m_bSelectEnable = 1;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_NONE;
	m_dwControlID = 0;
	m_pEventListener = nullptr;
	m_bModal = 0;

	float fWidthRatio = (float)g_pDevice->m_dwScreenWidth / 800.0f;
	float fHeightRatio = (float)g_pDevice->m_dwScreenHeight / 600.0f;
	m_nPosX = inPosX * fWidthRatio;
	m_nPosY = inPosY * fHeightRatio;
	m_nWidth = inWidth * fWidthRatio;
	m_nHeight = inHeight * fHeightRatio;
	m_dwUniqueID = SControl::m_dwStaticID++;
}

SControl::~SControl()
{
}

int SControl::OnPacketEvent(unsigned int dwCode, char* buf)
{
	return 0;
}

int SControl::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	return 0;
}

int SControl::OnKeyDownEvent(unsigned int iKeyCode)
{
	return 0;
}

int SControl::OnKeyUpEvent(unsigned int iKeyCode)
{
	return 0;
}

int SControl::OnCharEvent(char iCharCode, int lParam)
{
	return 0;
}

int SControl::OnChangeIME()
{
	return 0;
}

int SControl::OnIMEEvent(char* ipComposeString)
{
	return 0;
}

int SControl::IsIMENative()
{
	return 0;
}

void SControl::SetControlID(unsigned int idwControlID)
{
	m_dwControlID = idwControlID;
}

unsigned int SControl::GetControlID()
{
	return m_dwControlID;
}

unsigned int SControl::GetUniqueID()
{
	return m_dwUniqueID;
}

void SControl::SetEventListener(IEventListener* ipEventListener)
{
	m_pEventListener = ipEventListener;
}

void SControl::Update()
{
}

void SControl::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
}

void SControl::SetAlwaysOnTop(int bAlwaysOnTop)
{
	m_bAlwaysOnTop = bAlwaysOnTop;
}

void SControl::SetVisible(int bVisible)
{
	m_bVisible = bVisible;
}

void SControl::SetEnable(int bEnable)
{
	m_bEnable = bEnable;
}

void SControl::SetFocused(int bFocused)
{
	m_bFocused = bFocused;
}

int SControl::IsVisible()
{
	return m_bVisible;
}

int SControl::IsFocused()
{
	return m_bFocused;
}

int SControl::IsOver()
{
	return m_bOver;
}

TMVector2 SControl::GetPos()
{
	return TMVector2(m_nPosX, m_nPosY);
}

int SControl::ChildCount()
{
	return 0;
}

void SControl::SetPos(float nPosX, float nPosY)
{
	m_nPosX = nPosX * 1.0f;
	m_nPosY = nPosY * 1.0f;
}

void SControl::SetSize(float nWidth, float nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void SControl::SetRealPos(float nPosX, float nPosY)
{
	m_nPosX = nPosX;
	m_nPosY = nPosY;
}

void SControl::SetRealSize(float nWidth, float nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void SControl::SetAutoSize()
{
	m_nPosX = (float)(m_nPosX / 800.0f) * (float)g_pDevice->m_dwScreenWidth;
	m_nPosY = (float)(m_nPosY / 600.0f) * (float)g_pDevice->m_dwScreenHeight;
}

void SControl::SetCenterSize()
{
	m_nPosX = (float)((g_pDevice->m_dwScreenWidth - 800) >> 1) + m_nPosX;
	m_nPosY = (float)((g_pDevice->m_dwScreenHeight - 600) >> 1) + m_nPosY;
}

void SControl::SetStickLeft()
{
	m_nPosX = 0.0f;
}

void SControl::SetStickRight()
{
	m_nPosX = (float)g_pDevice->m_dwScreenWidth - m_nWidth;
}

void SControl::SetStickTop()
{
	m_nPosY = 0.0f;
}

void SControl::SetStickBottom()
{
	m_nPosY = (float)g_pDevice->m_dwScreenHeight - m_nHeight;
}

int SControl::PtInControl(int inPosX, int inPosY)
{
	return PointInRect(inPosX, inPosY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);
}

CONTROL_TYPE SControl::GetControlType()
{
	return CONTROL_TYPE::CTRL_TYPE_NONE;
}

void SControl::SetCenterPos(unsigned int dwControlID, float inPosX, float inPosY, float inWidth, float inHeight)
{
	if (g_pDevice == nullptr)
		return;

	static unsigned int dwCenterUI[6] = { 769, 4622, 65870, 4617, 5638, 0 };

	for (int i = 0; i < 5; ++i)
	{
		if (dwControlID == dwCenterUI[i])
		{
			m_nPosX = ((float)g_pDevice->m_dwScreenWidth * 0.5f) - inWidth * 0.5f;
		}
	}
}

SPanel::SPanel(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, RENDERCTRLTYPE eRenderType)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_GCPanel = GeomControl(eRenderType, inTextureSetIndex, 0.0f, 0.0f, inWidth, inHeight, 0, idwColor);
	m_pDescPanel = 0;
	m_bPickable = 0;
	m_bPicked = 0;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_PANEL;
}

SPanel::~SPanel()
{
	SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;

	if (pControlContainer != nullptr && pControlContainer->m_pPickedControl == this)
		pControlContainer->m_pPickedControl = nullptr;

	if (pControlContainer != nullptr && m_GCPanel.nLayer >= 0)
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCPanel, m_GCPanel.nLayer);
}

int SPanel::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	if (m_dwControlID == 65943 || m_dwControlID == 65947)
		return 0;

	int bInCaption = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, 24.0f);
	m_bOver = PointInRect(nX, nY, m_nPosX, m_nPosY, m_nWidth, m_nHeight);

	if (m_bOver == 0 && m_pDescPanel != nullptr)
		m_pDescPanel->SetVisible(0);

	switch (dwFlags)
	{
	case WM_MOUSEMOVE:
	{
		if (m_bPicked != 0 && m_bPickable != 0)
		{
			m_nPosX = (float)(nX - m_nPickPosX) + m_nPosX;
			m_nPosY = (float)(nY - m_nPickPosY) + m_nPosY;
			if (m_nPosX < 0.0)
				m_nPosX = 0.0;
			if (m_nPosY < 0.0)
				m_nPosY = 0.0;

			if ((float)(m_nPosX + m_nWidth) > (float)g_pApp->m_dwScreenWidth)
				m_nPosX = (float)g_pApp->m_dwScreenWidth - m_nWidth;
			if ((float)(m_nPosY + m_nHeight) > (float)g_pApp->m_dwScreenHeight)
				m_nPosY = (float)g_pApp->m_dwScreenHeight - m_nHeight;

			m_nPickPosX = nX;
			m_nPickPosY = nY;
		}		
		if (m_bOver == 1 && (wParam & 1))
			return 1;
		
		if (m_bOver == 1 && m_pDescPanel != nullptr)
			m_pDescPanel->SetVisible(1);		
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (m_bPickable != 0 && bInCaption != 0 && g_pCurrentScene->m_pControlContainer->m_pPickedControl == nullptr)
		{
			g_pCurrentScene->m_pControlContainer->m_pPickedControl = this;
			m_bPicked = 1;
			m_nPickPosX = nX;
			m_nPickPosY = nY;
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (g_pCurrentScene->m_pControlContainer->m_pPickedControl == this)
			g_pCurrentScene->m_pControlContainer->m_pPickedControl = nullptr;

		if (m_bPickable != 0)
			m_bPicked = 0;
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		if (m_bModal == 1 && m_bOver == 1)
			return 1;

		return SControl::OnMouseEvent(dwFlags, wParam, nX, nY);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (m_bOver == 1)
			return 1;
	}
	break;
	}

	if (m_bOver == 1)
		return 1;

	return m_bModal == 1 && m_bOver == 1;
}

void SPanel::SetTextureSetIndex(int inTextureSetIndex)
{
	m_GCPanel.nTextureSetIndex = inTextureSetIndex;
}

GeomControl* SPanel::GetGeomControl()
{
	return &m_GCPanel;
}

void SPanel::SetVisible(int bVisible)
{
	m_bVisible = bVisible;

	if (m_bVisible == 0 && g_pCurrentScene != nullptr && g_pCurrentScene->m_pControlContainer != nullptr)
	{
		if (g_pCurrentScene->m_pControlContainer->m_pPickedControl == this)
			g_pCurrentScene->m_pControlContainer = nullptr;
	}
}

void SPanel::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	if (m_GCPanel.nTextureSetIndex >= 0 || (m_GCPanel.dwColor & 0xFF000000))
	{
		m_GCPanel.nPosX = ivParentPos.x + m_nPosX;
		m_GCPanel.nPosY = ivParentPos.y + m_nPosY;
		m_GCPanel.nWidth = m_nWidth;
		m_GCPanel.nHeight = m_nHeight;
		m_GCPanel.nLayer = inParentLayer;
		if ((float)(m_GCPanel.nPosX + m_GCPanel.nWidth) >= 0.0f && 
			(float)(m_GCPanel.nPosY + m_GCPanel.nHeight) >= 0.0f &&
			m_GCPanel.nPosX <= (float)(800.0 * RenderDevice::m_fWidthRatio) && 
			m_GCPanel.nPosY <= (float)(600.0 * RenderDevice::m_fHeightRatio))
		{
			AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
		}
	}
}

S3DObj::S3DObj(int nObjIndex, float inX, float inY, float inWidth, float inHeight)
	: SControl(inX, inY, inWidth, inHeight)
{
	m_GCObj = GeomControl(RENDERCTRLTYPE::RENDER_3DOBJ, 0, 0.0f, 0.0f, inWidth, inHeight, 0, 0xFFFFFFFF);
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_3DOBJ;
	m_GCObj.n3DObjIndex = nObjIndex;
}

S3DObj::~S3DObj()
{
	SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;

	if (pControlContainer != nullptr && m_GCObj.nLayer >= 0)
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCObj, m_GCObj.nLayer);
}

void S3DObj::SetObjIndex(int nObjIndex)
{
	m_GCObj.n3DObjIndex = nObjIndex;
}

GeomControl* S3DObj::GetGeomControl()
{
	return &m_GCObj;
}

int S3DObj::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (m_bSelectEnable == 0)
		return 0;

	if (dwFlags == WM_MOUSEMOVE)
		return 0;

	return OnMouseEvent(dwFlags, wParam, nX, nY);
}

void S3DObj::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	float fAngle = 0.0f;
	if (m_bOver == 1)
	{
		fAngle = (float)(g_pTimerManager->GetServerTime() % 3000);
		fAngle = (fAngle * 6.28f) / 3000.0f;
	}

	m_GCObj.fAngle = fAngle;
	m_GCObj.nPosX = ivParentPos.x + m_nPosX;
	m_GCObj.nPosY = ivParentPos.y + m_nPosY;
	m_GCObj.nWidth = m_nWidth;
	m_GCObj.nHeight = m_nHeight;
	m_GCObj.nLayer = inParentLayer;
	AddRenderControlItem(pDrawList, &m_GCObj, inParentLayer);
}

SCursor::SCursor(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, 0x77777777, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_eStyle = ECursorStyle::TMC_CURSOR_HAND;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_CURSOR;
	m_GCPanel.nTextureIndex = 0;
	m_nPosX = inX;
	m_nPosY = inY;
	g_pCursor = this;
	m_pAttachedItem = nullptr;
}

SCursor::~SCursor()
{
	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
	{
		SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;
		if (pControlContainer != nullptr)
		{
			if(m_GeomItem.nLayer >= 0)
				RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GeomItem, m_GeomItem.nLayer);
		}
	}
}

int SCursor::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)
{
	if (dwFlags == WM_MOUSEMOVE)
		SetPosition(nX, nY);

	return SControl::OnMouseEvent(dwFlags, wParam, nX, nY);
}

void SCursor::FrameMove2(stGeomList* pDrawList, TMVector2 ivParenPos, int inParentLayer, int nFlag)
{
	m_GCPanel.nPosX = m_nPosX;
	m_GCPanel.nPosY = m_nPosY;

	if (SCursor::m_nCursorType == 2)
	{
		m_GCPanel.nPosX = -100.0f;
		m_GCPanel.nPosY = -100.0f;
	}

	m_GCPanel.nWidth = m_nWidth;
	m_GCPanel.nHeight = m_nHeight;
	m_GCPanel.nLayer = inParentLayer;
	m_bAlwaysOnTop = 1;

	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
	{
		if (m_pAttachedItem == nullptr)
		{
			m_eStyle = ECursorStyle::TMC_CURSOR_HAND;
			return;
		}

		m_GeomItem = *m_pAttachedItem->GetGeomControl();

		if (m_GeomItem.eRenderType == RENDERCTRLTYPE::RENDER_3DOBJ)
		{
			m_GeomItem.nPosX += m_pAttachedItem->GetPos().x;
			m_GeomItem.nPosY += m_pAttachedItem->GetPos().y;

			m_GeomItem.nLayer = m_GCPanel.nLayer;
			m_GeomItem.fScale = 1.5f;

			unsigned int dwServerTime = g_pTimerManager->GetServerTime() % 3000;
			m_GeomItem.fAngle = ((float)dwServerTime * 6.28f) / 3000.0f;
		}
		else
		{
			m_GeomItem.nPosX = (float)(m_nPosX + m_pAttachedItem->GetPos().x) - 12.0f;
			m_GeomItem.nPosY = (float)(m_nPosY + m_pAttachedItem->GetPos().y) - 12.0f;
			m_GeomItem.nLayer = m_GCPanel.nLayer;
		}

		AddRenderControlItem(pDrawList, &m_GeomItem, inParentLayer);
		AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
	}
	else
	{
		AddRenderControlItem(pDrawList, &m_GCPanel, inParentLayer);
	}
}

void SCursor::SetPosition(int iX, int iY)
{
	m_nPosX = (float)iX;
	m_nPosY = (float)iY;
}

void SCursor::SetVisible(int bVisible)
{
	m_bVisible = bVisible;
}

void SCursor::SetStyle(ECursorStyle eStyle)
{
	m_eStyle = eStyle;
}

ECursorStyle SCursor::GetStyle()
{
	return m_eStyle;
}

GeomControl* SCursor::GetGeomControl()
{
	return &m_GCPanel;
}

int SCursor::AttachItem(SGridControlItem* pItem)
{
	if (pItem == nullptr)
		return 0;
	if (m_eStyle != ECursorStyle::TMC_CURSOR_HAND)
		return 0;

	m_eStyle = ECursorStyle::TMC_CURSOR_PICKUP;
	m_pAttachedItem = pItem;
	return 1;
}

SGridControlItem* SCursor::DetachItem()
{
	SGridControlItem* pItem = m_pAttachedItem;
	m_pAttachedItem = nullptr;

	if (m_eStyle == ECursorStyle::TMC_CURSOR_PICKUP)
		m_eStyle = ECursorStyle::TMC_CURSOR_HAND;

	return pItem;
}

int PointInRect(int inPosX, int inPosY, float ifX, float ifY, float ifWidth, float ifHeight)
{
	return (float)inPosX >= ifX
		&& (float)inPosY >= ifY
		&& (float)(ifX + ifWidth) > (float)inPosX
		&& (float)(ifY + ifHeight) > (float)inPosY;
}

void RemoveRenderControlItem(stGeomList* pDrawList, GeomControl* pGeomControl, int nLayer)
{
	if (nLayer >= MAX_DRAW_CONTROL)
		return;

	GeomControl* pCurrent = pDrawList[nLayer].pHeadGeom;

	if (pCurrent == nullptr)
		return;

	if (pCurrent == pGeomControl)
	{
		pDrawList[nLayer].pHeadGeom = pCurrent->m_pNextGeom;
		return;
	}

	int nCount{ 0 };
	while (pCurrent != nullptr && pCurrent->m_pNextGeom != nullptr)
	{
		if (pCurrent->m_pNextGeom == pGeomControl)
		{
			pCurrent->m_pNextGeom = pCurrent->m_pNextGeom->m_pNextGeom;
			return;
		}

		pCurrent = pCurrent->m_pNextGeom;
		if (++nCount > g_pDebugMaxCount)
			g_pDebugMaxCount = nCount;

		if (nCount > MAX_DRAW_CONTROL)
			return;
	}
}

int AddRenderControlItem(stGeomList* pDrawList, GeomControl* pGeomControl, int nLayer)
{
	if (nLayer >= MAX_DRAW_CONTROL)
		return 0;

	if (pGeomControl)
		pGeomControl->nLayer = nLayer;

	if (pDrawList[nLayer].pHeadGeom)
		pDrawList[nLayer].pTailGeom->m_pNextGeom = pGeomControl;
	else
		pDrawList[nLayer].pHeadGeom = pGeomControl;

	pDrawList[nLayer].pTailGeom = pGeomControl;
	return 1;
}