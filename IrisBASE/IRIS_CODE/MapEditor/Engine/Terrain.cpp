#include "stdafx.h"
#include "Terrain.h"

#include "NaviMesh.h"
#include "Ray.h"
#include "Box.h"
#include "TerrainNode.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "LookAtCamera.h"

#ifdef MAP_EDITOR
#include "../HeightMap.h"
#include "../TerrainEditor.h"
#include "../MapEditorView.h"
#endif

cTerrainTexture::cTerrainTexture( unsigned int index, const cString& pathName, LPDIRECT3DTEXTURE9 tex )
: mIndexByTerrain( index )
, mRefCount( 0 )
, mTexture( tex )
{
	assert( tex );

	::GetFileName( &mFileName, pathName );

	///
	if( TERRAIN )
		TERRAIN->AddTexture( index, this );

	/// 이미지를 로딩
	/// 텍스처 색상값을 참조하는데 사용한다.
	cTargaImage& img = mImageData;

	if( img.Load( pathName ) == false )
	{
		img.Resize( img.GetWidth(), img.GetHeight(), 3 );
	}
	else
	{
		img.ChangeColorByteOrder();

		/// 이미지로부터 장치 독립 비트맵을 생성
		BITMAPINFO bi;
		bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bi.bmiHeader.biWidth = img.GetWidth();
		bi.bmiHeader.biHeight = img.GetHeight();
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = (WORD)(img.GetBytesPerPixel() * 8);
		bi.bmiHeader.biCompression = 0;
		bi.bmiHeader.biSizeImage = (DWORD)img.GetSize();
		bi.bmiHeader.biXPelsPerMeter = 0;
		bi.bmiHeader.biYPelsPerMeter = 0;
		bi.bmiHeader.biClrUsed = 0;
		bi.bmiHeader.biClrImportant = 0;
		bi.bmiColors[0].rgbBlue = 0;
		bi.bmiColors[0].rgbGreen = 0;
		bi.bmiColors[0].rgbRed = 0;
		bi.bmiColors[0].rgbReserved = 0;

		void* p = 0;
		HDC memdc = CreateCompatibleDC( 0 );
		HBITMAP hbmp = CreateDIBSection( memdc, &bi, DIB_RGB_COLORS, &p, 0, 0 );

		if( p )
			::memcpy( p, img.GetBufferPtr(), img.GetSize() );
		DeleteDC( memdc );

		/// 비트맵 부착
		mBitmap.Attach( hbmp );

		img.ChangeColorByteOrder();
	}
}

cTerrainTexture::~cTerrainTexture()
{
	mBitmap.DeleteObject();

	if( mTexture )
	{
		mTexture->Release();
		mTexture = 0;
	}

	if( TERRAIN && mIndexByTerrain != 0 )
		TERRAIN->RemoveTexture( mIndexByTerrain );
}


void cTerrainTexture::Grab()
{
	if( mIndexByTerrain != 0 )
		++mRefCount;
	else
		mRefCount = 1;
}

void cTerrainTexture::Drop()
{
	assert( mRefCount > 0 && "bad reference counting!" );

	if( mIndexByTerrain != 0 && --mRefCount == 0 )
		delete this;
}


bool cTerrainTexture::GetColor( float* red, float* green, float* blue, float x, float y )
{
	unsigned char r, g, b;

	if( mImageData.GetPixel( &r, &g, &b, x, y ) )
	{
		*red = r / 255.0f;
		*green = g / 255.0f;
		*blue = b / 255.0f;
		return true;
	}
	else
	{
		return false;
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
	else
	{
		assert( 0 );
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
	else
	{
		assert( 0 );
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
	else
	{
		assert( 0 );
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
	else
	{
		assert( 0 );
	}
	mAlphaBuffer->Unlock();
}

cTerrain* cTerrain::mSingleton = 0;

cTerrain::cTerrain()
: mInited( false )
, mCellCount( 0 )
, mLineCount( 0 )
, mMetersPerVertex( 0.0f )
, mUnitsPerMeter( 0 )
, mUnitsPerVertex( 0.0f )
, mUnitsPerLeafNode( 0.0f )
, mRootNode( 0 )
, mHeights( 0 )
, mNormals( 0 )
, mColors( 0 )
, mAlphas( 0 )
, mPaintAlphas( 0 )
, mLodEnabled( false )
, mNumTextures( 0 )
, mTextureMap( TERRAIN_TEXTURE_COUNT )
, mBuffer( 0 )
, mVertexDeclaration0( 0 )
, mVertexDeclaration1( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	for( unsigned int i = 0; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		mTextures[i] = 0;
	}

	/// 안개 속성을 생성
	mFogProp = NiNew NiFogProperty;
	mFogProp->SetFog( true );
	mFogProp->SetFogColor( NiColor::WHITE );
	mFogProp->SetDepth( 0.1f );

	/// 주변광을 생성
	mAmbientLight = NiNew NiAmbientLight;
	mAmbientLight->SetAmbientColor( NiColor::WHITE );
	mAmbientLight->SetDiffuseColor( NiColor::WHITE );
	mAmbientLight->SetDimmer( 1.0f );

	/// Lod 거리 설정
	float dist = 15000.0f;

	for( unsigned int i = 0, iend = TERRAIN_LOD_COUNT - 1; i < iend; ++i )
	{
		mSquaredDistanceLod[i] = dist * dist;
		dist *= 1.5f;
	}
	mSquaredDistanceLod[TERRAIN_LOD_COUNT-1] = NI_INFINITY;

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

#ifdef MAP_EDITOR
	mViewMode = eTERRAIN_VIEW_TEXTURED;
	mModified = false;
#endif
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

	if( mTextures[0] != 0 )
	{
		delete mTextures[0];
		mTextures[0] = 0;
	}

	if( mIndexBuffer8x8 )
	{
		mIndexBuffer8x8->Release();
		mIndexBuffer8x8 = 0;
	}
	if( mIndexBuffer4x4 )
	{
		mIndexBuffer4x4->Release();
		mIndexBuffer4x4 = 0;
	}
	if( mIndexBuffer2x2 )
	{
		mIndexBuffer2x2->Release();
		mIndexBuffer2x2 = 0;
	}

	for( unsigned int i=0;i<eCRACK_MAX;i++ )
	{
		if( mCrackIndexBuffer4x4[i] )
		{
			mCrackIndexBuffer4x4[i]->Release();
			mCrackIndexBuffer4x4[i] = 0;
		}
		if( mCrackIndexBuffer2x2[i] )
		{
			mCrackIndexBuffer2x2[i]->Release();
			mCrackIndexBuffer2x2[i] = 0;
		}
	}

	mSingleton = 0;
}

void cTerrain::Clear()
{
	mInited = false;

	delete mRootNode;
	mRootNode = 0;
	NiDelete [] mPaintAlphas;
	mPaintAlphas = 0;
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
	for( unsigned int i = 1; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		delete mTextures[i];
		mTextures[i] = 0;
	}
	mTextureMap.Clear();

	/// 정점 선언 및 인덱스 버퍼를 제거
	if( mVertexDeclaration0 )
	{
		mVertexDeclaration0->Release();
		mVertexDeclaration0 = 0;
	}
	if( mVertexDeclaration1 )
	{
		mVertexDeclaration1->Release();
		mVertexDeclaration1 = 0;
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

	///
	mCellCount = 0;
	mLineCount = 0;
	mMetersPerVertex = 0.0f;
	mUnitsPerMeter = 0;
	mUnitsPerVertex = 0.0f;
	mUnitsPerLeafNode = 0.0f;
}

void cTerrain::Init( unsigned int cellCount, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();

	/// 기본 텍스처를 로딩
#ifdef MAP_EDITOR
	LoadTexture( 0, "MapData/detail0.tga" );
#else
	LoadTexture( 0, "Data/2DData/detail0.tga" );
#endif

	/// 그리드 크기를 검사
	if( CheckCellCount( cellCount ) == false )
	{
		assert( 0 && "invalid terrain grid size" );
		return;
	}

	/// 단위 설정
	mCellCount = cellCount;
	mLineCount = mCellCount + 1;
	mMetersPerVertex = metersPerVertex;
	mUnitsPerMeter = unitsPerMeter;
	mUnitsPerVertex = metersPerVertex * (float)unitsPerMeter;
	mUnitsPerLeafNode = cTerrainLeafNode::mCellCount[0] * mUnitsPerVertex;

	/// 높이맵 생성 및 초기화
	unsigned int numVerts = mLineCount * mLineCount;
	mHeights = new float[numVerts];

	for( unsigned int i = 0, iend = numVerts; i < iend; ++i )
	{
		mHeights[i] = 0.0f;
	}

	/// 법선맵 생성 및 초기화
	mNormals = NiNew NiPoint3[numVerts];

	for( unsigned int i = 0, iend = numVerts; i < iend; ++i )
	{
		mNormals[i] = NiPoint3( 0.0f, 0.0f, 1.0f );
	}

	/// 색상맵 생성 및 초기화
	mColors = NiNew NiColor[numVerts];

	for( unsigned int i = 0, iend = numVerts; i < iend; ++i )
	{
		mColors[i] = NiColor::WHITE;
	}

	/// 알파맵 생성 및 초기화
	mAlphas = NiNew NiPoint3[numVerts];
	mPaintAlphas = NiNew NiPoint3[numVerts];

	for( unsigned int i = 0, iend = numVerts; i < iend; ++i )
	{
		mAlphas[i] = NiPoint3( 1.0f, 0.0f, 0.0f );
		mPaintAlphas[i] = NiPoint3( 1.0f, 0.0f, 0.0f );
	}

	/// 버퍼 생성 및 초기화
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	mBuffer = new cTerrainBuffer*[buffCount];

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		mBuffer[yi] = new cTerrainBuffer[buffCount];
	}

	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].Init( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT );
		}
	}

	/// 리프 노드 배열 생성
	unsigned int leafCount = mCellCount / TERRAIN_LEAF_CELL_COUNT;
	unsigned int numNodes = leafCount * leafCount;
	mNodeArray.Resize( numNodes );

	/// 가시 노드 배열의 용량을 설정
	mVisibleArray.Reserve( numNodes );

	/// 루트 노드 생성 및 초기화
	mRootNode = new cTerrainBranchNode( 0, 0, 0, mCellCount );
	mRootNode->SyncHeight();
	mRootNode->SyncAlpha();
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


	mInited = true;
#ifdef MAP_EDITOR
	mModified = false;
#endif
}

void cTerrain::Process( bool /*testing*/ )
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
	frustum.EnableAllPlanes();
	cTerrainNode::mFrustum = &frustum;
	cTerrainNode::mVisibleArray = &mVisibleArray;
	mRootNode->Cull();

	/// 정렬
	if( mVisibleArray.GetSize() > 2 )
		::Sort( mVisibleArray.Begin(), mVisibleArray.End(), cTerrainNodeCompareForRendering() );
}

void cTerrain::Render()
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

	///
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();

#ifdef MAP_EDITOR
	/// 렌더링
	switch( mViewMode )
	{
	case eTERRAIN_VIEW_WIREFRAME:
		{
			device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			Render0( device, renderer->GetD3DViewProj() );
			break;
		}
	case eTERRAIN_VIEW_SOLID:
		{
			device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			Render1( device, renderer->GetD3DViewProj() );
			break;
		}
	case eTERRAIN_VIEW_TEXTURED:
		{
			device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			Render0( device, renderer->GetD3DViewProj() );
			break;
		}
	case eTERRAIN_VIEW_WIRETEXTURED:
		{
			device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			Render0( device, renderer->GetD3DViewProj() );

			device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			Render1( device, renderer->GetD3DViewProj() );
			break;
		}
	}


	/// 렌더링 상태를 복원
	device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#else
	{
		//device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		Render0( device, renderer->GetD3DViewProj() );
	}
#endif /// MAP_EDITOR
}

void cTerrain::Render0( LPDIRECT3DDEVICE9 device, const D3DXMATRIX& worldViewProj )
{
	assert( device );

	/// 장치 설정을 백업
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 장치를 설정
	device->SetVertexDeclaration( mVertexDeclaration0 );

	/// 셰이더를 시작
	if( mShader0.Begin( CAMERAMAN->GetCurrent(), worldViewProj ) == false )
		return;

	mShader0.SetAmbientLight( mAmbientLight );
	mShader0.SetFog( mFogProp );

	///
	cTerrainLeafNode* node = 0;
	cTerrainBuffer* buff = 0;
	unsigned int tval = 0xFFFFFFFF;
	unsigned int baseVertIndex = 0;

	int indexCount = 0;

#ifdef MAP_EDITOR
	if( VIEW->IsPlayerMode() == false )
#endif
	{
		device->SetIndices( mIndexBuffer8x8 );
		indexCount = mIndexCount8x8;
	}

	for( unsigned int i = 0, iend = mVisibleArray.GetSize(); i < iend; ++i )
	{
		node = mVisibleArray[i];

#ifdef MAP_EDITOR
		if( VIEW->IsPlayerMode() == true )
		{
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
		}
#endif
		mShader0.SetOrigin( node->mXIndex * mUnitsPerVertex, node->mYIndex * mUnitsPerVertex );
		mShader0.SetPointLight( node->mLight );
		mShader0.CommitChanges();

		if( buff != node->mBuffer )
		{
			buff = node->mBuffer;

			device->SetStreamSource( 0, node->mBuffer->mPosCoordBuffer, 0, sizeof(NiPoint3) );
			device->SetStreamSource( 1, node->mBuffer->mNormalBuffer, 0, sizeof(NiPoint3) );
			device->SetStreamSource( 2, node->mBuffer->mColorBuffer, 0, sizeof(NiColor) );
			device->SetStreamSource( 3, node->mBuffer->mAlphaBuffer, 0, sizeof(NiPoint3) );
		}
		if( tval != node->mTextureValue1 )
		{
			tval = node->mTextureValue1;

			device->SetTexture( 0, node->GetD3DTexture0() );
			device->SetTexture( 1, node->GetD3DTexture1() );
			device->SetTexture( 2, node->GetD3DTexture2() );
		}

		baseVertIndex = (node->mYIndex - buff->mYIndex) * TERRAIN_BUFF_LINE_COUNT + (node->mXIndex - buff->mXIndex);

		device->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, baseVertIndex, 0, TERRAIN_BUFF_VERT_COUNT, 0,  indexCount - 2 );
	}

	mShader0.End();

	/// 장치 설정을 복원
	if( oldVertDecl )
		device->SetVertexDeclaration( oldVertDecl );
}

void cTerrain::Render1( LPDIRECT3DDEVICE9 device, const D3DXMATRIX& worldViewProj )
{
	assert( device );

	/// 장치 설정을 백업
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 장치를 설정
	device->SetVertexDeclaration( mVertexDeclaration1 );

	/// 셰이더를 시작
	if( mShader1.Begin( CAMERAMAN->GetCurrent(), worldViewProj ) == false )
		return;

	mShader1.SetAmbientLight( mAmbientLight );
	mShader1.SetFog( mFogProp );
	mShader1.CommitChanges();

	///
	cTerrainLeafNode* node = 0;
	cTerrainBuffer* buff = 0;
	unsigned int baseVertIndex = 0;
	int indexCount = 0;

#ifdef MAP_EDITOR
	if( VIEW->IsPlayerMode() == false )
#endif
	{
		device->SetIndices( mIndexBuffer8x8 );
		indexCount = mIndexCount8x8;
	}


	for( unsigned int i = 0, iend = mVisibleArray.GetSize(); i < iend; ++i )
	{
		node = mVisibleArray[i];

#ifdef MAP_EDITOR
		if( VIEW->IsPlayerMode() == true )
		{
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
		}
#endif

		if( buff != node->mBuffer )
		{
			buff = node->mBuffer;

			device->SetStreamSource( 0, node->mBuffer->mPosCoordBuffer, 0, sizeof(NiPoint3) );
			device->SetStreamSource( 1, node->mBuffer->mColorBuffer, 0, sizeof(NiColor) );
		}

		baseVertIndex = (node->mYIndex - buff->mYIndex) * TERRAIN_BUFF_LINE_COUNT + (node->mXIndex - buff->mXIndex);

		device->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, baseVertIndex, 0, TERRAIN_BUFF_VERT_COUNT, 0,  indexCount - 2 );
	}

	mShader1.End();

	/// 장치 설정을 복원
	if( oldVertDecl )
		device->SetVertexDeclaration( oldVertDecl );
}

bool cTerrain::Pick( NiPoint3* pos, int mouseX, int mouseY )
{
	assert( pos );
	assert( mRootNode );

	NiPoint3 origin, dir;
	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );
	return Pick( pos, cRay(origin, dir) );
}

bool cTerrain::Pick( NiPoint3* pos, const cRay& ray )
{
	float dist = NI_INFINITY;
	cTerrainNode::mRay = &ray;
	cTerrainNode::mDistance = &dist;
	cTerrainNode::mContact = pos;
	return mRootNode->CollideRay();
}

bool cTerrain::CollideSphere( tArray<cTerrainLeafNode*>* pickedArray, const cSphere& sphere )
{
	assert( pickedArray );
	assert( mRootNode );

	cTerrainNode::mSphere = &sphere;
	cTerrainNode::mPickedArray = pickedArray;
	return mRootNode->CollideSphere();
}

#ifdef MAP_EDITOR
void cTerrain::SetViewMode( eTerrainViewMode viewMode )
{
	mViewMode = viewMode;
}

void cTerrain::ClearHeightMap()
{
	for( unsigned int i = 0, iend = mLineCount*mLineCount; i < iend; ++i )
	{
		mHeights[i] = 0.0f;
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncHeight();
}

bool cTerrain::ImportHeightMap( const cString& pathName, float scale )
{
	cTargaImage img;

	if( img.Load( pathName ) == false )
		return false;

	scale *= 100.0f;

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			float x = float(xi) / float(mLineCount-1);
			float y = float(yi) / float(mLineCount-1);
			unsigned char red = 0;

			img.GetPixel( &red, 0, 0, x, y );
			mHeights[i] = red * scale;
		}
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncHeight();
	return true;
}

bool cTerrain::ImportHeightMap( const cString& pathName )
{
	cHeightMap hmp;

	if( hmp.Load( pathName ) == false )
		return false;

	if( mLineCount == hmp.GetWidth() && mLineCount == hmp.GetHeight() )
	{
		for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
		{
			for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
			{
				hmp.GetValue( &(mHeights[i]), xi, yi );
			}
		}
	}
	else
	{
		for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
		{
			for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
			{
				float x = float(xi) / float(mLineCount-1);
				float y = float(yi) / float(mLineCount-1);
				hmp.GetValue( &(mHeights[i]), x, y );
			}
		}
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncHeight();
	return true;
}

bool cTerrain::ExportHeightMap( const cString& pathName )
{
	cString ext;
	::GetFileExtension( &ext, pathName );

	if( ext == "tga" )
	{
		cTargaImage img;
		img.Resize( mLineCount, mLineCount, 1 );

		const cBox& box = mRootNode->GetBoundBox();
		float maxHeight = box.GetMax().z - box.GetMin().z;
		float minHeight = box.GetMin().z;
		float z;

		for( unsigned int yi = 0; yi < mLineCount; ++yi )
		{
			for( unsigned int xi = 0; xi < mLineCount; ++xi )
			{
				z = (mHeights[yi * mLineCount + xi] - minHeight) / maxHeight * 255.0f;
				img.SetPixel( xi, yi, (unsigned char)z, 0, 0 );
			}
		}

		return img.Save( pathName );
	}
	else if( ext == "hmap" )
	{
		cHeightMap hmp( mLineCount, mLineCount );

		for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
		{
			for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
			{
				hmp.SetValue( xi, yi, mHeights[i] );
			}
		}

		return hmp.Save( pathName );
	}
	else
	{
		return false;
	}
}

void cTerrain::ClearAlphaMap()
{
	for( unsigned int i = 0, iend = mLineCount*mLineCount; i < iend; ++i )
	{
		mAlphas[i].x = 1.0f;
		mAlphas[i].y = 0.0f;
		mAlphas[i].z = 0.0f;
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdateAlpha( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncAlpha();
}

bool cTerrain::ImportAlphaMap( const cString& pathName )
{
	cTargaImage img;

	if( img.Load( pathName ) == false )
	{
		return false;
	}

	unsigned int imgWidth = img.GetWidth();
	unsigned int imgHeight = img.GetHeight();
	unsigned char red = 0, green = 0, blue = 0;
	float x, y;

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			x = float(xi) / float(mLineCount-1);
			y = float(yi) / float(mLineCount-1);
			img.GetPixel( &red, &green, &blue, 0, (unsigned int)(x * (imgWidth-1)), (unsigned int)(y * (imgHeight-1)) );
			mAlphas[i].x = red / 255.0f;
			mAlphas[i].y = green / 255.0f;
			mAlphas[i].z = blue / 255.0f;
		}
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdateAlpha( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncAlpha();
	return true;
}

bool cTerrain::ExportAlphaMap( const cString& pathName )
{
	cTargaImage img;

	img.Resize( mLineCount, mLineCount, 4 );

	float r, g, b;

	for( unsigned int yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi )
		{
			NiPoint3& a = mAlphas[yi * mLineCount + xi];
			r = a.x * 255.0f;
			g = a.y * 255.0f;
			b = a.z * 255.0f;
			img.SetPixel( xi, yi, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 );
		}
	}

	return img.Save( pathName );
}

void cTerrain::ClearColorMap()
{
	for( unsigned int i = 0, iend = mLineCount*mLineCount; i < iend; ++i )
	{
		mColors[i].r = 1.0f;
		mColors[i].g = 1.0f;
		mColors[i].b = 1.0f;
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdateColor( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncColor();
}

bool cTerrain::ImportColorMap( const cString& pathName )
{
	cTargaImage img;

	if( img.Load( pathName ) == false )
	{
		return false;
	}

	unsigned int imgWidth = img.GetWidth();
	unsigned int imgHeight = img.GetHeight();
	unsigned char red = 0, green = 0, blue = 0, alpha = 0;
	float x, y;

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			x = float(xi) / float(mLineCount-1);
			y = float(yi) / float(mLineCount-1);
			img.GetPixel( &red, &green, &blue, &alpha, (unsigned int)(x * (imgWidth-1)), (unsigned int)(y * (imgHeight-1)) );
			mColors[i].r = (red / 255.0f);
			mColors[i].g = (green / 255.0f);
			mColors[i].b = (blue / 255.0f);
		}
	}

	/// 버퍼를 갱신
	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdateColor( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	/// 노드를 갱신
	mRootNode->SyncColor();
	return true;
}

bool cTerrain::ExportColorMap( const cString& pathName )
{
	cTargaImage img;

	img.Resize( mLineCount, mLineCount, 4 );

	float r, g, b;

	for( unsigned int yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi )
		{
			NiColor& c = mColors[yi * mLineCount + xi];
			r = c.r * 255.0f;
			g = c.g * 255.0f;
			b = c.b * 255.0f;
			img.SetPixel( xi, yi, (unsigned char)r, (unsigned char)g, (unsigned char)b );
		}
	}

	return img.Save( pathName );
}

void cTerrain::AdjustHeight( float scale )
{
	if( scale == 1.0f )
		return;

	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			mHeights[i] *= scale;
		}
	}

	mRootNode->SyncHeight();
	mModified = true;
}

void cTerrain::SyncAllToNaviMesh()
{
	for( unsigned int i = 0, yi = 0; yi < mLineCount; ++yi )
	{
		for( unsigned int xi = 0; xi < mLineCount; ++xi, ++i )
		{
			NAVIMESH->GetHeight( &(mHeights[i]), xi, yi );
		}
	}

	unsigned int buffCount = mCellCount / TERRAIN_BUFF_CELL_COUNT;
	for( unsigned int yi = 0; yi < buffCount; ++yi )
	{
		for( unsigned int xi = 0; xi < buffCount; ++xi )
		{
			mBuffer[yi][xi].UpdatePosCoord( this, xi * TERRAIN_BUFF_CELL_COUNT, yi * TERRAIN_BUFF_CELL_COUNT, TERRAIN_BUFF_LINE_COUNT );
		}
	}

	mRootNode->SyncHeight();
	mModified = true;
}

void cTerrain::SetDetailTextures( cTerrainLeafNode* n, unsigned int index0, unsigned int index1, unsigned int index2 )
{
	assert( n );
	assert( index0 < TERRAIN_TEXTURE_COUNT );
	assert( index1 < TERRAIN_TEXTURE_COUNT );
	assert( index2 < TERRAIN_TEXTURE_COUNT );

	n->SetTextures( mTextures[index0], mTextures[index1], mTextures[index2] );
}

void cTerrain::SetDetailTexturesToAll( unsigned int index0, unsigned int index1, unsigned int index2 )
{
	assert( index0 < TERRAIN_TEXTURE_COUNT );
	assert( index1 < TERRAIN_TEXTURE_COUNT );
	assert( index2 < TERRAIN_TEXTURE_COUNT );

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		cTerrainLeafNode* n = mNodeArray[i];

		n->SetTextures( mTextures[index0], mTextures[index1], mTextures[index2] );
	}
}
#endif /// MAP_EDITOR

void cTerrain::AddTexture( unsigned int i, cTerrainTexture* tex )
{
	assert( tex );
	assert( mNumTextures < TERRAIN_TEXTURE_COUNT );

	/// 텍스처 배열에 추가
	mTextures[i] = tex;

	/// 텍스처 맵에 추가
	mTextureMap.Insert( tex->mFileName, tex->mTexture );

	if( i != 0 )
		++mNumTextures;
}

void cTerrain::RemoveTexture( unsigned int i )
{
	assert( i < TERRAIN_TEXTURE_COUNT );
	assert( mNumTextures );

	if( i == 0 )
		return;

	/// 텍스처 배열에서 제거
	cTerrainTexture* tex = mTextures[i];

	if( tex )
	{
		mTextures[i] = 0;
		--mNumTextures;

		/// 텍스처 맵에서 제거
		mTextureMap.Erase( tex->mFileName );
	}
}

bool cTerrain::CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius )
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
	int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

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

bool cTerrain::CalcRangeTight( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius )
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
	int xb = (int)((pos.x - outerRadius) / mUnitsPerLeafNode);
	int yb = (int)((pos.y - outerRadius) / mUnitsPerLeafNode);
	int xe = (int)((pos.x + outerRadius) / mUnitsPerLeafNode) + 1;
	int ye = (int)((pos.y + outerRadius) / mUnitsPerLeafNode) + 1;
	int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

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

bool cTerrain::CheckCellCount( unsigned int cellCount )
{
	if( cellCount % TERRAIN_DEFAULT_RESOLUTION != 0 )
	{
		assert( 0 );
		return false;
	}

	cTerrainLeafNode::mCellCount[0] = TERRAIN_LEAF_CELL_COUNT;
	cTerrainLeafNode::mLineCount[0] = TERRAIN_LEAF_LINE_COUNT;

	for( unsigned int i = 1; i < TERRAIN_LOD_COUNT; ++i )
	{
		cTerrainLeafNode::mCellCount[i] = cTerrainLeafNode::mCellCount[i-1] / 2;
		cTerrainLeafNode::mLineCount[i] = cTerrainLeafNode::mCellCount[i] + 1;
	}
	return true;
}

void cTerrain::SetAmbientLightAmbient( const NiColor& color )
{
	mAmbientLight->SetAmbientColor( color );
}

void cTerrain::SetAmbientLightDiffuse( const NiColor& color )
{
	mAmbientLight->SetDiffuseColor( color );
}

void cTerrain::SetAmbientLightDimmer( float dimmer )
{
	mAmbientLight->SetDimmer( dimmer );
}

void cTerrain::SetFog( bool enabled, const NiColor& color, float depth )
{
	mFogProp->SetFog( enabled );
	mFogProp->SetFogColor( color );
	mFogProp->SetDepth( depth );
}

void cTerrain::SetFogColor( const NiColor& color )
{
	mFogProp->SetFogColor( color );
}

void cTerrain::SetFogDepth( float depth )
{
	mFogProp->SetDepth( depth );
}

void cTerrain::SetLeafNode( unsigned int xi, unsigned int yi, cTerrainLeafNode* node )
{
	assert( node );

	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];
	mNodeArray[yi * nodeCount + xi] = node;
}

cTerrainLeafNode* cTerrain::GetLeafNode( float x, float y ) const
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
	int nodeCount = (int)(mCellCount / cTerrainLeafNode::mCellCount[0]);

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

void cTerrain::SetHeight( unsigned int xi, unsigned int yi, float height )
{
	mHeights[yi * mLineCount + xi] = height;
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

float cTerrain::GetHeight( float x, float y ) const
{
	int xi = (int)(x / mUnitsPerVertex);
	if( xi < 0 || xi >= (int)mLineCount )
	{
		assert( 0 && "x pos out of range" );
		return 0.0f;
	}

	int yi = (int)(y / mUnitsPerVertex);
	if( yi < 0 || yi >= (int)mLineCount )
	{
		assert( 0 && "y pos out of range" );
		return 0.0f;
	}
	return mHeights[yi * mLineCount + xi];
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

bool cTerrain::GetAlpha( NiPoint3* alpha, unsigned int xi, unsigned int yi )
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*alpha = mAlphas[yi * mLineCount + xi];
	return true;
}

bool cTerrain::GetPaintAlpha( NiPoint3* alpha, unsigned int xi, unsigned int yi )
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*alpha = mPaintAlphas[yi * mLineCount + xi];
	return true;
}


bool cTerrain::GetColor( NiColor* color, unsigned int xi, unsigned int yi )
{
	if( xi < 0 || xi > mCellCount )
		return false;
	if( yi < 0 || yi > mCellCount )
		return false;

	*color = mColors[yi * mLineCount + xi];
	return true;
}

bool cTerrain::GetColor( NiColor* color, const NiPoint3& pos ) const
{
	float x = pos.x;
	float y = pos.y;

	/// 해당 위치가 지형 안에 들어오는지 검사
	const cBox& box = mRootNode->GetBoundBox();

	if( x < box.GetMin().x || y < box.GetMin().y ||
		x > box.GetMax().x || y > box.GetMax().y )
	{
		return false;
	}

	x += 50.0f;
	y += 50.0f;
	unsigned int xi = (unsigned int)(x / mUnitsPerVertex);
	unsigned int yi = (unsigned int)(y / mUnitsPerVertex);

	*color = mColors[yi * mLineCount + xi];
	return true;
}

const cBox& cTerrain::GetBoundBox() const
{
	return mRootNode->GetBoundBox();
}

unsigned int cTerrain::GetLeafCellCount() const
{
	return cTerrainLeafNode::mCellCount[0];
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
