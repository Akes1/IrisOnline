#include "StdAfx.h"
#include "gatheringscenenode.h"

#include "NaviMesh.h"
#include "NifAnimationInfo.h"

cGatheringSceneNode::cGatheringSceneNode( eSceneNodeType type )
: cStaticSceneNode( type )
, mGatheringIdx(UINT_MAX)
, mAppendZ(0.0f)
{
}

cGatheringSceneNode::~cGatheringSceneNode()
{
}

bool cGatheringSceneNode::Init( cGatheringSceneNodeParam& param )
{
	if( param.mGatheringIdx == UINT_MAX )
	{
		assert(0);
		return false;
	}

	/// 아이템 정보들을 받는다.
	mGatheringIdx = param.mGatheringIdx;
	mAppendZ = param.mAppandZ;

	float height = 0.0f;
	NAVIMESH->CalcHeight( &height, param.mTranslate.x, param.mTranslate.y );
	param.mTranslate.z = height + mAppendZ;

	if( cStaticSceneNode::Init( param ) == false )
		return false;

	return true;
}

void cGatheringSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}

	cSceneNode::Process( deltaTime, accumTime );

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;
}

void cGatheringSceneNode::SetTranslate( const NiPoint3& trans )
{
	NiPoint3 pos = trans;
	pos.z += mAppendZ;
	GetNiObj()->SetTranslate( pos );

	mNeedUpdateTransform = true;
}
