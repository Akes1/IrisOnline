/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 구
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Box.h"

/// 구
/// 주로 교차 검사 및 가시 검사를 위한 경계 구로 사용된다.
/// 구의 중심점과 반지름으로 정의한다.
class cSphere
{
public:
	cSphere();
	cSphere( const NiPoint3& center, float radius );
	cSphere( float x, float y, float z, float radius );
	cSphere( const cSphere& other );

	/// 설정
	void Set( const NiPoint3& center, float radius );
	void Set( float x, float y, float z, float radius );

	/// 중심점
	void SetCenter( const NiPoint3& center );
	const NiPoint3& GetCenter() const;

	/// 반지름
	void SetRadius( float radius );
	float GetRadius() const;
	float GetRadius2() const;

	/// 교차 검사
	/// 주의: 반직선의 원점이 구 안에 있을 때는 거리 제한을 무시하고 무조건 충돌한 것으로 본다.
	bool IntersectRay( const cRay& ray ) const;
	bool IntersectRay( const cRay& ray, float maxDistance ) const;
	bool IntersectRay( NiPoint3* out, float* dist, const cRay& ray ) const;
	bool IntersectRay( NiPoint3* out, float* dist, const cRay& ray, float maxDistance ) const;
	bool IntersectLine( const NiPoint3& start, const NiPoint3& end ) const;
	bool IntersectPlane( const NiPlane& plane ) const;
	bool IntersectSphere( const cSphere& sphere ) const;

	/// 포함 검사
	bool ContainPoint( const NiPoint3& v ) const;
	bool ContainBox( const cBox& box ) const;
	bool ContainSphere( const cSphere& sphere ) const;

	/// 평면의 어느쪽
	unsigned int WhichSide( const NiPlane& plane ) const;

	/// 한 점에서 중점까지의 거리
	float GetDistance( const NiPoint3& point ) const;

	/// 한 점에서 중점까지의 거리의 제곱
	float GetDistance2( const NiPoint3& point ) const;

protected:
	/// 중심점
	NiPoint3 mCenter;

	/// 반지름
	float mRadius;

	/// 반지름의 제곱
	float mRadius2;
};

inline cSphere::cSphere()
{
	mCenter = NiPoint3::ZERO;
	mRadius = 0.0f;
	mRadius2 = 0.0f;
}

inline cSphere::cSphere( const NiPoint3& center, float radius )
{
	mCenter = center;
	mRadius = radius;
	mRadius2 = radius * radius;
}

inline cSphere::cSphere( float x, float y, float z, float radius )
{
	mCenter.x = x;
	mCenter.y = y;
	mCenter.z = z;
	mRadius = radius;
	mRadius2 = radius * radius;
}

inline cSphere::cSphere( const cSphere& other )
{
	mCenter = other.mCenter;
	mRadius = other.mRadius;
	mRadius2 = other.mRadius2;
}

inline void cSphere::Set( const NiPoint3& center, float radius )
{
	mCenter = center;
	mRadius = radius;
	mRadius2 = radius * radius;
}

inline void cSphere::Set( float x, float y, float z, float radius )
{
	mCenter.x = x;
	mCenter.y = y;
	mCenter.z = z;
	mRadius = radius;
	mRadius2 = radius * radius;
}

inline void cSphere::SetCenter( const NiPoint3& center )
{
	mCenter = center;
}

inline void cSphere::SetRadius( float radius )
{
	mRadius = radius;
	mRadius2 = radius * radius;
}

inline const NiPoint3& cSphere::GetCenter() const
{
	return mCenter;
}

inline float cSphere::GetRadius() const
{
	return mRadius;
}

inline float cSphere::GetRadius2() const
{
	return mRadius2;
}

inline bool cSphere::IntersectPlane( const NiPlane& plane ) const
{
	return NiAbs( plane.Distance(mCenter) ) <= mRadius;
}

inline bool cSphere::IntersectSphere( const cSphere& sphere ) const
{
	NiPoint3 l = mCenter - sphere.GetCenter();
	float r = mRadius + sphere.GetRadius();
	return l.SqrLength() <= r * r;
}

inline bool cSphere::ContainPoint( const NiPoint3& v ) const
{
	return (mCenter - v).SqrLength() <= mRadius2;
}

inline bool cSphere::ContainBox( const cBox& box ) const
{
	return ContainPoint( box.GetMin() ) && ContainPoint( box.GetMax() );
}

inline bool cSphere::ContainSphere(const cSphere& sphere) const
{
	if( mRadius < sphere.mRadius )
		return false;

	NiPoint3 l = mCenter - sphere.mCenter;
	float r = mRadius - sphere.mRadius;
	return l.SqrLength() <= r * r;
}

inline unsigned int cSphere::WhichSide( const NiPlane& plane ) const
{
	float dist = plane.Distance( mCenter );

	if( dist <= -mRadius )
		return NiPlane::NEGATIVE_SIDE;
	else if( dist >= mRadius )
		return NiPlane::POSITIVE_SIDE;
	else
		return NiPlane::NO_SIDE;
}

inline float cSphere::GetDistance(const NiPoint3& point) const
{
	return (mCenter - point).Length();
}

inline float cSphere::GetDistance2(const NiPoint3& point) const
{
	return (mCenter - point).SqrLength();
}
