/* ====================================================================
 * 파    일 :	Sender.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/02/13
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOG_SENDER_H__
#define __LOG_SENDER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpudpsend.h"

#include "logprotocol.h"
#include "serverprotocol.h"
#include "servermanager.h"

class cSender :
	public cIocpUdpSend
{
private:
	SOCKADDR_IN      mGame;
	u_short          mGameSeq;
	IoContextBuffer* mGameBuffer;
	DWORD            mGameRelay;

	SOCKADDR_IN      mLog;
	u_short          mLogSeq;
	IoContextBuffer* mLogBuffer;
	DWORD            mLogRelay;

	SOCKADDR_IN      mRecv;

public:
// Log 관련 메소드.
	bool    PostServerEvent     ( char* message );
	bool    PostMemberEvent     ( char type, char category, long memberIdx, char* message );
	bool    PostConcurrentEvent ( long serverIdx, long minimum, long maximum, char* message );

public:
// Game 관련 메소드.
	bool    PostChSync          ( );
	bool    PostChList          ( ServerTable* root );
	bool    PostChStatus        ( long cid, BYTE status );

private:
	bool    SendComplete        ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cSender(void);

	bool    Initialize          ( char* send, char* recv, u_short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void    Shutdown            ( DWORD maxWait=INFINITE );

	bool    ReSendPost          ( u_long addr, u_short port, u_long seq );
	bool    SendReliable        ( u_long addr, u_short port, u_long seq );

	char*   GetPacket           ( void** handle, char tos, SOCKADDR_IN addr, u_short& seq );
	bool    SendPacket          ( void* handle, unsigned long length=0 );

	void    GetRegisterDate     ( TIMESTAMP_STRUCT* ts );

	DWORD   BackendThread       ( );

public:
// 클래스 파괴 메소드.
	virtual ~cSender(void);
};

#endif // __LOG_SENDER_H__
