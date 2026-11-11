/* ========================================================================== 
* 파 일 : Monster_Common.h 
* 목 적 : 몬스터 데이타 
* 작 성 자 : 박경희 
* 작 성 일 : 2006-10-02 
* 주의사항 : 
*===========================================================================*/ 

#pragma once 

#include "CommonNetworkMsgStruct.h" 
#include "BaseObject_Common.h" 

class cMonster;

//struct sItemDropData;
enum eItemSlotSize;

#pragma pack(push,1) 

const unsigned int MAX_PATH_COUNT = 128;

const int SWITCH_RANDTIME = 57 * SECOND;			/// 채널간 선택적 리젠시 동시 리젠을 줄이기위해 리젠시간에 약간의 랜덤폭을 줌
const int SWITCH_CHANNEL_CHECKTIME = 3 * SECOND;	/// 채널별 선택적인 리젠 그룹선택시 리젠을 하기로한 채널의 동기를 맞춰주는 시간
const float SWITCH_SYNCTIME_RATE = 0.5f;			/// 리젠된 채널에서 몬스터가 살아있음을 주기적으로 알려주는 시간 리젠시간*비율

/// 몬스터 상태값 
enum eACTION_ID 
{ 
	eACT_MOVE = 3,			/// 몬스터 주위 이동 
	eACT_FOLLOW = 4,		/// 몬스터 타겟 추적 
	eACT_ATTACK = 5,		/// 몬스터 공격(전투) 
	eACT_IDLE = 6,			/// 몬스터 대기(정지) 상태(모드 효과 연출중)
	eACT_ESCAPE = 7,		/// 몬스터 도망 
	eACT_DIE = 8,			/// 몬스터 죽음 
	eACT_PUSHPULL = 9,		/// 강제 밀림/땡겨짐
};

/// 상태 변경 값 
enum eACTION_CHANGE 
{ 
	eACTCH_CONTINUE	= 1,			/// 프로세스 계속 수행		 

	eACTCH_ERROR = 10,				/// 스크립트 오류
	eACTCH_ESCAPE,					/// 몬스터 도망 

	eACTCH_MOVE_END		= 30,		/// 이동 프로세스 종료 

	eACTCH_FOLLOW_END	= 40,		/// 추격 종료 
	eACTCH_FOLLOW_ATTACK,			/// 추격 프로세스중 공격	

	eATTCH_ATTACK_END	= 50,		/// 전투 종료 
	eATTCH_ATTACK_FOLLOW,			/// 전투중 추격				 

	eACTCH_MODE_END		= 60,		/// 모드 대기 상태 종료

	eACTCH_ESCAPE_END	= 70,		/// 도망 프로세스 종료 
	eACTCH_ESCAPE_FOLLOW,			/// 도망끝 추적 

	eACTCH_DIE_END		= 80,		/// 죽음 프로세스 종료 

	eACTCH_PUSHPULL_END	= 90,		/// 강제 밀림 이동 끝(공격) 
	eACTCH_PUSHPULL_IDLE,			/// 이전 대기중이던 상태 복귀
};


/// 타겟 변경 방법 
enum eTARGET_CHANGE 
{ 
	eTARGETCHANGE_RAND = 0,			/// 랜덤 선택 
	eTARGETCHANGE_OLDTARGET,		/// 기존 타겟 고정 
	eTARGETCHANGE_NEAR,				/// 가장 가까운 대상 
	eTARGETCHANGE_TOPDAMAGE,		/// 데미지 합이 가장 많은 대상 
	eTARGETCHANGE_LOWLEVEL,			/// 레벨이 가장 낮은 대상 
	eTARGETCHANGE_TOPLEVEL,			/// 레벨이 가장 높은 대상 
	eTARGETCHANGE_TOPHEAL,			/// HP를 가장 많이 회복시켜준 대상 - 세부 설정 필요(우선 사용안함) 
};


/// 도망조건 
enum eESCAPE_TYPE 
{ 
	eESCAPETYPE_NONE,				/// 도망안감 
	eESCAPETYPE_HP50LOW,			/// HP 50%이하 
	eESCAPETYPE_HP30LOW,			/// HP 30%이하 
};

enum eMOVE_SPEED_TYPE
{
	eMOVESPEED_STOP,		/// 정지
	eMOVESPEED_WALK,		/// 걷기
	eMOVESPEED_RUN,			/// 뛰기
};

/// 몬스터 속성
enum eMONSTER_TYPE
{
	eMONSTERTYPE_HUMAN = 1,			/// 인간
	eMONSTERTYPE_ANIMAL,			/// 동물
	eMONSTERTYPE_PLANT,				/// 식물
	eMONSTERTYPE_BUG,				/// 곤충
	eMONSTERTYPE_SOUL,				/// 정령
	eMONSTERTYPE_UNDEAD,			/// 언데드
	eMONSTERTYPE_GIANT,				/// 거인
	eMONSTERTYPE_DRAGON,			/// 용
	eMONSTERTYPE_CHARACTER,			/// 캐릭터

	eMONSTERTYPE_MAX,
};

/// 몬스터 상황별 대사 갯수
const unsigned long MONSTER_TALK_SIZE = 20;

/// 대화 상황값
enum eMONSTER_TALK_STATE
{ 
	eMONSTERTALK_STAND = 0,
	eMONSTERTALK_WORK,
	eMONSTERTALK_RUN,
	eMONSTERTALK_FIGHT,
	eMONSTERTALK_ATTACK1,
	eMONSTERTALK_ATTACK2,
	eMONSTERTALK_SKILL1,
	eMONSTERTALK_SKILL2,
	eMONSTERTALK_SKILL3,
	eMONSTERTALK_DAMAGE,
	eMONSTERTALK_FOLLOW,
	eMONSTERTALK_GIVEUP,
	eMONSTERTALK_ESCAPE,
	eMONSTERTALK_DIE1,
	eMONSTERTALK_DIE2,

	eMONSTERTALK_MAX,

	eMONSTERTALK_ATTACKSTART = eMONSTERTALK_ATTACK1,
	eMONSTERTALK_ATTACKEND = eMONSTERTALK_SKILL3,

};

/// 대사 표시 설정
enum eMONSTER_TALK_VIEWTYPE
{
	eM_TALKVIEW_BALLOON	= 0, 	/// 말풍선
	eM_TALKVIEW_NORMAL,		 	/// 말풍선 + (일반)채팅창
	eM_TALKVIEW_SHOUT,		 	/// 말풍선 + (외치기)채팅창
};

// 몬스터 분루
enum eMONSTER_KIND_TYPE
{
	eMONSTER_KIND_NORMAL = 0,
	eMONSTER_KIND_VETERAN,
	eMONSTER_KIND_ELITE,
	eMONSTER_KIND_BOSS,

	eMONSTER_KIND_MAX,
};


/// 몬스터리스트 
struct sMonsterScript 
{ 
	//몬스터 종류 index 
	unsigned int	mMonsterClassIdx;
	/// 몬스터 이름 
	TCHAR			mName[256];
	/// 몬스터 직업이름
	TCHAR			mJobName[256];
	/// kfm파일idx 
	unsigned long	mKfmIdx;
	/// 이름 높이값 
	unsigned long	mNameHeight;
	/// 분류
	unsigned char	mKind;
	/// 속성 
	unsigned char	mType;
	/// 레벨 
	unsigned char	mLevel;
	/// HP 
	unsigned long	mMaxHp;
	/// MP 
	unsigned long	mMaxMp;
	/// exp획득량 
	unsigned long	mExp;
	/// sxp획득량 
	unsigned long	mSxp;
	/// 물리방어 
	unsigned short	mDef;
	/// 마법방어 
	unsigned short	mMdef;
	/// 명중수치
	unsigned long	mHit;
	/// 회피율 
	unsigned short	mAvoid;
	/// 시야범위 
	unsigned long	mSeekRange;
	/// 비중 
	unsigned char	mImportance;
	/// 크기 
	float			mMonsterScale;
	/// 연출이펙트 크기
	float			mEffectScale;
	/// 피해범위 - 보간 사이즈 
	float			mMonsterFixSize;
	/// 행동범위	 
	unsigned long	mFreeMoveRange;
	/// 행동(정지) 
	unsigned short	mActionStopRatio;
	/// 행동(걷기) 
	unsigned short	mActionWalkRatio;
	/// 행동(뛰기) 
	unsigned short	mActionRunRatio;
	/// 정지시간 
	unsigned long	mActionStopTime;
	/// 이동속도(걷기) 
	unsigned long	mActionWalkSpd;
	/// 이동속도(뛰기,도망) 
	unsigned long	mActionRunSpd;
	/// 선공여부 
	bool			mIsPrecedeAttack;
	/// 변신인식
	bool			mIsChgMonCheck;
	/// 추적시간 
	unsigned long	mFollowTime;
	/// 추적거리 
	unsigned long	mFollowRange;
	/// 도망조건 
	unsigned char	mEscapeType;
	/// 도망확률 
	unsigned char	mEscapePer;
	/// 회복여부 
	bool			mIsRecovery;
	/// 회복시작시간 
	unsigned long	mRecoveryTime;
	/// 추적 최소거리 관련정보
	float			mSkillMinRange;		// 스킬 최소거리
	unsigned char	mSkillMinRangeType;	// 최소거리 스킬의 거리 타입

	tPointerHashMap<unsigned long, void*>*	mpQuestItemDropMap;

	sMonsterScript()
	{
		mpQuestItemDropMap = NULL;
		mSkillMinRange = 0.0f;
		mSkillMinRangeType = 0;
	}
};



/// 몬스터 그룹 정보 
struct sRegenGroupScript 
{ 
	/// 그룹번호 
	unsigned long	mGroupIdx;
	/// 최초리젠설정 
	bool			mServerOnRegen;
	/// 몬스터 리젠 퍼센티지 
	unsigned char	mRegenPer;
	/// 리젠범위 
	unsigned long	mRegenRange;
	/// 그룹리젠시간 
	unsigned long	mRegenTime;
	/// 맵 그룹번호
	unsigned long	mMapGroupIdx;
	/// 채널 공유 체크
	bool			mChannelCheck;

	/// 리젠 그룹 번호 Arr
	tArray<unsigned long> mRegenGroupAry;
	
	tArray<unsigned long>	mMonsterAry;
};



/// 몬스터 리젠 정보 
struct sRegenScript 
{ 
	/// 몬스터 리젠 번호 
	unsigned long	mRegenIdx;
	/// 몬스터 종류 
	unsigned int	mMonsterClassIdx;
	/// 맵번호 
	unsigned short	mMapNumber;
	/// 모드
	unsigned char	mLevelMode;
	/// 리젠그룹번호 
	unsigned long	mGroupNumber;
	/// 리젠 x좌표 
	float			mRegenX;
	/// 리젠 y좌표 
	float			mRegenY;
	/// 방향값 
	float			mDirection;
	/// 리젠시간 
	unsigned long	mRegenTime;
	/// 리젠 유지시간
	unsigned long	mRegenLifeTime;
	/// 동족그룹번호
	unsigned long	mFamilyNum;
	/// 패턴인덱스
	unsigned long	mPatternIdx;
	/// 행동그룹번호
	unsigned long	mActionGroupNum;
};


/// 몬스터 패턴 코스 정보
struct sMonsterPatternPos
{
	float			mPosX;
	float			mPosY;
	unsigned long	mRange;
	bool			mRelaxPoint;
	unsigned long	mWaitTime;
};

/// 몬스터 패턴 상위 정보
struct sMonsterPattern
{
	unsigned long	mPatternIdx;		/// 이동 패턴 인덱스
	bool			mType;				/// true 순환 false 역주행

	/// <sMonsterPatternPos*>
	tArray<void*>	mPosAry;
};


/// 몬스터스텟 
struct sMonsterStatus 
{ 
	float			mPhysicAttack;		/// 물리:근거리공격력 
	float			mPhysicRangeAttack;	/// 물리:원거리공격력 
	float			mPhysicDefense;		/// 물리:방어력 
	float			mMagicAttack;		/// 마법:공격력 
	float			mMagicDefense;		/// 마법:방어력 
	float			mPhysicCritical;	/// 물리:크리티컬 공격
	float			mMagicCritical;		/// 마법:크리티컬 공격
	float			mPhysicCriticalDef;	/// 물리:크리티컬 방어
	float			mMagicCriticalDef;	/// 마법:크리티컬 방어
	float			mMaxHP;				/// 최대HP 
	float			mMaxMP;				/// 최대MP 
	float			mPhysicAttackRate;	/// 물리:명중률 
	float			mMagicAttackRate;	/// 마법:명중률
	float			mPhysicAvoid;		/// 물리:회피
	float			mMagicAvoid;		/// 마법:회피
	float			mAttackSpeed;		/// 일반공격속도 
	float			mCooltime;			/// cooltime 
	float			mInRange;			/// 근거리사정거리 
	float			mOutRange;			/// 원거리(+마법)사정거리 
	float			mMoveSpeed;			/// 이동속도 

	sMonsterStatus() 
	{ 
		memset(this, 0, sizeof(sMonsterStatus));
	} 
};


/// 몬스터 전체 데이터 
struct sMonsterData 
{ 
	unsigned long	mMonsterIdx;
	unsigned int	mMonsterClassIdx;
	float			mPosX;
	float			mPosY;
	float			mDirection;
	unsigned int	mHP;
	unsigned int	mMaxHP;
	unsigned int	mMP;
	unsigned int	mMaxMP;
	unsigned short	mMoveSpeed;
	unsigned short	mScale;
	char			mAutoDummy;		// 0:아님, 1:오토방지용더미
};

/// 몬스터 버프 리스트
struct sMonsterInfluence
{
	unsigned long	mUniqueIdx;
	unsigned long	mInfluenceClassIdx;
};


/// 몬스터 대화 스크립트
struct sMonsterTalk
{
	unsigned char	mViewType;						/// 표시 설정 ( eMONSTER_TALK_VIEWTYPE )
	unsigned char	mViewPercent;					/// 표시 확률
	unsigned long	mCheckTime;						/// 대사 표시 체크 시간
	unsigned long	mTalkIdx[MONSTER_TALK_SIZE];	/// 대사 인덱스

};

/// 소환 패턴
struct sSummonPatternScript
{
	float			mDir;
	unsigned long	mDelayTime;
};


///// 소환 패턴 정보
//struct sSummonPatternInfo
//{
//	sObject			mObject;
//	unsigned long	mSummonPatternIdx;
//	unsigned long	mCount;
//	NiPoint2		mCenterPos;
//	float			mCenterDir;
//	unsigned long	mRangeMin;
//	unsigned long	mRangeMax;
//	unsigned char	mNowPatternPos;
//	unsigned long	mNextPatternTime;
//};

struct MSG_SYN_MOVE_MONSTER : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned short	mCount;
	unsigned short	mMoveSpeed;
	unsigned char	mMoveSpeedType;

	NiPoint2		mPathArray[1];		/// 길찾기 경로 - 길찾기 노드 위치 

	/// 실제보내는 메세지 크기 
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_MOVE_MONSTER) - sizeof(mPathArray) + (mCount * sizeof(NiPoint2));} 
};

struct MSG_SYN_ACTIONMOVE_MONSTER : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned short	mCount;
	unsigned short	mMoveSpeed;
	float			mRange;				/// 범위 
	sObject			mTarget;			/// 타겟 정보 
	NiPoint2		mPathArray[1];		/// 길찾기 경로 

	/// 실제보내는 메세지 크기 
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_ACTIONMOVE_MONSTER) - sizeof(mPathArray) + (mCount * sizeof(NiPoint2));} 
};

/// 몬스터 구조체 
struct MSG_MONSTERTEAM_INFO : public MSGROOT 
{ 
	/// 몬스터 정보 
	sMonsterData	mMonsterInfo;
	/// 몬스터 팀정보
	unsigned long	mTeamType;
	/// 효과 갯수
	unsigned short	mCount;
	/// 몬스터 효과 정보
	sMonsterInfluence	mInfluence[1];

	/// 실제보내는 메세지 크기 
	unsigned long GetMsgLength() { return sizeof(MSG_MONSTERTEAM_INFO) - sizeof(mInfluence) + (mCount * sizeof(sMonsterInfluence));} 
};

/// 몬스터 구조체 
struct MSG_MONSTER_INFO : public MSGROOT 
{ 
	/// 몬스터 정보 
	sMonsterData	mMonsterInfo;
	/// 효과 갯수
	unsigned short	mCount;
	/// 몬스터 효과 정보
	sMonsterInfluence	mInfluence[1];

	/// 실제보내는 메세지 크기 
	unsigned long GetMsgLength() { return sizeof(MSG_MONSTER_INFO) - sizeof(mInfluence) + (mCount * sizeof(sMonsterInfluence));} 
};

/// 몬스터 삭제
struct MSG_MONSTER_SIGHTOUT : public MSGROOT 
{ 
	/// 몬스터 고유 인덱스
	unsigned long	mMonsterIdx;
	///// 효과 갯수
	//unsigned long	mCount;
	///// 몬스터 효과 인덱스
	//unsigned long	mInfluenceUniqueIdx[1];

	///// 실제보내는 메세지 크기 
	//unsigned long GetMsgLength() { return sizeof(MSG_MONSTER_SIGHTOUT) - sizeof(mInfluenceUniqueIdx) + (mCount * sizeof(unsigned long));} 
};


/// 모델 데이타 로드에 필요한 정보 발송
struct MSG_MODELNUMBER : public MSGROOT 
{ 
	unsigned short	mCount;
	unsigned long	mClassIdx[1];
	/// 실제보내는 메세지 크기 
	unsigned long GetMsgLength() { return sizeof(MSG_MODELNUMBER) - sizeof(mClassIdx) + (mCount * sizeof(unsigned long));} 
};


struct MSG_SYN_MONSTER_DESTROY : public MSGROOT
{
	unsigned long	mMonsterIdx;
};

struct MSG_SYN_MONSTER_HP : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned long	mHP;
	unsigned long	mMaxHP;
	bool			mDie;
};

struct MSG_SYN_MONSTER_MP : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned long	mMP;
	unsigned long	mMaxMP;
};

struct MSG_SYN_MONSTER_MOVESPEED : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned short	mMoveSpeed;
};

struct MSG_SYN_MONSTER_SCALE : public MSGROOT 
{ 
	unsigned long	mMonsterIdx;
	unsigned short	mScale;
};

struct MSG_SYN_MONSTER_MOVESTOP : public MSGROOT
{
	unsigned long	mMonsterIdx;
	NiPoint2		mMonsterPos;
};

struct MSG_SYN_MONSTER_IDLE2ANI : public MSGROOT
{
	unsigned long	mMonsterIdx;
};

struct MSG_SYN_MONSTER_SPEECH : public MSGROOT
{
	unsigned long	mMonsterIdx;		/// 몬스터 고유 번호
	unsigned char	mViewType;			/// 대사 표시 설정 eMONSTER_TALK_VIEWTYPE
	unsigned long	mTalkIdx;			/// 대사 인덱스
	unsigned long	mCharacterIdx;		/// 캐릭터 고유 번호
};

struct MSG_SYN_MONSTER_MODECHANGE : public MSGROOT
{
	unsigned long	mMonsterIdx;		/// 몬스터 고유번호
    char			mModeIdx;			/// 모드 번호
};

struct MSG_SYN_MOSTER_MODESPEECH : public MSGROOT
{
	unsigned long	mMonsterIdx;		/// 몬스터 고유 번호
	unsigned long	mTalkIdx;			/// 대사 인덱스
	unsigned long	mCharacterIdx;		/// 캐릭터 고유 번호
};

struct MSG_SYN_POSTMONSTER_SWITCHREGEN : public MSGROOT
{
	unsigned short	mLevelMode;
	unsigned long	mSwitchRegen;
	
};

#pragma pack(pop) 
