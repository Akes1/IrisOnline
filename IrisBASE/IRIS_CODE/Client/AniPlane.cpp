#include "StdAfx.h"
#include "aniplane.h"

#include "Application.h"

cBaseAniPlane::cBaseAniPlane()
: mFrames(0)
, mCurFrame(0)
, mLoop(false)
, mBeginAnim(false)
, mTimeOut(0)
, mLastAccumTime(0)
, mTexX(0)
, mTexY(0)
, mTexW(0)
, mTexH(0)
{
}

cBaseAniPlane::~cBaseAniPlane()
{
}

bool cBaseAniPlane::Create( NiTexture* pTexture, int screenX, int screenY, unsigned int screenW, unsigned int screenH,
						    unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom )
{
	bool check = cPlaneObject::Create( pTexture, screenX, screenY, screenW, screenH, texLeft, texTop, texRight, texBottom );
	if( check == false )
		return false;

	mTexX = texLeft;
	mTexY = texTop;

	mTexW = texRight - texLeft;
	mTexH = texBottom - texTop;

	if( mTexW == 0 )
		return false;
	return true;
}


void cBaseAniPlane::Process( unsigned long accumTime )
{
	if( mBeginAnim == false )
		return;

	if( accumTime - mLastAccumTime >= mTimeOut )
	{
		/// frame change
		if( mCurFrame >= mFrames )
		{
			if( mLoop )
				mCurFrame = 0;
			else
				EndAnimation();
		}
		mLastAccumTime = accumTime;

		unsigned int tx = mTexX + ( mCurFrame * mTexW );
		unsigned int ty = mTexY;

		SetTextureRect( tx, ty, tx + mTexW, ty + mTexH );

		mCurFrame++;
	}
}

void cBaseAniPlane::SetAnimationInfo( short totalFrame, unsigned long timeOut, bool loop )
{
	mFrames = totalFrame;
	mTimeOut = timeOut;
	mLoop = loop;

	mCurFrame = 0;
}

void cBaseAniPlane::BeginAnimation()
{
	mBeginAnim = true;
	mCurFrame = 0;
	mLastAccumTime = THEAPP->GetWorldAccumTime();
}

void cBaseAniPlane::EndAnimation()
{
	mBeginAnim = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////
cEffetAniPlane::cEffetAniPlane()
: mColumCount(0)
{
}

cEffetAniPlane::~cEffetAniPlane()
{
}

bool cEffetAniPlane::Create( NiTexture* pTexture, int screenX, int screenY, unsigned int screenW, unsigned int screenH,
							unsigned int texLeft, unsigned int texTop, unsigned int texRight, unsigned int texBottom )
{
	bool check = cPlaneObject::Create( pTexture, screenX, screenY, screenW, screenH, texLeft, texTop, texRight, texBottom );
	if( check == false )
		return false;

	mTexX = texLeft;
	mTexY = texTop;

	mTexW = texRight - texLeft;
	mTexH = texBottom - texTop;

	if( mTexW == 0 )
		return false;

	mColumCount = mTextureWidth / mTexW;
	if( mColumCount == 0 )
		return false;

	mpAlphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_ONE );
	mpAlphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_ONE );
	return true;
}


void cEffetAniPlane::Process( unsigned long accumTime )
{
	if( mBeginAnim == false )
		return;

	if( accumTime - mLastAccumTime >= mTimeOut )
	{
		/// frame change
		if( mCurFrame >= mFrames )
		{
			if( mLoop )
				mCurFrame = 0;
			else
				EndAnimation();
		}
		mLastAccumTime = accumTime;

		unsigned int row = mCurFrame % mColumCount;
		unsigned int col = mCurFrame / mColumCount;

		unsigned int tx = mTexX + ( row * mTexW );
		unsigned int ty = mTexY + ( col * mTexH );

		SetTextureRect( tx, ty, tx + mTexW, ty + mTexH );

		mCurFrame++;
	}
}

void cEffetAniPlane::Draw()
{
	if( mBeginAnim == false )
		return;

	cPlaneObject::Draw();
}
