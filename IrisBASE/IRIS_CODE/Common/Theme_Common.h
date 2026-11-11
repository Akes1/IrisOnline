/* ==========================================================================
* 파    일 : Theme_Common.h
* 목    적 : 
* 작 성 자 : 
* 작 성 일 : 2009-06-03
* 주의사항 :
*===========================================================================*/
#pragma once 

#include "CommonNetworkMsgStruct.h" 
#include "sqltypes.h"
#pragma pack(push,1) 


const unsigned long CIRCUS_START_POS_IDX = 220001;		/// 서커스 테마 서버 시작 포인트
const unsigned long RIOHA_START_POS_IDX = 220101;		/// 라이오하 테마 서버 시작 포인트
const unsigned long DECOMUSS_START_POS_IDX = 220201;	/// 데코무스 테마 서버 시작 포인트

enum eTHEME_MODE
{
	eTHEME_MODE_NONE,  /// 일반 맵
	eTHEME_MODE_NORMAL,
	eTHEME_MODE_HARD,

	eTHEME_MODE_MAX,
};

//////////////////////////////////////////////////////////////////////////
struct sThemePartyInfo
{
	unsigned long	mPlayerIdx;			/// 플레이어 정보
	unsigned char	mThemeMode;			/// 테마 모드
	bool			mIsJoin;			/// 진입 상태 ( 진입 : true , 미진입 : false )
	unsigned long	mThemeUniqueIdx;	/// 테마 고유번호
};

struct MSG_SYN_THEME_READYQUESTION : public MSGROOT
{
	unsigned short	mThemeMapNum;

	unsigned char	mCount;
	sThemePartyInfo	mPartyInfo[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_THEME_READYQUESTION) - sizeof(mPartyInfo) + (mCount * sizeof(sThemePartyInfo)); } 
};

struct MSG_REQ_THEME_READY : public MSGROOT
{
	unsigned short	mThemeMapNum;		/// 테마 번호
	unsigned char	mThemeMode;			/// 테마 모드
};

const unsigned char ERROR_THEME_READY_SUCCESS				= 0;	// 성공
const unsigned char ERROR_THEME_READY_PARTY					= 1;	// 파티가 아님
const unsigned char ERROR_THEME_READY_ITEM					= 2;	// 필요 아이템 없음
const unsigned char ERROR_THEME_READY_REJECTION				= 3;	// 수락 거절
const unsigned char ERROR_THEME_READY_DEPENDENCY			= 4;	// 이미 종속된 방이 있음
const unsigned char ERROR_THEME_READY_SCRIPT				= 5;	// 스크립트 데이터오류
const unsigned char ERROR_THEME_READY_PLAYER_LEVEL			= 6;	// 레벨조건을 만족하지 못함
const unsigned char ERROR_THEME_READY_PLAYER_FULL			= 7;	// 인원초과
const unsigned char ERROR_THEME_READY_SERVER_NOTRUN			= 8;	// 테마서버가 실행안됨
const unsigned char ERROR_THEME_READY_SERVER_READY			= 9;	// 테마서버가 준비중임
const unsigned char ERROR_THEME_READY_REQUESTDEPENDENCY		= 10;	// 요청자 귀속안됨
const unsigned char ERROR_THEME_READY_EXIST					= 11;	// 테마 던전에 입장 정보가 남아 있어 입장할 수 없습니다.
const unsigned char ERROR_THEME_READY_NOT_EXIST				= 12;	// 캐릭터 없음
const unsigned char ERROR_THEME_READY_STATE					= 13;	// 캐릭터 수락 불가 상태
const unsigned char ERROR_THEME_READY_NOT_JOIN				= 14;	// 아직 테마 인던에 들어와 있지 않음.
const unsigned char ERROR_THEME_READY_ALREADY_JOIN			= 15;	// 이미인던에 접속중 일정시간 대기(3분)
const unsigned char ERROR_THEME_READY_FULL_MAKEROOM			= 16;	// 방생성 갯수 초과

struct MSG_RES_THEME_READY : MSG_ERROR
{
};

struct sPlayerThemeList
{
	unsigned short		mMapNumber;		/// 테마맵번호(종류)
	unsigned char		mThemeMode;
	unsigned long		mUniqueIdx;		/// 고유번호
	unsigned long		mResetLeftTime;	/// 초기화 잔여시간

	sPlayerThemeList()
	{
		mMapNumber = 0;
		mUniqueIdx = 0;
		mResetLeftTime = 0;
	}
};

struct MSG_SYN_THEME_LIST : public MSGROOT
{
	unsigned long		mCount;			/// 테마 카운트
	sPlayerThemeList	mThemeList[1];	/// 테마 정보

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_THEME_LIST) - sizeof(mThemeList) + (mCount * sizeof(sPlayerThemeList));} 
};

struct MSG_SYN_THEME_EXPIRE_TIME : public MSGROOT
{
	unsigned long		mExpireTime;    /// 테마 초기화 잔여 시간
};

struct MSG_SYN_THEME_EXPIRE_TIME_NOTICE : public MSGROOT
{};

#pragma pack(pop) 

