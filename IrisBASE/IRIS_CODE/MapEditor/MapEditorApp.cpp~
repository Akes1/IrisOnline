#include "stdafx.h"
#include "MapEditorApp.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "Engine/Application.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

cMapEditorApp theApp;

BEGIN_MESSAGE_MAP(cMapEditorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()

cMapEditorApp::cMapEditorApp()
{
}

BOOL cMapEditorApp::InitInstance()
{
	/// 프로그램 중복 실행 방지
//	if( CheckMutex() == false )
//	{
//		return FALSE;
//	}

	/// 실행 파일의 경로를 기반 디렉토리로 설정
	TCHAR exePathName[MAX_PATH];
	::GetModuleFileName( AfxGetApp()->m_hInstance, exePathName, MAX_PATH );

	cString exePath = exePathName;
	GetFilePath( &exePath, exePath );

	SetCurrentDirectory( exePath.Cstr() );
	mBaseDir = exePath;

	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControls()가 필요합니다. 
	// InitCommonControls()를 사용하지 않으면 창을 만들 수 없습니다.
	InitCommonControls();

	if( CWinApp::InitInstance() == FALSE )
		return FALSE;

	// OLE 라이브러리를 초기화
	if(!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// 타이머 초기화: 멀티미디어 타이머 해상도를 1000분의 1로 조정
	timeBeginPeriod(1);

	/// 프레임 생성
	cMainFrame* frame = new cMainFrame;
	if( frame == 0)
		return FALSE;
	m_pMainWnd = frame;

	// 프레임을 만들어 리소스와 함께 로드합니다.
	if( frame->LoadFrame( IDR_MAINFRAME ) == 0 )
		return FALSE;

	// 창 하나만 초기화되었으므로 이를 표시하고 업데이트합니다.
	frame->ShowWindow( SW_SHOW );
	frame->UpdateWindow();


	return TRUE;
}

bool cMapEditorApp::CheckMutex()
{
	/// 뮤텍스 생성
	HANDLE mutex = ::CreateMutex( 0, TRUE, "Iris MapEditor Mutex" );

	/// 뮤텍스가 이미 존재하는지 검사
	bool alreadyExist = false;

	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
		alreadyExist = true;

	if( mutex )
		::ReleaseMutex( mutex );

	if( alreadyExist )
	{
		return false;
	}
	return true;
}

BOOL cMapEditorApp::OnIdle( LONG count )
{
	if( VIEW == m_pMainWnd->GetFocus() )
	{
		VIEW->Update();
		return TRUE;
	}
	else
	{
		return CWinApp::OnIdle(count);
	}
}

class cAboutDialog : public CDialog
{
public:
	cAboutDialog();

	enum { IDD = IDD_ABOUTBOX };
};

cAboutDialog::cAboutDialog()
: CDialog(cAboutDialog::IDD)
{
}

void cMapEditorApp::OnAppAbout()
{
	cAboutDialog aboutDlg;
	aboutDlg.DoModal();
}
