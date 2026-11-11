#include "stdafx.h"
#include "DuelMsgWindow.h"
#include "Label.h"
#include "GameResourceManager.h"
#include "Duel_Common.h"
#include "ChatManager.h"
#include "Application.h"
#include "ObjectManager.h"
#include "Hero.h"

cDuelMsgWindow::cDuelMsgWindow()
: mRangeCheck( DUEL_REQ_RANGE )
, mDuelTargetIdx( 0 )
, mpMsgText( 0 )
{
	mWindowCloseTime = 0;
	mLeftTime = 0;
}

cDuelMsgWindow::~cDuelMsgWindow()
{
}

void cDuelMsgWindow::ShowModal()
{
	cUIWindow::ShowModal( eMBEVENT_NONE );
}

void cDuelMsgWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);

	SetRelativePos( cUIPos( x, y) );
}

bool cDuelMsgWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	// 결투 요청
	cLabel* pLabel = (cLabel*)GetChild( eUIID_GAME_DUELMSG_CAPTION );
	if( pLabel )
	{
		pLabel->SetTextColor( COLOR_WHITE );
		pLabel->SetText( GAMERESOURCEMAN->GetGameText( 801 ) );
	}

	mpMsgText = (cLabel*)GetChild( eUIID_GAME_DUELMSG_TEXT );
	return true;
}

void cDuelMsgWindow::OnShow()
{
	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.35f);
	SetRelativePos( cUIPos( x, y) );

	/// 자동 창 닫히는 시간 설정
	mWindowCloseTime = THEAPP->GetWorldAccumTime() + WINDOW_CLOSE_TIME;

	/// 대상 이름 출력
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mDuelTargetIdx );
	if( pPlayer == NULL )
		return;

	mLeftTime = WINDOW_CLOSE_TIME / 1000;

	cStringT Message;
	Message.Format( GAMERESOURCEMAN->GetGameText( 802 ), pPlayer->GetName(), mLeftTime );

	// 결투 요청 메시지
	if( mpMsgText )
	{
		mpMsgText->SetTextColor( COLOR_WHITE );
		mpMsgText->SetText( (LPCTSTR)Message.Cstr() );
	}
}


bool cDuelMsgWindow::HandleEvent(const cUIEvent& event )
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
			cUIWindow::HandleEvent( event );
		}
		break;
	case eUIEVENT_COMMAND:
		if( event.mID == eMESSAGE_BUTTON_NO )
		{	
			ButtonSelect( eUIID_GAME_DUELMSG_CANCEL );
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

void cDuelMsgWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	ButtonSelect( id );
	Hide();
}

void cDuelMsgWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	/// 자식들 업데이트 
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	if( mWindowCloseTime <= accumTime )
	{
		ButtonSelect( eUIID_GAME_DUELMSG_CANCEL );
		Hide();
		return;
	}

	/// 타겟 체크
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mDuelTargetIdx );
	if( pPlayer == NULL )
	{
		ButtonSelect( eUIID_GAME_DUELMSG_CANCEL );
		Hide();
		return;
	}

	if( mRangeCheck.IsNotRange( pPlayer->GetPos(), HERO->GetPos() ) == true )
	{
		ButtonSelect( eUIID_GAME_DUELMSG_CANCEL );
		Hide();
		return;
	}

	unsigned long leftTime = ( mWindowCloseTime - accumTime + 1000 ) / 1000;
	if( mLeftTime != leftTime )
	{
		mLeftTime = leftTime; 

		cStringT Message;
		Message.Format( GAMERESOURCEMAN->GetGameText( 802 ), pPlayer->GetName(), mLeftTime );

		// 결투 요청 메시지
		if( mpMsgText )
		{
			mpMsgText->SetTextColor( COLOR_WHITE );
			mpMsgText->SetText( (LPCTSTR)Message.Cstr() );
		}
	}

}

void cDuelMsgWindow::ButtonSelect( unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_DUELMSG_YES:
		{
			/// 결투 수락
			MSG_REQ_DUEL_ACCEPT msg;
			msg.Category = NM_DUEL;
			msg.Protocol = NM_DUEL_ACCEPT_REQ;
			msg.mIsAccept = true;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
		}
		break;
	case eUIID_GAME_DUELMSG_CANCEL:
		{
			/// 결투 거부
			MSG_REQ_DUEL_ACCEPT msg;
			msg.Category = NM_DUEL;
			msg.Protocol = NM_DUEL_ACCEPT_REQ;
			msg.mIsAccept = false;

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 803 ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );
		}
		break;
	}
}