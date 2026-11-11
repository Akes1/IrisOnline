/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 네비필드 노드
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Box.h"

class cRay;
class NiFrustumPlanes;
class cSceneNode;
class cNaviFieldBranchNode;
class cNaviFieldLeafNode;
class cNaviFieldBuffer;
#ifdef MAP_EDITOR
class cNaviFieldNodePaintingInfo;
class cNaviFieldNodePainting;
#endif

/// 네비필드 노드
class cNaviFieldNode
{
public:
	cNaviFieldNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cNaviFieldNode();

	/// 동기화
	virtual void SyncHeight() = 0;

	/// 컬링
	virtual void Cull( tArray<cNaviFieldLeafNode*>* visibleArray, NiFrustumPlanes& frustum ) = 0;

	/// 가시 배열에 추가
	virtual void AddToVisibleArray( tArray<cNaviFieldLeafNode*>* visibleArray ) = 0;

	/// 반직선에 대한 충돌 검사
	virtual bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray ) = 0;

	/// 경계 상자를 리턴
	const cBox& GetBoundBox() const;

public:
	/// 부모
	cNaviFieldBranchNode* mParent;

	/// 경계 상자
	cBox mBoundBox;
	NiPoint3 mCenter;
	float mRadius;
};

inline
const cBox& cNaviFieldNode::GetBoundBox() const
{
	return mBoundBox;
}

/// 네비필드 가지 노드
class cNaviFieldBranchNode : public cNaviFieldNode
{
	friend class cNaviFieldLeafNode;

public:
	cNaviFieldBranchNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cNaviFieldBranchNode();

	/// 동기화
	void SyncHeight();

	/// 컬링
	void Cull( tArray<cNaviFieldLeafNode*>* visibleArray, NiFrustumPlanes& frustum );

	/// 가시 배열에 추가
	void AddToVisibleArray( tArray<cNaviFieldLeafNode*>* visibleArray );

	/// 반직선에 대한 충돌 검사
	bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray );

protected:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

public:
	/// 자식 배열
	cNaviFieldNode* mChild[4];
};

/// 네비필드 리프 노드
class cNaviFieldLeafNode : public cNaviFieldNode
{
	friend class cNaviField;
	friend class cNaviFieldBranchNode;
	friend class cNaviFieldNodeCompareForRendering;
#ifdef MAP_EDITOR
	friend class cNaviFieldNodePainting;
#endif

public:
	cNaviFieldLeafNode( cNaviFieldBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int gridSize );
	~cNaviFieldLeafNode();

	/// 동기화
	void SyncHeight();

	/// 컬링
	void Cull( tArray<cNaviFieldLeafNode*>* visibleArray, NiFrustumPlanes& frustum );

	/// 가시 배열에 추가
	void AddToVisibleArray( tArray<cNaviFieldLeafNode*>* visibleArray );

	/// 반직선에 대한 충돌 검사
	bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray );

	/// 지형에 맞춤
	bool SyncToNaviField( const NiPoint3& pos, float innerRadius, float radius );

	/// 오브젝트에 맞춤
	bool SyncToObject( const NiPoint3& pos, float innerRadius, float radius, float z );

#ifdef MAP_EDITOR
	/// 색칠
	bool Color( const NiPoint3& pos, float radius, float red, float green, float blue, unsigned char value );
#endif

private:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

	/// 기하 정보를 갱신
	void UpdateColors();

#ifdef MAP_EDITOR
	/// 변경전 정보를 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupPainting( cNaviFieldNodePaintingInfo* info, const NiPoint3& pos, float radius );

	/// 네비필드의 색상맵과 동기화 및 갱신
	void UpdatePainting( cNaviFieldNodePaintingInfo* info, const NiPoint3& pos, float radius );

	/// Undo, Redo
	void Undo( const cNaviFieldNodePainting& doing );
	void Redo( const cNaviFieldNodePainting& doing );
#endif

	/// 정점 범위를 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float radius );

private:
	/// 좌표 (네비필드 그리드 상의 위치)
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// 버퍼
	cNaviFieldBuffer* mBuffer;

private:
	/// 리프 노드의 그리드 크기
	static unsigned int mCellCount;
	static unsigned int mLineCount;
};

/// 네비필드 노드 비교 함수자
class cNaviFieldNodeCompareForRendering
{
public:
	bool operator () ( const cNaviFieldLeafNode* left, const cNaviFieldLeafNode* right ) const
	{
		cNaviFieldBuffer* lbuff = left->mBuffer;
		cNaviFieldBuffer* rbuff = right->mBuffer;

		return lbuff < rbuff;
	};
};
