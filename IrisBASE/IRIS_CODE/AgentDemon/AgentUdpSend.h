/* ====================================================================
 * 파    일 :	AgentUdpSend.h
 * 목    적 :
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/07/09
 * 주의사항 :	
 * =================================================================== */
#ifndef __AGENT_UDP_SEND_H__
#define __AGENT_UDP_SEND_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpudpsend.h"
#include "serverprotocol.h"

#include <sqltypes.h>
#include "logprotocol.h"

class AgentUdpSend :
	public cIocpUdpSend
{
private:
	SOCKADDR_IN      mLogin;
	u_long           mLoginSeq;
	IoContextBuffer* mLoginBuffer;

	SOCKADDR_IN      mGame;
	u_long           mGameSeq;
	IoContextBuffer* mGameBuffer;

	SOCKADDR_IN      mLog;
	u_long           mLogSeq;
	IoContextBuffer* mLogBuffer;

private:
	bool    SendComplete    ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

private:
	MSGBUF* GetPacket       ( void** handle, char tos, char protocol, SOCKADDR_IN addr, u_long& seq );
	bool    SendPacket      ( void* handle, u_long length );

public:
	// 클래스 생성 및 공용 메소드.
	AgentUdpSend(void);

	bool    Initialize      ( char* addr, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void    Shutdown        ( DWORD maxWait=INFINITE );

	bool    PostServerEvent ( char* message );

public:
	// 클래스 파괴 메소드.
	virtual ~AgentUdpSend(void);
};

#endif // __AGENT_UDP_SEND_H__