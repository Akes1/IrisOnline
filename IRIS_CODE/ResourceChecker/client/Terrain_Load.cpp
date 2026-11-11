#include "stdafx.h"
#include "Terrain.h"
 
#include "RenderSystem.h"
#include "TerrainNode.h"
 
static const char* gTerrainFileCode = "IrisTerrain";
 
bool cTerrain::LoadTexture( unsigned int index, const cString& pathName )
{
	if( index >= TERRAIN_TEXTURE_COUNT )
	{
		assert(0);
		return false;
	}

	/// 텍스처 로딩
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 );
		return false;
	}

	cRenderer* renderer = RENDERSYS->GetRenderer();
	LPDIRECT3DTEXTURE9 tex = 0;
	HRESULT ret = ::D3DXCreateTextureFromFileInMemory( renderer->GetD3DDevice(), loader.GetBufferPtr(), loader.GetSize(), &tex );

	if( FAILED(ret) || tex == 0 )
	{
		assert( 0 );
		return false;
	}

	///
	SAFE_DELETE( mTextures[index] );

	new cTerrainTexture( index, tex );
	return true;
}
 
bool cTerrain::LoadHeader( unsigned int& count, const cString& pathName )
{
	Clear();

	/// 기본 텍스처를 로딩
	LoadTexture( 0, "./Data/2DData/detail0.tga" );

	/// 파일 열기
	assert( mLoader.GetSize() == 0 );
	cFileLoader& loader = mLoader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to open file to load terrain" );
		return false;
	}

	/// 파일 헤더 로딩
	cTerrainFileHeader header;

	if( loader.Read( &header, sizeof(cTerrainFileHeader) ) != sizeof(cTerrainFileHeader) )
	{
		assert( 0 && "failed to load terrain file header" );
		return false;
	}

	/// 헤더 검사
	if( memcmp( header.mCode, gTerrainFileCode, 12 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		return false;
	}
	switch( header.mVersion )
	{
	case 2:
	case 3:
	case 4:
		--header.mCellCount;
		break;
	case 5:
	case 6:
		break;
	default:
		assert( 0 && "invalid file version" );
		return false;
	}

	/// 버전을 설정
	cTerrainLeafNode::mVersion = header.mVersion;

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
	mUnitsPerLeafNode = TERRAIN_LEAF_CELL_COUNT * mUnitsPerVertex;
	mSegmentLength = float(mCellCount) * mUnitsPerVertex;

	/// 리프 노드 배열의 용량을 설정
	unsigned int leafCount = mCellCount / TERRAIN_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 가시 노드 배열의 용량을 설정
	mVisibleArray.Reserve( numNodes );

	/// 텍스처 배열을 로딩
	cString texPathName;
	unsigned int iend = 0;
	switch( header.mVersion )
	{
	case 2:
		iend = TERRAIN_TEXTURE_COUNT_VER2;
		break;
	case 3:
		iend = TERRAIN_TEXTURE_COUNT;
		break;
	case 4:
	case 5:
	case 6:
		break;
	}

	switch( header.mVersion )
	{
	case 2:
	case 3:
		{
			char texName[64];

			for( unsigned int i = 1; i < iend; ++i )
			{
				if( loader.Read( texName, 64 ) != 64 )
				{
					assert( 0 && "failed to load terrain texture name" );
					return false;
				}
				if( texName[0] == 0 && texName[1] == 0 )
					continue;

				texName[63] = 0;
				texPathName.Format( "./data/map/%s", texName );

				if( LoadTexture( i, texPathName ) == false )
				{
					assert( 0 && "failed to load terrain texture" );
				}
			}
			break;
		}
	case 4:
	case 5:
	case 6:
		{
			unsigned int texIndex = 0;
			char texName[64];

			for( unsigned int i = 0, iend = header.mNumTextures; i < iend; ++i )
			{
				texIndex = 0;
				loader.ReadUnsignedInt( &texIndex );
				loader.Read( texName, 64 );

				if( texIndex == 0 )
					continue;
				if( texName[0] == 0 && texName[1] == 0 )
					continue;

				texName[63] = 0;
				texPathName.Format( "./data/map/%s", texName );

				if( LoadTexture( texIndex, texPathName ) == false )
				{
					assert( 0 && "failed to load terrain texture" );
				}
			}
			break;
		}
	}

	/// 높이맵, 알파맵, 색상맵을 로딩
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];
	mNormals = NiNew NiPoint3[numVerts];
	mColors = NiNew NiColor[numVerts];
	mAlphas = NiNew NiPoint3[numVerts];

	switch( header.mVersion )
	{
	case 2:
	case 3:
	case 4:
		break;
	case 5:
	case 6:
		loader.Read( mHeights, sizeof(float) * numVerts );
		loader.Read( mAlphas, sizeof(NiPoint3) * numVerts );
		loader.Read( mColors, sizeof(NiColor) * numVerts );
		ComputeNormals( 0, 0, mLineCount, mLineCount );
		break;
	}

	/// 버퍼 생성 및 초기화
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	mBuffer = new cTerrainBuffer*[buffCount];

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		mBuffer[yi] = new cTerrainBuffer[buffCount];
	}

	switch( header.mVersion )
	{
	case 2:
	case 3:
	case 4:
		for( unsigned int yi = 0; yi < buffCount; ++yi )
		{
			for( unsigned int xi = 0; xi < buffCount; ++xi )
			{
				mBuffer[yi][xi].Init( 0, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT );
			}
		}
		break;
	case 5:
	case 6:
		for( unsigned int yi = 0; yi < buffCount; ++yi )
		{
			for( unsigned int xi = 0; xi < buffCount; ++xi )
			{
				mBuffer[yi][xi].Init( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT );
			}
		}
		break;
	}

	/// 루트 노드 생성
	mRootNode = new cTerrainBranchNode( 0, 0, mCellCount );

	/// 로딩 준비
	assert( mLoadArray == 0 );
	mLoadArray = new cTerrainNode*[MAX_NODES];
	assert( mLoadArray );
	mLoadCount = 0;
	mLoadIndex = 0;

	mRootNode->AddToLoadArray( mLoadArray, mLoadCount );
	count = mLoadCount;

	/// set sibling leaf node
	unsigned int nodeCount = mCellCount / TERRAIN_LEAF_CELL_COUNT;
	for( unsigned int i=0;i<numNodes; i++ )
	{
		cTerrainLeafNode* n = (cTerrainLeafNode*)mNodeArray[i];

		unsigned int xi = n->mXIndex / TERRAIN_LEAF_CELL_COUNT;
		unsigned int yi = n->mYIndex / TERRAIN_LEAF_CELL_COUNT;

		if( xi != 0 )
		{
			/// exist left node
			n->mSiblingLeafNode[0] = (cTerrainLeafNode*)mNodeArray[yi * nodeCount + xi-1];
		}
		if( xi < nodeCount-1 )
		{
			/// exist right node
			n->mSiblingLeafNode[1] = (cTerrainLeafNode*)mNodeArray[yi * nodeCount + xi+1];
		}
		if( yi != 0 )
		{
			/// exist up node
			n->mSiblingLeafNode[2] = (cTerrainLeafNode*)mNodeArray[(yi-1) * nodeCount + xi];
		}
		if( yi < nodeCount-1 )
		{
			/// exist down node
			n->mSiblingLeafNode[3] = (cTerrainLeafNode*)mNodeArray[(yi+1) * nodeCount + xi];
		}
	}

	return true;
}

int cTerrain::LoadNodes( unsigned int count )
{
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
