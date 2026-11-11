/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.23
* 내    용 : 사운드 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"

/// 사운드 장면 노드 인자
class cSoundSceneNodeParam : public cSceneNodeParam
{
public:
	cSoundSceneNodeParam();

public:
	/// 반지름
	float mOuterRadius;

	/// 감쇠 거리
	float mAttenDistance;

	/// 반복 횟수
	unsigned int mLoopCount;

	/// 재생 간격
	float mLoopInterval;

	/// 음량 비율 [0, 1]
	float mVolumeRatio;
};

inline
cSoundSceneNodeParam::cSoundSceneNodeParam()
: mOuterRadius( 1000.0f )
, mAttenDistance( 100.0f )
, mLoopCount( 0 )
, mLoopInterval( 0.0f )
, mVolumeRatio( 1.0f )
{
	mQueryType = NiPick::QUERY_FIRST;
	mPickSort = NiPick::SORT_OFF;
	mPickIntersect = NiPick::INTERSECT_BOUND;
	mPickFrontOnly = true;
}

#ifdef MAP_EDITOR
#include "Sphere.h"

/// 사운드 장면 노드
class cSoundSceneNode : public cSceneNode
{
	friend class cSceneManager;
	friend class cSoundEditor;

	enum eListenerStatus
	{
		LISTENER_IN,
		LISTENER_OUT
	};

public:
	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 픽킹
	bool Pick( const cRay& ray );

	/// 중심점
	const NiPoint3& GetCenter();

	/// 반지름
	void SetRadius( float radius );
	float GetRadius();

	/// 감쇠 거리
	void SetAttenDistance( float dist );
	float GetAttenDistance() const;

	///
	void SetListenerStatus( eListenerStatus status, const cSphere& sphere );

	virtual inline NiNode* GetNiNode() { return 0; }
	virtual inline NiAVObject* GetNiObj() { return (NiAVObject*)mSound; }

protected:
	cSoundSceneNode( eSceneNodeType type = SCENENODE_SOUND );
	virtual ~cSoundSceneNode();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 초기화
	virtual bool Init( const cSoundSceneNodeParam& param );

	/// 반복 횟수
	void SetLoopCount( unsigned int count );
	int GetLoopCount() const;

	/// 재생 간격
	void SetLoopInterval( float interval );
	float GetLoopInterval() const;

	/// 음량
	void SetVolumeRatio( float ratio );
	float GetVolumeRatio() const;

protected:
	/// 엔진 노드
	NiAudioSourcePtr mSound;

	/// 리스너 상태
	eListenerStatus mListenerStatus;
	eListenerStatus mOldListenerStatus;

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
#endif /// MAP_EDITOR
