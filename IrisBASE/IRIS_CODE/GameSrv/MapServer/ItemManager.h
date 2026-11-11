/* ====================================================================
 * 파    일 :	ItemManager.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/11/02
 * 주의사항 :	
 * =================================================================== */
#ifndef __ITEM_MANAGER_H__
#define __ITEM_MANAGER_H__

///#ifndef WINVER
///#define WINVER          0x0500
///#endif
///
///#ifndef _WIN32_WINNT
///#define _WIN32_WINNT    0x0500
///#endif

#pragma once

#include "gamesrvdefines.h"
#include "itemscript.h"

/// 세트 아이템 정보
struct sSetAblityOption
{
	short mOptType[2];		/// 옵션 인덱스겸 타입
	short mOptValue[2];		/// 옵션 값

	unsigned long mActIdx;

	sSetAblityOption()
	{
		mOptType[0] = 0;
		mOptValue[0] = 0;

		mOptType[1] = 0;
		mOptValue[1] = 0;

		mActIdx = 0;
	}
};

class cItemSetInfo
{
	friend class cItemManager;
public:
	~cItemSetInfo();

	void AddOption( unsigned int cnt, sSetAblityOption* opt ) { mSetOption.Insert( cnt, opt ); }

	sSetAblityOption* GetItemOption( unsigned int cnt ) { return (sSetAblityOption*)mSetOption.GetAt(cnt); }

private:
	/// 갯수별 추가 효과( key:갯수 )
	tPointerHashMap<unsigned int, void*> mSetOption;
};

class cItemManager
{
typedef tPointerHashMap<unsigned long, TB_ITEM_DEFINE*> ItemDefineMap;

private:
	TB_ITEM_DEFINE*        mItemDefine;
	DWORD                  mItemDefineOffset;
	DWORD                  mItemDefineLength;

	TB_ITEM_ABILITY*       mItemAbility;
	DWORD                  mItemAbilityOffset;
	DWORD                  mItemAbilityLength;

	TB_ITEM_COOLTIME2*     mItemCooltime2;
	DWORD                  mItemCooltime2Offset;
	DWORD                  mItemCooltime2Length;

	TB_ITEM_LIMIT*         mItemLimit;
	DWORD                  mItemLimitOffset;
	DWORD                  mItemLimitLength;

	TB_ITEM_CARD*          mItemCard;
	DWORD                  mItemCardOffset;
	DWORD                  mItemCardLength;

	TB_ITEM_CARD_SLOT*     mItemCardSlot;
	DWORD                  mItemCardSlotOffset;
	DWORD                  mItemCardSlotLength;

	TB_ITEM_TAROT*         mItemTarot;
	DWORD                  mItemTarotOffset;
	DWORD                  mItemTarotLength;

	TB_ITEM_ENHANCED*      mItemEnhanced;
	DWORD                  mItemEnhancedOffset;
	DWORD                  mItemEnhancedLength;

	TB_ITEM_ENHANCED_RATE* mItemEnhancedRate;
	DWORD                  mItemEnhancedRateOffset;
	DWORD                  mItemEnhancedRateLength;

	TB_ITEM_DISJOINT*      mItemDisjoint;
	DWORD                  mItemDisjointOffset;
	DWORD                  mItemDisjointLength;

	TB_ITEM_CHANGE*        mItemChange;
	DWORD                  mItemChangeOffset;
	DWORD                  mItemChangeLength;

	ItemDefineMap*         mItemDefineMap;

	unsigned long          mItemDropIdx;
	unsigned long          mLastDropIdx;

    cItemScript*		    mpItemScript;

	typedef tPointerHashMap<unsigned int, void*> cItemSetMap;
	cItemSetMap mSetInfoMap;

public:
	cItemManager(void);

	bool                   Init                   ( );

	bool                   CheckItemDefine        ( ) { return mItemDefineOffset == mItemDefineLength; }
	DWORD                  ItemDefineLength       ( ) { return mItemDefineLength; }
	void                   AllocItemDefine        ( DWORD length );
	void                   FreeItemDefine         ( );

	bool                   CheckItemAbility       ( ) { return mItemAbilityOffset == mItemAbilityLength; }
	DWORD                  ItemAbilityLength      ( ) { return mItemAbilityLength; }
	void                   AllocItemAbility       ( DWORD length );
	void                   FreeItemAbility        ( );

	bool                   CheckItemCooltime2     ( ) { return mItemCooltime2Offset == mItemCooltime2Length; }
	DWORD                  ItemCooltime2Length    ( ) { return mItemCooltime2Length; }
	void                   AllocItemCooltime2     ( DWORD length );
	void                   FreeItemCooltime2      ( );

	bool                   CheckItemLimit         ( ) { return mItemLimitOffset == mItemLimitLength; }
	DWORD                  ItemLimitLength        ( ) { return mItemLimitLength; }
	void                   AllocItemLimit         ( DWORD length );
	void                   FreeItemLimit          ( );

	bool                   CheckItemCard          ( ) { return mItemCardOffset == mItemCardLength; }
	DWORD                  ItemCardLength         ( ) { return mItemCardLength; }
	void                   AllocItemCard          ( DWORD length );
	void                   FreeItemCard           ( );

	bool                   CheckItemCardSlot      ( ) { return mItemCardSlotOffset == mItemCardSlotLength; }
	DWORD                  ItemCardSlotLength     ( ) { return mItemCardSlotLength; }
	void                   AllocItemCardSlot      ( DWORD length );
	void                   FreeItemCardSlot       ( );

	bool                   CheckItemTarot         ( ) { return mItemTarotOffset == mItemTarotLength; }
	DWORD                  ItemTarotLength        ( ) { return mItemTarotLength; }
	void                   AllocItemTarot         ( DWORD length );
	void                   FreeItemTarot          ( );

	bool                   CheckItemEnhanced      ( ) { return mItemEnhancedOffset == mItemEnhancedLength; }
	DWORD                  ItemEnhancedLength     ( ) { return mItemEnhancedLength; }
	void                   AllocItemEnhanced      ( DWORD length );
	void                   FreeItemEnhanced       ( );

	bool                   CheckItemEnhancedRate  ( ) { return mItemEnhancedRateOffset == mItemEnhancedRateLength; }
	DWORD                  ItemEnhancedRateLength ( ) { return mItemEnhancedRateLength; }
	void                   AllocItemEnhancedRate  ( DWORD length );
	void                   FreeItemEnhancedRate   ( );

	bool                   CheckItemDisjoint      ( ) { return mItemDisjointOffset == mItemDisjointLength; }
	DWORD                  ItemDisjointLength     ( ) { return mItemDisjointLength; }
	void                   AllocItemDisjoint      ( DWORD length );
	void                   FreeItemDisjoint       ( );

	bool                   CheckItemChange        ( ) { return mItemChangeOffset == mItemChangeLength; }
	DWORD                  ItemChangeLength       ( ) { return mItemChangeLength; }
	void                   AllocItemChange        ( DWORD length );
	void                   FreeItemChange         ( );

	bool                   IsItemDefineIndex      ( long idx );
	bool                   IsItemDefineType       ( long idx, ITEM_TYPE type );
	bool                   IsItemDefineType       ( long idx, ITEM_TYPE type, ITEM_TYPE subType );

	ITEM_TYPE              GetItemDefineType      ( long itemDefineIdx );
	ITEM_TYPE              GetItemDefineSubType   ( long itemDefineIdx );

	TB_ITEM_DEFINE*        GetItemDefineByIndex   ( long itemDefineIndex ) { return mItemDefineMap->GetAt( itemDefineIndex ); }
	TB_ITEM_DEFINE*        GetItemDefine          ( long itemDefineIdx );
	TB_ITEM_ABILITY*       GetItemAbility         ( long itemDefineIdx );
	TB_ITEM_COOLTIME2*     GetItemCooltime2       ( long itemDefineIdx );
	TB_ITEM_LIMIT*         GetItemLimit           ( long itemDefineIdx );
	TB_ITEM_CARD*          GetItemCard            ( long itemDefineIdx );
	TB_ITEM_CARD_SLOT*     GetItemCardSlot        ( long itemDefineIdx );
	TB_ITEM_TAROT*         GetItemTarot           ( long itemDefineIdx );
	TB_ITEM_ENHANCED*      GetItemEnhanced        ( long itemDefineIdx );
	TB_ITEM_ENHANCED_RATE* GetItemEnhancedRate    ( long itemDefineIdx );
	TB_ITEM_DISJOINT*      GetItemDisjoint        ( long itemDefineIdx );
	TB_ITEM_CHANGE*        GetItemChange          ( long itemDefineIdx );

	void                   SetItemDefine          ( TB_ITEM_DEFINE* itemDefine );
	void                   SetItemAbility         ( TB_ITEM_ABILITY* itemAbility );
	void                   SetItemCooltime2       ( long idx, long rowCount, TB_ITEM_COOLTIME1* itemCooltime1 );
	void                   SetItemLimit           ( TB_ITEM_LIMIT* itemLimit );
	void                   SetItemCard            ( TB_ITEM_CARD* itemCard );
	void                   SetItemCardSlot        ( TB_ITEM_CARD_SLOT* itemCardSlot );
	void                   SetItemTarot           ( TB_ITEM_TAROT* itemTarot );
	void                   SetItemEnhanced        ( TB_ITEM_ENHANCED* itemEnhanced );
	void                   SetItemEnhancedRate    ( TB_ITEM_ENHANCED_RATE* itemEnhancedRate );
	void                   SetItemDisjoint        ( TB_ITEM_DISJOINT* itemDisjoint );
	void                   SetItemChange          ( TB_ITEM_CHANGE* itemChange );

	unsigned long          MakeItemIdx            ( );

	///
	cItemSetInfo*		   GetItemSetInfo		  ( unsigned int setIndex ) const;

private:
	bool				   LoadItemSet			  ( const cString& pathName );

public:
	virtual ~cItemManager(void);

public:
	static cItemManager* mpItemManager;
};

#define ITEMMANAGER	cItemManager::mpItemManager
#define COOLTIMEPOOL cCooltimePool::mpCooltimePool

void Inventory2sInventory( sInventory* sinventory, TB_INVENTORY* inventory );
BYTE ItemCardType2Mtr( BYTE type );
BYTE ItemSlotType2Mtr( BYTE type );

#endif /// __ITEM_MANAGER_H__
