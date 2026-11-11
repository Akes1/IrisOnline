#include "stdafx.h"
#include "Ray.h"

bool cRay::IntersectTri( const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const
{
	/// find vectors for two edges sharing vert0
	NiPoint3 e1 = v1 - v0;
	NiPoint3 e2 = v2 - v0;

	/// begin calculating determinant - also used to calculate U parameter
	NiPoint3 pvec = mDirection.Cross( e2 );

	/// if determinant is near zero, ray lies in plane of triangle
	float det = e1.Dot( pvec );

	NiPoint3 tvec;
	if( det > 0.0f )
	{
		tvec = mOrigin - v0;
	}
	else
	{
		tvec = v0 - mOrigin;
		det = -det;
	}

	if( det < 0.0001f )
		return false;

	/// calculate U parameter and test bounds
	float u = tvec.Dot( pvec );
	if( u < 0.0f || u > det )
		return false;

	/// prepare to test V parameter
	NiPoint3 qvec = tvec.Cross( e1 );

	/// calculate V parameter and test bounds
	float v = mDirection.Dot( qvec );
	if( v < 0.0f || u + v > det )
		return false;

	/// calculate t, scale parameters, ray intersects triangle
	float t = e2.Dot( qvec );
	if( t < 0.0f )
		return false;

	return true;
}

bool cRay::IntersectTri( NiPoint3* out, const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const
{
	/// find vectors for two edges sharing vert0
	NiPoint3 e1 = v1 - v0;
	NiPoint3 e2 = v2 - v0;

	/// begin calculating determinant - also used to calculate U parameter
	NiPoint3 pvec = mDirection.Cross( e2 );

	/// if determinant is near zero, ray lies in plane of triangle
	float det = e1.Dot( pvec );

	if( det < 0.0001f )
		return false;

	NiPoint3 tvec = mOrigin - v0;

	/// calculate U parameter and test bounds
	float u = tvec.Dot( pvec );
	if( u < 0.0f || u > det )
		return false;

	/// prepare to test V parameter
	NiPoint3 qvec = tvec.Cross( e1 );

	/// calculate V parameter and test bounds
	float v = mDirection.Dot( qvec );
	if( v < 0.0f || u + v > det )
		return false;

	/// calculate t, scale parameters, ray intersects triangle
	float t = e2.Dot( qvec );
	if( t < 0.0f )
		return false;

	*out = mOrigin + mDirection * ( t / det );
	return true;
}

bool cRay::IntersectTri( NiPoint3* out, NiPoint2* uv, const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const
{
	/// find vectors for two edges sharing vert0
	NiPoint3 e1 = v1 - v0;
	NiPoint3 e2 = v2 - v0;

	/// begin calculating determinant - also used to calculate U parameter
	NiPoint3 pvec = mDirection.Cross( e2 );

	/// if determinant is near zero, ray lies in plane of triangle
	float det = e1 * pvec;

	if( det < 0.0001f )
		return false;

	NiPoint3 tvec = mOrigin - v0;

	/// calculate U parameter and test bounds
	float u = tvec.Dot( pvec );
	if( u < 0.0f || u > det )
		return false;

	/// prepare to test V parameter
	NiPoint3 qvec = tvec.Cross( e1 );

	/// calculate V parameter and test bounds
	float v = mDirection.Dot( qvec );
	if( v < 0.0f || u + v > det )
		return false;

	/// calculate t, scale parameters, ray intersects triangle
	float t = e2.Dot( qvec );
	if( t < 0.0f )
		return false;

	float invDet = 1.0f / det;
	uv->x = u * invDet;
	uv->y = v * invDet;
	*out = mOrigin + mDirection * ( t * invDet );
	return true;
}
