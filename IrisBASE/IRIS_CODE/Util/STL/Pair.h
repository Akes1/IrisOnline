/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.12
* 내    용 : 쌍 템플릿
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 쌍
template<class T0, class T1>
class tPair
{
	typedef tPair<T0, T1> cSelf;

public:
	typedef T0 FIRST;
	typedef T1 SECOND;

public:
	FIRST mFirst;
	SECOND mSecond;

public:
	tPair()
	{
	}

	tPair( const T0& first, const T1& second )
		: mFirst( first )
		, mSecond( second )
	{
	}

	tPair( const cSelf& y )
		: mFirst( y.mFirst )
		, mSecond( y.mSecond )
	{
	}

	bool operator == ( const cSelf& y )
	{
		return mFirst == y.mFirst && mSecond == y.mSecond;
	}

	bool operator != ( const cSelf& y )
	{
		return !operator == ( y );
	}

	bool operator < ( const cSelf& y )
	{
		return mFirst < y.mFirst || (!(y.mFirst < mFirst) && mSecond < y.mSecond);
	}

	bool operator > ( const cSelf& y )
	{
		return y < *this;
	}

	bool operator <= ( const cSelf& y )
	{
		return !(y < *this);
	}

	bool operator >= ( const cSelf& y )
	{
		return !operator < (y);
	}
};
