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

#include "GameSrvDefines.h"

#include "BaseObject.h"
#include "Player_Common.h"
#include "Npc_Common.h"
#include "Stage_Common.h"
#include "trade_common.h"
#include "Tarot_Common.h"
#include "Quest_Common.h"
#include "Chat_Common.h"
#include "Party_Common.h"
#include "RangeCheck.h"

#include "ItemBillPool.h"
#include "ItemGetPool.h"
#include "ItemCountPool.h"
#include "CooltimePool.h"
#include "ThemeUserPool.h"
#include "SQLGameStmt.h"

// PLAYER DBUPDATE FLAG
const int PLAYER_DBUPDATE_ERROR         = 0;
const int PLAYER_DBUPDATE_PROCESS       = 1;
const int PLAYER_DBUPDATE_LEVELUP       = 2;
const int PLAYER_DBUPDATE_LEVELCHEAT    = 3;
const int PLAYER_DBUPDATE_SKILLUP       = 4;
const int PLAYER_DBUPDATE_SKILLCHEAT    = 5;
const int PLAYER_DBUPDATE_SKILLADD      = 6;
const int PLAYER_DBUPDATE_SKILLRESET    = 7;
const int PLAYER_DBUPDATE_POINTPLUS     = 8;
const int PLAYER_DBUPDATE_LOGOUT        = 9;
const int PLAYER_DBUPDATE_EXPRECOVER    = 10;
const int PLAYER_DBUPDATE_LOGIN         = 11;


/// 추적 퍼짐위치 최대값/중심값
const int FOLLOW_POS_MAX = 9;
const int FOLLOW_POS_CENTER = 4;

/// 스킬 포인트 증가 수치
const unsigned char SKILL_LEVELUP_POINT = 2;

/// 일정시간 일괄저장 시간
const unsigned long BATCH_SAVE_TIME = 15 * MINUTE;

/// 맵 이동 랜덤 좌표 체크 간격
const unsigned long MAP_RANGE_CHECK_RING = 5;

/// pc방 안걸린버프 재시도 시간
const unsigned long INFLUENE_PCBANG_NEXT_ADDTIME = 3 * SECOND;
// 캐쉬 버프 주기적으로 저장시간
const unsigned long INFLUENCE_CASH_NEXT_SAVETIME = 10 * MINUTE;

/// GM이벤트 버프 재시도 시간
const unsigned long INFLUENCE_GMEVENT_NEXT_ADDTIME = 3 * SECOND;

struct MAKESKILL_SELECT;
struct MAKESKILL_INSERT;
struct sGMEventInfo;

enum eREQUEST_REJECTION
{
	eREQREJCT_NONE,			/// 요청 거절 상태 없음(요청수락)
	eREQREJCT_GAMEFINISH,	/// 게임 종료 상태
	eREQREJCT_DUELWAIT,		/// 결투 요청 대기 상태
	eREQREJCT_DUELPLAY,		/// 결투 상태
	eREQREJCT_EXCHANGE,		/// 거래 상태
	eREQREJCT_PARTY,		/// 파티 상태
	eREQREJCT_GUILD,		/// 길드 상태
	eREQREJCT_THEME,		/// 테마 상태
};

enum ePLAYER_PARTYSTATE
{
	ePARTY_NONE,		/// 파티 없음
	ePARTY_COMPLETE,	/// 파티중
	ePARTY_SENDJOIN,	/// 파티원 초대중
	ePARTY_RECVJOIN,	/// 파티초대 받는중
};

enum XCOPY_INVENTORY_OPT
{
	XCOPY_INVENTORY_ALL = 0,
	XCOPY_INVENTORY_COUNT,
	XCOPY_INVENTORY_SWAP,
};

// 방어구 세트 조합
#define ARMOR_SETS_NONE					0x0000
#define ARMOR_SETS_ROBES				0x0001
#define ARMOR_SETS_LIGHT				0x0002
#define ARMOR_SETS_HEAVY				0x0004

enum eCHANGESTATE_OPTION
{
	eCHANGESTATEOPTION_NONE = 0,
	eCHANGESTATEOPTION_RESURRECTION,
	eCHANGESTATEOPTION_RUSH,
};

// 판매아이템 구조체
struct StallSellItem
{
	eStallSellItemStatus status;	// 상태
	TB_INVENTORY*        inventory;	// 인벤토리 참조
	unsigned long        price;		// 가격(개당)
};

struct sGameOption
{
	union
	{
		struct
		{
			bool showGauge				: 1;	// 
			bool showBubble				: 1;	//
			bool showHeroNameCard		: 1;	//
			bool showHeroTotal			: 1;	//
			bool showPlayerNameCard		: 1;	//
			bool showPlayerTotal		: 1;	//
			bool showNPCNameCard		: 1;	//
			bool showMonsterNameCard	: 1;	//
			bool showHelmet				: 1;	//
			bool showTip				: 1;	//

			bool rejectionTrade			: 1;	//
			bool rejectionDuel			: 1;	//
			bool rejectionWhisper		: 1;	//
			bool rejectionParty			: 1;	//
			bool rejectionFriend		: 1;	//
			bool rejectionPvP			: 1;	//

			bool autoSendTwitterMsg		: 1;	//
		} option1;
		long optionData1;
	};
};

class cPlayer : public cBaseObject
{
public:
	cPlayer();
	~cPlayer();

	void* operator new    ( size_t n ); 
	void  operator delete ( void* ptr, size_t n );

	/// 각종 데이터 초기화
	bool				Init( unsigned long connectionIdx, unsigned long playerIdx, unsigned long money, unsigned long deposit );

	/// 플레이어 업데이트 프로세스
	void				Update( unsigned long elapsedTime, unsigned long accumTime );

	/// inline functions
	unsigned long		GetConnectionIdx ( ) { return mConnectionIdx; }

	sPlayerInfo*		GetPlayerInfo    ( ) { return &mPlayerInfo;        }
	sPlayerExrInfo*		GetExrInfo       ( ) { return &mPlayerExrInfo;     }
	sHeroInfo*			GetHeroInfo      ( ) { return &mHeroInfo;          }

	wchar_t*			GetUserID        ( ) { return NULL; }

	wchar_t*			GetName          ( ) { return mPlayerInfo.strName; }
	char				GetRace          ( ) { return mPlayerInfo.Race;    }
	char				GetGender        ( ) { return mPlayerInfo.Gender;  }
	eWEAPON_STATE		GetWeaponState   ( ) { return mPlayerWeaponState;  }

	void				SetName          ( wchar_t* name );
	void				SetRace          ( char race );
	void				SetGender        ( char gender );
    short               GetBagEnd        ( ) { return mHeroInfo.BagEnd;    }
    unsigned short      GetWearEnd       ( ) { return mHeroInfo.WearEnd;   }

    void				SetStaticFaceIndex( unsigned long faceIdx );

	/// 070607 PKH 플레이어 생존 유무 셋팅
	bool				GetStateDie( )	{ return mPlayerExrInfo.mState == eOBJECT_STATE_DIE; }

	/// 070115 PKH 직업
	ePLAYER_JOB			GetJob() { return mPlayerInfo.Job; }
	void				SetJob(ePLAYER_JOB job, unsigned long* skillList, unsigned char skillListCnt );

	/// 경험치 습득 및 레벨업
	void				AddExp( unsigned long addExp, bool addOption, bool expRecover );
	void				AddSxp( unsigned long addSxp, bool addOption );
	void				AddPartyExp( unsigned long addExp, bool addOption, bool expRecover, unsigned long addBaseExp, unsigned long addPartyExp );
	void				AddPartySxp( unsigned long addSxp, bool addOption, unsigned long addBaseSxp, unsigned long addPartySxp );

	unsigned char		GetLevel( ) { return mPlayerInfo.Level; }
	bool				LevelUp( unsigned char addLevel, unsigned long totalExp, bool isCheat ); 
	unsigned char		GetSkillLevel() { return mHeroInfo.SkillLevel; }
	bool				SkillLevelUp( unsigned char addLevel, unsigned long totalSxp, bool isCheat );

	/// 070807 PKH HP초기 셋팅
	void				InitHP( unsigned long HP );
	void				InitMP( unsigned long MP );

	/// 070807 PKH HP/MP값을 알려줌
	unsigned long		GetHP()	{ return mPlayerExrInfo.RestHP; }
	unsigned long		GetMP()	{ return mPlayerExrInfo.RestMP; }

	/// 070807 PKH HP감소
	unsigned long		HPDamage( sObject attacker, unsigned long damage, bool *die, bool msgSend, bool cri = false );
	void				HPUse( unsigned long useValue, bool msgSend );

	/// 070807 PKH 순식간에 HP증가
	void				HPHeal( unsigned long hp, bool msgSend, sObject healer, long distressPoint, eTAKEDAMAGE_TYPE type );
	bool                SendHPHeal( );
	void				SendMaxHP();

	/// 070807 PKH mp처리
	void				MPDamage( unsigned long mp, bool msgSend );
	void				MPUse( unsigned long useValue, bool msgSend );

	void				MPHeal( unsigned long mp, bool msgSend );
	bool                SendMPHeal( );
	void				SendMaxMP();

	inline unsigned int	GetRaceGender(){return (mPlayerInfo.Race*eGENDER_MAX + mPlayerInfo.Gender);}

	void				SetMoveReq( char protocol, NiPoint2 startPos, NiPoint2 destPos, unsigned long clientTime,
		float range, sObject targetInfo, NiPoint2 targetPos );

	/// 061208 PKH 플레이어 이동 목적지 설정
	void				SetMoveTargetPos( float x, float y );
	float				GetMoveTargetPosX( ) { return mGotoX; }
	float				GetMoveTargetPosY( ) { return mGotoY; }

	char				IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos );
	bool				IsClientMovePossible( NiPoint2 clientPos, NiPoint2 goalPos );
	bool				IsPushPullPossible( NiPoint2 startPos, NiPoint2 goalPos );

	void				SetRangeTarget( float tPosX, float tPosY, sObject& tInfo, float range );
	void				InitRangeTarget()	{ mRange = 0.0f; }

	void				SetClientMoveReq( NiPoint2 targetPos ,NiPoint2 clientPos, bool key );
	void				CalcClientMoveDeff( unsigned long endTime );

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				GetFixedObjectSize()	{ return mFixedObjectSize; }
	void				CalcFixedObjectSize( short sizePer );
	unsigned short		GetFixedObjectSizePer()	{ return mPlayerExrInfo.mFixedObjectSizePer; }

	/// 몬스터 외형 변신
	unsigned long		GetChgMonsterIdx()	{ return mPlayerExrInfo.mChgMonsterIdx; }
	void				SetChgMonster( unsigned long infObjectIdx, unsigned long monsterIdx );
	void				EndChgMonster();
	bool				MonsterPrecedeAttackPass();	/// 몬스터 선공검사시 통과 체크

	/// 세력 친밀도/포인트
	void				SetForceType( unsigned char forceType, bool msgSend = true );
	unsigned char		GetForceType()	{ return mPlayerExrInfo.mForceType; }
	unsigned long		FirePointPlus( unsigned long firePoint, bool msgSend = true );
	unsigned long		FireFriendlyPlus( unsigned long fireFriendly, bool msgSend = true );
	unsigned long		WaterPointPlus( unsigned long waterPoint, bool msgSend = true );
	unsigned long		WaterFriendlyPlus( unsigned long waterFriendly, bool msgSend = true );
	unsigned long		WindPointPlus( unsigned long windPoint, bool msgSend = true );
	unsigned long		WindFriendlyPlus( unsigned long windFriendly, bool msgSend = true );
	unsigned long		EarthPointPlus( unsigned long earthPoint, bool msgSend = true );
	unsigned long		EarthFriendlyPlus( unsigned long earthFriendly, bool msgSend = true );

	void				ForcePointDBUpdate();

public:

	/// 스텟
	void				InitStatus2()	{ memset( &mStatus2, 0 , sizeof(mStatus2) ); }
	const sStatus1*		GetStatus1()	{ return &mStatus1; }
	void				SetStatus1( ePLAYER_STATUS statusBase, float value );
	const sStatus1*		GetStatus1Plus()	{ return &mStatus1Plus; }
	void				SetStatus1Plus( ePLAYER_STATUS statusBasePlus, float value );
	const sStatus1*		GetStatus1Per()		{ return &mStatus1Per; }
	void				SetStatus1Per( ePLAYER_STATUS statusBasePer, float value );
	float*				GetPointerStatus2Plus()	{ return mStatus2Plus; }
	float*				GetPointerStatus2Per()	{ return mStatus2Per; }
	float				GetStatus2Plus( ePLAYER_STATUS_EXT extStatus );
	float				GetStatus2Per( ePLAYER_STATUS_EXT extStatus );
	const sStatus2*		GetStatus2()	{ return &mStatus2; }
	void				SetStatus2( ePLAYER_STATUS_EXT extStatus, float value );
	const sStatusEtc*	GetStatusEtc()	{ return &mStatusEtc; }
	void				SetStatusEtc( ePLAYER_STATUS_EXT_ADD statusAdd, float value );

	void				SetOriHPMax( unsigned long maxHP ) { mOriMaxHP = maxHP; }
	unsigned long		GetOriHPMax() { return mOriMaxHP; }
	void				SetOriMPMax( unsigned long maxMP ) { mOriMaxMP = maxMP; }
	unsigned long		GetOriMPMax() { return mOriMaxMP; }

	/// MP 소모량 증가/감소 효과
	unsigned long		CalcUseMPExt( unsigned long originUseMP );

	/// 캐스팅 시간 증가/감소 효과
	unsigned long		CalcCastingExt( unsigned long originCastingTime );

	//const sDamageCalc*	GetDamagecalc()	{ return &mDamageCalc; }
	//void				SetDamageCalc( ePLAYER_STATUS_DAMAGE statusDamage, unsigned long value );

	/// 순간 쓰고 사라지는 스킬효과(데미지) 계산 
	//void				CalcInstantSkill( unsigned long statusType1, unsigned long value1, unsigned long statusType2, unsigned long value2 );
	bool				AttackSuccess( sObject target, unsigned long attributeType, short accuracyValue, unsigned long instantSuccess, unsigned long instantSuccessPer );
	bool				CriticalSuccess( sObject target, unsigned long attributeType, unsigned short criticalValue, unsigned long instantCri, unsigned long instantCriPer );
	unsigned long		DamageCalc( sObject target, unsigned long skillIdx, unsigned long attributeType, unsigned long rangeType,
	unsigned long       skillDamage, unsigned long skillDamagePer,bool isIgnoreDefence, float addTotalDamage, float addPerTotalDamage );
	/// 다른 계산과 다르게 도트데미지는 공격자가없어도 계산을 해야되기때문에 기준객체가 타겟객체다!!
	unsigned long		DotDamageCalc( unsigned char attackerType, unsigned char attackerLv, unsigned long attackMin, 
	unsigned long       attackMax, unsigned char dotType, unsigned long skillDamage, 
	unsigned long       skillDamagePer, unsigned long totalTime, unsigned long dotTime );
	unsigned long		HealCalc( sObject target, long skillHealPlus, long skillHealPer, bool applyHP );
	unsigned long		DotHealCalc( sObject target, long infHealPlus, long infHealPer, unsigned long heal, unsigned long dotTotalTime, unsigned long dotTime );
	unsigned long		ConditionOdd( sObject target, long influenceRate, unsigned long skillClassIdx );
	

	unsigned long		GetMaxHP()	{ return mPlayerExrInfo.MaxHP; }
	unsigned long		GetMaxMP()	{ return mPlayerExrInfo.MaxMP; }

	unsigned short		GetMoveSpeed( );
	unsigned short		GetMoveUpdateSpeed( unsigned long accumTime );

	float				CalcStatusSkillRange( float skillRange, eRANGETYPE mRangeType, bool IsNormalAttack );

	/// 070215 PKH 장비 입출력
	sPlayerWeaponInfo*	GetWeaponInfo( ) { return &mPlayerWeaponInfo; }
	sPlayerWearInfo*	GetWearInfo  ( ) { return &mPlayerWearInfo;   }

	/// Player의 상태 정보
	unsigned int		GetState() { return mPlayerExrInfo.mState; }
	bool				IsChangeState( unsigned int newState, eCHANGESTATE_OPTION option = eCHANGESTATEOPTION_NONE );
	bool				ChangeState( unsigned int newState, eCHANGESTATE_OPTION option = eCHANGESTATEOPTION_NONE );
	/// 서버 셧다운시 상태 안바뀌는것들 강제로 바꿔버린다.
	void				ShutdownChangeState();

	/// 파티 상태
	void				SetPartyState( ePLAYER_PARTYSTATE state ) { mPartyState = state; }
	ePLAYER_PARTYSTATE	GetPartyState() { return mPartyState; }

	/// 평화/전투 상태
	void				SetStateIdle( unsigned char stateIdle )	{ mPlayerExrInfo.mStateIdle = stateIdle; }
	void				SetStateStop( unsigned char stateStop )	{ mPlayerExrInfo.mStateStop = stateStop; }

	unsigned char		GetStateStop() { return mPlayerExrInfo.mStateStop; }

	/// 070628 PKH 현재 대화중인 npcidx
	unsigned long		GetNpcIdx() { return mNpcIdx; }
	void				SetNpcIdx( unsigned long npcIdx ) { mNpcIdx = npcIdx; }

	/// 070702 PKH 플레이어 부활
	void				Resurrection( unsigned long resurrectionType );
	bool				SkillResurrection();	/// 스킬 부활 
	void				PvPResurrection( bool timeCheck = true );
	int					TutorialResurrection( ULONG_PTR socketContext );

	/// 적대 리스트에 포함한 몬스터
	void				AddTargetingMonster( unsigned long monsterIdx );
	void				DelTargetingMonster( unsigned long monsterIdx );
	void				ClearTargetingMonster();
	void				DietargetingMonster();
	
	void				GetTargetingMonster( tHashSet<unsigned long>& monsterSet );
	
	/// 플레이어가 힐을 받은 경우 몬스터에 공격 포인트 등록
	void				HealAttackToMonster( unsigned long healerIdx, unsigned long heal, long distressPoint, eTAKEDAMAGE_TYPE type );


	/// 효과 
	//bool				AddInfluence( unsigned long /*uniqueIdx*/, unsigned long /*influenceClassIdx*/ );
	bool				AddInfluence( cInfluenceObject* pInf );	
	tHashSet<unsigned long>* GetInfluenceSet() { return &mInfluenceSet; }
	void				DeleteInfluence( unsigned long influenceIdx );
	void				ReleaseInfluenceSet( bool logOut ); 
	void				DieDeleteInfluence(); 
	void				GameInInfApply();
	void				InfluenceProcessStart();
	void				InfluenceProcessStop();

	/// 종료시 효과 중복 저장을 막기위해 updatecomplete 이후에는 저장을 못하게 한다.
	void				SetInfNoSave()	{ mInfNoSave = true; }
	/// updatecomplete 진행중 이전 저장 명령이 있었는지 체크 - 끝날때까지 대기
	bool				IsCashInfSave()	{ return mIsCashInfSave == true || mIsCashInfCycleSave == true ? true : false; }
	/// 캐시 효과 시작 끝 시점에 저장 플래그
	void				SetCashInfSave( bool save )	{ mIsCashInfSave = save; }
	/// 캐시 효과 중간저장 플래그
	void				SetCashInfSaveCycle( bool save )	{ mIsCashInfCycleSave = save; }

	bool				IsParentEqual( unsigned long parentUniqueIdx );
	bool				IsInfluenceHave( unsigned long influenceIdx );

	void				SkillResurrectionHP( unsigned long HP );
	void				SkillResurrectionMP( unsigned long MP );

	void				AddActInfluence( unsigned long actIdx );
	void				ClearActInfluence();
	tHashMap<unsigned long, unsigned long>* GetActInfluenceMap() { return &mActInfluenceMap; }

	/// SkillPoint
	unsigned short		GetSkillPoint()	{ return mHeroInfo.SkillPointRemain; }
	unsigned short		GetSkillTotalPoint()	{ return mHeroInfo.SkillPointTotal; }
	unsigned short		GetJobUsedSkillPoint( unsigned char jobstep );
	bool				SkillPointMinus( unsigned short minusSP );
	void				SkillPointMinusRestore( unsigned short minusSP );
	bool				SkillPointPlus( unsigned short plusSP );

	/// 스킬 초기화
	int					SkillResetReq( unsigned short itemSlotNum );
	void				SetSkillReset( TB_CHARACTER_SKILL* pSkill, long cnt );
	void				SendSkillPoint();

	/// 테마 정보 초기화(캐시)
	int					ThemeResetReq( unsigned short itemSlotNum, unsigned short themeNum, unsigned char themeMode );

	/// 080619 KCJ 맵이동 요청.
	bool                IsMapChange ( ) { return mMapChange; }
	void                MapChange ( bool boolean ) { mMapChange = boolean; }
	bool				MapChange( unsigned long posIdx );
	bool				CheatMapChange( unsigned short mapNumber, float x, float y );
	bool				IsItemMapChange( unsigned long posIdx );
	bool				ItemMapChange( unsigned long posIdx, unsigned long useItemIdx );
	char				UserPortalMapChange( unsigned long userPortalIdx );
	bool				CashItemMapChange( unsigned short mapNumber, float x, float y, unsigned long useItemIdx );

	// 튜토리얼 정보 세팅
	bool				IsTutorialMode() { return (GetMapDataNumber() == MAP_TUTORIAL); }
	int					GetTutorialMode() { return mTutorialModeIndex; }
	void				SetTutorialMode( int modeIndex, bool send = true );
	
	bool				MapChangeByTutorial( unsigned long posIdx );			// 기본맵으로 돌아간다
	bool				GMMapChangeByTutorial( unsigned short mapNum, float x, float y );			// 기본맵으로 돌아간다

	void				SetMapNum0Save( bool save )	{ mMapNum0Save = save; }
	bool				IsMapNum0Save()	{ return mMapNum0Save; }

	//////////////////////////////////////////////////////////////////////////
	/// 인던(채널) 이동 관련
	bool				IsInDunJoin()	{ return mIsInDunJoin; }
	void				SetInDunJoin( bool inJunJoin )	{ mIsInDunJoin = inJunJoin; }
	unsigned short		GetMapDataNumber( ) { return (mIsInDunJoin == true) ? mInDunMapDataNum : mMapNumber; }
	unsigned short      GetRoomDataNumber( ) { return (mIsInDunJoin == true) ? mMapNumber : 0; }
	bool				InDunMapChange( unsigned short mapNumber, unsigned short mapDataNumber, float x, float y );
	void				ApplyReturnGameSrv()	{ mMapNumber = mGameSrvReturnMapNum; mObjectPos = mGameSrvReturnPos; }

	/// 테마 관련
	void				CalcThemeReturnGameSrv( unsigned short beforeMapNumber, float beforePosX, float beforePosY, bool isAuto );

	/// pvp 관련
	void				CalcPvPReturnGameSrv( unsigned short beforeMapNumber, float beforePosX, float beforePosY );
	void				ChannelMovePvPWait( long pvpGoCid, unsigned long waitTime );
	bool				CheckPvPJoin();
	void				PvPOut();
	void				AutoPvPOut();
	unsigned long		GetPvPDMIdx()	{ return mPvPDMIdx; }
	void				SetPvPDMTeam( unsigned char teamType )	{ mPlayerExrInfo.mPvPDMTeamType = teamType; }
	unsigned long		GetPvPDMTeam()	{ return mPlayerExrInfo.mPvPDMTeamType; }
	bool				SendPvPTeamChat( char* msg, int length );
	void				SetPvPJoinAryPos( unsigned long aryPos )	{ mPvPJoinAryPos = aryPos; }
	unsigned long		GetPvPJoinAryPos()	{ return mPvPJoinAryPos; }
	//////////////////////////////////////////////////////////////////////////

	///
	/// 071025 PKH 맵이동 위치.
	void                SetMapTargetPos( sTargetPos targetPos ) { mMapTargetPos = targetPos; }
	void                ApplyMapTargetPos( );

	/// 몬스터가 추적하는 비중 합
	bool				MonsterImportancePlus( unsigned char importance );
	void				MonsterImportanceMinus( unsigned char importance );
	long				GetMonsterImportance()	{ return mMonsterImportance; }

	void				MoveStop()	{ mGotoX = GetXPos(); mGotoY = GetYPos(); mRange = 0.0f; }
	void				MoveStopReq( MSG_REQ_MOVE_STOP* pmsg, char reqProtocol, char resProtocol );

	/// 스텟 변경값 클라이언트에 전송
	void				SendMoveSpeed();		/// 이동 속도
	void				SendAttackSpeed();		/// 공격 속도
	void				SendPlayerSize();		/// 캐릭터 크기

	void				SetGameIn();
	void				SetGameOut()	{ mGameIn = false; }
	bool				GetGameIn()     { return mGameIn; }

	bool                IsGameFinish();		/// updatecomplete에서 정상 종료 최종 체크

	void				CalcNatureRecovery();

	/// 캐릭터 처음 접속시 스텟 계산이후에 적용할 hp mp값
	void				SetInitRestHP();
	void				SetInitRestMP();
	void				ApplyInitRest();

	/// 이동오차 초기화
	void				ClearMoveSyncCalc()	{ mServerMoveTime = 0; mClientMoveDefTime = 0; mClientMovePlusMinus = 0; }

	/// 생활 스킬
	void				SetCommunitySkill( unsigned long communitySkillIdx);

	/// 몬스터가 플레이어를 추적하는 위치
	unsigned char		SelectFollowPos( unsigned char followPos );
	void				UnSelectFollowPos( unsigned char followPos );

	/// 2차상태
	bool				IsRequestRejection();
	unsigned long		GetRequestRejection()	{ return mIsRequestRejection; }
	void				StartRequestRejection( eREQUEST_REJECTION IsRequestRejection );
	void				EndRequestRejection( eREQUEST_REJECTION IsRequestRejection );

	//
	void				EndRequestRejectionAll();		// 전체 취소 처리
	
	/// 경험치 하락
	unsigned long		ExpDown( float minimum, float maximum );	

	/// 플레이어 정보 DB 저장
	bool				DBUpdate( bool updateComplete, int updateFlag );
	bool				IsDbUpdate()	{ return mIsDbUpdate; }
	void				PauseDBUpdate()	{ mIsDbUpdate = true; }
	void				EndDBUpdate()	{ mIsDbUpdate = false; }

	/// 몬스터 변신시 쿨타임 처리
	bool				IsChgMonCoolTimeEnd( eMONSTERATTACK_TYPE type );
	void				UpdateChgMonCoolTime( eMONSTERATTACK_TYPE type );
	void				SendCoolTimeChgMon();

	/*--------------- 결투 ---------------*/
	/// 결투 고유번호
	unsigned long		GetDuelIdx()	{ return mDuelIdx; }
	void				SetDuelIdx( unsigned long duelIdx );

	/// 결투 대상자 기록
	void				SetDuelPlayerIdx( unsigned long playerIdx, bool IsAttacker )	{ mDuelPlayerIdx = playerIdx; mIsDuelRequester = IsAttacker; }
	unsigned long		GetDuelPlayerIdx()	{ return mDuelPlayerIdx; }

	/// 본인이 나간경우 대상자와 결투 관련 처리
	void				DuelEnd();

	/// 연승 기록
	void				SetDuelWin()	{ ++mDuelStraightWin; }
	void				ClearDuelWin()	{ mDuelStraightWin = 0; }
	unsigned long		GetDuelWin()	{ return mDuelStraightWin; }

	/// 결투 종료후 결투에 사용된 디버프 제거
	void				DuelEndDebuffClear( unsigned long enemyIdx );

	/// pvp 종료후 디버프 제거
	void				PvPEndDebuffClear();

	/// 받은 데미지로 pvp 스코어 계산
	void				AddTakeDamage( sObject attacker, unsigned long damage, long distressPoint, eTAKEDAMAGE_TYPE type );

	/// 공격 가능 유무
	bool				IsAttack( unsigned long targetIdx );

	/*--------------- 상태 이상 ---------------*/
	void				SetStateOddity( long* pOddity );
	unsigned long		GetStateOddity( eODDITYTYPE oddityType )	{ return mODDITY[oddityType]; }
	
	bool				IsCantSkill( unsigned long attributeType );
	bool				IsCantMove()	{ return mODDITY[eODDITYTYPE_CANTMOVE] != 0 || mODDITY[eODDITYTYPE_SLEEP] != 0 || mODDITY[eODDITYTYPE_STUN] != 0; }

	/// 타겟 해제
	void				MonsterInTargeting( tArray<sObject>* targetAry, float range );

	/// 각 직업당 사용한 SP 저장
	void				UpdateJobUsedSP();

	void				StatusCalc();

	unsigned long		GetGatheringIdx()	{ return mGatheringIdx; }
	void				SetGatheringIdx( unsigned long gatheringIdx )	{ mGatheringIdx = gatheringIdx; }
	bool				CancelGathering( unsigned long gatheringIdx );

	/// 제조스킬
	unsigned char		AddMakeSkill( unsigned short itemSlotNum );
	void				SetMakeSkill( MAKESKILL_INSERT* pInsert );

	unsigned char		ReqDelMakeSkill( unsigned char makeSkill );
	void				DelMakeSkill( unsigned char makeSkillIdx );

	unsigned char		AddRecipe( unsigned short itemSlotNum );
	void				SetRecipe( unsigned char makeSkill, unsigned long recipeIdx );
	void				SetRecipeGroup( sRecipeGroupInfo* groupInfo, unsigned long cnt );

	bool				InitMakeSkill( MAKESKILL_SELECT* pMakeSkill );
	void				SendMsgRecipeList();
	bool				IsMakeSkill( unsigned char makeSkillIdx );


	int					ItemMixReq( unsigned char makeSkill, unsigned long mixRecipeIdx, unsigned short makeCnt );	/// 클라이언트에서 조합 요청 - 프로세스 진행여부 체크
	int					ItemMix( sMakeSkillScript* script );			                    /// 아이템 조합
	void				ItemMixEnd();
	void				ItemMixCancel();	                                                /// 조합 취소
	void				ItemMixForceCancel();	                                            /// 상태이상으로인한 강제 취소
	bool				SaveRecipeCoolTime( RECIPE_COOLTIME* skillCoolTime );
    void                MakeSkillExpPlus( sMakeSkill* pMakeSkill, sMakeSkillScript* Recipe );

	/// 탈것
	void				SetVehicle( unsigned long infObjectIdx, unsigned long vehicleIdx );
	void				ArightVehicle( bool sendMsg, bool sendHero );
	bool				IsVehicle()	{ return mPlayerExrInfo.mVehicleIdx != 0 ? true : false; }

	/// 클라이언트에서 탈것 종료 요청온 처리
	void				EndVehicleClient( bool sendSync );
	/// 탈것 관련 아이템 상태 변경시 탈것 종료 처리
	void				EndVehicleItem( unsigned long skillClassIdx );

	void				SetSkillPushPull( unsigned short type, cBaseObject* pAttacker, unsigned long range );
	bool				IsSkillPushPull()	{ return GetState() == eOBJECT_STATE_PUSHPULL; } 
	NiPoint2			SetSkillRush( cBaseObject* pTarget, bool blink, unsigned long skillIdx, unsigned char skillStep );
	unsigned long		GetRushAfterSkillIdx()	{ return mCheckRushSkillIdx; }
	unsigned char		GetRushAfterSkillStep()	{ return mCheckRushSkillStep; }

	void				SetDoorPushPull( sDoorInfo* pInfo );

	/// 점프
	void				Jump();
	void				SetJumpTime();
	unsigned long		GetJumpEndTime()	{ return mJumpEndTime; }

	void				SetThemePartyDB( bool themePartyDb )	{ mThemePartyDb = themePartyDb; }
	bool				GetThemePartyDB()	{ return mThemePartyDb; }

	void				SetThemeRoomIdx( unsigned long roomIdx )	{ mThemeRoomIdx = roomIdx; }
	unsigned long		GetThemeRoomIdx()	{ return mThemeRoomIdx; }
	void				SetThemeModeIdx( unsigned char modeIdx )	{ mThemeMode = modeIdx; }
	unsigned long		GetThemeModeIdx()	{ return mThemeMode; }

	

	ThemeUserRoot*      GetThemeUserRoot ( ) { return &mThemeUserRoot; }

	PerThemeUser*       SearchThemeUser  ( long cid );
	bool                IsThemeUser      ( long cid );

	void                AddThemeUser     ( THEME_USER* table, long rowCount );
	bool                SaveThemeUser    ( long cid );

	void				SendThemeInfo    ( );
	void				SendThemeExpireTime( );

	/// 면역 효과 처리
	void				ClearImmune()	{ mImmuneBuffSet.Clear(); mImmuneDebuffSet.Clear(); mImmuneKindSet.Clear(); }
	void				AddImmuneApplyType( unsigned char InfluenceType, unsigned short applyType );
	void				AddImmuneKind( unsigned long infKind );
	bool				IsImmuneApplyType( unsigned char InfluenceType, unsigned long applyType );
	bool				IsImmuneApplyAll( unsigned long applyType );
	bool				IsImmuneKind( unsigned long infKind );

	/// 스킬사용 카운트 - 스피드핵
	void				SkillReqCntPlus()	{ ++mSkillReqCnt; }
	void				SkillSuccessCntPlus()	{ ++mSkillSuccessCnt; }
	void				SkillCoolTimeFailPlus()	{ ++mSkillCoolTimeFail; }
	void				MonSkillReqCntPlus()	{ ++mMonSkillReqCnt; }
	void				MonSkillSuccessCntPlus()	{ ++mMonSkillSuccessCnt; }
	void				MonSkillCoolTimeFailPlus()	{ ++mMonSkillCoolTimeFail; }

	void				SetUserPortal( unsigned long userPortalIdx );

	void				SetSkillLastAttack( sObject attacker, unsigned char kind )	{ mSkillLastAttack = attacker; mSkillLastKind = kind; }
	sObject				GetSkillLastAttack()	{ return mSkillLastAttack; }
	unsigned char		GetSkillLastKind()	{ return (unsigned char)mSkillLastKind; }

	void				CalcDirection( float targetX, float targetY );
	float				GetDirection()	{ return mDirection; }

	unsigned long		CalcChangeMPDamage( unsigned char attributeType, unsigned long damage, bool applyHP );
	unsigned long		CalcGuard( unsigned char attributeType, unsigned long damage );
	bool				CalcCashExpGuard( unsigned long* pInfIdx, unsigned char* leftCnt );

	bool				GetVehicleAttack()	{ return mVehicleAttack; }
	void				SetExpRecoverEndTime( TIMESTAMP_STRUCT endTime )	{ mExpRecoverEndTime = endTime; }

protected:
	sPlayerInfo			mPlayerInfo;
	sPlayerExrInfo		mPlayerExrInfo;
    
	sPlayerWeaponInfo	mPlayerWeaponInfo;
	sPlayerWearInfo		mPlayerWearInfo;

	sHeroInfo			mHeroInfo;

	typedef tArray<void*> cAry;
	typedef tArray<unsigned long> cLongAry;
	typedef tHashSet<unsigned long> cHashSet;

	/// 스텟1 기본값
	sStatus1			mStatus1;
	/// 스텟1 +값
	sStatus1			mStatus1Plus;
	sStatus1			mStatus1Per;

	/// 스텟2 +값
	float				mStatus2Plus[ePLAYER_STATUS_EXT_MAX];
	/// 스텟2 %값
	float				mStatus2Per[ePLAYER_STATUS_EXT_MAX];
	/// 스텟2 최종값
	sStatus2			mStatus2;
	
	/// 스텟외 스텟
	sStatusEtc			mStatusEtc;

	/// 데미지 계산용 순간효과 변수
	//sDamageCalc			mDamageCalc;

	//
	unsigned long		mOriMaxHP;

	/// mp % 소모시 계산될 수치
	unsigned long		mOriMaxMP;

	eWEAPON_STATE		mPlayerWeaponState;

	unsigned long		mConnectionIdx;	

	/// 061208 PKH 이동시 목적지 좌표
	float				mGotoX;
	float				mGotoY;

	/// 이동 명령이 들어왔을시 목적지가 못가는 위치면 먼저 클라이언트가 서있던 위치로 이동시키다가 목적지로
	/// 이동을 하기 시작하는데 그 목적지 위치 변수임
	NiPoint2			mReadyGotoPos;

	bool				mIsMoveMsg;
	sMoveMsg			mMoveMsg;

	///
	float				mRange;
	NiPoint2			mRangeTargetPos;
	sObject				mRangeTarget;

	ePLAYER_PARTYSTATE	mPartyState;
	bool				mIsGuildAddReq;		// 길드 초대를 하고 있는지 여부

	/// 070628 PKH 현재 대화중인 npcidx
	unsigned long		mNpcIdx;

	typedef tHashSet<unsigned long> cSkillInfluenceSet;
	cSkillInfluenceSet	mInfluenceSet;

	typedef tHashMap<unsigned long, unsigned long> cActInfluenceMap;
	cActInfluenceMap	mActInfluenceMap;

	typedef tArray<unsigned short> cJobUsedSpArr;
	cJobUsedSpArr	mJobUsedSpArr;

	typedef tHashSet<unsigned long> cTargetingMonsterSet;
	cTargetingMonsterSet mTargetingMonsterSet;

	typedef tHashSet<unsigned long> cHashSet;
	cHashSet			mImmuneKindSet;				/// 면역 효과종류 목록
	cHashSet			mImmuneBuffSet;				/// 면역 적용효과값 목록
	cHashSet			mImmuneDebuffSet;			/// 면역 적용효과값 목록

	/// 부활
	bool				mIsSkillResurrection;
	unsigned long		mSkillResurrectionHP;
	unsigned long		mSkillResurrectionMP;

	/// 080619 KCJ 맵이동 요청.
	bool                mMapChange;
	sTargetPos          mMapTargetPos;

	/// 처음 접속시 맵번호가 0인경우 저장한다.
	bool				mMapNum0Save;

	/// 인던 관련 맵이동 정보
	bool				mIsInDunJoin;
	unsigned short		mGameSrvReturnMapNum;			/// 인던에서 게임서버로 복귀시 위치
	NiPoint2			mGameSrvReturnPos;

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				mFixedObjectSize;

	/// 플레이어를 쫓아오는 몬스터의 비중 합계
	long				mMonsterImportance;

	/// 게임 안에 들어 갔는지 기록
	bool				mGameIn;

	/// 자연 회복 다음 적용 시간
	unsigned long		mNextRecoveryTime;

	/// 캐릭터 처음 접속시 스텟 계산이후에 적용할 hp mp값
	unsigned long		mInitRestHP;
	unsigned long		mInitRestMP;

	/// 생활 스킬 종료시간
	float				mCommunitySkillEndTime;

	/// 방향값
	float				mDirection;

	/// 추적 퍼짐위치 자리 선정값
	bool				mFollowPos[FOLLOW_POS_MAX];

	cRangeCheck			mRangeCheck;

	/// 요청을 받을수 없는 상태
	eREQUEST_REJECTION	mIsRequestRejection;
	unsigned long		mRequestCharacterIndex;	// 요청자 인덱스

	/// 결투 
	unsigned long		mDuelIdx;			/// 고유번호
	unsigned long		mDuelPlayerIdx;		/// 결투 대상자
	bool				mIsDuelRequester;	/// 본인이 결투 요청자인지 구분
	unsigned long		mDuelStraightWin;	/// 결투 연승 카운트

	unsigned long		mBatchSaveTime;

	long				mODDITY[eODDITYTYPE_MAX];

	eCHEAT_SPEEDUP		mCheatSpeedUp;

	unsigned long		mChgMonCoolTime[eMONSTERATTACK_MAX];

	unsigned long		mChannelPvPMoveTime;
	long				mChannelPvPGoCid;
	unsigned long		mPvPDMIdx;
	unsigned long		mPvPDieTime;
	unsigned long		mPvPDieEndTime;
	unsigned short		mInDunMapDataNum;
	unsigned long		mPvPJoinAryPos;

	unsigned long		mGatheringIdx;

	/// 제조 스킬 & 하위 레시피
	typedef tHashMap<unsigned long, unsigned long> cRecipeGroup;
	cRecipeGroup		mRecipeGroup1;		/// 제조스킬의 레시피그룹 보유목록
	cRecipeGroup		mRecipeGroup2;		

	/// 사용 중인 조합 기술
	unsigned long		mItemMixEndTime;	/// 조합 완료 시간
	unsigned char		mMixMakeSkill;
	unsigned long		mMixRecipeIdx;
	unsigned long		mMixCnt;

	// 아이템 강화
	unsigned long		mItemEnhanceEndTime;	/// 강화 완료 시간
	unsigned short		mEnhanceSlot1;          /// 강화 아이템
	unsigned short		mEnhanceSlot2;          /// 강화 카드
	unsigned short		mEnhanceSlot3;          /// 강화 보호 카드

	unsigned long		mChgMonsterInfIdx;	/// 몬스터 변신 효과 고유 번호
	unsigned long		mVehicleInfIdx;		/// 탈것 효과 고유번호

	/// 클라이언트에 서버위치를 표시해주는 테스트코드의 메세지발송 다음시간
	unsigned long		mServerPosTime;

	/// 이동좌표 오차율계산
	unsigned long		mServerMoveTime;
	long				mClientMoveDefTime;
	char				mClientMovePlusMinus;

	bool				mKeyboardMove;

	unsigned long		mJumpEndTime;
	unsigned long		mJumpTime;
	unsigned long		mJumpMoveSpeedDownEndTime;
	unsigned long		mJumpSpeedState;

	bool				mThemePartyDb;
	unsigned long		mThemeRoomIdx;
	char				mThemeMode;
	ThemeUserRoot       mThemeUserRoot;

	// 튜토리얼 정보 
	int					mTutorialModeIndex;

	/// 스킬 사용 성공 실패 횟수 - 스피드핵 카운트
	unsigned long		mGameInTime;
	unsigned long		mSkillReqCnt;
	unsigned long		mSkillSuccessCnt;
	unsigned long		mSkillCoolTimeFail;
	unsigned long		mMonSkillReqCnt;
	unsigned long		mMonSkillSuccessCnt;
	unsigned long		mMonSkillCoolTimeFail;

	unsigned long		mUserPortalIdx;

	sObject				mSkillLastAttack;
	unsigned char		mSkillLastKind;		/// eDAMAGE_KIND

	/// <unsigned long influenceIdx>
	cLongAry			mGuardAry;
	/// <unsigned long influenceIdx>
	cLongAry			mChangeMPDamageAry;
	/// <unsigned long influenceIdx>
	cLongAry			mDeleteTypeDamagedAry;

	bool				mInfNoSave;

	bool				mVehicleAttack;

	/// dbupdate 호출전 쌓여있는 경험치량
	unsigned long		mWaitSaveExp;

	/// 손실된경험치 복구 가능시간
	TIMESTAMP_STRUCT	mExpRecoverEndTime;

	unsigned long		mPcBangNextCheckTime;
	/// pc방 버프중 걸려야 하는데 못걸리고 대기중인 효과
	cHashSet			mReadyPcInfSet;

	/// GM이벤트 버프중 걸려야 하는데 못걸리고 대기중인 효과
	cGMEventList		mReadyGMEventList;
	unsigned long		mGMEventNextCheckTime;

	/// 삭제 대기중인 GM 이벤트 버프
	cGMEventList		mReadyDelGMEventList;
	unsigned long		mGMEventDelNextCheckTime;

	unsigned long		mCashInfNextSaveTime;
	bool				mIsCashInfSave;
	bool				mIsCashInfCycleSave;

	unsigned long		mCheckRushSkillIdx;
	unsigned char		mCheckRushSkillStep;


	/// 스킬 사용조건 체크를 위한 보유중인 버프/디버프 맵. 키 값은 ApplyValueType(효과 적용 타입) 이다.
	//typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned short, cHashSet> cApplyValueTypeMap;
	cApplyValueTypeMap  mBuffmap;
	cApplyValueTypeMap  mDeBuffmap;

	// 소지금액 & 예치금액 관련 변수정의.
private:
	unsigned long       mMoney;
	unsigned long       mDeposit;

	// 소지금액 & 예치금액  관련 메소드.
public:
	unsigned long       GetMoney                 ( ) { return mMoney; }
	bool                IsAddMoney               ( long money );
	long                AddMoney                 ( sObject object, long money, bool save=true );
	bool                SaveMoney                ( sObject object, unsigned long befor, unsigned long after, long money );
	bool                SendMoney                ( sObject object, long addMoney=0 );

	unsigned long       GetDeposit               ( ) { return mDeposit; }
	long                AddDeposit               ( sObject object, long deposit );
	bool                SaveDeposit              ( sObject object, unsigned long befor, unsigned long after, long deposit );
	bool                SendDeposit              ( sObject object, long addDeposit=0 );

	void                NpcDeposit               ( MSG_REQ_NPC_DEPOSIT* msg );
	void                NpcWithdraw              ( MSG_REQ_NPC_WITHDRAW* msg );

	// 포인트 관련 메소드.
public:
	unsigned long       GetTarotPoint            ( ) { return mHeroInfo.TarotPoint; }
	long                AddTarotPoint            ( long point );
	bool                SaveTarotPoint           ( long point );
	bool                SendTarotPoint           ( long point );

	unsigned long       GetPvPPoint              ( unsigned char forceType );
	long                AddPvPPoint              ( long point, unsigned char forceType );
	bool                SendPvPPoint             ( long point, unsigned char forceType );

private:
	bool				mIsDbUpdate;		/// 캐릭터 기본정보 db저장중 플래그

	// 아이템 & 인벤토리 관련 변수정의.
private:
	union // Database Flag - Inventory
	{
		struct
		{
			bool insert               : 1; // 생성.(운영자만 가능).
			bool remove               : 1; // 삭제.
			bool use                  : 1; // 사용.
			bool swap                 : 1; // 이동중 교환(예외적인 이동).
			bool move                 : 1; // 이동.
			bool except               : 1; // 이동중 이동(예외적인 이동).
			bool merge                : 1; // 이동중 병합(예외적인 이동).
			bool divide               : 1; // 나누기.
			bool move2                : 1; // 이동(인벤<>창고 전용).
			bool enhanced             : 1; // 강화.
			bool disjoint             : 1; // 분해.
			bool putCard              : 1; // 카드장착
			bool change               : 1; // 체인지(교환).
			bool sell                 : 1; // 판매.
			bool buy                  : 1; // 구매.
			bool get                  : 1; // 줍기.
			bool reward               : 1; // 보상.
			bool take				  : 1; // 회수.
			bool collect              : 1; // 컬렉션북.
			bool exchange             : 1; // 거래.
			bool stallSellGet         : 1; // 구매(노점).
			bool itemMix              : 1; // 아이템조합.
			bool skillReset           : 1; // 스킬 초기화.
			bool agentInsert          : 1; // 아이템 판매대행(등록).
			bool agentDelete          : 1; // 아이템 판매대행(삭제).
			bool agentUpdate          : 1; // 아이템 판매대행(구매).
			bool postInsert           : 1; // 우편 보내기.
			bool postUpdate           : 1; // 우편 첨부받기.
			bool cash                 : 1; // 캐시아이템 물품수령
			bool seal                 : 1; // 인증
			bool itemBill             : 1; // 아이템 빌
			bool themeReset           : 1; // 테마 초기화
		} mInventoryDb;
		long mInventoryDatabase;
	};

	TB_INVENTORY        mInventory[ LEN_INVENTORY ];

	eItemActiveWeapon   mItemActiveWeapon;

	bool                mItemChange;
	unsigned short      mItemWeight;

	bool				mXCopyComplete;		// 아이템이 db 갱신 완료후 플래그

	sEquipAbility       mEquipAbility;
	unsigned short      mArmorSets;

	DWORD               mItemEnhancedSeed;
	DWORD               mItemChangeSeed;

	ItemBillRoot        mItemBillRoot;
	bool                mItemBillStart;

	ItemGetRoot         mItemGetRoot;
	ItemCountRoot       mItemCountRoot;
	CooltimeRoot        mCooltimeRoot;

	// 아이템 & 인벤토리 관련 메소드.
public:
	long                InventoryDatabase        ( ) { return mInventoryDatabase; }

	bool                IsInventory              ( TB_INVENTORY* inventory );
	bool                IsInventoryRange         ( unsigned short number );
	bool                IsInventoryBag           ( unsigned short number );
    bool                IsInventoryEquip         ( unsigned short number );
	bool                IsInventoryQuest         ( unsigned short number );
	bool                IsInventoryWarehouse     ( unsigned short number );
	bool                IsInventoryCollect       ( unsigned short number );
	bool                IsInventoryCash          ( unsigned short number );
	bool                IsInventoryMove          ( unsigned short number1, unsigned short number2, short* except, int& depth );
	bool                IsInventoryMerge         ( TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory1, TB_INVENTORY* inventory2 );
	bool                IsInventorySeal          ( TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory );
	bool                IsInventorySeal          ( TB_INVENTORY* inventory );
	bool                IsInventoryMegaphone     ( long inventoryIdx );

    bool                SetInvenLimit            ( TB_ITEM_DEFINE* itemDefine, CHARACTER_INVEN_SIZE* invenSize );
    short               GetEmptyBagNumber        ( short number=INVENTORY_BAG_BEGIN );
	short               GetEmptyQuestNumber      ( short number=INVENTORY_QUEST_BEGIN );

	void                InventorySwap            ( unsigned short number1, unsigned short number2 );
	void                InventoryMove            ( unsigned short number1, unsigned short number2 );
	void                InventoryExcept          ( unsigned short number1, unsigned short number2 );

	bool                IsItemLimit              ( long itemDefineIdx );
    bool                IsItemExchange           ( TB_ITEM_DEFINE* itemDefine, TB_INVENTORY* inventory );

    bool                IsMaxInventory           ( TB_ITEM_DEFINE* itemDefine, long count );
	bool                IsMaxInventory           ( sQuestItem* items, unsigned int rowCount );

    long                GetItemSumCount          ( long itemDefineIndex );
	short               GetItemCount             ( long itemDefineIndex );

	bool                IsItem                   ( long itemDefineIndex );
	bool                IsItemIdx                ( long inventoryIdx );
	bool                IsItemUse                ( long itemDefineIndex, unsigned short count=1 );
	bool                ItemUse                  ( long itemDefineIndex, unsigned short count=1 );
	long                IsItemMapChange          ( short number, long itemDefineIndex, bool isCash, long itemAbilityInfluence );
	long                IsItemVehicle            ( short number, long itemDefineIndex, long itemAbilityInfluence );
	bool                ItemUseMapChange         ( long inventoryIdx );
	void                ItemUseCashMapChange     ( long inventoryIdx );
	bool                SendItemUse              ( TB_INVENTORY* inventory );

	unsigned int        IsItemReward             ( long inventoryIdx, eDisjointItem* items, int maxCount );
	unsigned int        IsItemReward             ( sQuestItem* items, int maxCount, short slotBegin, short slotEnd );
	unsigned int        IsItemTake               ( sQuestItem* items, int maxCount );
	unsigned int		GetQuestItemCount		 ( long itemDefineIndex, int maxCount );

	void                SetActiveWeapon          ( eItemActiveWeapon activeWeapone ) { mItemActiveWeapon = activeWeapone; }
	eItemActiveWeapon   GetActiveWeapon          ( ) { return mItemActiveWeapon; }

	void                EquipItems               ( );
	void                CalcItemWeight           ( );
	unsigned short      GetItemWeight            ( ) { return mItemWeight; }

	void                AddEquipAbility          ( short opt, short value );
	void                AddEquipAbility          ( TB_ITEM_ABILITY* itemAbility );
	void                AddEquipEnhanced         ( TB_ITEM_ENHANCED* itemEnhanced, BYTE enhanced );
	void                AddEquipCards            ( TB_INVENTORY* inventory );
	sEquipAbility*      GetEquipAbility          ( ) { return &mEquipAbility; }

	void                ResetArmorSets           ( );
	unsigned short      GetArmorSets             ( ) { return mArmorSets; }

	ItemBillRoot*       GetItemBillRoot          ( ) { return &mItemBillRoot; }
	ItemGetRoot*        GetItemGetRoot           ( ) { return &mItemGetRoot; }
	ItemCountRoot*      GetItemCountRoot         ( ) { return &mItemCountRoot; }

	CooltimeRoot*       GetCooltimeRoot          ( ) { return &mCooltimeRoot; }
	void                AddCooltime              ( TB_INVENTORY_COOLTIME* table );

	PerItemBill*        AddItemBill              ( TB_ITEM_BILL* table );
	bool                SaveItemBill             ( long itemBillIdx, long elapsed, long before, long after, long inventoryIdx );
	bool                UpdateItemBill           ( );
	void                StartItemBill            ( );
	void                StopItemBill             ( );

	bool                CheckInventory           ( long itemDefineIndex, int itemCount );

	TB_INVENTORY*       XCopyInventory           ( TB_INVENTORY* destination, TB_INVENTORY* source, XCOPY_INVENTORY_OPT opt=XCOPY_INVENTORY_ALL );
	TB_INVENTORY*       XCopyInventory           ( TB_INVENTORY* inventory, XCOPY_INVENTORY_OPT opt=XCOPY_INVENTORY_ALL );

	TB_INVENTORY*		SelectInventory          ( short number=MIN_INVENTORY );
	TB_INVENTORY*		SelectInventoryIdx       ( long idx, short begin, short end );

	bool                FindFirstInventory       ( TB_ITEM_DEFINE* itemDefine, short* number, short remainCount, short* pcvValue, bool overlap );
	bool                FindNextInventory        ( TB_ITEM_DEFINE* itemDefine, short* number, short remainCount, short* pcvValue, bool overlap );

	TB_INVENTORY*       CreateInventory          ( TB_ITEM_DEFINE* itemDefine, short number, short count, short* pcbValue );
	TB_INVENTORY*       CreateInventory          ( long itemDefineIndex, short number, short count, short* pcbValue );
	short               UpdateInventory          ( TB_INVENTORY* inventory, short count );
	short               UpdateInventory          ( short number, short count );
	bool                RemoveInventory          ( TB_INVENTORY* inventory );
	bool                RemoveInventory          ( short number );

	void                ItemInsInventory         ( ULONG_PTR socketContext, MSG_REQ_CHEAT_MAKE* msg );
	void                ItemInsInventory         ( ULONG_PTR socketContext, MSG_REQ_CHEAT_MAKE_ITEMS* msg );
	void                ItemMovInventory         ( ULONG_PTR socketContext, MSG_REQ_ITEM_MOV_INVENTORY* msg );
	void                ItemUseInventory         ( ULONG_PTR socketContext, MSG_REQ_ITEM_USE_INVENTORY* msg );
	void                ItemDelInventory         ( ULONG_PTR socketContext, MSG_REQ_ITEM_DEL_INVENTORY* msg );
	void                ItemSwiInventory         ( ULONG_PTR socketContext, MSG_REQ_ITEM_SWI_INVENTORY* msg );
	void                ItemDivideInventory      ( ULONG_PTR socketContext, MSG_REQ_ITEM_DIVIDE_INVENTORY* msg );
	void                ItemMov2Inventory        ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_MOV2_INVENTORY* msg );
	void                ItemDisjointInventory    ( ULONG_PTR socketContext, MSG_REQ_ITEM_DISJOINT_INVENTORY* msg );
	void                ItemPutCardInventory     ( ULONG_PTR socketContext, MSG_REQ_ITEM_PUT_CARD_INVENTORY* msg );
	void                ItemChangeInventory      ( ULONG_PTR socketContext, MSG_REQ_ITEM_CHANGE_INVENTORY* msg );
	void                ItemCashInventory        ( ULONG_PTR socketContext );

	void                ItemSell                 ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_SELL* msg );
	void                ItemBuy                  ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_BUY*  msg );
	void                ItemCollect              ( ULONG_PTR socketContext, MSG_REQ_ITEM_COLLECT*  msg );
	void                ItemCollectChange        ( ULONG_PTR socketContext, MSG_REQ_COLLECT_CARD_CHANGE* msg );

	void				ItemObtEvent			 ( ULONG_PTR socketContext, BYTE cbtUser, BYTE obtUser, BYTE fourGamer );

	int                 IsItemEnhanced           ( unsigned short equip, unsigned short card, unsigned short safeCard, int* rateType  );
    void                ItemEnhancedStart        ( MSG_REQ_ITEM_ENHANCED_START* msg );
	void                ItemEnhancedEnd          ();
	void                ItemEnhancedCancel       ( );
	void                ItemEnhancedForceCancel  ( );		// 강제 취소( 상태이상등 )

    void                ItemLicense              ( ULONG_PTR socketContext, MSG_REQ_ITEM_LICENSE* msg );
    void                ItemSummonPost           ( ULONG_PTR socketContext, MSG_REQ_TRADE_POST_SUMMON_OPEN*     msg );
    void                ItemSummonAgent          ( ULONG_PTR socketContext, MSG_REQ_TRADE_AGENT_SUMMON_OPEN*    msg );
    void                ItemSummonWareHouse      ( ULONG_PTR socketContext, MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN* msg );

	void				ItemNpcEvent			 ( ULONG_PTR socketContext, unsigned long eventIndex );

	bool                SendInventory            ( ULONG_PTR socketContext );
    bool                SendInventoryCooltime    ( ULONG_PTR socketContext );
	bool                SendInventoryCash        ( ULONG_PTR socketContext );

	bool				SetWearInfoWithSendMsg   ( eWEAR_TYPE    type, unsigned long itemDefineIndex );
	bool				SetCosWearInfoWithSendMsg( eWEAR_TYPE    type, unsigned long itemDefineIndex );
	void                SendCosWearInfo          ( eWEAR_TYPE    type, unsigned long itemDefineIndex );
	bool				SetCosAccInfoWithSendMsg ( eCOSTUME_LINK type, unsigned long itemDefineIndex );
	void				SetWeaponInfoWithSendMsg ( TB_INVENTORY* left, TB_INVENTORY* right );

    void                SetInventoryEquip        ( sInventory* sInventory, unsigned long* count );

	// Callback 관련 메소드 - Inventory & Item.
	bool                CbItemBillDel            ( ULONG_PTR socketContext, ITEM_BILL_REMOVE* remove, u_long requestResult );

	void                CbInventoryInsert        ( ULONG_PTR socketContext, INVENTORY_INSERT*   inventoryInsert,   u_long requestResult );
	void                CbInventoryRemove        ( ULONG_PTR socketContext, INVENTORY_REMOVE*   inventoryRemove,   u_long requestResult );
	void                CbInventoryUse           ( ULONG_PTR socketContext, INVENTORY_USE*      inventoryUse,      u_long requestResult );
	void                CbInventorySwap          ( ULONG_PTR socketContext, INVENTORY_SWAP*     inventorySwap,     u_long requestResult );
	void                CbInventoryMove          ( ULONG_PTR socketContext, INVENTORY_MOVE*     inventoryMove,     u_long requestResult );
	void                CbInventoryExcept        ( ULONG_PTR socketContext, INVENTORY_EXCEPT*   inventoryExcept,   u_long requestResult );
	void                CbInventoryMerge         ( ULONG_PTR socketContext, INVENTORY_MERGE*    inventoryMerge,    u_long requestResult );
	void                CbInventoryDivide        ( ULONG_PTR socketContext, INVENTORY_DIVIDE*   inventoryDivide,   u_long requestResult );
	void                CbInventoryMove2         ( ULONG_PTR socketContext, INVENTORY_MOVE2*    inventoryMove2,    u_long requestResult );
	void                CbInventoryEnhanced      ( ULONG_PTR socketContext, INVENTORY_ENHANCED* inventoryEnhanced, u_long requestResult );
	void                CbInventoryDisjoint      ( ULONG_PTR socketContext, INVENTORY_DISJOINT* inventoryDisjoint, u_long requestResult );
	void                CbInventoryPutCard       ( ULONG_PTR socketContext, INVENTORY_PUT_CARD* inventoryPutCard,  u_long requestResult );
	void                CbInventoryChange        ( ULONG_PTR socketContext, INVENTORY_CHANGE*   inventoryChange,   u_long requestResult );
    void                CbInventorySeal          ( ULONG_PTR socketContext, INVENTORY_SEAL*     inventorySeal,     u_long requestResult );
    void                CbInventoryCashSelect    ( ULONG_PTR socketContext, INVENTORY_SELECT*   inventorySelect,   u_long requestResult );

	void                CbItemSell               ( ULONG_PTR socketContext, ITEM_SELL*           itemSell,      u_long requestResult );
	void                CbItemBuy                ( ULONG_PTR socketContext, ITEM_BUY*            itemBuy,       u_long requestResult );
	void                CbItemCollect            ( ULONG_PTR socketContext, ITEM_COLLECT*        itemCollect,   u_long requestResult );
	void                CbItemCollectChange      ( ULONG_PTR socketContext, ITEM_COLLECT_CHANGE* itemCollect,   u_long requestResult );

	void                CbItemGet                ( ULONG_PTR socketContext, ITEM_GET*        itemGet,       u_long requestResult );
	void                CbItemGetQuest           ( ULONG_PTR socketContext, ITEM_GET*        itemGet,       u_long requestResult );
	void                CbItemPartyGive          ( ULONG_PTR socketContext, ITEM_PARTY_GIVE* itemPartyGive, u_long requestResult );
	void                CbItemUse                ( ULONG_PTR socketContext, ITEM_USE*        itemUse,       u_long requestResult );
	void                CbItemObtEvent           ( ULONG_PTR socketContext, ITEM_OBT_EVENT*  itemEvent,     u_long requestResult );

	void                CbExchangeEnd            ( TB_INVENTORY* table, unsigned long rowCount, u_long requestResult );
	void                CbStallSellGet           ( ULONG_PTR socketContext, STALL_SELL_GET* stallSellGet, u_long requestResult );
	void                CbCharacterSkillreset    ( ULONG_PTR socketContext, CHARACTER_SKILL_RESET* characterSkillReset, u_long requestResult );
	void                CbInventoryReward        ( );
	void                CbInventoryTake          ( );
	void				CbCharacterThemeReset    ( ULONG_PTR socketContext, CHARACTER_THEME_RESET* characterThemeReset, u_long requestResult );
	void				CbCharacterGMThemeReset  ( CHARACTER_GM_THEME_RESET* characterThemeReset );

	bool				CbInventoryItemMix       ( );
	bool				CbInventoryEnhanced      ( );

    void                CbItemSummonPost         ( ULONG_PTR socketContext, ITEM_SUMMON_POST*       summonPost,  u_long requestResult );
    void                CbItemSummonAgent        ( ULONG_PTR socketContext, ITEM_SUMMON_AGENT*      summonAgent, u_long requestResult );
    void                CbItemSummonWareHouse    ( ULONG_PTR socketContext, ITEM_SUMMON_WAREHOUSE*  summonWare,  u_long requestResult );


	void                CbItemNpcEvent			 ( ULONG_PTR socketContext, ITEM_NPC_EVENT* npcEvent,	u_long requestResult );

	// 아이템줍기 관련 변수정의.
private:
	bool                mItemGetOpen;
	unsigned long       mItemGetIdx;

	int  				mItemGetCount;
	sItemData			mItemGetData[ MAX_ITEMS ];

	// 아이템줍기 관련 메소드.
public:
	void                ItemGetOpen              ( MSG_REQ_ITEM_GET_OPEN* msg );
	void                ItemGetClose             ( MSG_REQ_ITEM_GET_CLOSE* msg );
	void                ItemGetClose             ( );
	void                ItemGet                  ( MSG_REQ_ITEM_GET* msg );
    int                 ItemPartyGive            ( MSG_REQ_ITEM_PARTY_GIVE* msg );
    int                 ItemPartyTake            ( TB_ITEM_DEFINE* takeItem, short count, unsigned long giveCharacterIdx );

	unsigned short      ItemGetAutoPush          ( unsigned long itemDefineIndex, unsigned short count );
	bool				ItemGetAutoEnd           ( bool apply );

	bool                ItemGetQuest             ( unsigned long itemDefineIndex, unsigned short count );

	// 아이템줍기 관련 메소드.
private:
	bool                SendItemGetOpen          ( cBaseObject* baseObject );
	bool                SendItemGetClose         ( );
	bool                SendItemGetLeave         ( cBaseObject* baseObject );
	bool                SendItemGatheringOpen    ( cBaseObject* baseObject );

	// 거래 관련 변수정의.
private:
	eItemExchangeStatus mExchangeStatus;
	sObject             mExchangeTarget;
	unsigned long       mExchangeItems;
	unsigned long       mExchangeMoney;

	// 거래 관련 메소드.
public:
	eItemExchangeStatus GetExchangeStatus        ( ) { return mExchangeStatus; }
	long                GetExchangeMoney         ( ) { return mExchangeMoney; }

	void                ExchangeAsk              ( MSG_REQ_ITEM_EXCHANGE_ASK* msg );
	void                ExchangeAsk              ( sObject object );
	void                ExchangeReply            ( MSG_REQ_ITEM_EXCHANGE_REP* msg );
	void                ExchangeReply            ( BYTE type );
	void                ExchangeReplySyn         ( BYTE type, int error );

	void                ExchangeAdd              ( MSG_REQ_ITEM_EXCHANGE_ADD* msg );
	void                ExchangeAdd              ( TB_INVENTORY* inventory );
	void                ExchangeAdd              ( unsigned long money );
	void                ExchangeDel              ( MSG_REQ_ITEM_EXCHANGE_DEL* msg );
	void                ExchangeDel              ( TB_INVENTORY* inventory );

	void                ExchangeOk               ( MSG_REQ_ITEM_EXCHANGE_OK* msg );
	void                ExchangeRetry            ( MSG_REQ_ITEM_EXCHANGE_RETRY* msg );
	void                ExchangeRetry            ( );

	void                ExchangeCancel           ( MSG_REQ_ITEM_EXCHANGE_CANCEL* msg );
	void                ExchangeCancel           ( bool sendme = true );

	void                ExchangeClear            ( );

	void                ExchangeEnd              ( ) { mInventoryDb.exchange = true; }
	void                ExchangeEnd              ( MSG_REQ_ITEM_EXCHANGE_END* msg );

	bool                SendMsgResExchange       ( char category, char protocol, int error );
	bool                SendMsgResExchangeEnd    ( TB_INVENTORY* table, unsigned long rowCount );

	// 노점 관련 변수정의.
private:
	// 노점 아이템(ITEM)
	bool                mStallSell;
	wchar_t             mStallSellTitle[ MAX_STALL_TITLE_LEN ];
	StallSellItem       mStallSellItems[ MAX_STALL_ITEM ];
	unsigned long       mStallSellItemOffset;

	unsigned long       mStallSellGuests[ MAX_STALL_GUEST ];
	unsigned long       mStallSellGuestOffset;

	// 노점 게스트(GUEST)
	sObject             mStallSellOwner;

	// 노점 관련 메소드.
public:
	bool                IsStallSellOpen          ( ) { return mStallSell; }
	void                StallSellOpen            ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_OPEN* msg );
	void                StallSellClose           ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_CLOSE* msg );

	void                StallSellOpenDie         ( );
	void                StallSellOpenShutdown    ( bool shutdown=false );

	void                StallSellUseDie          ( );
	void                StallSellUseShutdown     ( bool shutdown=false );

	StallSellItem*      GetStallSellItem         ( TB_INVENTORY* inventory, long price );
	StallSellItem*      GetStallSellItem         ( TB_INVENTORY* inventory );
	bool                ReleaseStallSellItem     ( StallSellItem* stallSellItem );
	void                ClearStallSellItem       ( );

	bool                GetStallSellGuest        ( unsigned long characterIdx );
	bool                ReleaseStallSellGuest    ( unsigned long characterIdx );
	void                ReleaseStallSellAllGuests( );

	void                StallSellRename          ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_RENAME* msg );

	void                StallSellAdd             ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_ADD* msg );
	void                StallSellDel             ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_DEL* msg );
	void                StallSellMod             ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_MOD* msg );

	void                StallSellJoin            ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_JOIN* msg );
	void                StallSellJoin            ( cPlayer* player );
	void                StallSellLeave           ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_LEAVE* msg );
	bool                StallSellLeave           ( cPlayer* player );
	void                StallSellLeave           ( int error );
	void                StallSellGet             ( ULONG_PTR socketContext, MSG_REQ_ITEM_STALL_SELL_GET* msg );

	bool                SendMsgResStallSell      ( char category, char protocol, int error );
	void                SendMsgSynStallSell      ( char category, char protocol, TB_INVENTORY* inventory, long price );

	// 판매대행 관련 변수정의.
public:
	bool                mItemAgentCheck;
	unsigned long       mItemAgentNextCheck;

	// 판매대행 관련 메소드.
public:
	void                SetItemAgentCheck   ( long nextCheck );

	bool                ItemAgentPageOpen   ( ULONG_PTR socketContext, long startPage );
	void                ItemAgentAdd        ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_ADD* msg );
	void                ItemAgentDel        ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_DEL* msg );
	void                ItemAgentGet        ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_GET* msg );

	void                CbItemAgentCheck    ( ITEM_AGENT_CHECK* check );
	void                CbItemAgentAdd      ( ULONG_PTR socketContext, ITEM_AGENT_INSERT* insert, u_long requestResult );
	void                CbItemAgentDel      ( ULONG_PTR socketContext, ITEM_AGENT_DELETE* del, u_long requestResult );
	void                CbItemAgentGet      ( ULONG_PTR socketContext, ITEM_AGENT_UPDATE* update, u_long requestResult );

	//bool                ItemAgentSearch     ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH* msg );
	//bool                ItemAgentSearchName ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_NAME* msg );
	bool                ItemAgentSearchType ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_TYPE* msg );
	bool                ItemAgentSearchAll  ( ULONG_PTR socketContext, MSG_REQ_NPC_ITEM_AGENT_SEARCH_ALL* msg );

	// 우편 관련 변수정의.
public:
	bool                mPostCheck;
	unsigned long       mPostNextCheck;

	// 우편 관련 메소드.
public:
	void                SetPostCheck  ( long nextCheck );

	bool				PostOpen	  ( ULONG_PTR socketContext, long startPage=1 );
	bool                PostOpen      ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_OPEN* msg );
	void                PostClose     ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_CLOSE* msg );

	void                PostRead      ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_READ* msg );
	void                PostWrite     ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_WRITE* msg );
	void                PostDownload  ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_DOWNLOAD* msg );
	void                PostDelete    ( ULONG_PTR socketContext, MSG_REQ_CHAT_POST_DELETE* msg );

    void                SendPostSelect( ULONG_PTR socketContext, long startPage, long endPage, long noneRead, POST_RESULT* postResult, long postCount );

	void                CbPostCheck    ( POST_CHECK* check );
	void                CbPostRead     ( POST_UPDATE_STATUS* status );
	void                CbPostWrite    ( ULONG_PTR socketContext, POST_INSERT* insert );
	void                CbPostDownload ( ULONG_PTR socketContext, POST_UPDATE* update );


	// 타로 관련 변수정의.
private:
	wchar_t             mTarotTitle[ MAX_TAROT_TITLE ];
	unsigned long       mTarotPrice;
	unsigned short      mTarotUserCount;
	short               mTarotItemCount;
	bool                mTarotCloseReserved;

	unsigned long       mTarotGuests[ MAX_TAROT_GUEST ];
	int					mTarotGuestOffset;

	unsigned long       mTarotReaderIdx;
	TB_INVENTORY*       mTarotCards[ MAX_TAROT_CARDS ];
	TB_INVENTORY*       mSpread[ MAX_SPREAD ];
	int                 mSpreadOffset;

	unsigned long       mTarotResultIndex;
	unsigned long       mTarotResultValue;

	// 임시 npc 타롯 저장소
	unsigned long		mTempNpcTarotCards[ MAX_TAROT_CARDS ];

	// 타로 관련 메소드 - 타로는 Reader와 Seeker로 구분.
public:
	void                SetTarotResultIndex      ( long index ) { mTarotResultIndex=index; }
	void                SetTarotResultValue      ( long value ) { mTarotResultValue=value; }

	wchar_t*            GetTarotTitle            ( ) { return mTarotTitle; }
	unsigned long       GetTarotPrice            ( ) { return mTarotPrice; }

	bool				IsTarotReaderClose       ( );
	bool                IsTarotCloseReserved     ( ) { return mTarotCloseReserved; }

	void				TarotReaderReady         ( ULONG_PTR socketContext, MSG_REQ_TAROT_READER_READY* msg );
	void				TarotReaderOpen          ( ULONG_PTR socketContext, MSG_REQ_TAROT_READER_OPEN* msg );
	void				TarotReaderClose         ( ULONG_PTR socketContext, MSG_REQ_TAROT_READER_CLOSE* msg );
	void				TarotReaderClose         ( int error );

	bool                GetTarotGuest            ( unsigned long characterIdx );
	bool                ReleaseTarotGuest        ( unsigned long characterIdx );
	void                ReleaseTarotAllGuests    ( );

	bool                GetTarotCards            ( TB_INVENTORY* inventory[], short begin );
	void                GetSpreads               ( TB_INVENTORY* inventory[], short begin, int& offset );
	short               GetTarotItemCount        ( ) { return mTarotItemCount; }
	void                UseTarotCard             ( sObject object, long money, long itemDefineIndex );

	void                TarotSeekerOpen          ( ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_OPEN* msg );
	void                TarotSeekerClose         ( ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_CLOSE* msg );
	void                TarotSeekerJoin          ( ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_JOIN* msg );
	void                TarotSeekerResult        ( ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_RESULT* msg );
	void                TarotSeekerAccept        ( ULONG_PTR socketContext, MSG_REQ_TAROT_SEEKER_ACCEPT* msg );

	void                TarotTutorialSeekerJoin  ();
	void                TarotTutorialSeekerResult( ULONG_PTR socketContext, MSG_REQ_TAROT_TUTORIAL_SEEKER_RESULT* msg );
	void                TarotTutorialSeekerAccept( ULONG_PTR socketContext );

	int                 TarotSelfJoin            ( ULONG_PTR socketContext, MSG_REQ_TAROT_SELF_JOIN* msg  );
	int                 TarotSelfResult          ( ULONG_PTR socketContext, MSG_REQ_TAROT_SELF_RESULT* msg  );
	int                 TarotSelfAccept          ( ULONG_PTR socketContext  );

	void                TarotReaderDie           ( );
	void                TarotSeekerDie           ( );
	void                TarotReaderShutdown      ( bool shutdown=false );
	void                TarotSeekerShutdown      ( bool shutdown=false );

	bool                SendMsgResTarot          ( char category, char protocol, int error );

	bool                SendTarotReaderJoin      ( wchar_t* seekerName );
	bool                SendTarotReaderLeave     ( wchar_t* seekerName );

	bool                SendTarotStoreOpen       ( wchar_t* readerName, wchar_t* tarotTitle, unsigned long price );
	bool                SendTarotStoreJoin       ( wchar_t* tarotTitle, unsigned long price );

	// 퀘스트 관련 데이터
private:
	///	개인 보유 퀘스트
	TB_QUEST_PROGRESS	mQuest[ MAX_KEEPQUEST ];

	/// 완료된 퀘스트 목록 ( 보상까지 )
	typedef	tHashSet<unsigned long> cCompleteQuestSet;
	cCompleteQuestSet	mCompleteQuestSet;

	/// 완료된 퀘스트 그룹 목록
	typedef	tHashSet<unsigned int> cCompleteGroupQuestSet;
	cCompleteGroupQuestSet	mCompleteGroupQuestSet;

	/// 만기일 퀘스트 목록
	typedef tHashMap<unsigned long, TIMESTAMP_STRUCT> cValidQuestMap;
	cValidQuestMap			mValidQuestMap;

	/// 만기일 퀘스트 그룹 목록
	typedef	tHashSet<unsigned int> cValidGroupQuestSet;
	cValidGroupQuestSet		mValidGroupQuestSet;

	// 퀘스트 관련 메소드
public:
	TB_QUEST_PROGRESS*  GetQuest				 ( int number );
	bool                SendQuest		         ( ULONG_PTR socketContext );
	bool				SaveQuestProgress		 ( ULONG_PTR socketContext );	/// DB 에 퀘스트 진행정보 갱신		
	bool				SendDefaultReward		 ( unsigned long questIdx, unsigned long* skillList, unsigned char skillListCnt );
	bool				SendRewardBuff			 ( unsigned long buffIdx );
	bool				SendLimitPenalty		 ( unsigned long questIdx, bool gameIn = false );	/// 제약사항 걸기
	bool				SendFailPenalty			 ( unsigned long questIdx );	/// 실패로 인한 패널티
	bool				SendEndPenalty			 ( unsigned long questIdx );	/// 퀘스트 종료시의 처리

	/// 퀘스트 진행 검사 ( targetIdx : 몬스터, npc 종류인덱스 )
	void				UpdateDutyHunt			 ( unsigned int classIdx, unsigned long playerIdx );	
	void				UpdateDutyItem			 ( );
	void				UpdateDutyActiveSkill	 ( unsigned int classIdx, unsigned long skillIndex );
	void				UpdateDutyPVP			 ( unsigned char pvptype, bool win );
	void				UpdateDutyTutorial		 ( eDutyType dutyType );
	
	bool				IsEquipQuestItem		 ( unsigned long questIdx );	/// 퀘스트진행 아이템을 착용하고 있는지 검사
	int					IsRegistQuest			 ( eQUESTADD_TYPE type, unsigned long questIdx, unsigned long npcIdx );	/// 신규가능한 퀘스트인지 검사
	int					IsUseQuestItem			 ( unsigned int number, unsigned long& questIdx );		/// 의뢰아이템을 사용할수 있는지 체크
	bool				QuestMapChange			 ( unsigned long posIdx );
	bool				IsCompleteGathering		 ( unsigned long questIndex );

	int					IsKeepQuest				 ( int arrIdx, unsigned long questIdx ); /// 보유중인 퀘스트인지 검사
	int					IsKeepQuest				 ( unsigned long questIdx );
	int					IsKeepGroupQuest		 ( unsigned long questIdx );				/// 해당 퀘스트와 동일한 그룹의 퀘스트 보유중 검사
	bool				IsCompleteQuest			 ( unsigned long questIdx, bool checkGroup = true );		/// 완전 완료한 퀘스트인지 검사
	bool				AddCompleteQuest		 ( unsigned long questIdx );	/// 완전히 완료한 퀘스트 목록
	int					FindEmptyQuest			 ( );
	void				ClearQuestList			 ( );
	void				ClearCompleteList		 ( );
	void				DeleteQuest				 ( int arrIdx );	
	unsigned int		GetDropItemCount		 ( unsigned int arrIdx, unsigned long itemIdx );	/// 몬스터 드랍에 필요한 아이템 갯수 리턴

	bool				AddValidQuest			 ( long questIdx, TIMESTAMP_STRUCT valid );
	bool				IsValidEndQuest			 ( long questIdx );				/// 만기일이 지난 퀘스트인지 검사(즉 또 받을수 있는지 검사)
	void				ClearValidQuestList		 ( );

	int					SaveQuestInsert			  ( ULONG_PTR socketContext, unsigned long characterIdx, unsigned long questIdx );
	int					SaveQuestInsertByItem	  ( ULONG_PTR socketContext, unsigned long characterIdx, unsigned long questIndex, unsigned short number );
	int					SaveQuestComplete		  ( ULONG_PTR socketContext, MSG_REQ_NPC_QUEST_COMPLETE* msg, unsigned long dbIdx );	
	int                 SaveQuestReward           ( ULONG_PTR socketContext, MSG_REQ_NPC_QUEST_REWARD* msg, unsigned long dbIdx );
	bool				SaveQuestDelete			  ( ULONG_PTR socketContext, MSG_REQ_QUEST_DEL* msg, unsigned long dbIdx );
	bool				SaveQuestDeleteAuto		  ( ULONG_PTR socketContext, MSG_REQ_QUEST_DELAUTO* msg, unsigned long dbIdx );
	
	bool				SendNewQuestList		  ( );
	bool				SendEventQuestList 		  ( short eventtype );			/// 일정 이벤트에 따른 신규 퀘스트


	// 커뮤니티 메소드 - 친구.
private:
	TB_FRIEND           mFriends[ MAX_FRIEND ];

	// 친구 DB 사용
	bool				mFriendUseDB;
	bool                mFriendSend;

public:
	void                ClearFriends             ( );
	TB_FRIEND*          SelectFriend             ( short offset=0 );
	bool                UpdateFriend             ( );
	void                SendFriendList           ( ULONG_PTR socketContext );
	void				SendFriendConnect		 ( BYTE connect );
	void				PostFriendConnect		 ( BYTE connect );
	void				SyncFriendConnect		 ( long characterIdx, char level, unsigned long job, unsigned short mapnum, BYTE connect, DWORD channelNum );

	void				SetFriendUseDB			 ( bool flag ) { mFriendUseDB = flag; }
	bool				GetFriendUseDB			 () { return mFriendUseDB; }

	void                SetFriendSend            ( bool enable ) { mFriendSend = enable; }
	bool                GetFriendSend            ( ) { return mFriendSend; }

	// 자동사냥 신고 관련 변수정의.
public:
    time_t              mTrialTime;                             // 자동사냥 신고 재사용 시간 
    time_t              mTrialRejectTime;                       // 자동사냥 신고 거부 시간

    bool                mAutoPlayer;                            // 오토 판정 여부
    bool                mAutoTrial;                             // 자동사냥 신고 상태

    unsigned long       mTrialAnswerTime;                       // 심판 시간
    unsigned char       mTrialAnswerCount;                      // 정답 입력 횟수
    unsigned char       mCaptchaCount;                          // 캡차 이미지 새로고침 횟수

    unsigned long       mTrialMemberIdx;                        // 신고자                    
    unsigned long       mLastAttackTime;                        // 마지막 몬스터 공격 시간
    wchar_t             mCaptchaName[MAX_TRIAL_ANSWER_LEN + 1]; // 심판 정답 - 캡차 이미지 이름

	// 자동사냥 신고 관련 메소드.
public:
    time_t              GetTrialTime()           { return mTrialTime;           }
    time_t              GetTrialRejectTime()     { return mTrialRejectTime;     }
    void                SetTrialTime             ( time_t trial  )   { mTrialTime = trial;          }
    void                SetTrialRejectTime       ( time_t reject )   { mTrialRejectTime = reject;   }

    unsigned long       GetTrialMemberIdx()      { return mTrialMemberIdx;      }
    bool                IsAutoPlayer()           { return mAutoPlayer;          }
    bool                IsAutoTrial()            { return mAutoTrial;           }
    bool                IsAutoTrial              ( bool bTrial )        { return mAutoTrial = bTrial;}
    void                SetCaptchaName           ( wchar_t* name )      { ::wcsncpy( mCaptchaName, name, MAX_TRIAL_ANSWER_LEN );    }
    

    void                CaptChaCountDown()       { --mCaptchaCount;             }
    unsigned char       GetCaptChaCount()        { return mCaptchaCount;        }
    unsigned char       GetTrialAnswerCount()    { return mTrialAnswerCount;    }

    unsigned long       GetLastAttackTime()      { return mLastAttackTime;      }
    unsigned long       SetLastAttackTime        ( unsigned long attack )   { return mLastAttackTime = attack;   }

    int                 TrialOpenReq             ( ULONG_PTR socketContext, BYTE slevel, cPlayer* pTarget );
    int                 TrialAnswer              ( ULONG_PTR socketContext, wchar_t* answer );
    void                TrialStart               ( unsigned long trialPlayerIdx, wchar_t* answer );
    void                TrialEnd                 (  bool bAuto=false );

    void                SendTrialReqResult       ( ULONG_PTR socketContext, int error, cPlayer* pTarget );

	// 오토판정시 제한 처리
	void				AutoLimit				 ( );

	// 네트워크 관련 메소드.
public:
	bool                SendPlayerInfo           ( char category, char protocol, ULONG_PTR socketContext, unsigned char inDunType );
	bool                SendSightIn              ( char category, char protocol, unsigned long connectionIdx );
	bool                SendSightOut             ( char category, char protocol, unsigned long connectionIdx );
	bool				SendBuff				 ( char protocol, unsigned long connectionIdx );

	bool				SendHeroInfo();

	/// 치트
protected:
	bool          mCheatHideMode;
	bool          mCheatUndeadMode;
	unsigned long mCheatChatDuration;
	unsigned long mCheatChatAccumTime;
	unsigned long mCheatStopDuration;

public:
	bool SetCheatHideMode   ( bool hide );
	bool GetCheatHideMode   ( ) const;

	void SetCheatUndeadMode ( bool undead );
	void CheatRecover       ( );
	void CheatRebirth       ( );

	void CheatStop          ( unsigned long duration );
	void CheatCancelStop    ( );

	void SetCheatSpeedUp    ( unsigned long speedUp );
	void CalcCheatSpeedUp   ( );

	/// 호칭관리
private:
	/// 플레이어가 소유한 호칭 맵
	typedef tPointerHashSet<unsigned long> cHaveTitleSet;
	cHaveTitleSet mHaveTitleSet;

	/// 호칭 셋팅으로 인해 적용된 효과의 고유 인덱스( 효과삭제를 위해 저장 )
	unsigned long mTitleUniqIndex;
	
public:
	bool          SendHaveTitleList  ( ULONG_PTR socketContext );	/// 보유 호칭 목록 send
	void          SendHaveTitle      ( unsigned long titleIdx );	/// 보유 호칭 send

	void          ClearHaveTitleList ( );							/// 보유 호칭 목록 초기화
	bool          AddHaveTitle       ( unsigned long titleIdx );	/// 보유 호칭 추가
	bool		  IsHaveTitle( unsigned long titleIdx );

	bool          InitTitle          ( );
	bool          ChangeTitle        ( unsigned long titleIdx );	/// 현재 호칭으로 설정

	unsigned long GetTitleIndex      ( ) { return mPlayerExrInfo.mTitleIndex; }
	unsigned long GetTitleInfIdx     ( ) { return mTitleUniqIndex; }

public:
	void          SetPartyIndex         ( unsigned long idx );
	unsigned long GetPartyIndex         ( ) { return mPlayerExrInfo.mPartyIndex; }
	void          GetPartyUserData      ( sPartyUserData* data );

	void          SendPartyDiceStart    ( );

	void          SendPartyAddoneSyn    ( sPartyUserData* userData );
	void          SendPartyOutSyn       ( unsigned long playerIdx );
	void          SendPartyThrowSyn     ( unsigned long playerIdx );

	void          SendPartyLeaderSyn    ( unsigned long leaderIdx );
	void          SendPartyDivideType   ( char type );
	void          SendPartyDataSyn      ( unsigned long connectionIdx );
	void          SendPartyDataLevelSyn ( unsigned long connectionIdx );
	void          SendPartyDataLevelSyn ( unsigned long playerIdx, unsigned char level );
	void          SendPartyMapchangeSyn ( unsigned long connectionIdx );
	void          SendPartyMapchangeSyn ( unsigned long playerIdx, short map, short room, DWORD channelnum );
	void          SendPartyDeleteSyn    ( );

public:
	void          SetPartyUnionIndex ( unsigned long idx );
	unsigned long GetPartyUnionIndex ( ) {return mPlayerExrInfo.mPartyUnionIndex; }

public:
	void          SetGuildAddReq            ( bool set ) { mIsGuildAddReq = set; }
	bool          IsGuildAddReq             ( ) { return mIsGuildAddReq; }		/// 길드 초대하고 있는지 여부

	bool          ChangeGuild               ( unsigned long guildIndex, char position );
	void          ChangeGuildPosition       ( char position );
	void          ChangeGuildMarkRegistDate ( );

	unsigned long GetGuildIndex()			{ return mPlayerExrInfo.mGuildIndex; }
	unsigned char GetGuildPosition()		{ return mPlayerExrInfo.mGuildPosition; }

	//
	void		  SendGuildConnectSyn		( unsigned long characterIdx, BYTE connect, DWORD channelNum );
	void		  GetGuildUserData			( sGuildConnect* data );

private:
	/// 옵션 값
	sGameOption  mOptionData1;

public:
	void         SetOptionData ( long option ) { mOptionData1.optionData1 = option; }
	sGameOption* GetOptionData ( ) { return &mOptionData1; }

private:
	/// NPC 점집 이용
	TIMESTAMP_STRUCT mFortuneThru[5];

public:
	TIMESTAMP_STRUCT* GetFortuneThru( int i );
	bool SaveFortuneData( ULONG_PTR socketContext );

	bool SendInitTodayWord( );
	bool ChangeTodayWord( ULONG_PTR socketContext, wchar_t* word, unsigned long color );

public:
	// PC 방 빌링 메소드 정의
	void Pg_Game_Start   ( char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain );
	void Pg_User_Alert   ( long Packet_Result, char* Bill_Method, long Bill_Remain );
	void Pg_Billing_Auth ( char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain );
	void Pg_Client_Move  ( char* User_Status );
	void Pg_Close        ( );

	/// 페이레터 빌링
	void BP_Start();
	void BP_End();
	void BP_ItemGive();

public:
	// GM 이벤트 메소드 정의
	void AddAllGMEvent	 ( );
	void AddGMEvent		 ( sGMEventInfo eventInfo );
	void DelGMEvent		 ( unsigned long DBIndex, unsigned long influenceIdx );

public:
	// 블록 관련 변수 정의
	bool   mBlockApply;
	time_t mBlockValidThru;

public:
	// 블록 관련 메소드 정의
	bool IsBlock  ( );
	bool SetBlock ( bool apply, unsigned long second );

public:
	// 캐릭터가 가지고 있는 buff/debuff 목록 체크
	bool IsHaveApplyBuffType( unsigned short applyValueType );
	bool IsHaveApplyDeBuffType( unsigned short applyValueType );

public:
	// 캐릭터 이벤트 관련 메소드.
	bool PostCharacterEvent ( char category, LPCTSTR format, ... );
};

inline void cPlayer::SetName( wchar_t* name ) 
{
	::wcsncpy( mPlayerInfo.strName, name, MAX_NAME_SIZE ); 
	mPlayerInfo.strName[MAX_NAME_SIZE] = 0;
}

inline void cPlayer::SetRace( char race )
{
	mPlayerInfo.Race = race;
}

inline void cPlayer::SetGender( char gender )
{
	mPlayerInfo.Gender = gender;
}

inline void	cPlayer::SetStaticFaceIndex( unsigned long faceIdx )
{
	mPlayerInfo.HeadInfo[eHEAD_FACE_STATIC] = faceIdx;
}

inline void cPlayer::SetRangeTarget( float tPosX, float tPosY, sObject& tInfo, float range )
{
	mRange = range;
	mRangeTargetPos = NiPoint2(tPosX, tPosY);
	mRangeTarget = tInfo;
}

inline bool cPlayer::MonsterImportancePlus( unsigned char importance )
{
	if( mMonsterImportance + importance > 100 )
	{
		//printf("\n 100 < importancePlus [%d]\n", mMonsterImportance + importance );
		return false;
	}

	//printf("\n mMonsterImportance[%d] + ", mMonsterImportance );
	mMonsterImportance = mMonsterImportance + importance;
	//printf("importance[%d] = MonsterImportancePlus[%d] \n", importance, mMonsterImportance );
	return true;
}

inline void cPlayer::MonsterImportanceMinus( unsigned char importance )
{
	if( mMonsterImportance - importance < 0 )
	{
		//printf("\n 0 > importancePlus [%d]\n", mMonsterImportance - importance );
		mMonsterImportance = 0;
		return;
	}

	//printf("\n mMonsterImportance[%d] - ", mMonsterImportance );
	mMonsterImportance = mMonsterImportance - importance;
	//printf("importance[%d] = MonsterImportancePlus[%d] \n", importance, mMonsterImportance);

}

inline void cPlayer::ApplyMapTargetPos( )
{
	mMapNumber = mMapTargetPos.mMapNumber;
	mGotoX = mObjectPos.x = mMapTargetPos.mPosX;
	mGotoY = mObjectPos.y = mMapTargetPos.mPosY;
	mDirection = mMapTargetPos.mRotAngle;
}

inline bool cPlayer::IsInventory(TB_INVENTORY* inventory)
{
	if ( inventory != NULL )
		return ((inventory->idx > 0) && (inventory->count > 0));
	return false;
}

inline bool cPlayer::IsInventoryRange(unsigned short number)
{
    if( number <= MAX_VALID_INVENTORY )
    {
        if( number > mHeroInfo.WearEnd  && number <= INVENTORY_WEAR_END )
            return false;
        if( number > mHeroInfo.BagEnd   && number <= INVENTORY_BAG_END )
            return false;
        if( number > mHeroInfo.WareHouseEnd && number <= INVENTORY_WAREHOUSE_END )
            return false;

        return true;
    }
    return false;
}

inline bool cPlayer::IsInventoryBag(unsigned short number)
{
    return !(number < INVENTORY_BAG_BEGIN || number > mHeroInfo.BagEnd);
}

inline bool cPlayer::IsInventoryEquip(unsigned short number)
{
    if( number <= INVENTORY_EQUIP_END)
    {
        if( number > mHeroInfo.WearEnd && number <= INVENTORY_WEAR_END )
            return false;

        return true;
    }
    return false;
}

inline bool cPlayer::IsInventoryQuest(unsigned short number)
{
	return !(number < INVENTORY_QUEST_BEGIN || number > INVENTORY_QUEST_END);
}

inline bool cPlayer::IsInventoryWarehouse(unsigned short number)
{
    return !(number < INVENTORY_WAREHOUSE_BEGIN || number > mHeroInfo.WareHouseEnd);
}

inline bool cPlayer::IsInventoryCollect(unsigned short number)
{
    return !(number < INVENTORY_COLLECTION_BEGIN || number > INVENTORY_COLLECTION_END);
}

inline bool cPlayer::IsInventoryCash(unsigned short number)
{
    return !(number < INVENTORY_CASH_ITEM_BEGIN || number > INVENTORY_CASH_ITEM_END);
}

inline TB_INVENTORY* cPlayer::SelectInventory(short number)
{
	return (number < MIN_INVENTORY || number > MAX_INVENTORY) ? (NULL) : (mInventory + number);
}

inline TIMESTAMP_STRUCT* cPlayer::GetFortuneThru( int i )
{
	return ( i >= 0 && i < 5 ) ? mFortuneThru + i : NULL;
}

/// 배열 인덱스로 가져오기
inline TB_QUEST_PROGRESS* cPlayer::GetQuest( int number )
{
	return (number < 0 || number > MAX_KEEPQUEST ) ? (NULL) : (mQuest + number);
}

inline void cPlayer::ClearQuestList()
{
	memset( mQuest, 0, sizeof(mQuest) );
}

inline void cPlayer::ClearCompleteList()
{
	mCompleteQuestSet.Clear();
	mCompleteGroupQuestSet.Clear();
}

inline void cPlayer::ClearValidQuestList()
{
	mValidQuestMap.Clear();
	mValidGroupQuestSet.Clear();
}

/// 치트
inline
bool cPlayer::GetCheatHideMode() const
{
	return mCheatHideMode;
}

inline
void cPlayer::SetCheatUndeadMode( bool mode )
{
	mCheatUndeadMode = mode;
}

//inline
//void cPlayer::SetCheatWatch( bool mode )
//{
//	mCheatWatch = mode;
//}

