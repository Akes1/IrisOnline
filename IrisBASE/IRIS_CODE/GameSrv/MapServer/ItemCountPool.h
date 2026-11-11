/* ====================================================================
 * 파    일 :	ItemCountPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/01/19
 * 주의사항 :	
 * =================================================================== */
#ifndef __ITEM_COUNT_POOL_H__
#define __ITEM_COUNT_POOL_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

#include "memorypool.h"

struct PerItemCount : PerNode
{
	long index;							// K 아이템 정의 테이블 기획/분류
	long equip;							// 착용
	long bag;							// 가방
	long etc;						    // 기타
	long sum;							// 합계
};

struct ItemCountRoot : NodeRoot
{};

class cItemCountPool
{
private:
	PerItemCount* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cItemCountPool( );

	void          Shutdown         ( );

	PerItemCount* SearchItemCount  ( ItemCountRoot* root, long index );
	PerItemCount* UpdateItemCount  ( ItemCountRoot* root, long index, short number, short count );

	PerItemCount* GetItemCount     ( ItemCountRoot* root, long index, short number, short count );
	void          ReleaseItemCount ( ItemCountRoot* root, PerItemCount* perItemCount );

public:
	virtual ~cItemCountPool(void);

public:
	static cItemCountPool* mpItemCountPool;
};

#define ITEMCOUNTPOOL     cItemCountPool::mpItemCountPool

#endif /// __ITEM_COUNT_POOL_H__
