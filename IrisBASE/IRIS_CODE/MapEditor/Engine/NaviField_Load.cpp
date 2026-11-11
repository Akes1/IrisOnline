#include "stdafx.h"
#include "NaviField.h"

#include "RenderSystem.h"
#include "NaviFieldNode.h"

static NiColor gNaviFieldColors[16] =
{
	NiColor( 0.0f, 0.0f, 0.0f ),
		NiColor( 1.0f, 0.0f, 0.0f ),
		NiColor( 0.0f, 1.0f, 0.0f ),
		NiColor( 0.0f, 0.0f, 1.0f ),
		NiColor( 1.0f, 1.0f, 0.0f ),
		NiColor( 1.0f, 0.0f, 1.0f ),
		NiColor( 0.0f, 1.0f, 1.0f ),
		NiColor( 0.5f, 0.0f, 0.0f ),
		NiColor( 0.0f, 0.5f, 0.0f ),
		NiColor( 0.0f, 0.0f, 0.5f ),
		NiColor( 0.5f, 0.5f, 0.0f ),
		NiColor( 0.5f, 0.0f, 0.5f ),
		NiColor( 0.0f, 0.5f, 0.5f ),
		NiColor( 1.0f, 1.0f, 1.0f ),
		NiColor( 0.5f, 0.5f, 0.5f ),
		NiColor( 0.25f, 0.25f, 0.25f ),
};

static const char* gpNaviFieldFileCode = "IrisNaviField";

int cNaviField::Load( const cString& pathName )
{
	Clear();

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return NAVIFIELD_LOAD_ERROR_OPEN;
	}

	/// 파일 헤더를 로딩
	cNaviFieldFileHeader header;

	if( loader.Read( &header, sizeof(cNaviFieldFileHeader) ) != sizeof(cNaviFieldFileHeader) )
	{
		return NAVIFIELD_LOAD_ERROR_FILE_HEADER;
	}

	if( ::memcmp( header.mCode, gpNaviFieldFileCode, 14 ) != 0 )
	{
		return NAVIFIELD_LOAD_ERROR_FILE_TYPE;
	}

	switch( header.mVersion )
	{
	case 2:
	case 3:
		--header.mCellCount;
		break;
	case 4:
		break;
	default:
		return NAVIFIELD_LOAD_ERROR_FILE_VERSION;
	}

	/// 그리드 크기를 검사
	if( CheckCellCount( header.mCellCount ) == false )
	{
		return NAVIFIELD_LOAD_ERROR_GRID_SIZE;
	}

	/// 단위 설정
	mCellCount = header.mCellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = header.mMetersPerVertex;
	mUnitsPerMeter = header.mUnitsPerMeter;
	mUnitsPerVertex = mMetersPerVertex * (float)mUnitsPerMeter;
	mUnitsPerLeafNode = cNaviFieldLeafNode::mCellCount * mUnitsPerVertex;

	/// 필드값 배열을 생성 및 로딩
	unsigned int numVerts = mLineCount * mLineCount;
	mValues = new unsigned char[numVerts];
	mpValues = new unsigned char*[mLineCount];

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpValues[i] = &mValues[i * mLineCount];
	}

	loader.Read( mValues, sizeof(unsigned char) * numVerts );

	/// 높이값 배열을 생성 및 로딩
	mHeights = new float[numVerts];
	mpHeights = new float*[mLineCount];

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpHeights[i] = &mHeights[i * mLineCount];
	}

	loader.Read( mHeights, sizeof(float) * numVerts );

	/// 색상 배열을 생성 및 초기화
	mColors = NiNew NiColor[numVerts];
	unsigned char value;

	for( unsigned int i = 0; i < numVerts; ++i )
	{
		value = mValues[i];
		assert( value < 16 );

		mColors[i] = gNaviFieldColors[value];
	}

	/// 버퍼 생성 및 초기화
	unsigned int buffCount = mCellCount / NAVIFIELD_BUFF_CELL_COUNT;
	mBuffer = new cNaviFieldBuffer*[buffCount];

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		mBuffer[yi] = new cNaviFieldBuffer[buffCount];
	}

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].Init( this, xi * NAVIFIELD_BUFF_CELL_COUNT, yi * NAVIFIELD_BUFF_CELL_COUNT );
		}
	}

	/// 리프 노드 배열 생성
	unsigned int leafCount = mCellCount / NAVIFIELD_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 가시 노드 배열의 용량을 설정
	mVisibleArray.Reserve( numNodes );

	/// 루트 노드 생성 및 초기화
	mRootNode = new cNaviFieldBranchNode( 0, 0, 0, mCellCount );
	mRootNode->SyncHeight();

	/// 셰이더를 초기화
	cRenderer* renderer = RENDERSYS->GetRenderer();
	mShader.Init( renderer->GetNaviFieldEffect() );

	/// 정점 선언을 생성
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();

	D3DVERTEXELEMENT9 decl[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		D3DDECL_END()
	};

	device->CreateVertexDeclaration( decl, &mVertexDeclaration );

	/// 인덱스 버퍼를 생성
	unsigned int indexCount = NAVIFIELD_LEAF_LINE_COUNT_X2;
	unsigned short indexArray[NAVIFIELD_LEAF_LINE_COUNT_X2];
	unsigned short* p = indexArray;

	for( unsigned int xi = 0, xend = NAVIFIELD_LEAF_LINE_COUNT; xi < xend; ++xi )
	{
		*p = (unsigned short)(xi); ++p;
		*p = (unsigned short)(xi + NAVIFIELD_BUFF_LINE_COUNT); ++p;
	}

	mIndexBuffer = renderer->CreateIndexBuffer( indexCount, indexArray );
	assert( mIndexBuffer );

	///
	mInited = true;
	mModified = false;
	return 1;
}
