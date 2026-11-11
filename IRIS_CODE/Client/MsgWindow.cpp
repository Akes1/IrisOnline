#include "stdafx.h"
#include "MsgWindow.h"

#include "BaseStage.h"
#include "UIMsgboxEvent.h"

cMsgWindow::cMsgWindow()
: mEventType( eMBEVENT_NONE )
, mMsgHandler( 0 )
, mAnsType( MB_OK )
{
}

cMsgWindow::~cMsgWindow()
{
}

bool cMsgWindow::Create( unsigned int eventType, cMsgBoxHandler* msgHandler, const cString& skinPrefix, cUIContainer* parent, unsigned int id )
{
	mEventType = eventType;
	mMsgHandler = msgHandler;
	mSkinPrefix = skinPrefix;

	return cUIWindow::Create( skinPrefix + "_Window", parent, id );
}

void cMsgWindow::ShowModal( unsigned int ansType )
{
	switch( ansType )
	{
	case MB_OK:
		{
			mOk_OkButton.Show();
			mOkCancel_OkButton.Hide();
			mOkCancel_CancelButton.Hide();
		}
		break;
	case MB_OKCANCEL:
		{
			mOk_OkButton.Hide();
			mOkCancel_OkButton.Show();
			mOkCancel_CancelButton.Show();
		}
		break;
	default:
		assert( 0 && "invalid msg window type" );
	}

	mAnsType = ansType;
	cUIWindow::ShowModal();
}

bool cMsgWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
	{
		return false;
	}

	/// 자식 컨트롤들을 생성
	if( mCaptionLabel.Create( mSkinPrefix + "_Caption", this, 1 ) == false )
	{
		assert( 0 && "failed to create msg caption" );
		return false;
	}
	if( mTextLabel.Create( mSkinPrefix + "_Text", this, 1 ) == false )
	{
		assert( 0 && "failed to create msg text" );
		return false;
	}
	if( mOk_OkButton.Create( "eMB_OK_ok", this, eUIID_OK ) == false )
		return false;
	if( mOkCancel_OkButton.Create( "eMB_YESNO_yes", this, eUIID_OK ) == false )
		return false;
	if( mOkCancel_CancelButton.Create( "eMB_YESNO_no", this, eUIID_CANCEL ) == false )
		return false;
	return true;
}

void cMsgWindow::OnProcess( unsigned long accumTime )
{
	cUIWindow::OnProcess( accumTime );
}

void cMsgWindow::OnOk()
{
	cUIWindow::OnOk();

	if( mMsgHandler )
		mMsgHandler->MsgBoxEventParser( mEventType, eMESSAGE_BUTTON_OK );
}

void cMsgWindow::OnCancel()
{
	cUIWindow::OnCancel();

	if( mMsgHandler )
		mMsgHandler->MsgBoxEventParser( mEventType, eMESSAGE_BUTTON_CANCEL );
}
