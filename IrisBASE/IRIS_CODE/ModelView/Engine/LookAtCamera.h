/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.08
* 내    용 : 시점 고정 카메라 클래스
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Camera.h"

/// 시점 고정 카메라
class cLookAtCamera : public cCamera
{
public:
	cLookAtCamera();
	virtual ~cLookAtCamera();

	/// 시점
	void SetLookAt( const NiPoint3& target );
	void SetLookAt( float x, float y, float z );
	const NiPoint3& GetLookAt() const;

	/// 목표점으로부터의 실제 거리
	float GetActualLookAtDistance() const;
	float GetDesiredLookAtDistance() const;

	void SetMinCameraDist( float dist );
	void SetMaxCameraDist( float dist );

protected:
	/// 처리
	virtual void OnProcess( float deltaTime );
	virtual void OnProcessInput( float deltaTime );

protected:
	/// 시점
	NiPoint3 mLookAt;

	float mMinCameraDist;
	float mMaxCameraDist;

	/// 방향
	NiPoint3 mTargetDirection;
	NiPoint3 mDirection;

	/// 위치
	NiPoint3 mTargetPos;
	NiPoint3 mPos;

	/// 회전 각도
	//float mTargetYawAngle;
	float mYawAngle;
	//float mTargetPitchAngle;
	float mPitchAngle;

	/// 시점과 카메라 사이의 거리
	float mDesiredLookAtDistance;

	/// 보정된 시점과 카메라 사이의 실제 거리
	float mActualLookAtDistance;
};

inline void cLookAtCamera::SetLookAt( const NiPoint3& target )
{
	mLookAt = target;
}

inline void cLookAtCamera::SetLookAt( float x, float y, float z )
{
	mLookAt.x = x;
	mLookAt.y = y;
	mLookAt.z = z;
}

inline const NiPoint3& cLookAtCamera::GetLookAt() const
{
	return mLookAt;
}

inline float cLookAtCamera::GetActualLookAtDistance() const
{
	return mActualLookAtDistance;
}

inline float cLookAtCamera::GetDesiredLookAtDistance() const
{
	return mDesiredLookAtDistance;
}
