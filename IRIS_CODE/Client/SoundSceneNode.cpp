#include "stdafx.h"
#include "SoundSceneNode.h"

#include "Ray.h"

#include "SoundSystem.h"
#include "ResourceManager.h"
#include "DynamicSceneNode.h"
#include "SceneManager.h"
#include "OptionManager.h"

#include "ObjectManager.h"
#include "hero.h"

const float FADE_SPEED = 2.2f;

cSoundSceneNode::cSoundSceneNode( eType type )
: cSceneNode( type )
, mRadius( 1000.0f )
, mAttenDistance( 100.0f )
, mLoopCount( 0 )
, mTempCount( 0 )
, mLoopInterval( 0.0f )
, mTempInterval( 0.0f )
, mVolumeRatio( 1.0f )
, mGain( 0.0f )
, mTargetGain( 0.0f )
, mRemoved(false)
, mFadeOut(false)
, mpOwnerSceneNode(0)
, mSound(0)
{
	mActiveApp = SOUNDSYS->IsSoundActive();

	mListenerStatus = eLISTENER_OUT;
}

cSoundSceneNode::~cSoundSceneNode()
{
	if( mSound != 0 )
	{
		RemoveSelf();
		mSound = 0;
	}
}

void cSoundSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mFadeOut )
		RemoveSelf();

	///
	if( mpOwnerSceneNode )
		SetTranslate( mpOwnerSceneNode->GetWorldTranslate() );

	switch( mSoundType )
	{
	case eSOUNDTYPE_EFFECT:
	case eSOUNDTYPE_NOFADE_EFFECT:
		{
			///
			float dt = float(deltaTime) * 0.001f;
			if( mActiveApp == true )
			{
				if( mTargetGain != mVolumeRatio )
					mTargetGain = mVolumeRatio;

				if( mGain != mTargetGain )
				{
					if( mSoundType == eSOUNDTYPE_NOFADE_EFFECT )
						mGain = mTargetGain;
					else
						mGain += (mTargetGain - mGain) * min(dt * 5.5f, 1.0f);
				}

				mSound->SetGain( mGain );
			}
			else
			{
				mSound->SetGain( 0.0f );
			}

			/// 루프 카운트
			NiAudioSource::Status status = mSound->GetStatus();
			if( status != NiAudioSource::PLAYING )
			{
				if( mLoopCount == 0 )
				{
					mTempInterval += dt;

					if( mTempInterval >= mLoopInterval )
					{
						mTempInterval = 0.0f;
						mSound->Stop();
						mSound->Play();
					}
				}
				else
				{
					if( mTempCount < mLoopCount )
					{
						mTempInterval += dt;

						if( mTempInterval >= mLoopInterval )
						{
							mTempInterval = 0.0f;
							mTempCount += 1;
							mSound->Stop();
							mSound->Play();
						}
					}
					else
					{
						if( mGain == 0.0f )
							RemoveSelf();
					}
				}
			}
		}
		break;
	case eSOUNDTYPE_AMBIENT_ENV:
		{
			NiAudioSource::Status status = mSound->GetStatus();

			/// in/out
			if( mListenerStatus == eLISTENER_PLAY_IN )
			{
				mListenerStatus = eLISTENER_OUT;

				mTargetGain = mVolumeRatio;
				if( status != NiAudioSource::PLAYING )
					mSound->Play();
			}
			else if( mListenerStatus == eLISTERNER_NOPLAY_IN )
			{
				mListenerStatus = eLISTENER_OUT;

				if( status == NiAudioSource::PLAYING )
					mTargetGain = 0.0f;
				else
					mTargetGain = mVolumeRatio;

				if( mGain == 0.0f )
				{
					if( status == NiAudioSource::PLAYING )
						mSound->Stop();
					return;
				}
			}
			else
			{
				if( mTargetGain != 0.0f )
					mTargetGain = 0.0f;

				if( mGain == 0.0f )
				{
					if( status == NiAudioSource::PLAYING )
						mSound->Stop();
					return;
				}
			}

			float dt = float(deltaTime) * 0.001f;
			if( mActiveApp == true )
			{
				if( mGain != mTargetGain )
					mGain += (mTargetGain - mGain) * min(dt * FADE_SPEED, 1.0f);

				mSound->SetGain( mGain );
			}
			else
			{
				mSound->SetGain( 0.0f );
			}
		}
		break;
	}
}

bool cSoundSceneNode::Init( cSoundSceneNodeParam& param )
{
	NiAudioSource* src = 0;
	mSoundType = param.mSoundType;

	mFileName = param.mPathName.Cstr();

	switch( mSoundType )
	{
	case eSOUNDTYPE_EFFECT:
	case eSOUNDTYPE_NOFADE_EFFECT:
		{
			/// 이펙트용 사운드는 ResourceManager에서 획득한다.
			src = RESOURCEMAN->CloneStageSoundByName( param.mPathName.Cstr(), NiAudioSource::TYPE_3D );
			if( src == 0 )
			{
				assert( 0 );
				return false;
			}
			mSound = NiDynamicCast(NiAudioSource, src);

			src->SetLoopCount( 1 );
		}
		break;
	case eSOUNDTYPE_AMBIENT_ENV:
		{
			/// 엔진 노드를 생성
			src = RESOURCEMAN->CloneStageSoundByName( param.mPathName.Cstr() );
			if( src == 0 )
			{
				assert( 0 );
				return false;
			}
			mSound = NiDynamicCast(NiAudioSource, src);

			mListenerStatus = eLISTENER_OUT;

			src->SetLoopCount( 0 );
		}
		break;
	default: 
		return false;
	}

	if( mSound == 0 )
		return false;

	mpOwnerSceneNode = param.mpOwnerSceneNode;

	/// 멤버 초기화
	mRadius = param.mRadius;
	mAttenDistance = param.mAttenDistance;
	mLoopCount = param.mLoopCount;
	mLoopInterval = param.mLoopInterval;
	mVolumeRatio = param.mVolumeRatio;

	/// 상위 초기화
	GetFileName( &param.mPathName, param.mPathName );

	src->SetDirectionVector( -NiPoint3::UNIT_Y );
	src->SetUpVector( NiPoint3::UNIT_Z );
	src->SetMinMaxDistance( 300.0f, 80000.0f );

	src->SetGain( 1.0f );
	src->Stop();

	if( cSceneNode::Init( param ) == false )
		return false;

	return true;
}

bool cSoundSceneNode::Pick( const cRay& ray )
{
	return mBoundSphere.IntersectRay( ray );
}

void cSoundSceneNode::ActiveSound( bool active )
{
	if( mSound == 0 )
		return;

	mActiveApp = active;
	if( active )
	{
		mSound->SetGain( mGain );
	}
	else
	{
		mSound->SetGain( 0.0f );
	}
}

void cSoundSceneNode::RemoveSelf()
{
	if( mRemoved == true )
		return;

	SCENEMAN->AddDeleteSoundNode( this );
	mRemoved = true;

	if( mpOwnerSceneNode )
	{
		mpOwnerSceneNode->UnLinkSound( this );
		mpOwnerSceneNode = 0;
	}
}

void cSoundSceneNode::Remove()
{
	if( mRemoved == true )
		return;

	if( mpOwnerSceneNode )
	{
		mpOwnerSceneNode->UnLinkSound( this );
		mpOwnerSceneNode = 0;
	}

	SCENEMAN->AddDeleteSoundNode( this );
	mRemoved = true;
}

void cSoundSceneNode::ParentNodeRemove()
{
	if( mRemoved == true )
		return;

	mpOwnerSceneNode = 0;
	SCENEMAN->AddDeleteSoundNode( this );
	mRemoved = true;
}

void cSoundSceneNode::UpdatVolumeRatio()
{
	if( mSoundType == eSOUNDTYPE_EFFECT )
		mVolumeRatio = OPTIONMAN->GetEffVolume();
	else
		mVolumeRatio = OPTIONMAN->GetEnvVolume();
}
/*
void cSoundSceneNode::UpdatePickInfo( void* extra )
{
	if( IsRemoved() == true )
		return;
}
*/
