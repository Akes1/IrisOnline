/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 장면 트리
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Sphere.h"
#include "SceneNode.h"

class cCamera;
class cSceneTree;

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
	virtual ~cSceneTreeNode();

	void* operator new( size_t n ); 
	void operator delete( void* p );

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
	bool CollideRay_Kind();

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool CollideSphere_Kind();
	bool CollideSphere_Type();

protected:
	/// 가시 배열에 추가
	void AddToVisibleArray() const;

	void PushDirect( cSceneNode* node );

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
	typedef tPointerList<void*> cObjectList;
	cObjectList mObjectList;

	/// 메모리 풀
	static tPool<cSceneTreeNode> mPool;

	/// 컬링용
	static NiFrustumPlanes* mFrustum;
	static cSceneNode::eType mPickType;

	/// 충돌 검사용
	static const cRay* mRay;
	static bool mCameraRay;
	static float mMaxDistance;
	static const cSphere* mSphere;
	static tArray<void*>* mCollidedArray;
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
	void Cull( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray, cCamera* cam, cSceneNode::eType type );

	/// 반직선과 충돌하는 장면 노드들을 수집
	/// 삼각형 단위로 검사한다.
	bool CollideRay_Kind( tArray<void*>* collidableArray, const cRay& ray, float maxDistance, bool sortByDistance, cSceneNode::eType type );

	/// 구와 충돌하는 장면 노드들을 수집
	/// 구 단위로 검사한다.
	bool CollideSphere_Kind( tArray<void*>* collidableArray, const cSphere& sphere, cSceneNode::eType type );
	bool CollideSphere_Type( tArray<void*>* collidableArray, const cSphere& sphere, cSceneNode::eType type );

	/// 중점을 리턴
	const NiPoint3& GetCenter() const;

	/// 최소 반지름을 리턴
	float GetMinRadius() const;

	/// 최대 반지름을 리턴
	float GetMaxRadius() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

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

inline
bool cSceneTree::IsEmpty() const
{
	return mRootNode ? false : true;
}
