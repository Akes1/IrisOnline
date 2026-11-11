#include "stdafx.h"

#ifdef MAP_EDITOR
#include "TerrainNode.h"
#include "Terrain.h"
#include "../Doing/TerrainDetailing.h"

bool cTerrainLeafNode::BackupDetailing( cTerrainNodeDetailingInfo* info, const NiPoint3& pos, float outerRadius )
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
	NiPoint3* p = info->mUndoAlphaArray = NiNew NiPoint3[(yend - ybegin) * (xend - xbegin)];

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetAlpha( p, mXIndex + xi, mYIndex + yi );
		}
	}
	return true;
}

void cTerrainLeafNode::UpdateDetailing( cTerrainNodeDetailingInfo* info, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return;
	}

	/// Redo 정보를 수집
	NiPoint3* p = info->mRedoAlphaArray = NiNew NiPoint3[(yend - ybegin) * (xend - xbegin)];

	/// 지형의 알파맵과 동기화
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetAlpha( p, mXIndex + xi, mYIndex + yi );
			mAlphas[i + xi] = *p;
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mAlphas[i][j] = mAlphas[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateAlphas();
}

void cTerrainLeafNode::Undo( const cTerrainNodeDetailing& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiPoint3* p = doing.mUndoAlphaArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mAlphas[i + xi] = *p;
			TERRAIN->SetAlpha( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mAlphas[i][j] = mAlphas[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateAlphas();
}

void cTerrainLeafNode::Redo( const cTerrainNodeDetailing& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiPoint3* p = doing.mRedoAlphaArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mAlphas[i + xi] = *p;
			TERRAIN->SetAlpha( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mAlphas[i][j] = mAlphas[0][y * step * mGridSize[0] + x * step];
			}
		}
	}
	*/

	/// 기하 정보를 갱신
	UpdateAlphas();
}

bool cTerrainLeafNode::PaintAlpha( const NiPoint3& pos, float innerRadius, float outerRadius, const NiPoint3& alpha )
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
				r = 1;
			else
				r = (outerRadius - d) / (outerRadius - innerRadius);

			NiPoint3& a = mAlphas[i + xi];
			a = a + r * (alpha - a);

			mPaintAlphas[i + xi] = mAlphas[i + xi];

			/// 지형의 색상맵을 갱신
			TERRAIN->SetAlpha( mXIndex + xi, mYIndex + yi, a );
			TERRAIN->SetPaintAlpha( mXIndex + xi, mYIndex + yi, a );
		}
	}
	return true;
}

bool cTerrainLeafNode::SharpenAlpha( const NiPoint3& pos, float outerRadius, float ratio )
{
	if( ratio < 0.1f )
	{
		assert( "too small smooth ratio" );
		return false;
	}

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
			unsigned int tx = mXIndex + xi;
			unsigned int ty = mYIndex + yi;
			float x = tx * 100.0f;
			float y = ty * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);

			if( d > outerRadius )
				continue;

			/// 평균값을 설정
			NiPoint3 alpha = mAlphas[i + xi];
			NiPoint3 paintAlpha = mPaintAlphas[i + xi];

			mAlphas[i+xi] = paintAlpha + (alpha - paintAlpha) * ratio;
		}
	}

	/// 지형의 색상맵을 갱신
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			TERRAIN->SetAlpha( mXIndex + xi, mYIndex + yi, mAlphas[i + xi] );
		}
	}
	return true;

}

bool cTerrainLeafNode::BlurAlpha( const NiPoint3& pos, float outerRadius, float ratio )
{
	if( ratio < 0.1f )
	{
		assert( "too small smooth ratio" );
		return false;
	}

	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	NiPoint3 a[9], suma;
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

			/// 주변 점들의 알파값을 얻음
			suma = NiPoint3::ZERO;
			count = 0;

			if( TERRAIN->GetAlpha( &a[0], tx - 1, ty - 1 ) )
			{
				suma += a[0];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[1], tx, ty - 1 ) )
			{
				suma += a[1];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[2], tx + 1, ty - 1 ) )
			{
				suma += a[2];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[3], tx - 1, ty ) )
			{
				suma += a[3];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[4], tx, ty ) )
			{
				suma += a[4];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[5], tx + 1, ty) )
			{
				suma += a[5];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[6], tx - 1, ty + 1) )
			{
				suma += a[6];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[7], tx, ty + 1) )
			{
				suma += a[7];
				++count;
			}
			if( TERRAIN->GetAlpha( &a[8], tx + 1, ty + 1) )
			{
				suma += a[8];
				++count;
			}

			/// 평균값을 설정
			if( count )
			{
				NiPoint3 alpha = mAlphas[i + xi];
				NiPoint3 avga = suma / float(count);
				mAlphas[i + xi] = alpha + (avga - alpha) * ratio;
			}
		}
	}

	/// 지형의 색상맵을 갱신
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			TERRAIN->SetAlpha( mXIndex + xi, mYIndex + yi, mAlphas[i + xi] );
		}
	}
	return true;
}
#endif /// MAP_EDITOR
