/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.02
* 내    용 : 아이템
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Item_Common.h"
#include "Player_Common.h"

class cIconParam;

/// 아이템 제한
const unsigned char ITEMLIMIT_MINLEVEL = 1;
const unsigned char ITEMLIMIT_MAXLEVEL = 100;

/// 아이템 분해 데이터
const int MAX_DISJOINT = 6;
struct sDisjointData
{
	unsigned long itemIndex;
	unsigned int count;
};

/// 아이템 분해
struct sItemDisjoint
{
	/// 비용
	unsigned long mPrice;

	/// 분해 데이터
	sDisjointData mDisjoint[MAX_DISJOINT];
};

enum eEnhancedType
{
	eENHANCE_PHYSICATTACK = 1,		/// 물공
	eENHANCE_MAGICATTACK,			/// 마공
	eENHANCE_PHYSICDEFENSE,			/// 물방
	eENHANCE_MAGICDEFENSE,			/// 마방
	eENHANCE_HEAL,					/// 힐 회복량
};

/// 사용할 수 있는 맵 제한
enum eUseMapType
{
	eUSE_MAP_ALL = 0,		// 모든 맵
	eUSE_MAP_PVP,       // PvP전용
	eUSE_MAP_TUTORIAL,  // 튜토리얼 전용
	eUSE_MAP_FIELD,     // 필드맵 전용( 튜토리얼, PvP, 테마던젼에서 사용불가 )
};

// 아이템 사용기한
enum eLicenseClass
{
	eLICENSE_NONE = 0,		// 일반
	eLICENSE_REALTIME,	// 실제시간
	eLICENSE_GAMETIME,	// 게임시간
	eLICENSE_NOLIMIT,	// 무제한
};

/// 아이템 강화
struct sItemEnhanced
{
	/// 강화되는 속성
	unsigned char mEnhancedType;
	
	/// 증가값
	short mValue[MAX_ITEM_ENHANCED];
};

typedef tPointerArray<void*> cEnhancedOptionArr;

/// 어빌리티 효과 정보
struct sAblityOption
{
	short mOptType;		/// 옵션 인덱스겸 타입
	short mOptValue;	/// 옵션 값

	sAblityOption()
	{
		mOptType = 0;
		mOptValue = 0;
	}
};

struct sSetAblityOption
{
	short mOptType[2];
	short mOptValue[2];

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

/// 카드 장착
struct sItemPutCardSlot
{
	short mSlotType[MAX_CARD_SLOT];
};

/// 아이템 장착 타입
enum eItemEquip
{
	ITEM_EQUIP_NORMAL = 0,
	ITEM_EQUIP_ONE_HAND,
	ITEM_EQUIP_TWO_HANDS,
	ITEM_EQUIP_SHIELD
};

/// 아이템 플랙
/// 아이템과 슬롯이 동일한 타입인지 검사 ( 아닌곳에 장착되지 못함 )
enum eItemFlag
{
	ITEM_FLAG_NULL		= 0,
	ITEM_FLAG_WEAPON	= (1 <<  0),
	ITEM_FLAG_WEAR		= (1 <<  1),
	ITEM_FLAG_ACCESSORY	= (1 <<  2),

	ITEM_FLAG_SWORD		= (1 <<  3) | ITEM_FLAG_WEAPON,		/// 한손검
	ITEM_FLAG_BLADE		= (1 <<  4) | ITEM_FLAG_WEAPON,		/// 양손검
	ITEM_FLAG_DUAL		= (1 <<  5) | ITEM_FLAG_WEAPON,		/// 이도류
	ITEM_FLAG_CUTTER	= (1 <<  6) | ITEM_FLAG_WEAPON,		/// 단검
	ITEM_FLAG_GUN		= (1 <<  7) | ITEM_FLAG_WEAPON,		/// 총
	ITEM_FLAG_STAFF		= (1 <<  8) | ITEM_FLAG_WEAPON,		/// 지팡이
	ITEM_FLAG_SHIELD	= (1 <<  9) | ITEM_FLAG_WEAPON,		/// 방패

	ITEM_FLAG_HEAD		    = (1 << 10) | ITEM_FLAG_WEAR,	/// 머리
	ITEM_FLAG_UPPER		    = (1 << 11) | ITEM_FLAG_WEAR,	/// 상의
	ITEM_FLAG_LOWER		    = (1 << 12) | ITEM_FLAG_WEAR,	/// 하의
	ITEM_FLAG_HANDS		    = (1 << 13) | ITEM_FLAG_WEAR,	/// 손
	ITEM_FLAG_FEET		    = (1 << 14) | ITEM_FLAG_WEAR,	/// 발
	ITEM_FLAG_ONEPIECE	    = (1 << 15) | ITEM_FLAG_WEAR,	/// 원피스

	ITEM_FLAG_COSTUME_HEAD  = (1 << 16) | ITEM_FLAG_WEAR,	/// 코스튬 머리
	ITEM_FLAG_COSTUME_UPPER = (1 << 17) | ITEM_FLAG_WEAR,	/// 코스튬 상의
	ITEM_FLAG_COSTUME_LOWER = (1 << 18) | ITEM_FLAG_WEAR,	/// 코스튬 하의
	ITEM_FLAG_COSTUME_HANDS = (1 << 19) | ITEM_FLAG_WEAR,	/// 코스튬 손
	ITEM_FLAG_COSTUME_FEET  = (1 << 20) | ITEM_FLAG_WEAR,	/// 코스튬 발

	ITEM_FLAG_EARRING	= (1 << 21) | ITEM_FLAG_ACCESSORY,	/// 귀걸이
	ITEM_FLAG_NECKLACE	= (1 << 22) | ITEM_FLAG_ACCESSORY,	/// 목걸이
	ITEM_FLAG_BROOCH	= (1 << 23) | ITEM_FLAG_ACCESSORY,	/// 브로치
	ITEM_FLAG_BRACELET	= (1 << 24) | ITEM_FLAG_ACCESSORY,	/// 팔찌
	ITEM_FLAG_RING		= (1 << 25) | ITEM_FLAG_ACCESSORY,	/// 반지

	ITEM_FLAG_COSTUME_FACE = (1 << 26) | ITEM_FLAG_ACCESSORY,	/// 코스튬 얼굴
	ITEM_FLAG_COSTUME_BACK = (1 << 27) | ITEM_FLAG_ACCESSORY,	/// 코스튬 등
	
	ITEM_FLAG_EQUIPCARD	= (1 << 28),						    /// 장착용 카드
	ITEM_FLAG_UNKNOWN	= (1 << 29),

	ITEM_FLAG_ALL		= 0x7FFFFFFF
};

/// 일반/미인증 아이템 거래 가능
enum eItemExchange
{
	eNORMAL_ITEM_EXCHAGNE_YES,
	eNORMAL_ITEM_EXCHAGNE_NO,
	eSEALED_ITEM_EXCHAGNE_YES,
	eSEALED_ITEM_EXCHAGNE_NO,
};

/// 세트 아이템 정보
class cItemSetInfo
{
	friend class cItemManager;
public:
	~cItemSetInfo()
	{
		tPointerHashMap<unsigned int, void*>::cIterator i = mSetOption.Begin();
		tPointerHashMap<unsigned int, void*>::cIterator end = mSetOption.End();
		for( ; i != end; ++i )
		{
			delete (sSetAblityOption*)(i->mSecond);
		}
		mSetOption.Clear();

		mItemList.Clear();
	}

	void AddItem( unsigned long itemIdx ) { mItemList.PushBack( itemIdx ); }
	void AddOption( unsigned int cnt, sSetAblityOption* opt ) { mSetOption.Insert( cnt, opt ); }

	tList<unsigned long>* GetItemList() { return &mItemList; }
	unsigned int GetItemCount() { return mItemList.GetSize(); }
	unsigned int GetNameIndex() { return mSetNameIdx; }

	sSetAblityOption* GetItemOption( unsigned int cnt ) { return (sSetAblityOption*)mSetOption.GetAt(cnt); }

private:
	unsigned int mSetNameIdx;

	/// 갯수별 추가 효과( key:갯수 )
	tPointerHashMap<unsigned int, void*> mSetOption;

	/// 포함 아이템 리스트
	tList<unsigned long> mItemList;
};

/// 아이템 능력
class cItemAbility
{
	friend class cItemManager;

public:
	inline long  GetDescription() { return mDescription; }
	inline short GetTargetType() { return mTargetType; }
	inline short GetDefenseType() { return mDefenseType; }
	inline short GetUseRange() { return mUseRange; }
	inline short GetPhysicMaxAttack() { return mPhysicMaxAttack; }
	inline short GetPhysicMinAttack() { return mPhysicMinAttack; }
	inline short GetMagicMaxAttack() { return mMagicMaxAttack; }
	inline short GetMagicMinAttack() { return mMagicMinAttack; }
	inline short GetPhysicDefense() { return mPhysicDefense; }
	inline short GetMagicDefense() { return mMagicDefense; }
	inline short GetHealValue() { return mHealValue; }
	inline short GetAttackRange() { return mAttackRange; }
	inline short GetAttackSpeed() { return mAttackSpeed; }
	inline long  GetCoolTime() { return mCoolTime; }
	inline unsigned int GetGroupNum() { return mGroupNum; }

	inline sAblityOption* GetOption() { return mOption; }
	inline long GetInfluenceIndex() { return mInfluenceIndex; }
	inline long GetActiveInfluenceIndex() { return mActiveInfluenceIndex; }

private:
	/// 사용효과 설명 
	long mDescription; 

	/// 방어구재질
	short mDefenseType;

	/// 범위방식
	short mRangeType;

	/// 효과적용타겟
	short mTargetType;

	/// 사용거리
	short mUseRange;

	/// 물리 방어력
	short mPhysicDefense;
	/// 마법 방어력
	short mMagicDefense;

	/// 사정거리
	short mAttackRange;

	/// 공격 속도
	short mAttackSpeed;

	/// 재사용시간
	long mCoolTime;

	/// 그룹쿨타임 번호
	unsigned int mGroupNum;

	/// 물리 공격력(소)
	short mPhysicMinAttack;
	/// 물리 공격력(대)
	short mPhysicMaxAttack;
	/// 마법 공격력(소)
	short mMagicMinAttack;
	/// 마법 공격력(대)
	short mMagicMaxAttack;

	/// 힐 회복량
	short mHealValue;

	/// 옵션 값
	sAblityOption mOption[5];

	/// 소모효과
	long mInfluenceIndex;

	/// 발동 효과
	long mActiveInfluenceIndex;
};

/// 아이템 제한
class cItemLimit
{
	friend class cItemManager;

public:
	cItemLimit();

	inline char GetRace() { return mCharRace; }
	inline char GetGender() { return mCharGender; }
	inline long GetJob1() { return mCharJob1; }
	inline long GetJob2() { return mCharJob2; }
	inline unsigned char GetLevel() { return mCharLevel; }
	inline unsigned char GetMaxLevel() { return mCharMaxLevel; }
	inline eUseMapType GetUseMapType() { return mUseMapType; }
	inline char GetForceType() { return mForceType; }
	inline long GetMakeSkill() { return mMakeSkill; }
	inline unsigned long GetMakeSkillExp() { return mMakeSkillExp; }

private:
	/// 캐릭터:종족
	char mCharRace;

	/// 캐릭터:성별
	char mCharGender;

	/// 직업1
	long mCharJob1;

	/// 직업2
	long mCharJob2;

	/// 캐릭터:레벨
	unsigned char mCharLevel;

	/// 캐릭터:최대레벨
	unsigned char mCharMaxLevel;

	/// 사용할 수 있는 맵 종류( 0 : 모든 맵, 1 : 전장전용, 2 : 튜토리얼전용 3 : 일반맵전용 )
	eUseMapType mUseMapType;

	/// 세력 ( 0 : 전부, 1: 헬렌, 2:네로, 3:아네스, 4:데라 )
	char mForceType;

	/// 보조직업
	long mMakeSkill;

	/// 보조직업 숙련도
	unsigned long mMakeSkillExp;
};

inline cItemLimit::cItemLimit()
: mCharRace( 0 )
, mCharGender( 0 )
, mCharLevel( 0 )
, mCharMaxLevel( 0 )
, mCharJob1( 0 )
, mCharJob2( 0 )
, mUseMapType( eUSE_MAP_ALL )
, mForceType( 0 )
, mMakeSkill( 0 )
, mMakeSkillExp( 0 )
{
}

/// 스프레드 데이터
class cSpreadParam
{
public:
	/// 이미지 인덱스
	int mImageIndex;

	/// 좌표
	unsigned int mX;

	/// 좌표
	unsigned int mY;

	/// 방향
	int mDir;

	/// 계산식 ( 0 : 노말, 1 : *, 2 : /, 3 : + )
	int mCalc;

	/// 계산수치
	int mValue;
};

/// 아이템 스프레드
class cItemSpread
{
	friend class cItemManager;

public:
	cItemSpread();

public:
	/// 스프레드 데이터 배열
	cSpreadParam mParam[10];
};

inline cItemSpread::cItemSpread()
{	
	memset( mParam, 0, sizeof(mParam) );
}

/// 아이템 카드
class cItemCard
{
	friend class cItemManager;

public:
	cItemCard();

private:
	/// 판
	char mEpisode;

	/// 카드넘버
	unsigned int mCardNumber;

	/// 희귀도
	char mRareLevel;

	/// 카드분류
	char mCardType;

	/// 장착위치
	char mWearPos;

	/// 이미지인덱스
	unsigned int mImageIndex;
};

inline
cItemCard::cItemCard()
: mEpisode(0)
, mCardNumber(0)
, mRareLevel(0)
, mCardType(0)
, mWearPos(0)
, mImageIndex(0)
{

}

/// 아이템 타로카드
class cItemTarot
{
	friend class cItemManager;

public:
	cItemTarot();

	///
	inline unsigned int GetImageIndex() { return mImageIndex; }
	inline unsigned char GetUniqueNumber() { return mNumber; }

	///
	inline short GetStraightValue() { return mStraightValue; }
	inline short GetInverseValue() { return mInverseValue; }

	/// 해석번호 
	inline unsigned long GetStraightDesc() { return mStraightIdx; }
	inline unsigned long GetInverseDesc() { return mInverseIdx; }

private:
	/// 카드 이미지 인덱스
	unsigned int mImageIndex;

	/// 고유 번호
	char mNumber;

	/// 카드레벨
	char mLevel;

	/// 정방향수치
	short mStraightValue;

	/// 역방향수치
	short mInverseValue;

	/// 속성
	char mProperty;

	/// 에피소드(판)
	char mEpisode;

	/// 정방향해석 인덱스 
	unsigned long mStraightIdx;

	/// 역방향해석 인덱스
	unsigned long mInverseIdx;
};

inline cItemTarot::cItemTarot()
: mImageIndex(0)
, mNumber(0)
, mLevel(0)
, mStraightValue(0)
, mInverseValue(0)
, mProperty(0)
, mEpisode(0)
, mStraightIdx(0)
, mInverseIdx(0) 
{

}

/// 아이템 정의
class cItemDefine
{
	friend class cItemManager;

public:
	cItemDefine();

	inline LPCTSTR GetName() { return mName; }
	inline LPCTSTR GetDescription() { return mDescription; }
	inline unsigned char GetType() { return mType; }
	inline unsigned char GetSubType() {    return mSubType; }
	inline unsigned long GetHighType() {   return mHighType; }
	inline unsigned long GetMiddleType() { return mMiddleType; }
	inline unsigned long GetLowType() { return mLowType; }
	inline unsigned short GetTipType() { return mTooltipColor; }
	inline short GetWeight() { return mWeight; }
	inline unsigned short GetCapacity() { return mCapacity; }
	inline cIconParam* GetIconParam() { return mIconParam; }
	inline const char* GetFileName0( unsigned int race, unsigned int gender );
	inline const char* GetFileName1( unsigned int race, unsigned int gender );
	inline eItemEquip GetEquipType() { return mEquipType; }
	inline cItemCard* GetCardDefine() { return mCard; }
	inline cItemTarot*	GetTarotDefine() { return mTarot; }
	inline cItemSpread* GetSpreadDefine() { return mSpread; }
	inline sItemDisjoint* GetDisjoint() { return mDisjoint; }
	inline sItemPutCardSlot* GetPutCard() { return mPutCardSlot; } 
	inline cItemAbility* GetAbililty() { return mAbility; }
	inline cItemLimit* GetLimit() { return mLimit; }
	inline cEnhancedOptionArr*  GetEnhancedOptionArr() { return mEnhanceArr; }
	inline unsigned long GetBuyPrice() { return mBuyPrice; }
	inline unsigned long GetSellPrice() { return mSellPrice; }
	inline unsigned long GetBuyType() { return mBuyType; }
	inline unsigned long GetSellType() { return mSellType; }
	inline unsigned short GetPutSlotCount() { return mPutSlotCount; }
	inline char GetExchange() { return mExchange; }
	inline unsigned long GetTitleIndex() { return mTitleIndex; }
	inline unsigned long GetSetIndex() { return mSetIndex; }
	inline char GetHideEar() { return mHideEar; }		/// 0 : 귀표시 on, 1 : 귀표시 off
	inline unsigned int GetMaxHaveInven() { return mMaxHaveInven; }
	inline BYTE GetLicenseClass() { return mLicenseClass; }
	inline long GetLicenseValidThru() { return mLicenseValidThru; }
	inline BYTE GetLicenseType() { return mLicenseType; }

private:
	/// 이름
	LPCTSTR mName;

	/// 설명
	LPCTSTR mDescription;

	/// 종류
	unsigned char mType;

	/// 세부 종류
	unsigned char mSubType;
	
	/// 검색 대분류
	unsigned long mHighType;

	/// 검색 중분류
	unsigned long mMiddleType;

	/// 검색 소분류
	unsigned long mLowType;

	/// 무게
	short mWeight;

	/// 제한 수량
	unsigned short mCapacity;

	/// 판매 방법
	unsigned long mSellType;

	/// 판매가격
	unsigned long mSellPrice;

	/// 구입 방법
	unsigned long mBuyType;

	/// 구입가격
	unsigned long mBuyPrice;

	/// 최대 출현갯수
	unsigned long mPresentNum;

	/// 보유 가능 개수
	unsigned int mMaxHaveInven;

	// 기간제 설정(0일반/1실제시간/2게임시간/3무제한)
	BYTE	mLicenseClass;

	// 사용기한
	long    mLicenseValidThru;	

	// 인증여부(0인증완료/1미인증)
	BYTE    mLicenseType;					

	/// 툴팁 컬러 ( 레어도관련 )
	unsigned short mTooltipColor;

	// 홈페이지 공개여부(0공개/1공개안함)
	BYTE    mWebPage;

	/// 호칭 인덱스
	unsigned long mTitleIndex;

	/// 세트 인덱스
	unsigned long mSetIndex;

	/// 거래 가능 여부(0거래가능/1거래불가/2미인증거래가능/3미인증 거래불가)
	char mExchange;

	/// 능력
	cItemAbility* mAbility;

	/// 제한
	cItemLimit* mLimit;

	/// 카드 인덱스
	cItemCard* mCard;

	/// 타롯 인덱스 
	cItemTarot* mTarot;

	/// 스프레드 인덱스 
	cItemSpread* mSpread;

	/// 분해 정보
	sItemDisjoint* mDisjoint;

	/// 강화 정보
	cEnhancedOptionArr* mEnhanceArr;

	/// 카드장착 슬롯 정보
	sItemPutCardSlot* mPutCardSlot;
	unsigned short mPutSlotCount;			/// 카드장착 가능한 슬롯 갯수

	/// 아이콘 이미지
	cIconParam* mIconParam;

	/// 파일명 배열
	cString* mFileNames0;
	cString* mFileNames1;

	/// 장착 타입
	eItemEquip mEquipType;

	/// 투구 보기
	char mHideEar;

	/// 플랙
	eItemFlag mFlag;
};

inline
cItemDefine::cItemDefine()
: mName( 0 )
, mDescription( 0 )
, mType( 0 )
, mSubType( 0 )
, mWeight( 0 )
, mBuyPrice( 0 )
, mSellPrice( 0 )
, mAbility( 0 )
, mLimit( 0 )
, mCard( 0 )
, mTarot(0)
, mSpread( 0 )
, mDisjoint( 0 )
, mEnhanceArr( 0 )
, mPutCardSlot( 0 )
, mIconParam( 0 )
, mFileNames0( 0 )
, mFileNames1( 0 )
, mEquipType( ITEM_EQUIP_NORMAL )
, mFlag( ITEM_FLAG_NULL )
, mTooltipColor( 0 )
, mPutSlotCount( 0 )
, mTitleIndex( 0 )
, mExchange( false )
, mHideEar( 0 )
, mMaxHaveInven( 0 )
, mSetIndex(0)
, mHighType(0)
, mMiddleType(0)
, mLowType(0)
, mLicenseClass(0)
, mLicenseValidThru(0)
, mLicenseType(0)
, mWebPage(0)
{
}

inline
const char* cItemDefine::GetFileName0( unsigned int race, unsigned int gender )
{
	assert( race < eRACE_MAX );
	assert( gender < eGENDER_MAX );

	if( mFileNames0 == 0 )
		return 0;
	else
	{
		if( mType == ITEM_WEAPON )
		{
			/// None인 경우는 인간/남자 사용
			if( mFileNames0[race * eGENDER_MAX + gender] == "None" )
				return mFileNames0[0].Cstr();
			else
				return mFileNames0[race * eGENDER_MAX + gender].Cstr();
		}
		else
		{
			return mFileNames0[race * eGENDER_MAX + gender].Cstr();
		}
	}
}

inline
const char* cItemDefine::GetFileName1( unsigned int race, unsigned int gender )
{
	assert( race < eRACE_MAX );
	assert( gender < eGENDER_MAX );

	if( mFileNames1 == 0 )
		return 0;
	else
	{
		if( mType == ITEM_WEAPON )
		{
			/// None인 경우는 인간/남자 사용
			if( mFileNames1[race * eGENDER_MAX + gender] == "None" )
				return mFileNames1[0].Cstr();
			else
				return mFileNames1[race * eGENDER_MAX + gender].Cstr();
		}
		else
		{
			return mFileNames1[race * eGENDER_MAX + gender].Cstr();
		}
	}
}

struct sItemOnePieceInfo
{
	unsigned long mIndex;

	unsigned long mUpperIndex;
	unsigned long mLowerIndex;
	unsigned long mHandsIndex;
	unsigned long mFootsIndex;
};
