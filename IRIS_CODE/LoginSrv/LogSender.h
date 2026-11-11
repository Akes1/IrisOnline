/* ====================================================================
 * 파    일 :	logSender.h
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

#include "packet.h"
#include "logprotocol.h"

class cLogSender :
	public cIocpUdpSend
{
public:
// 클래스 생성 및 공용 메소드.
	cLogSender(void);

	bool    Initialize          ( char* ipAddr="", unsigned short port=14700, unsigned short numWorkerThreads=2 );
	void    Shutdown            ( DWORD maxWait=INFINITE );

	MSGBUF* GetPacket           ( void** handle, char protocol );
	bool    SendPacket          ( void* handle, unsigned long length );

	bool    PostServerEvent     ( char* message );
	bool    PostMemberEvent     ( char type, char category, long memberIdx, char* message );
	bool    PostConcurrentEvent ( long serverIdx, long minimum, long maximum, char* message );

public:
// 클래스 파괴 메소드.
	virtual ~cLogSender(void);
};

#endif // __LOG_SENDER_H__
