/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 반직선
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 반직선
/// 반직선의 시작점과 방향으로 정의한다.
class cRay
{
public:
	cRay();
	cRay( const NiPoint3& origin, const NiPoint3& dir );

	/// 설정
	/// 반직선의 방향은 단위 벡터여야 한다.
	void Set( const NiPoint3& origin, const NiPoint3& dir );

	/// 시작점
	void SetOrigin( const NiPoint3& origin );
	const NiPoint3& GetOrigin() const;

	/// 방향
	/// 반직선의 방향은 단위 벡터여야 한다.
	void SetDirection( const NiPoint3& dir );
	const NiPoint3& GetDirection() const;

	/// 교차 검사
	bool IntersectTri( const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const;
	bool IntersectTri( NiPoint3* out, const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const;
	bool IntersectTri( NiPoint3* out, NiPoint2* uv, const NiPoint3& v0, const NiPoint3& v1, const NiPoint3& v2 ) const;

protected:
	/// 시작점
	NiPoint3 mOrigin;

	/// 방향
	NiPoint3 mDirection;
};

inline cRay::cRay()
{
}

inline cRay::cRay( const NiPoint3& origin, const NiPoint3& dir )
{
	Set( origin, dir );
}

inline void cRay::Set( const NiPoint3& origin, const NiPoint3& dir )
{
	this->mOrigin = origin;
	this->mDirection = dir;
}

inline void cRay::SetOrigin( const NiPoint3& origin )
{
	this->mOrigin = origin;
}

inline void cRay::SetDirection( const NiPoint3& dir )
{
	this->mDirection = dir;
}

inline const NiPoint3& cRay::GetOrigin() const
{
	return mOrigin;
}

inline const NiPoint3& cRay::GetDirection() const
{
	return mDirection;
}
