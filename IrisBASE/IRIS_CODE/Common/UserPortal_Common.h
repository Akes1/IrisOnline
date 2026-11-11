#pragma once
#pragma pack(push,1)
#include "BaseObject_Common.h"
#include "CommonNetworkMsgStruct.h"

const int USERPORTAL_ALIVE_TIME = 30 * SECOND;

struct MSG_USERPORTAL_CREATE : public MSGROOT
{
	unsigned long	mObjectIdx;				/// 포탈 고유번호
	unsigned long	mMapChangePosIdx;		/// 맵이동 좌표 스크립트 인덱스
	unsigned long	mPlayerIdx;				/// 포탈 생성자
    float			mPosX;					/// 생성 좌표
	float			mPosY;
};

struct MSG_USERPORTAL_DELETE : public MSGROOT
{
	unsigned long	mObjectIdx;				/// 포탈 고유번호
};

struct MSG_REQ_USERPORTAL_MAPCHANGE : public MSGROOT
{
	unsigned long	mUserPortalObjectIdx;
};

#pragma pack(pop)
