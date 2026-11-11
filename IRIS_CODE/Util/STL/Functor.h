/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 범용 함수자 템플릿 모음
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "CharTraits.h"

/// 멤버 함수자
template<class RESULT, class T>
class tMemberFunction
{
	typedef RESULT (T::*Func)( void );

public:
	explicit tMemberFunction( Func func )
		: mFunc( func )
	{
	}

	RESULT operator () ( T* p ) const
	{
		return (p->*mFunc)();
	}

private:
	/// 함수 포인터
	Func mFunc;
};

template<class RESULT, class T> inline
tMemberFunction<RESULT, T> MemFunc(RESULT (T::*f)())
{
	return tMemberFunction<RESULT, T>(f);
}

template<class RESULT, class T>
class tMemberFunctionRef
{
	typedef RESULT (T::*Func)( void );

public:
	explicit tMemberFunctionRef( Func func )
		: mFunc( func )
	{
	}

	RESULT operator () ( T& r ) const
	{
		return (r.*mFunc)();
	}

private:
	/// 함수 레퍼런스
	Func mFunc;
};

template<class RESULT, class T> inline
tMemberFunctionRef<RESULT, T> MemFuncRef(RESULT (T::*f)())
{
	return tMemberFunctionRef<RESULT, T>( f );
}

/// 단항 비교 함수자
template<class T>
class tUnaryEqual
{
public:
	T mVal;

	tUnaryEqual( const T& x )
		: mVal( x )
	{
	}

	bool operator () ( const T& x ) const
	{
		return x == mVal;
	}
};

template<>
class tUnaryEqual<char*>
{
public:
	char* mVal;

	bool operator () ( const char* x ) const
	{
		return tCharTraits<char>::Compare( mVal, x ) == 0;
	}
};

template<>
class tUnaryEqual<const char*>
{
public:
	const char* mVal;

	bool operator () ( const char* x ) const
	{
		return tCharTraits<char>::Compare( mVal, x ) == 0;
	}
};

/// 비교 함수자
template<class T>
class tBinaryEqual
{
public:
	bool operator () ( const T& x, const T& y ) const
	{
		return x == y;
	}
};

template<>
class tBinaryEqual<char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) == 0;
	}
};

template<>
class tBinaryEqual<const char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) == 0;
	}
};

template<class T>
class tBinaryGreater
{
public:
	bool operator () ( const T& x, const T& y ) const
	{
		return x > y;
	}
};

template<>
class tBinaryGreater<char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) > 0;
	}
};

template<>
class tBinaryGreater<const char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) > 0;
	}
};

template<class T>
class tBinaryLess
{
public:
	bool operator () ( const T& x, const T& y ) const
	{
		return x < y;
	}
};

template<>
class tBinaryLess<char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) < 0;
	}
};

template<>
class tBinaryLess<const char*>
{
public:
	bool operator () ( const char* x, const char* y ) const
	{
		return tCharTraits<char>::Compare( x, y ) < 0;
	}
};

/// 선택 함수자
template<class Pair>
class tSelect1st
{
public:
	const typename Pair::FIRST& operator () ( const Pair& x ) const
	{
		return x.mFirst;
	}
};

template<class Pair>
class tSelect2nd
{
public:
	const typename Pair::SECOND& operator () ( const Pair& x ) const
	{
		return x.mSecond;
	}
};

template<class T>
class tIdentity
{
public:
	const T& operator () ( const T& x ) const
	{
		return x;
	}
};
