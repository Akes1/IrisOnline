#pragma once

#include "DynamicSceneNode.h"

class cMonster;

/// MONSTER 장면 노드 인자
class cMonsterSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cMonsterSceneNodeParam();

public:
	float		mHeadHeight;

	unsigned long	mMonClassIndex;
};

inline cMonsterSceneNodeParam::cMonsterSceneNodeParam()
: mHeadHeight( 0 )
, mMonClassIndex(0)
{
}

class cTutorialSignboard;
class cMonsterSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cMonsterSceneNode();

	/// 초기화
	virtual bool Init( const cMonsterSceneNodeParam& param );

	/// 더미에 객체 링크
	virtual unsigned int LinkObject( unsigned int dummyIdx, const char* fileName );

	virtual cEffectSceneNode* 		LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow );
	virtual cEffectSceneNode*		LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse );

	virtual	void UpdateTutorialBoard();
	virtual	void ClearTutorialBoard();

	/// 객체의 중심좌표를 반환한다.
	virtual const NiPoint3& GetObjectFoot();
	virtual const NiPoint3& GetObjectCenter();
	virtual const NiPoint3& GetObjectHead();

protected:
	cMonsterSceneNode( eType type = eMONSTER );

	/// 링크공간을 정의한다.
	virtual bool	InitLinkInfo();

	/// 기본 애니메이션을 정의한다.
	virtual void	InitializeRandomIdle();

	///
	virtual cObjectNameCard* CreateNameCard( cBaseObject* pObj );

	virtual void AddDepthPlaneObject();

private:
	// 튜토리얼용
	cTutorialSignboard*		mpTutorialBoard;
};
