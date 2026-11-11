#include "StdAfx.h"
#include "GameSrv.h"
#include "actionfollow.h"

#include "Monster.h"
#include "Player.h"
#include "SkillManager.h"
#include "AIManager.h"
#include "ObjectManager.h"
#include "GridManager.h"
#include "MonsterScript.h"



cActionFollow::cActionFollow(void) : cAction( eACT_FOLLOW )
{
	mRePathTime = 0;
}



cActionFollow::~cActionFollow(void)
{
}



void cActionFollow::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );

	/// 타겟 검사
	cBaseObject* pTarget = pMonster->GetTarget();
	if( pTarget == NULL || pTarget->GetStateDie() == true )
		return;

	mFollowTargetPos.x = pTarget->GetXPos();
	mFollowTargetPos.y = pTarget->GetYPos();

	mFirstFollowMove = true;

	mDelayTime = accumTime;

	mCantFollowPos = 0;

	mIsTalk = true;

	mNextSelectSkill = 0;

	/// 공격 성공이면 추적 시간 재설정
	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionFollow::ActionInit pMonsterScript == NULL");
		return;
	}
	pMonster->SetFollowEndTime( accumTime + pMonsterScript->mFollowTime );
}


eACTION_CHANGE cActionFollow::Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionFollow::Action pMonsterScript == NULL");
		return eACTCH_ERROR;
	}

	/// 타겟에 대해 스킬 발동
	cBaseObject* pTarget = (cBaseObject*)pMonster->GetTarget();
	if( pTarget == NULL || pTarget->GetStateDie() == true )
	{
		/// 081205 PKH 타겟 선정
		pMonster->ChangeActionTarget();
		return eACTCH_CONTINUE;
	}

	/// 스킬 재선정 체크
	float skillrange = pMonster->CalcStatusSkillRange( pMonsterScript->mSkillMinRange, (eRANGETYPE)pMonsterScript->mSkillMinRangeType );
	if( pMonster->GetAttackType() == eMONSTERATTACK_MAX || mNextSelectSkill < accumTime )
	{
		/// 스킬 사용후 지연시간 체크
		if( pMonster->IsSkillWaitEnd() == true )
		{
			eMONSTERATTACK_TYPE attackType = pMonster->SelectSkill();
			pMonster->SetAttackType( attackType );
			
			pTarget = (cBaseObject*)pMonster->GetTarget();
			if( pTarget == NULL || pTarget->GetStateDie() == true )
				return eACTCH_CONTINUE;

			if( attackType != eMONSTERATTACK_MAX )
			{
				sMonsterSkillScript* pMonsterSkillScript = SKILLSCRIPT->GetMonsterSkillInfo( pMonster->GetRaceGender(), attackType );
				if( pMonsterSkillScript != NULL )	/// 스킬 발동 거리
				{
					if( pMonsterSkillScript->mApplyType == eAPPLYTYPE_ENEMY )
						mNextSelectSkill = accumTime + 100;		/// 부하 방지
					else
						mNextSelectSkill = accumTime + 3000;	/// 동료한테 쓰는 스킬은 3초간 쓸때까지 쫓아감

					if( pMonsterSkillScript->mBoundType == eBOUNDTYPE_FIELD )
					{
						mRangechk.SetRadius( OBJECTMANAGER->ObjectSizeRange( pTarget, pMonster, (float)pMonsterScript->mSeekRange ) );
						if( mRangechk.IsNotRange( pTarget->GetPos(), pMonster->GetPos() ) == true )
						{
							pMonster->ChangeActionTarget();
							return eACTCH_CONTINUE;
						}
						else
							return eACTCH_FOLLOW_ATTACK;
					}
					else
						skillrange = pMonster->CalcStatusSkillRange( pMonsterSkillScript->mTargetDist, pMonsterSkillScript->mRangeType );
				}
			}
		}
	}

	/// 이동 안하는 몬스터 인경우
	if( pMonsterScript->mActionRunSpd == 0 && pMonsterScript->mActionWalkSpd == 0 )
	{
		/// 시아 범위안에 몬스터가 들어와있으면 공격한다.
		cPlayer* pPlayer = TargetSelect( pMonster, pMonsterScript->mSeekRange );
		if( pPlayer != NULL)
		{
			const sRegenScript* pRegen = pMonster->GetRegenMonsterInfo();
			if( pRegen != NULL && pRegen->mFamilyNum != 0 )
				AIMANAGER->FamilyAttack( pMonster->GetObjectID(), pRegen->mFamilyNum, pPlayer->GetObject() );

			pMonster->AddTakeDamage( pPlayer->GetObject(), 0, 0, eTAKEDAMAGETYPE_ADDPLAYER );
			return eACTCH_FOLLOW_ATTACK;
		}
		else
			return eACTCH_CONTINUE;
	}

	/// 공격거리 안에 들어온경우
	float totalRange = OBJECTMANAGER->ObjectSizeRange( pMonster, pTarget, skillrange );
	mRangechk.SetRadius( totalRange );
	if( mRangechk.IsRange( pMonster->GetPos(), pTarget->GetPos() ) )
	{
		if( SKILLMANAGER->MonsterAttack( pMonster->GetObjectID(), pMonster->GetAttackType(), pTarget->GetObject() ) == true )
			return eACTCH_FOLLOW_ATTACK;
		else
			return eACTCH_CONTINUE;
	}

	/// 추적 시간 체크
	mRangechk.SetRadius( (float)pMonsterScript->mFollowRange );

	bool isFollowEnd = false;

    if( pMonster->GetFollowEndTime() <= accumTime )
		isFollowEnd = true;

	if( mRangechk.IsNotRange(pMonster->GetPos(), pMonster->GetFollowStartPos()) )
		isFollowEnd = true;
	
	if( isFollowEnd == true )
	{
		if( pMonster->IsFollowContinue() == false )
		{
			/// 회기 상태 설정
			pMonster->ClearTakeDamage();
			pMonster->Comeback();
			return eACTCH_FOLLOW_END;
		}
		else
		{
			pTarget = (cBaseObject*)pMonster->GetTarget();
			if( pTarget == 0 )
			{
				pMonster->ClearTakeDamage();
				pMonster->Comeback();
				return eACTCH_FOLLOW_END;
			}
		}
	}

	NiPoint2 followPos = pTarget->GetPos();

	if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
	{
		cPlayer* pPlayer = (cPlayer*)pTarget;
		/// 플레이어에 추적 퍼짐 위치 재신청 - 가운데 자리가 빈 경우 가운데로 이동
		pMonster->SetPlayerFollowPos( pPlayer->SelectFollowPos( pMonster->GetPlayerFollowPos() ) );

		/// 추적 퍼짐위치 선정 nipoint2 -> nipoint3
		followPos = OBJECTMANAGER->CalcFollowPos( pMonster->GetPlayerFollowPos(), pPlayer->GetPos(), totalRange );

		/// 몬스터 추변위치 설정인지 실제 위치인지 유무
		bool playerRealPos = false;

		/// 추적 퍼짐 위치가 이동 가능 지역인지 체크
		if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), followPos.x, followPos.y, pPlayer->GetObject() ) == true )
		{
			/// 추적 퍼짐 위치 까지 도달 한경우 실제 위치 설정
			mRangechk.SetRadius( totalRange );
			if( mRangechk.IsRange( pMonster->GetPos(), followPos ) == true )
			{
				playerRealPos = true;
				followPos = pPlayer->GetPos();

				if( SKILLMANAGER->MonsterAttack( pMonster->GetObjectID(), pMonster->GetAttackType(), pPlayer->GetObject() ) == true )
					return eACTCH_FOLLOW_ATTACK;
			}
		}
		else	/// 추적 퍼짐위치가 못가는 지역이므로 실제 위치로 간다.
			playerRealPos = true;

		/// 실제 위치로 이동 해야 하는데
		if( playerRealPos == true )
		{
			/// 못가는 지역 인경우
			if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->GetObject() ) == false )
			{
				/// 카운트 증가
				++mCantFollowPos;

				/// 랜덤 좌표 선택
				NiPoint2 randPos( pPlayer->GetXPos(), pPlayer->GetYPos() );

				float fixedSkillRange = pMonsterScript->mSkillMinRange;

				switch( mCantFollowPos )
				{
				case 1:	randPos.x -= fixedSkillRange / 2; break;	/// 높이 거리를 감안해 2로 나눈다.
				case 2:	randPos.x += fixedSkillRange / 2; break;
				case 3:	randPos.y -= fixedSkillRange / 2; break;
				case 4:	randPos.y += fixedSkillRange / 2; break;
				case 5:	randPos.x -= fixedSkillRange / 3; randPos.y += fixedSkillRange / 3; break;	/// 높이와 대각선 거리를 감안해 3으로 나눈다.
				case 6:	randPos.x += fixedSkillRange / 3; randPos.y += fixedSkillRange / 3; break;
				case 7:	randPos.x -= fixedSkillRange / 3; randPos.y -= fixedSkillRange / 3; break;
				case 8:	randPos.x += fixedSkillRange / 3; randPos.y -= fixedSkillRange / 3; break;
				}

				/// 못가는 지역 인경우
				if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), randPos.x, randPos.y, pPlayer->GetObject() ) == false )
				{
					/// 위에서 정한 8방향 좌표가 전부 못가는 지역일경우 도망
					if( mCantFollowPos >= 8 )
					{
						//return eACTCH_ESCAPE;
						pMonster->ChangeActionTarget();
					}

					return eACTCH_CONTINUE;
				}
				else
				{
					followPos.x = randPos.x;
					followPos.y = randPos.y;
					mCantFollowPos = 0;
					totalRange = 0.0f;
					skillrange = 0.0f;
				}
			}
		}
	}


	/// 몬스터 현재위치와 타겟 거리를 계산
	float range = ( pMonster->GetPos() - pTarget->GetPos() ).Length() * 0.1f;

	/// range 값이 100보다 작을순 없다.
	if( range < 100 ) 
		range = 100; 

	/// 현재 타겟의 예전에 기록한 위치와 현재 위치의 거리차를 가져옴
	float objectLength = ( mFollowTargetPos - pTarget->GetPos() ).Length();

	/// object기록 위치랑 현재위치가 range값보다 큰경우 위치 object위치기록 갱신 - 노드갱신
	if( objectLength > range )
	{
		/// 0.5초마다 검사 - 부하를 줄이기 위함
		if( mDelayTime > accumTime )
		{
			sObject target;
			target.index = pTarget->GetObjectID();
			target.type = pTarget->GetObjectType();

			/// 몬스터 추적 속도
			pMonster->SetMoveSpeed( pMonster->GetMonsterInfo()->mActionRunSpd, eMOVESPEED_RUN );

			/// 길찾기 노드 재생성
			pMonster->SetPath( followPos.x, followPos.y, skillrange, pTarget->GetObject() );

			/// 스킬 사용 가능인지 체크
			if( pMonster->IsSkillWaitEnd() == true && mIsTalk == true )
			{
				mIsTalk = false;
				pMonster->SendSpeech( eMONSTERTALK_FOLLOW );
			}

			/// 플레이어가 이동 할 경우 길찾기 재설정을 위해 플레이어 위치 기록
			mFollowTargetPos.x = pTarget->GetXPos();
			mFollowTargetPos.y = pTarget->GetYPos();
		}
		else
			mDelayTime = mDelayTime + 500;

	}


	/// 몬스터 이동
	if( pMonster->MoveUpdate( deltaTime ) == false || mFirstFollowMove == true )
	{
		mFirstFollowMove = false;
		sObject target;
		target.index = pTarget->GetObjectID();
		target.type = pTarget->GetObjectType();

		/// 몬스터 추적 속도
		pMonster->SetMoveSpeed( pMonster->GetMonsterInfo()->mActionRunSpd, eMOVESPEED_RUN );

		if(	mRePathTime < accumTime )
		{
			mRePathTime = accumTime + 300;
			/// 스킬 범위 밖이면 다시 이동 셋팅
			pMonster->SetPath( followPos.x, followPos.y, skillrange, pTarget->GetObject() );
		}

		/// 스킬 사용 가능인지 체크
		if( pMonster->IsSkillWaitEnd() == true && mIsTalk == true )
		{
			mIsTalk = false;
			pMonster->SendSpeech( eMONSTERTALK_FOLLOW );
		}

		/// 플레이어가 이동 할 경우 길찾기 재설정을 위해 플레이어 위치 기록
		mFollowTargetPos.x = pTarget->GetXPos();
		mFollowTargetPos.y = pTarget->GetYPos();
	}

	/// 다른 이벤트가 걸리기 전까지 계속 추적모드
	return eACTCH_CONTINUE;
}


cPlayer* cActionFollow::TargetSelect( cMonster* pMonster, unsigned long seekRange )
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
			if( pScript != NULL && pScript->mIsChgMonCheck == false )
			{
				/// 공격을 받은 정보가 있는지 체크
				PerTakeDamage* per = TAKEDAMAGEPOOL->GetTakeDamage( pMonster->GetTakeDamageRoot(), pPlayer->GetObjectID() );
				if( per == NULL )
				{
					pPlayer = GRIDMANAGER->FindNextPlayer();
					continue;
				}
			}
		}

		/// 플레이어가 죽지 않은 경우만
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
