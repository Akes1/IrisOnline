// Include
#include "gamesrv.h"
#include <conio.h>
#include "RandomTable.h"
#include "FileSystem.h"

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// Local definitions
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // memory를 안전하게 해제
#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // handle를 안전하게 해제

#define ODBC_DSN_ACCOUNT	"IRIS_MEMBERDB"
#define ODBC_DSN_GAME		"IRIS_GAMEDB"

#define ODBC_UID			"iris"				// IDC test
#define ODBC_PWD			"vedsa239HPJam8Y"		// IDC test

// Global data
cGameSrv* g_gameSrv    = NULL;
bool      g_verbose    = false;
bool      g_fps        = false;
bool      g_packet     = false;
bool      g_udp        = false;
bool      g_udpPacket  = false;
bool      g_udpFrom    = false;
bool      g_eventPrint = false;

u_short   g_loginPort  = 15002;
u_short   g_gamePort   = 15004;
u_short   g_logPort    = 15000;

unsigned int g_BillType = 1;

// 서비스 이름 및 표시이름 설정 - 기본값.
TCHAR g_serviceName[MAX_PATH] = TEXT( "GameServer" );
TCHAR g_description[MAX_PATH] = TEXT( "Copyright(c) 2009 EYA INTERACTIVE Corporation All rights reserved" );
TCHAR g_displayFmt[]          = TEXT( "IRIS ON LINE - %s" );

// GetLogicalAddress Prototype
BOOL __stdcall GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD &section, DWORD &offset)
{
	#pragma warning( disable: 4311 )
	#pragma warning( disable: 4312 )

	MEMORY_BASIC_INFORMATION mbi;

	if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
		return FALSE;

	PIMAGE_DOS_HEADER     pDosHdr  = (PIMAGE_DOS_HEADER)hMod;
	PIMAGE_NT_HEADERS     pNtHdr   = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHdr);
	DWORD                 rva      = (DWORD)addr - hMod; // RVA is offset from module load address

	for ( unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd   = sectionStart + max( pSection->SizeOfRawData, pSection->Misc.VirtualSize );

		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			section = i + 1;
			offset  = rva - sectionStart;
			return TRUE;
		}
	}
	return FALSE;
}

// WriteStackDetails Prototype
void WriteStackDetails(PCONTEXT context)
{
	#pragma warning( disable: 4127 )

	char       fileName[MAX_PATH];
	SYSTEMTIME st;
	char       buffer[MAX_PATH];

	GetLocalTime( &st );
	sprintf_s( fileName
			  ,MAX_PATH
			  ,"GameServer_%4d%02d%02d_%02d%02d.StackWalk"
			  ,st.wYear
			  ,st.wMonth
			  ,st.wDay
			  ,st.wHour
			  ,st.wMinute );

	FILE* stream = fopen( fileName, "wt" );
	if ( stream != NULL )
	{
		fputs( "#Fields: Address Frame Function SourceFile\n", stream );

		STACKFRAME sf;
		memset( &sf, 0, sizeof(sf) );

		// Initialize the STACKFRAME structure for the first call. This is only   
		//  necessary for Intel CPUs, and isn't mentioned in the documentation.   
		sf.AddrPC.Offset    = context->Eip;   
		sf.AddrPC.Mode      = AddrModeFlat;   
		sf.AddrStack.Offset = context->Esp;   
		sf.AddrStack.Mode   = AddrModeFlat;   
		sf.AddrFrame.Offset = context->Ebp;   
		sf.AddrFrame.Mode   = AddrModeFlat;   

		while ( 1 )   
		{
			// Get the next stack frame
			// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag
			if ( !StackWalk( IMAGE_FILE_MACHINE_I386
							,GetCurrentProcess( )
							,GetCurrentThread( )
							,&sf
							,context
							,0
							,SymFunctionTableAccess
							,SymGetModuleBase
							,0 ) )
				break;

			if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure the frame is OK. Bail if not.
				break;

			sprintf( buffer, "%08X %08X ", sf.AddrPC.Offset, sf.AddrFrame.Offset );
			fputs( buffer, stream );

			// Get the name of the function for this stack frame entry
			BYTE         symbolBuffer[ sizeof(SYMBOL_INFO) + 1024 ];
			PSYMBOL_INFO pSymbol  = (PSYMBOL_INFO)symbolBuffer;
			pSymbol->SizeOfStruct = sizeof(symbolBuffer);
			pSymbol->MaxNameLen   = 1024;

			// relative to the start of the symbol
			{
				TCHAR szModule[MAX_PATH]  =  "";
				DWORD section = 0, offset = 0;

				GetLogicalAddress( (PVOID)sf.AddrPC.Offset, szModule, sizeof(szModule), section, offset );

				sprintf( buffer, "%04X:%08X %s", section, offset, szModule );
				fputs( buffer, stream );
			}

			// Get the source line for this stack frame entry
			IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
			DWORD         dwLineDisplacement;

			if ( SymGetLineFromAddr( GetCurrentProcess( )
									,sf.AddrPC.Offset
									,&dwLineDisplacement
									,&lineInfo ) )
			{
				sprintf( buffer, "%s line %u", lineInfo.FileName, lineInfo.LineNumber );
				fputs( buffer, stream );
			}
			
			fputs( "\n", stream );
		}
		fclose( stream );
	}
}

// GenerateDump Prototype
//  1. 속성 < C/C++ < 일반 < 디버깅 정보 형식 = 프로그램 데이터베이스(/Zi)
//  2. SetUnhandledExceptionFilter( Exception_Minidump );
//  3. exceptionPointers->ExceptionRecord->ExceptionCode는 winbase.h and winnt.h 파일참조
//     (EXCEPTION_ACCESS_VIOLATION ... CONTROL_C_EXIT 등)
//
//  주의: DBGHELP.DLL 파일의 버전은 최소 5.1.2600.0 이상이어야 한다.
//        (MiniDumpWriteDump 함수가 DBGHELP.DLL에 들어 있음)
LONG __stdcall GenerateDump(EXCEPTION_POINTERS* exceptionPointers)
{
	char       fileName[MAX_PATH];
	SYSTEMTIME st;

	GetLocalTime( &st );
	sprintf_s( fileName
			  ,MAX_PATH
			  ,"GameServer_%4d%02d%02d_%02d%02d%02d(0x%08X 0x%08p 0x%02X 0x%04X 0x%08X).dmp"
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

	HANDLE process   = GetCurrentProcess( );
	DWORD  processID = GetCurrentProcessId( );
	HANDLE file      = CreateFile( fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	MINIDUMP_EXCEPTION_INFORMATION expParam;

	expParam.ThreadId          = GetCurrentThreadId( );
	expParam.ExceptionPointers = exceptionPointers;
	expParam.ClientPointers    = TRUE;

	MiniDumpWriteDump( process, processID, file, MiniDumpWithDataSegs, &expParam, NULL, NULL );

	SymSetOptions( SYMOPT_DEFERRED_LOADS );

	if ( SymInitialize( GetCurrentProcess( ), 0, TRUE ) )
	{
		WriteStackDetails( exceptionPointers->ContextRecord );
	}

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
	HMODULE module = NULL; // default to system source
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
	if ( g_gameSrv )
		return g_gameSrv->ConsolCtrlHandler( opcode );
	else
		return FALSE;
}

// main Prototype (This method is the standard Windows program entry point.)
int main(int argc, char* argv[])
{
	LPTOP_LEVEL_EXCEPTION_FILTER previousFilter = NULL;
	previousFilter = SetUnhandledExceptionFilter( GenerateDump );

	cGameSrv* gameSrv = NULL;

	gameSrv = new cGameSrv( );
	gameSrv->ConsoleRun( argc, argv );

	SafeDelete( gameSrv );
	
	SetUnhandledExceptionFilter( previousFilter );
	return 0L;
}

// ServiceCtrlHandler Prototype - Windows Service 제어용 함수.
void WINAPI ServiceCtrlHandler(DWORD opcode)
{
	if ( g_gameSrv )
		g_gameSrv->ServiceCtrlHandler( opcode );
}

// ServiceMain Prototype (This method is the entry point for a service.)
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	cGameSrv* gameSrv = NULL;

	gameSrv = new cGameSrv( );
	gameSrv->ServiceRun( argc, argv );

	SafeDelete( gameSrv );
}

// cGameSrv Constructor.
cGameSrv::cGameSrv(void) : mSender(NULL), mSqlAccount(NULL), mSqlGame(NULL), mBillConnector(NULL), mBillPayletter(NULL), mGameProcess(NULL), mVerbose(NULL), mRecver(NULL)
{
	g_gameSrv       = this;

	mUserName       = NULL;
	mRunService     = false;
	mEndService     = false;
	mClose          = false;
	mVersion        = VERSION_HEADER;

	mServerNo       = 0;
	mChannelNo      = 0;
	mAhnhsEnbale    = 0;
	mAhnhsCheckTime = 0;
	mAhnhsWaitTime  = 0;
}

// ~cGameSrv Destructor.
cGameSrv::~cGameSrv(void)
{
	// cRecver 클래스 해제.
	SafeDelete( mRecver );

	// cGameProcess 클래스 해제.
	SafeDelete( mGameProcess );

	// cBillConnector 클래스 해제.
	SafeDelete( mBillPayletter );

	// cBillConnector 클래스 해제.
	SafeDelete( mBillConnector );

	// cSQLGame 클래스 해제.
	SafeDelete( mSqlGame );

	// cSQLAccount 클래스 해제.
	SafeDelete( mSqlAccount );

	// cSender 클래스 해제.
	SafeDelete( mSender );

	// cVerbose 클래스 해제.
	SafeDelete( mVerbose );

	g_gameSrv = NULL;
}

// InstallService Method - 서비스 설치 함수.
bool cGameSrv::InstallService(LPCTSTR binaryPathName, LPCTSTR serviceName, DWORD startType)
{
	SC_HANDLE           hManager(NULL);
	SC_HANDLE           hService(NULL);
	bool                retValue(false);

	TCHAR batchPathName[MAX_PATH];
	sprintf( batchPathName, "%s -def %s", binaryPathName, serviceName );

	TCHAR displayName[MAX_PATH];
	sprintf( displayName, g_displayFmt, serviceName );

	// Open a handle to the SC Manager database.
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( hManager != NULL )
	{
		// The function creates a service object and adds it to the specified service control manager database.
		hService = CreateService( hManager,                  // handle to SCM database
								  serviceName,               // name of service to start
								  displayName,               // display name
								  SERVICE_ALL_ACCESS,        // type of access to service
								  SERVICE_WIN32_OWN_PROCESS, // type of service
								  startType,                 // when to start service
								  SERVICE_ERROR_NORMAL,      // severity of service failure
								  batchPathName,             // name of binary file
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
bool cGameSrv::UninstallService(LPCTSTR serviceName)
{
	SC_HANDLE hManager(NULL);
	SC_HANDLE hService(NULL);
	bool      retValue(false);
	DWORD     lastError(0);

	// Open a handle to the SC Manager database.
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( hManager != NULL )
	{
		// Open a handle to the service.
		hService = OpenService( hManager, serviceName, SERVICE_ALL_ACCESS );
		if ( hService != NULL )
		{
			// The function marks the specified service for deletion from the service control manager database.
			if ( DeleteService( hService ) != 0 )
				retValue = true;	// Success DeleteService
			else
				lastError = GetLastError( );

			// Close the handle to the service.
			CloseServiceHandle( hService );
		}
		else
			lastError = GetLastError( );
	}
	if ( lastError != 0 )
	{
		// 오류메시지 출력 - OpenService
		VOID* msgBuf;
		if ( ErrorCode2String( &msgBuf ) )
		{
			printf( "\nOpenService (%d): %s\n", GetLastError( ), (char*)msgBuf );
			LocalFree( msgBuf );
		}
	}
	return retValue;
}

// ErrorInsert Method
int cGameSrv::ErrorInsert(char* msg)
{
	printf( "%s\n", msg );
	printf( "Press Any Key to Continue...\n" );
	_getch( );
	return 1;
}

// ConsolCtrlHandler Method
BOOL cGameSrv::ConsolCtrlHandler(DWORD opcode)
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
void cGameSrv::ConsoleRun(DWORD argc, LPTSTR* argv)
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
		printf( "%s [/INSTALL | /UNINSTALL | /INSERT | /CLEAR | /TEST]\n\n", fileName );

		printf( "    /INSTALL   Creates a service object and adds it to the specified service\n" );
		printf( "               control manager database.\n" );
		printf( "    OPTION     -DEF NAME Uses the name specified during installation.\n" );
		printf( "    /UNINSTALL Marks the specified service for deletion from the service\n" );
		printf( "               control manager database.\n" );
		printf( "    OPTION     -DEF NAME While deleting the specified name is used.\n" );
		printf( "    /INSERT    Upload script files to MSSQL Database.\n" );
		printf( "    /TEST      Running the Server Program in Console mode.\n" );
		printf( "    OPTIONS    -VERBOSE  Display Debug Information.\n" );
		printf( "               -PACKET   Display Network Information.\n" );
		printf( "               -DEF NAME Uses the name specified during execution.\n" );
	}
	else if ( argc == 3 && !_stricmp( argv[1], "-def" ) )
	{
		// 서비스 이름 재설정.
		strcpy( g_serviceName, argv[2] );

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
	else if ( !_stricmp( argv[1], "/VER" ) )
	{
		int major = MAJOR_VERSION( mVersion  );
		int minor = MINOR_VERSION( mVersion  );
		printf( "\nVersion(%d.%d)\n", major, minor );
		printf( "\nPress Any Key to Continue." );
		_getch( );
	}
	else if ( !_stricmp( argv[1], "/INSTALL" ) )
	{
		if ( argc == 4 && !stricmp( argv[2], "-DEF" ) )
		{
			// 서비스로 설치를 시도합니다.
			printf( "\nInstallation to your service is initiated.\n" );

			if ( InstallService( moduleName, argv[3] ) )
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
		else
		{
			printf( "USAGE:\n" );
			printf( "    /INSTALL   Creates a service object and adds it to the specified service\n" );
			printf( "               control manager database.\n" );
			printf( "    OPTION     -DEF NAME  Uses the name specified during installation." );
		}
	}
	else if ( !_stricmp( argv[1], "/UNINSTALL" ) )
	{
		if ( argc == 4 && !stricmp( argv[2], "-def" ) )
		{
			// 서비스로 삭제를 시도합니다.
			printf( "\nDeleting your service is initiated.\n" );

			if ( UninstallService( argv[3] ) )
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
		else
		{
			printf( "USAGE:\n" );
			printf( "    /UNINSTALL Marks the specified service for deletion from the service\n" );
			printf( "               control manager database.\n" );
			printf( "    OPTION     -DEF NAME  While deleting the specified name is used." );
		}
	}
	else if ( !_stricmp( argv[1], "/INSERT" ) )
	{
		// 우선순위 1. cSQLGame 생성.
		cSQLGame*    sqlGame    = new cSQLGame( );
		cFileSystem* fileSystem = new cFileSystem( );
		int          errorCount = 0;

		printf( "\n=============== INSERT START ===============\n" );

		if ( sqlGame->ConsoleInsertInit( ) )
			printf( "\nSQLGame Initialization & Data loading success.\n" );
		else
			errorCount += ErrorInsert( "SQLGame Initialization & Data loading failed!!!" );

		if ( sqlGame->InsertItemDefine( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_DEFINE table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_DEFINE table was not complete!!!" );

		if ( sqlGame->InsertItemAbility( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_ABILITY table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_ABILITY table was not complete!!!" );

		if ( sqlGame->InsertItemLimit( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_LIMIT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_LIMIT table was not complete!!!" );

		if ( sqlGame->InsertItemCard( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_CARD table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_CARD table was not complete!!!" );

		if ( sqlGame->InsertItemCardSlot( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_CARD_SLOT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_CARD_SLOT table was not complete!!!" );

		if ( sqlGame->InsertItemTarot( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_TAROT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_TAROT table was not complete!!!" );

		if ( sqlGame->InsertItemTarotResult( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_TAROT_RESULT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_TAROT_RESULT table was not complete!!!" );

		if ( sqlGame->InsertItemSpread( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_SPREAD table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_SPREAD table was not complete!!!" );

		if ( sqlGame->InsertItemDisjoint( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_DISJOINT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_DISJOINT table was not complete!!!" );

		if ( sqlGame->InsertItemEnhanced( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_ENHANCED table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_ENHANCED table was not complete!!!" );

		if ( sqlGame->InsertItemEnhancedRate( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_ENHANCED_RATE table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_ENHANCED_RATE table was not complete!!!" );

		if ( sqlGame->InsertItemChange( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEM_CHANGE table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEM_CHANGE table was not complete!!!" );

		if ( sqlGame->InsertDefaultItems( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_DEFAULT_ITEMS table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_DEFAULT_ITEMS table was not complete!!!" );

		if ( sqlGame->InsertSkillPC( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_SKILLLIST_PC table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_SKILLLIST_PC table was not complete!!!" );

		if ( sqlGame->InsertDefaultSkill( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_DEFAULT_SKILL table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_DEFAULT_SKILL table was not complete!!!" );

		if ( sqlGame->InsertJobTree( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_JOBTREE table has been completed.\n" );
		else
			errorCount += ErrorInsert( "nTB_JOBTREE table was not complete!!!" );

		/// 퀘스트 데이터 로드
		if ( sqlGame->InsertQuestData( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_QUEST table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_QUEST table was not complete!!!" );

		/// 드랍시간제약 데이터 로드
		if ( sqlGame->InsertItemDropLimitData( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
			printf( "\nTB_ITEMDROP_LIMIT table has been completed.\n" );
		else
			errorCount += ErrorInsert( "TB_ITEMDROP_LIMIT table was not complete!!!" );

		sqlGame->ConsoleInsertRelease( );

		printf( "\n=============== Completed ===============\n" );
		printf( "\nINSERT - ERROR(=%d)\n", errorCount );
		printf( "\nPress Any Key to Continue." );
		_getch( );

		SafeDelete( fileSystem );
		SafeDelete( sqlGame );
	}
	else if ( !_stricmp( argv[1], "/CLEAR" ) )
	{
		cSQLGame* sqlGame = new cSQLGame( );
		int       c;

		printf( "\n=============== CLEAR START ===============\n" );
		printf( "\nWould you like to go? 'Y' or 'N' = " );

		c = _getch( );
		if ( c == 'Y' || c == 'y' )
		{
			int total;
			int remove;
			int result;

			bool optimizedInventory;
			bool optimizedInfluence;
			bool optimizedPost;
			bool optimizedAgent;
			bool optimizedQuestProgress;
			bool optimizedQuestValid;

			printf( "Operation starts.\n\n" );
			printf( "NUM TABLE NAME                      ROW   REMOVE   RESULT (%%)\n" );

			//TB_INVENTORY
			printf( "1/6 TB_INVENTORY..................: " );
			sqlGame->sp_inventory( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedInventory = !(remove > 0);

			//TB_CHARACTER_INFLUENCE
			printf( "2/6 TB_CHARACTER_INFLUENCE........: " );
			sqlGame->sp_influence( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedInfluence = !(remove > 0);

			//TB_POST
			printf( "3/6 TB_POST.......................: " );
			sqlGame->sp_post( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedPost = !(remove > 0);

			//TB_ITEM_AGENT
			printf( "4/6 TB_ITEM_AGENT.................: " );
			sqlGame->sp_agent( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedAgent = !(remove > 0);

			//TB_QUEST_PROGRESS
			printf( "5/6 TB_QUEST_PROGRESS.............: " );
			sqlGame->sp_quest_progress( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedQuestProgress = !(remove > 0);

			//TB_QUEST_VALID
			printf( "6/6 TB_QUEST_VALID................: " );
			sqlGame->sp_quest_valid( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			result = total-remove;
			if ( remove > 0 )
				printf( "%8d %8d %8d (%5.2f%%).\n", total, remove, result, (float)result/(float)total*100.f );
			else
				printf( "Optimization completed.\n" );
			optimizedQuestValid = !(remove > 0);

			// 1/4
			if ( !optimizedInventory )
			{
				printf( "\n1/6 TB_INVENTORY..................: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );
					printf( "Call sp_remove_inventory..........: " );	// SELECT * INTO BACKUPDB.TB_INVENTORY FROM TB_INVENTORY WHERE APPLY=1

					bool success = sqlGame->sp_remove_inventory( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_inventory..........: " );	// DELETE TB_INVENTORY WHERE APPLY=1

					if ( success && sqlGame->sp_delete_inventory( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_inventory......: " );	// DBCC DBREINDEX (TB_INVENTORY)

					if ( success && sqlGame->sp_db_reindex_inventory( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 2/4
			if ( !optimizedInfluence )
			{
				printf( "\n2/6 TB_CHARACTER_INFLUENCE........: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );

					printf( "Call sp_remove_influence..........: " );	// SELECT * INTO BACKUPDB.TB_CHARACTER_INFLUENCE FROM TB_CHARACTER_INFLUENCE WHERE IS_DEL=1

					bool success = sqlGame->sp_remove_influence( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_influence..........: " );	// DELETE TB_CHARACTER_INFLUENCE WHERE IS_DEL=1

					if ( success && sqlGame->sp_delete_influence( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_influence......: " );	// DBCC DBREINDEX (TB_CHARACTER_INFLUENCE)

					if ( success && sqlGame->sp_db_reindex_influence( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 3/4
			if ( !optimizedPost )
			{
				printf( "\n3/6 TB_POST.......................: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );
					printf( "Call sp_remove_post...............: " );	// SELECT * INTO BACKUPDB.TB_POST FROM TB_POST WHERE APPLY=1

					bool success = sqlGame->sp_remove_post( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_post...............: " );	// DELETE TB_POST WHERE APPLY=1

					if ( success && sqlGame->sp_delete_post( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_post...........: " );	// DBCC DBREINDEX (TB_POST)

					if ( success && sqlGame->sp_db_reindex_post( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 4/4
			if ( !optimizedAgent )
			{
				printf( "\n4/6 TB_ITEM_AGENT.................: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );
					printf( "Call sp_remove_agent..............: " );	// SELECT * INTO BACKUPDB.TB_ITEM_AGENT FROM TB_ITEM_AGENT WHERE APPLY=1

					bool success = sqlGame->sp_remove_agent( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_agent..............: " );	// DELETE TB_ITEM_AGENT WHERE APPLY=1

					if ( success && sqlGame->sp_delete_agent( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_agent..........: " );	// DBCC DBREINDEX (TB_ITEM_AGENT)

					if ( success && sqlGame->sp_db_reindex_agent( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 5/6
			if ( !optimizedQuestProgress )
			{
				printf( "\n5/6 TB_QUEST_PROGRESS.............: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );
					printf( "Call sp_remove_quest_progress.....: " );	// SELECT * INTO BACKUPDB.TB_QUEST_PROGRESS FROM TB_QUEST_PROGRESS DELETEWAIT =1

					bool success = sqlGame->sp_remove_quest_progress( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_quest_progress.....: " );	// DELETE TB_QUEST_PROGRESS WHERE DELETEWAIT =1

					if ( success && sqlGame->sp_delete_quest_progress( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_quest_progress.: " );	// DBCC DBREINDEX (TB_QUEST_PROGRESS)

					if ( success && sqlGame->sp_db_reindex_quest_progress( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 6/6
			if ( !optimizedQuestValid )
			{
				printf( "\n6/6 TB_QUEST_VALID................: Operations do? 'Y' or 'N' = " );
				c = _getch( );
				if ( c == 'Y' || c == 'y' )
				{
					printf( "Continue\n" );
					printf( "Call sp_remove_quest_valid........: " );	// SELECT * INTO BACKUPDB.TB_QUEST_VALID FROM TB_QUEST_VALID DELETEWAIT =1

					bool success = sqlGame->sp_remove_quest_valid( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD );
					if ( success )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_delete_quest_valid........: " );	// DELETE TB_QUEST_VALID WHERE DELETEWAIT =1

					if ( success && sqlGame->sp_delete_quest_valid( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );

					printf( "Call sp_db_reindex_quest_valid....: " );	// DBCC DBREINDEX (TB_QUEST_PROGRESS)

					if ( success && sqlGame->sp_db_reindex_quest_valid( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD ) )
						printf( "completed \n" );
					else
						printf( "failed \n" );
				}
				else
					printf( "Ignore.\n" );
			}

			// 결과 출력
			printf( "\nResults.\n" );

			sqlGame->sp_inventory( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "1/6 TB_INVENTORY..................: %8d\n", total );

			sqlGame->sp_influence( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "2/6 TB_CHARACTER_INFLUENCE........: %8d\n", total );

			sqlGame->sp_post( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "3/6 TB_POST.......................: %8d\n", total );

			sqlGame->sp_agent( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "4/6 TB_ITEM_AGENT.................: %8d\n", total );

			sqlGame->sp_quest_progress( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "5/6 TB_QUEST_PROGRESS.............: %8d\n", total );

			sqlGame->sp_quest_valid( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, total, remove );
			printf( "6/6 TB_QUEST_VALID................: %8d\n", total );

		}
		else
		{
			printf( "To cancel the operation.\n" );
		}

		printf( "\n=============== Completed ===============\n" );
		printf( "\nPress Any Key to Continue." );
		_getch( );
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
				else if ( !_stricmp( argv[ i ], "-DEF" ) )
				{
					// 서비스 이름 재설정.
					if ( (++i) < argc )
						strcpy( g_serviceName, argv[i] );
				}
				else if ( !_stricmp( argv[ i ], "-EVENT" ) )
				{
					g_eventPrint = true;
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
void cGameSrv::ServiceCtrlHandler(DWORD opcode)
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
void cGameSrv::ServiceRun(DWORD argc, LPTSTR* argv)
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

// Initialization Method (Stub initialization function.)
// . 클래스 생성 순서 주위
DWORD cGameSrv::Initialization(DWORD /*argc*/, LPTSTR* /*argv*/, DWORD* /*error*/)
{
	strcpy( mServerBillAddr, "0.0.0.0" );
	mServerBillPort = 0;

	// WINSOCK2 초기화 (버전 2.2).
	if ( LOBYTE( m_wsaData.wVersion ) != 2 || HIBYTE( m_wsaData.wVersion ) != 2 )
		return 1L;

	// 타이머의 성능을 향상시킨다.
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

	// 모듈정의 파일 적용.
	FILE* stream = fopen( filename, "rt" );
	char  buffer[MAX_PATH];

	char  seps[] = " \t\n";			// 파싱구문 Parsing.
	char *token;

	if ( stream != NULL )
	{
		try {
			while ( fgets( buffer, MAX_PATH, stream ) != NULL )
			{
				token = strtok( buffer, seps );
				if ( token && stricmp( token, "SERVICE_NAME:" ) == 0 )
				{
					token = strtok( NULL, seps );
					if ( token && stricmp( token, g_serviceName ) == 0 )
					{
						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "SERVER_NO:" ) != 0 ) throw false;
						mServerNo = (DWORD)atoi( strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "CHANNEL_NO:" ) != 0 ) throw false;
						mChannelNo = (DWORD)atoi( strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "GAME_SERVER_ADDRESS(IPv4):" ) != 0 ) throw false;
						sprintf( mServerAddr, "%s", strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "GAME_BROADCAST_RECV(IPv4):" ) != 0 ) throw false;
						sprintf( mServerBroadcastRecv, "%s", strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "GAME_BROADCAST_SEND(IPv4):" ) != 0 ) throw false;
						sprintf( mServerBroadcastSend, "%s", strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "GAME_FIREWALL(IPv4):" ) != 0 ) throw false;
						sprintf( mServerFirewall, "%s", strtok( NULL, seps ) );

						//if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

						//token = strtok( buffer, seps );
						//if ( token && stricmp( token, "GAME_BILLING_TYPE:" ) != 0 ) throw true;
						//g_BillType = atoi( strtok( NULL, seps ) );
						g_BillType = 0;

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "GAME_BILLING(IPv4):" ) != 0 ) throw true;
						sprintf( mServerBillAddr, "%s", strtok( NULL, seps ) );
						mServerBillPort = atoi( strtok( NULL, seps ) );

						if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw true;

						token = strtok( buffer, seps );
						if ( token && stricmp( token, "AHN_HACK_SHIELD:" ) != 0 ) throw true;
						mAhnhsEnbale    = atoi( strtok( NULL, seps ) );
						mAhnhsCheckTime = (DWORD)atoi( strtok( NULL, seps ) );
						mAhnhsWaitTime  = (DWORD)atoi( strtok( NULL, seps ) );

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
				}
			}
			throw false;
		}
		catch ( bool success ) {
			fclose( stream );
			if ( success == false )
				return 1L;
		}
	}
	else
		return 1L;

	sprintf( mServerName, "%s-%d%03d", g_serviceName, mServerNo, mChannelNo );

	if ( g_verbose == true )
	{
		printf( "\nServer Name..........: %s", mServerName );
		printf( "\nServer Number........: %d", mServerNo );
		printf( "\nChannel Number.......: %d", mChannelNo );
		printf( "\nGame Server Address..: %s:%d (FW%s)", mServerAddr, T_GAME_CPORT, mServerFirewall );
		printf( "\nGame Broadcast Recv..: %s", mServerBroadcastRecv );
		printf( "\nGame Broadcast Send..: %s", mServerBroadcastSend );
		printf( "\nGame Billing Type....: %d", g_BillType );
		printf( "\nGame Billing Address.: %s:%d", mServerBillAddr, mServerBillPort );
		printf( "\nAhn Hack Shield:.....: %s(CT%d:WT%d)", (mAhnhsEnbale==1?"ON":"OFF"), mAhnhsCheckTime, mAhnhsWaitTime );
		printf( "\nLogin Port...........: %d", g_loginPort );
		printf( "\nGame Port............: %d", g_gamePort );
		printf( "\nLog Port.............: %d", g_logPort );
		printf( "\n" );
	}

	/*-- ODBC-DNS & WINSOCK2 초기화. --*/

	// 작업 디렉토리 설정.
	if ( SetCurrentDirectory( directory ) == false )
		return 1L;

	// 우선순위 1. cSQLAccount 생성.
	mSender = new cSender( );
	if ( mSender->Initialize( mServerBroadcastSend, mServerBroadcastRecv, 1, 32768 ) == false )
		return 1L;

	// 우선순위 2. cSQLAccount 생성.
	mSqlAccount = new cSQLAccount( );
	if ( mSqlAccount->Initialize( ODBC_DSN_ACCOUNT, ODBC_UID, ODBC_PWD, 2, 32768 ) == false )
		return 1L;

	// 우선순위 3. cSQLGame 생성.
	mSqlGame = new cSQLGame( );
	if ( mSqlGame->Initialize( ODBC_DSN_GAME, ODBC_UID, ODBC_PWD, 4, 32768 ) == false )
		return 1L;

	if( g_BillType == 1 )
	{
		// 우선순위 4. cBillConnector 생성 - '0.0.0.0' 주소가 아닐경우만 실행.
		mBillConnector = new cBillConnector( );
		if ( strcmp( mServerBillAddr, "0.0.0.0" ) )
		{
			if ( mBillConnector->Initialize( mServerBillAddr, (u_short)mServerBillPort, 1, 8192 ) == false )
				return 1L;
		}
	}
	else if( g_BillType == 2 )
	{
		// 우선순위 4. cBillPayletter 생성 - '0.0.0.0' 주소가 아닐경우만 실행.
		mBillPayletter = new cBillPayletter( );
		if ( strcmp( mServerBillAddr, "0.0.0.0" ) )
		{
			if ( mBillPayletter->BPInitialize( mServerBillAddr, (u_short)mServerBillPort, 1, 8192, (unsigned short)mServerNo, (unsigned short)mChannelNo ) == false )
				return 1L;
		}
	}

	// 우선순위 5. cGameProcess 생성.
	mGameProcess = new cGameProcess( mServerNo, mChannelNo, mServerFirewall/*, mAhnhsEnbale, mAhnhsCheckTime, mAhnhsWaitTime*/ );
	if ( mGameProcess->Initialize( mServerAddr, T_GAME_CPORT, 2, 16384 ) == false )
		return 1L;

	mVerbose = new cVerbose( );
	if ( mVerbose == NULL )
		return 1L;

	// 우선순위 6. cRecver 생성.
	mRecver = new cRecver( );
	if ( mRecver->Initialize( mServerBroadcastRecv, g_gamePort, 1, 32768 ) == false )
		return 1L;

	return 0L;
}

// Destroy Method
// . 클래스 종료 순서 주위.
void cGameSrv::Destroy(void)
{
	if ( mEndService == false )
	{
		mEndService = true;					// 서비스 종료.
		if ( mRecver != NULL )
		{
			mRecver->Shutdown( );			// 우선순위 1. cRecver 종료.
		}
		if ( mGameProcess != NULL )
		{
			mGameProcess->Shutdown( );		// 우선순위 2. cGameProcess 종료.
		}
		if ( mBillPayletter != NULL )
		{
			mBillPayletter->Shutdown( );	// 우선순위 3. mBillConnector 종료.
		}
		if ( mBillConnector != NULL )
		{
			mBillConnector->Shutdown( );	// 우선순위 4. mBillConnector 종료.
		}
		if ( mSqlAccount != NULL )
		{
			mSqlAccount->Shutdown( );		// 우선순위 5. cSQLAccount 종료.
		}
		if ( mSqlGame != NULL )
		{
			mSqlGame->Shutdown( );			// 우선순위 6. cSQLGame 종료.
		}
		if ( mSender != NULL )
		{
			mSender->Shutdown( );			// 우선순위 7. cSender 종료.
		}
		if ( mUserName != NULL )
		{
			GlobalFree( mUserName );		// 메모리 해제.
			mUserName = NULL;
		}
		mRunService = false;				// 실행종료.
	}
}

// Close Method
void cGameSrv::Close( )
{
	mClose = true;
}

// Run Method
void cGameSrv::Run(void)
{
	DWORD currentTick;
	DWORD elapsedTick;

	// 서비스가 시작 됨.
	mRunService = true;
	while ( mRunService )
	{
		currentTick = GetTickCount( );

		if ( mClose == true )
			Destroy( );

		// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
		elapsedTick = GetTickCount( ) - currentTick;
		if ( elapsedTick < 100 )
		{
			Sleep( (100 - elapsedTick) );
		}
	}
}
