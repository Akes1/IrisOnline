#include "gamesrv.h"
#include "stdafx.h"
#include "ObjectManager.h"
#include "UserPortal.h"
#include "Party.h"
#include "PartyManager.h"

cUserPortal::cUserPortal( void ) : cBaseObject( eOBJECTTYPE_USERPORTAL )
{
}


cUserPortal::~cUserPortal( void )
{
}			


void* cUserPortal::operator new( size_t n )
{
	if( n != sizeof(cUserPortal) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocUserPortal();
}


void cUserPortal::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cUserPortal) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreeUserPortal( static_cast<cUserPortal*>(ptr) );

	return;
}


bool cUserPortal::InitUserPortal( unsigned long objectIdx, unsigned long creatorIdx, unsigned long mapChangePosIdx,
								 unsigned short mapNumber, float posX, float posY )
{
	mEndTime = NETWORK2->GetAccumTime() + USERPORTAL_ALIVE_TIME;
	mObject.index = objectIdx;
	mMapNumber = mapNumber;
	mObjectPos.x = posX;
	mObjectPos.y = posY;
	mMapChangePosIdx = mapChangePosIdx;
	mCreator = creatorIdx;

	return true;
}


void cUserPortal::ProcessUserPortal( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( accumTime > mEndTime )
	{
		OBJECTMANAGER->InsertDeleteUserPortal( mObject.index );
	}
}


bool cUserPortal::IsMapChange( unsigned long playerIdx )
{
	if( mCreator == playerIdx )
		return true;

	cPlayer* pCreator = OBJECTMANAGER->GetPlayer( mCreator );
	if( pCreator == NULL )
		return false;

	cParty* pParty = PARTYMAN->GetParty( pCreator->GetPartyIndex() );
	if( pParty == NULL )
		return false;

	for( unsigned long i = 0 ; pParty->GetCount() > i ; ++i )
	{
		if( pParty->GetUserArr()[i] == playerIdx )
			return true;
	}

	return false;
}


// SendSightIn Method
bool cUserPortal::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE                 handle  = NULL;
	MSG_USERPORTAL_CREATE* sendMsg = (MSG_USERPORTAL_CREATE*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mObjectIdx         = mObject.index;
		sendMsg->mMapChangePosIdx	= mMapChangePosIdx;
		sendMsg->mPlayerIdx			= mCreator;
		sendMsg->mPosX              = mObjectPos.x;
		sendMsg->mPosY              = mObjectPos.y;

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_USERPORTAL_CREATE) );
	}
	return false;
}

// SendSightOut Method
bool cUserPortal::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE                 handle  = NULL;
	MSG_USERPORTAL_DELETE* sendMsg = (MSG_USERPORTAL_DELETE*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mObjectIdx = mObject.index;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_USERPORTAL_DELETE) );
	}
	return false;
}
