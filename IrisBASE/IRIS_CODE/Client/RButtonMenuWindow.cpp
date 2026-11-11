#include "StdAfx.h"
#include "rbuttonmenuwindow.h"

#include "InputSystem.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "hero.h"
#include "PartyManager.h"
#include "DuelManager.h"
#include "ItemManager.h"
#include "ChatManager.h"
#include "GameResourceManager.h"
#include "GuildManager.h"
#include "PartyUnionManager.h"
#include "label.h"
#include "ListBox.h"
#include "PlaneObject.h"
#include "Protocol.h"
#include "CamaelWindow.h"
#include "Button.h"
#include "PVPManager.h"

#ifdef _GMTOOL
#include "CheatManager.h"
#endif


cRButtonMenuWindow::cRButtonMenuWindow()
: mpMenuList(0)
, mpName(0)
, mInitWidth(0)
{
	mPlayerIdx = 0;
	mPlayerName = _T("");

	memset( mpBackImage, 0, sizeof(mpBackImage) );
	memset( mpBackSkin, 0, sizeof(mpBackSkin) );
}

cRButtonMenuWindow::~cRButtonMenuWindow()
{
	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
		SAFE_DELETE( mpBackImage[i] );

	Clear();
}

void cRButtonMenuWindow::Clear()
{
	if( mpMenuList == 0 )
		return;

	mpMenuList->Clear();

	mPlayerIdx = 0;
	mPlayerName = _T("");
}

void cRButtonMenuWindow::Open()
{
}

void cRButtonMenuWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cRButtonMenuWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

bool cRButtonMenuWindow::OnCreate( cUINodeProperty* prop )
{
	if( cUIWindow::OnCreate( prop ) == false )
		return false;

	mpMenuList = (cListBox*)GetChild( eUIID_GAME_RBUTTONMENU_LISTBOX );
	mpName = (cLabel*)GetChild( eUIID_GAME_RBUTTONMENU_NAME );

	if( mpMenuList )
	{
		mpMenuList->SetContinueEvent( true );
		mpMenuList->SetHoverTextColor( COLOR_YELLOW );
	}

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

    /// 배경 이미지 생성
	mpBackSkin[eBACKIMAGE_TOP] = pSkin->GetNodeSkin( "RButtonMenu_Top" );
	mpBackSkin[eBACKIMAGE_BOTTOM] = pSkin->GetNodeSkin( "RButtonMenu_Bottom" );
	mpBackSkin[eBACKIMAGE_CENTER] = pSkin->GetNodeSkin( "RButtonMenu_Back" );
	mpBackSkin[eBACKIMAGE_LEFT] = pSkin->GetNodeSkin( "RButtonMenu_Left" );
	mpBackSkin[eBACKIMAGE_RIGHT] = pSkin->GetNodeSkin( "RButtonMenu_Right" );

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackSkin[i] && mpBackSkin[i]->mpTexture )
		{
			/// 텍스처 영역 설정
			unsigned short tx = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexY;
			unsigned short tw = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexWidth;
			unsigned short th = (unsigned short)mpBackSkin[i]->mSkinInfo->mTexHeight;
			unsigned int x = GetAbsoluteRect().mLeft;
			unsigned int y = GetAbsoluteRect().mTop;

			mpBackImage[i] = new cPlaneObject;

			if( mpBackImage[i]->Create( mpBackSkin[i]->mpTexture, x, y, x+1, y+1, tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create back image");
				return false;
			}
		}
	}

	UIMAN->AddEventOtherClick( this );

	mInitWidth = GetAbsoluteRect().GetWidth();

	if( mpMenuList )
		mpMenuList->SetHoverTextColor( COLOR_TIP );
	return true;
}

void cRButtonMenuWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	/// 자식 렌더링
	cChildList::cIterator b = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; b != end; ++b )
	{
		if( ((cUINode*)(*b))->IsVisible() == true )
			((cUINode*)(*b))->OnRender(fontKeeper);
	}

	///
	fontKeeper->DrawAll();
}

void cRButtonMenuWindow::OnMouseOtherClick( const cUIPos& pos )
{
	if( mAbsoluteRect.ContainPoint( pos ) == false )
	{
		Hide();
	}
}

void cRButtonMenuWindow::OnListBoxChanged( cUINode* caller, unsigned int id )
{
	if( mpMenuList )
	{
		unsigned int selectIdx = mpMenuList->GetSelectIndex();
		ePopupMenuType type = (ePopupMenuType)(unsigned long)mpMenuList->GetExtraData( selectIdx );
		switch( type )
		{
		case ePopupMenuType_Whisper:
			{
				GAMEUI->OnFocusChatWindow( KEY_APOSTROPHE, mPlayerName.Cstr() );
			}
			break;
		case ePopupMenuType_Friend:
			{
				/// 서버에 친구 추가 요청
				MSG_REQ_FRIEND_ADD msg;
				::memset( &msg, 0, sizeof( msg ) );

				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_FRIEND_ADD_REQ;
				Sstrncpy( msg.Name, MAX_NAME_BUFFER_SIZE, mPlayerName.Cstr(), MAX_NAME_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			}
			break;
		case ePopupMenuType_PartyIn:
			{
				PARTYMAN->RequestPartyAdd( mPlayerName.Cstr() );
			}
			break;
		case ePopupMenuType_PartyLeader:
			{
				PARTYMAN->RequestChangeLeader( mPlayerIdx );
			}
			break;
		case ePopupMenuType_PartyOut:
			{
				PARTYMAN->RequestThrow( mPlayerIdx );
			}
			break;
		case ePopupMenuType_Duel:
			{
				HERO->DuelRequest( mPlayerIdx );
			}
			break;
		case ePopupMenuType_Trade:
			{
				/// /거래 + 타겟팅 인지 검사
				cPlayer* player = OBJECTMAN->GetPlayer( mPlayerIdx );
				if( player )
				{
					#ifdef _GMTOOL
					 if( CHEATMAN->GetHideMode() == true )
					 {
					 	CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2020 ) );
					 	break;
					 }
					#endif
					
					ITEMMAN->TryToTradeAsk( player );
				}
				else
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 121 ) );
			}
			break;
		case ePopupMenuType_GuildIn:
			{
				GUILDMAN->GuildAdd( mPlayerName );
			}
			break;
		case ePopupMenuType_Follow:
			{
				HERO->RequestFollowPlayer( mPlayerIdx );
			}
			break;
		case ePopupMenuType_UnionLeader:
			{
				PARTYUNIONMAN->TryToChangeLeader( mPlayerIdx );
			}
			break;
 		case ePopupMenuType_Camael:
 			{
 				cCamaelWindow* camaelWin = GAMEUI->GetCamaelWindow();
 				if( camaelWin )
 					camaelWin->TryToReport( mPlayerIdx );
 				else
 					assert(0);
 			}
 			break;
		case ePopupMenuType_PvPReport:
			{
				PVPMAN->RequestReport( mPlayerName );
			}
			break;
		case ePopupMenuType_LookPlayer:
			{
				HERO->RequestLookPlayer( mPlayerIdx );
			}
			break;
		default:
			break;
		}
	}

	Hide();
}


void cRButtonMenuWindow::PopUp( int posX, int posY, unsigned long playerIdx, LPCTSTR playerName, ePopupPos caller )
{
	if( mpMenuList == 0 )
		return;

	cPlayer* player = OBJECTMAN->GetPlayer( playerIdx );
	if( player )
	{
		if( HERO && HERO->IsEnemy( player ) == true )
			return;
	}

	mpMenuList->Clear();

	mPlayerIdx = playerIdx;
	
	if( playerName && wcslen( playerName ) > 0 )
	{
		mPlayerName = playerName;
		mpName->SetText( mPlayerName.Cstr() );
	}
	else
	{
		mPlayerName.Clear();
		mpName->Clear();
	}

	switch( caller )
	{
	case ePopupPos_Character:
	case ePopupPos_TargetWindow:
		{
			if( player != NULL )
			{
				// 살펴보기
				mpMenuList->AddRow( UIMAN->GetUIText( 813 ), COLOR_WHITE, (void*)ePopupMenuType_LookPlayer );

				/// 귓속말
				mpMenuList->AddRow( UIMAN->GetUIText( 800 ), COLOR_WHITE, (void*)ePopupMenuType_Whisper );

				/// 친구등록
				if( CHATMANAGER->GetFriendData( playerIdx ) == false )
				{	
					mpMenuList->AddRow( UIMAN->GetUIText( 802 ), COLOR_WHITE, (void*)ePopupMenuType_Friend );
				}
				
				if( HERO->GetPartyIndex() > 0 )
				{
					if( PARTYMAN->IsHeroLeader() )
					{
						/// 상대방이 파티원이어야만 가능
						if( PARTYMAN->IsHeroParty( playerIdx ) )
						{
							/// 파티장 위임
							mpMenuList->AddRow( UIMAN->GetUIText( 804 ), COLOR_WHITE, (void*)ePopupMenuType_PartyLeader );

							/// 파티원 추방
							mpMenuList->AddRow( UIMAN->GetUIText( 805 ), COLOR_WHITE, (void*)ePopupMenuType_PartyOut );
						}
						else
						{
							/// 파티초대
							mpMenuList->AddRow( UIMAN->GetUIText( 803 ), COLOR_WHITE, (void*)ePopupMenuType_PartyIn );
						}
					}
				}
				else if( HERO->GetPartyUnionIndex() > 0 )
				{
					if( PARTYUNIONMAN->IsHeroLeader() )
					{
						/// 연합장 위임
						mpMenuList->AddRow( UIMAN->GetUIText( 810 ), COLOR_WHITE, (void*)ePopupMenuType_UnionLeader );
					}
				}
				else
				{
					/// 파티초대
					mpMenuList->AddRow( UIMAN->GetUIText( 803 ), COLOR_WHITE, (void*)ePopupMenuType_PartyIn );
				}

				/// 결투 신청
				if( DUELMAN->IsDuelEnable() == false )
				{
					if( player->IsDie() == false && HERO->IsDie() == false )
						mpMenuList->AddRow( UIMAN->GetUIText( 806 ), COLOR_WHITE, (void*)ePopupMenuType_Duel );
				}

				/// 거래 요청
				mpMenuList->AddRow( UIMAN->GetUIText( 807 ), COLOR_WHITE, (void*)ePopupMenuType_Trade );

				/// 길드 초대
				if( HERO->GetGuildIndex() > 0 )
					mpMenuList->AddRow( UIMAN->GetUIText( 808 ), COLOR_WHITE, (void*)ePopupMenuType_GuildIn );

				/// 따라가기
				mpMenuList->AddRow( UIMAN->GetUIText( 809 ), COLOR_WHITE, (void*)ePopupMenuType_Follow );

 				/// 자동사냥신고
 				if( HERO->GetLevel() >= 10 )
 				{
 					/// 사용가능한 맵인지 체크(마을, 테마, 전장 사용 불가)
 					unsigned short heroMapIndex = HERO->GetMapNum();
 					if( ( heroMapIndex > MAP_MIN && heroMapIndex <= MAP_MAX ) )
 						mpMenuList->AddRow( UIMAN->GetUIText( 811 ), COLOR_WHITE, (void*)ePopupMenuType_Camael );					
 				}

				// 전장 자리비움신고
				if( PVPMAN && PVPMAN->IsEnableBattle() == true )
				{
					mpMenuList->AddRow( UIMAN->GetUIText( 812 ), COLOR_WHITE, (void*)ePopupMenuType_PvPReport );
				}
			}
			else
			{
				assert(0);
				return;
			}
		}
		break;
	case ePopupPos_PartyWindow:
		{
			/// 귓속말
			mpMenuList->AddRow( UIMAN->GetUIText( 800 ), COLOR_WHITE, (void*)ePopupMenuType_Whisper );

			/// 친구등록
			if( CHATMANAGER->GetFriendData( playerIdx ) == false )
			{
				mpMenuList->AddRow( UIMAN->GetUIText( 802 ), COLOR_WHITE, (void*)ePopupMenuType_Friend );
			}

			if( HERO->GetPartyIndex() > 0 )
			{
				if( PARTYMAN->IsHeroLeader() )
				{
					/// 상대방이 파티원이어야만 가능
					if( PARTYMAN->IsHeroParty( playerIdx ) )
					{
						/// 파티장 위임
						mpMenuList->AddRow( UIMAN->GetUIText( 804 ), COLOR_WHITE, (void*)ePopupMenuType_PartyLeader );

						/// 파티원 추방
						mpMenuList->AddRow( UIMAN->GetUIText( 805 ), COLOR_WHITE, (void*)ePopupMenuType_PartyOut );
					}
				}
			}
			else if( HERO->GetPartyUnionIndex() > 0 )
			{
				if( PARTYUNIONMAN->IsHeroLeader() )
				{
					/// 연합장 위임
					mpMenuList->AddRow( UIMAN->GetUIText( 810 ), COLOR_WHITE, (void*)ePopupMenuType_UnionLeader );
				}
			}

			if( player != NULL )
			{
				/// 결투 신청
				if( DUELMAN->IsDuelEnable() == false )
				{
					if( player->IsDie() == false && HERO->IsDie() == false )
						mpMenuList->AddRow( UIMAN->GetUIText( 806 ), COLOR_WHITE, (void*)ePopupMenuType_Duel );
				}

				/// 거래 요청
				mpMenuList->AddRow( UIMAN->GetUIText( 807 ), COLOR_WHITE, (void*)ePopupMenuType_Trade );
			}
			
			/// 길드 초대
			if( HERO->GetGuildIndex() > 0 )
				mpMenuList->AddRow( UIMAN->GetUIText( 808 ), COLOR_WHITE, (void*)ePopupMenuType_GuildIn );

			if( player != NULL )
			{
				/// 따라가기
				mpMenuList->AddRow( UIMAN->GetUIText( 809 ), COLOR_WHITE, (void*)ePopupMenuType_Follow );
			}

 			/// 자동사냥신고
 			if( player != NULL )
 			{
 				if( HERO->GetLevel() >= 10 )
 				{
 					/// 사용가능한 맵인지 체크(마을, 테마, 전장 사용 불가)
 					unsigned short heroMapIndex = HERO->GetMapNum();
 					if( ( heroMapIndex > MAP_MIN && heroMapIndex <= MAP_MAX ) )
 						mpMenuList->AddRow( UIMAN->GetUIText( 811 ), COLOR_WHITE, (void*)ePopupMenuType_Camael );					
 				}
 			}

			// 전장 자리비움신고
			if( PVPMAN && PVPMAN->IsEnableBattle() == true )
			{
				mpMenuList->AddRow( UIMAN->GetUIText( 812 ), COLOR_WHITE, (void*)ePopupMenuType_PvPReport );
			}
		}
		break;
	default:
		return;
	}

	if( mpMenuList->GetRowCount() == 0 )
		return;
    
	unsigned int rowHeight = mpMenuList->GetRowCount() * mpMenuList->GetRowHeight();
	int listWidth = mpMenuList->GetAbsoluteRect().GetWidth();
	mpMenuList->SetRelativeSize( cUISize( listWidth, rowHeight ) );

	rowHeight += mpName->GetRelativeRect().GetHeight();

	int w = mInitWidth + (2 * MENU_GAPX);
	int h = rowHeight + (2* MENU_GAPY);
	SetRelativeRect( cUIRect( posX, posY, posX+w, posY + h ) );

	/// 이미지 설정
	int x = GetAbsoluteRect().mLeft;
	int y = GetAbsoluteRect().mTop;
	if( mpBackImage[eBACKIMAGE_TOP] )
	{
		int skinHeight = mpBackSkin[eBACKIMAGE_TOP]->mSkinInfo->mHeight;
		mpBackImage[eBACKIMAGE_TOP]->SetScreenRect( x, y, w, skinHeight );
	}
	if( mpBackImage[eBACKIMAGE_CENTER] )
	{
		mpBackImage[eBACKIMAGE_CENTER]->SetScreenRect( x + MENU_GAPX, y + MENU_GAPY, mInitWidth, rowHeight );
	}
	if( mpBackImage[eBACKIMAGE_BOTTOM] )
	{
		int skinHeight = mpBackSkin[eBACKIMAGE_BOTTOM]->mSkinInfo->mHeight;
		mpBackImage[eBACKIMAGE_BOTTOM]->SetScreenRect( x, y + rowHeight + MENU_GAPY, w, skinHeight );
	}
	if( mpBackImage[eBACKIMAGE_LEFT] )
	{
		int skinWidth = mpBackSkin[eBACKIMAGE_LEFT]->mSkinInfo->mWidth;
		mpBackImage[eBACKIMAGE_LEFT]->SetScreenRect( x, y + MENU_GAPY, skinWidth, rowHeight );
	}
	if( mpBackImage[eBACKIMAGE_RIGHT] )
	{
		int skinWidth = mpBackSkin[eBACKIMAGE_RIGHT]->mSkinInfo->mWidth;
		mpBackImage[eBACKIMAGE_RIGHT]->SetScreenRect( x + MENU_GAPX + mInitWidth, y + MENU_GAPY, skinWidth, rowHeight );
	}

	Show( false );
}


//-------------------------------------------------------------------------
cTalkButtonWindow::cTalkButtonWindow()
: mNpcIndex(0)
{
	SetBackupVisible( false );
}

cTalkButtonWindow::~cTalkButtonWindow()
{
	
}

void cTalkButtonWindow::Show( unsigned long px, unsigned long py )
{
	if( HERO->IsOpenNpcTalk() == true )
		return;

	if( mNpcIndex == 0 )
		return;

	int x = (int)( px - (GetAbsoluteRect().GetWidth() * 0.5f) );
	if( x < 0 )
		x = 0;

	int y = (py - GetAbsoluteRect().GetHeight());
	if( y < 0 )
		y = 0;

	SetRelativePos( cUIPos( x, y ) );

	cUIWindow::Show( false );
}

bool cTalkButtonWindow::Create( const cString& skinName )
{
	if( cUINode::CreateBySkinName( skinName, 0, 1 ) == false )
		return false;

	if( IsValid() == false )
		return false;

	SetVisible( false );
	return true;
}

bool cTalkButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	cButton* button = new cButton;
	if( button )
		button->CreateBySkinName( "TalkButton_Button", this, eNODE_COMMON );
	else
	{
		assert(0);
		return false;
	}
	return true;
}

void cTalkButtonWindow::OnCommand( cUINode* , unsigned int id )
{
	if( mNpcIndex == 0 )
	{
		assert(0);
		return;
	}

	cNpc* pNpc = OBJECTMANAGER->GetNpc( mNpcIndex );
	if( pNpc )
	{
		if( HERO->GetState() == eOBJECT_STATE_STOP )
		{
			if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
				return;
		}

		HERO->AfterNpcTalk( pNpc );
	}
	else
		assert(0);

	Hide();
}
