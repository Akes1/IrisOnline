/* ==========================================================================
* 파    일 : Monster.h
* 목    적 : 몬스터 클래스
* 작 성 자 : 박경희
* 작 성 일 : 2006-09-20
* 주의사항 : 몬스터는 풀에서 메모리를 할당 받기때문에 객체 생성소멸 override함
*===========================================================================*/

#pragma once

#include "BaseObject.h"

#include "actionmove.h"
#include "ActionDie.h"
#include "ActionAttack.h"
#include "ActionEscape.h"
#include "ActionFollow.h"
#include "ActionIdle.h"
#include "ActionPushPull.h"
#include "Monster_Common.h"
#include "RangeCheck.h"
#include "Skill_Common.h"

#include "TakeDamagePool.h"
#include "SummonPatternPool.h"

class cPathFinder;
enum eOBJECTTYPE;

class cModeAgent;
class cMode;

const unsigned long TARGET_CHECK_RANGE = 5000;		/// 타겟 선정시 유효범위
const unsigned long	ESCAPE_HP50 = 50;				/// 도망시 조건
const unsigned long	ESCAPE_HP30 = 30;
const unsigned long SKILL_USE_HP40 = 40;			/// 스킬 사용 조건
const unsigned long SKILL_USE_HP80 = 80;
const unsigned long TALK_DELAY = 1000;				/// 대사간 최소 간격
const unsigned long SKILL_WAIT_TIME = 500;			/// 스킬 재사용 대기 시간
const unsigned long MAX_DUMMY_DAMAGE = 5;			/// 더미 공격 최대 허용 횟수

struct sMonsterTakeDamageInfo
{
	unsigned long takeDamage;
	unsigned long distressPoint;
};

class cMonster : public cBaseObject
{

public:
	/// 생성소멸
	cMonster( void );
	~cMonster( void );

	/// objectmanager의 메모리풀에서 몬스터 공간을 할당받음
	void* operator new ( size_t n ); 
	/// 메모리풀에서 할당 받은 몬스터를 제거해줌
	void operator delete ( void* ptr, size_t n );


	/// 일반적인 생성
	bool				Init( unsigned long uniqueidx, unsigned long classIdx, unsigned long regenIdx, unsigned short mapNumber, unsigned char modeNum );
	void				Release( bool parentLinkRelase = true );

	/// 리젠 스크립트로 생성된 몬스터가 아닌 외부 소환으로 생성된 몬스터
	void				SetSummonInfo( cMonster* pParent, unsigned short mapNumber, float regenX, float regenY, 
									   unsigned long regenMin, unsigned long regenMax, unsigned long destroyTime, bool isRand = true, bool isAutuDummy = false );

	void				SetDMTeamType( unsigned long teamType )	{ mDMTeamType = teamType; }
	unsigned long		GetDMTeamType()	{ return mDMTeamType; }
	void				SendTeamRegen();

	unsigned long		GetfirstAttack()	{ return mFirstAttackCharacter; }
	void				SetFirstAttack( unsigned long playerIdx )	{ mFirstAttackCharacter = playerIdx; }
	void				SetLastAttack( unsigned long playerIdx )	{ mLastAttacker = playerIdx; }

	cBaseObject*		GetTarget();
	bool				SetTarget ( unsigned char kind, unsigned long idx );

	void				OutsideDestroyTrue()	{ mOutsideDestroy = true; }
	bool				IsCheatTempMonster()	{ return mOutsideAddMonster; }
	void				Die();

	void				SendRegen();
	NiPoint2			CalcMinMaxRandPos( float centerX, float centerY, unsigned long rangeMin, unsigned long rangeMax );

	///*--------------- 스크립트 관련 참조 데이타 ---------------*/
	const sMonsterScript*	GetMonsterInfo()	{ return mpMonsterInfo; }
	const sRegenScript*	GetRegenMonsterInfo()	{ return mpRegenMonsterInfo; }
	const sMonsterPattern*	GetMovePattern()	{ return mpMovePattern; }


	/*--------------- 기본 정보 ---------------*/
	unsigned int		GetRaceGender()	{ return mpMonsterInfo->mMonsterClassIdx; }
	unsigned long		GetRegenGroupNum()	{ return mpRegenMonsterInfo->mGroupNumber; }
	float				GetDirection()	{ return mDirection; }
	unsigned char		GetLevel( ) { return mpMonsterInfo->mLevel; }


	/*--------------- 스킬 관련 ---------------*/
	/// 효과 목록 추가
	//bool				AddInfluence( unsigned long /*uniqueIdx*/, unsigned long /*influenceClassIdx*/ );
	bool				AddInfluence( cInfluenceObject* pInf );	
	tHashSet<unsigned long>* GetInfluenceSet() { return &mInfluenceSet; }
	void				ReleaseInfluenceSet( bool logOut ); 
	void				DeleteInfluence( unsigned long influenceIdx );

	bool				IsParentEqual( unsigned long parentUniqueIdx );

	// 오토더미용 몬스터
	bool				IsAutoDummy() { return (mAutoDummy==1); }

	/// 받은 데미지 정보 기록

	/// 몬스터에 피해를 주는 객체에 대한 정보를 등록
	void AddTakeDamage( sObject attacker, unsigned long damage, long distressPoint, eTAKEDAMAGE_TYPE type );
	void DelTakeDamagePlayer( unsigned long playerIdx );
	void ClearDistressPointPlayer( unsigned long playerIdx );
	void ClearDistressPoint();
	void ClearTakeDamage();

	/// 오토더미 공격 정보 저장
	void AttackDummy( sObject attacker );

	/// 해당 플레이어가 가지고 있는 어그로 수치를 % 만큼 증가시킨다.
	void AddDistressPer( sObject attacker, long distressPointPer, eTAKEDAMAGE_TYPE type );

	/// 해당 attaker와 target의 어그로 포인트를 교체한다.
	void SwapDistressPoint( unsigned long attackerIdx, unsigned long targetIdx );

	//void				AddTakeDamage( unsigned long playerIdx, unsigned long takeDamage );
	unsigned long		GetTakeDamage( unsigned long playerIdx );
	TakeDamageRoot*		GetTakeDamageRoot() { return &mTakeDamageRoot; }

	/// 스킬 선정
	eMONSTERATTACK_TYPE	SelectSkill();
	eMONSTERATTACK_TYPE	SelectApplyTargetSkill( unsigned long monsClassIdx, eMONSTERATTACK_TYPE attackType );
	/// 스킬 쿨타임 갱신
	void				UpdateSkillCoolTime( eMONSTERATTACK_TYPE attackType );

	/// 스킬 발동후 대기 시간
	void				CalcSkillWaitTime( unsigned long skillEndTime );
	bool				IsSkillWaitEnd();

	/// AI에 의해 공격대상이 변경되어야 하는경우 호출
	void				ChangeActionTarget();

	/// 타겟 제거 스킬에 인한 타겟 변경
	void				ChangeTargetDeleteSkill();

	/// 면역 효과 처리
	void				ClearImmune()	{ mImmuneBuffSet.Clear(); mImmuneDebuffSet.Clear(); mImmuneKindSet.Clear(); }
	void				AddImmuneApplyType( unsigned char InfluenceType, unsigned short applyType );
	void				AddImmuneKind( unsigned long infKind );
	bool				IsImmuneApplyType( unsigned char InfluenceType, unsigned long applyType );
	bool				IsImmuneApplyAll( unsigned long applyType );
	bool				IsImmuneKind( unsigned long infKind );

	void				SetSkillLastAttack( sObject attacker, unsigned char kind )	{ mSkillLastAttack = attacker; mSkillLastKind = kind; }
	sObject				GetSkillLastAttack()	{ return mSkillLastAttack; }
	unsigned char		GetSkillLastKind()	{ return (unsigned char)mSkillLastKind; }

	unsigned long		CalcChangeMPDamage( unsigned char attributeType, unsigned long damage, bool applyHP );
	unsigned long		CalcGuard( unsigned char attributeType, unsigned long damage );

	/*--------------- 스테이터스 관련 ---------------*/
	sMonsterStatus*		GetStatusPlus()	{ return &mStatusPlus; }
	sMonsterStatus*		GetStatusPer()	{ return &mStatusPer; }

	unsigned long		CalcPhysicAttack( unsigned long skillAttack );
	unsigned long		CalcPhysicRangeAttack( unsigned long skillAttack );
	unsigned long		CalcMagicAttack( unsigned long skillAttack );
	unsigned long		GetPhysicDefense();
	unsigned long		GetMagicDefense();
	unsigned long		GetPhysicAvoid();
	unsigned long		GetMagicAvoid();
	unsigned long		GetPhysicAttackRate();
	unsigned long		GetMagicAttackRate();
	//unsigned long		CalcPhysicCritical( unsigned long physicCritical );
	//unsigned long		CalcMagicCritical( unsigned long magicCritical );
	float				GetAttackSpeed();

	float				CalcStatusSkillRange( float skillRange, eRANGETYPE rangeType );

	/// 현재 몬스터의 이동속도
	unsigned short		GetMoveSpeed();
	unsigned short		SetMoveSpeed( unsigned long moveSpeed, eMOVE_SPEED_TYPE mMoveSpeedType );
	void				SendMoveSpeed();

	/// 최대 hp/mp
	unsigned long		GetMaxHP();
	unsigned long		GetMaxMP()	{ return (unsigned long)( mpMonsterInfo->mMaxMp + ( mpMonsterInfo->mMaxMp * mStatusPer.mMaxMP ) + mStatusPlus.mMaxMP ); }

	/*--------------- 스테이터스 관련 기타---------------*/
	/// HP/MP 정보
	unsigned long		GetHP()	{ return mHP; }
	unsigned long		GetMP()	{ return mMP; }

	/// HP/MP 증감
	unsigned long		HPDamage( sObject attacker, unsigned long damage, bool *die, bool msgSend, bool cri = false );
	void				HPUse( unsigned long useValue, bool msgSend );
	void				HPHeal( unsigned long hp, bool msgSend, sObject healer, long distressPoint, eTAKEDAMAGE_TYPE type );
	void				MPDamage( unsigned long damage, bool msgSend );
	void				MPUse( unsigned long useValue, bool msgSend );
	void				MPHeal( unsigned long heal, bool msgSend );

	unsigned long		GetOriHPMax() { return mpMonsterInfo->mMaxHp; }
	unsigned long		GetOriMPMax() { return mpMonsterInfo->mMaxMp; }


	/// 데미지 처리
	bool				AttackSuccess( sObject target, unsigned long attributeType, short accuracyValue, unsigned long instantSuccess, unsigned long instantSuccessPer );
	bool				CriticalSuccess( sObject target, unsigned long attributeType, unsigned short criticalValue, unsigned long instantCri, unsigned long instantCriPer );
	unsigned long		DamageCalc( sObject target, unsigned long skillIdx, unsigned long attributeType, unsigned long rangeType,
		unsigned long skillDamage, unsigned long skillDamagePer,bool isIgnoreDefence, float addTotalDamage, float addPerTotalDamage );
	/// 다른 계산과 다르게 도트데미지는 공격자가없어도 계산을 해야되기때문에 기준객체가 타겟객체다!!
	unsigned long		DotDamageCalc( unsigned char attackerType, unsigned char attackerLv, unsigned long attackMin, 
		unsigned long attackMax, unsigned char dotType, unsigned long skillDamage, 
		unsigned long skillDamagePer, unsigned long totalTime, unsigned long dotTime );
	unsigned long		HealCalc(  sObject /*target*/, long /*skillHealPlus*/, long /*skillHealPer*/, bool /*applyHP*/ )  { assert(NULL); return 0; }
	unsigned long		DotHealCalc( sObject /*target*/, long /*infHealPlus*/, long /*infHealPer*/, 
		unsigned long /*heal*/, unsigned long /*dotTotalTime*/, unsigned long /*dotTime*/ )	{ assert(NULL); return 0; }
	unsigned long		ConditionOdd( sObject target, long influenceRate, unsigned long skillClassIdx );

	void				StatusCalc();

	/*--------------- 상태 관련 ---------------*/
	/// 액션 수행 - 액션 진행/중지 상태값 리턴
	eACTION_CHANGE		Update( unsigned long elapsedTime, unsigned long accumTime );
	void				UpdateMode( unsigned long elapsedTime, unsigned long accumTime );

	bool				DeathUpdate();

	void				ActionChange( eACTION_ID state );
	eACTION_ID			GetActionID()	{ return mpAction->GetActionID(); }

	void				SetGroupRegen( bool groupRegen )	{ mGroupRegen = groupRegen; }
	bool				IsGroupRegen()	{ return mGroupRegen; };

	/// 추적시 공격범위에 들어온경우 공격형태
	void				SetAttackType( eMONSTERATTACK_TYPE attackType ) { mAttackType = attackType; }
	eMONSTERATTACK_TYPE	GetAttackType() { return mAttackType; }
	void				SetLastUsedSkillIdx( unsigned long lastUsedSkillIdx )	{ mLastUsedSkillIdx = (eMONSTERATTACK_TYPE)lastUsedSkillIdx; }

	void				SkillEscape( unsigned long type );
	void				SetEscape();

	void				Comeback();
	void				ReservationComeback();
	void				CheckComback();

	/// 몬스터 생존 유무
	bool				GetStateDie();

	/// 외부 몬스터 삭제 설정
	void				OutsideDestroy();

	/// 대기 상태 종료시간
	unsigned long		GetIdleEndTime()	{ return mIdleEndTime; }
	void				SetIdleEndTime( unsigned long endTime )	{ mIdleEndTime = endTime; }

	bool				GetDeathMonster()	{ return mDeathMonster; }

	/*--------------- 상태 이상 ---------------*/
	void				SetStateOddity( long* pOddity );
	unsigned long		GetStateOddity( eODDITYTYPE oddityType )	{ return mODDITY[oddityType]; }

	bool				IsCantSkill( unsigned long attributeType );
	bool				IsCantSkill( unsigned long attributeType, eMONSTERATTACK_TYPE attackType );
	bool				IsCantMove()	{ return mODDITY[eODDITYTYPE_CANTMOVE] != 0 || mODDITY[eODDITYTYPE_SLEEP] != 0 || mODDITY[eODDITYTYPE_STUN] != 0; }

	void				TargetChange( sObject attacker, long distressPoint );

	void				SetSkillPushPull( unsigned short type, cBaseObject* pAttacker, unsigned long range );
	bool				IsSkillPushPull()	{ return GetActionID() == eACT_PUSHPULL; }
	bool				IsPushPullPossible( NiPoint2 startPos, NiPoint2 goalPos );

	/*--------------- 이동 관련 ---------------*/
	/// 이동 프로세스
	bool				MoveUpdate( unsigned long deltaTime );
	bool				PushPullUpdate( unsigned long deltaTime );

	/// 다음 이동 경로 최종위치 선정
	bool				SelectNextPath( float* pPosX, float* pPosY );
	void				GetNextPattern( long* pPatternPos, char* pPatternDir );

	/// 이동 경로 셋팅
	unsigned int		SetPath( float targetX, float targetY );
	unsigned int		SetPath( float targetX, float targetY, float range , sObject target );

	/// 이동좌표 최종 목적지까기 가기전 정지의 범위
	float				GetMoveStopRange() { return mMoveStopRange; }
	void				SetMoveStopRange( float moveStopRange ) { mMoveStopRange = moveStopRange; }

	/// 몬스터가 실제 리젠된 좌표
	const NiPoint2&		GetRegenPos() const	{ return mRegenPos; }

	/// 추적 시작시 기준 정보
	NiPoint2			GetFollowStartPos()	{ return mFollowStartPos; }
	void				SetFollowEndTime( unsigned long followEndTime )	{ mFollowEndTime = followEndTime; }
	unsigned long		GetFollowEndTime()	{ return mFollowEndTime; }		

	/// 추적하다가 원래 이동 구역으로 이동중
	bool				GetFollowEndMove()	{ return mFollowEndMove; }
	void				SetFollowEndMove( bool followEndMove )	{ mFollowEndMove = followEndMove; }

	/// 이동 정지
	void				MoveStop( bool sendMsg );
	/// 방향 설정
	void				CalcDirection( NiPoint2 targetPos );

	/// 처음 이동
	bool				IsFirstMove()	{ return mIsFirstMove; }
	bool				FirstMoveEnd()	{ return mIsFirstMove = false; }

	unsigned char		GetPlayerFollowPos()	{ return mPlayerFollowPos; }
	void				SetPlayerFollowPos( unsigned char playerFollowPos )	{ mPlayerFollowPos = playerFollowPos; }

	/// 길찾기
	bool				FindPath( const NiPoint2& goal );
	void				SetPathFinder( cPathFinder* pathFinder );
	cPathFinder*		GetPathFinder()	{ return mPathFinder; }
	unsigned int		GetPathCount() const;
	const NiPoint2*		GetPathArray() const;

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				GetFixedObjectSize()	{ return mFixedObjectSize; }
	void				SetFixedObjectSizePer( short sizePer );
	unsigned short		GetFixedObjectSizePer()	{ return mFixedObjectSizePer; }
	void				SendMonsterSize();

	unsigned char		GetFollowImportance()	{ return mFollowImportance; }

	/// 패턴 이동 후 대기시간
	unsigned long		GetPatternWait()	{ return mPatternWait; }
	void				ClearPatternWait()	{ mPatternWait = 0; }

	void				SetGroupMoveReady( unsigned char moveReady )	{ mGroupMoveReady = moveReady; }
	unsigned char		GetGroupMoveReady()	{ return mGroupMoveReady; }

	/*--------------- 대사 관련 ---------------*/
	/// 대사 선택 발송
	void				SendSpeech( unsigned long talkState );
	void				SpeechProcess();
	bool				IsCriDie()	{ return mCriDie; }

	
	/*--------------- 보스(엘리트) 관련 ---------------*/
	/// 소환된 몬스터 추가
	void				AddSummonMonster( unsigned long monsterIdx );
	void				DieSummonMonster( unsigned long monsterIdx, NiPoint2 diePos );

	void				ModeSkillEnd();
	eMONSTERATTACK_TYPE	GetModeSkill();

	void				TargetFixed( unsigned long playerIdx, unsigned long influenceIdx );
	void				TargetFixedEnd();

	void				LinkObstruction();
	void				DeathGenMonster();
	void				LinkTeleportPortal();

public:
	bool                SendSightIn     ( char category, char protocol, unsigned long connectionIdx );
	bool                SendSightOut    ( char category, char protocol, unsigned long connectionIdx );
	bool			    SendMonsterMove ( unsigned long connectionIdx );

	/// 몬스터 주변 그리드에 해당 플레이어가 존재하는지 검사
	cPlayer*			NearGridFindPlayer( unsigned long playerIdx, bool diePass );

	/// 변수 셋팅
	void SetCombatFlag( bool set );
	inline bool IsCombat() { return mIsCombat; }

	long				CalcSkillRate( cBaseObject* pTarget, eMONSTERATTACK_TYPE attackType, long scriptRate );

	bool				IsModeState() { return (mCurrentMode != 0); }
	tPointerArray<void*>*	GetBuffImmune();

	bool IsFollowContinue();

	// 캐릭터가 가지고 있는 buff/debuff 목록 체크
	bool IsHaveApplyBuffType( unsigned short applyValueType );
	bool IsHaveApplyDeBuffType( unsigned short applyValueType );

private:

	/// 액션 상태 변경
	void				SetAction( cAction& pNewAtion );

	/// 초기화
	bool				MonsterInit( unsigned long monsterClassIdx );

	/// 전투 시작/종료시점
	void CombatStart();
	void CombatEnd();

	/// 모드 변환
	void ChangeCurrentMode( long modeIdx );

	bool ConditionValueCheck( unsigned char type, unsigned long value );

	void ProcessTakeDamage( unsigned long elapsedTime, unsigned long accumTime );

private:

	typedef tPointerArray<void*> cPArray;
	typedef tArray<void*>	cArray;
	typedef tArray<unsigned long> cLongAry;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned long, unsigned long> cHashMap;
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;


	/// 범위 검사
	cRangeCheck			mRangeCheck;
	/// 선공 플레이어
	unsigned long		mFirstAttackCharacter;
	unsigned long		mFirstAttackParty;
	/// 마지막 공격 플레이어
	unsigned long		mLastAttacker;
	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				mFixedObjectSize;
	unsigned short		mFixedObjectSizePer;
	/// 정상 죽음이 아닌 외부삭제 명령으로 인한제거(경험치 관련 드롭안함)
	bool				mOutsideDestroy;
	/// 리젠 스크립트가 아닌 외부 호출로 생성된 몬스터
	bool				mOutsideAddMonster;

	/// 전장 팀타입
	unsigned long		mDMTeamType;

	///*--------------- 스크립트 관련 참조 데이타 ---------------*/
	sMonsterScript*		mpMonsterInfo;		/// 스크립트에서 참조한 몬스터 기본정보
	sRegenScript*		mpRegenMonsterInfo;	/// 몬스터 리젠정보
	sMonsterPattern*	mpMovePattern;
	
	unsigned char		mFollowImportance;
	/*--------------- 스킬 관련 ---------------*/
	/// 보유 효과 셋
	typedef tHashSet<unsigned long> cSkillInfluenceSet;
	cSkillInfluenceSet	mInfluenceSet;

	/// 받은 데미지
	TakeDamageRoot mTakeDamageRoot;

	/// 다음 타겟 변경 시간
	unsigned long		mNextTargetChangeTime;
	/// 쿨타임
	typedef tHashMap<unsigned long, unsigned long> cCoolTime;
	cCoolTime			mSkillCoolTimeMap;
	cCoolTime			mEliteCoolTimeMap;

	/// 스킬 공격 타입
	eMONSTERATTACK_TYPE	mAttackType;
	/// 스킬 재사용 가능 시간
	unsigned long		mSkillWaitEndTime;

	/// 마지막으로 사용한 스킬종류
	eMONSTERATTACK_TYPE	mLastUsedSkillIdx;

	/// 스킬 선정시 가능한 스킬들 목록을 담아둘 임시 공간
	cHashMap			mTempSelectSkill;

	typedef tHashSet<unsigned long> cHashSet;
	cHashSet			mImmuneKindSet;				/// 면역 효과종류 목록
	cHashSet			mImmuneBuffSet;				/// 면역 적용효과값 목록
	cHashSet			mImmuneDebuffSet;			/// 면역 적용효과값 목록

	sObject				mSkillLastAttack;
	unsigned char		mSkillLastKind;		/// eDAMAGE_KIND


	/// <unsigned long influenceIdx>
	cLongAry			mGuardAry;
	/// <unsigned long influenceIdx>
	cLongAry			mChangeMPDamageAry;
	/// <unsigned long influenceIdx>
	cLongAry			mDeleteTypeDamagedAry;

	/*--------------- 스테이터스 관련 ---------------*/
	unsigned long		mHP;
	unsigned long		mMP;

	/// 몬스터 스텟 기본값
	sMonsterStatus		mStatusBase;
	/// 몬스터 스텟 확장값
	sMonsterStatus		mStatusPlus;		
	sMonsterStatus		mStatusPer;

	float				mMoveSpeed;	


	/*--------------- 상태 관련 ---------------*/
	cAction*			mpAction;			/// 수행중인 액션의 포인터
	bool				mGroupRegen;		/// 그룹 단위 리젠 설정

	/// 추적시 공격범위에 들어온경우 공격형태
	eMONSTERATTACK_TYPE	attackType;

	/// 몬스터에서 사용할 행동
	cActionMove			mActionMove;
	cActionDie			mActionDie;
	cActionAttack		mActionAttack;
	cActionEscape		mActionEscape;
	cActionFollow		mActionFollow;
	cActionIdle			mActionIdle;
	cActionPushPull		mActionPushPull;

	unsigned long		mDestroyTime;
	unsigned long		mIdleEndTime;

	bool				mReservationComeback;

	/*--------------- 상태 이상 ---------------*/
	long				mODDITY[eODDITYTYPE_MAX];
	unsigned long		mTFixedPlayerIdx;	/// 타겟 고정 playeridx
	unsigned long		mTFixedInfIdx;		/// 타겟 고정 효과 번호

	/*--------------- 이동 관련 ---------------*/
	NiPoint2			mLastPos;			/// 이동 패스 최종 목적지
	NiPoint2			mNextPos;			/// 이동 패스 다음 노드 위치
	float				mMoveStopRange;		/// 목적지 전 정지 범위

	NiPoint2			mRegenPos;			/// 몬스터가 실제 리젠된 좌표

	NiPoint2			mFollowStartPos;	/// 추적 시작 위치
	unsigned long		mFollowEndTime;		/// 추적 종료 시간

	bool				mFollowEndMove;		/// 추적하다가 원래 이동 구역으로 이동중

	float				mMoveOverTime;		/// 경로 목적지까지 이동중 프로세스 간격에의해 넘어간 시간

	eMOVE_SPEED_TYPE	mMoveSpeedType;		/// 이동 방법

	bool				mIsFirstMove;		/// 처음 이동 인지 설정

	unsigned char		mPlayerFollowPos;

	float				mDirection;			/// 방향값

	long				mPatternPos;		/// 패턴 목적지
	char				mPatternDir;		/// 패턴 이동 방향( 1:정방향, -1:역방향)
	unsigned long		mPatternWait;		/// 패턴 이동후 대기시간
	char				mGroupMoveReady;	/// 0:이동중, 1:이동끝나고대기시간, 2:대기시간끝이동가능

	/// 클라이언트에 서버위치를 표시해주는 테스트코드의 메세지발송 다음시간
	unsigned long		mServerPosTime;

	/// 길찾기
	cPathFinder*		mPathFinder;
	NiPoint2			mPathArray[MAX_PATH_COUNT];
	unsigned int		mPathCount;
	unsigned int		mPathIndex;

	/*--------------- 대사 관련 ---------------*/
	unsigned long		mTalkDealyEndTime;
	unsigned char		mTalkState;
	unsigned long		mTalkProcessCheckTime;
	bool				mCriDie;

	// 오토더미 여부 (0:아님,1:오토더미)
	char				mAutoDummy;	

	/*--------------- 몬스터 모드 관련  ---------------*/
	bool mIsCombat;

	eMONSTERATTACK_TYPE	mModeSkillIdx;
	eMONSTERATTACK_TYPE	mModeInitSkillIdx;

	unsigned long mTotalTime;
	unsigned long mModeTime;

	cModeAgent* mModeAgent;
	cMode*		mCurrentMode;

	/// 부모 몬스터가 있는 경우 처리
	bool mIsParentLink;
	unsigned long mParentMonsterIdx;

	/// 모드별 소환된 몬스터 목록
	/// tHashMap<unsigned long modeIdx, cHashSet<monsterIdx>*>
	cPHashMap			mSummonListMap;
	bool				mDeathMonster;

	/// 소환한 채집물 목록
	cLongAry			mSummonGatherAry;

	/// GenMonster cooltime
	/// typedef tHashMap<unsigned long orderNumMon, unsigned long nextGenTime> cGenMonCoolTimeMap;
	cHashMap			mGenMonCoolTimeMap;

	/// GenTotem CoolTime
	/// typedef tHashMap<unsigned long orderNumTotem, unsigned long nextGenTime> cGenTotemCoolTimeMap;
	cHashMap			mGenTotemCoolTimeMap;

	/// GenGathering CoolTime
	/// typedef tHashMap<unsigned long orderNumTotem, unsigned long nextGenTime> cGenGatheringCoolTimeMap;
	cHashMap			mGenGatheringCoolTimeMap;

	// 오토더미 공격한 유저정보
	cHashMap			mAutoAttackerMap;

	/// PatternGen
	/// <sSummonPatternInfo*>
	//cPArray				mSummonPatternInfoAry;
	sSummonPatternRoot	mSummonPatternRoot;

	/// 스킬 사용조건 체크를 위한 보유중인 버프/디버프 맵. 키 값은 ApplyValueType(효과 적용 타입) 이다.
	//typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned short, cHashSet> cApplyValueTypeMap;
	cApplyValueTypeMap  mBuffmap;
	cApplyValueTypeMap  mDeBuffmap;

};

inline
void cMonster::SetPathFinder( cPathFinder* pathFinder )
{
	mPathFinder = pathFinder;
}

inline
unsigned int cMonster::GetPathCount() const
{
	int c = mPathCount - mPathIndex;

	if( c >= 0 )
	{
		return c;
	}
	else
	{
		assert( 0 );
		return 0;
	}
}

inline
const NiPoint2* cMonster::GetPathArray() const
{
	if( mPathIndex < mPathCount )
	{
		return &mPathArray[mPathIndex];
	}
	else
	{
		assert( 0 );
		return &mPathArray[0];
	}
}
