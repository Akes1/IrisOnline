/* ====================================================================
 * 파    일 :	logRecver.h
 * 목    적 :	각 서버의 로그를 받아서, SQL-Pool로 전달
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/02/14
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOG_RECVER_H__
#define __LOG_RECVER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpudprecv.h"

#include "logprotocol.h"

#include "packetpool.h"

#ifndef __LOG_RECVER_TYPE__
#define __LOG_RECVER_TYPE__

enum LOG_RECVER_REQUEST_TYPE
{
	// DEFAULT_LOG_RECVER - defined logdemon.h file
	LOG_RECVER_REQUEST = DEFAULT_LOG_RECVER,
};

#endif // __LOG_RECVER_TYPE__

class cLogRecver :
	public IocpUdpRecv
{
private:
	PacketPool* mPacketPool;

private:
// 서버 관련 메소드 - 보조.
	bool  SendSQL          ( MSGBUF* msgBuf, int msgLen, SQL_REQUEST_LOG_TYPE logType );

private:
// 서버 관련 메소드 - 보조.
	bool  BatchComplete    ( MSGBUF* msgBuf, int msgLen );

private:
// 서버 관련 메소드 - 주.
	bool  RecvComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool  CallbackComplete ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cLogRecver(void);

	void  ErrorLog         ( LPCTSTR format, ... );

	bool  Initialize       ( char* ipAddr="", unsigned short port=14700, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void  Shutdown         ( DWORD maxWait=INFINITE );

	DWORD BackendThread    ( );

public:
// 클래스 파괴 메소드.
	virtual ~cLogRecver(void);
};

#endif // __LOG_RECVER_H__