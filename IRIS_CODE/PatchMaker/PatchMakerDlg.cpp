// PatchMakerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "PatchMaker.h"
#include "PatchMakerDlg.h"

#include <io.h>
#include "PatchMaker.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPatchMakerDlg 대화 상자
CPatchMakerDlg::CPatchMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatchMakerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CPatchMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_WORKER,  mEditWorker);
	DDX_Control(pDX, IDC_EDIT_SOURCE,  mEditSource);
	DDX_Control(pDX, IDC_EDIT_TARGET,  mEditTarget);
	DDX_Control(pDX, IDC_EDIT_BACKUP,  mEditBackup);
	DDX_Control(pDX, IDC_EDIT_SOURCE_LIST,  mEditSourceList);
}

BEGIN_MESSAGE_MAP(CPatchMakerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE( WM_PATCH_MSG, PatchMsg )

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_WORKER, &CPatchMakerDlg::OnBnClickedButtonWorker)
	ON_BN_CLICKED(IDC_BUTTON_SOURCE, &CPatchMakerDlg::OnBnClickedButtonSource)
	ON_BN_CLICKED(IDC_BUTTON_TARGET, &CPatchMakerDlg::OnBnClickedButtonTarget)
	ON_BN_CLICKED(IDC_BUTTON_BACKUP, &CPatchMakerDlg::OnBnClickedButtonBackup)
	ON_BN_CLICKED(IDC_BUTTON_SOURCE_LIST, &CPatchMakerDlg::OnBnClickedButtonSourceList)
	ON_BN_CLICKED(IDC_BUTTON_START, &CPatchMakerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BT_OPTION, &CPatchMakerDlg::OnClickDelOption)
END_MESSAGE_MAP()


// CPatchMakerDlg 메시지 처리기
BOOL CPatchMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	LoadOptionFile();
	
	mPatchMaker = new Patchmaker( );

	if( mPatchMaker == NULL )
		EndDialog( IDCANCEL );

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

BOOL CPatchMakerDlg::DestroyWindow()
{
	/// 옵션 파일 자동 저장.
	SaveCurrentPathOption();
	
	if( mPatchMaker )
		mPatchMaker->Destroy();

	SafeDelete( mPatchMaker );

	return CDialog::DestroyWindow();
}

void CPatchMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CPatchMakerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CPatchMakerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/// 폴더 탐색기
BOOL CPatchMakerDlg::BrowseFolder( TCHAR* pTitle, TCHAR* pFolderPath )
{
	ITEMIDLIST        *pidlBrowse  = NULL;

	BROWSEINFO BrInfo;
	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.pidlRoot  = NULL;

	memset( &BrInfo, 0, sizeof(BrInfo) );
	BrInfo.pszDisplayName = pFolderPath;
	BrInfo.lpszTitle = pTitle;
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS;

	// 다이얼로그를 띄우기
	pidlBrowse = ::SHBrowseForFolder(&BrInfo);    

	if( pidlBrowse != NULL)
	{
		// 패스를 얻어옴
		::SHGetPathFromIDList(pidlBrowse, pFolderPath);    
		return true;
	}
	return false;
}

void CPatchMakerDlg::OnBnClickedButtonWorker()
{
	TCHAR    pathName[MAX_PATH] = {0,};
	TCHAR*	 pTitle = "Select Worker Directory";

	if( BrowseFolder( pTitle, pathName ) == TRUE )
		mEditWorker.SetWindowText( pathName );
}
void CPatchMakerDlg::OnBnClickedButtonSource()
{
	TCHAR    pathName[MAX_PATH] = {0,};
	TCHAR*	 pTitle = "Select Source Directory";

	if( BrowseFolder( pTitle, pathName ) == TRUE )
		mEditSource.SetWindowText( pathName );
}

void CPatchMakerDlg::OnBnClickedButtonTarget()
{
	TCHAR    pathName[MAX_PATH] = {0,};
	TCHAR*	 pTitle = "Select Target Directory";

	if( BrowseFolder( pTitle, pathName ) == TRUE )
		mEditTarget.SetWindowText( pathName );
}

void CPatchMakerDlg::OnBnClickedButtonBackup()
{
	TCHAR    pathName[MAX_PATH] = {0,};
	TCHAR*	 pTitle = "Select Backup Directory";

	if( BrowseFolder( pTitle, pathName ) == TRUE )
		mEditBackup.SetWindowText( pathName );
}
void CPatchMakerDlg::OnBnClickedButtonSourceList()
{
	TCHAR    pathName[MAX_PATH] = {0,};
	TCHAR*	 pTitle = "Select Source List";

	if( BrowseFolder( pTitle, pathName ) == TRUE )
		mEditSourceList.SetWindowText( pathName );
}

void CPatchMakerDlg::LoadOptionFile()
{
	CStdioFile optionFile;
	CFileException ex;
	CString strFileName;
	TCHAR currentPath[MAX_PATH] = {0,};
	
	GetCurrentDirectory( MAX_PATH, currentPath );

	strFileName.Format( "%s\\option.txt", currentPath );
	
	if( optionFile.Open( strFileName, CFile::modeRead, &ex ) != 0 )
	{
		CString workerPath;
		CString sourcePath;
		CString targetPath;
		CString backupPath;
		CString sourceListPath;

		bool    optionCheck = false;

		BOOL isReadLine;
		
		isReadLine=optionFile.ReadString(workerPath);
		if( isReadLine == FALSE )
			return;
		
		isReadLine=optionFile.ReadString(sourcePath);
		if( isReadLine == FALSE )
			return;
		
		isReadLine=optionFile.ReadString(targetPath);
		if( isReadLine == FALSE )
			return;
		
		isReadLine=optionFile.ReadString(backupPath);
		if( isReadLine == FALSE )
			return;
		
		isReadLine=optionFile.ReadString(sourceListPath);
		if( isReadLine == FALSE )
			return;


		if( _access( workerPath, 0 ) == 0 )
			mEditWorker.SetWindowText( workerPath );
		
		if( _access( sourcePath, 0 ) == 0 )
			mEditSource.SetWindowText( sourcePath );
		
		if( _access( targetPath, 0 ) == 0 )
			mEditTarget.SetWindowText( targetPath );
		
		if( _access( backupPath, 0 ) == 0 )
			mEditBackup.SetWindowText( backupPath );
		
		if( _access( sourceListPath, 0 ) == 0 )
			mEditSourceList.SetWindowText( sourceListPath );

		optionFile.Close();
	}
}

void CPatchMakerDlg::LoadPatchList()
{
	CFile		   patchListFile;
	CFileException ex;
	CString		   strPath;
	TCHAR		   currentPath[MAX_PATH] = {0,};
	TCHAR*		   buf    = NULL;
	UINT		   bufSize = 0;
	bool		   sourcePathExist = false;

	GetCurrentDirectory( MAX_PATH, currentPath );

	mEditSourceList.GetWindowText( strPath );

	if( _access( strPath.GetBuffer(), 0 ) == 0 )
		sourcePathExist = true;

	strPath += "\\SourceList.txt";

	/// 소스 리스트 경로는 있는데 파일이 없으면 소스폴더에서 소스리스트에서 재구성.
	if( _access( strPath.GetBuffer(), 0 ) != 0 && sourcePathExist == true )
	{
		mEditSource.GetWindowText( strPath );
		mPatchMaker->SetStatus( GetSafeHwnd(), "Collect Source List..." );

		if( mPatchMaker->CollectListInfo( strPath.GetBuffer(), NULL )  == 0 )
			mPatchMaker->SetUseSourceListFlag( true );
		return;
	}

	if( patchListFile.Open( strPath, CFile::modeRead, &ex ) != 0 )
	{
		bufSize = (UINT)patchListFile.GetLength();

		buf = new TCHAR[bufSize]; 
		
		patchListFile.Read( buf, bufSize );

		mPatchMaker->ReadSourceList( buf );
		
		delete[] buf;

		patchListFile.Close();
	}
}

void CPatchMakerDlg::SaveCurrentPathOption()
{
	CStdioFile optionFile;
	CFileException ex;
	CString strFileName;
	TCHAR currentPath[MAX_PATH] = {0,};

	GetCurrentDirectory( MAX_PATH, currentPath );

	strFileName.Format( "%s\\option.txt", currentPath );

	if( optionFile.Open( strFileName, CFile::modeCreate | CFile::modeWrite, &ex ) != 0 )
	{
		CString workerPath;
		CString sourcePath;
		CString targetPath;
		CString backupPath;
		CString sourceListPath;

		mEditWorker.GetWindowText( workerPath );
		mEditSource.GetWindowText( sourcePath );
		mEditTarget.GetWindowText( targetPath );
		mEditBackup.GetWindowText( backupPath );
		mEditSourceList.GetWindowText( sourceListPath );

		workerPath += "\n";
		sourcePath  += "\n";
		targetPath  += "\n";
		backupPath  += "\n";
		sourceListPath += "\n";

		/// 라인 단위로 값 저장
		optionFile.WriteString( workerPath );
		optionFile.WriteString( sourcePath );
		optionFile.WriteString( targetPath );
		optionFile.WriteString( backupPath );
		optionFile.WriteString( sourceListPath );

		optionFile.Close();
	}
}
void CPatchMakerDlg::OnBnClickedButtonStart()
{
	/// 실행중 필요없는 부분 닫기.
	SetEnableState( FALSE );

	CString workerPath;
	CString sourcePath;
	CString targetPath;
	CString backupPath;
	CString sourceListPath;

	mEditWorker.GetWindowText( workerPath );
	mEditSource.GetWindowText( sourcePath );
	mEditTarget.GetWindowText( targetPath );
	mEditBackup.GetWindowText( backupPath );
	mEditSourceList.GetWindowText( sourceListPath );

	SendDlgItemMessage( IDC_PROGRESS , PBM_SETPOS, 0, 0 );

	mPatchMaker->ClearList();

	if( mPatchMaker->SetDirectory( workerPath.GetBuffer(), sourcePath.GetBuffer(), targetPath.GetBuffer(), backupPath.GetBuffer(), sourceListPath.GetBuffer() ) == false )
		return;

	LoadPatchList();

	if( mPatchMaker->Run( GetSafeHwnd() ) == -1 )
		return;
}

LRESULT CPatchMakerDlg::PatchMsg(WPARAM wParam, LPARAM lParam )
{
	if( wParam == ePM_COMPLETE || wParam == ePM_ERROR )
		SetEnableState( TRUE );

	return mPatchMaker->PatchMsg( GetSafeHwnd(), wParam, lParam );

}

void CPatchMakerDlg::SetEnableState( BOOL isPatchMaking )
{
	mEditWorker.EnableWindow( isPatchMaking );
	mEditSource.EnableWindow( isPatchMaking );
	mEditTarget.EnableWindow( isPatchMaking );
	mEditBackup.EnableWindow( isPatchMaking );
	mEditSourceList.EnableWindow( isPatchMaking );

	GetDlgItem( IDC_BUTTON_WORKER )->EnableWindow( isPatchMaking );
	GetDlgItem( IDC_BUTTON_SOURCE )->EnableWindow( isPatchMaking );
	GetDlgItem( IDC_BUTTON_TARGET )->EnableWindow( isPatchMaking );
	GetDlgItem( IDC_BUTTON_BACKUP )->EnableWindow( isPatchMaking );
	GetDlgItem( IDC_BUTTON_SOURCE_LIST )->EnableWindow( isPatchMaking );
	GetDlgItem( IDC_BUTTON_START )->EnableWindow( isPatchMaking );
}

void CPatchMakerDlg::OnClickDelOption()
{
	CButton * pSource = (CButton*)GetDlgItem( IDC_CHK_SOURCE );
	CButton * pTarget = (CButton*)GetDlgItem( IDC_CHK_TARGETPATH );

	CString path;

	/// sourceList.txt 삭제
	if( pSource->GetCheck() )
	{
		mEditSourceList.GetWindowText( path );
		path += "\\SourceList.txt";

		if( _access( path, 0 ) == 0 )
			DeleteFile( path );
	}

	/// target Directory 파일들 삭제
	if( pTarget->GetCheck() )
	{
		char target[ 1024 ] = { 0, };
		mEditTarget.GetWindowText( path );
		strcpy_s( target, path.GetBuffer(path.GetLength()) );

		if( _access( target, 0 ) == 0 )
			mPatchMaker->DeleteSubFiles( target, NULL );
	}
}
