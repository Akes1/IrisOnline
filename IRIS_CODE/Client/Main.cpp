#include "stdafx.h"
#include "resource.h"

#include "MiniDumper.h"
#include "Application.h"
//#include "HShield.h"
#include <shellapi.h>

#define GAMEEXE_PARAM				"+Iris1000000"
#define LAUNCHER_FILENAME			L".\\IrisLauncher.exe"
#define DEV_LAUNCHER_FILENAME		L".\\IrisLauncher_DEV.exe"

#define HS_GAMECODE					5831
#define HS_LICENSE					L"CBC1EE5976E5748C02F7AF5D"

#define MAX_STRING					100
#define szTitle						"ERROR"

DWORD  g_dwMainThreadID;
char	g_szMsg[255] = {0,};
bool	g_ErrorHackShield = false;


void ShowShellExecuteError( int code )
{
	cString result;

	switch( code )
	{
	case 0:							result="The operating system is out of memory or resources."; break;
	case ERROR_FILE_NOT_FOUND:		result="The operating system is out of memory or resources."; break;
	case ERROR_PATH_NOT_FOUND:		result="The specified path was not found."; break;
	case ERROR_BAD_FORMAT:			result="The .EXE file is invalid (non-Win32 .EXE or error in .EXE image)."; break;
	case SE_ERR_ACCESSDENIED:		result="The operating system denied access to the specified file."; break;
	case SE_ERR_ASSOCINCOMPLETE:	result="The filename association is incomplete or invalid."; break;
	case SE_ERR_DDEBUSY:			result="The DDE transaction could not be completed because other DDE transactions were being processed."; break;
	case SE_ERR_DDEFAIL:			result="The DDE transaction failed."; break;
	case SE_ERR_DDETIMEOUT:			result="The DDE transaction could not be completed because the request timed out."; break;
	case SE_ERR_DLLNOTFOUND:		result="The specified dynamic-link library was not found."; break;
	case SE_ERR_NOASSOC:			result="Unbekannter Extender."; break;
	case SE_ERR_OOM:				result="There was not enough memory to complete the operation."; break;
	case SE_ERR_SHARE:				result="A sharing violation occurred."; break;
	default:						result="Invalid error"; break;
	}

	NiMessageBox( result.Cstr(), "Launcher execute error." );
}

int RunLauncher()
{
	cStringT path;

#if defined(_GMTOOL)	
	path.Format( _T(".\\%s"), DEV_LAUNCHER_FILENAME );
#else
	path.Format( _T(".\\%s"), LAUNCHER_FILENAME );
#endif

	int ret = (int)::ShellExecute( 0, L"open", path.Cstr(), 0, 0, SW_SHOWNORMAL );
	if( ret <= 32 )
		ShowShellExecuteError(ret);

	return 0;
}

/* ------------------------------------------------------------------
 * 함수이름 :	WinMain(HINSTANCE hI, HINSTANCE hPI, LPSTR cmdline, int iWinMode )
 * 목    적 :	어플리케이션 메인 함수
 * 주의사항 :	
 * ------------------------------------------------------------------ */
int WINAPI WinMain(HINSTANCE hI, HINSTANCE /* hPI */, LPSTR cmdline, int /* iWinMode */ )
{
	/// 버그 트랙킹..vb
	cMiniDump::Install(cMiniDump::DUMP_LEVEL_MEDIUM);

	/// 타이머의 해상도를 조절할 수 있다. 1ms단위로 조절한다.(OS마다 다를수 있는 부분을 없앤다.)
	timeBeginPeriod(1);

	//////////////////////////////////////////////////////////////////////////
	/// 언어별 폴더 지정
	unsigned int countryCode = 0;

	switch( countryCode )
	{
	case 82:
		cApplication::mLangaugeFolderW = L"korea";
		cApplication::mLangaugeFolder = "korea";
		break;
	case 83:
		cApplication::mLangaugeFolderW = L"japan";
		cApplication::mLangaugeFolder = "japan";
		break;
	default:
		cApplication::mLangaugeFolderW = L"runup_c";
		cApplication::mLangaugeFolder = "runup_c";
		break;
	}
	//////////////////////////////////////////////////////////////////////////

	cString ip;
	/// 게임을 실행시키기 위한 환경에 대한 검사루틴들..
#if defined(NISHIPPING) && !defined(_DEVSYS) //&& !defined(_DEVMODE)
	/// 실행 인자 검사
	if( ::strlen( cmdline ) == 0 )
		return RunLauncher();

	if( cApplication::CheckExecutionApplication() == FALSE )
		return 0;
#endif
	//////////////////////////////////////////////////////////////////////////


    /// 3543712878 - 211.56.252.46 
    /// 3672778883 - 218.234.16.195
    /// 암호화된 IP
    unsigned long uIP         = strtoul( cmdline, NULL, 10 );
    unsigned long reverseIP   = ntohl( uIP );
    unsigned long reversePort = htonl( 14400 );
    char*         loginIP = NULL;
    in_addr       addr;    
    
    reverseIP    ^= reversePort;
    addr.s_addr  = reverseIP;

    loginIP = inet_ntoa( addr );
	ip.Format( "%s", loginIP );

	if( ip.IsEmpty() )
	{
		NiMessageBox( "Wrong execute....\n\nShutdown Application!! Bye..", "Error Info" );
		return 0;
	}

	/// DirectX Version Check
	if( cApplication::CheckDirectXVersion() == false) 
		return 0;

	/// HardDisk Memory Check
	if( cApplication::CheckEnoughHardDisk() == false )
		return 0;

	/// Physical & Virtual Memory Check 현재 138ms정도
	if( cApplication::CheckEnoughMemory() == false )
		return 0;
	//////////////////////////////////////////////////////////////////////////

	const DWORD LANGUAGE_KOREAN  = 0x0412;
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if ( GetVersionEx( &osv ) == FALSE )
	{
		NiMessageBox( "Fail to Get OS Information...","Error Info" );
		return 0;
	}
	unsigned (__stdcall* SetThreadUILanguage)(LANGID) = 0;
	HINSTANCE ins = NULL;

	LCID systemLang = 0;
	systemLang = GetThreadLocale();
	switch( systemLang )
	{
	case LANGUAGE_KOREAN: 
		SetThreadLocale( MAKELCID( MAKELANGID( LANG_KOREAN , SUBLANG_KOREAN ) , SORT_DEFAULT)); 
		break;

	default:
		{
			if ( osv.dwMajorVersion > 5 )
			{
				ins = LoadLibrary( _T("KERNEL32.DLL") );
				if ( ins != NULL )
				{
					SetThreadUILanguage = (unsigned (__stdcall*)(LANGID))GetProcAddress( ins, "SetThreadUILanguage" );
					if ( SetThreadUILanguage != NULL )
						SetThreadUILanguage( MAKELANGID( LANG_ENGLISH , SUBLANG_ENGLISH_US ) );
				}
			}
			else
			{
				SetThreadLocale( MAKELCID( MAKELANGID( LANG_ENGLISH , SUBLANG_ENGLISH_US ) , SORT_DEFAULT)); 
			}

		}
		break;
	}

	//////////////////////////////////////////////////////////////////////////
	/// Begin Application Root
	///
	/// Gamebryo 스마트 포인트 사용객체에 대한 체크
#ifdef	_DEBUG
	unsigned int initialCount = NiRefObject::GetTotalObjectCount();
#endif

	/// Static Data Manager 초기화 : library 들을 요구한다. 즉 엔진 시스템의 load라고 보여짐..
	NiInitOptions option;
	option.SetFloodgateParallelExecution( false );
	NiStaticDataManager::Init( &option );
	//	NiInit();

	cApplication* pTheApp = cApplication::Create();
	if( pTheApp == NULL )
	{
		NiShutdown();
		timeEndPeriod( 1 );

		if ( ins != NULL )
		{
			SetThreadUILanguage( (LANGID)MAKELCID( systemLang , SORT_DEFAULT ));
			FreeLibrary( ins );
		}
		else
		{
			SetThreadLocale( MAKELCID( systemLang , SORT_DEFAULT )); 
		}

		return 0;
	}

	if( pTheApp->Init( hI, ip, -1 ) )
	{
		/// Main Loop..
		pTheApp->Run();
	}

	pTheApp->Exit();

	if( g_ErrorHackShield )
	{
		::ShowWindow( pTheApp->GetHWND(), false );
		MessageBoxA( NULL, g_szMsg, "ERROR", MB_OK | MB_TOPMOST);
	}

	SAFE_DELETE( pTheApp );

	if ( ins != NULL )
	{
		SetThreadUILanguage( (LANGID)MAKELCID( systemLang , SORT_DEFAULT ));
		FreeLibrary( ins );
	}
	else
	{
		SetThreadLocale( (LANGID)MAKELCID( systemLang , SORT_DEFAULT )); 
	}
	///
	/// End Application Root
	//////////////////////////////////////////////////////////////////////////

	/// load된 library에 대한 해제
	NiShutdown();

#ifdef	_DEBUG
	unsigned int finalCount = NiRefObject::GetTotalObjectCount();

	TCHAR msg[256] = {0,};
	_stprintf( msg, _T("\nGamebryo NiRefObject counts:  initial = %u, final = %u. "), initialCount, finalCount );
	OutputDebugString( msg );

	if( finalCount > initialCount )
	{
		unsigned int diff = finalCount - initialCount;
		_stprintf( msg, _T("Application is leaking %u objects\n\n"), diff );
		OutputDebugString( msg );
	}
#endif

	/// 해상도 조절한 부분 제거..
	timeEndPeriod(1); 

	return 0;
}
