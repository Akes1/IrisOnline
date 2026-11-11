#include "stdafx.h"
#include "NaviMeshNode.h"

#include "NaviMesh.h"
#include "RenderSystem.h"

bool cNaviMeshBranchNode::Load( cFileLoader& loader )
{
	/// 경계 상자를 로딩
	if( loader.Read( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;

	/// 자식 노드들을 로딩
	if( mChild[0]->Load( loader ) == false )
		return false;
	if( mChild[1]->Load( loader ) == false )
		return false;
	if( mChild[2]->Load( loader ) == false )
		return false;
	if( mChild[3]->Load( loader ) == false )
		return false;
	return true;
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

			/// 버퍼를 갱신
			mBuffer->UpdatePosCoord( NAVIMESH, mXIndex, mYIndex, mLineCount );
			break;
		}
	case 3:
		{
			break;
		}
	}
	return true;
}
