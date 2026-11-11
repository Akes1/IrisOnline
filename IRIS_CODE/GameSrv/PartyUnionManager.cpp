#include "stdafx.h"
#include "gamesrv.h"
#include "PartyUnionManager.h"
#include "Player.h"
#include "ObjectManager.h"
#include "GridManager.h"
#include "InfluenceObject.h"
#include "SkillManager.h"

cPartyUnionManager* cPartyUnionManager::mpSingleton = 0;

cPartyUnionManager::cPartyUnionManager()
{
	mpSingleton = this;
}

cPartyUnionManager::~cPartyUnionManager()
{
	Release();
	mpSingleton = 0;
}

bool cPartyUnionManager::Init()
{
	mPartyPool.Reserve( 100, 10 );
	return true;
}

void cPartyUnionManager::Release()
{
	cPartyUnionMap::cIterator i = mPartyUnionMap.Begin();
	cPartyUnionMap::cIterator end = mPartyUnionMap.End();
	for( ; i != end; ++i )
	{
		cPartyUnion* p = (cPartyUnion*)(*i).mSecond;
		SAFE_DELETE( p );
	}
	mPartyUnionMap.Clear();
}

void cPartyUnionManager::Process( unsigned long, unsigned long accumTime )
{
	if( mPartyUnionMap.IsEmpty() )
		return;

	/// 일정시간마다 파티의 정보를 send
	cPartyUnionMap::cIterator i = mPartyUnionMap.Begin();
	cPartyUnionMap::cIterator end = mPartyUnionMap.End();
	for( ; i != end; ++i )
	{
		cPartyUnion* p = (cPartyUnion*)(i->mSecond);
		if( p && accumTime - p->GetLastAccumTime() > PARTYUNION_UPDATETIME )
		{
			// 전체 갱신
			SendUnionUpdate( p );

			// 그룹 갱신
			SendUnionGroupUpdate( p );

			p->SetLastAccumTime( accumTime );
		}
	}
}

// 연합 생성
unsigned long cPartyUnionManager::CreateUnion()
{
	cPartyUnion* p = new cPartyUnion;
	if( !p )
	{
		SAFE_DELETE( p );
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::CreateUnion : failed to create union" );
		assert(0);
		return 0;
	}

	unsigned long index = mIndexGen.GeneratIdx();

	if( mPartyUnionMap.Insert( index, p ) == false )
	{
		SAFE_DELETE( p );
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddParty : failed to insert union map" );
		assert(0);
		return 0;
	}
	
	p->SetIndex( index );
	return index;
}

bool cPartyUnionManager::DeleteUnion( unsigned long index )
{	
	cPartyUnion* p = GetUnion( index );
	if( p )
	{
		if( p->GetCount() > 0 )
			return false;

		// 삭제
		mPartyUnionMap.Erase( index );
		mIndexGen.DelIdx( index );
		SAFE_DELETE( p );
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::DeleteUnion : failed to find union(%d)", index );
		return false;
	}
	return true;
}

cPartyUnion* cPartyUnionManager::GetUnion( unsigned long index )
{
	return (cPartyUnion*)mPartyUnionMap.GetAt( index );
}

bool cPartyUnionManager::AddUser( unsigned long index, unsigned long userIdx )
{
	cPlayer* newUser = OBJECTMANAGER->GetPlayer( userIdx );
	if( newUser != NULL )
	{
	/*	cPartyUnion* p = GetUnion( index );
		if( !p )
		{
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : failed to find union(%d)", index );
			assert(0);
			return false;
		}*/

		// 이미 파티나 연합에 가입중인지 검사
		if( newUser->GetPartyIndex() > 0 )
		{
			//DeleteUnion( index );
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : already party(%d, %d)", index, newUser->GetPartyIndex() );
			return false;
		}

		if( newUser->GetPartyUnionIndex() > 0 )
		{
			DeleteUnion( index );		// 인원을 보고 삭제하므로 상관없다
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : already party union(%d, %d)", index, newUser->GetPartyUnionIndex() );
			return false;
		}

		cPartyUnion* p = GetUnion( index );
		if( !p )
		{
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : failed to find union(%d)", index );
			assert(0);
			return false;
		}

		int count = p->GetCount();
		if( count >= MAX_PARTYUNION )
		{
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : count(%d)", count );
			assert(0);
			return false;
		}

		// 한명도 없으면 추가되는 유저가 리더
		bool isLeader = ( count == 0 ) ? true : false;

		int arrIdx = p->AddUser( userIdx );
		if( arrIdx == -1 )
		{
			DeleteUnion( index );

			NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : failed to insert user(%d,%d)", index, userIdx );
			assert(0);
			return false;
		}

		if( isLeader )
			p->SetLeader( userIdx );

		// 해당 유저에 파티연합 가입중 정보 세팅
		newUser->SetPartyUnionIndex( index );

		// 추가된 유저에게는 연합리스트 
		SendUnionList( index, userIdx );

		// 기존 유저들에게는 추가된 유저 정보
		SendUnionAddOne( index, userIdx, arrIdx );
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::AddUser : failed to find user(%d)", userIdx );
		assert(0);
		return false;
	}
	return true;
}

bool cPartyUnionManager::DeleteUser( cPlayer* player )
{
	if( player == NULL )
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::DeleteUser : player is null" );
		return false;
	}

	unsigned long unionIndex = player->GetPartyUnionIndex();
	unsigned long playerIndex = player->GetObjectID();
	
	cPartyUnion* p = GetUnion( unionIndex );
	if( !p )
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::DeleteUser : failed to find union(%d)", unionIndex );
		assert(0);
		return false;
	}

	unsigned long leaderIndex = p->GetLeader();

	// 해당 유저 정보 삭제
	if( p->DeleteUser( playerIndex ) == false )
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::DeleteUser : failed to delete player(%d,%d)", playerIndex, unionIndex );
		assert(0);
		return false;
	}

	// 해당 유저에게 탈퇴됨을 알림
	NETWORK2->SendMsgError( player->GetConnectionIdx(), NM_PARTYUNION, NM_PARTYUNION_OUT_RES, ERROR_PARTYUNION_OUT_SUCESS );
	
	// 기존 유저들에게 탈퇴 알림
	SendUnionOutSyn( unionIndex, playerIndex );

	// 탈퇴 유저가 연합장인 경우 연합장 위임
	if( leaderIndex == playerIndex )
	{
		unsigned long findNextLeader = p->FindNextLeader();
		if( findNextLeader != -1 )
		{
			// 새로운 연합장 위임
			ChangeLeader( leaderIndex, findNextLeader );
		}
		else
		{
			p->SetLeader( 0 );
		}
	}

	// 정보 초기화
	player->SetPartyUnionIndex( 0 );

	if( p->GetCount() == 0 )
	{
		DeleteUnion( unionIndex );
	}
	return true;
}

int cPartyUnionManager::MoveUser( unsigned long leaderIdx, unsigned char i0, unsigned char i1 )
{
	cPlayer* leader = OBJECTMANAGER->GetPlayer( leaderIdx );
	if( leader != NULL )
	{
		// 그룹 검색
		unsigned long index = leader->GetPartyUnionIndex();
		cPartyUnion* p = GetUnion( index );
		if( !p )
		{
			assert(0);
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::MoveUser : failed to find union(%d)", index );
			return ERROR_PARTYUNION_MOVEGROUP_FAIL;
		}

		// 리더 검사
		if( leaderIdx != p->GetLeader() )
			return ERROR_PARTYUNION_MOVEGROUP_LEADER;

		// 
		if( p->MoveUser( i0, i1 ) == false )
		{
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::MoveUser : failed to move user(%d,%d,%d)", leaderIdx, i0, i1 );
			return ERROR_PARTYUNION_MOVEGROUP_FAIL;
		}

		// 기존 유저들에게 그룹변경 알림
		SendUnionMoveUser( index, i0, i1 );
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::MoveUser : failed to find leader(%d)", leaderIdx );
		assert(0);
		return ERROR_PARTYUNION_MOVEGROUP_FAIL;
	}
	return ERROR_PARTYUNION_MOVEGROUP_SUCESS;
}

// 연합장 위임
int cPartyUnionManager::ChangeLeader( unsigned long leaderIdx, unsigned long userIdx )
{
	cPlayer* leader = OBJECTMANAGER->GetPlayer( leaderIdx );
	cPlayer* user = OBJECTMANAGER->GetPlayer( userIdx );
	if( leader && user )
	{
		unsigned long index = leader->GetPartyUnionIndex();
		cPartyUnion* p = GetUnion( index );
		if( !p )
		{
			assert(0);
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::DeleteUser : failed to find union(%d)", index );
			return ERROR_PARTYUNION_CHANGELEADER_FAIL;
		}

		if( leader->GetPartyUnionIndex() == 0 || user->GetPartyUnionIndex() == 0 )
		{
			assert(0);
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::ChangeLeader : not partyunion", index, leaderIdx, userIdx );
			return ERROR_PARTYUNION_CHANGELEADER_FAIL;
		}

		// 파티연합 검사
		if( leader->GetPartyUnionIndex() != user->GetPartyUnionIndex() )
			return ERROR_PARTYUNION_CHANGELEADER_FAIL;

		// 리더 검사
		if( leaderIdx != p->GetLeader() )
			return ERROR_PARTYUNION_CHANGELEADER_LEADER;

		// 자기 자신인지 검사
		if( leaderIdx == userIdx )
			return ERROR_PARTYUNION_CHANGELEADER_FAIL;

		// 연합장 정보 변경
		p->SetLeader( userIdx );

		// 기존 유저들에게 변경 알림
		SendUnionChangeLeader( index, userIdx );
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::ChangeLeader : failed to change leader(%d,%d)", leaderIdx, userIdx );
		assert(0);
		return ERROR_PARTYUNION_CHANGELEADER_FAIL;
	}

	return ERROR_PARTYUNION_CHANGELEADER_SUCESS;
}

void cPartyUnionManager::MapIn( cPlayer* player )
{
	if( player != NULL )
	{	
		unsigned long unionIndex = player->GetPartyUnionIndex();

		if( unionIndex != 0 )
		{
			SendUnionList( unionIndex, player->GetObjectID() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::MapIn : player is null" );
		assert(0);
	}
}

// 새 user 에게 연합정보 리스트 보내기
void cPartyUnionManager::SendUnionList( unsigned long index, unsigned long userIdx )
{
	cPartyUnion* p = GetUnion( index );
	cPlayer* newUser = OBJECTMANAGER->GetPlayer( userIdx );
	if( newUser && p )
	{
		HANDLE						handle = NULL;
		MSG_SYN_PARTYUNION_LIST*	sendMsg = (MSG_SYN_PARTYUNION_LIST*)NETWORK2->GetMsgRoot( &handle, 
											   newUser->GetConnectionIdx(), NM_PARTYUNION, NM_PARTYUNION_LIST_SYN );
		if( sendMsg != NULL )
		{
			unsigned long			length = sizeof(MSG_SYN_PARTYUNION_LIST) - sizeof(sendMsg->mTable);

			sendMsg->mIndex			= p->GetIndex();
			sendMsg->mLeaderIndex	= p->GetLeader();

			sUnionUserData* data = sendMsg->mTable;
			unsigned long* userArr = p->GetUserArr();
			for( int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
			{
				if( (*userArr) == 0 )
					continue;

				cPlayer* player = OBJECTMANAGER->GetPlayer( (*userArr) );
				if( player != NULL )
				{
					data->idx			= i;
					data->userIndex		= (*userArr);
					wcscpy( data->name,	player->GetName() );
					data->level			= player->GetLevel();
					data->job			= (unsigned short)player->GetJob();
					data->mapNum		= player->GetMapNumber();
					data->channelNum	= NETWORK2->GetChannelNum();
					data->posX			= player->GetXPos();
					data->posY			= player->GetYPos();
					data->hp			= player->GetHP();
					data->maxhp			= player->GetMaxHP();
					data->mp			= player->GetMP();
					data->maxmp			= player->GetMaxMP();

					data++;
					sendMsg->RowCount++;
				}
				else
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionList : not exist user %d", (*userArr) );
					continue;
				}
			}

			length += ( sendMsg->RowCount * sizeof(sendMsg->mTable) );
			NETWORK2->SendMsgRoot( handle, length );
		}
		else
		{
			NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionList : sendMsg is null" );
			assert(0);
		}
	}	
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionList(%d,%d)", index, userIdx );
		assert(0);
	}
}

// 기존 연합 유저들에게 새 user 정보 보내기
void cPartyUnionManager::SendUnionAddOne( unsigned long index, unsigned long userIdx, unsigned int arrIdx )
{
	cPartyUnion* p = GetUnion( index );
	cPlayer* newUser = OBJECTMANAGER->GetPlayer( userIdx );
	if( p && newUser )
	{
		if( p->GetCount() == 0 )
			return;

		cPlayer* player = 0;
		unsigned long*	userArr = p->GetUserArr();
		for( int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
		{
			if( (*userArr) == 0 )
				continue;

			// 새 유저는 제외
			if( userIdx == (*userArr) )
				continue;

			player = OBJECTMANAGER->GetPlayer( (*userArr) );
			if( player != NULL )
			{
				HANDLE						handle = NULL;
				MSG_SYN_PARTYUNION_ADDONE*  sendMsg = (MSG_SYN_PARTYUNION_ADDONE*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(),
																										NM_PARTYUNION, NM_PARTYUNION_ADDONE_SYN );
				if( sendMsg != NULL )
				{
					sendMsg->mTable.idx			= arrIdx;
					sendMsg->mTable.userIndex	= userIdx;
					wcscpy( sendMsg->mTable.name, newUser->GetName() );
					sendMsg->mTable.level		= newUser->GetLevel();
					sendMsg->mTable.job			= (unsigned short)newUser->GetJob();
					sendMsg->mTable.posX		= newUser->GetXPos();
					sendMsg->mTable.posY		= newUser->GetYPos();
					sendMsg->mTable.hp			= newUser->GetHP();
					sendMsg->mTable.maxhp		= newUser->GetMaxHP();
					sendMsg->mTable.mp			= newUser->GetMP();
					sendMsg->mTable.maxmp		= newUser->GetMaxMP();
					sendMsg->mTable.mapNum		= newUser->GetMapNumber();
					sendMsg->mTable.channelNum	= NETWORK2->GetChannelNum();

					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYUNION_ADDONE) );
				}
				else
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionAddOne : sendMsg is null" );
					assert(0);
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionAddOne : not exist user %d", (*userArr) );
				assert(0);
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionAddOne(%d,%d)", index, userIdx );
		assert(0);
	}
}

void cPartyUnionManager::SendUnionOutSyn( unsigned long index, unsigned long userIdx )
{
	cPartyUnion* p = GetUnion( index );
	if( p != NULL )
	{
		if( p->GetCount() == 0 )
			return;

		cPlayer* player = 0;
		unsigned long*	userArr = p->GetUserArr();
		for( int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
		{
			if( (*userArr) == 0 )
				continue;

			player = OBJECTMANAGER->GetPlayer( (*userArr) );
			if( player != NULL )
			{
				HANDLE						handle = NULL;
				MSG_SYN_PARTYUNION_OUT*  sendMsg = (MSG_SYN_PARTYUNION_OUT*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(),
												   NM_PARTYUNION, NM_PARTYUNION_OUT_SYN );
				if( sendMsg != NULL )
				{
					sendMsg->mPlayerIndex = userIdx;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYUNION_OUT) );
				}
				else
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionOutSyn : sendMsg is null" );
					assert(0);
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionOutSyn : not exist user %d", (*userArr) );
				assert(0);
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionOutSyn(%d,%d)", index, userIdx );
		assert(0);
	}
}

// 연합장 위임 알림
void cPartyUnionManager::SendUnionChangeLeader( unsigned long index, unsigned long userIdx )
{
	cPartyUnion* p = GetUnion( index );
	if( p != NULL )
	{
		if( p->GetCount() == 0 )
			return;

		cPlayer* player = 0;
		unsigned long*	userArr = p->GetUserArr();
		for( int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
		{
			if( (*userArr) == 0 )
				continue;

			player = OBJECTMANAGER->GetPlayer( (*userArr) );
			if( player != NULL )
			{
				HANDLE								handle = NULL;
				MSG_SYN_PARTYUNION_CHANGELEADER*	sendMsg = (MSG_SYN_PARTYUNION_CHANGELEADER*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(),
																NM_PARTYUNION, NM_PARTYUNION_CHANGELEADER_SYN );
				if( sendMsg != NULL )
				{
					sendMsg->mPlayerIndex = userIdx;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYUNION_CHANGELEADER) );
				}
				else
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionChangeLeader : sendMsg is null" );
					assert(0);
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionChangeLeader : not exist user %d", (*userArr) );
				assert(0);
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionChangeLeader(%d,%d)", index, userIdx );
		assert(0);
	}
}

// 그룹 이동 알림
void cPartyUnionManager::SendUnionMoveUser( unsigned long index, unsigned char i0, unsigned char i1 )
{
	cPartyUnion* p = GetUnion( index );
	if( p != NULL )
	{
		if( p->GetCount() == 0 )
			return;

		cPlayer* player = 0;
		unsigned long*	userArr = p->GetUserArr();
		for( int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
		{
			if( (*userArr) == 0 )
				continue;

			player = OBJECTMANAGER->GetPlayer( (*userArr) );
			if( player != NULL )
			{
				HANDLE								handle = NULL;
				MSG_SYN_PARTYUNION_MOVEGROUP*	sendMsg = (MSG_SYN_PARTYUNION_MOVEGROUP*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(),
														   NM_PARTYUNION, NM_PARTYUNION_MOVEGROUP_SYN );
				if( sendMsg != NULL )
				{	
					sendMsg->mIndex0 = i0;
					sendMsg->mIndex1 = i1;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PARTYUNION_MOVEGROUP) );
				}
				else
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionMoveUser : sendMsg is null" );
					assert(0);
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionMoveUser : not exist user %d", (*userArr) );
				assert(0);
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionMoveUser(%d)", index );
		assert(0);
	}
}

// 전체 연합 정보 갱신
void cPartyUnionManager::SendUnionUpdate( cPartyUnion* p )
{
	if( p != NULL )
	{
		if( p->GetCount() == 0 )
			return;

		unsigned long* userArr1 = p->GetUserArr();
		unsigned long* userArr2 = p->GetUserArr();

		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			unsigned long playerIdx1 = *(userArr1+i);
			if( playerIdx1 == 0 )
				continue;

			cPlayer* player1 = OBJECTMANAGER->GetPlayer( playerIdx1 );
			if( player1 == NULL )
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionUpdate( player1 : %d,%d )", p->GetIndex(), playerIdx1 );
				assert(0);
				continue;
			}

			HANDLE						handle = NULL;
			MSG_SYN_PARTYUNION_UPDATE* sendMsg = (MSG_SYN_PARTYUNION_UPDATE*)NETWORK2->GetMsgRoot( &handle, player1->GetConnectionIdx(), 
																								   NM_PARTYUNION, NM_PARTYUNION_UPDATE_SYN );

			if( sendMsg != NULL )
			{
				sendMsg->rowCount = 0;
				
				// 18명 정보를 추림
				sUnionUpdate* table = sendMsg->table;
				for( int j = 0; j < MAX_PARTYUNION; ++j )
				{
					unsigned long playerIdx2 = *(userArr2+j);
					if( playerIdx2 == 0 )
						continue;

					cPlayer* player2 = OBJECTMANAGER->GetPlayer( playerIdx2 );
					if( player2 == NULL )
					{
						NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionUpdate( player2 : %d,%d )", p->GetIndex(), playerIdx2 );
						assert(0);
						continue;
					}

					/// 1. 맵 위치 검사
					if( player1->GetMapNumber() != player2->GetMapNumber() )
						continue;

					/// 2. 그리드 검사 - 근접 그리드안에 없는 유저정보만 추려서 쏜다
					if( GRIDMANAGER->IsSight( player1, player2 ) == false )
					{
						// 정보 담기
						table->idx		 = j;
						table->userIndex = playerIdx2;
						table->maxhp	 = player2->GetMaxHP();
						table->hp		 = player2->GetHP();
						table->posX		 = player2->GetXPos();
						table->posY		 = player2->GetYPos();

						table++;
						sendMsg->rowCount++;
					}
				}

				// 쏘기
				if( sendMsg->rowCount > 0 )
				{	
					NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
				}
				else
				{
					NETWORK2->ReleaseMsgRoot( (PerIoContext*)handle, sendMsg->GetMsgLength() );
				}
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionUpdate - not exist party union" );
		assert(0);
	}
}

// 그룹 정보 갱신
void cPartyUnionManager::SendUnionGroupUpdate( cPartyUnion* p )
{
	if( p != NULL )
	{
		if( p->GetCount() == 0 )
			return;

		typedef tHashSet<unsigned long> cSkillInfluenceSet;

		unsigned long* userArr1 = p->GetUserArr();
		
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			unsigned long playerIdx1 = *(userArr1+i);
			if( playerIdx1 == 0 )
				continue;

			cPlayer* player1 = OBJECTMANAGER->GetPlayer( playerIdx1 );
			if( player1 == NULL )
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionGroupUpdate( player1 : %d,%d )", p->GetIndex(), playerIdx1 );
				assert(0);
				continue;
			}
	
			int groupIndex = p->GetGroup( playerIdx1 );
			if( groupIndex == -1 )
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionGroupUpdate - GetGroup(%d,%d)", p->GetIndex(), playerIdx1 );
				assert(0);
				continue;
			}

			unsigned long* userArr2 = p->GetGroupArr( playerIdx1 );
			if( userArr2 == 0 )
			{
				NETWORK2->PostServerEvent( "Error cPartyUnionManager::GetGroupArr - GetGroupArr(%d,%d)", p->GetIndex(), playerIdx1 );
				assert(0);
				continue;
			}

			for( unsigned int j = 0; j < MAX_PARTYUSER; ++j )
			{
				unsigned long playerIdx2 = *(userArr2+j);
				if( playerIdx2 == 0 )
					continue;

				// 자기자신 정보는 보내지 않는다
				if( playerIdx1 == playerIdx2 )
					continue;

				cPlayer* player2 = OBJECTMANAGER->GetPlayer( playerIdx2 );
				if( player2 == NULL )
				{
					NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionUpdate( player2 : %d,%d )", p->GetIndex(), playerIdx2 );
					assert(0);
					continue;
				}

				/// 1. 맵 위치 검사
				if( player1->GetMapNumber() != player2->GetMapNumber() )
					continue;

				/// 2. 그리드 검사 - 근접 그리드안에 없는 유저정보만 추려서 쏜다
				if( GRIDMANAGER->IsSight( player1, player2 ) == false )
				{
					HANDLE							 handle = NULL;
					MSG_SYN_PARTYUNION_GROUPUPDATE*	sendMsg = (MSG_SYN_PARTYUNION_GROUPUPDATE*)NETWORK2->GetMsgRoot( &handle, player1->GetConnectionIdx(), 
																													 NM_PARTYUNION, NM_PARTYUNION_GROUPUPDATE_SYN );
					if( sendMsg != NULL )
					{
						sendMsg->idx		= (groupIndex * MAX_PARTYUSER) + j;
						sendMsg->userIndex	= playerIdx2;
						sendMsg->level		= player2->GetLevel();
						sendMsg->maxhp		= player2->GetMaxHP();
						sendMsg->hp			= player2->GetHP();
						sendMsg->maxmp		= player2->GetMaxMP();
						sendMsg->mp			= player2->GetMP();
						sendMsg->posX		= player2->GetXPos();
						sendMsg->posY		= player2->GetYPos();
						sendMsg->rowCount	= 0;

						/// 버프리스트 담기
						cSkillInfluenceSet* influenceSet = player2->GetInfluenceSet();
						if( influenceSet && influenceSet->GetSize() > 0  )
						{
							cSkillInfluenceSet::cIterator i = influenceSet->Begin();
							cSkillInfluenceSet::cIterator end = influenceSet->End();

							short& buffCount = sendMsg->rowCount;
							cInfluenceObject* pObj = 0;
							for( ; i != end; ++i )
							{
								pObj = SKILLMANAGER->GetInfluence( (unsigned long)(*i) );
								if( !pObj )
								{
									assert(0);
									NETWORK2->PostServerEvent("cPartyUnionManager::SendUnionGroupUpdate[%d,%d,%d]", NM_PARTYUNION_UPDATE_SYN, player1->GetConnectionIdx(), playerIdx2 );
									continue;
								}

								sendMsg->table[buffCount].mInfluenceClassIdx = pObj->GetInfluenceClassIdx();
								sendMsg->table[buffCount].mUniqueIdx = pObj->GetUniqueIdx();
								sendMsg->table[buffCount].mRestTime = 0;

								buffCount++;
							}
						}

						NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
					}
				}
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "Error cPartyUnionManager::SendUnionGroupUpdate - not exist party union" );
		assert(0);
	}
}

