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

	void SetLookAtDistance( float dist );
	float GetLookAtDistance() const;
	float GetDesiredLookAtDistance() const;

	/// 갱신 여부
	bool GetNeedUpdate() const;

protected:
	/// 처리
	virtual void Process( unsigned long deltaTime );
	virtual void ProcessInput( unsigned long deltaTime );

protected:
	/// 시점과 카메라 사이의 거리
	float mDesiredLookAtDistance;
	float mLookAtDistance;

	/// 시점
	NiPoint3 mTargetLookAt;
	NiPoint3 mLookAt;

	/// 회전
	float mTargetYawAngle;
	float mTargetPitchAngle;
	float mYawAngle;
	float mPitchAngle;

	/// 갱신 여부
	bool mNeedUpdate;
};

inline
void cLookAtCamera::SetLookAt( const NiPoint3& target )
{
	mTargetLookAt = target;
}

inline
void cLookAtCamera::SetLookAt( float x, float y, float z )
{
	mTargetLookAt.x = x;
	mTargetLookAt.y = y;
	mTargetLookAt.z = z;
}

inline
const NiPoint3& cLookAtCamera::GetLookAt() const
{
	return mLookAt;
}

inline
void cLookAtCamera::SetLookAtDistance( float dist )
{
	mLookAtDistance = dist;
}

inline
float cLookAtCamera::GetLookAtDistance() const
{
	return mLookAtDistance;
}

inline
float cLookAtCamera::GetDesiredLookAtDistance() const
{
	return mDesiredLookAtDistance;
}

inline
bool cLookAtCamera::GetNeedUpdate() const
{
	return mNeedUpdate;
}
