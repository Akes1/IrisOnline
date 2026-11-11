/* ====================================================================
 * 파    일 :	sqlAccountStmt.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12/
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_ACCOUNT_STATEMENT_H__
#define __SQL_ACCOUNT_STATEMENT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "odbcsql.h"
#include "CommonDefines.h"


struct SHUTDOWN_ACCOUNT
{
	long serverNum;
	long channelNum;
};

struct CHANNEL_CHECK
{
	long    serverNum;
	wchar_t serverName[51];
	char    serverDesc[201];
	long    channelNum;
	char    inetnum[21];
	long    ipv4;
	short   port;
	long    retvalue;
};

struct MEMBER_CHECK
{
	long loginIdx;
	long memberIdx;
	int  serverNum;
	int  channelNum;
	int  lastChannel;
	long characterIdx;
	BYTE instantDungeon;
	BYTE slevel;
	long pc;
//	BYTE cbtUser;
//	BYTE obtUser;
//	BYTE fourGamer;

	long retvalue;
};

struct MEMBER_OBT_EVENT_COMPLETE
{
	long memberIdx;
	BYTE cbtUser;
	BYTE obtUser;
	BYTE fourGamer;

	long retvalue;
};

struct MEMBER_LOGOUT
{
	long loginIdx;
	long memberIdx;
//	BYTE cbtUser;
//	BYTE obtUser;
//	BYTE fourGamer;
	long retvalue;
};

struct MEMBER_OUT
{
	long loginIdx;
	long memberIdx;
	long error;
};

struct LOGIN_UPDATE
{
	long loginIdx;
};

struct GAME_TO_GAME
{
	long  loginIdx;
	long  serverNum;
	long  channelNum;
	long  lastChannel;
	long  characterIdx;
	BYTE  instantDungeon;
	long  ipv4;
	short port;
	long  retvalue;
};

struct MEMBER_TWITTER
{
	long memberIdx;
	char twitterUid[101];
	char twitterPwd[101];
};

/*      자동사냥 신고
*/
struct TRIAL_SELECT
{
	long    memberIdx;
    long    trialTime;
    long    trialRejectTime;
};

// 신고 재사용 시간
struct MEMBER_TRIAL
{
	long    memberIdx;
    long    autoCharacterIdx;
    long    trialTime;
	long    retvalue;
};

// 신고 거부 시간
struct TRIAL_REJECT
{
	long    memberIdx;
    long    rejectTime;
	long    retvalue;
};

struct CAPTCHA_OFFSET
{
    unsigned long offset;
};

// captcha 이미지 SELECT
struct CAPTCHA_SELECT
{
	wchar_t         captchaName[MAX_TRIAL_ANSWER_LEN+1];    // 정답
	BYTE            captcha[MAX_CAPTCHA_SIZE];              // captcha 이미지
    unsigned short  captchaSize;                            // 이미지 사이즈
    unsigned long   captchaOffset;                          // 이미지 offset
    long            tiralMemberIdx;                         // 신고자 memberIdx
    long            autoCharacterIdx;                       // 오토 플레이어 characterIdx
    long            retvalue;
};

// captcha 이미지 새로고침
struct CAPTCHA_RELOAD
{
	BYTE            captcha[MAX_CAPTCHA_SIZE];
	wchar_t         captchaName[MAX_TRIAL_ANSWER_LEN+1];    // 정답
    unsigned short  captchaSize;                            // 이미지 사이즈
    unsigned long   captchaOffset;                          // 이미지 offset
};

// 로그인 제한
struct LOGIN_LIMIT
{
    unsigned char   count;
	long            memberIdx;
	long            retvalue;
};

class cSQLAccountStmt :
	public cSQLStatement
{
public:
// 클래스 생성 및 공용 메소드.
	cSQLAccountStmt(void);

	SQLRETURN Shutdown            ( SHUTDOWN_ACCOUNT* shutdownAccount );
	SQLRETURN ChannelCheck        ( CHANNEL_CHECK* channelCheck );

	SQLRETURN MemberCheck         ( MEMBER_CHECK* memberCheck );
	SQLRETURN MemberLogout        ( MEMBER_LOGOUT* memberLogout );
	SQLRETURN MemberOut           ( MEMBER_OUT* memberOut );

	SQLRETURN LoginUpdate         ( LOGIN_UPDATE* loginUpdate );
	SQLRETURN GameToGame          ( GAME_TO_GAME* gameToGame );

	SQLRETURN MemberTwitter       ( MEMBER_TWITTER* memberTwitter );

	// 자동사냥 신고
	SQLRETURN CaptchaOffset       ( CAPTCHA_OFFSET* captchaOffset   );
	SQLRETURN CaptchaSelect	      ( CAPTCHA_SELECT* captchaSelect   );
	SQLRETURN CaptchaReload	      ( CAPTCHA_RELOAD* captchaReload   );

    SQLRETURN TrialSelect         ( TRIAL_SELECT* trialSelect       );
    SQLRETURN TrialCheck          ( MEMBER_TRIAL* trialCheck        );
	SQLRETURN TrialUpdate         ( MEMBER_TRIAL* trialUpdate       );
	SQLRETURN TrialRejectUpdate   ( TRIAL_REJECT* rejectUpdate      );

    // 로그인 제한
	SQLRETURN LoginLimit          ( LOGIN_LIMIT* loginLimit         );

	// OBT 이벤트 완료
	SQLRETURN ObtEventComplete    ( MEMBER_OBT_EVENT_COMPLETE* obtEvent );
    
public:
// 클래스 파괴 메소드.
	virtual ~cSQLAccountStmt(void);
};

#endif // __SQL_ACCOUNT_STATEMENT_H__
