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
class cNaviMeshBuffer;
#ifdef MAP_EDITOR
class cNaviMeshNodeBuildingInfo;
class cNaviMeshNodeBuilding;
#endif

/// 네비메시 노드
class cNaviMeshNode
{
public:
	cNaviMeshNode( cNaviMeshBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	virtual ~cNaviMeshNode();

	/// 로딩
	virtual bool Load( cFileLoader& loader ) = 0;

	/// 저장
	virtual bool Save( cFileSaver& saver ) = 0;

	/// 네비메시의 높이맵과 동기화
	virtual void SyncHeight() = 0;

	/// 컬링
	virtual void Cull( tArray<cNaviMeshLeafNode*>* visibleArray, NiFrustumPlanes& frustum ) = 0;

	/// 가시 배열에 추가
	virtual void AddToVisibleArray( tArray<cNaviMeshLeafNode*>* visibleArray ) = 0;

	/// 충돌 검사
	virtual bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray, float maxDistance ) = 0;

	/// 경계 상자
	const cBox& GetBoundBox() const;

public:
	/// 부모
	cNaviMeshBranchNode* mParent;

	/// 경계
	cBox mBoundBox;
	NiPoint3 mCenter;
	float mRadius;
};

inline
const cBox& cNaviMeshNode::GetBoundBox() const
{
	return mBoundBox;
}

/// 네비메시 가지 노드
class cNaviMeshBranchNode : public cNaviMeshNode
{
	friend class cNaviMeshLeafNode;

public:
	cNaviMeshBranchNode( cNaviMeshBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int cellCount );
	~cNaviMeshBranchNode();

	/// 로딩
	bool Load( cFileLoader& loader );

	/// 저장
	bool Save( cFileSaver& saver );

	/// 지형의 높이맵과 동기화
	void SyncHeight();

	/// 컬링
	void Cull( tArray<cNaviMeshLeafNode*>* visibleArray, NiFrustumPlanes& frustum );

	/// 가시 배열에 추가
	void AddToVisibleArray( tArray<cNaviMeshLeafNode*>* visibleArray );

	/// 충돌 검사
	bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray, float maxDistance = NI_INFINITY );

protected:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

public:
	/// 자식 배열
	cNaviMeshNode* mChild[4];
};

/// 네비메시 리프 노드
class cNaviMeshLeafNode : public cNaviMeshNode
{
	friend class cNaviMesh;
	friend class cNaviMeshBranchNode;
	friend class cNaviMeshNodeCompareForRendering;
#ifdef MAP_EDITOR
	friend class cNaviMeshNodeBuilding;
	friend class cNaviFieldNodePainting;
#endif

public:
	cNaviMeshLeafNode( cNaviMeshBranchNode* parent, unsigned int xi, unsigned int yi, unsigned int gridSize );
	~cNaviMeshLeafNode();

	/// 로딩
	bool Load( cFileLoader& loader );

	/// 저장
	bool Save( cFileSaver& saver );

	/// 지형의 높이맵과 동기화
	void SyncHeight();

	/// 컬링
	void Cull( tArray<cNaviMeshLeafNode*>* visibleArray, NiFrustumPlanes& frustum );

	/// 가시 배열에 추가
	void AddToVisibleArray( tArray<cNaviMeshLeafNode*>* visibleArray );

	/// 충돌 검사
	bool CollideRay( NiPoint3* contact, float* distance, const cRay& ray, float maxDistance = NI_INFINITY );

#ifdef MAP_EDITOR
	/// 올림
	bool Raise( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 낮춤
	bool Lower( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 평평하게 만듦
	bool Flatten( const NiPoint3& pos, float innerRadius, float outerRadius, float maxValue );

	/// 부드럽게 만듦
	/// ratio는 [0.1, 1] 범위의 값을 가져야 한다.
	bool Smooth( const NiPoint3& pos, float outerRadius, float ratio );
#endif

	/// 지형에 맞춤
	bool SyncToTerrain( const NiPoint3& pos, float innerRadius, float outerRadius );

	/// 오브젝트에 맞춤
	bool SyncToObject( const NiPoint3& pos, float outerRadius );

	/// 픽킹 높이에 맞춤
	bool SyncToPickHeight( const NiPoint3& pos, float innerRadius, float outerRadius, float z );

private:
	/// 경계 상자를 갱신
	void UpdateBoundUpward();

	/// 기하 정보를 갱신
	void UpdatePosCoords();

#ifdef MAP_EDITOR
	/// 변경전 정보를 백업
	/// Undo, Redo에 필요한 정보를 수집한다.
	bool BackupBuilding( cNaviMeshNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius );

	/// 네비메시의 높이맵과 동기화 및 갱신
	/// 동기화는 계산상의 오차로 인한 크랙을 방지한다.
	void UpdateBuilding( cNaviMeshNodeBuildingInfo* info, const NiPoint3& pos, float outerRadius );

	/// Undo, Redo
	void Undo( const cNaviMeshNodeBuilding& doing );
	void Redo( const cNaviMeshNodeBuilding& doing );
#endif

	/// 정점 범위를 계산
	bool CalcRange( unsigned int* xbegin, unsigned int* ybegin, unsigned int* xend, unsigned int* yend, const NiPoint3& pos, float outerRadius );

private:
	/// 좌표 (네비메시 그리드 상의 위치)
	unsigned int mXIndex;
	unsigned int mYIndex;

	/// 버퍼
	cNaviMeshBuffer* mBuffer;

	/// 높이 배열
	float* mHeights;

private:
	/// 버전
	static unsigned int mVersion;

	/// 리프 노드의 그리드 크기
	static unsigned int mCellCount;
	static unsigned int mLineCount;
};

/// 네비메시 노드 비교 함수자
class cNaviMeshNodeCompareForRendering
{
public:
	bool operator () ( const cNaviMeshLeafNode* left, const cNaviMeshLeafNode* right ) const
	{
		cNaviMeshBuffer* lbuff = left->mBuffer;
		cNaviMeshBuffer* rbuff = right->mBuffer;

		return lbuff < rbuff;
	};
};
