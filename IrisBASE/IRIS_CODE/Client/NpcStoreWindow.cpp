#include "stdafx.h"
#include "NpcStoreWindow.h"
#include "DoubleItemIconListBox.h"
#include "NumberEditBox.h"
#include "GameUIManager.h"
#include "DragWindow.h"
#include "RadioButton.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"

cNpcStoreWindow::cNpcStoreWindow()
: mNpcIndex(0)
, mpList(0)
, mpRadioSkin(0)
, mpBuy(0)
, mTabIndex(0)
{
	::memset( mpRadio, 0, sizeof( mpRadio ) );
}

cNpcStoreWindow::~cNpcStoreWindow()
{

}

void cNpcStoreWindow::Open()
{
}

void cNpcStoreWindow::Close()
{
	cUIWindow::Close();

	///
	UIMAN->HideDrag( this );
	HERO->CloseNpcTalk( false );

	for( unsigned int i = 0; i < MAX_RADIO; ++i )
	{
		if( mpRadio[i] )
			mpRadio[i]->Hide();
	}
}

void cNpcStoreWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 리스트 갱신
	if( mpList )
	{
		mpList->Clear();

		/// 데이터 삽입
		cItemNpcStore* store = ITEMSCRIPT->GetItemNpcStore( mNpcIndex );
		if( store )
		{
			/// 라디오 버튼 세팅
			unsigned int tabCount = store->GetTabCount();
			for( unsigned int i = 0; i < tabCount; ++i )
			{
				/// 세팅
				if( mpRadio[i] )
				{
					unsigned long textIndex = store->GetTextIndex( i );
					mpRadio[i]->SetText( UIMAN->GetUIText( textIndex ) );
					mpRadio[i]->Show();
				}
			}

			UpdateItemList( 0 );
		}
	}

	if( mpRadio[0] )
		mpRadio[0]->SetPress( true );
}

void cNpcStoreWindow::OnHide()
{
	cUIWindow::OnHide();

	///
	UIMAN->HideDrag( this );
	HERO->CloseNpcTalk();

	for( unsigned int i = 0; i < MAX_RADIO; ++i )
	{
		if( mpRadio[i] )
			mpRadio[i]->Hide();
	}
	mTabIndex = ULONG_MAX;
}

void cNpcStoreWindow::Show( unsigned long npcIndex )
{
	mNpcIndex = npcIndex;
	cUIWindow::Show();
}

bool cNpcStoreWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpList = (cDoubleItemIconListBox*)GetChild( eUIID_NPCSTORE_LIST );
	mpBuy = (cButton*)GetChild( eUIID_NPCSTORE_BUY );

	/// 
	mpList->CreateIcon( "NpcStore_Slot%02d", NPCSTORE_SLOT_COUNT );
	mpList->SetSlotIndexAllIcon( NPCSTORE_SLOT_COUNT, NPCSTORE_SLOT_BEGIN );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( pSkin )
	{
		mpRadioSkin = pSkin->GetNodeSkin( "NpcStore_Radio" );
		if( mpRadioSkin == 0 )
			return false;
	}
		
	/// 라디오버튼 생성
	for( unsigned int i = 0; i < MAX_RADIO; ++i )
	{
		mpRadio[i] = new cRadioButton;
		if( mpRadio[i]->CreateBySkinName( "NpcStore_Radio", this, i ) == false )
			return false;
	
		/// 위치 세팅
		cUIPos pos;
		pos.mX = mpRadioSkin->mSkinInfo->mX + (mpRadioSkin->mSkinInfo->mWidth * i );
		pos.mY = mpRadioSkin->mSkinInfo->mY;
		mpRadio[i]->SetRelativePos( pos );
		mpRadio[i]->Hide();
	}
	return true;
}

void cNpcStoreWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpRadioSkin )
	{
		for( unsigned int i = 0; i < MAX_RADIO; ++i )
		{
			if( mpRadio[i] )
			{
				cUIPos pos;
				pos.mX = mpRadioSkin->mSkinInfo->mX + (mpRadioSkin->mSkinInfo->mWidth * i );
				pos.mY = mpRadioSkin->mSkinInfo->mY;
				mpRadio[i]->SetRelativePos( pos );
			}
		}
	}
}

void cNpcStoreWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	
	if( mpRadioSkin )
	{
		for( unsigned int i = 0; i < MAX_RADIO; ++i )
		{
			if( mpRadio[i] )
			{
				cUIPos pos;
				pos.mX = mpRadioSkin->mSkinInfo->mX + (mpRadioSkin->mSkinInfo->mWidth * i );
				pos.mY = mpRadioSkin->mSkinInfo->mY;
				mpRadio[i]->SetRelativePos( pos );
			}
		}
	}
}

void cNpcStoreWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_NPCSTORE_CLOSE:
		Hide();
		break;
	case eUIID_NPCSTORE_BUY:
		{
			/// 구입
			int row = mpList->GetSelectIndex();
			unsigned long itemIndex = (unsigned long)mpList->GetExtraData( row );

			if( itemIndex > 0 )
			{
				cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
				assert( define );

				if( define )
				{
					/// 1개라도 살수 있어야 구입가능
					if( ITEMMAN->CheckBuyItem( itemIndex, 1 ) == false )
						return;

					if( define->GetCapacity() == 1 )
						GAMEUI->ShowItemBuyAskWindow( itemIndex, 1 );
					else
						GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_BUY, itemIndex );
				}
			}
		}
		break;
	}
}

void cNpcStoreWindow::OnRadioPressed( cUINode* , unsigned int id )
{
	if( mpList )
	{
		/// 재 세팅
		if( id >= 0 && id < MAX_RADIO )
		{
			mpList->Clear();
			UpdateItemList( id );
		}
	}
}

void cNpcStoreWindow::OnListBoxSelected( cUINode* , unsigned int )
{
	OnCommand( 0, eUIID_NPCSTORE_BUY );
}

void cNpcStoreWindow::OnListBoxSelectedRButton( cUINode* caller, unsigned int , bool shift )
{
	int row = mpList->GetSelectIndex();
	unsigned long itemIndex = (unsigned long)mpList->GetExtraData( row );
	cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	if( shift )
		ITEMMAN->TryToBuyItem( itemIndex, define->GetCapacity() );
	else
		ITEMMAN->TryToBuyItem( itemIndex, 1 );
}

void cNpcStoreWindow::OnIconDragged( cUINode* caller, unsigned int, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_NPC_STORE, icon, pos, shift );
}

void cNpcStoreWindow::OnIconRightClicked( cUINode* caller, unsigned int , bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
	{
		unsigned long itemIndex = icon->GetItemIndex();
		cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
		if( !define )
		{
			assert(0);
			return;
		}

		if( shift )
			ITEMMAN->TryToBuyItem( itemIndex, define->GetCapacity() );
		else
			ITEMMAN->TryToBuyItem( itemIndex, 1 );
	}
}

void cNpcStoreWindow::UpdateItemList( unsigned int tabIndex )
{
	mTabIndex = tabIndex;
	/// 데이터 삽입
	cItemNpcStore* store = ITEMSCRIPT->GetItemNpcStore( mNpcIndex );
	if( store )
	{
		/// 아이템 탭 설정
		cStringT priceStr;
		unsigned int count = store->GetItemCount( tabIndex );
		for( unsigned int i = 0; i < count; ++i )
		{
			unsigned long itemIndex = store->GetItemIndex( tabIndex, i );
			cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
			if( define )
			{
				unsigned long priceRate = store->GetItemSellRate( tabIndex );
				if( priceRate == ULONG_MAX )
					return;

				unsigned long color = COLOR_WHITE;
				ITEMMAN->GetTooltipColor( define->GetTipType(), color );

				TCHAR name[ITENNAME_LEN] = {0,};
				/// 텍스트가 들어갈 곳의 너비를 얻어온다.
				unsigned int colWidth = mpList->GetColumeWidth();
				bool ret = UIMAN->ModifyText( define->GetName(), name, ITENNAME_LEN, colWidth );

				LPCTSTR itemName = ( ret == true ) ? name : define->GetName();
				unsigned int row = mpList->AddRow( itemName, color, (void*)itemIndex );

				/// 가격 방법 세팅
				TCHAR temp[128]={0,};
				unsigned long price = unsigned long(define->GetBuyPrice() * priceRate * 0.01);
				LPCTSTR priceStr = cNumberEditBox::FormatNumber( price );
				LPCTSTR uiStr = UIMAN->GetUIText( 355 );

				/// 삽입
				unsigned long tooltipIdx = ITEMMAN->GetPriceUnit( store->GetItemSellType( tabIndex ), itemIndex );
				::_stprintf_s( temp, _T("%s %s %s"), uiStr, priceStr, GAMERESOURCEMAN->GetItemToolTipText( tooltipIdx ) );
				mpList->AddColume( row, temp, mDefaultColor );
			}
		}
	}
}

void cNpcStoreWindow::OnNetLock( int lockTry )
{
	if( mpBuy )
		mpBuy->NetLock( lockTry );
}

void cNpcStoreWindow::OnNetUnLock( int lockTry )
{
	if( mpBuy )
		mpBuy->NetUnLock( lockTry );
}

unsigned char cNpcStoreWindow::GetCurrentItemSellType()
{
	if( mTabIndex == ULONG_MAX )
		return UCHAR_MAX;

	/// 데이터 삽입
	cItemNpcStore* store = ITEMSCRIPT->GetItemNpcStore( mNpcIndex );
	if( store )
	{
		/// 아이템 탭 설정
		return store->GetItemSellType( mTabIndex );
	}
	return UCHAR_MAX;
}

unsigned long cNpcStoreWindow::GetCurrentItemPriceRate()
{
	if( mTabIndex == ULONG_MAX )
		return ULONG_MAX;

	/// 데이터 삽입
	cItemNpcStore* store = ITEMSCRIPT->GetItemNpcStore( mNpcIndex );
	if( store )
	{
		/// 아이템 탭 설정
		return store->GetItemSellRate( mTabIndex );
	}

	return ULONG_MAX;
}
