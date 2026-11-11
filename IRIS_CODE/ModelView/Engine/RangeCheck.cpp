#include "stdafx.h"
#include "RangeCheck.h"

cRangeCheck::cRangeCheck( float radius )
: mSquareRadius( radius * radius )
{
}

cRangeCheck::~cRangeCheck()
{
}


/// 범위 안에 있는지 검사
bool cRangeCheck::IsRange( NiPoint3 userpos,NiPoint3 targetpos )
{
	if( mSquareRadius == 0 )
	{
		return true;
	}
	if( (targetpos - userpos).SqrLength() <= mSquareRadius )
	{
		return true;
	}

	return false;
}

/// 범위 밖에 있는지 검사
bool cRangeCheck::IsNotRange( NiPoint3 userpos,NiPoint3 targetpos )
{
	if( mSquareRadius == 0 )
	{
		return false;
	}
	if( (targetpos - userpos).SqrLength() > mSquareRadius )
	{
		return true;
	}

	return false;
}