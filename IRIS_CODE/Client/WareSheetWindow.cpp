#include "stdafx.h"
#include "WareSheetWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "DragWindow.h"
#include "ItemIcon.h"


cWareSheetWindow::cWareSheetWindow( eUINodeType type )
: cUIWindow( type )
, mTabIndex( 0 )
{
	for( int i = 0; i < WAREHOUSE_SLOT_COUNT; ++i )
		mIcon[i] = 0;
}

cWareSheetWindow::~cWareSheetWindow()
{
}

void cWareSheetWindow::InitWindow( unsigned int tabIndex )
{
	mTabIndex = tabIndex;

	/// 아이콘을 생성
	cString str;
	unsigned int slotIndex = 0;

	switch( mTabIndex )
	{
	case 0:
		slotIndex = INVENTORY_WAREHOUSE0_BEGIN;
		break;
// 	case 1:
// 		slotIndex = INVENTORY_WAREHOUSE_MALL0_BEGIN;
// 		break;
// 	case 2:
// 		slotIndex = INVENTORY_WAREHOUSE1_BEGIN;
// 		break;
	default:
		assert( 0 && "tab index out of range" );
		return;
	}

	for( unsigned int i = 0; i < WAREHOUSE_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "WareSheet_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( (unsigned short)slotIndex, icon );
	}
}

bool cWareSheetWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;
	return true;
}

void cWareSheetWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cWareSheetWindow::OnIconClicked( cUINode* caller, unsigned int id )
{
	/// 창고윈도우에 아이콘 눌림 이벤트
	if( caller )
		SendEvent( mpParent, eUIEVENT_ICON_CLICKED, caller, id );
}

void cWareSheetWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}

	unsigned short curSlotIdx = icon->GetSlotIndex();
	unsigned short slotIdx = 0;
	if( ITEMMAN->FindEmptySlot( &slotIdx, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == true )
	{
		ITEMMAN->TryToMoveItem( curSlotIdx, slotIdx, icon );
	}
	else
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 202 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
	}
}
