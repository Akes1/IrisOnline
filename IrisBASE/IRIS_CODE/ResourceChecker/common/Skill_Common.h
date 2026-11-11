/* ====================================================================
* 파    일 :	
* 목    적 :	
* 작 성 자 :	정원주
* 작 성 일 :	2006.09.28
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.09.28	정원주	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

#include "BaseObject_Common.h"
#include "Player_Common.h"
#include "float.h"

#include "sqltypes.h"

#pragma pack(push,1)


const unsigned short	PERCENT_HUNDRED = 100;
const unsigned long		NORMAL_ATTACK_SKILL = 1;
const unsigned long		NORMAL_ATTACK_SKILL_MAX = 99;
const unsigned short	SECOND_THOUSAND = 1000;
const unsigned char		SKILL_START_STEP = 0;
const unsigned char		TARGET_CNT_MAX = 50;

const unsigned long		SKILL_PUSHPULL_SPEED = 2000;

// 최대 전직 차수
const unsigned short	MAX_JOBSTEP = 9;


//enum eDAMAGE_KIND 
//{
//	/// 정의안함
//	eDAMAGE_NONE = 0,
//	/// 일반공격
//	eDAMAGE_NORMAL,
//	/// 크리티컬
//	eDAMAGE_CRITICAL,
//	/// 미스-회피
//	eDAMAGE_MISS,
//};


enum eDAMAGE_KIND 
{
	/// 정의안함
	eDAMAGE_NONE = 0,
	/// 일반공격
	eDAMAGE_NORMAL,
	/// 크리티컬
	eDAMAGE_CRITICAL,
	/// 일반공격
	eDAMAGE_NORMAL_MP,
	/// 크리티컬
	eDAMAGE_CRITICAL_MP,
	/// 미스-회피
	eDAMAGE_MISS,
};


/// 스킬 분류
enum eSKILLTYPE
{
	eSKILLTYPE_PASSIVE = 1,				/// 패시브
	eSKILLTYPE_DAMAGE,					/// 액티브-데미지
	eSKILLTYPE_NONACTIVE,				/// 액티브-비공격형(효과생성)
	eSKILLTYPE_HEAL,					/// 액티브-힐
	eSKILLTYPE_MPDAMAGE,				/// 액티브-mp데미지
	eSKILLTYPE_TOTEM,					/// 토템생성
	eSKILLTYPE_PORTAL,					/// 포탈 생성
	eSKILLTYPE_NONSTOP,					/// 정지없이 효과만 생성
};

/// 몬스터 스킬 분류
enum eMONSTERSKILLTYPE
{
	eMONSTERSKILLTYPE_DAMAGE = 1,		/// 액티브-데미지
	eMONSTERSKILLTYPE_NONACTIVE,		/// 액티브-비공격형(효과생성)
	eMONSTERSKILLTYPE_HEAL,				/// 액티브-힐
	eMONSTERSKILLTYPE_MPDAMAGE,			/// 액티브-mp데미지
	eMONSTERSKILLTYPE_TOTEM,			/// 토템생성
	eMONSTERSKILLTYPE_PORTAL,			/// 포탈생성
	eMONSTERSKILLTYPE_NONSTOP,			/// 정지없이 효과만 생성
};

enum eATTRIBUTETYPE
{
	eATTRIBUTETYPE_NONE	= 0,
	eATTRIBUTETYPE_PHYSICAL,			/// 물리
	eATTRIBUTETYPE_MAGIC,				/// 마법
};

enum eSHOTTYPE
{
	eSHOTTYPE_NONE = 0,					/// 없음
	eSHOTTYPE_SPOT,						///	즉시시전
	eSHOTTYPE_CASTING,					/// 캐스팅
	eSHOTTYPE_CHANNEL,					///	채널링
	eSHOTTYPE_AURA,						/// 오라 
	eSHOTTYPE_BLINK,					/// 순간이동
};

enum eMONSTERSHOTTYPE
{
	eMONSTERSHOTTYPE_SPOT = 1,			///	즉시시전
	eMONSTERSHOTTYPE_CASTING,			/// 캐스팅
};

/// 스킬 발동시 범위 방식
enum eBOUNDTYPE
{
	eBOUNDTYPE_NONE = 0,				/// 범위 없음
	eBOUNDTYPE_SELF_TARGET,				/// 시전자(자신) 기준범위 - 발동시 타겟 존재
	eBOUNDTYPE_SELF_NOTARGET,			/// 시전자(자신) 기준범위 - 발동시 타겟 없어도 됨
	eBOUNDTYPE_TARGET,					/// 타겟 기준범위
	eBOUNDTYPE_FIELD,					/// 필드 기준범위
	eBOUNDTYPE_AUTOFIELD,				/// 전방 필드 범위
};


/// 효과를 적용받는 대상
enum eAPPLYTYPE
{
	eAPPLYTYPE_SELF = 1,				/// 시전자(자신)
	eAPPLYTYPE_ENEMY,					/// 적
	eAPPLYTYPE_BUDDY,					/// 동료
	/// 이하 몬스터 사용 안함
	eAPPLYTYPE_PARTY,					/// 파티
	eAPPLYTYPE_DIEBUDDY,				/// 죽은동료
};

enum eRANGETYPE
{
	eRANGETYPE_CLOSE = 1,				/// 근거리
	eRANGETYPE_LONG,					/// 원거리
};

enum eDOT_TYPE
{
	eDOTTYPE_NONE = 0,					/// 없음
	eDOTTYPE_PHYSICALCLOSE,				/// 물리 근거리
	eDOTTYPE_PHYSICALRANGE,				/// 물리 원거리
	eDOTTYPE_MAGIC,						/// 마법
};

enum eEQUIPTYPE
{
	eEQUIPTYPE_NONE = 0,
	eEQUIPTYPE_SWORD,
	eEQUIPTYPE_LONGSWORD,
	eEQUIPTYPE_DOUBLESWORD,
	eEQUIPTYPE_SHORTSWORD,
	eEQUIPTYPE_GUN,
	eEQUIPTYPE_STAFF,

	eEQUIPTYPE_SHIELD,
	eEQUIPTYPE_SWORD_LONGSWORD,
	eEQUIPTYPE_SWORD_DOUBLESWORD,
	eEQUIPTYPE_LONGSWORD_DOUBLESWORD,
	eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD,
	eEQUIPTYPE_SHORTSWORD_GUN,

	eEQUIPTYPE_CLOTH,
	eEQUIPTYPE_LIGHTARMOR,
	eEQUIPTYPE_HEAVYARMOR,
};

enum /* eINFLUENCETYPE */
{
	eINFLUENCETYPE_BUF = 1,		/// 버프
	eINFLUENCETYPE_DEBUF,		/// 디버프
	eINFLUENCETYPE_CASH = 11,	/// 캐쉬
	eINFLUENCETYPE_PC,			/// PC방
};

enum /* eINFLUENCETYPEDETAIL */
{
	eINFLUENCETYPEDETAIL_AURA = 1,				/// 오라
	eINFLUENCETYPEDETAIL_BUILD,					/// 강화
	eINFLUENCETYPEDETAIL_MAGIC,					/// 마법

	eINFLUENCETYPEDETAIL_TRAUMA = 101,			/// 외상
	eINFLUENCETYPEDETAIL_ELEMENT,				/// 원소
	eINFLUENCETYPEDETAIL_WEAK,					/// 약화 (손상+중독)
	eINFLUENCETYPEDETAIL_AFTEREFFECT,			/// 부활후유증
	eINFLUENCETYPEDETAIL_LEADER,				/// 수장선택
	eINFLUENCETYPEDETAIL_NOPROPERTY,			/// 무속성

	eINFLUENCETYPEDETAIL_RESURRECTION = 201,	/// 부활
	eINFLUENCETYPEDETAIL_POTION = 301,			/// 물약
	eINFLUENCETYPEDETAIL_GOODS,					/// 소모 잡화 버프

	eINFLUENCETYPEDETAIL_COOKFULL = 311,		/// 요리-포만감
	eINFLUENCETYPEDETAIL_COOKSATISFACTION,		/// 요리-만족감
	eINFLUENCETYPEDETAIL_COOKFLAVOR,			/// 요리-감칠맛
	eINFLUENCETYPEDETAIL_COOKRECOVERY,			/// 요리-회복

	eINFLUENCETYPEDETAIL_ALCHEMYPOTION = 321,	/// 연금술-물약
	eINFLUENCETYPEDETAIL_ALCHEMYPROMOTION,		/// 연금술-촉진제
	eINFLUENCETYPEDETAIL_ALCHEMYENHANCE,		/// 연금술-강화제
	eINFLUENCETYPEDETAIL_ALCHEMYDRUG,			/// 연금술-영약

	eINFLUENCETYPEDETAIL_CARD = 401,			/// 카드 버프

	eINFLUENCETYPEDETAIL_NAME = 501,			/// 호칭 버프
	EINFLUENCETYPEDETAIL_VEHICLE = 601,			/// 탑승
};

enum /* eSKILLRACE */
{
	eSKILLRACE_ALL = 0,
	eSKILLRACE_HUMAN,
	eSKILLRACE_BEAST,
	eSKILLRACE_ELF,
	eSKILLRACE_HUMAN_BEAST,
	eSKILLRACE_HUMAN_ELF,
	eSKILLRACE_BEAST_ELF,
};

enum eSWITCH_TYPE
{
	eSWITCHTYPE_NORMAL,			/// 일반효과
	eSWITCHTYPE_PARENT,			/// 스위치 타입의 부모 효과
	eSWITCHTYPE_CHILD,			/// 스위치 타입의 자식 효과
	eSWITCHTYPE_TOTEM,			/// 토템에서 생성된 스위치 타입의 자식 효과
};


enum eMONSTERATTACK_TYPE
{
	eMONSTERATTACK_NORMAL1 = 0,
	eMONSTERATTACK_NORMAL2,

	eMONSTERATTACK_SKILL1,
	eMONSTERATTACK_SKILL2,
	eMONSTERATTACK_SKILL3,

	eMONSTERATTACK_MAX,

	/// 모드 스킬
	eMONSTERATTACK_MODESKILL_START = 11,
	eMONSTERATTACK_MODESKILL_1 = eMONSTERATTACK_MODESKILL_START,
	eMONSTERATTACK_MODESKILL_2,
	eMONSTERATTACK_MODESKILL_3,
	eMONSTERATTACK_MODESKILL_4,
	eMONSTERATTACK_MODESKILL_5,
	eMONSTERATTACK_MODESKILL_6,
	eMONSTERATTACK_MODESKILL_7,

	eMONSTERATTACK_MODESKILL_END,
};

enum eMONSTERHPCHECK
{
	eMONSTERHPCHECK_NONE = 0,	/// 조건 없음
	eMONSTERHPCHECK_LOWHP40,	///	hp40%이하
	eMONSTERHPCHECK_LOWHP80,	/// hp80%이하
};

enum eACTSTATUS
{
	eACTSTATUS_ACTIVESKILL = 1,		/// active target skill
	eACTSTATUS_HEALSKILL,		/// heal skill
	eACTSTATUS_DAMAGE,			/// damage
};

enum eINFGUARD_TYPE
{
	eINFGUARDTYPE_NONE = 0,
	eINFGUARDTYPE_PHY,
	eINFGUARDTYPE_MAG,
	eINFGUARDTYPE_ALL,
};

enum eINFCHANGEMP_TYPE
{
	eINFCHANGEMPTYPE_NONE = 0,
	eINFCHANGEMPTYPE_PHY,
	eINFCHANGEMPTYPE_MAG,
	eINFCHANGEMPTYPE_ALL,
};

enum eINF_TIMETYPE
{
	eINFTIMETYPE_GAME = 0,		/// 게임 플레이 시간
	eINFTIMETYPE_REAL,			/// 현실 시간
};

enum eSTATUS_PLUS;

/// skill의 단계별 정보
struct sPlayerSkillStepInfo
{
	/// 툴팁Index
	unsigned long	mToolTipIdx;
	/// SP필요값	
	unsigned short	mRequireSP;
	/// 직업 Index
	unsigned long	mJobType;
	/// 캐릭터 필요Lv	
	unsigned char	mPlayerLevel;
	/// 스킬습득조건 - 필요스킬idx 1
	unsigned long	mLearnSkillIdx1;
	/// 스킬습득조건 - 필요단계 1
	unsigned char	mLearnSkillStep1;
	/// 스킬습득조건 - 필요스킬idx 2
	unsigned long	mLearnSkillIdx2;
	/// 스킬습득조건 - 필요단계 2
	unsigned char	mLearnSkillStep2;
	/// 소모금액
	unsigned long	mUseMoney;
	/// 상태수치값	
	unsigned short	mUseStateValue;
	/// 소모MP값
	unsigned long	mUseMP;
	/// 소모HP값
	unsigned long	mUseHP;
	/// 소모아이템개수	
	unsigned char	mUseItemCount;
	/// 사거리
	float			mTargetDist;
	/// 캐스팅시간
	unsigned long	mCastingTime;
	/// 쿨타임	
	unsigned long	mCoolTime;
	/// 범위거리
	unsigned short	mBoundDist1;
	/// 범위거리
	unsigned short	mBoundDist2;
	/// 적용인원
	unsigned char	mApplyCnt;
	/// 고유명중률	
	short			mAccuracyValue;
	/// 고유크리티컬	
	unsigned short	mCriticalValue;
	/// 고유어그로
	int				mDistressValue;
	/// 적용값 type1
	unsigned short	mApplyValueType1;
	/// 적용값1	
	int				mApplyValue1;
	/// 적용값 type2
	unsigned short	mApplyValueType2;
	/// 적용값2	
	int				mApplyValue2;

	/// 효과 Index
	unsigned long	mInfulenceIdx;

};

/// skill의 기본 정보
struct sPlayerSkillBaseInfo
{
	/// 스킬 종류Idx
	unsigned long	mSkillIdx;
	/// 스킬 이름Idx
	unsigned long	mNameIdx;
	/// 스킬 아이콘Idx
	unsigned long	mIconIdx;

	/// 스킬습득조건 - 종족
	unsigned char	mRace;
	/// 직업 단계
	unsigned char	mJobStep;
	/// 스킬습득조건 - 이전직업 요구 SP량
	unsigned long	mBeforeReqSP;
	/// 스킬사용조건 - 필요장비	eEQUIPTYPE
	unsigned char	mUseEquipment;
	/// 스킬사용조건 - 캐릭터상태
	unsigned char	mUseState;
	/// 스킬사용조건 - 소모아이템
	unsigned long	mUseItem;

	/// 스킬속성 - 분류
	eSKILLTYPE		mType;
	/// 스킬속성 - 속성유형
	eATTRIBUTETYPE	mAttributeType;
	/// 스킬속성 - 시전유형
	eSHOTTYPE		mShotType;
	/// 스킬속성 - 범위방식
	eBOUNDTYPE		mBoundType;
	/// 스킬속성 - 효과적용타겟
	eAPPLYTYPE		mApplyType;
	/// 스킬속성 - 거리유형
	eRANGETYPE		mRangeType;

	/// 스킬속성 - 글로벌쿨타임 적용 여부
	bool			mIsGlobalCoolTime;

	/// 스킬속성 - 그룹쿨타임 번호
	unsigned int	mGroupCoolTimeIdx;

	/// 스킬의 단계별정보를 정의
	unsigned char			mStepCount;
	sPlayerSkillStepInfo*	mpSetpInfoArray;

	sPlayerSkillBaseInfo()
	{
		mStepCount = 0;
		mpSetpInfoArray = 0;
	}
	~sPlayerSkillBaseInfo()
	{
		SAFE_DELETE_ARRAY(mpSetpInfoArray);
	}
};


struct sMonsterSkillScript
{
	/// 몬스터Idx
	unsigned long	mMonsterIdx;
	/// 공격방식
	eMONSTERATTACK_TYPE		mAttackType;
	/// Drama file name
	cString					mDramaFile;
	/// 툴팁Idx
	unsigned long	mToolTipIdx;
	/// 스킬 분류
	eMONSTERSKILLTYPE	mType;
	/// 속성유형
	eATTRIBUTETYPE	mAttributeType;
	/// 거리유형
	eRANGETYPE		mRangeType;
	/// 공격력(소)
	unsigned long	mMinAttackValue;
	/// 공격력(대)
	unsigned long	mMaxAttackValue;
	/// 고유명중률
	short			mAccuracyValue;
	/// 고유크리티컬
	unsigned short	mCriticalValue;
	/// 사용조건(HP)
	eMONSTERHPCHECK	mUseHPCheck;
	/// 사용조건(소모MP)
	unsigned long	mUseMP;
	/// 사거리
	float			mTargetDist;
	/// 공격속도 
	unsigned long	mAttackSpd;
	/// 쿨타임
	unsigned long	mCoolTime;
	/// 사용확률
	unsigned char	mSkillUsePer;
	/// 시전유형
	eMONSTERSHOTTYPE	mShotType;
	/// 범위방식
	eBOUNDTYPE		mBoundType;
	/// 범위거리
	unsigned short	mBoundDist1;
	/// 범위거리
	unsigned short	mBoundDist2;
	/// 적용인원
	unsigned char	mApplyCnt;
	/// 효과적용타겟
	eAPPLYTYPE		mApplyType;
	/// 캐스팅시간
	unsigned long	mCastingTime;
	/// 효과번호1
	unsigned short	mApplyValueType1;
	/// 효과값1
	int				mApplyValue1;
	/// 효과번호2
	unsigned short	mApplyValueType2;
	/// 효과값2
	int				mApplyValue2;
	/// 효과Idx
	unsigned long	mInfulenceIdx;
};


struct sTargetDamageDetail
{
	unsigned short	mDamage;
	unsigned char	mDamageKind;			/// eDAMAGE_KIND

	/// 공격자에게 데미지를 반사하는 량
	short			mDamageReflection;
	/// 공격자가 타겟의 데미지를 가져와 HP회복하는 량
	short			mDamageAbsorbHP;
	/// 공격자가 타겟의 데미지를 가져와 MP회복하는 량
	short			mDamageAbsorbMP;
};


struct sTargetDamage
{
	sObject			mTarget;
	bool			mIsOnlyClientObjectDel;	/// 타겟은 이미 죽었고 클라이언트 객체만 지우기 위함
	unsigned char	mDetailCnt;
	bool			mInfluenceMiss;			/// 효과 생성시 실패 유무
	sTargetDamageDetail mDetail[1];

	unsigned long GetLength() { return sizeof(sTargetDamage) - sizeof(mDetail) + (mDetailCnt * sizeof(sTargetDamageDetail)); }
};

struct sTargetHeal
{
	sObject			mTarget;
	unsigned short	mHeal;
	bool			mInfluenceMiss;			/// 효과 생성시 실패 유무
};

struct sTargetDamageHP
{
	unsigned long	mObjectHP;
	unsigned long	mObjectMaxHP;
	unsigned char	mDamageKind;			/// eDAMAGE_KIND
	unsigned long	mObjectMP;
	unsigned long	mObjectMaxMP;
	bool			mDie;
	sObject			mTarget;
};

struct sTargetApply
{
	sObject			mTarget;
	bool			mInfluenceMiss;			/// 효과 생성시 실패 유무
};


struct sInfluenceScript
{
	/// 효과Idx
	unsigned long	mInfluenceIdx;
	/// 효과 연출 파일명
	cString			mDramaturgyFile;
	/// 이름Idx
	unsigned long	mNameidx;
	/// 효과아이콘Idx
	unsigned long	mIconIdx;
	/// 툴팁효과Idx
	unsigned long	mToolTipIdx;
	/// 속성
	unsigned char	mType;			/// eINFLUENCETYPE
	/// 도트속성
	unsigned char	mDotType;		/// eDOTTYPE
	/// 분류
	unsigned long	mTypeDetail;	/// eINFLUENCETYPEDETAIL
	/// 죽는경우 효과제거
	bool			mDieDelete;
	/// 효력 그룹
	unsigned long	mGroupIdx;
	/// 효력
	unsigned long	mGroupOrder;
	/// 중복불가그룹
	unsigned long	mNotOverlapGroupIdx;
	/// 시간 타입
	unsigned char	mTimeType;		/// eINF_TIMETYPE
	/// 지속(채널링)시간
	long		 	mContinuanceTime;
	/// 발동확률
	unsigned char	mActivityPer;
	/// 지속효과번호1
	unsigned short	mStatusPlusIdx1;
	/// 시간1
	unsigned long	mTime1;
	/// 효과값1
	long			mValue1;
	/// 지속효과번호2
	unsigned short	mStatusPlusIdx2;
	/// 시간2
	unsigned long	mTime2;
	/// 효과값2
	long			mValue2;
	/// 지속효과번호3
	unsigned short	mStatusPlusIdx3;
	/// 효과값3
	long			mValue3;
};

/// 발동
struct sActInfluenceScript
{
	/// 발동 Idx
	unsigned long	mActivityIdx;

	/// 발동 툴팁
	unsigned long	mTooltipIdx;

	/// 연출 파일
	cString			mDramaturgyFile;

	/// 발동 상황
	eACTSTATUS		mActivityState;

	/// 발동 확률
	unsigned long	mActivityPer;

	/// 재사용 대기 시간
	unsigned long	mCooltime;

	/// 적용 대상
	eAPPLYTYPE		mApplyType;

	/// 범위 방식
	eBOUNDTYPE		mBoundType;

	float			mBoundDist;

	/// 형태에 따른 참조 번호
	unsigned long	mInfIdx;
};

struct sSkillInfo
{
	unsigned long skillIdx;
	unsigned char skillStep;
};

/// 스킬
struct sHaveSkill
{
	unsigned long	mSkillIdx;				/// 스킬 인덱스 
	unsigned char	mSkillStep;				/// 스킬 단계
	unsigned long	mRestCoolTime;			/// 쿨타임 남은 시간
};

/// 효과
struct sInfluence
{
	unsigned long		mUniqueIdx;
	unsigned long		mDbIdx;
	unsigned long		mInfluenceClassIdx;
	sObject				mAttacker;
	bool				mIsRealTime;
	unsigned long		mRestTime;
	TIMESTAMP_STRUCT	mStartDate;
	TIMESTAMP_STRUCT	mEndDate;
	unsigned long		mTime1;
	unsigned long		mTime2;
	long				mInfluenceValue1;
	long				mInfluenceValue2;
	unsigned char		mAttriButeType;
	unsigned char		mAttackerLv;
	unsigned long		mAttackerDamageMin;
	unsigned long		mAttackerDamageMax;
	unsigned long		mHeal;
	unsigned char		mLeftGuardCnt;
	unsigned short		mLeftGuardValue;
};

/// 클라이언트 전송용 효과
struct sInfluenceClient
{
	unsigned long	mUniqueIdx;				/// 효과 고유 번호
	unsigned long	mInfluenceClassIdx;		/// 효과 종류 번호
	unsigned long	mAuraSkillClassIdx;		/// 오라 스킬 인덱스
	bool			mDieDelete;				/// 죽은경우 삭제 여부
	unsigned long	mRestTime;				/// 효과 남은 시간

	sInfluenceClient()
	{
		memset( this, 0, sizeof(sInfluenceClient) );
	}
};

/// 클라이언트 전송용 효과시간
struct sInfluenceTimeClient
{
	unsigned long	mUniqueIdx;
	unsigned long	mRestTime;				/// 효과 남은 시간
};



/// 클라이언트에서 서버에 타겟 중심 스킬 요청 
struct MSG_REQ_SKILL_USED : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;				/// 
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	sObject			mTarget;				/// 타겟 정보
	float			mClientPosX;			/// 공격자 위치 좌표
	float			mClientPosY;
};
/// 타겟중심의 스킬(+일반공격) 싱크
struct MSG_SYN_SKILL_USED : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	unsigned long	mSkillIndex;			/// 스킬 고유index
	sObject			mAttacker;				/// 공격자 정보
	float			mAttackerPosX;			/// 공격자 좌표
	float			mAttackerPosY;
	sObject			mTarget;				/// 타겟 정보
	float			mTargetPosX;			/// 타겟 좌표
	float			mTargetPosY;
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mObjectMaxMP;
};

struct MSG_REQ_SKILL_ITEMMAPCHANGE : public MSG_REQ_SKILL_USED
{
	unsigned short	mSlotNumber;
	unsigned long	mUseItemIndex;
};

struct MSG_REQ_SKILL_VEHICLE : public MSG_REQ_SKILL_ITEMMAPCHANGE
{
};

/// 순간이동 메세지
struct MSG_REQ_SKILL_BLINK : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	float			mTargetPosX;			/// 순간이동 요청 목적지
	float			mTargetPosY;
};
struct MSG_RES_SKILL_BLINK : public MSG_ERROR
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned long	mSkillIndex;			/// 스킬 고유index
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mMaxMP;
	float			mTargetPosX;			/// 순간이동 목적지
	float			mTargetPosY;
};
struct MSG_SYN_SKILL_BLINK : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	unsigned long	mSkillIndex;			/// 스킬 고유index
	sObject			mAttacker;				/// 공격자 정보
	float			mTargetPosX;			/// 순간이동 목적지
	float			mTargetPosY;
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mObjectMaxMP;
};

struct MSG_RES_SKILL_NONSTOP : public MSG_ERROR
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mMaxMP;

	unsigned char	mCount;
	sTargetApply	mTarget[1];

	unsigned long GetLength() { return sizeof(MSG_RES_SKILL_NONSTOP) - sizeof(mTarget) + (mCount * sizeof(sTargetApply)); }
};
struct MSG_SYN_SKILL_NONSTOP_TARGET : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	sObject			mAttacker;				/// 공격자 정보
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mMaxMP;

	sTargetApply	mTargetApply;			/// 자신이 받은 효과 성공/실패
};
struct MSG_SYN_SKILL_NONSTOP_AROUND : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	sObject			mAttacker;				/// 공격자 정보
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mMaxMP;
};

/// 몬스터 - 타겟중심의 스킬(+일반공격) 싱크
struct MSG_SYN_MONSTER_SKILL_USED : public MSGROOT
{
	unsigned long			mSkillIndex;			/// 스킬 고유index
	unsigned char			mAttackType;			/// 공격방식 eMONSTERATTACK_TYPE
	sObject					mAttacker;				/// 공격자 정보
	float					mAttackerPosX;			/// 몬스터 좌표
	float					mAttackerPosY;
	float					mAttackSpeedFactor;		/// 공격속도
	sObject					mTarget;				/// 타겟 정보
};


/// 클라이언트에서 서버에 좌표 중심 스킬 요청 
struct MSG_REQ_POS_USED : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	float			mTargetPosX;			/// 타겟(필드) 좌표
	float			mTargetPosY;
	float			mClientPosX;			/// 공격자 위치 좌표
	float			mClientPosY;
};
/// 좌표중심의 스킬 싱크
struct MSG_SYN_POS_USED : public MSGROOT
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned char	mSkillStep;
	unsigned long	mSkillIndex;			/// 스킬 고유index
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	sObject			mAttacker;				/// 공격자 정보
	float			mAttackerPosX;			/// 공격자 좌표
	float			mAttackerPosY;
	float			mTargetPosX;			/// 타겟(필드) 좌표
	float			mTargetPosY;
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mObjectMaxMP;
};


/// 몬스터 - 좌표심의 스킬 싱크
struct MSG_SYN_POS_MONSTER_SKILL_USED : public MSGROOT
{
	unsigned long			mSkillIndex;			/// 스킬 고유index
	unsigned char			mAttackType;			/// 공격방식 eMONSTERATTACK_TYPE
	sObject					mAttacker;				/// 공격자 정보
	float					mAttackerPosX;			/// 몬스터 좌표
	float					mAttackerPosY;
	float					mTargetPosX;			/// 타겟(필드) 좌표
	float					mTargetPosY;
};


const int SKILL_USED_ERR_SUCCESS = 0;
const int SKILL_USED_ERR_SKILLTYPE = 1;			/// 스킬 종류 / 인덱스가 틀림
const int SKILL_USED_ERR_SCRIPT = 2;			/// 스크립트 오류
const int SKILL_USED_ERR_UNIQUEIDX = 3;			/// 스킬 객체 고유번호 오류
const int SKILL_USED_ERR_OBJECTINIT = 4;		/// 스킬 객체 초기화 실패

const int SKILL_USED_ERR_ATTAKCERPOS = 101;		/// 공격자 위치가 안맞음
const int SKILL_USED_ERR_USING = 102;			/// 스킬 사용중
const int SKILL_USED_ERR_TARGETDIST = 103;		/// 스킬 사거리밖에 타겟
const int SKILL_USED_ERR_COOLTIME = 104;		/// 쿨타임
const int SKILL_USED_ERR_ATTACKERSTATE = 105;	/// 공격자 상태 변경 불가
const int SKILL_USED_ERR_USEVALUE = 106;		/// 소모값 부족
const int SKILL_USED_ERR_TARGETOBJECT = 107;	/// 타겟 객체 오류
const int SKILL_USED_ERR_TARGETSELECT = 108;	/// 타겟선정 오류
const int SKILL_USED_ERR_PASSIVE = 109;			/// 패시브 사용
const int SKILL_USED_ERR_DONTHAVE = 110;		/// 보유스킬이 아님
const int SKILL_USED_ERR_EQUIPMENT = 111;		/// 장비 오류
const int SKILL_USED_ERR_CHG_MONSTER = 112;		/// 몬스터 변신중 - 스킬사용 불가
const int SKILL_USED_ERR_MAPCHANGE_STATE = 113;	/// 맵 이동 상태 변경 불가

/// 스킬(+일반공격) 성공/실패
struct MSG_RES_SKILL_USED : public MSG_ERROR
{
	unsigned long	mSkillClassIndex;		/// 스킬 종류index
	unsigned long	mSkillIndex;			/// 스킬 고유index
	unsigned long	mManagedKey;			/// 클라이언트에서 생성된 임시 고유키
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mMaxMP;
};


/// 타겟 중심의 캐스팅 스킬 발동 싱크 
struct MSG_SYN_SKILL_CAST_ACTIVE : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index
	sObject			mAttacker;				/// 공격자 정보
	float			mAttackerPosX;			/// 공격자 좌표
	float			mAttackerPosY;
	unsigned long	mObjectMP;				/// 공격자의 mp수치
	unsigned long	mObjectMaxMp;
};

/// 스킬 캐스팅 실패
#define FAIL_SKILL_CASTING_MP_EMPTY		1	/// 사용조건 부족
#define FAIL_SKILL_CASTING_DONT_TARGET	2	/// 타겟이 없다.
#define FAIL_SKILL_CASTING_DONT_ATTACKER	3	/// 공격자가 죽었다
#define FAIL_SKILL_CASTING_DONT_MAPCHANGE	4	/// 맵이동 실패
#define FAIL_SKILL_CASTING_DONT_VEHICLE		5	/// 탈것 실패
#define FAIL_SKILL_CASTING_DONT_SCRIPT		6	/// 스크립트 에러
#define FAIL_SKILL_CASTING_DONT_OBSTACLE	7	/// 강제 해제

struct MSG_SYN_SKILL_CAST_ACTIVITY_FAIL : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index
	sObject			mAttacker;				/// 공격자 정보
	unsigned char	mFailCode;				/// 스킬 실패 코드
};

struct MSG_REQ_SKILL_CAST_CANCEL : public MSGROOT
{
	sObject			mAttacker;				/// 캐스팅 시전자 정보
};

/// 캐스팅 취소 응답 메세지는 취소 성공시에만 발송된다.
struct MSG_RES_SKILL_CAST_CANCEL : public MSGROOT	
{
	unsigned long	mSkillIndex;			/// 스킬 고유index
};

struct MSG_SYN_SKILL_CAST_CANCEL : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index
};

struct MSG_SYN_SKILL_DAMAGE_HP : public MSGROOT
{
	sObject			mAttacker;
	unsigned char	mCount;
	sTargetDamageHP mTargetDamage[100];

	unsigned long GetMsgLength() 
	{ 
		return sizeof(MSG_SYN_SKILL_DAMAGE_HP) - sizeof(mTargetDamage) + (mCount * sizeof(sTargetDamageHP)); 
	}
};

struct MSG_SYN_SKILL_DAMAGE_ATTACKER : public MSGROOT
{
	unsigned short	mMsgSize;
	unsigned long	mSkillIndex;			/// 스킬 고유index
	unsigned char	mSkillStep;

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mTargetCount;			/// 데미지를 받는 타겟의 갯수
	sTargetDamage	mTargetDamage[1];		/// 데미지 정보

	MSG_SYN_SKILL_DAMAGE_ATTACKER()
	{
		mMsgSize = 0;
	}

	/// 실제보내는 메세지 크기
	unsigned short GetMsgLength() 
	{ 
		return mMsgSize;
	}

	void SetMsgLength( unsigned short msgSize ) 
	{
		mMsgSize = msgSize;
	}
};

struct sDamageTargetHead : public MSGROOT
{
	unsigned short	mMsgSize;
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mTargetCount;			/// 연출 타겟
};
struct MSG_SYN_SKILL_DAMAGE_TARGET : public sDamageTargetHead
{
	sObject			mTarget[1];
	sTargetDamage	mTargetDamage;			/// 데미지 정보

	sObject* GetTarget()
	{
		return mTarget;
	}

	sTargetDamage* GetTargetDamage()
	{
		return (sTargetDamage*)(mTarget + mTargetCount);
	}

	unsigned long GetMsgLength() 
	{ 
		return mMsgSize;
	}

	void SetMsgLength( unsigned short size )
	{
		mMsgSize = size;
	}
};

struct MSG_SYN_SKILL_DAMAGE_AROUND : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mTargetCount;			/// 데미지를 받는 타겟의 갯수
	sObject			mTarget[1];				/// 데미지를 받는 타겟

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_DAMAGE_AROUND) - sizeof(mTarget) + (mTargetCount * sizeof(sObject)); }
};

struct MSG_SYN_SKILL_HEAL_ATTACKER : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mTargetCount;			/// 힐을 받는 타겟의 갯수
	sTargetHeal		mTargetHeal[1];			/// 힐 정보

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_HEAL_ATTACKER) - sizeof(mTargetHeal) + (mTargetCount * sizeof(sTargetHeal)); }
};


struct MSG_SYN_SKILL_HEAL_TARGET : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	sTargetHeal		mTargetHeal;			/// 자신이 받은 힐정보
	unsigned char	mTargetCount;			/// 주변에 힐을 받은유저 목록
	sObject			mTarget[1];

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_HEAL_TARGET) - sizeof(mTarget) + (mTargetCount * sizeof(sObject)); }
};

struct MSG_SYN_SKILL_HEAL_AROUND : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mTargetCount;			/// 주변에 힐을 받은유저 목록
	sObject			mTarget[1];

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_HEAL_AROUND) - sizeof(mTarget) + (mTargetCount * sizeof(sObject)); }
};

struct MSG_SYN_SKILL_APPLY : public MSGROOT
{
	unsigned long	mSkillIndex;			/// 스킬 고유index

	unsigned char	mAttackerType;			/// eOBJECTTYPE

	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	bool			mIsChangeMonster;		/// 몬스터 변신상태 여부

	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mApplyCount;			/// 적용을 받는 타겟의 갯수
	sTargetApply	mApplyObject[1];		/// 적용 오브젝트 정보

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_APPLY) - sizeof(mApplyObject) + (mApplyCount * sizeof(sTargetApply)); }
};

struct MSG_SYN_SKILL_DELTARGET : public MSGROOT
{
	unsigned long	mSkillIndex;
	unsigned long	mSkillClassIndex;
	unsigned char	mSkillStep;
	unsigned char	mAttackerType;			/// eOBJECTTYPE
	bool			mAttackerDie;			/// 공격자가 죽어 객체만 제거

	unsigned char	mDelTargetType;
	unsigned long	mDelTargetIndex;
	unsigned long	mDramaKey;				/// (player: race/gender/weapon) || (monster : class index)

	unsigned char	mApplyCount;
	sTargetApply	mApplyObject[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_DELTARGET) - sizeof(mApplyObject) + (mApplyCount * sizeof(sTargetApply)); }
};


/// 스킬 발동 실패시 좌표 보간
struct MSG_RES_SKILL_POS : public MSGROOT
{
	sObject			mAttacker;				/// 공격자 정보
	float			mAttackerPosX;			/// 공격자 좌표
	float			mAttackerPosY;
	sObject			mTarget;				/// 자겟 정보
	float			mTargetPosX;			/// 타겟 좌표
	float			mTargetPosY;
};


struct MSG_SYN_INFLUENCE_CREATE : public MSGROOT
{
	sObject			mTarget;
	sObject			mAttacker;
	unsigned long	mUniqueIdx;
	unsigned long	mInfluenceClassIdx;
	unsigned long	mAuraSkillClassIdx;
	bool			mDieDelete;
	unsigned long	mPlusTime;

	MSG_SYN_INFLUENCE_CREATE()
	{
		memset( this, 0, sizeof( MSG_SYN_INFLUENCE_CREATE ) );
	}
};

struct MSG_SYN_ACTINFLUNCE_ACTION : public MSGROOT
{
	sObject			mPlayer;
	unsigned long	mActIdx;
	MSG_SYN_ACTINFLUNCE_ACTION()
	{
		memset( this, 0, sizeof( MSG_SYN_ACTINFLUNCE_ACTION ) );
	}
};

struct sInfluenceCreate
{
	sObject			mTarget;
	sObject			mAttacker;
	unsigned long	mUniqueIdx;
	unsigned long	mInfluenceClassIdx;
	unsigned long	mAuraSkillClassIdx;
	bool			mDieDelete;
	unsigned long	mPlusTime;

	sInfluenceCreate()
	{
		memset( this, 0, sizeof( sInfluenceCreate ) );
	}
};

struct MSG_SYN_INFLUENCEARY_CREATE : public MSGROOT
{
	unsigned short	mTargetCnt;
	sInfluenceCreate mTable[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_INFLUENCEARY_CREATE) - sizeof(mTable) + (mTargetCnt * sizeof(sInfluenceCreate)); }
};


struct MSG_SYN_INFLUENCE_UPDATE : public MSGROOT
{
	unsigned long	mUniqueIdx;
	unsigned long	mInfluenceClassIdx;
};



struct MSG_REQ_INFLUENCE_DELETE : public MSGROOT
{
	unsigned long	mUniqueIdx;
};

struct MSG_SYN_INFLUENCE_DELETE : public MSGROOT
{
	unsigned long	mUniqueIdx;	

	MSG_SYN_INFLUENCE_DELETE()
	{
		memset( this, 0, sizeof( MSG_SYN_INFLUENCE_DELETE ) );
	}
};


struct MSG_REQ_INFLUENCE_AURA_DELETE : public MSGROOT
{
	unsigned long	mUniqueIdx;
	unsigned long	mSkillClassIdx;
};


struct MSG_SYN_INFLUENCE_DAMAGE_HP : public MSGROOT
{
	sObject			mTarget;
	unsigned long	mUniqueIdx;
	unsigned long	mTargetHP;
	bool			mTargetDie;
};

struct MSG_SYN_INFLUENCE_DAMAGE_MP : public MSGROOT
{
	sObject			mTarget;
	unsigned long	mUniqueIdx;
	unsigned long	mTargetMP;
};

struct MSG_RES_SKILL_LIST : public MSGROOT
{
	unsigned char	mRowCount;			/// 보유스킬 갯수
	sHaveSkill		mTable[1];			/// 보유스킬 리스트

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_SKILL_LIST) - sizeof(mTable) + (mRowCount * sizeof(sHaveSkill)); }
};

struct MSG_RES_INFLUENCE_LIST : public MSGROOT
{
	unsigned char	mRowCount;			/// 보유효과 갯수
	sInfluenceClient	mTable[1];		/// 보유효과 리스트

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_INFLUENCE_LIST) - sizeof(mTable) + (mRowCount * sizeof(sInfluenceClient)); }
};

struct sInfGuardList
{
	sInfluenceClient	mInfInfo;				/// 효과 정보
	unsigned char		mGuardCnt;				/// 보호막 횟수
	unsigned short		mGuardValue;			/// 보호막 수치
};
struct MSG_SYN_SKILL_INFLUENCE_GUARDLIST : public MSGROOT
{
	unsigned char	mRowCount;
	sInfGuardList	mGuardList[1];

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_SKILL_INFLUENCE_GUARDLIST) - sizeof(mGuardList) + (mRowCount * sizeof(sInfGuardList)); }
};

struct MSG_SYN_INFLUENCE_ADDLIST : public MSGROOT
{
	sObject			mObject;
	unsigned char	mRowCount;			/// 보유효과 갯수
	sInfluenceClient	mTable[1];		/// 보유효과 리스트

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_INFLUENCE_ADDLIST) - sizeof(mTable) + (mRowCount * sizeof(sInfluenceClient)); }
};

struct MSG_SYN_SKILL_INFLUENCE_HPCHANGE : public MSGROOT
{
	unsigned long	mInfluenceIdx;	/// 효과 고유번호
	sObject			mTarget;		/// 타겟
	short			mChangeValue;	/// 변동값
};

struct MSG_SYN_SKILL_INFLUENCE_MPCHANGE : public MSGROOT
{
	unsigned long	mInfluenceIdx;	/// 효과 고유번호
	sObject			mTarget;		/// 타겟
	short			mChangeValue;	/// 변동값
};

struct MSG_SYN_SKILL_APPLY_HPMP : public MSGROOT
{
	sObject			mTarget;
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
};

struct MSG_SYN_SKILL_APPLY_DIE : public MSGROOT
{
	sObject			mAttacker;				/// 공격자
	sObject			mTarget;
	unsigned char	mDamageKind;
};

struct MSG_SYN_SKILL_NONSKILL_HPCHANGE: public MSGROOT
{
	unsigned long	mInfluenceClassIdx;		/// 효과 종류 번호
	sObject			mTarget;				/// 타겟
	short			mChangeValue;			/// 변동값
};

struct MSG_SYN_SKILL_NONSKILL_MPCHANGE : public MSGROOT
{
	unsigned long	mInfluenceClassIdx;		/// 효과 종류 번호
	sObject			mTarget;				/// 타겟
	short			mChangeValue;			/// 변동값
};

struct MSG_RES_SKILL_RESURRECTION : public MSGROOT
{
	unsigned long	mAttackerIdx;			/// 부활스킬을 써준플레이어
};

struct MSG_SYN_SKILL_RESURRECTION : public MSGROOT
{
	sObject				mTarget;		/// 부활된 타겟
	unsigned long		mTargetHP;		/// hp회복량
	unsigned long		mTargetMaxHp;
	unsigned long		mTargetMP;		/// mp회복량
	unsigned long		mTargetMaxMp;
};

struct MSG_RES_SKILL_CHGMON_COOLTIME : public MSGROOT
{
	unsigned long	mSkill1CoolTime;
	unsigned long	mSkill2CoolTime;
	unsigned long	mSkill3CoolTime;
};

struct MSG_SYN_SKILL_INFLUENCE_GUARDCHANGE : public MSGROOT
{
	unsigned long	mInfluenceIdx;
	unsigned char	mChangeCnt;
	unsigned short	mChangeValue;
};

struct MSG_SYN_SKLL_PUSHPULL : public MSGROOT
{
	sObject			mTarget;
	float			mPosX;
	float			mPosY;
};

#pragma pack(pop)
