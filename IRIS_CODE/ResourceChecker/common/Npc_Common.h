/* ==========================================================================
* 파    일 : Npc_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-04-25
* 주의사항 :
*===========================================================================*/

#pragma once
#pragma pack(push,1)

#include "CommonNetworkMsgStruct.h"
#include "Item_common.h"
#include "Quest_Common.h"

#define NPC_VALID_DISTANCE		350.0f
#define NPC_FUNCHAVE_MAX		5			// 한 npc가 가질 수 있는 최대 기능 갯수

/// npc 대화 방식
enum eNPC_TALK_TYPE
{
	eNPCTALK_OPEN = 0,			/// NPC대화 시작
	eNPCTALK_ITEMBUY,			/// item 사기
	eNPCTALK_ITEMSELL,			/// item 팔기
	eNPCTALK_INVENTORY,			/// 창고 사용
	eNPCTALK_QUESTADD,			/// 퀘스트 추가
	eNPCTALK_QUESTREWARD,		/// 퀘스트 보상
	eNPCTALK_GUILDCREATE,		/// 길드 생성
	eNPCTALK_FORTUNETAROT,		/// NPC 운세 점집
	eNPCTALK_TAROT,				/// 유저형태 점집
	eNPCTALK_DEAL,				/// 판매대행
	eNPCTALK_THEME,				/// 테마던젼
};

// NPC 타입
enum eNpcType 
{
	eNPCTYPE_NORMAL = 0,		// 일반
	eNPCTYPE_MAIL,				// 우편함
	eNPCTYPE_BOARD,				// 게시판
	eNPCTYPE_TUTORIALBOOK,		// 튜토리얼 북 형
};

/// Npc 기능
enum eNpcFunc
{
	NPC_FUNC_STORE = 0,
	NPC_FUNC_WAREHOUSE,
	NPC_FUNC_ITEMDISJOINT,		/// 아이템 분해
	NPC_FUNC_GUILD,				/// 길드
	NPC_FUNC_FORTUNETAROT,		/// 운세 타롯
	NPC_FUNC_TAROT,				/// 유저형태 타롯
	NPC_FUNC_DEAL,				/// 판매대행
	NPC_FUNC_THEME,				/// 테마던젼
	NPC_FUNC_MAIL,				/// 우편
	NPC_FUNC_SKILLPREVIEW,		/// 스킬 미리보기
	NPC_FUNC_BOARD,				/// 게시판
	NPC_FUNC_TUTORIALBOOK,		/// 튜토리얼 북

	NPC_FUNC_MAX,
};

struct sNPCList
{
	unsigned long	mNpcClassIdx;	/// NPC 종류
	TCHAR			mName[32];		/// NPC 이름
	TCHAR			mJobName[32];	/// JOB 이름
	unsigned short  mType;			/// npc type
	unsigned int	mModelIndex;	/// NPC 모델 번호
	unsigned long	mJobIconIndexLarge;	/// 직업아이콘 인덱스 - 큰지도용
	unsigned long	mJobIconIndexSmall;	/// 직업아이콘 인덱스 - 작은지도용
	unsigned long   mJobIconIndexTarget;/// 직업아이콘 인덱스 - 타겟창용
};

struct sNPCRegen
{
	unsigned long	mNpcClassIdx;	/// NPC 종류
	unsigned short	mMapNumber;		/// 맵번호
	float			mPosX;			/// X좌표
	float			mPosY;			/// Y좌표
	float			mDirection;		/// 방향
};

/// NPC 전체 데이터
struct sNpcData
{
	unsigned long	mNpcIdx;		/// 고유인덱스
	unsigned int	mNpcClassIdx;	/// 종류번호
	unsigned short	mMapNumber;		/// 맵번호
	float			mPosX;			/// 좌표
	float			mPosY;
	float			mDirection;		/// 방향
};

/// NPC Func 데이터
struct sNpcFuncData
{
	int				mFuncCount;		/// 기능 갯수
	unsigned long	mFunc[5];		/// npc store 타입
	unsigned long   mThemeIndex;

	unsigned long	mTarotPrice;	 /// 
	unsigned long	mTarotIndex[22]; /// 타롯 인덱스
	unsigned long	mSpreadIndex[5]; /// 스프레드 인덱스
	unsigned int	mSpreadCount;

	sNpcFuncData()
	{
		mFuncCount = 0;
		memset( mFunc, 0, sizeof(mFunc) );
		mThemeIndex = 0;
		mTarotPrice = 0;
		mSpreadCount = 0;

		memset( mTarotIndex, 0, sizeof(mTarotIndex) );
		memset( mSpreadIndex, 0, sizeof(mSpreadIndex) );
	}
};

/// NPC 구조체
struct MSG_NPC_INFO : public MSGROOT
{
	unsigned long	mCount;			/// NPC 수
	sNpcData		mNpcInfo[1];	/// NPC 정보

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_NPC_INFO) - sizeof(mNpcInfo) + (mCount * sizeof(sNpcData)); }
};

/// NPC IDX
struct MSG_NPCIDX : public MSGROOT
{
	unsigned long	mNpcIdx;
};


#define ERROR_NPC_OPEN_SUCCESS		0	/// 성공
#define ERROR_NPC_OPEN_RANGE		1	/// 허용거리 밖
#define ERROR_NPC_OPEN_PLAYERSTATE	2	/// 플레이어 상태 오류
#define ERROR_NPC_OPEN_NPC			3	/// npc 대화 상태가 틀림
#define ERROR_NPC_OPEN_DEALLIST		4	/// npc에 맞지않는 목록
#define ERROR_NPC_OPEN_ALREADY		5	/// 대화중인 npc에 다시 요청 들어옴

#define ERROR_NPC_TAROT_JOIN_SUCESS		6	// 성공
#define ERROR_NPC_TAROT_JOIN_FAIL		7	// 실패
#define ERROR_NPC_TAROT_JOIN_MONEY		8	// 실패 - 금액부족
#define ERROR_NPC_TAROT_JOIN_ONCE		9	// 실패 - 하루에 한번만 가능

struct MSG_REQ_NPC_OPEN : public MSGROOT
{
	unsigned long	mNpcIdx;
};

struct MSG_RES_NPC_OPEN : public MSG_ERROR
{
	unsigned long	mNpcIdx;
	float			mPosX;
	float			mPosY;
};

struct MSG_SYN_NPC_OPEN : public MSG_ERROR
{
	unsigned long	mCharacterIdx;
	float			mPosX;
	float			mPosY;
};

struct MSG_SYN_NPC_CLOSE : public MSG_ERROR
{
	unsigned long	mCharacterIdx;
};

// 창고 이용중 입금 
#define ERROR_NPC_DEPOSIT_SUCCESS	0	// 성공
#define ERROR_NPC_DEPOSIT_FAIL		1   // 실패
#define ERROR_NPC_DEPOSIT_NPC		2   // npc인증 실패

struct MSG_REQ_NPC_DEPOSIT : public MSGROOT
{
	unsigned long npcIdx;
	unsigned long money;
};
struct MSG_RES_NPC_DEPOSIT : public MSG_ERROR
{};

// 창고 이용중 출금
#define ERROR_NPC_WITHDRAW_SUCCESS	0	// 성공
#define ERROR_NPC_WITHDRAW_FAIL		1   // 실패
#define ERROR_NPC_WITHDRAW_NPC		2   // npc인증 실패

struct MSG_REQ_NPC_WITHDRAW : public MSGROOT
{
	unsigned long npcIdx;
	unsigned long money;
};
struct MSG_RES_NPC_WITHDRAW : public MSG_ERROR
{};


/*-- 아이템 인벤 -> 창고
*/
#define ERROR_ITEM_MOV2_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_MOV2_INVENTORY_MONEY			1	// 금액부족
#define ERROR_ITEM_MOV2_INVENTORY_FAIL			2	// 실패
#define ERROR_ITEM_MOV2_INVENTORY_NOTYET		3	// DB처리중
#define ERROR_ITEM_MOV2_INVENTORY_NPC			4	// npc 인증
#define ERROR_ITEM_MOV2_INVENTORY_MERGE			5	// 병합중

struct MSG_REQ_NPC_ITEM_MOV2_INVENTORY : MSGROOT
{
	unsigned long  npcIdx;							// NPC 고유번호
	unsigned short number1;							// 인벤토리 번호(원본)
	unsigned short number2;							// 인벤토리 번호(대상)
	unsigned short count;							// 수량
};
struct MSG_RES_NPC_ITEM_MOV2_INVENTORY : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_MOV2_INVENTORY) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};

/*-- 아이템 팔기(인벤토리)
*/
#define ERROR_ITEM_SELL_SUCCESS					0	// 성공
#define ERROR_ITEM_SELL_FAIL					1	// 실패
#define ERROR_ITEM_SELL_NOTYET					2   // DB처리중
#define ERROR_ITEM_SELL_NPC						3	// npc인증 실패

struct MSG_REQ_NPC_ITEM_SELL : MSGROOT
{
	unsigned long  npcIdx;							// NPC 고유번호
	unsigned short inventoryNumber;					// 인벤토리 번호
	unsigned short count;							// 수량
};

struct MSG_RES_NPC_ITEM_SELL : MSG_ERROR
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_SELL) - sizeof(inventory) + (rowCount * sizeof(sInventory)); }
};


/*-- 아이템 사기(인벤토리)
*/
#define ERROR_ITEM_BUY_SUCCESS					0	// 성공
#define ERROR_ITEM_BUY_MONEY					1	// 금액부족
#define ERROR_ITEM_BUY_PVP_POINT				2	// 전장포인트 부족
#define ERROR_ITEM_BUY_TAROT_POINT				3	// 타로포이트 부족
#define ERROR_ITEM_BUY_FAIL						4	// 실패
#define ERROR_ITEM_BUY_NOTYET					5	// DB처리중
#define ERROR_ITEM_BUY_NPC						6	// npc인증 실패

struct MSG_REQ_NPC_ITEM_BUY : MSGROOT
{
	unsigned long   npcIdx;							// NPC 고유번호
	unsigned int    tabIdx;							// NPC 고유번호
	unsigned long   itemDefineIndex;				// 아이템 인덱스
	unsigned short  count;							// 수량
};

struct MSG_RES_NPC_ITEM_BUY : MSG_ERROR
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_BUY) - sizeof(inventory) + (rowCount * sizeof(sInventory)); }
};

#define ERROR_QUEST_COMPLETE_SUCCESS		0	// 성공 - 진행 완료 요청 성공
#define ERROR_QUEST_COMPLETE_NOTYET			1	// 실패 - DB 사용중
#define ERROR_QUEST_COMPLETE_FAIL			2	// 실패 - 오류
#define ERROR_QUEST_COMPLETE_POSSIBLE		3	// 실패 - 완료조건 아님
#define ERROR_QUEST_COMPLETE_TAKE			4	// 실패 - 아이템 회수 실패
#define ERROR_QUEST_COMPLETE_NPC			5	// 실패 - NPC 인증
#define ERROR_QUEST_COMPLETE_TIME			6	// 실패 - 시간제한 오버
#define ERROR_QUEST_COMPLETE_ALREADY		7	// 실패 - 이미 완료한 퀘스트
#define ERROR_QUEST_COMPLETE_TUTORIALMODE	8	// 실패 - 튜토리얼 모드 변경 실패

/// 퀘스트 진행 완료
struct MSG_REQ_NPC_QUEST_COMPLETE : MSGROOT
{
	int			  arrayIdx;					// 퀘스트 목록내 인덱스
	unsigned long questIndex;
	unsigned long npcIndex;
};

struct MSG_RES_NPC_QUEST_COMPLETE : MSG_ERROR
{
	int			arrayIdx;
	char		status;					/// 상태(2:진행완료, 4:진행실패)
};

#define ERROR_QUEST_REWARD_SUCCESS			0	// 성공 - 완료 요청 성공
#define ERROR_QUEST_REWARD_NOTYET			1	// 실패 - DB 사용중
#define ERROR_QUEST_REWARD_FAIL				2	// 실패 - 오류
#define ERROR_QUEST_REWARD_ITEM				3	// 실패 - 아이템 보상 지급 실패
#define ERROR_QUEST_REWARD_MONEY			4   // 실패 - 보상 금액 지급 실패
#define ERROR_QUEST_REWARD_COUNT			5	// 실패 - 선택갯수 적음
#define ERROR_QUEST_REWARD_NPC				6	// 실패 - NPC 인증
#define ERROR_QUEST_REWARD_COMPLETE			7	// 실패 - 진행완료상태 아님

/// 퀘스트 보상 
struct MSG_REQ_NPC_QUEST_REWARD : MSGROOT
{
	int			  arrayIdx;					// 퀘스트 목록내 인덱스
	unsigned long questIndex;
	unsigned long npcIndex;
	unsigned int  rowCount;
	sRewardData	  table[5];					/// 선택보상 목록
};

struct MSG_RES_NPC_QUEST_REWARD : MSG_ERROR
{};

struct sNpcQuestStatus
{
	unsigned long	npcIdx;		
	unsigned char	status;				/// 상태 eNpcQuestStatus
};

// 퀘스트 관련 상태 
struct MSG_RES_NPC_QUESTSTATUS : public MSGROOT
{
	unsigned long	rowCount;			/// NPC 수
	sNpcQuestStatus table[1];
};

// 퀘스트 신규리스트 재발송 요청
struct MSG_REQ_NPC_QUESTLIST_RESEND : MSGROOT
{
	unsigned long npcIndex;
};

#define ERROR_NPC_GUILD_CREATE_SUCESS		0	// 성공
#define ERROR_NPC_GUILD_CREATE_FAIL			1	// 실패
#define ERROR_NPC_GUILD_CREATE_CONDITION	2	// 실패 - 가입조건 충족 안됨 (레벨, 금액)
#define ERROR_NPC_GUILD_CREATE_ALREADY		3	// 실패 - 이미 길드 가입중
#define ERROR_NPC_GUILD_CREATE_ERRORNAME	4	// 실패 - 길드명 없거나 12자 넘음
#define ERROR_NPC_GUILD_CREATE_EXISTNAME	5	// 실패 - 길드명 중복
#define ERROR_NPC_GUILD_CREATE_NPC			6	// 실패 - NPC 인증실패

/// 길드생성
struct MSG_REQ_NPC_GUILD_CREATE :MSGROOT
{
	unsigned long	npcIndex;
	wchar_t			name[MAX_GUILDNAME_BUFFER_SIZE];	/// 길드명
	unsigned long   GetMsgLength( ) { return sizeof(MSG_REQ_NPC_GUILD_CREATE) - sizeof(name) + wcslen(name)*sizeof(wchar_t); }
};

struct MSG_RES_NPC_GUILD_CREATE : MSG_ERROR
{};

/// NPC 점집 열어보기
struct MSG_REQ_NPC_TAROT_OPEN : MSGROOT
{};

struct MSG_RES_NPC_TAROT_OPEN : MSG_ERROR
{
	unsigned long	fortuneData;			/// 운세 비트
};

#define ERROR_NPC_TAROT_CLOSE_SUCESS	0	// 성공
#define ERROR_NPC_TAROT_CLOSE_FAIL		1	// 실패

/// NPC 점집 닫기
struct MSG_REQ_NPC_TAROT_CLOSE : MSGROOT
{};

struct MSG_RES_NPC_TAROT_CLOSE : MSG_ERROR
{};

/// NPC 점집 참여
struct MSG_REQ_NPC_TAROT_JOIN : MSGROOT
{
	unsigned long	npcIndex;				// npc 인덱스
};

struct MSG_RES_NPC_TAROT_JOIN : MSG_ERROR
{};

#define ERROR_NPC_TAROT_FORTUNE_SUCESS		0	// 성공
#define ERROR_NPC_TAROT_FORTUNE_FAIL		1	// 실패
#define ERROR_NPC_TAROT_FORTUNE_ONCE		2	// 실패 - 하루에 한번만 가능

/// 운세 선택
struct MSG_REQ_NPC_TAROT_FORTUNE : MSGROOT
{
	short			fortuneProp;			// 운세 속성
};

struct MSG_RES_NPC_TAROT_FORTUNE : MSG_ERROR
{};

/// NPC 점집 참여
struct MSG_REQ_NPC_THEME_JOIN : MSGROOT
{
	unsigned long	npcIndex;				// npc 인덱스
};

// 판매대행 시스템
#define NPC_ITEM_AGENT_TIME1	24
#define NPC_ITEM_AGENT_TIME2	48
#define NPC_ITEM_AGENT_TIME3	72

// NM_NPC_ITEM_AGENT_OPEN_REQ
#define ERROR_NPC_ITEM_AGENT_OPEN_SUCCESS	0			// 성공
#define ERROR_NPC_ITEM_AGENT_OPEN_FAIL		1			// 실패
#define ERROR_NPC_ITEM_AGENT_OPEN_NOTYET	2			// DB처리중

struct MSG_REQ_NPC_ITEM_AGENT_OPEN : public MSGROOT
{
	long npcIndex;										// npc 인덱스
};

// NM_NPC_ITEM_AGENT_OPEN_PAGE_REQ
#define ERROR_NPC_ITEM_AGENT_OPEN_PAGE_SUCCESS	0			// 성공
#define ERROR_NPC_ITEM_AGENT_OPEN_PAGE_FAIL		1			// 실패
#define ERROR_NPC_ITEM_AGENT_OPEN_PAGE_NOTYET	2			// DB처리중

struct MSG_REQ_NPC_ITEM_AGENT_PAGE_OPEN : public MSGROOT
{
	long StartPage;									// 페이지 번호 (1부터 시작 ... )
};


struct NPC_ITEM_AGENT_OPEN_RESULT
{
	long             Idx;							// 고유번호
	SYSTEMTIME       ValidThru;						// 유효만기
	long             Price;							// 판매가격
	sInventory       Inventory;						// 인벤토리
};

struct MSG_RES_NPC_ITEM_AGENT_OPEN : public MSG_ERROR
{
	long                   StartPage;				// 페이지 번호 시작(1부터...)
	long                   EndPage;					// 페이지 번호 끝
	long                   RowCount;				// 영향받은 행 수
	NPC_ITEM_AGENT_OPEN_RESULT Result[1];				// 결과

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_AGENT_OPEN) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_NPC_ITEM_AGENT_ADD_REQ
#define ERROR_NPC_ITEM_AGENT_ADD_SUCCESS	0		// 성공
#define ERROR_NPC_ITEM_AGENT_ADD_FAIL		1		// 실패
#define ERROR_NPC_ITEM_AGENT_ADD_NOTYET		2		// DB처리중
#define ERROR_NPC_ITEM_AGENT_ADD_OVER		3		// 등록제한 초과

struct MSG_REQ_NPC_ITEM_AGENT_ADD : public MSGROOT
{
	long           StartPage;						// 페이지 번호 (1부터 시작 ... )
	BYTE           ValidThru;						// 등록기간
	unsigned short Number;							// 인벤토리 번호
	unsigned short Count;							// 인벤토리 수량
	long           Price;							// 등록 가격
};
struct MSG_RES_NPC_ITEM_AGENT_ADD : public MSG_ERROR
{
	long                   StartPage;				// 페이지 번호 시작(1부터...)
	long                   EndPage;					// 페이지 번호 끝
	long                   RowCount;				// 영향받은 행 수
	NPC_ITEM_AGENT_OPEN_RESULT Result[1];				// 결과

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_AGENT_ADD) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_NPC_ITEM_AGENT_DEL_REQ
#define ERROR_NPC_ITEM_AGENT_DEL_SUCCESS	0			// 성공
#define ERROR_NPC_ITEM_AGENT_DEL_FAIL		1			// 실패
#define ERROR_NPC_ITEM_AGENT_DEL_NOTYET		2			// DB처리중

struct MSG_REQ_NPC_ITEM_AGENT_DEL : public MSGROOT
{
	long StartPage;									// 페이지 번호 (1부터 시작 ... )
	long Idx;										// 고유번호
};
struct MSG_RES_NPC_ITEM_AGENT_DEL : public MSG_ERROR
{
	sInventory             Inventory;				// 인벤토리

	long                   StartPage;				// 페이지 번호 시작(1부터...)
	long                   EndPage;					// 페이지 번호 끝
	long                   RowCount;				// 영향받은 행 수
	NPC_ITEM_AGENT_OPEN_RESULT Result[1];				// 결과

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_AGENT_DEL) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_NPC_ITEM_AGENT_SEARCH_NAME_REQ
// NM_NPC_ITEM_AGENT_SEARCH_TYPE_REQ
// NM_NPC_ITEM_AGENT_SEARCH_ALL_REQ
#define MAX_ITEM_DEFINE_NAME			51
#define ALL_ITEM_DEFINE_SUB_TYPE        255
#define ALL_INVENTORY_ENHANCED          255
#define ALL_INVENTORY_RARE		        255

#define ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS	0			// 성공
#define ERROR_NPC_ITEM_AGENT_SEARCH_FAIL	1			// 실패
#define ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET	2			// 사용중

struct MSG_REQ_NPC_ITEM_AGENT_SEARCH : public MSGROOT
{
	long startPage;									// 페이지 번호 (1부터 시작 ... )
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} sort;
		BYTE sortData;
	};
	BYTE itemDefineRareLevel;						// 아이템 레어수준
	BYTE itemLimitCharLevelB;						// 아이템 캐릭터 사용제한
	BYTE itemLimitCharLevelE;						// "
	BYTE inventoryEnhanced;							// 인벤토리 강화단계
};
struct MSG_REQ_NPC_ITEM_AGENT_SEARCH_NAME : public MSGROOT
{
	long    startPage;								// 페이지 번호 (1부터 시작 ... )
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} sort;
		BYTE sortData;
	};
	wchar_t itemDefineName[ MAX_ITEM_DEFINE_NAME ];	// 아이템 이름
	BYTE    itemDefineRareLevel;					// 아이템 레어수준
	BYTE    itemLimitCharLevelB;					// 아이템 캐릭터 사용제한
	BYTE    itemLimitCharLevelE;					// "
	BYTE    inventoryEnhanced;						// 인벤토리 강화단계
};
struct MSG_REQ_NPC_ITEM_AGENT_SEARCH_TYPE : public MSGROOT
{
	long startPage;									// 페이지 번호 (1부터 시작 ... )
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} sort;
		BYTE sortData;
	};
	BYTE  itemDefineSearchType;						// 아이템 정의 검색종류-대분류
	short itemDefineSearchSubType;					// 아이템 정의 검색종류-중분류
	BYTE  itemDefineRareLevel;						// 아이템 레어수준
	BYTE  itemLimitCharLevelB;						// 아이템 캐릭터 사용제한
	BYTE  itemLimitCharLevelE;						// "
	BYTE  inventoryEnhanced;							// 인벤토리 강화단계
};
struct MSG_REQ_NPC_ITEM_AGENT_SEARCH_ALL : public MSGROOT
{
	long    startPage;								// 페이지 번호 (1부터 시작 ... )
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} sort;
		BYTE sortData;
	};
	wchar_t itemDefineName[ MAX_ITEM_DEFINE_NAME ];	// 아이템 이름
	BYTE    itemDefineSearchType;					// 아이템 정의 검색종류-대분류
	short   itemDefineSearchSubType;				// 아이템 정의 검색종류-중분류
	BYTE    itemDefineRareLevel;						// 아이템 레어수준
	BYTE    itemLimitCharLevelB;					// 아이템 캐릭터 사용제한
	BYTE    itemLimitCharLevelE;					// "
	BYTE    inventoryEnhanced;						// 인벤토리 강화단계
};
struct NPC_ITEM_AGENT_SEARCH_RESULT
{
	long             Idx;							// 고유번호
	wchar_t          CharacterName[ 17 ];			// 캐릭터 이름(소유)
	SYSTEMTIME       ValidThru;						// 유효만기
	unsigned long    Price;							// 판매가격
	sInventory       Inventory;						// 인벤토리
};
struct MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT : public MSG_ERROR
{
	long                     StartPage;				// 페이지 번호 시작(1부터...)
	long                     EndPage;				// 페이지 번호 끝
	long                     RowCount;				// 적용된 행 수
	NPC_ITEM_AGENT_SEARCH_RESULT Result[1];				// 결과 값

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

// NM_NPC_ITEM_AGENT_GET_REQ
struct MSG_REQ_NPC_ITEM_AGENT_GET : public MSGROOT
{
	long    Idx;									// 고유번호(구매할 판매대행 고유번호)
	short   Count;									// 구매수량
	long    startPage;								// 페이지 번호 (1부터 시작 ... )
	union
	{
		struct {
			BYTE flag  : 1;							// 정렬-0내림,1오름
			BYTE filed : 1;							// 정렬-0가격,1이름
		} sort;
		BYTE sortData;
	};
	wchar_t itemDefineName[ MAX_ITEM_DEFINE_NAME ];	// 아이템 이름
	BYTE    itemDefineSearchType;					// 아이템 정의 검색종류-대분류
	short   itemDefineSearchSubType;				// 아이템 정의 검색종류-중분류
	BYTE    itemDefineRareLevel;					// 아이템 레어수준
	BYTE    itemLimitCharLevelB;					// 아이템 캐릭터 사용제한
	BYTE    itemLimitCharLevelE;					// "
	BYTE    inventoryEnhanced;						// 인벤토리 강화단계
};
struct MSG_RES_NPC_ITEM_AGENT_GET : public MSG_ERROR
{
	long                     StartPage;				// 페이지 번호 시작(1부터...)
	long                     EndPage;				// 페이지 번호 끝
	long                     RowCount;				// 적용된 행 수
	NPC_ITEM_AGENT_SEARCH_RESULT Result[1];				// 결과 값

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_NPC_ITEM_AGENT_GET) - sizeof(Result) + (RowCount * sizeof(Result)); }
};

#pragma pack(pop)
