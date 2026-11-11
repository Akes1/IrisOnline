#include "StdAfx.h"
#include "SelectWindow.h"
#include "StageManager.h"
#include "SelectStage.h"

#include "GameResourceManager.h"

#include "button.h"
#include "Label.h"
#include "Application.h"

#include "DisplayObject.h"


cSelectInfoWindow::cSelectInfoWindow()
: mpLevel(0)
, mpName(0)
, mpJob(0)
, mpMap(0)
, mpBack(0)
{
}

cSelectInfoWindow::~cSelectInfoWindow()
{
}

void cSelectInfoWindow::Open()
{
	Hide();

	SetExceptEvent( true );

	if( mpLevel )
		mpLevel->Clear();
	if( mpName )
		mpName->Clear();
	if( mpJob )
		mpJob->Clear();
	if( mpMap )
		mpMap->Clear();
}

bool cSelectInfoWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpLevel = (cLabel*)GetChild( eUIID_SELECTINFO_LEVEL );
	mpName = (cLabel*)GetChild( eUIID_SELECTINFO_NAME );
	mpJob = (cLabel*)GetChild( eUIID_SELECTINFO_JOB );
	mpMap = (cLabel*)GetChild( eUIID_SELECTINFO_MAP );

	mpBack = (cButton*)GetChild( eUIID_SELECTINFO_BACK );
	if( mpBack )
		mpBack->SetExceptEvent( true );

	/// 해상도에 따른 사이즈 갱신
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* winSkin = pSkin->GetNodeSkin( mSkinName );
	if( winSkin )
	{
		int x = ( GetScreenWidth() * winSkin->mSkinInfo->mX ) / mDefaultWidth;
		int y = ( GetScreenHeight() * winSkin->mSkinInfo->mY ) / mDefaultHeight;
		int w = ( GetScreenWidth() * winSkin->mSkinInfo->mWidth ) / mDefaultWidth;
		int h = ( GetScreenHeight() * winSkin->mSkinInfo->mHeight ) / mDefaultHeight;

		SetRelativeRect( cUIRect( x, y, x+w, y+h) );
	}

	if( mpBack )
	{
		cUINodeSkin* pskin = pSkin->GetNodeSkin( mpBack->GetSkinName() );
		if( pskin )
		{
			int x = ( GetScreenWidth() * pskin->mSkinInfo->mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * pskin->mSkinInfo->mY ) / mDefaultHeight;
			mpBack->SetRelativePos( cUIPos( x, y ) );
		}
	}
	return true;
}

void cSelectInfoWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	cUINodeSkin* winSkin = pSkin->GetNodeSkin( mSkinName );
	if( winSkin )
	{
		int x = ( GetScreenWidth() * winSkin->mSkinInfo->mX ) / mDefaultWidth;
		int y = ( GetScreenHeight() * winSkin->mSkinInfo->mY ) / mDefaultHeight;
		int w = ( GetScreenWidth() * winSkin->mSkinInfo->mWidth ) / mDefaultWidth;
		int h = ( GetScreenHeight() * winSkin->mSkinInfo->mHeight ) / mDefaultHeight;

		SetRelativeRect( cUIRect( x, y, x+w, y+h) );
	}

	if( mpBack )
	{
		cUINodeSkin* pskin = pSkin->GetNodeSkin( mpBack->GetSkinName() );
		if( pskin )
		{
			int x = ( GetScreenWidth() * pskin->mSkinInfo->mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * pskin->mSkinInfo->mY ) / mDefaultHeight;
			mpBack->SetRelativePos( cUIPos( x, y ) );
		}
	}
}

void cSelectInfoWindow::SetLevel( int level )
{
	if( mpLevel )
	{
		TCHAR buf[20] = {0,};
		::swprintf_s( buf, _T("%d"), level );

		mpLevel->SetText( buf );
	}
}

void cSelectInfoWindow::SetName( LPCTSTR name )
{
	if( mpName )
		mpName->SetText( name );
}

void cSelectInfoWindow::SetJob( LPCTSTR job )
{
	if( mpJob )
		mpJob->SetText( job );
}

void cSelectInfoWindow::SetMapName( LPCTSTR mapName )
{
	if( mpMap )
		mpMap->SetText( mapName );
}

void cSelectInfoWindow::ClearInfo()
{
	if( mpLevel )
		mpLevel->Clear();
	if( mpName )
		mpName->Clear();
	if( mpJob )
		mpJob->Clear();
	if( mpMap )
		mpMap->Clear();
}




////////////////////////////////////////////////////////
cSelectWindow::cSelectWindow()
: mpConnect(0)
, mpMake(0)
, mpBeforeCharacter(0)
, mpNextCharacter(0)
{
}

cSelectWindow::~cSelectWindow()
{
}

void cSelectWindow::Open()
{
	Hide();

	SetExceptEvent( true );

	if( mpMake )
		mpMake->Hide();
	if( mpConnect )
		mpConnect->Hide();
}


/// 주의사항 : 
bool cSelectWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpConnect = (cButton*)GetChild( eUIID_SELECT_CONNECT_BUTTON );
	mpMake = (cButton*)GetChild( eUIID_SELECT_CREATE_BUTTON );
	mpBeforeCharacter = (cButton*)GetChild( eUIID_SELECT_LEFTSELECT_BUTTON );
	mpNextCharacter = (cButton*)GetChild( eUIID_SELECT_RIGHTSELECT_BUTTON );
	return true;
}

void cSelectWindow::SetEnabled( bool enabled )
{
	cUIWindow::SetEnabled( enabled );

	if( mpMake )
		mpMake->SetEnabled( enabled );
	if( mpConnect )
		mpConnect->SetEnabled( enabled );
}

void cSelectWindow::ShowFrame( unsigned char showOpt )
{
	switch( showOpt )
	{
	case eShow_StartButton:
		{
			if( mpConnect )
				mpConnect->Show();
			if( mpBeforeCharacter )
				mpBeforeCharacter->Show();
			if( mpNextCharacter )
				mpNextCharacter->Show();

			if( mpMake )
				mpMake->Hide();
		}
		break;
	case eShow_MakeButton:
		{
			if( mpMake )
				mpMake->Show();
			if( mpBeforeCharacter )
				mpBeforeCharacter->Show();
			if( mpNextCharacter )
				mpNextCharacter->Show();

			if( mpConnect )
				mpConnect->Hide();
		}
		break;
	case eShow_None:
		{
			if( mpConnect )
				mpConnect->Hide();
			if( mpMake )
				mpMake->Hide();
			if( mpBeforeCharacter )
				mpBeforeCharacter->Show();
			if( mpNextCharacter )
				mpNextCharacter->Show();
		}
		break;
	default:
		assert(0);
		break;
	}
}

/// 주의사항 : 버튼이벤트가 일어난경우 
void cSelectWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_SELECT_CONNECT_BUTTON:
		{
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
			{
				cDisplayObject* obj = p->GetCurrentDisplayObject();
				if( obj && obj->GetDeleteWaitTime() != LONG_MAX )
				{
					// 현재 선택된 캐릭터가 삭제 대기일경우, 취소 패킷 
					TCHAR temp[256] = {0,};
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 52 ), obj->GetName() );

					UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
										eMBEVENT_CHARACTER_DELETE_CANCEL, eSKIN_YESNO,
										temp,
										GAMERESOURCEMAN->GetGameText( 2 ), 
										(void*)(obj->GetStandIdx())
										);
					return;
				}

				/// 접속 버튼
				MSGROOT msg;
				memset( &msg, 0, sizeof(msg) );
				/// SET MSGROOT [CATEGORY & PROTOCOL].
				msg.Category    = NM_USER;
				msg.Protocol    = NM_USER_CHANNEL_LIST_REQ;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_NONE,
					GAMERESOURCEMAN->GetGameText( 10 ),
					GAMERESOURCEMAN->GetGameText( 2 )
					);

			}
			else
				assert(0);
		}
		break;
	case eUIID_SELECT_CREATE_BUTTON:
		{
			/// 생성창으로 이동
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
				p->MoveMakeStage();
		}
		break;
	case eUIID_SELECT_LEFTSELECT_BUTTON:
		{
			/// 캐릭터 이동
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
				p->MoveCard( true );
		}
		break;
	case eUIID_SELECT_RIGHTSELECT_BUTTON:
		{
			/// 캐릭터 이동
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
				p->MoveCard( false );
		}
		break;
	default:
		assert(0);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
cSelectButtonWindow::cSelectButtonWindow()
: mpDelete(0)
, mpBegin(0)
, mpExit(0)
, mpDeleteCancel(0)
{

}

cSelectButtonWindow::~cSelectButtonWindow()
{

}

void cSelectButtonWindow::Open()
{
	Hide();

	SetExceptEvent( true );
	
	//
	ShowButtons( true );
	SetEnableButtons( true );

	if( mpDelete )
		mpDelete->SetEnabled( true );

	if( mpDeleteCancel )
		mpDeleteCancel->SetEnabled( false );
}

bool cSelectButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpDelete = (cButton*)GetChild( eUIID_SELECTBUTTON_DELETE );
	mpBegin = (cButton*)GetChild( eUIID_SELECTBUTTON_BEGIN );
	mpExit = (cButton*)GetChild( eUIID_SELECTBUTTON_EXIT );
	mpDeleteCancel = (cButton*)GetChild( eUIID_SELECTBUTTON_DELETECANCEL );
	return true;
}

void cSelectButtonWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_SELECTBUTTON_DELETE:
		{
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
			{
				cDisplayObject* obj = p->GetCurrentDisplayObject();
				if( obj )
				{
					TCHAR temp[256] = {0,};
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 62 ), obj->GetName() );

					UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
						eMBEVENT_CHARACTER_DELETE, eSKIN_YESNO,
						temp,
						GAMERESOURCEMAN->GetGameText( 2 )
						);
				}
			}
		}
		break;
	case eUIID_SELECTBUTTON_DELETECANCEL:
		{
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
			{
				cDisplayObject* obj = p->GetCurrentDisplayObject();
				if( obj && obj->GetDeleteWaitTime() != LONG_MAX )
				{
					// 현재 선택된 캐릭터가 삭제 대기일경우, 취소 패킷 
					TCHAR temp[256] = {0,};
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 52 ), obj->GetName() );

					UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
						eMBEVENT_CHARACTER_DELETE_CANCEL, eSKIN_YESNO,
						temp,
						GAMERESOURCEMAN->GetGameText( 2 ), 
						(void*)(obj->GetStandIdx())
						);
				}
			}
		}
		break;
	case eUIID_SELECTBUTTON_BEGIN:
		{
			if( THEAPP->GetAuthCode() == -1 )
			{
				/// 로그인창으로 이동
				STAGEMAN->ChangeStage( eStage_Login );
			}
			else
				assert(0);
		}
		break;
	case eUIID_SELECTBUTTON_EXIT:
		{
			// 게임종료
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	}
}

void cSelectButtonWindow::ShowDelete( bool show )
{
	if( mpDelete )
	{
		mpDelete->SetEnabled( show );

		if( show )
			mpDelete->Show();
		else
			mpDelete->Hide();
	}
}

void cSelectButtonWindow::ShowDeleteCancel( bool show )
{
	if( mpDeleteCancel )
	{
		mpDeleteCancel->SetEnabled( show );

		if( show )
			mpDeleteCancel->Show();
		else
			mpDeleteCancel->Hide();
	}
}

void cSelectButtonWindow::SetEnableButtons( bool enabled )
{
	if( mpBegin )
		mpBegin->SetEnabled( enabled );

	if( mpExit )
		mpExit->SetEnabled( enabled );
}

void cSelectButtonWindow::ShowButtons( bool show )
{
	// 웹런쳐 모드이면
	if( THEAPP->GetAuthCode() != -1 )
	{
		if( mpBegin )
			mpBegin->Hide();

		// 게임종료만 보임
		if( show == true )
		{
			if( mpExit )
				mpExit->Show();
		}
		else
		{
			if( mpExit )
				mpExit->Hide();
		}
	}
	else
	{
		// 로그아웃, 게임종료 보임
		if( show == true )
		{
			if( mpBegin )
				mpBegin->Show();

			if( mpExit )
				mpExit->Show();
		}
		else
		{
			if( mpBegin )
				mpBegin->Hide();

			if( mpExit )
				mpExit->Hide();
		}
	}
}
