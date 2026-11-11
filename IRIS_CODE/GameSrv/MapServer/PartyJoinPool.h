/* ====================================================================
 * 파    일 :	PartyJoinPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/01/19
 * 주의사항 :	
 * =================================================================== */
#ifndef __PARTY_JOIN_POOL_H__
#define __PARTY_JOIN_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "memorypool.h"
#include "player_common.h"
#include "party_common.h"

struct PartyJoin : PerNode
{
	wchar_t        name[MAX_NAME_BUFFER_SIZE];	// 캐릭터 이름
	unsigned long  partyIdx;					// 파티인덱스
	sPartyUserData playerDat;					// 캐릭터 데이타
	DWORD          delay;						// 응답대기
};

struct PartyJoinRoot : NodeRoot
{};

class cPartyJoinPool
{
private:
	PartyJoin*    mNonPagedPoolUsage;	// 페이지 안된 풀.
	PartyJoinRoot mPartyJoinRoot;

public:
	cPartyJoinPool( );

	void           Shutdown ( );

	PartyJoinRoot* GetRoot  ( ) { return &mPartyJoinRoot; }
	PartyJoin*     Search   ( wchar_t* name );

	PartyJoin*     Get      ( wchar_t* name );
	void           Release  ( PartyJoin* join );

public:
	virtual ~cPartyJoinPool(void);

public:
	static cPartyJoinPool* mpPartyJoinPool;
};

#define PARTYJOINPOOL     cPartyJoinPool::mpPartyJoinPool

#endif // __PARTY_JOIN_POOL_H__
