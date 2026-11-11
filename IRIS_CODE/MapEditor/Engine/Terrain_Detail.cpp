#include "stdafx.h"

#ifdef MAP_EDITOR
#include "Terrain.h"
#include "TerrainNode.h"
#include "../Doing/DoingManager.h"
#include "../Doing/TerrainDetailing.h"

bool cTerrain::BackupDetailing( cTerrainDetailingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodeDetailingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];
	bool ret = false;

	for( unsigned int yi = ybegin, i = nodeCount * ybegin; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			if( mNodeArray[i + xi]->BackupDetailing( n, pos, outerRadius ) )
			{
				ret = true;
			}
		}
	}
	return ret;
}

void cTerrain::UpdateDetailing( cTerrainDetailingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	cTerrainNodeDetailingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = nodeCount * ybegin; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			mNodeArray[i + xi]->UpdateDetailing( n, pos, outerRadius );
		}
	}
}

bool cTerrain::PaintAlpha( const NiPoint3& pos, float innerRadius, float outerRadius, const NiPoint3& alpha, bool applyDetailTex, unsigned int texIndex0, unsigned int texIndex1, unsigned int texIndex2 )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRangeTight( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainDetailingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = nodeCount * ybegin; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			cTerrainLeafNode* n = mNodeArray[xi+i];
			if( applyDetailTex )
				SetDetailTextures( n, texIndex0, texIndex1, texIndex2 );
			n->PaintAlpha( pos, innerRadius, outerRadius, alpha );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainDetailing( info );
	mModified = true;
	return true;
}

bool cTerrain::SharpenAlpha( const NiPoint3& pos, float outerRadius, float ratio )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainDetailingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = nodeCount * ybegin; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->SharpenAlpha( pos, outerRadius, ratio );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainDetailing( info );
	mModified = true;
	return true;
}

bool cTerrain::BlurAlpha( const NiPoint3& pos, float outerRadius, float ratio )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
		return false;

	/// 실행 정보를 수집
	cTerrainDetailingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cTerrainLeafNode::mCellCount[0];

	for( unsigned int yi = ybegin, i = nodeCount * ybegin; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->BlurAlpha( pos, outerRadius, ratio );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdateDetailing( &info, xbegin, ybegin, xend, yend, pos, outerRadius );

	/// 실행 정보를 추가
	DOINGMAN->PushTerrainDetailing( info );
	mModified = true;
	return true;
}
#endif /// MAP_EDITOR
