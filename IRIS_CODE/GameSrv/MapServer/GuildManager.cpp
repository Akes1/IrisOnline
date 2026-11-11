#include "stdafx.h"
#include "gamesrv.h"
#include "GuildManager.h"
#include "Guild.h"
#include "Player.h"
#include "Guild_Common.h"
#include "ObjectManager.h"
#include "ItemManager.h"
#include "AppTimer.h"

cGuildManager* cGuildManager::mpSingleton = NULL;

cGuildManager::cGuildManager()
: mGuildSelectLength(0)
, mGuildTotalLength(0)
, mGuildTotalOffset(0)
, mGuildSaveOffset(0)
, mAccumTime(0)
{
	mpSingleton = this;
}

cGuildManager::~cGuildManager()
{
	Release();
	mpSingleton = 0;
}

void cGuildManager::Release()
{
	cGuildMap::cIterator i = mGuildMap.Begin();
	cGuildMap::cIterator end = mGuildMap.End();
	for( ; i != end; ++i )
	{
		cGuild* p = (cGuild*)(i->mSecond);
		SAFE_DELETE( p );
	}
	mGuildMap.Clear();

	//
	mJoinMap.Clear();
}

bool cGuildManager::Init()
{
	mAccumTime = NETWORK2->GetAccumTime();
	return true;
}

cGuild* cGuildManager::GetGuild( unsigned long idx )
{
	return (cGuild*)mGuildMap.GetAt( idx );
}

wchar_t* cGuildManager::GetGuildName( unsigned long idx )
{
	cGuild* guild = GetGuild( idx );
	if ( guild != NULL )
		return guild->GetGuildName();
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::GetGuildName[%d], GuildCount[%d]", idx, mGuildMap.GetSize() );
	}
	return 0;
}

void cGuildManager::GetGuildMarkDate( unsigned long idx, tm& registDate, bool& useMark )
{
	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{	
		registDate = TIMER->ConvertTStoTM( guild->GetGuildMarkDate() );

		// 초기값 판단
		time_t time = mktime( &registDate );
		useMark = ( time > -1 ) ? true : false;
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::GetGuildMarkDate[%d], GuildCount[%d]", idx, mGuildMap.GetSize() );
	}
}

/// select 수 확인
bool cGuildManager::CheckGuildLength()
{
	return ( mGuildSelectLength == mGuildMap.GetSize() );
}

bool cGuildManager::CheckGuildTotal()
{
	mGuildTotalOffset++;
	return ( mGuildTotalLength == mGuildTotalOffset );
}

/// 서버종료시 업데이트 검사
bool cGuildManager::CheckGuildSaveTotal()
{
	mGuildSaveOffset++;
	return ( mGuildMap.GetSize() == mGuildSaveOffset );
}

/// 길드 정보 추가
bool cGuildManager::VerifyGuildSelect( TB_GUILD* guild )
{
	cGuild* p = new cGuild;

	if( guild->markDate.year == 0 )
	{
		struct tm initTm;
		memset( &initTm, 0, sizeof(initTm) );
		initTm.tm_year = 1900;
		initTm.tm_mon  = 1;
		initTm.tm_mday = 1;

		guild->markDate = TIMER->ConvertTMtoTS( initTm );
	}

	p->VerifySetInfo( guild );

	return mGuildMap.Insert( guild->idx, p );
}

/// 길드 유저 정보 세팅
bool cGuildManager::VerifyGuildUser( unsigned long idx, TB_GUILD_USER* user )
{
	cGuild* guild = GetGuild( idx );
	return (guild != NULL) ? guild->VerifyAddPlayer( user ) : NULL;
}

/// 길드명 중복검사
bool cGuildManager::FindGuildName( wchar_t* name )
{
	cGuildMap::cIterator i = mGuildMap.Begin();
	cGuildMap::cIterator end = mGuildMap.End();
	for( ; i != end; ++i )
	{
		cGuild* p = (cGuild*)(i->mSecond);
		if ( p != NULL )
		{
			if ( wcsicmp( p->GetGuildName(), name ) == 0 )
				return true;
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::FindGuildName[%s], GuildCount[%d]", name, mGuildMap.GetSize() );
			continue;
		}
	}
	return false;
}

/// 프로세스
void cGuildManager::Process( unsigned long, unsigned long accumTime )
{
	if( mGuildMap.GetSize() == 0 )
		return;

	cGuildMap::cIterator i = mGuildMap.Begin();
	cGuildMap::cIterator end = mGuildMap.End();
	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)(i->mFirst);
		cGuild* p = (cGuild*)(i->mSecond);
		if ( p != NULL )
		{
			// 클라이언트 갱신 시간 계산
			if( accumTime - p->GetAccumTime() > GUILD_UPDATETIME )
			{
				// 갱신 & 쏘기
				SendGuildUpdateInfo( p );

				// TTL 전송 : 다른 채널에 길드원 접속 유지정보 보냄
				PostGuildTTL( p );

				p->SetAccumTime( accumTime );
			}

			// DB 갱신 시간 계산
			if( accumTime - p->GetDBAccumTime() > GUILDDB_UPDATETIME )
			{
				SaveDBGuild( p );
				p->SetDBAccumTime( accumTime );
			}

			// TTL 체크 ( 접속 정보 갱신 안된 유저 처리 )
			unsigned long userIndex = p->GetLostPlayer( GetTickCount() );
			if( userIndex > 0 )
			{
				// 종료 처리
				SendGuildUserConnect( idx, userIndex, 0, 0, true );
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::Process" );
			continue;
		}
	}
}

/// 길드 생성
bool cGuildManager::AddGuild( GUILD_CREATE* data, cPlayer* master )
{
	cGuild* p = new cGuild( );

	struct tm initTm;
	memset( &initTm, 0, sizeof(initTm) );
	initTm.tm_year = 1900;
	initTm.tm_mon  = 1;
	initTm.tm_mday = 1;

	TIMESTAMP_STRUCT timeStamp = TIMER->ConvertTMtoTS( initTm );

	p->SetInfo( data->idx, data->characterIdx, data->name, data->masterName, timeStamp );
	p->AddPlayer( master, eGUILD_MASTER, NETWORK2->GetChannelNum() );
	p->SetMaster( master->GetObjectID(), master->GetName() );

	cGuild* guild = (cGuild*)mGuildMap.GetAt( data->idx );
	if( guild )
	{
		NETWORK2->PostServerEvent("cGuild::AddGuild[%d], GuildCount[%d]", data->idx, mGuildMap.GetSize() );
		return false;
	}

	return mGuildMap.Insert( data->idx, p );
}

/// 길드 삭제
bool cGuildManager::DeleteGuild( unsigned long idx )
{
	cGuildMap::cIterator i = mGuildMap.Find( idx );
	if( i != mGuildMap.End() )
	{
		cGuild* p = (cGuild*)(i->mSecond);
		SAFE_DELETE( p );
		mGuildMap.Erase(i);
	}
	else
	{
		NETWORK2->PostServerEvent("cGuild::DeleteGuild[%d], GuildCount[%d]", idx, mGuildMap.GetSize() );
		return false;
	}
	return true;
}

/// 신청정보 저장
bool cGuildManager::AddRequest( unsigned long recvIdx, unsigned long reqIdx )
{
	return ( mJoinMap.Insert( recvIdx, reqIdx ) );
}

void cGuildManager::DelRequest( unsigned long recvIdx )
{
	mJoinMap.Erase( recvIdx );
}

unsigned long cGuildManager::GetRequest( unsigned long recvIdx )
{
	cJoinMap::cIterator i = mJoinMap.Find( recvIdx );
	return ( i != mJoinMap.End() ) ? (unsigned long)(*i).mSecond : 0;
}

/// player 에게 길드정보 보내기
void cGuildManager::SendGuildInfo( ULONG_PTR socketContext, cPlayer* player, unsigned long idx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( player == NULL )
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo player == NULL");
		return;
	}

	cGuild* guild = GetGuild( idx );
	if ( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			HANDLE				handle = NULL;
			MSG_SYN_GUILD_LIST*	sendMsg = (MSG_SYN_GUILD_LIST*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_GUILD, NM_GUILD_LIST_SYN );
			if( sendMsg != NULL )
			{
				unsigned long length  = sizeof(MSG_SYN_GUILD_LIST) - sizeof(sendMsg->table);

				TB_GUILD* info = guild->GetGuildInfo();

				/// 1. 길드 정보 복사
				sendMsg->data.idx			= info->idx;
				sendMsg->data.masterIndex	= info->masterIndex;
				sendMsg->data.position		= player->GetGuildPosition();

				GetGuildMarkDate( info->idx, sendMsg->data.markRegistDate, sendMsg->data.useMark );

				sendMsg->rowCount			= 0;

				wcscpy( sendMsg->data.masterName, info->masterName );
				wcscpy( sendMsg->data.name, info->name );
				wcscpy( sendMsg->data.notice, info->notice );

				sGuildUserInfo* user = sendMsg->table;
				cGuildUserMap::cIterator i = mUserMap->Begin();
				cGuildUserMap::cIterator end = mUserMap->End();
				for( ; i != end; ++i, ++user )
				{
					TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
					if( p == NULL )
					{
						NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo p == NULL");
						continue;
					}

					user->characterIdx	= p->characterIdx;
					user->job			= (unsigned short)p->job;
					user->level			= p->level;
					user->mapnum		= p->mapnum;
					user->position		= p->position;
					user->channelNum	= p->channelNum;
					wcscpy( user->name, p->name );

					sendMsg->rowCount++;
				}

				length += (sendMsg->rowCount * sizeof(sendMsg->table));
				NETWORK2->SendMsgRoot( handle, length );
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo - Guild User Map EMPTY [%d]", idx );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo1[%d,%d], GuildCount[%d]", player->GetObjectID(), idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
									mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
}

//
void cGuildManager::SendGuildInfo( cPlayer* player, unsigned long idx )
{
	if( player == NULL )
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo player == NULL");
		return;
	}

	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if ( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			HANDLE				handle = NULL;
			MSG_SYN_GUILD_LIST*	sendMsg = (MSG_SYN_GUILD_LIST*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), NM_GUILD, NM_GUILD_LIST_SYN );
			if( sendMsg != NULL )
			{
				unsigned long length  = sizeof(MSG_SYN_GUILD_LIST) - sizeof(sendMsg->table);

				TB_GUILD* info = guild->GetGuildInfo();

				/// 1. 길드 정보 복사
				sendMsg->data.idx			= info->idx;
				sendMsg->data.masterIndex	= info->masterIndex;
				sendMsg->data.position		= player->GetGuildPosition();
				GetGuildMarkDate( info->idx, sendMsg->data.markRegistDate, sendMsg->data.useMark );

				sendMsg->rowCount			= 0;

				wcscpy( sendMsg->data.masterName, info->masterName );
				wcscpy( sendMsg->data.name, info->name );
				wcscpy( sendMsg->data.notice, info->notice );

				sGuildUserInfo* user = sendMsg->table;
				cGuildUserMap::cIterator i = mUserMap->Begin();
				cGuildUserMap::cIterator end = mUserMap->End();
				for( ; i != end; ++i, ++user )
				{
					TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
					if( p == NULL )
					{
						NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo p == NULL" );
						continue;
					}

					user->characterIdx	= p->characterIdx;
					user->job			= (unsigned short)p->job;
					user->level			= p->level;
					user->mapnum		= p->mapnum;
					user->position		= p->position;
					user->channelNum	= p->channelNum;
					wcscpy( user->name, p->name );

					sendMsg->rowCount++;
				}

				length += (sendMsg->rowCount * sizeof(sendMsg->table));
				NETWORK2->SendMsgRoot( handle, length );
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo - Guild User Map EMPTY [%d]", idx );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildInfo2[%d,%d], GuildCount[%d]", player->GetObjectID(), idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("2: mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
									mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
}

// 현재 서버에 접속해있는 길드원들에게 정보 복구를 보낸다
void cGuildManager::SendGuildRestore( unsigned long idx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildRestore p == NULL");
					continue;
				}

				cPlayer* player = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( player )
				{
					SendGuildInfo( player, idx );
				}
			}
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildRestore[%d], GuildCount[%d]", idx, mGuildMap.GetSize() );
	}
}

// 길드에게 길드원 추가 정보 보내기
void cGuildManager::SendGuildUserAdd( cGuild* guild, long characterIdx, char level, long job, short mapnum, wchar_t* name, char position, DWORD channelNum )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		/// 길드 유저에게 보내주기
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildUserAdd p == NULL");
					continue;
				}

				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user && user->GetObjectID() != (unsigned long)characterIdx )
				{
					/// 접속해 있는 유저면
					HANDLE	handle = NULL;
					MSG_SYN_GUILD_USERADD* sendMsg = (MSG_SYN_GUILD_USERADD*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_USERADD_SYN );
					if( sendMsg != NULL )
					{
						sGuildUserInfo* info = &sendMsg->user;
						info->characterIdx  = characterIdx;
						info->job			= (unsigned short)job;
						info->level			= level;
						info->mapnum		= mapnum;
						info->position		= position;
						info->channelNum	= channelNum;
						wcscpy( info->name, name );

						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_GUILD_USERADD) );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUserAdd - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUserAdd[%d]", characterIdx );
	}
}

/// 길드에게 길드원 탈퇴 정보 보내주기
void cGuildManager::SendGuildUserOut( cGuild* guild, unsigned long characterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildUserOut p == NULL");
					continue;
				}

				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
				{
					/// 접속해 있는 유저면
					HANDLE	handle = NULL;
					MSG_SYN_GUILD_OUT* sendMsg = (MSG_SYN_GUILD_OUT*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_OUT_SYN );
					if( sendMsg != NULL )
					{
						sendMsg->characterIdx = characterIdx;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_GUILD_OUT) );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUserOut - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUserOut[%d]", characterIdx );
	}
}

/// 길드에 추방 유저 정보 보내주기
void cGuildManager::SendGuildUserCut( cGuild* guild, unsigned long characterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildUserCut p == NULL");
					continue;
				}
				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
				{
					/// 접속해 있는 유저면
					HANDLE	handle = NULL;
					MSG_SYN_GUILD_USERCUT* sendMsg = (MSG_SYN_GUILD_USERCUT*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_USERCUT_SYN );
					if( sendMsg != NULL )
					{
						sendMsg->characterIdx = characterIdx;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_GUILD_USERCUT) );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUserCut - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUserCut[%d]", characterIdx );
	}
}

/// 길드직위 변경 및 갱신 정보 보내기
void cGuildManager::SendGuildChangePosition( cGuild* guild, unsigned long characterIdx, char position )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		TB_GUILD_USER* user = IsGuildUser( guild->GetGuildIndex(), characterIdx );
		if( user )
		{
			/// 새 길드장일 경우 기존 길드장 직위변경
			if( position == eGUILD_MASTER )
			{
				/// 기존 길드장 정보 변경 및 새 길드장 세팅
				cPlayer* master = OBJECTMANAGER->GetPlayer( guild->GetGuildMasterIndex() );
				if( master )
					master->ChangeGuildPosition( eGUILD_GENERAL );

				guild->SetMaster( characterIdx, user->name );
			}

			/// 해당 유저의 직위 변경
			user->position = position;
			cPlayer* player = OBJECTMANAGER->GetPlayer( characterIdx );
			if( player )
				player->ChangeGuildPosition( user->position );

			/// 직위 변경 갱신 보내기
			cGuildUserMap* mUserMap = guild->GetGuildUser();
			if( mUserMap != NULL )
			{
				cGuildUserMap::cIterator i = mUserMap->Begin();
				cGuildUserMap::cIterator end = mUserMap->End();
				for( ; i != end; ++i )
				{
					TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
					if( p == NULL )
					{
						NETWORK2->PostServerEvent("cGuildManager::SendGuildChangePosition p == NULL");
						continue;
					}
					cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
					if( user )
					{
						/// 접속해 있는 유저면
						HANDLE	handle = NULL;
						MSG_SYN_GUILD_GIVEPOSITION* sendMsg = (MSG_SYN_GUILD_GIVEPOSITION*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_GIVEPOSITION_SYN );
						if( sendMsg != NULL )
						{
							sendMsg->characterIdx	= characterIdx;
							sendMsg->position		= position;
							NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_GUILD_GIVEPOSITION) );
						}
					}
				}
			}
			else
			{
				NETWORK2->PostServerEvent("cGuildManager::SendGuildChangePosition - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
			}
		}
		else
		{
			/// 에러
			NETWORK2->PostServerEvent("cGuildManager::SendGuildChangePosition[%d, %d, %d]", guild->GetGuildIndex(), characterIdx, position );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildChangePosition" );
	}
}

/// 해당 길드원들에게 갱신된 정보만 보내기
void cGuildManager::SendGuildUpdateInfo( cGuild* guild )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			// 1. 쏠 정보 구성
			unsigned char rowCount = 0;
			sGuildUserUpdate temp[50];
			memset( temp, 0, sizeof(temp) );

			sGuildUserUpdate* tempList = temp;
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				/// 갱신이 필요한 정보만 쏜다
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo p == NULL");
					continue;
				}

				if( p->sendInfo == true )
				{
					p->sendInfo	= false;

					/// 유저가 존재하면 최신정보로 갱신
					cPlayer* guildUser = OBJECTMANAGER->GetPlayer( p->characterIdx );
					if( guildUser != NULL )
					{
						p->level		= guildUser->GetLevel();
						p->job			= guildUser->GetJob();
						p->mapnum		= guildUser->GetMapDataNumber();
					}

					/// 메세지 구성
					tempList->characterIdx	= p->characterIdx;
					tempList->job			= (unsigned short)p->job;
					tempList->level			= p->level;
					tempList->mapnum		= p->mapnum;
					tempList->channelNum	= p->channelNum;

					tempList++;
					rowCount++;
				}
			}

			/// 2. 클라이언트에 쏘기
			if( rowCount > 0 )
			{
				cGuildUserMap::cIterator ui = mUserMap->Begin();
				cGuildUserMap::cIterator uend = mUserMap->End();
				for( ; ui != uend; ++ui )
				{
					TB_GUILD_USER* p = (TB_GUILD_USER*)(ui->mSecond);
					if( p == NULL )
					{
						NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo p == NULL");
						continue;
					}
					cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
					if( user )
					{
						/// 접속해 있는 유저에게
						HANDLE	handle = NULL;
						MSG_SYN_GUILD_UPDATE* sendMsg = (MSG_SYN_GUILD_UPDATE*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_UPDATE_SYN );
						if( sendMsg != NULL )
						{
							unsigned long length = sizeof(MSG_SYN_GUILD_UPDATE) - sizeof(sendMsg->table);
							sendMsg->rowCount	 = rowCount;
							
							sGuildUserUpdate* data = temp;
							sGuildUserUpdate* table = sendMsg->table;
							for( unsigned char i = 0; i < rowCount; ++i, ++data, ++table )
							{
								*table = *data;
							}

							length += (sendMsg->rowCount * sizeof(sendMsg->table));
							NETWORK2->SendMsgRoot( handle, length );
						}
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo" );
	}
}

/// 길드 공지
bool cGuildManager::SendGuildNotice( unsigned long idx, wchar_t* notice )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{
		/// 공지 길이 확인
		int len = (int)wcslen( notice );
		if( len == 0 )
			return false;

		guild->SetNotice( notice );
        
		/// 공지 내용 쏘기
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildNotice p == NULL");
					continue;
				}

				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
				{
					HANDLE		handle = NULL;
					MSG_SYN_GUILD_NOTICE* sendMsg = (MSG_SYN_GUILD_NOTICE*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_GUILD, NM_GUILD_NOTICE_SYN );
					if( sendMsg != NULL )
					{
						wcscpy( sendMsg->notice, notice );
						NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildNotice - Guild User Map EMPTY [%d]", idx );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildNotice[%d]", idx );
		return false;
	}
	return true;
}

void cGuildManager::SendGuildMarkUpdate( cGuild* guild )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( guild != NULL )
	{
		/// 마크 갱신보내기 
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildMarkUpdate p == NULL");
					continue;
				}
				
				/// 각 유저의 마크 정보 변경
				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
					user->ChangeGuildMarkRegistDate();
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildMarkUpdate - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildMarkUpdate" );
	}
}

/// 길드마크 정보 보내주기
bool cGuildManager::SendGuildMark( unsigned long idx, cPlayer* player )
{
	if( player == NULL )
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildMark player == NULL");
		return false;
	}

	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{
		TB_GUILD* info = guild->GetGuildInfo();

		HANDLE		handle = NULL;
		MSG_RES_GUILDMARK_SEND* sendMsg = (MSG_RES_GUILDMARK_SEND*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), NM_GUILD, NM_GUILDMARK_SEND_RES );
		if( sendMsg != NULL )
		{
			sendMsg->guildIndex = idx;
			sendMsg->registDate = TIMER->ConvertTStoTM( info->markDate );
			memcpy( &sendMsg->mark, &info->mark, MARK_SIZE );
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_GUILDMARK_SEND) );
		}
	}
	else
	{
		// 정보 요청 순간에 길드가 삭제 처리 되었을수 있음
		// 그래서 로그 안남김
		return false;
	}
	return true;
}

/// 길드 채팅
void cGuildManager::SendGuildChat( cPlayer* hero, wchar_t* message, u_int length, unsigned short number )
{
	if( hero == NULL )
		return;

	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	if( hero->GetGuildIndex() == 0 )
		return;

	TB_INVENTORY*	inventory = hero->SelectInventory( number );
	cGuild* guild = GetGuild( hero->GetGuildIndex() );
	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SendGuildChat p == NULL");
					continue;
				}
				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
				{
					HANDLE		handle = NULL;
					MSG_SYN_CHAT_GUILD* sendMsg = (MSG_SYN_CHAT_GUILD*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_CHAT, NM_CHAT_GUILD_SYN );
					if( sendMsg != NULL )
					{
						sendMsg->job = (unsigned short)hero->GetJob();

                        wcscpy( sendMsg->Name, hero->GetName() );
						memcpy( sendMsg->Message, message, length );

						if( hero->IsInventory( inventory ) == true )
							Inventory2sInventory( &sendMsg->inventory, inventory );

						NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo - Guild User Map EMPTY [%d]", guild->GetGuildIndex() );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUpdateInfo" );
	}
}

// 길드유저 게임 접속 정보 보내기
void cGuildManager::SendGuildUserConnect( unsigned long idx, unsigned long characterIdx, BYTE connect, DWORD channelNum, bool sendConnect )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	DWORD curChannelNum = NETWORK2->GetChannelNum();

	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			// 정보 저장
			TB_GUILD_USER* player = (TB_GUILD_USER*)mUserMap->GetAt( characterIdx );
			if( player != NULL )
			{
				player->tick = 0;

				if( connect == 1 )
				{
					// 접속
					player->channelNum = channelNum;

					// 동일 채널이 아니면
					if( curChannelNum != channelNum )
					{
						player->tick = GetTickCount() + GUILD_TTL_TICK;
					}
				}
				else
				{
					// 비접속
					player->channelNum = 0;
				}

				player->sendInfo = true;
			}

			// send
			if( sendConnect == true )
			{
				DWORD newChannelNum = ( connect == 1 ) ? channelNum : 0;

				cGuildUserMap::cIterator i = mUserMap->Begin();
				cGuildUserMap::cIterator end = mUserMap->End();
				for( ; i != end; ++i )
				{
					TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
					if( p )
					{
						if( p == player )
							continue;

						// 접속여부 알림
						cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
						if( user != NULL )
							user->SendGuildConnectSyn( characterIdx, connect, newChannelNum );
					}
					else
					{
						NETWORK2->PostServerEvent("cGuildManager::SendGuildUserConnect p == NULL");
						continue;
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUserConnect - Guild User Map EMPTY [%d]", idx );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SendGuildUserConnect(%d,%d)", idx, characterIdx );
	}
}

/// 전체 길드 정보 DB 저장
void cGuildManager::SaveAllDb()
{
	if ( mGuildMap.GetSize() != 0 )
	{
		cGuildMap::cIterator i = mGuildMap.Begin();
		cGuildMap::cIterator end = mGuildMap.End();

		for ( ; i != end; ++i )
		{
			cGuild* p = (cGuild*)(i->mSecond);
			if ( p != NULL )
			{
				SaveDBGuild( p );
			}
			else
				NETWORK2->PostServerEvent("Error - cGuildManager::SaveAllDb( )" );
		}
	}
}

// DB 에 길드 정보 저장
void cGuildManager::SaveDBGuild( cGuild* guild )
{	
	TB_GUILD*		info = guild->GetGuildInfo();

	HANDLE			handle		= NULL;
	GUILD_UPDATE*	guildUpdate = (GUILD_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_GUILD_UPDATE );
	if( guildUpdate != NULL )
	{
		guildUpdate->idx   = info->idx;
		wcscpy( guildUpdate->notice, info->notice );
		NETWORK2->SendSQL( handle, sizeof(GUILD_UPDATE) );
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SaveDBGuild[%d]", info->idx );
	}
}

// player가 받고있는 신청 정보 취소 
// 맵이동시, 게임종료시 처리
void cGuildManager::ClearRequest( cPlayer* player )
{
	if( player != NULL )
	{
		unsigned long reqIdx = GetRequest( player->GetObjectID() );
		if( reqIdx > 0 )
		{
			// 길드초대한 유저가 존재하면 알려준다
			cPlayer* requester = OBJECTMANAGER->GetPlayer( reqIdx );
			if( requester )
			{
				// 신청상태 풀기
				requester->SetGuildAddReq( false );

				// 게임을 나감으로써 자동 거절 처리
				HANDLE              handle = NULL;
				MSG_RES_GUILD_ADD* sendMsg = (MSG_RES_GUILD_ADD*)NETWORK2->GetMsgRoot( &handle, requester->GetConnectionIdx(), NM_GUILD, NM_GUILD_ADD_RES );
				if( sendMsg != NULL )
				{
					sendMsg->ErrorCode = ERROR_GUILD_ADD_REFUSE;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_GUILD_ADD) );
				}
			}	

			/// 
			DelRequest( player->GetObjectID() );

			///
			player->EndRequestRejection( eREQREJCT_GUILD );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::ClearRequest" );
	}
}

// 플레이어 들어옴 : 맵이동
void cGuildManager::PlayerMapIn( cPlayer* hero )
{
	if( hero && hero->GetGuildIndex() > 0 )
	{
		short mapnum = hero->GetMapDataNumber();
		GuildUpdateChannel( hero->GetGuildIndex(), hero->GetObjectID(), NETWORK2->GetChannelNum(), mapnum );

		//
		SendGuildInfo( hero, hero->GetGuildIndex() );
	}
}

bool cGuildManager::GuildUpdateChannel( unsigned long idx, unsigned long characterIdx, DWORD channelNum, short mapnum )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			TB_GUILD_USER* user = (TB_GUILD_USER*)mUserMap->GetAt( characterIdx );
			if( user )
			{
				/// 접속 채널 번호 갱신 ( 0 이면 접속안함 )
				user->channelNum = channelNum;
				user->mapnum	 = mapnum;
				user->sendInfo = true;
			}
			else
				NETWORK2->PostServerEvent("cGuildManager::GuildUpdateChannel - Not exist user [%d,%d]", idx, characterIdx );
		}
		else
			NETWORK2->PostServerEvent("cGuildManager::GuildUpdateChannel - Guild User Map EMPTY [%d]", idx );
	}
	else
		NETWORK2->PostServerEvent("cGuildManager::GuildUpdateChannel[%d]", idx );

	return true;
}

// 
unsigned long cGuildManager::CheckGuildUser( unsigned long characterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuildMap::cIterator i = mGuildMap.Begin();
	cGuildMap::cIterator end = mGuildMap.End();
	for( ; i != end; ++i )
	{
		cGuild* guild = (cGuild*)(*i).mSecond;
		if( guild != NULL )
		{
			cGuildUserMap* mUserMap = guild->GetGuildUser();
			if( mUserMap != NULL )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)mUserMap->GetAt( characterIdx );
				if( p != NULL )
					return guild->GetGuildIndex();
			}
			else
			{
				NETWORK2->PostServerEvent("cGuildManager::CheckGuildUser[%d,%d], GuildCount[%d] - not exist user map", 
										   characterIdx, guild->GetGuildIndex(), mGuildMap.GetSize() );
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::CheckGuildUser[%d], GuildCount[%d] - not exist guild",
									   characterIdx, mGuildMap.GetSize() );
		}
	}
	return 0;
}

// 내 길드원인지 검사
TB_GUILD_USER* cGuildManager::IsGuildUser( unsigned long idx, unsigned long characterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if ( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if ( mUserMap != NULL )
			return (TB_GUILD_USER*)mUserMap->GetAt( characterIdx );
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::IsGuildUser[%d,%d], GuildCount[%d]", characterIdx, idx, mGuildMap.GetSize() );
	}
	return 0;
}

// 직위 변경 가능한지 검사
bool cGuildManager::CheckChangePosition( cPlayer* hero, unsigned long characterIdx, char newposition )
{
	if( hero == NULL )
		return false;

	// 내 길드원인지 검사
	TB_GUILD_USER* user = IsGuildUser( hero->GetGuildIndex(), characterIdx );
	if( user )
	{
		// 권한 검사
		char heroPosition = hero->GetGuildPosition();
		if( heroPosition == eGUILD_NONE )
		{
			NETWORK2->PostServerEvent("cGuildManager::CheckChangePosition[%d,%d,%d,%d,%d]", hero->GetObjectID(), heroPosition, characterIdx, user->position, newposition );
			return false;
		}

		if( !(heroPosition == eGUILD_MASTER || heroPosition == eGUILD_SUBMASTER) )
			return false;

		// 해당 유저의 직위가 자신보다 낮아야함( 상대방이 동일한 길드장일수 없음!! )
		if( heroPosition == eGUILD_MASTER && !(heroPosition < user->position) )
		{
			NETWORK2->PostServerEvent("cGuildManager::CheckChangePosition[%d,%d,%d,%d,%d]", hero->GetObjectID(), heroPosition, characterIdx, user->position, newposition );
			return false;
		}

		// 해당유저의 직책이 자신보다 높거나, 변경직책이 자신보다 높을수 없다
		if( heroPosition == eGUILD_SUBMASTER && !(heroPosition < user->position && heroPosition < newposition) )
			return false;
			
		return true;
	}
	return false;
}

/// 갱신 요청 - 레벨업, 전직
bool cGuildManager::RequestUpdateUser( unsigned long idx, unsigned long characterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuild* guild = GetGuild( idx );
	if( guild )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			TB_GUILD_USER* user = (TB_GUILD_USER*)mUserMap->GetAt( characterIdx );
			if( user )
			{
				/// 갱신 플래그 세우기
				user->sendInfo = true;
			}
			else
				NETWORK2->PostServerEvent("cGuildManager::RequestUpdateUser - Guild User error [%d,%d]", idx, characterIdx );
		}
		else
			NETWORK2->PostServerEvent("cGuildManager::RequestUpdateUser - Guild User Map EMPTY [%d]", idx );
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::RequestUpdateUser[%d], GuildCount[%d]", idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
										mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
	return true;
}

//--------------------------------------------------------
// GAME SYN
bool cGuildManager::SynAddGuild( void* msg, DWORD channelNum )
{
	MSG_SYN_POSTGUILD_CREATE* pmsg = (MSG_SYN_POSTGUILD_CREATE*)msg;

	struct tm initTm;
	memset( &initTm, 0, sizeof(initTm) );
	initTm.tm_year = 1900;
	initTm.tm_mon  = 1;
	initTm.tm_mday = 1;

	TIMESTAMP_STRUCT timeStamp = TIMER->ConvertTMtoTS( initTm );

	cGuild* p = new cGuild();
	p->SetInfo( pmsg->idx, pmsg->masterIndex, pmsg->name, pmsg->masterName, timeStamp );
	p->AddPlayer( pmsg->masterIndex, pmsg->level, pmsg->job, pmsg->mapnum, pmsg->masterName, pmsg->position, channelNum );
	p->SetMaster( pmsg->masterIndex, pmsg->masterName );

	cGuild* guild = (cGuild*)mGuildMap.GetAt( pmsg->idx );
	if( guild )
	{
		NETWORK2->PostServerEvent("cGuild::AddGuildPostSyn[%d], GuildCount[%d]", pmsg->idx, mGuildMap.GetSize() );
		return false;
	}

	return mGuildMap.Insert( pmsg->idx, p );
}

bool cGuildManager::SynAddGuildUser( void* msg, DWORD channelNum )
{
	MSG_SYN_POSTGUILD_USERADD* pmsg = (MSG_SYN_POSTGUILD_USERADD*)msg;

	cGuild* guild = GetGuild( pmsg->idx );
	if( guild )
	{
		if( guild->AddPlayer( pmsg->characterIdx, pmsg->level, pmsg->job, pmsg->mapnum, pmsg->name, pmsg->position, channelNum ) == true )
		{
			SendGuildUserAdd( guild, pmsg->characterIdx, pmsg->level, pmsg->job, pmsg->mapnum, pmsg->name, pmsg->position, channelNum );
		}
		else
		{
			NETWORK2->PostGuildEvent( pmsg->characterIdx, pmsg->idx, pmsg->position, "FAIL - SynAddGuildUser()" );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SynAddGuildUser[%d], GuildCount[%d]", pmsg->idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
									mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
	return true;
}

bool cGuildManager::SynCutGuildUser( void* msg )
{
	MSG_SYN_POSTGUILD_USERCUT* pmsg = (MSG_SYN_POSTGUILD_USERCUT*)msg;

	cGuild* guild = GetGuild( pmsg->idx );
	if( guild )
	{
		if( guild->DeletePlayer( pmsg->characterIdx ) == true )
		{
			/// 추방된 유저에게 알리기
			cPlayer* cutUser = OBJECTMANAGER->GetPlayer( pmsg->characterIdx );
			if( cutUser )
			{
				cutUser->ChangeGuild( 0, eGUILD_NONE );

				// 추방 
				HANDLE			handle = NULL;
				MSG_RES_GUILD_OUT* sendMsg = (MSG_RES_GUILD_OUT*)NETWORK2->GetMsgRoot( &handle, cutUser->GetConnectionIdx(), NM_GUILD, NM_GUILD_CUT_SYN );
				if( sendMsg != NULL )
				{
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_RES_GUILD_OUT) );
				}

				// 길드 신청 정보 있으면 삭제
				cutUser->SetGuildAddReq( false );
			}

			/// 다른유저들에게 알리기
			SendGuildUserCut( guild, pmsg->characterIdx );
		}
		else
		{
			NETWORK2->PostGuildEvent( pmsg->characterIdx, pmsg->idx, eGUILD_NONE, "FAIL - SynCutGuilduser()" );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SynCutGuilduser[%d], GuildCount[%d]", pmsg->idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
										mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
	return true;
}

bool cGuildManager::SynOutGuildUser( void* msg )
{
	MSG_SYN_POSTGUILD_USEROUT* pmsg = (MSG_SYN_POSTGUILD_USEROUT*)msg;

	cGuild* guild = GetGuild( pmsg->idx );
	if( guild )
	{
		if( guild->DeletePlayer( pmsg->characterIdx ) == true )
		{
			// 길드원들에게 쏘기
			SendGuildUserOut( guild, pmsg->characterIdx );
		}
		else
		{
			NETWORK2->PostGuildEvent( pmsg->characterIdx, pmsg->idx, eGUILD_NONE, "FAIL - SynOutGuilduser()" );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SynOutGuilduser[%d], GuildCount[%d]", pmsg->idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
									mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
	return true;
}

bool cGuildManager::SynUpdateMark( void* msg )
{
	MSG_SYN_POSTGUILD_UPDATEMARK* pmsg = (MSG_SYN_POSTGUILD_UPDATEMARK*)msg;

	cGuild* guild = GetGuild( pmsg->idx );
	if( guild )
	{
		// 길드 마크 정보 변경
		int size = sizeof(pmsg->mark);
		guild->UpdateMark( pmsg->markRegistDate, pmsg->mark, size );

		// 길드원들에게 변경된 마크인덱스 전송
		SendGuildMarkUpdate( guild );
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SynUpdateMark[%d], GuildCount[%d]", pmsg->idx, mGuildMap.GetSize() );
		NETWORK2->PostServerEvent("1 : mGuildTotalLength[%d], mGuildTotalOffset[%d], mGuildSaveOffset[%d], mGuildSelectLength[%d]",
			mGuildTotalLength, mGuildTotalOffset, mGuildSaveOffset, mGuildSelectLength );
	}
	return true;
}

bool cGuildManager::SynGuildChat( void* msg )
{
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;
	MSG_SYN_POSTGUILD_CHAT* pmsg = (MSG_SYN_POSTGUILD_CHAT*)msg;

	cGuild* guild = GetGuild( pmsg->idx );
	if( guild != NULL )
	{
		cGuildUserMap* mUserMap = guild->GetGuildUser();
		if( mUserMap != NULL )
		{
			cGuildUserMap::cIterator i = mUserMap->Begin();
			cGuildUserMap::cIterator end = mUserMap->End();
			for( ; i != end; ++i )
			{
				TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
				if( p == NULL )
				{
					NETWORK2->PostServerEvent("cGuildManager::SynGuildChat p == NULL");
					continue;
				}
				cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
				if( user )
				{
					HANDLE		handle = NULL;
					MSG_SYN_CHAT_GUILD* sendMsg = (MSG_SYN_CHAT_GUILD*)NETWORK2->GetMsgRoot( &handle, user->GetConnectionIdx(), NM_CHAT, NM_CHAT_GUILD_SYN );
					if( sendMsg != NULL )
					{
                        sendMsg->job = pmsg->Job;
						wcscpy( sendMsg->Name,        pmsg->Name );
						wcscpy( sendMsg->Message,     pmsg->Message );
						memcpy( &sendMsg->inventory, &pmsg->inventory, sizeof(sInventory) );

						NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
					}
				}
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SynGuildChat - Guild User Map EMPTY [%d]", pmsg->idx );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGuildManager::SynGuildChat[%d]", pmsg->idx );
	}

	return true;
}

//----------------------------------------------------------------------------------------------------
// sync
void cGuildManager::PostGuildUserConnect( unsigned long idx, unsigned long characterIdx, BYTE connect, DWORD channelnum, bool sendConnect )
{
	cSender* sender = g_gameSrv->GetSender();

	MSG_SYN_POSTGUILD_CONNECT sync;
	memset( &sync, 0, sizeof(MSG_SYN_POSTGUILD_CONNECT) );

	sync.Category		= NM_GUILD;
	sync.Protocol		= NM_GUILDPOST_CONNECT_SYN;
	sync.idx			= idx;
	sync.characterIdx	= characterIdx;
	sync.channelNum		= channelnum;
	sync.connect		= connect;
	sync.sendConnect	= sendConnect;

	sender->PostGuildSync( NETWORK2->GetCID(), (char*)&sync, sizeof(MSG_SYN_POSTGUILD_CONNECT) );
}

// 길드원 접속 유지 정보 보내기
void cGuildManager::PostGuildTTL( cGuild* guild )
{
	if( guild == NULL )
	{
		NETWORK2->PostServerEvent("cGuildManager::PostGuildTTL" );
		return;
	}

	// 
	typedef tPointerHashMap<unsigned long, void*> cGuildUserMap;

	cGuildUserMap* mUserMap = guild->GetGuildUser();
	if( mUserMap == NULL )
	{
		NETWORK2->PostServerEvent("cGuildManager::PostGuildTTL - Guild User Map EMPTY [%d]" );
		return;
	}

	cSender* sender = g_gameSrv->GetSender();

	// 접속 유지중인 유저 정보를 보낸다
	cGuildUserMap::cIterator i = mUserMap->Begin();
	cGuildUserMap::cIterator end = mUserMap->End();
	for( ; i != end; ++i )
	{
		TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
		if( p )
		{
			cPlayer* user = OBJECTMANAGER->GetPlayer( p->characterIdx );
			if( user != NULL )
			{
				MSG_SYN_POSTGUILD_TTL sync;
				memset( &sync, 0, sizeof(MSG_SYN_POSTGUILD_TTL) );

				sync.Category	= NM_GUILD;
				sync.Protocol	= NM_GUILDPOST_TTL_SYN;
				sync.idx		= guild->GetGuildIndex();
				sync.data.channelNum = NETWORK2->GetChannelNum();
				user->GetGuildUserData( &sync.data );

				sender->PostGuildSync( NETWORK2->GetCID(), (char*)&sync, sizeof(MSG_SYN_POSTGUILD_TTL) );
			}
		}
		else
		{
			NETWORK2->PostServerEvent("cGuildManager::SendGuildUserConnect p == NULL");
			continue;
		}
	}
}

// PostGuildCreate Method
bool cGuildManager::PostGuildCreate( GUILD_CREATE* guildCreate, cPlayer* player )
{
	cSender* sender = g_gameSrv->GetSender( );

	/// 정보구성
	MSG_SYN_POSTGUILD_CREATE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_CREATE_SYN;
	msg.idx			= guildCreate->idx;
	msg.masterIndex = player->GetObjectID();
	msg.level		= player->GetLevel();
	msg.job			= (unsigned short)player->GetJob();
	msg.mapnum		= player->GetMapNumber();
	msg.position	= player->GetGuildPosition();
	wcscpy( msg.name, guildCreate->name );
	wcscpy( msg.masterName, guildCreate->masterName );

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_CREATE) );
}

// PostGuildDelete Method
bool cGuildManager::PostGuildDelete( long idx )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_DELETE msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_DELETE) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_DELETE_SYN;
	msg.idx		 = idx;

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_DELETE) );
}

// PostGuildUserAdd Method
bool cGuildManager::PostGuildUserAdd( long idx, cPlayer* player, short mapnum )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_USERADD msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_USERADD) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_USERADD_SYN;
	msg.idx		 = idx;
	msg.characterIdx = player->GetObjectID();
	msg.level		= player->GetLevel();
	msg.job			= (unsigned short)player->GetJob();
	msg.mapnum		= mapnum;
	msg.position	= player->GetGuildPosition();
	wcscpy( msg.name, player->GetName() );

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_USERADD) );
}

// PostGuildUserCut Method
bool cGuildManager::PostGuildUserCut( long idx, long characterIdx )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_USERCUT msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_USERCUT) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_USERCUT_SYN;
	msg.idx			 = idx;
	msg.characterIdx = characterIdx;

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_USERCUT) );
}

// PostGuildUserOut Method
bool cGuildManager::PostGuildUserOut( long idx, long characterIdx )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_USEROUT msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_USEROUT) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_USEROUT_SYN;
	msg.idx			 = idx;
	msg.characterIdx = characterIdx;

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_USEROUT) );
}

// PostGuildGivePosition Method
bool cGuildManager::PostGuildGivePosition( long idx, long characterIdx, char position )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_GIVEPOSITION msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_GIVEPOSITION) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_GIVEPOSITION_SYN;
	msg.idx		 = idx;
	msg.characterIdx = characterIdx;
	msg.position = position;

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_GIVEPOSITION) );
}

// PostGuildUpdateMark Method
bool cGuildManager::PostGuildUpdateMark( long idx, TIMESTAMP_STRUCT markRegistDate, BYTE* mark, int size )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_UPDATEMARK msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_UPDATEMARK) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_UPDATEMARK_SYN;
	msg.idx				= idx;
	msg.markRegistDate  = markRegistDate;
	memcpy( &msg.mark, mark, size );

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, sizeof(MSG_SYN_POSTGUILD_UPDATEMARK) );
}

// PostGuildNotice Method
bool cGuildManager::PostGuildNotice( long idx, wchar_t* notice )
{
	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_NOTICE msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_NOTICE) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_NOTICE_SYN;
	msg.idx		 = idx;
	wcscpy( msg.notice, notice );

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, msg.GetMsgLength() );
}

bool cGuildManager::PostGuildChat( cPlayer* player, wchar_t* message, u_int length, sInventory* inventory )
{
    if ( player == NULL )
        return false;

	cSender* sender = g_gameSrv->GetSender( );

	MSG_SYN_POSTGUILD_CHAT msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTGUILD_CHAT) );
	msg.Category = NM_GUILD;
	msg.Protocol = NM_GUILDPOST_CHAT_SYN;
	msg.idx		 = player->GetGuildIndex();
    msg.Job      = (unsigned short)player->GetJob();

	wcscpy( msg.Name, player->GetName() );
	memcpy( msg.Message, message, length );	
	memcpy( &msg.inventory, inventory, sizeof(sInventory) );

	return sender->PostGuildSync( NETWORK2->GetCID(), (char*)&msg, msg.GetMsgLength() );
}
