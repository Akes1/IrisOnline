#pragma once

#include "memorypool.h"

struct PerPvPPlayer : PerNode
{
	short			mCharacterChannel;	/// 플레이어의 신청시 채널
	unsigned long	mPlayerIdx;			/// 플레이어idx
	unsigned char	mLevel;				/// 플레이어레벨
	unsigned char	mForceType;			/// 세력타입
	bool			mSelection;			/// 전장참여에 선정/비선정 여부
};

struct PvPPlayerRoot : NodeRoot
{
	unsigned long count;

	PvPPlayerRoot( )
	{
		pool  = NULL;
		root  = NULL;
		count = 0;
	}
};

class cPvPPlayerListPool
{
private:
	PerPvPPlayer* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cPvPPlayerListPool( );

	void          Shutdown         ( );

	PerPvPPlayer* AddPlayer( PvPPlayerRoot* root, short channel, unsigned long playerIdx, unsigned char level, 
		unsigned char forceType, bool selection );
	PerPvPPlayer* GetPlayer( PvPPlayerRoot* root, long index );

	void          ReleasePlayer( PvPPlayerRoot* root, PerPvPPlayer* perPlayer );

public:
	virtual ~cPvPPlayerListPool(void);

public:
	static cPvPPlayerListPool* mpPvPPlayerListPool;
};

#define PVPPLAYERLISTPOOL     cPvPPlayerListPool::mpPvPPlayerListPool
