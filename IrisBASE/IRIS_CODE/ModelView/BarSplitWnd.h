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


class CBarSplitWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CBarSplitWnd);
public:
	CBarSplitWnd();
	virtual ~CBarSplitWnd();

public:
	// pane that should be constant and its size
	void SetBarInfo(int nPane, int nSize);

protected:
	virtual int HitTest(CPoint pt) const;
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);

protected:
	int m_nBarPane;
	int m_nBarSize;

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
