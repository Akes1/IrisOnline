#include "stdafx.h"
#include "NaviMesh.h"

#include "NaviMeshNode.h"

static const char* gNaviMeshFileCode = "IrisNaviMesh";

bool cNaviMesh::LoadHeader( unsigned int& count, const cString& pathName )
{
	Clear();

	/// 파일 열기
	assert( mLoader.GetSize() == 0 );
	cFileLoader& loader = mLoader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to open file to load navimesh" );
		return false;
	}

	/// 파일 헤더를 로딩
	cNaviMeshFileHeader header;

	if( loader.Read( &header, sizeof(cNaviMeshFileHeader) ) != sizeof(cNaviMeshFileHeader) )
	{
		assert( 0 && "failed to load navimesh file header" );
		return false;
	}

	if( ::memcmp( header.mCode, gNaviMeshFileCode, 13 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		return false;
	}

	switch( header.mVersion )
	{
	case 2:
		--header.mCellCount;
		break;
	case 3:
		break;
	default:
		assert( 0 && "invalid file version" );
		return false;
	}

	/// 버전을 설정
	cNaviMeshLeafNode::mVersion = header.mVersion;

	/// 그리드 크기를 검사
	if( CheckCellCount( header.mCellCount ) == false )
	{
		assert( 0 && "invalid grid size" );
		return false;
	}

	/// 단위 설정
	mCellCount = header.mCellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = header.mMetersPerVertex;
	mUnitsPerMeter = header.mUnitsPerMeter;
	mUnitsPerVertex = mMetersPerVertex * (float)mUnitsPerMeter;
	mUnitsPerLeafNode = cNaviMeshLeafNode::mCellCount * mUnitsPerVertex;
	mSegmentLength = float(mCellCount) * mUnitsPerVertex;

	/// 높이맵을 로딩
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];

	switch( header.mVersion )
	{
	case 2:
		break;
	case 3:
		loader.Read( mHeights, sizeof(float) * numVerts );
		break;
	}

	/// 리프 노드 배열 생성
	unsigned int leafCount = mCellCount / NAVIMESH_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 루트 노드 생성 및 로딩
	mRootNode = new cNaviMeshBranchNode( 0, 0, mCellCount );

	/// 로딩 준비
	assert( mLoadArray == 0 );
	mLoadArray = new cNaviMeshNode*[MAX_NODES];
	assert( mLoadArray );
	mLoadCount = 0;
	mLoadIndex = 0;

	mRootNode->AddToLoadArray( mLoadArray, mLoadCount );
	count = mLoadCount;

	return true;
}

int cNaviMesh::LoadNodes( unsigned int count )
{
	assert( mLoadArray );
	if( mLoadArray == 0 )
		return -1;

	unsigned int i = 0;
	for( ; i < count; ++i, ++mLoadIndex )
	{
		if( mLoadIndex >= mLoadCount )
		{
			delete [] mLoadArray;
			mLoadArray = 0;

			mLoader.Close();
			return -2;
		}

		if( mLoadArray[mLoadIndex]->Load( mLoader ) == false )
			return -1;
	}
	return i;
}
