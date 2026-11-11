/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.15
* 내    용 : std::vector를 감싼 스택 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4267 )
#include <vector>

using namespace std;

template <class T, _STLP_DEFAULT_ALLOCATOR_SELECT(T) >
class tStack;

/// 스택
/// 한번 늘어난 용량(수용가능한 최대 원소 수)은 줄어들지 않는다.
template<class T, class ALLOC>
class tStack : protected vector<T, ALLOC>
{
	typedef tStack<T, ALLOC> cSelf;

public:
	/*
	tStack();
	explicit tStack( unsigned int capacity );

	/// 모든 원소를 제거
	void Clear();

	/// 용량을 설정
	void Reserve( unsigned int capacity );

	/// 원소를 마지막에 추가
	/// 용량을 넘어서면 false를 리턴한다.
	bool PushBack( const T& val );

	/// 마지막 원소를 제거
	/// 원소가 하나도 없는 경우 false를 리턴한다.
	bool PopBack();

	/// 마지막 원소를 리턴
	T& Back();
	const T& Back() const;

	/// 원소 수를 리턴
	unsigned int GetSize() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 용량이 다 찼는지 여부를 리턴
	bool IsFull() const;

	/// 주의: 부하를 야기할 수 있으므로 조심해서 사용한다.
	tStack( const cSelf& other );
	*/

private:
	cSelf& operator = ( const cSelf& other );

#	include "Stack.inl"
};

#pragma warning( pop )
