/* ====================================================================
 * 파    일 :	ServerManager.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	08/10/17
 * 주의사항 :	
 * =================================================================== */
#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "serverprotocol.h"

struct PerChannel
{
	long   cid;								// Connection Index (Server & Channel Number)
	BYTE   status;							// SERVER_STATUS 참조
	DWORD  timeToLive;						// TIME TO LIVE

	struct PerChannel* next;				// Linked List - next
	struct PerChannel* prev;				// "           - prev

	struct PerChannel* parent;				// Binary Tree - Parent
	struct PerChannel* left;				// "           - Child, left
	struct PerChannel* right;				// "           - Child, right

	void Status ( BYTE s ) { status = s; }
	void TimeToLive ( DWORD t ) { timeToLive = t; }
};

struct ServerTable
{
	long        idx;						// 고유번호
	wchar_t     name[51];					// 이름
	BYTE        status;						// SERVER_STATUS 참조
	BYTE        age;						// 적용나이
	BYTE        channelCounter;				// 채널 수
	DWORD       concurrentUser;				// 동시접속자 수
	DWORD       minConcurrentUser;			// 최소 동접자 수
	DWORD       maxConcurrentUser;			// 최대 동접자 수
	PerChannel* pool;						// 채널 Pool 포인터
	PerChannel* root;						// 채널 BST Root 포인터
	DWORD       sms;						// SMS 전송 수
};

#define MAX_SMS			32					// SMS 최대 수

class cServerManager
{
private:
	ServerTable  mServerTable[ MAX_SERVER ];
	PerChannel*  mNonPagedPoolUsage;        // 페이지 안된 풀.

	SIZE_T       mQuotaPagedPoolUsage;      // 현재 페이지 된 풀   - 사용량.
	SIZE_T       mQuotaNonPagedPoolUsage;   // 현재 페이지 안된 풀 - 사용량.
	SIZE_T       mWorkingSetSize;           // 현재 작업 설정크기.

	PerChannel*  AllocChannel    ( );
	void         FreeChannel     ( PerChannel** pch );

	int          CompareCID      ( PerChannel* pch1, PerChannel* pch2 );
	int          CompareCID      ( long cid1, long cid2 );

	void         AttachPool      ( PerChannel** pool, PerChannel* pch );
	void         DetachPool      ( PerChannel** pool, PerChannel* pch );

	bool         AttachBst       ( PerChannel** root, PerChannel* pch );
	bool         DetachBst       ( PerChannel** root, PerChannel* pch );

	PerChannel*  GetPool         ( ServerTable* serverTable, long cid );
	void         ReleasePool     ( ServerTable* serverTable, PerChannel* pch );

public:
	cServerManager( );

	void         GetMemoryInfo   ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );
	void         Shutdown        ( );

	ServerTable* GetServerTable  ( ) { return mServerTable; }
	ServerTable* GetServerTable  ( long cid );

	PerChannel*  SearchChannel   ( ServerTable* serverTable, long cid );
	long         FindChannel     ( long cid );

	PerChannel*  GetChannel      ( ServerTable* serverTable, long cid );
	bool         ReleaseChannel  ( long cid );

	void         UpdateProcess   ( DWORD currentTick );

public:
	virtual ~cServerManager(void);
};

inline void cServerManager::GetMemoryInfo(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	quotaPagedPoolUsage    = mQuotaPagedPoolUsage;
	quotaNonPagedPoolUsage = mQuotaNonPagedPoolUsage;
	workingSetSize         = mWorkingSetSize;
}

#endif // __SERVER_MANAGER_H__
