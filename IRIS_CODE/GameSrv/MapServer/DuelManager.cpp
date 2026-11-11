#include "stdafx.h"

#include "gamesrv.h"
#include "DuelManager.h"
#include "ObjectManager.h"
#include "PVPManager.h"
#include "GridManager.h"
#include "Totem.h"


cDuelManager* cDuelManager::mpDuelManager = NULL;


cDuelManager::cDuelManager()
{
	if(mpDuelManager)
	{
		MessageBox(NULL,"cSkillManager : 이 클래스는 객체가 유일한객체(singleton)이어야 합니다.\
			이전 객체를 사용하던지 이전 객체를 지우고 생성하십시오.","경고",MB_OK);
	}
	else
	{
		mpDuelManager = this;
	}

}


cDuelManager::~cDuelManager()
{
	Release();
}


bool cDuelManager::Init()
{
	mDuelObjectPool.Reserve(100,10);

	return true;
}


void cDuelManager::Release()
{
	cPHashMap::cIterator start = mDuelMap.Begin();
	cPHashMap::cIterator end = mDuelMap.End();

	cDuelObject* pDuel = NULL;
	while( start != end )
	{
		pDuel = (cDuelObject*)(*start++).mSecond;
		SAFE_DELETE( pDuel );
	}
	mDuelMap.Clear();
}


void cDuelManager::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	for( unsigned long i = 0 ; i < mDelDuel.GetSize() ; ++i )
	{
		cDuelObject* pDuel = (cDuelObject*)mDuelMap.GetAt( mDelDuel[i] );
		if( pDuel == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent( "cDuelManager::Process mDelDuel[i] cDuelObject == NULL", i );
			continue;
		}

		SAFE_DELETE( pDuel );
		mDuelMap.Erase( mDelDuel[i] );
	}
	mDelDuel.Clear();	

	cPHashMap::cIterator start = mDuelMap.Begin();
	cPHashMap::cIterator end = mDuelMap.End();
	while( start != end )
	{
		cDuelObject* pDuel = (cDuelObject*)(*start++).mSecond;
		if( pDuel == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cDuelManager::Process mDuelMap cDuelObject == NULL");
			continue;
		}

		pDuel->Process( elapsedTime, accumTime );
	}
}


int cDuelManager::ReqDuel( unsigned long attackerIdx, unsigned long targetIdx )
{
	/// 객체 체크
	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attackerIdx );
	if( pAttacker == NULL || pAttacker->GetStateDie() == true || pAttacker->GetCheatHideMode() == true )
		return ERR_DUEL_ADD_FAIL;

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( targetIdx );
	if( pTarget == NULL || pTarget->GetStateDie() == true )
		return ERR_DUEL_ADD_FAIL;

    if( pAttacker->IsAutoTrial() == true || pTarget->IsAutoTrial() )
        return ERR_DUEL_ADD_FAIL;

	if( pAttacker->IsRequestRejection() == true )
		return ERR_DUEL_ADD_FAIL;

	/// 2차상태 체크
	if( pTarget->IsRequestRejection() == true )
		return ERR_DUEL_ADD_FAIL;

	if( pTarget->GetPvPDMIdx() != 0 )
		return ERR_DUEL_ADD_FAIL;

	if( pAttacker->GetPvPDMIdx() != 0  )
		return ERR_DUEL_ADD_FAIL;

	/// 옵션 상태로 인한 거절
	sGameOption* targetOption = pTarget->GetOptionData();
	if( targetOption->option1.rejectionDuel == true )
		return ERR_DUEL_ADD_OPTIONREFUSE;

	if( IsDuel( pAttacker, pTarget ) == false )
		return ERR_DUEL_ADD_FAIL;

	return ERR_DUEL_ADD_SUCCESS;
}


bool cDuelManager::IsDuel( cPlayer* pAttacker, cPlayer* pTarget )
{
	if( pAttacker == NULL )
		return false;

	if( pTarget == NULL )
		return false;

	if( pAttacker == pTarget )
		return false;

	/// 전투중인지 체크
	if( pAttacker->GetDuelIdx() != 0 )
		return false;
	if( pTarget->GetDuelIdx() != 0 )
		return false;

	/// 거래 체크
	if( pAttacker->GetExchangeStatus() > ItemExchangeAsk )
		return false;
	if( pTarget->GetExchangeStatus() > ItemExchangeAsk )
		return false;

	/// 상태 체크 - 변경은 하지 않는다.
	if( pAttacker->IsChangeState( eOBJECT_STATE_STOP ) == false )
		return false;
	if( pTarget->IsChangeState( eOBJECT_STATE_STOP ) == false )
		return false;

	/// 거리 체크
	mRangeCheck.SetRadius( DUEL_ACCEPT_READY_RANGE );
	if( mRangeCheck.IsNotRange( pTarget->GetPos(), pAttacker->GetPos() ) == true )
		return false;

	return true;
}


void cDuelManager::DuelAllReady( bool mIsAccept, cPlayer* pTarget )
{

	unsigned long attackerIdx = 0;
	unsigned long targetIdx = 0;

	cPlayer* pAttacker = NULL;

	try
	{
		attackerIdx = pTarget->GetDuelPlayerIdx();
		targetIdx = pTarget->GetObjectID();

		/// 객체 확인
		if( pTarget == NULL || pTarget->GetStateDie() == true )
			throw ERR_DUEL_ACCEPT_RES_ERROR;

		pAttacker = OBJECTMANAGER->GetPlayer( attackerIdx );
		if( pAttacker == NULL || pAttacker->GetStateDie() == true )
			throw ERR_DUEL_ACCEPT_REQ_ERROR;

		if( mIsAccept == false )
		{
			/// 결투 설정 끔
			pTarget->SetDuelPlayerIdx( 0, false );
			pAttacker->SetDuelPlayerIdx( 0, false );

			throw ERR_DUEL_ACCEPT_RES_REJECTION;
		}

		if( CreateDuel( pAttacker, pTarget ) == false )
			throw ERR_DUEL_ACCEPT_RES_ERROR;

	}
	catch ( int err )
	{		
		HANDLE handle  = NULL;

		if( pAttacker != NULL )
		{
			MSG_RES_DUEL_ACCEPT* sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
				pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
			if ( sendMsg != NULL )
			{
				sendMsg->mEnemyIdx = targetIdx;
				sendMsg->ErrorCode = err == ERR_DUEL_ACCEPT_RES_REJECTION ? ERR_DUEL_ACCEPT_RES_REJECTION : ERR_DUEL_ACCEPT_RES_REJECTION;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );
			}
		}

		if( pTarget != NULL )
		{
			pTarget->EndRequestRejection( eREQREJCT_DUELWAIT );

			/// 대상자가 거절인경우 이미 클라에서 처리했음
			if( ERR_DUEL_ACCEPT_RES_REJECTION != err )
			{
				MSG_RES_DUEL_ACCEPT* sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
					pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
				if ( sendMsg != NULL )
				{
					sendMsg->mEnemyIdx = attackerIdx;
					sendMsg->ErrorCode = ERR_DUEL_ACCEPT_RES_ERROR;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );
				}
			}
		}
	}
}


bool cDuelManager::CreateDuel( cPlayer* pAttacker, cPlayer* pTarget )
{
	if( pAttacker == NULL )
		return false;

	if( pTarget == NULL )
		return false;

	unsigned long attackerIdx = pAttacker->GetObjectID();
	unsigned long targetIdx = pTarget->GetObjectID();

	// 요청자의 상태 검사
	if( pAttacker->IsRequestRejection() == true )
		return false;

	if( IsDuel( pAttacker, pTarget ) == false )
		return false;

	/// 고유키 생성
	unsigned long duelIdx = mDuelGen.GeneratIdx();
	if( duelIdx == 0 )	
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent( "ERROR - CreateDuel GeneratIdx[%d]", duelIdx );
		return false;
	}

	/// 결투 객체 생성
	cDuelObject* pDuelObject = new cDuelObject;
	if( pDuelObject->Init( duelIdx, attackerIdx, targetIdx ) == false )
	{ 
		SAFE_DELETE( pDuelObject );
		assert(NULL);
		NETWORK2->PostServerEvent( "ERROR - CreateDuel new cDuelObject" );
		return false;
	}

	/// 결투 객체 등록
	if( mDuelMap.Insert( duelIdx, pDuelObject ) == false )
	{
		SAFE_DELETE( pDuelObject );
		assert(NULL);
		NETWORK2->PostServerEvent( "ERROR - CreateDuel mDuelMap.Insert Duplicate" );
		return false;
	}

	HANDLE handle  = NULL;

	/// 결투 시작 카운트 메세지 발송
	MSG_RES_DUEL_ACCEPT* sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
		pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mEnemyIdx = targetIdx;
		sendMsg->ErrorCode = ERR_DUEL_ACCEPT_SUCCESS;
		sendMsg->mCenterPos = pDuelObject->GetCenterPos();
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );
	}

	sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
		pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
	if ( sendMsg != NULL )
	{
		sendMsg->mEnemyIdx = attackerIdx;
		sendMsg->ErrorCode = ERR_DUEL_ACCEPT_SUCCESS;
		sendMsg->mCenterPos = pDuelObject->GetCenterPos();
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );
	}

	MSG_SYN_DUEL_ACCEPT synMsg;
	synMsg.Category = NM_DUEL;
	synMsg.Protocol = NM_DUEL_ACCEPT_SYN;
	synMsg.mAttackerIdx = attackerIdx;
	synMsg.mTargetIdx = targetIdx;

	TwoPlayerQuickSend( pAttacker, pTarget, (char*)&synMsg, sizeof(synMsg) );

	/// 결투 상태로 바꿈
	pAttacker->StartRequestRejection( eREQREJCT_DUELPLAY );
	pTarget->StartRequestRejection( eREQREJCT_DUELPLAY );

	return true;
}


void cDuelManager::DuelGiveUp( unsigned long duelIdx, unsigned long playerIdx )
{
	cDuelObject* pDuelObject = (cDuelObject*)mDuelMap.GetAt( duelIdx );
	if( pDuelObject == NULL )
		return;

	pDuelObject->FightEnd( playerIdx );
}


void cDuelManager::DelDuel( unsigned long duelIdx )
{
	mDelDuel.PushBack( duelIdx );
	mDuelGen.DelIdx( duelIdx );
}


void cDuelManager::TwoPlayerQuickSend( cPlayer* pPlayer1, cPlayer* pPlayer2, char* msg, int length )
{
	/// Player1, Player2에 해당하는 발송자 명단을 모두 읽어옴 - 중복유저 1개로 합침(set)
	if( pPlayer1 != NULL )
	{
		cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pPlayer1 );
		while( pPlayer != NULL )
		{
			mSendSet.Insert( pPlayer->GetConnectionIdx() );
			pPlayer = GRIDMANAGER->FindNextPlayer();
		}
	}

	if( pPlayer2 != NULL )
	{
		cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pPlayer2 );
		while( pPlayer != NULL )
		{
			mSendSet.Insert( pPlayer->GetConnectionIdx() );
			pPlayer = GRIDMANAGER->FindNextPlayer();
		}
	}

	cHashSet::cIterator begin = mSendSet.Begin();
	cHashSet::cIterator end = mSendSet.End();

	unsigned long conIdx;
	while( begin != end )
	{
		conIdx = (*begin++);

		if( pPlayer1->GetConnectionIdx() == conIdx )
			continue;

		if( pPlayer2->GetConnectionIdx() == conIdx )
			continue;

		NETWORK2->SendToUser( conIdx, msg, length );
	}
}


bool cDuelManager::DuelLastAttack( sObject attacker, sObject target, unsigned long damage, bool isCri, unsigned long* appDamage )
{
	bool die;
	*appDamage = 0;
	
	sObject attackerPlayer = attacker;
	/// 일반적인 공격 ///
	if( attacker.type != eOBJECTTYPE_PLAYER )
	{
		if( attacker.type == eOBJECTTYPE_TOTEM )
		{
			cTotem* pTotem = OBJECTMANAGER->GetTotem( attacker.type );
			if( pTotem == NULL )
				return false;

			attackerPlayer = pTotem->GetAttacker();
			NETWORK2->PostServerEvent("cDuelManager::DuelLastAttack attacker.type == eOBJECTTYPE_TOTEM[%d,%d][%d,%d]", 
				attacker.type, attacker.index, attackerPlayer.type, attackerPlayer.index );
		}
		else
			return false;
	}

	if( attackerPlayer.type != eOBJECTTYPE_PLAYER )
		return false;

	if( target.type != eOBJECTTYPE_PLAYER )
		return false;

	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attackerPlayer.index );
	if( pAttacker == NULL )
		return false;

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( target.index );
	if( pTarget == NULL )
		return false;

	unsigned long duelIdx = pAttacker->GetDuelIdx();
	cDuelObject* pDuel = (cDuelObject*)mDuelMap.GetAt( duelIdx );
	if( pDuel == NULL )
	{
		if( attackerPlayer.index == target.index && attackerPlayer.type == target.type )
			return false;

		/// pvp 공격 체크
		if( pAttacker->GetPvPDMIdx() == pTarget->GetPvPDMIdx() && pAttacker->GetPvPDMTeam() != pTarget->GetPvPDMTeam() )
			return false;
		else
			return true;	/// 결투가 아닌(끝) 플레이어간 공격 - 공격불가
	}

	if( pAttacker->IsAttack( target.index ) == false )
		return true;

	if( pTarget->GetHP() > damage )
		return false;

	/// 결투 마지막 공격 처리 ///
	*appDamage = pTarget->GetHP() - 1;
	pTarget->HPDamage( attackerPlayer, *appDamage , &die, false, isCri );

	pDuel->LastAttack( target.index );

	return true;

}


void cDuelManager::DuelObjectEnd( unsigned long attackerIdx, unsigned long targetIdx, unsigned long duelIdx, bool isAttackerEnd )
{
	HANDLE handle  = NULL;

	cDuelObject* pDuelObject = (cDuelObject*)mDuelMap.GetAt( duelIdx );
	if( pDuelObject == NULL )
	{
		/// 결투 진행상태가 아닌 경우
		
		cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attackerIdx );
		if( pAttacker != NULL )
		{
			MSG_RES_DUEL_ACCEPT* sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
				pAttacker->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
			if ( sendMsg != NULL )
			{
				sendMsg->mEnemyIdx = targetIdx;
				sendMsg->ErrorCode = ERR_DUEL_ACCEPT_RES_ERROR;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );

				pAttacker->EndRequestRejection( eREQREJCT_DUELPLAY );
			}
		}

		cPlayer* pTarget = OBJECTMANAGER->GetPlayer( targetIdx );
		if( pTarget != NULL )
		{
			MSG_RES_DUEL_ACCEPT* sendMsg = (MSG_RES_DUEL_ACCEPT*)NETWORK2->GetMsgRoot( &handle, 
				pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_ACCEPT_RES );
			if ( sendMsg != NULL )
			{
				sendMsg->mEnemyIdx = attackerIdx;
				sendMsg->ErrorCode = ERR_DUEL_ACCEPT_REQ_ERROR;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ACCEPT) );

				pTarget->EndRequestRejection( eREQREJCT_DUELPLAY );
			}
		}

	}
	/// 결투 진행 상태면 결투 객체에 승패를 설정해준다.
	else
	{
		if( isAttackerEnd == true )
			pDuelObject->FightEnd( attackerIdx );
		else
			pDuelObject->FightEnd( targetIdx );
	}
}
