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
	static const unsigned int MIN_NEAR_DIST = 1;
	static const unsigned int MAX_FAR_DIST = 200000;

public:
	cCamera();
	virtual ~cCamera();

	/// 초기화 및 종료
	virtual bool Init( unsigned int screenWidth, unsigned int screenHeight );

	/// 카메라의 위치 방향 정보를 리셋
	virtual void Reset(){}// = 0;

	/// 갱신
	void Update( float time );

	/// 근거리
	float GetNearDistance() const;

	/// 원거리
	void SetFarDistance( float farDist );
	float GetFarDistance() const;

	/// 시야 각뿔대
	void SetViewFrustum( unsigned int screenWidth, unsigned int screenHeight );
	void SetViewFrustum( unsigned int screenWidth, unsigned int screenHeight, float fovyDegree, float nearDist, float farDist );
	void SetViewFrustum( float left, float right, float top, float bottom, float nearDist, float farDist );
	void SetOrtho( bool ortho );
	bool GetOrtho() const;

	/// 뷰포트
	void SetViewPort( float left = 0, float right = 1, float top = 1, float bottom = 0 );

	/// 이동
	void Translate( const NiPoint3& move );
	void SetTranslate( const NiPoint3& trans );
	void SetTranslate( float x, float y, float z );
	const NiPoint3& GetWorldTranslate() const;

	/// 회전
	void Rotate( const NiPoint3& axis, float angle );
	void SetRotate( const NiMatrix3& rot );
	void Pitch( float angle );
	void Yaw( float angle );
	const NiMatrix3& GetWorldRotate() const;

	/// 방향
	NiPoint3 GetWorldDirection() const;
	NiPoint3 GetWorldUpVector() const;
	NiPoint3 GetWorldRightVector() const;

	/// 속력
	void SetTranslateSpeed( float unitsPerSec );
	void SetRotateSpeed( float anglesPerSec );

	/// 종횡비
	float GetAspectRatio() const;

	/// 마우스 좌표에 해당하는 반직선
	bool GetRayFromWindowPoint( long mouseX, long mouseY, NiPoint3& origin, NiPoint3& dir ) const;

	/// 엔진 카메라로 형변환
	operator NiCamera*() const
	{
		return mNiCamera;
	}

protected:
	/// 처리
	virtual void OnProcess( float time );
	virtual void OnProcessInput( float time ) = 0;

protected:
	/// 화면 크기
	unsigned int mScreenWidth;
	unsigned int mScreenHeight;

	/// 수직 시야각
	float mFovyDegree;

	/// 근거리
	float mNearDistance;

	/// 원거리
	float mFarDistance;

	/// 종횡비
	float mAspectRatio;

	/// 좌, 우, 상, 하
	float mLeft, mRight, mTop, mBottom;

	/// 직교 모드 여부
	bool mOrtho;

	/// 엔진 카메라
	NiCameraPtr mNiCamera;

	/// 카메라 변환용 노드
	NiNodePtr mOrientNode;

	/// 리스너 부착용 노드
	NiNodePtr mListenerNode;

	/// 속력
	float mTranslateSpeed;
	float mRotateSpeed;
};

inline
void cCamera::Update( float time )
{
	mOrientNode->Update( time );
}

inline
void cCamera::SetTranslateSpeed( float unitsPerSec )
{
	mTranslateSpeed = unitsPerSec;
}

inline
void cCamera::SetRotateSpeed( float anglesPerSec )
{
	mRotateSpeed = anglesPerSec;
}

inline
void cCamera::SetOrtho( bool ortho )
{
	mOrtho = ortho;
}

inline
bool cCamera::GetOrtho() const
{
	return mOrtho;
}

inline
const NiPoint3& cCamera::GetWorldTranslate() const
{
	return mNiCamera->GetWorldTranslate();
}

inline
const NiMatrix3& cCamera::GetWorldRotate() const
{
	return mNiCamera->GetWorldRotate();
}

inline
NiPoint3 cCamera::GetWorldDirection() const
{
	return mNiCamera->GetWorldDirection();
}

inline
NiPoint3 cCamera::GetWorldUpVector() const
{
	return mNiCamera->GetWorldUpVector();
}

inline
NiPoint3 cCamera::GetWorldRightVector() const
{
	return mNiCamera->GetWorldRightVector();
}

inline
bool cCamera::GetRayFromWindowPoint( long mouseX, long mouseY, NiPoint3& origin, NiPoint3& dir ) const
{
	return mNiCamera->WindowPointToRay( mouseX, mouseY, origin, dir );
}

inline
float cCamera::GetNearDistance() const
{
	return mNearDistance;
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
