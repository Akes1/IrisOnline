#include "stdafx.h"
#include "NaviMesh.h"

#include "Ray.h"
#include "NaviMeshNode.h"
//#include "CameraManager.h"

cNaviMesh* cNaviMesh::mSingleton = 0;

cNaviMesh::cNaviMesh()
: mCellCount( 0 )
, mLineCount( 0 )
, mMetersPerVertex( 0.0f )
, mUnitsPerMeter( 0 )
, mUnitsPerVertex( 0.0f )
, mUnitsPerLeafNode( 0.0f )
, mSegmentLength( 0.0f )
, mRootNode( 0 )
, mHeights( 0 )
, mLoadArray( 0 )
, mLoadCount( 0 )
, mLoadIndex( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cNaviMesh::~cNaviMesh()
{
	Clear();

	mSingleton = 0;
}

void cNaviMesh::Clear()
{
	delete mRootNode;
	mRootNode = 0;
	delete [] mHeights;
	mHeights = 0;
	
	mCellCount = 0;
	mLineCount = 0;
	mMetersPerVertex = 0.0f;
	mUnitsPerMeter = 0;
	mUnitsPerVertex = 0.0f;
	mUnitsPerLeafNode = 0.0f;
	mSegmentLength = 0.0f;

	mNodeArray.Clear();

	/// 로딩 배열을 제거
	delete [] mLoadArray;
	mLoadArray = 0;
	mLoadCount = 0;
	mLoadIndex = 0;
}

bool cNaviMesh::Pick( NiPoint3* pos, int mouseX, int mouseY )
{
	assert( pos );
	assert( mRootNode );

 	NiPoint3 origin, dir;
// 	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );

	return Pick( pos, cRay(origin, dir) );
}

bool cNaviMesh::Pick( NiPoint3* pos, const cRay& ray )
{
	float dist = NI_INFINITY;
	cNaviMeshNode::mRay = &ray;
	cNaviMeshNode::mMaxDistance = NI_INFINITY;
	cNaviMeshNode::mDistance = &dist;
	cNaviMeshNode::mContact = pos;
	return mRootNode->CollideRay();
}

bool cNaviMesh::Pick( NiPoint3* pos, float* dist, const cRay& ray, float maxDistance )
{
	if( mRootNode )
	{
		cNaviMeshNode::mRay = &ray;
		cNaviMeshNode::mMaxDistance = maxDistance;
		cNaviMeshNode::mDistance = dist;
		cNaviMeshNode::mContact = pos;
		return mRootNode->CollideRay();
	}
	else
	{
		return false;
	}
}

bool cNaviMesh::CalcHeight( float* height, float x, float y ) const
{
	assert( height );

	/// 해당 위치가 지형 안에 들어오는지 검사
	const cBox& box = mRootNode->GetBoundBox();

	if( x < box.GetMin().x || y < box.GetMin().y ||
		x > box.GetMax().x || y > box.GetMax().y )
	{
		return false;
	}

	/// 기울기를 검사
	int xi = (int)(x / 100.0f);
	int yi = (int)(y / 100.0f);
	float x0 = xi * 100.0f;
	float y0 = yi * 100.0f;
	float d = 1.0f;

	if( x != x0 )
	{
		d = (y - y0) / (x - x0);
		assert( d >= 0.0f );
	}

	if( d >= 1.0f )
	{
		/// 왼쪽 위 삼각형
		NiPoint3 p0( x0, y0 + 100.0f, mHeights[(yi+1) * mLineCount + xi] );
		NiPoint3 p1( x0 + 100.0f, y0 + 100.0f, mHeights[(yi+1) * mLineCount + xi+1] );
		NiPoint3 p2( x0, y0, mHeights[yi * mLineCount + xi] );

		*height = p0.z + ((p1.z - p0.z) / (p1.x - p0.x)) * (x - p0.x) + ((p2.z - p0.z) / (p0.y - p2.y)) * (p0.y - y);
	}
	else
	{
		/// 오른쪽 아래 삼각형
		NiPoint3 p0( x0 + 100.0f, y0, mHeights[yi * mLineCount + xi+1] );
		NiPoint3 p1( x0, y0, mHeights[yi * mLineCount + xi] );
		NiPoint3 p2( x0 + 100.0f, y0 + 100.0f, mHeights[mLineCount * (yi+1) + xi+1] );

		*height = p0.z + ((p1.z - p0.z) / (p0.x - p1.x)) * (p0.x - x) + ((p2.z - p0.z) / (p2.y - p0.y)) * (y - p0.y);
	}
	return true;
}

bool cNaviMesh::CheckCellCount( unsigned int cellCount )
{
	if( cellCount % NAVIMESH_DEFAULT_RESOLUTION != 0 )
	{
		assert( 0 );
		return false;
	}

	cNaviMeshLeafNode::mCellCount = NAVIMESH_LEAF_CELL_COUNT;
	cNaviMeshLeafNode::mLineCount = NAVIMESH_LEAF_LINE_COUNT;
	return true;
}

void cNaviMesh::SetLeafNode( unsigned int xi, unsigned int yi, cNaviMeshLeafNode* node )
{
	assert( node );

	unsigned int nodeCount = mCellCount / cNaviMeshLeafNode::mCellCount;
	mNodeArray[yi * nodeCount + xi] = node;
}

const cBox& cNaviMesh::GetBoundBox() const
{
	return mRootNode->GetBoundBox();
}

cNaviMeshLeafNode* cNaviMesh::GetLeafNode( float x, float y ) const
{
	/// 해당 위치가 지형 안에 들어오는지 검사
	const cBox& box = mRootNode->GetBoundBox();

	if( x < box.GetMin().x || y < box.GetMin().y ||
		x > box.GetMax().x || y > box.GetMax().y )
	{
		return 0;
	}

	/// 범위 계산
	int xi = (int)(x / mUnitsPerLeafNode);
	int yi = (int)(y / mUnitsPerLeafNode);
	int nodeCount = (int)(mCellCount / cNaviMeshLeafNode::mCellCount);

	if( xi < 0 )
		xi = 0;
	if( yi < 0 )
		yi = 0;
	if( xi > nodeCount - 1 )
		xi = nodeCount - 1;
	if( yi > nodeCount - 1 )
		yi = nodeCount - 1;

	return (cNaviMeshLeafNode*)mNodeArray[yi * nodeCount + xi];
}
