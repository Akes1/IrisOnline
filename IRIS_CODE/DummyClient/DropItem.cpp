#include "stdafx.h"
#include "Object.h"

cDropItem::cDropItem( cObjectManager* manager )
: cObject( eOBJECTTYPE_ITEM, manager )
{
	memset( mOwner, 0, sizeof(unsigned long)*MAX_ITEM_OWNER );
	mTimeToOwn = 0;

	memset( mParty, 0, sizeof(unsigned long)*MAX_ITEM_PARTY );
	mTimeToParty = 0;

	mIsRareItem = false;

	mAllOwn = false;
}

cDropItem::~cDropItem()
{

}

bool cDropItem::Init( unsigned long idx, NiPoint3 pos, unsigned long* owner, unsigned long ownTime, unsigned long* party, unsigned long partyTime, bool rare )
{
	mObjectId = idx;
	mTrans.m_Translate = pos;

	memcpy( mOwner, owner, sizeof(unsigned long)*MAX_ITEM_OWNER );
	mTimeToOwn = ownTime;

	memcpy( mParty, party, sizeof(unsigned long)*MAX_ITEM_PARTY );
	mTimeToParty = partyTime;

	mIsRareItem = rare;

	if( mTimeToOwn == 0 && mTimeToParty == 0 )
		mAllOwn = true;

	return true;
}

void cDropItem::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mTimeToOwn < deltaTime )
	{
		mTimeToOwn = 0;
		mAllOwn = true;
	}
	else
		mTimeToOwn -= deltaTime;
}

bool cDropItem::IsOwner( unsigned long idx )
{
	if( mAllOwn == false )
	{
		for( unsigned int i=0; i<MAX_ITEM_OWNER; i++ )
		{
			if( mOwner[i] == idx )
				return true;
		}
	}

	return mAllOwn;
}
