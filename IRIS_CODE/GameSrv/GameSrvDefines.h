/* ====================================================================
 * 파    일 :	gameSrvDefines.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/26/07
 * 주의사항 :	
 * =================================================================== */
#ifndef __GAME_SERVER_DEFINES_H__
#define __GAME_SERVER_DEFINES_H__

#include "Item_Common.h"
#include "Skill_Common.h"
#include "Guild_Common.h"
#include "PVP_Common.h"

#pragma once

#pragma pack(1) // 1Byte 정렬 설정

enum eItemBillApply
{
	ItemBillApplyNone =0,					// 사 용 중 - 기본값.
	ItemBillApplyRemoved,					// 사용중지 - Database를 통해 입력.
	ItemBillApplyReserved,					// 삭제예약 - GameServer에서만 사용.
};

struct TB_ITEM_BILL
{
	long idx;								// 고유번호
	BYTE type;								// 종류(0 기간, 1 시간)
	long validDate;							// 유효기간(초 단위 사용)
	long validTime;							// 유효시간(초 단위 사용)
	long inventoryIdx;						// TB_INVENTORY::IDX 고유번호
	BYTE apply;								// 사용여부(0 사용중, 1 사용중지)
};

struct TB_ITEM_DEFINE
{
	long    idx;							// 고유번호
	long    idxAbility;						// 테이블의 고유번호 - 속    성 (가상 컬럼)
	long    idxLimit;						// 테이블의 고유번호 - 제    한 (가상 컬럼)
	long    idxCard;						// 테이블의 고유번호 - 카    드 (가상 컬럼)
	long    idxCardSlot;					// 테이블의 고유번호 - 카드슬롯 (가상 컬럼)
	long    idxTarot;						// 테이블의 고유번호 - 타    로 (가상 컬럼)
	long    idxEnhanced;					// 테이블의 고유번호 - 강    화 (가상 컬럼)
	long    idxEnhancedRate;				// 테이블의 고유번호 - 강화확률 (가상 컬럼)
	long    idxDisjoint;					// 테이블의 고유번호 - 분    해 (가상 컬럼)
	long    idxChange;						// 테이블의 고유번호 - 교    환 (가상 컬럼	)
	long    index;							// 기획:분류번호
	wchar_t name[51];						// 이름
	BYTE    type;							// 종류(기획참조)
	BYTE    subType;						// 종류(세부항목)
	BYTE    searchType;						// 검색종류(대분류)
	short   searchSubType;					// 검색종류(중분류)
	short   weight;							// 무게
	short   capacity;						// 제한수량
	long    sellType;						// 판매방법(기획참조)
	long    sellPrice;						// 판매가격
	long    buyType;						// 구입방법(기획참조)
	long    buyPrice;						// 구입가격
	long    present;						// 출현수
	long    maxPresent;						// 최대 출현수
	long    maxInventory;					// 보유가능개수
	BYTE    licenseClass;					// 기간제 설정(0일반/1실제시간/2게임시간/3무제한)
	long    licenseValidThru;				// 사용기한
	BYTE    licenseType;					// 인증여부(0인증완료/1미인증)
	//BYTE    sealcount;						// 봉인설정
	BYTE    exchange;						// 사용자거래 여부(0거래가능/1거래불가/2미인증거래가능/3미인증거래불가)
	long    abilityIndex;					// 기획:분류번호
	long    limitIndex;						// 기획:분류번호
	long    cardIndex;						// 기획:분류번호
	long    cardSlotIndex;					// 기획:분류번호
	long    tarotIndex;						// 기획:분류번호
	long    spreadIndex;					// 기획:분류번호
	long    enhancedIndex;					// 기획:분류번호
	long    enhancedRateIndex;				// 기획:분류번호
	long    disjointIndex;					// 기획:분류번호
	long    changeIndex;					// 기획:분류번호
	long    titleIndex;						// 기획:분류번호
	long    setIndex;						// 기획:분류번호
	long    iconIndex;						// 기획:분류번호
	BYTE    rareLevel;						// 기획:분류번호
	BYTE    webPage;						// 홈페이지 공개여부(0공개/1공개안함)
//	wchar_t desc[1];						// 설명(생략)
	BYTE    apply;							// 적용여부(0미사용/1사용중/2사용안함)
};

struct TB_ITEM_ABILITY
{
	long  idx;								// 고유번호
	long  index;							// 기획:분류번호
	short def_class;						// 방어구재질
	BYTE  range_type;						// 범위방식
	BYTE  target_type;						// 효과적용타겟
	short use_range;						// 사용거리
	short p_def;							// 물리:방어력
	short m_def;							// 마법:방어력
	short atk_range;						// 공격 거리
	short atk_spd;							// 공격 속도
	long  cool_time1;						// 쿨 타임
	long  cool_time2;						// 그룹 쿨 타임
	short min_p_atk;						// 물리:최소공격력
	short max_p_atk;						// 물리:최대공격력
	short min_m_atk;						// 마법:최소공격력
	short max_m_atk;						// 마법:최대공격력
	short heal;								// 힐 회복량

	short opt01_index;						// 옵션01인덱스(기획참조)
	short opt01_value;						// 옵션01효과값(기획참조)
	short opt02_index;						// 옵션02인덱스(기획참조)
	short opt02_value;						// 옵션02효과값(기획참조)
	short opt03_index;						// 옵션03인덱스(기획참조)
	short opt03_value;						// 옵션03효과값(기획참조)
	short opt04_index;						// 옵션04인덱스(기획참조)
	short opt04_value;						// 옵션04효과값(기획참조)
	short opt05_index;						// 옵션05인덱스(기획참조)
	short opt05_value;						// 옵션05효과값(기획참조)

	long  influence_idx;					// 스킬효과
	long  active_index;						// 발동효과
};

struct TB_ITEM_COOLTIME1
{
	TB_ITEM_COOLTIME1* next;				// 그룹 쿨 타임
	long               itemDefineIdx;		// 테이블의 고유번호
	long               itemDefineIndex;		// 기획:분류번호
	long               cooltime;			// 쿨 타임
};
struct TB_ITEM_COOLTIME2
{
	long               idx;					// 구룹번호
	TB_ITEM_COOLTIME1* cooltime1;			// 쿨 타임
};

enum ITEM_LIMIT_SERVER_TYPE
{
	ITEM_LIMIT_COMMON = 0,
	ITEM_LIMIT_PVP,
	ITEM_LIMIT_TUTORIAL,
	ITEM_LIMIT_FIELD,
};

struct TB_ITEM_LIMIT
{
	long            idx;                            // 고유번호
	long            index;							// 기획:분류번호
	BYTE            charRace;						// 캐릭터:종족
	BYTE            charGender;						// 캐릭터:성별
	long            charJob1;						// 캐릭터:직업
	long            charJob2;						// 캐릭터:직업
	BYTE            charLevelMin;					// 캐릭터:레벨
	BYTE            charLevelMax;					// 캐릭터:레벨
	BYTE            serverType;						// 서버분류(0없음/1전장전용/2튜토리얼전용/3필드전용)
	BYTE            forceType;						// 세력
	BYTE            makeSkill;						// 보조직업
    unsigned long   makeSkillExp;                   // 보조직업 숙련도
};

struct TB_ITEM_CARD
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	long episode;							// 판
	long number;							// 카드번호
	BYTE level;								// 희귀도
	BYTE type;								// 카드분류
	BYTE subType;							// 장착위치
};

struct TB_ITEM_CARD_SLOT
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	BYTE slot1;								// 카드슬롯1(0없음/1A형/2B형/3AB형/4O형)
	BYTE slot2;								// 카드슬롯2(0없음/1A형/2B형/3AB형/4O형)
	BYTE slot3;								// 카드슬롯3(0없음/1A형/2B형/3AB형/4O형)
	BYTE slot4;								// 카드슬롯4(0없음/1A형/2B형/3AB형/4O형)
	BYTE slot5;								// 카드슬롯5(0없음/1A형/2B형/3AB형/4O형)
};

struct TB_ITEM_TAROT
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	long number;							// 번호
	BYTE level;								// 레벨
	long strValue;							// 정방향수치
	long invValue;							// 약방향수치
	long prop;								// 속성
	long episode;							// 에피소드(판)
};

struct TB_ITEM_TAROT_RESULT
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	long min_value;							// 최소값
	long max_value;							// 최대값
	long influence_idx1;					// 스킬효과
	long influence_idx2;					// 스킬효과
	long influence_idx3;					// 스킬효과
};

struct TB_ITEM_SPREAD
{
	long  idx;								// 고유번호
	long  index;							// 기획:분류번호
	BYTE  axisArray;						// 좌표 인덱스
	short xPos;								// X좌표
	short yPos;								// Y좌표
	BYTE  dir;								// 방향
	char  calc[2];							// 계산식
	short value;							// 계산수치
};

struct TB_ITEM_ENHANCED
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	long row;								// 적용행수
	struct {
		//BYTE equip;							// 설정
		BYTE type;							// 속성1(0없음,1물리공격력,2마법공격력,3물리방어력,4마법방어력,5힐회복량)
		short value[MAX_ITEM_ENHANCED];
	} value[3];
};

struct TB_ITEM_ENHANCED_RATE
{
	long idx;								// 고유번호
	long index;								// 기획:분류번호
	struct {
		BYTE type;							// 종류(1일반/2고급)
		long success [MAX_ITEM_ENHANCED];	// 성공
		long down    [MAX_ITEM_ENHANCED];	// 하락
		long destruct[MAX_ITEM_ENHANCED];	// 파괴
	} value[2];
};

struct TB_ITEM_DISJOINT
{
	long  idx;								// 고유번호
	long  index;							// 기획:분류번호
	long  price;							// 가격
	long  itemDefineIndex1;					// 아이템1(TB_ITEM_DEFINE::INDEX)
	short inventoryCount1;					// 인벤토리 지급 수량1
	long  itemDefineIndex2;					// 아이템2(TB_ITEM_DEFINE::INDEX)
	short inventoryCount2;					// 인벤토리 지급 수량2
	long  itemDefineIndex3;					// 아이템3(TB_ITEM_DEFINE::INDEX)
	short inventoryCount3;					// 인벤토리 지급 수량3
	long  itemDefineIndex4;					// 아이템4(TB_ITEM_DEFINE::INDEX)
	short inventoryCount4;					// 인벤토리 지급 수량4
	long  itemDefineIndex5;					// 아이템5(TB_ITEM_DEFINE::INDEX)
	short inventoryCount5;					// 인벤토리 지급 수량5
	long  itemDefineIndex6;					// 아이템6(TB_ITEM_DEFINE::INDEX)
	short inventoryCount6;					// 인벤토리 지급 수량6
};

struct TB_ITEM_CHANGE
{
	long  idx;										// 고유번호
	long  index;									// 기획:분류번호
	long  minMoney;									// 최소 금액
	long  maxMoney;									// 최대 금액
	long  rate;										// 변환가능 수량
	long  row;										// 적용된 행수
	long  itemDefineIndex[ MAX_ITEM_CHANGE_DATA ];	// 아이템 정의 인덱스
	short itemCount[ MAX_ITEM_CHANGE_DATA ];		// 아이템 수량
	short enhanced[ MAX_ITEM_CHANGE_DATA ];			// enhanced
	long  changeRate[ MAX_ITEM_CHANGE_DATA ];		// 변환확률
};


struct TB_CHARACTER_LIST
{
	long        idx;							    // 캐릭터 인덱스
	wchar_t     name[17];						    // 캐릭터 이름
	char        level;							    // 캐릭터 레벨
	char        race;							    // 캐릭터 종족
	char        gender;							    // 캐릭터 성별
	long        job;							    // 캐릭터 직업

	long        hair;							    // 캐릭터 헤어
	long        hairColor;						    // 캐릭터 머리색
	long        face;							    // 캐릭터 얼굴

    long        equipItem[LEN_INVENTORY_EQUIP];		// 착용중 아이템
	char        enhancedItem[LEN_INVENTORY_EQUIP];  // 착용중 강화 아이템

	char        activeWeapon;					    // 아이템 무기 스왑

	short       mapNum;							    // 캐릭터 맵(위치)
	char        standIdx;						    // 캐릭터 생성순서

	long        exp;							    // 캐릭터 경험치
	char        skillLevel;					        // 캐릭터 스킬 레벨
	long        skillExp;						    // 캐릭터 스킬 경험치
	long        skillPointTotal;				    // 캐릭터 스킬 포인트
	long        guild_index;					    // 캐릭터 길드 인덱스

	long        delWaitTime;					    // 캐릭터 삭제대기 시간
	BYTE        apply;
};

struct TB_CHARACTER
{
	long    idx;							// 캐릭터 인덱스
	wchar_t name[17];						// 캐릭터 이름
	char    level;							// 캐릭터 레벨
	char    race;							// 캐릭터 종족
	char    gender;							// 캐릭터 성별
	long    job;							// 캐릭터 직업

	long    hair;							// 캐릭터 헤어
	long    hairColor;						// 캐릭터 머리색
	long    face;							// 캐릭터 얼굴

    long    equipItem[LEN_INVENTORY_EQUIP];		// 착용중 아이템
	char    enhancedItem[LEN_INVENTORY_EQUIP];  // 착용중 강화 아이템

	char    activeWeapon;					// 활성 무기정보 (0 Front, 1 Back)

	short   mapNum;							// 캐릭터 맵(위치)
	float   xPos;							// 캐릭터 X 좌표
	float   yPos;							// 캐릭터 Y 좌표

	long    exp;							// 캐릭터 경험치
	long	HP;								// 캐릭터 HP
	long	MP;								// 캐릭터 MP

	char    skillLevel;						// 스킬 레벨
	long    skillPointRemain;				// 스킬 남은포인트
	long	skillPointTotal;				// 스킬 총 획득량
	long    skillExp;						// 스킬 경험치

	long    tarotPoint;						// 타로 포인트

	__int64 money;							// 소지금액
	__int64 deposit;						// 예치금액

	long    option1;						// 옵션1

	unsigned char	mForceType;				// 선택 세력
	unsigned long	mPvPPoint;				// PVP  포인트
	unsigned long	mFirePoint;				// 불   포인트
	unsigned long	mFireFriendly;			// 불   친밀도
	unsigned long	mWaterPoint;			// 물   포인트
	unsigned long	mWaterFriendly;			// 물   친밀도
	unsigned long	mWindPoint;				// 바람 포인트
	unsigned long	mWindFriendly;			// 바람 친밀도
	unsigned long	mEarthPoint;			// 땅   포인트
	unsigned long	mEarthFriendly;			// 땅   친밀도

	unsigned long	mTitleIndex;			// 셋팅된 호칭인덱스

	unsigned long	mGuildIndex;			// 길드 인덱스
	unsigned char	mGuildPosition;			// 길드내 직위

	unsigned char	mMakeSkill1;			// 전문 기술
	unsigned char	mMakeSkill2;
	unsigned char	mMakeSkillStep1;		// 전문 기술 단계
	unsigned char	mMakeSkillStep2;
	unsigned long	mMakeSkillExp1;			// 전문 기술 숙련도
	unsigned long	mMakeSkillExp2;

	char            standIdx;				// 캐릭터 생성순서

    unsigned short  mBagEnd;                // 인벤토리 크기
    unsigned short  mWearEnd;               // 카드 슬롯
    unsigned short  mWareHouseEnd;          // 창고 크기
    unsigned short  mMakeSkillEnd;          // 보조직업 갯수

	unsigned long	tutorialModeIndex;		// 튜토리얼 모드 인덱스

	unsigned long    mExpRecover;			// 경험치 복구량
	TIMESTAMP_STRUCT mExpRecoverendDate;	// 경험치 복구 종료시간
};

struct TB_DEFAULT_ITEMS
{
	long  characterJob;
	long  itemDefineIndex;
	short inventoryNumber;
	short inventoryCount;
};

enum eInventoryApply
{
	InventoryApplyNone =0,					// 인벤토리 사 용 중 - 기본값.
	InventoryApplyRemoved,					// 인벤토리 사용중지 - Database를 통해 입력.
	InventoryApplyExchange,					// 인벤토리 거 래 중 - GameServer에서만 사용.
	InventoryApplyStallSell,				// 인벤토리 판 매 중 - GameServer에서만 사용.
	InventoryApplyGetAuto,					// 인벤토리 자동습득 - GameServer에서만 사용.
};

struct TB_INVENTORY
{
	long  idx;								// 고유번호
	long  itemDefineIdx;					// 아이템 정의 고유번호
	long  itemDefineIndex;					// 아이템 정의 인덱스
	short number;							// 번호(슬롯인덱스)
	short count;							// 수량
	BYTE  enhanced;							// 강화단계(0~15 까지)
	long  cardSlot1;						// 카드슬롯1
	long  cardSlot2;						// 카드슬롯2
	long  cardSlot3;						// 카드슬롯3
	long  cardSlot4;						// 카드슬롯4
	long  cardSlot5;						// 카드슬롯5
	BYTE  seal;								// 봉인(0봉인해제/1봉인됨)
	BYTE  cash;								// 캐시(0일반/1캐시)
	BYTE  apply;							// 사용여부
};

struct TB_INVENTORY_COOLTIME
{
	long itemDefineIndex;					// 아이템 정의 인덱스
	long cooltime2;							// 쿨타임 그룹 인덱스
	long cooltime1;							// 쿨타임(적용된 시간 ms)
	long cooltime;							// 쿨타임(남은시간 ms)
};

struct TB_FRIEND
{
	long           idx;						// 고유번호
	long           characterIdx;			// 캐릭터 고유번호
	wchar_t        characterName[ 17 ];		// 캐릭터 이름
	char           race;					// 캐릭터 종족
	char           gender;					// 캐릭터 성별
	char           level;					// 캐릭터 레벨
	unsigned long  job;						// 캐릭터 직업(ePLAYER_JOB)
	unsigned short mapNum;					// 캐릭터 맵(위치)
	DWORD		   channelNum;				// 접속한 채널 번호
	//BYTE           gameIn;					// 접속상태(0접속안됨/1접속중)
	BYTE           status;					// 등록상태(0등록완료/1등록신청)
	BYTE           block;					// 차단상태(0차단안함/1차단함)
};

struct TB_STALL_SELL
{
	long idx;								// 고유번호
	long characterIdx;						// 캐릭터 고유번호
	long price;								// 판매 가격
};

struct TB_SHORTCUT
{
	char binary[500];						// 이진데이터(500BYTE)
};

struct TB_CHARACTER_SKILL
{
	unsigned long	mSkillIdx;				// 스킬 인덱스
	unsigned char	mSkillStep;				// 스킬 단계
	long			mEndCoolTime;			// 쿨타임 종료 시간
};

struct TB_RECIPE_COOLTIME
{
	unsigned long	mMakeSkill;				// 조합스킬
	unsigned long	mRecipeIdx;				// 레시피
	unsigned long	mLeftCoolTime;			// 쿨타임
};

struct TB_GUILDMARK
{
	BYTE binary[2360];						// 이진데이터(2348BYTE) - 24 * 24 * (32/8) + 18 + 26
};

// 효과
typedef sInfluence TB_CHARACTER_INFLUENCE;

/// 퀘스트 진행정보
struct TB_QUEST_PROGRESS
{
	unsigned long   idx;						// [In]  db 고유인덱스
	unsigned long   questIdx;					// [In]  퀘스트 고유번호
	__int64	 		check;						// [In]  진행정보
	unsigned long	startTime;					// 시간 검사용 
	unsigned long	restTime;					// [Out] 남은시간
	char			status;						// [Out] 퀘스트상태 (0:진행중, 2:진행완료, 4:진행실패)
	long			retvalue;					// [Out] 결과
};

/// 호칭 정보
struct TB_TITLE
{
	long			idx;						// [In] db 고유인덱스
	unsigned long	titleIdx;					// [In] 호칭 인덱스
};

/// 길드 정보
struct TB_GUILD
{			
	unsigned long		idx;						// [Out] 길드 고유번호(길드인덱스)
	wchar_t				name[13];					// [Out] 길드명
	unsigned long		masterIndex;				// [Out] 길드장 인덱스
	wchar_t				masterName[17];				// [Out] 길드명
	wchar_t				notice[41];					// [Out] 길드 공지
	TIMESTAMP_STRUCT	markDate;					// [Out] 마크 최신 등록일
	BYTE				mark[2360];					// [Out] 마크 - 24 * 24 * (32/8) + 18 + 26 (헤더크기 + 확장)
	char				apply;						// [Out] 삭제 정보
};

/// 길드 유저 정보
struct TB_GUILD_USER
{			
	unsigned long	characterIdx;				// [Out] 캐릭터 인덱스
	wchar_t			name[17];					// [Out] 캐릭터명
	unsigned char	level;						// [Out] 레벨
	unsigned long   job;						// [Out] 캐릭터 직업
	unsigned short	mapnum;						// [Out] 맵 번호
	unsigned char	position;					// [Out] 직위
	DWORD			channelNum;					// 접속한 채널 번호
	bool			sendInfo;					// 전송 플래그
	unsigned long	tick;						// 접속 유지 정보
};

/// 제조스킬
struct TB_MAKESKILL
{			
	unsigned long	mMakeSkill;			// [Out] 제조스킬
	unsigned long	mRecipeIdx;			// [Out] 레시피idx
	unsigned long	mCoolTime;			// [Out] 쿨타임
};

struct TB_PARTY_USER
{
	long  idx;
	long  characterIdx;
	short ch;
};

/// 아이템 드랍제약
struct TB_ITEM_DROPLIMIT
{
	unsigned long		mItemIdx;				/// 아이템 종류
	unsigned long		mDropTerm;				/// 드랍 주기
	TIMESTAMP_STRUCT	mDropEndTime;			/// 드랍 현재 주기 끝나는 시간
	unsigned char		mIsDrop;				/// 이미 드랍했는지 유무
};


//--------------------------------------------------------------------
// GAME SYN
struct MSG_SYN_POSTFRIEND_CONNECT : MSGROOT
{
	long			characterIdx;
	char            level;				// 캐릭터 레벨
	unsigned long   job;				// 캐릭터 직업(ePLAYER_JOB)
	unsigned short  mapNum;				// 캐릭터 맵(위치)
	BYTE			connect;			// (1:접속,0:미접속)
	bool			sendConnect;		//
	long			friendlist[ 50 ];	// 친구 인덱스
};

// 길드원 접속 여부
struct MSG_SYN_POSTGUILD_CONNECT : MSGROOT
{
	unsigned long	idx;
	unsigned long	characterIdx;
	DWORD			channelNum;			// 접속하는 채널번호
	BYTE			connect;			// (1:접속,0:미접속)
	bool			sendConnect;		//
};

struct sGuildConnect
{
	unsigned long	characterIdx;
	char			level;
	unsigned short	job;
	DWORD			channelNum;
};

// 길드원 및 서버 LIVE 체크
struct MSG_SYN_POSTGUILD_TTL : MSGROOT
{	
	unsigned long	idx;
	sGuildConnect	data;
};

/// 길드 생성
struct MSG_SYN_POSTGUILD_CREATE : MSGROOT
{
	long			idx;									// 길드 인덱스
	wchar_t			name[MAX_GUILDNAME_BUFFER_SIZE];		// 길드명
	wchar_t			masterName[MAX_NAME_BUFFER_SIZE];		// 길드장명
	unsigned long	masterIndex;						    // 길드장 인덱스

	// master 정보
	char			level;						// 레벨
	unsigned short	job;						// 캐릭터 직업
	short			mapnum;						// 맵 번호
	char			position;					// 직위
};

/// 길드 삭제
struct MSG_SYN_POSTGUILD_DELETE : MSGROOT
{
	long			idx;					/// 길드 인덱스
};

/// 길드 유저 추가
struct MSG_SYN_POSTGUILD_USERADD : MSGROOT
{
	long			idx;						// 길드 인덱스
	unsigned long	characterIdx;			    // 유저 인덱스
	wchar_t			name[MAX_NAME_BUFFER_SIZE];	// 길드장명
	char			level;						// 레벨
	unsigned short	job;						// 캐릭터 직업
	short			mapnum;						// 맵 번호
	char			position;					// 직위
};

/// 길드원 제명
struct MSG_SYN_POSTGUILD_USERCUT : MSGROOT
{
	long			idx;						// 길드 인덱스
	unsigned long	characterIdx;			    // 유저 인덱스
};

/// 길드 탈퇴
struct MSG_SYN_POSTGUILD_USEROUT : public MSG_SYN_POSTGUILD_USERCUT
{
	long			idx;						// 길드 인덱스
	unsigned long	characterIdx;			    // 유저 인덱스
};

/// 직위 부여
struct MSG_SYN_POSTGUILD_GIVEPOSITION : MSGROOT
{
	long			idx;						// 길드 인덱스
	unsigned long	characterIdx;			    // 유저 인덱스
	char			position;					// 직위
};

/// 길드 마크 변경
struct MSG_SYN_POSTGUILD_UPDATEMARK : MSGROOT
{
	long			 idx;					// 길드 인덱스
	TIMESTAMP_STRUCT markRegistDate;		// 마크 등록일
	BYTE			 mark[MARK_SIZE];		/// 길드마크
};

/// 길드 공지
struct MSG_SYN_POSTGUILD_NOTICE : MSGROOT
{
	long			idx;									// 길드 인덱스
	wchar_t			notice[MAX_GUILD_NOTICE_BUFFER_SIZE];	// 새 공지

	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_POSTGUILD_NOTICE) - sizeof(notice) + ::wcslen(notice)*sizeof(wchar_t); }
};

/// 길드 채팅
struct MSG_SYN_POSTGUILD_CHAT : MSGROOT
{
	long			idx;								// 길드 인덱스
	sInventory		inventory;							// 아이템 정보
    unsigned short  Job;                                // 보낸 플레이어 직업
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];			// 보낸 플레이어 이름
	wchar_t			Message[MAX_CHAT_BUFFER_SIZE];		// 메세지

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_POSTGUILD_CHAT) - sizeof(Message) + ::wcslen(Message)*sizeof(wchar_t); }
};

/// pvp 접속/비접속 명단
struct MSG_SYN_POSTPVP_PLAYERLIST
{
	unsigned long	mCount;
	unsigned long   mAryPos;
	sPvPPlayerList	mList[1];

	unsigned long	GetMsgLength() { return sizeof(MSG_SYN_POSTPVP_PLAYERLIST) - sizeof(mList) + (mCount * sizeof(sPvPPlayerList)); }
};

#pragma pack( ) // 1Byte 정렬 해제

#endif // __GAME_SERVER_DEFINES_H__
