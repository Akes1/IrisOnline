#include "stdafx.h"
#include "gamesrv.h"
#include "PartyManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include "InfluenceObject.h"
#include "SkillManager.h"
#include "GridManager.h"

cPartyManager* cPartyManager::mpSingleton = NULL;

// cPartyManager Constructor
cPartyManager::cPartyManager( )
{
	mpSingleton = this;
	mpPartyJoinPool = NULL;
}

// ~cPartyManager Destructor
cPartyManager::~cPartyManager( )
{
	Release( );
	mpSingleton = 0;
}

// Init Method
bool cPartyManager::Init()
{
	mPartyPool.Reserve( 100, 10 );
	mpPartyJoinPool = new cPartyJoinPool( );
	return true;
}

// Release Method
void cPartyManager::Release( )
{
	// Memory Pool - cPartyJoinPool class.
	SAFE_DELETE( mpPartyJoinPool );
}

// Process Method
void cPartyManager::Process(unsigned long /*elapsedTime*/, unsigned long accumTime)
{
	cPartyMap::cIterator begin = mPartyMap.Begin( );
	cPartyMap::cIterator end   = mPartyMap.End( );
	unsigned long lostPlayer;

	while ( begin != end )
	{
		cParty* party = (*begin++).mSecond;
		if ( party != NULL )
		{
			// 파티(원)정보 업데이트
			if ( accumTime > party->GetLastAccumTime( )  )
			{
				party->SendUpdate( );
				party->SetLastAccumTime( accumTime+PARTY_UPDATETIME );
			}

			// 주사위 굴리기
			if ( party->GetDiceEndTime( ) > 0 )
			{
				if ( party->IsDiceRollEnd( ) || party->GetDiceEndTime( ) <= accumTime )
					SendDiceResult( party );
			}

			// 파티(원)관리
			lostPlayer = party->GetLostPlayer( GetTickCount( )-PARTY_DELETETIME );
			if ( lostPlayer > 0 )
			{
				PartyDelPlayer( party, lostPlayer );
			}
		}
	}


	/*-- 파티초대 응답 대기자 처리 --*/

	PartyJoinRoot* root = mpPartyJoinPool->GetRoot( );
	PartyJoin*     join = (PartyJoin*)root->pool;
	PartyJoin*     next = join;
	DWORD          tick = GetTickCount( );

	cPlayer*           player;
	ePLAYER_PARTYSTATE state;

	while ( join != NULL )
	{
		next = (PartyJoin*)join->next;
		if ( tick > join->delay )
		{
			player = OBJECTMANAGER->GetPlayer( join->playerDat.userIndex );
			if ( player != NULL )
			{
				if ( player->GetPartyState( ) == ePARTY_SENDJOIN )
				{
					state = (player->GetPartyIndex( ) > 0) ? ePARTY_COMPLETE : ePARTY_NONE;
					player->SetPartyState( state );

					NETWORK2->SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_USER );
				}
			}
			mpPartyJoinPool->Release( join );
		}
		join = next;
	}
}

// ResPartyList Method
bool cPartyManager::ResPartyList(cPlayer* player, cParty* party)
{
	HANDLE              handle  = NULL;
	MSG_SYN_PARTY_LIST* sendMsg = (MSG_SYN_PARTY_LIST*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_LIST_SYN );

	if ( sendMsg != NULL )
	{
		unsigned long   length     = sizeof(MSG_SYN_PARTY_LIST) - sizeof(sendMsg->mTable);
		unsigned long*  userArr    = party->GetUserArr( );
		unsigned int    partyCount = party->GetCount( );

		unsigned char&  rowCount   = sendMsg->RowCount;
		sPartyUserData* data       = sendMsg->mTable;

		sendMsg->mIndex       = party->GetIndex( );
		sendMsg->mLeaderIndex = party->GetLeader( );
		sendMsg->mDivideType  = (char)party->GetDivideType( );
		sendMsg->RowCount     = 0;

		for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
		{
			// 자기자신 제외
			cPlayer* user = OBJECTMANAGER->GetPlayer( (*userArr) );

			if ( user != NULL )
				user->GetPartyUserData( data );
			else
				(*data) = party->GetUserData( )[ i ];

			data++;
			rowCount++;
		}

		if( partyCount == 0 )
		{
			NETWORK2->PostServerEvent( "Error cPartyManager::ResPartyList : partycount == 0" );
		}

		length += (sendMsg->RowCount * sizeof(sendMsg->mTable));
		return NETWORK2->SendMsgRoot( handle, length );
	}
	return true;
}	

// ResPartyAddOne Method - 파티원 추가 정보 보내기
bool cPartyManager::ResPartyAddOne(cPlayer* player, cParty* party)
{
	unsigned long* userArr    = party->GetUserArr( );
	unsigned int   partyCount = party->GetCount( );

	// 추가된 유저를 제외한 나머지 파티원에게 보내기
	for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
	{
		cPlayer* user = OBJECTMANAGER->GetPlayer( (*userArr) );
		if ( user != NULL )
		{
			if ( player != user )
			{
				HANDLE                handle  = NULL;
				MSG_SYN_PARTY_ADDONE* sendMsg = (MSG_SYN_PARTY_ADDONE*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_PARTY, NM_PARTY_ADDONE_SYN );
				if ( sendMsg != NULL )
				{
					player->GetPartyUserData( &sendMsg->mTable );
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_ADDONE) );
				}
			}
		}
	}

	return true;
}

// PartyDelPlayer Method - 파티유저 탈퇴
void cPartyManager::PartyDelPlayer(cParty* party, unsigned long playerIdx)
{
	unsigned long partyIdx = party->GetIndex( );

	/*-- 파티 리더처리 --*/
	unsigned long leaderIdx = party->GetLeader( );

	if ( party->IsLeader( playerIdx ) && party->GetCount( ) > 2 )
	{
		leaderIdx = party->NextLeader( );				// 다음 파티장 검색
		party->SetLeader( leaderIdx );					// 파티장 세팅
		party->SendPartyLeaderSyn( leaderIdx );			// 파티장 위임 알림 
	}

	/*-- 파티 탈퇴처리 --*/
	if ( party->GetCount( ) > 2 )
	{
		party->SendPartyOutSyn( playerIdx );			// 탈퇴 알림
		party->DeleteUser( playerIdx );					// 탈퇴-파티원
		PostDelUser( partyIdx, leaderIdx, playerIdx );	// 채널 동기화
	}
	else
	{
		party->SendPartyDeleteSyn( );					// 삭제 알림
		DelParty( &party );								// 삭제-파티
		PostDelParty( partyIdx );						// 채널 동기화
	}
}

// PartyOutPlayer Method - 파티유저 탈퇴
void cPartyManager::PartyOutPlayer(cPlayer* player)
{
	/*-- 탈퇴인증 --*/
	unsigned long partyIdx  = player->GetPartyIndex( );
	unsigned long playerIdx = player->GetObjectID( );

	if ( partyIdx == 0 )
		throw ERROR_PARTY_OUT_FAIL;

	cParty* party = GetParty( partyIdx );
	if ( party == NULL )
	{
		player->SetPartyIndex( 0 );
		player->SetPartyState( ePARTY_NONE );
		throw ERROR_PARTY_OUT_FAIL;
	}

	if ( party->IsUser( playerIdx ) == false )
		throw ERROR_PARTY_OUT_FAIL;

	/*-- 파티원 탈퇴처리 --*/
	player->SetPartyIndex( 0 );
	player->SetPartyState( ePARTY_NONE );

	/*-- 파티 리더처리 --*/
	unsigned long leaderIdx = party->GetLeader( );

	if ( party->IsLeader( playerIdx ) && party->GetCount( ) > 2 )
	{
		leaderIdx = party->NextLeader( );				// 다음 파티장 검색
		party->SetLeader( leaderIdx );					// 파티장 세팅
		party->SendPartyLeaderSyn( leaderIdx );			// 파티장 위임 알림 
	}

	/*-- 파티 탈퇴처리 --*/
	if ( party->GetCount( ) > 2 )
	{
		party->DeleteUser( playerIdx );					// 탈퇴-파티원
		party->SendPartyOutSyn( playerIdx );			// 탈퇴 알림
		PostDelUser( partyIdx, leaderIdx, playerIdx );	// 채널 동기화
	}
	else
	{
		party->DeleteUser( playerIdx );					// 삭제-파티원
		party->SendPartyDeleteSyn( );					// 삭제 알림
		DelParty( &party );								// 삭제-파티
		PostDelParty( partyIdx );						// 채널 동기화
	}
}

// PartyThrowPlayer Method
void cPartyManager::PartyThrowPlayer(cPlayer* leader, unsigned long playerIdx)
{
	/*-- 추방인증 --*/
	unsigned long partyIdx  = leader->GetPartyIndex( );
	unsigned long leaderIdx = leader->GetObjectID( );

	if ( partyIdx == 0 )
		throw ERROR_PARTY_THROW_FAIL;

	cParty* party = PARTYMAN->GetParty( partyIdx );
	if ( party == NULL )
	{
		leader->SetPartyIndex( 0 );
		leader->SetPartyState( ePARTY_NONE );
		throw ERROR_PARTY_THROW_FAIL;
	}

	if ( party->IsLeader( leaderIdx ) == false )
		throw ERROR_PARTY_THROW_LEADER;

	if ( party->IsUser( playerIdx ) == false )
		throw ERROR_PARTY_THROW_FAIL;

	/*-- 파티원 추방알림 --*/
	cPlayer* player = OBJECTMANAGER->GetPlayer( playerIdx );
	if ( player != NULL )
	{
		NETWORK2->SendMsgRoot( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_CUT_SYN );
		player->SetPartyIndex( 0 );
		player->SetPartyState( ePARTY_NONE );
	}

	/*-- 파티 추방처리 --*/
	if ( party->GetCount( ) > 2 )
	{
		party->DeleteUser( playerIdx );					// 삭제-파티원
		party->SendPartyThrowSyn( playerIdx );			// 탈퇴 알림
		PostOutUser( partyIdx, leaderIdx, playerIdx );	// 채널 동기화
	}
	else
	{
		party->DeleteUser( playerIdx );					// 삭제-파티원
		party->SendPartyDeleteSyn( );					// 삭제 알림
		DelParty( &party );								// 삭제-파티
		PostDelParty( partyIdx );						// 채널 동기화
	}
}

// PartyChangeLeader Method - 파티장 변경 
void cPartyManager::PartyChangeLeader(cPlayer* leader, unsigned long playerIdx)
{
	/*-- 파티장 변경 인증 --*/
	unsigned long partyIdx = leader->GetPartyIndex( );

	if ( partyIdx == 0 )
		throw ERROR_PARTY_CHANGELEADER_FAIL;

	cParty* party = GetParty( partyIdx );
	if ( party == NULL )
	{
		leader->SetPartyIndex( 0 );
		leader->SetPartyState( ePARTY_NONE );
		throw ERROR_PARTY_CHANGELEADER_FAIL;
	}

	if ( party->IsLeader( leader->GetObjectID( ) ) == false )
		throw ERROR_PARTY_CHANGELEADER_LEADER;

	if ( party->IsUser( playerIdx ) == false )
		throw ERROR_PARTY_CHANGELEADER_FAIL;

	/*-- 파티장 변경처리 --*/
	party->SetLeader( playerIdx );				// 파티장 정보 재세팅
	party->SendPartyLeaderSyn( playerIdx );		// 파티장 위임 알림 
	PostSynLeader( party );						// 채널 동기화
}

// PartyChangeDivide Method - 파티분배방식 변경
void cPartyManager::PartyChangeDivide(cPlayer* leader, PARTY_DIVIDE_TYPE type)
{
	/*-- 분배방식 인증 --*/
	unsigned long partyIdx = leader->GetPartyIndex( );

	if ( partyIdx == 0 )
		throw ERROR_PARTY_CHANGEDIVIDE_FAIL;

	cParty* party = GetParty( partyIdx );

	if ( party == NULL )
	{
		leader->SetPartyIndex( 0 );
		leader->SetPartyState( ePARTY_NONE );
		throw ERROR_PARTY_CHANGEDIVIDE_FAIL;
	}

	// 검증
	if( type == PARTY_DIVIDE_NONE || type == PARTY_DEVIDE_MAX )
		throw ERROR_PARTY_CHANGEDIVIDE_FAIL;

	if ( party->IsLeader( leader->GetObjectID( ) ) == false )
		throw ERROR_PARTY_CHANGEDIVIDE_LEADER;
	
	if ( party->GetDivideType( ) == type )
		throw ERROR_PARTY_CHANGEDIVIDE_LEADER;

	/*-- 분배방식 처리 --*/
	party->SetDivideType( type );					// 분배방식 변경
	party->SendPartyDivideType( (char)type );		// 분배방식 변경 알림
	PostSynDivideType( party );							// 채널 동기화
}

// PartyMapIn Method - 파티원 맵체인지
void cPartyManager::PartyMapIn(cPlayer* player)
{
	ePLAYER_PARTYSTATE state = player->GetPartyState( );

	// 파티중이라면
	if ( state == ePARTY_COMPLETE )
	{
		cParty* party = GetParty( player->GetPartyIndex( ) );
		if ( party )
		{
			// 다른유저들에게 맵 체인지 알리기
			cPlayer*       partyUser  = 0;
			unsigned long* userArr    = party->GetUserArr( );
			unsigned int   partyCount = party->GetCount( );

			// 맵 체인지 알리기 - 체인지한 유저만 안 보냄.
			for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
			{
				partyUser = OBJECTMANAGER->GetPlayer( (*userArr) );
				if ( partyUser != NULL )
				{
					if ( partyUser != player )
					{
						player->SendPartyMapchangeSyn( partyUser->GetConnectionIdx( ) );
					}
				}
			}

			// 채널동기화 - 맵 체인지 알리기.
			PostSynMapData( party->GetIndex( ), player );

			// 이동한유저들에겐 파티 정보 새로 보내기
			ResPartyList( player, party );

			// 주사위 굴리기 포기 설정
			if ( party->GetDiceEndTime() > 0 )
			{
				sDiceMember* pMember = party->GetDiceMember( player->GetObjectID( ) );
				if ( pMember && pMember->mRoll == false )
				{
					HANDLE                          handle  = NULL;
					MSG_SYN_PARTY_DICE_REMAIN_TIME* sendMsg = (MSG_SYN_PARTY_DICE_REMAIN_TIME*)NETWORK2->GetMsgRoot( &handle
																													,player->GetConnectionIdx( )
																													,NM_PARTY
																													,NM_PARTY_DICE_REMAIN_TIME_SYN );
					if ( sendMsg != NULL )
					{
						sendMsg->mTime = party->GetDiceEndTime( ) - NETWORK2->GetAccumTime( );
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_DICE_REMAIN_TIME) );
					}
				}
			}
		}
		else
			NETWORK2->PostServerEvent( "Error cPartyManager::PartyMapIn : not exist party" );
	}
}

// SendToJobChange Method - 전직 정보 보내기
void cPartyManager::SendToJobChange(cPlayer* player)
{
	cParty* party = GetParty( player->GetPartyIndex( ) );
	if ( party != NULL )
	{
		cPlayer*       partyUser  = 0;
		unsigned long* userArr    = party->GetUserArr( );
		unsigned int   partyCount = party->GetCount( );

		for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
		{
			if ( (*userArr) != player->GetObjectID( ) )
			{
				partyUser = OBJECTMANAGER->GetPlayer( (*userArr) );
				if ( partyUser )
				{
					HANDLE					 handle  = NULL;
					MSG_SYN_PARTY_CHANGEJOB* sendMsg = (MSG_SYN_PARTY_CHANGEJOB*)NETWORK2->GetMsgRoot( &handle
																									  ,partyUser->GetConnectionIdx( )
																									  ,NM_PARTY
																									  ,NM_PARTY_JOBCHANGE_SYN );
					if ( sendMsg != NULL )
					{
						sendMsg->userIndex = player->GetObjectID();
						sendMsg->job       = (unsigned short)player->GetJob();
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_CHANGEJOB) );
					}
				}
			}
		}
	}
}

// SendToPartyHunt Method - 파티원이 몬스터 사냥한 정보를 보낸다 (퀘스트 업데이트용)
void cPartyManager::SendToPartyHunt(cMonster* monster, cPlayer* player, unsigned int racegender)
{
	cParty* party = GetParty( player->GetPartyIndex( ) );
	if ( party != NULL )
	{
		cPlayer*       partyUser  = 0;
		unsigned long* userArr    = party->GetUserArr( );
		unsigned int   partyCount = party->GetCount( );

		for ( unsigned int i = 0; i < partyCount; ++i, ++userArr )
		{
			partyUser = OBJECTMANAGER->GetPlayer( (*userArr) );
			if ( partyUser != NULL )
			{
				// 거리 체크
				if ( monster->GetMapNumber( ) == partyUser->GetMapNumber( ) )
				{
					NiPoint2 monPos  = monster->GetPos( );
					NiPoint2 userPos = partyUser->GetPos( );

					// 일정 범위안에 있으면 퀘스트 정보 갱신
					cRangeCheck range( PARTY_RANGE );
					if ( range.IsRange( monPos, userPos ) == true )
					{
						// 파티원 퀘스트 정보 갱신
						partyUser->UpdateDutyHunt( racegender, player->GetObjectID() );
					}
				}
			}
		}
	}
}

// PartyDiceStart Method
void cPartyManager::PartyDiceStart(cPlayer* player)
{
	unsigned long partyIdx = player->GetPartyIndex( );
	cParty*       party;

	if ( partyIdx == 0 )
		throw ERROR_DICE_START_PARTY_STATE;

	party = PARTYMAN->GetParty( partyIdx );

	if ( party == NULL )
		throw ERROR_DICE_START_PARTY_STATE;

	if ( party->IsLeader( player->GetObjectID( ) ) == false )
        throw ERROR_DICE_START_PARTY_LEADER;

	if ( party->DiceStart( ) == false )
        throw ERROR_DICE_START_FAIL;

	PostSynDiceStart( partyIdx );
}

// PartyDiceResult Method
void cPartyManager::PartyDiceResult(cPlayer* player, unsigned short value)
{
	unsigned long partyIdx  = player->GetPartyIndex( );
	unsigned long playerIdx = player->GetObjectID( );

	cParty*       party;
	int           retvalue;

	if ( partyIdx == 0 )
		throw ERROR_DICE_ROLL_PARTY_STATE;

	party = PARTYMAN->GetParty( partyIdx );
	if ( party == NULL )
		throw ERROR_DICE_ROLL_PARTY_STATE;

	retvalue = party->DiceRollReq( playerIdx, value );
	if ( retvalue == ERROR_DICE_ROLL_SUCCESS )
	{
		PostSynDiceResult( partyIdx, playerIdx, value );
	}
	else
		throw retvalue;
}

// SendDiceResult Method
void cPartyManager::SendDiceResult(cParty* party)
{
	PerIoContext*           ioContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_PARTY_DICE_END* sendMsg   = (MSG_SYN_PARTY_DICE_END*)ioContext->buffer;

	sendMsg->Category = NM_PARTY;
	sendMsg->Protocol = NM_PARTY_DICE_END_SYN;

	// 굴리기 선택한 멤버만..
	short rollCnt  = 0;
	short totalCnt = 0;
	for ( unsigned int i = 0; i < party->GetDiceCount(); i++ )
	{
		sDiceMember* pMember = party->GetDiceMember( i );
		if ( pMember )
		{
			if( pMember->mRandom > 0 )
			{
				sendMsg->mResult[rollCnt].mRandom = pMember->mRandom;
				wcscpy( sendMsg->mResult[rollCnt].mCharName, pMember->mCharName );
				++rollCnt;
			}
			++totalCnt;
		}
	}

	sendMsg->mNotRollCnt = totalCnt - rollCnt;
	sendMsg->mTotalCnt   = totalCnt;
	sendMsg->mRowCount   = rollCnt;

	// 주사위 참여 멤버 중 파티원에만 전송
	for ( unsigned int i = 0; i < party->GetDiceCount( ); i++ )
	{
		sDiceMember* pMember = party->GetDiceMember( i );
		if ( pMember != NULL )
		{
			cPlayer* partyUser = OBJECTMANAGER->GetPlayer( pMember->mUserIdx );
			if ( partyUser && party->GetIndex( ) == partyUser->GetPartyIndex( ) )
			{
				NETWORK2->SendToUser( partyUser->GetConnectionIdx( ), (char*)sendMsg, sendMsg->GetMsgLength( ) );
			}
		}
	}

	ioContext->offset = sendMsg->GetMsgLength( );
	NETWORK2->ReleaseIoContext( ioContext );

	party->DiceRelease( );
}

// SendToPartyAll Method
void cPartyManager::SendToPartyAll(cParty* party, char* msg, unsigned long length)
{
	unsigned long* pUserArr = party->GetUserArr( );
	for ( int i = 0; i < (int)party->GetCount( ); i++ )
	{
		if ( pUserArr[ i ] )
		{
			cPlayer* partyUser = OBJECTMANAGER->GetPlayer( pUserArr[ i ] );
			if ( partyUser != NULL )
			{
				NETWORK2->SendToUser( partyUser->GetConnectionIdx(), msg, length );
			}
		}
	}
}

// CheckPlayer Method
int cPartyManager::CheckPlayer(cPlayer* player)
{
	sGameOption*       gameOption = player->GetOptionData( );
	ePLAYER_PARTYSTATE partyState = player->GetPartyState( );

	if ( player->GetPartyUnionIndex( ) > 0 )			// 파티연합 상태 검사
		return ERROR_PARTY_ADD_PARTYUNION;

	if ( player->IsRequestRejection( ) == true )		// 응답가능 상태 검사
		return ERROR_PARTY_ADD_STATUS;

	if ( player->GetGameIn( ) == false )				// 맵 이동 검사
		return ERROR_PARTY_ADD_MAP;

	if ( gameOption->option1.rejectionParty )		// 옵션 - 파티거부 검사
		return ERROR_PARTY_ADD_OPTIONREFUSE;

	// 초대받은 유저 상태 검사
	switch ( partyState )
	{
	case ePARTY_COMPLETE:
		return ERROR_PARTY_ADD_COMPLETE;
	case ePARTY_RECVJOIN:
	case ePARTY_SENDJOIN:
		return ERROR_PARTY_ADD_STATUS;
	}

	return ERROR_PARTY_ADD_SUCCESS;
}

// AddParty Method
cParty*	cPartyManager::AddParty(unsigned long partyIndex, unsigned long leaderIdx, unsigned long playerIdx)
{
	// 파티 생성
	cParty* pParty = new cParty;
	if ( pParty != NULL )
	{
		// 파티 기본정보 세팅
		pParty->Init( partyIndex, leaderIdx, PARTY_DIVIDE_TURN, NETWORK2->GetAccumTime( ) + PARTY_UPDATETIME );

		// 파티원 세팅
		pParty->AddUser( leaderIdx );
		pParty->AddUser( playerIdx );

		// 파티 삽입 
		if ( mPartyMap.Insert( partyIndex, pParty ) == false )
		{
			SAFE_DELETE( pParty );
			NETWORK2->PostServerEvent( "Error cPartyManager::AddParty : failed to insert party map" );
		}
	}
	else
		NETWORK2->PostServerEvent( "Error cPartyManager::AddParty : failed to create party" );

	return pParty;
}

// DelParty Method
void cPartyManager::DelParty(cParty** party)
{
	mPartyMap.Erase( (*party)->GetIndex( ) );
	SAFE_DELETE( (*party) );
}

// AddRestore Method
void cPartyManager::AddRestore(unsigned long playerIdx, unsigned int partyIdx)
{
	cJoinMap::cIterator i = mRestore.Find( playerIdx );
	if ( i == mRestore.End( ) )
	{
		mRestore.Insert( playerIdx, partyIdx );
	}
}

// SetRestore Method
void cPartyManager::SetRestore(cPlayer* player)
{
	cJoinMap::cIterator i = mRestore.Find( player->GetObjectID( ) );
	if ( i != mRestore.End( ) )
	{
		unsigned long partyIdx = (unsigned long)(*i).mSecond;
		cParty*       party    = GetParty( partyIdx );
		if ( party != NULL )
		{
			if ( party->IsUser( player->GetObjectID( ) ) == true )
			{
				party->SetUserData( player, 0 );
				player->SetPartyIndex( partyIdx );
				player->SetPartyState( ePARTY_COMPLETE );
			}
		}
		DelRestore( player->GetObjectID( ) );
	}
}

// DelRestore Method
void cPartyManager::DelRestore(unsigned long playerIdx)
{
	mRestore.Erase( playerIdx );
}

// GetParty Method
cParty*	cPartyManager::GetParty(unsigned int partyIdx)
{
	return ( partyIdx > 0 ) ? (cParty*)mPartyMap.GetAt( partyIdx ) : NULL;
}

// SearchJoin Method
PartyJoin* cPartyManager::SearchJoin(wchar_t* name)
{
	return mpPartyJoinPool->Search( name );
}

// GetJoin Method
PartyJoin* cPartyManager::GetJoin(wchar_t* name)
{
	PartyJoin* join = mpPartyJoinPool->Search( name );
	if ( join != NULL )
	{
		mpPartyJoinPool->Release( join );
	}
	join = mpPartyJoinPool->Get( name );
	return join;
}

// ReleaseJoin Method
void cPartyManager::ReleaseJoin(PartyJoin* join)
{
	mpPartyJoinPool->Release( join );
}

// ClearRequest Method - 맵아웃(신청받는유저) 자동 거절 처리한다
void cPartyManager::ClearRequest(cPlayer* player)
{
	ePLAYER_PARTYSTATE state = player->GetPartyState( );

	// 파티초대를 받는중이었다면
	if ( state == ePARTY_RECVJOIN )
	{
		// 신청한 유저에게 거절메세지 보내기
		PartyJoin* join = mpPartyJoinPool->Search( player->GetName( ) );
		if ( join != NULL )
		{
			cPlayer* requester = OBJECTMANAGER->GetPlayer( join->playerDat.userIndex );
			if ( requester != NULL )
			{
				ePLAYER_PARTYSTATE state = (requester->GetPartyIndex( ) > 0) ? ePARTY_COMPLETE : ePARTY_NONE;
				requester->SetPartyState( state );

				NETWORK2->SendMsgError( requester->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_REFUSE );
			}
			else
			{
				PostRepParty( join->playerDat.userIndex, join->name, ERROR_PARTY_ADD_REFUSE, 0 );
			}

			// 맵체인지한 유저의 상태 변경
			player->SetPartyState( ePARTY_NONE );
			player->EndRequestRejection( eREQREJCT_PARTY );

			mpPartyJoinPool->Release( join );	// 리퀘스트 삭제
		}
	} 
}

void cPartyManager::PostSynParty(cParty* party)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_PARTY;
	sync.synParty.partyIdx  = party->GetIndex( );
	sync.synParty.leaderIdx = party->GetLeader( );

	unsigned long*  userArr = party->GetUserArr( );
	sPartyUserData* userDat = party->GetUserData( );
	cPlayer*        player;

	for ( unsigned long i = 0; i < party->GetCount( ); ++i, ++userArr, ++userDat )
	{
		player = OBJECTMANAGER->GetPlayer( (*userArr) );
		(player != NULL) ? player->GetPartyUserData( &sync.synParty.player[ i ] ) : sync.synParty.player[ i ] = (*userDat);
	}

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNPARTY( )  );
}

void cPartyManager::PostAskParty(unsigned long partyIdx, cPlayer* player, wchar_t* playerName)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_ASK_PARTY;
	sync.askParty.partyIdx = partyIdx;
	player->GetPartyUserData( &sync.askParty.leader );
	wcscpy( sync.askParty.playerName, playerName );

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.ASKPARTY( )  );
}

void cPartyManager::PostRepParty(unsigned long leaderIdx, wchar_t* playerName, unsigned long error, unsigned char replay)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_REP_PARTY;
	sync.repParty.leaderIdx = leaderIdx;
	wcscpy( sync.repParty.playerName, playerName );
	sync.repParty.error     = error;
	sync.repParty.replay    = replay;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.REPPARTY( )  );
}

void cPartyManager::PostAddParty(unsigned long partyIdx, sPartyUserData* leader, sPartyUserData* player)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_ADD_PARTY;
	sync.addParty.partyIdx  = partyIdx;
	sync.addParty.leader = (*leader);
	sync.addParty.player = (*player);

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.ADDPARTY( )  );
}

void cPartyManager::PostDelParty(unsigned long partyIdx)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_DEL_PARTY;
	sync.delParty.partyIdx = partyIdx;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.DELPARTY( )  );
}

void cPartyManager::PostAddUser(unsigned long partyIdx, cPlayer* player)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_ADD_USER;
	sync.addUser.partyIdx  = partyIdx;
	player->GetPartyUserData( &sync.addUser.player );

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.ADDUSER( )  );
}

void cPartyManager::PostDelUser(unsigned long partyIdx, unsigned long leaderIdx, unsigned long playerIdx)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_DEL_USER;
	sync.delUser.partyIdx  = partyIdx;
	sync.delUser.leaderIdx = leaderIdx;
	sync.delUser.playerIdx = playerIdx;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.DELUSER( )  );
}

void cPartyManager::PostOutUser(unsigned long partyIdx, unsigned long leaderIdx, unsigned long playerIdx)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_OUT_USER;
	sync.outUser.partyIdx  = partyIdx;
	sync.outUser.leaderIdx = leaderIdx;
	sync.outUser.playerIdx = playerIdx;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.OUTUSER( )  );
}

void cPartyManager::PostSynLeader(cParty* party)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_LEADER;
	sync.synLeader.partyIdx  = party->GetIndex( );
	sync.synLeader.leaderIdx = party->GetLeader( );

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNLEADER( )  );
}

void cPartyManager::PostSynDivideType(cParty* party)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_DIVIDE_TYPE;
	sync.synDivideType.partyIdx   = party->GetIndex( );
	sync.synDivideType.divideType = (unsigned char)party->GetDivideType( );

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNDIVIDETYPE( )  );
}

//void cPartyManager::PostSynLevelData(unsigned long partyIdx, cPlayer* player)
//{
//	cSender* sender = g_gameSrv->GetSender( );
//
//	PARTY_DATA_SYNC sync;
//	memset( &sync, 0, sizeof(sync) );
//
//	sync.protocol = PSP_SYN_LEVEL_DATA;
//	sync.synLevelData.partyIdx = partyIdx;
//	player->GetPartyUserData( &sync.synLevelData.player );
//
//	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNLEVELDATA( )  );
//}

void cPartyManager::PostSynMapData(unsigned long partyIdx, cPlayer* player)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_MAP_DATA;
	sync.synMapData.partyIdx = partyIdx;
	player->GetPartyUserData( &sync.synMapData.player );

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNMAPDATA( )  );
}

// PostSynUser Method
void cPartyManager::PostSynUser(unsigned long partyIdx, unsigned long playerIdx, long gotoCID)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_USER;
	sync.synUser.gotoCID   = gotoCID;
	sync.synUser.partyIdx  = partyIdx;
	sync.synUser.playerIdx = playerIdx;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNUSER( )  );
}

// PostSynDiceStart Method
void cPartyManager::PostSynDiceStart(unsigned long partyIdx)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_DICE_START;
	sync.synDiceStart.partyIdx = partyIdx;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNDICESTART( ) );
}

void cPartyManager::PostSynDiceResult(unsigned long partyIdx, unsigned long playerIdx, unsigned short value)
{
	cSender* sender = g_gameSrv->GetSender( );

	PARTY_DATA_SYNC sync;
	memset( &sync, 0, sizeof(sync) );

	sync.protocol = PSP_SYN_DICE_RESULT;
	sync.synDiceResult.partyIdx  = partyIdx;
	sync.synDiceResult.playerIdx = playerIdx;
	sync.synDiceResult.value     = value;

	sender->PostPartySync( NETWORK2->GetCID( ), (char*)&sync, sync.SYNDICERESULT( ) );
}
