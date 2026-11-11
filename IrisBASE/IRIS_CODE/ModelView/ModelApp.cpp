#include "stdafx.h"
#include "ModelApp.h"
#include "MainFrame.h"

#include "ModelDoc.h"
#include "ModelView.h"


#include "MPIDocTemplate.h"
#include "MPIChildFrame.h"

#include "CharacterForm.h"
#include "MonsterForm.h"
#include "NpcForm.h"
#include "Application.h"

#include "Languagedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

cModelApp theApp;

BEGIN_MESSAGE_MAP(cModelApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_COLOR1, OnBackColor1)
	ON_COMMAND(ID_COLOR2, OnBackColor2)
	ON_COMMAND(ID_COLOR3, OnBackColor3)
	ON_COMMAND(ID_COLOR4, OnBackColor4)
	ON_COMMAND(ID_COLOR5, OnBackColor5)
	ON_COMMAND(ID_COLOR6, OnBackColor6)
END_MESSAGE_MAP()

cModelApp::cModelApp()
{
	mColorChecked = 0;
}

BOOL cModelApp::InitInstance()
{

	/// 프로그램 중복 실행 방지
	if( CheckMutex() == false )
	{
		return FALSE;
	}

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

	//// OLE 라이브러리를 초기화
	//if(!AfxOleInit())
	//{
	//	AfxMessageBox(IDP_OLE_INIT_FAILED);
	//	return FALSE;
	//}
	//AfxEnableControlContainer();

	/// 언어 선택 Dialog띄우기
	cLanguageDlg dlg;
	dlg.DoModal();
	///

	// 타이머 초기화: 멀티미디어 타이머 해상도를 1000분의 1로 조정
	timeBeginPeriod(1);

	// create the MPI document template
	CMPIDocTemplate* pDocTemplate;
	pDocTemplate = new CMPIDocTemplate( 
		IDR_MAINFRAME, 
		RUNTIME_CLASS(CModelDoc), 
		RUNTIME_CLASS(CMPIChildFrame) );
	AddDocTemplate(pDocTemplate);


	/// 프레임 생성
	cMainFrame* frame = new cMainFrame;
	if( frame == 0)
		return FALSE;
	m_pMainWnd = frame;

	// 프레임을 만들어 리소스와 함께 로드합니다.
	if( frame->LoadFrame( IDR_MAINFRAME ) == 0 )
		return FALSE;

	pDocTemplate->CreateNewDocument();
	BEGIN_MPI_FRAME(pDocTemplate, 1)
		MPID_VSPLIT(ID_SPLITTER_MAIN, 25)
		MPI_TABS(IDR_MAINTABS)
		MPID_VIEW(IDD_DIALOG_CHARACTER, cCharacterForm)
		MPID_VIEW(IDD_DIALOG_MONSTER, cMonsterForm)
		MPID_VIEW(IDD_DIALOG_NPC, cNpcForm)

		MPID_VIEW(ID_FORM_MODEL, CModelView)
	END_MPI_FRAME


	/// model view init
	CMPIChildFrame* frm = pDocTemplate->GetChildFrame(0);
	CModelView* pView = (CModelView*)frm->GetWndFromUID(ID_FORM_MODEL);
	pView->Init();

	mMainSplitter = (CSplitterWnd*)frm->GetWndFromUID(ID_SPLITTER_MAIN);
	mMainSplitter->SetColumnInfo( 0, 400,0 );
	
	// 창 하나만 초기화되었으므로 이를 표시하고 업데이트합니다.
	frame->MDIActivate( pDocTemplate->GetChildFrame(0) );
	frame->ShowWindow( SW_SHOW );
	frame->UpdateWindow();

	return TRUE;
}

bool cModelApp::CheckMutex()
{
	/// 뮤텍스 생성
	HANDLE mutex = ::CreateMutex( 0, TRUE, "Iris RegenTool Mutex" );

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

BOOL cModelApp::OnIdle( LONG count )
{
	if( m_pMainWnd->IsTopParentActive() )
	{
		VIEW->Update();
		return TRUE;
	}

	return CWinApp::OnIdle(count);
}

void cModelApp::OnBackColor1()
{
	VIEW->ChangeBackColor( NiColor(0.0f, 0.0f, 0.0f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR1, MF_CHECKED );

	mColorChecked = 0;
}

void cModelApp::OnBackColor2()
{
	VIEW->ChangeBackColor( NiColor(1.0f, 1.0f, 1.0f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR2, MF_CHECKED );

	mColorChecked = 1;
}

void cModelApp::OnBackColor3()
{
	VIEW->ChangeBackColor( NiColor(0.5f, 0.5f, 0.5f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR3, MF_CHECKED );

	mColorChecked = 2;
}

void cModelApp::OnBackColor4()
{
	VIEW->ChangeBackColor( NiColor(0.0f, 174.0f/255.0f, 239.0f/255.0f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR4, MF_CHECKED );

	mColorChecked = 3;
}

void cModelApp::OnBackColor5()
{
	VIEW->ChangeBackColor( NiColor(1.0f, 247.0f/255.0f, 153.0f/255.0f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR5, MF_CHECKED );

	mColorChecked = 4;
}

void cModelApp::OnBackColor6()
{
	VIEW->ChangeBackColor( NiColor(246.0f/255.0f, 150.0f/255.0f, 121.0f/255.0f) );

	HMENU menu = GetMenu( m_pMainWnd->m_hWnd );
	CheckMenuItem( menu, ID_COLOR1+mColorChecked, MF_UNCHECKED );
	CheckMenuItem( menu, ID_COLOR6, MF_CHECKED );

	mColorChecked = 5;
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

void cModelApp::OnAppAbout()
{
	cAboutDialog aboutDlg;
	aboutDlg.DoModal();
}
