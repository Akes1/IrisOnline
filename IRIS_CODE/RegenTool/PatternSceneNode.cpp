#include "StdAfx.h"
#include "PatternSceneNode.h"
#include "NaviMesh.h"

cPatternSceneNode::cPatternSceneNode( eSceneNodeType type )
: cStaticSceneNode( type )
, mAppendZ(0)
{

}

cPatternSceneNode::~cPatternSceneNode()
{
}

bool cPatternSceneNode::Init( cPatternSceneNodeParam& param )
{
	mAppendZ = param.mAppandZ;

	float height = 0.0f;
	NAVIMESH->CalcHeight( &height, param.mTranslate.x, param.mTranslate.y );
	param.mTranslate.z = height + mAppendZ;

	return cStaticSceneNode::Init( param );
}

void cPatternSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// 반복 동작을 가진 경우 엔진 노드를 처리
	/*if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}*/

	cSceneNode::Process( deltaTime, accumTime );

	mCheckFrustum = ( mAlpha < 0.000001f ) ? false : true;
}	