#include "stdafx.h"
#include "AreaSceneNode.h"

#include "Ray.h"
#include "Sphere.h"
#include "CameraManager.h"

cAreaSceneNode::cAreaSceneNode( eSceneNodeType type )
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
, mFogEnabled( false )
, mFogColor( NiColor::WHITE )
, mFogDepth( 0.0f )
, mGroupIndex( 0 )
{
}

cAreaSceneNode::~cAreaSceneNode()
{
}

bool cAreaSceneNode::Save( cFileSaver& saver )
{
	/// 상위 정보(이름, 변환 행렬)를 저장
	if( cSceneNode::Save( saver ) == false )
		return false;

	/// 반지름을 저장
	saver.WriteFloat( mRadius );

	/// 주변광을 저장
	saver.Write( &mSkyAmbient, sizeof(NiColor) );
	saver.WriteFloat( mSkyDimmer );
	saver.Write( &mTerrainAmbient, sizeof(NiColor) );
	saver.WriteFloat( mTerrainDimmer );
	saver.Write( &mStaticObjectAmbient, sizeof(NiColor) );
	saver.WriteFloat( mStaticObjectDimmer );
	saver.Write( &mDynamicObjectAmbient, sizeof(NiColor) );
	saver.WriteFloat( mDynamicObjectDimmer );

	/// 안개를 저장
	saver.WriteUnsignedInt( mFogEnabled );
	saver.Write( &mFogColor, sizeof(NiColor) );
	saver.WriteFloat( mFogDepth );

	/// 사용자 인덱스를 저장
	saver.WriteUnsignedInt( mGroupIndex );
	return true;
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
	mDynamicObjectAmbient = param.mDynamicObjectAmbient;
	mDynamicObjectDimmer = param.mDynamicObjectDimmer;
	mFogEnabled = param.mFogEnabled;
	mFogColor = param.mFogColor;
	mFogDepth = param.mFogDepth;
	mGroupIndex = param.mGroupIndex;


//	char test[256];
//	::sprintf( test, "AreaSceneInit %f", mDynamicObjectDimmer );
//	NiMessageBox(test, "check");

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
	return mBoundSphere.IntersectRay( &mPickPos, &mPickDistance, ray );
}
