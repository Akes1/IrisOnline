/* ====================================================================
 * 파    일 :	loginSrv.h
 * 목    적 :	로그인 서버 클래스
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/21
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

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
	DEFAULT_SQL_WEB        = 0x1000, // SQL Web        Request default type
	DEFAULT_SQL_ACCOUNT    = 0x2000, // SQL Account    "
	DEFAULT_SERVER_PROCESS = 0x4000, // Server Process "
	DEFAULT_CLIENT_PROCESS = 0x8000, // Client Process "
};

#include "sqlweb.h"
#include "sqlaccount.h"
#include "loginprocess.h"

#include "sender.h"
#include "recver.h"

class cLoginSrv : public cWSAStartup
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
	TCHAR                 mServerAddr[MAX_PATH];
	TCHAR                 mServerBroadcastRecv[MAX_PATH];
	TCHAR                 mServerBroadcastSend[MAX_PATH];

private:
// 클래스.
	cSender*              mSender;
	cSQLWeb*              mSqlWeb;
	cSQLAccount*          mSqlAccount;
	cLoginProcess*        mLoginProcess;
	cRecver*              mRecver;

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
	cLoginSrv(void);

	BOOL                  ConsolCtrlHandler  ( DWORD opcode );
	void                  ConsoleRun         ( DWORD argc, LPTSTR* argv );

	void                  ServiceCtrlHandler ( DWORD opcode );
	void                  ServiceRun         ( DWORD argc, LPTSTR* argv );

	void                  Close              ( );

public:
// 클래스 공유 메소드.
	TCHAR*                GetHostName        ( ) { return mFixedInfo->HostName; }

	cSender*              GetSender          ( ) { return mSender; }
	cSQLWeb*              GetSQLWeb          ( ) { return mSqlWeb; }
	cSQLAccount*          GetSQLAccount      ( ) { return mSqlAccount; }
	cLoginProcess*        GetLoginProcess    ( ) { return mLoginProcess; }
	cRecver*              GetRecver          ( ) { return mRecver; }

public:
// 클래스 파괴 메소드.
	virtual ~cLoginSrv(void);
};

DWORD ErrorCode2String ( VOID** msgBuf, DWORD lastError = GetLastError(), WORD priLangId = LANG_NEUTRAL, WORD subLangId = SUBLANG_DEFAULT );

extern cLoginSrv* g_loginSrv;
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

#endif // __LOGIN_SERVER_H__
