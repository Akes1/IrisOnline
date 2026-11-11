#include "stdafx.h"

#ifdef MAP_EDITOR
#include "Terrain.h"
#include "TerrainNode.h"
#include "../Doing/DoingManager.h"
#include "../Doing/TerrainPainting.h"

bool cTerrain::BackupPainting( cTerrainPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodePaintingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];
	bool ret = false;

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			if( mNodeArray[i + xi]->BackupPainting( n, pos, outerRadius ) )
			{
				ret = true;
			}
		}
	}
	return ret;
}

void cTerrain::UpdatePainting( cTerrainPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodePaintingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			mNodeArray[i + xi]->UpdatePainting( n, pos, outerRadius );
		}
	}
}

bool cTerrain::Color( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Color( pos, innerRadius, outerRadius, color );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainPainting( info );
	mModified = true;
	return true;
}

bool cTerrain::Brush( const NiPoint3& pos, float innerRadius, float outerRadius, const NiColor& color, float opacity )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Brush( pos, innerRadius, outerRadius, color, opacity );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainPainting( info );
	mModified = true;
	return true;
}

bool cTerrain::Lighter( const NiPoint3& pos, float innerRadius, float outerRadius )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Lighter( pos, innerRadius, outerRadius );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainPainting( info );
	mModified = true;
	return true;
}

bool cTerrain::Darker( const NiPoint3& pos, float innerRadius, float outerRadius )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Darker( pos, innerRadius, outerRadius );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainPainting( info );
	mModified = true;
	return true;
}

bool cTerrain::BlurColor( const NiPoint3& pos, float outerRadius, float ratio )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->BlurColor( pos, outerRadius, ratio );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainPainting( info );
	mModified = true;
	return true;
}
#endif /// MAP_EDITOR
