#include "stdafx.h"
#include "SoundSceneNode.h"

#include "Ray.h"
#include "Sphere.h"

#include "SoundSystem.h"
#include "ResourceManager.h"
#include "DynamicSceneNode.h"
#include "SceneManager.h"

const float FADE_SPEED = 2.2f;

cSoundSceneNode::cSoundSceneNode( eType type )
: cSceneNode( type )
, mListenerEntered( false )
, mOldListenerEntered( false )
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
, mEffectSound( false )
{
	mActiveApp = SOUNDSYS->IsSoundActive();
}

cSoundSceneNode::~cSoundSceneNode()
{
	RemoveSelf();

	mSound = 0;
}

void cSoundSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mFadeOut )
	{
		RemoveSelf();
	}

	///
	if( mpOwnerSceneNode )
	{
		SetTranslate( mpOwnerSceneNode->GetWorldTranslate() );
	}

	///
	float dt = float(deltaTime) * 0.001f;
	cSceneNode::OnProcess( deltaTime, accumTime );

	/// 음량
	if( mListenerEntered == true )
	{
		if( mOldListenerEntered == false )
		{
			mTempCount = 0;
			mTempInterval = mLoopInterval;
		}

		float dist = (mBoundSphere.GetCenter() - mListenerSphere.GetCenter()).Length();

		if( dist <= mAttenDistance )
		{
			mTargetGain = mVolumeRatio;
		}
		else
		{
			mTargetGain = mVolumeRatio * (1.0f - (dist - mAttenDistance) / (mRadius + mListenerSphere.GetRadius() - mAttenDistance));
		}
	}
	else
	{
		mTargetGain = 0.0f;
	}

	if( mActiveApp == true )
	{
		if( mGain != mTargetGain )
		{
			mGain += (mTargetGain - mGain) * min(deltaTime * FADE_SPEED, 1.0f);
			mSound->SetGain( mGain );
		}
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
				/// 제거 시점
				RemoveSelf();
			}
		}
	}

	mOldListenerEntered = mListenerEntered;
	mListenerEntered = false;
}

bool cSoundSceneNode::Init( cSoundSceneNodeParam& param )
{
	NiAudioSource* src = 0;
	if( param.mManaged == false )
	{
		/// 엔진 노드를 생성
		NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
		src = as->CreateSource( NiAudioSource::TYPE_AMBIENT );
		mSound = src;

		src->SetFilename( param.mPathName.Cstr() );

		NiFilename filename( param.mPathName.Cstr() );
		if( _strcmpi( filename.GetExt(), ".mp3") == 0 )
		{
			src->SetStreamed( true );
			src->SetAllowSharing( false );
		}
		else
		{
			src->SetStreamed( false );
			src->SetAllowSharing( true );
		}

		/// 사운드를 로딩
		if( src->Load() == false )
		{
			assert( 0 );
			return false;
		}
	}
	else
	{
		/// 이펙트용 사운드는 ResourceManager에서 획득한다.
		src = RESOURCEMAN->CloneStageSoundByName( param.mPathName.Cstr() );
		if( src == 0 )
		{
			assert( 0 );
			return false;
		}
		mSound = NiDynamicCast(NiAudioSource, src);
	}

	if( mSound == 0 )
		return false;

	mpOwnerSceneNode = param.mpOwnerSceneNode;
	//	if( mpOwnerSceneNode )
	//		mOwnerSceneNodeIdx = mpOwnerSceneNode->GetIndexByManger();

	/// 멤버 초기화
	mRadius = param.mRadius;
	mAttenDistance = param.mAttenDistance;
	mLoopCount = param.mLoopCount;
	mLoopInterval = param.mLoopInterval;
	mVolumeRatio = param.mVolumeRatio;
	mEffectSound = param.mEffectSound;

	/// 상위 초기화
	GetFileName( &param.mPathName, param.mPathName );

	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// 속성을 설정
	src->SetLoopCount( 1 );

	//if( param.mManaged )
	{
		src->SetGain( 0.0f );
		src->Stop();
	}
	//else
	//{
	//	if( SOUNDSYS->IsSoundActive() )
	//	{
	//		src->SetGain( mVolumeRatio );
	//	}
	//	else
	//	{
	//		mGain = mVolumeRatio;
	//		src->SetGain( 0.0f );
	//	}
	//}
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

	//	if( mOwnerSceneNodeIdx != (unsigned long)-1 )
	//	{
	//		cDynamicSceneNode* node = SCENEMAN->GetDynamicSceneNode( mOwnerSceneNodeIdx );
	//		if( node )
	//		{
	//			assert( node == mpOwnerSceneNode );
	//			node->UnLinkSound( this );
	//		}
	//	}
}

void cSoundSceneNode::Remove()
{
	if( mRemoved == true )
		return;

	//	if( mOwnerSceneNodeIdx != (unsigned long)-1 )
	//	{
	//		cDynamicSceneNode* node = SCENEMAN->GetDynamicSceneNode( mOwnerSceneNodeIdx );
	//		if( node )
	//		{
	//			assert( node == mpOwnerSceneNode );
	//			node->UnLinkSound( this );
	//		}
	//	}

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

void cSoundSceneNode::UpdatePickInfo( void* extra )
{
	if( IsRemoved() == true )
		return;

	mListenerEntered = true;
	mListenerSphere = *((cSphere*)extra);
}
