#include "stdafx.h"
#include "CardImage.h"
#include "UIImage.h"
#include "TarotManager.h"
#include "UIContainer.h"

//#include "AniPlane.h"
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
//	SAFE_DELETE(mOverImage);
//	SAFE_DELETE(mCloseImage);
//	SAFE_DELETE(mOpenImage);
}
