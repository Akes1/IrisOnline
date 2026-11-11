// Include
#include "logdemon.h"

// Local definitions
// memory를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; }
// handle를 안전하게 해제
#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; }

#define ODBC_DSN_LOG		"IRIS_LOGDB"

#define ODBC_UID			"iris"				// IDC test
#define ODBC_PWD			"vedsa239HPJam8Y"		// IDC test

// Global data
cLogDemon* g_logDemon  = NULL;
bool       g_verbose   = false;
bool       g_fps       = false;
bool       g_packet    = false;
bool       g_udp       = false;
bool       g_udpPacket = false;
bool       g_udpFrom   = false;

u_short    g_loginPort = 15002;
u_short    g_gamePort  = 15004;
u_short    g_logPort   = 15000;

// 서비스 이름 및 표시이름 설정 - 기본값.
TCHAR g_serviceName[MAX_PATH] = TEXT( "LogDemon" );
TCHAR g_description[MAX_PATH] = TEXT( "Copyright(c) 2009 EYA INTERACTIVE Corporation All rights reserved" );
TCHAR g_displayFmt[]          = TEXT( "IRIS ON LINE - %s" );

// GenerateDump Prototype
//	 1. 속성 < C/C++ < 일반 < 디버깅 정보 형식 = 프로그램 데이터베이스(/Zi)
//	 2. SetUnhandledExceptionFilter( Exception_Minidump );
//	 3. exceptionPointers->ExceptionRecord->ExceptionCode는 winbase.h and winnt.h 파일참조
//	    (EXCEPTION_ACCESS_VIOLATION ... CONTROL_C_EXIT 등)
//
//	 주의: DBGHELP.DLL 파일의 버전은 최소 5.1.2600.0 이상이어야 한다.
//	       (MiniDumpWriteDump 함수가 DBGHELP.DLL에 들어 있음)
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

LONG __stdcall GenerateDump(EXCEPTION_POINTERS* exceptionPointers)
{
	char fileName[MAX_PATH];
	SYSTEMTIME st;

	GetLocalTime( &st );
	sprintf_s( fileName
			  ,MAX_PATH
			  ,"LogDemon_%4d%02d%02d_%02d%02d%02d(0x%08X 0x%08p 0x%02X 0x%04X 0x%08X).dmp"
			  ,st.wYear
			  ,st.wMonth
			  ,st.wDay
			  ,st.wHour
			  ,st.wMinute
			  ,st.wSecond
			  ,exceptionPointers->ExceptionRecord->ExceptionCode
			  ,exceptionPointers->ExceptionRecord->ExceptionAddress
			  ,exceptionPointers->ExceptionRecord->NumberParameters
			  ,exceptionPointers->ExceptionRecord->ExceptionInformation[0]
			  ,exceptionPointers->ExceptionRecord->ExceptionInformation[1] );

	//GetModuleFileName( NULL, fileName, sizeof(fileName) );
	//char* ext = strrchr( fileName, '.' );
	//strcpy( ext ? ext : fileName+strlen( fileName ), ".dmp" );

	HANDLE process   = GetCurrentProcess( );
	DWORD  processID = GetCurrentProcessId( );
	HANDLE file      = CreateFile( fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	MINIDUMP_EXCEPTION_INFORMATION expParam;

	expParam.ThreadId          = GetCurrentThreadId( );
	expParam.ExceptionPointers = exceptionPointers;
	expParam.ClientPointers    = TRUE;

	MiniDumpWriteDump( process, processID, file, MiniDumpWithFullMemory, &expParam, NULL, NULL );

	return EXCEPTION_EXECUTE_HANDLER;
}

// ErrorCode2String Method
//
//	오류 코드를 메시지로 변환한다.
//
//	msgBuf    :오류메시지 (사용후 LocalFree 하여 준다.)
//	lastError :오류코드
//	priLangId :primary language identifier
//	subLangId :sublanguage identifier
//
//	문자메시지 언어는 아래와 같이 설정한다.
//
//	Primary language identifier : Sublanguage identifier : Meaning 
//	LANG_NEUTRAL                  SUBLANG_NEUTRAL          Language neutral  
//	LANG_NEUTRAL                  SUBLANG_DEFAULT          User default language
//	LANG_NEUTRAL                  SUBLANG_SYS_DEFAULT      System default language 
DWORD ErrorCode2String(VOID** msgBuf, DWORD lastError, WORD priLangId, WORD subLangId)
{
	// default to system source
	HMODULE module = NULL;

	DWORD   bufferLength;
	DWORD   flags;

	flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;

	// If lastError is in the network range, load the message source.
	// 만약 lastError가 NERR_BASE 에서 MAX_NERR 일경우, netmsg.dll 의
	// 메시지를 사용한다. (NERR_BASE, MAX_NERR는 lmerr.h 파일에 정의)
	if ( lastError >= NERR_BASE && lastError <= MAX_NERR )
	{
		module = LoadLibraryEx( "netmsg.dll", NULL, LOAD_LIBRARY_AS_DATAFILE );
		if ( module != NULL )
		{
			flags |= FORMAT_MESSAGE_FROM_HMODULE;
		}
	}

	// If lastError is in the Exception Code range, load the message source.
	// 만약 lastError가 0xC0000000L 보다 클경우, ntdll.dll의 메시지를 사용한다.
	// (winbase.h, winnt.h 파일에 정의)
	if ( lastError > 0xC0000000L )
	{
		module = LoadLibraryEx( "ntdll.dll", NULL, LOAD_LIBRARY_AS_DATAFILE );
		if ( module != NULL )
		{
			flags |= FORMAT_MESSAGE_FROM_HMODULE;
		}
	}

	// Call FormatMessage() to allow for message text to be acquired
	// from the system or from the supplied module handle. module to
	// get message from (NULL == system)
	bufferLength = FormatMessage( flags, module, lastError, MAKELANGID(priLangId, subLangId), (LPTSTR)msgBuf, 0, NULL );

	// If we loaded a message source, unload it.
	if ( module != NULL )
	{
		FreeLibrary( module );
	}
	return bufferLength;
}

// ConsolCtrlHandler Prototype - Windows Command Processor 제어용 함수.
BOOL WINAPI ConsolCtrlHandler(DWORD opcode)
{
	if ( g_logDemon )
		return g_logDemon->ConsolCtrlHandler( opcode );
	else
		return FALSE;
}

// main Prototype (This method is the standard Windows program entry point.)
int main(int argc, char* argv[])
{
	LPTOP_LEVEL_EXCEPTION_FILTER previousFilter = NULL;
	previousFilter = SetUnhandledExceptionFilter( GenerateDump );

	cLogDemon* logDemon = NULL;

	logDemon = new cLogDemon( );
	logDemon->ConsoleRun( argc, argv );

	SafeDelete( logDemon );

	SetUnhandledExceptionFilter( previousFilter );
	return 0L;
}

// ServiceCtrlHandler Prototype - Windows Service 제어용 함수.
void WINAPI ServiceCtrlHandler(DWORD opcode)
{
	if ( g_logDemon )
		g_logDemon->ServiceCtrlHandler( opcode );
}

// ServiceMain Prototype (This method is the entry point for a service.)
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	cLogDemon* logDemon = NULL;

	logDemon = new cLogDemon( );
	logDemon->ServiceRun( argc, argv );

	SafeDelete( logDemon );
}

// cLogDemon Constructor.
cLogDemon::cLogDemon(void) : mLogSender(NULL), mSqlLog(NULL), mLogRecver(NULL)
{
	g_logDemon  = this;

	mRunService = false;
	mEndService = false;
	mClose      = false;
	mVersion    = MAKEWORD(1, 0);

	memset( mServerBroadcastSend, 0, sizeof(mServerBroadcastSend) );
	memset( mServerBroadcastRecv, 0, sizeof(mServerBroadcastRecv) );
}

// ~cLogDemon Destructor.
cLogDemon::~cLogDemon(void)
{
	// cLogRecver 클래스 해제.
	SafeDelete( mLogRecver );

	// SQLLog2Log 클래스 해제.
	SafeDelete( mSqlLog );

	// cLogSender 클래스 해제.
	SafeDelete( mLogSender );

	g_logDemon = NULL;
}

// InstallService Method - 서비스 설치 함수.
bool cLogDemon::InstallService(LPCTSTR binaryPathName, DWORD startType)
{
	SC_HANDLE           hManager(NULL);
	SC_HANDLE           hService(NULL);
	bool                retValue(false);

	TCHAR displayName[MAX_PATH];
	sprintf( displayName, g_displayFmt, g_serviceName );

	// Open a handle to the SC Manager database.
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( hManager != NULL )
	{
		// The function creates a service object and adds it to the specified service control manager database.
		hService = CreateService( hManager,                  // handle to SCM database
								  g_serviceName,             // name of service to start
								  displayName,               // display name
								  SERVICE_ALL_ACCESS,        // type of access to service
								  SERVICE_WIN32_OWN_PROCESS, // type of service
								  startType,                 // when to start service
								  SERVICE_ERROR_NORMAL,      // severity of service failure
								  binaryPathName,            // name of binary file
								  NULL,                      // name of load ordering group
								  NULL,                      // tag identifier
								  NULL,                      // array of dependency names
								  NULL,                      // account name
								  NULL );                    // account password
		if ( hService != NULL )
		{
			// Success CreateService
			retValue = true;

			// The function changes the optional configuration parameters of a service.
			SERVICE_DESCRIPTION scDesc;
			scDesc.lpDescription = g_description;
			ChangeServiceConfig2( hService, SERVICE_CONFIG_DESCRIPTION, &scDesc );

			// Close the handle to the service.
			CloseServiceHandle( hService );
		}
		else
		{
			// 오류메시지 출력 - CreateService.
			VOID* msgBuf;
			if ( ErrorCode2String( &msgBuf ) )
			{
				printf( "\nCreateService (%d): %s\n", GetLastError( ), (char*)msgBuf );
				LocalFree( msgBuf );
			}
		}
	}
	return retValue;
}

// UninstallService Method - 서비스 삭제 함수.
bool cLogDemon::UninstallService()
{
	SC_HANDLE hManager(NULL);
	SC_HANDLE hService(NULL);
	bool      retValue(false);

	// Open a handle to the SC Manager database.
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( hManager != NULL )
	{
		// Open a handle to the service.
		hService = OpenService( hManager, g_serviceName, SERVICE_ALL_ACCESS );
		if ( hService != NULL )
		{
			// The function marks the specified service for deletion from the service control manager database.
			if ( DeleteService( hService ) != 0 )
			{
				// Success DeleteService
				retValue = true;
			}
			else
			{
				// 오류메시지 출력 - DeleteService
				VOID* msgBuf;
				if ( ErrorCode2String( &msgBuf ) )
				{
					printf( "\nDeleteService (%d): %s\n", GetLastError( ), (char*)msgBuf );
					LocalFree( msgBuf );
				}
			}

			// Close the handle to the service.
			CloseServiceHandle( hService );
		}
		else
		{
			// 오류메시지 출력 - OpenService
			VOID* msgBuf;
			if ( ErrorCode2String( &msgBuf ) )
			{
				printf( "\nOpenService (%d): %s\n", GetLastError( ), (char*)msgBuf );
				LocalFree( msgBuf );
			}
		}
	}
	return retValue;
}

// ConsolCtrlHandler Method
BOOL cLogDemon::ConsolCtrlHandler(DWORD opcode)
{
    switch ( opcode ) 
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		Destroy( );
		return TRUE;

	default:
		// unknown type--better pass it on.
		return FALSE;
	}
}

// ConsoleRun Method
void cLogDemon::ConsoleRun(DWORD argc, LPTSTR* argv)
{
	char moduleName[MAX_PATH];
	GetModuleFileName( NULL, moduleName, sizeof(moduleName) );

	char drive[_MAX_DRIVE];
	char directory[_MAX_DIR];
	char fileName[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath( moduleName, drive, directory, fileName, ext );

	if ( argc < 2 )
	{
		// 사용설명.
		printf( "USAGE:\n" );
		printf( "%s [/INSTALL | /UNINSTALL | /TEST]\n\n", fileName );

		printf( "    /INSTALL   Creates a service object and adds it to the specified service\n" );
		printf( "               control manager database.\n" );
		printf( "    /UNINSTALL Marks the specified service for deletion from the service\n" );
		printf( "               control manager database.\n" );
		printf( "    /TEST      Running the Demon Program in Console mode.\n" );
		printf( "    OPTIONS    -VERBOSE  Display Debug Information.\n" );
		printf( "               -PACKET   Display Network Information.\n" );

		// 1. 서비스 프로세스의 메인 쓰레드를 SCM(Service Control Manager)로 넘기는 함수다.
		//    이 함수를 부른 쓰레드는 Service Control Dispatcher 쓰레드가 된다.
		//
		//	 2. 서비스 프로그램이 시작되면, SCM은 StartServiceCtrlDispatcher를 불리길 기다리게 된다.
		//     StartServiceCtrlDispatcher가 성공하면, 쓰레드는 SCM 이 관리하며, 서비스가 끝날때까지 리턴되지 않는다.
		//     실제 서비스의 시작지점은 이 함수가 넘기는 인자값인 ServiceMain 이라고 볼수 있다.
		//
		//	 !. SERVICE_TABLE_ENTRY를 등록해야만 ServiceMain이 호출 된다.
		SERVICE_TABLE_ENTRY DispatchTable[]=
		{
			{ g_serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL,          (LPSERVICE_MAIN_FUNCTION)NULL        }
		};

		printf( "\n" );
		printf( "StartServiceCtrlDispatcher being called.\n" );
		printf( "This may take several seconds.  Please wait.\n" );

 		if ( !StartServiceCtrlDispatcher( DispatchTable ) )
		{
			VOID* msgBuf;
			if ( ErrorCode2String( &msgBuf ) )
			{
				printf( "\nStartServiceCtrlDispatcher (%d): %s\n", GetLastError( ), (char*)msgBuf );
				LocalFree( msgBuf );
			}
		}
	}
	else if ( !_stricmp( argv[1], "/INSTALL" ) )
	{
		// 서비스로 설치를 시도합니다.
		printf( "\nInstallation to your service is initiated.\n" );

		if ( InstallService( moduleName ) )
		{
			// 서비스로 설치를 완료 하였습니다.
			printf( "\nInstallation to your service is completed.\n" );
		}
		else
		{
			// 서비스로 설치를 완료하지 못 하였습니다.
			printf( "\nInstallation to your service is not completed.\n" );
		}
	}
	else if ( !_stricmp( argv[1], "/UNINSTALL" ) )
	{
		// 서비스로 삭제를 시도합니다.
		printf( "\nDeleting your service is initiated.\n" );

		if ( UninstallService( ) )
		{
			// 서비스로 삭제를 완료 하였습니다.
			printf( "\nDeleting your service is completed.\n" );
		}
		else
		{
			// 서비스로 삭제를 완료하지 못 하였습니다.
			printf( "\nDeleting our service is not completed.\n" );
		}
	}
	else if ( !_stricmp( argv[1], "/TEST" ) )
	{
		DWORD status;
		DWORD error;

		if ( SetConsoleCtrlHandler( ::ConsolCtrlHandler, TRUE ) != 0 )
		{
			// 테스트 옵션 검사.
			for ( DWORD i = 2; i < argc; i++ )
			{
				if ( !_stricmp( argv[ i ], "-VERBOSE" ) )
				{
					g_verbose = true;
				}
				else if ( !_stricmp( argv[ i ], "-FPS" ) )
				{
					g_fps = true;
				}
				else if ( !_stricmp( argv[ i ], "-PACKET" ) )
				{
					g_packet = true;
				}				
				else if ( !_stricmp( argv[ i ], "-UDP" ) )
				{
					g_udp = true;
				}				
				else if ( !_stricmp( argv[ i ], "-UDPPACKET" ) )
				{
					g_udpPacket = true;
				}				
				else if ( !_stricmp( argv[ i ], "-UDPFROM" ) )
				{
					g_udpFrom = true;
				}				
			}

			// 콘솔에서의 테스트.
			status = Initialization( argc, argv, &error );
			if ( status == NO_ERROR )
			{
				printf( "\nRunning the Server in Console mode.\n" );
				Run( );
				printf( "\nStopping the Server in Console mode.\n" );
			}
			else
			{
				printf( "\nInitialize fail.\n" );
			}
			SetConsoleCtrlHandler( ::ConsolCtrlHandler, FALSE );
		}
	}
}

// ServiceCtrlHandler Method
void cLogDemon::ServiceCtrlHandler(DWORD opcode)
{
	switch ( opcode )
	{
	case SERVICE_CONTROL_STOP:
		// Notifies a service that it should stop.
		mServiceStatus.dwWin32ExitCode = 0;
		mServiceStatus.dwCurrentState  = SERVICE_STOPPED;
		mServiceStatus.dwCheckPoint    = 0;
		mServiceStatus.dwWaitHint      = 0;
		// Updates the service control manager's status information for the calling service.
		SetServiceStatus( mServiceStatusHandle, &mServiceStatus );
		// 종료할 코드는 Destroy 함수를 사용.
		Destroy( );
		return;
	case SERVICE_CONTROL_PAUSE:
		// Notifies a service that it should pause.
		mServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		// Notifies a paused service that it should resume.
		mServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		// Notifies a service that it should report its current status information to the service control manager.
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		// Notifies a service that the system is shutting down so the service can perform cleanup tasks.
		break;
	case SERVICE_CONTROL_PARAMCHANGE:
		// Notifies a service that its startup parameters have changed. The service should reread its startup parameters.
		break;
	case SERVICE_CONTROL_NETBINDADD:
		// Notifies a network service that there is a new component for binding. The service should bind to the new component.
		break;
	case SERVICE_CONTROL_NETBINDREMOVE:
		// Notifies a network service that a component for binding has been removed.
		// The service should reread its binding information and unbind from the removed component.
		break;
	case SERVICE_CONTROL_NETBINDENABLE:
		// Notifies a network service that a disabled binding has been enabled.
		// The service should reread its binding information and add the new binding.
		break;
	case SERVICE_CONTROL_NETBINDDISABLE:
		// Notifies a network service that one of its bindings has been disabled.
		// The service should reread its binding information and remove the binding.
		break;
	}
	// Updates the service control manager's status information for the calling service.
	SetServiceStatus( mServiceStatusHandle, &mServiceStatus );
}

// ServiceRun Method
void cLogDemon::ServiceRun(DWORD argc, LPTSTR* argv)
{
	DWORD status;
	DWORD error;

	mServiceStatus.dwServiceType             = SERVICE_WIN32;
	mServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
	mServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
	mServiceStatus.dwWin32ExitCode           = 0;
	mServiceStatus.dwServiceSpecificExitCode = 0;
	mServiceStatus.dwCheckPoint              = 0;
	mServiceStatus.dwWaitHint                = 0;

	// 서비스 제어 처리자를 등록한다. - 실패시 0를 반환 한다.
	mServiceStatusHandle = RegisterServiceCtrlHandler( g_serviceName, ::ServiceCtrlHandler );
	if ( mServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 )
		return;

	// Initialization code goes here.
	// - 초기화 할 코드는 ServiceInitialization 함수를 사용
	// - 실패시 NO_ERROR(0)가 아닌 값를 반환.
	status = Initialization( argc, argv, &error );	

	// Handle error condition - 초기화 실패 할 경우 종료코드(status)를 표시해 준다.
	if ( status != NO_ERROR )
	{
		mServiceStatus.dwCurrentState            = SERVICE_STOPPED;
		mServiceStatus.dwCheckPoint              = 0;
		mServiceStatus.dwWaitHint                = 0;
		mServiceStatus.dwWin32ExitCode           = status;
		mServiceStatus.dwServiceSpecificExitCode = error;

		// Updates the service control manager's status information for the calling service.
		if ( SetServiceStatus( mServiceStatusHandle, &mServiceStatus ) == FALSE )
			return;
	}

	// Initialization complete - report running status.
	// - 초기화 완료 - 서비스 상태를 동작(RUNNING)으로 설정.
	mServiceStatus.dwCurrentState = SERVICE_RUNNING;
	mServiceStatus.dwCheckPoint   = 0;
	mServiceStatus.dwWaitHint     = 0;

	// Updates the service control manager's status information for the calling service.
	if ( SetServiceStatus( mServiceStatusHandle, &mServiceStatus ) == FALSE )
		return;

	// Run - 서비스 시작.
	Run( );
}

/*-- Initialization Method (Stub initialization function.)
*/
DWORD cLogDemon::Initialization(DWORD argc, LPTSTR* argv, DWORD* error)
{
	// WINSOCK2 초기화 (버전 2.2).
	if ( LOBYTE( m_wsaData.wVersion ) != 2 || HIBYTE( m_wsaData.wVersion ) != 2 )
		return 1L;

	/*-- ODBC-DNS & WINSOCK2 초기화. --*/

	timeBeginPeriod( 1 );

	// 사용자 정보.
	DWORD userNameSize   = 0;
	DWORD userNameRetVal = 0;

	userNameRetVal = GetUserName( mUserName, &userNameSize );

	if ( userNameRetVal == 0 && GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
	{
		mUserName      = (TCHAR*)GlobalAlloc( GPTR, userNameSize );
		userNameRetVal = GetUserName( mUserName, &userNameSize );
	}
	if ( userNameRetVal == 0 && GetLastError( ) == ERROR_INSUFFICIENT_BUFFER )
	{
		if ( g_verbose == true )
		{
			VOID* msgBuf;
			if ( ErrorCode2String( &msgBuf ) )
			{
				printf( "\nGetUserName (%d): %s\n", GetLastError( ), (char*)msgBuf );
				LocalFree( msgBuf );
			}
		}
		return 1L;
	}

	// 호스트 정보.
	if ( GetLocalHostInfo( ) == false )
	{
		if ( g_verbose == true )
		{
			VOID* msgBuf;
			if ( ErrorCode2String( &msgBuf ) )
			{
				printf( "\nGetNetworkParams (%d): %s\n", GetLastError( ), (char*)msgBuf );
				LocalFree( msgBuf );
			}
		}
		return 1L;
	}

	// 사용자 & 호스트 정보출력.
	if ( g_verbose == true )
	{
		printf( "\n" );
		printf( "User Name............: %s \n", mUserName );
		printf( "Host Name............: %s \n", mFixedInfo->HostName );
		printf( "NetBIOS Scope ID.....: %s \n", mFixedInfo->ScopeId );
		printf( "IP Routing Enabled...: %s \n", (mFixedInfo->EnableRouting ? "YES" : "NO") );
		printf( "WINS Proxy Enabled...: %s \n", (mFixedInfo->EnableProxy ? "YES" : "NO") );
		printf( "\n" );
	}

	// 실행환경 설정.
	char  drive[_MAX_DRIVE];
	char  dir[_MAX_DIR];
	char  fname[_MAX_FNAME];
	char  ext[_MAX_EXT];

	TCHAR filename[MAX_PATH]="\0";
	TCHAR directory[MAX_PATH]="\0";

	GetModuleFileName( NULL, filename, sizeof(filename) );
	_splitpath( filename, drive, dir, fname, ext );

	char* _ext = strrchr( filename, '.' );
	strcpy( _ext ? _ext : filename+strlen( filename ), ".cfg" );

	sprintf( directory, "%s%s", drive, dir );

	// 서비스 & 콘솔 & 디버그 모드 호환성 적용.
	if ( GetFileAttributes( filename ) == INVALID_FILE_ATTRIBUTES )
	{
		GetCurrentDirectory( sizeof(directory), directory );
		sprintf( filename, "%s\\%s.cfg", directory, fname );
	}

	// 실행파일경로 출력.
	if ( g_verbose == true )
	{
		printf( "Binary Path Name.....: %s%s%s%s \n", drive, dir, fname, ext );
		printf( "Working Directory....: %s \n", directory );
		printf( "Module Path Name.....: %s \n", filename );
		printf( "\n" );
	}

	FILE* stream = fopen( filename, "rt" );
	char  buffer[MAX_PATH];

	char  seps[] = " \t\n";			// 파싱구문 Parsing.
	char *token;

	if ( stream != NULL )
	{
		try {
			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGDEMON_BROADCAST_SEND(IPv4):" ) != 0 ) throw false;
			sprintf( mServerBroadcastSend, "%s", strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGDEMON_BROADCAST_RECV(IPv4):" ) != 0 ) throw false;
			sprintf( mServerBroadcastRecv, "%s", strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGIN_PORT:" ) != 0 ) throw true;
			g_loginPort = (u_short)atoi( strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "GAME_PORT:" ) != 0 ) throw true;
			g_gamePort = (u_short)atoi( strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOG_PORT:" ) != 0 ) throw true;
			g_logPort = (u_short)atoi( strtok( NULL, seps ) );

			throw true;
		}
		catch ( bool success ) {
			fclose( stream );
			if ( success == false )
				return 1L;
		}
	}
	else
		return 1L;

	if ( g_verbose == true )
	{
		printf( "\nLogDemon Broadcast Recv..: %s", mServerBroadcastRecv );
		printf( "\nLogDemon Broadcast Send..: %s", mServerBroadcastSend );
		printf( "\nLogin Port...............: %d", g_loginPort );
		printf( "\nGame Port................: %d", g_gamePort );
		printf( "\nLog Port.................: %d", g_logPort );
		printf( "\n" );
	}

	// 작업 디렉토리 설정.
	if ( SetCurrentDirectory( directory ) == false )
		return 1L;

	// cLogRecver 생성.
	mLogSender = new cLogSender( );
	if ( mLogSender->Initialize( mServerBroadcastSend, mServerBroadcastRecv, 2, 32768 ) == false )
		return 1L;

	// cSQLLog 생성.
	mSqlLog = new cSQLLog( );
	if ( mSqlLog->Initialize( ODBC_DSN_LOG, ODBC_UID, ODBC_PWD, 4 ) == false )
		return 1L;

	// cLogRecver 생성.
	mLogRecver = new cLogRecver( );
	if ( mLogRecver->Initialize( mServerBroadcastRecv, g_logPort, 2, 32768 ) == false )
		return 1L;

	return 0L;
}

// Destroy Method
void cLogDemon::Destroy(void)
{
	if ( mEndService == false )
	{
		mEndService = true;

		// cLogRecver 종료.
		mLogRecver->Shutdown( );

		// cSQLLog 종료.
		mSqlLog->Shutdown( );

		// cLogSender 종료.
		mLogSender->Shutdown( );

		// 실행종료.
		mRunService = false;
	}
}

// Close Method
void cLogDemon::Close( )
{
	mClose = true;
}

// Run Method
void cLogDemon::Run(void)
{
	DWORD currentTick = 0;
	DWORD elapsedTick = 0;

	// 서비스가 시작 됨.
	mRunService = true;
	while ( mRunService )
	{
		currentTick = GetTickCount( );

		if ( mClose == true )
			Destroy( );

		// 지연시간 약100(ms) - CPU 과부화 방지 & 내부 동기화.
		elapsedTick = GetTickCount( ) - currentTick;
		if ( elapsedTick < 100 )
		{
			Sleep( (100 - elapsedTick) );
		}
	}
}
