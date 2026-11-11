#pragma once
#pragma pack(push,1)

#include "CommonNetworkMsgStruct.h"

struct MSG_SYN_TELEPORTPORTAL_LIST : public MSGROOT
{
	unsigned char	mCount;
	unsigned long	mMapChangePosIdx[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_TELEPORTPORTAL_LIST) - sizeof(mMapChangePosIdx) + (mCount * sizeof(unsigned long));} 
};

struct MSG_SYN_TELEPORTPORTAL_CREATE : public MSGROOT
{
	unsigned long	mMapChangePosIdx;
};

struct MSG_REQ_TELEPORTPORTAL_USE : public MSGROOT
{
	unsigned long	mMapChangePosIdx;
};

const int ERROR_TELEPORTPORTAL_USE_SUCCESS		= 0;		/// 성공
const int ERROR_TELEPORTPORTAL_USE_RANGE		= 1;		/// 거리가 안맞음
const int ERROR_TELEPORTPORTAL_USE_STATE		= 2;		/// 플레이어 이동할수 없는 상태

const int ERROR_TELEPORTPORTAL_USE_THEME		= 100;		/// 없는 테마
const int ERROR_TELEPORTPORTAL_USE_PORTAL		= 101;		/// 없는 포탈 인덱스
const int ERROR_TELEPORTPORTAL_USE_SCRIPT		= 102;		/// 없는 스크립트 번호
const int ERROR_TELEPORTPORTAL_USE_MAP			= 103;		/// 없는 스크립트 번호
const int ERROR_TELEPORTPORTAL_USE_PVP			= 104;		/// 없는 전장
const int ERROR_TELEPORTPORTAL_USE_UNKNOWN		= 105;		/// 없는 서버타입	

struct MSG_RES_TELEPORTPORTAL_USE : public MSG_ERROR
{
	float			mDestX;
	float			mDestY;
};

struct MSG_SYN_TELEPORTPORTAL_USE : public MSGROOT
{
	unsigned long	mObjectIdx;
	float			mDestX;
	float			mDestY;
};

#pragma pack(pop)
