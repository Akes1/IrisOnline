/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 네비메시 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Box.h"

class cRay;
class cNaviMeshBranchNode;
class cNaviMeshLeafNode;

/// 네비메시 노드
class cNaviMeshNode
{
public:
	cNaviMeshNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cNaviMeshNode();

	/// 로딩
	virtual bool Load( cFileLoader& loader ) = 0;
	virtual void AddToLoadArray( cNaviMeshNode** loadArray, unsigned int& count ) = 0;

	/// 충돌 검사
	virtual bool CollideRay() = 0;

	/// 경계 상자
	const cBox& GetBoundBox() const;

public:
	/// 경계 상자
	cBox mBoundBox;
	NiPoint3 mCenter;
	float mRadius;

protected:
	/// 충돌 검사용
	static const cRay* mRay;
	static float mMaxDistance;
	static float* mDistance;
	static NiPoint3* mContact;
};

inline
const cBox& cNaviMeshNode::GetBoundBox() const
{
	return mBoundBox;
}

/// 네비메시 가지 노드
class cNaviMeshBranchNode : public cNaviMeshNode
{
public:
	cNaviMeshBranchNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cNaviMeshBranchNode();

	/// 로딩
	bool Load( cFileLoader& loader );
	void AddToLoadArray( cNaviMeshNode** loadArray, unsigned int& count );

	/// 충돌 검사
	bool CollideRay();

public:
	/// 자식 배열
	cNaviMeshNode* mChild[4];
};

/// 네비메시 리프 노드
class cNaviMeshLeafNode : public cNaviMeshNode
{
	friend class cNaviMesh;
	friend class cNaviMeshBranchNode;

public:
	cNaviMeshLeafNode( unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cNaviMeshLeafNode();

	/// 로딩
	bool Load( cFileLoader& loader );
	void AddToLoadArray( cNaviMeshNode** loadArray, unsigned int& count );

	/// 충돌 검사
	bool CollideRay();

private:
	/// 좌표 (네비메시 그리드 상의 위치)
	unsigned int mXIndex;
	unsigned int mYIndex;

private:
	/// 버전
	static unsigned int mVersion;

	/// 리프 노드의 그리드 크기
	static unsigned int mCellCount;
	static unsigned int mLineCount;
};
