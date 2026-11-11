/* ====================================================================
 * 파    일 :	sqlGame.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_GAME_H__
#define __SQL_GAME_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "stdafx.h"

#include "sqlpool.h"
#include "sqlgamestmt.h"
#include "iocontextpool.h"

#include "DramaturgyManager.h"
#include "Skillscript.h"
#include "QuestManager.h"
#include "TriggerManager.h"
#include "TitleManager.h"
#include "StatusScript.h"
#include "DropScript.h"


#ifndef __SQL_REQUEST_GAME_TYPE__
#define __SQL_REQUEST_GAME_TYPE__

enum SQL_REQUEST_GAME_TYPE
{
	// DEFAULT_SQL_GAME
	SQL_REQUEST_GAME_TEST = 0x2000,				// DEFAULT_SQL_GAME(0x2000) - defined gamesrv.h file
	SQL_REQUEST_GAME_INIT,						// Init Game Database
	SQL_REQUEST_GAME_SHUTDOWN,					// Shutdown Game Database

	SQL_GAME_PROCESS_THEME_CREATE,				// Create Theme (Info & Room).
	SQL_GAME_PROCESS_THEME_ROOM_CHECK,			// Check  "
	SQL_GAME_PROCESS_THEME_USER_CHECK,			// Check  Theme User.
	SQL_GAME_PROCESS_THEME_USER_INSERT,			// Insert Theme User.
	SQL_GAME_PROCESS_THEME_USER_SELECT,			// Select Theme User.
	SQL_GAME_PROCESS_THEME_USER_UPDATE,			// Update Theme User.
	SQL_GAME_PROCESS_THEME_USER_REMOVE,			// Remove Theme User.
	SQL_GAME_PROCESS_THEME_USER_SAVE,			// Save   Theme User.
	SQL_GAME_PROCESS_THEME_PARTY_SELECT,	    // Select Theme Party.
	SQL_GAME_PROCESS_THEME_PARTYROOM_SELECT,	// Select Theme Party.
	SQL_GAME_PROCESS_GM_THEME_USER_SELECT,		// Select Theme User.(GM Cheat)

	// Item Table Data
	SQL_GAME_PROCESS_VERIFY_ITEM_DEFINE,		// 아이템 테이블 검사 - 정의
	SQL_GAME_PROCESS_VERIFY_ITEM_ABILITY,		// "      테이블 검사 - 속성
	SQL_GAME_PROCESS_VERIFY_ITEM_COOLTIME2,		// "      테이블 검사 - 쿨타임
	SQL_GAME_PROCESS_VERIFY_ITEM_LIMIT,			// "      테이블 검사 - 제한
	SQL_GAME_PROCESS_VERIFY_ITEM_CARD,			// "      테이블 검사 - 카드
	SQL_GAME_PROCESS_VERIFY_ITEM_CARD_SLOT,		// "      테이블 검사 - 카드슬롯
	SQL_GAME_PROCESS_VERIFY_ITEM_TAROT,			// "      테이블 검사 - 타로
	SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCED,		// "      테이블 검사 - 강화
	SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCEDRATE,	// "      테이블 검사 - 확률(강화)
	SQL_GAME_PROCESS_VERIFY_ITEM_DISJOINT,		// "      테이블 검사 - 분해
	SQL_GAME_PROCESS_VERIFY_ITEM_CHANGE,		// "      테이블 검사 - 교환
	
	SQL_GAME_PROCESS_ITEM_DEFINE,				// 아이템 테이블 검색 - 정의
	SQL_GAME_PROCESS_ITEM_ABILITY,				// 아이템 테이블 검색 - 속성
	SQL_GAME_PROCESS_ITEM_COOLTIME2,			// 아이템 테이블 검색 - 쿨타임
	SQL_GAME_PROCESS_ITEM_LIMIT,				// 아이템 테이블 검색 - 제한
	SQL_GAME_PROCESS_ITEM_CARD,					// 아이템 테이블 검색 - 카드
	SQL_GAME_PROCESS_ITEM_CARD_SLOT,			// 아이템 테이블 검색 - 카드슬롯
	SQL_GAME_PROCESS_ITEM_TAROT,				// 아이템 테이블 검색 - 타로
	SQL_GAME_PROCESS_ITEM_ENHANCED,				// 아이템 테이블 검색 - 강화
	SQL_GAME_PROCESS_ITEM_ENHANCEDRATE,			// 아이템 테이블 검색 - 확률(강화)
	SQL_GAME_PROCESS_ITEM_DISJOINT,				// 아이템 테이블 검색 - 분해
	SQL_GAME_PROCESS_ITEM_CHANGE,				// 아이템 테이블 검색 - 교환

	SQL_GAME_PROCESS_VERIFY_DEFAULT_ITEMS,		// 기본지급 아이템 테이블 검사.

	// Character Table Data
	SQL_GAME_PROCESS_CHARACTER_LIST,			// 캐릭터 목록검색
	SQL_GAME_PROCESS_CHARACTER_NAME,			// 캐릭터 이름
	SQL_GAME_PROCESS_CHARACTER_INSERT,			// 캐릭터 추가
	SQL_GAME_PROCESS_CHARACTER_DELETE_WAIT,		// 캐릭터 삭제대기
    SQL_GAME_PROCESS_CHARACTER_DELETE_CANCEL, 	// 캐릭터 삭제취소
	SQL_GAME_PROCESS_CHARACTER_DELETE,			// 캐릭터 삭제
	SQL_GAME_PROCESS_CHARACTER_UPDATE,			// 캐릭터 갱신
	SQL_GAME_PROCESS_CHARACTER_SELECT,			// 캐릭터 선택
	SQL_GAME_PROCESS_CHARACTER_MONEY,			// 캐릭터 소지금액
	SQL_GAME_PROCESS_CHARACTER_DEPOSIT,			// 캐릭터 예치금액
	SQL_GAME_PROCESS_CHARACTER_TAROT_POINT,		// 캐릭터 타로포인트
	SQL_GAME_PROCESS_CHARACTER_ACTIVE_WEPON,	// 캐릭터 활성 무기정보
	SQL_GAME_PROCESS_CHARACTER_OPTION,			// 캐릭터 옵션
	SQL_GAME_PROCESS_CHARACTER_FORCETYPE,		// 캐릭터 세력 타입
	SQL_GAME_PROCESS_CHARACTER_SKILLRESET,		// 캐릭터 스킬 초기화
	SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT,	// 캐릭터 블록(채팅) 검색
	SQL_GAME_PROCESS_CHARACTER_BLOCK_INSERT,	// 캐릭터 블록(채팅) 추가
	SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT_LOG,// 캐릭터 블록(채팅)중인 유저 검색
	SQL_GAME_PROCESS_CHARACTER_GM_NAME,			// 캐릭터 GM이름 등록
	SQL_GAME_PROCESS_CHARACTER_INVEN_SIZE,		// 캐릭터 인벤토리 크기
	SQL_GAME_PROCESS_CHARACTER_TTL,				// 캐릭터 TTL
	SQL_GAME_PROCESS_CHARACTER_THEMERESET,		// 캐릭터 테마 초기화
	SQL_GAME_PROCESS_CHARACTER_CHANNEL,			// 캐릭터 접속 채널 검색
	SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET,	// 캐릭터 GM 테마 초기화
	SQL_GAME_PROCESS_CHARACTER_KICK_SELECT,		// 캐릭터 강제 종료

	// Inventory & Item Table Data
	SQL_GAME_PROCESS_INVENTORY_INSERT,			// 인벤토리 추가
	SQL_GAME_PROCESS_INVENTORY_SELECT,			// 인벤토리 검색
    SQL_GAME_PROCESS_INVENTORY_CASH_SELECT,     // 인벤토리 캐쉬아이템
	SQL_GAME_PROCESS_INVENTORY_DELETE,			// 인벤토리 삭제
	SQL_GAME_PROCESS_INVENTORY_COOLTIME_SELECT,	// 인벤토리 쿨타임(검색)
	SQL_GAME_PROCESS_INVENTORY_COOLTIME_INSERT,	// 인벤토리 쿨타임
	SQL_GAME_PROCESS_INVENTORY_REMOVE,			// 인벤토리 삭제
	SQL_GAME_PROCESS_INVENTORY_USE,				// 인벤토리 사용
	SQL_GAME_PROCESS_INVENTORY_SWAP,			// 이벤토리 교환
	SQL_GAME_PROCESS_INVENTORY_MOVE,			// 인벤토리 이동
	SQL_GAME_PROCESS_INVENTORY_EXCEPT,			// 인벤토리 예외(예외적인 이동처리)
	SQL_GAME_PROCESS_INVENTORY_MERGE,			// 인벤토리 병합(예외적인 이동처리)
	SQL_GAME_PROCESS_INVENTORY_DIVIDE,			// 인벤토리 나누기
	SQL_GAME_PROCESS_INVENTORY_MOVE2,			// 인벤토리 이동(창고영역 포함)
	SQL_GAME_PROCESS_INVENTORY_ENHANCED,		// 인벤토리 강화
	SQL_GAME_PROCESS_INVENTORY_DISJOINT,		// 인벤토리 분해
	SQL_GAME_PROCESS_INVENTORY_PUT_CARD,		// 인벤토리 카드장착
	SQL_GAME_PROCESS_INVENTORY_CHANGE,			// 인벤토리 체인지
	SQL_GAME_PROCESS_INVENTORY_SEAL,			// 인벤토리 인증여부
	SQL_GAME_PROCESS_ITEM_BILL_SELECT,			// 아이템 빌 검색
	SQL_GAME_PROCESS_ITEM_BILL_REMOVE,			// 아이템 빌 삭제
	SQL_GAME_PROCESS_ITEM_BILL_UPDATE,			// 아이템 빌 갱신
	SQL_GAME_PROCESS_ITEM_SELL,					// 아이템 판매
	SQL_GAME_PROCESS_ITEM_BUY,					// 아이템 구매
	SQL_GAME_PROCESS_ITEM_COLLECT,				// 아이템 수집(컬렉션북)
	SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE,       // 아이템 수집(카드변환)
	SQL_GAME_PROCESS_ITEM_GET,					// 아이템 습득(줍기)
	SQL_GAME_PROCESS_ITEM_GET_QUEST,			// 아이템 습득(퀘스트)
	SQL_GAME_PROCESS_ITEM_GET_PARTY_GIVE,		// 아이템 습득(파티장 지급)
	SQL_GAME_PROCESS_ITEM_USE,					// 아이템 사용(인벤토리 외)
	SQL_GAME_PROCESS_ITEM_EXCHANGE,				// 아이템 거래
	SQL_GAME_PROCESS_ITEM_MIX,					// 아이템 조합
	SQL_GAME_PROCESS_ITEM_AGENT_CHECK,			// 아이템 판매대행 검사
	SQL_GAME_PROCESS_ITEM_AGENT_SELECT_OWNER,	// 아이템 판매대행 검색(소유)
	SQL_GAME_PROCESS_ITEM_AGENT_INSERT,			// 아이템 판매대행 등록
	SQL_GAME_PROCESS_ITEM_AGENT_DELETE,			// 아이템 판매대행 삭제
	SQL_GAME_PROCESS_ITEM_AGENT_UPDATE,			// 아이템 판매대행 갱신
	SQL_GAME_PROCESS_ITEM_AGENT_SELECT,			// 아이템 판매대행 검색
	SQL_GAME_PROCESS_ITEM_AGENT_SELECT_NAME,	// 아이템 판매대행 검색(이름)
	SQL_GAME_PROCESS_ITEM_AGENT_SELECT_TYPE,	// 아이템 판매대행 검색(종류)
	SQL_GAME_PROCESS_ITEM_AGENT_SELECT_ALL,		// 아이템 판매대행 검색(모두)
	SQL_GAME_PROCESS_ITEM_SUMMON_POST,		    // 소환 주문서(우편함)
	SQL_GAME_PROCESS_ITEM_SUMMON_AGENT,		    // 소환 주문서(판매대행)
	SQL_GAME_PROCESS_ITEM_SUMMON_WAREHOUSE,	    // 소환 주문서(창고)

	SQL_GAME_PROCESS_ITEM_GATHERING_USE,		// 아이템 소모(채집시)

	SQL_GAME_PROCESS_ITEM_NPC_EVENT,			// NPC이벤트 아이템 지급


	SQL_GAME_PROCESS_ITEM_OBT_EVENT,			// 일본 OBT이벤트 아이템 지급


	// Post Table Data
	SQL_GAME_PROCESS_POST_CHECK,				// 우편 검사(안읽은우편)
	SQL_GAME_PROCESS_POST_SELECT,				// 우편 검색
	SQL_GAME_PROCESS_POST_INSERT,				// 우편 추가
	SQL_GAME_PROCESS_POST_UPDATE_STATUS,		// 우편 갱신(상태)
	SQL_GAME_PROCESS_POST_UPDATE,				// 우편 갱신
	SQL_GAME_PROCESS_POST_DELETE,				// 우편 삭제

	// Friend Table Data
	SQL_GAME_PROCESS_FRIEND_SELECT,				// 친구 검색
	SQL_GAME_PROCESS_FRIEND_INSERT,				// 친구 추가
	SQL_GAME_PROCESS_FRIEND_UPDATE,				// 친구 갱신
	SQL_GAME_PROCESS_FRIEND_DELETE,				// 친구 삭제

	// Item Table Data
	SQL_GAME_PROCESS_ITEM_SPREAD_VALUE,			// 아이템 스프레드 결과값 반환
	SQL_GAME_PROCESS_ITEM_TAROT_RESULT,			// 아이템 타로효과 결과값 반환

	SQL_GAME_PROCESS_ITEM_TUTORIALSPREAD_VALUE, // 아이템 스프레드 결과값 반환
	SQL_GAME_PROCESS_ITEM_TUTORIALTAROT_RESULT,	// 아이템 타로효과 결과값 반환

	SQL_GAME_PROCESS_ITEM_SELF_SPREAD_VALUE,	// 아이템 스프레드 결과값 반환
	SQL_GAME_PROCESS_ITEM_SELF_TAROT_RESULT,	// 아이템 타로효과 결과값 반환

	// Skill Table Data
	SQL_GAME_PROCESS_SKILL_SELECT,				// 보유스킬 리스트
	SQL_GAME_PROCESS_SKILL_INSERT,				// 보유스킬 추가
	SQL_GAME_PROCESS_SKILL_UPDATE,				// 보유스킬 단계업
	SQL_GAME_PROCESS_SKILL_COOLTIME,			// 보유스킬 쿨타임변경

	SQL_GAME_PROCESS_SKILL_CHEAT_INSERT,		// 보유스킬 치트 추가
	SQL_GAME_PROCESS_SKILL_CHEAT_UPDATE,		// 보유스킬 치트 단계업

	// Skill Influence Data
	SQL_GAME_PROCESS_INFLUENCE_SELECT,			// 효과 리스트 
	SQL_GAME_PROCESS_INFLUENCE_UPDATE,			// 효과 추가

	// Stall Sell Table Data
	SQL_GAME_PROCESS_STALL_SELL_GET,			// 판매 아이템 구입

	// Stall Buy Table Data
	SQL_GAME_PROCESS_STALL_BUY_SEARCH,			// 구입 리스트 검색
	SQL_GAME_PROCESS_STALL_BUY_INSERT,			// 구입 리스트 등록
	SQL_GAME_PROCESS_STALL_BUYL_DELETE,			// 구입 리스트 삭제

	// Shortcut Table Data
	SQL_GAME_PROCESS_SHORTCUT_SELECT,			// 큇슬롯 검색
	SQL_GAME_PROCESS_SHORTCUT_UPDATE,			// 큇슬롯 갱신

	// Quest Table Data
	SQL_GAME_PROCESS_QUEST_INSERT,				// 퀘스트 신규 등록
	SQL_GAME_PROCESS_QUEST_INSERTBYITEM,		// 퀘스트 신규 등록 - 의뢰아이템
	SQL_GAME_PROCESS_QUEST_SELECT,				// 퀘스트 리스트 받기
	SQL_GAME_PROCESS_QUEST_UPDATE,				// 퀘스트 정보 업데이트
	SQL_GAME_PROCESS_QUEST_DELETE,				// 퀘스트 정보 삭제
	SQL_GAME_PROCESS_QUEST_DELETEAUTO,			// 퀘스트 정보 자동 삭제
	SQL_GAME_PROCESS_QUEST_COMPLETE,			// 퀘스트 완료 요청
	SQL_GAME_PROCESS_QUEST_REWARD,				// 퀘스트 보상 요청
	SQL_GAME_PROCESS_QUEST_ENDLISTSELECT,		// 퀘스트 완료 목록 받기
	SQL_GAME_PROCESS_QUEST_VALIDLISTSELECT,		// 퀘스트 만기일 목록 받기
	SQL_GAME_PROCESS_QUEST_REMOVE,				// 퀘스트 제거
	
	// Title Data
	SQL_GAME_PROCESS_TITLE_INSERT,				// 호칭 추가
	SQL_GAME_PROCESS_TITLE_SELECT,				// 호칭 리스트 받기

	// Guild Data
	SQL_GAME_PROCESS_VERIFY_GUILD,				// 길드 리스트 검사
	SQL_GAME_PROCESS_GUILDUSER_SELECT,			// 길드 유저 리스트 검색
	
	SQL_GAME_PROCESS_GUILD_SELECT,				// 길드 유저 리스트 검색
	SQL_GAME_PROCESS_GUILD_CREATE,				// 길드 생성
	SQL_GAME_PROCESS_GUILD_DELETE,				// 길드 해체
	SQL_GAME_PROCESS_GUILD_UPDATE,				// 길드 저장
	SQL_GAME_PROCESS_GUILDUSER_ADD,				// 길드 유저 추가
	SQL_GAME_PROCESS_GUILDUSER_OUT,				// 길드 유저 탈퇴
	SQL_GAME_PROCESS_GUILDUSER_CUT,				// 길드 유저 추방
	SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION,	// 길드 유저 직위변경
	SQL_GAME_PROCESS_GUILDMARK_UPDATE,			// 길드 마크 갱신

	// 제조스킬
	SQL_GAME_PROCESS_MAKESKILL_INSERT,
	SQL_GAME_PROCESS_MAKESKILL_SELECT,
	SQL_GAME_PROCESS_MAKESKILL_DELETE,

	SQL_GAME_PROCESS_RECIPE_INSERT,
	SQL_GAME_PROCESS_RECIPE_GROUP_INSERT,
	SQL_GAME_PROCESS_RECIPE_COOLTIME,

	// 운세 속성
	SQL_GAME_PROCESS_FORTUNE_SELECT,
	SQL_GAME_PROCESS_FORTUNE_UPDATE,

	// pvp 세력 포인트
	SQL_GAME_PROCESS_FORCEPOINT_UPDATE,

	// PVP Table Data
	SQL_GAME_PROCESS_PVP_USER_INSERT,
	SQL_GAME_PROCESS_PVP_USER_SELECT,
	SQL_GAME_PROCESS_PVP_USER_REMOVE,
	SQL_GAME_PROCESS_PVP_USER_CLEAR,

	// Party Table Data
	SQL_GAME_PROCESS_PARTY_INSERT,

	// Whisper(귀엣말)
	SQL_GAME_PROCESS_WHISPER,

	// GM 전직 치트
	SQL_GAME_PROCESS_JOB_CHEAT_UPDATE,

	SQL_GAME_PROCESS_VERIFY_GM_EVENT,	// GM이벤트 테이블 검사
	SQL_GAME_PROCESS_GM_EVENT_SEARCH,	// GM이벤트 테이블 검사 = SQL_GAME_PROCESS_VERIFY_GM_EVENT 과 동일. 처리결과만 따로 처리함(클라이언트에게 정보전송)
	SQL_GAME_PROCESS_GM_EVENT_INSERT,	// GM이벤트 추가
	SQL_GAME_PROCESS_GM_EVENT_UPDATE,   // GM이벤트 삭제( db 테이블에서 apply=1 로 셋팅)

	SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT,	// 드랍 아이템 제약
	SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE,	// 드랍된 아이템 정보 갱신

	// MAX
	MAX_SQL_GAME
};

#endif // __SQL_REQUEST_GAME_TYPE__


class cSQLGame :
	public cSQLPool
{
private:
	cIoContextPool*   mIoContextPool;

protected:
// Page Pool 관리 메소드.
	PerSQLConnection* AllocSQLConnection ( void );

private:
// LOG 관련 메소드.
	bool              PostServerEvent         ( LPCTSTR format, ... );

public:
// DB Clear 메소드
	bool              call_stored_procedure   ( char* dsn, char* uid, char* pwd, char* sp_name );

	bool              sp_inventory            ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_inventory     ( char* dsn, char* uid, char* pwd );
	bool              sp_delete_inventory     ( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_inventory ( char* dsn, char* uid, char* pwd );

	bool              sp_influence            ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_influence     ( char* dsn, char* uid, char* pwd );
	bool              sp_delete_influence     ( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_influence ( char* dsn, char* uid, char* pwd );

	bool              sp_post                 ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_post          ( char* dsn, char* uid, char* pwd );
	bool              sp_delete_post          ( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_post      ( char* dsn, char* uid, char* pwd );

	bool              sp_agent                ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_agent         ( char* dsn, char* uid, char* pwd );
	bool              sp_delete_agent         ( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_agent     ( char* dsn, char* uid, char* pwd );

	bool              sp_quest_progress       ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_quest_progress( char* dsn, char* uid, char* pwd );
	bool              sp_delete_quest_progress( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_quest_progress( char* dsn, char* uid, char* pwd );

	bool              sp_quest_valid          ( char* dsn, char* uid, char* pwd, int& total, int& remove );
	bool              sp_remove_quest_valid   ( char* dsn, char* uid, char* pwd );
	bool              sp_delete_quest_valid   ( char* dsn, char* uid, char* pwd );
	bool              sp_db_reindex_quest_valid( char* dsn, char* uid, char* pwd );

public:
// Item Insert 메소드(Script).
	bool              ConsoleInsertInit       ( );
	void              ConsoleInsertRelease    ( );

	bool              InsertItemDefine        ( char* dsn, char* uid, char* pwd );
	bool              InsertItemAbility       ( char* dsn, char* uid, char* pwd );
	bool              InsertItemLimit         ( char* dsn, char* uid, char* pwd );
	bool              InsertItemCard          ( char* dsn, char* uid, char* pwd );
	bool              InsertItemCardSlot      ( char* dsn, char* uid, char* pwd );
	bool              InsertItemTarot         ( char* dsn, char* uid, char* pwd );
	bool              InsertItemTarotResult   ( char* dsn, char* uid, char* pwd );
	bool              InsertItemSpread        ( char* dsn, char* uid, char* pwd );
	bool              InsertItemDisjoint      ( char* dsn, char* uid, char* pwd );
	bool              InsertItemEnhanced      ( char* dsn, char* uid, char* pwd );
	bool              InsertItemEnhancedRate  ( char* dsn, char* uid, char* pwd );
	bool              InsertItemChange        ( char* dsn, char* uid, char* pwd );
	bool              InsertDefaultItems      ( char* dsn, char* uid, char* pwd );
	bool              InsertSkillPC           ( char* dsn, char* uid, char* pwd );
	bool              InsertDefaultSkill      ( char* dsn, char* uid, char* pwd );
	bool              InsertJobTree           ( char* dsn, char* uid, char* pwd );
	bool			  InsertQuestData         ( char* dsn, char* uid, char* pwd );
	bool			  InsertItemDropLimitData ( char* dsn, char* uid, char* pwd );

public:
// 클래스 생성 및 공용 메소드.
	cSQLGame(void);

	bool              Initialize              ( char* dsn, char* uid, char* pwd, unsigned int numWorkerThreads=2, unsigned int bufferLength=65535 );
	void              Shutdown                ( void );

	void              GetIoContextPool        ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );

	PerIoContext*     Get                     ( );
	void              Release                 ( PerIoContext* perIoContext );

	DWORD             WorkerThread            ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLGame(void);

private:
	cSkillScript*		mpSkillScript;
	cQuestManager*		mpQuestManager;
	cTriggerManager*	mpTriggerManager;
	cTitleManager*		mpTitleManager;
	cStatusScript*		mpStatusScript;
	cDropScript*		mpDropScript;
};

#endif // __SQL_GAME_H__
