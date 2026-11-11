#include "StdAfx.h"
#include "CamaelWindow.h"

#include "Player_Common.h"
#include "Protocol.h"

#include "Application.h"
#include "RenderSystem.h"

#include "ChatManager.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"

#include "Button.h"
#include "EditBox.h"
#include "Label.h"
#include "UIImage.h"

cCamaelWindow::cCamaelWindow(void)
: mpEdit(0)
, mpTextImg(0) 
, mpOK(0)
, mpRefresh(0)
, mpTime(0)
, mpImageSkin(0)
, mAnswerTime(0) 
, mIsCheckTime( false )
, mIsOverRefresh( false )
{
}

cCamaelWindow::~cCamaelWindow(void)
{
	SAFE_DELETE( mpTextImg );
}

bool cCamaelWindow::OnCreate( cUINodeProperty *property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpEdit	  = (cEditBox*)GetChild( eUIID_GAME_CAMAEL_EDIT );	
	mpOK	  = (cButton*)GetChild( eUIID_GAME_CAMAEL_OK );
	mpRefresh = (cButton*)GetChild( eUIID_GAME_CAMAEL_REFRESH );
	mpTime	  = (cLabel*)GetChild( eUIID_GAME_CAMAEL_LIMIT_TIME );
	if( mpTime )
		mpTime->SetTextColor( COLOR_RED );

	
	/// 스킬 아이콘 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	mpImageSkin = pSkin->GetNodeSkin( "Camael_TestImage" );
	if( !mpImageSkin )
	{
		assert(0);
		return false;
	}

	return true;
}

void cCamaelWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpTextImg )
	{
		cUIRect captchaRect;
		captchaRect.mLeft   = mAbsoluteRect.mLeft + mpImageSkin->mSkinInfo->mX;
		captchaRect.mTop    = mAbsoluteRect.mTop  + mpImageSkin->mSkinInfo->mY;
		captchaRect.mRight  = captchaRect.mLeft + mpImageSkin->mSkinInfo->mWidth;
		captchaRect.mBottom = captchaRect.mTop  + mpImageSkin->mSkinInfo->mHeight;

		mpTextImg->SetScreenRect( captchaRect );
	}
}

void cCamaelWindow::OnCommand( cUINode* , unsigned int id )
{
	switch ( id )
	{
	case eUIID_GAME_CAMAEL_OK:
		{
			/// 텍스트 글 수 체크
			if( mpEdit )
			{
				if( mpEdit->GetLength() != MAX_TRIAL_ANSWER_LEN )
				{		
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11808 ) );
					break;
				}

				MSG_REQ_PLAYER_TRIAL_ANSWER msg;
				::memset( &msg, 0, sizeof(MSG_REQ_PLAYER_TRIAL_ANSWER) );
				msg.Category = NM_PLAYER;
				msg.Protocol = NM_PLAYER_TRIAL_ANSWER_REQ;
				Sstrncpy( msg.answer, MAX_TRIAL_ANSWER_LEN+1, mpEdit->GetText(), mpEdit->GetLength() );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_PLAYER_TRIAL_ANSWER ) );

				SetRefreshButtonEnable( false );
				mpEdit->Clear();
			}
		}
		break;
	case eUIID_GAME_CAMAEL_REFRESH:
		{
			if( mVisible )
			{
				MSGROOT msg;
				msg.Category = NM_PLAYER;
				msg.Protocol = NM_PLAYER_TRIAL_CAPTCHA_REQ;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSGROOT ) );

				SetRefreshButtonEnable( false );
				mpEdit->Clear();
			}
		}
		break;
	}
}

bool cCamaelWindow::HandleEvent( const cUIEvent& event )
{
	if( mVisible == false || mEnabled == false )
		return false;

	if( event.mType == eUIEVENT_EDITBOX_CHAR_UPDATED )
	{
		if( mpOK && mpEdit )
		{
			bool enabled = false;
			unsigned int length = mpEdit->GetLength();
			if( length >= CAMAEL_TEXT_LENGTH )
			{
				enabled = true;
			}
			else if( length == CAMAEL_TEXT_LENGTH - 1 && mpEdit->IsComposition() )
			{
				enabled = true;
			}

			mpOK->SetEnabled( enabled );
		}
	}
	else if( event.mType == eUIEVENT_EDITBOX_ENTER && event.mCode == eKEY_ESCAPE )
		return true;
	
	return cUIWindow::HandleEvent( event );
}

void cCamaelWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mIsCheckTime )
	{
		mAnswerTime += deltaTime;

		/// 남은 시간 체크 & 갱신
 		if( mAnswerTime >= TRIAL_ANSWER_TIME )
 		{
 			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 				eMBEVENT_CAMAEL_KICK, eSKIN_OK,
 				GAMERESOURCEMAN->GetGameText( 11807 ),
 				GAMERESOURCEMAN->GetGameText( 301 )
 				);
 			Hide();
 		}
 		else
 		{
			unsigned long time = ( TRIAL_ANSWER_TIME - mAnswerTime ) / SECOND;
			mpTime->SetValue( time );
		}
	}
}

void cCamaelWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );
	if( mpTextImg )
		mpTextImg->Draw();
}

bool cCamaelWindow::SetCaptchaImage( unsigned char* pImageInfo, unsigned long imageSize )
{
	LPDIRECT3DTEXTURE9 dxTex = 0;
	cRenderer* renderer = RENDERSYS->GetRenderer();
	HRESULT ret = ::D3DXCreateTextureFromFileInMemoryEx( renderer->GetD3DDevice(), pImageInfo, imageSize,
		 D3DX_DEFAULT , D3DX_DEFAULT, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0,
		&dxTex );

	if( FAILED(ret) || dxTex == 0 )
	{
		assert( 0 );
		return false;
	}

	NiTexturePtr jpgTex = renderer->CreateNiTextureFromD3DTexture( (LPDIRECT3DBASETEXTURE9)dxTex );
	if( jpgTex == 0 )
	{
		assert( 0 );
		return false;
	}

	SAFE_DELETE( mpTextImg );
 	mpTextImg = new cUIImage( jpgTex );

	if( mpTextImg )
	{
		cUIRect captchaRect;
		captchaRect.mLeft   = mAbsoluteRect.mLeft + mpImageSkin->mSkinInfo->mX;
		captchaRect.mTop    = mAbsoluteRect.mTop  + mpImageSkin->mSkinInfo->mY;
		captchaRect.mRight  = captchaRect.mLeft + mpImageSkin->mSkinInfo->mWidth;
		captchaRect.mBottom = captchaRect.mTop  + mpImageSkin->mSkinInfo->mHeight;

		mpTextImg->SetScreenRect( captchaRect );
		mpTextImg->SetTextureRect( 0, 0, jpgTex->GetWidth(), jpgTex->GetHeight() );
	}

	return true;
}

void cCamaelWindow::SetTimeCheck( bool isCheck ) 
{ 
	mIsCheckTime = isCheck; 
	if( isCheck == true )
	{
		mAnswerTime  = 0;
		mIsOverRefresh = false;
		if( mpEdit )
			mpEdit->Clear();
	}
	SetRefreshButtonEnable( true );
}

void cCamaelWindow::SetEnabledButtons( bool set )
{
	if( mpOK && mpRefresh )
	{
		mpOK->SetEnabled( set );

		/// 다시 버튼을 활성화 시킬때 5회 초과횟수를 넘었으면 컨트롤을 못한다.
		if( mIsOverRefresh == true )
			mpRefresh->SetEnabled( false );
		else
			mpRefresh->SetEnabled( set );
	}
	else
	{
		assert(0);
	}
}

void cCamaelWindow::SetRefreshButtonEnable( bool set )
{
	if( mpRefresh )
	{
		/// 다시 버튼을 활성화 시킬때 5회 초과횟수를 넘었으면 컨트롤을 못한다.
		if( mIsOverRefresh == true )
			mpRefresh->SetEnabled( false );
		else
			mpRefresh->SetEnabled( set );
	}
	else
	{
		assert(0);
	}
}

void cCamaelWindow::SetOverRefresh( bool set )
{ 
	mIsOverRefresh = set; 
}

void cCamaelWindow::OnEditBoxEntered( cUINode* node, unsigned int )
{
	if( mpEdit && node == mpEdit )
	{
		/// 텍스트 글 수 체크
		if( mpEdit->GetLength() != MAX_TRIAL_ANSWER_LEN )
		{		
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11808 ) );
			return;
		}

		MSG_REQ_PLAYER_TRIAL_ANSWER msg;
		::memset( &msg, 0, sizeof(MSG_REQ_PLAYER_TRIAL_ANSWER) );
		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_TRIAL_ANSWER_REQ;
		Sstrncpy( msg.answer, MAX_TRIAL_ANSWER_LEN+1, mpEdit->GetText(), mpEdit->GetLength() );

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof( MSG_REQ_PLAYER_TRIAL_ANSWER ) );

		SetRefreshButtonEnable( false );
		mpEdit->Clear();
	}
}

void cCamaelWindow::TryToReport( unsigned long targetIndex )
{
	/// /신고 + 타겟팅 인지 검사
	cBaseObject* obj = OBJECTMAN->GetPlayer( targetIndex );
	if( obj == NULL )
		obj = HERO->GetTargetObject();

	if( obj )
	{
		if( obj->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			/// 래밸 체크
			if( HERO->GetLevel() < 10 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11804 ) );
				return;
			}

			/// 이미 심판 중인지 체크
			if( IsVisible() == true )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11805 ) );
				return;
			}
			
			/// 사용가능한 맵인지 체크(마을, 테마, 전장 사용 불가)
			unsigned short heroMapIndex = HERO->GetMapNum();
			if( !( heroMapIndex > MAP_MIN && heroMapIndex <= MAP_MAX ) )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 11802 ) );
				return;
			}
			
			///신고 메시지 쏘기
			MSG_REQ_PLAYER_TRIAL_OPEN msg;
			msg.Category = NM_PLAYER;
			msg.Protocol = NM_PLAYER_TRIAL_OPEN_REQ;
			msg.characterIdx = obj->GetObjectID();

			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(MSG_REQ_PLAYER_TRIAL_OPEN) );
		}
		else
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
	}
	else
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
	}
}
