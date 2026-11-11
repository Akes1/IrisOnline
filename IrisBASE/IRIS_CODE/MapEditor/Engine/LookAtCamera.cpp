#include "stdafx.h"
#include "LookAtCamera.h"

#include "Ray.h"
#include "MouseAgent.h"

const float MIN_LOOKAT_DIST = 300.0f;
const float MAX_LOOKAT_DIST = 1800.f;
const float LOOKAT_SPEED = 6.0f;
const float ZOOM_SPEED = 3.0f;
const float YAW_SPEED = 10.0f;
const float PITCH_SPEED = 10.0f;
const float MOUSE_SENSITIVITY = 0.02f;
const float MOUSE_WHEEL_SENSITIVITY = 0.1f;

cLookAtCamera::cLookAtCamera()
: mDesiredLookAtDistance( MAX_LOOKAT_DIST )
, mLookAtDistance( MAX_LOOKAT_DIST )
, mTargetLookAt( NiPoint3::ZERO )
, mLookAt( NiPoint3::ZERO )
, mTargetYawAngle( 0.0f )
, mTargetPitchAngle( 0.0f )
, mYawAngle( 0.0f )
, mPitchAngle( 0.0f )
, mNeedUpdate( true )
{
	mFovyDegree = 75.0f;
}

cLookAtCamera::~cLookAtCamera()
{
}

void cLookAtCamera::Process( unsigned long deltaTime )
{
	/// 사용자 입력을 처리
	ProcessInput( deltaTime );

	mNeedUpdate = false;

	/// 시점
	float dt = float(deltaTime) * 0.001f;
	if( mLookAt.x == 0.0f )
	{
		mNeedUpdate = true;
		mLookAt = mTargetLookAt;
	}
	else if( mLookAt != mTargetLookAt  )
	{
		mNeedUpdate = true;
		mLookAt += (mTargetLookAt - mLookAt) * min(dt * LOOKAT_SPEED, 1.0f);
	}

	/// 거리
	if( mLookAtDistance != mDesiredLookAtDistance )
	{
		mNeedUpdate = true;
		mLookAtDistance += (mDesiredLookAtDistance - mLookAtDistance) * min(dt * ZOOM_SPEED, 1.0f);
	}

	/// 회전
	if( mYawAngle != mTargetYawAngle )
	{
		mNeedUpdate = true;
		mYawAngle += (mTargetYawAngle - mYawAngle) * min(dt * YAW_SPEED, 1.0f);
	}
	if( mPitchAngle != mTargetPitchAngle )
	{
		mNeedUpdate = true;
		mPitchAngle += (mTargetPitchAngle - mPitchAngle) * min(dt * PITCH_SPEED, 1.0f);
	}

	///
	if( mNeedUpdate )
	{
		NiMatrix3 ry, rz;
		ry.MakeYRotation( mYawAngle * NI_PI / 180.0f );
		rz.MakeZRotation( mPitchAngle * NI_PI / 180.0f );
		mNiCamera->SetRotate( ry * rz );
		mOrientNode->Update( 0.0f );

		SetTranslate( mLookAt + mLookAtDistance * -mNiCamera->GetWorldDirection() );
	}

	///
	Update( 0.0f );

	///
	//mListenerNode->SetTranslate( mNiCamera->GetWorldTranslate() );
	//mListenerNode->Update( 0.0f, false );
}

void cLookAtCamera::ProcessInput( unsigned long /*deltaTime*/ )
{
	/// 회전
	int dx = MOUSE->GetDeltaX();
	int dy = MOUSE->GetDeltaY();

	if( dx || dy )
	{
		if( MOUSE->IsRButtonDown() )
		{
			float rspeed = mRotateSpeed * MOUSE_SENSITIVITY;

			if( dx )
			{
				mTargetYawAngle += float(dx) * rspeed;
			}
			if( dy )
			{
				mTargetPitchAngle += float(dy) * rspeed;

				if( mTargetPitchAngle > 80.0f )
					mTargetPitchAngle = 80.0f;
				else if( mTargetPitchAngle < -60.0f )
					mTargetPitchAngle = -60.0f;
			}
		}
	}

	/// 휠값으로 거리 조절
	mDesiredLookAtDistance -= MOUSE->GetDeltaWheel() * MOUSE_WHEEL_SENSITIVITY;

	if( mDesiredLookAtDistance <= MIN_LOOKAT_DIST )
	{
		mDesiredLookAtDistance = MIN_LOOKAT_DIST;
	}
	if( mDesiredLookAtDistance >= MAX_LOOKAT_DIST )
	{
		mDesiredLookAtDistance = MAX_LOOKAT_DIST;
	}
}
