#include "stdafx.h"
#include "Terrain.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "TerrainNode.h"

static const char* gTerrainFileCode = "IrisTerrain";

int cTerrain::Load( const cString& pathName )
{
	Clear();

#ifdef REGEN_TOOL
	LoadTexture( 0, "./Data/2DData/detail0.tga" );
#endif

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return TERRAIN_LOAD_ERROR_OPEN;
	}

	/// 파일 헤더 로딩
	cTerrainFileHeader header;

	if( loader.Read( &header, sizeof(cTerrainFileHeader) ) != sizeof(cTerrainFileHeader) )
	{
		return TERRAIN_LOAD_ERROR_FILE_HEADER;
	}

	if( ::memcmp( header.mCode, gTerrainFileCode, 12 ) != 0 )
	{
		return TERRAIN_LOAD_ERROR_FILE_TYPE;
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
		return TERRAIN_LOAD_ERROR_FILE_VERSION;
	}

	/// 버전을 설정
	cTerrainLeafNode::mVersion = header.mVersion;

	/// 그리드 크기를 검사
	if( CheckCellCount( header.mCellCount ) == false )
	{
		return TERRAIN_LOAD_ERROR_GRID_SIZE;
	}

	/// 단위 설정
	mCellCount = header.mCellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = header.mMetersPerVertex;
	mUnitsPerMeter = header.mUnitsPerMeter;
	mUnitsPerVertex = mMetersPerVertex * (float)mUnitsPerMeter;
	mUnitsPerLeafNode = cTerrainLeafNode::mCellCount[0] * mUnitsPerVertex;

	/// 텍스처 배열을 로딩
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

	cString texPathName;
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
					return TERRAIN_LOAD_ERROR_TEXTURE_NAME;
				}
				if( texName[0] == 0 && texName[1] == 0 )
					continue;

				texName[63] = 0;
#ifdef MAP_EDITOR
				if( LoadTexture( i, texName ) == false )
#else
				texPathName.Format( ".data/map/%s", texName );

				if( LoadTexture( i, texPathName ) == false )
#endif
				{
					return TERRAIN_LOAD_ERROR_TEXTURE;
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

#ifdef MAP_EDITOR
				if( LoadTexture( texIndex, texName ) == false )
#else
				texPathName.Format( "./data/map/%s", texName );

				if( LoadTexture( texIndex, texPathName ) == false )
#endif
				{
					return TERRAIN_LOAD_ERROR_TEXTURE;
				}
				else
				{
					--header.mNumTextures;
				}
			}
			assert( header.mNumTextures == 0 );
			break;
		}
	}

	/// 높이맵, 알파맵, 색상맵을 로딩
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];
	mNormals = NiNew NiPoint3[numVerts];
	mColors = NiNew NiColor[numVerts];
	mAlphas = NiNew NiPoint3[numVerts];

#ifdef MAP_EDITOR
	mPaintAlphas = NiNew NiPoint3[numVerts];
#endif

	switch( header.mVersion )
	{
	case 2:
	case 3:
	case 4:
		break;
	case 5:
		loader.Read( mHeights, sizeof(float) * numVerts );
		loader.Read( mAlphas, sizeof(NiPoint3) * numVerts );
		loader.Read( mColors, sizeof(NiColor) * numVerts );
		ComputeNormals( 0, 0, mLineCount, mLineCount );
#ifdef MAP_EDITOR
		::memcpy( mPaintAlphas, mAlphas, sizeof(NiPoint3)*numVerts );
#endif
		break;
	case 6:
		loader.Read( mHeights, sizeof(float) * numVerts );
		loader.Read( mAlphas, sizeof(NiPoint3) * numVerts );
		loader.Read( mColors, sizeof(NiColor) * numVerts );
		ComputeNormals( 0, 0, mLineCount, mLineCount );
#ifdef MAP_EDITOR
		if( LoadPaintAlpha( pathName, numVerts ) == false )
			::memcpy( mPaintAlphas, mAlphas, sizeof(NiPoint3)*numVerts );
#endif
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

	/// 셰이더를 초기화
	cRenderer* renderer = RENDERSYS->GetRenderer();
	mShader0.Init( renderer->GetTerrainEffect() );
	mShader1.Init( renderer->GetVColorEffect() );

	/// 정점 선언을 생성
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();

	D3DVERTEXELEMENT9 decl0[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		{ 3, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	D3DVERTEXELEMENT9 decl1[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		D3DDECL_END()
	};

	device->CreateVertexDeclaration( decl0, &mVertexDeclaration0 );
	device->CreateVertexDeclaration( decl1, &mVertexDeclaration1 );

	/// 리프 노드 배열 생성
	unsigned int leafCount = mCellCount / TERRAIN_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 가시 노드 배열의 용량을 설정
	mVisibleArray.Reserve( numNodes );

	/// 루트 노드 생성 및 로딩
	mRootNode = new cTerrainBranchNode( 0, 0, 0, mCellCount );

	if( mRootNode->Load( loader ) == false )
	{
		return TERRAIN_LOAD_ERROR_NODE;
	}

	mRootNode->SyncHeight();
	mRootNode->SyncAlpha();
#ifdef MAP_EDITOR
	mRootNode->SyncPaintAlpha();
#endif
	mRootNode->SyncColor();

	/// 장면 트리가 지형보다 작으면 재생성
	const cBox& box = mRootNode->GetBoundBox();
	NiPoint3 center = (box.GetMin() + box.GetMax()) * 0.5f;
	center.z = 0.0f;
	float maxRadius = (box.GetMax() - center).Length() * 1.1f;

	if( SCENEMAN->GetMaxRadius() < maxRadius )
	{
		SCENEMAN->InitTree( *this );
	}

	///
	mInited = true;
#ifdef MAP_EDITOR
	mModified = false;
#endif
	for( unsigned int i=0;i<numNodes; i++ )
	{
		cTerrainLeafNode* n = mNodeArray[i];

		unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];
		unsigned int xi = n->mXIndex / cTerrainLeafNode::mCellCount[0];
		unsigned int yi = n->mYIndex / cTerrainLeafNode::mCellCount[0];

		if( xi != 0 )
		{
			/// exist left node
			n->mSiblingLeafNode[0] = mNodeArray[yi * nodeCount + xi-1];
		}
		if( xi < nodeCount-1 )
		{
			/// exist right node
			n->mSiblingLeafNode[1] = mNodeArray[yi * nodeCount + xi+1];
		}
		if( yi != 0 )
		{
			/// exist up node
			n->mSiblingLeafNode[2] = mNodeArray[(yi-1) * nodeCount + xi];
		}
		if( yi < nodeCount-1 )
		{
			/// exist down node
			n->mSiblingLeafNode[3] = mNodeArray[(yi+1) * nodeCount + xi];
		}
	}
	return 1;
}

#ifdef MAP_EDITOR
bool cTerrain::LoadPaintAlpha( const cString& pathName, unsigned int numVerts )
{
	NiFilename filename(pathName.Cstr());
	filename.SetExt(".paint");

	char buffer[MAX_PATH] = {0,};
	filename.GetFullPath(buffer, MAX_PATH);
	cString path( buffer );


	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( path, true ) == false )
		return false;

	loader.Read( mPaintAlphas, sizeof(NiPoint3) * numVerts );

	return true;
}
#endif


bool cTerrain::LoadTexture( const cString& pathName )
{
	if( mNumTextures >= TERRAIN_TEXTURE_COUNT )
		return false;

	cString fileName;
	if( ::GetFileName( &fileName, pathName ) == false )
		fileName = pathName;

	/// 이미 존재하는지 검사
	LPDIRECT3DTEXTURE9 tex = 0;
	cTextureMap::cIterator pos = mTextureMap.Find( fileName );

	if( pos != mTextureMap.End() )
	{
		NiMessageBox( "Same Texture FileName Error", "Error" );
		return false;
//		tex = pos->mSecond;
	}
	else
	{
		/// 텍스처 로딩
		cFileLoader loader;
		if( loader.Open( pathName, true ) == false )
		{
			assert( 0 );
			return false;
		}

		cRenderer* renderer = RENDERSYS->GetRenderer();
		HRESULT ret = ::D3DXCreateTextureFromFileInMemory( renderer->GetD3DDevice(), loader.GetBufferPtr(), loader.GetSize(), &tex );

		if( FAILED(ret) )
		{
			assert( 0 );
			return false;
		}
	}

	if( tex == 0 )
	{
		assert( 0 );
		return false;
	}

	for( unsigned int i = 1; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		if( mTextures[i] == 0 )
		{
			mTextures[i] = new cTerrainTexture( i, pathName, tex );
			break;
		}
	}
	return true;
}

bool cTerrain::LoadTexture( unsigned int index, const cString& pathName )
{
	if( mNumTextures >= TERRAIN_TEXTURE_COUNT )
		return false;

	cString fileName;
	if( ::GetFileName( &fileName, pathName ) == false )
		fileName = pathName;

	/// 이미 존재하는지 검사
	LPDIRECT3DTEXTURE9 tex = 0;
	cTextureMap::cIterator pos = mTextureMap.Find( fileName );

	if( pos != mTextureMap.End() )
	{
		NiMessageBox( "Same Texture FileName Error", "Error" );
		return true;
//		tex = pos->mSecond;
	}
	else
	{

		/// 텍스처 로딩
		cFileLoader loader;
		if( loader.Open( pathName, true ) == false )
		{
			assert( 0 );
			return false;
		}


		cRenderer* renderer = RENDERSYS->GetRenderer();
		HRESULT ret = ::D3DXCreateTextureFromFileInMemory( renderer->GetD3DDevice(), loader.GetBufferPtr(), loader.GetSize(), &tex );

		if( FAILED(ret) )
		{
			assert( 0 );
			return false;
		}
	}

	if( tex == 0 )
	{
		assert( 0 );
		return false;
	}

	///
	cTerrainTexture* oldTex = mTextures[index];
	delete oldTex;
	mTextures[index] = 0;

	cTerrainTexture* newTex = new cTerrainTexture( index, pathName, tex );
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = mNodeArray[i];
		if( n == 0 )
			continue;

		if( n->mTexture0 == oldTex )
		{
			n->SetTexture0( newTex );
		}
		if( n->mTexture1 == oldTex )
		{
			n->SetTexture1( newTex );
		}
		if( n->mTexture2 == oldTex )
		{
			n->SetTexture2( newTex );
		}
	}

	return true;
}

void cTerrain::UnLoadTexture( unsigned int index )
{
	if( index == 0 )
		return;

	cTerrainTexture* tex = mTextures[index];

	if( tex == 0 )
		return;

	bool forceToUnLoad = true;

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = mNodeArray[i];

		if( n->mTexture0 == tex )
		{
			n->SetTexture0( mTextures[0] );
			forceToUnLoad = false;
		}
		if( n->mTexture1 == tex )
		{
			n->SetTexture1( mTextures[0] );
			forceToUnLoad = false;
		}
		if( n->mTexture2 == tex )
		{
			n->SetTexture2( mTextures[0] );
			forceToUnLoad = false;
		}
	}

	if( forceToUnLoad )
		delete tex;

	assert( mTextures[index] == 0 );
}
