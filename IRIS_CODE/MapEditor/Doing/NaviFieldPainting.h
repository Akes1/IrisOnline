/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 네비필드 색칠 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Doing.h"

class cNaviFieldLeafNode;

/// 네비필드 노드 색칠 정보
class cNaviFieldNodePaintingInfo
{
public:
	cNaviFieldNodePaintingInfo();

public:
	/// 리프 노드
	cNaviFieldLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 색상 배열
	NiColor* mUndoColorArray;

	/// 플랙값 배열
	unsigned char* mUndoValueArray;

	/// Redo 정보
	/// 색상 배열
	NiColor* mRedoColorArray;

	/// 플랙값 배열
	unsigned char* mRedoValueArray;
};

inline
cNaviFieldNodePaintingInfo::cNaviFieldNodePaintingInfo()
: mNode( 0 )
, mXBegin( 0 )
, mYBegin( 0 )
, mXEnd( 0 )
, mYEnd( 0 )
, mUndoColorArray( 0 )
, mUndoValueArray( 0 )
, mRedoColorArray( 0 )
, mRedoValueArray( 0 )
{
}

/// 네비필드 색칠 정보
class cNaviFieldPaintingInfo
{
public:
	/// 노드 정보 배열
	typedef tArray<cNaviFieldNodePaintingInfo> cNodeArray;
	cNodeArray mNodeArray;
};

/// 네비필드 노드 색칠
class cNaviFieldNodePainting
{
public:
	cNaviFieldNodePainting( const cNaviFieldNodePaintingInfo& info );
	~cNaviFieldNodePainting();

	void Undo();
	void Redo();

public:
	/// 리프 노드
	cNaviFieldLeafNode* mNode;

	/// 적용 범위 (리프 노드내 정점의 인덱스)
	unsigned int mXBegin;
	unsigned int mYBegin;
	unsigned int mXEnd;
	unsigned int mYEnd;

	/// Undo 정보
	/// 색상 배열
	NiColor* mUndoColorArray;

	/// 플랙값 배열
	unsigned char* mUndoValueArray;

	/// Redo 정보
	/// 색상 배열
	NiColor* mRedoColorArray;

	/// 플랙값 배열
	unsigned char* mRedoValueArray;
};

/// 네비필드 색칠
class cNaviFieldPainting : public cDoing
{
	friend class cDoingManager;
	friend class cNaviField;

private:
	cNaviFieldPainting( const cNaviFieldPaintingInfo& info );
	~cNaviFieldPainting();

	void Undo();
	void Redo();

private:
	/// 노드 배열
	typedef tArray<cNaviFieldNodePainting*> cNodeArray;
	cNodeArray mNodeArray;
};
