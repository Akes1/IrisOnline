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
#include "StatusCalc_Server.h"
#include "AIManager.h"
#include "Party.h"
#include "PartyManager.h"
#include "GridManager.h"
#include "PartyUnion.h"
#include "PartyUnionManager.h"

cPlayerSkillObject::cPlayerSkillObject()
{
	mSkillIdx = 0;
	mSkillStep = 0;
}



cPlayerSkillObject::~cPlayerSkillObject()
{
}



void* cPlayerSkillObject::operator new( size_t n )
{
	if( n != sizeof(cPlayerSkillObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocPlayerSkillObject();
}



void cPlayerSkillObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cPlayerSkillObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreePlayerSkillObject( static_cast<cPlayerSkillObject*>(ptr) );

	return;
}



unsigned char cPlayerSkillObject::InitPlayerNormalAttack( unsigned long playerIdx, sObject target, unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerNormalAttack, playerIdx error [%d]", playerIdx );
		return 1;
	}

	/// skill pc정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pSkillInfo == NULL )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerNormalAttack - GetPlayerSkillInfo(%d)", skillIdx );
		return 2; 
	}

	/// 공격자의 보유스킬인지 체크
	cHaveSkillObject* pPlayerHaveSkill = SKILLMANAGER->GetPlayerHaveSkill( playerIdx, skillIdx );
	if( !pPlayerHaveSkill )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerNormalAttack - GetPlayerHaveSkill(%d,%d)", playerIdx, skillIdx  );
		return 3; 
	}

	/// 스킬 스텝정보 읽어옴
	unsigned char haveStep = pPlayerHaveSkill->GetSkillStep();
	if( haveStep >= pSkillInfo->mStepCount )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerNormalAttack - mSkillStep(%d)", haveStep );
		return 4; 
	}
	if( haveStep < skillStep )
	{
		NETWORK2->PostServerEvent("InitPlayerNormalAttack - haveStep < skillStep(%d,%d)", haveStep, skillStep );
		return 5; 
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
		pPlayer->GetRace(), pPlayer->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerNormalAttack, pDramaInfo error [%d,%d,%d,%d,%d]", 
			playerIdx, skillIdx, (unsigned short)pPlayer->GetWeaponState(), pPlayer->GetRace(), pPlayer->GetGender() );
		return 6;
	}
	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkill, GetDramaState error [%d,%d,%d,%d,%d]", 
			playerIdx, skillIdx, (unsigned short)pPlayer->GetWeaponState(), pPlayer->GetRace(), pPlayer->GetGender() );
		return 7;
	}

	/// 공격자 셋팅
	mAttacker.index = playerIdx;
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;

	mSkillStep = skillStep;
	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();
	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return 8;

	if( mDetailCnt > 1 )
	{
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return 9;
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return 10;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return 11;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return 12;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return 13;
	}

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();

	/// 고유 idx 셋팅
	mUniqueIdx = uniqueIdx;

	/// 공격속도 ( 아이템, 버프 포함 )
	float attackSpeed = pPlayer->GetStatus2()->mAttackSpeed;

	if( attackSpeed <= 0.0f || attackSpeed > 2.0f )
		attackSpeed = 1.0f;

	mSpeedFactor = attackSpeed;

	//
	mMapNumber = pPlayer->GetMapNumber();

	/// 타겟 셋팅
	mCenterTarget = target;
	
	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pPlayer->GetPos();
	/// 발사물 타겟위치를 시전자 기준 범위로 발사물 처리할대 목적지점을 바꾼다.
	mCenterPos.x = pPlayer->GetXPos();
	mCenterPos.y = pPlayer->GetYPos();
	mCenterPos.z = pPlayer->Height();

	mSkillState = eSKILLPROCESS_ACTIVITY;
	mCastingTime = 0;

	mDistressPoint = pStepInfo->mDistressValue;
	mAttributeType = pSkillInfo->mAttributeType;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pPlayer, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	/// 쿨타임 갱신
	SKILLMANAGER->UpdateSkillCoolTime( mAttacker, mSkillIdx, mSkillStep, NETWORK2->GetAccumTime(), mSpeedFactor );

	return 0;
}



bool cPlayerSkillObject::InitPlayerSkill( unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx, cPlayer* pAttacker, cBaseObject* pTarget, unsigned long castingTime )
{
	if( pAttacker == 0 )
		return false;

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// skill pc정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pSkillInfo == NULL )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkill - GetPlayerSkillInfo(%d)", skillIdx );
		return false; 
	}

	/// 공격자의 보유스킬인지 체크
	cHaveSkillObject* pPlayerHaveSkill = SKILLMANAGER->GetPlayerHaveSkill( mAttacker.index, skillIdx );
	if( !pPlayerHaveSkill )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkill - GetPlayerHaveSkill(%d,%d)", mAttacker.index, skillIdx  );
		return false; 
	}

	/// 스킬 스텝정보 읽어옴
	unsigned char haveStep = pPlayerHaveSkill->GetSkillStep();
	if( haveStep >= pSkillInfo->mStepCount )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkill - mSkillStep(%d)", haveStep );
		return false; 
	}
	if( haveStep < skillStep ) 
	{
		NETWORK2->PostServerEvent("InitPlayerSkill - haveStep < skillStep(%d,%d)", haveStep, skillStep );
		return false; 
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerSkill - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}
	
	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkill, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)pAttacker->GetWeaponState(), pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;
	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mSkillStep = skillStep;

	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	/// 타겟 셋팅
	if( pTarget )
	{
		mCenterTarget.index = pTarget->GetObjectID();
		mCenterTarget.type = pTarget->GetObjectType();
	}
	else
	{
		mCenterTarget.index = 0;
		mCenterTarget.type = eOBJECTTYPE_NONE;
	}

	/// 공격속도 ( 아이템, 버프 포함 )
	float attackSpeed = pAttacker->GetStatus2()->mAttackSpeed;

	if( attackSpeed <= 0.0f || attackSpeed > 2.0f )
		attackSpeed = 1.0f;

	mSpeedFactor = attackSpeed;

	//
	mMapNumber = pAttacker->GetMapNumber();

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();

	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return false;

	if( mDetailCnt > 1 )
	{
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return false;
	}

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// 발사물 타겟위치를 시전자 기준 범위로 발사물 처리할대 목적지점을 바꾼다.
	mCenterPos.x = pAttacker->GetXPos();
	mCenterPos.y = pAttacker->GetYPos();
	mCenterPos.z = pAttacker->Height();

	mDistressPoint = pStepInfo->mDistressValue;
	mAttributeType = pSkillInfo->mAttributeType;

	/// 타겟 해제 스킬
	if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_DELETE_TARGETING || pStepInfo->mApplyValueType2 == eSTATUSPLUS_DELETE_TARGETING)
        mCreateType = eSKILLCREATETYPE_DELTARGET;
	else if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_INSTANT_CLEAR_SKILL_COOLTIME || pStepInfo->mApplyValueType2 == eSTATUSPLUS_INSTANT_CLEAR_SKILL_COOLTIME )
		mCreateType = eSKILLCREATETYPE_CLEAR_COOLTIME;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	if( pTarget )
	{
		/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
		pMsgTarget = GRIDMANAGER->FindFirstPlayer( pTarget, true );
		while( pMsgTarget != NULL )
		{
			mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
			pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
		}
	}

	mCastingTime = pAttacker->CalcCastingExt( castingTime );
	if( mCastingTime != 0 )
	{
		mSkillState = eSKILLPROCESS_CASTING;
	}
	else
	{
		mSkillState = eSKILLPROCESS_ACTIVITY;

		/// 쿨타임 갱신
		SKILLMANAGER->UpdateSkillCoolTime( mAttacker, mSkillIdx, mSkillStep, NETWORK2->GetAccumTime(), mSpeedFactor );
	}

	return true;
}



bool cPlayerSkillObject::InitPlayerSkillPosition( unsigned long playerIdx, float targetXPos, float targetYPos, unsigned long skillIdx, 
												 unsigned char skillStep, unsigned long uniqueIdx, bool casting )
{

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - playerIdx[%d]", playerIdx );
		return false;
	}

	/// 공격자 셋팅
	mAttacker.index = playerIdx;
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// skill pc정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pSkillInfo == NULL )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - GetPlayerSkillInfo(%d)", skillIdx );
		return false; 
	}

	/// 공격자의 보유스킬인지 체크
	cHaveSkillObject* pPlayerHaveSkill = SKILLMANAGER->GetPlayerHaveSkill( mAttacker.index, skillIdx );
	if( !pPlayerHaveSkill )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - GetPlayerHaveSkill(%d,%d)", mAttacker.index, skillIdx  );
		return false; 
	}

	/// 스킬 스텝정보 읽어옴
	unsigned char haveStep = pPlayerHaveSkill->GetSkillStep();
	if( haveStep >= pSkillInfo->mStepCount )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - mSkillStep(%d)", haveStep );
		return false; 
	}
	if( haveStep < skillStep ) 
	{
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - haveStep < skillStep(%d,%d)", haveStep, skillStep );
		return false; 
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
		pPlayer->GetRace(), pPlayer->GetGender() ,&mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkillPosition - pDramaInfo[%d,%d,%d,%d]", skillIdx, (unsigned short)pPlayer->GetWeaponState(), 
			pPlayer->GetRace(), pPlayer->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkill, GetDramaState error [%d,%d,%d,%d,%d]", 
			playerIdx, skillIdx, (unsigned short)pPlayer->GetWeaponState(), pPlayer->GetRace(), pPlayer->GetGender() );
		return false;
	}

	/// 고유 idx 셋팅
	mUniqueIdx = uniqueIdx;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mSkillStep = skillStep;
	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	/// 공격속도 ( 아이템, 버프 포함 )
	float attackSpeed = pPlayer->GetStatus2()->mAttackSpeed;

	if( attackSpeed <= 0.0f || attackSpeed > 2.0f )
		attackSpeed = 1.0f;

	mSpeedFactor = attackSpeed;

	mMapNumber = pPlayer->GetMapNumber();

	/// 타겟오브젝트는 없다.
	mCenterTarget.index = 0;
	mCenterTarget.type = eOBJECTTYPE_NONE;

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();

	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return false;

	if( mDetailCnt > 1 )
	{
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return false;
	}

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pPlayer->GetPos();

	/// 기준좌표 셋팅
	mCenterPos.x = targetXPos;
	mCenterPos.y = targetYPos;
	AIMANAGER->CalcHeight( pPlayer->GetMapNumber(), &mCenterPos.z, mCenterPos.x, mCenterPos.y );

	mDistressPoint = pStepInfo->mDistressValue;
	mAttributeType = pSkillInfo->mAttributeType;

	if( pSkillInfo->mShotType == eSHOTTYPE_CASTING )
	{
		mCastingTime = pPlayer->CalcCastingExt( pStepInfo->mCastingTime );
	}
	else
		mCastingTime = 0;

	if( casting == true )
		mSkillState = eSKILLPROCESS_CASTING;
	else
	{
		mSkillState = eSKILLPROCESS_ACTIVITY;
		/// 쿨타임 갱신
		SKILLMANAGER->UpdateSkillCoolTime( mAttacker, mSkillIdx, mSkillStep, NETWORK2->GetAccumTime(), mSpeedFactor );
	}

	if( pSkillInfo->mShotType == eSHOTTYPE_CHANNELING )
	{
		mIsChanneling = true;
		mpChannelDramaState = pDramaState;
		mChannelingTime = pStepInfo->mCastingTime + mCreationTime;
	}

	return true;
}

bool cPlayerSkillObject::InitPlayerMapChange( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long useItemDBIdx, unsigned long castingTime )
{
	if( pAttacker == 0 )
		return false;

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mMapNumber = pAttacker->GetMapNumber();

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// 타겟 셋팅
	//sMultiTarget multiTarget;
	//multiTarget.mTarget.type = eOBJECTTYPE_PLAYER;
	//multiTarget.mTarget.index = pAttacker->GetObjectID();
	////multiTarget.mApplyHP = true;
	//mTargetAry.PushBack( multiTarget );
	sObject object = { eOBJECTTYPE_PLAYER, pAttacker->GetObjectID() };
	SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, object );

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	if( useItemDBIdx == 0 )
		return false;

	mCreateType = eSKILLCREATETYPE_MAPCHANGE;
	mUseItemIdx = useItemDBIdx;

	cDramaturgyInfo* pDramaInfo;
	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerMapchange - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerMapchange, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)eWEAPON_STATE_NONE, pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	mDetailCnt = 0;
	mpDetailBegin = 0;
	mpDetailEnd = 0;

	/// 발사체 유무
	mIsProjectile = false;
	/// 발사체 속도
	mProjectileSpeed = 0;
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// skill pc정보 읽어옴
	mCastingTime = castingTime;
	if( mCastingTime != 0 )
		mSkillState = eSKILLPROCESS_CASTING;
	else
		mSkillState = eSKILLPROCESS_ACTIVITY;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}


bool cPlayerSkillObject::InitPlayerVehicle( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long useItemIdx, unsigned long castingTime )
{
	if( pAttacker == 0 )
		return false;

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mMapNumber = pAttacker->GetMapNumber();

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// 타겟 셋팅
	//sMultiTarget multiTarget;
	//multiTarget.mTarget.type = eOBJECTTYPE_PLAYER;
	//multiTarget.mTarget.index = pAttacker->GetObjectID();
	////multiTarget.mApplyHP = true;
	//mTargetAry.PushBack( multiTarget );
	sObject object = { eOBJECTTYPE_PLAYER, pAttacker->GetObjectID() };
	SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, object );


	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	if( useItemIdx == 0 )
		return false;

	mCreateType = eSKILLCREATETYPE_VEHICLE;
	mUseItemIdx = useItemIdx;

	cDramaturgyInfo* pDramaInfo;
	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerVehicle - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerVehicle, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)eWEAPON_STATE_NONE, pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	mDetailCnt = 0;
	mpDetailBegin = 0;
	mpDetailEnd = 0;

	/// 발사체 유무
	mIsProjectile = false;
	/// 발사체 속도
	mProjectileSpeed = 0;
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// skill pc정보 읽어옴
	mCastingTime = castingTime;
	if( mCastingTime != 0 )
		mSkillState = eSKILLPROCESS_CASTING;
	else
		mSkillState = eSKILLPROCESS_ACTIVITY;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}


bool cPlayerSkillObject::InitPlayerBlink( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker,
								NiPoint2 targetPos )
{
	if( pAttacker == 0 )
		return false;

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mMapNumber = pAttacker->GetMapNumber();

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// 타겟 셋팅
	//sMultiTarget multiTarget;
	//multiTarget.mTarget.type = eOBJECTTYPE_PLAYER;
	//multiTarget.mTarget.index = pAttacker->GetObjectID();
	////multiTarget.mApplyHP = true;
	//mTargetAry.PushBack( multiTarget );
	sObject object = { eOBJECTTYPE_PLAYER, pAttacker->GetObjectID() };
	SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, object );

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	mCreateType = eSKILLCREATETYPE_BLINK;
	mCenterPos.x = targetPos.x;
	mCenterPos.y = targetPos.y;
	AIMANAGER->CalcHeight( pAttacker->GetMapNumber(), &mCenterPos.z, mCenterPos.x, mCenterPos.y );

	cDramaturgyInfo* pDramaInfo;
	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerVehicle - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerVehicle, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)eWEAPON_STATE_NONE, pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	mDetailCnt = 0;
	mpDetailBegin = 0;
	mpDetailEnd = 0;

	/// 발사체 유무
	mIsProjectile = false;
	/// 발사체 속도
	mProjectileSpeed = 0;
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// skill pc정보 읽어옴
	mCastingTime = 0;
	mSkillState = eSKILLPROCESS_ACTIVITY;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}


bool cPlayerSkillObject::InitCashMapChange( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long castingTime, 
								  unsigned short mapNum, float x, float y, unsigned long useItemIdx )
{
	if( pAttacker == 0 )
		return false;

	mCashMapNum = mapNum;
	mCashX = x;
	mCashY = y;

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;

	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mMapNumber = pAttacker->GetMapNumber();

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	mUseItemIdx = useItemIdx;

	/// 타겟 셋팅
	//sMultiTarget multiTarget;
	//multiTarget.mTarget.type = eOBJECTTYPE_PLAYER;
	//multiTarget.mTarget.index = pAttacker->GetObjectID();
	////multiTarget.mApplyHP = true;
	//mTargetAry.PushBack( multiTarget );
	sObject object = { eOBJECTTYPE_PLAYER, pAttacker->GetObjectID() };
	SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, object );

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	mCreateType = eSKILLCREATETYPE_CASHMAPCHANGE;

	cDramaturgyInfo* pDramaInfo;
	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerMapchange - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)eWEAPON_STATE_NONE, 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerMapchange, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)eWEAPON_STATE_NONE, pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	mDetailCnt = 0;
	mpDetailBegin = 0;
	mpDetailEnd = 0;

	/// 발사체 유무
	mIsProjectile = false;
	/// 발사체 속도
	mProjectileSpeed = 0;
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// skill pc정보 읽어옴
	mCastingTime = castingTime;
	if( mCastingTime != 0 )
		mSkillState = eSKILLPROCESS_CASTING;
	else
		mSkillState = eSKILLPROCESS_ACTIVITY;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	return true;
}

bool cPlayerSkillObject::InitRushAfterSkill( unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx, cPlayer* pAttacker, cBaseObject* pTarget, unsigned long castingTime )
{
	if( pAttacker == 0 )
		return false;

	/// 공격자 셋팅
	mAttacker.index = pAttacker->GetObjectID();
	mAttacker.type = eOBJECTTYPE_PLAYER;

	/// skill pc정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pSkillInfo == NULL )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkill - GetPlayerSkillInfo(%d)", skillIdx );
		return false; 
	}

	/// 스킬 스텝정보 읽어옴
	if( skillStep >= pSkillInfo->mStepCount )
	{
		assert(NULL); 
		NETWORK2->PostServerEvent("InitPlayerSkill - mSkillStep(%d)", skillStep );
		return false; 
	}

	/// 연출 스크립트 파일에서 시간 값을 읽어옴
	cDramaturgyInfo* pDramaInfo = DRAMATURGYMAN->GetPlayerDramaturgyInfo( skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
		pAttacker->GetRace(), pAttacker->GetGender(), &mDramaKey );
	if( pDramaInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent( "InitPlayerSkill - pDramaInfo error[%d,%d,%d,%d]", skillIdx, (unsigned short)pAttacker->GetWeaponState(), 
			pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	sDramaState* pDramaState = pDramaInfo->GetDramaState();
	if( pDramaState == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("InitPlayerSkill, GetDramaState error [%d,%d,%d,%d,%d]", 
			mAttacker.index, skillIdx, (unsigned short)pAttacker->GetWeaponState(), pAttacker->GetRace(), pAttacker->GetGender() );
		return false;
	}

	/// 고유 idx 셋팅
	mUniqueIdx =  uniqueIdx;
	/// 스킬idx의 값을 셋팅.
	mSkillIdx = skillIdx;
	mSkillStep = skillStep;

	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	/// 타겟 셋팅
	if( pTarget )
	{
		mCenterTarget.index = pTarget->GetObjectID();
		mCenterTarget.type = pTarget->GetObjectType();
	}
	else
	{
		mCenterTarget.index = 0;
		mCenterTarget.type = eOBJECTTYPE_NONE;
	}

	mMapNumber = pAttacker->GetMapNumber();

	/// 스킬 오브젝트 생성 시간
	mCreationTime = NETWORK2->GetAccumTime();

	mDetailCnt = pDramaState->GetDamageCount();
	mpDetailBegin = (sDramaAction*)pDramaState->GetDamageBegin();
	mpDetailEnd = (sDramaAction*)pDramaState->GetDamageEnd();

	/// 스크립트 검사
	if( mDetailCnt >= 10 )
		return false;

	if( mDetailCnt > 1 )
	{
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PUSH )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_AROUND_PULL )
			return false;
		if( pStepInfo->mApplyValueType2 == eSTATUSPLUS_TARGET_PUSH )
			return false;
	}

	/// 발사체 유무
	mIsProjectile = pDramaInfo->IsBullet();
	/// 발사체 속도
	mProjectileSpeed = pDramaInfo->GetBulletSpeed();
	/// 발동 종료시간
	mActivityTime = pDramaInfo->GetActivityEndTime();
	/// 공격 시작위치 = 발사물 시작위치
	mProjectilePos = pAttacker->GetPos();

	/// 발사물 타겟위치를 시전자 기준 범위로 발사물 처리할대 목적지점을 바꾼다.
	mCenterPos.x = pAttacker->GetXPos();
	mCenterPos.y = pAttacker->GetYPos();
	mCenterPos.z = pAttacker->Height();

	mDistressPoint = pStepInfo->mDistressValue;
	mAttributeType = pSkillInfo->mAttributeType;

	/// 타겟 해제 스킬
	if( pStepInfo->mApplyValueType1 == eSTATUSPLUS_DELETE_TARGETING || pStepInfo->mApplyValueType2 == eSTATUSPLUS_DELETE_TARGETING)
		mCreateType = eSKILLCREATETYPE_DELTARGET;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pMsgTarget = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pMsgTarget != NULL )
	{
		mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
		pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
	}

	if( pTarget )
	{
		/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
		pMsgTarget = GRIDMANAGER->FindFirstPlayer( pTarget, true );
		while( pMsgTarget != NULL )
		{
			mSendAroundSet.Insert( pMsgTarget->GetObjectID() );
			pMsgTarget = GRIDMANAGER->FindNextPlayer( true );
		}
	}

	mCastingTime = castingTime;
	if( mCastingTime != 0 )
	{
		mSkillState = eSKILLPROCESS_CASTING;
	}
	else
	{
		mSkillState = eSKILLPROCESS_ACTIVITY;
	}

	return true;
}

bool cPlayerSkillObject::CastingObjectCheck( cBaseObject* pAttacker )
{
	unsigned char errorCode = FAIL_SKILL_CASTING_DONT_TARGET;

	/// 생존 여부 확인
	if( pAttacker->GetStateDie() == false )
	{
		sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
		if( pSkillInfo != NULL )
		{
			if( ((cPlayer*)pAttacker)->IsCantSkill( pSkillInfo->mAttributeType ) == false )
			{
				/// 범위 방식에 따른 타겟 객체 확인 여부
				switch( pSkillInfo->mBoundType )
				{
				case eBOUNDTYPE_SELF_NOTARGET:
				case eBOUNDTYPE_FIELD:
				case eBOUNDTYPE_AUTOFIELD:
					return true;
				case eBOUNDTYPE_NONE:			/// 범위 없음
				case eBOUNDTYPE_TARGET:			/// 타겟 기준범위
				case eBOUNDTYPE_SELF_TARGET:	/// 시전자(자신) 기준범위 - 발동시 타겟 존재
					{
						/// 자신이 적용 대상이면 리턴
						if( pSkillInfo->mApplyType == eAPPLYTYPE_SELF	   ||
							pSkillInfo->mApplyType == eAPPLYTYPE_BUFF_SELF ||
							pSkillInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF ) 
							return true;
					}
				}

				/// 타겟 객체 체크
				cBaseObject* pTarget = OBJECTMANAGER->GetObject( mCenterTarget.type, mCenterTarget.index );
				if( pTarget != NULL )	
				{ 
					/// 부활 스킬인지 체크
					bool isResurrection = false;
					if( pSkillInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
						isResurrection = true;

					/// 생존 유무 체크
					if( pTarget->GetStateDie() == false && isResurrection == false )
						return true;

					/// 부활 스킬 조건 만족
					if( pTarget->GetStateDie() == true && isResurrection == true )
						return true;
				}
			}
			else
			{
				errorCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::CastingObjectCheck pSkillInfo[%d] == NULL", pAttacker->GetObjectID(), mSkillIdx );
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
bool cPlayerSkillObject::RequireValue( cBaseObject* pAttacker )
{	
	/// skill pc정보 스텝정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::RequireValue pSkillInfo[%d] == NULL", 
			pAttacker->GetObjectID(), mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return false;
	}

	if( mSkillStep > pSkillInfo->mStepCount )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::RequireValue mSkillStep[%d] > pSkillInfo->mStepCount", 
			pAttacker->GetObjectID(), mSkillStep );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return false;
	}
	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	/// 소모 체크
	switch( mCreateType )
	{
	case eSKILLCREATETYPE_NORMAL:
		{
			/// 공격자가 접속 종료중 이면 사용불가를 알림
			if( ((cPlayer*)pAttacker)->GetGameIn() == false )
			{
				/// 캐스팅 발동 실패 메세지 전송
				MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
				failMsg.Category = NM_SKILL;
				failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
				failMsg.mAttacker = mAttacker;
				failMsg.mSkillIndex = mUniqueIdx;
				failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_OBSTACLE;

				NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

				return false; 
			}

			unsigned long useMP = pStepInfo->mUseMP;
			if( pStepInfo->mUseMPPer != 0 )
				useMP += ((cPlayer*)pAttacker)->GetOriMPMax() * pStepInfo->mUseMPPer / 100;

			/// 소모 체크
			if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				useMP = ((cPlayer*)pAttacker)->CalcUseMPExt( useMP );
			}

			if( pAttacker->GetHP() <= pStepInfo->mUseHP || 
				pAttacker->GetMP() < useMP ||
				( pSkillInfo->mUseItem != 0 && ((cPlayer*)pAttacker)->IsItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount ) == false	) )
			{ 
				/// 캐스팅 발동 실패 메세지 전송
				MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
				failMsg.Category = NM_SKILL;
				failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
				failMsg.mAttacker = mAttacker;
				failMsg.mSkillIndex = mUniqueIdx;
				failMsg.mFailCode = FAIL_SKILL_CASTING_MP_EMPTY;

				NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

				return false; 
			}

			if( pStepInfo->mUseItemCount != 0 )
				((cPlayer*)pAttacker)->ItemUse( pSkillInfo->mUseItem, pStepInfo->mUseItemCount );


			/// MP를 소모 하는 스킬
			if( useMP != 0 )
				pAttacker->MPUse( useMP, false );

			/// HP를 소모 하는 스킬
			if( pStepInfo->mUseHP != 0 )
			{
				/// HP 소모 & 메세지 발송
				pAttacker->HPUse( pStepInfo->mUseHP, true );
			}

			/// 쿨타임 갱신
			SKILLMANAGER->UpdateSkillCoolTime( mAttacker, mSkillIdx, mSkillStep, NETWORK2->GetAccumTime(), mSpeedFactor );
		}
		break;
	case eSKILLCREATETYPE_MAPCHANGE:
		{
			/// item을 유무 확인
			if( ((cPlayer*)pAttacker)->IsItemIdx( mUseItemIdx ) == false )
			{
				/// 캐스팅 발동 실패 메세지 전송
				MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
				failMsg.Category = NM_SKILL;
				failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
				failMsg.mAttacker = mAttacker;
				failMsg.mSkillIndex = mUniqueIdx;
				failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_MAPCHANGE;

				NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

				return false; 
			}
		}
		break;
	case eSKILLCREATETYPE_VEHICLE:
		{
			/// item을 유무 확인
			if( ((cPlayer*)pAttacker)->IsItem( mUseItemIdx ) == false )
			{
				/// 캐스팅 발동 실패 메세지 전송
				MSG_SYN_SKILL_CAST_ACTIVITY_FAIL failMsg;
				failMsg.Category = NM_SKILL;
				failMsg.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
				failMsg.mAttacker = mAttacker;
				failMsg.mSkillIndex = mUniqueIdx;
				failMsg.mFailCode = FAIL_SKILL_CASTING_DONT_VEHICLE;

				NETWORK2->QuickSend( pAttacker, (char*)&failMsg, sizeof(failMsg) );

				return false; 
			}
		}
		break;
	}

	/// 캐스팅 스킬 발동시 mp소모와 발동을 알리는 싱크 전송
	MSG_SYN_SKILL_CAST_ACTIVE msgEtc;
	msgEtc.Category = NM_SKILL;
	if( pSkillInfo->mBoundType == eBOUNDTYPE_FIELD || pSkillInfo->mBoundType == eBOUNDTYPE_AUTOFIELD)
		msgEtc.Protocol = NM_SKILL_POS_CAST_ACTIVE_SYN;
	else
		msgEtc.Protocol = NM_SKILL_CAST_ACTIVE_SYN;
	msgEtc.mSkillIndex = mUniqueIdx;
	msgEtc.mAttacker = mAttacker;
	msgEtc.mAttackerPosX = pAttacker->GetXPos();
	msgEtc.mAttackerPosY = pAttacker->GetYPos();
	msgEtc.mObjectMP = pAttacker->GetMP();
	msgEtc.mObjectMaxMp = pAttacker->GetMaxMP();
	NETWORK2->QuickSend( pAttacker, (char*)&msgEtc, sizeof(msgEtc) );

	///발동 체크 
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( mCenterTarget.type, mCenterTarget.index );
	if( pTarget )
	{
		switch( pSkillInfo->mType )
		{
		case eSKILLTYPE_DAMAGE:
		case eSKILLTYPE_MPDAMAGE:
			SKILLMANAGER->CheckActInfluence( eACTSTATUS_ACTIVESKILL, pAttacker, pTarget );
			break;
		case eSKILLTYPE_HEAL:
			SKILLMANAGER->CheckActInfluence( eACTSTATUS_HEALSKILL, pAttacker, pTarget );
			break;
		}

		/// 스킬 발동 효과
		if( pStepInfo && pStepInfo->mActiveIdx != 0 )
			SKILLMANAGER->CheckSkillActInfluence( pStepInfo->mActiveIdx, pAttacker, pTarget );
	}

	return true;
}



bool cPlayerSkillObject::SetCenterPos()
{

	/// skill pc정보 스텝정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject::SetCenterPos pSkillInfo[%d] == NULL", mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return false;
	}

	/// 데미지 적용시 기준이 될 위치좌표 산출
	switch( pSkillInfo->mBoundType )
	{
	case eBOUNDTYPE_NONE:			/// 범위 공격이 아님
	case eBOUNDTYPE_TARGET:			/// 타겟 기준범위
		{
			/// 사거리 체크 대상이 시전자인경우 필요 없음
			if( pSkillInfo->mApplyType != eAPPLYTYPE_SELF	   &&
				pSkillInfo->mApplyType != eAPPLYTYPE_BUFF_SELF &&
				pSkillInfo->mApplyType != eAPPLYTYPE_DEBUFF_SELF )
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


void cPlayerSkillObject::TargetSelect( cBaseObject* pAttacker )
{

	/// skill pc정보 스텝정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::TargetSelect pSkillInfo[%d] == NULL", pAttacker->GetObjectID(), mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}

	if( mSkillStep > pSkillInfo->mStepCount )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::TargetSelect mSkillStep[%d] > pSkillInfo->mStepCount", pAttacker->GetObjectID(), mSkillStep );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	bool applyHP = pSkillInfo->mType == eSKILLTYPE_MPDAMAGE ? false : true;

	/// 발사체중 타겟을 향해 날아 가다 타겟이 사라진경우 데미지 미스처리 여부
	if( mNoTargetNoDamage == true )
		return;

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

	/// 타겟 선정
	if( pSkillInfo->mBoundType != eBOUNDTYPE_NONE )
	{
		switch( pSkillInfo->mApplyType )
		{
		case eAPPLYTYPE_SELF:
		case eAPPLYTYPE_BUFF_SELF:
		case eAPPLYTYPE_DEBUFF_SELF:
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
			
				SelectPlayerTarget( pAttacker, mCenterPos, pStepInfo->mBoundDist1, pStepInfo->mBoundDist2, &targetCnt, 
					targetMaxCnt, applyHP, false, pSkillInfo->mBoundType );
				//OBJECTMANAGER->PlayerInCenterPoint( pAttacker->GetMapNumber(), mCenterPos, tempRange, &mTargetAry, applyHP, targetMaxCnt );
			}
			break;
		case eAPPLYTYPE_PARTY:
		case eAPPLYTYPE_BUFF_PARTY:
		case eAPPLYTYPE_DEBUFF_PARTY:
			{
				//mTargetAry.Clear();

				/// 공격자 자신먼저 체크
				float tempRange = OBJECTMANAGER->ObjectSizeRange( NULL, pAttacker, pStepInfo->mBoundDist1 );
				mRangeCheck.SetRadius( tempRange );
				NiPoint2 centerPos( mCenterPos.x, mCenterPos.y );
				if( mRangeCheck.IsRange( centerPos, pAttacker->GetPos() ) == true )
				{
					sObject targetObject;
					targetObject.index = pAttacker->GetObjectID();
					targetObject.type = eOBJECTTYPE_PLAYER;
					SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, targetObject );
				}

				unsigned long* userArr = NULL;
				unsigned int partyCount = 0;

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
					unsigned char targetCnt = 0;
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
						if( mRangeCheck.IsRange( centerPos, partyUser->GetPos() ) == true )
						{
							/// 타겟에 추가
							sObject targetObject;
							targetObject.type = eOBJECTTYPE_PLAYER;
							targetObject.index = userArr[i];

							SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, targetObject );

							++targetCnt;

							if( targetCnt >= targetMaxCnt )
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
				//mTargetAry.Clear();
				unsigned char targetCnt = 0;

				if( mCreateType == eSKILLCREATETYPE_DELTARGET )
				{
					float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, NULL, pStepInfo->mBoundDist1 );

					mTempDelTargetAry.Clear();
					((cPlayer*)pAttacker)->MonsterInTargeting( &mTempDelTargetAry, tempRange );
					for( unsigned long jj = 0 ; jj < mTempDelTargetAry.GetSize() ; ++jj )
					{
						sObject pObject = mTempDelTargetAry[jj];
						SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, pObject );
					}
					mTempDelTargetAry.Clear();
				}
				else
				{
					SelectPlayerTarget( pAttacker, mCenterPos, pStepInfo->mBoundDist1, pStepInfo->mBoundDist2, &targetCnt,
						targetMaxCnt, applyHP, true, pSkillInfo->mBoundType );
					SelectMonsterTarget( pAttacker, mCenterPos, pStepInfo->mBoundDist1, pStepInfo->mBoundDist2, &targetCnt,
						targetMaxCnt, applyHP, pSkillInfo->mBoundType );
					//OBJECTMANAGER->MonsterInCenterPoint( pAttacker->GetMapNumber(), mCenterPos, tempRange, &mTargetAry, applyHP, targetMaxCnt );
				}

				//OBJECTMANAGER->PlayerInCenterPoint( pAttacker->GetMapNumber(), mCenterPos, tempRange, &mTargetAry, 
				//	pAttacker->GetObjectID(), applyHP, targetMaxCnt );

			}
			break;
		}
	}
	else
	{
		//sMultiTarget multiTarget;
		//multiTarget.mTarget = mCenterTarget;
		////multiTarget.mApplyHP = applyHP;
		//mTargetAry.PushBack( multiTarget );
		SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, mCenterTarget );

		// 오토더미 공격
		if( mCenterTarget.type == eOBJECTTYPE_MONSTER )
		{
			cMonster* mon = OBJECTMANAGER->GetMonster( mCenterTarget.index );
			if( mon != NULL )
			{
				mon->AttackDummy( pAttacker->GetObject() );
			}
			else
				NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::TargetSelect Monster null", pAttacker->GetObjectID(), mCenterTarget.index );
		}
	}
}

void cPlayerSkillObject::ApplyDamage( cBaseObject* pAttacker )
{
    /// 자동사냥 신고 관련
    if ( pAttacker != NULL && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
    {
        cPlayer* pHero = (cPlayer*)pAttacker;
        if ( pHero ) 
        {
            pHero->SetLastAttackTime( NETWORK2->GetAccumTime() );
        }
        else
        {
		    NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::ApplyDamage [LastAttackTime] pHero == NULL", pAttacker->GetObjectID() );
        }
    }

	/// skill pc정보 스텝정보 읽어옴
	sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
	if( pSkillInfo == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::ApplyDamage pSkillInfo[%d] == NULL", pAttacker->GetObjectID(), mSkillIdx );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}

	if( mSkillStep > pSkillInfo->mStepCount )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cPlayerSkillObject[%d]::ApplyDamage mSkillStep[%d] > pSkillInfo->mStepCount", pAttacker->GetObjectID(), mSkillStep );
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[mSkillStep];

	/// 타입별 데미지 처리
	switch( pSkillInfo->mType )
	{
	case eSKILLTYPE_DAMAGE:
		{
			SendDamage( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pSkillInfo->mRangeType,
				pStepInfo->mApplyValueType1, pStepInfo->mApplyValueType2, pStepInfo->mApplyValue1, pStepInfo->mApplyValue2,
				pStepInfo->mInfulenceIdx, pStepInfo->mAccuracyValue, pStepInfo->mCriticalValue, true );	
		}
		break;
	case eSKILLTYPE_HEAL:
		{
			SendHeal( pAttacker, pSkillInfo->mShotType, 
				pStepInfo->mApplyValueType1, pStepInfo->mApplyValueType2, pStepInfo->mApplyValue1, pStepInfo->mApplyValue2,
				pStepInfo->mInfulenceIdx );
		}
		break;
	case eSKILLTYPE_NONACTIVE:
		{
			if( eSKILLCREATETYPE_DELTARGET == mCreateType )
			{
				SendDelTarget( pAttacker, pStepInfo->mAccuracyValue );
			}
			else if( eSKILLCREATETYPE_BLINK == mCreateType )
			{
				ApplyBlink( pAttacker );
			}
			else if( eSKILLCREATETYPE_CLEAR_COOLTIME == mCreateType )
			{
				if( pAttacker && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					SKILLMANAGER->ClearPlayerHaveSkillCoolTime( pAttacker->GetObjectID(), mSkillIdx );

					PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
					MSG_SYN_SKILL_CLEAR_COOLTIME* synMsg = NULL;

					/// 적용 메세지
					synMsg = (MSG_SYN_SKILL_CLEAR_COOLTIME*)perIoContext->buffer;
					synMsg->Category     = NM_SKILL;
					synMsg->Protocol     = NM_SKILL_CLEAR_COOLTIME;
					synMsg->mExceptSKillIdx = mSkillIdx; 
					synMsg->mCharacterIdx = pAttacker->GetObjectID();

					NETWORK2->SendToUser( ((cPlayer*)pAttacker)->GetConnectionIdx(), (char*)synMsg, sizeof( MSG_SYN_SKILL_CLEAR_COOLTIME ) );
					perIoContext->offset = sizeof( MSG_SYN_SKILL_CLEAR_COOLTIME );
					NETWORK2->ReleaseIoContext( perIoContext );
				}
			}	
			else
			{
				SendApply( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pStepInfo->mAccuracyValue,
					pStepInfo->mApplyValueType1, pStepInfo->mApplyValueType2, 
					pStepInfo->mApplyValue1, pStepInfo->mApplyValue2, pStepInfo->mInfulenceIdx );
			}
		}
		break;
	case eSKILLTYPE_MPDAMAGE:
		{
			SendDamage( pAttacker, pSkillInfo->mShotType, pSkillInfo->mAttributeType, pSkillInfo->mRangeType,
				pStepInfo->mApplyValueType1, pStepInfo->mApplyValueType2, pStepInfo->mApplyValue1, pStepInfo->mApplyValue2,
				pStepInfo->mInfulenceIdx, pStepInfo->mAccuracyValue, pStepInfo->mCriticalValue, false );	
		}
		break;
	case eSKILLTYPE_TOTEM:
		{
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

			AddTotem( pAttacker, ((cPlayer*)pAttacker)->GetPartyIndex(), ((cPlayer*)pAttacker)->GetPartyUnionIndex(), pSkillInfo->mApplyType, pSkillInfo->mAttributeType, pStepInfo->mInfulenceIdx, targetMaxCnt );
		}
		break;
	case eSKILLTYPE_PORTAL:
		{
			AddPortal( pAttacker, pStepInfo->mInfulenceIdx );
		}
		break;
	}
}
