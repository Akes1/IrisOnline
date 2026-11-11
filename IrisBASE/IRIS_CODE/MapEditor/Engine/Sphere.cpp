#include "stdafx.h"
#include "Sphere.h"
#include "Ray.h"

bool cSphere::IntersectRay( const cRay& ray ) const
{
	NiPoint3 l = mCenter - ray.GetOrigin();
	float s = l.Dot( ray.GetDirection() );
	float ll = l.SqrLength();

	if( s < 0 && ll > mRadius2 )
		return false;

	float mm = ll - s * s;
	if( mm > mRadius2 )
		return false;
	return true;
}

bool cSphere::IntersectRay( const cRay& ray, float maxDistance ) const
{
	NiPoint3 l = mCenter - ray.GetOrigin();
	float s = l.Dot( ray.GetDirection() );
	float ll = l.SqrLength();

	if( s < 0 && ll > mRadius2 )
		return false;

	float mm = ll - s * s;
	if( mm > mRadius2 )
		return false;

	float q = NiSqrt( mRadius2 - mm );

	if( ll > mRadius2 && (s - q) > maxDistance )
		return false;
	return true;
}

bool cSphere::IntersectRay( NiPoint3* out, float* dist, const cRay& ray ) const
{
	const NiPoint3& o = ray.GetOrigin();
	const NiPoint3& d = ray.GetDirection();

	NiPoint3 l = mCenter - o;
	float s = l.Dot( d );
	float ll = l.SqrLength();

	if( s < 0 && ll > mRadius2 )
		return false;

	float mm = ll - s * s;
	if( mm > mRadius2 )
		return false;

	float q = NiSqrt( mRadius2 - mm );
	float t = ll > mRadius2 ? s - q : s + q;
	*out = o + d * t;
	*dist = t;
	return true;
}

bool cSphere::IntersectRay( NiPoint3* out, float* dist, const cRay& ray, float maxDistance ) const
{
	const NiPoint3& o = ray.GetOrigin();
	const NiPoint3& d = ray.GetDirection();

	NiPoint3 l = mCenter - o;
	float s = l.Dot( d );
	float ll = l.SqrLength();

	if( s < 0 && ll > mRadius2 )
		return false;

	float mm = ll - s * s;
	if( mm > mRadius2 )
		return false;

	float q = NiSqrt( mRadius2 - mm );
	float t;

	if( ll > mRadius2 )
	{
		t = s - q;
		if( t > maxDistance )
			return false;
	}
	else
	{
		t = s + q;
	}

	*out = o + d * t;
	*dist = t;
	return true;
}

bool cSphere::IntersectLine( const NiPoint3& start, const NiPoint3& end ) const
{
	NiPoint3 r, s, e;
	float a;

	s = start - mCenter;
	e = end - mCenter;
	r = e - s;
	a = -s * r;

	if ( a <= 0 )
	{
		return ( s * s < mRadius * mRadius );
	}
	else if ( a >= r * r )
	{
		return ( e * e < mRadius * mRadius );
	}
	else
	{
		r = s + ( a / ( r * r ) ) * r;
		return ( r * r < mRadius * mRadius );
	}
}
