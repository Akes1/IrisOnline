#include "stdafx.h"

#ifdef MAP_EDITOR
#include "Terrain.h"
#include "TerrainNode.h"
#include "../Doing/DoingManager.h"
#include "../Doing/TerrainBuilding.h"

bool cTerrain::BackupBuilding( cTerrainBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodeBuildingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];
	bool ret = false;

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			if( mNodeArray[i + xi]->BackupBuilding( n, pos, outerRadius ) )
			{
				ret = true;
			}
		}
	}
	return ret;
}

void cTerrain::UpdateBuilding( cTerrainBuildingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodeBuildingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			mNodeArray[i + xi]->UpdateBuilding( n, pos, outerRadius );
		}
	}
}

bool cTerrain::Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float strength )
{
	float maxValue = strength * (mMetersPerVertex * mUnitsPerMeter * 10);

	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Raise( pos, innerRadius, outerRadius, maxValue );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}

bool cTerrain::Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float strength )
{
	float maxValue = strength * (mMetersPerVertex * mUnitsPerMeter * 10);

	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Lower( pos, innerRadius, outerRadius, maxValue );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}

bool cTerrain::Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float strength )
{
	float maxValue = strength * (mMetersPerVertex * mUnitsPerMeter * 10);

	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Flatten( pos, innerRadius, outerRadius, maxValue );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}

bool cTerrain::Smooth( const NiPoint3& pos, float outerRadius, float ratio )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Smooth( pos, outerRadius, ratio );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}

bool cTerrain::SyncToNaviMesh( const NiPoint3& pos, float innerRadius, float outerRadius )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->SyncToNaviMesh( pos, innerRadius, outerRadius );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}

bool cTerrain::SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float z )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainBuildingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->SyncToPickHeight( pos, innerRadius, outerRadius, z );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateBuilding( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainBuilding( info );
	mModified = true;
	return true;
}
#endif /// MAP_EDITOR
