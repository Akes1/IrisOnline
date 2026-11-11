#include "stdafx.h"
#include "CardImage.h"
#include "UIImage.h"
#include "TarotManager.h"
#include "UIContainer.h"

#include "AniPlane.h"
#include "Application.h"

cCardImage::cCardImage( eUINodeType type )
: cLabel( type )
, mShowImage( false )
, mLock( true )
, mpOpenParam(0)
, mpCloseParam(0)
, mTarotItemIdx(0)
, mOpenImage(0)
, mCloseImage(0)
, mOverImage(0)
{
	mFadeState = eFade_None;
	mAccumTime = 0;
	mFadeTime = 0;

	mProcessEnable = true;
}

cCardImage::~cCardImage()
{
	SAFE_DELETE(mOverImage);
	SAFE_DELETE(mCloseImage);
	SAFE_DELETE(mOpenImage);
}

bool cCardImage::OnCreate( cUINodeProperty* )
{
	/// 뒷면이미지 세팅 ( 카드이미지스크립트 참고 )
	cCardParam* pCloseParam = TAROTMAN->GetImageParam( 22 );
	if( pCloseParam )
	{
		/// 저장
		mpCloseParam = pCloseParam;
	}
	else
	{
		assert(0);
	}

	mOpenImage = new cEffetAniPlane;
	if( mOpenImage->cPlaneObject::Create( "./Data/2DData/taro_open.tga", 0, 0, 80, 120, 0, 0, 80, 120 ) == false )
	{
		SAFE_DELETE(mOpenImage);
	}
	else
	{
		mOpenImage->SetAnimationInfo( 24, 1000/24, false );
	}

	mCloseImage = new cEffetAniPlane;
	if( mCloseImage->cPlaneObject::Create( "./Data/2DData/taro_close.tga", 0, 0, 80, 120, 0, 0, 80, 120 ) == false )
	{
		SAFE_DELETE(mCloseImage);
	}
	else
	{
		mCloseImage->SetAnimationInfo( 24, 1000/24, false );
	}

	mOverImage = new cEffetAniPlane;
	if( mOverImage->cPlaneObject::Create( "./Data/2DData/taro_rollover.tga", 0, 0, 80, 120, 0, 0, 80, 120 ) == false )
	{
		SAFE_DELETE(mOverImage);
	}
	else
	{
		mOverImage->SetAnimationInfo( 24, 1000/24, true );
	}

	return true;
}

bool cCardImage::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_LABEL ) == false )
	{
		assert( 0 && "not label skin type" );
		return false;
	}

	/// 이미지 설정
	if( pskin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( pskin->mpTexture );
		}
		else
		{
			assert(0);
		}

		/// 텍스처 영역 설정
		unsigned int tx = pskin->mSkinInfo->mTexX;
		unsigned int ty = pskin->mSkinInfo->mTexY;
		unsigned int tw = pskin->mSkinInfo->mTexWidth;
		unsigned int th = pskin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );

		/// 이미지 영역을 갱신
		mpImage->SetScreenRect( mAbsoluteRect );
	}

	unsigned int x = pskin->mSkinInfo->mX;
	unsigned int y = pskin->mSkinInfo->mY;
	unsigned int w = pskin->mSkinInfo->mWidth;
	unsigned int h = pskin->mSkinInfo->mHeight;

	SetRelativeRect( cUIRect(x, y, x + w, y + h) );

	return true;
}

bool cCardImage::HandleEvent( const cUIEvent& event )
{
	return cUINode::HandleEvent( event );
}

/// 주의사항 : 
bool cCardImage::IsValid() const
{
	return cUINode::IsValid();
}

void cCardImage::UpdateRect()
{
	cLabel::UpdateRect();

	if( mpImage )
		mpImage->SetScreenRect( mAbsoluteRect );

	if( mOpenImage )
		mOpenImage->SetScreenXY( mAbsoluteRect.mLeft - 10, mAbsoluteRect.mTop - 10 );

	if( mCloseImage )
		mCloseImage->SetScreenXY( mAbsoluteRect.mLeft - 10, mAbsoluteRect.mTop - 10 );

	if( mOverImage )
		mOverImage->SetScreenXY( mAbsoluteRect.mLeft - 10, mAbsoluteRect.mTop - 10 );
}


///
void cCardImage::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	if( mFadeState != eFade_None )
	{
		if( mpImage )
		{
			unsigned long delta = accumTime - mAccumTime;
			switch( mFadeState )
			{
			case eFade_In:
				{
					if( delta >= mFadeTime )
					{
						mpImage->SetAlpha( 1.0f );
						mFadeState = eFade_None;
					}
					else
						mpImage->SetAlpha( (float)delta/(float)mFadeTime );
				}
				break;
			case eFade_Out:
				{
					if( delta >= mFadeTime )
					{
						mpImage->SetAlpha( 0.0f );
						mFadeState = eFade_None;
					}
					else
						mpImage->SetAlpha( 1.0f - (float)delta/(float)mFadeTime );
				}
				break;
			case eFade_In_Out:
				{
					if( delta >= mFadeTime )
					{
						mpImage->SetAlpha( 1.0f );
						ShowImage( true, true, eFade_Out );
						mFadeTime = 500;
						mAccumTime = accumTime;
					}
					else
						mpImage->SetAlpha( (float)delta/(float)mFadeTime );
				}
				break;
			case eFade_Out_In:
				{
					if( delta >= mFadeTime )
					{
						mpImage->SetAlpha( 0.0f );
						ShowImage( true, true, eFade_In );
						mFadeTime = 500;
						mAccumTime = accumTime;
					}
					else
						mpImage->SetAlpha( 1.0f - (float)delta/(float)mFadeTime );
				}
				break;
			default:
				mFadeState = eFade_None;
				break;
			}
		}
	}
	
	if( mOpenImage )
		mOpenImage->Process( accumTime );

	if( mCloseImage )
		mCloseImage->Process( accumTime );

	if( mOverImage )
		mOverImage->Process( accumTime );
}

///
void cCardImage::OnRender( cUIFontItemKeeper* )
{
	if( mpImage )
		mpImage->Draw();

	if( mOpenImage )
		mOpenImage->Draw();

	if( mCloseImage )
		mCloseImage->Draw();

	if( mOverImage )
		mOverImage->Draw();
}

///
void cCardImage::OnLButtonDown( const cUIPos& pos, bool ctrl, bool AltPress, bool shiftPress )
{
	cLabel::OnLButtonDown( pos, ctrl, AltPress, shiftPress );

	/// 락이 안걸려야 뒤집기 가능
	if( mLock == false )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_IMAGE_CLICKED;
		mpParent->HandleEvent( event );
	}
}

///
void cCardImage::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, 0, 0, 0 );
}

void cCardImage::OnMouseHovered(const cUIPos& pos )
{
	SendEvent( mpParent, eUIEVENT_IMAGE_HOVERED, this, mID );

	if( mOverImage )
		mOverImage->BeginAnimation();
}

void cCardImage::OnMouseLeft(const cUIPos& pos )
{
	ReleaseCapture();
	UIMAN->HideTip();

	if( mOverImage )
		mOverImage->EndAnimation();
}

///
void cCardImage::UpdateImage()
{
	cCardParam* param = 0;
	param = ( mShowImage == true ) ? mpOpenParam : mpCloseParam;
	if( param == 0 )
		return;

	/// 기존의 이미지가 없었으면 생성 
	if( mpImage == 0 )
		mpImage = new cUIImage( param->mpTexture );
	else
		mpImage->SetTexture( param->mpTexture );

	/// 텍스처 영역 설정
	unsigned int tx = param->mTexPos.mX;
	unsigned int ty = param->mTexPos.mY;
	unsigned int tw = CARD_WIDTH;
	unsigned int th = CARD_HEIGHT;

	mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );

	/// 이미지 영역을 갱신
	mpImage->SetScreenRect( mAbsoluteRect );
}

void cCardImage::SetDirection( BYTE dir )
{
	if( mpImage )
	{
		if( dir == 0 )
			mpImage->SetRotate( 0 );
		else
			mpImage->SetRotate( 3.14f );
	}
}

///
void cCardImage::SetImage( unsigned long tarotItemIdx, cCardParam* param )
{
	if( param )
	{
		mpOpenParam = param;
		UpdateImage();
	}
	else
	{
		assert(0);
	}

	mTarotItemIdx = tarotItemIdx;
}

void cCardImage::ShowImage( bool show, bool showAni, unsigned char fadeState )
{
	mShowImage = show;

	if( mShowImage == true )
	{
		if( mOpenImage && showAni == true )
			mOpenImage->BeginAnimation();

		SetLock( true );
	}
	else
	{
		if( mCloseImage && showAni == true )
			mCloseImage->BeginAnimation();

		SetLock( false );
	}


	UpdateImage();

	if( mpImage )
	{
		switch( fadeState )
		{
		case eFade_In:
			mFadeTime = 1000;
			mpImage->SetAlpha( 0.0f );
			break;
		case eFade_In_Out:
			mFadeTime = 500;
			mpImage->SetAlpha( 0.0f );
			break;
		case eFade_Out:
			mFadeTime = 1000;
			mpImage->SetAlpha( 1.0f );
			break;
		case eFade_Out_In:
			mFadeTime = 500;
			mpImage->SetAlpha( 1.0f );
			break;
		default:
			return;
		}
		mFadeState = fadeState;
		mAccumTime = THEAPP->GetWorldAccumTime();
	}
}