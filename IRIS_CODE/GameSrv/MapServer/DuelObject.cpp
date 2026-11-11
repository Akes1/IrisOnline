#include "stdafx.h"
#include "gamesrv.h"
#include "Duelobject.h"
#include "Duel_Common.h"
#include "DuelManager.h"
#include "ObjectManager.h"
#include "SkillManager.h"


cDuelObject::cDuelObject()
{
}


cDuelObject::~cDuelObject()
{
}


void* cDuelObject::operator new( size_t n )
{
	if( n != sizeof(cDuelObject) ) 
	{
		assert(0);
		return NULL;
	}

	return DUELMANAGER->AllocDuelObject();
}


void cDuelObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cDuelObject) ) 
	{
		assert(0);
		return;
	}

	DUELMANAGER->FreeDuelObject( static_cast<cDuelObject*>(ptr) );

	return;
}


bool cDuelObject::Init( unsigned long duelIdx, unsigned long attackerIdx, unsigned long targetIdx )
{
	mDuelIdx = duelIdx;
	mAttackerIdx = attackerIdx;
	mTargetIdx = targetIdx;

	mDuelProcess = eDUELPROCESS_FIGHT_COUNT;

	mDelayTime = 0;

	mAttackerOutTime = 0;
	mTargetOutTime = 0;

	mAttacerRangeIn = true;
	mTargetRangeIn = true;

	mDieIdx = 0;

	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttackerIdx );
	if( pAttacker == NULL )
		return false;

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mTargetIdx );
	if( pTarget == NULL )
		return false;

	pAttacker->SetDuelIdx( mDuelIdx );	
	pTarget->SetDuelIdx( mDuelIdx );

	/// 메세지 대기상태 끝
	pTarget->EndRequestRejection( eREQREJCT_DUELWAIT );

	/// 결투 중심점 산출
	mCenterPos = pAttacker->GetPos() - ( ( pAttacker->GetPos() - pTarget->GetPos() ) / 2);

	/// 결투 시작 시간 설정
	mProcessWaitTime = NETWORK2->GetAccumTime() + DUEL_FIGHT_COUNT_TIME;

	::wcsncpy( mAttackerName, pAttacker->GetName(), MAX_NAME_SIZE ); 
	mAttackerName[MAX_NAME_SIZE] = 0;

	::wcsncpy( mTargetName, pTarget->GetName(), MAX_NAME_SIZE ); 
	mAttackerName[MAX_NAME_SIZE] = 0;

	return true;
}


void cDuelObject::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	/// 결투 시작 전 카운트
	switch( mDuelProcess )
	{
	case eDUELPROCESS_FIGHT_COUNT:
		{
			/// 결투시작 대기 카운트
			if( mProcessWaitTime > accumTime )
				return;

			/// 결투 시작 처리
			mDuelProcess = eDUELPROCESS_FIGHT;	
            FightStart();
		}
		break;
	case eDUELPROCESS_FIGHT:
		{
			/// 프로세스 부하 방지 - DUEL_FIGHT_DELAY_TIME 시간마다 체크
			if( mDelayTime > accumTime )
				return;
			
			/// 다음번 체크시간 설정
			mDelayTime = accumTime + DUEL_FIGHT_DELAY_TIME;

			/// 결투 진행중 체크 항목
			FightProcess( elapsedTime, accumTime );
		}
		break;
	case eDUELPROCESS_FIGHT_END:
		{
			/// 접속 종료가 아닌 공격으로 인해 결투가 끝난경우
			FightEnd( mDieIdx );
		}
		break;
	}

}


void cDuelObject::FightStart()
{
	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttackerIdx );
	if( pAttacker == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDuelObject::Process pAttacker[%d] == NULL", mAttackerIdx );
		FightEnd( mAttackerIdx );
		return;
	}

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mTargetIdx );
	if( pTarget == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDuelObject::Process pTarget[%d] == NULL", mTargetIdx );
		FightEnd( mTargetIdx );
		return;
	}

	NETWORK2->SendMsgRoot( pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_START_RES );
	NETWORK2->SendMsgRoot( pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_START_RES );
}


void cDuelObject::FightProcess( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	/// 결투 거리 체크
	mRangeCheck.SetRadius( DUEL_FIGHT_RANGE );

	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttackerIdx );
	if( pAttacker == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDuelObject::Process pAttacker[%d] == NULL", mAttackerIdx );
		FightEnd( mAttackerIdx );
		return;
	}

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mTargetIdx );
	if( pTarget == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDuelObject::Process pTarget[%d] == NULL", mTargetIdx );
		FightEnd( mTargetIdx );
		return;
	}

	if( mRangeCheck.IsNotRange( mCenterPos, pAttacker->GetPos() ) == true )
	{
		/// 이번 프레임에서 안에 있다가 나감
		if( mAttacerRangeIn == true )
		{
			/// 시간 설정
			mAttackerOutTime = accumTime;
			mAttacerRangeIn = false;

			/// 메세지 발송
			NETWORK2->SendMsgRoot( pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_FIGHT_OUTRANGE_RES );
		}

		/// 10초이상 나가있음
		if( mAttackerOutTime + DUEL_FIGHT_RANGE_TIME <= accumTime )
		{
			/// 플레이어1 패배
			FightEnd( mAttackerIdx );
		}
	}
	else
	{
		/// 이번 프레임에서 밖에 있다가 들어옴
		if( mAttacerRangeIn == false )
		{
			mAttacerRangeIn = true;

			/// 메세지 발송
			NETWORK2->SendMsgRoot( pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_FIGHT_INRANGE_RES );
		}
	}

	if( mRangeCheck.IsNotRange( mCenterPos, pTarget->GetPos() ) == true )
	{
		/// 이번 프레임에서 안에 있다가 나감
		if( mTargetRangeIn == true )
		{
			/// 시간 설정
			mTargetOutTime = accumTime;
			mTargetRangeIn = false;

			/// 메세지 발송
			NETWORK2->SendMsgRoot( pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_FIGHT_OUTRANGE_RES );
		}

		/// 10초이상 나가있음
		if( mTargetOutTime + DUEL_FIGHT_RANGE_TIME <= accumTime )
		{
			/// 플레이어2 패배
			FightEnd( mTargetIdx );
		}
	}
	else
	{
		/// 이번 프레임에서 밖에 있다가 들어옴
		if( mTargetRangeIn == false )
		{
			mTargetRangeIn = true;

			/// 메세지 발송
			NETWORK2->SendMsgRoot( pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_FIGHT_INRANGE_RES );
		}
	}
}


void cDuelObject::LastAttack( unsigned long dieIdx )
{
	/// 스킬 데미지 메세지 먼저 발송한후에 한프로세스 뒤에 결투종료 처리를 한다.
	mDuelProcess = eDUELPROCESS_FIGHT_END;	
	mDieIdx = dieIdx;
}


void cDuelObject::FightEnd( unsigned long dieIdx )
{
	if( mDuelProcess == eDUELPROCESS_END )
		return;

	sObject player;
	player.type = eOBJECTTYPE_PLAYER;

	HANDLE handle  = NULL;

	/// 연승
	unsigned short straightWin = 0;

	/// 객체 삭제 전에 여기서 먼저 호출되므로 객체가 NULL일수 없다.
	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( mTargetIdx );
	if( pTarget == NULL )	
	{
		assert(NULL);	
		NETWORK2->PostServerEvent("cDuelObject::FightEnd pTarget[%d] == NULL", mTargetIdx );
		DUELMANAGER->DelDuel( mDuelIdx );
		return;
	}

	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttackerIdx );
	if( pAttacker == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDuelObject::FightEnd pAttacker[%d] == NULL", mAttackerIdx );
		DUELMANAGER->DelDuel( mDuelIdx );
		return;
	}

	pTarget->EndRequestRejection( eREQREJCT_DUELPLAY );	/// 결투 상태 종료
	pTarget->SetDuelIdx( 0 );						/// 결투 설정 종료
	pTarget->DuelEndDebuffClear( mAttackerIdx );	/// 공격 받았던 디버프 제거
	pTarget->SetDuelPlayerIdx( 0, false );			/// 결투 대상 클리어
	pTarget->MoveStop();
		
	pAttacker->EndRequestRejection( eREQREJCT_DUELPLAY );
	pAttacker->SetDuelIdx( 0 );
	pAttacker->DuelEndDebuffClear( mTargetIdx );
	pAttacker->SetDuelPlayerIdx( 0, false );
	pAttacker->MoveStop();

	/// 결투 종료 시점에 채널링 스킬이 써져있으면 강제 종료 시키기.
	bool isDuelEnd = true;
	SKILLMANAGER->ChannelingSkillCancel( pAttacker->GetObject(), true, eATTRIBUTETYPE_NONE, isDuelEnd );
	SKILLMANAGER->ChannelingSkillCancel( pTarget->GetObject(), true, eATTRIBUTETYPE_NONE, isDuelEnd );

	MSG_SYN_DUEL_END synMsg;

	/// 승리/패배 처리
	if( dieIdx == mAttackerIdx )
	{
		/// 대상자 승리
		pTarget->SetDuelWin();					/// 승리 기록
		straightWin = (unsigned short)pTarget->GetDuelWin();	/// 연승

		/// 승리 버프 추가
		player.index = mTargetIdx;
		SKILLMANAGER->AddInfluence( player, player, DUEL_WIN_ADD_INFLUENCEIDX, 0, true );		

		/// 승리 메세지 발송
		MSG_SYN_DUEL_WINNER* winMsg = (MSG_SYN_DUEL_WINNER*)NETWORK2->GetMsgRoot( &handle, 
			pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_WINNER_SYN );
		if ( winMsg != NULL )
		{
			winMsg->mWinnerIdx = mTargetIdx;
			::wcsncpy( winMsg->mWinnerName, mTargetName, MAX_NAME_SIZE ); 
			winMsg->mLoserIdx = mAttackerIdx;
			::wcsncpy( winMsg->mLoserName, mAttackerName, MAX_NAME_SIZE ); 
			winMsg->mStraightWin = straightWin;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_DUEL_WINNER) );
		}


		/// 요청자 패배
		pAttacker->ClearDuelWin();	/// 연승 기록 제거

		/// 패배 버프 추가
		player.index = mAttackerIdx;
		SKILLMANAGER->AddInfluence( player, player, DUEL_LOSE_ADD_INFLUENCEIDX, 0, true );		

		/// 패배 메세지 발송
		MSG_SYN_DUEL_LOSER* loseMsg = (MSG_SYN_DUEL_LOSER*)NETWORK2->GetMsgRoot( &handle, 
			pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_LOSER_SYN );
		if ( loseMsg != NULL )
		{
			loseMsg->mWinnerIdx = mTargetIdx;
			::wcsncpy( loseMsg->mWinnerName, mTargetName, MAX_NAME_SIZE ); 
			loseMsg->mLoserIdx = mAttackerIdx;
			::wcsncpy( loseMsg->mLoserName, mAttackerName, MAX_NAME_SIZE ); 
			loseMsg->mStraightWin = straightWin;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_DUEL_LOSER) );
		}

		synMsg.Category = NM_DUEL;
		synMsg.Protocol = NM_DUEL_END_SYN;
		synMsg.mWinnerIdx = mTargetIdx;
		::wcsncpy( synMsg.mWinnerName, mTargetName, MAX_NAME_SIZE ); 
		synMsg.mLoserIdx = mAttackerIdx;
		::wcsncpy( synMsg.mLoserName, mAttackerName, MAX_NAME_SIZE ); 
		synMsg.mStraightWin = straightWin;
	}
	else
	{
		/// 요청자 승리
		pAttacker->SetDuelWin();				/// 승리 기록
		straightWin = (unsigned short)pAttacker->GetDuelWin();	/// 연승

		/// 승리 버프 추가
		player.index = mAttackerIdx;
		SKILLMANAGER->AddInfluence( player, player, DUEL_WIN_ADD_INFLUENCEIDX, 0, true );	

		/// 승리 메세지 발송
		MSG_SYN_DUEL_WINNER* winMsg = (MSG_SYN_DUEL_WINNER*)NETWORK2->GetMsgRoot( &handle, 
			pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_WINNER_SYN );
		if ( winMsg != NULL )
		{
			winMsg->mWinnerIdx = mAttackerIdx;
			::wcsncpy( winMsg->mWinnerName, mAttackerName, MAX_NAME_SIZE ); 
			winMsg->mLoserIdx = mTargetIdx;
			::wcsncpy( winMsg->mLoserName, mTargetName, MAX_NAME_SIZE ); 
			winMsg->mStraightWin = straightWin;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_DUEL_WINNER) );
		}


		/// 대상자 패배
		pTarget->ClearDuelWin();				/// 연승 기록 제거

		/// 패배 디버프 추가
		player.index = mTargetIdx;
		SKILLMANAGER->AddInfluence( player, player, DUEL_LOSE_ADD_INFLUENCEIDX, 0, true );		

		/// 패배 메세지 발송
		MSG_SYN_DUEL_LOSER* loseMsg = (MSG_SYN_DUEL_LOSER*)NETWORK2->GetMsgRoot( &handle, 
			pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_LOSER_SYN );
		if ( loseMsg != NULL )
		{
			loseMsg->mWinnerIdx = mAttackerIdx;
			::wcsncpy( loseMsg->mWinnerName, mAttackerName, MAX_NAME_SIZE ); 
			loseMsg->mLoserIdx = mTargetIdx;
			::wcsncpy( loseMsg->mLoserName, mTargetName, MAX_NAME_SIZE ); 
			loseMsg->mStraightWin = straightWin;

			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_DUEL_LOSER) );
		}

		synMsg.Category = NM_DUEL;
		synMsg.Protocol = NM_DUEL_END_SYN;
		synMsg.mWinnerIdx = mAttackerIdx;
		::wcsncpy( synMsg.mWinnerName, mAttackerName, MAX_NAME_SIZE ); 
		synMsg.mLoserIdx = mTargetIdx;
		::wcsncpy( synMsg.mLoserName, mTargetName, MAX_NAME_SIZE ); 
		synMsg.mStraightWin = straightWin;
	}

	/// 결투자 두명을 제외한 전체에게 맵단위 전송
	NETWORK2->QuickSendExceptMap( pAttacker, pTarget, (char*)&synMsg, sizeof(synMsg) );

	/// 삭제 배열에 추가
	DUELMANAGER->DelDuel( mDuelIdx );

	mDuelProcess = eDUELPROCESS_END;
}