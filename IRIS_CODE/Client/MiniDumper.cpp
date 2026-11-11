#include "stdafx.h"
#include "MiniDumper.h"
#include <dbghelp.h>

#ifndef Assert
#include <assert.h>
#define Assert assert
#define LogToFile (void)(0);
#endif


#define ASIZE(A)	(sizeof(A)/sizeof((A)[0]))

cMiniDump::DumpLevel cMiniDump::ms_DumpLevel          = cMiniDump::DUMP_LEVEL_LIGHT;
bool                 cMiniDump::ms_AddTimeStamp       = true;
TCHAR                cMiniDump::ms_AppName[_MAX_PATH] = {0,};
TCHAR                cMiniDump::ms_CallStack[8192]    = {0,};
TCHAR                cMiniDump::ms_Modules[8192]      = {0,};
LPCTSTR              cMiniDump::ms_DialogTemplate     = NULL;
DLGPROC              cMiniDump::ms_DialogProc         = NULL;

namespace
{
	/// \brief 예외의 원인에 대한 문자열을 반환한다.
	LPCTSTR GetFaultReason(PEXCEPTION_POINTERS exPtrs);

	/// \brief 사용자 정보를 반환한다.
	LPCTSTR GetUserInfo();

	/// \brief 윈도우즈 버전을 반환한다.
	LPCTSTR GetOSInfo();

	/// \brief CPU 정보를 반환한다.
	LPCTSTR GetCpuInfo();

	/// \brief 메모리 정보를 반환한다.
	LPCTSTR GetMemoryInfo();

	/// \brief 윈도우즈 버전을 알아낸다.
	bool GetWinVersion(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild);

	/// \brief strrchr TCHAR 버전
	TCHAR* lstrrchr(TCHAR* str, TCHAR ch);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief 미니 덤프 기능을 초기화한다.
/// \param dumpLevel 덤프 레벨
/// \param addTimeStamp 덤프 파일 이름에다가 덤프 파일이 생성된 날짜를
/// 집어넣는가의 여부.
/// \param dialogTemplate 대화창 템플릿
/// \param dialogProc 대화창 프로시져
////////////////////////////////////////////////////////////////////////////////
void cMiniDump::Install(
						DumpLevel dumpLevel, bool addTimeStamp, LPCTSTR dialogTemplate, DLGPROC dialogProc)
{
	Assert(ms_AppName[0] == 0);
	Assert(dumpLevel >= DUMP_LEVEL_LIGHT);
	Assert(dumpLevel <= DUMP_LEVEL_HEAVY);

	ms_DumpLevel      = dumpLevel;
	ms_AddTimeStamp   = addTimeStamp;
	ms_DialogTemplate = dialogTemplate;
	ms_DialogProc     = dialogProc;

	// 모듈 경로를 알아낸다.
	// C:\somewhere\something.exe
	TCHAR szFileName[_MAX_PATH];
	::GetModuleFileName(NULL, szFileName, _MAX_PATH);

	// 확장자를 제거한 모듈 경로를 준비해둔다.
	// C:\somewhere\something.exe -> C:\somewhere\something
	TCHAR* dot = lstrrchr(szFileName, '.');
	::lstrcpyn(ms_AppName, szFileName, (int)(dot - szFileName + 1));

	dot = ::_tcsrchr(ms_AppName, _T('\\'));
	::lstrcpyn(ms_AppName, &dot[1], ::_tcslen(ms_AppName) );

	// 예외 처리 핸들러를 설정한다.
	::SetUnhandledExceptionFilter(WriteDump);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief 예외에 대한 정보를 받아서, 미니 덤프 파일을 생성한다. 
/// 
/// SetUnhandledExceptionFilter() API에 의해서 설정되고, 프로세스 내부에서 
/// Unhandled Exception이 발생될 경우, 호출되게 된다. 단 디버거가 붙어있는 
/// 경우, Unhandled Exception Filter는 호출되지 않는다. 이 말은 이 함수 
/// 내부를 디버깅할 수는 없다는 말이다. 이 함수 내부를 디버깅하기 위해서는 
/// 메시지 박스 또는 파일을 이용해야한다.
/// 
/// \param exPtrs 예외 정보
/// \return LONG 이 함수를 실행하고 난 다음, 취할 행동값. 자세한 것은 SEH
/// 문서를 참고하도록.
////////////////////////////////////////////////////////////////////////////////
LONG WINAPI cMiniDump::WriteDump(PEXCEPTION_POINTERS exPtrs)
{
	// based on dbghelp.h
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
		HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);

#ifdef UNICODE
	fwprintf(stderr, 
		L"=============================================\n"
		L"unhandled excetpion triggerd! writing dump...\n"
		L"=============================================\n"
		);
#else
	fprintf(stderr, 
		"=============================================\n"
		"unhandled excetpion triggerd! writing dump...\n"
		"=============================================\n"
		);
#endif

	// 대화창이 설정되어 있다면 보여준다.
	if (ms_DialogTemplate != NULL && ms_DialogProc != NULL)
	{
		if (DialogBox(NULL, ms_DialogTemplate, HWND_DESKTOP, ms_DialogProc) != IDOK)
			return EXCEPTION_EXECUTE_HANDLER;
	}

	HMODULE hDLL = NULL;
	TCHAR szDbgHelpPath[_MAX_PATH] = {0, };
	TCHAR szDumpPath[MAX_PATH * 2] = {0,};

	// 덤프 파일 이름 += 시간 문자열
	::lstrcat(szDumpPath, ms_AppName);
	if (ms_AddTimeStamp)
	{
		SYSTEMTIME t;
		::GetLocalTime(&t);

		TCHAR szTail[_MAX_PATH];
#ifdef UNICODE
		swprintf(szTail, ASIZE(szTail)-1, 
			L" %04d-%02d-%02d %02d-%02d-%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#else
		_snprintf(szTail, ASIZE(szTail)-1, _TRUNCATE,
			" %04d-%02d-%02d %02d-%02d-%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#endif

		::lstrcat(szDumpPath, szTail);
	}
	::lstrcat(szDumpPath, _T(".dmp"));

	// 먼저 실행 파일이 있는 디렉토리에서 DBGHELP.DLL을 로드해 본다.
	// Windows 2000 의 System32 디렉토리에 있는 DBGHELP.DLL 파일은 버전이 
	// 오래된 것일 수 있기 때문이다. (최소 5.1.2600.0 이상이어야 한다.)
	if (::GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH))
	{
		if (LPTSTR slash = ::lstrrchr(szDbgHelpPath, '\\'))
		{
			::lstrcpy(slash + 1, _T("DBGHELP.DLL"));
			hDLL = ::LoadLibrary(szDbgHelpPath);
		}
	}

	// 현재 디렉토리에 없다면, 아무 버전이나 로드한다.
	if (hDLL == NULL) hDLL = ::LoadLibrary(_T("dbghelp.dll"));

	// DBGHELP.DLL을 찾을 수 없다면 더 이상 진행할 수 없다.
	if (hDLL == NULL)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// DLL 내부에서 MiniDumpWriteDump API를 찾는다.
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
		(MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");
	if (pfnMiniDumpWriteDump == NULL)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// 파일을 생성한다.
	HANDLE hFile = ::CreateFile(
		szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	MINIDUMP_EXCEPTION_INFORMATION ExParam;
	ExParam.ThreadId = ::GetCurrentThreadId();
	ExParam.ExceptionPointers = exPtrs;
	ExParam.ClientPointers = FALSE;

	MINIDUMP_TYPE dumptype = MiniDumpNormal;
	switch (ms_DumpLevel)
	{
	case DUMP_LEVEL_LIGHT:  dumptype = MiniDumpNormal;         break;
	case DUMP_LEVEL_MEDIUM: dumptype = MiniDumpWithDataSegs;   break;
	case DUMP_LEVEL_HEAVY:  dumptype = MiniDumpWithFullMemory; break;
	}

	// 덤프 파일 생성 결과를 로그 파일에다 기록한다.
	if (pfnMiniDumpWriteDump(
		::GetCurrentProcess(), ::GetCurrentProcessId(), 
		hFile, dumptype, &ExParam, NULL, NULL) == 0)
	{
		Assert(false);
	}

	::CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

namespace
{
	/// \brief 예외의 원인에 대한 문자열을 반환한다.
	/// \param exPtrs 예외 구조체 포인터
	/// \return LPCTSTR 원인 문자열
	LPCTSTR GetFaultReason(PEXCEPTION_POINTERS exPtrs)
	{
		if (::IsBadReadPtr(exPtrs, sizeof(EXCEPTION_POINTERS))) 
			return _T("bad exception pointers");

		// 간단한 에러 코드라면 그냥 변환할 수 있다.
		switch (exPtrs->ExceptionRecord->ExceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:         return _T("EXCEPTION_ACCESS_VIOLATION");
		case EXCEPTION_DATATYPE_MISALIGNMENT:    return _T("EXCEPTION_DATATYPE_MISALIGNMENT");
		case EXCEPTION_BREAKPOINT:               return _T("EXCEPTION_BREAKPOINT");
		case EXCEPTION_SINGLE_STEP:              return _T("EXCEPTION_SINGLE_STEP");
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		case EXCEPTION_FLT_DENORMAL_OPERAND:     return _T("EXCEPTION_FLT_DENORMAL_OPERAND");
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
		case EXCEPTION_FLT_INEXACT_RESULT:       return _T("EXCEPTION_FLT_INEXACT_RESULT");
		case EXCEPTION_FLT_INVALID_OPERATION:    return _T("EXCEPTION_FLT_INVALID_OPERATION");
		case EXCEPTION_FLT_OVERFLOW:             return _T("EXCEPTION_FLT_OVERFLOW");
		case EXCEPTION_FLT_STACK_CHECK:          return _T("EXCEPTION_FLT_STACK_CHECK");
		case EXCEPTION_FLT_UNDERFLOW:            return _T("EXCEPTION_FLT_UNDERFLOW");
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
		case EXCEPTION_INT_OVERFLOW:             return _T("EXCEPTION_INT_OVERFLOW");
		case EXCEPTION_PRIV_INSTRUCTION:         return _T("EXCEPTION_PRIV_INSTRUCTION");
		case EXCEPTION_IN_PAGE_ERROR:            return _T("EXCEPTION_IN_PAGE_ERROR");
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return _T("EXCEPTION_ILLEGAL_INSTRUCTION");
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
		case EXCEPTION_STACK_OVERFLOW:           return _T("EXCEPTION_STACK_OVERFLOW");
		case EXCEPTION_INVALID_DISPOSITION:      return _T("EXCEPTION_INVALID_DISPOSITION");
		case EXCEPTION_GUARD_PAGE:               return _T("EXCEPTION_GUARD_PAGE");
		case EXCEPTION_INVALID_HANDLE:           return _T("EXCEPTION_INVALID_HANDLE");
			//case EXCEPTION_POSSIBLE_DEADLOCK:        return _T("EXCEPTION_POSSIBLE_DEADLOCK");
		case CONTROL_C_EXIT:                     return _T("CONTROL_C_EXIT");
		case 0xE06D7363:                         return _T("Microsoft C++ Exception");
		default:
			break;
		}

		// 뭔가 좀 더 복잡한 에러라면...
		static TCHAR szFaultReason[2048];
		::lstrcpy(szFaultReason, _T("Unknown")); 
		::FormatMessage(
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
			::GetModuleHandle(_T("ntdll.dll")),
			exPtrs->ExceptionRecord->ExceptionCode, 
			0,
			szFaultReason,
			0,
			NULL);

		return szFaultReason;
	}

	/// \brief 사용자 정보를 반환한다.
	/// \return LPCTSTR 사용자 이름
	LPCTSTR GetUserInfo()
	{
		static TCHAR szUserName[200] = {0,};

		ZeroMemory(szUserName, sizeof(szUserName));
		DWORD UserNameSize = ASIZE(szUserName) - 1;

		if (!::GetUserName(szUserName, &UserNameSize))
			::lstrcpy(szUserName, _T("Unknown"));

		return szUserName;
	}

	/// \brief 윈도우즈 버전을 반환한다.
	/// \return LPCTSTR 윈도우즈 버전 문자열
	LPCTSTR GetOSInfo()
	{
		TCHAR szWinVer[50] = {0,};
		TCHAR szMajorMinorBuild[50] = {0,};
		int nWinVer = 0;
		::GetWinVersion(szWinVer, &nWinVer, szMajorMinorBuild);

		static TCHAR szOSInfo[512] = {0,};
#ifdef UNICODE
		swprintf(szOSInfo, ASIZE(szOSInfo)-1, L"%s (%s)",
			szWinVer, szMajorMinorBuild);
#else
		_snprintf(szOSInfo, ASIZE(szOSInfo)-1, _TRUNCATE, "%s (%s)",
			szWinVer, szMajorMinorBuild);
#endif
		szOSInfo[ASIZE(szOSInfo)-1] = 0;
		return szOSInfo;
	}

	/// \brief CPU 정보를 반환한다.
	/// \return LPCTSTR CPU 정보 문자열
	LPCTSTR GetCpuInfo()
	{
		// CPU 정보 기록
		SYSTEM_INFO    SystemInfo;
		GetSystemInfo(&SystemInfo);

		static TCHAR szCpuInfo[512] = {0,};
#ifdef UNICODE
		swprintf(szCpuInfo, ASIZE(szCpuInfo)-1, 
			L"%d processor(s), type %d",
			SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);
#else
		_snprintf(szCpuInfo, ASIZE(szCpuInfo)-1, _TRUNCATE, 
			"%d processor(s), type %d",
			SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);
#endif
		return szCpuInfo;
	}

	/// \brief 메모리 정보를 반환한다.
	/// \return LPCTSTR 메모리 정보 문자열
	LPCTSTR GetMemoryInfo()
	{
		static const int ONE_K = 1024;
		static const int ONE_M = ONE_K * ONE_K;
		static const int ONE_G = ONE_K * ONE_K * ONE_K;

		MEMORYSTATUS MemInfo;
		MemInfo.dwLength = sizeof(MemInfo);
		GlobalMemoryStatus(&MemInfo);

		static TCHAR szMemoryInfo[2048] = {0,};
#ifdef UNICODE
		swprintf(szMemoryInfo, ASIZE(szMemoryInfo)-1, 
			L"%d%% of memory in use.\n"
			L"%d MB physical memory.\n"
			L"%d MB physical memory free.\n"
			L"%d MB paging file.\n"
			L"%d MB paging file free.\n"
			L"%d MB user address space.\n"
			L"%d MB user address space free.",
			MemInfo.dwMemoryLoad, 
			(MemInfo.dwTotalPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalVirtual + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailVirtual + ONE_M - 1) / ONE_M);
#else
		_snprintf(szMemoryInfo, ASIZE(szMemoryInfo)-1, _TRUNCATE,
			"%d%% of memory in use.\n"
			"%d MB physical memory.\n"
			"%d MB physical memory free.\n"
			"%d MB paging file.\n"
			"%d MB paging file free.\n"
			"%d MB user address space.\n"
			"%d MB user address space free.",
			MemInfo.dwMemoryLoad, 
			(MemInfo.dwTotalPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalVirtual + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailVirtual + ONE_M - 1) / ONE_M);
#endif

		return szMemoryInfo;
	}

	/// \brief 윈도우즈 버전을 알아낸다.
	///
	/// This table has been assembled from Usenet postings, personal observations, 
	/// and reading other people's code.  Please feel free to add to it or correct 
	/// it.
	///
	/// <pre>
	/// dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
	/// 95            1               4                 0            950
	/// 95 SP1        1               4                 0            >950 && <=1080
	/// 95 OSR2       1               4               <10            >1080
	/// 98            1               4                10            1998
	/// 98 SP1        1               4                10            >1998 && <2183
	/// 98 SE         1               4                10            >=2183
	/// ME            1               4                90            3000
	///
	/// NT 3.51       2               3                51
	/// NT 4          2               4                 0            1381
	/// 2000          2               5                 0            2195
	/// XP            2               5                 1            2600
	/// 2003 Server   2               5                 2            3790
	///
	/// CE            3
	/// </pre>
	///
	/// \param pszVersion 버전 문자열을 집어넣을 포인터
	/// \param nVersion 버전 숫자값을 집어넣을 포인터
	/// \param pszMajorMinorBuild 빌드 문자열을 집어넣을 포인터
	/// \return bool 무사히 실행한 경우에는 true, 뭔가 에러가 생긴 경우에는 false
	bool GetWinVersion(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild)
	{
		// from winbase.h
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s 0
#endif

#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif

#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT 2
#endif

#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE 3
#endif

		static LPCTSTR WUNKNOWNSTR     = _T("Unknown Windows Version");
		static LPCTSTR W95STR          = _T("Windows 95");
		static LPCTSTR W95SP1STR       = _T("Windows 95 SP1");
		static LPCTSTR W95OSR2STR      = _T("Windows 95 OSR2");
		static LPCTSTR W98STR          = _T("Windows 98");
		static LPCTSTR W98SP1STR       = _T("Windows 98 SP1");
		static LPCTSTR W98SESTR        = _T("Windows 98 SE");
		static LPCTSTR WMESTR          = _T("Windows ME");
		static LPCTSTR WNT351STR       = _T("Windows NT 3.51");
		static LPCTSTR WNT4STR         = _T("Windows NT 4");
		static LPCTSTR W2KSTR          = _T("Windows 2000");
		static LPCTSTR WXPSTR          = _T("Windows XP");
		static LPCTSTR W2003SERVERSTR  = _T("Windows 2003 Server");
		static LPCTSTR WCESTR          = _T("Windows CE");

		static const int WUNKNOWN      = 0;
		static const int W9XFIRST      = 1;
		static const int W95           = 1;
		static const int W95SP1        = 2;
		static const int W95OSR2       = 3;
		static const int W98           = 4;
		static const int W98SP1        = 5;
		static const int W98SE         = 6;
		static const int WME           = 7;
		static const int W9XLAST       = 99;
		static const int WNTFIRST      = 101;
		static const int WNT351        = 101;
		static const int WNT4          = 102;
		static const int W2K           = 103;
		static const int WXP           = 104;
		static const int W2003SERVER   = 105;
		static const int WNTLAST       = 199;
		static const int WCEFIRST      = 201;
		static const int WCE           = 201;
		static const int WCELAST       = 299;

		if (!pszVersion || !nVersion || !pszMajorMinorBuild) return false;

		::lstrcpy(pszVersion, WUNKNOWNSTR);
		*nVersion = WUNKNOWN;

		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (!GetVersionEx(&osinfo)) return false;

		DWORD dwPlatformId   = osinfo.dwPlatformId;
		DWORD dwMinorVersion = osinfo.dwMinorVersion;
		DWORD dwMajorVersion = osinfo.dwMajorVersion;
		DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF; // Win 95 needs this

		TCHAR buf[50] = {0, };
#ifdef UNICODE
		swprintf(buf, ASIZE(buf), L"%u.%u.%u", 
			dwMajorVersion, dwMinorVersion, dwBuildNumber);
#else
		_snprintf(buf, ASIZE(buf), _TRUNCATE, "%u.%u.%u", 
			dwMajorVersion, dwMinorVersion, dwBuildNumber);
#endif
		::lstrcpy(pszMajorMinorBuild, buf);

		if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
		{
			if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
			{
				::lstrcpy(pszVersion, W95STR);
				*nVersion = W95;
			}
			else if ((dwMinorVersion < 10) && 
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
			{
				::lstrcpy(pszVersion, W95SP1STR);
				*nVersion = W95SP1;
			}
			else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
			{
				::lstrcpy(pszVersion, W95OSR2STR);
				*nVersion = W95OSR2;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
			{
				::lstrcpy(pszVersion, W98STR);
				*nVersion = W98;
			}
			else if ((dwMinorVersion == 10) && 
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
			{
				::lstrcpy(pszVersion, W98SP1STR);
				*nVersion = W98SP1;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
			{
				::lstrcpy(pszVersion, W98SESTR);
				*nVersion = W98SE;
			}
			else if (dwMinorVersion == 90)
			{
				::lstrcpy(pszVersion, WMESTR);
				*nVersion = WME;
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
			{
				::lstrcpy(pszVersion, WNT351STR);
				*nVersion = WNT351;
			}
			else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
			{
				::lstrcpy(pszVersion, WNT4STR);
				*nVersion = WNT4;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
			{
				::lstrcpy(pszVersion, W2KSTR);
				*nVersion = W2K;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
			{
				::lstrcpy(pszVersion, WXPSTR);
				*nVersion = WXP;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
			{
				::lstrcpy(pszVersion, W2003SERVERSTR);
				*nVersion = W2003SERVER;
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
		{
			::lstrcpy(pszVersion, WCESTR);
			*nVersion = WCE;
		}

		return true;

#undef VER_PLATFORM_WIN32s
#undef VER_PLATFORM_WIN32_WINDOWS
#undef VER_PLATFORM_WIN32_NT
#undef VER_PLATFORM_WIN32_CE
	}

	/// \brief strrchr TCHAR 버전
	/// \param str 검색할 문자열
	/// \param ch 찾고자 하는 글자
	/// \return TCHAR* 주어진 글자를 찾은 경우 해당 위치 포인터를 반환하고, 
	/// 찾지 못한 경우에는 NULL을 반환한다.
	TCHAR* lstrrchr(TCHAR* str, TCHAR ch)
	{
		TCHAR* start = str;
		while (*str++) ;
		while (--str != start && *str != ch) ;
		return *str == ch ? str : NULL;
	}
}


/*
#include "StdAfx.h"
#include "MiniDumper.h"
#include <dbghelp.h>

#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
	HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

cMiniDumper::DumpLevel cMiniDumper::ms_eDumpLevel				= cMiniDumper::DUMP_LEVEL_0;
BOOL                  cMiniDumper::ms_bAddTimeStamp				= TRUE;
TCHAR                 cMiniDumper::ms_acAppName[_MAX_PATH]		= {0,};
TCHAR                 cMiniDumper::ms_acFaultReason[2048]		= {0,};

//////////////////////////////////////////////////////////////////////////////
/// \brief 생성자
/// \param DL 덤프 레벨
/// \param bAddTimeStamp 덤프 파일 이름에다가 덤프 파일이 생성된 날짜를
/// 집어넣는가의 여부.
//////////////////////////////////////////////////////////////////////////////
cMiniDumper::cMiniDumper(DumpLevel DL, BOOL bAddTimeStamp)
{
//	Assert(s_szAppName[0] == 0);
//	Assert(DL >= 0);
//	Assert(DL <= DUMP_LEVEL_2);

	ms_eDumpLevel     = DL;
	ms_bAddTimeStamp = bAddTimeStamp;

	// 모듈 경로를 알아낸다.
	TCHAR filename[_MAX_PATH];
	::GetModuleFileName(NULL, filename, _MAX_PATH);

	// 확장자를 제거한 모듈 경로를 준비해두고...
	TCHAR* dot = ::_tcsrchr(filename, _T('.'));
	::lstrcpyn(ms_acAppName, filename, (int)(dot - filename + 1));

	dot = ::_tcsrchr(ms_acAppName, _T('\\'));
	::lstrcpyn(ms_acAppName, &dot[1], ::_tcslen(ms_acAppName) );


	// 예외 처리 핸들러를 설정한다.
	::SetUnhandledExceptionFilter(TopLevelFilter);
}

//////////////////////////////////////////////////////////////////////////////
/// \brief 소멸자
//////////////////////////////////////////////////////////////////////////////
cMiniDumper::~cMiniDumper()
{
}

//////////////////////////////////////////////////////////////////////////////
/// \brief 예외에 대한 정보를 받아서, 미니 덤프 파일을 생성한다. 
/// 
/// SetUnhandledExceptionFilter() API에 의해서 설정되고, 프로세스 내부에서 
/// Unhandled Exception이 발생될 경우, 호출되게 된다. 단 디버거가 붙어있는 
/// 경우, Unhandled Exception Filter는 호출되지 않는다. 이 말은 이 함수 
/// 내부를 디버깅할 수는 없다는 말이다. 이 함수 내부를 디버깅하기 위해서는 
/// 메시지 박스 또는 파일을 이용해야한다.
/// 
/// \param pExceptionInfo 예외 정보
/// \return LONG 이 함수를 실행하고 난 다음, 취할 행동값. 자세한 것은 SEH
/// 문서를 참고하도록.
//////////////////////////////////////////////////////////////////////////////
LONG WINAPI cMiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS* pExPtr)
{
	LONG    retval                   = EXCEPTION_CONTINUE_SEARCH;
	HMODULE hDLL                     = NULL;
	TCHAR   szDbgHelpPath[_MAX_PATH] = {0, };
	TCHAR   szDumpPath[MAX_PATH * 2] = {0,};

	// 먼저 실행 파일이 있는 디렉토리에서 DBGHELP.DLL을 로드해 본다.
	// Windows 2000 의 System32 디렉토리에 있는 DBGHELP.DLL 파일은 버전이 
	// 오래된 것일 수 있기 때문이다. (최소 5.1.2600.0 이상이어야 한다.)
	if (::GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH))
	{
		LPTSTR pSlash = ::_tcsrchr(szDbgHelpPath, _T('\\'));
		if (pSlash)
		{
			::lstrcpy(pSlash + 1, _T("DBGHELP.DLL"));
			hDLL = ::LoadLibrary(szDbgHelpPath);
		}
	}

	// 현재 디렉토리에 없다면, 아무 버전이나 로드한다.
	if (hDLL == NULL) hDLL = ::LoadLibrary(_T("DBGHELP.DLL"));

	// DBGHELP.DLL을 찾을 수 없다면 더 이상 진행할 수 없다.
	if (hDLL == NULL)
	{
		OutputDebugString(_T("DBGHELP.DLL not found"));
		return retval;
	}

	// DLL 내부에서 MiniDumpWriteDump API를 찾는다.
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
		(MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");

	// 미니덤프 함수를 찾을 수 없다면 더 이상 진행할 수 없다.
	if (pfnMiniDumpWriteDump == NULL)
	{
		OutputDebugString(_T("DBGHELP.DLL too old"));
		return retval;
	}

	if (ms_bAddTimeStamp)
	{
		// 현재 시간을 얻어온다.
		SYSTEMTIME t;
		::GetLocalTime(&t);

		// 시간 문자열을 준비한다.
		TCHAR szTail[_MAX_PATH];
		::_sntprintf(szTail, chDIMOF(szTail)-1, 
			_T("_%04d_%02d_%02d_%02d_%02d_%02d"),
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

		// 덤프 파일 이름 += 시간 문자열
//		::lstrcat(szDumpPath, _T("./Log/"));
		::lstrcat(szDumpPath, ms_acAppName);
		::lstrcat(szDumpPath, szTail);
	}

	// 덤프 파일 이름 += 확장자
	::lstrcat(szDumpPath, _T(".dmp"));

	// 파일을 생성한다.
	HANDLE hFile = ::CreateFile(
		szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// 파일을 생성할 수 없다면 더 이상 진행할 수 없다.
	if (hFile == INVALID_HANDLE_VALUE)
	{
//		filelog(NULL, "Failed to create dump file '%s' (error %s)", 
//			szDumpPath, GetLastErrorString().c_str());
		return retval;
	}

	MINIDUMP_EXCEPTION_INFORMATION ExceptionParam;

	ExceptionParam.ThreadId = ::GetCurrentThreadId();
	ExceptionParam.ExceptionPointers = pExPtr;
	ExceptionParam.ClientPointers = FALSE;

	// 옵션에 따라 덤프 파일을 생성한다. 
	BOOL bResult = FALSE;
	switch (ms_eDumpLevel)
	{
	case DUMP_LEVEL_0: // MiniDumpNormal
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpNormal, &ExceptionParam, NULL, NULL);
		break;
	case DUMP_LEVEL_1: // MiniDumpWithDataSegs 
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpWithDataSegs, &ExceptionParam, NULL, NULL);
		break;
	case DUMP_LEVEL_2: // MiniDumpWithFullMemory 
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpWithFullMemory, &ExceptionParam, NULL, NULL);
		break;
	default:
		bResult = pfnMiniDumpWriteDump(
			::GetCurrentProcess(), ::GetCurrentProcessId(), 
			hFile, MiniDumpNormal, &ExceptionParam, NULL, NULL);
		break;
	}

	// 덤프 파일 생성 결과를 로그 파일에다 기록한다.
	if (bResult)
	{
		TCHAR szMessage[8192] = {0,};
		lstrcat(szMessage, _T("Saved dump file to '"));
		lstrcat(szMessage, szDumpPath);
		lstrcat(szMessage, _T("'.\nFault Reason : "));
		lstrcat(szMessage, GetFaultReason(pExPtr));

//		filelog(NULL, szMessage);
//		retval = EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
//		filelog(NULL, "Failed to save dump file to '%s' (error %d,%s)", 
//		szDumpPath, ::GetLastError(), GetLastErrorString().c_str());

//		Assert(false);
	}

	::CloseHandle(hFile);

	return retval;
}

//////////////////////////////////////////////////////////////////////////////
/// \brief 
/// 
/// \param pExPtrs 
/// \return LPCTSTR
//////////////////////////////////////////////////////////////////////////////
LPCTSTR cMiniDumper::GetFaultReason(struct _EXCEPTION_POINTERS* pExPtrs)
{
	if (::IsBadReadPtr(pExPtrs, sizeof(EXCEPTION_POINTERS))) 
		return _T("BAD EXCEPTION POINTERS");

	// 간단한 에러 코드라면 그냥 변환할 수 있다.
	switch (pExPtrs->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:         return _T("EXCEPTION_ACCESS_VIOLATION");
	case EXCEPTION_DATATYPE_MISALIGNMENT:    return _T("EXCEPTION_DATATYPE_MISALIGNMENT");
	case EXCEPTION_BREAKPOINT:               return _T("EXCEPTION_BREAKPOINT");
	case EXCEPTION_SINGLE_STEP:              return _T("EXCEPTION_SINGLE_STEP");
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
	case EXCEPTION_FLT_DENORMAL_OPERAND:     return _T("EXCEPTION_FLT_DENORMAL_OPERAND");
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
	case EXCEPTION_FLT_INEXACT_RESULT:       return _T("EXCEPTION_FLT_INEXACT_RESULT");
	case EXCEPTION_FLT_INVALID_OPERATION:    return _T("EXCEPTION_FLT_INVALID_OPERATION");
	case EXCEPTION_FLT_OVERFLOW:             return _T("EXCEPTION_FLT_OVERFLOW");
	case EXCEPTION_FLT_STACK_CHECK:          return _T("EXCEPTION_FLT_STACK_CHECK");
	case EXCEPTION_FLT_UNDERFLOW:            return _T("EXCEPTION_FLT_UNDERFLOW");
	case EXCEPTION_INT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
	case EXCEPTION_INT_OVERFLOW:             return _T("EXCEPTION_INT_OVERFLOW");
	case EXCEPTION_PRIV_INSTRUCTION:         return _T("EXCEPTION_PRIV_INSTRUCTION");
	case EXCEPTION_IN_PAGE_ERROR:            return _T("EXCEPTION_IN_PAGE_ERROR");
	case EXCEPTION_ILLEGAL_INSTRUCTION:      return _T("EXCEPTION_ILLEGAL_INSTRUCTION");
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
	case EXCEPTION_STACK_OVERFLOW:           return _T("EXCEPTION_STACK_OVERFLOW");
	case EXCEPTION_INVALID_DISPOSITION:      return _T("EXCEPTION_INVALID_DISPOSITION");
	case EXCEPTION_GUARD_PAGE:               return _T("EXCEPTION_GUARD_PAGE");
	case EXCEPTION_INVALID_HANDLE:           return _T("EXCEPTION_INVALID_HANDLE");
	case 0xE06D7363:                         return _T("Microsoft C++ Exception");
	default:
		break;
	}

	// 뭔가 좀 더 복잡한 에러라면...
	::lstrcpy(ms_acFaultReason, _T("Unknown")); 
	::FormatMessage(
		FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
		::GetModuleHandle(_T("NTDLL.DLL")),
		pExPtrs->ExceptionRecord->ExceptionCode, 
		0,
		ms_acFaultReason,
		0,
		NULL);

	return ms_acFaultReason;
}
*/