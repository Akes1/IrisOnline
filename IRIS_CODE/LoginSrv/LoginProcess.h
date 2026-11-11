/* ====================================================================
 * 파    일 :	LoginProcess.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/22
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOGIN_PROCESS_H__
#define __LOGIN_PROCESS_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpserver.h"
#include "loginsrvdefines.h"

#include "protocol.h"
#include "commonnetworkmsgstruct.h"
#include "servermanager.h"
#include "rc5.h"


class cLoginProcess :
	public cIocpServer
{
private:
// 환경변수.
	union
	{
		struct
		{
			bool serverMatrix : 1;
			bool serverTable  : 1;
		} mRequest;
		long mRequestData;
	};
	union
	{
		struct
		{
			bool startLog     : 1; // 시작로그
			bool serverMatrix : 1; // 매트릭스 검사
			bool serverTable  : 1; // 서버테이블 로딩
			bool serverSync   : 1; // 서버싱크
		} mComplete;
		long mCompleteData;
	};

	DWORD           mFrame;
	DWORD           mFPSTick;
	DWORD           mCCUTick;

	cServerManager* mServerManager;
	DWORD           mSecurityLevel;

public:
// LOG 관련 메소드.
	bool     PostServerEvent     ( LPCTSTR format, ... );
	bool     PostConcurrentEvent ( long serverIdx, long minimum, long maximum, LPCTSTR format, ... );
	bool     PostMemberEvent     ( char type, char category, long memberIdx, LPCTSTR format, ... );

public:
// SQL 관련 메소드.
	bool     MatrixCreate        ( long maxServer, long maxChannel );
	bool     ServerList          ( );
	bool     ServerDown          ( long serverNum, long channelNum );
	bool     ChannelUpdate       ( long serverNum, long channelNum, BYTE status );
	bool     ConcurrentUser      ( long serverNum );

	bool     WebLauncher         ( PerSocketContext* perSocketContext, long loginIdx );
	bool     WebCheck            ( PerSocketContext* perSocketContext, char* uid, char* pwd, char* sec );
	bool     WebLogin            ( PerSocketContext* perSocketContext, char* uid, char* pwd, char* sec, char* ip );
	bool     WebSMS              ( char* msg );

	bool     MemberLogin         ( PerSocketContext* perSocketContext, long memberIdx, char slevel, long pc );
	bool     MemberLogout        ( PerSocketContext* perSocketContext, long loginIdx, long memberIdx );
	bool     LoginUpdate         ( PerSocketContext* perSocketContext, long loginIdx );
	bool     GameToGame          ( PerSocketContext* perSocketContext, long loginIdx, short serverNum, short channelNum, short lastChannel );

private:
// 서버 관련 메소드 - 응답.
	bool     LoginSuccess        ( PerSocketContext* perSocketContext, long result, long memberIdx, long loginIdx, char* twitterUid, char* twitterPwd );
	bool     ServerList          ( PerSocketContext* perSocketContext, ServerTable* serverTable, char category, char protocol );
	bool     LoginToGame         ( PerSocketContext* perSocketContext, long ipv4, short port );

private:
// 서버 관련 메소드 - 암호.
	void     Decrypt             ( BYTE* ptr, RC5_32_KEY key );
	void     Encrypt             ( BYTE* ptr, RC5_32_KEY key );

private:
// 서버 관련 메소드 - 보조.
	bool     BatchComplete       ( PerSocketContext* perSocketContext, void* ptr );
	void     BackendThreadBegin  ( void );
	void     BackendThreadProc   ( void );
	void     BackendThreadEnd    ( void );
	void     BackendThreadFps    ( DWORD currentTick );

private:
// 서버 관련 메소드 - 주.
	bool     SendExec            ( PerIoContext* perIoContext );
	bool     AcceptComplete      ( PerSocketContext* perSocketContext );
	bool     RecvComplete        ( PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool     CallbackComplete    ( PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 서버 관련 메소드 - 채널관리.
	void     ChannelStatus       ( long cid, BYTE status );

public:
// 클래스 생성 및 공용 메소드.
	cLoginProcess(void);

	bool     Initialize          ( char* ipAddr="", unsigned short port=10000, unsigned short numWorkerThreads=2 );
	void     Shutdown            ( DWORD maxWait=INFINITE );

	DWORD    SecurityLevel       ( ) { return mSecurityLevel; }

	MSGROOT* GetMsgRoot          ( void** handle, PerSocketContext* perSocketContext );
	bool     SendMsgRoot         ( void* handle, unsigned long length );

	bool     SendMsgError        ( PerSocketContext* perSocketContext, char category, char protocol, int result );

	DWORD    BackendThread       ( );

public:
// 클래스 파괴 메소드.
	virtual ~cLoginProcess(void);

public:
	static cLoginProcess* mLoginProcess;
};

#define LOGINPROCESS     cLoginProcess::mLoginProcess

#endif // __LOGIN_PROCESS_H__
