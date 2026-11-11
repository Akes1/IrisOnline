#pragma once

#include "memorypool.h"

struct sSkillTargetDetail
{
	long			mDetailDamageHP;
	long			mDetailDamageMP;
	unsigned char	mDetailDamageKind;
	long			mDetailReflection;
	long			mDetailAttkAbsorbHP;
	long			mDetailAttkAbsorbMP;
};

struct sSkillTargetInfo : PerNode
{
	sObject		mTarget;

	unsigned long	mDetailCnt;
	sSkillTargetDetail	mDetail[10];

	sSkillTargetInfo()
	{
		mTarget.type = eOBJECTTYPE_NONE;
		mTarget.index = 0;
		mDetailCnt = 0;
		memset( mDetail, 0, sizeof(mDetail) );
	}
};

struct sSkillTargetRoot : NodeRoot
{
	unsigned long count;

	sSkillTargetRoot( )
	{
		pool  = NULL;
		root  = NULL;
		count = 0;
	}
};

class cSkillTargetPool
{
private:
	sSkillTargetInfo* mNonPagedPoolUsage;	// 페이지 안된 풀.

public:
	cSkillTargetPool( );

	void          Shutdown         ( );

	sSkillTargetInfo* AddSkillTarget( sSkillTargetRoot* pRoot, sObject mTarget );

	void          ReleaseSkillTarget( sSkillTargetRoot* pRoot, sSkillTargetInfo* pSkillTarget );

public:
	virtual ~cSkillTargetPool(void);

public:
	static cSkillTargetPool* mpSkillTargetPool;
};

#define SKILLTARGETPOOL     cSkillTargetPool::mpSkillTargetPool
