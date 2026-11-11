/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.13
* 내    용 : 맵 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4311 )
#pragma warning( disable: 4996 )
#include <map>
#include "Pair.h"
#include "Functor.h"

using namespace std;

/// 맵
/// 키/값 쌍을 하나의 원소로 취급하는 컨테이너이다.
/// 중복된 키를 허용하지 않는다.
/// 제공된 정렬 기준에 따라서 원소를 자동으로 정렬한다.
template <class KEY, class T, class COMPARE = less<KEY>, class ALLOC = allocator< pair<KEY, T> > >
class tMap
{
public:
	typedef typename tPair<KEY, T> cPair;

private:
	typedef tMap<KEY, T, COMPARE, ALLOC> cSelf;
	typedef _STLP_PRIV _MapTraitsT<cPair> _MapTraits;
	typedef _STLP_PRIV _Rb_tree<KEY, COMPARE, cPair, tSelect1st<cPair>, _MapTraits, ALLOC> cTree;

public:
	typedef typename cTree::iterator cIterator;
	typedef typename cTree::const_iterator cConstIterator;
	typedef typename cTree::allocator_type cAllocatorType;

private:
	/// 적흑 트리
	cTree mTree;

public:
	/*
	tMap();

	/// 모든 원소들을 제거
	void Clear();

	/// 동일한 키를 가지는 첫번째 원소의 위치를 리턴
	/// 원소가 존재하지 않는다면 End()를 리턴
	cIterator Find( const KEY& key );
	cConstIterator Find( const KEY& key) const;

	/// 해당 키에 값을 삽입
	/// 실패하면 false를 리턴한다.
	bool Insert( const KEY& key, const T& val );

	/// [first, last) 범위의 원소를 삽입
	void Insert( const cPair* first, const cPair* last );

	/// [first, last) 범위의 원소를 삽입
	void Insert( cConstIterator first, cConstIterator last );

	/// 해당 키를 가지는 원소를 제거
	/// 제거된 원소의 개수를 리턴한다.
	unsigned int Erase( const KEY& key );

	/// pos위치의 원소를 제거
	void Erase( cIterator pos );

	/// [first, last) 범위의 원소들을 제거
	void Erase( cIterator first, cIterator last );

	/// 반복자를 리턴
	cIterator Begin();
	cConstIterator Begin() const;
	cIterator End();
	cConstIterator End() const;

	/// key에 해당하는 값을 얻어옴
	/// 실패하면 false를 리턴한다.
	bool GetAt( T* val, const KEY& key ) const;

	/// 동일한 key를 가지는 원소의 개수를 리턴
	unsigned int GetCount( const KEY& key ) const;

	/// 원소 수를 리턴
	unsigned int GetSize() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 참조 연산
	/// 주의: 원소의 삽입이 아닌 값을 얻는 용도로만 안전하게 사용해야 한다.
	T& operator [] ( const KEY& key );

	/// 주의: 부하를 야기할 수 있으므로 조심해서 사용한다.
	tMap( const cSelf& other );
	*/

private:
	cSelf& operator = ( const cSelf& other );

#	include "Map.inl"
};

#pragma warning( pop )
