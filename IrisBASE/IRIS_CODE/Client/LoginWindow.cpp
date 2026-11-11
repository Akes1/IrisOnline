#include "stdafx.h"
#include "LoginWindow.h"
#include "LoginUIManager.h"
#include "GameResourceManager.h"

#include "EditBox.h"
#include "Application.h"
#include "UIImage.h"
#include "Label.h"
#include "Button.h"


cLoginWindow::cLoginWindow()
: mpID(0)
, mpPass(0)
, mpLogoImage(0)
, mpLogoSkin(0)
, mSendLogin(false)
{
}

cLoginWindow::~cLoginWindow()
{
	SAFE_DELETE( mpLogoImage );
}

void cLoginWindow::Open()
{
	/// COPY & PASTE 기능 막음.
	if( mpID )
		mpID->SetCopyPaste( false );

	if( mpPass )
	{
		mpPass->SetCopyPaste( false );	
		mpPass->Clear();
	}

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	mSendLogin = false;
}

bool cLoginWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 관리 윈도우 참조..
	mpID = (cEditBox*)GetChild( eUIID_LOGIN_ID_EDIT );
	mpPass = (cEditBox*)GetChild( eUIID_LOGIN_PASS_EDIT );
	mpConnect = (cButton*)GetChild( eUIID_LOGIN_CONNECT );

	if( !mpID || !mpPass || !mpConnect )
		return false;

	/// 로고 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	mpLogoSkin = pSkin->GetNodeSkin( "Login_Logo" );
	if( mpLogoSkin )
	{
		mpLogoImage = new cUIImage( mpLogoSkin->mpTexture );
		unsigned int x = (int)(( GetScreenWidth() - mpLogoSkin->mSkinInfo->mWidth ) * 0.5f);
		unsigned int y = mpLogoSkin->mSkinInfo->mY;
		unsigned int tx = mpLogoSkin->mSkinInfo->mTexX;
		unsigned int ty = mpLogoSkin->mSkinInfo->mTexY;
		unsigned int w = mpLogoSkin->mSkinInfo->mWidth;
		unsigned int h = mpLogoSkin->mSkinInfo->mHeight;
		mpLogoImage->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ) );
		mpLogoImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}
	return true;
}

void cLoginWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpLogoSkin && mpLogoImage )
	{
		/// 화면 중앙에 출력
		unsigned int x = (int)(( GetScreenWidth() - mpLogoSkin->mSkinInfo->mWidth ) * 0.5f);
		unsigned int y = mpLogoSkin->mSkinInfo->mY;
		mpLogoImage->SetPos( x, y );
	}
}

void cLoginWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );
}

void cLoginWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mSendLogin == false )
	{
	
		bool enable = false;

		if( mpID && mpPass )
		{
			// 일반
			if( mpID->IsText() && mpPass->IsText() )
			{
				// 로그인 + 비밀번호
				enable = true;
			}
		}
		
		mpConnect->SetEnabled( enable );
	}
}

void cLoginWindow::OnCommand( cUINode* , unsigned int id )
{
	switch ( id )
	{
	case eUIID_LOGIN_CONNECT:
		{	
			SendLoginMsg();
		}
		break;
	case eUIID_LOGIN_CANCEL:
		{
			/// 프로그램 종료
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	}
}

void cLoginWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpLogoImage )
		mpLogoImage->Draw();
}

void cLoginWindow::OnEditBoxEntered( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_LOGIN_ID_EDIT:
		{
			if( mpPass )
				mpPass->SetFocus();
		}
		break;
	case eUIID_LOGIN_PASS_EDIT:
		{
			/// text 가 존재하면 보낸다 
			if( mpID && mpPass )
			{
				if( mpID->IsText() && mpPass->IsText() )
				{
					SendLoginMsg();
				}
			}
		}
		break;
	}
}

/// 
void cLoginWindow::ClearID( bool focus )
{
	if( mpID )
	{
		mpID->Clear();

		if( focus )
			mpID->SetFocus();
	}
}

/// 
void cLoginWindow::ClearPass( bool focus )
{
	if( mpPass )
	{
		mpPass->Clear();

		if( focus )
			mpPass->SetFocus();
	}
}

void cLoginWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpID )
		mpID->SetFocus();
}

void cLoginWindow::SendLoginMsg()
{
	// 접속 버튼
	if( NETWORK->IsConnected() == false )
	{
		assert(0);
		return;
	}

	if( !mpID || !mpPass || !mpConnect )
	{
		assert(0);
		return;
	}

	mpID->ReleaseFocus();
	mpPass->ReleaseFocus();

	// 일반 로그인
	MSG_IDPASS msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_USER;
	msg.Protocol = NM_USER_LOGIN_REQ;
	ConvertToAscii( mpID->GetText(),   msg.uid, MAX_UID_BUFFER_SIZE );
	ConvertToAscii( mpPass->GetText(), msg.pwd, MAX_PWD_BUFFER_SIZE );

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 인증중 대기창 등록..
	UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
		eMBEVENT_NONE, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 4 ), // 인증 처리 진행
		GAMERESOURCEMAN->GetGameText( 2 ) // 알림
		);

	mSendLogin = true;
	mpConnect->SetEnabled( false );
}

//-------------------------------------------------------------------
cVersionWindow::cVersionWindow()
: mpVersion(0)
{

}

cVersionWindow::~cVersionWindow()
{

}

bool cVersionWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpVersion = (cLabel*)GetChild( eUIID_LOGIN_VERSION );
	
	if( !mpVersion )
	{
		assert(0);
		return false;
	}

	mpVersion->SetText( THEAPP->GetStrVer() );
	mpVersion->SetTextColor( COLOR_WHITE );

	int y = GetScreenHeight() - mAbsoluteRect.GetHeight();
	SetRelativePos( cUIPos(10, y) );
	return true;
}

void cVersionWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	int y = GetScreenHeight() - mAbsoluteRect.GetHeight();
	SetRelativePos( cUIPos(10, y) );
}