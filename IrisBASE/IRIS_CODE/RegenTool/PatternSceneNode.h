#pragma once
#include "StaticSceneNode.h"

class cPatternSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cPatternSceneNodeParam();

public:
	float mAppandZ;
};

inline cPatternSceneNodeParam::cPatternSceneNodeParam()
:mAppandZ(0)
{
}

class cPatternSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

protected:
	cPatternSceneNode( eSceneNodeType type = SCENENODE_PATTERN );
	virtual ~cPatternSceneNode();

	virtual bool Init( cPatternSceneNodeParam& param );
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

protected:
	float mAppendZ;
};