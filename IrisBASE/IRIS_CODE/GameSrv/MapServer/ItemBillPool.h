/* ====================================================================
 * 파    일 :	ItemBillPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/12/17
 * 주의사항 :	
 * =================================================================== */
#ifndef __ITEM_BILL_POOL_H__
#define __ITEM_BILL_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include <time.h>
#include "memorypool.h"

enum ITEMBILL_TYPE
{
	ITEM_BILL_NONE=0,
	ITEM_BILL_DATE,
	ITEM_BILL_TIME,
};

struct PerItemBill : PerNode
{
	long    idx;				//   고유번호
	BYTE    type;				//   종류(0 일반 / 1 기간 / 2 시간)
	long    validDate;			//   유효기간(초 단위 사용)
	long    validTime;			//   유효시간(초 단위 사용)
	long    inventoryIdx;		// K TB_INVENTORY::IDX 고유번호
	time_t  timeBegin;			//   시작시간
	time_t  timeEnd;			//   종료시간
	BYTE    apply;				//   사용여부(0 사용중, 1 사용중지, 2 삭제예약)
};

struct ItemBillRoot : NodeRoot
{};

class cItemBillPool
{
private:
	PerItemBill* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cItemBillPool( );

	void         Shutdown         ( );

	PerItemBill* SearchItemBill  ( ItemBillRoot* root, long inventoryIdx );

	PerItemBill* GetItemBill     ( ItemBillRoot* root, long inventoryIdx );
	void         ReleaseItemBill ( ItemBillRoot* root, PerItemBill* itemBill );

public:
	virtual ~cItemBillPool(void);

public:
	static cItemBillPool* mpItemBillPool;
};

#define ITEMBILLPOOL     cItemBillPool::mpItemBillPool

#endif // __ITEM_BILL_POOL_H__
