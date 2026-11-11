#include "stdafx.h"
#include "TerrainNode.h"

#include "Terrain.h"
#include "RenderSystem.h"

bool cTerrainBranchNode::Load( cFileLoader& loader )
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

bool cTerrainLeafNode::Load( cFileLoader& loader )
{
	Clear();

	/// 경계 상자를 로딩
	if( loader.Read( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;

	/// 디테일 텍스처 인덱스를 로딩
	unsigned int ti0 = 0;
	unsigned int ti1 = 0;
	unsigned int ti2 = 0;
	loader.ReadUnsignedInt( &ti0 );
	loader.ReadUnsignedInt( &ti1 );
	loader.ReadUnsignedInt( &ti2 );

	if( ti0 >= TERRAIN_TEXTURE_COUNT || ti1 >= TERRAIN_TEXTURE_COUNT || ti2 >= TERRAIN_TEXTURE_COUNT )
	{
		assert( 0 );
		return false;
	}

	SetTextures( TERRAIN->GetTexture( ti0 ), TERRAIN->GetTexture( ti1 ), TERRAIN->GetTexture( ti2 ) );

	/// 버전별 로딩
	switch( mVersion )
	{
	case 2:
	case 3:
	case 4:
		{
			/// 높이 배열을 로딩
			float h;
			for( unsigned int yi = mYIndex, yend = mYIndex + mLineCount[0]; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + mLineCount[0]; xi < xend; ++xi )
				{
					loader.ReadFloat( &h );
					TERRAIN->SetHeight( xi, yi, h );
				}
			}

			/// 알파 배열을 로딩
			NiPoint3 a;
			for( unsigned int yi = mYIndex, yend = mYIndex + mLineCount[0]; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + mLineCount[0]; xi < xend; ++xi )
				{
					loader.Read( &a, sizeof(NiPoint3) );
					TERRAIN->SetAlpha( xi, yi, a );
				}
			}

			/// 색상 배열을 생성 및 로딩
			NiColorA c;
			for( unsigned int yi = mYIndex, yend = mYIndex + mLineCount[0]; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + mLineCount[0]; xi < xend; ++xi )
				{
					loader.Read( &c, sizeof(NiColorA) );
					TERRAIN->SetColor( xi, yi, NiColor(c.r, c.g, c.b) );
				}
			}

			/// 버퍼를 갱신
			mBuffer->UpdatePosCoord( TERRAIN, mXIndex, mYIndex, mLineCount[0] );
			mBuffer->UpdateAlpha( TERRAIN, mXIndex, mYIndex, mLineCount[0] );
			mBuffer->UpdateColor( TERRAIN, mXIndex, mYIndex, mLineCount[0] );
			break;
		}
	case 5:
	case 6:
		{
			/// 가시 여부를 로딩
			unsigned int visible = 0;
			loader.ReadUnsignedInt( &visible );
			mVisible = visible != 0;
			break;
		}
	}

	///
	/*for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		unsigned short numVerts = mNumVerts[i];
		unsigned int gridSize = mLineCount[i];
		NiPoint3* posCoords = mPosCoords[i] = NiNew NiPoint3[numVerts];
		NiPoint3* alphas = mAlphas[i] = NiNew NiPoint3[numVerts];
		NiColor* colors = mColors[i] = NiNew NiColor[numVerts];

		for( unsigned int j = 0, y = 0; y < gridSize; ++y )
		{
			for( unsigned int x = 0; x < gridSize; ++x, ++j )
			{
				posCoords[j] = mPosCoords[0][y * step * mLineCount[0] + x * step];
				alphas[j] = mAlphas[0][y * step * mLineCount[0] + x * step];
				colors[j] = mColors[0][y * step * mLineCount[0] + x * step];
			}
		}
	}
	*/

	/*// 삼각형 스트립을 생성
	for( unsigned int i = 0, step = 1; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		unsigned short numVerts = mNumVerts[i];
		NiPoint3* posCoords = mPosCoords[i];
		NiPoint3* alphas = mAlphas[i];
		NiColor* colors = mColors[i];

		/// 스트립 생성
		cTerrainStrip* tris = mTriStrips[i] = NiNew cTerrainStrip(
			numVerts,
			posCoords,
			alphas,
			colors,
			TERRAIN->GetTexCoords( i ),
			mNumTris[i],
			mNumStrips[i],
			TERRAIN->GetStripLength( i ),
			TERRAIN->GetStripIndex( i ),
			mCenter, mRadius );

		/// 렌더러에 미리 등록
		RENDERSYS->GetRenderer()->AddTerrainGeomToGroup( tris );
	}
	*/

	/*// 삼각형 리스트를 생성
	for( unsigned int i = 0; i < TERRAIN_LOD_COUNT; ++i )
	{
		unsigned int gridSize = mLineCount[i];
		unsigned short numTris = unsigned short(gridSize / 2);

		/// 위치 배열을 생성 및 설정
		mCrackPosCoords[i][0] = NiNew NiPoint3[gridSize];
		mCrackPosCoords[i][1] = NiNew NiPoint3[gridSize];
		mCrackPosCoords[i][2] = NiNew NiPoint3[gridSize];
		mCrackPosCoords[i][3] = NiNew NiPoint3[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackPosCoords[i][0][j] = mPosCoords[i][0 + j];
			mCrackPosCoords[i][1][j] = mPosCoords[i][(gridSize - 1) * gridSize + j];
			mCrackPosCoords[i][2][j] = mPosCoords[i][j * gridSize + 0];
			mCrackPosCoords[i][3][j] = mPosCoords[i][j * gridSize + gridSize - 1];
		}

		/// 알파 배열을 생성 및 설정
		mCrackAlphas[i][0] = NiNew NiPoint3[gridSize];
		mCrackAlphas[i][1] = NiNew NiPoint3[gridSize];
		mCrackAlphas[i][2] = NiNew NiPoint3[gridSize];
		mCrackAlphas[i][3] = NiNew NiPoint3[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackAlphas[i][0][j] = mAlphas[i][0 + j];
			mCrackAlphas[i][1][j] = mAlphas[i][(gridSize - 1) * gridSize + j];
			mCrackAlphas[i][2][j] = mAlphas[i][j * gridSize + 0];
			mCrackAlphas[i][3][j] = mAlphas[i][j * gridSize + gridSize - 1];
		}

		/// 색상 배열을 생성 및 설정
		mCrackColors[i][0] = NiNew NiColorA[gridSize];
		mCrackColors[i][1] = NiNew NiColorA[gridSize];
		mCrackColors[i][2] = NiNew NiColorA[gridSize];
		mCrackColors[i][3] = NiNew NiColorA[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			mCrackColors[i][0][j] = mColors[i][0 + j];
			mCrackColors[i][1][j] = mColors[i][(gridSize - 1) * gridSize + j];
			mCrackColors[i][2][j] = mColors[i][j * gridSize + 0];
			mCrackColors[i][3][j] = mColors[i][j * gridSize + gridSize - 1];
		}

		/// 텍스처 배열을 생성 및 설정
		NiPoint2* texArray = TERRAIN->GetTexCoords( i );

		/// 하
		NiPoint2* texCoords = mCrackTexCoords[i][0] = NiNew NiPoint2[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			texCoords[j] = texArray[0 + j];
		}

		/// 상
		texCoords = mCrackTexCoords[i][1] = NiNew NiPoint2[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			texCoords[j] = texArray[(gridSize-1) * gridSize + j];
		}

		/// 좌
		texCoords = mCrackTexCoords[i][2] = NiNew NiPoint2[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			texCoords[j] = texArray[j * gridSize + 0];
		}

		/// 우
		texCoords = mCrackTexCoords[i][3] = NiNew NiPoint2[gridSize];

		for( unsigned int j = 0; j < gridSize; ++j )
		{
			texCoords[j] = texArray[j * gridSize + gridSize - 1];
		}

		for( unsigned int j = 0; j < 4; ++j )
		{
			/// 리스트 배열을 생성 및 설정
			unsigned short* crackList = mCrackList[i][j] = NiAlloc( unsigned short, numTris * 3 );

			for( unsigned short k = 0, k2 = 0, kend = numTris * 3; k < kend; k += 3, k2 += 2 )
			{
				crackList[k + 0] = k2 + 0;
				crackList[k + 1] = k2 + 1;
				crackList[k + 2] = k2 + 2;
			}

			/// 리스트 생성
			NiTriShape* tris = mTriCracks[i][j] = NiNew NiTriShape(
				unsigned short(gridSize),
				mCrackPosCoords[i][j],
				mCrackAlphas[i][j],
				mCrackColors[i][j],
				mCrackTexCoords[i][j],
				1,
				NiGeometryData::NBT_METHOD_NONE,
				numTris,
				crackList );
		}
	}
	*/
	return true;
}
