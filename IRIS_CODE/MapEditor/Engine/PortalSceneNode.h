#pragma once
#include "StaticSceneNode.h"

const int POTALLINE_WIDTH = 150;
const int POTALLINE_HEIGHT = 30;

class cPortalSceneNodeParam : public cStaticSceneNodeParam
{
public:
	float mNameHeight;
};

class cPortalSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

public:
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	void RenderRect();

	void SetNameHeight( float height );
	float GetNameHeight() { return mNameHeight; }

protected:
	cPortalSceneNode( eSceneNodeType type = SCENENODE_PORTAL );

	virtual bool Init( cPortalSceneNodeParam& param );

	void CalcRect();

protected:
	// 포탈명 출력 위치
	NiMeshPtr mNameLine;

	float mNameHeight;
};


//////////////////////////////////////////////////////////////////////////
class cDoorSceneNodeParam : public cStaticSceneNodeParam
{
public:
	float mAppendZ;

	float mRectTopWidth;
	float mRectBottomWidth;
	float mRectLeftHeight;
	float mRectRightHeight;

	float mRectAppendY;
};

class cDoorSceneNode : public cStaticSceneNode
{
	friend class cSceneManager;

public:
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	inline float GetAppendZ() { return mAppendZ; }
	inline void SetAppendZ( float z ) { mAppendZ = z; }

	virtual void SetTranslate( const NiPoint3& trans );

	void GetRectInfo( float& topWidth, float& bottomWidth, float& leftHeight, float& rightHeight );
	void SetRectInfo( float topWidth, float bottomWidth, float leftHeight, float rightHeight );

	float GetRectAppendY() { return mRectAppendY; }
	void SetRectAppendY( float appendY );

	NiPoint3* GetRectPos() { return mRectPos; }

	void RenderRect();

protected:
	cDoorSceneNode( eSceneNodeType type = SCENENODE_DOOR );

	virtual bool Init( cDoorSceneNodeParam& param );

	void CalcRect();

protected:
	float mAppendZ;

	/// 제한 라인
	NiMeshPtr	mLimitRect;

	float mRectTopWidth;
	float mRectBottomWidth;
	float mRectLeftHeight;
	float mRectRightHeight;

	float mRectAppendY;

	NiPoint3 mRectPos[4];
};