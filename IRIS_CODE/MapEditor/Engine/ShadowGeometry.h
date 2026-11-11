/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.11.23
* 내    용 : 그림자 기하
* 주의사항 : 
*===========================================================================*/
#pragma once

class cDynamicSceneNode;

/// 그림자 기하
class cShadowGeometry
{
	friend class cShadowGeomCompareForRendering;

public:
	cShadowGeometry();
	~cShadowGeometry();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( NiTexture* tex, float maxRadius, float height = 2.0f, bool zTest = true );

	/// 처리
	void Process( bool needUpdate, const NiPoint3& center, float alpha );

	void Draw( NiRenderer* renderer );

	///
	void SetTexture( NiTexture* tex );
	NiTexture* GetTexture() const;

	void SetRadius( float r );

protected:
	void AddShadowTriangle( NiPoint3* v, NiDataStreamElementLock& kPositionLock, NiDataStreamElementLock& kTexCoordLock );
	void CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, float x, float y, float radius );

	void SetActiveCount( unsigned int count );

protected:
	/// 반지름
	float mMaxRadius;
	float mRadius;

	/// 지표면으로부터의 높이
	float mHeight;

	///
	int mLineCount;

	/// 최대 정점 수
	unsigned int mMaxVerts;
	unsigned int mActiveCount;

	///
	NiPoint3 mCenter;
	float mOOWidth;

	/// 알파값
	float mAlpha;

	/// 그림자 삼각형
	NiMeshPtr mTris;

	NiTexturingPropertyPtr mTexProp;
	NiTexture* mTex;
};

inline
NiTexture* cShadowGeometry::GetTexture() const
{
	return mTex;
}

inline
void cShadowGeometry::SetRadius( float r )
{
	if( r > mMaxRadius )
		mRadius = mMaxRadius;
	else
		mRadius = r;
}

/// 그림자 기하 비교 함수자
class cShadowGeomCompareForRendering
{
public:
	bool operator () ( const cShadowGeometry* left, const cShadowGeometry* right ) const
	{
		return left->mTex > right->mTex;
	};
};
