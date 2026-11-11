#pragma once

#include "DynamicSceneNode.h"

class cNpc;

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

	/// 초기화
	virtual bool Init( const cNpcSceneNodeParam& param );

	virtual const NiPoint3& GetObjectCenter();
	virtual const NiPoint3& GetObjectFoot();
	virtual const NiPoint3& GetObjectHead();

protected:
	cNpcSceneNode( eType type = eNPC );

	/// 링크공간을 정의한다.
	virtual bool	InitLinkInfo();

	/// 기본 애니메이션을 정의한다.
	virtual void	InitializeRandomIdle();
};