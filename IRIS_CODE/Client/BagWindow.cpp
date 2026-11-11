#include "stdafx.h"
#include "BagWindow.h"
#include "GameUIManager.h"
#include "InventoryWindow.h"
#include "DragWindow.h"
#include "Label.h"
#include "ItemIcon.h"

cBagWindow::cBagWindow( cInventoryWindow* invenWin, unsigned int index )
: mInventoryWindow( invenWin )
, mIndex( index )
, mCaption(0)
, mSelectIcon(0)
, mpSelectImage(0)
, mShowSelect( false )
, mUseFlag( false )
{
	for( int i = 0;i < BAG_SLOT_COUNT; ++i )
		mIcon[i] = 0;

	/// npc 대화상태에 visible 백업 안함!!
	/// 왜냐면, 인벤토리의 가방버튼에 따라 visible 재 세팅 하므로
	SetBackupVisible( false );
}

cBagWindow::~cBagWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cBagWindow::Open()
{
}

void cBagWindow::Close()
{
	cUIWindow::Close();

	mSelectIcon = 0;
	mShowSelect = false;
	mUseFlag = false;
}

void cBagWindow::Show( bool onsound )
{
	if( mUseFlag )
	{
		cUIWindow::Show( onsound );
	}
}

void cBagWindow::OnShow()
{
	///
	if( mCaption )
		mCaption->SetEnabled( false );

	cUIWindow::OnHide();
}


void cBagWindow::OnHide()
{
	cUIWindow::OnHide();

	/// 선택된 아이콘 표시 
	mSelectIcon = 0;
	mShowSelect = false;
}

void cBagWindow::HideESC( bool onsound )
{
	cUIWindow::HideESC( onsound );

	if( mInventoryWindow )
		mInventoryWindow->Hide();
}

bool cBagWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	if( mIndex == 0 )
	{
		mCaption = (cLabel*)GetChild( eUIID_BAG0_CAPTION );
	}
	else if( mIndex == 1 )
	{
		mCaption = (cLabel*)GetChild( eUIID_BAG1_CAPTION );
	}
	else if( mIndex == 2 )
	{
		mCaption = (cLabel*)GetChild( eUIID_BAG2_CAPTION );
	}
	else
	{
		assert(0);
	}

	if( mCaption )
		mCaption->SetEnabled( false );

	/// 아이콘
	unsigned short slotIndex = (unsigned short)(INVENTORY_BAG1_BEGIN + mIndex * BAG_SLOT_COUNT);

	cString str;
	for( unsigned int i = 0; i < BAG_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "Bag_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( slotIndex, icon );
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

void cBagWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mSelectIcon && mpSelectImage )
	{
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
	}
}

bool cBagWindow::Reset()
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

void cBagWindow::UpdateSkin()
{
	UpdateRect();

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	/// 자식 스킨 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
			n->UpdateSkin();
	}
}

void cBagWindow::OnRender(cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mShowSelect && mpSelectImage )
		mpSelectImage->Draw();
}

void cBagWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	switch( id )
	{
	case eUIID_BAG0_CLOSE:
		if( mInventoryWindow )
			mInventoryWindow->SetBagButtonPressed( 0, false );
		break;
	case eUIID_BAG1_CLOSE:
		if( mInventoryWindow )
			mInventoryWindow->SetBagButtonPressed( 1, false );
		break;
	case eUIID_BAG2_CLOSE:
		if( mInventoryWindow )
			mInventoryWindow->SetBagButtonPressed( 2, false );
		break;
	}

	Hide();
}

void cBagWindow::OnIconDragged( cUINode* caller, unsigned int /*id*/, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cBagWindow::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	SendEvent( GAMEUI->GetInventoryWindow(), eUIEVENT_ICON_DOUBLECLICKED, caller, id );
}

void cBagWindow::OnIconClicked( cUINode* caller, unsigned int id )
{
	UpdateSelect( (cItemIcon*)caller );
}

void cBagWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	SendEvent( GAMEUI->GetInventoryWindow(), eUIEVENT_ICON_RIGHTCLICKED, caller, id, shift );
}

/// 새로운 슬롯으로 선택 갱신
void cBagWindow::ChangeSelect( unsigned short slotIndex )
{
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
	{
		cItemIcon* newIcon = item->GetIcon();
		UpdateSelect( newIcon );
	}
}

void cBagWindow::UpdateSelect( cItemIcon* icon )
{
	if( icon == 0 )
	{
		assert(0);
		return;
	}

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
