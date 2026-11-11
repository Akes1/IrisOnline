#include "stdafx.h"

#ifdef MAP_EDITOR
#include "AreaGroup.h"
#include "AreaSceneNode.h"

cAreaGroup* cAreaGroup::mCurrentGroup = 0;

cAreaGroup::cAreaGroup( NiAudioSource* snd, NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th )
: mImage( tex, x, y, tx, ty, tw, th )
, mSound( snd )
{
	assert( tex );

	mHeroIn = false;
	mTempTime = 0.0f;
	mTargetAlpha = 0.0f;
	mAlpha = 0.0f;
	mTargetGain = 0.0f;
	mGain = 0.0f;

	mImage.SetAlpha( 0.0f );

	if( mSound )
	{
		mSound->SetLoopCount( 1 );
		mSound->SetGain( 0.0f );
		mSound->Stop();
	}
}

void cAreaGroup::Process( unsigned long deltaTime, const cSphere& heroSphere )
{
	bool oldHeroIn = mHeroIn;
	mHeroIn = false;

	cAreaList::cIterator i = mAreaList.Begin();
	cAreaList::cIterator iend = mAreaList.End();

	for( ; i != iend; ++i )
	{
		cAreaSceneNode* n = *i;
		
		if( n->GetBoundSphere().ContainSphere( heroSphere ) )
		{
			mHeroIn = true;
			break;
		}
	}

	///
	if( mTargetAlpha > 0.0f )
	{
		mTempTime += deltaTime;

		if( mTempTime > 5.0f )
		{
			mTargetAlpha = 0.0f;
		}
	}
	if( oldHeroIn )
	{
		//if( mHeroIn == false )
		//{
		//	//mTargetAlpha = 0.0f;
		//	//mTargetGain = 0.0f;
		//	mTempTime = 0.0f;
		//}
		//else
		//{
		//	if( mTargetAlpha > 0.0f )
		//	{
		//		mTempTime += deltaTime;

		//		if( mTempTime > 5.0f )
		//		{
		//			mTargetAlpha = 0.0f;
		//		}
		//	}
		//}
	}
	else
	{
		if( mHeroIn && mCurrentGroup != this )
		{
			mTargetAlpha = 1.0f;
			mTargetGain = 1.0f;
			mTempTime = 0.0f;

			if( mSound )
			{
				mSound->Play();
			}

			if( mCurrentGroup )
			{
				mCurrentGroup->FadeOut();
			}
			mCurrentGroup = this;
		}
	}

	/// 이미지 알파 처리
	if( mAlpha != mTargetAlpha )
	{
		float da = deltaTime * 1.2f;

		if( mAlpha < mTargetAlpha )
		{
			/// 페이드 인
			mAlpha += da;

			if( mAlpha > mTargetAlpha )
				mAlpha = mTargetAlpha;
		}
		else
		{
			/// 페이드 아웃
			mAlpha -= da;

			if( mAlpha < mTargetAlpha )
				mAlpha = mTargetAlpha;
		}
		mImage.SetAlpha( mAlpha );
	}

	/// 사운드 음량 처리
	if( mGain != mTargetGain )
	{
		float dg = deltaTime * 0.5f;

		if( mGain < mTargetGain )
		{
			/// 페이드 인
			mGain += dg;

			if( mGain > mTargetGain )
				mGain = mTargetGain;
		}
		else
		{
			/// 페이드 아웃
			mGain -= dg;

			if( mGain < mTargetGain )
				mGain = mTargetGain;
		}

		if( mSound )
		{
			mSound->SetGain( mGain );
			mSound->Update( 0.0f );

			if( mTargetGain == 0.0f && mGain == 0.0f )
				mSound->Stop();
		}
	}
}

void cAreaGroup::OnRender()
{
	if( mAlpha > 0.0f )
	{
		mImage.Draw();
	}
}

void cAreaGroup::AddArea( cAreaSceneNode* area )
{
	assert( area );

	mAreaList.PushBack( area );
}

void cAreaGroup::FadeOut()
{
	mTargetAlpha = 0.0f;
	mTargetGain = 0.0f;
}
#endif /// MAP_EDITOR
