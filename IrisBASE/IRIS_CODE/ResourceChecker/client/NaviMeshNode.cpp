#include "stdafx.h"
#include "NaviMeshNode.h"

#include "Ray.h"
#include "NaviMesh.h"

/// 충돌 검사용
const cRay* cNaviMeshNode::mRay = 0;
float cNaviMeshNode::mMaxDistance = 0.0f;
float* cNaviMeshNode::mDistance = 0;
NiPoint3* cNaviMeshNode::mContact = 0;

cNaviMeshNode::cNaviMeshNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
{
	/// 경계 상자를 설정
	float unitsPerVertex = NAVIMESH->GetUnitsPerVertex();
	float minx = (float)(xi) * unitsPerVertex;
	float miny = (float)(yi) * unitsPerVertex;
	float maxx = minx + (float)(cellCount) * unitsPerVertex;
	float maxy = miny + (float)(cellCount) * unitsPerVertex;

	mBoundBox.Set( minx, miny, -0.0001f, maxx, maxy, 0.0001f );

	mCenter.x = (minx + maxx) * 0.5f;
	mCenter.y = (miny + maxy) * 0.5f;
	mCenter.z = 0.0f;

	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;
}

cNaviMeshNode::~cNaviMeshNode()
{
}

cNaviMeshBranchNode::cNaviMeshBranchNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
: cNaviMeshNode( xi, yi, cellCount )
{
	/// 자식 노드를 생성
	unsigned int childCount = cellCount / 2;

	if( childCount > cNaviMeshLeafNode::mCellCount )
	{
		mChild[0] = new cNaviMeshBranchNode( xi, yi, childCount );
		mChild[1] = new cNaviMeshBranchNode( xi + childCount, yi, childCount );
		mChild[2] = new cNaviMeshBranchNode( xi, yi + childCount, childCount );
		mChild[3] = new cNaviMeshBranchNode( xi + childCount, yi + childCount, childCount );
	}
	else if( childCount == cNaviMeshLeafNode::mCellCount )
	{
		mChild[0] = new cNaviMeshLeafNode( xi, yi, childCount );
		mChild[1] = new cNaviMeshLeafNode( xi + childCount, yi, childCount );
		mChild[2] = new cNaviMeshLeafNode( xi, yi + childCount, childCount );
		mChild[3] = new cNaviMeshLeafNode( xi + childCount, yi + childCount, childCount );
	}
	else
	{
		assert( 0 && "invalid cell count" );
	}

	assert( mChild[0] && mChild[1] && mChild[2] && mChild[3] );
}

cNaviMeshBranchNode::~cNaviMeshBranchNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];
}

bool cNaviMeshBranchNode::CollideRay()
{
	bool ret = false;
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( *mRay, scale, mMaxDistance ) )
	{
		/// 자식들에 대해 교차 검사
		ret |= mChild[0]->CollideRay();
		ret |= mChild[1]->CollideRay();
		ret |= mChild[2]->CollideRay();
		ret |= mChild[3]->CollideRay();
	}
	return ret;
}

unsigned int cNaviMeshLeafNode::mVersion = 0;
unsigned int cNaviMeshLeafNode::mCellCount = 0;
unsigned int cNaviMeshLeafNode::mLineCount = 0;

cNaviMeshLeafNode::cNaviMeshLeafNode( unsigned int xi, unsigned int yi, unsigned int cellCount )
: cNaviMeshNode( xi, yi, cellCount )
, mXIndex( xi )
, mYIndex( yi )
{
	assert( cellCount == mCellCount && "not leaf node!" );

	/// 지형의 리프 노드 배열에 설정
	NAVIMESH->SetLeafNode( xi / mCellCount, yi / mCellCount, this );
}

cNaviMeshLeafNode::~cNaviMeshLeafNode()
{
}

bool cNaviMeshLeafNode::CollideRay()
{
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( *mRay, scale, mMaxDistance ) == false )
		return false;

	/// 삼각형들에 대해 교차 검사
	bool ret = false;
	NiPoint3 p0, p1, p2, p3, out;

	for( unsigned int yi = mYIndex, yend = mYIndex + mCellCount; yi < yend; ++yi )
	{
		for( unsigned int xi = mXIndex, xend = mXIndex + mCellCount; xi < xend; ++xi )
		{
			float x0 = xi * 100.0f;
			float y0 = yi * 100.0f;
			p0.x = x0;
			p0.y = y0;
			p0.z = NAVIMESH->GetHeightFast( xi, yi );
			p1.x = x0 + 100.0f;
			p1.y = y0 + 100.0f;
			p1.z = NAVIMESH->GetHeightFast( xi + 1, yi + 1 );
			p2.x = x0;
			p2.y = y0 + 100.0f;
			p2.z = NAVIMESH->GetHeightFast( xi, yi + 1 );
			p3.x = x0 + 100.0f;
			p3.y = y0;
			p3.z = NAVIMESH->GetHeightFast( xi + 1, yi );

			/// 왼쪽 위 삼각형
			if( mRay->IntersectTri( &out, p0, p1, p2 ) == true )
			{
				ret = true;
				float d = (out - mRay->GetOrigin()).Length();
				if( d < *mDistance )
				{
					*mDistance = d;
					*mContact = out;
				}
			}

			/// 오른쪽 아래 삼각형
			if( mRay->IntersectTri( &out, p0, p3, p1 ) == true )
			{
				ret = true;
				float d = (out - mRay->GetOrigin()).Length();
				if( d < *mDistance )
				{
					*mDistance = d;
					*mContact = out;
				}
			}
		}
	}
	return ret;
}
