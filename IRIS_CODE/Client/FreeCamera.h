/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 시점 자유 카메라
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

/// 시점 자유 카메라
class cFreeCamera : public cCamera
{
public:
	cFreeCamera();
	virtual ~cFreeCamera();

	virtual bool Init( float width = 800.f, float height = 600.f );

	virtual void SetLookAt( const NiPoint3& target );

protected:
	virtual void OnProcessMouse( unsigned long time );

protected:
	float	mSpeedOption;
};
