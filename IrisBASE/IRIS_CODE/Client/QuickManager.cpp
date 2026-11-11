#include "stdafx.h"
#include "QuickManager.h"
#include "ItemIcon.h"
#include "SkillIcon.h"
#include "SkillScript.h"
#include "CommunityScript.h"
#include "ItemManager.h"
#include "CoolTimeManager.h"

#include "Application.h"
#include "SkillManager.h"
#include "SkillExecuter.h"
#include "ObjectManager.h"
#include "hero.h"

#include "GameUIManager.h"
#include "SkillWindow.h"

#include "SoundSystem.h"
#include "TutorialManager.h"
#include "PVPManager.h"

cQuickManager* cQuickManager::mSingleton = 0;

cQuickManager::cQuickManager()
: mTabIndexFS(0)
, mIsRowQuickFS( false )
, mDragIcon(0)
, mIsLockByMonster( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	for( unsigned int data = 0; data < QUICKSLOT_TOTAL; ++data )
	{
		mQuick[data] = 0;
	}

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		mBindIcon[i].Clear();
	}

	for( unsigned int i = 0; i < QUICKTYPE_COMMON_COUNT; ++i )
		mIsRowQuick[i] = false;
}

cQuickManager::~cQuickManager()
{
}

void cQuickManager::Clear()
{
	mDragIcon = 0;
	mIsLockByMonster = false;

	/// 데이터초기화
	for( unsigned int data = 0; data < QUICKSLOT_TOTAL; ++data )
	{
		mQuick[data] = 0;
	}

	/// 아이콘 초기화
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		mBindIcon[i].Clear();
	}
}

void cQuickManager::Open()
{
	UpdateTutorial( mTabIndexFS );

	UpdateUI();

	/// 
	sPlayerExrInfo* info = HERO->GetPlayerExrInfo();
	if( info )
	{
		ChangeMonster( info->mChgMonsterIdx );
	}
	else
		assert(0);
}

void cQuickManager::SetRowQuick( bool IsRow, int type )
{
	if ( type < QUICKTYPE_COMMON || type > QUICKTYPE_COMMON_EXTRA2  )
	{
		assert(0);
		return;
	}
	
	mIsRowQuick[type] = IsRow;
}

bool cQuickManager::IsRowQuick( int type )
{
	if ( type < QUICKTYPE_COMMON || type > QUICKTYPE_COMMON_EXTRA2  )
	{
		assert(0);
		return false;
	}

	return mIsRowQuick[type];
}

void cQuickManager::SetRowQuickFS( bool IsRow )
{
	mIsRowQuickFS = IsRow;
}

///
bool cQuickManager::IsUseSlot( unsigned int iconId, bool IsFS )
{
	/// 몬스터 변신상태이고, 0, 1, 2, 아이콘이면 몬스터용 스킬 타입 리턴
	if( IsFS && mIsLockByMonster && ( iconId >= QUICKSLOT_ICON_MON_BEGIN && iconId <= QUICKSLOT_ICON_MON_END ) )
		return false;

	int dataIndex = iconId;
	if( IsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return false;
	}

	/// 사용하고 있지 않음.
	if( mQuick[dataIndex].mType == eSHORTCUT_NONE )
		return false;

	return true;
}

///
eSHORTCUT_TYPE cQuickManager::GetQuickType( unsigned int iconId, bool IsFS )
{
	int dataIndex = -1;

	if( IsFS == false )
	{
		dataIndex = iconId;
	}
	else
	{
		/// 몬스터 변신상태이고, 0, 1, 2, 아이콘이면 몬스터용 스킬 타입 리턴
		if( mIsLockByMonster == true && ( iconId >= QUICKSLOT_ICON_MON_BEGIN && iconId <= QUICKSLOT_ICON_MON_END ) )
		{
			dataIndex = QUICKSLOT_MON_BEGIN + (iconId - QUICKSLOT_ICON_MON_BEGIN);
		}
		else
		{
			dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
		}
	}

	if( dataIndex == -1 )
	{
		assert(0);
		return eSHORTCUT_NONE;
	}

	return (eSHORTCUT_TYPE)mQuick[dataIndex].mType;
}

///
unsigned long cQuickManager::GetItemIndex( unsigned int iconId, bool IsFS )
{
	int dataIndex = iconId;
	if( IsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return 0;
	}

	return mQuick[dataIndex].mItemIndex;
}

///
unsigned long cQuickManager::GetSkillIndex( unsigned int iconId, bool IsFS )
{
	int dataIndex = iconId;
	if( IsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return 0;
	}

	return mQuick[dataIndex].mSkillIndex;
}

unsigned char cQuickManager::GetSkillStep( unsigned int iconId, bool IsFS )
{
	int dataIndex = iconId;
	if( IsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return 0;
	}

	return mQuick[dataIndex].mSkillStep;
}



short cQuickManager::GetMonSkillIndex( unsigned int iconId )
{
	if( iconId < QUICKSLOT_ICON_MON_BEGIN || iconId > QUICKSLOT_ICON_MON_END )
	{
		assert(0);
		return -1;
	}

	int dataIndex = QUICKSLOT_MON_BEGIN + (iconId - QUICKSLOT_ICON_MON_BEGIN);

	return (short)mQuick[dataIndex].mSkillIndex;
}

///
void cQuickManager::OnMouseDown( unsigned int iconId, bool IsFS )
{
	/// 몬스터 변신상태면,
	if( IsFS && mIsLockByMonster && ( iconId >= QUICKSLOT_ICON_MON_BEGIN && iconId <= QUICKSLOT_ICON_MON_END ) )
		UseMonsterQuickSlot( iconId );
	else
		UseQuickSlot( iconId, IsFS );
}

///
void cQuickManager::OnKeyDown(  int iconID  )
{	
	if( iconID < QUICKSLOT_ICON_COMMON_BEGIN || iconID >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	bool IsFS = false;
	if( iconID >= QUICKSLOT_ICON_FS_BEGIN && iconID <= QUICKSLOT_ICON_FS_END )
		IsFS = true;

	/// 몬스터 변신상태면,
	if( IsFS && mIsLockByMonster && ( iconID >= QUICKSLOT_ICON_MON_BEGIN && iconID <= QUICKSLOT_ICON_MON_END ) )
		UseMonsterQuickSlot( iconID );
	else
		UseQuickSlot( iconID, IsFS );
}

/// 퀵슬롯 사용 
void cQuickManager::UseQuickSlot( unsigned int iconId, bool IsFS )
{
	int dataIndex = iconId;
	if( IsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}
	
	if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
	{
		HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

		/// 스킬
		if( mQuick[dataIndex].mSkillIndex < NORMAL_ATTACK_SKILL_MAX )
		{
			if( HERO->GetState() != eOBJECT_STATE_ATTACK )
				SKILLEXECUTER->ExecuteAttack( HERO->GetTargetObject() );
		}
		else
		{
			if( SKILLMAN->IsUsedKeepSkill( mQuick[dataIndex].mSkillIndex, mQuick[dataIndex].mSkillStep ) == true )
				SKILLEXECUTER->ExecuteSkill( mQuick[dataIndex].mSkillIndex, mQuick[dataIndex].mSkillStep );
		}

		UIMAN->HideTip();
	}
	else if( mQuick[dataIndex].mType == eSHORTCUT_ITEM )
	{
		/// 아이템
		sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count > 0 )
		{
			unsigned short slotIndex = ITEMMAN->FindSlotIndex( mQuick[dataIndex].mItemIndex );
			if( slotIndex <= 0 )
			{
				assert(0 && "failed to find slotindex" );
				return;
			}

			/// 아이템 사용 
			ITEMMAN->TryToEquipItem( slotIndex );
			UIMAN->HideTip();
		}
	}
	else if( mQuick[dataIndex].mType == eSHORTCUT_COMMU )
	{
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( mQuick[dataIndex].mSkillIndex );
		if( pInfo )
		{
			if( pInfo->mType == 1 )
			{
				if( HERO->GetState() == eOBJECT_STATE_IDLE )
				{
					cSkillWindow* win = GAMEUI->GetSkillWindow();
					if( win )
						win->RequestCommuSkill( pInfo->mIndex );
				}
			}
			else
			{
				cSkillWindow* win = GAMEUI->GetSkillWindow();
				if( win )
					win->RequestLifeCommand( pInfo->mIndex, 0, 0 );
			}
		}
	}
}

void cQuickManager::UseMonsterQuickSlot( unsigned int iconId )
{
	/// 몬스터 변신이 안되어있으면 리턴
	if( HERO->IsTransformMonster() == false )
		return;

	if( iconId < QUICKSLOT_ICON_MON_BEGIN || iconId > QUICKSLOT_ICON_MON_END )
	{
		assert(0);
		return;
	}

	int dataIndex = QUICKSLOT_MON_BEGIN + (iconId - QUICKSLOT_ICON_MON_BEGIN);
	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}

	/// 연결된 몬스터 스킬이 없는 경우 리턴
	if( mQuick[dataIndex].mSkillIndex < eMONSTERATTACK_SKILL1 || 
		mQuick[dataIndex].mSkillIndex >= eMONSTERATTACK_MAX )
		return;


	cIcon* icon = mBindIcon[iconId].mColumnIcon;
	if( !icon ) 
	{
		assert(0);
		return;
	}

	if( icon->IsActive() == true )
		return;

	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	if( SKILLMAN->IsUsedMonsterSkill( mQuick[dataIndex].mSkillIndex ) == true )
		SKILLEXECUTER->ExecuteSkill( mQuick[dataIndex].mSkillIndex, mQuick[dataIndex].mSkillStep );


	UIMAN->HideTip();
}

/// 탭 체인지 
void cQuickManager::ChangeTabFS( unsigned int tabindex )
{
	mTabIndexFS = tabindex;

	/// FS 퀵슬롯
	unsigned int slot = QUICKSLOT_FS1_BEGIN + (mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		int iconId = QUICKSLOT_ICON_FS_BEGIN + i;
		UpdateIcon( slot + i, iconId, true );
	}
	
	/// 몬스텨 변신일경우 FS퀵슬롯 재세팅
	if( mIsLockByMonster == true )
	{
		for( int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].Clear();

			/// 퀵슬롯 아이콘 세팅
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].ShowNumber( false );
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].SetColor( mQuick[QUICKSLOT_MON_BEGIN+i].mColor );
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].SetParam( UIMAN->GetIconParam(MONICON_PARAMINDEX + i) );
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].SetAniParam( mQuick[QUICKSLOT_MON_BEGIN+i].mAniParam );
			mBindIcon[ QUICKSLOT_ICON_MON_BEGIN+i ].SetTipType( mQuick[QUICKSLOT_MON_BEGIN+i].mTipType );
		}
	}

	/// 쿨타임 로딩
	LoadCoolTime();
	LoadCoolTimeMon();
}

//
void cQuickManager::UpdateTutorial( unsigned int tabindexfs )
{
	if( TUTORIALMAN->IsTutorialMode() == false )
		return;

	if( HERO )
	{
		// 셋팅
		eWEAPON_STATE weaponState = HERO->GetWeaponState();

		// 정렬을 위함
		typedef tList<unsigned long> cSortList;
		cSortList sortList;

		typedef tPointerHashMap<unsigned long, void*> cSkillMap;
		cSkillMap* skillMap = SKILLMAN->GetKeepSkillMap();
		if( skillMap )
		{
			cSkillMap::cIterator i = skillMap->Begin();
			cSkillMap::cIterator end = skillMap->End();
			for( ; i != end; ++i )
			{
				unsigned long skillIndex = (unsigned long)(i->mFirst);
				sKeepSkill*   keepSkill  = (sKeepSkill*)(i->mSecond);
				if( !keepSkill )
				{
					assert(0);
					return;
				}

				sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
				if( pInfo )
				{
					// 패시브는 넘김
					if( pInfo->mType == eSKILLTYPE_PASSIVE )
						continue;

					if( pInfo->mUseEquipment )
					{
						/// 착용 장비 체크
						switch( pInfo->mUseEquipment )
						{
						case eEQUIPTYPE_SWORD:
							if( weaponState != eWEAPON_STATE_SWORD &&
								weaponState != eWEAPON_STATE_SWORD_SHEILD )
								continue;
							break;
						case eEQUIPTYPE_LONGSWORD:
							if( weaponState != eWEAPON_STATE_LONGSWORD )
								continue;
							break;
						case eEQUIPTYPE_DOUBLESWORD:
							if( weaponState != eWEAPON_STATE_DOUBLESWORD )
								continue;
							break;
						case eEQUIPTYPE_SHORTSWORD:
							if( weaponState != eWEAPON_STATE_SHORTSWORD )
								continue;
							break;
						case eEQUIPTYPE_GUN:
							if( weaponState != eWEAPON_STATE_GUN )
								continue;
							break;
						case eEQUIPTYPE_STAFF:
							if( weaponState != eWEAPON_STATE_STAFF )
								continue;
							break;
						case eEQUIPTYPE_SHIELD:
							if( weaponState != eWEAPON_STATE_SHEILD &&
								weaponState != eWEAPON_STATE_SWORD_SHEILD )
								continue;
							break;
						case eEQUIPTYPE_SWORD_LONGSWORD:
							if( weaponState != eWEAPON_STATE_SWORD &&
								weaponState != eWEAPON_STATE_SWORD_SHEILD &&
								weaponState != eWEAPON_STATE_LONGSWORD )
								continue;
							break;
						case eEQUIPTYPE_SWORD_DOUBLESWORD:
							if( weaponState != eWEAPON_STATE_SWORD &&
								weaponState != eWEAPON_STATE_SWORD_SHEILD &&
								weaponState != eWEAPON_STATE_DOUBLESWORD )
								continue;
							break;
						case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
							if( weaponState != eWEAPON_STATE_LONGSWORD &&
								weaponState != eWEAPON_STATE_DOUBLESWORD )
								continue;
							break;
						case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
							if( weaponState != eWEAPON_STATE_SWORD &&
								weaponState != eWEAPON_STATE_SWORD_SHEILD &&
								weaponState != eWEAPON_STATE_LONGSWORD &&
								weaponState != eWEAPON_STATE_DOUBLESWORD )
								continue;
							break;
						case eEQUIPTYPE_SHORTSWORD_GUN:
							if( weaponState != eWEAPON_STATE_SHORTSWORD &&
								weaponState != eWEAPON_STATE_GUN )
								continue;
							break;
						}
					}

					sortList.PushBack( skillIndex );
				}
			}
		}

		// 초기화
		for( int i = QUICKSLOT_FS1_BEGIN; i <= QUICKSLOT_FS2_END; ++i )
		{
			mQuick[i] = 0;
		}

		// 정렬 데이터 담기
		unsigned int slot = ( tabindexfs * QUICKSLOT_ICON_COUNT_FOR_EACH ) + QUICKSLOT_FS1_BEGIN;
		unsigned int count = 0;

		// sort
		sortList.Sort();

		cSortList::cIterator si = sortList.Begin();
		cSortList::cIterator send = sortList.End();
		for( ; si != send; ++si, ++count )
		{
			if( count >= QUICKSLOT_ICON_COUNT_FOR_EACH )
				break;
	
			unsigned long skillIndex = (unsigned long)(*si);

			/// 퀵슬롯 정보 세팅 
			mQuick[slot].mType = eSHORTCUT_SKILL;
			mQuick[slot].mSkillIndex = skillIndex;
			mQuick[slot].mDBIndex = 0;
			mQuick[slot].mItemIndex = 0;
			
			slot++;	
		}
	}
}

void cQuickManager::DragIcon( cIcon* icon )
{
	if( icon )
	{
		mDragIcon = icon;
		mDragIcon->ShowGauge( false );
		mDragIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
	}
}

///
void cQuickManager::UnDragIcon()
{
	if( mDragIcon )
	{
		cUIContainer* fromParent = mDragIcon->GetParent();
		if( !fromParent )
		{
			assert(0);
			return;
		}

		/// fs 슬롯에서 온건지 판별
		bool fromIsFS = false;
		if( fromParent->GetID() == eUIID_ROW_QUICKSLOT_FS || fromParent->GetID() == eUIID_COLUMN_QUICKSLOT_FS ) 
			fromIsFS = true;

		int dataIndex = mDragIcon->GetID();
		if( fromIsFS == true )
			dataIndex = mDragIcon->GetID() + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
		
		/// 기존 저장되어있던 컬러로 변환 
		mDragIcon->ShowGauge( true );
		mDragIcon->SetColor( mQuick[dataIndex].mColor );
	}
}

/// 서버로부터 기존의 퀵슬롯 정보를 받아 세팅
void cQuickManager::NetSetData( unsigned int dataIndex, const sShortcut& shortcut )
{
	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex>= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}

	if( mQuick[dataIndex].mType != eSHORTCUT_NONE )
	{
		assert(0 && "already exist");
	}

	/// 퀵슬롯 정보 세팅 
	mQuick[dataIndex].mType = shortcut.mType;
	
	if( shortcut.mType == eSHORTCUT_ITEM )
	{
		mQuick[dataIndex].mItemIndex = shortcut.mIndex;
		mQuick[dataIndex].mDBIndex = shortcut.mExData;
	}
	else if( shortcut.mType == eSHORTCUT_SKILL )
	{
		mQuick[dataIndex].mSkillIndex = shortcut.mIndex;
		mQuick[dataIndex].mSkillStep = (unsigned char)shortcut.mExData;
	}
	else if( shortcut.mType == eSHORTCUT_COMMU )
	{
		mQuick[dataIndex].mSkillIndex = shortcut.mIndex;
	}
}	

///
void cQuickManager::NetUpdateData()
{
	if( TUTORIALMAN->IsTutorialMode() == true ) 
		return;

	/// 서버에 퀵슬롯 정보 보내기
	MSG_REQ_PLAYER_SHORTCUT_UPDATE msg;
	msg.Category = NM_PLAYER;
	msg.Protocol = NM_PLAYER_SHORTCUT_UPDATE_REQ;

	/// 변신상태 스킬은 등록할 필요없다!
	for( unsigned int i = 0; i < QUICKSLOT_COUNT; ++i )
	{
		msg.mShortcut[i].mType = mQuick[i].mType;
		
		if( mQuick[i].mItemIndex > 0 )
		{
			msg.mShortcut[i].mIndex = mQuick[i].mItemIndex;
			msg.mShortcut[i].mExData = mQuick[i].mDBIndex; 
		}
		else if( mQuick[i].mSkillIndex > 0 )
		{
			msg.mShortcut[i].mIndex = mQuick[i].mSkillIndex;
			msg.mShortcut[i].mExData = mQuick[i].mSkillStep; 
		}
	}

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
}

void cQuickManager::SkillReset()
{
	/// 변신상태 스킬은 등록할 필요없다!
	for( unsigned int i = 0; i < QUICKSLOT_COUNT; ++i )
	{
		if( mQuick[i].mType == eSHORTCUT_SKILL )
		{
			/// 데이터초기화
			mQuick[i] = 0;
		}
	}

	UpdateUI();

	/// 
	if( mIsLockByMonster == true )
	{
		for( int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			unsigned int dataIndex = QUICKSLOT_MON_BEGIN      + i;
			unsigned int iconId	   = QUICKSLOT_ICON_MON_BEGIN + i;

			/// 초기화
			mQuick[dataIndex] = 0;
			mBindIcon[iconId].Clear();

			sMonsterSkillScript* skill = SKILLSCRIPT->GetMonsterSkillInfo( HERO->GetTransMonsterClassIdx(), (eMONSTERATTACK_TYPE)(eMONSTERATTACK_SKILL1 + i) );
			if( skill )
			{
				mQuick[dataIndex].mType = eSHORTCUT_MONSKILL;
				mQuick[dataIndex].mSkillIndex = eMONSTERATTACK_SKILL1 + i;

				if( UsableMonSkillIcon( mQuick[dataIndex].mSkillIndex ) == true )
				{
					mQuick[dataIndex].mColor = NiColorA::WHITE;
				}
				else
				{
					mQuick[dataIndex].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
			}
			else
			{
				mQuick[dataIndex].mType = eSHORTCUT_MONSKILL;
				mQuick[dataIndex].mSkillIndex = ULONG_MAX;
				mQuick[dataIndex].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
			}

			/// 퀵슬롯 아이콘 세팅
			mBindIcon[iconId].ShowNumber( false );
			mBindIcon[iconId].SetColor( mQuick[dataIndex].mColor );
			mBindIcon[iconId].SetParam( UIMAN->GetIconParam(MONICON_PARAMINDEX + i) );
			mBindIcon[iconId].SetColor( mQuick[dataIndex].mColor );
			mBindIcon[iconId].SetTipType( mQuick[dataIndex].mTipType );
		}

		LoadCoolTimeMon();
	}
}

///
void cQuickManager::AddData( unsigned int iconId, cIcon* srcIcon, bool toIsFS )
{
	int dataIndex = iconId;
	if( toIsFS == true )
		dataIndex = iconId + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	/// 범위 체크
	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}

	if( iconId < QUICKSLOT_ICON_COMMON_BEGIN || iconId >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	if( srcIcon == NULL )
	{
		assert(0);
		return;
	}

	/// 비어있으면 등록
	unsigned int type = srcIcon->GetIconType();
	switch( type )
	{
	case eICON_ITEM:
		{
			cItemIcon* itemIcon = (cItemIcon*)srcIcon;
			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIcon->GetItemIndex() );
			if( !itemDefine )
			{
				assert(0);
				return;
			}

			// 봉인된 아이템은 등록이 안됨.
			if( itemIcon->IsSeal() == true )
				return;

			/// 일정 타입만 등록됨
			unsigned char type = itemDefine->GetType();	
			unsigned char subtype = itemDefine->GetSubType();
			if( type == ITEM_ETC1 || type == ITEM_MONSTER || (type == ITEM_CARD && subtype == ITEM_CARD_COMMON) 
				|| (type == ITEM_MATERIAL && subtype == ITEM_MATERIAL_MAP)
				|| (type == ITEM_MATERIAL && subtype == ITEM_MATERIAL_VEHICLE) )
			{
				/// 자리검사 - 기존데이터가 있으면 삭제 처리
				if( mQuick[dataIndex].mType != eSHORTCUT_NONE )
					DeleteData( iconId, dataIndex );

				mQuick[dataIndex].mType		= eSHORTCUT_ITEM;
				mQuick[dataIndex].mDBIndex	= itemIcon->GetDBIndex();
				mQuick[dataIndex].mItemIndex= itemIcon->GetItemIndex();
				mQuick[dataIndex].mAniParam	= itemIcon->GetAniParam();
				mQuick[dataIndex].mTipType	= itemIcon->GetTipType();

				/// 총 갯수 설정
				sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
				unsigned int count = ( p ) ? p->bag : 0;
				if( count <= 0 )
				{
					mQuick[dataIndex].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
				}
				else
				{
					int ret0 = ITEMMAN->CheckLimit( mQuick[dataIndex].mItemIndex );
					int ret1 = ITEMMAN->CheckLimitEtc( mQuick[dataIndex].mItemIndex );

					NiColorA color = NiColorA::WHITE;
					if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
					{
						color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
					}
					mQuick[dataIndex].mColor = color;
				}

				/// 아이콘세팅
				mBindIcon[iconId].Clear();

				mBindIcon[iconId] = *srcIcon;
				mBindIcon[iconId].SetAniParam( itemIcon->GetAniParam() );
				mBindIcon[iconId].SetTipType( itemIcon->GetTipType() );
				mBindIcon[iconId].SetCount( count );
				mBindIcon[iconId].ShowNumber( true );
				mBindIcon[iconId].SetColor( mQuick[dataIndex].mColor );
				//mBindIcon[iconId].SetColor( mQuick[i].mColor );

				/// 아이템 추가 시 사운드 재생
				ITEMMAN->PlayIconSettingSound( itemIcon->GetSlotIndex() );
			}
		}
		break;
	case eICON_SKILL:
		{
			/// 자리검사 /// 기존데이터가 있으면 삭제 처리
			if( mQuick[dataIndex].mType != eSHORTCUT_NONE )
				DeleteData( iconId, dataIndex );

			cSkillIcon* skillIcon = (cSkillIcon*)srcIcon;
			mQuick[dataIndex].mType = eSHORTCUT_SKILL;
			mQuick[dataIndex].mSkillIndex = skillIcon->GetSkillIdx();
			mQuick[dataIndex].mSkillStep = skillIcon->GetSkillStep();
			mQuick[dataIndex].mAniParam = skillIcon->GetAniParam();
			mQuick[dataIndex].mTipType = eTOOLTIP_NONE;


			/// 아이콘세팅
			mBindIcon[iconId].Clear();

			mBindIcon[iconId] = *srcIcon;
			mBindIcon[iconId].SetAniParam( skillIcon->GetAniParam() );
			mBindIcon[iconId].SetTipType( eTOOLTIP_NONE );
			mBindIcon[iconId].SetCount( skillIcon->GetSkillStep()+1 );

			// 오라 타입이면
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( mQuick[dataIndex].mSkillIndex );
			if( info && info->mShotType == eSHOTTYPE_AURA )
			{
				mQuick[dataIndex].mSkillStep = 0;

				sKeepSkill* p = SKILLMAN->GetKeepInfo( mQuick[dataIndex].mSkillIndex );
				if( p )
					mQuick[dataIndex].mSkillStep = p->mStep;
				else
					assert(0);
			}


			/// 
			eWEAPON_STATE wstate = HERO->GetWeaponState();
			if( UsableSkillIcon( wstate, skillIcon->GetSkillIdx(), skillIcon->GetSkillStep() ) == true )
			{
				//mQuick[dataIndex].mColor =  NiColorA::WHITE;
				mQuick[dataIndex].mColor =  NiColorA::WHITE;
				mBindIcon[iconId].SetColor( NiColorA::WHITE );
			}
			else
			{
				//mQuick[dataIndex].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				mQuick[dataIndex].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				mBindIcon[iconId].SetColor( NiColorA( 1.0f, 0.0f, 0.0f, 1.0f ) );
			}

			/// 퀵슬롯 추가 시 사운드(잡화,기타) 재생
			SOUNDSYS->Play2DSound( 3036 );
		}
		break;
	case eICON_COMMUNITY:
		{
			if( mQuick[dataIndex].mType != eSHORTCUT_NONE )
				DeleteData( iconId, dataIndex );

			/// 
			cCommunityIcon* comIcon = (cCommunityIcon*)srcIcon;
			mQuick[dataIndex].mType = eSHORTCUT_COMMU;
			mQuick[dataIndex].mSkillIndex = comIcon->GetCommunityIndex();
			mQuick[dataIndex].mColor =  NiColorA::WHITE;


			/// 아이콘세팅
			mBindIcon[iconId].Clear();

			mBindIcon[iconId] = *srcIcon;
			mBindIcon[iconId].ShowNumber( false );
			mBindIcon[iconId].SetColor( NiColorA::WHITE );

			/// 퀵슬롯 추가 시 사운드(잡화,기타) 재생
			SOUNDSYS->Play2DSound( 3036 );
		}
		break;
	default:
		assert(0);
		return;
	}

	NetUpdateData();
}

/// fromIsFS: 시작 지점 아이콘이 fs 슬롯인지 여부
/// toIsFS: 도착 지점 아이콘이 fs 슬롯인지 여부
void cQuickManager::MoveData( unsigned int toIconID, cIcon* srcIcon, bool toIsFS )
{
	if( mDragIcon == 0 || srcIcon == 0 )
	{
		assert(0);
		return;
	}

	cUIContainer* fromParent = mDragIcon->GetParent();
	if( !fromParent )
	{
		assert(0);
		return;
	}

	bool fromIsFS = false;
	if( fromParent->GetID() == eUIID_ROW_QUICKSLOT_FS || fromParent->GetID() == eUIID_COLUMN_QUICKSLOT_FS ) 
		fromIsFS = true;

	int fromDataIndex = mDragIcon->GetID();
	if( fromIsFS == true )
		fromDataIndex = mDragIcon->GetID() + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
	
	int toDataIndex = toIconID;
	if( toIsFS )
		toDataIndex = toIconID + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

	/// 범위 체크
	if( toIconID < QUICKSLOT_ICON_COMMON_BEGIN || toIconID >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	if( fromDataIndex < QUICKSLOT_COMMON_BEGIN || fromDataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}

	if( toDataIndex < QUICKSLOT_COMMON_BEGIN || toDataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}


	/// 자리검사 
	if( mQuick[toDataIndex].mType != eSHORTCUT_NONE )
	{
		/// 다른데이터가 있으면 스왑
		cQuickData temp = mQuick[fromDataIndex];
		mQuick[fromDataIndex] = mQuick[toDataIndex];
		mQuick[toDataIndex] = temp;

		/// 아이콘 정보 스왑
		cBindIcon tempIcon;
		tempIcon.mColumnIcon = srcIcon;
		tempIcon.mRowIcon = srcIcon;
		
		int fromIconID = mDragIcon->GetID();
	
		tempIcon = mBindIcon[toIconID];
		mBindIcon[toIconID] = mBindIcon[fromIconID];
		mBindIcon[fromIconID] = tempIcon;

		/// 색 변경
		mBindIcon[toIconID].SetColor( mQuick[toDataIndex].mColor );
		mBindIcon[fromIconID].SetColor( mQuick[fromDataIndex].mColor );

		/// 애니메이션 세팅
		mBindIcon[toIconID].SetAniParam( mQuick[toDataIndex].mAniParam );
		mBindIcon[fromIconID].SetAniParam( mQuick[fromDataIndex].mAniParam );

		/// 레어도타입 세팅
		mBindIcon[toIconID].SetTipType( mQuick[toDataIndex].mTipType );
		mBindIcon[fromIconID].SetTipType( mQuick[fromDataIndex].mTipType );
	}
	else
	{
		/// 없으면 이동 
		mQuick[toDataIndex] = mQuick[fromDataIndex];

		/// 아이콘 세팅
		mBindIcon[toIconID] = *mDragIcon;
		mBindIcon[toIconID].SetColor( mQuick[toDataIndex].mColor );
		mBindIcon[toIconID].SetAniParam( mQuick[toDataIndex].mAniParam );
		mBindIcon[toIconID].SetTipType( mQuick[toDataIndex].mTipType );

		/// 삭제 
		int fromIconID = mDragIcon->GetID();
		
		DeleteData( fromIconID, fromDataIndex );
	}

	NetUpdateData();

	/// 슬롯 셋팅 시 사운드 재생
	switch( mQuick[toDataIndex].mType )
	{
	case eSHORTCUT_ITEM:
		{
			unsigned short slotIndex = ITEMMAN->FindSlotIndex( mQuick[toDataIndex].mItemIndex );
			if( slotIndex >= INVENTORY_BAG0_BEGIN && slotIndex <= HERO->GetBagEndSlot() )
			{
				ITEMMAN->PlayIconSettingSound( slotIndex );
			}
			else 
			{
				/// 아이템을 다 소모한 경우 소리 나지 않기
				break;
			}
		}
		break;
	default:
		{
			/// 아이템이 아닌 것은 잡화(기타) 사운드 재생
			SOUNDSYS->Play2DSound( 3036 );
		}
		break;
	}
}

void cQuickManager::UpdateSkillStep( unsigned long skillIndex, unsigned char step )
{
	bool netUpdate = false;
	for( unsigned int i = 0; i < QUICKSLOT_COUNT; ++i )
	{
		/// 스킬 아이콘만 세팅
		if( mQuick[i].mType != eSHORTCUT_SKILL )
			continue;

		// 동일스킬의 정보 갱신
		if( mQuick[i].mSkillIndex == skillIndex )
		{
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
			if( info )
			{
				if( mQuick[i].mSkillStep >= info->mStepCount )
				{
					assert(0);
					mQuick[i] = 0;
					continue;
				}

				// 오라 타입이면
				if( info->mShotType == eSHOTTYPE_AURA )
				{
					mQuick[i].mSkillStep = step;
				}
				else
				{
					if( mQuick[i].mSkillStep + 1 == step )
					{
						mQuick[i].mSkillStep = step;
					}
				}

				netUpdate = true;
			}
			else
				assert(0);
		}
	}

	// 아이콘만 갱신
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		/// 몬스터 변신상태인경우,
		if( mIsLockByMonster )
		{
			if( i >= QUICKSLOT_ICON_MON_BEGIN && i <= QUICKSLOT_ICON_MON_END )
				continue;
		}

		unsigned int dataIndex = i;
		/// FS 슬롯
		if( i >= QUICKSLOT_ICON_FS_BEGIN )
			dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		if( mQuick[dataIndex].mType != eSHORTCUT_SKILL )
			continue;

		if( mQuick[dataIndex].mSkillIndex != skillIndex )
			continue;

		mBindIcon[i].Clear();

		// UI 갱신
		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
		if( info )
		{
			cIconParam* param = UIMAN->GetIconParam( info->mIconIdx );
			if( param )
			{
				/// 세팅
				mBindIcon[i].SetParam( param );
				mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
				mBindIcon[i].SetAniParam( mQuick[dataIndex].mAniParam );
				mBindIcon[i].SetTipType( mQuick[dataIndex].mTipType );
				mBindIcon[i].SetCount( mQuick[dataIndex].mSkillStep+1 );
				mBindIcon[i].ShowNumber( true );
			}
		}
	}

	if( netUpdate == true )
		NetUpdateData();

	// 쿨타임 로딩
	LoadCoolTime();
	LoadCoolTimeMon();
}

void cQuickManager::UpdateUI()
{
	// 몬스터 변신 상태에서 스킬을 배우더라도 퀵슬롯을 갱신하지 않는다.
	if( mIsLockByMonster )
		return;

	mTabIndexFS = ITEMMAN->GetActiveWeapon();

	/// 090216 PKH 
	bool netUpdate = false;

	/// 일반 퀵슬롯
	for( unsigned int i = QUICKSLOT_COMMON_BEGIN; i <= QUICKSLOT_COMMON_EXTRA2_END; ++i )
	{
		int iconId = QUICKSLOT_ICON_COMMON_BEGIN + i;
		if( UpdateIcon( i, iconId, true ) == true )
			netUpdate = true;
	}

	/// FS 퀵슬롯
	unsigned int slot = QUICKSLOT_FS1_BEGIN + (mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		int iconId = QUICKSLOT_ICON_FS_BEGIN + i;
		if( UpdateIcon( slot + i, iconId, true ) == true )
			netUpdate = true;
	}

	if( netUpdate == true )
		NetUpdateData();

	/// 쿨타임 로딩
	LoadCoolTime();
	LoadCoolTimeMon();
}

/// 퀵슬롯 생성후 OnShow 시 자동호출
/// 현재 탭인덱스를 기반으로 퀵슬롯 정보 업데이트 
bool cQuickManager::UpdateIcon( int i, int iconId, bool countCheck )
{
	if( i < QUICKSLOT_COMMON_BEGIN || i >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return false;
	}

	if( iconId < QUICKSLOT_ICON_COMMON_BEGIN || iconId >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return false;
	}

	/// 초기화
	/// 0 이되면 아이콘 안에서 아무것도 그리지 않으므로 갯수도 안그려진다.
	mBindIcon[iconId].Clear();

	/// 스킬 아이콘 세팅
	if( mQuick[i].mType == eSHORTCUT_SKILL )
	{
		sKeepSkill* p = SKILLMAN->GetKeepInfo( mQuick[i].mSkillIndex );
		if( p == NULL )
		{
			/// 090216 PKH 
			mQuick[i] = 0;
			return true;
		}
		
		sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( mQuick[i].mSkillIndex );
		if( info )
		{
			if( mQuick[i].mSkillStep >= info->mStepCount )
			{
				assert(0);
				mQuick[i] = 0;
				return true;
			}

			// 오라 타입이면
			if( info->mShotType == eSHOTTYPE_AURA )
			{
				mQuick[i].mSkillStep = p->mStep;
			}

			cIconParam* param = UIMAN->GetIconParam( info->mIconIdx );
			if( param )
			{
				/// 세팅
				mBindIcon[iconId].SetParam( param );
				mBindIcon[iconId].SetColor( mQuick[i].mColor );
				mBindIcon[iconId].SetAniParam( mQuick[i].mAniParam );
				mBindIcon[iconId].SetTipType( mQuick[i].mTipType );
				mBindIcon[iconId].SetCount( mQuick[i].mSkillStep+1 );
				mBindIcon[iconId].ShowNumber( true );
			}
		}
		else
		{
			assert(0);
		}
	}
	else if( mQuick[i].mType == eSHORTCUT_COMMU )
	{
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( mQuick[i].mSkillIndex );
		if( pInfo )
		{
			cIconParam* param = UIMAN->GetIconParam( pInfo->mIconIndex );
			if( param )
			{
				/// 세팅
				mBindIcon[iconId].ShowNumber( false );
				mBindIcon[iconId].SetParam( param );
				mBindIcon[iconId].SetColor( mQuick[i].mColor );
				mBindIcon[iconId].SetAniParam( mQuick[i].mAniParam );
				mBindIcon[iconId].SetTipType( mQuick[i].mTipType );
			}
		}
		else
		{
			assert(0);
		}
	}
	/// 아이템 아이콘 세팅 
	else if( mQuick[i].mType == eSHORTCUT_ITEM )
	{
		cItemDefine* itemDefine = ITEMMAN->GetItemDefine( mQuick[i].mItemIndex );
		if( itemDefine )
		{
			cIconParam* param = itemDefine->GetIconParam();
			if( param )
			{
				/// 세팅
				mBindIcon[iconId].SetParam( param );

				/// 갯수 검사를 해야할때만한다
				if( countCheck )
				{
					sItemCount* p = ITEMMAN->GetItemCount( mQuick[i].mItemIndex );
					unsigned int count = ( p ) ? p->bag : 0;
					if( count <= 0 )
					{
						mQuick[i].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
					}
					else
					{
						int ret0 = ITEMMAN->CheckLimit( mQuick[i].mItemIndex );
						int ret1 = ITEMMAN->CheckLimitEtc( mQuick[i].mItemIndex );

						NiColorA color = NiColorA::WHITE;
						if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
						{
							 color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
						}
						mQuick[i].mColor = color;
					}

					mBindIcon[iconId].SetCount( count );
					mBindIcon[iconId].ShowNumber( true );
				}
				/// 갯수 검사 체크안하면 
				else
				{
					int ret0 = ITEMMAN->CheckLimit( mQuick[i].mItemIndex );
					int ret1 = ITEMMAN->CheckLimitEtc( mQuick[i].mItemIndex );

					NiColorA color = NiColorA::WHITE;
					if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
					{
						color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
					}
					mQuick[i].mColor = color;
				}

				///
				mQuick[i].mTipType = (unsigned int)itemDefine->GetTipType();
				mBindIcon[iconId].SetAniParam( mQuick[i].mAniParam );
				mBindIcon[iconId].SetTipType( mQuick[i].mTipType );
				mBindIcon[iconId].SetColor( mQuick[i].mColor );
			}
		}
		else
		{
			assert(0);
		}
	}		

	return false;
}

/// 데이터삭제
void cQuickManager::DeleteData()
{
	if( mDragIcon )
	{
		cUIContainer* fromParent = mDragIcon->GetParent();
		if( !fromParent )
		{
			assert(0);
			return;
		}

		/// fs 슬롯에서 온건지 판별
		bool fromIsFS = false;
		if( fromParent->GetID() == eUIID_ROW_QUICKSLOT_FS || fromParent->GetID() == eUIID_COLUMN_QUICKSLOT_FS ) 
			fromIsFS = true;
		
		int dataIndex = mDragIcon->GetID();
		if( fromIsFS == true )
			dataIndex = mDragIcon->GetID() + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		DeleteData( mDragIcon->GetID(), dataIndex );

		NetUpdateData();
	}
}

///
void cQuickManager::DeleteData( unsigned int iconId, unsigned int dataIndex )
{
	if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
	{
		assert(0);
		return;
	}
	
	if( iconId < QUICKSLOT_ICON_COMMON_BEGIN || iconId >= QUICKSLOT_ICON_COUNT )
	{
		assert(0);
		return;
	}

	/// 데이터초기화
	mQuick[dataIndex] = 0;

	/// 아이콘 초기화
	mBindIcon[iconId].Clear();
}

/// 탭체인지시, 각 쿨타임 데이터 로드
void cQuickManager::LoadCoolTime()
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		/// 몬스터 변신상태인경우,
		if( mIsLockByMonster )
		{
			/// FS 슬롯인 경우 제외
			if( i >= QUICKSLOT_ICON_FS_BEGIN && i <= QUICKSLOT_ICON_FS_BEGIN + QUICKSLOT_COUNT_MON )
				continue;
		}

		unsigned int dataIndex = i;

		/// FS 슬롯인 경우 데이터 인덱스를 현재 선택된 FS 슬롯에 맞게 셋팅
		if( i >= QUICKSLOT_ICON_FS_BEGIN )
			dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		if( dataIndex < QUICKSLOT_COMMON_BEGIN || dataIndex >= QUICKSLOT_TOTAL )
		{
			assert(0);
			continue;
		}

		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
		{
			sKeepSkill* keepSkill = SKILLMAN->GetKeepInfo( mQuick[dataIndex].mSkillIndex );
			if( keepSkill && keepSkill->mRestTime > 0 )
			{
				unsigned long accumTime = THEAPP->GetWorldAccumTime();

				cIcon* colIcon = mBindIcon[i].mColumnIcon;
				colIcon->UpdateGauge( accumTime, keepSkill->mTotalTime - keepSkill->mRestTime, keepSkill->mTotalTime, true );

				cIcon* rowIcon = mBindIcon[i].mRowIcon;
				rowIcon->UpdateGauge( accumTime, keepSkill->mTotalTime - keepSkill->mRestTime, keepSkill->mTotalTime, true );
			}
		}
		else if( mQuick[dataIndex].mType == eSHORTCUT_ITEM )
		{
			sCoolTime* coolTime = COOLMAN->GetProcessData( mQuick[dataIndex].mItemIndex );
			if( coolTime )
			{
				cIcon* colIcon = mBindIcon[i].mColumnIcon;
				colIcon->UpdateGauge( coolTime->startTime, 0, coolTime->restTime, true );

				cIcon* rowIcon = mBindIcon[i].mRowIcon;
				rowIcon->UpdateGauge( coolTime->startTime, 0, coolTime->restTime, true );
			}
		}
	}
}

/// 몬스터 쿨타임 세팅
void cQuickManager::LoadCoolTimeMon()
{
	/// 몬스터 변신시
	if( !mIsLockByMonster )
		return;

	for( unsigned int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
	{
		unsigned int dataIndex = QUICKSLOT_MON_BEGIN + i;
		unsigned int iconId = QUICKSLOT_ICON_FS_BEGIN + i;

		if( mQuick[dataIndex].mType == eSHORTCUT_MONSKILL )
		{
			sKeepSkill* keepSkill = SKILLMAN->GetCoolTimeMon( mQuick[dataIndex].mSkillIndex );
			if( keepSkill && keepSkill->mRestTime > 0 )
			{
				unsigned long accumTime = THEAPP->GetWorldAccumTime();

				cIcon* colIcon = mBindIcon[iconId].mColumnIcon;
				colIcon->UpdateGauge( accumTime, keepSkill->mTotalTime - keepSkill->mRestTime, keepSkill->mTotalTime, true );

				cIcon* rowIcon = mBindIcon[iconId].mRowIcon;
				rowIcon->UpdateGauge( accumTime, keepSkill->mTotalTime - keepSkill->mRestTime, keepSkill->mTotalTime, true );
			}
		}
	}
}


/// 스킬 사용시 호출
void cQuickManager::StartSkillCoolTime( unsigned long skillIndex, unsigned long cool )
{
	if( skillIndex <= 0 )
	{
		assert(0);
		return;
	}

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		unsigned int dataIndex = i;

		/// FS 슬롯
		if( i >= QUICKSLOT_ICON_FS_BEGIN )
			dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
		{
			if( mQuick[dataIndex].mSkillIndex == skillIndex )
			{
				/// 아이콘 세팅
				mBindIcon[i].mColumnIcon->StartDelay( cool );
				mBindIcon[i].mRowIcon->StartDelay( cool );
			}
		}
	}

}

void cQuickManager::StartMonSkillCoolTime( short skillIdx, unsigned long cool )
{
	if( cool <= 0 )
		return;

	int i = skillIdx - eMONSTERATTACK_SKILL1;

	if( i >= 0 && i < QUICKSLOT_COUNT_MON )
	{
		mBindIcon[QUICKSLOT_ICON_FS_BEGIN+i].mColumnIcon->StartDelay( cool );
		mBindIcon[QUICKSLOT_ICON_FS_BEGIN+i].mRowIcon->StartDelay( cool );
	}
	else
	{
		assert(0);
	}
}


void cQuickManager::SetSkillAniAura( unsigned long skillIndex, sAniParam* param )
{
	for( unsigned int i = 0; i < QUICKSLOT_COUNT; ++i )
	{
		if( mQuick[i].mType != eSHORTCUT_SKILL )
			continue;

		/// 아이콘 애니메이션 세팅
		if( mQuick[i].mSkillIndex == skillIndex )
			mQuick[i].mAniParam = param;
	}

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		unsigned int dataIndex = i;

		/// FS 슬롯
		if( i >= QUICKSLOT_ICON_FS_BEGIN )
			dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
		{
			/// 아이콘 애니메이션 세팅
			if( mQuick[dataIndex].mSkillIndex == skillIndex )
				mBindIcon[i].SetAniParam( param );
		}
	}
}	

void cQuickManager::CheckAllIcon()
{
	/// HERO 가 착용한 무기 타입 
	eWEAPON_STATE wstate = HERO->GetWeaponState();

	/// 일반 퀵슬롯 체크
	for( unsigned int i = 0; i < QUICKSLOT_COUNT; ++i )
	{
		char type = mQuick[i].mType;

		if( type == eSHORTCUT_SKILL )
		{
			/// 스킬이면,
			if( UsableSkillIcon( wstate, mQuick[i].mSkillIndex, mQuick[i].mSkillStep) == true )
			{
				mQuick[i].mColor = NiColorA::WHITE;
			}
			else
			{
				mQuick[i].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
		}
		else if( type == eSHORTCUT_ITEM )
		{
			/// 아이템이면,
			sItemCount* p = ITEMMAN->GetItemCount( mQuick[i].mItemIndex );
			unsigned int count = ( p ) ? p->bag : 0;
			if( count <= 0 )
			{
				mQuick[i].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
			}
			else
			{
				int ret0 = ITEMMAN->CheckLimit( mQuick[i].mItemIndex );
				int ret1 = ITEMMAN->CheckLimitEtc( mQuick[i].mItemIndex );

				NiColorA color = NiColorA::WHITE;
				if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
				{
					color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
				mQuick[i].mColor = color;
			}
		}
	}

	/// 일반 
	for( int i = 0; i <= QUICKSLOT_ICON_COMMON_EXTRA2_END; ++i )
	{
		unsigned int dataIndex = i;
		char type = mQuick[dataIndex].mType;

		if( type == eSHORTCUT_SKILL )
		{
			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
		else if( type == eSHORTCUT_ITEM )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
			unsigned int count = ( p ) ? p->bag : 0;

			mBindIcon[i].SetCount( count );

			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
	}

	// FS
	for( int i = QUICKSLOT_ICON_FS_BEGIN; i <= QUICKSLOT_ICON_FS_END; ++i )
	{
		unsigned int dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
		char type = mQuick[dataIndex].mType;

		if( type == eSHORTCUT_SKILL )
		{
			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
		else if( type == eSHORTCUT_ITEM )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
			unsigned int count = ( p ) ? p->bag : 0;
			mBindIcon[i].SetCount( count );

			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
	}

	/// 몬스터 변신일경우 FS퀵슬롯 재세팅
	if( mIsLockByMonster == true )
	{
		for( int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].SetColor( mQuick[QUICKSLOT_MON_BEGIN+i].mColor );
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].SetCount( 0 );
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].ShowNumber( false );
		}
	}
}

void cQuickManager::CheckItemIcon()
{
	/// 일반 
	for( int i = 0; i <= QUICKSLOT_ICON_COMMON_EXTRA2_END; ++i )
	{
		unsigned int dataIndex = i;
		char type = mQuick[dataIndex].mType;

		if( type == eSHORTCUT_ITEM )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
			unsigned int count = ( p ) ? p->bag : 0;
			if( count <= 0 )
			{
				mQuick[dataIndex].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
			}
			else
			{
				int ret0 = ITEMMAN->CheckLimit( mQuick[dataIndex].mItemIndex );
				int ret1 = ITEMMAN->CheckLimitEtc( mQuick[dataIndex].mItemIndex );

				NiColorA color = NiColorA::WHITE;
				if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
				{
					color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
				mQuick[dataIndex].mColor = color;
			}

			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
	}

	// FS
	for( int i = QUICKSLOT_ICON_FS_BEGIN; i <= QUICKSLOT_ICON_FS_END; ++i )
	{
		unsigned int dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );
		char type = mQuick[dataIndex].mType;

		if( type == eSHORTCUT_ITEM )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mQuick[dataIndex].mItemIndex );
			unsigned int count = ( p ) ? p->bag : 0;
			if( count <= 0 )
			{
				mQuick[dataIndex].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
			}
			else
			{
				int ret0 = ITEMMAN->CheckLimit( mQuick[dataIndex].mItemIndex );
				int ret1 = ITEMMAN->CheckLimitEtc( mQuick[dataIndex].mItemIndex );

				NiColorA color = NiColorA::WHITE;
				if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
				{
					color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
				}
				mQuick[dataIndex].mColor = color;
			}

			mBindIcon[i].ShowNumber( true );
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
	}

	/// 몬스터 변신일경우 FS퀵슬롯 재세팅
	if( mIsLockByMonster == true )
	{
		for( int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].SetColor( mQuick[QUICKSLOT_MON_BEGIN+i].mColor );
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].SetCount( 0 );
			mBindIcon[ QUICKSLOT_ICON_FS_BEGIN+i ].ShowNumber( false );
		}

	}
}

void cQuickManager::CheckSkillIcon()
{
	/// HERO 가 착용한 무기 타입 
	eWEAPON_STATE wstate = HERO->GetWeaponState();

	sChargeInfo chargeInfo = SKILLEXECUTER->GetCurrentChargeSkill(); 

	/// 퀵슬롯 전체 검사
	for( unsigned int slot = 0; slot < QUICKSLOT_COUNT; ++slot )
	{
		if( mQuick[slot].mType != eSHORTCUT_SKILL )
			continue;

		if( UsableSkillIcon( wstate, mQuick[slot].mSkillIndex, mQuick[slot].mSkillStep ) == true )
		{
			if( chargeInfo.mSkillIdx == mQuick[slot].mSkillIndex && chargeInfo.mSkillStep == mQuick[slot].mSkillStep )
			{
				mQuick[slot].mColor = NiColorA( 0.78f, 0.77f, 0.12f, 1.0f );
			}
			else
				mQuick[slot].mColor = NiColorA::WHITE;
		}
		else
		{
			mQuick[slot].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
		}
	}

	/// 몬스터 변신상태인경우.
	if( mIsLockByMonster == true )
	{
		for( unsigned int slot = 0; slot < QUICKSLOT_COUNT_MON; ++slot )
		{
			unsigned int dataIndex = QUICKSLOT_MON_BEGIN + slot;
			if( mQuick[dataIndex].mSkillIndex == ULONG_MAX )
				continue;

			if( UsableMonSkillIcon( mQuick[dataIndex].mSkillIndex ) == true )
			{
				if( chargeInfo.mSkillIdx == mQuick[dataIndex].mSkillIndex )
				{
					mQuick[dataIndex].mColor = NiColorA( 0.78f, 0.77f, 0.12f, 1.0f );
				}
				else
					mQuick[dataIndex].mColor = NiColorA::WHITE;
			}
			else
			{
				mQuick[dataIndex].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
		}
	}

	/// 일반 슬롯 아이콘 갱신
	for( unsigned int i = 0; i <= QUICKSLOT_ICON_COMMON_EXTRA2_END; ++i )
	{
		unsigned int dataIndex = i;
		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
	}

	/// FS 슬롯 아이콘 갱신
	for( unsigned int i = QUICKSLOT_ICON_FS_BEGIN; i <= QUICKSLOT_ICON_FS_END; ++i )
	{
		unsigned int dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		/// 몬스터 변신인경우,
		if( mIsLockByMonster == true && i < QUICKSLOT_ICON_FS_BEGIN + QUICKSLOT_COUNT_MON )
			dataIndex = i + QUICKSLOT_MON_BEGIN - QUICKSLOT_ICON_FS_BEGIN;

		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL || 
			mQuick[dataIndex].mType == eSHORTCUT_MONSKILL )
		{
			mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}
	}
}

bool cQuickManager::UsableMonSkillIcon( unsigned long skillindex )
{
	if( HERO->IsCantSkill( skillindex ) == true )
		return false;

	unsigned long monIdx = HERO->GetTransMonsterClassIdx();
	sMonsterSkillScript* skill = SKILLSCRIPT->GetMonsterSkillInfo( monIdx, (eMONSTERATTACK_TYPE)skillindex );
	if( !skill )
	{
		assert(0);
		return false;
	}

	/// 소모 MP 량 검사
	unsigned long curMP = HERO->GetMP();
	unsigned long totalUseMP = HERO->CalcUseMPExt( skill->mUseMP );
	if( curMP < totalUseMP )
		return false;
	return true;
}

bool cQuickManager::UsableSkillIcon( eWEAPON_STATE heroWeaponState, unsigned long skillindex, unsigned char skillstep )
{
	///
	if( HERO->IsTransformMonster() )
		return false;

	if( HERO->IsCantSkill( skillindex ) == true )
		return false;

	/// 동일한 스킬을 모두 세팅.
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillindex );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	if( skillstep >= pInfo->mStepCount )
	{
		assert(0);
		return false;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[skillstep];

	/// use item check
	if( pInfo->mUseItem )
	{
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
			return false;
	}

	/// 사용 상태가 맞는지 체크
	if( pInfo->mUseState )
	{
		switch( pInfo->mUseState )
		{
		case eSKILL_USE_STATE_HP_PER_UP: // hp up
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_HP_PER_DOWN: // hp down
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_UP: // mp up
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_DOWN: // mp down
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_BUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_DEBUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		default:
			{
				assert(0);
				return false;
			}
		}
	}

	/// 현재 HP량
	unsigned long curHp = HERO->GetHP();
	if( curHp < pStepInfo->mUseHP )
		return false;

	/// 현재 MP량
	unsigned long curMP = HERO->GetMP();
	unsigned long totalUseMP = HERO->CalcUseMPExt( pStepInfo->mUseMP );
	if( curMP <  totalUseMP )
		return false;
	if( pStepInfo->mUseMPPer > 0 )
	{
		unsigned long currentUseMP = HERO->GetOriMpMax();
		currentUseMP = (unsigned long)((currentUseMP * pStepInfo->mUseMPPer) / 100);
		totalUseMP = HERO->CalcUseMPExt( currentUseMP );
		if( curMP < totalUseMP )
			return false;
	}

	if( pInfo->mUseEquipment )
	{
		/// 착용 장비 체크
		switch( pInfo->mUseEquipment )
		{
		case eEQUIPTYPE_SWORD:
			if( heroWeaponState != eWEAPON_STATE_SWORD &&
				heroWeaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD:
			if( heroWeaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_DOUBLESWORD:
			if( heroWeaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD:
			if( heroWeaponState != eWEAPON_STATE_SHORTSWORD )
				return false;
			break;
		case eEQUIPTYPE_GUN:
			if( heroWeaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		case eEQUIPTYPE_STAFF:
			if( heroWeaponState != eWEAPON_STATE_STAFF )
				return false;
			break;
		case eEQUIPTYPE_SHIELD:
			if( heroWeaponState != eWEAPON_STATE_SHEILD &&
				heroWeaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD:
			if( heroWeaponState != eWEAPON_STATE_SWORD &&
				heroWeaponState != eWEAPON_STATE_SWORD_SHEILD &&
				heroWeaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_DOUBLESWORD:
			if( heroWeaponState != eWEAPON_STATE_SWORD &&
				heroWeaponState != eWEAPON_STATE_SWORD_SHEILD &&
				heroWeaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
			if( heroWeaponState != eWEAPON_STATE_LONGSWORD &&
				heroWeaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
			if( heroWeaponState != eWEAPON_STATE_SWORD &&
				heroWeaponState != eWEAPON_STATE_SWORD_SHEILD &&
				heroWeaponState != eWEAPON_STATE_LONGSWORD &&
				heroWeaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD_GUN:
			if( heroWeaponState != eWEAPON_STATE_SHORTSWORD &&
				heroWeaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		}
	}

	if( pInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
	{
		if( PVPMAN->IsEnableBattle() )
			return false;
	}
	return true;
}

void cQuickManager::SetCharge( unsigned long skillIndex, bool charge )
{
	if( HERO->IsTransformMonster() == true )
	{
		assert(0);
		return;
	}

	eWEAPON_STATE wstate = HERO->GetWeaponState();

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIndex );
	if( pInfo )
	{
		/// 사용 가능 무기가 아니면 리턴
		if( wstate != pInfo->mUseEquipment )
			return;
	}

	/// 퀵슬롯 전체 검사
	bool update = false;
	for( unsigned int slot = 0; slot < QUICKSLOT_COUNT; ++slot )
	{
		if( mQuick[slot].mType != eSHORTCUT_SKILL )
			continue;

		/// 해당스킬이면 챠지
		if( mQuick[slot].mSkillIndex == skillIndex )
		{
			update = true;

			if( charge == true )
				mQuick[slot].mColor = NiColorA( 0.78f, 0.77f, 0.12f, 1.0f );
			else
			{
				/// charge가 풀릴때 상태검사
				eWEAPON_STATE wstate = HERO->GetWeaponState();
				if( UsableSkillIcon( wstate, mQuick[slot].mSkillIndex, mQuick[slot].mSkillStep ) == true )
					mQuick[slot].mColor =  NiColorA::WHITE;
				else
					mQuick[slot].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
		}
	}

	if( update == true )
	{
		for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
		{
			unsigned int dataIndex = i;

			/// FS 슬롯 
			if( i >= QUICKSLOT_ICON_FS_BEGIN )
				dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

			if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
				mBindIcon[i].SetColor( mQuick[dataIndex].mColor );
		}

	}
}

/// 몬스터 스킬 챠지
void cQuickManager::SetMonCharge( unsigned long skillIndex, bool charge )
{
	for( unsigned int slot = 0; slot < QUICKSLOT_COUNT_MON; ++slot )
	{
		int dataIndex = QUICKSLOT_MON_BEGIN + slot;
		if( mQuick[dataIndex].mSkillIndex == ULONG_MAX )
			continue;

		if( mQuick[dataIndex].mSkillIndex == skillIndex )
		{
			if( charge == true )
				mQuick[dataIndex].mColor = NiColorA( 0.78f, 0.77f, 0.12f, 1.0f );
			else
			{
				if( UsableMonSkillIcon( skillIndex ) == true )
					mQuick[dataIndex].mColor = NiColorA::WHITE;
				else
					mQuick[dataIndex].mColor = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}

			mBindIcon[QUICKSLOT_ICON_FS_BEGIN + slot].SetColor( mQuick[dataIndex].mColor );
		}
	}
}

void cQuickManager::ChangeMonster( unsigned long monIndex )
{
	/// 몬스터 변신
	if( monIndex > 0 )
	{	
		mIsLockByMonster = true;

		/// 세팅
		for( int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			unsigned int dataIndex = QUICKSLOT_MON_BEGIN + i;
			unsigned int iconId = QUICKSLOT_ICON_FS_BEGIN + i;

			/// 초기화
			mQuick[dataIndex] = 0;
			mBindIcon[iconId].Clear();

			sMonsterSkillScript* skill = SKILLSCRIPT->GetMonsterSkillInfo( monIndex, (eMONSTERATTACK_TYPE)(eMONSTERATTACK_SKILL1 + i) );
			if( skill )
			{
				mQuick[dataIndex].mType = eSHORTCUT_MONSKILL;
				mQuick[dataIndex].mSkillIndex = eMONSTERATTACK_SKILL1 + i;
				mQuick[dataIndex].mColor = NiColorA::WHITE;
			}
			else
			{
				mQuick[dataIndex].mType = eSHORTCUT_MONSKILL;
				mQuick[dataIndex].mSkillIndex = ULONG_MAX;
				mQuick[dataIndex].mColor = NiColorA( 0.5f, 0.5f, 0.5f, 1.0f );
			}

			/// 퀵슬롯 아이콘 세팅
			mBindIcon[iconId].ShowNumber( false );
			mBindIcon[iconId].SetColor( mQuick[dataIndex].mColor );
			mBindIcon[iconId].SetParam( UIMAN->GetIconParam(MONICON_PARAMINDEX + i) );
			mBindIcon[iconId].SetColor( mQuick[dataIndex].mColor );
			mBindIcon[iconId].SetTipType( mQuick[dataIndex].mTipType );
		}
	}
	/// 변신 해제
	else
	{
		mIsLockByMonster = false; 

		/// FS 퀵슬롯 재세팅 - 앞에서부터 3개만 리셋
		unsigned int slot = (mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH) + QUICKSLOT_FS1_BEGIN;
		for ( unsigned int i = 0; i < QUICKSLOT_COUNT_MON; ++i )
		{
			UpdateIcon( slot + i , QUICKSLOT_ICON_FS_BEGIN + i );
		}

		LoadCoolTime();
		LoadCoolTimeMon();
	}
}

/// 스킬 사용시 호출
void cQuickManager::ClearCoolTime( unsigned long exceptSkillIdx )
{
	if( exceptSkillIdx <= 0 )
	{
		assert(0);
		return;
	}

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT; ++i )
	{
		unsigned int dataIndex = i;

		/// FS 슬롯
		if( i >= QUICKSLOT_ICON_FS_BEGIN )
			dataIndex = i + ( mTabIndexFS * QUICKSLOT_ICON_COUNT_FOR_EACH );

		if( mQuick[dataIndex].mType == eSHORTCUT_SKILL )
		{
			if( mQuick[dataIndex].mSkillIndex > 0 && mQuick[dataIndex].mSkillIndex != exceptSkillIdx )
			{
				/// 아이콘 세팅
				if( mBindIcon[i].mColumnIcon && mBindIcon[i].mColumnIcon->IsActive() )
					mBindIcon[i].mColumnIcon->ClearCooltime();

				if( mBindIcon[i].mRowIcon && mBindIcon[i].mRowIcon->IsActive() )
					mBindIcon[i].mRowIcon->ClearCooltime();
			}
		}
	}
}
