/* ==========================================================================
* 작 성 자 : 이법석
* 작 성 일 : 2007.01.18
* 내    용 : 이펙트 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "SceneNode.h"

class cDynamicSceneNode;
class cNifAnimationInfo;

/// 장면 노드 인자
class cEffectSceneNodeParam : public cSceneNodeParam
{
public:	
	cEffectSceneNodeParam();

public:
	/// follow 객체
	cDynamicSceneNode* mpFollowSceneNode;
	unsigned int mDummyIdx;

	bool mFollowing;
	bool mLooping;
	bool mIsDistCheck;
};

inline cEffectSceneNodeParam::cEffectSceneNodeParam()
: mpFollowSceneNode(0)
, mDummyIdx((unsigned int)-1)
, mLooping(false)
, mFollowing(false)
, mIsDistCheck(true)
{
}

class cEffectSceneNode : public cSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	inline bool IsRemoved() { return mRemoved; }

	void		Remove();
	void		ParentNodeRemove();

	/// 단독객체를 생성 (발사체류에 사용)
	bool InitSelfNode( const cString& pathName, bool distCheck, NiTransform& trans, bool bLoop );

	void SetLoopFlag( bool flag );
	bool GetLoopFlag();

	inline unsigned int GetDummyIdx() { return mDummyIdx; }

	inline bool IsDamageEffect() { return mIsDamage; }
	inline void EnableDamageEffectFlag() { mIsDamage = true; DisableZBuffer( mSceneNiNode ); }

	virtual void UpdateAlpha(){}

protected:
	cEffectSceneNode( eType type = eEFFECT );
	virtual ~cEffectSceneNode();

	/// 링크는 객체에서 획득해야 한다.
	virtual bool Init( const cEffectSceneNodeParam& param );
	virtual bool InitDamageEffect( const cEffectSceneNodeParam& param, bool IsMiss, bool zFalse );

	bool FindDamageGeom( NiNode* pNode, NiMesh** pArray );

	void DisableZBuffer( NiAVObject* pObj );

	void RemoveSelf();

	virtual void CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty );

protected:
	/// 따라다니기...
	unsigned long mFollowSceneNodeIdx;
	cDynamicSceneNode* mpFollowSceneNode;
	unsigned int mDummyIdx;

	/// 애니 컨트롤러의 정보
	cNifAnimationInfo* mNifAniInfo;

	bool mRemoved;
	bool mFadeOut;

	bool mIsDamage;
	bool mIsDistCheck;
};

