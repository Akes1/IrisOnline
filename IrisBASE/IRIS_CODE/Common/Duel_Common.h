/* ==========================================================================
* 파    일 : Duel_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-04-24
* 주의사항 :
*===========================================================================*/
#pragma once 

#include "CommonNetworkMsgStruct.h" 

#pragma pack(push,1) 


const unsigned long	DUEL_ACCEPT_READY_TIME = 10000;		/// 결투 요청 수락 대기 시간
const float			DUEL_ACCEPT_READY_RANGE = 1000;		/// 결투 요청 거리
const unsigned long	DUEL_FIGHT_COUNT_TIME = 5000;		/// 결투 레디에서 시작까지 시간
const float			DUEL_FIGHT_RANGE = 3000;			/// 결투시 결투 범위
const unsigned long	DUEL_FIGHT_RANGE_TIME = 10000;		/// 결투시 범위 나간경우 허용 시간
const unsigned long	DUEL_FIGHT_DELAY_TIME = 200;		/// 결투시 거리체크 부하 방지 딜레이 시간
const unsigned long DUEL_WIN_ADD_INFLUENCEIDX = 100101;		/// 결투 승리자에 추가해줄 효과인덱스
const unsigned long DUEL_LOSE_ADD_INFLUENCEIDX = 100102;	/// 결투 패배자에 추가해줄 효과인덱스


const int ERR_DUEL_ADD_SUCCESS = 0;
const int ERR_DUEL_ADD_FAIL = 1;
const int ERR_DUEL_ADD_OPTIONREFUSE = 2;		/// 옵션으로 인한 거절
struct MSG_REQ_DUEL_ADD : public MSGROOT
{
	unsigned long		mTargetIdx;		/// 결투 대상 idx
};

struct MSG_RES_DUEL_ADD : public MSG_ERROR
{
	unsigned long		mAttackerIdx;	/// 결투 신청자 idx
};

struct MSG_REQ_DUEL_ACCEPT : public MSGROOT
{
	bool				mIsAccept;		/// 결투 수락/거절
};

const int ERR_DUEL_ACCEPT_SUCCESS = 0;
const int ERR_DUEL_ACCEPT_RES_ERROR = 1;		/// 대상자 상태 불가
const int ERR_DUEL_ACCEPT_RES_REJECTION = 2;	/// 대상자 거절
const int ERR_DUEL_ACCEPT_REQ_ERROR = 3;		/// 요청자 불가
struct MSG_RES_DUEL_ACCEPT : public MSG_ERROR
{
	unsigned long		mEnemyIdx;		/// 반대편 플레이어idx
	NiPoint2			mCenterPos;		/// 결투 중심 점
};

struct MSG_SYN_DUEL_ACCEPT : public MSGROOT
{
	unsigned long		mAttackerIdx;	/// 결투 신청자 idx
	unsigned long		mTargetIdx;		/// 결투 대상 idx
};

struct MSG_SYN_DUEL_END : public MSGROOT
{
	unsigned long		mWinnerIdx;	/// 결투 신청자 idx
	wchar_t				mWinnerName[MAX_NAME_BUFFER_SIZE];
	unsigned long		mLoserIdx;		/// 결투 대상 idx
	wchar_t				mLoserName[MAX_NAME_BUFFER_SIZE];
	unsigned short		mStraightWin;	/// 연승
};

struct MSG_SYN_DUEL_WINNER : public MSG_SYN_DUEL_END
{
};

struct MSG_SYN_DUEL_LOSER : public MSG_SYN_DUEL_END
{
};

#pragma pack(pop) 
