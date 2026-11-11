#include "StdAfx.h"
#include "RowQuickSlot.h"
#include "DragWindow.h"
#include "RadioButton.h"
#include "GameUIManager.h"
#include "SkillTipWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "CommunityScript.h"
#include "SkillScript.h"
#include "GameResourceManager.h"
#include "ShortcutManager.h"

cRowQuickSlot::cRowQuickSlot( eQuickSlotType type )
:mType( type )
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mpKeyImage[i] = 0;

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mIcon[i] = 0;
}

cRowQuickSlot::~cRowQuickSlot()
{
	for( unsigned int i = 0; i < 10; ++i )
		SAFE_DELETE( mpKeyImage[i] );
}

void cRowQuickSlot::Open()
{
	if( mType == QUICKTYPE_COMMON )
		cUIWindow::Open();

}

/// 주의사항 : 
bool cRowQuickSlot::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cString str;
	int     iconIndex = 0;
	/// 아이콘 생성
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		switch( mType )
		{
		case QUICKTYPE_COMMON:		  str.Format( "QuickRow_Icon%02d",  i ); iconIndex = QUICKSLOT_ICON_COMMON_BEGIN;		 break;
		case QUICKTYPE_COMMON_EXTRA1: str.Format( "ExtraRow1_Icon%02d", i ); iconIndex = QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN; break;
		case QUICKTYPE_COMMON_EXTRA2: str.Format( "ExtraRow2_Icon%02d", i ); iconIndex = QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN; break;
		default : assert(0); return false;
		}

		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, iconIndex + i ) == false )
			return false;

		mIcon[i] = icon;
		QUICKMAN->BindRowIcon( iconIndex + i, icon );
	}

	/// 얼굴 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cString skinName;
	typedef tPointerHashMap< int, void* > sKeyMap;
	sKeyMap *keyboardMap;
	keyboardMap = SHORTCUTMAN->GetKeyboardScriptMap();
	if( keyboardMap )
	{
		sKeyMap::cIterator pos = keyboardMap->Begin();
		sKeyMap::cIterator end = keyboardMap->End();
		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;

			skinName.Format( "Key_Skin_%04d", index );
			cUINodeSkin* pNodeSkin = pSkin->GetNodeSkin( skinName.Cstr() );
			if( pNodeSkin == NULL )
			{
				assert( 0 && "Bad skin name" );
				return false;
			}
			mKeySkinMap.Insert( index, pNodeSkin );
		}
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

void cRowQuickSlot::UpdateRect()
{
	cUIWindow::UpdateRect();

	for( unsigned int i = 0; i < 10; ++i )
	{
		if( mpKeyImage[i] )
		{
			unsigned int x = mIcon[i]->GetAbsoluteRect().mLeft + 1;
			unsigned int y = mIcon[i]->GetAbsoluteRect().mTop + 1;

			mpKeyImage[i]->SetPos( x, y );
		}
	}
}

void cRowQuickSlot::OnRender(cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	for( unsigned int i = 0; i < 10; ++i )
	{
		if( mpKeyImage[i])
			mpKeyImage[i]->Draw();
	}
}

void cRowQuickSlot::OnCommand( cUINode*, unsigned int id )
{
	switch(id)
	{ 
	case eUIID_ROW_ROTATE:
		{	
			Hide();

			///
			cUIWindow* colQuick = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_QUICKSLOT );
			if( colQuick )
			{
				colQuick->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON );
			}
		}
		break;
	case eUIID_ROW_EXTRASLOT1_ROTATE:
		{	
			Hide();

			///
			cUIWindow* colExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_EXTRASLOT1 );
			if( colExtra1 )
			{
				colExtra1->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON_EXTRA1 );
			}
			else
				assert(0);
		}
		break;
	case eUIID_ROW_EXTRASLOT2_ROTATE:
		{	
			Hide();

			///
			cUIWindow* colExtra2 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_EXTRASLOT2 );
			if( colExtra2 )
			{
				colExtra2->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON_EXTRA2 );
			}
			else
				assert(0);
		}
		break;
	case eUIID_ROW_EXTRA:
		{
			/// 채팅창 더 보이기
			cUIWindow* rowExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_EXTRASLOT1 );
			cUIWindow* colExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_EXTRASLOT1 );
			if( colExtra1 && rowExtra1 )
			{
				if( colExtra1->IsVisible() == false && rowExtra1->IsVisible() == false )
				{
					rowExtra1->Show();
					QUICKMAN->SetRowQuick( true, QUICKTYPE_COMMON_EXTRA1 );
					return;
				}
			}
			else
				assert(0);

			cUIWindow* rowExtra2 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_EXTRASLOT2 );
			cUIWindow* colExtra2 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_EXTRASLOT2 );
			if( rowExtra2 && colExtra2 )
			{
				if( colExtra2->IsVisible() == false && rowExtra2->IsVisible() == false )
				{
					rowExtra2->Show();
					QUICKMAN->SetRowQuick( true, QUICKTYPE_COMMON_EXTRA2 );
					return;
				}
			}
			else
				assert(0);

		}
		break;
	case eUIID_ROW_EXTRASLOT1_CLOSE:
	case eUIID_ROW_EXTRASLOT2_CLOSE:
		{
			///현재 창 닫기
			Hide();
		}
		break;
	default: 
		assert(0);
		break;
	}

}

void cRowQuickSlot::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
{
	if( id < QUICKSLOT_COMMON_BEGIN || id > QUICKSLOT_COMMON_EXTRA2_END )
	{
		assert(0 && "error id");
		return;
	}

	cIcon* icon = (cIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}

	eSHORTCUT_TYPE type = QUICKMAN->GetQuickType( id );
	if( type == eSHORTCUT_ITEM )
	{
		/// 아이템
		unsigned int itemIndex = QUICKMAN->GetItemIndex( id );
		UIMAN->ShowItemTip( eTIP_QUICK, pos, itemIndex, USHRT_MAX, false );
	}
	else if( type == eSHORTCUT_SKILL )
	{
		/// 스킬
		unsigned int skillIndex = QUICKMAN->GetSkillIndex( id );
		unsigned char skillStep = QUICKMAN->GetSkillStep( id );

		/// 스킬 팁 
		UIMAN->ShowSkillTip( eTIP_QUICK, GAMEUI->GetSkillTip(), pos, skillIndex, skillStep );
	}
	else if( type == eSHORTCUT_COMMU )
	{
		unsigned int index = QUICKMAN->GetSkillIndex( id );
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( index );
		if( pInfo )
		{
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, UIMAN->GetUIText( pInfo->mTooltipIndex) );
		}
	}
}

void cRowQuickSlot::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
	{
		bool IsUse = QUICKMAN->IsUseSlot( id );

		/// 데이터가 있는 슬롯만 드래그 가능함.
		if( IsUse == true )
			UIMAN->ShowDrag( this, DRAG_FROM_QUICKSLOT, icon, pos, shift );

	}
}

/// 퀵슬롯 사용
void cRowQuickSlot::OnIconRightClicked( cUINode*, unsigned int id, bool )
{
	QUICKMAN->OnMouseDown( id );
}

/// 퀵슬롯 사용
void cRowQuickSlot::OnIconDoubleClicked( cUINode*, unsigned int id )
{
	QUICKMAN->OnMouseDown( id );
}

void cRowQuickSlot::ChangeShortcut( unsigned int slotIndex, int keyboardIndex )
{
	if( !( 0 <= slotIndex && slotIndex < QUICKSLOT_COUNT ) )
	{
		assert( 0 && "Bad QuickSlot index" );
		return;
	}

	/// 해당 퀵 슬롯에 아무런 버튼도 할당 되어 있지 않은 경우 이미지 안보이기.
	if( keyboardIndex == -1 )
	{
		if( mpKeyImage[slotIndex] )
			mpKeyImage[slotIndex]->SetScreenRect( cUIRect( 0, 0, 0, 0 ) );
		return;
	}

	cUINodeSkin* pNodeSkin = (cUINodeSkin*)mKeySkinMap.GetAt( keyboardIndex );

	if( !pNodeSkin || !mIcon[slotIndex] )
	{
		assert(0);
		return;
	}

	if( mpKeyImage[slotIndex] == NULL )
		mpKeyImage[slotIndex] = new cUIImage( pNodeSkin->mpTexture );

	/// 이미지 셋팅
	unsigned int x = mIcon[slotIndex]->GetAbsoluteRect().mLeft + 1;
	unsigned int y = mIcon[slotIndex]->GetAbsoluteRect().mTop + 1;
	unsigned int tx = pNodeSkin->mSkinInfo->mTexX;
	unsigned int ty = pNodeSkin->mSkinInfo->mTexY;
	unsigned int w =  pNodeSkin->mSkinInfo->mWidth;
	unsigned int h =  pNodeSkin->mSkinInfo->mHeight;

	mpKeyImage[slotIndex]->SetTextureRect( tx, ty, tx + w, ty + h );
	mpKeyImage[slotIndex]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	
}

//////////////////////////////////////////////////////////////////////// 
/// FS 퀵슬롯
cFSRowQuickSlot::cFSRowQuickSlot()
: cRowQuickSlot( QUICKTYPE_FS )
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mIcon[i] = 0;
}

cFSRowQuickSlot::~cFSRowQuickSlot()
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		SAFE_DELETE( mpKeyImage[i] );
}

bool cFSRowQuickSlot::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cString str;
	/// 아이콘 생성
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		str.Format( "QuickFSRow_Icon%02d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, QUICKSLOT_ICON_FS_BEGIN + i ) == false )
			return false;

		mIcon[i] = icon;
		QUICKMAN->BindFSRowIcon( QUICKSLOT_ICON_FS_BEGIN + i, icon );
	}

	/// 얼굴 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cString skinName;
	typedef tPointerHashMap< int, void* > sKeyMap;
	sKeyMap *keyboardMap;
	keyboardMap = SHORTCUTMAN->GetKeyboardScriptMap();
	if( keyboardMap )
	{
		sKeyMap::cIterator pos = keyboardMap->Begin();
		sKeyMap::cIterator end = keyboardMap->End();
		for( ; pos!= end; ++pos )
		{
			int index = (*pos).mFirst;

			skinName.Format( "Key_Skin_%04d", index );
			cUINodeSkin* pNodeSkin = pSkin->GetNodeSkin( skinName.Cstr() );
			if( pNodeSkin == NULL )
			{
				assert( 0 && "Bad skin name" );
				return false;
			}
			mKeySkinMap.Insert( index, pNodeSkin );
		}
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

void cFSRowQuickSlot::OnCommand( cUINode* caller, unsigned int id )
{
	if( id == eUIID_ROW_ROTATE_FS )
	{	
		Hide();

		///
		cUIWindow* colQuick = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_QUICKSLOT_FS );
		if( colQuick )
		{
			colQuick->Show();
			QUICKMAN->SetRowQuickFS( false );
		}
	}
}

void cFSRowQuickSlot::OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos )
{
	if( id < QUICKSLOT_ICON_FS_BEGIN || id > QUICKSLOT_ICON_FS_END )
	{
		assert(0 && "error id");
		return;
	}

	cIcon* icon = (cIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}

	eSHORTCUT_TYPE type = QUICKMAN->GetQuickType( id, true );
	if( type == eSHORTCUT_ITEM )
	{
		/// 아이템
		unsigned int itemIndex = QUICKMAN->GetItemIndex( id, true );
		UIMAN->ShowItemTip( eTIP_QUICK, pos, itemIndex, USHRT_MAX, false );
	}
	else if( type == eSHORTCUT_SKILL )
	{
		/// 스킬
		unsigned int skillIndex = QUICKMAN->GetSkillIndex( id, true );
		unsigned char skillStep = QUICKMAN->GetSkillStep( id, true );

		UIMAN->ShowSkillTip( eTIP_QUICK, GAMEUI->GetSkillTip(), pos, skillIndex, skillStep );
	}
	else if( type == eSHORTCUT_MONSKILL )
	{
		/// 몬스터 변신상태
		short skillIndex = QUICKMAN->GetMonSkillIndex( id );
		if( skillIndex == -1 || skillIndex >= eMONSTERATTACK_MAX )
			return;

		unsigned long monIdx = HERO->GetTransMonsterClassIdx();
		sMonsterSkillScript* skill = SKILLSCRIPT->GetMonsterSkillInfo( monIdx, (eMONSTERATTACK_TYPE)skillIndex );
		if( !skill )
		{
			assert(0);
			return;
		}

		// 재사용시간
		cStringT timeStr;
		unsigned int restTime = skill->mCoolTime /1000;
		if( restTime == 0 )
		{
			assert(0);
			timeStr.Format( GAMERESOURCEMAN->GetGameText( 72 ), 0 );
		}
		else
		{
			unsigned int d = (restTime / 60) / 60 / 24;
			unsigned int h = (restTime / 60) / 60 % 24;
			unsigned int m = (restTime / 60) % 60;
			unsigned int s = (restTime % 60) % 60;

			/// 반올림해서 출력
			if( d != 0 )
				d++;
			if( h != 0 )
				h++;
			if( m != 0 )
				m++;

			if( d != 0 )
				timeStr.Format( GAMERESOURCEMAN->GetGameText( 74 ), d );
			else if( h != 0 )
				timeStr.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );
			else if( m != 0 )
				timeStr.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
			else
				timeStr.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );
		}

		// 소모비용 & 재사용시간
		unsigned long totalUseMP = HERO->CalcUseMPExt( skill->mUseMP );
		cStringT tooltip;
		tooltip.Format( GAMERESOURCEMAN->GetSkillToolTipText(skill->mToolTipIdx), (LPCTSTR)timeStr.Cstr(), totalUseMP );
		UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, (LPCTSTR)tooltip.Cstr() );
	}
	else if( type == eSHORTCUT_COMMU )
	{
		unsigned int index = QUICKMAN->GetSkillIndex( id, true );
		sCommunitySkillInfo* pInfo = COMMUNITYSCRIPT->GetCommunityInfo( index );
		if( pInfo )
		{
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, UIMAN->GetUIText( pInfo->mTooltipIndex) );
		}
	}
}

void cFSRowQuickSlot::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
	{
		bool IsUse = QUICKMAN->IsUseSlot( id, true );

		/// 데이터가 있는 슬롯만 드래그 가능함.
		if( IsUse == true )
			UIMAN->ShowDrag( this, DRAG_FROM_QUICKSLOT, icon, pos, shift );
	}
}

void cFSRowQuickSlot::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	QUICKMAN->OnMouseDown( id, true );
}

void cFSRowQuickSlot::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	QUICKMAN->OnMouseDown( id, true );
}


