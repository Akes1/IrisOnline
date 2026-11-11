#include "stdafx.h"
#include "NaviMesh.h"

#include "RenderSystem.h"
#include "CameraManager.h"

#include "Terrain.h"
#include "Ray.h"
#include "Box.h"
#include "NaviMeshNode.h"
#include "NaviField.h"

cNaviMeshBuffer::cNaviMeshBuffer()
: mXIndex( 0 )
, mYIndex( 0 )
, mPosCoordBuffer( 0 )
{
}

cNaviMeshBuffer::~cNaviMeshBuffer()
{
	/// 버퍼를 해제
	if( mPosCoordBuffer )
	{
		mPosCoordBuffer->Release();
		mPosCoordBuffer = 0;
	}
}

bool cNaviMeshBuffer::Init( cNaviMesh* naviMesh, unsigned int xi, unsigned int yi )
{
	assert( naviMesh );

	mXIndex = xi;
	mYIndex = yi;

	/// 버퍼를 생성
	cRenderer* renderer = RENDERSYS->GetRenderer();

	mPosCoordBuffer = renderer->CreateVertexBuffer( NAVIMESH_BUFF_VERT_COUNT, sizeof(NiPoint3), 0 );
	if( mPosCoordBuffer == 0 )
		return false;

	/// 버퍼를 초기화
	UpdatePosCoord( naviMesh, mXIndex, mYIndex, NAVIMESH_BUFF_LINE_COUNT );
	return true;
}

void cNaviMeshBuffer::UpdatePosCoord( cNaviMesh* naviMesh, unsigned int xstart, unsigned ystart, unsigned int count )
{
	assert( naviMesh );
	assert( mPosCoordBuffer );

	float unitsPerVertex = naviMesh->GetUnitsPerVertex();
	unsigned int length = NAVIMESH_BUFF_VERT_COUNT * sizeof(NiPoint3);
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
				i = (yi - mYIndex) * NAVIMESH_BUFF_LINE_COUNT + (xi - mXIndex);
				posCoords[i].x = xi * unitsPerVertex;
				posCoords[i].y = yi * unitsPerVertex;
				posCoords[i].z = naviMesh->GetHeightFast( xi, yi );
			}
		}
	}
	mPosCoordBuffer->Unlock();
}

cNaviMesh* cNaviMesh::mSingleton = 0;

cNaviMesh::cNaviMesh()
: mInited( false )
, mCellCount( 0 )
, mLineCount( 0 )
, mMetersPerVertex( 0.0f )
, mUnitsPerMeter( 0 )
, mUnitsPerVertex( 0.0f )
, mUnitsPerLeafNode( 0.0f )
, mRootNode( 0 )
, mHeights( 0 )
, mBuffer( 0 )
, mVertexDeclaration( 0 )
, mIndexBuffer( 0 )
, mModified( false )
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
	mInited = false;

	delete [] mHeights;
	mHeights = 0;
	delete mRootNode;
	mRootNode = 0;

	/// 버퍼를 제거
	if( mBuffer )
	{
		unsigned int buffCount = mCellCount / NAVIMESH_BUFF_CELL_COUNT;

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

	///
	if( mVertexDeclaration )
	{
		mVertexDeclaration->Release();
		mVertexDeclaration = 0;
	}
	if( mIndexBuffer )
	{
		mIndexBuffer->Release();
		mIndexBuffer = 0;
	}
}

void cNaviMesh::Init( unsigned int cellCount, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();

	/// 그리드 크기를 검사
	if( CheckCellCount( cellCount ) == false )
	{
		assert( 0 && "invalid navimesh grid size" );
		return;
	}

	/// 단위 설정
	mCellCount = cellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = metersPerVertex;
	mUnitsPerMeter = unitsPerMeter;
	mUnitsPerVertex = metersPerVertex * (float)unitsPerMeter;
	mUnitsPerLeafNode = cNaviMeshLeafNode::mCellCount * mUnitsPerVertex;

	/// 높이맵 생성 및 초기화
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];

	for( unsigned int i = 0, iend = numVerts; i < iend; ++i )
	{
		mHeights[i] = 0.0f;
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

	/// 루트 노드 생성 및 초기화
	mRootNode = new cNaviMeshBranchNode( 0, 0, 0, mCellCount );
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
}

void cNaviMesh::Process()
{
	if( mInited == false )
		return;
	if( mRootNode == 0 )
		return;

	/// 컬링
	const cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 );
		return;
	}

	NiFrustumPlanes frustum( *((NiCamera*)(*cam)) );
	mVisibleArray.Clear();
	mRootNode->Cull( &mVisibleArray, frustum );

	/// 정렬
	if( mVisibleArray.GetSize() > 2 )
		::Sort( mVisibleArray.Begin(), mVisibleArray.End(), cNaviMeshNodeCompareForRendering() );
}

void cNaviMesh::Render()
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

	/// 장치 설정을 백업
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 장치를 설정
	device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	device->SetVertexDeclaration( mVertexDeclaration );
	device->SetIndices( mIndexBuffer );

	/// 셰이더를 시작
	if( mShader.Begin( CAMERAMAN->GetCurrent(), renderer->GetD3DViewProj() ) == false )
		return;

	///
	cNaviMeshLeafNode* node = 0;
	cNaviMeshBuffer* buff = 0;
	unsigned int baseVertIndex = 0;

	for( unsigned int i = 0, iend = mVisibleArray.GetSize(); i < iend; ++i )
	{
		node = mVisibleArray[i];

		if( buff != node->mBuffer )
		{
			buff = node->mBuffer;

			device->SetStreamSource( 0, node->mBuffer->mPosCoordBuffer, 0, sizeof(NiPoint3) );
		}

		baseVertIndex = (node->mYIndex - buff->mYIndex) * NAVIMESH_BUFF_LINE_COUNT + (node->mXIndex - buff->mXIndex);

		for( unsigned int j = 0, jend = NAVIMESH_LEAF_LINE_COUNT-1; j < jend; ++j )
		{
			device->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, baseVertIndex, 0, NAVIMESH_BUFF_VERT_COUNT, 0, NAVIMESH_LEAF_LINE_COUNT_X2-2 );
			baseVertIndex += NAVIMESH_BUFF_LINE_COUNT;
		}
	}

	mShader.End();

	/// 장치 설정을 복원
	if( oldVertDecl )
		device->SetVertexDeclaration( oldVertDecl );
	device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

bool cNaviMesh::Pick( NiPoint3* pos, int mouseX, int mouseY )
{
	assert( pos );
	assert( mRootNode );

	NiPoint3 origin, dir;
	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );

	float dist = NI_INFINITY;
	return mRootNode->CollideRay( pos, &dist, cRay(origin, dir) );
}

bool cNaviMesh::Pick( NiPoint3* pos, const cRay& ray )
{
	float dist = NI_INFINITY;
	return mRootNode->CollideRay( pos, &dist, ray );
}

bool cNaviMesh::Pick( NiPoint3* pos, float* dist, const cRay& ray, float maxDistance )
{
	return mRootNode->CollideRay( pos, dist, ray, maxDistance );
}

void cNaviMesh::SyncAllToTerrain()
{
	for( unsigned int yi = 0, i = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			TERRAIN->GetHeight( &(mHeights[i]), xi, yi );
		}
	}

	unsigned int buffCount = mCellCount / NAVIMESH_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * NAVIMESH_BUFF_CELL_COUNT, yi * NAVIMESH_BUFF_CELL_COUNT, NAVIMESH_BUFF_LINE_COUNT );
		}
	}

	mRootNode->SyncHeight();
	mModified = true;
}

bool cNaviMesh::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius )
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
	int xb = (int)((pos.x - outerRadius) / mUnitsPerLeafNode) - 1;
	int yb = (int)((pos.y - outerRadius) / mUnitsPerLeafNode) - 1;
	int xe = (int)((pos.x + outerRadius) / mUnitsPerLeafNode) + 2;
	int ye = (int)((pos.y + outerRadius) / mUnitsPerLeafNode) + 2;
	int row = mCellCount / cNaviMeshLeafNode::mCellCount;

	if( xb < 0 )
		xb = 0;
	if( yb < 0 )
		yb = 0;
	if( xe > row )
		xe = row;
	if( ye > row )
		ye = row;

	*xbegin = xb;
	*ybegin = yb;
	*xend = xe;
	*yend = ye;
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

bool cNaviMesh::GetHeight( float* height, unsigned int xi, unsigned int yi ) const
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*height = mHeights[yi * mLineCount + xi];
	return true;
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

	return mNodeArray[yi * nodeCount + xi];
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
