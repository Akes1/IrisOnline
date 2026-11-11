/* ====================================================================
 * 파    일 :	threadPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	05/02/18
 * 주의사항 :	
 * =================================================================== */
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include <windows.h>

#define MAX_WORKER_THREAD   64
#define POOL_SHUTDOWN       ((OVERLAPPED*)((__int64)-1))

class cThreadPool
{
protected:
	HANDLE        mRequestQueue;
	int           mWorkerThreadNumber;
	HANDLE        mWorkerThread[MAX_WORKER_THREAD];

public:
	cThreadPool(void);

	virtual bool  Initialize   ( HANDLE completion=INVALID_HANDLE_VALUE, int numThreads=0, int numWorkerThreads=2 );
	virtual void  Shutdown     ( );

	virtual BOOL  QueueRequest ( ULONG_PTR completionKey, LPOVERLAPPED overlapped, DWORD bytesTransferred );

	virtual DWORD WorkerThread ( );

public:
	virtual ~cThreadPool(void);

public:
	static DWORD WINAPI WorkerThreadStartingPoint ( void* ptr );
};

#endif // __THREAD_POOL_H__