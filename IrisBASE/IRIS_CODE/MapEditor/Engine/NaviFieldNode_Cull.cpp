#include "stdafx.h"
#include "NaviFieldNode.h"

void cNaviFieldBranchNode::Cull( tArray<cNaviFieldLeafNode*>* visibleArray, NiFrustumPlanes& frustum )
{
	assert( visibleArray );

	unsigned int saveActive = frustum.GetActivePlaneState();
	unsigned int i = 0;

	for( ; i < NiFrustumPlanes::MAX_PLANES; ++i )
	{
		if( frustum.IsPlaneActive(i) )
		{
			unsigned int side = mBoundBox.WhichSide( frustum.GetPlane(i) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				// The object is not visible since it is on the negative
				// side of the plane.
				frustum.SetActivePlaneState(saveActive);
				return;
			}

			if( side == NiPlane::POSITIVE_SIDE )
			{
				// The object is fully on the positive side of the plane,
				// so there is no need to compare child objects to this plane.
				frustum.DisablePlane(i);
			}
		}
	}

	if( frustum.IsAnyPlaneActive() == false )
	{
		/// 완전 포함 -> 자손들을 모두 추가
		AddToVisibleArray( visibleArray );
	}
	else if( i == NiFrustumPlanes::MAX_PLANES )
	{
		/// 교차 -> 컬링 검사를 계속
		mChild[0]->Cull( visibleArray, frustum );
		mChild[1]->Cull( visibleArray, frustum );
		mChild[2]->Cull( visibleArray, frustum );
		mChild[3]->Cull( visibleArray, frustum );
	}

	frustum.SetActivePlaneState(saveActive);
}

void cNaviFieldBranchNode::AddToVisibleArray( tArray<cNaviFieldLeafNode*>* visibleArray )
{
	assert( visibleArray );

	mChild[0]->AddToVisibleArray( visibleArray );
	mChild[1]->AddToVisibleArray( visibleArray );
	mChild[2]->AddToVisibleArray( visibleArray );
	mChild[3]->AddToVisibleArray( visibleArray );
}

void cNaviFieldLeafNode::Cull( tArray<cNaviFieldLeafNode*>* visibleArray, NiFrustumPlanes& frustum )
{
	assert( visibleArray );

	for( unsigned int i = 0; i < NiFrustumPlanes::MAX_PLANES; ++i )
	{
		if( frustum.IsPlaneActive(i) )
		{
			unsigned int side = mBoundBox.WhichSide( frustum.GetPlane(i) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				return;
			}
		}
	}

	visibleArray->PushBack( this );
}

void cNaviFieldLeafNode::AddToVisibleArray( tArray<cNaviFieldLeafNode*>* visibleArray )
{
	assert( visibleArray );

	visibleArray->PushBack( this );
}
