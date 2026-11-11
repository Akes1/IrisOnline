#include "StdAfx.h"
#include "TodayWordWindow.h"
#include "PlaneObject.h"
#include "EditBox.h"
#include "FilterManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"

cTodayWordWindow::cTodayWordWindow()
: mpSelectImage(0)
, mSelectIndex(0)
, mpOkBtn(0)
, mpEdit(0)
{
	memset( mColorRect, 0, sizeof(mColorRect) );
}

cTodayWordWindow::~cTodayWordWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cTodayWordWindow::Open()
{
}

void cTodayWordWindow::Close()
{
	cUIWindow::Close();

	if( mpEdit )
		mpEdit->Clear();
}

void cTodayWordWindow::OnShow()
{
	cUIWindow::OnShow();

	mSelectIndex = 0;
	UpdateSelectImage();
}

void cTodayWordWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpEdit )
		mpEdit->Clear();
}

bool cTodayWordWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpOkBtn = GetChild( eUIID_GAME_TODAYWORD_OK );
	mpEdit = (cEditBox*)GetChild( eUIID_GAME_TODAYWORD_TEXT );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return 0;
	}

	cString str;
	for( unsigned int i = 0; i < COLOR_COUNT; ++i )
	{
		str.Format( "TodayColor_Rect%02d", i );
		cUINodeSkin* pRectSkin = pSkin->GetNodeSkin( str );
		if( pRectSkin )
		{
			mColorRect[i].mLeft = pRectSkin->mSkinInfo->mX;
			mColorRect[i].mTop = pRectSkin->mSkinInfo->mY;
			mColorRect[i].mRight = mColorRect[i].mLeft + pRectSkin->mSkinInfo->mWidth;
			mColorRect[i].mBottom = mColorRect[i].mTop + pRectSkin->mSkinInfo->mHeight;
		}
	}

	/// 선택 이미지 생성
	cUINodeSkin* imageSkin = pSkin->GetNodeSkin( "TodayColor_Select" );
	if( imageSkin )
	{
		unsigned int tx = imageSkin->mSkinInfo->mTexX;
		unsigned int ty = imageSkin->mSkinInfo->mTexY;
		unsigned int tw = imageSkin->mSkinInfo->mWidth;
		unsigned int th = imageSkin->mSkinInfo->mHeight;

		mpSelectImage = new cPlaneObject;
		if( mpSelectImage->Create( imageSkin->mpTexture, 0, 0, tw, th,
			tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create select image");
			return false;
		}
	}
	return true;
}

void cTodayWordWindow::UpdateRect()
{
	cUIWindow::UpdateRect();
	UpdateSelectImage();
}

void cTodayWordWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpSelectImage )
		mpSelectImage->Draw();
}

void cTodayWordWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TODAYWORD_CLOSE:
	case eUIID_GAME_TODAYWORD_CANCEL:
		{
			sPlayerExrInfo* exrInfo = HERO->GetPlayerExrInfo();
			if( exrInfo )
			{
				HERO->SetTodayWord( exrInfo->mTodayWord, exrInfo->mTodayColor );
			}

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_TODAYWORD_EXIT, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 337 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case eUIID_GAME_TODAYWORD_PREVIEW:
		{
			if( mpEdit )
			{
				HERO->PreviewTodayWord( (LPCTSTR)mpEdit->GetText(), GetWordColor( mSelectIndex ) );
			}
			else
			{
				assert(0);
			}
		}
		break;
	case eUIID_GAME_TODAYWORD_OK:
		{
			if( mpEdit )
			{
				LPCTSTR word = mpEdit->GetText();
				if( mpEdit->GetLength() >= 0 )
				{
					/// 필터 검사
					if( FILTERMAN->CheckPartMatch( word ) == true )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 336 ),
							GAMERESOURCEMAN->GetGameText( 301 ),
							0, (cUIWindow*)GAMEUI->GetTodayWordWindow() );
						return;
					}
				}

				/// 오늘의 한마디 입력
				MSG_REQ_PLAYER_TODAYWORD_CHANGE msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_PLAYER;
				msg.Protocol = NM_PLAYER_TODAYWORD_CHANGE_REQ;
				msg.mWordColor = GetWordColor( mSelectIndex );

				if( mpEdit->GetLength() > 0 )
					::Sstrncpy( msg.mWord, MAX_TODAYWORD_BUFFER_SIZE, word, MAX_TODAYWORD_SIZE );

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				///
				NetLock();
			}
		}
		break;
	}
}

void cTodayWordWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, true );

	for( unsigned short i = 0; i < COLOR_COUNT; ++i )
	{
		cUIRect rc = mColorRect[i];
		rc.mLeft += GetAbsoluteRect().mLeft;
		rc.mTop += GetAbsoluteRect().mTop;
		rc.mRight = rc.mLeft + mColorRect[i].GetWidth();
		rc.mBottom = rc.mTop + mColorRect[i].GetHeight();

		if( rc.ContainPoint( pos ) == true )
		{
			mSelectIndex = i;
			UpdateSelectImage();
			break;
		}
	}
}

void cTodayWordWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, true, true, true );
}

void cTodayWordWindow::UpdateSelectImage()
{
	if( mpSelectImage )
	{
		mpSelectImage->SetScreenRect( mColorRect[mSelectIndex].mLeft + GetAbsoluteRect().mLeft, 
										mColorRect[mSelectIndex].mTop + GetAbsoluteRect().mTop, 
										mColorRect[mSelectIndex].GetWidth(), mColorRect[mSelectIndex].GetHeight() );
	}
}

unsigned long cTodayWordWindow::GetWordColor( int arrIdx )
{
	unsigned long color = WORD_WHITE;
	switch( arrIdx )
	{
	case 0:	color = WORD_WHITE; break;
	case 1: color = WORD_BROWN; break;
	case 2: color = WORD_RED; break;
	case 3:	color = WORD_ORANGE; break;
	case 4:	color = WORD_YELLOW; break;
	case 5:	color = WORD_BRIGHTGREEN; break;
	case 6:	color = WORD_GREEN; break;
	case 7:	color = WORD_BRIGHTBLUE; break;
	case 8:	color = WORD_BLUE; break;
	case 9:	color = WORD_PINK; break;
	case 10:color = WORD_PURPLE; break;
	case 11:color = WORD_BLACK; break;
	}
	return color;
}

void cTodayWordWindow::OnNetLock( int lockTry )
{
	if( mpOkBtn )
		mpOkBtn->NetLock( lockTry );
}

void cTodayWordWindow::OnNetUnLock( int lockTry )
{
	if( mpOkBtn )
		mpOkBtn->NetUnLock( lockTry );
}