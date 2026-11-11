#include "stdafx.h"

#ifdef MAP_EDITOR
#include "TerrainNode.h"
#include "Terrain.h"
#include "../Doing/TerrainPainting.h"

bool cTerrainLeafNode::BackupPainting( cTerrainNodePaintingInfo* info, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// Undo 정보를 수집
	info->mNode = this;
	info->mXBegin = xbegin;
	info->mYBegin = ybegin;
	info->mXEnd = xend;
	info->mYEnd = yend;
	NiColor* p = info->mUndoColorArray = NiNew NiColor[(yend - ybegin) * (xend - xbegin)];

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetColor( p, mXIndex + xi, mYIndex + yi );
		}
	}
	return true;
}

void cTerrainLeafNode::UpdatePainting( cTerrainNodePaintingInfo* info, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return;
	}

	/// Redo 정보를 수집
	NiColor* p = info->mRedoColorArray = NiNew NiColor[(yend - ybegin) * (xend - xbegin)];

	/// 지형의 색상맵과 동기화
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetColor( p, mXIndex + xi, mYIndex + yi );
			mColors[i + xi] = *p;
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mColors[i][j] = mColors[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateColors();
}

void cTerrainLeafNode::Undo( const cTerrainNodePainting& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiColor* p = doing.mUndoColorArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mColors[i + xi] = *p;
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mColors[i][j] = mColors[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateColors();
}

void cTerrainLeafNode::Redo( const cTerrainNodePainting& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiColor* p = doing.mRedoColorArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mColors[i + xi] = *p;
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mColors[i][j] = mColors[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateColors();
}

bool cTerrainLeafNode::Color( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);
			float r;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				r = 1.0f;
			else
				r = (outerRadius - d) / (outerRadius - innerRadius);

			NiColor& c = mColors[i + xi];
			c = c + r * (color - c);

			/// 지형의 색상맵을 갱신
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, c );
		}
	}
	return true;
}

bool cTerrainLeafNode::Brush( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color, float opacity )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	NiColor color0 = NiColor::WHITE + opacity * (color - NiColor::WHITE);

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);
			float r;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				r = 1.0f;
			else
				r = (outerRadius - d) / (outerRadius - innerRadius);

			NiColor color1 = NiColor::WHITE + r * (color0 - NiColor::WHITE);
			NiColor& c = mColors[i + xi];
			c *= color1;

			/// 지형의 색상맵을 갱신
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, c );
		}
	}
	return true;
}

bool cTerrainLeafNode::Lighter( const NiPoint3& pos, float innerRadius, float outerRadius )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);
			float r;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				r = 1.0f;
			else
				r = (outerRadius - d) / (outerRadius - innerRadius);

			NiColor& c = mColors[i + xi];
			c.r += r * 0.05f;
			c.g += r * 0.05f;
			c.b += r * 0.05f;

			if( c.r > 1.0f )
				c.r = 1.0f;
			if( c.g > 1.0f )
				c.g = 1.0f;
			if( c.b > 1.0f )
				c.b = 1.0f;

			/// 지형의 색상맵을 갱신
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, c );
		}
	}
	return true;
}

bool cTerrainLeafNode::Darker( const NiPoint3& pos, float innerRadius, float outerRadius )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);
			float r;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				r = 1.0f;
			else
				r = (outerRadius - d) / (outerRadius - innerRadius);

			NiColor& c = mColors[i + xi];
			c.r -= r * 0.05f;
			c.g -= r * 0.05f;
			c.b -= r * 0.05f;

			if( c.r < 0.0f )
				c.r = 0.0f;
			if( c.g < 0.0f )
				c.g = 0.0f;
			if( c.b < 0.0f )
				c.b = 0.0f;

			/// 지형의 색상맵을 갱신
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, c );
		}
	}
	return true;
}

bool cTerrainLeafNode::BlurColor( const NiPoint3& pos, float outerRadius, float ratio )
{
	if( ratio < 0.1f )
	{
		assert( "too small ratio" );
		return false;
	}

	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	NiColor c[9], sumc;
	int count;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			unsigned int tx = mXIndex + xi;
			unsigned int ty = mYIndex + yi;
			float x = tx * 100.0f;
			float y = ty * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);

			if( d > outerRadius )
				continue;

			/// 주변 점들의 색상값을 얻음
			sumc.r = 0;
			sumc.g = 0;
			sumc.b = 0;
			count = 0;

			if( TERRAIN->GetColor( &c[0], tx - 1, ty - 1 ) )
			{
				sumc += c[0];
				++count;
			}
			if( TERRAIN->GetColor( &c[1], tx, ty - 1 ) )
			{
				sumc += c[1];
				++count;
			}
			if( TERRAIN->GetColor( &c[2], tx + 1, ty - 1 ) )
			{
				sumc += c[2];
				++count;
			}
			if( TERRAIN->GetColor( &c[3], tx - 1, ty ) )
			{
				sumc += c[3];
				++count;
			}
			if( TERRAIN->GetColor( &c[4], tx, ty ) )
			{
				sumc += c[4];
				++count;
			}
			if( TERRAIN->GetColor( &c[5], tx + 1, ty) )
			{
				sumc += c[5];
				++count;
			}
			if( TERRAIN->GetColor( &c[6], tx - 1, ty + 1) )
			{
				sumc += c[6];
				++count;
			}
			if( TERRAIN->GetColor( &c[7], tx, ty + 1) )
			{
				sumc += c[7];
				++count;
			}
			if( TERRAIN->GetColor( &c[8], tx + 1, ty + 1) )
			{
				sumc += c[8];
				++count;
			}

			/// 평균값을 설정
			if( count )
			{
				NiColor color = mColors[i + xi];
				NiColor avgc = sumc / (float)count;
				mColors[i + xi] = color + (avgc - color) * ratio;
			}
		}
	}

	/// 지형의 색상맵을 갱신
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			TERRAIN->SetColor( mXIndex + xi, mYIndex + yi, mColors[i + xi] );
		}
	}
	return true;
}
#endif /// MAP_EDITOR
