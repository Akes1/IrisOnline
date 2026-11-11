#include "stdafx.h"
#include "LookAtCamera.h"

#include "Ray.h"
#include "MouseAgent.h"

const float MIN_CAMERA_DIST = 300.0f;
const float MAX_CAMERA_DIST = 1300.f;

cLookAtCamera::cLookAtCamera()
: mDesiredLookAtDistance( 1000.0f )
, mTargetDirection( NiPoint3::UNIT_Y )
, mDirection( NiPoint3::UNIT_Y )
, mYawAngle( 0.0f )
, mPitchAngle( 0.0f )
, mTargetPos( NiPoint3::ZERO )
, mPos( NiPoint3::ZERO )
{
	mFovyDegree = 75.0f;

	mMinCameraDist = MIN_CAMERA_DIST;
	mMaxCameraDist = MAX_CAMERA_DIST;
}

cLookAtCamera::~cLookAtCamera()
{
}

void cLookAtCamera::OnProcess( float deltaTime )
{
	OnProcessInput( deltaTime );
	/// 방향 보간
	mDirection = mTargetDirection;

	///
	mActualLookAtDistance = mDesiredLookAtDistance;

	NiPoint3 pos;
	mTargetPos = mLookAt - mDirection * mDesiredLookAtDistance;

	mTargetPos += mDirection * 50.0f;
	//
	mPos = mTargetPos;

	SetTranslate( mPos );
	Update( deltaTime );
}

void cLookAtCamera::OnProcessInput( float /*deltaTime*/ )
{
	/// 회전
	int dx = MOUSE->GetDeltaX();
	int dy = MOUSE->GetDeltaY();

	if( dx || dy )
	{
		if( MOUSE->IsRButtonDown() )
		{
			float rspeed = mRotateSpeed * 0.02f; //* deltaTime;

			if( dx )
			{
				mYawAngle += float(dx) * rspeed;
			}
			if( dy )
			{
				mPitchAngle += float(dy) * rspeed;
			}

			NiMatrix3 ry, rz;
			ry.MakeYRotation( mYawAngle * NI_PI / 180.0f );
			rz.MakeZRotation( mPitchAngle * NI_PI / 180.0f );
			mNiCamera->SetRotate( ry * rz );

			mOrientNode->Update( 0.0f );
			mTargetDirection = mNiCamera->GetWorldDirection();
			mTargetDirection.Unitize();
		}
	}

	/// 휠값으로 거리 조절
	mDesiredLookAtDistance -= MOUSE->GetDeltaWheel() * 0.05f; // 5.0f * deltaTime;

	if( mDesiredLookAtDistance <= mMinCameraDist )
	{
		mDesiredLookAtDistance = mMinCameraDist;
	}
	if( mDesiredLookAtDistance >= mMaxCameraDist )
	{
		mDesiredLookAtDistance = mMaxCameraDist;
	}
}

void cLookAtCamera::SetMinCameraDist( float dist )
{
	if( dist <= 0 )
		dist = MIN_CAMERA_DIST;

	mMinCameraDist = dist;
}

void cLookAtCamera::SetMaxCameraDist( float dist )
{
	if( dist <= 0 )
		dist = MAX_CAMERA_DIST;

	mMaxCameraDist = dist;
}
