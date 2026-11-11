#include "stdafx.h"
#include "FreeCamera.h"

#include "InputSystem.h"

#include "box.h"
#include "NaviMesh.h"

cFreeCamera::cFreeCamera()
{
	mSpeedOption = 1.0f;
}

cFreeCamera::~cFreeCamera()
{
}

bool cFreeCamera::Init( float width, float height )
{
	mSpeed = 2500.0f;

	return cCamera::Init( width, height );
}


void cFreeCamera::OnProcessMouse( unsigned long time )
{
	if( KEYBOARD->GetKeyDown( KEY_CONTROL ) )
	{
		if( mSpeedOption == 1.0f )
			mSpeedOption = 0.5f;
		else
			mSpeedOption = 1.0f;
	}

	float dt = float(time) * 0.001f;

	float tspeed = mSpeed * mSpeedOption * dt;

	int dx = MOUSE->GetDeltaX();
	int dy = MOUSE->GetDeltaY();

	if( dx || dy )
	{
		if( MOUSE->RButtonPressed() )
		{
			float rspeed = 5.f * dt;

			if( dx )
			{
				float yawAngle = float(dx) * rspeed * NI_PI / 180.f;
				Yaw( yawAngle );
			}
			if( dy )
			{
				float pitchAngle = float(dy) * rspeed * NI_PI / 180.f;
				Pitch( pitchAngle );
			}
		}
//		if( MOUSE->MButtonPressed() )
//		{
//			if( dx )
//				Translate( NiPoint3::UNIT_X * float(dx) * tspeed * 0.5f );
//			if( dy )
//				Translate( NiPoint3::UNIT_Z * float(dy) * -tspeed * 0.5f );
//		}
	}

	float dw = (float)MOUSE->GetWheel();
	if( dw )
		Translate( NiPoint3::UNIT_Y * dw );

	if( KEYBOARD->GetKeyPressed( KEY_W ) )
	{
		Translate( NiPoint3::UNIT_Y * +tspeed );
	}
	if( KEYBOARD->GetKeyPressed( KEY_S ) )
	{
		Translate( NiPoint3::UNIT_Y * -tspeed );
	}
	if( KEYBOARD->GetKeyPressed( KEY_A ) )
	{
		Translate( NiPoint3::UNIT_X * -tspeed );
	}
	if( KEYBOARD->GetKeyPressed( KEY_D ) )
	{
		Translate( NiPoint3::UNIT_X * +tspeed );
	}

	NiPoint3 pos = mNiCamera->GetWorldTranslate();

	/// 네비메시와 충돌 검사
	const cBox& box = NAVIMESH->GetBoundBox();
	if( pos.x < box.GetMin().x || pos.y < box.GetMin().y ||
		pos.x > box.GetMax().x || pos.y > box.GetMax().y )
	{
		pos.x = 500.0f;
		pos.y = 500.0f;
	}

	float z;
	if( NAVIMESH->CalcHeight( &z, pos.x, pos.y) )
	{
		if( z+50.0f > pos.z )
			Translate( NiPoint3(0.0f,0.0f, z-pos.z+50.0f ) );
	}
}

void cFreeCamera::SetLookAt( const NiPoint3& target )
{
	if( mNiCamera->LookAtWorldPoint( mLookAt, -NiPoint3::UNIT_Z ) == false )
		return;

	NiMatrix3 mat = mNiCamera->GetRotate();
	mOrientNode->SetRotate( mOrientNode->GetRotate() * mat );
}