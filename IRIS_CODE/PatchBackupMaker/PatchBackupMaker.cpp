/*-- Include
*/
#include "PatchBackupMaker.h"
#include <stdio.h>
#include <time.h>
#include <io.h>
/*-- Local definitions
*/
#define SafeDelete(P)	{ if (P!=NULL) { delete(P); (P)=NULL; } }       // 메모리를 안전하게 해제.

#define WM_PATCH_MSG	WM_USER+0x10
#define PM_COMPLETE		  0
#define PM_ERROR_XCOPY    1
#define PM_ERROR          2
#define PM_ERROR_STRLOWER 3

/*-- Global data
*/
HINSTANCE   g_instance   = NULL;
PatchBackupMaker* g_patchmaker = NULL;

DWORD CALLBACK CopyProgressRoutine( LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
								   LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber,
								   DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData)
{
	g_patchmaker->Progress( TotalFileSize.LowPart, TotalBytesTransferred.LowPart );
	return 0;
}

/*-- WinMain Prototype
*/
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	// Initialize COM Library - INTERNET API(wininet.h/wininet.lib)를 사용하기 위해 선언.
	if ( CoInitialize( NULL ) != S_OK )
	{
		MessageBox( NULL, "CoInitialize Failed!\r\n", "PatchBackupMaker", MB_OK );
		return 0L;
	}

	PatchBackupMaker* patchMaker = new PatchBackupMaker( );
	HANDLE      handle     = CreateMutex( NULL, FALSE, patchMaker->GetWindowName( ) );
	int         result     = GetLastError( );

	g_instance = instance;
	if ( handle != NULL )
	{
		if ( result != ERROR_ALREADY_EXISTS )
			result = patchMaker->Run( instance, prevInstance, cmdLine, cmdShow );
		CloseHandle( handle );
	}

	SafeDelete( patchMaker );

	// Finished with COM Library
	CoUninitialize( );
	return result;
}

/*-- DlgProc Prototype
*/
LRESULT CALLBACK DlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return g_patchmaker->MsgProc( wnd, msg, wParam, lParam );
}

/*-- WorkerThreadStartingPoint Prototype
*/
DWORD WINAPI WorkerThreadStartingPoint(void* ptr)
{
	return g_patchmaker->WorkerThread( );
}

/*-- PatchBackupMaker Constructor
*/
PatchBackupMaker::PatchBackupMaker(void)
{
	g_patchmaker   = this;
	m_wnd          = NULL;
	m_icon         = NULL;
	m_windowName   = TEXT( "BackupMaker - IRIS ON LINE" );
	m_endApp       = false;
	m_isCopyMode   = false;  

	m_packlist.pack    = NULL;
	m_packlist.offset  = 0;
	m_packlist.length  = 0;

	m_patchlist.file   = NULL;
	m_patchlist.offset = 0;
	m_patchlist.length = 0;

	m_workerThread = NULL;
}

/*-- ~PatchBackupMaker Destructor
*/
PatchBackupMaker::~PatchBackupMaker(void)
{
	g_patchmaker = NULL;
}

/*-- EventWrite Method
*/
void PatchBackupMaker::EventWrite(LPCTSTR format, ...)
{
	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;
	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) +1;
	msgBuf       = malloc( bufferLength );
	vsprintf_s( (char*)msgBuf, bufferLength, format, args );

	va_end( args );

	time_t t;
	tm    pt;
	char time_array[ 1024 ];
	errno_t err;

	time(&t);
	err = localtime_s( &pt, &t );

	sprintf_s( time_array, "%d/%d/%d %d:%d:%d \0",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, pt.tm_hour,pt.tm_min,pt.tm_sec );

	if ( msgBuf != NULL )
	{
		char filename[1024];
		sprintf_s( filename, "%s\\%s.log", m_currentDirectory, GetWindowName( ) );

		FILE* stream;
		err = fopen_s( &stream, filename, "at" );
		if ( err == 0 )
		{
			fputs( time_array   , stream );
			fputs( (char*)msgBuf, stream );
			fclose( stream );
			stream = NULL;
		}

		free( msgBuf );
	}
}

/*-- GetFileTimeUTC Method
*/
bool PatchBackupMaker::GetFileTimeUTC(char* filename, SYSTEMTIME* utc)
{
	HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( file != INVALID_HANDLE_VALUE ) 
	{
		FILETIME write;
		GetFileTime( file, NULL, NULL, &write );
		FileTimeToSystemTime( &write, utc );
		CloseHandle( file );
		return true;
	}
	return false;
}

/*-- SetFileTimeUTC Method
*/
bool PatchBackupMaker::SetFileTimeUTC(char* filename, SYSTEMTIME* utc)
{
	HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( file != INVALID_HANDLE_VALUE ) 
	{
		FILETIME write;
		SystemTimeToFileTime( utc, &write );
		SetFileTime( file, NULL, NULL, &write );
		CloseHandle( file );
		return true;
	}
	return false;
}

/*-- Run Method
*/
int PatchBackupMaker::Run(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	// 현재 폴더 저장.
	GetCurrentDirectory( sizeof(m_currentDirectory), m_currentDirectory );

	// 응용 프로그램, 다이얼로그 생성
	m_wnd = CreateDialog( instance, MAKEINTRESOURCE(IDD_PATCHMAKER), NULL, (DLGPROC)DlgProc );
	if ( m_wnd == NULL )
		return GetLastError( );

	// 윈도우 이름 입력
	SetWindowText( m_wnd, GetWindowName( ) );

	/*-- Command Line 처리.
	*/
	if ( CommandLine( cmdLine ) == true )
		StartPatch( );

	// 응용 프로그램, 다이얼로그 표시
	ShowWindow( m_wnd, cmdShow );

	// 메시지 루프 시작
	MSG  msg;
	BOOL result;

	while ( (result = GetMessage( &msg, NULL, 0, 0 )) )
	{
		if ( result == -1 )
		{
			// handle the error and possibly exit
			return GetLastError( );
		}
		else if ( !IsWindow( m_wnd ) || !IsDialogMessage( m_wnd, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	return (int)msg.wParam;
}

/*-- MsgProc Method
*/
LRESULT PatchBackupMaker::MsgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
	case WM_DESTROY:    return Destroy    ( wnd, wParam, lParam );
	case WM_INITDIALOG: return InitDialog ( wnd, wParam, lParam );
	case WM_COMMAND:    return Command    ( wnd, wParam, lParam );
	case WM_PATCH_MSG:  return PatchMsg   ( wnd, wParam, lParam );
	}
	return 0L;
}

/*-- CommandLine Method
*/
bool PatchBackupMaker::CommandLine(LPSTR cmdLine)
{
	if ( strlen( cmdLine ) > 0 )
	{
		char  mode[ 1024 ] = { 0, };
		char  path[ 1024 ] = "\0";
		char  seps[] = " ";
		bool  patch  = true;

		char* token     = NULL;
		char* nextToken = NULL;
		DWORD retcode;

		token = strtok_s( cmdLine, seps, &nextToken );
		if ( token != NULL )
			strcpy_s( m_backupDirectory, token );
		else
			memset( m_backupDirectory, 0, sizeof(m_backupDirectory) );

		retcode = GetFileAttributes( m_backupDirectory );
		if ( (retcode != INVALID_FILE_ATTRIBUTES) && (retcode & FILE_ATTRIBUTE_DIRECTORY) )
			SetWindowText( GetDlgItem( m_wnd, IDC_SOURCE_DIR ), m_backupDirectory );
		else
			patch = false;
		
		token = strtok_s( NULL, seps, &nextToken );
		if ( token != NULL )
			strcpy_s( m_pdbDirectory, token );
		else
			memset( m_pdbDirectory, 0, sizeof( m_pdbDirectory ) );

		retcode = GetFileAttributes( m_pdbDirectory );
		if ( (retcode != INVALID_FILE_ATTRIBUTES) && (retcode & FILE_ATTRIBUTE_DIRECTORY) )
			SetWindowText( GetDlgItem( m_wnd, IDC_PDB_DIR ), m_pdbDirectory );
		else
			patch = false;

        return patch;
	}
	return false;
}

/*-- SetStatus Method
*/
BOOL PatchBackupMaker::SetStatus(HWND wnd, char* msg)
{
	HWND dlgItem = GetDlgItem( wnd, IDC_STATUS );
	BOOL retcode = SetWindowText( dlgItem, msg );
	UpdateWindow( dlgItem );
	return retcode;
}

/*-- Progress Method
*/
void PatchBackupMaker::Progress(DWORD length, DWORD offset)
{
	if ( m_wnd != NULL )
	{
		int percent = (int)((float)offset / (float)length * 100.f);
		SendMessage( GetDlgItem( m_wnd, IDC_PROGRESS ), PBM_SETPOS, percent, 0 );
	}
}

/*-- Destroy Method
*/
LRESULT PatchBackupMaker::Destroy(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	// 안전종료.
	m_endApp = true;

	// m_workerThread 핸들를 해제.
	if ( m_workerThread != NULL )
	{
		// WorkerThread가 종료될때까지 대기.
		if ( WaitForSingleObject( m_workerThread, 5000 ) != WAIT_OBJECT_0 )
		{
			DWORD exitCode;
			GetExitCodeThread( m_workerThread, &exitCode );
			if ( exitCode == STILL_ACTIVE )
			{
				TerminateThread( m_workerThread, 0 );
			}
		}
		CloseHandle( m_workerThread );
		m_workerThread = NULL;
	}

	// ICON 해제
	if ( m_icon != NULL )
	{
		DestroyIcon( m_icon );
		m_icon = NULL;
	}

	// 윈도우 탐색기로 부터 Drag & Drop 종료.
	DragAcceptFiles( wnd, FALSE );

	// 응용 프로그램을 종료 한다.
	PostQuitMessage( 0 );
	return 0L;
}

/*-- InitDialog Method
*/
LRESULT PatchBackupMaker::InitDialog(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	// 윈도우 탐색기로 부터 Drag & Drop 시작.
	DragAcceptFiles( wnd, TRUE );

	// ICON 로드
	m_icon = LoadIcon( g_instance, MAKEINTRESOURCE( IDI_ICON ) );
	SendMessage( wnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)m_icon );

	// 상태.
	SetStatus( wnd, "Ready" );
	return 1L;
}

/*-- Command Method
*/
LRESULT PatchBackupMaker::Command(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	switch ( LOWORD(wParam) )
	{
	case IDOK:
		//DestroyWindow( wnd );
		break;
	case IDCANCEL:
		DestroyWindow( wnd );
		break;
	}
	return 0L;
}

/*-- PatchMsg Method
*/
LRESULT PatchBackupMaker::PatchMsg(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case PM_COMPLETE:
		MessageBox( wnd, "Success", GetWindowName( ), MB_OK );
		break;
	case PM_ERROR_XCOPY:
		MessageBox( wnd, "Error Xcopy", GetWindowName( ), MB_OK );
		break;
	case PM_ERROR:
		MessageBox( wnd, "Error", GetWindowName( ), MB_OK );
		break;
	case PM_ERROR_STRLOWER:
		MessageBox( wnd, "Error STRLOWER", GetWindowName( ), MB_OK );
		break;
	default:
		MessageBox( wnd, "Unkonwn", GetWindowName( ), MB_OK );
		break;
	}
	PostMessage( m_wnd, WM_CLOSE, 0, 0 );
	return 0L;
}


/*-- StartPatch Method
*/
bool PatchBackupMaker::StartPatch( )
{
	DWORD threadId;
	m_workerThread = CreateThread( NULL, 0, WorkerThreadStartingPoint, (LPVOID)this, 0, &threadId );
	return (m_workerThread != NULL) ? true : false;
	
}

/*-- WorkerThread Method
*/
DWORD PatchBackupMaker::WorkerThread( )
{
	bool success = false;

	SetStatus( m_wnd, "Start Xcopy..." );

	if( CreateTodayBackupDirectory() == false )
	{
		ErrorPrint( "today backup directory create fail" );
		return 0;
	}

	/// pdb file backup
	HANDLE          search  = NULL;
	WIN32_FIND_DATA fileData;
	char searchDirectory[ 1024 ] = { 0, };
	sprintf_s( searchDirectory, "%s\\*.*", m_pdbDirectory );

	search = FindFirstFile( searchDirectory, &fileData );
	if ( search != INVALID_HANDLE_VALUE )
	{
		do {
			if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
			{
				if( strstr( fileData.cFileName, ".pdb" ) != NULL || strstr( fileData.cFileName, ".exe" ) != NULL )
				{
					char pdbFile[ 1024 ] = { 0, };
					char backup [ 1024 ] = { 0, };

					sprintf_s( pdbFile, "%s\\%s", m_pdbDirectory,    fileData.cFileName );
					sprintf_s( backup,  "%s\\%s", m_backupDirectory, fileData.cFileName );
					SYSTEMTIME utc;

					// CopyFile에서 확장. [12/11/2008 Jo]
					if ( CopyFileEx( pdbFile, backup, CopyProgressRoutine, NULL, (LPBOOL)m_endApp, 0 ) == TRUE ) 
					{
						GetFileTimeUTC( pdbFile, &utc );
						SetFileTimeUTC( backup,  &utc );
					}
				}
			}
		} while ( FindNextFile( search, &fileData ) );

		FindClose( search );
		search = NULL;
	}
	else
	{
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)PM_ERROR_XCOPY, (LPARAM)0 );
		SetStatus( m_wnd, "Error Xcopy..." );
		return 0;
	}
	
	success = true;

	SetStatus( m_wnd, "End Xcopy..." );
	
	if ( success )
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)PM_COMPLETE, (LPARAM)0 );
	else
		PostMessage( m_wnd, WM_PATCH_MSG, (WPARAM)PM_ERROR, (LPARAM)0 );

	return 0;
}

void PatchBackupMaker::ErrorPrint(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL);
	MessageBox(m_wnd, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

bool PatchBackupMaker::CreateTodayBackupDirectory()
{
	char todayDirectory[ 1024 ] = { 0, };
	char todaySubDirectory[ 1024 ] = { 0, };
	int  todayCounter = 0;

	time_t t;
	tm    pt;
	errno_t err;

	time(&t);
	err = localtime_s( &pt, &t );
	
	sprintf_s( todaySubDirectory, "%04d-%02d-%02d [%02d]_PDB_EXE_",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, todayCounter );
	sprintf_s( todayDirectory, "%s\\%s", m_backupDirectory, todaySubDirectory );

	while( _access( todayDirectory, 0 ) == 0 )
	{
		++todayCounter;
		sprintf_s( todaySubDirectory, "%04d-%02d-%02d [%02d]_PDB_EXE_\0",	pt.tm_year+1900, pt.tm_mon+1, pt.tm_mday, todayCounter );
		sprintf_s( todayDirectory, "%s\\%s\0", m_backupDirectory, todaySubDirectory );
	}

	if ( CreateDirectory( todayDirectory, NULL ) == FALSE )
		if ( GetLastError( ) != ERROR_ALREADY_EXISTS )
		{
			ErrorPrint( "today backup directory create fail" );
			return false;
		}

	strcpy_s( m_backupDirectory, todayDirectory );

	return true;
}