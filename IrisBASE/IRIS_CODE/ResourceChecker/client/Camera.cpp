#include "stdafx.h"
#include "Camera.h"

cCamera::cCamera()
: mWidth( 0.0f )
, mHeight( 0.0f )
, mFovyDegree( 75.0f )
, mFarDistance( (float)MAX_FAR_DIST )
, mAspectRatio( 1.0f )
, mSpeed( 5000.0f )
, mLookAt( NiPoint3::ZERO )
, mLookAtDistance( 0.0f )
, mNeedUpdate( true )
{
}

cCamera::~cCamera()
{
	assert(mOrientNode->GetRefCount()==1);
	mOrientNode = 0;

	assert(mNiCamera->GetRefCount()==1);
	mNiCamera = 0;
}

void cCamera::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	Update( accumTime );
}

bool cCamera::Init( float width, float height )
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

	mOrientNode = NiNew NiNode;
	mOrientNode->AttachChild( mNiCamera );
	mOrientNode->SetSelectiveUpdate( true );
	mOrientNode->SetSelectiveUpdateTransforms( true );
	mOrientNode->SetSelectiveUpdatePropertyControllers( true );
	mOrientNode->SetSelectiveUpdateRigid( false );

	mOrientNode->SetTranslate( NiPoint3::ZERO );
	mOrientNode->SetRotate( rotZ * rotX );
	mOrientNode->Update( 0.f );
	return true;
}

void cCamera::ChangeNiCamera( NiCamera* cam )
{
	mOrientNode->DetachChild( mNiCamera );

	mNiCamera = cam;
	mOrientNode->AttachChild( mNiCamera );

	SetViewPort( 0.f, 1.f, 1.f, 0.f );
	SetViewFrustum( mWidth, mHeight );

	mOrientNode->Update( 0.f );
}

void cCamera::SetTranslate( float x, float y, float z )
{
	mNiCamera->SetTranslate( y, z, x );
}

void cCamera::SetTranslate( const NiPoint3& trans )
{
	mNiCamera->SetTranslate( trans.y, trans.z, trans.x );
}

void cCamera::Translate( const NiPoint3& move )
{
	NiPoint3 trans = mNiCamera->GetTranslate();
	trans += mNiCamera->GetRotate() * NiPoint3(move.y, move.z, move.x);
	mNiCamera->SetTranslate( trans );
}

void cCamera::SetRotate( const NiMatrix3& rot )
{
	mNiCamera->SetRotate( rot );
}

void cCamera::Rotate( const NiPoint3& axis, float angle )
{
	NiMatrix3 r;
	r.MakeRotation( angle, axis.y, axis.z, axis.x );
	mNiCamera->SetRotate( r * mNiCamera->GetRotate() );
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
//	SetViewFrustum( mWidth, mHeight, mFovyDegree, MAX_FAR_DIST );
	SetViewFrustum( mWidth, mHeight, mFovyDegree, farDist );
}

void cCamera::SetViewFrustum( float width, float height )
{
	SetViewFrustum( width, height, mFovyDegree, mFarDistance );
}

void cCamera::SetViewFrustum( float width, float height, float fovyDegree, float farDist )
{
	mWidth = width;
	mHeight = height;
	mFovyDegree = fovyDegree;
	mAspectRatio = width / height;

	float fovyRadian = fovyDegree * (NI_PI / 180.f);
	float viewPlaneHalfHeight = tanf( fovyRadian / 2.0f ) * 0.5f;
	float viewPlaneHalfWidth = viewPlaneHalfHeight * mAspectRatio;

	SetViewFrustum( -viewPlaneHalfWidth, viewPlaneHalfWidth, viewPlaneHalfHeight, -viewPlaneHalfHeight, farDist );
}

void cCamera::SetViewFrustum( float l, float r, float t, float b, float f )
{
	mFarDistance = f > (float)MAX_FAR_DIST ? (float)MAX_FAR_DIST : f;
	assert( mFarDistance > float(MIN_NEAR_DIST) );

	mNiCamera->SetViewFrustum( NiFrustum( l, r, t, b, (float)MIN_NEAR_DIST, mFarDistance ) );
}
