/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.12
* 
* 내    용 : 메모리 할당자 템플릿
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <new>

/// 메모리 할당자
/// 단순히 메모리를 할당, 해제하는 역할을 수행한다.
template<class T>
class tAllocator
{
public:
	/// n개의 원소에 대해 필요한 메모리를 할당
	static T* Allocate( unsigned int n )
	{
		if( n )
		{
			return (T*)operator new( n * sizeof(T) );
		}
		return 0;
	}

	/// 메모리를 해제
	static void Deallocate( T* p )
	{
		operator delete( p );
	}
};
