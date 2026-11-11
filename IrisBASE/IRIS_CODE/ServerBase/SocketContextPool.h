/* ====================================================================
 *<
 * 파    일 :	socketContext.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	04/12/02
 * 주의사항 :	Header:  Declared in winsock2.h
				Library: Use ws2_32.lib
**********************************************************************/
#ifndef __SOCKET_CONTEXT_POOL_H__
#define __SOCKET_CONTEXT_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#ifndef SOCKET_CONTEXT_VER
#define SOCKET_CONTEXT_VER 0x0001
#else
#if defined(SOCKET_CONTEXT_VER) && (SOCKET_CONTEXT_VER < 0x0001)
#error SOCKET_CONTEXT_VER setting conflicts
#endif
#endif

#pragma once

#include <winsock2.h>
#include <windows.h>

#include "criticalsectionlock.h"
//#include "AntiCpXSvr.h"


#ifndef __MIN_MAX_TTL__
#define __MIN_MAX_TTL__

#define MAX_TTL     0x2BF20                     // 180000 = (60*1000)*3 = 3분(연장)
#define MIN_TTL     0XEA60                      // 60000  = (60*1000)   = 1분(연장)

#endif // __MIN_MAX_TTL__


#ifndef PER_SOCKET_CONTEXT
#define PER_SOCKET_CONTEXT

struct PerSocketContext
{
	SOCKET                   socket;            // 클라이언트 소켓
	u_long                   seq;               // Sequence Number
	u_long                   ack;               // Acknowledgment Number

	union
	{
		struct
		{
			u_char connectionID   : 1;          // CID 사용      ( 0 Off / 1 On )
			u_char connectionDead : 1;          // 연결이 끊어짐 ( 0 Off / 1 On )
			u_char closeSocket    : 1;          // 소켓을 닫는다 ( 0 Off / 1 On )
			u_char closeWait      : 1;          // 닫기 대기중   ( 0 Off / 1 On )
		} status;

		BYTE statusMask;
		BYTE statusData;
	};

	SOCKADDR_IN              addr;              // 인터넷 주소
	DWORD                    timeToLive;        // Time To Live (TTL)

	char*                    buffer;            // 버퍼
	u_long                   offset;            // 버퍼의 오프셋
	u_long                   length;            // 버퍼의 길이

	u_long                   Internal;          // 내부 오프셋
	u_long                   InternalHigh;      // 내부 오프셋

	DWORD                    cid;               // Connection ID

	char*                    wsaBuf;            // 버퍼
	u_long                   wsaOff;            // 버퍼의 오프셋
	u_long                   wsaLen;            // 버퍼의 길이

	struct PerSocketContext* next;              // Dual Linked List 참조 포인터
	struct PerSocketContext* prev;              // Dual Linked List 참조 포인터

	struct PerSocketContext* parent;            // BST - 모체
	struct PerSocketContext* left;              // BST - 왼쪽   (Left  = Less than)
	struct PerSocketContext* right;             // BST - 오른쪽 (Right = Greater than)
};

#endif // PER_SOCKET_CONTEXT


class cSocketContextPool
{
protected:
	CRITICAL_SECTION* mCs;                     // 동기화 제어
	u_long            mBufferLength;           // 버퍼 크기

protected:
	PerSocketContext* mPagedPoolUsage;         // 페이지 된 풀
	PerSocketContext* mNonPagedPoolUsage;      // 페이지 안된 풀
	PerSocketContext* mBstRoot;                // BST - 루트.

protected:
	SIZE_T            mQuotaPagedPoolUsage;    // 현재 페이지 된 풀   - 사용량
	SIZE_T            mQuotaNonPagedPoolUsage; // 현재 페이지 안된 풀 - 사용량
	SIZE_T            mWorkingSetSize;         // 현재 작업 설정크기.

protected:
	PerSocketContext* AllocSocketContext      ( );
	void              FreeSocketContext       ( PerSocketContext** perSocketContext );

	int               CompareCID              ( PerSocketContext* perSocketContext1, PerSocketContext* perSocketContext2 );
	int               CompareCID              ( int cid1, int cid2 );

protected:
	void              AttachPool              ( PerSocketContext** pool, PerSocketContext* perSocketContext );
	void              DetachPool              ( PerSocketContext** pool, PerSocketContext* perSocketContext );

	bool              AttachBst               ( PerSocketContext* perSocketContext );
	bool              DetachBst               ( PerSocketContext* perSocketContext );

	PerSocketContext* GetPool                 ( );
	void              ReleasePool             ( PerSocketContext* perSocketContext, bool isDelete=false );

public:
	cSocketContextPool(CRITICAL_SECTION* cs, u_long bufferLength=0x10000);

	bool              DefaultWorkingSize      ( DWORD workingSize );
	void              GetProcessMemoryInfo    ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );
	void              Shutdown                ( );

	PerSocketContext* GetPagedPoolUsage       ( ) { return mPagedPoolUsage; }

	bool              ClearCID                ( PerSocketContext* perSocketContext );
	PerSocketContext* GetCID                  ( DWORD cid );
	bool              SetCID                  ( PerSocketContext* perSocketContext, DWORD cid );

	PerSocketContext* GetPerSocketContext     ( SOCKET socket, SOCKADDR_IN addr, DWORD ttl=MAX_TTL );
	void              ReleasePerSocketContext ( PerSocketContext* perSocketContext, bool isDelete=false );

public:
	virtual ~cSocketContextPool(void);
};

#endif // __SOCKET_CONTEXT_POOL_H__
