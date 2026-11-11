/// 참고 및 변형 http://serious-code.net/moin.cgi/MiniDump
///				 http://www.codeproject.com/threads/StackWalker.asp
#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////
/// \class cMiniDump
/// \brief 미니 덤프를 실행하기 위한 클래스. 특별히 어떤 당위성이 있어서 만든 
/// 클래스는 아니고, 정적 변수와 함수를 가지고 있기 위해 만든 네임 스페이스 
/// 역할 클래스이다. 
///
/// 덤프의 초기화는 Install 함수를 통해 이루어진다. 프로그램 시작 부분 아무 
/// 곳에서나 cMiniDump::Install(...) 함수를 호출해주면 된다. 
///
/// <pre>
/// int main()
/// {
///     ...
///     cMiniDump::Install(cMiniDump::DUMP_LEVEL_LIGHT);
///     ...
/// }
/// </pre>
///
/// GUI를 사용할 수 있는 프로그램의 경우, 사용자에게 덤프 파일 생성 여부를 묻는
/// 것도 괜찮다고 생각해서, 대화창 콜백 함수를 하나 집어 넣었다. Install 함수를 
/// 통해 대화창 콜백 함수를 설정하면, 덤프 생성시 이를 확인해 먼저 대화창을 
/// 띄운다. 대화창이 IDOK로 끝나면, 덤프 파일을 생성하고, 그외의 값으로 끝나면 
/// 덤프 파일을 생성하지 않는다. 기본적인 대화창 함수를 구현하자면 대충 아래와 
/// 같을 것이다.
///
/// <pre>
/// BOOL CALLBACK CrashDialogProc(
///     HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM /*lParam*/)
/// {
///     RECT parent, dlg;
///     int x, y;
/// 
///     switch (iMessage)
///     {
///     case WM_INITDIALOG:
///         ::GetWindowRect(::GetForegroundWindow(), &parent);
///         ::GetWindowRect(hDlg, &dlg);
///         x = (parent.left + parent.right - dlg.right) / 2;
///         y = (parent.top + parent.bottom - dlg.bottom) / 2;
///         ::MoveWindow(hDlg, x, y, dlg.right, dlg.bottom, FALSE);
//          ...
///         return TRUE;
///     case WM_COMMAND:
///         switch (LOWORD(wParam))
///         {
///         case IDOK: EndDialog(hDlg, IDOK); break;
///         case IDCANCEL: EndDialog(hDlg, IDCANCEL); break;
///         default: break;
///         }
///         return FALSE;
///     default:
///         break;
///     }
/// 
///     return FALSE;
/// }
/// </pre>
////////////////////////////////////////////////////////////////////////////////


class cMiniDump
{
public:
	/// 덤프할 데이터의 수준
	enum DumpLevel
	{
		DUMP_LEVEL_LIGHT,  ///< MiniDumpNormal을 사용
		DUMP_LEVEL_MEDIUM, ///< MiniDumpWithDataSegs를 사용
		DUMP_LEVEL_HEAVY   ///< MiniDumpWithFullMemory를 사용
	};


private:
	static DumpLevel ms_DumpLevel;          ///< 덤프 레벨.
	static bool      ms_AddTimeStamp;       ///< 날짜 기반 덤프 파일 이름 사용
	static TCHAR     ms_AppName[_MAX_PATH]; ///< 덤프 파일 이름
	static TCHAR     ms_CallStack[8192];    ///< 콜스택 문자열
	static TCHAR     ms_Modules[8192];      ///< 모듈 문자열
	static LPCTSTR   ms_DialogTemplate;     ///< 대화창 템플릿
	static DLGPROC   ms_DialogProc;         ///< 대화창 프로시져


public:
	/// \brief 미니 덤프 기능을 초기화한다.
	static void Install(DumpLevel dumpLevel, bool addTimestamp=true, 
		LPCTSTR dialogTemplate=NULL, DLGPROC dialogProc=NULL);

	/// \brief 콜스택 문자열을 반환한다.
	static LPCTSTR GetCallStack() { return ms_CallStack; }

	/// \brief 모듈 문자열을 반환한다.
	static LPCTSTR GetModules() { return ms_Modules; }


private:
	/// \brief 예외에 대한 정보를 받아서, 미니 덤프 파일을 생성한다. 
	static LONG WINAPI WriteDump(PEXCEPTION_POINTERS exPtrs);

	/// \brief 생성 금지
	cMiniDump() {}

	/// \brief 복사 생성 금지
	cMiniDump(const cMiniDump&) {}

	/// \brief 대입 연산 금지
	cMiniDump& operator = (const cMiniDump&) { return *this; }
};



/*
#pragma once

#ifndef __NONCOPYABLE_H__
#include "Noncopyable.h"
#endif

class cMiniDumper : private Noncopyable
{
public:
	/// \brief 덤프할 데이터의 수준
	enum DumpLevel
	{
		DUMP_LEVEL_0, ///< MiniDumpNormal을 사용
		DUMP_LEVEL_1, ///< MiniDumpWithDataSegs를 사용
		DUMP_LEVEL_2  ///< MiniDumpWithFullMemory를 사용
	};

private:
	/// 덤프 레벨.
	static DumpLevel ms_eDumpLevel;            

	/// 덤프 파일 이름에다가 덤프 파일이 생성된 날짜를 집어넣는가의 여부.
	static BOOL ms_bAddTimeStamp;

	/// 덤프 파일 이름 문자열.
	static TCHAR ms_acAppName[_MAX_PATH];

	/// 덤프가 발생한 이유
	static TCHAR ms_acFaultReason[2048];

public:
	/// \brief 생성자
	cMiniDumper(DumpLevel DL, BOOL bAddTimestamp=TRUE);

	/// \brief 소멸자
	~cMiniDumper();

private:
	/// \brief 예외에 대한 정보를 받아서, 미니 덤프 파일을 생성한다. 
	static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS* pExPtrs);

	/// \brief 예외의 원인에 대한 문자열을 반환한다.
	static LPCTSTR GetFaultReason(struct _EXCEPTION_POINTERS* pExPtrs);
};
*/