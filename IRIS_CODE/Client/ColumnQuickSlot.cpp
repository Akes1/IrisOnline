#include "StdAfx.h"
#include "ColumnQuickSlot.h"
#include "QuickManager.h"
#include "DragWindow.h"
#include "RadioButton.h"
#include "GameUIManager.h"
#include "SkillTipWindow.h"
#include "objectmanager.h"
#include "hero.h"
#include "CommunityScript.h"
#include "SkillScript.h"
#include "GameResourceManager.h"
#include "ShortcutManager.h"

cColumnQuickSlot::cColumnQuickSlot( eQuickSlotType type )
:mType( type )
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mpKeyImage[i] = 0;

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mIcon[i] = 0;
}

cColumnQuickSlot::~cColumnQuickSlot()
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		SAFE_DELETE( mpKeyImage[i] );
}

void cColumnQuickSlot::Open()
{
	if( mType == QUICKTYPE_COMMON )
		cUIWindow::Open();
}

/// 주의사항 : 
bool cColumnQuickSlot::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cString str;
	int		iconIndex = 0;
	/// 아이콘 생성
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		switch( mType )
		{
		case QUICKTYPE_COMMON:		  str.Format( "QuickColumn_Icon%02d", i  ); iconIndex = QUICKSLOT_ICON_COMMON_BEGIN;	    break;
		case QUICKTYPE_COMMON_EXTRA1: str.Format( "ExtraColumn1_Icon%02d", i ); iconIndex = QUICKSLOT_ICON_COMMON_EXTRA1_BEGIN; break;
		case QUICKTYPE_COMMON_EXTRA2: str.Format( "ExtraColumn2_Icon%02d", i ); iconIndex = QUICKSLOT_ICON_COMMON_EXTRA2_BEGIN; break;
		}
	
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, iconIndex + i ) == false )
			return false;

		mIcon[i] = icon;
		QUICKMAN->BindColumnIcon( iconIndex + i, icon );
	}

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

void cColumnQuickSlot::UpdateRect()
{
	cUIWindow::UpdateRect();

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		if( mpKeyImage[i] )
		{
			unsigned int x = mIcon[i]->GetAbsoluteRect().mLeft + 1;
			unsigned int y = mIcon[i]->GetAbsoluteRect().mTop + 1;
			
			mpKeyImage[i]->SetPos( x, y );
		}
	}
}

void cColumnQuickSlot::OnRender(cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{
		if( mpKeyImage[i])
			mpKeyImage[i]->Draw();
	}
}

void cColumnQuickSlot::OnCommand( cUINode*, unsigned int id )
{
	switch(id)
	{ 
	case eUIID_COLUMN_ROTATE:
		{	
			Hide();

			///
			cUIWindow* colQuick = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_QUICKSLOT );
			if( colQuick )
			{
				colQuick->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON );
			}
		}
		break;
	case eUIID_COLUMN_EXTRASLOT1_ROTATE:
		{	
			Hide();

			///
			cUIWindow* rowExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_EXTRASLOT1 );
			if( rowExtra1 )
			{
				rowExtra1->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON_EXTRA1 );
			}
			else
				assert(0);
		}
		break;
	case eUIID_COLUMN_EXTRASLOT2_ROTATE:
		{	
			Hide();

			///
			cUIWindow* rowExtra2 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_EXTRASLOT2 );
			if( rowExtra2 )
			{
				rowExtra2->Show();
				QUICKMAN->SetRowQuick( false, QUICKTYPE_COMMON_EXTRA2 );
			}
			else
				assert(0);
		}
		break;
	case eUIID_COLUMN_EXTRA:
		{
			/// 채팅창 더 보이기
			cUIWindow* rowExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_EXTRASLOT1 );
			cUIWindow* colExtra1 = (cUIWindow*)UIMAN->FindNodeByID( eUIID_COLUMN_EXTRASLOT1 );
			if( colExtra1 && rowExtra1 )
			{
				if( colExtra1->IsVisible() == false && rowExtra1->IsVisible() == false )
				{
					colExtra1->Show();
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
					colExtra2->Show();
					QUICKMAN->SetRowQuick( true, QUICKTYPE_COMMON_EXTRA2 );
					return;
				}
			}
			else
				assert(0);
		}
		break;
	case eUIID_COLUMN_EXTRASLOT1_CLOSE:
	case eUIID_COLUMN_EXTRASLOT2_CLOSE:
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

void cColumnQuickSlot::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
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

void cColumnQuickSlot::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
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
void cColumnQuickSlot::OnIconRightClicked( cUINode*, unsigned int id, bool )
{
	QUICKMAN->OnMouseDown( id );
}

/// 퀵슬롯 사용
void cColumnQuickSlot::OnIconDoubleClicked( cUINode*, unsigned int id )
{
	QUICKMAN->OnMouseDown( id );
}

void cColumnQuickSlot::ChangeShortcut( unsigned int slotIndex, int keyboardIndex )
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
cFSColumnQuickSlot::cFSColumnQuickSlot()
: cColumnQuickSlot( QUICKTYPE_FS )
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		mIcon[i] = 0;
}

cFSColumnQuickSlot::~cFSColumnQuickSlot()
{
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
		SAFE_DELETE( mpKeyImage[i] );
}

bool cFSColumnQuickSlot::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cString str;
	/// 아이콘 생성
	for( unsigned int i = 0; i < QUICKSLOT_ICON_COUNT_FOR_EACH; ++i )
	{	
		str.Format( "QuickFSColumn_Icon%02d", i );
		cIcon* icon = new cIcon;
		if( icon->CreateBySkinName( str, this, QUICKSLOT_ICON_FS_BEGIN + i ) == false )
			return false;

		mIcon[i] = icon;
		QUICKMAN->BindFSColumnIcon( QUICKSLOT_ICON_FS_BEGIN + i, icon );
	}

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

void cFSColumnQuickSlot::OnCommand( cUINode* caller, unsigned int id )
{
	if( id == eUIID_COLUMN_ROTATE_FS )
	{	
		Hide();

		cUIWindow* rowQuick = (cUIWindow*)UIMAN->FindNodeByID( eUIID_ROW_QUICKSLOT_FS );
		if( rowQuick )
		{
			rowQuick->Show();
			QUICKMAN->SetRowQuickFS( true );
		}
	}
}

void cFSColumnQuickSlot::OnIconHovered(cUINode* caller, unsigned int id, const cUIPos& pos )
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
		unsigned long itemIndex = QUICKMAN->GetItemIndex( id, true );
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

void cFSColumnQuickSlot::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
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

void cFSColumnQuickSlot::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	QUICKMAN->OnMouseDown( id, true );
}

void cFSColumnQuickSlot::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	QUICKMAN->OnMouseDown( id, true );
}
