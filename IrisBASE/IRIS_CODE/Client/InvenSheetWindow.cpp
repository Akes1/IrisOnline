#include "stdafx.h"
#include "InvenSheetWindow.h"
#include "InventoryWindow.h"
#include "DragWindow.h"
#include "ItemIcon.h"
#include "ObjectManager.h"
#include "hero.h"

cInvenSheetWindow::cInvenSheetWindow( eUINodeType type )
: cUIWindow( type )
, mTabIndex( 0 )
{
	for( int i = 0; i < INVENSHEET_SLOT_COUNT; ++i )
		mIcon[i] = 0;
}

cInvenSheetWindow::~cInvenSheetWindow()
{
}

bool cInvenSheetWindow::BindSlot( unsigned int tabIndex )
{
	mTabIndex = tabIndex;

	/// 아이콘을 생성
	cString str;
	unsigned short slotIndex = 0;

	switch( mTabIndex )
	{
	case 0:
		slotIndex = INVENTORY_BAG0_BEGIN;
		break;
	case 1:
		slotIndex = INVENTORY_QUEST_BEGIN;
		break;
	case 2:
	//	slotIndex = INVENTORY_MALL_BEGIN;
		break;
	default:
		assert( 0 && "index out of range" );
		return false;
	}

	for( unsigned short i = 0; i < INVENSHEET_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "InvenSheet_Icon%02d", i );
 
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( slotIndex, icon );
	}
	return true;
}

void cInvenSheetWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cInvenSheetWindow::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, caller, id );
}

void cInvenSheetWindow::OnIconClicked( cUINode* caller, unsigned int id )
{
	SendEvent( mpParent, eUIEVENT_ICON_CLICKED, caller, id );
}

void cInvenSheetWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	SendEvent( mpParent, eUIEVENT_ICON_RIGHTCLICKED, caller, id, shift );
}
