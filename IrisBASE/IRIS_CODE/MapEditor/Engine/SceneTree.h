/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 장면 트리
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Sphere.h"
#include "SceneNode.h"

class cSceneTree;
class cSceneCuller;

/// 테스트용 구
/// 장면 노드를 완전히 포함하는 지를 검사하는데 쓰인다.
class cTestSphere : public cSphere
{
public:
	/// 검사 결과
	bool mResult;
};

/// 장면 트리 노드
/// 여러 장면 노드들을 포함하는 컨테이너이다.
class cSceneTreeNode : public cSphere
{
	friend class cSceneTree;

public:
	cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const NiPoint3& center, float radius );
	cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const cSphere& sphere );
	~cSceneTreeNode();

	void* operator new( size_t n ); 
	void operator delete( void* p );

	/// 트리 노드안의 모든 장면 노드들을 지움
	void Clear();

	/// 장면 노드를 추가
	void Push( cSceneNode* node );

	/// 장면 노드를 제거
	void Remove( cSceneNode* node );

	/// 장면 노드의 이동 또는 크기 변화에 대해 갱신
	void Update( cSceneNode* node );

	/// 컬링
	void Cull();

	/// 반직선과 충돌하는 장면 노드들을 수집
	/// 삼각형 단위로 검사한다.
	bool CollideRay();

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool CollideSphere();

protected:
	/// 가시 배열에 추가
	void AddToVisibleArray() const;

private:
	/// 트리
	cSceneTree* mTree;

	/// 부모
	cSceneTreeNode* mParent;

	/// 자식 배열
	cSceneTreeNode* mChild[4];

	/// 테스트용 구
	cTestSphere mTestSpheres[4];

	/// 장면 노드 리스트
	typedef tList<cSceneNode*> cObjectList;
	cObjectList mObjectList;

	/// 메모리 풀
	static tPool<cSceneTreeNode> mPool;

	/// 컬링용
	static NiFrustumPlanes* mFrustum;
	static cSceneCuller* mCuller;
	static eSceneNodeType mPickType;
	static NiVisibleArray* mSolidArray;
	static NiVisibleArray* mAlphaArray;
	static NiVisibleArray* mAlphaTestArray;

	/// 충돌 검사용
	static const cRay* mRay;
	static float mMaxDistance;
	static const cSphere* mSphere;
	static tArray<cSceneNode*>* mCollideArray;
};

/// 장면 트리
/// 게임 내의 수많은 장면 노드들을 컬링하거나 충돌 검사를 수행하기 위해 사용한다.
class cSceneTree
{
public:
	cSceneTree();
	virtual ~cSceneTree();

	/// 모든 장면 노드들을 지움
	void Clear();

	/// 초기화
	void Init( const NiPoint3& center, float minRadius, float maxRadius );

	/// 장면 노드를 추가
	void Push( cSceneNode* node );

	/// 컬링
	void Cull( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray, NiFrustumPlanes& frustum, cSceneCuller& culler, eSceneNodeType type );

	/// 반직선과 충돌하는 장면 노드들을 수집
	/// 삼각형 단위로 검사한다.
	bool CollideRay( tArray<cSceneNode*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance, eSceneNodeType type );

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool CollideSphere( tArray<cSceneNode*>* pickedArray, const cSphere& sphere, eSceneNodeType type );

	/// 중점을 리턴
	const NiPoint3& GetCenter() const;

	/// 최소 반지름을 리턴
	float GetMinRadius() const;

	/// 최대 반지름을 리턴
	float GetMaxRadius() const;

protected:
	/// 루트 노드
	cSceneTreeNode* mRootNode;

	/// 중점
	NiPoint3 mCenter;

	/// 최소 반지름
	float mMinRadius;

	/// 최대 반지름
	float mMaxRadius;
};

inline
const NiPoint3& cSceneTree::GetCenter() const
{
	return mCenter;
}

inline
float cSceneTree::GetMinRadius() const
{
	return mMinRadius;
}

inline
float cSceneTree::GetMaxRadius() const
{
	return mMaxRadius;
}
