/* ====================================================================
 * 파    일 :	CommonNetworkMsgStruct.h
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 *		2006.12.04	이웅주	MSG_ITEM ... 추가
 *		2007.06.21	이법석	규칙 수정.
 * 
 * 주의사항 :
 * =================================================================== */
#pragma once

#include "CommonDefines.h"

#pragma pack(push,1)

struct MSGROOT
{
	char	Category;
	char	Protocol;
};

// 에러 코드
struct MSG_ERROR : public MSGROOT
{
	// 0 성공 / 나머지 error코드
	int	ErrorCode;
};

// NM_USER_WEB_LAUNCHER_REQ
struct MSG_REQ_WEB_LAUNCHER : MSGROOT
{
	long LoginIdx;
};

#define ERROR_WEB_LAUNCHER_SUCCESS		0	// 성공
#define ERROR_WEB_LAUNCHER_FAIL			1	// 실패-인증코드 오류

struct MSG_RES_WEB_LAUNCHER : MSG_ERROR
{};

// NM_USER_SECURITY_NUM_REQ
struct MSG_REQ_SECURITY_NUM : MSGROOT
{
	char sec[MAX_SEC_BUFFER_SIZE];			// 2차보안
};

// NM_USER_LOGIN_REQ
struct MSG_IDPASS : public MSGROOT
{
	char uid[MAX_UID_BUFFER_SIZE];			// 아이디
	char pwd[MAX_PWD_BUFFER_SIZE];			// 패스워드
};

#define ERROR_DIST_USERINFO_SUCCESS		0	// 성공
#define ERROR_DIST_USERINFO_ID_PWD		1	// 실패-ID/PWD
#define ERROR_DIST_USERINFO_SLEVEL		2	// 실패-보안등급 적용중
#define ERROR_DIST_USERINFO_BLOCK		3	// 실패-BLOCK 됨
#define ERROR_DIST_USERINFO_EXISTS		4	// 실패-접속중
#define ERROR_DIST_USERINFO_IS_PIN		5	// 실패-보안번호 미등록
#define ERROR_DIST_USERINFO_PIN			6	// 실패-보안번호 오류
#define ERROR_DIST_USERINFO_AGE			7	// 실패-연령제한
#define ERROR_DIST_USERINFO_FAIL		8	// 실패
#define ERROR_DIST_USERINFO_LOGIN_LIMIT	9	// 실패-로그인 제한

struct MSG_DIST_USERINFO : public MSG_ERROR
{	
	unsigned long	connectionIdx;					// connectionidx	
	unsigned long	UserIdx;						// 계  정 인덱스	
	unsigned long   LoginIdx;						// 로그인 인덱스
	char            twitterUid[LEN_TWITTER_UID];	// 트위터 계정
	char            twitterPwd[LEN_TWITTER_PWD];	// 트위터 암호
};

struct MSG_REQ_SERVERLIST : public MSGROOT
{};
struct MSG_RES_SERVERLIST : public MSGROOT
{
	unsigned long ServerNo;		/// 마지막 사용한 서버번호.
	unsigned long RowCount;
	struct
	{
		long    NumOfServer;
		wchar_t Name[51];
		BYTE    Status;
		BYTE    Age;
	} Servers[1];
};

/// 070404 추가 - 곽철중 [채널리스트]
struct MSG_REQ_CHANNELLIST : public MSGROOT
{
	long NumOfServer;
};
struct MSG_RES_CHANNELLIST : public MSGROOT
{
	unsigned long ChannelNo;	/// 마지막 사용한 채널번호 (현재 사용중도 포함)
	unsigned long RowCount;		/// 채널 리스트 수
	struct
	{
		long NumOfChannel;		/// 채널 번호
		BYTE Status;			/// 채널 상태
	} Channels[1];
};

/// 070405 추가 - 곽철중
#define ERROR_COMEIN_GAME_SUCCESS	0 // 성공
#define ERROR_COMEIN_GAME_FAIL		1 // 미등록 서버
#define ERROR_COMEIN_GAME_DB		2 // DB     오류
#define ERROR_COMEIN_GAME_SYSTEM	3 // SYSTEM 오류
#define ERROR_COMEIN_GAME_AGE       4 // 나이제한

struct MSG_REQ_COMEIN_GAME : public MSGROOT
{
	long          NumOfServer;
	long          NumOfChannel;
	unsigned long CharacterIdx;
	BYTE          Tutorial;
};
struct MSG_RES_COMEIN_GAME : public MSG_ERROR
{};

struct MSG_REQ_GAMESRV : public MSGROOT
{
	long LoginIdx;
	long MemberIdx;
};
struct MSG_RES_GAMESRV : public MSG_ERROR
{};


// NM_USER_TWITTER_REQ
#define ERROR_TWITTER_SUCCESS	0	// 성공
#define ERROR_TWITTER_FAIL		1	// 실패

struct MSG_REQ_TWITTER : public MSGROOT
{
	char uid[LEN_TWITTER_UID];	// 트위터 계정
	char pwd[LEN_TWITTER_PWD];	// 트위터 암호
};
struct MSG_RES_TWITTER : public MSG_ERROR
{};

/******* SERVERTOOL 관련
*/
struct MSG_RES_SRVTOOL_SRVLIST : public MSGROOT
{
    unsigned char rowCount;
    struct
    {
        char    serviceName[MAX_PATH];
        char    ip[50];
        BYTE    state;
    } Servers[1];

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_SRVTOOL_SRVLIST) - sizeof(Servers) + (rowCount * sizeof(Servers)); }
};

struct MSG_REQ_SRVTOOL_AGENT_ONOFF : public MSGROOT
{
    char serviceName[MAX_PATH];
};

struct MSG_RES_SRVTOOL_CHANNEL_INFO : public MSGROOT
{
	unsigned long	mOrderNum;
	wchar_t         mServerName[51];
	DWORD           mServerNum;
	DWORD           mChannelNum;
};

struct MSG_REQ_SRVTOOL_PING : public MSGROOT
{
	unsigned long mToolSendTime;
};

#define SRVTOOL_SECURITY_NONE	(-1)	// SKIP
#define SRVTOOL_SECURITY_LEVEL0	0		// GM
#define SRVTOOL_SECURITY_LEVEL1	1		// TESTER
#define SRVTOOL_SECURITY_LEVEL2	2		// PATH FINDER
#define SRVTOOL_SECURITY_LEVEL3 3		// USER

struct MSG_RES_SRVTOOL_PING : public MSGROOT
{
	unsigned long   mToolSendTime;
	unsigned long   mUserCnt;
	unsigned long   mSecurityLevel;
	int             mHackShield;
};

struct MSG_REQ_SRVTOOL_STATUS : public MSGROOT
{
    char serviceName[MAX_PATH];
};

struct MSG_RES_SRVTOOL_STATUS : public MSGROOT
{
    char    serviceName[MAX_PATH];
    DWORD   status;
};

#pragma pack(pop)
