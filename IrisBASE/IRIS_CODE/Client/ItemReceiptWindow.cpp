#include "stdafx.h"
#include "ItemReceiptWindow.h"
#include "ItemIcon.h"
#include "ItemManager.h"
#include "DragWindow.h"
#include "GameResourceManager.h"
#include "Application.h"
#include "shellapi.h"

cItemReceiptWindow::cItemReceiptWindow()
: mpGetBtn(0)
, mpCloseBtn(0)
, mSaveResult(0)
, mDelayTime(0)
{

}

cItemReceiptWindow::~cItemReceiptWindow()
{
	ClearTempInven();
}

void cItemReceiptWindow::Close()
{
	ClearTempInven();

	cUIWindow::Close();
}

void cItemReceiptWindow::ClearTempInven()
{
	cTempInvenMap::cIterator i = mTempInvenMap.Begin();
	cTempInvenMap::cIterator end = mTempInvenMap.End();
	for( ; i != end; ++i )
	{
		sCashInventory* p = (sCashInventory*)(i->mSecond);
		SAFE_DELETE( p );
	}
	mTempInvenMap.Clear();
}

void cItemReceiptWindow::Show( bool onsound )
{
	if( mDelayTime != 0 )
		return;

	cUIWindow::Show( onsound );

	ClearTempInven();
	ITEMMAN->ClearCashInven();

	if( ITEMMAN->TryToReceiptList() == true )
	{
		// 딜레이 타임 시작
		mDelayTime = THEAPP->GetWorldAccumTime() + DEAL_DELAYTIME;

		// 대기중 메세지 박스 띄우기
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_ITEMRECEIPT_WAIT, eSKIN_NONE,
			GAMERESOURCEMAN->GetGameText( 1522 ), 
			GAMERESOURCEMAN->GetGameText( 301 )
			);
	}
	else
	{
		// msg box
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 295 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
	}
}

void cItemReceiptWindow::Hide( bool onsound )
{
	if( mDelayTime != 0 )
		return;

	cUIWindow::Hide( onsound );
}

bool cItemReceiptWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpGetBtn = GetChild( eUIID_RECEIPT_GET );
	mpCloseBtn = GetChild( eUIID_RECEIPT_CLOSE );

	// 아이콘 생성
	unsigned short slotIndex = INVENTORY_CASH_ITEM_BEGIN;

	cString str;
	for( unsigned int i = 0; i < RECEIPT_SLOT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "ReceiptItem_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		mIcon[i] = icon;
		ITEMMAN->BindIcon( slotIndex, icon );
	}

	return true;
}

void cItemReceiptWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mDelayTime == 0 )
		return;
	if( IsNetLock() == true )
		return;

	if( mDelayTime <= accumTime )
	{
		mDelayTime = 0;

		// 메세지박스 지우기
		UIMAN->DestroyMsgBox( eMBEVENT_ITEMRECEIPT_WAIT );

		// mSaveResult 결과 처리	
		switch( mSaveResult )
		{
		case ERROR_ITEM_CASH_LIST_SUCCESS:
			{
				// 수령함 초기화
				ITEMMAN->ClearCashInven();

				// 인벤토리 셋팅
				cTempInvenMap::cIterator i = mTempInvenMap.Begin();
				cTempInvenMap::cIterator end = mTempInvenMap.End();
				for( ; i != end; ++i )
				{
					sCashInventory* inv = (sCashInventory*)(i->mSecond);
					if( inv )
						ITEMMAN->NetSetCashItem( inv );
					else
						assert(0);
				}
			}
			break;
		case ERROR_ITEM_CASH_LIST_FAIL:
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 297 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			break;
		case ERROR_ITEM_CASH_LIST_LOADING:
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 298 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			break;
		default:
			assert(0);
			break;
		}

		ClearTempInven();
		mSaveResult = 0;
	}
}

void cItemReceiptWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_RECEIPT_CLOSE:
		Hide();
		break;
	case eUIID_RECEIPT_GET:
		{
			ClearTempInven();
			ITEMMAN->ClearCashInven();

			// 물품 수령
			if( ITEMMAN->TryToReceiptList() == true )
			{
				// 딜레이 타임 시작
				mDelayTime = THEAPP->GetWorldAccumTime() + DEAL_DELAYTIME;

				// 대기중 메세지 박스 띄우기
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_ITEMRECEIPT_WAIT, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 1522 ), 
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else
			{
				// msg box
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 295 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
		}
		break;
	case eUIID_RECEIPT_MALLLINK:
		{
			// 아이템몰링크
			ShellExecute( NULL, L"open", L"http://bill.gpotato.jp/itemshopweb/iris/shop.asp", NULL, NULL, SW_NORMAL );
		}
		break;
	}
}

void cItemReceiptWindow::OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)pcaller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_INVENTORY, icon, pos, shift );
}

// 인벤으로 이동
void cItemReceiptWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}

	unsigned short slotIndex0 = icon->GetSlotIndex();
	unsigned short slotIndex1 = 0;

	if( ITEMMAN->FindEmptySlot( &slotIndex1, INVENTORY_BAG_BEGIN, INVENTORY_BAG_END ) == true )
	{
		ITEMMAN->TryToMoveItem( slotIndex0, slotIndex1, icon );
	}
	else
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 202 ),
							GAMERESOURCEMAN->GetGameText( 301 ) );
	}
}

void cItemReceiptWindow::OnIconClicked( cUINode* /*caller*/, unsigned int /*id*/ )
{
	// 클릭 갱신
}

void cItemReceiptWindow::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cItemReceiptWindow::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}

void cItemReceiptWindow::AddTempInven( sCashInventory* inven )
{
	if( !inven )
	{
		assert(0);
		return;
	}

	sCashInventory* p = new sCashInventory;
	*p = *inven;

	if( mTempInvenMap.Insert( p->idx, p ) == false )
	{
		SAFE_DELETE( p );
		assert(0);
	}
}
