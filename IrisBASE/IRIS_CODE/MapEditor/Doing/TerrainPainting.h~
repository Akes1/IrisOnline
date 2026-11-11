/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 지형 색칠 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cTerrainLeafNode;

/// 지형 노드 색칠 정보
class cTerrainNodePaintingInfo
{
public:
	cTerrainNodePaintingInfo();

public:
	/// 리프 노드
	cTerrainLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 색상 배열
	NiColor* mUndoColorArray;

	/// Redo 정보
	/// 색상 배열
	NiColor* mRedoColorArray;
};

inline
cTerrainNodePaintingInfo::cTerrainNodePaintingInfo()
: mNode( 0 )
, mXBegin( 0 )
, mYBegin( 0 )
, mXEnd( 0 )
, mYEnd( 0 )
, mUndoColorArray( 0 )
, mRedoColorArray( 0 )
{
}

/// 지형 색칠 정보
class cTerrainPaintingInfo
{
public:
	/// 노드 정보 배열
	typedef tArray<cTerrainNodePaintingInfo> cNodeArray;
	cNodeArray mNodeArray;
};

/// 지형 노드 색칠
class cTerrainNodePainting
{
public:
	cTerrainNodePainting( const cTerrainNodePaintingInfo& info );
	~cTerrainNodePainting();

	void Undo();
	void Redo();

public:
	/// 리프 노드
	cTerrainLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 색상 배열
	NiColor* mUndoColorArray;

	/// Redo 정보
	/// 색상 배열
	NiColor* mRedoColorArray;
};

/// 지형 색칠
class cTerrainPainting : public cDoing
{
	friend class cDoingManager;
	friend class cTerrain;

private:
	cTerrainPainting( const cTerrainPaintingInfo& info );
	~cTerrainPainting();

	void Undo();
	void Redo();

private:
	/// 노드 배열
	typedef tArray<cTerrainNodePainting*> cNodeArray;
	cNodeArray mNodeArray;
};
