#include "stdafx.h"
#include "WareHouseWindow.h"
#include "PushButton.h"
#include "WareSheetWindow.h"
#include "ItemIcon.h"
#include "Label.h"
#include "Npc_Common.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "UIManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "NumberEditBox.h"
#include "DragWindow.h"
#include "ItemManager.h"
#include "Player_Common.h"

cWareHouseWindow::cWareHouseWindow()
: mSelectIcon(0)
, mpMoney(0)
, mpSelectImage(0)
, mShowSelect( false )
, mShowBit(0)
{
	for( int i = 0; i < 3; ++i )
		mBagButton[i] = 0;
}

cWareHouseWindow::~cWareHouseWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cWareHouseWindow::Open()
{
	mShowBit = 0;

	SetWareBagEndSlot( HERO->GetWareHouseEndSlot() );

	/// 허용된 가방만 보이기
	/*int	bagButtonIndex = -1;
	unsigned short endBagSlot = HERO->GetWareHouseEndSlot();
 	switch( endBagSlot )
 	{
	case INVENTORY_WAREHOUSE0_END:	return;
 	case INVENTORY_WAREHOUSE1_END:	bagButtonIndex =  0;	break;
 	case INVENTORY_WAREHOUSE2_END:	bagButtonIndex =  1;	break;
 	case INVENTORY_WAREHOUSE3_END:	bagButtonIndex =  2;    break;
 	default:
 		{
 			assert( 0 && "Bad slot index" );
 			return;
 		}
 		break;
 	}
 
	cWareHouseBagWindow* wareBagWin[3] = {0,};
	wareBagWin[0] = GAMEUI->GetWareHouseBagWindow(0);
	wareBagWin[1] = GAMEUI->GetWareHouseBagWindow(1);
	wareBagWin[2] = GAMEUI->GetWareHouseBagWindow(2);
	if( !wareBagWin[0] || !wareBagWin[1] || !wareBagWin[2] )
	{
		assert(0);
		return;
	}

 	/// 해당 슬롯 인덱스까지 아이콘 enable 시키기
 	for( int idx = 0; idx < 3; ++idx )
 	{
 		if( idx <= bagButtonIndex )
 		{
 			mBagButton[idx]->SetEnabled( true );
 			mBagButton[idx]->Show();
			wareBagWin[idx]->SetUseFlag( true );
 		}
 		else
 		{
 			mBagButton[idx]->SetEnabled( false );
 			mBagButton[idx]->Hide();
			wareBagWin[idx]->SetUseFlag( false );
 		}
 	}*/
}

void cWareHouseWindow::Close()
{
	cUIWindow::Close();
	
	mSelectIcon = 0;
	mShowSelect = false;

	///
	UIMAN->HideDrag( this );

	if( HERO->GetNpcFunc() == NPC_FUNC_WAREHOUSE )
	{
		HERO->CloseNpcTalk( false );
	}
}

void cWareHouseWindow::OnShow()
{
	cUIWindow::OnShow();

	SetMoney( HERO->GetDeposit() );

	BYTE byte = mShowBit;
	for( int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetWareHouseBagWindow(i);
		if( b && (byte & 1) )
			b->Show();

		byte = byte >> 1;
	}
}

void cWareHouseWindow::OnHide()
{
	cUIWindow::OnHide();

	mSelectIcon = 0;
	mShowSelect = false;

	///
	UIMAN->HideDrag( this );

	if( HERO->GetNpcFunc() == NPC_FUNC_WAREHOUSE )
	{
		HERO->CloseNpcTalk();
	}
	else if( HERO->GetNpcFunc() == NPC_FUNC_MAX && HERO->GetStopFlag() == eSTOP_SUMMON_PAPER )
	{	
		HERO->CloseBySummonItem( NM_ITEM, NM_ITEM_WAREHOUSE_SUMMON_CLOSE_REQ );
	}

	for( int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetWareHouseBagWindow(i);
		if( b )
			b->Hide( false );
	}
}

void cWareHouseWindow::SetBagPressed( unsigned int index, bool pressed )
{
	if( index < 0 || index > 3 )
	{
		assert( 0 && "Bad Index" );
		return;
	}

	/// 허용된 가방만 보이기
	unsigned int   endBagIndex = UINT_MAX;
	unsigned short endBagSlot = HERO->GetWareHouseEndSlot();
	switch( endBagSlot )
	{
	case INVENTORY_WAREHOUSE0_END:	return;
	case INVENTORY_WAREHOUSE1_END:	endBagIndex =  0;	break;
	case INVENTORY_WAREHOUSE2_END:	endBagIndex =  1;	break;
	case INVENTORY_WAREHOUSE3_END:	endBagIndex =  2;    break;
	default:
		{
			assert( 0 && "Bad slot index" );
			return;
		}
		break;
	}

	if( index > endBagIndex )
		return;

	if( mBagButton[index] )
		mBagButton[index]->SetPress( pressed );

	//특정비트를 1로 지정하는 공식은 a | (a<<(n-1)) 가 되고
	//특정비트를 0으로 지정하는 공식은 (a&~(1<<n-1)) 가 되고
	//원하는 특정 비트를 반대로 하려면 (a^(1<<n-1)) 가 된다.

	BYTE byte = 0;
	BYTE end = 1;
	if( pressed )
	{
		byte = (BYTE)(1 << index);
	}

	end = (BYTE)(1 << index);
	BYTE mask = 0xFF ^ end;

	/// 마스크 씌워서 해당 위치의 비트만 세팅하기
	mShowBit = ( mShowBit & mask ) | byte;
}

void cWareHouseWindow::SetMoney( unsigned long money )
{
	if( mpMoney )
		mpMoney->SetValue( money );
}

bool cWareHouseWindow::OnCreate( cUINodeProperty* property )
{
	if( cTabWindow::OnCreate( property ) == false )
		return false;

	/// 탭쉬트 윈도우를 생성
	cWareSheetWindow* sheet = (cWareSheetWindow*)GetChild( eUIID_WAREHOUSE_SHEET1 );
	if( sheet )
	{
		sheet->InitWindow( 0 );
		sheet->Hide();
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 선택 이미지 생성
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "Inventory_SelectImage" );
	if( selectSkin )
	{
		mpSelectImage = new cUIImage( selectSkin->mpTexture );
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int w = selectSkin->mSkinInfo->mWidth;
		unsigned int h = selectSkin->mSkinInfo->mHeight;
		unsigned int x = selectSkin->mSkinInfo->mX;
		unsigned int y = selectSkin->mSkinInfo->mY;
		mpSelectImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSelectImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	SetCurrent(0);

	mBagButton[0] = (cPushButton*)GetChild( eUIID_WAREHOUSE_BAG1 );
	mBagButton[1] = (cPushButton*)GetChild( eUIID_WAREHOUSE_BAG2 );
	mBagButton[2] = (cPushButton*)GetChild( eUIID_WAREHOUSE_BAG3 );
	mpMoney = (cNumberEditBox*)GetChild( eUIID_WAREHOUSE_MONEY );

	if( mpMoney )
	{
		mpMoney->SetMaxValue( MAX_HAVE_MONEY );
	}
	else
	{
		assert(0);
	}

	return true;
}

void cWareHouseWindow::UpdateRect()
{
	cTabWindow::UpdateRect();

	if( mSelectIcon && mpSelectImage )
	{
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
	}
}

void cWareHouseWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	cTabWindow::OnRender( pKeeper );	

	if( mShowSelect && mpSelectImage )
		mpSelectImage->Draw();
}

void cWareHouseWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	switch( id )
	{
	case eUIID_WAREHOUSE_CLOSE:
		Hide();
		break;
	case eUIID_WAREHOUSE_INMONEY:
		/// 입금
		GAMEUI->ShowMoneyWindow( eMONEY_IN, eMBEVENT_WAREHOUSE_MONEYIN );
		break;
	case eUIID_WAREHOUSE_OUTMONEY:
		/// 출금
		GAMEUI->ShowMoneyWindow( eMONEY_OUT, eMBEVENT_WAREHOUSE_MONEYOUT );
		break;
	}
}

void cWareHouseWindow::OnPushButtonPressed( cUINode* /*caller*/, unsigned int id )
{
	switch( id )
	{
	case eUIID_WAREHOUSE_BAG1:
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow( 0 );
			if( bag )
			{
				bag->Toggle();
				SetBagPressed( 0, bag->IsVisible() );
			}
		}
		break;
	case eUIID_WAREHOUSE_BAG2:
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow( 1 );
			if( bag )
			{
				bag->Toggle();
				SetBagPressed( 1, bag->IsVisible() );
			}
		}
		break;
	case eUIID_WAREHOUSE_BAG3:
		{
			cWareHouseBagWindow* bag = GAMEUI->GetWareHouseBagWindow( 2 );
			if( bag )
			{
				bag->Toggle();
				SetBagPressed( 2, bag->IsVisible() );
			}
		}
		break;
	}
}

void cWareHouseWindow::OnTabButtonPressed(cUINode* caller, unsigned int id )
{
	cTabWindow::OnTabButtonPressed( caller, id );
	mShowSelect = false;
	//mShowSelect = ( id == eUIID_WAREHOUSE_PRIVATE && mSelectIcon ) ? true : false;
}

void cWareHouseWindow::OnIconClicked( cUINode* caller, unsigned int /*id*/ )
{
	UpdateSelect( (cItemIcon*)caller );
}

/// 새로운 슬롯으로 선택 갱신
void cWareHouseWindow::ChangeSelect( unsigned short slotIndex )
{
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
	{
		cItemIcon* newIcon = item->GetIcon();
		UpdateSelect( newIcon );
	}
}

/// 눌린아이콘을 선택된 아이콘으로 저장.
void cWareHouseWindow::UpdateSelect( cItemIcon* icon )
{
	if( icon )
	{
		unsigned int itemIdex = icon->GetItemIndex();
		if( mpSelectImage && itemIdex > 0 )
		{
			/// 새 선택 아이콘 눌림효과
			cUIRect rc = icon->GetAbsoluteRect();
			mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
			mSelectIcon = icon;
			mShowSelect = true;

			ITEMMAN->SetInvenSelect( this );
		}
		else
			mShowSelect = false;
	}
	else
	{
		assert(0);
	}
}

bool cWareHouseWindow::SetWareBagEndSlot( unsigned short endBagSlot )
{
	/// 허용된 가방만 보이기
	int	bagButtonIndex = -1;
	switch( endBagSlot )
	{
	case INVENTORY_WAREHOUSE0_END:	bagButtonIndex = -1;	break;
	case INVENTORY_WAREHOUSE1_END:	bagButtonIndex =  0;	break;
	case INVENTORY_WAREHOUSE2_END:	bagButtonIndex =  1;	break;
	case INVENTORY_WAREHOUSE3_END:	bagButtonIndex =  2;    break;
	default:
		{
			assert( 0 && "Bad slot index" );
			return false;
		}
		break;
	}

	cWareHouseBagWindow* wareBagWin[3] = {0,};
	wareBagWin[0] = GAMEUI->GetWareHouseBagWindow(0);
	wareBagWin[1] = GAMEUI->GetWareHouseBagWindow(1);
	wareBagWin[2] = GAMEUI->GetWareHouseBagWindow(2);
	if( !wareBagWin[0] || !wareBagWin[1] || !wareBagWin[2] )
	{
		assert(0);
		return false;
	}

	/// 해당 슬롯 인덱스까지 아이콘 enable 시키기
	for( int idx = 0; idx < 3; ++idx )
	{
		if( idx <= bagButtonIndex )
		{
			mBagButton[idx]->SetEnabled( true );
			mBagButton[idx]->Show();
			wareBagWin[idx]->SetUseFlag( true );
		}
		else
		{
			mBagButton[idx]->SetEnabled( false );
			mBagButton[idx]->Hide();
			wareBagWin[idx]->SetUseFlag( false );
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
/// 창고 가방 윈도우
cWareHouseBagWindow::cWareHouseBagWindow( cWareHouseWindow* wareHouseWindow, unsigned int index )
: mWareHouseWindow( wareHouseWindow )
, mSelectIcon(0)
, mpSelectImage(0)
, mShowSelect(false)
, mIndex(index)
, mUseFlag(false)
{
	assert( wareHouseWindow );

	for( int i = 0; i < WAREHOUSE_SLOT_COUNT; ++i )
		mIcon[i] = 0;
}

cWareHouseBagWindow::~cWareHouseBagWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cWareHouseBagWindow::Open()
{
}

void cWareHouseBagWindow::Close()
{
	cUIWindow::Close();

	mSelectIcon = 0;
	mShowSelect = false;
	mUseFlag = false;
}

void cWareHouseBagWindow::Show( bool onsound )
{
	if( mUseFlag )
	{
		cUIWindow::Show();
	}
}

void cWareHouseBagWindow::OnHide()
{
	cUIWindow::OnHide();

	mSelectIcon = 0;
	mShowSelect = false;
}

bool cWareHouseBagWindow::Reset()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return false;
	}

	unsigned int x = pnodeSkin->mSkinInfo->mX;
	unsigned int y = pnodeSkin->mSkinInfo->mY;
	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;

	/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
	/// 붙어있도록 한다 
	if( (pnodeSkin->mSkinInfo->mX + pnodeSkin->mSkinInfo->mWidth) >= (int)GetScreenWidth() )
	{
		x = GetScreenWidth() - w;
	}

	if( (pnodeSkin->mSkinInfo->mY + pnodeSkin->mSkinInfo->mHeight) >= (int)GetScreenHeight() )
	{
		y = GetScreenHeight() - h;
	}

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

bool cWareHouseBagWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	unsigned int slotIndex = 0;

	if( mIndex == 0 )
	{
		mCaption = (cLabel*)GetChild( eUIID_WAREHOUSE_BAG1_CAPTION );
		slotIndex = INVENTORY_WAREHOUSE1_BEGIN;
	}
	else if( mIndex == 1 )
	{
		mCaption = (cLabel*)GetChild( eUIID_WAREHOUSE_BAG2_CAPTION );
		slotIndex = INVENTORY_WAREHOUSE2_BEGIN;
	}
	else if( mIndex == 2 )
	{
		mCaption = (cLabel*)GetChild( eUIID_WAREHOUSE_BAG3_CAPTION );
		slotIndex = INVENTORY_WAREHOUSE3_BEGIN;
	}
	else
	{
		assert(0);
	}

	if( mCaption )
		mCaption->SetEnabled( false );

	/// 아이콘을 생성
	cString str;
	for( unsigned int i = 0; i < WAREHOUSE_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "WareBagSheet_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( (unsigned short)slotIndex, icon );
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 선택 이미지 생성
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "Inventory_SelectImage" );
	if( selectSkin )
	{
		mpSelectImage = new cUIImage( selectSkin->mpTexture );
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int w = selectSkin->mSkinInfo->mWidth;
		unsigned int h = selectSkin->mSkinInfo->mHeight;
		unsigned int x = selectSkin->mSkinInfo->mX;
		unsigned int y = selectSkin->mSkinInfo->mY;
		mpSelectImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSelectImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	return true;
}

void cWareHouseBagWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mSelectIcon && mpSelectImage )
	{
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
	}
}

void cWareHouseBagWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cUIWindow::OnProcess( deltaTime, accumTime );
}

void cWareHouseBagWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mShowSelect && mpSelectImage )
		mpSelectImage->Draw();
}

void cWareHouseBagWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_WAREHOUSE_BAG1_CLOSE:
		if( mWareHouseWindow )
			mWareHouseWindow->SetBagPressed( 0, false );
		break;
	case eUIID_WAREHOUSE_BAG2_CLOSE:
		if( mWareHouseWindow )
			mWareHouseWindow->SetBagPressed( 1, false );
		break;
	case eUIID_WAREHOUSE_BAG3_CLOSE:
		if( mWareHouseWindow )
			mWareHouseWindow->SetBagPressed( 2, false );
		break;
	}

	Hide();
}

void cWareHouseBagWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cWareHouseBagWindow::OnIconClicked( cUINode* caller, unsigned int id )
{
	/// 창고 윈도우1에 아이콘 눌림 이벤트
	UpdateSelect( (cItemIcon*)caller );
}

/// 새로운 슬롯으로 선택 갱신
void cWareHouseBagWindow::ChangeSelect( unsigned short slotIndex )
{
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
	{
		cItemIcon* newIcon = item->GetIcon();
		UpdateSelect( newIcon );
	}
}

/// 눌린아이콘을 선택된 아이콘으로 저장.
void cWareHouseBagWindow::UpdateSelect( cItemIcon* icon )
{
	if( icon )
	{
		unsigned int itemIdex = icon->GetItemIndex();
		if( mpSelectImage && itemIdex > 0 )
		{
			/// 새 선택 아이콘 눌림효과
			cUIRect rc = icon->GetAbsoluteRect();
			mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
			mSelectIcon = icon;
			mShowSelect = true;

			ITEMMAN->SetInvenSelect( this );
		}
		else
			mShowSelect = false;
	}
	else
	{
		assert(0);
	}
}

void cWareHouseBagWindow::HideESC( bool onsound )
{
	cUIWindow::HideESC( onsound );

	if( mWareHouseWindow )
		mWareHouseWindow->Hide();
}

void cWareHouseBagWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
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
