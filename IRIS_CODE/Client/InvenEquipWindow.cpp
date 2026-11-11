#include "stdafx.h"
#include "InvenEquipWindow.h"
#include "DragWindow.h"
#include "ItemIcon.h"
#include "ObjectManager.h"
#include "Hero.h"

cInvenEquipWindow::cInvenEquipWindow()
{
	for( int i = 0; i < EQUIP_SLOT_COUNT; ++i )
		mIcon[i] = 0;
}

cInvenEquipWindow::~cInvenEquipWindow()
{
}

void cInvenEquipWindow::Open()
{
	/// 아이템 슬롯 검사
	ITEMMAN->CheckItemSlot( INVENTORY_HAND_RIGHT1 );
	ITEMMAN->CheckItemSlot( INVENTORY_HAND_RIGHT2 );
	ITEMMAN->CheckItemSlot( INVENTORY_COSTUME_BODY1 );

	/// 슬롯 잠그기.
	unsigned short slotEndIndex = HERO->GetWearCardEndSlot();
	if( slotEndIndex < INVENTORY_WEAR_RING || slotEndIndex > INVENTORY_WEAR_CARD3 )	
	{
		assert( 0 && "Bad slot index" );
		return;
	}

	/// 해당 슬롯 인덱스까지 아이콘 enable 시키기
	for( int slotIndex = INVENTORY_WEAR_CARD1; slotIndex <= INVENTORY_WEAR_CARD3; ++slotIndex )
	{
		int iconIndex = slotIndex - INVENTORY_WEAR_BEGIN;

		if( slotIndex <= slotEndIndex )
		{
			mIcon[iconIndex]->SetEnabled( true );
		}
		else
		{
			mIcon[iconIndex]->SetEnabled( false );
			/// 슬롯 막힘 이미지로 바꾸기
			cIconParam* pIconParam = UIMAN->GetIconParam( 217 );
			mIcon[iconIndex]->ChangeImage( pIconParam );
		}
	}
}

bool cInvenEquipWindow::OnCreate( cUINodeProperty* /*property*/ )
{
	/// 아이콘을 생성x
	cString str;
	for( unsigned short i = 0, slotIndex = INVENTORY_WEAR_BEGIN; i < EQUIP_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "InvenEquip_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( slotIndex, icon );

		/// 아이콘 좌표 세팅( 무기스위칭관련 )
		ITEMMAN->SetWeaponIconPos( slotIndex, icon->GetRelativeRect() );
	}
	return true;
}

void cInvenEquipWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cInvenEquipWindow::OnIconDoubleClicked( cUINode* caller, unsigned int /*id*/ )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		ITEMMAN->TryToUnEquipItem( icon->GetSlotIndex() );
}

void cInvenEquipWindow::OnIconClicked( cUINode* caller, unsigned int id )
{
	/// 인벤토리 윈도우에 아이콘 눌림 이벤트
	if( caller )
		SendEvent( mpParent, eUIEVENT_ICON_CLICKED, caller, id );
}

bool cInvenEquipWindow::SetWearCardSlot( unsigned short slotEndIndex )
{
	if( slotEndIndex < INVENTORY_WEAR_CARD1 || slotEndIndex > INVENTORY_WEAR_CARD3 )	
	{
		assert( 0 && "Bad slot index" );
		return false;
	}

	if( slotEndIndex <= INVENTORY_WEAR_BEGIN )
	{
		assert(0);
		return false;
	}

	/// 해당 슬롯 인덱스 아이콘 enable 시키기
	unsigned short iconIndex = (unsigned short)(slotEndIndex - INVENTORY_WEAR_BEGIN);

	if( mIcon[iconIndex] )
	{
		mIcon[iconIndex]->SetEnabled( true );
		mIcon[iconIndex]->ClearImage();
	}

	return true;
}