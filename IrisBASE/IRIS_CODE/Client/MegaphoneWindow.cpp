#include "StdAfx.h"
#include "MegaphoneWindow.h"
#include "EditBox.h"
#include "ItemIcon.h"
#include "ChatManager.h"
#include "ItemManager.h"
#include "DragWindow.h"
#include "CoolTimeManager.h"

cMegaphoneWindow::cMegaphoneWindow()
: mpChatEdit(0)
, mpLinkIcon(0)
, mpOk(0)
, mInventoryIndex(0)
, mInventorySlotIndex(USHRT_MAX)
, mLinkItemIndex(0)
{
	SetBackupVisible( false );
}

cMegaphoneWindow::~cMegaphoneWindow()
{

}

void cMegaphoneWindow::Clear()
{
	if( mpChatEdit )
		mpChatEdit->Clear();

	// 링크아이콘 클리어
	DelLinkItem();

	// 메가폰 호출 아이템 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( Item )
				Item->SetLock( false );
		}

		mInventorySlotIndex = USHRT_MAX; 
	}

	//
	mInventoryIndex = 0;
	mLinkItemIndex = 0;
}

void cMegaphoneWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cMegaphoneWindow::Show( unsigned short slotIndex, unsigned long dbidx )
{
	if( dbidx == 0 )
	{
		assert(0);
		return;
	}

	// 메가폰 호출 아이템 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( Item )
				Item->SetLock( false );
		}

		mInventorySlotIndex = USHRT_MAX; 
	}

	// 셋팅
	mInventoryIndex = dbidx;
	mInventorySlotIndex = slotIndex;

	// 메가폰 호출 아이템 락 걸기
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
		item->SetLock( true );

	cUIWindow::Show();
}

void cMegaphoneWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );
}

void cMegaphoneWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

bool cMegaphoneWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpChatEdit = (cEditBox*)GetChild( eUIID_MEGAPHONE_EDIT );
	mpOk = GetChild( eUIID_MEGAPHONE_OK );

	// 링크아이콘 생성
	mpLinkIcon = new cItemIcon;
	if( mpLinkIcon->CreateBySkinName( "MegaPhone_LinkIcon", this, 100 ) == false )
		return false;

	return true;
}

void cMegaphoneWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_MEGAPHONE_OK:
		if( mpChatEdit )
		{
			if( CHATMANAGER->SendChatMsg( eCHAT_MEGAPHONE, mpChatEdit->GetText() ) == true )
			{
				mpChatEdit->Clear();
				mpChatEdit->ReleaseFocus();

				DelLinkItem();

				//
				NetLock();
			}
		}
		else
			assert(0);
		break;
	case eUIID_MEGAPHONE_CANCEL:
	case eUIID_MEGAPHONE_CLOSE:
		Hide();
		break;
	}
}

void cMegaphoneWindow::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_MEGAPHONELINK, icon, pos, shift );
}

void cMegaphoneWindow::OnNetLock( int lockTry )
{
	if( mpOk )
		mpOk->NetLock( lockTry );
}

void cMegaphoneWindow::OnNetUnLock( int lockTry )
{
	if( mpOk )
		mpOk->NetUnLock( lockTry );
}

bool cMegaphoneWindow::IsLinked()
{
	if( GetLinkItemIndex() > 0 || GetLinkSlotIndex() != USHRT_MAX )
		return true;
	return false;
}


void cMegaphoneWindow::StartCoolTime()
{
	if( mInventoryIndex > 0 && mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( item && item->GetDefine() )
			{	
				cItemDefine* define = item->GetDefine();
				cItemAbility* abil = define->GetAbililty();
				if( abil && abil->GetCoolTime() > 0 )
				{
					COOLMAN->StartCoolTime( item->GetIndex() );
				}
			}
			else
				assert(0);
		}
		else
			assert(0);
	}
	else
		assert(0);
}

bool cMegaphoneWindow::AddLinkItem( unsigned short slotIndex, unsigned long itemIndex )
{
	if( !mpLinkIcon )
	{
		assert(0);
		return false;
	}

	if( itemIndex > 0 )
	{
		if( mLinkItemIndex == itemIndex )
			return true;

		cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
		if( !define )
		{
			assert(0);
			return false;
		}

		/// 기존 락 해제
		unsigned short itemSlotIndex = mpLinkIcon->GetSlotIndex();
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
				oldItem->SetLock( false );
		}

		// 아이템 인덱스로 세팅하는 경우,
		mLinkItemIndex = itemIndex;

		mpLinkIcon->SetSlotIndex( USHRT_MAX );
		mpLinkIcon->SetItemIndex( mLinkItemIndex );
		mpLinkIcon->ChangeImage( define->GetIconParam(), ICON_SIZE, ICON_SIZE );
	}
	else if( slotIndex != USHRT_MAX )
	{
		// 슬롯으로 세팅하는경우,
		if( slotIndex > MAX_INVENTORY )
			return false;

		cItem* item = ITEMMAN->GetItem( slotIndex );
		if( !item )
		{
			assert(0);
			return false;
		}

		cItemDefine* define = item->GetDefine();
		if( !define )
		{
			assert(0);
			return false;
		}

		/// 기존 등록 아이템 삭제
		unsigned short itemSlotIndex = mpLinkIcon->GetSlotIndex();
		if( itemSlotIndex == slotIndex )
			return true;

		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
				oldItem->SetLock( false );
		}

		/// 참조 슬롯 인덱스
		mpLinkIcon->SetSlotIndex( slotIndex );
		item->SetLock( true );

		/// 아이템 이미지 체인지
		mpLinkIcon->SetItemIndex( item->GetIndex() );
		mpLinkIcon->ChangeImage( define->GetIconParam(), ICON_SIZE, ICON_SIZE );

		cItem* newItem = ITEMMAN->GetItem( slotIndex );
		if( newItem )
			mpLinkIcon->SetSeal( newItem->IsSeal() );

		mLinkItemIndex = 0;
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

bool cMegaphoneWindow::DelLinkItem()
{
	if( mpLinkIcon )
	{
		unsigned short itemSlotIndex = mpLinkIcon->GetSlotIndex();
		if( itemSlotIndex != USHRT_MAX )
		{
			/// 아이템 락 초기화
			if( itemSlotIndex <= MAX_INVENTORY )
			{
				cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
				if( oldItem )
				{
					oldItem->SetLock( false );
				}
			}
		}

		mLinkItemIndex = 0;

		mpLinkIcon->SetSlotIndex( USHRT_MAX );
		mpLinkIcon->ChangeImage( 0 );
		mpLinkIcon->SetItemIndex( 0 );
		mpLinkIcon->SetTipType( eTOOLTIP_NONE );
		mpLinkIcon->SetSeal( false );
	}
	return true;
}

unsigned short cMegaphoneWindow::GetLinkSlotIndex()
{
	return ( mpLinkIcon ) ? mpLinkIcon->GetSlotIndex() : USHRT_MAX;
}

unsigned long cMegaphoneWindow::GetLinkItemIndex()
{
	return mLinkItemIndex;
}
