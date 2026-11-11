/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 자유 카메라
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

/// 자유 카메라
class cFreeCamera : public cCamera
{
public:
	cFreeCamera();
	virtual ~cFreeCamera();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 바라보는 지점을 설정
	void LookAt( const NiPoint3& target );
	void LookAt( float x, float y, float z );

protected:
	virtual void ProcessInput( unsigned long time );

protected:
	/// 목표점
	NiPoint3 mTarget;
};
