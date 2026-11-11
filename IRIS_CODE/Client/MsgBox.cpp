#include "StdAfx.h"
#include "MsgBox.h"
#include "Button.h"
#include "Label.h"
#include "EditBox.h"
#include "UISkinLexer.h"
#include "Parser.h"
#include "FontAgent.h"
#include "Application.h"
#include "SoundSystem.h"

cMsgBox::cMsgBox( eUINodeType type )
: cUIWindow( type )
, mpYesButton(0)
, mpNoButton(0)
, mpEdit(0)
, mpLabel(0)
, mSkinType(eSKIN_NONE)
, mExtraData(0)
, mCaptionPos(0, 0)
, mStartTime(0)
, mTotalTime(0)
, mTimePrint(false)
, mTimeResult(false)
, mEnableLength(0)
{
	SetBackupVisible( false );

	mModalType = eMSGBOX_MODAL;
}

cMsgBox::~cMsgBox()
{
	if( mSkinType == eSKIN_EDIT )
		ReleaseFocus();

	mExtraData = 0;
	mpCaller = 0;
}

void cMsgBox::Show( bool onsound )
{
	if( mVisible )
		return;

	if( onsound && mOpenSoundIdx != ULONG_MAX )
		SOUNDSYS->Play2DSound( mOpenSoundIdx );

	SetVisible( true );

	OnShow();
}

void cMsgBox::OnShow()
{
	if( mSkinType == eSKIN_EDIT )
		mpEdit->SetFocus();

	cUIWindow::OnShow();
}

void cMsgBox::HideESC( bool onsound )
{
	if( mSkinType == eSKIN_OK )
	{
		if( mMsgHandler )
			mMsgHandler->MsgBoxEventParser( mEventType, eMESSAGE_BUTTON_NO, mExtraData );

		UIMAN->DestroyMsgBox( this );
	}
}

bool cMsgBox::OnCreate( cUINodeProperty* )
{
	/// 스킨타입에 따라 버튼 생성여부를 결정한다.
	switch( mSkinType )
	{
	case eSKIN_OK:
		{
			mpYesButton = new cButton;
			if( mpYesButton->CreateBySkinName( mOkButtonSkin, this, eMESSAGE_BUTTON_YES ) == false )
				return false;
		}
		break;
	case eSKIN_YESNO:
		{
			mpYesButton = new cButton;
			mpNoButton = new cButton;
			if( mpYesButton->CreateBySkinName( mYesButtonSkin, this, eMESSAGE_BUTTON_YES ) == false )
				return false;

			if( mpNoButton->CreateBySkinName( mNoButtonSkin, this, eMESSAGE_BUTTON_NO ) == false )
				return false;
		}
		break;
	case eSKIN_CANCEL:
		{
			mpNoButton = new cButton;
			if( mpNoButton->CreateBySkinName( mNoButtonSkin, this, eMESSAGE_BUTTON_YES ) == false )
				return false;
		}
		break;
	case eSKIN_EDIT:
		{
			mpYesButton = new cButton;
			mpNoButton = new cButton;
			mpEdit = new cEditBox;
			if( mpYesButton->CreateBySkinName( mYesButtonSkin, this, eMESSAGE_BUTTON_YES ) == false )
				return false;

			if( mpNoButton->CreateBySkinName( mNoButtonSkin, this, eMESSAGE_BUTTON_NO ) == false )
				return false;

			if( mpEdit->CreateBySkinName( mEditSkin, this, eMESSAGE_EDIT ) == false )
				return false;

			if( mEnableLength > 0 )
				mpYesButton->SetEnabled( false );

			if( mpEdit )
				mpEdit->SetCopyPaste( false );

		}
		break;
	}

	mpLabel = new cLabel;
	if( mpLabel->CreateBySkinName( "eMB_LABEL", this, eMESSAGE_LABEL ) == false )
	{
		assert( 0 && "failed to create label" );
		return false;
	}

	if( mpLabel )
		mpLabel->SetEnabled( false );

	SetExceptSnap( true );
	return true;
}

bool cMsgBox::Create( cBaseStage* msgHandler, eMSGBox_Type boxtype, eMSGBox_Event eventtype, eBoxSkin_Type skintype, void* extraData )
{
	mEventType = eventtype;
	mSkinType  = skintype;
	mModalType = boxtype;

	mExtraData = extraData;
	mMsgHandler = msgHandler;

	//읽어들일 스킨 결정 
	cString skinname;
	switch( skintype )
	{
	case eSKIN_NONE:	skinname = "eMB_NONE";	break;
	case eSKIN_OK:		skinname = "eMB_OK";	break;
	case eSKIN_YESNO:	skinname = "eMB_YESNO";	break;
	case eSKIN_CANCEL:	skinname = "eMB_Cancel";break;
	case eSKIN_GAUGE:	break;
	case eSKIN_EDIT:	skinname = "eMB_EDIT";	break;
	}

	/// 메시지 박스 생성 ( 부모는 루트윈도우 )
	if( cUINode::CreateBySkinName( skinname, 0, 1 ) == false )
		return false;

	if( IsValid() == false )
		return false;

	SetVisible( false );

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
	return true;
}

void cMsgBox::SetTime( unsigned long totaltime, bool printTime, bool success )
{
	/// 시간정보 세팅
	mStartTime = THEAPP->GetWorldAccumTime();
	mTotalTime = totaltime;

	mTimeResult = success;
	mTimePrint = printTime;
}

/// 주의사항 : 
bool cMsgBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_MESSAGEBOX ) == false )
	{
		assert( 0 && "not messagebox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	cMessageBoxSkin* p = (cMessageBoxSkin*)pskin;

	mCaptionRect = p->mCaptionRect; 

	if( mCaptionRect.GetWidth() < 0 || mCaptionRect.GetHeight() < 0 )
	{
		assert( 0 && "caption rect is 0" );
		return false;
	}

	mOkButtonSkin = p->mOkButtonSkin;
	mYesButtonSkin = p->mYesButtonSkin;
	mNoButtonSkin = p->mNoButtonSkin;
	mEditSkin = p->mEditSkin;
	return true;
}

bool cMsgBox::HandleEvent( const cUIEvent& event )
{
	if( mVisible == false || mEnabled == false )
		return false;

	switch( event.mType )
	{
	case eUIEVENT_LBUTTON_DOWN:
	case eUIEVENT_LBUTTON_UP:
	case eUIEVENT_MOUSE_MOVE:
		{
			cUINode* pNode = GetNode( event.mPos );
			if( pNode )
			{
				if( pNode->GetType() == eUINODE_BUTTON || pNode->GetType() == eUINODE_EDITBOX )
				{
					pNode->HandleEvent( event );
				}
			}
		}
		break;
	case eUIEVENT_EDITBOX_ENTER:
		{
			/// 엔터키를 누르면 확인되었음을 적용
			if( mMsgHandler )
				mMsgHandler->MsgBoxEventParser( mEventType, eMESSAGE_BUTTON_YES, mExtraData );

			UIMAN->DestroyMsgBox( this );
			UIMAN->ReleaseFocusedNode( mpEdit );
		}	
		break;
	case eUIEVENT_COMMAND:
		{
			if( mMsgHandler )
				mMsgHandler->MsgBoxEventParser( mEventType, event.mID, mExtraData );

			// 이벤트 처리후 삭제
			UIMAN->DestroyMsgBox( this );
		}
		break;
	case eUIEVENT_EDITBOX_CHAR_UPDATED:
		if( mpYesButton && mpEdit )
		{
			bool enabled = false;
			unsigned int length = GetLength();
			if( length >= mEnableLength )
			{
				enabled = true;
			}
			else if( length == mEnableLength - 1 && mpEdit->IsComposition() )
			{
				enabled = true;
			}

			mpYesButton->SetEnabled( enabled );
		}
		break;
	}

	return cUIWindow::HandleEvent( event );
}

void cMsgBox::UpdateRect()
{
	cUIWindow::UpdateRect();

	//
	UpdateCaption();
}

void cMsgBox::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

void cMsgBox::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mTotalTime <= 0 )
		return;

	/// 시간 체크
	if( accumTime - mStartTime > mTotalTime )
	{
		/// 자동 거절
		cUIEvent event;
		event.mType = eUIEVENT_COMMAND;
		event.mID = (mTimeResult)? eMESSAGE_BUTTON_YES : eMESSAGE_AUTONO;
		HandleEvent( event );

		mStartTime = accumTime;
	}
	else if( mTimePrint && mText.IsEmpty() == false )
	{
		unsigned long elased = (mTotalTime - (accumTime - mStartTime))/1000;
		unsigned int s = elased % 60;

		cStringT str;
		str.Format( (LPCTSTR)mText.Cstr(), s );
		ChangeText( (LPCTSTR)str.Cstr() );
	}
}

void cMsgBox::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	//
	pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mCaptionText.Cstr()), mCaptionPos.mX, mCaptionPos.mY, COLOR_WHITE, mOutLine );

	pKeeper->DrawAll();
}

void cMsgBox::OnKeyUp( eKeyCode code )
{
	/// 확인용 메시지 박스면, 엔터키를 누르면 자동으로 확인키가 눌린다
	if( mSkinType == eSKIN_OK && (code == eKEY_RETURN || code == eKEY_ESCAPE ) )
	{
		if( mMsgHandler )
			mMsgHandler->MsgBoxEventParser( mEventType, (mTimeResult) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO, mExtraData );

		UIMAN->DestroyMsgBox( this );
	}
}


void cMsgBox::OnKeyDown( eKeyCode code )
{
	/// 확인용 메시지 박스면, 엔터키를 누르면 자동으로 확인키가 눌린다
	if( mSkinType == eSKIN_OK && (code == eKEY_RETURN || code == eKEY_ESCAPE ) )
	{
		if( mMsgHandler )
			mMsgHandler->MsgBoxEventParser( mEventType, (mTimeResult) ? eMESSAGE_BUTTON_YES : eMESSAGE_BUTTON_NO, mExtraData );

		UIMAN->DestroyMsgBox( this );
	}
}

/// 주의사항 : 텍스트 좌표 업데이트 
void cMsgBox::UpdateCaption()
{
	/// 텍스트가 뿌려질 위치 계산 
	cUIRect rc = GetAbsoluteRect();
	
	/// 캡션텍스트 위치 조정 
	mCaptionPos.mX = rc.mLeft + (rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI_Bold, mCaptionText.Cstr(), mCaptionText.GetLength() ) ) / 2;
	mCaptionPos.mY = rc.mTop + ( mCaptionRect.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI_Bold) ) / 2;
}

void cMsgBox::SetText( LPCTSTR text, LPCTSTR caption )
{
	mCaptionText = caption;
	mText = text;

	if( mpLabel )
		mpLabel->SetText( text );

	UpdateCaption();
}

void cMsgBox::ChangeText( LPCTSTR text )
{
	if( mpLabel )
		mpLabel->SetText( text );

	UpdateCaption();
}

void cMsgBox::SetEnableLength( unsigned int length )
{
	if( length > 0 )
	{
		mEnableLength = length;
		
		if( mpYesButton )
			mpYesButton->SetEnabled( false );
	}
}

void cMsgBox::SetMaxLength( int length )
{
	if( mpEdit )
		mpEdit->SetMaxEditLength( length );
}

LPCTSTR cMsgBox::GetEditText()
{
	return ( mpEdit ) ? mpEdit->GetText() : 0;
}

///
unsigned int cMsgBox::GetLength()
{
	return ( mpEdit ) ? mpEdit->GetLength() : 0;
}

//////////////////////////////////////////////////////////////////////
cMessageBoxSkin::cMessageBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mMoveable( true )
{
	mCaptionRect.Set( 0, 0, 0, 0 );
}

cMessageBoxSkin::~cMessageBoxSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cMessageBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	/// 그다음토큰을 계속 검사
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_CAPTIONXY:
			{
				mCaptionRect.mLeft = parser.ParseInt();
				mCaptionRect.mTop = parser.ParseInt();
			}
			break;
		case eTOKEN_CAPTIONWH:
			{
				mCaptionRect.mRight = mCaptionRect.mLeft + parser.ParseInt();
				mCaptionRect.mBottom = mCaptionRect.mTop + parser.ParseInt();
			}
			break;
		case eTOKEN_EDITSKIN:
			{
				mEditSkin = parser.ParseString();
			}
			break;
		case eTOKEN_OKBUTTONSKIN:
			{
				mOkButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_YESBUTTONSKIN:
			{
				mYesButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_NOBUTTONSKIN:
			{
				mNoButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_MOVABLE:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mMoveable = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mMoveable = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}


