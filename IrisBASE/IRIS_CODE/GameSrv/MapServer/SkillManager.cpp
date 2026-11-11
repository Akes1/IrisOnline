#include "gamesrv.h"

#include "stdafx.h"

#include "SkillManager.h"
#include "ObjectManager.h"

#include "BasicString.h"

#include "Skill_Common.h"
#include "Npc_Common.h"
#include "GameFile.h"
#include "BaseObject.h"
#include "Protocol.h"
#include "Player.h"
#include "Monster.h"
#include "Totem.h"
#include "StatusCalc_Server.h"
#include "SkillScript.h"
#include "CommunityScript.h"
#include "MakeSkillScript.h"
#include "RangeCheck.h"
#include "AppTimer.h"
#include "InfluenceObject.h"
#include "DuelManager.h"
#include "StatusCalc_Server.h"
#include "AIManager.h"
#include "DamageCalc.h"
#include "Party.h"
#include "PartyManager.h"
#include "ModeInfo.h"
#include "PartyUnionManager.h"
#include "PartyUnion.h"
#include "RandomTable.h"
#include "GridManager.h"
#include "StageScript.h"
#include "TutorialSkillScript.h"



cSkillManager* cSkillManager::mpSkillManager = NULL;


cSkillManager::cSkillManager()
{
	if(mpSkillManager)
	{
		MessageBox(NULL,"cSkillManager : 이 클래스는 객체가 유일한객체(singleton)이어야 합니다.\
			이전 객체를 사용하던지 이전 객체를 지우고 생성하십시오.","경고",MB_OK);
	}
	else
	{
		mpSkillManager = this;
	}

	mpMakeSkillScript = 0;
	mpCommuScript = 0;
	mpSkillScript = 0;
}



cSkillManager::~cSkillManager()
{
	ReleasePlayerHaveSkill();
	ReleaseInfluence();
	Release();
}



bool cSkillManager::Init()
{
	mPlayerSkillObjectPool.Reserve(1000,100);
	mMonsterSkillObjectPool.Reserve(2000,100);
	mPlayerMonSkillObjectPool.Reserve(1000,100);
	mInfluenceObjectPool.Reserve(5000,100);
	mPlayerHaveSkillPool.Reserve(40000,100);

	mpSkillScript = new cSkillScript;
	if( !mpSkillScript->Init() )
	{
		return false;
	}

	mpCommuScript = new cCommunityScript;
	if( !mpCommuScript->Init() )
	{
		return false;
	}

	if( !mDramaturgyManager.Init() )
	{
		return false;
	}

	mpMakeSkillScript = new cMakeSkillScript;
	if( !mpMakeSkillScript->Init() )
	{
		return false;
	}

	return true;
}



void cSkillManager::Release()
{
	SAFE_DELETE( mpMakeSkillScript );
	SAFE_DELETE( mpCommuScript );
	SAFE_RELEASE_DELETE( mpSkillScript );	

	cPointerHashMap::cIterator iter;

	cBaseSkillObject* pPlayerSkillObj = NULL;
	cMonsterSkillObject* pMonsterSkillObj = NULL;

	cArray* pSkillArray = NULL;
	for(iter = mPlayerSkillMap.Begin(); iter != mPlayerSkillMap.End(); ++iter)
	{
		pSkillArray = (cArray*)(*iter).mSecond;

		if( pSkillArray == NULL )
			continue;

		for( unsigned int i = 0 ; pSkillArray->GetSize() > i ; ++i )
		{
			pPlayerSkillObj = (cBaseSkillObject*)(*pSkillArray)[i];
			SAFE_DELETE( pPlayerSkillObj );
		}
		pSkillArray->Clear();

		SAFE_DELETE( pSkillArray );
	}
	mPlayerSkillMap.Clear();

	for(iter = mMonsterSkillMap.Begin(); iter != mMonsterSkillMap.End(); ++iter)
	{
		pSkillArray = (cArray*)(*iter).mSecond;

		if( pSkillArray == NULL )
			continue;

		for( unsigned int i = 0 ; pSkillArray->GetSize() > i ; ++i )
		{
			pMonsterSkillObj = (cMonsterSkillObject*)(*pSkillArray)[i];
			SAFE_DELETE( pMonsterSkillObj );
		}
		pSkillArray->Clear();

		SAFE_DELETE( pSkillArray );
	}
	mMonsterSkillMap.Clear();
}


void cSkillManager::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	cBaseSkillObject*    pPlayerSkillObj  = NULL;
	cMonsterSkillObject* pMonsterSkillObj = NULL;
	cInfluenceObject*    pInfluenceObject = NULL;

	cPointerHashMap::cIterator begin;
	cPointerHashMap::cIterator end;

try {

	/// 플레이어 액티브 스킬 프로세스
	end = mPlayerSkillMap.End();
	for( begin = mPlayerSkillMap.Begin() ; begin != end; ++begin )
	{
		cArray* pSkillArray = (cArray*)(*begin).mSecond;

		if( pSkillArray != NULL )
		{
			unsigned int arraySize = pSkillArray->GetSize();
			for( unsigned int i = arraySize ; i > 0 ; --i )
			{
				pPlayerSkillObj = (cBaseSkillObject*)(*pSkillArray)[i-1];
				if( pPlayerSkillObj != NULL )
					pPlayerSkillObj->Process( elapsedTime, accumTime );
				else
				{				
					NETWORK2->PostServerEvent( "cSkillManager::Process() pPlayerSkillObj == NULL" );
					pSkillArray->PopAt( i-1 );
				}
			}
		}
		else if ( end != mPlayerSkillMap.End( ) )
		{
			NETWORK2->PostServerEvent( "In if ( end != mPlayerSkillMap.End( ) ) { ... }. Throwing 'cSkillManager::Process' exception." );
		}
		else
			assert(NULL);
	}

} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mPlayerSkillMap ) { (0x%x)->Process( %u, %u ); }. Throwing 'cSkillManager::Process' exception.", pPlayerSkillObj, elapsedTime, accumTime );
	throw;
}

try {

	/// 몬스터 액티브 스킬 프로세스
	end = mMonsterSkillMap.End();
	for( begin = mMonsterSkillMap.Begin() ; begin != end; ++begin )
	{
		cArray* pSkillArray = (cArray*)(*begin).mSecond;
		if( pSkillArray != NULL )
		{
			unsigned int arraySize = pSkillArray->GetSize( );
			for( unsigned int i = arraySize ; i > 0 ; --i )
			{
				pMonsterSkillObj = (cMonsterSkillObject*)(*pSkillArray)[i-1];
				if( pMonsterSkillObj != NULL )
					pMonsterSkillObj->Process( elapsedTime, accumTime );
				else
				{
					NETWORK2->PostServerEvent("cSkillManager::Process() pMonsterSkillObj == NULL");
					pSkillArray->PopAt( i-1 );
				}
			}
		}
		else if ( end != mMonsterSkillMap.End( ) )
		{
			NETWORK2->PostServerEvent( "In if ( end != mMonsterSkillMap.End( ) ) { ... }. Throwing 'cSkillManager::Process' exception." );
		}
		else
			assert(NULL);
	}

} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mMonsterSkillMap ) { (0x%x)->Process( %u, %u ) }. Throwing 'cSkillManager::Process' exception.", pMonsterSkillObj, elapsedTime, accumTime );
	throw;
}


try {

	/// 효과 리스트 프로세스
	end = mInfluenceMap.End();
	for( begin = mInfluenceMap.Begin() ; begin != end; ++begin )
	{
		pInfluenceObject = (cInfluenceObject*)((*begin).mSecond);
		if( pInfluenceObject != NULL )
		{
			if( pInfluenceObject->Process( elapsedTime, accumTime ) == false )
                mDeleteInfluenceAry.PushBack( pInfluenceObject->GetUniqueIdx() );
		}
		else if ( end != mInfluenceMap.End( ) )
			NETWORK2->PostServerEvent( "In if ( end != mInfluenceMap.End( ) ) { ... }. Throwing 'cSkillManager::Process' exception." );
		else
			assert(NULL);
	}

} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mInfluenceMap ) { (0x%x)->Process( %u, %u ) }. Throwing 'cSkillManager::Process' exception.", pInfluenceObject, elapsedTime, accumTime );
	throw;
}


try {
	/// 플레이어 hp/mp 변동 메세지 발송
	cHashSet::cIterator b = mPlayerChangeSet.Begin();
	cHashSet::cIterator e = mPlayerChangeSet.End();
	for( ; b != e ; ++b )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( *b );
		if( pPlayer != NULL )
		{
			if( pPlayer->GetStateDie() == true )
			{
				MSG_SYN_SKILL_APPLY_DIE sync;
				sync.Category = NM_SKILL;
				sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				sync.mAttacker = pPlayer->GetSkillLastAttack();
				sync.mDamageKind = pPlayer->GetSkillLastKind();
				sync.mTarget = pPlayer->GetObject();				
				NETWORK2->QuickSend( pPlayer, (char*)&sync, sizeof(sync) );
			}
			else
			{
				MSG_SYN_SKILL_APPLY_HPMP sync;
				sync.Category = NM_SKILL;
				sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				sync.mTarget = pPlayer->GetObject();
				sync.mHP = pPlayer->GetHP();
				sync.mMaxHP = pPlayer->GetMaxHP();
				sync.mMP = pPlayer->GetMP();
				sync.mMaxMP = pPlayer->GetMaxMP();
				NETWORK2->QuickSend( pPlayer, (char*)&sync, sizeof(sync) );
			}
		}
	}
	mPlayerChangeSet.Clear();
} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mDeleteSkillAry ) { ... }. Throwing 'cSkillManager::Process' exception." );
	throw;
}

try {
	/// 몬스터 hp/mp 변동 메세지 발송
	cHashSet::cIterator b = mMonsterChangeSet.Begin();
	cHashSet::cIterator e = mMonsterChangeSet.End();
	for( ; b != e ; ++b )
	{
		cMonster* pMonster = OBJECTMANAGER->GetMonster( *b );
		if( pMonster != NULL )
		{
			if( pMonster->GetStateDie() == true )
			{
				MSG_SYN_SKILL_APPLY_DIE sync;
				sync.Category = NM_SKILL;
				sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				sync.mAttacker = pMonster->GetSkillLastAttack();
				sync.mDamageKind = pMonster->GetSkillLastKind();
				sync.mTarget = pMonster->GetObject();				
				NETWORK2->QuickSend( pMonster, (char*)&sync, sizeof(sync) );
			}
			else
			{
				MSG_SYN_SKILL_APPLY_HPMP sync;
				sync.Category = NM_SKILL;
				sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				sync.mTarget = pMonster->GetObject();
				sync.mHP = pMonster->GetHP();
				sync.mMaxHP = pMonster->GetMaxHP();
				sync.mMP = pMonster->GetMP();
				sync.mMaxMP = pMonster->GetMaxMP();
				NETWORK2->QuickSend( pMonster, (char*)&sync, sizeof(sync) );
			}
		}
	}
	mMonsterChangeSet.Clear();
} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mDeleteSkillAry ) { ... }. Throwing 'cSkillManager::Process' exception." );
	throw;
}

try {

	/// 플레이어, 몬스터 액티브 스킬 삭제
	unsigned int DelSkillArySize = mDeleteSkillAry.GetSize( );

	sDelSkillInfo* info;
	for( unsigned int i = 0 ; i < DelSkillArySize ; ++i )
	{
		info = &mDeleteSkillAry[i];
		if( info != NULL )
		{
			if( DeleteSkillObject( info->skillUser.type, info->skillUser.index, info->mSkillIdx ) == false )
				NETWORK2->PostServerEvent( "cSkillManager::Process DeleteSkillObject[%d,%d,%d]", info->skillUser.type, info->skillUser.index, info->mSkillIdx );
		}                		
	}
	mDeleteSkillAry.Clear();

} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mDeleteSkillAry ) { ... }. Throwing 'cSkillManager::Process' exception." );
	throw;
}

try {

	/// 효과 삭제 목록 처리
	unsigned int mDelSize = mDeleteInfluenceAry.GetSize( );
	for( unsigned int i = 0; i < mDelSize; ++i )
	{
		unsigned long uniqueIdx = mDeleteInfluenceAry[i];
		cInfluenceObject* pInf = GetInfluence( uniqueIdx );
		if( pInf != NULL )
			DeleteInfluence( pInf->GetTarget(), uniqueIdx );
	}
	mDeleteInfluenceAry.Clear();

} catch ( ... ) {
	NETWORK2->PostServerEvent( "In for( mDeleteInfluenceAry ) { ... }. Throwing 'cSkillManager::Process' exception." );
	throw;
}

}



bool cSkillManager::IsUsingSkill( unsigned char type, unsigned long objectIdx ) 
{
	switch( type )
	{
	case eOBJECTTYPE_MONSTER:
		{
			cArray* pSkillArray = (cArray*)mMonsterSkillMap.GetAt( objectIdx );
			if( pSkillArray == NULL )
			{
				return false;
			}

			cMonsterSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cMonsterSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
					continue;

				/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
				if( pSkillObject->GetSkillState() < eSKILLPROCESS_PROJECTILE )
					return true;
			}
		}
		break;
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			cArray* pSkillArray = (cArray*)mPlayerSkillMap.GetAt( objectIdx );
			if( pSkillArray == NULL )
			{
				return false;
			}

			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
					continue;

				if( pSkillObject->IsProjectile() == false )
				{
					/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
					if( pSkillObject->GetSkillState() < eSKILLPROCESS_HP || pSkillObject->IsDetailEnd() == false )
						return true;
				}
				else
				{
					/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
					if( pSkillObject->GetSkillState() < eSKILLPROCESS_PROJECTILE )
						return true;
				}
			}
		}
		break;		
	default:
		assert(NULL);
		NETWORK2->PostServerEvent("IsUsingSkill type error[%d]", type );
		return false;
	}

	return false;
}


unsigned long cSkillManager::SkillLeftEndTime( unsigned char type, unsigned long objectIdx ) 
{
	switch( type )
	{
	case eOBJECTTYPE_MONSTER:
		{
			cArray* pSkillArray = (cArray*)mMonsterSkillMap.GetAt( objectIdx );
			if( pSkillArray == NULL )
			{
				return 0;
			}

			cMonsterSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cMonsterSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
					continue;

				/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
				if( pSkillObject->GetSkillState() < eSKILLPROCESS_PROJECTILE )
					return pSkillObject->SkillEndTime();
			}
		}
		break;
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			cArray* pSkillArray = (cArray*)mPlayerSkillMap.GetAt( objectIdx );
			if( pSkillArray == NULL )
			{
				return 0;
			}

			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
					continue;

				if( pSkillObject->IsProjectile() == false )
				{
					/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
					if( pSkillObject->GetSkillState() < eSKILLPROCESS_HP || pSkillObject->IsDetailEnd() == false )
						return pSkillObject->SkillEndTime();
				}
				else
				{
					/// 데미지 적용까지 가기전 단계이면 스킬 사용 중
					if( pSkillObject->GetSkillState() < eSKILLPROCESS_PROJECTILE )
						return pSkillObject->SkillEndTime();
				}
			}
		}
		break;		
	default:
		assert(NULL);
		NETWORK2->PostServerEvent("IsUsingSkill type error[%d]", type );
		return 0;
	}

	return 0;
}


void cSkillManager::CastSkillCancel( sObject attacker, bool isObstacle, unsigned long attributeType )
{
	if( attacker.type == eOBJECTTYPE_PLAYER || attacker.type == eOBJECTTYPE_HERO )
	{
		cPlayer* pAttacker = OBJECTMANAGER->GetPlayer(attacker.index);
		if( pAttacker == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::CastSkillCancel - attacker[%d,%d]", attacker.type, attacker.index );
			return;
		}

		unsigned long skillIndex = 0;

		cArray* pSkillArray = (cArray*)mPlayerSkillMap.GetAt( attacker.index );
		if( pSkillArray != NULL )
		{
			bool isCasting = false;
			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("CastSkillCancel - mPlayerSkillMap.GetAt( %d )", attacker.index );
					continue;
				}

				if( pSkillObject->GetSkillState() == eSKILLPROCESS_CASTING || pSkillObject->IsChanneling() )
				{
					if( attributeType != eATTRIBUTETYPE_NONE && pSkillObject->GetAttributeType() != (eATTRIBUTETYPE)attributeType )
						break;

					skillIndex = pSkillObject->GetUniqueIdx();
					/// 캐스팅 취소처리
					DeleteSkillObject( attacker.type, attacker.index, skillIndex );
					if( pAttacker->ChangeState( eOBJECT_STATE_IDLE ) == false )
					{
						if( pAttacker->GetStateDie() == false )
						{
							NETWORK2->PostServerEvent("cSkillManager::CastSkillCancel pAttacker[%d,%d,%d,%d]->ChangeState( eOBJECT_STATE_IDLE ) == false",
								attacker.index, pAttacker->GetState(), pAttacker->GetStateStop(), pAttacker->GetRequestRejection() );
						}
					}

					isCasting = true;
					break;
				}
			}

			/// 어떤 스킬도 캐스팅 중이아니면 취소성공 메세지를 보내지 않는다.
			if( isCasting == false )
				return;
		}

		if( isObstacle == false )		/// 유저가 직접 캐스팅을 끈경우
		{
			/// 캐스팅 취소 응답 메세지는 취소 성공시에만 발송된다.
			HANDLE                     handle  = NULL;
			MSG_RES_SKILL_CAST_CANCEL* pResMsg = (MSG_RES_SKILL_CAST_CANCEL*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_CAST_CANCEL_RES );
			if ( pResMsg != NULL )
			{
				pResMsg->mSkillIndex = skillIndex;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_CAST_CANCEL) );
			}

			MSG_SYN_SKILL_CAST_CANCEL pSynMsg;
			pSynMsg.Category = NM_SKILL;
			pSynMsg.Protocol = NM_SKILL_CAST_CANCEL_SYN;
			pSynMsg.mSkillIndex = skillIndex;
			NETWORK2->QuickSendExcept( pAttacker, (char*)&pSynMsg, sizeof(pSynMsg) );
		}
		else		/// 상태이상 효과로 인한 캐스팅을 중지당한 경우
		{
			/// 캐스팅 발동 실패 메세지 전송
			MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
			failMsg.Category = NM_SKILL;
			failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
			failMsg.mAttacker = attacker;
			failMsg.mSkillIndex = skillIndex;
			failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;

			NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );
		}
	}
	else if( attacker.type == eOBJECTTYPE_MONSTER )
	{
		cMonster* pAttacker = OBJECTMANAGER->GetMonster(attacker.index);
		if( pAttacker == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::CastSkillCancel - attacker[%d,%d]", attacker.type, attacker.index );
			return;
		}

		unsigned long skillIndex = 0;

		cArray* pSkillArray = (cArray*)mMonsterSkillMap.GetAt( attacker.index );
		if( pSkillArray != NULL )
		{
			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
				{
					NETWORK2->PostServerEvent("CastSkillCancel - mMonsterSkillMap.GetAt( %d )", attacker.index );
					continue;
				}

				if( pSkillObject->GetSkillState() == eSKILLPROCESS_CASTING )
				{
					if( attributeType != eATTRIBUTETYPE_NONE && pSkillObject->GetAttributeType() != (eATTRIBUTETYPE)attributeType )
						break;

					skillIndex = pSkillObject->GetUniqueIdx();
					/// 캐스팅 취소처리
					DeleteSkillObject( attacker.type, attacker.index, skillIndex );
					pAttacker->CalcSkillWaitTime( NETWORK2->GetAccumTime() );

					/// 캐스팅 발동 실패 메세지 전송
					MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
					failMsg.Category = NM_SKILL;
					failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
					failMsg.mAttacker = attacker;
					failMsg.mSkillIndex = skillIndex;
					failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;

					NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );
					return;
				}
			}
		}
	}
}

void cSkillManager::ChannelingSkillCancel( sObject attacker, bool isObstacle, unsigned long attributeType, bool isDuelEndCancel )
{
	if( attacker.type == eOBJECTTYPE_PLAYER || attacker.type == eOBJECTTYPE_HERO )
	{
		cPlayer* pAttacker = OBJECTMANAGER->GetPlayer(attacker.index);
		if( pAttacker == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::ChannelingSkillCancel - attacker[%d,%d]", attacker.type, attacker.index );
			return;
		}

		unsigned long skillIndex = 0;
		bool isActivity = false;

		cArray* pSkillArray = (cArray*)mPlayerSkillMap.GetAt( attacker.index );
		if( pSkillArray != NULL )
		{
			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("ChannelingSkillCancel - mPlayerSkillMap.GetAt( %d )", attacker.index );
					continue;
				}

				if( pSkillObject->GetSkillState() == eSKILLPROCESS_ACTIVITY && pSkillObject->IsChanneling() == true )
				{
					if( attributeType != eATTRIBUTETYPE_NONE && pSkillObject->GetAttributeType() != (eATTRIBUTETYPE)attributeType )
						break;

					skillIndex = pSkillObject->GetUniqueIdx();
					/// 캐스팅 취소처리
					DeleteSkillObject( attacker.type, attacker.index, skillIndex );
					if( pAttacker->ChangeState( eOBJECT_STATE_IDLE ) == false )
					{
						if( pAttacker->GetStateDie() == false )
						{
							NETWORK2->PostServerEvent("cSkillManager::ChannelingSkillCancel pAttacker[%d,%d,%d,%d]->ChangeState( eOBJECT_STATE_IDLE ) == false",
								attacker.index, pAttacker->GetState(), pAttacker->GetStateStop(), pAttacker->GetRequestRejection() );
						}
					}

					isActivity = true;
					break;
				}
			}
		}

		/// 어떤 스킬도 캐스팅 중이아니면 취소성공 메세지를 보내지 않는다.
		if( isActivity == false )
			return;

		if( isObstacle == false )		/// 유저가 직접 캐스팅을 끈경우
		{
			/// 캐스팅 취소 응답 메세지는 취소 성공시에만 발송된다.
			HANDLE                     handle  = NULL;
			MSG_RES_CHANNELING_SKILL_CANCEL* pResMsg = (MSG_RES_CHANNELING_SKILL_CANCEL*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_CHANNELING_CANCEL_RES );
			if ( pResMsg != NULL )
			{
				pResMsg->mUniqueSkillIndex = skillIndex;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_CHANNELING_SKILL_CANCEL) );
			}

			MSG_SYN_CHANNELING_SKILL_CANCEL pSynMsg;
			pSynMsg.Category = NM_SKILL;
			pSynMsg.Protocol = NM_SKILL_CHANNELING_CANCEL_SYN;
			pSynMsg.mUniqueSkillIndex = skillIndex;
			NETWORK2->QuickSendExcept( pAttacker, (char*)&pSynMsg, sizeof(pSynMsg) );
		}
		else		/// 상태이상 효과로 인한 캐스팅을 중지당한 경우
		{
			/// 캐스팅 발동 실패 메세지 전송
			MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
			failMsg.Category = NM_SKILL;
			failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
			failMsg.mAttacker = attacker;
			failMsg.mSkillIndex = skillIndex;	
			failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;

			if( isDuelEndCancel )
				failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_DUEL_END;

			NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );
		}
	}
	else if( attacker.type == eOBJECTTYPE_MONSTER )
	{
		cMonster* pAttacker = OBJECTMANAGER->GetMonster(attacker.index);
		if( pAttacker == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::ChannelingSkillCancel - attacker[%d,%d]", attacker.type, attacker.index );
			return;
		}

		unsigned long skillIndex = 0;

		cArray* pSkillArray = (cArray*)mMonsterSkillMap.GetAt( attacker.index );
		if( pSkillArray != NULL )
		{
			cBaseSkillObject* pSkillObject = NULL;
			for( unsigned int i = 0 ; i < pSkillArray->GetSize() ; ++i )
			{
				pSkillObject = (cBaseSkillObject*)(*pSkillArray)[i];
				if( pSkillObject == NULL )
				{
					NETWORK2->PostServerEvent("ChannelingSkillCancel - mMonsterSkillMap.GetAt( %d )", attacker.index );
					continue;
				}

				if( attributeType != eATTRIBUTETYPE_NONE && pSkillObject->GetAttributeType() != (eATTRIBUTETYPE)attributeType )
					break;

				skillIndex = pSkillObject->GetUniqueIdx();
				/// 캐스팅 취소처리
				DeleteSkillObject( attacker.type, attacker.index, skillIndex );
				pAttacker->CalcSkillWaitTime( NETWORK2->GetAccumTime() );

				/// 캐스팅 발동 실패 메세지 전송
				MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
				failMsg.Category = NM_SKILL;
				failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
				failMsg.mAttacker = attacker;
				failMsg.mSkillIndex = skillIndex;
				failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;

				NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );
				return;
			
			}
		}
	}
}

void cSkillManager::PlayerNormalAttackRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg )
{

	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerNormalAttackRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MoveStop();
	pAttacker->SkillReqCntPlus();	

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 기본 공격 스킬 인덱스가 아님
		if( skillIdx > NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerNormalAttackRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 타겟 객체 체크
		cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
		if( pTarget == NULL )	
			throw SKILL_USED_ERR_TARGETOBJECT;

		pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

		if( pTarget->GetStateDie() == true )	
			throw SKILL_USED_ERR_TARGETOBJECT;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( target.type, target.index );


		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker     = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget       = target;
				posMsg->mTargetPosX   = pTarget->GetXPos( );
				posMsg->mTargetPosY   = pTarget->GetYPos( );

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}


		/// 이미 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
		{
			pAttacker->SkillCoolTimeFailPlus();
			throw SKILL_USED_ERR_USING;
		}


		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("playerNormalAttackRequest skillScript[%d]", skillIdx );
			throw SKILL_ADD_ERR_SCRIPT;
		}


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("playerNormalAttackRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}


		/// 기본 공격도 쿨타임 체크를 해야 하기 때문에 보유스킬 리스트에 대표번호(1)로 넣는다.
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( attacker.index, NORMAL_ATTACK_SKILL );
		if( !pPlayerHaveSkill )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent("playerNormalAttackRequest GetPlayerHaveSkill[%d,%d]", attacker.index, NORMAL_ATTACK_SKILL );
			throw SKILL_USED_ERR_DONTHAVE;
		}


		/// 스킬 스크립트 스텝정보 읽어옴
		unsigned int haveStep = pPlayerHaveSkill->GetSkillStep();
		if( haveStep < skillStep )
		{
			NETWORK2->PostServerEvent("cSkillManager::PlayerNormalAttackRequest haveStep < skillStep(%d,%d,%d)", NORMAL_ATTACK_SKILL, haveStep, skillStep );
			throw SKILL_USED_ERR_DONTHAVE;
		}
		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];

		/// 내 장비가 맞는지 체크
		eWEAPON_STATE weaponState = pAttacker->GetWeaponState();
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			NETWORK2->PostServerEvent("player[%d]NormalAttack[%d]Request IsEquipment[%d,%d]== false", 
				attacker.index, weaponState,pAttacker->GetArmorSets(), skillIdx );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 타겟 조건 체크
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			unsigned long attackerTeamType = pAttacker->GetPvPDMTeam();
			if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
			{
				if( attackerTeamType == ((cMonster*)pTarget)->GetDMTeamType() )
					throw SKILL_USED_ERR_TARGETSELECT;
			}
		}
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			if( pAttacker->IsAttack( target.index ) == false )
				throw SKILL_USED_ERR_TARGETSELECT;
		}
		else
			throw SKILL_USED_ERR_TARGETSELECT;

		/// 스킬+아이템+효과 사거리
		float range = pAttacker->CalcStatusSkillRange( pStepInfo->mTargetDist, pSkillInfo->mRangeType, true );

		range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

		/// 클라,서버 도착 시간 차이만큼의 거리 허용치
		cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );

		NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
		NiPoint3 targetPos( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height() );

		/// 스킬 사거리 밖에 타겟이 있다.
		if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
		{ 
			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );

			if ( posMsg != NULL )
			{
				posMsg->mAttacker     = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget       = target;
				posMsg->mTargetPosX   = pTarget->GetXPos( );
				posMsg->mTargetPosY   = pTarget->GetYPos( );

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_TARGETDIST;
		}


		unsigned long currentTime = NETWORK2->GetAccumTime();

		/// 쿨타임
		if( pStepInfo->mCoolTime != 0 && pPlayerHaveSkill->GetEndCoolTime() > currentTime )	
		{ 			
			pAttacker->SkillCoolTimeFailPlus();

			throw SKILL_USED_ERR_COOLTIME;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "playerNormalAttackRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		unsigned char errorCode = pSkillObject->InitPlayerNormalAttack( characterIdx, target, skillIdx, skillStep, skillUniqueIdx );
		if( errorCode != 0 )
		{ 
			NETWORK2->PostServerEvent( "playerNormalAttackRequest new SkillObject[%d] == false", errorCode );
			SAFE_DELETE( pSkillObject );
			throw SKILL_USED_ERR_OBJECTINIT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "playerNormalAttackRequest VehicleAttack[%d,%d,%d]", characterIdx, skillIdx, skillStep );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_USED_RES );

		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_USED_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = pTarget->GetXPos();
		synMsg.mTargetPosY = pTarget->GetYPos();
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->SkillSuccessCntPlus();

//		/// 발동 체크 
//		CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
		return;

	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_USED_RES );

		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::PlayerObjectSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool casting )
{

	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;


	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerObjectSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MoveStop();
	pAttacker->SkillReqCntPlus();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 기본 공격 스킬 인덱스인경우 오류
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 스킬 사용 불가
		if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
			throw SKILL_USED_ERR_USEVALUE;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING && casting != true )
		{
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest[%d] ShotType != eSHOTTYPE_CASTING[%d]", characterIdx, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}


		if( pSkillInfo->mShotType != eSHOTTYPE_CASTING && casting == true )
		{
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest[%d] ShotType == eSHOTTYPE_CASTING[%d]", characterIdx, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}


		/// 공격자의 보유스킬인지 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( attacker.index, skillIdx );
		if( !pPlayerHaveSkill )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest GetPlayerHaveSkill[%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_DONTHAVE;
		}
		unsigned char stepIdx = pPlayerHaveSkill->GetSkillStep();

		if( stepIdx < skillStep )
		{
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest stepIdx < skillStep[%d,%d,%d,%d]", attacker.index, skillIdx, stepIdx, skillStep );
			throw SKILL_USED_ERR_DONTHAVE;
		}

		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];

		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pAttacker->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest IsEquipment == false" );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{

			if( pAttacker->GetMaxHP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 1");
				throw SKILL_USED_ERR_USEVALUE;
			}

			if( pAttacker->GetMaxMP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 2");
				throw SKILL_USED_ERR_USEVALUE;
			}

			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerObjectSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}


		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모 HP 수치 체크
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}

		cBaseObject* pTarget = NULL;
		NiPoint3 targetPos = NiPoint3::ZERO;

		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_SELF_NOTARGET:			/// 자신이 범위 시작위치여서 거리체크 안함	
			{
				/// 자신을 타겟으로 선정
				target = attacker;
				targetPos.x = pAttacker->GetXPos();
				targetPos.y = pAttacker->GetYPos();
				targetPos.z = pAttacker->Height();
				pTarget = pAttacker;
			}
			break;
		case eBOUNDTYPE_NONE:					/// 범위 없음
		case eBOUNDTYPE_TARGET:					/// 타겟 기준범위
		case eBOUNDTYPE_SELF_TARGET:			/// 시전자(자신) 기준범위 - 발동시 타겟 존재
			{

				/// 자신 일경우 거리체크 안함
				if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF || 
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF || 
					pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
				{
					/// 자신을 타겟으로 선정
					target = attacker;
					targetPos.x = pAttacker->GetXPos();
					targetPos.y = pAttacker->GetYPos();
					targetPos.z = pAttacker->Height();
					pTarget = pAttacker;
					break;
				}

				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pStepInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "PlayerObjectSkillRequest[%d] mTargetDist == 0", skillIdx );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 부활 스킬인지 체크
				bool isResurrection = false;

				if( pSkillInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
				{
					isResurrection = true;
					if( NETWORK2->GetServerType() == _E_ST_ID_PVP_ )
						throw SKILL_USED_ERR_TARGETSELECT;
				}

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true && isResurrection == false )
					throw SKILL_USED_ERR_TARGETOBJECT;
				else if( pTarget->GetStateDie() == false && isResurrection == true )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
				case eAPPLYTYPE_BUFF_ENEMY:
				case eAPPLYTYPE_DEBUFF_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type == eOBJECTTYPE_MONSTER )
						{
							unsigned long attackerTeamType = pAttacker->GetPvPDMTeam();
							if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
							{
								if( attackerTeamType == ((cMonster*)pTarget)->GetDMTeamType() )
									throw SKILL_USED_ERR_TARGETSELECT;
							}
						}
						else if( target.type == eOBJECTTYPE_PLAYER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_BUFF_PARTY:
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 자신은 무조건 가능
						if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
							break;

						bool isParty = false;
						
						/// 타겟이 파티원인지 체크
						cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
						if( pParty && pParty->IsUser( target.index ) )
						{
							isParty = true;
						}
						else
						{
							/// 파티연합인지 체크
							cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
							if( partyUnion && partyUnion->IsUser( pAttacker->GetObjectID(), target.index ) )
							{
								isParty = true;
							}
						}

						/// 타겟이 파티원이 아닌 경우
						if( isParty == false )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				}

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pStepInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

			}break;
			/// 필드인경우는 이함수에 들어올수 없음
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				/// 스크립트 내용이 잘못됐다.
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerObjectSkillRequest mBoundType[%d,%d]", skillIdx, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
		if( pSkillInfo->mApplyValueType != eSTATUSPLUS_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_BUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_DEBUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_BUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_DEBUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_BUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;

				}
				break;
			case eAPPLYTYPE_DEBUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}
		
					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;

			}//end switch( pSkillInfo->mApplyType )
		}

		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		unsigned long currentTime = NETWORK2->GetAccumTime();

		/// 쿨타임
		if( pStepInfo->mCoolTime != 0 && pPlayerHaveSkill->GetEndCoolTime() > currentTime )	
		{ 
			pAttacker->SkillCoolTimeFailPlus();

			throw SKILL_USED_ERR_COOLTIME;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitPlayerSkill( skillIdx, skillStep, skillUniqueIdx, pAttacker, pTarget,  pStepInfo->mCastingTime );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			assert(NULL);
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest VehicleAttack[%d,%d,%d]", characterIdx, skillIdx, skillStep );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 캐스팅을 하지 않는 스킬이면 스킬사용시 소모항목 감소
		if( casting == false )
		{
			/// MP를 소모 하는 스킬
			if( useMP != 0 )
				pAttacker->MPUse( useMP, false );
			
			/// HP를 소모 하는 스킬
			if( pStepInfo->mUseHP != 0 )
			{
				/// HP 소모 & 메세지 발송
				pAttacker->HPUse( pStepInfo->mUseHP, true );
			}

			/// 소모아이템 IDX
			if( pStepInfo->mUseItemCount != 0 )
			{
				if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
				}
			}

			/// 캐스팅이 아닌 경우 발동 체크 
			/// *** 캐스팅인 경우는 skillObj의 RequireValue() 에서 체크 ***
			switch( pSkillInfo->mType )
			{
			case eSKILLTYPE_DAMAGE:
			case eSKILLTYPE_MPDAMAGE:
				CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
				break;
			case eSKILLTYPE_HEAL:
				CheckActInfluence( eACTSTATUS_HEALSKILL, pAttacker, pTarget );
				break;
			}

			/// 스킬 발동 효과
			if( pStepInfo->mActiveIdx != 0 )
				SKILLMANAGER->CheckSkillActInfluence( pStepInfo->mActiveIdx, pAttacker, pTarget );
				
		}

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			if( casting == true )
				resMsg->Protocol = NM_SKILL_CAST_USED_RES;
			else
				resMsg->Protocol = NM_SKILL_USED_RES;

			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		/// 캐스팅 여부에 따른 메세지 내용 정의
		if( casting == true )
			synMsg.Protocol = NM_SKILL_CAST_USED_SYN;
		else
			synMsg.Protocol = NM_SKILL_USED_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = targetPos.x;
		synMsg.mTargetPosY = targetPos.y;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->SkillSuccessCntPlus();
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			if( casting == true )
				resMsg->Protocol = NM_SKILL_CAST_USED_RES;
			else
				resMsg->Protocol = NM_SKILL_USED_RES;

			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}



void cSkillManager::PlayerPositionSkillRequest( unsigned long characterIdx, MSG_REQ_POS_USED* pMsg, bool casting )
{

	/// 이함수를 통해 들어오는 공격자의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	float targetXPos = pMsg->mTargetPosX;
	float targetYPos = pMsg->mTargetPosY;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;


	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerPositionSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MoveStop();
	pAttacker->SkillReqCntPlus();

	pAttacker->CalcDirection( targetXPos, targetYPos );

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;
		
		/// 기본 공격 스킬 인덱스인경우 오류
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용 불가
		if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
			throw SKILL_USED_ERR_USEVALUE;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 이미 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( eOBJECTTYPE_NONE, 0 );


		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos( pAttacker->GetXPos(), pAttacker->GetYPos() );

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker     = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget.type  = eOBJECTTYPE_NONE;
				posMsg->mTargetPosX   = 0.0f;
				posMsg->mTargetPosY   = 0.0f;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}


		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING && casting != true )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest[%d] ShotType != eSHOTTYPE_CASTING[%d]", characterIdx, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}


		if( pSkillInfo->mShotType != eSHOTTYPE_CASTING && casting == true )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest[%d] ShotType == eSHOTTYPE_CASTING[%d]", characterIdx, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}

		/// 공격자의 보유스킬인지 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( attacker.index, skillIdx );
		if( !pPlayerHaveSkill )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest GetPlayerHaveSkill[%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_DONTHAVE;
		}
		unsigned char stepIdx = pPlayerHaveSkill->GetSkillStep();

		if( stepIdx < skillStep )
		{
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest stepIdx < skillStep[%d,%d,%d,%d]", attacker.index, skillIdx, stepIdx, skillStep );
			throw SKILL_USED_ERR_DONTHAVE;
		}

		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];

		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pAttacker->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerPositionSkillRequest IsEquipment == false" );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{
			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerPositionSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}


		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;

		/// 소모 HP
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}


		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
			/// 필드인경우만 이함수에 들어옴
		case eBOUNDTYPE_AUTOFIELD:
		case eBOUNDTYPE_FIELD:
			{
				NiPoint3 targetPos( targetXPos, targetYPos, 0.0f );
				AIMANAGER->CalcHeight( pAttacker->GetMapNumber(), &targetPos.z, targetXPos, targetYPos );

				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pStepInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, NULL, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos();
						posMsg->mAttackerPosY = pAttacker->GetYPos();
						posMsg->mTarget.type  = eOBJECTTYPE_NONE;
						posMsg->mTargetPosX   = 0.0f;
						posMsg->mTargetPosY   = 0.0f;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

			}break;
		default:
			{
				/// 스크립트 내용이 잘못됐다.
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerPositionSkillRequest pSkillInfo->mBoundType[%d]", pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
		if( pSkillInfo->mApplyValueType != eSTATUSPLUS_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_BUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_DEBUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case eAPPLYTYPE_BUFF_ENEMY:
			case eAPPLYTYPE_DEBUFF_ENEMY:
			case eAPPLYTYPE_BUFF_PARTY:
			case eAPPLYTYPE_DEBUFF_PARTY:
				NETWORK2->PostServerEvent("PlayerPositionSkillRequest pSkillInfo->mApplyValueType[%d] bad value", pSkillInfo->mApplyType );
				throw SKILL_USED_ERR_APPLYVALUE_STATE;
				break;

			}//end switch( pSkillInfo->mApplyType )
		}

		unsigned long currentTime = NETWORK2->GetAccumTime();

		/// 쿨타임
		if( pStepInfo->mCoolTime != 0 && pPlayerHaveSkill->GetEndCoolTime() > currentTime )	
		{ 
			pAttacker->SkillCoolTimeFailPlus();

			throw SKILL_USED_ERR_COOLTIME;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "PlayerPositionSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitPlayerSkillPosition( characterIdx, targetXPos, targetYPos, skillIdx, skillStep, skillUniqueIdx, casting );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			assert(NULL);
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "PlayerPositionSkillRequest new SkillObject" );
			throw SKILL_USED_ERR_OBJECTINIT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "PlayerPositionSkillRequest VehicleAttack[%d,%d,%d]", characterIdx, skillIdx, skillStep );


		/// 스킬 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 캐스팅을 하지 않는 스킬이면 스킬사용시 소모항목 감소
		if( casting == false )
		{
			/// MP를 소모 하는 스킬
			if( useMP != 0 )
				pAttacker->MPUse( useMP, false );

			/// HP를 소모 하는 스킬
			if( pStepInfo->mUseHP != 0 )
			{
				/// HP 소모 & 메세지 발송
				pAttacker->HPUse( pStepInfo->mUseHP, true );
			}


			/// 소모아이템 IDX
			if( pStepInfo->mUseItemCount != 0 )
			{
				if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
				}
			}
		}

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL );

		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			if( casting == true )
				resMsg->Protocol = NM_SKILL_POS_CAST_USED_RES;
			else
				resMsg->Protocol = NM_SKILL_POS_USED_RES;

			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_POS_USED synMsg;
		synMsg.Category = NM_SKILL;
		/// 캐스팅 여부에 따른 메세지 내용 정의
		if( casting == true )
			synMsg.Protocol = NM_SKILL_POS_CAST_USED_SYN;
		else
			synMsg.Protocol = NM_SKILL_POS_USED_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTargetPosX = targetXPos;
		synMsg.mTargetPosY = targetYPos;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->SkillSuccessCntPlus();
	}
	catch( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL );
		if ( resMsg != NULL )
		{
			if( casting == true )
				resMsg->Protocol = NM_SKILL_POS_CAST_USED_RES;
			else
				resMsg->Protocol = NM_SKILL_POS_USED_RES;

			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::PlayerMonSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool casting )
{

	eMONSTERATTACK_TYPE attackType = (eMONSTERATTACK_TYPE)pMsg->mSkillClassIndex;

	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long clientIdx = pMsg->mManagedKey;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerMonSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MonSkillReqCntPlus();

	pAttacker->MoveStop();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 몬스터 변신중이 아닌데 몬스터 공격이 들어온 경우 - 메세지 시간차로 인해 들어올수 있음
		unsigned long monsterClassIdx = pAttacker->GetChgMonsterIdx();
		if( monsterClassIdx == 0 )
			throw SKILL_USED_ERR_SKILLTYPE;

		if( attackType >= eMONSTERATTACK_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerMonSkillRequest - attackType[%d] > eMONSTERATTACK_MAX", attackType );
			throw SKILL_USED_ERR_SKILLTYPE;
		}


		/// 스킬index 체크
		sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, attackType );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonSkillRequest skillScript[%d,%d]", monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용 불가
		if( attackType > eMONSTERATTACK_NORMAL2 )
		{
			/// 기본 공격은 제외하고 처리한다.
			if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
				throw SKILL_USED_ERR_USEVALUE;
		}

		if( pAttacker->GetChgMonsterIdx() == 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
		{
			pAttacker->MonSkillCoolTimeFailPlus();
			throw SKILL_USED_ERR_USING;
		}


		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING && casting != true )
		{
			NETWORK2->PostServerEvent("PlayerMonSkillRequest[%d,%d] ShotType != eSHOTTYPE_CASTING[%d]", characterIdx, monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}


		if( pSkillInfo->mShotType != eSHOTTYPE_CASTING && casting == true )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonSkillRequest[%d,%d] ShotType == eSHOTTYPE_CASTING[%d]", characterIdx, monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 소모 MP 체크
		unsigned long useMP = pSkillInfo->mUseMP;
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( pSkillInfo->mUseMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
 			throw SKILL_USED_ERR_USEVALUE;


		cBaseObject* pTarget = NULL;
		NiPoint3 targetPos = NiPoint3::ZERO;


		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_SELF_NOTARGET:			/// 자신이 범위 시작위치여서 거리체크 안함	
			break;
		case eBOUNDTYPE_NONE:			/// 범위 없음
		case eBOUNDTYPE_TARGET:			/// 타겟 기준범위
		case eBOUNDTYPE_SELF_TARGET:	/// 시전자(자신) 기준범위 - 발동시 타겟 존재
			{

				/// 자신 일경우 거리체크 안함
				if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF	   ||
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF ||
					pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
				{
					/// 자신을 타겟으로 선정
					target = attacker;
					targetPos.x = pAttacker->GetXPos();
					targetPos.y = pAttacker->GetYPos();
					targetPos.z = pAttacker->Height();
					pTarget = pAttacker;
					break;
				}

				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pSkillInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "PlayerMonSkillRequest pStepInfo[%d,%d]->mTargetDist == 0", monsterClassIdx, attackType );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true  )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
				case eAPPLYTYPE_BUFF_ENEMY:
				case eAPPLYTYPE_DEBUFF_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type == eOBJECTTYPE_MONSTER )
						{
							unsigned long attackerTeamType = pAttacker->GetPvPDMTeam();
							if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
							{
								if( attackerTeamType == ((cMonster*)pTarget)->GetDMTeamType() )
									throw SKILL_USED_ERR_TARGETSELECT;
							}
						}
						else if( target.type == eOBJECTTYPE_PLAYER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_BUFF_PARTY:
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 자신은 무조건 가능
						if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
							break;

						bool isParty = false;

						/// 타겟이 파티원인지 체크
						cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
						if( pParty && pParty->IsUser( target.index ) )
						{
							isParty = true;
						}
						else
						{
							/// 파티연합인지 체크
							cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
							if( partyUnion && partyUnion->IsUser( pAttacker->GetObjectID(), target.index ) )
							{
								isParty = true;
							}
						}

						/// 타겟이 파티원이 아닌 경우
						if( isParty == false )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_DIEBUDDY:
					{
						/// 몬스터는 파티나 죽은동료를 설정하지 않는다.
						NETWORK2->PostServerEvent("PlayerMonSkillRequest mBoundType[%d,%d]", pMsg->mSkillClassIndex, pSkillInfo->mApplyType );
						throw SKILL_USED_ERR_SCRIPT;
					}
					break;
				}

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pSkillInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

				pAttacker->CalcDirection( targetPos.x, targetPos.y );

			}break;
			/// 필드인경우는 이함수에 들어올수 없음
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				/// 스크립트 내용이 잘못됐다.
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerMonSkillRequest mBoundType[%d,%d]", monsterClassIdx, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	


		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();


		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		if( pAttacker->IsChgMonCoolTimeEnd( attackType ) == false )
		{
			throw SKILL_USED_ERR_COOLTIME;
		}


		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "PlayerMonSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerMonSkillObject* pSkillObject = new cPlayerMonSkillObject;
		bool init = pSkillObject->InitPlayerMonAttack( characterIdx, pAttacker->GetChgMonsterIdx(), 
			attackType, target, skillUniqueIdx, casting, 
			pAttacker->GetStatus2()->mAttackSpeed,	pAttacker->GetMapNumber() );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			assert(NULL);
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "PlayerMonSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "PlayerMonSkillRequest VehicleAttack[%d,%d,%d]", characterIdx, pAttacker->GetChgMonsterIdx(), attackType );


		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 캐스팅을 하지 않는 스킬이면 스킬사용시 소모항목 감소
		if( casting == false )
		{
			/// MP를 소모 하는 스킬
			if( useMP != 0 )
				pAttacker->MPUse( useMP, false );

			/// 캐스팅이 아닌 경우 발동 체크 
			/// *** 캐스팅인 경우는 skillObj의 RequireValue() 에서 체크 ***
			if( attackType >= eMONSTERATTACK_SKILL1 )
			{
				switch( pSkillInfo->mType )
				{
				case eMONSTERSKILLTYPE_DAMAGE:
				case eMONSTERSKILLTYPE_MPDAMAGE:
					CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
					break;
				case eMONSTERSKILLTYPE_HEAL:
					CheckActInfluence( eACTSTATUS_HEALSKILL, pAttacker, pTarget );
					break;
				}

				/// 스킬 발동 효과
				if( pSkillInfo->mActiveIdx != 0 )
					SKILLMANAGER->CheckSkillActInfluence( pSkillInfo->mActiveIdx, pAttacker, pTarget );

			}
		}

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			if( casting == true )
				resMsg->Protocol = NM_SKILL_CAST_USED_MON_RES;
			else
				resMsg->Protocol = NM_SKILL_USED_MON_RES;

			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = pMsg->mSkillClassIndex;
			resMsg->mManagedKey      = pMsg->mManagedKey;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}


		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		/// 캐스팅 여부에 따른 메세지 내용 정의
		if( casting == true )
			synMsg.Protocol = NM_SKILL_CAST_USED_SYN;
		else
			synMsg.Protocol = NM_SKILL_USED_SYN;
		synMsg.mSkillClassIndex = attackType;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = targetPos.x;
		synMsg.mTargetPosY = targetPos.y;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->MonSkillSuccessCntPlus();
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			if( casting == true )
				resMsg->Protocol = NM_SKILL_CAST_USED_MON_RES;
			else
				resMsg->Protocol = NM_SKILL_USED_MON_RES;

			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = attackType;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::PlayerMonPosSkillRequest( unsigned long characterIdx, MSG_REQ_POS_USED* pMsg, bool casting )
{

	eMONSTERATTACK_TYPE attackType = (eMONSTERATTACK_TYPE)pMsg->mSkillClassIndex;

	/// 이함수를 통해 들어오는 공격자의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	float targetXPos = pMsg->mTargetPosX;
	float targetYPos = pMsg->mTargetPosY;
	unsigned long clientIdx = pMsg->mManagedKey;


	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerPositionSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MonSkillReqCntPlus();
	pAttacker->MoveStop();

	pAttacker->CalcDirection( targetXPos, targetYPos );

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 몬스터 변신중이 아닌데 몬스터 공격이 들어온 경우 - 메세지 시간차로 인해 들어올수 있음
		unsigned long monsterClassIdx = pAttacker->GetChgMonsterIdx();
		if( monsterClassIdx == 0 )
			throw SKILL_USED_ERR_SKILLTYPE;

		if( attackType >= eMONSTERATTACK_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerMonSkillRequest - attackType[%d] > eMONSTERATTACK_MAX", attackType );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬index 체크
		sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, attackType );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonSkillRequest skillScript[%d,%d]", monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용 불가
		if( attackType > eMONSTERATTACK_NORMAL2 )
		{
			/// 기본 공격은 제외하고 처리한다.
			if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
				throw SKILL_USED_ERR_USEVALUE;
		}

		if( pAttacker->GetChgMonsterIdx() == 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
		{
			pAttacker->MonSkillCoolTimeFailPlus();
			throw SKILL_USED_ERR_USING;
		}


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( eOBJECTTYPE_NONE, 0 );


		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos( pAttacker->GetXPos(), pAttacker->GetYPos() );

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker     = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget.type  = eOBJECTTYPE_NONE;
				posMsg->mTargetPosX   = 0.0f;
				posMsg->mTargetPosY   = 0.0f;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}


		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING && casting != true )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonPosSkillRequest[%d] ShotType != eSHOTTYPE_CASTING[%d]", characterIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}


		if( pSkillInfo->mShotType != eSHOTTYPE_CASTING && casting == true )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonPosSkillRequest[%d] ShotType == eSHOTTYPE_CASTING[%d]", characterIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}


		/// 소모 MP 체크
		unsigned long useMP = pSkillInfo->mUseMP;
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( pSkillInfo->mUseMP );
		}

		/// 소모 MP
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
			/// 필드인경우만 이함수에 들어옴
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
			{
				NiPoint3 targetPos( targetXPos, targetYPos, 0.0f );
				AIMANAGER->CalcHeight( pAttacker->GetMapNumber(), &targetPos.z, targetXPos, targetYPos );

				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pSkillInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, NULL, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos();
						posMsg->mAttackerPosY = pAttacker->GetYPos();
						posMsg->mTarget.type  = eOBJECTTYPE_NONE;
						posMsg->mTargetPosX   = 0.0f;
						posMsg->mTargetPosY   = 0.0f;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

			}break;
		default:
			{
				/// 스크립트 내용이 잘못됐다.
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerMonPosSkillRequest pSkillInfo->mBoundType[%d]", pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		if( pAttacker->IsChgMonCoolTimeEnd( attackType ) == false )
		{
			throw SKILL_USED_ERR_COOLTIME;
		}


		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "PlayerMonPosSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerMonSkillObject* pSkillObject = new cPlayerMonSkillObject;
		bool init = pSkillObject->InitPlayerMonPosition( characterIdx, monsterClassIdx, attackType, targetXPos, targetYPos,
			skillUniqueIdx, casting, pAttacker->GetMapNumber() );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			assert(NULL);
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "PlayerMonPosSkillRequest new SkillObject" );
			throw SKILL_USED_ERR_OBJECTINIT;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "PlayerMonPosSkillRequest VehicleAttack[%d,%d,%d]", characterIdx, monsterClassIdx, attackType );


		/// 스킬 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 캐스팅을 하지 않는 스킬이면 스킬사용시 소모항목 감소
		if( casting == false )
		{
			/// MP를 소모 하는 스킬
			if( useMP != 0 )
				pAttacker->MPUse( useMP, false );
		}

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL );

		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			if( casting == true )
				resMsg->Protocol = NM_SKILL_POS_CAST_USED_MON_RES;
			else
				resMsg->Protocol = NM_SKILL_POS_USED_MON_RES;

			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = attackType;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_POS_USED synMsg;
		synMsg.Category = NM_SKILL;
		/// 캐스팅 여부에 따른 메세지 내용 정의
		if( casting == true )
			synMsg.Protocol = NM_SKILL_POS_CAST_USED_MON_SYN;
		else
			synMsg.Protocol = NM_SKILL_POS_USED_MON_SYN;
		synMsg.mSkillClassIndex = attackType;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTargetPosX = targetXPos;
		synMsg.mTargetPosY = targetYPos;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->SkillSuccessCntPlus();
	}
	catch( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL );
		if ( resMsg != NULL )
		{
			if( casting == true )
				resMsg->Protocol = NM_SKILL_POS_CAST_USED_MON_RES;
			else
				resMsg->Protocol = NM_SKILL_POS_USED_MON_RES;

			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = attackType;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::MapChangeSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_ITEMMAPCHANGE* pMsg )
{
	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::MapChangeSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		return; 
	}

	pAttacker->MoveStop();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		/// 상태 체크
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		if( pAttacker->IsRequestRejection() == true )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			NETWORK2->PostServerEvent("cSkillManager::MapChangeSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}
		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
		if( pStepInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::MapChangeSkillRequest pStepInfo == NULL [%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// inventory 검증..
		long useItemDBIdx = pAttacker->IsItemMapChange( pMsg->mSlotNumber, pSkillInfo->mUseItem, false, skillIdx );
		if( useItemDBIdx == 0 )
			throw SKILL_USED_ERR_USEVALUE;

		/// 맵이동 가능 체크
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_MAPCHANGE )
		{
			if( pAttacker->IsItemMapChange( pStepInfo->mApplyValue1 ) == false )
				throw SKILL_USED_ERR_MAPCHANGE_STATE;
		}
		else if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_MAPCHANGE )
		{
			if( pAttacker->IsItemMapChange( pStepInfo->mApplyValue2 ) == false )
				throw SKILL_USED_ERR_MAPCHANGE_STATE;
		}
		else
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			NETWORK2->PostServerEvent( "cSkillManager::MapChangeSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitPlayerMapChange( skillIdx, skillUniqueIdx, pAttacker, useItemDBIdx, pStepInfo->mCastingTime );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "cSkillManager::MapChangeSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "MapChangeSkillRequest VehicleAttack[%d,%d]", characterIdx, skillIdx  );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_ITEMMAPCHANGE_RES );
		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_ITEMMAPCHANGE_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = pAttacker->GetXPos();
		synMsg.mTargetPosY = pAttacker->GetYPos();
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_ITEMMAPCHANGE_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::VehiCleSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_VEHICLE* pMsg )
{
	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::VehicleSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		return; 
	}

	pAttacker->MoveStop();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		/// 상태 체크
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			NETWORK2->PostServerEvent("cSkillManager::VehicleSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}
		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
		if( pStepInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::VehiCleSkillRequest pStepInfo == NULL [%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// inventory 검증..
		long useItemDBIdx = pAttacker->IsItemVehicle( pMsg->mSlotNumber, pSkillInfo->mUseItem, skillIdx );
		if( useItemDBIdx == 0 )
			throw SKILL_USED_ERR_USEVALUE;

		/// 몬스터 변신 상태이면 사용 불가
		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 
		sInfluenceScript* pInf = SKILLSCRIPT->GetInfluenceInfo( pStepInfo->mInfulenceIdx );
		if( pInf == 0 )
		{
			throw SKILL_USED_ERR_ATTACKERSTATE;
		}

		if( pInf->mStatusPlusIdx1 != eSTATUSPLUS_VEHICLE &&
			pInf->mStatusPlusIdx2 != eSTATUSPLUS_VEHICLE &&
			pInf->mStatusPlusIdx3 != eSTATUSPLUS_VEHICLE )
		{
			throw SKILL_USED_ERR_ATTACKERSTATE;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			NETWORK2->PostServerEvent( "cSkillManager::VehicleSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitPlayerVehicle( skillIdx, skillUniqueIdx, pAttacker, pSkillInfo->mUseItem, pStepInfo->mCastingTime );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "cSkillManager::VehicleSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "VehicleSkillRequest VehicleAttack[%d,%d]", characterIdx, skillIdx  );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_ITEMVEHICLE_RES );

		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			resMsg->ErrorCode        = SKILL_USED_ERR_SUCCESS;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_ITEMVEHICLE_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = pAttacker->GetXPos();
		synMsg.mTargetPosY = pAttacker->GetYPos();
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_ITEMVEHICLE_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::CashMapChangeSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_CASHMAPCHANGE* pMsg )
{
	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned short targetMapNum = pMsg->mMapNumber;
	float targetX = pMsg->mPosX;	
	float targetY = pMsg->mPosY;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::CashMapChangeSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		return; 
	}

	pAttacker->MoveStop();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		/// 상태 체크
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		if( pAttacker->IsRequestRejection() == true )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			NETWORK2->PostServerEvent("cSkillManager::CashMapChangeSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}
		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
		if( pStepInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::CashMapChangeSkillRequest pStepInfo == NULL [%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// inventory 검증..
		long useItemDBIdx = pAttacker->IsItemMapChange( pMsg->mSlotNumber, pSkillInfo->mUseItem, true, skillIdx );
		if( useItemDBIdx == 0 )
			throw SKILL_USED_ERR_USEVALUE;

		/// 맵이동 가능 체크
		if( !( pStepInfo->mApplyValueType1 == eSTATUSPLUS_CASHMAPCHANGE || pStepInfo->mApplyValueType2 == eSTATUSPLUS_CASHMAPCHANGE ) )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		if( pAttacker->IsAutoTrial() == true )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		if( targetMapNum < MAP_MIN || MAP_MAX < targetMapNum )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;

		if( NETWORK2->GetServerType() == _E_ST_ID_PVP_ || NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
			throw SKILL_USED_ERR_MAPCHANGE_STATE;
		else if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
		{
			if( AIMANAGER->IsPossible( targetMapNum, targetX, targetY, attacker ) == false )
				throw SKILL_USED_ERR_MAPCHANGE_STATE;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			NETWORK2->PostServerEvent( "cSkillManager::CashMapChangeSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitCashMapChange( skillIdx, skillUniqueIdx, pAttacker, pStepInfo->mCastingTime,
			pMsg->mMapNumber, pMsg->mPosX, pMsg->mPosY, useItemDBIdx );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "cSkillManager::CashMapChangeSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "cSkillManager::CashMapChangeSkillRequest VehicleAttack[%d,%d]", characterIdx, skillIdx  );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_CASHMAPCHANGE_RES );
		if ( resMsg != NULL )
		{
			/// 캐스팅 여부에 따른 메세지 내용 정의
			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_CASHMAPCHANGE_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = pAttacker->GetXPos();
		synMsg.mTargetPosY = pAttacker->GetYPos();
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL, NM_SKILL_CASHMAPCHANGE_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}


void cSkillManager::BlinkSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_BLINK* pMsg )
{
	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::BlinkSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		return; 
	}

	pAttacker->MoveStop();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		/// 상태 체크
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			NETWORK2->PostServerEvent("cSkillManager::BlinkSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}
		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
		if( pStepInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::BlinkSkillRequest pStepInfo == NULL [%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 순간이동 스킬만 쓸수 있다
		if( pSkillInfo->mShotType != eSHOTTYPE_BLINK && pSkillInfo->mShotType != eSHOTTYPE_BLINK_BACK )
			throw SKILL_USED_ERR_SCRIPT;

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{

			if( pAttacker->GetMaxHP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 1");
				throw SKILL_USED_ERR_USEVALUE;
			}

			if( pAttacker->GetMaxMP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 2");
				throw SKILL_USED_ERR_USEVALUE;
			}

			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerObjectSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}
	

		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;

		/// 소모 HP 수치 체크
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			NETWORK2->PostServerEvent( "cSkillManager::BlinkSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 목적지점
		NiPoint2 targetPos( pMsg->mTargetPosX, pMsg->mTargetPosY );

		/// 목적지까지의 방향정보
		NiPoint2 dir = targetPos - pAttacker->GetPos();
		float range = dir.Unitize();

		/// 이동제한 거리 체크
		if( pStepInfo->mTargetDist + SYNC_MOVE_RANGE < range )
		{
			/// 범위를 벗어나는 경우면 허용범위 안까지만 이동 시킨다.
			range = pStepInfo->mTargetDist + SYNC_MOVE_RANGE;
			targetPos = pAttacker->GetPos() + ( dir * range );
		}

		/// 목적지점까지 이동이 가능한지 체크 - 불가능 할경우 거리를 쫍혀간다.
		if( pAttacker->IsMovePossible( pAttacker->GetPos(), targetPos ) != 0 )
		{
			/// 분할 숫자
			unsigned long divideCnt = 10;
			targetPos = pAttacker->GetPos();

			/// 이동거리를 분할
			float divideRange = range / (float)divideCnt;
			for( unsigned long i = 1 ; i < divideCnt ; ++i )	/// 양끝점을 제외하고 분할된 위치만큼 가능체크
			{
				/// 목적지에서 분할한 거리만큼 빼가면서 이동가능한지 체크후 가능하면 그위치로 이동한다.
				NiPoint2 pos = pAttacker->GetPos() + ( dir * ( range - ( divideRange * i ) ) );
				if( pAttacker->IsMovePossible( pAttacker->GetPos(), pos ) == 0 )
				{
					targetPos = pos;
					break;
				}
			}
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitPlayerBlink( skillIdx, skillUniqueIdx, pAttacker, targetPos );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "cSkillManager::BlinkSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "BlinkSkillRequest VehicleAttack[%d,%d]", characterIdx, skillIdx  );


		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );

		/// HP를 소모 하는 스킬
		if( pStepInfo->mUseHP != 0 )
		{
			/// HP 소모 & 메세지 발송
			pAttacker->HPUse( pStepInfo->mUseHP, true );
		}

		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
			}
		}

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_BLINK* resMsg = (MSG_RES_SKILL_BLINK*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(),
			NM_SKILL, NM_SKILL_BLINK_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = SKILL_USED_ERR_SUCCESS;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();
			resMsg->mTargetPosX      = targetPos.x;
			resMsg->mTargetPosY      = targetPos.y;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_BLINK) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_BLINK synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_BLINK_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mTargetPosX = targetPos.x;
		synMsg.mTargetPosY = targetPos.y;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_BLINK* resMsg = (MSG_RES_SKILL_BLINK*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(),
			NM_SKILL, NM_SKILL_BLINK_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mTargetPosX = pAttacker->GetXPos();
			resMsg->mTargetPosY = pAttacker->GetYPos();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_BLINK) );
		}
	}
}


void cSkillManager::PlayerNonStopSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg )
{

	/// 이함수를 통해 들어오는 공격자의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;

	/// 서버에는 hero가 없다aa
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	mSendMsgSet.Clear();

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerNonStopSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendMsgSet.Insert( pPlayer->GetConnectionIdx() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 기본 공격 스킬 인덱스인경우 오류
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 스킬 사용 불가
		if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
			throw SKILL_USED_ERR_USEVALUE;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 이미 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( eOBJECTTYPE_NONE, 0 );


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}


		/// 액티브 타임도 없는데 케스팅할일은 없다.
		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest[%d,%d] ShotType == eSHOTTYPE_CASTING", attacker.index, pMsg->mSkillClassIndex );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 공격자의 보유스킬인지 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( attacker.index, skillIdx );
		if( !pPlayerHaveSkill )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest GetPlayerHaveSkill[%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_DONTHAVE;
		}


		/// 스킬 스크립트 스텝정보 읽어옴
		unsigned int stepIdx = pPlayerHaveSkill->GetSkillStep();
		if( stepIdx < skillStep )
		{
			NETWORK2->PostServerEvent("cSkillManager::PlayerNonStopSkillRequest stepIdx >= pSkillInfo->mStepCount [%d,%d, (%d > %d)]", attacker.index, skillIdx, stepIdx, pSkillInfo->mStepCount );
			throw SKILL_USED_ERR_SCRIPT;
		}

		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];
		if( pStepInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerNonStopSkillRequest pStepInfo == NULL [%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pAttacker->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerNonStopSkillRequest IsEquipment == false" );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{
			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerNonStopSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}


		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모 HP
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}


		NiPoint3 targetPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
		cBaseObject* pTarget = NULL;

		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_SELF_NOTARGET:			/// 자신이 범위 시작위치여서 거리체크 안함	
			{
				/// 자신을 타겟으로 선정
				target = attacker;
				targetPos.x = pAttacker->GetXPos();
				targetPos.y = pAttacker->GetYPos();
				targetPos.z = pAttacker->Height();
				pTarget = pAttacker;
			}
			break;
		case eBOUNDTYPE_NONE:					/// 범위 없음
		case eBOUNDTYPE_TARGET:					/// 타겟 기준범위
		case eBOUNDTYPE_SELF_TARGET:			/// 시전자(자신) 기준범위 - 발동시 타겟 존재
			{

				/// 자신 일경우 거리체크 안함
				if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF	   ||
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF ||
					pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
				{
					/// 자신을 타겟으로 선정
					target = attacker;
					targetPos.x = pAttacker->GetXPos();
					targetPos.y = pAttacker->GetYPos();
					targetPos.z = pAttacker->Height();
					pTarget = pAttacker;
					break;
				}

				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pStepInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "PlayerNonStopSkillRequest[%d] mTargetDist == 0", skillIdx );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
				case eAPPLYTYPE_BUFF_ENEMY:
				case eAPPLYTYPE_DEBUFF_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type != eOBJECTTYPE_MONSTER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_BUFF_PARTY:
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 자신은 무조건 가능
						if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
							break;

						bool isParty = false;

						/// 타겟이 파티원인지 체크
						cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
						if( pParty && pParty->IsUser( target.index ) )
						{
							isParty = true;
						}
						else
						{
							/// 파티연합인지 체크
							cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
							if( partyUnion && partyUnion->IsUser( pAttacker->GetObjectID(), target.index ) )
							{
								isParty = true;
							}
						}

						/// 타겟이 파티원이 아닌 경우
						if( isParty == false )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				}

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pStepInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

			}break;
			/// 논스탑으로 필드를 지정하면서 설정을 하진 않는다. - 스크립트오류
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerNonStopSkillRequest mBoundType[%d,%d]", skillIdx, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
		if( pSkillInfo->mApplyValueType != eSTATUSPLUS_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_BUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_DEBUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_BUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_DEBUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_BUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;

				}
				break;
			case eAPPLYTYPE_DEBUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;

			}//end switch( pSkillInfo->mApplyType )
		}

		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		unsigned long currentTime = NETWORK2->GetAccumTime();

		/// 쿨타임
		if( pStepInfo->mCoolTime != 0 && pPlayerHaveSkill->GetEndCoolTime() > currentTime )	
		{ 
			pAttacker->SkillCoolTimeFailPlus();

			throw SKILL_USED_ERR_COOLTIME;
		}

		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );

		/// HP를 소모 하는 스킬
		if( pStepInfo->mUseHP != 0 )
		{
			/// HP 소모 & 메세지 발송
			pAttacker->HPUse( pStepInfo->mUseHP, true );
		}


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
			}
		}

		/// 쿨타임 갱신
		SKILLMANAGER->UpdateSkillCoolTime( attacker, pMsg->mSkillClassIndex, (unsigned char)stepIdx, NETWORK2->GetAccumTime(), 1.0f );

		unsigned char targetMaxCnt = pStepInfo->mApplyCnt;
		if( targetMaxCnt == 0 || targetMaxCnt > TARGET_CNT_MAX )
			targetMaxCnt = TARGET_CNT_MAX;
		else
		{
			/// 적용 인원이 제한된 경우 공격자가 범위 스킬 적용 인원 증가 효과에 걸려있는지 체크 하여 넣기
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				long addApplyCnt = (long)pAttacker->GetStateOddity( eODDITYTYPE_ADD_APPLY_CNT );
				if( addApplyCnt > 0 )
				{
					if( (addApplyCnt + targetMaxCnt) > UCHAR_MAX )
					{
						assert(0);
						targetMaxCnt = TARGET_CNT_MAX;
					}
					else
					{
						targetMaxCnt = targetMaxCnt + (unsigned char)addApplyCnt;
						if ( targetMaxCnt > TARGET_CNT_MAX )
						{
							assert(0);
							targetMaxCnt = TARGET_CNT_MAX;
						}
					}
				}
			}
		}

		HANDLE              handle = NULL;
		MSG_RES_SKILL_NONSTOP* resMsg = (MSG_RES_SKILL_NONSTOP*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, NM_SKILL_NONSTOP_RES );
		if ( resMsg == NULL )
			throw SKILL_USED_ERR_OBJECTINIT;

		resMsg->mSkillClassIndex = pMsg->mSkillClassIndex;
		resMsg->mManagedKey = pMsg->mManagedKey;
		resMsg->mObjectMP = pAttacker->GetMP();
		resMsg->mMaxMP = pAttacker->GetMaxMP();
		resMsg->mCount = 0;

		/// 타겟 선정
		if( pSkillInfo->mBoundType != eBOUNDTYPE_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_SELF:
			case eAPPLYTYPE_BUFF_SELF:
			case eAPPLYTYPE_DEBUFF_SELF:
				{
					bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pAttacker->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
					resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
					resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
					++resMsg->mCount;
				}
				break;
			case eAPPLYTYPE_BUDDY:
				{
					tArray<unsigned long> objectIdxAry;

					if( pStepInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pStepInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = targetPos.x;
						centerPos.y = targetPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}
				}
				break;
			case eAPPLYTYPE_PARTY:
			case eAPPLYTYPE_BUFF_PARTY:
			case eAPPLYTYPE_DEBUFF_PARTY:
				{
					unsigned long* userArr = NULL;
					unsigned int partyCount = 0;
					NiPoint2 attackerPos = pAttacker->GetPos();

					/// 파티원 검색
					cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
					if( pParty != NULL )
					{
						userArr = pParty->GetUserArr();
						partyCount = pParty->GetCount();
					}
					else
					{
						cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
						if( partyUnion != NULL )
						{
							userArr = partyUnion->GetGroupArr( pAttacker->GetObjectID() );
							partyCount = MAX_PARTYUSER;
						}
					}

					/// 파티나 유니온이 있는 경우
					if( userArr != NULL )
					{
						for( unsigned int i = 0 ; i < partyCount ; ++i )
						{
							/// 파티가 없는 경우 사용시를 처리하기위해 자신은 위에서 먼저 넣었음
							if( pAttacker->GetObjectID() == userArr[i] )
								continue;

							/// 타겟 객체 확인
							cPlayer* partyUser = OBJECTMANAGER->GetPlayer( userArr[i] );
							if( partyUser == NULL )
								continue;

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, partyUser, pStepInfo->mBoundDist1 );
							/// 거리 확인
							mRangeCheck.SetRadius( tempRange );
							if( mRangeCheck.IsRange( attackerPos, partyUser->GetPos() ) == true )
							{
								/// 타겟에 추가
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pAttacker->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								/// 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
								cPlayer* partyUser = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
								while( partyUser != NULL )
								{
									mSendMsgSet.Insert( partyUser->GetConnectionIdx() );
									partyUser = GRIDMANAGER->FindNextPlayer( true );
								}

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
				}
				break;
			case eAPPLYTYPE_ENEMY:
			case eAPPLYTYPE_BUFF_ENEMY:
			case eAPPLYTYPE_DEBUFF_ENEMY:
				{
					tArray<unsigned long> objectIdxAry;

					if( pStepInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( pCheckTarget->IsAttack( pAttacker->GetObjectID() ) == false )
									continue;
							}

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pStepInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = centerPos.x;
						centerPos.y = centerPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( pCheckTarget->IsAttack( pAttacker->GetObjectID() ) == false )
									continue;
							}

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}

					if( pStepInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindMonsters( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cMonster* pCheckTarget = OBJECTMANAGER->GetMonster( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pStepInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = centerPos.x;
						centerPos.y = centerPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( -pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( -pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pStepInfo->mBoundDist1 * 0.5f ) * cos + ( +pStepInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pStepInfo->mBoundDist2 * 0.5f ) * cos - ( +pStepInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindMonsters( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cMonster* pCheckTarget = OBJECTMANAGER->GetMonster( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}
				}
				break;
			}
		}
		else
		{
			bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pAttacker->GetObject(), pStepInfo->mInfulenceIdx, skillIdx, true );
			resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
			resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
			++resMsg->mCount;
		}

		/// 공격자는 메세지를 따로 보내니깐지운다.
		mSendMsgSet.Erase( pAttacker->GetConnectionIdx() );

		for( unsigned char j = 0 ; j < resMsg->mCount ; ++j )
		{
			if( resMsg->mTarget[j].mTarget.type != eOBJECTTYPE_PLAYER )
				continue;

			cPlayer* pTarget = OBJECTMANAGER->GetPlayer( resMsg->mTarget[j].mTarget.index );
			if( pTarget == NULL )
				continue;

			/// 타겟은 메세지를 따로보내니깐 지운다.
			mSendMsgSet.Erase( pTarget->GetConnectionIdx() );

			if( attacker.index == resMsg->mTarget[j].mTarget.index )
				continue;

			HANDLE              handle = NULL;
			MSG_SYN_SKILL_NONSTOP_TARGET* targetMsg = (MSG_SYN_SKILL_NONSTOP_TARGET*)NETWORK2->GetMsgRoot( &handle, pTarget->GetConnectionIdx(), 
				NM_SKILL, NM_SKILL_NONSTOP_TARGET_SYN );
			if ( resMsg != NULL )
			{
				targetMsg->mSkillClassIndex = resMsg->mSkillClassIndex;
				targetMsg->mAttacker = attacker;
				targetMsg->mObjectMP = pAttacker->GetMP();
				targetMsg->mMaxMP = pAttacker->GetMaxMP();
				targetMsg->mTargetApply.mTarget = resMsg->mTarget[j].mTarget;
				targetMsg->mTargetApply.mInfluenceMiss = resMsg->mTarget[j].mInfluenceMiss;
			}

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKILL_NONSTOP_TARGET) );
		}

		/// 주변 케릭터들에게는 목록만 발송한다.
		MSG_SYN_SKILL_NONSTOP_AROUND aroundMsg;
		aroundMsg.Category = NM_SKILL;
		aroundMsg.Protocol = NM_SKILL_NONSTOP_AROUND_SYN;
		aroundMsg.mSkillClassIndex = pMsg->mSkillClassIndex; 
		aroundMsg.mAttacker = attacker;
		aroundMsg.mObjectMP = pAttacker->GetMP();
		aroundMsg.mMaxMP = pAttacker->GetMaxMP();

		mSendMsgSet.Erase( attacker.index );
		cHashSet::cIterator sB = mSendMsgSet.Begin();
		cHashSet::cIterator sE = mSendMsgSet.End();
		for( ; sB != sE ; ++sB )
		{
			NETWORK2->SendToUser( *sB, (char*)&aroundMsg, sizeof(MSG_SYN_SKILL_NONSTOP_AROUND) );
		}

		NETWORK2->SendMsgRoot( handle, resMsg->GetLength() );
	}
	catch( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_NONSTOP* resMsg = (MSG_RES_SKILL_NONSTOP*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, NM_SKILL_NONSTOP_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_NONSTOP) );
		}
	}
}


void cSkillManager::PlayerMonNonStopSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg )
{

	eMONSTERATTACK_TYPE attackType = (eMONSTERATTACK_TYPE)pMsg->mSkillClassIndex;

	/// 이함수를 통해 들어오는 공격자의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long clientIdx = pMsg->mManagedKey;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	mSendMsgSet.Clear();

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendMsgSet.Insert( pPlayer->GetConnectionIdx() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 몬스터 변신중이 아닌데 몬스터 공격이 들어온 경우 - 메세지 시간차로 인해 들어올수 있음
		unsigned long monsterClassIdx = pAttacker->GetChgMonsterIdx();
		if( monsterClassIdx == 0 )
			throw SKILL_USED_ERR_SKILLTYPE;

		if( attackType >= eMONSTERATTACK_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest - attackType[%d] > eMONSTERATTACK_MAX", attackType );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬index 체크
		sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, attackType );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest skillScript[%d,%d]", monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}


		/// 스킬 사용 불가
		if( attackType > eMONSTERATTACK_NORMAL2 )
		{
			/// 기본 공격은 제외하고 처리한다.
			if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
				throw SKILL_USED_ERR_USEVALUE;
		}


		if( pAttacker->GetChgMonsterIdx() == 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( eOBJECTTYPE_NONE, 0 );


		/// 액티브 타임도 없는데 케스팅할일은 없다.
		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest[%d,%d] ShotType == eSHOTTYPE_CASTING[%d]", characterIdx, monsterClassIdx, attackType );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 소모 MP 체크
		unsigned long useMP = pSkillInfo->mUseMP;
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( pSkillInfo->mUseMP );
		}

		/// 소모 MP
		if( useMP > pAttacker->GetMP() )
 			throw SKILL_USED_ERR_USEVALUE;


		NiPoint3 targetPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
		cBaseObject* pTarget = NULL;

		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_SELF_NOTARGET:			/// 자신이 범위 시작위치여서 거리체크 안함	
			{
				/// 자신을 타겟으로 선정
				target = attacker;
				targetPos.x = pAttacker->GetXPos();
				targetPos.y = pAttacker->GetYPos();
				targetPos.z = pAttacker->Height();
				pTarget = pAttacker;
			}
			break;
		case eBOUNDTYPE_NONE:					/// 범위 없음
		case eBOUNDTYPE_TARGET:					/// 타겟 기준범위
		case eBOUNDTYPE_SELF_TARGET:			/// 시전자(자신) 기준범위 - 발동시 타겟 존재
			{

				/// 자신 일경우 거리체크 안함
				if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF ||
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF ||
					pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
				{
					/// 자신을 타겟으로 선정
					target = attacker;
					targetPos.x = pAttacker->GetXPos();
					targetPos.y = pAttacker->GetYPos();
					targetPos.z = pAttacker->Height();
					pTarget = pAttacker;
					break;
				}

				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pSkillInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "PlayerMonNonStopSkillRequest[%d] mTargetDist == 0", pMsg->mSkillClassIndex );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type != eOBJECTTYPE_MONSTER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_DIEBUDDY:
				case eAPPLYTYPE_BUFF_SELF:  // 현재는 몬스터 변신 스킬상에서는 사용하고 있지 않다 [4/6/2010 Jo_Client]
				case eAPPLYTYPE_DEBUFF_SELF:		
				case eAPPLYTYPE_BUFF_ENEMY:		
				case eAPPLYTYPE_DEBUFF_ENEMY:	
				case eAPPLYTYPE_BUFF_PARTY:		
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 몬스터는 파티나 죽은동료를 설정하지 않는다.
						NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest mBoundType[%d,%d]", pMsg->mSkillClassIndex, pSkillInfo->mApplyType );
						throw SKILL_USED_ERR_SCRIPT;
					}
					break;
				}

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pSkillInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

			}break;
			/// 논스탑으로 필드를 지정하면서 설정을 하진 않는다. - 스크립트오류
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerMonNonStopSkillRequest mBoundType[%d,%d]", pMsg->mSkillClassIndex, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();


		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		if( pAttacker->IsChgMonCoolTimeEnd( attackType ) == false )
		{
			throw SKILL_USED_ERR_COOLTIME;
		}

 		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );

		/// 캐스팅이 아닌 경우 발동 체크 
		/// *** 캐스팅인 경우는 skillObj의 RequireValue() 에서 체크 ***
		switch( pSkillInfo->mType )
		{
		case eMONSTERSKILLTYPE_DAMAGE:
		case eMONSTERSKILLTYPE_MPDAMAGE:
			CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
			break;
		case eMONSTERSKILLTYPE_HEAL:
			CheckActInfluence( eACTSTATUS_HEALSKILL, pAttacker, pTarget );
			break;
		}

		/// 스킬 발동 효과
		if( pSkillInfo->mActiveIdx != 0 )
			SKILLMANAGER->CheckSkillActInfluence( pSkillInfo->mActiveIdx, pAttacker, pTarget );

		/// 쿨타임 갱신
		pAttacker->UpdateChgMonCoolTime( attackType );

		unsigned char targetMaxCnt = pSkillInfo->mApplyCnt;
		if( targetMaxCnt == 0 || targetMaxCnt > TARGET_CNT_MAX )
			targetMaxCnt = TARGET_CNT_MAX;
		else
		{
			/// 적용 인원이 제한된 경우 공격자가 범위 스킬 적용 인원 증가 효과에 걸려있는지 체크 하여 넣기
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				long addApplyCnt = (long)pAttacker->GetStateOddity( eODDITYTYPE_ADD_APPLY_CNT );
				if( addApplyCnt > 0 )
				{
					if( (addApplyCnt + targetMaxCnt) > UCHAR_MAX )
					{
						assert(0);
						targetMaxCnt = TARGET_CNT_MAX;
					}
					else
					{
						targetMaxCnt = targetMaxCnt + (unsigned char)addApplyCnt;
						if ( targetMaxCnt > TARGET_CNT_MAX )
						{
							assert(0);
							targetMaxCnt = TARGET_CNT_MAX;
						}
					}
				}
			}
		}
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_NONSTOP* resMsg = (MSG_RES_SKILL_NONSTOP*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, NM_SKILL_NONSTOP_MON_RES );
		if ( resMsg == NULL )
			throw SKILL_USED_ERR_OBJECTINIT;

		resMsg->mSkillClassIndex = pMsg->mSkillClassIndex;
		resMsg->mManagedKey = pMsg->mManagedKey;
		resMsg->mObjectMP = pAttacker->GetMP();
		resMsg->mMaxMP = pAttacker->GetMaxMP();
		resMsg->mCount = 0;

		/// 타겟 선정
		if( pSkillInfo->mBoundType != eBOUNDTYPE_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_SELF:
				{
					bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pAttacker->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
					resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
					resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
					++resMsg->mCount;
				}
				break;
			case eAPPLYTYPE_BUDDY:
				{
					tArray<unsigned long> objectIdxAry;

					if( pSkillInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pSkillInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = targetPos.x;
						centerPos.y = targetPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}
				}
				break;
			case eAPPLYTYPE_ENEMY:
				{
					tArray<unsigned long> objectIdxAry;

					if( pSkillInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( pCheckTarget->IsAttack( pAttacker->GetObjectID() ) == false )
									continue;
							}

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pSkillInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = centerPos.x;
						centerPos.y = centerPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindPlayers( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cPlayer* pCheckTarget = OBJECTMANAGER->GetPlayer( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							mSendMsgSet.Insert( pCheckTarget->GetConnectionIdx() );

							if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( pCheckTarget->IsAttack( pAttacker->GetObjectID() ) == false )
									continue;
							}

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}

					if( pSkillInfo->mBoundDist2 == 0 )
					{
						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindMonsters( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cMonster* pCheckTarget = OBJECTMANAGER->GetMonster( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							float tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, pCheckTarget, pSkillInfo->mBoundDist1 );
							mRangeCheck.SetRadius( tempRange );

							NiPoint3 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();
							targetPoint.z = pCheckTarget->Height();

							/// 플레이어가 범위 안에 들어왔는지 체크
							if ( mRangeCheck.IsRange( targetPos, targetPoint ) )
							{
								bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
								resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
								resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
								++resMsg->mCount;

								if( resMsg->mCount >= targetMaxCnt )
									break;
							}
						}
					}
					else
					{
						NiPoint2 centerPos;
						centerPos.x = centerPos.x;
						centerPos.y = centerPos.y;

						float direction = pAttacker->GetDirection();

						float cos = cosf( direction );
						float sin = sinf( direction );

						/// 사각범위 위치 계산
						NiPoint2 rectPos[4];
						rectPos[0].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[0].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[1].x = ( ( -pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[1].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( -pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[2].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( -pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[2].y = ( ( -pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );
						rectPos[3].x = ( ( +pSkillInfo->mBoundDist1 * 0.5f ) * cos + ( +pSkillInfo->mBoundDist2 * 0.5f ) * sin );
						rectPos[3].y = ( ( +pSkillInfo->mBoundDist2 * 0.5f ) * cos - ( +pSkillInfo->mBoundDist1 * 0.5f ) * sin );

						rectPos[0] = rectPos[0] + centerPos;
						rectPos[1] = rectPos[1] + centerPos;
						rectPos[2] = rectPos[2] + centerPos;
						rectPos[3] = rectPos[3] + centerPos;

						/// 중심위치로 검색하도록 바꿔야 한다
						GRIDMANAGER->FindMonsters( pTarget, &objectIdxAry );
						for( unsigned long i = 0 ; i < objectIdxAry.GetSize() ; ++i )
						{
							cMonster* pCheckTarget = OBJECTMANAGER->GetMonster( objectIdxAry[i] );
							if( pCheckTarget == NULL || pCheckTarget->GetStateDie() == true )
								continue;

							NiPoint2 targetPoint;
							targetPoint.x = pCheckTarget->GetXPos();
							targetPoint.y = pCheckTarget->GetYPos();

							if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
								continue;

							bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pCheckTarget->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
							resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
							resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
							++resMsg->mCount;

							if( resMsg->mCount >= targetMaxCnt )
								break;
						}
					}
				}
				break;
				/* ///현재는 몬스터 변신 스킬상에서는 사용하고 있지 않다.
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:		
				case eAPPLYTYPE_BUFF_ENEMY:		
				case eAPPLYTYPE_DEBUFF_ENEMY:	
				case eAPPLYTYPE_BUFF_PARTY:		
				case eAPPLYTYPE_DEBUFF_PARTY:
				*/
			}
		}
		else
		{
			bool isInf = SKILLMANAGER->AddInfluence( pAttacker->GetObject(), pAttacker->GetObject(), pSkillInfo->mInfulenceIdx, pMsg->mSkillClassIndex, true );
			resMsg->mTarget[resMsg->mCount].mTarget = pAttacker->GetObject();
			resMsg->mTarget[resMsg->mCount].mInfluenceMiss = isInf == false;
			++resMsg->mCount;
		}

		mSendMsgSet.Erase( pAttacker->GetConnectionIdx() );

		for( unsigned char j = 0 ; j < resMsg->mCount ; ++j )
		{
			if( resMsg->mTarget[j].mTarget.type != eOBJECTTYPE_PLAYER )
				continue;

			cPlayer* pTarget = OBJECTMANAGER->GetPlayer( resMsg->mTarget[j].mTarget.index );
			if( pTarget == NULL )
				continue;

			mSendMsgSet.Erase( pTarget->GetConnectionIdx() );

			HANDLE              handle = NULL;
			MSG_SYN_SKILL_NONSTOP_TARGET* targetMsg = (MSG_SYN_SKILL_NONSTOP_TARGET*)NETWORK2->GetMsgRoot( &handle, pTarget->GetConnectionIdx(), 
				NM_SKILL, NM_SKILL_NONSTOP_MON_TARGET_SYN );
			if ( resMsg == NULL )
			{
				targetMsg->mSkillClassIndex = resMsg->mSkillClassIndex;
				targetMsg->mAttacker = attacker;
				targetMsg->mObjectMP = pAttacker->GetMP();
				targetMsg->mMaxMP = pAttacker->GetMaxMP();
				targetMsg->mTargetApply = resMsg->mTarget[j];
			}

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKILL_NONSTOP_TARGET) );
		}

		/// 주변 케릭터들에게는 목록만 발송한다.
		MSG_SYN_SKILL_NONSTOP_AROUND aroundMsg;
		aroundMsg.Category = NM_SKILL;
		aroundMsg.Protocol = NM_SKILL_NONSTOP_MON_AROUND_SYN;
		aroundMsg.mSkillClassIndex = pMsg->mSkillClassIndex; 
		aroundMsg.mAttacker = attacker;
		aroundMsg.mObjectMP = pAttacker->GetMP();
		aroundMsg.mMaxMP = pAttacker->GetMaxMP();

		mSendMsgSet.Erase( attacker.index );
		cHashSet::cIterator sB = mSendMsgSet.Begin();
		cHashSet::cIterator sE = mSendMsgSet.End();
		for( ; sB != sE ; ++sB )
		{
			NETWORK2->SendToUser( *sB, (char*)&aroundMsg, sizeof(MSG_SYN_SKILL_NONSTOP_AROUND) );
		}

		NETWORK2->SendMsgRoot( handle, resMsg->GetLength() );
	}
	catch( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_NONSTOP* resMsg = (MSG_RES_SKILL_NONSTOP*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, NM_SKILL_NONSTOP_MON_RES );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = pMsg->mSkillClassIndex;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_NONSTOP) );
		}
	}
}


void cSkillManager::PlayerRushSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool blink )
{

	/// 이함수를 통해 들어오는 공격자의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;

	/// 서버에는 hero가 없다aa
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;

	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	char protocol = NM_SKILL_RUSH_RES;
	char syncProtocol = NM_SKILL_RUSH_SYN;
	if( blink == true )
	{
		protocol = NM_SKILL_RUSHBLINK_RES;
		syncProtocol = NM_SKILL_RUSHBLINK_SYN;
	}

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 기본 공격 스킬 인덱스인경우 오류
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		if( blink == true )
		{
			if( pSkillInfo->mShotType != eSHOTTYPE_RUSHBLINK )
				throw SKILL_USED_ERR_SKILLTYPE;
		}
		else
		{
			if( pSkillInfo->mShotType != eSHOTTYPE_RUSHRUN )
				throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 스킬 사용 불가
		if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
			throw SKILL_USED_ERR_USEVALUE;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 이미 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;


		/// 공격자에게 타겟을 설정한다
		pAttacker->SetTarget( eOBJECTTYPE_NONE, 0 );


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}


		/// 액티브 타임도 없는데 케스팅할일은 없다.
		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
		{
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest[%d,%d] ShotType == eSHOTTYPE_CASTING", attacker.index, pMsg->mSkillClassIndex );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 공격자의 보유스킬인지 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( attacker.index, skillIdx );
		if( !pPlayerHaveSkill )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest GetPlayerHaveSkill[%d,%d]", attacker.index, skillIdx );
			throw SKILL_USED_ERR_DONTHAVE;
		}


		/// 스킬 스크립트 스텝정보 읽어옴
		unsigned int stepIdx = pPlayerHaveSkill->GetSkillStep();
		if( stepIdx < skillStep )
		{
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest stepIdx >= pSkillInfo->mStepCount  [%d,%d,(%d >= %d)]", attacker.index, skillIdx, stepIdx, pSkillInfo->mStepCount );
			throw SKILL_USED_ERR_DONTHAVE;
		}

		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];

		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pAttacker->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest IsEquipment == false" );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{
			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}


		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모 HP
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}


		NiPoint3 targetPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
		cBaseObject* pTarget = NULL;

		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_NONE:					/// 범위 없음
			{
				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pStepInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "cSkillManager::PlayerRushSkillRequest[%d] mTargetDist == 0", skillIdx );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
				case eAPPLYTYPE_BUFF_ENEMY:
				case eAPPLYTYPE_DEBUFF_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type != eOBJECTTYPE_MONSTER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_BUFF_PARTY:
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 자신은 무조건 가능
						if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
							break;

						bool isParty = false;

						/// 타겟이 파티원인지 체크
						cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
						if( pParty && pParty->IsUser( target.index ) )
						{
							isParty = true;
						}
						else
						{
							/// 파티연합인지 체크
							cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
							if( partyUnion && partyUnion->IsUser( pAttacker->GetObjectID(), target.index ) )
							{
								isParty = true;
							}
						}

						/// 타겟이 파티원이 아닌 경우
						if( isParty == false )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				}

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( pStepInfo->mTargetDist + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

			}break;
			/// 논스탑으로 필드를 지정하면서 설정을 하진 않는다. - 스크립트오류
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::PlayerRushSkillRequest mBoundType[%d,%d]", skillIdx, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
		if( pSkillInfo->mApplyValueType != eSTATUSPLUS_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_BUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_DEBUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_BUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_DEBUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_BUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;

				}
				break;
			case eAPPLYTYPE_DEBUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;

			}//end switch( pSkillInfo->mApplyType )
		}

		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		unsigned long currentTime = NETWORK2->GetAccumTime();

		/// 쿨타임
		if( pStepInfo->mCoolTime != 0 && pPlayerHaveSkill->GetEndCoolTime() > currentTime )	
		{ 
			pAttacker->SkillCoolTimeFailPlus();

			throw SKILL_USED_ERR_COOLTIME;
		}

		if( pAttacker->IsMovePossible( pAttacker->GetPos(), pTarget->GetPos() ) != 0 )
			throw SKILL_USED_ERR_RUSH_OBSTRUCTION;

		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );

		/// HP를 소모 하는 스킬
		if( pStepInfo->mUseHP != 0 )
		{
			/// HP 소모 & 메세지 발송
			pAttacker->HPUse( pStepInfo->mUseHP, true );
		}


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
			}
		}

		/// 쿨타임 갱신
		SKILLMANAGER->UpdateSkillCoolTime( attacker, pMsg->mSkillClassIndex, (unsigned char)stepIdx, NETWORK2->GetAccumTime(), 1.0f );

		unsigned char targetMaxCnt = pStepInfo->mApplyCnt;
		if( targetMaxCnt == 0 || targetMaxCnt > TARGET_CNT_MAX )
			targetMaxCnt = TARGET_CNT_MAX;
		else
		{
			/// 적용 인원이 제한된 경우 공격자가 범위 스킬 적용 인원 증가 효과에 걸려있는지 체크 하여 넣기
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				long addApplyCnt = (long)pAttacker->GetStateOddity( eODDITYTYPE_ADD_APPLY_CNT );
				if( addApplyCnt > 0 )
				{
					if( (addApplyCnt + targetMaxCnt) > UCHAR_MAX )
					{
						assert(0);
						targetMaxCnt = TARGET_CNT_MAX;
					}
					else
					{
						targetMaxCnt = targetMaxCnt + (unsigned char)addApplyCnt;
						if ( targetMaxCnt > TARGET_CNT_MAX )
						{
							assert(0);
							targetMaxCnt = TARGET_CNT_MAX;
						}
					}
				}
			}
		}

		unsigned long nextSkill = SKILLSCRIPT->GetBridgeSkillMap( pMsg->mSkillClassIndex, skillStep );

		/// 돌진 설정
		NiPoint2 sendPos = pAttacker->SetSkillRush( pTarget, blink, nextSkill, (unsigned char)skillStep );

		HANDLE              handle = NULL;
		MSG_RES_SKILL_RUSH* resMsg = (MSG_RES_SKILL_RUSH*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, protocol );
		if ( resMsg == NULL )
			throw SKILL_USED_ERR_OBJECTINIT;

		resMsg->mSkillClassIndex = pMsg->mSkillClassIndex;
		resMsg->mNextSkill = nextSkill;
		resMsg->mNextStep = (unsigned char)skillStep;
		resMsg->mManagedKey = pMsg->mManagedKey;
		resMsg->mObjectMP = pAttacker->GetMP();
		resMsg->mMaxMP = pAttacker->GetMaxMP();
		resMsg->mTarget = pTarget->GetObject();
		resMsg->mPosX = sendPos.x;
		resMsg->mPosY = sendPos.y;
		resMsg->mBlink = blink;

		NETWORK2->SendMsgRoot( handle, sizeof( MSG_RES_SKILL_RUSH ) );

		/// 주변 케릭터들에게는 목록만 발송한다.
		MSG_SYN_SKILL_RUSH aroundMsg;
		aroundMsg.Category = NM_SKILL;
		aroundMsg.Protocol = syncProtocol;
		aroundMsg.mSkillClassIndex = pMsg->mSkillClassIndex; 
		aroundMsg.mAttacker = attacker;
		aroundMsg.mObjectMP = pAttacker->GetMP();
		aroundMsg.mMaxMP = pAttacker->GetMaxMP();
		aroundMsg.mTarget = pTarget->GetObject();
		aroundMsg.mPosX = sendPos.x;
		aroundMsg.mPosY = sendPos.y;
		aroundMsg.mBlink = blink;
		NETWORK2->QuickSendExcept( pAttacker, (char*)&aroundMsg, sizeof( MSG_SYN_SKILL_RUSH ) );

	}
	catch( int error )
	{
		/// 스킬 실패 메세지
		HANDLE              handle = NULL;
		MSG_RES_SKILL_RUSH* resMsg = (MSG_RES_SKILL_RUSH*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), 
			NM_SKILL, protocol );
		if ( resMsg != NULL )
		{
			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_RUSH) );
		}
	}
}

void cSkillManager::PlayerRushAfterSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg )
{

	/// 이함수를 통해 들어오는 유저의 타입은 플레이어 뿐이다.
	sObject attacker = { eOBJECTTYPE_PLAYER, characterIdx };

	sObject target = pMsg->mTarget;
	unsigned long skillIdx = pMsg->mSkillClassIndex;
	unsigned long clientIdx = pMsg->mManagedKey;
	unsigned char skillStep = pMsg->mSkillStep;

	/// 서버에는 hero가 없다
	if( target.type == eOBJECTTYPE_HERO )
		target.type = eOBJECTTYPE_PLAYER;


	/// 공격자 객체 체크
	cPlayer* pAttacker = (cPlayer*)OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("PlayerObjectSkillRequest - GetObject( %d, %d )", attacker.type, attacker.index );
		assert(NULL);
		return; 
	}

	pAttacker->MoveStop();
	pAttacker->SkillReqCntPlus();

	try
	{
		if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
			throw SKILL_USED_ERR_ATTACKERSTATE;

		/// 기본 공격 스킬 인덱스인경우 오류
		if( skillIdx <= NORMAL_ATTACK_SKILL_MAX )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest - skillIdx[%d] > NORMAL_ATTACK_SKILL_MAX", skillIdx );
			throw SKILL_USED_ERR_SKILLTYPE;
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->IsChangeState( eOBJECT_STATE_ATTACK, eCHANGESTATEOPTION_RUSH ) == false )
			throw SKILL_USED_ERR_ATTACKERSTATE;

	
		/// 스킬index 체크
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
		if( pSkillInfo == NULL )	
		{ 
			/// 스킬 스크립트 데이터 획득 오류
			assert(NULL); 
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest skillScript[%d]", skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}

		/// 스킬 사용 불가
		if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == true )
			throw SKILL_USED_ERR_USEVALUE;


		if( pAttacker->GetChgMonsterIdx() != 0 )
			throw SKILL_USED_ERR_CHG_MONSTER;


		/// 스킬 사용중인지 체크
		if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
			throw SKILL_USED_ERR_USING;

		if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
		{
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest[%d] ShotType != eSHOTTYPE_CASTING[%d]", characterIdx, skillIdx );
			throw SKILL_USED_ERR_SCRIPT;
		}


		/// 액티브 스킬이 아닌 경우
		if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest eSKILLTYPE_PASSIVE[%d,%d]", skillIdx, pSkillInfo->mType );
			throw SKILL_USED_ERR_PASSIVE;
		}

		/// 스킬 스크립트 스텝정보 읽어옴
		unsigned int stepIdx = pMsg->mSkillStep;
		if( stepIdx >= pSkillInfo->mStepCount )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::PlayerRushAfterSkillRequest stepIdx >= pSkillInfo->mStepCount  [%d,%d,(%d >= %d)]", attacker.index, skillIdx, stepIdx, pSkillInfo->mStepCount );
			throw SKILL_USED_ERR_DONTHAVE;
		}

		sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[pMsg->mSkillStep];

		/// 돌진후 이어지는 스킬은 돌진에서 설정된 값을 체크후 사용 한다.
		if( pAttacker->GetRushAfterSkillIdx() != skillIdx )
			throw SKILL_USED_ERR_DONTHAVE;

		if( pAttacker->GetRushAfterSkillStep() != stepIdx )
			throw SKILL_USED_ERR_DONTHAVE;

		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pAttacker->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pAttacker->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		if( IsEquipment == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("PlayerObjectSkillRequest IsEquipment == false" );
			throw SKILL_USED_ERR_EQUIPMENT;
		}

		/// 캐릭터 상태
		if( pSkillInfo->mUseState != eSKILL_USE_STATE_NONE ) // 모두
		{

			if( pAttacker->GetMaxHP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 1");
				throw SKILL_USED_ERR_USEVALUE;
			}

			if( pAttacker->GetMaxMP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 2");
				throw SKILL_USED_ERR_USEVALUE;
			}

			switch( pSkillInfo->mUseState )
			{
			case eSKILL_USE_STATE_HP_PER_UP:	// HPUP
				if( pStepInfo->mUseStateValue > ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_HP_PER_DOWN: // HPDONW:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetHP() * PERCENT_HUNDRED / pAttacker->GetMaxHP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_UP: // MPUP:		
				if( pStepInfo->mUseStateValue > ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_MP_PER_DOWN: // MPDOWN:	
				if( pStepInfo->mUseStateValue < ( pAttacker->GetMP() * PERCENT_HUNDRED / pAttacker->GetMaxMP() ) )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_BUFF: // 
				if( pAttacker->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )	
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			case eSKILL_USE_STATE_SELF_DEBUFF: // MPDOWN:	
				if( pAttacker->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
				{ 
					throw SKILL_USED_ERR_USEVALUE;
				}break;
			default:
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerObjectSkillRequest pSkillInfo->mUseState[%d]", pSkillInfo->mUseState );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}


		unsigned long useMP = pStepInfo->mUseMP;
		if( pStepInfo->mUseMPPer != 0 )
			useMP += pAttacker->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

		/// 소모 MP 체크
		if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
		}

		/// 소모 MP 수치 체크
		if( useMP > pAttacker->GetMP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모 HP 수치 체크
		if( pStepInfo->mUseHP >= pAttacker->GetHP() )	
			throw SKILL_USED_ERR_USEVALUE;


		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
				throw SKILL_USED_ERR_USEVALUE;
		}

		cBaseObject* pTarget = NULL;
		NiPoint3 targetPos = NiPoint3::ZERO;

		/// 타겟 사거리 체크
		switch( pSkillInfo->mBoundType )
		{
		case eBOUNDTYPE_SELF_NOTARGET:			/// 자신이 범위 시작위치여서 거리체크 안함	
			{
				/// 자신을 타겟으로 선정
				target = attacker;
				targetPos.x = pAttacker->GetXPos();
				targetPos.y = pAttacker->GetYPos();
				targetPos.z = pAttacker->Height();
				pTarget = pAttacker;
			}
			break;
		case eBOUNDTYPE_NONE:					/// 범위 없음
		case eBOUNDTYPE_TARGET:					/// 타겟 기준범위
		case eBOUNDTYPE_SELF_TARGET:			/// 시전자(자신) 기준범위 - 발동시 타겟 존재
			{

				/// 자신 일경우 거리체크 안함
				if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF || 
					pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF || 
					pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
				{
					/// 자신을 타겟으로 선정
					target = attacker;
					targetPos.x = pAttacker->GetXPos();
					targetPos.y = pAttacker->GetYPos();
					targetPos.z = pAttacker->Height();
					pTarget = pAttacker;
					break;
				}

				/// 타겟이 있는 스킬의 사거리는 0이 될수 없다. - 스크립트 작성 오류
				if( pStepInfo->mTargetDist == 0 )
				{
					assert(NULL);
					NETWORK2->PostServerEvent( "PlayerObjectSkillRequest[%d] mTargetDist == 0", skillIdx );
					throw SKILL_USED_ERR_SCRIPT;
				}

				pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
				/// 타겟 객체 확인
				if( pTarget == NULL )	
					throw SKILL_USED_ERR_TARGETOBJECT;

				targetPos.x = pTarget->GetXPos();
				targetPos.y = pTarget->GetYPos();
				targetPos.z = pTarget->Height();

				/// 부활 스킬인지 체크
				bool isResurrection = false;

				if( pSkillInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
				{
					isResurrection = true;
					if( NETWORK2->GetServerType() == _E_ST_ID_PVP_ )
						throw SKILL_USED_ERR_TARGETSELECT;
				}

				/// 타겟 생존 유무 체크
				if( pTarget->GetStateDie() == true && isResurrection == false )
					throw SKILL_USED_ERR_TARGETOBJECT;
				else if( pTarget->GetStateDie() == false && isResurrection == true )
					throw SKILL_USED_ERR_TARGETOBJECT;

				/// 타겟 타입 체크
				switch( pSkillInfo->mApplyType ) 
				{
				case eAPPLYTYPE_SELF:
				case eAPPLYTYPE_BUFF_SELF:
				case eAPPLYTYPE_DEBUFF_SELF:
					break;
				case eAPPLYTYPE_ENEMY:
				case eAPPLYTYPE_BUFF_ENEMY:
				case eAPPLYTYPE_DEBUFF_ENEMY:
					{
						/// 타겟 조건 체크
						if( target.type == eOBJECTTYPE_MONSTER )
						{
							unsigned long attackerTeamType = pAttacker->GetPvPDMTeam();
							if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
							{
								if( attackerTeamType == ((cMonster*)pTarget)->GetDMTeamType() )
									throw SKILL_USED_ERR_TARGETSELECT;
							}
						}
						else if( target.type == eOBJECTTYPE_PLAYER )
						{
							if( pAttacker->IsAttack( target.index ) == false )
								throw SKILL_USED_ERR_TARGETSELECT;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_BUDDY:
					{
						/// pvp유저 빼해야함
						if( target.type != eOBJECTTYPE_PLAYER )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				case eAPPLYTYPE_PARTY:
				case eAPPLYTYPE_BUFF_PARTY:
				case eAPPLYTYPE_DEBUFF_PARTY:
					{
						/// 자신은 무조건 가능
						if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
							break;

						bool isParty = false;

						/// 타겟이 파티원인지 체크
						cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pAttacker)->GetPartyIndex() );
						if( pParty && pParty->IsUser( target.index ) )
						{
							isParty = true;
						}
						else
						{
							/// 파티연합인지 체크
							cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pAttacker)->GetPartyUnionIndex() );
							if( partyUnion && partyUnion->IsUser( pAttacker->GetObjectID(), target.index ) )
							{
								isParty = true;
							}
						}

						/// 타겟이 파티원이 아닌 경우
						if( isParty == false )
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					break;
				}

				/// 스킬+아이템+효과 사거리
				float range = pAttacker->CalcStatusSkillRange( pStepInfo->mTargetDist, pSkillInfo->mRangeType, false );

				range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

				/// 스킬 사거리 밖에 타겟이 있다.
				cRangeCheck skillRangeCheck( range + SYNC_MOVE_RANGE );
				NiPoint3 attackerPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );

				if( !skillRangeCheck.IsRange( attackerPos, targetPos ) )	
				{ 
					HANDLE             handle = NULL;
					MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
					if ( posMsg != NULL )
					{
						posMsg->mAttacker     = attacker;
						posMsg->mAttackerPosX = pAttacker->GetXPos( );
						posMsg->mAttackerPosY = pAttacker->GetYPos( );
						posMsg->mTarget       = target;
						posMsg->mTargetPosX   = targetPos.x;
						posMsg->mTargetPosY   = targetPos.y;

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
					}

					throw SKILL_USED_ERR_TARGETDIST;
				}

				pAttacker->CalcDirection( pTarget->GetXPos(), pTarget->GetYPos() );

				/// 공격자에게 타겟을 설정한다
				pAttacker->SetTarget( target.type, target.index );

			}break;
			/// 필드인경우는 이함수에 들어올수 없음
		case eBOUNDTYPE_FIELD:
		case eBOUNDTYPE_AUTOFIELD:
		default:
			{
				/// 스크립트 내용이 잘못됐다.
				assert(NULL);
				NETWORK2->PostServerEvent("PlayerObjectSkillRequest mBoundType[%d,%d]", skillIdx, pSkillInfo->mBoundType );
				throw SKILL_USED_ERR_SCRIPT;
			}
		}	

		/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
		if( pSkillInfo->mApplyValueType != eSTATUSPLUS_NONE )
		{
			switch( pSkillInfo->mApplyType )
			{
			case eAPPLYTYPE_BUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_DEBUFF_SELF:
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
						throw SKILL_USED_ERR_APPLYVALUE_STATE;
				}
				break;
			case  eAPPLYTYPE_BUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_DEBUFF_ENEMY:
				{
					/// 타겟 조건 체크
					if( target.type == eOBJECTTYPE_MONSTER )
					{
						if( pTarget )
						{
							/// 보유중인 효과에서 해당 효과가 있는지 체크
							if( ((cMonster*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;

					}
					else if( target.type == eOBJECTTYPE_PLAYER )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pTarget )
						{
							if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
								throw SKILL_USED_ERR_APPLYVALUE_STATE;
						}
						else
							throw SKILL_USED_ERR_TARGETSELECT;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;
			case eAPPLYTYPE_BUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;

				}
				break;
			case eAPPLYTYPE_DEBUFF_PARTY:
				{
					/// 자신은 무조건 가능
					if( target.index == pAttacker->GetObjectID() && target.type == pAttacker->GetObjectType() )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( pAttacker->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
						else
							break;
					}

					if( pTarget )
					{
						/// 보유중인 효과에서 해당 효과가 있는지 체크
						if( ((cPlayer*)pTarget)->IsHaveApplyDeBuffType( pSkillInfo->mApplyValueType ) == false )
							throw SKILL_USED_ERR_APPLYVALUE_STATE;
					}
					else
						throw SKILL_USED_ERR_TARGETSELECT;
				}
				break;

			}//end switch( pSkillInfo->mApplyType )
		}

		/// 플레이어가 목적지에 도착하기전에 공격메세지가 들어오는 경우 허용범위 계산
		mRangeCheck.SetRadius( SYNC_MOVE_RANGE );
		NiPoint2 clientPos( pMsg->mClientPosX, pMsg->mClientPosY );
		NiPoint2 serverPos = pAttacker->GetPos();

		if( mRangeCheck.IsRange( serverPos, clientPos ) == false )
		{
			/// 이동 중이던 상태 정지
			pAttacker->SetMoveTargetPos( serverPos.x, serverPos.y );
			pAttacker->ClearMoveSyncCalc();

			HANDLE             handle = NULL;
			MSG_RES_SKILL_POS* posMsg = (MSG_RES_SKILL_POS*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx( ), NM_SKILL, NM_SKILL_USED_POS_ERROR_RES );
			if ( posMsg != NULL )
			{
				posMsg->mAttacker = attacker;
				posMsg->mAttackerPosX = pAttacker->GetXPos( );
				posMsg->mAttackerPosY = pAttacker->GetYPos( );
				posMsg->mTarget = target;
				posMsg->mTargetPosX = targetPos.x;
				posMsg->mTargetPosY = targetPos.y;

				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_POS) );
			}

			throw SKILL_USED_ERR_ATTAKCERPOS;
		}

		/// 고유키 생성
		unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
		if( skillUniqueIdx == 0 )	
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest GeneratIdx[%d]", skillUniqueIdx );
			throw SKILL_USED_ERR_UNIQUEIDX;
		}

		/// 스킬 객체 생성
		cPlayerSkillObject* pSkillObject = new cPlayerSkillObject;
		bool init = pSkillObject->InitRushAfterSkill( skillIdx, skillStep, skillUniqueIdx, pAttacker, pTarget,  pStepInfo->mCastingTime );
		if( init == false )
		{ 
			/// 스킬 객체 생성 실패
			assert(NULL);
			SAFE_DELETE( pSkillObject );
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest new SkillObject" );		
			throw SKILL_USED_ERR_OBJECTINIT;			
		}

		/// 플레이어의 상태 체크한다.
		if( pAttacker->ChangeState( eOBJECT_STATE_ATTACK, eCHANGESTATEOPTION_RUSH ) == false )
			assert(NULL);

		if( pAttacker->GetVehicleAttack() == true )
			NETWORK2->PostServerEvent( "PlayerObjectSkillRequest VehicleAttack[%d,%d,%d]", characterIdx, skillIdx, skillStep );

		/// 스킬을 리스트에 등록
		InsertPlayerSkillMap( characterIdx, pSkillObject );

		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );

		/// HP를 소모 하는 스킬
		if( pStepInfo->mUseHP != 0 )
		{
			/// HP 소모 & 메세지 발송
			pAttacker->HPUse( pStepInfo->mUseHP, true );
		}

		/// 소모아이템 IDX
		if( pStepInfo->mUseItemCount != 0 )
		{
			if( pAttacker->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("ERROR - item use skill itemidx[%d], itemcnd[%d]", pSkillInfo->mUseItem, pStepInfo->mUseItemCount );
			}
		}

		/// 캐스팅이 아닌 경우 발동 체크 
		/// *** 캐스팅인 경우는 skillObj의 RequireValue() 에서 체크 ***
		switch( pSkillInfo->mType )
		{
		case eSKILLTYPE_DAMAGE:
		case eSKILLTYPE_MPDAMAGE:
			CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
			break;
		case eSKILLTYPE_HEAL:
			CheckActInfluence( eACTSTATUS_HEALSKILL, pAttacker, pTarget );
			break;
		}

		/// 스킬 발동 효과
		if( pStepInfo->mActiveIdx != 0 )
			SKILLMANAGER->CheckSkillActInfluence( pStepInfo->mActiveIdx, pAttacker, pTarget );

		/// 스킬 생성 성공 메세지를 해당 히어로에게 전달
		HANDLE              handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			resMsg->Protocol = NM_SKILL_RUSHAFTER_RES;
			resMsg->ErrorCode        = 0;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;
			resMsg->mSkillIndex      = skillUniqueIdx;
			resMsg->mObjectMP        = pAttacker->GetMP();
			resMsg->mMaxMP           = pAttacker->GetMaxMP();

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		synMsg.Protocol = NM_SKILL_RUSHAFTER_SYN;
		synMsg.mSkillClassIndex = skillIdx;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker = attacker;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTarget = target;
		synMsg.mTargetPosX = targetPos.x;
		synMsg.mTargetPosY = targetPos.y;
		synMsg.mObjectMP = pAttacker->GetMP();
		synMsg.mObjectMaxMP = pAttacker->GetMaxMP();
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->SkillSuccessCntPlus();
	}
	catch ( int error )
	{
		/// 스킬 실패 메세지
		HANDLE handle = NULL;
		MSG_RES_SKILL_USED* resMsg = (MSG_RES_SKILL_USED*)NETWORK2->GetMsgRoot( &handle, pAttacker->GetConnectionIdx(), NM_SKILL );

		if ( resMsg != NULL )
		{
			resMsg->Protocol = NM_SKILL_RUSHAFTER_RES;

			resMsg->ErrorCode        = error;
			resMsg->mSkillClassIndex = skillIdx;
			resMsg->mManagedKey      = clientIdx;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_USED) );
		}
	}
}

bool cSkillManager::MonsterAttack( unsigned long monsterIdx, eMONSTERATTACK_TYPE attackType, sObject target )
{
	if( attackType == eMONSTERATTACK_MAX )
		return false;

	/// 이함수를 통해 들어오는 유저의 타입은 몬스터 뿐이다.
	sObject attacker = { eOBJECTTYPE_MONSTER, monsterIdx };


	/// 공격자 객체 체크
	cMonster* pAttacker = OBJECTMANAGER->GetMonster( attacker.index );
	if( pAttacker == NULL )	
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent("MonsterAttack - attacker.index[%d]", attacker.index );
		return false; 
	}

	if( pAttacker->GetStateDie() == true || pAttacker->IsSkillPushPull() == true )
		return false;


	/// 이미 스킬 사용중인지 체크
	if( IsUsingSkill( pAttacker->GetObjectType(), pAttacker->GetObjectID() ) == true )	
		return false; 


	/// 타겟 객체 체크
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL || pTarget->GetStateDie() == true )	
		return false; 


	/// 스킬index 체크
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( pAttacker->GetRaceGender(), attackType );
	if( pSkillInfo == NULL )	
	{ 
		/// 스킬 스크립트 데이터 획득 오류
		assert(NULL); 
		NETWORK2->PostServerEvent("MonsterAttack - GetMonsterSkillInfo[%d,%d]", pAttacker->GetRaceGender(), attackType );
		return false; 
	}

	switch( pSkillInfo->mApplyType )
	{
	case eAPPLYTYPE_SELF:
		{
			if( target.type != eOBJECTTYPE_MONSTER )
				return false;

			if( monsterIdx != target.index )
				return false;
		}
		break;
	case eAPPLYTYPE_ENEMY:
		{
			/// 타겟 설정 체크 
			if( target.type != eOBJECTTYPE_PLAYER )
				return false;
		}
		break;
	case eAPPLYTYPE_BUDDY:
		{
			if( target.type != eOBJECTTYPE_MONSTER )
				return false;
		}
		break;
	case eAPPLYTYPE_PARTY:
		{
			return false;
		}
		break;
	case eAPPLYTYPE_DIEBUDDY:
		{
			assert(NULL);
			return false;
		}
		break;
	// 현재는 몬스터 쪽에서 사용하고 있지 않다 [4/6/2010 Jo_Client]
	case eAPPLYTYPE_BUFF_SELF:
	case eAPPLYTYPE_DEBUFF_SELF:		
	case eAPPLYTYPE_BUFF_ENEMY:		
	case eAPPLYTYPE_DEBUFF_ENEMY:	
	case eAPPLYTYPE_BUFF_PARTY:		
	case eAPPLYTYPE_DEBUFF_PARTY:
		return false;
		break;
	}

	/// 소모 MP 체크
	unsigned long useMP = pSkillInfo->mUseMP;
	if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
	{
		useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( pSkillInfo->mUseMP );
	}

	/// 소모 MP 수치 체크
	if( useMP > pAttacker->GetMP() )	
		return false;

	bool casting = false;
	if( pSkillInfo->mShotType == eMONSTERSHOTTYPE_CASTING )
		casting = true;

	/// 스킬 고유번호 생성
	unsigned long skillUniqueIdx = mSkillGen.GeneratIdx();
	if( skillUniqueIdx == 0 )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent( "MonsterAttack skillUniqueIdx[%d]", skillUniqueIdx );
		return false; 
	}

	float attackSpeed = 1.0f;

	/// 일반 공격만 공속값 변경
	if( eMONSTERATTACK_NORMAL1 == attackType || eMONSTERATTACK_NORMAL2 == attackType )
	{
		attackSpeed = pAttacker->GetAttackSpeed();
	}

	cMonsterSkillObject* pSkillObject = new cMonsterSkillObject;
	/// 임의 위치 지정
	if( pSkillInfo->mBoundType == eBOUNDTYPE_FIELD || pSkillInfo->mBoundType == eBOUNDTYPE_AUTOFIELD )
	{
		float radian = pAttacker->GetDirection();
		NiPoint2 dir = NiPoint2( -sinf(radian), -cosf(radian) );
		dir.Unitize();

		NiPoint2 pos = pAttacker->GetPos() + dir*pSkillInfo->mTargetDist;

		/// 못가는 지역에 쏘는 경우를 위해 검증코드 - 설정 부분을 기획팀에서 눈으로 볼수있게 따로 예외처리를 하진 않는다.
		if( AIMANAGER->CalcPathCnt( pAttacker->GetMapNumber(), pAttacker->GetPos(), pos ) == 0 )
			NETWORK2->PostServerEvent( "cSkillManager::MonsterAttack pSkillInfo->mBoundType == eBOUNDTYPE_FIELD[%d,%d] NotPath",
			pAttacker->GetRaceGender(), attackType );

		/// 스킬 객체 생성
		bool init = pSkillObject->InitMonsterSkillPosition( monsterIdx, pAttacker->GetRaceGender(), attackType, 
			pos.x, pos.y, skillUniqueIdx, casting, pAttacker->GetMapNumber() );
		if( init == false )
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "MonsterAttack new SkillObject pSkillInfo->mBoundType == eBOUNDTYPE_FIELD" );
			SAFE_DELETE( pSkillObject );
			return false;
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_POS_MONSTER_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		if( casting == true )
			synMsg.Protocol = NM_SKILL_POS_MONSTER_CAST_USED_SYN;
		else
			synMsg.Protocol = NM_SKILL_POS_MONSTER_USED_SYN;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker.type = eOBJECTTYPE_MONSTER;
		synMsg.mAttackType = (unsigned char)attackType;
		synMsg.mAttacker.index = monsterIdx;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mTargetPosX = pos.x;
		synMsg.mTargetPosY = pos.y;
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );
	}
	else
	{

		float range = pAttacker->CalcStatusSkillRange( pSkillInfo->mTargetDist, pSkillInfo->mRangeType );

		range = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, range );

		/// 스킬 사거리 밖에 타겟이 있다.
		cRangeCheck skillRangeCheck( range );

		if( !skillRangeCheck.IsRange( pAttacker->GetPos(), pTarget->GetPos() ) )
		{
			SAFE_DELETE( pSkillObject );
			return false; 
		}

		/// 스킬 객체 생성
		bool init = pSkillObject->InitMonsterAttack( monsterIdx, pAttacker->GetRaceGender(), attackType, 
			target, skillUniqueIdx, casting, attackSpeed, pAttacker->GetMapNumber() );
		if( init == false )
		{ 
			assert(NULL);
			NETWORK2->PostServerEvent( "MonsterAttack new SkillObject pSkillInfo->mBoundType != eBOUNDTYPE_FIELD" );
			SAFE_DELETE( pSkillObject );
			return false;
		}

		/// 스킬 생성 메세지를 다른 클라이언트에게 전달
		MSG_SYN_MONSTER_SKILL_USED synMsg;
		synMsg.Category = NM_SKILL;
		if( casting == true )
			synMsg.Protocol = NM_SKILL_MONSTER_CAST_USED_SYN;
		else
			synMsg.Protocol = NM_SKILL_MONSTER_USED_SYN;
		synMsg.mSkillIndex = skillUniqueIdx;
		synMsg.mAttacker.type = eOBJECTTYPE_MONSTER;
		synMsg.mAttackType = (unsigned char)attackType;
		synMsg.mAttacker.index = monsterIdx;
		synMsg.mAttackerPosX = pAttacker->GetXPos();
		synMsg.mAttackerPosY = pAttacker->GetYPos();
		synMsg.mAttackSpeedFactor = attackSpeed;
		synMsg.mTarget = target;
		NETWORK2->QuickSendExcept( pAttacker, (char*)&synMsg, sizeof(synMsg) );

		pAttacker->CalcDirection( pTarget->GetPos() );
	}

	/// 캐스팅을 하지 않는 스킬이면 스킬사용시 소모항목 감소
	if( casting == false )
	{
		/// MP를 소모 하는 스킬
		if( useMP != 0 )
			pAttacker->MPUse( useMP, false );
		
		pAttacker->UpdateSkillCoolTime( attackType );
	}

	/// 스킬을 리스트에 등록
	InsertMonsterSkillMap( monsterIdx, pSkillObject );

	pAttacker->SetAttackType( attackType );
	pAttacker->ModeSkillEnd();

	/// 모드스킬을 제외한 스킬만 대사를 출력한다.
	/// ※ 모드 스킬은 따로 출력하지 않고 모드 전환시 모드에 해당하는 대사만 한다.
	if( eMONSTERATTACK_NORMAL1 <= attackType && attackType <= eMONSTERATTACK_SKILL3 )
		pAttacker->SendSpeech( attackType + eMONSTERTALK_ATTACKSTART );

	pAttacker->MoveStop( false );

	unsigned long attackEndTime = 0;
	if( pAttacker->GetAttackType() == eMONSTERATTACK_NORMAL1 || pAttacker->GetAttackType() == eMONSTERATTACK_NORMAL2 )
		attackEndTime = NETWORK2->GetAccumTime() + (unsigned long)( pSkillInfo->mAttackSpd * ( 2.0f - pAttacker->GetAttackSpeed() ) );
	else
		attackEndTime = NETWORK2->GetAccumTime() + pSkillInfo->mAttackSpd;

	pAttacker->CalcSkillWaitTime( attackEndTime );

	return true;
}


bool cSkillManager::DeleteSkillObject( unsigned char type, unsigned long objectIdx, unsigned long skillUniqueIdx )
{
	switch( type )
	{
	case eOBJECTTYPE_MONSTER:
		{
			cMonsterSkillObject* pSkillObject = NULL;

			cArray* pArray = (cArray*)mMonsterSkillMap.GetAt( objectIdx );
			if( pArray == NULL )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::DeleteSkillObject(%d,%d,%d) pArray == NULL", type, objectIdx, skillUniqueIdx );
				return false;
			}

			bool eraseOK = false;
			for( unsigned int i = pArray->GetSize() ; i > 0 ; --i )
			{
				pSkillObject = (cMonsterSkillObject*)(*pArray)[i-1];
				if( pSkillObject == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("cSkillManager::DeleteSkillObject(%d,%d,%d) pSkillObject == NULL", type, objectIdx, skillUniqueIdx );
					pArray->PopAt( i-1 );
					continue;
				}

				if( pSkillObject->GetUniqueIdx() == skillUniqueIdx )
				{
					/// 몬스터가 마지막으로 사용한 스킬을 기록한다.
					cMonster* pMonster = OBJECTMANAGER->GetMonster( objectIdx );
					if( pMonster != NULL )
						pMonster->SetLastUsedSkillIdx( pSkillObject->GetSkillIdx() );

					SAFE_DELETE( pSkillObject );
					pArray->PopAt( i-1 );
					eraseOK = true;
					break;;
				}
			}

			if( pArray->GetSize() == 0 )
			{
				SAFE_DELETE( pArray );
				mMonsterSkillMap.Erase( objectIdx );
			}

			if( eraseOK == true )
				return true;
		}
		break;
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			cBaseSkillObject* pSkillObject = NULL;

			cArray* pArray = (cArray*)mPlayerSkillMap.GetAt( objectIdx );
			if( pArray == NULL )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::DeleteSkillObject(%d,%d,%d) pArray == NULL", type, objectIdx, skillUniqueIdx );
				return false;
			}

			bool eraseOK = false;
			for( unsigned int i = pArray->GetSize() ; i > 0 ; --i )
			{
				pSkillObject = (cBaseSkillObject*)(*pArray)[i-1];
				if( pSkillObject == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("cSkillManager::DeleteSkillObject(%d,%d,%d) pSkillObject == NULL", type, objectIdx, skillUniqueIdx );
					pArray->PopAt( i-1 );
					continue;
				}

				if( pSkillObject->GetUniqueIdx() == skillUniqueIdx )
				{
					SAFE_DELETE( pSkillObject );
					pArray->PopAt( i-1 );
					eraseOK = true;
					break;
				}
			}

			if( pArray->GetSize() == 0 )
			{
				SAFE_DELETE( pArray );
				mPlayerSkillMap.Erase( objectIdx );
			}

			if( eraseOK == true )
				return true;
		}
		break;
	default:
		return false;
	}

	return false;
}


void cSkillManager::InsertDeleteSkillObject( sObject skillUser, unsigned long skillUniqueIdx ) 
{ 
	sDelSkillInfo info;
	info.skillUser = skillUser;
	info.mSkillIdx = skillUniqueIdx;

	mDeleteSkillAry.PushBack( info ); 
}


unsigned char cSkillManager::AddPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep, unsigned long cooltime )
{

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkill - playerIdx[%d]", playerIdx );
		return SKILL_ADD_ERR_PLAYER; 
	}

	/// 스킬index 체크
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( !pSkillInfo )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkill - skillIdx[%d]", skillIdx );
		return SKILL_ADD_ERR_SCRIPT; 
	}

	/// 스킬 단계 값 체크
	if( skillStep < 0 || pSkillInfo->mStepCount <= skillStep )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkill - skillStep[%d]", skillStep );
		return SKILL_ADD_ERR_SKILLSTEP; 
	}

	sPlayerSkillStepInfo* pSkillStep = &pSkillInfo->mpSetpInfoArray[skillStep];

	/*--------------- skilllist_pc ---------------*/

	/// 전직업 사용 SP량
	if( pSkillInfo->mJobStep > 0 && pSkillInfo->mJobStep < MAX_JOBSTEP )
	{
		unsigned long point  = pPlayer->GetJobUsedSkillPoint( pSkillInfo->mJobStep-1 );

		if( point < pSkillInfo->mBeforeReqSP )
			return SKILL_ADD_ERR_JOBSP;
	}

	/// 종족 구분 있는경우
	if( pSkillInfo->mRace != 0 ) /// 0 = ALL
	{
		/// 종족 체크
		switch( pPlayer->GetRace() )
		{
		case eRACE_HUMAN:
			if( !( pSkillInfo->mRace == eSKILLRACE_HUMAN || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_ELF ) )	
			{
				return SKILL_ADD_ERR_RACE; 
			}break;
		case eRACE_BEAST:
			if( !( pSkillInfo->mRace == eSKILLRACE_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_BEAST_ELF ) )
			{ 
				return SKILL_ADD_ERR_RACE; 
			}break;
		case eRACE_ELF:
			if( !( pSkillInfo->mRace == eSKILLRACE_ELF || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_ELF || 
				pSkillInfo->mRace == eSKILLRACE_BEAST_ELF ) )	
			{ 
				return SKILL_ADD_ERR_RACE; 
			}break;
		default:
			assert(NULL); 
			return SKILL_ADD_ERR_SCRIPT; 	
		}
	}

	/// 선행 스킬1 필요
	if( pSkillStep->mLearnSkillIdx1 != 0 )
	{
		/// 선행 보유 스킬 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( playerIdx, pSkillStep->mLearnSkillIdx1 );
		if( !pPlayerHaveSkill )	
			return SKILL_ADD_ERR_LEARNSKILL; 

		/// 선행 보유 스킬의 스텝
		if( pSkillStep->mLearnSkillStep1 > pPlayerHaveSkill->GetSkillStep() + 1 )	
			return SKILL_ADD_ERR_LEARNSTEP; 
	}

	/// 선행 스킬2 필요
	if( pSkillStep->mLearnSkillIdx2 != 0 )
	{
		/// 선행 보유 스킬 체크
		cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( playerIdx, pSkillStep->mLearnSkillIdx2 );
		if( !pPlayerHaveSkill )	
			return SKILL_ADD_ERR_LEARNSKILL; 

		/// 선행 보유 스킬의 스텟
		if( pSkillStep->mLearnSkillStep2 > pPlayerHaveSkill->GetSkillStep() + 1 )	
			return SKILL_ADD_ERR_LEARNSTEP; 
	}


	///*--------------- skilllist_sub ---------------*/
	/// 직업구분 있는 경우
	if( pSkillInfo->mpSetpInfoArray->mJobType != 0 )
	{
		/// 직업을 체크
		if( !SKILLSCRIPT->IsPlayerSkillJob( pSkillInfo->mpSetpInfoArray->mJobType, static_cast<unsigned long>(pPlayer->GetJob()) ) )
			return SKILL_ADD_ERR_JOB; 
	}

	/// 캐릭터 필요 level
	if( pSkillInfo->mpSetpInfoArray->mPlayerLevel > pPlayer->GetLevel() )	
		return SKILL_ADD_ERR_LEVEL; 

	///*---------------------------------------------*/


	/// 추가할 스킬이 보유리스트에 있는지 읽어옴
	cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( playerIdx, skillIdx );

	/// 새로 배우는 스킬인지 체크
	if( skillStep == SKILL_START_STEP )
	{
		/// 이미 스킬이 있으면 오류
		if( pPlayerHaveSkill )	
		{ 
			NETWORK2->PostServerEvent("AddPlayerHaveSkill -alreay have skill[%d](mSkillStep[%d]) skillStep[%d]", skillIdx, pPlayerHaveSkill->GetSkillStep(), skillStep );
			return SKILL_ADD_ERR_SKILLSTEP; 
		}

		/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
		/// 보유한 스킬리스트가 없으면
		if ( !pPlayerHaveSkillMap )
		{
			/// 새로운 스킬리스트를 만듬
			pPlayerHaveSkillMap = new cPointerHashMap;

			/// 새로 만든 스킬리스트를 기록
			if( !( mPlayerHaveSkillMap.Insert( playerIdx, pPlayerHaveSkillMap ) ) )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("AddPlayerHaveSkill[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep );
				SAFE_DELETE( pPlayerHaveSkillMap );
				return SKILL_ADD_ERR_MAPINSERT;
			}
		}

		/// 보유스킬리스트 정보를 기록
		cHaveSkillObject* pPlayerHaveSkill = new cHaveSkillObject;
		pPlayerHaveSkill->Init( skillIdx, 0, cooltime );

		if( !(pPlayerHaveSkillMap->Insert( skillIdx, pPlayerHaveSkill ) ) )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("AddPlayerHaveSkill[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep  );
			SAFE_DELETE( pPlayerHaveSkill );
			return SKILL_ADD_ERR_MAPINSERT;
		}
	}
	else	/// 단계를 업시키는 경우
	{
		/// 보유스킬이 없으면 오류
		if( pPlayerHaveSkill == 0 )	
		{ 
			return SKILL_ADD_ERR_SKILLSTEP; 
		}

		/// 단계 업조건이 만족하지 않으면 오류
		if( pPlayerHaveSkill->StepUp( skillStep, cooltime ) == false )	
		{ 
			return SKILL_ADD_ERR_SKILLSTEP; 
		}
	}

	/// 패시브 일경우 스텟 재계산
	if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
	{
		/// 기본항목을 제외한 확장(+,%) 항목만 계산
		STATUSCALC->CalcPlayerExtensionGlobal( pPlayer->GetObject() );
	}

	/// 오라스킬이면 기존 스킬 끔
	if( pSkillInfo->mShotType == eSHOTTYPE_AURA )
		AuraInfluenceOff( playerIdx, skillIdx );

	return SKILL_ADD_ERR_SUCCESS;
}

unsigned char cSkillManager::AddPlayerHaveSkillByCheat( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep, unsigned long cooltime )
{

	/// 유저 체크
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkillByCheat - playerIdx[%d]", playerIdx );
		return ERROR_CHEAT_SKILL_ADD_ERR_PLAYER; 
	}

	/// 스킬index 체크
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( !pSkillInfo )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkillByCheat - skillIdx[%d]", skillIdx );
		return ERROR_CHEAT_SKILL_ADD_ERR_SCRIPT; 
	}

	/// 스킬 단계 값 체크
	if( skillStep < 0 || pSkillInfo->mStepCount <= skillStep )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("AddPlayerHaveSkillByCheat - skillStep[%d]", skillStep );
		return ERROR_CHEAT_SKILL_ADD_ERR_SKILLSTEP; 
	}


	/*--------------- skilllist_pc ---------------*/

	/// 종족 구분 있는경우
	if( pSkillInfo->mRace != 0 ) /// 0 = ALL
	{
		/// 종족 체크
		switch( pPlayer->GetRace() )
		{
		case eRACE_HUMAN:
			if( !( pSkillInfo->mRace == eSKILLRACE_HUMAN || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_ELF ) )	
			{
				return ERROR_CHEAT_SKILL_ADD_ERR_RACE; 
			}break;
		case eRACE_BEAST:
			if( !( pSkillInfo->mRace == eSKILLRACE_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_BEAST || 
				pSkillInfo->mRace == eSKILLRACE_BEAST_ELF ) )
			{ 
				return ERROR_CHEAT_SKILL_ADD_ERR_RACE; 
			}break;
		case eRACE_ELF:
			if( !( pSkillInfo->mRace == eSKILLRACE_ELF || 
				pSkillInfo->mRace == eSKILLRACE_HUMAN_ELF || 
				pSkillInfo->mRace == eSKILLRACE_BEAST_ELF ) )	
			{ 
				return ERROR_CHEAT_SKILL_ADD_ERR_RACE; 
			}break;
		default:
			assert(NULL); 
			return ERROR_CHEAT_SKILL_ADD_ERR_SCRIPT; 	
		}
	}


	///*--------------- skilllist_sub ---------------*/
	/// 직업구분 있는 경우
	if( pSkillInfo->mpSetpInfoArray->mJobType != 0 )
	{
		/// 직업을 체크
		if( !SKILLSCRIPT->IsPlayerSkillJob( pSkillInfo->mpSetpInfoArray->mJobType, static_cast<unsigned long>(pPlayer->GetJob()) ) )
			return ERROR_CHEAT_SKILL_ADD_ERR_JOB; 
	}

	///*---------------------------------------------*/
	/// 추가할 스킬이 보유리스트에 있는지 읽어옴
	cHaveSkillObject* pPlayerHaveSkill = GetPlayerHaveSkill( playerIdx, skillIdx );

	/// 새로 배우는 스킬인지 체크
	if( pPlayerHaveSkill == NULL )	
	{ 
		/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
		/// 보유한 스킬리스트가 없으면
		if ( !pPlayerHaveSkillMap )
		{
			/// 새로운 스킬리스트를 만듬
			pPlayerHaveSkillMap = new cPointerHashMap;

			/// 새로 만든 스킬리스트를 기록
			if( !( mPlayerHaveSkillMap.Insert( playerIdx, pPlayerHaveSkillMap ) ) )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("AddPlayerHaveSkillCheat[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep );
				SAFE_DELETE( pPlayerHaveSkillMap );
				return ERROR_CHEAT_SKILL_ADD_ERR_MAPINSERT;
			}
		}

		/// 보유스킬리스트 정보를 기록
		cHaveSkillObject* pPlayerHaveSkill = new cHaveSkillObject;
		pPlayerHaveSkill->Init( skillIdx, skillStep, cooltime );

		if( !(pPlayerHaveSkillMap->Insert( skillIdx, pPlayerHaveSkill ) ) )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("AddPlayerHaveSkillCheat[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep  );
			SAFE_DELETE( pPlayerHaveSkill );
			return ERROR_CHEAT_SKILL_ADD_ERR_MAPINSERT;
		}
	}
	else	/// 단계를 업시키는 경우
	{
		pPlayerHaveSkill->StepUpByCheat( skillStep, cooltime );
	}

	/// 패시브 일경우 스텟 재계산
	if( pSkillInfo->mType == eSKILLTYPE_PASSIVE )
	{
		/// 기본항목을 제외한 확장(+,%) 항목만 계산
		STATUSCALC->CalcPlayerExtensionGlobal( pPlayer->GetObject() );
	}

	/// 오라스킬이면 기존 스킬 끔
	if( pSkillInfo->mShotType == eSHOTTYPE_AURA )
		AuraInfluenceOff( playerIdx, skillIdx );

	return ERROR_CHEAT_SKILL_ADD_SUCCESS;
}

void cSkillManager::InsertPlayerSkillMap( unsigned long characterIdx, cBaseSkillObject* pSkillObject )
{
	cArray* pSkillArray = (cArray*)mPlayerSkillMap.GetAt( characterIdx );
	if( pSkillArray == NULL )
	{
		pSkillArray = new cArray;
		mPlayerSkillMap.Insert( characterIdx, pSkillArray );
	}

	pSkillArray->PushBack( pSkillObject );
}



void cSkillManager::InsertMonsterSkillMap( unsigned long monsterIdx, cMonsterSkillObject* pSkillObject )
{
	cArray* pSkillArray = (cArray*)mMonsterSkillMap.GetAt( monsterIdx );
	if( pSkillArray == NULL )
	{
		pSkillArray = new cArray;
		mMonsterSkillMap.Insert( monsterIdx, pSkillArray );
	}

	pSkillArray->PushBack( pSkillObject );
}



cHaveSkillObject* cSkillManager::GetPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx )
{
	cPointerHashMap* cPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
	if ( !cPlayerHaveSkillMap )
	{
		return NULL;
	}

	/// 기본 공격인 경우 LBS추가
	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
		return (cHaveSkillObject*)cPlayerHaveSkillMap->GetAt( NORMAL_ATTACK_SKILL );

	return (cHaveSkillObject*)cPlayerHaveSkillMap->GetAt( skillIdx );
}



void* cSkillManager::GetPlayerHaveSkill( unsigned long playerIdx )
{
	return mPlayerHaveSkillMap.GetAt( playerIdx );
}


bool cSkillManager::DelPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx )
{
	/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
	cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
	/// 보유한 스킬리스트가 없으면
	if ( !pPlayerHaveSkillMap )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cSkillManager::DelPlayerHaveSkill pPlayer[%d]HaveSkillMap == NULL", playerIdx );
		return false;
	}

	if( !(pPlayerHaveSkillMap->Erase( skillIdx ) ) )
	{
		return false;
	}

	return true;
}


void cSkillManager::RollBackPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx )
{
	cHaveSkillObject* pHaveSkillObject = GetPlayerHaveSkill( playerIdx, skillIdx );
	if( !pHaveSkillObject ) 
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::RollBackPlayerHaveSkill pHaveSkillObject[%d,%d] == NULL", playerIdx, skillIdx );
		return; 
	}

	if( pHaveSkillObject->GetSkillStep() == SKILL_START_STEP )
	{
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
		if( !pPlayerHaveSkillMap )	
		{ 
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::RollBackPlayerHaveSkill pPlayerHaveSkillMap[%d] == NULL", playerIdx );
			return; 
		}

		SAFE_DELETE( pHaveSkillObject );
		pPlayerHaveSkillMap->Erase( skillIdx );
	}
	else
	{
		/// 스텝 한단계 감소
		pHaveSkillObject->StepDown();
	}
}


void cSkillManager::ReleasePlayerHaveSkill()
{
	cPointerHashMap::cIterator startPlayer = mPlayerHaveSkillMap.Begin();
	cPointerHashMap::cIterator endPlayer = mPlayerHaveSkillMap.End();

	for( ; startPlayer != endPlayer ; ++startPlayer )
	{
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)(*startPlayer).mSecond;
		if ( !pPlayerHaveSkillMap )		{ continue; }

		cPointerHashMap::cIterator startSkill = pPlayerHaveSkillMap->Begin();
		cPointerHashMap::cIterator endSkill = pPlayerHaveSkillMap->End();

		cHaveSkillObject* p;
		for( ; startSkill != endSkill ; ++startSkill )
		{
			p = (cHaveSkillObject*)(*startSkill).mSecond;
			SAFE_DELETE( p );
		}
		SAFE_DELETE( pPlayerHaveSkillMap );
	}

	mPlayerHaveSkillMap.Clear();
}



void cSkillManager::ReleaseInfluence()
{
	cPointerHashMap::cIterator startInfluence = mInfluenceMap.Begin();
	cPointerHashMap::cIterator endInfluence = mInfluenceMap.End();

	cInfluenceObject* pInfluenceObject;
	for( ; startInfluence != endInfluence ; ++startInfluence )
	{
		pInfluenceObject = (cInfluenceObject*)(*startInfluence).mSecond;
		SAFE_DELETE( pInfluenceObject );
	}

	mInfluenceMap.Clear();
}


bool cSkillManager::SaveAllPlayerInfluence( unsigned long playerIdx, SKILL_INFLUENCE_UPDATE* pInfluenceUpdate )
{

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SavePlayerInfluence pPlayer[%d] == NULL", playerIdx );
		return false; 
	}

	tHashSet<unsigned long>* pInfluenceAry = pPlayer->GetInfluenceSet();
	if( !pInfluenceAry )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SavePlayerInfluence pInfluenceAry == NULL" );
		return false; 
	}

	cInfluenceObject* pInfluenceObject = NULL;

	tHashSet<unsigned long>::cIterator start = pInfluenceAry->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceAry->End();

	/// 로그로 발송할 메세지 생성
	HANDLE handle = NULL;
	TB_CHARACTER_INFLUENCE* pLogTable = NULL;
	SKILL_INFLUENCE_UPDATE* pLogMsg = (SKILL_INFLUENCE_UPDATE*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ) );
	if( pLogMsg != NULL )
	{ 
		pLogMsg->mCharacterIdx = playerIdx;
		pLogMsg->mCycleSave = false;
		pLogMsg->mRetvalue = -1;
		pLogMsg->mRowCount = 0;
		pLogTable = pLogMsg->mTable;
	}
	else
		NETWORK2->PostServerEvent("cSkillManager::SaveAllPlayerInfluence pLogMsg = NULL" );

	// 08/03/11 수정
	TB_CHARACTER_INFLUENCE* table = pInfluenceUpdate->mTable;	// long i = 0;
	for( ; start != end ; ++start )
	{
		pInfluenceObject = GetInfluence( (*start) );
		if( !pInfluenceObject ) 
		{ 
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::SavePlayerInfluence pInfluenceObject[%d] == NULL", (*start) );
			//-/-//InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		if( pInfluenceObject->IsSavePlayerInfluence( false ) == false )
			continue;

		if( pLogMsg != NULL && pLogTable != NULL )
		{
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo(  pInfluenceObject->GetInfluenceClassIdx() );
			if( pScript != NULL && pScript->mType == eINFLUENCETYPE_CASH )
			{
				pInfluenceObject->SavePlayerInfluence( pLogTable );
				++pLogMsg->mRowCount;
				++pLogTable;
			}
		}

		// 08/03/11 수정
		pInfluenceObject->SavePlayerInfluence( table );		// pInfluenceObject->SavePlayerInfluence( &pInfluenceUpdate->mTable[i] );
		table++;											// i++;

		pInfluenceUpdate->mRowCount++;
	}
	// 08/03/11 수정
	// pInfluenceUpdate->mRowCount = i;
	pInfluenceUpdate->mCharacterIdx = playerIdx;
	pInfluenceUpdate->mCycleSave = false;
	pPlayer->SetCashInfSave( true );

	/// 로그기록
	unsigned long releaseSize = 0;
	if( pLogMsg != NULL )
	{
		if( pLogMsg->mRowCount > 0 )
			NETWORK2->PostInfCashEvent( pPlayer->GetObjectID(), EVENT_INFCASH_LOGOUT, pLogMsg );

		releaseSize = sizeof(SKILL_INFLUENCE_UPDATE) - sizeof(pLogMsg->mTable) + ( pLogMsg->mRowCount * sizeof(TB_CHARACTER_INFLUENCE) );
		NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, releaseSize );
	}

	return true;
}


void cSkillManager::SavePlayerInfluence( unsigned long playerIdx, unsigned long infObjectIdx, bool del )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;

	cInfluenceObject* pInf = GetInfluence( infObjectIdx );
	if( pInf == NULL )
	{
		NETWORK2->PostServerEvent("cSkillManager::SavePlayerInfluence pInfluenceObject[%d] == NULL", playerIdx );
		return;
	}

	if( pInf->IsSavePlayerInfluence( false ) == false )
		return;

	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
	if( pScript == NULL )
		return;

	HANDLE                  handle          = NULL;
	SKILL_INFLUENCE_UPDATE* pUpdate         = (SKILL_INFLUENCE_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INFLUENCE_UPDATE );
	unsigned long           length          = sizeof(SKILL_INFLUENCE_UPDATE) - sizeof(pUpdate->mTable);

	pInf->SavePlayerInfluence( pUpdate->mTable );
	pUpdate->mTable[0].mDel = del == true ? 1 : 0;
	pUpdate->mRowCount = 1;
	pUpdate->mCharacterIdx = playerIdx;
	pUpdate->mCycleSave = false;
	pPlayer->SetCashInfSave( true );

	char logType = EVENT_INFCASH_INSERT;
	if( del == true )
		logType = EVENT_INFCASH_DELETE;

	/// 로그기록
	if( pScript->mType == eINFLUENCETYPE_CASH )
		NETWORK2->PostInfCashEvent( pPlayer->GetObjectID(), logType, pUpdate );

	length += pUpdate->mRowCount * sizeof(TB_CHARACTER_INFLUENCE);
	NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, length );	
}


void cSkillManager::CycleSaveCashInfluence( cPlayer* pPlayer )
{
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::CycleSaveCashInfluence pPlayer[] == NULL" );
		return; 
	}

	tHashSet<unsigned long>* pInfluenceAry = pPlayer->GetInfluenceSet();
	if( !pInfluenceAry )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::CycleSaveCashInfluence pInfluenceAry == NULL" );
		return; 
	}

	tHashSet<unsigned long>::cIterator start = pInfluenceAry->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceAry->End();


	HANDLE                  handle          = NULL;
	SKILL_INFLUENCE_UPDATE* pUpdate         = (SKILL_INFLUENCE_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_INFLUENCE_UPDATE );
	unsigned long           length          = sizeof(SKILL_INFLUENCE_UPDATE) - sizeof(pUpdate->mTable);
	pUpdate->mCharacterIdx = pPlayer->GetObjectID();
	pUpdate->mCycleSave = true;

	// 08/03/11 수정
	TB_CHARACTER_INFLUENCE* table = pUpdate->mTable;
	for( ; start != end ; ++start )
	{
		cInfluenceObject* pInf = GetInfluence( (*start) );
		if( !pInf ) 
		{ 
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::SavePlayerInfluence pInfluenceObject[%d] == NULL", (*start) );
			//-/-//InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		if( pInf->IsSavePlayerInfluence( true ) == false )
			continue;

		if( pInf->IsDelReady() == true )
			continue;

		// 08/03/11 수정
		pInf->SavePlayerInfluence( table );
		table++;

		pUpdate->mRowCount++;
	}

	if( pUpdate->mRowCount == 0 )
	{
		NETWORK2->ReleaseSQL( handle, length );
		return;
	}

	/// 로그기록
	NETWORK2->PostInfCashEvent( pPlayer->GetObjectID(), EVENT_INFCASH_CYCLE, pUpdate );

	pPlayer->SetCashInfSaveCycle( true );
	length += pUpdate->mRowCount * sizeof(TB_CHARACTER_INFLUENCE);
	NETWORK2->SendSQL( pPlayer->GetConnectionIdx(), handle, length );	

	return;
}


void cSkillManager::RestorePlayerInfluence( unsigned long playerIdx, SKILL_INFLUENCE_SELECT* pInfluenceSelect, unsigned long rowCnt )
{

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !pPlayer ) 
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::RestorePlayerInfluence pPlayer[%d] == NULL", playerIdx );
		return; 
	}

	unsigned long uniqueIdx = 0;
	cInfluenceObject* pInf = NULL;
	for( unsigned long i = 0 ; i < rowCnt ; ++i )
	{
		sInfluence* pInfluence = &pInfluenceSelect->mTable[i];
		if( pInfluence == NULL )
			continue;

		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInfluence->mInfluenceClassIdx );
		if( pScript == NULL )
			continue;

		/// 신규 생성
		pInf = new cInfluenceObject;
		uniqueIdx = mInfluenceGen.GeneratIdx();
		if( pInf->RestorePlayerInfluence( playerIdx, uniqueIdx, pInfluence ) == false )
		{
			assert(NULL);
			SAFE_DELETE( pInf );
			continue;
		}

		if( !( mInfluenceMap.Insert( uniqueIdx, pInf ) ) )
		{
			assert(NULL);
			SAFE_DELETE( pInf );
			NETWORK2->PostServerEvent( "cSkillManager::RestorePlayerInfluence mInfluenceMap.Insert Duplicate" );
			continue;
		}

		//if( pPlayer->AddInfluence( uniqueIdx, pInf->GetInfluenceClassIdx() ) == false )
		if( pPlayer->AddInfluence( pInf ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent( "cSkillManager::RestorePlayer[%d]Influence[%d] pPlayer->AddSkillInfluence[%d] == false", playerIdx, uniqueIdx, pInf->GetInfluenceClassIdx() );
			NETWORK2->PostServerEvent( "cSkillManager::RestorePlayer[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",
			pInfluence->mUniqueIdx,
			pInfluence->mDbIdx,
			pInfluence->mInfluenceClassIdx,
			pInfluence->mAttacker.type,
			pInfluence->mAttacker.index,
			pInfluence->mIsRealTime,
			pInfluence->mRestTime,
			pInfluence->mTime1,
			pInfluence->mTime2,
			pInfluence->mInfluenceValue1,
			pInfluence->mInfluenceValue2 );
			continue;
		}
	}
}


void cSkillManager::ReleasePlayerHaveSkill( unsigned long playerIdx )
{
	/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
	cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
	/// 보유한 스킬리스트가 없으면
	if ( !pPlayerHaveSkillMap )	{ return; }

	cPointerHashMap::cIterator startSkill = pPlayerHaveSkillMap->Begin();
	cPointerHashMap::cIterator endSkill = pPlayerHaveSkillMap->End();

	cHaveSkillObject* p = NULL;
	for( ; startSkill != endSkill ; ++startSkill )
	{
		p = (cHaveSkillObject*)(*startSkill).mSecond;
		SAFE_DELETE( p );
	}
	SAFE_DELETE( pPlayerHaveSkillMap );

	mPlayerHaveSkillMap.Erase( playerIdx );
}


bool cSkillManager::AddInfluence( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, bool msgSend, sInfluenceCreate* pInf )
{
	//////////////////////////////////////////////////////////////////////////
	/// 참조 데이타 로드
	
	/// 신규효과 스크립트
	sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pNewScript == NULL ) 	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewScript[%d]==NULL", infClassIdx );
		return false; 
	}
	const int colNum = 3;
	long type[colNum] = { pNewScript->mStatusPlusIdx1, pNewScript->mStatusPlusIdx2, pNewScript->mStatusPlusIdx3 };
	long value[colNum] = { pNewScript->mValue1, pNewScript->mValue2, pNewScript->mValue3 };

	/// 공격자, 타겟 포인터
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pAttacker[%d,%d]==NULL", attacker.type, attacker.index );
		return false; 
	}

	if( pAttacker->GetStateDie() == true )
		return false;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( !pTarget )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pTarget[%d,%d]==NULL", target.type, target.index );
		return false; 
	}

	// 오토 더미 체크
	if( target.type == eOBJECTTYPE_MONSTER )
	{
		cMonster* mon = (cMonster*)pTarget;
		if( mon->IsAutoDummy() == true )
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 발동 확률 체크부
	unsigned long skillRate = pNewScript->mActivityPer;
	if( skillRate != 0 )
	{
		/// 확률 값
		unsigned long checkPer = skillRate;

		if( skillClassIdx != 0 )
		{
			/// 공격자와 타겟간의 상태이상 공식을 적용 해야 하는지 유무
			bool isBadInfluence = false;
			for( int i = 0 ; i < colNum ; ++i )
			{
				if( eSTATUSPLUS_DISPEL <= type[i] && type[i] <= eSTATUSPLUS_CONDITIONINDISPOSITION_MAX )
				{
					isBadInfluence = true;
					break;
				}
			}

			/// 상태 이상 확률값 산출
			if( isBadInfluence == true )
				checkPer = pAttacker->ConditionOdd( target, skillRate, skillClassIdx );
		}

		/// 확률 체크
		unsigned long activityRand = rand() % 100;
		if( checkPer <= activityRand )	
			return false; 
	}

	bool EndVehicle = false;
	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성 체크부
	bool createInfluence = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		/// 면역체크
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			/// 면역 체크
			if( ((cMonster*)pTarget)->IsImmuneApplyType( pNewScript->mType, type[i] ) == true )
				return false;

			if( ((cMonster*)pTarget)->IsImmuneKind( pNewScript->mTypeDetail ) == true )
				return false;

			if( eSTATUSPLUS_CHANGE_MONSTER == type[i] )
				return false;
		}
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;

			/// 탈것 탄상태에선 몬스터 변신 불가
			if( type[i] == eSTATUSPLUS_CHANGE_MONSTER )
			{
				if( pPlayer->IsVehicle() == true )
					EndVehicle = true;

				if( (pPlayer->GetState() == eOBJECT_STATE_IDLE || pPlayer->GetState() == ePLAYER_STATE_ITEMPICK ) == false )
					return false;
			}
			else if( type[i] == eSTATUSPLUS_VEHICLE )
			{
				/// 점프중에 탈것타는 경우 효과생성(탈것타기)불가
				if( pPlayer->GetJumpEndTime() > NETWORK2->GetAccumTime() )
					return false;
			}

			/// 면역 체크
			if( pPlayer->IsImmuneApplyType( pNewScript->mType, type[i] ) == true )
				return false;

			if( pPlayer->IsImmuneKind( pNewScript->mTypeDetail ) == true )
				return false;
		}

		/// 스킬에서만 사용 - 기입 에러
		else if( eSTATUSPLUS_INSTANTPERHIT <= type[i] && type[i] <= eSTATUSPLUS_DELETE_TARGETING )
		{
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence type[%d,%d] error", infClassIdx, type[i] );
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 효과 객체를 생성하는지 체크
		if( ( eSTATUSPLUS_STR_PLUS <= type[i] && type[i] <= eSTATUSPLUS_BASE_MAX ) ||
//			( eSTATUSPLUS_STR_PERCENT <= type && type <= eSTATUSPLUS_BASE_PERCENT_MAX ) || // 아직 %옵션이 적용되는 시점이 아님 [7/13/2010 JO_CLIENT]
			( eSTATUSPLUS_BASEALL_PLUS == type[i] ) ||
			( eSTATUSPLUS_ATK_PLUS <= type[i] && type[i] <= eSTATUSPLUS_BUFF_ETC_MAX ) ||
			( eSTATUSPLUS_DOT_HPCHANGE_PLUS <= type[i] && type[i] <= eSTATUSPLUS_DOT_MAX ) ||
			( eSTATUSPLUS_STOPMOVE <= type[i] && type[i] <= eSTATUSPLUS_CONDITIONINDISPOSITION_MAX ) ||
			eSTATUSPLUS_CHAR_SIZE == type[i] ||
			eSTATUSPLUS_ATTACKERTARGETING == type[i] || eSTATUSPLUS_CASHEXPGUARDCNT == type[i] ||
			( eSTATUSPLUS_NEED_STATE_ICE <= type[i] && type[i] <= eSTATUSPLUS_NEED_STATE_MAX ) )
		{
			createInfluence = true;
		}
	}

	/// 이펙트만 붙이는 효과
	if( type[0] == 0 && type[1] == 0 && type[2] == 0 && pNewScript->mContinuanceTime > 0 )
		createInfluence = true;

	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성부
	unsigned long uniqueIdx = 0;
	if( createInfluence == true )
	{
		cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

		/// 중복/그룹 체크
		tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
		tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();
	
		while( begin != end )
		{
			/// 보유 효과 오브젝트
			cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
			if( !pHaveInf )	
			{ 
				NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL" );
				continue; 
			}
	
			/// 보유 효과의 스크립트 정보
			sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
			if( !pHaveInf )	
			{ 
				NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL" );
				continue; 
			}

			bool notOverLap = false;

			/// 중복 불가인지 체크
			if( pNewScript->mNotOverlapGroupIdx != 0 )
			{
				/// 기본 중복불가 그룹 체크 같은경우면 걸수 없다.
				if( pHaveScript->mNotOverlapGroupIdx == pNewScript->mNotOverlapGroupIdx )
					notOverLap = true;

				if( notOverLap == false )
				{
					if( pNewScript->mOverlapScriptIdx != 0 )
					{
						if( SKILLSCRIPT->IsInfOverLap( pNewScript->mOverlapScriptIdx, pHaveScript->mNotOverlapGroupIdx ) == true )
							notOverLap = true;
					}
				}

				if( notOverLap == false )
				{
					if( pHaveScript->mOverlapScriptIdx != 0 )
					{
						if( SKILLSCRIPT->IsInfOverLap( pHaveScript->mOverlapScriptIdx, pNewScript->mNotOverlapGroupIdx ) == true )
							notOverLap = true;
					}
				}
			}			

			/// 버프종류별 우선순위 : 캐시 > PC > 버프,디버프
			/// 같은 [중복불가그룹] 인 경우
			if( notOverLap == true )
			{
				/// 같은 중복불가에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
				if( pHaveScript->mType == eINFLUENCETYPE_CASH )
					return false;

				/// 일반버프가 보유중인 pc방버프를 없엘수 없다
				if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pNewScript->mType == eINFLUENCETYPE_BUF || pNewScript->mType == eINFLUENCETYPE_DEBUF ) )
					return false;

				/// pc방 버프가 중복불가 그룹으로 같이 묶일수 없다.
				if( pHaveScript->mType == eINFLUENCETYPE_PC && pNewScript->mType == eINFLUENCETYPE_PC )
					return false;

				if( pNewScript->mType == eINFLUENCETYPE_CASH || pNewScript->mType == eINFLUENCETYPE_PC )
				{
					pHaveInf->NoStatusCalc();
					DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
					break;
				}

				/// [그룹 번호] 비교
				if( pNewScript->mGroupIdx != 0 )
				{
					if( pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
					{
						/// [그룹효력값] 비교 
						if( pNewScript->mGroupOrder >= pHaveScript->mGroupOrder || pNewScript->mType == eINFLUENCETYPE_PC || pNewScript->mType == eINFLUENCETYPE_CASH )
						{
							pHaveInf->NoStatusCalc();
							/// 기존이 작으면 기존 삭제
							DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
							break;
						}
						else	/// 기존이 큰경우 신규 생성안하고 종료
							return false;
					}
					else	/// [그룹 번호]가 다르면
					{
						pHaveInf->NoStatusCalc();
						/// 기존 효과 제거					
						DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
						break;
					}
				}
			}
			else	/// 다른 [중복불가그룹]인 경우
			{
				/// [그룹번호] 비교
				if( pNewScript->mGroupIdx != 0 && pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
				{
					/// 같은 그룹에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
					if( pHaveScript->mType == eINFLUENCETYPE_CASH )
						return false;

					/// 일반버프가 보유중인 pc방버프를 없엘수 없다
					if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pNewScript->mType == eINFLUENCETYPE_BUF || pNewScript->mType == eINFLUENCETYPE_DEBUF ) )
						return false;

					/// pc방 버프가 그룹으로 같이 묶일수 없다.
					if( pHaveScript->mType == eINFLUENCETYPE_PC && pNewScript->mType == eINFLUENCETYPE_PC )
						return false;

					if( pNewScript->mType == eINFLUENCETYPE_CASH || pNewScript->mType == eINFLUENCETYPE_PC )
					{
						pHaveInf->NoStatusCalc();
						DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
						break;
					}

					/// [그룹효력값] 비교
					if( pNewScript->mGroupOrder >= pHaveScript->mGroupOrder )
					{
						pHaveInf->NoStatusCalc();
						/// 기존이 작으면 기존 삭제
						DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
						break;
					}
					else	/// 기존이 큰경우 신규 생성안하고 종료
						return false;
				}
			}
		}

		/// 생성
		unsigned long plusTime = 0;
		cInfluenceObject* pNewObject = new cInfluenceObject;
		uniqueIdx = mInfluenceGen.GeneratIdx();
		if( !pNewObject->Init( attacker, target, uniqueIdx, infClassIdx, skillClassIdx, eSWITCHTYPE_NORMAL, 0, 0, &plusTime ) )
		{
			SAFE_DELETE( pNewObject );
			return false;
		}

		if( !( mInfluenceMap.Insert( uniqueIdx, pNewObject ) ) )
		{
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence mInfluenceMap duplicate");
			SAFE_DELETE( pNewObject );
			return false;
		}

		/// 타겟 효과 리스트에 추가
		if( pTarget->AddInfluence( pNewObject ) == false )
		{
			NETWORK2->PostServerEvent( "cSkillManager::AddInfluence pTarget[%d,%d]->AddSkillInfluence[%d,%d] == false", 
				target.type, target.index, uniqueIdx, infClassIdx );
			return false;
		}

		if( pNewScript->mType == eINFLUENCETYPE_CASH && target.type == eOBJECTTYPE_PLAYER )
			SKILLMANAGER->SavePlayerInfluence( target.index, uniqueIdx, false );

		if( msgSend == true )
		{
			if( pInf != NULL )
			{
				pInf->mTarget = target;
				pInf->mAttacker = attacker;
				pInf->mUniqueIdx = uniqueIdx;
				pInf->mInfluenceClassIdx = infClassIdx;
				pInf->mAuraSkillClassIdx = 0;
				pInf->mDeleteType = pNewObject->GetDeleteType();
				pInf->mPlusTime = plusTime;
				pInf->mIsRealTime = pNewObject->IsRealTime();
				tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
				time_t tTime = mktime( &tmTime );
				pInf->mRealEndTime = tTime;
			}
			else
			{
				MSG_SYN_INFLUENCE_CREATE msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_INFLUENCE_CREATE_SYN;
				msg.mTarget = target;
				msg.mAttacker = attacker;
				msg.mUniqueIdx = uniqueIdx;
				msg.mInfluenceClassIdx = infClassIdx;
				msg.mAuraSkillClassIdx = 0;
				msg.mDeleteType = pNewObject->GetDeleteType();
				msg.mPlusTime = plusTime;
				msg.mIsRealTime = pNewObject->IsRealTime();
				tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
				time_t tTime = mktime( &tmTime );
				msg.mRealEndTime = tTime;

				NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	/// 적용부
	bool calcStatus = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		if( ApplyInfluence( pAttacker, pTarget, uniqueIdx, infClassIdx, type[i], value[i] ) == true )
			calcStatus = true;
	}


	//////////////////////////////////////////////////////////////////////////
	/// 스텟 계산
	if( calcStatus == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			STATUSCALC->CalcPlayerExtensionGlobal( pTarget->GetObject() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			STATUSCALC->CalcMonster( target.index );
		else
			assert(NULL);
	}

	if( EndVehicle == true )
	{
		((cPlayer*)pTarget)->EndVehicleClient( false );
	}

	return true;
}


unsigned long cSkillManager::AddInfSwitch( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, bool msgSend )
{
	//////////////////////////////////////////////////////////////////////////
	/// 참조 데이타 로드

	/// 신규효과 스크립트
	sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pNewScript == NULL ) 	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewScript[%d]==NULL", infClassIdx );
		return 0; 
	}
	const int colNum = 3;
	long type[colNum] = { pNewScript->mStatusPlusIdx1, pNewScript->mStatusPlusIdx2, pNewScript->mStatusPlusIdx3 };
	long value[colNum] = { pNewScript->mValue1, pNewScript->mValue2, pNewScript->mValue3 };

	/// 공격자, 타겟 포인터
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfSwitch pAttacker[%d,%d]==NULL", attacker.type, attacker.index );
		return 0; 
	}
	
	if( pAttacker->GetStateDie() == true )
		return 0;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( !pTarget )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pTarget[%d,%d]==NULL", target.type, target.index );
		return 0; 
	}


	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성부

	cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();

	/// 자신이 켜둔 오라 먼저 체크
	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL Aura" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL Aura" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 중복체크
		if( pHaveScript->mNotOverlapGroupIdx != 0 &&
			pHaveScript->mNotOverlapGroupIdx == pNewScript->mNotOverlapGroupIdx )
		{				
			/// 공격자도 같으면 효과 제거
			if( pHaveInf->GetAttacker().index == attacker.index &&
				pHaveInf->GetAttacker().type == attacker.type )
			{
				/// 중복 불가 인데 기존 보유효과가 캐시 버프이면 신규효과 생성 안시킴
				if( pHaveScript->mType == eINFLUENCETYPE_CASH )
					return 0;

				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
		}
	}

	begin = pInfluenceSet->Begin();
	end = pInfluenceSet->End();

	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 효력그룹 비교
		if( pNewScript->mGroupIdx != 0 && pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
		{
			/// 같은 중복불가에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_CASH )
				return false;

			/// 일반버프가 보유중인 pc방버프를 없엘수 없다
			if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pNewScript->mType == eINFLUENCETYPE_BUF || pNewScript->mType == eINFLUENCETYPE_DEBUF ) )
				return false;

			/// pc방 버프가 중복불가 그룹으로 같이 묶일수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_PC && pNewScript->mType == eINFLUENCETYPE_PC )
				return false;

			if( pNewScript->mType == eINFLUENCETYPE_CASH || pNewScript->mType == eINFLUENCETYPE_PC )
			{
				pHaveInf->NoStatusCalc();
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}

			/// 신규 가 더 높거나 같으면 신규 생성(기존삭제)
			if( pNewScript->mGroupOrder >= pHaveScript->mGroupOrder )
			{
				pHaveInf->NoStatusCalc();
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
			else
				return 0;
		}
	}

	cInfluenceObject* pNewObject = new cInfluenceObject;
	unsigned long uniqueIdx = mInfluenceGen.GeneratIdx();
	if( !pNewObject->Init( attacker, target, uniqueIdx, infClassIdx, skillClassIdx, eSWITCHTYPE_PARENT, 0, 0, NULL  ) )
	{
		SAFE_DELETE( pNewObject );
		return 0;
	}

	if( !( mInfluenceMap.Insert( uniqueIdx, pNewObject ) ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence mInfluenceMap duplicate");
		SAFE_DELETE( pNewObject );
		return 0;
	}

	/// 타겟 효과 리스트에 추가
	if( pTarget->AddInfluence( pNewObject ) == false )
	{
		NETWORK2->PostServerEvent( "cSkillManager::AddInfluence pTarget[%d,%d]->AddSkillInfluence[%d,%d] == false", 
			target.type, target.index, uniqueIdx, infClassIdx );
		return 0;
	}

	if( msgSend == true )
	{
		MSG_SYN_INFLUENCE_CREATE msg;
		msg.Category = NM_SKILL;
		msg.Protocol = NM_SKILL_INFLUENCE_CREATE_SYN;
		msg.mTarget = target;
		msg.mAttacker = attacker;
		msg.mUniqueIdx = uniqueIdx;
		msg.mInfluenceClassIdx = infClassIdx;
		msg.mAuraSkillClassIdx = skillClassIdx;
		msg.mDeleteType = pNewObject->GetDeleteType();
		msg.mPlusTime = 0;
		msg.mIsRealTime = pNewObject->IsRealTime();
		tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
		time_t tTime = mktime( &tmTime );
		msg.mRealEndTime = tTime;

		NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
	}


	//////////////////////////////////////////////////////////////////////////
	/// 적용부
	bool calcStatus = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		if( ApplyInfluence( pAttacker, pTarget, uniqueIdx, infClassIdx, type[i], value[i] ) == true )
			calcStatus = true;
	}


	//////////////////////////////////////////////////////////////////////////
	/// 스텟 계산
	if( calcStatus == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			STATUSCALC->CalcPlayerExtensionGlobal( pTarget->GetObject() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			STATUSCALC->CalcMonster( target.index );
		else
			assert(NULL);
	}

	return uniqueIdx;
}


bool cSkillManager::AddInfChildAura( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, unsigned long parentIdx, bool msgSend )
{
	//////////////////////////////////////////////////////////////////////////
	/// 참조 데이타 로드

	/// 신규효과 스크립트
	sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pNewScript == NULL ) 	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pNewScript[%d]==NULL", infClassIdx );
		return false; 
	}
	const int colNum = 3;
	long type[colNum] = { pNewScript->mStatusPlusIdx1, pNewScript->mStatusPlusIdx2, pNewScript->mStatusPlusIdx3 };
	long value[colNum] = { pNewScript->mValue1, pNewScript->mValue2, pNewScript->mValue3 };

	/// 공격자, 타겟 포인터
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pAttacker[%d,%d]==NULL", attacker.type, attacker.index );
		return false; 
	}

	if( pAttacker->GetStateDie() == true )
		return false;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( !pTarget )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pTarget[%d,%d]==NULL", target.type, target.index );
		return false; 
	}


	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성부
	cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();
	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pHaveInfluenceObject==NULL" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pHaveInfluenceScript==NULL" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 효력그룹 비교
		if( pNewScript->mGroupIdx != 0 && pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
		{
			/// 같은 중복불가에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_CASH )
				return false;

			/// 일반버프가 보유중인 pc방버프를 없엘수 없다
			if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pNewScript->mType == eINFLUENCETYPE_BUF || pNewScript->mType == eINFLUENCETYPE_DEBUF ) )
				return false;

			/// pc방 버프가 중복불가 그룹으로 같이 묶일수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_PC && pNewScript->mType == eINFLUENCETYPE_PC )
				return false;

			if( pNewScript->mType == eINFLUENCETYPE_CASH || pNewScript->mType == eINFLUENCETYPE_PC )
			{
				pHaveInf->NoStatusCalc();
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}

			/// 신규 가 더 높으면 신규 생성(기존삭제)
			if( pNewScript->mGroupOrder > pHaveScript->mGroupOrder )
			{
				pHaveInf->NoStatusCalc();
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
			else
				return false;
		}
	}

	cInfluenceObject* pNewObject = new cInfluenceObject;
	unsigned long uniqueIdx = mInfluenceGen.GeneratIdx();
	if( !pNewObject->Init( attacker, target, uniqueIdx, infClassIdx, skillClassIdx, eSWITCHTYPE_CHILD, parentIdx, 0, NULL ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura pNewInfluenceObject->Init err");
		SAFE_DELETE( pNewObject );
		return false;
	}

	if( !( mInfluenceMap.Insert( uniqueIdx, pNewObject ) ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfChildAura mInfluenceMap duplicate");
		SAFE_DELETE( pNewObject );
		return false;
	}

	/// 타겟 효과 리스트에 추가
	if( pTarget->AddInfluence( pNewObject ) == false )
	{
		NETWORK2->PostServerEvent( "cSkillManager::AddInfChildAura pTarget[%d,%d]->AddSkillInfluence[%d,%d] == false", 
			target.type, target.index, uniqueIdx, infClassIdx );
		return false;
	}

	if( msgSend == true )
	{
		MSG_SYN_INFLUENCE_CREATE msg;
		msg.Category = NM_SKILL;
		msg.Protocol = NM_SKILL_INFLUENCE_CREATE_SYN;
		msg.mTarget = target;
		msg.mAttacker = attacker;
		msg.mUniqueIdx = uniqueIdx;
		msg.mInfluenceClassIdx = infClassIdx;
		msg.mAuraSkillClassIdx = 0;
		msg.mDeleteType = pNewObject->GetDeleteType();
		msg.mPlusTime = 0;
		msg.mIsRealTime = pNewObject->IsRealTime();
		tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
		time_t tTime = mktime( &tmTime );
		msg.mRealEndTime = tTime;

		NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
	}


	//////////////////////////////////////////////////////////////////////////
	/// 적용부
	bool calcStatus = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		if( ApplyInfluence( pAttacker, pTarget, uniqueIdx, infClassIdx, type[i], value[i] ) == true )
			calcStatus = true;
	}


	//////////////////////////////////////////////////////////////////////////
	/// 스텟 계산
	if( calcStatus == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			STATUSCALC->CalcPlayerExtensionGlobal( pTarget->GetObject() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			STATUSCALC->CalcMonster( target.index );
		else
			assert(NULL);
	}

	return true;
}


bool cSkillManager::AddInfluenceTotem( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, unsigned long parentIdx, 
									  bool msgSend, unsigned long continuanceTime, sInfluenceCreate* pInf )
{
	//////////////////////////////////////////////////////////////////////////
	/// 참조 데이타 로드

	sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pNewScript == NULL ) 	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewScript[%d]==NULL", infClassIdx );
		return false; 
	}
	const int colNum = 3;
	long type[colNum] = { pNewScript->mStatusPlusIdx1, pNewScript->mStatusPlusIdx2, pNewScript->mStatusPlusIdx3 };
	long value[colNum] = { pNewScript->mValue1, pNewScript->mValue2, pNewScript->mValue3 };

	/// 공격자, 타겟 포인터
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
		return false; 

	if( pAttacker->GetStateDie() == true )
		return false;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( !pTarget )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pTarget[%d,%d]==NULL", target.type, target.index );
		return false; 
	}

	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성 체크부
	for( int i = 0 ; i < colNum ; ++i )
	{
		/// 면역체크
		if( target.type == eOBJECTTYPE_MONSTER )
		{
			/// 면역 체크
			if( ((cMonster*)pTarget)->IsImmuneApplyType( pNewScript->mType, type[i] ) == true )
				return false;

			if( ((cMonster*)pTarget)->IsImmuneKind( pNewScript->mTypeDetail ) == true )
				return false;

			if( eSTATUSPLUS_CHANGE_MONSTER == type[i] )
				return false;
		}
		else if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;

			/// 면역 체크
			if( pPlayer->IsImmuneApplyType( pNewScript->mType, type[i] ) == true )
				return false;

			if( pPlayer->IsImmuneKind( pNewScript->mTypeDetail ) == true )
				return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성부
	cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();
	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 효력그룹 비교
		if( pNewScript->mGroupIdx != 0 && pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
		{
			/// 같은 중복불가에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_CASH )
				return false;

			/// 일반버프가 보유중인 pc방버프를 없엘수 없다
			if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pNewScript->mType == eINFLUENCETYPE_BUF || pNewScript->mType == eINFLUENCETYPE_DEBUF ) )
				return false;

			/// pc방 버프가 중복불가 그룹으로 같이 묶일수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_PC && pNewScript->mType == eINFLUENCETYPE_PC )
				return false;

			if( pNewScript->mType == eINFLUENCETYPE_CASH || pNewScript->mType == eINFLUENCETYPE_PC )
			{
				pHaveInf->NoStatusCalc();
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}

			/// 신규 가 더 높으면 신규 생성(기존삭제)
			if( pNewScript->mGroupOrder > pHaveScript->mGroupOrder )
			{
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
			else
				return false;
		}
	}

	cInfluenceObject* pNewObject = new cInfluenceObject;
	unsigned long uniqueIdx = mInfluenceGen.GeneratIdx();
	if( !pNewObject->Init( attacker, target, uniqueIdx, infClassIdx, skillClassIdx, eSWITCHTYPE_TOTEM, parentIdx, continuanceTime, NULL ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewInfluenceObject->Init err");
		SAFE_DELETE( pNewObject );
		return false;
	}

	if( !( mInfluenceMap.Insert( uniqueIdx, pNewObject ) ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence mInfluenceMap duplicate");
		SAFE_DELETE( pNewObject );
		return false;
	}

	/// 타겟 효과 리스트에 추가
	if( pTarget->AddInfluence( pNewObject ) == false )
	{
		NETWORK2->PostServerEvent( "cSkillManager::AddInfluence pTarget[%d,%d]->AddSkillInfluence[%d,%d] == false", 
			target.type, target.index, uniqueIdx, infClassIdx );
		return false;
	}

	if( msgSend == true )
	{
		if( pInf != NULL )
		{
			pInf->mTarget = target;
			pInf->mAttacker = attacker;
			pInf->mUniqueIdx = uniqueIdx;
			pInf->mInfluenceClassIdx = infClassIdx;
			pInf->mAuraSkillClassIdx = 0;
			pInf->mDeleteType = pNewObject->GetDeleteType();
			pInf->mPlusTime = 0;
			pInf->mIsRealTime = pNewObject->IsRealTime();
			tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
			time_t tTime = mktime( &tmTime );
			pInf->mRealEndTime = tTime;

		}
		else
		{
			MSG_SYN_INFLUENCE_CREATE msg;
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_CREATE_SYN;
			msg.mTarget = target;
			msg.mAttacker = attacker;
			msg.mUniqueIdx = uniqueIdx;
			msg.mInfluenceClassIdx = infClassIdx;
			msg.mAuraSkillClassIdx = 0;
			msg.mDeleteType = pNewObject->GetDeleteType();
			msg.mPlusTime = 0;
			msg.mIsRealTime = pNewObject->IsRealTime();
			tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
			time_t tTime = mktime( &tmTime );
			msg.mRealEndTime = tTime;

			NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
		}
	}


	//////////////////////////////////////////////////////////////////////////
	/// 적용부
	bool calcStatus = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		if( ApplyInfluence( pAttacker, pTarget, uniqueIdx, infClassIdx, type[i], value[i] ) == true )
			calcStatus = true;
	}


	//////////////////////////////////////////////////////////////////////////
	/// 스텟 계산
	if( calcStatus == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			STATUSCALC->CalcPlayerExtensionGlobal( pTarget->GetObject() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			STATUSCALC->CalcMonster( target.index );
		else
			assert(NULL);
	}

	return true;
}


unsigned long cSkillManager::AddInfMode( sObject attacker, sObject target, unsigned long infClassIdx, bool enemyApply, 
										unsigned short range, bool msgSend )
{
	//////////////////////////////////////////////////////////////////////////
	/// 참조 데이타 로드

	/// 신규효과 스크립트
	sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pNewScript == NULL ) 	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewScript[%d]==NULL", infClassIdx );
		return 0; 
	}
	const int colNum = 3;
	long type[colNum] = { pNewScript->mStatusPlusIdx1, pNewScript->mStatusPlusIdx2, pNewScript->mStatusPlusIdx3 };
	long value[colNum] = { pNewScript->mValue1, pNewScript->mValue2, pNewScript->mValue3 };

	/// 공격자, 타겟 포인터
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pAttacker == NULL )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfSwitch pAttacker[%d,%d]==NULL", attacker.type, attacker.index );
		return 0; 
	}

	if( pAttacker->GetStateDie() == true )
		return 0;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( !pTarget )	
	{ 
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pTarget[%d,%d]==NULL", target.type, target.index );
		return 0; 
	}


	//////////////////////////////////////////////////////////////////////////
	/// 효과 객체 생성부

	cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();

	/// 자신이 켜둔 오라 먼저 체크
	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL Aura" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL Aura" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 중복체크
		if( pHaveScript->mNotOverlapGroupIdx != 0 &&
			pHaveScript->mNotOverlapGroupIdx == pNewScript->mNotOverlapGroupIdx )
		{				
			/// 공격자도 같으면 효과 제거
			if( pHaveInf->GetAttacker().index == attacker.index &&
				pHaveInf->GetAttacker().type == attacker.type )
			{
				/// 중복 불가 인데 기존 보유효과가 캐시 버프이면 신규효과 생성 안시킴
				if( pHaveScript->mType == eINFLUENCETYPE_CASH )
					return 0;

				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
		}
	}

	begin = pInfluenceSet->Begin();
	end = pInfluenceSet->End();

	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceObject==NULL" );
			continue; 
		}

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveScript )	
		{ 
			NETWORK2->PostServerEvent("cSkillManager::AddInfluence pHaveInfluenceScript==NULL" );
			continue; 
		}

		/// 보유하고 있는 효과와 신규 효과의 효력그룹 비교
		if( pNewScript->mGroupIdx != 0 && pNewScript->mGroupIdx == pHaveScript->mGroupIdx )
		{
			/// 신규 가 더 높거나 같으면 신규 생성(기존삭제)
			if( pNewScript->mGroupOrder >= pHaveScript->mGroupOrder )
			{
				DeleteInfluence( target, pHaveInf->GetUniqueIdx() );
				break;
			}
			else
				return 0;
		}
	}

	cInfluenceObject* pNewObject = new cInfluenceObject;
	unsigned long uniqueIdx = mInfluenceGen.GeneratIdx();
	if( !pNewObject->Init( attacker, target, uniqueIdx, infClassIdx, 0, eSWITCHTYPE_PARENT, 0, 0, NULL ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence pNewInfluenceObject->Init err");
		SAFE_DELETE( pNewObject );
		return 0;
	}
	if( enemyApply == true )
		pNewObject->AddModeType( eAPPLYTYPE_ENEMY, range );
	else
		pNewObject->AddModeType( eAPPLYTYPE_BUDDY, range );

	if( !( mInfluenceMap.Insert( uniqueIdx, pNewObject ) ) )
	{
		NETWORK2->PostServerEvent("cSkillManager::AddInfluence mInfluenceMap duplicate");
		SAFE_DELETE( pNewObject );
		return 0;
	}

	/// 타겟 효과 리스트에 추가
	if( pTarget->AddInfluence( pNewObject ) == false )
	{
		NETWORK2->PostServerEvent( "cSkillManager::AddInfluence pTarget[%d,%d]->AddSkillInfluence[%d,%d] == false", 
			target.type, target.index, uniqueIdx, infClassIdx );
		return 0;
	}

	if( msgSend == true )
	{
		MSG_SYN_INFLUENCE_CREATE msg;
		msg.Category = NM_SKILL;
		msg.Protocol = NM_SKILL_INFLUENCE_CREATE_SYN;
		msg.mTarget = target;
		msg.mAttacker = attacker;
		msg.mUniqueIdx = uniqueIdx;
		msg.mInfluenceClassIdx = infClassIdx;
		msg.mAuraSkillClassIdx = 0;
		msg.mDeleteType = pNewObject->GetDeleteType();
		msg.mPlusTime = 0;
		msg.mIsRealTime = pNewObject->IsRealTime();
		tm tmTime = TIMER->ConvertTStoTM( pNewObject->GetInfRealEndTime() );
		time_t tTime = mktime( &tmTime );
		msg.mRealEndTime = tTime;

		NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
	}


	//////////////////////////////////////////////////////////////////////////
	/// 적용부
	bool calcStatus = false;
	for( int i = 0 ; i < colNum ; ++i )
	{
		if( ApplyInfluence( pAttacker, pTarget, uniqueIdx, infClassIdx, type[i], value[i] ) == true )
			calcStatus = true;
	}


	//////////////////////////////////////////////////////////////////////////
	/// 스텟 계산
	if( calcStatus == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			STATUSCALC->CalcPlayerExtensionGlobal( pTarget->GetObject() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			STATUSCALC->CalcMonster( target.index );
		else
			assert(NULL);
	}

	return uniqueIdx;
}


int cSkillManager::ItemCheckAddInf( cBaseObject* pAttacker, cBaseObject* pTarget, unsigned long infClassIdx )
{
	if( pAttacker == NULL )
	{
		NETWORK2->PostServerEvent("cSkillManager::ItemCheckAddInf pAttacker == NULL");
		return ERROR_ITEM_USE_SKILL_SYSTEM;
	}

	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent("cSkillManager::ItemCheckAddInf pTarget == NULL");
		return ERROR_ITEM_USE_SKILL_SYSTEM;
	}

	if( pAttacker->GetStateDie() == true )
	{
		NETWORK2->PostServerEvent("cSkillManager::ItemCheckAddInf pAttacker->GetStateDie() == true");
		return ERROR_ITEM_USE_SKILL_SYSTEM;
	}

	const int colNum = 3;
	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pScript == NULL )
	{
		NETWORK2->PostServerEvent("cSkillManager::ItemCheckAddInf pScript == NULL[%d]", infClassIdx);
		return ERROR_ITEM_USE_SKILL_SYSTEM;
	}

	long type[colNum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };
	long value[colNum] = { pScript->mValue1, pScript->mValue2, pScript->mValue3 };

	for( int i = 0 ; i < colNum ; ++i )
	{
		/// 죽었을때만 가능한 효과 체크
		if( pTarget->GetStateDie() == true )
		{
			switch( type[i] )
			{
			case eSTATUSPLUS_RESURRECTION_HP_PLUS:
			case eSTATUSPLUS_RESURRECTION_HP_PER:
			case eSTATUSPLUS_RESURRECTION_MP_PLUS:
			case eSTATUSPLUS_RESURRECTION_MP_PER:
				if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER || value[i] <= 0 )
					return ERROR_ITEM_USE_SKILL_STATE;
			default:
				return ERROR_ITEM_USE_SKILL_STATE;
			}
		}

		/// 정상적인 상태일때 효과 체크
		switch( type[i] )
		{
		case eSTATUSPLUS_CHANGE_MONSTER:
			if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pPlayer = (cPlayer*)pTarget;
				if( !(pPlayer->GetState() == eOBJECT_STATE_IDLE || pPlayer->GetState() == ePLAYER_STATE_ITEMPICK ) )
					return ERROR_ITEM_USE_SKILL_STATE;

				if( pPlayer->GetJumpEndTime() > NETWORK2->GetAccumTime() )
					return ERROR_ITEM_USE_SKILL_STATE;
				/// 
				//if( pPlayer->IsVehicle() == true )
				//{
				//	return false;
				//}
			}
			break;
		}
	}

	cHashSet* pInfluenceSet = pTarget->GetInfluenceSet();

	/// 중복/그룹 체크
	tHashSet<unsigned long>::cIterator begin = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();

	while( begin != end )
	{
		/// 보유 효과 오브젝트
		cInfluenceObject* pHaveInf = GetInfluence( *begin++ );
		if( !pHaveInf )	
			continue; 

		/// 보유 효과의 스크립트 정보
		sInfluenceScript* pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveInf->GetInfluenceClassIdx() );
		if( !pHaveInf )	
			continue;

		bool notOverLap = false;

		/// 중복 불가인지 체크
		if( pScript->mNotOverlapGroupIdx != 0 )
		{
			/// 기본 중복불가 그룹 체크 같은경우면 걸수 없다.
			if( pHaveScript->mNotOverlapGroupIdx == pScript->mNotOverlapGroupIdx )
				notOverLap = true;

			if( notOverLap == false )
			{
				if( pScript->mOverlapScriptIdx != 0 )
				{
					if( SKILLSCRIPT->IsInfOverLap( pScript->mOverlapScriptIdx, pHaveScript->mNotOverlapGroupIdx ) == true )
						notOverLap = true;
				}
			}

			if( notOverLap == false )
			{
				if( pHaveScript->mOverlapScriptIdx != 0 )
				{
					if( SKILLSCRIPT->IsInfOverLap( pHaveScript->mOverlapScriptIdx, pScript->mNotOverlapGroupIdx ) == true )
						notOverLap = true;
				}
			}
		}	

		/// 버프종류별 우선순위 : 캐시 > PC > 버프,디버프
		/// 같은 [중복불가그룹] 인 경우
		if( notOverLap == true )
		{
			/// 같은 중복불가에 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_CASH )
			{
				if( pScript->mType == eINFLUENCETYPE_CASH )
					return ERROR_ITEM_USE_SKILL_DUPLICATE;
				else
					return ERROR_ITEM_USE_SKILL_GROUPORDER;
			}

			/// 일반버프가 보유중인 pc방버프를 없엘수 없다
			if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pScript->mType == eINFLUENCETYPE_BUF || pScript->mType == eINFLUENCETYPE_DEBUF ) )
				return ERROR_ITEM_USE_SKILL_PCOVER;

			/// pc방 버프가 중복불가 그룹으로 같이 묶일수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_PC && pScript->mType == eINFLUENCETYPE_PC )
				return ERROR_ITEM_USE_SKILL_PCSCRIPT;
		}
		
		/// 보유하고 있는 효과와 신규 효과의 효력그룹 비교
		if( pScript->mGroupIdx != 0 && pScript->mGroupIdx == pHaveScript->mGroupIdx )
		{
			/// 같은 이미 걸려있는 버프가 케시 타입이면 새로 걸수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_CASH )
			{
				if( pScript->mType == eINFLUENCETYPE_CASH )
					return ERROR_ITEM_USE_SKILL_DUPLICATE;
				else
					return ERROR_ITEM_USE_SKILL_GROUPORDER;
			}

			/// 일반버프가 보유중인 pc방버프를 없엘수 없다
			if( pHaveScript->mType == eINFLUENCETYPE_PC && ( pScript->mType == eINFLUENCETYPE_BUF || pScript->mType == eINFLUENCETYPE_DEBUF ) )
				return ERROR_ITEM_USE_SKILL_PCOVER;

			/// pc방 버프가 동일그룹으로 같이 묶일수 없다.
			if( pHaveScript->mType == eINFLUENCETYPE_PC && pScript->mType == eINFLUENCETYPE_PC )
				return ERROR_ITEM_USE_SKILL_PCSCRIPT;

			if( pScript->mType == eINFLUENCETYPE_PC || pScript->mType == eINFLUENCETYPE_CASH )
				return ERROR_ITEM_USE_INVENTORY_SUCCESS;

			/// 신규 가 더 높거나 같으면 신규 생성(기존삭제)
			if( pScript->mGroupOrder < pHaveScript->mGroupOrder )
				return ERROR_ITEM_USE_SKILL_GROUPORDER;
		}
	}

	return ERROR_ITEM_USE_INVENTORY_SUCCESS;
}


bool cSkillManager::ApplyInfluence( cBaseObject* pAttacker, cBaseObject* pTarget, unsigned long infObjectIdx, unsigned long infClassIdx, 
								   unsigned long type, long value, bool noProcessApply )
{

	bool statusCalc = false;

	if( pAttacker == NULL )
		return statusCalc;

	if( pTarget == NULL )
		return statusCalc;

	if( pAttacker->GetStateDie() == true )
		return statusCalc;

	/// 스텟 계산을 하는지만 체크하는 부분
	if( ( eSTATUSPLUS_STR_PLUS <= type && type <= eSTATUSPLUS_BASE_MAX ) ||
		( eSTATUSPLUS_STR_PERCENT <= type && type <= eSTATUSPLUS_BASE_PERCENT_MAX ) ||
		eSTATUSPLUS_BASEALL_PLUS == type ||
		( eSTATUSPLUS_ATK_PLUS <= type && type <= eSTATUSPLUS_BUFF_ETC_MAX ) || 
		eSTATUSPLUS_CHAR_SIZE == type || 
		eSTATUSPLUS_CHANGE_MONSTER == type ||
		( eSTATUSPLUS_STOPMOVE <= type && type <= eSTATUSPLUS_CONDITIONINDISPOSITION_MAX ) )
	{
		statusCalc = true;
	}

	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( pScript == NULL )
		return false;

	/// 프로세스 시점에만 hp/mp 변화를 적용한다.
	if( noProcessApply == false )
	{
		switch( type )
		{
		case eSTATUSPLUS_NONSKILL_HPCHANGE_PLUS:
			{
				bool die = false;

				if( value > 0 )
					pTarget->HPHeal( value, false, pAttacker->GetObject(), 0, eTAKEDAMAGETYPE_POTION );
				else if( value < 0 )
				{
					unsigned long applyDamage = 0;
					if( DUELMANAGER->DuelLastAttack( pAttacker->GetObject(), pTarget->GetObject(), abs(value), eDAMAGE_NORMAL, &applyDamage ) == false )
						pTarget->HPDamage( pAttacker->GetObject(), abs(value), &die, false );						
				}

				MSG_SYN_SKILL_NONSKILL_HPCHANGE msg; 
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_SYN;
				msg.mInfluenceClassIdx = infClassIdx;
				msg.mTarget = pTarget->GetObject();
				msg.mChangeValue = (short)value;

				MSG_SYN_SKILL_NONSKILL_HPCHANGE aroundMsg; 
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = infClassIdx;
				aroundMsg.mTarget = pTarget->GetObject();
				aroundMsg.mChangeValue = (short)value;

				SendHPMPInf( pTarget, pAttacker->GetObject(), (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				InsertChangeSet( pTarget, pAttacker->GetObject(), eDAMAGE_NORMAL );

				//if( pTarget->GetStateDie() == true )
				//{
				//	MSG_SYN_SKILL_APPLY_DIE sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				//	sync.mAttacker = pTarget->GetSkillLastAttack();
				//	sync.mDamageKind = pTarget->GetSkillLastKind();
				//	sync.mTarget = pTarget->GetObject();				
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
				//else
				//{
				//	MSG_SYN_SKILL_APPLY_HPMP sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				//	sync.mTarget = pTarget->GetObject();
				//	sync.mHP = pTarget->GetHP();
				//	sync.mMaxHP = pTarget->GetMaxHP();
				//	sync.mMP = pTarget->GetMP();
				//	sync.mMaxMP = pTarget->GetMaxMP();
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
			}
			break;
		case eSTATUSPLUS_NONSKILL_HPCHANGE_PER:
			{
				bool die = false;

				long applyValue = (long)((long)pTarget->GetMaxHP() * value * 0.01f);

				if( applyValue > 0 )
					pTarget->HPHeal( applyValue, false, pAttacker->GetObject(), 0, eTAKEDAMAGETYPE_POTION );
				else if( applyValue < 0 )
				{
					unsigned long applyDamage = 0;
					if( DUELMANAGER->DuelLastAttack( pAttacker->GetObject(), pAttacker->GetObject(), abs(applyValue), eDAMAGE_NORMAL, &applyDamage ) == false )
						pTarget->HPDamage( pAttacker->GetObject(), abs(applyValue), &die, false );						
				}

				MSG_SYN_SKILL_NONSKILL_HPCHANGE msg; 
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_SYN;
				msg.mInfluenceClassIdx = infClassIdx;
				msg.mTarget = pTarget->GetObject();
				msg.mChangeValue = (short)applyValue;

				MSG_SYN_SKILL_NONSKILL_HPCHANGE aroundMsg; 
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = infClassIdx;
				aroundMsg.mTarget = pTarget->GetObject();
				aroundMsg.mChangeValue = (short)applyValue;

				SendHPMPInf( pTarget, pAttacker->GetObject(), (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				InsertChangeSet( pTarget, pAttacker->GetObject(), eDAMAGE_NORMAL );

				//if( pTarget->GetStateDie() == true )
				//{
				//	MSG_SYN_SKILL_APPLY_DIE sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				//	sync.mAttacker = pTarget->GetSkillLastAttack();
				//	sync.mDamageKind = pTarget->GetSkillLastKind();
				//	sync.mTarget = pTarget->GetObject();				
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
				//else
				//{
				//	MSG_SYN_SKILL_APPLY_HPMP sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				//	sync.mTarget = pTarget->GetObject();
				//	sync.mHP = pTarget->GetHP();
				//	sync.mMaxHP = pTarget->GetMaxHP();
				//	sync.mMP = pTarget->GetMP();
				//	sync.mMaxMP = pTarget->GetMaxMP();
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
			}
			break;
		case eSTATUSPLUS_NONSKILL_MPCHANGE_PLUS:
			{
				if( value > 0 )
					pTarget->MPHeal( value, false );
				else
					pTarget->MPDamage( abs(value), false );

				MSG_SYN_SKILL_NONSKILL_MPCHANGE msg; 
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_SYN;
				msg.mInfluenceClassIdx = infClassIdx;
				msg.mTarget = pTarget->GetObject();
				msg.mChangeValue = (short)value;

				MSG_SYN_SKILL_NONSKILL_MPCHANGE aroundMsg; 
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = infClassIdx;
				aroundMsg.mTarget = pTarget->GetObject();
				aroundMsg.mChangeValue = (short)value;

				SendHPMPInf( pTarget, pAttacker->GetObject(), (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				InsertChangeSet( pTarget, pAttacker->GetObject(), eDAMAGE_NORMAL );

				//if( pTarget->GetStateDie() == true )
				//{
				//	MSG_SYN_SKILL_APPLY_DIE sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				//	sync.mAttacker = pTarget->GetSkillLastAttack();
				//	sync.mDamageKind = pTarget->GetSkillLastKind();
				//	sync.mTarget = pTarget->GetObject();				
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
				//else
				//{
				//	MSG_SYN_SKILL_APPLY_HPMP sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				//	sync.mTarget = pTarget->GetObject();
				//	sync.mHP = pTarget->GetHP();
				//	sync.mMaxHP = pTarget->GetMaxHP();
				//	sync.mMP = pTarget->GetMP();
				//	sync.mMaxMP = pTarget->GetMaxMP();
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
			}
			break;
		case eSTATUSPLUS_NONSKILL_MPCHANGE_PER:
			{
				long applyValue = (long)((long)pTarget->GetMaxMP() * value * 0.01f);

				if( applyValue > 0 )
					pTarget->MPHeal( applyValue, false );
				else if( applyValue < 0 )
					pTarget->MPDamage( abs(applyValue), false );

				MSG_SYN_SKILL_NONSKILL_MPCHANGE msg; 
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_SYN;
				msg.mInfluenceClassIdx = infClassIdx;
				msg.mTarget = pTarget->GetObject();
				msg.mChangeValue = (short)applyValue;

				MSG_SYN_SKILL_NONSKILL_MPCHANGE aroundMsg; 
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = infClassIdx;
				aroundMsg.mTarget = pTarget->GetObject();
				aroundMsg.mChangeValue = (short)applyValue;

				SendHPMPInf( pTarget, pAttacker->GetObject(), (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				InsertChangeSet( pTarget, pAttacker->GetObject(), eDAMAGE_NORMAL );

				//if( pTarget->GetStateDie() == true )
				//{
				//	MSG_SYN_SKILL_APPLY_DIE sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_DIE_SYN;
				//	sync.mAttacker = pTarget->GetSkillLastAttack();
				//	sync.mDamageKind = pTarget->GetSkillLastKind();
				//	sync.mTarget = pTarget->GetObject();				
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
				//else
				//{
				//	MSG_SYN_SKILL_APPLY_HPMP sync;
				//	sync.Category = NM_SKILL;
				//	sync.Protocol = NM_SKILL_DAMAGE_APPLY_HPMP_SYN;
				//	sync.mTarget = pTarget->GetObject();
				//	sync.mHP = pTarget->GetHP();
				//	sync.mMaxHP = pTarget->GetMaxHP();
				//	sync.mMP = pTarget->GetMP();
				//	sync.mMaxMP = pTarget->GetMaxMP();
				//	NETWORK2->QuickSend( pTarget, (char*)&sync, sizeof(sync) );
				//}
			}
			break;
		case eSTATUSPLUS_NONSKILL_DISTRESS_PLUS:
			{
				/// 플레이어가 몬스터에게 쓰는 경우
				if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					cMonster* pMonster = (cMonster*)pTarget;
					if( pMonster )
						pMonster->AddTakeDamage( pAttacker->GetObject(), 0, value, eTAKEDAMAGETYPE_DAMAGE );
				}
				/// 플레이어가 플레이어에게 쓰는 경우(자신이 자신에게 쓰는경우도 포함) [6/1/2010 Jo_Client]
				else if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					typedef tHashSet<unsigned long> cTargetingMonsterSet;
					cTargetingMonsterSet targetingMonsterSet;

					((cPlayer*)pTarget)->GetTargetingMonster( targetingMonsterSet );

					cTargetingMonsterSet::cIterator start = targetingMonsterSet.Begin();
					cTargetingMonsterSet::cIterator end = targetingMonsterSet.End();

					while( start != end )
					{
						unsigned long monsterIdx = (*start++);

						cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
						if ( mon != NULL )
						{
							mon->AddTakeDamage( pTarget->GetObject(), 0, value, eTAKEDAMAGETYPE_DAMAGE );
						}
					}
				}
			}
			break;
		case eSTATUSPLUS_NONSKILL_DISTRESS_PER:
			{
				/// 플레이어가 몬스터에게 쓰는 경우만 체크
				if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					cMonster* pMonster = (cMonster*)pTarget;
					if( pMonster )
						pMonster->AddDistressPer( pAttacker->GetObject(), value, eTAKEDAMAGETYPE_DAMAGE );
				}
				/// 플레이어가 플레이어에게 쓰는 경우(자신이 자신에게 쓰는경우도 포함) [6/1/2010 Jo_Client]
				else if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					typedef tHashSet<unsigned long> cTargetingMonsterSet;
					cTargetingMonsterSet targetingMonsterSet;

					((cPlayer*)pTarget)->GetTargetingMonster( targetingMonsterSet );

					cTargetingMonsterSet::cIterator start = targetingMonsterSet.Begin();
					cTargetingMonsterSet::cIterator end = targetingMonsterSet.End();

					while( start != end )
					{
						unsigned long monsterIdx = (*start++);

						cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
						if ( mon )
						{
							mon->AddDistressPer( pAttacker->GetObject(), value, eTAKEDAMAGETYPE_DAMAGE );
						}
					}
				}
			}
			break;
		}
	}

	switch( type )
	{
	case eSTATUSPLUS_DISPEL:
		SKILLMANAGER->DispelInfluence( pTarget->GetObject(), value, pScript->mGroupOrder );
		break;
	case eSTATUSPLUS_RESURRECTION_HP_PLUS:
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )	/// 플레이어만 부활
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetStateDie() == true && value > 0 )
			{
				/// 부활시 hp회복량 설정
				pPlayer->SkillResurrectionHP( value );

				/// 클라이언트에 부활 요청 메세지 발송
				HANDLE handle = NULL;
				MSG_RES_SKILL_RESURRECTION* sendMsg = (MSG_RES_SKILL_RESURRECTION*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), NM_SKILL, NM_SKILL_RESURRECTION_RES );
				if ( sendMsg != NULL )
				{
					sendMsg->mAttackerIdx = pAttacker->GetObjectID();
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_RESURRECTION) );
				}
			}
		}
		break;
	case eSTATUSPLUS_RESURRECTION_HP_PER:
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )	/// 플레이어인지 체크
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetStateDie() == true && value > 0 && pPlayer->GetMaxHP() > 0)
			{
				long resurrectionValue = pPlayer->GetMaxHP() * value / 100;

				/// 부활시 hp회복량 설정
				pPlayer->SkillResurrectionHP( resurrectionValue );

				/// 클라이언트에 부활 요청 메세지 발송
				HANDLE handle = NULL;
				MSG_RES_SKILL_RESURRECTION* sendMsg = (MSG_RES_SKILL_RESURRECTION*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), NM_SKILL, NM_SKILL_RESURRECTION_RES );
				if ( sendMsg != NULL )
				{
					sendMsg->mAttackerIdx = pAttacker->GetObjectID();
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_SKILL_RESURRECTION) );
				}
			}
		}
		break;
	case eSTATUSPLUS_RESURRECTION_MP_PLUS:
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )	/// 플레이어인지 체크
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetStateDie() == true )
			{
				/// 부활시 mp회복량 설정
				/// ※ SkillResurrectionHP 가 설정된경우만 셋팅됨
				pPlayer->SkillResurrectionMP( value );
			}
		}
		break;
	case eSTATUSPLUS_RESURRECTION_MP_PER:
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )	/// 플레이어인지 체크
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetStateDie() == true && value > 0 && pPlayer->GetMaxMP() > 0)
			{
				long resurrectionValue = pPlayer->GetMaxMP() * value / 100;

				/// 부활시 mp회복량 설정
				/// ※ SkillResurrectionHP 가 설정된경우만 셋팅됨
				pPlayer->SkillResurrectionMP( resurrectionValue );
			}
		}
		break;
	case eSTATUSPLUS_DISPEL_ALL:
		SKILLMANAGER->DispelInfluenceAll( pTarget->GetObject(), pScript->mGroupOrder );
		break;
	case eSTATUSPLUS_MONSTER_ESCAPE:
		if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			((cMonster*)pTarget)->SkillEscape( eSTATUSPLUS_MONSTER_ESCAPE );
		break;
	case eSTATUSPLUS_SWAP_DISTRESS_POINT:
		{
			/// 타겟과 나의 어그로 포인트를 교환한다.
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				typedef tHashSet<unsigned long> cTargetingMonsterSet;
				cTargetingMonsterSet targetingMonsterSet;

				((cPlayer*)pAttacker)->GetTargetingMonster( targetingMonsterSet );
				((cPlayer*)pTarget)->GetTargetingMonster( targetingMonsterSet );
				
				cTargetingMonsterSet::cIterator start = targetingMonsterSet.Begin();
				cTargetingMonsterSet::cIterator end = targetingMonsterSet.End();

				while( start != end )
				{
					unsigned long monsterIdx = (*start++);

					cMonster* mon = OBJECTMANAGER->GetMonster( monsterIdx );
					if ( mon != NULL )
					{
						mon->SwapDistressPoint( pAttacker->GetObjectID(), pTarget->GetObjectID() );
					}
				}
			}
		}
		break;
	case eSTATUSPLUS_CHAR_SIZE:
		break;
	case eSTATUSPLUS_CHANGE_MONSTER:
		{
			if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
				return false;

			cPlayer* pPlayer = (cPlayer*)pTarget;
			pPlayer->SetChgMonster( infObjectIdx, value );
		}
		break;
	case eSTATUSPLUS_VEHICLE:
		{
			if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
				return statusCalc;

			cPlayer* pPlayer = (cPlayer*)pTarget;
			pPlayer->SetVehicle( infObjectIdx, value );
		}
		break;
	case eSTATUSPLUS_ATTACKERTARGETING:
		{
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				cMonster* pMonster = (cMonster*)pTarget;
				pMonster->TargetFixed( pAttacker->GetObjectID(), infObjectIdx );
			}
		}
		break;
	}

	return statusCalc;
}

void cSkillManager::DispelInfluence( sObject target, unsigned long influenceTypeDetail, unsigned long groupOrder )
{
	/// 타겟 포인터 확인
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
		return;

	/// 플레이어의 보유 효과 해쉬셋을 읽어옴
	tHashSet<unsigned long>* pHashSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pHashSet->Begin();
	tHashSet<unsigned long>::cIterator end = pHashSet->End();

	cInfluenceObject* pHaveObejct = NULL;
	sInfluenceScript* pHaveScript = NULL;
	unsigned long uniqueIdx;

	/// 전체 검색
	while( begin != end )
	{
		uniqueIdx = *begin++;
		pHaveObejct = SKILLMANAGER->GetInfluence( uniqueIdx );

		if( pHaveObejct == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("pHaveInfluenceObejct err - unique[%d]", uniqueIdx );
			//-/-//InsertDeleteInfluenceObject( pHaveObejct );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		/// 보유한 효과의 스크립트 정보를 읽어옴
		pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveObejct->GetInfluenceClassIdx() );
		if( pHaveScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("DispelInfluence InfluenceClassIdx err - [%d]", pHaveObejct->GetInfluenceClassIdx() );
			continue;
		}

		/// 효력 그룹 비교
		if( groupOrder < pHaveScript->mGroupOrder )
			continue;

		/// 입력한 디스펠 정보와 보유 효과 정보가 같으면 효과 제거
		if( pHaveScript->mTypeDetail == influenceTypeDetail )
		{
			DeleteInfluence( target, uniqueIdx );
			break;
		}
	}
}


void cSkillManager::DispelInfluenceAll( sObject target, unsigned long groupOrder )
{
	/// 타겟 포인터 확인
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
		return;

	/// 타겟 객체의 보유 효과 해시셋을 읽어옴
	tHashSet<unsigned long>* pHashSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pHashSet->Begin();
	tHashSet<unsigned long>::cIterator end = pHashSet->End();

	cInfluenceObject* pHaveObejct = NULL;
	sInfluenceScript* pHaveScript = NULL;
	unsigned long uniqueIdx;

	/// 전체 검색
	while( begin != end )
	{
		uniqueIdx = *begin++;
		pHaveObejct = SKILLMANAGER->GetInfluence( uniqueIdx );

		if( pHaveObejct == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("pHaveInfluenceObejct err - unique[%d]", uniqueIdx );
			//-/-//InsertDeleteInfluenceObject( pHaveObejct );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		/// 보유한 효과의 스크립트 정보를 읽어옴
		pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveObejct->GetInfluenceClassIdx() );
		if( pHaveScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("DispelInfluence InfluenceClassIdx err - [%d]", pHaveObejct->GetInfluenceClassIdx() );
			continue;
		}

		/// 효력 그룹 비교
		if( groupOrder < pHaveScript->mGroupOrder )
			continue;

		/// 디스펠 전체는 디버프만 제거한다.
		if( pHaveScript->mType != eINFLUENCETYPE_DEBUF )
			continue;

		SKILLMANAGER->DeleteInfluenceList( uniqueIdx );
	}
}

// 효과 면역
void cSkillManager::ClearImmuneKind( sObject target, unsigned long typeDetail )
{
	/// 타겟 포인터 확인
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
		return;

	/// 타겟 객체의 보유 효과 해시셋을 읽어옴
	tHashSet<unsigned long>* pHashSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pHashSet->Begin();
	tHashSet<unsigned long>::cIterator end = pHashSet->End();

	cInfluenceObject* pHaveObejct = NULL;
	sInfluenceScript* pHaveScript = NULL;
	unsigned long uniqueIdx;

	/// 전체 검색
	while( begin != end )
	{
		uniqueIdx = *begin++;
		pHaveObejct = SKILLMANAGER->GetInfluence( uniqueIdx );

		if( pHaveObejct == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("pHaveInfluenceObejct err - unique[%d]", uniqueIdx );
			//-/-//InsertDeleteInfluenceObject( pHaveObejct );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		/// 보유한 효과의 스크립트 정보를 읽어옴
		pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveObejct->GetInfluenceClassIdx() );
		if( pHaveScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("DispelInfluence InfluenceClassIdx err - [%d]", pHaveObejct->GetInfluenceClassIdx() );
			continue;
		}

		/// 분류비교
		if( typeDetail != pHaveScript->mTypeDetail )
			continue;

		SKILLMANAGER->DeleteInfluenceList( uniqueIdx );
	}
}

// 버프/디버프 면역
void cSkillManager::ClearImmuneApplyType( sObject target, unsigned char InfluenceType, unsigned short applyType )
{
	/// 타겟 포인터 확인
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
		return;

	/// 타겟 객체의 보유 효과 해시셋을 읽어옴
	tHashSet<unsigned long>* pHashSet = pTarget->GetInfluenceSet();

	tHashSet<unsigned long>::cIterator begin = pHashSet->Begin();
	tHashSet<unsigned long>::cIterator end = pHashSet->End();

	cInfluenceObject* pHaveObejct = NULL;
	sInfluenceScript* pHaveScript = NULL;
	unsigned long uniqueIdx;

	/// 전체 검색
	while( begin != end )
	{
		uniqueIdx = *begin++;
		pHaveObejct = SKILLMANAGER->GetInfluence( uniqueIdx );

		if( pHaveObejct == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("pHaveInfluenceObejct err - unique[%d]", uniqueIdx );
			//-/-//InsertDeleteInfluenceObject( pHaveObejct );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}

		/// 보유한 효과의 스크립트 정보를 읽어옴
		pHaveScript = SKILLSCRIPT->GetInfluenceInfo( pHaveObejct->GetInfluenceClassIdx() );
		if( pHaveScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("DispelInfluence InfluenceClassIdx err - [%d]", pHaveObejct->GetInfluenceClassIdx() );
			continue;
		}

		/// 지속 효과 번호 검사
		if( applyType != pHaveScript->mStatusPlusIdx1 && applyType != pHaveScript->mStatusPlusIdx2 != applyType != pHaveScript->mStatusPlusIdx3 )
			continue;

		// 동일 타입의 효과만 삭제
		if( pHaveScript->mType != InfluenceType )
			continue;

		SKILLMANAGER->DeleteInfluenceList( uniqueIdx );
	}
}


bool cSkillManager::ClientAuraInfluenceOff( unsigned long playerIdx, unsigned long uniqueIdx, unsigned long skillClassIdx )
{
	cInfluenceObject* pInfluenceObject = (cInfluenceObject*)mInfluenceMap.GetAt( uniqueIdx );
	if( pInfluenceObject == NULL )
		return false;

	/// 스킬 번호가 같은지 확인
	if( pInfluenceObject->GetSkillIdx() != skillClassIdx )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("ClientAuraInfluenceOff skillClassIdx ERROR [%d,%d,%d]", playerIdx, uniqueIdx, skillClassIdx );
		return false;;
	}

	/// 부모 효과인지 확인
	if( pInfluenceObject->GetSwitchType() != eSWITCHTYPE_PARENT )
		return false;

	/// 효과 주인 확인
	if( pInfluenceObject->GetTarget().index != playerIdx ||
		pInfluenceObject->GetTarget().type != eOBJECTTYPE_PLAYER )
	{
		/// 조작된 메세지
		assert(NULL);
		NETWORK2->PostServerEvent("ClientAuraInfluenceOff Hack? player[%d,%d,%d]", playerIdx, uniqueIdx, skillClassIdx );
		return false;
	}

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return false;

	SKILLMANAGER->DeleteInfluence( pPlayer->GetObject(), uniqueIdx );

	return true;
}


void cSkillManager::AuraInfluenceOff( unsigned long playerIdx, unsigned long skillClassIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer != NULL )
	{
		cHashSet* pInfSet = pPlayer->GetInfluenceSet();
		cHashSet::cIterator begin = pInfSet->Begin();
		cHashSet::cIterator end = pInfSet->End();

		unsigned long influenceIdx;
		while( begin != end )
		{
			influenceIdx = *begin++;

			cInfluenceObject* pInf = GetInfluence( influenceIdx );
			if( pInf == NULL )
			{
				NETWORK2->PostServerEvent("cSkillManager::AuraInfluenceOff pInf[%d,%d] == NULL", playerIdx, skillClassIdx );
				//-/-//InsertDeleteInfluenceObject( pInf );	/// 객체 삭제 대기열에 넣는다.
				continue;
			}

			if( pInf->GetSkillIdx() != skillClassIdx )
				continue;

			if( pInf->GetSwitchType() != eSWITCHTYPE_PARENT )
				continue;

			if( pInf->GetTarget().index != playerIdx ||
				pInf->GetTarget().type != eOBJECTTYPE_PLAYER )
				continue;

			SKILLMANAGER->DeleteInfluence( pPlayer->GetObject(), influenceIdx );
			return;
		}
	}
}


void cSkillManager::SwitchParentAllOff(  unsigned long playerIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer != NULL )
	{
		cHashSet* pInfSet = pPlayer->GetInfluenceSet();
		cHashSet::cIterator begin = pInfSet->Begin();
		cHashSet::cIterator end = pInfSet->End();

		unsigned long influenceIdx;
		while( begin != end )
		{
			influenceIdx = *begin++;

			cInfluenceObject* pInf = GetInfluence( influenceIdx );
			if( pInf == NULL )
			{
				NETWORK2->PostServerEvent("cSkillManager::SwitchInfAllOff pInf[%d] == NULL", playerIdx );
				continue;
			}

			if( pInf->GetSwitchType() != eSWITCHTYPE_PARENT )
				continue;

			if( pInf->GetSkillIdx() == 0 )
				continue;

			SKILLMANAGER->DeleteInfluence( pPlayer->GetObject(), influenceIdx );
			return;
		}
	}
}


void cSkillManager::InfluenceOff( unsigned long playerIdx, unsigned long influenceClassIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer != NULL )
	{
		cHashSet* pInfSet = pPlayer->GetInfluenceSet();
		cHashSet::cIterator i = pInfSet->Begin();
		cHashSet::cIterator end = pInfSet->End();

		for( ; i != end; ++i )
		{
			unsigned long uniqueIndex = (unsigned long)(*i);
			cInfluenceObject* pInf = GetInfluence( uniqueIndex );
			if( pInf == NULL )
			{
				NETWORK2->PostServerEvent("cSkillManager::InfluenceOff pInf[%d, %d, %d] == NULL", playerIdx, uniqueIndex, influenceClassIdx );
				continue;
			}

			if( pInf->GetInfluenceClassIdx() != influenceClassIdx )
				continue;

			if( pInf->GetTarget().index != playerIdx ||
				pInf->GetTarget().type != eOBJECTTYPE_PLAYER )
				continue;

			SKILLMANAGER->DeleteInfluence( pPlayer->GetObject(), uniqueIndex );
			return;
		}
	}
}


void cSkillManager::DeleteInfluence( sObject target, unsigned long uniqueIdx )
{
	/// ※ 널참조 주의
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	unsigned long infClassIdx = 0;
	bool statusCalc = true;
	cInfluenceObject* pInf = GetInfluence( uniqueIdx );
	if( pInf != NULL )
		infClassIdx = pInf->GetInfluenceClassIdx();

	if( pTarget != NULL )
	{
		/// 탈것이면 점프중에 지울수 없다.
		const int maximum = 3;
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
		if( pScript != NULL )
		{
			unsigned short idx[maximum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };

			for( int i = 0 ; i < maximum ; ++i )
			{
				if( idx[i] == eSTATUSPLUS_VEHICLE )
				{
					if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget->GetStateDie() == false )
						if( ((cPlayer*)pTarget)->GetJumpEndTime() > NETWORK2->GetAccumTime() )
							return;
				}
			}
		}

		/// 타겟 객체 목록에 들어있는 효과 번호를 지운다.
		pTarget->DeleteInfluence( uniqueIdx );
	}

	/// 효과 객체를 지운다.
	if( pInf != NULL )
	{
		/// 토템에 의해 만들어진 효과면 삭제시 부모토템에서 목록에서 뺌
		if( pInf->GetSwitchType() == eSWITCHTYPE_TOTEM )
			pInf->TotemSetEraseTarget();

		statusCalc = pInf->IsDelStatusCalc();

		if( pTarget )
		{
			///
			const int maximum = 3;
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
			if( pScript != NULL )
			{
				unsigned short idx[maximum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };

				for( int i = 0 ; i < maximum ; ++i )
				{
					switch( idx[i] )
					{
					case eSTATUSPLUS_CHANGE_MONSTER:
						{
							if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
								((cPlayer*)pTarget)->EndChgMonster(); 
						}
						break;
					case eSTATUSPLUS_VEHICLE:
						{
							if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
								((cPlayer*)pTarget)->ArightVehicle( true, true ); 
						}
						break;
					case eSTATUSPLUS_ATTACKERTARGETING:
						{
							/// 생성시 타겟 객체가 몬스터 인지 확인 했음
							if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
								((cMonster*)pTarget)->TargetFixedEnd();
						}
					}
				}
			}
			else
				NETWORK2->PostServerEvent("cPlayer::EraseSkillInfluence SKILLSCRIPT->GetInfluenceInfo[%d,%d](%d) == NULL", 
				target.type, target.index, pInf->GetInfluenceClassIdx() );

			///
			if( pInf->IsSendDelMsg() == true )
			{
				MSG_SYN_INFLUENCE_DELETE msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_INFLUENCE_DELETE_SYN;
				msg.mUniqueIdx = uniqueIdx;

				NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
			}
		}

		/// 제거
		SAFE_DELETE( pInf );
	}

	mInfluenceMap.Erase( uniqueIdx );

	if( pTarget != NULL && statusCalc == true )
		pTarget->StatusCalc();
}


void cSkillManager::DeleteInfluenceClassIdx( sObject target, unsigned long classIdx )
{
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
		return;

	cHashSet* pInfSet = pTarget->GetInfluenceSet();
	if( pInfSet == NULL )
		return;

	cHashSet::cIterator start = pInfSet->Begin();
	cHashSet::cIterator end = pInfSet->End();

	unsigned long infIdx = 0;
	cInfluenceObject* pInfluence = NULL;
	while( start != end )
	{
		infIdx = *start++;
		pInfluence = SKILLMANAGER->GetInfluence( infIdx );
		if( pInfluence == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::DeleteInfluenceClassIdx  pInfluence[%d,%d] == NULL", target.type, target.index );
			continue;
		}

		if( pInfluence->GetInfluenceClassIdx() != classIdx )
			continue;        

		SKILLMANAGER->DeleteInfluenceList( infIdx );
	}
}


void cSkillManager::VehicleDeleteInfluence( sObject target, unsigned long uniqueIdx, bool sendSync )
{
	/// ※ 널참조 주의
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	unsigned long infClassIdx = 0;
	cInfluenceObject* pInf = GetInfluence( uniqueIdx );
	if( pInf != NULL )
		infClassIdx = pInf->GetInfluenceClassIdx();

	if( pTarget != NULL )
	{
		/// 탈것이면 점프중에 지울수 없다.
		const int maximum = 3;
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
		if( pScript != NULL )
		{
			unsigned short idx[maximum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };

			for( int i = 0 ; i < maximum ; ++i )
			{
				if( idx[i] == eSTATUSPLUS_VEHICLE )
				{
					if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
						if( ((cPlayer*)pTarget)->GetJumpEndTime() > NETWORK2->GetAccumTime() )
							return;
				}
			}
		}

		/// 타겟 객체 목록에 들어있는 효과 번호를 지운다.
		pTarget->DeleteInfluence( uniqueIdx );
		pTarget->StatusCalc();
	}

	/// 효과 객체를 지운다.
	if( pInf != NULL )
	{
		if( pTarget )
		{
			///
			const int maximum = 3;
			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
			if( pScript != NULL )
			{
				unsigned short idx[maximum] = { pScript->mStatusPlusIdx1, pScript->mStatusPlusIdx2, pScript->mStatusPlusIdx3 };

				for( int i = 0 ; i < maximum ; ++i )
				{
					switch( idx[i] )
					{
					case eSTATUSPLUS_CHANGE_MONSTER:
						{
							if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
								((cPlayer*)pTarget)->EndChgMonster(); 
						}
						break;
					case eSTATUSPLUS_VEHICLE:
						{
							if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
								((cPlayer*)pTarget)->ArightVehicle( sendSync, false );
						}
						break;
					}
				}
			}
			else
				NETWORK2->PostServerEvent("cPlayer::EraseSkillInfluence SKILLSCRIPT->GetInfluenceInfo[%d,%d](%d) == NULL", 
				target.type, target.index, pInf->GetInfluenceClassIdx() );

			///
			if( pInf->IsSendDelMsg() == true )
			{
				MSG_SYN_INFLUENCE_DELETE msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_INFLUENCE_DELETE_SYN;
				msg.mUniqueIdx = uniqueIdx;

				NETWORK2->QuickSend( pTarget, (char*)&msg, sizeof(msg) );
			}
		}

		/// 제거
		SAFE_DELETE( pInf );
		mInfluenceMap.Erase( uniqueIdx );
	}
}


void cSkillManager::DeleteInfluenceList( unsigned long uniqueIdx )
{
	cInfluenceObject* pInf = GetInfluence( uniqueIdx );
	if( pInf != NULL )
		pInf->DelReady(); 

	mDeleteInfluenceAry.PushBack( uniqueIdx );
}


void cSkillManager::ClientDeleteInfluenceObject( unsigned long playerIdx, unsigned long uniqueIdx )
{

	cInfluenceObject* pInfluenceObject = (cInfluenceObject*)mInfluenceMap.GetAt( uniqueIdx );
	if( pInfluenceObject == NULL )
		return;	/// 메세지 지연으로 나올수 있음

	/// 조작된 메세지
	if( pInfluenceObject->GetTarget().index != playerIdx ||
		pInfluenceObject->GetTarget().type != eOBJECTTYPE_PLAYER )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("ClientDeleteInfluenceObject Hack");
		return;
	}

	sInfluenceScript* pInflueneScript = SKILLSCRIPT->GetInfluenceInfo( pInfluenceObject->GetInfluenceClassIdx() );
	if( pInflueneScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("ClientDeleteInfluenceObject GetInfluenceInfo[%d]", pInfluenceObject->GetInfluenceClassIdx() );
		return;
	}

	/// 디버프 삭제 요청 불가
	if( pInflueneScript->mType == eINFLUENCETYPE_DEBUF )
		return;

	// 호칭타입은 삭제 불가
	if( pInflueneScript->mTypeDetail == eINFLUENCETYPEDETAIL_NAME )
		return;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;

	/// 호칭 효과는 호칭관리자 쪽에서 효과를 끈다.
	if( pPlayer->GetTitleInfIdx() == uniqueIdx )
		return;

	DeleteInfluence( pPlayer->GetObject(), uniqueIdx );
	return;
}


void cSkillManager::SendHPMPInf( cBaseObject* pTarget, sObject attacker,
								   char* targetMsg, unsigned short targetLen, char* aroundMsg, unsigned short aroundLen )
{
	if( pTarget == NULL )
		return;

	if( targetMsg == NULL )
		return;

	if( aroundMsg == NULL )
		return;

	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget );
	while( pPlayer != NULL )
	{
		/// 주위 플레이어가 공격자면 데미지를 보여준다.
		if( pPlayer->GetObjectID() == attacker.index && pPlayer->GetObjectType() == attacker.type )
		{
			NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), targetMsg, targetLen );

			pPlayer = GRIDMANAGER->FindNextPlayer();
			continue;
		}
		else	/// 주위 플레이어가 공격자의 파티이면 데미지를 보여준다.
		{
			if( attacker.type == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pAttacker = GRIDMANAGER->GetPlayer( attacker.index );
				if( pAttacker != NULL )
				{
					if( pAttacker->GetPartyIndex() == pPlayer->GetPartyIndex() && pPlayer->GetPartyIndex() != 0 )
					{
						NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), targetMsg, targetLen );

						pPlayer = GRIDMANAGER->FindNextPlayer();
						continue;
					}
				}
			}
		}

		/// 주위 플레이어가 타겟이면 데미지를 보여준다.
		if( pPlayer->GetObjectID() == pTarget->GetObjectID() && pPlayer->GetObjectType() == pTarget->GetObjectType() )
		{
			if( attacker.index == pTarget->GetObjectID() && attacker.type == pTarget->GetObjectType() )
			{
				pPlayer = GRIDMANAGER->FindNextPlayer();
				continue;
			}
			else
			{
				NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), targetMsg, targetLen );

				pPlayer = GRIDMANAGER->FindNextPlayer();
				continue;
			}
		}
		else		/// 주위 플레이어가 타겟의 파티이면 데미지를 보여준다.
		{
			if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( ((cPlayer*)pTarget)->GetPartyIndex() == pPlayer->GetPartyIndex() && pPlayer->GetPartyIndex() != 0 )
				{
					NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), targetMsg, targetLen );

					pPlayer = GRIDMANAGER->FindNextPlayer();
					continue;
				}
			}
		}

		NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), aroundMsg, aroundLen );

		pPlayer = GRIDMANAGER->FindNextPlayer();
	}
}


void cSkillManager::CheckActInfluence( eACTSTATUS status, cBaseObject* pObject, cBaseObject* pTarget )
{
	if( pObject == 0 )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::CheckActInfluence pPlayer==NULL" );
		return;
	}
	if( pObject->GetObjectType() != eOBJECTTYPE_PLAYER && pObject->GetObjectType() != eOBJECTTYPE_HERO )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cSkillManager::CheckActInfluence - pPlayer object type is not Player or hero" );
		return;
	}

	cPlayer* pPlayer = (cPlayer*)pObject;
	if( pPlayer->GetStateDie() )
		return;

	cHashMap* pActMap = pPlayer->GetActInfluenceMap();
	unsigned long checkTime = NETWORK2->GetAccumTime();

	if( pActMap->IsEmpty() == true )
		return;

	cHashMap::cIterator i = pActMap->Begin();
	cHashMap::cIterator end = pActMap->End();
	for( ; i != end; ++i )
	{
		unsigned long actIdx = (unsigned long)((*i).mFirst);
		unsigned long coolTime = (unsigned long)((*i).mSecond);

		sActInfluenceScript* pInfo = SKILLSCRIPT->GetActInfluenceInfo(actIdx);
		if( pInfo == 0 )
			continue;
		/// 상황 체크
		if( pInfo->mActivityState != status )
			continue;
		/// 쿨타임 체크
		if( coolTime > checkTime )
			continue;
		/// 확률 체크
		unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;
		if( pInfo->mActivityPer < randValue )
			continue;

		bool sendMsg = false;
		if( pInfo->mDramaturgyFile != "None" )
			sendMsg = true;

		//////////////////////////////////////////////////////////////////////////
		/// 발동 효과 부여...
		if( pInfo->mApplyType == eAPPLYTYPE_SELF )
		{
			if( AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true ) == false )
				continue;
		}
		else if( pInfo->mApplyType == eAPPLYTYPE_ENEMY )
		{
			if( pInfo->mBoundType == eBOUNDTYPE_NONE )
			{
				/// 설정 타겟에 버프 부여
				if( pTarget == 0 )
					continue;
				if( pTarget->GetStateDie() == true )
					continue;

				if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					if( pPlayer->IsAttack( pTarget->GetObjectID() ) == false )
						continue;
				}
				else if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
					continue;

				if( AddInfluence( pPlayer->GetObject(), pTarget->GetObject(), pInfo->mInfIdx, 0, true ) == false )
					continue;
			}
			else
			{
				/// 중심위치, 범위 계산
				float tempRange;
				NiPoint3 centerPos;

				if( pInfo->mBoundType == eBOUNDTYPE_SELF_TARGET )
				{
					tempRange = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, pInfo->mBoundDist );
					centerPos = NiPoint3( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height());
				}
				else if( pInfo->mBoundType == eBOUNDTYPE_TARGET)
				{
					if( pTarget == 0 )
						continue;

					tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, NULL, pInfo->mBoundDist );
					centerPos = NiPoint3( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height());
				}
				else
					continue;

				/// 적 리스트 탐색 및 버프/디버프 부여
				cRangeCheck rangeCheck;
				rangeCheck.SetRadius( tempRange );

				//OBJECTMANAGER->MonsterInCenterPoint( pPlayer->GetMapNumber(), centerPos, tempRange, &targetArr, false );
				//OBJECTMANAGER->PlayerInCenterPoint( pPlayer->GetMapNumber(), centerPos, tempRange, &targetArr, 0, false );

				mTempActTargetAry.Clear();
				GRIDMANAGER->FindPlayers( centerPos.x, centerPos.y, pPlayer->GetMapNumber(), tempRange, &mTempActTargetAry );
				for( unsigned long i = 0 ; i < mTempActTargetAry.GetSize() ; ++i )
				{
					sObject pMulTar = { eOBJECTTYPE_PLAYER, mTempActTargetAry[i] };
					AddInfluence( pPlayer->GetObject(), pMulTar, pInfo->mInfIdx, 0, true );
				}
				mTempActTargetAry.Clear();
				GRIDMANAGER->FindMonsters( centerPos.x, centerPos.y, pPlayer->GetMapNumber(), tempRange, &mTempActTargetAry );
				for( unsigned long i = 0 ; i < mTempActTargetAry.GetSize() ; ++i )
				{
					sObject pMulTar = { eOBJECTTYPE_MONSTER, mTempActTargetAry[i] };
					AddInfluence( pPlayer->GetObject(), pMulTar, pInfo->mInfIdx, 0, true );
				}
				mTempActTargetAry.Clear();
			}
		}
		else if( pInfo->mApplyType == eAPPLYTYPE_PARTY )
		{
			if( pInfo->mBoundType == eBOUNDTYPE_NONE )
			{
				AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true );
			}
			else
			{
				/// 중심위치, 범위 계산
				float tempRange;
				NiPoint3 centerPos;
				if( pInfo->mBoundType == eBOUNDTYPE_SELF_TARGET )
				{
					tempRange = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, pInfo->mBoundDist );
					centerPos = NiPoint3( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height());

				}
				else if( pInfo->mBoundType == eBOUNDTYPE_TARGET)
				{
					if( pTarget == 0 )
						continue;

					tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, NULL, pInfo->mBoundDist );
					centerPos = NiPoint3( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height());
				}
				else
					continue;

				cRangeCheck rangeCheck;
				rangeCheck.SetRadius( tempRange );

				/// 파티 정보 탐색 및 버프 부여
				cParty* pParty = PARTYMAN->GetParty( pPlayer->GetPartyIndex() );
				if( pParty != NULL )
				{
					unsigned long* userArr = pParty->GetUserArr();
					unsigned int partyCount = pParty->GetCount();

					for( unsigned int i = 0 ; i < partyCount ; ++i )
					{
						/// 파티원을 읽어옴
						cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
						if( pChild == NULL )
							continue;
						if( pChild->GetStateDie() == true )
							continue;
						if( pChild->GetGameIn() == false )
							continue;

						/// 거리안에 들어오는 경우
						NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
						if( rangeCheck.IsRange( childPos, centerPos ) )
							AddInfluence( pPlayer->GetObject(), pChild->GetObject(), pInfo->mInfIdx, 0, true );
					}
				}
				else
				{
					/// 파티연합으로부터 정보 읽음
					cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( pPlayer->GetPartyUnionIndex() );
					if( partyUnion != NULL )
					{
						unsigned long* userArr = partyUnion->GetGroupArr( pPlayer->GetObjectID() );

						for( unsigned int i = 0; i < MAX_PARTYUSER; ++i )
						{
							/// 파티원을 읽어옴
							cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
							if( pChild == NULL )
								continue;
							if( pChild->GetStateDie() == true )
								continue;
							if( pChild->GetGameIn() == false )
								continue;

							/// 거리안에 들어오는 경우
							NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
							if( rangeCheck.IsRange( childPos, centerPos ) )
								AddInfluence( pPlayer->GetObject(), pChild->GetObject(), pInfo->mInfIdx, 0, true );
						}
					}
					else
					{
						/// 파티 및 파티 유니온 정보가 없다면 자신만 건다..
						if( AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true ) == false )
							continue;
					}
				}
			}
		}
		else
		{
			/// 현재는 아이템 발동효과 쪽에서는 사용하고 있지 않다 [4/6/2010 Jo_Client]
			/*	eAPPLYTYPE_BUFF_SELF:
				eAPPLYTYPE_DEBUFF_SELF:		
				eAPPLYTYPE_BUFF_ENEMY:		
				eAPPLYTYPE_DEBUFF_ENEMY:	
				eAPPLYTYPE_BUFF_PARTY:		
				eAPPLYTYPE_DEBUFF_PARTY:
			*/
			continue;
		}

		//////////////////////////////////////////////////////////////////////////
		/// 쿨타임 갱신
		(*i).mSecond = pInfo->mCooltime + checkTime;

		//////////////////////////////////////////////////////////////////////////
		/// msg send
		if( sendMsg == true )
		{
			MSG_SYN_ACTINFLUNCE_ACTION msg;
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_ACTINFLUENCE_ACTION_SYN;
			msg.mPlayer = pPlayer->GetObject();
			msg.mActIdx = actIdx;

			NETWORK2->QuickSend( pPlayer, (char*)&msg, sizeof(msg) );
		}

		break;
	}
}

void cSkillManager::CheckSkillActInfluence( unsigned long actIdx, cBaseObject* pObject, cBaseObject* pTarget )
{
	if( pObject == 0 )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::CheckSkillActInfluence pPlayer==NULL" );
		return;
	}
	if( pObject->GetObjectType() != eOBJECTTYPE_PLAYER && pObject->GetObjectType() != eOBJECTTYPE_HERO )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cSkillManager::CheckSkillActInfluence - pPlayer object type is not Player or hero" );
		return;
	}

	cPlayer* pPlayer = (cPlayer*)pObject;
	if( pPlayer->GetStateDie() )
		return;


	sActInfluenceScript* pInfo = SKILLSCRIPT->GetActInfluenceInfo( actIdx );
	if( pInfo == 0 )
		return;


	/// 확률 체크
	unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;
	if( pInfo->mActivityPer < randValue )
		return;

	bool sendMsg = false;
	if( pInfo->mDramaturgyFile != "None" )
		sendMsg = true;

	//////////////////////////////////////////////////////////////////////////
	/// 발동 효과 부여...
	if( pInfo->mApplyType == eAPPLYTYPE_SELF )
	{
		if( AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true ) == false )
			return;
	}
	else if( pInfo->mApplyType == eAPPLYTYPE_ENEMY )
	{
		if( pInfo->mBoundType == eBOUNDTYPE_NONE )
		{
			/// 설정 타겟에 버프 부여
			if( pTarget == 0 )
				return;
			if( pTarget->GetStateDie() == true )
				return;

			if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( pPlayer->IsAttack( pTarget->GetObjectID() ) == false )
					return;
			}
			else if( pTarget->GetObjectType() != eOBJECTTYPE_MONSTER )
				return;

			if( AddInfluence( pPlayer->GetObject(), pTarget->GetObject(), pInfo->mInfIdx, 0, true ) == false )
				return;
		}
		else
		{
			/// 중심위치, 범위 계산
			float tempRange;
			NiPoint3 centerPos;

			if( pInfo->mBoundType == eBOUNDTYPE_SELF_TARGET )
			{
				tempRange = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, pInfo->mBoundDist );
				centerPos = NiPoint3( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height());
			}
			else if( pInfo->mBoundType == eBOUNDTYPE_TARGET)
			{
				if( pTarget == 0 )
					return;

				tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, NULL, pInfo->mBoundDist );
				centerPos = NiPoint3( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height());
			}
			else
				return;

			/// 적 리스트 탐색 및 버프/디버프 부여
			cRangeCheck rangeCheck;
			rangeCheck.SetRadius( tempRange );

			mTempActTargetAry.Clear();
			GRIDMANAGER->FindPlayers( centerPos.x, centerPos.y, pPlayer->GetMapNumber(), tempRange, &mTempActTargetAry );
			for( unsigned long i = 0 ; i < mTempActTargetAry.GetSize() ; ++i )
			{
				sObject pMulTar = { eOBJECTTYPE_PLAYER, mTempActTargetAry[i] };
				AddInfluence( pPlayer->GetObject(), pMulTar, pInfo->mInfIdx, 0, true );
			}
			mTempActTargetAry.Clear();
			GRIDMANAGER->FindMonsters( centerPos.x, centerPos.y, pPlayer->GetMapNumber(), tempRange, &mTempActTargetAry );
			for( unsigned long i = 0 ; i < mTempActTargetAry.GetSize() ; ++i )
			{
				sObject pMulTar = { eOBJECTTYPE_MONSTER, mTempActTargetAry[i] };
				AddInfluence( pPlayer->GetObject(), pMulTar, pInfo->mInfIdx, 0, true );
			}
			mTempActTargetAry.Clear();
		}
	}
	else if( pInfo->mApplyType == eAPPLYTYPE_PARTY )
	{
		if( pInfo->mBoundType == eBOUNDTYPE_NONE )
		{
			AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true );
		}
		else
		{
			/// 중심위치, 범위 계산
			float tempRange;
			NiPoint3 centerPos;
			if( pInfo->mBoundType == eBOUNDTYPE_SELF_TARGET )
			{
				tempRange = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, pInfo->mBoundDist );
				centerPos = NiPoint3( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height());
			}
			else if( pInfo->mBoundType == eBOUNDTYPE_TARGET)
			{
				if( pTarget == 0 )
					return;

				tempRange = OBJECTMANAGER->ObjectSizeRange( pTarget, NULL, pInfo->mBoundDist );
				centerPos = NiPoint3( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height());
			}
			else
				return;

			cRangeCheck rangeCheck;
			rangeCheck.SetRadius( tempRange );

			/// 파티 정보 탐색 및 버프 부여
			cParty* pParty = PARTYMAN->GetParty( pPlayer->GetPartyIndex() );
			if( pParty != NULL )
			{
				unsigned long* userArr = pParty->GetUserArr();
				unsigned int partyCount = pParty->GetCount();

				for( unsigned int i = 0 ; i < partyCount ; ++i )
				{
					/// 파티원을 읽어옴
					cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
					if( pChild == NULL )
						continue;
					if( pChild->GetStateDie() == true )
						continue;
					if( pChild->GetGameIn() == false )
						continue;

					/// 거리안에 들어오는 경우
					NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
					if( rangeCheck.IsRange( childPos, centerPos ) )
						AddInfluence( pPlayer->GetObject(), pChild->GetObject(), pInfo->mInfIdx, 0, true );
				}
			}
			else
			{
				/// 파티연합으로부터 정보 읽음
				cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( pPlayer->GetPartyUnionIndex() );
				if( partyUnion != NULL )
				{
					unsigned long* userArr = partyUnion->GetGroupArr( pPlayer->GetObjectID() );

					for( unsigned int i = 0; i < MAX_PARTYUSER; ++i )
					{
						/// 파티원을 읽어옴
						cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
						if( pChild == NULL )
							continue;
						if( pChild->GetStateDie() == true )
							continue;
						if( pChild->GetGameIn() == false )
							continue;

						/// 거리안에 들어오는 경우
						NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
						if( rangeCheck.IsRange( childPos, centerPos ) )
							AddInfluence( pPlayer->GetObject(), pChild->GetObject(), pInfo->mInfIdx, 0, true );
					}
				}
				else
				{
					/// 파티 및 파티 유니온 정보가 없다면 자신만 건다..
					if( AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pInfo->mInfIdx, 0, true ) == false )
						return;
				}
			}
		}
	}
	else
	{
		/// 현재는 아이템 발동효과 쪽에서는 사용하고 있지 않다 [4/6/2010 Jo_Client]
		/*	eAPPLYTYPE_BUFF_SELF:
		eAPPLYTYPE_DEBUFF_SELF:		
		eAPPLYTYPE_BUFF_ENEMY:		
		eAPPLYTYPE_DEBUFF_ENEMY:	
		eAPPLYTYPE_BUFF_PARTY:		
		eAPPLYTYPE_DEBUFF_PARTY:
		*/
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	/// msg send
	if( sendMsg == true )
	{
		MSG_SYN_ACTINFLUNCE_ACTION msg;
		msg.Category = NM_SKILL;
		msg.Protocol = NM_SKILL_ACTINFLUENCE_ACTION_SYN;
		msg.mPlayer = pPlayer->GetObject();
		msg.mActIdx = actIdx;

		NETWORK2->QuickSend( pPlayer, (char*)&msg, sizeof(msg) );
	}
}


void cSkillManager::InsertChangeSet( cBaseObject* pTarget, sObject attacker, unsigned char damageKind )
{ 
	if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
	{
		mPlayerChangeSet.Insert( pTarget->GetObjectID() );
		cPlayer* pPlayer = (cPlayer*)pTarget;

		pPlayer->SetSkillLastAttack( attacker, damageKind );
	}
	else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
	{
		mMonsterChangeSet.Insert( pTarget->GetObjectID() );
		cMonster* pMonster = (cMonster*)pTarget;

		pMonster->SetSkillLastAttack( attacker, damageKind );
	}
}


bool cSkillManager::NeedAddPlayerHaveSkill( unsigned long skillIdx, unsigned char skillStep, /*unsigned long* money,*/ unsigned short* sp )
{
	/// 플레이어 스킬스크립트 참조
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( !pSkillInfo )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::NeedAddPlayerHaveSkill pSkillInfo==NULL" );
		return false; 
	}

	/// 플레이어 스킬스텝 스크립트 참조
	if( skillStep < 0 || pSkillInfo->mStepCount <= skillStep )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::NeedAddPlayerHaveSkill pSkillInfo->mStepCount ERR" );
		return false; 
	}

	sPlayerSkillStepInfo* pSkillStep = &pSkillInfo->mpSetpInfoArray[skillStep];
	if( pSkillStep == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cSkillManager::NeedAddPlayerHaveSkill pSkillStep==NULL" );
		return false;
	}

	//*money = pSkillStep->mUseMoney;
	*sp = pSkillStep->mRequireSP;

	return true;

}



void cSkillManager::UpdateSkillCoolTime( sObject attacker, unsigned long skillIdx, unsigned char skillStep, unsigned long /*accumTime*/, float speedFactor )
{
	/// 플레이어만 사용 몬스터는 따로 가지고 있다
	switch( attacker.type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_PLAYER:
		{
			/// 플레이어 스킬스크립트 참조
			sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
			if( !pSkillInfo )	
			{ 
				assert(NULL); 
				NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime pSkillInfo[%d]==NULL", skillIdx );
				return; 
			}

			/// 플레이어 스킬스텝 스크립트 참조
			if( skillStep < 0 || pSkillInfo->mStepCount <= skillStep )	
			{ 
				assert(NULL); 
				NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime pSkillInfo->mStepCount[%d] ERR", pSkillInfo->mStepCount );
				return; 
			}
			sPlayerSkillStepInfo* pSkillStep = &pSkillInfo->mpSetpInfoArray[skillStep];

			cHaveSkillObject* pHaveSkill = GetPlayerHaveSkill( attacker.index, skillIdx );
			if( pHaveSkill == NULL ) 
			{ 
				assert(NULL); 
				NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime GetPlayerHaveSkill[%d,%d]", attacker.index, skillIdx );
				return; 
			}
			pHaveSkill->UpdateCoolTime( pSkillStep->mCoolTime, speedFactor );

			/// 그룹 쿨타임 체크
			if( pSkillInfo->mGroupCoolTimeIdx != 0 )
			{
				/// 플레이어 보유스킬 포인트 획득
				cPointerHashMap* pHaveSkillMap = (cPointerHashMap*)GetPlayerHaveSkill( attacker.index );
				if( !pHaveSkillMap )	
				{ 
					assert(NULL); 
					NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime pHaveSkillMap[%d]==NULL", attacker.index );
					return; 
				}

				/// 보유 스킬들의 쿨타임 관련 항목 업데이트
				cPointerHashMap::cIterator begin = pHaveSkillMap->Begin();
				cPointerHashMap::cIterator end = pHaveSkillMap->End();
				for( ; begin != end ; ++begin )
				{
					cHaveSkillObject* pHave = (cHaveSkillObject*)(*begin).mSecond;
					if( pHave == NULL )
					{
						assert(NULL);
						NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime pHave==NULL" );
						continue;
					}
					sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( pHave->GetSkillIdx() );
					if( pInfo == NULL ) 
					{ 
						assert(NULL); 
						NETWORK2->PostServerEvent("cSkillManager::UpdateSkillCoolTime pInfo==NULL" );
						continue; 
					}

					if( pInfo->mGroupCoolTimeIdx == 0 )
						continue;
					if( skillIdx == pHave->GetSkillIdx() )
						continue;

					/// 같은 그룹인경우 사용 스킬의 쿨타임으로 갱신
					if( pSkillInfo->mGroupCoolTimeIdx == pInfo->mGroupCoolTimeIdx )
					{
						sPlayerSkillStepInfo* pStep = &pInfo->mpSetpInfoArray[pHave->GetSkillStep()];
						if( pSkillStep->mCoolTime > pStep->mCoolTime )
							pHave->UpdateCoolTime( pStep->mCoolTime, speedFactor );
						else
							pHave->UpdateCoolTime( pSkillStep->mCoolTime, speedFactor );
					}
				}
			}
		}break;
	}
}

void cSkillManager::AddSkillReset( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep )
{
	/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
	cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
	/// 보유한 스킬리스트가 없으면
	if ( !pPlayerHaveSkillMap )
	{
		/// 새로운 스킬리스트를 만듬
		pPlayerHaveSkillMap = new cPointerHashMap;

		/// 새로 만든 스킬리스트를 기록
		if( !( mPlayerHaveSkillMap.Insert( playerIdx, pPlayerHaveSkillMap ) ) )
		{
			NETWORK2->PostServerEvent("AddSkillReset[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep );
			SAFE_DELETE( pPlayerHaveSkillMap );
			return;
		}
	}

	/// 보유스킬리스트 정보를 기록
	cHaveSkillObject* pPlayerHaveSkill = new cHaveSkillObject;
	pPlayerHaveSkill->Init( skillIdx, skillStep, 0 );

	if( !(pPlayerHaveSkillMap->Insert( skillIdx, pPlayerHaveSkill ) ) )
	{
		NETWORK2->PostServerEvent("AddSkillReset[%d,%d,%d] - mPlayerHaveSkillMap.Insert error", playerIdx, skillIdx, skillStep  );
		SAFE_DELETE( pPlayerHaveSkill );
		return;
	}
}


bool cSkillManager::SaveSkillCoolTime( unsigned long playerIdx, SKILL_COOLTIME* skillCoolTime )
{

	skillCoolTime->mCharacterIdx = playerIdx;
	skillCoolTime->mRetvalue = 0;

	/// 플레이어 보유스킬 포인트 획득
	cPointerHashMap* pHaveSkillMap = (cPointerHashMap*)GetPlayerHaveSkill( playerIdx );
	if( pHaveSkillMap == NULL)	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SaveSkillCoolTime pHaveSkillMap==NULL" );
		return false; 
	}

	cPointerHashMap::cIterator begin = pHaveSkillMap->Begin();
	cPointerHashMap::cIterator end = pHaveSkillMap->End();

	/// 보유스킬 포인터
	cHaveSkillObject* pHaveSkill = NULL;
	long restTime = 0;
	unsigned long row = 0;

	for( ; begin != end ; ++begin )
	{
		pHaveSkill = (cHaveSkillObject*)((*begin).mSecond);
		if( pHaveSkill == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::SaveSkillCoolTime pHaveSkill==NULL" );
			continue;
		}

		/// 기본 공격 통과
		if( pHaveSkill->GetSkillIdx() <= NORMAL_ATTACK_SKILL_MAX )
			continue;

		/// 남은 쿨타임이 없는 경우 통과
		if( pHaveSkill->DbSaveDataCoolTime() <= NETWORK2->GetAccumTime() )
			continue;

		/// 재사용 가능시간이 DB_SAVE_COOLTIME ms초 이상인 것들만 저장
		restTime = pHaveSkill->DbSaveDataCoolTime() - NETWORK2->GetAccumTime();
		if( restTime > DB_SAVE_COOLTIME && pHaveSkill->GetSkillIdx() != 0 )
		{
			skillCoolTime->mTable[row].mSkillIdx = pHaveSkill->GetSkillIdx();
			skillCoolTime->mTable[row].mSkillStep = pHaveSkill->GetSkillStep();
			skillCoolTime->mTable[row].mEndCoolTime = restTime;
			++row;
		}
	}

	skillCoolTime->mRowCount = row;

	return true;

}


bool cSkillManager::RestorePlayerSkill( SKILL_SELECT* skillSelect )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( skillSelect->mCharacterIdx );
	if( pPlayer == NULL )
		return false;

	///// 보유목록 전체 제거
	//if( skillSelect->mRowCount >= 1 )
	//	ReleasePlayerHaveSkill( skillSelect->mCharacterIdx );

	/// 기본공격(idx:0) 넣어 줘야함
	SKILLMANAGER->AddPlayerHaveSkill( skillSelect->mCharacterIdx, NORMAL_ATTACK_SKILL, SKILL_START_STEP, 0 );

	long restTime;
	for ( long i = 0; i < skillSelect->mRowCount; i++ )
	{
		restTime = skillSelect->mTable[i].mEndCoolTime;

		/// 사용 가능 시간이 된경우
		if( restTime < 0 )	{ restTime = 0; }

		/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( skillSelect->mCharacterIdx );
		/// 보유한 스킬리스트가 없으면
		if ( !pPlayerHaveSkillMap )
		{
			/// 새로운 스킬리스트를 만듬
			pPlayerHaveSkillMap = new cPointerHashMap;

			/// 새로 만든 스킬리스트를 기록
			if( !( mPlayerHaveSkillMap.Insert( skillSelect->mCharacterIdx, pPlayerHaveSkillMap ) ) )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::RestorePlayerSkill mPlayerHaveSkillMap.Insert duplicate" );
				SAFE_DELETE( pPlayerHaveSkillMap );
				return false;
			}
		}

		if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
		{
			cHaveSkillObject* pPlayerHaveSkill = (cHaveSkillObject*)pPlayerHaveSkillMap->GetAt( skillSelect->mTable[i].mSkillIdx );
			if( pPlayerHaveSkill != NULL )
				continue;
		}

		/// 보유스킬리스트 정보를 기록
		cHaveSkillObject* pPlayerHaveSkill = new cHaveSkillObject;
		pPlayerHaveSkill->RestoreInit( skillSelect->mTable[i].mSkillIdx, skillSelect->mTable[i].mSkillStep, restTime );

		if( !(pPlayerHaveSkillMap->Insert( skillSelect->mTable[i].mSkillIdx, pPlayerHaveSkill ) ) )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::RestorePlayerSkill pPlayerHaveSkillMap.Insert duplicate" );
			SAFE_DELETE( pPlayerHaveSkill );
			return false;
		}
	}

	//if( NETWORK2->GetServerType() == _E_ST_ID_TUTORIAL_ )
	//	return AddTutorialHaveSkill( pPlayer );

	return true;
}

bool cSkillManager::AddTutorialHaveSkill( cPlayer* pPlayer )
{
	/// 스크립트 데이터 로드
	cPArray* pScriptAry = TUTORIALSKILLSCRIPT->GetTutorialSkillArr();
	if( pScriptAry == NULL )
		return false;

	unsigned long playerIdx = pPlayer->GetObjectID();

	for( unsigned long i = 0; i < pScriptAry->GetSize() ; i++ )
	{
		sTutorialSkill* pTutorial = (sTutorialSkill*)(*pScriptAry)[i];

		/// playeridx로 플레이어가 보유한 스킬리스트 포인터를 얻어옴
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
		/// 보유한 스킬리스트가 없으면
		if ( !pPlayerHaveSkillMap )
		{
			/// 새로운 스킬리스트를 만듬
			pPlayerHaveSkillMap = new cPointerHashMap;

			/// 새로 만든 스킬리스트를 기록
			if( !( mPlayerHaveSkillMap.Insert( playerIdx, pPlayerHaveSkillMap ) ) )
			{
				NETWORK2->PostServerEvent("cSkillManager::AddTutorialHaveSkill mPlayerHaveSkillMap.Insert duplicate" );
				SAFE_DELETE( pPlayerHaveSkillMap );
				return false;
			}
		}

		if( pPlayerHaveSkillMap->GetAt( pTutorial->skillIndex ) != NULL )
			continue;

		sPlayerSkillBaseInfo* pBase = SKILLSCRIPT->GetPlayerSkillInfo( pTutorial->skillIndex );
		if( pBase == NULL )
		{
			NETWORK2->PostServerEvent("cSkillManager::AddTutorialHaveSkill pBase == NULL[%d]", pTutorial->skillIndex);
			continue;
		}

		if( pBase->mStepCount <= pTutorial->skillStep )
		{
			NETWORK2->PostServerEvent("cSkillManager::AddTutorialHaveSkill pBase->mStepCount < pTutorial->skillStep[%d]");
			continue;
		}

		sPlayerSkillStepInfo* pStep = &pBase->mpSetpInfoArray[pTutorial->skillStep];

		/// 종족 구분 있는경우
		if( pBase->mRace != 0 ) /// 0 = ALL
		{
			/// 종족 체크
			switch( pPlayer->GetRace() )
			{
			case eRACE_HUMAN:
				if( !( pBase->mRace == eSKILLRACE_HUMAN || pBase->mRace == eSKILLRACE_HUMAN_BEAST || pBase->mRace == eSKILLRACE_HUMAN_ELF ) )	
					continue; 
				break;
			case eRACE_BEAST:
				if( !( pBase->mRace == eSKILLRACE_BEAST || pBase->mRace == eSKILLRACE_HUMAN_BEAST || pBase->mRace == eSKILLRACE_BEAST_ELF ) )
					continue; 
				break;
			case eRACE_ELF:
				if( !( pBase->mRace == eSKILLRACE_ELF || pBase->mRace == eSKILLRACE_HUMAN_ELF || pBase->mRace == eSKILLRACE_BEAST_ELF ) )	
					continue; 
				break;
			default:
				assert(NULL); 
				continue; 	
			}
		}

		/// 직업구분 있는 경우
		if( pStep->mJobType != 0 )
		{
			/// 직업을 체크
			if( !SKILLSCRIPT->IsPlayerSkillJob( pStep->mJobType, (unsigned long)(pPlayer->GetJob()) ) )
				continue;
		}

		/// 보유스킬리스트 정보를 기록
		cHaveSkillObject* pPlayerHaveSkill = new cHaveSkillObject;
		pPlayerHaveSkill->RestoreInit( pTutorial->skillIndex, pTutorial->skillStep, 0 );

		if( !(pPlayerHaveSkillMap->Insert( pTutorial->skillIndex, pPlayerHaveSkill ) ) )
		{
			NETWORK2->PostServerEvent("cSkillManager::AddTutorialHaveSkill pPlayerHaveSkillMap.Insert duplicate" );
			SAFE_DELETE( pPlayerHaveSkill );
			return false;
		}
	}

	return true;
}

void cSkillManager::SendMsgSkillHaveToPlayer( unsigned long connectionIdx, unsigned long characterIdx )
{
	HANDLE              handle  = NULL;
	MSG_RES_SKILL_LIST* sendMsg = (MSG_RES_SKILL_LIST*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_SKILL, NM_SKILL_LIST_RES );

	if ( sendMsg != NULL )
	{
		cPointerHashMap* pPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( characterIdx );

		if( pPlayerHaveSkillMap )
		{
			cHaveSkillObject* pPlayerHaveSkill;

			cPointerHashMap::cIterator start = pPlayerHaveSkillMap->Begin();
			cPointerHashMap::cIterator end = pPlayerHaveSkillMap->End();

			unsigned char& row = (sendMsg->mRowCount = 0);
			for( ; start != end ; ++start )
			{
				pPlayerHaveSkill = (cHaveSkillObject*)(*start).mSecond;

				if( pPlayerHaveSkill == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("cSkillManager::SendMsgSkillHaveToPlayer pPlayerHaveSkill==NULL" );
					continue;
				}

				if( pPlayerHaveSkill->GetSkillIdx() == 0 )	
					continue; 

				sendMsg->mTable[row].mSkillIdx     = pPlayerHaveSkill->GetSkillIdx();
				sendMsg->mTable[row].mSkillStep    = pPlayerHaveSkill->GetSkillStep();
				sendMsg->mTable[row].mRestCoolTime = pPlayerHaveSkill->GetRestTime();
				++row;
			}
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}

	cPlayer* player = OBJECTMANAGER->GetPlayer( characterIdx );
	if( player )
		player->SendSkillPoint();
	else
		assert(0);
}



void cSkillManager::SendMsgInfluenceToPlayer( unsigned long connectionIdx, unsigned long characterIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( characterIdx );
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SendMsgInfluenceToPlayer pPlayer[%d]==NULL", characterIdx);
		return; 
	}

	tHashSet<unsigned long>* pInfluenceSet = pPlayer->GetInfluenceSet();
	if( !pInfluenceSet )	
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent("cSkillManager::SendMsgInfluenceToPlayer pInfluenceSet==NULL" );
		return; 
	}

	tHashSet<unsigned long>::cIterator start = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();

	HANDLE handle1  = NULL;
	MSG_RES_INFLUENCE_LIST* pInfList = (MSG_RES_INFLUENCE_LIST*)NETWORK2->GetMsgRoot( &handle1, connectionIdx, 
		NM_SKILL, NM_SKILL_INFLUENCE_LIST_RES );
	if ( pInfList == NULL )
	{
		assert(NULL);
		return;
	}

	HANDLE handle2 = NULL;
	MSG_SYN_SKILL_INFLUENCE_GUARDLIST* pGuardList = (MSG_SYN_SKILL_INFLUENCE_GUARDLIST*)NETWORK2->GetMsgRoot( &handle2, connectionIdx,
		NM_SKILL, NM_SKILL_INFLUENCE_GUARDLIST_RES );
	if( pGuardList == NULL )
	{
		assert(NULL);
		return;
	}

	pInfList->mRowCount = 0;
	pGuardList->mRowCount = 0;

	for( ; start != end ; ++start )
	{
		cInfluenceObject* pInf = GetInfluence( (*start) );
		if( pInf == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cSkillManager::SendMsgInfluenceToPlayer pInfluenceObject[%d]==NULL", (*start) );
			continue;
		}

		if( pInf->GetGuardType() == eINFGUARDTYPE_NONE )
		{
			pInfList->mTable[pInfList->mRowCount].mUniqueIdx = pInf->GetUniqueIdx();
			pInfList->mTable[pInfList->mRowCount].mInfluenceClassIdx = pInf->GetInfluenceClassIdx();
			if( pInf->GetSwitchType() == eSWITCHTYPE_PARENT )
				pInfList->mTable[pInfList->mRowCount].mAuraSkillClassIdx = pInf->GetSkillIdx();
			else
				pInfList->mTable[pInfList->mRowCount].mAuraSkillClassIdx = 0;
			pInfList->mTable[pInfList->mRowCount].mIsRealTime = pInf->IsRealTime();
			if( pInf->IsRealTime() == true )
			{				
				pInfList->mTable[pInfList->mRowCount].mRestTime = 0;
				
				tm tmTime = TIMER->ConvertTStoTM( pInf->GetInfRealEndTime() );
				time_t tTime = mktime( &tmTime );
				pInfList->mTable[pInfList->mRowCount].mEndRealTime = tTime;
			}
			else
				pInfList->mTable[pInfList->mRowCount].mRestTime = pInf->GetInfluenceRestTime();

			pInfList->mTable[pInfList->mRowCount].mDeleteType = pInf->GetDeleteType();
			++pInfList->mRowCount;
		}
		else
		{
			pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mUniqueIdx = pInf->GetUniqueIdx();
			pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mInfluenceClassIdx = pInf->GetInfluenceClassIdx();
			if( pInf->GetSwitchType() == eSWITCHTYPE_PARENT )
				pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mAuraSkillClassIdx = pInf->GetSkillIdx();
			else
				pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mAuraSkillClassIdx = 0;
			pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mIsRealTime = pInf->IsRealTime();
			if( pInf->IsRealTime() == true )
			{
				pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mRestTime = 0;

				tm tmTime = TIMER->ConvertTStoTM( pInf->GetInfRealEndTime() );
				time_t tTime = mktime( &tmTime );
				pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mEndRealTime = tTime;
			}
			else
				pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mRestTime = pInf->GetInfluenceRestTime();

			pGuardList->mGuardList[pGuardList->mRowCount].mInfInfo.mDeleteType = pInf->GetDeleteType();
			
			pGuardList->mGuardList[pGuardList->mRowCount].mGuardCnt = pInf->GetLeftGuardCnt();
			pGuardList->mGuardList[pGuardList->mRowCount].mGuardValue = pInf->GetLeftGuardValue();
			++pGuardList->mRowCount;
		}
	}

	NETWORK2->SendMsgRoot( handle1, pInfList->GetMsgLength( ) );
	NETWORK2->SendMsgRoot( handle2, pGuardList->GetMsgLength( ) );
}


void cSkillManager::SendAddSynMsgInfluenceToPlayer( unsigned long connectionIdx, unsigned long characterIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( characterIdx );
	if( !pPlayer )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SendAddSynMsgInfluenceToPlayer pPlayer[%d]==NULL", characterIdx );
		return; 
	}

	tHashSet<unsigned long>* pInfluenceSet = pPlayer->GetInfluenceSet();
	if( !pInfluenceSet )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cSkillManager::SendAddSynMsgInfluenceToPlayer pInfluenceSet==NULL" );
		return; 
	}

	cInfluenceObject* pInf = NULL;

	tHashSet<unsigned long>::cIterator start = pInfluenceSet->Begin();
	tHashSet<unsigned long>::cIterator end = pInfluenceSet->End();

	HANDLE                     handle  = NULL;
	MSG_SYN_INFLUENCE_ADDLIST* sendMsg = (MSG_SYN_INFLUENCE_ADDLIST*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_SKILL, NM_SKILL_INFLUENCE_ADDLIST_SYN );

	if ( sendMsg != NULL )
	{
		sendMsg->mObject.type  = eOBJECTTYPE_PLAYER;
		sendMsg->mObject.index = characterIdx;

		sendMsg->mRowCount = 0;
		for( ; start != end ; ++start )
		{
			pInf = GetInfluence( (*start) );

			if( pInf == NULL )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cSkillManager::SendAddSynMsgInfluenceToPlayer pInfluenceObject[%d]==NULL", (*start) );
				//-/-//InsertDeleteInfluenceObject( pInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
				continue;
			}

			if( pInf->IsRealTime() == false && NETWORK2->GetAccumTime() >= pInf->GetInfluenceEndTime() && pInf->GetSwitchType() == eSWITCHTYPE_NORMAL )
				continue;
			if( pInf->IsRealTime() == true && TIMER->DiffRealTime( pInf->GetInfRealEndTime() ) < 0 && pInf->GetSwitchType() == eSWITCHTYPE_NORMAL )
				continue;

			sendMsg->mTable[sendMsg->mRowCount].mUniqueIdx = pInf->GetUniqueIdx();
			sendMsg->mTable[sendMsg->mRowCount].mInfluenceClassIdx = pInf->GetInfluenceClassIdx();
			sendMsg->mTable[sendMsg->mRowCount].mDeleteType = pInf->GetDeleteType();
			sendMsg->mTable[sendMsg->mRowCount].mIsRealTime = pInf->IsRealTime();
			sendMsg->mTable[sendMsg->mRowCount].mRestTime = 0;
			if( pInf->IsRealTime() == true )
			{
				tm tmTime = TIMER->ConvertTStoTM( pInf->GetInfRealEndTime() );
				time_t tTime = mktime( &tmTime );
				sendMsg->mTable[sendMsg->mRowCount].mEndRealTime = tTime;
			}
			else
			{
				if( NETWORK2->GetAccumTime() < pInf->GetInfluenceEndTime() )
					sendMsg->mTable[sendMsg->mRowCount].mRestTime = pInf->GetInfluenceEndTime() - NETWORK2->GetAccumTime();
			}
			++sendMsg->mRowCount;
		}

		NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	else
	{
		NETWORK2->PostServerEvent( "cSkillManager::SendAddSynMsgInfluenceToPlayer sendMsg(%d,%d) == NULL", connectionIdx, characterIdx );
	}
}

void cSkillManager::ClearPlayerHaveSkillCoolTime( unsigned long playerIdx, unsigned long exceptSkillIdx )
{
	cPointerHashMap* cPlayerHaveSkillMap = (cPointerHashMap*)mPlayerHaveSkillMap.GetAt( playerIdx );
	if ( !cPlayerHaveSkillMap )
	{
		return;
	}

	cPointerHashMap::cConstIterator pos = cPlayerHaveSkillMap->Begin();
	cPointerHashMap::cConstIterator end = cPlayerHaveSkillMap->End();
	for( ; pos != end; ++pos )
	{
		cHaveSkillObject* haveSkill = (cHaveSkillObject*)(*pos).mSecond;
		if( haveSkill && haveSkill->GetSkillIdx() != exceptSkillIdx )
			haveSkill->ClearRestTime();
	}
}
