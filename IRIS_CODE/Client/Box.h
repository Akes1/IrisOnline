/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : (축정렬) 상자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cRay;
class cSphere;

/// (축정렬) 상자
/// 주로 교차 검사 및 가시 검사를 위한 경계 상자로 사용된다.
/// 상자의 최소점과 최대점으로 정의한다.
class cBox
{
public:
	cBox();
	cBox( const cBox& box );
	cBox( const NiPoint3& minPoint, const NiPoint3& maxPoint );
	cBox( float minx, float miny, float minz, float maxx, float maxy, float maxz );

	/// 설정
	void Set( const NiPoint3& minPoint, const NiPoint3& maxPoint );
	void Set( float minx, float miny, float minz, float maxx, float maxy, float maxz );

	/// 최소점
	const NiPoint3& GetMin() const;

	/// 최대점
	const NiPoint3& GetMax() const;

	/// 추가
	/// 확장 여부를 리턴한다.
	bool AddPoint( const NiPoint3& point );
	bool AddPointX( float x );
	bool AddPointY( float y );
	bool AddPointZ( float z );
	bool AddBox( const cBox& box );
	bool AddBox( const NiPoint3& minPoint, const NiPoint3& maxPoint );

	/// 교차 검사
	bool IntersectTri( const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const;
	/// 주의: 반직선의 원점이 상자 안에 있을 때는 거리 제한을 무시하고 무조건 충돌한 것으로 본다.
	/// intersection point is origin + dir * scale
	bool IntersectRay( const cRay& r, float& scale ) const;
	bool IntersectRay( const cRay& r, float& scale, float maxDistance ) const;
	bool IntersectSphere( const cSphere& sphere ) const;
	bool IntersectBox( const cBox& box ) const;
	bool IntersectBox( const NiPoint3& minPoint, const NiPoint3& maxPoint ) const;

	/// 포함 검사
	bool ContainBox( const cBox& box ) const;
	bool ContainBox( const NiPoint3& minPoint, const NiPoint3& maxPoint ) const;

	/// 평면의 어느쪽
	unsigned int WhichSide( const NiPlane& plane ) const;

protected:
	NiPoint3 mPoints[2];
};

inline cBox::cBox()
{
	mPoints[0].x = NI_INFINITY;
	mPoints[0].y = NI_INFINITY;
	mPoints[0].z = NI_INFINITY;
	mPoints[1].x = -NI_INFINITY;
	mPoints[1].y = -NI_INFINITY;
	mPoints[1].z = -NI_INFINITY;
}

inline cBox::cBox( const cBox& box )
{
	mPoints[0] = box.mPoints[0];
	mPoints[1] = box.mPoints[1];
}

inline cBox::cBox( const NiPoint3& min, const NiPoint3& max )
{
	mPoints[0] = min;
	mPoints[1] = max;
}

inline cBox::cBox( float minx, float miny, float minz, float maxx, float maxy, float maxz )
{
	mPoints[0].x = minx;
	mPoints[0].y = miny;
	mPoints[0].z = minz;
	mPoints[1].x = -maxx;
	mPoints[1].y = -maxy;
	mPoints[1].z = -maxz;
}

inline void cBox::Set( const NiPoint3& min, const NiPoint3& max )
{
	mPoints[0] = min;
	mPoints[1] = max;
}

inline void cBox::Set( float minx, float miny, float minz, float maxx, float maxy, float maxz )
{
	mPoints[0].x = minx;
	mPoints[0].y = miny;
	mPoints[0].z = minz;
	mPoints[1].x = maxx;
	mPoints[1].y = maxy;
	mPoints[1].z = maxz;
}

inline const NiPoint3& cBox::GetMin() const
{
	return mPoints[0];
}

inline const NiPoint3& cBox::GetMax() const
{
	return mPoints[1];
}

inline bool cBox::AddPoint( const NiPoint3& pt )
{
	bool expanded = false;

	if( mPoints[0].x > pt.x )
	{
		mPoints[0].x = pt.x;
		expanded = true;
	}
	if( mPoints[0].y > pt.y )
	{
		mPoints[0].y = pt.y;
		expanded = true;
	}
	if( mPoints[0].z > pt.z )
	{
		mPoints[0].z = pt.z;
		expanded = true;
	}
	if( mPoints[1].x < pt.x )
	{
		mPoints[1].x = pt.x;
		expanded = true;
	}
	if( mPoints[1].y < pt.y )
	{
		mPoints[1].y = pt.y;
		expanded = true;
	}
	if( mPoints[1].z < pt.z )
	{
		mPoints[1].z = pt.z;
		expanded = true;
	}
	return expanded;
}

inline bool cBox::AddPointX( float x )
{
	bool expanded = false;

	if( mPoints[0].x > x )
	{
		mPoints[0].x = x;
		expanded = true;
	}
	if( mPoints[1].x < x )
	{
		mPoints[1].x = x;
		expanded = true;
	}
	return expanded;
}

inline bool cBox::AddPointY( float y )
{
	bool expanded = false;

	if( mPoints[0].y > y )
	{
		mPoints[0].y = y;
		expanded = true;
	}
	if( mPoints[1].y < y )
	{
		mPoints[1].y = y;
		expanded = true;
	}
	return expanded;
}

inline bool cBox::AddPointZ( float z )
{
	bool expanded = false;

	if( mPoints[0].z > z )
	{
		mPoints[0].z = z;
		expanded = true;
	}
	if( mPoints[1].z < z )
	{
		mPoints[1].z = z;
		expanded = true;
	}
	return expanded;
}

inline bool cBox::AddBox( const cBox& box )
{
	return AddBox( box.mPoints[0], box.mPoints[1] );
}

inline bool cBox::AddBox( const NiPoint3& min, const NiPoint3& max )
{
	bool expanded = false;

	if( mPoints[0].x > min.x )
	{
		mPoints[0].x = min.x;
		expanded = true;
	}
	if( mPoints[0].y > min.y )
	{
		mPoints[0].y = min.y;
		expanded = true;
	}
	if( mPoints[0].z > min.z )
	{
		mPoints[0].z = min.z;
		expanded = true;
	}
	if( mPoints[1].x < max.x )
	{
		mPoints[1].x = max.x;
		expanded = true;
	}
	if( mPoints[1].y < max.y )
	{
		mPoints[1].y = max.y;
		expanded = true;
	}
	if( mPoints[1].z < max.z )
	{
		mPoints[1].z = max.z;
		expanded = true;
	}
	return expanded;
}

inline bool cBox::ContainBox( const cBox& box ) const
{
	if( mPoints[0].x > box.mPoints[0].x )
		return false;
	if( mPoints[0].y > box.mPoints[0].y )
		return false;
	if( mPoints[0].z > box.mPoints[0].z )
		return false;
	if( mPoints[1].x < box.mPoints[1].x )
		return false;
	if( mPoints[1].y < box.mPoints[1].y )
		return false;
	if( mPoints[1].z < box.mPoints[1].z )
		return false;
	return true;
}

inline unsigned int cBox::WhichSide( const NiPlane& plane ) const
{
	const NiPoint3& n = plane.GetNormal();
	NiPoint3 v0, v1;

	// x
	if( n.x >= 0.0f )
	{
		v0.x = mPoints[1].x;
		v1.x = mPoints[0].x;
	}
	else
	{
		v0.x = mPoints[0].x;
		v1.x = mPoints[1].x;
	}

	// y
	if( n.y >= 0.0f )
	{
		v0.y = mPoints[1].y;
		v1.y = mPoints[0].y;
	}
	else
	{
		v0.y = mPoints[0].y;
		v1.y = mPoints[1].y;
	}

	// z
	if( n.z >= 0.0f )
	{
		v0.z = mPoints[1].z;
		v1.z = mPoints[0].z;
	}
	else
	{
		v0.z = mPoints[0].z;
		v1.z = mPoints[1].z;
	}

	if( plane.Distance( v0 ) < 0.0f )
		return NiPlane::NEGATIVE_SIDE;
	if( plane.Distance( v1 ) > 0.0f )
		return NiPlane::POSITIVE_SIDE;
	else
		return NiPlane::NO_SIDE;
}
