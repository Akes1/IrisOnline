#include "stdafx.h"
#include "SkillManager.h"

#include "SkillExecuter.h"
#include "damageprintmanager.h"

#include "hero.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "DramaturgyManager.h"
#include "WorldManager.h"

#include "Application.h"

#include "Protocol.h"
#include "ChatManager.h"
#include "Player_Common.h"

#include "Skill_Common.h"
#include "SkillScript.h"

#include "SkillWindow.h"
#include "SkillStepListWindow.h"
#include "GameUIManager.h"
#include "GameResourceManager.h"
#include "BuffWindow.h"
#include "QuickManager.h"
#include "ItemManager.h"
#include "DuelManager.h"
#include "PVPManager.h"

cSkillManager* cSkillManager::mpSkillManager = NULL;

cSkillManager::cSkillManager()
{
	mpSkillManager = this;

	mpSkillExecuter = new cSkillExecuter;
	mpDamagePrintManager = new cDamagePrintManager;

	::memset( mMonSkillCool, 0, sizeof(mMonSkillCool) );
}

cSkillManager::~cSkillManager()
{
	Close();

	///
	mSkillObjectPool.Clear();
	mInfluenceObjectPool.Clear();
	mChannelingSkillObjectPool.Clear();

	SAFE_DELETE( mpSkillExecuter );
	SAFE_DELETE( mpDamagePrintManager );
}

bool cSkillManager::Init()
{
	mSkillObjectPool.Reserve( 200, 50 );
	mInfluenceObjectPool.Reserve( 500, 50 );
	mApplyObjectPool.Reserve( 200, 50 );
	mChannelingSkillObjectPool.Reserve( 200, 50 );

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
	if( mpSkillExecuter )
		mpSkillExecuter->Close();

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

	ClearAllKeepSkill();

	mDummyIndexGen.RestoreAll();
	mApplyIndexGen.RestoreAll();

	///
	ClearCoolTimeMon();
}

unsigned long cSkillManager::CreateHeroSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, cBaseObject* pTarget, float speedFactor )
{
//	cSkillObject* pSkillObj = new cSkillObject;
	cSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		pSkillObj = new cSkillObject;
	}

	if( pSkillObj->Init( skillIdx, skillStep, pUser, pTarget, speedFactor ) == false )
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

	mpSkillExecuter->SetCurrentHeroSkillObject( pSkillObj );

	return key;
}

unsigned long cSkillManager::CreateHeroSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor )
{
//	cSkillObject* pSkillObj = new cSkillObject;
	cSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		pSkillObj = new cSkillObject;
	}

	if( pSkillObj->InitField( skillIdx, skillStep, pUser, fieldPos, speedFactor ) == false )
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

	mpSkillExecuter->SetCurrentHeroSkillObject( pSkillObj );

	return key;
}

unsigned long cSkillManager::CreateHeroMapChangeSkillObject( unsigned long skillIdx, cBaseObject* pUser )
{
	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitMapChange( skillIdx, pUser ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	unsigned long key = mDummyIndexGen.GenerateIndex();
	if( mDummySkillMap.Insert( key, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		mDummyIndexGen.RestoreIndex(key);
		return 0;
	}
	pSkillObj->SetManagedKey( key, true );

	mpSkillExecuter->SetCurrentHeroSkillObject( pSkillObj );

	return key;
}

unsigned long cSkillManager::CreateHeroVehicleSkillObject( unsigned long skillIdx, cBaseObject* pUser )
{
	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitVehicle( skillIdx, pUser ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	unsigned long key = mDummyIndexGen.GenerateIndex();
	if( mDummySkillMap.Insert( key, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		mDummyIndexGen.RestoreIndex(key);
		return 0;
	}
	pSkillObj->SetManagedKey( key, true );

	mpSkillExecuter->SetCurrentHeroSkillObject( pSkillObj );

	return key;
}

unsigned long cSkillManager::CreateHeroBlinkSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser )
{
	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitBlink( skillIdx, skillStep, pUser ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	unsigned long key = mDummyIndexGen.GenerateIndex();
	if( mDummySkillMap.Insert( key, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		mDummyIndexGen.RestoreIndex(key);
		return 0;
	}
	pSkillObj->SetManagedKey( key, true );

	mpSkillExecuter->SetCurrentHeroSkillObject( pSkillObj );

	return key;
}

void cSkillManager::CreateMapChangeSkillObject( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pUser )
{
	if( pUser && pUser == HERO )
	{
		assert(0);
		return;
	}

	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitMapChange( skillIdx, pUser ) == false )
	{
		delete pSkillObj;
		return;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return;
	}
	pSkillObj->SetManagedKey( uIdx, false );
}

void cSkillManager::CreateVehicleSkillObject( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pUser )
{
	if( pUser && pUser == HERO )
	{
		assert(0);
		return;
	}

	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitVehicle( skillIdx, pUser ) == false )
	{
		delete pSkillObj;
		return;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return;
	}
	pSkillObj->SetManagedKey( uIdx, false );
}

void cSkillManager::CreateBlinkSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 blinkPos )
{
	if( pUser && pUser == HERO )
	{
		assert(0);
		return;
	}

	cSkillObject* pSkillObj = new cSkillObject;
	if( pSkillObj->InitBlink( skillIdx, skillStep, pUser ) == false )
	{
		delete pSkillObj;
		return;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return;
	}
	pSkillObj->SetManagedKey( uIdx, false );
	pSkillObj->SetBlinkPos( blinkPos );
}

void cSkillManager::CreateSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, cBaseObject* pTarget, float speedFactor )
{
	if( pUser && pUser == HERO )
	{
		assert(0);
		return;
	}

//	cSkillObject* pSkillObj = new cSkillObject;
	cSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		pSkillObj = new cSkillObject;
	}

	if( pSkillObj->Init( skillIdx, skillStep, pUser, pTarget, speedFactor ) == false )
	{
		delete pSkillObj;
		return;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return;
	}
	pSkillObj->SetManagedKey( uIdx, false );
}

void cSkillManager::CreateSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor )
{
	if( pUser && pUser == HERO )
	{
		assert(0);
	}

//	cSkillObject* pSkillObj = new cSkillObject;
	cSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		pSkillObj = new cSkillObject;
	}

	if( pSkillObj->InitField( skillIdx, skillStep, pUser, fieldPos, speedFactor ) == false )
	{
		delete pSkillObj;
		return;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return;
	}
	pSkillObj->SetManagedKey( uIdx, false );
}

cSkillObject* cSkillManager::CreateRevisionSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, unsigned char attackerType, unsigned long dramaKey, bool tranMon )
{
	cSkillObject* pSkillObj = new cSkillObject;

	if( pSkillObj->RevisionInit( skillIdx, skillStep, attackerType, dramaKey, tranMon ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return 0;
	}
	pSkillObj->SetManagedKey( uIdx, false );

	return pSkillObj;
}

cSkillObject* cSkillManager::CreateRevisionChannelingSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, unsigned char attackerType, unsigned long dramaKey, bool tranMon, float fieldX, float fieldY, unsigned long playerIdx )
{
	cChannelingSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		assert(0);
		return 0;
	}

	if( pSkillObj->RevisionInit( skillIdx, skillStep, attackerType, dramaKey, tranMon, fieldX, fieldY, playerIdx ) == false )
	{
		delete pSkillObj;
		return 0;
	}

	if( mSkillObjectMap.Insert( uIdx, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		return 0;
	}
	pSkillObj->SetManagedKey( uIdx, false );

	return pSkillObj;
}

cSkillObject* cSkillManager::CreateNoneStopObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser )
{
//	cSkillObject* pSkillObj = new cSkillObject;
	cSkillObject* pSkillObj = NULL;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo && pInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		pSkillObj = new cChannelingSkillObject;
	}
	else
	{
		pSkillObj = new cSkillObject;
	}

	if( pSkillObj->Init( skillIdx, skillStep, pUser, NULL, 1.0f ) == false )
	{
		delete pSkillObj;
		return NULL;
	}

	/// 새로운 더미키 생성
	unsigned long key = mDummyIndexGen.GenerateIndex();
	if( mDummySkillMap.Insert( key, pSkillObj ) == false )
	{
		assert(0);
		delete pSkillObj;
		mDummyIndexGen.RestoreIndex(key);
		return NULL;
	}
	pSkillObj->SetManagedKey( key, true );

	return pSkillObj;
}

cInfluenceObject* cSkillManager::CreateInfluenceObject( unsigned long uIdx, unsigned long infIdx, unsigned long skillIdx, cBaseObject* pTarget, unsigned char deleteType )
{
	if( pTarget == 0 ) 
		return 0;

	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( infIdx );
	if( pInfo == 0 )
		return 0;

	bool buff = (pInfo->mType != eINFLUENCETYPE_DEBUF);

	cInfluenceObject* p = new cInfluenceObject;
	if( p == 0 || p->Init( uIdx, infIdx, skillIdx, buff, pTarget, deleteType ) == false )
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

	/// 071024 PKH 스테이터스 재계산
	if( pTarget->GetObjectType() == eOBJECTTYPE_HERO )
		HERO->CalcStatus();

	return p;
}

cInfluenceObject* cSkillManager::CreateInfluenceObjectEx( unsigned long uIdx, unsigned long infIdx, unsigned long skillIdx, cBaseObject* pTarget, unsigned char deleteType, unsigned long restTime )
{
	if( pTarget == 0 ) 
		return 0;

	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( infIdx );
	if( pInfo == 0 )
		return 0;

	bool buff = (pInfo->mType != eINFLUENCETYPE_DEBUF);

	cInfluenceObject* p = new cInfluenceObject;
	if( p == 0 || p->Init( uIdx, infIdx, skillIdx, buff, pTarget, deleteType ) == false )
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

	/// 071024 PKH 스테이터스 재계산
	if( pTarget->GetObjectType() == eOBJECTTYPE_HERO )
		HERO->CalcStatus();

	p->SetRestTime( restTime );

	return p;
}


cApplyObject* cSkillManager::CreateApplyObject( cBaseObject* pUser, cBaseObject* pTarget, unsigned char weaponType, eAPPLYDRAMA_TYPE type )
{
	if( pTarget == 0 )
		return 0;

	sObject target;
	target.index = pTarget->GetObjectID();
	target.type = pTarget->GetObjectType();

	/// 생성
	unsigned long key = mApplyIndexGen.GenerateIndex();
	cApplyObject* p = new cApplyObject;
	if( pUser )
	{
		sObject user;
		user.index = pUser->GetObjectID();
		user.type = pUser->GetObjectType();

		if( p->Init( key, &user, &target, weaponType, type ) == false )
		{
			delete p;
			mApplyIndexGen.RestoreIndex( key );
			return 0;
		}
	}
	else
	{
		if( p->Init( key, 0, &target, weaponType, type ) == false )
		{
			delete p;
			mApplyIndexGen.RestoreIndex( key );
			return 0;
		}
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

cApplyObject* cSkillManager::CreateApplyObject( cBaseObject* pTarget, unsigned long influenceIdx )
{
	if( pTarget == 0 )
		return 0;

	/// 생성
	unsigned long key = mApplyIndexGen.GenerateIndex();
	cApplyObject* p = new cApplyObject;
	if( p->Init( key, pTarget, influenceIdx ) == false )
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

cApplyObject* cSkillManager::CreateActApplyObject( cBaseObject* pTarget, unsigned long actIdx )
{
	if( pTarget == 0 )
		return 0;

	unsigned long key = mApplyIndexGen.GenerateIndex();
	cApplyObject* p = new cApplyObject;
	if( p->InitAct( key, pTarget, actIdx) == false )
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
			if( p->mRestTime < (long)deltaTime )
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

	/// 몬스터 변신시의 쿨타임 처리
	if( HERO && HERO->IsTransformMonster() == true )
	{
		for( int i = 0; i < eMONSTERATTACK_MAX; ++i )
		{
			if( mMonSkillCool[i].mRestTime != 0 )
			{
				if( mMonSkillCool[i].mRestTime < (long)deltaTime )
					mMonSkillCool[i].mRestTime = 0;
				else
					mMonSkillCool[i].mRestTime -= deltaTime;

				if( mMonSkillCool[i].mRestTime <= 0 )
				{
					mMonSkillCool[i].mStartTime = 0;
					mMonSkillCool[i].mRestTime = 0;
					mMonSkillCool[i].mTotalTime = 0;
				}
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
				if( p == mpSkillExecuter->GetCurrentHeroSkillObject() )
					mpSkillExecuter->SetCurrentHeroSkillObject( 0 );

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

		/// 071024 PKH 스테이터스 재계산
		if( calc )
			HERO->CalcStatus();
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


bool cSkillManager::ProcessLButtonClick( int mouseX, int mouseY )
{
	cBaseObject* pObj = 0;
	if( SKILLEXECUTER->IsSkillCharge() == false )
	{
		eOBJECTTYPE type = (eOBJECTTYPE)OBJECTMAN->ObjectPicking( mouseX, mouseY, &pObj );
		if( pObj )
		{
			switch( type )
			{
			case eOBJECTTYPE_PLAYER:
				{
					cBaseObject* curTarget = HERO->GetTargetObject();
					if( pObj == curTarget )
					{
						if( DUELMAN->IsDuelEnemy( (cPlayer*)curTarget ) ||
							PVPMAN->IsEnableAttack( (cPlayer*)curTarget ) )
						{
							HERO->SetWaitMoveFlag( eWAITMOVE_NONE );
							mpSkillExecuter->ExecuteAttack( pObj );
							return true;
						}
					}
				}
				break;
			case eOBJECTTYPE_MONSTER:
				{
					cBaseObject* curTarget = HERO->GetTargetObject();
					if( pObj == curTarget )
					{
						HERO->SetWaitMoveFlag( eWAITMOVE_NONE );
						mpSkillExecuter->ExecuteAttack( pObj );
						return true;
					}
				}
				break;
			default:
				break;
			}
		}
		return false;
	}

	/// 스킬 장착
	/// 바닥 기준 스킬이 장착인 경우
	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	if( SKILLEXECUTER->IsFieldSkillCharge() == true )
	{
		NiPoint3 pos;
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
		{
			assert(0);

			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
		}
		else
		{
			sAfterAction* action = HERO->GetCurrentAfterAction();
			if( action->mActionType == eAfterAction_UseFieldSkill )
				SKILLEXECUTER->ExecuteSkill( action->mExData1, (unsigned char)action->mExData2, pos );
			else
			{
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, pos );
			}
		}

		return true;
	}

	///
	eOBJECTTYPE type = (eOBJECTTYPE)OBJECTMAN->ObjectPicking( mouseX, mouseY, &pObj );
	if( pObj )
	{
		cBaseObject* curTarget = HERO->GetTargetObject();
		if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
		{
			/// 신규 타겟 등록후 발동
			HERO->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );
		}
		else
		{
			/// 타겟 변경 없이 발동
			cBaseObject* target = OBJECTMAN->GetObject( (unsigned char)type, pObj->GetObjectID() );
			sAfterAction* action = HERO->GetCurrentAfterAction();
			if( action )
			{	
				if( action->mActionType == eAfterAction_UseTargetSkill )
					SKILLEXECUTER->ExecuteSkill( action->mExData1, (unsigned char)action->mExData2, target );
				else
				{
					SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );
				}
			}
			else
			{
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );
			}
		}
	}

	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );

	return true;
}

bool cSkillManager::ProcessLButtonDClick( int mouseX, int mouseY )
{
	if( SKILLEXECUTER->IsSkillCharge() == false )
	{
		/// 기본 공격
		cBaseObject* pObj = 0;
		eOBJECTTYPE type = (eOBJECTTYPE)OBJECTMAN->ObjectPicking( mouseX, mouseY, &pObj );
		if( pObj )
		{
			switch( type )
			{
			case eOBJECTTYPE_PLAYER:
				{
					cBaseObject* curTarget = HERO->GetTargetObject();
					if( pObj == curTarget )
					{
						if( DUELMAN->IsDuelEnemy( (cPlayer*)curTarget ) ||
							PVPMAN->IsEnableAttack( (cPlayer*)curTarget ) )
						{
							HERO->SetWaitMoveFlag( eWAITMOVE_NONE );
							mpSkillExecuter->ExecuteAttack( pObj );
							return true;
						}
					}
				}
				break;
			case eOBJECTTYPE_MONSTER:
				{
					cBaseObject* curTarget = HERO->GetTargetObject();
					if( pObj == curTarget )
					{
						if( PVPMAN && PVPMAN->IsEnableBattle() )
						{
							if( ((cMonster*)pObj)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pObj)->GetPvPTeamType() == HERO->GetPVPTeam() )
								return false;
						}

						HERO->SetWaitMoveFlag( eWAITMOVE_NONE );
						mpSkillExecuter->ExecuteAttack( pObj );
						return true;
					}
				}
				break;
			default:
				break;
			}
		}
		return false;
	}

	sChargeInfo chargeInfo = SKILLEXECUTER->GetChargeSkill();
	if( SKILLEXECUTER->IsFieldSkillCharge() == true )
	{
		NiPoint3 pos;
		if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
		{
			assert(0);
			SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );
		}
		else
		{
			sAfterAction* action = HERO->GetCurrentAfterAction();
			if( action->mActionType == eAfterAction_UseFieldSkill )
				SKILLEXECUTER->ExecuteSkill( action->mExData1, (unsigned char)action->mExData2, pos );
			else
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, pos );
		}

		return true;
	}

	/// skill charge
	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)OBJECTMAN->ObjectPicking( mouseX, mouseY, &pObj );
	if( pObj )
	{
		cBaseObject* curTarget = HERO->GetTargetObject();
		if( curTarget == 0 || curTarget->GetState() == eOBJECT_STATE_DIE )
		{
			/// 신규 타겟 등록후 발동
			HERO->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );
		}
		else
		{
			/// 타겟 변경 없이 발동
			cBaseObject* target = OBJECTMAN->GetObject( (unsigned char)type, pObj->GetObjectID() );
			sAfterAction* action = HERO->GetCurrentAfterAction();
			if( action )
			{	
				if( action->mActionType == eAfterAction_UseTargetSkill )
					SKILLEXECUTER->ExecuteSkill( action->mExData1, (unsigned char)action->mExData2, target );
				else
					SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );
			}
			else
				SKILLEXECUTER->ExecuteSkill( chargeInfo.mSkillIdx, chargeInfo.mSkillStep, target );

		}
	}

	SKILLEXECUTER->ChargeOff( chargeInfo.mSkillIdx, chargeInfo.mSkillStep );

	return true;
}

/// game in 시 skill UI 세팅
void cSkillManager::InitSkillUI()
{
	cSkillMap::cIterator i = mKeepSkillMap.Begin();
	cSkillMap::cIterator end = mKeepSkillMap.End();
	
	for( ; i != end; ++i )
	{
		unsigned long	skillIdx = (unsigned long)(*i).mFirst;
		sKeepSkill*		keepSkill = (sKeepSkill*)(*i).mSecond;

		if( keepSkill )
		{
			UpdateSkillStep( skillIdx, keepSkill->mStep, false );
			StartCoolTime( skillIdx, keepSkill->mRestTime );
		}
		else
			assert(0);
	}
}

/// load, mapchange 시 스킬 정보 세팅
void cSkillManager::InitKeepSkill( sHaveSkill* pdata )
{
	if( pdata == 0 )
	{
		assert(0);
		return;
	}

	sKeepSkill* pSkill = new sKeepSkill;
	pSkill->mStep = pdata->mSkillStep;
	pSkill->mRestTime = pdata->mRestCoolTime;

	/// casting time setting
	sPlayerSkillBaseInfo* p = SKILLSCRIPT->GetPlayerSkillInfo( pdata->mSkillIdx );
	if( p == 0 )
	{
		assert(0);
		return;
	}
	if( p->mStepCount <= pdata->mSkillStep )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo stepInfo =  p->mpSetpInfoArray[pdata->mSkillStep];
	pSkill->mCastingTime = stepInfo.mCastingTime;

	if( mKeepSkillMap.Insert( pdata->mSkillIdx, pSkill ) == false )
	{
		assert(0);
	}

	if( p->mType != eSKILLTYPE_PASSIVE )
		DRAMATURGYMAN->DirectLoadPlayerDramaObj( pdata->mSkillIdx );
}

void cSkillManager::AddKeepSkill( unsigned long skillIdx, unsigned char skillStep )
{
	sKeepSkill* pSkill = new sKeepSkill;
	pSkill->mStep = skillStep;
	pSkill->mRestTime = 0;

	/// casting time setting
	sPlayerSkillBaseInfo* p = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( p == 0 )
	{
		assert(0);
	}
	if( p->mStepCount <= skillStep )
	{
		assert(0);
	}
	sPlayerSkillStepInfo stepInfo =  p->mpSetpInfoArray[skillStep];
	pSkill->mCastingTime = stepInfo.mCastingTime;

	if( mKeepSkillMap.Insert( skillIdx, pSkill ) == false )
	{
		assert(0);
	}

	if( p->mType != eSKILLTYPE_PASSIVE )
		DRAMATURGYMAN->DirectLoadPlayerDramaObj( skillIdx );

	UpdateSkillStep( skillIdx, skillStep, false );
}

void cSkillManager::UpdateAllSkill()
{
	cSkillWindow* pWin = (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
	if( pWin == 0 )
	{
		assert(0);
		return;
	}

	/// 정보 갱신
	cSkillMap::cIterator i = mKeepSkillMap.Begin();
	cSkillMap::cIterator iend = mKeepSkillMap.End();
	for( ;i!=iend;i++ )
	{
		unsigned long idx = (*i).mFirst;
		sKeepSkill* p = (sKeepSkill*)(*i).mSecond;
		pWin->UpdateSkillStep( idx, p->mStep );
	}
}

void cSkillManager::ClearAllKeepSkill()
{
	cSkillMap::cIterator i = mKeepSkillMap.Begin();
	cSkillMap::cIterator iend = mKeepSkillMap.End();
	for( ;i!=iend;i++ )
	{
		sKeepSkill* p = (sKeepSkill*)(*i).mSecond;
		delete p;
	}
	mKeepSkillMap.Clear();
}


void cSkillManager::UpdateSkillStep( unsigned long idx, unsigned char step, bool stepUpdateAuto )
{
	sKeepSkill* p = (sKeepSkill*)mKeepSkillMap.GetAt( idx );
	if( p )
	{
		/// casting time setting
		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( idx );
		if( pInfo == 0 )
		{
			assert(0);
		}
		if( pInfo->mStepCount <= step )
		{
			assert(0);
		}
		sPlayerSkillStepInfo stepInfo =  pInfo->mpSetpInfoArray[step];

		p->mCastingTime = stepInfo.mCastingTime;
		p->mStep = step;

		/// skill window 변경
		cSkillWindow* pWin = GAMEUI->GetSkillWindow();
		if( pWin )
		{
			pWin->UpdateSkillStep( idx, p->mStep );

			cSkillStepListWindow* pStepListWin = pWin->GetStepListWindow();
			if( pStepListWin )
				pStepListWin->ResetSkillInfo();
		}

		if( stepUpdateAuto == true )
		{
			QUICKMAN->UpdateSkillStep( idx, step );
		}
		else
		{
			// 해당스킬이 오라 이면, 퀵슬롯에 자동갱신한다
			if( pInfo->mShotType == eSHOTTYPE_AURA )
			{
				QUICKMAN->UpdateUI();
			}
		}
	}
	else
	{
		assert(0);
	}
}

void cSkillManager::NetworkMsgParser( char* pMsg )
{
	MSGROOT* pTemMsg = (MSGROOT*)pMsg;

	switch( pTemMsg->Protocol )
	{
	case NM_SKILL_USED_RES:
	case NM_SKILL_USED_MON_RES:
	case NM_SKILL_POS_USED_RES:
	case NM_SKILL_POS_USED_MON_RES:
	case NM_SKILL_RUSHAFTER_RES:
		{
			MSG_RES_SKILL_USED* pResMsg = (MSG_RES_SKILL_USED*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mDummySkillMap.GetAt( pResMsg->mManagedKey );

			/// 발동 성공
			if( pResMsg->ErrorCode == 0 )
			{
				/// mp setting
				HERO->MPChange( pResMsg->mObjectMP, pResMsg->mMaxMP );

				if( pObj )
				{
					/// 인증 성공이므로 관리형태로 이동
					mDummyIndexGen.RestoreIndex( pResMsg->mManagedKey );
					mDummySkillMap.Erase( pResMsg->mManagedKey );

					/// 관리 구조에 포함 시킨다.
					if( mSkillObjectMap.Insert( pResMsg->mSkillIndex, pObj ) )
					{
						pObj->SetManagedKey( pResMsg->mSkillIndex, false );
						
						if( pTemMsg->Protocol !=  NM_SKILL_RUSHAFTER_RES )
						{
							if( pTemMsg->Protocol == NM_SKILL_USED_MON_RES  || pTemMsg->Protocol == NM_SKILL_POS_USED_MON_RES )
								SKILLMAN->StartCoolTimeMon( pResMsg->mSkillClassIndex );
							else 
								SKILLMAN->StartCoolTime( pResMsg->mSkillClassIndex );
						}
					}
					else
					{
						assert(0);
						SKILLMAN->DeleteSkillObject( pObj );

						if( HERO->IsReadyUseSkill() == false )
							HERO->SetReadyUseSkill( true );
						return;
					}
				}
			}
			else	/// 발동 실패
			{
				switch( pResMsg->ErrorCode )
				{
				case SKILL_USED_ERR_SKILLTYPE:	/// 스킬 종류 / 인덱스가 틀림
				case SKILL_USED_ERR_SCRIPT:		/// 스크립트 오류
				case SKILL_USED_ERR_UNIQUEIDX:	/// 스킬 객체 고유번호 오류
				case SKILL_USED_ERR_OBJECTINIT:	/// 스킬 객체 초기화 실패
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6200 ) );
					}
					break;

				case SKILL_USED_ERR_ATTAKCERPOS:	/// 공격자 위치가 안맞음
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6201 ) );
					}
					break;
				case SKILL_USED_ERR_USING:			/// 스킬 사용중
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6202 ) );
					}
					break;	
				case SKILL_USED_ERR_TARGETDIST:		/// 스킬 사거리밖에 타겟
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6203 ) );
					}
					break;
				case SKILL_USED_ERR_COOLTIME:		/// 쿨타임
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6204 ) );
					}
					break;
				case SKILL_USED_ERR_ATTACKERSTATE:	/// 공격자 상태 변경 불가
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6205 ) );
					}
					break;
				case SKILL_USED_ERR_USEVALUE:		/// 소모값 부족
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETOBJECT:	/// 타겟 객체 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETSELECT:	/// 타겟선정 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_PASSIVE:		/// 패시브 사용
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6208 ) );
					}
					break;
				case SKILL_USED_ERR_DONTHAVE:		/// 보유스킬이 아님
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6209 ) );
					}
					break;
				case SKILL_USED_ERR_EQUIPMENT:		/// 장비 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6210 ) );
					}
					break;
				case SKILL_USED_ERR_CHG_MONSTER:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6213 ) );
					}
					break;
				case SKILL_USED_ERR_APPLYVALUE_STATE:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_RUSH_OBSTRUCTION:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6219 ) );
					}
					break;
				}

				/// 실패 등록
				if( pObj )
                    pObj->VerifyFaile( false );
			}
		}
		break;
	case NM_SKILL_ITEMVEHICLE_SYN:
	case NM_SKILL_ITEMMAPCHANGE_SYN:
	case NM_SKILL_USED_SYN:
	case NM_SKILL_CAST_USED_SYN:
	case NM_SKILL_CASHMAPCHANGE_SYN:
	case NM_SKILL_RUSHAFTER_SYN:
		{
			MSG_SYN_SKILL_USED* pSynMsg = (MSG_SYN_SKILL_USED*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			cBaseObject* pTarget = OBJECTMAN->GetObject( &pSynMsg->mTarget );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->FixPos( pSynMsg->mAttackerPosX, pSynMsg->mAttackerPosY );
			if( pTarget )
				pTarget->FixPos( pSynMsg->mTargetPosX, pSynMsg->mTargetPosY );

			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mObjectMaxMP );

			/// 기본 공격
			if( pSynMsg->mSkillClassIndex < NORMAL_ATTACK_SKILL_MAX )
			{
				cPlayer* pPlayer = (cPlayer*)pAttacker;
				pPlayer->PlayerAttack( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex, pTarget );
			}
			else	/// 스킬 공격
			{
				cPlayer* pPlayer = (cPlayer*)pAttacker;
				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( pSynMsg->mSkillClassIndex );
				if( pInfo )
				{
					sPlayerSkillStepInfo stepInfo = pInfo->mpSetpInfoArray[0];
					if( stepInfo.mApplyValueType1 == eSTATUSPLUS_MAPCHANGE || stepInfo.mApplyValueType1 == eSTATUSPLUS_CASHMAPCHANGE )
					{
						pPlayer->PlayerMapChangeSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex );
					}
					else
					{
						sInfluenceScript* pInfluence = SKILLSCRIPT->GetInfluenceInfo( stepInfo.mInfulenceIdx );
						if( pInfluence )
						{
							if( pInfluence->mStatusPlusIdx1 == eSTATUSPLUS_VEHICLE ||
								pInfluence->mStatusPlusIdx2 == eSTATUSPLUS_VEHICLE ||
								pInfluence->mStatusPlusIdx3 == eSTATUSPLUS_VEHICLE )
							{
									pPlayer->PlayerVehicleSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex );
							}
							else
							{
								pPlayer->PlayerUseSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pTarget );
							}

						}
						else
						{
							pPlayer->PlayerUseSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pTarget );
						}
					}
				}
			}
		}
		break;
	case NM_SKILL_NONSTOP_MON_RES:
	case NM_SKILL_NONSTOP_RES:
		{
			MSG_RES_SKILL_NONSTOP* pResMsg = (MSG_RES_SKILL_NONSTOP*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mDummySkillMap.GetAt( pResMsg->mManagedKey );

			/// 발동 성공
			if( pResMsg->ErrorCode == 0 )
			{
				/// mp setting
				HERO->MPChange( pResMsg->mObjectMP, pResMsg->mMaxMP );

				if( pObj )
				{
					if( pResMsg->mCount == 0 )
					{
						pObj->VerifySuccess();
						return;
					}

					if( pTemMsg->Protocol == NM_SKILL_NONSTOP_MON_RES )
						SKILLMAN->StartCoolTimeMon( pResMsg->mSkillClassIndex );
					else 
						SKILLMAN->StartCoolTime( pResMsg->mSkillClassIndex );

					sTargetApply* pTargetApply = pResMsg->mTarget;
					for( unsigned char i=0; i<pResMsg->mCount; ++i )
					{
						cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetApply->mTarget );
						if( pTarget )
						{
							if( pTarget == HERO )
							{
								if( pTargetApply->mInfluenceMiss == true )
									pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
							}
						}
						pObj->AddTargetInfo( &pTargetApply->mTarget );
					}

					pObj->VerifySuccess();
				}
			}
			else	/// 발동 실패
			{
				switch( pResMsg->ErrorCode )
				{
				case SKILL_USED_ERR_SKILLTYPE:	/// 스킬 종류 / 인덱스가 틀림
				case SKILL_USED_ERR_SCRIPT:		/// 스크립트 오류
				case SKILL_USED_ERR_UNIQUEIDX:	/// 스킬 객체 고유번호 오류
				case SKILL_USED_ERR_OBJECTINIT:	/// 스킬 객체 초기화 실패
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6200 ) );
					}
					break;

				case SKILL_USED_ERR_ATTAKCERPOS:	/// 공격자 위치가 안맞음
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6201 ) );
					}
					break;
				case SKILL_USED_ERR_USING:			/// 스킬 사용중
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6202 ) );
					}
					break;	
				case SKILL_USED_ERR_TARGETDIST:		/// 스킬 사거리밖에 타겟
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6203 ) );
					}
					break;
				case SKILL_USED_ERR_COOLTIME:		/// 쿨타임
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6204 ) );
					}
					break;
				case SKILL_USED_ERR_ATTACKERSTATE:	/// 공격자 상태 변경 불가
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6205 ) );
					}
					break;
				case SKILL_USED_ERR_USEVALUE:		/// 소모값 부족
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETOBJECT:	/// 타겟 객체 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETSELECT:	/// 타겟선정 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_PASSIVE:		/// 패시브 사용
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6208 ) );
					}
					break;
				case SKILL_USED_ERR_DONTHAVE:		/// 보유스킬이 아님
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6209 ) );
					}
					break;
				case SKILL_USED_ERR_EQUIPMENT:		/// 장비 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6210 ) );
					}
					break;
				case SKILL_USED_ERR_CHG_MONSTER:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6213 ) );
					}
					break;
				case SKILL_USED_ERR_APPLYVALUE_STATE:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_RUSH_OBSTRUCTION:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6219 ) );
					}
					break;
				}

				/// 실패 등록
				if( pObj )
					pObj->VerifyFaile( false );
			}
		}
		break;
	case NM_SKILL_NONSTOP_TARGET_SYN:
	case NM_SKILL_NONSTOP_MON_TARGET_SYN:
		{
			MSG_SYN_SKILL_NONSTOP_TARGET* pSynMsg = (MSG_SYN_SKILL_NONSTOP_TARGET*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mMaxMP );

			/// 기본 공격
			if( pSynMsg->mSkillClassIndex < NORMAL_ATTACK_SKILL_MAX )
			{
				assert(0);
				return;
			}

			cSkillObject* pObj = SKILLMAN->CreateNoneStopObject( pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pAttacker );
			if( pObj == 0 )
			{
				assert(0);
				return;
			}

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pSynMsg->mTargetApply.mTarget );
			if( pTarget )
			{
				if( pTarget == HERO )
				{
					if( pSynMsg->mTargetApply.mInfluenceMiss == true )
						pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
				}
			}

			pObj->AddTargetInfo( &pSynMsg->mTargetApply.mTarget );
			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_NONSTOP_AROUND_SYN:
	case NM_SKILL_NONSTOP_MON_AROUND_SYN:
		{
			MSG_SYN_SKILL_NONSTOP_AROUND* pSynMsg = (MSG_SYN_SKILL_NONSTOP_AROUND*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mMaxMP );

			/// 기본 공격
			if( pSynMsg->mSkillClassIndex < NORMAL_ATTACK_SKILL_MAX )
			{
				assert(0);
				return;
			}

			cSkillObject* pObj = SKILLMAN->CreateNoneStopObject( pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pAttacker );
			if( pObj == 0 )
			{
				assert(0);
				return;
			}

			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_RUSH_RES:
	case NM_SKILL_RUSHBLINK_RES:
		{
			MSG_RES_SKILL_RUSH* pResMsg = (MSG_RES_SKILL_RUSH*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mDummySkillMap.GetAt( pResMsg->mManagedKey );

			/// 발동 성공
			if( pResMsg->ErrorCode == 0 )
			{
				/// mp setting
				HERO->MPChange( pResMsg->mObjectMP, pResMsg->mMaxMP );

				if( pObj )
				{
					SKILLMAN->StartCoolTime( pResMsg->mSkillClassIndex );

					pObj->VerifySuccess();

					cBaseObject* pTarget = OBJECTMAN->GetObject( pResMsg->mTarget.type, pResMsg->mTarget.index );
					if( pTarget != NULL )
						HERO->SetSkillRush( pResMsg->mPosX, pResMsg->mPosY, pResMsg->mTarget, pResMsg->mNextSkill, pResMsg->mNextStep, pResMsg->mBlink );
				}
			}
			else	/// 발동 실패
			{
				switch( pResMsg->ErrorCode )
				{
				case SKILL_USED_ERR_SKILLTYPE:	/// 스킬 종류 / 인덱스가 틀림
				case SKILL_USED_ERR_SCRIPT:		/// 스크립트 오류
				case SKILL_USED_ERR_UNIQUEIDX:	/// 스킬 객체 고유번호 오류
				case SKILL_USED_ERR_OBJECTINIT:	/// 스킬 객체 초기화 실패
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6200 ) );
					}
					break;

				case SKILL_USED_ERR_ATTAKCERPOS:	/// 공격자 위치가 안맞음
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6201 ) );
					}
					break;
				case SKILL_USED_ERR_USING:			/// 스킬 사용중
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6202 ) );
					}
					break;	
				case SKILL_USED_ERR_TARGETDIST:		/// 스킬 사거리밖에 타겟
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6203 ) );
					}
					break;
				case SKILL_USED_ERR_COOLTIME:		/// 쿨타임
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6204 ) );
					}
					break;
				case SKILL_USED_ERR_ATTACKERSTATE:	/// 공격자 상태 변경 불가
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6205 ) );
					}
					break;
				case SKILL_USED_ERR_USEVALUE:		/// 소모값 부족
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETOBJECT:	/// 타겟 객체 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETSELECT:	/// 타겟선정 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_PASSIVE:		/// 패시브 사용
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6208 ) );
					}
					break;
				case SKILL_USED_ERR_DONTHAVE:		/// 보유스킬이 아님
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6209 ) );
					}
					break;
				case SKILL_USED_ERR_EQUIPMENT:		/// 장비 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6210 ) );
					}
					break;
				case SKILL_USED_ERR_CHG_MONSTER:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6213 ) );
					}
					break;
				case SKILL_USED_ERR_APPLYVALUE_STATE:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_RUSH_OBSTRUCTION:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6219 ) );
					}
					break;
				}

				/// 실패 등록
				if( pObj )
					pObj->VerifyFaile( false );
			}
		}
		break;
	case NM_SKILL_RUSH_SYN:
	case NM_SKILL_RUSHBLINK_SYN:
		{
			MSG_SYN_SKILL_RUSH* pSynMsg = (MSG_SYN_SKILL_RUSH*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mMaxMP );

			/// 기본 공격
			if( pSynMsg->mSkillClassIndex < NORMAL_ATTACK_SKILL_MAX )
			{
				assert(0);
				return;
			}

			cSkillObject* pObj = SKILLMAN->CreateNoneStopObject( pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pAttacker );
			if( pObj == 0 )
			{
				assert(0);
				return;
			}

			pObj->VerifySuccess();

			if( pSynMsg->mAttacker.type == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pSynMsg->mAttacker.index );
				if( pPlayer != NULL )
				{
					if( pSynMsg->mBlink == true )
					{
						NiPoint3 pos;
						pos.x = pSynMsg->mPosX;
						pos.y = pSynMsg->mPosY;

						/// 높이값 계산
						if( WORLDMAN )
						{
							if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
								assert(NULL);
						}

						pPlayer->SetPos( pos );
					}
					else
					{
						cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pSynMsg->mTarget );
						if( pTarget != NULL )
						{
							pPlayer->Move( pTarget->GetXPos(), pTarget->GetYPos() );
							pPlayer->SetSkillRush( pSynMsg->mPosX, pSynMsg->mPosY, pSynMsg->mTarget );
						}
					}
				}
			}
		}
		break;
	case NM_SKILL_MONSTER_USED_SYN:
	case NM_SKILL_MONSTER_CAST_USED_SYN:
		{
			MSG_SYN_MONSTER_SKILL_USED* pSynMsg = (MSG_SYN_MONSTER_SKILL_USED*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_MONSTER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			cBaseObject* pTarget = OBJECTMAN->GetObject( &pSynMsg->mTarget );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->FixPos( pSynMsg->mAttackerPosX, pSynMsg->mAttackerPosY );

			cMonster* pMonster = (cMonster*)pAttacker;
			pMonster->MonsterAttack( pSynMsg->mSkillIndex, pSynMsg->mAttackType, pTarget, pSynMsg->mAttackSpeedFactor );
		}
		break;
	case NM_SKILL_POS_MONSTER_USED_SYN:
	case NM_SKILL_POS_MONSTER_CAST_USED_SYN:
		{
			MSG_SYN_POS_MONSTER_SKILL_USED* pSynMsg = (MSG_SYN_POS_MONSTER_SKILL_USED*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_MONSTER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->FixPos( pSynMsg->mAttackerPosX, pSynMsg->mAttackerPosY );

			cMonster* pMonster = (cMonster*)pAttacker;
			NiPoint3 targetPos( pSynMsg->mTargetPosX, pSynMsg->mTargetPosY, 0 );
			WORLDMAN->CalcHeight( &targetPos.z, targetPos.x, targetPos.y );
			pMonster->MonsterAttack( pSynMsg->mSkillIndex, pSynMsg->mAttackType, targetPos, 1.0f );
		}
		break;
	case NM_SKILL_ITEMVEHICLE_RES:
	case NM_SKILL_ITEMMAPCHANGE_RES:
	case NM_SKILL_CAST_USED_MON_RES:
	case NM_SKILL_CAST_USED_RES:
	case NM_SKILL_POS_CAST_USED_RES:
	case NM_SKILL_CASHMAPCHANGE_RES:
		{
			//MSG_RES_SKILL_USED
			MSG_RES_SKILL_USED* pResMsg = (MSG_RES_SKILL_USED*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mDummySkillMap.GetAt( pResMsg->mManagedKey );
			if( pObj == 0 || pObj->GetSkillIdx() != pResMsg->mSkillClassIndex )
			{
				assert(0);
				return;
			}

			/// 발동 성공
			if( pResMsg->ErrorCode == 0 )
			{
				/// 인증 성공이므로 관리형태로 이동
				mDummyIndexGen.RestoreIndex( pResMsg->mManagedKey );
				mDummySkillMap.Erase( pResMsg->mManagedKey );

				/// 관리 구조에 포함 시킨다.
				if( mSkillObjectMap.Insert( pResMsg->mSkillIndex, pObj ) )
				{
					pObj->SetManagedKey( pResMsg->mSkillIndex, false );
				}
				else
				{
					assert(0);
					return;
				}
			}
			else /// 캐스팅 요청 실패
			{
				switch( pResMsg->ErrorCode )
				{
				case SKILL_USED_ERR_SKILLTYPE:	/// 스킬 종류 / 인덱스가 틀림
				case SKILL_USED_ERR_SCRIPT:		/// 스크립트 오류
				case SKILL_USED_ERR_UNIQUEIDX:	/// 스킬 객체 고유번호 오류
				case SKILL_USED_ERR_OBJECTINIT:	/// 스킬 객체 초기화 실패
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6200 ) );
					}
					break;

				case SKILL_USED_ERR_ATTAKCERPOS:	/// 공격자 위치가 안맞음
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6201 ) );
					}
					break;
				case SKILL_USED_ERR_USING:			/// 스킬 사용중
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6202 ) );
					}
					break;	
				case SKILL_USED_ERR_TARGETDIST:		/// 스킬 사거리밖에 타겟
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6203 ) );
					}
					break;
				case SKILL_USED_ERR_COOLTIME:		/// 쿨타임
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6204 ) );
					}
					break;
				case SKILL_USED_ERR_ATTACKERSTATE:	/// 공격자 상태 변경 불가
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6205 ) );
					}
					break;
				case SKILL_USED_ERR_USEVALUE:		/// 소모값 부족
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETOBJECT:	/// 타겟 객체 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_TARGETSELECT:	/// 타겟선정 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6207 ) );
					}
					break;
				case SKILL_USED_ERR_PASSIVE:		/// 패시브 사용
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6208 ) );
					}
					break;
				case SKILL_USED_ERR_DONTHAVE:		/// 보유스킬이 아님
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6209 ) );
					}
					break;
				case SKILL_USED_ERR_EQUIPMENT:		/// 장비 오류
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6210 ) );
					}
					break;
				case SKILL_USED_ERR_CHG_MONSTER:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6213 ) );
					}
					break;
				case SKILL_USED_ERR_MAPCHANGE_STATE:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6218 ) );
					}
					break;
					break;
				case SKILL_USED_ERR_APPLYVALUE_STATE:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6206 ) );
					}
					break;
				case SKILL_USED_ERR_RUSH_OBSTRUCTION:
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6219 ) );
					}
					break;
				}

				/// 실패 등록
				pObj->VerifyFaile( true );

				if( pObj->GetSpecialType() == eSpecial_MapChange )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
				}
				else if( pObj->GetSpecialType() == eSpecial_Vehicle )
				{
					ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
				}
			}
		}
		break;
	case NM_SKILL_CAST_ACTIVE_SYN:
	case NM_SKILL_POS_CAST_ACTIVE_SYN:
		{
			MSG_SYN_SKILL_CAST_ACTIVE* pSynMsg = (MSG_SYN_SKILL_CAST_ACTIVE*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );;
			if( pSkillObj )
			{
				pSkillObj->EndCast();

				cBaseObject* pObj = (cBaseObject*)OBJECTMANAGER->GetObject( &pSynMsg->mAttacker );
				if( pObj )
				{
					if( pSynMsg->mAttacker.type == eOBJECTTYPE_HERO )
					{
						if( pSkillObj->GetSpecialType() == eSpecial_MapChange )
						{
							ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
						}
						else if( pSkillObj->GetSpecialType() == eSpecial_Vehicle )
						{	
							ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
						}
					}

					if( pSkillObj->GetSpecialType() > eSpecial_Field )
						return;

					///
					if( pSynMsg->mAttacker.type == eOBJECTTYPE_HERO )
						SKILLMAN->StartCoolTime( pSkillObj->GetSkillIdx() );
					else if( pSynMsg->mAttacker.type == eOBJECTTYPE_PLAYER )
						pObj->MPChange( pSynMsg->mObjectMP, pSynMsg->mObjectMaxMp );

					pObj->FixPos( pSynMsg->mAttackerPosX, pSynMsg->mAttackerPosY );
				}
			}
		}
		break;
	case NM_SKILL_CAST_ACTIVITY_FAIL_SYN:
		{
			MSG_SYN_SKILL_CAST_ACTIVITY_FAIL* pSynMsg = (MSG_SYN_SKILL_CAST_ACTIVITY_FAIL*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pSkillObj )
			{
				pSkillObj->VerifyFaile( true );
			}

			cBaseObject* pObject = OBJECTMANAGER->GetObject( &pSynMsg->mAttacker );
			if( pObject && pObject->GetObjectType() == eOBJECTTYPE_HERO )
			{
				unsigned long textIndex = 0;
				switch( pSynMsg->mFailCode )
				{
				case FAIL_SKILL_CASTING_MP_EMPTY:			textIndex = 6206;	break;
				case FAIL_SKILL_CASTING_DONT_TARGET:		textIndex = 6205;	break;
				case FAIL_SKILL_CASTING_DONT_ATTACKER:		textIndex = 6207;	break;
				case FAIL_SKILL_CASTING_DONT_MAPCHANGE:		textIndex = 98;		break;
				case FAIL_SKILL_CASTING_DONT_VEHICLE:		textIndex = 6206;	break;
				case FAIL_SKILL_CASTING_DONT_SCRIPT:		textIndex = 6200;	break;
				case FAIL_SKILL_CASTING_DONT_OBSTACLE:		textIndex = 6217;	break;
				case FAIL_SKILL_CASTING_DONT_DUEL_END:		textIndex = 0;		break;
				default: assert(0); return;
				}
				
				if( textIndex != 0 )
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );

				if( ITEMMAN->GetItemTry() == ITEM_TRY_MAPCHANGE )
					ITEMMAN->UnlockItemTry( ITEM_TRY_MAPCHANGE );
				else if( ITEMMAN->GetItemTry() == ITEM_TRY_VEHICLE )
					ITEMMAN->UnlockItemTry( ITEM_TRY_VEHICLE );
			}
		}
		break;
	case NM_SKILL_CAST_CANCEL_RES:
		{
			/// cancel이 성공한 경우에만 받는다.
			MSG_RES_SKILL_CAST_CANCEL* pResMsg = (MSG_RES_SKILL_CAST_CANCEL*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pResMsg->mSkillIndex );
			if( pSkillObj )
				pSkillObj->VerifyFaile();
		}
		break;
	case NM_SKILL_CAST_CANCEL_SYN:
		{
			/// cancel이 성공한 경우에만 받는다.
			MSG_SYN_SKILL_CAST_CANCEL* pSynMsg = (MSG_SYN_SKILL_CAST_CANCEL*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pSkillObj )
				pSkillObj->VerifyFaile( true );
		}
		break;
	case NM_SKILL_CHANNELING_CANCEL_RES:
		{
			/// cancel이 성공한 경우에만 받는다.
			MSG_RES_CHANNELING_SKILL_CANCEL* pResMsg = (MSG_RES_CHANNELING_SKILL_CANCEL*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pResMsg->mUniqueSkillIndex );
			if( pSkillObj )
				pSkillObj->VerifyFaile();
		}
		break;
	case NM_SKILL_CHANNELING_CANCEL_SYN:
		{
			/// cancel이 성공한 경우에만 받는다.
			MSG_SYN_CHANNELING_SKILL_CANCEL* pSynMsg = (MSG_SYN_CHANNELING_SKILL_CANCEL*)pMsg;

			cSkillObject* pSkillObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mUniqueSkillIndex );
			if( pSkillObj )
				pSkillObj->VerifyFaile( true );
		}
		break;
	case NM_SKILL_POS_USED_SYN:
	case NM_SKILL_POS_CAST_USED_SYN:
	case NM_SKILL_POS_USED_MON_SYN:
	case NM_SKILL_POS_CAST_USED_MON_SYN:
		{
			//MSG_SYN_POS_USED;
			MSG_SYN_POS_USED* pSynMsg = (MSG_SYN_POS_USED*)pMsg;

			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->FixPos( pSynMsg->mAttackerPosX, pSynMsg->mAttackerPosY );
			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mObjectMaxMP );

			/// 기본 공격
			if( pSynMsg->mSkillClassIndex < NORMAL_ATTACK_SKILL_MAX )
			{
				assert(0);
				return;
			}

			NiPoint3 pos;
			pos.x = pSynMsg->mTargetPosX;
			pos.y = pSynMsg->mTargetPosY;

			cPlayer* pPlayer = (cPlayer*)pAttacker;
			pPlayer->PlayerUseSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, pos );
		}
		break;
	case NM_SKILL_USED_POS_ERROR_RES:
		{
			MSG_RES_SKILL_POS* pResMsg = (MSG_RES_SKILL_POS*)pMsg;

			HERO->FixPos( pResMsg->mAttackerPosX, pResMsg->mAttackerPosY );

			cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pResMsg->mTarget );
			if( pTarget )
			{
				pTarget->FixPos( pResMsg->mTargetPosX, pResMsg->mTargetPosY );
			}
		}
		break;
	//case NM_SKILL_DAMAGE_HP_SYN:
	//case NM_SKILL_DAMAGE_MP_SYN:
	//	{
	//		MSG_SYN_SKILL_DAMAGE* pSynMsg = (MSG_SYN_SKILL_DAMAGE*)pMsg;

	//		cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
	//		if( pObj == 0 )
	//		{
	//			if( pSynMsg->mAttackerDie == true )
	//				return;

	//			/// 스킬 객체가 없는 경우..
	//			/// 연출 보정을 위해서 객체를 생성해서 적용한다.
	//			pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
	//														pSynMsg->mSkillClassIndex, 
	//														pSynMsg->mAttackerType,
	//														pSynMsg->mDramaKey, 
	//														pSynMsg->mIsChangeMonster );
	//			if( pObj == 0 )
	//			{
	//				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
	//				assert(0);
	//				return;
	//			}
	//		}

	//		if( pSynMsg->mAttackerDie == true )
	//		{
	//			pObj->VerifyFaile( false );
	//			return;
	//		}

	//		if( pSynMsg->mTargetCount == 0 )
	//		{
	//			pObj->VerifySuccess();
	//			return;
	//		}

	//		cBaseObject* attacker = pObj->GetUser();
	//		sTargetDamage* pTargetDamage = pSynMsg->mTargetDamage;
	//		for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
	//		{
	//			/// 클라이언트 스킬객체 삭제 목적의 메세지 인경우 타겟 hp 감소 안함
	//			if( pTargetDamage->mIsOnlyClientObjectDel == false )
	//			{
	//				cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetDamage->mTarget );
	//				if( pTarget )
	//				{
	//					if( pTargetDamage->mInfluenceMiss == true )
	//						pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
	//				}
	//			}

	//			pObj->AddDamageTargetInfo( pTargetDamage );
	//			pObj->VerifySuccess();

	//			/// 080710 PKH 가변에 가변 길이 메세지 포인터위치 재계산
	//			pTargetDamage = (sTargetDamage*)((char*)pTargetDamage + pTargetDamage->GetLength( ));
	//		}
	//	}
	//	break;
	case NM_SKILL_DAMAGE_ATTACKER_SYN:
	case NM_SKILL_DAMAGE_TARGET_SYN:
		{
			MSG_SYN_SKILL_DAMAGE_ATTACKER* pSynMsg = (MSG_SYN_SKILL_DAMAGE_ATTACKER*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey, 
															pSynMsg->mIsChangeMonster );
				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
					assert(0);
					return;
				}
			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			if( pSynMsg->mTargetCount == 0 )
			{
				pObj->VerifySuccess();
				return;
			}

			sTargetDamage* pTargetDamage = pSynMsg->mTargetDamage;
			for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
			{
				/// 클라이언트 스킬객체 삭제 목적의 메세지 인경우 타겟 hp 감소 안함
				if( pTargetDamage->mIsOnlyClientObjectDel == false )
				{
					cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetDamage->mTarget );
					if( pTarget )
					{
						if( pTargetDamage->mInfluenceMiss == true )
							pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
					}
				}

				pObj->AddDamageTargetInfo( pTargetDamage );

				/// 080710 PKH 가변에 가변 길이 메세지 포인터위치 재계산
				pTargetDamage = (sTargetDamage*)((char*)pTargetDamage + pTargetDamage->GetLength( ));
			}
			pObj->VerifySuccess();
		}
		break;
	//case NM_SKILL_DAMAGE_TARGET_SYN:
	//	{
	//		MSG_SYN_SKILL_DAMAGE_TARGET* pSynMsg = (MSG_SYN_SKILL_DAMAGE_TARGET*)pMsg;

	//		cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
	//		if( pObj == 0 )
	//		{
	//			if( pSynMsg->mAttackerDie == true )
	//				return;

	//			/// 스킬 객체가 없는 경우..
	//			/// 연출 보정을 위해서 객체를 생성해서 적용한다.
	//			pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
	//														pSynMsg->mSkillClassIndex,
	//														pSynMsg->mSkillStep,
	//														pSynMsg->mAttackerType,
	//														pSynMsg->mDramaKey, 
	//														pSynMsg->mIsChangeMonster );
	//			if( pObj == 0 )
	//			{
	//				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
	//				assert(0);
	//				return;
	//			}
	//		}

	//		if( pSynMsg->mAttackerDie == true )
	//		{
	//			pObj->VerifyFaile( false );
	//			return;
	//		}

	//		/// arround == target
	//		for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
	//		{
	//			pObj->AddTargetInfo( &pSynMsg->mTarget[i], ePRINT_DAMAGE );
	//		}

	//		/// hero == target
	//		/// PKH 스킬 발동시 효과를 같이 걸어서 미스가 났는지 확인해 출력까지 한다.
	//		if( pSynMsg->GetTargetDamage()->mIsOnlyClientObjectDel == false )
	//		{
	//			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pSynMsg->GetTargetDamage()->mTarget );
	//			if( pTarget )
	//			{
	//				if( pSynMsg->GetTargetDamage()->mInfluenceMiss == true )
	//					pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
	//			}
	//		}
	//		pObj->AddDamageTargetInfo( pSynMsg->GetTargetDamage()  );
	//		pObj->VerifySuccess();
	//	}
	//	break;
	case NM_SKILL_DAMAGE_AROUND_SYN:
		{
			MSG_SYN_SKILL_DAMAGE_AROUND* pSynMsg = (MSG_SYN_SKILL_DAMAGE_AROUND*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey, 
															pSynMsg->mIsChangeMonster );

				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
					assert(0);
					return;
				}

			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			/// arround == target
			for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
			{
				pObj->AddTargetInfo( &pSynMsg->mTarget[i], ePRINT_DAMAGE );
			}

			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_DAMAGE_AROUND_CHANNELING_SYN:
		{
			MSG_SYN_SKILL_DAMAGE_AROUND_CHANNELING* pSynMsg = (MSG_SYN_SKILL_DAMAGE_AROUND_CHANNELING*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionChannelingSkillObject( pSynMsg->mSkillIndex, 
					pSynMsg->mSkillClassIndex, 
					pSynMsg->mSkillStep,
					pSynMsg->mAttackerType,
					pSynMsg->mDramaKey, 
					pSynMsg->mIsChangeMonster,
					pSynMsg->mTargetPosX,
					pSynMsg->mTargetPosY,
					pSynMsg->mPlayerIdx);

				if( pObj == 0 )
				{
					//CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
					assert(0);
					return;
				}

			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			/// arround == target
			for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
			{
				pObj->AddTargetInfo( &pSynMsg->mTarget[i], ePRINT_DAMAGE );
			}

			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_DAMAGE_APPLY_DIE_SYN:
		{
			MSG_SYN_SKILL_APPLY_DIE* pSynMsg = (MSG_SYN_SKILL_APPLY_DIE*)pMsg;

			cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pSynMsg->mTarget );
			cBaseObject* pAttacker = OBJECTMANAGER->GetObject( &pSynMsg->mAttacker );
			if( pTarget == 0 )
				return;

			pTarget->MPChange( 0, pTarget->GetMaxMP() );
			pTarget->HPChangeDie( pTarget->GetMaxHP(), pAttacker, pSynMsg->mDamageKind );
		}
		break;
	case NM_SKILL_DAMAGE_APPLY_HPMP_SYN:
		{
			MSG_SYN_SKILL_APPLY_HPMP* pSynMsg = (MSG_SYN_SKILL_APPLY_HPMP*)pMsg;

			cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pSynMsg->mTarget );
			if( pTarget == 0 )
				return;

			pTarget->MPChange( pSynMsg->mMP, pSynMsg->mMaxMP );
			pTarget->HPChange( pSynMsg->mHP, pSynMsg->mMaxHP );
		}
		break;
//	case NM_SKILL_DAMAGE_APPLY_SYN:
//		{
//			assert(0);
//			MSG_SYN_SKILL_DAMAGE_HP* pSynMsg = (MSG_SYN_SKILL_DAMAGE_HP*)pMsg;
//
//			cBaseObject* pAttacker = OBJECTMANAGER->GetObject( &pSynMsg->mAttacker );
//			for( unsigned int i=0; i<pSynMsg->mCount; i++ )
//			{
//				cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pSynMsg->mTargetDamage[i].mTarget );
//				if( pTarget == 0 )
//					continue;
//
////				pTarget->MPChange( pSynMsg->mTargetDamage[i].mObjectMP, pSynMsg->mTargetDamage[i].mObjectMaxMP );
////				pTarget->HPChange( &pSynMsg->mTargetDamage[i], pAttacker );
//			}
//		}
//		break;
	case NM_SKILL_BLINK_RES:
		{
			MSG_RES_SKILL_BLINK* pResMsg = (MSG_RES_SKILL_BLINK*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mDummySkillMap.GetAt( pResMsg->mManagedKey );
			if( pResMsg->ErrorCode == 0 )
			{
				/// 발동 성공
				/// mp setting
				HERO->MPChange( pResMsg->mObjectMP, pResMsg->mMaxMP );

				if( pObj )
				{
					/// 인증 성공이므로 관리형태로 이동
					mDummyIndexGen.RestoreIndex( pResMsg->mManagedKey );
					mDummySkillMap.Erase( pResMsg->mManagedKey );

					/// 관리 구조에 포함 시킨다.
					if( mSkillObjectMap.Insert( pResMsg->mSkillIndex, pObj ) )
					{
						pObj->SetManagedKey( pResMsg->mSkillIndex, false );
						pObj->SetBlinkPos( NiPoint3(pResMsg->mTargetPosX, pResMsg->mTargetPosY,0.0f) );

						SKILLMAN->StartCoolTime( pResMsg->mSkillClassIndex );
					}
					else
					{
						assert(0);
						SKILLMAN->DeleteSkillObject( pObj );

						if( HERO->IsReadyUseSkill() == false )
							HERO->SetReadyUseSkill( true );
						return;
					}
				}
			}
			else	/// 발동 실패
			{
				if( pObj )
				{
					/// 인증 성공이므로 관리형태로 이동
					mDummyIndexGen.RestoreIndex( pResMsg->mManagedKey );
					mDummySkillMap.Erase( pResMsg->mManagedKey );

					/// 관리 구조에 포함 시킨다.
					if( mSkillObjectMap.Insert( pResMsg->mSkillIndex, pObj ) )
					{
						pObj->SetManagedKey( pResMsg->mSkillIndex, false );
						pObj->SetBlinkPos( NiPoint3(pResMsg->mTargetPosX, pResMsg->mTargetPosY,0.0f) );
					}
					else
					{
						assert(0);
						SKILLMAN->DeleteSkillObject( pObj );

						if( HERO->IsReadyUseSkill() == false )
							HERO->SetReadyUseSkill( true );
						return;
					}
				}
			}
		}
		break;
	case NM_SKILL_BLINK_SYN:
		{
			MSG_SYN_SKILL_BLINK* pSynMsg = (MSG_SYN_SKILL_BLINK*)pMsg;
			if( pSynMsg->mAttacker.type != eOBJECTTYPE_PLAYER  )
			{
				assert(0);
				return;
			}

			cBaseObject* pAttacker = OBJECTMAN->GetObject( &pSynMsg->mAttacker );
			if( pAttacker == 0 )
			{
				assert(0);
				return;
			}

			pAttacker->MPChange( pSynMsg->mObjectMP, pSynMsg->mObjectMaxMP );

			cPlayer* pPlayer = (cPlayer*)pAttacker;
			pPlayer->PlayerBlinkSkill( pSynMsg->mSkillIndex, pSynMsg->mSkillClassIndex, pSynMsg->mSkillStep, NiPoint3(pSynMsg->mTargetPosX, pSynMsg->mTargetPosY,0.0f) );
		}
		break;
	//case NM_SKILL_HEAL_SYN:
	//	{
	//		MSG_SYN_SKILL_HEAL* pSynMsg = (MSG_SYN_SKILL_HEAL*)pMsg;
	//		cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
	//		if( pObj == 0 )
	//		{
	//			if( pSynMsg->mAttackerDie == true )
	//				return;

	//			/// 스킬 객체가 없는 경우..
	//			/// 연출 보정을 위해서 객체를 생성해서 적용한다.
	//			pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
	//														pSynMsg->mSkillClassIndex, 
	//														pSynMsg->mAttackerType,
	//														pSynMsg->mDramaKey,
	//														pSynMsg->mIsChangeMonster );
	//			if( pObj == 0 )
	//			{
	//				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
	//				assert(0);
	//				return;
	//			}
	//		}

	//		if( pSynMsg->mAttackerDie == true )
	//		{
	//			pObj->VerifyFaile( false );
	//			return;
	//		}

	//		if( pSynMsg->mTargetCount == 0 )
	//		{
	//			pObj->VerifySuccess();
	//			return;
	//		}

	//		for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
	//		{
	//			/// hp setting
	//			sTargetHeal* pTargetHeal = &pSynMsg->mTargetHeal[i];
	//			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetHeal->mTarget );
	//			if( pTarget )
	//			{
	//				pTarget->HPChange( pTargetHeal->mObjectHP, pTargetHeal->mObjectMaxHP );

	//				if( pTargetHeal->mInfluenceMiss == true )
	//				{
	//					pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
	//				}
	//			}

	//			pObj->AddHealTargetInfo( pTargetHeal );
	//			pObj->VerifySuccess();
	//		}
	//	}
	//	break;

	case NM_SKILL_HEAL_ATTACKER_SYN:
	case NM_SKILL_HEAL_TARGET_SYN:
		{
			MSG_SYN_SKILL_HEAL_ATTACKER* pSynMsg = (MSG_SYN_SKILL_HEAL_ATTACKER*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey,
															pSynMsg->mIsChangeMonster );
				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
					assert(0);
					return;
				}
			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
			{
				/// hp setting
				sTargetHeal* pTargetHeal = &pSynMsg->mTargetHeal[i];
				cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetHeal->mTarget );
				if( pTarget )
				{
					if( pTargetHeal->mInfluenceMiss == true )
						pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
				}
				pObj->AddHealTargetInfo( pTargetHeal );
			}
			pObj->VerifySuccess();
		}
		break;
	//case NM_SKILL_HEAL_TARGET_SYN:
	//	{
	//		MSG_SYN_SKILL_HEAL_TARGET* pSynMsg = (MSG_SYN_SKILL_HEAL_TARGET*)pMsg;

	//		cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
	//		if( pObj == 0 )
	//		{
	//			if( pSynMsg->mAttackerDie == true )
	//				return;

	//			/// 스킬 객체가 없는 경우..
	//			/// 연출 보정을 위해서 객체를 생성해서 적용한다.
	//			pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
	//														pSynMsg->mSkillClassIndex, 
	//														pSynMsg->mSkillStep,
	//														pSynMsg->mAttackerType,
	//														pSynMsg->mDramaKey,
	//														pSynMsg->mIsChangeMonster );
	//			if( pObj == 0 )
	//			{
	//				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
	//				assert(0);
	//				return;
	//			}
	//		}

	//		if( pSynMsg->mAttackerDie == true )
	//		{
	//			pObj->VerifyFaile( false );
	//			return;
	//		}

	//		sTargetHeal* pTargetHeal = &pSynMsg->mTargetHeal;
	//		if( pTargetHeal )
	//		{
	//			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &(pTargetHeal->mTarget) );
	//			if( pTarget )
	//			{
	//				if( pTargetHeal->mInfluenceMiss == true )
	//					pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );

	//				pObj->AddHealTargetInfo( pTargetHeal );
	//			}
	//		}

	//		for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
	//		{
	//			pObj->AddTargetInfo( &pSynMsg->mTarget[i], ePRINT_HEAL );
	//		}

	//		pObj->VerifySuccess();
	//	}
	//	break;
	case NM_SKILL_HEAL_AROUND_SYN:
		{
			MSG_SYN_SKILL_HEAL_AROUND* pSynMsg = (MSG_SYN_SKILL_HEAL_AROUND*)pMsg;

			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey,
															pSynMsg->mIsChangeMonster );
				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(2)!") );
					assert(0);
					return;
				}
			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			for( unsigned char i=0; i<pSynMsg->mTargetCount; ++i )
			{
				pObj->AddTargetInfo( &pSynMsg->mTarget[i], ePRINT_HEAL );
			}

			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_APPLY_SYN:
		{
			/// None_Active중 Heal을 제외한 모든 스킬의 적용 메세지..
			MSG_SYN_SKILL_APPLY* pSynMsg = (MSG_SYN_SKILL_APPLY*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey,
															pSynMsg->mIsChangeMonster );
				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(1)!") );
					assert(0);
					return;
				}
			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			if( pSynMsg->mApplyCount == 0 )
			{
				pObj->VerifySuccess();
				return;
			}

			cBaseObject* skillOwner = pObj->GetUser();
			for( unsigned char i=0; i<pSynMsg->mApplyCount; ++i )
			{
				sTargetApply pTargetApply = pSynMsg->mApplyObject[i];
				cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetApply.mTarget );
				if( pTarget )
				{
					if( skillOwner == HERO )
					{
						if( pTargetApply.mInfluenceMiss == true )
							pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
					}
					else
					{
						if( pTarget == HERO )
						{
							if( pTargetApply.mInfluenceMiss == true )
								pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
						}
					}
				}
				pObj->AddTargetInfo( &pTargetApply.mTarget );
			}
			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_DELTARGET_SYN:
		{
			/// None_Active중 Heal을 제외한 모든 스킬의 적용 메세지..
			MSG_SYN_SKILL_DELTARGET* pSynMsg = (MSG_SYN_SKILL_DELTARGET*)pMsg;
			cSkillObject* pObj = (cSkillObject*)mSkillObjectMap.GetAt( pSynMsg->mSkillIndex );
			if( pObj == 0 )
			{
				if( pSynMsg->mAttackerDie == true )
					return;

				/// 스킬 객체가 없는 경우..
				/// 연출 보정을 위해서 객체를 생성해서 적용한다.
				pObj = SKILLMAN->CreateRevisionSkillObject( pSynMsg->mSkillIndex, 
															pSynMsg->mSkillClassIndex, 
															pSynMsg->mSkillStep,
															pSynMsg->mAttackerType,
															pSynMsg->mDramaKey,
															false );
				if( pObj == 0 )
				{
//					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, _T("Wrong revision skill object(1)!") );
					assert(0);
					return;
				}
			}

			if( pSynMsg->mAttackerDie == true )
			{
				pObj->VerifyFaile( false );
				return;
			}

			if( pSynMsg->mDelTargetIndex != 0 )
			{
				/// 타겟 해제
				const sObject* target = HERO->GetTargetInfo();
				if( target->type == pSynMsg->mDelTargetType && target->index == pSynMsg->mDelTargetIndex )
					HERO->SetTargetObject( eOBJECTTYPE_NONE, 0 );
			}

			if( pSynMsg->mApplyCount == 0 )
			{
				pObj->VerifySuccess();
				return;
			}

			cBaseObject* skillOwner = pObj->GetUser();
			for( unsigned char i=0; i<pSynMsg->mApplyCount; ++i )
			{
				sTargetApply pTargetApply = pSynMsg->mApplyObject[i];
				cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pTargetApply.mTarget );
				if( pTarget )
				{
					if( skillOwner == HERO )
					{
						if( pTargetApply.mInfluenceMiss == true )
							pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
					}
					else
					{
						if( pTarget == HERO )
						{
							if( pTargetApply.mInfluenceMiss == true )
								pTarget->ApplyDamageDrama( 0, eDAMAGEPRINT_MISS );
						}
					}
				}

				pObj->AddTargetInfo( &pTargetApply.mTarget );
			}
			pObj->VerifySuccess();
		}
		break;
	case NM_SKILL_ACTINFLUENCE_ACTION_SYN:
		{
			MSG_SYN_ACTINFLUNCE_ACTION* pSynMsg = (MSG_SYN_ACTINFLUNCE_ACTION*)pMsg;
			cBaseObject* pPlayer = OBJECTMAN->GetPlayer( pSynMsg->mPlayer.index );
			if( pPlayer )
				CreateActApplyObject( pPlayer, pSynMsg->mActIdx );
		}
		break;
	case NM_SKILL_INFLUENCE_CREATE_SYN:
		{
			MSG_SYN_INFLUENCE_CREATE* pSynMsg = (MSG_SYN_INFLUENCE_CREATE*)pMsg;

			cSkillWindow* pSkillWindow = GAMEUI->GetSkillWindow();

			cBaseObject* pObj = OBJECTMAN->GetObject( &(pSynMsg->mTarget) );
			if( pObj )
			{
				cInfluenceObject* p = CreateInfluenceObject( pSynMsg->mUniqueIdx, pSynMsg->mInfluenceClassIdx, pSynMsg->mAuraSkillClassIdx, pObj, pSynMsg->mDeleteType );
				if( p )
				{
					p->StartProcess( -1, false );
					if( pSynMsg->mIsRealTime == true )
						p->SetRealEndTime( pSynMsg->mRealEndTime );

					if( p->IsHeroInfluence() && pSynMsg->mAuraSkillClassIdx > 0 )
					{
						/// 오라버프를 거는 스킬이면 윈도우에도 세팅
						if( pSkillWindow )
							pSkillWindow->SetOnAura( pSynMsg->mAuraSkillClassIdx, pSynMsg->mUniqueIdx, true );
					}
				}
				else
				{
					assert(0);
					return;
				}
			}
		}
		break;
	case NM_SKILL_INFLUENCE_CREATELIST_SYN:
		{
			cSkillWindow* pSkillWindow = GAMEUI->GetSkillWindow();

			MSG_SYN_INFLUENCEARY_CREATE* pSynMsg = (MSG_SYN_INFLUENCEARY_CREATE*)pMsg;
			for( unsigned short i = 0 ; i < pSynMsg->mTargetCnt ; ++i )
			{
				sInfluenceCreate* pTable = &pSynMsg->mTable[i];
				cBaseObject* pObj = OBJECTMAN->GetObject( &pTable->mTarget );
				if( pObj )
				{
					cInfluenceObject* p = CreateInfluenceObject( pTable->mUniqueIdx, pTable->mInfluenceClassIdx, pTable->mAuraSkillClassIdx, pObj, pTable->mDeleteType );
					if( p )
					{
						p->StartProcess( -1, true );
						if( pTable->mIsRealTime == true )
							p->SetRealEndTime( pTable->mRealEndTime );

						if( p->IsHeroInfluence() && pTable->mAuraSkillClassIdx > 0 )
						{
							/// 오라버프를 거는 스킬이면 윈도우에도 세팅
							if( pSkillWindow )
								pSkillWindow->SetOnAura( pTable->mAuraSkillClassIdx, pTable->mUniqueIdx, true );
						}
					}
					else
					{
						assert(0);
						return;
					}
				}
			}
		}
		break;
	case NM_SKILL_INFLUENCE_ADDLIST_SYN:
		{
			MSG_SYN_INFLUENCE_ADDLIST* pmsg = (MSG_SYN_INFLUENCE_ADDLIST*)pMsg;

			sInfluenceClient* pInfluence;
			cBaseObject* pObj = OBJECTMAN->GetObject( &pmsg->mObject );
			if( pObj )
			{
				for( long i = 0 ; i < pmsg->mRowCount ; ++i )
				{
					pInfluence = &pmsg->mTable[i];
					cInfluenceObject* p = CreateInfluenceObjectEx( pInfluence->mUniqueIdx, pInfluence->mInfluenceClassIdx, pInfluence->mAuraSkillClassIdx, pObj, pInfluence->mDeleteType, pInfluence->mRestTime );
					if( p )
					{
						p->StartProcess( pInfluence->mRestTime, true );
						if( pInfluence->mIsRealTime == true )
							p->SetRealEndTime( pInfluence->mEndRealTime );
					}
					else
					{
						assert(0);
						return;
					}
				}
			}
		}
		break;

	case NM_SKILL_INFLUENCE_DELETE_SYN:
		{
			MSG_SYN_INFLUENCE_DELETE* pSynMsg = (MSG_SYN_INFLUENCE_DELETE*)pMsg;

			DeleteInfluenceObject( pSynMsg->mUniqueIdx );
		}
		break;
//	case NM_SKILL_INFLUENCE_HPCHAGNE_SYN:
//		{
//			MSG_SYN_SKILL_INFLUENCE_HPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_HPCHANGE*)pMsg;
//
//			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
//			if( pTarget )
//			{
////				pTarget->HPChange( pmsg->mObjectHP, pmsg->mObjectMaxHP, pmsg->mDie );
//
//				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
//				if( pObj )
//				{
//					/// 적용
//					pObj->SetTickInfo( cInfluenceObject::eTick_HP, pmsg->mChangeValue );
//				}
//				else
//				{
//					//assert(0);
//				}
//			}
//		}
//		break;
//	case NM_SKILL_INFLUENCE_MPCHAGNE_SYN:
//		{
//			MSG_SYN_SKILL_INFLUENCE_MPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_MPCHANGE*)pMsg;
//
//			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
//			if( pTarget )
//			{
////				pTarget->MPChange( pmsg->mObjectMP, pmsg->mObjectMaxMP );
//
//				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
//				if( pObj )
//				{
//					/// 적용
//					pObj->SetTickInfo( cInfluenceObject::eTick_MP, pmsg->mChangeValue );
//				}
//				else
//				{
//					assert(0);
//				}
//			}
//		}
//		break;
	case NM_SKILL_INFLUENCE_HPCHAGNE_SYN:
		{
			MSG_SYN_SKILL_INFLUENCE_HPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_HPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
				if( pObj )
				{
					/// 적용
					pObj->SetTickInfo( cInfluenceObject::eTick_HP, pmsg->mChangeValue );
				}
				else
				{
					//assert(0);
				}
			}
		}
		break;
	case NM_SKILL_INFLUENCE_MPCHAGNE_SYN:
		{
			MSG_SYN_SKILL_INFLUENCE_MPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_MPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
				if( pObj )
				{
					/// 적용
					pObj->SetTickInfo( cInfluenceObject::eTick_MP, pmsg->mChangeValue );
				}
				else
				{
					assert(0);
				}
			}
		}
		break;

	case NM_SKILL_INFLUENCE_HPCHAGNE_AROUND_SYN:
		{
			MSG_SYN_SKILL_INFLUENCE_HPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_HPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
				if( pObj )
				{
					/// 적용
					pObj->SetTickInfo( cInfluenceObject::eTick_HP_NOPRINT, pmsg->mChangeValue );
				}
				else
				{
					//assert(0);
				}
			}
		}
		break;
	case NM_SKILL_INFLUENCE_MPCHAGNE_AROUND_SYN:
		{
			MSG_SYN_SKILL_INFLUENCE_MPCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_MPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				cInfluenceObject* pObj = (cInfluenceObject*)mInfluenceMap.GetAt( pmsg->mInfluenceIdx );
				if( pObj )
				{
					/// 적용
					pObj->SetTickInfo( cInfluenceObject::eTick_MP_NOPRINT, pmsg->mChangeValue );
				}
				else
				{
					assert(0);
				}
			}
		}
		break;

	//case NM_SKILL_NONSKILL_HPCHANGE_SYN:
	//	{
	//		MSG_SYN_SKILL_NONSKILL_HPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_HPCHANGE*)pMsg;

	//		cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
	//		if( pTarget )
	//		{
	//			if( pmsg->mDie == true )
	//				pTarget->HPChangeDie( pmsg->mObjectMaxHP, 0, eDAMAGE_NORMAL );
	//			else
	//				pTarget->HPChange( pmsg->mObjectHP, pmsg->mObjectMaxHP );

	//			 
	//			if( pmsg->mInfluenceClassIdx != 0 )
	//				CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );

	//			bool enemy = false;
	//			switch( pTarget->GetObjectType() )
	//			{
	//			case eOBJECTTYPE_MONSTER:
	//				enemy = true;
	//				break;
	//			case eOBJECTTYPE_PLAYER:
	//				{
	//					if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
	//						enemy = true;
	//				}
	//				break;
	//			}

	//			if( pmsg->mChangeValue > 0.0f )
	//			{
	//				if( enemy == false )
	//					pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_HEAL_OUR );
	//				else
	//					pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_HEAL_ENEMY );
	//			}
	//			else
	//			{
	//				if( enemy == false )
	//					pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_NORMAL_OUR );
	//				else
	//					pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_NORMAL_ENEMY );
	//			}
	//		}
	//	}
	//	break;
	//case NM_SKILL_NONSKILL_MPCHANGE_SYN:
	//	{
	//		MSG_SYN_SKILL_NONSKILL_MPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_MPCHANGE*)pMsg;

	//		cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
	//		if( pTarget )
	//		{
	//			pTarget->MPChange( pmsg->mObjectMP, pmsg->mObjectMaxMP );

	//			 
	//			if( pmsg->mInfluenceClassIdx != 0 )
	//				CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );


	//			if( pmsg->mChangeValue > 0.0f )
	//				pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_MP_HEAL );
	//			else
	//				pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_MP_NORMAL );
	//		}
	//	}
	//	break;
	case NM_SKILL_NONSKILL_HPCHANGE_SYN:
		{
			MSG_SYN_SKILL_NONSKILL_HPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_HPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				/// 
				if( pmsg->mInfluenceClassIdx != 0 )
					CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );

				bool enemy = false;
				switch( pTarget->GetObjectType() )
				{
				case eOBJECTTYPE_MONSTER:
					enemy = true;
					break;
				case eOBJECTTYPE_PLAYER:
					{
						if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
							enemy = true;
					}
					break;
				}

				if( pmsg->mChangeValue > 0.0f )
				{
					if( enemy == false )
						pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_HEAL_OUR );
					else
						pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_HEAL_ENEMY );
				}
				else
				{
					if( enemy == false )
						pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_NORMAL_OUR );
					else
						pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_NORMAL_ENEMY );
				}
			}
		}
		break;
	case NM_SKILL_NONSKILL_MPCHANGE_SYN:
		{
			MSG_SYN_SKILL_NONSKILL_MPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_MPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				/// 
				if( pmsg->mInfluenceClassIdx != 0 )
					CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );


				if( pmsg->mChangeValue > 0.0f )
					pTarget->ApplyDamageDrama( pmsg->mChangeValue, eDAMAGEPRINT_MP_HEAL );
				else
					pTarget->ApplyDamageDrama( ::abs(pmsg->mChangeValue), eDAMAGEPRINT_MP_NORMAL );
			}
		}
		break;
	case NM_SKILL_NONSKILL_HPCHANGE_AROUND_SYN:
		{
			MSG_SYN_SKILL_NONSKILL_HPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_HPCHANGE*)pMsg;
			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				if( pmsg->mInfluenceClassIdx != 0 )
					CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );

				bool enemy = false;
				switch( pTarget->GetObjectType() )
				{
				case eOBJECTTYPE_MONSTER:
					enemy = true;
					break;
				case eOBJECTTYPE_PLAYER:
					{
						if( HERO &&	HERO->IsEnemy( (cPlayer*)pTarget ) == true )
							enemy = true;
					}
					break;
				}

				if( pmsg->mChangeValue > 0.0f )
				{
					if( enemy == false )
						pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_HEAL_OUR );
					else
						pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_HEAL_ENEMY );
				}
				else
				{
					if( enemy == false )
						pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_NORMAL_OUR );
					else
						pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_NORMAL_ENEMY );
				}
			}
		}
		break;
	case NM_SKILL_NONSKILL_MPCHANGE_AROUND_SYN:
		{
			MSG_SYN_SKILL_NONSKILL_MPCHANGE* pmsg = (MSG_SYN_SKILL_NONSKILL_MPCHANGE*)pMsg;

			cBaseObject* pTarget = (cBaseObject*)OBJECTMANAGER->GetObject( &pmsg->mTarget );
			if( pTarget )
			{
				/// 
				if( pmsg->mInfluenceClassIdx != 0 )
					CreateApplyObject( pTarget, pmsg->mInfluenceClassIdx );


				if( pmsg->mChangeValue > 0.0f )
					pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_MP_HEAL );
				else
					pTarget->ApplyDamageDrama( -1, eDAMAGEPRINT_MP_NORMAL );
			}
		}
		break;
	case NM_SKILL_INFLUENCE_GUARDCHANGE_SYN:
		{
			MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE* pmsg = (MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE*)pMsg;

			cInfluenceObject* pInf = GetInfluenceObject( pmsg->mInfluenceIdx );
			if( pInf )
			{
				pInf->SetGuardInfo( pmsg->mChangeCnt, pmsg->mChangeValue );

				if( pmsg->mChangeCnt == 0 || pmsg->mChangeValue == 0 )
				{
					assert(0);
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	case NM_SKILL_RESURRECTION_RES:
		{
			cStringT str;

			MSG_RES_SKILL_RESURRECTION* msg = (MSG_RES_SKILL_RESURRECTION*)pMsg;
			
			cPlayer* player = OBJECTMANAGER->GetPlayer( msg->mAttackerIdx );
			if( player )
			{
				UIMAN->SendEventToMsgbox( eMBEVENT_REVIVAL_STAND, false );

				str.Format( GAMERESOURCEMAN->GetGameText( 902 ), player->GetName() );
				
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_REVIVAL_SKILL, eSKIN_YESNO,
					(LPCTSTR)str.Cstr(),
					GAMERESOURCEMAN->GetGameText( 301 )	);
			}
		}
		break;
	case NM_SKILL_RESURRECTION_SYN:
		{
			/// 부활 처리
			MSG_SYN_SKILL_RESURRECTION* synMsg = (MSG_SYN_SKILL_RESURRECTION*)pMsg;

			if( synMsg->mTarget.type == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( synMsg->mTarget.index );
				if( pPlayer )
				{
					pPlayer->SetState( eOBJECT_STATE_IDLE );
					pPlayer->HPChange( synMsg->mTargetHP, synMsg->mTargetMaxHp );
					pPlayer->MPChange( synMsg->mTargetMP, synMsg->mTargetMaxMp );
				}
			}
		}
		break;
	case NM_SKILL_CHGMON_SKILLCOOLTIME:
		{
			MSG_RES_SKILL_CHGMON_COOLTIME* pmsg = (MSG_RES_SKILL_CHGMON_COOLTIME*)pMsg;

			/// 기본공격 초기화
			SKILLMAN->StartCoolTimeMon( eMONSTERATTACK_NORMAL1, 0 );
			SKILLMAN->StartCoolTimeMon( eMONSTERATTACK_NORMAL2, 0 );

			/// 남은 쿨타임이 있는 경우에만 셋팅
			if( !( pmsg->mSkill1CoolTime == ULONG_MAX || pmsg->mSkill1CoolTime == 0 ) )
				SKILLMAN->StartCoolTimeMon( eMONSTERATTACK_SKILL1, pmsg->mSkill1CoolTime );
			if( !( pmsg->mSkill2CoolTime == ULONG_MAX || pmsg->mSkill2CoolTime == 0 ) )
				SKILLMAN->StartCoolTimeMon( eMONSTERATTACK_SKILL2, pmsg->mSkill2CoolTime );
			if( !( pmsg->mSkill3CoolTime == ULONG_MAX || pmsg->mSkill3CoolTime == 0 ) )
				SKILLMAN->StartCoolTimeMon( eMONSTERATTACK_SKILL3, pmsg->mSkill3CoolTime );

			QUICKMAN->LoadCoolTimeMon();
		}
		break;
	case NM_SKILL_ADD_SUC_RES:
		{
			MSG_RES_SKILL_ADD* pResMsg = (MSG_RES_SKILL_ADD*)pMsg;

			cSkillWindow* win = GAMEUI->GetSkillWindow();
			if( win )
				win->NetUnLock();

			if( GetKeepInfo( pResMsg->mSkillClassIdx ) == 0 )
			{
				/// add
				AddKeepSkill( pResMsg->mSkillClassIdx, pResMsg->mSkillStep );
			}
			else
			{
				/// update
				UpdateSkillStep( pResMsg->mSkillClassIdx, pResMsg->mSkillStep, true );
			}

			/// 071024 PKH 스테이터스 재계산
			HERO->CalcStatus();

			/// skill sheet skill levelup button visiable 업데이트
			if( win )
				win->ShowSkillUpButton();

			/// 이펙트 추가
			NiTransform trans;
			trans.MakeIdentity();
			trans.m_Translate = NiPoint3( 0.0f, 0.0f, 30.0f );

			HERO->SetLinkdEffect( eLINK_HEAD, "./Data/Effect/Eff_PC_Get_01.nif", &trans, false, true );
		}
		break;
	case NM_SKILL_ADD_ERR_RES:
		{
			MSG_ERROR* pResMsg = (MSG_ERROR*)pMsg;

			cSkillWindow* win = GAMEUI->GetSkillWindow();
			if( win )
				win->NetUnLock();

			switch( pResMsg->ErrorCode )
			{
			case SKILL_ADD_ERR_MONEY:
				{
					/// 소지금액 부족
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6100 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_SP:
				{
					/// skill point 부족
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6101 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_JOBSP:
				{
					/// 이전 직업의 스킬포인트 사용이 부족
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6110 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_SKILLSTEP:
				{
					/// 스킬 습득 단계 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6103 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_LEARNSKILL:
				{
					/// 선행 보유 스킬 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6104 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_LEARNSTEP:
				{
					/// 선행 보유 스킬 단계 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6105 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_RACE:
				{
					/// 종족 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6106 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_JOB:
				{
					/// 직업 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6107 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_LEVEL:
				{
					/// 레벨 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6108 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_ALREADY:
				{
					/// 이미 배운 스킬을 다시 배우려함
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6109 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			case SKILL_ADD_ERR_NPC:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 420 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			default:
				{
					/// 그외 모든 오류
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 6102 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			}
		}
		break;
	case NM_SKILL_PUSHPULL_SYN:
		{
			MSG_SYN_SKLL_PUSHPULL* pResMsg = (MSG_SYN_SKLL_PUSHPULL*)pMsg;
			
			cBaseObject* pTarget = OBJECTMANAGER->GetObject( &pResMsg->mTarget );
			if( pTarget != NULL )
				pTarget->SetSkillPushPull( pResMsg->mPosX, pResMsg->mPosY );
		}
		break;
	case NM_SKILL_CLEAR_COOLTIME:
		{
			MSG_SYN_SKILL_CLEAR_COOLTIME* pSynMsg = (MSG_SYN_SKILL_CLEAR_COOLTIME*)pMsg;
			if( pSynMsg->mCharacterIdx == HERO->GetObjectID() )
			{
				ClearSkillCooltime( pSynMsg->mExceptSKillIdx );
			}
		}
		break;
	}
}

bool cSkillManager::CheckBuySkill( unsigned long skillIdx, unsigned char skillStep )
{
	if( skillStep )
	{
		/// 보유 스킬 아니면 구매 불가능
		if( mKeepSkillMap.GetAt( skillIdx ) == 0 )
			return false;
	}

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
		return false;

	if( pInfo->mStepCount <= skillStep )
		return false;

	sPlayerSkillStepInfo pStepInfo = pInfo->mpSetpInfoArray[skillStep];

	if( HERO->GetSkillPointRemain() < pStepInfo.mRequireSP )
		return false;

	if( HERO->GetLevel() < pStepInfo.mPlayerLevel )
		return false;

	/// 종족 체크
	switch( HERO->GetRace() )
	{
	case eRACE_HUMAN:
		if( pInfo->mRace != 0 && pInfo->mRace != 1 && pInfo->mRace != 4 && pInfo->mRace != 5 )
			return false;
		break;
	case eRACE_BEAST:
		if( pInfo->mRace != 0 && pInfo->mRace != 2 && pInfo->mRace != 4 && pInfo->mRace != 6 )
			return false;
		break;
	case eRACE_ELF:
		if( pInfo->mRace != 0 && pInfo->mRace != 3 && pInfo->mRace != 5 && pInfo->mRace != 6 )
			return false;
		break;
	default:
		assert(0);
		return false;
	}

	// 총사용 SP Check
	if( pInfo->mJobStep > 0 && pInfo->mJobStep < MAX_JOBSTEP )
	{
		unsigned long point = HERO->GetJobUsedSkillPoint( pInfo->mJobStep-1 );
		if( point < pInfo->mBeforeReqSP )
			return false;
	}
	
	// 선행 스킬 1
	if( pStepInfo.mLearnSkillIdx1 != 0 )
	{
		sKeepSkill* p = SKILLMAN->GetKeepInfo( pStepInfo.mLearnSkillIdx1 );
		if( p == 0 )
			return false;

		if( pStepInfo.mLearnSkillStep1 > p->mStep + 1 )
			return false;
	}

	// 선행 스킬 2
	if( pStepInfo.mLearnSkillIdx2 != 0 )
	{
		sKeepSkill* p = SKILLMAN->GetKeepInfo( pStepInfo.mLearnSkillIdx2 );
		if( p == 0 )
			return false;

		if( pStepInfo.mLearnSkillStep2 > p->mStep + 1 )
			return false;
	}

	/// 직업 체크
	if( SKILLSCRIPT->IsPlayerSkillJob( pStepInfo.mJobType, HERO->GetJob() ) == false )
		return false;

	return true;
}

void cSkillManager::StartCoolTime( unsigned long skillIdx, unsigned long restTime )
{
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

	float speedFactor = 1.0f;
	if( HERO && idx == NORMAL_ATTACK_SKILL )
	{
		speedFactor = HERO->GetAttackSpeed();

		if( speedFactor <= 0.0f )
			speedFactor = 0.1f;
		if( speedFactor >= 2.0f )
			speedFactor = 1.9f;
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
	p->mTotalTime = (unsigned long)(pStep->mCoolTime * ( 2.0f - speedFactor ) );
	if( restTime == ULONG_MAX )
	{
		p->mRestTime = (unsigned long)(pStep->mCoolTime * ( 2.0f - speedFactor ) );

		/// 그룹 쿨타임 체크..
		if( idx != NORMAL_ATTACK_SKILL )
			UpdateExtraCooltime( skillIdx, p->mRestTime, pInfo->mGroupCoolTimeIdx );
	}
	else
	{
		p->mRestTime = (unsigned long)(restTime * ( 2.0f - speedFactor ) );
	}

	p->mStartTime = THEAPP->GetWorldAccumTime();

	/// skill window cooltime변경
	cSkillWindow* pWin = (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
	if( pWin )
	    pWin->StartCoolTime( idx, p->mRestTime, p->mTotalTime );
}

bool cSkillManager::IsUsedKeepSkill( unsigned long skillIdx, unsigned char skillStep )
{
	/// 스킬 사용 불가
	if( HERO->IsCantSkill( skillIdx ) == true )
		return false;

	sKeepSkill* p = (sKeepSkill*)mKeepSkillMap.GetAt( skillIdx );
	if( p == 0 )
		return false;

	if( p->mStep < skillStep )
		return false;

	if( p->mRestTime == 0  || p->mRestTime == ULONG_MAX )
		return true;

	return false;
}

bool cSkillManager::IsUsedMonsterSkill( unsigned long skillIdx )
{
	/// 스킬 사용 불가
	if( HERO->IsCantSkill( skillIdx ) == true )
		return false;

	/// 쿨타임은 별도로 저장하지 않고 아이콘에 저장된 값을 사용한다.
	if( skillIdx >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return false;
	}

	if( mMonSkillCool[skillIdx].mRestTime == 0 || mMonSkillCool[skillIdx].mRestTime == ULONG_MAX )
		return true;

	return false;
}

void cSkillManager::UpdateExtraCooltime( unsigned long skillIdx, unsigned long restTime, unsigned long groupIdx  )
{
	/// skill window cooltime변경
	cSkillWindow* pWin = (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
	if( pWin == 0 )
		return;

	cSkillMap::cIterator k, kend;
	k = mKeepSkillMap.Begin();
	kend = mKeepSkillMap.End();
	for( ; k != kend; ++k )
	{
		unsigned long idx = (unsigned long)(*k).mFirst;
		if( skillIdx == idx )
			continue;
		sKeepSkill* pKeepSkill = (sKeepSkill*)(*k).mSecond;

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( idx );
		if( pInfo == 0 )
			continue;

		bool applyGroup = false;
		if( groupIdx != 0 && pInfo->mGroupCoolTimeIdx == groupIdx )
			applyGroup = true;

		if( applyGroup == true )
		{
			/// Apply Group cooltime
			if( pInfo->mStepCount <= pKeepSkill->mStep )
			{
				assert(0);
			}

			sPlayerSkillStepInfo* pStep = &pInfo->mpSetpInfoArray[pKeepSkill->mStep];
			if( pKeepSkill->mRestTime != 0 )
			{
				if( pKeepSkill->mRestTime >= (long)restTime )
					continue;

				pKeepSkill->mRestTime = restTime;
				pKeepSkill->mTotalTime = restTime;
			}
			else
			{
				if( pStep->mCoolTime > restTime )
				{
					pKeepSkill->mRestTime = restTime;
					pKeepSkill->mTotalTime = restTime;
				}
				else
				{
					pKeepSkill->mRestTime = pStep->mCoolTime;
					pKeepSkill->mTotalTime = pStep->mCoolTime;
				}
			}
		}
		else
		{
			if( pInfo->mIsGlobalCoolTime  == true )
			{
				/// Apply Global cooltime 0.5min
				if( pKeepSkill->mRestTime == 0 )
				{
					if( pInfo->mType == eSKILLTYPE_PASSIVE )
						continue;

					pKeepSkill->mRestTime = 500;
					pKeepSkill->mTotalTime = 500;
				}
			}
		}

		pKeepSkill->mStartTime = THEAPP->GetWorldAccumTime();
		pWin->StartCoolTime( idx, pKeepSkill->mRestTime, pKeepSkill->mTotalTime );
	}
}

void cSkillManager::StartCoolTimeMon( unsigned long skillIdx, unsigned long restTime )
{
	unsigned long classIdx = HERO->GetTransMonsterClassIdx();
	if( classIdx == 0 )
	{
		assert(0);
		return;
	}

	if( skillIdx >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return;
	}

	float speedFactor = 1.0f;
	if( HERO && skillIdx < eMONSTERATTACK_SKILL1 )
	{
		speedFactor = HERO->GetAttackSpeed();

		if( speedFactor <= 0.0f )
			speedFactor = 0.1f;
		if( speedFactor >= 2.0f )
			speedFactor = 1.9f;
	}

	/// 스킬 사용
	sMonsterSkillScript* pInfo = (sMonsterSkillScript*)SKILLSCRIPT->GetMonsterSkillInfo( classIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo )
	{
		unsigned long total = unsigned long(pInfo->mCoolTime * ( 2.0f - speedFactor ) );

		if( restTime == ULONG_MAX )
		{
			mMonSkillCool[skillIdx].mRestTime = (unsigned long)(pInfo->mCoolTime * ( 2.0f - speedFactor ) );

			if( skillIdx >= eMONSTERATTACK_SKILL1 )
				QUICKMAN->StartMonSkillCoolTime( (short)skillIdx, total );
		}
		else
		{
			mMonSkillCool[skillIdx].mRestTime = (unsigned long)(restTime * ( 2.0f - speedFactor ) );
		}

		/// 쿨타임 저장
		mMonSkillCool[skillIdx].mStartTime	= THEAPP->GetWorldAccumTime();
		mMonSkillCool[skillIdx].mTotalTime	= total;
	}
	else
		assert(0);
}

void cSkillManager::ClearCoolTimeMon()
{
	::memset( mMonSkillCool, 0, sizeof(mMonSkillCool) );
}

sKeepSkill* cSkillManager::GetCoolTimeMon( unsigned long skillIdx )
{
	if( skillIdx >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return 0;
	}

	return &mMonSkillCool[skillIdx];
}

void cSkillManager::DeleteInfluenceObject( unsigned long uniqueIdx )
{
	cInfluenceObject* p = (cInfluenceObject*)mInfluenceMap.GetAt( uniqueIdx );
	if( p )
	{
		cSkillWindow* pSkillWindow = GAMEUI->GetSkillWindow();
		cBuffWindow* pBuffWindow = GAMEUI->GetBuffWindow();
		cBuffWindow* pCashBuffWindow = GAMEUI->GetCashBuffWindow();
		cUIWindow* pDuelWindow = (cUIWindow*)GAMEUI->GetDuelWindow();

		if( p->IsHeroInfluence() )
		{
			/// 오라버프를 거는 스킬이면 윈도우에도 세팅
			if( pSkillWindow && p->IsAuraType() )
				pSkillWindow->SetOnAura( p->GetSkillIdx(), 0, false );

			if( pBuffWindow && pBuffWindow->IsNetLock() == true )
				pBuffWindow->NetUnLock();

			if( pCashBuffWindow && pCashBuffWindow->IsNetLock() == true )
				pCashBuffWindow->NetUnLock();

			if( pDuelWindow && pDuelWindow->IsNetLock() == true )
				pDuelWindow->NetUnLock();
		}

		bool calc = true;
		if( p->IsRemoved() == false )
		{
			p->Removed();

			unsigned long key = p->GetManagedKey();
			mInfluenceMap.Erase( key );

			calc = p->IsHeroInfluence();

			delete p;

			/// 071024 PKH 스테이터스 재계산
			if( calc )
				HERO->CalcStatus();

//			mDeleteInfluenceArray.PushBack( p );
		}
	}
}

void cSkillManager::DeleteInfluenceObject( cInfluenceObject* p )
{
	if( p )
	{
		cSkillWindow* pSkillWindow = GAMEUI->GetSkillWindow();
		cBuffWindow* pBuffWindow = GAMEUI->GetBuffWindow();
		cBuffWindow* pCashBuffWindow = GAMEUI->GetCashBuffWindow();
		if( p->IsHeroInfluence() )
		{
			/// 오라버프를 거는 스킬이면 윈도우에도 세팅
			if( pSkillWindow &&  p->IsAuraType() )
				pSkillWindow->SetOnAura( p->GetSkillIdx(), 0, false );

			if( pBuffWindow && pBuffWindow->IsNetLock() == true )
				pBuffWindow->NetUnLock();

			if( pCashBuffWindow && pCashBuffWindow->IsNetLock() == true )
				pCashBuffWindow->NetUnLock();

			HERO->CalcStatus();
		}

		if( p->IsRemoved() == false )
		{
			p->Removed();

			mDeleteInfluenceArray.PushBack( p );
		}
	}
}

void cSkillManager::ClearSkillCooltime( unsigned long exceptskillIdx )
{
	ClearCoolTimeMon();

	/// skill window cooltime변경
	cSkillWindow* pWin = (cSkillWindow*)UIMAN->FindNodeByID( eUIID_GAME_SKILLWINDOW );
	if( pWin )
	{
		pWin->ClearCoolTime( exceptskillIdx );
	}
	else
	{
		assert(0);
		return;
	}

	cSkillMap::cIterator k, kend;
	k = mKeepSkillMap.Begin();
	kend = mKeepSkillMap.End();
	for( ; k != kend; ++k )
	{
		unsigned long idx = (unsigned long)(*k).mFirst;
		if( exceptskillIdx == idx )
			continue;

		sKeepSkill* pKeepSkill = (sKeepSkill*)(*k).mSecond;

		sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( idx );
		if( pInfo == 0 )
			continue;

		/// Apply Global cooltime 0.5min
		if( pKeepSkill->mRestTime > 0 )
		{
			if( pInfo->mType == eSKILLTYPE_PASSIVE )
				continue;

			pKeepSkill->mRestTime = 0;
		}
	}
}
