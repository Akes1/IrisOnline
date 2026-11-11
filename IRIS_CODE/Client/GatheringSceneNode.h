#pragma once
#include "staticscenenode.h"

class cNameCard;
class cShadowGeometry;

class cGatheringSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cGatheringSceneNodeParam();

public:
	/// 서버 관리용 인덱스
	unsigned long mObjectIdx;
	unsigned long mClassIdx;

	// pvp 사용 팀정보
	unsigned char mTeamType;
};

inline cGatheringSceneNodeParam::cGatheringSceneNodeParam()
: mObjectIdx(0)
, mClassIdx(0)
, mTeamType(0)
{
}

class cEffectSceneNode;

class cGatheringSceneNode : public cManagedStaticSceneNode
{
	friend class cSceneManager;

public:
	inline unsigned long GetGatheringIndex() { return mObjectIdx; }

	inline unsigned long GetGatheringClassIdx() { return mClassIdx; }

	inline bool IsRemove() { return mRemoveFlag; }

	bool IsEnableGathering();

	void Remove();

	virtual bool OnVisible();

	virtual bool Pick( const cRay& ray );

protected:
	cGatheringSceneNode( eType type = eGATHERING );
	virtual ~cGatheringSceneNode();

	/// 초기화
	virtual bool Init( cGatheringSceneNodeParam& param );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	virtual void AddToVisibleArray();

	void AddEffect();
	void ClearEffect();

protected:
	unsigned long mObjectIdx;

	unsigned long mClassIdx;

	//
	unsigned char mTeamType;

	long mQuestIdx;

	bool mRemoveFlag;

	/// 객체의 정보를 표시하기 위한 객체
	cNameCard* mpNameCard;
	float mNameHeight;

	unsigned long		mEffectCheckTimer;

	unsigned long		mEffectIdxByManager;
	cEffectSceneNode*	mEnableGatheringEffect;

	cShadowGeometry* mShadowGeom;
	bool mNeedUpdateShadow;

};
