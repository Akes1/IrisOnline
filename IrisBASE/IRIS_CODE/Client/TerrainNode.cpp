#include "stdafx.h"
#include "TerrainNode.h"

#include "ObjectManager.h"
#include "Hero.h"

/// 컬링용
NiFrustumPlanes* cTerrainNode::mFrustum = 0;
tArray<void*>* cTerrainNode::mVisibleArray = 0;

/// 충돌 검사용
const cSphere* cTerrainNode::mSphere = 0;
tArray<void*>* cTerrainNode::mPickedArray = 0;

cTerrainNode::cTerrainNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
{
	/// 경계 상자를 설정
	float unitsPerVertex = TERRAIN->GetUnitsPerVertex();
	float minx = float(xi) * unitsPerVertex;
	float miny = float(yi) * unitsPerVertex;
	float maxx = minx + float(cellCount) * unitsPerVertex;
	float maxy = miny + float(cellCount) * unitsPerVertex;

	mBoundBox.Set( minx, miny, -1.0f, maxx, maxy, 1.0f );
	
	mCenter.x = (minx + maxx) * 0.5f;
	mCenter.y = (miny + maxy) * 0.5f;
	mCenter.z = 0.0f;

	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;
}

cTerrainNode::~cTerrainNode()
{
}

cTerrainBranchNode::cTerrainBranchNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
: cTerrainNode( xi, yi, cellCount )
{
	/// 자식 노드를 생성
	unsigned int childCount = cellCount / 2;

	if( childCount > TERRAIN_LEAF_CELL_COUNT )
	{
		mChild[0] = new cTerrainBranchNode( xi, yi, childCount );
		mChild[1] = new cTerrainBranchNode( xi + childCount, yi, childCount );
		mChild[2] = new cTerrainBranchNode( xi, yi + childCount, childCount );
		mChild[3] = new cTerrainBranchNode( xi + childCount, yi + childCount, childCount );
	}
	else if( childCount == TERRAIN_LEAF_CELL_COUNT )
	{
		mChild[0] = new cTerrainLeafNode( xi, yi, childCount );
		mChild[1] = new cTerrainLeafNode( xi + childCount, yi, childCount );
		mChild[2] = new cTerrainLeafNode( xi, yi + childCount, childCount );
		mChild[3] = new cTerrainLeafNode( xi + childCount, yi + childCount, childCount );
	}
	else
	{
		assert( 0 && "invalid cell count" );
	}

	assert( mChild[0] && mChild[1] && mChild[2] && mChild[3] );
}

cTerrainBranchNode::~cTerrainBranchNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];
}

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

bool cTerrainBranchNode::CollideSphere()
{
	bool ret = false;

	if( mBoundBox.IntersectSphere( *mSphere ) )
	{
		/// 자식들에 대해 교차 검사
		ret |= mChild[0]->CollideSphere();
		ret |= mChild[1]->CollideSphere();
		ret |= mChild[2]->CollideSphere();
		ret |= mChild[3]->CollideSphere();
	}
	return ret;
}

unsigned int cTerrainLeafNode::mVersion = 0;
//unsigned int cTerrainLeafNode::mCellCount[TERRAIN_LOD_COUNT] = { 0 };
//unsigned int cTerrainLeafNode::mLineCount[TERRAIN_LOD_COUNT] = { 0 };

cTerrainLeafNode::cTerrainLeafNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
: cTerrainNode( xi, yi, cellCount )
, mXIndex( xi )
, mYIndex( yi )
, mVisible( true )
, mTexture0( 0 )
, mTexture1( 0 )
, mTexture2( 0 )
, mTextureValue0( 0 )
, mTextureValue1( 0 )
, mLight( 0 )
, mIndexByLight( 0xFFFFFFFF )
, mLod( UINT_MAX )
, mCrackIndex( UINT_MAX )
{
	assert( cellCount == TERRAIN_LEAF_CELL_COUNT && "not leaf node!" );

	for( unsigned int i=0;i<4;i++)
		mSiblingLeafNode[i] = 0;

	/// 지형의 리프 노드 배열에 설정
	TERRAIN->SetLeafNode( xi / TERRAIN_LEAF_CELL_COUNT, yi / TERRAIN_LEAF_CELL_COUNT, this );

	/// 버퍼를 설정
	mBuffer = TERRAIN->GetBuffer( xi, yi );
}

cTerrainLeafNode::~cTerrainLeafNode()
{
	Clear();

	for( unsigned int i=0;i<4;i++)
		mSiblingLeafNode[i] = 0;

	/// 조명에서 탈착
	if( mLight )
	{
//		mLight->Detach( this );
		mLight = 0;
	}
}

void cTerrainLeafNode::Clear()
{
	/// 텍스처를 해제
	mTextureValue0 = 0;
	mTextureValue1 = 0;

	if( mTexture0 )
	{
		mTexture0->Drop();
		mTexture0 = 0;
	}
	if( mTexture1 )
	{
		mTexture1->Drop();
		mTexture1 = 0;
	}
	if( mTexture2 )
	{
		mTexture2->Drop();
		mTexture2 = 0;
	}
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

void cTerrainLeafNode::SetTextures( cTerrainTexture* tex0, cTerrainTexture* tex1, cTerrainTexture* tex2 )
{
	if( tex0 == 0 || tex1 == 0 || tex2 == 0 )
	{
		assert( 0 );
		return;
	}

	if( tex0 != mTexture0 )
	{
		tex0->Grab();

		if( mTexture0 )
			mTexture0->Drop();
		mTexture0 = tex0;
	}
	if( tex1 != mTexture1 )
	{
		tex1->Grab();

		if( mTexture1 )
			mTexture1->Drop();
		mTexture1 = tex1;
	}
	if( tex2 != mTexture2 )
	{
		tex2->Grab();

		if( mTexture2 )
			mTexture2->Drop();
		mTexture2 = tex2;
	}

	CalcTextureValue();
}

void cTerrainLeafNode::CalcTextureValue()
{
	assert( mTexture0 );
	assert( mTexture1 );
	assert( mTexture2 );

	unsigned int val[3] =
	{
		0, 0, 0
	};
	unsigned int tex[3] =
	{
		mTexture0->mIndexByTerrain, mTexture1->mIndexByTerrain, mTexture2->mIndexByTerrain
	};

	for( int i = 0; i < 3; ++i )
	{
		for( int j = 2; j >= 0; --j )
		{
			if( tex[i] > val[j] )
			{
				for( int k = 1; k <= j; ++k )
				{
					val[k-1] = val[k];
				}

				val[j] = tex[i];
				break;
			}
		}
	}

	mTextureValue0 = val[0] * 10000 + val[1] * 100 + val[2];
	mTextureValue1 = tex[0] * 10000 + tex[1] * 100 + tex[2];
}

bool cTerrainLeafNode::CollideSphere()
{
	if( mBoundBox.IntersectSphere( *mSphere ) )
	{
		mPickedArray->PushBack( this );
		return true;
	}
	return false;
}

void cTerrainLeafNode::SetLight( cLightSceneNode* light, unsigned int indexByLight )
{
	if( mLight == light )
		return;

//	if( mLight )
//		mLight->Detach( this );

	mLight = light;
	mIndexByLight = indexByLight;
}

void cTerrainLeafNode::UpdateLODInfo()
{
	NiPoint3 heroPos = HERO->GetPos();
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
}
