#include "stdafx.h"

#ifdef MAP_EDITOR
#include "NaviFieldNode.h"
#include "NaviField.h"
#include "../Doing/NaviFieldPainting.h"

bool cNaviFieldLeafNode::BackupPainting( cNaviFieldNodePaintingInfo* info, const NiPoint3& pos, float outerRadius )
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
	NiColor* color = info->mUndoColorArray = NiNew NiColor[(yend - ybegin) * (xend - xbegin)];
	unsigned char* value = info->mUndoValueArray = new unsigned char[(yend - ybegin) * (xend - xbegin)];

	for( unsigned int yi = ybegin, i = ybegin * mLineCount; yi < yend; ++yi, i += mLineCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++color, ++value )
		{
			NAVIFIELD->GetColor( color, mXIndex + xi, mYIndex + yi );
			NAVIFIELD->GetValue( value, mXIndex + xi, mYIndex + yi );
		}
	}
	return true;
}

void cNaviFieldLeafNode::UpdatePainting( cNaviFieldNodePaintingInfo* info, const NiPoint3& pos, float outerRadius )
{
	assert( info );

	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, outerRadius ) == false )
	{
		return;
	}

	/// Redo 정보를 수집
	NiColor* color = info->mRedoColorArray = NiNew NiColor[(yend - ybegin) * (xend - xbegin)];
	unsigned char* value = info->mRedoValueArray = new unsigned char[(yend - ybegin) * (xend - xbegin)];

	/// 네비필드의 색상맵과 동기화
	for( unsigned int yi = ybegin, i = ybegin * mLineCount; yi < yend; ++yi, i += mLineCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++color, ++value )
		{
			NAVIFIELD->GetColor( color, mXIndex + xi, mYIndex + yi );
			NAVIFIELD->GetValue( value, mXIndex + xi, mYIndex + yi );
		}
	}

	/// 기하 정보를 갱신
	UpdateColors();
}

void cNaviFieldLeafNode::Undo( const cNaviFieldNodePainting& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiColor* color = doing.mUndoColorArray;
	unsigned char* value = doing.mUndoValueArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount; yi < yend; ++yi, i += mLineCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++color, ++value )
		{
			NAVIFIELD->SetColor( mXIndex + xi, mYIndex + yi, *color );
			NAVIFIELD->SetValue( mXIndex + xi, mYIndex + yi, *value );
		}
	}

	/// 기하 정보를 갱신
	UpdateColors();
}

void cNaviFieldLeafNode::Redo( const cNaviFieldNodePainting& doing )
{
	unsigned int xbegin = doing.mXBegin;
	unsigned int ybegin = doing.mYBegin;
	unsigned int xend = doing.mXEnd;
	unsigned int yend = doing.mYEnd;
	NiColor* color = doing.mRedoColorArray;
	unsigned char* value = doing.mRedoValueArray;

	for( unsigned int yi = ybegin, i = ybegin * mLineCount; yi < yend; ++yi, i += mLineCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi, ++color, ++value )
		{
			NAVIFIELD->SetColor( mXIndex + xi, mYIndex + yi, *color );
			NAVIFIELD->SetValue( mXIndex + xi, mYIndex + yi, *value );
		}
	}

	/// 기하 정보를 갱신
	UpdateColors();
}

bool cNaviFieldLeafNode::Color( const NiPoint3& pos, float radius, float red, float green, float blue, unsigned char value )
{
	/// 범위 계산
	unsigned int xbegin, ybegin, xend, yend;
	if( CalcRange( &xbegin, &ybegin, &xend, &yend, pos, radius ) == false )
	{
		return false;
	}

	/// 범위 내의 정점들에 대해 적용
	for( unsigned int yi = ybegin, i = ybegin * mLineCount; yi < yend; ++yi, i += mLineCount )
	{
		for( unsigned int xi = xbegin; xi < xend; ++xi )
		{
			float x = (mXIndex + xi) * 100.0f;
			float y = (mYIndex + yi) * 100.0f;
			float dx = x - pos.x;
			float dy = y - pos.y;
			float d = NiSqrt(dx*dx + dy*dy);

			if( d > radius )
				continue;

			/// 네비필드의 색상맵과 플랙맵을 갱신
			NAVIFIELD->SetColor( mXIndex + xi, mYIndex + yi, NiColor(red, green, blue) );
			NAVIFIELD->SetValue( mXIndex + xi, mYIndex + yi, value );
		}
	}
	return true;
}
#endif /// MAP_EDITOR
