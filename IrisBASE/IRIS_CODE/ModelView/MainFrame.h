// MainFrame.h : cMainFrame 클래스의 인터페이스
//
#pragma once

#include "MPITabWnd.h"

class cMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(cMainFrame)
public:
	cMainFrame();

	// 재정의
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// 구현
public:
	virtual ~cMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// 메시지 맵 함수를 생성했습니다.
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

protected:
public:
	afx_msg void OnAppDramaReload();
};


