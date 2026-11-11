/* ====================================================================
 * 파    일 :	rudpprocess.h
 * 목    적 :
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/08/31
 * 주의사항 :	
 * =================================================================== */
#ifndef __RUDP_PROCESS_H__
#define __RUDP_PROCESS_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocprudp.h"
#include "packetpool.h"

#include "serverProtocol.h"
#include "logprotocol.h"

#include "servermanager.h"

class cRudpProcess :
	public IocpRudp
{
private:
	PacketPool*      mPacketPool;

	SOCKADDR_IN      mGame;
	u_short          mGameSeq;
	IoContextBuffer* mGameBuffer;

	SOCKADDR_IN      mLog;
	u_short          mLogSeq;
	IoContextBuffer* mLogBuffer;

public:
// Log 관련 메소드.
	void  GetRegisterDate     ( TIMESTAMP_STRUCT* ts );
	bool  PostServerEvent     ( LPCTSTR format, ... );

	bool  PostServerEvent     ( char* message );
	bool  PostMemberEvent     ( char type, char category, long memberIdx, char* message );
	bool  PostConcurrentEvent ( long serverIdx, long minimum, long maximum, char* message );

public:
// Game 관련 메소드.
	bool  PostChSync          ( );
	bool  PostChList          ( ServerTable* root );
	bool  PostChStatus        ( long cid, BYTE status );

private:
// 서버 관련 메소드 - 보조.
	bool  BatchComplete       ( MSGBUF* msgBuf, int msgLen );

private:
// 서버 관련 메소드 - 주.
	bool  RecvComplete        ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool  SendComplete        ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cRudpProcess(void);

	bool  Initialize          ( char* recv="", char* send="", unsigned short port=14700, unsigned short numWorkerThreads=2 );
	void  Shutdown            ( DWORD maxWait=INFINITE );

	bool  ReSendExec          ( u_long addr, u_short port, u_long seq );
	bool  SendReliable        ( u_long addr, u_short port, u_long seq );

	char* GetPacket           ( void** handle, char tos, SOCKADDR_IN addr, u_short& seq );
	bool  SendPacket          ( void* handle, unsigned long length=0 );

	DWORD BackendThread       ( );

public:
// 클래스 파괴 메소드.
	virtual ~cRudpProcess(void);
};

#endif // __RUDP_PROCESS_H__
