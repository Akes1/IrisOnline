/* ====================================================================
 * 파    일 :	LogDemon.h
 * 목    적 :	아이리스 로그데몬 클래스
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/21
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOG_DEMON_H__
#define __LOG_DEMON_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#ifndef WSA_STARTUP_VER
#define WSA_STARTUP_VER 0x0002
#endif

#pragma once

// 윈속2    헤더 - link "ws2_32.lib"
#include <winsock2.h>
// 윈도우즈 헤더
#include <windows.h>
#include <winerror.h>
#include <lmerr.h>

#include <stdio.h>

#include "wsastartup.h"

enum REQUEST_DEFAULT_TYPE
{
	DEFAULT_SQL_LOG    = 0x1000, // SQL Log    Request default type
	DEFAULT_LOG_RECVER = 0x2000, // Log Recver "
};

#include "sqllog.h"

#include "logsender.h"
#include "logrecver.h"

class cLogDemon : public cWSAStartup
{
private:
	// 서비스 및 공용 환경변수.
	SERVICE_STATUS_HANDLE mServiceStatusHandle;
	SERVICE_STATUS        mServiceStatus;

	TCHAR*                mUserName;
	bool                  mRunService;
	bool                  mEndService;
	volatile bool         mClose;
	WORD                  mVersion;

private:
	// 환경변수.
	TCHAR                 mServerBroadcastSend[MAX_PATH];
	TCHAR                 mServerBroadcastRecv[MAX_PATH];

private:
	// 클래스 변수.
	cLogSender*           mLogSender;
	cSQLLog*              mSqlLog;
	cLogRecver*           mLogRecver;

private:
	// 서비스 제어 메소드.
	bool                  InstallService     ( LPCTSTR binaryPathName, DWORD startType = SERVICE_DEMAND_START );
	bool                  UninstallService   ( void );

private:
	// 응용프로그램 제어 메소드.
	DWORD                 Initialization     ( DWORD argc, LPTSTR* argv, DWORD* error );
	void                  Destroy            ( void );
	void                  Run                ( void );

public:
	// 클래스 생성 및 공용 메소드.
	cLogDemon(void);

	BOOL                  ConsolCtrlHandler  ( DWORD opcode );
	void                  ConsoleRun         ( DWORD argc, LPTSTR* argv );

	void                  ServiceCtrlHandler ( DWORD opcode );
	void                  ServiceRun         ( DWORD argc, LPTSTR* argv );

	void                  Close              ( );

public:
	// 클래스 공유 메소드.
	cLogSender*           GetLogSender       ( ) { return mLogSender; }
	cSQLLog*              GetSQLLog          ( ) { return mSqlLog; }
	cLogRecver*           GetLogRecver       ( ) { return mLogRecver; }

public:
	// 클래스 파괴 메소드.
	virtual ~cLogDemon(void);
};

DWORD ErrorCode2String ( VOID** msgBuf, DWORD lastError = GetLastError(), WORD priLangId = LANG_NEUTRAL, WORD subLangId = SUBLANG_DEFAULT );

// 전역변수 선언(참조)
extern cLogDemon* g_logDemon;
extern bool       g_verbose;
extern bool       g_fps;
extern bool       g_packet;
extern bool       g_udp;
extern bool       g_udpPacket;
extern bool       g_udpFrom;

extern u_short    g_loginPort;
extern u_short    g_gamePort;
extern u_short    g_logPort;

extern TCHAR      g_serviceName[MAX_PATH];

#endif // __LOG_DEMON_H__
