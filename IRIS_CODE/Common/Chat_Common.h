/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	2008.02.15
 * 주의사항 :	
 * =================================================================== */
#pragma once
#pragma pack(push,1)

#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"
#include "Item_Common.h"

#define MAX_FRIEND          50
#define MAX_CHAT_SESSION    3

/*-- 일반채팅 패킷
*/
struct MSG_REQ_CHAT : public MSGROOT
{
	unsigned long emotionIdx;						// 표정 연출 번호
	short		  InventoryNumber;					// 인벤 링크 넘버
	unsigned long itemIndex;						// 링크 아이템 인덱스
	wchar_t	Message[MAX_CHAT_BUFFER_SIZE];			// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

#define ERROR_CHAT_SUCCESS			0				// 성공
#define ERROR_CHAT_BLOCK			1				// 채팅 금지 적용중
#define ERROR_CHAT_FAIL				2				// 실패

struct MSG_RES_CHAT : public MSG_ERROR
{
	time_t validTrue;							// 유효만기
};

struct MSG_SYN_CHAT : public MSGROOT
{
	unsigned long   CharacterIdx;					// 보낸 캐릭터 고유번호
	unsigned long   emotionIdx;						// 표정 연출 번호
    unsigned short  Job;                            // 직업

	sInventory	  inventory;						// 아이템 정보
	wchar_t       Name[MAX_NAME_BUFFER_SIZE];		// 보낸 캐릭터 이름
	wchar_t       Message[MAX_CHAT_BUFFER_SIZE];	// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};


/*-- 외치기 패킷
*/
struct MSG_REQ_CHAT_SHOUT : public MSGROOT
{
	wchar_t	Message[MAX_CHAT_BUFFER_SIZE];			// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_SHOUT) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};
struct MSG_SYN_CHAT_SHOUT : public MSGROOT
{
	unsigned long   CharacterIdx;						// 보낸 플레이어 인덱스
    unsigned short  Job;                                // 직업
	wchar_t         Name[MAX_NAME_BUFFER_SIZE];		    // 보낸 플레이어 이름
	wchar_t         Message[MAX_CHAT_BUFFER_SIZE];	    // 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_SHOUT) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

/*-- 거래채팅 패킷
*/
struct MSG_REQ_CHAT_TRADE : public MSGROOT
{
	short			InventoryNumber;
	unsigned long	itemIndex;
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];

	/// 실제 보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_REQ_CHAT_TRADE) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};
struct MSG_SYN_CHAT_TRADE : public MSGROOT
{
	sInventory		inventory;
    unsigned short  Job;
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];

	/// 실제 보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_CHAT_TRADE) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

/*-- 귓속말(귀엣말) 패킷
*/
struct MSG_REQ_CHAT_WHISPER : public MSGROOT
{
	short			InventoryNumber;
	unsigned long   itemIndex;					

	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 받는 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_WHISPER) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

#define ERROR_CHAT_WHISPER_SUCCESS			0	// 성공
#define ERROR_CHAT_WHISPER_CHARACTER_NAME	1	// 존재하지 않는 캐릭터 이름
#define ERROR_CHAT_WHISPER_DISCONNECT		2	// 게임에 접속하고 있지 않음
#define ERROR_CHAT_WHISPER_NO_MESSAGE		3	// 메시지입력 없음
#define ERROR_CHAT_WHISPER_INPUT_TIME		4	// 메시지 입력시간이 짧음
#define ERROR_CHAT_WHISPER_MYSELF			5	// 자기자신에게 보내기
#define ERROR_CHAT_WHISPER_OPTIONREFUSE		6	// 상대방 옵션 거부 상태

struct MSG_RES_CHAT_WHISPER : public MSG_ERROR
{
	sInventory		inventory;
    unsigned short  Job;                                // 받는 플레이어 직업
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 받는 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHAT_WHISPER) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};
struct MSG_SYN_CHAT_WHISPER : public MSGROOT
{
	sInventory		inventory;
    unsigned short  Job;                                // 보낸 플레이어 직업
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 보낸 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_WHISPER) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

/*-- 친구 패킷 - 목록/추가/삭제/갱신
*/
#define ERROR_CHAT_FRIEND_SUCCESS			0					// 성공
#define ERROR_CHAT_FRIEND_CHARACTER_NAME	1					// 존재하지 않는 캐릭터 이름
#define ERROR_CHAT_FRIEND_OVER1				2					// 등록갯수 초과
#define ERROR_CHAT_FRIEND_OVER2				3					// 등록갯수 초과
#define ERROR_CHAT_FRIEND_UNKNOWN			4					// 알수없는 오류

struct MSG_REQ_FRIEND_LIST : public MSGROOT
{};
struct MSG_RES_FRIEND_LIST : public MSGROOT
{
	long RowCount;												// 영향받은 행 갯수
	struct
	{
		long			Idx;									// 고유번호
		unsigned long	CharacterIdx;							// 캐릭터 고유번호
		wchar_t			CharacterName[ MAX_NAME_BUFFER_SIZE ];	// 캐릭터 이름(친구)
		char            Race;									// 종족
		char            Gender;									// 성별
		char			Level;									// 레벨
		unsigned short	Job;									// 직업(ePLAYER_JOB)
		short			MapNum;									// 맵(위치)
		DWORD			ChannelNum;								// 접속상태(0접속안됨/채널번호)
		BYTE            Status;									// 친구상태(0등록완료/1등록신청)
		BYTE            Block;									// 차단상태(0차단안함/1차단함)
	} table[1];
};
struct MSG_REQ_FRIEND_ADD : public MSGROOT
{
	wchar_t	Name[MAX_NAME_BUFFER_SIZE];							// 캐릭터 이름(친구)
};
struct MSG_RES_FRIEND_ADD : public MSG_ERROR
{};
struct MSG_REQ_FRIEND_DEL : public MSGROOT
{
	long Idx;													// 고유번호
};
struct MSG_REQ_FRIEND_REP : public MSGROOT
{
	long Idx;													// 고유번호
};

// 친구 접속 여부
struct MSG_SYN_FRIEND_CONNECT : public MSGROOT
{
	unsigned long	characterIndex;
	char            level;					// 캐릭터 레벨
	unsigned long   job;					// 캐릭터 직업(ePLAYER_JOB)
	unsigned short  mapNum;					// 캐릭터 맵(위치)
	BYTE			connect;
	DWORD			channelNum;
};

/*-- 파티 패킷
*/
struct MSG_REQ_CHAT_PARTY : public MSGROOT
{
	short			InventoryNumber;					// 인벤 링크 넘버
	unsigned long	itemIndex;
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_PARTY) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

struct MSG_SYN_CHAT_PARTY : public MSGROOT
{
	unsigned long	playerIndex;						// 보낸 플레이어 인덱스
    unsigned short  job;                                // 직업
	sInventory		inventory;							// 아이템 정보
    wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 보낸 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지
	
	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_PARTY) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

struct MSG_SYN_CHAT_PARTYUNION : public MSGROOT
{
	unsigned long	playerIndex;
    unsigned short  job;                                // 직업
	sInventory		inventory;							// 아이템 정보
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 보낸 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_PARTYUNION) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};
/*-- 길드 패킷
*/

struct MSG_REQ_CHAT_GUILD : public MSGROOT
{
	short	InventoryNumber;					// 인벤 링크 넘버
	unsigned long itemIndex;
	wchar_t	Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_GUILD) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

struct MSG_SYN_CHAT_GUILD : public MSGROOT
{
	sInventory		inventory;							// 아이템 정보
    unsigned short  job;                                // 직업
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 보낸 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_GUILD) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};


//////////////////////////////////////////////////////////
// GAME SYN
struct MSG_SYN_POSTCHAT_WHISPER : public MSGROOT
{
	sInventory		inventory;							// 인벤정보

	unsigned long	senderIndex;						// 보낸 유저
    unsigned short  senderJob;                          // 보낸 유저 직업
	wchar_t			senderName[MAX_NAME_BUFFER_SIZE];	// 보낸 플레이어 이름
	wchar_t			recvName[MAX_NAME_BUFFER_SIZE];		// 받는 플레이어 이름
	wchar_t			message[MAX_CHAT_BUFFER_SIZE];		// 메세지
	
	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_POSTCHAT_WHISPER) - sizeof(message) + ::wcslen(message)*sizeof(wchar_t); }
};

struct MSG_SYN_POSTCHAT_WHISPERRESULT : public MSGROOT
{
	sInventory		inventory;							// 인벤정보
	int				error;								// 결과

	unsigned long	senderIndex;						// 보낸 유저
    unsigned short  recvJob;                            // 받는 플레이어 직업
	wchar_t			recvName[MAX_NAME_BUFFER_SIZE];		// 받는 플레이어 이름
	wchar_t			message[MAX_CHAT_BUFFER_SIZE];		// 메세지
	
	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_POSTCHAT_WHISPERRESULT) - sizeof(message) + ::wcslen(message)*sizeof(wchar_t); }
};


// NM_CHAT_SYSTEM_SYN 참조
struct MSG_SYN_CHAT_SYSTEM : public MSGROOT
{
	wchar_t    characterName[ MAX_NAME_BUFFER_SIZE ];	// 캐릭터 이름
	sInventory inventory;								// 인벤토리
};

// TODO - 우편시스템 - 패킷 구조체
#define	MAIL_DEFAULT_CHARGE			100				// 기본 수수료
#define MAIL_ITEM_CHARGE			300				// 아이템 종류당 수수료

#define POST_TYPE_NORMAL			0				// 우편
#define POST_TYPE_AGENT_CANCEL		1				// 판매대행 등록취소
#define POST_TYPE_AGENT_SALE		2				// 판매대행 판매완료
#define POST_TYPE_AGENT_PURCHASE	3				// 판매대행 구입완료

struct POST_CONTENT
{
	wchar_t title[ LEN_POST_TITLE ];			// 제목
	wchar_t message[ MAX_POST_MESSAGE ];		// 메시지
};
struct POST_RECEIPT
{
	long       price;							// 금액
	long       salesCharges;					// 수수료
	sInventory inventory;						// 아이템 - 판매한 아이템
};

struct POST_OPEN_RESULT
{
	long             idx;						// 고유번호
	SYSTEMTIME       registerDate;				// 등록일
	long             validThru;					// 유효만기

	BYTE             type;						// 종류(0우편, 1등록취소, 2판매완료, 3구입완료)
	BYTE             status;					// 상태(0읽지않음,1읽음,2삭제,3유효만기)

	wchar_t          fromName[ LEN_POST_NAME ];	// 캐릭터 이름(보낸/구매)

	union
	{
		POST_CONTENT content;					// 우편     내용
		POST_RECEIPT receipt;					// 판매대행 영수증
	};

	// 첨부
	sInventory	     inventory1;				// 아이템 정보
	sInventory	     inventory2;				// 아이템 정보
	sInventory	     inventory3;				// 아이템 정보
	long             money;						// 돈
};

// NM_CHAT_POST_OPEN_REQ 참조

#define ERROR_CHAT_POST_OPEN_SUCCESS	0	// 성공
#define ERROR_CHAT_POST_OPEN_FAIL		1	// 실패

struct MSG_REQ_CHAT_POST_OPEN : public MSGROOT
{
	long startPage;
};
struct MSG_RES_CHAT_POST_OPEN : public MSG_ERROR
{
	long             startPage;
	long             endPage;
	long             noneRead;

	long             RowCount;				// 영향받은 행 수
	POST_OPEN_RESULT Result[1];				// 결과

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHAT_POST_OPEN) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_CHAT_POST_CLOSE_REQ 참조

#define ERROR_CHAT_POST_CLOSE_SUCCESS	0	// 성공
#define ERROR_CHAT_POST_CLOSE_FAIL		1	// 실패

struct MSG_REQ_CHAT_POST_CLOSE : public MSGROOT
{};
struct MSG_RES_CHAT_POST_CLOSE : public MSG_ERROR
{};

// NM_CHAT_POST_READ_REQ 참조

#define ERROR_CHAT_POST_READ_SUCCESS		0	// 성공
#define ERROR_CHAT_POST_READ_FAIL			1	// 실패

struct MSG_REQ_CHAT_POST_READ : public MSGROOT
{
	long idx;								// 고유번호
};
struct MSG_RES_CHAT_POST_READ : public MSG_ERROR
{
	long noneRead;								// 읽지않은 우편
};

// NM_CHAT_POST_WRITE_REQ 참조

#define ERROR_CHAT_POST_WRITE_SUCCESS		0	// 성공
#define ERROR_CHAT_POST_WRITE_FAIL			1	// 실패
#define ERROR_CHAT_POST_WRITE_NOT_EXISTS	2	// 실패 - 존재하지 않는 캐릭터
#define ERROR_CHAT_POST_WRITE_UNKNOW		3	// 실패 - 알수없는 오류(DB)
#define ERROR_CHAT_POST_WRITE_TRADE_MONEY   4	// 실패 - 최대 거래금액 초과
#define ERROR_CHAT_POST_WRITE_EXCHANGE      5   // 실패 - 거래불가 아이템

struct MSG_REQ_CHAT_POST_WRITE : public MSGROOT
{
	wchar_t         name[ LEN_POST_NAME ];			// 보낼 - 캐릭터 이름
	wchar_t         title[ LEN_POST_TITLE ];		// 제목
	BYTE            rowCount;						// 적용 - 인벤토리 수
	short           inventoryNumber1;				// 인벤토리 - 번호(슬롯)
	short           inventoryNumber2;				// "
	short           inventoryNumber3;				// "
	unsigned long   money;							// 돈
	wchar_t         message[ LEN_POST_MESSAGE ];	// 메시지
};
struct MSG_RES_CHAT_POST_WRITE : public MSG_ERROR
{};

// NM_CHAT_POST_DOWNLOAD_REQ 참조

#define ERROR_CHAT_POST_DOWNLOAD_SUCCESS	0	// 성공
#define ERROR_CHAT_POST_DOWNLOAD_FAIL		1	// 실패
#define ERROR_CHAT_POST_DOWNLOAD_UNKNOWN	2	// 실패-알수없는 오류

struct MSG_REQ_CHAT_POST_DOWNLOAD : public MSGROOT
{
	long  idx;								// 고유번호
	short inventoryNumber1;					// 인벤토리 번호1(슬롯)
	short inventoryNumber2;					// 인벤토리 번호2(슬롯)
	short inventoryNumber3;					// 인벤토리 번호3(슬롯)
	BYTE  money;							// 돈
};
struct MSG_RES_CHAT_POST_DOWNLOAD : public MSG_ERROR
{
	BYTE       money;						// 받은금액 수령여부(0받지않음 / 1받음)
	BYTE       rowCount;
	sInventory inventory[1];

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHAT_POST_DOWNLOAD) - sizeof(inventory) + (rowCount * sizeof(inventory)); }
};

// NM_CHAT_POST_DELETE_REQ 참조

#define ERROR_CHAT_POST_DELETE_SUCCESS	0	// 성공
#define ERROR_CHAT_POST_DELETE_FAIL		1	// 실패

struct MSG_REQ_CHAT_POST_DELETE : public MSGROOT
{
	long  startPage;

	BYTE  rowCount;							// 적용 - 우편 수
	long  idx[MAX_POST_DELETE];				// 고유번호 - 최대 MAX_POST_DELETE(10) 까지.

	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_POST_DELETE) - sizeof(idx) + (rowCount * sizeof(long)); }
};
struct MSG_RES_CHAT_POST_DELETE : public MSG_ERROR
{
	long             startPage;
	long             endPage;
	long             noneRead;

	BYTE             RowCount;				// 목록 수.
	POST_OPEN_RESULT Result[1];				// 삭제후 목록.

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHAT_POST_DELETE) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_CHAT_POST_RECEIVED_SYN
struct MSG_SYN_CHAT_POST_RECEIVED : public MSGROOT
{};

// 캐시아이템-메가폰
struct MSG_REQ_CHAT_MEGAPHONE : public MSGROOT
{
	unsigned long InventoryIdx;					    // 인벤토리 고유번호(DB)
	unsigned long EmotionIdx;						// 표정 연출 번호
	short		  InventoryNumber;					// 인벤 링크 넘버
	unsigned long ItemIndex;						// 링크 아이템 인덱스
	wchar_t	      Message[MAX_CHAT_BUFFER_SIZE];	// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHAT_MEGAPHONE) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

#define ERROR_CHAT_MEGAPHONE_SUCCESS	0			// 성공
#define ERROR_CHAT_MEGAPHONE_BLOCK		1			// 채팅 금지 적용중
#define ERROR_CHAT_MEGAPHONE_COOLTIME	2			// 쿨타임 적용중
#define ERROR_CHAT_MEGAPHONE_FAIL		3			// 실패

struct MSG_RES_CHAT_MEGAPHONE : public MSG_ERROR
{
	time_t validTrue;							// 유효만기
};

struct MSG_SYN_CHAT_MEGAPHONE : public MSGROOT
{
	unsigned long   ChannenNo;						// 채널번호
	unsigned long   CharacterIdx;					// 보낸 캐릭터 고유번호
	unsigned long   EmotionIdx;						// 표정 연출 번호
    unsigned short  Job;                            // 직업
	sInventory	  Inventory;						// 아이템 정보
	wchar_t       Name[MAX_NAME_BUFFER_SIZE];		// 보낸 캐릭터 이름
	wchar_t       Message[MAX_CHAT_BUFFER_SIZE];	// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_CHAT_MEGAPHONE) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

// 오토더미 공격시
struct MSG_SYN_CHAT_AUTODUMMY : public MSGROOT
{
};

#pragma pack(pop)
