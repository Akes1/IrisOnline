/* ====================================================================
 * 파    일 :	iocpUdpRecv.h
 * 목    적 :	UDP - Receive 전용
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/02/14
 * 주의사항 :	UDP datagram의 최대 크기#
				이론적인 IP datagram의 최대 크기는 65535byte이다. 여기서 IP header 20byte, UDP header 8byte를 제외하면,
				UDP datagram의 최대 크기는 65507byte가 된다. 하지만 Socket API의 버퍼 크기 제한을 통해서 변화될 수 있다.
				일반적으로 UDP socket은 8192 byte 이상이다. 또한 Kernel의 구현에 의해 크기가 변화될 수 있다. 일반적으로
				UDP 프로그램들은 512 byte이다.
 * =================================================================== */
#ifndef __IOCP_UDP_RECV_H__
#define __IOCP_UDP_RECV_H__

#ifndef WINVER
#define WINVER          0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#ifndef IOCP_UDP_RECV_VER
#define IOCP_UDP_RECV_VER 0x0001
#else
#if defined(IOCP_UDP_RECV_VER) && (IOCP_UDP_RECV_VER < 0x0001)
#error IOCP_UDP_RECV_VER setting conflicts
#endif
#endif

#pragma once

#include <winsock2.h>
#include <windows.h>
#include <lmerr.h>

#include "iocontextpool.h"
#include "criticalSectionLock.h"

#define IOCP_MAX_WORKER_THREAD  16
#define IOCP_SHUTDOWN           ((OVERLAPPED*)((__int64)-1))

class IocpUdpRecv
{
protected:
	CRITICAL_SECTION    m_cs;
	HANDLE              m_iocp;
	SOCKET              m_socket;
	SOCKADDR_IN         m_addr;
	int                 m_iocpWorkerThreadNumber;
	HANDLE              m_iocpWorkerThread[IOCP_MAX_WORKER_THREAD];
	HANDLE              m_iocpBackendThread;

	cIoContextPool*     m_ioContextPool;

	IoContextBuffer*    m_ioContextFrontBuffer;
	IoContextBuffer*    m_ioContextBackBuffer;

	bool                m_runServer;
	bool                m_endServer;

protected:
	virtual bool        RecvPost         ( ULONG_PTR completionKey, PerIoContext* perIoContext );
	virtual bool        CallbackPost     ( ULONG_PTR completionKey, PerIoContext* perIoContext );

	virtual bool        RecvComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	virtual bool        CallbackComplete ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
	IocpUdpRecv(void);

	virtual bool        Initialize       ( char* ipAddr="", unsigned short port=5001, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	virtual void        Shutdown         ( DWORD maxWait=INFINITE );

	virtual void        GetIoPoolUsage   ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize );
	virtual BOOL        QueueRequest     ( ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred );

	virtual DWORD       WorkerThread     ( );
	virtual DWORD       BackendThread    ( );

public:
	virtual ~IocpUdpRecv(void);

public:
	static DWORD WINAPI WorkerThreadStartingPoint  ( void* ptr );
	static DWORD WINAPI BackendThreadStartingPoint ( void* ptr );
};

#endif // __IOCP_UDP_RECV_H__