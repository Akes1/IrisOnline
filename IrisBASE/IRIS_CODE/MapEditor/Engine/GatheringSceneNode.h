#pragma once
#include "staticscenenode.h"


class cGatheringSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cGatheringSceneNodeParam();

public:

	/// 서버 관리용 인덱스
	unsigned long	mGatheringIdx;
	float			mAppandZ;
};

inline cGatheringSceneNodeParam::cGatheringSceneNodeParam()
: mGatheringIdx(UINT_MAX)
, mAppandZ(0.0f)
{
}


class cGatheringSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

public:
	inline float GetAppendZ() { return mAppendZ; }
	inline void SetAppendZ( float z ) { mAppendZ = z; }

	virtual void SetTranslate( const NiPoint3& trans );

protected:
	cGatheringSceneNode( eSceneNodeType type = SCENENODE_GATHERING );
	virtual ~cGatheringSceneNode();

	/// 초기화
	virtual bool Init( cGatheringSceneNodeParam& param );
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

protected:
	unsigned long mGatheringIdx;
	float mAppendZ;
};
