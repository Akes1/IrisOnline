#include "Stdafx.h"
#include "BarButton.h"

#include "UIEvent.h"
#include "PlaneObject.h"
#include "UIContainer.h"

cBarButton::cBarButton( eUINodeType type )
: cButton( type )
, mpBarImage(0)
, mDragStart( 0, 0 )
{
}

cBarButton::~cBarButton()
{
	SAFE_DELETE( mpBarImage );
}

void cBarButton::Hide()
{
	cUINode::Hide();
}


/// 주의사항 : 
bool cBarButton::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_BARBUTTON ) == false )
	{
		assert( 0 && "not button skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cButtonSkin* p = (cButtonSkin*)pskin;
	mUpPos = p->mUpPos;
	mDownPos = p->mDownPos;
	if( p->mUpOverPos.mX == 0 && p->mUpOverPos.mY == 0 )
		mUpOverPos = p->mUpPos;
	else
		mUpOverPos = p->mUpOverPos;

	if( pskin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short w = (unsigned short)pskin->mSkinInfo->mWidth;
		unsigned short h = (unsigned short)pskin->mSkinInfo->mHeight;
		unsigned short tx = (unsigned short)pskin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)pskin->mSkinInfo->mTexY;

		if( mpBarImage == 0 )
		{
			mpBarImage = new cPlaneObject;
			
			if( mpBarImage->Create( pskin->mpTexture, (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop, 
									w, h, tx, ty, tx + w, ty + h ) == false )
			{
				assert( 0 && "failed to create bar button image");
				return false;
			}

			
		}
		else
		{
			mpBarImage->SetTextureRect( tx, ty, tx + w, ty + h );
		}

		mOriginalTexWidth = w;
		mOriginalTexHeight = h;
	}
	return true;
}

/// 주의사항 : 
void cBarButton::UpdateRect()
{
	cUINode::UpdateRect();

	///
	if( mpBarImage )
	{
		mpBarImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
		mpBarImage->SetScreenWH( (unsigned short)GetAbsoluteRect().GetWidth(), (unsigned short)GetAbsoluteRect().GetHeight() );
	}
}

/// 주의사항 : 
void cBarButton::OnRender( cUIFontItemKeeper* )
{
	/// 이미지 렌더링
	if( mpBarImage )
        mpBarImage->Draw();
}

/// 주의사항 : 
void cBarButton::OnMouseMove( const cUIPos& pos )
{
	if( mPressed )
	{
		// 만약 스크롤바용이라면..
		cUIEvent event;
		event.mType = eUIEVENT_SCROLLBAR_MOVE;
		event.mpCaller = this;
		event.mID = mID;
		event.mPos = pos;
		mpParent->HandleEvent( event );
	}
}

/// 주의사항 : 
void cBarButton::OnLButtonDown( const cUIPos& pos, bool, bool, bool )
{
	/// 캡쳐가 없으면 캡쳐를 갖는다
	if( GetCapture() != this )
	{
		SetCapture();
	}

	mPressed = true;
	mDragStart.mX = pos.mX;
	mDragStart.mY = pos.mY;

	// 소스 이미지 바꾸기
	mpBarImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
							 (unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );

}

/// 주의사항 :
void cBarButton::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, 0, 0, 0 );
}

/// 주의사항 : 
void cBarButton::OnLButtonUp( const cUIPos& )
{
	ReleaseCapture();
	mPressed = false;

	// 소스 이미지 바꾸기
	mpBarImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
							 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
}

/// 주의사항 : 마우스 좌표를 받아 Y 축으로만 이동
void cBarButton::MoveY( int moveY, const cUIPos& mousePos )
{
	Move( cUIPos( 0, moveY ) );
	mDragStart = mousePos;
}

/// 주의사항 : 마우스 좌표를 받아 X 축으로만 이동
void cBarButton::MoveX( int moveX, const cUIPos& mousePos )
{
	Move( cUIPos( moveX, 0 ) );
	mDragStart = mousePos;
}

