#include "stdafx.h"
#include "GameSrv.h"
#include "HaveSkillObject.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "StatusCalc_Server.h"
#include "ObjectManager.h"
#include "Protocol.h"
#include "Skill_Common.h"



cHaveSkillObject::cHaveSkillObject()
{
	mSkillIdx = 0;
	mSkillStep = 0;

	mCreationTime = 0;
	mRestTime = 0;
}



cHaveSkillObject::~cHaveSkillObject()
{
}



void* cHaveSkillObject::operator new( size_t n )
{
	if( n != sizeof(cHaveSkillObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocHaveSkillObject();
}



void cHaveSkillObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cHaveSkillObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreeHaveSkillObject( static_cast<cHaveSkillObject*>(ptr) );

	return;
}



void cHaveSkillObject::Init( unsigned long skillIdx, unsigned char skillStep, unsigned long restCoolTime )
{
	mSkillIdx = skillIdx;
	mSkillStep = skillStep;
	mCreationTime = NETWORK2->GetAccumTime();
	mRestTime = restCoolTime;
}



void cHaveSkillObject::RestoreInit( unsigned long skillIdx, unsigned char skillStep, unsigned long restCoolTime )
{
	mSkillIdx = skillIdx;
	mSkillStep = skillStep;
	mCreationTime = NETWORK2->GetAccumTime();
	mRestTime = restCoolTime;
}


bool cHaveSkillObject::StepUp( unsigned char skillStep, unsigned long restCoolTime )
{
	if( mSkillStep + 1 != skillStep )	
	{ 
		NETWORK2->PostServerEvent("cHaveSkillObject::StepUp mSkillstep[%d] != skillStep[%d] err", mSkillStep, skillStep );
		return false; 
	}
	mSkillStep = skillStep;
	mCreationTime = NETWORK2->GetAccumTime();
	mRestTime = restCoolTime;
	return true;
}

void cHaveSkillObject::StepUpByCheat( unsigned char skillStep, unsigned long restCoolTime )
{
	mSkillStep = skillStep;
	mCreationTime = NETWORK2->GetAccumTime();
	mRestTime = restCoolTime;
	return;
}


void cHaveSkillObject::UpdateCoolTime( unsigned long restCoolTime, float speedFactor )
{ 
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
	if( pInfo == 0 )
	{
		assert(NULL);
		return;
	}
	
	unsigned long fixCoolTime = restCoolTime;

	if( mSkillIdx <= NORMAL_ATTACK_SKILL_MAX )	/// 일반공격만 공속을 적용시킨다.
		fixCoolTime = (unsigned long)( restCoolTime * ( 2.0f - speedFactor ) );

	/// 허용치를 포함한 쿨타임
	if( fixCoolTime > SYNC_NORMALATTACK )
		fixCoolTime = fixCoolTime - SYNC_NORMALATTACK;
	else
		fixCoolTime = 0; 

	if( mCreationTime + mRestTime >= NETWORK2->GetAccumTime() + fixCoolTime )
		return;

	mCreationTime = NETWORK2->GetAccumTime();
	mRestTime = fixCoolTime;
}



unsigned long cHaveSkillObject::GetEndCoolTime()	
{ 
	return mCreationTime + mRestTime; 
}


unsigned long cHaveSkillObject::GetRestTime()
{
	unsigned long restTime = 0;
	if( ( mCreationTime + mRestTime ) > NETWORK2->GetAccumTime() )
		restTime = ( mCreationTime + mRestTime ) - NETWORK2->GetAccumTime();

    return restTime; 
}