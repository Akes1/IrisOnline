/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 네비메시 건축 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cNaviMeshLeafNode;

/// 네비메시 노드 건축 정보
class cNaviMeshNodeBuildingInfo
{
public:
	cNaviMeshNodeBuildingInfo();

public:
	/// 리프 노드
	cNaviMeshLeafNode* mNode;

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
cNaviMeshNodeBuildingInfo::cNaviMeshNodeBuildingInfo()
: mNode( 0 )
, mXBegin( 0 )
, mYBegin( 0 )
, mXEnd( 0 )
, mYEnd( 0 )
, mUndoHeightArray( 0 )
, mRedoHeightArray( 0 )
{
}

/// 네비메시 건축 정보
class cNaviMeshBuildingInfo
{
public:
	/// 노드 정보 배열
	typedef tArray<cNaviMeshNodeBuildingInfo> cNodeArray;
	cNodeArray mNodeArray;
};

/// 네비메시 노드 건축
class cNaviMeshNodeBuilding
{
public:
	cNaviMeshNodeBuilding( const cNaviMeshNodeBuildingInfo& info );
	~cNaviMeshNodeBuilding();

	void Undo();
	void Redo();

public:
	/// 리프 노드
	cNaviMeshLeafNode* mNode;

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

/// 네비메시 건축
class cNaviMeshBuilding : public cDoing
{
	friend class cDoingManager;
	friend class cNaviMesh;

private:
	cNaviMeshBuilding( const cNaviMeshBuildingInfo& info );
	~cNaviMeshBuilding();

	void Undo();
	void Redo();

private:
	/// 노드 배열
	typedef tArray<cNaviMeshNodeBuilding*> cNodeArray;
	cNodeArray mNodeArray;
};
