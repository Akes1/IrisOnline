// Include
#include "gamesrv.h"
#include "SkillTargetPool.h"

cSkillTargetPool* cSkillTargetPool::mpSkillTargetPool = NULL;

cSkillTargetPool::cSkillTargetPool( )
{
	// Global Variable
	mpSkillTargetPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

cSkillTargetPool::~cSkillTargetPool(void)
{
	// 해제.
	Shutdown();

	// Global Variable
	mpSkillTargetPool = NULL;
}

void cSkillTargetPool::Shutdown()
{
	sSkillTargetInfo* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (sSkillTargetInfo*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

sSkillTargetInfo* cSkillTargetPool::AddSkillTarget( sSkillTargetRoot* pRoot, sObject mTarget )
{
	sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(sSkillTargetInfo) );
	if ( pSkillTarget )
	{
		MEMORY_POOL->AttachPool( &pRoot->pool, pSkillTarget );

		pSkillTarget->mTarget = mTarget;

		pRoot->count++;
	}

	return pSkillTarget;
}

void cSkillTargetPool::ReleaseSkillTarget( sSkillTargetRoot* pRoot, sSkillTargetInfo* pSkillTarget )
{
	if( pSkillTarget == NULL )
		return;

	pSkillTarget->mTarget.type = eOBJECTTYPE_NONE;
	pSkillTarget->mTarget.index = 0;
	pSkillTarget->mDetailCnt = 0;
	memset( pSkillTarget->mDetail, 0, sizeof(pSkillTarget->mDetail) );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&pRoot->pool, (PerNode*)pSkillTarget );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)pSkillTarget );
	--pRoot->count;
}
