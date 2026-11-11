/* ======================================================================================
* 파    일 :	RangeCheck.h , RangeCheck.cpp
* 목    적 :	범위 검사
* 작 성 자 :	정원주
* 작 성 일 :    2006년 10월 19일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* ======================================================================================*/

#pragma once

class cRangeCheck
{
private:
	float mSquareRadius;

public:
	void SetRadius( float radius ) { mSquareRadius = radius * radius; }
	bool IsRange( NiPoint3 userpos, NiPoint3 targetpos );
	bool IsNotRange( NiPoint3 userpos, NiPoint3 targetpos );

	cRangeCheck(){};
	cRangeCheck( float radius );
	~cRangeCheck();
};
