/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.18
* 
* 내    용 : 범용 알고리즘 함수 모음
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <algorithm>

using namespace std;

/// 반복
template<class INPUTITER, class FUNC> inline
FUNC ForEach( INPUTITER first, INPUTITER last, FUNC func )
{
	for( ; first != last; ++first )
	{
		func( *first );
	}
	return func;
}


/// 변환
template<class INPUTITER, class OUTPUTITER, class UNARYOP> inline
UNARYOP Transform( INPUTITER first, INPUTITER last, OUTPUTITER out, UNARYOP op )
{
	for( ; first != last; ++first, ++out )
	{
		*out = op( *first );
	}
	return op;
}

template<class INPUTITER0, class INPUTITER1, class OUTPUTITER, class BINARYOP> inline
BINARYOP Transform( INPUTITER0 first1, INPUTITER0 last1, INPUTITER1 first2, OUTPUTITER out, BINARYOP op )
{
	for( ; first1 != last1; ++first1, ++first2, ++out )
	{
		*out = op( *first1, *first2 );
	}
	return op;
}

/// 세기
template<class INPUTITER, class T> inline
unsigned int Count( INPUTITER first, INPUTITER last, const T& val )
{
	unsigned int n = 0;
	for( ; first != last; ++first )
	{
		if( *first == val )
			++n;
	}
	return n;
}

template<class INPUTITER, class PREDICATE> inline
unsigned int CountIf( INPUTITER first, INPUTITER last, PREDICATE pred )
{
	unsigned int n = 0;
	for( ; first != last; ++first )
	{
		if( pred( *first ) )
			++n;
	}
	return n;
}

/// 검색
template<class INPUTITER, class T> inline
INPUTITER Find( INPUTITER first, INPUTITER last, const T& val )
{
	while( first != last && *first != val )
	{
		++first;
	}
	return first;
}

template<class INPUTITER, class PREDICATE> inline
INPUTITER FindIf( INPUTITER first, INPUTITER last, PREDICATE pred )
{
	while( first != last && !pred(*first) )
	{
		++first;
	}
	return first;
}

/// 정렬
template<class INPUTITER> inline
void Sort( INPUTITER first, INPUTITER last )
{
	sort( first, last );
}

template<class INPUTITER, class COMPARE> inline
void Sort( INPUTITER first, INPUTITER last, COMPARE comp )
{
	sort( first, last, comp);
}
