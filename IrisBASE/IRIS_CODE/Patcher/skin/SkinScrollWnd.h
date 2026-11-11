#pragma once

//
#include "SkinScrollBar.h"

class cSkinScrollWnd : public CWnd
{
public:
	cSkinScrollWnd();

public:
	WNDPROC		m_funOldProc;

public:
	BOOL SkinWindow(CWnd *pWnd,HBITMAP hBmpScroll);
	virtual ~cSkinScrollWnd();

protected:
	afx_msg LRESULT OnDestMove(WPARAM wParam,LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	int				m_nScrollWid;
	cSkinScrollBar	m_sbHorz,m_sbVert;
	CWnd			m_wndLimit;
	HBITMAP			m_hBmpScroll;
	BOOL			m_bOp;
	int				m_nAngleType;
};

extern "C" BOOL UnskinWndScroll(CWnd *pWnd);

extern "C" cSkinScrollWnd* SkinWndScroll(CWnd *pWnd,HBITMAP hBmpScroll);
