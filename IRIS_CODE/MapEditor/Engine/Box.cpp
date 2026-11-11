#include "stdafx.h"
#include "Box.h"

#include "Ray.h"
#include "Sphere.h"

bool cBox::IntersectRay( const cRay& r, float& scale ) const
{
	const NiPoint3 &origin = r.GetOrigin();
	const NiPoint3 &dir = r.GetDirection();

	int ax0, ax1, ax2, side, inside;
	float f;
	NiPoint3 hit;

	ax0 = -1;
	inside = 0;

	for( int i = 0; i < 3; ++i )
	{
		if( origin[i] < mPoints[0][i] )
		{
			side = 0;
		}
		else if( origin[i] > mPoints[1][i] )
		{
			side = 1;
		}
		else
		{
			inside++;
			continue;
		}
		if( dir[i] == 0.0f )
		{
			continue;
		}
		f = ( origin[i] - mPoints[side][i] );
		if( ax0 < 0 || NiAbs( f ) > NiAbs( scale * dir[i] ) )
		{
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}

	if( ax0 < 0 )
	{
		scale = 0.0f;
		/// return true if the start point is inside the bounds
		return ( inside == 3 );
	}

	ax1 = (ax0 + 1) % 3;
	ax2 = (ax0 + 2) % 3;
	hit[ax1] = origin[ax1] + scale * dir[ax1];
	hit[ax2] = origin[ax2] + scale * dir[ax2];

	return ( hit[ax1] >= mPoints[0][ax1] && hit[ax1] <= mPoints[1][ax1] &&
		hit[ax2] >= mPoints[0][ax2] && hit[ax2] <= mPoints[1][ax2] );
}

bool cBox::IntersectRay( const cRay& r, float& scale, float maxDistance ) const
{
	const NiPoint3 &origin = r.GetOrigin();
	const NiPoint3 &dir = r.GetDirection();

	int ax0, ax1, ax2, side, inside;
	float f;
	NiPoint3 hit;

	ax0 = -1;
	inside = 0;

	for( int i = 0; i < 3; ++i )
	{
		if( origin[i] < mPoints[0][i] )
		{
			side = 0;
		}
		else if( origin[i] > mPoints[1][i] )
		{
			side = 1;
		}
		else
		{
			inside++;
			continue;
		}
		if( dir[i] == 0.0f )
		{
			continue;
		}
		f = ( origin[i] - mPoints[side][i] );
		if( ax0 < 0 || NiAbs( f ) > NiAbs( scale * dir[i] ) )
		{
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}

	if( ax0 < 0 )
	{
		scale = 0.0f;
		/// return true if the start point is inside the bounds
		return ( inside == 3 );
	}
	if( scale > maxDistance )
		return false;

	ax1 = (ax0 + 1) % 3;
	ax2 = (ax0 + 2) % 3;
	hit[ax1] = origin[ax1] + scale * dir[ax1];
	hit[ax2] = origin[ax2] + scale * dir[ax2];

	return ( hit[ax1] >= mPoints[0][ax1] && hit[ax1] <= mPoints[1][ax1] &&
		hit[ax2] >= mPoints[0][ax2] && hit[ax2] <= mPoints[1][ax2] );
}

bool cBox::IntersectSphere( const cSphere& sphere ) const
{
	const NiPoint3& c = sphere.GetCenter();
	float s, d = 0.0f;

	// x
	if( c.x < mPoints[0].x )
	{
		s = c.x - mPoints[0].x;
		d += s * s;
	}
	else if( c.x > mPoints[1].x )
	{
		s = c.x - mPoints[1].x;
		d += s * s;
	}

	// y
	if( c.y < mPoints[0].y )
	{
		s = c.y - mPoints[0].y;
		d += s * s;
	}
	else if( c.y > mPoints[1].y )
	{
		s = c.y - mPoints[1].y;
		d += s * s;
	}

	// z
	if( c.z < mPoints[0].z )
	{
		s = c.z - mPoints[0].z;
		d += s * s;
	}
	else if( c.z > mPoints[1].z )
	{
		s = c.z - mPoints[1].z;
		d += s * s;
	}

	if( d > sphere.GetRadius2() )
		return false;
	return true;
}

bool cBox::IntersectBox( const cBox& box ) const
{
	const NiPoint3& min = box.GetMin();
	const NiPoint3& max = box.GetMax();

	/// x
	if( mPoints[0].x > max.x || mPoints[1].x < min.x )
		return false;
	/// y
	if( mPoints[0].y > max.y || mPoints[1].y < min.y )
		return false;
	/// z
	if( mPoints[0].z > max.z || mPoints[1].z < min.z )
		return false;
	return true;
}
