/* ====================================================================
* 파    일 :	
* 목    적 :	
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.08.29	정성대	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once
#pragma pack(push,1)
#include "BaseObject_Common.h"
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"
#include "Cheat_Common.h"

// 인벤토리, 창고 최대 소지할 수 있는 금액 - 30억
const unsigned long MAX_HAVE_MONEY = 3000000000;

// 최대 거래 금액 - 10억
const unsigned long MAX_TRADE_MONEY = 1000000000;

// 가방 추가시 증가되는 무게 수치(누적값으로 적용)
const unsigned long ADD_BAG1_WEIGHT = 200;
const unsigned long ADD_BAG2_WEIGHT = 600;          // ADD_BAG1_WEIGHT + 400
const unsigned long ADD_BAG3_WEIGHT = 1400;         // ADD_BAG2_WEIGHT + 800


/// 부활 경험치 범위
const float RESURRECTION_EXP_STAND_MIN = 3.0f;
const float RESURRECTION_EXP_STAND_MAX = 5.0f;
const float RESURRECTION_EXP_SAFE_MIN = 1.0f;
const float RESURRECTION_EXP_SAFE_MAX = 3.0f;
const float RESURRECTION_EXP_SKILL_MIN = 1.0f;
const float RESURRECTION_EXP_SKILL_MAX = 3.0f;
const float RESURRECTION_THEME_EXP_SAFE_MIN = 0.1f;
const float RESURRECTION_THEME_EXP_SAFE_MAX = 0.3f;
const float RESURRECTION_THEME_EXP_SKILL_MIN = 0.1f;
const float RESURRECTION_THEME_EXP_SKILL_MAX = 0.3f;

/// 제자리부활 디버프
const int RESURRECTION_STAND_INFLUENCE_CLASSIDX = 25000;

const int PLAYER_ITEMMAPCHANGE_WAITTIME = 10 * SECOND;
const int PLAYER_ITEMMAPCHANGE_CLEARTIME = 3 * SECOND;

/// 자연회복 시간
const unsigned long NATURE_RECOVERY_TIME = 10 * SECOND;
const unsigned long SITDOWN_RECOVERY_TIME = 5 * SECOND;

/// 몬스터 변신시 체공 시간
const unsigned long PLAYER_JUMPTIME_CHGMONSTER = 1100;

/// enum 변수들...
enum /*Gender*/
{
	eGENDER_MALE = 0,
	eGENDER_FEMALE,

	eGENDER_MAX,
};

enum /*Race*/
{
	eRACE_HUMAN = 0,
	eRACE_BEAST,
	eRACE_ELF,

	eRACE_MAX,
};

enum eHAND_TYPE
{
	eHAND_LEFT = 0,
	eHAND_RIGHT,

	eHAND_ALL,
	eHAND_NONE,
};

enum eCOSTUME_LINK
{
	eCOSTUME_HEAD = 0,
	eCOSTUME_BACK,

	eCOSTUME_MAX,
};

/// head, wear type은 구조체 관리를 위한 형태 
enum eHEAD_TYPE
{
	eHEAD_HAIR = 0,
	eHEAD_HAIRCOLOR,
	eHEAD_FACE,
	eHEAD_FACE_STATIC,	/// static face emote

	eHEAD_MAX,
};
/* unsigned char */
enum eWEAR_TYPE
{
	eWEAR_BODY1 = 0,
	eWEAR_BODY2,
	eWEAR_HAND,
	eWEAR_FOOT,

	eWEAR_HAT,

	eWEAR_MAX,
	eWEAR_ONEPIECE,
};
///

/* unsigned char */
enum eWEAPON_STATE	
{
	//
	eWEAPON_STATE_NONE = 0,
	eWEAPON_STATE_SWORD = 1,		// 한손검
	// 

	eWEAPON_STATE_LONGSWORD,		// 양손검
	eWEAPON_STATE_DOUBLESWORD,		// 이도류
	eWEAPON_STATE_SHORTSWORD,		// 단검
	eWEAPON_STATE_GUN,				// 총
	eWEAPON_STATE_STAFF,			// 지팡이

	eWEAPON_STATE_MAX,				// 이 값은 연출과애니 연결에만 사용한다.

	/// 아래 두개는 애니 및 각종 상태에서 예외처리 된다.
	eWEAPON_STATE_SHEILD,			// 방패만
	eWEAPON_STATE_SWORD_SHEILD		= eWEAPON_STATE_SWORD + eWEAPON_STATE_SHEILD,			// 한손검 + 방패
};

enum ePLAYER_ANIMATIONTYPE
{
	/// 무기별 애니와 공통 애니를 구분하기 위한 정의
	ANITYPE_COMMON = 799,	

	/// 무기별
	ANITYPE_IDLE = 0,
	ANITYPE_IDLE2 = 10,

	ANITYPE_RUN = 20,

	ANITYPE_ATTACK1 = 30,
	ANITYPE_ATTACK2 = 40,
	ANITYPE_ATTACK3 = 50,

	ANITYPE_CASTING1 = 60,  
	ANITYPE_CASTING2 = 70,  
	ANITYPE_CASTING3 = 80,  

	ANITYPE_DAMAGE1 = 90,
	ANITYPE_DAMAGE2 = 100,

	ANITYPE_DIE1	= 110,
	ANITYPE_DIE2	= 120,

	ANITYPE_COMMON_SKILL_BEGIN = 130,
	ANITYPE_USESKILL1	= ANITYPE_COMMON_SKILL_BEGIN,
	ANITYPE_USESKILL2	= 140,
	ANITYPE_COMMON_SKILL_END = 150,

	ANITYPE_JUMP	= 180,

	ANITYPE_JUMP_END = 190,

	ANITYPE_SKILL_BEGIN = 201,
	ANITYPE_SWORD_SKILL = ANITYPE_SKILL_BEGIN,
	ANITYPE_LONGSWORD_SKILL = 301,
	ANITYPE_DOUBLESWORD_SKILL = 401,
	ANITYPE_SHORTSWORD_SKILL = 501,
	ANITYPE_GUN_SKILL = 601,
	ANITYPE_STAFF_SKILL = 701,
	ANITYPE_SKILL_END = 800,

	/// 공통 애니
	ANITYPE_PEACE_IDLE = 801,
	ANITYPE_PEACE_IDLE2 = 802,

	ANITYPE_ITEMPICK = 803,

	ANITYPE_SITDOWN = 804,
	ANITYPE_SITDOWN_IDLE = 805,
	ANITYPE_STANDUP = 806,

	ANITYPE_ENHANCED = 816,
	ANITYPE_ENHANCEDSUCCESS = 817,
	ANITYPE_ENHANCEDFAIL = 818,

	ANITYPE_DUELWIN = 819,
	ANITYPE_DUELLOSE = 820,

	ANITYPE_GATHERING1 = 831,
	ANITYPE_GATHERING2 = 832,

	ANITYPE_EMOTE_START = 900,

	/// 
	ANITYPE_VEHICLE_IDLE = 1001,
	ANITYPE_VEHICLE_IDLE2 = 1501,
	ANITYPE_VEHICLE_RUN = 2001,
};

enum ePLAYER_JOB_GROUP
{
	/// 직업계열의 최상위 정보..
	ePLAYER_JOBGROUP_FIGHTER	= 1,
	ePLAYER_JOBGROUP_MAGE,
	ePLAYER_JOBGROUP_RANGER,
	ePLAYER_JOBGROUP_SHAMAN,
	ePLAYER_JOBGROUP_WARRIOR,
	ePLAYER_JOBGROUP_ROGUE,

	ePLAYER_JOBGROUP_MAX = ePLAYER_JOBGROUP_ROGUE,
};

/// 플레이어 직업 정의
enum ePLAYER_JOB		/* unsigned short */
{ 
	/// 직업구분이없다.
	ePLAYER_NONEJOB = 0,

	/// 파이터 군
	ePLAYER_FIGHTER				= 1000,		/// 파이터
	ePLAYER_KNIGHT				= 1101,		/// 나이트
	ePLAYER_GLADIATOR			= 1102,		/// 글라디에이터
	ePLAYER_MERCENARY			= 1103,		/// 머셔너리
	ePLAYER_CRUSADER			= 1201,		/// 크루세이더
	ePLAYER_TEMPLAR				= 1202,		/// 템플러
	ePLAYER_SOULBLADER			= 1203,		/// 소울블레이더

	/// 메이지 군
	ePLAYER_MAGE				= 2000,		/// 메이지
	ePLAYER_MAGICIAN			= 2101,		/// 매지션
	ePLAYER_PRIEST				= 2102,		/// 프리스트
	ePLAYER_WIZARD				= 2201,		/// 위자드
	ePLAYER_HIGHPRIEST			= 2202,		/// 하이프리스트

	/// 원더러 군
	ePLAYER_WANDERER			= 3000,		/// 원더러
	ePLAYER_WINDWALKER			= 3101,		/// 윈드워커
	ePLAYER_SCOUT				= 3102,		/// 스카우트
	ePLAYER_ASSASSIN			= 3201,		/// 어쌔신
	ePLAYER_SNIPER				= 3202,		/// 스나이퍼

	/// 워록 군
	ePLAYER_WARLOCK				= 4000,		/// 워록
	ePLAYER_SHAMAN				= 4101,		/// 샤먼
	ePLAYER_SAGE				= 4102,		/// 세이지
	ePLAYER_SORCERER			= 4201,		/// 소서러
	ePLAYER_MENTALIST			= 4202,		/// 멘탈리스트

	/// 암즈맨 군
	ePLAYER_ARMSMAN				= 5000,		/// 암즈맨
	ePLAYER_VETERAN				= 5101,		/// 베테랑
	ePLAYER_BERSERK				= 5102,		/// 버서커
	ePLAYER_WARRIOR				= 5103,		///	워리어
	ePLAYER_CHAMPION			= 5201,		/// 챔피온
	ePLAYER_SLAYER				= 5202,		/// 슬레이어
	ePLAYER_HIGHLANDER			= 5203,		///	하이랜더

	/// 레인저 군
	ePLAYER_RANGER				= 6000,		/// 레인져
	ePLAYER_SHADOWWALKER		= 6101,		/// 섀도우워커
	ePLAYER_HUNTER				= 6102,		/// 헌터	
	ePLAYER_AVENGER				= 6201,		/// 어밴져
	ePLAYER_SHARPSHOOTER		= 6202,		/// 샤프슈터

	ePLAYERJOB_MAX,			/// db스크립트 insert용 최대값
};

enum ePLAYER_STATUS
{
	ePLAYER_STATUS_STR = 0,			/// 힘
	ePLAYER_STATUS_DEX,				/// 민첩
	ePLAYER_STATUS_CON,				/// 체력
	ePLAYER_STATUS_INT,				/// 지능
	ePLAYER_STATUS_WIS,				/// 지혜

	ePLAYER_STATUS_LEVEL,			/// 레벨 - 스크립트에서만 사용 = ePLAYER_STATUS_MAX

	ePLAYER_STATUS_LEVEL_MAX		/// 레벨 포함 최대 값
};

enum ePLAYER_STATUS_EXT
{
	ePLAYER_STATUS_EXT_ATK = 0,		/// 물리근접공격
	ePLAYER_STATUS_EXT_RATK,		/// 물리원거리공격
	ePLAYER_STATUS_EXT_DEF,			/// 물리 방어력
	ePLAYER_STATUS_EXT_MATK,		/// 마법 공격력
	ePLAYER_STATUS_EXT_MDEF,		/// 마법 방어력
	ePLAYER_STATUS_EXT_CRI,			/// 물리 공격 크리
	ePLAYER_STATUS_EXT_MCRI,		/// 마법 공격 크리
	ePLAYER_STATUS_EXT_MAXHP,		/// HP
	ePLAYER_STATUS_EXT_MAXMP,		/// MP
	ePLAYER_STATUS_EXT_RECOVHP,		/// HP 회복량
	ePLAYER_STATUS_EXT_RECOVMP,		/// MP 회복량
	ePLAYER_STATUS_EXT_HIT,			/// 물리명중률
	ePLAYER_STATUS_EXT_MHIT,		/// 마법명중률
	ePLAYER_STATUS_EXT_AVOID,		/// 회피율
	ePLAYER_STATUS_EXT_WEIGHT,		/// 무게
	ePLAYER_STATUS_EXT_ASPD,		/// 일반공격속도
	ePLAYER_STATUS_EXT_COOLTIME,	/// cooltime
	ePLAYER_STATUS_EXT_RANGE,		/// 근거리사정거리
	ePLAYER_STATUS_EXT_RRANGE,		/// 원거리(+마법)사정거리
	ePLAYER_STATUS_EXT_SPD,			/// 이동속도
	ePLAYER_STATUS_EXT_HEAL,		/// 힐회복량
	ePLAYER_STATUS_EXT_RECEIVEHEAL,	/// 받는 회복량 증가치
	ePLAYER_STATUS_EXT_MAVOID,		/// 마법회피율(저항력)
	ePLAYER_STATUS_EXT_CRIDEF,		/// 물리 공격 크리 감소
	ePLAYER_STATUS_EXT_MCRIDEF,		/// 마법 공격 크리 감소
	ePLAYER_STATUS_EXT_DISTRESS,	/// 어그로 감소 추가
	ePLAYER_STATUS_EXT_USE_MP,		/// MP 소모량 
	ePLAYER_STATUS_EXT_CASTING,		/// 캐스팅 시간 
	ePLAYER_STATUS_EXT_HUMAN_ATK,   /// 인간형 공격력
	ePLAYER_STATUS_EXT_ANIMAL_ATK,	/// 동물형 공격력
	ePLAYER_STATUS_EXT_PLANT_ATK,	/// 식물형 공격력
	ePLAYER_STATUS_EXT_BUG_ATK,		/// 곤충형 공격력
	ePLAYER_STATUS_EXT_SOUL_ATK,	/// 정령형 공격력
	ePLAYER_STATUS_EXT_UNDEAD_ATK,	/// 언데드형 공격력
	ePLAYER_STATUS_EXT_GIANT_ATK,	/// 거인형 공격력
	ePLAYER_STATUS_EXT_DRAGON_ATK,	/// 용형 공격력
	ePLAYER_STATUS_EXT_NORMAL_ATK,  /// 일반 몬스터형 공격력
	ePLAYER_STATUS_EXT_VETERAN_ATK, /// 정예 몬스터형 공격력
	ePLAYER_STATUS_EXT_ELITE_ATK,   /// 엘리트 몬스터형 공격력
	ePLAYER_STATUS_EXT_BOSS_ATK,    /// 보스 몬스터형 공격력
	ePLAYER_STATUS_EXT_CHARACTER_ATK,/// 캐릭터형 공격력

	ePLAYER_STATUS_EXT_ATKTOTAL,	/// 공격력 전체
	ePLAYER_STATUS_EXT_DEFTOTAL,	/// 방어력 전체
	ePLAYER_STATUS_EXT_ATKMin,		/// 물리근접 공격력 최소
	ePLAYER_STATUS_EXT_ATKMax,		/// 물리근접 공격력 최대
	ePLAYER_STATUS_EXT_RATKMin,		/// 물리원거리 공격력 최소
	ePLAYER_STATUS_EXT_RATKMax,		/// 물리원거리 공격력 최대
	ePLAYER_STATUS_EXT_MATKMin,		/// 마법 공격력 최소
	ePLAYER_STATUS_EXT_MATKMax,		/// 마법 공격력 최소

	ePLAYER_STATUS_EXT2_MAX,		/// 최소 최대를 포함한 최대값

	ePLAYER_STATUS_EXT_MAX = ePLAYER_STATUS_EXT_ATKTOTAL,	/// 최소 최대를 포함 하지 않은 최대값

	ePLAYER_STATUS_EXT_JUMP = 99,	/// 점프 체공시간
};

enum ePLAYER_STATUS_EXT_ADD
{
	ePLAYER_STATUS_EXT_ADD_EXPADDPER,		/// 경험치
	ePLAYER_STATUS_EXT_ADD_EXPADDPLUS,
	ePLAYER_STATUS_EXT_ADD_SXPADDPER,		/// 스킬경험치
	ePLAYER_STATUS_EXT_ADD_SXPADDPLUS,
	ePLAYER_STATUS_EXT_ADD_MONEYPER,		/// 테르
	ePLAYER_STATUS_EXT_ADD_PVPPOINTPER,		/// 전장포인트

	ePLAYER_STATUS_EXT_ADD_MAX,
};

enum ePLAYER_STATUS_DAMAGE
{
	/// 공격 성공 확률											
	ePLAYER_STATUS_DAMAGE_INSTANTPERHIT,			/// %순간 효과			
	ePLAYER_STATUS_DAMAGE_INSTANTPLUSHIT,			/// 순간 효과				
	/// 크리티컬 발생 확률(물리)									
	ePLAYER_STATUS_DAMAGE_PHYINSTANTPERCRI,			/// %순간 효과				
	ePLAYER_STATUS_DAMAGE_PHYINSTANTPLUSCRI,		/// 순간 효과				
	/// 크리티컬 발생 확률(마법)									
	ePLAYER_STATUS_DAMAGE_MAGICINSTANTPERCRI,		/// %순간 효과					
	ePLAYER_STATUS_DAMAGE_MAGICINSTANTPLUSCRI,		/// 순간 효과					
	/// 공격력(물리)									
	ePLAYER_STATUS_DAMAGE_PHYINSTANTPERATTACK,		/// %순간 효과					
	ePLAYER_STATUS_DAMAGE_PHYINSTANTPLUSATTACK,		/// 순간 효과					
	/// 공격력(마법)									
	ePLAYER_STATUS_DAMAGE_MAGICINSTANTPERATTACK,	/// %순간 효과					
	ePLAYER_STATUS_DAMAGE_MAGICINSTANTPLUSATTACK,   /// 순간 효과						
	/// 최종 데미지									
	ePLAYER_STATUS_DAMAGE_INSTANTPERFINAL,			/// %순간 효과				
	ePLAYER_STATUS_DAMAGE_INSTANTPLUSFINAL,			/// 순간 효과				

	ePLAYER_STATUS_DAMAGE_MAX,
};

enum ePLAYERIDLE
{
	ePLAYER_IDLE_WAR,
	ePLAYER_IDLE_PEACE,
};

enum //ePLAYERSTATE
{
	ePLAYER_STATE_SITDOWN = eOBJECT_STATE_MAX,
	ePLAYER_STATE_ITEMPICK,
	ePLAYER_STATE_GATHERING,
	ePLAYER_STATE_RUSH,
};

enum eIDLEFLAG
{
	eIDLE_NORMAL = 0,
	eIDLE_PEACE,
};

enum eSTOPFLAG
{
	eSTOP_NONE = 0,

	eSTOP_READYTAROT,		/// 개설자 대기상태
	eSTOP_OPENTAROT,		/// 개설자 오픈 상태
	eSTOP_ENTERTAROT,       /// 관람자 타롯
	eSTOP_USETAROT,			/// 참여자 타롯 사용중
	eSTOP_SELFTAROT,		/// 타롯 사용중(SELF)
	eSTOP_OPENSTALL,		/// 노점 오픈 상태
	eSTOP_USESTALL,			/// 노점 이용 상태
	eSTOP_NPCSPEECH,		/// NPC대화중
		
	//
	eSTOP_GAMEFINISH,		/// 게임 종료중
	eSTOP_ENHANCED,			/// 강화
	eSTOP_ITEMMIX,			/// 조합
	eSTOP_SKILLRESET,		/// 스킬초기화
	eSTOP_MAPCHANGE,		/// 맵체인지 상태 처리
	eSTOP_CHEATSTOP,		/// 치트 명령으로 인한 정지
	eSTOP_TELEPORTPORTAL,	/// 텔레포트 포탈 이용중
	eSTOP_THEMERESET,		/// 테마초기화
	eSTOP_SUMMON_PAPER,		/// 소환주문서 사용중
};

enum eRESURRECTION_TYPE
{
	eRESURRECTIONTYPE_STAND = 0,		/// 제자리 부활
	eRESURRECTIONTYPE_SAFE,				/// 안전지대 부활
	eRESURRECTIONTYPE_SKILL,			/// 스킬 부활
};

enum eFORCE_TYPE
{
	eFORCETYPE_NONE = 0,
	eFORCETYPE_FIRE,
	eFORCETYPE_WATER,
	eFORCETYPE_WIND,
	eFORCETYPE_EARTH,

	eFORCETYPE_MAX,
};

/// 구조체...
struct sPlayerInfo
{
	unsigned long	CharacterIdx;					/// 캐릭터 인덱스
	wchar_t			strName[MAX_NAME_BUFFER_SIZE];	/// 캐릭터 이름
	unsigned char	Level;							/// 현재 레벨
	unsigned char	Race;							/// 종족 - enum /*Race*/
	unsigned char	Gender;							/// 성별 - enum /*Gender*/
	ePLAYER_JOB		Job;							/// 직업

	unsigned long	HeadInfo[eHEAD_MAX];

	sPlayerInfo() 
	{
		::memset(this, 0, sizeof(sPlayerInfo));
		HeadInfo[eHEAD_FACE] = 1;
	}
};

struct sPlayerWearInfo
{
	/// 그래픽 정보를 포함한다.(HAT은 예외로 한다.)
	unsigned long	WearIdx[eWEAR_MAX];

	/// 코스튬 착용 정보
	unsigned long	CostumeIdx[eWEAR_MAX];

	/// 코스튬 악세서리
	unsigned long	CostumeAccIdx[eCOSTUME_MAX];

	/// 그래픽적 데이터의 변경이 필요 없는 부분
	//	unsigned long	Hat;
	unsigned long	Earring;
	unsigned long	Necklace;
	unsigned long	Brooch;
	unsigned long	Bracelet;
	unsigned long	Ring;

	sPlayerWearInfo()
	{
		::memset(this, 0, sizeof(sPlayerWearInfo));
	}
};

struct sPlayerWeaponInfo
{
	unsigned long	WeaponIdx[eHAND_ALL];
	unsigned char   WeaponEnhanced[eHAND_ALL];

	sPlayerWeaponInfo()
	{
		::memset(this, 0, sizeof(sPlayerWeaponInfo));
	}
};


struct sPlayerExrInfo
{
	unsigned long		RestHP;					/// 남은 생명력
	unsigned long		RestMP;					/// 남은 마나

	unsigned long		MaxHP;
	unsigned long		MaxMP;

	/// player는 이 변수에 담아져 일괄 발송되고 
	/// hero는 status계산 전이라 계산후에 따로(heroinfo) 발송한다.
	///////////////////////////////////////////////////////
	float				mBaseFixedSize;			/// 오브젝트 기본 크기
	unsigned short		mFixedObjectSizePer;	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	long				mMoveSpeed;				/// 이동 속도
	float				mAttackSpeed;			/// 기본공격 공속을 위한 변수
	unsigned long		mChgMonsterIdx;			/// 몬스터 외형 변신 몬스터 고유번호
	unsigned long		mVehicleIdx;			/// 탈것
	///////////////////////////////////////////////////////

	/// 071109 PKH 평화 대기상태 - 서버에서 처리는 하지않고 클라이언트에 알려주기만함
	unsigned char		mStateIdle;				/// eIDLEFLAG
	/// 070601 PKH 플레이어 상태 
	unsigned char		mState;					/// eOBJECTSTATE
	/// 071109 PKH 정지 상태일경우 세부사항
	unsigned char		mStateStop;				/// eSTOPFLAG

	unsigned long		mCommunitySkillIdx;

	/// 결투중인지 여부
	bool				mIsDuelIdx;

	unsigned char		mForceType;				/// eFORCE_TYPE

	/// pvp
	unsigned char		mPvPDMTeamType;			/// ePVPDM_TEAM_TYPE


	/// 설정된 호칭 정보
	unsigned long		mTitleIndex;

	/// 파티 인덱스
	unsigned long		mPartyIndex;

	/// 파티 연합 인덱스
	unsigned long		mPartyUnionIndex;

	/// 길드
	unsigned long		mGuildIndex;			
	wchar_t				mGuildName[MAX_GUILDNAME_BUFFER_SIZE];	/// 길드명
    unsigned char		mGuildPosition;							/// 길드 직위 인덱스

	// 길드마크정보
	tm					mGuildMarkDate;
	bool				mUseGuildMark;							/// 길드마크 사용 여부
	
	/// 오늘의 한마디
	wchar_t				mTodayWord[MAX_TODAYWORD_BUFFER_SIZE];	
	unsigned long		mTodayColor;							/// 색

	sPlayerExrInfo()
	{
		memset(this, 0, sizeof(sPlayerExrInfo));
	}

};

struct sMakeSkill
{
	unsigned char		mIndex;		// 스킬 인덱스
	unsigned char		mStep;		// 단계
	unsigned long		mExp;		// 숙련도

	sMakeSkill()
	{
		memset(this, 0, sizeof(sMakeSkill));
	}
};

struct sHeroInfo
{
	unsigned long		Exp;					// 현재 경험치

	/// --스킬과 관련된 포인트--
	unsigned char		SkillLevel;				/// 스킬 경험치로 레벨업을 함
	unsigned int		SkillExp;				/// 현재 스킬경험치
	unsigned short		SkillPointRemain;		/// 잔여 스킬포인트
	unsigned short		SkillPointTotal;		/// 총 획득 스킬 포인트

	/// 세력 포인트
	unsigned long		mFirePoint;
	unsigned long		mFireFriendly;
	unsigned long		mWaterPoint;
	unsigned long		mWaterFriendly;
	unsigned long		mWindPoint;
	unsigned long		mWindFriendly;
	unsigned long		mEarthPoint;
	unsigned long		mEarthFriendly;

	/// 타로 포인트
	unsigned long		TarotPoint;

	/// 전문 기술
	sMakeSkill			MakeSkill1;
	sMakeSkill			MakeSkill2;

    unsigned short      BagEnd;
    unsigned short      WearEnd;
    unsigned short      WareHouseEnd;
    unsigned short      MakeSkillEnd;
	
	/// 하락된 경험치량 - 복구 가능경험치
	unsigned long		mLoseExp;

	sHeroInfo()
	{
		memset(this, 0, sizeof(sHeroInfo));
	}

};

struct sStatus1
{
	float			mStr;			/// 힘
	float			mDex;			/// 민첩
	float			mCon;			/// 체력
	float			mInt;			/// 지능
	float			mWis;			/// 지혜

	sStatus1()
	{
		memset(this, 0, sizeof(sStatus1));
	}
};

struct sStatus2
{
	float			mPhysicMinAttack;	/// 물리:최소근거리공격력
	float			mPhysicMaxAttack;	/// 물리:최대근거리공격력
	float			mPhysicMinRangeAttack;	/// 물리:최소원거리공격력
	float			mPhysicMaxRangeAttack;	/// 물리:최대원거리공격력
	float			mPhysicDefense;		/// 물리:방어력
	float			mMagicMinAttack;	/// 마법:최소공격력
	float			mMagicMaxAttack;	/// 마법:최대공격력
	float			mMagicDefense;		/// 마법:방어력
	float			mPhysicCritical;	/// 물리:크리티컬
	float			mMagicCritical;		/// 마법:크리티컬
//	float			mMaxHP;				/// 증가:최대HP 
//	float			mMaxMP;				/// 증가:최대MP 
	float			mRecovHP;			/// 증가:회복HP%
	float			mRecovMP;			/// 증가:회복MP%
	float			mPhysicAttackRate;	/// 물리:공격률
	float			mMagicAttackRate;	/// 마법:공격률
	float			mPhysicAvoid;		/// 증가:회피
	float			mMagicAvoid;		/// 증가:회피
	float			mMaxWeight;			/// 무게
	float			mAttackSpeed;		/// 일반공격속도
	float			mCooltime;			/// cooltime
	float			mInRange;			/// 근거리사정거리
	float			mOutRange;			/// 원거리(+마법)사정거리
	float			mMoveSpeed;			/// 이동속도
	float			mHeal;				/// 회복량
	float			mReceiveHeal;		/// 받는 회복량 증가
	float			mPhyCriDef;			/// 물리 크리 감소(방호)
	float			mMagCriDef;			/// 마법 크리 감소(방호)
	float			mDistress;			/// 어그로 수치  -아직 작업 안됨[4/14/2010 Jo_Client]
	float			mUseMP;				/// MP 소모량 감소
	float			mCasting;			/// 캐스팅감소	 -아직 작업 안됨[4/14/2010 Jo_Client]
	float			mHumanTypePer;	    /// 증가:인간형 공격력(%)
	float			mAnimalTypePer;		/// 증가:동물형 공격력(%)
	float			mPlantTypePer;		/// 증가:증가:식물형 공격력(%)
	float			mBugTypePer;		/// 증가:곤충형 공격력(%)
	float			mSoulTypePer;		/// 증가:정령형 공격력(%)
	float			mUndeadTypePer;		/// 증가:언데드형 공격력(%)
	float			mGiantTypePer;		/// 증가:거인형 공격력(%)
	float			mDragonTypePer;		/// 증가:용형 공격력(%)
	float			mNormalTypePer;		/// 증가:일반 몬스터 공격력(%)
	float			mVeteranTypePer;	/// 증가:정예 몬스터 공격력(%)
	float			mEliteTypePer;		/// 증가:엘리트 몬스터 공격력(%)
	float			mBossTypePer;		/// 증가:보스 몬스터 공격력(%)

	sStatus2()
	{
		memset(this, 0, sizeof(sStatus2));
	}
};

struct sStatusEtc
{
	float			mExpAddPer;			/// exp 증가 %
	float			mExpAddPlus;		/// exp 증가 +
	float			mSxpAddPer;			/// sxp 증가 %
	float			mSxpAddPlus;		/// sxp 증가 +
	float			mMoneyAddPer;		/// 테르 획득 증가 %
	float			mPvPPointAddPer;	/// 전장 포인트 획득 증가 %
};

struct sDamageCalc
{
	/// 공격 성공 확률											
	long		mInstantPerHit;			/// %순간 효과			
	long		mInstantPlusHit;		/// 순간 효과				
	/// 크리티컬 발생 확률(물리)								
	long		mPhyInstantPerCri;		/// %순간 효과				
	long		mPhyInstantPlusCri;		/// 순간 효과				
	/// 크리티컬 발생 확률(마법)									
	long		mMagicInstantPerCri;	/// %순간 효과					
	long		mMagicInstantPlusCri;	/// 순간 효과					
	/// 공격력(물리)									
	long		mPhyInstantPerAttack;	/// %순간 효과					
	long		mPhyInstantPlusAttack;	/// 순간 효과					
	/// 공격력(마법)									
	long		mMagicInstantPerAttack;	/// %순간 효과					
	long		mMagicInstantPlusAttack;	/// 순간 효과						
	/// 최종 데미지									
	long		mInstantPerFinal;		/// %순간 효과				
	long		mInstantPlusFinal;		/// 순간 효과	
};

struct sMoveMsg
{
	bool				mMsgError;
	char				mProtocol;
	NiPoint2			mStartPos;
	NiPoint2			mDestPos;
	float				mRange;
	sObject				mTargetInfo;
	NiPoint2			mTargetPos;
};

/// GM 이벤트 정보 구조체
struct sGMEventInfo
{
	unsigned long index;		// DB Index;
	unsigned long influenceIdx; // 효과번호
	time_t		  startTime;	// 시작시간
	time_t		  validThru;	// 유효만기
};

typedef tList<sGMEventInfo> cGMEventList;

/// 070418 이동 "CommonNetworkMsgStruct.h(219)"
/// struct MSG_PLAYERINFO : public MSGROOT


/// 캐릭터 선택창에 보여질 정보 구조체
struct MSG_CHARACTER_BASEINFO
{
	sPlayerInfo			PlayerInfo;					/// 캐릭터 정보
	sPlayerWeaponInfo	WeaponInfo;					/// 무기   정보
	sPlayerWearInfo     WearInfo;					/// 방어구 정보

	char				StandIdx;					/// 캐릭터 순서
	unsigned short		MapNum;						/// 맵     번호
	long		        DelWaitTime;				/// 삭제대기시간
};

/// 캐릭터 선택창으로 이동시 보내는 network msg 구조체
struct MSG_CHARACTER_LIST : public MSG_ERROR
{
	unsigned char			CharacterNum;	/// 만들어진 캐릭터 수
	unsigned char           StandIdx;       /// 마지막 사용 캐릭터
	MSG_CHARACTER_BASEINFO	BaseInfo[1];	/// 캐릭터 정보	

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength()	{ return sizeof(MSG_CHARACTER_LIST) - sizeof(MSG_CHARACTER_BASEINFO) + (CharacterNum * sizeof(MSG_CHARACTER_BASEINFO)); }
};

/// 네트워크 구조체..
struct MSG_PLAYERINFO : public MSG_ERROR
{
	sPlayerInfo			PlayerInfo;		/// 캐릭터 기본
	sPlayerWeaponInfo	WeaponInfo;		/// 캐릭터 무기
	sPlayerWearInfo		WearInfo;		/// 캐릭터 장비
	sPlayerExrInfo		Exrinfo;		/// 캐릭터 ...  (070122 PKH 기본정보 외에 추가 정보)

	unsigned short		MapNum;			/// 맵 번호
	float        		XPos;			/// X 위치
	float				YPos;			/// Y 위치
	unsigned char		MapMode;

	unsigned char		IndunType;		/// SERVER_TYPE
	float				RotAngle;		/// 
};

struct MSG_HEROINFO : public MSG_ERROR
{
	sHeroInfo			mHeroInfo; 
	sPlayerExrInfo		mExrInfo;
	bool				mCanTMove;
	bool				mCanTSkillPhy;
	bool				mCanTSkillMag;
	bool				mSleep;
	bool				mStun;
	bool				mAvoidMonLook;
	unsigned long		mChgMonsterIdx;
};

/*
struct MSG_MOVE_INFO : public MSG_ERROR
{
	unsigned long characterIdx;		/// characteridx는 NM_PLAYER_MOVE_SYN에서만 쓰시오
	int		nStartX;
	int		nStartZ;
	int		nEndX;
	int		nEndZ;

	/// 070216 PKH 서버와 클라이언트 동기화된 시계
	unsigned long	mClientTimer;	/// characteridx는 NM_PLAYER_MOVE_RES에서만 쓰시오

	void	SetStartPos( float fx, float fz )	{ nStartX = (int)fx; nStartZ = (int)fz; }
	void	SetEndPos( float fx, float fz )		{ nEndX = (int)fx; nEndZ = (int)fz; }
};
*/

struct MSG_REQ_MOVE : public MSGROOT
{
	float	startX;
	float	startY;
	float	destX;
	float	destY;
	unsigned long	mClientTime;
};
struct MSG_REQ_ACTIONMOVE : public MSGROOT
{
	float	range;

	float	startX;
	float	startY;

	/// 범위 검사 기준
	float	targetX;
	float	targetY;

	float	destX;
	float	destY;

	sObject	targetInfo;
	unsigned long	mClientTime;
};
/// 키보드 이동
struct MSG_REQ_KEYMOVE : public MSGROOT
{
	float			mStartX;
	float			mStartY;
	float			mDestX;
	float			mDestY;
	unsigned long	mClientTime;
};


struct MSG_SYN_MOVE : public MSGROOT
{
	unsigned long	characterIdx;
	float			destX;
	float			destY;
	unsigned short	moveSpeed;	
};
struct MSG_SYN_ACTIONMOVE : public MSG_SYN_MOVE
{
	/// 범위
	float			range;

	/// 범위 검사 기준
	float			targetX;
	float			targetY;

	sObject			targetInfo;
};
struct MSG_SYN_KEYMOVE : public MSGROOT
{
	unsigned long	mCharacterIdx;
	float			mDestX;
	float			mDestY;
	long			mMoveSpeed;	
};


struct MSG_RES_KEYMOVE : public MSGROOT
{
	float			mCharacterPosX;
	float			mCharacterPosY;
};


struct MSG_REQ_MOVE_STOP : public MSGROOT
{
	float			mClientPosX;
	float			mClientPosY;
	unsigned long	mClientTime;
};
struct MSG_RES_MOVE_STOP : public MSGROOT
{
	float			mCharacterPosX;
	float			mCharacterPosY;
};
struct MSG_SYN_MOVE_STOP : public MSGROOT
{
	unsigned long	mCharacterIdx;
	float			mCharacterPosX;
	float			mCharacterPosY;
};


struct MSG_SERVERPOS_TEST : public MSGROOT
{
	unsigned long	mCharacterIdx;
	float			mServerPosX;
	float			mServerPosY;
};


struct MSG_SYN_MOVESPEED : public MSGROOT
{
	unsigned long	mCharacterIdx;
	long			moveSpeed;			/// 이동 속도
};

struct MSG_RES_ERR_MOVE : public MSGROOT
{
	float			destX;
	float			destY;
};

struct MSG_SYN_ATTACKSPEED : public MSGROOT
{
	unsigned long	mCharacterIdx;
	float			mAttackSpeed;	
};

#define ERROR_CHARACTER_NAME_SUCCESS	0	/// 성공.
#define ERROR_CHARACTER_NAME_EXIST		1	/// 실패-중복.
#define ERROR_CHARACTER_NAME_RULE		2	/// 실패-이름 룰 오류.
#define ERROR_CHARACTER_NAME_FAIL		3	/// 실패.

struct MSG_REQ_CHARACTER_NAME : public MSGROOT
{
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];
};
struct MSG_RES_CHARACTER_NAME : public MSG_ERROR
{};

#define ERROR_CHARACTER_CREATE_SUCCESS		0	/// 성공.
#define ERROR_CHARACTER_CREATE_EXIST		1	/// 실패-중복.
#define ERROR_CHARACTER_CREATE_OVER			2	/// 실패-더이상 생성할 수 없음.
#define ERROR_CHARACTER_CREATE_STAND_IDX	3	/// 실패-STAND IDX 오류.
#define ERROR_CHARACTER_CREATE_ITEMS		4	/// 실패-기본 아이템 지급 오류.
#define ERROR_CHARACTER_CREATE_SKILLS		5	/// 실패-기본 스  킬 지급 오류.
#define ERROR_CHARACTER_CREATE_RULE			6	/// 실패-생성 룰 오류(이름,종족,성별,직업,헤어,얼굴,헤어칼라...).
#define ERROR_CHARACTER_CREATE_SYSTEM		7	/// 실패-시스템 관리자.
#define ERROR_CHARACTER_CREATE_FAIL			8	/// 실패.

struct MSG_REQ_CHARACTER_CREATE : public MSGROOT
{
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];
	char			Race;
	char			Gender;

	unsigned short	Job;
	unsigned char	Hair;
	unsigned short	HairColor;
	unsigned char	Face;

	unsigned char   StandIdx;
};
struct MSG_RES_CHARACTER_CREATE : public MSG_ERROR
{};

#define ERROR_CHARACTER_DELETE_SUCCESS		0	/// 성공.
#define ERROR_CHARACTER_DELETE_FAIL			1	/// 실패-
#define ERROR_CHARACTER_DELETE_GUILDMASTER	2	/// 실패-길드 마스터

struct MSG_REQ_CHARACTER_DELETE : public MSGROOT
{
	unsigned long		CharacterIdx;	/// 캐릭터 인덱스
};

struct MSG_RES_CHARACTER_DELETE : public MSG_ERROR
{
	unsigned long		StandIdx;		/// 캐릭터 생성위치
};


#define ERROR_CHARACTER_DELETE_WAIT_SUCCESS		    0	/// 성공.
#define ERROR_CHARACTER_DELETE_WAIT_FAIL	    	1	/// 실패-
#define ERROR_CHARACTER_DELETE_WAIT_GUILDMASTER	    2	/// 실패-길드 마스터

struct MSG_REQ_CHARACTER_DELETE_WAIT : public MSGROOT
{
	unsigned long		CharacterIdx;	/// 캐릭터 인덱스
};

struct MSG_RES_CHARACTER_DELETE_WAIT : public MSG_ERROR
{
	unsigned long		StandIdx;		/// 캐릭터 생성위치
	long		        DelWaitTime;	/// 캐릭터 삭제 기한
};


#define ERROR_CHARACTER_DELETE_CANCEL_SUCCESS		0	/// 성공
#define ERROR_CHARACTER_DELETE_CANCEL_FAIL			1	/// 실패

struct MSG_REQ_CHARACTER_DELETE_CANCEL : public MSGROOT
{
	unsigned long		CharacterIdx;	/// 캐릭터 인덱스
};

struct MSG_RES_CHARACTER_DELETE_CANCEL : public MSG_ERROR
{
	unsigned long		StandIdx;		/// 캐릭터 생성위치
};

struct MSG_REQ_CHARACTER_SELECT : public MSGROOT
{
	unsigned char		StandIdx;
};

struct MSG_REQ_GAMEIN : public MSGROOT
{};

struct MSG_RES_GAMEIN : public MSG_ERROR
{
	unsigned short channelNum;
};


#define ERROR_GAMEFINISH_SUCCESS		0	// 성공
#define ERROR_GAMEFINISH_FAIL			1	// 실패


/// 070625 PKH 플레이어 레벨
struct MSG_RES_LEVEL : public MSGROOT
{
	unsigned char		mLevel;
	unsigned long		mExp;
	unsigned short		mCon;		/// 스텟 증가치
	unsigned short		mDex;
	unsigned short		mInt;
	unsigned short		mStr;
	unsigned short		mWis;
	unsigned long		mHP;
	unsigned long		mMaxHP;
	unsigned long		mMP;
	unsigned long		mMaxMP;
};

struct MSG_SYN_LEVEL : public MSGROOT
{
	unsigned long		mCharacterIdx;
	unsigned char		mLevel;
	unsigned long		mHP;
	unsigned long		mMaxHP;
	unsigned long		mMP;
	unsigned long		mMaxMP;

};

/// 070625 PKH 플레이어 스킬 레벨
struct MSG_RES_SKILLLEVEL : public MSGROOT
{
	unsigned char		mSkillLevel;
	unsigned long		mSkillExp;
	unsigned short		mSkillPoint;
	unsigned short		mSkillPointTotal;
};

struct MSG_SYN_SKILLLEVEL : public MSGROOT
{
	unsigned long		mPlayerIdx;
};

struct MSG_RES_EXP : public MSGROOT
{
	unsigned long	mExp;			/// 얻은 경험치
	unsigned long	mTotalExp;		/// 플레이어의 경험치 전체량
};

struct MSG_RES_SXP : public MSGROOT
{
	unsigned long	mSxp;			/// 얻은 스킬 경험치
	unsigned long	mTotalSxp;		/// 플레이어의 스킬 경험치 전체량
};

struct MSG_RES_PARTY_EXP : public MSGROOT
{
	unsigned long	mExp;			/// 얻은 경험치
	unsigned long	mPartyExp;		/// 파티원에 따른 추가 경험치
	unsigned long	mTotalExp;		/// 플레이어의 경험치 전체량
};

struct MSG_RES_PARTY_SXP : public MSGROOT
{
	unsigned long	mSxp;			/// 얻은 경험치
	unsigned long	mPartySxp;		/// 파티원에 따른 추가 경험치
	unsigned long	mTotalSxp;		/// 플레이어의 경험치 전체량
};

struct MSG_CHARACTERIDX : public MSGROOT
{
	unsigned long		mCharacterIdx;
};


struct MSG_REQ_PLAYER_MAPCHANGE : public MSGROOT
{
	unsigned long	mMapChangePosIndex;
};

const int ERROR_RES_MAPCHANGE_SUCCESS		= 0;	/// 성공
const int ERROR_RES_MAPCHANGE_STATE			= 1;	/// 맵이동 불가 상태
const int ERROR_RES_MAPCHANGE_SCRIPT		= 2;	/// 스크립트정보오류
const int ERROR_RES_MAPCHANGE_POSITION		= 3;	/// 거리좌표 오류(너무멀리있음)
const int ERROR_RES_MAPCHANGE_NOTPARTY		= 4;	/// 같은 파티가 아님
const int ERROR_RES_MAPCHANGE_NOTPORTAL		= 5;	/// 포탈 없음

struct MSG_RES_PLAYER_EQUIP_WEAR : public MSGROOT
{
	unsigned char		mType;				/// eWEAR_TYPE
	unsigned long		mWearInfo;
};
struct MSG_SYN_PLAYER_EQUIP_WEAR : public MSGROOT
{
	unsigned long		mCharacterIdx;
	unsigned char		mType;				/// eWEAR_TYPE
	unsigned long		mWearInfo;
};

struct MSG_RES_PLAYER_EQUIP_WEAPON : public MSGROOT
{
	unsigned char       mWeaponState;		/// eWEAPON_STATE
	sPlayerWeaponInfo	mWeaponInfo;
};
struct MSG_SYN_PLAYER_EQUIP_WEAPON : public MSGROOT
{
	unsigned long		mCharacterIdx;
	unsigned char       mWeaponState;		/// eWEAPON_STATE
	sPlayerWeaponInfo	mWeaponInfo;
};


/* 코스튬 장비
*/
struct MSG_RES_PLAYER_COSTUME_WEAR : public MSGROOT
{
    unsigned char       mType;
	unsigned long		mWearInfo;
};
struct MSG_SYN_PLAYER_COSTUME_WEAR : public MSGROOT
{
	unsigned long		mCharacterIdx;
    unsigned char       mType;
	unsigned long		mWearInfo;
};

struct MSG_RES_PLAYER_COSTUME_ACC : public MSGROOT
{
    unsigned char       mType;
	unsigned long		mWearInfo;
};
struct MSG_SYN_PLAYER_COSTUME_ACC : public MSGROOT
{
	unsigned long		mCharacterIdx;
    unsigned char       mType;
	unsigned long		mWearInfo;
};


#define MAX_SHORTCUT_SIZE			50	// 10 x 5(Tab)

enum eSHORTCUT_TYPE
{
	eSHORTCUT_NONE = 0,					/// 없음
	eSHORTCUT_ITEM,						/// 아이템
	eSHORTCUT_SKILL,					/// 스킬
	eSHORTCUT_MONSKILL,					/// 몬스터스킬
	eSHORTCUT_COMMU,					/// community
	eSHORTCUT_UNKNOWN,					/// 오류
};

struct sShortcut
{
	char          mType;				/// 종류(0 아이템, 1 스킬, ... )
	unsigned long mIndex;				/// 인덱스
	unsigned long mExData;				/// 공통 정보(인벤토리 고유번호, 스킬 스텝)
};

struct MSG_REQ_PLAYER_SHORTCUT_LIST : public MSGROOT
{};

struct MSG_RES_PLAYER_SHORTCUT_LIST : public MSGROOT
{
	sShortcut mShortcut[ MAX_SHORTCUT_SIZE ];
};

struct MSG_REQ_PLAYER_SHORTCUT_UPDATE : public MSGROOT
{
	sShortcut mShortcut[ MAX_SHORTCUT_SIZE ];
};

struct MSG_RES_PLAYER_SHORTCUT_UPDATE : public MSGROOT
{};


struct MSG_REQ_PLAYER_OPTION_VALUE : public MSGROOT
{};
struct MSG_RES_PLAYER_OPTION_VALUE : public MSGROOT
{
	long option1;								/// 옵션값(32BIT)
};

#define ERROR_PLAYER_OPTION_UPDATE_SUCCESS	0	/// 성공
#define ERROR_PLAYER_OPTION_UPDATE_FAIL		1	/// 실패

struct MSG_REQ_PLAYER_OPTION_UPDATE : public MSGROOT
{
	long option1;								/// 옵션값(32BIT)
};
struct MSG_RES_PLAYER_OPTION_UPDATE : public MSG_ERROR
{};


struct MSG_SYN_PLAYER_HP : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mHp;
	unsigned long	mMaxHp;
	bool			mDie;
};

struct MSG_SYN_PLAYER_MP : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mMp;
	unsigned long	mMaxMp;
};

struct MSG_SYN_PLAYER_MAXHP : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mMaxHp;
};

struct MSG_SYN_PLAYER_MAXMP : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mMaxMp;
};

/// 부활
#define ERROR_PLAYER_RESURRECTION_SUCCESS		0	/// 성공.
#define ERROR_PLAYER_RESURRECTION_FAIL			1	/// 실패

struct MSG_REQ_PLAYER_RESURRECTION : MSGROOT
{
	unsigned char	mFlag;
};

struct MSG_RES_PLAYER_RESURRECTION : MSG_ERROR
{
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
	unsigned long	mExpDown;		/// 경험치 감소량
	unsigned long	mPlayerExp;		/// 경험치 보유량
	float			mPosX;
	float			mPosY;
};

struct MSG_RES_PLAYER_CASHEXP_RESURRECTION : MSG_RES_PLAYER_RESURRECTION
{
	unsigned long	mInfluenceIdx;	/// 효과 번호
	unsigned char	mCashExpCnt;	/// 경험치 감소 방지 남은횟수
};

struct MSG_SYN_PLAYER_RESURRECTION : MSG_ERROR
{
	unsigned long	mCharacterIdx;
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
	float			mPosX;
	float			mPosY;
};

///
struct MSG_REQ_PLAYER_STATE_IDLE : public MSGROOT
{
	unsigned char	mPlayerStateIdle;
};

struct MSG_SYN_PLAYER_STATE_IDLE : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned char	mPlayerStateIdle;
};

struct MSG_REQ_PLAYER_STATE_SITDWON : public MSGROOT
{
	bool			mSitDown;
};

struct MSG_RES_PLAYER_STATE_SITDWON : public MSG_ERROR
{
	bool			mSitDown;
};

struct MSG_SYN_PLAYER_STATE_SITDWON : public MSGROOT
{
	unsigned long	mCharacterIdx;
	bool			mSitDown;
};

struct MSG_RES_PLAYER_MONEY : public MSGROOT
{
	unsigned long mMoney;
	unsigned char mObjectType;		/// eOBJECTTYPE
	long          mAddMoney;
};

struct MSG_RES_PLAYER_DEPOSIT : public MSGROOT
{
	unsigned long mDeposit;
	unsigned char mObjectType;		/// eOBJECTTYPE
	long          mAddDeposit;
};

struct MSG_RES_PLAYER_TAROT_POINT : public MSGROOT
{
	unsigned long mPoint;
	long          mAddPoint;
};

struct MSG_RES_PLAYER_PVP_POINT : public MSGROOT
{
	unsigned char mForceType;
	unsigned long mPoint;
	long          mAddPoint;
};

struct MSG_RES_PLAYER_SP : public MSGROOT
{
	unsigned short	mSP;
	unsigned short	mTotalSP;

	unsigned short rowCount;
	unsigned short usedSP[1];
};

struct MSG_REQ_PLAYER_COMMUNITYSKILL : public MSGROOT
{
	unsigned long	mCommunitySkillIdx;
};

struct MSG_RES_PLAYER_COMMUNITYSKILL : public MSG_ERROR
{
	unsigned long	mCommunitySkillIdx;
};

struct MSG_SYN_PLAYER_COMMUNITYSKILL : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mCommunitySkillIdx;
};

struct MSG_SYN_PLAYER_SIZE : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned short	mScale;
};

//struct MSG_SYN_PLAYER_STATUS : public MSGROOT
//{
//	unsigned long	mCharacterIdx;
//	unsigned short	mScale;
//	long			mMoveSpeed;	
//	float			mAttackSpeed;	
//	bool			mIsCantMove;
//	bool			mIsSleep;
//	bool			mIsStun;
//	unsigned long	mMonsterIdx;
//	unsigned long	mVehicleIdx;
//};

struct MSG_SYN_PLAYER_CANTMOVE : public MSGROOT
{
	unsigned long	mCharaterIdx;
	NiPoint2		mCharacterPos;
	bool			mIsCantMove;
};

struct MSG_SYN_PLAYER_SLEEP : public MSGROOT
{
	unsigned long	mCharaterIdx;
	NiPoint2		mCharacterPos;
	bool			mIsSleep;
};

struct MSG_SYN_PLAYER_STUN : public MSGROOT
{
	unsigned long	mCharaterIdx;
	NiPoint2		mCharacterPos;
	bool			mIsStun;
};

struct MSG_SYN_PLAYER_CANTSKILL : public MSGROOT
{
	unsigned char	mAttributeType;
	bool			mIsCantSkill;
};

struct MSG_REQ_PLAYER_STATICFACE : public MSGROOT
{
	unsigned long	mStaticFaceIdx;
};

struct MSG_SYN_PLAYER_STATICFACE : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mStaticFaceIdx;
};

struct MSG_SYN_PLAYER_CHG_MONSTER : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mMonsterIdx;
};

struct MSG_SYN_PLAYER_FORCEVALUE : public MSGROOT
{
	unsigned char	mForceType;
	unsigned char	mValueType;				/// 1:point / 2:friendly
	unsigned long	mValue;
};

struct MSG_SYN_PLAYER_FORCE : public MSGROOT
{
	unsigned long	mPlayerIdx;
	unsigned char	mForceType;
};

struct MSG_SYN_PLAYER_JUMP : public MSGROOT
{
	unsigned long	mPlayerIdx;
};

const int ERROR_PLAYER_ITEMMAPCHANGE_SUCCESS = 0;
const int ERROR_PLAYER_ITEMMAPCHANGE_FAIL = 1;

struct MSG_RES_JOB : public MSGROOT
{
	unsigned short		mJob;				/// ePLAYER_JOB
	unsigned char		mSkillListCnt;		/// 전직시 자동습득 스킬 목록 갯수
	unsigned long		mSkillClassIdx[1];	/// 전직시 자동습득 스킬 목록

	unsigned long GetMsgLength() { return sizeof(MSG_RES_JOB) - sizeof(mSkillClassIdx) + (mSkillListCnt * sizeof(unsigned long));} 
};

struct MSG_SYN_JOB : public MSGROOT
{
	unsigned long		mPlayerIdx;
	unsigned short		mJob;
};

/// 호칭
struct MSG_RES_PLAYER_TITLEADD : public MSGROOT
{
	unsigned long	mTitleIndex;			/// 호칭 추가
};

struct MSG_RES_PLAYER_TITLELIST : public MSGROOT
{
	long			rowCount;
	unsigned long	mTitleIndex[1];			/// 호칭 리스트
};

/// 호칭 설정 변경
struct MSG_REQ_PLAYER_TITLE_SET : public MSGROOT
{
	unsigned long	mTitleIndex;
};

const int ERROR_PLAYER_TITLE_SET_SUCCESS = 0;
const int ERROR_PLAYER_TITLE_SET_FAIL = 1;

struct MSG_RES_PLAYER_TITLE_SET : public MSG_ERROR
{};

struct MSG_SYN_PLAYER_TITLE_SET : public MSGROOT
{
	unsigned long	mCharacterIndex;		/// 캐릭터
	unsigned long	mTitleIndex;			/// 설정 호칭 ( 0 : 삭제 )
};

struct MSG_SYN_PLAYER_PARTY_SET : public MSGROOT
{
	unsigned long	mCharacterIndex;		/// 캐릭터
	unsigned long	mPartyIndex;			/// 파티인덱스 (0 : 없음 )
};

struct MSG_SYN_PLAYER_PARTYUNION_SET : public MSGROOT
{
	unsigned long	mCharacterIndex;		/// 캐릭터
	unsigned long	mPartyUnionIndex;		/// 파티연합인덱스 (0 : 없음 )
};

/// 길드 설정 변경
struct MSG_SYN_PLAYER_GUILD_SET : public MSGROOT
{
	unsigned long	mCharacterIndex;					/// 캐릭터
	unsigned long	mGuildIndex;						/// 길드인덱스 (0 : 없음 )
	unsigned char	mPosition;							/// 길드직위
	tm				mMarkRegistDate;					/// 길드 마크 등록일
	bool			mUseGuildMark;						/// 길드 마크 사용여부
	wchar_t			mName[MAX_GUILDNAME_BUFFER_SIZE];	/// 길드명
};

struct MSG_SYN_PLAYER_GUILDMARK_CHANGE : public MSGROOT
{
	unsigned long	mCharacterIndex;					/// 캐릭터
	tm				mMarkRegistDate;					/// 길드 마크 등록일
	bool			mUseGuildMark;						/// 길드 마크 사용여부
};

struct MSG_SYN_PLAYER_VEHICLE : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mVehicleIdx;
};

struct MSG_REQ_PLAYER_VEHICLEDELETE : public MSGROOT
{
	unsigned long	mInfUniqueIdx;
	unsigned long	mItemDbIdx;
};

const int ERROR_PLAYER_TODAYWORD_CHANGE_SUCCESS = 0;
const int ERROR_PLAYER_TODAYWORD_CHANGE_INIT    = 1;
const int ERROR_PLAYER_TODAYWORD_CHANGE_FAIL    = 2;

/// 오늘의한마디 변경
struct MSG_REQ_PLAYER_TODAYWORD_CHANGE : public MSGROOT
{
	unsigned long	mWordColor;							/// 텍스트 컬러
	wchar_t			mWord[MAX_TODAYWORD_BUFFER_SIZE];		/// 오늘의한마디
};

struct MSG_RES_PLAYER_TODAYWORD_CHANGE : public MSG_ERROR
{
	unsigned long	mWordColor;							/// 텍스트 컬러
	wchar_t			mWord[MAX_TODAYWORD_BUFFER_SIZE];	/// 오늘의한마디
};

struct MSG_SYN_PLAYER_TODAYWORD_CHANGE : public MSGROOT
{
	unsigned long	mCharacterIndex;					/// 캐릭터
	unsigned long	mWordColor;							/// 텍스트 컬러
	wchar_t			mWord[MAX_TODAYWORD_BUFFER_SIZE];	/// 오늘의한마디
};

struct MSG_REQ_PLAYER_SKILLRESET : public MSGROOT
{
	unsigned short	mSlotNum;				/// 인벤슬롯 번호
};

#define ERROR_SKILLRESET_SUCCESS            0
#define ERROR_SKILLRESET_STATE              1
#define ERROR_SKILLRESET_ITEM               2
#define ERROR_SKILLRESET_DBERROR            3
#define ERROR_SKILLRESET_ITEM_LICENSE       4           // 미인증 아이템 사용

/// 스킬
struct sInitSkill
{
	unsigned long	mSkillIdx;				/// 스킬 인덱스 
	unsigned char	mSkillStep;				/// 스킬 단계
};

struct MSG_RES_PLAYER_SKILLRESET : public MSG_ERROR
{
	unsigned short	mSkillPoint;		/// 스킬 포인트

	long			mRowCount;			/// 보유스킬 갯수
	sInitSkill		mTable[1];			/// 보유스킬 리스트

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_PLAYER_SKILLRESET) - sizeof(mTable) + (mRowCount * sizeof(sInitSkill)); }
};

/// 스킬
const unsigned char SKILL_ADD_ERR_SUCCESS   = 0;
const unsigned char	SKILL_ADD_ERR_DBERROR   = 1;
const unsigned char SKILL_ADD_ERR_PLAYER    = 2;
const unsigned char SKILL_ADD_ERR_SCRIPT    = 3;
const unsigned char SKILL_ADD_ERR_MAPINSERT = 4;

const unsigned char SKILL_ADD_ERR_MONEY         = 101;			/// 소지금액 부족
const unsigned char SKILL_ADD_ERR_SP            = 102;			/// skill point 부족
const unsigned char SKILL_ADD_ERR_SKILLSTEP     = 103;		    /// 스킬 습득 단계 오류
const unsigned char SKILL_ADD_ERR_LEARNSKILL    = 104;		    /// 선행 보유 스킬 오류
const unsigned char SKILL_ADD_ERR_LEARNSTEP     = 105;		    /// 선행 보유 스킬 단계 오류
const unsigned char SKILL_ADD_ERR_RACE          = 106;			/// 종족 오류
const unsigned char SKILL_ADD_ERR_JOB           = 107;			/// 직업 오류
const unsigned char SKILL_ADD_ERR_LEVEL         = 108;			/// 레벨 오류
const unsigned char SKILL_ADD_ERR_ALREADY       = 109;		    /// 이미 배운 스킬을 다시 배우려함
const unsigned char SKILL_ADD_ERR_NPC           = 110;			/// npc 대화 조건이 안맞음
const unsigned char SKILL_ADD_ERR_JOBSP         = 111;			/// 전직업 사용SP 부족

struct MSG_REQ_SKILL_ADD : public MSGROOT
{
	unsigned long	mSkillClassIdx;
	unsigned char	mSkillStep;
};

struct MSG_RES_SKILL_ADD : public MSGROOT
{
	unsigned long	mSkillClassIdx;
	unsigned char	mSkillStep;
};

/// 인벤토리/착용카드 슬롯/창고 크기 변경
struct MSG_SYN_PLAYER_INVEN_SIZE : public MSGROOT
{
	unsigned short  mBag;
	unsigned short  mWear;
	unsigned short  mWareHouse;
    unsigned short  mMakeSkill;
};


/*     자동사냥 신고(오토노트)
*/

/// 심판 제한시간 3분
const unsigned long TRIAL_ANSWER_TIME = 3 * MINUTE;
#define MAX_TRIAL_ANSWER_COUNT          3                   /// 정답 입력 횟수
#define MAX_TRIAL_CAPTCHA_COUNT         5                   /// CAPTCHA 새로고침 횟수

#define ERROR_PLAYER_TRIAL_OPEN_SUCCESS         0
#define ERROR_PLAYER_TRIAL_OPEN_FAIL            1
#define ERROR_PLAYER_TRIAL_OPEN_MAP             2           /// 신고기능을 사용 할 수 없는 맵
#define ERROR_PLAYER_TRIAL_OPEN_TIME            3           /// 재사용 시간(3분/1시간)이 지나지 않음
#define ERROR_PLAYER_TRIAL_OPEN_LEVEL           4           /// 10레벨 이상 사용 가능
#define ERROR_PLAYER_TRIAL_OPEN_TRIAL           5           /// 신고자 또는 내가 심판중
#define ERROR_PLAYER_TRIAL_OPEN_CAPTCHA         6           /// CAPTCHA 이미지 오류
#define ERROR_PLAYER_TRIAL_OPEN_TRIAL_REJECT    7		    /// 심판자가 문제 푼지 5분이 안지났음.
#define ERROR_PLAYER_TRIAL_OPEN_GM              8			/// 운영자 케릭터는 신고 할 수 없음

// NM_PLAYER_TRIAL_OPEN_REQ
struct MSG_REQ_PLAYER_TRIAL_OPEN : public MSGROOT
{
    unsigned long   characterIdx;           /// 신고할 캐릭터
};

// NM_PLAYER_TRIAL_OPEN_RES
struct MSG_RES_PLAYER_TRIAL_OPEN : public MSG_ERROR
{
    long    remainTime;                     /// 신고 재사용 남은 시간/신고 거부 시간
};

// NM_PLAYER_TRIAL_OPEN_SYN
struct MSG_SYN_PLAYER_TRIAL_OPEN : public MSGROOT
{
    unsigned short  size;
    BYTE            captcha[1];

    unsigned long GetMsgLength( ) { return sizeof(MSG_SYN_PLAYER_TRIAL_OPEN) - sizeof(captcha) + (size * sizeof(BYTE)); }
};


#define ERROR_PLAYER_TRIAL_ANSWER_SUCCESS         0
#define ERROR_PLAYER_TRIAL_ANSWER_FAIL            1
#define ERROR_PLAYER_TRIAL_ANSWER_WRONG           2         /// 정답 틀림
#define ERROR_PLAYER_TRIAL_ANSWER_TIME_OVER       3         /// 심판 시간 초과
#define ERROR_PLAYER_TRIAL_ANSWER_TEXTLEN         4			/// 글자 수 틀림.

// NM_PLAYER_TRIAL_ANSWER_REQ
struct MSG_REQ_PLAYER_TRIAL_ANSWER : public MSGROOT
{
	wchar_t answer[MAX_TRIAL_ANSWER_LEN+1];                 /// 입력된 정답
};

// NM_PLAYER_TRIAL_ANSWER_RES
struct MSG_RES_PLAYER_TRIAL_ANSWER : public MSG_ERROR
{
	unsigned char   trialCount;                             /// 정답 입력 가능 횟수
};


#define ERROR_PLAYER_TRIAL_RESULT_AUTO         0            /// 오토 판정
#define ERROR_PLAYER_TRIAL_RESULT_NOT_AUTO     1            /// 정상 유저 판정

// NM_PLAYER_TRIAL_RESULT_SYN
struct MSG_SYN_PLAYER_TRIAL_RESULT : public MSG_ERROR
{
	wchar_t name[MAX_NAME_BUFFER_SIZE];                     /// 신고한 캐릭터 이름
};


#define ERROR_PLAYER_TRIAL_CAPTCHA_SUCCESS          0
#define ERROR_PLAYER_TRIAL_CAPTCHA_FAIL             1
#define ERROR_PLAYER_TRIAL_CAPTCHA_COUNT            2       /// 새로고침 횟수 초과
#define ERROR_PLAYER_TRIAL_CAPTCHA_TRIAL            3       /// 심판중이 아님

// NM_PLAYER_TRIAL_CAPTCHA_RES
struct MSG_RES_PLAYER_TRIAL_CAPTCHA : public MSG_ERROR
{
    unsigned char   count;                                  /// 새로고침 횟수
    unsigned short  size;
    BYTE            captcha[1];

    unsigned long GetMsgLength( ) { return sizeof(MSG_RES_PLAYER_TRIAL_CAPTCHA) - sizeof(captcha) + (size * sizeof(BYTE)); }
};


struct MSG_RES_RECOVER_EXP : public MSG_RES_EXP
{
};

/* ------- 살펴보기 
*/
#define ERROR_PLAYER_LOOK_OTHER_EQUIP_SUCCESS       0
#define ERROR_PLAYER_LOOK_OTHER_EQUIP_FAIL          1

// NM_PLAYER_LOOK_OTHER_EQUIP_REQ
struct MSG_REQ_PLAYER_LOOK_OTHER_EQUIP : public MSGROOT
{
    unsigned long characterIdx;
};

// NM_PLAYER_LOOK_OTHER_EQUIP_RES
struct MSG_RES_PLAYER_LOOK_OTHER_EQUIP : public MSG_ERROR
{
	wchar_t         name[MAX_NAME_BUFFER_SIZE];     // 캐릭터 이름
	unsigned char	level;							// 레벨
    unsigned short	job;                            // 직업
    unsigned short  wearEnd;                        // 장착 카드
    char            activeWeapon;                   // 활성 무기

	unsigned long RowCount;
	sInventory    Inventory[1];

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_PLAYER_LOOK_OTHER_EQUIP) - sizeof(Inventory) + (RowCount * sizeof(sInventory)); }
};

struct MSG_SYN_PLAYEREXT_AVOID_MON_LOOK : public MSG_ERROR
{
	bool mIsAvoidMonLookOn;		/// 몬스터 선공 인식 회피 스킬 On/Off
};

// NM_PLAYEREXT_DOORPUSH_SYN
struct MSG_SYN_PLAYEREXT_DOORPUSH : public MSGROOT
{
	sObject			mTarget;
	float			mPosX;
	float			mPosY;
};

/* ------- 캐릭터 옵션 변경
*/
struct MSG_SYN_PLAYEREXT_OPTION_SHOW : public MSGROOT
{
	unsigned long	mCharacterIdx;
	bool			mShowHelmet;
	bool			mShowCostume;
};

#pragma pack(pop)
