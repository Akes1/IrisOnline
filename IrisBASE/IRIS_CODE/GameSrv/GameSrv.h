/* ====================================================================
 * 파    일 :	gameSrv.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/21
 * 주의사항 :	
 * =================================================================== */
#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

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


#ifndef __DEFAULT_DEFINE__
#define __DEFAULT_DEFINE__

#define	DEFAULT_SQL_ACCOUNT		0x1000 // SQL Account  Request default type
#define	DEFAULT_SQL_GAME		0x2000 // SQL Game     "
#define	DEFAULT_GAME_PROCESS	0x4000 // Game Process "

#endif // __DEFAULT_DEFINE__


#include "sqlaccount.h"
#include "sqlgame.h"
#include "billconnector.h"
#include "BillPayletter.h"
#include "gameprocess.h"
#include "verbose.h"
#include "sender.h"
#include "recver.h"

class cGameSrv : public cWSAStartup
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
// 공용 환경변수.
	DWORD                 mServerNo;
	DWORD                 mChannelNo;
	TCHAR                 mServerName[MAX_PATH];
	TCHAR                 mServerAddr[MAX_PATH];
	TCHAR                 mServerBroadcastRecv[MAX_PATH];
	TCHAR                 mServerBroadcastSend[MAX_PATH];
	TCHAR                 mServerFirewall[MAX_PATH];
	TCHAR                 mServerBillAddr[MAX_PATH];
	int                   mServerBillPort;
	int                   mAhnhsEnbale;
	DWORD                 mAhnhsCheckTime;
	DWORD                 mAhnhsWaitTime;

private:
// 클래스.
	cSender*              mSender;
	cSQLAccount*          mSqlAccount;
	cSQLGame*             mSqlGame;
	cBillConnector*       mBillConnector;
	cBillPayletter*       mBillPayletter;
	cGameProcess*         mGameProcess;
	cVerbose*             mVerbose;
	cRecver*              mRecver;

private:
// 서비스 제어 메소드.
	bool                  InstallService     ( LPCTSTR binaryPathName, LPCTSTR serviceName, DWORD startType = SERVICE_DEMAND_START );
	bool                  UninstallService   ( LPCTSTR serviceName );

private:
// 응용프로그램 제어 메소드.
	DWORD                 Initialization     ( DWORD argc, LPTSTR* argv, DWORD* error );
	void                  Destroy            ( void );
	void                  Run                ( void );

private:
// Command Line 제어 메소드.
	int                   ErrorInsert        ( char* msg );

public:
// 클래스 생성 및 공용 메소드.
	cGameSrv(void);

	BOOL                  ConsolCtrlHandler  ( DWORD opcode );
	void                  ConsoleRun         ( DWORD argc, LPTSTR* argv );

	void                  ServiceCtrlHandler ( DWORD opcode );
	void                  ServiceRun         ( DWORD argc, LPTSTR* argv );

	void                  Close              ( );

public:
// 클래스 공유 메소드.
	TCHAR*                GetServerName      ( ) { return mServerName; }
	TCHAR*                GetHostName        ( ) { return mFixedInfo->HostName; }

	cSender*              GetSender          ( ) { return mSender; }
	cSQLAccount*          GetSQLAccount      ( ) { return mSqlAccount; }
	cSQLGame*             GetSQLGame         ( ) { return mSqlGame; }
	cBillConnector*       GetBillConnector   ( ) { return mBillConnector; }
	cGameProcess*         GetGameProcess     ( ) { return mGameProcess; }
	cRecver*              GetRecver          ( ) { return mRecver; }

public:
// 클래스 파괴 메소드.
	virtual ~cGameSrv(void);
};

DWORD  ErrorCode2String ( VOID** msgBuf, DWORD lastError = GetLastError(), WORD priLangId = LANG_NEUTRAL, WORD subLangId = SUBLANG_DEFAULT );

extern cGameSrv* g_gameSrv;
extern bool      g_verbose;
extern bool      g_fps;
extern bool      g_packet;
extern bool      g_udp;
extern bool      g_udpPacket;
extern bool      g_udpFrom;
extern bool      g_eventPrint;

extern u_short   g_loginPort;
extern u_short   g_gamePort;
extern u_short   g_logPort;
extern unsigned int g_BillType;

extern TCHAR     g_serviceName[MAX_PATH];

#endif // __GAME_SERVER_H__
