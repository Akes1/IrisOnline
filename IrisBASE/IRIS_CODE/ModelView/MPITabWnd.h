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

#pragma once

#include "MPITabCtrl.h"

class CMPITabWnd : public CSplitterWnd
{
public:
	CMPITabWnd();
	virtual ~CMPITabWnd();

public:
	// Show the given page
	void SetPage(int nPage);

	// Load tabs from toolbar resource
	int CreateTabs(int nResourceID);

public:
	virtual void RecalcLayout();
protected:
	virtual int HitTest(CPoint pt) const;
	virtual void DrawAllSplitBars(CDC* pDC, int cxInside, int cyInside);

protected:
	CMPITabCtrl m_wndTabCtrl;
	int m_nCurPage;

protected:
	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

