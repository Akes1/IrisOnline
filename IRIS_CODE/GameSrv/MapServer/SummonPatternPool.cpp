// Include
#include "gamesrv.h"
#include "SummonPatternPool.h"

cSummonPatternPool* cSummonPatternPool::mpSummonPatternPool = NULL;

cSummonPatternPool::cSummonPatternPool( )
{
	// Global Variable
	mpSummonPatternPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

cSummonPatternPool::~cSummonPatternPool(void)
{
	// 해제.
	Shutdown();

	// Global Variable
	mpSummonPatternPool = NULL;
}

void cSummonPatternPool::Shutdown()
{
	sSummonPatternInfo* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (sSummonPatternInfo*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

sSummonPatternInfo* cSummonPatternPool::AddSummonPattern( sSummonPatternRoot* pRoot, sObject object,
														 unsigned long summonPatternIdx,
														 unsigned long count,
														 NiPoint2 centerPos,
														 float centerDir,
														 unsigned long rangeMin,
														 unsigned long rangeMax,
														 unsigned long applyType,
														 unsigned char nowPatternPos,
														 unsigned long nextPatternTime )
{
	sSummonPatternInfo* pSummonPattern = (sSummonPatternInfo*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(sSummonPatternInfo) );
	if ( pSummonPattern )
	{
		MEMORY_POOL->AttachPool( &pRoot->pool, pSummonPattern );

		pSummonPattern->mObject = object;
		pSummonPattern->mSummonPatternIdx = summonPatternIdx;
		pSummonPattern->mCount = count;
		pSummonPattern->mCenterPos = centerPos;
		pSummonPattern->mCenterDir = centerDir;
		pSummonPattern->mRangeMin = rangeMin;
		pSummonPattern->mRangeMax = rangeMax;
		pSummonPattern->mApplyType = applyType;
		pSummonPattern->mNowPatternPos = nowPatternPos;
		pSummonPattern->mNextPatternTime = nextPatternTime;

		pRoot->count++;
	}

	return pSummonPattern;
}

void cSummonPatternPool::ReleaseSummonPattern( sSummonPatternRoot* pRoot, sSummonPatternInfo* pSummonPattern )
{
	if( pSummonPattern == NULL )
		return;

	sObject object = { eOBJECTTYPE_NONE, 0 };
	pSummonPattern->mObject = object;
	pSummonPattern->mSummonPatternIdx = 0;
	pSummonPattern->mCount = 0;
	pSummonPattern->mCenterPos = NiPoint2::ZERO;
	pSummonPattern->mCenterDir = 0.0f;
	pSummonPattern->mRangeMin = 0;
	pSummonPattern->mRangeMax = 0;
	pSummonPattern->mNowPatternPos = 0;
	pSummonPattern->mNextPatternTime = 0;

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&pRoot->pool, (PerNode*)pSummonPattern );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)pSummonPattern );
	--pRoot->count;
}
