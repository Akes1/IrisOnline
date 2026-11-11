/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.03.13
* 내    용 : 궤적 기하
* 주의사항 : 
*===========================================================================*/
#pragma once

#define MIN_ADDPOINT_CNT	8

/// 리본 기하
class cRibbonGeometry : public NiMesh
{
public:
	cRibbonGeometry();
	virtual ~cRibbonGeometry();

	/// 지움
	void Clear();

	/// 초기화
	bool Init( unsigned long scaleAccum, NiTexture* texture, unsigned int maxVerts, unsigned long pushInterval );

	/// 처리
	void Process( unsigned long deltaTime );

	/// 렌더링
	virtual void RenderImmediate( NiRenderer* renderer );

	///
	inline void SetPushEnabled( bool enabled ) { mPushEnabled = enabled; }

	void PushTrailPoint( unsigned long pushTime, const NiPoint3& v0, const NiPoint3& v1 );

	bool IsDelete();

	inline unsigned int GetActiveVerts() { return mActiveVerts; }

protected:
	void SetActiveCount( unsigned int count );
	void UpdateBound();

	const NiDataStream* GetVertexDataStream();

protected:
	struct sPointInfo
	{
		NiPoint3		mPoint[2];
		float			mAlpha;

		NiPoint3		mDir21;
		float			mDirLen;

		void UpdateAlpha( float da )
		{
			if( mAlpha < da )
				mAlpha = 0.0f;
			else
				mAlpha -= da;
		}
	};

	/// 정점 정보 보관
	NiTPointerList<sPointInfo*> mTrailPointList;

	bool mPushEnabled;
	bool mUpdatePoint;

	unsigned int mMaxVerts;
	unsigned int mActiveVerts;

	unsigned long mPushChekTime;
	unsigned long mPushInterval;

	/// 최소 5개 이상의 포인트를 입력받도록 보정
	unsigned int mCheckAddPoint;
};

inline
bool cRibbonGeometry::IsDelete()
{
	if( mCheckAddPoint > MIN_ADDPOINT_CNT && mTrailPointList.IsEmpty() )
		return true;

	return false;
}