/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.23
* 내    용 : 영역 장면 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "SceneNode.h"

/// 영역 장면 노드 인자
class cAreaSceneNodeParam : public cSceneNodeParam
{
public:
	cAreaSceneNodeParam();

public:
	/// 반지름
	float mRadius;

	/// 주변광
	NiColor mSkyAmbient;
	float mSkyDimmer;
	NiColor mTerrainAmbient;
	float mTerrainDimmer;
	NiColor mStaticObjectAmbient;
	float mStaticObjectDimmer;
	NiColor mDynamicObjectAmbient;
	float mDynamicObjectDimmer;

	/// 안개
	bool mFogEnabled;
	NiColor mFogColor;
	float mFogDepth;

	/// 그룹 인덱스
	unsigned int mGroupIndex;
};

inline
cAreaSceneNodeParam::cAreaSceneNodeParam()
: mRadius( 1000.0f )
, mSkyAmbient( NiColor::WHITE )
, mSkyDimmer( 1.0f )
, mTerrainAmbient( NiColor::WHITE )
, mTerrainDimmer( 1.0f )
, mStaticObjectAmbient( NiColor::WHITE )
, mStaticObjectDimmer( 1.0f )
, mDynamicObjectAmbient( NiColor::WHITE )
, mDynamicObjectDimmer( 1.0f )
, mFogEnabled( false )
, mFogColor( NiColor::WHITE )
, mFogDepth( 0.0f )
, mGroupIndex( 0 )
{
	mQueryType = NiPick::QUERY_FIRST;
	mPickSort = NiPick::SORT_OFF;
	mPickIntersect = NiPick::INTERSECT_BOUND;
	mPickFrontOnly = true;
}

/// 영역 장면 노드
class cAreaSceneNode : public cSceneNode
{
	friend class cSceneManager;
	friend class cObjectEditor;
	friend class cAreaEditor;

public:
	/// 픽킹
	bool Pick( const cRay& ray );

	/// 중심점
	const NiPoint3& GetCenter();

	/// 반지름
	void SetRadius( float radius );
	float GetRadius();

	/// 주변광
	void SetSkyAmbient( const NiColor& color );
	void SetSkyDimmer( float dimmer );
	void SetTerrainAmbient( const NiColor& color );
	void SetTerrainDimmer( float dimmer );
	void SetStaticObjectAmbient( const NiColor& color );
	void SetStaticObjectDimmer( float dimmer );
	void SetDynamicObjectAmbient( const NiColor& color );
	void SetDynamicObjectDimmer( float dimmer );
	const NiColor& GetSkyAmbient() const;
	float GetSkyDimmer() const;
	const NiColor& GetTerrainAmbient() const;
	float GetTerrainDimmer() const;
	const NiColor& GetStaticObjectAmbient() const;
	float GetStaticObjectDimmer() const;
	const NiColor& GetDynamicObjectAmbient() const;
	float GetDynamicObjectDimmer() const;

	/// 안개
	void SetFogEnabled( bool enabled );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );
	bool IsFogEnabled() const;
	const NiColor& GetFogColor() const;
	float GetFogDepth() const;

	/// 그룹 인덱스
	void SetGroupIndex( unsigned int index );
	unsigned int GetGroupIndex() const;

protected:
	cAreaSceneNode( eSceneNodeType type = SCENENODE_AREA );
	virtual ~cAreaSceneNode();

	/// 저장
	virtual bool Save( cFileSaver& saver );

	/// 초기화
	virtual bool Init( const cAreaSceneNodeParam& param );

protected:
	/// 반지름
	float mRadius;

	/// 주변광
	NiColor mSkyAmbient;
	float mSkyDimmer;
	NiColor mTerrainAmbient;
	float mTerrainDimmer;
	NiColor mStaticObjectAmbient;
	float mStaticObjectDimmer;
	NiColor mDynamicObjectAmbient;
	float mDynamicObjectDimmer;

	/// 안개
	bool mFogEnabled;
	NiColor mFogColor;
	float mFogDepth;

	/// 그룹 인덱스
	unsigned int mGroupIndex;
};

inline
const NiPoint3& cAreaSceneNode::GetCenter()
{
	return mSceneNiNode->GetWorldTranslate();
}

inline
void cAreaSceneNode::SetRadius( float radius )
{
	mNeedUpdateTransform = true;
	mRadius = radius;
}

inline
float cAreaSceneNode::GetRadius()
{
	return mRadius;
}

inline
void cAreaSceneNode::SetSkyAmbient( const NiColor& color )
{
	mSkyAmbient = color;
}

inline
void cAreaSceneNode::SetSkyDimmer( float dimmer )
{
	mSkyDimmer = dimmer;
}

inline
const NiColor& cAreaSceneNode::GetSkyAmbient() const
{
	return mSkyAmbient;
}

inline
float cAreaSceneNode::GetSkyDimmer() const
{
	return mSkyDimmer;
}

inline
void cAreaSceneNode::SetTerrainAmbient( const NiColor& color )
{
	mTerrainAmbient = color;
}

inline
void cAreaSceneNode::SetTerrainDimmer( float dimmer )
{
	mTerrainDimmer = dimmer;
}

inline
const NiColor& cAreaSceneNode::GetTerrainAmbient() const
{
	return mTerrainAmbient;
}

inline
float cAreaSceneNode::GetTerrainDimmer() const
{
	return mTerrainDimmer;
}

inline
void cAreaSceneNode::SetStaticObjectAmbient( const NiColor& color )
{
	mStaticObjectAmbient = color;
}

inline
void cAreaSceneNode::SetStaticObjectDimmer( float dimmer )
{
	mStaticObjectDimmer = dimmer;
}

inline
const NiColor& cAreaSceneNode::GetStaticObjectAmbient() const
{
	return mStaticObjectAmbient;
}

inline
float cAreaSceneNode::GetStaticObjectDimmer() const
{
	return mStaticObjectDimmer;
}

inline
void cAreaSceneNode::SetDynamicObjectAmbient( const NiColor& color )
{
	mDynamicObjectAmbient = color;
}

inline
void cAreaSceneNode::SetDynamicObjectDimmer( float dimmer )
{
	mDynamicObjectDimmer = dimmer;
}

inline
const NiColor& cAreaSceneNode::GetDynamicObjectAmbient() const
{
	return mDynamicObjectAmbient;
}

inline
float cAreaSceneNode::GetDynamicObjectDimmer() const
{
	return mDynamicObjectDimmer;
}

inline
void cAreaSceneNode::SetFogEnabled( bool enabled )
{
	mFogEnabled = enabled;
}

inline
void cAreaSceneNode::SetFogColor( const NiColor& color )
{
	mFogColor = color;
}

inline
void cAreaSceneNode::SetFogDepth( float depth )
{
	mFogDepth = depth;
}

inline
bool cAreaSceneNode::IsFogEnabled() const
{
	return mFogEnabled;
}

inline
const NiColor& cAreaSceneNode::GetFogColor() const
{
	return mFogColor;
}

inline
float cAreaSceneNode::GetFogDepth() const
{
	return mFogDepth;
}

inline
void cAreaSceneNode::SetGroupIndex( unsigned int index )
{
	mGroupIndex = index;
}

inline
unsigned int cAreaSceneNode::GetGroupIndex() const
{
	return mGroupIndex;
}
