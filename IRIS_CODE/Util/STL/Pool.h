/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.17
* 내    용 : 메모리 풀
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Allocator.h"
#include "HashSet.h"

template<class T>
class tPool;

/// 메모리 풀 노드
template<class T, class ALLOC = tAllocator<T> >
class tPoolNode
{
	typedef tPoolNode<T, ALLOC> cSelf;
	friend class tPool<T>;

protected:
	tPoolNode( unsigned int capacity );
	~tPoolNode();

	/// 오브젝트를 리턴
	T* GetObject( unsigned int i );

protected:
	/// 오브젝트 배열
	T* mpObjectArray;

	/// 크기 (오브젝트 수)
	unsigned int mCapacity;

	/// 다음 노드
	cSelf* mpNext;

private:
	tPoolNode( const cSelf& );
	cSelf& operator = ( const cSelf& );
};

/// 메모리 풀
template<class T>
class tPool
{
	typedef tPool<T> cSelf;
	typedef tPoolNode<T> cNode;

public:
	explicit tPool( unsigned int initialCapacity = 0, unsigned int stepCapacity = 0, unsigned int hashHint = 1000 );
	~tPool();

	/// 할당된 메모리를 모두 제거
	void Clear();

	/// 용량을 설정
	void Reserve( unsigned int capacity, unsigned int stepCapacity = 0 );

	/// 할당
	T* Alloc();

	/// 해제
	void Free( T* p );

	/// 크기(할당된 오브젝트 수)를 리턴
	unsigned int GetSize() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;

protected:
	/// 새로운 노드를 생성
	bool CreateNode( unsigned int capacity );

protected:
	/// 자유 오브젝트 셑
	typedef tHashSet<T*> cObjectSet;
	cObjectSet mFreeObjectSet;

	/// 루트 노드
	cNode* mpRootNode;

	/// 크기
	unsigned int mSize;

	/// 용량
	unsigned int mCapacity;

	/// 초기 용량
	unsigned int mInitialCapacity;

	/// 증가 용량
	unsigned int mStepCapacity;

private:
	tPool( const cSelf& );
	cSelf& operator = ( const cSelf& );
};

#include "Pool.inl"
