#include "StdAfx.h"
#include "RevivalWindow.h"
#include "Button.h"
#include "Label.h"
#include "GameResourceManager.h"
#include "StageManager.h"
#include "ObjectManager.h"
#include "GameUIManager.h"
#include "Hero.h"

cRevivalWindow::cRevivalWindow()
: mpTownButton(0)
, mpStandButton(0)
, mpTime(0)
, mRestTime(0)
, mCheckTime(0)
{
}

cRevivalWindow::~cRevivalWindow()
{

}

void cRevivalWindow::Open()
{
}

void cRevivalWindow::Close()
{
	cUIWindow::Close();
	SetTimer( false );
}

void cRevivalWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	/// 타임 설정
	mCheckTime = ULONG_MAX;
	mRestTime = REVIVAL_TIME;

	if( mpTime )
	{
		cStringT tm;
		unsigned int m = ((unsigned int)mRestTime/1000 ) / 60;
		tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
		mpTime->SetText( tm.Cstr() );
	}

	/// 테마 인던 사망시 제자리 부활 비활성화
	unsigned long idx = STAGEMAN->GetMapIdx();
	if( idx >=MAP_THEME_MIN  && idx<= MAP_THEME_MAX )
		mpStandButton->SetEnabled( false );
	else
		mpStandButton->SetEnabled( true );
}

void cRevivalWindow::OnHide()
{
	cUIWindow::OnHide();
	SetTimer( false );
}

void cRevivalWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);

	SetRelativePos( cUIPos( x, y) );
}

bool cRevivalWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpTime = (cLabel*)GetChild( eUIID_GAME_REVIVAL_TIME );
	mpTownButton = (cButton*)GetChild( eUIID_GAME_REVIVAL_TOWN );
	mpStandButton = (cButton*)GetChild( eUIID_GAME_REVIVAL_STAND );
	
	if( mpTime )
		mpTime->SetTextColor( COLOR_RED );
	return true;
}

void cRevivalWindow::OnCommand(cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_REVIVAL_TOWN:
		{
			/// 마을 부활
			MSG_REQ_PLAYER_RESURRECTION msg;
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
			msg.mFlag = eRESURRECTIONTYPE_SAFE;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			/// 락 걸기
			NetLock();
		}
		break;
	case eUIID_GAME_REVIVAL_STAND:
		{
			/// 10랩 이하는 바로 부활 요청
			if ( HERO->GetLevel() < 10 )
			{
				cUIWindow* win = GAMEUI->GetReVivalWindow();
				if( win )
					win->NetLock();

				/// 제자리부활
				MSG_REQ_PLAYER_RESURRECTION msg;
				msg.Category = NM_PLAYER;
				msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
				msg.mFlag = eRESURRECTIONTYPE_STAND;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			else
			{
				/// 제자리부활
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_REVIVAL_STAND, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 901 ),	// 
					GAMERESOURCEMAN->GetGameText( 301 )	// 알림
					);
			}
		}
		break;
	default:
		assert(0);
		break;
	}
}

void cRevivalWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mCheckTime == ULONG_MAX )
		mCheckTime = accumTime;

	unsigned long delta = accumTime-mCheckTime;

	if( mRestTime  <= delta )
	{
		/// 혹시 제자리부활. 스킬부활 떠있으면 닫기
		UIMAN->SendEventToMsgbox( eMBEVENT_REVIVAL_STAND, false );
		UIMAN->SendEventToMsgbox( eMBEVENT_REVIVAL_SKILL, false );

		/// 마을 부활 패킷
		MSG_REQ_PLAYER_RESURRECTION msg;
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_RESURRECTION_REQ;
		msg.mFlag = eRESURRECTIONTYPE_SAFE;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 락 걸기
		NetLock();

		Hide();
		return;
	}
	else
	{
		mRestTime -= delta;
		mCheckTime = accumTime;
	}

	unsigned int m = ((unsigned int)mRestTime/1000 ) / 60;
	unsigned int s = ((unsigned int)(mRestTime/1000) % 60) % 60;

	/// 반올림 출력
	if( m != 0 )
		m++;

	/// 분단위로만 표시 ( 1분이하는 초단위 )
	cStringT tm,ts;
	if( m > 0 )
		tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000809 ), m );
	else
		ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), s );

	tm += ts;

	if( mpTime )
		mpTime->SetText( tm.Cstr() );
}

void cRevivalWindow::OnNetLock( int lockTry )
{
	if( mpTownButton )
		mpTownButton->NetLock( lockTry );
	if( mpStandButton )
		mpStandButton->NetLock( lockTry );
}

void cRevivalWindow::OnNetUnLock( int lockTry )
{
	if( mpTownButton )
		mpTownButton->NetUnLock( lockTry );
	if( mpStandButton )
		mpStandButton->NetUnLock( lockTry );
}
