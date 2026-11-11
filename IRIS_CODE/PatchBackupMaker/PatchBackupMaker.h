/**********************************************************************
 *<
	FILE: patchmaker.h

	DESCRIPTION:

	CREATED BY: Kwak CJ

	HISTORY: Created 08/11/19

 *>	Copyright (c) 2008, All Rights Reserved.
 **********************************************************************/
#ifndef __PATCHMAKER_H__
#define __PATCHMAKER_H__

#ifndef WINVER
#define WINVER          0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#pragma once
#include <windows.h>        // 윈도우즈 헤더
#include <commctrl.h>
#include "resource.h"

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

class PatchBackupMaker
{
private:
	HWND       m_wnd;
	HICON      m_icon;
	TCHAR*     m_windowName;
	bool       m_endApp;
	bool       m_isCopyMode;
	char       m_currentDirectory[ MAX_PATH ];

	char       m_backupDirectory[ 1024 ];
	char       m_pdbDirectory   [ 1024 ];
	

	Packlist   m_packlist;
	Patchlist  m_patchlist;
	HANDLE     m_workerThread;

	void       EventWrite      ( LPCTSTR format, ... );

	bool       GetFileTimeUTC  ( char* filename, SYSTEMTIME* utc );
	bool       SetFileTimeUTC  ( char* filename, SYSTEMTIME* utc );

	bool       StartPatch      ( );
	bool       CreateTodayBackupDirectory( /*backup directory*/ );

public:
	PatchBackupMaker(void);

	int        Run             ( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow );
	LRESULT    MsgProc         ( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );

	HWND       GetHwnd         ( ) { return m_wnd; }
	TCHAR*     GetWindowName   ( ) { return m_windowName; }

	bool       CommandLine     ( LPSTR cmdLine );
	BOOL       SetStatus       ( HWND wnd, char* msg );
	void       Progress        ( DWORD length, DWORD offset );

	DWORD      WorkerThread    ( );
	void	   ErrorPrint(char *msg);
private:
	LRESULT    Destroy         ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    InitDialog      ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    Command         ( HWND wnd, WPARAM wParam, LPARAM lParam );
	LRESULT    PatchMsg        ( HWND wnd, WPARAM wParam, LPARAM lParam );

public:
	virtual ~PatchBackupMaker(void);
};

#endif // __PATCHMAKER_H__
