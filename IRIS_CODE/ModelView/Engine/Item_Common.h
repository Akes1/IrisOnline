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

	ITEM_MATERIAL   = 4,				// 재료
	ITEM_MATERIAL_SKILL = 0,		// 스킬
	ITEM_MATERIAL_CLOTH,			// 옷감
	ITEM_MATERIAL_LEATHER,			// 가죽
	ITEM_MATERIAL_METAL,			// 금속
	ITEM_MATERIAL_JEWEL,			// 보석
	ITEM_MATERIAL_MISC,				// 잡화
	ITEM_MATERIAL_MAP,				// 이동주문서
	ITEM_MATERIAL_VEHICLE,			// 탈것

	ITEM_QUEST      = 5,				// 퀘스트
	ITEM_ETC2       = 6,				// 일반잡화
	ITEM_SPREAD     = 7,				// 스프레드

	ITEM_CARD       = 8,				// 카드
	ITEM_CARD_TAROT = 0,			// 카드-타로(메이저)
	ITEM_CARD_COMMON,				// 카드-소모
	ITEM_CARD_EQUIP_C,				// 카드-캐릭터장착
	ITEM_CARD_EQUIP_A,				// 카드-A타입
	ITEM_CARD_EQUIP_B,				// 카드-B타입
	ITEM_CARD_EQUIP_AB,				// 카드-AB타입
	ITEM_CARD_EQUIP_O,				// 카드-O타입
	ITEM_CARD_ENHANCE_WEAPON_LV1,	// 카드-무기강화
	ITEM_CARD_ENHANCE_WEAPON_LV2,	// 카드-무기강화
	ITEM_CARD_ENHANCE_WEAR_LV1,		// 카드-방어구강화
	ITEM_CARD_ENHANCE_WEAR_LV2,		// 카드-방어구강화

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
	eTOOLTIP_MALL,		/// 몰
	eTOOLTIP_ETC,		/// 잡화
	eTOOLTIP_NORMAL,	/// 일반
	eTOOLTIP_MAGIC,		/// 매직
	eTOOLTIP_RARE,		/// 레어
	eTOOLTIP_UNIQUE,	/// 유니크
	eTOOLTIP_EPIC,		/// 에픽
	eTOOLTIP_REGEND,	/// 레젼드

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

#define MIN_ITEM_ENHANCED		1		// 아이템 강화 유효단계 (0~15까지, 0일경우 강화없음)
#define MAX_ITEM_ENHANCED		15		// 아이템 강화 유효단계 (0~15까지, 0일경우 강화없음)

#define CHATPRINT_ENHANCED		15		// 

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

	INVENTORY_TAROT_BEGIN = INVENTORY_COLLECTION_BEGIN,
	INVENTORY_TAROT_END   = INVENTORY_TAROT_BEGIN + 21,

	INVENTORY_SPREAD_BEGIN,
	INVENTORY_SPREAD_END  = INVENTORY_SPREAD_BEGIN + 4,

	INVENTORY_COLLECTION_END = INVENTORY_COLLECTION_BEGIN + 99,

	// INVENTORY_CASH_ITEM_TYPE - 캐시아이템 수령함 전용
	INVENTORY_CASH_ITEM_BEGIN,
	INVENTORY_CASH_ITEM_END = INVENTORY_CASH_ITEM_BEGIN + 100,
};

#define INVENTORY_EQUIP_NONE			(-1)
#define INVENTORY_EQUIP_BEGIN			INVENTORY_WEAR_BEGIN
#define INVENTORY_EQUIP_END				INVENTORY_COSTUME_BACK

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

#pragma pack(pop)
