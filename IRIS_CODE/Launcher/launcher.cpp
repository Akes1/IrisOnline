/*-- Include
*/
#include "launcher.h"
#include <stdio.h>
#include <winsock.h>
#include <process.h>

#ifdef _DEVSYS
	#include "DevLauncher.h"
#endif

/*-- Local definitions
*/
#define SafeDelete(P)	{ if (P!=NULL) { delete(P); (P)=NULL; } }       // 메모리를 안전하게 해제.

#define WM_UPDATE_MSG	WM_USER+0x10
#define TRAY_NOTIFY     WM_USER+0x11

/*-- Global data
*/
HINSTANCE         g_instance         = NULL;
Launcher*         g_launcher         = NULL;

/*-- GetString Prototype
*/
int GetString(int id, char* buffer, int bufferSize )
{
	return (g_instance != NULL) ? LoadString( g_instance, id, buffer, bufferSize ) : 0;
}

/*-- WinMain Prototypeo
*/
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	InitCommonControls( );

	Launcher* launcher = new Launcher( );
	HANDLE    handle   = CreateMutex( NULL, TRUE, launcher->GetWindowName( ) );
	int       result   = GetLastError( );

	mUsageCnt++;
	if( mUsageCnt > 1 )
        result = ERROR_ALREADY_EXISTS;

    if( ::FindWindow( NULL, launcher->GetWindowName( ) ) )
        result = ERROR_ALREADY_EXISTS;

	g_instance = instance;
	if ( handle != NULL )
	{
		if ( result != ERROR_ALREADY_EXISTS )
			result = launcher->Run( instance, prevInstance, cmdLine, cmdShow );

		CloseHandle( handle );
	}

	SafeDelete( launcher );

	return result;
}

/*-- DlgProc Prototype
*/
LRESULT CALLBACK DlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_launcher->MsgProc( wnd, msg, wParam, lParam );
}

/*-- WorkerThreadStartingPoint Prototype
*/
unsigned __stdcall WorkerThreadStartingPoint(void* ptr)
{
	return g_launcher->WorkerThread( );
}

/*-- Launcher Constructor
*/
Launcher::Launcher(void)
{
	g_launcher				= this;

	m_wnd					= NULL;
	m_RunModeSetDlgWnd		= NULL;
	m_icon					= NULL;
	m_libClassDesc			= NULL;
	m_workerThread			= NULL;

	m_windowName			= TEXT( "Launcher - IRIS ON LINE" );
	m_endIO					= true;
	m_endApp				= false;
	m_runPatcher			= false;

	m_packlist.pack			= NULL;
	m_packlist.offset		= 0;
	m_packlist.length		= 0;

	m_patchlist.file		= NULL;
	m_patchlist.offset		= 0;
	m_patchlist.length		= 0;

	m_DownLoadExitCode		= 0;
}

/*-- ~LiveUpdate Destructor
*/
Launcher::~Launcher(void)
{
	if ( m_packlist.pack != NULL )
	{
		GlobalFree( m_packlist.pack );
		m_packlist.pack		= NULL;
		m_packlist.offset	= 0;
		m_packlist.length	= 0;
	}

	if ( m_patchlist.file != NULL )
	{
		GlobalFree( m_patchlist.file );
		m_patchlist.file   = NULL;
		m_patchlist.offset = 0;
		m_patchlist.length = 0;
	}

	g_launcher = NULL;
	
	SafeDelete( m_libClassDesc );
	m_libClassDesc = NULL;

#ifdef _DEVSYS
 	SafeDelete( m_DevLauncher );
#endif

 	m_DevLauncher = NULL;
}


/*-- FileTimeToUTC Method
*/
void Launcher::FileTimeToUTC(FILETIME* filetime)
{
	SYSTEMTIME utc;
	FileTimeToSystemTime( filetime, &utc );
	SystemTimeToFileTime( &utc, filetime );
}

/*-- FileTimeToDOS Method
*/
void Launcher::FileTimeToDOS(FILETIME* filetime)
{
	WORD fatDate, fatTime;
	FileTimeToDosDateTime( filetime, &fatDate, &fatTime );
	DosDateTimeToFileTime( fatDate, fatTime, filetime );
}

/*-- Run Method
*/
int Launcher::Run(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	/// 현재 폴더 저장.
	GetCurrentDirectory( sizeof(m_currentDirectory), m_currentDirectory );

	/// 응용 프로그램, 다이얼로그 생성
	m_wnd = CreateDialog( instance, MAKEINTRESOURCE(IDD_LAUNCHER), NULL, (DLGPROC)DlgProc );
	if ( m_wnd == NULL )
		return GetLastError( );

	/// 윈도우 이름 입력
	SetWindowText( m_wnd, GetWindowName( ) );

#ifdef _DEVSYS

	ShowWindow( m_wnd, FALSE );

	/// 런쳐 실행 모드 로더 초기화
	m_DevLauncher		= new cDevLauncher;
	m_RunModeSetDlgWnd	= m_DevLauncher->Init( m_wnd );
#elif _LOCAL

	// 응용 프로그램, 다이얼로그 표시
	ShowWindow( m_wnd, TRUE );

	// 업데이트 시작.
	unsigned int threadId;
	m_workerThread = (HANDLE)_beginthreadex( NULL, 0, WorkerThreadStartingPoint, 0, 0, &threadId );
//	m_workerThread = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( m_workerThread == NULL )
	{
		DWORD lastError = GetLastError( );
		char  buffer[MAX_PATH];

		sprintf_s( buffer, "Error(=0x%08x)", lastError );
		MessageBox( m_wnd, buffer, GetWindowName( ), MB_OK | MB_ICONERROR );
		PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	}

#else

/*
    // 인증코드가 없으면 홈페이지 open
    if( strlen( cmdLine ) <= 0 )
    {
#ifdef _TEST_SRV
        ShellExecute( 0, 0, "Iexplore.exe", "http://iris.enpang.com", 0, SW_SHOWNORMAL );
#else
        ShellExecute( 0, 0, "Iexplore.exe", "http://iris.enpang.com", 0, SW_SHOWNORMAL );
#endif
        return ERROR_SUCCESS;
    }
*/

	// 응용 프로그램, 다이얼로그 표시
	ShowWindow( m_wnd, TRUE );

	// 업데이트 시작.
	unsigned int threadId;
	m_workerThread = (HANDLE)_beginthreadex( NULL, 0, WorkerThreadStartingPoint, 0, 0, &threadId );
//	m_workerThread = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( m_workerThread == NULL )
	{
		DWORD lastError = GetLastError( );
		char  buffer[MAX_PATH];

		sprintf_s( buffer, "Error(=0x%08x)", lastError );
		MessageBox( m_wnd, buffer, GetWindowName( ), MB_OK | MB_ICONERROR );
		PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	}
#endif

	// 메시지 루프 시작
	MSG  msg;
	BOOL result;

	while ( (result = GetMessage( &msg, NULL, 0, 0 )) )
	{
		if ( result == -1 )
		{
			return GetLastError( );
		}
		else if ( !IsWindow( m_wnd ) || !IsDialogMessage( m_wnd, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	// Patcher 실행
	if ( m_runPatcher == true )
	{
		SetCurrentDirectory( m_currentDirectory );

        char mode[50]     = {0,};
        char fileName[50] = {0,};



#ifdef _DEVSYS
		switch( m_DevLauncher->GetLauncherMode() )
		{
		case eLAUNCHER_MODE_GMTOOL_TEST: GetString( IDS_UPDATE_GM_EXECUTE_TEST,  mode, sizeof(mode) );  break;
		case eLAUNCHER_MODE_GMTOOL_IDC:  GetString( IDS_UPDATE_GM_EXECUTE_IDC,   mode, sizeof(mode) );  break;
//		case eLAUNCHER_MODE_CLIENT_TEST: GetString( IDS_UPDATE_EXECUTE_TEST,     mode, sizeof(mode) );  break;
//		case eLAUNCHER_MODE_CLIENT_IDC:  GetString( IDS_UPDATE_EXECUTE_IDC,      mode, sizeof(mode) );  break;
//      case eLAUNCHER_MODE_CLIENT_DEV:  GetString( IDS_UPDATE_EXECUTE_DEV,      mode, sizeof(mode) );  break;
//		case eLAUNCHER_MODE_GMTOOL_DEV:  GetString( IDS_UPDATE_GM_EXECUTE_DEV,   mode, sizeof(mode) );  break;
		}
/*
        if ( m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_CLIENT_DEV  ||
             m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_GMTOOL_DEV  )
        {
            GetString( IDS_UPDATE_EXE_DEV, fileName, sizeof(fileName) );
        }
        else
*/
        //if ( m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_CLIENT_TEST  ||
        if ( m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_GMTOOL_TEST  )
        {
            GetString( IDS_UPDATE_EXE_TEST, fileName, sizeof(fileName) );
        }
        else 
        {
            GetString( IDS_UPDATE_EXE, fileName, sizeof(fileName) );
        }

#elif _LOCAL
        GetString( IDS_UPDATE_EXE_TEST,     fileName, sizeof(fileName) );
        GetString( IDS_UPDATE_EXECUTE_TEST, mode,     sizeof(mode)     );

#elif _TEST_SRV
        GetString( IDS_UPDATE_EXE_TEST,     fileName, sizeof(fileName) );
        GetString( IDS_UPDATE_EXECUTE_TEST, mode,     sizeof(mode)     );
#else
        GetString( IDS_UPDATE_EXE,          fileName, sizeof(fileName) );
        GetString( IDS_UPDATE_EXECUTE_IDC,  mode,     sizeof(mode)     );
#endif

        if ( strlen( mode ) > 0 )
        {
		    char totalCmdLine[ MAX_PATH ] = { 0, };

            // ***웹런처를 통해서 실행되지 않으면 구분자 지정해서 넘겨줘야 한다.
#if defined (_DEVSYS) || defined (_LOCAL)
            sprintf_s( totalCmdLine, "%s -2", mode );
#else
            sprintf_s( totalCmdLine, "%s %s", mode, cmdLine );
#endif

			SHELLEXECUTEINFO shExecInfo;
			shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

			shExecInfo.fMask    = NULL;
			shExecInfo.hwnd     = NULL;
			shExecInfo.lpVerb   = "open";
			shExecInfo.lpFile   = fileName;
			shExecInfo.lpParameters = totalCmdLine;
			shExecInfo.lpDirectory  = NULL;
			shExecInfo.nShow    = SW_SHOWNORMAL;
			shExecInfo.hInstApp = NULL;

			ShellExecuteEx(&shExecInfo);
        }
	}
	return (int)msg.wParam;
}

/*-- MsgProc Method
*/
LRESULT Launcher::MsgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
	case WM_DESTROY:			return Destroy    ( wnd, wParam, lParam );
	case WM_INITDIALOG:			return InitDialog ( wnd, wParam, lParam );
	case WM_COMMAND:			return Command    ( wnd, wParam, lParam );
	case WM_UPDATE_MSG:			return UpdateMsg  ( wnd, wParam, lParam );

#ifdef _DEVSYS
	case WM_LAUNCHER_RUN_MSG:	return RunLauncher( );
#endif
	}

	return 0L;
}

/*-- SetStatus Method
*/
BOOL Launcher::SetStatus(HWND wnd, char* msg)
{
	if ( m_endApp == false )
	{
		HWND dlgItem = GetDlgItem( wnd, IDC_STATUS );
		BOOL retcode = SetWindowText( dlgItem, msg );
		UpdateWindow( dlgItem );
		return retcode;
	}
	return FALSE;
}

/*-- Destroy Method
*/
LRESULT Launcher::Destroy(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	// ICON 해제
	if ( m_icon != NULL )
	{
		DestroyIcon( m_icon );
		m_icon = NULL;
	}

	// 응용 프로그램을 종료 한다.
	PostQuitMessage( 0 );
	return 0L;
}

/*-- InitDialog Method
*/
LRESULT Launcher::InitDialog(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	// ICON 로드
	m_icon = LoadIcon( g_instance, MAKEINTRESOURCE( IDI_ICON ) );
	SendMessage( wnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)m_icon );

	// Progress Bar - 범위 설정(0 ~ 100%).
	SendMessage( GetDlgItem( wnd, IDC_PROGRESS ), PBM_SETRANGE, 0, MAKELPARAM(0,100) ); 

	// Dll 로드
	SetStatus( wnd, "Ready" );
	return 1L;
}

/*-- Command Method
*/
LRESULT Launcher::Command(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	switch ( LOWORD(wParam) )
	{
	case IDOK:
		break;
	case IDCANCEL:
		{
			// 안전종료.
			m_endApp = true;
			m_DownLoadExitCode = LPR_STOP;

			Sleep(1);

			// file I/O 가 끝날때까지 대기
			while( m_endIO != true )
			{
				Sleep(1);
				// wait for IO...
			}

			// m_updateThread 핸들를 해제.
			if ( m_workerThread != NULL )
			{
				// UpdateThread가 종료될때까지 대기.
				if ( WaitForSingleObjectEx( m_workerThread, INFINITE, TRUE ) != WAIT_OBJECT_0 )
				{
					DWORD exitCode;
					GetExitCodeThread( m_workerThread, &exitCode );
					//			if ( exitCode == STILL_ACTIVE )
					//			{
					//				TerminateThread( m_workerThread, 0 );
					//			}
				}
				CloseHandle( m_workerThread );
				m_workerThread = NULL;
			}

			DestroyWindow( wnd );
			if( m_RunModeSetDlgWnd )
				DestroyWindow( m_RunModeSetDlgWnd );
		}
		break;
	}
	return 0L;
}

/*-- UpdateMsg Method
*/
LRESULT Launcher::UpdateMsg(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case _UMT_UPDATE_OPEN_:
		MessageBox( wnd, "ERROR URL", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error URL" );
		break;
	case _UMT_UPDATE_LIST_:
		MessageBox( wnd, "ERROR DOWNLOAD", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Download" );
		break;
	case _UMT_UPDATE_FILE_:
		MessageBox( wnd, "ERROR UPDATE FILE", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_LOADER_INIT_:
		MessageBox( wnd, "ERROR UPDATE LOADER INIT", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_LOADER_OPEN_:
		MessageBox( wnd, "ERROR UPDATE LOADER OPEN", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_LOADER_DOWN_:
		MessageBox( wnd, "ERROR UPDATE LOADER DOWN", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_PATCHLIST_UNPACK_:
		MessageBox( wnd, "ERROR UPDATE PATCHLIST UNPACK", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_PATCHLIST_PARSING_:
		MessageBox( wnd, "ERROR UPDATE PATCHLIST PARSING", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_PATCHLIST_DOWN_:
		MessageBox( wnd, "ERROR UPDATE PATCHLIST DOWN", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_NOT_EXIST_PATCHLIST_:
		MessageBox( wnd, "ERROR UPDATE. PATCHLIST FILE NOT EXIST", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_PATCHFILE_DOWN_:
		MessageBox( wnd, "ERROR UPDATE PATCHFILE DOWN", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_SAVE_FILE_:
		MessageBox( wnd, "ERROR UPDATE SAVE FILE", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_ALLOCATION_:
		MessageBox( wnd, "ERROR UPDATE ALLOCATION", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_MAKELIST_WRONG_HEADER:
		MessageBox( wnd, "ERROR UPDATE PARSE SERVER PATCH LIST HEADER FILE.", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_MAKELIST_WRONG_EXECUTE:
		MessageBox( wnd, "ERROR UPDATE WRONG EXECUTE MODE.", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_MAKELIST_DIFF_PATCHLIST:
		MessageBox( wnd, "ERROR UPDATE  EXECUTE MODE DIFFERENT SERVER PATCH LIST FILE.", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_MAKELIST_PARSE:
		MessageBox( wnd, "ERROR UPDATE  PARSE SERVER PATCH LIST FILE.", GetWindowName( ), MB_OK );
		SetStatus( wnd, "Error Update" );
		break;
	case _UMT_UPDATE_MAKELIST_END_APP:
	case _UMT_UPDATE_COMPLETE_:
		break;
	default:
		break;
	}

	PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	return 0L;
}

/*-- LoaderProgress Method
*/
DWORD Launcher::LoaderProgress(DWORD totalFileSize, DWORD totalBytesTransferred, void* data)
{
	if ( m_wnd != NULL && m_endApp == false )
	{
		int percent = (int)((float)totalBytesTransferred / (float)totalFileSize * 100.f);
		SendMessage( GetDlgItem( m_wnd, IDC_PROGRESS ), PBM_SETPOS, percent, 0 );
	}
	return (m_endApp != true) ? LPR_CONTINUE : LPR_STOP;
}

/*-- LoaderProgressRoutine Method
*/
DWORD Launcher::LoaderProgressRoutine(DWORD totalFileSize, DWORD totalBytesTransferred, void* data)
{
	Launcher* launcher = (Launcher*)data;
	return launcher->LoaderProgress( totalFileSize, totalBytesTransferred, data );
}

/*-- MakeList Method
*/
eMAKELIST_RETURN_VALUE Launcher::MakeList(char* buffer)
{
	// 메모리 토큰.
	char  seps[] = "\r\n/;";
	char* token     = NULL;
	char* nextToken = NULL;

	// Header
	token = strtok_s( (char*)buffer, seps, &nextToken );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	if ( strcmp( token, "patchlist" ) )
		return eERROR_MAKELIST_WRONG_HEADER;

	token = strtok_s( NULL, seps, &nextToken );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	if ( strcmp( token, "ver.110" ) )
		return eERROR_MAKELIST_WRONG_HEADER;

	token = strtok_s( NULL, seps, &nextToken );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	m_packlist.offset  = 0;
	m_packlist.length  = atoi( token );
	m_packlist.pack    = (Packfile*)GlobalAlloc( GPTR, sizeof(Packfile) * m_packlist.length );

	token = strtok_s( NULL, seps, &nextToken );
	if ( token == NULL )
		return eERROR_MAKELIST_WRONG_HEADER;

	m_patchlist.offset = 0;
	m_patchlist.length = atoi( token );
	m_patchlist.file   = (Patchfile*)GlobalAlloc( GPTR, sizeof(Patchfile) * m_patchlist.length );

	// Body
	Packfile* packfile = m_packlist.pack;
	for ( ULONG i = 0; i < m_packlist.length; i++, packfile++ )
	{
		// 파일이름
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		strcpy_s( packfile->directory, token );
	}

	Patchfile* patchfile = m_patchlist.file;
	for ( ULONG i = 0; i < m_patchlist.length; i++, patchfile++ )
	{
		// 파일이름
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		strcpy_s( patchfile->fileName, token );

		// 파일크기-Low
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->sizeLow = (DWORD)atof( token );

		// 파일크기-High
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->sizeHigh = (DWORD)atof( token );

		// 파일날짜-Low
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->writeTime.dwLowDateTime = (DWORD)atof( token );

		// 파일날짜-High
		if ( (token = strtok_s( NULL, seps, &nextToken )) == NULL )
			return eERROR_MAKELIST_PARSE;
		patchfile->writeTime.dwHighDateTime = (DWORD)atof( token );
	}
	return eMAKELIST_SUCCESS;
}

/*-- VerifyFile Method
*/
bool Launcher::VerifyFile(Patchfile* patchfile)
{
	/// 프로그램이 종료되었다면 빠른 종료를 위해 임시로 true 리턴.
	if( m_endApp == true )
		return true;

	char filename[ 1024 ];
	sprintf_s( filename, "%s\\%s", m_currentDirectory, patchfile->fileName );

	m_endIO = false;

	HANDLE file = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL );
	if ( file != INVALID_HANDLE_VALUE && m_endApp == false )
	{
		bool retcode = true;

		DWORD sizeHigh;
		DWORD sizeLow = GetFileSize( file, &sizeHigh );

		FILETIME write;
		GetFileTime( file, NULL, NULL, &write );

 		FileTimeToDOS( &write );
 
 		FileTimeToDOS( &patchfile->writeTime );

		if ( sizeLow != patchfile->sizeLow || sizeHigh != patchfile->sizeHigh )
			retcode = false;

		if ( CompareFileTime( &write, &patchfile->writeTime ) != 0 )
			retcode = false;

		CloseHandle( file );
		m_endIO = true;

// #ifdef _DEVSYS
// 		if( m_DevLauncher && retcode == false )
// 		{
// 			m_DevLauncher->EventWrite( "CompareFile[%s] Size: old[%d,%d], new[%d,%d] Time: old[%u,%u], new[%u,%u] \n", patchfile->fileName
// 																													 , sizeHigh, sizeLow, patchfile->sizeHigh, patchfile->sizeLow
// 																													 , write.dwHighDateTime, write.dwLowDateTime
// 																													 , patchfile->writeTime.dwHighDateTime, patchfile->writeTime.dwLowDateTime);
// 		}
// #endif

		return retcode;
	}
	else
	{
		CloseHandle( file );
		m_endIO = true;
		return false;
	}

	return false;
}

/*-- UpdateFile Method
*/
bool Launcher::UpdateFile(Patchfile* patchfile, char* buffer, DWORD bufferLen)
{
	/// 파일 경로 얻기
	char drive    [ _MAX_DRIVE ] = {0,};
	char directory[ _MAX_DIR   ] = {0,};
	char makeDir  [ 1024       ] = {0,};
	char filename [ 1024       ] = {0,};

	sprintf_s( filename, "%s\\%s", m_currentDirectory, patchfile->fileName );
	_splitpath_s( filename, drive, _MAX_DRIVE, directory, _MAX_DIR, NULL, 0, NULL, 0 );
	memset( makeDir, 0, sizeof(makeDir) );

	strcat_s( makeDir, drive );
	strcat_s( makeDir, "/" );

	/// 폴더 생성
	char  seps[]	= "/\\";
	char* nextToken = NULL;
	char* token		= strtok_s( directory, seps, &nextToken );
	while ( token != NULL )
	{
		strcat_s( makeDir, token );
		strcat_s( makeDir, "/" );
		
		if ( GetFileAttributes( makeDir ) == 0xFFFFFFFF )
		{
			if ( CreateDirectory( makeDir, NULL ) == FALSE )
				return false;
		}
		token = strtok_s( NULL, seps, &nextToken );
	}

	bool retcode	= false;
	m_endIO			= false;

	/// 임시파일 생성(.tmp)
	char tempname[ 1024 ] = {0,};
	sprintf_s( tempname, "%s.tmp", filename );

	HANDLE file = CreateFile( tempname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file != INVALID_HANDLE_VALUE )
	{
		DWORD  written;
		WriteFile( file, buffer, bufferLen, &written, NULL );

		CloseHandle( file );
		m_endIO = true;

		if ( written == bufferLen )
		{
			m_endIO = false;
			gzFile gz = gzopen( tempname, "rb" );
			if ( gz != NULL )
			{
				m_endIO = false;
				file = CreateFile( filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if ( file != INVALID_HANDLE_VALUE )
				{
					char  buf[16384];			// CHUNK 16KByte
					DWORD len = sizeof(buf);

					DWORD bytesRead;
					DWORD bytesWritten;
					while ( true )
					{
						if( m_endApp == true )
						{
							gzclose( gz );
							CloseHandle( file );
							m_endIO = true;
							DeleteFile( tempname );

							return false;
						}

						bytesRead = gzread( gz, buf, len );
						if ( bytesRead > 0 )
						{
							char  reverse_buf[16384];
							char* pbuf = buf + bytesRead - 1;
							char* prev = reverse_buf;
							for( DWORD cnt = 0; cnt < bytesRead; ++cnt )
							{
								*prev = *pbuf;
								pbuf--;
								prev++;
							}

							WriteFile( file, reverse_buf, bytesRead, &bytesWritten, NULL );
						}
						else
							break;
					}

					SetFileTime( file, NULL, NULL, &patchfile->writeTime );

					DWORD    sizeHigh;
					DWORD    sizeLow;
					sizeLow = GetFileSize( file, &sizeHigh );

					CloseHandle( file );
					m_endIO = true;

					retcode = patchfile->sizeLow == sizeLow && patchfile->sizeHigh == sizeHigh;

// #ifdef _DEVSYS
// 					if( m_DevLauncher && retcode == true )
// 					{
// 						m_DevLauncher->EventWrite( "UpdateFile[%s] Size: old[%d,%d], new[%d,%d] Time: new[%u,%u] \n", patchfile->fileName
// 																													, sizeHigh, sizeLow, patchfile->sizeHigh, patchfile->sizeLow
// 																													, patchfile->writeTime.dwHighDateTime, patchfile->writeTime.dwLowDateTime);
// 					}
// #endif

				}

				gzclose( gz );
				m_endIO = true;
			} /// gzopen
		}

		DeleteFile( tempname );
	}

	return retcode;
}

/*-- WorkerThread Method
*/
DWORD Launcher::WorkerThread( )
{
	m_libClassDesc = new Loader;

	if( m_libClassDesc == NULL )
	{
		PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_LOADER_INIT_, (LPARAM)0 );
		return _UMT_UPDATE_LOADER_INIT_;
	}

	char	hostaddr[ 1024 ]	= {0,};
	char	hosturl [ 1024 ]	= {0,};
	char	hostlist[ 1024 ]	= {0,};
	unsigned int devPortNum = 80; 

#ifdef _DEVSYS
	strcpy_s( hostaddr, m_DevLauncher->GetHost() );
	strcpy_s( hosturl,	m_DevLauncher->GetAddr() );

	devPortNum = m_DevLauncher->GetPort();
	GetString( IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );

#elif _LOCAL

	char CLIENT_TEST_HOST[] = { 104, -110,  -33, -66,  99, -114,  -40, -10, 100, -120,  -41, -11,  99,  -97, -112, -13,  98, -59, -43, -30,   0 };
	char CLIENT_TEST_ADDR[] = { 104, -110,  -33, -65, 100, -103,  -41, -29,  34, -101,  -33, -28, 110, -125, -111, -64, 108, -97, -35,  -8, 104, -103, 0 };
	//char GMTOOL_TEST_ADDR[] = {  78, -121,  -41, -11,  99,  -97,   -7, -35,   0	};

	BYTE    key[]   = { 0x0d, 0xeb, 0xbe, 0x90 };
	size_t  strLen  = 0;

	strLen = ::strlen( CLIENT_TEST_HOST );
	for( size_t index = 0; index < strLen; ++index )
		CLIENT_TEST_HOST[index] ^= key[index%4];

	strLen = ::strlen( CLIENT_TEST_ADDR );
	for( size_t index = 0; index < strLen; ++index )
		CLIENT_TEST_ADDR[index] ^= key[index%4];

	strcpy_s( hostaddr, CLIENT_TEST_HOST );
	strcpy_s( hosturl,	CLIENT_TEST_ADDR );

	GetString( IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );
	devPortNum = 80;

#elif _TEST_SRV
	GetString( IDS_UPDATE_HOST_TEST, hostaddr, sizeof(hostaddr) );
	GetString( IDS_UPDATE_URL_TEST,  hosturl,  sizeof(hosturl)  );
	GetString( IDS_UPDATE_LIST, hostlist, sizeof(hostlist) );
#else
	GetString( IDS_UPDATE_HOST_IDC, hostaddr, sizeof(hostaddr) );
	GetString( IDS_UPDATE_URL_IDC,  hosturl,  sizeof(hosturl)  );
	GetString( IDS_UPDATE_LIST,     hostlist, sizeof(hostlist) );
	
#endif 

	/// 런쳐가 너무 빨리 꺼지는것을 방지
	Sleep( 500 );
 
 	if( m_endApp == true )
 		return 0;

	SetStatus( m_wnd, "Start" );

	/// 연결
	if (  m_libClassDesc->Open( hostaddr, devPortNum ) != 0 )
	{
		PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_LOADER_OPEN_, (LPARAM)0 );
		return _UMT_UPDATE_LOADER_OPEN_;
	}

	char  url[ 1024 ]	= {0,};
	char* buffer		= NULL;
	DWORD bufferLen		= 0;

	DWORD retValue;

	/// 패치리스트 목록 생성.
	sprintf_s( url, "%s/%s", hosturl, hostlist );
	m_DownLoadExitCode = LPR_CONTINUE;
	retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &m_DownLoadExitCode, this, &buffer, &bufferLen );
    
	try
	{
		/// 재접속 요청하여 다시 다운 ..
		if( retValue != 0 ) //ERROR_INTERNET_TIMEOUT || retValue == ERROR_INTERNET_CONNECTION_RESET )
		{
			if( retValue == ERROR_INTERNET_INVALID_URL )
			{
				PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_NOT_EXIST_PATCHLIST_, (LPARAM)0 );
				return 0;
			}

			int retryConnetion = 0;
			do 
			{
				/// 연결
				if( m_libClassDesc->Open( hostaddr, devPortNum ) ) 
				{
					PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_LOADER_OPEN_, (LPARAM)0 );
					return _UMT_UPDATE_LOADER_OPEN_;;
				}

				/// 데이타 받기
				m_DownLoadExitCode = LPR_CONTINUE;
				retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &m_DownLoadExitCode, this, &buffer, &bufferLen );

				if( ++retryConnetion > 20 )
					throw( retValue );

// 				if( retValue != 0 )
// 				{
// 					if( retValue != ERROR_INTERNET_CONNECTION_RESET && retValue != ERROR_INTERNET_TIMEOUT )
// 						throw( 0 );
// 				}

				if( m_endApp == true )
				{
					if( buffer )
						GlobalFree( buffer );

					m_endIO = true;
					return 0;
				}
			} while( retValue != 0 );
		}

		/// 파일 저장 후 patchlist/packlist 데이타 추가
		if ( buffer != NULL && bufferLen > 0 && m_endApp == false && retValue == 0 )
		{
			if( UnpackPatchList( &buffer, &bufferLen ) == false ) 
			{
				PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_PATCHLIST_UNPACK_, (LPARAM)0 );
				return _UMT_UPDATE_PATCHLIST_UNPACK_;
			}

			eMAKELIST_RETURN_VALUE retMakeList;
			retMakeList = MakeList( buffer );
			if ( retMakeList != eMAKELIST_SUCCESS )
			{
				GlobalFree( buffer );

				int converter = 0;
				switch( retMakeList )
				{
				case eERROR_MAKELIST_WRONG_HEADER:  converter = _UMT_UPDATE_MAKELIST_WRONG_HEADER;   break;
				case eERROR_MAKELIST_WRONG_EXECUTE: converter = _UMT_UPDATE_MAKELIST_WRONG_EXECUTE;  break;
				case eERROR_MAKELIST_DIFF_PATCHLIST:converter = _UMT_UPDATE_MAKELIST_DIFF_PATCHLIST; break;
				case eERROR_MAKELIST_PARSE:         converter = _UMT_UPDATE_MAKELIST_PARSE;          break;
				case eERROR_MAKELIST_END_APP:       converter = _UMT_UPDATE_MAKELIST_END_APP;		break;
				default: converter = _UMT_UPDATE_PATCHLIST_PARSING_; break;
				}

				PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)converter, (LPARAM)0 );

				return _UMT_UPDATE_PATCHLIST_PARSING_;
			}
			GlobalFree( buffer );
		}
		else 
			throw( 0 );

	} 
	catch ( ... )
	{
		if( buffer )
			GlobalFree( buffer );

		PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_PATCHLIST_DOWN_, (LPARAM)0 );
		return _UMT_UPDATE_PATCHLIST_DOWN_;
	}

	/// 실제 패치 파일들 다운
	Patchfile* files = m_patchlist.file;
	for ( ULONG i = 0; i < m_patchlist.length && m_endApp == false ; i++, files++ )
	{
		if ( VerifyFile( files ) == false )
		{
			sprintf_s( url, "%s/%s.z", hosturl, files->fileName );
			m_DownLoadExitCode = LPR_CONTINUE;
			retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &m_DownLoadExitCode, this, &buffer, &bufferLen );
			
			try
			{
				/// 연결 재시도
				if( retValue != 0 )//== ERROR_INTERNET_TIMEOUT || retValue == ERROR_INTERNET_CONNECTION_RESET )
				{

					if( retValue == ERROR_INTERNET_INVALID_URL )
						throw 0;

					int retryConnetion = 0;

					do 
					{
						/// 재 접속
						if( m_libClassDesc->Open( hostaddr, devPortNum ) ) 
						{
							ErrorMessage( WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_PATCHFILE_DOWN_, (LPARAM)files->fileName );
							return _UMT_UPDATE_PATCHFILE_DOWN_;
						}

						m_DownLoadExitCode = LPR_CONTINUE;
						retValue = m_libClassDesc->Get( url, &LoaderProgressRoutine, &m_DownLoadExitCode, this, &buffer, &bufferLen );

						if( ++retryConnetion > 20 )
							throw(retValue);

// 						if( retValue != 0 )
// 						{
// 							if( retValue != ERROR_INTERNET_CONNECTION_RESET && retValue != ERROR_INTERNET_TIMEOUT )
// 								throw(0);
// 						}

						if( m_endApp == true )
						{
							if( buffer )
								GlobalFree( buffer );

							m_endIO = true;
							return 0;
						}

					} while( retValue != 0 );
				}

				/// 패치 파일 저장
				if ( buffer != NULL && bufferLen > 0 && m_endApp == false && retValue == 0 )
				{
					if ( UpdateFile( files, buffer, bufferLen ) == false )
					{
						if( buffer )
							GlobalFree( buffer );

						ErrorMessage( WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_FILE_, (LPARAM)files->fileName );
						return _UMT_UPDATE_FILE_;
					}	
				}
				else 
					throw( 0 );

			} 
			catch ( ... )	
			{
				if( buffer )
					GlobalFree( buffer );

				ErrorMessage( WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_PATCHFILE_DOWN_, (LPARAM)files->fileName );
				return _UMT_UPDATE_PATCHFILE_DOWN_;
			}

			if( buffer )
				GlobalFree( buffer );
		}
	} /// for

	m_libClassDesc->Close( );
	SetStatus( m_wnd, "End" );

	Sleep( 500 );

	// 업데이트 완료.
	if( m_endApp == false )
	{
#ifdef _SERVER
		if( /*m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_SERVER_DEV || */
            m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_SERVER_IDC ||
            m_DevLauncher->GetLauncherMode() == eLAUNCHER_MODE_SERVER_TEST )
        {
			MessageBox( m_wnd, "Update Complete", "End", MB_OK );
        }
		else
#endif
        {
		    m_runPatcher = true;
        }

		PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_COMPLETE_, (LPARAM)0 );
	}

	return 0L;
}

bool Launcher::UnpackPatchList( char** buffer, DWORD *bufferLen )
{
	/// "patchlist.txt.z" String
	char packName[1024] = {0,};
	GetString( IDS_UPDATE_LIST, packName, sizeof(packName) );


	/// patchlist.txt.z 파일 생성
	DWORD  written;
	BOOL   ret  = FALSE;
	m_endIO		= false;

	HANDLE file = CreateFile( packName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( file != INVALID_HANDLE_VALUE )
	{
		ret = WriteFile( file, (*buffer), (*bufferLen), &written, NULL );
		CloseHandle( file );
		GlobalFree( (*buffer) );

		m_endIO = true;
	}
	else if ( ret == 0 || m_endApp == true )
	{
		PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_SAVE_FILE_, (LPARAM)0 );
		CloseHandle( file );
		GlobalFree( (*buffer) );

		m_endIO = true;
		return false;
	}

	/// patchlist.txt.z 파일 데이타 저장
	if ( written == (*bufferLen) )
	{
		m_endIO = false;

		gzFile gz = gzopen( packName, "rb" );
		if ( gz != NULL )
		{
			char  buf[16384];			// CHUNK 16KByte
			DWORD bytesRead		= 1;
			DWORD totalBytes	= 0;
			DWORD len			= sizeof(buf);

			/// 파일 크기 얻기
			while ( bytesRead > 0 )
			{
				bytesRead	= gzread( gz, buf, len );
				totalBytes	+= bytesRead;
			}

			/// 메모리 할당
			(*bufferLen)	= totalBytes;
			(*buffer)		=  (char*)GlobalAlloc( GPTR, (*bufferLen)+1 );
			char *ptr = (*buffer);

			if ( (*buffer) == NULL || m_endApp == true )
			{
				PostMessage( m_wnd, WM_UPDATE_MSG, (WPARAM)_UMT_UPDATE_ALLOCATION_, (LPARAM)0 );
				gzclose( gz );
				GlobalFree( (*buffer) );

				m_endIO = true;
				return false;
			}

			gzrewind(gz);

			bytesRead = gzread( gz, (*buffer), (*bufferLen) );
			if( bytesRead != (*bufferLen) )
			{
				gzclose( gz );
				GlobalFree( (*buffer) );

				m_endIO = true;
				return false;
			}

			m_endIO = true;
			gzclose( gz );
			DeleteFile( packName );
		}
		return true;
	}

	return false;
}

void Launcher::ErrorMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEVSYS
	MessageBox( NULL, (char*)lParam, GetWindowName( ), MB_OK );
	PostMessage( m_wnd, WM_CLOSE, 0, 0 );

#else
	PostMessage( m_wnd, Msg, wParam, lParam );

#endif
}

LRESULT Launcher::RunLauncher()
{
	// 응용 프로그램, 다이얼로그 표시
	ShowWindow( m_wnd, TRUE );

	// 업데이트 시작.
	unsigned int threadId;
	m_workerThread = (HANDLE)_beginthreadex( NULL, 0, WorkerThreadStartingPoint, 0, 0, &threadId );
//	m_workerThread = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	if ( m_workerThread == NULL )
	{
		DWORD lastError = GetLastError( );
		char  buffer[MAX_PATH];

		sprintf_s( buffer, "Error(=0x%08x)", lastError );
		MessageBox( m_wnd, buffer, GetWindowName( ), MB_OK | MB_ICONERROR );
		PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	}

	return TRUE;
}
