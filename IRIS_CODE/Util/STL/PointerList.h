/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.06
* 내    용 : std::list를 감싼 포인터 리스트 템플릿
* 주의사항 : 널 포인터가 원소가 되는 것을 허용하지 않는다.
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4267 )
#include <list>

using namespace std;

template <class T, _STLP_DEFAULT_ALLOCATOR_SELECT(T) >
class tPointerList;

/// 포인터 리스트
/// 널 포인터가 원소가 되는 것을 허용하지 않는다.
template<class T, class ALLOC>
class tPointerList : protected list<T, ALLOC>
{
	typedef list<T, ALLOC> list;
	typedef tPointerList<T, ALLOC> cSelf;

public:
	typedef list::iterator cIterator;
	typedef list::const_iterator cConstIterator;

public:
	/*
	explicit tPointerList();

	/// 모든 원소를 제거
	void Clear();

	/// pos 위치에 원소를 삽입
	/// 새로운 원소의 위치를 리턴한다.
	cIterator Insert( cIterator pos, const T val );

	/// 맨 앞에 원소를 추가
	bool PushFront( const T val );

	/// 맨 뒤에 원소를 추가
	bool PushBack( const T val );

	/// pos 위치의 원소를 제거
	/// 다음 원소의 위치를 리턴한다.
	cIterator Erase( cIterator pos );

	/// [first, last) 범위의 모든 원소를 제거
	/// 다음 원소의 위치를 리턴한다.
	cIterator Erase( cIterator first, cIterator last );

	/// 맨 앞의 원소를 제거
	void PopFront();

	/// 맨 끝의 원소를 제거
	void PopBack();

	/// 리스트 other의 모든 원소를 pos 위치 앞으로 이동
	void Splice( cIterator pos, cSelf& other );

	/// 리스트 other의 [first, last) 범위의 모든 원소를 pos 위치 앞으로 이동
	void Splice( cIterator pos, cSelf& other, cIterator first, cIterator last );

	/// 리스트 other의 i에 있는 원소를 pos 위치 앞으로 이동
	void Splice( cIterator pos, cSelf& other, cIterator i );

	/// 값이 val인 모든 원소를 제거
	void Remove( const T val );

	/// 함수자 op가 true를 리턴하는 모든 원소를 제거
	template<class Predicate>
	void RemoveIf( Predicate op );

	/// < 연산자를 정렬 기준으로 정렬
	void Sort();

	/// 함수자 op를 기준으로 정렬
	template<class StrictWeakOrdering>
	void Sort( StrictWeakOrdering op );

	/// 반복자를 리턴
	cIterator Begin();
	cConstIterator Begin() const;
	cIterator End();
	cConstIterator End() const;

	/// 원소를 리턴
	T& Front();
	const T& Front() const;
	T& Back();
	const T& Back() const;

	/// 원소 수를 리턴
	unsigned int GetSize() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 주의: 부하를 야기할 수 있으므로 조심해서 사용한다.
	tPointerList( const cSelf& other );
	*/

private:
	cSelf& operator = ( const cSelf& other );

private:
	/// 주의: 아래 함수들은 사용 빈도가 낮거나 부하를 야기하므로 컴파일 금지!

	/// 모든 원소들의 순서를 뒤바꿈
	void Reverse();

	/// 같은 값을 가지는 연속된 원소들의 중복을 제거
	/// 예) 1, 2, 3, 3, 4, 3 -> 1, 2, 3, 4, 3
	void Unique();

	/// op가 true를 리턴하는 연속된 원소들의 중복을 제거
	template<class BinaryPredicate>
		void Unique( BinaryPredicate op );

	/// 이미 정렬된 두 리스트를 함수자 op를 기준으로 정렬 병합
	template<class StrictWeakOrdering>
		void Merge( cSelf& other, StrictWeakOrdering op );

#	include "PointerList.inl"
};

#pragma warning( pop )
