#include "Stdafx.h"
#include "ScrollBar.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
#include "PlaneObject.h"
#include "SoundSystem.h"

#include "Button.h"
#include "BarButton.h"

cScrollBar::cScrollBar( eUINodeType type )
: cUIWindow( type )
, mpScrollImage(0)
, mpUpButton(0)
, mpDownButton(0)
, mpBarButton(0)
{	
}

cScrollBar::~cScrollBar()
{
	SAFE_DELETE( mpScrollImage );

	mpBarButton = 0;
	mpDownButton = 0;
	mpUpButton = 0;
}

void cScrollBar::OnShow()
{
	mpBarButton->Show();
	mpDownButton->Show();
	mpUpButton->Show();

	cUIWindow::OnShow();
}

void cScrollBar::OnHide()
{
	cUIWindow::OnHide();

	mpBarButton->Hide();
	mpDownButton->Hide();
	mpUpButton->Hide();
}

void cScrollBar::Show( bool onsound )
{
	if( mVisible )
		return;

	if( onsound && mOpenSoundIdx != ULONG_MAX )
		SOUNDSYS->Play2DSound( mOpenSoundIdx );

	SetVisible( true );

	OnShow();
}

/// 주의사항 : 
bool cScrollBar::OnCreate( cUINodeProperty* )
{
	mpBarButton = new cBarButton;
	if( mpBarButton->CreateBySkinName( mBarButtonSkin, this, eSCROOLBAR_BARBUTTON ) == false )
	{
		return false;
	}

	mpUpButton = new cButton;
	if( mpUpButton->CreateBySkinName( mUpButtonSkin, this, eSCROOLBAR_UP ) == false )
	{
		return false;
	}

	mpDownButton = new cButton;
	if( mpDownButton->CreateBySkinName( mDownButtonSkin, this, eSCROOLBAR_DOWN ) == false )
	{
		return false;
	}

	/// BAR 버튼의 위치는 스크롤바를 기준으로 재조정
	cUIPos pos;
	pos.mX = 0;
	pos.mY = mpUpButton->GetRelativeRect().mBottom;
	mpBarButton->SetRelativePos( pos );

	return true;
}

/// 주의사항 : 
bool cScrollBar::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_SCROLLBAR ) == false )
	{
		assert( 0 && "not editbox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}
	
	cScrollBarSkin* p = (cScrollBarSkin*)pskin;

	mBarButtonSkin = p->mBarButtonSkin;
	mUpButtonSkin = p->mUpButtonSkin;
	mDownButtonSkin = p->mDownButtonSkin;

	cUISkin* pUISkin = UIMAN->GetSkin();
	if( !pskin )
	{
		assert(0);
		return false;
	}

	/// 스킨 네임 유효성 검사
	if( mBarButtonSkin && pUISkin->GetNodeSkin( mBarButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mUpButtonSkin && pUISkin->GetNodeSkin( mUpButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mDownButtonSkin && pUISkin->GetNodeSkin( mDownButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}

	if( pskin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short w = (unsigned short)pskin->mSkinInfo->mWidth;
		unsigned short h = (unsigned short)pskin->mSkinInfo->mHeight;
		unsigned short tx = (unsigned short)pskin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)pskin->mSkinInfo->mTexY;

		mpScrollImage = new cPlaneObject;
		if( mpScrollImage->Create( pskin->mpTexture, 0, 0, w, h, tx, ty, tx + w, ty + h ) == false )
		{
			assert( 0 && "failed to create scroll back image");
			return false;
		}
	}

	/// 자체이미지를 따로 생성해서 사용하므로 필요없음 
	SAFE_DELETE( mpImage );

	return true;
}

/// 주의사항 : 
bool cScrollBar::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return true;
	}

	if( event.mType == eUIEVENT_SCROLLBAR_MOVE )
	{
		UpdateBarButtonPos( event.mPos );
	}
	if( event.mType == eUIEVENT_LBUTTON_DBLCLK )
		return true;
	if( event.mType == eUIEVENT_LBUTTON_DOWN ||
		event.mType == eUIEVENT_MOUSE_MOVE )
		return true;

	return mpParent->HandleEvent( event );
}

/// 주의사항 : 
void cScrollBar::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	
	/// BAR 버튼의 위치는 스크롤바를 기준으로 재조정
	if( mpUpButton && mpBarButton )
	{
		cUIPos pos;
		pos.mX = 0;
		pos.mY = mpUpButton->GetRelativeRect().mBottom;
		mpBarButton->SetRelativePos( pos );
	}
}

///
void cScrollBar::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpScrollImage )
	{
		mpScrollImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	}
}

/// 주의사항 : 
void cScrollBar::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpScrollImage )
	{
		mpScrollImage->Draw();
	}

    /// 자식 렌더링
	cUIWindow::OnRender(pKeeper);
}

/// 주의사항 : 스크롤바의 바버튼이 움직일경우, 
void cScrollBar::UpdateBarButtonPos( const cUIPos& pos )
{
	/// 스크롤바가 그려지지 않는 상태면 리턴
	if( mpBarButton->IsVisible() == false )
		return;

	cUIRect child = mpBarButton->GetAbsoluteRect();
	cUIPos& oldPos = mpBarButton->GetOldMousePos();

	int uppos = mpUpButton->GetAbsoluteRect().mBottom;
	int downpos = mpDownButton->GetAbsoluteRect().mTop;
	int value = 0;

	/// 위쪽 이동이면
	cUIRect parentRc = mpParent->GetAbsoluteRect();
	if( parentRc.ContainPoint( pos ) == false )
		return;

	if( pos.mY - oldPos.mY < 0 )
	{
		if( child.mTop > uppos )
		{
			if( child.mTop + pos.mY - oldPos.mY < uppos)
			{
				/// 최소 위치까지 보정
				value = uppos - child.mTop;
			}
			else
			{
				/// 마우스 이동량 만큼 이동
				value = pos.mY - oldPos.mY;
			}
		}
		mpBarButton->MoveY( value , pos );
	}
	/// 아래쪽 이동이면
	else if( pos.mY - oldPos.mY > 0 )
	{
		if( child.mBottom < downpos )
		{
			/// 
			if( child.mBottom + pos.mY - oldPos.mY >= downpos )
			{
				/// 최대 위치까지 보정
				value = downpos - child.mBottom;
			}
			else
			{
				/// 마우스 이동량 만큼 이동
				value = pos.mY - oldPos.mY;
			}
		}
		mpBarButton->MoveY( value , pos );
	}
}

/// 주의사항 : 
void cScrollBar::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );

	if( GetCapture() != this )
	{
		SetCapture();
	}
}

/// 주의사항 : 
void cScrollBar::OnLButtonUp( const cUIPos& pos )
{
	/// 영역 밖에서 마우스를 놔버리면
	if( mAbsoluteRect.ContainPoint( pos ) == false )
	{
		ReleaseCapture();
	}
}

/// 주의사항 : 스크롤바의 초기 X 위치 세팅 
void cScrollBar::SetPosX( unsigned int x )
{
	if( mpScrollImage )
	{
		cUIPos pos;
		pos.mX = x;
		pos.mY = GetRelativeRect().mTop;
		SetRelativePos( pos );

		mpScrollImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	}
}

void cScrollBar::SetPosY( unsigned int y )
{
	if( mpScrollImage )
	{
		cUIPos pos;
		pos.mX = GetRelativeRect().mLeft;
		pos.mY = y;
		SetRelativePos( pos );

		mpScrollImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	}
}

/// 주의사항 : 스크롤바의 높이값 세팅 
void cScrollBar::SetHeight( unsigned int h )
{
	if( mpScrollImage && mpDownButton )
	{
		cUISize size;
		size.mWidth = GetRelativeRect().GetWidth();
		size.mHeight = h;
		SetRelativeSize( size );

		mpScrollImage->SetScreenWH( (short)GetAbsoluteRect().GetWidth(), (short)GetAbsoluteRect().GetHeight() );

		/// DOWN 버튼의 위치는 스크롤바를 기준으로 재조정 
		cUIPos pos;
		pos.mX = 0;
		pos.mY = h - mpDownButton->GetRelativeRect().GetHeight();
		mpDownButton->SetRelativePos( pos );
	}
}

/// 주의사항 : 스크롤바 전체 길이가 적을때는 바버튼만 안보여준다.
void cScrollBar::SetVisibleBarButton( bool visible )
{
	if( !mpBarButton )
		return;

	if( visible )
		mpBarButton->Show();
	else
		mpBarButton->Hide();
}

/// 주의사항 : 스크롤바 높이 값 얻기 ( 바 버튼이 스크롤 가능한 영역만 계산 )
unsigned int cScrollBar::GetScrollBarHeight()
{
	return ( GetAbsoluteRect().GetHeight() - (2 * mpUpButton->GetAbsoluteRect().GetHeight()) );
}

unsigned int cScrollBar::GetScrollButtonHeight()
{
	return (mpUpButton) ? mpUpButton->GetAbsoluteRect().GetHeight() : 0;
}

/// 주의사항 : 바 버튼의 위치 얻기 
unsigned int cScrollBar::GetBarButtonPosY()
{
	return (mpBarButton->GetRelativeRect().mTop - mpUpButton->GetRelativeRect().GetHeight() );
}

/// 주의사항 : 스크롤 [가능한] 영역 사이즈 리턴
unsigned int cScrollBar::GetScrollMarginHeight()
{ 
	/// 스크롤바 길이 - 바 버튼 길이 - 업, 다운 버튼 길이 
	return ( GetAbsoluteRect().GetHeight() - mpBarButton->GetAbsoluteRect().GetHeight() -
			 (2 * mpUpButton->GetAbsoluteRect().GetHeight()) );
}

/// 주의사항 : 바 버튼이 작아질수 있는 최소 사이즈 
unsigned int cScrollBar::GetMinimumHeight()
{
	return mpUpButton->GetAbsoluteRect().GetHeight();
}

/// 주의사항 : 스크롤바의 윗 버튼이 눌렸을경우의 처리
void cScrollBar::MoveScrollUp( int h )
{
	/// 바 버튼의 top 위치
	int toplimit = mpBarButton->GetAbsoluteRect().mTop;
	/// up 버튼의 bottom 위치
	int bottomlimit = mpUpButton->GetAbsoluteRect().mBottom;

	/// top 위치가 bottom 작으면 바 버튼이 up버튼을 뚫고 지나가는 현상 발생
	if( toplimit >= bottomlimit )
	{
		int value = toplimit - bottomlimit;
		/// 남은 영역이 h 보다 적으면,
		if( h > value )
		{
			mpBarButton->Move( cUIPos( 0, -value ) );
		}
		else
		{
			mpBarButton->Move( cUIPos( 0, -h ) );
		}
	}
}

/// 주의사항 : 스크롤바의 아래 버튼이 눌렸을 경우의 처리
void cScrollBar::MoveScrollDown( int h )
{
	/// 바 버튼의 bottom 위치
	int bottomlimit = mpBarButton->GetAbsoluteRect().mBottom;
	/// down 버튼의 top 위치
	int toplimit = mpDownButton->GetAbsoluteRect().mTop;

	/// bottom 위치가 top 작으면 바 버튼이 down버튼을 뚫고 지나가는 현상 발생
	if( bottomlimit <= toplimit )
	{
		int value = toplimit - bottomlimit;
		/// 남은 영역이 h 보다 적으면,
		if( h > value )
		{
			mpBarButton->Move( cUIPos( 0, value ) );
		}
		else
		{
			mpBarButton->Move( cUIPos( 0, h ) );
		}		
	}
}

/////////////////////////////////////////////////////////////////////////
cScrollBarSkin::cScrollBarSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cScrollBarSkin::~cScrollBarSkin()
{

}

/// 주의사항 : 
bool cScrollBarSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_BARBUTTONSKIN:
			{
				mBarButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_UPBUTTONSKIN:
			{
				mUpButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_DOWNBUTTONSKIN:
			{
				mDownButtonSkin = parser.ParseString();
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}		
	}
	return true;
}

