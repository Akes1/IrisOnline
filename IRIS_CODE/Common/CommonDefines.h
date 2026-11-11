/* ====================================================================
 * 파    일 :	CommonDefines.h
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once
#define BUILD_NUM						18061

#define VERSION_HEADER					(WORD)(1 | (18 << 8)) //(WORD)(0x005C << 8) | 0
#define MAJOR_VERSION(V)                ( V & 0x00ff )
#define MINOR_VERSION(V)                ( V >> 8 )

#define	MAX_IPADDRESS_SIZE				15
#define	MAX_IPADDRESS_BUFFER_SIZE		(MAX_IPADDRESS_SIZE+1)

#define MAX_UID_SIZE					16
#define MAX_UID_BUFFER_SIZE				MAX_UID_SIZE+1

#define MAX_PWD_SIZE					16
#define MAX_PWD_BUFFER_SIZE			    MAX_PWD_SIZE+1

#define MAX_SEC_SIZE                    4
#define MAX_SEC_BUFFER_SIZE             MAX_SEC_SIZE+1

#define MAX_NAME_SIZE					16
#define MAX_NAME_BUFFER_SIZE			(MAX_NAME_SIZE+1)
#define MAX_CHAT_SIZE					127
#define MAX_CHAT_BUFFER_SIZE			(MAX_CHAT_SIZE+1)
#define MAX_GUILDNAME_SIZE				12
#define MAX_GUILDNAME_BUFFER_SIZE		(MAX_GUILDNAME_SIZE+1)
#define MAX_GUILD_NOTICE_SIZE			40
#define MAX_GUILD_NOTICE_BUFFER_SIZE	(MAX_GUILD_NOTICE_SIZE+1)
#define MAX_TODAYWORD_SIZE				12
#define MAX_TODAYWORD_BUFFER_SIZE		(MAX_TODAYWORD_SIZE+1)

#define MAX_LOADSTRING					1024


// 우편 시스템 관련 정의.
#define MAX_POST_NAME		16
#define LEN_POST_NAME		MAX_POST_NAME+1

#define MAX_POST_TITLE		30
#define LEN_POST_TITLE		MAX_POST_TITLE+1

#define ANY_POST_MESSAGE	1
#define MIN_POST_MESSAGE	0
#define MAX_POST_MESSAGE	150
#define LEN_POST_MESSAGE	MAX_POST_MESSAGE+1

#define MAX_POST_DELETE		10


// 트위터 관련 정의
#define MAX_TWITTER_UID		64
#define LEN_TWITTER_UID		MAX_TWITTER_UID+1

#define MAX_TWITTER_PWD		64
#define LEN_TWITTER_PWD		MAX_TWITTER_PWD+1

// 자동사냥 신고 관련 정의
//#define MAX_CAPTCHA_SIZE            82 * 26 * (24 / 8)    /// 캡차 이미지 사이즈 (24bit)
#define MAX_CAPTCHA_SIZE                8000                /// 캡차 이미지 사이즈 (24bit)
#define MAX_TRIAL_ANSWER_LEN            4                   /// 정답 입력 길이
