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
#include "MPITabWnd.h"

#include "ModelApp.h"
#include "ModelView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMPITabWnd::CMPITabWnd()
{
	m_cxSplitter = 2;
	m_cySplitter = 2;
	m_cxBorderShare = 0;
	m_cyBorderShare = 0;
	m_cxSplitterGap = 2;
	m_cySplitterGap = 2;

	m_nCurPage = -1;
}

CMPITabWnd::~CMPITabWnd()
{
}


BEGIN_MESSAGE_MAP(CMPITabWnd, CSplitterWnd)
	ON_NOTIFY(TCN_SELCHANGE, AFX_IDW_PANE_FIRST, OnSelChange)
END_MESSAGE_MAP()


int CMPITabWnd::CreateTabs(int nResourceID)
{
	// create tab control
	if (!m_wndTabCtrl.Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST))
		return 0;

	// load tabs from resource and return number of tabs
	return m_wndTabCtrl.LoadTabs(nResourceID);
}

int CMPITabWnd::HitTest(CPoint /*pt*/) const
{
	return 0;	// disable mouse dragging
}

void CMPITabWnd::OnSelChange(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	// handle notification from the tab control
	SetPage(m_wndTabCtrl.GetCurSel());
	*pResult = 0;
}

void CMPITabWnd::SetPage(int nPage)
{
	if (m_nCurPage == nPage)
		return;

	CWnd* pOld = GetDlgItem(IdFromRowCol(1, 0));
	CWnd* pNew = GetDlgItem(nPage+1);

	if (pOld)
	{	// restore ID and hide old page
		pOld->SetDlgCtrlID(m_nCurPage+1);
		pOld->ShowWindow(SW_HIDE);
	}
	else
	{	// initial update - hide all pages
		for (int i=0; i<256; i++)
		{
			pOld = GetDlgItem(i+1);
			if (!pOld)
				break;
			pOld->ShowWindow(SW_HIDE);
		}
	}
	if (pNew)
	{	// set ID of bottom splitter's pane
		pNew->SetDlgCtrlID(IdFromRowCol(1, 0));
		// show window
		pNew->ShowWindow(SW_SHOW);
		// additional handling for view windows
		if (pNew->IsKindOf(RUNTIME_CLASS(CView)))
			GetParentFrame()->SetActiveView((CView*)pNew);
	}

	m_nCurPage = nPage;

	RecalcLayout();

	if( VIEW )
		VIEW->ChangeStage( (unsigned char)m_nCurPage );
}

void CMPITabWnd::RecalcLayout()
{
	CRect rcClient;
	GetClientRect(&rcClient);

	// calculate the height of the tab control
	CRect rcTab(0, 0, 100, 1);
	m_wndTabCtrl.AdjustRect(TRUE, &rcTab);

	// reposition the tab window
	CRect rcWnd = rcClient;
	rcWnd.bottom = rcTab.Height() + 4;
	rcWnd.DeflateRect(2, 2);
	m_wndTabCtrl.MoveWindow(&rcWnd);

	rcWnd = rcClient;
	rcWnd.top = rcTab.Height();

	CWnd* pWnd = GetPane(1, 0);

	// adjust for frame display
	if (!(pWnd->GetExStyle() & WS_EX_CLIENTEDGE) && !pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		rcWnd.DeflateRect(2, 2);

	// reposition current page
	pWnd->MoveWindow(&rcWnd);

	DrawAllSplitBars(NULL, 0, 0);
}

void CMPITabWnd::DrawAllSplitBars(CDC* pDC, int /*cxInside*/, int /*cyInside*/)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	OnDrawSplitter(pDC, splitBorder, &rcClient);
}
