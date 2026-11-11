/* ====================================================================
 * 파    일 :	logProtocol.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/26
 * 주의사항 :	
 * =================================================================== */
#ifndef __LOG_PROTOCOL_H__
#define __LOG_PROTOCOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

// 윈도우즈 헤더
#include <windows.h>
#include "msgbuf.h"

enum LOG_PROTOCOL
{
	MB_SERVER_EVENT = 0,
	MB_MEMBER_EVENT,
	MB_CHARACTER_EVENT,
	MB_MONEY_EVENT,
	MB_INVENTORY_EVENT,
	MB_CONCURRENT_EVENT,
	MB_DEPOSIT_EVENT,
	MB_QUEST_EVENT,
	MB_GUILD_EVENT,
	MB_DROPITEM_EVENT,
	MB_SKILL_EVENT,
	MB_ITEM_BILL_EVENT,
	MB_INFCASH_EVENT,
	MB_RESERVED_EVENT,
	MB_POST_EVENT,
};

// 1Byte 정렬 설정
#pragma pack(1)

#ifndef SOURCE_BUF
#define SOURCE_LEN      50
#define SOURCE_BUF      SOURCE_LEN+1
#endif

#ifndef COMPUTER_BUF
#define COMPUTER_LEN    132
#define COMPUTER_BUF    COMPUTER_LEN+1
#endif

#ifndef ANY_SIZE
#define ANY_SIZE     1
#endif

struct SERVER_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	char             source[ SOURCE_BUF ];
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};


#define EVENT_MEMBER_SUCCESS_AUDIT	0
#define EVENT_MEMBER_FAILURE_AUDIT	1
#define EVENT_MEMBER_TIMEOUT        2
#define EVENT_MEMBER_LOGIN			0
#define EVENT_MEMBER_LOGOUT			(~0)

struct MEMBER_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	char             type;
	char             source[ SOURCE_BUF ];
	char             category;
	long             memberIdx;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

#define EVENT_CHARACTER_CREATE			0
#define EVENT_CHARACTER_DELETE			1
#define EVENT_CHARACTER_UPDATE			2
#define EVENT_CHARACTER_LOGIN			3
#define EVENT_CHARACTER_LOGOUT			4
#define EVENT_CHARACTER_DELETE_WAIT		5
#define EVENT_CHARACTER_DELETE_CANCEL	6

struct CHARACTER_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	char             source[ SOURCE_BUF ];
	char             category;
	long             memberIdx;
	long             serverIdx;
	long             characterIdx;
	char             characterLevel;
	long             characterExp;
	char             characterSkillLevel;
	long             characterSkillExp;
	long             characterSkillPoint;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

struct MONEY_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             characterIdx;
	long             before;
	long             after;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

struct DEPOSIT_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             characterIdx;
	long             before;
	long             after;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

#define EVENT_INVENTORY_CREATE		0
#define EVENT_INVENTORY_DELETE		1
#define EVENT_INVENTORY_UPDATE		2

struct INVENTORY_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	char             category;
	long             inventoryIdx;
	long             characterIdx;
	long             itemDefineIndex;
	short            number;
	short            count;
	BYTE             enhanced;
	BYTE             seal;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

#define EVENT_QUEST_ADD				0
#define EVENT_QUEST_UPDATE			1
#define EVENT_QUEST_DELETE			2
#define EVENT_QUEST_COMPLETE		3
#define EVENT_QUEST_REWARD			4
#define EVENT_QUEST_REMOVE			7
#define EVENT_QUEST_TIMESELECT		8

struct QUEST_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	char             category;
	long             characterIdx;
	long             questIdx;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

struct GUILD_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             characterIdx;
	long             guildIdx;
	char			 guildPosition;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

struct sDropItem
{
	long			mMonsterIdx;
	long			mDieCnt;
	long			mMoneyCnt;
	long			mItemIdx;
	long			mItemCnt;
	char			mDropType;
};

struct DROPITEM_EVENT : MSGBUF
{
	long			mServerIdx;
	long			mChannelNum;
	char			mComputer[ COMPUTER_BUF ];
	long			mCount;
	sDropItem		mDropItem[ ANY_SIZE ];

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(DROPITEM_EVENT) - sizeof(mDropItem) + (mCount * sizeof(sDropItem)); }
};

struct SKILL_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT	mRegisterDate;
	long				mServerIdx;
	long				mCharacterIdx;
	bool				mIsReset;
	long				mSkillIdx;
	char				mSkillStep;
	char				mComputer[ COMPUTER_BUF ];
};

struct CONCURRENT_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             minimum;
	long             maximum;
	char             computer[ COMPUTER_BUF ];
	char             message[ ANY_SIZE ];
};

enum
{
	EVENT_RESERVED_TUTORIAL = 0,
	EVENT_RESERVED_CHANGEJOB,		// 전직
	EVENT_RESERVED_PVP_01 = 100,	// Death Match
	EVENT_RESERVED_PVP_02,			// Flag Match
	EVENT_RESERVED_THEME_01 = 200,	// Circus
	EVENT_RESERVED_THEME_02 = 201,	// Rioha
	EVENT_RESERVED_THEME_03 = 202,	// Decomuss
};

struct RESERVED_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	short            type;
	long             characterIdx;
	long             memberIdx;
	char             computer[ COMPUTER_BUF ];
};

struct ITEM_BILL_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             characterIdx;
	long             inventoryIdx;
	long             before;
	long             after;
	char             computer[ COMPUTER_BUF ];
};

/// 효과
struct sLogInf
{
	unsigned long		mUniqueIdx;
	unsigned long		mDbIdx;
	unsigned long		mInfluenceClassIdx;
	unsigned long		mAttackerIdx;
	unsigned long		mAttackerType;
	bool				mIsRealTime;
	unsigned long		mRestTime;
	TIMESTAMP_STRUCT	mEndDate;
	unsigned long		mTime1;
	unsigned long		mTime2;
	long				mInfluenceValue1;
	long				mInfluenceValue2;
	unsigned char		mAttriButeType;
	unsigned char		mAttackerLv;
	unsigned long		mAttackerDamageMin;
	unsigned long		mAttackerDamageMax;
	unsigned long		mHeal;
	unsigned char		mLeftGuardCnt;
	unsigned short		mLeftGuardValue;
	unsigned char		mDel;
};

#define EVENT_INFCASH_CYCLE			1		/// 주기적으로 저장
#define EVENT_INFCASH_INSERT		2		/// 등록
#define EVENT_INFCASH_DELETE		3		/// 삭제
#define EVENT_INFCASH_LOGOUT		4		/// 로그아웃

struct MB_SYN_INFCASH_SAVE : MSGBUF
{
	long			mServerNum;							// Connection Index (Server) 
	long			mCharacterIdx;
	char			mType;
	char			mComputer[ COMPUTER_BUF ];
	long			mCount;
	sLogInf			mInfCash[ ANY_SIZE ];
};


struct POST_EVENT : MSGBUF
{
	TIMESTAMP_STRUCT registerDate;
	long             serverIdx;
	long             postIdx;
	long             characterIdx;
	long             inventoryIdx1;
	long             inventoryIdx2;
	long             inventoryIdx3;
	long             money;
	char             computer[ COMPUTER_BUF ];
};


// 1Byte 정렬 해제
#pragma pack( )

#endif // __LOG_PROTOCOL_H__
