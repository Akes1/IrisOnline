/* ====================================================================
 * 파    일 :	CooltimePool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	08/10/02
 * 주의사항 :	
 * =================================================================== */
#ifndef __COOLTIME_POOL_H__
#define __COOLTIME_POOL_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

#include "memorypool.h"

#define MIN_SAVE_ITEM_COOLTIME	15	// 아이템 쿨타임 저장시간 - 최소값(초)

struct PerCooltime : PerNode
{
	long   index;					// K 아이템 정의 테이블 기획/분류
	long   cooltime1;				//   쿨타임 - 적용된 총 시간
	long   cooltime2;				//   쿨타임 - 적용된 그룹번호
	time_t validThru;				//   쿨타임 - 남은시간
};

struct CooltimeRoot : NodeRoot
{};

class cCooltimePool
{
private:
	PerCooltime* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cCooltimePool( );

	void         Shutdown        ( );

	bool         IsCooltime      ( CooltimeRoot* cooltimeRoot, long index );
	void         ApplyCooltime   ( CooltimeRoot* cooltimeRoot, TB_INVENTORY* inventory );

	bool         UpdateCooltime  ( PerCooltime* perCooltime, long cooltime1, long cooltime2, time_t* ltime );

	PerCooltime* GetCooltime     ( CooltimeRoot* cooltimeRoot, long index );
	void         ReleaseCooltime ( CooltimeRoot* cooltimeRoot, PerCooltime* perCooltime );

public:
	virtual ~cCooltimePool(void);

public:
	static cCooltimePool* mpCooltimePool;
};

#endif /// __COOLTIME_POOL_H__
