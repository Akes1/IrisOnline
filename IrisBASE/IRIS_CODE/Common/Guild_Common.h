/* ==========================================================================
* 파    일 : Guild_Common.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-10-31
* 주의사항 :
*===========================================================================*/
#pragma once
#pragma pack(push,1)

#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"
#include "Item_Common.h"
#include "sqltypes.h"

#define	MAX_GUILD	50				/// 길드의 최대 유저 수
#define	GUILDLIMIT_LEVEL	10		/// 길드를 생성할 수 있는 레벨
#define	GUILDLIMIT_MONEY	10000	/// 길드를 생성할 수 있는 금액
#define MARK_WIDTH	24
#define MARK_HEIGHT	24
#define MARK_BPP	32
#define TGA_EXTENSION			    26		/// TGA 파일 끝 확장 부분

/// 포토샾에서 24x24x32bit 파일을 뽑아내면 원래 크기보다 2바이트가 더 늘어남...
#define MARK_SIZE	(MARK_WIDTH*MARK_HEIGHT*(MARK_BPP/8) + 54 +2 )


/// 길드 직위
enum eGUILD_POSITION
{
	eGUILD_NONE  = 0,
	eGUILD_MASTER,			/// 길드장
	eGUILD_SUBMASTER,		/// 부 길드장
	eGUILD_ELDER,			/// 장로
	eGUILD_UPPERHOUSE,		/// 상원 의원
	eGUILD_LOWERHOUSE,		/// 하원 의원
	eGUILD_GENERAL,			/// 일반
};

// 길드 정보
struct sGuildInfo
{
	unsigned long		idx;									// 길드 인덱스
	wchar_t				name[MAX_GUILDNAME_BUFFER_SIZE];		// 길드명
	unsigned long		masterIndex;							// 길드장 인덱스
	wchar_t				masterName[MAX_NAME_BUFFER_SIZE];		// 길드장명
	wchar_t				notice[MAX_GUILD_NOTICE_BUFFER_SIZE];	// 길드 공지
	char				position;								// 내 직위
	tm					markRegistDate;							// 길드마크 최신 갱신날짜
	bool				useMark;
};

// 길드 유저 정보
struct sGuildUserInfo
{
	unsigned long	characterIdx;				// 캐릭터 인덱스
	wchar_t			name[MAX_NAME_BUFFER_SIZE];	// 캐릭터명
	char			level;						// 레벨
	unsigned short	job;						// 캐릭터 직업
	short			mapnum;						// 맵 번호
	char			position;					// 직위
	DWORD			channelNum;					// 채널 번호
};

// 시간마다 갱신할 유저정보
struct sGuildUserUpdate
{
	long			characterIdx;				// 캐릭터 인덱스
	char			level;						// 레벨
	unsigned short	job;						// 캐릭터 직업
	short			mapnum;						// 맵 번호
	DWORD			channelNum;					// 채널 번호
};

/// 길드 생성 정보
struct MSG_RES_GUILD_CREATEDATA : MSGROOT
{
	long		idx;								// 길드 인덱스
	wchar_t		name[MAX_GUILDNAME_BUFFER_SIZE];	// 길드명
};

/// 길드 유저 정보
struct MSG_SYN_GUILD_LIST : MSGROOT
{
	unsigned char	rowCount;						// 열 수
	sGuildInfo		data;							// 길드 데이터
	sGuildUserInfo	table[1];						// 길드 유저 리스트
};

#define ERROR_GUILD_ADD_SUCCESS			0	// 성공 - 길드 신청 성공
#define ERROR_GUILD_ADD_FAIL			1	// 실패
#define ERROR_GUILD_ADD_REQUEST			2	// 실패 - 이미 신청중임
#define ERROR_GUILD_ADD_NOTEXIST		3	// 실패 - 상대가 존재하지 않음
#define ERROR_GUILD_ADD_HAVEGUILD		4	// 실패 - 상대가 이미 길드 가입중
#define ERROR_GUILD_ADD_MAXGUILD		5	// 실패 - 길드 인원 제한
#define ERROR_GUILD_ADD_REFUSE			6	// 실패 - 초대 거절
#define ERROR_GUILD_ADD_REFUSETIME		7	// 실패 - 초대 거절 - 응답 시간 초과
#define ERROR_GUILD_ADD_STATUSHERO		8	// 실패 - 신청가능 상태 아님
#define ERROR_GUILD_ADD_STATUS			9	// 실패 - 상대방이 신청받기 가능 상태 아님

/// 길드 초대 요청
struct MSG_REQ_GUILD_ADD : MSGROOT 
{
	wchar_t name[MAX_NAME_BUFFER_SIZE];				// 길드 초대할 유저명
};

struct MSG_RES_GUILD_ADD : MSG_ERROR
{};

/// 길드초대 받음
struct MSG_SYN_GUILD_ADD : MSGROOT
{
	wchar_t name[MAX_NAME_BUFFER_SIZE];				// 초대 유저명
	wchar_t guildName[MAX_GUILDNAME_BUFFER_SIZE];	// 초대 길드명
};

#define ERROR_GUILD_ADDREPLY_SUCCESS		0	// 성공 - 가입
#define ERROR_GUILD_ADDREPLY_FAIL			1	// 실패
#define ERROR_GUILD_ADDREPLY_EXIST			2	// 실패
#define ERROR_GUILD_ADDREPLY_FAILUPDATE		3	// 실패
#define ERROR_GUILD_ADDREPLY_REFUSE			4	// 초대 거절
#define ERROR_GUILD_ADDREPLY_REFUSETIME		5	// 초대 거절

#define GUILD_REPLAY_SUCCESS				0	// 수락
#define GUILD_REPLAY_REFUSE					1	// 거절
#define GUILD_REPLAY_REFUSETIME				2	// 거절

/// 길드초대 답변 보내기
struct MSG_REQ_GUILD_REPLY : MSGROOT
{
	char		reply;					
};

struct MSG_RES_GUILD_REPLY : MSG_ERROR 
{};

/// 신규가입 유저 정보 보내기
struct MSG_SYN_GUILD_USERADD : MSGROOT
{
	sGuildUserInfo		user;				/// 추가된 유저정보 보내기
};

#define ERROR_GUILD_OUT_SUCCESS		0		// 성공 - 탈퇴
#define ERROR_GUILD_OUT_FAIL		1		// 실패
#define ERROR_GUILD_OUT_MASTER		2		// 실패 - 길드장 탈퇴 불가

// 길드 탈퇴 요청
struct MSG_REQ_GUILD_OUT : MSGROOT
{};

struct MSG_RES_GUILD_OUT : MSG_ERROR
{};

struct MSG_SYN_GUILD_OUT : MSGROOT
{
	unsigned long	characterIdx;			// 탈퇴한 유저인덱스
};

#define ERROR_GUILD_CUT_SUCCESS		0		// 성공 - 탈퇴
#define ERROR_GUILD_CUT_FAIL		1		// 실패
#define ERROR_GUILD_CUT_MASTER		2		// 실패 - 길드장만 추방 가능

// 길드 추방 요청
struct MSG_REQ_GUILD_CUT : MSGROOT
{
	unsigned long	characterIdx;			// 추방할 유저인덱스
};

struct MSG_RES_GUILD_CUT : MSG_ERROR
{};

struct MSG_SYN_GUILD_CUT : MSGROOT			// 자신이 추방당함
{};

struct MSG_SYN_GUILD_USERCUT : MSGROOT
{
	unsigned long	characterIdx;			// 추방당한 유저인덱스
};

#define ERROR_GUILD_DELETE_SUCCESS		0		// 성공 - 해체
#define ERROR_GUILD_DELETE_FAIL			1		// 실패
#define ERROR_GUILD_DELETE_MASTER		2		// 실패 - 길드장만 해체 가능
#define ERROR_GUILD_DELETE_USEREXIST	3		// 실패 - 길드유저가 남아있음

// 길드 해체 요청
struct MSG_REQ_GUILD_DELETE : MSGROOT
{};

struct MSG_RES_GUILD_DELETE : MSG_ERROR
{};

/// 일정시간 갱신
struct MSG_SYN_GUILD_UPDATE : MSGROOT
{
	unsigned char		rowCount;				// 열 수
	sGuildUserUpdate	table[1];				// 변경된 유저 정보 리스트
};

#define ERROR_GUILD_GIVEPOSITION_SUCCESS	0		// 성공 
#define ERROR_GUILD_GIVEPOSITION_FAIL		1		// 실패
#define ERROR_GUILD_GIVEPOSITION_POSITION	2		// 실패 - 권한 실패

// 길드 직위 변경
struct MSG_REQ_GUILD_GIVEPOSITION : MSGROOT
{
	unsigned long		characterIdx;			// 직위변경할 유저인덱스
	char				position;				// 직위
};

struct MSG_RES_GUILD_GIVEPOSITION : MSG_ERROR
{};

struct MSG_SYN_GUILD_GIVEPOSITION : MSGROOT
{
	unsigned long		characterIdx;			// 직위변경된 유저인덱스
	char				position;				// 직위
};

#define ERROR_GUILD_NOTICE_SUCCESS	0		// 성공 
#define ERROR_GUILD_NOTICE_FAIL		1		// 실패
#define ERROR_GUILD_NOTICE_MASTER	2		// 실패 - 길드장

// 길드 공지 수정
struct MSG_REQ_GUILD_NOTICE : MSGROOT
{
	wchar_t			notice[MAX_GUILD_NOTICE_BUFFER_SIZE];	// 등록할 공지
};

struct MSG_RES_GUILD_NOTICE : MSG_ERROR
{};

struct MSG_SYN_GUILD_NOTICE : MSGROOT
{
	wchar_t			notice[MAX_GUILD_NOTICE_BUFFER_SIZE];	// 새 공지

	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_GUILD_NOTICE) - sizeof(notice) + ::wcslen(notice)*sizeof(wchar_t); }
};

#define ERROR_GUILDMARK_UPDATE_SUCCESS  0		// 성공
#define ERROR_GUILDMARK_UPDATE_FAIL		1		// 실패
#define ERROR_GUILDMARK_UPDATE_POSITION 2		// 권한실패

// 길드 마크 변경
struct MSG_REQ_GUILDMARK_UPDATE : MSGROOT
{
	BYTE		mark[MARK_SIZE];		/// 길드마크 갱신
};

struct MSG_RES_GUILDMARK_UPDATE : MSG_ERROR
{};

#define ERROR_GUILDMARK_SEND_SUCCESS	0		// 성공
#define ERROR_GUILDMARK_SEND_FAIL		1		// 실패

// 길드 마크 send 요청
struct MSG_REQ_GUILDMARK_SEND : MSGROOT
{
	unsigned long	guildIndex;			/// 길드 인덱스
};

struct MSG_RES_GUILDMARK_SEND : MSG_ERROR
{
	unsigned long	guildIndex;				/// 길드 인덱스
	tm				registDate;				/// 등록 날짜
	BYTE			mark[MARK_SIZE];		/// 길드마크
};

// 길드유저 접속 정보
struct MSG_SYN_GUILDUSER_CONNECT : MSGROOT
{
	unsigned long	characterIndex;
	BYTE			connect;
	DWORD			channelNum;
};
#pragma pack(pop)
