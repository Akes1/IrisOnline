/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.13
* 내    용 : 해시 함수자 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "CharTraits.h"

/// 해시 함수자
template<class T>
class tHasher
{
public:
	unsigned int operator () ( const T& val ) const
	{
		return (unsigned int)val;
	}
};

template<>
class tHasher<char*>
{
public:
	unsigned int operator () ( const char* str ) const
	{
		return tCharTraits<char>::GetHashCode( str );
	}
};

template<>
class tHasher<const char*>
{
public:
	unsigned int operator () ( const char* str ) const
	{
		return tCharTraits<char>::GetHashCode( str );
	}
};

template<>
class tHasher<char>
{
public:
	unsigned int operator () ( char val ) const
	{
		return val;
	}
};

template<>
class tHasher<unsigned char>
{
public:
	unsigned int operator () ( unsigned char val ) const
	{
		return val;
	}
};

template<>
class tHasher<short>
{
public:
	unsigned int operator () ( short val ) const
	{
		return val;
	}
};

template<>
class tHasher<unsigned short>
{
public:
	unsigned int operator () ( unsigned short val ) const
	{
		return val;
	}
};

template<>
class tHasher<int>
{
public:
	unsigned int operator () ( int val ) const
	{
		return val;
	}
};

template<>
class tHasher<unsigned int>
{
public:
	unsigned int operator () ( unsigned int val ) const
	{
		return val;
	}
};

template<>
class tHasher<long>
{
public:
	unsigned int operator () ( long val ) const
	{
		return val;
	}
};

template<>
class tHasher<unsigned long>
{
public:
	unsigned int operator () ( unsigned long val ) const
	{
		return val;
	}
};
