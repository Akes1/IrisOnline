#pragma once

#include "StaticSceneNode.h"

class cDoorSceneNodeParam : public cStaticSceneNodeParam
{
public:
	unsigned long mDoorIdx;
};

class cDoorSceneNode : public cManagedStaticSceneNode
{
	friend class cSceneManager;

public:
	void Remove( unsigned long accumTime );

protected:
	cDoorSceneNode( eType type = eDOOR );
	virtual ~cDoorSceneNode();

	/// √ ±‚»≠
	virtual bool Init( const cDoorSceneNodeParam& param );

	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	inline unsigned long GetDoorIdx() { return mDoorIdx; }

protected:
	unsigned long mDoorIdx;
	bool mRemove;
};
