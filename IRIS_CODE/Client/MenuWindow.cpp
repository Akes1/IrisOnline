#include "StdAfx.h"
#include "MenuWindow.h"
#include "GameUIManager.h"
#include "Label.h"
#include "Button.h"
#include "ExitWindow.h"
#include "RenderSystem.h"
#include "UIManager.h"
#include "TarotManager.h"
#include "GameResourceManager.h"
#include "ItemMixWindow.h"
#include "ItemMixManager.h"
#include "TarotManager.h"
#include "UserSellWindow.h"
#include "EnhancedWindow.h"
#include "InventoryWindow.h"
#include "TwitterWindow.h"
#include "ItemReceiptWindow.h"
#include "SkillWindow.h"

cMenuButtonWindow::cMenuButtonWindow()
{
	memset( mpMenu, 0, sizeof(mpMenu) );
}
cMenuButtonWindow::~cMenuButtonWindow()
{
}

bool cMenuButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	UIMAN->AddEventOtherClick( this );
	return true;
}

void cMenuButtonWindow::AddMenu( unsigned int i, cUIWindow* p )
{
	if( i < eMENU_MAX && p )
	{
		mpMenu[i] = p;
	}
	else
	{
		assert(0);
	}
}

/// 주의사항 : 
void cMenuButtonWindow::OnCommand( cUINode*, unsigned int id )
{
	eMenuType menuType = eMENU_MAX;

	switch( id )
	{
	
	case eUIID_GAME_MENUBUTTON_HERO:		menuType = eMENU_HERO;		break;
	case eUIID_GAME_MENUBUTTON_COMMU:		menuType = eMENU_COMMU;		break;
	case eUIID_GAME_MENUBUTTON_OPTION:		menuType = eMENU_OPTION;	break;
	case eUIID_GAME_MENUBUTTON_LIFE:		menuType = eMENU_LIFE;		break;
	case eUIID_GAME_MENUBUTTON_TWITTER:		
		{
			menuType = eMENU_TWITTER;		
			cTwitterWindow* pTwit = GAMEUI->GetTwitterWindow();
			if( pTwit )
			{
				if( pTwit->IsVisible() )
				{
					pTwit->Hide();
				}
				else
				{
					pTwit->Show();

					/// 나머지 메뉴 숨기기
					for( int i = 0; i < eMENU_MAX; ++i )
					{
						if( mpMenu[i] && i != menuType )
							mpMenu[i]->Hide();
					}
				}
			}
		}
		break;
	case eUIID_GAME_MENUBUTTON_RECEIPT:
		{
			menuType = eMENU_RECEIPT;
			cItemReceiptWindow* pReceiptWIn = GAMEUI->GetItemReceiptWindow();
			if( pReceiptWIn )
			{
				if( pReceiptWIn->IsVisible() )
				{
					pReceiptWIn->Hide();
				}
				else
				{
					pReceiptWIn->Show();

					/// 나머지 메뉴 숨기기
					for( int i = 0; i < eMENU_MAX; ++i )
					{
						if( mpMenu[i] && i != menuType )
							mpMenu[i]->Hide();
					}
				}
			}
		}
		break;

	default: assert(0); return;
	}

	if( menuType == eMENU_MAX )
		return;

	if( eMENU_HERO <= menuType && menuType <= eMENU_LIFE)
	{
		if( mpMenu[menuType] )
		{
			if( mpMenu[menuType]->IsVisible() )
			{
				mpMenu[menuType]->Hide();
			}
			else
			{
				/// 나머지 메뉴 숨기기
				for( int i = 0; i < eMENU_MAX; ++i )
				{
					if( mpMenu[i] && i != menuType )
						mpMenu[i]->Hide();
				}

				mpMenu[menuType]->Show();
			}
		}
	}	
	
}

///-------------------------------------------------------------------------
cMenuHeroWindow::cMenuHeroWindow()
: mpReceipt(0)
{
}

cMenuHeroWindow::~cMenuHeroWindow()
{
}

bool cMenuHeroWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpReceipt = GetChild( eUIID_GAME_MENUHERO_RECEIPT );

	UIMAN->AddEventOtherClick( this );
	return true;
}

bool cMenuHeroWindow::SetSkin( const cUINodeSkin* skin )
{
	if( skin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( skin->mpTexture );
		}
		else
			assert(0);

		/// 컬러를 지정
		SetColor( skin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - skin->mSkinInfo->mX;
	int y = GetScreenHeight() - skin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

void cMenuHeroWindow::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}

	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - pnodeSkin->mSkinInfo->mX;
	int y = GetScreenHeight() - pnodeSkin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	mHoverTipIndex = pnodeSkin->mSkinInfo->mHoverTipIndex;

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();
}

void cMenuHeroWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_MENUHERO_CLOSE:
		break;
	case eUIID_GAME_MENUHERO_INFO:
		GAMEUI->ToggleHeroInfoWindow();
		break;
	case eUIID_GAME_MENUHERO_INVENTORY:
		GAMEUI->ToggleInventoryWindow();
		break;	
	case eUIID_GAME_MENUHERO_QUEST:
		GAMEUI->ToggleQuestWindow();
		break;
	case eUIID_GAME_MENUHERO_THEMESTATE:
		GAMEUI->ToggleThemeStateWindow();
		break;
	case eUIID_GAME_MENUHERO_SKILL:
		{
			cSkillWindow* pSkillWin = GAMEUI->GetSkillWindow();

			if( pSkillWin )
			{
				if( pSkillWin->IsVisible() )
				{
					cCommuSheetWindow* pCurSheetWin = (cCommuSheetWindow*)pSkillWin->GetCurrentSheet();
					cCommuSheetWindow* pCommuWin = pSkillWin->GetCommunityWindow();

					if( pCurSheetWin && pCommuWin )
					{
						if( pCurSheetWin == pCommuWin )
						{
							pSkillWin->Hide();
							pSkillWin->Show();							
						}
						else
						{
							pSkillWin->Hide();
						}

					}					
				}
				else
				{
					pSkillWin->Show();
				}
			}
		}		
		break;
	case eUIID_GAME_MENUHERO_COMMUNITY:
		{			
			cSkillWindow* pSkillWin = GAMEUI->GetSkillWindow();

			if( pSkillWin )
			{
				if( pSkillWin->IsVisible() )
				{
					cCommuSheetWindow* pCurSheetWin = (cCommuSheetWindow*)pSkillWin->GetCurrentSheet();
					cCommuSheetWindow* pCommuWin = pSkillWin->GetCommunityWindow();

					if( pCurSheetWin && pCommuWin )
					{
						if(  pCurSheetWin == pCommuWin )
						{
							pSkillWin->Hide();
						}
						else
						{
							pSkillWin->Show();
							pSkillWin->SetCurrent(0);	// 0번이 Community Index
						}						
					}					
				}
				else
				{
					pSkillWin->Show();
					pSkillWin->SetCurrent(0);	// 0번이 Community Index
				}
			}	
		}
		break;		
	}

	Hide();
}

void cMenuHeroWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cUIWindow* win = GAMEUI->GetMenuButtonWindow();
	if( win )
	{
		cUIRect winRc = win->GetAbsoluteRect();
		if( winRc.ContainPoint( pos ) == false && 
			mAbsoluteRect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}

void cMenuHeroWindow::OnNetLock( int lockTry )
{
	if( mpReceipt )
		mpReceipt->NetLock( lockTry );
}

void cMenuHeroWindow::OnNetUnLock( int lockTry )
{
	if( mpReceipt )
		mpReceipt->NetUnLock( lockTry );
}

///-------------------------------------------------------------------------
cMenuLifeWindow::cMenuLifeWindow()
{
}

cMenuLifeWindow::~cMenuLifeWindow()
{
}

bool cMenuLifeWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	UIMAN->AddEventOtherClick( this );
	return true;
}

bool cMenuLifeWindow::SetSkin( const cUINodeSkin* skin )
{
	if( skin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( skin->mpTexture );
		}
		else
			assert(0);

		/// 컬러를 지정
		SetColor( skin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - skin->mSkinInfo->mX;
	int y = GetScreenHeight() - skin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

void cMenuLifeWindow::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}

	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - pnodeSkin->mSkinInfo->mX;
	int y = GetScreenHeight() - pnodeSkin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	mHoverTipIndex = pnodeSkin->mSkinInfo->mHoverTipIndex;

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();
}

void cMenuLifeWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_MENULIFE_CLOSE:
		break;

	case eUIID_GAME_MENULIFE_ENHANCE:
		{			
			cEnhancedWindow *pEnHancedWin = GAMEUI->GetEnhancedWindow();
			if( pEnHancedWin )
			{
				if( pEnHancedWin->IsVisible() == false )
					pEnHancedWin->Show();
				else
					pEnHancedWin->Hide();
			}

			cInventoryWindow* pInvenWin = GAMEUI->GetInventoryWindow();
			if( pInvenWin )
			{
				if( pEnHancedWin->IsVisible() == false )
					pInvenWin->Hide();
				else
					pInvenWin->Show();
			}
		}
		break;

	case eUIID_GAME_MENULIFE_PUTCARD:
		GAMEUI->TogglePutCardWindow();
		break;

	case eUIID_GAME_MENULIFE_TAROT:
		TAROTMAN->TarotSelfOpen();
		break;

	case eUIID_GAME_MENULIFE_USERSELL:
		{
			cUserSellWindow* pUserSell = GAMEUI->GetUserSellWindow();
			if( pUserSell && pUserSell->IsVisible() == false )
				ITEMMAN->TryToUserSellOpen();
		}
		break;

	case eUIID_GAME_MENULIFE_ITEMMIX:
		{
			cItemMixWindow* pMix = GAMEUI->GetItemMixWindow();
			if( pMix )
			{
				if( pMix->IsVisible() == false )
				{
					ITEMMIXMAN->ItemMixOpen();
				}
				else
				{
					ITEMMIXMAN->ItemMixCancel();
				}
			}
		}
		break;

	case eUIID_GAME_MENULIFE_TITLE:
		GAMEUI->ToggleTitleWindow();
		break;

	case eUIID_GAME_MENULIFE_MAP: 
		/// 큰지도 열기		
		UIMAN->ShowGameUI();
		GAMEUI->ToggleMiniMapWindow();
		break;

	case eUIID_GAME_MENULIFE_CARDBOOK:		
		GAMEUI->ToggleCardBookWindow();
		break;
	}

	Hide();
}

void cMenuLifeWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cUIWindow* win = GAMEUI->GetMenuButtonWindow();
	if( win )
	{
		cUIRect winRc = win->GetAbsoluteRect();
		if( winRc.ContainPoint( pos ) == false && 
			mAbsoluteRect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}


///-------------------------------------------------------------------------
cMenuCommuityWindow::cMenuCommuityWindow()
{
}

cMenuCommuityWindow::~cMenuCommuityWindow()
{
}

bool cMenuCommuityWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	UIMAN->AddEventOtherClick( this );
	return true;
}

bool cMenuCommuityWindow::SetSkin( const cUINodeSkin* skin )
{
	if( skin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( skin->mpTexture );
		}
		else
			assert(0);

		/// 컬러를 지정
		SetColor( skin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - skin->mSkinInfo->mX;
	int y = GetScreenHeight() - skin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

void cMenuCommuityWindow::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}

	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - pnodeSkin->mSkinInfo->mX;
	int y = GetScreenHeight() - pnodeSkin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	mHoverTipIndex = pnodeSkin->mSkinInfo->mHoverTipIndex;

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();
}

void cMenuCommuityWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_MENUCOMMU_CLOSE:
		break;
	case eUIID_GAME_MENUCOMMU_FRIEND:
		GAMEUI->ToggleFriendWindow();
		break;
	case eUIID_GAME_MENUCOMMU_GUILD:
		GAMEUI->ToggleGuildWindow();
		break;
	case eUIID_GAME_MENUCOMMU_SOCIAL:
		GAMEUI->ToggleSocialWindow();
		break;
	}

	Hide();
}

void cMenuCommuityWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cUIWindow* win = GAMEUI->GetMenuButtonWindow();
	if( win )
	{
		cUIRect winRc = win->GetAbsoluteRect();
		if( winRc.ContainPoint( pos ) == false && 
			mAbsoluteRect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}

///-------------------------------------------------------------------------
cMenuOptionWindow::cMenuOptionWindow()
: mpChange(0)
, mpExit(0)
{
}

cMenuOptionWindow::~cMenuOptionWindow()
{
}

bool cMenuOptionWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpChange = GetChild( eUIID_GAME_MENUOPTION_CHANGE );
	mpExit = GetChild( eUIID_GAME_MENUOPTION_EXIT );

	UIMAN->AddEventOtherClick( this );
	return true;
}


bool cMenuOptionWindow::SetSkin( const cUINodeSkin* skin )
{
	if( skin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( skin->mpTexture );
		}
		else
			assert(0);

		/// 컬러를 지정
		SetColor( skin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - skin->mSkinInfo->mX;
	int y = GetScreenHeight() - skin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

void cMenuOptionWindow::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}

	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;
	int x = GetScreenWidth() - pnodeSkin->mSkinInfo->mX;
	int y = GetScreenHeight() - pnodeSkin->mSkinInfo->mY;
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	mHoverTipIndex = pnodeSkin->mSkinInfo->mHoverTipIndex;

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();
}


void cMenuOptionWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_MENUOPTION_CLOSE:
		break;
	case eUIID_GAME_MENUOPTION_HELP:
		UIMAN->ShowGameUI();
		GAMEUI->ToggleShortcutWindow();
		break;
	case eUIID_GAME_MENUOPTION_SETTING:
		GAMEUI->ToggleOptionWindow();
		break;
	case eUIID_GAME_MENUOPTION_CHANGE:
		GAMEUI->ReqFinish( eEXIT_GAMEOUT );
		break;
	case eUIID_GAME_MENUOPTION_EXIT:
		GAMEUI->ReqFinish( eEXIT_DISCONNECT );
		break;
	case eUIID_GAME_MENUOPTION_SHORTCUT:
		GAMEUI->ToggleShortcutWindow();
		break;
	case eUIID_GAME_MENUOPTION_VIDEO:

		if( RENDERSYS )
		{
			if( RENDERSYS->IsVideoCapturing() )
				RENDERSYS->ToggleVideoCapture();
			else
			{
				UIMAN->CreateMsgBox( eStage_Make, eMSGBOX_MODAL,
					eMBEVENT_VIDEO_CAPTURE, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 10003 ),
					GAMERESOURCEMAN->GetGameText( 2 ) // 알림
					);
			}			
		}

		break;
	}

	Hide();
}

void cMenuOptionWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cUIWindow* win = GAMEUI->GetMenuButtonWindow();
	if( win )
	{
		cUIRect winRc = win->GetAbsoluteRect();
		if( winRc.ContainPoint( pos ) == false && 
			mAbsoluteRect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}

void cMenuOptionWindow::OnNetLock( int lockTry )
{
	if( mpChange )
		mpChange->NetLock( lockTry );

	if( mpExit )
		mpExit->NetLock( lockTry );
}

void cMenuOptionWindow::OnNetUnLock( int lockTry )
{
	if( mpChange )
		mpChange->NetUnLock( lockTry );

	if( mpExit )
		mpExit->NetUnLock( lockTry );
}
