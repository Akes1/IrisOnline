/* ==========================================================================
* 파    일 : Level_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-05-15
* 주의사항 :
*===========================================================================*/
#pragma once

#pragma pack(push,1)



/// 경험치 테이블
struct sExpTable
{
	unsigned char	mLevel;
	unsigned long	mExp;
	unsigned long	mAccumulationExp;
};



/// SP 테이블
struct sSxpTable
{
	unsigned char	mLevel;
	unsigned long	mSxp;
	unsigned long	mAccumulationSxp;
};



#pragma pack(pop)