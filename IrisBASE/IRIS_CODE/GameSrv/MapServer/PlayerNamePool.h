/* ====================================================================
 * 파    일 :	PlayerNamePool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/01/08
 * 주의사항 :	
 * =================================================================== */
#ifndef __PLAYER_NAME_POOL_H__
#define __PLAYER_NAME_POOL_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

#include "memorypool.h"
#include "player_common.h"

struct PerPlayerName : PerNode
{
	ULONG   connectionIdx;				// connection index
	ULONG	characterIdx;				// character index
	wchar_t	name[MAX_NAME_BUFFER_SIZE];	// character name
};

struct PlayerNameRoot : NodeRoot
{};

class cPlayerNamePool
{
private:
	PerPlayerName* mNonPagedPoolUsage;	// 페이지 안된 풀.
	PlayerNameRoot mPlayerNameRoot;

public:
	cPlayerNamePool( );

	void           Shutdown        ( );

	PerPlayerName* Search          ( wchar_t* name );

	PerPlayerName* GetPlayerName     ( ULONG connectionIdx, ULONG characterIdx, wchar_t* name );
	void           ReleasePlayerName ( PerPlayerName* perPlayerName );

public:
	virtual ~cPlayerNamePool(void);

public:
	static cPlayerNamePool* mpPlayerNamePool;
};

#define PLAYER_NAME_POOL     cPlayerNamePool::mpPlayerNamePool

#endif /// __PLAYER_NAME_POOL_H__
