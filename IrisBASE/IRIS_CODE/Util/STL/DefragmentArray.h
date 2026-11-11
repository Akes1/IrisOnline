/* ==========================================================================
* 파    일 : DefragmentArray.h
* 목    적 : 앞에서부터 데이터 끝 위치까지 빈 공간이 없는 배열형태의 자료구조
* 작 성 자 : 박경희
* 작 성 일 : 2006-11-13
* 
* 추가내용 :
*   추가날짜 작업자 추가내용
*
* 주의사항 : 정렬하지않음, 포인터 형태만 사용하는 것이 좋음
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4267 )
#include <vector>

using namespace std;

/// 배열
/// 한번 늘어난 용량(수용가능한 최대 원소 수)은 줄어들지 않는다.

template<class T>
class tDefragmentArray : protected vector<T>
{
	typedef tDefragmentArray<T> cSelf;
	unsigned int mCount;

public:
	typedef T* cIterator;
	typedef const T* cConstIterator;

public:

	explicit tDefragmentArray();
	explicit tDefragmentArray( unsigned int size );
	tDefragmentArray( const cSelf& other );

	/// 반복자를 리턴
	cIterator Begin();
	cConstIterator Begin() const;
	cIterator End();
	cConstIterator End() const;


	/// 참조 연산자
	/// 주의: 인덱스의 범위를 검사하지 않는다.
	T& operator [] ( unsigned int i );
	const T& operator [] ( unsigned int i ) const;

	/// 데이터가 들어 있는 갯수 출력
	unsigned int Count();
	/// 자료 입력
	void Insert(T val);
	/// 자료 삭제
	bool Erase(T val);
	/// 자료구조 데이타 모두 삭제
	void Clear();
	
private:
	cSelf& operator = ( const cSelf& other );

};

#include "DefragmentArray.inl"
#pragma warning( pop )
