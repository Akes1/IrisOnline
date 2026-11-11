#include "stdafx.h"
#include "BaseSkillObject.h"
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
#include "DuelManager.h"
#include "GridManager.h"
#include "UserPortal.h"
#include "StageScript.h"
#include "Player.h"
#include "Monster.h"
#include "Party.h"
#include "PartyManager.h"
#include "MonsterScript.h"

cBaseSkillObject::cBaseSkillObject()
{
	mUniqueIdx = 0;

	mAttacker.index = 0;
	mAttacker.index = eOBJECTTYPE_NONE;

	mCenterTarget.index = 0;
	mCenterTarget.index = eOBJECTTYPE_NONE;

	mIsProjectile = false;
	mActivityTime = 0;
	mSpeedFactor = 1.0f;
	mNoTargetNoDamage = false;
	mMapNumber = 0;
	mDetailPos = 0;
	mDetailEndTime = 0;
	mIsChgMonsSkill = false;
	mAttackerDie = 0;
	mCreateType = eSKILLCREATETYPE_NORMAL;
	mUseItemIdx = 0;
	mDistressPoint = 0;
	mDelTarget = false;

	mAttributeType = eATTRIBUTETYPE_PHYSICAL;

	mSendAttackerSet.Clear();
	mSendTargetSet.Clear();
	mSendAroundSet.Clear();

	mPushPull = 0;
	mPushRange = 0;

	mCashMapNum = 0;
	mCashX = 0.0f;
	mCashY = 0.0f;

	mIsChanneling = false;
	mpChannelDramaState = 0;
	mChannelingTime = 0;
}


cBaseSkillObject::~cBaseSkillObject()
{
	SKILLMANAGER->DelSkillUniqueIdx( mUniqueIdx );

	PerNode* node = mTargetRoot.pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pInfo = (sSkillTargetInfo*)node;

		node = node->next;
		SKILLTARGETPOOL->ReleaseSkillTarget( &mTargetRoot, pInfo );
	}
}



void cBaseSkillObject::Process( unsigned long elapsedTime, unsigned long accumTime )
{

	/// 공격자가 없는 경우 this오브젝트를 삭제 리스트에 넣음
	cBaseObject* pAttacker = OBJECTMANAGER->GetObject( mAttacker.type, mAttacker.index );
	if( pAttacker == NULL || mMapNumber != pAttacker->GetMapNumber() )
	{
		AttackerNullDamage();

		/// 소멸 프로세스 수행
		mSkillState = eSKILLPROCESS_DESTRUCTION;
	}

	if( mIsChanneling )
	{
		/// 채널링 지속 시간 체크.
		if( mChannelingTime <= accumTime )
		{
			/// 스킬 삭제 메시지 보내기
			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_HERO )
			{
				cPlayer* pPlayer = (cPlayer*)pAttacker;
				if( pPlayer->ChangeState( eOBJECT_STATE_IDLE ) == false )
				{
					if( pPlayer->GetStateDie() == false )
					{
						NETWORK2->PostServerEvent("cBaseSkillObject::Process ChannelingSkillCancel pAttacker[%d,%d,%d,%d]->ChangeState( eOBJECT_STATE_IDLE ) == false",
							mAttacker.index, pPlayer->GetState(), pPlayer->GetStateStop(), pPlayer->GetRequestRejection() );
					}
				}
			}

			/// 캐스팅 발동 실패 메세지 전송
			MSG_RES_CHANNELING_SKILL_CANCEL synSkillEnd;
			synSkillEnd.Category = NM_SKILL;
			synSkillEnd.Protocol = NM_SKILL_CAST_ACTIVITY_FAIL_SYN;
			synSkillEnd.mUniqueSkillIndex = mUniqueIdx;

			NETWORK2->QuickSend( pAttacker, (char*)&synSkillEnd, sizeof(synSkillEnd) );

			mSkillState = eSKILLPROCESS_DESTRUCTION;
		}
	}

	/// 캐스팅
	if( mSkillState == eSKILLPROCESS_CASTING )
	{
		/// 캐스팅시 공격자 / 타겟 객체 체크
		if( CastingObjectCheck( pAttacker ) == false )
			mSkillState = eSKILLPROCESS_DESTRUCTION;

		/// 캐스팅 끝날때까지 루프
		/// ※ 위에서 죽었다라고 판정이나도 실제 발동 시간을 체크해서 발동 가능시간이면 발동
		if( mCastingTime <= accumTime - mCreationTime )	
		{
			/// 소모 항목 적용
			if( RequireValue( pAttacker ) == true )
				mSkillState = eSKILLPROCESS_ACTIVITY;		/// 발동
			else
				mSkillState = eSKILLPROCESS_DESTRUCTION;	/// 소멸
		}
	}


	/// 발동
	if( mSkillState == eSKILLPROCESS_ACTIVITY )
	{
		if( pAttacker->GetStateDie() == false )
		{
			/// 발동 끝날때까지 루프
			if( mActivityTime > ( (accumTime - ( mCreationTime + mCastingTime )) * mSpeedFactor  ) )
				return; 

			/// 발사체 유무 검사
			if( mIsProjectile == true )
				mSkillState = eSKILLPROCESS_PROJECTILE;		/// 발사체
			else
			{
				if( mCreateType == eSKILLCREATETYPE_NORMAL )
					SetCenterPos();

				mSkillState = eSKILLPROCESS_DAMAGE;			/// 데미지
			}
		}
		else
		{
			mSkillState = eSKILLPROCESS_DAMAGE;

			if( mActivityTime * 0.7 >
				( (accumTime - ( mCreationTime + mCastingTime )) * mSpeedFactor  ) )
				mAttackerDie = 1;
			else
				mAttackerDie = 2;
		}

		if( mIsChanneling )
		{
			/// 캐스팅시 공격자 / 타겟 객체 체크
			if( CastingObjectCheck( pAttacker ) == false )
				mSkillState = eSKILLPROCESS_DESTRUCTION;
		}
	}

	/// 발사체 이동
	if( mSkillState == eSKILLPROCESS_PROJECTILE )
	{
		/// 발사체 목적지 설정
		///※ 발사체가 도달하기까지 목적지 좌표가 틀려질수 있으므로 목적지는 계속 체크
		if(	SetCenterPos() == false )
		{
			/// 타겟이 없으면 0데미지 발송
			mNoTargetNoDamage = true;
			mSkillState = eSKILLPROCESS_DAMAGE;	
		}
		else
		{
			/// 발사체 이동
			if( IsProjectileEndProcess( elapsedTime ) == true )
				mSkillState = eSKILLPROCESS_DAMAGE;			/// 데미지
		}
	}


	/// 데미지
	if( mSkillState == eSKILLPROCESS_DAMAGE )
	{
		/// 타겟 선정
		if( mCreateType == eSKILLCREATETYPE_NORMAL )
			TargetSelect( pAttacker );

		/// 데미지 적용
		ApplyDamage( pAttacker );

		mSkillState = eSKILLPROCESS_HP;		/// 소멸
	}


	/// HP적용
	if( mSkillState == eSKILLPROCESS_HP )
	{
		/// 시간 체크
		if( mDetailEndTime > accumTime )
			return;

		/// 더 적용 할것이 없으면 소멸
		if( ApplyHP( pAttacker ) == false )
		{
			if( mIsChanneling == false )
				mSkillState = eSKILLPROCESS_DESTRUCTION;		/// 소멸
			else
			{
				mCreationTime = accumTime;
				
				if( mpChannelDramaState )
				{
					mpDetailBegin = (sDramaAction*)mpChannelDramaState->GetDamageBegin();
					mpDetailEnd = (sDramaAction*)mpChannelDramaState->GetDamageEnd();
				}
				else
					mDetailCnt = 0;

				mSendAttackerSet.Clear();
				mSendTargetSet.Clear();
				mSendAroundSet.Clear();

				PerNode* node = mTargetRoot.pool;
				while ( node != NULL )
				{
					sSkillTargetInfo* pInfo = (sSkillTargetInfo*)node;

					node = node->next;
					SKILLTARGETPOOL->ReleaseSkillTarget( &mTargetRoot, pInfo );
				}

				mDetailPos = 0;

				mSkillState = eSKILLPROCESS_ACTIVITY;
			}
		}
	}


	/// 소멸
	if( mSkillState == eSKILLPROCESS_DESTRUCTION )
	{
		SKILLMANAGER->InsertDeleteSkillObject( mAttacker, mUniqueIdx );
		return;
	}

}



bool cBaseSkillObject::IsProjectileEndProcess( unsigned long elapsedTime )
{
	float moveElapsedTime = (float)elapsedTime / SECOND_THOUSAND;

	/// 방향 설정
	NiPoint2 dir1;
	dir1.x = mCenterPos.x - mProjectilePos.x;
	dir1.y = mCenterPos.y - mProjectilePos.y;
	dir1.Unitize();

	/// 속도 * 경과시간 만큼의 스킬 효과위치 이동
	NiPoint2 estimatePos;
	estimatePos.x = mProjectilePos.x + (mProjectileSpeed*moveElapsedTime*dir1.x);
	estimatePos.y = mProjectilePos.y + (mProjectileSpeed*moveElapsedTime*dir1.y);

	/// 목적지 좌표와 예상 이동 위치를 비교
	NiPoint2 dir2;
	dir2.x = mCenterPos.x - estimatePos.x;
	dir2.y = mCenterPos.y - estimatePos.y;
	dir2.Unitize();

	/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달할때까지 이동
	if( dir1.Dot(dir2) > 0 )
	{
		mProjectilePos = estimatePos;

		/// 프로세스 계속 수행
		return false;
	}

	return true;
}


void cBaseSkillObject::AttackerNullDamage()
{

	cPlayer* pPlayer = NULL;

	PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_APPLY* applyMsg = NULL;

	/// 적용 메세지
	applyMsg = (MSG_SYN_SKILL_APPLY*)perIoContext->buffer;

	applyMsg->Category     = NM_SKILL;
	applyMsg->Protocol     = NM_SKILL_APPLY_SYN;
	applyMsg->mSkillIndex  = mUniqueIdx;
	applyMsg->mAttackerType = mAttacker.type;
	applyMsg->mSkillClassIndex = mSkillIdx;
	applyMsg->mDramaKey = mDramaKey;
	applyMsg->mIsChangeMonster = mIsChgMonsSkill;

	/// 타겟의 수만큼 데미지 계산 & 통합 메세지 발송
	cBaseObject* pTarget = NULL;
	sObject target;
	unsigned long targetCnt = 0;

	PerNode* node = mTargetRoot.pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
		if( pSkillTarget == NULL )
		{
			node = node->next;
			continue;
		}

		target = pSkillTarget->mTarget;

		switch( target.type )
		{
		case eOBJECTTYPE_MONSTER:
			pTarget = GRIDMANAGER->GetMonster( target.index );
			break;
		case eOBJECTTYPE_PLAYER:	
			pTarget = GRIDMANAGER->GetPlayer( target.index ); 
			break;
		default:
			pTarget = NULL;
		}

		if( pTarget == NULL ) 
		{
			node = node->next;
			continue;
		}

		applyMsg->mApplyObject[targetCnt].mTarget = target;
		++targetCnt;

		/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
		pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
		while( pPlayer != NULL )
		{
			mSendAroundSet.Insert( pPlayer->GetObjectID() );
			pPlayer = GRIDMANAGER->FindNextPlayer( true );
		}

		node = node->next;
	}

	applyMsg->mApplyCount = (unsigned char)targetCnt;

	cHashSet::cIterator begin = mSendAroundSet.Begin();
	cHashSet::cIterator end = mSendAroundSet.End();
	while( begin != end )
	{
		cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *begin++ );
		if( pSendPlayer != NULL )
			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
	}

	perIoContext->offset = applyMsg->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext );
}


void cBaseSkillObject::SelectPlayerTarget( cBaseObject* pAttacker, NiPoint3 centerPos, float rangeX, float rangeY, 
										 unsigned char* pTargetCnt, unsigned long maxCnt, bool /*applyHP*/, bool enemy, eBOUNDTYPE boundType )
{
	if( pAttacker == NULL )
		return;

	if( pTargetCnt == NULL )
		return;

	mObjectIdxAry.Clear();

	if( rangeY == 0 )
	{
		GRIDMANAGER->FindPlayers( centerPos.x, centerPos.y, pAttacker->GetMapNumber(), 1600, &mObjectIdxAry );
		for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
		{
			cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
			if( pTarget == NULL || pTarget->GetStateDie() == true )
				continue;

			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && enemy == true )
			{
				if( pTarget->IsAttack( pAttacker->GetObjectID() ) == false )
					continue;
			}

			float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, rangeX );
			mRangeCheck.SetRadius( tempRange );

			sObject target;

			NiPoint3 targetPoint;
			targetPoint.x = pTarget->GetXPos();
			targetPoint.y = pTarget->GetYPos();
			targetPoint.z = pTarget->Height();

			/// 플레이어가 범위 안에 들어왔는지 체크
			if ( mRangeCheck.IsRange( mCenterPos, targetPoint ) )
			{
				target.index = pTarget->GetObjectID();
				target.type = pTarget->GetObjectType();
				//sMultiTarget multiTarget;
				//multiTarget.mTarget = target;
				////multiTarget.mApplyHP = applyHP;
				//mTargetAry.PushBack( multiTarget );
				SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, target );
				++(*pTargetCnt);

				if( *pTargetCnt >= maxCnt )
					return;
			}
		}
	}
	else
	{
		NiPoint2 centerPos;
		centerPos.x = mCenterPos.x;
		centerPos.y = mCenterPos.y;

		float direction = pAttacker->GetDirection();

		float cos = cosf( direction );
		float sin = sinf( direction );

		/// 사각범위 위치 계산
		NiPoint2 rectPos[4];
		if( boundType == eBOUNDTYPE_SELF_TARGET || boundType == eBOUNDTYPE_SELF_NOTARGET )
		{
			rectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.0f ) * sin );
			rectPos[0].y = ( ( +rangeY * 0.0f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 1.0f ) * sin );
			rectPos[1].y = ( ( -rangeY * 1.0f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 1.0f ) * sin );
			rectPos[2].y = ( ( -rangeY * 1.0f ) * cos - ( +rangeX * 0.5f ) * sin );
			rectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.0f ) * sin );
			rectPos[3].y = ( ( +rangeY * 0.0f ) * cos - ( +rangeX * 0.5f ) * sin );
		}
		else
		{
			rectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
			rectPos[0].y = ( ( +rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
			rectPos[1].y = ( ( -rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
			rectPos[2].y = ( ( -rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
			rectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
			rectPos[3].y = ( ( +rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
		}

		rectPos[0] = rectPos[0] + centerPos;
		rectPos[1] = rectPos[1] + centerPos;
		rectPos[2] = rectPos[2] + centerPos;
		rectPos[3] = rectPos[3] + centerPos;
	
		float maxRange = rangeX > rangeY ? rangeX : rangeY;
		maxRange = maxRange * 1.5f;

		GRIDMANAGER->FindPlayers( centerPos.x, centerPos.y, pAttacker->GetMapNumber(), maxRange, &mObjectIdxAry );
		for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
		{
			cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
			if( pTarget == NULL || pTarget->GetStateDie() == true )
				continue;

			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( pTarget->IsAttack( pAttacker->GetObjectID() ) == false )
					continue;
			}

			NiPoint2 targetPoint;
			targetPoint.x = pTarget->GetXPos();
			targetPoint.y = pTarget->GetYPos();

			if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
				continue;

			sObject target;
			target.index = pTarget->GetObjectID( );
			target.type = pTarget->GetObjectType();
			//sMultiTarget multiTarget;
			//multiTarget.mTarget = target;
			////multiTarget.mApplyHP = applyHP;
			//mTargetAry.PushBack( multiTarget );
			SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, target );
			++(*pTargetCnt);

			if( *pTargetCnt >= maxCnt )
				return;
		}
	}

	mObjectIdxAry.Clear();
}


void cBaseSkillObject::SelectMonsterTarget( cBaseObject* pAttacker, NiPoint3 centerPos, float rangeX, float rangeY, 
										   unsigned char* pTargetCnt, unsigned long maxCnt, bool /*applyHP*/, eBOUNDTYPE boundType )
{
	if( pAttacker == NULL )
		return;

	if( pTargetCnt == NULL )
		return;

	mObjectIdxAry.Clear();

	if( rangeY == 0 )
	{
		GRIDMANAGER->FindMonsters( centerPos.x, centerPos.y, pAttacker->GetMapNumber(), 1600, &mObjectIdxAry );
		for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
		{
			cMonster* pTarget = OBJECTMANAGER->GetMonster( mObjectIdxAry[i] );
			if( pTarget == NULL || pTarget->GetStateDie() == true )
				continue;

			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pPlayerAttacker = (cPlayer*)pAttacker;
				unsigned long attackerTeamType = pPlayerAttacker->GetPvPDMTeam();
				if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
				{
					if( attackerTeamType == pTarget->GetDMTeamType() )
						continue;
				}
			}

			float tempRange = OBJECTMANAGER->ObjectSizeRange( pAttacker, pTarget, rangeX );
			mRangeCheck.SetRadius( tempRange );

			sObject target;

			NiPoint3 targetPoint;
			targetPoint.x = pTarget->GetXPos();
			targetPoint.y = pTarget->GetYPos();
			targetPoint.z = pTarget->Height();

			/// 플레이어가 범위 안에 들어왔는지 체크
			if ( mRangeCheck.IsRange( mCenterPos, targetPoint ) )
			{
				target.index = pTarget->GetObjectID();
				target.type = pTarget->GetObjectType();
				//sMultiTarget multiTarget;
				//multiTarget.mTarget = target;
				////multiTarget.mApplyHP = applyHP;
				//mTargetAry.PushBack( multiTarget );
				SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, target );
				++(*pTargetCnt);

				if( *pTargetCnt >= maxCnt )
					return;
			}
		}
	}
	else
	{
		NiPoint2 centerPos;
		centerPos.x = mCenterPos.x;
		centerPos.y = mCenterPos.y;

		float direction = pAttacker->GetDirection();

		float cos = cosf( direction );
		float sin = sinf( direction );

		/// 사각범위 위치 계산
		NiPoint2 rectPos[4];
		if( boundType == eBOUNDTYPE_SELF_TARGET || boundType == eBOUNDTYPE_SELF_NOTARGET )
		{
			rectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.0f ) * sin );
			rectPos[0].y = ( ( +rangeY * 0.0f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 1.0f ) * sin );
			rectPos[1].y = ( ( -rangeY * 1.0f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 1.0f ) * sin );
			rectPos[2].y = ( ( -rangeY * 1.0f ) * cos - ( +rangeX * 0.5f ) * sin );
			rectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.0f ) * sin );
			rectPos[3].y = ( ( +rangeY * 0.0f ) * cos - ( +rangeX * 0.5f ) * sin );
		}
		else
		{
			rectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
			rectPos[0].y = ( ( +rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
			rectPos[1].y = ( ( -rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
			rectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
			rectPos[2].y = ( ( -rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
			rectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
			rectPos[3].y = ( ( +rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
		}

		rectPos[0] = rectPos[0] + centerPos;
		rectPos[1] = rectPos[1] + centerPos;
		rectPos[2] = rectPos[2] + centerPos;
		rectPos[3] = rectPos[3] + centerPos;

		float maxRange = rangeX > rangeY ? rangeX : rangeY;
		maxRange = maxRange * 1.5f;

		GRIDMANAGER->FindMonsters( centerPos.x, centerPos.y, pAttacker->GetMapNumber(), maxRange, &mObjectIdxAry );
		for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
		{
			cMonster* pTarget = OBJECTMANAGER->GetMonster( mObjectIdxAry[i] );
			if( pTarget == NULL || pTarget->GetStateDie() == true )
				continue;

			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				cPlayer* pPlayerAttacker = (cPlayer*)pAttacker;
				unsigned long attackerTeamType = pPlayerAttacker->GetPvPDMTeam();
				if( ePVPDM_TEAMTYPE_MAX != attackerTeamType )
				{
					if( attackerTeamType == pTarget->GetDMTeamType() )
						continue;
				}
			}

			NiPoint2 targetPoint;
			targetPoint.x = pTarget->GetXPos();
			targetPoint.y = pTarget->GetYPos();

			if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 1 )
				continue;

			sObject target;
			target.index = pTarget->GetObjectID( );
			target.type = pTarget->GetObjectType();
			//sMultiTarget multiTarget;
			//multiTarget.mTarget = target;
			////multiTarget.mApplyHP = applyHP;
			//mTargetAry.PushBack( multiTarget );
			SKILLTARGETPOOL->AddSkillTarget( &mTargetRoot, target );
			++(*pTargetCnt);

			if( *pTargetCnt >= maxCnt )
				return;
		}
	}

	mObjectIdxAry.Clear();
}


void cBaseSkillObject::AddSendTarget( unsigned long playerIdx )
{
	/// 공격자이거나 공격자와 같은파티면 공격자메세지셋에 넣어준다.
	if( mAttacker.type == eOBJECTTYPE_PLAYER )
	{
		cPlayer* pAttacker = GRIDMANAGER->GetPlayer( mAttacker.index );
		if( pAttacker == NULL )
			return;

		if( mAttacker.index == playerIdx )
		{
			mSendAttackerSet.Insert( playerIdx );
			return;
		}

		cParty* pParty = NULL;
		if( pAttacker->GetPartyIndex() != 0 )
			pParty = PARTYMAN->GetParty( pAttacker->GetPartyIndex() );

		if( pParty != NULL )
		{
			unsigned long* pPartyIdx = pParty->GetUserArr();
			if( pPartyIdx == NULL )
				return;

			unsigned long partyCnt = pParty->GetCount();

			for( unsigned long i = 0 ; i < partyCnt ; ++i )
			{
				if( pPartyIdx[i] == playerIdx )
				{
					mSendAttackerSet.Insert( playerIdx );
					return;
				}
			}
		}
	}

	/// 타겟이거나 타겟의 파티이면 타겟메세지셋에 넣는다.
	PerNode* node = mTargetRoot.pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
		if( pSkillTarget == NULL )
		{
			node = node->next;
			continue;
		}

		sObject target = pSkillTarget->mTarget;

		if( target.type != eOBJECTTYPE_PLAYER )
		{
			node = node->next;
			continue;
		}

		cPlayer* pTarget = GRIDMANAGER->GetPlayer( target.index );
		if( pTarget == NULL )
			return;

		if( target.index == playerIdx )
		{
			mSendTargetSet.Insert( playerIdx );
			return;
		}

		cParty* pParty = NULL;
		if( pTarget->GetPartyIndex() != 0 )
			pParty = PARTYMAN->GetParty( pTarget->GetPartyIndex() );

		if( pParty != NULL )
		{
			unsigned long* pPartyIdx = pParty->GetUserArr();
			if( pPartyIdx == NULL )
				return;

			unsigned long partyCnt = pParty->GetCount();

			for( unsigned long i = 0 ; i < partyCnt ; ++i )
			{
				if( pPartyIdx[i] == playerIdx )
				{
					mSendAttackerSet.Insert( playerIdx );
					return;
				}
			}
		}

		node = node->next;
	}

	/// 어느곳에도 포함되지 않으면 주변메세지셋에 넣는다.
	mSendAroundSet.Insert( playerIdx );
}


void cBaseSkillObject::SendDamage( cBaseObject* pAttacker, 
								  unsigned long shotType, eATTRIBUTETYPE attribType, eRANGETYPE rangeType,
								  unsigned short applyType1, unsigned short applyType2, unsigned long applyValue1, unsigned long applyValue2,
								  unsigned long influenceClassIdx, short accuracyValue, unsigned short criticalValue, bool applyHP )
{
	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	cBaseObject* pTarget = NULL;

	bool allDamageKindMiss = false;

	/// 풀에서 메모리 할당
	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	if( perIoContext == NULL )
	{
		NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage perIoContext == NULL" );
		return;
	}

	MSG_SYN_SKILL_DAMAGE_ATTACKER* damageMsg = (MSG_SYN_SKILL_DAMAGE_ATTACKER*)perIoContext->buffer;

	damageMsg->Category = NM_SKILL;
	damageMsg->Protocol = NM_SKILL_DAMAGE_ATTACKER_SYN;
	damageMsg->mSkillIndex  = mUniqueIdx;
	damageMsg->mAttackerType = mAttacker.type;
	damageMsg->mSkillClassIndex = mSkillIdx;
	damageMsg->mDramaKey = mDramaKey;
	damageMsg->mIsChangeMonster = mIsChgMonsSkill;
	damageMsg->mTargetCount = 0;
	damageMsg->mAttackerDie = mAttackerDie == 1;
	damageMsg->SetMsgLength( sizeof(MSG_SYN_SKILL_DAMAGE_ATTACKER) - sizeof(sTargetDamage) );
	sTargetDamage* targetDamage = damageMsg->mTargetDamage;

	/// 풀에서 메모리 할당
	PerIoContext* perIoContext2 = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	if( perIoContext2 == NULL )
	{
		NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage perIoContext2 == NULL" );
		return;
	}

	MSG_SYN_INFLUENCEARY_CREATE* pInfMsg = (MSG_SYN_INFLUENCEARY_CREATE*)perIoContext2->buffer;
	pInfMsg->Category = NM_SKILL;
	pInfMsg->Protocol = NM_SKILL_INFLUENCE_CREATELIST_SYN;
	pInfMsg->mTargetCnt = 0;
	sInfluenceCreate* pInf = pInfMsg->mTable;

	/// 타겟의 수만큼 데미지 계산 & 통합 메세지 발송
	sObject target;
	unsigned long targetCnt = 0;
	unsigned long damage = 0;			/// 계산된 데미지
	long attkAbsorbHPPer = pAttacker->GetStateOddity( eODDITYTYPE_HP_ABSORB );	/// 공격자 HP흡수 비율
	long attkAbsorbMPPer = pAttacker->GetStateOddity( eODDITYTYPE_MP_ABSORB ); /// 공격자 MP흡수 비율
	long absorbPer = 0;		/// 타겟이 데미지 흡수 비율
	long reflectionPer = 0;	/// 타겟이 데미지 반사 비율
	long absorbPhyRangePer = 0;	/// 타겟이 물리 원거리 데미지 흡수 비율
	bool ignoreDefence = false; /// 방어력 무시.

	if( mAttackerDie != 1 )
	{
		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}
			
			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( target.type == eOBJECTTYPE_MONSTER )
				{
					((cPlayer*)pAttacker)->UpdateDutyActiveSkill( pTarget->GetRaceGender(), mSkillIdx );
				}
			}

			pTarget->AddTakeDamage( mAttacker, 0, 2, eTAKEDAMAGETYPE_ADDPLAYER );

			targetDamage->mTarget = target;

			unsigned long instantSkillSuccessPer = 0;
			unsigned long instantSkillSuccess = 0;
			unsigned long instantSkillCriPer = 0;
			unsigned long instantSkillCri = 0;
			unsigned long instantSkillDamagePer = 0;
			unsigned long instantSkillDamage = 0;
			/// 몬스터 속성/ 분류에 따른 추가 대미지 산출 [4/9/2010 Jo_Client]					
			float addTotalDamage = 0.0f; 
			float perTotalDamage = 1.0f;

			/// 스킬의 데미지 항목 읽어옴
			switch( applyType1 )
			{
			case 0:	break;
			case eSTATUSPLUS_INSTANTPERHIT:			instantSkillSuccessPer += applyValue1;	break;
			case eSTATUSPLUS_INSTANTPLUSHIT:		instantSkillSuccess += applyValue1;	break;
			case eSTATUSPLUS_INSTANTPERCRI:			instantSkillCriPer += applyValue1;	break;
			case eSTATUSPLUS_INSTANTPLUSCRI:		instantSkillCri += applyValue1;	break;
			case eSTATUSPLUS_AROUND_PUSH:			mPushPull = eSTATUSPLUS_AROUND_PUSH;	mPushRange = applyValue1;	break;
			case eSTATUSPLUS_AROUND_PULL:			mPushPull = eSTATUSPLUS_AROUND_PULL;	break;
			case eSTATUSPLUS_INSTANTPERATTACK:		instantSkillDamagePer += applyValue1;	break;
			case eSTATUSPLUS_INSTANTPLUSATTACK:		instantSkillDamage += applyValue1;	break;
			case eSTATUSPLUS_HPABSORB_PER:			attkAbsorbHPPer += applyValue1; break;
			case eSTATUSPLUS_MPABSORB_PER:			attkAbsorbMPPer += applyValue1; break;
			case eSTATUSPLUS_TARGET_PUSH:			mPushPull = eSTATUSPLUS_TARGET_PUSH;	mPushRange = applyValue1;	break;
			case eSTATUSPLUS_INSTANT_IGNORE_DEFENCE: ignoreDefence = true; break;
			case eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_PLANT_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_BUG_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_SOUL_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_GIANT_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_DRAGON_TYPE_PER_ATTACK:
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
								if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
							if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
								perTotalDamage += ((float)applyValue1 * 0.01f); 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						/// 유저가 몬스터로 변신했는지 체크하여 속성 데미지 반영
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
								if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_VETERAN_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_ELITE_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_BOSS_TYPE_PER_ATTACK:	
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의  분류(eMONSTER_KIND_TYPE)를 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
							if( pMonScriptInfo->mType == instantMonKindType )
								perTotalDamage += ((float)applyValue1 * 0.01f); 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PER_ATTACK:
				{
					if( pTarget && (pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER) )
					{
						if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							/// 변신 안한 경우에만 데미지 추가
							unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
							if( chgMonIndex == 0 )
							{
								perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_PLANT_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_BUG_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_SOUL_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_GIANT_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_DRAGON_TYPE_PLUS_ATTACK:
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
								if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
							if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
								addTotalDamage += (float)applyValue1; 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
								if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_VETERAN_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_ELITE_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_BOSS_TYPE_PLUS_ATTACK:	
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의  분류(eMONSTER_KIND_TYPE)를 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
							if( pMonScriptInfo->mType == instantMonKindType )
								addTotalDamage += (float)applyValue1; 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PLUS_ATTACK:
				{
					if( pTarget && (pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER) )
					{
						if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							/// 변신 안한 경우에만 데미지 추가
							unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
							if( chgMonIndex == 0 )
							{
								addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			default:
				NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage applyType1[%d,%d,%d,%d,%d]", mAttacker.type, mAttacker.index, mSkillIdx, applyType1, applyValue1 );
			}
			switch( applyType2 )
			{
			case 0:	break;
			case eSTATUSPLUS_INSTANTPERHIT:			instantSkillSuccessPer += applyValue2;	break;
			case eSTATUSPLUS_INSTANTPLUSHIT:		instantSkillSuccess += applyValue2;	break;
			case eSTATUSPLUS_INSTANTPERCRI:			instantSkillCriPer += applyValue2;	break;
			case eSTATUSPLUS_INSTANTPLUSCRI:		instantSkillCri += applyValue2;	break;
			case eSTATUSPLUS_AROUND_PUSH:			mPushPull = eSTATUSPLUS_AROUND_PUSH;	mPushRange = applyValue2;	break;
			case eSTATUSPLUS_AROUND_PULL:			mPushPull = eSTATUSPLUS_AROUND_PULL;	break;
			case eSTATUSPLUS_INSTANTPERATTACK:		instantSkillDamagePer += applyValue2;	break;
			case eSTATUSPLUS_INSTANTPLUSATTACK:		instantSkillDamage += applyValue2;	break;
			case eSTATUSPLUS_HPABSORB_PER:			attkAbsorbHPPer += applyValue2; break;
			case eSTATUSPLUS_MPABSORB_PER:			attkAbsorbMPPer += applyValue2; break;
			case eSTATUSPLUS_TARGET_PUSH:			mPushPull = eSTATUSPLUS_TARGET_PUSH;	mPushRange = applyValue2;	break;
			case eSTATUSPLUS_INSTANT_IGNORE_DEFENCE: ignoreDefence = true; break;
			case eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_PLANT_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_BUG_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_SOUL_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_GIANT_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_DRAGON_TYPE_PER_ATTACK:
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
								if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
							if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
								perTotalDamage += ((float)applyValue1 * 0.01f); 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK ) / 2; 
								if( pMonScriptInfo->mType == (instantMonType + 1) ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_VETERAN_TYPE_PER_ATTACK:
			case eSTATUSPLUS_INSTANT_ELITE_TYPE_PER_ATTACK:	
			case eSTATUSPLUS_INSTANT_BOSS_TYPE_PER_ATTACK:	
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의  분류(eMONSTER_KIND_TYPE)를 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
							if( pMonScriptInfo->mType == instantMonKindType )
								perTotalDamage += ((float)applyValue1 * 0.01f); 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_PLANT_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_BUG_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_SOUL_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_GIANT_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_DRAGON_TYPE_PLUS_ATTACK:
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
								if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
							if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
								addTotalDamage += (float)applyValue1; 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonType = (applyType1 - eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK ) / 2;
								if( pMonScriptInfo->mType == (instantMonType+ 1)  ) /// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PER_ATTACK:
				{
					if( pTarget && (pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER) )
					{
						if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							/// 변신 안한 경우에만 데미지 추가
							unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
							if( chgMonIndex == 0 )
							{
								perTotalDamage += ((float)applyValue1 * 0.01f); 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_VETERAN_TYPE_PLUS_ATTACK:
			case eSTATUSPLUS_INSTANT_ELITE_TYPE_PLUS_ATTACK:	
			case eSTATUSPLUS_INSTANT_BOSS_TYPE_PLUS_ATTACK:	
				if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						/// 몬스터의  분류(eMONSTER_KIND_TYPE)를 읽어온다.
						const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
						if( pMonScriptInfo )
						{
							int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
							if( pMonScriptInfo->mType == instantMonKindType )
								addTotalDamage += (float)applyValue1; 
						}
					}
				}
				else if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER && pTarget )
				{
					if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
						if( chgMonIndex != 0 )
						{
							/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
							const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
							if( pMonScriptInfo )
							{
								int instantMonKindType = (applyType1 - eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK) / 2;
								if( pMonScriptInfo->mType == instantMonKindType )
									addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			case eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PLUS_ATTACK:
				{
					if( pTarget && (pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER || pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER) )
					{
						if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
						{
							/// 변신 안한 경우에만 데미지 추가
							unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
							if( chgMonIndex == 0 )
							{
								addTotalDamage += (float)applyValue1; 
							}
						}
					}
				}
				break;
			default:
				NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage applyType2[%d,%d,%d,%d,%d]", mAttacker.type, mAttacker.index, mSkillIdx, applyType2, applyValue2 );
			}

			/// 데미지 무시 효과가 있는경우 또 체크할 필요가 없음
			if( ignoreDefence == false )
			{
				/// 물리 데미지 무시 효과를 가지고 있는지 체크
				if( attribType == eATTRIBUTETYPE_PHYSICAL )
					ignoreDefence = (pAttacker->GetStateOddity( eODDITYTYPE_IGNORE_DEFENCE_PHY ) != 0) ? true : false ;
				/// 마법 데미지 무시 효과를 가지고 있는지 체크
				else if( attribType == eATTRIBUTETYPE_MAGIC )
					ignoreDefence = (pAttacker->GetStateOddity( eODDITYTYPE_IGNORE_DEFENCE_MAG ) != 0) ? true : false ;
			}

			/// 몬스터 속성에 따른 추가 데미지 
			if( pTarget && pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				/// 몬스터의 속성(eMONSTER_TYPE)과 분류(eMONSTER_KIND_TYPE) 읽어온다.
				unsigned char monsterType     = (unsigned char)eMONSTERTYPE_MAX;
				unsigned char monsterKindType = (unsigned char)eMONSTER_KIND_MAX;

				if(  pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					unsigned long chgMonIndex = ((cPlayer*)pTarget)->GetChgMonsterIdx();
					if( chgMonIndex != 0 )
					{
						/// 몬스터의 속성(eMONSTER_TYPE)을 읽어온다.
						const sMonsterScript* pMonScriptInfo = MONSTERSCRIPT->GetMonsterListInfo( chgMonIndex );
						if( pMonScriptInfo )
						{
							monsterType = pMonScriptInfo->mType;
							monsterKindType = pMonScriptInfo->mKind;
						}
					}
					else
					{
						/// 변신 안한경우 캐릭터(유저)형으로 분류
						monsterType = eMONSTERTYPE_CHARACTER;
					}
				}
				else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
				{
					if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER )
						NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage Monster Attacked Monster[%d,%d,%d,%d,%d]", mAttacker.type, mAttacker.index, target.index, target.type, mSkillIdx );

					/// 몬스터의  분류(eMONSTER_KIND_TYPE)를 읽어온다.
					const sMonsterScript* pMonScriptInfo = ((cMonster*)pTarget)->GetMonsterInfo();
					if( pMonScriptInfo )
					{
						monsterType = pMonScriptInfo->mType;
						monsterKindType = pMonScriptInfo->mKind;
					}
				}
				
				if( monsterType != (unsigned char)eMONSTERTYPE_MAX )
				{
					ePLAYER_STATUS_EXT status2Index = ePLAYER_STATUS_EXT(ePLAYER_STATUS_EXT_HUMAN_ATK + monsterType -1);
					if( monsterType == eMONSTERTYPE_CHARACTER ) /// 캐릭터형이 추가되서 인덱스 계산 시 예외 처리. [5/10/2010 Jo_Client]
						status2Index = ePLAYER_STATUS_EXT_CHARACTER_ATK;
					
					float status2Add = ((cPlayer*)pAttacker)->GetStatus2Plus( status2Index );/// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
					addTotalDamage += status2Add; 
					float status2Per = ((cPlayer*)pAttacker)->GetStatus2Per( status2Index );/// eMONSTER_TYPE 가 1부터 시작해서 1을 보정해줬음
					perTotalDamage += (status2Per * 0.01f); 
				}

				if( monsterKindType != (unsigned char)eMONSTER_KIND_MAX )
				{
					ePLAYER_STATUS_EXT status2Index = ePLAYER_STATUS_EXT(ePLAYER_STATUS_EXT_NORMAL_ATK + monsterKindType);
					float status2Add = ((cPlayer*)pAttacker)->GetStatus2Plus( status2Index );
					addTotalDamage += status2Add;
					float status2Per = ((cPlayer*)pAttacker)->GetStatus2Per( status2Index );
					perTotalDamage += (status2Per * 0.01f );
				}
			}

			/// 데미지 산출 
			damage = pAttacker->DamageCalc( target, mSkillIdx, attribType, rangeType, instantSkillDamage, instantSkillDamagePer, ignoreDefence, addTotalDamage, perTotalDamage );

			if( attribType == eATTRIBUTETYPE_MAGIC )
			{
				/// 마법 데미지 흡수
				absorbPer = pTarget->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_MAG );
				/// 마법 데미지 반사
				reflectionPer = pTarget->GetStateOddity( eODDITYTYPE_DAMAGE_REFLECTION_MAG );
			}
			else
			{
				/// 물리 데미지 흡수
				absorbPer = pTarget->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_PHY );
				/// 물리 데미지 반사
				reflectionPer = pTarget->GetStateOddity( eODDITYTYPE_DAMAGE_REFLECTION_PHY );
				/// 물리 원거리 데미지 흡수
				if( rangeType == eRANGETYPE_LONG ) 
					absorbPhyRangePer = pTarget->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_PHY_RANGE );
			}

			// 오토 더미 세팅
			bool IsAutoDummy = false;
			if( pAttacker->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				// 공격자가 오토더미
				cMonster* mon = (cMonster*)pAttacker;
				if( mon && mon->IsAutoDummy() == true )
				{
					IsAutoDummy = true;
				}
			}
			else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				// 타겟이 오토더미
				cMonster* mon = (cMonster*)pTarget;
				if( mon && mon->IsAutoDummy() == true )
				{
					IsAutoDummy = true;
				}
			}

			sDramaAction* damageThis = mpDetailBegin;
			if( mDetailCnt != 0 )
				allDamageKindMiss = true;

			/// 대미지 분할 계산
			unsigned long damageI = 0;
			for( ; damageI < mDetailCnt ; ++damageI )
			{
				long divideDamage = 0;
				long divideReflection = 0;
				long divideAttkAbsorbHP = 0;
 				long divideAttkAbsorbMP = 0;
				eDAMAGE_KIND damageKind = eDAMAGE_MISS;
				bool applyMP = applyHP == false;
				long hideDamage = 0;

				if( pTarget->GetStateDie() == false )
				{
					/// 공격성공시 대미지 분할 계산
					bool success = pAttacker->AttackSuccess( target, attribType, accuracyValue, instantSkillSuccess, instantSkillSuccessPer );
					if( success == true && mAttackerDie != 2 )
					{
						/// 크리티컬 체크
						bool critical = pAttacker->CriticalSuccess( target, attribType, criticalValue, instantSkillCri, instantSkillCriPer );
						if( critical == false )
						{
							allDamageKindMiss = false;
							damageKind   = ( applyHP == true ) ? eDAMAGE_NORMAL : eDAMAGE_NORMAL_MP;
							divideDamage = FloatToInt( damage * damageThis->percent ); 
						
							bool damageZero = false;

							/// 무적 확인 (1)
							if( attribType == eATTRIBUTETYPE_PHYSICAL && pTarget->GetStateOddity( eODDITYTYPE_INVINCIBILITY_PHY ) != 0 )
							{
								divideDamage = 0;
								damageZero = true;
							}
							else if( attribType == eATTRIBUTETYPE_MAGIC && pTarget->GetStateOddity( eODDITYTYPE_INVINCIBILITY_MAG ) != 0 )
							{
								divideDamage = 0;
								damageZero = true;
							}

							/// 보호막 (2)
							if( damageZero == false && divideDamage != 0 )
							{
								divideDamage = pTarget->CalcGuard( (unsigned char)attribType, divideDamage );
								if( divideDamage == 0 )
									damageZero = true;
							}

							/// 데미지 흡수 계산 (3)
							if( damageZero == false && ( absorbPer != 0 || absorbPhyRangePer != 0) )
							{
								if( absorbPhyRangePer != 0 )
									absorbPer += absorbPhyRangePer;

								if( absorbPer < -100 )	absorbPer = -100;
								if( absorbPer > 100 )	absorbPer = 100;

								divideDamage = FloatToInt( divideDamage * ( 100 - absorbPer ) * 0.01f );

								if( divideDamage == 0 )
									damageZero = true;
							}
							
							/// 데미지 반사 (4)
							if( damageZero == false && reflectionPer != 0 )
							{
								/// 공격자가 받는 대미지 = 공격자의 대미지*(방어자의 반사량/100)
								divideReflection = (long)( divideDamage * ( reflectionPer * 0.01f ) );

								//	방어자가 받는 대미지 = IF((공격자의 대미지-ABS(공격자가 받은 반사량))<0,0,(공격자의 대미지-ABS(공격자가 받은 반사량)))
								if( divideDamage - abs( divideReflection ) < 0 )
									divideDamage = 0;
								else
									divideDamage =  divideDamage - abs( divideReflection );

								if( divideDamage == 0 )
									damageZero = true;
							}

							/// 공격자 데미지 hp흡수 (6)
							if( damageZero == false && attkAbsorbHPPer != 0 )
							{
								if( attkAbsorbHPPer < -100 ) attkAbsorbHPPer = -100;
								if( attkAbsorbHPPer > 100 )	attkAbsorbHPPer = 100;
								divideAttkAbsorbHP = FloatToInt( ( divideDamage * abs(attkAbsorbHPPer) ) * 0.01f );
								if( attkAbsorbHPPer < 0 )
									divideAttkAbsorbHP = divideAttkAbsorbHP * -1;
							}

							/// 공격자 데미지 mp흡수 (6)
							if( damageZero == false && attkAbsorbMPPer != 0 )
							{
								if( attkAbsorbMPPer < -100 ) attkAbsorbMPPer = -100;
								if( attkAbsorbMPPer > 100 )	attkAbsorbMPPer = 100;
								divideAttkAbsorbMP = FloatToInt( ( divideDamage * abs(attkAbsorbMPPer) ) * 0.01f );
								if( attkAbsorbMPPer < 0 )
									divideAttkAbsorbMP = divideAttkAbsorbMP * -1;
							}

							/// mp보호막 (6)
							if( damageZero == false && divideDamage != 0 )
							{
								/// 적용됐지만 남은 hp데미지에가려 보이지 않는 mp데미지
								hideDamage = divideDamage;							

								long leftDamage = pTarget->CalcChangeMPDamage( (unsigned char)attribType, divideDamage, applyHP );
								/// 적용하고 남은값이 없으면 전부mp데미지로 흡수
								if( leftDamage == 0 )
								{
									/// mp플래그를켜준다.
									applyMP = true;
									/// 잔여 mp데미지 없음
									hideDamage = 0;
								}
								else
								{
									/// 적용은 됐지만 완전히 적용이 안되고 일부만 된경우
									if( leftDamage != divideDamage )
									{
										/// 적용된 mp데미지 값만 기록
										hideDamage = divideDamage - leftDamage;
										/// hp적용값 기록
										divideDamage = leftDamage;
									}
									else
										hideDamage = 0;
								}
							}

							if( applyHP == true )
							{
								if( applyMP == true )
									damageKind = eDAMAGE_NORMAL_MP;
							}

							if( damageZero == false && divideDamage < 1 )
								divideDamage = 1;

							if( IsAutoDummy == true )
								divideDamage = 1;
						}
						else
						{
							if( applyHP == true )
								damageKind = eDAMAGE_CRITICAL;
							else
								damageKind = eDAMAGE_CRITICAL_MP;

							divideDamage = FloatToInt( damage * damageThis->percent * CRITICAL_RATE );
							divideReflection = 0;
							divideAttkAbsorbHP = 0;
							divideAttkAbsorbMP = 0;
							allDamageKindMiss = false;

							bool damageZero = false;

							/// 무적 확인 (1)
							if( attribType == eATTRIBUTETYPE_PHYSICAL && pTarget->GetStateOddity( eODDITYTYPE_INVINCIBILITY_PHY ) != 0 )
							{
								divideDamage = 0;
								damageZero = true;
							}
							else if( attribType == eATTRIBUTETYPE_MAGIC && pTarget->GetStateOddity( eODDITYTYPE_INVINCIBILITY_MAG ) != 0 )
							{
								divideDamage = 0;
								damageZero = true;
							}

							/// 보호막 (2)
							if( damageZero == false && divideDamage != 0 )
							{
								divideDamage = pTarget->CalcGuard( (unsigned char)attribType, divideDamage );
								if( divideDamage == 0 )
									damageZero = true;
							}

							/// 데미지 흡수 계산 (3)
							if( damageZero == false && ( absorbPer != 0 || absorbPhyRangePer != 0) )
							{
								if( absorbPhyRangePer != 0 )
									absorbPer += absorbPhyRangePer;

								if( absorbPer < -100 )	absorbPer = -100;
								if( absorbPer > 100 )	absorbPer = 100;
								divideDamage = FloatToInt( divideDamage * ( 100 - absorbPer ) * 0.01f );

								if( divideDamage == 0 )
									damageZero = true;
							}

							/// 데미지 반사 (4)
							if( damageZero == false && reflectionPer != 0 )
							{
								/// 공격자가 받는 대미지 = 공격자의 대미지*(방어자의 반사량/100)
								divideReflection = (long)( divideDamage * ( reflectionPer * 0.01f ) );

								//	방어자가 받는 대미지 = IF((공격자의 대미지-ABS(공격자가 받은 반사량))<0,0,(공격자의 대미지-ABS(공격자가 받은 반사량)))
								if( divideDamage - abs( divideReflection ) < 0 )
									divideDamage = 0;
								else
									divideDamage =  divideDamage - abs( divideReflection );

								if( divideDamage == 0 )
									damageZero = true;
							}

							/// 공격자 데미지 hp흡수 (6)
							if( damageZero == false && attkAbsorbHPPer != 0 )
							{
								if( attkAbsorbHPPer < -100 ) attkAbsorbHPPer = -100;
								if( attkAbsorbHPPer > 100 )	attkAbsorbHPPer = 100;
								divideAttkAbsorbHP = FloatToInt( ( divideDamage * abs(attkAbsorbHPPer) ) * 0.01f );
								if( attkAbsorbHPPer < 0 )
									divideAttkAbsorbHP = divideAttkAbsorbHP * -1;
							}

							/// 공격자 데미지 mp흡수 (6)
							if( damageZero == false && attkAbsorbMPPer != 0 )
							{
								if( attkAbsorbMPPer < -100 ) attkAbsorbMPPer = -100;
								if( attkAbsorbMPPer > 100 )	attkAbsorbMPPer = 100;
								divideAttkAbsorbMP = FloatToInt( ( divideDamage * abs(attkAbsorbMPPer) ) * 0.01f );
								if( attkAbsorbMPPer < 0 )
									divideAttkAbsorbMP = divideAttkAbsorbMP * -1;
							}

							/// mp보호막 (6)
							if( damageZero == false && divideDamage != 0 )
							{
								/// 적용됐지만 남은 hp데미지에가려 보이지 않는 mp데미지
								hideDamage = divideDamage;							

								long leftDamage = pTarget->CalcChangeMPDamage( (unsigned char)attribType, divideDamage, applyHP );
								/// 적용하고 남은값이 없으면 전부mp데미지로 흡수
								if( leftDamage == 0 )
								{
									/// mp플래그를켜준다.
									applyMP = true;
									/// 잔여 mp데미지 없음
									hideDamage = 0;
								}
								else
								{
									/// 적용은 됐지만 완전히 적용이 안되고 일부만 된경우
									if( leftDamage != divideDamage )
									{
										/// 적용된 mp데미지 값만 기록
										hideDamage = hideDamage - divideDamage;
										/// hp적용값 기록
										divideDamage = leftDamage;
									}
									else
										hideDamage = 0;
								}
							}

							if( applyHP == true )
							{
								if( applyMP == true )
									damageKind = eDAMAGE_CRITICAL_MP;
							}

							if( damageZero == false && divideDamage < 1 )
								divideDamage = 1;

							if( IsAutoDummy == true )
								divideDamage = 1;
						}
					}
				}

				damageThis = (sDramaAction*)damageThis->pNext;

				targetDamage->mDetail[damageI].mDamage = (short)divideDamage;
				targetDamage->mDetail[damageI].mDamageKind = (unsigned char)damageKind;
				targetDamage->mDetail[damageI].mDamageReflection = (short)divideReflection;
				targetDamage->mDetail[damageI].mDamageAbsorbHP = (short)divideAttkAbsorbHP;
				targetDamage->mDetail[damageI].mDamageAbsorbMP = (short)divideAttkAbsorbMP;

				//! 검사용 코드 테스트후 삭제.
				if ( !(damageI < 10) )
					NETWORK2->PostServerEvent( "cBaseSkillObject::SendDamage:mDetailCnt(=%d), damageI(=%d).", mDetailCnt, damageI );

				if( applyHP )
				{
					if( applyMP == true )
					{
						pSkillTarget->mDetail[damageI].mDetailDamageHP = 0;
						pSkillTarget->mDetail[damageI].mDetailDamageMP = divideDamage;
					}
					else
					{
						pSkillTarget->mDetail[damageI].mDetailDamageHP = divideDamage;
						if( hideDamage != 0 )
						{
							pSkillTarget->mDetail[damageI].mDetailDamageMP = hideDamage;
						}
					}
				}
				else
				{
					pSkillTarget->mDetail[damageI].mDetailDamageHP = 0;
					pSkillTarget->mDetail[damageI].mDetailDamageMP = divideDamage;
				}

				pSkillTarget->mDetail[damageI].mDetailDamageKind = (unsigned char)damageKind;
				pSkillTarget->mDetail[damageI].mDetailReflection = divideReflection;
				pSkillTarget->mDetail[damageI].mDetailAttkAbsorbHP = divideAttkAbsorbHP;
				pSkillTarget->mDetail[damageI].mDetailAttkAbsorbMP = divideAttkAbsorbMP;
			}

			pSkillTarget->mDetailCnt = damageI;
			targetDamage->mDetailCnt = (unsigned char)damageI;

			/// 스킬공격이 전체 실패가 아닐경우 부가 효과/밀고당기기 체크
			if(  allDamageKindMiss == false )	
			{ 
				/// 효과 인덱스와 공격타입 체크
				if( influenceClassIdx != 0 && shotType != eSHOTTYPE_AURA )
				{
					/// 효과 생성
					if( SKILLMANAGER->AddInfluence( mAttacker, target, influenceClassIdx, mSkillIdx, true, &pInf[pInfMsg->mTargetCnt] ) == false )
						targetDamage->mInfluenceMiss = true;
					else
						++pInfMsg->mTargetCnt;
				}

				/// 공격 성공시 밀기/당기기 설정이 있는지 체크
				if( mPushPull != 0 )
				{
					/// 밀기/당기기 효과생성 둘중에 하나라도 실패시 미스를 출력한다.
					if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
					{
						/// 면역 체크
						if( ((cPlayer*)pTarget)->IsImmuneApplyAll( mPushPull ) == true )
						{
							mPushPull = 0;
							targetDamage->mInfluenceMiss = true;
						}
					}
					else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
					{
						if( ((cMonster*)pTarget)->IsImmuneApplyAll( mPushPull ) == true )
						{
							mPushPull = 0;
							targetDamage->mInfluenceMiss = true;
						}
					}
				}

			}
			else
				mPushPull = 0;		/// 밀기/당기기 기능 사용 안함

			damageMsg->SetMsgLength( damageMsg->GetMsgLength() + (unsigned short)targetDamage->GetLength( ) );
			targetDamage = (sTargetDamage*)((char*)targetDamage + (unsigned short)targetDamage->GetLength( ));

			++targetCnt;

			node = node->next;
		}
	}

	damageMsg->mTargetCount = (unsigned char)targetCnt;

	/// 그리드에서 메세지 발송자 명단을 받아온다.
	GRIDMANAGER->FindSkillSendPlayer( this );


	//////////////////////////////////////////////////////////////////////////
	/// 공격자에게 발송하는 메세지
	{
		cHashSet::cIterator b = mSendAttackerSet.Begin();
		cHashSet::cIterator e = mSendAttackerSet.End();
		for( ; b != e ; ++b )
		{
			cPlayer* pPlayer = GRIDMANAGER->GetPlayer( *b );
			if( pPlayer == NULL )
				continue;
			
			mSendAroundSet.Erase( *b );

			/// 데미지 발송
			unsigned long connectionIdx = pPlayer->GetConnectionIdx();
			if( NETWORK2->SendToUser( connectionIdx, (char*)damageMsg, damageMsg->GetMsgLength() ) == false )
				NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] damageMsg attacker == false", damageMsg->GetMsgLength(), damageMsg->mTargetCount );

			/// 효과 발송
			if( pInfMsg->mTargetCnt > 0 )
			{
				if( NETWORK2->SendToUser( connectionIdx, (char*)pInfMsg, pInfMsg->GetMsgLength() ) == false )
					NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] pInfMsg attacker == false", pInfMsg->GetMsgLength(), pInfMsg->mTargetCnt );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 타겟들에게 발송하는 메세지
	{
		cHashSet::cIterator b = mSendTargetSet.Begin();
		cHashSet::cIterator e = mSendTargetSet.End();
		for( ; b != e ; ++b )
		{
			cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *b );
			if( pSendPlayer == NULL )
				continue;

			mSendAroundSet.Erase( *b );

			/// 풀에서 메모리 할당
			PerIoContext* pIoTarget = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
			if( pIoTarget == NULL )
			{
				NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage pIoTarget == NULL" );
				return;
			}

			MSG_SYN_SKILL_DAMAGE_ATTACKER* targetMsg = (MSG_SYN_SKILL_DAMAGE_ATTACKER*)pIoTarget->buffer;
			targetMsg->Category = NM_SKILL;
			targetMsg->Protocol = NM_SKILL_DAMAGE_TARGET_SYN;
			targetMsg->mSkillIndex  = mUniqueIdx;
			targetMsg->mAttackerType = mAttacker.type;
			targetMsg->mSkillClassIndex = mSkillIdx;
			targetMsg->mDramaKey = mDramaKey;
			targetMsg->mIsChangeMonster = mIsChgMonsSkill;
			targetMsg->mAttackerDie = mAttackerDie == 1;
			targetMsg->SetMsgLength( sizeof(MSG_SYN_SKILL_DAMAGE_ATTACKER) - sizeof(sTargetDamage) );

			unsigned short msgSize = sizeof(MSG_SYN_SKILL_DAMAGE_ATTACKER) - sizeof(sTargetDamage);

			sTargetDamage* pTargetDamage = targetMsg->mTargetDamage;
			sTargetDamage* pBaseDamage = damageMsg->mTargetDamage;

			for( int i = 0 ; i < damageMsg->mTargetCount ; ++i )
			{
				/// 타겟이거나 같은파티 소속인지 체크
				bool viewInfo = false;
				if( pBaseDamage->mTarget.type == eOBJECTTYPE_PLAYER )
				{
					if( pBaseDamage->mTarget.index == pSendPlayer->GetObjectID() )
						viewInfo = true;
					else
					{
						if( pSendPlayer->GetPartyIndex() != 0 )
						{
							cPlayer* pTargetPlayer = GRIDMANAGER->GetPlayer( pBaseDamage->mTarget.index );
							if( pTargetPlayer != NULL && pSendPlayer->GetPartyIndex() == pTargetPlayer->GetPartyIndex() )
								viewInfo = true;
						}
					}
				}

				memcpy( pTargetDamage, pBaseDamage, pBaseDamage->GetLength() );

				if( viewInfo == false )
					pTargetDamage->mDetailCnt = 0;

				msgSize = msgSize + (unsigned short)pTargetDamage->GetLength();

				pTargetDamage = (sTargetDamage*)(((char*)pTargetDamage) + (unsigned short)pTargetDamage->GetLength());
				pBaseDamage = (sTargetDamage*)(((char*)pBaseDamage) + (unsigned short)pBaseDamage->GetLength());
			}
			targetMsg->mTargetCount = damageMsg->mTargetCount;
			targetMsg->SetMsgLength( msgSize );

			if( NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)targetMsg, targetMsg->GetMsgLength() ) == false )
				NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] targetMsg target == false", targetMsg->GetMsgLength(), targetMsg->mTargetCount );

			/// 효과 발송
			if( pInfMsg->mTargetCnt > 0 )
			{
				if( NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)pInfMsg, pInfMsg->GetMsgLength() ) == false )
					NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] pInfMsg target == false", pInfMsg->GetMsgLength(), pInfMsg->mTargetCnt );
			}

			pIoTarget->offset = targetMsg->GetMsgLength();
			NETWORK2->ReleaseIoContext( pIoTarget );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 주변 유저에게발송하는 메세지
	{
		PerIoContext* pIoAround = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
		if( pIoAround == NULL )
		{
			NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage pIoAround == NULL" );
			return;
		}

		if( mIsChanneling )
		{
			MSG_SYN_SKILL_DAMAGE_AROUND_CHANNELING* aroundChannelingMsg = (MSG_SYN_SKILL_DAMAGE_AROUND_CHANNELING*)pIoAround->buffer;

			aroundChannelingMsg->Category = NM_SKILL;
			aroundChannelingMsg->Protocol = NM_SKILL_DAMAGE_AROUND_CHANNELING_SYN;
			aroundChannelingMsg->mSkillIndex  = mUniqueIdx;
			aroundChannelingMsg->mAttackerType = mAttacker.type;
			aroundChannelingMsg->mSkillClassIndex = mSkillIdx;
			aroundChannelingMsg->mDramaKey = mDramaKey;
			aroundChannelingMsg->mIsChangeMonster = mIsChgMonsSkill;
			aroundChannelingMsg->mAttackerDie = mAttackerDie == 1;
			aroundChannelingMsg->mTargetCount = damageMsg->mTargetCount;
			/// 채널링 추가 메시지
			aroundChannelingMsg->mTargetPosX = mCenterPos.x;
			aroundChannelingMsg->mTargetPosY = mCenterPos.y;
			aroundChannelingMsg->mPlayerIdx  = mAttacker.index;

			for( int i = 0 ; i < damageMsg->mTargetCount ; ++i )
			{
				aroundChannelingMsg->mTarget[i].type = damageMsg->mTargetDamage[i].mTarget.type;
				aroundChannelingMsg->mTarget[i].index = damageMsg->mTargetDamage[i].mTarget.index;
			}
			/// 발송
			cHashSet::cIterator b = mSendAroundSet.Begin();
			cHashSet::cIterator e = mSendAroundSet.End();
			for( ; b != e ; ++b )
			{
				cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *b );
				if( pSendPlayer == NULL )
					continue;

				/// 데미지 발송
				unsigned long connectionIdx = pSendPlayer->GetConnectionIdx();
				if( NETWORK2->SendToUser( connectionIdx, (char*)aroundChannelingMsg, aroundChannelingMsg->GetMsgLength() ) == false )
					NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] aroundChannelingMsg around == false", aroundChannelingMsg->GetMsgLength(), aroundChannelingMsg->mTargetCount );

				/// 효과 발송
				if( pInfMsg->mTargetCnt > 0 )
				{
					if( NETWORK2->SendToUser( connectionIdx, (char*)pInfMsg, pInfMsg->GetMsgLength() ) == false )
						NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] pInfMsg around == false", pInfMsg->GetMsgLength(), pInfMsg->mTargetCnt );
				}
			}

			pIoAround->offset = aroundChannelingMsg->GetMsgLength();
		}
		else
		{
			MSG_SYN_SKILL_DAMAGE_AROUND* aroundMsg = (MSG_SYN_SKILL_DAMAGE_AROUND*)pIoAround->buffer;

			aroundMsg->Category = NM_SKILL;
			aroundMsg->Protocol = NM_SKILL_DAMAGE_AROUND_SYN;
			aroundMsg->mSkillIndex  = mUniqueIdx;
			aroundMsg->mAttackerType = mAttacker.type;
			aroundMsg->mSkillClassIndex = mSkillIdx;
			aroundMsg->mDramaKey = mDramaKey;
			aroundMsg->mIsChangeMonster = mIsChgMonsSkill;
			aroundMsg->mAttackerDie = mAttackerDie == 1;
			aroundMsg->mTargetCount = damageMsg->mTargetCount;

			for( int i = 0 ; i < damageMsg->mTargetCount ; ++i )
			{
				aroundMsg->mTarget[i].type = damageMsg->mTargetDamage[i].mTarget.type;
				aroundMsg->mTarget[i].index = damageMsg->mTargetDamage[i].mTarget.index;
			}

			/// 발송
			cHashSet::cIterator b = mSendAroundSet.Begin();
			cHashSet::cIterator e = mSendAroundSet.End();
			for( ; b != e ; ++b )
			{
				cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *b );
				if( pSendPlayer == NULL )
					continue;

				/// 데미지 발송
				unsigned long connectionIdx = pSendPlayer->GetConnectionIdx();
				if( NETWORK2->SendToUser( connectionIdx, (char*)aroundMsg, aroundMsg->GetMsgLength() ) == false )
					NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] aroundMsg around == false", aroundMsg->GetMsgLength(), aroundMsg->mTargetCount );

				/// 효과 발송
				if( pInfMsg->mTargetCnt > 0 )
				{
					if( NETWORK2->SendToUser( connectionIdx, (char*)pInfMsg, pInfMsg->GetMsgLength() ) == false )
						NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage[%d,%d] pInfMsg around == false", pInfMsg->GetMsgLength(), pInfMsg->mTargetCnt );
				}
			}

			pIoAround->offset = aroundMsg->GetMsgLength();
		}	
		
		NETWORK2->ReleaseIoContext( pIoAround );
	}

	//////////////////////////////////////////////////////////////////////////
	/// 효과 메모리 제거
	perIoContext2->offset = pInfMsg->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext2 );

	//////////////////////////////////////////////////////////////////////////
	/// 데미지 메모리 제거
	perIoContext->offset = damageMsg->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext );

	/// 분할 대미지 적용 시작 시점 계산
	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;
}


void cBaseSkillObject::SendHeal( cBaseObject* pAttacker, unsigned long shotType,
								unsigned short applyType1, unsigned short applyType2, unsigned long applyValue1, unsigned long applyValue2,
								unsigned long influenceClassIdx )
{

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetConnectionIdx() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_HEAL_ATTACKER* healMsg = NULL;

	/// 힐 메세지
	healMsg = (MSG_SYN_SKILL_HEAL_ATTACKER*)perIoContext->buffer;

	healMsg->Category     = NM_SKILL;
	healMsg->Protocol     = NM_SKILL_HEAL_ATTACKER_SYN;
	healMsg->mSkillIndex  = mUniqueIdx;
	healMsg->mAttackerType = mAttacker.type;
	healMsg->mSkillClassIndex = mSkillIdx;
	healMsg->mDramaKey = mDramaKey;
	healMsg->mAttackerDie = mAttackerDie == 1;
	healMsg->mIsChangeMonster = mIsChgMonsSkill;
	healMsg->mTargetCount = 0;

	/// 자신도 타겟에 포함되었는지 체크
	bool attackerTarget = false;

	/// 타겟의 수만큼 데미지 계산 & 통합 메세지 발송
	cBaseObject* pTarget = NULL;
	sObject target;
	unsigned long heal = 0;
	unsigned long targetCnt = 0;
	if( mAttackerDie != 1 )
	{
		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			if( mAttacker.type == target.type && mAttacker.index == target.index )
				attackerTarget = true;

			/// 스킬 회복 기본량
			unsigned long skillHealPlus = 0;
			unsigned long skillHealPer = 0;
			if( applyType1 != 0 )
			{
				if( applyType1 == eSTATUSPLUS_INSTANTPLUSFINAL )
					skillHealPlus = skillHealPlus + applyValue1;
				else if( applyType1 == eSTATUSPLUS_INSTANTPERFINAL )
					skillHealPer = skillHealPer + pTarget->GetMaxHP() * applyValue1 / 100;
				else
				{
					assert(NULL);
					NETWORK2->PostServerEvent("eSKILLTYPE_HEAL ApplyValue1[%d,%d,%d,%d,%d] error", mAttacker.type, mAttacker.index, mSkillIdx, applyType1, applyValue1 );
				}
			}
			if( applyType2 != 0 )
			{
				if( applyType2 == eSTATUSPLUS_INSTANTPLUSFINAL )
					skillHealPlus = skillHealPlus + applyValue2;
				else if( applyType2 == eSTATUSPLUS_INSTANTPERFINAL )
					skillHealPer = skillHealPer + pTarget->GetMaxHP() * applyValue2 / 100;
				else
				{
					assert(NULL);
					NETWORK2->PostServerEvent("eSKILLTYPE_HEAL ApplyValue2[%d,%d,%d,%d,%d] error", mAttacker.type, mAttacker.index, mSkillIdx, applyType2, applyValue2 );
				}
			}

			if( mAttackerDie != 2 )
			{
				/// 효과 생성조건 만족 확인
				if( influenceClassIdx != 0 && shotType != eSHOTTYPE_AURA )	
				{ 
					NiPoint2 pos( mCenterPos.x, mCenterPos.y );
					/// 효과 생성
					if( SKILLMANAGER->AddInfluence( mAttacker, target, influenceClassIdx, mSkillIdx, true ) == false )
						healMsg->mTargetHeal[targetCnt].mInfluenceMiss = true;
				}

				/// 힐 계산
				heal = pAttacker->HealCalc( target, skillHealPlus, skillHealPer, true );
			}

			/// 밀기/당기기 효과생성 둘중에 하나라도 실패시 미스를 출력한다.
			if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			{
				if( mPushPull == 1 )
				{
					if( ((cPlayer*)pTarget)->IsImmuneApplyAll( eSTATUSPLUS_AROUND_PUSH ) == true )
					{
						mPushPull = 0;
						healMsg->mTargetHeal[targetCnt].mInfluenceMiss = true;
					}
				}
				else if( mPushPull == 2 )
				{
					if( ((cPlayer*)pTarget)->IsImmuneApplyAll( eSTATUSPLUS_AROUND_PULL ) == true )
					{
						mPushPull = 0;
						healMsg->mTargetHeal[targetCnt].mInfluenceMiss = true;
					}
				}
			}
			else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
			{
				if( mPushPull == 1 )
				{
					if( ((cMonster*)pTarget)->IsImmuneApplyAll( eSTATUSPLUS_AROUND_PUSH ) == true )
					{
						mPushPull = 0;
						healMsg->mTargetHeal[targetCnt].mInfluenceMiss = true;
					}
				}
				else if( mPushPull == 2 )
				{
					if( ((cMonster*)pTarget)->IsImmuneApplyAll( eSTATUSPLUS_AROUND_PULL ) == true )
					{
						mPushPull = 0;
						healMsg->mTargetHeal[targetCnt].mInfluenceMiss = true;
					}
				}
			}

			healMsg->mTargetHeal[targetCnt].mTarget = target;
			healMsg->mTargetHeal[targetCnt].mHeal = (short)heal;

			pSkillTarget->mDetailCnt = 1;
			pSkillTarget->mDetail[0].mDetailDamageHP = heal * -1;
			pSkillTarget->mDetail[0].mDetailDamageKind = eDAMAGE_NORMAL;
			pSkillTarget->mDetail[0].mDetailDamageMP = 0;
			pSkillTarget->mDetail[0].mDetailReflection = 0;
			pSkillTarget->mDetail[0].mDetailAttkAbsorbHP = 0;
			pSkillTarget->mDetail[0].mDetailAttkAbsorbMP = 0;

			++targetCnt;

			node = node->next;
		}
	}

	healMsg->mTargetCount = (unsigned char)targetCnt;
	perIoContext->offset = healMsg->GetMsgLength();

	/// 그리드에서 메세지 발송자 명단을 받아온다.
	GRIDMANAGER->FindSkillSendPlayer( this );

	//////////////////////////////////////////////////////////////////////////
	/// 공격자에게 발송하는 메세지
	{
		cHashSet::cIterator b = mSendAttackerSet.Begin();
		cHashSet::cIterator e = mSendAttackerSet.End();
		for( ; b != e ; ++b )
		{
			cPlayer* pPlayer = GRIDMANAGER->GetPlayer( *b );
			if( pPlayer == NULL )
				continue;

			mSendAroundSet.Erase( *b );

			unsigned long connectionIdx = pPlayer->GetConnectionIdx();
			NETWORK2->SendToUser( connectionIdx, (char*)healMsg, healMsg->GetMsgLength() );
		}
	}
 
	//////////////////////////////////////////////////////////////////////////
	/// 타겟들에게 발송하는 메세지
	{
		cHashSet::cIterator b = mSendTargetSet.Begin();
		cHashSet::cIterator e = mSendTargetSet.End();
		for( ; b != e ; ++b )
		{
			cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *b );
			if( pSendPlayer == NULL )
				continue;

			mSendAroundSet.Erase( *b );

			PerIoContext*         pIoTarget = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
			if( pIoTarget == NULL )
			{
				NETWORK2->PostServerEvent("cBaseSkillObject::SendHeal pIoTarget == NULL" );
				return;
			}

			MSG_SYN_SKILL_HEAL_ATTACKER* targetMsg = (MSG_SYN_SKILL_HEAL_ATTACKER*)pIoTarget->buffer;
			targetMsg->Category     = NM_SKILL;
			targetMsg->Protocol     = NM_SKILL_HEAL_TARGET_SYN;
			targetMsg->mSkillIndex  = mUniqueIdx;
			targetMsg->mAttackerType = mAttacker.type;
			targetMsg->mSkillClassIndex = mSkillIdx;
			targetMsg->mDramaKey = mDramaKey;
			targetMsg->mAttackerDie = mAttackerDie == 1;
			targetMsg->mIsChangeMonster = mIsChgMonsSkill;
			targetMsg->mTargetCount = healMsg->mTargetCount;

			sTargetHeal* pTargetDamage = targetMsg->mTargetHeal;
			sTargetHeal* pBaseDamage = healMsg->mTargetHeal;

			for( int i = 0 ; i < healMsg->mTargetCount ; ++i )
			{
				/// 타겟이거나 같은파티 소속인지 체크
				bool viewInfo = false;
				if( pBaseDamage->mTarget.type == eOBJECTTYPE_PLAYER )
				{
					if( pBaseDamage->mTarget.index == pSendPlayer->GetObjectID() )
						viewInfo = true;
					else
					{
						if( pSendPlayer->GetPartyIndex() != 0 )
						{
							cPlayer* pTargetPlayer = GRIDMANAGER->GetPlayer( pBaseDamage->mTarget.index );
							if( pTargetPlayer != NULL && pSendPlayer->GetPartyIndex() == pTargetPlayer->GetPartyIndex() )
								viewInfo = true;
						}
					}
				}

				memcpy( pTargetDamage, pBaseDamage, sizeof(sTargetHeal) );

				if( viewInfo == false )
					pTargetDamage->mHeal = 0;

				++pTargetDamage;
				++pBaseDamage;
			}

			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)targetMsg, targetMsg->GetMsgLength() );

			pIoTarget->offset = targetMsg->GetMsgLength();
			NETWORK2->ReleaseIoContext( pIoTarget );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// 주변 유저에게발송하는 메세지
	{
		PerIoContext*         pIoAround = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
		if( pIoAround == NULL )
		{
			NETWORK2->PostServerEvent("cBaseSkillObject::SendHeal pIoAround == NULL" );
			return;
		}

		MSG_SYN_SKILL_HEAL_AROUND* aroundMsg = (MSG_SYN_SKILL_HEAL_AROUND*)pIoAround->buffer;
		aroundMsg->Category     = NM_SKILL;
		aroundMsg->Protocol     = NM_SKILL_HEAL_AROUND_SYN;
		aroundMsg->mSkillIndex  = mUniqueIdx;
		aroundMsg->mAttackerType = mAttacker.type;
		aroundMsg->mSkillClassIndex = mSkillIdx;
		aroundMsg->mDramaKey = mDramaKey;
		aroundMsg->mAttackerDie = mAttackerDie == 1;
		aroundMsg->mIsChangeMonster = mIsChgMonsSkill;
		aroundMsg->mTargetCount = healMsg->mTargetCount;

		for( int i = 0 ; i < healMsg->mTargetCount ; ++i )
		{
			aroundMsg->mTarget[i].type = healMsg->mTargetHeal[i].mTarget.type;
			aroundMsg->mTarget[i].index = healMsg->mTargetHeal[i].mTarget.index;
		}

		/// 발송
		cHashSet::cIterator b = mSendAroundSet.Begin();
		cHashSet::cIterator e = mSendAroundSet.End();
		for( ; b != e ; ++b )
		{
			cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *b );
			if( pSendPlayer == NULL )
				continue;

			unsigned long connectionIdx = pSendPlayer->GetConnectionIdx();
			NETWORK2->SendToUser( connectionIdx, (char*)aroundMsg, aroundMsg->GetMsgLength() );
		}


		pIoAround->offset = aroundMsg->GetMsgLength();
		NETWORK2->ReleaseIoContext( pIoAround );
	}

	NETWORK2->ReleaseIoContext( perIoContext );

	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;

	/// 검증 코드
	if( mDetailCnt > 1 )
		mDetailCnt = 0;
}


void cBaseSkillObject::SendApply( cBaseObject* pAttacker, unsigned long shotType, eATTRIBUTETYPE attribType, short accuracyValue,
								 unsigned short applyType1, unsigned short applyType2, 
								 unsigned long applyValue1, unsigned long applyValue2, unsigned long influenceClassIdx )
{
	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_APPLY* applyMsg = NULL;

	/// 적용 메세지
	applyMsg = (MSG_SYN_SKILL_APPLY*)perIoContext->buffer;

	applyMsg->Category     = NM_SKILL;
	applyMsg->Protocol     = NM_SKILL_APPLY_SYN;
	applyMsg->mSkillIndex  = mUniqueIdx;
	applyMsg->mAttackerType = mAttacker.type;
	applyMsg->mSkillClassIndex = mSkillIdx;
	applyMsg->mDramaKey = mDramaKey;
	applyMsg->mAttackerDie = mAttackerDie == 1;
	applyMsg->mIsChangeMonster = mIsChgMonsSkill;

	/// 타겟의 수만큼 데미지 계산 & 통합 메세지 발송
	cBaseObject* pTarget = NULL;
	sObject target;
	unsigned long targetCnt = 0;
	if( mAttackerDie != 1 )
	{
		//for( unsigned long i = 0 ; i < mTargetAry.GetSize() ; ++i )
		//{
		//	/// 타겟 객체 확인
		//	target = mTargetAry[i].mTarget;

		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			pTarget->AddTakeDamage( mAttacker, 0, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );

			applyMsg->mApplyObject[targetCnt].mTarget = target;

			/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
			pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
			while( pPlayer != NULL )
			{
				mSendAroundSet.Insert( pPlayer->GetObjectID() );
				pPlayer = GRIDMANAGER->FindNextPlayer( true );
			}

			if( mAttackerDie != 2 )
			{
				switch( mCreateType )
				{
				case eSKILLCREATETYPE_NORMAL:
					{
						if( influenceClassIdx != 0 )
						{
							/// 오라 처리
							if( target.index == mAttacker.index && target.type == mAttacker.type && shotType == eSHOTTYPE_AURA )	
							{ 
								if( SKILLMANAGER->AddInfSwitch( mAttacker, target, influenceClassIdx, mSkillIdx, true ) == 0 )
									applyMsg->mApplyObject[targetCnt].mInfluenceMiss = true;
							}

							/// 오라외 효과 처리
							if( shotType != eSHOTTYPE_AURA )	
							{ 
								/// 효과 생성
								if( SKILLMANAGER->AddInfluence( mAttacker, target, influenceClassIdx, mSkillIdx, true ) == false )
									applyMsg->mApplyObject[targetCnt].mInfluenceMiss = true;
							}
						}

						unsigned long instantSkillSuccessPer = 0;
						unsigned long instantSkillSuccess = 0;

						/// 스킬의 데미지 항목 읽어옴
						switch( applyType1 )
						{
						case 0:	break;
						case eSTATUSPLUS_INSTANTPERHIT:			instantSkillSuccessPer += applyValue1;	break;
						case eSTATUSPLUS_INSTANTPLUSHIT:		instantSkillSuccess += applyValue1;	break;
						case eSTATUSPLUS_AROUND_PUSH:			mPushPull = eSTATUSPLUS_AROUND_PUSH;	mPushRange = applyValue1;	break;
						case eSTATUSPLUS_AROUND_PULL:			mPushPull = eSTATUSPLUS_AROUND_PULL;	break;
						case eSTATUSPLUS_TARGET_PUSH:			mPushPull = eSTATUSPLUS_TARGET_PUSH;	mPushRange = applyValue1;	break;
							break;
						default:
							NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage applyType1[%d,%d,%d,%d,%d]", mAttacker.type, mAttacker.index, mSkillIdx, applyType1, applyValue1 );
						}
						switch( applyType2 )
						{
						case 0:	break;
						case eSTATUSPLUS_INSTANTPERHIT:			instantSkillSuccessPer += applyValue2;	break;
						case eSTATUSPLUS_INSTANTPLUSHIT:		instantSkillSuccess += applyValue2;	break;
						case eSTATUSPLUS_AROUND_PUSH:			mPushPull = eSTATUSPLUS_AROUND_PUSH;	mPushRange = applyValue2;	break;
						case eSTATUSPLUS_AROUND_PULL:			mPushPull = eSTATUSPLUS_AROUND_PULL;	break;
						case eSTATUSPLUS_TARGET_PUSH:			mPushPull = eSTATUSPLUS_TARGET_PUSH;	mPushRange = applyValue2;	break;
						default:
							NETWORK2->PostServerEvent("cBaseSkillObject::SendDamage applyType2[%d,%d,%d,%d,%d]", mAttacker.type, mAttacker.index, mSkillIdx, applyType2, applyValue2 );
						}

						/// 공격성공시 대미지 분할 계산
						if( pAttacker->AttackSuccess( target, attribType, accuracyValue, instantSkillSuccess, instantSkillSuccessPer ) == true )
						{
							/// 밀기/당기기 효과생성 둘중에 하나라도 실패시 미스를 출력한다.
							if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
							{
								if( ((cPlayer*)pTarget)->IsImmuneApplyAll( mPushPull ) == true )
								{
									mPushPull = 0;
									applyMsg->mApplyObject[targetCnt].mInfluenceMiss = true;
								}
							}
							else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
							{
								if( ((cMonster*)pTarget)->IsImmuneApplyAll( mPushPull ) == true )
								{
									mPushPull = 0;
									applyMsg->mApplyObject[targetCnt].mInfluenceMiss = true;
								}
							}

							pTarget->SetSkillPushPull( mPushPull, pAttacker, mPushRange );
						}
					}
					break;
				case eSKILLCREATETYPE_MAPCHANGE:
					{
						if( applyType1 == eSTATUSPLUS_MAPCHANGE && target.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pTarget)->ItemMapChange( applyValue1, mUseItemIdx );
						}
						if( applyType2 == eSTATUSPLUS_MAPCHANGE && target.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pTarget)->ItemMapChange( applyValue2, mUseItemIdx );
						}
					}
					break;
				case eSKILLCREATETYPE_VEHICLE:
					{
						if( influenceClassIdx != 0 && target.index == mAttacker.index && target.type == mAttacker.type )
						{ 
							if( SKILLMANAGER->AddInfSwitch( mAttacker, target, influenceClassIdx, mSkillIdx, true ) == 0 )
								applyMsg->mApplyObject[targetCnt].mInfluenceMiss = true;
						}
					}
					break;
				case eSKILLCREATETYPE_CASHMAPCHANGE:
					{
						if( applyType1 == eSTATUSPLUS_CASHMAPCHANGE && target.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pTarget)->CashItemMapChange( mCashMapNum, mCashX, mCashY, mUseItemIdx );
						}
						if( applyType2 == eSTATUSPLUS_CASHMAPCHANGE && target.type == eOBJECTTYPE_PLAYER )
						{
							((cPlayer*)pTarget)->CashItemMapChange( mCashMapNum, mCashX, mCashY, mUseItemIdx );
						}
					}
					break;
				}

			}

			++targetCnt;

			node = node->next;
		}
	}

	applyMsg->mApplyCount = (unsigned char)targetCnt;

	cHashSet::cIterator begin = mSendAroundSet.Begin();
	cHashSet::cIterator end = mSendAroundSet.End();
	while( begin != end )
	{
		cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *begin++ );
		if( pSendPlayer != NULL )
			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
	}

	perIoContext->offset = applyMsg->GetMsgLength();

	char text[255];
	sprintf( text,
		"cBaseSkillObject::SendApply:MSG_SYN_SKILL_APPLY(%d %d %d %d %d %d %d %d %d)",
		mAttacker.type,
		mAttacker.index,
		applyMsg->GetMsgLength(),
		applyMsg->mSkillIndex,
		applyMsg->mAttackerType,
		applyMsg->mSkillClassIndex,
		applyMsg->mDramaKey,
		applyMsg->mIsChangeMonster,
		applyMsg->mApplyCount );

	NETWORK2->ReleaseIoContext( perIoContext, text );

	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;

	/// 검증 코드
	if( mDetailCnt > 1 )
		mDetailCnt = 0;
}

void cBaseSkillObject::SendDelTarget( cBaseObject* pAttacker, short accuracyValue )
{
	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_DELTARGET* applyMsg = NULL;

	/// 적용 메세지
	applyMsg = (MSG_SYN_SKILL_DELTARGET*)perIoContext->buffer;
	applyMsg->Category     = NM_SKILL;
	applyMsg->Protocol     = NM_SKILL_DELTARGET_SYN;
	applyMsg->mSkillIndex  = mUniqueIdx;
	applyMsg->mSkillClassIndex = mSkillIdx;
	applyMsg->mAttackerType = mAttacker.type;
	applyMsg->mAttackerDie = mAttackerDie == 1;
	applyMsg->mDramaKey = mDramaKey;

	applyMsg->mDelTargetType = pAttacker->GetObjectType();
	applyMsg->mDelTargetIndex = pAttacker->GetObjectID();

	/// 타겟의 수만큼 데미지 계산 & 통합 메세지 발송
	cBaseObject* pTarget = NULL;
	sObject target;
	unsigned long targetCnt = 0;
	if( mAttackerDie != 1 )
	{
		//for( unsigned long i = 0 ; i < mTargetAry.GetSize() ; ++i )
		//{
		//	/// 타겟 객체 확인
		//	target = mTargetAry[i].mTarget;

		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			pTarget->AddTakeDamage( mAttacker, 0, mDistressPoint, eTAKEDAMAGETYPE_NONETARGET );

			applyMsg->mApplyObject[targetCnt].mTarget = target;

			/// 확률 체크
			unsigned long activityRand = rand() % 100;
			applyMsg->mApplyObject[targetCnt].mInfluenceMiss = (accuracyValue <= (short)activityRand);

			/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
			pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
			while( pPlayer != NULL )
			{
				mSendAroundSet.Insert( pPlayer->GetObjectID() );
				pPlayer = GRIDMANAGER->FindNextPlayer( true );
			}
			++targetCnt;

			node = node->next;
		}
	}
	applyMsg->mApplyCount = (unsigned char)targetCnt;

	//for( unsigned long i = 0 ; i < mTargetAry.GetSize() ; ++i )
	//{
	//	/// apply target 
	//	target = mTargetAry[i].mTarget;

	unsigned long i = 0;
	PerNode* node = mTargetRoot.pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
		if( pSkillTarget == NULL )
		{
			node = node->next;
			continue;
		}

		target = pSkillTarget->mTarget;

		///
		if( target.type == eOBJECTTYPE_PLAYER )
		{
			/// msg send;
			cPlayer* player = GRIDMANAGER->GetPlayer( target.index );
			if( player && applyMsg->mApplyObject[i].mInfluenceMiss == false )
			{
				NETWORK2->SendToUser( player->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
				mSendAroundSet.Erase( target.index );
			}
		}
		else
		{
			cMonster* monster = GRIDMANAGER->GetMonster( target.index );
			if( monster )
			{
				if( applyMsg->mApplyObject[i].mInfluenceMiss == false )
					monster->ChangeTargetDeleteSkill();
			}
		}

		++i;
		node = node->next;
	}

	//applyMsg->mDelTargetType = 0;
	//applyMsg->mDelTargetIndex = 0;

	cHashSet::cIterator begin = mSendAroundSet.Begin();
	cHashSet::cIterator end = mSendAroundSet.End();
	while( begin != end )
	{
		cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *begin++ );
		if( pSendPlayer != NULL )
			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
	}

	perIoContext->offset = applyMsg->GetMsgLength();

	char text[255];
	sprintf( text,
		"cBaseSkillObject::SendDelTarget:MSG_SYN_SKILL_DELTARGET(%d %d %d %d %d %d %d %d)",
		mAttacker.type,
		mAttacker.index,
		applyMsg->GetMsgLength(),
		applyMsg->mSkillIndex,
		applyMsg->mAttackerType,
		applyMsg->mSkillClassIndex,
		applyMsg->mDramaKey,
		applyMsg->mApplyCount );

	NETWORK2->ReleaseIoContext( perIoContext, text );

	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;
}


void cBaseSkillObject::ApplyBlink( cBaseObject* pAttacker )
{
	/// 공격자가 죽은경우는 부활메세지쪽에서 자체적으로 위치를 보정해준다.
	if( pAttacker->GetStateDie() == false )
	{
		pAttacker->SetPos( mCenterPos.x, mCenterPos.y );
		if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = (cPlayer*)pAttacker;
			pPlayer->ClearMoveSyncCalc();
			pPlayer->SetMoveTargetPos( mCenterPos.x, mCenterPos.y );
		}
	}
}


bool cBaseSkillObject::ApplyHP( cBaseObject* pAttacker )
{
	mSendTargetSet.Clear();

	if( mpDetailBegin == NULL )
		return false;

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );

	while( pPlayer != NULL )
	{
		mSendTargetSet.Insert( pPlayer->GetConnectionIdx() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	bool die = false;
	unsigned char targetCnt = 0;
	unsigned long applyDamage = 0;

	PerNode* node = mTargetRoot.pool;
	while ( node != NULL )
	{
		sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
		if( pSkillTarget == NULL )
		{
			node = node->next;
			continue;
		}

		sObject target = pSkillTarget->mTarget;

		cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
		if( pTarget == NULL || pTarget->GetStateDie() == true )
		{
			node = node->next;
			continue;
		}

		/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
		pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
		while( pPlayer != NULL )
		{
			mSendTargetSet.Insert( pPlayer->GetConnectionIdx() );
			pPlayer = GRIDMANAGER->FindNextPlayer( true );
		}

		long			mDetailDamageHP = pSkillTarget->mDetail[mDetailPos].mDetailDamageHP;
		long			mDetailDamageMP = pSkillTarget->mDetail[mDetailPos].mDetailDamageMP;
		unsigned char	mDetailDamageKind = pSkillTarget->mDetail[mDetailPos].mDetailDamageKind;

		if( mDetailDamageHP > 0 )
		{
			/// 결투 데미지 체크시 결투 종료면 hpdamage 내부적으로 처리
			if( DUELMANAGER->DuelLastAttack( mAttacker, target, mDetailDamageHP, mDetailDamageKind == eDAMAGE_CRITICAL, &applyDamage ) == false )
				applyDamage = pTarget->HPDamage( mAttacker, mDetailDamageHP, &die, false, mDetailDamageKind == eDAMAGE_CRITICAL );
		}
		else if( mDetailDamageHP < 0 )
		{
			pTarget->HPHeal( -mDetailDamageHP, false, mAttacker, mDistressPoint, eTAKEDAMAGETYPE_HEAL );
		}

		if( mDetailDamageMP > 0 )
		{
			pTarget->MPDamage( mDetailDamageMP , false );
			applyDamage = 0;
		}

		if( mDetailDamageHP != 0 || mDetailDamageMP != 0 )
			SKILLMANAGER->InsertChangeSet( pTarget, mAttacker, mDetailDamageKind );

		++targetCnt;

		/// 받은 데미지 처리
		pTarget->AddTakeDamage( mAttacker, applyDamage, (mDetailDamageKind == eDAMAGE_MISS ? 0 : mDistressPoint), eTAKEDAMAGETYPE_DAMAGE );

		/// 데미지를 받고 죽지 않은 경우만 밀고당긴다.
		if( mDetailDamageKind != eDAMAGE_MISS && pTarget->GetStateDie() == false )
			pTarget->SetSkillPushPull( mPushPull, pAttacker, mPushRange );

		node = node->next;
	}

	long damage = 0;
	long totalDamage = 0;
	long mp = 0;
	long totalMp = 0;

	/// 데미지 반사이거나 공격자 데미지 흡수 인경우
	if( pAttacker != NULL && pAttacker->GetStateDie() != true )
	{
		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			sObject target = pSkillTarget->mTarget;

			cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
			if( pTarget == NULL )
			{
				node = node->next;
				continue;
			}

			damage = pSkillTarget->mDetail[mDetailPos].mDetailReflection - pSkillTarget->mDetail[mDetailPos].mDetailAttkAbsorbHP;
			totalDamage = totalDamage + damage;					 

			if( damage > 0 )
			{
				if( pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_NORMAL || pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_CRITICAL )
				{
					/// 결투 데미지 체크시 결투 종료면 hpdamage 내부적으로 처리
					if( DUELMANAGER->DuelLastAttack( target, mAttacker, damage, eDAMAGE_NORMAL, &applyDamage ) == false )
						applyDamage = pAttacker->HPDamage( target, damage, &die, false, eDAMAGE_NORMAL );
				}
				else if( pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_NORMAL_MP || pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_CRITICAL_MP )
				{
					pAttacker->MPDamage( damage, false );
					applyDamage = 0;
				}

				/// 받은 데미지 처리
				pAttacker->AddTakeDamage( target, applyDamage, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );
			}
			else if( damage < 0 )
			{
				if( pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_NORMAL || pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_CRITICAL )
					pAttacker->HPHeal( -damage, false, pAttacker->GetObject(), 0, eTAKEDAMAGETYPE_NONE );	/// 데미지 반사 인경우임
				else if( pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_NORMAL_MP || pSkillTarget->mDetail[mDetailPos].mDetailDamageKind == eDAMAGE_CRITICAL_MP )
					pAttacker->MPHeal( -damage, false );
			}

			mp      = pSkillTarget->mDetail[mDetailPos].mDetailAttkAbsorbMP;
			totalMp = totalMp + mp;

			if( mp > 0 )
				pAttacker->MPHeal( mp, false );
			else if( mp < 0 )
				pAttacker->MPDamage( abs(mp), false );

			node = node->next;
		}

		if( totalDamage != 0 || totalMp != 0 )
		{
			SKILLMANAGER->InsertChangeSet( pAttacker, mAttacker, eDAMAGE_NORMAL );

			++targetCnt;
		}
	}

	if( mpDetailBegin == 0 )
	{
		assert(0);
		return false;
	}

	mpDetailBegin = (sDramaAction*)mpDetailBegin->pNext;
	++mDetailPos;

	if( mDetailPos >= 10 )
	{
		NETWORK2->PostServerEvent("cBaseSkillObject::ApplyHP mDetailPos[%d] >= 10", mDetailPos );
		return false;
	}

	/// 적용항목 더 없음
	if( mpDetailBegin == NULL)
		return false;

	mDetailEndTime = mDetailStartTime + mpDetailBegin->time;
	return true;
}


void cBaseSkillObject::AddTotem( cBaseObject* pAttacker, unsigned long partyIdx, unsigned long partyUnionIdx, eAPPLYTYPE applyType, 
								eATTRIBUTETYPE attributeType, unsigned long influenceClassIdx, unsigned char applyCnt )
{

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_APPLY* applyMsg = NULL;

	/// 적용 메세지
	applyMsg = (MSG_SYN_SKILL_APPLY*)perIoContext->buffer;

	applyMsg->Category     = NM_SKILL;
	applyMsg->Protocol     = NM_SKILL_APPLY_SYN;
	applyMsg->mSkillIndex  = mUniqueIdx;
	applyMsg->mAttackerType = mAttacker.type;
	applyMsg->mSkillClassIndex = mSkillIdx;
	applyMsg->mDramaKey = mDramaKey;
	applyMsg->mAttackerDie = mAttackerDie == 1;
	applyMsg->mIsChangeMonster = mIsChgMonsSkill;
	applyMsg->mApplyCount = 0;

	/// 메세지 보내줄 타겟들 선정
	cBaseObject* pTarget = NULL;
	sObject target;
	if( mAttackerDie != 1 )
	{
		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
			pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
			while( pPlayer != NULL )
			{
				mSendAroundSet.Insert( pPlayer->GetObjectID() );
				pPlayer = GRIDMANAGER->FindNextPlayer( true );
			}

			node = node->next;
		}
	}

	cHashSet::cIterator begin = mSendAroundSet.Begin();
	cHashSet::cIterator end = mSendAroundSet.End();
	while( begin != end )
	{
		cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *begin++ );
		if( pSendPlayer != NULL )
			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
	}

	perIoContext->offset = applyMsg->GetMsgLength();

	char text[255];
	sprintf( text,
		"cBaseSkillObject::SendApply:MSG_SYN_SKILL_APPLY(%d %d %d %d %d %d %d %d %d)",
		mAttacker.type,
		mAttacker.index,
		applyMsg->GetMsgLength(),
		applyMsg->mSkillIndex,
		applyMsg->mAttackerType,
		applyMsg->mSkillClassIndex,
		applyMsg->mDramaKey,
		applyMsg->mIsChangeMonster,
		applyMsg->mApplyCount );

	NETWORK2->ReleaseIoContext( perIoContext, text );

	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;

	unsigned long skillIdx = 0;
	if( target.type == eOBJECTTYPE_PLAYER )
		skillIdx = mSkillIdx;

	NiPoint2 centerPos( mCenterPos.x, mCenterPos.y );
	cTotem* pTotem = OBJECTMANAGER->AddTotem( influenceClassIdx, partyIdx, partyUnionIdx, centerPos, pAttacker->GetObject(), applyType, attributeType, skillIdx, pAttacker->GetDirection(), applyCnt );
	if( pTotem != NULL )
		GRIDMANAGER->AddTotem( pTotem );
}


void cBaseSkillObject::AddPortal( cBaseObject* pAttacker, unsigned long mapChangePosIdx )
{

	/// 공격자 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pAttacker, true );
	while( pPlayer != NULL )
	{
		mSendAroundSet.Insert( pPlayer->GetObjectID() );
		pPlayer = GRIDMANAGER->FindNextPlayer( true );
	}

	PerIoContext*         perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_SKILL_APPLY* applyMsg = NULL;

	/// 적용 메세지
	applyMsg = (MSG_SYN_SKILL_APPLY*)perIoContext->buffer;

	applyMsg->Category     = NM_SKILL;
	applyMsg->Protocol     = NM_SKILL_APPLY_SYN;
	applyMsg->mSkillIndex  = mUniqueIdx;
	applyMsg->mAttackerType = mAttacker.type;
	applyMsg->mSkillClassIndex = mSkillIdx;
	applyMsg->mDramaKey = mDramaKey;
	applyMsg->mAttackerDie = mAttackerDie == 1;
	applyMsg->mIsChangeMonster = mIsChgMonsSkill;
	applyMsg->mApplyCount = 0;

	/// 메세지 보내줄 타겟들 선정
	cBaseObject* pTarget = NULL;
	sObject target;
	if( mAttackerDie != 1 )
	{
		PerNode* node = mTargetRoot.pool;
		while ( node != NULL )
		{
			sSkillTargetInfo* pSkillTarget = (sSkillTargetInfo*)node;
			if( pSkillTarget == NULL )
			{
				node = node->next;
				continue;
			}

			target = pSkillTarget->mTarget;

			switch( target.type )
			{
			case eOBJECTTYPE_MONSTER:
				pTarget = GRIDMANAGER->GetMonster( target.index );
				break;
			case eOBJECTTYPE_PLAYER:	
				pTarget = GRIDMANAGER->GetPlayer( target.index ); 
				break;
			default:
				pTarget = NULL;
			}

			if( pTarget == NULL ) 
			{
				node = node->next;
				continue;
			}

			/// 타겟 그리드에 해당하는 플레이어를 모두 읽어 데미지 발송자 명단에 넣는다.
			pPlayer = GRIDMANAGER->FindFirstPlayer( pTarget, true );
			while( pPlayer != NULL )
			{
				mSendAroundSet.Insert( pPlayer->GetObjectID() );
				pPlayer = GRIDMANAGER->FindNextPlayer( true );
			}

			node = node->next;
		}
	}

	cHashSet::cIterator begin = mSendAroundSet.Begin();
	cHashSet::cIterator end = mSendAroundSet.End();
	while( begin != end )
	{
		cPlayer* pSendPlayer = GRIDMANAGER->GetPlayer( *begin++ );
		if( pSendPlayer != NULL )
			NETWORK2->SendToUser( pSendPlayer->GetConnectionIdx(), (char*)applyMsg, applyMsg->GetMsgLength() );
	}

	perIoContext->offset = applyMsg->GetMsgLength();

	char text[255];
	sprintf( text,
		"cBaseSkillObject::SendApply:MSG_SYN_SKILL_APPLY(%d %d %d %d %d %d %d %d %d)",
		mAttacker.type,
		mAttacker.index,
		applyMsg->GetMsgLength(),
		applyMsg->mSkillIndex,
		applyMsg->mAttackerType,
		applyMsg->mSkillClassIndex,
		applyMsg->mDramaKey,
		applyMsg->mIsChangeMonster,
		applyMsg->mApplyCount );

	NETWORK2->ReleaseIoContext( perIoContext, text );

	mDetailStartTime = NETWORK2->GetAccumTime();
	if( mpDetailBegin != NULL )
		mDetailEndTime = mDetailStartTime + mpDetailBegin->time;

	if( pAttacker->GetObjectType() == eOBJECTTYPE_PLAYER )
	{
		NiPoint2 centerPos( mCenterPos.x, mCenterPos.y );
		cUserPortal* pUserPortal = OBJECTMANAGER->AddUserPortal( mapChangePosIdx, pAttacker->GetObjectID(), pAttacker->GetMapNumber(), 
			centerPos.x, centerPos.y );
		if( pUserPortal != NULL )
		{
			((cPlayer*)pAttacker)->SetUserPortal( pUserPortal->GetObjectID() );
			GRIDMANAGER->AddUserPortal( pUserPortal );
		}
	}
	else
		NETWORK2->PostServerEvent("cBaseSkillObject::AddPortal pAttacker->GetObjectType() != eOBJECTTYPE_PLAYER" );
}


unsigned long cBaseSkillObject::SkillEndTime()
{
	unsigned long detailEndTime = 0;
	if( mpDetailEnd != NULL )
		detailEndTime = mpDetailEnd->time;

	return (mCreationTime + mCastingTime + mActivityTime) - NETWORK2->GetAccumTime() + detailEndTime;
}
