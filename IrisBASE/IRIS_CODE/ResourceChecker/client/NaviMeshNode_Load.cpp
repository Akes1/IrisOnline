#include "stdafx.h"
#include "NaviMeshNode.h"

#include "NaviMesh.h"

void cNaviMeshBranchNode::AddToLoadArray( cNaviMeshNode** loadArray, unsigned int& count )
{
	if( count >= cNaviMesh::MAX_NODES )
	{
		assert( 0 );
		return;
	}
	else
	{
		loadArray[count] = this;
		++count;
	}

	mChild[0]->AddToLoadArray( loadArray, count );
	mChild[1]->AddToLoadArray( loadArray, count );
	mChild[2]->AddToLoadArray( loadArray, count );
	mChild[3]->AddToLoadArray( loadArray, count );
}

bool cNaviMeshBranchNode::Load( cFileLoader& loader )
{
	/// 경계 상자를 로딩
	if( loader.Read( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;
	return true;
}

void cNaviMeshLeafNode::AddToLoadArray( cNaviMeshNode** loadArray, unsigned int& count )
{
	if( count >= cNaviMesh::MAX_NODES )
	{
		assert( 0 );
		return;
	}
	else
	{
		loadArray[count] = this;
		++count;
	}
}

bool cNaviMeshLeafNode::Load( cFileLoader& loader )
{
	/// 경계 상자를 로딩
	if( loader.Read( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;

	/// 버전별 로딩
	switch( mVersion )
	{
	case 2:
		{
			/// 높이 배열을 로딩
			float h;
			for( unsigned int yi = mYIndex, yend = mYIndex + mLineCount; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + mLineCount; xi < xend; ++xi )
				{
					loader.ReadFloat( &h );
					NAVIMESH->SetHeight( xi, yi, h );
				}
			}
			break;
		}
	case 3:
		{
			break;
		}
	}
	return true;
}
