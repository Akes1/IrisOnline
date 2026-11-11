#include "stdafx.h"
#include "actionmove.h"
#include "gamesrv.h"

#include "../Common/Protocol.h"
#include "Monster_Common.h"

#include "MonsterScript.h"

#include "Monster.h"
#include "Player.h"
#include "AIManager.h"
#include "GridManager.h"
#include "ObjectManager.h"


cActionMove::cActionMove(void): cAction( eACT_MOVE )
{
}



cActionMove::~cActionMove(void)
{
}



void cActionMove::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );

	if( pMonster->IsFirstMove() == true )
	{
		/// 리젠 후 일정 시간동안 선공 안하는 시간
		mRegenCantMoveEndTime = accumTime + FIRSTMOVE_WAIT_TIME;

		/// 이동 정지 & 리젠 정지 시간이 중복 되므로 큰값이 적용 되게 계산
		if( accumTime + MOVE_WAIT_TIME > accumTime + FIRSTMOVE_WAIT_TIME )
			mWaitEndTime = accumTime + MOVE_WAIT_TIME;
		else
			mWaitEndTime = accumTime + FIRSTMOVE_WAIT_TIME;

		/// 처음이동 플래그 사용 끝 - 이동,선공 일정 시간동안 안함
		pMonster->FirstMoveEnd();
	}
	else
		mWaitEndTime = accumTime + MOVE_WAIT_TIME;

	mDelayTime = accumTime;
	mDelayTime2 = accumTime;

	mSetPathErrCnd = 0;

	mIsIdle2Send = false;
	mIdle2SendTime = 0;

	mFirstMoveEndCheck = false;
	mFirstWaitEndCheck = false;
}



eACTION_CHANGE cActionMove::Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionRegen::Action pRegenGroupScript == NULL");
		return eACTCH_ERROR;
	}

	/// 스킬 사용이 끝날때 까지 통과
	if( pMonster->IsSkillWaitEnd() == false )
		return eACTCH_CONTINUE;

	pMonster->CheckComback();

	if( pMonster->IsAutoDummy() == false )
	{
		/// 몬스터가 선공몹인경우 타겟 검사
		if( pMonsterScript->mIsPrecedeAttack == true && pMonster->GetFollowEndMove() == false )
		{
			/// 0.5초후에 재검사 - 부하방지
			if( mDelayTime <= accumTime )
			{
				mDelayTime = accumTime + 500;

				/// 리젠 후 일정시간 동안 선공 안함
				if( mRegenCantMoveEndTime < accumTime )
				{
					/// 범위안에 들어온 플레이어 선택
					cPlayer* pPlayer = TargetSelect( pMonster, pMonsterScript->mSeekRange );
					if( pPlayer != NULL)
					{
						const sRegenScript* pRegen = pMonster->GetRegenMonsterInfo();
						if( pRegen != NULL && pRegen->mFamilyNum != 0 )
							AIMANAGER->FamilyAttack( pMonster->GetObjectID(), pRegen->mFamilyNum, pPlayer->GetObject() );

						pMonster->AddTakeDamage( pPlayer->GetObject(), 0, 0, eTAKEDAMAGETYPE_ADDPLAYER );
						return eACTCH_MOVE_END;
					}
				}
			}
		}
	}
	
	/// 몬스터 이동
	if( pMonster->MoveUpdate( deltaTime ) == true )
	{
		mFirstMoveEndCheck = false;

		/// 마지막 이동까지 대기시간 3초 설정
		mWaitEndTime = accumTime + MOVE_WAIT_TIME;
		return eACTCH_CONTINUE;
	}

	/// 이동 끝나자마자 한번 체크
	if( mFirstMoveEndCheck == false )	
	{
		mFirstMoveEndCheck = true;
		pMonster->SendSpeech( eMONSTERTALK_MAX );

		/// 이동 패턴이 있는경우 경유지에서만 대기함
		const sMonsterPattern* pPattern = pMonster->GetMovePattern();
		if( pPattern != NULL )
		{
			unsigned long waitTime = pMonster->GetPatternWait();
			mWaitEndTime = accumTime + waitTime;

			pMonster->SetGroupMoveReady( 1 );
		}
	}

	/// 이동이 끝나면 추적이동 회귀는 완료
	pMonster->SetFollowEndMove( false );


	/// 대기동작2 발송 체크
	if( mIsIdle2Send == true && accumTime < mIdle2SendTime )
	{
		/// 메세지 발송
		MSG_SYN_MONSTER_IDLE2ANI synMsg;
		synMsg.Category = NM_MONSTER;
		synMsg.Protocol = NM_MONSTER_IDLE2_ANI_SYN;
		synMsg.mMonsterIdx = pMonster->GetObjectID();
		NETWORK2->QuickSend( (cBaseObject*)pMonster, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_IDLE2ANI) );

		mIdle2SendTime = false;
		mIdle2SendTime = 0;
	}

	/// 대기시간 비교
	if( mWaitEndTime > accumTime )
		return eACTCH_CONTINUE;

	/// 다음 이동 경로 선정
	float targetPosX = pMonster->GetXPos();
	float targetPosY = pMonster->GetYPos();

	unsigned long familyNum = 0;
	if( pMonster->GetRegenMonsterInfo() != NULL )
		familyNum = pMonster->GetRegenMonsterInfo()->mFamilyNum;

	if( mFirstWaitEndCheck == true )
	{
		mFirstWaitEndCheck = false;

		const sMonsterPattern* pPattern = pMonster->GetMovePattern();
		if( pPattern != NULL && familyNum != 0 )
			pMonster->SetGroupMoveReady( 2 );
	}

	if( pMonster->GetGroupMoveReady() == 2 )
		return eACTCH_CONTINUE;

	pMonster->ClearPatternWait();

	/// 이동속도가 없으면 다음 경로 선정 안함
	if( pMonsterScript->mActionRunSpd == 0 && pMonsterScript->mActionWalkSpd == 0 )
		return eACTCH_CONTINUE;

	/// 다음 이동 경로 선정
	if( pMonster->SelectNextPath( &targetPosX, &targetPosY ) == false )
		return eACTCH_CONTINUE;

	if( familyNum != 0 )
	{
		if( AIMANAGER->FamilyFar( pMonster->GetMapNumber(), familyNum, pMonster->GetPos() ) == true )
			pMonster->SetMoveSpeed( pMonsterScript->mActionRunSpd, eMOVESPEED_RUN );
		else
			pMonster->SetMoveSpeed( pMonsterScript->mActionWalkSpd, eMOVESPEED_WALK );
	}

	/// 경로 설정시 실패 한경우면 다른 이동방식 선정 안하고 기존 이동방법으로 선정
	if( mSetPathErrCnd == 0 )
	{
		/// 이동이 끝난경우 자유이동 방식 선정
		unsigned long moveSpeed = 0;
		eMOVE_SPEED_TYPE moveSpeedType = RandMoveSpeed( pMonster, &moveSpeed );
		/// 행동 정지 선택
		if( moveSpeed == 0 )
		{
			/// 대기시간 추가
			mWaitEndTime = accumTime + pMonsterScript->mActionStopTime;

			/// 대기시간 중 일정시간에 대기동작2 메세지 발송 결정
			mIsIdle2Send = (rand() % 100) + 1 <= IDLE2_ANI_SEND_PER;
			if( mIsIdle2Send == true )
				mIdle2SendTime = accumTime + ( pMonsterScript->mActionStopTime / 3 );

			pMonster->SendSpeech( eMONSTERTALK_STAND );

			return eACTCH_CONTINUE;
		}

		/// 몬스터의 이동 속도 결정
		pMonster->SetMoveSpeed( moveSpeed, moveSpeedType );

		if( moveSpeedType == eMOVESPEED_WALK )
			pMonster->SendSpeech( eMONSTERTALK_WORK );
		else if( moveSpeedType == eMOVESPEED_RUN )
			pMonster->SendSpeech( eMONSTERTALK_RUN );
	}
	
	/// 몬스터가 이동할 경로 셋팅
	if( pMonster->SetPath( targetPosX, targetPosY ) == 0 )
		++mSetPathErrCnd;
	else
		mSetPathErrCnd = 0;

	/// 갈수없는 경로 연속실패 횟수가 너무 많은 경우 몬스터가 길찾기 허용 범위까지 나갔을 가능성이 크다.
	if( mSetPathErrCnd > 100 )
	{
		/// 강제 이동
		pMonster->SetPos( targetPosX, targetPosY );
		mSetPathErrCnd = 0;
	}

	/// 특별한 이벤트에 걸리지 않는 한 계속 이동
	return eACTCH_CONTINUE;
}



eMOVE_SPEED_TYPE cActionMove::RandMoveSpeed( cMonster* pMonster, unsigned long *moveSpeed )
{
	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		*moveSpeed = 0;
		return eMOVESPEED_RUN;
	}

	*moveSpeed = pMonsterScript->mActionWalkSpd;
	return eMOVESPEED_WALK;

	/*
	if( pMonster == NULL )
		return eMOVESPEED_RUN;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		*moveSpeed = 0;
		return eMOVESPEED_RUN;
	}

	/// 비율 합계
	unsigned short totalRatio = pMonsterScript->mActionStopRatio + pMonsterScript->mActionWalkRatio + pMonsterScript->mActionRunRatio;

	/// 랜덤 선택값
	if( totalRatio == 0 ) totalRatio = 1;
	unsigned short randSelect = (unsigned short)( rand() % totalRatio ) + 1;
	if( randSelect <= pMonsterScript->mActionStopRatio )
	{
		*moveSpeed = 0;
		/// 정지 선택
		return eMOVESPEED_STOP;
	}
	else if( randSelect <= pMonsterScript->mActionStopRatio + pMonsterScript->mActionWalkRatio  )
	{
		*moveSpeed = pMonsterScript->mActionWalkSpd;
		/// 걷기 선택
		return eMOVESPEED_WALK;
	}
	else
	{
		*moveSpeed = pMonsterScript->mActionRunSpd;
		/// 뛰기 선택
		return eMOVESPEED_RUN;
	}
	*/
}



cPlayer* cActionMove::TargetSelect( cMonster* pMonster, unsigned long seekRange )
{
	if( pMonster == NULL )
		return NULL;

	float tempRange;

	/// 주변 그리드에 플레이어가 있는지 먼저 체크 - 부하방지
	if( GRIDMANAGER->IsPlayer( pMonster ) == 0 )
		return NULL;

	/// 각각의 플레이어를 검색함
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pMonster );
	while( pPlayer != NULL )
	{
		/// 타겟 선정전 비중 확인
		if( pPlayer->GetMonsterImportance() + pMonster->GetFollowImportance() > 100 )
		{
			pPlayer = GRIDMANAGER->FindNextPlayer();
			continue;
		}

		/// 몬스터로 변신한 케릭터인 경우
		if( pPlayer->GetChgMonsterIdx() != 0 || pPlayer->GetStateOddity( eODDITYTYPE_AVOID_MON_LOOK ) != 0 )
		{
			/// 해당 몬스터가 변신한 플레이어를 인식하는지 안하는지 정보를 얻음
			sMonsterScript* pScript = MONSTERSCRIPT->GetMonsterListInfo( pMonster->GetRaceGender() );
			if( pScript == NULL )
			{
				pPlayer = GRIDMANAGER->FindNextPlayer();
				continue;
			}

			/// 변신 인식을 안하는 경우 변신플레이어 통과
			if( pScript->mIsChgMonCheck == false )
			{
				pPlayer = GRIDMANAGER->FindNextPlayer();
				continue;
			}
		}

		/// 플레이어가 있는데 죽은경우는 통과
		if( pPlayer->GetStateDie() != true ) 
		{
			tempRange = OBJECTMANAGER->ObjectSizeRange( pMonster, pPlayer, (float)seekRange );

			/// 몬스터 범위 설정
			mRangechk.SetRadius( tempRange );

			/// 몬스터 위치
			NiPoint3 monsterpoint( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
			/// 플레이어 위치
			NiPoint3 playerpoint( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );

			/// 못가는 지역인경우 통과
			if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), playerpoint.x, playerpoint.y, pPlayer->GetObject() ) == false )
			{
				pPlayer = GRIDMANAGER->FindNextPlayer();
				continue;
			}

			/// 몬스터와 플레이어 충돌 체크
			if( mRangechk.IsRange( monsterpoint, playerpoint ) )
			{
				return pPlayer;
			}
		}

		pPlayer = GRIDMANAGER->FindNextPlayer();
	}

	return NULL;
}
