#include "stdafx.h"
#include "FreeCamera.h"

#include "MouseAgent.h"
#include "KeyAgent.h"

cFreeCamera::cFreeCamera()
: mTarget( NiPoint3::ZERO )
{
}

cFreeCamera::~cFreeCamera()
{
}

void cFreeCamera::OnProcessInput( float /*time*/ )
{
	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) )
	{
		return;
	}

	float rspeed = mRotateSpeed * 0.01f;
	float tspeed = mTranslateSpeed * 0.01f; // time;

	bool rotate = false;
	int dx = 0;
	int dy = 0;

	if( MOUSE->IsRButtonDown() )
	{
		rotate = true;
		dx = MOUSE->GetDeltaX();
		dy = MOUSE->GetDeltaY();
	}
	else if( KEY->IsDown( KEY_Q ) )
	{
		rotate = true;
		dx = -6;
	}
	else if( KEY->IsDown( KEY_E ) )
	{
		rotate = true;
		dx = 6;
	}

	if( rotate )
	{
		if( dx )
		{
			float yawAngle = float(dx) * rspeed * NI_PI / 180.0f;
			Yaw( yawAngle );
		}
		if( dy )
		{
			float pitchAngle = float(dy) * rspeed * NI_PI / 180.0f;
			Pitch( pitchAngle );
		}
	}
	if( MOUSE->IsMButtonDown() )
	{
		dx = MOUSE->GetDeltaX();
		dy = MOUSE->GetDeltaY();

		if( dx )
			Translate( NiPoint3::UNIT_X * float(dx) * tspeed * 0.5f );
		if( dy )
			Translate( NiPoint3::UNIT_Z * float(dy) * -tspeed * 0.5f );
	}

	/// 마우스 휠
	float dw = MOUSE->GetDeltaWheel();
	if( dw )
		Translate( NiPoint3::UNIT_Y * dw );

	/// 키
	if( KEY->IsDown( KEY_W ) )
	{
		Translate( NiPoint3::UNIT_Y * +tspeed );
	}
	if( KEY->IsDown( KEY_S ) )
	{
		Translate( NiPoint3::UNIT_Y * -tspeed );
	}
	if( KEY->IsDown( KEY_A ) )
	{
		Translate( NiPoint3::UNIT_X * -tspeed );
	}
	if( KEY->IsDown( KEY_D ) )
	{
		Translate( NiPoint3::UNIT_X * +tspeed );
	}
}

bool cFreeCamera::Save( cFileSaver& saver )
{
	/// 이동
	saver.Write( &(mNiCamera->GetTranslate()), sizeof(NiPoint3) );

	/// 회전
	saver.Write( &(mNiCamera->GetRotate()), sizeof(NiMatrix3) );

	/// 직교모드 여부
	saver.WriteUnsignedInt( mOrtho ? 1 : 0 );

	/// 원거리
	saver.WriteFloat( mFarDistance );
	return true;
}

void cFreeCamera::LookAt( const NiPoint3& target )
{
	mTarget = target;
	mOrientNode->Update( 0.0f );

	mNiCamera->LookAtWorldPoint( target, mOrtho ? NiPoint3::UNIT_Y : NiPoint3::UNIT_Z );
}

void cFreeCamera::LookAt( float x, float y, float z )
{
	LookAt( NiPoint3(x, y, z) );
}
