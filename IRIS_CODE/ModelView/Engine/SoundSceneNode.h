/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.23
* 내    용 : 사운드 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"
#include "Sphere.h"

class cDynamicSceneNode;

/// 사운드 장면 노드 인자
class cSoundSceneNodeParam : public cSceneNodeParam
{
public:
	cSoundSceneNodeParam();

public:
	/// 반지름
	float mRadius;

	/// 감쇠 거리
	float mAttenDistance;

	/// 반복 횟수
	unsigned int mLoopCount;

	/// 재생 간격
	float mLoopInterval;

	/// 음량 비율 [0, 1]
	float mVolumeRatio;

	/// ResourceManager 관리 판단
	bool mManaged;

	bool mEffectSound;

	/// follow 객체
	cDynamicSceneNode* mpOwnerSceneNode;
};

inline
cSoundSceneNodeParam::cSoundSceneNodeParam()
: mRadius( 1000.0f )
, mAttenDistance( 100.0f )
, mLoopCount( 0 )
, mLoopInterval( 0.0f )
, mVolumeRatio( 1.0f )
, mpOwnerSceneNode(0)
, mManaged( false )
{
	mQueryType = NiPick::QUERY_FIRST;
	mPickSort = NiPick::SORT_OFF;
	mPickIntersect = NiPick::INTERSECT_BOUND;
	mPickFrontOnly = false;

	mEffectSound = false;
}

/// 사운드 장면 노드
class cSoundSceneNode : public cSceneNode
{
	friend class cSceneManager;
	friend class cSoundEditor;

public:
	/// 처리
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	/// 픽킹
	bool Pick( const cRay& ray );

	/// 중심점
	virtual const NiPoint3& GetCenter();

	/// 반지름
	void SetRadius( float radius );
	virtual float GetRadius();

	/// 감쇠 거리
	void SetAttenDistance( float dist );
	float GetAttenDistance() const;

	/// 제거 함수
	inline bool IsRemoved() { return mRemoved; }

	void Remove();
	void ParentNodeRemove();

	inline void FadeOut() { mFadeOut = true; }

	void ActiveSound( bool active );

	virtual inline NiNode* GetNiNode() { return 0; }
	virtual inline NiAVObject* GetNiObj() { return (NiAVObject*)mSound; }

	virtual void UpdatePickInfo( void* /*extra*/ );

protected:
	cSoundSceneNode( eType type = eSOUND );
	virtual ~cSoundSceneNode();

	/// 초기화
	virtual bool Init( cSoundSceneNodeParam& param );

	/// 반복 횟수
	void SetLoopCount( unsigned int count );
	int GetLoopCount() const;

	/// 재생 간격
	void SetLoopInterval( float interval );
	float GetLoopInterval() const;

	/// 음량
	void SetVolumeRatio( float ratio );
	float GetVolumeRatio() const;

	void RemoveSelf();

protected:
	void SetRemoveFlag( bool val ) { mRemoved = val; }

protected:
	/// 엔진 노드
	NiAudioSourcePtr mSound;

	/// 리스너 진입 여부
	bool mListenerEntered;
	bool mOldListenerEntered;

	/// 반지름
	float mRadius;

	/// 감쇠 거리
	float mAttenDistance;

	/// 반복 회수
	unsigned int mLoopCount;
	unsigned int mTempCount;

	/// 재생 간격
	float mLoopInterval;
	float mTempInterval;

	/// 음량
	float mVolumeRatio;
	float mGain;
	float mTargetGain;

	/// 리스너 구
	cSphere mListenerSphere;

	bool mRemoved;
	bool mFadeOut;

	bool mEffectSound;

	//	unsigned long mOwnerSceneNodeIdx;
	cDynamicSceneNode* mpOwnerSceneNode;

	bool mActiveApp;
};

inline
const NiPoint3& cSoundSceneNode::GetCenter()
{
	return mSound->GetWorldTranslate();
}

inline
void cSoundSceneNode::SetRadius( float radius )
{
	mNeedUpdateTransform = true;
	mRadius = radius;
}

inline
float cSoundSceneNode::GetRadius()
{
	return mRadius;
}

inline
void cSoundSceneNode::SetAttenDistance( float dist )
{
	mAttenDistance = dist;
}

inline
float cSoundSceneNode::GetAttenDistance() const
{
	return mAttenDistance;
}

inline
void cSoundSceneNode::SetLoopCount( unsigned int count )
{
	mLoopCount = count;
}

inline
int cSoundSceneNode::GetLoopCount() const
{
	return mLoopCount;
}

inline
void cSoundSceneNode::SetLoopInterval( float interval )
{
	mLoopInterval = interval;
}

inline
float cSoundSceneNode::GetLoopInterval() const
{
	return mLoopInterval;
}

inline
void cSoundSceneNode::SetVolumeRatio( float ratio )
{
	mVolumeRatio = ratio;
}

inline
float cSoundSceneNode::GetVolumeRatio() const
{
	return mVolumeRatio;
}
