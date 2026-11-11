/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.11
* 내    용 : std::hashtable를 포함한 해시셑 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4311 )
//#pragma warning( disable: 4996 )
#include <hash_set>
#include "Hasher.h"
#include "Functor.h"

using namespace std;

/// 해시 셑
/// 중복된 키를 허용하지 않는다.
template<class T, class HASHER = tHasher<T>, class EQUAL = tBinaryEqual<T> >
class tHashSet
{
	typedef tHashSet<T, HASHER, EQUAL> cSelf;
	typedef _STLP_PRIV _HashSetTraitsT<T> _HashSetTraits;
	typedef hashtable<T, T, HASHER, _HashSetTraits, tIdentity<T>, EQUAL, allocator<T> > cTable;

public:
	typedef typename cTable::iterator cIterator;
	typedef typename cTable::const_iterator cConstIterator;
	typedef typename cTable::hasher cHasher;
	typedef typename cTable::key_equal cKeyEqual;
	typedef typename cTable::allocator_type cAllocatorType;

private:
	/// 해시 테이블
	cTable mTable;

public:
	/*
	tHashSet();
	explicit tHashSet( unsigned int hint );

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

	/// 070208 웅주 인라인 파일에 선언되어 있어 주석처리 했어요
	//tHashSet( const cSelf& other );

private:
	cSelf& operator = ( const cSelf& other );

#	include "HashSet.inl"
};

#pragma warning( pop )
