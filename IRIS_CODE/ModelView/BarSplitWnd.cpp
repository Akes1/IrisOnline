/*********************************************************************
* Splitter Window Extension, version 1.3 (March 6, 2003)
* Copyright (C) 2002-2003 Michal Mecinski.
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
#include "BarSplitWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CBarSplitWnd, CSplitterWnd);

CBarSplitWnd::CBarSplitWnd()
{
	m_cxSplitter = 2;
	m_cySplitter = 2;
	m_cxBorderShare = 0;
	m_cyBorderShare = 0;
	m_cxSplitterGap = 2;
	m_cySplitterGap = 2;

	m_nBarPane = 0;
	m_nBarSize = 0;
}

CBarSplitWnd::~CBarSplitWnd()
{
}


BEGIN_MESSAGE_MAP(CBarSplitWnd, CSplitterWnd)
	ON_WM_SIZE()
END_MESSAGE_MAP()


int CBarSplitWnd::HitTest(CPoint /*pt*/) const
{
	return 0;	// disable mouse dragging
}

void CBarSplitWnd::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
{
	if (nType == splitBorder)
	{	// draw splitter only on one side
		UINT nFlags = BF_RECT;
		if (m_nBarPane > 0 && (rect.top > 0 || rect.left > 0))
			nFlags &= ~(m_nCols > 1 ? BF_LEFT : BF_TOP);
		pDC->DrawEdge((LPRECT)&rect, EDGE_SUNKEN, nFlags);
	}
	else
		CSplitterWnd::OnDrawSplitter(pDC, nType, rect);
}


void CBarSplitWnd::SetBarInfo(int nPane, int nSize)
{
	m_nBarPane = nPane;
	m_nBarSize = nSize;

	if (m_nBarPane == 0)
	{
		if (m_nCols > 1)
			SetColumnInfo(0, nSize, nSize);
		else if (m_nRows > 1)
			SetRowInfo(0, nSize, nSize);
	}
}

void CBarSplitWnd::OnSize(UINT nType, int cx, int cy) 
{
	CSplitterWnd::OnSize(nType, cx, cy);
	
	if (!m_pColInfo || m_nBarPane==0)
		return;

	// resize if the bar is in the bottom or right side
	if (m_nCols==2 && m_nRows==1)	// vertical
	{
		SetColumnInfo(0, max(cx - m_nBarSize - 6, 0), 0);
		RecalcLayout();
	}
	else if (m_nCols==1 && m_nRows==2)	// horizontal
	{
		SetRowInfo(0, max(cy - m_nBarSize - 6, 0), 0);
		RecalcLayout();
	}
}
