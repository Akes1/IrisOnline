/* ==========================================================================
* 파    일 : Quest_Common.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-03-03
* 주의사항 :
*===========================================================================*/
#pragma once
#pragma pack(push,1)

#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"

/// 최대 보유 가능 갯수
#define MAX_KEEPQUEST	20
#define MAX_DUTY		8		/// 한 퀘스트에서 가지는 최대 임무수 (지불금액은 제외)

#define QUEST_BUFF_MAX	3
#define QUEST_ITEM_MAX	5

#define QUEST_INITHOUR 24		/// 퀘스트 초기화 절대 시간

/// 퀘스트 타입
enum eQuestEventType
{
	eQUEST_TIMEEVENT = 0,	/// 시간제
	eQUEST_TRANSMON,	/// 변신상태
	eQUEST_SETTITLE,	/// 호칭상태

	eQUEST_MAX			/// 일반
};

/// 퀘스트 반복타입
enum eQuestRepeat
{
	eQUEST_CHALLENGE = 0,	/// 성공시까지
	eQUEST_REPEAT,		/// 반복
};

/// 임무 수행타입
enum eDutyType
{
	eDUTY_NONE = 0,
	eDUTY_HUNT,			/// 몬스터 사냥
	eDUTY_COLLECT,		/// 아이템 수집
	eDUTY_MEET,			/// 만나기
	eDUTY_ACTIVESKILL,	/// 스킬 발동 (몬스터에게)
	eDUTY_ENHANCE,		/// 강화
	eDUTY_PUTCARD,		/// 카드 장착
	eDUTY_PVPJOIN,		/// 전장참여
	eDUTY_PVPWIN,		/// 전장승리
	eDUTY_PVPLOSE,		/// 전장패배
};

/// DB의상태와 맞춰져있다
/// 현재 퀘스트의 상태
enum eQuestStatus
{
	// 퀘스트 자체 상태
	eQUEST_PLAYING	= 0,	/// 진행중
	//eQUEST_REWARD	= 1,	/// 보상완료
	eQUEST_COMPLETE	= 2,	/// 진행완료
	//eQUEST_DELETE	= 3,	/// 포기
	//eQUEST_REMOVE	= 5,	/// 강제삭제
};

// npc가 이용하는 퀘스트 알림상태
// 우선순위별로 정렬 ( 보상가능 > 완료가능 > 신규가능 > 진행중 > 실패 > NONE )
enum eNpcQuestStatus
{
	eNPCQUEST_NONE = 0,
	eNPCQUEST_PLAYING,			/// 진행중
	eNPCQUEST_NEW,				/// 신규가능
	eNPCQUEST_COMPLETE,			/// 진행완료가능 ( 진행중의 다른 상태 )
	eNPCQUEST_REWARD,			/// 보상가능
 	eNPCQUEST_MAX,
};

/// npc의 퀘스트 이펙트 종류
enum eNpcQuestEffect
{
	eNPCQUEST_EFFECT_NONE = 0,

	eNPCQUEST_EFFECT_PLAYING,							/// 퀘스트 진행중

	eNPCQUEST_EFFECT_NEW,								/// 일반 퀘스트 신규가능
	eNPCQUEST_EFFECT_COMPLETE,							/// 일반 퀘스트 진행완료가능 ( 진행중의 다른 상태 )
	eNPCQUEST_EFFECT_REWARD,							/// 일반 퀘스트 보상가능

	eNPCQUEST_EFFECT_THEME_NEW,							/// 테마 던젼 퀘스트 신규가능     상태
	eNPCQUEST_EFFECT_THEME_COMPLETE,					/// 테마 던젼 퀘스트 진행완료가능 상태
	eNPCQUEST_EFFECT_THEME_REWARD,						/// 테마 던젼 퀘스트 보상가능     상태

	eNPCQUEST_EFFECT_REPEAT_NEW,						/// 반복 퀘스트 신규가능     상태
	eNPCQUEST_EFFECT_REPEAT_COMPLETE,					/// 반복 퀘스트 진행완료가능 상태
	eNPCQUEST_EFFECT_REPEAT_REWARD,						/// 반복 퀘스트 보상가능     상태

	/// eNPCQUEST_NEW 일때 클라이언트의 특수 상태
	eNPCQUEST_EFFECT_LOW_LEVEL_NEW,						/// 저 레벨 신규가능( 퀘스트 수주 가능, 이펙트 표시 안함 )
	eNPCQUEST_EFFECT_NEXT_LEVEL_NEW,					/// 고 레벨 신규 퀘스트가 있음( 퀘스트 수주 불가, 단순 표시용 )
};

/// 퀘스트 제한시간 타입
enum eQuestTimeType
{
	eTIME_NONE = 0,
	eTIME_REAL,		/// 실시간타입
	eTIME_GAME,		/// 게임시간타입
};	

/// 제공 & 반납 타입
enum eGiveType
{	
	eGIVE_GIVE = 0,		/// 아이템 제공용
	eGIVE_TAKE,		/// 아이템 반납용
};

/// 아이템 보상 타입
enum eRewardType
{	
	eREWARD_DEFAULT = 0,	/// 기본보상
	eREWARD_SELECT,		/// 선택보상
};

enum eDeleteType
{
	eDELETE_YES = 0,		/// 삭제 가능
	eDELETE_NO,			/// 삭제 불가
};	

/// 요일 타입
enum eDayType
{
	eDAY_SUN = 0,		/// tm의 wday와 맞춘다
	eDAY_MON,
	eDAY_TUE,
	eDAY_WED,
	eDAY_THU,
	eDAY_FRI,
	eDAY_SAT,

	eDAY_MAX,
};

/// 퀘스트 진행정보
struct sQuestList
{
	unsigned long	arrIdx;				// array 인덱스
	unsigned long   questIdx;			// [In]  퀘스트 고유번호
	__int64 check;						// [In]  진행정보
	char	status;						// [Out] 퀘스트상태 (0:진행중, 2:진행완료, 4:진행실패)
	long	restTime;					// [Out] 남은시간
};

enum eREWARD_TYPE
{
	eREWARD_ITEM = 0,				/// 아이템
	eREWARD_SKILL,					/// 스킬
};

struct sRewardData
{
	eREWARD_TYPE	type;				/// 보상 타입
	unsigned long	index;				/// 인덱스
};

struct sQuestItem
{
	unsigned long  itemIndex;
	unsigned long  count;
};

/// NPC가 가진 퀘스트 리스트
struct sNpcQuestList
{
	tHashSet<unsigned long> mQuestSet;
};

///
#define ERROR_QUEST_ADD_SUCCESS			0	// 성공 - 퀘스트 추가 성공
#define ERROR_QUEST_ADD_FAIL			1	// 실패 - 알수 없는 오류
#define ERROR_QUEST_ADD_NOTYET			2	// 실패 - DB 사용중
#define ERROR_QUEST_ADD_EXIST			3	// 실패 - 이미 해당 퀘스트를 가지고있음
#define ERROR_QUEST_ADD_COMPLETE		4	// 실패 - 이미 완료한 퀘스트
#define ERROR_QUEST_ADD_TRIGGER			5	// 실패 - 퀘스트 받을 조건이 안됨
#define ERROR_QUEST_ADD_MAX				6	// 실패 - 퀘스트 목록이 FULL 
#define ERROR_QUEST_ADD_ITEM			7	// 실패 - 퀘스트 제공아이템 받을수 없음
#define ERROR_QUEST_ADD_NPC				8   // 실패 - NPC 인증 실패
#define ERROR_QUEST_ADD_TIME			9   // 실패 - 다시받을수 있는 시간이 안됬음
#define ERROR_QUEST_ADD_MONEY			10  // 실패 - 지불할 금액이 부족함
#define ERROR_QUEST_ADD_POINT			11  // 실패 - 지불할 포인트가 부족함
#define ERROR_QUEST_ADD_ITEMFAIL		12	// 실패 - 아이템
#define ERROR_QUEST_ADD_COOLTIME		13	// 실패 - 아이템 쿨타임
#define ERROR_QUEST_ADD_TUTORIALMODE	14	// 실패 - 튜토리얼 모드 변경 실패
#define ERROR_QUEST_ADD_MAX_ITEM		15  // 실패 - 퀘스트 제공아이템 받을수 없음(최대 보유수량)
#define ERROR_QUEST_ADD_LICENSE         16  // 실패 - 미인증 의뢰 아이템

enum eQUESTADD_TYPE
{
	eQUESTADD_NPC = 0,		/// NPC 에 의한 퀘스트 발동
	eQUESTADD_ETC,			/// 그외의 발동( 아이템, 사망시 등 )
};

/// 신규퀘스트 등록 신청
struct MSG_REQ_QUEST_ADD : MSGROOT 
{
	unsigned long	questIndex;
	unsigned long	npcIndex;		/// npc 인덱스
};

struct MSG_RES_QUEST_ADD : MSG_ERROR
{
	sQuestList	table;
};

/// 퀘스트 목록 
struct MSG_RES_QUEST_LIST : MSGROOT
{
	unsigned char	rowCount;	
	sQuestList		table[1];
};

/// 퀘스트 갱신
struct MSG_RES_QUEST_UPDATE : MSGROOT
{
	unsigned char	rowCount;
	sQuestList		table[1];
};

#define ERROR_QUEST_DEL_SUCCESS			0	// 성공 - 퀘스트 포기 성공
#define ERROR_QUEST_DEL_FAIL			1	// 실패 - 오류
#define ERROR_QUEST_DEL_NOTYET			2	// 실패 - DB 사용중
#define ERROR_QUEST_DEL_QUEST			3	// 실패 - 잘못된 퀘스트

/// 퀘스트 삭제
struct MSG_REQ_QUEST_DEL : MSGROOT
{
	int			  arrayIdx;					// 퀘스트 목록내 인덱스
	unsigned long questIndex;			
};

struct MSG_RES_QUEST_DEL : MSG_ERROR
{};

#define ERROR_QUEST_DELAUTO_SUCCESS		0	// 성공 - 퀘스트 실패요청 성공
#define ERROR_QUEST_DELAUTO_FAIL		1	// 실패 - 오류
#define ERROR_QUEST_DELAUTO_NOTYET		2	// 실패 - DB 사용중

/// 퀘스트 삭제 - 시간 오버로 인한 자동삭제
struct MSG_REQ_QUEST_DELAUTO : MSGROOT
{
	int			  arrayIdx;					// 퀘스트 목록내 인덱스
	unsigned long questIndex;			
};

struct MSG_RES_QUEST_DELAUTO : MSG_ERROR
{};

/*
#define ERROR_QUEST_ADD_SUCCESS			0	// 성공 - 퀘스트 추가 성공
#define ERROR_QUEST_ADD_FAIL			1	// 실패 - 알수 없는 오류
#define ERROR_QUEST_ADD_NOTYET			2	// 실패 - DB 사용중
#define ERROR_QUEST_ADD_EXIST			3	// 실패 - 이미 해당 퀘스트를 가지고있음
#define ERROR_QUEST_ADD_COMPLETE		4	// 실패 - 이미 완료한 퀘스트
#define ERROR_QUEST_ADD_TRIGGER			5	// 실패 - 퀘스트 받을 조건이 안됨
#define ERROR_QUEST_ADD_MAX				6	// 실패 - 퀘스트 목록이 FULL 
#define ERROR_QUEST_ADD_ITEM			7	// 실패 - 퀘스트 제공아이템 받을수 없음
#define ERROR_QUEST_ADD_NPC				8   // 실패 - NPC 인증 실패
#define ERROR_QUEST_ADD_TIME			9   // 실패 - 다시받을수 있는 시간이 안됬음
#define ERROR_QUEST_ADD_MONEY			10  // 실패 - 지불할 금액이 부족함
#define ERROR_QUEST_ADD_POINT			11  // 실패 - 지불할 포인트가 부족함
#define ERROR_QUEST_ADD_ITEMFAIL		12	// 실패 - 아이템
#define ERROR_QUEST_ADD_COOLTIME		13	// 실패 - 아이템 쿨타임
#define ERROR_QUEST_ADD_TUTORIALMODE	14	// 실패 - 튜토리얼 모드 변경 실패
*/

/// 의뢰아이템으로 인한 퀘스트 부여
struct MSG_REQ_QUEST_ADDBYITEM : MSGROOT
{
	unsigned short	number;				/// 인벤슬롯 번호
};

struct MSG_RES_QUEST_ADDBYITEM : MSG_ERROR
{
	sQuestList	table;
};

// 미니맵으로 인한 퀘스트 신규리스트 재발송 요청
struct MSG_REQ_NEWQUESTLIST : MSGROOT
{
	unsigned long mapIndex;
};
//
//#define ERROR_NEWQUESTLIST_SUCCESS			0	// 성공 - 완료 요청 성공
//#define ERROR_NEWQUESTLIST_FAIL				1	// 실패 - 오류
//#define ERROR_NEWQUESTLIST_MAP				2	// MAP  

struct MSG_SYN_NEWQUESTLIST : public MSGROOT
{
	unsigned short	rowCount;
	unsigned long	questIdxList[1];	/// 신규 가능 퀘스트 목록
};

struct MSG_SYN_NEXT_LEVEL_NEWQUESTLIST : public MSG_SYN_NEWQUESTLIST
{
};

#pragma pack(pop)
