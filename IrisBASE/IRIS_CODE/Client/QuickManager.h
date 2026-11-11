/* ==========================================================================
* 파    일 : QuickManager.h
* 목    적 : 퀵슬롯 매니저
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-20
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Icon.h"
#include "Player_Common.h"
#include "Item_Common.h"

/// 몬스터 스킬아이콘 이미지 인덱스 ( 3 ~ 5 )
const int MONICON_PARAMINDEX = 3; 

/// 각각 퀵슬롯의 아이콘 개수
const int QUICKSLOT_ICON_COUNT_FOR_EACH = 10;

/// 퀵슬롯 아이콘 인덱스 정의
enum eQuickSlotIconIndex
{
	/// 일반 퀵슬롯
	QUICKSLOT_ICON_COMMON_BEGIN = 0,
	QUICKSLOT_ICON_COMMON_END = QUICKSLOT_ICON_COMMON_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,
	
	/// 추가1 퀵슬롯
	QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN,
	QUICKSLOT_ICON_COMMON_EXTRA1_END = QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// 추가2 퀵슬롯
	QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN,
	QUICKSLOT_ICON_COMMON_EXTRA2_END = QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// FS 퀵슬롯
	QUICKSLOT_ICON_FS_BEGIN,
	QUICKSLOT_ICON_FS_END = QUICKSLOT_ICON_FS_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// MONSTER 퀵슬롯( FS 퀵슬롯 1,2,3번째 사용 )
	QUICKSLOT_ICON_MON_BEGIN = QUICKSLOT_ICON_FS_BEGIN,
	QUICKSLOT_ICON_MON_END   = QUICKSLOT_ICON_MON_BEGIN + 3 - 1,

	QUICKSLOT_ICON_COUNT = QUICKSLOT_ICON_FS_END + 1,
};


/// 슬롯 정의
enum eQuickSlotIndex
{
	/// 일반 퀵슬롯
	QUICKSLOT_COMMON_BEGIN = 0,
	QUICKSLOT_COMMON_END = QUICKSLOT_COMMON_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// 추가1 퀵슬롯
	QUICKSLOT_COMMON_EXTRA1_BEGIN,
	QUICKSLOT_COMMON_EXTRA1_END   = QUICKSLOT_COMMON_EXTRA1_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// 추가2 퀵슬롯
	QUICKSLOT_COMMON_EXTRA2_BEGIN,
	QUICKSLOT_COMMON_EXTRA2_END   = QUICKSLOT_COMMON_EXTRA2_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// FS 퀵슬롯
	QUICKSLOT_FS1_BEGIN,				
	QUICKSLOT_FS1_END = QUICKSLOT_FS1_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	QUICKSLOT_FS2_BEGIN,				
	QUICKSLOT_FS2_END = QUICKSLOT_FS2_BEGIN + QUICKSLOT_ICON_COUNT_FOR_EACH - 1,

	/// MONSTER 퀵슬롯
	QUICKSLOT_MON_BEGIN,
	QUICKSLOT_MON_END = QUICKSLOT_MON_BEGIN + 3 - 1,

	QUICKSLOT_COUNT     = QUICKSLOT_FS2_END  - QUICKSLOT_COMMON_BEGIN + 1,  /// 일반 퀵(30) + FS퀵(20)
	QUICKSLOT_COUNT_MON	= QUICKSLOT_MON_END - QUICKSLOT_MON_BEGIN    + 1,  /// 몬스터퀵(3)
	QUICKSLOT_TOTAL		= QUICKSLOT_COUNT + QUICKSLOT_COUNT_MON,
};

/// 퀵슬롯 타입 정의
enum eQuickSlotType
{
	QUICKTYPE_COMMON,		    /// 일반    퀵슬롯
	QUICKTYPE_COMMON_EXTRA1,	/// 추가1   퀵슬롯
	QUICKTYPE_COMMON_EXTRA2,	/// 추가2   퀵슬롯
	QUICKTYPE_FS,				/// FS      퀵슬롯
	QUICKTYPE_MON,				/// MONSTER 퀵슬롯
	
	QUICKTYPE_COUNT,

	QUICKTYPE_BEGIN = QUICKTYPE_COMMON,
	QUICKTYPE_END   = QUICKTYPE_MON,

	QUICKTYPE_COMMON_COUNT = QUICKTYPE_COMMON_EXTRA2 - QUICKTYPE_COMMON + 1, /// 일반 퀵슬롯(1) + 추가 퀵슬롯(2)
};

class cIcon;
class cQuickData
{
public:
	cQuickData();
	cQuickData( cQuickData& other );

	/// 대입
	void operator=( const cQuickData& other );
	void operator=( unsigned int i );

public:
	/// 데이터 타입 ( 0 : 아이템류, 1 : 스킬류 )
	char mType;

	/// 고유 인덱스
	unsigned long mDBIndex;
	/// 아이템 인덱스
	unsigned long mItemIndex;

	/// 스킬 인덱스
	unsigned long mSkillIndex;
	unsigned char mSkillStep;
	
    /// 아이콘 컬러
	NiColorA mColor;

	/// 애니메이션
	sAniParam* mAniParam;

	/// 레어도 관련 팁 타입 ( 이미지 테두리 )
	unsigned int mTipType;
};

inline cQuickData::cQuickData()
: mType( eSHORTCUT_NONE )
, mDBIndex(0)
, mItemIndex(0)
, mSkillIndex(0)
, mSkillStep(0)
, mColor( NiColorA::WHITE )
, mAniParam(0)
, mTipType( eTOOLTIP_NONE )
{
}

inline cQuickData::cQuickData( cQuickData& other )
: mType( eSHORTCUT_NONE )
, mDBIndex(0)
, mItemIndex(0)
, mSkillIndex(0)
, mSkillStep(0)
, mColor( NiColorA::WHITE )
, mAniParam(0)
, mTipType( eTOOLTIP_NONE )
{
	operator=( other );
}

inline void cQuickData::operator=( const cQuickData& other )
{
	mType = other.mType;
	mDBIndex = other.mDBIndex;
	mItemIndex = other.mItemIndex;
	mSkillIndex = other.mSkillIndex;
	mSkillStep = other.mSkillStep;
	mColor = other.mColor;
	mAniParam = other.mAniParam;
	mTipType = other.mTipType;
}

inline void cQuickData::operator=( unsigned int i )
{
	if( i <= 0 )
	{
		mType = eSHORTCUT_NONE;
		mDBIndex = 0;
		mItemIndex = 0;
		mSkillIndex = 0;
		mSkillStep = 0;
		mColor = NiColorA::WHITE;
		mAniParam = 0;
		mTipType = eTOOLTIP_NONE;
	}
	else
	{
		assert(0);
	}
}

/// 아이콘 묶음
class cBindIcon
{
public:
	cBindIcon();

	/// 대입
	void operator=( const cBindIcon& other );
	void operator=( cIcon& other );
	
	void Clear();
	void SetParam( cIconParam* param );
	void SetCount( unsigned int count );
	void ShowNumber( bool show );
	void SetColor( const NiColorA& color );
	void SetColor( float r, float g, float b, float a );
	void SetAniParam( sAniParam* param );
	void SetTipType( unsigned int type );
	void StopDelay();

public:
	cIcon* mRowIcon;
	cIcon* mColumnIcon;
};

inline cBindIcon::cBindIcon()
: mRowIcon(0)
, mColumnIcon(0)
{
}

inline void cBindIcon::operator=( const cBindIcon& other )
{
	cIcon* icon = other.mColumnIcon;
	operator=( *icon );
}

inline void cBindIcon::operator=( cIcon& other )
{
	SetParam( other.GetIconParam() );
	SetCount( other.GetNumber() );
	ShowNumber( other.IsShowNumber() );

	SetAniParam( other.GetAniParam() );
	SetTipType( other.GetTipType() );

	mRowIcon->ChangeGauge( other.GetGauge() );
	mColumnIcon->ChangeGauge( other.GetGauge() );
}

inline void cBindIcon::Clear()
{
	SetParam(0);
	SetAniParam(0);
	SetColor( NiColorA::WHITE );
	SetCount(0);
	StopDelay();
	ShowNumber( false );
	SetTipType( eTOOLTIP_NONE );
}

inline void cBindIcon::SetParam( cIconParam* param )
{
	if( mRowIcon )
		mRowIcon->ChangeImage( param );
	if( mColumnIcon )
		mColumnIcon->ChangeImage( param );
}

inline void cBindIcon::SetCount( unsigned int count )
{
	if( mRowIcon )
		mRowIcon->SetNumber( count );
	if( mColumnIcon )
		mColumnIcon->SetNumber( count );
}

inline void cBindIcon::SetColor( float r, float g, float b, float a )
{
	if( mRowIcon )
		mRowIcon->SetColor( r, g, b, a );
	if( mColumnIcon )
		mColumnIcon->SetColor( r, g, b, a );
}

inline void cBindIcon::SetColor( const NiColorA& color )
{
	if( mRowIcon )
		mRowIcon->SetColor( color );
	if( mColumnIcon )
		mColumnIcon->SetColor( color );
}

inline void cBindIcon::ShowNumber( bool show )
{
	if( mRowIcon )
		mRowIcon->ShowNumber( show );
	if( mColumnIcon )
		mColumnIcon->ShowNumber( show );
}

inline void cBindIcon::SetAniParam( sAniParam* param )
{
	if( mRowIcon )
		mRowIcon->SetAniParam( param );
	if( mColumnIcon )
		mColumnIcon->SetAniParam( param );
}

inline void cBindIcon::SetTipType( unsigned int type )
{
	if( mRowIcon )
		mRowIcon->SetTipType( type );
	if( mColumnIcon )	
		mColumnIcon->SetTipType( type );
}

inline void cBindIcon::StopDelay()
{
	if( mRowIcon )
			mRowIcon->StopDelay();
	if( mColumnIcon )
		mColumnIcon->StopDelay();
}


/// 퀵슬롯 매니저 
class cQuickManager
{
public:
	static cQuickManager* mSingleton;

public:
	cQuickManager();
	~cQuickManager();

	///
	void Open();

	/// 모든 데이터 삭제
	void Clear();
	
	/// 네트워크 세팅 
	void NetSetData( unsigned int dataIndex, const sShortcut& shortcut );
	void NetUpdateData();

	void SkillReset();

	/// 퀵슬롯 사용 - 단축키
	void OnKeyDown( int iconID );
	void OnMouseDown( unsigned int iconId, bool IsFS = false );
	
	/// 퀵슬롯 업데이트
	void UpdateUI();
	bool UpdateIcon( int i, int iconId, bool countCheck = true );

	void UpdateSkillStep( unsigned long skillIndex, unsigned char step );

	// 튜토리얼 상태에서 퀵슬롯 갱신
	void UpdateTutorial( unsigned int tabindexfs );

	/// hero의 착용 무기와 스킬 적합 여부 검사
	void CheckSkillIcon();
	void CheckAllIcon();

	/// 일정 상황에서만 현재 보이는 탭만 검사한다
	void CheckItemIcon();	

	/// 퀵슬롯 탭체인지
	void ChangeTabFS( unsigned int tabindex );

	/// 퀵슬롯의 아이콘 위치와 복사할 아이콘 정보
	void AddData( unsigned int iconId, cIcon* srcIcon, bool toIsFS );
	void MoveData( unsigned int iconId, cIcon* srcIcon, bool toIsFS );
	void DeleteData();
	void DeleteData( unsigned int iconId, unsigned int dataIndex );

	/// 등록 데이터가 있는지 여부
    bool IsUseSlot( unsigned int iconId, bool IsFS = false );
	eSHORTCUT_TYPE GetQuickType( unsigned int iconId, bool IsFS = false );
	unsigned long GetItemIndex( unsigned int iconId, bool IsFS = false );
	unsigned long GetSkillIndex( unsigned int iconId, bool IsFS = false );
	unsigned char GetSkillStep( unsigned int iconId, bool IsFS = false );
	short GetMonSkillIndex( unsigned int iconId );

	void DragIcon( cIcon* icon );
	void UnDragIcon();

	/// UI 아이콘과 바인딩
	void BindRowIcon( unsigned int index, cIcon* icon );
	void BindColumnIcon( unsigned int index, cIcon* icon );
 	void BindFSRowIcon( unsigned int index, cIcon* icon );
 	void BindFSColumnIcon( unsigned int index, cIcon* icon );

	void SetRowQuick( bool IsRow , int type = QUICKTYPE_COMMON );
	bool IsRowQuick( int type = QUICKTYPE_COMMON );

	void SetRowQuickFS( bool IsRow );
	bool IsRowQuickFS() { return mIsRowQuickFS; }

	/// 아이콘 애니메이션 돌리기
	void SetSkillAniAura( unsigned long skillIndex, sAniParam* param );

	void SetCharge( unsigned long skillIndex, bool charge );
	void SetMonCharge( unsigned long skillIndex, bool charge );	/// 몬스터 스킬 챠지

	void LoadCoolTime();
	void LoadCoolTimeMon();
    
	/// 스킬관련 쿨타임
	void StartSkillCoolTime( unsigned long skillIndex, unsigned long cool );
	void StartMonSkillCoolTime( short skillIdx, unsigned long cool );
	void ClearCoolTime( unsigned long exceptSkillIdx );

	/// 몬스터 스킬 관련
	void ChangeMonster( unsigned long monIndex );

private:
	/// 퀵슬롯 사용
	void UseQuickSlot( unsigned int iconId, bool IsFS = false );
	void UseMonsterQuickSlot( unsigned int iconId );	

	bool UsableSkillIcon( eWEAPON_STATE heroWeaponState, unsigned long skillindex, unsigned char skillstep );
	bool UsableMonSkillIcon( unsigned long skillindex );

public:
	static cQuickManager* GetSingleton();

private:
	/// 퀵슬롯 정보 ( 일반 퀵(30) + FS 퀵(20) + 몬스터 퀵(3) )
	cQuickData mQuick[QUICKSLOT_TOTAL];

	/// 퀵슬롯 아이콘 정보 ( 일반 퀵 아이콘(30) + FS 퀵 아이콘(10) )
	cBindIcon  mBindIcon[QUICKSLOT_ICON_COUNT];

	/// 몬스터 변신으로 인한 퀵슬롯 락
	bool mIsLockByMonster;

	/// 퀵슬롯 탭 인덱스
	unsigned int mTabIndexFS;		/// FS 슬롯 인덱스( 무기 스왑할때 마다 바뀜. )

	/// 세로형 퀵슬롯 여부 ( 0 : 가로형, 1 : 세로형 )
	bool mIsRowQuick[QUICKTYPE_COMMON_COUNT];
 	bool mIsRowQuickFS;				/// FS 슬롯

	/// 드래그중인 아이콘 
	cIcon* mDragIcon;
};

inline void cQuickManager::BindRowIcon( unsigned int index, cIcon* icon )
{
	assert( icon );
	
	if( index < QUICKSLOT_ICON_COMMON_BEGIN || index >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	mBindIcon[index].mRowIcon = icon;
}

inline void cQuickManager::BindColumnIcon( unsigned int index, cIcon* icon )
{
	assert( icon );

	if( index < QUICKSLOT_ICON_COMMON_BEGIN || index >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	mBindIcon[index].mColumnIcon = icon;	
}

inline void cQuickManager::BindFSRowIcon( unsigned int index, cIcon* icon )
{
	assert( icon );

	if( index < QUICKSLOT_ICON_FS_BEGIN || index > QUICKSLOT_ICON_FS_END )
	{
		assert(0);
		return;
	}

	mBindIcon[index].mRowIcon = icon;
}

inline void cQuickManager::BindFSColumnIcon( unsigned int index, cIcon* icon )
{
	assert( icon );

	if( index < QUICKSLOT_ICON_FS_BEGIN || index > QUICKSLOT_ICON_FS_END )
	{
		assert(0);
		return;
	}

	mBindIcon[index].mColumnIcon = icon;	
}

inline 
cQuickManager* cQuickManager::GetSingleton()
{
	return mSingleton;
}

#define QUICKMAN cQuickManager::GetSingleton()
