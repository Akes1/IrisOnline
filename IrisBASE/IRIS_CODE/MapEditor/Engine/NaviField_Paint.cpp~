#include "stdafx.h"

#ifdef MAP_EDITOR
#include "NaviField.h"
#include "NaviFieldNode.h"
#include "../Doing/DoingManager.h"
#include "../Doing/NaviFieldPainting.h"

bool cNaviField::BackupPainting( cNaviFieldPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float radius )
{
	assert( info );

	cNaviFieldNodePaintingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cNaviFieldLeafNode::mCellCount;
	bool ret = false;

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			if( mNodeArray[i + xi]->BackupPainting( n, pos, radius ) )
			{
				ret = true;
			}
		}
	}
	return ret;
}

void cNaviField::UpdatePainting( cNaviFieldPaintingInfo* info, unsigned int xbegin, unsigned int ybegin, unsigned int xend, unsigned int yend, const NiPoint3& pos, float radius )
{
	assert( info );

	cNaviFieldNodePaintingInfo* n = &(info->mNodeArray[0]);
	unsigned int nodeCount = mCellCount / cNaviFieldLeafNode::mCellCount;

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++n )
		{
			mNodeArray[i + xi]->UpdatePainting( n, pos, radius );
		}
	}
}

bool cNaviField::Color( const NiPoint3& pos, float radius, float red, float green, float blue, unsigned char value )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, radius ) == false )
		return false;

	/// 실행 정보를 수집
	cNaviFieldPaintingInfo info;
	info.mNodeArray.Resize( (yend - ybegin) * (xend - xbegin) );

	/// 범위 내의 리프 노드들을 백업
	/// 백업한 리프 노드가 하나도 없으면 바로 리턴한다.
	if( BackupPainting( &info, xbegin, ybegin, xend, yend, pos, radius ) == false )
	{
		return false;
	}

	/// 범위 내의 리프 노드들에 대해 적용
	unsigned int nodeCount = mCellCount / cNaviFieldLeafNode::mCellCount;

	for( unsigned int yi = ybegin, i = ybegin * nodeCount; yi < yend; ++yi, i += nodeCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			mNodeArray[i + xi]->Color( pos, radius, red, green, blue, value );
		}
	}

	/// 범위 내의 리프 노드들을 갱신
	UpdatePainting( &info, xbegin, ybegin, xend, yend, pos, radius );

	/// 실행 정보를 추가
	DOINGMAN->PushNaviFieldPainting( info );
	mModified = true;
	return true;
}
#endif /// MAP_EDITOR
