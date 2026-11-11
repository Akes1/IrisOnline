/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.02
* 내    용 : 아이템 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Item_Common.h"
#include "ItemScript.h"

class cItemDefine;
class cItemAbility;
class cItemLimit;
class cItemCard;
class cItemSetInfo;
class cItemTarot;
class cItemSpread;
struct sItemDisjoint;
struct sItemPutCardSlot;

typedef tPointerArray<void*>  cEnhancedOptionArr;


/// npc 아이템 상점 탭 갯수
const int NPCSTORE_SLOT_COUNT = 10;

// 수령함 슬롯 갯수
const int RECEIPT_SLOT_COUNT = 100;

enum eNotInvenSlotIndex
{
	/// 아이템 Npc 상점 슬롯
	NPCSTORE_SLOT_BEGIN = MAX_INVENTORY + 1,
	NPCSTORE_SLOT_END = NPCSTORE_SLOT_BEGIN + NPCSTORE_SLOT_COUNT - 1,

	/// 카드장착 아이템 슬롯
	PUTCARD_ITEM_SLOT,
	PUTCARD_CARD_SLOT_BEGIN,
	PUTCARD_CARD_SLOT_END = PUTCARD_CARD_SLOT_BEGIN + MAX_CARD_SLOT - 1,

	/// 아이템몰 수령함
	RECEIPT_SLOT_BEGIN,
	RECEIPT_SLOT_END = RECEIPT_SLOT_BEGIN + RECEIPT_SLOT_COUNT - 1,

	NOTINVENSLOT_MAX,
};

/// 인벤토리 장착 슬롯
const int EQUIP_SLOT_COUNT = INVENTORY_EQUIP_END - INVENTORY_WEAR_BEGIN + 1;

/// 인벤토리 쉬트 슬롯
const int INVENSHEET_SLOT_COUNT = INVENTORY_BAG0_END - INVENTORY_BAG0_BEGIN + 1;

/// 인벤토리 가방 슬롯
const int BAG_SLOT_COUNT = INVENTORY_BAG1_END - INVENTORY_BAG1_BEGIN + 1;

/// 아이템 창고 슬롯
const int WAREHOUSE_SLOT_COUNT = INVENTORY_WAREHOUSE0_END - INVENTORY_WAREHOUSE0_BEGIN + 1;

/// 아이템 슬롯
const int ITEM_SLOT_COUNT = MAX_INVENTORY + NOTINVENSLOT_MAX + 1;

/// 아이템 슬롯 소속
/// 각슬롯이 어느 창에 속하는지 알수 있음.
enum eItemIn
{
	ITEM_IN_NULL = 0,
	ITEM_IN_EQUIP,
	ITEM_IN_INVEN_PRIVATE,
	ITEM_IN_INVEN_QUEST,
	ITEM_IN_INVEN_MALL,
	ITEM_IN_WAREHOUSE_PRIVATE,
	ITEM_IN_WAREHOUSE_MALL,
	ITEM_IN_NPCSTORE,
	ITEM_IN_USERSELL,	
	ITEM_IN_PUTCARD_ITEMSLOT,
	ITEM_IN_PUTCARD_CARDSLOT,
};

// 아이템 제한 체크
enum 
{
	ITEMLIMIT_SUCCESS = 0,
	ITEMLIMIT_ERROR,

	ITEMLIMIT_RACE,			/// 종족
	ITEMLIMIT_GENDER,		/// 성별
	ITEMLIMIT_JOB,			/// 직업
	ITEMLIMIT_LEVEL,		/// 레벨
	ITEMLIMIT_PVPONLY,		/// 전장 
	ITEMLIMIT_TUTORIALONLY, /// 튜토리얼
	ITEMLIMIT_FORCETYPE,	/// 세력 오류
	ITEMLIMIT_MAKESKILL,	/// 전문직업 오류
	ITEMLIMIT_MIXSKILLSTEP,	/// 전문직업 단계 에러
	ITEMLIMIT_MIXRECIPESTEP, /// 레시피의 요구단계 에러
	ITEMLIMIT_MIXRECIPEEXP,	/// 숙련도 에러

	//
	ITEMLIMIT_HAVETITLE,	/// 이미소유한 호칭
};

struct sDealCategory
{
	char mHighKey;	/// 대분류
	short mMiddleKey;	/// 중분류

	sDealCategory()
	{
		mHighKey = 0;
		mMiddleKey = 0;
	}
};

struct sChangeCount
{
	unsigned long	itemIdx;
	int				total;

	sChangeCount()
	{
		itemIdx = 0;
		total = 0;
	}
};

// 아이템 갯수 정보
struct sItemCount
{
	int		bag;		//장착+가방
	int		warehouse;	//창고
	int		total;		//모든 합

	sItemCount()
	{
		bag = 0;
		warehouse = 0;
		total = 0;
	}
};

/// 아이템 관리자
class cItemManager
{
	static cItemManager* mSingleton;
public:

	/// 정보 ( text index, 분류 리스트 )
	typedef tPair<unsigned int, void*> cDataPair;

public:
	cItemManager();
	~cItemManager();

	static cItemManager* GetSingleton();

	/// 초기화
	bool Init();

	cItemDefine* GetItemDefine( unsigned long itemIndex );
	cItemAbility* GetItemAbility( unsigned long itemIndex );
	cItemSetInfo* GetItemSetInfo( unsigned int setIndex );
	cItemLimit* GetItemLimit( unsigned long itemIndex );
	cItemCard* GetItemCard( unsigned long itemIndex );
	cItemTarot* GetItemTarot( unsigned long itemIndex );
	cItemSpread* GetItemSpread( unsigned long itemIndex );
	sItemDisjoint* GetItemDisjoint( unsigned long index );
	cEnhancedOptionArr* GetItemEnhanced( unsigned long index );
	sItemPutCardSlot* GetItemPutCardSlot( unsigned long index );

protected:
	/// 로딩
	bool LoadDefine( const cString& pathName );
	bool LoadAbility( const cString& pathName );
	bool LoadLimit( const cString& pathName );
	bool LoadCard( const cString& pathName );
	bool LoadTarot( const cString& pathName );
	bool LoadSpread( const cString& pathName );
	bool LoadNpcStore( const cString& pathName );
	bool LoadDisjoint( const cString& pathName );
	bool LoadEnhanced( const cString& pathName );
	bool LoadCardSlot( const cString& pathName );
	bool LoadItemSet( const cString& pathName );

protected:

	/// 아이템 정의
	typedef tHashMap<unsigned long, void*> cDefineMap;
	cDefineMap mDefineMap;

	/// 아이템 능력
	typedef tHashMap<unsigned long, void*> cAbilityMap;
	cAbilityMap mAbilityMap;

	/// 아이템 제한
	typedef tHashMap<unsigned long, void*> cLimitMap;
	cLimitMap mLimitMap;

	/// 아이템 카드
	typedef tHashMap<unsigned long, void*> cCardMap;
	cCardMap mCardMap;

	/// 아이템 타롯
	typedef tHashMap<unsigned long, void*> cTarotMap;
	cTarotMap mTarotMap;

	/// 아이템 스프레드
	typedef tHashMap<unsigned long, void*> cSpreadMap;
	cSpreadMap mSpreadMap;

	/// 아이템 분해 데이터 
	typedef tHashMap<unsigned long, void*> cDisjointMap;
	cDisjointMap mDisjointMap;

	/// 아이템 강화 데이터 
	typedef tHashMap<unsigned long, void*> cEnhancedMap;
	cEnhancedMap mEnhancedMap;

	/// 카드 슬롯 데이터
	typedef tHashMap<unsigned long, void*> cPutCardSlotMap;
	cPutCardSlotMap	mPutCardSlotMap;

	/// 아이템 세트 데이터
	typedef tPointerHashMap<unsigned long, void*> cSetInfoMap;
	cSetInfoMap mSetInfoMap;
};

inline cItemManager* cItemManager::GetSingleton()
{
	return mSingleton;
}

#define ITEMMAN cItemManager::GetSingleton()
