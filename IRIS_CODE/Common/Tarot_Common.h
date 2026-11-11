/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	2007.09.03
 * 주의사항 :	
 * =================================================================== */
#pragma once
#pragma pack(push,1)

#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"


#define MAX_TAROT_TITLE			51					// 타로 점집 이름
#define MIN_TAROT_CARDS			10					// 선택 타로 카드(최소)
#define MAX_TAROT_CARDS			22					// 타로카드 한벌
#define MIN_SPREAD				1
#define MAX_SPREAD				5
#define MAX_TAROT_GUEST			10
#define	MAX_NPCSPREAD_COUNT		13					/// 한 스프레드의 정보 최대 수


// 타로 재료 아이템(인덱스)
#define TAROT_MATERIAL_ITEM		        868002
#define TAROT_MATERIAL_ITEM_SILVER		868004
#define TAROT_MATERIAL_ITEM_GOLD        868005
#define TAROT_MATERIAL_ITEM_DIA	        868006


// 카드변환 체인지 아이템(인덱스)
#define TAROT_CHANGE_ITEM_SILVER        808026
#define TAROT_CHANGE_ITEM_GOLD          808027
#define TAROT_CHANGE_ITEM_DIA           808028


// 타로카드 등급
enum eTAROT_CLASS
{
    eTAROT_CLASS_NORMAL = 0,
    eTAROT_CLASS_SILVER,
    eTAROT_CLASS_GOLD,
    eTAROT_CLASS_DIA,

    eTAROT_CLASS_MAX = eTAROT_CLASS_DIA
};


/*-- 타로 이용
*/
#define ERROR_TAROT_SEEKER_OPEN_SUCCESS		0		// 성공
#define ERROR_TAROT_SEEKER_OPEN_FAIL		1		// 실패
#define ERROR_TAROT_SEEKER_OPEN_READER		2		// 실패-개설자오류
#define ERROR_TAROT_SEEKER_OPEN_OVER		3		// 실패-게스트초과
#define ERROR_TAROT_SEEKER_OPEN_HAVE_MONEY	4		// 실패-최대 보유금액 초과
#define ERROR_TAROT_SEEKER_OPEN_RESERVED	5		// 실패-닫기 예약 중



struct MSG_REQ_TAROT_SEEKER_OPEN : MSGROOT
{
	unsigned long CharacterIdx;						// 타로 개설자 인덱스
};

struct MSG_RES_TAROT_SEEKER_OPEN : MSG_ERROR
{
	unsigned long TarotCardIndex[ MAX_TAROT_CARDS ];// 아이템 정의 인덱스 - MAX_TAROT_CARD 22
	unsigned long SpreadIndex[ MAX_SPREAD ];		// 아이템 정의 인덱스 - MAX_SPREAD     5
	wchar_t	      Name[17];							// 이름
	wchar_t	      Title[ MAX_TAROT_TITLE ];			// 타이틀
	unsigned long Price;							// 이용금액
};


#define ERROR_TAROT_SEEKER_CLOSE_SUCCESS	0		// 성공
#define ERROR_TAROT_SEEKER_CLOSE_READER		1		// 실패-개설자 종료
#define ERROR_TAROT_SEEKER_CLOSE_FAIL		2		// 실패

struct MSG_REQ_TAROT_SEEKER_CLOSE : MSGROOT
{};
struct MSG_RES_TAROT_SEEKER_CLOSE : MSG_ERROR
{};


#define ERROR_TAROT_SEEKER_JOIN_SUCCESS		0		// 성공
#define ERROR_TAROT_SEEKER_JOIN_FAIL		1		// 실패
#define ERROR_TAROT_SEEKER_JOIN_READER		2		// 실패-개설자오류
#define ERROR_TAROT_SEEKER_JOIN_ITEM		3		// 실패-아이템부족
#define ERROR_TAROT_SEEKER_JOIN_MONEY		4		// 실패-이용금액 부족
#define ERROR_TAROT_SEEKER_JOIN_HAVE_MONEY	5		// 실패-최대 보유금액 초과

struct MSG_REQ_TAROT_SEEKER_JOIN : MSGROOT
{};
struct MSG_RES_TAROT_SEEKER_JOIN : MSG_ERROR
{
	unsigned long TarotCardIndex[ MIN_TAROT_CARDS ];// 아이템 정의 인덱스 - MAX_TAROT_CARD 10
	wchar_t	      strTitle[ MAX_TAROT_TITLE ];		// 개설명
	unsigned long Price;							// 이용금액
};


#define ERROR_TAROT_SEEKER_RESULT_SUCCESS	0		// 성공
#define ERROR_TAROT_SEEKER_RESULT_FAIL		1		// 실패
#define ERROR_TAROT_SEEKER_RESULT_READER	2		// 실패-개설자오류

#define TAROT_SEEKER_RESULT_STRAIGHT		0
#define TAROT_SEEKER_RESULT_INVERSE			1

struct sTarotResult
{
	BYTE							Dir;			// 타로카드 방향
	unsigned long					TarotCardIndex;	// 아이템 정의 인덱스
	unsigned long                   Value;			// 타로카드 값(스프레드 적용후)
};

struct MSG_REQ_TAROT_SEEKER_RESULT : MSGROOT
{
	unsigned long SpreadIndex;						// Select Spread Index
};
struct MSG_RES_TAROT_SEEKER_RESULT : MSG_ERROR
{
	unsigned long SpreadIndex;						// 아이템 정의 인덱스
	sTarotResult  Result[ MIN_TAROT_CARDS ];		// 타로카드 결과 - MAX_TAROT_RESULT 10
	unsigned long ResultValue;                      // 결과 값
};


#define ERROR_TAROT_SEEKER_ACCEPT_SUCCESS	0		// 성공
#define ERROR_TAROT_SEEKER_ACCEPT_FAIL		1		// 실패
#define ERROR_TAROT_SEEKER_ACCEPT_READER	2		// 실패-개설자오류

struct MSG_REQ_TAROT_SEEKER_ACCEPT : MSGROOT
{};
struct MSG_RES_TAROT_SEEKER_ACCEPT : MSG_ERROR
{};


#define ERROR_TAROT_READER_READY_SUCCESS	0		// 성공
#define ERROR_TAROT_READER_READY_FAIL		1		// 실패

struct MSG_REQ_TAROT_READER_READY : MSGROOT
{};

struct MSG_RES_TAROT_READER_READY : MSG_ERROR
{};

struct MSG_SYN_TAROT_READER_READY : MSGROOT
{
	unsigned long mCharacterIdx;					// 캐릭터 고유번호
};

#define ERROR_TAROT_READER_OPEN_SUCCESS		    0		// 성공
#define ERROR_TAROT_READER_OPEN_CARD		    1		// 실패-아이템 - 카드
#define ERROR_TAROT_READER_OPEN_SPREAD		    2		// 실패-아아템 - 스프레드
#define ERROR_TAROT_READER_OPEN_ITEM		    3		// 실패-아이템 - 재료
#define ERROR_TAROT_READER_OPEN_FAIL		    4		// 실패
#define ERROR_TAROT_READER_OPEN_TRADE_MONEY     5       // 실패-최대 거래금액 초과
#define ERROR_TAROT_READER_OPEN_HAVE_MONEY      6       // 실패-최대 보유금액 초과

struct MSG_REQ_TAROT_READER_OPEN : MSGROOT
{
	wchar_t	      strTitle[ MAX_TAROT_TITLE ];		// 개설명
    unsigned long Price;							// 가격
};
struct MSG_RES_TAROT_READER_OPEN : MSG_ERROR
{
	wchar_t	       strTitle[ MAX_TAROT_TITLE ];		// 개설명
    unsigned long  Price;							// 가  격
	unsigned short UserCount;						// 이용자 수
	unsigned short Point;							// 포인트
};
struct MSG_SYN_TAROT_READER_OPEN : MSGROOT
{
	unsigned long mCharacterIdx;					// 캐릭터 고유번호
	wchar_t	      mTitle[ MAX_TAROT_TITLE ];		// 개설명
};


#define ERROR_TAROT_READER_CLOSE_SUCCESS		0	// 성공
#define ERROR_TAROT_READER_CLOSE_FAIL			1	// 실패-사용중
#define ERROR_TAROT_READER_CLOSE_ITEM			2	// 아이템부족
#define ERROR_TAROT_READER_CLOSE_RESERVED_ON	3	// 실패-예약
#define ERROR_TAROT_READER_CLOSE_RESERVED_OFF	4	// 실패-예약해지

struct MSG_REQ_TAROT_READER_CLOSE : MSGROOT
{};
struct MSG_RES_TAROT_READER_CLOSE : MSG_ERROR
{};
struct MSG_SYN_TAROT_READER_CLOSE : MSG_ERROR
{
	unsigned long mCharacterIdx;					// 캐릭터 고유번호
};

struct MSG_RES_TAROT_READER_JOIN : MSGROOT
{
	wchar_t	       seekerName[17];					// 캐릭터 이름
	unsigned short userCount;						// 이용자 수
	unsigned short point;							// 포인트
};
struct MSG_RES_TAROT_READER_LEAVE : MSGROOT
{
	wchar_t	seekerName[17];						// 캐릭터 이름
};

/*--
-- 유저형태의 NPC 타롯 이용 ( 튜토리얼용 )
*/
// 관람
#define ERROR_TAROT_TUTORIAL_SEEKER_OPEN_SUCCESS	0		// 성공
#define ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL		1		// 실패
#define ERROR_TAROT_TUTORIAL_SEEKER_OPEN_NPC		2		// 실패
#define ERROR_TAROT_TUTORIAL_SEEKER_OPEN_OVER		3		// 실패-게스트초과

struct MSG_REQ_TAROT_TUTORIAL_SEEKER_OPEN : MSGROOT
{
	unsigned long npcIndex;						// 타로 npc 인덱스
};

// 닫기
#define ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_SUCCESS	0		// 성공
#define ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_FAIL		1		// 실패

struct MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE : MSGROOT
{};
struct MSG_RES_TAROT_TUTORIAL_SEEKER_CLOSE : MSG_ERROR
{};

// 참여
#define ERROR_TAROT_TUTORIAL_SEEKER_JOIN_SUCCESS	0		// 성공
#define ERROR_TAROT_TUTORIAL_SEEKER_JOIN_FAIL		1		// 실패

struct MSG_REQ_TAROT_TUTORIAL_SEEKER_JOIN : MSGROOT
{};
struct MSG_RES_TAROT_TUTORIAL_SEEKER_JOIN : MSG_ERROR
{
	unsigned long TarotCardIndex[ MIN_TAROT_CARDS ];// 아이템 정의 인덱스 - MAX_TAROT_CARD 10
};

// 결과요청
#define ERROR_TAROT_TUTORIAL_SEEKER_RESULT_SUCCESS	0		// 성공
#define ERROR_TAROT_TUTORIAL_SEEKER_RESULT_FAIL		1		// 실패

struct MSG_REQ_TAROT_TUTORIAL_SEEKER_RESULT : MSGROOT
{
	unsigned long SpreadIndex;						// Select Spread Index
};
struct MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT : MSG_ERROR
{
	unsigned long SpreadIndex;						// 아이템 정의 인덱스
	sTarotResult  Result[ MIN_TAROT_CARDS ];		// 타로카드 결과 - MAX_TAROT_RESULT 10
	unsigned long ResultValue;                      // 결과 값
};

// 결과
#define ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_SUCCESS	0		// 성공
#define ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_FAIL		1		// 실패

struct MSG_REQ_TAROT_TUTORIAL_SEEKER_ACCEPT : MSGROOT
{};
struct MSG_RES_TAROT_TUTORIAL_SEEKER_ACCEPT : MSG_ERROR
{};


/*-- 혼자 타로점 보기
*/

// NM_TAROT_SELF_OPEN_REQ
#define ERROR_TAROT_SELF_OPEN_SUCCESS       0		// 성공
#define ERROR_TAROT_SELF_OPEN_FAIL		    1		// 실패

// NM_TAROT_SELF_JOIN_REQ
#define ERROR_TAROT_SELF_JOIN_SUCCESS       0		// 성공
#define ERROR_TAROT_SELF_JOIN_FAIL		    1		// 실패
#define ERROR_TAROT_SELF_JOIN_METARIAL	    2		// 재료부족
#define ERROR_TAROT_SELF_JOIN_SPREAD	    3		// 스프레드 카드가 없음.
#define ERROR_TAROT_SELF_JOIN_CLASS	        4		// 타로카드 등급

struct MSG_REQ_TAROT_SELF_JOIN : MSGROOT
{
//    unsigned char   tarotClass;
};

struct MSG_RES_TAROT_SELF_JOIN : MSG_ERROR
{
	unsigned long TarotCardIndex[MIN_TAROT_CARDS];    // 아이템 정의 인덱스 - MAX_TAROT_CARD 10
};

// NM_TAROT_SELF_CLOSE_REQ
#define ERROR_TAROT_SELF_CLOSE_SUCCESS      0		// 성공
#define ERROR_TAROT_SELF_CLOSE_FAIL		    2		// 실패

// NM_TAROT_SELF_RESULT_REQ
#define ERROR_TAROT_SELF_RESULT_SUCCESS	    0		// 성공
#define ERROR_TAROT_SELF_RESULT_FAIL		1		// 실패

struct MSG_REQ_TAROT_SELF_RESULT : MSGROOT
{
	unsigned long SpreadIndex;						// Select Spread Index
};

struct MSG_RES_TAROT_SELF_RESULT : MSG_ERROR
{
	unsigned long SpreadIndex;						// 아이템 정의 인덱스
	sTarotResult  Result[MIN_TAROT_CARDS];		    // 타로카드 결과 - MAX_TAROT_RESULT 10
	unsigned long ResultValue;                      // 결과 값
};

// NM_TAROT_SELF_ACCEPT_REQ
#define ERROR_TAROT_SELF_ACCEPT_SUCCESS     0		// 성공
#define ERROR_TAROT_SELF_ACCEPT_FAIL		1		// 실패

#pragma pack(pop)
