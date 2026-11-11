/* ====================================================================
 * 파    일 :	ioContextPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	04/11/17
 * 주의사항 :	
 * =================================================================== */
#ifndef __IO_CONTEXT_POOL_H__
#define __IO_CONTEXT_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#ifndef IO_CONTEXT_VER
#define IO_CONTEXT_VER 0x0001
#else
#if defined(IO_CONTEXT_VER) && (IO_CONTEXT_VER < 0x0001)
#error IO_CONTEXT_VER setting conflicts
#endif
#endif

#pragma once

#include <winsock2.h>
#include <windows.h>
#include "criticalsectionlock.h"

#define DEF_TCP_PACKET_SIZE 0x10000             // 65535+1
#define HAL_TCP_PACKET_SIZE 0x08000             // 32767+1

// UDP  Packet size 65,535
// IPv4 Header size     60
// UDP  Header size      8
#define MAX_UDP_PACKET_SIZE 65467				// 65,535 - 60 - 8 = 65,467
#define DEF_UDP_PACKET_SIZE 65536				// 65,535+1

#ifndef PER_IO_CONTEXT
#define PER_IO_CONTEXT

enum IOCP_REQUESTTYPE
{
	IOCP_REQUEST_READ = 0,                      // 읽기(수신)
	IOCP_REQUEST_WRITE,                         // 쓰기(송신)
	IOCP_REQUEST_CALLBACK,                      // 콜백
};

enum IOCP_REQUESTRESULT
{
	IOCP_REQUEST_SUCCESS = 0,                   // 성공
	IOCP_REQUEST_ERROR,                         // 오류
};

struct PerIoContext
{
	WSAOVERLAPPED        wsaOverlapped;         // overlapped I/O 모델 사용을 위한 구조체
	SOCKET               socket;                // 클라이언트 소켓
	SOCKADDR_IN          addr;                  // 클라이언트 주소

	char*                buffer;                // 송/수신 버퍼
	u_long               offset;                // 버퍼의 오프셋
	u_long               length;                // 버퍼의 길이

	u_long               Internal;              // 내부 오프셋
	u_long               InternalHigh;          // 내부 오프셋

	u_long               requestType;           // Request Type
	u_long               requestResult;         // Request Result

	UINT_PTR             iParam;                // i param
	ULONG_PTR            lParam;                // l Param

	struct PerIoContext* prev;                  // 선형리스트의 포인터 - 이전
	struct PerIoContext* next;                  // 선형리스트의 포인터 - 다음
};

#endif // PER_IO_CONTEXT


#define MAX_IO_CONTEXT_BUFFER_LEN 131072

struct IoContextBuffer
{
	PerIoContext* buffer[MAX_IO_CONTEXT_BUFFER_LEN];
	long          offset;
};

class cIoContextPool
{
protected:
	CRITICAL_SECTION mCs;                     // 동기화 제어
	u_long           mBufferLength;           // 버퍼 크기

	PerIoContext*    mPagedPoolUsage;         // 페이지 된 풀   - overlapped I/O 모델
	PerIoContext*    mNonPagedPoolUsage;      // 페이지 안된 풀 - overlapped I/O 모델

	SIZE_T           mQuotaPagedPoolUsage;    // 현재 페이지 된 풀   - 사용량
	SIZE_T           mQuotaNonPagedPoolUsage; // 현재 페이지 안된 풀 - 사용량
	SIZE_T           mWorkingSetSize;         // 현재 작업 설정크기.

protected:
	PerIoContext*    AllocIoContext         ( );
	void             FreeIoContext          ( PerIoContext** perIoContext );

protected:
	void             AttachPool             ( PerIoContext** pool, PerIoContext* perIoContext );
	void             DetachPool             ( PerIoContext** pool, PerIoContext* perIoContext );

	PerIoContext*    GetPool                ( );
	void             ReleasePool            ( PerIoContext* perIoContext, bool deleteIoContext=false );

public:
	cIoContextPool(u_long bufferLength=DEF_TCP_PACKET_SIZE);

	bool             DefaultWorkingSize     ( DWORD workingSize );
	void             GetProcessMemoryInfo   ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );
	void             Shutdown               ( );

	u_long           GetBufferLength        ( ) { return mBufferLength; }
	PerIoContext*    GetPagedPoolUsage      ( ) { return mPagedPoolUsage; }

	PerIoContext*    GetIoContext           ( );
	PerIoContext*    GetIoContext           ( SOCKET socket, u_long request );
	PerIoContext*    GetIoContext           ( SOCKET socket, u_long request, SOCKADDR_IN addr );
	void             ReleaseIoContext       ( PerIoContext* perIoContext, bool deleteIoContext=false, DWORD* errorPtr=NULL );

public:
	virtual ~cIoContextPool(void);
};

#endif // __IO_CONTEXT_POOL_H__