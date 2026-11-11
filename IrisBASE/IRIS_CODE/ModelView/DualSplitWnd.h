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

#pragma once

class CDualSplitWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CDualSplitWnd);
public:
	CDualSplitWnd();
	virtual ~CDualSplitWnd();

	virtual void RecalcLayout();    // call after changing sizes
	static void DeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout,
		CWnd* pWnd, int x, int y, int cx, int cy, BOOL bScrollBar);
	static void LayoutRowCol(CSplitterWnd::CRowColInfo* pInfoArray,
		int nMax, int nSize, int nSizeSplitter);

public:
	// Initial ratio for two-pane splitter
	void SetInitialRatio(double dRatio) { m_dRatio = dRatio; }

	// Get current ratio, so it can be stored in configuration file
	double GetRatio() const { return m_dRatio; }

public:
	virtual void OnInvertTracker(const CRect& rect);
	virtual void SetSplitCursor(int ht);

protected:
	virtual void StartTracking(int ht);
	virtual void StopTracking(BOOL bAccept);

	enum HitTestValue
	{
		noHit = 0,
		vSplitterBox = 1,
		hSplitterBox = 2,
		bothSplitterBox = 3,
		vSplitterBar1 = 101,
		vSplitterBar15 = 115,
		hSplitterBar1 = 201,
		hSplitterBar15 = 215,
		splitterIntersection1 = 301,
		splitterIntersection225 = 525
	};

	int m_nPrev;
	BOOL m_bChange;
	double m_dRatio;
	BOOL m_bDragFull;
	int m_nTrackPos;

protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

