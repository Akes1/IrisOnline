#pragma once

#include "DynamicSceneNode.h"

class cMonster;

/// 몬스터 장면 노드 인자
class cMonsterSceneNodeParam : public cDynamicSceneNodeParam
{
};

class cMonsterSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cMonsterSceneNode();

	enum
	{
		/// 링크를 위한 더미들
		eLINK_HEAD = 0,
		eLINK_BODY,
		eLINK_RHAND,
		eLINK_LHAND,
		eLINK_FOOT,

		eLINK_MAX,
	};

	/// 초기화
	virtual bool Init( const cMonsterSceneNodeParam& param );

//	/// 더미에 객체 링크
//	virtual unsigned int LinkObject( unsigned int dummyIdx, const char* fileName );

	/// 객체의 중심좌표를 반환한다.
	virtual const NiPoint3& GetObjectCenter();


protected:
	cMonsterSceneNode( eSceneNodeType type = SCENENODE_MONSTER );

	/// 링크공간을 정의한다.
	virtual bool InitLinkInfo();

	/// 분할 정보를 정의한다. (더미 포함)
	virtual bool InitPartsName();

	/// 기본 애니메이션을 정의한다.
	virtual void InitializeRandomIdle();

	///
	//virtual cObjectNameCard* CreateNameCard( cBaseObject* pObj );

	///
	//virtual void AddDepthPlaneObject() const;
};