#include "stdafx.h"
#include "ItemMsgWindow.h"

#include "ItemManager.h"
#include "GameResourceManager.h"
#include "StageManager.h"
#include "Button.h"
#include "Label.h"
#include "NumberEditBox.h"
#include "GameUIManager.h"
#include "NpcStoreWindow.h"

cItemMsgWindow::cItemMsgWindow()
: mpOk(0)
, mpYes(0)
, mpCancel(0)
, mItemIndex(0)
, mSlotIndex0(USHRT_MAX)
, mSlotIndex1(USHRT_MAX)
, mCount(0)
, mpCaption(0)
, mpText(0)
{
}

cItemMsgWindow::~cItemMsgWindow()
{
}

void cItemMsgWindow::Open()
{
}

void cItemMsgWindow::Close()
{
	cUIWindow::Close();

	if( mpText )
		mpText->Clear();

	mItemIndex = 0;
	mSlotIndex0 = USHRT_MAX;
	mSlotIndex1 = USHRT_MAX;
	mCount = 0;
}

void cItemMsgWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpText )
		mpText->Clear();

	mItemIndex = 0;
	mSlotIndex0 = USHRT_MAX;
	mSlotIndex1 = USHRT_MAX;
	mCount = 0;
}

bool cItemMsgWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpOk = (cButton*)GetChild( eUIID_ITEMMSG_OK );
	mpYes = (cButton*)GetChild( eUIID_ITEMMSG_YES );
	mpCancel = (cButton*)GetChild( eUIID_ITEMMSG_CANCEL );
	mpCaption = (cLabel*)GetChild( eUIID_ITEMMSG_CAPTION );
	mpText = (cLabel*)GetChild( eUIID_ITEMMSG_TEXT );

	if( mpCaption )
		mpCaption->SetTextColor( COLOR_WHITE );

	if( mpText )
		mpText->SetTextColor( COLOR_WHITE );
	return true;
}

bool cItemMsgWindow::HandleEvent(const cUIEvent& event )
{	
	if( mEnabled == false || mVisible == false )
		return mpParent->HandleEvent( event );

	switch( event.mType )
	{
	case eUIEVENT_LBUTTON_DOWN:
	case eUIEVENT_LBUTTON_UP:
	case eUIEVENT_MOUSE_MOVE:
		{
			cUINode* pNode = this->GetNode( event.mPos );
			if( pNode && (pNode->GetType() == eUINODE_BUTTON || pNode->GetType() == eUINODE_EDITBOX ))
				pNode->HandleEvent( event );
			else
				cUIWindow::HandleEvent( event );
		}
		break;
	case eUIEVENT_KEY_UP:
	case eUIEVENT_KEY_DOWN:
		if( event.mCode == eKEY_RETURN || event.mCode == eKEY_ESCAPE )
		{
			if( mpOk && mpOk->IsVisible() == true )
				Hide();
		}
		break;
	case eUIEVENT_COMMAND:
		if( event.mID == eMESSAGE_BUTTON_NO && mMsgHandler )
		{
			if( mpOk && mpOk->IsVisible() == true )
				mMsgHandler->MsgBoxEventParser( mEventType, eUIID_ITEMMSG_OK, 0 );
			else
				mMsgHandler->MsgBoxEventParser( mEventType, eUIID_ITEMMSG_CANCEL, 0 );
		}
		else
			cUIWindow::HandleEvent( event );
		break;
	default:
		cUIWindow::HandleEvent( event );
		break;
	}
	return true;
}

void cItemMsgWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_ITEMMSG_OK:
	case eUIID_ITEMMSG_YES:
	case eUIID_ITEMMSG_CANCEL:
		if( mMsgHandler )
			mMsgHandler->MsgBoxEventParser( mEventType, id, 0 );

		Hide();
		break;
	}
}

void cItemMsgWindow::ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, 
							    unsigned int count, unsigned long price )
{
	cUIWindow::ShowModal( eventtype );

	if( mpOk )
		mpOk->Hide();
	if( mpYes )
		mpYes->Show();
	if( mpCancel )
		mpCancel->Show();

	cStringT str;
	unsigned long itemPrice = 0;
	switch( eventtype )
	{
	case eMBEVENT_ITEM_SELL:
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
				{
					/// 판매불가, 미인증 아이템 상점 판매 체크
					if( itemDefine->GetSellType() == ItemSellNone || itemDefine->GetLicenseType() != 0 )
					{
						str.Format( GAMERESOURCEMAN->GetGameText( 204 ) );

						if( mpOk )
							mpOk->Show();
						if( mpYes )
							mpYes->Hide();
						if( mpCancel )
							mpCancel->Hide();
					}
					else
					{
						LPCTSTR name = itemDefine->GetName();
						unsigned long unitIndex = ITEMMAN->GetPriceUnit( itemDefine->GetSellType() );
						itemPrice = count * itemDefine->GetSellPrice();
						cStringT priceStr = cNumberEditBox::FormatNumber( itemPrice );
						cStringT countStr = cNumberEditBox::FormatNumber( count );

						str.Format( GAMERESOURCEMAN->GetGameText( 206 ), name, (LPCTSTR)countStr.Cstr(), (LPCTSTR)priceStr.Cstr(), GAMERESOURCEMAN->GetItemToolTipText( unitIndex ) );
					}
				}
			}
		}
		break;
	case eMBEVENT_ITEM_DUMP:
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
					str.Format( GAMERESOURCEMAN->GetGameText( 214 ), itemDefine->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_BUY:
		{
			/// 아이템 인덱스 이용
			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
			if( itemDefine )
			{
				LPCTSTR name = itemDefine->GetName();
				unsigned long unitIndex = 2000091; 
				unsigned long sellRate	= 100;
				cNpcStoreWindow* win = GAMEUI->GetNpcStore();
				if( win )
				{
					unsigned char npcSellType = win->GetCurrentItemSellType();
					if( npcSellType != UCHAR_MAX )
						unitIndex = ITEMMAN->GetPriceUnit( npcSellType, itemIndex );
					else
						assert(0);

					sellRate = win->GetCurrentItemPriceRate();
					if( sellRate == ULONG_MAX )
					{	
						assert(0);
						sellRate = 100;
					}
				}
				else
					assert(0);

				itemPrice = count * ( itemDefine->GetBuyPrice() * sellRate / 100 );
				cStringT priceStr = cNumberEditBox::FormatNumber( itemPrice );
				cStringT countStr = cNumberEditBox::FormatNumber( count );

				str.Format( GAMERESOURCEMAN->GetGameText( 203 ), name, (LPCTSTR)countStr.Cstr(), (LPCTSTR)priceStr.Cstr(), GAMERESOURCEMAN->GetItemToolTipText( unitIndex ) );		
			}
		}
		break;
	case eMBEVENT_ITEM_USERSELLBUY:
		{
			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
			if( itemDefine )
			{
				LPCTSTR name = itemDefine->GetName();
				itemPrice = count * price;
				cStringT priceStr = cNumberEditBox::FormatNumber( itemPrice );
				cStringT countStr = cNumberEditBox::FormatNumber( count );

				str.Format( GAMERESOURCEMAN->GetGameText( 216 ), name, (LPCTSTR)countStr.Cstr(), (LPCTSTR)priceStr.Cstr() );		
			}
		}
		break;
	case eMBEVENT_ITEM_NPCDEALBUY:
		{
			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
			if( itemDefine )
			{
				LPCTSTR name = itemDefine->GetName();
				itemPrice = count * price;
				cStringT priceStr = cNumberEditBox::FormatNumber( itemPrice );
				cStringT countStr = cNumberEditBox::FormatNumber( count );

				str.Format( GAMERESOURCEMAN->GetGameText( 216 ), name, (LPCTSTR)countStr.Cstr(), (LPCTSTR)priceStr.Cstr() );		
			}
		}
		break;
	default:
		assert(0);
		return;
	}

	if( mpText )
		mpText->SetText( (LPCTSTR)str.Cstr() );

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;
	mSlotIndex0 = slotIndex;
	mSlotIndex1 = 0;
	mItemIndex = itemIndex;
	mCount = count;
}

void cItemMsgWindow::ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex0, unsigned long slotIndex1, unsigned long itemIndex )
{
	cUIWindow::ShowModal( eventtype );

	if( mpOk )
		mpOk->Hide();
	if( mpYes )
		mpYes->Show();
	if( mpCancel )
		mpCancel->Show();

	cStringT str;
	switch( eventtype )
	{
	case eMBEVENT_ITEM_PUTCARD_NEW:	/// 새로운 카드 장착
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
					str.Format( GAMERESOURCEMAN->GetGameText( 2601 ), itemDefine->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_PUTCARD_UPDATE:	/// 기존 카드 덮어쓰기
		{
			cItem* item0 = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item0 )
			{
				cItemDefine* itemDefine0 = item0->GetDefine();
				cItemDefine* itemDefine1 = ITEMMAN->GetItemDefine( itemIndex );
				if( itemDefine0 && itemDefine1 )
					str.Format( GAMERESOURCEMAN->GetGameText( 2602 ), itemDefine0->GetName(), itemDefine1->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_TAROTNEW:
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
					str.Format( GAMERESOURCEMAN->GetGameText( 340 ), itemDefine->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_TAROTRETURN:
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
					str.Format( GAMERESOURCEMAN->GetGameText( 341 ), itemDefine->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_SPREADNEW:
		{
			cItem* item = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item )
			{
				cItemDefine* itemDefine = item->GetDefine();
				if( itemDefine )
					str.Format( GAMERESOURCEMAN->GetGameText( 344 ), itemDefine->GetName() );
			}
		}
		break;
	case eMBEVENT_ITEM_CARDBOOK_SPREADUPDATE:
		{
			cItem* item0 = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
			if( item0 )
			{
				cItemDefine* itemDefine0 = item0->GetDefine();

				cItemDefine* itemDefine1 = ITEMMAN->GetItemDefine( itemIndex );
				if( itemDefine0 && itemDefine1 )
					str.Format( GAMERESOURCEMAN->GetGameText( 345 ), itemDefine0->GetName(), itemDefine1->GetName() );

			}
		}
		break;
	default:
		assert(0);
		return;
	}

	if( mpText )
		mpText->SetText( (LPCTSTR)str.Cstr() );

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;
	mSlotIndex0 = slotIndex0;
	mSlotIndex1 = slotIndex1;
	mItemIndex = itemIndex;
}
