#include "stdafx.h"
#include "Terrain.h"

#include "RenderSystem.h"
#include "Ray.h"
#include "TerrainNode.h"
#include "CameraManager.h"
#include "LightAgent.h"
#include "FogAgent.h"

#if (defined(_DEVSYS) || defined(_DEBUG))
#include "DevSystem.h"
#endif

cTerrainTexture::cTerrainTexture( unsigned int index, LPDIRECT3DTEXTURE9 tex )
: mIndexByTerrain( index )
, mRefCount( 0 )
, mTexture( tex )
{
	assert( tex );

	///
	if( TERRAIN )
		TERRAIN->AddTexture( index, this );
}

cTerrainTexture::~cTerrainTexture()
{
	if( mTexture )
		mTexture->Release();

	if( TERRAIN )
		TERRAIN->RemoveTexture( mIndexByTerrain );

	assert( mRefCount == 0 && "bad reference counting!" );
}

void cTerrainTexture::Grab()
{
	++mRefCount;
}

void cTerrainTexture::Drop()
{
	if( --mRefCount == 0 )
	{
		delete this;
	}
}

cTerrainBuffer::cTerrainBuffer()
: mXIndex( 0 )
, mYIndex( 0 )
, mPosCoordBuffer( 0 )
, mNormalBuffer( 0 )
, mAlphaBuffer( 0 )
, mColorBuffer( 0 )
{
}

cTerrainBuffer::~cTerrainBuffer()
{
	/// 버퍼를 해제
	if( mPosCoordBuffer )
	{
		mPosCoordBuffer->Release();
		mPosCoordBuffer = 0;
	}
	if( mNormalBuffer )
	{
		mNormalBuffer->Release();
		mNormalBuffer = 0;
	}
	if( mAlphaBuffer )
	{
		mAlphaBuffer->Release();
		mAlphaBuffer = 0;
	}
	if( mColorBuffer )
	{
		mColorBuffer->Release();
		mColorBuffer = 0;
	}
}

bool cTerrainBuffer::Init( cTerrain* terrain, unsigned int xi, unsigned int yi )
{
	mXIndex = xi;
	mYIndex = yi;

	/// 버퍼를 생성
	cRenderer* renderer = RENDERSYS->GetRenderer();

	mPosCoordBuffer = renderer->CreateVertexBuffer( TERRAIN_BUFF_VERT_COUNT, sizeof(NiPoint3), 0 );
	if( mPosCoordBuffer == 0 )
		return false;

	mNormalBuffer = renderer->CreateVertexBuffer( TERRAIN_BUFF_VERT_COUNT, sizeof(NiPoint3), 0 );
	if( mNormalBuffer == 0 )
		return false;

	mColorBuffer = renderer->CreateVertexBuffer( TERRAIN_BUFF_VERT_COUNT, sizeof(NiColor), 0 );
	if( mColorBuffer == 0 )
		return false;

	mAlphaBuffer = renderer->CreateVertexBuffer( TERRAIN_BUFF_VERT_COUNT, sizeof(NiPoint3), 0 );
	if( mAlphaBuffer == 0 )
		return false;

	/// 버퍼를 초기화
	if( terrain )
	{
		UpdatePosCoord( terrain, mXIndex, mYIndex, TERRAIN_BUFF_LINE_COUNT );
		UpdateColor( terrain, mXIndex, mYIndex, TERRAIN_BUFF_LINE_COUNT );
		UpdateAlpha( terrain, mXIndex, mYIndex, TERRAIN_BUFF_LINE_COUNT );
	}
	return true;
}

void cTerrainBuffer::UpdatePosCoord( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count )
{
	assert( terrain );
	assert( mPosCoordBuffer );
	assert( mNormalBuffer );

	float unitsPerVertex = terrain->GetUnitsPerVertex();
	unsigned int length = TERRAIN_BUFF_VERT_COUNT * sizeof(NiPoint3);
	void* p = 0;
	HRESULT ret = mPosCoordBuffer->Lock( 0, length, &p, 0 );

	if( SUCCEEDED(ret) )
	{
		NiPoint3* posCoords = (NiPoint3*)p;
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * TERRAIN_BUFF_LINE_COUNT + (xi - mXIndex);
				posCoords[i].x = xi * unitsPerVertex;
				posCoords[i].y = yi * unitsPerVertex;
				posCoords[i].z = terrain->GetHeightFast( xi, yi );
			}
		}
	}
	mPosCoordBuffer->Unlock();

	///
	length = TERRAIN_BUFF_VERT_COUNT * sizeof(NiPoint3);
	p = 0;
	ret = mNormalBuffer->Lock( 0, length, &p, 0 );

	if( SUCCEEDED(ret) )
	{
		NiPoint3* normals = (NiPoint3*)p;
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * TERRAIN_BUFF_LINE_COUNT + (xi - mXIndex);
				normals[i] = terrain->GetNormalFast( xi, yi );
			}
		}
	}
	mNormalBuffer->Unlock();
}

void cTerrainBuffer::UpdateColor( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count )
{
	assert( terrain );
	assert( mColorBuffer );

	unsigned int length = TERRAIN_BUFF_VERT_COUNT * sizeof(NiColor);
	void* p = 0;
	HRESULT ret = mColorBuffer->Lock( 0, length, &p, 0 );

	if( SUCCEEDED(ret) )
	{
		NiColor* colors = (NiColor*)p;
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * TERRAIN_BUFF_LINE_COUNT + (xi - mXIndex);
				colors[i] = terrain->GetColorFast( xi, yi );
			}
		}
	}
	mColorBuffer->Unlock();
}

void cTerrainBuffer::UpdateAlpha( cTerrain* terrain, unsigned int xstart, unsigned int ystart, unsigned int count )
{
	assert( terrain );
	assert( mAlphaBuffer );

	unsigned int length = TERRAIN_BUFF_VERT_COUNT * sizeof(NiPoint3);
	void* p = 0;
	HRESULT ret = mAlphaBuffer->Lock( 0, length, &p, 0 );

	if( SUCCEEDED(ret) )
	{
		NiPoint3* alphas = (NiPoint3*)p;
		unsigned int i = 0;

		for( unsigned int yi = ystart, yend = ystart + count; yi < yend; ++yi )
		{
			for( unsigned int xi = xstart, xend = xstart + count; xi < xend; ++xi )
			{
				i = (yi - mYIndex) * TERRAIN_BUFF_LINE_COUNT + (xi - mXIndex);
				alphas[i] = terrain->GetAlphaFast( xi, yi );
			}
		}
	}
	mAlphaBuffer->Unlock();
}

cTerrain* cTerrain::mSingleton = 0;

cTerrain::cTerrain()
: mCellCount( 0 )
, mLineCount( 0 )
, mMetersPerVertex( 0.0f )
, mUnitsPerMeter( 0 )
, mUnitsPerVertex( 0.0f )
, mUnitsPerLeafNode( 0.0f )
, mSegmentLength( 0.0f )
, mRootNode( 0 )
, mHeights( 0 )
, mNormals( 0 )
, mColors( 0 )
, mAlphas( 0 )
, mBuffer( 0 )
, mVertexDeclaration( 0 )
, mLoadArray( 0 )
, mLoadCount( 0 )
, mLoadIndex( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	for( unsigned int i = 0; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		mTextures[i] = 0;
	}

	/// Lod 거리 설정
	float dist = 15000.0f;

	for( unsigned int i = 0, iend = TERRAIN_LOD_COUNT - 1; i < iend; ++i )
	{
		mSquaredDistanceLod[i] = dist * dist;
		dist *= 1.5f;
	}
	mSquaredDistanceLod[TERRAIN_LOD_COUNT-1] = NI_INFINITY;

	/// 셰이더를 초기화
	cRenderer* renderer = RENDERSYS->GetRenderer();
	mShader.Init( renderer->GetTerrainEffect() );

	/// 정점 선언을 생성
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();

	D3DVERTEXELEMENT9 decl[] = 
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
		{ 2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
		{ 3, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	device->CreateVertexDeclaration( decl, &mVertexDeclaration );

	/// create Index buffer
	mIndexCount8x8 = 0;
	mIndexCount4x4 = 0;
	mIndexCount2x2 = 0;
	mIndexBuffer8x8 = NULL;
	mIndexBuffer4x4 = NULL;
	mIndexBuffer2x2 = NULL;

	for( unsigned int i=0;i<eCRACK_MAX;i++ )
	{
		mCrackIndexCount4x4[i] = 0;
		mCrackIndexCount2x2[i] = 0;

		mCrackIndexBuffer4x4[i] = NULL;
		mCrackIndexBuffer2x2[i] = NULL;
	}

	CreateIndexedBuffer();
}

void cTerrain::CreateIndexedBuffer()
{
	cRenderer* renderer = RENDERSYS->GetRenderer();
	{
		/// 기본 인덱스 버퍼를 생성 8x8
		mIndexCount8x8 = (TERRAIN_LEAF_LINE_COUNT_X2*TERRAIN_LEAF_CELL_COUNT) + (TERRAIN_LEAF_CELL_COUNT-1)*2;
		unsigned short* pIndex = new unsigned short[mIndexCount8x8];
		unsigned short* p = pIndex;

		unsigned int offsetCount = TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = TERRAIN_LEAF_CELL_COUNT; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = TERRAIN_LEAF_LINE_COUNT; xi < xend; ++xi )
			{
				*p = (unsigned short)(xi +  yi   *offsetCount); ++p;
				*p = (unsigned short)(xi + (yi+1)*offsetCount); ++p;

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mIndexBuffer8x8 = renderer->CreateIndexBuffer( mIndexCount8x8, pIndex );
		assert( mIndexBuffer8x8 );

		delete [] pIndex;
	}

	{
		/// 기본 인덱스 버퍼를 생성 4x4
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mIndexCount4x4 = (cellCnt+1)*2*cellCnt + (cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mIndexCount4x4];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
				*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}

		mIndexBuffer4x4 = renderer->CreateIndexBuffer( mIndexCount4x4, pIndex );
		assert( mIndexBuffer4x4 );

		delete[] pIndex;
	}

	{
		/// 기본 인덱스 버퍼를 생성 2x2
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mIndexCount2x2 = (cellCnt+1)*2*cellCnt + (cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mIndexCount2x2];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
				*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}

		mIndexBuffer2x2 = renderer->CreateIndexBuffer( mIndexCount2x2, pIndex );
		assert( mIndexBuffer2x2 );

		delete [] pIndex;
	}

	{
		/// 4x4 left crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[0] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[0]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( xi == 0 )
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)( (yi*2 + 1)*TERRAIN_BUFF_LINE_COUNT ); ++p;
					*p = (unsigned short)((xi+1)*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[0] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[0], pIndex );
		assert( mCrackIndexBuffer4x4[0] );

		delete [] pIndex;
	}
	{
		/// 4x4 right crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[1] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[1]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( xi == xend-1 )
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(((xi-1)*2) + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi*2+1)*TERRAIN_BUFF_LINE_COUNT); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[1] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[1], pIndex );
		assert( mCrackIndexBuffer4x4[1] );

		delete [] pIndex;
	}
	{
		/// 4x4 up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[2] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[2]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi+1 != xend )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*2+1) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[2] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[2], pIndex );
		assert( mCrackIndexBuffer4x4[2] );

		delete [] pIndex;
	}
	{
		/// 4x4 down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[3] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[3]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 && xi != 0 )
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)((xi*2-1) + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[3] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[3], pIndex );
		assert( mCrackIndexBuffer4x4[3] );

		delete [] pIndex;
	}
	{
		/// 4x4 left_up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[4] = (cellCnt+1)*2*cellCnt + (3*cellCnt-2)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[4]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((yi*2 + 1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)((xi*2+1) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else if( xi+1 < xend )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*2+1) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)( (yi*2 + 1)*TERRAIN_BUFF_LINE_COUNT ); ++p;
						*p = (unsigned short)((xi+1)*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[4] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[4], pIndex );
		assert( mCrackIndexBuffer4x4[4] );

		delete [] pIndex;
	}
	{
		/// 4x4 left_down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[5] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[5]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 )
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((yi*2 + 1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)((xi+1)*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*2-1) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)( (yi*2 + 1)*TERRAIN_BUFF_LINE_COUNT ); ++p;
						*p = (unsigned short)((xi+1)*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[5] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[5], pIndex );
		assert( mCrackIndexBuffer4x4[5] );

		delete [] pIndex;
	}
	{
		/// 4x4 right_up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[6] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[6]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*2) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi*2+1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*2+1) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*2) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi*2+1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[6] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[6], pIndex );
		assert( mCrackIndexBuffer4x4[6] );

		delete [] pIndex;
	}
	{
		/// 4x4 right_down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/2;
		mCrackIndexCount4x4[7] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount4x4[7]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 2*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 )
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*2-1) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi*2+1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(((xi+1)*2) +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*2+1) + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*2) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi*2+1)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*2 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*2 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer4x4[7] = renderer->CreateIndexBuffer( mCrackIndexCount4x4[7], pIndex );
		assert( mCrackIndexBuffer4x4[7] );

		delete [] pIndex;
	}

	//////////////////////////////////////////////////////////////////////////
	{
		/// 2x2 left crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[0] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[0]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( xi == 0 )
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)( (yi*4 + 2)*TERRAIN_BUFF_LINE_COUNT ); ++p;
					*p = (unsigned short)((xi+1)*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[0] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[0], pIndex );
		assert( mCrackIndexBuffer2x2[0] );

		delete [] pIndex;
	}
	{
		/// 2x2 right crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[1] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[1]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( xi == xend-1 )
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(((xi-1)*4) + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi*4+2)*TERRAIN_BUFF_LINE_COUNT); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[1] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[1], pIndex );
		assert( mCrackIndexBuffer2x2[1] );

		delete [] pIndex;
	}
	{
		/// 2x2 up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[2] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[2]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi+1 != xend )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*4+2) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[2] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[2], pIndex );
		assert( mCrackIndexBuffer2x2[2] );

		delete [] pIndex;
	}
	{
		/// 2x2 down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[3] = (cellCnt+1)*2*cellCnt +(cellCnt*2 -1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[3]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 && xi != 0 )
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)((xi*4-2) + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}
				else
				{
					*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[3] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[3], pIndex );
		assert( mCrackIndexBuffer2x2[3] );

		delete [] pIndex;
	}
	{
		/// 2x2 left_up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[4] = (cellCnt+1)*2*cellCnt + (3*cellCnt-2)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[4]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((yi*4 + 2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)((xi*4+2) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else if( xi+1 < xend )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*4+2) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)( (yi*4 + 2)*TERRAIN_BUFF_LINE_COUNT ); ++p;
						*p = (unsigned short)((xi+1)*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[4] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[4], pIndex );
		assert( mCrackIndexBuffer2x2[4] );

		delete [] pIndex;
	}
	{
		/// 2x2 left_down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[5] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[5]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 )
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((yi*4 + 2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)((xi+1)*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*4-2) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == 0 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)( (yi*4 + 2)*TERRAIN_BUFF_LINE_COUNT ); ++p;
						*p = (unsigned short)((xi+1)*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[5] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[5], pIndex );
		assert( mCrackIndexBuffer2x2[5] );

		delete [] pIndex;
	}
	{
		/// 2x2 right_up crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[6] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[6]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == 0 )
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*4) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi*4+2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)((xi*4+2) +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*4) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi*4+2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[6] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[6], pIndex );
		assert( mCrackIndexBuffer2x2[6] );

		delete [] pIndex;
	}
	{
		/// 2x2 right_down crack index 생성
		unsigned int cellCnt = TERRAIN_LEAF_CELL_COUNT/4;
		mCrackIndexCount2x2[7] = (cellCnt+1)*2*cellCnt + (3*cellCnt-1)*2;
		unsigned short* pIndex = new unsigned short[mCrackIndexCount2x2[7]];
		unsigned short* p = pIndex;

		unsigned int offsetCount = 4*TERRAIN_BUFF_LINE_COUNT;
		for( unsigned int yi = 0, yend = cellCnt; yi < yend; ++yi )
		{
			for( unsigned int xi = 0, xend = cellCnt+1; xi < xend; ++xi )
			{
				if( yi == yend-1 )
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*4-2) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi*4+2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(((xi+1)*4) +  yi   *offsetCount); ++p;
						*p = (unsigned short)((xi*4+2) + (yi+1)*offsetCount); ++p;
					}
				}
				else
				{
					if( xi == xend-1 )
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(((xi-1)*4) + (yi+1)*offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi*4+2)*TERRAIN_BUFF_LINE_COUNT); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
					else
					{
						*p = (unsigned short)(xi*4 +  yi   *offsetCount); ++p;
						*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					}
				}

				if( xi+1 >= xend && yi+1 < yend )
				{
					*p = (unsigned short)(xi*4 + (yi+1)*offsetCount); ++p;
					*p = (unsigned short)(     (yi+1)*offsetCount); ++p;
				}
			}
		}
		mCrackIndexBuffer2x2[7] = renderer->CreateIndexBuffer( mCrackIndexCount2x2[7], pIndex );
		assert( mCrackIndexBuffer2x2[7] );

		delete [] pIndex;
	}
}

cTerrain::~cTerrain()
{
	Clear();

	SAFE_RELEASE(mIndexBuffer8x8);
	SAFE_RELEASE(mIndexBuffer4x4);
	SAFE_RELEASE(mIndexBuffer2x2);

	for( unsigned int i=0;i<eCRACK_MAX;i++ )
	{
		SAFE_RELEASE(mCrackIndexBuffer4x4[i]);
		SAFE_RELEASE(mCrackIndexBuffer2x2[i]);
	}

	/// 정점 선언 및 인덱스 버퍼를 제거
	SAFE_RELEASE(mVertexDeclaration);

	mSingleton = 0;
}

void cTerrain::Clear()
{
	delete mRootNode;
	mRootNode = 0;
	NiDelete [] mAlphas;
	mAlphas = 0;
	NiDelete [] mColors;
	mColors = 0;
	NiDelete [] mNormals;
	mNormals = 0;
	delete [] mHeights;
	mHeights = 0;
	
	mNodeArray.Clear();
	mVisibleArray.Clear();

	/// 지형 텍스처들을 모두 제거
	for( unsigned int i = 0; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		delete mTextures[i];
		mTextures[i] = 0;
	}

	/// 정점 버퍼를 제거
	if( mBuffer )
	{
		unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;

		for( unsigned int i = 0; i < buffCount; ++i )
		{
			delete [] mBuffer[i];
		}
		delete [] mBuffer;
		mBuffer = 0;
	}

	/// 로딩 배열을 제거
	delete [] mLoadArray;
	mLoadArray = 0;
	mLoadCount = 0;
	mLoadIndex = 0;

	mCellCount = 0;
	mLineCount = 0;
	mMetersPerVertex = 0.0f;
	mUnitsPerMeter = 0;
	mUnitsPerVertex = 0.0f;
	mUnitsPerLeafNode = 0.0f;
	mSegmentLength = 0.0f;
}

void cTerrain::Process()
{
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
	frustum.EnableAllPlanes();
	cTerrainNode::mFrustum = &frustum;
	cTerrainNode::mVisibleArray = &mVisibleArray;
	mRootNode->Cull();

	/// 정렬
	if( mVisibleArray.GetSize() > 1 )
		::Sort( mVisibleArray.Begin(), mVisibleArray.End(), cTerrainNodeCompareForRendering() );

#if (defined(_DEVSYS) || defined(_DEBUG))
	if( DEVSYSTEM )
		DEVSYSTEM->mTerrainRenderCount = mVisibleArray.GetSize();
#endif
}

void cTerrain::Render()
{
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

	/// 장치 설정을 보관
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 장치를 설정
	device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	device->SetVertexDeclaration( mVertexDeclaration );

	/// 셰이더를 시작
	if( mShader.Begin( CAMERAMAN->GetCurrent(), renderer->GetD3DViewProj() ) == true )
	{
		mShader.SetAmbientLight( LIGHTAGENT->GetTerrainAmbientLight() );
		mShader.SetFog( FOGAGENT->GetFogProperty() );

		///
		cTerrainLeafNode* node = 0;
		cTerrainBuffer* buff = 0;
//		unsigned int tval = 0xFFFFFFFF;

		cTerrainTexture* tval[3] = {0,};

		unsigned int baseVertIndex = 0;
		int indexCount = 0;
		for( unsigned int i = 0, iend = mVisibleArray.GetSize(); i < iend; ++i )
		{
			node = (cTerrainLeafNode*)mVisibleArray[i];

			if( node->mLod == 0 )
			{
				device->SetIndices( mIndexBuffer8x8 );
				indexCount = mIndexCount8x8;
			}
			else if( node->mLod == 1 )
			{
				if( node->mCrackIndex == UINT_MAX )
				{
					device->SetIndices( mIndexBuffer4x4 );
					indexCount = mIndexCount4x4;
				}
				else
				{
					device->SetIndices( mCrackIndexBuffer4x4[node->mCrackIndex] );
					indexCount = mCrackIndexCount4x4[node->mCrackIndex];
				}
			}
			else if( node->mLod == 2 )
			{
				if( node->mCrackIndex == UINT_MAX )
				{
					device->SetIndices( mIndexBuffer2x2 );
					indexCount = mIndexCount2x2;
				}
				else
				{
					device->SetIndices( mCrackIndexBuffer2x2[node->mCrackIndex] );
					indexCount = mCrackIndexCount2x2[node->mCrackIndex];
				}
			}
			else
			{
				assert(0);
			}


			mShader.SetOrigin( node->mXIndex * mUnitsPerVertex, node->mYIndex * mUnitsPerVertex );
			mShader.SetPointLight( node->mLight );
			mShader.CommitChanges();

			if( buff != node->mBuffer )
			{
				buff = node->mBuffer;

				device->SetStreamSource( 0, node->mBuffer->mPosCoordBuffer, 0, sizeof(NiPoint3) );
				device->SetStreamSource( 1, node->mBuffer->mNormalBuffer, 0, sizeof(NiPoint3) );
				device->SetStreamSource( 2, node->mBuffer->mColorBuffer, 0, sizeof(NiColor) );
				device->SetStreamSource( 3, node->mBuffer->mAlphaBuffer, 0, sizeof(NiPoint3) );
			}
			if( tval[0] != node->mTexture0 )
			{
				tval[0] = node->mTexture0;
				device->SetTexture( 0, node->mTexture0->mTexture );
			}
			if( tval[1] != node->mTexture1 )
			{
				tval[1] = node->mTexture1;
				device->SetTexture( 1, node->mTexture1->mTexture );
			}
			if( tval[2] != node->mTexture2 )
			{
				tval[2] = node->mTexture2;
				device->SetTexture( 2, node->mTexture2->mTexture );
			}

/*			if( tval != node->mTextureValue1 )
			{
				tval = node->mTextureValue1;

				device->SetTexture( 0, node->mTexture0->mTexture );
				device->SetTexture( 1, node->mTexture1->mTexture );
				device->SetTexture( 2, node->mTexture2->mTexture );
			}
*/
			baseVertIndex = (node->mYIndex - buff->mYIndex) * TERRAIN_BUFF_LINE_COUNT + (node->mXIndex - buff->mXIndex);

			device->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, baseVertIndex, 0, TERRAIN_BUFF_VERT_COUNT, 0, indexCount - 2 );
		}

		mShader.End();
	}

	/// 장치 설정을 복원
	device->SetVertexDeclaration( oldVertDecl );
}

void cTerrain::AddTexture( unsigned int i, cTerrainTexture* tex )
{
	assert( tex );
	assert( i < TERRAIN_TEXTURE_COUNT );

	/// 텍스처 배열에 추가
	mTextures[i] = tex;
}

void cTerrain::RemoveTexture( unsigned int i )
{
	assert( i < TERRAIN_TEXTURE_COUNT );

	/// 텍스처 배열에서 제거
	cTerrainTexture* tex = mTextures[i];

	if( tex )
	{
		mTextures[i] = 0;
	}
}

bool cTerrain::CollideSphere( tArray<void*>* pickedArray, const cSphere& sphere )
{
	assert( pickedArray );
	assert( mRootNode );

	cTerrainNode::mSphere = &sphere;
	cTerrainNode::mPickedArray = pickedArray;
	return mRootNode->CollideSphere();
}

bool cTerrain::CheckCellCount( unsigned int cellCount )
{
	if( cellCount % TERRAIN_DEFAULT_RESOLUTION != 0 )
	{
		assert( 0 );
		return false;
	}
/*
	cTerrainLeafNode::mCellCount[0] = TERRAIN_LEAF_CELL_COUNT;
	cTerrainLeafNode::mLineCount[0] = TERRAIN_LEAF_LINE_COUNT;

	for( unsigned int i = 1; i < TERRAIN_LOD_COUNT; ++i )
	{
		cTerrainLeafNode::mCellCount[i] = cTerrainLeafNode::mCellCount[i-1] / 2;
		cTerrainLeafNode::mLineCount[i] = cTerrainLeafNode::mCellCount[i] + 1;
	}
*/
	return true;
}

void cTerrain::SetLeafNode( unsigned int xi, unsigned int yi, cTerrainLeafNode* node )
{
	assert( node );

	unsigned int nodeCount = mCellCount / TERRAIN_LEAF_CELL_COUNT;
	assert( xi >= 0 && xi < nodeCount );
	assert( yi >= 0 && yi < nodeCount );

	mNodeArray[yi * nodeCount + xi] = node;
}

cTerrainLeafNode* cTerrain::GetLeafNode( float x, float y ) const
{
	assert( mRootNode );

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
	int nodeCount = (int)(mCellCount / TERRAIN_LEAF_CELL_COUNT);

	if( xi < 0 )
		xi = 0;
	if( yi < 0 )
		yi = 0;
	if( xi > nodeCount - 1 )
		xi = nodeCount - 1;
	if( yi > nodeCount - 1 )
		yi = nodeCount - 1;

	return (cTerrainLeafNode*)mNodeArray[yi * nodeCount + xi];
}

const cBox& cTerrain::GetBoundBox() const
{
	assert( mRootNode );

	return mRootNode->GetBoundBox();
}

bool cTerrain::GetHeight( float* height, unsigned int xi, unsigned int yi )
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*height = mHeights[yi * mLineCount + xi];
	return true;
}

void cTerrain::ComputeNormals( unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend )
{
	assert( mHeights );
	assert( mNormals );

	NiPoint3 sum, v0, v1, v2;
	unsigned int count;

	for( unsigned int yi = ybegin; yi < yend; ++yi )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			/// 정점의 법선을 계산
			sum = NiPoint3::ZERO;
			v0.x = xi * mUnitsPerVertex;
			v0.y = yi * mUnitsPerVertex;
			v0.z = mHeights[yi * mLineCount + xi];
			count = 0;

			/// 우상 삼각형
			if( GetHeight( &v1.z, xi + 1, yi ) &&
				GetHeight( &v2.z, xi, yi + 1 ) )
			{
				v1.x = (xi + 1) * mUnitsPerVertex;
				v1.y = (yi    ) * mUnitsPerVertex;
				v2.x = (xi    ) * mUnitsPerVertex;
				v2.y = (yi + 1) * mUnitsPerVertex;

				sum += (v1 - v0).UnitCross( v2 - v0 );
				++count;
			}

			/// 좌상 삼각형
			if( GetHeight( &v1.z, xi, yi + 1 ) &&
				GetHeight( &v2.z, xi - 1, yi ) )
			{
				v1.x = (xi    ) * mUnitsPerVertex;
				v1.y = (yi + 1) * mUnitsPerVertex;
				v2.x = (xi - 1) * mUnitsPerVertex;
				v2.y = (yi    ) * mUnitsPerVertex;

				sum += (v1 - v0).UnitCross( v2 - v0 );
				++count;
			}

			/// 좌하 삼각형
			if( GetHeight( &v1.z, xi - 1, yi ) &&
				GetHeight( &v2.z, xi, yi - 1 ) )
			{
				v1.x = (xi - 1) * mUnitsPerVertex;
				v1.y = (yi    ) * mUnitsPerVertex;
				v2.x = (xi    ) * mUnitsPerVertex;
				v2.y = (yi - 1) * mUnitsPerVertex;

				sum += (v1 - v0).UnitCross( v2 - v0 );
				++count;
			}

			/// 우하 삼각형
			if( GetHeight( &v1.z, xi, yi - 1 ) &&
				GetHeight( &v2.z, xi + 1, yi ) )
			{
				v1.x = (xi    ) * mUnitsPerVertex;
				v1.y = (yi - 1) * mUnitsPerVertex;
				v2.x = (xi + 1) * mUnitsPerVertex;
				v2.y = (yi    ) * mUnitsPerVertex;

				sum += (v1 - v0).UnitCross( v2 - v0 );
				++count;
			}

			assert( count > 0 );
			sum /= float(count);
			sum.Unitize();
			mNormals[yi * mLineCount + xi] = sum;
		}
	}
}

unsigned int cTerrain::CalcLOD( float sqrDist )
{
	for( unsigned int j = 0; j < TERRAIN_LOD_COUNT; ++j )
	{
		if( sqrDist <= mSquaredDistanceLod[j] )
			return j;
	}
	return 2;
}
