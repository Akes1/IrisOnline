/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 지형 디테일 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cTerrainLeafNode;

/// 지형 노드 디테일 정보
class cTerrainNodeDetailingInfo
{
public:
	cTerrainNodeDetailingInfo();

public:
	/// 리프 노드
	cTerrainLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 알파 배열
	NiPoint3* mUndoAlphaArray;

	/// Redo 정보
	/// 알파 배열
	NiPoint3* mRedoAlphaArray;
};

inline
cTerrainNodeDetailingInfo::cTerrainNodeDetailingInfo()
: mNode( 0 )
, mXBegin( 0 )
, mYBegin( 0 )
, mXEnd( 0 )
, mYEnd( 0 )
, mUndoAlphaArray( 0 )
, mRedoAlphaArray( 0 )
{
}

/// 지형 디테일 정보
class cTerrainDetailingInfo
{
public:
	/// 노드 정보 배열
	typedef tArray<cTerrainNodeDetailingInfo> cNodeArray;
	cNodeArray mNodeArray;
};

/// 지형 노드 디테일
class cTerrainNodeDetailing
{
public:
	cTerrainNodeDetailing( const cTerrainNodeDetailingInfo& info );
	~cTerrainNodeDetailing();

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
	/// 알파 배열
	NiPoint3* mUndoAlphaArray;

	/// Redo 정보
	/// 알파 배열
	NiPoint3* mRedoAlphaArray;
};

/// 지형 디테일
class cTerrainDetailing : public cDoing
{
	friend class cDoingManager;
	friend class cTerrain;

private:
	cTerrainDetailing( const cTerrainDetailingInfo& info );
	~cTerrainDetailing();

	void Undo();
	void Redo();

private:
	/// 노드 배열
	typedef tArray<cTerrainNodeDetailing*> cNodeArray;
	cNodeArray mNodeArray;
};
