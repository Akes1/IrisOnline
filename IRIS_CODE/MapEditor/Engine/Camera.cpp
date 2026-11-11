#include "stdafx.h"
#include "Camera.h"

#include "SoundSystem.h"

cCamera::cCamera()
{
	mScreenWidth = 800;
	mScreenHeight = 600;
	mFovyDegree = 75.0f;
	mNearDistance = 20.0f;
	mFarDistance = 50000.0f;
	mAspectRatio = 1.0f;
	mLeft = mRight = mTop = mBottom = 0.0f;
	mOrtho = false;
	mTranslateSpeed = 7000.0f;
	mRotateSpeed = 15.0f;

	mNiCamera = NiNew NiCamera;
}

cCamera::~cCamera()
{
}

void cCamera::Process( unsigned long deltaTime )
{
	ProcessInput( deltaTime );
	Update( (float)deltaTime * 0.001f );

	///
	mListenerNode->SetTranslate( mNiCamera->GetWorldTranslate() );
	mListenerNode->Update( 0.0f, false );
}

bool cCamera::Init( unsigned int screenWidth, unsigned int screenHeight )
{
	SetViewPort( 0.0f, 1.0f, 1.0f, 0.0f );
	SetViewFrustum( screenWidth, screenHeight );
	//mpNiCamera->SetMinNearPlaneDist( FLT_EPSILON );
	//mpNiCamera->SetMaxFarNearRatio( 50000.0f );

	//// The orient node is used to make Z up
	NiMatrix3 rotX;
	NiMatrix3 rotZ;
	rotX.MakeXRotation( -NI_HALF_PI );
	rotZ.MakeZRotation( -NI_HALF_PI );

	mOrientNode = NiNew NiNode;
	mOrientNode->AttachChild( mNiCamera, true );
	mOrientNode->SetSelectiveUpdate( true );
	mOrientNode->SetSelectiveUpdateTransforms( true );
	mOrientNode->SetSelectiveUpdatePropertyControllers( true );
	mOrientNode->SetSelectiveUpdateRigid( false );

	mOrientNode->SetTranslate( NiPoint3::ZERO );
	mOrientNode->SetRotate( rotZ * rotX );
	mOrientNode->Update( 0.0f );

	mListenerNode = NiNew NiNode;
	return true;
}

void cCamera::Translate( const NiPoint3& move )
{
	NiPoint3 trans = mNiCamera->GetTranslate();
	trans += mNiCamera->GetRotate() * NiPoint3(move.y, move.z, move.x);
	mNiCamera->SetTranslate( trans );
}

void cCamera::SetTranslate( float x, float y, float z )
{
	mNiCamera->SetTranslate( y, z, x );
}

void cCamera::SetTranslate( const NiPoint3& trans )
{
	mNiCamera->SetTranslate( trans.y, trans.z, trans.x );
}

void cCamera::Rotate( const NiPoint3& axis, float angle )
{
	NiMatrix3 r;
	r.MakeRotation( angle, axis.y, axis.z, axis.x );
	mNiCamera->SetRotate( r * mNiCamera->GetRotate() );
}

void cCamera::SetRotate( const NiMatrix3& r )
{
	mNiCamera->SetRotate( r );
}

void cCamera::Pitch( float angle )
{
	NiMatrix3 r;
	r.MakeRotation( angle, mNiCamera->GetRotate() * NiPoint3::UNIT_Z );
	mNiCamera->SetRotate( r * mNiCamera->GetRotate() );
}

void cCamera::Yaw( float angle )
{
	NiMatrix3 r;
	r.MakeRotation( angle, NiPoint3::UNIT_Y );
	mNiCamera->SetRotate( r * mNiCamera->GetRotate() );
}

void cCamera::SetViewPort( float l, float r, float t, float b )
{
	mNiCamera->SetViewPort( NiRect<float>( l, r, t, b ) );
}

void cCamera::SetFarDistance( float farDist )
{
	if( mOrtho )
	{
		if( farDist > (float)MAX_FAR_DIST )
			farDist = MAX_FAR_DIST;

		mFarDistance = farDist;
		mNiCamera->SetViewFrustum( NiFrustum( mLeft, mRight, mTop, mBottom, mNearDistance, farDist, mOrtho ) );
	}
	else
	{
		SetViewFrustum( mScreenWidth, mScreenHeight, mFovyDegree, mNearDistance, farDist );
	}
}

void cCamera::SetViewFrustum( unsigned int screenWidth, unsigned int screenHeight )
{
	SetViewFrustum( screenWidth, screenHeight, mFovyDegree, mNearDistance, mFarDistance );
}

void cCamera::SetViewFrustum( unsigned int screenWidth, unsigned int screenHeight, float fovyDegree, float nearDist, float farDist )
{
	if( mOrtho == false )
	{
		mScreenWidth = screenWidth;
		mScreenHeight = screenHeight;
		mFovyDegree = fovyDegree;
		mAspectRatio = float(screenWidth) / float(screenHeight);

		fovyDegree *= 0.5f;
		float fovyRadian = fovyDegree * NI_PI / 180.0f;
		float viewPlaneHalfHeight = ::tanf( fovyRadian / 2.0f );
		float viewPlaneHalfWidth = viewPlaneHalfHeight * mAspectRatio;

		SetViewFrustum( -viewPlaneHalfWidth, viewPlaneHalfWidth, viewPlaneHalfHeight, -viewPlaneHalfHeight, nearDist, farDist );
	}
}

void cCamera::SetViewFrustum( float l, float r, float t, float b, float n, float f )
{
	if( n < (float)MIN_NEAR_DIST )
		n = (float)MIN_NEAR_DIST;
	if( f > (float)MAX_FAR_DIST )
		f = MAX_FAR_DIST;

	mLeft = l;
	mRight = r;
	mTop = t;
	mBottom = b;
	mNearDistance = n;
	mFarDistance = f;
	mNiCamera->SetViewFrustum( NiFrustum( l, r, t, b, n, f, mOrtho ) );
}
