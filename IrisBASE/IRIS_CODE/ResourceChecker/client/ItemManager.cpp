#include "stdafx.h"
#include "ItemManager.h"
#include "TarotManager.h"

#include "UIText.h"
#include "Gauge.h"

#include "Application.h"

#include "ItemDefine.h"

#define ITEM_FILENAME_FILENAME		"./Script/Resource/Item_FileName.txt"
#define ITEM_SETINFO_FILENAME		"./Script/Resource/Item_Set.txt"
#define ITEM_DEFINE_FILENAME		"./Script/Resource/Item_Define.txt"
#define ITEM_ABILITY_FILENAME		"./Script/Resource/Item_Ability.txt"
#define ITEM_LIMIT_FILENAME			"./Script/Resource/Item_Limit.txt"
#define ITEM_CARD_FILENAME			"./Script/Resource/Item_Card.txt"
#define ITEM_TAROT_FILENAME			"./Script/Resource/Item_Tarot.txt"
#define ITEM_SPREAD_FILENAME		"./Script/Resource/Item_Spread.txt"
#define ITEM_NPCSTORE_FILENAME		"./Script/Resource/Item_NpcStore.txt"
#define ITEM_DISJOINT_FILENAME		"./Script/Resource/Item_Disjoint.txt"
#define ITEM_ENHANCED_FILENAME		"./Script/Resource/Item_Enhanced.txt"
#define ITEM_CARDSLOT_FILENAME		"./Script/Resource/Item_Card_Slot.txt"
#define ITEM_DEALCATEGORY_FILENAME	"./Script/Resource/Item_DealCategoryTable.txt"

cItemManager* cItemManager::mSingleton = 0;

cItemManager::cItemManager()
: mDropItemMap( 200 )
, mLocked( false )
, mWeightGauge( 0 )
, mMaxWeight( 0.0f )
, mCurWeight( 0.0f )
//, mLinkedIcon( 0 )
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
// 	for( unsigned int i = MIN_INVENTORY; i < LEN_INVENTORY; ++i )
// 	{
// 		mFlagArray[i] = ITEM_FLAG_ALL;
// 	}
// 
// 	mFlagArray[INVENTORY_WEAR_HAT] = ITEM_FLAG_HEAD;
// 	mFlagArray[INVENTORY_WEAR_BODY1] = ITEM_FLAG_UPPER;
// 	mFlagArray[INVENTORY_WEAR_BODY2] = ITEM_FLAG_LOWER;
// 	mFlagArray[INVENTORY_WEAR_HAND] = ITEM_FLAG_HANDS;
// 	mFlagArray[INVENTORY_WEAR_FOOT] = ITEM_FLAG_FEET;
// 
// 	mFlagArray[INVENTORY_WEAR_EARRING] = ITEM_FLAG_EARRING;
// 	mFlagArray[INVENTORY_WEAR_NECKLACE] = ITEM_FLAG_NECKLACE;
// 	mFlagArray[INVENTORY_WEAR_BROOCH] = ITEM_FLAG_BROOCH;
// 	mFlagArray[INVENTORY_WEAR_BRACELET] = ITEM_FLAG_BRACELET;
// 	mFlagArray[INVENTORY_WEAR_RING] = ITEM_FLAG_RING;
// 
// 	mFlagArray[INVENTORY_WEAR_CARD1] = ITEM_FLAG_EQUIPCARD;
// 	mFlagArray[INVENTORY_WEAR_CARD2] = ITEM_FLAG_EQUIPCARD;
// 	mFlagArray[INVENTORY_WEAR_CARD3] = ITEM_FLAG_EQUIPCARD;
// 
// 	mFlagArray[INVENTORY_HAND_LEFT1] = ITEM_FLAG_SHIELD;
// 	mFlagArray[INVENTORY_HAND_RIGHT1] = ITEM_FLAG_SWORD | ITEM_FLAG_BLADE | ITEM_FLAG_CUTTER | ITEM_FLAG_GUN | ITEM_FLAG_STAFF | ITEM_FLAG_DUAL;
// 	mFlagArray[INVENTORY_HAND_LEFT2] = ITEM_FLAG_SHIELD;
// 	mFlagArray[INVENTORY_HAND_RIGHT2] = ITEM_FLAG_SWORD | ITEM_FLAG_BLADE | ITEM_FLAG_CUTTER | ITEM_FLAG_GUN | ITEM_FLAG_STAFF | ITEM_FLAG_DUAL;
// 
// 	mFlagArray[INVENTORY_COSTUME_ONEPIECE] = ITEM_FLAG_ONEPIECE;
// 	mFlagArray[INVENTORY_COSTUME_EQUIP1] = ITEM_FLAG_COSTUME1;
// 	mFlagArray[INVENTORY_COSTUME_EQUIP2] = ITEM_FLAG_COSTUME2;
// 	mFlagArray[INVENTORY_COSTUME_EQUIP3] = ITEM_FLAG_COSTUME3;

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

	/// Npc 상점
    SAFE_DELETE( mpNpcStore );

	/// 아이템 총갯수
	{
		mItemCountMap.Clear();
	}

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

	mDropItemMap.Clear();
}

bool cItemManager::Init()
{
	mItemText = new cUIText;


	cString path;
	path.Format("./language/%s/Item_Name.txt", cApplication::mLangaugeFolder.Cstr() );
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

///// game in 시에 데이터 세팅
//void cItemManager::Open()
//{
//	/// 무기 장착 변경
//	SetSwitchWeapon( mActiveWeapon );
//}
//
//void cItemManager::Close()
//{
//	/// 초기화 
//	mActiveWeapon = ItemActiveMax;
//
//	/// 총 갯수 세기 초기화
//	mItemCountMap.Clear();
//	mDropItemMap.Clear();
//
//	/// 쿨타임 삭제
//	COOLMAN->Clear();
//
//	/// 퀵슬롯 삭제
//	QUICKMAN->Clear();
//
//	/// 아이템 모두 삭제
//	for( unsigned int i=MIN_INVENTORY; i<=MAX_INVENTORY; ++i )
//	{
//		mItemArray[i].Clear();
//		mItemArray[i] = 0;
//	}
//
//	mCurWeight = 0.f;
//	mMaxWeight = 0.f;
//	mLastAccutime = 0;
//
//	/// 거래 사용자 정보 클리어
//	mTradeTarget = 0;
//	mTradeMoney  = 0;
//	mTradeName.Clear();
//	mUserSellName.Clear();
//	mUserSellTarget = 0;
//
//	/// 아이템 관련 락 풀기
//	/// 초기화용
//	mLocked = false;
//	mTry = ITEM_TRY_NULL;
//
//	if( mSlotIndex0 <= MAX_INVENTORY )
//	{
//		cItem& item0 = mItemArray[mSlotIndex0];
//		item0.SetLock( false );
//	}
//
//	if( mSlotIndex1 <= MAX_INVENTORY )
//	{
//		cItem& item1 = mItemArray[mSlotIndex1];
//		item1.SetLock( false );
//	}
//
//	mSlotIndex0 = USHRT_MAX;
//	mSlotIndex1 = USHRT_MAX;
//	mValue = 0;
//}

bool cItemManager::LoadText( const cString& pathName )
{
	if( mItemText->Load( pathName ) == false )
	{
		assert( 0 && "failed to load item text" );
		return false;
	}
	return true;
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
