// MainFrame.cpp : cMainFrame 클래스의 구현
//

#include "stdafx.h"
#include "ModelApp.h"

#include "MainFrame.h"
#include "ModelView.h"
#include ".\mainframe.h"

#include "SceneManager.h"
#include "DramaturgyManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(cMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(cMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_APP_DRAMA_RELOAD, OnAppDramaReload)
END_MESSAGE_MAP()


// CMainFrame 생성/소멸

cMainFrame::cMainFrame()
{
	// TODO: 여기에 멤버 초기화 코드를 추가합니다.
}

cMainFrame::~cMainFrame()
{
}


int cMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL cMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}

// cMainFrame 진단

#ifdef _DEBUG
void cMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void cMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// cMainFrame 메시지 처리기
void cMainFrame::OnAppDramaReload()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( SCENEMAN->IsInited() == false )
		return;

	DRAMATURGYMAN->ReLoadDramaturgyFileList();
}
