#include "stdafx.h"
#include "NaviMesh.h"

#include "RenderSystem.h"
#include "NaviMeshNode.h"

static const char* gNaviMeshFileCode = "IrisNaviMesh";

int cNaviMesh::Load( const cString& pathName )
{
	Clear();

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return NAVIMESH_LOAD_ERROR_OPEN;
	}

	/// 파일 헤더를 로딩
	cNaviMeshFileHeader header;

	if( loader.Read( &header, sizeof(cNaviMeshFileHeader) ) != sizeof(cNaviMeshFileHeader) )
	{
		return NAVIMESH_LOAD_ERROR_FILE_HEADER;
	}

	if( ::memcmp( header.mCode, gNaviMeshFileCode, 13 ) != 0 )
	{
		return NAVIMESH_LOAD_ERROR_FILE_TYPE;
	}

	switch( header.mVersion )
	{
	case 2:
		--header.mCellCount;
		break;
	case 3:
		break;
	default:
		return NAVIMESH_LOAD_ERROR_FILE_VERSION;
	}

	/// 버전을 설정
	cNaviMeshLeafNode::mVersion = header.mVersion;

	/// 그리드 크기를 검사
	if( CheckCellCount( header.mCellCount ) == false )
	{
		return NAVIMESH_LOAD_ERROR_GRID_SIZE;
	}

	/// 단위 설정
	mCellCount = header.mCellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = header.mMetersPerVertex;
	mUnitsPerMeter = header.mUnitsPerMeter;
	mUnitsPerVertex = mMetersPerVertex * (float)mUnitsPerMeter;
	mUnitsPerLeafNode = cNaviMeshLeafNode::mCellCount * mUnitsPerVertex;

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

	/// 버퍼 생성 및 초기화
	unsigned int buffCount = mCellCount / NAVIMESH_BUFF_CELL_COUNT;
	mBuffer = new cNaviMeshBuffer*[buffCount];

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		mBuffer[yi] = new cNaviMeshBuffer[buffCount];
	}

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].Init( this, xi * NAVIMESH_BUFF_CELL_COUNT, yi * NAVIMESH_BUFF_CELL_COUNT );
		}
	}

	/// 리프 노드 배열 생성
	unsigned int leafCount = mCellCount / NAVIMESH_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 가시 노드 배열의 용량을 설정
	mVisibleArray.Reserve( numNodes );

	/// 루트 노드 생성 및 로딩
	mRootNode = new cNaviMeshBranchNode( 0, 0, 0, mCellCount );

	if( mRootNode->Load( loader ) == false )
	{
		return NAVIMESH_LOAD_ERROR_NODE;
	}

	mRootNode->SyncHeight();

	/// 셰이더를 초기화
	cRenderer* renderer = RENDERSYS->GetRenderer();
	mShader.Init( renderer->GetNaviMeshEffect() );

	/// 정점 선언을 생성
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();

	D3DVERTEXELEMENT9 decl[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	device->CreateVertexDeclaration( decl, &mVertexDeclaration );

	/// 인덱스 버퍼를 생성
	unsigned int indexCount = NAVIMESH_LEAF_LINE_COUNT_X2;
	unsigned short indexArray[NAVIMESH_LEAF_LINE_COUNT_X2];
	unsigned short* p = indexArray;

	for( unsigned int xi = 0, xend = NAVIMESH_LEAF_LINE_COUNT; xi < xend; ++xi )
	{
		*p = (unsigned short)(xi); ++p;
		*p = (unsigned short)(xi + NAVIMESH_BUFF_LINE_COUNT); ++p;
	}

	mIndexBuffer = renderer->CreateIndexBuffer( indexCount, indexArray );
	assert( mIndexBuffer );

	///
	mInited = true;
	mModified = false;
	return 1;
}
