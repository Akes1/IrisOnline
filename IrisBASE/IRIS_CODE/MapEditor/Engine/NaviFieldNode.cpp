#include "stdafx.h"
#include "NaviFieldNode.h"

#include "RenderSystem.h"
#include "Ray.h"
#include "NaviField.h"

cNaviFieldNode::cNaviFieldNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: mParent( parent )
{
	/// 경계 상자를 설정
	float unitsPerVertex = NAVIFIELD->GetUnitsPerVertex();
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

cNaviFieldNode::~cNaviFieldNode()
{
}

cNaviFieldBranchNode::cNaviFieldBranchNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: cNaviFieldNode( parent, xi, yi, cellCount )
{
	/// 자식 노드를 생성
	unsigned int childCount = cellCount / 2;

	if( childCount > cNaviFieldLeafNode::mCellCount )
	{
		mChild[0] = new cNaviFieldBranchNode( this, xi, yi, childCount );
		mChild[1] = new cNaviFieldBranchNode( this, xi + childCount, yi, childCount );
		mChild[2] = new cNaviFieldBranchNode( this, xi, yi + childCount, childCount );
		mChild[3] = new cNaviFieldBranchNode( this, xi + childCount, yi + childCount, childCount );
	}
	else if( childCount == cNaviFieldLeafNode::mCellCount )
	{
		mChild[0] = new cNaviFieldLeafNode( this, xi, yi, childCount );
		mChild[1] = new cNaviFieldLeafNode( this, xi + childCount, yi, childCount );
		mChild[2] = new cNaviFieldLeafNode( this, xi, yi + childCount, childCount );
		mChild[3] = new cNaviFieldLeafNode( this, xi + childCount, yi + childCount, childCount );
	}
	else
	{
		assert( 0 && "invalid cell count" );
	}

	assert( mChild[0] && mChild[1] && mChild[2] && mChild[3] );
}

cNaviFieldBranchNode::~cNaviFieldBranchNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];
}

void cNaviFieldBranchNode::UpdateBoundUpward()
{
	/// 경계 상자를 갱신
	cBox box;
	box.AddBox( mChild[0]->GetBoundBox() );
	box.AddBox( mChild[1]->GetBoundBox() );
	box.AddBox( mChild[2]->GetBoundBox() );
	box.AddBox( mChild[3]->GetBoundBox() );

	mBoundBox = box;
	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;

	/// 부모의 경계 상자를 갱신
	if( mParent )
		mParent->UpdateBoundUpward();
}

void cNaviFieldBranchNode::SyncHeight()
{
	/// 자식 노드에 대해 재귀 호출
	mChild[0]->SyncHeight();
	mChild[1]->SyncHeight();
	mChild[2]->SyncHeight();
	mChild[3]->SyncHeight();
}

bool cNaviFieldBranchNode::CollideRay( NiPoint3* contact, float* distance, const cRay& ray )
{
	bool ret = false;
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( ray, scale ) )
	{
		/// 자식들에 대해 교차 검사
		ret |= mChild[0]->CollideRay( contact, distance, ray );
		ret |= mChild[1]->CollideRay( contact, distance, ray );
		ret |= mChild[2]->CollideRay( contact, distance, ray );
		ret |= mChild[3]->CollideRay( contact, distance, ray );
	}
	return ret;
}

unsigned int cNaviFieldLeafNode::mCellCount = 0;
unsigned int cNaviFieldLeafNode::mLineCount = 0;

cNaviFieldLeafNode::cNaviFieldLeafNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount )
: cNaviFieldNode( parent, xi, yi, cellCount )
, mXIndex( xi )
, mYIndex( yi )
{
	assert( cellCount == mCellCount && "not leaf node!" );

	/// 지형의 리프 노드 배열에 설정
	NAVIFIELD->SetLeafNode( xi / mCellCount, yi / mCellCount, this );

	/// 버퍼를 설정
	mBuffer = NAVIFIELD->GetBuffer( xi, yi );
}

cNaviFieldLeafNode::~cNaviFieldLeafNode()
{
}

void cNaviFieldLeafNode::UpdateBoundUpward()
{
	/// 리프 노드 안의 모든 정점으로부터 최소값, 최대값을 계산
	float minz = +NI_INFINITY;
	float maxz = -NI_INFINITY;
	float z = 0.0f;

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			z = NAVIFIELD->GetHeightFast( mXIndex + xi, mYIndex + yi );

			if( z < minz )
				minz = z;
			if( z > maxz )
				maxz = z;
		}
	}

	/// 경계 상자를 갱신
	NiPoint3 min = mBoundBox.GetMin();
	NiPoint3 max = mBoundBox.GetMax();
	min.z = minz;
	max.z = maxz;
	mBoundBox.Set( min, max );
	mCenter = (min + max) * 0.5f;
	mRadius = (max - min).Length() * 0.5f;

	/// 부모의 경계 상자를 갱신
	mParent->UpdateBoundUpward();
}

void cNaviFieldLeafNode::UpdateColors()
{
	assert( mBuffer );

	mBuffer->UpdateColor( NAVIFIELD, mXIndex, mYIndex, mLineCount );
}

void cNaviFieldLeafNode::SyncHeight()
{
	/// 경계 상자를 갱신
	UpdateBoundUpward();
}

bool cNaviFieldLeafNode::CollideRay( NiPoint3* contact, float* distance, const cRay& ray )
{
	float scale = NI_INFINITY;

	if( mBoundBox.IntersectRay( ray, scale ) == false )
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
			p0.z = NAVIFIELD->GetHeightFast( xi, yi );
			p1.x = x0 + 100.0f;
			p1.y = y0 + 100.0f;
			p1.z = NAVIFIELD->GetHeightFast( xi + 1, yi + 1 );
			p2.x = x0;
			p2.y = y0 + 100.0f;
			p2.z = NAVIFIELD->GetHeightFast( xi, yi + 1 );
			p3.x = x0 + 100.0f;
			p3.y = y0;
			p3.z = NAVIFIELD->GetHeightFast( xi + 1, yi );

			/// 왼쪽 위 삼각형
			if( ray.IntersectTri( &out, p0, p1, p2 ) == true )
			{
				ret = true;
				float d = (out - ray.GetOrigin()).Length();
				if( d < *distance )
				{
					*distance = d;
					*contact = out;
				}
			}

			/// 오른쪽 아래 삼각형
			if( ray.IntersectTri( &out, p0, p3, p1 ) == true )
			{
				ret = true;
				float d = (out - ray.GetOrigin()).Length();
				if( d < *distance )
				{
					*distance = d;
					*contact = out;
				}
			}
		}
	}
	return ret;
}

bool cNaviFieldLeafNode::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius )
{
	float unitsPerVertex = NAVIFIELD->GetUnitsPerVertex();
	float x = pos.x;
	float y = pos.y;

	int xb = (int)((x - radius) / unitsPerVertex) - mXIndex;
	int yb = (int)((y - radius) / unitsPerVertex) - mYIndex;
	int xe = (int)((x + radius) / unitsPerVertex) - mXIndex;
	int ye = (int)((y + radius) / unitsPerVertex) - mYIndex;

	--xb;
	--yb;
	++xe;
	++ye;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe > (int)mLineCount )
		xe = mLineCount;
	if( ye > (int)mLineCount )
		ye = mLineCount;

	if( xb >= xe || yb >= ye )
	{
		return false;
	}

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
	return true;
}
