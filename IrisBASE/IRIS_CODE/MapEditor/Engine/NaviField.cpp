#include "stdafx.h"
#include "NaviField.h"

#include "RenderSystem.h"
#include "CameraManager.h"

#include "Ray.h"
#include "Box.h"
#include "NaviFieldNode.h"
#include "NaviMesh.h"

cNaviFieldBuffer::cNaviFieldBuffer()
: mXIndex( 0 )
, mYIndex( 0 )
, mPosCoordBuffer( 0 )
, mColorBuffer( 0 )
{
}

cNaviFieldBuffer::~cNaviFieldBuffer()
{
	/// 버퍼를 해제
	if( mPosCoordBuffer )
	{
		mPosCoordBuffer->Release();
		mPosCoordBuffer = 0;
	}
	if( mColorBuffer )
	{
		mColorBuffer->Release();
		mColorBuffer = 0;
	}
}

bool cNaviFieldBuffer::Init( cNaviField* naviField, unsigned int xi, unsigned int yi )
{
	assert( naviField );

	mXIndex = xi;
	mYIndex = yi;

	/// 버퍼를 생성
	cRenderer* renderer = RENDERSYS->GetRenderer();

	mPosCoordBuffer = renderer->CreateVertexBuffer( NAVIFIELD_BUFF_VERT_COUNT, sizeof(NiPoint3), 0 );
	if( mPosCoordBuffer == 0 )
		return false;

	mColorBuffer = renderer->CreateVertexBuffer( NAVIFIELD_BUFF_VERT_COUNT, sizeof(NiColor), 0 );
	if( mColorBuffer == 0 )
		return false;

	/// 버퍼를 초기화
	UpdatePosCoord( naviField, mXIndex, mYIndex, NAVIFIELD_BUFF_LINE_COUNT );
	UpdateColor( naviField, mXIndex, mYIndex, NAVIFIELD_BUFF_LINE_COUNT );
	return true;
}

void cNaviFieldBuffer::UpdatePosCoord( cNaviField* naviField, unsigned int xstart, unsigned ystart, unsigned int count )
{
	assert( naviField );
	assert( mPosCoordBuffer );

	float unitsPerVertex = naviField->GetUnitsPerVertex();
	unsigned int length = NAVIFIELD_BUFF_VERT_COUNT * sizeof(NiPoint3);
	void* p = 0;
	HRESULT ret = mPosCoordBuffer->Lock( 0, length, &p, 0 );
	NiPoint3* posCoords = (NiPoint3*)p;

	if( SUCCEEDED(ret) )
	{
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * NAVIFIELD_BUFF_LINE_COUNT + (xi - mXIndex);
				posCoords[i].x = xi * unitsPerVertex;
				posCoords[i].y = yi * unitsPerVertex;
				posCoords[i].z = naviField->GetHeightFast( xi, yi ) + 30.0f;
			}
		}
	}
	mPosCoordBuffer->Unlock();
}

void cNaviFieldBuffer::UpdateColor( cNaviField* naviField, unsigned int xstart, unsigned ystart, unsigned int count )
{
	assert( naviField );
	assert( mColorBuffer );

	unsigned int length = NAVIFIELD_BUFF_VERT_COUNT * sizeof(NiColor);
	void* p = 0;
	HRESULT ret = mColorBuffer->Lock( 0, length, &p, 0 );
	NiColor* colors = (NiColor*)p;

	if( SUCCEEDED(ret) )
	{
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * NAVIFIELD_BUFF_LINE_COUNT + (xi - mXIndex);
				colors[i] = naviField->GetColorFast( xi, yi );
			}
		}
	}
	mColorBuffer->Unlock();
}

cNaviField* cNaviField::mSingleton = 0;

cNaviField::cNaviField()
: mInited( false )
, mCellCount( 0 )
, mLineCount( 0 )
, mMetersPerVertex( 0.0f )
, mUnitsPerMeter( 0 )
, mUnitsPerVertex( 0.0f )
, mUnitsPerLeafNode( 0.0f )
, mRootNode( 0 )
, mpValues( 0 )
, mValues( 0 )
, mpHeights( 0 )
, mHeights( 0 )
, mColors( 0 )
, mBuffer( 0 )
, mVertexDeclaration( 0 )
, mIndexBuffer( 0 )
, mModified( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cNaviField::~cNaviField()
{
	Clear();

	mSingleton = 0;
}

void cNaviField::Clear()
{
	mInited = false;

	delete [] mpValues;
	mpValues = 0;
	delete [] mValues;
	mValues = 0;
	delete [] mpHeights;
	mpHeights = 0;
	delete [] mHeights;
	mHeights = 0;
	NiDelete [] mColors;
	mColors = 0;
	delete mRootNode;
	mRootNode = 0;

	/// 버퍼를 제거
	if( mBuffer )
	{
		unsigned int buffCount = mCellCount / NAVIFIELD_BUFF_CELL_COUNT;

		for( unsigned int i = 0; i < buffCount; ++i )
		{
			delete [] mBuffer[i];
		}
		delete [] mBuffer;
		mBuffer = 0;
	}

	mCellCount = 0;
	mLineCount = 0;
	mMetersPerVertex = 0.0f;
	mUnitsPerMeter = 0;
	mUnitsPerVertex = 0.0f;
	mUnitsPerLeafNode = 0.0f;

	mNodeArray.Clear();
	mVisibleArray.Clear();
}

void cNaviField::Init( unsigned int cellCount, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();

	/// 그리드 크기를 검사
	if( CheckCellCount( cellCount ) == false )
	{
		assert( 0 && "invalid navifield grid size" );
		return;
	}

	/// 단위 설정
	mCellCount = cellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = metersPerVertex;
	mUnitsPerMeter = unitsPerMeter;
	mUnitsPerVertex = metersPerVertex * (float)unitsPerMeter;
	mUnitsPerLeafNode = cNaviFieldLeafNode::mCellCount * mUnitsPerVertex;

	/// 높이값 배열 생성 및 초기화
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];
	mpHeights = new float*[mLineCount];

	for( unsigned int i = 0; i < numVerts; ++i )
	{
		mHeights[i] = 0.0f;
	}

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpHeights[i] = &mHeights[i * mLineCount];
	}

	/// 색상 배열 생성 및 초기화
	mColors = NiNew NiColor[numVerts];

	for( unsigned int i = 0; i < numVerts; ++i )
	{
		mColors[i] = NiColor::BLACK;
	}

	/// 비트 플랙 배열 생성 및 초기화
	mValues = new unsigned char[numVerts];
	mpValues = new unsigned char*[mLineCount];

	for( unsigned int i = 0; i < numVerts; ++i )
	{
		mValues[i] = 0;
	}

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpValues[i] = &mValues[i * mLineCount];
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
}

void cNaviField::Process()
{
	if( mInited == false )
		return;
	if( mRootNode == 0 )
		return;

	/// 컬링
	const cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 && "can't get current camera" );
		return;
	}

	NiFrustumPlanes frustum( *((NiCamera*)*cam) );
	mVisibleArray.Clear();
	mRootNode->Cull( &mVisibleArray, frustum );

	/// 정렬
	if( mVisibleArray.GetSize() > 2 )
		::Sort( mVisibleArray.Begin(), mVisibleArray.End(), cNaviFieldNodeCompareForRendering() );
}

void cNaviField::Render()
{
	if( mInited == false )
		return;
	if( mRootNode == 0 )
		return;

	///
	unsigned int numGeoms = mVisibleArray.GetSize();
	if( numGeoms == 0 )
		return;

	///
	cRenderer* renderer = RENDERSYS->GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 );
		return;
	}

	/// 렌더링 상태를 저장
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 장치를 설정
	device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	device->SetVertexDeclaration( mVertexDeclaration );
	device->SetIndices( mIndexBuffer );

	/// 셰이더를 시작
	if( mShader.Begin( CAMERAMAN->GetCurrent(), renderer->GetD3DViewProj() ) == false )
		return;

	///
	cNaviFieldLeafNode* node = 0;
	cNaviFieldBuffer* buff = 0;
	unsigned int baseVertIndex = 0;

	for( unsigned int i = 0, iend = mVisibleArray.GetSize(); i < iend; ++i )
	{
		node = mVisibleArray[i];

		if( buff != node->mBuffer )
		{
			buff = node->mBuffer;

			device->SetStreamSource( 0, node->mBuffer->mPosCoordBuffer, 0, sizeof(NiPoint3) );
			device->SetStreamSource( 1, node->mBuffer->mColorBuffer, 0, sizeof(NiColor) );
		}

		baseVertIndex = (node->mYIndex - buff->mYIndex) * NAVIFIELD_BUFF_LINE_COUNT + (node->mXIndex - buff->mXIndex);

		for( unsigned int j = 0, jend = NAVIFIELD_LEAF_LINE_COUNT-1; j < jend; ++j )
		{
			device->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, baseVertIndex, 0, NAVIFIELD_BUFF_VERT_COUNT, 0, NAVIFIELD_LEAF_LINE_COUNT_X2-2 );
			baseVertIndex += NAVIFIELD_BUFF_LINE_COUNT;
		}
	}

	mShader.End();

	/// 장치 설정을 복원
	if( oldVertDecl )
		device->SetVertexDeclaration( oldVertDecl );
}

bool cNaviField::Pick( NiPoint3* pos, int mouseX, int mouseY )
{
	assert( pos );
	assert( mRootNode );

	NiPoint3 origin, dir;
	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );

	float dist = NI_INFINITY;
	return mRootNode->CollideRay( pos, &dist, cRay(origin, dir) );
}

bool cNaviField::Pick( NiPoint3* pos, const cRay& ray )
{
	float dist = NI_INFINITY;
	return mRootNode->CollideRay( pos, &dist, ray );
}

void cNaviField::SyncAllToNaviMesh()
{
	if( mMetersPerVertex != NAVIMESH->GetMetersPerVertex() )
	{
		assert( 0 );
		return;
	}

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			NAVIMESH->GetHeight( &(mHeights[i]), xi, yi );
		}
	}

	unsigned int buffCount = mCellCount / NAVIFIELD_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * NAVIFIELD_BUFF_CELL_COUNT, yi * NAVIFIELD_BUFF_CELL_COUNT, NAVIFIELD_BUFF_LINE_COUNT );
		}
	}

	mRootNode->SyncHeight();
	mModified = true;
}

bool cNaviField::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius )
{
	/// 해당 위치가 지형 안에 들어오는지 검사
	const cBox& box = mRootNode->GetBoundBox();

	if( pos.x < box.GetMin().x || pos.y < box.GetMin().y ||
		pos.x > box.GetMax().x || pos.y > box.GetMax().y )
	{
		assert( 0 && "pos out of range" );
		return false;
	}

	/// 범위 계산
	int xb = (int)((pos.x - radius) / mUnitsPerLeafNode) - 1;
	int yb = (int)((pos.y - radius) / mUnitsPerLeafNode) - 1;
	int xe = (int)((pos.x + radius) / mUnitsPerLeafNode) + 2;
	int ye = (int)((pos.y + radius) / mUnitsPerLeafNode) + 2;
	int nodeCount = mCellCount / cNaviFieldLeafNode::mCellCount;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe > nodeCount )
		xe = nodeCount;
	if( ye > nodeCount )
		ye = nodeCount;

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
	return true;
}

bool cNaviField::CheckCellCount( unsigned int cellCount )
{
	if( cellCount % NAVIFIELD_DEFAULT_RESOLUTION != 0 )
	{
		assert( 0 );
		return false;
	}

	cNaviFieldLeafNode::mCellCount = NAVIFIELD_LEAF_CELL_COUNT;
	cNaviFieldLeafNode::mLineCount = NAVIFIELD_LEAF_LINE_COUNT;
	return true;
}

void cNaviField::SetLeafNode( unsigned int xi, unsigned int yi, cNaviFieldLeafNode* node )
{
	assert( node );

	unsigned int nodeCount = mCellCount / cNaviFieldLeafNode::mCellCount;
	mNodeArray[yi * nodeCount + xi] = node;
}

void cNaviField::SetValue( unsigned int xi, unsigned int yi, unsigned char value )
{
	mValues[yi * mLineCount + xi] = value;
}

void cNaviField::SetColor( unsigned int xi, unsigned int yi, const NiColor& color )
{
	mColors[yi * mLineCount + xi] = color;
}

unsigned char cNaviField::GetValue( unsigned int xi, unsigned int yi ) const
{
	assert( mpValues );

	if( xi < mLineCount && yi < mLineCount )
	{
		return mpValues[yi][xi];
	}
	else
	{
		assert( 0 && "index out of range" );
		return 0;
	}
}

bool cNaviField::GetValue( unsigned char* value, unsigned int xi, unsigned int yi ) const
{
	assert( value );
	assert( mpValues );

	if( xi < mLineCount && yi < mLineCount )
	{
		*value = mpValues[yi][xi];
		return true;
	}
	else
	{
		assert( 0 && "index out of range" );
		return false;
	}
}

bool cNaviField::GetValue( unsigned char* value, const NiPoint3& pos ) const
{
	assert( value );
	assert( mpValues );

	/// 범위를 벗어나면 바로 리턴
	int xi = (int)((pos.x + 50.0f) / mUnitsPerVertex);
	int yi = (int)((pos.y + 50.0f) / mUnitsPerVertex);

	if( xi < 0 )
		return false;
	if( yi < 0 )
		return false;
	if( xi >= (int)mLineCount )
		return false;
	if(	yi >= (int)mLineCount )
		return false;

	*value = mpValues[yi][xi];
	return true;
}

bool cNaviField::GetHeight( float* height, unsigned int xi, unsigned int yi ) const
{
	assert( height );
	assert( mpHeights );

	if( xi < mLineCount && yi < mLineCount )
	{
		*height = mpHeights[yi][xi];
		return true;
	}
	else
	{
		assert( 0 && "index out of range" );
		return false;
	}
}

bool cNaviField::GetHeight( float* height, const NiPoint3& pos ) const
{
	assert( height );
	assert( mpHeights );

	/// 범위를 벗어나면 바로 리턴
	int xi = (int)(pos.x / mUnitsPerVertex);
	int yi = (int)(pos.y / mUnitsPerVertex);

	if( xi < 0 )
		return false;
	if( yi < 0 )
		return false;
	if( xi >= (int)mLineCount )
		return false;
	if(	yi >= (int)mLineCount )
		return false;

	*height = mpHeights[yi][xi];
	return true;
}

bool cNaviField::GetColor( NiColor* color, unsigned int xi, unsigned int yi )
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*color = mColors[yi * mLineCount + xi];
	return true;
}

bool cNaviField::GetColor( NiColor* color, const NiPoint3& pos ) const
{
	/// 범위를 벗어나면 바로 리턴
	int xi = (int)(pos.x / mUnitsPerVertex);
	int yi = (int)(pos.y / mUnitsPerVertex);

	if( xi < 0 )
		return false;
	if( yi < 0 )
		return false;
	if( xi >= (int)mLineCount )
		return false;
	if(	yi >= (int)mLineCount )
		return false;

	*color = mColors[yi * mLineCount + xi];
	return true;
}

unsigned int cNaviField::GetLeafGridSize() const
{
	return cNaviFieldLeafNode::mLineCount;
}
