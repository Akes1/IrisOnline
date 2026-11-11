#include "stdafx.h"
#include "TradeWindow.h"
#include "DragWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameUIManager.h"
#include "TipWindow.h"
#include "MoneyWindow.h"
#include "PushButton.h"
#include "Label.h"
#include "NumberEditBox.h"
#include "ItemIcon.h"
#include "Application.h"

cTradeWindow::cTradeWindow()
: mTradeButton(0)
, mpName(0)
, mpOtherName(0)
, mMyLastSlot(0)
, mThatLastSlot(0)
, mpMyMoney(0)
, mpThatMoney(0)
, mpMyOk(0)
, mpThatOk(0)
, mStatus( ItemExchangeBegin )
, mThatStatus( ItemExchangeBegin )
, mOKAccumTime(0)
{
	memset( mMyItem, 0, sizeof(mMyItem) );
}

cTradeWindow::~cTradeWindow()
{
}

void cTradeWindow::Clear()
{
	/// 자기 슬롯 초기화
	for( unsigned int i = 0; i < MAX_ITEM_EXCHANGE; ++i )
	{
		cItemIcon* icon = mMyItem[i];
		if( icon )
		{
			/// 락 풀기
			unsigned short itemSlotIndex = icon->GetSlotIndex();
			if( itemSlotIndex <= MAX_INVENTORY )
			{
				cItem* invItem = ITEMMAN->GetItem( itemSlotIndex );
				if( invItem )
					invItem->SetLock( false );

				icon->SetSlotIndex( USHRT_MAX );
			}

			icon->ChangeImage( 0 );
			icon->SetItemIndex( 0 );
			icon->SetNumber( 0 );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->ShowNumber( false );
			icon->SetSeal( false );
		}
	}

	mMyLastSlot = 0;

	/// 상대방 슬롯 초기화
	for( unsigned int i = 0; i < MAX_ITEM_EXCHANGE; ++i )
	{
		sThatInvetory& thatInv = mThatItem[i];

		/// 인벤정보 초기화
		sInventory& inv = thatInv.mThatInv;
		inv.idx			= 0;
		inv.ItemIndex	= 0;
		inv.number		= 0;
		inv.count		= 0;
		inv.enhanced	= 0;
		inv.seal		= 0;
		inv.cash		= 0;

		/// 아이콘 초기화
		cItemIcon* icon = thatInv.mThatIcon;
		if( icon )
		{
			icon->ChangeImage( 0 );
			icon->SetItemIndex( 0 );
			icon->SetNumber( 0 );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->ShowNumber( false );
			icon->SetSeal( false );
		}
	}

	mThatLastSlot = 0;

	SetStatus( ItemExchangeBegin, true );
	SetStatus( ItemExchangeBegin, false );

	AddMyMoney( 0 );
	AddThatMoney( 0 );

	mOKAccumTime = 0;
	if( mpMyOk )
	{
		mpMyOk->SetEnabled( true );
		mpMyOk->SetPress( false );
	}
}

void cTradeWindow::Open()
{
}

void cTradeWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cTradeWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();

	cUIWindow* win = (cUIWindow*)GAMEUI->GetInventoryWindow();
	if( win )
		win->Hide();

	UIMAN->RestoreShow();
}

void cTradeWindow::HideESC( bool onsound )
{
	/// 닫기 요청
	ITEMMAN->TryToTradeCancel();
}

bool cTradeWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpName = (cLabel*)GetChild( eUIID_TRADE_NAME );
	mpOtherName = (cLabel*)GetChild( eUIID_TRADE_THATNAME );

	mTradeButton = (cButton*)GetChild( eUIID_TRADE_TRADE );
	mpMyMoney = (cLabel*)GetChild( eUIID_TRADE_MONEY );
	mpThatMoney = (cLabel*)GetChild( eUIID_TRADE_THATMONEY );
	mpMyOk = (cPushButton*)GetChild( eUIID_TRADE_OK );
	mpThatOk = (cPushButton*)GetChild( eUIID_TRADE_THATOK );

	if( mTradeButton )
		mTradeButton->SetEnabled( false );

	if( mpThatOk )
		mpThatOk->SetEnabled( false );

	AddMyMoney( 0 );
	AddThatMoney( 0 );

	/// 내 등록 아이콘 생성
	cString str;
	for( unsigned int i = 0; i < MAX_ITEM_EXCHANGE; ++i )
	{
		str.Format( "Trade_MyIcon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		mMyItem[i] = icon;
	}

	/// 상대방 등록 아이콘 생성
	for( unsigned int i = 0; i < MAX_ITEM_EXCHANGE; ++i )
	{
		str.Format( "Trade_ThatIcon%02d", i );
		cItemIcon* icon = new cItemIcon;;
		if( icon->CreateBySkinName( str, this, i+100 ) == false )
			return false;

		/// hover 이벤트는 처리하지만 이동 불가 
		icon->SetUsedFlag( false, false );
		icon->SetLinked( true );

		mThatItem[i].mThatIcon = icon;
	}

	/// 초기화
	SetStatus( ItemExchangeBegin, true );
	SetStatus( ItemExchangeBegin, false );
	return true;
}

void cTradeWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mVisible )
	{
		GAMEUI->UpdateInvenWithUI( (cUIWindow*)this, false );
	}
}


void cTradeWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mOKAccumTime != 0 && mOKAccumTime < accumTime )
	{
		if( mpMyOk )
		{
			if( mStatus == ItemExchangeBegin )
			{
				mpMyOk->SetEnabled( true );
				mpMyOk->SetPress( false );
			}
			else if( mStatus == ItemExchangeOk )
			{
				mpMyOk->SetEnabled( true );
				mpMyOk->SetPress( true );
			}
		}

		mOKAccumTime = 0;
	}
}


void cTradeWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	cUIWindow::OnRender( pKeeper );
}

void cTradeWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	if( id == eUIID_TRADE_CLOSE )
	{
		/// 닫기 요청
		ITEMMAN->TryToTradeCancel();
	}
	else if( id == eUIID_TRADE_TRADE )
	{
		/// 거래 완료 요청
		if( mStatus == ItemExchangeOk && mThatStatus == ItemExchangeOk )
			ITEMMAN->TryToTradeEnd();
	}
	else if( id == eUIID_TRADE_MONEYBUTTON )
	{
		/// 금액 입력버튼
		if( mStatus == ItemExchangeBegin )
			GAMEUI->ShowMoneyWindow( eMONEY_IN, eMBEVENT_TRADE_MONEYIN );
	}
}

void cTradeWindow::OnPushButtonPressed( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_TRADE_OK:
		{
			if( mOKAccumTime > THEAPP->GetWorldAccumTime() )
				return;

			/// 물품확인요청
			if( mStatus == ItemExchangeBegin )
			{
				mpMyOk->SetPress( false );
				ITEMMAN->TryToTradeOk();
			}
			/// 물품 수정요청
			else if( mStatus == ItemExchangeOk )
			{
				mpMyOk->SetPress( true );
				ITEMMAN->TryToTradeReTry();
			}
		}
		break;
	}
}

void cTradeWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_TRADE, icon, pos, shift );
}

void cTradeWindow::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller;
	
	sThatInvetory* thatInv = mThatItem;
	for( int i = 0; i < MAX_ITEM_EXCHANGE; ++i, ++thatInv )
	{
		if( hoverIcon == thatInv->mThatIcon )
		{
			UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, thatInv->mThatInv );
			return;
		}
	}
}

/// 거래자명 세팅
void cTradeWindow::SetNames( LPCTSTR otherName )
{
	if( mpName )
		mpName->SetText( HERO->GetName() );

	if( mpOtherName )
		mpOtherName->SetText( otherName );
}

bool cTradeWindow::AddMyItem( unsigned short slot )
{
	if( slot > MAX_INVENTORY )
		return false;

	/// 아이템 등록
	cItem* item = ITEMMAN->GetItem( slot );
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
		
	if( mMyLastSlot >= MAX_ITEM_EXCHANGE )
	{
		assert(0);
		return false;
	}

	if( !mMyItem[mMyLastSlot] )
	{
		assert(0);
		return false;
	}

	/// 락걸기 - 인벤토리 내, 참조된 아이템 
	item->SetLock( true );

	/// 참조한 슬롯인덱스 저장
	mMyItem[mMyLastSlot]->SetSlotIndex( slot );
	mMyItem[mMyLastSlot]->SetItemIndex( item->GetIndex() );
	mMyItem[mMyLastSlot]->ChangeImage( define->GetIconParam() );
	mMyItem[mMyLastSlot]->SetTipType( define->GetTipType() );
	mMyItem[mMyLastSlot]->SetNumber( item->GetCount() );
	mMyItem[mMyLastSlot]->SetSeal( item->IsSeal() );

	bool show = ( define->GetCapacity() > 1 ) ? true : false;
	mMyItem[mMyLastSlot]->ShowNumber( show );
	
	mMyLastSlot++;

	/// 아이템 추가 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slot );

	return true;
}

bool cTradeWindow::DeleteMyItem( unsigned short arrIndex )
{
	/// 정해진 슬롯 아니면 리턴
	if( arrIndex >= MAX_ITEM_EXCHANGE )
	{
		assert( 0 );
		return false;
	}

	/// 
	if( arrIndex >= mMyLastSlot )
	{
		assert(0);
		return false;
	}

	if( !mMyItem[arrIndex] )
	{
		assert(0);
		return false;
	}

	unsigned short itemSlotIndex = mMyItem[arrIndex]->GetSlotIndex();
	if( itemSlotIndex > MAX_INVENTORY )
		return false;

	/// 락걸린 아이템 락풀기
	cItem* invItem = ITEMMAN->GetItem( itemSlotIndex );
	if( invItem )
	{
		invItem->SetLock( false );
	}

	/// 슬롯에서 삭제
	mMyItem[arrIndex]->SetSlotIndex( USHRT_MAX );
	mMyItem[arrIndex]->ChangeImage( 0 );
	mMyItem[arrIndex]->SetItemIndex( 0 );
	mMyItem[arrIndex]->SetTipType( eTOOLTIP_NONE );
	mMyItem[arrIndex]->SetNumber( 0 );
	mMyItem[arrIndex]->SetSeal( false );

	UpdateMyData( arrIndex );

	if( mMyLastSlot > 0 )
		mMyLastSlot--;

	return true;
}

/// 데이터 땡기기
void cTradeWindow::UpdateMyData( unsigned short index )
{
	for( unsigned short i = index; i < mMyLastSlot-1; ++i )
	{
		cItemIcon* icon1 = mMyItem[i];
		cItemIcon* icon2 = mMyItem[i+1];

		if( icon1 && icon2 )
		{
			icon1->SetSlotIndex( icon2->GetSlotIndex() );
			icon1->ChangeImage( icon2->GetIconParam() );
			icon1->SetItemIndex( icon2->GetItemIndex() );
			icon1->SetTipType( icon2->GetTipType() );
			icon1->SetNumber( icon2->GetNumber() );
			icon1->ShowNumber( icon2->IsShowNumber() );
			icon1->SetSeal( icon2->IsSeal() );
		}
		else
			assert(0);
	}

	/// 마지막 아이템 초기화
	cItemIcon* lastIcon = mMyItem[mMyLastSlot-1];
	if( lastIcon )
	{
		lastIcon->SetSlotIndex( USHRT_MAX );
		lastIcon->ChangeImage( 0 );
		lastIcon->SetTipType( eTOOLTIP_NONE );
		lastIcon->SetItemIndex( 0 );
		lastIcon->SetNumber( 0 );
		lastIcon->SetSeal( false );
	}
}

/// 상대방 거래아이템 등록
bool cTradeWindow::AddThatItem( sInventory* inv )
{
	if( mThatLastSlot >= MAX_ITEM_EXCHANGE )
	{
		assert(0);
		return false;
	}
	
	cItemDefine* define = ITEMMAN->GetItemDefine( inv->ItemIndex );
	if( !define )
	{
		assert(0);
		return false;
	}

	/// 아이템 삽입
	sThatInvetory* thatInv = &mThatItem[mThatLastSlot];
	thatInv->mThatInv = *inv;

	/// 아이콘 세팅
	cItemIcon* icon = thatInv->mThatIcon;
	if( icon )
	{
		icon->ChangeImage( define->GetIconParam() );
		icon->SetTipType( define->GetTipType() );
		icon->SetNumber( inv->count );
		icon->SetSeal( (inv->seal==1) );

		bool show = ( define->GetCapacity() > 1 ) ? true : false;
		icon->ShowNumber( show );
	}
	
	mThatLastSlot++;
	return true;
}

/// 상대방 거래아이템 삭제
bool cTradeWindow::DeleteThatItem( sInventory* inv )
{
	if( inv->ItemIndex == 0 )
		return false;

	/// 툴팁 닫기
	UIMAN->HideTip();

	sThatInvetory* thatItem = mThatItem;
	for( unsigned short i = 0; i < mThatLastSlot; ++i, ++thatItem )
	{
		sInventory& thatInv = thatItem->mThatInv;	
		if( thatInv.number == inv->number )
		{
			/// 동일한 슬롯 이면, 삭제
			thatInv.idx			= 0;
			thatInv.ItemIndex	= 0;
			thatInv.number		= 0;
			thatInv.count		= 0;
			thatInv.enhanced	= 0;
			thatInv.seal		= 0;

			/// 아이콘 초기화
			cItemIcon* icon = thatItem->mThatIcon;
			if( icon )
			{
				icon->ChangeImage( 0 );
				icon->SetItemIndex( 0 );
				icon->SetTipType( eTOOLTIP_NONE );
				icon->SetNumber( 0 );
				icon->ShowNumber( false );
				icon->SetSeal( false );
			}
			
			UpdateThatData( i );

			if( mThatLastSlot > 0 )
				mThatLastSlot--;
			return true;
		}
	}
	return true;
}

/// 땡기기 
void cTradeWindow::UpdateThatData( unsigned short index )
{
	sThatInvetory* data1 = mThatItem + index;
	sThatInvetory* data2 = 0;

	for( unsigned short i = index; i < mThatLastSlot-1; ++i, ++data1 )
	{
		data2 = data1 + 1;
		data1->mThatInv = data2->mThatInv;

		/// 이미지 업데이트
		cItemIcon* icon = data1->mThatIcon;
		cItemIcon* nextIcon = data2->mThatIcon;
		if( icon && nextIcon )
		{
			icon->ChangeImage( nextIcon->GetIconParam() );
			icon->SetTipType( nextIcon->GetTipType() );
			icon->SetNumber( nextIcon->GetNumber() );
			icon->ShowNumber( nextIcon->IsShowNumber() );
			icon->SetSeal( nextIcon->IsSeal() );
			icon->SetUsedFlag( false, false );
		}
	}

	/// 마지막 아이템 초기화
	sThatInvetory& thatInv = mThatItem[mThatLastSlot-1];
	cItemIcon* icon = thatInv.mThatIcon;
	if( icon )
	{
		icon->ChangeImage( 0 );
		icon->SetItemIndex( 0 );
		icon->SetNumber( 0 );
		icon->ShowNumber( false );
		icon->SetSeal( false );
	}

	sInventory& inv = thatInv.mThatInv;
	inv.idx			= 0;
	inv.ItemIndex	= 0;
	inv.number		= 0;
	inv.count		= 0;
	inv.enhanced	= 0;
	inv.seal		= 0;
}

void cTradeWindow::AddMyMoney( unsigned long money )
{
	if( mpMyMoney )
		mpMyMoney->SetText( cNumberEditBox::FormatNumber( money ) );
}

void cTradeWindow::AddThatMoney( unsigned long money )
{
	if( mpThatMoney )
		mpThatMoney->SetText( cNumberEditBox::FormatNumber( money ) );
}

/// 각 거래 상태 세팅
void cTradeWindow::SetStatus( eItemExchangeStatus status, bool myTurn )
{
	switch( status )
	{
	case ItemExchangeBegin:
		if( myTurn )
		{
			mStatus = status;

			mpMyOk->SetEnabled( true );
			mpMyOk->SetPress( false );

			mTradeButton->SetEnabled( false );
			mTradeButton->SetPress( false );

			/// 예외처리
			if( mThatStatus == ItemExchangeEnd )
			{
				SetStatus( ItemExchangeOk, false );
			}

			mOKAccumTime = THEAPP->GetWorldAccumTime() + 1000;
			mpMyOk->SetEnabled( false );
			mpMyOk->SetPress( false );
		}
		else
		{
			mThatStatus = status;

			mpThatOk->SetEnabled( false );
			mpThatOk->SetPress( false );

			mTradeButton->SetEnabled( false );
			mTradeButton->SetPress( false );

			/// 예외처리
			if( mStatus == ItemExchangeEnd )
			{
				SetStatus( ItemExchangeOk, true );
			}
		}
		break;
	case ItemExchangeOk:
		if( myTurn )
		{
			mStatus = status;

			mpMyOk->SetEnabled( true );
			mpMyOk->SetPress( true );

			mTradeButton->SetEnabled( false );
			mTradeButton->SetPress( false );

			if( mThatStatus == ItemExchangeOk )
			{
				mTradeButton->SetEnabled( true );
				mTradeButton->SetPress( false );
			}

			mOKAccumTime = THEAPP->GetWorldAccumTime() + 1000;
			mpMyOk->SetEnabled( false );
			mpMyOk->SetPress( true );
		}
		else
		{
			mThatStatus = status;

			mpThatOk->SetEnabled( false );
			mpThatOk->SetPress( true );

			if( mStatus == ItemExchangeOk )
			{
				mTradeButton->SetEnabled( true );
				mTradeButton->SetPress( false );
			}
		}
		break;
	case ItemExchangeEnd:
		if( myTurn )
		{
			mStatus = status;

			mpMyOk->SetEnabled( false );
			mpMyOk->SetPress( true );

			mTradeButton->SetEnabled( false );
			mTradeButton->SetPress( true );
		}	
		else
		{
			mThatStatus = status;

			mpThatOk->SetEnabled( false );
			mpThatOk->SetPress( true );
		}
		break;
	}
}

unsigned short cTradeWindow::GetMySlotIndex( int i )
{
	if( i >= MAX_ITEM_EXCHANGE )
		return USHRT_MAX;

	return ( mMyItem[i] ) ? mMyItem[i]->GetSlotIndex() : USHRT_MAX;
}

bool cTradeWindow::IsNetLock()
{
	if( mpMyOk && mpMyOk->IsNetLock() == true )
		return true;
	return false;
}

void cTradeWindow::OnNetLock( int lockTry )
{
	if( mpMyOk )
		mpMyOk->NetLock( lockTry );
}

void cTradeWindow::OnNetUnLock( int lockTry )
{
	if( mpMyOk )
		mpMyOk->NetUnLock( lockTry );
}

unsigned long cTradeWindow::GetMyTotalWeight()
{
	unsigned long totalWeight = 0;
	for( unsigned int i = 0; i < MAX_ITEM_EXCHANGE; ++i )
	{
		unsigned long itemIndex = mMyItem[i]->GetItemIndex();
		if( itemIndex > 0 )
		{
			unsigned long itemSlotIndex = mMyItem[i]->GetSlotIndex();
			if( itemSlotIndex <= MAX_INVENTORY )
			{
				cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
				if( !define )
				{
					assert(0);
					continue;
				}
				totalWeight += ( define->GetWeight() * mMyItem[i]->GetNumber() );
			}
		}
	}

	return totalWeight;
}
