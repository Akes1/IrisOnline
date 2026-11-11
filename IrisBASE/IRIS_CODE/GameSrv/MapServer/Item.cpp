#include "gamesrv.h"

#include "stdafx.h"
#include "Item.h"
#include "ObjectManager.h"
#include "ItemManager.h"


// cItem Constructor.
cItem::cItem(void) : cBaseObject( eOBJECTTYPE_ITEM )
{
}

// ~cItem Destructor.
cItem::~cItem(void)
{
}

// operator new
void* cItem::operator new( size_t n )
{
	if( n != sizeof(cItem) ) 
	{
		assert(0);
		return NULL;
	}
	return OBJECTMANAGER->AllocItem();
}

// operator delete
void cItem::operator delete(void* ptr, size_t n)
{
	/// NULL 포인터 검사
	if ( ptr == 0 )
	{
		assert(0);
		return;
	}
	if ( n != sizeof(cItem) ) 
	{
		assert(0);
		return;
	}
	OBJECTMANAGER->FreeItem( static_cast<cItem*>(ptr) );
}

// Init Method
bool cItem::Init(unsigned long itemIdx, cBaseObject* baseObject, bool getAuto)
{
	mObject.index = itemIdx;

	mMapNumber  = baseObject->GetMapNumber( );
	mObjectPos  = baseObject->GetPos( );

	mFromObject = baseObject->GetObject( );

	mOpenObject.type  = eOBJECTTYPE_NONE;
	mOpenObject.index = 0;

	mGetAuto    = getAuto;
	mOpen       = false;

	mApplyOwner         = false;
	mApplyParty         = false;
    mApplyPartyLeader   = false;

	mTimeToLive  = mGetAuto ? 0 : NETWORK2->GetItemTimeToLive( );
	mTimeToOwner = NETWORK2->GetItemTimeToOwner( );
	mTimeToParty = NETWORK2->GetItemTimeToParty( );

	mOwnerCount = 0;
	memset( mOwner, 0, sizeof(mOwner) );

	mPartyCount = 0;
	memset( mParty, 0, sizeof(mParty) );

	mRowCount = 0;
	memset( mItemData, 0, sizeof(mItemData) );
	return true;
}

// IsOpen Method
bool cItem::IsOpen(cBaseObject* baseObject)
{
	if ( mOpen == true )
	{
		sObject object = baseObject->GetObject( );
		return (mOpenObject.type == object.type && mOpenObject.index == object.index) ? true : false;
	}
	return false;
}

// IsOwner Method
bool cItem::IsOwner(cBaseObject* baseObject)
{
	if ( mApplyOwner == true && mOwnerCount > 0 )
	{
		unsigned long objectIndex = baseObject->GetObjectID( );
		for ( BYTE i = 0; i <  mOwnerCount; i++ )
		{
			if ( mOwner[ i ] == objectIndex )
				return true;
		}
		return false;
	}
	return true;
}

// IsParty Method
bool cItem::IsParty(cBaseObject* baseObject)
{
	if ( mApplyParty == true && mPartyCount > 0 )
	{
		unsigned long objectIndex = baseObject->GetObjectID( );
		for ( BYTE i = 0; i <  mPartyCount; i++ )
		{
			if ( mParty[ i ] == objectIndex )
				return true;
		}
		return false;
	}
	return true;
}

// PushOwner Method
bool cItem::PushOwner(unsigned long ownerIndex)
{
	if ( mOwnerCount < MAX_ITEM_OWNER )
	{
		mApplyOwner = true;
		mOwner[ mOwnerCount ] = ownerIndex;
		mOwnerCount++;
		return true;
	}
	return false;
}

// PushParty Method
bool cItem::PushParty(unsigned long partyIndex, bool partyLeader)
{
	if ( mPartyCount < MAX_ITEM_PARTY )
	{
		mApplyParty = true;
		mParty[ mPartyCount ] = partyIndex;
		mPartyCount++;

        mApplyPartyLeader = partyLeader;

        if ( mApplyPartyLeader && NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
            mTimeToOwner = NETWORK2->GetAccumTime() + 180000;       // 3분
        else
            mTimeToOwner = NETWORK2->GetAccumTime() + 30000;

		return true;
	}
	return false;
}

// GetOwnerInfo Method
void cItem::GetOwnerInfo(unsigned long* owner, int ownerLen, unsigned long& timeToOwner)
{
	unsigned long* ptr = mOwner;
	BYTE           cnt = 0;
	int            len = sizeof(unsigned long);

	while ( cnt < mOwnerCount && !(ownerLen < len) )
	{
		(*owner) = (*ptr);

		owner++;
		ptr++;
		cnt++;

		ownerLen -= len;
	}

	timeToOwner = mApplyOwner ? mTimeToOwner - NETWORK2->GetAccumTime( ) : 0;
}

// GetPartyInfo Method
void cItem::GetPartyInfo(unsigned long* party, int partyLen, unsigned long& timeToParty)
{
	unsigned long* ptr = mParty;
	BYTE           cnt = 0;
	int            len = sizeof(unsigned long);

	while ( cnt < mPartyCount && !(partyLen < len) )
	{
		(*party) = (*ptr);

		party++;
		ptr++;
		cnt++;

		partyLen -= len;
	}

	timeToParty = mApplyParty ? mTimeToParty - NETWORK2->GetAccumTime( ) : 0;
}

// PushItem Method
unsigned short cItem::PushItem(unsigned long itemDefineIndex, unsigned short count)
{
	if ( mRowCount < MAX_ITEMS )
	{
		TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
		if ( itemDefine != NULL )
		{
			if ( count > itemDefine->capacity )
				count = itemDefine->capacity;

			mItemData[ mRowCount ].itemDefineIndex = itemDefineIndex;
			mItemData[ mRowCount ].count           = count;
			mRowCount +=1;

			return count;
		}
	}
	return 0;
}

// PopItem Method
unsigned short cItem::PopItem(unsigned long itemDefineIndex, unsigned short count)
{
	sItemData* itemData = mItemData;
	for ( int i = 0; i < MAX_ITEMS && mRowCount > 0; i++, itemData++ )
	{
		if ( itemData->itemDefineIndex == itemDefineIndex && itemData->count >= count )
		{
			itemData->count = itemData->count - count;
			if ( itemData->count == 0 )
			{
				itemData->itemDefineIndex = 0;
				itemData->count           = 0;
				mRowCount -=1;
			}
			return count;
		}
	}
	return count;
}

// GetItemData Method
BYTE cItem::GetItemData(sItemData* itemData)
{
	BYTE rowCount = 0;
	for ( int i = 0; i < MAX_ITEMS && rowCount < mRowCount; i++ )
	{
		if ( mItemData[ i ].itemDefineIndex > 0 )
		{
			itemData->itemDefineIndex = mItemData[ i ].itemDefineIndex;
			itemData->count           = mItemData[ i ].count;
			itemData++;

			rowCount++;
		}
	}
	return rowCount;
}
sItemData* cItem::GetItemData(BYTE& rowCount)
{
	rowCount = mRowCount;
	return mItemData;
}

sItemData* cItem::GetItemData(unsigned long itemDefineIndex)
{
	for ( int i = 0; i < MAX_ITEMS; i++ )
	{
		if ( mItemData[ i ].itemDefineIndex == itemDefineIndex )
			return &mItemData[ i ];
	}
	return NULL;
}

// GetItemRare Method
BYTE cItem::GetItemRare( )
{
	sItemData*      ptr = mItemData;
	TB_ITEM_DEFINE* itemDefine;
	BYTE            rare = 0;

	for ( BYTE i = 0; i < mRowCount; i++, ptr++ )
	{
		itemDefine = ITEMMANAGER->GetItemDefineByIndex( ptr->itemDefineIndex );
		if ( itemDefine )
		{
			rare = max( rare, itemDefine->rareLevel );
		}
	}

	return rare;
}

// Open Method
void cItem::Open(cBaseObject* baseObject)
{
	if ( mOpen == true )
		throw ERROR_ITEM_GET_OPEN_EXIST;

	if ( IsOwner( baseObject) == false )
		throw ERROR_ITEM_GET_OPEN_OWNER;

	if ( IsPartyLeader() == false && IsParty( baseObject) == false )
		throw ERROR_ITEM_GET_OPEN_PARTY;

	float tempRange = OBJECTMANAGER->ObjectSizeRange( baseObject, this, ITEM_VALID_DISTANCE + SYNC_MOVE_RANGE );
	if ( (baseObject->GetPos( ) - mObjectPos).Length( ) > tempRange )
		throw ERROR_ITEM_GET_OPEN_DISTANCE;

	mOpenObject = baseObject->GetObject( );
	mOpen       = true;
}

// Close Method
bool cItem::Close( )
{
	if ( mOpen == true )
	{
		mOpen = false;
		return true;
	}
	return false;
}

// Update Method
bool cItem::Update(unsigned long /*elapsedTime*/, unsigned long accumTime)
{
	if ( mOpen == true )
	{
		cPlayer* player = OBJECTMANAGER->GetPlayer( mOpenObject.index );
		if ( player != NULL )
		{
			unsigned int playerState = player->GetState( );
			NiPoint2     playerPos   = player->GetPos( );

			float tempRange = OBJECTMANAGER->ObjectSizeRange( player, this, ITEM_VALID_DISTANCE + SYNC_MOVE_RANGE );
			if ( (playerPos - mObjectPos).Length( ) > tempRange || (playerState != ePLAYER_STATE_ITEMPICK) )
			{
				player->ItemGetClose( );
				Close( );
			}
		}
		else
			mOpen = false;

		return true;
	}
	else
	{
		if ( mApplyOwner == true )
		{
			if ( mTimeToOwner < accumTime )
			{
				mTimeToOwner = 0;
				mApplyOwner = false;
			}
		}
		if ( mApplyParty == true )
		{
			if ( mTimeToParty < accumTime )
			{
				mTimeToParty = 0;
				mApplyParty = false;
			}
		}
		return ( !(mRowCount > 0) || !(mTimeToLive > accumTime) ) ? false : true;
	}
}

// SendSightIn Method
bool cItem::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE         handle  = NULL;
	MSG_ITEM_INFO* sendMsg = (MSG_ITEM_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sItemDrop* itemDrop = &sendMsg->itemDrop;

		GetOwnerInfo( sendMsg->owner, sizeof(sendMsg->owner), sendMsg->timeToOwner );
		GetPartyInfo( sendMsg->party, sizeof(sendMsg->party), sendMsg->timeToParty );
		sendMsg->rare = GetItemRare( );

		itemDrop->idx  = mObject.index;
		itemDrop->xPos = mObjectPos.x;
		itemDrop->yPos = mObjectPos.y;

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_ITEM_INFO) );
	}
	return false;
}

// SendSightOut Method
bool cItem::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE            handle  = NULL;
	MSG_ITEMIDX* sendMsg = (MSG_ITEMIDX*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->idx = mObject.index;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_ITEMIDX) );
	}
	return false;
}
