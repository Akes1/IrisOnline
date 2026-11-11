// Include
#include "loginsrv.h"
#include <conio.h>

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

// Local definitions
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // memory ∞≤»´ Õ∑≈
#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // handle ∞≤»´πÿ±’

#define ODBC_DSN_WEB		"WEB_MEMBERDB"
#define ODBC_DSN_ACCOUNT	"IRIS_MEMBERDB"

#define ODBC_UID_WEB		"iris"			// local test
#define ODBC_PWD_WEB		"vedsa239HPJam8Y"		// local test

#define ODBC_UID			"iris"				// IDC test
#define ODBC_PWD			"vedsa239HPJam8Y"		// IDC test

// Global data
cLoginSrv* g_loginSrv = NULL;
bool       g_verbose    = false;
bool       g_fps        = false;
bool       g_packet     = false;
bool       g_udp        = false;
bool       g_udpPacket  = false;
bool       g_udpFrom    = false;

u_short    g_loginPort  = 15002;
u_short    g_gamePort   = 15004;
u_short    g_logPort    = 15000;

// …Ë÷√∑˛ŒÒ√˚≥∆∫Õœ‘ æ√˚≥∆ - ƒ¨»œ.
TCHAR g_serviceName[MAX_PATH] = TEXT( "LoginServer" );
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
	char       fileName[MAX_PATH];
	SYSTEMTIME st;
	char       buffer[MAX_PATH];

	GetLocalTime( &st );
	sprintf_s( fileName
			  ,MAX_PATH
			  ,"LoginServer_%4d%02d%02d_%02d%02d.StackWalk"
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

			DWORD64 symDisplacement = 0; // Displacement of the input address,

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
//  1. º”º∫ < C/C++ < ¿œπ› < µπˆ±Î ¡§∫∏ «¸Ωƒ = «¡∑Œ±◊∑• µ•¿Ã≈Õ∫£¿ÃΩ∫(/Zi)
//  2. SetUnhandledExceptionFilter( Exception_Minidump );
//  3. exceptionPointers->ExceptionRecord->ExceptionCode¥¬ winbase.h and winnt.h ∆ƒ¿œ¬¸¡∂
//     (EXCEPTION_ACCESS_VIOLATION ... CONTROL_C_EXIT µÓ)
//
//  ¡÷¿«: DBGHELP.DLL ∆ƒ¿œ¿« πˆ¿¸¿∫ √÷º“ 5.1.2600.0 ¿ÃªÛ¿ÃæÓæﬂ «—¥Ÿ.
//        (MiniDumpWriteDump «‘ºˆ∞° DBGHELP.DLLø° µÈæÓ ¿÷¿Ω)

LONG __stdcall GenerateDump(EXCEPTION_POINTERS* exceptionPointers)
{
	char       fileName[MAX_PATH];
	SYSTEMTIME st;

	GetLocalTime( &st );
	sprintf_s( fileName
			  ,MAX_PATH
			  ,"LoginServer_%4d%02d%02d_%02d%02d%02d(0x%08X 0x%08p 0x%02X 0x%04X 0x%08X).dmp"
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
//	¥ÌŒÛ¥˙¬Î◊™ªªŒ™œ˚œ¢.
//
//	msgBuf    :¥ÌŒÛ–≈œ¢ (ªÁøÎ»ƒ LocalFree «œø© ¡ÿ¥Ÿ.)
//	lastError :¥ÌŒÛ¥˙¬Î
//	priLangId :primary language identifier
//	subLangId :sublanguage identifier
//
//	πÆ¿⁄∏ﬁΩ√¡ˆ ææÓ¥¬ æ∆∑°øÕ ∞∞¿Ã º≥¡§«—¥Ÿ.
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
	// ∏∏æ‡ lastError∞° NERR_BASE ø°º≠ MAX_NERR ¿œ∞ÊøÏ, netmsg.dll ¿«
	// ∏ﬁΩ√¡ˆ∏¶ ªÁøÎ«—¥Ÿ. (NERR_BASE, MAX_NERR¥¬ lmerr.h ∆ƒ¿œø° ¡§¿«)
	if ( lastError >= NERR_BASE && lastError <= MAX_NERR )
	{
		module = LoadLibraryEx( "netmsg.dll", NULL, LOAD_LIBRARY_AS_DATAFILE );
		if ( module != NULL )
		{
			flags |= FORMAT_MESSAGE_FROM_HMODULE;
		}
	}

	// If lastError is in the Exception Code range, load the message source.
	// ∏∏æ‡ lastError∞° 0xC0000000L ∫∏¥Ÿ ≈¨∞ÊøÏ, ntdll.dll¿« ∏ﬁΩ√¡ˆ∏¶ ªÁøÎ«—¥Ÿ.
	// (winbase.h, winnt.h ∆ƒ¿œø° ¡§¿«)
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

// ConsolCtrlHandler Prototype - Windows Command Processor Control Functions.
BOOL WINAPI ConsolCtrlHandler(DWORD opcode)
{
	if ( g_loginSrv )
		return g_loginSrv->ConsolCtrlHandler( opcode );
	else
		return FALSE;
}

// main Prototype (This method is the standard Windows program entry point.)
int main(int argc, char* argv[])
{
	LPTOP_LEVEL_EXCEPTION_FILTER previousFilter = NULL;
	previousFilter = SetUnhandledExceptionFilter( GenerateDump );

	cLoginSrv* loginSrv = NULL;

	loginSrv = new cLoginSrv( );
	loginSrv->ConsoleRun( argc, argv );

	SafeDelete( loginSrv );

	SetUnhandledExceptionFilter( previousFilter );
	return 0L;
}

// ServiceCtrlHandler Prototype - Windows Service Control Functions.
void WINAPI ServiceCtrlHandler(DWORD opcode)
{
	if ( g_loginSrv )
		g_loginSrv->ServiceCtrlHandler( opcode );
}

// ServiceMain Prototype (This method is the entry point for a service.)
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	cLoginSrv* loginSrv = NULL;

	loginSrv = new cLoginSrv( );
	loginSrv->ServiceRun( argc, argv );

	SafeDelete( loginSrv );
}

// cLoginSrv Constructor.
cLoginSrv::cLoginSrv(void) : mSender(NULL), mSqlWeb(NULL), mSqlAccount(NULL), mLoginProcess(NULL), mRecver(NULL)
{
	// ¥¥Ω®“ª∏ˆ»´æ÷¿‡∂‘œÛ.
	g_loginSrv  = this;

	// ≥ı ºª∑æ≥±‰¡ø.
	mUserName   = NULL;
	mRunService = false;
	mEndService = false;
	mClose      = false;
	mVersion    = MAKEWORD(1, 0);

	memset( mServerAddr,          0, sizeof(mServerAddr)          );
	memset( mServerBroadcastRecv, 0, sizeof(mServerBroadcastRecv) );
	memset( mServerBroadcastSend, 0, sizeof(mServerBroadcastSend) );
}

// ~cLoginSrv Destructor.
cLoginSrv::~cLoginSrv(void)
{
	// cRecver ¿‡ Õ∑≈.
	SafeDelete( mRecver );

	// cLoginProcess ¿‡ Õ∑≈.
	SafeDelete( mLoginProcess );

	// cSQLAccount ¿‡ Õ∑≈.
	SafeDelete( mSqlAccount );

	// cSQLWeb ¿‡ Õ∑≈.
	SafeDelete( mSqlWeb );

	// cRecver ¿‡ Õ∑≈.
	SafeDelete( mSender );

	// »´æ÷¿‡ Õ∑≈.
	g_loginSrv = NULL;
}

// InstallService Method - ∑˛ŒÒ∞≤◊∞∫Ø ˝.
bool cLoginSrv::InstallService(LPCTSTR binaryPathName, DWORD startType)
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
			// ø¿∑˘∏ﬁΩ√¡ˆ √‚∑¬ - CreateService.
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

// UninstallService Method - ∑˛ŒÒ–∂‘ÿ∫Ø ˝.
bool cLoginSrv::UninstallService()
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
				// ø¿∑˘∏ﬁΩ√¡ˆ √‚∑¬ - DeleteService
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
			// ø¿∑˘∏ﬁΩ√¡ˆ √‚∑¬ - OpenService
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
BOOL cLoginSrv::ConsolCtrlHandler(DWORD opcode)
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
void cLoginSrv::ConsoleRun(DWORD argc, LPTSTR* argv)
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
		// ªÁøÎº≥∏Ì.
		printf( "USAGE:\n" );
		printf( "%s [/INSTALL | /UNINSTALL | /INSERT | /TEST]\n\n", fileName );

		printf( "    /INSTALL   Creates a service object and adds it to the specified service\n" );
		printf( "               control manager database.\n" );
		printf( "    /UNINSTALL Marks the specified service for deletion from the service\n" );
		printf( "               control manager database.\n" );
		printf( "    /INSERT    Upload script files to MSSQL Database.\n" );
		printf( "    /TEST      Running the Server Program in Console mode.\n" );
		printf( "    OPTIONS    -VERBOSE  Display Debug Information.\n" );
		printf( "               -PACKET   Display Network Information.\n" );

		// 1. º≠∫ÒΩ∫ «¡∑ŒººΩ∫¿« ∏ﬁ¿Œ æ≤∑πµÂ∏¶ SCM(Service Control Manager)∑Œ ≥—±‚¥¬ «‘ºˆ¥Ÿ.
		//    ¿Ã «‘ºˆ∏¶ ∫Œ∏• æ≤∑πµÂ¥¬ Service Control Dispatcher æ≤∑πµÂ∞° µ»¥Ÿ.
		//
		//	 2. º≠∫ÒΩ∫ «¡∑Œ±◊∑•¿Ã Ω√¿€µ«∏È, SCM¿∫ StartServiceCtrlDispatcher∏¶ ∫“∏Æ±Ê ±‚¥Ÿ∏Æ∞‘ µ»¥Ÿ.
		//     StartServiceCtrlDispatcher∞° º∫∞¯«œ∏È, æ≤∑πµÂ¥¬ SCM ¿Ã ∞¸∏Æ«œ∏Á, º≠∫ÒΩ∫∞° ≥°≥Ø∂ß±Ó¡ˆ ∏Æ≈œµ«¡ˆ æ ¥¬¥Ÿ.
		//     Ω«¡¶ º≠∫ÒΩ∫¿« Ω√¿€¡ˆ¡°¿∫ ¿Ã «‘ºˆ∞° ≥—±‚¥¬ ¿Œ¿⁄∞™¿Œ ServiceMain ¿Ã∂Û∞Ì ∫ººˆ ¿÷¥Ÿ.
		//
		//	 !. SERVICE_TABLE_ENTRY∏¶ µÓ∑œ«ÿæﬂ∏∏ ServiceMain¿Ã »£√‚ µ»¥Ÿ.
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
		// º≠∫ÒΩ∫∑Œ º≥ƒ°∏¶ Ω√µµ«’¥œ¥Ÿ.
		printf( "\nInstallation to your service is initiated.\n" );

		if ( InstallService( moduleName ) )
		{
			// º≠∫ÒΩ∫∑Œ º≥ƒ°∏¶ øœ∑· «œø¥Ω¿¥œ¥Ÿ.
			printf( "\nInstallation to your service is completed.\n" );
		}
		else
		{
			// º≠∫ÒΩ∫∑Œ º≥ƒ°∏¶ øœ∑·«œ¡ˆ ∏¯ «œø¥Ω¿¥œ¥Ÿ.
			printf( "\nInstallation to your service is not completed.\n" );
		}
	}
	else if ( !_stricmp( argv[1], "/UNINSTALL" ) )
	{
		// º≠∫ÒΩ∫∑Œ ªË¡¶∏¶ Ω√µµ«’¥œ¥Ÿ.
		printf( "\nDeleting your service is initiated.\n" );

		if ( UninstallService( ) )
		{
			// º≠∫ÒΩ∫∑Œ ªË¡¶∏¶ øœ∑· «œø¥Ω¿¥œ¥Ÿ.
			printf( "\nDeleting your service is completed.\n" );
		}
		else
		{
			// º≠∫ÒΩ∫∑Œ ªË¡¶∏¶ øœ∑·«œ¡ˆ ∏¯ «œø¥Ω¿¥œ¥Ÿ.
			printf( "\nDeleting our service is not completed.\n" );
		}
	}
	else if ( !_stricmp( argv[1], "/INSERT" ) )
	{
		cSQLAccount* sqlAccount = new cSQLAccount( );
		int          errorCount = 0;

		printf( "\n=============== INSERT START ===============\n" );

		if ( sqlAccount->InsertCaptcha( ODBC_DSN_ACCOUNT, ODBC_UID, ODBC_PWD ) )
		{
			printf( "\nTB_CAPTCHA table has been completed.\n" );
		}
		else
		{
			printf( "TB_CAPTCHA table was not complete!!!\n" );
			printf( "Press Any Key to Continue...\n" );
			_getch( );
			errorCount += 1;
		}

		printf( "\n=============== Completed ===============\n" );
		printf( "\nINSERT - ERROR(=%d)\n", errorCount );
		printf( "\nPress Any Key to Continue." );
		_getch( );

		SafeDelete( sqlAccount );
	}
	else if ( !_stricmp( argv[1], "/TEST" ) )
	{
		DWORD status;
		DWORD error;

		if ( SetConsoleCtrlHandler( ::ConsolCtrlHandler, TRUE ) != 0 )
		{
			// ≈◊Ω∫∆Æ ø…º« ∞ÀªÁ.
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

			// øÿ÷∆Ã®≤‚ ‘.
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
void cLoginSrv::ServiceCtrlHandler(DWORD opcode)
{
	switch ( opcode )
	{
	case SERVICE_CONTROL_STOP:
		// ¡æ∑·«“ ƒ⁄µÂ¥¬ Destroy «‘ºˆ∏¶ ªÁøÎ.
		Destroy( );

		// Notifies a service that it should stop.
		mServiceStatus.dwWin32ExitCode = 0;
		mServiceStatus.dwCurrentState  = SERVICE_STOPPED;
		mServiceStatus.dwCheckPoint    = 0;
		mServiceStatus.dwWaitHint      = 0;
		// Updates the service control manager's status information for the calling service.
		SetServiceStatus( mServiceStatusHandle, &mServiceStatus );
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
void cLoginSrv::ServiceRun(DWORD argc, LPTSTR* argv)
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

	// º≠∫ÒΩ∫ ¡¶æÓ √≥∏Æ¿⁄∏¶ µÓ∑œ«—¥Ÿ. - Ω«∆–Ω√ 0∏¶ π›»Ø «—¥Ÿ.
	mServiceStatusHandle = RegisterServiceCtrlHandler( g_serviceName, ::ServiceCtrlHandler );
	if ( mServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 )
		return;

	// Initialization code goes here.
	// - √ ±‚»≠ «“ ƒ⁄µÂ¥¬ ServiceInitialization «‘ºˆ∏¶ ªÁøÎ
	// - Ω«∆–Ω√ NO_ERROR(0)∞° æ∆¥— ∞™∏¶ π›»Ø.
	status = Initialization( argc, argv, &error );	

	// Handle error condition - √ ±‚»≠ Ω«∆– «“ ∞ÊøÏ ¡æ∑·ƒ⁄µÂ(status)∏¶ «•Ω√«ÿ ¡ÿ¥Ÿ.
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
	// - √ ±‚»≠ øœ∑· - º≠∫ÒΩ∫ ªÛ≈¬∏¶ µø¿€(RUNNING)¿∏∑Œ º≥¡§.
	mServiceStatus.dwCurrentState = SERVICE_RUNNING;
	mServiceStatus.dwCheckPoint   = 0;
	mServiceStatus.dwWaitHint     = 0;

	// Updates the service control manager's status information for the calling service.
	if ( SetServiceStatus( mServiceStatusHandle, &mServiceStatus ) == FALSE )
		return;

	// Run - º≠∫ÒΩ∫ Ω√¿€.
	Run( );
}

// Initialization Method (Stub initialization function.)
// ¡÷¿ß: √ ±‚»≠ π◊ ª˝º∫ º¯º≠∏¶ ∫Ø∞Ê«“ ºˆ æ¯¿Ω.
DWORD cLoginSrv::Initialization(DWORD argc, LPTSTR* argv, DWORD* error)
{
	// WINSOCK2 √ ±‚»≠ (πˆ¿¸ 2.2).
	if ( LOBYTE( m_wsaData.wVersion ) != 2 || HIBYTE( m_wsaData.wVersion ) != 2 )
		return 1L;

	// ªÁøÎ¿⁄ ¡§∫∏.
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

	// »£Ω∫∆Æ ¡§∫∏.
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

	// ªÁøÎ¿⁄ & »£Ω∫∆Æ ¡§∫∏√‚∑¬.
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

	// Ω««‡»Ø∞Ê º≥¡§.
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

	// º≠∫ÒΩ∫ & ƒ‹º÷ & µπˆ±◊ ∏µÂ »£»Øº∫ ¿˚øÎ.
	if ( GetFileAttributes( filename ) == INVALID_FILE_ATTRIBUTES )
	{
		GetCurrentDirectory( sizeof(directory), directory );
		sprintf( filename, "%s\\%s.cfg", directory, fname );
	}

	// Ω««‡∆ƒ¿œ∞Ê∑Œ √‚∑¬.
	if ( g_verbose == true )
	{
		printf( "Binary Path Name.....: %s%s%s%s \n", drive, dir, fname, ext );
		printf( "Working Directory....: %s \n", directory );
		printf( "Module Path Name.....: %s \n", filename );
		printf( "\n" );
	}

	FILE* stream = fopen( filename, "rt" );
	char  buffer[MAX_PATH];

	char  seps[] = " \t\n";			// ∆ƒΩÃ±∏πÆ Parsing.
	char *token;

	if ( stream != NULL )
	{
		try {
			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGIN_SERVER_ADDRESS(IPv4):" ) != 0 ) throw false;
			sprintf( mServerAddr, "%s", strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGIN_BROADCAST_RECV(IPv4):" ) != 0 ) throw false;
			sprintf( mServerBroadcastRecv, "%s", strtok( NULL, seps ) );

			if ( fgets( buffer, MAX_PATH, stream ) == NULL ) throw false;

			token = strtok( buffer, seps );
			if ( token && stricmp( token, "LOGIN_BROADCAST_SEND(IPv4):" ) != 0 ) throw false;
			sprintf( mServerBroadcastSend, "%s", strtok( NULL, seps ) );

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
		printf( "\nLogin Server Address..: %s", mServerAddr );
		printf( "\nLogin Broadcast Recv..: %s", mServerBroadcastRecv );
		printf( "\nLogin Broadcast Send..: %s", mServerBroadcastSend );
		printf( "\nLogin Port............: %d", g_loginPort );
		printf( "\nGame Port.............: %d", g_gamePort );
		printf( "\nLog Port..............: %d", g_logPort );
		printf( "\n" );
	}

	/*-- ODBC-DNS & WINSOCK2 ≥ı ºªØ --*/

	// …Ë÷√π§◊˜ƒø¬º.
	if ( SetCurrentDirectory( directory ) == false )
		return 1L;

	// cSender ¥¥Ω®.
	mSender = new cSender( );
	if ( mSender->Initialize( mServerBroadcastSend, mServerBroadcastSend, 2, 32768 ) == false )
		return 1L;

	// cSQLWeb ¥¥Ω®.
	mSqlWeb = new cSQLWeb( );
	if ( mSqlWeb->Initialize( ODBC_DSN_WEB, ODBC_UID_WEB, ODBC_PWD_WEB ) == false )
		return 1L;

	// cSQLAccount ¥¥Ω®.
	mSqlAccount = new cSQLAccount( );
	if ( mSqlAccount->Initialize( ODBC_DSN_ACCOUNT, ODBC_UID, ODBC_PWD ) == false )
		return 1L;

	// cLoginProcess ¥¥Ω®.
	mLoginProcess = new cLoginProcess( );
	if ( mLoginProcess->Initialize( mServerAddr, T_LOGIN_CPORT ) == false )
		return 1L;

	// cSender ¥¥Ω®.
	mRecver = new cRecver( );
	if ( mRecver->Initialize( mServerBroadcastRecv, g_loginPort, 2, 32768 ) == false )
		return 1L;

	return 0L;
}

// Destroy Method
// ¡÷¿ß: ¡æ∑· º¯º≠∏¶ ∫Ø∞Ê «“ ºˆ æ¯¿Ω.
void cLoginSrv::Destroy(void)
{
	if ( mEndService == false )
	{
		mEndService = true;				// º≠∫ÒΩ∫ ¡æ∑·.

		if ( mRecver != NULL )
		{
			mRecver->Shutdown( );		// cRecver ¡æ∑·.
		}
		if ( mLoginProcess != NULL )
		{
			mLoginProcess->Shutdown( );	// cLoginProcess ¡æ∑·.
		}
		if ( mSqlAccount != NULL )
		{
			mSqlAccount->Shutdown( );	// cSQLAccount ¡æ∑·.
		}
		if ( mSqlWeb != NULL )
		{
			mSqlWeb->Shutdown( );		// cSQLWeb ¡æ∑·.
		}
		if ( mSender != NULL )
		{
			mSender->Shutdown( );		// cSender ¡æ∑·.
		}
		if ( mUserName != NULL )
		{
			GlobalFree( mUserName );	// ∏ﬁ∏∏Æ «ÿ¡¶.
			mUserName = NULL;
		}
		mRunService = false;			// Ω««‡¡æ∑·.
	}
}

// Close Method
void cLoginSrv::Close( )
{
	mClose = true;
}

// Run Method
void cLoginSrv::Run(void)
{
	DWORD currentTick = 0;
	DWORD elapsedTick = 0;

	// º≠∫ÒΩ∫∞° Ω√¿€ µ .
	mRunService = true;
	while ( mRunService )
	{
		currentTick = GetTickCount( );

		if ( mClose == true )
			Destroy( );

		// ¡ˆø¨Ω√∞£ æ‡100(ms) - CPU ∞˙∫Œ»≠ πÊ¡ˆ & ≥ª∫Œ µø±‚»≠.
		elapsedTick = GetTickCount( ) - currentTick;
		if ( elapsedTick < 100 )
		{
			Sleep( (100 - elapsedTick) );
		}
	}
}
