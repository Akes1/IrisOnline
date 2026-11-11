/* ====================================================================
 * 파    일 :	recver.h
 * 목    적 :
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/02/14
 * 주의사항 :	
 * =================================================================== */
#ifndef __RECVER_H__
#define __RECVER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpudprecv.h"

#include "serverProtocol.h"
#include "logprotocol.h"

#include "packetpool.h"

class cRecver :
	public IocpUdpRecv
{
private:
	PacketPool* mPacketPool;

private:
// LOG 관련 메소드.
	bool  PostServerEvent  ( LPCTSTR format, ... );

private:
// 서버 관련 메소드 - 보조.
	bool  BatchComplete    ( MSGBUF* msgBuf, int msgLen );

private:
// 서버 관련 메소드 - 주.
	bool  RecvComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cRecver(void);

	bool  Initialize       ( char* ipAddr="", unsigned short port=14700, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void  Shutdown         ( DWORD maxWait=INFINITE );

	DWORD BackendThread    ( );

public:
// 클래스 파괴 메소드.
	virtual ~cRecver(void);
};

#endif // __RECVER_H__
