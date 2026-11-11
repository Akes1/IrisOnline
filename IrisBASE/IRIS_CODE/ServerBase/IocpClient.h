/**********************************************************************
 *<
	FILE: iocpClient.h

	DESCRIPTION:

	CREATED BY: ∞˚√∂¡ﬂ

	HISTORY: Created 05/20/06

 *>	Copyright (c) 2006, All Rights Reserved.
 **********************************************************************/
#ifndef __IOCP_CLIENT_H__
#define __IOCP_CLIENT_H__

#ifndef WINVER
#define WINVER          0x0500
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif

#ifndef IOCP_CLIENT_VER
#define IOCP_CLIENT_VER 0x0001
#else
#if defined(IOCP_CLIENT_VER) && (IOCP_CLIENT_VER < 0x0001)
#error IOCP_CLIENT_VER setting conflicts
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

#ifndef __MIN_MAX_TTL__
#define __MIN_MAX_TTL__

#define MAX_TTL     0x2BF20             // 180000 = (60*1000)*3 = 3∫–(ø¨¿Â)
#define MIN_TTL     0XEA60              // 60000  = (60*1000)   = 1∫–(ø¨¿Â)

#endif // __MIN_MAX_TTL__

class cIocpClient
{
protected:
	CRITICAL_SECTION    mCs;
	HANDLE              mIocp;
	SOCKET              mSocket;
	SOCKADDR_IN         mAddr;
	unsigned short      mPort;
	int                 mIocpWorkerThreadNumber;
	HANDLE              mIocpWorkerThread[IOCP_MAX_WORKER_THREAD];
	HANDLE              mIocpBackendThread;

	cIoContextPool*     mIoContextPool;

	bool                mConnected;
	bool                mConnectionDead;
	bool                mCloseSocket;
	DWORD               mTimeToLive;

	bool                mRunClient;
	bool                mEndClient;

protected:
	virtual bool        SendPost         ( ULONG_PTR completionKey, PerIoContext* perIoContext );
	virtual bool        RecvPost         ( ULONG_PTR completionKey, PerIoContext* perIoContext );
	virtual bool        CallbackPost     ( ULONG_PTR completionKey, PerIoContext* perIoContext );

	virtual void        Close            ( ULONG_PTR completionKey, PerIoContext* perIoContext );

	virtual bool        SendComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	virtual bool        RecvComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	virtual bool        CallbackComplete ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
	cIocpClient(void);

	virtual bool        Initialize       ( char* ipAddr="", unsigned short port=5001, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	virtual void        Shutdown         ( DWORD maxWait=INFINITE );

	virtual bool        IsConnect        ( void ) { return (mConnected == true && mConnectionDead == false) ? true : false; }
	virtual bool        Connect          ( SOCKADDR_IN addr );
	virtual bool        Disconnect       ( void );

	virtual void        GetIoPoolUsage   ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonePagedPoolUsage, SIZE_T& workingSetSize );
	virtual BOOL        QueueRequest     ( ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred );

	virtual DWORD       WorkerThread     ( void );
	virtual DWORD       BackendThread    ( void );

public:
	virtual ~cIocpClient(void);

public:
	static DWORD WINAPI WorkerThreadStartingPoint  ( void* ptr );
	static DWORD WINAPI BackendThreadStartingPoint ( void* ptr );
};

#endif // __IOCP_CLIENT_H__