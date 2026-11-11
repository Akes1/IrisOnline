/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.02
* 내    용 : 아이템 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Item_Common.h"
#include "Item.h"
#include "ItemScript.h"
#include "BaseStage.h"
#include "UIRect.h"
typedef tUIRect<int> cUIRect;

struct MSG_RES_NPC_ITEM_MOV2_INVENTORY;
struct MSG_RES_NPC_ITEM_BUY;
struct MSG_RES_NPC_ITEM_SELL;
struct MSG_RES_NPC_ITEM_AGENT_ADD;
struct MSG_RES_NPC_ITEM_AGENT_DEL;
struct MSG_RES_NPC_ITEM_AGENT_GET;
struct MSG_RES_NPC_OBT_EVENT;

/// 루팅창등 출력용
const int MAX_ITEMNAME_LEN = 10;
const int ITENNAME_LEN = 128;
const int MAX_NAME_WIDTH = 102;
const unsigned long OPEN_TIME = 500;

const unsigned long TRADE_WAIT_TIME = 10000;
const unsigned long DEAL_DELAYTIME = 1000;	// 판매대행, 수령함

class cParser;
class cUIText;
class cItemDefine;
class cItemAbility;
class cItemLimit;
class cItemCard;
class cGauge;
class cPlayer;
class cUIWindow;

/// drop item 그래픽 정보
class cSceneNode;

/// npc 아이템 상점 탭 갯수
const int NPCSTORE_SLOT_COUNT = 16;

/// 인벤토리 제외 슬롯들
/// 아이템 아이콘을 관리하기위한 슬롯으로, 서버와 관련없다
enum eNotInvenSlotIndex
{
	/// 아이템 Npc 상점 슬롯
	NPCSTORE_SLOT_BEGIN = MAX_INVENTORY + 1,
	NPCSTORE_SLOT_END = NPCSTORE_SLOT_BEGIN + NPCSTORE_SLOT_COUNT - 1,

	/// 카드장착 아이템 슬롯
	PUTCARD_ITEM_SLOT,
	PUTCARD_CARD_SLOT_BEGIN,
	PUTCARD_CARD_SLOT_END = PUTCARD_CARD_SLOT_BEGIN + MAX_CARD_SLOT - 1,

	NOTINVENSLOT_MAX,
};

/// 인벤토리 장착 슬롯
const int EQUIP_SLOT_COUNT = INVENTORY_EQUIP_END - INVENTORY_EQUIP_BEGIN + 1;

/// 인벤토리 쉬트 슬롯
const int INVENSHEET_SLOT_COUNT = INVENTORY_BAG0_END - INVENTORY_BAG0_BEGIN + 1;

/// 인벤토리 가방 슬롯
const int BAG_SLOT_COUNT = INVENTORY_BAG1_END - INVENTORY_BAG1_BEGIN + 1;

/// 아이템 창고 슬롯
const int WAREHOUSE_SLOT_COUNT = INVENTORY_WAREHOUSE0_END - INVENTORY_WAREHOUSE0_BEGIN + 1;

/// 아이템 슬롯
const int ITEM_SLOT_COUNT = MAX_INVENTORY + NOTINVENSLOT_MAX + 1;

// 수령함 슬롯 갯수
const int RECEIPT_SLOT_COUNT = INVENTORY_CASH_ITEM_END - INVENTORY_CASH_ITEM_BEGIN;

/// 아이템 슬롯 소속
/// 각슬롯이 어느 창에 속하는지 알수 있음.
enum eItemIn
{
	ITEM_IN_NULL = 0,
	ITEM_IN_EQUIP,
	ITEM_IN_INVEN_PRIVATE,
	ITEM_IN_INVEN_QUEST,
	ITEM_IN_WAREHOUSE_PRIVATE,
	ITEM_IN_NPCSTORE,
	ITEM_IN_USERSELL,	
	ITEM_IN_PUTCARD_ITEMSLOT,
	ITEM_IN_PUTCARD_CARDSLOT,
	ITEM_IN_RECEIPT,
	ITEM_IN_CARDBOOK_NORMAL,
	ITEM_IN_CARDBOOK_NORMAL_SPREAD,
};

/// 아이템 시도
enum eItemTry
{
	ITEM_TRY_NULL = 0,
	ITEM_TRY_MOVE,
    ITEM_TRY_USE,
	ITEM_TRY_EQUIP,
	ITEM_TRY_UNEQUIP,
	ITEM_TRY_DUMP,
	ITEM_TRY_SPLIT,
	ITEM_TRY_MERGE,
	ITEM_TRY_SWITCH,
	ITEM_TRY_BUY,
	ITEM_TRY_SELL,
	ITEM_TRY_GET,				/// 상자로부터 아이템 획득
	ITEM_TRY_DISJOINT,
	ITEM_TRY_ENHANCEDSTART,
	ITEM_TRY_ENHANCEDCANCEL,
	ITEM_TRY_TRADEADD,
	ITEM_TRY_TRADEDELETE,
	ITEM_TRY_SELLITEMADD,
	ITEM_TRY_SELLITEMDELETE,
	ITEM_TRY_SELLGET,			/// 노점아이템 구입
	ITEM_TRY_QUEST_REWARDTAKE,	/// 아이템 보상지급 & 반납 처리
	ITEM_TRY_QUEST_ADDBYITEM,	/// 아이템 보상지급 & 의뢰아이템 삭제 처리
	ITEM_TRY_QUEST_TAKE,		/// 아이템 반납
	ITEM_TRY_PUTCARD,			/// 카드 장착
	ITEM_TRY_CARDBOOKADD,		/// 컬렉션북 등록
	ITEM_TRY_PICK,				/// 스페이스바로 아이템 줍기
	ITEM_TRY_CHANGE,			/// 체인지 아이템
	ITEM_TRY_MIXSKILL,			/// 조합기술 아이템
	ITEM_TRY_SKILLRESET,		/// 스킬 초기화
	ITEM_TRY_ITEMMIXDBWORK,		/// 조합 DB 응답 대기중
	ITEM_TRY_ENHANCEDBWORK,		/// 강화 DB 응답 대기중
	ITEM_TRY_NPCDEALREGIST,		/// 판매대행 등록
	ITEM_TRY_NPCDEALDELETE,		/// 판매대행 삭제
	ITEM_TRY_NPCDEALBUY,		/// 판매대행 구입
	ITEM_TRY_MAILDOWNLOAD,		/// 메일 - 아이템 다운로드
	ITEM_TRY_MAILSEND,			/// 메일 - 아이템 보내기
	ITEM_TRY_MAPCHANGE,
	ITEM_TRY_VEHICLE,
	ITEM_TRY_SEAL,
	ITEM_TRY_LISTRECEIPT,
	ITEM_TRY_THEMERESET,
	ITEM_TRY_GIVEPARTY,
	ITEM_TRY_SUMMON,			// 편의 기능 소환 주문서

	ITEM_TRY_NPCEVENT,			// 이벤트 보상
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
	ITEMLIMIT_FIELDONLY,		/// 필드맵 
	ITEMLIMIT_FORCETYPE,	/// 세력 오류
	ITEMLIMIT_MAKESKILL,	/// 전문직업 오류
	ITEMLIMIT_MAKESKILLEXP, /// 전문직업 숙련도 오류
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
	int		etc;		//etc ( 그 외 )
	int		total;		//모든 합
	
	sItemCount()
	{
		bag = 0;
		etc = 0;
		total = 0;
	}
};

struct sItemBillEx : public sItemBill
{
	time_t begin;
};

/// 아이템 관리자
class cItemManager
{
	friend class cCheatManager;

public:
	static cItemManager* mSingleton;

	/// 정보 ( text index, 분류 리스트 )
	typedef tPair<unsigned int, void*> cDataPair;

public:
	cItemManager();
	~cItemManager();

	/// 초기화
	bool Init();
	void Open();

	/// 데이터 삭제
	void Close();

	/// 아이콘
	void BindIcon( unsigned short slotIndex, cItemIcon* icon );
	void LinkIcon( cItemIcon* icon );
	void UnLinkIcon( bool setcolor = false );
	void CheckItemSlot( unsigned short slotIndex );

	/// 쿨타임 세팅
	void StartCoolTime( unsigned long itemIndex, unsigned long coolTime );
	void EndCoolTime( unsigned long itemIndex );
	void CopyCoolTime( unsigned long itemIndex, unsigned long startTime, unsigned long restTime, unsigned long totalTime );

	/// 게임접속시, 전체 쿨타임 정보 복사 세팅 
	void SetAllItemCool();

	/// 무게
	void SetWeightGauge( cGauge* gauge );
	void SetMaxWeightGauge( float max );

	/// 아이템 변화시 정보 체크 ( 무게, 갯수, 사용여부등 )
	void UpdateItem();
	float GetWeight();

	/// 아이템 어빌리티 체크
	void CheckEquipAbility();

	sEquipAbility* GetBasicAbil();

	/// hero 장착 무기 리턴
	cItem* GetHeroRightWeapon();
	cItem* GetHeroLeftWeapon();

	/// 무기를 장착하고 있는지 여부
	bool IsEquipWeapon();
	void SetActiveWeapon( int active ) { mActiveWeapon = (eItemActiveWeapon)active; }
	int GetActiveWeapon() { return (int)mActiveWeapon; }

	/// 인벤토리내에 선택된 아이콘을 한개만 세팅하도록 함
	void SetInvenSelect( cUIWindow* win );

	/// 아이템 툴팁 컬러 리턴 
	void GetTooltipColor( unsigned short tiptype, unsigned long& color );

	/// 가격 단위 툴팁 획득
	unsigned long GetPriceUnit( unsigned long type, unsigned long itemIndex );
	unsigned long GetPriceUnit( unsigned long itemIndex );

	/// 아이템 사용제한 조건 체크
	void CheckItemLimitAll();

	/// 아이템 limit 검사
	int CheckLimit( unsigned long itemIndex );
	int CheckLimitEtc( unsigned long itemIndex );
	bool CheckLimitJob( cItemLimit* itemLimit );

	/// 아이템
	bool TryToUnSeal( unsigned short slotIndex );
	bool TryToMoveItem( unsigned short slotIndex0, unsigned short slotIndex1, cItemIcon* icon );
	bool TryToEquipItem( unsigned short slotIndex );
	bool TryToUnEquipItem( unsigned short slotIndex );
	bool TryToDumpItem( unsigned short slotIndex );
	bool TryToSplitItem( unsigned short slotIndex0, unsigned short slotIndex1, unsigned int value );
	bool TryToUseSkillReset( unsigned short slotIndex );
	bool TryToUseExpRecover( unsigned short slotIndex );
	bool TryToUseCashItem( unsigned short slotIndex );
	bool TryToUseInventory( unsigned short slotIndex ); // 확장 주문서류
	
	/// 무기
	bool TryToSwitchWeapon();

	/// 슬롯에 총 들어갈수 있는 갯수 구하기
	unsigned int GetRemainCount( unsigned long itemIndex, unsigned int mincount );

	bool CheckDisjoint( unsigned short slotIndex );
	bool CheckBuyItem( unsigned long itemIndex, unsigned int count );

	/// 상점
	bool TryToBuyItem( unsigned long itemIndex, unsigned int count );
	bool TryToSellItem( unsigned short slotIndex, unsigned int count );

	/// 아이템 줍기
	bool TryToGetItem( unsigned long itemIndex, unsigned int count );

	/// 아이템 분해, 강화 (아이템,카드)
	bool TryToDisjoint( unsigned short slotIndex );

	bool TryToEnhancedStart( unsigned short slotIndex0, unsigned short slotIndex1, unsigned short slotIndex2 = 0 );
	bool TryToEnhancedCancel( bool close = true );
	//bool TryToEnhancedResult( unsigned short slotIndex0, unsigned short slotIndex1 );

	/// 080402 PKH Npc
	void NpcNetTryItem( unsigned int protocol, void* msg = 0 );

	/// 거래관련
	bool TryToTradeAsk( cPlayer* player );	/// 거래요청
	void TryToTradeAccept();								/// 거래 승인
	void TryToTradeReject( BYTE type );						/// 거래 거절
	void TryToTradeCancel();								/// 닫기 요청
	bool TryToTradeItemAdd( unsigned short slotIndex );		/// 아이템 등록
	bool TryToTradeMoneyAdd( unsigned long money );			/// 거래금액 등록
	bool TryToTradeItemDelete( unsigned short slotIndex, unsigned int arrIndex );	/// 아이템 삭제
	void TryToTradeOk();									/// 거래물품 확인 완료
	void TryToTradeReTry();									/// 수정 요청
	void TryToTradeEnd();									/// 거래 종료 

	/// 노점상관련
	unsigned long GetUserSellTarget() { return mUserSellTarget; }
	void ClearUserSellTarget();
	bool TryToUserSellOpen();
	bool TryToUserSellReName( LPCTSTR title );
	void TryToUserSellClose();
	bool TryToUserSellAdd( unsigned short slot, unsigned long price );
	bool TryToUserSellDelete( unsigned short slot );
	bool TryToUserSellModify( unsigned short slot );
	void TryToUserSellJoin( sObject& obj );
	void TryToUserSellJoinClose();										/// 노점상 참여 닫기
	bool TryToUserSellGet( unsigned short slot, unsigned int count );	/// 아이템 구입요청

	/// 카드장착 관련 ( 장착아이템, 장착할카드, 장착윈도우의 슬롯 )
	bool TryToPutCard( unsigned short itemSlot, unsigned short cardSlot, unsigned short putSlot );

	/// 컬렉션북 카드 장착 관련
	bool TryToCardBookAdd( unsigned short slot0, unsigned short slot1 );

	/// 판매대행 관련
	bool TryToNpcDealRegistReq( unsigned short slotIndex, unsigned long price, char validThru );
	bool TryToNpcDealOpen( unsigned long npcIndex );
	bool TryToNpcDealOpenPage( long page );
	bool TryToNpcDealRegistDelete( long idx );
	bool TryToNpcDealBuy( long idx, long page, short count, LPCTSTR itemName, char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData );

	/// 판매대행 - 검색
	bool TryToNpcDealSearchAll( long page, LPCTSTR itemName, char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData );
	bool TryToNpcDealSearchType( long page, char type, short subType, char lowLv, char highLv, char enhance, char rare, BYTE sortData );

	/// 아이템수령함
	bool TryToReceiptList();

	// 테마 귀속 초기화
	bool TryToThemeReset( unsigned short slot, unsigned short themeNum, unsigned char themeMode );

	// 파티원 분배 요청
	bool TryToGiveParty( unsigned long playerIndex, unsigned long itemIndex );

	/// mTry의 락이 move 에서 merge로 바꾸기.
	void LockChangeToMerge();

	/// 우편 관련
	void DeleteInventory( unsigned short slotIndex );

	///
	cItem* GetItem( unsigned short slotIndex );
	cItemDefine* GetItemDefine( unsigned long itemIndex );
	cItemAbility* GetItemAbility( unsigned long itemIndex );
	eItemIn GetItemIn( unsigned short slotIndex );
	sItemCount* GetItemCount( unsigned long itemIndex );
	unsigned short FindSlotIndex( unsigned long itemIndex );
	unsigned short FindSlotIndexByType( ITEM_TYPE mainType, ITEM_TYPE subType );
	LPCTSTR GetItemName( unsigned long textIndex );
	sItemOnePieceInfo* GetItemOnePieceInfo( unsigned long itemIndex );
	const char* GetItemFileName( unsigned long itemFileIdx, char race, char gender, bool weapon );

	bool IsEquipedItem( unsigned long itemIndex );

	cItemSetInfo* GetItemSetInfo( unsigned int setIndex );

	// 드랍 아이템 그래픽 인덱스
	unsigned long GetDropNodeIndex( unsigned long dropIndex );

	/// 잠금
	void LockItemTry( eItemTry itemTry, unsigned short slotIndex0 = USHRT_MAX );	/// 외부 사용잠금

	/// 잠금 풀기 
	eItemTry GetItemTry() { return mTry; }
	void UnlockItemTry( eItemTry itemTry );
	bool IsLocked();

	/// 네트워크 메시지
	void NetworkMsgParser( char* msg );
	void NetworkMsgParserTrade( char* msg );

	void NetSetItem( sInventory* inv, bool setCool = true );
	bool NetSetItemBill( sItemBill* bill );
	void DeleteItemBill( unsigned long idx );

	//
	void ClearCashInven();
	void NetSetCashItem( sCashInventory* inv );

	/// DropItem 인터페이스..
	bool CreateDropItem( MSG_ITEM_INFO* dropMsg );
	void DeleteDropItem( unsigned long idx );

	// 드랍아이템 의 튜토리얼 이미지 갱신
	void UpdateTutorialDrop();
	void ClearTutorialDrop();

	bool CheckNearestDropItem();
	bool CheckOpenTime();

	/// 무기 스위칭 좌표 세팅
	void SetWeaponIconPos( unsigned short slotIndex, cUIRect rect );

	//
	bool FindEmptySlotArr( unsigned short* slotIndexArr, unsigned int count, unsigned short begin, unsigned short end );
	bool FindEmptySlot( unsigned short* slotIndex, unsigned short begin, unsigned short end );

	/// 아이템 놓는 사운드 재생
	void PlayIconSettingSound( unsigned short slotIndex );

	///
	tArray<unsigned char>* GetDealHighArr() { return &mDealHighArr; }
	tArray<void*>*		   GetDealCategoryMap( char key ) { return (tArray<void*>*)mDealCategoryMap.GetAt( key ); }

	/// 아이템 정의맵 획득
	tHashMap<unsigned long, void*>& GetDefineMap() { return mDefineMap; }
	
protected:
	/// 로딩
	bool LoadText( const cStringW& pathName );
	bool LoadDefine( const cString& pathName );
	bool LoadAbility( const cString& pathName );
	bool LoadLimit( const cString& pathName );
	bool LoadCard( const cString& pathName );
	bool LoadTarot( const cString& pathName );
	bool LoadSpread( const cString& pathName );
	bool LoadFileName( const cString& pathName );
	bool LoadNpcStore( const cString& pathName );
	bool LoadDisjoint( const cString& pathName );
	bool LoadEnhanced( const cString& pathName );
	bool LoadCardSlot( const cString& pathName );
	bool LoadDealCategoryTable( const cString& pathName );
	bool LoadItemSet( const cString& pathName );
	bool LoadItemOnePieceInfo( const cString& pathName );

	/// 쿨타임 관련 데이터 로드
	void LoadCoolData();
	
	///
	void LockAndBackup( eItemTry itemTry, unsigned short slotIndex0, unsigned short slotIndex1 = USHRT_MAX, unsigned long tempIndex = 0, unsigned int value = 0, unsigned long price = 0, cItemIcon* icon = 0 );

	/// 장착
	bool TestMoveItemDown( unsigned short slotIndex0, unsigned short slotIndex1 );
	bool TestMoveItemUp( unsigned short slotIndex0, unsigned short slotIndex1 );

	bool MoveItemDown( unsigned short slotIndex0, unsigned short slotIndex1 );
	bool MoveItemUp( unsigned short slotIndex0, unsigned short slotIndex1 );

	bool TestEquipWeapon( unsigned short& slotIndex1, eItemEquip equipType );
	bool TestEquipWeapon_OneHand( unsigned short& slotIndex1 );
	bool TestEquipWeapon_TwoHands( unsigned short& slotIndex1 );
	bool TestEquipWeapon_Shield( unsigned short& slotIndex1 );
	bool TestEquipWear( unsigned short& slotIndex1, unsigned char subType );
	bool TestEquipAccessory( unsigned short& slotIndex1, unsigned char subType );
	bool TestEquipCard( unsigned short& slotIndex1 );

	bool TestUseItem( unsigned short slotIndex0 );

	bool EquipWeapon( unsigned short i0, unsigned short i1, eItemEquip equip0, eItemEquip equip1 );
	bool EquipWear( unsigned short i0, unsigned short i1, unsigned char subType );
	bool EquipAccessory( unsigned short i0, unsigned short i1, unsigned char subType );
	bool EquipCard( unsigned short i0, unsigned short i1 );

	/// 네트워크
	void NetTryItem( unsigned int protocol, void* msg = 0 );
	void NetMoveItem( unsigned short slotIndex0, unsigned short slotIndex1 );
	void NetUseItem( unsigned short slotIndex );
	void NetDumpItem( unsigned short slotIndex, bool onSound = false );
	void NetSplitItem( MSG_RES_ITEM_DIVIDE_INVENTORY* msgData );
	void NetSplitItem( MSG_RES_NPC_ITEM_MOV2_INVENTORY* msgData, unsigned short slotIndex0, unsigned short slotIndex1 );

	void SetSwitchWeapon( eItemActiveWeapon activeWeapon, bool onSound = false );
	void NetSwitchWeapon( eItemActiveWeapon activeWeapon );
	void NetBuyItem( MSG_RES_NPC_ITEM_BUY* msgData );
	void NetGetItem( MSG_RES_ITEM_GET* msgData );
	void NetSellItem( MSG_RES_NPC_ITEM_SELL* msgData );
	void NetRewardItem( MSG_RES_ITEM_REWARD* msgData );
	void NetTakeItem( MSG_RES_ITEM_TAKE* msgData );
	void NetDisjointItem( MSG_RES_ITEM_DISJOINT_INVENTORY* msgData );
	void NetEnhancedItem( MSG_SYN_ITEM_ENHANCED_END* msgData );
	void NetTradeItem( MSG_RES_ITEM_EXCHANGE_END* msgData );
	void NetChangeItem( MSG_RES_ITEM_CHANGE_INVENTORY* msgData );
	void NetMixSkillItem( MSG_RES_MAKESKILL_ITEM* msgData );
	void NetMixItem( MSG_RES_ITEM_MIXED* msgData );
	void NetSkillResetItem( MSG_RES_ITEM_SKILLRESET* msgData );
	void NetItemAgentAddItem( MSG_RES_NPC_ITEM_AGENT_ADD* msgData );
	void NetItemAgentDelItem( MSG_RES_NPC_ITEM_AGENT_DEL* msgData );
	void NetItemAgentGetItem( MSG_RES_NPC_ITEM_AGENT_GET* msgData );
	void NetGivePartyItem( MSG_SYN_ITEM_PARTY_GIVE* msgData );
	
	///
	cItemLimit* GetItemLimit( unsigned long itemIndex );
	cString* GetItemFileNames( unsigned long itemIndex );
	cItemSpread* GetItemSpread( unsigned long itemIndex );
	cItemCard* GetItemCard( unsigned long itemIndex );
	cItemTarot* GetItemTarot( unsigned long itemIndex );
	sItemDisjoint* GetItemDisjoint( unsigned long index );
	cEnhancedOptionArr* GetItemEnhanced( unsigned long index );
	sItemPutCardSlot* GetItemPutCardSlot( unsigned long index );
	
	/// 아이템 어빌리티 체크
	void AddAbility( cItem& item );
	void AddSpecialAbility( short type, short value );

	void AddEnhance( cItem& item );
	void AddPutCard( unsigned long cardIndex );

	/// 아이템 전체 무게 검사 && 아이템 총갯수 관리
	void CheckWeight();
	void CheckItemCount();

	/// 선택된 슬롯 갱신
	void ChangeSelectSlot( unsigned short slot );
	
	/// 치트
	bool CheatMakeItem( sInventory* inv );

	//
	void ClearItemCount();

public:
	/// 단일체를 리턴
	static cItemManager* GetSingleton();

protected:
	/// 텍스트
	cUIText* mItemText;

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

	/// 아이템 파일명 배열
	typedef tHashMap<unsigned long, void*> cFileNamesMap;
	cFileNamesMap mFileNamesMap;

	/// 아이템 분해 데이터 
	typedef tHashMap<unsigned long, void*> cDisjointMap;
	cDisjointMap mDisjointMap;

	/// 아이템 강화 데이터 
	typedef tPointerArray<void*>  cEnhancedOptionArr;
	typedef tHashMap<unsigned long, void*> cEnhancedMap;
	cEnhancedMap mEnhancedMap;

	/// 카드 슬롯 데이터
	typedef tHashMap<unsigned long, void*> cPutCardSlotMap;
	cPutCardSlotMap	mPutCardSlotMap;

	/// 아이템 세트 데이터
	typedef tPointerHashMap<unsigned long, void*> cSetInfoMap;
	cSetInfoMap mSetInfoMap;

	/// 아이템 분류를 위한 자료구조
	/// 아이템 정의 인덱스
	typedef tArray<unsigned char> cNameIdxArr;
	typedef tPointerHashMap<unsigned char, void*> cNameIdxArrMap;

	/// sDealCategory
	typedef tArray<void*> cDealArr;
	cNameIdxArr		mDealHighArr;		// 대분류
	cNameIdxArrMap	mDealCategoryMap;	// highkey, *cDealArr

	typedef tPointerHashMap<unsigned long, void*> cItemOnpieceInfo;
	cItemOnpieceInfo mItemOnePieceInfo;

protected:

	/// 아이템 배열
	cItem mItemArray[LEN_INVENTORY];

	/// 아이템 플랙 배열
	int mFlagArray[LEN_INVENTORY];

	/// 잠김 여부
	bool mLocked;

	/// 활성화된 장착 정보
	eItemActiveWeapon mActiveWeapon;

	/// front / back 상대좌표
	cUIRect mFrontLeft;
	cUIRect mFrontRight;
	cUIRect mBackLeft;
	cUIRect mBackRight;

	/// 백업
	eItemTry mTry;
	unsigned short mSlotIndex0;
	unsigned short mSlotIndex1;
	unsigned long mTempIndex;	/// 각종 상황별로 저장할 인덱스
	unsigned int mValue;
	unsigned long mPrice;
	cItemIcon* mLinkedIcon;
	NiColorA mLinkColor;

	/// 무게
	cGauge* mWeightGauge;
	float mMaxWeight;
	float mCurWeight;

	/// 장비 어빌리티 ( 기본, 추가 )
	sEquipAbility mBasicAbil;

	/// Npc 상점
    cItemScript*    mpNpcStore;

	// 아이템 갯수
	typedef tPointerHashMap<unsigned long, void*> cItemCountMap;
	cItemCountMap mItemCountMap;

	/// Drop Item의 정보를 좀더 쉽게 참조하기 위한 자료구조
	/// server에서 제공된 index / 그래픽 Node 포인터
	typedef tPointerHashMap<unsigned long, unsigned long> cDropItemMap;
	cDropItemMap mDropItemMap;

	// 캐쉬아이템 시간정보 (idx, sItemBillEx)
	typedef tPointerHashMap<unsigned long, void*> cItemBillMap;
	cItemBillMap mItemBillMap;

	/// 거래할 상대방 정보
	unsigned long mTradeTarget;
	cStringT mTradeName;
	unsigned long mTradeMoney;

	/// 노점관련 정보
	cStringT mUserSellName;	
	unsigned long mUserSellTarget;	/// 보고있는 노점의 타겟인덱스

	/// 아이템 상자 오픈시간 체크
	unsigned long mLastAccutime;
};

inline void cItemManager::SetWeightGauge( cGauge* gauge )
{
	mWeightGauge = gauge;
}

inline float cItemManager::GetWeight()
{
	return mCurWeight;
}

inline cItem* cItemManager::GetItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
	{
		assert( 0 && "index out of range" );
		return 0;
	}

	return &mItemArray[ slotIndex ];
}

inline sEquipAbility*	cItemManager::GetBasicAbil()
{
	return &mBasicAbil;
}

inline void cItemManager::ClearUserSellTarget()
{
	mUserSellTarget = 0;
}

inline cItemManager* cItemManager::GetSingleton()
{
	return mSingleton;
}

#define ITEMMAN cItemManager::GetSingleton()
