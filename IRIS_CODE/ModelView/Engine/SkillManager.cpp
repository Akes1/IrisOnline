#include "stdafx.h"
#include "SkillManager.h"

#include "ObjectManager.h"
#include "DramaturgyManager.h"

#include "Application.h"

#include "Player_Common.h"

#include "FileSystem.h"

#include "Skill_Common.h"
#include "SkillScript.h"

#include "GameResourceManager.h"

cSkillManager* cSkillManager::mpSkillManager = NULL;

cSkillManager::cSkillManager()
{
	mpSkillManager = this;
}

cSkillManager::~cSkillManager()
{
	Close();

	///
	mSkillObjectPool.Clear();
	mInfluenceObjectPool.Clear();
}

bool cSkillManager::Init()
{
	mSkillObjectPool.Reserve( 200, 50 );
	mInfluenceObjectPool.Reserve( 500, 50 );
	mApplyObjectPool.Reserve( 200, 50 );

	mDeleteSkillArray.Reserve( 100 );
	mDeleteInfluenceArray.Reserve( 100 );
	mDeleteApplyObjectArray.Reserve( 100 );

	return true;
}

void cSkillManager::Open()
{

}

void cSkillManager::Close()
{
	///
	cSkillObject* pSkill = 0;
	cSkillObjectMap::cIterator i, iend;
	i = mDummySkillMap.Begin();
	iend = mDummySkillMap.End();
	for( ; i != iend; ++i )
	{
		pSkill = (cSkillObject*)(*i).mSecond;
		delete pSkill;
	}
	mDummySkillMap.Clear();

	/// 
	i = mSkillObjectMap.Begin();
	iend = mSkillObjectMap.End();
	for( ; i != iend; ++i )
	{
		pSkill = (cSkillObject*)(*i).mSecond;
		delete pSkill;
	}
	mSkillObjectMap.Clear();

	/// 
	i = mInfluenceMap.Begin();
	iend = mInfluenceMap.End();
	for( ; i != iend; ++i )
	{
		cInfluenceObject* pInf = (cInfluenceObject*)(*i).mSecond;
		delete pInf;
	}
	mInfluenceMap.Clear();

	i = mApplyObjectMap.Begin();
	iend = mApplyObjectMap.End();
	for( ; i != iend; ++i )
	{
		cApplyObject* pA = (cApplyObject*)(*i).mSecond;
		delete pA;
	}
	mApplyObjectMap.Clear();

	mDeleteSkillArray.Clear();
	mDeleteInfluenceArray.Clear();
	mDeleteApplyObjectArray.Clear();

	mDummyIndexGen.RestoreAll();
	mApplyIndexGen.RestoreAll();
}

unsigned long cSkillManager::CreateHeroSkillObject( unsigned long skillIdx, float speedFactor )
{
	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->Init( skillIdx, speedFactor ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	/// 새로운 더미키 생성
	unsigned long key = mDummyIndexGen.GenerateIndex();
	if( mDummySkillMap.Insert( key, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		mDummyIndexGen.RestoreIndex(key);
		return 0;
	}
	pSkillObj->SetManagedKey( key, true );

	return key;
}

cInfluenceObject* cSkillManager::CreateInfluenceObject( unsigned long uIdx, unsigned long infIdx )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( infIdx );
	if( pInfo == 0 )
		return 0;

	bool buff = (pInfo->mType == 1);

	cInfluenceObject* p = new cInfluenceObject;
	if( p == 0 || p->Init( uIdx, infIdx, buff ) == false )
	{
		delete p;
		return 0;
	}

	if( mInfluenceMap.Insert( uIdx, p ) == false )
	{
		assert(0);
		delete p;
		return 0;
	}

	return p;
}

cApplyObject* cSkillManager::CreateApplyObject( unsigned char weaponType, eAPPLYDRAMA_TYPE type )
{
	/// 생성
	unsigned long key = mApplyIndexGen.GenerateIndex();
	cApplyObject* p = new cApplyObject;
	if( p->Init( key, weaponType, type ) == false )
	{
		delete p;
		mApplyIndexGen.RestoreIndex( key );
		return 0;
	}

	/// 추가
	if( mApplyObjectMap.Insert( key, p ) == false )
	{
		assert(0);
		delete p;
		mApplyIndexGen.RestoreIndex( key );
		return 0;
	}

	return p;
}

cApplyObject* cSkillManager::CreateApplyObject( unsigned long influenceIdx )
{
	/// 생성
	unsigned long key = mApplyIndexGen.GenerateIndex();
	cApplyObject* p = new cApplyObject;
	if( p->Init( key, influenceIdx ) == false )
	{
		delete p;
		mApplyIndexGen.RestoreIndex( key );
		return 0;
	}

	/// 추가
	if( mApplyObjectMap.Insert( key, p ) == false )
	{
		assert(0);
		delete p;
		mApplyIndexGen.RestoreIndex( key );
		return 0;
	}

	return p;
}

void cSkillManager::UpdateInfluenceObject()
{

}

void cSkillManager::DamageInfluenceObject()
{

}

void cSkillManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// 보유 스킬에 대한 쿨타임 처리
	cSkillMap::cIterator k, kend;
	k = mKeepSkillMap.Begin();
	kend = mKeepSkillMap.End();
	for( ; k != kend; ++k )
	{
		sKeepSkill* p = (sKeepSkill*)(*k).mSecond;

		/// 쿨타임 체크
		if( p->mRestTime != 0 )
		{
			if( p->mRestTime < deltaTime )
				p->mRestTime = 0;
			else
				p->mRestTime -= deltaTime;

			if( p->mRestTime <= 0 )
			{
				p->mStartTime = 0;
				p->mRestTime = 0;
				p->mTotalTime = 0;
			}
		}
	}

	cSkillObject* pSkill = 0;
	cSkillObjectMap::cIterator i, iend;

	/// Dummy 스킬 처리

	if( mDummySkillMap.IsEmpty() == false )
	{
		i = mDummySkillMap.Begin();
		iend = mDummySkillMap.End();
		for( ; i != iend; ++i )
		{
			pSkill = (cSkillObject*)(*i).mSecond;
			pSkill->Process( accumTime );
		}
	}

	/// 효과 처리
	if( mInfluenceMap.IsEmpty() == false )
	{
		cInfluenceObject* pInf = 0;
		i = mInfluenceMap.Begin();
		iend = mInfluenceMap.End();
		for( ; i != iend; ++i )
		{
			pInf = (cInfluenceObject*)(*i).mSecond;
			pInf->Process( accumTime );
		}
	}

	/// 인증 스킬 처리
	if( mSkillObjectMap.IsEmpty() == false )
	{
		i = mSkillObjectMap.Begin();
		iend = mSkillObjectMap.End();
		for( ; i != iend; ++i )
		{
			pSkill = (cSkillObject*)(*i).mSecond;
			pSkill->Process( accumTime );
		}
	}

	/// Apply Drama객체 처리
	if( mApplyObjectMap.IsEmpty() == false )
	{
		i = mApplyObjectMap.Begin();
		iend = mApplyObjectMap.End();
		for( ; i != iend; ++i )
		{
			cApplyObject* pA = (cApplyObject*)(*i).mSecond;
			pA->Process( accumTime );
		}
	}

	/// 삭제 목록에 등록된 객체에 대한 처리
	DeleteProcess();
}

void cSkillManager::DeleteProcess()
{
	/// 삭제 객체를 제거한다. (주의: Object참조는 이미 제거된 상태다)
	if( mDeleteSkillArray.IsEmpty() == false )
	{
		for( unsigned int i = 0, end = mDeleteSkillArray.GetSize(); i < end; ++i )
		{
			cSkillObject* p = (cSkillObject*)mDeleteSkillArray[i];
			if( p )
			{
				unsigned long key = p->GetManagedKey();
				if( p->IsDummySkill() )
				{
					mDummyIndexGen.RestoreIndex( key );
					mDummySkillMap.Erase( key );
				}
				else
				{
					mSkillObjectMap.Erase( key );
				}
				delete p;
			}
		}
		mDeleteSkillArray.Clear();
	}

	if( mDeleteInfluenceArray.IsEmpty() == false )
	{
		bool calc = false;
		for( unsigned int i = 0, end = mDeleteInfluenceArray.GetSize(); i < end; ++i )
		{
			cInfluenceObject* p = (cInfluenceObject*)mDeleteInfluenceArray[i];
			if( p )
			{
				if( p->IsHeroInfluence() )
					calc = true;

				unsigned long key = p->GetManagedKey();
				mInfluenceMap.Erase( key );

				delete p;
			}
		}
		mDeleteInfluenceArray.Clear();
	}

	if( mDeleteApplyObjectArray.IsEmpty() == false )
	{
		for( unsigned int i = 0, end = mDeleteApplyObjectArray.GetSize(); i < end; ++i )
		{
			cApplyObject* p = (cApplyObject*)mDeleteApplyObjectArray[i];
			if( p )
			{
				unsigned long key = p->GetManagedKey();
				mApplyIndexGen.RestoreIndex( key );
				mApplyObjectMap.Erase( key );
				delete p;
			}
		}
		mDeleteApplyObjectArray.Clear();
	}
}

void cSkillManager::StartCoolTime( unsigned long skillIdx, unsigned long restTime, float speedFactor )
{
	/*if( HERO->IsTransformMonster() )
	{
	StartCoolTime_Mon( skillIdx, restTime, speedFactor );
	return;
	}*/

	unsigned long idx = skillIdx;
	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		/// 기본공격 (보유스킬 절대번호)
		idx = NORMAL_ATTACK_SKILL;
	}

	/// 스킬 사용
	sKeepSkill* p = (sKeepSkill*)mKeepSkillMap.GetAt( idx );
	if( p == 0 )
	{
		assert(0);
		return;
	}

	sPlayerSkillBaseInfo* pInfo = (sPlayerSkillBaseInfo*)SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	if( p->mStep >= pInfo->mStepCount )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo* pStep = &pInfo->mpSetpInfoArray[p->mStep];
	p->mTotalTime = (unsigned long)(pStep->mCoolTime * speedFactor);
	if( restTime == ULONG_MAX )
	{
		p->mRestTime = (unsigned long)(pStep->mCoolTime * speedFactor);
	}
	else
	{
		p->mRestTime = (unsigned long)(restTime * speedFactor);
	}

	p->mStartTime = THEAPP->GetAccumTime();
}

void cSkillManager::DeleteInfluenceObject( unsigned long uniqueIdx )
{
	cInfluenceObject* p = (cInfluenceObject*)mInfluenceMap.GetAt( uniqueIdx );
	if( p )
	{
		if( p->IsRemoved() == false )
		{
			p->Removed();

			unsigned long key = p->GetManagedKey();
			mInfluenceMap.Erase( key );

			delete p;
		}
	}
}

void cSkillManager::DeleteInfluenceObject( cInfluenceObject* p )
{
	if( p )
	{
		if( p->IsRemoved() == false )
		{
			p->Removed();

			mDeleteInfluenceArray.PushBack( p );
		}
	}
}

void cSkillManager::DeleteAll()
{
	///
	cSkillObject* pSkill = 0;
	cSkillObjectMap::cIterator i, iend;
	i = mDummySkillMap.Begin();
	iend = mDummySkillMap.End();
	for( ; i != iend; ++i )
	{
		pSkill = (cSkillObject*)(*i).mSecond;
		delete pSkill;
	}
	mDummySkillMap.Clear();

	/// 
	i = mSkillObjectMap.Begin();
	iend = mSkillObjectMap.End();
	for( ; i != iend; ++i )
	{
		pSkill = (cSkillObject*)(*i).mSecond;
		delete pSkill;
	}
	mSkillObjectMap.Clear();

	/// 
	i = mInfluenceMap.Begin();
	iend = mInfluenceMap.End();
	for( ; i != iend; ++i )
	{
		cInfluenceObject* pInf = (cInfluenceObject*)(*i).mSecond;
		delete pInf;
	}
	mInfluenceMap.Clear();

	i = mApplyObjectMap.Begin();
	iend = mApplyObjectMap.End();
	for( ; i != iend; ++i )
	{
		cApplyObject* pA = (cApplyObject*)(*i).mSecond;
		delete pA;
	}
	mApplyObjectMap.Clear();

	mDeleteSkillArray.Clear();
	mDeleteInfluenceArray.Clear();
	mDeleteApplyObjectArray.Clear();

	mDummyIndexGen.RestoreAll();
	mApplyIndexGen.RestoreAll();

	DeleteProcess();
}
