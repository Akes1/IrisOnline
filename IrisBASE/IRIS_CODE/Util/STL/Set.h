/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.13
* 내    용 : 셑 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4311 )
#pragma warning( disable: 4996 )
#include <set>
#include "Functor.h"

using namespace std;

/// 셑
/// 중복된 키를 허용하지 않는다.
/// 제공된 정렬 기준에 따라서 원소를 자동으로 정렬한다.
template < class T, class COMPARE = less<T>, class ALLOC = allocator<T> >
class tSet
{
	typedef tSet<T, COMPARE, ALLOC> cSelf;
	typedef _STLP_PRIV _SetTraitsT<T> _SetTraits;
	typedef _STLP_PRIV _Rb_tree<T, COMPARE, T, tIdentity<T>, _SetTraits, ALLOC> cTree;

public:
	typedef typename cTree::iterator cIterator;
	typedef typename cTree::const_iterator cConstIterator;
	typedef typename cTree::allocator_type cAllocatorType;

private:
	/// 적흑 트리
	cTree mTree;

public:
	/*
	tSet();

	/// 모든 원소들을 제거
	void Clear();

	/// 해시 테이블의 버킷 크기를 변경
	void Resize( unsigned int hint );

	/// 동일한 값를 가지는 원소의 위치를 리턴
	cIterator Find( const T& val );
	cConstIterator Find( const T& val) const;

	/// 값을 삽입
	/// 실패하면 false를 리턴한다.
	bool Insert( const T& val );

	/// [first, last) 범위의 원소를 삽입
	void Insert( const T* first, const T* last );

	/// [first, last) 범위의 원소를 삽입
	void Insert( cConstIterator first, cConstIterator last );

	/// 해당 값을 가지는 원소를 제거
	/// 제거된 원소의 개수를 리턴한다.
	unsigned int Erase( const T& val );

	/// pos위치의 원소를 제거
	void Erase( cIterator pos );

	/// [first, last) 범위의 원소들을 제거
	void Erase( cIterator first, cIterator last );

	/// 반복자를 리턴
	cIterator Begin();
	cConstIterator Begin() const;
	cIterator End();
	cConstIterator End() const;

	/// 동일한 값을 가지는 원소의 개수를 리턴
	unsigned int GetCount( const T& val ) const;

	/// 원소 수를 리턴
	unsigned int GetSize() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;
	*/

private:
	cSelf& operator = ( const cSelf& other );

#	include "Set.inl"
};

#pragma warning( pop )
