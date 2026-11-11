/* ==========================================================================
* 파    일 : Totem_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-10-21
* 주의사항 :
*===========================================================================*/
#pragma once
#pragma pack(push,1)
#include "BaseObject_Common.h"
#include "CommonNetworkMsgStruct.h"

struct sTotemScript
{
	unsigned long	mTotemIdx;				/// 토템 종류 번호
	unsigned long	mInfluenceIdx;			/// 효과 번호
	cString			mFileName;				/// 연출 파일명
	bool			mDependent;				/// 종속 0 : 비종속 1 구분
	unsigned long	mContinuanceTime;		/// 토템 유지시간
	unsigned long	mApplyTime;				/// 적용시간(주기)
	unsigned short	mAttackerRange;			/// 공격자와의 유지 거리
	unsigned short	mApplyRange1;			/// 적용 범위1
	unsigned short	mApplyRange2;			/// 적용 범위2
};

struct MSG_TOTEM_INFO : public MSGROOT
{
	unsigned long	mObjectIdx;				/// 토템 고유번호
    float			mPosX;					/// 생성 좌표
	float			mPosY;
	float			mDirection;				/// 방향
	unsigned long	mCreateInfClassIdx;		/// 토템 생성 효과 번호
};

struct MSG_TOTEMIDX : public MSGROOT
{
	unsigned long	mObjectIdx;				/// 토템 고유번호
};


#pragma pack(pop)
