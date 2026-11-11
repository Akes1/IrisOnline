#include "stdafx.h"
#include "PartyUnion.h"
#include "PartyUnionManager.h"

cPartyUnion::cPartyUnion()
: mIndex(0)
, mLeaderIndex(0)
, mCount(0)
{
	::memset( &mUserArr, 0, sizeof(mUserArr) );
}

cPartyUnion::~cPartyUnion()
{

}

void* cPartyUnion::operator new( size_t n )
{
	if( n != sizeof(cPartyUnion) )
	{
		assert(0);
		return NULL;
	}
	return PARTYUNIONMAN->AllocPartyUnion();
}

///
void cPartyUnion::operator delete( void* ptr, size_t n )
{
	if( ptr == NULL )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cPartyUnion) )
	{
		assert(0);
		return;
	}
	PARTYUNIONMAN->FreePartyUnion( static_cast<cPartyUnion*>(ptr) );
	return;
}

int cPartyUnion::AddUser( unsigned long userIdx )
{
	if( mCount < MAX_PARTYGROUP * MAX_PARTYUNION )
	{
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			if( mUserArr[i] == 0 )
			{
				mUserArr[i] = userIdx;
				mCount++;
				return i;
			}
		}
	}
	return -1;
}

bool cPartyUnion::DeleteUser( unsigned long userIdx )
{
	if( mCount > 0 )
	{
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			if( mUserArr[i] == userIdx )
			{
				mUserArr[i] = 0;
				mCount--;
				return true;
			}
		}
	}
	return false;
}

// 바꿀 배열 인덱스들
bool cPartyUnion::MoveUser( unsigned char i0, unsigned char i1 )
{
	if( i0 < MAX_PARTYUNION && i1 < MAX_PARTYUNION )
	{
		// i0은 반드시 있어야한다
		if( mUserArr[i0] == 0 )
			return false;

		// 
		unsigned long index = mUserArr[i0];
		mUserArr[i0] = mUserArr[i1];
		mUserArr[i1] = index;
		return true;
	}
	return false;
}

// 해당 유저 다음으로 존재하는 유저 인덱스 찾기
int cPartyUnion::FindNextLeader()
{
	if( mCount > 0 )
	{
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			if( mUserArr[i] > 0 )
				return mUserArr[i];
		}
	}
	return -1;
}

bool cPartyUnion::IsUser( unsigned long userIdx1, unsigned long userIdx2 )
{
	if( mCount > 0 )
	{
		unsigned long* userArr = GetGroupArr( userIdx1 );
		if( userArr )
		{
			for( unsigned int i = 0; i < MAX_PARTYUSER; ++i )
			{
				if( userArr[i] == userIdx2 )
					return true;
			}
		}
	}
	return false;
}

unsigned long* cPartyUnion::GetGroupArr( unsigned long userIdx )
{
	if( mCount > 0 )
	{
		int groupIndex = GetGroup( userIdx );
		if( groupIndex != -1 )
		{
			return &mUserArr[groupIndex * MAX_PARTYUSER];
		}
	}
	return 0;
}

int cPartyUnion::GetGroup( unsigned long userIdx )
{
	if( mCount > 0 )
	{
		for( int i = 0; i < MAX_PARTYUNION; ++i )
		{
			if( mUserArr[i] == userIdx )
				return GetGroup( i );
		}
	}
	return -1;
}

int cPartyUnion::GetGroup( int i )
{
	int group = -1;

	if( i >= 0 && i < MAX_PARTYUSER )
	{
		group = eUNION_GROUP1;
	}
	else if( i >= MAX_PARTYUSER && i < MAX_PARTYUSER * eUNION_GROUP3 )
	{
		group = eUNION_GROUP2;
	}
	else if( i >= MAX_PARTYUSER * eUNION_GROUP3 && i < MAX_PARTYUSER * eUNION_MAX )
	{
		group = eUNION_GROUP3;
	}

	return group;
}