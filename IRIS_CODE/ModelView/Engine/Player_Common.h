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
//#include "CommonNetworkMsgStruct.h"

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

/// head, wear type은 구조체 관리를 위한 형태 
enum eHEAD_TYPE
{
	eHEAD_HAIR = 0,
	eHEAD_FACE,
	eHEAD_MAX,
};
enum eWEAR_TYPE
{
	eWEAR_BODY1 = 0,
	eWEAR_BODY2,
	eWEAR_HAND,
	eWEAR_FOOT,

	eWEAR_HAT,

	eWEAR_MAX,
};
///

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

	ANITYPE_SKILL_BEGIN = 130,
	ANITYPE_USESKILL1	= ANITYPE_SKILL_BEGIN,
	ANITYPE_USESKILL2	= 140,

	ANITYPE_SWORD_SKILL = 201,
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

	ANITYPE_EMOTE_START = 900,
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
enum ePLAYER_JOB 
{ 
	/// 직업구분이없다.
	ePLAYER_NONEJOB = 0,

	/// 파이터 군
	ePLAYER_FIGHTER=1000,	/// 파이터=1000
	ePLAYER_CRUSADER=1101,	/// 크루세이더=1101
	ePLAYER_KNIGHT=1102,	/// 나이트=1102
	ePLAYER_TEMPLAR=1201,	/// 템플러=1201
	ePLAYER_LOAD=1202,		/// 로드=1202

	/// 메이지 군
	ePLAYER_MAGE=2000,		/// 메이지=2000
	ePLAYER_MAGICIAN=2101,	/// 매지션=2101
	ePLAYER_PRIEST=2102,	/// 프리스트=2102	
	ePLAYER_WIZARD=2201,	/// 위자드=2201	
	ePLAYER_BISHOP=2202,	/// 비숍=2202

	/// 레인져 군
	ePLAYER_RANGER=3000,	/// 레인져=3000
	ePLAYER_HUNTER=3101,	/// 헌터=3101
	ePLAYER_SHOOTER=3102,	/// 슈터=3102
	ePLAYER_SCOUT=3201,		/// 스카우트=3201
	ePLAYER_SNIPER=3202,	/// 스나이퍼=3202

	/// 샤먼 군
	ePLAYER_SHAMAN=4000,	/// 샤먼=4000
	ePLAYER_WARLOCK=4101,	/// 워록=4101
	ePLAYER_SEER=4102,		/// 씨어=4102	
	ePLAYER_SORCERER=4201,	/// 소서러=4201	
	ePLAYER_CONJURER=4202,	/// 컨저러=4202

	/// 워리어 군
	ePLAYER_WARRIOR=5000,	/// 워리어=5000
	ePLAYER_VETERAN=5101,	/// 베테랑=5101
	ePLAYER_GLADIATOR=5102,	/// 글라디에이터=5102
	ePLAYER_CHAMPION=5201,	/// 챔피온=5201
	ePLAYER_BERSERK=5202,	/// 버서커=5202

	/// 씨프 군
	ePLAYER_THIEF=6000,		/// 시프=6000
	ePLAYER_ROGUE=6101,		/// 로그=6101
	ePLAYER_ROBBER=6102,	/// 라버=6102		
	ePLAYER_ASSASSIN=6201,	/// 어쌔신=6201	
	ePLAYER_AVENGER=6202,	/// 어밴져=6202

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

	ePLAYER_STATUS_EXT_ATKMin,		/// 물리근접 공격력 최소
	ePLAYER_STATUS_EXT_ATKMax,		/// 물리근접 공격력 최대
	ePLAYER_STATUS_EXT_RATKMin,		/// 물리원거리 공격력 최소
	ePLAYER_STATUS_EXT_RATKMax,		/// 물리원거리 공격력 최대
	ePLAYER_STATUS_EXT_MATKMin,		/// 마법 공격력 최소
	ePLAYER_STATUS_EXT_MATKMax,		/// 마법 공격력 최소
	ePLAYER_STATUS_EXT_HEALMin,		/// 회복량 최소
	ePLAYER_STATUS_EXT_HEALMax,		/// 회복량 최대

	ePLAYER_STATUS_EXT2_MAX,		/// 최소 최대를 포함한 최대값

	ePLAYER_STATUS_EXT_MAX = ePLAYER_STATUS_EXT_ATKMin,	/// 최소 최대를 포함 하지 않은 최대값

};

enum ePLAYER_STATUS_EXT_ADD
{
	ePLAYER_STATUS_EXT_ADD_EXPADDPER,
	ePLAYER_STATUS_EXT_ADD_EXPADDPLUS,
	ePLAYER_STATUS_EXT_ADD_SXPADDPER,
	ePLAYER_STATUS_EXT_ADD_SXPADDPLUS,

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
	eSTOP_BUSYTAROT,		/// 개설자 타롯점 치는중
	eSTOP_ENTERTAROT,       /// 관람자 타롯
	eSTOP_USETAROT,			/// 참여자 타롯 사용중
	eSTOP_OPENSTORE,
	eSTOP_USESTORE,

	eSTOP_NPCSPEECH,
};

/// 구조체...
struct sPlayerInfo
{
	unsigned long	CharacterIdx;					/// 캐릭터 인덱스
	wchar_t			strName[MAX_NAME_BUFFER_SIZE];	/// 캐릭터 이름
	char			Level;							/// 현재 레벨
	char			Race;							/// 종족 - enum /*Race*/
	char			Gender;							/// 성별 - enum /*Gender*/
	ePLAYER_JOB		Job;							/// 직업

	unsigned long	HeadInfo[eHEAD_MAX];
	unsigned short	HairColor;

	sPlayerInfo() 
	{
		::memset(this, 0, sizeof(sPlayerInfo));
	}
};

struct sPlayerWearInfo
{
	/// 그래픽 정보를 포함한다.(HAT은 예외로 한다.)
	unsigned long	WearIdx[eWEAR_MAX];

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

	sPlayerWeaponInfo()
	{
		WeaponIdx[eHAND_LEFT] = WeaponIdx[eHAND_RIGHT] = 0;//0xffffffff;
	}
};


struct sPlayerExrInfo
{
	char			MaxLevel;				// 캐릭터가 도달한 최고 경험치
	unsigned long	Exp;					// 현재 경험치

	/// --스킬과 관련된 포인트--
	unsigned char	SkillLevel;				/// 스킬 경험치로 레벨업을 함 - 플레이어레벨,스킬레벨
	/// 각각 1업을 할때마다 스킬포인트가 1씩 올라감
	unsigned int	SkillExp;				/// 현재 스킬경험치
	unsigned short	SkillPointUse;			/// 사용포인트 -> 스킬의 레벨업을 할때 기준이된다.
	unsigned short	SkillPointRemain;		/// 잔여스킬경험치

	unsigned short	GradeUpPoint;			/// 추가 스텟포인트 확장..

	unsigned long	RestHP;					/// 남은 생명력
	unsigned long	RestMP;					/// 남은 마나

	unsigned long	MaxHP;
	unsigned long	MaxMP;

	unsigned int	Fame;					/// 명성
	char			Rank;					/// 지위

	/// 071109 PKH 평화 대기상태 - 서버에서 처리는 하지않고 클라이언트에 알려주기만함
	eIDLEFLAG		mStateIdle;
	/// 070601 PKH 플레이어 상태 
	//	ePLAYERSTATE	mState;
	unsigned int	mState;					/// eOBJECTSTATE
	/// 071109 PKH 정지 상태일경우 세부사항
	eSTOPFLAG		mStateStop;

	unsigned long	mCommunitySkillIdx;

	unsigned long	ServerTimer;			/// 서버 동기화 시간

	sPlayerExrInfo()
	{
		memset(this, 0, sizeof(sPlayerExrInfo));
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
	float			mMaxHP;				/// 증가:최대HP 
	float			mMaxMP;				/// 증가:최대MP 
	float			mRecovHP;			/// 증가:회복HP%
	float			mRecovMP;			/// 증가:회복MP%
	float			mPhysicAttackRate;	/// 물리:공격률
	float			mMagicAttackRate;	/// 마법:공격률
	float			mAvoid;				/// 증가:회피
	float			mMaxWeight;			/// 무게
	float			mAttackSpeed;		/// 일반공격속도
	float			mCooltime;			/// cooltime
	float			mInRange;			/// 근거리사정거리
	float			mOutRange;			/// 원거리(+마법)사정거리
	float			mMoveSpeed;			/// 이동속도
	float			mMinHeal;			/// 최소회복량
	float			mMaxHeal;			/// 최대회복량

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
};

struct sDamageCalc
{
	/// 공격 성공 확률											
	unsigned long	mInstantPerHit;			/// %순간 효과			
	unsigned long	mInstantPlusHit;		/// 순간 효과				
	/// 크리티컬 발생 확률(물리)								
	unsigned long	mPhyInstantPerCri;		/// %순간 효과				
	unsigned long	mPhyInstantPlusCri;		/// 순간 효과				
	/// 크리티컬 발생 확률(마법)									
	unsigned long	mMagicInstantPerCri;	/// %순간 효과					
	unsigned long	mMagicInstantPlusCri;	/// 순간 효과					
	/// 공격력(물리)									
	unsigned long	mPhyInstantPerAttack;	/// %순간 효과					
	unsigned long	mPhyInstantPlusAttack;	/// 순간 효과					
	/// 공격력(마법)									
	unsigned long	mMagicInstantPerAttack;	/// %순간 효과					
	unsigned long	mMagicInstantPlusAttack;	/// 순간 효과						
	/// 최종 데미지									
	unsigned long	mInstantPerFinal;		/// %순간 효과				
	unsigned long	mInstantPlusFinal;		/// 순간 효과	
};

/// 070418 이동 "CommonNetworkMsgStruct.h(219)"
/// struct MSG_PLAYERINFO : public MSGROOT

/*
/// 캐릭터 선택창에 보여질 정보 구조체
struct MSG_CHARACTER_BASEINFO
{
	sPlayerInfo			PlayerInfo;					/// 캐릭터 정보
	sPlayerWeaponInfo	WeaponInfo;					/// 무기   정보
	sPlayerWearInfo     WearInfo;					/// 방어구 정보

	char				StandIdx;					/// 캐릭터 순서
	unsigned short		MapNum;						/// 맵     번호
};

/// 캐릭터 선택창으로 이동시 보내는 network msg 구조체
struct MSG_CHARACTER_LIST : public MSG_ERROR
{
	unsigned short			CharacterNum;	/// 만들어진 캐릭터 수
	MSG_CHARACTER_BASEINFO	BaseInfo[5];	/// 캐릭터 정보	

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength()	{ return sizeof(MSG_CHARACTER_LIST) - (5-CharacterNum)*sizeof(MSG_CHARACTER_BASEINFO); }
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

	float				RotAngle;		/// 
};


//struct MSG_MOVE_INFO : public MSG_ERROR
//{
//	unsigned long characterIdx;		/// characteridx는 NM_PLAYER_MOVE_SYN에서만 쓰시오
//	int		nStartX;
//	int		nStartZ;
//	int		nEndX;
//	int		nEndZ;
//
//	/// 070216 PKH 서버와 클라이언트 동기화된 시계
//	unsigned long	mClientTimer;	/// characteridx는 NM_PLAYER_MOVE_RES에서만 쓰시오
//
//	void	SetStartPos( float fx, float fz )	{ nStartX = (int)fx; nStartZ = (int)fz; }
//	void	SetEndPos( float fx, float fz )		{ nEndX = (int)fx; nEndZ = (int)fz; }
//};


struct MSG_REQ_MOVE : public MSGROOT
{
	float	startX;
	float	startY;
	float	destX;
	float	destY;
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
};

struct MSG_SYN_MOVE : public MSGROOT
{
	unsigned long	characterIdx;
	float			destX;
	float			destY;
    long			moveSpeed;	
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


struct MSG_REQ_SELECTNPC : public MSGROOT
{
	unsigned long	mNpcIdx;	
};

struct MSG_RES_SELECTNPC : public MSG_ERROR
{
	unsigned long	mNpcIdx;	
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
#define ERROR_CHARACTER_CREATE_FAIL			7	/// 실패.

struct MSG_REQ_CHARACTER_CREATE : public MSGROOT
{
	wchar_t			Name[MAX_NAME_BUFFER_SIZE];
	char			Race;
	char			Gender;

	unsigned long	Job;
	unsigned long	Hair;
	unsigned long	Face;
	unsigned short	HairColor;

	unsigned char   StandIdx;
};
struct MSG_RES_CHARACTER_CREATE : public MSG_ERROR
{};

struct MSG_REQ_CHARACTER_DELETE : public MSGROOT
{
	unsigned long		CharacterIdx;	/// 캐릭터 인덱스
};

struct MSG_RES_CHARACTER_DELETE : public MSG_ERROR
{};

struct MSG_REQ_CHARACTER_SELECT : public MSGROOT
{
	unsigned char		StandIdx;
};

struct MSG_REQ_GAMEIN : public MSGROOT
{};

struct MSG_RES_GAMEIN : public MSG_ERROR
{
	unsigned long		ServerTimer;	/// 서버 동기화 시간
};

/// 070625 PKH 플레이어 레벨
struct MSG_RES_LEVEL : public MSGROOT
{
	unsigned char		mLevel;
};

struct MSG_SYN_LEVEL : public MSGROOT
{
	unsigned long		mCharacterIdx;
	unsigned char		mLevel;
};

/// 070625 PKH 플레이어 스킬 레벨
struct MSG_RES_SKILLLEVEL : public MSGROOT
{
	unsigned char		mSkillLevel;
	unsigned short		mSkillPoint;
};

/// 경험치
struct MSG_RES_EXP_SXP : public MSGROOT
{
	unsigned long	mExp;			/// 얻은 경험치
	unsigned long	mSxp;			/// 얻은 스킬 경험치
	unsigned long	mTotalExp;		/// 플레이어의 경험치 전체량
	unsigned long	mTotalSxp;		/// 플레이어의 스킬 경험치 전체량
};


struct MSG_CHARACTERIDX : public MSGROOT
{
	unsigned long		mCharacterIdx;
};


struct MSG_REQ_PLAYER_MAPCHANGE : public MSGROOT
{
	unsigned long	mMapChangePosIndex;
};


struct MSG_RES_PLAYER_EQUIP_WEAR : public MSGROOT
{
	eWEAR_TYPE			mType;
	unsigned long		mWearInfo;
};
struct MSG_SYN_PLAYER_EQUIP_WEAR : public MSGROOT
{
	unsigned long		mCharacterIdx;
	eWEAR_TYPE			mType;
	unsigned long		mWearInfo;
};

struct MSG_RES_PLAYER_EQUIP_WEAPON : public MSGROOT
{
	eWEAPON_STATE       mWeaponState;
	sPlayerWeaponInfo	mWeaponInfo;
};
struct MSG_SYN_PLAYER_EQUIP_WEAPON : public MSGROOT
{
	unsigned long		mCharacterIdx;
	eWEAPON_STATE       mWeaponState;
	sPlayerWeaponInfo	mWeaponInfo;
};



#define MAX_SHORTCUT_SIZE			30	// 10 x 3(Tab)

enum eSHORTCUT_TYPE
{
	eSHORTCUT_NONE = 0,					/// 없음
	eSHORTCUT_ITEM,						/// 아이템
	eSHORTCUT_SKILL,					/// 스킬
	eSHORTCUT_COMMU,					/// community
	eSHORTCUT_UNKNOWN,					/// 오류
};

struct sShortcut
{
	char          mType;				/// 종류(0 아이템, 1 스킬, ... )
	unsigned long mIndex;				/// 인덱스
	unsigned long mInventoryIdx;		/// 인벤토리 고유번호
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
	unsigned long	mMP;
};

struct MSG_SYN_PLAYER_RESURRECTION : MSG_ERROR
{
	unsigned long	mCharacterIdx;
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
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
	eOBJECTTYPE   mObjectType;
	long          mAddMoney;
};

struct MSG_RES_PLAYER_DEPOSIT : public MSGROOT
{
	unsigned long mDeposit;
	eOBJECTTYPE   mObjectType;
	long          mAddDeposit;
};

struct MSG_RES_PLAYER_SP : public MSGROOT
{
	unsigned short	mSP;
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
*/
#pragma pack(pop)
