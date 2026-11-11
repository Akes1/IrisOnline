#include <assert.h>

template<class T> inline
tDefragmentArray<T>::tDefragmentArray()
: vector<T>()
{
	mCount = 0;
}

template<class T> inline
tDefragmentArray<T>::tDefragmentArray( unsigned int size )
: vector<T>( size )
{
	mCount = 0;
}

template<class T> inline
tDefragmentArray<T>::tDefragmentArray( const cSelf& other )
: vector<T>( other )
{
	mCount = 0;
}

template<class T> inline
T* tDefragmentArray<T>::Begin()
{
	return begin();
}

template<class T> inline
const T* tDefragmentArray<T>::Begin() const
{
	return begin();
}

template<class T> inline
T* tDefragmentArray<T>::End()
{
	return end();
}

template<class T> inline
const T* tDefragmentArray<T>::End() const
{
	return end();
}


template<class T> inline
T& tDefragmentArray<T>::operator [] ( unsigned int i )
{
	return *(begin() + i);
}

template<class T> inline
const T& tDefragmentArray<T>::operator [] ( unsigned int i ) const
{
	return *(begin() + i);
}

template<class T> inline
unsigned int tDefragmentArray<T>::Count() 
{ 
	return mCount; 
}

template<class T> inline
void tDefragmentArray<T>::Insert(T val) 
{ 
	/// 추가될경우 카운트 1증가
	push_back(val); 
	++mCount; 
}

template<class T> inline
bool tDefragmentArray<T>::Erase(T val) 
{ 
	if( mCount <= 0 )
	{
		/// 0이하 삭제시 오류
		return false;
	}

	cIterator i = begin();
	cIterator End = end();
	/// 마지막위치에 해당하는 데이터의 값
	cIterator lastdata = end();
	--lastdata;

	/// 정상 삭제 유무 체크
	bool check=false;

	for( ; i != End; ++i )
	{
		/// 입력한 값이 있는 경우
		if( val == (*i) )
		{
			/// 벡터의 마지막 값을 현재 위치로 복사 (마지막은 후에 지움)
			*i = *lastdata;
			check = true;
			break;
		}
		check = false;
	}

	/// 검색 되지 않은경우 false
	if( check == false ) 
		return false;

	/// 검색 된경우 마지막의 값을 지워버린다.
	pop_back(); 
	--mCount; 
	return true;
}

template<class T> inline
void tDefragmentArray<T>::Clear() 
{
	cIterator i = begin();
	cIterator End = end();

	for( ; i != End; ++i )
	{
		pop_back(); 
		
	}
	mCount = 0;
}
