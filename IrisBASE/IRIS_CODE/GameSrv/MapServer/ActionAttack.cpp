#include "StdAfx.h"
#include "actionattack.h"

#include "../Common/Protocol.h"

#include "Monster.h"
#include "SkillManager.h"
#include "Player.h"
#include "gamesrv.h"
#include "SkillScript.h"
#include "ObjectManager.h"



cActionAttack::cActionAttack(void) : cAction( eACT_ATTACK )
{
}



cActionAttack::~cActionAttack(void)
{
}



void cActionAttack::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionAttack::ActionInit pMonsterScript == NULL");
		return;
	}

	/// 추적에서 선택된 공격 방식으로 먼저 공격을 한다
	cBaseObject* pTarget = (cPlayer*)pMonster->GetTarget();
	if( pTarget == NULL )
		return;

	/// 스킬 사용후 지연시간 체크
	if( pMonster->IsSkillWaitEnd() == false )
		return;

	/// 모드 스킬이 가능하면 모드스킬을 쓴다
	eMONSTERATTACK_TYPE	attackType = pMonster->GetModeSkill();
	if( attackType == eMONSTERATTACK_MAX )
		attackType = pMonster->GetAttackType();

	/// 스킬index 체크
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( pMonster->GetRaceGender(), pMonster->GetAttackType() );
	if( pSkillInfo == NULL )	
		return; 

	if( pMonster->IsCantSkill( pSkillInfo->mAttributeType ) == true )
		return;

	sObject target = { pTarget->GetObjectType(), pTarget->GetObjectID() };
	SKILLMANAGER->MonsterAttack( pMonster->GetObjectID(), attackType, target );
	//if( SKILLMANAGER->MonsterAttack( pMonster->GetObjectID(), attackType, target ) == true )
	//{
	//	/// 스킬index 체크
	//	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( pMonster->GetRaceGender(), pMonster->GetAttackType() );
	//	if( pSkillInfo == NULL )	
	//	{ 
	//		assert(NULL); 
	//		NETWORK2->PostServerEvent("MonsterAttack - GetMonsterSkillInfo[%d,%d]", pMonster->GetRaceGender(), pMonster->GetAttackType() );
	//		return; 
	//	}

	//	unsigned long attackEndTime = 0;
	//	if( pMonster->GetAttackType() == eMONSTERATTACK_NORMAL1 ||
	//		pMonster->GetAttackType() == eMONSTERATTACK_NORMAL2 )
	//	{
	//		attackEndTime = accumTime + (unsigned long)( pSkillInfo->mAttackSpd * ( 2.0f - pMonster->GetAttackSpeed() ) );
	//	}
	//	else
	//		attackEndTime = accumTime + pSkillInfo->mAttackSpd;

	//	pMonster->CalcSkillWaitTime( attackEndTime );
	//}
}



eACTION_CHANGE cActionAttack::Action( cMonster* pMonster, unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionAttack::Action pMonsterScript == NULL");
		return eACTCH_ERROR;
	}

	/// 스킬 사용이 끝날때 까지 통과
	if( SKILLMANAGER->IsUsingSkill( eOBJECTTYPE_MONSTER, pMonster->GetObjectID() ) )
		return eACTCH_CONTINUE;

	/// 플레이어가 사라졌거나 플레이어가 있느데 죽은경우
	cBaseObject* pTarget = (cBaseObject*)pMonster->GetTarget();
	if( pTarget == NULL || pTarget->GetStateDie() == true || pTarget->GetObjectType() == eOBJECTTYPE_MONSTER ) 
	{
		/// 081205 PKH 타겟 선정
		pMonster->ChangeActionTarget();
		return eACTCH_CONTINUE;
	}

	if( pMonster->GetStateOddity( eODDITYTYPE_SLEEP ) != 0 )
		return eACTCH_CONTINUE; 
	if( pMonster->GetStateOddity( eODDITYTYPE_STUN ) != 0 )
		return eACTCH_CONTINUE; 
	if( pMonster->GetIdleEndTime() > accumTime )
		return eACTCH_CONTINUE;

	//////////////////////////////////////////////////////////////////////////
	/// 도망 공격시 애니메이션 스킵현상때문에 공격속도끝날때까지 다른 행동안함

	///// 스킬 사용후 지연시간 체크
	//if( pMonster->IsSkillWaitEnd() == false )
	//{
	//	float range; // = pMonster->CalcStatusSkillRange( pSkillInfo->mTargetDist, pSkillInfo->mRangeType );

	//	//range = OBJECTMANAGER->ObjectSizeRange( pMonster, pPlayer, range );
	//	range = OBJECTMANAGER->ObjectSizeRange( pMonster, pPlayer, 0 );

	//	cRangeCheck skillRangeCheck( range );

	//	NiPoint3 attackerPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
	//	NiPoint3 targetPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );

	//	/// 스킬 사거리 밖에 타겟이 있다.
	//	if( skillRangeCheck.IsNotRange( attackerPos, targetPos ) == true )	
	//		return eATTCH_ATTACK_FOLLOW;
	//	else
	//		return eACTCH_CONTINUE; 
	//}
	//////////////////////////////////////////////////////////////////////////

	/// 스킬 사용후 지연시간 체크
	if( pMonster->IsSkillWaitEnd() == false )
		return eACTCH_CONTINUE; 

	pMonster->SetAttackType( eMONSTERATTACK_MAX );

	/// 스킬 사용 불가 상태
	eMONSTERATTACK_TYPE attackType = pMonster->SelectSkill();
	if( attackType == eMONSTERATTACK_MAX )
		return eATTCH_ATTACK_FOLLOW;

	pTarget = pMonster->GetTarget();
	pTarget = (cBaseObject*)pMonster->GetTarget();
	if( pTarget == NULL || pTarget->GetStateDie() == true ) 
	{
		return eACTCH_CONTINUE;
	}

	float targetDist = 0;
	eRANGETYPE rangeType = eRANGETYPE_CLOSE;
	unsigned long attackSpeed = 1000;
	/// 스킬index 체크
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( pMonster->GetRaceGender(), attackType );
	if( pSkillInfo == NULL )	
	{ 
		NETWORK2->PostServerEvent("cActionAttack::Action pSkillInfo[%d,%d] == NULL", pMonster->GetRaceGender(), attackType );
	}
	else
	{
		targetDist = pSkillInfo->mTargetDist;
		rangeType = pSkillInfo->mRangeType;
		attackSpeed = pSkillInfo->mAttackSpd;
	}

	float range = pMonster->CalcStatusSkillRange( targetDist, rangeType );

	range = OBJECTMANAGER->ObjectSizeRange( pMonster, pTarget, range );
	//range = OBJECTMANAGER->ObjectSizeRange( pMonster, pPlayer, 0 );

	cRangeCheck skillRangeCheck( range );

	NiPoint3 attackerPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
	NiPoint3 targetPos( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height() );

	/// 스킬 사거리 밖에 타겟이 있다.
	if( skillRangeCheck.IsNotRange( attackerPos, targetPos ) == true )	
		return eATTCH_ATTACK_FOLLOW;

	sObject target = { pTarget->GetObjectType(), pTarget->GetObjectID() };
	if( SKILLMANAGER->MonsterAttack( pMonster->GetObjectID(), attackType, target ) == false )
		return eATTCH_ATTACK_FOLLOW;	/// 스킬 사용 불가 상태이므로 추적

	//unsigned long attackEndTime = 0;
	//if( pMonster->GetAttackType() == eMONSTERATTACK_NORMAL1 ||
	//	pMonster->GetAttackType() == eMONSTERATTACK_NORMAL2 )
	//{
	//	attackEndTime = accumTime + (unsigned long)( attackSpeed * ( 2.0f - pMonster->GetAttackSpeed() ) );
	//}
	//else
	//	attackEndTime = accumTime + attackSpeed;

	//pMonster->CalcSkillWaitTime( attackEndTime );
	/// 공격 성공이면 추적 시간 재설정
	pMonster->SetFollowEndTime( accumTime + pMonsterScript->mFollowTime );

	/// 공격 프로세스 계속 수행
	return eACTCH_CONTINUE;
}
