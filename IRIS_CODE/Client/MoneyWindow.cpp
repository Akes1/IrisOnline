#include "stdafx.h"
#include "MoneyWindow.h"

#include "StageManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Button.h"
#include "NumberEditBox.h"
#include "ItemManager.h"

cMoneyWindow::cMoneyWindow()
: mMaxValue( 1000000000 )
, mNumEdit(0)
, mMsgHandler(0)
, mItemSlotIndex(USHRT_MAX)
, mEventType(eMBEVENT_NONE)
, mWindowType(eWINDOW_MONEY)
, mpMax(0)
, mpOk(0)
, mpCancel(0)
, mpModify(0)
, mpDelete(0)
, mSendEvent( true )
{
}

cMoneyWindow::~cMoneyWindow()
{
}

void cMoneyWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mSendEvent && mWindowType == eWINDOW_MODIFY )
		OnCommand( 0, eUIID_MONEY_MODIFY );
}

/// 금액 입력용 윈도우
void cMoneyWindow::ShowMoney( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex )
{
	cUIWindow::ShowModal( eventtype );

	mSendEvent = true;
	mItemSlotIndex = slotIndex;
	mWindowType = eWINDOW_MONEY;

	///
	if( mpOk )
		mpOk->Show();
	if( mpMax )
		mpMax->Show();
	if( mpCancel )
		mpCancel->Show();
	if( mpModify )
		mpModify->Hide();
	if( mpDelete )
		mpDelete->Hide();

	if( type == eMONEY_IN )
	{
		/// 입금 - 최대는 현재 소지금
		mMaxValue = HERO->GetMoney();
	}
	else if( type == eMONEY_OUT )
	{
		/// 출금 - 최대는 창고 보유 금액
		mMaxValue = HERO->GetDeposit();
	}
	else
	{			
		if( mpMax )
			mpMax->Hide();
	}

	if( mMaxValue > MAX_TRADE_MONEY)
		mMaxValue = MAX_TRADE_MONEY;

	if( mNumEdit )
	{
		mNumEdit->SetMaxValue( mMaxValue );
		mNumEdit->SetValue( 0 );
		mNumEdit->SetFocus();
		mNumEdit->SetAllSelect();
	}

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;
}	

/// 금액변경용 윈도우
void cMoneyWindow::ShowMoneyModify( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex )
{
	cUIWindow::ShowModal( eventtype );

	mSendEvent = true;
	mItemSlotIndex = slotIndex;
	mWindowType = eWINDOW_MODIFY;

	if( mpOk )
		mpOk->Hide();
	if( mpMax )
		mpMax->Hide();
	if( mpCancel )
		mpCancel->Hide();
	if( mpModify )
		mpModify->Show();
	if( mpDelete )
		mpDelete->Show();

	if( mNumEdit )
	{
		mNumEdit->SetMaxValue( mMaxValue );
		mNumEdit->SetValue( 0 );
		mNumEdit->SetFocus();
		mNumEdit->SetAllSelect();
	}

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;
}	

///
bool cMoneyWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mNumEdit = (cNumberEditBox*)GetChild( eUIID_MONEY_NUMBER );
	mpMax = (cButton*)GetChild( eUIID_MONEY_MAX );
	mpOk = (cButton*)GetChild( eUIID_MONEY_OK );
	mpCancel = (cButton*)GetChild( eUIID_MONEY_CANCLE );
	mpModify = (cButton*)GetChild( eUIID_MONEY_MODIFY );
	mpDelete = (cButton*)GetChild( eUIID_MONEY_DELETE );
	return true;
}


bool cMoneyWindow::HandleEvent(const cUIEvent& event )
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
	case eUIEVENT_COMMAND:
		if( event.mID == eMESSAGE_BUTTON_NO && mMsgHandler )
		{
			if( mWindowType == eWINDOW_MODIFY )
			{
				mMsgHandler->MsgBoxEventParser( eMBEVENT_USERSELL_ITEMMONEYCANCEL, eUIID_MONEY_CANCLE, 0 );
				mSendEvent = false;
			}
		}
		else
			cUIWindow::HandleEvent( event );
		break;
	case eUIEVENT_EDITBOX_ENTER:
	case eUIEVENT_KEY_UP:
	case eUIEVENT_KEY_DOWN:
		{
			if( event.mCode == eKEY_RETURN )
			{
				if( mWindowType == eWINDOW_MODIFY )
					OnCommand( 0, eUIID_MONEY_MODIFY ); 
				else
					OnCommand( 0, eUIID_MONEY_OK );
			}
			else if( event.mCode == eKEY_ESCAPE )
			{
				OnCommand( 0, eUIID_MONEY_CANCLE );
			}
		}
		break;
	default:
		cUIWindow::HandleEvent( event );
		break;
	}
	return true;
}

void cMoneyWindow::OnCommand( cUINode*, unsigned int id )
{
	if( !mNumEdit )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_MONEY_UP:
		{
			long value = mNumEdit->GetValue();
			value++;

			if( value > (long)mMaxValue )
				value = mMaxValue;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_MONEY_DOWN:
		{
			long value = mNumEdit->GetValue();
			value--;

			if( value < 0 )
				value = 0;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_MONEY_MAX:
		mNumEdit->SetValue( mMaxValue );
		break;
	case eUIID_MONEY_OK:
	case eUIID_MONEY_MODIFY:
	case eUIID_MONEY_DELETE:
		{
			if( mMsgHandler )
				mMsgHandler->MsgBoxEventParser( mEventType, id, 0 );
			
			mSendEvent = false;
			Hide();
		}
		break;
	case eUIID_MONEY_CLOSE:
	case eUIID_MONEY_CANCLE:
		{
			if( mWindowType == eWINDOW_MODIFY )
			{
				if( mMsgHandler )
					mMsgHandler->MsgBoxEventParser( eMBEVENT_USERSELL_ITEMMONEYCANCEL, id, 0 );

				mSendEvent = false;
			}
   
			Hide();
		}
		break;
	default:
		assert(0);
		break;
	}
}

void cMoneyWindow::OnButtonDowned( cUINode* , unsigned int id )
{
	if( !mNumEdit )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_MONEY_UP:
		{
			long value = mNumEdit->GetValue();
			value++;

			if( value > (long)mMaxValue )
				value = mMaxValue;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_MONEY_DOWN:
		{
			long value = mNumEdit->GetValue();
			value--;

			if( value < 0 )
				value = 0;

			mNumEdit->SetValue( value );
		}
		break;
	}
}

void cMoneyWindow::SetMoney( unsigned int value)
{
	if( mNumEdit )
		mNumEdit->SetValue( value );
}

unsigned long cMoneyWindow::GetMoney()
{
	return ( mNumEdit ) ? mNumEdit->GetValue() : 0;
}

///
void cMoneyWindow::OnNetLock( int lockTry )
{
	if( mpOk )
		mpOk->NetLock( lockTry );
}

void cMoneyWindow::OnNetUnLock( int lockTry )
{
	if( mpOk )
		mpOk->NetUnLock( lockTry );
}
