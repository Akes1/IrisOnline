/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 탐색기 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "ExplorerFileTreeCtrl.h"

/// 탐색기 윈도우
class cExplorerWindow : public CView
{
protected:
	cExplorerWindow();
	DECLARE_DYNCREATE(cExplorerWindow)

public:
	virtual void OnDraw(CDC* dc);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();

public:
	virtual ~cExplorerWindow();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* poldWnd);
	DECLARE_MESSAGE_MAP()
	
private:
	/// 파일 트리
	cExplorerFileTreeCtrl mFileTree;
};
