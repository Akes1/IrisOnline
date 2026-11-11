#include "StdAfx.h"
#include "ItemIcon.h"
#include "TipWindow.h"

cItemIcon::cItemIcon( eUINodeType type )
: cIcon( type )
, mSlotIndex( USHRT_MAX )
, mItemIndex( 0 )
, mDBIndex(0)
, mIsSelect( false )
, mpSelectImage(0)
, mLinked( false )
, mpSealImage(0)
, mSeal( false )
{
	SetIconType( eICON_ITEM );

	mImageFont = 0;
	mParam = 0;
	mNumber = 0;
}

cItemIcon::~cItemIcon()
{
	SAFE_DELETE( mpSelectImage );
	SAFE_DELETE( mpSealImage );
}

void cItemIcon::operator=( cItemIcon& other )
{
	mParam = other.mParam;
	mImageFont = other.mImageFont;
	mItemIndex = other.mItemIndex;
	mDBIndex = other.mDBIndex;
	mNumber = other.mNumber;
	mNumText = other.mNumText;
	mGauge = other.mGauge;
	mSeal = other.mSeal;

	ShowGauge( other.mShowGauge );
	ShowNumber( other.mShowNumber );
	SetColor( other.mColor );
	SetUsedFlag( other.mEnableUse );
	SetSelect( other.mIsSelect );
	SetTipType( other.mTipType );

	sAniParam* aniparam = other.mAniParam;
	SetAniParam( aniparam );
}

void cItemIcon::Clear()
{
	mItemIndex = 0;
	mDBIndex   = 0;
	mIsSelect  = false;
	mNumber    = 0;
	mTipType   = eTOOLTIP_NONE;
	mSeal	   = false;
}

bool cItemIcon::OnCreate( cUINodeProperty* pproperty )
{
	if( cIcon::OnCreate( pproperty) == false )
		return false;

	/// 선택 이미지
	cIconParam* pParam = UIMAN->GetIconParam( SELECT_INDEX );
	if( pParam )
	{
		mpSelectImage = new cUIImage( pParam->mpTexture );
		unsigned int tx = pParam->mTexPos.mX;
		unsigned int ty = pParam->mTexPos.mY;
		unsigned int x = GetAbsoluteRect().mLeft;
		unsigned int y = GetAbsoluteRect().mTop;
		unsigned int w = GetAbsoluteRect().GetWidth();
		unsigned int h = GetAbsoluteRect().GetHeight();

		mpSelectImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSelectImage->SetScreenRect( cUIRect(x, y, x+w, y+h) );
	}

	// 미인증 이미지
	pParam = UIMAN->GetIconParam( SEAL_INDEX );
	if( pParam )
	{
		mpSealImage = new cUIImage( pParam->mpTexture );
		unsigned int tx = pParam->mTexPos.mX;
		unsigned int ty = pParam->mTexPos.mY;
		unsigned int x = GetAbsoluteRect().mLeft;
		unsigned int y = GetAbsoluteRect().mTop;
		unsigned int w = GetAbsoluteRect().GetWidth();
		unsigned int h = GetAbsoluteRect().GetHeight();

		mpSealImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSealImage->SetScreenRect( cUIRect(x, y, x+w, y+h) );
	}

	return true;
}

void cItemIcon::UpdateRect()
{
	cIcon::UpdateRect();

	if( mpSelectImage )
		mpSelectImage->SetPos( GetAbsoluteRect().mLeft, GetAbsoluteRect().mTop );

	if( mpSealImage )
		mpSealImage->SetPos( GetAbsoluteRect().mLeft, GetAbsoluteRect().mTop );
}

void cItemIcon::OnRender(cUIFontItemKeeper* pKeeper )
{
	cIcon::OnRender( pKeeper );

	if( mIsSelect && mpSelectImage )
		mpSelectImage->Draw();

	if( mpSealImage && mSeal )
		mpSealImage->Draw();
}

void cItemIcon::OnMouseHovered( const cUIPos& pos )
{
	if( mHoverTipIndex > 0 )
		UIMAN->ShowUITip( pos, mHoverTipIndex );

	if( mLinked == true )
	{
		/// 참조용 아이콘이면 부모에게 이벤트 
		SendEvent( mpParent, eUIEVENT_ICON_HOVERED, this, mID, mShiftDown );
	}
	else
	{
		if( mItemIndex > 0 )
			UIMAN->ShowItemTip( eTIP_ITEM, pos, mItemIndex, mSlotIndex, false );
	}
}

void cItemIcon::OnLButtonDoubleClick( const cUIPos& )
{
	if( mEnableUse && mItemIndex )
	{
		SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, this, mID );
		UIMAN->HideTip();
	}
}

void cItemIcon::OnMouseMove( const cUIPos& )
{
	if( mPressed && mEnableUse && mItemIndex )
	{
		ReleaseCapture();
		mPressed = false;

		SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, this, mID, mShiftDown );
		UIMAN->HideTip();
	}
}
