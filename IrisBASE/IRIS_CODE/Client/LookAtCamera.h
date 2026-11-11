/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.08
* 내    용 : 시점 고정 카메라 클래스
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

class cNifAnimationInfo;


/// 시점 고정 카메라
class cLookAtCamera : public cCamera
{
public:
	cLookAtCamera();
	virtual ~cLookAtCamera();

	/// 초기화
	virtual bool Init( float width, float height );

	/// 스테이지 변경시 데이터 초기화
	void Reset();

	void ViewHeroFront( NiPoint3 heroDir );

	/// 처리
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );


	///
	virtual void Shake();

	void SetFixedPos() { mFixed = true; }

protected:
	virtual void OnProcessMouse( unsigned long time );

protected:
	/// 시점과 카메라 사이의 거리
	float mDesiredLookAtDistance;

	/// 회전
	float mTargetYawAngle;
	float mYawAngle;
	float mTargetPitchAngle;
	float mPitchAngle;

	///
	cNifAnimationInfo* mNifAniInfo;

	bool mFixed;
};
