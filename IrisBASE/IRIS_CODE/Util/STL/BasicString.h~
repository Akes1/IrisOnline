/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.12
* 내    용 : std::basic_string을 감싼 기본 스트링 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
//#pragma warning( disable: 4996 )
#pragma warning( disable: 4245 )
#pragma warning( disable: 4267 )
#include <string>
#include "CharTraits.h"

using namespace std;

template <class T, class TRAITS, _STLP_DEFAULT_ALLOCATOR_SELECT(T) >
class tBasicString;

/// 문자열 기반
template<class T, class TRAITS, class ALLOC>
class tBasicString : protected basic_string<T, TRAITS, ALLOC>
{
	typedef basic_string<T, TRAITS, ALLOC> basic_string;
	typedef tBasicString<T, TRAITS, ALLOC> cSelf;
	static const int BUFFER_SIZE = 1024;

public:
	typedef T* cIterator;
	static const unsigned int NPOS = (unsigned int)basic_string::npos;

public:
	/*
	explicit tBasicString();
	tBasicString( const cSelf& s );
	tBasicString( const cSelf& s, unsigned int pos, unsigned int n = NPOS );
	tBasicString( const T* s );
	tBasicString( const T* s, unsigned int n );
	tBasicString( const T* first, const T* last );
	tBasicString( unsigned int n, T c );
	explicit tBasicString( unsigned int n );

	/// 모든 글자를 제거
	void Clear();

	/// 용량을 설정
	void Reserve( unsigned int n );

	/// 글자를 마지막에 추가
	void PushBack( T c );

	/// 마지막 글자를 제거
	void PopBack();

	/// 글자 수를 설정
	/// 메모리 용량이 글자 수에 맞게 변한다.
	void Resize( unsigned int n );
	void Resize( unsigned int n, T c );

	/// 문자열을 뒤에 추가
	void Append( cIterator first, cIterator last );
	void Append( const cSelf& s );
	bool Append( const cSelf& s, unsigned int pos, unsigned int n );
	void Append( const T* s );
	void Append( const T* s, unsigned int n );

	/// 글자를 마지막에 추가
	void Append( T c );

	/// 해당 위치에 문자열을 삽입
	bool Insert( unsigned int pos, const cSelf& s );
	bool Insert( unsigned int pos, const cSelf& s, unsigned int beg, unsigned int n );
	bool Insert( unsigned int pos, const T* s, unsigned int n );
	bool Insert( unsigned int pos, const T* s );

	/// [first, last) 범위의 문자열을 삭제
	void Erase( cIterator first, cIterator last );
	bool Erase( unsigned int pos = 0, unsigned int n = NPOS );

	/// 왼쪽 공백 제거
	void TrimLeft();
	void TrimLeft( const T* s );
	void TrimLeft( T c );

	/// 오른쪽 공백 제거
	void TrimRight();
	void TrimRight( const T* s );
	void TrimRight( T c );

	/// 문자열 교체
	void Replace( cIterator first0, cIterator last0, cIterator first1, cIterator last1 );
	bool Replace( unsigned int pos, unsigned int n, const cSelf& s );
	bool Replace( unsigned int pos0, unsigned int n0, const cSelf& s, unsigned int pos1, unsigned int n1 );
	bool Replace( unsigned int pos, unsigned int n0, const T* s, unsigned int n1 );
	bool Replace( unsigned int pos, unsigned int n, const T* s );
	void Replace( cIterator first, cIterator last, const cSelf& s );
	void Replace( cIterator first, cIterator last, const T* s, unsigned int n );
	void Replace( cIterator first, cIterator last, const T* s );
	void Replace( const cSelf& oldStr, const cSelf& newStr );
	void Replace( const T* oldStr, const T* newStr );
	void Replace( T oldChar, T newChar );

	/// 검색
	unsigned int Find( const cSelf& s, unsigned int pos = 0 ) const;
	unsigned int Find( const T* s, unsigned int pos = 0 ) const;
	unsigned int Find( T c, unsigned int pos = 0 ) const;
	unsigned int FindReverse( const cSelf& s, unsigned int pos = NPOS ) const;
	unsigned int FindReverse( const T* s, unsigned int pos = NPOS ) const;
	unsigned int FindReverse( T c, unsigned int pos = NPOS ) const;

	/// 문자열 s의 부분인 첫번째 문자를 검색
	unsigned int FindFirstof( const cSelf& s, unsigned int pos = 0 ) const;
	unsigned int FindFirstof( const T* s, unsigned int pos = 0 ) const;

	/// 문자열 s의 부분인 마지막 문자를 검색
	unsigned int FindLastof( const cSelf& s, unsigned int pos = NPOS ) const;
	unsigned int FindLastof( const T* s, unsigned int pos = NPOS ) const;

	/// 문자열 s의 부분이 아닌 첫번째 문자의 위치를 리턴
	unsigned int FindFirstNotof( const cSelf& s, unsigned int pos = 0 ) const;
	unsigned int FindFirstNotof( const T* s, unsigned int pos = 0 ) const;

	/// 문자 c와 같은 첫번째 문자의 위치를 리턴
	unsigned int FindFirstNotof( T c, unsigned int pos = 0 ) const;

	/// 문자열 s의 부분이 아닌 마지막 문자의 위치를 리턴
	unsigned int FindLastNotof( const cSelf& s, unsigned int pos = NPOS ) const;
	unsigned int FindLastNotof( const T* s, unsigned int pos = NPOS ) const;

	/// 문자 c가 아닌 마지막 문자의 위치를 리턴
	unsigned int FindLastNotof( T c, unsigned int pos = NPOS ) const;

	/// 부분 문자열을 리턴
	cSelf SubStr( unsigned int pos = 0, unsigned int n = NPOS ) const;
	cSelf Mid( unsigned int pos ) const;
	cSelf Mid( unsigned int pos, unsigned int count ) const;
	cSelf Left( unsigned int count ) const;
	cSelf Right( unsigned int count ) const;

	/// 비교
	int Compare( const cSelf& s ) const;
	int Compare( const cSelf& s, unsigned int pos, unsigned int len ) const;
	int Compare( unsigned int pos, unsigned int n, const cSelf& s ) const;
	int Compare( unsigned int pos0, unsigned int n0, const cSelf& s, unsigned int pos1, unsigned int n1 ) const;
	int Compare( const T* s ) const;
	int Compare( const T* s, unsigned int pos, unsigned int len ) const;
	int Compare( unsigned int pos, unsigned int n, const T* s ) const;
	int Compare( unsigned int pos, unsigned int n, const T* s, unsigned int n1 ) const;
	int CompareNoCase( const cSelf& s ) const;
	int CompareNoCase( const cSelf& str, unsigned int pos, unsigned int len ) const;
	int CompareNoCase( const T* str ) const;
	int CompareNoCase( const T* str, unsigned int pos, unsigned int len ) const;

	/// 형식화 입력
	void Format( const T* fmt, ... );
	void FormatV( const T* fmt, va_list vl );

	/// 변환
	void ToUpper();
	void ToLower();
	int ToInt() const;
	float ToFloat() const;

	/// 반복자를 리턴
	cIterator Begin();
	const T* Begin() const;
	cIterator End();
	const T* End() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 해시 코드를 리턴
	operator unsigned int() const;
	unsigned int GetHashCode() const;

	/// 크기(길이)를 리턴
	unsigned int GetSize() const;
	unsigned int GetLength() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;

	/// 0으로 끝나는 문자열을 리턴
	const T* Cstr() const;

	/// 대입 연산자
	cSelf& operator = ( const cSelf& s );
	cSelf& operator = ( const T* s );
	cSelf& operator = ( T c );
	cSelf& operator += ( const cSelf& s );
	cSelf& operator += ( const T* s );
	cSelf& operator += ( T c );

	/// 참조 연산자
	T& operator [] ( unsigned int i );
	const T& operator [] ( unsigned int i ) const;
	*/

#	include "BasicString.inl"
};

template<class T, class TRAITS, class ALLOC> inline
tBasicString<T, TRAITS, ALLOC> operator + ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	typedef tBasicString<T, TRAITS, ALLOC> Str;
	Str out( x.GetSize() + y.GetSize() );
	out.Append( x );
	out.Append( y );
	return out;
}

template<class T, class TRAITS, class ALLOC> inline
tBasicString<T, TRAITS, ALLOC> operator + ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	typedef tBasicString<T, TRAITS, ALLOC> Str;
	unsigned int n = TRAITS::length( s );
	Str out( n + y.GetSize() );
	out.Append( s );
	out.Append( y );
	return out;
}

template<class T, class TRAITS, class ALLOC> inline
tBasicString<T, TRAITS, ALLOC> operator + ( T c, const tBasicString<T, TRAITS, ALLOC>& y )
{
	typedef tBasicString<T, TRAITS, ALLOC> Str;
	Str out( 1 + y.GetSize() );
	out.PushBack( c );
	out.Append( y );
	return out;
}

template<class T, class TRAITS, class ALLOC> inline
tBasicString<T, TRAITS, ALLOC> operator + ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	typedef tBasicString<T, TRAITS, ALLOC> Str;
	unsigned int n = TRAITS::length( s );
	Str out( x.GetSize() + n );
	out.Append( x );
	out.Append( s );
	return out;
}

template<class T, class TRAITS, class ALLOC> inline
tBasicString<T, TRAITS, ALLOC> operator + ( const tBasicString<T, TRAITS, ALLOC>& x, const T c )
{
	typedef tBasicString<T, TRAITS, ALLOC> Str;
	Str out( x.GetSize() + 1 );
	out.Append( x );
	out.PushBack( c );
	return out;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator == ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return x.GetSize() == y.GetSize() && TRAITS::Compare( x.Cstr(), y.Cstr(), x.GetSize() ) == 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator == ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	unsigned int n = TRAITS::length( s );
	return n == y.GetSize() && TRAITS::Compare( s, y.Cstr(), n ) == 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator == ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	unsigned int n = TRAITS::length( s );
	return x.GetSize() == n && TRAITS::Compare( x.Cstr(), s, n ) == 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator < ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return TRAITS::Compare( x.Begin(), y.Begin() ) < 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator < ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return TRAITS::Compare( s, y.Begin() ) < 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator < ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	return TRAITS::Compare( x.Begin(), s ) < 0;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator != ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(x == y);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator > ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return y < x;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator <= ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(y < x);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator >= ( const tBasicString<T, TRAITS, ALLOC>& x, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(x < y);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator != ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(s == y);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator != ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	return !(x == s);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator > ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return y < s;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator > ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	return s < x;
}

template<class T, class TRAITS, class ALLOC> inline
bool operator <= ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(y < s);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator <= ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s )
{
	return !(s < x);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator >= ( const T* s, const tBasicString<T, TRAITS, ALLOC>& y )
{
	return !(s < y);
}

template<class T, class TRAITS, class ALLOC> inline
bool operator >= ( const tBasicString<T, TRAITS, ALLOC>& x, const T* s  )
{
	return !(x < s);
}

#pragma warning( pop )

/// 문자열
typedef tBasicString<char, tCharTraits<char> > cString;
typedef tBasicString<wchar_t, tCharTraits<wchar_t> > cStringW;

#ifdef _UNICODE
;	typedef wchar_t charT;
;	typedef	cStringW cStringT;
#else
;	typedef char charT;
;	typedef	cString cStringT;	
#endif //_UNICODE
