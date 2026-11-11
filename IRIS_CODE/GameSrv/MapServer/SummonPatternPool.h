#pragma once

#include "memorypool.h"

/// 소환 패턴 정보
struct sSummonPatternInfo : PerNode
{
	sObject			mObject;
	unsigned long	mSummonPatternIdx;
	unsigned long	mCount;
	NiPoint2		mCenterPos;
	float			mCenterDir;
	unsigned long	mRangeMin;
	unsigned long	mRangeMax;
	unsigned long	mApplyType;		/// eAPPLYTYPE
	unsigned char	mNowPatternPos;
	unsigned long	mNextPatternTime;
};

struct sSummonPatternRoot : NodeRoot
{
	unsigned long count;

	sSummonPatternRoot( )
	{
		pool  = NULL;
		root  = NULL;
		count = 0;
	}
};

class cSummonPatternPool
{
private:
	sSummonPatternInfo* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cSummonPatternPool( );

	void          Shutdown         ( );

	sSummonPatternInfo* AddSummonPattern( sSummonPatternRoot* pRoot, sObject mObject,
		unsigned long mSummonPatternIdx,
		unsigned long mCount,
		NiPoint2 mCenterPos,
		float mCenterDir,
		unsigned long mRangeMin,
		unsigned long mRangeMax,
		unsigned long mApplyType,
		unsigned char mNowPatternPos,
		unsigned long mNextPatternTime );

	void          ReleaseSummonPattern( sSummonPatternRoot* pRoot, sSummonPatternInfo* pSummonPattern );

public:
	virtual ~cSummonPatternPool(void);

public:
	static cSummonPatternPool* mpSummonPatternPool;
};

#define SUMMONPATTERNPOOL     cSummonPatternPool::mpSummonPatternPool
