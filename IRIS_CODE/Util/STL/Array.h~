/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : std::vector를 감싼 배열 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4267 )
#include <vector>

using namespace std;

template <class T, _STLP_DEFAULT_ALLOCATOR_SELECT(T) >
class tArray;

/// 배열
/// 한번 늘어난 용량(수용가능한 최대 원소 수)은 줄어들지 않는다.
template<class T, class ALLOC>
class tArray : protected vector<T, ALLOC>
{
	typedef vector<T, ALLOC> vector;
	typedef tArray<T, ALLOC> cSelf;

public:
	typedef T* cIterator;
	typedef const T* cConstIterator;

public:
	/*
	explicit tArray();
	explicit tArray( unsigned int size );
	tArray( unsigned int size, const T& val );
	tArray( const cSelf& other );

	/// 모든 원소를 제거
	void Clear();

	/// n개의 값을 할당
	void Assign( unsigned int n, const T& val );

	/// [first, last) 범위의 값을 할당
	void Assign( cIterator first, cIterator last );

	/// 최대로 수용가능한 원소의 수를 설정
	void Reserve( unsigned int capacity );

	/// 원소를 마지막에 추가
	void PushBack( const T& val );

	/// 마지막 원소를 제거
	bool PopBack();

	/// 해당 인덱스의 원소를 제거
	/// 주의: 해당 위치에 마지막 원소를 복사한 후, 마지막 원소를 제거하므로
	/// 원소를 삽입한 순서가 바뀐다.
	/// 마지막 원소를 제거한 경우에만 순서가 유지된다.
	bool PopAt( unsigned int i );
	bool PopAt( bool* orderChanged, unsigned int i );

	/// 실제 원소 수를 설정
	void Resize( unsigned int size );
	void Resize( unsigned int size, const T& val );

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

	/// 해당 인덱스의 원소를 리턴
	/// 인덱스가 적합한 범위를 벗어나면 false를 리턴한다.
	bool GetAt( T* val, unsigned int i );

	/// 크기(원소 수)를 리턴
	unsigned int GetSize() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 참조 연산자
	/// 주의: 인덱스의 범위를 검사하지 않는다.
	T& operator [] ( unsigned int i );
	const T& operator [] ( unsigned int i ) const;
	*/

private:
	cSelf& operator = ( const cSelf& other );

private:
	/// 주의: 아래 함수들은 사용 빈도가 낮거나 부하를 야기하므로 컴파일 금지!

	/// 해당 위치에 값을 삽입
	cIterator Insert( cIterator pos, const T& val );

	/// 해당 위치에 [first, last) 범위의 값을 삽입
	void Insert( cIterator pos, cIterator first, cIterator last );

	/// 해당 위치에 n개의 값을 삽입
	void Insert( cIterator pos, unsigned int n, const T& val );

	/// 해당 위치의 원소를 삭제
	cIterator Erase( cIterator pos );

	/// [first, last) 범위의 원소를 삭제
	cIterator Erase( cIterator first, cIterator last );

#	include "Array.inl"
};

#pragma warning( pop )
