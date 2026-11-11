#include "stdafx.h"
#include "TerrainNode.h"

#ifdef MAP_EDITOR
#include "../MapEditorView.h"
#include "LookAtCamera.h"
#endif

void cTerrainBranchNode::Cull()
{
	unsigned int saveActive = mFrustum->GetActivePlaneState();
	unsigned int side = 0;
	unsigned int c = 0;
	{
		if( mFrustum->IsPlaneActive( NiFrustumPlanes::FAR_PLANE ) )
		{
			side = mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				// The object is not visible since it is on the negative
				// side of the plane.
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				// The object is fully on the positive side of the plane,
				// so there is no need to compare child objects to this plane.
				mFrustum->DisablePlane( NiFrustumPlanes::FAR_PLANE );
				++c;
			}
		}
		if( mFrustum->IsPlaneActive( NiFrustumPlanes::LEFT_PLANE ) )
		{
			side = mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane( NiFrustumPlanes::LEFT_PLANE );
				++c;
			}
		}
		if( mFrustum->IsPlaneActive( NiFrustumPlanes::RIGHT_PLANE ) )
		{
			side = mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane( NiFrustumPlanes::RIGHT_PLANE );
				++c;
			}
		}
		if( mFrustum->IsPlaneActive( NiFrustumPlanes::TOP_PLANE ) )
		{
			side = mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane( NiFrustumPlanes::TOP_PLANE );
				++c;
			}
		}
		if( mFrustum->IsPlaneActive( NiFrustumPlanes::BOTTOM_PLANE ) )
		{
			side = mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane( NiFrustumPlanes::BOTTOM_PLANE );
				++c;
			}
		}
	}

	if( c == 5 )
	{
		/// 완전 포함 -> 자손들을 모두 추가
		AddToVisibleArray();
	}
	else
	{
		/// 교차 -> 컬링 검사를 계속
		mChild[0]->Cull();
		mChild[1]->Cull();
		mChild[2]->Cull();
		mChild[3]->Cull();
	}

	mFrustum->SetActivePlaneState( saveActive );
}

void cTerrainBranchNode::AddToVisibleArray()
{
	mChild[0]->AddToVisibleArray();
	mChild[1]->AddToVisibleArray();
	mChild[2]->AddToVisibleArray();
	mChild[3]->AddToVisibleArray();
}

void cTerrainLeafNode::Cull()
{
	if( mVisible == false )
		return;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::FAR_PLANE) && mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) ) == NiPlane::NEGATIVE_SIDE )
		return;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::LEFT_PLANE) && mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
		return;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::RIGHT_PLANE) && mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
		return;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::TOP_PLANE) && mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) ) == NiPlane::NEGATIVE_SIDE )
		return;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::BOTTOM_PLANE) && mBoundBox.WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) ) == NiPlane::NEGATIVE_SIDE )
		return;

	mVisibleArray->PushBack( this );
	UpdateLODInfo();
}

void cTerrainLeafNode::AddToVisibleArray()
{
	if( mVisible )
	{
		mVisibleArray->PushBack( this );
		UpdateLODInfo();
	}
}

void cTerrainLeafNode::UpdateLODInfo()
{
#ifdef MAP_EDITOR
	cLookAtCamera* cam = VIEW->GetLookAtCamera();
	NiPoint3 heroPos = cam->GetLookAt();
	heroPos.z = 0.0f;
	NiPoint3 nodePos = GetCenter();
	nodePos.z = 0.0f;

	/// LOD 계산
	float sqrDist = (heroPos - nodePos).SqrLength();
	mLod = TERRAIN->CalcLOD( sqrDist );

	/// sibling LOD 계산
	for( unsigned int i=0;i<4;i++ )
	{
		if( mSiblingLeafNode[i] )
		{
			nodePos = mSiblingLeafNode[i]->GetCenter();
			nodePos.z = 0.0f;

			sqrDist = (heroPos - nodePos).SqrLength();
			mSiblingLeafNode[i]->mLod = TERRAIN->CalcLOD( sqrDist );
		}
	}

	/// calc crack index 
	mCrackIndex = UINT_MAX;

	unsigned int cx = UINT_MAX;
	if( mSiblingLeafNode[0] != 0 )
	{
		if( mSiblingLeafNode[0]->mLod < mLod )
			cx = 0;
	}
	if( mSiblingLeafNode[1] != 0 )
	{
		if( mSiblingLeafNode[1]->mLod < mLod )
		{
			NIASSERT(cx == UINT_MAX);
			cx = 1;
		}
	}
	unsigned int cy = UINT_MAX;
	if( mSiblingLeafNode[2] != 0 )
	{
		if( mSiblingLeafNode[2]->mLod < mLod )
			cy = 2;
	}
	if( mSiblingLeafNode[3] != 0 )
	{
		if( mSiblingLeafNode[3]->mLod < mLod )
		{
			NIASSERT(cy == UINT_MAX);
			cy = 3;
		}
	}

	if( cx == UINT_MAX )
	{
		if( cy != UINT_MAX )
			mCrackIndex = cy;
	}
	else
	{
		if( cy == UINT_MAX )
			mCrackIndex = cx;
		else
		mCrackIndex = (cx+1)*2 + cy;
	}
#endif
}
