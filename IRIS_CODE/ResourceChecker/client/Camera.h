/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 카메라
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 카메라
//////////////////////////////////////////////////////////////////////////
//       Y                         Z
// NI:   |__ Z         MAX(APP):   |__ X
//      /                         /
//    -X                        -Y
//////////////////////////////////////////////////////////////////////////
class cCamera
{
	friend class cCameraManager;
	static const unsigned int MIN_NEAR_DIST = 10;
	static const unsigned int MAX_FAR_DIST = 50000;

public:
	cCamera();
	virtual ~cCamera();

	/// 초기화 및 종료
	virtual bool Init( float width = 800.f, float height = 600.f );

	/// 카메라의 위치 방향 정보를 리셋
	virtual void Reset(){}// = 0;

	virtual void Shake() {}

	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	virtual void Update( unsigned long accumTime );

	/// 근거리
	float GetNearDistance() const;

	/// 원거리
	void SetFarDistance( float farDist );
	float GetFarDistance() const;

	/// 시야 각뿔대
	void SetViewFrustum( float width, float height );
	void SetViewFrustum( float width, float height, float fovyDegree, float farDist );
	void SetViewFrustum( float left, float right, float top, float bottom, float farDist );

	/// 뷰포트
	void SetViewPort( float left = 0.f, float right = 1.f, float top = 1.f, float bottom = 0.f );

	/// 이동
	void SetTranslate( const NiPoint3& trans );
	void SetTranslate( float x, float y, float z );
	void Translate( const NiPoint3& move );
	const NiPoint3& GetWorldTranslate() const;

	/// 회전
	void SetRotate( const NiMatrix3& rot );
	void Rotate( const NiPoint3& axis, float angle );
	void Pitch( float angle );
	void Yaw( float angle );

	/// 방향
	NiPoint3 GetWorldDirection() const;
	NiPoint3 GetWorldRightDir() const;

	NiNode*	GetOrientNode() { return mOrientNode; }

	void ChangeNiCamera( NiCamera* cam );

	/// 속력
	void SetSpeed( float unitsPerSec );

	/// 종횡비
	float GetAspectRatio() const;

	/// 마우스 좌표에 해당하는 반직선
	bool GetRayFromWindowPoint( long mouseX, long mouseY, NiPoint3& origin, NiPoint3& dir ) const;
	
	/// 시점
	virtual void SetLookAt( const NiPoint3& target );
	void SetLookAt( float x, float y, float z );
	const NiPoint3& GetLookAt() const;
	void ResetLookAt( const NiPoint3& target );

	/// 시점으로부터의 거리
	void SetLookAtDistance( float dist );
	float GetLookAtDistance() const;

	/// 갱신 여부
	bool GetNeedUpdate() const;

	/// 엔진 카메라로 형변환
	operator NiCamera*() const
	{
		return mNiCamera;
	}

protected:
	/// 입력 처리
	virtual void OnProcessMouse( unsigned long time ) = 0;

protected:
	/// 화면 크기
	float mWidth;
	float mHeight;

	/// 수직 시야각
	float mFovyDegree;

	/// 원거리
	float mFarDistance;

	/// 종횡비
	float mAspectRatio;

	/// 엔진 카메라
	NiCameraPtr mNiCamera;

	/// 카메라 변환용 노드
	NiNodePtr mOrientNode;

	/// 속력
	float mSpeed;

	/// 시점
	NiPoint3 mTargetLookAt;
	NiPoint3 mLookAt;

	/// 보정된 시점과 카메라 사이의 실제 거리
	float mLookAtDistance;

	/// 갱신 여부
	bool mNeedUpdate;
};

inline
void cCamera::Update( unsigned long accumTime )
{
	mOrientNode->Update( accumTime * 0.001f );
}

inline
void cCamera::SetSpeed( float unitsPerSec )
{
	mSpeed = unitsPerSec;
}

inline
float cCamera::GetNearDistance() const
{
	return float(MIN_NEAR_DIST);
}

inline
float cCamera::GetFarDistance() const
{
	return mFarDistance;
}

inline
float cCamera::GetAspectRatio() const
{
	return mAspectRatio;
}

inline
const NiPoint3& cCamera::GetWorldTranslate() const
{
	return mNiCamera->GetWorldTranslate();
}

inline
NiPoint3 cCamera::GetWorldDirection() const
{
	return mNiCamera->GetWorldDirection();
}

inline
NiPoint3 cCamera::GetWorldRightDir() const
{
	return mNiCamera->GetWorldRightVector();
}

inline
bool cCamera::GetRayFromWindowPoint( long mouseX, long mouseY, NiPoint3& origin, NiPoint3& dir ) const
{
	return mNiCamera->WindowPointToRay( mouseX, mouseY, origin, dir );
}

inline
void cCamera::SetLookAt( const NiPoint3& target )
{
	mTargetLookAt = target;
}

inline
void cCamera::SetLookAt( float x, float y, float z )
{
	mTargetLookAt.x = x;
	mTargetLookAt.y = y;
	mTargetLookAt.z = z;
}

inline
const NiPoint3& cCamera::GetLookAt() const
{
	return mLookAt;
}

inline
void cCamera::ResetLookAt( const NiPoint3& target )
{
	mLookAt = mTargetLookAt = target;
}

inline
void cCamera::SetLookAtDistance( float dist )
{
	mLookAtDistance = dist;
}

inline
float cCamera::GetLookAtDistance() const
{
	return mLookAtDistance;
}

inline
bool cCamera::GetNeedUpdate() const
{
	return mNeedUpdate;
}
