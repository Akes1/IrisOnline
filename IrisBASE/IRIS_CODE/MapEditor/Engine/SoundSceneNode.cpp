#include "stdafx.h"

#ifdef MAP_EDITOR
#include "SoundSceneNode.h"
#include "Ray.h"
#include "Sphere.h"

const float FADE_SPEED = 2.2f;

cSoundSceneNode::cSoundSceneNode( eSceneNodeType type )
: cSceneNode( type )
, mListenerStatus( LISTENER_OUT )
, mOldListenerStatus( LISTENER_OUT )
, mRadius( 1000.0f )
, mAttenDistance( 100.0f )
, mLoopCount( 0 )
, mTempCount( 0 )
, mLoopInterval( 0.0f )
, mTempInterval( 0.0f )
, mVolumeRatio( 1.0f )
, mGain( 0.0f )
, mTargetGain( 0.0f )
{
}

cSoundSceneNode::~cSoundSceneNode()
{
}

void cSoundSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	cSceneNode::Process( deltaTime, accumTime );

	/// 음량
	if( mListenerStatus == LISTENER_IN )
	{
		if( mOldListenerStatus == LISTENER_OUT )
		{
			mTempCount = 0;
			mTempInterval = mLoopInterval;
		}

		float dist = (mBoundSphere.GetCenter() - mListenerSphere.GetCenter()).Length();

		if( dist <= mAttenDistance )
			mTargetGain = mVolumeRatio;
		else
			mTargetGain = mVolumeRatio * (1.0f - (dist - mAttenDistance) / (mRadius + mListenerSphere.GetRadius() - mAttenDistance));
	}
	else
	{
		mTargetGain = 0.0f;
	}

	if( mGain != mTargetGain )
	{
		mGain += (mTargetGain - mGain) * min(deltaTime * FADE_SPEED, 1.0f);
		mSound->SetGain( mGain );
	}

	/// 루프 카운트
	NiAudioSource::Status status = mSound->GetStatus();

	if( status != NiAudioSource::PLAYING )
	{
		if( mLoopCount == 0 )
		{
			mTempInterval += deltaTime;

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
				mTempInterval += deltaTime;

				if( mTempInterval >= mLoopInterval )
				{
					mTempInterval = 0.0f;
					mTempCount += 1;
					mSound->Stop();
					mSound->Play();
				}
			}
		}
	}

	mOldListenerStatus = mListenerStatus;
	mListenerStatus = LISTENER_OUT;
}

void cSoundSceneNode::SetListenerStatus( eListenerStatus status, const cSphere& sphere )
{
	mListenerStatus = status;
	mListenerSphere = sphere;
}

bool cSoundSceneNode::Save( cFileSaver& saver )
{
	/// 상위 정보(이름, 변환 행렬)를 저장
	if( cSceneNode::Save( saver ) == false )
		return false;

	/// 반지름을 저장
	saver.WriteFloat( mRadius );

	/// 감쇠 거리를 저장
	saver.WriteFloat( mAttenDistance );

	/// 반복 횟수를 저장
	saver.WriteUnsignedInt( mLoopCount );

	/// 재생 간격을 저장
	saver.WriteFloat( mLoopInterval );

	/// 음량 비율을 저장
	saver.WriteFloat( mVolumeRatio );
	return true;
}

bool cSoundSceneNode::Init( const cSoundSceneNodeParam& param )
{
	/// 엔진 노드를 생성
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	NiAudioSource* src = as->CreateSource( NiAudioSource::TYPE_AMBIENT );
	mSound = src;

	src->SetStreamed( false );
	src->SetFilename( param.mPathName.Cstr() );
	src->SetAllowSharing( true );

	/// 사운드를 로딩
	if( src->Load() == false )
	{
		assert( 0 && "failed to load sound" );
		return false;
	}

	/// 멤버 초기화
	mRadius = param.mOuterRadius;
	mAttenDistance = param.mAttenDistance;
	mLoopCount = param.mLoopCount;
	mLoopInterval = param.mLoopInterval;
	mVolumeRatio = param.mVolumeRatio;

	/// 상위 초기화
	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// 속성을 설정
	src->SetLoopCount( 1 );
	src->SetGain( 0.0f );
	src->Stop();
	return true;
}

bool cSoundSceneNode::Pick( const cRay& ray )
{
	return mBoundSphere.IntersectRay( &mPickPos, &mPickDistance, ray );
}
#endif /// MAP_EDITOR
