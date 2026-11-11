#include "stdafx.h"
#include "AreaSceneNode.h"

#include "Ray.h"
#include "Sphere.h"

cAreaSceneNode::cAreaSceneNode( eType type )
: cSceneNode( type )
, mRadius( 0.0f )
, mSkyAmbient( NiColor::WHITE )
, mSkyDimmer( 1.0f )
, mTerrainAmbient( NiColor::WHITE )
, mTerrainDimmer( 1.0f )
, mStaticObjectAmbient( NiColor::WHITE )
, mStaticObjectDimmer( 1.0f )
, mDynamicObjectAmbient( NiColor::WHITE )
, mDynamicObjectDimmer( 1.0f )
, mFogColor( NiColor::WHITE )
, mFogDepth( 0.0f )
, mGroupIndex( 0 )
{
}

cAreaSceneNode::~cAreaSceneNode()
{
}

bool cAreaSceneNode::Init( const cAreaSceneNodeParam& param )
{
	/// 엔진 노드를 생성
	mSceneNiNode = NiNew NiNode;
	mRadius = param.mRadius;
	mSkyAmbient = param.mSkyAmbient;
	mSkyDimmer = param.mSkyDimmer;
	mTerrainAmbient = param.mTerrainAmbient;
	mTerrainDimmer = param.mTerrainDimmer;
	mStaticObjectAmbient = param.mStaticObjectAmbient;
	mStaticObjectDimmer = param.mStaticObjectDimmer;
	mDynamicObjectAmbient = param.mDynamicObjectAmbient;//param.mStaticObjectAmbient;
	mDynamicObjectDimmer = param.mDynamicObjectDimmer;//param.mStaticObjectDimmer;
	mFogColor = param.mFogColor;
	mFogDepth = param.mFogDepth;
	mGroupIndex = param.mGroupIndex;

	/// 상위 초기화
	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// 갱신 선택 사항을 설정
	mSceneNiNode->SetSelectiveUpdate( true );
	mSceneNiNode->SetSelectiveUpdateTransforms( true );
	mSceneNiNode->SetSelectiveUpdatePropertyControllers( false );
	mSceneNiNode->SetSelectiveUpdateRigid( false );
	return true;
}

bool cAreaSceneNode::Pick( const cRay& ray )
{
	return mBoundSphere.IntersectRay( ray );
}
