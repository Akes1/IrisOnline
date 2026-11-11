#pragma once
#include "staticscenenode.h"

class cNameCard;

class cMapPortalSceneNodeParam : public cStaticSceneNodeParam
{
public:
	cMapPortalSceneNodeParam();

public:

	unsigned long	mPosScriptIdx;
};

inline cMapPortalSceneNodeParam::cMapPortalSceneNodeParam()
: mPosScriptIdx(0)
{
}

//////////////////////////////////////////////////////////////////////////
class cMapPortalSceneNode : public cManagedStaticSceneNode
{
	friend class cSceneManager;

public:
	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	inline unsigned long GetPortalIdx() { return mPortalIdx; }
	inline unsigned long GetPosScriptIdx() { return mPosScriptIdx; }

protected:
	cMapPortalSceneNode( eType type = eMAPPORTAL );
	virtual ~cMapPortalSceneNode();

	/// 초기화
	virtual bool Init( cMapPortalSceneNodeParam& param );

	virtual void AddToVisibleArray();

protected:
	unsigned long mPortalIdx;
	unsigned long mPosScriptIdx;

	/// 객체의 정보를 표시하기 위한 객체
	cNameCard* mpNameCard;

	float	mNameHeight;
};

class cUserPortalSceneNodeParam : public cMapPortalSceneNodeParam
{
public:
	cUserPortalSceneNodeParam();

public:
	unsigned long mPortalIdx;
	unsigned long mOwnerIdx;
};

inline cUserPortalSceneNodeParam::cUserPortalSceneNodeParam()
: mOwnerIdx(0)
, mPortalIdx(0)
{
}

//////////////////////////////////////////////////////////////////////////
class cUserPortalSceneNode : public cMapPortalSceneNode
{
	friend class cSceneManager;
public:

	virtual bool Pick( const cRay& ray );

protected:
	virtual bool Init( cUserPortalSceneNodeParam& param );

protected:
	unsigned long mOwnerIdx;
};
