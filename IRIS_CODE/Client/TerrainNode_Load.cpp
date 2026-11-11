#include "stdafx.h"
#include "TerrainNode.h"

void cTerrainBranchNode::AddToLoadArray( cTerrainNode** loadArray, unsigned int& count )
{
	if( count >= cTerrain::MAX_NODES )
	{
		assert( 0 );
		return;
	}
	else
	{
		loadArray[count] = this;
		++count;
	}

	mChild[0]->AddToLoadArray( loadArray, count );
	mChild[1]->AddToLoadArray( loadArray, count );
	mChild[2]->AddToLoadArray( loadArray, count );
	mChild[3]->AddToLoadArray( loadArray, count );
}

bool cTerrainBranchNode::Load( cFileLoader& loader )
{
	/// 경계 상자를 로딩
	if( loader.Read( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	mCenter = (mBoundBox.GetMin() + mBoundBox.GetMax()) * 0.5f;
	mRadius = (mBoundBox.GetMax() - mBoundBox.GetMin()).Length() * 0.5f;
	return true;
}

void cTerrainLeafNode::AddToLoadArray( cTerrainNode** loadArray, unsigned int& count )
{
	if( count >= cTerrain::MAX_NODES )
	{
		assert( 0 );
		return;
	}
	else
	{
		loadArray[count] = this;
		++count;
	}
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
			for( unsigned int yi = mYIndex, yend = mYIndex + TERRAIN_LEAF_LINE_COUNT; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + TERRAIN_LEAF_LINE_COUNT; xi < xend; ++xi )
				{
					loader.ReadFloat( &h );
					TERRAIN->SetHeight( xi, yi, h );
				}
			}

			/// 알파 배열을 로딩
			NiPoint3 a;
			for( unsigned int yi = mYIndex, yend = mYIndex + TERRAIN_LEAF_LINE_COUNT; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + TERRAIN_LEAF_LINE_COUNT; xi < xend; ++xi )
				{
					loader.Read( &a, sizeof(NiPoint3) );
					TERRAIN->SetAlpha( xi, yi, a );
				}
			}

			/// 색상 배열을 생성 및 로딩
			NiColorA c;
			for( unsigned int yi = mYIndex, yend = mYIndex + TERRAIN_LEAF_LINE_COUNT; yi < yend; ++yi )
			{
				for( unsigned int xi = mXIndex, xend = mXIndex + TERRAIN_LEAF_LINE_COUNT; xi < xend; ++xi )
				{
					loader.Read( &c, sizeof(NiColorA) );
					TERRAIN->SetColor( xi, yi, NiColor(c.r, c.g, c.b) );
				}
			}

			/// 버퍼를 갱신
			mBuffer->UpdatePosCoord( TERRAIN, mXIndex, mYIndex, TERRAIN_LEAF_LINE_COUNT );
			mBuffer->UpdateAlpha( TERRAIN, mXIndex, mYIndex, TERRAIN_LEAF_LINE_COUNT );
			mBuffer->UpdateColor( TERRAIN, mXIndex, mYIndex, TERRAIN_LEAF_LINE_COUNT );
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
	return true;
}
