#include "StdAfx.h"
#include "actionescape.h"
#include "GameSrv.h"

#include "Monster.h"
#include "SkillManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include "AIManager.h"



cActionEscape::cActionEscape(void) : cAction( eACT_ESCAPE )
{
	mIsSetPath = false;
}



cActionEscape::~cActionEscape(void)
{
}



void cActionEscape::ActionInit( cMonster* pMonster, unsigned long accumTime )
{
	if( pMonster == NULL )
		return;

	cAction::ActionInit( pMonster, accumTime );

	/// 도망 시작 시간 설정
	mEscapeStartTime = accumTime;

	/// 중간 코스 변경 여부
	mCourseChange = rand() % 2 == 0 ? true : false;

	/// 몬스터를 도망 속도로 설정
	pMonster->SetMoveSpeed( pMonster->GetMonsterInfo()->mActionRunSpd, eMOVESPEED_RUN );

	if( pMonster->GetMoveSpeed() <= 0 )
		return;

	/// 이동 최대/최소 거리를 구함
	unsigned long escapeMaxRange = ESCAPE_MAXIMUM_TIME / SECOND_THOUSAND * pMonster->GetMoveSpeed();
	unsigned long escapeMinRange = ESCAPE_MINIMUM_TIME / SECOND_THOUSAND * pMonster->GetMoveSpeed();

	pMonster->SendSpeech( eMONSTERTALK_ESCAPE );

	/// 스킬 사용이 끝날때 까지 경로설정 안함
	if( SKILLMANAGER->IsUsingSkill( eOBJECTTYPE_MONSTER, pMonster->GetObjectID() ) )
		return;

	/// 이동 좌표를 설정
	mIsSetPath = EscapeTargetPos( pMonster, escapeMinRange, escapeMaxRange, pMonster->GetXPos(), pMonster->GetYPos() );
}



eACTION_CHANGE cActionEscape::Action( cMonster* pMonster, unsigned long deltaTime, unsigned long accumTime )
{
	if( pMonster == NULL )
		return eACTCH_ERROR;

	const sMonsterScript* pMonsterScript = pMonster->GetMonsterInfo();
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cActionEscape::Action pMonsterScript == NULL");
		return eACTCH_ERROR;
	}

	/// 스킬 사용이 끝날때까지 대기
	if( SKILLMANAGER->IsUsingSkill( eOBJECTTYPE_MONSTER, pMonster->GetObjectID() ) )
		return eACTCH_CONTINUE;

	/// 도망 시작 시점부터 경과 시간
	unsigned long elapseTime = 0;
	if( accumTime > mEscapeStartTime )
		elapseTime = accumTime - mEscapeStartTime;

	/// 도망 경과 시간이 지난경우 상태 변경
	if( ESCAPE_MAXIMUM_TIME < elapseTime )
	{
		/// 081205 PKH 타겟 선정
		if( pMonster->GetTarget() == 0 )
			pMonster->ChangeActionTarget();
		else
			pMonster->ActionChange( eACT_FOLLOW );

		return eACTCH_CONTINUE;
	}

	/// 중간 코스 변경
	if( mCourseChange == true )
	{
		if( ESCAPE_CHANGE_TIME < elapseTime )
		{
			mCourseChange = false;

			/// 이동 설정
			long escapeMaxRange = 0;
			if( ESCAPE_MAXIMUM_TIME > elapseTime )
				escapeMaxRange = ESCAPE_MAXIMUM_TIME - elapseTime;

			if( pMonster->GetMoveSpeed() != 0 )
				escapeMaxRange = escapeMaxRange / SECOND_THOUSAND * pMonster->GetMoveSpeed();

			if( escapeMaxRange > 0 )
				EscapeTargetPos( pMonster, 0, escapeMaxRange, pMonster->GetXPos(), pMonster->GetYPos() );
		}
	}

	/// 경로 설정이 안되있는 경우
	if( mIsSetPath == false )
	{
		/// 이동 설정
		long escapeMaxRange = 0;
		if( ESCAPE_MAXIMUM_TIME > elapseTime )
			escapeMaxRange = ESCAPE_MAXIMUM_TIME - elapseTime;

		long escapeMinRange = 0;
		if( ESCAPE_MINIMUM_TIME > elapseTime )
			escapeMinRange = ESCAPE_MINIMUM_TIME - elapseTime;

		if( pMonster->GetMoveSpeed() != 0 )
		{
			escapeMaxRange = escapeMaxRange / SECOND_THOUSAND * pMonster->GetMoveSpeed();
			escapeMinRange = escapeMinRange / SECOND_THOUSAND * pMonster->GetMoveSpeed();
		}

		if( escapeMaxRange > 0 )
			mIsSetPath = EscapeTargetPos( pMonster, escapeMinRange, escapeMaxRange, pMonster->GetXPos(), pMonster->GetYPos() );
	}


	/// 경로 설정이 된경우
	if( mIsSetPath == true )
	{
		/// 몬스터 이동
		if( pMonster->MoveUpdate( deltaTime ) == false )
		{
			cBaseObject* pTarget = pMonster->GetTarget();
			if( pTarget == NULL )
				return eACTCH_ESCAPE_FOLLOW;

			if( pTarget->GetStateDie() == true )
				return eACTCH_ESCAPE_FOLLOW;

			/// 추적 시간 재설정 - 현재시간 + 추적시간 + 도망온시간
			pMonster->SetFollowEndTime( accumTime + pMonsterScript->mFollowTime + ( accumTime - mEscapeStartTime ) );

			NiPoint2 targetPos = pTarget->GetPos();
			/// 다시 쫓아갈 타겟이 못가는 지역 인경우 자유이동
			if( AIMANAGER->IsPossible( pTarget->GetMapNumber(), targetPos.x, targetPos.y, pTarget->GetObject() ) == false )
				return eACTCH_ESCAPE_END;
			else
				return eACTCH_ESCAPE_FOLLOW;
		}
	}

	return eACTCH_CONTINUE;
}


bool cActionEscape::EscapeTargetPos( cMonster* pMonster, unsigned long escapeMinRange, unsigned long escapeMaxRange, float centerX, float centerY  )
{
	if( pMonster == NULL )
		return false;

	/// 유효 거리
	long validityRange = escapeMaxRange - escapeMinRange;
	if( validityRange < 1 )
		validityRange = 1;

	/// 유효 범위 설정
	float escapePosX = centerX - validityRange + ( rand() % ( validityRange * 2 ) );
	float escapePosY = centerY - validityRange + ( rand() % ( validityRange * 2 ) );

	/// 설정된 범위에 최소 사이즈 만큼 증가
	if( escapePosX > centerX )
		escapePosX += escapeMinRange;
	else if( escapePosX < centerX )
		escapePosX -= escapeMinRange;
	if( escapePosY > centerY )
		escapePosY += escapeMinRange;
	else if( escapePosY < centerY )
		escapePosY -= escapeMinRange;

	/// 갈수 없는 지역이면 다시 선정
	if( AIMANAGER->IsPossible( pMonster->GetMapNumber(), escapePosX, escapePosY, pMonster->GetObject() ) == false )
		return false;

	return pMonster->SetPath( escapePosX, escapePosY ) != 0;
}
