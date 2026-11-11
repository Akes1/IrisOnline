/* ==========================================================================
* 파    일 : Item.h
* 목    적 : 
* 작 성 자 : 곽철중
* 작 성 일 : 2007-11-05
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Item_Common.h"
#include "BaseObject.h"

class cItem : public cBaseObject
{
private:
	sObject       mFromObject;
	sObject       mOpenObject;

	bool          mGetAuto;
	bool          mOpen;

	bool          mApplyOwner;
	bool          mApplyParty;
	bool          mApplyPartyLeader;

	unsigned long mTimeToLive;
	unsigned long mTimeToOwner;
	unsigned long mTimeToParty;

	BYTE          mOwnerCount;
	unsigned long mOwner[ MAX_ITEM_OWNER ];

	BYTE          mPartyCount;
	unsigned long mParty[ MAX_ITEM_PARTY ];

	BYTE          mRowCount;
	sItemData     mItemData[ MAX_ITEMS ];

public:
	cItem(void);
	virtual ~cItem(void);

	void* operator new                ( size_t n ); 
	void  operator delete             ( void* ptr, size_t n );

	// 데이터 초기화
	bool           Init               ( unsigned long itemIdx, cBaseObject* baseObject, bool getAuto );

	bool           IsGetAuto          ( ) { return mGetAuto; }
	bool           IsOpen             ( cBaseObject* baseObject );

	bool           IsOwner            ( cBaseObject* baseObject );
	bool           IsParty            ( cBaseObject* baseObject );
    bool           IsPartyLeader      ( ) { return mApplyPartyLeader; }

	bool           PushOwner          ( unsigned long ownerIndex );
	bool           PushParty          ( unsigned long partyIndex, bool partyLeader );
    void           PartyLeader        ( bool partyLeader ) { mApplyPartyLeader = partyLeader; }

	void           GetOwnerInfo       ( unsigned long* owner, int ownerLen, unsigned long& timeToOwner );
	void           GetPartyInfo       ( unsigned long* party, int partyLen, unsigned long& timeToParty );

	unsigned short PushItem           ( unsigned long itemDefineIndex, unsigned short count );
	unsigned short PopItem            ( unsigned long itemDefineIndex, unsigned short count );

	BYTE           GetItemData        ( sItemData* itemData );
	sItemData*     GetItemData        ( BYTE& rowCount );
	sItemData*     GetItemData        ( unsigned long itemDefineIndex );
	BYTE           GetItemRare        ( );

	void           Open               ( cBaseObject* baseObject );
	bool           Close              ( );

	bool           Update             ( unsigned long elapsedTime, unsigned long accumTime );

	// cBaseObject 의 순수 가상함수 - 정의.
	unsigned long  GetHP              ( ) { return 0; }
	unsigned long  GetMP              ( ) { return 0; }
	unsigned long  GetMaxHP           ( ) { return 0; }
	unsigned long  GetMaxMP           ( ) { return 0; }

	float          GetFixedObjectSize ( ) { return 0.0f; }

public:
	bool           SendSightIn        ( char category, char protocol, unsigned long connectionIdx );
	bool           SendSightOut       ( char category, char protocol, unsigned long connectionIdx );
};
