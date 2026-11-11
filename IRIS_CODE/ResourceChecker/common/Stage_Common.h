/* ==========================================================================
* 파    일 : Stage_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-10-22
* 주의사항 :
*===========================================================================*/
#pragma once

#include "CommonNetworkMsgStruct.h"

#pragma pack(push,1)


const unsigned int MAPNUMBER_UNIT = 1000;
const unsigned long CHARACTER_CREATE_POS_IDX = 200101;
const unsigned long TUTORIAL_POS_IDX = 501001;
const unsigned char DEFAULT_MAPMODE = 0;

struct sStageChangePos
{
//	/// 좌표 인덱스
	unsigned long		mPosIdx;

	cString				mFileName;

	/// 
	char				mPotalName[256];

	/// 맵번호
	unsigned short		mMapNumber;
	/// 좌표X
	float				mPosX;
	/// 좌표Y
	float				mPosY;
	/// 범위
	float				mRange;

	/// 목적지맵idx
	unsigned long		mTargetPosIdx;

	/// 회전 각도
	float				mRotAngle;

	// 포탈명 출력 위치
	float				mNameHeight;

	/// 시작시 자동 생성
	bool				mFirstRegen;

	sStageChangePos*	mTargetInfo;

	sStageChangePos()
	{
		mTargetInfo = 0;
	}
};

struct sTargetPos
{
	/// 맵번호
	unsigned short		mMapNumber;
	/// 좌표X
	float				mPosX;
	/// 좌표Y
	float				mPosY;
	/// 회전 각도
	float				mRotAngle;
};


//////////////////////////////////////////////////////////////////////////
/// door

struct sDoorInfo
{
	unsigned long mIndex;

	unsigned short mMapNumber;
	cString		   mFileName;

	float mPosX;
	float mPosY;
	float mAppenZ;
	float mRotAngle;

	NiPoint2 mRectPos[4];
};


#pragma pack(pop)
