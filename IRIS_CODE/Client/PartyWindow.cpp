#include "stdafx.h"
#include "PartyWindow.h"
#include "Label.h"
#include "Button.h"
#include "GameUIManager.h"
#include "PartyManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PartyStateWindow.h"
#include "UIMsgboxEvent.h"
#include "ChatManager.h"
#include "HeroStateWindow.h"
#include "PartyUnionManager.h"
#include "GameResourceManager.h"
#include "PushButton.h"

cPartyWindow::cPartyWindow()
: mpDivide(0)
, mpMenuButton(0)
, mDisplayType( ePARTY_STATE_DISPLAY_NORMAL )
, mpMoveLock(0)
, mpDiceButton(0)
, mpNodeSkin(0)
{

}

cPartyWindow::~cPartyWindow()
{

}

void cPartyWindow::Clear()
{
	SetLeader( false );
	SetDivideType( PARTY_DIVIDE_TURN );

	if( mpMenuButton )
		mpMenuButton->Show();

	if( mpDiceButton )
		mpDiceButton->Hide();
}

void cPartyWindow::Open()
{
}

void cPartyWindow::Close()
{
	cUIWindow::Close();

	GAMEUI->HidePartyMenu();
}

void cPartyWindow::OnHide()
{
	cUIWindow::OnHide();

	///
	GAMEUI->HidePartyMenu();
}

bool cPartyWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpDivide = (cLabel*)GetChild( eUIID_GAME_PARTY_DIVIDE );
	mpMenuButton = (cButton*)GetChild( eUIID_GAME_PARTY_MENU );
	mpMoveLock = (cPushButton*)GetChild( eUIID_GAME_PARTY_MOVELOCK );
	mpDiceButton = (cButton*)GetChild( eUIID_GAME_PARTY_DICE );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	mpNodeSkin = pSkin->GetNodeSkin( mSkinName );
	if( !mpNodeSkin )
	{
		assert(0);
		return false;
	}
	return true;
}

void cPartyWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	/// 이미지 영역을 갱신
	if( mpNodeSkin && mpImage )
	{
		cUIRect imgRect( mAbsoluteRect.mLeft, 
						mAbsoluteRect.mTop, 
						mAbsoluteRect.mLeft + mpNodeSkin->mSkinInfo->mWidth, 
						mAbsoluteRect.mTop  + mpNodeSkin->mSkinInfo->mHeight );

		mpImage->SetScreenRect( imgRect );
	}
}

void cPartyWindow::OnMouseMove( const cUIPos& pos )
{
	// 이동기능 lock 아니면 
	if( IsLockedMove() == false )
		cUIWindow::OnMouseMove( pos );
}

void cPartyWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_PARTY_MENU:
		{
			/// 파티 관리 메뉴 위치 갱신
			cPartyMenu* pPartyMenu = GAMEUI->GetPartyMenu();
			if( pPartyMenu )
			{
				if( pPartyMenu->IsVisible() == false )
				{
					cUIRect partyMenuRect = pPartyMenu->GetRelativeRect();
					cUIRect partyWinRect  = this->GetRelativeRect();

					pPartyMenu->SetRelativeRect( cUIRect( partyWinRect.mRight + 2,
														  partyWinRect.mTop,
														  partyWinRect.mRight + 2 + partyMenuRect.GetWidth(),
														  partyWinRect.mTop   +		partyMenuRect.GetHeight() ));
				}
			}

			/// 파티 관리 메뉴 토글
			GAMEUI->TogglePartyMenu();
		}
		break;
	case eUIID_GAME_PARTY_DISPLAY:
		{
			unsigned char type = (unsigned char)((mDisplayType + 1) % ePARTY_STATE_DISPLAY_MAX);
			ChangeDisplayType( type );
		}
		break;
	case eUIID_GAME_PARTY_DICE:
		{
			if( PARTYMAN->IsHeroLeader() == false )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 122 ) );
				return;
			}

			// 주사위 굴리기 요청
			MSGROOT msg;
			::memset( &msg, 0, sizeof( msg ) );
			msg.Category = NM_PARTY;
			msg.Protocol = NM_PARTY_DICE_START_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			//
			NetLock();
		}
		break;
	}
}

void cPartyWindow::ChangeDisplayType( unsigned char type )
{
	if( type == mDisplayType )
		return;

	if( type >= ePARTY_STATE_DISPLAY_MAX )
	{
		assert(0);
		return;
	}

	mDisplayType = type;

	if( HERO->GetPartyIndex() > 0 )
	{
		PARTYMAN->ChangeDisplayType( mDisplayType );
	}
	else if( HERO->GetPartyUnionIndex() > 0 )
	{
		PARTYUNIONMAN->ChangeDisplayType( mDisplayType );
	}
}

void cPartyWindow::ChangeTarget( cPartyStateWindow* w )
{
	/// 타겟팅을 새로해야하면 타겟창에 타겟설정
	if( w && w->GetUserIndex() > 0 )
		HERO->SetTargetObject( eOBJECTTYPE_PLAYER, w->GetUserIndex() );
}

void cPartyWindow::SetUnion()
{
	SetDivideType( PARTY_DIVIDE_TURN );

	SetLeader( false );

	if( mpMenuButton )
		mpMenuButton->Hide();
}

void cPartyWindow::SetLeader( bool set )
{
	cPartyMenu* menu = GAMEUI->GetPartyMenu();
	if( menu )
		menu->SetEnableButton( set );

	/// 자기자신이 리더면 리더 세팅
	cHeroStateWindow* heroState = GAMEUI->GetHeroStateWindow();
	if( heroState )
		heroState->ShowLeaderImage( set );

	// 윈도우 사이즈 재 지정
	if( mpNodeSkin && mpDiceButton )
	{
		cUISize size;
		size.mWidth = mpNodeSkin->mSkinInfo->mWidth;
		size.mHeight = mpNodeSkin->mSkinInfo->mHeight;

		if( set == true )
			size.mWidth += mpDiceButton->GetRelativeRect().GetWidth();

		SetRelativeSize( size );
	}

	if( mpDiceButton )
	{
		bool isDice = ( PARTYMAN->IsDice() == false );
		mpDiceButton->SetEnabled( isDice );

		if( set == true )
			mpDiceButton->Show();
		else
			mpDiceButton->Hide();
	}
}

void cPartyWindow::SetDivideType( PARTY_DIVIDE_TYPE type )
{
	unsigned int textId = 0;
	switch( type )
	{
	case PARTY_DIVIDE_TURN:		textId = 265;		break;
	case PARTY_DIVIDE_RANDOM:	textId = 266;		break;
	case PARTY_DIVIDE_DAMAGE:	textId = 267;		break;
	case PARTY_DIVIDE_FREE:		textId = 268;		break;
	case PARTY_DIVIDE_LEADER:	textId = 269;		break;
	default:					assert(0);			return;
	}

	if( mpDivide )
	{
		mpDivide->SetTextColor( COLOR_WHITE );
		mpDivide->SetText( UIMAN->GetUIText( textId ) );
	}
}

// 이동 기능 고정
void cPartyWindow::SetMoveLock( bool lock )
{
	if( mpMoveLock )
		mpMoveLock->SetPress( lock );
}

bool cPartyWindow::IsLockedMove()
{
	return ( mpMoveLock ) ? mpMoveLock->IsPress() : false;
}

cUIRect cPartyWindow::GetButtonRect()
{
	if( mpMenuButton )
		return mpMenuButton->GetAbsoluteRect();
	else
		return cUIRect( 0, 0, 0, 0);
}

void cPartyWindow::SetEnableDice( bool enable )
{
	if( mpDiceButton )
		mpDiceButton->SetEnabled( enable );
}

void cPartyWindow::OnNetLock( int lockTry )
{
	if( mpDiceButton )
		mpDiceButton->NetLock( lockTry );
}

void cPartyWindow::OnNetUnLock( int lockTry )
{
	if( mpDiceButton )
		mpDiceButton->NetUnLock( lockTry );
}

////////////////////////////////////////////////////////////////////////////////
cPartyMenu::cPartyMenu()
: mpInvite(0)
, mpChangeLeader(0)
, mpChangeDivide(0)
, mpThrowUser(0)
, mpOut(0)
, mSetEnable( false )
{

}

cPartyMenu::~cPartyMenu()
{

}

void cPartyMenu::OnHide()
{
	cUIWindow::OnHide();

	GAMEUI->HidePartyDivideMenu();
}

void cPartyMenu::SetEnableButton( bool set )
{
	mSetEnable = set;

	if( mpInvite )
		mpInvite->SetEnabled( set );

	if( mpChangeLeader )
		mpChangeLeader->SetEnabled( set );

	if( mpChangeDivide )
		mpChangeDivide->SetEnabled( set );

	if( mpThrowUser )
		mpThrowUser->SetEnabled( set );
}

bool cPartyMenu::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpInvite = (cButton*)GetChild( eUIID_GAME_PARTYMENU_INVITE );
	mpChangeLeader = (cButton*)GetChild( eUIID_GAME_PARTYMENU_CHANGELEADER );
	mpChangeDivide = (cButton*)GetChild( eUIID_GAME_PARTYMENU_CHANGEDIVDIE );
	mpThrowUser = (cButton*)GetChild( eUIID_GAME_PARTYMENU_THROWUSER );
	mpOut = (cButton*)GetChild( eUIID_GAME_PARTYMENU_OUT );

	if( mpOut )
		mpOut->SetEnabled( true );
	return true;
}

void cPartyMenu::OnMouseOtherClick( const cUIPos& pos )
{
	/// 
	cPartyWindow* party = GAMEUI->GetPartyWindow();
	if( !party )
	{
		assert(0);
		return;
	}

	cPartyDivideMenu* divide = GAMEUI->GetPartyDivideMenu();
	if( !divide )
	{
		assert(0);
		return;
	}

	cUIRect divideRc = divide->GetAbsoluteRect();
	cUIRect partyRc = party->GetButtonRect();
    
	if( divide->IsVisible() == true )
	{
		if( mAbsoluteRect.ContainPoint( pos ) == false && 
			divideRc.ContainPoint( pos ) == false &&
			partyRc.ContainPoint( pos) == false )
		{
			Hide();
		}
	}
	else
	{
		if( mAbsoluteRect.ContainPoint( pos ) == false && 
			partyRc.ContainPoint( pos) == false )
		{
			Hide();
		}
	}
}

void cPartyMenu::OnCommand( cUINode*, unsigned int id )
{
	cStringT Message;

	switch( id )
	{
	case eUIID_GAME_PARTYMENU_INVITE:
		{
			/// 파티원초대
			/// /파티 + 타겟팅 인지 검사
			cBaseObject* pTarget = HERO->GetTargetObject();
			if( pTarget )
			{
				// 타겟이 Player가 아닌 다른 Object인 경우 취소
				if( pTarget->GetObjectType() != eOBJECTTYPE_PLAYER )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 142 ) );
					return;
				}

				/// 타겟 있음
				PARTYMAN->RequestPartyAdd( pTarget->GetName() );
			}
			else
			{
				/// 타겟 없음.
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 102 ) );
			}

			Hide();
		}
		break;
	case eUIID_GAME_PARTYMENU_CHANGELEADER:
		{
			/// 파티장 위임
			LPCTSTR name = PARTYMAN->GetSelectUserName();
			
			if( name && ::_tcslen(name) > 0 )
			{
				Message.Format( GAMERESOURCEMAN->GetGameText( 128 ), name );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_PARTY_CHANGELEADER_REQ, eSKIN_YESNO,
					(LPCTSTR)Message.Cstr(),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
			}

			Hide();
		}
		break;
	case eUIID_GAME_PARTYMENU_CHANGEDIVDIE:
		{
			/// 파티 관리 메뉴 위치 갱신
			cPartyDivideMenu* pPartyDivideMenu = GAMEUI->GetPartyDivideMenu();
			if( pPartyDivideMenu )
			{
				if( pPartyDivideMenu->IsVisible() == false )
				{
					cUIRect partyDivideMenuRect = pPartyDivideMenu->GetRelativeRect();
					cUIRect partyMenuRect		= this->GetRelativeRect();

					pPartyDivideMenu->SetRelativeRect( cUIRect( partyMenuRect.mRight,
																partyMenuRect.mTop,
																partyMenuRect.mRight + partyDivideMenuRect.GetWidth(),
																partyMenuRect.mTop   + partyDivideMenuRect.GetHeight() ));
				}
			}

			/// 파티분배타입변경
			GAMEUI->TogglePartyDivideMenu();
		}
		break;
	case eUIID_GAME_PARTYMENU_THROWUSER:
		{
			LPCTSTR name = PARTYMAN->GetSelectUserName();

			if( name && ::_tcslen(name) > 0 )
			{
				/// 파티원추방
				Message.Format( GAMERESOURCEMAN->GetGameText( 129 ), name );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_PARTY_THROW, eSKIN_YESNO,
					(LPCTSTR)Message.Cstr(),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
			}

			Hide();
		}
		break;
	case eUIID_GAME_PARTYMENU_OUT:
		{
			/// 파티탈퇴
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_PARTY_OUT, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 130 ),	// 파티 탈퇴
				GAMERESOURCEMAN->GetGameText( 301 )
				);

			Hide();
		}
		break;
	}
}

void cPartyMenu::OnNetLock( int lockTry )
{
	if( mpInvite )
		mpInvite->NetLock( lockTry );

	if( mpChangeLeader )
		mpChangeLeader->NetLock( lockTry );

	if( mpChangeDivide )
		mpChangeDivide->NetLock( lockTry );

	if( mpThrowUser )
		mpThrowUser->NetLock( lockTry );

	if( mpOut )
		mpOut->NetLock( lockTry );
}

void cPartyMenu::OnNetUnLock( int lockTry )
{
	if( mpInvite )
		mpInvite->NetUnLock( lockTry );

	if( mpChangeLeader )
		mpChangeLeader->NetUnLock( lockTry );

	if( mpChangeDivide )
		mpChangeDivide->NetUnLock( lockTry );

	if( mpThrowUser )
		mpThrowUser->NetUnLock( lockTry );

	if( mpOut )
		mpOut->NetUnLock( lockTry );

	/// 원래세팅으로 돌림
	SetEnableButton( mSetEnable );

	if( mpOut )
		mpOut->SetEnabled( true );
}

////////////////////////////////////////////////////////////////////////////////
cPartyDivideMenu::cPartyDivideMenu()
: mpSequence(0)
, mpRandom(0)
, mpDamage(0)
, mpFree(0)
, mpLeader(0)
{
}

cPartyDivideMenu::~cPartyDivideMenu()
{

}

void cPartyDivideMenu::OnHide()
{
	cUIWindow::OnHide();

	GAMEUI->HidePartyMenu();
}

bool cPartyDivideMenu::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpSequence = (cButton*)GetChild( eUIID_GAME_PARTYDIVIDE_SEQUENCE );
	mpRandom = (cButton*)GetChild( eUIID_GAME_PARTYDIVIDE_RANDOM );
	mpDamage = (cButton*)GetChild( eUIID_GAME_PARTYDIVIDE_DAMAGE );
	mpFree = (cButton*)GetChild( eUIID_GAME_PARTYDIVIDE_FREE );
	mpLeader = (cButton*)GetChild( eUIID_GAME_PARTYDIVIDE_LEADER );
	return true;
}

void cPartyDivideMenu::OnCommand( cUINode*, unsigned int id )
{
	PARTY_DIVIDE_TYPE type = PARTY_DIVIDE_NONE;

	switch( id )
	{
	case eUIID_GAME_PARTYDIVIDE_SEQUENCE:	type = PARTY_DIVIDE_TURN; break;	/// 순차분배
	case eUIID_GAME_PARTYDIVIDE_RANDOM:		type = PARTY_DIVIDE_RANDOM;	break; 	/// 랜덤분배
	case eUIID_GAME_PARTYDIVIDE_DAMAGE:		type = PARTY_DIVIDE_DAMAGE;	break;	/// 데미지분배
	case eUIID_GAME_PARTYDIVIDE_FREE:		type = PARTY_DIVIDE_FREE;	break;	/// 자유분배
	case eUIID_GAME_PARTYDIVIDE_LEADER:		type = PARTY_DIVIDE_LEADER;	break;	/// 파티장 분배
	}

	PARTYMAN->RequestChangeDivide( type );
	Hide();
}

void cPartyDivideMenu::OnNetLock( int lockTry )
{
	if( mpSequence )
		mpSequence->NetLock( lockTry );

	if( mpRandom )
		mpRandom->NetLock( lockTry );

	if( mpDamage )
		mpDamage->NetLock( lockTry );

	if( mpFree )
		mpFree->NetLock( lockTry );

	if( mpLeader )
		mpLeader->NetLock( lockTry );
}

void cPartyDivideMenu::OnNetUnLock( int lockTry )
{
	if( mpSequence )
		mpSequence->NetUnLock( lockTry );

	if( mpRandom )
		mpRandom->NetUnLock( lockTry );

	if( mpDamage )
		mpDamage->NetUnLock( lockTry );

	if( mpFree )
		mpFree->NetUnLock( lockTry );

	if( mpLeader )
		mpLeader->NetUnLock( lockTry );
}
