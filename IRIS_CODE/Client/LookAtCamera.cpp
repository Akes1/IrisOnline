#include "stdafx.h"
#include "LookAtCamera.h"

#include "Application.h"
#include "Ray.h"
#include "InputSystem.h"
#include "ResourceManager.h"
#include "NifAnimationInfo.h"

#include "ObjectManager.h"
#include "Hero.h"

const float MIN_LOOKAT_DIST = 300.0f;
const float MAX_LOOKAT_DIST = 1800.0f;
const float DEF_LOOKAT_DIST = 1000.0f;
const float MOVE_SPEED = 3.6f;
const float ZOOM_SPEED = 1.8f;
const float YAW_SPEED = 7.0f;
const float PITCH_SPEED = 7.0f;
const float MOUSE_SENSITIVITY = 0.3f;
const float MOUSE_WHEEL_SENSITIVITY = 1.5f;

cLookAtCamera::cLookAtCamera()
: mTargetYawAngle( 0.0f )
, mYawAngle( 0.0f )
, mTargetPitchAngle( 25.0f )
, mPitchAngle( 25.0f )
{
	mFixed = false;

	mDesiredLookAtDistance = DEF_LOOKAT_DIST;
	mLookAtDistance = DEF_LOOKAT_DIST;
	mTargetLookAt = NiPoint3::ZERO;
	mLookAt = NiPoint3::ZERO;
	
	mNifAniInfo = new cNifAnimationInfo;
}

cLookAtCamera::~cLookAtCamera()
{
	SAFE_DELETE(mNifAniInfo);
}

bool cLookAtCamera::Init( float width, float height )
{
	mNiCamera = NiNew NiCamera;

	SetViewPort( 0.f, 1.f, 1.f, 0.f );
	SetViewFrustum( width, height );
	//mpNiCamera->SetMinNearPlaneDist( FLT_EPSILON );
	//mpNiCamera->SetMaxFarNearRatio( 50000.f );

	//// The orient node is used to make Z up
	NiMatrix3 rotX;
	NiMatrix3 rotZ;
	rotX.MakeXRotation( -NI_HALF_PI );
	rotZ.MakeZRotation( -NI_HALF_PI );

	if( RESOURCEMAN->LoadNIF( "./Data/Effect/Camera_Shake01.nif" ) == false )
	{
		assert( 0 );
		return false;
	}
	if( (mOrientNode = RESOURCEMAN->CloneObjectByName( "./Data/Effect/Camera_Shake01.nif" )) == 0 )
	{
		assert( 0 );
		return false;
	}

	mOrientNode->AttachChild( mNiCamera );

	bool selectiveUpdate = true;
	bool rigid = false;
	mOrientNode->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	mOrientNode->SetTranslate( NiPoint3::ZERO );
	mOrientNode->SetRotate( rotZ * rotX );
	mOrientNode->Update( 0.f );
	mOrientNode->UpdateProperties();
	mOrientNode->UpdateEffects();
	mOrientNode->UpdateNodeBound();

	mNifAniInfo->CollectData( mOrientNode );

	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetCycleType( NiTimeController::LOOP );
		mNifAniInfo->SetLooping( false );

		mNifAniInfo->SetTarget( mOrientNode );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

void cLookAtCamera::Reset()
{
	mTargetLookAt = NiPoint3::ZERO;
	mLookAt = NiPoint3::ZERO;
/*
	mDesiredLookAtDistance = DEF_LOOKAT_DIST;
	mLookAtDistance = DEF_LOOKAT_DIST;
	mTargetYawAngle = 0.0f;
	mYawAngle = 0.0f;
	mTargetPitchAngle = 25.0f;
	mPitchAngle = 25.0f;
*/
	mNeedUpdate = true;

}

void cLookAtCamera::ViewHeroFront( NiPoint3 heroDir )
{
	NiPoint3 viewDir = mNiCamera->GetWorldDirection();

	float fRadian = ::acosf(heroDir.Dot(viewDir) / (heroDir.Length() * viewDir.Length()));

	NiPoint3 heroDir2 = heroDir;
	if(D3DX_PI - fRadian < 0.0005f)
		heroDir2 += NiPoint3(0.0005f, 0.0005f, 0.0005f);

	heroDir2.UnitCross( viewDir );

	NiMatrix3 mat;
	mat.MakeRotation( fRadian, heroDir2 );

	float xang = 0.0f, yang = 0.0f, zang = 0.0f;
	mat.ToEulerAnglesXYZ( xang, yang, zang );

	mYawAngle = mTargetYawAngle = zang * 180.0f / NI_PI;

	mNeedUpdate = true;
}


void cLookAtCamera::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	float dt = float(deltaTime) * 0.001f;
	mNeedUpdate = false;

	/// 시점
	if( mLookAt.x == 0.0f )
	{
		mNeedUpdate = true;
		mLookAt = mTargetLookAt;
	}
	else if( mLookAt != mTargetLookAt  )
	{
		mNeedUpdate = true;

		if( mFixed )
			mLookAt = mTargetLookAt;
		else
			mLookAt += (mTargetLookAt - mLookAt) * min(dt * MOVE_SPEED, 1.0f);
	}

	/// 거리
	if( mLookAtDistance != mDesiredLookAtDistance )
	{
		mNeedUpdate = true;

		if( mFixed )
			mLookAtDistance = mDesiredLookAtDistance;
		else
			mLookAtDistance += (mDesiredLookAtDistance - mLookAtDistance) * min(dt * ZOOM_SPEED, 1.0f);
	}

	/// 회전
	bool checkMoveMsg = false;
	if( mYawAngle != mTargetYawAngle )
	{
		mNeedUpdate = true;

		if( mFixed )
			mYawAngle = mTargetYawAngle;
		else
			mYawAngle += (mTargetYawAngle - mYawAngle) * min(dt * YAW_SPEED, 1.0f);

		checkMoveMsg = true;
	}

	if( mPitchAngle != mTargetPitchAngle )
	{
		mNeedUpdate = true;

		if( mFixed )
			mPitchAngle = mTargetPitchAngle;
		else
			mPitchAngle += (mTargetPitchAngle - mPitchAngle) * min(dt * PITCH_SPEED, 1.0f);
	}

	///
	if( mNeedUpdate )
	{
		NiMatrix3 ry, rz;
		ry.MakeYRotation( mYawAngle * NI_PI / 180.0f );
		rz.MakeZRotation( mPitchAngle * NI_PI / 180.0f );
		mNiCamera->SetRotate( ry * rz );
//		Update( accumTime );
		mOrientNode->Update( 0.0f );

		SetTranslate( mLookAt + mLookAtDistance * - mNiCamera->GetWorldDirection() );
	}

	///
	if( mNifAniInfo )
	{
		if( mNifAniInfo->Update( deltaTime, accumTime ) == cNifAnimationInfo::eEvent_End )
		{
			mNifAniInfo->Stop();
		}
	}

	///
	Update( accumTime );

	if( checkMoveMsg == true && HERO )
		HERO->CheckSendMoveMsg( deltaTime, mYawAngle == mTargetYawAngle );

	if( mFixed == true )
		mFixed = false;
}

void cLookAtCamera::OnProcessMouse( unsigned long time )
{
	/// 회전
	int dx = MOUSE->GetDeltaX();
	int dy = MOUSE->GetDeltaY();

	if( dx || dy )
	{
		if( MOUSE->RButtonPressed() )
		{
			if( dx )
			{
				mTargetYawAngle += float(dx) * MOUSE_SENSITIVITY;
			}
			if( dy )
			{
				mTargetPitchAngle += float(dy) * MOUSE_SENSITIVITY;

				if( mTargetPitchAngle > 80.0f )
					mTargetPitchAngle = 80.0f;
				else if( mTargetPitchAngle < -60.0f )
					mTargetPitchAngle = -60.0f;
			}
		}
	}

	/// 휠값으로 거리 조절
	mDesiredLookAtDistance -= MOUSE->GetWheel() * MOUSE_WHEEL_SENSITIVITY;

	if( mDesiredLookAtDistance <= MIN_LOOKAT_DIST )
	{
		mDesiredLookAtDistance = MIN_LOOKAT_DIST;
	}
	if( mDesiredLookAtDistance >= MAX_LOOKAT_DIST )
	{
		mDesiredLookAtDistance = MAX_LOOKAT_DIST;
	}
}

void cLookAtCamera::Shake()
{
	if( mNifAniInfo )
		mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
}
