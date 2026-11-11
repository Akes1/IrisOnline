#pragma once

#include "DynamicSceneNode.h"

/// NPC 장면 노드 인자
class cNpcSceneNodeParam : public cDynamicSceneNodeParam
{
public:
	cNpcSceneNodeParam();
};

inline cNpcSceneNodeParam::cNpcSceneNodeParam()
{
}

class cNpcSceneNode : public cDynamicSceneNode
{
	friend class cSceneManager;

public:
	virtual ~cNpcSceneNode();

	enum
	{
		/// 링크를 위한 더미들
		eLINK_HEAD = 0,
		eLINK_BODY,
		eLINK_FOOT,

		eLINK_MAX,
	};

	/// 초기화
	virtual bool Init( const cNpcSceneNodeParam& param );


	virtual const NiPoint3& GetObjectCenter() { return NiPoint3::ZERO; }

protected:
	cNpcSceneNode( eSceneNodeType type = SCENENODE_NPC );

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
