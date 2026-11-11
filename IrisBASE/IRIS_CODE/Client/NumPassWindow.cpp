#include "stdafx.h"
#include "LoginWindow.h"
#include "NumPassWindow.h"
#include "LoginUIManager.h"

#include "EditBox.h"
#include "Button.h"
#include "Application.h"
#include "GameResourceManager.h"

cNumPassWindow::cNumPassWindow()
:mpPass( 0 )
, mpConnect(0)
, mSendLogin( false )
{
	for( unsigned short idx = 0; idx < 10; ++idx )
		mpNumBtn[idx] = 0;
}

cNumPassWindow::~cNumPassWindow()
{
}

void cNumPassWindow::Open()
{
	mSendLogin = false;

	/// 화면 중앙에 출력
	/*int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );

	cUIWindow* win = LOGINUI->GetLoginWindow();
	if( win )
	{
		int x = (int)(( GetScreenWidth() - win->GetAbsoluteRect().GetWidth() ) * 0.5f);
		win->SetRelativePos( cUIPos( x, y + GetAbsoluteRect().GetHeight() ) );
	}*/

	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	/// COPY & PASTE 기능 막음.
	if( mpPass )
	{
		mpPass->SetCopyPaste( false );
		mpPass->Clear();
		SetAllNumEnable( true );
	}

	if( mpConnect )
		mpConnect->SetEnabled( false );

	MixNumberPos();

#ifdef _DEBUG
	mpPass->SetText(L"0000");
	SetAllNumEnable( false );
#endif
}

bool cNumPassWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 관리 윈도우 참조..
	mpPass = (cEditBox*)GetChild( eUIID_LOGIN_NUMPASS_PASS_EDIT );
	mpConnect = (cButton*)GetChild( eUIID_LOGIN_NUMPASS_CONNECT );

	for( unsigned short idx = 0; idx < 10; ++idx )
	{
		mpNumBtn[idx] = (cButton*)GetChild( eUIID_LOGIN_NUMPASS_0_BUTTON + idx );
	}

	if( mpPass )
		mpPass->SetEnabled( false );

	if( mpConnect )
		mpConnect->SetEnabled( false );
	return true;
}

void cNumPassWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	cUIWindow* win = LOGINUI->GetLoginWindow();
	if( win )
	{
		/// 화면 중앙에 출력
		int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = win->GetRelativeRect().mBottom;

		SetRelativePos( cUIPos( x, y ) );
	}
}

void cNumPassWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mpConnect && mpPass )
	{
		bool enable = false;

		if( mSendLogin == false )
		{
			long authcode = THEAPP->GetAuthCode();
			int len = mpPass->GetLength();
			if( authcode != -1 )
			{
				// 웹런쳐
				// 4자리 수 입력
				enable = ( len == 4 );
			}
			else
			{
				cLoginWindow* win = LOGINUI->GetLoginWindow();
				if( win )
				{
					// 일반
					if( len == 4 && win->IsExistID() && win->IsExistPass() )
					{
						// 4자리 수 입력 + 비밀번호 + 로그인
						enable = true;
					}
				}
			}
		}

		//
		mpConnect->SetEnabled( enable );
	}
}

void cNumPassWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_LOGIN_NUMPASS_0_BUTTON:
	case eUIID_LOGIN_NUMPASS_1_BUTTON:
	case eUIID_LOGIN_NUMPASS_2_BUTTON:
	case eUIID_LOGIN_NUMPASS_3_BUTTON:
	case eUIID_LOGIN_NUMPASS_4_BUTTON:
	case eUIID_LOGIN_NUMPASS_5_BUTTON:
	case eUIID_LOGIN_NUMPASS_6_BUTTON:
	case eUIID_LOGIN_NUMPASS_7_BUTTON:
	case eUIID_LOGIN_NUMPASS_8_BUTTON:
	case eUIID_LOGIN_NUMPASS_9_BUTTON:
		{
			if( !mpPass )
			{
				assert(0);
				return;
			}

			int num = 0;
			num = id - eUIID_LOGIN_NUMPASS_0_BUTTON;

			charT    text[2] ={0,};
			cStringT str;
			errno_t  error;

			str += mpPass->GetText();
			if( str.GetLength() > 3 )
				break;

			error = ::_itot_s( num, text, sizeof(charT), 10 );
			if( error )
			{
				assert( 0 && "fail to convert int to char" );
				break;
			}
			str += text;
			mpPass->SetText( str.Cstr() );

			/// 4자리 숫자를 다 채우면 번호들 enable 시키기
			if( str.GetLength() == 4 )
				SetAllNumEnable( false );
		}
		break;

	case eUIID_LOGIN_NUMPASS_CLEAR_BUTTON:
		{
			mpPass->Clear();
			SetAllNumEnable( true );
		}
		break;
		
	case eUIID_LOGIN_NUMPASS_CONNECT:
		{	
			// 접속 버튼
			if( NETWORK->IsConnected() == false )
			{
				assert(0);
				return;
			}

			if( !mpPass )
			{
				assert(0);
				return;
			}

			LPCTSTR passNum = mpPass->GetText();
			if( !passNum )
			{
				assert( 0 && "fail to get pin num" );
				return;
			}

			if( THEAPP->GetAuthCode() != -1 )
			{
				// 웹런쳐을 통한 로그인
				MSG_REQ_SECURITY_NUM msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category = NM_USER;
				msg.Protocol = NM_USER_SECURITY_NUM_REQ;
				ConvertToAscii( passNum, msg.sec, MAX_SEC_BUFFER_SIZE );
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 인증중 대기창 등록..
				UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 4 ), // 인증 처리 진행
					GAMERESOURCEMAN->GetGameText( 2 ) // 알림
					);
			}
			else
			{
				cLoginWindow* win = LOGINUI->GetLoginWindow();
				if( !win )
				{
					assert(0);
					return;
				}

				cEditBox* idEdit = win->GetIDEdit();
				cEditBox* passEdit = win->GetPassEdit();

				if( !idEdit || !passEdit )
				{
					assert(0);
					return;
				}

				idEdit->ReleaseFocus();
				passEdit->ReleaseFocus();

				// 일반 로그인
				MSG_IDPASS msg;
				::memset( &msg, 0, sizeof(msg) );

				msg.Category = NM_USER;
				msg.Protocol = NM_USER_LOGIN_REQ;
				ConvertToAscii( idEdit->GetText(),   msg.uid, MAX_UID_BUFFER_SIZE );
				ConvertToAscii( passEdit->GetText(), msg.pwd, MAX_PWD_BUFFER_SIZE );
				ConvertToAscii( passNum,             msg.sec, MAX_SEC_BUFFER_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				/// 인증중 대기창 등록..
				UIMAN->CreateMsgBox( eStage_Login, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_NONE,
									GAMERESOURCEMAN->GetGameText( 4 ), // 인증 처리 진행
									GAMERESOURCEMAN->GetGameText( 2 ) // 알림
									);

				mSendLogin = true;
			}
		}
		break;
	case eUIID_LOGIN_NUMPASS_CANCEL:
		{
			/// 프로그램 종료
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	}
	return;
}

/// 
void cNumPassWindow::ClearPass( bool focus )
{
	if( mpPass )
	{
		mpPass->Clear();
		SetAllNumEnable( true );
	}
}

void cNumPassWindow::MixNumberPos()
{
	srand( (unsigned)time( NULL ) );
	for( unsigned int cnt = 0; cnt < 10; ++cnt )
	{
		int swapPos;
		swapPos = rand()%10;
		
		if( mpNumBtn[cnt] == 0 || mpNumBtn[swapPos] == 0 )
		{
			assert( 0 && "Bad pointer" );
			return;
		}

		cUIRect rect1, rect2;
		rect1 = mpNumBtn[cnt]->GetRelativeRect();
		rect2 = mpNumBtn[swapPos]->GetRelativeRect();
		
		mpNumBtn[cnt]->SetRelativePos( cUIPos( rect2.mLeft, rect2.mTop ) );
		mpNumBtn[swapPos]->SetRelativePos( cUIPos( rect1.mLeft, rect1.mTop ) );

	}
}

void cNumPassWindow::SetAllNumEnable( bool enable )
{
	for( unsigned short idx = 0; idx < 10; ++idx )
	{
		if( mpNumBtn[idx] )
			mpNumBtn[idx]->SetEnabled( enable );
	}
}
