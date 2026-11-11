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
#include "DualSplitWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CDualSplitWnd, CSplitterWnd);

CDualSplitWnd::CDualSplitWnd()
{
	m_nPrev = -1;
	m_bChange = FALSE;
	m_dRatio = 0.5;
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bDragFull, 0);

	m_cxSplitter = m_cySplitter = 1;
	m_cxBorderShare = m_cyBorderShare = 1;
	m_cxSplitterGap = m_cySplitterGap = 1;
	m_cxBorder = m_cyBorder = 1;
}

CDualSplitWnd::~CDualSplitWnd()
{
}


BEGIN_MESSAGE_MAP(CDualSplitWnd, CSplitterWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CDualSplitWnd::OnInvertTracker(const CRect& rect)
{
	if (!m_bDragFull)
		CSplitterWnd::OnInvertTracker(rect);
}

void CDualSplitWnd::SetSplitCursor(int ht)
{
	if (m_bDragFull)	// use system cursors
	{
		if (ht >= vSplitterBar1 && ht <= vSplitterBar15)
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
		else if (ht >= hSplitterBar1 && ht <= hSplitterBar15)
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		else if (ht >= splitterIntersection1 && ht <= splitterIntersection225)
			SetCursor(LoadCursor(NULL, IDC_SIZEALL));
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	else
		CSplitterWnd::SetSplitCursor(ht);
}

void CDualSplitWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// only static splitter supported
	ASSERT(!(GetStyle() & SPLS_DYNAMIC_SPLIT));

	if (!m_bDragFull)	// normal dragging
	{
		CSplitterWnd::OnMouseMove(nFlags, point);
		if (m_bTracking)
			m_bChange = TRUE;
		return;
	}

	if (m_bTracking && GetCapture() != this)
		StopTracking(FALSE);

	if (m_bTracking)
	{
		if (point.y < m_rectLimit.top)
			point.y = m_rectLimit.top;
		else if (point.y > m_rectLimit.bottom)
			point.y = m_rectLimit.bottom;
		if (point.x < m_rectLimit.left)
			point.x = m_rectLimit.left;
		else if (point.x > m_rectLimit.right)
			point.x = m_rectLimit.right;

		if (m_htTrack == vSplitterBar1)
		{
			if (m_pRowInfo[0].nCurSize != point.y - m_nTrackPos)
			{
				m_pRowInfo[0].nIdealSize = point.y - m_nTrackPos;
				RecalcLayout();
				RedrawWindow();
				m_bChange = TRUE;
			}
		}
		else if (m_htTrack == hSplitterBar1)
		{
			if (m_pColInfo[0].nIdealSize != point.x - m_nTrackPos)
			{
				m_pColInfo[0].nIdealSize = point.x - m_nTrackPos;
				RecalcLayout();
				RedrawWindow();
				m_bChange = TRUE;
			}
		}
	}
	else
	{
		int ht = HitTest(point);
		SetSplitCursor(ht);
	}
}

void CDualSplitWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (!m_pColInfo)
		return;
/*
	if (m_nCols==2 && m_nRows==1)	// vertical resize
	{
		int cxCur, cxMin;
		GetColumnInfo(0, cxCur, cxMin);

		if (m_bChange && m_nPrev>0)
		{
			m_dRatio = (double)cxCur / (double)m_nPrev;	// calc new ratio
			m_bChange = FALSE;
		}
		SetColumnInfo(0, (int)(cx * m_dRatio), cxMin);
		RecalcLayout();

		m_nPrev = cx;
	}
	else if (m_nCols==1 && m_nRows==2)	// horizontal resize
	{
		int cyCur, cyMin;
		GetRowInfo(0, cyCur, cyMin);

		if (m_bChange && m_nPrev>0)
		{
			m_dRatio = (double)cyCur / (double)m_nPrev;
			m_bChange = FALSE;
		}
		SetRowInfo(0, (int)(cy * m_dRatio), cyMin);
		RecalcLayout();

		m_nPrev = cy;
	}
*/

	SetColumnInfo(0, 235, 0);

	CSplitterWnd::OnSize(nType, cx, cy);
}


void CDualSplitWnd::StartTracking(int ht)
{
	if (!m_bDragFull)
	{
		CSplitterWnd::StartTracking(ht);
		return;
	}

	GetInsideRect(m_rectLimit);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (ht >= vSplitterBar1 && ht <= vSplitterBar15)
		m_nTrackPos = ptCursor.y - m_pRowInfo[0].nCurSize;
	else if (ht >= hSplitterBar1 && ht <= hSplitterBar15)
		m_nTrackPos = ptCursor.x - m_pColInfo[0].nCurSize;

	SetCapture();

	m_bTracking = TRUE;
	m_htTrack = ht;
	SetSplitCursor(ht);
}


void CDualSplitWnd::StopTracking(BOOL bAccept)
{
	if (!m_bDragFull)
		CSplitterWnd::StopTracking(bAccept);
	else
	{
		ReleaseCapture();
		m_bTracking = FALSE;
	}
}

void CDualSplitWnd::LayoutRowCol(CSplitterWnd::CRowColInfo* pInfoArray,
									int nMax, int nSize, int nSizeSplitter)
{
	ASSERT(pInfoArray != NULL);
	ASSERT(nMax > 0);
	ASSERT(nSizeSplitter > 0);

	CSplitterWnd::CRowColInfo* pInfo;
	int i;

	if (nSize < 0)
		nSize = 0;  // if really too small, layout as zero size

	// start with ideal sizes
	for (i = 0, pInfo = pInfoArray; i < nMax-1; i++, pInfo++)
	{
		if (pInfo->nIdealSize < pInfo->nMinSize)
			pInfo->nIdealSize = 0;      // too small to see
		pInfo->nCurSize = pInfo->nIdealSize;
	}
	pInfo->nCurSize = INT_MAX;  // last row/column takes the rest

	for (i = 0, pInfo = pInfoArray; i < nMax; i++, pInfo++)
	{
		ASSERT(nSize >= 0);
		if (nSize == 0)
		{
			// no more room (set pane to be invisible)
			pInfo->nCurSize = 0;
			continue;       // don't worry about splitters
		}
		else if (nSize < pInfo->nMinSize && i != 0)
		{
			// additional panes below the recommended minimum size
			//   aren't shown and the size goes to the previous pane
			pInfo->nCurSize = 0;

			// previous pane already has room for splitter + border
			//   add remaining size and remove the extra border
			(pInfo-1)->nCurSize += nSize + 2;
			nSize = 0;
		}
		else
		{
			// otherwise we can add the second pane
			ASSERT(nSize > 0);
			if (pInfo->nCurSize == 0)
			{
				// too small to see
				if (i != 0)
					pInfo->nCurSize = 0;
			}
			else if (nSize < pInfo->nCurSize)
			{
				// this row/col won't fit completely - make as small as possible
				pInfo->nCurSize = nSize;
				nSize = 0;
			}
			else
			{
				// can fit everything
				nSize -= pInfo->nCurSize;
			}
		}

		// see if we should add a splitter
		ASSERT(nSize >= 0);
		if (i != nMax - 1)
		{
			// should have a splitter
			if (nSize > nSizeSplitter)
			{
				nSize -= nSizeSplitter; // leave room for splitter + border
				ASSERT(nSize > 0);
			}
			else
			{
				// not enough room - add left over less splitter size
				pInfo->nCurSize += nSize;
				if (pInfo->nCurSize > (nSizeSplitter - 2))
					pInfo->nCurSize -= (nSizeSplitter - 2);
				nSize = 0;
			}
		}
	}
	ASSERT(nSize == 0); // all space should be allocated
}

/* Original Method Implementation:
*
* AFX_STATIC void AFXAPI _AfxDeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout,
*	CWnd* pWnd, int x, int y, int cx, int cy, BOOL bScrollBar)
*
* In WINSPLIT.CPP
*
* Credit for this adaptation goes to Kris.
*/
// repositions client area of specified window
// assumes everything has WS_BORDER or is inset like it does
//  (includes scroll bars)
void CDualSplitWnd::DeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout,
									  CWnd* pWnd, int x, int y, int cx, int cy, BOOL bScrollBar)
{
	ASSERT(pWnd != NULL);
	ASSERT(pWnd->m_hWnd != NULL);

	if (bScrollBar)
	{
		// if there is enough room, draw scroll bar without border
		// if there is not enough room, set the WS_BORDER bit so that
		//   we will at least get a proper border drawn
		BOOL bNeedBorder = (cx <= 1 || cy <= 1);
		pWnd->ModifyStyle(bNeedBorder ? 0 : 1, bNeedBorder ? 1 : 0);
	}
	CRect rect(x, y, x+cx, y+cy);

	// adjust for 3d border (splitter windows have implied border)
	if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) || pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
	{
		rect.InflateRect(1, 1); // for proper draw CFlatSplitterWnd in view
		//		rect.InflateRect(afxData.cxBorder2, afxData.cyBorder2);
	}

	// first check if the new rectangle is the same as the current
	CRect rectOld;
	pWnd->GetWindowRect(rectOld);
	pWnd->GetParent()->ScreenToClient(&rectOld);
	if (rect != rectOld)
		AfxRepositionWindow(lpLayout, pWnd->m_hWnd, rect);
}

void CDualSplitWnd::RecalcLayout()
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 0 && m_nCols > 0); // must have at least one pane

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

	CRect rectInside;
	GetInsideRect(rectInside);

	// layout columns (restrict to possible sizes)
	LayoutRowCol(m_pColInfo, m_nCols, rectInside.Width(), m_cxSplitterGap);
	LayoutRowCol(m_pRowInfo, m_nRows, rectInside.Height(), m_cySplitterGap);

	// adjust the panes (and optionally scroll bars)

	// give the hint for the maximum number of HWNDs
	AFX_SIZEPARENTPARAMS layout;
	layout.hDWP = ::BeginDeferWindowPos((m_nCols + 1) * (m_nRows + 1) + 1);

	// size of scrollbars
	int cx = rectClient.right - rectInside.right;
	int cy = rectClient.bottom - rectInside.bottom;

	// reposition size box
	if (m_bHasHScroll && m_bHasVScroll)
	{
		CWnd* pScrollBar = GetDlgItem(AFX_IDW_SIZE_BOX);
		ASSERT(pScrollBar != NULL);

		// fix style if necessary
		BOOL bSizingParent = (GetSizingParent() != NULL);
		// modifyStyle returns TRUE if style changes
		if (pScrollBar->ModifyStyle(SBS_SIZEGRIP|SBS_SIZEBOX,
			bSizingParent ? SBS_SIZEGRIP : SBS_SIZEBOX))
			pScrollBar->Invalidate();
		pScrollBar->EnableWindow(bSizingParent);

		// reposition the size box
		DeferClientPos(&layout, pScrollBar,
			rectInside.right,
			rectInside.bottom, cx, cy, TRUE);
	}

	// reposition scroll bars
	if (m_bHasHScroll)
	{
		int cxSplitterBox = m_cxSplitter;// split box bigger
		int x = rectClient.left;
		int y = rectInside.bottom;
		for (int col = 0; col < m_nCols; col++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_HSCROLL_FIRST + col);
			ASSERT(pScrollBar != NULL);
			int cx = m_pColInfo[col].nCurSize;
			if (col == 0 && m_nCols < m_nMaxCols)
				x += cxSplitterBox, cx -= cxSplitterBox;
			DeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
			x += cx + m_cxSplitterGap;
		}
	}

	if (m_bHasVScroll)
	{
		int cySplitterBox = m_cySplitter;// split box bigger
		int x = rectInside.right;
		int y = rectClient.top;
		for (int row = 0; row < m_nRows; row++)
		{
			CWnd* pScrollBar = GetDlgItem(AFX_IDW_VSCROLL_FIRST + row);
			ASSERT(pScrollBar != NULL);
			int cy = m_pRowInfo[row].nCurSize;
			if (row == 0 && m_nRows < m_nMaxRows)
				y += cySplitterBox, cy -= cySplitterBox;
			DeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
			y += cy + m_cySplitterGap;
		}
	}

	//BLOCK: Reposition all the panes
	{
		int x = rectClient.left;
		for (int col = 0; col < m_nCols; col++)
		{
			int cx = m_pColInfo[col].nCurSize;
			int y = rectClient.top;
			for (int row = 0; row < m_nRows; row++)
			{
				int cy = m_pRowInfo[row].nCurSize;
				CWnd* pWnd = GetPane(row, col);
				DeferClientPos(&layout, pWnd, x, y, cx, cy, FALSE);
				y += cy + m_cySplitterGap;
			}
			x += cx + m_cxSplitterGap;
		}
	}

	// move and resize all the windows at once!
	if(layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP) )
		ASSERT("Warning: DeferWindowPos failed - low system resources.\n");

	// invalidate all the splitter bars (with NULL pDC)
	DrawAllSplitBars(NULL, rectInside.right, rectInside.bottom);
}
