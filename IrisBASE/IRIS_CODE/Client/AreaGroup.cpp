#include "stdafx.h"
#include "AreaGroup.h"

#include "AreaSceneNode.h"

#include "OptionManager.h"
#include "ObjectManager.h"
#include "hero.h"

#include "SoundSystem.h"
#include "RenderSystem.h"
#include "Application.h"

cAreaGroup* cAreaGroup::mCurrentGroup = 0;

cAreaGroup::cAreaGroup( NiAudioSource* snd, NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th, unsigned int nameIdx )
: mImage( tex, x, y, tx, ty, tw, th )
{
	assert( tex );

	mHeroIn = false;
	mTempTime = 0;
	mTargetAlpha = 0.0f;
	mAlpha = 0.0f;

	mImage.SetAlpha( 0.0f );
	mImage.SetScreenRect( cUIRect( x, y, x + tw, y + th));
	mImageWidth = tw;

	mSound = snd;

	mNameIndex = nameIdx;
}

cAreaGroup::~cAreaGroup()
{
}

void cAreaGroup::OnProcess( unsigned long deltaTime, unsigned long accumTime, const cSphere& heroSphere )
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

		if( mTempTime > 5000 )
		{
			mTargetAlpha = 0.0f;
		}
	}

	if( oldHeroIn == false )
	{
		if( mHeroIn && mCurrentGroup != this )
		{
			mTargetAlpha = 1.0f;
			mTempTime = 0;

			/// Area sound fadeIn;
			if( mSound )
				SOUNDSYS->PlayAreaSound( mSound );

			/// change current group
			if( mCurrentGroup )
				mCurrentGroup->FadeOut();
			mCurrentGroup = this;

			HERO->SetAreaNameIdx( mNameIndex );
		}
	}
	else
	{
		/// 겹친지역에 대한 처리부
		if( mHeroIn && mCurrentGroup != this )
		{
			if( mCurrentGroup->IsHeroIn() == false )
			{
				mTargetAlpha = 1.0f;
				mTempTime = 0;

				/// Area Sound fadeIn;
				if( mSound )
					SOUNDSYS->PlayAreaSound( mSound );

				/// change current group
				if( mCurrentGroup )
					mCurrentGroup->FadeOut();
				mCurrentGroup = this;

				HERO->SetAreaNameIdx( mNameIndex );
			}
		}
	}

	/// 이미지 알파 처리
	if( mAlpha != mTargetAlpha )
	{
		float da = deltaTime * 0.0012f;

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
//		mImage.SetAlpha( mAlpha );
	}

	if( mSound )
		mSound->Update( accumTime * 0.001f );
}

void cAreaGroup::OnRender()
{
	if( mAlpha > 0.0f )
	{
		unsigned int x = (RENDERSYS->GetScreenWidth() - mImageWidth ) / 2;
		unsigned int y = 60;

		mImage.SetAlpha( mAlpha );
		mImage.SetPos( x, y );
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

	if( mSound )
		mSound->FadeToGain( 0.0f, THEAPP->GetWorldAccumTime()*0.001f, 3.0f );
}

void cAreaGroup::SetActive( bool active )
{
	if( mSound == 0 )
		return;

	if( active == false )
		mSound->SetGain( 0.0f );
}
