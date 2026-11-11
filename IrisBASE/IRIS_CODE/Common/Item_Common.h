/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	2007.06.25
 * 주의사항 :	
 * =================================================================== */
#pragma once
#pragma pack(push,1)

#include "BaseObject_Common.h"
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"
#include "Theme_Common.h"

#define MAX_ITEM_OWNER			1		// 최대 드롭 아이템 소유자
#define MAX_ITEM_PARTY			6		// 최대 드롭 아이템 소유자(파티)
#define MAX_ITEMS				30		// 최대 드롭 아이템 개수

#define ITEM_VALID_DISTANCE		100.f	// 아이템 줍기 유효거리
#define MAX_ITEM_EXCHANGE		15		// 최대 거래 개수

#define ITEM_AGENT_NPC_FEE		500		// 판매대행 등록 수수료

enum eItemSellType
{
	ItemSellNone = 0,	// 판매불가
	ItemSellMoney,		// 게임머니
	ItemSellPvp,		// 전장 포인트 - 불
	ItemSellTarot,		// 점집 포인트
};
enum eItemBuyType
{
	ItemBuyNone = 0,	// 판매불가
	ItemBuyMoney,		// 게임머니
	ItemBuyPvp,			// 전장 포인트
	ItemBuyTarot,		// 점집 포인트
};

enum eItemActiveWeapon
{
	ItemActiveFront = 0,	// 활성 무기정보 - Front.
	ItemActiveBack,			// "             - Back.
	ItemActiveMax,

	ItemActiveSwitch = 1,
};

enum eItemExchangeStatus
{
	ItemExchangeNone = 0,
	ItemExchangeAsk,
	ItemExchangeBegin,
	ItemExchangeOk,
	ItemExchangeEnd,
};


#define STALL_TITLE_LEN			50						// 노점 타이틀 길이
#define MAX_STALL_TITLE_LEN		(STALL_TITLE_LEN+1)		// 노점 타이틀 최대길이(NULL 포함)
#define MAX_STALL_GUEST			10						// 노점 게스트(GEUST) 인원
#define MAX_STALL_ITEM			10						// 등록 가능한 아이템의 개수

enum eStallSellItemStatus
{
	StallSellItemNone = 0,	// 등록안됨
	StallSellItemStart,		// 판매시작
	StallSellItemPause,		// 판매일시중지
};


#ifndef __ITEM_TYPE__
#define __ITEM_TYPE__

enum ITEM_TYPE
{
	ITEM_WEAPON    = 0,					// 무기
		ITEM_WEAPON_SWORD = 0,			// 한손검
		ITEM_WEAPON_BLADE,				// 양손검
		ITEM_WEAPON_DUAL,				// 이도류
		ITEM_WEAPON_CUTTER,				// 단  검
		ITEM_WEAPON_GUN,				// 총,
		ITEM_WEAPON_STAFF,				// 지팡이
		ITEM_WEAPON_SHIELD,				// 방패

	ITEM_WEAR      = 1,					// 방어구
		ITEM_WEAR_HEAD = 0,				// 머리
		ITEM_WEAR_UPPER,				// 상의
		ITEM_WEAR_LOWER,				// 하의
		ITEM_WEAR_HANDS,				// 손
		ITEM_WEAR_FEET,					// 발
		ITEM_WEAR_ONEPIECE,				// 원피스
		ITEM_WEAR_COS_ONEPIECE,			// 코스튬 원피스
		ITEM_WEAR_COS_HEAD,			    // 코스튬 머리
		ITEM_WEAR_COS_UPPER,			// 코스튬 상의
		ITEM_WEAR_COS_LOWER,			// 코스튬 하의
		ITEM_WEAR_COS_HANDS,			// 코스튬 손
		ITEM_WEAR_COS_FEET,				// 코스튬 발

	ITEM_ACCESSORY  = 2,				// 액세서리
		ITEM_ACCESSORY_EARRING = 0,		// 귀걸이
		ITEM_ACCESSORY_NECKLACE,		// 목걸이
		ITEM_ACCESSORY_BROOCH,			// 브로치
		ITEM_ACCESSORY_BRACELET,		// 팔찌
		ITEM_ACCESSORY_RING,			// 반지
		ITEM_ACCESSORY_COS_FACE,		// 코스튬 얼굴장식
		ITEM_ACCESSORY_COS_BACK,		// 코스튬 몸통장식

	ITEM_ETC1       = 3,				// 소모잡화
		ITEM_ETC1_POTION = 0,			// 물약
		ITEM_ETC1_MISC,					// 잡화
		ITEM_ETC1_QUEST,				// 퀘스트
		ITEM_ETC1_CHANGE,				// 체인지
        ITEM_ETC1_TITLE,				// 호칭
        ITEM_ETC1_SKILLRESET,			// 스킬 초기화
        ITEM_ETC1_EXP_RESTORE,          // 경험치 복구
        ITEM_ETC1_THEME_RESET,          // 테마 던전 귀속 초기화
        ITEM_ETC1_FREE_MOVE,            // 자유 이동
        ITEM_ETC1_SHOUT,                // 전체 외치기
        ITEM_ETC1_COOLTIME_CASH,        // 버프성 소모 캐시
		ITEM_ETC1_AUTODETECT,			// 오토감지용


	ITEM_MATERIAL   = 4,				// 재료
		ITEM_MATERIAL_SKILL = 0,		// 스킬
		ITEM_MATERIAL_CLOTH,			// 옷감
		ITEM_MATERIAL_LEATHER,			// 가죽
		ITEM_MATERIAL_METAL,			// 금속
		ITEM_MATERIAL_JEWEL,			// 보석
		ITEM_MATERIAL_MISC,				// 잡화
		ITEM_MATERIAL_MAP,				// 이동주문서
		ITEM_MATERIAL_VEHICLE,			// 탈것
		ITEM_MATERIAL_ENHANCED_DOWN_30,    // 아이템 강화 보호(30레벨 이하) : 하락 방지
		ITEM_MATERIAL_ENHANCED_DOWN_60,	   // 아이템 강화 보호(31~60레벨)   : 하락 방지
		ITEM_MATERIAL_ENHANCED_DESTROY_30, // 아이템 강화 보호(30레벨 이하) : 파괴만 보호
		ITEM_MATERIAL_ENHANCED_DESTROY_60, // 아이템 강화 보호(31~60레벨)   : 파괴만 보호

	ITEM_QUEST      = 5,				// 퀘스트
	ITEM_ETC2       = 6,				// 일반잡화
	ITEM_SPREAD     = 7,				// 스프레드
		ITEM_SPREAD_NORMAL = 0,			// 스프레드 - 일반
		ITEM_SPREAD_SILVER,				// 스프레드 - 실버
		ITEM_SPREAD_GOLD,				// 스프레드 - 골드
		ITEM_SPREAD_DIA,				// 스프레드 - 다이아

	ITEM_CARD       = 8,				    // 카드
		ITEM_CARD_TAROT = 0,			    // 카드-타로(메이저)
		ITEM_CARD_COMMON,				    // 카드-소모
		ITEM_CARD_EQUIP_C,				    // 카드-캐릭터장착
		ITEM_CARD_EQUIP_A,				    // 카드-A타입
		ITEM_CARD_EQUIP_B,				    // 카드-B타입
		ITEM_CARD_EQUIP_AB,				    // 카드-AB타입
		ITEM_CARD_EQUIP_O,				    // 카드-O타입
		ITEM_CARD_ENHANCE_WEAPON_LV1,	    // 카드-일반 무기강화(50레벨 이하)
		ITEM_CARD_ENHANCE_WEAPON_LV2,	    // 카드-고급 무기강화(50레벨 이하)
		ITEM_CARD_ENHANCE_WEAR_LV1,		    // 카드-일반 방어구강화(50레벨 이하)
		ITEM_CARD_ENHANCE_WEAR_LV2,		    // 카드-고급 방어구강화(50레벨 이하)
		ITEM_CARD_ENHANCE_WEAPON_LV1_75,    // 카드-일반 무기강화(51~75레벨)
		ITEM_CARD_ENHANCE_WEAPON_LV2_75,    // 카드-고급 무기강화(51~75레벨)
		ITEM_CARD_ENHANCE_WEAR_LV1_75,	    // 카드-일반 방어구강화(51~75레벨)
		ITEM_CARD_ENHANCE_WEAR_LV2_75,	    // 카드-고급 방어구강화(51~75레벨)
		ITEM_CARD_TAROT_SILVER,				// 카드-실버
		ITEM_CARD_TAROT_GOLD,				// 카드-골드
		ITEM_CARD_TAROT_DIA,				// 카드-다이아


	ITEM_MONSTER    = 9,				// 몬스터카드
		ITEM_MONSTER_UNKNOWN = 0,		// 몬스터-미분류
		ITEM_MONSTER_HUMAN,				// 몬스터-인간
		ITEM_MONSTER_ANIMAL,			// 몬스터-동물
		ITEM_MONSTER_PLANT,				// 몬스터-식물
		ITEM_MONSTER_INSECT,			// 몬스터-곤충
		ITEM_MONSTER_ELF,				// 몬스터-정령
		ITEM_MONSTER_UNDEAD,			// 몬스터-언데드
		ITEM_MONSTER_GIANT,				// 몬스터-거인
		ITEM_MONSTER_DRAGON,			// 몬스터-용

	ITEM_COIN		= 10,				// 코인
		ITEM_COIN_GOLD = 0,				// 코인-금
		ITEM_COIN_SILVER,				// 코인-은
		ITEM_COIN_COPPER,				// 코인-동
		ITEM_COIN_MEDAL_OLD,			// 낡은 훈장 
		ITEM_COIN_MEDAL_NORMAL,			// 일반 훈장
		ITEM_COIN_MEDAL_SILVER,			// 은빛 훈장
		ITEM_COIN_MEDAL_GOLD,			// 금빛 훈장

	ITEM_MIX_SKILL	= 11,				// 보조직업(조합 전문기술)
		ITEM_MIX_SKILL_L1 = 0,			// 초급
		ITEM_MIX_SKILL_L2,				// 중급
		ITEM_MIX_SKILL_L3,				// 고급
		ITEM_MIX_SKILL_L4,				// 마스터
		ITEM_MIX_RECIPE,				// 레시피
		ITEM_MIX_RECIPEGROUP,			// 레시피(그룹)

    ITEM_INVENTORY  = 12,               // 인벤토리
        ITEM_INVENTORY_BAG = 0,         // 인벤토리 확장
        ITEM_INVENTORY_CARD,            // 카드 슬롯 확장
        ITEM_INVENTORY_WAREHOUSE,       // 창고 확장
        ITEM_INVENTORY_MAKESKILL,       // 보조직업 확장
        ITEM_INVENTORY_PULL_CARD,       // 장착 카드 추출 주문서
        ITEM_INVENTORY_FREE_PUT_CARD,   // 카드 자유 장착 주문서
        ITEM_INVENTORY_AGENT_SUMMON,    // 판매대행 소환 주문서(소모용)
        ITEM_INVENTORY_POST_SUMMON,     // 우편함 소환 주문서(소모용)
        ITEM_INVENTORY_WAREHOUSE_SUMMON,// 창고 소환 주문서(소모용)
        ITEM_INVENTORY_USEFUL_PAPER,    // 편의기능 주문서

	ITEM_UNKNOWN   = 13,
};

#endif // __ITEM_TYPE__


#ifndef __ITEM_TARGET_TYPE__

#define ITEM_TARGET_NONE		0x00	// 사용불가
#define ITEM_TARGET_MYSELF		0x01	// 플레이어 자신
#define ITEM_TARGET_MONSTER		0x02	// 적
#define ITEM_TARGET_PLAYER		0x04	// 다른 플레이어
#define ITEM_TARGET_PARTY		0x08	// 파티구성원

#endif // __ITEM_TARGET_TYPE__


#ifndef __ITEM_TOOLTIP__

/// 툴팁컬러 ( 레어도 관련
enum eTooltipType
{
	eTOOLTIP_MALL = 0,	/// 몰
	eTOOLTIP_ETC,		/// 잡화
	eTOOLTIP_NORMAL,	/// 일반
	eTOOLTIP_MAGIC,		/// 매직
	eTOOLTIP_RARE,		/// 레어
	eTOOLTIP_UNIQUE,	/// 유니크
	eTOOLTIP_EPIC,		/// 에픽
	eTOOLTIP_REGEND,	/// 레젼드

	eTOOLTIP_MAX = eTOOLTIP_REGEND,

	eTOOLTIP_NONE, 
};

#endif __ITEM_TOOLTIP__


#ifndef __ITEM_LIMIT__

enum ITEM_RACE
{
	ITEM_RACE_HUMAN = 0,				// 인간
	ITEM_RACE_BEAST,					// 수인
	ITEM_RACE_ELF,						// 정령
	ITEM_RACE_ALL,						// 모두
};

enum ITEM_GENDER
{
	ITEM_GENDER_MALE = 0,				// 남자
	ITEM_GENDER_FEMALE,					// 여자
	ITEM_GENDER_ALL,					// 모두
};

enum ITEM_JOB
{
	ITEM_JOB_FIGHTER  = 0x01,			// 파이터 군
	ITEM_JOB_MAGE     = 0x02,			// 메이지 군
	ITEM_JOB_WANDERER = 0x04,			// 원더러 군
	ITEM_JOB_WARLOCK  = 0x08,			// 워  록 군
	ITEM_JOB_ARMSMAN  = 0x10,			// 암즈맨 군
	ITEM_JOB_RANGER   = 0x20,			// 레인저 군
	ITEM_JOB_ALL      = 0,				// 모  두 - (제한없음)
};

#endif // __ITEM_LIMIT__


#ifndef __ITEM_ENHANCED__

#define MIN_ITEM_ENHANCED		1		// 아이템 강화 유효단계 (0~10까지, 0일경우 강화없음)
#define MAX_ITEM_ENHANCED		10		// 아이템 강화 유효단계 (0~10까지, 0일경우 강화없음)

enum ITEM_ENHANCED_TYPE
{
	ITEM_ENHANCED_NONE=0,				// 없음
	ITEM_ENHANCED_PHYSIC_ATTACK,		// 물리공격력
	ITEM_ENHANCED_MAGIC_ATTACK,			// 마법공격력
	ITEM_ENHANCED_PHYSIC_DEFENSE,		// 물리방어력
	ITEM_ENHANCED_MAGIC_DEFENSE,		// 마법방어력
	ITEM_ENHANCED_MAGIC_HEAL,			// 힐 회복량
};

#endif // __ITEM_ENHANCED__


#ifndef __ITEM_CARD_SLOT__

#define ITEM_CARD_SLOT_NONE	0			// 없음
#define ITEM_CARD_SLOT_A	1			// A형
#define ITEM_CARD_SLOT_B	2			// B형
#define ITEM_CARD_SLOT_AB	3			// AB형
#define ITEM_CARD_SLOT_O	4			// O형

enum ITEM_CARD_SLOT
{
	ITEM_CARD_SLOT_1 = 1,
	ITEM_CARD_SLOT_2,
	ITEM_CARD_SLOT_3,
	ITEM_CARD_SLOT_4,
	ITEM_CARD_SLOT_5
};

#define MIN_CARD_SLOT	ITEM_CARD_SLOT_1	// 카드슬롯 최소
#define MAX_CARD_SLOT	ITEM_CARD_SLOT_5	// 카드슬롯 최대

#endif // __ITEM_CARD_SLOT__


#ifndef __ITEM_CHANGE__

#define MIN_ITEM_CHANGE			1			// 최소 교환가능 수량.
#define MAX_ITEM_CHANGE			3			// 최대 교환가능 수량.

#define MAX_ITEM_CHANGE_RATE	10			// 최대 아이템 교환데이터(확률).
#define MAX_ITEM_CHANGE_DATA	40			// 최대 아이템 교환데이터(인덱스&수량).

#endif // __ITEM_CHANGE__


#ifndef __INVENTORY_NUMBER__
#define __INVENTORY_NUMBER__

#define MIN_INVENTORY				0

enum INVENTORY_NUMBER
{
	// INVENTORY_WEAR_TYPE
	INVENTORY_WEAR_BEGIN = MIN_INVENTORY,

		INVENTORY_WEAR_HAT = INVENTORY_WEAR_BEGIN,
		INVENTORY_WEAR_BODY1,
		INVENTORY_WEAR_BODY2,
		INVENTORY_WEAR_HAND,
		INVENTORY_WEAR_FOOT,

		INVENTORY_WEAR_EARRING,
		INVENTORY_WEAR_NECKLACE,
		INVENTORY_WEAR_BROOCH,
		INVENTORY_WEAR_BRACELET,
		INVENTORY_WEAR_RING,

		INVENTORY_WEAR_CARD1,
		INVENTORY_WEAR_CARD2,
		INVENTORY_WEAR_CARD3,

	INVENTORY_WEAR_END = INVENTORY_WEAR_CARD3,

	// INVENTORY_HAND_TYPE
	INVENTORY_HAND_BEGIN,

		INVENTORY_HAND_LEFT1 = INVENTORY_HAND_BEGIN,
		INVENTORY_HAND_RIGHT1,

		INVENTORY_HAND_LEFT2,
		INVENTORY_HAND_RIGHT2,

	INVENTORY_HAND_END = INVENTORY_HAND_RIGHT2,

	// INVENTORY_AVATAR - 예약번호(아직 내용없음)
	INVENTORY_AVATAR_BEGIN,
	INVENTORY_COSTUME_HAT = INVENTORY_AVATAR_BEGIN,
    INVENTORY_COSTUME_BODY1,
	INVENTORY_COSTUME_BODY2,
	INVENTORY_COSTUME_HAND,
    INVENTORY_COSTUME_FOOT,
    INVENTORY_COSTUME_FACE,
    INVENTORY_COSTUME_BACK,
	INVENTORY_AVATAR_END = INVENTORY_AVATAR_BEGIN + 15,

	// INVENTORY_BAG_TYPE
	INVENTORY_BAG0_BEGIN,
	INVENTORY_BAG0_END = INVENTORY_BAG0_BEGIN + 24,

	INVENTORY_BAG1_BEGIN,
	INVENTORY_BAG1_END = INVENTORY_BAG1_BEGIN + 24,

	INVENTORY_BAG2_BEGIN,
	INVENTORY_BAG2_END = INVENTORY_BAG2_BEGIN + 24,

	INVENTORY_BAG3_BEGIN,
	INVENTORY_BAG3_END = INVENTORY_BAG3_BEGIN + 24,

	// INVENTORY_QUEST_TYPE
	INVENTORY_QUEST_BEGIN,
	INVENTORY_QUEST_END = INVENTORY_QUEST_BEGIN + 24,

	// INVENTORY_WAREHOUSE_TYPE - 일반
	INVENTORY_WAREHOUSE0_BEGIN,
	INVENTORY_WAREHOUSE0_END = INVENTORY_WAREHOUSE0_BEGIN + 24,

	INVENTORY_WAREHOUSE1_BEGIN,
	INVENTORY_WAREHOUSE1_END = INVENTORY_WAREHOUSE1_BEGIN + 24,

	INVENTORY_WAREHOUSE2_BEGIN,
	INVENTORY_WAREHOUSE2_END = INVENTORY_WAREHOUSE2_BEGIN + 24,

	INVENTORY_WAREHOUSE3_BEGIN,
	INVENTORY_WAREHOUSE3_END = INVENTORY_WAREHOUSE3_BEGIN + 24,

	// INVENTORY_COLLECTION_TYPE - 예약번호(아직 내용없음)
	INVENTORY_COLLECTION_BEGIN,

		INVENTORY_TAROT_NORMAL_BEGIN    = INVENTORY_COLLECTION_BEGIN,
		INVENTORY_TAROT_NORMAL_END      = INVENTORY_TAROT_NORMAL_BEGIN + 21,

		INVENTORY_SPREAD_NORMAL_BEGIN,
		INVENTORY_SPREAD_NORMAL_END     = INVENTORY_SPREAD_NORMAL_BEGIN + 4,

		INVENTORY_TAROT_SILVER_BEGIN,
		INVENTORY_TAROT_SILVER_END     = INVENTORY_TAROT_SILVER_BEGIN + 21,

		INVENTORY_SPREAD_SILVER_BEGIN,
		INVENTORY_SPREAD_SILVER_END    = INVENTORY_SPREAD_SILVER_BEGIN + 4,

		INVENTORY_TAROT_GOLD_BEGIN,
		INVENTORY_TAROT_GOLD_END        = INVENTORY_TAROT_GOLD_BEGIN + 21,

		INVENTORY_SPREAD_GOLD_BEGIN,
		INVENTORY_SPREAD_GOLD_END       = INVENTORY_SPREAD_GOLD_BEGIN + 4,

		INVENTORY_TAROT_DIA_BEGIN,
		INVENTORY_TAROT_DIA_END         = INVENTORY_TAROT_DIA_BEGIN + 21,

		INVENTORY_SPREAD_DIA_BEGIN,
		INVENTORY_SPREAD_DIA_END        = INVENTORY_SPREAD_DIA_BEGIN + 4,

	INVENTORY_COLLECTION_END = INVENTORY_COLLECTION_BEGIN + 99,

	// INVENTORY_CASH_ITEM_TYPE - 캐시아이템 수령함 전용
	INVENTORY_CASH_ITEM_BEGIN,
	INVENTORY_CASH_ITEM_END = INVENTORY_CASH_ITEM_BEGIN + 100,
};

#define INVENTORY_EQUIP_NONE			(-1)
#define INVENTORY_EQUIP_BEGIN			INVENTORY_WEAR_BEGIN
#define INVENTORY_EQUIP_END				INVENTORY_COSTUME_BACK
#define LEN_INVENTORY_EQUIP				INVENTORY_EQUIP_END+1

#define INVENTORY_BAG_NONE				(-1)
#define INVENTORY_BAG_BEGIN				INVENTORY_BAG0_BEGIN
#define INVENTORY_BAG_END				INVENTORY_BAG3_END
#define INVENTORY_BAG_END_QUEST			INVENTORY_QUEST_END

#define INVENTORY_QUEST_NONE			(-1)

#define INVENTORY_CHARACTER_BEGIN		INVENTORY_WEAR_BEGIN
#define INVENTORY_CHARACTER_END			INVENTORY_BAG3_END

#define INVENTORY_WAREHOUSE_NONE		(-1)
#define INVENTORY_WAREHOUSE_BEGIN		INVENTORY_WAREHOUSE0_BEGIN
#define INVENTORY_WAREHOUSE_END			INVENTORY_WAREHOUSE3_END

#define MAX_VALID_INVENTORY				INVENTORY_COLLECTION_END
#define MAX_INVENTORY					INVENTORY_CASH_ITEM_END
#define LEN_INVENTORY					MAX_INVENTORY+1

#endif // __INVENTORY_NUMBER__


struct sInventory
{
	unsigned long  idx;								// 인벤토리 고유번호
	unsigned long  ItemIndex;						// 아이템 정의 인덱스
	unsigned short number;							// 아이템 번호 (슬롯인덱스)
	unsigned short count;							// 아이템 수량
	unsigned char  enhanced;						// 아이템 강화단계 (0~15까지)
	unsigned long  cardSlot1;						// 카드슬롯1
	unsigned long  cardSlot2;						// 카드슬롯2
	unsigned long  cardSlot3;						// 카드슬롯3
	unsigned long  cardSlot4;						// 카드슬롯4
	unsigned long  cardSlot5;						// 카드슬롯5
	unsigned char  seal;							// 아이템 봉인 (0봉인해제/1봉인됨)
	unsigned char  cash;							// 캐시 아이템 (0일반/1캐시)
};

struct sInventoryCooltime
{
	unsigned long ItemIndex;						// 아이템 정의 인덱스
	unsigned long cooltime2;						// 그룹 쿨타임 인덱스
	unsigned long cooltime;							// 쿨타임 남은시간
	unsigned long cooltime1;						// 쿨타임 적용시간
};


#ifndef __ITEM_ABILITY_OPT__

// 순서는 기획서 참조 - 기획서 기준 정렬
enum ITEM_ABILITY_OPT
{
	ITEM_OPT_NONE=0,						// 사용안함
	ITEM_OPT_STR,							// 힘
	ITEM_OPT_DEX,							// 민첩
	ITEM_OPT_CON,							// 체력
	ITEM_OPT_INT,							// 지능
	ITEM_OPT_WIS,							// 지혜
	ITEM_OPT_ALL,							// 모두(힘,민첩,체력,지능,지혜)

	ITEM_OPT_PLUS_PHYSIC_ATTACK=101,		// 물리공격력
	ITEM_OPT_PLUS_MAGIC_ATTACK,				// 마법공격력
	ITEM_OPT_PLUS_PHYSIC_DEFENSE,			// 물리방어력
	ITEM_OPT_PLUS_MAGIC_DEFENSE,			// 마법방어력
	ITEM_OPT_PLUS_PHYSIC_ATTACK_RATE,		// 물리명중률
	ITEM_OPT_PLUS_MAGIC_ATTACK_RATE,		// 마법명중률
	ITEM_OPT_PLUS_PHYSIC_CRITICAL,			// 물리크리티컬
	ITEM_OPT_PLUS_MAGIC_CRITICAL,			// 마법크리티컬
	ITEM_OPT_PLUS_DOD,						// 회피
	ITEM_OPT_PLUS_WEI,						// 무게
	ITEM_OPT_PLUS_MAX_HP,					// 최대량 HP
	ITEM_OPT_PLUS_MAX_MP,					// 최대량 MP
	ITEM_OPT_PLUS_MOV,						// 이동
	ITEM_OPT_PLUS_HP,						// HP 회복
	ITEM_OPT_PLUS_MP,						// MP 회복
	ITEM_OPT_PLUS_ATTACK_SPEED,				// 일반공격속도
	ITEM_OPT_PLUS_SKILL_COOLTIME_SPEED,		// 스킬쿨타임속도
	ITEM_OPT_PLUS_SHORT_RANGE,				// 근거리사정거리
	ITEM_OPT_PLUS_DISTANCE_RANGE,			// 원거리사정거리
	ITEM_OPT_PLUS_HEAL,						// 힐 회복량
	ITEM_OPT_PLUS_HP_RECOVERTY,				// HP 회복시 증가량
	ITEM_OPT_PLUS_ALL_ATTACK,				// 모든 공격력
	ITEM_OPT_PLUS_ALL_DEFENSE,				// 모든 방어력
	ITEM_OPT_PLUS_PHYSIC_CRITICAL_DEC,		// 물리크리티컬 감소
	ITEM_OPT_PLUS_MAGIC_CRITICAL_DEC,		// 마법크리티컬 감소
	ITEM_OPT_PLUS_RESISTANCE,				// 저항력

	ITEM_OPT_PERCENT_PHYSIC_ATTACK=201,		// 물리공격력
	ITEM_OPT_PERCENT_MAGIC_ATTACK,			// 마법공격력
	ITEM_OPT_PERCENT_PHYSIC_DEFENSE,		// 물리방어력
	ITEM_OPT_PERCENT_MAGIC_DEFENSE,			// 마법방어력
	ITEM_OPT_PERCENT_PHYSIC_ATTACK_RATE,	// 물리명중률
	ITEM_OPT_PERCENT_MAGIC_ATTACK_RATE,		// 마법명중률
	ITEM_OPT_PERCENT_PHYSIC_CRITICAL,		// 물리크리티컬
	ITEM_OPT_PERCENT_MAGIC_CRITICAL,		// 마법크리티컬
	ITEM_OPT_PERCENT_DOD,					// 회피
	ITEM_OPT_PERCENT_WEI,					// 무게
	ITEM_OPT_PERCENT_MAX_HP,				// 최대량 HP
	ITEM_OPT_PERCENT_MAX_MP,				// 최대량 MP
	ITEM_OPT_PERCENT_MOV,					// 이동
	ITEM_OPT_PERCENT_HP,					// HP 회복
	ITEM_OPT_PERCENT_MP,					// MP 회복
	ITEM_OPT_PERCENT_ATTACK_SPEED,			// 일반공격속도
	ITEM_OPT_PERCENT_SKILL_COOLTIME_SPEED,	// 스킬쿨타임속도
	ITEM_OPT_PERCENT_SHORT_RANGE,			// 근거리사정거리
	ITEM_OPT_PERCENT_DISTANCE_RANGE,		// 원거리사정거리
	ITEM_OPT_PERCENT_HEAL,					// 힐 회복량
	ITEM_OPT_PERCENT_HP_RECOVERTY,			// HP 회복시 증가량
	ITEM_OPT_PERCENT_ALL_ATTACK,			// 모든 공격력
	ITEM_OPT_PERCENT_ALL_DEFENSE,			// 모든 방어력
	ITEM_OPT_PERCENT_PHYSIC_CRITICAL_DEC,	// 물리크리티컬 감소
	ITEM_OPT_PERCENT_MAGIC_CRITICAL_DEC,	// 마법크리티컬 감소
	ITEM_OPT_PERCENT_RESISTANCE,			// 저항력

	ITEM_OPT_PLUS_DRAIN_HP = 301,			// HP 흡수
	ITEM_OPT_PLUS_DRAIN_MP,					// MP 흡수
	ITEM_OPT_PLUS_DAMAGE_DEC,				// 데미지 흡수/감소
	ITEM_OPT_PLUS_DAMAGE_REF,				// 데미지 반사

	ITEM_OPT_PERCENT_DRAIN_HP = 401,		// HP 흡수
	ITEM_OPT_PERCENT_DRAIN_MP,				// MP 흡수
	ITEM_OPT_PERCENT_DAMAGE_DEC,			// 데미지 흡수/감소
	ITEM_OPT_PERCENT_DAMAGE_REF,			// 데미지 반사
};

#endif // __ITEM_ABILITY_OPT__


// 장착한 아이템의 총합 어빌리티 - 스크립트 기준 정렬
struct sEquipAbility
{
	// 기본 값 적용.
	short mPhysicDefense;							// 물리:방어력
	short mMagicDefense;							// 마법:방어력
	short mAttackRange;								// 공격 거리
	short mAttackSpeed;								// 공격 속도

	short mPhysicMinAttack;							// 물리:최소공격력
	short mPhysicMaxAttack;							// 물리:최대공격력
	short mMagicMinAttack;							// 마법:최소공격력
	short mMagicMaxAttack;							// 마법:최대공격력
	short mHealValue;								// 힐 회복값

	// 옵션 값 적용.
	short mOptStr;									// 증가:힘
	short mOptDex;									// 증가:민첩
	short mOptCon;									// 증가:체력
	short mOptInt;									// 증가:지능
	short mOptWis;									// 증가:지혜

	// 옵션 값 - Plus
	short mOptPlusPhysicAttack;						// 물리공격력
	short mOptPlusMagicAttack;						// 마법공격력
	short mOptPlusPhysicDefense;					// 물리방어력
	short mOptPlusMagicDefense;						// 마법방어력
	short mOptPlusPhysicAttackRate;					// 물리명중률
	short mOptPlusMagicAttackRate;					// 마법명중률
	short mOptPlusPhysicCritical;					// 물리크리티컬
	short mOptPlusMagicCritical;					// 마법크리티컬
	short mOptPlusDod;								// 회피
	short mOptPlusWei;								// 무게
	short mOptPlusMaxHp;							// 최대량 HP
	short mOptPlusMaxMp;							// 최대량 MP
	short mOptPlusMov;								// 이동
	short mOptPlusHp;								// HP 회복
	short mOptPlusMp;								// MP 회복
	short mOptPlusAttackSpeed;						// 일반공격속도
	short mOptPlusSkillCooltimeSpeed;				// 스킬쿨타임속도
	short mOptPlusShortRange;						// 근거리사정거리
	short mOptPlusDistanceRange;					// 원거리사정거리
	short mOptPlusHeal;								// 힐 회복량
	short mOptPlusHpReceive;						// HP 받는 회복시 증가량
	//short mOptPlusAllAttack;						// 모든 공격력
	//short mOptPlusAllDefense;						// 모든 방어력
	short mOptPlusPhysicCriticalDec;				// 물리크리티컬 감소
	short mOptPlusMagicCriticalDec;					// 마법크리티컬 감소
	short mOptPlusResistance;						// 저항력

	// 옵션 값 - Percent
	short mOptPercentPhysicAttack;					// 물리공격력
	short mOptPercentMagicAttack;					// 마법공격력
	short mOptPercentPhysicDefense;					// 물리방어력
	short mOptPercentMagicDefense;					// 마법방어력
	short mOptPercentPhysicAttackRate;				// 물리명중률
	short mOptPercentMagicAttackRate;				// 마법명중률
	short mOptPercentPhysicCritical;				// 물리크리티컬
	short mOptPercentMagicCritical;					// 마법크리티컬
	short mOptPercentDod;							// 회피
	short mOptPercentWei;							// 무게
	short mOptPercentMaxHp;							// 최대량 HP
	short mOptPercentMaxMp;							// 최대량 MP
	short mOptPercentMov;							// 이동
	short mOptPercentHp;							// HP 회복
	short mOptPercentMp;							// MP 회복
	short mOptPercentAttackSpeed;					// 일반공격속도
	short mOptPercentSkillCooltimeSpeed;			// 스킬쿨타임속도
	short mOptPercentShortRange;					// 근거리사정거리
	short mOptPercentDistanceRange;					// 원거리사정거리
	short mOptPercentHeal;							// 힐 회복량
	short mOptPercentHpReceive;						// HP 받는 회복시 증가량
	//short mOptPercentAllAttack;						// 모든 공격력
	//short mOptPercentAllDefense;					// 모든 방어력
	short mOptPercentPhysicCriticalDec;				// 물리크리티컬 감소
	short mOptPercentMagicCriticalDec;				// 마법크리티컬 감소
	short mOptPercentResistance;					// 저항력

	// 옵션 값 - Plus Etc
	short mOptPlusDrainHp;							// HP 흡수
	short mOptPlusDrainMp;							// MP 흡수
	short mOptPlusDamageDec;						// 데미지 흡수/감소
	short mOptPlusDamageRef;						// 데미지 반사

	// 옵션 값 - Percent Etc
	short mOptPercentDrainHp;						// HP 흡수
	short mOptPercentDrainMp;						// MP 흡수
	short mOptPercentDamageDec;						// 데미지 흡수/감소
	short mOptPercentDamageRef;						// 데미지 반사
};

// 분해한 아이템의 보상 아이템
#define MAX_DISJOINT_ITEM	6

struct eDisjointItem
{
	long  itemDefineIndex;
	short inventoryCount;
};

struct MSG_REQ_INVENTORY : public MSGROOT
{};
struct MSG_RES_INVENTORY : public MSGROOT
{
	eItemActiveWeapon activeWeapon;					// 활성 무기정보(0 Front, 1Back).
	long              RowCount;						// 갯수
	sInventory        Inventory[1];					// 인벤토리
};

struct MSG_REQ_INVENTORY_COOLTIME : public MSGROOT
{};
struct MSG_RES_INVENTORY_COOLTIME : public MSGROOT
{
	long               RowCount;					// 갯수
	sInventoryCooltime InventoryCooltime[1];		// 인벤토리 쿨타임
};

/*-- 아이템 이동(인벤토리)
*/
#define ERROR_ITEM_MOV_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_MOV_INVENTORY_FAIL			1   // 실패
#define ERROR_ITEM_MOV_INVENTORY_NOTYET			2   // DB처리중
#define ERROR_ITEM_MOV_INVENTORY_MERGE			3	// 병합중
#define ERROR_ITEM_MOV_INVENTORY_LICENSE        4   // 미인증 아이템 사용
#define ERROR_ITEM_MOV_INVENTORY_MAX_ITEM       5   // 최대 보유갯수 갯수 초과
#define ERROR_ITEM_MOV_INVENTORY_STATE          6   // 이동 불가능 상태

struct MSG_REQ_ITEM_MOV_INVENTORY : public MSGROOT
{
	unsigned short number1;							// [in] 이동 전
	unsigned short number2;							// [in] 이동 후
};
struct MSG_RES_ITEM_MOV_INVENTORY : public MSG_ERROR
{};


#define ERROR_ITEM_MERGE_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_MERGE_INVENTORY_FAIL			1   // 실패

struct MSG_RES_ITEM_MERGE_INVENTORY : public MSG_ERROR
{};

/*-- 아이템 사용(인벤토리)
*/
#define ERROR_ITEM_USE_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_USE_INVENTORY_FAIL			1	// 실패
#define ERROR_ITEM_USE_INVENTORY_NOTYET			2   // DB처리중
#define ERROR_ITEM_USE_INVENTORY_COOLTIME		3	// 쿨타임 적용중
#define ERROR_ITEM_USE_INVENTORY_TITLE			4	// 호칭 보유중
#define ERROR_ITEM_USE_INVENTORY_SLOT			5	// 슬롯이 이미 최대로 확장됨
#define ERROR_ITEM_USE_SKILL_SYSTEM				6	// 시스템에러
#define ERROR_ITEM_USE_SKILL_STATE				7	// 대상의 상태가 안맞음
#define ERROR_ITEM_USE_SKILL_GROUPORDER			8	// 효력그룹이 낮음
#define ERROR_ITEM_USE_INVENTORY_LICENSE		9	// 미인증 아이템 사용
#define ERROR_ITEM_USE_EXP_RECOVER				10	// 경험치 복구 사용 오류
#define ERROR_ITEM_USE_SKILL_DUPLICATE			11	// 캐시 중복사용 불가능
#define ERROR_ITEM_USE_SKILL_PCOVER				12	// 일반 버프가 PC버프를 덮을수 없음
#define ERROR_ITEM_USE_SKILL_PCSCRIPT			13	// PC방 버프끼리 중복불가 설정 스크립트 오류


struct MSG_REQ_ITEM_USE_INVENTORY : public MSGROOT
{
	unsigned long  targetType;						// 대상 타  입 'sObject 참조'
	unsigned long  targetIndex;						// 대상 인덱스 "
	unsigned short number;							// 인벤토리 번호
};
struct MSG_RES_ITEM_USE_INVENTORY : public MSG_ERROR
{};


/*-- 아이템 삭제(인벤토리)
*/
#define ERROR_ITEM_DEL_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_DEL_INVENTORY_FAIL			1   // 실패
#define ERROR_ITEM_DEL_INVENTORY_NOTYET			2   // DB처리중
#define ERROR_ITEM_DEL_INVENTORY_ITEMPWD_CHECK	3   // 아이템 인증번호 확인해야 함

struct MSG_REQ_ITEM_DEL_INVENTORY : public MSGROOT
{
	unsigned short number;							// 인벤토리 번호
};
struct MSG_RES_ITEM_DEL_INVENTORY : public MSG_ERROR
{};


/*-- 아이템 스위칭/착용무기만 적용(인벤토리)
*/
#define ERROR_ITEM_SWI_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_SWI_INVENTORY_FAIL			1   // 실패
#define ERROR_ITEM_SWI_INVENTORY_NOTYET			2   // DB처리중

struct MSG_REQ_ITEM_SWI_INVENTORY : MSGROOT
{};
struct MSG_RES_ITEM_SWI_INVENTORY : MSG_ERROR
{
	eItemActiveWeapon activeWeapon;					// 활성 무기정보(0 Front, 1Back).
};


/*-- 아이템 나누기(인벤토리)
*/
#define ERROR_ITEM_DIVIDE_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_DIVIDE_INVENTORY_FAIL		1	// 실패
#define ERROR_ITEM_DIVIDE_INVENTORY_NOTYET		2   // DB처리중


struct MSG_REQ_ITEM_DIVIDE_INVENTORY : MSGROOT
{
	unsigned short number1;							// 인벤토리 번호(원본)
	unsigned short number2;							// 인벤토리 번호(대상)
	unsigned short divide;							// 누눌수량
};

struct MSG_RES_ITEM_DIVIDE_INVENTORY : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_DIVIDE_INVENTORY) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


/*-- 아이템 강화(인벤토리)
*/

const unsigned long ENHANCE_DELAYTIME = 5000;	// 5초

/// IsItemEnhanced에서 사용
/// START와 END 오류와 같은 번호사용한다.
#define ERROR_ITEM_ENHANCED_CHECK_SUCCESS               0
#define ERROR_ITEM_ENHANCED_CHECK_FAIL                  1
#define ERROR_ITEM_ENHANCED_CHECK_LICENSE               3   // 미인증 아이템 사용
#define ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_SAFE   4   // 장비 레벨(보호카드)
#define ERROR_ITEM_ENHANCED_CHECK_EQUIP_LEVEL_BY_CARD   5   // 장비 레벨(강화카드)

#define ERROR_ITEM_ENHANCED_START_SUCCESS		        0	// 성공
#define ERROR_ITEM_ENHANCED_START_FAIL			        1	// 실패
#define ERROR_ITEM_ENHANCED_START_STATE			        2
#define ERROR_ITEM_ENHANCED_START_LICENSE               3   // 미인증 아이템 사용
#define ERROR_ITEM_ENHANCED_START_EQUIP_LEVEL_BY_SAFE   4   // 장비 레벨(보호카드)
#define ERROR_ITEM_ENHANCED_START_EQUIP_LEVEL_BY_CARD   5   // 장비 레벨(강화카드)

struct MSG_REQ_ITEM_ENHANCED_START : public MSGROOT
{
	unsigned short	number1;							// 인벤토리 번호(강화아이템)
	unsigned short	number2;							// 인벤토리 번호(강화카드)
	unsigned short	number3;							// 인벤토리 번호(강화보호카드)
};

struct MSG_SYN_ITEM_ENHANCED_START : public MSGROOT
{
	unsigned long	characterIdx;
};

#define ERROR_ITEM_ENHANCED_END_SUCCESS			        0	// 성공
#define ERROR_ITEM_ENHANCED_END_FAIL			        1	// 실패
#define ERROR_ITEM_ENHANCED_END_NOTYET		            2	// DB처리중
#define ERROR_ITEM_ENHANCED_END_LICENSE                 3   // 미인증 아이템 사용
#define ERROR_ITEM_ENHANCED_END_EQUIP_LEVEL_BY_SAFE		4	// 장비 레벨(보호카드)
#define ERROR_ITEM_ENHANCED_END_EQUIP_LEVEL_BY_CARD		5	// 장비 레벨(강화카드)

struct MSG_SYN_ITEM_ENHANCED_END : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리(0강화장비/1강화카드/2강화보호카드)

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_ITEM_ENHANCED_END) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};

#define ERROR_ITEM_ENHANCED_SYN_SUCCESS			0	// 성공     - RESULT_INVENTORY_ENHANCED_SUCCESS
#define ERROR_ITEM_ENHANCED_SYN_DOWN			1	// 다운     - RESULT_INVENTORY_ENHANCED_DOWN
#define ERROR_ITEM_ENHANCED_SYN_DESTROY 		2	// 파괴     - RESULT_INVENTORY_ENHANCED_DESTROY
#define ERROR_ITEM_ENHANCED_SYN_KEEP			3	// 변화없음 - RESULT_INVENTORY_ENHANCED_KEEP
#define ERROR_ITEM_ENHANCED_SYN_ENDFAIL			4	// 실패

struct MSG_SYN_ITEM_ENHANCED_OTHEREND : MSG_ERROR
{
	unsigned long	characterIdx;
};

#define ERROR_ITEM_ENHANCED_CANCEL_SUCCESS    0
#define ERROR_ITEM_ENHANCED_CANCEL_DBWORK     1

struct MSG_RES_ITEM_ENHANCED_CANCEL : MSG_ERROR
{};

struct MSG_SYN_ITEM_ENHANCED_CANCEL : MSGROOT
{
	unsigned long	characterIdx;
};

// 강제 취소
struct MSG_SYN_ITEM_ENHANCED_CANCELFORCE : MSGROOT
{};

/*-- 아이템 분해(인벤토리)
*/
#define ERROR_ITEM_DISJOINT_INVENTORY_SUCCESS	    0	// 성공
#define ERROR_ITEM_DISJOINT_INVENTORY_FAIL		    1	// 실패
#define ERROR_ITEM_DISJOINT_INVENTORY_NOTYET	    2	// DB처리중
#define ERROR_ITEM_DISJOINT_INVENTORY_MAX_ITEM	    3	// 최대 보유갯수 갯수 초과
#define ERROR_ITEM_DISJOINT_INVENTORY_LICENSE	    4	// 미인증 아이템 사용
#define ERROR_ITEM_DISJOINT_INVENTORY_ITEMPWD_CHECK	5	// 아이템 인증번호 확인해야 함


struct MSG_REQ_ITEM_DISJOINT_INVENTORY : MSGROOT
{
	unsigned short number;							// 인벤토리 번호(원본)
};
struct MSG_RES_ITEM_DISJOINT_INVENTORY : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_DISJOINT_INVENTORY) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


/*-- 아이템 카드장착(인벤토리)
*/
#define ERROR_ITEM_PUT_CARD_INVENTORY_SUCCESS	0	// 성공
#define ERROR_ITEM_PUT_CARD_INVENTORY_FAIL		1	// 실패
#define ERROR_ITEM_PUT_CARD_INVENTORY_NOTYET	2	// DB처리중
#define ERROR_ITEM_PUT_CARD_INVENTORY_LICENSE	3	// 미인증 아이템 사용

struct MSG_REQ_ITEM_PUT_CARD_INVENTORY : MSGROOT
{
	unsigned short number1;							// 인벤토리 번호(장비)
	unsigned char  cardSlot;						// 인벤토리 카드슬롯 번호(1~5)
	unsigned short number2;							// 인벤토리 번호(카드)
};
struct MSG_RES_ITEM_PUT_CARD_INVENTORY : MSG_ERROR
{
	sInventory Inventory;							// 인벤토리
};


/*-- 아이템 체인지(인벤토리)
*/
#define ERROR_ITEM_CHANGE_INVENTORY_SUCCESS		0	// 성공
#define ERROR_ITEM_CHANGE_INVENTORY_FAIL		1	// 실패
#define ERROR_ITEM_CHANGE_INVENTORY_NOTYET		2	// DB처리중
#define ERROR_ITEM_CHANGE_INVENTORY_MAX_ITEM	3	// 최대 보유갯수 갯수 초과
#define ERROR_ITEM_CHANGE_INVENTORY_HAVE_MONEY	4	// 최대 보유금액 초과
#define ERROR_ITEM_CHANGE_INVENTORY_LICENSE     5   // 미인증 아이템 사용 

struct MSG_REQ_ITEM_CHANGE_INVENTORY : MSGROOT
{
	unsigned short number;							// 인벤토리 번호
};
struct MSG_RES_ITEM_CHANGE_INVENTORY : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_CHANGE_INVENTORY) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


/*-- 거래-사용자간
*/
#define ERROR_ITEM_EXCHANGE_ASK_SUCCESS			0	// 성공 - 신청후 대기중
#define ERROR_ITEM_EXCHANGE_ASK_FAIL			1	// 실패 - 대상이 없거나 오류인경우
#define ERROR_ITEM_EXCHANGE_ASK_ERROR			2	// 실패 - 거래상태 오류
#define ERROR_ITEM_EXCHANGE_ASK_DISTANCE		3	// 실패 - 거리
#define ERROR_ITEM_EXCHANGE_ASK_OPTIONREFUSE	4	// 실패 - 거부상태
#define ERROR_ITEM_EXCHANGE_ASK_TARGETOPTIONREFUSE	5	// 실패 - 상대방 거부상태
#define ERROR_ITEM_EXCHANGE_ASK_REPLAY				6	// 답변요청
#define ERROR_ITEM_EXCHANGE_ASK_TARGETERROR			7	// 실패 - 대상의 상태 오류
#define ERROR_ITEM_EXCHANGE_ASK_ITEMPWD_CHECK		8	// 실패 - 아이템 인증번호 확인해야 함

struct MSG_REQ_ITEM_EXCHANGE_ASK : MSGROOT
{
	sObject mTarget;								// 거래 상대 캐릭터
};
struct MSG_RES_ITEM_EXCHANGE_ASK : MSG_ERROR
{
	unsigned long characterIdx;						// 거래 신청 캐릭터 - ERROR_ITEM_EXCHANGE_ASK_REPLAY 일때만...
};


#define TYPE_ITEM_EXCHANGE_REP_ACCEPT			0	// 승락
#define TYPE_ITEM_EXCHANGE_REP_REJECT			1	// 거절
#define TYPE_ITEM_EXCHANGE_REP_REJECTTIME		2	// 거절 - 응답시간 초과

#define ERROR_ITEM_EXCHANGE_REP_SUCCESS			0	// 성공 또는 요청 승락
#define ERROR_ITEM_EXCHANGE_REP_REFUSE			1	// 거절
#define ERROR_ITEM_EXCHANGE_REP_TARGETREFUSE	2	// 상대방 거절
#define ERROR_ITEM_EXCHANGE_REP_FAIL			3	// 상대방의 상태 오류
#define ERROR_ITEM_EXCHANGE_REP_ERROR			4	// 내 상태 오류
#define ERROR_ITEM_EXCHANGE_REP_TARGETERROR		5	// 실패 - 대상의 상태 오류
#define ERROR_ITEM_EXCHANGE_REP_NOTEXIST		6	// 대상이 없음
#define ERROR_ITEM_EXCHANGE_REP_OPTIONREFUSE		7	// 내가 거부상태임
#define ERROR_ITEM_EXCHANGE_REP_TARGETOPTIONREFUSE	8	// 대상이 거부상태
#define ERROR_ITEM_EXCHANGE_REP_DISTANCE			9	// 실패 - 거리
#define ERROR_ITEM_EXCHANGE_REP_TIME			10	// 실패 - 응답시간 초과
#define ERROR_ITEM_EXCHANGE_REP_ITEMPWD_CHECK	11	// 실패 - 아이템 인증번호 확인해야 함


struct MSG_REQ_ITEM_EXCHANGE_REP : MSGROOT
{
	BYTE type;									// 답변
};
struct MSG_RES_ITEM_EXCHANGE_REP : MSG_ERROR
{};


struct MSG_RES_ITEM_EXCHANGE_BEGIN : MSGROOT
{};


#define TYPE_ITEM_EXCHANGE_ADD_ITEM				0	// 거래아이템
#define TYPE_ITEM_EXCHANGE_ADD_MONEY			1	// 거래금  액

#define ERROR_ITEM_EXCHANGE_ADD_SUCCESS			0	// 성공
#define ERROR_ITEM_EXCHANGE_ADD_FAIL			1	// 실패
#define ERROR_ITEM_EXCHANGE_ADD_TRADE_MONEY     2	// 최대 보유금액 초과
#define ERROR_ITEM_EXCHANGE_ADD_EXCHANGE        3   // 거래불가 아이템

struct MSG_REQ_ITEM_EXCHANGE_ADD : MSGROOT
{
	BYTE type;									// 종류(아이템/금액)
	union
	{
		unsigned short number;					// 인벤토리 번호 (슬롯인덱스)
		unsigned long  money;					// 금액
	};
};
struct MSG_RES_ITEM_EXCHANGE_ADD : MSG_ERROR
{};
struct MSG_SYN_ITEM_EXCHANGE_ADD : MSGROOT
{
	BYTE type;									// 종류(아이템/금액)
	union
	{
		sInventory     inventory;				// 인벤토리
		unsigned long  money;					// 금액
	};
};


#define ERROR_ITEM_EXCHANGE_DEL_SUCCESS			0	// 성공
#define ERROR_ITEM_EXCHANGE_DEL_FAIL			1	// 실패

struct MSG_REQ_ITEM_EXCHANGE_DEL : MSGROOT
{
	unsigned short number;						// 인벤토리 번호 (슬롯인덱스)
};
struct MSG_RES_ITEM_EXCHANGE_DEL : MSG_ERROR
{};
struct MSG_SYN_ITEM_EXCHANGE_DEL : MSGROOT
{
	sInventory inventory;						// 인벤토리
};


#define ERROR_ITEM_EXCHANGE_OK_SUCCESS			0	// 성공
#define ERROR_ITEM_EXCHANGE_OK_FAIL				1	// 실패
#define ERROR_ITEM_EXCHANGE_OK_PLAYER			2	// 대상

struct MSG_REQ_ITEM_EXCHANGE_OK : MSGROOT
{};
struct MSG_RES_ITEM_EXCHANGE_OK : MSG_ERROR
{};


#define ERROR_ITEM_EXCHANGE_RETRY_SUCCESS		0	// 성공
#define ERROR_ITEM_EXCHANGE_RETRY_FAIL			1	// 실패
#define ERROR_ITEM_EXCHANGE_RETRY_PLAYER		2	// 대상

struct MSG_REQ_ITEM_EXCHANGE_RETRY : MSGROOT
{};
struct MSG_RES_ITEM_EXCHANGE_RETRY : MSG_ERROR
{};


#define ERROR_ITEM_EXCHANGE_CANCEL_SUCCESS		0	// 성공
#define ERROR_ITEM_EXCHANGE_CANCEL_FAIL			1	// 실패
#define ERROR_ITEM_EXCHANGE_CANCEL_PLAYER		2	// 대상

struct MSG_REQ_ITEM_EXCHANGE_CANCEL : MSGROOT
{};
struct MSG_RES_ITEM_EXCHANGE_CANCEL : MSG_ERROR
{};


#define ERROR_ITEM_EXCHANGE_END_SUCCESS			0	// 성공
#define ERROR_ITEM_EXCHANGE_END_FAIL			1	// 실패
#define ERROR_ITEM_EXCHANGE_END_INVENTORY		2	// 인벤토리 오류
#define ERROR_ITEM_EXCHANGE_END_WAIT			3	// 대기
#define ERROR_ITEM_EXCHANGE_END_MYMONEY_FAIL	4	// 거래 성립 후 돈이 한도 초과된 경우 
#define ERROR_ITEM_EXCHANGE_END_THATMONEY_FAIL	5	// 거래 성립 후 돈이 한도 초과된 경우 
#define ERROR_ITEM_EXCHANGE_END_MY_MAX_ITEM	    6	// 최대 보유 수량
#define ERROR_ITEM_EXCHANGE_END_THAT_MAX_ITEM	7	// 최대 보유 수량


struct MSG_REQ_ITEM_EXCHANGE_END : MSGROOT
{};
struct MSG_RES_ITEM_EXCHANGE_END : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_EXCHANGE_END) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


/*-- 노점-판매
*/
#define ERROR_ITEM_STALL_SELL_OPEN_SUCCESS		    0	// 성공
#define ERROR_ITEM_STALL_SELL_OPEN_FAIL			    1	// 실패
#define ERROR_ITEM_STALL_SELL_OPEN_EXIST		    2	// 이미 오픈중
#define ERROR_ITEM_STALL_SELL_OPEN_ITEMPWD_CHECK	3	// 아이템 인증번호 확인해야 함

struct MSG_REQ_ITEM_STALL_SELL_OPEN : MSGROOT
{
	wchar_t title[ MAX_STALL_TITLE_LEN ];			// 노점명
};
struct MSG_RES_ITEM_STALL_SELL_OPEN : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_OPEN : MSGROOT
{
	unsigned long characterIdx;						// 캐릭터 고유번호
	wchar_t       title[ MAX_STALL_TITLE_LEN ];		// 노점명
};


#define ERROR_ITEM_STALL_SELL_CLOSE_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_CLOSE_FAIL		1	// 실패

struct MSG_REQ_ITEM_STALL_SELL_CLOSE : MSGROOT
{};
struct MSG_RES_ITEM_STALL_SELL_CLOSE : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_CLOSE : MSGROOT
{
	unsigned long characterIdx;						// 캐릭터 고유번호
};


#define ERROR_ITEM_STALL_SELL_RENAME_SUCCESS	0	// 성공
#define ERROR_ITEM_STALL_SELL_RENAME_FAIL		1	// 실패

struct MSG_REQ_ITEM_STALL_SELL_RENAME : MSGROOT
{
	wchar_t title[ MAX_STALL_TITLE_LEN ];			// 노점명
};
struct MSG_RES_ITEM_STALL_SELL_RENAME : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_RENAME : MSGROOT
{
	unsigned long characterIdx;						// 캐릭터 고유번호
	wchar_t       title[ MAX_STALL_TITLE_LEN ];		// 노점명
};


#define ERROR_ITEM_STALL_SELL_ADD_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_ADD_FAIL			1	// 실패
#define ERROR_ITEM_STALL_SELL_ADD_TRADE_MONEY   2	// 최대 보유금액 초과
#define ERROR_ITEM_STALL_SELL_ADD_EXCHANGE      3   // 거래불가 아이템

struct MSG_REQ_ITEM_STALL_SELL_ADD : MSGROOT
{
	unsigned short number;							// 인벤토리 번호(슬롯넘버)
	unsigned long  price;							// 판매가격
};
struct MSG_RES_ITEM_STALL_SELL_ADD : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_ADD : MSGROOT
{
	sInventory inventory;							// 인벤토리
	unsigned long price;							// 판매가격
};


#define ERROR_ITEM_STALL_SELL_DEL_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_DEL_FAIL			1	// 실패

struct MSG_REQ_ITEM_STALL_SELL_DEL : MSGROOT
{
	unsigned short number;							// 인벤토리 번호(슬롯넘버)
};
struct MSG_RES_ITEM_STALL_SELL_DEL : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_DEL : MSGROOT
{
	sInventory inventory;							// 인벤토리
};


#define ERROR_ITEM_STALL_SELL_MOD_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_MOD_FAIL			1	// 실패

struct MSG_REQ_ITEM_STALL_SELL_MOD : MSGROOT
{
	unsigned short number;							// 인벤토리 번호(슬롯넘버)
};
struct MSG_RES_ITEM_STALL_SELL_MOD : MSG_ERROR
{};
struct MSG_SYN_ITEM_STALL_SELL_MOD : MSGROOT
{
	sInventory inventory;							// 인벤토리
};

struct MSG_RES_ITEM_STALL_SELL_COM : MSGROOT
{
	wchar_t characterName[ 17 ];					// 구입자 이름
	sInventory inventory;							// 인벤토리
};

struct MSG_SYN_ITEM_STALL_SELL_DAT : MSGROOT
{
	sInventory    inventory;						// 인벤토리
	unsigned long price;							// 가격
};


#define ERROR_ITEM_STALL_SELL_JOIN_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_JOIN_FAIL			1	// 실패
#define ERROR_ITEM_STALL_SELL_JOIN_OVER			2	// 실패-인원제한

struct sStallSellData
{
	eStallSellItemStatus status;					// 상태
	sInventory           inventory;					// 인벤토리
	unsigned long        price;						// 가격
};

struct MSG_REQ_ITEM_STALL_SELL_JOIN : MSGROOT
{
	sObject mTarget;								// 타켓
};
struct MSG_RES_ITEM_STALL_SELL_JOIN : MSG_ERROR
{
	wchar_t        title[ MAX_STALL_TITLE_LEN ];	// 노점명

	BYTE           rowCount;						// 영향받은 행 수
	sStallSellData stallSellData[1];

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_STALL_SELL_JOIN) - sizeof(stallSellData) + (rowCount * sizeof(sStallSellData)); }
};


#define ERROR_ITEM_STALL_SELL_LEAVE_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_LEAVE_FAIL		1	// 실패
#define ERROR_ITEM_STALL_SELL_LEAVE_CLOSED		2	// 닫힘
#define ERROR_ITEM_STALL_SELL_LEAVE_MAX_ITEM	3	// 최대 보유갯수 갯수 초과
#define ERROR_ITEM_STALL_SELL_LEAVE_HAVE_MONEY	4	// 최대 보유금액 초과

struct MSG_REQ_ITEM_STALL_SELL_LEAVE : MSGROOT
{};
struct MSG_RES_ITEM_STALL_SELL_LEAVE : MSG_ERROR
{};


#define ERROR_ITEM_STALL_SELL_GET_SUCCESS		0	// 성공
#define ERROR_ITEM_STALL_SELL_GET_FAIL			1	// 실패

struct MSG_REQ_ITEM_STALL_SELL_GET : MSGROOT
{
	unsigned short number;							// 구매번호
	unsigned short count;							// 수량
};
struct MSG_RES_ITEM_STALL_SELL_GET : MSG_ERROR
{
	BYTE       RowCount;							// 영향받은 행 수
	sInventory Inventory;							// 인벤토리
};


/*-- 아이템 드롭&습득
*/
struct sItemDrop
{
	unsigned long idx;								// 고유번호
	float         xPos;								// X 위치
	float         yPos;								// Y 위치
};
struct sItemData
{
	unsigned long  itemDefineIndex;					// 아이템 정의 인덱스
	unsigned short count;							// 수량
};

// NM_ITEM_DROP_IN_SYN & NM_ITEM_SIGHT_IN_RES 참조
struct MSG_ITEM_INFO : MSGROOT
{
	unsigned long owner[ MAX_ITEM_OWNER ];			// 드롭 아이템 소유자
	unsigned long timeToOwner;						// 드롭 아이템 소유자의 남은시간(1/1000s)

	unsigned long party[ MAX_ITEM_PARTY ];			// 드롭 아이템 파티
	unsigned long timeToParty;						// 드롭 아이템 파티의   남은시간(1/1000s)

	unsigned char rare;								// 드롭 아이템 레어수준

	sItemDrop     itemDrop;              			// 드롭 아이템 정보
};
// NM_ITEM_DROP_OUT_SYN & NM_ITEM_SIGHT_OUT_RES 참조
struct MSG_ITEMIDX : MSGROOT
{
	unsigned long idx;								// 고유번호
};


#define ERROR_ITEM_GET_OPEN_SUCCESS		0			// 성공
#define ERROR_ITEM_GET_OPEN_FAIL		1			// 실패
#define ERROR_ITEM_GET_OPEN_OWNER       2           // 실패 - 소유권 오류
#define ERROR_ITEM_GET_OPEN_PARTY       3           // 실패 - 소유권 오류
#define ERROR_ITEM_GET_OPEN_DISTANCE    4           // 실패 - 거  리 오류
#define ERROR_ITEM_GET_OPEN_EXIST       5           // 실패 - 열람중 오류

// NM_ITEM_GET_OPEN_REQ 참조
struct MSG_REQ_ITEM_GET_OPEN : public MSGROOT
{
	unsigned long idx;								// 고유번호
};
// NM_ITEM_GET_OPEN_RES 참조
struct MSG_RES_ITEM_GET_OPEN : public MSG_ERROR
{
    unsigned long   openItemIdx;                        // 고유번호
	BYTE            rowCount;							// 영향받은 행 수
	sItemData       itemData[1];                        // 아이템 정보

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_ITEM_GET_OPEN) - sizeof(itemData) + (rowCount * sizeof(sItemData)); }
};
// MSG_SYN_ITEM_GET_OPEN 참조
struct MSG_SYN_ITEM_GET_OPEN : public MSGROOT
{
	unsigned long characterIdx;						// 캐릭터 고유번호
};


#define ERROR_ITEM_GET_CLOSE_SUCCESS	0			// 성공
#define ERROR_ITEM_GET_CLOSE_FAIL       1           // 실패

// NM_ITEM_GET_CLOSE_REQ 참조
struct MSG_REQ_ITEM_GET_CLOSE : public MSGROOT
{};
// NM_ITEM_GET_CLOSE_RES 참조
struct MSG_RES_ITEM_GET_CLOSE : public MSG_ERROR
{};
// NM_ITEM_GET_CLOSE_SYN 참조
struct MSG_SYN_ITEM_GET_CLOSE : public MSGROOT
{
	unsigned long characterIdx;						// 캐릭터 고유번호
};


#define ITEM_GET_ALL					(-1)		// 모두줍기
#define ERROR_ITEM_GET_SUCCESS			0			// 성공
#define ERROR_ITEM_GET_FAIL				1			// 실패
#define ERROR_ITEM_GET_AUTO				2			// 자동

// NM_ITEM_GET_REQ 참조
struct MSG_REQ_ITEM_GET : public MSGROOT
{
	unsigned long itemIndexOrInd;					// 아이템 정의 인덱스 또는 모두줍기
};
// NM_ITEM_GET_RES
struct MSG_RES_ITEM_GET : public MSG_ERROR
{
	BYTE       RowCount;							// 영향받은 행 수
	sInventory Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_GET) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};

// NM_ITEM_GET_LEAVE_RES 참조
struct MSG_RES_ITEM_GET_LEAVE : public MSGROOT
{
	BYTE          rowCount;							// 영향받은 행 수
	sItemData     itemData[1];						// 아이템 정보

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_ITEM_GET_LEAVE) - sizeof(itemData) + (rowCount * sizeof(sItemData)); }
};

// NM_ITEM_GATHERING_OPEN_RES 참조
struct MSG_RES_ITEM_GATHERING_OPEN : public MSG_RES_ITEM_GET_OPEN
{};

// NM_ITEM_USE_RES 참고
struct MSG_RES_ITEM_USE : public MSGROOT
{
	sInventory inventory;							// 인벤토리
};

// NM_ITEM_REWARD_RES 참고
struct MSG_RES_ITEM_REWARD : public MSGROOT
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_REWARD) - sizeof(inventory) + (rowCount * sizeof(inventory)); }
};

// NM_ITEM_MAKESKILL_RES , NM_ITEM_RECIPE_RES 참고
struct MSG_RES_MAKESKILL_ITEM : public MSGROOT
{
	sInventory inventory;						// 인벤토리
};

// NM_ITEM_TAKE_RES 참고
struct MSG_RES_ITEM_TAKE : public MSGROOT
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_TAKE) - sizeof(inventory) + (rowCount * sizeof(inventory)); }
};


#define ERROR_ITEM_GET_QUEST_SUCCESS	0			// 성공
#define ERROR_ITEM_GET_QUEST_FAIL		1			// 실패

// NM_ITEM_GET_QUEST_RES 참고
struct MSG_RES_ITEM_GET_QUEST : public MSG_ERROR
{
	BYTE       RowCount;							// 영향받은 행 수
	sInventory Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_GET_QUEST) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


#define ERROR_ITEM_COLLECT_SUCCESS	0				// 성공
#define ERROR_ITEM_COLLECT_FAIL		1				// 실패
#define ERROR_ITEM_COLLECT_NOTYET	2				// DB처리중
#define ERROR_ITEM_COLLECT_LICENSE  3               // 미인증 아이템 사용
#define ERROR_ITEM_COLLECT_STATE    4               // 등록 불가능 상태

// NM_ITEM_COLLECT_REQ
struct MSG_REQ_ITEM_COLLECT : public MSGROOT
{
	unsigned short number1;							// [in] 이동 전
	unsigned short number2;							// [in] 이동 전
};

// NM_ITEM_COLLECT_RES
struct MSG_RES_ITEM_COLLECT : public MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_COLLECT) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


#define ERROR_ITEM_COLLECT_CARD_CHANGE_SUCCESS      0       // 성공
#define ERROR_ITEM_COLLECT_CARD_CHANGE_FAIL		    1       // 실패
#define ERROR_ITEM_COLLECT_CARD_CHANGE_NOTYET	    2		// DB처리중
#define ERROR_ITEM_COLLECT_CARD_CHANGE_CLASS        3       // 타로 등급(변환 불가능)
#define ERROR_ITEM_COLLECT_CARD_CHANGE_COUNT        4       // 카드 갯수 부족
#define ERROR_ITEM_COLLECT_CARD_CHANGE_FULL         5       // 인벤토리 공간 부족
#define ERROR_ITEM_COLLECT_CARD_CHANGE_MAX_ITEM     6       // 보유가능 갯수

// NM_ITEM_COLLECT_CARD_CHANGE_REQ
struct MSG_REQ_COLLECT_CARD_CHANGE : MSGROOT
{
	char    tarotClass;
};

// NM_ITEM_COLLECT_CARD_CHANGE_RES
struct MSG_RES_COLLECT_CARD_CHANGE : MSG_ERROR
{
	unsigned long RowCount;							// 영향받은 행 수
	sInventory    Inventory[1];						// 인벤토리(0:체인지아이템/1:타로카드22장)

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_COLLECT_CARD_CHANGE) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};


// NM_ITEM_MIXED_RES 참고
struct MSG_RES_ITEM_MIXED : public MSGROOT
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_MIXED) - sizeof(inventory) + (rowCount * sizeof(inventory)); }
};

// NM_ITEM_SKILLRESET_RES
struct MSG_RES_ITEM_SKILLRESET : public MSGROOT
{
	sInventory inventory;							// 인벤토리
};


/*-- 미인증 아이템
*/
#define ERROR_ITEM_LICENSE_SUCCESS	    0				// 성공
#define ERROR_ITEM_LICENSE_FAIL		    1				// 실패
#define ERROR_ITEM_LICENSE_TYPE         2               // 미인증 아이템이 아님
#define ERROR_ITEM_LICENSE_ALREADY	    3				// 이미 인증된 아이템

// NM_ITEM_LICENSE_REQ
struct MSG_REQ_ITEM_LICENSE : public MSGROOT
{
	unsigned short number;
};

// NM_ITEM_LICENSE_RES
struct MSG_RES_ITEM_LICENSE : public MSG_ERROR
{
	unsigned short  number;
	unsigned char   seal;                               // 인증 여부
};

/*-- 캐쉬 아이템 수령
*/

struct sCashInventory
{
	unsigned long  idx;								// 인벤토리 고유번호
	unsigned long  ItemIndex;						// 아이템 정의 인덱스
	unsigned short number;							// 아이템 번호 (슬롯인덱스)
	unsigned short count;							// 아이템 수량
	unsigned char  seal;							// 아이템 봉인 (0봉인해제/1봉인됨)
	unsigned char  cash;							// 아이템 캐쉬 (0일반/1캐쉬)
};

#define ERROR_ITEM_CASH_OPEN_SUCCESS	    0				// 성공
#define ERROR_ITEM_CASH_OPEN_FAIL		    1			    // 실패

// NM_ITEM_CASH_OPEN_REQ
struct MSG_REQ_ITEM_CASH_OPEN : public MSGROOT
{};


#define ERROR_ITEM_CASH_CLOSE_SUCCESS	    0				// 성공
#define ERROR_ITEM_CASH_CLOSE_FAIL		    1			    // 실패

// NM_ITEM_CASH_CLOSE_REQ
struct MSG_REQ_ITEM_CASH_CLOSE : public MSGROOT
{};


#define ERROR_ITEM_CASH_LIST_SUCCESS	    0			// 성공
#define ERROR_ITEM_CASH_LIST_FAIL		    1		    // 실패
#define ERROR_ITEM_CASH_LIST_LOADING	    2		    // 실패-캐시아이템 로딩 중

// NM_ITEM_CASH_LIST_REQ
struct MSG_REQ_ITEM_CASH_LIST : public MSGROOT
{};

// NM_ITEM_CASH_LIST_RES
struct MSG_RES_ITEM_CASH_LIST : public MSG_ERROR
{
	long              RowCount;
    sCashInventory    Inventory[1];

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_ITEM_CASH_LIST) - sizeof(Inventory) + (RowCount * sizeof(sCashInventory)); }
};

/*-- 아이템 빌
*/
struct sItemBill
{
	long   idx;											// 고유번호
	BYTE   type;										// 종류(0 일반, 1 기간, 2 시간)
	time_t validDate;									// 유효기간(초 단위 사용)
	long   validTime;									// 유효시간(초 단위 사용)
	long   inventoryIdx;								// TB_INVENTORY::IDX 고유번호
};

// NM_ITEM_BILL_SYN
struct MSG_SYN_ITEM_BILL : public MSGROOT
{
	long      rowCount;									// 적용개수
	sItemBill itemBill[1];								// 아이템 빌
};

// NM_ITEM_BILL_ADD_SYN
struct MSG_SYN_ITEM_BILL_ADD : public MSGROOT
{
	sItemBill itemBill;									// 아이템 빌
};

// NM_ITEM_BILL_DEL_SYN
struct MSG_SYN_ITEM_BILL_DEL : public MSGROOT
{
	long idx;											// 아이템 빌::IDX 고유번호
};

#define ERROR_THEMERESET_SUCCESS            0
#define ERROR_THEMERESET_STATE              1			// 캐릭터 상태
#define ERROR_THEMERESET_ITEM               2			// 아이템 인증에러
#define ERROR_THEMERESET_DBERROR            3			// db처리 에러
#define ERROR_THEMERESET_ITEM_LICENSE       4           // 미인증 아이템 사용
#define ERROR_THEMERESET_READY				5			// 던전 초기화 대기 상태

// NM_ITEM_THEMERESET_REQ
struct MSG_REQ_ITEM_THEMERESET : public MSGROOT
{
	unsigned short	mSlotNum;							/// 인벤슬롯 번호
	unsigned short	mThemeNum;							/// 테마 종류(맵타입)
	unsigned char	mThemeMode;							///	난이도
};

// NM_ITEM_THEMERESET_RES
struct MSG_RES_ITEM_THEMERESET : public MSG_ERROR
{
	sInventory			mInventory;						/// 인벤토리
	unsigned short		mThemeNum;						/// 테마 종류(맵타입)
	unsigned char		mThemeMode;						///	난이도

};


/*-- 파티장 아이템 지급
*/
#define ERROR_ITEM_PARTY_GIVE_SUCCESS		0			// 성공
#define ERROR_ITEM_PARTY_GIVE_FAIL		    1			// 실패
#define ERROR_ITEM_PARTY_GIVE_TYPE		    2			// 실패 - 파티장 분배가 아님
#define ERROR_ITEM_PARTY_GIVE_OWNER		    3			// 실패 - 지급 권한 없음
#define ERROR_ITEM_PARTY_GIVE_TARGET		4			// 실패 - 지급할 캐릭터 없음
#define ERROR_ITEM_PARTY_GIVE_PARTY         5           // 실패 - 소유권 오류
#define ERROR_ITEM_PARTY_GIVE_MAP           6           // 실패 - 동일 맵 오류
#define ERROR_ITEM_PARTY_GIVE_INVEN_FULL    7           // 실패 - 인벤토리 부족
#define ERROR_ITEM_PARTY_GIVE_MAX_ITEM      8           // 실패 - 보유가능 갯수 초과
#define ERROR_ITEM_PARTY_GIVE_WEIGHT        9           // 실패 - 인벤토리 무게


// NM_ITEM_PARTY_GIVE_REQ
struct MSG_REQ_ITEM_PARTY_GIVE : public MSGROOT
{
    unsigned long characterIdx;
    unsigned long itemDefineIndex;
};

// NM_ITEM_PARTY_GIVE_RES
struct MSG_RES_ITEM_PARTY_GIVE : public MSG_ERROR
{ };


// NM_ITEM_PARTY_GIVE_SYN
struct MSG_SYN_ITEM_PARTY_GIVE : public MSGROOT
{
	BYTE       rowCount;							// 영향받은 행 수
	sInventory inventory[1];						// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_ITEM_PARTY_GIVE) - sizeof(inventory) + (rowCount * sizeof(sInventory)); }
};


/*-- 소환 주문서 - 창고
*/
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_SUCCESS		    0   // 성공- 소모용 소환 주문서
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_USEFUL_SUCCESS		1	// 성공- 기간제 편의기능 주문서
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_CHANGESTATE		2	// 성공- ChangeState(eOBJECT_STATE_STOP) == false 의 경우 소모만 되도록..
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_FAIL		        3	// 실패
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_LICENSE            4   // 실패- 미인증 아이템
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_ALREADY            5   // 실패- 소환주문서 사용중
#define ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_ITEMPWD_CHECK      6   // 실패- 아이템 인증번호 확인해야 함

// NM_ITEM_WAREHOUSE_SUMMON_OPEN_REQ
struct MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN : public MSGROOT
{
    unsigned short number;
};

// NM_ITEM_WAREHOUSE_SUMMON_OPEN_RES
struct MSG_RES_ITEM_WAREHOUSE_SUMMON_OPEN : public MSG_ERROR
{
	sInventory inventory;
};

// NM_ITEM_WAREHOUSE_SUMMON_OPEN_SYN
struct MSG_SYN_ITEM_WAREHOUSE_SUMMON_OPEN : public MSGROOT
{
	unsigned long characterIdx;
};

// NM_ITEM_WAREHOUSE_SUMMON_CLOSE_SYN
struct MSG_SYN_ITEM_WAREHOUSE_SUMMON_CLOSE : public MSGROOT
{
	unsigned long characterIdx;
};

#pragma pack(pop)
