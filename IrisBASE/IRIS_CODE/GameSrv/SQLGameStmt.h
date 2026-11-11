 /* ====================================================================
 *<
 * 파    일 :	sqlGameStmt.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_GAME_STATEMENT_H__
#define __SQL_GAME_STATEMENT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once
#include "odbcsql.h"
#include "gamesrvdefines.h"
#include "commondefines.h"
#include "makeskill_common.h"
#include "Party_Common.h"


struct INIT_GAME
{
	long channelNum;
	long removeCoolTimeCount;
};
struct SHUTDOWN_GAME
{
	long channelNum;
	long removeCoolTimeCount;
};

struct THEME_CREATE
{
	short            themeIdx;		// [In]  테마 인덱스
	BYTE             fixTime;		// [In]  고정 시간(0~23, 시간단위)
	BYTE             varTime;		// [In]  가변 시간(1~NN, 시간단위)
	short            maxRoom;		// [In]  최대 룸 갯수
	TIMESTAMP_STRUCT lastDate;		// [Out] 마지막 업데이트일
	TIMESTAMP_STRUCT validThru;		// [Out] 유효만기일
	long             retvalue;		// [Out] 반환값
};
struct THEME_ROOM_CHECK
{
	long            characterIdx;		//   유저 인덱스

	short           themeIdx;		// [In]  테마 인덱스
	BYTE            themeMode;		// [In]  모드(난이도)
	short           roomIdx;		// [Out] 룸   인덱스
	long            roomRetvalue;	// [Out] 결과

	long			retvalue;		// [Out] 유저 결과
};
struct THEME_USER_CHECK
{
	short themeIdx;					// [In]  테마   인덱스
	short roomIdx;					// [Out] 룸     인덱스
	char  themeMode;				// [Out] 모드(난이도)
	long  characterIdx;				// [In]  캐릭터 인덱스
};
struct THEME_USER_INSERT
{
	short themeIdx;					// [In]  테마   인덱스
	BYTE  themeMode;				// [In]  모드(난이도)
	short roomIdx;					// [In]  룸     인덱스
	long  characterIdx;				// [In]  캐릭터 인덱스
	long  characterMaxCnt;			// [In]  캐릭터 최대 인원
	long  retvalue;					// [Out] 결과
};
struct THEME_USER
{
	short            themeIdx;		// 테마 인덱스
	BYTE             themeMode;		// 모드(난이도)
	short            roomIdx;		// 룸   인덱스
	TIMESTAMP_STRUCT lastDate;		// 마지막 업데이트(일)
	TIMESTAMP_STRUCT validThru;		// 유효만기(일)
};
struct THEME_USER_SELECT
{
	long       characterIdx;		// [In]  캐릭터 인덱스
	long       rowCount;			// [Out] 적용된 행 수
	THEME_USER table[1];			// [Out] 결과
};
struct THEME_USER_UPDATE
{
	short themeIdx;					// [In]  테마   인덱스
	BYTE  themeMode;				// [In]  모드(난이도)
	short roomIdx;					// [Out] 룸     인덱스
	long  characterIdx;				// [In]  캐릭터 인덱스
	long  characterMaxCnt;			// [In]  캐릭터 최대 인원
	long  retvalue;					// [Out] 반환값
};
struct THEME_USER_REMOVE
{
	short themeIdx;					// [In]  테마   인덱스
	char  themeMode;				// [In]  모드(난이도)
	short roomIdx;					// [In]  룸     인덱스
	long  characterIdx;				// [In]  캐릭터 인덱스
	long  retvalue;					// [Out] 결과
};
struct THEME_USER_SAVE : public THEME_USER
{
	long  characterIdx;				// [In]  캐릭터 인덱스
	long  retvalue;					// [Out] 결과
};
struct sThemePartyInfoDB
{
	unsigned long	mPlayerIdx;		// 플레이어 인덱스
	unsigned char	mMode;			// 모드
	bool			mIsJoin;		// 입장
	bool			mIsDependency;	// 귀속
	unsigned long	mRoomIdx;		// 방번호
};
struct THEME_PARTYINFO_SELECT
{
	unsigned short	mThemeIdx;		// [In] 테마 인덱스
	unsigned char	mPartyCnt;		// [In] 파티원 인원
	unsigned long	mCharacterIdx[MAX_PARTYUSER];	// [In] 파티원 목록

	unsigned char	mCount;			// [Out] 파티정보 카운트
	sThemePartyInfoDB mPartyInfo[1];	// [Out] 파티원별 테마 정보 

	unsigned long GetMsgLength() { return sizeof(THEME_PARTYINFO_SELECT) - sizeof(mPartyInfo) + (mCount * sizeof(sThemePartyInfoDB));} 
};
struct THEME_PARTYROOM_SELECT
{
	unsigned short	mThemeIdx;		// [In] 테마 인덱스
	unsigned char	mMode;			// [In] 테마 모드
	unsigned char	mPartyCnt;		// [In] 파티원 인원
	unsigned long	mPartyUserIdx[MAX_PARTYUSER];	// [In] 파티원 목록
	unsigned short	mRoomIdx;		// [Out] 선정된방번호
};
struct THEME_GM_USER_SELECT
{
	wchar_t	   characterName[ 17 ];	// [In]  제재 당할 캐릭터 이름
	long	   characterIdx;		// [Out] 제재 당할 캐릭터 Index
	long       rowCount;			// [Out] 적용된 행 수
	THEME_USER table[1];			// [Out] 결과
};

struct VERIFY_ITEM_TABLE
{
	long count;
	long retvalue;
};
struct ITEM_DEFINE_TABLE
{
	long           idx;
	TB_ITEM_DEFINE table[1];
};

struct ITEM_ABILITY_TABLE
{
	long            idx;
	TB_ITEM_ABILITY table[1];
};

struct ITEM_COOLTIME2_TABLE
{
	long              idx;
	long              rowCount;
	TB_ITEM_COOLTIME1 table[1];
};

struct ITEM_LIMIT_TABLE
{
	long          idx;
	TB_ITEM_LIMIT table[1];
};

struct ITEM_CARD_TABLE
{
	long         idx;
	TB_ITEM_CARD table[1];
};

struct ITEM_CARD_SLOT_TABLE
{
	long              idx;
	TB_ITEM_CARD_SLOT table[1];
};

struct ITEM_TAROT_TABLE
{
	long          idx;
	TB_ITEM_TAROT table[1];
};

struct ITEM_ENHANCED_TABLE
{
	long             idx;
	TB_ITEM_ENHANCED table[1];
};

struct ITEM_ENHANCED_RATE_TABLE
{
	long                  idx;
	TB_ITEM_ENHANCED_RATE table[1];
};

struct ITEM_DISJOINT_TABLE
{
	long             idx;
	TB_ITEM_DISJOINT table[1];
};

struct ITEM_CHANGE_TABLE
{	
	long           idx;
	TB_ITEM_CHANGE table[1];
};

struct VERIFY_DEFAULT_ITEMS
{
	long rowCount;
	long retvalue;
};

struct CHARACTER_LIST
{
    long                userIdx;		// [In]  사용자 인덱스
	long                retvalue;       // [Out] 결과
	long                rowCount;		// [Out] 적용된 행의 수
	TB_CHARACTER_LIST   table[1];		// [Out] 캐릭터 테이블
};

struct CHARACTER_SELECT
{
	long         characterIdx;			// [In]  캐릭터 인덱스
	long         userIdx;				// [In]  사용자 인덱스
	TB_CHARACTER character;				// [Out] 캐릭터 테이블
};

struct CHARACTER_NAME
{
	wchar_t name[17];					// [In]  캐릭터 이름
	long    retvalue;					// [Out] 결과
};

struct CHARACTER_DELETE
{
	long  idx;							// [In]  캐릭터 고유번호
	char  level;						// [Out] 캐릭터 레벨
	long  exp;							// [Out] 캐릭터 경험치
	char  skillLevel;					// [Out] 캐릭터 스킬 레벨
	long  skillExp;						// [Out] 캐릭터 스킬 경험치
	long  skillPointTotal;				// [Out] 캐릭터 스킬 포인트
	long  guild_index;					// [Out] 캐릭터 길드 인덱스
	char  standIdx;						// [Out] 캐릭터 생성위치
	long  userIdx;						// [In]  사용자 고유번호
	long  retvalue;						// [Out] 결과
    char  request;                      // 클라이언트 요청 여부
};

struct CHARACTER_INSERT
{
	long    idx;						// [Out] 캐릭터 고유번호
	wchar_t name[17];					// [In]  캐릭터 이름
	char    race;						// [In]  캐릭터 종족
	char    gender;						// [In]  캐릭터 성별
	long    job;						// [In]  캐릭터 직업
							
	long    hair;						// [In]  캐릭터 헤어
	short   hairColor;					// [In]  캐릭터 머리색
	long    face;						// [In]  캐릭터 얼굴
	
	float   xPos;						// [In]  캐릭터 X위치
	float   yPos;						// [In]  캐릭터 Y위치
							
	long    hp;							// [In]  HP 기본값
	long    mp;							// [In]  MP 기본값
							
	char    standIdx;					// [In]  생성위치
	long    userIdx;					// [In]  사용자 고유번호

    unsigned short bagEnd;              // [In]  인벤토리 크기
    unsigned short wearEnd;             // [In]  착용 카드 슬롯
    unsigned short wareHouseEnd;        // [In]  창고 크기
    unsigned short makeSkillEnd;        // [In]  보조직업 크기

	long    retvalue;					// [Out] 결과
	long    rowCount;				    // [Out] 영향받은 행 수

	TB_INVENTORY        table[1];		// [Out] TB_INVENTORY 레코드
};

struct CHARACTER_DELETE_WAIT
{
	long  idx;							// [In]  캐릭터 고유번호
	char  level;						// [Out] 캐릭터 레벨
	long  exp;							// [Out] 캐릭터 경험치
	char  skillLevel;					// [Out] 캐릭터 스킬 레벨
	long  skillExp;						// [Out] 캐릭터 스킬 경험치
	long  skillPointTotal;				// [Out] 캐릭터 스킬 포인트
	long  guild_index;					// [Out] 캐릭터 길드 인덱스
	char  standIdx;						// [Out] 캐릭터 생성위치
	long  userIdx;						// [In]  사용자 고유번호
    long  delWaitTime;                  // [Out] 캐릭터 삭제 기한
	long  retvalue;						// [Out] 결과
};

struct CHARACTER_DELETE_CANCEL
{
	long  idx;							// [In]  캐릭터 고유번호
	char  level;						// [Out] 캐릭터 레벨
	long  exp;							// [Out] 캐릭터 경험치
	char  skillLevel;					// [Out] 캐릭터 스킬 레벨
	long  skillExp;						// [Out] 캐릭터 스킬 경험치
	long  skillPointTotal;				// [Out] 캐릭터 스킬 포인트
	long  guild_index;					// [Out] 캐릭터 길드 인덱스
	char  standIdx;						// [Out] 캐릭터 생성위치
	long  userIdx;						// [In]  사용자 고유번호
	long  retvalue;						// [Out] 결과
};

struct CHARACTER_UPDATE
{
	long  cid;                          // [In]  ConnectionIdx
	long  idx;							// [In]  캐릭터 고유번호
	short mapNumber;					// [In]  맵 번호
	float xPos;							// [In]  X위치 (소수점 포함)
	float yPos;							// [In]  Y위치 "
	long  hp;							// [In]  HP
	long  mp;							// [In]  MP
	long  tarotPoint;					// [In]  타로 포인트
	long  titleIndex;					// [In]  호칭 인덱스
	long  guildIndex;					// [In]  길드 인덱스
	char  guildPosition;				// [In]  길드 직위
	char  level;						// [In]  레벨
	long  exp;							// [In]  exp
	char  skillLevel;					// [In]  스킬레벨
	long  skillExp;						// [In]  스킬경험치
	long  skillPointRemain;				// [In]  잔여스킬포인트
	long  skillPointTotal;				// [In]  총습득스킬포인트
	long  retvalue;						// [Out] 결과
	int   flag;                         // [In]  flag
    unsigned long makeSkill1_Exp;       // [In]  전문기술1 숙련도
    unsigned long makeSkill2_Exp;       // [In]  전문기술2 숙련도
	unsigned long expRecover;			// [In]  경험치 복구량
	TIMESTAMP_STRUCT expRecoverendDate;	// [In]  경험치 복구 종료시간
};

struct CHARACTER_SKILL_RESET
{
	unsigned long	mCharacterIdx;		// [In] 캐릭터 고유번호
	char			mRace;				// [In]	종족
	unsigned char	mJobStep;			// [In] 단계
	long			mJob;				// [In] 직업
	long			mInventoryIdx;		// [In] 인벤토리 고유번호
	long			mRetvalue;			// [Out] 반환값

	TB_INVENTORY	mItem;				// [Out] TB_INVENTORY 레코드

	long				mRowCount;		// [Out]	영향받은 행 수
	TB_CHARACTER_SKILL	mTable[1];		// [Out] TB_CHARACTER_SKILL
};

struct CHARACTER_THEME_RESET
{
	unsigned long	mCharacterIdx;		// [In] 캐릭터 고유번호
	unsigned short	mThemeIdx;			// [In]	테마 종류
	unsigned char	mThemeMode;			// [In] 테마 모드
	long			mInventoryIdx;		// [In] 인벤토리 고유번호
	unsigned short	mSlotNum;			// 슬롯번호
	long			mRetvalue;			// [Out] 반환값

	TB_INVENTORY	mItem;				// [Out] TB_INVENTORY 레코드

	long			mRowCount;			// [Out] 영향받은 행 수
	THEME_USER		mTable[1];			// [Out] TB_THEME_USER
};

struct CHARACTER_GM_THEME_RESET
{
	unsigned long	mGMCharacterIdx;	// [In] GM 캐릭터 고유번호
	unsigned long	mCharacterIdx;		// [In] 초기화할 캐릭터 고유번호
	unsigned short	mThemeIdx;			// [In]	테마 종류
	unsigned char	mThemeMode;			// [In] 테마 모드
	
	long			mRetvalue;			// [Out] 반환값
	long			mRowCount;			// [Out] 영향받은 행 수
	THEME_USER		mTable[1];			// [Out] TB_THEME_USER
};

struct CHARACTER_MONEY
{
	// DB 처리를 위한 데이터.
	long          idx;					// [In]  캐릭터 고유번호
	__int64       money;				// [In]  변경금액 (부호포함)

	// LOG 처리를 위한 데이터.
	long          objectType;			// [In]  Object 종    류 - 출처
	long          objectIndex;			// [In]  Object 인 덱 스 - 출처
	long          objectRaceGender;     // [In]  Object Detail   - 출처
	unsigned long befor;				// [In]  변경전
	unsigned long after;				// [In]  변경후

	// DB 처리후 결과 데이터.
	long          retvalue;				// [Out] 반환값
};

struct CHARACTER_DEPOSIT
{
	// DB 처리를 위한 데이터.
	long          idx;					// [In]  캐릭터 고유번호
	__int64       money;				// [In]  변경금액 (부호포함)

	// LOG 처리를 위한 데이터.
	long          objectType;			// [In]  Object 종    류 - 출처
	long          objectIndex;			// [In]  Object 인 덱 스 - 출처
	long          objectRaceGender;     // [In]  Object Detail   - 출처
	unsigned long befor;				// [In]  변경전
	unsigned long after;				// [In]  변경후

	// DB 처리후 결과 데이터.
	long          retvalue;				// [Out] 반환값
};

struct CHARACTER_TAROT_POINT
{
	long idx;							// [In]  캐릭터 고유번호
	long point;							// [In]  변경포인트 (부호포함)
};

struct CHARACTER_ACTIVE_WEAPON
{
	long idx;							// [In]  캐릭터 고유번호
	char activeWeapon;					// [In]  캐릭터 활성 무기정보 (0 Front, 1Back)
	long retvalue;						// [Out] 반환값
};

struct CHARACTER_OPTION
{
	long idx;							// [In]  캐릭터 고유번호
	long option1;						// [In]  캐릭터 옵션1
	long retvalue;						// [Out] 반환값
};

struct CHARACTER_FORCE
{
	unsigned long		mIdx;			// [In]  캐릭터 고유번호
	unsigned char		mForceType;		// [In]  세력
	long				mRetValue;		// [Out] 반환값
};

struct CHARACTER_BLOCK_SELECT
{
	long characterIdx;					// [In]  캐릭터 고유번호
	int  rowCount;						// [In]  적용행수(적용여부)
	long validTrue;						// [In]  유효만기(분 단위)
};

struct CHARACTER_BLOCK_INSERT
{
	wchar_t		characterName[ 17 ];	// [In]  제재 당할 캐릭터 이름
	long		adminCharacterIdx;	    // [In]  제한을 가한 운영자 케릭터 이름
	long		validTrue;				// [In]  유효만기(분 단위)
	long		blockCharacterIdx;		// [Out] 블록 캐릭터 고유번호
	int			channelNum;				// [Out] 접속 여부
};

struct NO_CHAT_CHARACTER
{
	wchar_t			 characterName[ 17 ];	// 캐릭터 이름
	wchar_t			 adminName[ 17 ];	    // 제한을 가한 운영자 케릭터 이름
	TIMESTAMP_STRUCT registDate;			// 등록일
	TIMESTAMP_STRUCT validThru;				// 유효만기
};

struct CHARACTER_BLOCK_SELECT_LIST
{
	wchar_t				characterName[ 17 ]; // [In]  검색할 케릭터 인덱스
	long				retValue;			 // [Out] 반환값
	int					rowCount;			 // [Out] 적용행수
	NO_CHAT_CHARACTER	table[1];			 // [Out] TB_FRIEND 레코드
};

struct CHARACTER_GM_NAME
{
	long	characterIndex;					 // [In] 캐릭터 고유번호
	wchar_t characterName[17];					 // [In] 변경될 캐릭터 이름
	long	retvalue;							 // [Out] 반환값
};

struct CHARACTER_INVEN_SIZE
{
	long            idx;				// [In]  캐릭터 고유번호
	unsigned short  bag;				// [In]  인벤토리
	unsigned short  wear;				// [In]  착용카드 슬롯
	unsigned short  wareHouse;			// [In]  창고
	unsigned short  makeSkill;			// [In]  보조직업
	long            retvalue;			// [Out] 반환값
};

struct CHARACTER_TTL
{
	long idx;							// [In]  캐릭터 고유번호
	long ttl;							// [In]  Time To Alive
	DWORD	channelNum;					// [In]  Channel Num
};

struct CHARACTER_LOGIN
{
	long    idx;						// [Out] 캐릭터 고유번호
	wchar_t name[17];					// [In]  캐릭터 이름
};

struct CHARACTER_CHANNEL
{
	long    channelNum;					// [Out] 캐릭터가 접속된 채널번호 
	wchar_t name[17];					// [In]  캐릭터 이름
};

struct CHARACTER_KICK_SELECT
{
	wchar_t		characterName[ 17 ];	// [In]  제재 당할 캐릭터 이름
	long		kickCharacterIdx;		// [Out] 블록 캐릭터 고유번호
	int			channelNum;				// [Out] 접속 여부
};

/*-- 아이템 / 인벤토리
*/
enum INVENTORY_WARNING
{
	IW_ERROR_DUPLICATE_NUMBER = 0,		// 인벤토리 번호(슬롯) 중복
	IW_ERROR_ITEM_DEFINE_INDEX,			// 아이템 정의 인덱스 오류
	IW_ERROR_INVENTORY,					// 인벤토리 오류
	IW_ERROR_TUTORIAL,					// 튜토리얼 아이템 삭제
};
struct INVENTORY_INSERT
{
	long         characterIdx;			// [In]  캐릭터 고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[1];				// [In/Out] TB_INVENTORY 레코드
};

struct INVENTORY_SELECT
{
	long         characterIdx;			// [In]  캐릭터 고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_DELETE
{
	INVENTORY_WARNING	 warning;		// [In]  삭제원인
	long				 characterIdx;	// [In]  캐릭터 고유번호
	TB_INVENTORY		 table;			// [In/Out] TB_INVENTORY 레코드
};

struct INVENTORY_COOLTIME_SELECT
{
	long                  characterIdx;	// [In]  캐릭터 고유번호
	long                  rowCount;		// [In]  영향받은 행 수
	TB_INVENTORY_COOLTIME table[1];		// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_COOLTIME_INSERT
{
	long                  characterIdx;	// [In]  캐릭터 고유번호
	TB_INVENTORY_COOLTIME table[1];		// [In]  TB_INVENTORY 레코드
};

struct INVENTORY_USE
{
	TB_INVENTORY table[1];				// [In/Out] TB_INVENTORY 레코드
};

struct INVENTORY_REMOVE
{
	long  idx;							// [In]  고유번호
	short number;						// [In]  번호
};

struct INVENTORY_SWAP
{
	long  idx1;							// [In]  고유번호
	short number1;						// [In]  번호
	long  idx2;							// [In]  고유번호
	short number2;						// [In]  번호
	long  retvalue;						// [Out] 결과
};

struct INVENTORY_MOVE
{
	long         idx;					// [In]  고유번호
	short        number;				// [In]  이동할 번호
	bool         cashBox;				// [In]  수령함(캐시박스)
};

struct INVENTORY_EXCEPT
{
	long rowCount;						// [In]  영향받은 행 수
	struct {
		long  idx;						// [In]  고유번호
		short number;					// [In]  이동할 번호
	} table[1];
};

struct INVENTORY_MERGE
{
	long         idx1;					// [In]  고유번호1
	long         idx2;					// [In]  고유번호2
	short        capacity;				// [In]  최대수량

	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

#define ROW_INVENTORY_DIVIDE	2
struct INVENTORY_DIVIDE
{
	long         characterIdx;					// [In]  캐릭터 고유번호
	TB_INVENTORY table[ROW_INVENTORY_DIVIDE];	// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_MOVE2
{
	long         characterIdx;			// [In]  캐릭터 고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

#define RESULT_INVENTORY_ENHANCED_SUCCESS	0	// 성공
#define RESULT_INVENTORY_ENHANCED_DOWN		1	// 감소
#define RESULT_INVENTORY_ENHANCED_DESTROY	2	// 파괴
#define RESULT_INVENTORY_ENHANCED_KEEP		3	// 유지(변화없음)

struct INVENTORY_ENHANCED
{
	char         result;				// [In]  결과(0성공/1감소/2파괴/3없음)
	TB_INVENTORY table[3];				// [Out] TB_INVENTORY 레코드([0]강화장비 [1]강화카드 [2]강화보호카드)
};

struct INVENTORY_DISJOINT
{
	// 분해아이템
	long characterIdx;					// [In]  캐릭터 고유번호
	long characterMoney;				// [In]  캐릭터 변경금액 (부호포함)
	long retvalue;						// [In]  [Out] 결과

	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_PUT_CARD
{
	long         idx1;					// [In]  고유번호1(장비)
	BYTE         cardSlot;              // [In]  카드슬롯 번호(1~5)
	long         idx2;					// [In]  고유번호2(카드)
	long         itemDefineIndex;		// [In]  아이템 인덱스(TB_ITEM_DEFINE::INDEX [기획:분류번호])
	long         retvalue;				// [Out] 반환값
	TB_INVENTORY table[2];				// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_CHANGE
{
	// 교환아이템
	long characterIdx;					// [In]  캐릭터 고유번호
	long characterMoney;				// [In]  캐릭터 변경금액 (부호포함)
	long retvalue;						// [Out] 결과

	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

struct INVENTORY_SEAL
{
	long         characterIdx;			// [In]  TB_CHARACTER::IDX 고유번호
	TB_INVENTORY inventory;				// [In]  TB_INVENTORY 레코드
	TB_ITEM_BILL itemBill;				// [In]  TB_ITEM_BILL 레코드
	long         retvalue;				// [Out] 반환값
};

struct ITEM_BILL_SELECT
{
	long         characterIdx;			// [In]  캐릭터 고유번호

	long         rowCount;				// [Out] 영향받은 행 수
	TB_ITEM_BILL table[1];				// [Out] TB_ITEM_BILL 레코드
};

struct ITEM_BILL_REMOVE
{
	long         idx;					// [In]  고유번호
	TB_INVENTORY inventory;				// [In]  TB_INVENTORY
	long retvalue;						// [Out] 반환값
};

struct ITEM_BILL_UPDATE
{
	long characterIdx;					// [In]  TB_CHARACTER::IDX 고유번호

	long idx;							// [In]  고유번호
	long elapsed;						// [In]  경과시간(초 단위 사용)
	long inventoryIdx;					// [In]  TB_INVENTORY::IDX 고유번호

	long before;						// [Out] 변경전
	long after;							// [Out] 변경후
	long retvalue;						// [Out] 반환값
};

struct ITEM_SELL
{
	long         characterIdx;			// [In]  캐릭터 고유번호
	long         npcIdx;				// [In]  NPC    고유번호
	long         sellType;				// [In]  판매방식
	long         sellPrice;				// [In]  판매가격

	long         rowCount;				//  
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

struct ITEM_BUY
{
	long         characterIdx;			// [In]  캐릭터   고유번호
	long         npcIdx;				// [In]  NPC      고유번호
	long         buyType;				// [In]  구입방식
	long         buyPrice;				// [In]  구입가격
	BYTE         buyStore;              // [In]  구입세력

	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

struct ITEM_COLLECT
{
	long         characterIdx;			// [In]  캐릭터   고유번호
	TB_INVENTORY table[3];	            // [Out] TB_INVENTORY 레코드
	long         retvalue;				// [Out] 결과
};

struct ITEM_COLLECT_CHANGE
{
	long         characterIdx;			// [In]  캐릭터   고유번호
    short        begin;                 // [In]  등급별 인벤토리 대역(시작)
    short        end;                   // [In]  등급별 인벤토리 대역(끝)
    TB_INVENTORY changeItem;            // [In]  TB_INVENTORY 레코드(체인지아이템)
    long         retvalue;				// [Out] 결과

    long         rowCount;				// [In]  영향받은 행 수
    TB_INVENTORY table[1];	            // [Out] TB_INVENTORY 레코드(타로카드)
};

struct ITEM_GET
{
	long         characterIdx;			// [In]  캐릭터   고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[ 1 ];			// [Out] TB_INVENTORY 레코드
};

struct ITEM_PARTY_GIVE
{
	long         giveCharacterIdx;		// [In]  캐릭터 고유번호(파티장)
    long         takeCharacterIdx;		// [In]  캐릭터 고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[ 1 ];			// [Out] TB_INVENTORY 레코드
};

struct ITEM_USE
{
	long         characterIdx;			// [In]  캐 릭 터 고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[ 1 ];			// [Out] TB_INVENTORY 레코드
};


/*-- 거래
*/
struct ITEM_EXCHANGE
{
	long rowCount;
	struct {
		long         characterIdx;		// [In]     캐릭터 고유번호
		long         characterMoney;	// [In]     변경금액(부호포함)
		long         characterFrom;		// [In]     캐릭터 고유번호(대상)
		long         characterBefor;	// [In]     캐릭터 변경전 금액
		long         characterAfter;	// [In]     캐릭터 변경후 금액
		long         retvalue;			// [Out]    반환값

		long         rowCount;						// [In]     영향받은 행 수
		TB_INVENTORY table[ MAX_ITEM_EXCHANGE ];	// [In&Out] TB_INVENTORY 레코드
	} table[ 2 ];
};

/*-- 아이템 조합
*/
struct ITEM_MIX
{
	long         characterIdx;			// [In]  캐릭터   고유번호
	long         rowCount;				// [In]  영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

// 아이템 판매대행.
struct ITEM_AGENT_CHECK
{
	long characterIdx;							// [In]  캐릭터 고유번호
	long rowCount;								// [Out] 영향받은 행 수
	long nextCheck;								// [Out] 다음 검사 시간
	long retvalue;								// [Out] 반환값
};

struct ITEM_AGENT_RESULT_OWNER
{
	long             idx;						// 고유번호
	TB_INVENTORY     table;						// 인벤토리
	long             validThru;					// 유효만기
	long             price;						// 판매가격
};
struct ITEM_AGENT_SELECT_OWNER
{
	// 결과 목록
	long                    characterIdx;		// [In]  소유 캐릭터

	long                    startPage;			// [In]  시작 페이지
	long                    endPage;			// [Out] 끝   페이지

	long                    rowCount;			// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT_OWNER table[1];			// [Out] 레코드
};
struct ITEM_AGENT_INSERT
{
	long                    characterIdx;		// [In]  소유 캐릭터

	TB_INVENTORY            inventory;			// [In/Out] 인벤토리
	BYTE                    validThru;			// [In]  유효만기(기간)
	long                    nextCheck;			// [Out] 다음 검사시간
	long                    price;				// [In]  판매가격(개당)
	long                    retvalue;			// [Out] 반환값

	// 결과 목록.
	long                    startPage;			// [In]  시작 페이지
	long                    endPage;			// [Out] 끝   페이지

	long                    rowCount;			// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT_OWNER table[1];			// [Out] 레코드
};
struct ITEM_AGENT_DELETE
{
	long                    characterIdx;		// [In]  캐 릭 터 고유번호

	long                    idx;				// [In]  고유번호
	short                   inventoryNumber;	// [In]  인벤토리 번호
	long                    retvalue;			// [Out] 반환값
	TB_INVENTORY            result;				// [Out] 레코드

	// 결과 목록.
	long                    startPage;			// [In]  시작 페이지
	long                    endPage;			// [Out] 끝   페이지

	long                    rowCount;			// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT_OWNER table[1];			// [Out] 레코드
};

struct ITEM_AGENT_RESULT
{
	long             idx;						// 고유번호
	TB_INVENTORY     table;						// 인벤토리
	long             validThru;					// 유효만기
	wchar_t          characterName[ 17 ];		// 캐릭터 이름(소유)
	long             price;						// 판매가격
};
struct ITEM_AGENT_SELECT
{
	long              startPage;				// [In]  시작 페이지
	long              endPage;					// [Out] 끝   페이지
	BYTE              sort;						// [In]  정렬

	BYTE              itemDefineRareLevel;		// [In]  레어수준(255 모두)
	BYTE              itemLimitLevelB;			// [In]  제한레벨(시작)
	BYTE              itemLimitLevelE;			// [In]  제헌래밸(끝)
	BYTE              inventoryEnhanced;		// [In]  강화단계(0~15까지 / 255모두)

	long              rowCount;					// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT table[1];					// [Out] 레코드
};
struct ITEM_AGENT_SELECT_NAME
{
	long              startPage;				// [In]  시작 페이지
	long              endPage;					// [Out] 끝   페이지
	BYTE              sort;						// [In]  정렬

	wchar_t           itemDefineName[ 51 ];		// [In]  검색이름
	BYTE              itemDefineRareLevel;		// [In]  레어수준(255 모두)
	BYTE              itemLimitLevelB;			// [In]  제한레벨(시작)
	BYTE              itemLimitLevelE;			// [In]  제헌래밸(끝)
	BYTE              inventoryEnhanced;		// [In]  강화단계(0~15까지 / 255모두)

	long              rowCount;					// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT table[1];					// [Out] 레코드
};
struct ITEM_AGENT_SELECT_TYPE
{
	long              startPage;				// [In]  시작 페이지
	long              endPage;					// [Out] 끝   페이지
	BYTE              sort;						// [In]  정렬

	BYTE              itemDefineSearchType;		// [In]  종류
	short             itemDefineSearchSubType;	// [In]  세부항목
	BYTE              itemDefineRareLevel;		// [In]  레어수준(255 모두)
	BYTE              itemLimitLevelB;			// [In]  제한레벨(시작)
	BYTE              itemLimitLevelE;			// [In]  제헌래밸(끝)
	BYTE              inventoryEnhanced;		// [In]  강화단계(0~15까지 / 255모두)

	long              rowCount;					// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT table[1];					// [Out] 레코드
};
struct ITEM_AGENT_SELECT_ALL
{
	long              startPage;				// [In]  시작 페이지
	long              endPage;					// [Out] 끝   페이지
	BYTE              sort;						// [In]  정렬

	wchar_t           itemDefineName[ 51 ];		// [In]  검색이름
	BYTE              itemDefineRareLevel;		// [In]  레어수준(255 모두)
	BYTE              itemLimitLevelB;			// [In]  제한레벨(시작)
	BYTE              itemLimitLevelE;			// [In]  제헌래밸(끝)
	BYTE              inventoryEnhanced;		// [In]  강화단계(0~15까지 / 255모두)
	BYTE              itemDefineSearchType;		// [In]  종류
	short             itemDefineSearchSubType;	// [In]  세부항목

	long              rowCount;					// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT table[1];					// [Out] 레코드
};

struct ITEM_AGENT_UPDATE
{
	// 구매
	long              characterIdx;				// [In]  캐릭터 고유번호
	long              idx;						// [In]  고유번호
	short             count;					// [In]  구매수량
	long              price;					// [Out] 구매가격
	long              retvalue;					// [Out] 반환값

	long              sccIdx;					// [Out] 판매완료 캐릭터
	TB_INVENTORY      sciLog;					// [Out] 판매완료 아이템
	TB_INVENTORY      bciLog;					// [Out] 구매완료 아이템

	// 구매후 검색.
	long              startPage;				// [In]  시작 페이지
	long              endPage;					// [Out] 끝   페이지
	BYTE              sort;						// [In]  정렬

	wchar_t           itemDefineName[ 51 ];		// [In]  검색이름
	BYTE              itemDefineRareLevel;		// [In]  레어수준(255 모두)
	BYTE              itemLimitLevelB;			// [In]  제한레벨(시작)
	BYTE              itemLimitLevelE;			// [In]  제헌래밸(끝)
	BYTE              inventoryEnhanced;		// [In]  강화단계(0~15까지 / 255모두)
	BYTE              itemDefineSearchType;		// [In]  종류
	short             itemDefineSearchSubType;	// [In]  세부항목

	long              rowCount;					// [Out] 영향받은 행 수
	ITEM_AGENT_RESULT table[1];					// [Out] 레코드
};

/*-- 일본OBT 이벤트 아이템지급
*/
struct ITEM_OBT_EVENT
{
	long         characterIdx;			// [In]  캐릭터 고유번호
	BYTE		 cbtUser;				// [In]  CBT 참여유저 여부
	BYTE		 obtUser;				// [In]  OBT 신규유저 여부
	BYTE		 fourGamer;				// [In]  4gamer 유저 여부
	long         retvalue;				// [Out] 반환값

	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] 레코드
};


// TODO - 우편 구조체 정의

struct POST_CHECK
{
	long characterIdx;							// [In]  받는-캐릭터 고유번호
	long rowCount;								// [Out] 적용행 수
};

#define POST_STATUS_NONE		0			// 읽지않음
#define POST_STATUS_READ		1			// 읽음
#define POST_STATUS_DELETE		2			// 삭제
#define POST_STATUS_VALIDTHRU	3			// 유효만기

struct POST_RESULT
{
	long             idx;							// 고유번호

	TIMESTAMP_STRUCT registerDate;					// 등록일
	long             validThru;						// 유효만기
	BYTE             type;                          // 종류(0우편, 1등록취소, 2판매완료, 3구입완료)
	BYTE             status;						// 상태(0읽지않음,1읽음,2삭제,3유효만기)

	long             fromIdx;						// 캐릭터 고유번호(보낸/구매)
	wchar_t          fromName[ LEN_POST_NAME ];		// 캐릭터 이름    (보낸/구매)

	wchar_t          contenTtitle[ LEN_POST_TITLE ];		// 제목
	wchar_t          contentMessage[ MAX_POST_MESSAGE ];	// 메시지

	long             receiptPrice;					// 가격
	long             receiptSalesCharges;			// 수수료
	TB_INVENTORY     receiptInventory;				// 아이템
	short            receiptInventoryCount;			// 아이템수량

	TB_INVENTORY     inventory1;					// 인벤토리
	TB_INVENTORY     inventory2;					// "
	TB_INVENTORY     inventory3;					// "
	long             money;							// 돈
};
struct POST_SELECT
{
	long        characterIdx;				// [In]  받는-캐릭터 고유번호
	long        startPage;		            // [In]  시작 페이지
	long        endPage;		            // [Out] 끝   페이지
	long        noneRead;					// [Out] 읽지않은 우편

	long        rowCount;					// [Out] 영향받은 행 수
	POST_RESULT	table[1];					// [Out] 쪽지 내용
};
struct POST_INSERT
{
	long         toIdx;						// [In/Out] 받는-캐릭터 고유번호
	wchar_t      toName[ LEN_POST_NAME ];	// [In]  받는-캐릭터 이름

	BYTE         validThru;					// [In]  유효만기
	BYTE         type;						// [In]  종류(0우편, 1등록취소, 2판매완료, 3구입완료)

	long         fromIdx;					// [In]	 보낸/구매 캐릭터 고유번호
	wchar_t      fromName[ LEN_POST_NAME ];	// [In]  보낸/구매 캐릭터 이름

	// 우편
	wchar_t      contentTitle[ LEN_POST_TITLE ];	// [In]  제목
	wchar_t      contentMessage[ LEN_POST_MESSAGE ];// [In]  메시지

	// 판매대행
	long         receiptPrice;				// [사용안함] 가격
	long         receiptSalesCharges;		// [사용안함] 수수료
	TB_INVENTORY receiptInventory;			// [사용안함] 아이템

	// 첨부
	TB_INVENTORY inventory1;				// [In]  인벤토리
	TB_INVENTORY inventory2;				// [In]  "
	TB_INVENTORY inventory3;				// [In]  "
	long         money;						// [In]  돈

	long         serviceCharage;			// [In]  수수료

	long         retvalue;					// [Out] 반환값
};
struct POST_UPDATE_STATUS
{
	long         characterIdx;				// [In]     캐릭터 고유번호

	long         idx;						// [In]     고유번호
	BYTE         status;					// [In]     상태(0읽지않음,1읽음,2삭제,3유효만기)

	long         noneRead;					// [Out]    읽지않은 우편
};
struct POST_UPDATE
{
	long         characterIdx;				// [In]     캐릭터 고유번호

	long         idx;						// [In]     고유번호
	short        inventoryNumber1;			// [In]     첨부1-인벤토리 번호(슬롯)
	short        inventoryNumber2;			// [In]     첨부2-"
	short        inventoryNumber3;			// [In]     첨부3-"
	long         money;						// [In/Out] 첨부4-돈
	long         retvalue;					// [Out]    반환값

	long         rowCount;					// [Out]    영향받은 행 수
	TB_INVENTORY table[1];					// [Out]    TB_INVENTORY 레코드
};
struct POST_DELETE
{
	long        characterIdx;				// [In]  캐릭터 고유번호

	long        idx[ MAX_POST_DELETE ];		// [In]  고유번호
	long        retvalue;					// [Out] 반환값

	long        startPage;		            // [In]  시작 페이지
	long        endPage;		            // [Out] 끝   페이지
	long        noneRead;					// [Out] 읽지않은 우편

	long        rowCount;					// [Out] 영향받은 행 수
	POST_RESULT	table[1];					// [Out] 쪽지 내용
};

struct FRIEND_SELECT
{
	long      characterIdx;				// [In]  캐릭터 고유번호
	long      rowCount;					// [Out] 영향받은 행 수
	TB_FRIEND table[1];					// [Out] TB_FRIEND 레코드
};

struct FRIEND_INSERT
{
	long    characterIdx1;				// [In]  캐릭터(고유번호)
	long    characterIdx2;				// [Out] 캐릭터(고유번호)
	wchar_t characterName[ 17 ];		// [In]  캐릭터 이름(친구)
	long    retvalue;					// [Out] 결과
};

struct FRIEND_UPDATE
{
	long    idx;						// [In]  고유번호
	long    characterIdx1;				// [In]  캐릭터 고유번호
	long    characterIdx2;				// [Out] 캐릭터(고유번호)
};

struct FRIEND_DELETE
{
	long    idx;						// [In]  고유번호
	long    characterIdx1;				// [In]  캐릭터 고유번호
	long    characterIdx2;				// [Out] 캐릭터(고유번호)
};

/*-- Etc
*/
struct ITEM_SPREAD_VALUE
{
	long  itemDefineIndex;              // [In]  TB_ITEM_DEFINE::Index-아 이 템 인덱스
	long  spreadIndex;					// [In]  TB_ITEM_SPREAD::Index-스프레드 인덱스
	long  spreadValue;					// [Out] 스트레드 적용후 타로카드 값
	struct
	{
		long  itemDefineIndex;			// [In]  타로카드 인덱스
		BYTE  dir;						// [In]  타로카드 방향
		long  tarot;					// [In]  타로카드 값
		long  spread;					// [In]  스프레드 값
	} values[ 10 ];

};

struct ITEM_TAROT_RESULT
{
	long itemDefineIndex;				// [In]  TB_ITEM_DEFINE::Index 아이템 인덱스
	long value;							// [In]  입력값(타로결과값)

	long influence_idx1;				// [Out] 스킬효과[스킬인덱스참조]
	long influence_idx2;				// [Out] 스킬효과[스킬인덱스참조]
	long influence_idx3;				// [Out] 스킬효과[스킬인덱스참조]
};

// 스킬
struct SKILL_SELECT
{
	long				mCharacterIdx;	// [In]	캐릭터 고유번호
	long				mRowCount;		// [Out]	영향받은 행 수
	TB_CHARACTER_SKILL	mTable[1];		// [Out]	TB_CHARACTER_SKILL
};

struct SKILL_INSERT
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	unsigned long		mSkillClassIdx;	// [In]	스킬 종류번호
	//unsigned long		mMoney;			// [In]	소모금액
	unsigned short		mSP;			// [In]	소모SP
	unsigned char		skillLevel;		// [In] 스킬 레벨
	unsigned long		skillPoint;		// [In] 스킬 포인트
	long				mRetvalue;		// [Out]	결과
};

struct SKILL_CHEAT_INSERT
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	unsigned long		mSkillClassIdx;	// [In]	스킬 종류번호
	unsigned char		mSkillStep;		// [In]	스킬 단계
	long				mRetvalue;		// [Out]	결과
};

struct SKILL_UPDATE
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	unsigned long		mSkillClassIdx;	// [In]	스킬 종류번호
	unsigned char		mSkillStep;		// [In]	스킬 단계
	/*unsigned long		mMoney;			// [In]	소모금액*/
	unsigned short		mSP;			// [In]	소모SP
	unsigned char		skillLevel;		// [In] 스킬 레벨
	unsigned long		skillPoint;		// [In] 스킬 포인트
	long				mRetvalue;		// [Out]	결과
};

struct SKILL_CHEAT_UPDATE
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	unsigned long		mSkillClassIdx;	// [In]	스킬 종류번호
	unsigned char		mSkillStep;		// [In]	스킬 단계
	long				mRetvalue;		// [Out]	결과
};

struct SKILL_COOLTIME
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	long				mRowCount;		// [Out]	입력 행 수
	long				mRetvalue;		// [Out]	결과
	TB_CHARACTER_SKILL	mTable[1];		// [Out]	TB_CHARACTER_SKILL
};


// 효과
struct SKILL_INFLUENCE_SELECT
{
	long				mCharacterIdx;	// [In]	캐릭터 고유번호
	long				mRowCount;		// [Out]	영향받은 행 수
	TB_CHARACTER_INFLUENCE	mTable[1];	// [Out]	TB_CHARACTER_INFLUENCE
};

struct SKILL_INFLUENCE_UPDATE
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	bool				mCycleSave;		// [In] 저장형태
	long				mRowCount;		// [Out]	입력 행 수
	long				mRetvalue;		// [Out]	결과
	TB_CHARACTER_INFLUENCE	mTable[1];	// [In]	TB_CHARACTER_INFLUENCE
};

struct RECIPE_COOLTIME
{
	unsigned long		mCharacterIdx;	// [In]	캐릭터 고유번호
	long				mRowCount;		// [Out]	입력 행 수
	long				mRetvalue;		// [Out]	결과
	TB_RECIPE_COOLTIME	mTable[1];		// [Out]	TB_CHARACTER_SKILL
};

// 판매노점
struct STALL_SELL_GET
{
	sObject      fObject;			// [In]  캐 릭 터 고유번호
	long         fCharacterMoney;	// [In]  캐 릭 터 변경금액 (부호포함)
	TB_INVENTORY fInventory;		// [Out] TB_INVENTORY 레코드

	sObject      tObject;			// [In]  캐 릭 터 고유번호
	long         tCharacterMoney;	// [In]  캐 릭 터 변경금액 (부호포함)
	TB_INVENTORY tInventory;		// [Out] TB_INVENTORY 레코드

	long         retvalue;				// [Out] 반환값
};

/*-- 큇슬롯
*/
struct SHORTCUT_SELECT
{
	long        characterIdx;			// [In]  캐릭터 고유번호
	long		rowCount;				// [Out] select 카운트
	TB_SHORTCUT table;					// [In]  TB_SHORTCUT
};

struct SHORTCUT_UPDATE
{
	long        characterIdx;			// [In]  캐릭터 고유번호
	TB_SHORTCUT table;					// [In]  TB_SHORTCUT
};

/*-- 퀘스트
*/
struct QUEST_INSERT
{
	unsigned long   characterIdx;				// [In]  캐릭터 고유번호
	unsigned long	questIdx;					//
	__int64			check;						// [In]  정보 업뎃
	char			timeType;					// [In]  퀘스트 시간 타입
	unsigned long	restTime;					// [In]  퀘스트 남은시간
	long			takemoney;					// [In]  퀘스트 지불금액
	long			firepoint;					// [In]  전장 포인트 지불
	long			waterpoint;					// [In]  전장 포인트 지불
	long			windpoint;					// [In]  전장 포인트 지불
	long			earthpoint;					// [In]  전장 포인트 지불
	long			tutorialindex;				// [In]  튜토리얼 모드 인덱스
	long			retvalue;					// [Out] 결과

	TB_QUEST_PROGRESS quest;			// [Out] TB_QUEST_PROGRESS 레코드

	long         rowCount;				// [In/Out] 영향받은 행 수
	TB_INVENTORY inventory[1];			// [In/Out] TB_INVENTORY 레코드
};

struct QUEST_UPDATE
{
	unsigned long	characterIdx;				// [In]  캐릭터 고유번호
	long			rowCount;
	TB_QUEST_PROGRESS table[1];					// [In]  TB_QUEST_PROGRESS 레코드
};

struct QUEST_SELECT
{
	unsigned long	characterIdx;				// [In]  캐릭터 고유번호
	long			rowCount;
	TB_QUEST_PROGRESS table[1];
};

struct QUEST_DELETE
{
	unsigned long   idx;						// [In]  db 고유인덱스
	unsigned int 	arrIdx;						// [In]  배열인덱스
	unsigned long	characterIdx;				// [In]  캐릭터 고유번호
	long	retvalue;					// [Out] 결과
	
	// 회수 [아이템]
	long         rowCount;				// [In]     영향받은 행 수
	TB_INVENTORY table[1];				// [In/Out] TB_INVENTORY 레코드
};

// 퀘스트 진행 완료 요청
struct QUEST_COMPLETE
{
	unsigned long idx;					// [In]  db 고유인덱스
	unsigned int  arrIdx;				// [In]  배열인덱스
	unsigned long characterIdx;			// [In]  캐릭터 고유번호
	char repeatType;					// [In]  반복타입
	char status;						// [Out] 상태값
	long retvalue;						// [Out] 결과

	/// 업데이트할 정보
	__int64 check;						// [In]  정보 업뎃

	unsigned long tutorialIndex;		// [In]	 튜토리얼 모드 변경 인덱스
	
	// 회수 [아이템]
	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] TB_INVENTORY 레코드
};

// 보상 및 완료 요청
struct QUEST_REWARD
{
	unsigned long idx;							// [In&out] db 고유인덱스
	unsigned int  arrIdx;						// [In]     배열인덱스
	unsigned long characterIdx;					// [In]     캐릭터 고유번호
	
	///
	TIMESTAMP_STRUCT thru;				// [In]  만기일(시간지정퀘스트)

	// 보상 [버  프]
	long bufRowCount;					// [In]     영향받은 행 수
	long bufIndex[ 3 ];					// [In]     버프인덱스

	// 보상 [금  액]
	long characterMoney;				// [In]     캐릭터 변경금액 (부호포함)

	// 보상 [전  직]
	long jobIdx;						// [In]     전직인덱스
	long jobStep;						// [In]		직업단계
	char race;							// [In]		종족
    
	// 보상 스킬
	unsigned int skillPoint;			// [In]		스킬 포인트

	// 보상 [세  력]
	unsigned char forceType;			// [In]     세력 타입

	// 보상 [호  칭]
	long titleIdx;						// [In]		호칭 인덱스
	
	// 보상 [아이템]
	long         itemRowCount;			// [In/Out] 영향받은 행 수
	TB_INVENTORY table[5];				// [In/Out] TB_INVENTORY 레코드
	
	char skillRowCount;					// [Out] 영향받은 행 수
	unsigned long skillClassIdx[1];		// [Out] 전직시 자동습득된 스킬목록
};

// 완전히 종료한 퀘스트 목록받기
struct QUEST_ENDLIST
{
	unsigned long	characterIdx;				// [In]  캐릭터 고유번호
	long			rowCount;
	unsigned long	questIdx[1];				// [Out] 결과	
};

struct QUEST_VALIDLIST
{
	unsigned long	characterIdx;				// [In]  캐릭터 고유번호
	long			rowCount;

	struct {
		unsigned long		questIdx;	// [Out] 퀘스트인덱스	
		TIMESTAMP_STRUCT	valid;		// [Out] 만기일
	} table[1];
};

// 퀘스트 제거
struct QUEST_REMOVE
{
	unsigned long idx;							// [In]  고유번호
};

/// 호칭 삽입
struct TITLE_INSERT
{
	unsigned long	characterIdx;				// [In] 캐릭터 인덱스
	unsigned long	titleIdx;					// [In] 호칭 인덱스
	long	retvalue;					// [Out] 결과
};

/// 호칭 리스트 받기
struct TITLE_SELECT
{
	unsigned long	characterIdx;				// [In] 캐릭터 인덱스
	long		rowCount;					// [Out] 영향받은 행수 
	TB_TITLE	table[1];				// [Out] TB_TITLE 레코드
};

struct VERIFY_GUILD
{
	long		 count;					// 실제로 존재하는 길드수
	long		 total;					// 전체 길드수
	long         retvalue;			
};

/// 분할 select
struct GUILD_SELECT
{
	long		 idx;					// [In] 고유번호 (길드 인덱스)
	bool		 restore;				// 길드 정보 복구용
	TB_GUILD	 table[1];				// [Out] 길드 테이블
};

// 길드 유저 정보
struct GUILDUSER_SELECT
{
	unsigned long	idx;				// [In]  고유번호 (길드 인덱스)
	bool		    restore;			// 길드 정보 복구용
	long			rowCount;			// [Out] 적용된 행의 수
	TB_GUILD_USER	table[1];			// [Out] 길드 유저 정보
};

// 길드 생성
struct GUILD_CREATE
{
	unsigned long	idx;						// [Out] 고유번호 (길드 인덱스)
	unsigned long	characterIdx;				// [In] 캐릭터 인덱스
	wchar_t			name[13];					// [In] 길드명
	wchar_t			masterName[17];				// [In] 길드장 명
	long			money;						// [In] 길드생성 금액
	long			retvalue;					// [Out] 결과	
};

// 길드원 추가
struct GUILDUSER_ADD
{
	unsigned long	idx;						// [In] 고유번호 (길드 인덱스)
	unsigned long	charaterIdx;				// [In] 캐릭터 인덱스
	long	retvalue;					// [Out] 결과	
};

// 길드원 삭제
struct GUILDUSER_OUT
{
	unsigned long	idx;						// [In] 고유번호 (길드 인덱스)
	unsigned long	charaterIdx;				// [In] 캐릭터 인덱스
	long	retvalue;					// [Out] 결과	
};

// 길드 해체
struct GUILD_DELETE
{
	unsigned long	idx;						// [In] 고유번호 (길드 인덱스)
	unsigned long	charaterIdx;				// [In] 캐릭터 인덱스
	long	retvalue;					// [Out] 결과	
};

// 길드정보 갱신
struct GUILD_UPDATE
{
	unsigned long	idx;						// [In] 고유번호 (길드 인덱스)
	wchar_t notice[41];					// [In] 길드 공지
	long	retvalue;					// [Out] 결과	
};

// 길드유저 직위 부여
struct GUILDUSER_GIVEPOSITION
{
	unsigned long	idx;						// [In] 고유번호 (길드 인덱스)	
	unsigned long	characterIdx;				// [In] 캐릭터 인덱스
	char	position;					// [In] 변경할 직위
	long	retvalue;					// [Out] 결과	
};

// 길드마크 갱신
struct GUILDMARK_UPDATE
{
	unsigned long	 idx;						// [In] 길드인덱스
	TIMESTAMP_STRUCT markRegistDate;			// [In] 길드마크 등록일
	TB_GUILDMARK	 mark;						// [In] 길드마크 24 * 24 * (32/8)
	long			 retvalue;					// [Out] 결과
};

// 제조스킬
struct MAKESKILL_INSERT
{
	unsigned long	mCharacterIdx;		// [In] 캐릭터 고유번호
	bool			mIsFirstSkill;		// [In] 2개 직업중 1,2번 위치
	unsigned char	mMakeSkillIdx;		// [In] 전문 직업 스킬 idx	
	unsigned char	mMakeSkillStep;		// [In] 전문 직업 스킬 단계	
	long			mInventoryIdx;		// [In] 인벤토리 고유번호
	long			mRetvalue;			// [Out] 결과
	TB_INVENTORY	mInventory;			// [Out] TB_INVENTORY 레코드
};

struct MAKESKILL_SELECT
{
	unsigned long	mCharacterIdx;		// [In]		캐릭터 고유번호
	long			mRowCount;			// [Out]	영향받은 행 수
	TB_MAKESKILL	mTable[1];			// [Out]	TB_MAKESKILL
};

struct MAKESKILL_DELETE
{
	unsigned long	mCharacterIdx;		// [In]		캐릭터 고유번호
	unsigned char	mMakeSkillIdx;		// [In]		전문직업
	long			mRetvalue;			// [Out]	결과	
};

struct RECIPE_INSERT
{
	unsigned long	mCharacterIdx;		// [In] 캐릭터 고유번호
	unsigned char	mMakeSkillIdx;		// [In] 전문 직업 스킬 idx	
	unsigned long	mRecipeIdx;			// [In] 레시피 idx
	long			mInventoryIdx;		// [In] 인벤토리 고유번호
	long			retvalue;			// [Out] 결과
	TB_INVENTORY	mInventory;			// [Out] TB_INVENTORY 레코드
};

struct RECIPE_GROUP_INSERT
{
	unsigned long	mCharacterIdx;		// [In] 캐릭터 고유번호
	unsigned char	mMakeSkillIdx;		// [In] 전문 직업 스킬 idx	
	long			mInventoryIdx;		// [In] 인벤토리 고유번호
	TB_INVENTORY	mInventory;			// [Out] TB_INVENTORY 레코드
	long			mRetvalue;			// [Out] 결과
	unsigned long	mRowCnt;			// [in] 레시피 입력 갯수
	sRecipeGroupInfo mRecipeGroup[1];	// [In] 레시피 idx

	unsigned long GetMsgLength() { return sizeof(RECIPE_GROUP_INSERT) - sizeof(mRecipeGroup) + (mRowCnt * sizeof(sRecipeGroupInfo));} 
};

/// 운세 속성
struct FORTUNE_SELECT
{
	long				characterIdx;				// [In] 캐릭터 인덱스
	TIMESTAMP_STRUCT	thru[5];					// [Out] 결과	
	wchar_t				word[13];					// [Out] 오늘의 한마디
	unsigned long		color;						// [Out] 오늘의 한마디 컬러
};

struct FORTUNE_UPDATE
{
	long				characterIdx;				// [In] 캐릭터 인덱스
	TIMESTAMP_STRUCT	thru[5];					// [In] 시간정보	
	wchar_t				word[13];					// [In] 오늘의 한마디
	unsigned long		color;						// [In] 오늘의 한마디 컬러
	long				retvalue;					// [Out] 결과
};

struct FORCE_POINT_UPDATE
{
	unsigned long		mCharacterIdx;				// [In] 캐릭터 인덱스
	unsigned long		mFirePoint;					// [In] 속성 포인트 / 친밀도
	unsigned long		mFireFriendly;
	unsigned long		mWaterPoint;
	unsigned long		mWaterFriendly;
	unsigned long		mWindPoint;
	unsigned long		mWindFriendly;
	unsigned long		mEarthPoint;
	unsigned long		mEarthFriendly;
	long				mRetValue;
};

//
struct PVP_USER_INSERT
{
	short pvpIdx;									// [In]  PVP 인덱스
	short characterChannel;							// [In]  캐릭터 채널
	long  characterIdx;								// [In]  캐릭터 고유번호
	char  characterLevel;
	long  retvalue;									// [Out] 반환값
};
struct PVP_USER
{
	short characterChannel;
	long  characterIdx;
	BYTE  characterLevel;
	BYTE  characterForceType;
};
struct PVP_USER_SELECT
{
	short    pvpIdx;
	unsigned long aryPos;
	char     minLevel;
	char     maxLevel;

	long     rowCount;								// [Out] 적용된 행 수
	PVP_USER table[1];								// [Out] 테이블
};
struct PVP_USER_REMOVE
{
	short pvpIdx;									// [In]  PVP 인덱스
	long  characterIdx;								// [In]  캐릭터 고유번호
	long  retvalue;									// [Out] 반환값
};
struct PVP_USER_CLEAR
{
	short pvpIdx;									// [In]  PVP 인덱스
	char minLevel;
	char maxLevel;
	long  retvalue;									// [Out] 반환값
};

// Party
#define MAX_PARTY_USER	6

struct PARTY_INSERT
{
	long           partyIdx;						// [In]  TB_PARTY::IDX
	sPartyUserData leader;							// [In]  leader
	sPartyUserData player;							// [In]  player
};

struct ITEM_SUMMON_POST
{
    long            characterIdx;                   // [In]     캐릭터 고유번호
    TB_INVENTORY    inventory;                      // [In&Out] 인벤토리

    // 우편
	long            startPage;                      // [In]  시작 페이지
	long            endPage;                        // [Out] 끝   페이지
	long            noneRead;   		            // [Out] 읽지않은 우편

	long            postCount;  		            // [Out] 영향받은 행 수
	POST_RESULT	    post[1];			            // [Out] 쪽지 내용
};

struct ITEM_SUMMON_AGENT
{
    long                    characterIdx;           // [In]     캐릭터 고유번호
    TB_INVENTORY            inventory;              // [In&Out] 인벤토리

	// 판매 대행
	long                    startPage;			    // [In]  시작 페이지
	long                    endPage;			    // [Out] 끝   페이지

	long                    rowCount;			    // [Out] 영향받은 행 수
	ITEM_AGENT_RESULT_OWNER agent[1];			    // [Out] 레코드
};

struct ITEM_SUMMON_WAREHOUSE
{
    long            characterIdx;                   // [In]     캐릭터 고유번호
    TB_INVENTORY    inventory;                      // [In&Out] 인벤토리
};

struct ITEM_NPC_EVENT
{
	long         userIdx;				// [In]  사용자 인덱스
	long         characterIdx;			// [In]  캐릭터 고유번호
	long		 eventType;				// [In]	 이벤트 타입
	long         retvalue;				// [Out] 반환값
	long         rowCount;				// [Out] 영향받은 행 수
	TB_INVENTORY table[1];				// [Out] 레코드
};

// ETC
struct WHISPER_SELECT : CHARACTER_LOGIN
{
	wchar_t message[128];							// [In]  메시지
	short	slotIndex;								// [In]  슬롯번호
};

// GM
struct GM_JOB_UPDATE
{
	long characterIdx;								// [In] 케릭터 고유 번호
	
	long jobIndex;									// [In] 전직할 직업 인덱스
	long jobStep;									// [In]	직업단계
	char race;										// [In]	종족

	char skillRowCount;								// [Out] 영향받은 행 수
	unsigned long skillClassIdx[1];					// [Out] 전직시 자동습득된 스킬목록
};

struct GM_INFLUENCE_LIST
{
	long			 index;
	long			 infIndex;
	TIMESTAMP_STRUCT startTime;
	TIMESTAMP_STRUCT validThru;
};

struct VERIFY_GM_EVENT
{
	long		 rowcount;

	GM_INFLUENCE_LIST table[1];						// [Out] 유효한 이벤트 목록
};


struct GM_EVENT_INSERT
{
	long			 index;							// [Out] DB Index
	long			 influenceIndex;				// [In] 효과 인덱스
	TIMESTAMP_STRUCT startTime;						// [In] 시작시간
	TIMESTAMP_STRUCT validThru;						// [In] 만기시간

	long			 retvalue;						// [Out] 반환값
};

struct GM_EVENT_UPDATE
{
	long			 index;							// [In] DB Index
	long			 influenceIndex;				// [Out] 삭제된 효과값
	long			 retvalue;						// [Out] 반환값
};

struct ITEM_DROPLIMIT_TABLE
{
	long				mCount;
	TB_ITEM_DROPLIMIT	mTable[1];
};

struct ITEM_DROPLIMIT_UPDATE
{
	unsigned long		mItemIdx;
	TIMESTAMP_STRUCT	mEndTime;
	long				mRetValue;
};

class cSQLGameStmt :
	public cSQLStatement
{
private:
	SQLRETURN SQLClose                 ( SQLRETURN sqlReturn, SQLHSTMT sqlHStmt );
	void      SQLClose                 ( SQLHSTMT sqlHStmt );

	void      SQLGetFullInv            ( SQLHSTMT sqlHStmt, TB_INVENTORY* table );
	void      SQLGetBaseInv            ( SQLHSTMT sqlHStmt, TB_INVENTORY* table );

public:
// 클래스 생성 및 공용 메소드.
	cSQLGameStmt(void);

	/*-- Init & Shutdown Game Database --*/
	SQLRETURN Init                     ( INIT_GAME* initGame );
	SQLRETURN Shutdown                 ( SHUTDOWN_GAME* shutdownGame );

	/*-- Theme Database --*/
	SQLRETURN ThemeCreate              ( THEME_CREATE* create );
	SQLRETURN ThemeRoomCheck           ( THEME_ROOM_CHECK* themeRoomCheck );
	SQLRETURN ThemeUserCheck           ( THEME_USER_CHECK* themeUserCheck );
	SQLRETURN ThemeUserInsert          ( THEME_USER_INSERT* themeUserInsert );
	SQLRETURN ThemeUserSelect          ( THEME_USER_SELECT* themeUserSelect, u_long& cbLength );
	SQLRETURN ThemeUserUpdate          ( THEME_USER_UPDATE* themeUserUpdate );
	SQLRETURN ThemeUserRemove          ( THEME_USER_REMOVE* themeUserRemove );
	SQLRETURN ThemeUserSave            ( THEME_USER_SAVE* themeUserSave );
	SQLRETURN ThemePartySelect         ( THEME_PARTYINFO_SELECT* pPartySelect, u_long& cbLength );
	SQLRETURN ThemePartyRoomSelect     ( THEME_PARTYROOM_SELECT* pPartySelect );
	SQLRETURN ThemeGMUserSelect		   ( THEME_GM_USER_SELECT* themeUserSelect, u_long& cbLength );

	/*-- Item Databasae --*/
	SQLRETURN TruncateItemDefine           ( void );
	SQLRETURN TruncateItemAbility          ( void );
	SQLRETURN TruncateItemLimit            ( void );
	SQLRETURN TruncateItemCard             ( void );
	SQLRETURN TruncateItemCardSlot         ( void );
	SQLRETURN TruncateItemTarot            ( void );
	SQLRETURN TruncateItemTarotResult      ( void );
	SQLRETURN TruncateItemSpread           ( void );
	SQLRETURN TruncateItemDisjoint         ( void );
	SQLRETURN TruncateItemEnhanced         ( void );
	SQLRETURN TruncateItemEnhancedValue    ( void );
	SQLRETURN TruncateItemEnhancedRate     ( void );
	SQLRETURN TruncateItemEnhancedSuccess  ( void );
	SQLRETURN TruncateItemEnhancedDown     ( void );
	SQLRETURN TruncateItemEnhancedDestruct ( void );
	SQLRETURN TruncateItemChange           ( void );
	SQLRETURN TruncateItemChangeItems      ( void );
	SQLRETURN TruncateDefaultItems         ( void );

	SQLRETURN InsertItemDefine         ( TB_ITEM_DEFINE* tbItemDefine, long& retvalue );
	SQLRETURN InsertItemAbility        ( TB_ITEM_ABILITY* tbItemAbility, long& retvalue );
	SQLRETURN InsertItemLimit          ( TB_ITEM_LIMIT* tbItemLimit, long& retvalue );
	SQLRETURN InsertItemCard           ( TB_ITEM_CARD* tbItemCard, long& retvalue );
	SQLRETURN InsertItemCardSlot       ( TB_ITEM_CARD_SLOT* tbItemCardSlot, long& retvalue );
	SQLRETURN InsertItemTarot          ( TB_ITEM_TAROT* tbItemTarot, long& retvalue );
	SQLRETURN InsertItemTarotResult    ( TB_ITEM_TAROT_RESULT* tbItemTarotResult, long& retvalue );
	SQLRETURN InsertItemSpread         ( TB_ITEM_SPREAD* tbItemSpread, long& retvalue );
	SQLRETURN InsertItemDisjoint       ( TB_ITEM_DISJOINT* tbItemDisjoint, long& retvalue );
	SQLRETURN InsertItemEnhanced       ( TB_ITEM_ENHANCED* tbItemEnhanced );
	SQLRETURN InsertItemEnhancedRate   ( TB_ITEM_ENHANCED_RATE* tbItemEnhancedRate );
	SQLRETURN InsertItemChange         ( TB_ITEM_CHANGE* tbItemChange );
	SQLRETURN InsertDefaultItems       ( TB_DEFAULT_ITEMS* tbDefaultItems, long& retvalue );

	SQLRETURN VerifyItemDefine         ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemAbility        ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemCooltime2      ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemLimit          ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemCard           ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemCardSlot       ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemTarot          ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemEnhanced       ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemEnhancedRate   ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemDisjoint       ( VERIFY_ITEM_TABLE* verifyItemTable );
	SQLRETURN VerifyItemChange         ( VERIFY_ITEM_TABLE* verifyItemTable );

	SQLRETURN ItemDefine               ( ITEM_DEFINE_TABLE* itemDefineTable );
	SQLRETURN ItemAbility              ( ITEM_ABILITY_TABLE* itemAbilityTable );
	SQLRETURN ItemCooltime2            ( ITEM_COOLTIME2_TABLE* itemCooltime2Table, u_long& cbLength );
	SQLRETURN ItemLimit                ( ITEM_LIMIT_TABLE* itemLimitTable );
	SQLRETURN ItemCard                 ( ITEM_CARD_TABLE* itemCardTable );
	SQLRETURN ItemCardSlot             ( ITEM_CARD_SLOT_TABLE* itemCardSlotTable );
	SQLRETURN ItemTarot                ( ITEM_TAROT_TABLE* itemTarotTable );
	SQLRETURN ItemEnhanced             ( ITEM_ENHANCED_TABLE* itemEnhancedTable );
	SQLRETURN ItemEnhancedRate         ( ITEM_ENHANCED_RATE_TABLE* itemEnhancedRateTable );
	SQLRETURN ItemDisjoint             ( ITEM_DISJOINT_TABLE* itemDisjointTable );
	SQLRETURN ItemChange               ( ITEM_CHANGE_TABLE* itemChangeTable );

	SQLRETURN VerifyDefaultItems       ( VERIFY_DEFAULT_ITEMS* verifyDefaultItems );

	// Character
	SQLRETURN CharacterList            ( CHARACTER_LIST* characterList, u_long& cbLength );
	SQLRETURN CharacterName            ( CHARACTER_NAME* characterName );
	SQLRETURN CharacterInsert          ( CHARACTER_INSERT* characterInsert, u_long& cbLength );
	SQLRETURN CharacterDeleteWait      ( CHARACTER_DELETE_WAIT* characterDelWait );
	SQLRETURN CharacterDeleteCancel    ( CHARACTER_DELETE_CANCEL* characterDelCancel );
	SQLRETURN CharacterDelete          ( CHARACTER_DELETE* characterDelete );
	SQLRETURN CharacterUpdate          ( CHARACTER_UPDATE* characterUpdate );
	SQLRETURN CharacterSelect          ( CHARACTER_SELECT* characterSelect );
	SQLRETURN CharacterMoney           ( CHARACTER_MONEY* characterMoney );
	SQLRETURN CharacterDeposit         ( CHARACTER_DEPOSIT* characterDeposit );
	SQLRETURN CharacterTarotPoint      ( CHARACTER_TAROT_POINT* characterTarotPoint );
	SQLRETURN CharacterActiveWeapon    ( CHARACTER_ACTIVE_WEAPON* characterActiveWeapon );
	SQLRETURN CharacterOption          ( CHARACTER_OPTION* characterOption );
	SQLRETURN CharacterForce           ( CHARACTER_FORCE* characterForce );
	SQLRETURN CharacterSkillReset      ( CHARACTER_SKILL_RESET* reset, u_long& cbLength );
	SQLRETURN CharacterThemeReset      ( CHARACTER_THEME_RESET* reset, u_long& cbLength );
	SQLRETURN CharacterBlockSelect     ( CHARACTER_BLOCK_SELECT* characterBlockSelect );
	SQLRETURN CharacterBlockSelectList ( CHARACTER_BLOCK_SELECT_LIST* characterBlockSelectList, u_long& cbLength );
	SQLRETURN CharacterBlockInsert     ( CHARACTER_BLOCK_INSERT* characterBlockInsert );
	SQLRETURN CharacterGmName          ( CHARACTER_GM_NAME* characterGmName );
    SQLRETURN CharacterInvenSize       ( CHARACTER_INVEN_SIZE* invenSize );
	SQLRETURN CharacterTTL             ( CHARACTER_TTL* ttl );
	SQLRETURN CharacterChannelSelect   ( CHARACTER_CHANNEL* characterChannel );
	SQLRETURN CharacterGMThemeReset	   ( CHARACTER_GM_THEME_RESET* reset, u_long& cbLength );
	SQLRETURN CharacterGMKickSelect	   ( CHARACTER_KICK_SELECT* characterKickSelect );

	// Inventory&Item
	SQLRETURN InventorySelect          ( TB_INVENTORY* inventory );
	SQLRETURN InventoryInsert          ( INVENTORY_INSERT* inventoryInsert );
	SQLRETURN InventorySelect          ( INVENTORY_SELECT* inventorySelect, u_long& cbLength );
    SQLRETURN InventoryCashSelect      ( INVENTORY_SELECT* inventorySelect, u_long& cbLength );
	SQLRETURN InventoryDelete          ( INVENTORY_DELETE* inventoryDelete );
	SQLRETURN InventoryCooltimeSelect  ( INVENTORY_COOLTIME_SELECT* inventoryCooltimeSelect, u_long& cbLength );
	SQLRETURN InventoryCooltimeInsert  ( INVENTORY_COOLTIME_INSERT* inventoryCooltimeInsert );
	SQLRETURN InventoryRemove          ( INVENTORY_REMOVE* inventoryRemove );
	SQLRETURN InventoryUse             ( INVENTORY_USE* inventoryUse );
	SQLRETURN InventorySwap            ( INVENTORY_SWAP* inventorySwap );
	SQLRETURN InventoryMove            ( INVENTORY_MOVE* inventoryMove );
	SQLRETURN InventoryExcept          ( INVENTORY_EXCEPT* inventoryExcept );
	SQLRETURN InventoryMerge           ( INVENTORY_MERGE* inventoryMerge, u_long& cbLength );
	SQLRETURN InventoryDivide          ( INVENTORY_DIVIDE* inventoryDivide );
	SQLRETURN InventoryMove2           ( INVENTORY_MOVE2* inventoryMove2 );
	SQLRETURN InventoryEnhanced        ( INVENTORY_ENHANCED* inventoryEnhanced );
	SQLRETURN InventoryDisjoint        ( INVENTORY_DISJOINT* inventoryDisjoint );
	SQLRETURN InventoryPutCard         ( INVENTORY_PUT_CARD* inventoryPutCard );
	SQLRETURN InventoryChange          ( INVENTORY_CHANGE* inventoryChange );
	SQLRETURN InventorySeal            ( INVENTORY_SEAL* inventorySeal );
	SQLRETURN ItemBillSelect           ( ITEM_BILL_SELECT* select, u_long& cbLength );
	SQLRETURN ItemBillRemove           ( ITEM_BILL_REMOVE* remove );
	SQLRETURN ItemBillUpdate           ( ITEM_BILL_UPDATE* update );
	SQLRETURN ItemSell                 ( ITEM_SELL* itemSell );
	SQLRETURN ItemBuy                  ( ITEM_BUY* itemBuy );
	SQLRETURN ItemCollect              ( ITEM_COLLECT* itemCollect );
	SQLRETURN ItemCollectChange        ( ITEM_COLLECT_CHANGE* itemCollect );
	SQLRETURN ItemGet                  ( ITEM_GET* itemGet );
	SQLRETURN ItemPartyGive            ( ITEM_PARTY_GIVE* itemPartyGive );
	SQLRETURN ItemUse                  ( ITEM_USE* itemUse );
	SQLRETURN ItemExchange             ( ITEM_EXCHANGE* itemExchange );
	SQLRETURN ItemMix                  ( ITEM_MIX* itemMix );
	SQLRETURN ItemSummonPost           ( ITEM_SUMMON_POST*  summonPost,  u_long& cbLength );
	SQLRETURN ItemSummonAgent          ( ITEM_SUMMON_AGENT* summonAgent, u_long& cbLength );
	SQLRETURN ItemSummonWareHouse      ( ITEM_SUMMON_WAREHOUSE* summonWare );

	SQLRETURN ItemNpcEvent			   ( ITEM_NPC_EVENT* npcEvent, u_long& cbLength );


	SQLRETURN ItemObtEvent			   ( ITEM_OBT_EVENT* itemEvent, u_long& cbLength );


	// TODO - Agent&Item.
	SQLRETURN ItemAgentCheck           ( ITEM_AGENT_CHECK* itemAgentCheck );
	SQLRETURN ItemAgentSelectOwner     ( ITEM_AGENT_SELECT_OWNER* itemAgentSelectName, u_long& cbLength );
	SQLRETURN ItemAgentInsert          ( ITEM_AGENT_INSERT* itemAgentInsert, u_long& cbLength );
	SQLRETURN ItemAgentDelete          ( ITEM_AGENT_DELETE* itemAgentDelete, u_long& cbLength );
	SQLRETURN ItemAgentUpdate          ( ITEM_AGENT_UPDATE* itemAgentUpdate, u_long& cbLength );
	SQLRETURN ItemAgentSelect          ( ITEM_AGENT_SELECT* itemAgentSelect, u_long& cbLength );
	SQLRETURN ItemAgentSelectName      ( ITEM_AGENT_SELECT_NAME* itemAgentSelectName, u_long& cbLength );
	SQLRETURN ItemAgentSelectType      ( ITEM_AGENT_SELECT_TYPE* itemAgentSelectType, u_long& cbLength );
	SQLRETURN ItemAgentSelectAll       ( ITEM_AGENT_SELECT_ALL* itemAgentSelectAll, u_long& cbLength );

	// TODO - 우편 메소드 정의 - Post(우편)
	SQLRETURN PostCheck                ( POST_CHECK* check );
	SQLRETURN PostSelect               ( POST_SELECT* select, u_long& cbLength );
	SQLRETURN PostInsert               ( POST_INSERT* insert );
	SQLRETURN PostUpdateStatus         ( POST_UPDATE_STATUS* status );
	SQLRETURN PostUpdate               ( POST_UPDATE* update, u_long& cbLength );
	SQLRETURN PostDelete               ( POST_DELETE* del, u_long& cbLength );

	// Friend
	SQLRETURN FriendSelect             ( FRIEND_SELECT* friendSelect, u_long& cbLength );
	SQLRETURN FriendInsert             ( FRIEND_INSERT* friendInsert );
	SQLRETURN FriendUpdate             ( FRIEND_UPDATE* friendUpdate );
	SQLRETURN FriendDelete             ( FRIEND_DELETE* friendDelete );
	
	// 캐릭터 SKILL
	SQLRETURN SkillSelect              ( SKILL_SELECT* skillSelect, u_long& cbLength );
	SQLRETURN SkillInsert              ( SKILL_INSERT* skillInsert );
	SQLRETURN SkillUpdate              ( SKILL_UPDATE* skillUpdate );
	SQLRETURN SkillCoolTime            ( SKILL_COOLTIME* skillCoolTime );

	// 캐릭터 SKILL 치트로 배우기
	SQLRETURN SkillCheatInsert		   ( SKILL_CHEAT_INSERT* skillInsert );
	SQLRETURN SkillCheatUpdate		   ( SKILL_CHEAT_UPDATE* skillUpdate );

	// 캐릭터 스킬 효과
	SQLRETURN SkillInfluenceSelect     ( SKILL_INFLUENCE_SELECT* influenceSelect, u_long& cbLength );
	SQLRETURN SkillInfluenceUpdate     ( SKILL_INFLUENCE_UPDATE* influenceUpdate );
	// Skill 스크립트
	SQLRETURN TruncateSkillListPc      ( void );
	SQLRETURN SkillListPcInsert        ( void* pPlayerSkillBaseInfo );
	// 직업별 기본스킬 스크립트
	SQLRETURN TruncateDefalultSkill    ( void );
	SQLRETURN DefalultSkillInsert      ( unsigned long jobStep, unsigned long skillIdx, unsigned char race, unsigned long jobIdx );

	// 직업 트리
	SQLRETURN TruncateJobTree          ( void );
	SQLRETURN JobTreeInsert            ( unsigned long jobIdx, unsigned long parentJobIdx );

	/// 퀘스트 정보
	SQLRETURN TruncateQuestDefine      ( void );
	SQLRETURN TruncateQuestProgress    ( void );

	SQLRETURN InsertQuestDefine        ( void* questDefine );
	
	SQLRETURN QuestInsert              ( QUEST_INSERT* questInsert );
	SQLRETURN QuestSelect              ( QUEST_SELECT* questSelect, u_long& cbLength );
	SQLRETURN QuestDelete              ( QUEST_DELETE* questDelete );
	SQLRETURN QuestComplete            ( QUEST_COMPLETE* questComplete );
	SQLRETURN QuestReward              ( QUEST_REWARD* questReward, u_long& cbLength );
	SQLRETURN QuestUpdate              ( QUEST_UPDATE* questUpdate );
	SQLRETURN QuestEndListSelect       ( QUEST_ENDLIST* questEndList, u_long& cbLength );
	SQLRETURN QuestValidListSelect	   ( QUEST_VALIDLIST* questValidList, u_long& cbLength );
	SQLRETURN QuestRemove              ( QUEST_REMOVE* questRemove );
	
	// Etc
	SQLRETURN ItemSpreadValue          ( ITEM_SPREAD_VALUE* itemSpreadValue );
	SQLRETURN ItemTarotResult          ( ITEM_TAROT_RESULT* itemTarotResult );

	// 노점 [판매]
	SQLRETURN StallSellGet             ( STALL_SELL_GET* stallSellGet );

	// 큇슬롯
	SQLRETURN ShortcutSelect           ( SHORTCUT_SELECT* shortcutSelect );
	SQLRETURN ShortcutUpdate           ( SHORTCUT_UPDATE* shortcutUpdate );

	/// 호칭
	SQLRETURN TitleInsert			   ( TITLE_INSERT* titleInsert );
	SQLRETURN TitleSelect			   ( TITLE_SELECT* titleSelect, u_long& cbLength );

	/// 길드
	SQLRETURN VerifyGuild			   ( VERIFY_GUILD* verifyGuild );
	SQLRETURN GuildSelect			   ( GUILD_SELECT* guildSelect );
	SQLRETURN GuildUserSelect		   ( GUILDUSER_SELECT* guildUserSelect, u_long& cbLength );

	SQLRETURN GuildCreate			   ( GUILD_CREATE* guildCreate );
	SQLRETURN GuildDelete			   ( GUILD_DELETE* guildDelete );
	SQLRETURN GuildUpdate			   ( GUILD_UPDATE* guildUpdate );
	SQLRETURN GuildUserAdd			   ( GUILDUSER_ADD* guildUserAdd );
	SQLRETURN GuildUserOut			   ( GUILDUSER_OUT* guildUserOut );
	SQLRETURN GuildUserGivePosition	   ( GUILDUSER_GIVEPOSITION* guildUserGivePosition );
	SQLRETURN GuildMarkUpdate		   ( GUILDMARK_UPDATE* guildMarkUpdate );

	/// 전문직업
	SQLRETURN MakeSkillInsert          ( MAKESKILL_INSERT* pInsert );
	SQLRETURN MakeSkillSelect          ( MAKESKILL_SELECT* pSelect, u_long& cbLength );
	SQLRETURN MakeSkillDelete          ( MAKESKILL_DELETE* pDelete );
	SQLRETURN RecipeInsert             ( RECIPE_INSERT* pInsert );
	SQLRETURN RecipeGroupInsert        ( RECIPE_GROUP_INSERT* pRecipeGroup );
	SQLRETURN RecipeCoolTime           ( RECIPE_COOLTIME* recipeCoolTime );

	/// NPC 점집 운세
	SQLRETURN FortuneSelect			   ( FORTUNE_SELECT* fortuneSelect );
	SQLRETURN FortuneUpdate			   ( FORTUNE_UPDATE* fortuneUpdate );

	SQLRETURN ForcePointUpdate         ( FORCE_POINT_UPDATE* update );

	/// PVP
	SQLRETURN PVPUserInsert            ( PVP_USER_INSERT* insert );
	SQLRETURN PVPUserSelect            ( PVP_USER_SELECT* select, u_long& cbLength );
	SQLRETURN PVPUserRemove            ( PVP_USER_REMOVE* remove );
	SQLRETURN PVPUserClear             ( PVP_USER_CLEAR* clear );

	// Party
	SQLRETURN PartyInsert              ( PARTY_INSERT* insert );

	// ETC
	SQLRETURN WhisperSelect            ( WHISPER_SELECT* select );

	// GM
	SQLRETURN GMJobUpdate			   ( GM_JOB_UPDATE* update, u_long& cbLength );
	SQLRETURN GMEventSelect			   ( VERIFY_GM_EVENT* select, u_long& cbLength );
	SQLRETURN GMEventInsert			   ( GM_EVENT_INSERT* insert );
	SQLRETURN GMEventUpdate			   ( GM_EVENT_UPDATE* update );

	/// 아이템드랍 시간제약
	SQLRETURN ClearItemDropLimit       ( );
	SQLRETURN InsertItemDropLimit      ( unsigned long itemIdx, unsigned long limitTime );
	SQLRETURN ItemDropLimitSelect      ( ITEM_DROPLIMIT_TABLE* select, u_long& cbLength );
	SQLRETURN ItemDropLimitUpdate      ( ITEM_DROPLIMIT_UPDATE* pUpdate );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLGameStmt(void);
};

inline SQLRETURN cSQLGameStmt::SQLClose(SQLRETURN sqlReturn, SQLHSTMT sqlHStmt)
{
	if ( SQL_SUCCEEDED( sqlReturn ) )
	{
		while ( SQLMoreResults( sqlHStmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( sqlHStmt );								// 열려있는 결과 닫기.
	}
	return sqlReturn;
}
inline void cSQLGameStmt::SQLClose(SQLHSTMT sqlHStmt)
{
	while ( SQLMoreResults( sqlHStmt ) == SQL_SUCCESS );		// 저장프로시저 쿼리 마무리.
	SQLCloseCursor( sqlHStmt );									// 열려있는 결과 닫기.
}

// SQLGetFullInv Method
inline void cSQLGameStmt::SQLGetFullInv(SQLHSTMT sqlHStmt, TB_INVENTORY* table)
{
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT param;

	SQLGetData( sqlHStmt, param=1, SQL_C_SLONG,   &table->idx,             0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->itemDefineIdx,   0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->itemDefineIndex, 0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SSHORT,  &table->number,          0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SSHORT,  &table->count,           0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->enhanced,        0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->cardSlot1,       0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->cardSlot2,       0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->cardSlot3,       0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->cardSlot4,       0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->cardSlot5,       0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->seal,            0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->cash,            0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->apply,           0, &(strLenOrInd=0) );
}

// SQLGetBaseInv Method
inline void cSQLGameStmt::SQLGetBaseInv(SQLHSTMT sqlHStmt, TB_INVENTORY* table)
{
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT param;

	SQLGetData( sqlHStmt, param=1, SQL_C_SLONG,   &table->idx,             0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SLONG,   &table->itemDefineIndex, 0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SSHORT,  &table->number,          0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_SSHORT,  &table->count,           0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->enhanced,        0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->seal,            0, &(strLenOrInd=0) );
	SQLGetData( sqlHStmt, ++param, SQL_C_TINYINT, &table->apply,           0, &(strLenOrInd=0) );
}

#endif // __SQL_GAME_STATEMENT_H__
