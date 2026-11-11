#include "stdafx.h"
#include "PatcherApp.h"
#include "PatcherDlg.h"

#include "FileSystem.h"
#include "HttpDownloader.h"

#define PATCHER_MUTEX			"IrisPatcher Mutex"

cPatcherApp theApp;

cPatcherApp::cPatcherApp()
{
	mMutex = 0;
	mSkinManager = 0;

	/// 파일 시스템 생성
	new cFileSystem;
}

cPatcherApp::~cPatcherApp()
{
	if( mSkinManager )
	{
		delete mSkinManager;
		mSkinManager = 0;
	}

	delete FILESYSTEM;

	if( mMutex )
		::CloseHandle( mMutex );
}

BOOL cPatcherApp::InitInstance()
{
	CWinApp::InitInstance();

	/// 프로그램 중복 실행 방지
	if( CheckMutex() == false )
	{
		MessageBox( 0, "ERROR(01) : Patcher is already running", PATCHER, MB_OK );
		return FALSE;
	}

	eCLIENT_EXECUTE_MODE executeMode = eCLIENT_EXECUTE_IDC;

    /// CmdLine : [client실행모드_인증코드]
    char*   tMode      = NULL;
    char*   tAuthCode  = NULL;
    cString authCode;

    tMode = strtok( m_lpCmdLine, " " );
    if( !tMode ) 
	{
		MessageBox( 0, "ERROR(02) : This program must be executed by Launcher", PATCHER, MB_OK );
		return FALSE;
	}

    tAuthCode = strtok( NULL, " " );

#if defined( _DEVSYS )
    // 인증코드가 없거나 irisLauncher_DEV.exe를 통해 실행된 것이 아니면, 홈페이지 open
    //if( !tAuthCode || strcmp(tAuthCode, "-2") != 0 )
    //{
    //    ShellExecute( 0, 0, "Iexplore.exe", WEB_LAUNCHER_URL, 0, SW_SHOWNORMAL );
    //    return FALSE;
    //}

	if( ::stricmp(tMode, "-dkdlfltmeoqkr" ) == 0 )
		executeMode = eCLIENT_EXECUTE_IDC;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkr-DEV" ) == 0 )
		executeMode = eCLIENT_EXECUTE_DEV;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkr-TEST" ) == 0 )
		executeMode = eCLIENT_EXECUTE_TEST;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkrGM-IDC" ) == 0 )
		executeMode = eCLIENT_EXECUTE_GMTOOL_IDC;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkrGM-DEV" ) == 0 )
		executeMode = eCLIENT_EXECUTE_GMTOOL_DEV;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkrGM-TEST" ) == 0 )
		executeMode = eCLIENT_EXECUTE_GMTOOL_TEST;
	else
	{
        MessageBox( 0, "ERROR(02) : This program must be executed by Launcher", PATCHER, MB_OK );
		return 0;
	}
#elif defined( _TEST_SRV )
	if( ::stricmp(tMode, "-dkdlfltmeoqkr-TEST" ) == 0 )
		executeMode = eCLIENT_EXECUTE_TEST;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkrGM-TEST" ) == 0 )
		executeMode = eCLIENT_EXECUTE_GMTOOL_TEST;
	else
	{
		MessageBox( 0, "ERROR(02) : This program must be executed by Launcher", PATCHER, MB_OK );
		return 0;
	}

    // 인증코드가 없으면, 홈페이지 open
    //if( !tAuthCode )
    //{
    //    ShellExecute( 0, 0, "Iexplore.exe", WEB_LAUNCHER_URL, 0, SW_SHOWNORMAL );
    //    return FALSE;
    //}

#else

	if( ::stricmp(tMode, "-dkdlfltmeoqkr" ) == 0 )
		executeMode = eCLIENT_EXECUTE_IDC;
	else if( ::stricmp(tMode, "-dkdlfltmeoqkrGM-IDC" ) == 0 )
		executeMode = eCLIENT_EXECUTE_GMTOOL_IDC;
	else
	{
		MessageBox( 0, "ERROR(02) : This program must be executed by Launcher", PATCHER, MB_OK );
		return 0;
	}

    // 인증코드가 없으면, 홈페이지 open
    //if( !tAuthCode )
    //{
    //    ShellExecute( 0, 0, "Iexplore.exe", WEB_LAUNCHER_URL, 0, SW_SHOWNORMAL );
    //    return FALSE;
    //}

#endif

     authCode.Format( "%s", tAuthCode );

	/// Initialize COM Library - INTERNET API(wininet.h/wininet.lib)를 사용하기 위해
	if( CoInitialize( 0 ) != S_OK )
	{
		MessageBox( 0, "CoInitialize Failed!", PATCHER, MB_OK );
		return 0;
	}

    /// 스킨 정보 로딩
	if( LoadSkin() == false )
	{ 
		MessageBox( 0, "Failed to load skin info.", PATCHER, MB_OK );
		return FALSE;
	}

    /// 다이얼로그 띄움
    cPatcherDlg dlg( mPath, authCode );
    m_pMainWnd = &dlg;
    dlg.SetExecuteGM( executeMode );
    dlg.DoModal( mSkinManager->GetContainer( "main" ) );

	return TRUE;

}

int cPatcherApp::ExitInstance()
{
	CoUninitialize();
	return 0;
}

bool cPatcherApp::CheckMutex()
{
	/// 뮤텍스 생성
	HANDLE mutex = ::CreateMutex( 0, TRUE, PATCHER_MUTEX );

	/// 뮤텍스가 이미 존재하는지 검사
	bool alreadyExist = false;
	if( ::GetLastError() == ERROR_ALREADY_EXISTS )
		alreadyExist = true;

	mUsageCnt++;
	if( mUsageCnt > 1 )
        alreadyExist = true;

    if( ::FindWindow( NULL, "IrisPatcher" ) )
        alreadyExist = true;

	if( alreadyExist == false )
		mMutex = mutex;
	else if( mutex )
		::ReleaseMutex( mutex );

	return !alreadyExist;
}

bool cPatcherApp::LoadSkin()
{
	/// 경로 설정
	mPath.Format( "Launcher/japan/" );

	if( ::FileExist( mPath ) == false )
	{
		MessageBox( 0, "Failed to access local path", PATCHER, MB_OK );
        return false;
	}

	/// 스킨 관리자 생성
	mSkinManager = new cSkinManager( mPath );

	/// 스킨 리소스 로딩
	return mSkinManager->Load( mPath + SKNINFO_FILENAME );
}
