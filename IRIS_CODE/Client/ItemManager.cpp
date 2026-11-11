#include "stdafx.h"
#include "ItemManager.h"
#include "TarotManager.h"
#include "OptionManager.h"

#include "UIText.h"
#include "Gauge.h"

#include "GameUIManager.h"
#include "DragWindow.h"
#include "QuickManager.h"
#include "CoolTimeManager.h"
#include "GameUIManager.h"
#include "UIMsgboxEvent.h"
#include "LootingWindow.h"
#include "HeroInfoWindow.h"
#include "UserSellWindow.h"
#include "CardBookWindow.h"
#include "TarotSheetWindow.h"
#include "InventoryWindow.h"
#include "BagWindow.h"
#include "WareHouseWindow.h"
#include "EnhancedWindow.h"
#include "TradeWindow.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealSearchSheet.h"
#include "NpcDealWindow.h"
#include "NpcStoreWindow.h"
#include "TutorialManager.h"
#include "NumberEditBox.h"
#include "MegaphoneWindow.h"
#include "CashFreeMoveWindow.h"
#include "ThemeStateWindow.h"
#include "SummonWindow.h"
#include "CamaelWindow.h"
//#include "CertifyWIndow.h"

#include "ObjectManager.h"
#include "hero.h"

#include "SceneManager.h"
#include "DropItemSceneNode.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "Application.h"
#include "Npc_Common.h"
#include "Trade_Common.h"
#include "ItemMixManager.h"
#include "ItemMixWindow.h"
#include "PVPManager.h"
#include "SoundSystem.h"
#include "QuestManager.h"
#include "TitleManager.h"
#include "MakeSkillScript.h"
#include "DuelManager.h"

#define ITEM_FILENAME_FILENAME		"./script/resource/item_filename.txt"
#define ITEM_SETINFO_FILENAME		"./script/resource/item_set.txt"
#define ITEM_DEFINE_FILENAME		"./script/resource/item_define.txt"
#define ITEM_ABILITY_FILENAME		"./script/resource/item_ability.txt"
#define ITEM_LIMIT_FILENAME			"./script/resource/item_limit.txt"
#define ITEM_CARD_FILENAME			"./script/resource/item_card.txt"
#define ITEM_TAROT_FILENAME			"./script/resource/item_tarot.txt"
#define ITEM_SPREAD_FILENAME		"./script/resource/item_spread.txt"
#define ITEM_NPCSTORE_FILENAME		"./script/resource/item_npcstore.txt"
#define ITEM_DISJOINT_FILENAME		"./script/resource/item_disjoint.txt"
#define ITEM_ENHANCED_FILENAME		"./script/resource/item_enhanced.txt"
#define ITEM_CARDSLOT_FILENAME		"./script/resource/item_card_slot.txt"
#define ITEM_DEALCATEGORY_FILENAME	"./script/resource/item_dealcategorytable.txt"
#define ITEM_ONEPIECEINFO_FILENAME	"./script/resource/item_onepiece.txt"

cItemManager* cItemManager::mSingleton = 0;

cItemManager::cItemManager()
: mDropItemMap( 200 )
, mLocked( false )
, mWeightGauge( 0 )
, mMaxWeight( 0.0f )
, mCurWeight( 0.0f )
, mLinkedIcon( 0 )
, mLinkColor( NiColorA::WHITE )
, mTradeTarget(0)
, mTradeMoney(0)
, mUserSellTarget(0)
, mLastAccutime(0)
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mItemText   = 0;
    mpNpcStore  = NULL;

	/// 플랙 배열 초기화
	for( unsigned int i = MIN_INVENTORY; i < LEN_INVENTORY; ++i )
	{
		mFlagArray[i] = ITEM_FLAG_ALL;
	}

	mFlagArray[INVENTORY_WEAR_HAT] = ITEM_FLAG_HEAD;
	mFlagArray[INVENTORY_WEAR_BODY1] = ITEM_FLAG_UPPER;
	mFlagArray[INVENTORY_WEAR_BODY2] = ITEM_FLAG_LOWER;
	mFlagArray[INVENTORY_WEAR_HAND] = ITEM_FLAG_HANDS;
	mFlagArray[INVENTORY_WEAR_FOOT] = ITEM_FLAG_FEET;

	mFlagArray[INVENTORY_WEAR_EARRING] = ITEM_FLAG_EARRING;
	mFlagArray[INVENTORY_WEAR_NECKLACE] = ITEM_FLAG_NECKLACE;
	mFlagArray[INVENTORY_WEAR_BROOCH] = ITEM_FLAG_BROOCH;
	mFlagArray[INVENTORY_WEAR_BRACELET] = ITEM_FLAG_BRACELET;
	mFlagArray[INVENTORY_WEAR_RING] = ITEM_FLAG_RING;

	mFlagArray[INVENTORY_WEAR_CARD1] = ITEM_FLAG_EQUIPCARD;
	mFlagArray[INVENTORY_WEAR_CARD2] = ITEM_FLAG_EQUIPCARD;
	mFlagArray[INVENTORY_WEAR_CARD3] = ITEM_FLAG_EQUIPCARD;

	mFlagArray[INVENTORY_HAND_LEFT1]    = ITEM_FLAG_SHIELD;
	mFlagArray[INVENTORY_HAND_RIGHT1]   = ITEM_FLAG_SWORD | ITEM_FLAG_BLADE | ITEM_FLAG_CUTTER | ITEM_FLAG_GUN | ITEM_FLAG_STAFF | ITEM_FLAG_DUAL;
	mFlagArray[INVENTORY_HAND_LEFT2]    = ITEM_FLAG_SHIELD;
	mFlagArray[INVENTORY_HAND_RIGHT2]   = ITEM_FLAG_SWORD | ITEM_FLAG_BLADE | ITEM_FLAG_CUTTER | ITEM_FLAG_GUN | ITEM_FLAG_STAFF | ITEM_FLAG_DUAL;

	mFlagArray[INVENTORY_COSTUME_HAT]   = ITEM_FLAG_COSTUME_HEAD;
	mFlagArray[INVENTORY_COSTUME_BODY1] = ITEM_FLAG_COSTUME_UPPER;
	mFlagArray[INVENTORY_COSTUME_BODY2] = ITEM_FLAG_COSTUME_LOWER;
	mFlagArray[INVENTORY_COSTUME_HAND]  = ITEM_FLAG_COSTUME_HANDS;
    mFlagArray[INVENTORY_COSTUME_FOOT]  = ITEM_FLAG_COSTUME_FEET;
    mFlagArray[INVENTORY_COSTUME_FACE]  = ITEM_FLAG_COSTUME_FACE;
    mFlagArray[INVENTORY_COSTUME_BACK]  = ITEM_FLAG_COSTUME_BACK;

	/// 네트워크 정보
	mSlotIndex0 = USHRT_MAX;
	mSlotIndex1 = USHRT_MAX;
	mValue = 0;
	mTempIndex = 0;
	mActiveWeapon = ItemActiveMax;

	mFrontLeft.Set( 0, 0, 0, 0 );
	mFrontRight.Set( 0, 0, 0, 0 );
	mBackLeft.Set( 0, 0, 0, 0 );
	mBackRight.Set( 0, 0, 0, 0 );

	mTry = ITEM_TRY_NULL;
	mPrice = 0;
}

cItemManager::~cItemManager()
{
	mSingleton = 0;

	delete mItemText;

	/// 정의
	{
		cDefineMap::cIterator i = mDefineMap.Begin();
		cDefineMap::cIterator iend = mDefineMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemDefine*)(i->mSecond);
		}
		mDefineMap.Clear();
	}

	/// 능력
	{
		cAbilityMap::cIterator i = mAbilityMap.Begin();
		cAbilityMap::cIterator iend = mAbilityMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemAbility*)(i->mSecond);
		}
		mAbilityMap.Clear();
	}

	/// 제한
	{
		cLimitMap::cIterator i = mLimitMap.Begin();
		cLimitMap::cIterator iend = mLimitMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemLimit*)(i->mSecond);
		}
		mLimitMap.Clear();
	}

	/// 카드
	{
		cCardMap::cIterator i = mCardMap.Begin();
		cCardMap::cIterator iend = mCardMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemCard*)(i->mSecond);
		}
		mCardMap.Clear();
	}

	/// 타로
	{
		cTarotMap::cIterator i = mTarotMap.Begin();
		cTarotMap::cIterator iend = mTarotMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemTarot*)(i->mSecond);
		}
		mTarotMap.Clear();
	}

	/// 스프레드
	{
		cSpreadMap::cIterator i = mSpreadMap.Begin();
		cSpreadMap::cIterator iend = mSpreadMap.End();

		for( ; i != iend; ++i )
		{
			delete (cItemSpread*)(i->mSecond);
		}
		mSpreadMap.Clear();
	}

	/// 파일명
	{
		cFileNamesMap::cIterator i = mFileNamesMap.Begin();
		cFileNamesMap::cIterator iend = mFileNamesMap.End();

		for( ; i != iend; ++i )
		{
			delete [] ((cString*)(i->mSecond));
		}
		mFileNamesMap.Clear();
	}
	
	/// 분해 데이터
	{
		cDisjointMap::cIterator i = mDisjointMap.Begin();
		cDisjointMap::cIterator end = mDisjointMap.End();

		for( ; i != end; ++i )
		{	
			delete (sItemDisjoint*)(i->mSecond);
		}
		mDisjointMap.Clear();
	}	

	/// 강화 데이터
	{
		cEnhancedMap::cIterator i = mEnhancedMap.Begin();
		cEnhancedMap::cIterator end = mEnhancedMap.End();

		for( ; i != end; ++i )
		{
			cEnhancedOptionArr* optionArr = (cEnhancedOptionArr*)(i->mSecond);
			cEnhancedOptionArr::cIterator   i2 = optionArr->Begin();
			cEnhancedOptionArr::cIterator end2 = optionArr->End();
			for( ; i2 != end2; ++i2 )
			{
				delete (sItemEnhanced*)(*i2);
			}
			optionArr->Clear();

			delete optionArr;
		}
		
		mEnhancedMap.Clear();
	}

	/// 카드장착 데이터
	{
		cPutCardSlotMap::cIterator i = mPutCardSlotMap.Begin();
		cPutCardSlotMap::cIterator end = mPutCardSlotMap.End();

		for( ; i != end; ++i )
		{
			delete (sItemPutCardSlot*)(i->mSecond);
		}
		mPutCardSlotMap.Clear();
	}

	/// 
	{
		cSetInfoMap::cIterator i = mSetInfoMap.Begin();
		cSetInfoMap::cIterator end = mSetInfoMap.End();

		for( ; i != end; ++i )
		{
			delete (cItemSetInfo*)(i->mSecond);
		}
		mSetInfoMap.Clear();
	}

	{
		cItemOnpieceInfo::cIterator i = mItemOnePieceInfo.Begin();
		cItemOnpieceInfo::cIterator end = mItemOnePieceInfo.End();

		for( ; i != end; ++i )
		{
			delete (sItemOnePieceInfo*)(i->mSecond);
		}
		mItemOnePieceInfo.Clear();

	}

	//
	ClearItemCount();

	/// Npc 상점
    SAFE_DELETE( mpNpcStore );

	/// 아이템 판매 분류
	{
		mDealHighArr.Clear();

		cNameIdxArrMap::cIterator b = mDealCategoryMap.Begin();
		cNameIdxArrMap::cIterator end = mDealCategoryMap.End();
		for( ; b != end; ++b )
		{
			cDealArr* arr = (cDealArr*)(*b).mSecond;
			if( arr )
			{
				cDealArr::cIterator ai = arr->Begin();
				cDealArr::cIterator aend = arr->End();
				for( ; ai != aend; ++ai )
				{
					sDealCategory* data = (sDealCategory*)(*ai);
					SAFE_DELETE(data);
				}
				arr->Clear();
			}
			SAFE_DELETE( arr );
		}
		mDealCategoryMap.Clear();
	}

	// 아이템 캐쉬시간정보 삭제
	{
		cItemBillMap::cIterator i = mItemBillMap.Begin();
		cItemBillMap::cIterator end = mItemBillMap.End();
		for( ; i != end; ++i )
		{
			sItemBillEx* bill = (sItemBillEx*)(i->mSecond);
			SAFE_DELETE( bill );
		}

		mItemBillMap.Clear();
	}

	mDropItemMap.Clear();
}

bool cItemManager::Init()
{
	mItemText = new cUIText;

	cStringW path;
	path.Format( L"./language/%s/Item_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( LoadText( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load item text" );
		return false;
	}

	if( LoadFileName( ITEM_FILENAME_FILENAME ) == false )
	{
		assert( 0 && "failed to load item file name" );
		return false;
	}

	if( LoadAbility( ITEM_ABILITY_FILENAME ) == false )
	{
		assert( 0 && "failed to load item ability" );
		return false;
	}

	if( LoadLimit( ITEM_LIMIT_FILENAME ) == false )
	{
		assert( 0 && "failed to load item limit" );
		return false;
	}

	if( LoadCard( ITEM_CARD_FILENAME ) == false )
	{
		assert( 0 && "failed to load item card" );
		return false;
	}

	if( LoadTarot( ITEM_TAROT_FILENAME ) == false )
	{
		assert( 0 && "failed to load item tarot" );
		return false;
	}

	if( LoadSpread( ITEM_SPREAD_FILENAME ) == false )
	{
		assert( 0 && "failed to load item spread" );
		return false;
	}

	if( LoadDisjoint( ITEM_DISJOINT_FILENAME ) == false )
	{
		assert( 0 && "failed to load item disjoint" );
		return false;
	}

	if( LoadEnhanced( ITEM_ENHANCED_FILENAME ) == false )
	{
		assert( 0 && "failed to load item enhanced" );
		return false;
	}

	if( LoadCardSlot( ITEM_CARDSLOT_FILENAME ) == false )
	{
		assert( 0 && "failed to load item card slot" );
		return false;
	}

	if( LoadItemSet( ITEM_SETINFO_FILENAME ) == false )
	{
		assert( 0 && "failed to load item set info" );
		return false;
	}

	/// 위의 데이터를 참조하므로 가장 나중에 로드한다
	if( LoadDefine( ITEM_DEFINE_FILENAME ) == false )
	{
		assert( 0 && "failed to load item define" );
		return false;
	}

	if( LoadItemOnePieceInfo( ITEM_ONEPIECEINFO_FILENAME ) == false )
	{
		assert( 0 && "failed to load item onepiece" );
		return false;
	}

    mpNpcStore = new cItemScript;
    if( mpNpcStore->Init( ) == false )
	{
		assert( 0 && "failed to load npc store" );
		return false;
	}

	if( LoadDealCategoryTable( ITEM_DEALCATEGORY_FILENAME ) == false )
	{
		assert( 0 && "failed to load category table" );
		return false;
	}

	/// 쿨타임에 쿨타임관련 데이터 세팅
	LoadCoolData();
	return true;
}

/// game in 시에 데이터 세팅
void cItemManager::Open()
{
	/// 무기 장착 변경
	SetSwitchWeapon( mActiveWeapon );
}

void cItemManager::Close()
{
	/// 초기화 
	mActiveWeapon = ItemActiveMax;

	/// 총 갯수 세기 초기화
	ClearItemCount();

	//
	mDropItemMap.Clear();

	/// 쿨타임 삭제
	COOLMAN->Clear();

	/// 퀵슬롯 삭제
	QUICKMAN->Clear();

	/// 아이템 모두 삭제
	for( unsigned int i=MIN_INVENTORY; i<=MAX_INVENTORY; ++i )
	{
		mItemArray[i].Clear();
		mItemArray[i] = 0;
	}

	// 아이템 캐쉬시간정보 삭제
	{
		cItemBillMap::cIterator i = mItemBillMap.Begin();
		cItemBillMap::cIterator end = mItemBillMap.End();
		for( ; i != end; ++i )
		{
			sItemBillEx* bill = (sItemBillEx*)(i->mSecond);
			SAFE_DELETE( bill );
		}

		mItemBillMap.Clear();
	}

	mCurWeight = 0.f;
	mMaxWeight = 0.f;
	mLastAccutime = 0;

	/// 거래 사용자 정보 클리어
	mTradeTarget = 0;
	mTradeMoney  = 0;
	mTradeName.Clear();
	mUserSellName.Clear();
	mUserSellTarget = 0;

	/// 아이템 관련 락 풀기
	/// 초기화용
	mLocked = false;
	mTry = ITEM_TRY_NULL;

	if( mSlotIndex0 <= MAX_INVENTORY )
	{
		cItem& item0 = mItemArray[mSlotIndex0];
		item0.SetLock( false );
	}

	if( mSlotIndex1 <= MAX_INVENTORY )
	{
		cItem& item1 = mItemArray[mSlotIndex1];
		item1.SetLock( false );
	}

	mSlotIndex0 = USHRT_MAX;
	mSlotIndex1 = USHRT_MAX;
	mValue = 0;
}

void cItemManager::ClearItemCount()
{
	/// 아이템 총갯수
	cItemCountMap::cIterator i = mItemCountMap.Begin();
	cItemCountMap::cIterator end = mItemCountMap.End();
	for( ; i != end; ++i )
	{
		sItemCount* p = (sItemCount*)(i->mSecond);
		SAFE_DELETE( p );
	}
	mItemCountMap.Clear();
}

bool cItemManager::LoadText( const cStringW& pathName )
{
	if( mItemText->Load( pathName ) == false )
	{
		assert( 0 && "failed to load item text" );
		return false;
	}
	return true;
}

LPCTSTR cItemManager::GetItemName( unsigned long i )
{
	return mItemText->GetText( i ).Cstr();
}

cItemDefine* cItemManager::GetItemDefine( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cDefineMap::cConstIterator i = mDefineMap.Find( itemIndex );

	if( i == mDefineMap.End() )
		return 0;
	else
		return (cItemDefine*)i->mSecond;
}

cItemAbility* cItemManager::GetItemAbility( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cAbilityMap::cConstIterator i = mAbilityMap.Find( itemIndex );

	if( i == mAbilityMap.End() )
		return 0;
	else
		return (cItemAbility*)i->mSecond;
}

cItemSetInfo* cItemManager::GetItemSetInfo( unsigned int setIndex )
{
	return (cItemSetInfo*)mSetInfoMap.GetAt( setIndex );
}

cItemLimit* cItemManager::GetItemLimit( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cLimitMap::cConstIterator i = mLimitMap.Find( itemIndex );

	if( i == mLimitMap.End() )
		return 0;
	else
		return (cItemLimit*)i->mSecond;
}

cItemCard* cItemManager::GetItemCard( unsigned long itemIndex )
{
	cCardMap::cConstIterator i = mCardMap.Find( itemIndex );

	if( i == mCardMap.End() )
		return 0;
	else
		return (cItemCard*)i->mSecond;
}

cItemTarot* cItemManager::GetItemTarot( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cTarotMap::cConstIterator i = mTarotMap.Find( itemIndex );
	
	if( i == mTarotMap.End() )
		return 0;
	else
		return (cItemTarot*)i->mSecond;
}

cItemSpread* cItemManager::GetItemSpread( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cSpreadMap::cConstIterator i = mSpreadMap.Find( itemIndex );

	if( i == mSpreadMap.End() )
		return 0;
	else
		return (cItemSpread*)i->mSecond;
}

cString* cItemManager::GetItemFileNames( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	cFileNamesMap::cConstIterator i = mFileNamesMap.Find( itemIndex );

	if( i == mFileNamesMap.End() )
		return 0;
	else
		return (cString*)i->mSecond;
}

sItemDisjoint* cItemManager::GetItemDisjoint( unsigned long index )
{
	if( index == 0 )
		return 0;

	cDisjointMap::cConstIterator i = mDisjointMap.Find( index );
	
	if( i == mDisjointMap.End() )
		return 0;
	else
		return (sItemDisjoint*)(i->mSecond);
}

cEnhancedOptionArr* cItemManager::GetItemEnhanced( unsigned long index )
{
	if( index == 0 )
		return 0;

	cEnhancedMap::cConstIterator i = mEnhancedMap.Find( index );

	if( i == mEnhancedMap.End() )
		return 0;
	else
		return (cEnhancedOptionArr*)(i->mSecond);
}

sItemPutCardSlot* cItemManager::GetItemPutCardSlot( unsigned long index )
{
	cPutCardSlotMap::cConstIterator i = mPutCardSlotMap.Find( index );

	if( i == mPutCardSlotMap.End() )
		return 0;
	else
		return (sItemPutCardSlot*)(i->mSecond);
}


sItemCount* cItemManager::GetItemCount( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	return (sItemCount*)mItemCountMap.GetAt( itemIndex );
}

sItemOnePieceInfo* cItemManager::GetItemOnePieceInfo( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	return (sItemOnePieceInfo*)mItemOnePieceInfo.GetAt( itemIndex );
}

const char* cItemManager::GetItemFileName( unsigned long itemFileIdx, char race, char gender, bool weapon )
{
	assert( race < eRACE_MAX );
	assert( gender < eGENDER_MAX );

	cString* strs = GetItemFileNames( itemFileIdx );
	if( strs == 0 )
		return 0;
	else
	{
		if( strs[race * eGENDER_MAX + gender] == "None" )
		{
			if( weapon )
				return strs[0].Cstr();
			return 0;
		}
	}

	return strs[race * eGENDER_MAX + gender].Cstr();
}


/// 아이템인덱스로 슬롯 번호 찾기 
unsigned short cItemManager::FindSlotIndex( unsigned long itemIndex )
{
	for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= INVENTORY_QUEST_END; ++slot )
	{
		cItem& item = mItemArray[slot];
		if( item.mIndex > 0 && item.mIndex == itemIndex )
			return slot;
	}
	return USHRT_MAX;
}

/// 현재 착용중인지를 검사한다.
bool cItemManager::IsEquipedItem( unsigned long itemIndex )
{
	cItem& leftHand = mItemArray[INVENTORY_HAND_LEFT2];
	if( leftHand.mIndex > 0 && leftHand.mIndex == (unsigned long)itemIndex )
	{
		return ( CheckLimit( itemIndex ) == ITEMLIMIT_SUCCESS );
	}

	cItem& rightHand = mItemArray[INVENTORY_HAND_RIGHT2];
	if( rightHand.mIndex > 0 && rightHand.mIndex == (unsigned long)itemIndex )
	{
		return ( CheckLimit( itemIndex ) == ITEMLIMIT_SUCCESS );
	}

	for( unsigned int slot = INVENTORY_WEAR_BEGIN; slot <= INVENTORY_WEAR_END; ++slot )
	{
		cItem& item = mItemArray[slot];
		if( item.mIndex > 0 && item.mIndex == (unsigned long)itemIndex )
		{
			return ( CheckLimit( itemIndex ) == ITEMLIMIT_SUCCESS );
		}
	}
	return false;
}


/// 서브 타입으로 슬롯 번호 찾기
unsigned short cItemManager::FindSlotIndexByType( ITEM_TYPE mainType, ITEM_TYPE subType )
{
	for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= INVENTORY_QUEST_END; ++slot )
	{
		cItem& item = mItemArray[slot];
		if( item.mIndex > 0 && item.mDefine->GetType() == mainType && item.mDefine->GetSubType() == subType )
			return slot;
	}
	return USHRT_MAX;
}

/// 인벤, 가방중 한개의 윈도우만 선택된 아이콘을 갖도록 한다
void cItemManager::SetInvenSelect( cUIWindow* win )
{
	cInventoryWindow* inven = GAMEUI->GetInventoryWindow();
	cBagWindow* bag1 = GAMEUI->GetBagWindow(0);
	cBagWindow* bag2 = GAMEUI->GetBagWindow(1);
	cBagWindow* bag3 = GAMEUI->GetBagWindow(2);

	cWareHouseWindow*	 wareHouse = GAMEUI->GetWareHouseWindow();
	cWareHouseBagWindow* wareBag1  = GAMEUI->GetWareHouseBagWindow(0);
	cWareHouseBagWindow* wareBag2  = GAMEUI->GetWareHouseBagWindow(1);
	cWareHouseBagWindow* wareBag3  = GAMEUI->GetWareHouseBagWindow(2);
	
	if( inven && inven != win )
		inven->HideSelect();
	if( bag1 && bag1 != win )
		bag1->HideSelect();
	if( bag2 && bag2 != win )
		bag2->HideSelect();
	if( bag3 && bag3 != win )
		bag3->HideSelect();

	if( wareHouse && wareHouse != win )
		wareHouse->HideSelect();
	if( wareBag1 && wareBag1 != win )
		wareBag1->HideSelect();
	if( wareBag2 && wareBag2 != win )
		wareBag2->HideSelect();
	if( wareBag3 && wareBag3 != win )
		wareBag3->HideSelect();
}

void cItemManager::ChangeSelectSlot( unsigned short slot )
{
	/// UI 갱신
	eItemIn in = GetItemIn( slot );
	if( in == ITEM_IN_INVEN_PRIVATE || in == ITEM_IN_EQUIP )
	{
		/// 인벤
		if( slot <= INVENTORY_BAG0_END )
		{
			cInventoryWindow* inven = GAMEUI->GetInventoryWindow();
			if( inven )
				inven->ChangeSelect( slot );
		}
		/// 가방1
		else if( slot >= INVENTORY_BAG1_BEGIN && slot <= INVENTORY_BAG1_END )
		{
			cBagWindow* bag1 = GAMEUI->GetBagWindow(0);
			if( bag1 )
				bag1->ChangeSelect( slot );
		}
		/// 가방2
		else if( slot >= INVENTORY_BAG2_BEGIN && slot <= INVENTORY_BAG2_END )
		{
			cBagWindow* bag2 = GAMEUI->GetBagWindow(1);
			if( bag2 )
				bag2->ChangeSelect( slot );
		}
		/// 가방3
		else if( slot >= INVENTORY_BAG3_BEGIN && slot <= INVENTORY_BAG3_END )
		{
			cBagWindow* bag3 = GAMEUI->GetBagWindow(2);
			if( bag3 )
				bag3->ChangeSelect( slot );
		}
		else
		{
			assert(0);
		}
	}
	else if( in == ITEM_IN_WAREHOUSE_PRIVATE )
	{
		if( slot >= INVENTORY_WAREHOUSE0_BEGIN && slot <= INVENTORY_WAREHOUSE0_END )
		{
			 cWareHouseWindow* ware = GAMEUI->GetWareHouseWindow();
			 if( ware )
				 ware->ChangeSelect( slot );
		}
		else if( slot >= INVENTORY_WAREHOUSE1_BEGIN && slot <= INVENTORY_WAREHOUSE1_END )
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow(0);
			if( bag )
				bag->ChangeSelect( slot );
		}
		else if( slot >= INVENTORY_WAREHOUSE2_BEGIN && slot <= INVENTORY_WAREHOUSE2_END )
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow(1);
			if( bag )
				bag->ChangeSelect( slot );
		}
		else if( slot >= INVENTORY_WAREHOUSE3_BEGIN && slot <= INVENTORY_WAREHOUSE3_END )
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow(2);
			if( bag )
				bag->ChangeSelect( slot );
		}
	}
}

eItemIn cItemManager::GetItemIn( unsigned short i )
{
	if( i >= INVENTORY_EQUIP_BEGIN && i <= INVENTORY_EQUIP_END )
		return ITEM_IN_EQUIP;
	else if( i >= INVENTORY_BAG_BEGIN && i <= INVENTORY_BAG_END )
		return ITEM_IN_INVEN_PRIVATE;
	else if( i >= INVENTORY_QUEST_BEGIN && i <= INVENTORY_QUEST_END )
		return ITEM_IN_INVEN_QUEST;
	else if( i >= INVENTORY_WAREHOUSE0_BEGIN && i <= INVENTORY_WAREHOUSE3_END )
		return ITEM_IN_WAREHOUSE_PRIVATE;
	else if( i >= NPCSTORE_SLOT_BEGIN && i <= NPCSTORE_SLOT_END )
		return ITEM_IN_NPCSTORE;
	else if( i == PUTCARD_ITEM_SLOT )
		return ITEM_IN_PUTCARD_ITEMSLOT;
	else if( i >= PUTCARD_CARD_SLOT_BEGIN && i <= PUTCARD_CARD_SLOT_END )
		return ITEM_IN_PUTCARD_CARDSLOT;
	else if( i >= INVENTORY_CASH_ITEM_BEGIN && i <= INVENTORY_CASH_ITEM_END )
		return ITEM_IN_RECEIPT;
	else if( i >= INVENTORY_TAROT_NORMAL_BEGIN && i <= INVENTORY_TAROT_NORMAL_END )
		return ITEM_IN_CARDBOOK_NORMAL;
	else if( i >= INVENTORY_SPREAD_NORMAL_BEGIN && i <= INVENTORY_SPREAD_NORMAL_END )
		return ITEM_IN_CARDBOOK_NORMAL_SPREAD;
	else
		return ITEM_IN_NULL;
}

/// 쿨타임 세팅
void cItemManager::StartCoolTime( unsigned long itemIndex, unsigned long coolTime )
{
	// 일반
	for( unsigned int slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
	{
		cItem& item = mItemArray[slot];

		if( item.mIndex == itemIndex )
		{
			if( item.StartCoolTime( coolTime ) == false )
			{
				assert(0);
				continue;
			}
		}
	}

	QUICKMAN->LoadCoolTime();
	QUICKMAN->LoadCoolTimeMon();
}

void cItemManager::EndCoolTime( unsigned long itemIndex )
{
	// 인벤
	for( unsigned int slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
	{
		cItem& item = mItemArray[slot];

		if( item.mIndex == itemIndex )
			item.EndCoolTime();
	}

	QUICKMAN->LoadCoolTime();
	QUICKMAN->LoadCoolTimeMon();
}

/// 쿨타임복사
void cItemManager::CopyCoolTime( unsigned long itemIndex, unsigned long startTime, unsigned long restTime, unsigned long totalTime )
{
	// 인벤
	for( unsigned int slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
	{
		cItem& item = mItemArray[slot];

		if( item.mIndex == itemIndex )
			item.CopyCoolTime( startTime, restTime, totalTime );
	}
}

/// 게임접속시, 쿨타임 매니저로부터 모든 아이템의 쿨타임을 복사한다 
void cItemManager::SetAllItemCool()
{
	// 인벤
	for( unsigned int slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
	{
		cItem& item = mItemArray[slot];
		if( item.mIndex > 0 )
			COOLMAN->CopyCoolTime( item.mIndex );
	}
}

void cItemManager::BindIcon( unsigned short i, cItemIcon* icon )
{
	if( icon && i <= MAX_INVENTORY )
	{
		/// 아이콘 바인딩
		mItemArray[i].BindIcon( icon );

		///
		icon->SetSlotIndex( i );

		/// 퀘스트용 아이템이면 이동불가 처리
		eItemIn in = GetItemIn( i );
		if( in == ITEM_IN_INVEN_QUEST || in == ITEM_IN_CARDBOOK_NORMAL_SPREAD )
		{
			mItemArray[i].SetUsedFlag( false, false );
		}
	}
	else
		assert(0);
}

/// 백업용
void cItemManager::LinkIcon( cItemIcon* icon )
{
	if( icon )
	{
		if( mLinkedIcon == icon )
			return;

		if( mLinkedIcon )
		{
			mLinkedIcon->SetColor( NiColorA::WHITE );
			assert(0);
		}

		mLinkColor = icon->GetColor();

		NiColorA check;
		mLinkedIcon = icon;
		mLinkedIcon->ShowGauge( false );
		mLinkedIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
	}
	else
		assert(0);
}

/// 백업용
void cItemManager::UnLinkIcon( bool setcolor )
{
	if( mLinkedIcon )
	{
		mLinkedIcon->ShowGauge( true );

		if( setcolor )
			mLinkedIcon->SetColor( mLinkColor );

		mLinkedIcon = 0;
	}
}

bool cItemManager::FindEmptySlot( unsigned short* slotIndex, unsigned short begin, unsigned short end )
{
	for( unsigned short i = begin; i <= end; ++i )
	{
		if( mItemArray[i].mDBIndex == 0 )
		{
			*slotIndex = i;
			return true;
		}
	}
	return false;
}

bool cItemManager::FindEmptySlotArr( unsigned short* slotIndexArr, unsigned int count, unsigned short begin, unsigned short end )
{
	unsigned int cnt = 0;
	for( unsigned short i = begin; i <= end; ++i )
	{
		if( mItemArray[i].mDBIndex == 0 )
		{
			*slotIndexArr = i;
			slotIndexArr++;
			cnt++;

			if( cnt == count )
				return true;
		}
	}
	return false;
}

bool cItemManager::TryToMoveItem( unsigned short slotIndex0, unsigned short slotIndex1, cItemIcon* icon )
{
	assert( slotIndex0 <= MAX_INVENTORY && "index out of range" );
	assert( slotIndex1 <= MAX_INVENTORY && "index out of range" );
	assert( slotIndex0 != slotIndex1 );

	if( IsLocked() || slotIndex0 == slotIndex1 )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH || stopFlag == eSTOP_ENHANCED || 
			stopFlag == eSTOP_ITEMMIX || stopFlag == eSTOP_SKILLRESET || stopFlag == eSTOP_MAPCHANGE )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	/// 이동 가능 여부를 검사
	eItemIn in0 = GetItemIn( slotIndex0 );
	eItemIn in1 = GetItemIn( slotIndex1 );

	assert( in0 != ITEM_IN_NULL );
	assert( in1 != ITEM_IN_NULL );

	///
	cItem& item0 = mItemArray[slotIndex0];
	cItem& item1 = mItemArray[slotIndex1];
	cItemDefine* itemDefine = item0.mDefine;
	if( !itemDefine )
	{
		assert(0);
		return false;
	}

	if( slotIndex0 == INVENTORY_HAND_LEFT2 || 
		slotIndex0 == INVENTORY_HAND_RIGHT2 ||
		slotIndex1 == INVENTORY_HAND_LEFT2 ||
		slotIndex1 == INVENTORY_HAND_RIGHT2 )
	{
		/// 스킬 동작중 장비교체 불가
        if( HERO->IsReadyUseSkill() == false )
			return false;		
	}

	unsigned short count = item0.mCount;

	/// 장착 -> 인벤 개인
	if( in0 == ITEM_IN_EQUIP && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		if( item1.mSeal == true )
			return false;

		if( item1.mCount && (item1.mFlag & mFlagArray[slotIndex0] ) != item1.mFlag )
			return false;
		if( TestMoveItemDown( slotIndex0, slotIndex1 ) == false )
			return false;
	}
	/// 인벤 개인 -> 장착
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_EQUIP )
	{
		if( item0.mSeal == true )
			return false;

		/// 사용 제약조건 체크
		unsigned long textIndex = 0;
		int ret0 = CheckLimit( item0.GetIndex() );
		switch( ret0 )
		{
		case ITEMLIMIT_RACE:		textIndex = 280; break;
		case ITEMLIMIT_GENDER:		textIndex = 281; break;
		case ITEMLIMIT_JOB:			textIndex = 282; break;
		case ITEMLIMIT_LEVEL:		textIndex = 283; break;
		case ITEMLIMIT_PVPONLY:		textIndex = 284; break;
		case ITEMLIMIT_TUTORIALONLY:textIndex = 291; break;
		case ITEMLIMIT_FIELDONLY:    textIndex = 12400; break;
		case ITEMLIMIT_FORCETYPE:	textIndex = 285; break;
		case ITEMLIMIT_MAKESKILL:	textIndex = 286; break;
		case ITEMLIMIT_MAKESKILLEXP: textIndex = 292; break;
		case ITEMLIMIT_MIXSKILLSTEP:textIndex = 288; break;
		case ITEMLIMIT_MIXRECIPESTEP:textIndex = 289; break;
		case ITEMLIMIT_MIXRECIPEEXP: textIndex = 290; break;
		case ITEMLIMIT_ERROR:		assert(0);		textIndex = 239; break;
		}

		if( ret0 != ITEMLIMIT_SUCCESS   )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );
			return false;
		}

		//
		int ret1 = CheckLimit( item1.GetIndex() );
		switch( ret1 )
		{
		case ITEMLIMIT_RACE:		textIndex = 280; break;
		case ITEMLIMIT_GENDER:		textIndex = 281; break;
		case ITEMLIMIT_JOB:			textIndex = 282; break;
		case ITEMLIMIT_LEVEL:		textIndex = 283; break;
		case ITEMLIMIT_PVPONLY:		textIndex = 284; break;
		case ITEMLIMIT_TUTORIALONLY:textIndex = 291; break;
		case ITEMLIMIT_FIELDONLY:    textIndex = 12400; break;
		case ITEMLIMIT_FORCETYPE:	textIndex = 285; break;
		case ITEMLIMIT_MAKESKILL:	textIndex = 286; break;
		case ITEMLIMIT_MAKESKILLEXP: textIndex = 292; break;
		case ITEMLIMIT_MIXSKILLSTEP:textIndex = 288; break;
		case ITEMLIMIT_MIXRECIPESTEP:textIndex = 289; break;
		case ITEMLIMIT_MIXRECIPEEXP: textIndex = 290; break;
		case ITEMLIMIT_ERROR:		assert(0);		textIndex = 239; break;
		}

		if( ret1 != ITEMLIMIT_SUCCESS )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );
			return false;
		}
		if( item0.mEquipType == ITEM_EQUIP_TWO_HANDS )
		{
			if( slotIndex1 == INVENTORY_HAND_LEFT1 )
				slotIndex1 = INVENTORY_HAND_RIGHT1;
			else if( slotIndex1 == INVENTORY_HAND_LEFT2 )
				slotIndex1 = INVENTORY_HAND_RIGHT2;
		}
		if( item0.mFlag == ITEM_FLAG_NULL || item0.mFlag == ITEM_FLAG_UNKNOWN )
			return false;
		if( item0.mCount && (item0.mFlag & mFlagArray[slotIndex1] ) != item0.mFlag )
			return false;
		if( TestMoveItemUp( slotIndex0, slotIndex1 ) == false )
			return false;
	}
	/// 인벤 개인 -> 인벤 개인
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in0 == in1 )
	{
		/// 스킵
	}
	/// 창고 개인 -> 창고 개인
	else if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in0 == in1 )
	{
		/// 스킵
	}
	/// 창고 -> 인벤
	else if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		/// 무게체크
		float wei = mCurWeight + (itemDefine->GetWeight() * count);
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}
	}
	/// 아이템수령함 -> 인벤
	else if( in0 == ITEM_IN_RECEIPT && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		if( item1.mCount )
		{
            return false;
        }

        /// 보유가능 개수 체크
        if( itemDefine->GetMaxHaveInven() > 0 )
        {
            sItemCount* p = ITEMMAN->GetItemCount( item0.GetIndex() );
            unsigned int invenCount = ( p ) ? p->total : 0;

            if( invenCount + item0.GetCount() > itemDefine->GetMaxHaveInven() )
            {
			    UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				    eMBEVENT_NONE, eSKIN_OK,
				    GAMERESOURCEMAN->GetGameText( 220 ),
				    GAMERESOURCEMAN->GetGameText( 301 ) );
                return false;
            }
        }

		/// 무게체크
		float wei = mCurWeight + (itemDefine->GetWeight() * count);
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}
	}
	/// 인벤 -> 창고
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_WAREHOUSE_PRIVATE )
	{
	}
	/// 인벤 -> 카드북
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_CARDBOOK_NORMAL )
	{
	}
	/// 카드북 -> 인벤
	else if( in0 == ITEM_IN_CARDBOOK_NORMAL && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		// 빈자리 아니고 동일 아이템 아니면 리턴
		if( item1.mCount > 0 )
		{
			if( item0.GetIndex() != item1.GetIndex() )
				return false;
		}

		/// 보유가능 개수 체크
		if( itemDefine->GetMaxHaveInven() > 0 )
		{
			sItemCount* p = ITEMMAN->GetItemCount( item0.GetIndex() );
			unsigned int invenCount = ( p ) ? p->total : 0;

			if( invenCount + item0.GetCount() > itemDefine->GetMaxHaveInven() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 220 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				return false;
			}
		}

		/// 무게체크
		float wei = mCurWeight + (itemDefine->GetWeight() * count);
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}
	}
	else
		return false;

	/// 네트워크 메시지 전송
	if( (in0 == ITEM_IN_INVEN_PRIVATE && in0 == in1) ||
		(in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_EQUIP) ||
		(in0 == ITEM_IN_EQUIP && in1 == ITEM_IN_INVEN_PRIVATE) ||
		(in0 == ITEM_IN_WAREHOUSE_PRIVATE && in0 == in1) ||
		(in0 == ITEM_IN_RECEIPT && in1 == ITEM_IN_INVEN_PRIVATE ) || 
		(in0 == ITEM_IN_CARDBOOK_NORMAL && in1 == ITEM_IN_INVEN_PRIVATE ) ||
		(in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_CARDBOOK_NORMAL ) )
	{
		MSG_REQ_ITEM_MOV_INVENTORY msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_ITEM;
		msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
		msg.number1 = slotIndex0;
		msg.number2 = slotIndex1;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	/// 인벤 개인 <-> 창고 개인
	else if( (in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_WAREHOUSE_PRIVATE) ||
		(in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE) )
	{
		MSG_REQ_NPC_ITEM_MOV2_INVENTORY msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_NPC;
		msg.Protocol = NM_NPC_ITEM_MOV2_INVENTORY_REQ;
		msg.npcIdx = HERO->GetNpcIdx();
		msg.number1 = slotIndex0;
		msg.number2 = slotIndex1;
		msg.count = count;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	else
	{
		assert( 0 );
		return false;
	}

	/// 잠금
	LockAndBackup( ITEM_TRY_MOVE, slotIndex0, slotIndex1, 0, 0, 0, icon );
	return true;
}

/// 스킬 초기화 아이템 사용
bool cItemManager::TryToUseSkillReset( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	MSG_REQ_PLAYER_SKILLRESET msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_SKILLRESET_REQ;
	msg.mSlotNum = slotIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 상태 고정
	HERO->SetStopFlag( eSTOP_SKILLRESET );

	/// 잠금
	LockAndBackup( ITEM_TRY_SKILLRESET, slotIndex );

	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_SKILLRESETPROCESS, eSKIN_NONE,
						GAMERESOURCEMAN->GetGameText( 265 ), 
						GAMERESOURCEMAN->GetGameText( 301 )
						);
	return true;
}

/// 경험치 복구 물약 사용
bool cItemManager::TryToUseExpRecover( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	/// 사용
	MSG_REQ_ITEM_USE_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category	= NM_ITEM;
	msg.Protocol	= NM_ITEM_USE_INVENTORY_REQ;	
	msg.targetType	= HERO->GetObjectType();
	msg.targetIndex = HERO->GetObjectID();
	msg.number		= slotIndex;
	
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_USE, slotIndex );

	return true;
}

/// 캐시 아이템 사용
bool cItemManager::TryToUseCashItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	/// 사용
	MSG_REQ_ITEM_USE_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category	= NM_ITEM;
	msg.Protocol	= NM_ITEM_USE_INVENTORY_REQ;	
	msg.targetType	= HERO->GetObjectType();
	msg.targetIndex = HERO->GetObjectID();
	msg.number		= slotIndex;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_USE, slotIndex );

	return true;
}

bool cItemManager::TryToUseInventory( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	/// 사용
	MSG_REQ_ITEM_USE_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category    = NM_ITEM;
	msg.Protocol    = NM_ITEM_USE_INVENTORY_REQ;

	const sObject* target = HERO->GetTargetInfo();
	if( target )
	{
		/// 타겟이 none이면 기본으로 hero를 타겟으로 세팅
		msg.targetType = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectType() : target->type;
		msg.targetIndex = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectID() : target->index;

		msg.number = slotIndex;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 잠금
		LockAndBackup( ITEM_TRY_USE, slotIndex );
	}

	return true;
}

/// 아이템 사용
bool cItemManager::TryToEquipItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( !(stopFlag > eSTOP_NONE && stopFlag <= eSTOP_NPCSPEECH) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	/// 제한 슬롯 체크
	unsigned short bagEnd = HERO->GetBagEndSlot();
	if ( slotIndex > bagEnd )
	{
		assert( 0 && "Bad slot index" );
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 3101 ) );
		return false;
	}

	/// 장착
	cItem& item = mItemArray[slotIndex];
	cItemDefine* itemDefine = item.mDefine;
	if( !itemDefine )
		return false;

	// 인증 검사
	if( item.mSeal == true )
	{
		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText( 219 ), itemDefine->GetName() );
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_ITEM_SEAL, eSKIN_YESNO,
							(LPCTSTR)str.Cstr(),
							GAMERESOURCEMAN->GetGameText( 301 ),
							(void*)slotIndex
							);
		return false;
	}

	/// 사용 제약조건 체크
	unsigned long textIndex = 0;
	int ret = CheckLimit( item.GetIndex() );
	switch( ret )
	{
	case ITEMLIMIT_RACE:		textIndex = 280; break;
	case ITEMLIMIT_GENDER:		textIndex = 281; break;
	case ITEMLIMIT_JOB:			textIndex = 282; break;
	case ITEMLIMIT_LEVEL:		textIndex = 283; break;
	case ITEMLIMIT_PVPONLY:		textIndex = 284; break;
	case ITEMLIMIT_TUTORIALONLY:textIndex = 291; break;
	case ITEMLIMIT_FIELDONLY:    textIndex = 12400; break;
	case ITEMLIMIT_FORCETYPE:	textIndex = 285; break;
	case ITEMLIMIT_MAKESKILL:	textIndex = 286; break;
	case ITEMLIMIT_MAKESKILLEXP: textIndex = 292; break;
	case ITEMLIMIT_MIXSKILLSTEP:textIndex = 288; break;
	case ITEMLIMIT_MIXRECIPESTEP:textIndex = 289; break;
	case ITEMLIMIT_MIXRECIPEEXP: textIndex = 290; break;
	case ITEMLIMIT_ERROR:		assert(0);		textIndex = 239; break;
	}

	if( ret != ITEMLIMIT_SUCCESS )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );
		return false;
	}

	// 추가 제약조건 체크
	ret = CheckLimitEtc( item.GetIndex() );
	if( ret == ITEMLIMIT_HAVETITLE )
	{
		textIndex = 287;
	}

	if( ret != ITEMLIMIT_SUCCESS )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );
		return false;
	}

	unsigned char type = itemDefine->GetType();
	unsigned char subType = itemDefine->GetSubType();
	unsigned short i1 = 0;

	/// 거래등으로 LOCK 걸려있으면 사용가능한 다른아이템 찾는다
	if( item.IsLock() == true && (type == ITEM_ETC1 || type == ITEM_MONSTER || (type == ITEM_CARD && subType == ITEM_CARD_COMMON )) )
	{
		unsigned short findSlot = 0;
		for( unsigned short i = INVENTORY_BAG0_BEGIN; i <= INVENTORY_BAG_END; ++i )
		{	
			cItem& otherItem = mItemArray[i];
			if( otherItem.mIndex > 0 && otherItem.mIndex == item.mIndex )
			{
				/// 다른 슬롯의 동일한 아이템 있는지 찾기
				if( i != slotIndex && item.mSeal == otherItem.mSeal )
				{
					findSlot = i;
					break;
				}
			}
		}
	
		/// 못찾으면 리턴
		if( findSlot > 0 )
			slotIndex = findSlot;
		else
			return false;
	}
	/// 그 외아이템이 lock이면 리턴
	else if( item.IsLock() == true )
		return false;

	switch( type )
	{
	case ITEM_WEAPON:
		if( TestEquipWeapon( i1, item.mEquipType ) == false )
			return false;
		break;
	case ITEM_WEAR:
		if( TestEquipWear( i1, subType ) == false )
			return false;
		break;
	case ITEM_ACCESSORY:
		if( TestEquipAccessory( i1, subType ) == false )
			return false;
		break;
	case ITEM_ETC1:
		{
			switch( subType )
			{
			case ITEM_ETC1_QUEST:			// 퀘스트
				{
					cItemAbility* itemAbil = itemDefine->GetAbililty();
					if( itemAbil )
					{
						if( item.IsCoolTime() )	
							return false;

						if( itemAbil->GetInfluenceIndex() == 0 )
							return false;
					}
					else
					{
						assert(0);
						return false;
					}
				}
				break;
			case ITEM_ETC1_CHANGE:			// 체인지
				{
					///무게 체크
					if( mCurWeight >= mMaxWeight )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12403 ) );
						return false;
					}

					// 빈슬롯 체크
					if( item.mCount > 1 )
					{
						// 체인지 아이템이 겹쳐져 있을경우는 빈슬롯 없으면 리턴
						unsigned short slotIndex1 = 0;
						if( FindEmptySlot( &slotIndex1, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 294 ) );
							return false;
						}
					}
				}
				break;
			case ITEM_ETC1_SKILLRESET:		// 스킬 초기화
			case ITEM_ETC1_COOLTIME_CASH:	// 버프성 소모 캐시
			case ITEM_ETC1_EXP_RESTORE:			
				break;
			case ITEM_ETC1_TITLE:			// 호칭
				{
					// 호칭 에러
					if( itemDefine->GetTitleIndex() == 0 )
						return false;

					cTitleDefine* titleDefine = TITLEMAN->GetTitleDefine( itemDefine->GetTitleIndex() );
					if( titleDefine )
					{
						/// 타이틀 검사
						if( TITLEMAN->IsHaveTitle( itemDefine->GetTitleIndex() ) == true )
						{
							CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 68 ) );
							return false;
						}	
					}
					else
					{
						assert(0);
						return false;
					}
				}
				break;
			case ITEM_ETC1_THEME_RESET:		// 테마 던전 귀속 초기화
				{
					cThemeResetWindow* win = GAMEUI->GetThemeResetWindow();
					if( win )
						win->Show( slotIndex );	

					return true;
				}
				break;
			case ITEM_ETC1_FREE_MOVE:		// 자유 이동
				{
					if( TestUseItem( slotIndex ) == false )
						return false;

					// 케릭터 상태 체크
					if( HERO->GetState() != eOBJECT_STATE_IDLE || HERO->GetStopFlag() != eSTOP_NONE )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12200 ) );
						return false;
					}

					// 사용가능한 맵인지 체크
					unsigned short heroMapIndex = HERO->GetMapNum();
					if( ( heroMapIndex < MAP_MIN || heroMapIndex > MAP_MAX ) && ( heroMapIndex < MAP_THEME_MIN || heroMapIndex > MAP_THEME_MAX ) )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12201 ) );
						return false;
					}

					if( DUELMAN->IsDuelEnable() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(809) );
						return false;
					}

					if( PVPMAN->IsEnableBattle() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(842) );
						return false;
					}

					if( HERO->IsJumping() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText(12207) );
						return false;
					}

					/// 자동사냥 신고 창이 뜬 도중이라면 무시
					cUIWindow* camaelWin = (cUIWindow*)GAMEUI->GetCamaelWindow();
					if( camaelWin && camaelWin->IsVisible() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11817 ) );
						return false;
					}

					if( HERO->IsWeaponChanging() == true )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 12207 ) );
						return false;
					}


					cCashFreeMoveWindow* cashFreeMoveWin = GAMEUI->GetCashFreeMoveWindow();
					if( cashFreeMoveWin )
						cashFreeMoveWin->Show( slotIndex );

					return true;
				}
				break;
			case ITEM_ETC1_SHOUT:			// 전체 외치기
				{
					if( TestUseItem( slotIndex ) == false )
						return false;

					cMegaphoneWindow* win = GAMEUI->GetMegaphoneWindow();
					if( win )
						win->Show( slotIndex, item.mDBIndex );

					return true;
				}	
				break;
			default:
				{
					if( TestUseItem( slotIndex ) == false )
						return false;
				}
				break;
			}
		}
		break;
	case ITEM_MATERIAL:
		if( subType == ITEM_MATERIAL_MAP )
		{
			if( TestUseItem( slotIndex ) == false )
				return false;
		}
		break;
	case ITEM_SPREAD:
		{
			cCardBookWindow* cardBook = (cCardBookWindow*)GAMEUI->GetCardBookWindow();
			if( cardBook )
				cardBook->Show();
		}
		break;
	case ITEM_CARD:
		if( subType == ITEM_CARD_TAROT )
		{
			cCardBookWindow* cardBook = (cCardBookWindow*)GAMEUI->GetCardBookWindow();
			if( cardBook )
			{
				cItemTarot* tarot = itemDefine->GetTarotDefine();
				if( !tarot )
					return false;

				cTarotSheetWindow* sheet = cardBook->GetTarotSheet();
				if( !sheet )
					return false;

				unsigned short bookslotIndex = sheet->GetSlotIndex( tarot->GetUniqueNumber() );
				if( bookslotIndex != USHRT_MAX )
					GAMEUI->ShowItemCardBook( eMBEVENT_ITEM_CARDBOOK_TAROTNEW, slotIndex, bookslotIndex, 0 );
			}
		}
		else if( subType == ITEM_CARD_COMMON )	/// 소모용 카드이면 사용
		{
			/// 대기상태 아니면 소모용 카드 사용 불가
			if( !( state == eOBJECT_STATE_IDLE || state == ePLAYER_STATE_ITEMPICK ) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 242 ) );
				return false;
			}

			if( TestUseItem( slotIndex ) == false )
				return false;
		}
		else if( subType == ITEM_CARD_EQUIP_C )	/// 캐릭터 장착 카드
		{
			if( TestEquipCard( i1 ) == false )
				return false;
		}
		else if( subType >= ITEM_CARD_EQUIP_A && subType <= ITEM_CARD_EQUIP_O )	// 아이템 장착카드
		{
			cUIWindow* putcard = (cUIWindow*)GAMEUI->GetPutCardWindow();
			if( putcard )
				putcard->Show();
		}
		break;
	case ITEM_MONSTER:
		{
			/// 대기상태 아니면 소모용 카드 사용 불가
			if( !( state == eOBJECT_STATE_IDLE || state == ePLAYER_STATE_ITEMPICK ) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 242 ) );
				return false;
			}

			if( HERO->IsJumping() == true )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 242 ) );
				return false;
			}

			if( TestUseItem( slotIndex ) == false )
				return false;
		}
		break;
	case ITEM_MIX_SKILL:
		{
			if( subType <= ITEM_MIX_SKILL_L1 && subType >= ITEM_MIX_RECIPEGROUP )
			{
				cItemAbility* itemAbil = itemDefine->GetAbililty();
				if( itemAbil )
				{
					if( item.IsCoolTime() )	
						return false;

					if( itemAbil->GetInfluenceIndex() == 0 )
						return false;
				}
				else
				{
					assert(0);
					return false;
				}
			}
		}
		break;
	case ITEM_INVENTORY:
		{
			/// 이미 쿨타임 적용중인지 검사
			if( item.IsCoolTime() == true )
				return false;

			switch( subType )
			{
			case ITEM_INVENTORY_USEFUL_PAPER:	// 편의 기능 소환 주문서
				{
					cSummonWindow* pSummonWin = GAMEUI->GetSummonWindow();
					if( pSummonWin == NULL )
					{
						assert(0);
						return false;
					}

					pSummonWin->Show( slotIndex );

					return true;
				}
				break;
			}
		}
		break;
	default:
		return false;
	}

	/// 네트워크 메시지 전송
	switch( type )
	{
	case ITEM_WEAPON:
	case ITEM_WEAR:
	case ITEM_ACCESSORY:
		{
			MSG_REQ_ITEM_MOV_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
			msg.number1 = slotIndex;
			msg.number2 = i1;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			/// 잠금
			LockAndBackup( ITEM_TRY_EQUIP, slotIndex, i1 );
		}
		break;
	case ITEM_ETC1:
		{
			switch( subType )
			{
			case ITEM_ETC1_QUEST:
				{
					cItemAbility* abil = itemDefine->GetAbililty();
					if( !abil )
					{
						assert(0);
						return false;
					}

					unsigned long questIdx = abil->GetInfluenceIndex();
					cQuestDefine* define = QUESTMAN->GetQuestDefine( questIdx );
					if( !define )
					{
						assert(0);
						return false;
					}

					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 434 ), GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ) );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_QUEST_ADDBYITEM, eSKIN_YESNO,
						(LPCTSTR)str.Cstr(), 
						GAMERESOURCEMAN->GetGameText( 301 ),
						(void*)slotIndex
						);
				}
				break;
			case ITEM_ETC1_CHANGE:
				{
					/// 체인지 아이템
					MSG_REQ_ITEM_CHANGE_INVENTORY msg;
					::memset( &msg, 0, sizeof( msg ) );

					msg.Category = NM_ITEM;
					msg.Protocol = NM_ITEM_CHANGE_INVENTORY_REQ;
					msg.number = slotIndex;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					/// 잠금
					LockAndBackup( ITEM_TRY_CHANGE, slotIndex );
				}
				break;
			case ITEM_ETC1_SKILLRESET:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_SKILLRESET, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 266 ),
						GAMERESOURCEMAN->GetGameText( 301 ),
						(void*)slotIndex
						);
				}
				break;
			case ITEM_ETC1_EXP_RESTORE:
				{				
					sHeroInfo* info = HERO->GetHeroInfo();

					// 복구할 경험치가 없다면
					if( info->mLoseExp == 0 )
					{	
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 11902 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );

						return false;
					}

					cStringT str;
					cStringT recoverExp = cNumberEditBox::FormatNumber( info->mLoseExp );			
					str.Format( GAMERESOURCEMAN->GetGameText( 11901 ), (LPTSTR)recoverExp.Cstr() );

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_EXP_RECOVER, eSKIN_YESNO, 
						(LPTSTR)str.Cstr(), 
						GAMERESOURCEMAN->GetGameText( 301 ), 
						(void*)slotIndex 
						);
				}
				break;
			case ITEM_ETC1_COOLTIME_CASH:
				{
					cStringT msg;
					msg.Format( GAMERESOURCEMAN->GetGameText( 12401 ), itemDefine->GetName() );
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_CASH_ITEM_USE, eSKIN_YESNO,
						msg.Cstr(),
						GAMERESOURCEMAN->GetGameText( 301 ),
						(void*)slotIndex
						);
				}
				break;			
			default:
				{
					/// 사용
					MSG_REQ_ITEM_USE_INVENTORY msg;
					::memset( &msg, 0, sizeof( msg ) );

					msg.Category    = NM_ITEM;
					msg.Protocol    = NM_ITEM_USE_INVENTORY_REQ;

					const sObject* target = HERO->GetTargetInfo();
					if( target )
					{
						/// 타겟이 none이면 기본으로 hero를 타겟으로 세팅
						msg.targetType = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectType() : target->type;
						msg.targetIndex = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectID() : target->index;

						msg.number = slotIndex;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						/// 잠금
						LockAndBackup( ITEM_TRY_USE, slotIndex );
					}
				}
				break;
			}
		}
		break;
	case ITEM_MATERIAL:
		if( subType == ITEM_MATERIAL_MAP )
		{
			cItemAbility* abil = itemDefine->GetAbililty();
			if( abil == 0 )
			{
				assert(0);
				return false;
			}
			HERO->UseMapChangeSkill( abil->GetInfluenceIndex(), slotIndex );
		}
		else if( subType == ITEM_MATERIAL_VEHICLE )
		{
			cItemAbility* abil = itemDefine->GetAbililty();
			if( abil == 0 )
			{
				assert(0);
				return false;
			}
			HERO->UseVehicleSkill( abil->GetInfluenceIndex(), slotIndex );
		}
		break;
	case ITEM_CARD:
		if( subType == ITEM_CARD_COMMON )	/// 소모용 카드이면 사용
		{
			/// 사용
			MSG_REQ_ITEM_USE_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category    = NM_ITEM;
			msg.Protocol    = NM_ITEM_USE_INVENTORY_REQ;

			const sObject* target = HERO->GetTargetInfo();
			if( target )
			{
				/// 타겟이 none이면 기본으로 hero를 타겟으로 세팅
				msg.targetType = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectType() : target->type;
				msg.targetIndex = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectID() : target->index;

				msg.number = slotIndex;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 잠금
				LockAndBackup( ITEM_TRY_USE, slotIndex );
			}
		}
		else if( subType == ITEM_CARD_EQUIP_C )	/// 캐릭터 장착용 카드면
		{
			MSG_REQ_ITEM_MOV_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
			msg.number1 = slotIndex;
			msg.number2 = i1;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			/// 잠금
			LockAndBackup( ITEM_TRY_EQUIP, slotIndex, i1 );
		}
		break;
	case ITEM_MONSTER:
		{
			/// 사용
			MSG_REQ_ITEM_USE_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category    = NM_ITEM;
			msg.Protocol    = NM_ITEM_USE_INVENTORY_REQ;

			const sObject* target = HERO->GetTargetInfo();
			if( target )
			{
				/// 타겟이 none이면 기본으로 hero를 타겟으로 세팅
				msg.targetType = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectType() : target->type;
				msg.targetIndex = ( target->type == eOBJECTTYPE_NONE )? HERO->GetObjectID() : target->index;

				msg.number = slotIndex;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 잠금
				LockAndBackup( ITEM_TRY_USE, slotIndex );
			}
		}
		break;
	case ITEM_MIX_SKILL:
		if( subType >= ITEM_MIX_SKILL_L1 && subType <= ITEM_MIX_SKILL_L4 )
		{
			cItemAbility* itemAbil = itemDefine->GetAbililty();
			if( itemAbil )
			{
				if( ITEMMIXMAN->RegistSkill( slotIndex, item.mDefine->GetType(), item.mDefine->GetSubType(), 
					itemAbil->GetInfluenceIndex() ) == true )
				{
					LockAndBackup( ITEM_TRY_MIXSKILL, slotIndex );
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else if( subType == ITEM_MIX_RECIPE )
		{
			cItemAbility* itemAbil = itemDefine->GetAbililty();
			if( itemAbil )
			{
				if( ITEMMIXMAN->RegistRecipe( slotIndex, itemAbil->GetInfluenceIndex() ) == true )
				{
					/// 잠금
					LockAndBackup( ITEM_TRY_MIXSKILL, slotIndex );
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else if( subType == ITEM_MIX_RECIPEGROUP )
		{
			cItemAbility* itemAbil = item.mDefine->GetAbililty();
			if( itemAbil )
			{
				if( ITEMMIXMAN->RegistRecipeGroup( slotIndex, itemAbil->GetInfluenceIndex() ) == true )
				{
					/// 잠금
					LockAndBackup( ITEM_TRY_MIXSKILL, slotIndex );
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
		break;
	case ITEM_INVENTORY:
		{
			switch( subType )
			{
			case ITEM_INVENTORY_POST_SUMMON:
				{
					MSG_REQ_TRADE_POST_SUMMON_OPEN msg;
					::memset( &msg, 0, sizeof( msg ) );

					msg.Category	= NM_TRADE;
					msg.Protocol	= NM_TRADE_POST_SUMMON_OPEN_REQ;
					msg.number		= slotIndex;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					LockAndBackup( ITEM_TRY_SUMMON, slotIndex );
				}
				break;
			case ITEM_INVENTORY_AGENT_SUMMON:
				{
					MSG_REQ_TRADE_AGENT_SUMMON_OPEN msg;
					::memset( &msg, 0, sizeof( msg ) );

					msg.Category	= NM_TRADE;
					msg.Protocol	= NM_TRADE_AGENT_SUMMON_OPEN_REQ;
					msg.number		= slotIndex;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					LockAndBackup( ITEM_TRY_SUMMON, slotIndex );
				}
				break;
			case ITEM_INVENTORY_WAREHOUSE_SUMMON:
				{
					MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN msg;
					::memset( &msg, 0, sizeof( msg ) );

					msg.Category	= NM_ITEM;
					msg.Protocol	= NM_ITEM_WAREHOUSE_SUMMON_OPEN_REQ;
					msg.number		= slotIndex;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					LockAndBackup( ITEM_TRY_SUMMON, slotIndex );
				}
				break;
			default:
				{
					cStringT msg;
					msg.Format( GAMERESOURCEMAN->GetGameText( 12401 ), itemDefine->GetName() );
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_ITEM_USE_INVENTORY, eSKIN_YESNO,
						msg.Cstr(),
						GAMERESOURCEMAN->GetGameText( 301 ),
						(void*)slotIndex
						);
				}				
				break;
			}		
		}
		break;
	}
	return true;
}

/// 장착->인벤
bool cItemManager::TryToUnEquipItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( !(stopFlag > eSTOP_NONE && stopFlag <= eSTOP_NPCSPEECH) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	/// 탈착
	unsigned short slotIndex1 = 0;
	if( FindEmptySlot( &slotIndex1, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 202 ) );
		return false;
	}

	MSG_REQ_ITEM_MOV_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
	msg.number1 = slotIndex;
	msg.number2 = slotIndex1;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_UNEQUIP, slotIndex, slotIndex1 );
	return true;
}

bool cItemManager::TryToUnSeal( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	cItem& useItem = mItemArray[slotIndex];
	cItemDefine* useDefine = useItem.GetDefine();
	if( !useDefine )
	{
		assert(0);
		return false;
	}
	
	// 인증 풀기 전에 동일 계열 아이템 이미 인증 풀려있는지 검사
	// bag
	for( unsigned int i = INVENTORY_CHARACTER_BEGIN; i <= HERO->GetBagEndSlot(); ++i )
	{
		cItem& item = mItemArray[i];
		cItemDefine* define = item.GetDefine();
		if( !define )
			continue;
		
		// 동일한 인덱스 아이템인 경우,
		if( useItem.GetIndex() == item.GetIndex() )
		{
			if( item.IsSeal() == false )
			{
				// error
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12404 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				return false;
			}
		}
	}

	// warehouse
	for( unsigned int i = INVENTORY_WAREHOUSE_BEGIN; i <= INVENTORY_WAREHOUSE_END; ++i )
	{
		cItem& item = mItemArray[i];
		cItemDefine* define = item.GetDefine();
		if( !define )
			continue;

		// 동일 아이템인 경우,
		if( useItem.GetIndex() == item.GetIndex() )
		{
			if( item.IsSeal() == false )
			{
				// error
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12404 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				return false;
			}
		}
	}
	
	MSG_REQ_ITEM_LICENSE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_LICENSE_REQ;
    msg.number   = slotIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_SEAL, slotIndex );
	return true;
}

bool cItemManager::TryToDumpItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH || stopFlag == eSTOP_ENHANCED || 
			stopFlag == eSTOP_ITEMMIX || stopFlag == eSTOP_SKILLRESET || stopFlag == eSTOP_MAPCHANGE )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	/// 네트워크 메시지 전송
	MSG_REQ_ITEM_DEL_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_DEL_INVENTORY_REQ;
	msg.number = slotIndex;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_DUMP, slotIndex );
	return true;
}

bool cItemManager::TryToSplitItem( unsigned short slotIndex0, unsigned short slotIndex1, unsigned int value )
{
	if( slotIndex0 > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( slotIndex1 > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( value == 0 )
		return false;

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH || stopFlag == eSTOP_ENHANCED || 
			stopFlag == eSTOP_ITEMMIX || stopFlag == eSTOP_SKILLRESET || stopFlag == eSTOP_MAPCHANGE )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	eItemIn in0 = GetItemIn( slotIndex0 );
	eItemIn in1 = GetItemIn( slotIndex1 );

	assert( in0 != ITEM_IN_NULL );
	assert( in1 != ITEM_IN_NULL );

	cItem& item0 = mItemArray[slotIndex0];
	cItem& item1 = mItemArray[slotIndex1];

	if( item0.mCount <= 1 || item0.mCount <= value )
		return false;

	if( item1.mCount )
		return false;

	if( item0.mCash )
		return false;

	if( item0.mSeal )
		return false;

	cUIWindow* inven = GAMEUI->GetInventoryWindow();
	if( !inven )
	{
		assert(0);
		return false;
	}

	cItemDefine* itemDefine = item0.mDefine;
	if( !itemDefine )
	{
		assert(0);
		return false;
	}

	unsigned short capacity = itemDefine->mCapacity;

	if( value > capacity || item1.mCount >= capacity )
		return false;

	/// 인벤 개인 -> 인벤 개인
	if( in0 == ITEM_IN_INVEN_PRIVATE && in0 == in1 )
	{
		/// 스킵
	}
	/// 인벤 개인 -> 인벤 개인
	else if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in0 == in1 )
	{
		/// 스킵
	}
	/// 창고 -> 인벤
	else if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE )
	{
		if( item1.mCount )
			return false;

		/// 슬롯 full 검사 
		unsigned short slotIndex = 0;
		
		FindEmptySlot( &slotIndex, INVENTORY_BAG_BEGIN, HERO->GetBagEndSlot() );
		if( slotIndex == 0 )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 202 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}

		/// 무게체크
		float wei = mCurWeight + (itemDefine->GetWeight() * value);
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}
	}
	/// 인벤 -> 창고
	else if( in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_WAREHOUSE_PRIVATE )
	{
		if( item1.mCount )
			return false;

		unsigned short slotIndex = 0;
		unsigned short wareHouseEnd = HERO->GetWareHouseEndSlot();
		
		FindEmptySlot( &slotIndex, INVENTORY_WAREHOUSE0_BEGIN, wareHouseEnd );
		if( slotIndex == 0 )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 202 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, inven );
			return false;
		}
	}
	else
	{
		return false;
	}

	/// 네트워크 메시지 전송
	/// 인벤 개인 -> 인벤 개인
	if( in0 == ITEM_IN_INVEN_PRIVATE && in0 == in1 )
	{
		if( item0.mCount == 0 )
		{
			MSG_REQ_ITEM_MOV_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
			msg.number1 = slotIndex0;
			msg.number2 = slotIndex1;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		else
		{
			MSG_REQ_ITEM_DIVIDE_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_DIVIDE_INVENTORY_REQ;
			msg.number1 = slotIndex0;
			msg.number2 = slotIndex1;
			msg.divide = (unsigned short)value;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
	}
	/// 인벤 개인 <-> 창고 개인
	else if( (in0 == ITEM_IN_INVEN_PRIVATE && in1 == ITEM_IN_WAREHOUSE_PRIVATE) ||
		(in0 == ITEM_IN_WAREHOUSE_PRIVATE && in1 == ITEM_IN_INVEN_PRIVATE) )
	{
		MSG_REQ_NPC_ITEM_MOV2_INVENTORY msg;
		::memset( &msg, 0, sizeof( msg ) );

		msg.Category = NM_NPC;
		msg.Protocol = NM_NPC_ITEM_MOV2_INVENTORY_REQ;
		msg.npcIdx = HERO->GetNpcIdx();
		msg.number1 = slotIndex0;
		msg.number2 = slotIndex1;
		msg.count = (unsigned short)value;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	}
	/// 창고 개인 <-> 창고 개인
	else if( in0 == ITEM_IN_WAREHOUSE_PRIVATE && in0 == in1 )
	{
		if( item0.mCount == 0 )
		{
			MSG_REQ_ITEM_MOV_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_MOV_INVENTORY_REQ;
			msg.number1 = slotIndex0;
			msg.number2 = slotIndex1;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		else
		{
			MSG_REQ_ITEM_DIVIDE_INVENTORY msg;
			::memset( &msg, 0, sizeof( msg ) );

			msg.Category = NM_ITEM;
			msg.Protocol = NM_ITEM_DIVIDE_INVENTORY_REQ;
			msg.number1 = slotIndex0;
			msg.number2 = slotIndex1;
			msg.divide = (unsigned short)value;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
	}
	else
	{
		assert( 0 );
		return false;
	}

	/// 잠금
	LockAndBackup( ITEM_TRY_SPLIT, slotIndex0, slotIndex1, 0, value );
	return true;
}

bool cItemManager::TryToSwitchWeapon()
{
	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH || stopFlag == eSTOP_ENHANCED || 
			stopFlag == eSTOP_ITEMMIX || stopFlag == eSTOP_SKILLRESET || stopFlag == eSTOP_MAPCHANGE )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	/// 무기 스위칭시 툴팁 제거
	UIMAN->HideTip();

	/// 네트워크 메시지 전송
	MSG_REQ_ITEM_SWI_INVENTORY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_SWI_INVENTORY_REQ;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_SWITCH, 0 );
	return true;
}

/// 아이템 사기 - // NPC 구입
bool cItemManager::TryToBuyItem( unsigned long itemIndex, unsigned int count )
{
	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();

	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_NPCSPEECH) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}
	
	if( count == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 232 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	if( CheckBuyItem( itemIndex, count ) == false )
		return false;

	cNpcStoreWindow* win = GAMEUI->GetNpcStore();
	if( !win )
	{
		assert(0);
		return false;
	}
	else if( win->GetTabIndex() == ULONG_MAX )
	{
		assert(0);
		return false;
	}

	/// 네트워크 메시지 전송
	MSG_REQ_NPC_ITEM_BUY msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_BUY_REQ;
	msg.npcIdx = HERO->GetNpcIdx();
	msg.tabIdx = win->GetTabIndex();
	msg.itemDefineIndex = itemIndex;
	msg.count = (unsigned short)count;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_BUY, 0 );

	cUIWindow* store = (cUIWindow*)GAMEUI->GetNpcStore();
	if( store )
		store->NetLock();

	return true;
}

/// 아이템 줍기
bool cItemManager::TryToGetItem(unsigned long itemIndex, unsigned int count)
{
	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == ePLAYER_STATE_ITEMPICK) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}
	
	cLootingWindow* pLoot = GAMEUI->GetLootingWindow();
	if( !pLoot )
	{
		assert(0);
		return false;
	}

	/// 전체 습득인경우
	if( itemIndex == (unsigned long)ITEM_GET_ALL )
	{
		float wei = mCurWeight + pLoot->GetWeight();
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, pLoot );
			return false;
		}
	}
	/// 개별 습득인 경우
	else
	{
		cItemDefine* define = GetItemDefine( itemIndex );
		if( define )
		{
			/// 1. 소지할수 있는 공간 갯수 체크
			unsigned int remainCnt = GetRemainCount( itemIndex, count );
			if( remainCnt < count )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 202 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, pLoot );
				return false;
			}

			/// 2. max inven 체크 ( 갯수 제한 있을때만 )
			if( define->GetMaxHaveInven() > 0 )
			{
				sItemCount* p = GetItemCount( itemIndex );
				int haveCnt = ( p ) ? p->total : 0;
				if( haveCnt + count > (int)define->GetMaxHaveInven() )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 220 ),
						GAMERESOURCEMAN->GetGameText( 301 ),
						0, pLoot );
					return false;
				}
			}

			/// 3. 무게체크
			float wei = mCurWeight + (define->GetWeight() * count);
			if( wei > mMaxWeight )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 208 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, pLoot );
				return false;
			}
		}
		else
		{
			assert(0);
			return false;
		}
	}

	MSG_REQ_ITEM_GET msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_GET_REQ;
	msg.itemIndexOrInd = itemIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_GET, 0 );

	pLoot->NetLock( eLOCK_GETITEM );
	return true;
}


bool cItemManager::TryToSellItem( unsigned short slotIndex, unsigned int count )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();

	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_NPCSPEECH) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}
	
	if( count == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 233 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		return false;
	}

	/// 판매조건 검사
	cItem& item = mItemArray[slotIndex];
	cItemDefine* itemDefine = item.mDefine;
	if( itemDefine == 0 )
	{
		assert(0);
		return false;
	}

	// 최대 보유금액 검사
	unsigned long itemPrice = count * itemDefine->GetSellPrice();						
	if( itemPrice + HERO->GetMoney() > MAX_HAVE_MONEY )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 11204 ),
			GAMERESOURCEMAN->GetGameText( 301 ) // 알림
			);
		return false;
	}			

	// 퀘스트 용, 판매불가, 미인증 아이템 검사
	if( itemDefine->GetSellType() == ItemSellNone || itemDefine->GetLicenseType() != 0 ||
		itemDefine->GetType() == ITEM_QUEST )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 204 ),
			GAMERESOURCEMAN->GetGameText( 301 ) // 알림
			);
		return false;
	}

	/// 코인 획득시의 무게 측정 검사
	if( itemDefine->GetSellType() > ItemSellTarot )
	{
		cItemDefine* coinDefine = GetItemDefine( itemDefine->GetSellType() );
		if( coinDefine )
		{
			/// 기존무게 + 코인 무게 - 팔려는 아이템 무게
			float weight = (float)(coinDefine->GetWeight() * itemDefine->GetSellPrice()) + mCurWeight;
			weight -= ( itemDefine->GetWeight() * count );

			if( weight > mMaxWeight )
			{
				/// 무게게이지 풀
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 201 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				return false;
			}
		}
		else
		{
			assert(0);
			return false;
		}
	}

	/// 네트워크 메시지 전송
	MSG_REQ_NPC_ITEM_SELL msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category        = NM_NPC;
	msg.Protocol        = NM_NPC_ITEM_SELL_REQ;
	msg.npcIdx			= HERO->GetNpcIdx();
	msg.inventoryNumber = slotIndex;
	msg.count           = (unsigned short)count;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_SELL, slotIndex, 0, 0, count );
	return true;
}

/// 아이템 분해
bool cItemManager::TryToDisjoint( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();

	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_NPCSPEECH) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	MSG_REQ_ITEM_DISJOINT_INVENTORY msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_DISJOINT_INVENTORY_REQ;
	msg.number = slotIndex;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 인벤토리 락
	LockAndBackup( ITEM_TRY_DISJOINT, slotIndex );
	return true;
}

bool cItemManager::TryToEnhancedStart( unsigned short slotIndex0, unsigned short slotIndex1, unsigned short slotIndex2 )
{
	if( IsLocked() )
		return false;

	// 아이템 강화 단계가 10 이상이면
	cItem* cardItem = ITEMMAN->GetItem( slotIndex0 );
	if( cardItem->GetEnhanceStep() >= MAX_ITEM_ENHANCED )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 195 ) );
		return false;		
	}

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	/// 탈것 해제
	if( HERO->IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->AlightVehicle();
	}
	
	/// 따라가기 종료
	if( HERO->IsFollowing() )
		HERO->ReleaseFollowPlayer();

	MSG_REQ_ITEM_ENHANCED_START msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_ENHANCED_START_REQ;
	msg.number1 = slotIndex0;
	msg.number2 = slotIndex1;
	msg.number3 = slotIndex2;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_ENHANCEDSTART, 0 );

	cEnhancedWindow* win = GAMEUI->GetEnhancedWindow();
	if( win )
		win->NetLock();

	HERO->SetStopFlag( eSTOP_ENHANCED );
	return true;
}

bool cItemManager::TryToEnhancedCancel( bool close )
{
	if( HERO->GetStopFlag() == eSTOP_ENHANCED )
	{
		if( IsLocked() )
			return false;

		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_ITEM;
		msg.Protocol = NM_ITEM_ENHANCED_CANCEL_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->SetStopFlag( eSTOP_NONE );
		HERO->ClearItemEnhanceEffect();
	}

	cEnhancedWindow* win = GAMEUI->GetEnhancedWindow();
	if( win )
	{
		win->StopEnhanced();

		if( close == true )
			win->Hide();
	}
	return true;
}

/// 네트워크
bool cItemManager::TryToTradeAsk( cPlayer* player )
{
/*	unsigned char nCertifyState = HERO->GetCertifyState();

	/// 인증 번호를 설정하지 않은 경우
	if( nCertifyState ==  2 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_MAKE_CERTIFY_NUMBER, eSKIN_YESNO,
			GAMERESOURCEMAN->GetGameText( 13104 ),
			GAMERESOURCEMAN->GetGameText( 301 ) 
			);

		return false;
	}
	/// 인증 번호 검사를 한번도 안하였고, 인증 번호 검사 체크를 설정하였을 때
	else if( nCertifyState ==  1 )
	{		
		cCertifyWindow* pCertifyWin = GAMEUI->GetCertifyWindow();
		if( pCertifyWin == NULL )
		{
			assert(0);
			return false;
		}

		pCertifyWin->Show( eCertify_State_Send );

		return false;
	}	
*/

	if( IsLocked() )
		return false;

	if( player == 0 )
		return false;

	/// 상태검사
	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	if( TUTORIALMAN->IsTutorialMode() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11020 ) );
		return false;
	}
	
	/// 저장
	mTradeTarget = player->GetObjectID();
	mTradeName = player->GetName();

	MSG_REQ_ITEM_EXCHANGE_ASK msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_ASK_REQ;
	msg.mTarget.index = player->GetObjectID();
	msg.mTarget.type = player->GetObjectType();
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

/// 승인
void cItemManager::TryToTradeAccept()
{
	MSG_REQ_ITEM_EXCHANGE_REP msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_REP_REQ;
	msg.type = TYPE_ITEM_EXCHANGE_REP_ACCEPT;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2002 ) );
		msg.type = TYPE_ITEM_EXCHANGE_REP_REJECT;
	}
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

/// 거절
void cItemManager::TryToTradeReject( BYTE type )
{
	MSG_REQ_ITEM_EXCHANGE_REP msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_REP_REQ;
	msg.type = type;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

/// 아이템 등록
bool cItemManager::TryToTradeItemAdd( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	/// 거래불가 아이템인지 검사
	cItem& item = mItemArray[slotIndex];
	if( item.mDefine )
	{
		/// 미인증 거래 가능 아이템이면서 실제로 미인증된 아이템이여야만 거래가능. 그 외엔 불가.
		if( item.mDefine->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO || 
			item.mDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO ||
			(item.mDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES && item.mSeal == false) )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2019 ) );
			return false;
		}
		
	}
	else
	{
		assert(0);
		return false;
	}

	/// 아이템 등록 시점 검사
	cTradeWindow* tradeWin = GAMEUI->GetTradeWindow();
	if( tradeWin )
	{
		if( tradeWin->GetStatus() != ItemExchangeBegin )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2024 ) );
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}

	///
	MSG_REQ_ITEM_EXCHANGE_ADD msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_ADD_REQ;
	msg.type = TYPE_ITEM_EXCHANGE_ADD_ITEM;
	msg.number = slotIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 락
	LockAndBackup( ITEM_TRY_TRADEADD, slotIndex );

	tradeWin->NetLock();

	return true;
}

/// 금액 등록
bool cItemManager::TryToTradeMoneyAdd( unsigned long money )
{
	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	mTradeMoney = money;

	MSG_REQ_ITEM_EXCHANGE_ADD msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_ADD_REQ;
	msg.type = TYPE_ITEM_EXCHANGE_ADD_MONEY;
	msg.money = money;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	return true;
}

/// 아이템 삭제
bool cItemManager::TryToTradeItemDelete( unsigned short slotIndex, unsigned int arrIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	///
	MSG_REQ_ITEM_EXCHANGE_DEL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_DEL_REQ;
	msg.number = slotIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 락
	LockAndBackup( ITEM_TRY_TRADEDELETE, 0, 0, arrIndex );

	cUIWindow* trade = (cUIWindow*)GAMEUI->GetTradeWindow();
	if( trade )
		trade->NetLock();
	return true;
}

/// 거래 물품 확인 요청
void cItemManager::TryToTradeOk()
{
	if( IsLocked() )
		return;

	cTradeWindow* trade = GAMEUI->GetTradeWindow();
	if( !trade )
	{
		assert(0);
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2013 ) );
		return;
	}

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return;
	}

	/// 1. 각각의 아이템 검사
	float thatWeight = 0;
	sThatInvetory* thatItem = trade->GetThatItem();
	for( unsigned short i = 0; i < trade->GetThatLastSlot(); ++i, ++thatItem )
	{
		unsigned long itemIndex = thatItem->mThatInv.ItemIndex;
		unsigned int  count		= thatItem->mThatInv.count;

		cItemDefine* define = GetItemDefine( itemIndex );
		if( !define )
		{
			assert(0);
			return;
		}

		if( define->GetMaxHaveInven() > 0 )
		{
			sItemCount* p = GetItemCount( itemIndex );
			int haveCnt = ( p ) ? p->total : 0;
			if( haveCnt + count > (int)define->GetMaxHaveInven() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 220 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, trade );
				return;
			}
		}

		/// 무게 추가
		thatWeight += (define->GetWeight() * count);
	}

	/// 2. 인벤 슬롯 검사
	unsigned int thatSlotCnt = trade->GetThatLastSlot();
	if( thatSlotCnt > 0 )
	{
		unsigned int emptySlotCnt = 0;
		for( unsigned int slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
		{
			cItem& item = mItemArray[slot];
			if( item.GetIndex() == 0 )
			{
				emptySlotCnt++;

				if( emptySlotCnt >= thatSlotCnt )
					break;
			}		
		}

		/// 빈슬롯이 적을때, 현재 내가 등록한 슬롯의 갯수도 체크한다
		if( emptySlotCnt < thatSlotCnt )
		{
			unsigned int mySlotCnt = trade->GetMyLastSlot();
			if( mySlotCnt < thatSlotCnt )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 2021 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, trade );
				return;
			}
		}
	}

	/// 무게 감소
	thatWeight -= trade->GetMyTotalWeight();
	
	/// 3. 무게체크
	if( (mCurWeight + thatWeight) > mMaxWeight )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 201 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, trade );
		return;
	}

	MSG_REQ_ITEM_EXCHANGE_OK msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_OK_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	///
	trade->NetLock();
}

/// 거래 물품 수정요청
void cItemManager::TryToTradeReTry()
{
	if( IsLocked() )
		return;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return;
	}

	MSG_REQ_ITEM_EXCHANGE_RETRY msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_RETRY_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* trade = (cUIWindow*)GAMEUI->GetTradeWindow();
	if( trade )
		trade->NetLock();
}

/// 닫기요청
void cItemManager::TryToTradeCancel()
{
	if( IsLocked() )
		return;

	MSG_REQ_ITEM_EXCHANGE_CANCEL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_CANCEL_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* trade = (cUIWindow*)GAMEUI->GetTradeWindow();
	if( trade )
		trade->NetLock();
}

/// 거래 종료
void cItemManager::TryToTradeEnd()
{
	if( IsLocked() )
		return;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE || state == eOBJECT_STATE_STOP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return;
	}

	MSG_REQ_ITEM_EXCHANGE_END msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_EXCHANGE_END_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* trade = (cUIWindow*)GAMEUI->GetTradeWindow();
	if( trade )
		trade->NetLock();
}

/// 노점열기 요청
bool cItemManager::TryToUserSellOpen()
{
	if( IsLocked() )
		return false;

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	// 튜토리얼모드 불가
	if( TUTORIALMAN->IsTutorialMode() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11020 ) );
		return false;
	}

	/// 
	if( HERO->IsRideVehicle() == true )
	{
		MSGROOT msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_VEHICLE_END_REQ;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		HERO->AlightVehicle();
	}

	/// 따라가기 종료
	if( HERO->IsFollowing() )
		HERO->ReleaseFollowPlayer();

	mUserSellName = GAMERESOURCEMAN->GetGameText( 2514 );

	MSG_REQ_ITEM_STALL_SELL_OPEN msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_OPEN_REQ;
	Sstrncpy( msg.title, MAX_STALL_TITLE_LEN, (LPCTSTR)mUserSellName.Cstr(), mUserSellName.GetLength() );
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell )
		sell->NetLock( eLOCK_REGIST );
	return true;
}

bool cItemManager::TryToUserSellReName( LPCTSTR title )
{
	if( IsLocked() )
		return false;

	int len = ::_tcslen(title);
	if( len == 0 )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_OPENSTALL ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	mUserSellName = title;

	MSG_REQ_ITEM_STALL_SELL_RENAME msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_RENAME_REQ;
	Sstrncpy( msg.title, MAX_STALL_TITLE_LEN, title, len );
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell )
		sell->NetLock( eLOCK_RENAME );
	return true;
}

/// 노점닫기 요청
void cItemManager::TryToUserSellClose()
{
	if( IsLocked() )
		return;

	MSG_REQ_ITEM_STALL_SELL_CLOSE msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_CLOSE_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell )
		sell->NetLock( eLOCK_END );
}

/// 노점 아이템 등록
bool cItemManager::TryToUserSellAdd( unsigned short slot, unsigned long price )
{
	if( IsLocked() == true )
		return false;

	if( slot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_OPENSTALL ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	/// 거래불가 아이템 등록 막기
	cItem& item = mItemArray[slot];
	if( !item.mDefine )
	{
		assert(0);
		return false;
	}

 	// 거래불가 아이템일경우
	if( item.mDefine->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO || item.mDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2019 ) );
		return false;
	}

	/// 미인증 거래 가능 아이템이면서 실제로 미인증된 아이템이여야만 거래가능. 그 외엔 불가.
	if( item.mDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES && item.mSeal == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2019 ) );
		return false;
	}
	
	
	///
	MSG_REQ_ITEM_STALL_SELL_ADD msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_ADD_REQ;
	msg.number = slot;
	msg.price = price;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	LockAndBackup( ITEM_TRY_SELLITEMADD, slot, 0, 0, 0, price );

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell )
		sell->NetLock( eLOCK_ADD );
	return true;
}

/// 노점 아이템 삭제
bool cItemManager::TryToUserSellDelete( unsigned short slot )
{
	if( IsLocked() == true )
		return false;

	if( slot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_OPENSTALL ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell && sell->IsVisible() == false )
		return false;

	MSG_REQ_ITEM_STALL_SELL_DEL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_DEL_REQ;
	msg.number = slot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_SELLITEMDELETE, slot );
	
	if( sell )
		sell->NetLock( eLOCK_DELETE );
	return true;
}

/// 노점 아이템 수정
bool cItemManager::TryToUserSellModify( unsigned short slot )
{
	if( IsLocked() )
		return false;

	if( slot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_OPENSTALL ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	/// 락 걸려있으면 리턴
	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell && sell->IsNetLock() == true )
		return false;

	///
	MSG_REQ_ITEM_STALL_SELL_MOD msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_MOD_REQ;
	msg.number = slot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
	
	if( sell )
		sell->NetLock( eLOCK_ITEMUPDATE );
	return true;
}

void cItemManager::TryToUserSellJoin( sObject& obj )
{
	if( IsLocked() )
		return;

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return;
	}

	// 튜토리얼모드 불가
	if( TUTORIALMAN->IsTutorialMode() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11020 ) );
		return;
	}

	mUserSellTarget = obj.index;

	/// 노점상 열기 요청
	MSG_REQ_ITEM_STALL_SELL_JOIN msg;
	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_JOIN_REQ;
	msg.mTarget = obj;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

/// 노점상보기 닫기 요청
void cItemManager::TryToUserSellJoinClose()
{
	if( IsLocked() )
		return;

	MSG_REQ_ITEM_STALL_SELL_LEAVE msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_LEAVE_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell )
		sell->NetLock( eLOCK_JOINEND );
}

/// 노점 아이템 구입요청
bool cItemManager::TryToUserSellGet( unsigned short slot, unsigned int count )
{
	if( slot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && stopFlag == eSTOP_USESTALL ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cUserSellWindow* sell = GAMEUI->GetUserSellWindow();
	if( !sell )
	{
		assert(0);
		return false;
	}

	sUserSellData* data = sell->GetSelectJoinData();
	sInventory* inv = &data->mThatInven;

	/// 구입 조건 검사
	if( inv && data )
	{
		cItemDefine* define = GetItemDefine( inv->ItemIndex );
		if( !define )
			return false;

		/// 소지금 검사
		unsigned long price = data->mPrice * count;
		if( HERO->GetMoney() < price )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 200 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, sell	);
			return false;
		}

		/// 거래 최대 금액 여부 검사 (10억)		
		if( MAX_TRADE_MONEY < price )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 11208 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, sell	);
			return false;
		}

		/// 1. 소지할수 있는 공간 갯수 체크
		unsigned int remainCnt = GetRemainCount( inv->ItemIndex, count );
		if( remainCnt < count )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 202 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, sell );
			return false;
		}

		/// 2. max inven 체크 ( 갯수 제한 있을때만 )
		if( define->GetMaxHaveInven() > 0 )
		{
			sItemCount* p = GetItemCount( inv->ItemIndex );
			int haveCnt = ( p ) ? p->total : 0;
			if( haveCnt + count > (int)define->GetMaxHaveInven() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 220 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, sell );
				return false;
			}
		}

		/// 3. 무게체크
		float wei = mCurWeight + (define->GetWeight() * count);
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 201 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, sell );
			return false;
		}
	}
	else
	{
		assert( 0 );
		return false;
	}

	MSG_REQ_ITEM_STALL_SELL_GET msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_STALL_SELL_GET_REQ;
	msg.number = slot;
	msg.count = (unsigned short)count;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_SELLGET, 0 );

	sell->NetLock( eLOCK_GET );
	return true;
}

bool cItemManager::TryToPutCard( unsigned short itemSlot, unsigned short cardSlot, unsigned short putSlot )
{
	if( itemSlot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( cardSlot > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE || 
		state == eOBJECT_STATE_ATTACK || state == eOBJECT_STATE_STOP ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	MSG_REQ_ITEM_PUT_CARD_INVENTORY msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_PUT_CARD_INVENTORY_REQ;
	msg.number1 = itemSlot;
	msg.number2 = cardSlot;		/// 사용될 카드

	/// ( 0 ~ 5 까지 카드 삽입 슬롯 지정 )
	msg.cardSlot = (unsigned char)putSlot;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_PUTCARD, cardSlot );

	cUIWindow* putcard = (cUIWindow*)GAMEUI->GetPutCardWindow();
	if( putcard )
		putcard->NetLock();

	return true;
}

/*
bool cItemManager::TryToCashPullCard( unsigned short itemSlot, unsigned char pullCardSlot, unsigned short cashSlot )
{
	cUIWindow* putcard = (cUIWindow*)GAMEUI->GetPutCardWindow();
	if( putcard )
	{
		if( putcard->IsVisible() )
		{
			return false;
		}
	}

	if( itemSlot > MAX_INVENTORY || cashSlot > MAX_INVENTORY )
	{
		return false;
	}

	if( MIN_CARD_SLOT < pullCardSlot || pullCardSlot > MAX_CARD_SLOT )
	{
		assert(0);
		return false;
	}

	cItemDefine* pullCardDef = ITEMMAN->GetItemDefine( pullCardSlot );
	if( !pullCardDef )
	{
		assert(0);
		return false;
	}

	cItem* item = ITEMMAN->GetItem( itemSlot );
	if( !item )
	{
		assert(0);
		return false;
	}

	cItemDefine* itemDefine = item->GetDefine();
	if( !itemDefine )
	{
		assert(0);
		return false;
	}

	/// 카드 인덱스 유효 여부 검사
	unsigned long pullCardIdx = 0;
	if( pullCardSlot		== ITEM_CARD_SLOT_1 )	pullCardIdx = item->GetCardIndex1();
	else if( pullCardSlot	== ITEM_CARD_SLOT_2 )	pullCardIdx = item->GetCardIndex2();
	else if( pullCardSlot	== ITEM_CARD_SLOT_3 )	pullCardIdx = item->GetCardIndex3();
	else if( pullCardSlot	== ITEM_CARD_SLOT_4 )	pullCardIdx = item->GetCardIndex4();
	else if( pullCardSlot	== ITEM_CARD_SLOT_5 )	pullCardIdx = item->GetCardIndex5();
	if( pullCardIdx == 0 )
	{
		return false;
	}

	/// 무기, 방어구 타입 아니면 리턴
	if( !(itemDefine->GetType() == ITEM_WEAPON || itemDefine->GetType() == ITEM_WEAR || itemDefine->GetType() == ITEM_ACCESSORY) )
	{
		return false;
	}

	/// 카드 장착 슬롯 존재 여부 확인
	if( itemDefine->GetPutSlotCount() <= 0 )
	{
		return false;
	}

	cItem* cashItem = ITEMMAN->GetItem( cashSlot );
	if( !cashItem)
	{
		assert(0);
		return false;
	}

	/// 카드 추출 주문서 인덱스 인지 확인
	if( cashItem->GetIndex() != eSYSTEM_NORMAL )
	{		
		return false;
	}

	/// 미인증된 아이템이면 사용 불가능
	if( cashItem->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	// 추출 카드가 적재될 빈 슬롯이 있는지 여부
	unsigned short slotIndex1 = 0;
	if( FindEmptySlot( &slotIndex1, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 294 ) );
		return false;
	}

	unsigned long state = HERO->GetState();
	if( !(state == eOBJECT_STATE_IDLE || state == eOBJECT_STATE_MOVE || 
		state == eOBJECT_STATE_ATTACK || state == eOBJECT_STATE_STOP ) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	MSG_REQ_ITEM_PULL_CARD_INVENTORY msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_PULL_CARD_INVENTORY_REQ;
	msg.number1	 = itemSlot;		/// 아이템 슬롯
	msg.cardSlot = pullCardSlot;	/// 카드 슬롯
	msg.number2  = cashSlot;		/// 추출 주문서 슬롯

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_PULLCARD, cashSlot );

	if( putcard )
		putcard->NetLock();

	return true;	
}
*/
/// 컬렉션북 등록
bool cItemManager::TryToCardBookAdd( unsigned short slot0, unsigned short slot1 )
{
	if( slot0 > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( slot1 > MAX_INVENTORY )
	{
		assert(0);
		return false;
	}

	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	if( state == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( state == eOBJECT_STATE_STOP )
	{
		if( stopFlag == eSTOP_GAMEFINISH || stopFlag == eSTOP_NPCSPEECH )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
			return false;
		}
	}

	MSG_REQ_ITEM_COLLECT msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_COLLECT_REQ;
	msg.number1 = slot0;
	msg.number2 = slot1;		/// 스프레드
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_CARDBOOKADD, slot0, slot1 );
	return true;
}

/// 판매대행 등록
bool cItemManager::TryToNpcDealRegistReq( unsigned short slotIndex, unsigned long price, char validThru )
{
	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	/// 아이템 
	if( slotIndex > MAX_INVENTORY )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1501 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 판매금액
	if( price <= 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1500 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 등록수수료 검사
	unsigned long charge = (validThru/NPC_ITEM_AGENT_TIME1) * ITEM_AGENT_NPC_FEE;
	if( HERO->GetMoney() < charge )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1502 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 등록갯수 검사
	cItem& item = mItemArray[slotIndex];
	if( item.mCount <= 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1503 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	// 판매 설정 금액이 10억 초과 여부 검사
	if( item.mCount * price > MAX_TRADE_MONEY )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 11206 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	MSG_REQ_NPC_ITEM_AGENT_ADD msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_ADD_REQ;
	msg.StartPage = 1;
	msg.ValidThru = validThru;
	msg.Number = slotIndex;
	msg.Count = item.mCount;
	msg.Price = price;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_NPCDEALREGIST, slotIndex );

	win->NetLock();

	// 등록중
	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_NPCDEAL_ITEM_REGIST, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 1523 ), 
		GAMERESOURCEMAN->GetGameText( 301 )
		);
	return true;
}


/// 이름 & 타입으로 검색
bool cItemManager::TryToNpcDealSearchAll( long page, LPCTSTR itemName, char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData )
{
	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	int len = ::_tcslen(itemName);

	MSG_REQ_NPC_ITEM_AGENT_SEARCH_ALL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_SEARCH_ALL_REQ;
	Sstrncpy( msg.itemDefineName, MAX_ITEM_DEFINE_NAME, itemName, len );
	msg.startPage = page;
	msg.sortData = sortData;
	msg.itemDefineSearchType = type;
	msg.itemDefineSearchSubType = subType;
	msg.itemLimitCharLevelB = lowLv;
	msg.itemLimitCharLevelE = highLv;
	msg.inventoryEnhanced = enhance;
	msg.itemDefineRareLevel = rare;
	
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	win->NetLock();
	return true;
}

/// 타입 으로 검색
bool cItemManager::TryToNpcDealSearchType( long page, char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData )
{
	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	MSG_REQ_NPC_ITEM_AGENT_SEARCH_TYPE msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_SEARCH_TYPE_REQ;
	msg.startPage = page;
	msg.sortData = sortData;
	msg.itemDefineSearchType = type;
	msg.itemDefineSearchSubType = subType;
	msg.itemLimitCharLevelB = lowLv;
	msg.itemLimitCharLevelE = highLv;
	msg.inventoryEnhanced = enhance;
	msg.itemDefineRareLevel = rare;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	win->NetLock();
	return true;
}

bool cItemManager::TryToNpcDealOpen( unsigned long npcIndex )
{
	if( IsLocked() == true )
		return false;

	// 이미 npc 대화상태임.
	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	MSG_REQ_NPC_ITEM_AGENT_OPEN msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_OPEN_REQ;
	msg.npcIndex = npcIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	win->NetLock();
	return true;
}

bool cItemManager::TryToNpcDealOpenPage( long page )
{
	if( IsLocked() == true )
		return false;

	// 이미 npc 대화상태임.
	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	MSG_REQ_NPC_ITEM_AGENT_PAGE_OPEN msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_OPEN_PAGE_REQ;
	msg.StartPage = page;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	win->NetLock();
	return true;
}

bool cItemManager::TryToNpcDealRegistDelete( long idx )
{
	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	MSG_REQ_NPC_ITEM_AGENT_DEL msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category  = NM_NPC;
	msg.Protocol  = NM_NPC_ITEM_AGENT_DEL_REQ;
	msg.StartPage = 1;
	msg.Idx		  = idx;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_NPCDEALDELETE, 0 );

	win->NetLock();

	// 검색중
	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_NPCDEAL_ITEM_DELETE, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 1524 ), 
		GAMERESOURCEMAN->GetGameText( 301 )
		);
	return true;
}

bool cItemManager::TryToNpcDealBuy( long idx, long page, short count, LPCTSTR itemName, 
									char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData )
{
	if( IsLocked() == true )
		return false;

	unsigned long state = HERO->GetState();
	eSTOPFLAG stopFlag = HERO->GetStopFlag();
	if( !(state == eOBJECT_STATE_STOP && (stopFlag == eSTOP_NPCSPEECH || stopFlag == eSTOP_SUMMON_PAPER)) )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
		return false;
	}

	cNpcDealWindow* win = GAMEUI->GetNpcDealWindow();
	if( !win )
	{
		assert(0);
		return false;
	}

	if( win->IsNetLock() )
		return false;

	int len = ::_tcslen(itemName);

	MSG_REQ_NPC_ITEM_AGENT_GET msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_ITEM_AGENT_GET_REQ;
	msg.Idx		 = idx;
	msg.Count    = count;
	msg.sortData = sortData;
	msg.startPage = page;
	Sstrncpy( msg.itemDefineName, MAX_ITEM_DEFINE_NAME, itemName, len );
	msg.itemDefineSearchType = type;
	msg.itemDefineSearchSubType = subType;
	msg.itemLimitCharLevelB = lowLv;
	msg.itemLimitCharLevelE = highLv;
	msg.inventoryEnhanced = enhance;
	msg.itemDefineRareLevel = rare;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	LockAndBackup( ITEM_TRY_NPCDEALBUY, 0 );

	win->NetLock();

	// 구입중
	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_NPCDEAL_ITEM_BUY, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 1525 ), 
		GAMERESOURCEMAN->GetGameText( 301 )
		);
	return false;
}

// 서버 -> 수령함 요청
bool cItemManager::TryToReceiptList()
{
	if( IsLocked() )
		return false;

	ClearCashInven();

	MSG_REQ_ITEM_CASH_LIST msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_CASH_LIST_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 잠금
	LockAndBackup( ITEM_TRY_LISTRECEIPT, 0 );	

	cUIWindow* receipt = (cUIWindow*)GAMEUI->GetItemReceiptWindow();
	if( receipt )
		receipt->NetLock();
	return true;
}

bool cItemManager::TryToThemeReset( unsigned short slot, unsigned short themeNum, unsigned char themeMode )
{
	if( IsLocked() )
		return false;

	MSG_REQ_ITEM_THEMERESET msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_THEMERESET_REQ;
	msg.mSlotNum = slot;
	msg.mThemeNum = themeNum;
	msg.mThemeMode = themeMode;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* resetWin = (cUIWindow*)GAMEUI->GetThemeResetWindow();
	if( resetWin )
		resetWin->NetLock();
	
	LockAndBackup( ITEM_TRY_THEMERESET, 0 );	

	// 상태 stop
	HERO->SetStopFlag( eSTOP_THEMERESET );

	UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
		eMBEVENT_THEME_RESETWAIT, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 12008 ),
		GAMERESOURCEMAN->GetGameText( 301 )
		);

	return true;
}

bool cItemManager::TryToGiveParty( unsigned long playerIndex, unsigned long itemIndex )
{
	if( IsLocked() )
		return false;

	MSG_REQ_ITEM_PARTY_GIVE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_PARTY_GIVE_REQ;
	msg.characterIdx = playerIndex;
	msg.itemDefineIndex = itemIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
	if( win )
		win->NetLock();

	cUIWindow* looting = (cUIWindow*)GAMEUI->GetLootingWindow();
	if( looting )
		looting->NetLock();

	LockAndBackup( ITEM_TRY_GIVEPARTY, 0 );
	return true;
}

unsigned long cItemManager::GetDropNodeIndex( unsigned long dropIndex )
{
	cDropItemMap::cIterator i = mDropItemMap.Find( dropIndex );
	if( i != mDropItemMap.End() )
	{
		return (unsigned long)(i->mSecond);
	}
	return 0;
}

void cItemManager::DeleteInventory( unsigned short slotIndex )
{
	NetDumpItem( slotIndex );
}

void cItemManager::NpcNetTryItem( unsigned int protocol, void* msg )
{
	switch( protocol )
	{
	case NM_NPC_ITEM_MOV2_INVENTORY_RES:
	case NM_NPC_ITEM_BUY_RES:
	case NM_NPC_ITEM_SELL_RES:
	case NM_NPC_ITEM_AGENT_ADD_RES:
	case NM_NPC_ITEM_AGENT_DEL_RES:
	case NM_NPC_ITEM_AGENT_GET_RES:
		NetTryItem( protocol, msg );
		break;
	default:
		assert( 0 && "invalid item try - NPC" );
		break;
	}
}

void cItemManager::LockAndBackup(eItemTry itemTry, unsigned short slotIndex0, unsigned short slotIndex1, 
								 unsigned long tempItemIndex, unsigned int value, unsigned long price, cItemIcon* icon )
{
	mLocked = true;
	mTry = itemTry;

	mSlotIndex0 = slotIndex0;
	mSlotIndex1 = slotIndex1;
	mTempIndex = tempItemIndex;
	mValue = value;
	mPrice = price;
}

bool cItemManager::IsLocked()
{
	return mLocked;
}

/// 백업 데이터 빼고, try만 저장함
void cItemManager::LockItemTry( eItemTry itemTry, unsigned short slotIndex0 )
{
	mLocked = true;
	mTry = itemTry;
	mSlotIndex0 = slotIndex0;
	mSlotIndex1 = 0;
	mValue = 0;
	mTempIndex = 0;

	if( mSlotIndex0 <= MAX_INVENTORY )
	{
		cItem& item0 = mItemArray[mSlotIndex0];
		item0.SetLock( true );
	}
}

/// 풀어도 되는 락인지 검사후 unlock
void cItemManager::UnlockItemTry( eItemTry itemTry )
{
	if( mTry == itemTry )
	{
		mLocked = false;
		mTry = ITEM_TRY_NULL;

		if( mSlotIndex0 <= MAX_INVENTORY )
		{
			cItem& item0 = mItemArray[mSlotIndex0];
			item0.SetLock( false );
		}
	
		if( mSlotIndex1 <= MAX_INVENTORY )
		{
			cItem& item1 = mItemArray[mSlotIndex1];
			item1.SetLock( false );
		}

		mSlotIndex0 = USHRT_MAX;
		mSlotIndex1 = USHRT_MAX;
		mValue = 0;
	}
	else
	{
		assert(0);
	}
}

void cItemManager::CheckItemSlot( unsigned short slotIndex )
{
	switch( slotIndex )
	{
	case INVENTORY_COSTUME_BODY1:
	case INVENTORY_COSTUME_BODY2:
	case INVENTORY_COSTUME_HAND:
	case INVENTORY_COSTUME_FOOT:
		{
			cItem& item = mItemArray[slotIndex];

			// 코스튬 상의, 하의, 손, 발
			cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY1];
			cItem& item1 = mItemArray[INVENTORY_COSTUME_BODY2];
			cItem& item2 = mItemArray[INVENTORY_COSTUME_HAND];
			cItem& item3 = mItemArray[INVENTORY_COSTUME_FOOT];

			// 원피스면
			if( item.mDefine && item.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
			{
				item1.SetTwoHanded( true, item.mIcon ? item.mIcon->GetIconParam() : 0, item.mDefine );
				item2.SetTwoHanded( true, item.mIcon ? item.mIcon->GetIconParam() : 0, item.mDefine );
				item3.SetTwoHanded( true, item.mIcon ? item.mIcon->GetIconParam() : 0, item.mDefine );
			}
			else
			{
				if( item0.mCount == 0 )
					item0 = 0;
				else
					item0.SetTwoHanded( false );

				if( item1.mCount == 0 )
					item1 = 0;
				else
					item1.SetTwoHanded( false );

				if( item2.mCount == 0 )
					item2 = 0;
				else
					item2.SetTwoHanded( false );

				if( item3.mCount == 0 )
					item3 = 0;
				else
					item3.SetTwoHanded( false );
			}
		}
		break;
	case INVENTORY_HAND_LEFT1:
	case INVENTORY_HAND_RIGHT1:
		{
			cItem& lItem = mItemArray[INVENTORY_HAND_LEFT1];
			cItem& rItem = mItemArray[INVENTORY_HAND_RIGHT1];

			if( lItem.mCount == 0 )
				lItem = 0;
			if( rItem.mEquipType == ITEM_EQUIP_TWO_HANDS )
				lItem.SetTwoHanded( true, rItem.mIcon ? rItem.mIcon->GetIconParam() : 0, rItem.mDefine );
			else
				lItem.SetTwoHanded( false );
		}
		break;
	case INVENTORY_HAND_LEFT2:
	case INVENTORY_HAND_RIGHT2:
		{
			cItem& lItem = mItemArray[INVENTORY_HAND_LEFT2];
			cItem& rItem = mItemArray[INVENTORY_HAND_RIGHT2];

			if( lItem.mCount == 0 )
				lItem = 0;
			if( rItem.mEquipType == ITEM_EQUIP_TWO_HANDS )
				lItem.SetTwoHanded( true, rItem.mIcon ? rItem.mIcon->GetIconParam() : 0, rItem.mDefine );
			else
				lItem.SetTwoHanded( false );
		}
		break;
	}
}

void cItemManager::AddSpecialAbility( short type, short value )
{
	#define Warning4244(V,A) V = V + A

	switch ( type )
	{
	case ITEM_OPT_STR:	// 힘
		Warning4244( mBasicAbil.mOptStr, value );
		break;
	case ITEM_OPT_DEX:	// 민첩
		Warning4244( mBasicAbil.mOptDex, value );
		break;
	case ITEM_OPT_CON:	// 체력
		Warning4244( mBasicAbil.mOptCon, value );
		break;
	case ITEM_OPT_INT:	// 지능
		Warning4244( mBasicAbil.mOptInt, value );
		break;
	case ITEM_OPT_WIS:	// 지혜
		Warning4244( mBasicAbil.mOptWis, value );
		break;
	case ITEM_OPT_ALL:	// 모두(힘:민첩:체력:지능:지혜)
		Warning4244( mBasicAbil.mOptStr, value );
		Warning4244( mBasicAbil.mOptDex, value );
		Warning4244( mBasicAbil.mOptCon, value );
		Warning4244( mBasicAbil.mOptInt, value );
		Warning4244( mBasicAbil.mOptWis, value );
		break;

		// Option - Plus
	case ITEM_OPT_PLUS_PHYSIC_ATTACK:	// 물리공격력
		Warning4244( mBasicAbil.mOptPlusPhysicAttack, value );
		break;
	case ITEM_OPT_PLUS_MAGIC_ATTACK:	// 마법공격력
		Warning4244( mBasicAbil.mOptPlusMagicAttack, value );
		break;
	case ITEM_OPT_PLUS_PHYSIC_DEFENSE:	// 물리방어력
		Warning4244( mBasicAbil.mOptPlusPhysicDefense, value );
		break;
	case ITEM_OPT_PLUS_MAGIC_DEFENSE:	// 마법방어력
		Warning4244( mBasicAbil.mOptPlusMagicDefense, value );
		break;
	case ITEM_OPT_PLUS_PHYSIC_ATTACK_RATE:	// 물리명중률
		Warning4244( mBasicAbil.mOptPlusPhysicAttackRate, value );
		break;
	case ITEM_OPT_PLUS_MAGIC_ATTACK_RATE:	// 마법명중률
		Warning4244( mBasicAbil.mOptPlusMagicAttackRate, value );
		break;
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL:	// 물리크리티컬
		Warning4244( mBasicAbil.mOptPlusPhysicCritical, value );
		break;
	case ITEM_OPT_PLUS_MAGIC_CRITICAL:	// 마법크리티컬
		Warning4244( mBasicAbil.mOptPlusMagicCritical, value );
		break;
	case ITEM_OPT_PLUS_DOD:	// 회피
		Warning4244( mBasicAbil.mOptPlusDod, value );
		break;
	case ITEM_OPT_PLUS_WEI:	// 무게
		Warning4244( mBasicAbil.mOptPlusWei, value );
		break;
	case ITEM_OPT_PLUS_MAX_HP:	// 최대량 HP
		Warning4244( mBasicAbil.mOptPlusMaxHp, value );
		break;
	case ITEM_OPT_PLUS_MAX_MP:	// 최대량 MP
		Warning4244( mBasicAbil.mOptPlusMaxMp, value );
		break;
	case ITEM_OPT_PLUS_MOV:	// 이동
		Warning4244( mBasicAbil.mOptPlusMov, value );
		break;
	case ITEM_OPT_PLUS_HP:	// HP 회복
		Warning4244( mBasicAbil.mOptPlusHp, value );
		break;
	case ITEM_OPT_PLUS_MP:	// MP 회복
		Warning4244( mBasicAbil.mOptPlusMp, value );
		break;
	case ITEM_OPT_PLUS_ATTACK_SPEED:	// 일반공격속도
		Warning4244( mBasicAbil.mOptPlusAttackSpeed, value );
		break;
	case ITEM_OPT_PLUS_SKILL_COOLTIME_SPEED:	// 스킬쿨타임속도
		Warning4244( mBasicAbil.mOptPlusSkillCooltimeSpeed, value );
		break;
	case ITEM_OPT_PLUS_SHORT_RANGE:	// 근거리사정거리
		Warning4244( mBasicAbil.mOptPlusShortRange, value );
		break;
	case ITEM_OPT_PLUS_DISTANCE_RANGE:	// 원거리사정거리
		Warning4244( mBasicAbil.mOptPlusDistanceRange, value );
		break;
	case ITEM_OPT_PLUS_HEAL:			// 힐 회복량
		Warning4244( mBasicAbil.mOptPlusHeal, value );
		break;
	case ITEM_OPT_PLUS_HP_RECOVERTY:	// HP 회복시 증가량
		Warning4244( mBasicAbil.mOptPlusHpReceive, value );
		break;
	case ITEM_OPT_PLUS_ALL_ATTACK:		// 모든 공격력
		Warning4244( mBasicAbil.mOptPlusPhysicAttack, value );
		Warning4244( mBasicAbil.mOptPlusMagicAttack, value );
		break;
	case ITEM_OPT_PLUS_ALL_DEFENSE:		// 모든 방어력
		Warning4244( mBasicAbil.mOptPlusPhysicDefense, value );
		Warning4244( mBasicAbil.mOptPlusMagicDefense, value );
		break;
	case ITEM_OPT_PLUS_PHYSIC_CRITICAL_DEC:	// 물리크리티컬 감소
		Warning4244( mBasicAbil.mOptPlusPhysicCriticalDec, value );
		break;
	case ITEM_OPT_PLUS_MAGIC_CRITICAL_DEC:	// 마법크리티컬 감소
		Warning4244( mBasicAbil.mOptPlusMagicCriticalDec, value );
		break;
	case ITEM_OPT_PLUS_RESISTANCE:			// 저항력
		Warning4244( mBasicAbil.mOptPlusResistance, value );
		break;

		// Option - Percent
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK:	// 물리공격력
		Warning4244( mBasicAbil.mOptPercentPhysicAttack, value );
		break;
	case ITEM_OPT_PERCENT_MAGIC_ATTACK:	// 마법공격력
		Warning4244( mBasicAbil.mOptPercentMagicAttack, value );
		break;
	case ITEM_OPT_PERCENT_PHYSIC_DEFENSE:	// 물리방어력
		Warning4244( mBasicAbil.mOptPercentPhysicDefense, value );
		break;
	case ITEM_OPT_PERCENT_MAGIC_DEFENSE:	// 마법방어력
		Warning4244( mBasicAbil.mOptPercentMagicDefense, value );
		break;
	case ITEM_OPT_PERCENT_PHYSIC_ATTACK_RATE:	// 물리명중률
		Warning4244( mBasicAbil.mOptPercentPhysicAttackRate, value );
		break;
	case ITEM_OPT_PERCENT_MAGIC_ATTACK_RATE:	// 마법명중률
		Warning4244( mBasicAbil.mOptPercentMagicAttackRate, value );
		break;
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL:	// 물리크리티컬
		Warning4244( mBasicAbil.mOptPercentPhysicCritical, value );
		break;
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL:	// 마법크리티컬
		Warning4244( mBasicAbil.mOptPercentMagicCritical, value );
		break;
	case ITEM_OPT_PERCENT_DOD:	// 회피
		Warning4244( mBasicAbil.mOptPercentDod, value );
		break;
	case ITEM_OPT_PERCENT_WEI:	// 무게
		Warning4244( mBasicAbil.mOptPercentWei, value );
		break;
	case ITEM_OPT_PERCENT_MAX_HP:	// 최대량 HP
		Warning4244( mBasicAbil.mOptPercentMaxHp, value );
		break;
	case ITEM_OPT_PERCENT_MAX_MP:	// 최대량 MP
		Warning4244( mBasicAbil.mOptPercentMaxMp, value );
		break;
	case ITEM_OPT_PERCENT_MOV:	// 이동
		Warning4244( mBasicAbil.mOptPercentMov, value );
		break;
	case ITEM_OPT_PERCENT_HP:	// HP 회복
		Warning4244( mBasicAbil.mOptPercentHp, value );
		break;
	case ITEM_OPT_PERCENT_MP:	// MP 회복
		Warning4244( mBasicAbil.mOptPercentMp, value );
		break;
	case ITEM_OPT_PERCENT_ATTACK_SPEED:		// 일반공격속도
		Warning4244( mBasicAbil.mOptPercentAttackSpeed, value );
		break;
	case ITEM_OPT_PERCENT_SKILL_COOLTIME_SPEED:		// 스킬쿨타임속도
		Warning4244( mBasicAbil.mOptPercentSkillCooltimeSpeed, value );
		break;
	case ITEM_OPT_PERCENT_SHORT_RANGE:				// 근거리사정거리
		Warning4244( mBasicAbil.mOptPercentShortRange, value );
		break;
	case ITEM_OPT_PERCENT_DISTANCE_RANGE:			// 원거리사정거리
		Warning4244( mBasicAbil.mOptPercentDistanceRange, value );
		break;
	case ITEM_OPT_PERCENT_HEAL:						// 힐 회복량
		Warning4244( mBasicAbil.mOptPercentHeal, value );
		break;
	case ITEM_OPT_PERCENT_HP_RECOVERTY:				// HP 회복시 증가량
		Warning4244( mBasicAbil.mOptPercentHpReceive, value );
		break;
	case ITEM_OPT_PERCENT_ALL_ATTACK:				// 모든 공격력
		Warning4244( mBasicAbil.mOptPercentPhysicAttack, value );
		Warning4244( mBasicAbil.mOptPercentMagicAttack, value );
		break;
	case ITEM_OPT_PERCENT_ALL_DEFENSE:			// 모든 방어력
		Warning4244( mBasicAbil.mOptPercentPhysicDefense, value );
		Warning4244( mBasicAbil.mOptPercentMagicDefense, value );
		break;
	case ITEM_OPT_PERCENT_PHYSIC_CRITICAL_DEC:		// 물리크리티컬 감소
		Warning4244( mBasicAbil.mOptPercentPhysicCriticalDec, value );
		break;
	case ITEM_OPT_PERCENT_MAGIC_CRITICAL_DEC:		// 마법크리티컬 감소
		Warning4244( mBasicAbil.mOptPercentMagicCriticalDec, value );
		break;
	case ITEM_OPT_PERCENT_RESISTANCE:				// 저항력
		Warning4244( mBasicAbil.mOptPercentResistance, value );
		break;

		// Option - Plus Etc
	case ITEM_OPT_PLUS_DRAIN_HP:		// HP 흡수
		Warning4244( mBasicAbil.mOptPlusDrainHp, value );
		break;
	case ITEM_OPT_PLUS_DRAIN_MP:		// MP 흡수
		Warning4244( mBasicAbil.mOptPlusDrainMp, value );
		break;
	case ITEM_OPT_PLUS_DAMAGE_DEC:		// 데미지 흡수/감소
		Warning4244( mBasicAbil.mOptPlusDamageDec, value );
		break;
	case ITEM_OPT_PLUS_DAMAGE_REF:		// 데미지 반사
		Warning4244( mBasicAbil.mOptPlusDamageRef, value );
		break;

		// Option - Etc
	case ITEM_OPT_PERCENT_DRAIN_HP:				// HP 흡수
		Warning4244( mBasicAbil.mOptPercentDrainHp, value );
		break;
	case ITEM_OPT_PERCENT_DRAIN_MP:				// MP 흡수
		Warning4244( mBasicAbil.mOptPercentDrainMp, value );
		break;
	case ITEM_OPT_PERCENT_DAMAGE_DEC:			// 데미지 흡수/감소
		Warning4244( mBasicAbil.mOptPercentDamageDec, value );
		break;
	case ITEM_OPT_PERCENT_DAMAGE_REF:			// 데미지 반사
		Warning4244( mBasicAbil.mOptPercentDamageRef, value );
		break;
	}
}

/// 아이템 어빌리티 계산
void cItemManager::AddAbility( cItem& item )
{
	cItemDefine* define = item.mDefine;
	if( !define )
	{
		assert(0);
		return;
	}

	unsigned char type = define->GetType();
	//unsigned char subtype = define->GetSubType();

	cItemAbility* abil = define->GetAbililty();
	if( !abil )
	{
		assert(0);
		return;
	}

	/// 기본수치
	mBasicAbil.mPhysicMinAttack	= mBasicAbil.mPhysicMinAttack + abil->GetPhysicMinAttack();	/// 물리:최소공격력
	mBasicAbil.mPhysicMaxAttack = mBasicAbil.mPhysicMaxAttack + abil->GetPhysicMaxAttack();	/// 물리:최대공격력
	mBasicAbil.mMagicMinAttack	= mBasicAbil.mMagicMinAttack  + abil->GetMagicMinAttack();	/// 마법:최소공격력
	mBasicAbil.mMagicMaxAttack	= mBasicAbil.mMagicMaxAttack  + abil->GetMagicMaxAttack();	/// 마법:최대공격력
    mBasicAbil.mHealValue       = mBasicAbil.mHealValue       + abil->GetHealValue();	    /// 힐 회복량

	mBasicAbil.mPhysicDefense	= mBasicAbil.mPhysicDefense   + abil->GetPhysicDefense();	/// 물리:방어력
	mBasicAbil.mMagicDefense	= mBasicAbil.mMagicDefense    + abil->GetMagicDefense();	/// 마법:방어력
	mBasicAbil.mAttackRange		= mBasicAbil.mAttackRange     + abil->GetAttackRange();		/// 공격 거리
	mBasicAbil.mAttackSpeed		= mBasicAbil.mAttackSpeed     + abil->GetAttackSpeed();		/// 공격 속도

	/// 옵션 값
	sAblityOption* opt = abil->GetOption();
	for( int i = 0; i < 5; ++i, ++opt )
	{
		AddSpecialAbility( opt->mOptType, opt->mOptValue );
	}
			
	/// 무기, 방어구
	if( type == ITEM_WEAPON || type == ITEM_WEAR || type == ITEM_ACCESSORY )
	{
		/// 강화된 아이템의 수치 합하기
		AddEnhance( item );

		/// 아이템 장착카드 수치 합하기
		AddPutCard( item.mCardIndex1 );
		AddPutCard( item.mCardIndex2 );
		AddPutCard( item.mCardIndex3 );
		AddPutCard( item.mCardIndex4 );
		AddPutCard( item.mCardIndex5 );
	}
}

/// 강화정보 체크
void cItemManager::AddEnhance( cItem& item )
{
	cItemDefine* define = item.mDefine;
	if( !define )
	{
		assert(0);
		return;
	}

	unsigned char enhanceStep = item.GetEnhanceStep();
	if( enhanceStep > 0 )
	{
		/// 강화단계에 따른 + 수치 출력
		short addPhysicAttack  = 0;
		short addMagicAttack   = 0;
		short addPhysicDefense = 0;
		short addMagicDefense  = 0;
		short addHealValue     = 0;

		cEnhancedOptionArr* enhanceOptionArr = define->GetEnhancedOptionArr();
		if( !enhanceOptionArr || enhanceOptionArr->IsEmpty() )
		{
			assert(0);
			return;
		}

		unsigned int OptionSize = enhanceOptionArr->GetSize();

		for( unsigned int idx = 0; idx < OptionSize; idx++ )
		{
			sItemEnhanced* enhance = (sItemEnhanced*)(*enhanceOptionArr)[idx];
	
			if( !enhance )
			{
				assert(0);
				return;
			}

			/// 물리공격력 세팅
			if( enhance->mEnhancedType == eENHANCE_PHYSICATTACK )
			{
				addPhysicAttack = enhance->mValue[enhanceStep-1];

				mBasicAbil.mPhysicMinAttack	= mBasicAbil.mPhysicMinAttack + addPhysicAttack;	/// 물리:최소공격력
				mBasicAbil.mPhysicMaxAttack = mBasicAbil.mPhysicMaxAttack + addPhysicAttack;	/// 물리:최대공격력
			}

			/// 마법공격력 수치 & 세팅
			if( enhance->mEnhancedType == eENHANCE_MAGICATTACK )
			{
				addMagicAttack = enhance->mValue[enhanceStep-1];

				mBasicAbil.mMagicMinAttack	= mBasicAbil.mMagicMinAttack + addMagicAttack;	/// 마법:최소공격력
				mBasicAbil.mMagicMaxAttack	= mBasicAbil.mMagicMaxAttack + addMagicAttack;	/// 마법:최대공격력
			}

			/// 물리 방어력 세팅 
			if( enhance->mEnhancedType == eENHANCE_PHYSICDEFENSE )
			{
				addPhysicDefense = enhance->mValue[enhanceStep-1];
				mBasicAbil.mPhysicDefense	= mBasicAbil.mPhysicDefense + addPhysicDefense;		/// 물리:방어력
			}

			/// 마법 방어력 세팅 
			if( enhance->mEnhancedType == eENHANCE_MAGICDEFENSE )
			{
				addMagicDefense = enhance->mValue[enhanceStep-1];
				mBasicAbil.mMagicDefense	= mBasicAbil.mMagicDefense + addMagicDefense;		/// 마법:방어력
			}

			///힐 회복량 세팅
			if( enhance->mEnhancedType == eENHANCE_HEAL )
			{
				addHealValue = enhance->mValue[enhanceStep-1];
				mBasicAbil.mHealValue		= mBasicAbil.mHealValue + addHealValue;
			}
		}

	}
}

/// 카드장착 정보 체크
void cItemManager::AddPutCard( unsigned long cardIndex )
{
	if( cardIndex == 0 )
		return;

	cItemDefine* cardDefine = GetItemDefine( cardIndex );
	if( !cardDefine )
	{
		assert(0);
		return;
	}

	cItemAbility* abil = cardDefine->GetAbililty();
	if( !abil )
	{
		assert(0);
		return;
	}

	/// 장착한 카드의 정보만큼 갱신
	mBasicAbil.mPhysicMinAttack	= mBasicAbil.mPhysicMinAttack + abil->GetPhysicMinAttack();	/// 물리:최소공격력
	mBasicAbil.mPhysicMaxAttack = mBasicAbil.mPhysicMaxAttack + abil->GetPhysicMaxAttack();	/// 물리:최대공격력
	mBasicAbil.mMagicMinAttack	= mBasicAbil.mMagicMinAttack  + abil->GetMagicMinAttack();	/// 마법:최소공격력
	mBasicAbil.mMagicMaxAttack	= mBasicAbil.mMagicMaxAttack  + abil->GetMagicMaxAttack();	/// 마법:최대공격력

	mBasicAbil.mPhysicDefense	= mBasicAbil.mPhysicDefense   + abil->GetPhysicDefense();	/// 물리:방어력
	mBasicAbil.mMagicDefense	= mBasicAbil.mMagicDefense    + abil->GetMagicDefense();	/// 마법:방어력
	mBasicAbil.mAttackRange		= mBasicAbil.mAttackRange     + abil->GetAttackRange();		/// 공격 거리
	mBasicAbil.mAttackSpeed		= mBasicAbil.mAttackSpeed     + abil->GetAttackSpeed();		/// 공격 속도

	/// 옵션 값
	sAblityOption* opt = abil->GetOption();
	for( int i = 0; i < 5; ++i, ++opt )
	{
		AddSpecialAbility( opt->mOptType, opt->mOptValue );
	}
}

void cItemManager::CheckEquipAbility()
{
	/// 정보 초기화
	::memset( &mBasicAbil, 0, sizeof(sEquipAbility) );

	tHashSet<unsigned long> ItemSet;

	/// 무기제외
	for( unsigned int i = INVENTORY_WEAR_BEGIN; i <= INVENTORY_WEAR_CARD3; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDefine && CheckLimit( item.GetIndex() ) == ITEMLIMIT_SUCCESS && item.mSeal == false )
		{
			AddAbility( item );

			unsigned int setIdx = item.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );
		}
	}

	///아바타 아이템 갱신
	for( unsigned int i = INVENTORY_AVATAR_BEGIN; i <= INVENTORY_AVATAR_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDefine && CheckLimit( item.GetIndex() ) == ITEMLIMIT_SUCCESS && item.mSeal == false )
		{
			AddAbility( item );

			unsigned int setIdx = item.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );
		}
	}

	/// 장착 무기류 검사
	if( mActiveWeapon == ItemActiveFront )
	{
		cItem& hLeft2 = mItemArray[INVENTORY_HAND_LEFT2];	/// 방패류 혹은 양손
		cItem& hRight2 = mItemArray[INVENTORY_HAND_RIGHT2];	/// 한손 혹은 양손

		/// 무기착용시
		if( hRight2.mDefine && CheckLimit( hRight2.GetIndex() ) == ITEMLIMIT_SUCCESS && hRight2.mSeal == false )
		{
			AddAbility( hRight2 );

			unsigned int setIdx = hRight2.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );

		}

		/// 방패착용시
		if( hLeft2.mDefine && hLeft2.mEquipType == ITEM_EQUIP_SHIELD && 
			CheckLimit( hLeft2.GetIndex() ) == ITEMLIMIT_SUCCESS && hLeft2.mSeal == false )
		{
			AddAbility( hLeft2 );

			unsigned int setIdx = hLeft2.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );

		}
	}
	else
	{
		cItem& hLeft1 = mItemArray[INVENTORY_HAND_LEFT1];	/// 방패류 혹은 양손
		cItem& hRight1 = mItemArray[INVENTORY_HAND_RIGHT1];	/// 한손 혹은 양손

		/// 무기착용시
		if( hRight1.mDefine && CheckLimit( hRight1.GetIndex() ) == ITEMLIMIT_SUCCESS && hRight1.mSeal == false )
		{
			AddAbility(  hRight1 );

			unsigned int setIdx = hRight1.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );

		}

		/// 방패착용시
		if( hLeft1.mDefine && hLeft1.mEquipType == ITEM_EQUIP_SHIELD && 
			CheckLimit( hLeft1.GetIndex() ) == ITEMLIMIT_SUCCESS && hLeft1.mSeal == false )
		{
			AddAbility( hLeft1 );

			unsigned int setIdx = hLeft1.mDefine->GetSetIndex();
			if( setIdx != 0 )
				ItemSet.NoneUniqueInsert( setIdx );
		}
	}

	/// 세트 정보 갱신
	tHashSet<unsigned long>::cIterator i = ItemSet.Begin();
	tHashSet<unsigned long>::cIterator end = ItemSet.End();
	for(; i != end; )
	{
		/// 세트 인덱스와 갯수
		unsigned long idx = (unsigned long)(*i);
		unsigned int checkCnt = ItemSet.GetCount(idx);

		cItemSetInfo* pInfo = (cItemSetInfo*)mSetInfoMap.GetAt( idx );
		if( pInfo )
		{
			/// 갯수에 따른 설정
			for( unsigned int c=1; c<=checkCnt; ++c )
			{
				sSetAblityOption* opt = pInfo->GetItemOption(c);
				if( opt )
				{
					if( opt->mOptType[0] != 0 )
					{
						AddSpecialAbility( opt->mOptType[0], opt->mOptValue[0] );
					}
					if( opt->mOptType[1] != 0 )
					{
						AddSpecialAbility( opt->mOptType[1], opt->mOptValue[1] );
					}
				}
			}
		}

		/// reset
		ItemSet.Erase(idx);

		i = ItemSet.Begin();
		end = ItemSet.End();
	}

	HERO->CalcStatus();
}

cItem* cItemManager::GetHeroRightWeapon()
{
	unsigned short slot = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_RIGHT2 : INVENTORY_HAND_RIGHT1);
	return &mItemArray[slot];
}

cItem* cItemManager::GetHeroLeftWeapon()
{
	unsigned short slot = (unsigned short)(( mActiveWeapon == ItemActiveFront ) ? INVENTORY_HAND_LEFT2 : INVENTORY_HAND_LEFT1);
	return &mItemArray[slot];
}

bool cItemManager::IsEquipWeapon()
{
	cItem& rItem0 = mItemArray[INVENTORY_HAND_RIGHT1];
	cItem& rItem1 = mItemArray[INVENTORY_HAND_RIGHT2];
	cItem& lItem0 = mItemArray[INVENTORY_HAND_LEFT1];
	cItem& lItem1 = mItemArray[INVENTORY_HAND_LEFT2];

	if( rItem0.mCount > 0 || rItem1.mCount > 0 || lItem0.mCount > 0 || lItem1.mCount > 0 )
        return true;
	return false;
}

/// 교환 : 인벤토리 슬롯 갯수가 맞아야 한다
/// 분해 : 분해될 아이템의 자리까지 포함해서 계산해야한다
/// 그외 : 현재 인벤토리내의 슬롯만 계산한다
/// 해당 아이템이 인벤에 들어갈수 있는 최대 수 구하기
unsigned int cItemManager::GetRemainCount( unsigned long itemIndex, unsigned int mincount )
{
	cItemDefine* define = GetItemDefine( itemIndex );
	if( !define )
		return 0;

	unsigned short capacity = define->GetCapacity();
	if( capacity == 0 )
	{
		assert(0);
		return 0;
	}

	unsigned int emptySlotCnt = 0;
	unsigned int itemCount = 0;
	for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
	{
		/// 빈슬롯 검사
		cItem& item = mItemArray[slot];
		if( item.GetIndex() == 0 )
		{
			emptySlotCnt++;

			if( emptySlotCnt * capacity >= mincount )
				return mincount;
		}
		/// 겹치기 검사( 캐시 아이템은 겹치기가 되질 않는다.)
		else if( item.GetIndex() == itemIndex && item.mCash == 0 )
		{
			unsigned int remainCnt = capacity - item.GetCount();
			itemCount = itemCount + remainCnt;
			if( itemCount >= mincount )
				return mincount;
		}

		unsigned int applyCnt = emptySlotCnt * capacity;
		if( itemCount + applyCnt >= mincount )
			return mincount;
	}

	itemCount += (emptySlotCnt * capacity);
	return itemCount;
}

void cItemManager::CheckItemLimitAll()
{
	for( unsigned int i = INVENTORY_EQUIP_BEGIN; i <= INVENTORY_BAG_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mCount )
		{
			int ret0 = CheckLimit( item.GetIndex() );
			int ret1 = CheckLimitEtc( item.GetIndex() );

			NiColorA color = NiColorA::WHITE;
			if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
			{
				color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}

			item.SetColor( color );
		}
	}

	for( unsigned int i = INVENTORY_WAREHOUSE_BEGIN; i <= INVENTORY_WAREHOUSE_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mCount )
		{
			int ret0 = CheckLimit( item.GetIndex() );
			int ret1 = CheckLimitEtc( item.GetIndex() );

			NiColorA color = NiColorA::WHITE;
			if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
			{
				color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}

			item.SetColor( color );
		}
	}

	// 코스튬 검사
	for( unsigned int i = INVENTORY_EQUIP_BEGIN; i <= INVENTORY_EQUIP_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mCount )
		{
			NiColorA color = NiColorA::WHITE;
			if( item.mSeal == true )
			{
				color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}

			item.SetColor( color );
		}
	}

	//
	cItem& ritem1 = mItemArray[INVENTORY_HAND_RIGHT1];
	if( ritem1.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
		int ret0 = CheckLimit( ritem1.GetIndex() );
		int ret1 = CheckLimitEtc( ritem1.GetIndex() );

		NiColorA color = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS || ritem1.mSeal == true )
			color = NiColorA( 0.5f, 0.0f, 0.0f, 1.0f );

		cItem& litem1 = mItemArray[INVENTORY_HAND_LEFT1];
		litem1.SetColor( color );
	}

	cItem& ritem2 = mItemArray[INVENTORY_HAND_RIGHT2];
	if( ritem2.mEquipType == ITEM_EQUIP_TWO_HANDS )
	{
		int ret0 = CheckLimit( ritem2.GetIndex() );
		int ret1 = CheckLimitEtc( ritem2.GetIndex() );

		NiColorA color = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS || ritem2.mSeal == true )
			color = NiColorA( 0.5f, 0.0f, 0.0f, 1.0f );

		cItem& litem2 = mItemArray[INVENTORY_HAND_LEFT2];
		litem2.SetColor( color );
	}

	// 코스튬
	cItem& bodyItem = mItemArray[INVENTORY_COSTUME_BODY1];
	if( bodyItem.mDefine && bodyItem.mDefine->GetSubType() == ITEM_WEAR_COS_ONEPIECE )
	{
		int ret0 = CheckLimit( bodyItem.GetIndex() );
		int ret1 = CheckLimitEtc( bodyItem.GetIndex() );

		NiColorA color = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS || bodyItem.mSeal == true )
			color = NiColorA( 0.5f, 0.0f, 0.0f, 1.0f );

		cItem& item0 = mItemArray[INVENTORY_COSTUME_BODY2];
		cItem& item1 = mItemArray[INVENTORY_COSTUME_HAND];
		cItem& item2 = mItemArray[INVENTORY_COSTUME_FOOT];

		item0.SetColor( color );
		item1.SetColor( color );
		item2.SetColor( color );
	}
	
	// 퀵슬롯도 갱신
	QUICKMAN->CheckItemIcon();
}

int cItemManager::CheckLimit( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return ITEMLIMIT_SUCCESS;

	cItemDefine* itemDefine = GetItemDefine( itemIndex );
	if( !itemDefine )
	{
		assert(0);
		return ITEMLIMIT_ERROR;
	}

	cItemLimit* itemLimit = itemDefine->GetLimit();
	if( itemLimit )
	{
		/// 종족 검사
		if( itemLimit->GetRace() != ITEM_RACE_ALL && itemLimit->GetRace() != HERO->GetRace() )
			return ITEMLIMIT_RACE;

		/// 성별
		if( itemLimit->GetGender() != ITEM_GENDER_ALL && itemLimit->GetGender() != HERO->GetGender() )
			return ITEMLIMIT_GENDER;

		// 레벨 검사
		if( HERO->GetLevel() < itemLimit->GetLevel() )
			return ITEMLIMIT_LEVEL;

		// 최대 레벨 검사
		if( HERO->GetLevel() > itemLimit->GetMaxLevel() )
			return ITEMLIMIT_LEVEL;

		// 사용 가능한 맵 검사
		switch ( itemLimit->GetUseMapType() )
		{
		case eUSE_MAP_PVP: /// 전장
			{
				if( PVPMAN->IsEnableBattle() == false )
					return ITEMLIMIT_PVPONLY;
			}
			break;
		case eUSE_MAP_TUTORIAL: // 튜토리얼전용
			{
				if( TUTORIALMAN->IsTutorialMode() == false )
					return ITEMLIMIT_TUTORIALONLY;
			}
			break;
		case eUSE_MAP_FIELD: /// 필드
			{
				unsigned short mapIndex = HERO->GetMapNum();
				if( mapIndex < MAP_MIN || mapIndex > MAP_MAX )
					return ITEMLIMIT_FIELDONLY;
			}
		}

		/// 세력 검사
		if( itemLimit->GetForceType() != eFORCETYPE_NONE && itemLimit->GetForceType() != HERO->GetForceType() )
			return ITEMLIMIT_FORCETYPE;

		/// 전문 직업
		if( itemLimit->GetMakeSkill() > 0 )
		{
			sMakeSkill* mySkill = 0;

			// 익힌 스킬인지 검사
			sHeroInfo* info = HERO->GetHeroInfo();
			if( itemLimit->GetMakeSkill() == info->MakeSkill1.mIndex )
			{
				mySkill = &info->MakeSkill1;
			}
			else if( itemLimit->GetMakeSkill() == info->MakeSkill2.mIndex )
			{
				mySkill = &info->MakeSkill2;
			}

			// 해당 보조직업 익혔는지 검사
			if( mySkill == 0 )
				return ITEMLIMIT_MAKESKILL;	

			/// 보조 직업 숙련도가 있고
			if( itemLimit->GetMakeSkillExp() > 0 )
			{
				/// 캐릭터 보조직업 숙련도와 아이템 보조직업 숙련도 체크
				if( mySkill->mExp < itemLimit->GetMakeSkillExp() )
					return ITEMLIMIT_MAKESKILLEXP;
			}		

			if( itemDefine->GetType() == ITEM_MIX_SKILL )
			{
				switch( itemDefine->GetSubType() )
				{
					// 증서
				case ITEM_MIX_SKILL_L2:
				case ITEM_MIX_SKILL_L3:
				case ITEM_MIX_SKILL_L4:
					{
						char reqStep = itemDefine->GetSubType()-1;
						if( reqStep < 0 )
						{
							assert(0);
							return ITEMLIMIT_ERROR;
						}

						// 해당스킬의 전 단계와 맞는지 검사
						if( mySkill->mStep < reqStep )
							return ITEMLIMIT_MIXSKILLSTEP;
					}
					break;
					// 레시피
				case ITEM_MIX_RECIPE:
					{
						// 단계 & 숙련도 체크
						sMakeSkillScript* script = MAKESKILLSCRIPT->GetMakeSkill( itemIndex );
						if( script )
						{
							cItemLimit* pLimit = itemDefine->GetLimit();
							if( !pLimit )
							{
								assert(0);
								return ITEMLIMIT_ERROR;
							}
							// 자신의 등급과 검사
							if( mySkill->mStep < script->mMakeStep ) 
								return ITEMLIMIT_MIXRECIPESTEP;

							// 숙련도 비교
							if( mySkill->mExp < pLimit->GetMakeSkillExp() )
								return ITEMLIMIT_MIXRECIPEEXP;
						}
						else
							assert(0);
					}
					break;
					// 레시피 그룹
				case ITEM_MIX_RECIPEGROUP:
					{
						cItemAbility* abil = itemDefine->GetAbililty();
						if( abil )
						{
							// 단계 & 숙련도 체크
							sGroupInfo* groupInfo = MAKESKILLSCRIPT->GetGroupInfo( abil->GetInfluenceIndex() );
							if( groupInfo )
							{
								// 자신의 등급과 검사
								if( mySkill->mStep < groupInfo->mStep ) 
									return ITEMLIMIT_MIXRECIPESTEP;

								// 숙련도 비교
								if( mySkill->mExp < groupInfo->mExp )
									return ITEMLIMIT_MIXRECIPEEXP;
							}
							else
								assert(0);
						}
						else
							assert(0);
					}
					break;
				default:
					{
						// 레벨 1짜리는 보조직업 제한이 없어야 한다!
						assert(0);
					}
					break;
				}
			}
		}		

		/// 직업 검사
		if( CheckLimitJob( itemLimit ) == false )
			return ITEMLIMIT_JOB;
	}

	return ITEMLIMIT_SUCCESS;
}

// 제한 스크립트에 없는 다른 데이터 검사
int cItemManager::CheckLimitEtc( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return ITEMLIMIT_SUCCESS;

	cItemDefine* itemDefine = GetItemDefine( itemIndex );
	if( !itemDefine )
	{
		assert(0);
		return ITEMLIMIT_ERROR;
	}

	// 이미 습득한 호칭 검사
	if( itemDefine->GetTitleIndex() )
	{
		if( TITLEMAN->IsHaveTitle( itemDefine->GetTitleIndex() ) == true )
			return ITEMLIMIT_HAVETITLE;
	}

	return ITEMLIMIT_SUCCESS;
}

bool cItemManager::CheckLimitJob( cItemLimit* itemLimit )
{
	if( itemLimit == 0 )
		return true;

	unsigned long job1 = itemLimit->GetJob1();
	unsigned long job2 = itemLimit->GetJob2();
	if( job1 != ePLAYER_NONEJOB && job2 != ePLAYER_NONEJOB )
	{
		unsigned long curJob = HERO->GetJob();
		if( job1 == curJob || job2 == curJob )
			return true;

		tArray<unsigned long>* parentJobArr = GAMERESOURCEMAN->GetParentJob( curJob );
		if( parentJobArr )
		{
			for( unsigned int i=0;i<parentJobArr->GetSize(); ++i )
			{
				if( job1 == (*parentJobArr)[i] || job2 == (*parentJobArr)[i] )
					return true;
			}
		}

		return false;
	}
	return true;
}

/// 아이템 구입시 조건검사
bool cItemManager::CheckBuyItem(unsigned long itemIndex, unsigned int count)
{
	cItemDefine* define = GetItemDefine( itemIndex );
	if( !define )
		return false;

	/// 소지금 검사
	unsigned long price = define->GetBuyPrice() * count;
	
	/// 현재 NPC가 판매 비율이 적용되어 있는지 체크.
	cNpcStoreWindow* npcStoreWin = GAMEUI->GetNpcStore();
	if( npcStoreWin )
	{
		unsigned long sellRate = npcStoreWin->GetCurrentItemPriceRate();
		if( sellRate == ULONG_MAX )
		{	
			sellRate = 100;
		}
		price = count * ( define->GetBuyPrice() * sellRate / 100 );
	}

	switch( define->GetBuyType() )
	{
	case ItemBuyNone:
		{
			assert(0);
			return false;
		}
	case ItemBuyMoney:
		if( HERO->GetMoney() < price )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 200 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return false;
		}
		break;
	case ItemBuyTarot:
		if( HERO->GetTarotPoint() < price )				/// 090216 PKH ItemBuyPvp 위치 바꿈
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 197 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return false;
		}
		break;
	case ItemSellPvp:
		{
			/// 어떤 전장 포인트가 소모되야 하는지는 대화중인 npc의 탭의 selltype() 
			cNpcStoreWindow* win = GAMEUI->GetNpcStore();
			if( win )
			{
				unsigned char forceType = eFORCETYPE_NONE;
				unsigned char npcSellType = win->GetCurrentItemSellType();
				switch( npcSellType )
				{
				case eNPC_ITEM_SELL_TYPE_FIRE:  forceType = eFORCETYPE_FIRE;	break;
				case eNPC_ITEM_SELL_TYPE_WATER: forceType = eFORCETYPE_WATER;	break;
				case eNPC_ITEM_SELL_TYPE_WIND:  forceType = eFORCETYPE_WIND;	break; 
				case eNPC_ITEM_SELL_TYPE_EARTH: forceType = eFORCETYPE_EARTH;	break;
				default: 
					{
						assert(0);
						return false;
					}
					break;
				}
			
				if( HERO->GetPVPPoint( forceType ) < price )		
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 198 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
					return false;
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
		break;
	default:
		{
			/// 코인류 - 코인일때는 GetBuyType() 이 아이템 인덱스임.
			unsigned long needItemIndex = define->GetBuyType();
			cItemDefine* pNeedItemDefine = ITEMMAN->GetItemDefine( needItemIndex );
			if( pNeedItemDefine == NULL )
			{
				assert(0);
				return false;
			}

			if( pNeedItemDefine->GetType() == ITEM_COIN )
			{
				sItemCount* p = GetItemCount( needItemIndex );
				unsigned long haveCount = ( p ) ? p->bag : 0;
				if( haveCount < price )
				{
					int errorIndex = 199;
					if( pNeedItemDefine->GetSubType() == ITEM_COIN_GOLD || 
						pNeedItemDefine->GetSubType() == ITEM_COIN_SILVER || 
						pNeedItemDefine->GetSubType() == ITEM_COIN_COPPER )
						errorIndex = 199;

					else if( pNeedItemDefine->GetSubType() == ITEM_COIN_MEDAL_OLD    ||
							 pNeedItemDefine->GetSubType() == ITEM_COIN_MEDAL_NORMAL || 
							 pNeedItemDefine->GetSubType() == ITEM_COIN_MEDAL_SILVER ||
							 pNeedItemDefine->GetSubType() == ITEM_COIN_MEDAL_GOLD )
							 errorIndex = 12405;
					else
					{
						assert(0);
						return false;
					}

					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( errorIndex ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
					return false;
				}
			}
			else
			{
				assert(0);
				return false;
			}
		}
		break;
	}

	/// 슬롯 full 검사 
	unsigned short slotIndex = 0;
	FindEmptySlot( &slotIndex, INVENTORY_BAG_BEGIN, HERO->GetBagEndSlot() );
	if( slotIndex == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 202 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
		return false;
	}

	/// max inven 체크 ( 갯수 제한 있을때만 )
	if( define->GetMaxHaveInven() > 0 )
	{
		sItemCount* p = GetItemCount( itemIndex );
		int haveCnt = ( p ) ? p->total : 0;
		if( haveCnt + count > (int)define->GetMaxHaveInven() )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 194 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}
	}

	/// 무게체크
	float wei = mCurWeight + (define->GetWeight() * count);
	if( wei > mMaxWeight )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 208 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
		return false;
	}

	return true;
}

/// 분해시 조건검사
bool cItemManager::CheckDisjoint( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
		return false;

	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
	{
		cItemDefine* itemDefine = item->GetDefine();
		if( !itemDefine )
			return false;

		/// 분해 결과 아이템 정보
		sItemDisjoint* data = itemDefine->GetDisjoint();
		if( !data )
			return false;

		/// 1. 비용 검사
		if( HERO->GetMoney() < data->mPrice )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 200 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return false;
		}

		/// 2. 소지가능 검사
		int needCnt = 0;
		float totalItemWei = 0;
		for( int i = 0 ; i < MAX_DISJOINT; ++i, ++needCnt )
		{
			unsigned long itemIndex = data->mDisjoint[i].itemIndex;
			unsigned int  count		= data->mDisjoint[i].count;

			if( itemIndex == 0 )
				break;

			cItemDefine* define = GetItemDefine( itemIndex );
			if( !define )
			{
				assert(0);
				return false;
			}

			/// max inven 체크 ( 갯수 제한 있을때만 )
			if( define->GetMaxHaveInven() > 0 )
			{
				sItemCount* p = GetItemCount( itemIndex );
				int haveCnt = ( p ) ? p->total : 0;
				if( haveCnt + count > (int)define->GetMaxHaveInven() )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 193 ),
						GAMERESOURCEMAN->GetGameText( 301 ) );
					return false;
				}
			}
			totalItemWei += (define->GetWeight() * count);
		}

		/// 무게체크
		float wei = mCurWeight + totalItemWei;
		if( wei > mMaxWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 12402 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return false;
		}

		/// 3. 슬롯 체크
		int emptySlotCnt = 1;	/// 분해하는 아이템이 있으므로 기본 슬롯 1 더하기
		for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
		{
			/// 빈슬롯 검사
			cItem& item = mItemArray[slot];
			if( item.GetIndex() == 0 )
			{
				emptySlotCnt++;

				/// 빈슬롯이 더 많으면 통과
				if( emptySlotCnt >= needCnt )
					return true;
			}
		}

		for( int i = 0; i < needCnt; ++i )
		{
			unsigned long itemIndex = data->mDisjoint[i].itemIndex;
			int			  count		= data->mDisjoint[i].count;

			cItemDefine* define = GetItemDefine( itemIndex );
			if( !define )
			{
				assert(0);
				continue;
			}

			for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
			{
				cItem& item = mItemArray[slot];

				/// 겹치는 슬롯
				if( item.GetIndex() == itemIndex )
				{
					int remainCnt = define->GetCapacity() - item.GetCount();
					count -= remainCnt;

					if( count <= 0 )
						break;
				}
			}

			/// 다 못찾으면 빈슬롯 사용하기
			if( count > 0 )
			{
				emptySlotCnt--;
				if( emptySlotCnt < 0 )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 202 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
					return false;
				}
			}
		}
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

/// 아이템 변화시 모든 정보 갱신
void cItemManager::UpdateItem()
{
	/// 무게갱신
	CheckWeight();

	/// 총 갯수 갱신
	CheckItemCount();
}

/// 인벤토리 내부 모든 아이템 무게 검사
void cItemManager::CheckWeight()
{
	/// 인벤토리 무게 검사
	unsigned int totalWei = 0;
	for( unsigned int i = INVENTORY_CHARACTER_BEGIN; i <= INVENTORY_CHARACTER_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDefine )
		{
			totalWei += item.mDefine->GetWeight() * item.mCount;
		}
	}

	mCurWeight = (float)totalWei;

	/// ui 갱신
	if( mWeightGauge )
		mWeightGauge->SetGaugeNoAni( mCurWeight );
	else
		assert(0);
}

void cItemManager::SetMaxWeightGauge( float max )
{
	if( mWeightGauge )
	{
		mMaxWeight = max;
		mWeightGauge->SetMaxValue( max );

		if( mWeightGauge )
			mWeightGauge->SetGaugeNoAni( mCurWeight );
		else
			assert(0);
	}
	else
	{
		assert(0);
	}
}

/// 아이템 총갯수 검사
void cItemManager::CheckItemCount()
{
	ClearItemCount();

	// bag
	for( unsigned int i = INVENTORY_CHARACTER_BEGIN; i <= INVENTORY_CHARACTER_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDefine && item.IsSeal() == false )
		{
			sItemCount* p = GetItemCount( item.mIndex );
			if( p == NULL )
			{
				p = new sItemCount;
				if( mItemCountMap.Insert( item.mIndex, p ) == false )
				{
					assert(0 && "error to insert item total count");
					SAFE_DELETE( p );
					continue;
				}
			}

			p->bag += item.mCount;
			p->total += item.mCount;
		}
	}

	// etc
	for( unsigned int i = INVENTORY_QUEST_BEGIN; i <= INVENTORY_COLLECTION_END; ++i )
	{
		cItem& item = mItemArray[i];
		if( item.mDefine && item.IsSeal() == false )
		{
			sItemCount* p = GetItemCount( item.mIndex );
			if( p == NULL )
			{
				p = new sItemCount;
				if( mItemCountMap.Insert( item.mIndex, p ) == false )
				{
					assert(0 && "error to insert item total count");
					SAFE_DELETE( p );
					continue;
				}
			}

			p->etc += item.mCount;
			p->total += item.mCount;
		}
	}

	/// 퀵슬롯의 총갯수 갱신
	QUICKMAN->CheckAllIcon();

	/// 아이템 조합중일경우만 갱신
	cItemMixWindow* win = GAMEUI->GetItemMixWindow();
	if( win && win->IsVisible() )
	{
		win->UpdateRecipeList();
		win->UpdateItemCount();		
	}

	/// 강화중일경우에만 갱신
	cEnhancedWindow* enhance = GAMEUI->GetEnhancedWindow();
	if( enhance && enhance->IsVisible() )
	{
		enhance->UpdateCardCount();
		enhance->UpdateCashProtectCount();
	}
}

bool cItemManager::CreateDropItem( MSG_ITEM_INFO* dropMsg )
{
	if( dropMsg == 0 )
		return false;

	cDropItemSceneNodeParam param;
	param.mDropIndex = dropMsg->itemDrop.idx;
	param.mDropEndPos = NiPoint2( dropMsg->itemDrop.xPos, dropMsg->itemDrop.yPos );
	param.mTimeToOwn = dropMsg->timeToOwner;
	::memcpy( param.mOwner, dropMsg->owner, sizeof(unsigned long)*MAX_ITEM_OWNER );

	param.mTimeToParty = dropMsg->timeToParty;
	::memcpy( param.mParty, dropMsg->party, sizeof(unsigned long)*MAX_ITEM_PARTY );

	param.mIsRareItem = (dropMsg->rare >= eTOOLTIP_RARE) ? true : false;

	cSceneNode* node = SCENEMAN->CreateDropItem( param );
	if( node == 0 )
		return false;

	if( mDropItemMap.Insert( dropMsg->itemDrop.idx, node->GetIndexByManger() ) == false )
	{
		assert( 0 && "failed to insert dropitem define, maybe already exist" );
		return false;
	}

	return true;
}

void cItemManager::DeleteDropItem( unsigned long idx )
{
	unsigned long nodeIdx = (unsigned long)mDropItemMap.GetAt( idx );
	cDropItemSceneNode* node = SCENEMAN->GetDropItemSceneNode( nodeIdx );
	if( node )
		SCENEMAN->DestroyDropItemNode( node );

	mDropItemMap.Erase( idx );
}

void cItemManager::UpdateTutorialDrop()
{
	cDropItemMap::cIterator i = mDropItemMap.Begin();
	cDropItemMap::cIterator end = mDropItemMap.End();

	for( ; i != end; ++i )
	{
		unsigned long nodeIndex = (unsigned long)(i->mSecond);
		cSceneNode* node = SCENEMAN->GetManagedStaticNode( cSceneNode::eDROPITEM, nodeIndex );
		if( node )
		{
			((cDropItemSceneNode*)node)->UpdateTutorialBoard();
		}
	}
}

void cItemManager::ClearTutorialDrop()
{
	cDropItemMap::cIterator i = mDropItemMap.Begin();
	cDropItemMap::cIterator end = mDropItemMap.End();

	for( ; i != end; ++i )
	{
		unsigned long nodeIndex = (unsigned long)(i->mSecond);
		cSceneNode* node = SCENEMAN->GetManagedStaticNode( cSceneNode::eDROPITEM, nodeIndex );
		if( node )
		{
			((cDropItemSceneNode*)node)->ClearTutorialBoard();
		}
	}
}

//bool cItemManager::ProcessMouseOver( int mouseX, int mouseY )
//{
//	tArray<void*> CollidableArray;
//	CollidableArray.Clear();
//
//	if( SCENEMAN->PickDropItem(&CollidableArray, mouseX, mouseY, true) == true )
//	{
//		cDropItemSceneNode* n = (cDropItemSceneNode*)CollidableArray[0];
//		if( n == 0 )
//			return false;
//
//		return true;
//	}
//	return false;
//}


//bool cItemManager::ProcessLButtonClick( int mouseX, int mouseY )
//{
//	tArray<void*> CollidableArray;
//	CollidableArray.Clear();
//
//	if( SCENEMAN->PickDropItem(&CollidableArray, mouseX, mouseY, true) == true )
//	{
//		cDropItemSceneNode* n = (cDropItemSceneNode*)CollidableArray[0];
//		if( n == 0 )
//			return false;
//
//		if( n->IsHeroOwner() )
//		{
//			unsigned long idx = n->GetDropIndex();
//			HERO->AfterItemPick( idx, n->GetWorldTranslate().x, n->GetWorldTranslate().y );
//		}
//		else
//		{
//			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 211 ) );
//		}
//
//		return true;
//	}
//	return false;
//}
//
//bool cItemManager::ProcessLButtonDClick( int mouseX, int mouseY )
//{
////	if( HERO->GetState() == ePLAYER_STATE_ITEMPICK )
////		return false;
//
//	tArray<void*> CollidableArray;
//	CollidableArray.Clear();
//
//	if( SCENEMAN->PickDropItem(&CollidableArray, mouseX, mouseY, true) == true )
//	{
//		cDropItemSceneNode* n = (cDropItemSceneNode*)CollidableArray[0];
//		if( n == 0 )
//			return false;
//
//		unsigned long idx = n->GetDropIndex();
//		HERO->AfterItemPick( idx, n->GetWorldTranslate().x, n->GetWorldTranslate().y );
//		return true;
//	}
//
//	return false;
//}

void cItemManager::GetTooltipColor( unsigned short tiptype, unsigned long& color )
{
	color = COLOR_WHITE;
	switch( tiptype )
	{
	case eTOOLTIP_MALL:		color = COLOR_MALL;		break;
	case eTOOLTIP_ETC:		color = COLOR_ETC;			break;
	case eTOOLTIP_NORMAL:	color = COLOR_WHITE;		break;
	case eTOOLTIP_MAGIC:	color = COLOR_MAGIC;		break;
	case eTOOLTIP_RARE:		color = COLOR_RARE;		break;
	case eTOOLTIP_UNIQUE:	color = COLOR_UNIQUE;		break;
	case eTOOLTIP_EPIC:		color = COLOR_EPIC;		break;
	case eTOOLTIP_REGEND:	color = COLOR_REGEND;		break;
	}
}

unsigned long cItemManager::GetPriceUnit( unsigned long type, unsigned long itemIndex )
{

	/// 테르 기본 설정
	unsigned long index = 2000091;

	switch( type )
	{
		case eNPC_ITEM_SELL_TYPE_MONEY_OR_COIN:
			{
				/// 코인류
				cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
				if( itemDefine && itemDefine->GetBuyType() != ItemSellMoney )
				{
					cItemDefine* coinDefine = ITEMMAN->GetItemDefine( itemDefine->GetBuyType() );
					if( coinDefine )
					{
						switch( coinDefine->GetSubType() )
						{
						case ITEM_COIN_GOLD:	index = 2000092; break;
						case ITEM_COIN_SILVER:	index = 2000093; break;
						case ITEM_COIN_COPPER:	index = 2000094; break;
						case ITEM_COIN_MEDAL_OLD:    index = 2000108; break;
						case ITEM_COIN_MEDAL_NORMAL: index = 2000109; break;	
						case ITEM_COIN_MEDAL_SILVER: index = 2000110; break;
						case ITEM_COIN_MEDAL_GOLD:   index = 2000111; break;
						}
					}
					else
						index = 2000091;
				}
				else
					index = 2000091;
			}
			break;
		case eNPC_ITEM_SELL_TYPE_FIRE:  index = 2000104;	break;
		case eNPC_ITEM_SELL_TYPE_WATER: index = 2000105;	break;
		case eNPC_ITEM_SELL_TYPE_WIND:  index = 2000106;	break; 
		case eNPC_ITEM_SELL_TYPE_EARTH: index = 2000107;	break;
	}
	return index;
}

unsigned long cItemManager::GetPriceUnit( unsigned long type )
{
	unsigned long index = 0;
	switch( type )
	{
	case ItemSellNone:	break;						// ItemBuyNone  와 동일
	case ItemSellMoney:		index = 2000091; break;		// ItemBuyMoney 와 동일
	case ItemSellTarot:		index = 2000096; break;		// ItemBuyTarot	와 동일
	case ItemSellPvp:
		{
			/// 어떤 전장 포인트가 소모되야 하는지는 대화중인 npc의 탭의 selltype() 
			cNpcStoreWindow* win = GAMEUI->GetNpcStore();
			if( win )
			{
				unsigned char npcSellType = win->GetCurrentItemSellType();
				switch( npcSellType )
				{
				case eNPC_ITEM_SELL_TYPE_FIRE:  index = 2000104;	break;
				case eNPC_ITEM_SELL_TYPE_WATER: index = 2000105;	break;
				case eNPC_ITEM_SELL_TYPE_WIND:  index = 2000106;	break; 
				case eNPC_ITEM_SELL_TYPE_EARTH: index = 2000107;	break;
				default: assert(0); break;
				}
			}
			else
				assert(0);
		}
		break;
	default:
		{
			/// 코인류
			cItemDefine* coinDefine = ITEMMAN->GetItemDefine( type );
			if( coinDefine )
			{
				switch( coinDefine->GetSubType() )
				{
				case ITEM_COIN_GOLD:	index = 2000092; break;
				case ITEM_COIN_SILVER:	index = 2000093; break;
				case ITEM_COIN_COPPER:	index = 2000094; break;
				case ITEM_COIN_MEDAL_OLD:    index = 2000108; break;
				case ITEM_COIN_MEDAL_NORMAL: index = 2000109; break;	
				case ITEM_COIN_MEDAL_SILVER: index = 2000110; break;
				case ITEM_COIN_MEDAL_GOLD:   index = 2000111; break;
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	}

	return index;
}


bool cItemManager::CheckNearestDropItem()
{
	if( HERO->IsDie() == true )
		return false;
	if( HERO->IsJumping() == true )
		return false;

	if( mDropItemMap.IsEmpty() == true )
		return false;

	if( IsLocked() == true )
		return false;

	if( CheckOpenTime() == false )
		return false;

	cDropItemMap::cIterator i = mDropItemMap.Begin();
	cDropItemMap::cIterator iend = mDropItemMap.End();

	NiPoint3 heroPos = HERO->GetPos();
	float dist = 0;
	cDropItemSceneNode* nearestDropItem = 0;
	float nearestDist = FLT_MAX;
	for( ; i != iend; ++i )
	{
		unsigned long nodeIdx = (unsigned long)(i->mSecond);
		cDropItemSceneNode* n = SCENEMAN->GetDropItemSceneNode( nodeIdx );
		if( n==0 || n->IsHeroOwner() == false )
			continue;

		dist = (heroPos - n->GetWorldTranslate()).Length();

		if( dist <= 1000.0f )
		{
			if( dist < nearestDist )
			{
				nearestDist = dist;
				nearestDropItem = n;
			}
		}
	}

	if( nearestDropItem )
	{
		unsigned long idx = nearestDropItem->GetDropIndex();
		HERO->AfterItemPick( idx, nearestDropItem->GetWorldTranslate().x, nearestDropItem->GetWorldTranslate().y );
		return true;
	}
	return false;
}

bool cItemManager::CheckOpenTime() 
{
	if( THEAPP->GetWorldAccumTime() - mLastAccutime > OPEN_TIME )
	{
		mLastAccutime = THEAPP->GetWorldAccumTime();
		return true;
	}
	return false;
}

//ItemActiveFront = left2, right2
//ItemActiveBack = left1, right1
void cItemManager::SetSwitchWeapon( eItemActiveWeapon activeWeapon, bool onSound )
{
	/// 
	if( !(activeWeapon == ItemActiveFront || activeWeapon == ItemActiveBack) )
	{
		assert(0);
		return;
	}

	cItem& rItem0 = mItemArray[INVENTORY_HAND_RIGHT1];
	cItem& rItem1 = mItemArray[INVENTORY_HAND_RIGHT2];
	cItem& lItem0 = mItemArray[INVENTORY_HAND_LEFT1];
	cItem& lItem1 = mItemArray[INVENTORY_HAND_LEFT2];

	/// 저장
	mActiveWeapon = activeWeapon;

	/// 위치 변경
	if( mActiveWeapon == ItemActiveFront )
	{
		rItem1.SwicthPosition( mFrontRight );
		lItem1.SwicthPosition( mFrontLeft );

		rItem0.SwicthPosition( mBackRight );
		lItem0.SwicthPosition( mBackLeft );

		/// 사운드 플레이
		if( onSound )
		{
			PlayIconSettingSound( INVENTORY_HAND_RIGHT2 );
			PlayIconSettingSound( INVENTORY_HAND_LEFT2 );
		}

	}
	else if( mActiveWeapon == ItemActiveBack )
	{
		rItem0.SwicthPosition( mFrontRight );
		lItem0.SwicthPosition( mFrontLeft );

		rItem1.SwicthPosition( mBackRight );
		lItem1.SwicthPosition( mBackLeft );

		/// 사운드 플레이
		if( onSound )
		{
			PlayIconSettingSound( INVENTORY_HAND_RIGHT1 );
			PlayIconSettingSound( INVENTORY_HAND_LEFT1 );
		}
	}
	else
	{
		assert(0);
	}

	/// 전체 능력치검사
	CheckEquipAbility();

	CheckItemSlot( INVENTORY_HAND_RIGHT1 );
	CheckItemSlot( INVENTORY_HAND_RIGHT2 );

	ChangeSelectSlot( INVENTORY_HAND_RIGHT1 );
	ChangeSelectSlot( INVENTORY_HAND_RIGHT2 );
	
}

void cItemManager::SetWeaponIconPos( unsigned short slotIndex, cUIRect rect )
{
	if( slotIndex == INVENTORY_HAND_LEFT1 )
	{
		mBackLeft = rect;
	}
	else if( slotIndex == INVENTORY_HAND_RIGHT1 )
	{
		mBackRight = rect;
	}
	else if( slotIndex == INVENTORY_HAND_LEFT2 )
	{
		mFrontLeft = rect;
	}
	else if( slotIndex == INVENTORY_HAND_RIGHT2 )
	{
		mFrontRight = rect;
	}
}

void cItemManager::PlayIconSettingSound( unsigned short slotIndex )
{
	cItem* item = ITEMMAN->GetItem( slotIndex );	

	if( item == NULL || item->GetCount() == 0 )
		return;
	
	cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( item->GetIndex() );

	if( pItemDefine == NULL )
	{
		assert(0);
		return;
	}

	/// 기본적으로 잡화(기타) 종류 사운드 설정
	unsigned long soundIndex = 3036; 

	switch( pItemDefine->GetType() )
	{
	case ITEM_WEAPON : 
		{
			switch( pItemDefine->GetSubType() )
			{
			case ITEM_WEAPON_SWORD : //soundIndex = 3025;	break; ///한손검, 이도는 동일한 사운드
			case ITEM_WEAPON_DUAL  : soundIndex = 3025;	break; /// 이도
			case ITEM_WEAPON_BLADE : soundIndex = 3026;	break; /// 양손검
			case ITEM_WEAPON_CUTTER: soundIndex = 3027;	break; /// 단검
			case ITEM_WEAPON_STAFF : soundIndex = 3028;	break; /// 지팡이
			case ITEM_WEAPON_GUN   : soundIndex = 3029;	break; /// 총
			case ITEM_WEAPON_SHIELD: soundIndex = 3030;	break; /// 방패
			default: assert(0); break;
			}
		}
		break;
	case ITEM_WEAR : 
		{
			unsigned char subtype = pItemDefine->GetSubType();
			if( subtype >= ITEM_WEAR_HEAD && subtype <= ITEM_WEAR_ONEPIECE )
			{
				cItemAbility* pItemAbility = pItemDefine->GetAbililty();
				if( pItemAbility )
				{
					switch( pItemAbility->GetDefenseType() )
					{
					case ITEM_MATERIAL_CLOTH   : soundIndex = 3031; break; ///   천 방어구
					case ITEM_MATERIAL_LEATHER : soundIndex = 3032; break; /// 가죽 방어구
					case ITEM_MATERIAL_METAL   : soundIndex = 3033; break; /// 금속 방어구
					default : assert(0); break;
					}
				}
				else 
					assert( 0 && "Fail to get ability " );
			}
			else if( subtype >= ITEM_WEAR_COS_ONEPIECE && subtype <= ITEM_WEAR_COS_FEET )
			{
				/// 임시적용:코스튬 아이템 장착사운드는 천 방어구 사운드로 통일 [1/28/2010 Jo_Client]
				soundIndex = 3031;
				break;
			}
		}
		break;
	case ITEM_ACCESSORY : soundIndex = 3034;	break;
	case ITEM_CARD :      soundIndex = 3035;	break;
	default : 
		{
			/// 잡화,기타는 인덱스로(3036) 기본 셋팅되어 있는 값을 사용.
		}
		break;
	}

	SOUNDSYS->Play2DSound( soundIndex );
}
