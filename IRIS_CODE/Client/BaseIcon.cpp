#include "StdAfx.h"
#include "BaseIcon.h"
#include "Token.h"
#include "Parser.h"
#include "UIContainer.h"
#include "PlaneObject.h"
#include "Lexer.h"

cBaseIcon::cBaseIcon( eUINodeType type )
: cUINode( type )
, mParam( 0 )
, mShiftDown( false )
, mEnableUse( true )
, mIconType( eICON_NORMAL )
, mColor( NiColorA::WHITE )
{
	mProcessEnable = true;
}

cBaseIcon::~cBaseIcon()
{
}

void cBaseIcon::SetUsedFlag( bool enable, bool colorchange )
{
	mEnableUse = enable;

	if( enable == false )
	{
		if( colorchange )
			SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
	}
	else
	{
		SetColor( mColor );
	}
}

void cBaseIcon::ChangeImage( cIconParam* param, unsigned int texW, unsigned int texH )
{
	mParam = param;

	if( param == 0 )
		return;

	assert( param->mpTexture );

	//	/// 텍스처 설정
	if( mpImage == 0 )
		mpImage = new cUIImage( param->mpTexture );
	else
		mpImage->SetTexture( param->mpTexture );

	///
	mpImage->SetColor( mColor );

	/// 텍스처 영역 설정
	unsigned int tx = param->mTexPos.mX;
	unsigned int ty = param->mTexPos.mY;

	unsigned int tw = texW;
	unsigned int th = texH;

	if( texW == 0 || texH == 0 )
	{
		tw = mRelativeRect.GetWidth();
		th = mRelativeRect.GetHeight();
	} 

	mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );

	/// 이미지 영역을 갱신
	mpImage->SetScreenRect( mAbsoluteRect );
}

void cBaseIcon::ClearImage()
{
	mParam = NULL;
	
	/// 텍스처 설정
	if( mpImage )
		delete mpImage;
	
	mpImage = NULL;
}

void cBaseIcon::UpdateRect()
{
	cUINode::UpdateRect();

	/// 이미지 영역을 갱신
	if( mParam && mpImage )
		mpImage->SetScreenRect( mAbsoluteRect );
}

bool cBaseIcon::SetSkin( const cUINodeSkin* skin )
{
	if( skin->IsKindof( eUINODE_ICON ) == false )
	{
		assert( 0 && "not icon skin type" );
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
			mpImage = new cUIImage(skin->mpTexture);
		}
		else
		{
			assert(0);
		}

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

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return false;
	}

	unsigned int x = skin->mSkinInfo->mX;
	unsigned int y = skin->mSkinInfo->mY;
	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		x = ( GetScreenWidth() * skin->mSkinInfo->mX ) / mDefaultWidth;
		y = ( GetScreenHeight() * skin->mSkinInfo->mY ) / mDefaultHeight;

		/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
		/// 붙어있도록 한다 
		if( (skin->mSkinInfo->mX + skin->mSkinInfo->mWidth) >= (int)mDefaultWidth )
		{
			x = GetScreenWidth() - w;
		}

		if( (skin->mSkinInfo->mY + skin->mSkinInfo->mHeight) >= (int)mDefaultHeight )
		{
			y = GetScreenHeight() - h;
		}
	}

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	/// 
	mHoverTipIndex = skin->mSkinInfo->mHoverTipIndex;
	return true;
}

bool cBaseIcon::HandleEvent( const cUIEvent& event )
{
	if( mParam == 0 )
	{
		if( !(mHoverTipIndex > 0 && (event.mType == eUIEVENT_MOUSE_HOVERED || event.mType == eUIEVENT_MOUSE_LEFT)) )
			return mpParent->HandleEvent( event );
	}

	return cUINode::HandleEvent( event );
}

void cBaseIcon::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mParam && mpImage )
		mpImage->Draw();
}

void cBaseIcon::OnMouseHovered( const cUIPos& pos )
{
	if( mHoverTipIndex > 0 )
		UIMAN->ShowUITip( pos, mHoverTipIndex );

	SendEvent( mpParent, eUIEVENT_ICON_HOVERED, this, mID, mShiftDown );
}

void cBaseIcon::OnMouseLeft( const cUIPos& )
{
	ReleaseCapture();
	mPressed = false;
	UIMAN->HideTip();
}

void cBaseIcon::OnMouseMove( const cUIPos& )
{
	if( mPressed && mEnableUse )
	{
		ReleaseCapture();
		mPressed = false;

		SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, this, mID, mShiftDown );
		UIMAN->HideTip();
	}
}

void cBaseIcon::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	if( mEnableUse )
	{
		cUINode::OnLButtonDown( pos, ctrl, alt, shift );

		if( GetCapture() != this )
		{
			SetCapture();
		}

		mPressed = true;
		mShiftDown = shift;

		SendEvent( mpParent, eUIEVENT_ICON_CLICKED, this, mID );
	}
}

void cBaseIcon::OnLButtonUp( const cUIPos& /*pos*/ )
{
	ReleaseCapture();
	mPressed = false;
}

void cBaseIcon::OnLButtonDoubleClick( const cUIPos& /*pos*/ )
{
	if( mEnableUse )
		SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, this, mID );
}

void cBaseIcon::OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnRButtonDown( pos, ctrl, alt, shift );

	if( mEnableUse )
		SendEvent( mpParent, eUIEVENT_ICON_RIGHTCLICKED, this, mID, shift );
}


////////////////////////////////////////////////////////////////////////////////////////
cIconSkin::cIconSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cIconSkin::~cIconSkin()
{
}

bool cIconSkin::Load( cParser& parser )
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
			break;

		if( cUINodeSkin::ParseLine( parser, token ) == false )
		{
			return false;
		}
	}
	return true;
}

