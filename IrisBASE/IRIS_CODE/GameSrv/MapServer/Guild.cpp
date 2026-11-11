#include "stdafx.h"
#include "gamesrv.h"
#include "Guild.h"
#include "Guild_Common.h"
#include "Player.h"

cGuild::cGuild()
: mLastAccumTime(0)
, mDBLastAccumTime(0)
{
	memset( &mInfo, 0, sizeof(mInfo) );
}

cGuild::~cGuild()
{
	Clear();
}

void cGuild::Clear()
{
	/// 유저리스트 삭제
	cGuildUserMap::cIterator i = mGuildUserMap.Begin();
	cGuildUserMap::cIterator end = mGuildUserMap.End();
	for( ; i != end; ++i )
	{
		TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
		SAFE_DELETE( p );
	}
	mGuildUserMap.Clear();
}

/// 서버 시작시 세팅
bool cGuild::VerifyAddPlayer( TB_GUILD_USER* data )
{
	TB_GUILD_USER* p = new TB_GUILD_USER;

	/// 복사
	(*p) = (*data);

	/// 데이터 삽입
	return  mGuildUserMap.Insert( data->characterIdx, p );
}

/// 생성된 길드의 기본 정보 세팅
void cGuild::SetInfo( long idx, long masterIdx, wchar_t* name, wchar_t* masterName, TIMESTAMP_STRUCT initTime )
{
	mInfo.idx			= idx;
	mInfo.masterIndex	= masterIdx;
	mInfo.markDate		= initTime;

	wcscpy( mInfo.name, name );
	wcscpy( mInfo.masterName, masterName );
}

/// 플레이어 기본정보 생성
bool cGuild::AddPlayer( cPlayer* player, char position, DWORD channelNum )
{
	if( player == NULL )
		return false;

	TB_GUILD_USER* p = new TB_GUILD_USER;
	p->characterIdx	= player->GetObjectID();
	p->level		= player->GetLevel();
	p->job			= player->GetJob();
	p->mapnum		= player->GetMapNumber();
	p->position		= position;
	p->channelNum   = channelNum;
	p->sendInfo		= false;
	p->tick			= 0;
	wcscpy( p->name, player->GetName() );

	/// 데이터 삽입
	if( mGuildUserMap.Insert( p->characterIdx, p ) == false )
	{
		assert(0);
		return false;
	}
	return true;
}

bool cGuild::AddPlayer( long characterIdx, char level, long job, short mapnum, wchar_t* name, char position, DWORD channelNum )
{
	TB_GUILD_USER* p = new TB_GUILD_USER;
	p->characterIdx	= characterIdx;
	p->level		= level;
	p->job			= job;
	p->mapnum		= mapnum;
	p->position		= position;
	p->channelNum   = channelNum;
	p->sendInfo		= false;
	p->tick			= 0;
	wcscpy( p->name, name );

	/// 데이터 삽입
	if( mGuildUserMap.Insert( p->characterIdx, p ) == false )
	{
		assert(0);
		return false;
	}
	return true;
}

/// 플레이어 삭제
bool cGuild::DeletePlayer( unsigned long characterIdx )
{
	cGuildUserMap::cIterator i = mGuildUserMap.Find( characterIdx );
	if( i != mGuildUserMap.End() )
	{
		TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
		SAFE_DELETE( p );
		mGuildUserMap.Erase(i);
	}
	else
	{
		NETWORK2->PostServerEvent("cGuild::DeletePlayer[%d]", characterIdx );
		return false;
	}
	return true;
}

void cGuild::SetMaster( unsigned long characterIdx, wchar_t* name )
{
	/// 기존과 동일하면 리턴
	if( (unsigned long)mInfo.masterIndex == characterIdx )
		return;

	/// 기존 길드장 정보 변경
	TB_GUILD_USER* user = (TB_GUILD_USER*)mGuildUserMap.GetAt( mInfo.masterIndex );
	if( user )
	{
		user->position = eGUILD_GENERAL;
	}
	else
	{
		NETWORK2->PostServerEvent("cGuild::SetMaster[%d] - not exist master", mInfo.masterIndex );
	}

	/// 새 길드장 정보 세팅
	TB_GUILD_USER* master = (TB_GUILD_USER*)mGuildUserMap.GetAt( characterIdx );
	if( master )
	{
		master->position = eGUILD_MASTER;
	}
	else
	{
		NETWORK2->PostServerEvent("cGuild::SetMaster[%d] - not exist new master", characterIdx );
	}

	mInfo.masterIndex = characterIdx;
	wcscpy( mInfo.masterName, name );
}

void cGuild::SetNotice( wchar_t* notice )
{
	wcscpy( mInfo.notice, notice );
}

void cGuild::UpdateMark( TIMESTAMP_STRUCT markDate, BYTE* mark, int size )
{
	mInfo.markDate = markDate;

	if( size > 0 )
		memcpy( &mInfo.mark, mark, size );
}

void cGuild::UpdateInfo( sGuildConnect* data, unsigned long tickCount )
{
	if( data == NULL )
	{
		NETWORK2->PostServerEvent( "cGuild::UpdateInfo[%d]", mInfo.idx );
		return;
	}

	TB_GUILD_USER* user = (TB_GUILD_USER*)mGuildUserMap.GetAt( data->characterIdx );
	if( user )
	{
		user->level			= data->level;
		user->job			= data->job;
		user->channelNum	= data->channelNum;
		user->tick			= tickCount + GUILD_TTL_TICK;
		user->sendInfo		= true;
	}
	else
	{
		NETWORK2->PostServerEvent( "cGuild::UpdateInfo - user[%d,%d]", mInfo.idx, data->characterIdx );
	}
}

unsigned long cGuild::GetLostPlayer( unsigned long tickCount )
{
	cGuildUserMap::cIterator i = mGuildUserMap.Begin();
	cGuildUserMap::cIterator end = mGuildUserMap.End();
	for( ; i != end; ++i )
	{
		TB_GUILD_USER* p = (TB_GUILD_USER*)(i->mSecond);
		if( p )
		{
			if( p->tick > 0 && p->tick <= tickCount )
				return p->characterIdx;
		}
		else
		{
			NETWORK2->PostServerEvent( "cGuild::GetLostPlayer - user is null" );
			continue;
		}
	}
	return 0;
}
