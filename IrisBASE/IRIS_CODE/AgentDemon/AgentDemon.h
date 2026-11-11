/* ====================================================================
 * 파    일 :	AgentDemon.h
 * 목    적 :	아이리스 에이전트데몬 클래스
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/07/09
 * 주의사항 :	
 * =================================================================== */
#ifndef __AGENT_DEMON_H__
#define __AGENT_DEMON_H__

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
#include <assert.h>
#include "sqltypes.h"
// STL
#include "PointerArray.h"
#include "wsastartup.h"
#include "agentprocess.h"
#include "agentudpsend.h"

class AgentDemon : public cWSAStartup
{
private:
	// 서비스 및 공용 환경변수.
	SERVICE_STATUS_HANDLE mServiceStatusHandle;
	SERVICE_STATUS        mServiceStatus;

	TCHAR*                mUserName;
	bool                  mRunService;
	bool                  mEndService;
	WORD                  mVersion;

private:
	// 환경변수.
	TCHAR                 mServerAddr[MAX_PATH];
	TCHAR                 mBroadcastAddr[MAX_PATH];

private:
	// 클래스.
	AgentProcess*         mAgentProcess;
	AgentUdpSend*         mAgentUdpSend;

private:
	// 서비스 제어 메소드.
	bool                  InstallService     ( LPCTSTR binaryPathName, LPCTSTR serviceName, DWORD startType = SERVICE_DEMAND_START );
	bool                  UninstallService   ( LPCTSTR serviceName );

private:
	// 응용프로그램 제어 메소드.
	DWORD                 Initialization     ( DWORD argc, LPTSTR* argv, DWORD* error );
	void                  Destroy            ( void );
	void                  Run                ( void );

public:
	// 클래스 생성 및 공용 메소드.
	AgentDemon(void);

	BOOL                  ConsolCtrlHandler  ( DWORD opcode );
	void                  ConsoleRun         ( DWORD argc, LPTSTR* argv );

	void                  ServiceCtrlHandler ( DWORD opcode );
	void                  ServiceRun         ( DWORD argc, LPTSTR* argv );

public:
	// 클래스 공유 메소드.
	TCHAR*                GetHostName        ( ) { return mFixedInfo->HostName; }
	AgentUdpSend*         GetAgentUdpSend    ( ) { return mAgentUdpSend; }
	AgentProcess*         GetAgentProcess    ( ) { return mAgentProcess; }

public:
	// 클래스 파괴 메소드.
	virtual ~AgentDemon(void);
};

DWORD ErrorCode2String ( VOID** msgBuf, DWORD lastError = GetLastError(), WORD priLangId = LANG_NEUTRAL, WORD subLangId = SUBLANG_DEFAULT );

// 전역변수 선언(참조)
extern AgentDemon* g_agentDemon;
extern bool        g_verbose;
extern bool        g_fps;
extern bool        g_packet;

extern u_short     g_loginPort;
extern u_short     g_gamePort;
extern u_short     g_logPort;

extern TCHAR       g_serviceName[MAX_PATH];

#endif // __AGENT_DEMON_H__
