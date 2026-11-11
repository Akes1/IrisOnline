/* ====================================================================
 * 파    일 :	ItemGetPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/01/19
 * 주의사항 :	
 * =================================================================== */
#ifndef __ITEM_GET_POOL_H__
#define __ITEM_GET_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "memorypool.h"

struct PerItemGet : PerNode
{
	long         key;					// Key (인벤토리 번호사용)
	TB_INVENTORY inventory;				// 변경되는 인벤토리
};

struct ItemGetRoot : NodeRoot
{};

class cItemGetPool
{
private:
	PerItemGet* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cItemGetPool( );

	void        Shutdown         ( );

	PerItemGet* SearchItemGet  ( ItemGetRoot* root, long key );

	PerItemGet* GetItemGet     ( ItemGetRoot* root, long key );
	void        ReleaseItemGet ( ItemGetRoot* root, PerItemGet* perItemGet );

public:
	virtual ~cItemGetPool(void);

public:
	static cItemGetPool* mpItemGetPool;
};

#define ITEMGETPOOL     cItemGetPool::mpItemGetPool

#endif // __ITEM_GET_POOL_H__
