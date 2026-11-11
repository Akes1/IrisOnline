/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : 포인터 큐 템플릿
* 주의사항 : 널 포인터가 원소가 되는 것을 허용하지 않는다.
*===========================================================================*/
#pragma once

#include "Allocator.h"
#include "Element.h"

/// 포인터 큐
/// 널 포인터가 원소가 되는 것을 허용하지 않는다.
/// 한번 늘어난 용량(수용가능한 최대 원소 수)은 줄어들지 않는다.
template<class T, class ALLOC = tAllocator<T> >
class tPointerQueue
{
	typedef tPointerQueue<T, ALLOC> cSelf;
	static const unsigned int NPOS = ~(unsigned int)0;

public:
	/*
	tPointerQueue();
	explicit tPointerQueue( unsigned int capacity );
	~tPointerQueue();

	/// 모든 원소를 제거
	void Clear();

	/// 용량을 설정
	void Reserve( unsigned int capacity );

	/// 원소를 마지막에 추가
	/// 용량을 넘어서면 false를 리턴한다.
	bool PushBack( const T val );

	/// 첫번째 원소를 제거
	/// 원소가 하나도 없는 경우 false를 리턴한다.
	bool PopFront();

	/// 첫번째 원소를 리턴
	/// T Front();
	const T Front() const;

	/// 마지막 원소를 리턴
	T Back();
	const T Back() const;

	/// 원소 수를 리턴
	unsigned int GetSize() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 용량이 다 찼는지 여부를 리턴
	bool IsFull() const;

	/// 참조 연산자
	T operator [] ( unsigned int i );
	const T operator [] ( unsigned int i ) const;
	*/

private:
	tPointerQueue( const cSelf& other );
	cSelf& operator = ( const cSelf& other );

private:
	unsigned int mHead;
	unsigned int mTail;

	/// 끝 인덱스 (용량)
	unsigned int mEnd;

	/// 버퍼
	T* mpBuffer;

#	include "PointerQueue.inl"
};
