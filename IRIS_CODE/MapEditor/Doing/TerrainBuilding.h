/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 지형 건축 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cTerrainLeafNode;

/// 지형 노드 건축 정보
class cTerrainNodeBuildingInfo
{
public:
	cTerrainNodeBuildingInfo();

public:
	/// 리프 노드
	cTerrainLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 높이값 배열
	float* mUndoHeightArray;

	/// Redo 정보
	/// 높이값 배열
	float* mRedoHeightArray;
};

inline
cTerrainNodeBuildingInfo::cTerrainNodeBuildingInfo()
: mNode( 0 )
, mXBegin( 0 )
, mYBegin( 0 )
, mXEnd( 0 )
, mYEnd( 0 )
, mUndoHeightArray( 0 )
, mRedoHeightArray( 0 )
{
}

/// 지형 건축 정보
class cTerrainBuildingInfo
{
public:
	/// 노드 정보 배열
	typedef tArray<cTerrainNodeBuildingInfo> cNodeArray;
	cNodeArray mNodeArray;
};

/// 지형 노드 건축
class cTerrainNodeBuilding
{
public:
	cTerrainNodeBuilding( const cTerrainNodeBuildingInfo& info );
	~cTerrainNodeBuilding();

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
	/// 높이값 배열
	float* mUndoHeightArray;

	/// Redo 정보
	/// 높이값 배열
	float* mRedoHeightArray;
};

/// 지형 건축
class cTerrainBuilding : public cDoing
{
	friend class cDoingManager;
	friend class cTerrain;

private:
	cTerrainBuilding( const cTerrainBuildingInfo& info );
	~cTerrainBuilding();

	void Undo();
	void Redo();

private:
	/// 노드 배열
	typedef tArray<cTerrainNodeBuilding*> cNodeArray;
	cNodeArray mNodeArray;
};
