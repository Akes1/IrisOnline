#include "StdAfx.h"
#include "ExitWindow.h"
#include "GameUIManager.h"
#include "Button.h"

#include "label.h"
#include "msgbox.h"
#include "GameResourceManager.h"

#include "ObjectManager.h"
#include "hero.h"

cExitWindow::cExitWindow()
: mpDisConnect(0)
, mpGameOut(0)
, mpTitle(0)
, mpText(0)
{

}
cExitWindow::~cExitWindow()
{
}

void cExitWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

void cExitWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

bool cExitWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpGameOut = (cButton*)GetChild( eUIID_GAME_EXIT_EXIT );
	mpDisConnect = (cButton*)GetChild( eUIID_GAME_EXIT_CHANGE );
	mpTitle = (cLabel*)GetChild( eUIID_GAME_EXIT_WINDOWNAME );
	mpText = (cLabel*)GetChild( eUIID_GAME_EXIT_LOGOUTTEXT );

	if( mpTitle )
		mpTitle->SetText( UIMAN->GetUIText( 211 ) );

	if( mpText )
		mpText->SetText( UIMAN->GetUIText( 115 ) );
	return true;
}

/// 주의사항 : 
void cExitWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_EXIT_CHANGE:
		GAMEUI->ReqFinish( eEXIT_GAMEOUT );
		break;
	case eUIID_GAME_EXIT_EXIT:
		GAMEUI->ReqFinish( eEXIT_DISCONNECT );
		break;
	default:
		assert(0);
		return;
	}

	Hide();
}

void cExitWindow::OnNetLock( int lockTry )
{
	if( mpGameOut )
		mpGameOut->NetLock( lockTry );
	if( mpDisConnect )
		mpDisConnect->NetLock( lockTry );
}

void cExitWindow::OnNetUnLock( int lockTry )
{
	if( mpGameOut )
		mpGameOut->NetUnLock( lockTry );
	if( mpDisConnect )
		mpDisConnect->NetUnLock( lockTry );
}

//////////////////////////////////////////////////////////////////////////
cTutorialExitWindow::cTutorialExitWindow()
: mpTutorialOut(0)
, mpGameOut(0)
, mpTitle(0)
, mpText(0)
{

}

cTutorialExitWindow::~cTutorialExitWindow()
{

}

void cTutorialExitWindow::OnShow()
{
	cUIWindow::OnShow();

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

void cTutorialExitWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

bool cTutorialExitWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpTutorialOut = (cButton*)GetChild( eUIID_GAME_TUTORIALEXIT_TUTORIAL );
	mpGameOut = (cButton*)GetChild( eUIID_GAME_TUTORIALEXIT_GAMEOUT );
	mpTitle = (cLabel*)GetChild( eUIID_GAME_TUTORIALEXIT_WINDOWNAME );
	mpText = (cLabel*)GetChild( eUIID_GAME_TUTORIALEXIT_LOGOUTTEXT );

	if( mpTitle )
		mpTitle->SetText( UIMAN->GetUIText( 170 ) );

	if( mpText )
		mpText->SetText( UIMAN->GetUIText( 171 ) );
	return true;
}

/// 
void cTutorialExitWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TUTORIALEXIT_TUTORIAL:
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_TUTORIAL_EXIT, eSKIN_YESNO,
								UIMAN->GetUIText( 172 ),
								UIMAN->GetUIText( 170 )
								);
		}
		break;
	case eUIID_GAME_TUTORIALEXIT_GAMEOUT:
		GAMEUI->ShowExitWindow();
		break;
	default:
		assert(0);
		return;
	}

	Hide();
}

void cTutorialExitWindow::OnNetLock( int lockTry )
{
	if( mpTutorialOut )
		mpTutorialOut->NetLock( lockTry );
	if( mpGameOut )
		mpGameOut->NetLock( lockTry );
}

void cTutorialExitWindow::OnNetUnLock( int lockTry )
{
	if( mpTutorialOut )
		mpTutorialOut->NetUnLock( lockTry );
	if( mpGameOut )
		mpGameOut->NetUnLock( lockTry );
}


//////////////////////////////////////////////////////////////////////////

cExitMsgWindow::cExitMsgWindow()
:mpCaption(0)
, mpMsgText(0)
, mpCancelBtn(0)
, mExitEventType(eEXIT_NONE)
{
	mDelayTime = 0;
	mStartCount = false;
}

cExitMsgWindow::~cExitMsgWindow()
{

}

void cExitMsgWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

bool cExitMsgWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	// 결투 요청
	mpCaption = (cLabel*)GetChild( eUIID_EXITMSG_CAPTION );
	mpMsgText = (cLabel*)GetChild( eUIID_EXITMSG_TEXT );
	mpCancelBtn = (cButton*)GetChild( eUIID_EXITMSG_CANCEL );
	if( mpCaption )
		mpCaption->SetTextColor( COLOR_WHITE );

	if( mpMsgText )
		mpMsgText->SetTextColor( COLOR_WHITE );

	return true;
}

void cExitMsgWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;
	if( mStartCount == false )
		return;

	if( mDelayTime <= deltaTime+500 )
	{
		mDelayTime = 0;
		mStartCount = false;

		if( mpCancelBtn )
			mpCancelBtn->SetEnabled( false );

		/// 종료 처리
		switch( mExitEventType )
		{
		case eEXIT_DISCONNECT:
			GAMEUI->SendDisConnect();
			break;
		case eEXIT_GAMEOUT:
			GAMEUI->SendGameOut();
			break;
		default:
			assert(0);
			break;
		}
	}
	else
	{
		mDelayTime -= deltaTime;

	}

	if( mpMsgText )
	{
		cStringT Message;

		unsigned long textIdx = 0;
		switch( mExitEventType )
		{
		case eEXIT_DISCONNECT:		textIdx = 104;			break;
		case eEXIT_GAMEOUT:			textIdx = 106;			break;
		default:					assert(0);				break;
		}

		Message.Format( UIMAN->GetUIText( textIdx ), unsigned int(mDelayTime*0.001f) );
		mpMsgText->SetText( (LPCTSTR)Message.Cstr() );
	}
}

// 게임 종료요청중 윈도우가 닫히지 않도록 한다
void cExitMsgWindow::HideESC( bool onsound )
{
	if( IsNetLock() == false )
		cUIWindow::HideESC( onsound );
}

void cExitMsgWindow::ShowExitMsg( int type )
{
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );

	mExitEventType = type;
	mDelayTime = 5500;
	mStartCount = false;

	if( mpCancelBtn )
		mpCancelBtn->SetEnabled( true );

	if( mpCaption )
	{
		unsigned long textIdx = 0;
		switch( mExitEventType )
		{
		case eEXIT_DISCONNECT:		textIdx = 103;			break;
		case eEXIT_GAMEOUT:			textIdx = 105;			break;
		default:					assert(0);				break;
		}

		mpCaption->SetText( UIMAN->GetUIText( textIdx ) );
	}

	if( mpMsgText )
	{
		cStringT Message;

		unsigned long textIdx = 0;
		switch( mExitEventType )
		{
		case eEXIT_DISCONNECT:		textIdx = 104;			break;
		case eEXIT_GAMEOUT:			textIdx = 106;			break;
		default:					assert(0);				break;
		}

		Message.Format( UIMAN->GetUIText( textIdx ), unsigned int(mDelayTime*0.001f) );
		mpMsgText->SetText( (LPCTSTR)Message.Cstr() );
	}

	ShowModal( eMBEVENT_NONE );
}

void cExitMsgWindow::OnCommand( cUINode* node, unsigned int id )
{
	switch( id )
	{
	case eUIID_EXITMSG_CANCEL:
		{
			/// 종료 요청 취소
			MSGROOT Msg;
			Msg.Category = NM_PLAYER;
			Msg.Protocol = NM_PLAYER_GAMEFINISH_CANCEL_REQ;
			NETWORK->SendNetworkMsg( (char*)&Msg, sizeof(Msg) );

			///
			mStartCount = 0;
			Hide();

			HERO->SetState( eOBJECT_STATE_IDLE );
		}
		break;
	}
}

bool cExitMsgWindow::HandleEvent( const cUIEvent& event )
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
	default:
		cUIWindow::HandleEvent( event );
		break;
	}
	return true;
}

void cExitMsgWindow::OnNetLock( int lockTry )
{
	if( mpCancelBtn )
		mpCancelBtn->NetLock( lockTry );
}

void cExitMsgWindow::OnNetUnLock( int lockTry )
{
	if( mpCancelBtn )
		mpCancelBtn->NetUnLock( lockTry );
}