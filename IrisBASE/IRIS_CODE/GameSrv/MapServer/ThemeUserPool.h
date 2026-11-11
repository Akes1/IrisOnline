/* ====================================================================
 * 파    일 :	ThemeUserPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/06/15
 * 주의사항 :	
 * =================================================================== */
#ifndef __THEME_USER_POOL_H__
#define __THEME_USER_POOL_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

#include "memorypool.h"

// 테마와 룸 모드의 조합.
#define MAKETHEMECID(T,M) (((T<<16)&0xffff0000)|(M&0x0000ffff))

struct PerThemeUser : PerNode
{
	long   cid;					// K 테마 룸 연결 ID
	short  themeIdx;			//   테마 IDX
	short  roomIdx;				//   룸   IDX
	BYTE   mode;				//   룸   모드
	time_t lastDate;			//   룸   마지막 업데이트(일)
	time_t validThru;			//   룸   유효만기(일)
};

struct ThemeUserRoot : NodeRoot
{};

class cThemeUserPool
{
private:
	PerThemeUser* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cThemeUserPool( );

	void          Shutdown         ( );

	PerThemeUser* SearchThemeUser  ( ThemeUserRoot* themeUserRoot, long cid );
	bool          IsThemeUser      ( ThemeUserRoot* themeUserRoot, long cid );

	PerThemeUser* GetThemeUser     ( ThemeUserRoot* themeUserRoot, long cid );
	void          ReleaseThemeUser ( ThemeUserRoot* themeUserRoot, PerThemeUser* perThemeUser );

public:
	virtual ~cThemeUserPool(void);

public:
	static cThemeUserPool* mpThemeUserPool;
};

#define THEMEUSERPOOL cThemeUserPool::mpThemeUserPool

#endif /// __THEME_USER_POOL_H__
