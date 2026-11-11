/********************************************************************
* Multi-Page Interface, version 1.3 (August 20, 2004)
* Copyright (C) 2003-2004 Michal Mecinski.
*
* You may freely use and modify this code, but don't remove
* this copyright note.
*
* THERE IS NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, FOR
* THIS CODE. THE AUTHOR DOES NOT TAKE THE RESPONSIBILITY
* FOR ANY DAMAGE RESULTING FROM THE USE OF IT.
*
* E-mail: mimec@mimec.org
* WWW: http://www.mimec.org
********************************************************************/

#include "stdafx.h"
#include "DualSplitWnd.h"
#include "BarSplitWnd.h"
#include "MPIChildFrame.h"
#include "MainFrame.h"
#include "MPITabWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CMPIChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CMPIChildFrame, CMDIChildWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


CMPIChildFrame::CMPIChildFrame()
{
}

CMPIChildFrame::~CMPIChildFrame()
{
	POSITION pos = m_listSplitters.GetHeadPosition();
	while (pos)
		delete m_listSplitters.GetNext(pos);
}


BOOL CMPIChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	// disable closing or resizing the child window
	cs.style &= ~WS_SYSMENU;

	return TRUE;
}


#ifdef _DEBUG
void CMPIChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CMPIChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}
#endif //_DEBUG


void CMPIChildFrame::ActivateFrame(int /*nCmdShow*/) 
{
	CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED);
}

void CMPIChildFrame::OnClose() 
{
	// do nothing
}

void CMPIChildFrame::OnUpdateFrameTitle(BOOL /*bAddToTitle*/)
{
	GetMDIFrame()->OnUpdateFrameTitle(FALSE);

	// display child frame title instead of the document's title
	SetWindowText(m_strTitle);
}


BOOL CMPIChildFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext) 
{
	int nPos = 0;
	return CreateClientHelper(nPos, NULL, 0, (CMPICreateContext*)pContext);
}

void CMPIChildFrame::OnUpdateFrameMenu(BOOL /*bActivate*/, CWnd* /*pActivateWnd*/, HMENU hMenuAlt)
{
//	CMainFrame* pFrame = (CMainFrame*)GetMDIFrame();

	if (hMenuAlt == NULL)
		hMenuAlt = m_hMenuShared;

	// change the menu in the main frame's rebar control
//	if (hMenuAlt != NULL && bActivate)
//		pFrame->UpdateMenu(hMenuAlt);
//	else if (hMenuAlt != NULL && !bActivate && pActivateWnd == NULL)
//		pFrame->UpdateMenu(pFrame->m_hMenuDefault);
}


BOOL CMPIChildFrame::CreateClientHelper(int &nPos, CWnd *pParent, int nID, CMPICreateContext* pContext)
{
	int* pData = pContext->m_pSplitData;

	int nUID = pData[nPos++];
	int nType = pData[nPos++];

	CWnd* pNewWnd = NULL;

	if (nType == MPIT_VIEW)
	{
		// get view's runtime class information
		CRuntimeClass* pViewClass = (CRuntimeClass*)(INT_PTR)pData[nPos++];
		ASSERT_POINTER(pViewClass, CRuntimeClass);
		pContext->m_dwViewParam = (DWORD)pData[nPos++];
		if (!pParent)
		{	// the only view in this window, perform default operation
			pContext->m_pNewViewClass = pViewClass;
			if (!CMDIChildWnd::OnCreateClient(NULL, pContext))
				return FALSE;
			pNewWnd = GetDlgItem(AFX_IDW_PANE_FIRST);
		}
		else
		{
			// create view object and window
			CView* pView = (CView*)pViewClass->CreateObject();
			if (!pView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW & ~WS_BORDER, CRect(0,0,0,0), pParent, nID, pContext))
				return FALSE;
			pNewWnd = pView;
		}
	}
	else if (nType == MPIT_TABS)
	{
		// create the tabbed splitter window...
		CMPITabWnd* pTabWnd = new CMPITabWnd;
		if (!pParent)
		{	// ...as the childframe's child
			if (!pTabWnd->CreateStatic(this, 2, 1))
				return FALSE;
		}
		else
		{	// ...as another splitter's pane
			if (!pTabWnd->CreateStatic(pParent, 2, 1, WS_CHILD | WS_VISIBLE, nID))
				return FALSE;
		}
		// create the tab control and load tabs from toolbar resource
		int nResID = pData[nPos++];
		int nTabs = pTabWnd->CreateTabs(nResID);
		if (!nTabs)
			return FALSE;
		for (int i=0; i<nTabs; i++)
		{	// create tabs content recursively
			if (!CreateClientHelper(nPos, pTabWnd, i+1, pContext))
				return FALSE;
		}
		// initialize - display the first page only
		pTabWnd->SetPage(0);
		// remember the created window
		m_listSplitters.AddTail(pTabWnd);
		pNewWnd = pTabWnd;
	}
	else if (nType == MPIT_HSPLIT || nType == MPIT_VSPLIT)
	{
		CSplitterWnd* pSplitter;
		int nParam = 0;
		int nParam2 = 0;
		nParam = pData[nPos++];
		if (nParam < 0)
		{	// create a bar splitter and get initial bar size
			pSplitter = new CBarSplitWnd;
			nParam2 = pData[nPos++];
		}
		else	// create a dual splitter
			pSplitter = new CDualSplitWnd;
		if (!pParent)
		{	// create the splitter as the childframe's child
			if (!pSplitter->CreateStatic(this, nType==MPIT_VSPLIT ? 1 : 2, nType==MPIT_VSPLIT ? 2 : 1))
				return FALSE;
		}
		else
		{	// create the splitter as another splitter's pane
			if (!pSplitter->CreateStatic(pParent, nType==MPIT_VSPLIT ? 1 : 2, nType==MPIT_VSPLIT ? 2 : 1,
				WS_CHILD | WS_VISIBLE | WS_BORDER, nID))
				return FALSE;
		}
		// create both panes recursively
		if (!CreateClientHelper(nPos, pSplitter, pSplitter->IdFromRowCol(0, 0), pContext))
			return FALSE;
		if (!CreateClientHelper(nPos, pSplitter, pSplitter->IdFromRowCol(nType==MPIT_VSPLIT ? 0 : 1, nType==MPIT_VSPLIT ? 1 : 0), pContext))
			return FALSE;
		if (nParam < 0)
		{	// bar splitter
			int nRow = nType==MPIT_VSPLIT ? 0 : (nParam==MPIS_BAR_TOP ? 0 : 1);
			int nCol = nType==MPIT_VSPLIT ? (nParam==MPIS_BAR_LEFT ? 0 : 1) : 0;
			if (nParam2 == MPIBS_AUTO)
			{	// get the size of the bar (must be a scrollview or formview)
				CScrollView* pView = (CScrollView*)pSplitter->GetPane(nRow, nCol);
				ASSERT_KINDOF(CScrollView, pView);
				CSize szTotal = pView->GetTotalSize();
				nParam2 = nType==MPIT_VSPLIT ? szTotal.cx : szTotal.cy;
			}	// set the initial size of the bar
			((CBarSplitWnd*)pSplitter)->SetBarInfo(nParam==MPIS_BAR_TOP ? 0 : 1, nParam2);
		}
		else	// dual splitter - set panes size ratio
			((CDualSplitWnd*)pSplitter)->SetInitialRatio(nParam * 0.01);

		// remember the created window
		m_listSplitters.AddTail(pSplitter);
		pNewWnd = pSplitter;
	}
	else if (nType == MPIT_END)
	{
		ASSERT(FALSE); // layout description too short
		return FALSE;
	}
	else
	{
		ASSERT(FALSE); // unknown layout type
		return FALSE;
	}
	ASSERT(pParent || pData[nPos]==MPIT_END);// layout description too long

	if (nUID && pNewWnd)
		m_mapChildWnd[nUID] = pNewWnd;

	return TRUE;
}

CWnd* CMPIChildFrame::GetWndFromUID(int nUID)
{
	return (CWnd*)m_mapChildWnd[nUID];
}

