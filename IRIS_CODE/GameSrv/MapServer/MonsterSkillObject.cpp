#include "stdafx.h"
#include "Player_Common.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "BaseObject.h"
#include "DamageCalc.h"
#include "Protocol.h"
#include "Player.h"
#include "Monster.h"
#include "gamesrv.h"
#include "Drop.h"
#include "GridManager.h"
#include "AIManager.h"
#include "StageScript.h"



cMonsterSkillObject::cMonsterSkillObject()
{
	mMonsterClassIdx = 0;
}



cMonsterSkillObject::~cMonsterSkillObject()
{
}



void* cMonsterSkillObject::operator new( size_t n )
{
	if( n != sizeof(cMonsterSkillObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocMonsterSkillObject();
}



void cMonsterSkillObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cMonsterSkillObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreeMonsterSkillObject( static_cast<cMonsterSkillObject*>(ptr) );

	return;
}



bool cMonsterSkillObject::InitMonsterAttack( unsigned long monsterIdx, unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType,
											sObject target, unsigned long uniqueIdx, bool casting, float speedFactor, unsigned long mapNumber )
{
	/// 고유 idx 셋팅
	mUniqueIdx = uniqueIdx;

	/// 몬스터 인덱스
	mMonsterClassIdx = monsterClassIdx;

	mDramaKey = monsterClassIdx;
	/// 스킬idx의 값을 셋팅.
	mSkillIdx = attackType;

	mMapNumber = mapNumber;

	/// 공격자 셋팅
	mAttacker.index = monsterIdx;
	mAttacker.type = eOBJECTTYPE_MONSTER;

	mSpeedFactor = speedFactor;

	/// 타겟 셋팅
	mCenterTarget = target;

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )
	{
		/// 스킬 제거
		assert(NULL);
		NETWORK2->PostServerEvent("InitMonsterAttack - pMonster == NULL");
		return false;
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
	if( pDramaInfo == NULL )
	{
		/// 스킬 제거
		assert(NULL);
		NETWORK2->PostServerEvent("InitMonsterAttack - pDramaInfo == NULL");
		return false;
	}
	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitMonsterAttack, GetDramaState error [%d,%d]", mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
		return false;
	}

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();

	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return false;

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pMonster->GetPos();

	/// 발사물 타겟위치를 시전자 기준 범위로 발사물 처리할대 목적지점을 바꾼다.
	mCenterPos.x = pMonster->GetXPos();
	mCenterPos.y = pMonster->GetYPos();
	mCenterPos.z = pMonster->Height();

	sMonsterSkillScript* pMonsterSkill = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, attackType );
	if( pMonsterSkill == NULL )
	{
		NETWORK2->PostServerEvent("cMonsterSkillObject::InitMonsterAttack pMonsterSkill[%d,%d] == NULL", mMonsterClassIdx, attackType);
		return false;
	}

	if( mDetailCnt > 1 )
	{
		if( pMonsterSkill->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pMonsterSkill->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return false;
	}

	/// 타겟 해제 스킬
	if( pMonsterSkill->mApplyValueType1 == eSTATUSPLUS_DELETE_TARGETING || pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_DELETE_TARGETING )
		mCreateType = eSKILLCREATETYPE_DELTARGET;

	if( pMonsterSkill->mShotType == eMONSTERSHOTTYPE_SPOT )
		mCastingTime = 0;
	else
		mCastingTime = pMonsterSkill->mCastingTime;		

	if( casting == true )
		mSkillState = eSKILLPROCESS_CASTING;
	else
		mSkillState = eSKILLPROCESS_ACTIVITY;

	mAttributeType = pMonsterSkill->mAttributeType;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pMonster, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}


bool cMonsterSkillObject::InitMonsterSkillPosition( unsigned long monsterIdx, unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType, 
												 float targetXPos, float targetYPos, unsigned long uniqueIdx, bool casting, unsigned long mapNumber )
{
	/// 고유 idx 셋팅
	mUniqueIdx = uniqueIdx;

	/// 몬스터 인덱스
	mMonsterClassIdx = monsterClassIdx;

	mDramaKey = monsterClassIdx;
	/// 스킬idx의 값을 셋팅.
	mSkillIdx = attackType;

	mMapNumber = mapNumber;

	/// 공격자 셋팅
	mAttacker.index = monsterIdx;
	mAttacker.type = eOBJECTTYPE_MONSTER;

	/// 타겟오브젝트는 없다.
	mCenterTarget.index = 0;
	mCenterTarget.type = eOBJECTTYPE_NONE;

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )
	{
		NETWORK2->PostServerEvent("cMonsterSkillObject::InitMonsterSkillPosition pMonster == NULL[%d]", mMonsterClassIdx );
		return false;
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetMonsterDramaturgyInfo( mMonsterClassIdx, attackType );
	if( pDramaInfo == NULL )
	{
		/// 스킬 제거
		NETWORK2->PostServerEvent("cMonsterSkillObject::InitMonsterSkillPosition - pDramaInfo == NULL[%d]", mMonsterClassIdx );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonsterSkillObject::InitMonsterSkillPosition pDramaState == NULL[%d,%d]", mMonsterClassIdx, mSkillIdx );
		return false;
	}

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();

	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return false;

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pMonster->GetPos();

	/// 기준좌표 셋팅
	mCenterPos.x = targetXPos;
	mCenterPos.y = targetYPos;
	AIMANAGER->CalcHeight( pMonster->GetMapNumber(), &mCenterPos.z, mCenterPos.x, mCenterPos.y );

	sMonsterSkillScript* pMonsterSkill = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, attackType );
	if( pMonsterSkill == NULL )
	{
		NETWORK2->PostServerEvent("cMonsterSkillObject::InitMonsterAttack pMonsterSkill[%d,%d] == NULL", mMonsterClassIdx, attackType);
		return false;
	}

	if( mDetailCnt > 1 )
	{
		if( pMonsterSkill->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pMonsterSkill->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pMonsterSkill->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return false;
	}

	if( pMonsterSkill->mShotType == eMONSTERSHOTTYPE_SPOT )
		mCastingTime = 0;
	else
		mCastingTime = pMonsterSkill->mCastingTime;		

	if( casting == true )
		mSkillState = eSKILLPROCESS_CASTING;
	else
		mSkillState = eSKILLPROCESS_ACTIVITY;

	mAttributeType = pMonsterSkill->mAttributeType;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pMonster, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}


bool cMonsterSkillObject::CastingObjectCheck( cBaseObject* pAttacker )
{
	unsigned char errorCode = FAIL_SKILL_CASTING_DONT_TARGET;

	/// 생존 여부 확인
	if( pAttacker->GetStateDie() == false )
	{
		sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
		if( pSkillInfo != NULL )
		{
			if( pAttacker->IsCantSkill( pSkillInfo->mAttributeType ) == false )
			{
				/// 범위 방식에 따른 타겟 객체 확인 여부
				switch( pSkillInfo->mBoundType )
				{
				case eBOUNDTYPE_SELF_NOTARGET:
				case eBOUNDTYPE_FIELD:
					return true;
				case eBOUNDTYPE_NONE:			/// 범위 없음
				case eBOUNDTYPE_TARGET:			/// 타겟 기준범위
				case eBOUNDTYPE_SELF_TARGET:	/// 시전자(자신) 기준범위 - 발동시 타겟 존재
					{
						/// 자신이 적용 대상이면 리턴
						if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF ) 
							return true;
					}
				}

				/// 타겟 객체 체크
				cBaseObject* pTarget = OBJECTMANAGER->GetObject( mCenterTarget.type, mCenterTarget.index );
				if( pTarget != NULL && pTarget->GetStateDie() == false )	
					return true;
			}
			else
				errorCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;
		}
		else
		{
			NETWORK2->PostServerEvent("cMonsterSkillObject[%d]::CastingObjectCheck pSkillInfo[%d,%d] == NULL", pAttacker->GetObjectID(), mMonsterClassIdx, mSkillIdx);
			errorCode = FAIL_SKILL_CASTING_DONT_SCRIPT;
		}
	}
	else
		errorCode = FAIL_SKILL_CASTING_DONT_ATTACKER;

	/// 캐스팅 발동 실패 메세지 전송
	MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
	failMsg.Category = NM_SKILL;
	failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
	failMsg.mAttacker = mAttacker;
	failMsg.mSkillIndex = mUniqueIdx;
	failMsg.mFailCode = errorCode;

	NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

	/// 조건이 만족하는 경우 상단에 return true로 빠져나감 여기까지 온경우 조건을 만족 못하는 경우임
	return false;
}



/// 주의 : 캐스팅 하는 경우만 이함수로 들어옴
bool cMonsterSkillObject::RequireValue( cBaseObject* pAttacker )
{	
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );

	/// 소모 체크
	if( pAttacker->GetMP() < pSkillInfo->mUseMP )	
	{ 
		/// 캐스팅 발동 실패 메세지 전송
		MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
		failMsg.Category = NM_SKILL;
		failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
		failMsg.mSkillIndex = mUniqueIdx;
		failMsg.mFailCode = FAIL_SKILL_CASTING_MP_EMPTY;

		NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

		return false; 
	}

	/// MP를 소모 하는 스킬
	if( pSkillInfo->mUseMP != 0 )
		pAttacker->MPUse( pSkillInfo->mUseMP, false );

	((cMonster*)pAttacker)->UpdateSkillCoolTime( (eMONSTERATTACK_TYPE)mSkillIdx );

	/// 캐스팅 스킬 발동을 알리는 싱크 전송
	MSG_SYN_SKILL_CAST_ACTIVE msgEtc;
	msgEtc.Category = NM_SKILL;
	msgEtc.Protocol = NM_SKILL_CAST_ACTIVE_SYN;	
	msgEtc.mSkillIndex = mUniqueIdx;
	msgEtc.mAttacker = mAttacker;
	msgEtc.mAttackerPosX = pAttacker->GetXPos();
	msgEtc.mAttackerPosY = pAttacker->GetYPos();

	NETWORK2->QuickSendExcept( pAttacker, (char*)&msgEtc, sizeof(msgEtc) );

	return true;
}



bool cMonsterSkillObject::SetCenterPos()
{
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
	/// 데미지 적용시 기준이 될 위치좌표 산출
	switch( pSkillInfo->mBoundType )
	{
	case eBOUNDTYPE_NONE:			/// 범위 공격이 아님
	case eBOUNDTYPE_TARGET:			/// 타겟 기준범위
		{
			/// 사거리 체크 대상이 시전자인경우 필요 없음
			if( pSkillInfo->mApplyType != eAPPLYTYPE_SELF )
			{ 
				cBaseObject* pObject = OBJECTMANAGER->GetObject( mCenterTarget.type, mCenterTarget.index );
				/// 발동 전에 타겟이 사라진경우 데미지 미스
				if( pObject == NULL || pObject->GetMapNumber() != mMapNumber )	
					return false;

				mCenterPos.x = pObject->GetXPos();
				mCenterPos.y = pObject->GetYPos();
				mCenterPos.z = pObject->Height();
			}
		}break;
	}

	return true;
}


void cMonsterSkillObject::TargetSelect( cBaseObject* pAttacker )
{
	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonsterSkillObject[%d]::TargetSelect pSkillInfo[%d,%d] == NULL", 
			pAttacker->GetObjectID(), mMonsterClassIdx, mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}

	/// 발사체중 타겟을 향해 날아 가다 타겟이 사라진경우 데미지 미스처리 여부
	if( mNoTargetNoDamage == true )
		return;

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
	bool applyHP = pSkillInfo->mType == eMONSTERSKILLTYPE_MPDAMAGE ? false : true;

	/// 타겟 선정
	if( pSkillInfo->mBoundType != eBOUNDTYPE_NONE )
	{
		switch( pSkillInfo->mApplyType )
		{
		case eAPPLYTYPE_SELF:
			{
				//sMultiTarget multiTarget;
				//multiTarget.mTarget.type = eOBJECTTYPE_PLAYER;
				//multiTarget.mTarget.index = pAttacker->GetObjectID();
				////multiTarget.mApplyHP = applyHP;
				//mTargetAry.PushBack( multiTarget );
				sObject object = { eOBJECTTYPE_PLAYER, pAttacker->GetObjectID() };
				SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, object );
			}
			break;
		case eAPPLYTYPE_BUDDY:
			{
				//mTargetAry.Clear();
				unsigned char targetCnt = 0;
				
				SelectMonsterTarget( pAttacker, mCenterPos, pSkillInfo->mBoundDist1, pSkillInfo->mBoundDist2, &targetCnt, 
					targetMaxCnt, applyHP, pSkillInfo->mBoundType );
				
				//float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, NULL, pSkillInfo->mBoundDist1 );

				///// 범위 공격인경우
				//OBJECTMANAGER->MonsterInCenterPoint( pAttacker->GetMapNumber(), mCenterPos, tempRange, &mTargetAry, applyHP, targetMaxCnt );
			}break;
		case eAPPLYTYPE_ENEMY:
		case eAPPLYTYPE_FAR:
		case eAPPLYTYPE_RANDOM:
		case eAPPLYTYPE_DISTRESS2:
		case eAPPLYTYPE_DISTRESS3:
		case eAPPLYTYPE_DISTRESSMIN:
		case eAPPLYTYPE_HPMAX:
		case eAPPLYTYPE_HPMIN:
			{
				//mTargetAry.Clear();
				unsigned char targetCnt = 0;

				SelectPlayerTarget( pAttacker, mCenterPos, pSkillInfo->mBoundDist1, pSkillInfo->mBoundDist2, &targetCnt, 
					targetMaxCnt, applyHP, true, pSkillInfo->mBoundType );

				//float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, NULL, pSkillInfo->mBoundDist1 );
				///// 범위 공격인경우
				//OBJECTMANAGER->PlayerInCenterPoint( pAttacker->GetMapNumber(), mCenterPos, tempRange, &mTargetAry, applyHP, targetMaxCnt );
			}break;
		}
	}
	else
	{
		//sMultiTarget multiTarget;
		//multiTarget.mTarget = mCenterTarget;
		////multiTarget.mApplyHP = applyHP;
		//mTargetAry.PushBack( multiTarget );

		SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, mCenterTarget );
	}
}

void cMonsterSkillObject::ApplyDamage( cBaseObject* pAttacker )
{

	sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( mMonsterClassIdx, (eMONSTERATTACK_TYPE)mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonsterSkillObject[%d]::ApplyDamage pSkillInfo[%d,%d] == NULL", 
			pAttacker->GetObjectID(), mMonsterClassIdx, mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}

	/// 타입별 데미지 처리
	switch( pSkillInfo->mType )
	{
	case eMONSTERSKILLTYPE_DAMAGE:
		{
			SendDamage( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pSkillInfo->mRangeType,
				pSkillInfo->mApplyValueType1, pSkillInfo->mApplyValueType2, pSkillInfo->mApplyValue1, pSkillInfo->mApplyValue2,
				pSkillInfo->mInfulenceIdx, pSkillInfo->mAccuracyValue, pSkillInfo->mCriticalValue, true );	
		}
		break;
	case eMONSTERSKILLTYPE_HEAL:
		{
			SendHeal( pAttacker, pSkillInfo->mShotType,
				pSkillInfo->mApplyValueType1, pSkillInfo->mApplyValueType2, pSkillInfo->mApplyValue1, pSkillInfo->mApplyValue2,
				pSkillInfo->mInfulenceIdx );
		}
		break;
	case eMONSTERSKILLTYPE_NONACTIVE:
		{
			if( mCreateType == eSKILLCREATETYPE_DELTARGET )
			{
				SendDelTarget( pAttacker, pSkillInfo->mAccuracyValue );
			}
			else
			{
				SendApply( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pSkillInfo->mAccuracyValue,
					pSkillInfo->mApplyValueType1, pSkillInfo->mApplyValueType2, 
					pSkillInfo->mApplyValue1, pSkillInfo->mApplyValue2, pSkillInfo->mInfulenceIdx );
			}
		}
		break;
	case eMONSTERSKILLTYPE_MPDAMAGE:
		{
			SendDamage( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pSkillInfo->mRangeType,
				pSkillInfo->mApplyValueType1, pSkillInfo->mApplyValueType2, pSkillInfo->mApplyValue1, pSkillInfo->mApplyValue2,
				pSkillInfo->mInfulenceIdx, pSkillInfo->mAccuracyValue, pSkillInfo->mCriticalValue, false );	
		}
		break;
	case eMONSTERSKILLTYPE_TOTEM:
		{
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

			AddTotem( pAttacker, 0, 0, pSkillInfo->mApplyType, pSkillInfo->mAttributeType, pSkillInfo->mInfulenceIdx, targetMaxCnt );
		}
		break;
	}
}
