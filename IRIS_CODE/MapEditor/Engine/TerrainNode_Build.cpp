#include "stdafx.h"

#ifdef MAP_EDITOR
#include "TerrainNode.h"

#include "Terrain.h"
#include "NaviMesh.h"
#include "SceneNode.h"

#include "../Doing/TerrainBuilding.h"

bool cTerrainLeafNode::BackupBuilding( cTerrainNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius )
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
	float* p = info->mUndoHeightArray = new float[(yend - ybegin) * (xend - xbegin)];

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetHeight( p, mXIndex + xi, mYIndex + yi );
		}
	}
	return true;
}

void cTerrainLeafNode::UpdateBuilding( cTerrainNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return;
	}

	/// Redo 정보를 수집
	float* p = info->mRedoHeightArray = new float[(yend - ybegin) * (xend - xbegin)];

	/// 지형의 높이맵과 동기화
	/// 동기화는 계산상의 오차로 인한 크랙을 방지한다.
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			TERRAIN->GetHeight( p, mXIndex + xi, mYIndex + yi );
			mHeights[i + xi] = *p;
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mPosCoords[i][j].z = mPosCoords[0][y * step * mGridSize[0] + x * step].z;
			}
		}
	}
	*/

	/// 경계 상자를 갱신
	UpdateBoundUpward();

	/// 기하 정보를 갱신
	UpdatePosCoords();
}

void cTerrainLeafNode::Undo( const cTerrainNodeBuilding& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	float* p = doing.mUndoHeightArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mHeights[i + xi] = *p;
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mPosCoords[i][j].z = mPosCoords[0][y * step * mGridSize[0] + x * step].z;
			}
		}
	}
	*/

	/// 경계 상자를 갱신
	UpdateBoundUpward();

	/// 기하 정보를 갱신
	UpdatePosCoords();
}

void cTerrainLeafNode::Redo( const cTerrainNodeBuilding& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	float* p = doing.mRedoHeightArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++p )
		{
			mHeights[i + xi] = *p;
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, *p );
		}
	}
	/*
	for( unsigned int i = 1, step = 2; i < TERRAIN_LOD_COUNT; ++i, step *= 2 )
	{
		for( unsigned int j = 0, y = 0, yend = mGridSize[i]; y < yend; ++y )
		{
			for( unsigned int x = 0, xend = mGridSize[i]; x < xend; ++x, ++j )
			{
				mPosCoords[i][j].z = mPosCoords[0][y * step * mGridSize[0] + x * step].z;
			}
		}
	}
	*/

	/// 경계 상자를 갱신
	UpdateBoundUpward();

	/// 기하 정보를 갱신
	UpdatePosCoords();
}

bool cTerrainLeafNode::Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue )
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
			float v;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				v = maxValue;
			else
				v = maxValue * (outerRadius - d) / (outerRadius - innerRadius);

			float& z = mHeights[i + xi];
			z += v;

			/// 지형의 높이맵을 갱신
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, z );
		}
	}
	return true;
}

bool cTerrainLeafNode::Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue )
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
			float v;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				v = maxValue;
			else
				v = maxValue * (outerRadius - d) / (outerRadius - innerRadius);

			float& z = mHeights[i + xi];
			z -= v;

			/// 지형의 높이맵을 갱신
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, z );
		}
	}
	return true;
}

bool cTerrainLeafNode::Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue )
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
			float& z = mHeights[i + xi];

			if( z == pos.z )
				continue;

			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);
			float v;

			if( d > outerRadius )
				continue;
			if( innerRadius == outerRadius || d <= innerRadius )
				v = maxValue;
			else
				v = maxValue * (outerRadius - d) / (outerRadius - innerRadius);

			if( z > pos.z )
			{
				z -= v;
				if( z < pos.z )
					z = pos.z;
			}
			if( z < pos.z )
			{
				z += v;
				if( z > pos.z )
					z = pos.z;
			}

			/// 지형의 높이맵을 갱신
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, z );
		}
	}
	return true;
}

bool cTerrainLeafNode::Smooth( const NiPoint3& pos, float outerRadius, float ratio )
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
	float z[9], sumz;
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

			/// 주변 점들의 높이값을 얻음
			sumz = 0;
			count = 0;

			if( TERRAIN->GetHeight( &z[0], tx - 1, ty - 1 ) )
			{
				sumz += z[0];
				++count;
			}
			if( TERRAIN->GetHeight( &z[1], tx, ty - 1 ) )
			{
				sumz += z[1];
				++count;
			}
			if( TERRAIN->GetHeight( &z[2], tx + 1, ty - 1 ) )
			{
				sumz += z[2];
				++count;
			}
			if( TERRAIN->GetHeight( &z[3], tx - 1, ty ) )
			{
				sumz += z[3];
				++count;
			}
			if( TERRAIN->GetHeight( &z[4], tx, ty ) )
			{
				sumz += z[4];
				++count;
			}
			if( TERRAIN->GetHeight( &z[5], tx + 1, ty) )
			{
				sumz += z[5];
				++count;
			}
			if( TERRAIN->GetHeight( &z[6], tx - 1, ty + 1) )
			{
				sumz += z[6];
				++count;
			}
			if( TERRAIN->GetHeight( &z[7], tx, ty + 1) )
			{
				sumz += z[7];
				++count;
			}
			if( TERRAIN->GetHeight( &z[8], tx + 1, ty + 1) )
			{
				sumz += z[8];
				++count;
			}

			/// 평균값을 설정
			if( count )
			{
				float posz = mHeights[i + xi];
				float avgz = sumz / count;
				mHeights[i + xi] = posz + (avgz - posz) * ratio;
			}
		}
	}

	/// 지형의 높이맵을 갱신
	for( unsigned int yi = ybegin, i = ybegin * mLineCount[0]; yi < yend; ++yi, i += mLineCount[0] )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, mHeights[i + xi] );
		}
	}
	return true;
}

bool cTerrainLeafNode::SyncToNaviMesh( const NiPoint3& pos, float innerRadius, float outerRadius )
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

			if( d > outerRadius )
				continue;

			float& z = mHeights[i + xi];
			float nz = 0.0f;
			NAVIMESH->GetHeight( &nz, mXIndex + xi, mYIndex + yi );

			if( innerRadius == outerRadius || d <= innerRadius )
			{
				z = nz;
			}
			else
			{
				float r = ( outerRadius - d ) / (outerRadius - innerRadius);
				z = z + r * (nz - z);
			}

			/// 지형의 높이맵을 갱신
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, z );
		}
	}
	return true;
}

bool cTerrainLeafNode::SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float pickz )
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

			if( d > outerRadius )
				continue;

			float& z = mHeights[i + xi];

			if( innerRadius == outerRadius || d <= innerRadius )
			{
				z = pickz;
			}
			else
			{
				float r = ( outerRadius - d ) / (outerRadius - innerRadius);
				z = z + r * (pickz - z);
			}

			/// 지형의 높이맵을 갱신
			TERRAIN->SetHeight( mXIndex + xi, mYIndex + yi, z );
		}
	}
	return true;
}
#endif /// MAP_EDITOR
