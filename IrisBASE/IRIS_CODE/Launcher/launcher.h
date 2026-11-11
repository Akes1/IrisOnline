/**********************************************************************
 *<
	FILE: launcher.h

	DESCRIPTION:
		HINTERNET
		Header:  Declared in wininet.h
		Library: Use wininet.lib

	CREATED BY: Kwak CJ

	HISTORY: Created 08/11/14

 *>	Copyright (c) 2008, All Rights Reserved.
 **********************************************************************/
#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#ifndef WINVER
#define WINVER          0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#ifndef _WIN32_IE
#define _WIN32_IE		0x0600
#endif

#pragma once

#include <windows.h>        // 윈도우즈 헤더
#include <commctrl.h>

#include "zlib.h"
#include "httpdownloader.h"
#include "resource.h"

#pragma data_seg("shared")
long mUsageCnt = 0;
#pragma data_seg()
#pragma comment(linker, " /section:shared,rws")


class cDevLauncher;

enum UPDATE_MSG_TYPE
{
	_UMT_UPDATE_OPEN_,
	_UMT_UPDATE_LIST_,
	_UMT_UPDATE_FILE_,
	_UMT_UPDATE_LOADER_INIT_,
	_UMT_UPDATE_LOADER_OPEN_,
	_UMT_UPDATE_LOADER_DOWN_,
	_UMT_UPDATE_PATCHLIST_UNPACK_,
	_UMT_UPDATE_PATCHLIST_PARSING_,
	_UMT_UPDATE_PATCHLIST_DOWN_,
	_UMT_UPDATE_PATCHFILE_DOWN_,
	_UMT_UPDATE_NOT_EXIST_PATCHLIST_,
	_UMT_UPDATE_SAVE_FILE_,
	_UMT_UPDATE_ALLOCATION_,
	_UMT_UPDATE_COMPLETE_,

	_UMT_UPDATE_MAKELIST_WRONG_HEADER,
	_UMT_UPDATE_MAKELIST_WRONG_EXECUTE,
	_UMT_UPDATE_MAKELIST_DIFF_PATCHLIST,
	_UMT_UPDATE_MAKELIST_PARSE,
	_UMT_UPDATE_MAKELIST_END_APP,
};


enum eMAKELIST_RETURN_VALUE
{
	eMAKELIST_SUCCESS,
	eERROR_MAKELIST_WRONG_HEADER,
	eERROR_MAKELIST_WRONG_EXECUTE,
	eERROR_MAKELIST_DIFF_PATCHLIST,
	eERROR_MAKELIST_PARSE,
	eERROR_MAKELIST_END_APP,
};


struct Packfile
{
	char directory[ 1024 ];			// 디렉토리
};

struct Packlist
{
	Packfile* pack;
	ULONG     offset;				// 오프셋
	ULONG     length;				// 전체갯수
};

struct Patchfile
{
	char     fileName[ 1024 ];		// 파일이름
	DWORD    sizeHigh;				// 파일크기
	DWORD    sizeLow;				// 파일크기
	FILETIME writeTime;				// 파일날짜
};

struct Patchlist
{
	Patchfile* file;				// 업데이트 파일 구조체
	ULONG      offset;				// 오프셋
	ULONG      length;				// 전체갯수
};

class Launcher
{
private:
	HWND       m_wnd;
	HWND       m_RunModeSetDlgWnd;
	HICON      m_icon;
	TCHAR*     m_windowName;
	bool       m_endApp;
	bool       m_endIO;
	bool       m_runPatcher;
	char       m_currentDirectory[ MAX_PATH ];

	ClassDesc* m_libClassDesc;

	Packlist   m_packlist;
	Patchlist  m_patchlist;

	HANDLE     m_workerThread;
	
	cDevLauncher* m_DevLauncher;

	DWORD	   m_DownLoadExitCode;


private:
	void        FileTimeToUTC  ( FILETIME* filetime );
	void        FileTimeToDOS  ( FILETIME* filetime );

	bool        VerifyFile     ( Patchfile* patchfile );
	bool        UpdateFile     ( Patchfile* patchfile, char* buffer, DWORD bufferLen );
	bool        UnpackPatchList( char** buffer, DWORD *bufferLen );
	void		ErrorMessage   ( UINT Msg, WPARAM wParam, LPARAM lParam ); // 개발 모드인 경우 에러 메시지 출력
	eMAKELIST_RETURN_VALUE MakeList( char* buffer );

public:
	Launcher(void);

	int        Run            ( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow );
	LRESULT    MsgProc        ( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );

	HWND       GetHwnd        ( ) { return m_wnd; }
	TCHAR*     GetWindowName  ( ) { return m_windowName; }

	BOOL       SetStatus      ( HWND wnd, char* msg );

	DWORD      LoaderProgress ( DWORD totalFileSize, DWORD totalBytesTransferred, void* data );
	DWORD      WorkerThread   ( );

	LRESULT	   RunLauncher    ( );

private:
	LRESULT    Destroy        ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    InitDialog     ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    Command        ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    UpdateMsg      ( HWND wnd, WPARAM wParam, LPARAM lParam );

public:
	virtual ~Launcher(void);

public:
	static DWORD WINAPI LoaderProgressRoutine( DWORD totalFileSize, DWORD totalBytesTransferred, void* data );
};

#endif // __LAUNCHER_H__
