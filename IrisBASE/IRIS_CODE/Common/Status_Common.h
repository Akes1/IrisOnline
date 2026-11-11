/* ==========================================================================
* 파    일 : Status_Common.h
* 목    적 : 스테이터스 계산 데이타
* 작 성 자 : 박경희
* 작 성 일 : 2007-01-12
* 주의사항 :
*===========================================================================*/
#pragma once

#include "CommonNetworkMsgStruct.h"
#include "Player_Common.h"

#pragma pack(push,1)


/// 스텟 증가 항목 정의
enum eSTATUS_PLUS
{	
	eSTATUSPLUS_NONE	 = 0,	/// 사용안함

	eSTATUSPLUS_STR_PLUS = 1,	/// 힘(+)
	eSTATUSPLUS_DEX_PLUS,		/// 민첩(+)
	eSTATUSPLUS_CON_PLUS,		/// 체력(+)
	eSTATUSPLUS_INT_PLUS,		/// 지능(+)
	eSTATUSPLUS_WIS_PLUS,		/// 지혜(+)

	eSTATUSPLUS_BASE_MAX,

	eSTATUSPLUS_BASEALL_PLUS = 11,	/// 힘, 민첩, 체력, 지능, 지혜 전체(+)

	eSTATUSPLUS_STR_PERCENT = 21,	/// 힘(%)
	eSTATUSPLUS_DEX_PERCENT = 22,	/// 민첩(%)
	eSTATUSPLUS_CON_PERCENT = 23,	/// 체력(%)
	eSTATUSPLUS_INT_PERCENT = 24,	/// 지능(%)
	eSTATUSPLUS_WIS_PERCENT = 25,	/// 지혜(%)

	eSTATUSPLUS_BASE_PERCENT_MAX,

	eSTATUSPLUS_BASEALL_PERCENT = 31,	/// 힘, 민첩, 체력, 지능, 지혜 전체(%)

	eSTATUSPLUS_ATK_PLUS = 101,	/// 물리근접공격(+)
	eSTATUSPLUS_RATK_PLUS,		/// 물리원거리공격(+)
	eSTATUSPLUS_DEF_PLUS,		/// 물리 방어력(+)
	eSTATUSPLUS_MATK_PLUS,		/// 마법 공격력(+)
	eSTATUSPLUS_MDEF_PLUS,		/// 마법 방어력(+)

	eSTATUSPLUS_CRI_PLUS,		/// 물리공격크리(+)
	eSTATUSPLUS_MCRI_PLUS,		/// 마법공격크리(+)
	eSTATUSPLUS_HP_PLUS,		/// HP(+)
	eSTATUSPLUS_MP_PLUS,		/// MP(+)
	eSTATUSPLUS_RECOVHP_PLUS,	/// HP회복속도(+)	

	eSTATUSPLUS_RECOVMP_PLUS,	/// MP회복속도(+)
	eSTATUSPLUS_HIT_PLUS,		/// 물리명중률(+)
	eSTATUSPLUS_MHIT_PLUS,		/// 마법명중률(+)
	eSTATUSPLUS_AVOID_PLUS,		/// 회피율(+)
	eSTATUSPLUS_WEIGHT_PLUS,	/// 무게(+)

	eSTATUSPLUS_ASPD_PLUS,		/// 일반공격속도(+)
	eSTATUSPLUS_COOLTIME_PLUS,	/// 스킬쿨타임속도(+)
	eSTATUSPLUS_RANGE_PLUS,		/// 근거리사정거리(+)
	eSTATUSPLUS_RRANGE_PLUS,	/// 원거리(+마법)사정거리
	eSTATUSPLUS_SPD_PLUS,		/// 이동속도(+)	

	/// 힐회복량은 기본 값이 없어 plus값을 따로 저장 하고 있다.
	eSTATUSPLUS_SKILL_HP_PLUS_PLUS,	/// 힐회복량(+)
	eSTATUSPLUS_RECIVE_HP_PLUS,	/// 받는 힐회복량(+)
	eSTATUSPLUS_MAVOID_PLUS,	/// 마법회피(저항력)(+)
	eSTATUSPLUS_CRIDEF_PLUS,	/// 물리공격크리감소(+)
	eSTATUSPLUS_MCRIDEF_PLUS,	/// 마법공격크리감소(+)
	eSTATUSPLUS_DISTRESS_PLUS,	/// 어그로(+)
	eSTATUSPLUS_USE_MP_PLUS	,	/// MP 소모량(+)
	eSTATUSPLUS_CASTING_PLUS,   /// 캐스팅 시간(+)
	eSTATUSPLUS_HUMAN_TYPE_PLUS,	 /// 인간형 공격력(+)	
	eSTATUSPLUS_ANIMAL_TYPE_PLUS,	 /// 동물형 공격력(+)	
	eSTATUSPLUS_PLANT_TYPE_PLUS,	 /// 식물형 공격력(+)	
	eSTATUSPLUS_BUG_TYPE_PLUS,		 /// 곤충형 공격력(+)	
	eSTATUSPLUS_SOUL_TYPE_PLUS,		 /// 정령형 공격력(+)	
	eSTATUSPLUS_UNDEAD_TYPE_PLUS,	 /// 언데드형 공격력(+)	
	eSTATUSPLUS_GIANT_TYPE_PLUS,	 /// 거인형 공격력(+)	
	eSTATUSPLUS_DRAGON_TYPE_PLUS,	 /// 용형 공격력(+)	
	eSTATUSPLUS_NORMAL_TYPE_PLUS,    /// 일반 몬스터 추가 대미지(+)
	eSTATUSPLUS_VETERAN_TYPE_PLUS,   /// 정예 몬스터 추가 대미지(+)
	eSTATUSPLUS_ELITE_TYPE_PLUS,     /// 엘리트 몬스터 추가 대미지(+)
	eSTATUSPLUS_BOSS_TYPE_PLUS,      /// 보스 몬스터 추가 대미지(+)
	eSTATUSPLUS_CHARACTER_TYPE_PLUS, /// 캐릭터(유저) 추가 대미지(+)
	eSTATUSPLUS_EXT_PLUS_MAX     = eSTATUSPLUS_CHARACTER_TYPE_PLUS,

	/// 예외처리
	eSTATUSPLUS_ATKTOTAL_PLUS = 151,	/// 공격력 전체(+)
	eSTATUSPLUS_DEFTOTAL_PLUS = 152,	/// 방어력 전체(+)

	eSTATUSPLUS_ATK_PER = 201,	/// 물리근접공격(%)
	eSTATUSPLUS_RATK_PER,		/// 물리원거리공격(%)
	eSTATUSPLUS_DEF_PER,		/// 물리 방어력(%)
	eSTATUSPLUS_MATK_PER,		/// 마법 공격력(%)
	eSTATUSPLUS_MDEF_PER,		/// 마법 방어력(%)

	eSTATUSPLUS_CRI_PER,		/// 물리공격크리(%)
	eSTATUSPLUS_MCRI_PER,		/// 마법공격크리(%)
	eSTATUSPLUS_HP_PER,			/// HP(%)
	eSTATUSPLUS_MP_PER,			/// MP(%)
	eSTATUSPLUS_RECOVHP_PER,	/// HP회복속도(%)

	eSTATUSPLUS_RECOVMP_PER,	/// MP회복속도(%)
	eSTATUSPLUS_HIT_PER,		/// 물리명중률(%)
	eSTATUSPLUS_MHIT_PER,		/// 마법명중률(%)
	eSTATUSPLUS_AVOID_PER,		/// 회피율(%)
	eSTATUSPLUS_WEIGHT_PER,		/// 무게(%)	

	eSTATUSPLUS_ASPD_PER,		/// 일반공격속도(%)
	eSTATUSPLUS_COOLTIME_PER,	/// 스킬쿨타임속도(%)
	eSTATUSPLUS_RANGE_PER,		/// 근거리사정거리(%)
	eSTATUSPLUS_RRANGE_PER,		/// 원거리(%마법)사정거리
	eSTATUSPLUS_SPD_PER,		/// 이동속도(%)

	/// 힐회복량은 기본 값이 없어 plus값을 따로 저장 하고 있다.
	eSTATUSPLUS_SKILL_HP_PLUS_PER,	/// 힐회복량(%)
	eSTATUSPLUS_RECIVE_HP_PER,	/// 받는 힐회복량(%)
	eSTATUSPLUS_MAVOID_PER,		/// 마법회피(저항력)(%)
	eSTATUSPLUS_CRIDEF_PER,		/// 물리공격크리(%)
	eSTATUSPLUS_MCRIDEF_PER,	/// 마법공격크리(%)
	eSTATUSPLUS_DISTRESS_PER,	/// 어그로(%)
	eSTATUSPLUS_USE_MP_PER,		/// MP 소모량(%)
	eSTATUSPLUS_CASTING_PER,    /// 캐스팅 시간(%)
	eSTATUSPLUS_HUMAN_TYPE_PER,		  /// 인간형 공격력(%)
	eSTATUSPLUS_ANIMAL_TYPE_PER,	  /// 동물형 공격력(%)
	eSTATUSPLUS_PLANT_TYPE_PER,		  /// 식물형 공격력(%)
	eSTATUSPLUS_BUG_TYPE_PER,		  /// 곤충형 공격력(%)
	eSTATUSPLUS_SOUL_TYPE_PER,		  /// 정령형 공격력(%)
	eSTATUSPLUS_UNDEAD_TYPE_PER,	  /// 언데드형 공격력(%)
	eSTATUSPLUS_GIANT_TYPE_PER,		  /// 거인형 공격력(%)
	eSTATUSPLUS_DRAGON_TYPE_PER,	  /// 용형 공격력(%)
	eSTATUSPLUS_NORMAL_TYPE_PER,	  /// 일반 몬스터 추가 대미지(%)
	eSTATUSPLUS_VETERAN_TYPE_PER,	  /// 정예 몬스터 추가 대미지(%)
	eSTATUSPLUS_ELITE_TYPE_PER,		  /// 엘리트 몬스터 추가 대미지(%)
	eSTATUSPLUS_BOSS_TYPE_PER,        /// 보스 몬스터 추가 대미지(%)
	eSTATUSPLUS_CHARACTER_TYPE_PER,   /// 캐릭터(유저) 추가 대미지(%)
	eSTATUSPLUS_EXT_PER_MAX = eSTATUSPLUS_CHARACTER_TYPE_PER,

	/// 예외처리
	eSTATUSPLUS_ATKTOTAL_PER = 251,	/// 공격력 전체(%)
	eSTATUSPLUS_DEFTOTAL_PER = 252,	/// 방어력 전체(%)

	eSTATUSPLUS_EXPADDPER = 301,			/// 경험치 획득 증가(%)
	eSTATUSPLUS_EXPADDPLUS,					/// 경험치 획득 증가(+)
	eSTATUSPLUS_SXPADDPER,					/// 스킬경험치 획득 증가(%)
	eSTATUSPLUS_SXPADDPLUS,					/// 스킬경험치 획득 증가(+)
	eSTATUSPLUS_MONEYPER,					/// 테르 획득 증가(%)
	eSTATUSPLUS_PVPPOINTPER,				/// 전장 포인트 증가(%)

	eSTATUSPLUS_BUFF_ETC_MAX,


	eSTATUSPLUS_INSTANTPERHIT = 401,		/// 공격 성공 확률 - %순간 효과
	eSTATUSPLUS_INSTANTPLUSHIT,				/// 공격 성공 확률 - 순간 효과
	eSTATUSPLUS_INSTANTPERCRI,				/// 크리티컬 발생 확률 - %순간 효과
	eSTATUSPLUS_INSTANTPLUSCRI,				/// 크리티컬 발생 확률 - 순간 효과
	eSTATUSPLUS_AROUND_PUSH,				/// 타겟 밀기(자신 기준)
	eSTATUSPLUS_AROUND_PULL,				/// 타겟 당기기(자신 기준)
	eSTATUSPLUS_INSTANTPERATTACK,			/// 공격력 - %순간 효과
	eSTATUSPLUS_INSTANTPLUSATTACK,			/// 공격력 - 순간 효과
	eSTATUSPLUS_HPABSORB_PER,				/// hp흡수 %
	eSTATUSPLUS_MPABSORB_PER,				/// mp흡수 %
	eSTATUSPLUS_INSTANTPERFINAL,			/// 최종 데미지 - %순간 효과
	eSTATUSPLUS_INSTANTPLUSFINAL,			/// 최종 데미지 - 순간 효과

	eSTATUSPLUS_DELETE_TARGETING,			/// 타겟팅 제거
	eSTATUSPLUS_TARGET_PUSH,				/// 타겟 밀기(타겟 기준)
	eSTATUSPLUS_INSTANT_IGNORE_DEFENCE,		/// 방어력 무시 - 순간효과

	eSTATUSPLUS_INSTANT_HUMAN_TYPE_PER_ATTACK,		 /// 인간형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_HUMAN_TYPE_PLUS_ATTACK,		 /// 인간형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PER_ATTACK,		 /// 동물형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_ANIMAL_TYPE_PLUS_ATTACK,	 /// 동물형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_PLANT_TYPE_PER_ATTACK,		 /// 식물형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_PLANT_TYPE_PLUS_ATTACK,		 /// 식물형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_BUG_TYPE_PER_ATTACK,		 /// 곤충형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_BUG_TYPE_PLUS_ATTACK,		 /// 곤충형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_SOUL_TYPE_PER_ATTACK,		 /// 정령형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_SOUL_TYPE_PLUS_ATTACK,		 /// 정령형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PER_ATTACK,		 /// 언데드형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_UNDEAD_TYPE_PLUS_ATTACK,	 /// 언데드형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_GIANT_TYPE_PER_ATTACK,		 /// 거인형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_GIANT_TYPE_PLUS_ATTACK,		 /// 거인형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_DRAGON_TYPE_PER_ATTACK,		 /// 용형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_DRAGON_TYPE_PLUS_ATTACK,	 /// 용형 공격력 - +순간 효과
    
	eSTATUSPLUS_INSTANT_NORMAL_TYPE_PER_ATTACK,		  /// 일반 몬스터형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_NORMAL_TYPE_PLUS_ATTACK,	  /// 일반 몬스터형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_VETERAN_TYPE_PER_ATTACK,	  /// 정예 몬스터형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_VETERAN_TYPE_PLUS_ATTACK,	  /// 정예 몬스터형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_ELITE_TYPE_PER_ATTACK,		  /// 엘리트 몬스터형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_ELITE_TYPE_PLUS_ATTACK,		  /// 엘리트 몬스터형 공격력 - +순간 효과
	eSTATUSPLUS_INSTANT_BOSS_TYPE_PER_ATTACK,		  /// 보스 몬스터형 공격력 - %순간 효과
	eSTATUSPLUS_INSTANT_BOSS_TYPE_PLUS_ATTACK,	      /// 보스 몬스터형 공격력 - +순간 효과
	
	eSTATUSPLUS_INSTANT_CLEAR_SKILL_COOLTIME,		  /// 해당 스킬을 제외한 자신이 보유한 모든 스킬 쿨 타임 모두 삭제

	eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PER_ATTACK,	   /// 캐릭터형 공격력 - %순간 효과	
	eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PLUS_ATTACK,	   /// 캐릭터형 공격력 - +순간 효과	

	eSTATUSPLUS_DAMAGECALC_MAX = eSTATUSPLUS_INSTANT_CHARACTER_TYPE_PLUS_ATTACK,

	eSTATUSPLUS_DISPEL = 501,				/// 디스펠
	eSTATUSPLUS_RESURRECTION_HP_PLUS,		/// 부활+HP(+)
	eSTATUSPLUS_RESURRECTION_HP_PER,		/// 부활+HP(%)
	eSTATUSPLUS_RESURRECTION_MP_PLUS,		/// 부활+MP(+)
	eSTATUSPLUS_RESURRECTION_MP_PER,		/// 부활+MP(%)
	eSTATUSPLUS_EMPTY1,						/// 삭제 - 순간 이동
	eSTATUSPLUS_DISPEL_ALL,					/// 전체 디스펠
	eSTATUSPLUS_EMPTY2,						/// 삭제 - 공격자가 HP 순간 흡수
	eSTATUSPLUS_EMPTY3,						/// 삭제 - 공격자가 MP 순간 흡수
	eSTATUSPLUS_MONSTER_ESCAPE,				/// 몬스터 도망상태로 변경

	eSTATUSPLUS_STOPMOVE = 551,				/// 이동 불가
	eSTATUSPLUS_STOPSKILL,					/// 스킬 사용 불가(통합)
	eSTATUSPLUS_ATTACKERTARGETING,			/// 본인 타겟팅
	eSTATUSPLUS_SLEEP,						/// 수면
	eSTATUSPLUS_STUN,						/// 스턴
	eSTATUSPLUS_INVINCIBILITY_PHY,			/// 무적 - 물리
	eSTATUSPLUS_INVINCIBILITY_MAG,			/// 무적 - 마법
	eSTATUSPLUS_HP_ABSORB_LONG,				/// 공격자가 HP 흡수 지속
	eSTATUSPLUS_MP_ABSORB_LONG,				/// 공격자가 MP 흡수 지속
	eSTATUSPLUS_STOPITEM_KIND,				/// 아이템 사용 불가 - 종류별
	eSTATUSPLUS_STOPITEM_IDX,				/// 아이템 사용 불가 - 아이템인덱스별
	eSTATUSPLUS_DAMAGE_ABSORB_PHY,			/// 데미지 흡수(물리:보호막)
	eSTATUSPLUS_DAMAGE_ABSORB_MAG,			/// 데미지 흡수(마법:보호막)
	eSTATUSPLUS_DAMAGE_REFLECT_PHY,			/// 데미지 반사(물리)
	eSTATUSPLUS_DAMAGE_REFLECT_MAG,			/// 데미지 반사(마법)
	eSTATUSPLUS_IMMUNE_KIND,				/// 면역 - 효과 종류 별 면역
	eSTATUSPLUS_IMMUNEDEBUFF_APPLYTYPE,		/// 디버프 면역 - 효과 적용 번호 별
	eSTATUSPLUS_STOPSKILL_PHY,				/// 스킬 사용 불가(물리)
	eSTATUSPLUS_STOPSKILL_MAG,				/// 스킬 사용 불가(마법)
	eSTATUSPLUS_GUARDMP_PHY,				/// MP보호막-물리공격
	eSTATUSPLUS_GUARDMP_MAG,				/// MP보호막-마법공격
	eSTATUSPLUS_GUARDMP_ALL,				/// MP보호막-모든공격
	eSTATUSPLUS_GUARDVALUE_PHY,				/// 숫자 차감 보호막-물리공격
	eSTATUSPLUS_GUARDCNT_PHY,				/// 횟수 차감 보호막-물리공격
	eSTATUSPLUS_GUARDVALUE_MAG,				/// 숫자 차감 보호막-마법공격
	eSTATUSPLUS_GUARDCNT_MAG,				/// 횟수 차감 보호막-마법공격
	eSTATUSPLUS_GUARDVALUE_ALL,				/// 숫자 차감 보호막-모든공격
	eSTATUSPLUS_GUARDCNT_ALL,				/// 횟수 차감 보호막-모든공격
	eSTATUSPLUS_DAMAGE_ABSORB_PHY_RANGE,	/// 물리 원거리 데미지 흡수(%)
	eSTATUSPLUS_IGNORE_DEFENCE_PHY,			/// 물리 방어력 무시 공격 지속
	eSTATUSPLUS_IGNORE_DEFENCE_MAG,			/// 마법 방어력 무시 공격 지속
	eSTATUSPLUS_ADD_APPLYCNT,				/// 범위 스킬 적용 인원 증가 효과
	eSTATUSPLUS_AVOID_MON_LOOK,				/// 몬스터 선공 인식 회피
	eSTATUSPLUS_SWAP_DISTRESS_POINT,		/// 어그로 교체

	eSTATUSPLUS_IMMUNEBUFF_APPLYTYPE = 585,		/// 버프 면역 - 효과 적용 번호 별

	eSTATUSPLUS_CONDITIONINDISPOSITION_MAX, 


	eSTATUSPLUS_DOT_HPCHANGE_PLUS = 601,	///	(효과) 지속 HP +
	eSTATUSPLUS_DOT_HPCHANGE_PER,			/// 지속 HP %
	eSTATUSPLUS_DOT_MPCHANGE_PLUS,			/// 지속 MP +
	eSTATUSPLUS_DOT_MPCHANGE_PER,			/// 지속 MP %

	eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS,		/// 지속 HP 힐/데미지 + (계산식사용)
	eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS,		/// 지속 MP 힐/데미지 + (계산식사용)

	eSTATUSPLUS_DOT_HPCHANGE_CALC_PER,		/// 지속 HP 힐/데미지 % (계산식사용)
	eSTATUSPLUS_DOT_MPCHANGE_CALC_PER,		/// 지속 MP 힐/데미지 % (계산식사용)

	eSTATUSPLUS_DOT_MAX,


	eSTATUSPLUS_NONSKILL_HPCHANGE_PLUS = 701,	/// (스킬이 아닌) 순간 hp +
	eSTATUSPLUS_NONSKILL_HPCHANGE_PER,			/// 순간 hp %
	eSTATUSPLUS_NONSKILL_MPCHANGE_PLUS,			/// 순간 mp +
	eSTATUSPLUS_NONSKILL_MPCHANGE_PER,			/// 순간 mp %
	eSTATUSPLUS_NONSKILL_DISTRESS_PLUS,			/// 순간 보유 어그로 증가 +
	eSTATUSPLUS_NONSKILL_DISTRESS_PER,			/// 순간 보유 어그로 증가 %

	eSTATUSPLUS_NONSKILL_MAX,

	eSTATUSPLUS_NEED_STATE_ICE =801 ,			/// 스킬 시전을 위한 조건 사용 효과 -얼음
	eSTATUSPLUS_NEED_STATE_LIGHTNING,			/// 스킬 시전을 위한 조건 사용 효과 -전기
	eSTATUSPLUS_NEED_STATE_WIND,				/// 스킬 시전을 위한 조건 사용 효과 -바람
	eSTATUSPLUS_NEED_STATE_FIRE,				/// 스킬 시전을 위한 조건 사용 효과 -불

	eSTATUSPLUS_NEED_STATE_MAX,

	eSTATUSPLUS_CHAR_SIZE = 1001,		/// 캐릭터 크기변화
	eSTATUSPLUS_CHANGE_MONSTER,			/// 몬스터로 외형변신
	eSTATUSPLUS_VEHICLE,				/// 탈것 

	eSTATUSPLUS_MAPCHANGE = 2000,			/// 맵이동
	eSTATUSPLUS_CASHMAPCHANGE,				/// 캐시 맵이동

	eSTATUSPLUS_CASHEXPGUARDCNT = 2101,		/// 캐시 경험치 감소 방지 


	eSTATUSPLUS_TAROT_MAX,
};


enum eEXT_MULTIPL
{
	eEXT_MULTIPL_ATK_FIGHTER = 0,		/// 전사물리근접공격력
	eEXT_MULTIPL_ATK_RANGER,			/// 레인져물리근접공격력
	eEXT_MULTIPL_ATK_MAGIC,				/// 법사물리근접공격력 
	eEXT_MULTIPL_RATK_FIGHTER,			/// 전사물리원거리공격력
	eEXT_MULTIPL_RATK_RANGER,			/// 레인져물리원거리공격력
	eEXT_MULTIPL_RATK_MAGIC,			/// 법사물리원거리공격력 
	eEXT_MULTIPL_MATK_FIGHTER,			/// 전사마법공격력
	eEXT_MULTIPL_MATK_RANGER,			/// 레인져마법공격력
	eEXT_MULTIPL_MATK_MIGHC,			/// 법사마법공격력 
	eEXT_MULTIPL_DEF,					/// 물리 방어력
	eEXT_MULTIPL_MDEF,					/// 마법 방어력
	eEXT_MULTIPL_CRI,					/// 물리 크리티컬 수치
	eEXT_MULTIPL_MCRI,					/// 마법 크리티컬 수치
	eEXT_MULTIPL_MAXHP,					/// HP
	eEXT_MULTIPL_MAXMP,					/// MP
	eEXT_MULTIPL_RECOVHP,				/// HP 회복 속도
	eEXT_MULTIPL_RECOVMP,				/// MP 회복 속도
	eEXT_MULTIPL_HIT,					/// 명중 수치
	eEXT_MULTIPL_MHIT,					/// 마법 명중
	eEXT_MULTIPL_AVOID,					/// 회피 수치
	eEXT_MULTIPL_WEIGHT,				/// 무게 
	eEXT_MULTIPL_RANGE,					/// 사정거리
	eEXT_MULTIPL_RRANGE,				/// 방출혈사정거리
	eEXT_MULTIPL_COOLTIME,				/// 쿨타임
	eEXT_MULTIPL_SPD,					/// 이동속도
	eEXT_MULTIPL_ASPD,					/// 공격 속도
	eEXT_MULTIPL_MAVOID,				/// 마법회피(저항력)
	eEXT_MULTIPL_HEAL,					/// 힐 회복량
	eEXT_MULTIPL_CRIDEF,				/// 물리 크리티컬 감소(방호)
	eEXT_MULTIPL_MCRIDEF,				/// 마법 크리티컬 감소(방호)

	eEXT_MULTIPL_MAX = ULONG_MAX,
};


/// 스텟 기본 값
struct sStatusBaseScript
{
	/// 직업 코드
	ePLAYER_JOB			mJobIndex;
	/// 직업 이름
	char				mName[255];
	/// 기본스텟 항목 5개
	float				mBaseStatus[ePLAYER_STATUS_WIS+1];
};



/// 스텟 증가 수치
struct sStatusMultiplScript
{
	/// 직업 코드
	ePLAYER_JOB			mJobIndex;
	/// 직업 이름
	char				mName[255];
	/// 기본스텟 항목 5개
	float				mBaseStatus[ePLAYER_STATUS_WIS+1];
};

#pragma pack(pop)
