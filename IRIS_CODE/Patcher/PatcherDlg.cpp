#include "stdafx.h"
#include "PatcherDlg.h"

#include "Patcher.h"
#include "skin/SkinResource.h"
//#include "HSUpChk.h"

//#define HS_GAMECODE					5831
//#define HS_LICENSE					"CBC1EE5976E5748C02F7AF5D"
//#define szTitle						"IRIS ONLINE"

// 핵실드 스마트 업데이트 
// Ex 함수를 사용할때는 반드시 HSUpSetEvn.exe 설정 툴로 env 파일에 게임코드를 입력해야 한다!
BOOL UpdateHackShield()
{
/*
	DWORD	ret = 0;
	TCHAR	szMsg[255] = {0,};
	TCHAR	szFullFilePath[MAX_PATH] = {0,};

	// 핵실드 폴더 위치 지정
	GetCurrentDirectory( MAX_PATH, szFullFilePath );
	lstrcat( szFullFilePath, _T( "\\HackShield" ) );

	AHNHS_EXT_ERRORINFO error;
	ZeroMemory( &error, sizeof(AHNHS_EXT_ERRORINFO) );

	// 모니터링 정보를 지정합니다.
	strcpy( error.szServer, "127.0.0.1" );
	strcpy( error.szUserId, "Test" );
	strcpy( error.szGameVersion, "0.9" );

	// Update 함수 호출
	ret = _AhnHS_HSUpdateEx( szFullFilePath, 
							 1000 * 600,	// 업데이트 전체 타임 아웃
							 HS_GAMECODE,
							 AHNHSUPDATE_CHKOPT_HOSTFILE | AHNHSUPDATE_CHKOPT_GAMECODE,
							 error,
							 1000 * 20 );	// 서버 연결 타임 아웃

	if( ret != ERROR_SUCCESS )
	{
		//_stprintf(szMsg, _T("해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다."), ret );
		_stprintf(szMsg, _T("Fail to run HackShield.(Error Code = %x)\nProgram will be close."), ret );
		MessageBox(NULL, szMsg, szTitle, MB_OK);
		return FALSE;
	}
*/		
	return TRUE;
}


BEGIN_MESSAGE_MAP( cPatcherDlg, cSkinDialog )
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()

	ON_BN_CLICKED( CID_OK, OnOkButton )
	ON_BN_CLICKED( CID_CLOSE, OnCloseButton )
	ON_BN_CLICKED( CID_HOME, OnHomeButton )
	ON_BN_CLICKED( CID_JOIN, OnJoinButton )
	ON_BN_CLICKED( CID_FULL, OnScreenModeButton )
	ON_CBN_SELCHANGE( CID_LANGUAGE, OnSelChangeLanguage )
	ON_CBN_SELCHANGE( CID_RESOLUTION, OnSelChangeResolution )

	ON_MESSAGE(WM_USER_MESSAGE, OnUserMessage)
END_MESSAGE_MAP()



cPatcher* cPatcherDlg::mPatcher = 0;

cPatcherDlg::cPatcherDlg( const cString& path, const cString& authCode, CWnd* parent )
: cSkinDialog( parent )
, mPath( path )
, mAuthCode( authCode )
, mThread( 0 )
, mProgressTotalNum( 0 )
, mProgressProcessCnt ( 0 )
, mExecuteMode( eCLIENT_EXECUTE_IDC )
{
	mIcon = AfxGetApp()->LoadIcon( IDI_PATCHER_ICON );

	mBoardView  = 0;
	mStatus     = PATCH_SEARCHING;

	mSaveOption     = false;
    mPatchMode      = ePATCH_MODE_DOWNLOAD;
}

cPatcherDlg::~cPatcherDlg()
{
	if( mPatcher )
	{
		delete mPatcher;
		mPatcher = 0;
	}

	for(unsigned int i = 0; i < mLanguageArr.GetSize(); i++)
	{
		sLangauge* p = (sLangauge*)(mLanguageArr[i]);
		if( p )
		{
			delete p;
			p = 0;
		}
	}
	mLanguageArr.Clear();

	for(unsigned int i = 0; i < mResolutionArr.GetSize(); i++)
	{
		sResolution* p = (sResolution*)(mResolutionArr[i]);
		if( p )
		{
			delete p;
			p = 0;
		}
	}
	mResolutionArr.Clear();

	if( mSaveOption == true )
	{
		SaveOption( "./option/option.txt" );
	}

}

int cPatcherDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	lpCreateStruct->style |= WS_CLIPCHILDREN;

	if( cSkinDialog::OnCreate( lpCreateStruct ) == -1 )
		return -1;
	
	/// 윈도우 타이틀
	SetWindowText( "IrisPatcher" );

	/// Url 정보 로딩
	if( LoadUrl() == false )
	{
		MessageBox( "Failed to load url info", PATCHER );
		return -1;
	}

	/// load config
	if( LoadPatchConfig( "./launcher/patchconfig.txt" ) == false )
	{
		MessageBox( "Failed to load config file", PATCHER, MB_OK );
		return -1;
	}

	/// option.txt 로딩
	if( LoadOption("./Option/option.txt") == false )
	{
		/// default set
		mOptionData.mWindowed   = false;
		mOptionData.mWidth      = 1024;
		mOptionData.mHeight     = 768;
		mOptionData.mBpp        = 32;
		mOptionData.mGamma      = 0.5f;

		mOptionData.mEnvVolume = 0.5f;	///0.0f ~ 1.0f
		mOptionData.mEffVolume = 0.5f;	///0.0f ~ 1.0f

		mOptionData.mVideoMode = 20;

		mOptionData.mLanguage = 81;

		mSaveOption = true;
	}

    /// 지원가능한 해상도 로딩
	EnumResolution();

	/// 게임 실행 버튼
	mStartButton.Create( mResource->GetChild( "start" ), this, CID_OK );
	mStartButton.EnableWindow( 0 );

	/// 종료 버튼
	mCloseButton.Create( mResource->GetChild( "close" ), this, CID_CLOSE );

	/// 홈페이지 버튼
	mHomeButton.Create( mResource->GetChild( "home" ), this, CID_HOME );

	/// 회원가입 버튼
	mJoinButton.Create( mResource->GetChild( "join" ), this, CID_JOIN );

	/// 진행바
	mProgressBar.Create( mResource->GetChild( "probar" ), this, CID_PROGRESS );
	mProgressBar.SetValue( 0.0f );

	/// 게시물 뷰
	mBoardView = new cSkinHtmlView;
	mBoardView->Create( mResource->GetChild( "board" ), this, CID_BOARD );
	mBoardView->Go();

	//////////////////////////////////////////////////////////////////////////
	/// 정보 셋팅
	{
		mLanguageCombo.Create( mResource->GetChild( "language" ), this, CID_LANGUAGE );
		mLanguageCombo.SetFont( &mFont );

		if( mLanguageArr.GetSize() == 0 )
		{
			/// Default 언어를 제공
			sLangauge* p = new sLangauge;
			p->str = "japan";
			p->code = 81;
			mLanguageArr.PushBack( p );

			mSaveOption = true;
		}

		unsigned int curSel = 0;
		for( unsigned int i = 0; i<mLanguageArr.GetSize(); ++i )
		{
			sLangauge* p = (sLangauge*)mLanguageArr[i];
			if( p )
			{
				int r = mLanguageCombo.AddString( p->str.Cstr() );
				mLanguageCombo.SetItemDataPtr(r, (void*)p );

				if( p->code == mOptionData.mLanguage )
					curSel = r;
			}
		}
		mLanguageCombo.SetCurSel( curSel );
	}

	if( mLanguageArr.GetSize() == 1 )
	{
		mLanguageCombo.EnableWindow( FALSE );
		mLanguageCombo.ShowWindow( SW_HIDE );
	}
	//////////////////////////////////////////////////////////////////////////

	mScreenCheck.Create( mResource->GetChild("full"), this, CID_FULL );
	mScreenCheck.SetFont( &mFont );
	mScreenCheck.SetChecked( mOptionData.mWindowed );

	//////////////////////////////////////////////////////////////////////////
	/// 정보 셋팅
	{
		mResolutionCombo.Create( mResource->GetChild( "resolution" ), this, CID_RESOLUTION );
		mResolutionCombo.SetFont( &mFont );

		unsigned int curSel = 0;
		unsigned int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		unsigned int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		for( unsigned int i = 0; i<mResolutionArr.GetSize(); ++i )
		{
			sResolution* p = (sResolution*)mResolutionArr[i];
			if( p )
			{
				if( mOptionData.mWindowed == true )
				{
					if( p->width > screenWidth || p->height > screenHeight )
						continue;
				}

				int r= mResolutionCombo.AddString( p->strResolution.Cstr() );
				mResolutionCombo.SetItemDataPtr(r, (void*)p );

				if( p->width == mOptionData.mWidth && p->height == mOptionData.mHeight && p->bpp == mOptionData.mBpp )
					curSel = r;
			}
		}
		mResolutionCombo.SetCurSel(curSel);
	}
	//////////////////////////////////////////////////////////////////////////

	RECT rc;
 	rc.left = 21;
 	rc.right = 128;
	rc.top = 394;
	rc.bottom = 408;

	mStatusStatic.Create( "", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rc, this,  CID_STATUS );
	mStatusStatic.SetFont( &mFont );

	return 0;
}


BOOL cPatcherDlg::OnInitDialog()
{
	cSkinDialog::OnInitDialog();

	/// 아이콘을 설정
	SetIcon( mIcon, 1 );
	SetIcon( mIcon, 0 );

	mLanguageCombo.InitShow();
	mResolutionCombo.InitShow();

    mPatchMode = ePATCH_MODE_DOWNLOAD;

	/// 스레드를 시작
	mPatcher = new cPatcher( this );
	mPatcher->Init( GetSafeHwnd(), AfxGetInstanceHandle() );

    mPatcher->SetExcuteMode( mExecuteMode ) ;

	BeginThread();

	return 1;
}

void cPatcherDlg::OnDestroy()
{
	// 공지 사항 뷰
	if( mBoardView )
	{
		mBoardView->DestroyWindow();
	}

    SetPatchMode( ePATCH_MODE_EXIT );

	cSkinDialog::OnDestroy();
}

void cPatcherDlg::OnPaint()
{
	if( IsIconic() )
	{
		CPaintDC dc( this );

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

		/// 아이콘 드로잉
		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect( &rect );
		int x = ( rect.Width() - cxIcon + 1 ) / 2;
		int y = ( rect.Height() - cyIcon + 1 ) / 2;

		dc.DrawIcon( x, y, mIcon );
	}
	else
	{
		cSkinDialog::OnPaint();
	}
}

HCURSOR cPatcherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>( mIcon );
}

void cPatcherDlg::OnOK()
{
	static bool first = true;

	if( first == false )
		return;
	else
		first = false;

	if( mStartButton.IsWindowEnabled() )
	{
//        mPatchMode = ePATCH_MODE_PATCH_CHECK;
		// 게임 실행
		RunGame();
		EndDialog( 0 );
	}
}

void cPatcherDlg::OnCancel()
{
//    SetPatchMode( ePATCH_MODE_EXIT );

	if( mThread )
	{
 		mPatcher->StopWorking();
		::Sleep( 1 );
		/// IO 작업이 끝날 때 까지 기다린다.
        while( mPatcher->IsEndIO() != true )
		{
			::Sleep( 1 );
			/// wait for IO END...
		}

		/// 스레드가 종료될 때까지 대기
		if( ::WaitForSingleObject( mThread, INFINITE ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			::GetExitCodeThread( mThread, &exitCode );
//			if ( exitCode == STILL_ACTIVE )
//			{
//				::TerminateThread( mThread, 0 );
//			}
		}

		::CloseHandle( mThread );
		mThread = 0;
	}

	//if( mPatchStep == eSTEP_PATCH_COMPLETED )
	{
		cSkinDialog::OnCancel();
	}

	PostMessage( WM_CLOSE, 0, 0 );
}

void cPatcherDlg::OnOkButton()
{
	OnOK();
}

void cPatcherDlg::OnCloseButton()
{
	OnCancel();
}

void cPatcherDlg::OnHomeButton()
{
	ShellExecute( 0, 0, "Iexplore.exe", mUrlArray[URL_HOME].Cstr(), 0, SW_SHOWNORMAL );
}


void cPatcherDlg::OnJoinButton()
{
	ShellExecute( 0, 0, "Iexplore.exe", mUrlArray[URL_JOIN].Cstr(), 0, SW_SHOWNORMAL );
}

void cPatcherDlg::OnScreenModeButton()
{
	mOptionData.mWindowed = !mOptionData.mWindowed;

	//////////////////////////////////////////////////////////////////////////
	/// 정보 셋팅
	mResolutionCombo.ResetContent();

	unsigned int curSel = 0;
	unsigned int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	unsigned int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	for( unsigned int i = 0; i<mResolutionArr.GetSize(); ++i )
	{
		sResolution* p = (sResolution*)mResolutionArr[i];
		if( p )
		{
			if( mOptionData.mWindowed == true )
			{
				if( p->width > screenWidth || p->height > screenHeight )
					continue;
			}

			int r= mResolutionCombo.AddString( p->strResolution.Cstr() );
			mResolutionCombo.SetItemDataPtr(r, (void*)p );

			if( p->width == mOptionData.mWidth && p->height == mOptionData.mHeight && p->bpp == mOptionData.mBpp )
				curSel = r;
		}
	}
	mResolutionCombo.SetCurSel(curSel);
	//////////////////////////////////////////////////////////////////////////

	mSaveOption = true;
}

void cPatcherDlg::OnSelChangeLanguage()
{
	if( mLanguageArr.GetSize() == 0 )
		return;

	int curSel = mLanguageCombo.GetCurSel();
	if( curSel == -1 )
		return;


	sLangauge* p = (sLangauge*)mLanguageCombo.GetItemDataPtr( curSel );
	if( p )
	{
		mOptionData.mLanguage = p->code;

		mSaveOption = true;
	}
}

void cPatcherDlg::OnSelChangeResolution()
{
	if( mResolutionArr.GetSize() == 0 )
		return;

	int curSel = mResolutionCombo.GetCurSel();
	if( curSel == -1 )
		return;

	sResolution* p = (sResolution*)mResolutionCombo.GetItemDataPtr( curSel );
	if( p )
	{
		if( mOptionData.mWidth == p->width && mOptionData.mHeight == p->height && mOptionData.mBpp == p->bpp )
			return;

		mOptionData.mWidth = p->width;
		mOptionData.mHeight = p->height;
		mOptionData.mBpp = p->bpp;

		mSaveOption = true;
	}
}

void cPatcherDlg::BeginThread()
{
	/// 이미 작업중인 스레드가 있는지를 검사
	if( mThread != 0 )
	{
		MessageBox( "Thread is working.", PATCHER );
		return;
	}

	/// 스레드를 생성
	unsigned int threadId = 0;
	mThread = (HANDLE)_beginthreadex( NULL, 0, ThreadProc, 0, 0, &threadId );
//	mThread = ::CreateThread( 0, 0, ThreadProc, 0, 0, &threadId );

	if( mThread == 0 )
	{
		MessageBox( "Failed to create thread.", PATCHER );
		return;
	}
}

void cPatcherDlg::GameReady( bool result )
{
/*
    if( mPatchMode == ePATCH_MODE_DOWNLOAD_END || 
        mPatchMode == ePATCH_MODE_GAME_RUN     )
    {
        mProgressBar.SetValue( 1.0f );
		mProgressBar.Invalidate( 0 );
    }
*/
	if( result )
	{
		/// 게임 실행 준비
		mStartButton.EnableWindow( 1 );
        mProgressBar.SetValue( 1.0f );
		mProgressBar.Invalidate( 0 );
//		mStatus = PATCH_COMPLETE;
	}
/*
    else
    {
		mStartButton.EnableWindow( 0 );
    }
*/
}

unsigned int cPatcherDlg::ThreadProc( LPVOID /*param*/ )
{
	assert( mPatcher );
	return mPatcher->ThreadProc();
}

bool cPatcherDlg::LoadUrl()
{
	cString fileName = mPath;
	fileName += URLINFO_FILENAME;

	cFileLoader loader;
	if( loader.Open( fileName, true ) == false )
	{
		return false;
	}

	char temp[256];

	for( int i = 0; i < URL_COUNT; ++i )
	{
        int readLen = loader.ReadLine( temp, 256 );
		if( readLen == 0 || readLen > 256  )
			break;

        temp[readLen-1] = 0;
        mUrlArray[i].Format("%s", temp );

        memset( temp, 0, sizeof(temp) );
	}

	return true;
}

bool cPatcherDlg::RunGame()
{
	char *pCurrentPath = 0;

	if ( mPatcher )
	{
		pCurrentPath = mPatcher->GetWorkDirectory();
		SetCurrentDirectoryA( pCurrentPath );
	}
/*
    // 개선필요 : 홈페이지 주소 쓰는곳 검색해서 스크립트로 로드해서 적용하는 것으로..
    // client에 넘길 command line setting   
    cString cmdLine;
    if ( strcmp( mAuthCode.Cstr(), "-1" ) == 0 )
    {
        // 이전 런처 실행시(-1) 홈페이지 open
#ifdef _TEST_SRV
        ShellExecute( 0, 0, "Iexplore.exe", WEB_LAUNCHER_URL, 0, SW_SHOWNORMAL );
#else
        ShellExecute( 0, 0, "Iexplore.exe", WEB_LAUNCHER_URL, 0, SW_SHOWNORMAL );
#endif
        PostMessage( WM_CLOSE, 0 );
        return true;
    }
    else if ( strcmp( mAuthCode.Cstr(), "-2" ) == 0 )
    {
        // 직접 실행
        cmdLine.Format( "%u", mPatcher->GetLoginIP() );
    }
    else
    {
        // 웹런처 실행 [ip_인증코드]
        cmdLine.Format( "%u|%s", mPatcher->GetLoginIP(), mAuthCode.Cstr() );
    }

    /// 핵실드 적용
	if( mExecuteMode == eCLIENT_EXECUTE_IDC  || 
		mExecuteMode == eCLIENT_EXECUTE_TEST ||
		mExecuteMode == eCLIENT_EXECUTE_DEV  )
	{
		// 업데이트
		if( UpdateHackShield() == FALSE )
		{
			MessageBox( "Failed to update HackShield", PATCHER );
			return 0;
		}
	}
*/
	cString cmdLine;
	cmdLine.Format( "%u", mPatcher->GetLoginIP() );


	/// otion 저장
	if( mSaveOption )
	{
		cString str;
		str.Format( "%s\\%s", pCurrentPath, "option/option.txt");
		SaveOption( str.Cstr() );
	}

	cString path;
    if( mExecuteMode == eCLIENT_EXECUTE_GMTOOL_IDC  || 
        mExecuteMode == eCLIENT_EXECUTE_GMTOOL_TEST || 
        mExecuteMode == eCLIENT_EXECUTE_GMTOOL_DEV  )
    {
		path.Format( "%s\\%s", pCurrentPath, GAMEEXE_FILENAME_GM ); 
    }
    else
        path.Format( "%s\\%s", pCurrentPath, GAMEEXE_FILENAME );

	int ret = (int)::ShellExecute( 0, "open", path.Cstr(), cmdLine.Cstr(), 0, SW_SHOWNORMAL );

	if( ret >= 0 && ret <= 32 )
	{
        char szMsg[255] = {0,};
        sprintf_s(szMsg, "ERROR(%d) : Failed to run [%s]", ret, path.Cstr() );
        MessageBoxA(szMsg, PATCHER );

        PostMessage( WM_CLOSE, 0 );
		return false;
	}

    PostMessage( WM_CLOSE, 0 );
	return true;
}

void cPatcherDlg::SetProgressMax( ePatchStatus status, unsigned int max )
{
	mProgressTotalNum = max;
	mProgressBar.SetValue( 0.f );
	mProgressBar.Invalidate( 0 );

	SetStatus( status );
}

void cPatcherDlg::SetProgressPos( const char* /*fileName*/, unsigned int pos )
{
	mProgressProcessCnt = pos;
	float r = float(pos) / float(mProgressTotalNum);

	mProgressBar.SetValue( r );
	mProgressBar.Invalidate( 0 );

	SetStatus( PATCH_PATCHING );
}

void cPatcherDlg::InitStatus( )
{
	mStatusStatic.SetWindowText( "" );

	RECT rc;
 	rc.left = 21;
 	rc.right = 128;
	rc.top = 394;
	rc.bottom = 408;
	InvalidateRect( &rc, 0 );
}

void cPatcherDlg::SetStatus( ePatchStatus status )
{
	mStatus = status;

	cString out;
	switch( mStatus )
	{
	case PATCH_SEARCHING:
		out = mResource->GetText( 0 );
		break;
	case PATCH_PATCHING:
		{
			float v = mProgressBar.GetValue() * 100.0f;
			out.Format( mResource->GetText(1).Cstr(), v);
		}
		break;
	case PATCH_COMPLETE:
		out = mResource->GetText( 2 );
        break;
	case PATCH_LIST_CHECKING:
		out = mResource->GetText( 3 );
        break;
	case PATCH_LIST_CHECK_END:
		out = mResource->GetText( 4 );
		break;
	}

	mStatusStatic.SetWindowText( out.Cstr() );

	RECT rc;
 	rc.left = 21;
 	rc.right = 128;
	rc.top = 394;
	rc.bottom = 408;
	InvalidateRect( &rc, 0 );
}

HBRUSH cPatcherDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkMode(TRANSPARENT);
			return hbr;
		}
	}
	return hbr;
}

LRESULT cPatcherDlg::OnUserMessage(WPARAM wParam, LPARAM lParam)
{
	if(mPatcher->IsWorking() == false)
		return 1;

	switch( wParam )
	{
 	case _UMT_ERROR_INIT_ : 
 		MessageBox("Initialize  Error.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_SAVE_ : 
//		MessageBox("Fail to save file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_CONNECT_:
		MessageBox("Fail to connect to patch server.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_DOWNLOAD_:
//		MessageBox("Fail to download patch file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_DOWNLOAD_PATCHLIST_:
		MessageBox("Fail to download  patchlist file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_NOT_EXIST_PATCHLIST_:
		MessageBox("Fail to download. Patchlist file is not exist.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MEMORY_ALLOC_:
		MessageBox( "Fail to allocation memory.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MEMORY_FREE_:
		MessageBox( "Fail to free memory.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_UPDATE_PACK_FILE:
//		MessageBox( "Fail to Update Pack file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_UPDATE_NOPACK_FILE:
//		MessageBox( "Fail to Update UnPack file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_UPDATE_OPEN_PACK_FILE_:
		MessageBox( "Fail to open pack file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_UPDATE_ACCESS_PACK_FILE_:
		MessageBox( "Fail to access pack file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_SEARCH_:
		MessageBox( "Failed to search client files.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_CREATE_PACK_FILE_:
		MessageBox( "Fail to create pack file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_CREATE_FOLDER_:
		MessageBox( "Fail to create client folder.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MAKELIST_WRONG_HEADER:
		MessageBox("Fail to parse server patch list header file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MAKELIST_WRONG_EXECUTE:
		MessageBox("Fail to wrong execute mode.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MAKELIST_DIFF_PATCHLIST:
		MessageBox("Fail. execute mode different server patch list file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MAKELIST_PARSE:
		MessageBox("Fail to parse server patch list file.", PATCHER, MB_OK );
		break;
	case _UMT_ERROR_MAKELIST_END_APP:
		break;
    case _UMT_ERROR_PATCH_CHECK_FAIL:
		MessageBox("Patch differance Last File.", PATCHER, MB_OK );
        break;
	default : 
		MessageBox("Unknown Error");
		break;
	}

	PostMessage( WM_CLOSE, 0, 0 );

	return 1;
}

bool cPatcherDlg::LoadPatchConfig( const char* pathName )
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	if( loader.IsEmpty() )
		return false;

	for(unsigned int i = 0; i < mLanguageArr.GetSize(); i++)
	{
		sLangauge* p = (sLangauge*)(mLanguageArr[i]);
		if( p )
		{
			delete p;
			p = 0;
		}
	}
	mLanguageArr.Clear();

	cToken token;
	cPatchLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_LOCAL:
			{
				cString str = parser.ParseString();

				sLangauge* p = new sLangauge;
				p->str = str;
				p->code = GetLocalCode( p->str.Cstr() );

				mLanguageArr.PushBack( p );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cPatcherDlg::LoadOption( const char* pathName )
{
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
		return false;

	cToken token;
	cOptionLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_WINMODE:
			{
				mOptionData.mWindowed = parser.ParseInt()? true:false;
			}
			break;
		case eTOKEN_RESOLUTION:
			{
				mOptionData.mWidth = (unsigned int)parser.ParseInt();
				mOptionData.mHeight = (unsigned int)parser.ParseInt();
				mOptionData.mBpp = (unsigned char)parser.ParseInt();
			}
			break;
		case eTOKEN_RAMP:
			{
				mOptionData.mGamma = parser.ParseFloat();
				if( mOptionData.mGamma < 0.0f || mOptionData.mGamma > 1.0f )
					mOptionData.mGamma = 0.5f;
			}
			break;
		case eTOKEN_ENVVOLUME:
			{
				mOptionData.mEnvVolume = parser.ParseFloat();

				if( mOptionData.mEnvVolume < 0.0f || mOptionData.mEnvVolume > 1.0f )
					mOptionData.mEnvVolume = 0.5f;
			}
			break;
		case eTOKEN_EFFVOLUME:
			{
				mOptionData.mEffVolume = parser.ParseFloat();
				if( mOptionData.mEffVolume < 0.0f || mOptionData.mEffVolume > 1.0f )
					mOptionData.mEffVolume = 0.5f;
			}
			break;
		case eTOKEN_VIDEOMODE:
			{
				mOptionData.mVideoMode = parser.ParseInt();
			}
			break;
		case eTOKEN_LANGUAGE:
			{
				mOptionData.mLanguage = parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cPatcherDlg::SaveOption( const char* pathName )
{
	///
	if( ::FileExist( "Option" ) == false )
		::CreateDirectory( "Option", 0 );

	cFileSaver saver;
	if( saver.Open( pathName ) == false )
	{
		return false;
	}

	char buf[256] = {0,};

	/// window mode
	::sprintf_s( buf, "windowed %d\r\n", mOptionData.mWindowed? 1:0 );
	saver.Write( buf, strlen(buf) );

	/// resolution
	::sprintf_s( buf, "resolution %d %d %d\r\n", mOptionData.mWidth, mOptionData.mHeight, mOptionData.mBpp );
	saver.Write( buf, strlen(buf) );

	/// ramp
	::sprintf_s( buf, "ramp %f\r\n", mOptionData.mGamma );
	saver.Write( buf, strlen(buf) );

	/// back sound volume
	::sprintf_s( buf, "envvolume %f\r\n", mOptionData.mEnvVolume );
	saver.Write( buf, strlen(buf) );

	/// effect sound volume
	::sprintf_s( buf, "effvolume %f\r\n", mOptionData.mEffVolume );
	saver.Write( buf, strlen(buf) );

	/// video capture mode
	::sprintf_s( buf, "video %d ", mOptionData.mVideoMode );
	saver.Write( buf, strlen(buf) );

	::sprintf_s( buf, "language %d ", mOptionData.mLanguage );
	saver.Write( buf, strlen(buf) );

	mSaveOption = false;

	return true;
}

void cPatcherDlg::EnumResolution( )
{ 
	///
	for(unsigned int i = 0; i < mResolutionArr.GetSize(); i++)
	{
		sResolution* p = (sResolution*)(mResolutionArr[i]);
		if( p )
		{
			delete p;
			p = 0;
		}
	}
	mResolutionArr.Clear();

	///
	/// 그래픽 카드 해상도
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));

	dm.dmSize = sizeof(dm);
	EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm );

	int i = 0;

	while( EnumDisplaySettings(NULL, i++, &dm) )
	{
		/// 불필요한 모드 제외
		if( dm.dmPelsWidth <= 800 || dm.dmPelsHeight <= 600 || dm.dmBitsPerPel < 16 || dm.dmDisplayFrequency != 60 )
			continue;

		sResolution* p = new sResolution;
		p->width = dm.dmPelsWidth;
		p->height = dm.dmPelsHeight;
		p->bpp = (unsigned char)dm.dmBitsPerPel;

		p->strResolution.Format( "%dx%dx%d", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel );

		mResolutionArr.PushBack( p );
	}

	/// sorting
}

WORD cPatcherDlg::GetLocalCode( const char* local )
{
    if( strcmp( local, "Corea"     ) == 0 )         return 82;
    else if( strcmp( local, "japan"     ) == 0 )    return 81;
    else if( strcmp( local, "hongkong"  ) == 0 )    return 852;

	/// default
    return 0;
}
