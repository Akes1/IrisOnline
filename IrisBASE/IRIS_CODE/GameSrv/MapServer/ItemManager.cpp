// Include
#include "gamesrv.h"

#include "item.h"
#include "ItemManager.h"

#include "ObjectManager.h"

#include "Parser.h"
#include "Lexer.h"
#include "Tokenizer.h"


// Local definitions
#define SAFE_GLOBAL_FREE(P) if (P) { GlobalFree(P); P=NULL; }

#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }


// Global data
cItemManager* cItemManager::mpItemManager = NULL;

// Inventory2sInventory Prototype
void Inventory2sInventory(sInventory* sinventory, TB_INVENTORY* inventory)
{
	sinventory->idx       = inventory->idx;
	sinventory->ItemIndex = inventory->itemDefineIndex;
	sinventory->number    = inventory->number;
	sinventory->count     = inventory->count;
	sinventory->enhanced  = inventory->enhanced;
	sinventory->cardSlot1 = inventory->cardSlot1;
	sinventory->cardSlot2 = inventory->cardSlot2;
	sinventory->cardSlot3 = inventory->cardSlot3;
	sinventory->cardSlot4 = inventory->cardSlot4;
	sinventory->cardSlot5 = inventory->cardSlot5;
	sinventory->seal      = inventory->seal;
    sinventory->cash      = inventory->cash;
}

// ItemCardType2Mtr Prototype
BYTE ItemCardType2Mtr(BYTE type)
{
	switch ( type )
	{
	case ITEM_CARD_EQUIP_A:		// A  B  AB O (수혈가능-주기)
		return 0x0a;			// 1  0  1  0
	case ITEM_CARD_EQUIP_B:
		return 0x06;			// 0  1  1  0
	case ITEM_CARD_EQUIP_AB:
		return 0x02;			// 0  0  1  0
	case ITEM_CARD_EQUIP_O:
		return 0x0f;			// 1  1  1  1
	default:
		return 0x00;
	}
}

// ItemSlotType2Mtr Prototype
BYTE ItemSlotType2Mtr(BYTE type)
{
	switch ( type )
	{
	case ITEM_CARD_SLOT_A:		// A  B  AB O (수혈가능-받기)
		return 0x09;			// 1  0  0  1
		break;
	case ITEM_CARD_SLOT_B:
		return 0x05;			// 0  1  0  1
		break;
	case ITEM_CARD_SLOT_AB:
		return 0x0f;			// 1  1  1  1
		break;
	case ITEM_CARD_SLOT_O:
		return 0x01;			// 0  0  0  1
		break;
	default:
		return 0x00;
	}
}

//
cItemSetInfo::~cItemSetInfo()
{
	tPointerHashMap<unsigned int, void*>::cIterator i = mSetOption.Begin();
	tPointerHashMap<unsigned int, void*>::cIterator end = mSetOption.End();
	for( ; i != end; ++i )
	{
		delete (sSetAblityOption*)(i->mSecond);
	}
	mSetOption.Clear();
}


bool cItemManager::LoadItemSet( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item set info" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = (long)str.ToInt();
		assert( index > 0 );

		/// 갯수
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int count = (unsigned int)str.ToInt();

		/// 세트 이름 
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 옵션
		if( tokenizer.GetNext( &str ) == false )
			return false;

		short optType1 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		short optValue1 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		short optType2 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		short optValue2 = (short)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long actIdx = str.ToInt();


		cItemSetInfo* setInfo = (cItemSetInfo*)mSetInfoMap.GetAt(index);
		if( setInfo == 0 )
		{
			setInfo = new cItemSetInfo;
			if( setInfo == 0 || mSetInfoMap.Insert( index, setInfo ) == false )
			{
				assert(0);
				continue;
			}
		}

		/// 갯수별 옵션정보
		sSetAblityOption* opt = new sSetAblityOption;
		opt->mOptType[0] = optType1;
		opt->mOptValue[0] = optValue1;
		opt->mOptType[1] = optType2;
		opt->mOptValue[1] = optValue2;

		opt->mActIdx = actIdx;

		/// opt정보 셋팅
		setInfo->AddOption( count, opt );
	}

	return true;
}

cItemSetInfo* cItemManager::GetItemSetInfo( unsigned int setIndex ) const
{
	return (cItemSetInfo*)mSetInfoMap.GetAt(setIndex);
}


// cItemManager Constructor.
cItemManager::cItemManager(void)
{
	mpItemManager           = this;

	mItemDefine             = NULL;
	mItemDefineOffset       = 0;
	mItemDefineLength       = 0;

	mItemAbility            = NULL;
	mItemAbilityOffset      = 0;
	mItemAbilityLength      = 0;

	mItemCooltime2          = NULL;
	mItemCooltime2Offset    = 0;
	mItemCooltime2Length    = 0;

	mItemLimit              = NULL;
	mItemLimitOffset        = 0;
	mItemLimitLength        = 0;

	mItemCard               = NULL;
	mItemCardOffset         = 0;
	mItemCardLength         = 0;

	mItemCardSlot           = NULL;
	mItemCardSlotOffset     = 0;
	mItemCardSlotLength     = 0;

	mItemTarot              = NULL;
	mItemTarotOffset        = 0;
	mItemTarotLength        = 0;

	mItemEnhanced           = NULL;
	mItemEnhancedOffset     = 0;
	mItemEnhancedLength     = 0;

	mItemEnhancedRate       = NULL;
	mItemEnhancedRateOffset = 0;
	mItemEnhancedRateLength = 0;

	mItemDisjoint           = NULL;
	mItemDisjointOffset     = 0;
	mItemDisjointLength     = 0;

	mItemChange             = NULL;
	mItemChangeOffset       = 0;
	mItemChangeLength       = 0;

	mItemDefineMap          = NULL;

	mItemDropIdx            = 0;
	mLastDropIdx            = 0;
}

// ~cItemManager Destructor.
cItemManager::~cItemManager(void)
{
	FreeItemDefine( );
	FreeItemAbility( );
	FreeItemCooltime2( );
	FreeItemLimit( );
	FreeItemCard( );
	FreeItemCardSlot( );
	FreeItemTarot( );
	FreeItemEnhanced( );
	FreeItemEnhancedRate( );
	FreeItemDisjoint( );
	FreeItemChange( );


	{
		cItemSetMap::cIterator i = mSetInfoMap.Begin();
		cItemSetMap::cIterator iend = mSetInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemSetInfo*)(i->mSecond);
		}
		mSetInfoMap.Clear();
	}

    SAFE_DELETE( mpItemScript );

	mpItemManager = NULL;
}

// Init Method
bool cItemManager::Init( )
{
    mpItemScript = new cItemScript;
    if( !mpItemScript->Init() )
    {
        return false;
    }

	if( LoadItemSet( "./Script/Resource/Item_Set.txt" ) == false )
	{
		assert( 0 && "failed to load item set info" );
		return false;
	}

	return true;
}

// AllocItemDefine Method
void cItemManager::AllocItemDefine(DWORD length)
{
	FreeItemDefine( );

	mItemDefine       = (TB_ITEM_DEFINE*)GlobalAlloc( GPTR, sizeof(TB_ITEM_DEFINE) * length );
	mItemDefineOffset = 0;
	mItemDefineLength = length;

	mItemDefineMap = new ItemDefineMap( mItemDefineLength );
}

// FreeItemDefine Method
void cItemManager::FreeItemDefine( )
{
	if ( mItemDefine != NULL )
	{
		GlobalFree( mItemDefine );
		mItemDefine = NULL;
	}
	mItemDefineOffset  = 0;
	mItemDefineLength  = 0;

	if ( mItemDefineMap != NULL )
	{
		delete mItemDefineMap;
		mItemDefineMap = NULL;
	}
}

// IsItemDefineIndex Method
void cItemManager::AllocItemAbility(DWORD length)
{
	FreeItemAbility( );

	mItemAbility       = (TB_ITEM_ABILITY*)GlobalAlloc( GPTR, sizeof(TB_ITEM_ABILITY) * length );
	mItemAbilityOffset = 0;
	mItemAbilityLength = length;
}

// IsItemDefineIndex Method
void cItemManager::FreeItemAbility( )
{
	if ( mItemAbility != NULL )
	{
		GlobalFree( mItemAbility );
		mItemAbility = NULL;
	}
	mItemAbilityOffset  = 0;
	mItemAbilityLength  = 0;
}

// AllocItemCooltime2 Method
void cItemManager::AllocItemCooltime2(DWORD length)
{
	FreeItemCooltime2( );

	mItemCooltime2       = (TB_ITEM_COOLTIME2*)GlobalAlloc( GPTR, sizeof(TB_ITEM_COOLTIME2) * length );
	mItemCooltime2Offset = 0;
	mItemCooltime2Length = length;
}

// FreeItemCooltime2 Method
void cItemManager::FreeItemCooltime2( )
{
	if ( mItemCooltime2 != NULL )
	{
		for ( DWORD i = 0; i < mItemCooltime2Offset; i++ )
		{
			TB_ITEM_COOLTIME1* itemCooltime1 = (mItemCooltime2 + i)->cooltime1;
			TB_ITEM_COOLTIME1* next;
			while ( itemCooltime1 != NULL )
			{
				next = itemCooltime1->next;
				GlobalFree( itemCooltime1 );
				itemCooltime1 = next;
			}
		}
		GlobalFree( mItemCooltime2 );
		mItemCooltime2 = NULL;
	}
	mItemCooltime2Offset = 0;
	mItemCooltime2Length = 0;
}

// AllocItemLimit Method
void cItemManager::AllocItemLimit(DWORD length)
{
	FreeItemLimit( );

	mItemLimit       = (TB_ITEM_LIMIT*)GlobalAlloc( GPTR, sizeof(TB_ITEM_LIMIT) * length );
	mItemLimitOffset = 0;
	mItemLimitLength = length;
}

// FreeItemLimit Method
void cItemManager::FreeItemLimit( )
{
	if ( mItemLimit != NULL )
	{
		GlobalFree( mItemLimit );
		mItemLimit = NULL;
	}
	mItemLimitOffset = 0;
	mItemLimitLength = 0;
}

// AllocItemCard Method
void cItemManager::AllocItemCard(DWORD length)
{
	FreeItemCard( );

	mItemCard       = (TB_ITEM_CARD*)GlobalAlloc( GPTR, sizeof(TB_ITEM_CARD) * length );
	mItemCardOffset = 0;
	mItemCardLength = length;
}

// FreeItemCard Method
void cItemManager::FreeItemCard( )
{
	if ( mItemCard != NULL )
	{
		GlobalFree( mItemCard );
		mItemCard = NULL;
	}
	mItemCardOffset = 0;
	mItemCardLength = 0;
}

// AllocItemCard Method
void cItemManager::AllocItemCardSlot(DWORD length)
{
	FreeItemCardSlot( );

	mItemCardSlot       = (TB_ITEM_CARD_SLOT*)GlobalAlloc( GPTR, sizeof(TB_ITEM_CARD_SLOT) * length );
	mItemCardSlotOffset = 0;
	mItemCardSlotLength = length;
}

// FreeItemCardSlot Method
void cItemManager::FreeItemCardSlot( )
{
	if ( mItemCardSlot != NULL )
	{
		GlobalFree( mItemCardSlot );
		mItemCardSlot = NULL;
	}
	mItemCardSlotOffset = 0;
	mItemCardSlotLength = 0;
}

// IsItemDefineIndex Method
void cItemManager::AllocItemTarot(DWORD length)
{
	FreeItemTarot( );

	mItemTarot       = (TB_ITEM_TAROT*)GlobalAlloc( GPTR, sizeof(TB_ITEM_TAROT) * length );
	mItemTarotOffset = 0;
	mItemTarotLength = length;
}

// IsItemDefineIndex Method
void cItemManager::FreeItemTarot( )
{
	if ( mItemTarot != NULL )
	{
		GlobalFree( mItemTarot );
		mItemTarot = NULL;
	}
	mItemTarotOffset  = 0;
	mItemTarotLength  = 0;
}

// AllocItemEnhanced Method
void cItemManager::AllocItemEnhanced(DWORD length)
{
	FreeItemEnhanced( );

	mItemEnhanced       = (TB_ITEM_ENHANCED*)GlobalAlloc( GPTR, sizeof(TB_ITEM_ENHANCED) * length );
	mItemEnhancedOffset = 0;
	mItemEnhancedLength = length;
}

// FreeItemEnhanced Method
void cItemManager::FreeItemEnhanced( )
{
	if ( mItemEnhanced != NULL )
	{
		GlobalFree( mItemEnhanced );
		mItemEnhanced = NULL;
	}
	mItemEnhancedOffset  = 0;
	mItemEnhancedLength  = 0;
}

// AllocItemEnhancedRate Method
void cItemManager::AllocItemEnhancedRate(DWORD length)
{
	FreeItemEnhancedRate( );

	mItemEnhancedRate       = (TB_ITEM_ENHANCED_RATE*)GlobalAlloc( GPTR, sizeof(TB_ITEM_ENHANCED_RATE) * length );
	mItemEnhancedRateOffset = 0;
	mItemEnhancedRateLength = length;
}

// FreeItemEnhancedRate Method
void cItemManager::FreeItemEnhancedRate( )
{
	if ( mItemEnhancedRate != NULL )
	{
		GlobalFree( mItemEnhancedRate );
		mItemEnhancedRate = NULL;
	}
	mItemEnhancedRateOffset  = 0;
	mItemEnhancedRateLength  = 0;
}

// AllocItemDisjoint Method
void cItemManager::AllocItemDisjoint(DWORD length)
{
	FreeItemDisjoint( );

	mItemDisjoint       = (TB_ITEM_DISJOINT*)GlobalAlloc( GPTR, sizeof(TB_ITEM_DISJOINT) * length );
	mItemDisjointOffset = 0;
	mItemDisjointLength = length;
}

// FreeItemDisjoint Method
void cItemManager::FreeItemDisjoint( )
{
	if ( mItemDisjoint != NULL )
	{
		GlobalFree( mItemDisjoint );
		mItemDisjoint = NULL;
	}
	mItemDisjointOffset  = 0;
	mItemDisjointLength  = 0;
}

// AllocItemChange Method
void cItemManager::AllocItemChange(DWORD length)
{
	FreeItemChange( );

	mItemChange       = (TB_ITEM_CHANGE*)GlobalAlloc( GPTR, sizeof(TB_ITEM_CHANGE) * length );
	mItemChangeOffset = 0;
	mItemChangeLength = length;
}

// FreeItemChange Method
void cItemManager::FreeItemChange( )
{
	if ( mItemChange != NULL )
	{
		GlobalFree( mItemChange );
		mItemChange = NULL;
	}
	mItemChangeOffset  = 0;
	mItemChangeLength  = 0;
}

// IsItemDefineIndex Method
bool cItemManager::IsItemDefineIndex(long idx)
{
	unsigned long i = (idx - 1);
	return (i < mItemDefineLength);
}

// IsItemDefineType Method
bool cItemManager::IsItemDefineType(long idx, ITEM_TYPE type)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( idx );
	if ( itemDefine != NULL )
	{
		return (itemDefine->type == type);
	}
	return false;
}

// IsItemDefineType Method
bool cItemManager::IsItemDefineType(long idx, ITEM_TYPE type, ITEM_TYPE subType)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( idx );
	if ( itemDefine != NULL )
	{
		return (itemDefine->type == type && itemDefine->subType == subType);
	}
	return false;
}

// GetItemDefineType Method
ITEM_TYPE cItemManager::GetItemDefineType(long idx)
{
	unsigned long i = (idx - 1);

	if ( i < mItemDefineLength )
		return (ITEM_TYPE)mItemDefine[ i ].type;

	return ITEM_UNKNOWN;
}

// GetItemDefineSubType Method
ITEM_TYPE cItemManager::GetItemDefineSubType(long idx)
{
	unsigned long i = (idx - 1);

	if ( i < mItemDefineLength )
		return (ITEM_TYPE)mItemDefine[ i ].subType;

	return ITEM_UNKNOWN;
}

// GetItemDefine Method
TB_ITEM_DEFINE* cItemManager::GetItemDefine(long idx)
{
	unsigned long i = (idx - 1);
	if ( i < mItemDefineLength )
	{
		return (mItemDefine + i);
	}
	return NULL;
}

// GetItemAbility Method
TB_ITEM_ABILITY* cItemManager::GetItemAbility(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxAbility - 1);
		if ( i < mItemAbilityLength )
		{
			return (mItemAbility + i);
		}
	}
	return NULL;
}

// GetItemCooltime2 Method
TB_ITEM_COOLTIME2* cItemManager::GetItemCooltime2(long itemDefineIdx)
{
	TB_ITEM_ABILITY* itemAbility = GetItemAbility( itemDefineIdx );
	if ( itemAbility != NULL )
	{
		unsigned long i = (itemAbility->cool_time2 - 1);
		if ( i < mItemCooltime2Length )
		{
			return (mItemCooltime2 + i);
		}
	}
	return NULL;
}

// GetItemLimit Method
TB_ITEM_LIMIT* cItemManager::GetItemLimit(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxLimit - 1);
		if ( i < mItemLimitLength )
		{
			return (mItemLimit + i);
		}
	}
	return NULL;
}

// GetItemCard Method
TB_ITEM_CARD* cItemManager::GetItemCard(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxCard - 1);
		if ( i < mItemCardLength )
		{
			return (mItemCard + i);
		}
	}
	return NULL;
}

// GetItemCardSlot Method
TB_ITEM_CARD_SLOT* cItemManager::GetItemCardSlot(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxCardSlot - 1);
		if ( i < mItemCardSlotLength )
		{
			return (mItemCardSlot + i);
		}
	}
	return NULL;
}

// GetItemTarot Method
TB_ITEM_TAROT* cItemManager::GetItemTarot(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxTarot - 1);
		if ( i < mItemTarotLength )
		{
			return (mItemTarot + i);
		}
	}
	return NULL;
}

// GetItemEnhanced Method
TB_ITEM_ENHANCED* cItemManager::GetItemEnhanced(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxEnhanced - 1);
		if ( i < mItemEnhancedLength )
		{
			return (mItemEnhanced + i);
		}
	}
	return NULL;
}

// GetItemEnhancedRate Method
TB_ITEM_ENHANCED_RATE* cItemManager::GetItemEnhancedRate(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxEnhancedRate - 1);
		if ( i < mItemEnhancedRateLength )
		{
			return (mItemEnhancedRate + i);
		}
	}
	return NULL;
}

// GetItemDisjoint Method
TB_ITEM_DISJOINT* cItemManager::GetItemDisjoint(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxDisjoint - 1);
		if ( i < mItemDisjointLength )
		{
			return (mItemDisjoint + i);
		}
	}
	return NULL;
}

// GetItemChange Method
TB_ITEM_CHANGE* cItemManager::GetItemChange(long itemDefineIdx)
{
	TB_ITEM_DEFINE* itemDefine = GetItemDefine( itemDefineIdx );
	if ( itemDefine != NULL )
	{
		unsigned long i = (itemDefine->idxChange - 1);
		if ( i < mItemChangeLength )
		{
			return (mItemChange + i);
		}
	}
	return NULL;
}

// SetItemDefine Method - 'TB_ITEM_DEFINE::IDX'는 1부터 시작한다.
void cItemManager::SetItemDefine(TB_ITEM_DEFINE* itemDefine)
{
	TB_ITEM_DEFINE* table = (mItemDefine + (itemDefine->idx - 1));

	(*table) = (*itemDefine);

	// 오류검사 해야됨.
	mItemDefineMap->Insert( table->index, table );
	mItemDefineOffset++;
}

// SetItemAbility Method - 'TB_ITEM_ABILITY::IDX'는 1부터 시작한다.
void cItemManager::SetItemAbility(TB_ITEM_ABILITY* itemAbility)
{
	TB_ITEM_ABILITY* table = (mItemAbility + (itemAbility->idx - 1));
	(*table) = (*itemAbility);
	mItemAbilityOffset++;
}

// SetItemCooltime2 Method - 'TB_ITEM_COOLTIME2::IDX'는 1부터 시작한다.
void cItemManager::SetItemCooltime2(long idx, long rowCount, TB_ITEM_COOLTIME1* itemCooltime1)
{
	TB_ITEM_COOLTIME2* itemCoolTime2 = (mItemCooltime2 + (idx - 1));
	TB_ITEM_COOLTIME1* table         = itemCoolTime2->cooltime1;
	TB_ITEM_COOLTIME1* prev          = NULL;

	itemCoolTime2->idx = idx;

	if ( rowCount > 0 )
	{
		itemCoolTime2->cooltime1 = (TB_ITEM_COOLTIME1*)GlobalAlloc( GPTR, sizeof(TB_ITEM_COOLTIME1) );
		table = itemCoolTime2->cooltime1;

		table->next            = NULL;
		table->itemDefineIdx   = itemCooltime1->itemDefineIdx;
		table->itemDefineIndex = itemCooltime1->itemDefineIndex;
		table->cooltime        = itemCooltime1->cooltime;
		prev = table;

		itemCooltime1++;
	}
	for ( long i = 1; i < rowCount; i++ )
	{
		prev->next             = (TB_ITEM_COOLTIME1*)GlobalAlloc( GPTR, sizeof(TB_ITEM_COOLTIME1) );
		table                  = prev->next;

		table->itemDefineIdx   = itemCooltime1->itemDefineIdx;
		table->itemDefineIndex = itemCooltime1->itemDefineIndex;
		table->cooltime        = itemCooltime1->cooltime;
		prev = table;

		itemCooltime1++;
	}

	mItemCooltime2Offset++;
}

// SetItemLimit Method - 'TB_ITEM_LIMIT::IDX'는 1부터 시작한다.
void cItemManager::SetItemLimit(TB_ITEM_LIMIT* itemLimit)
{
	TB_ITEM_LIMIT* table = (mItemLimit + (itemLimit->idx - 1));
	(*table) = (*itemLimit);
	mItemLimitOffset++;
}

// SetItemCard Method - 'TB_ITEM_CARD::IDX'는 1부터 시작한다.
void cItemManager::SetItemCard(TB_ITEM_CARD* itemCard)
{
	TB_ITEM_CARD* table = (mItemCard + (itemCard->idx - 1));
	(*table) = (*itemCard);
	mItemCardOffset++;
}

// SetItemCardSlot Method - 'TB_ITEM_CARD_SLOT::IDX'는 1부터 시작한다.
void cItemManager::SetItemCardSlot(TB_ITEM_CARD_SLOT* itemCardSlot)
{
	TB_ITEM_CARD_SLOT* table = (mItemCardSlot + (itemCardSlot->idx - 1));
	(*table) = (*itemCardSlot);
	mItemCardSlotOffset++;
}

// SetItemTarot Method - 'TB_ITEM_TAROT::IDX'는 1부터 시작한다.
void cItemManager::SetItemTarot(TB_ITEM_TAROT* itemTarot)
{
	TB_ITEM_TAROT* table = (mItemTarot + (itemTarot->idx - 1));
	(*table) = (*itemTarot);
	mItemTarotOffset++;
}

// SetItemEnhanced Method - 'TB_ITEM_ENHANCED::IDX'는 1부터 시작한다.
void cItemManager::SetItemEnhanced(TB_ITEM_ENHANCED* itemEnhanced)
{
	TB_ITEM_ENHANCED* table = (mItemEnhanced + (itemEnhanced->idx - 1));
	(*table) = (*itemEnhanced);
	mItemEnhancedOffset++;
}

// SetItemEnhancedRate Method - 'TB_ITEM_ENHANCED_RATE::IDX'는 1부터 시작한다.
void cItemManager::SetItemEnhancedRate(TB_ITEM_ENHANCED_RATE* itemEnhancedRate)
{
	TB_ITEM_ENHANCED_RATE* table = (mItemEnhancedRate + (itemEnhancedRate->idx - 1));
	(*table) = (*itemEnhancedRate);
	mItemEnhancedRateOffset++;
}

// SetItemDisjoint Method - 'TB_ITEM_DISJOINT::IDX'는 1부터 시작한다.
void cItemManager::SetItemDisjoint(TB_ITEM_DISJOINT* itemDisjoint)
{
	TB_ITEM_DISJOINT* table = (mItemDisjoint + (itemDisjoint->idx - 1));
	(*table) = (*itemDisjoint);
	mItemDisjointOffset++;
}

// SetItemChange Method - 'TB_ITEM_CHANGE::IDX'는 1부터 시작한다.
void cItemManager::SetItemChange(TB_ITEM_CHANGE* itemChange)
{
	TB_ITEM_CHANGE* table = (mItemChange + (itemChange->idx - 1));
	(*table) = (*itemChange);
	mItemChangeOffset++;
}

// MakeItemIdx Method - unsigned long (ULONG_MAX=4,294,967,295) 이상일경우, 중복검사 후 발급.
unsigned long cItemManager::MakeItemIdx( )
{
	++mItemDropIdx;

	if ( mItemDropIdx > mLastDropIdx )
	{
		mLastDropIdx = mItemDropIdx;
	}
	else
	{
		if ( mItemDropIdx == 0 )
			mItemDropIdx = 1;

		while ( OBJECTMANAGER->GetItem( mItemDropIdx ) )
		{
			++mItemDropIdx;
		}
	}

	return mItemDropIdx;
}
