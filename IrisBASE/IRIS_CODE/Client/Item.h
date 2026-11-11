/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.06
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "ItemDefine.h"
#include "UIRoundGauge.h"
#include "UIRect.h"
#include "Icon.h"
typedef tUIRect<int> cUIRect;

class cItemIcon;
class cIconParam;
class cUIImageFont;
class cCoolData;

/// 아이템
/// 인벤토리에 들어가는 실제 아이템 정보를 포함한다.
class cItem
{
	friend class cItemManager;
	friend class cCoolTimeManager;

public:
	cItem();
	cItem( cItem& other );
	~cItem();

	void Clear();

	/// 아이콘을 연결 
	void BindIcon( cItemIcon* icon );
	cItemIcon* GetIcon() { return mIcon; }

	/// 아이템 사용 관련 활성화 설정
	void SetUsedFlag( bool enabled, bool colorchange = true );

	// 인증 설정
	void SetSeal( bool seal );

	// 캐쉬 설정
	void SetCash( bool cash );

	/// 아이템 락 : 모든 이벤트 불가 ( 툴팁제외 )
	/// 거래창, 노점창, 분해창등 UI에 등록할시에 사용한다 
	void SetLock( bool lock );
	bool IsLock() { return mLock; }
	
	unsigned long GetIndex() const;
	unsigned short GetCount() const;
	unsigned char GetEnhanceStep() const;
	cItemDefine* GetDefine();
	cItemTarot* GetTarotDefine();
	bool IsSeal() { return mSeal; } 
	bool IsCash() { return mCash; }
	sItemBill* GetBill() { return mItemBill; }

	unsigned long GetCardIndex1() { return mCardIndex1; }
	unsigned long GetCardIndex2() { return mCardIndex2; }
	unsigned long GetCardIndex3() { return mCardIndex3; }
	unsigned long GetCardIndex4() { return mCardIndex4; }
	unsigned long GetCardIndex5() { return mCardIndex5; }

	/// 쿨타임 복사
	void CopyCoolTime( unsigned long startTime, unsigned long restTime, unsigned long totalTime );
	bool StartCoolTime( unsigned long cool );
	void EndCoolTime();
	bool IsCoolTime() const;

	/// 컬러
	void SetColor( NiColorA color );
	NiColorA GetColor() { return mColor; }

	/// 다른 아이템과 위치 자체를 변경 ( 모든정보는 그대로 둠 )
	bool SwicthPosition( cUIRect& rect );

private:
	void SetTwoHanded( bool twoHanded, cIconParam* iconParam = 0, cItemDefine* define = 0 );
	void SetCount( unsigned short count );
	
	cItem& operator=( cItem& other );
	cItem& operator=( unsigned int i );
	void SetAniParam( sAniParam* param );

private:
	/// 아이템 인덱스
	unsigned long mIndex;

	/// 아이템 고유 인덱스 ( DB 인덱스 )
	unsigned long mDBIndex;

	/// 강화단계
	unsigned char mEnhanced;

	/// 아이템 수량
	unsigned short mCount;

	/// 아이템 봉인 여부
	bool mSeal;

	/// 캐쉬아이템 여부
	bool mCash;

	/// 아이템 장착 카드
	unsigned long  mCardIndex1;
	unsigned long  mCardIndex2;		
	unsigned long  mCardIndex3;		
	unsigned long  mCardIndex4;		
	unsigned long  mCardIndex5;		

	/// 아이템 정의
	cItemDefine* mDefine;

	/// 캐쉬 시간정보
	sItemBill* mItemBill;

	/// 장착 타입
	eItemEquip mEquipType;

	/// 플랙
	eItemFlag mFlag;

	/// 바인딩 아이콘 
	cItemIcon* mIcon;

	/// 사용 활성화
	bool mEnableUse;

	/// 락 : 모든 아이템관련 이벤트 불가
	bool mLock; 

	/// temp 아이템정보 ( 아이템 체인지시 temp인자 이용하므로 필요 )
	cUIRoundGauge mGauge;
	cString mNumText;
	NiColorA mColor;
	unsigned int mNumber;
	eTooltipType mTipType;		/// 레어도 관련
	bool mShowNumber;
	sAniParam mAniParam[eANI_MAX];
	cIconParam* mIconParam;
	cUIImageFont* mImageFont;
};

inline
cItem::cItem()
: mIndex( 0 )
, mDBIndex( 0 )
, mEnhanced( 0 )
, mCount( 0 )
, mSeal( false )
, mCash( false )
, mCardIndex1( 0 )
, mCardIndex2( 0 )
, mCardIndex3( 0 )
, mCardIndex4( 0 )
, mCardIndex5( 0 )
, mDefine( 0 )
, mEquipType( ITEM_EQUIP_NORMAL )
, mFlag( ITEM_FLAG_NULL )
, mIcon( 0 )
, mIconParam( 0 )
, mImageFont( 0 )
, mNumber( 0 )
, mEnableUse( true )
, mLock( false )
, mShowNumber( false )
, mColor( NiColorA::WHITE )
, mTipType( eTOOLTIP_NONE )
, mItemBill(0)
{
	::memset( mAniParam, 0, sizeof(mAniParam) );
}

inline
cItem::cItem( cItem& other )
: mIcon( 0 )
{
	operator=( other );
}

inline
cItem::~cItem()
{
}

inline
unsigned long cItem::GetIndex() const
{
	return mIndex;
}

inline
unsigned short cItem::GetCount() const
{
	return mCount;
}

inline
unsigned char cItem::GetEnhanceStep() const
{
	return mEnhanced;
}

inline
cItemDefine* cItem::GetDefine()
{
	return mDefine;
}

inline
cItemTarot* cItem::GetTarotDefine()
{
	return mDefine->GetTarotDefine();
}
