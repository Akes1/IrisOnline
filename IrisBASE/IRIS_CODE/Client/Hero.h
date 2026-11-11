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

#include "Player.h"
#include "Skill_Common.h"
#include "Sphere.h"
//#include "PlayerSceneNode.h"

class cSkillExecuter;
class cMapPortalSceneNode;

// 방어구 세트 조합
//#define ARMOR_SETS_NONE					0x0000
//#define ARMOR_SETS_ROBES				0x001F
//#define ARMOR_SETS_LIGHT_ARMOR			0x03E0
//#define ARMOR_SETS_HEAVY_ARMOR			0x7C00

// 방어구 세트 조합
#define ARMOR_SETS_NONE					0x0000
#define ARMOR_SETS_ROBES				0x0001
#define ARMOR_SETS_LIGHT				0x0002
#define ARMOR_SETS_HEAVY				0x0004

//#define ROBES_HAT						0x0001
//#define ROBES_UPPER						0x0002
//#define ROBES_LOWER						0x0004
//#define ROBES_ONEPIECE					0x0006
//#define ROBES_HANDS						0x0008
//#define ROBES_FEET						0x0010
//
//#define LIGHT_ARMOR_HAT					0x0020
//#define LIGHT_ARMOR_UPPER				0x0040
//#define LIGHT_ARMOR_LOWER				0x0080
//#define LIGHT_ARMOR_ONEPIECE			0x00C0
//#define LIGHT_ARMOR_HANDS				0x0100
//#define LIGHT_ARMOR_FEET				0x0200
//
//#define HEAVY_ARMOR_HAT					0x0400
//#define HEAVY_ARMOR_UPPER				0x0800
//#define HEAVY_ARMOR_LOWER				0x1000
//#define HEAVY_ARMOR_ONEPIECE			0x1800
//#define HEAVY_ARMOR_HANDS				0x2000
//#define HEAVY_ARMOR_FEET				0x4000

enum eAfterAction
{
	eAfterAction_None = 0,

	/// 이동후
	eAfterAction_Attack,
	eAfterAction_ItemPick,
	eAfterAction_UseTargetSkill,
	eAfterAction_UseFieldSkill,
	eAfterAction_NpcTalk,

	eAfterAction_UseTarotStore,

	eAfterAction_UseStall,	/// 노점상이용
	eAfterAction_MapChange,

	eAfterAction_GatheringPick,

	/// 공격후
	eAfterAction_Move,
};

enum eStateInfo_Type
{
	eSTATE_NORMAL,	/// 보통 상태
	eSTATE_PLUS,	/// 증가효과
	eSTATE_MINUS,	/// 감소효과
	eSTATE_OVERLAPPLUS, /// 증가 중첩효과 
	eSTATE_OVERLAPMINUS, /// 감소 중첩효과 
};

struct sAfterAction
{
	eAfterAction	mActionType;
	NiPoint3		mDestPos;
	float			mRange;

	sObject			mTarget;

	/// type에 따라 저장 정보를 달리한다..
	unsigned long	mExData1;
	unsigned long	mExData2;
};

enum eWaitMove_Type
{
	eWAITMOVE_NONE = 0,
	eWAITMOVE_MOUSE,
	eWAITMOVE_KEY,
};

enum eNpcFunc;
class cNpc;

class cHero : public cPlayer
{
	friend	class cSkillExecuter;

public:
	cHero( unsigned char type = eOBJECTTYPE_HERO );
	virtual ~cHero();

	/// 객체 생성
	virtual bool	Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, sPlayerExrInfo* pExrInfo,
		NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );

	/// TODO : 객체 재생성
	virtual void	ReCreate( char race, char gender );

	/// 진입 시점에 등록
	bool			GameIn( unsigned long channelNum );

	///
	void			SetHeroInfoNet( MSGROOT* pInfo );

	virtual void	SetState( unsigned char state , bool bShowAni = true );

	virtual	void SitDown();
	virtual void StandUp();

	void ToggleIdleFlag();
	void ToggleSitDown();

	/// 신규 경로 탐색..
	bool			ResetMovePath( NiPoint2 start, NiPoint2 goal );
	bool			ResetActionMovePath( float destX, float destY, float range );

	/// 처리부
	void			Update( unsigned long deltaTime, unsigned long accumTime );
	void			AttackUpdate( unsigned long deltaTime, unsigned long accumTime ); 
	virtual void	MoveUpdate( unsigned long deltaTime );
	virtual void	IdleUpdate( unsigned long deltaTime );
	void			FollowMoveUpdate( unsigned long deltaTime );
	void			FollowIdleUpdate();

	/// 리스너를 부착
	void			AttachListener();

	NiPoint3			GetDirection() const;
	const NiMatrix3&	GetRotate() const;

	/// 경계 구
	const cSphere& GetBoundSphere() const;
	const NiPoint3& GetCenter() const;

	virtual void	SetTargetObject( unsigned char type, unsigned long idx );

	inline sHeroInfo*		GetHeroInfo() { return &mHeroInfo; }
	inline sPlayerExrInfo*	GetHeroExrInfo() { return &mPlayerExrInfo; }

	/// 스텟
	const sStatus1*		GetStatus1()	{ return &mStatus1; }
	void				SetStatus1( ePLAYER_STATUS statusBase, float value );
	//void				SetStatus1Passive( ePLAYER_STATUS statusBase, float value );
	float				GetStatusBaseTotal( ePLAYER_STATUS statusBase );
	float				GetStatusPlusTotal( ePLAYER_STATUS statusBase );

	const sStatus1*		GetStatus1Plus()	{ return &mStatus1Plus; }
	void				SetStatus1Plus( ePLAYER_STATUS statusBasePlus, float value );
	const sStatus1*		GetStatus1Per()	{ return &mStatus1Per; }
	void				SetStatus1Per( ePLAYER_STATUS statusBasePer, float value );
	
	const sStatus2*		GetStatus2()	{ return &mStatus2; }
	void				SetStatus2( ePLAYER_STATUS_EXT extStatus, float value );
	const sStatusEtc*	GetStatusEtc()	{ return &mStatusEtc; }
	void				SetStatusEtc( ePLAYER_STATUS_EXT_ADD statusAdd, float value  );
	
	void				SetOriMpMax( unsigned long maxMP ) { mOriMaxMP = maxMP; }
	unsigned long		GetOriMpMax() { return mOriMaxMP; }

	/// MP 소모량 증가/감소 효과
	unsigned long		CalcUseMPExt( unsigned long originUseMP );
	/// 캐스팅 시간 증가/감소 효과
	unsigned long		CalcCastingExt( unsigned long originCastingTime );

	float				CalcStatusSkillRange( float skillRange, eRANGETYPE mRangeType, bool IsNormalAttack );

	void			SetSkillLevel( unsigned char skillLevel ) { mHeroInfo.SkillLevel = skillLevel; }
	unsigned char	GetSkillLevel() { return mHeroInfo.SkillLevel; }
	void			SetSkillPointRemain( unsigned short sp, unsigned short total )	{ mHeroInfo.SkillPointRemain = sp; mHeroInfo.SkillPointTotal = total; }
	unsigned long	GetSkillPointRemain(){ return mHeroInfo.SkillPointRemain;}
	unsigned long	GetSkillPointTotal() { return mHeroInfo.SkillPointTotal; }
	unsigned long	GetUsedSkillPoint() { return mHeroInfo.SkillPointTotal - mHeroInfo.SkillPointRemain; }

	void			SetJobUsedSkillPoint( unsigned int count, unsigned short* values );
	unsigned short	GetJobUsedSkillPoint( unsigned int jobLevel );
	void			InitJobUsedSkillPoint();

	/// 070508 PKH 스텟 증감(색상) 타입
	void			InitStatusInfoType();
	void			SetBaseStatusInfoType( ePLAYER_STATUS pos, eStateInfo_Type type ) { mStatusBaseInfoType[pos] = static_cast<unsigned char>(type); }
	eStateInfo_Type	GetBaseStatusInfoType( ePLAYER_STATUS pos ) { return static_cast<eStateInfo_Type>(mStatusBaseInfoType[pos]); }
	void			SetExtStatusInfoType( ePLAYER_STATUS_EXT pos, eStateInfo_Type type ) { mStatusExtInfoType[pos] = static_cast<unsigned char>(type); }
	eStateInfo_Type	GetExtStatusInfoType( ePLAYER_STATUS_EXT pos ) { return static_cast<eStateInfo_Type>(mStatusExtInfoType[pos]); }


	void			SetExp( unsigned long exp )	{ mHeroInfo.Exp = exp; }
	void			SetSxp( unsigned long exp )	{ mHeroInfo.SkillExp = exp; }
	void			SetLoseExp( unsigned long exp )	{ mHeroInfo.mLoseExp = exp; }

	inline unsigned short	GetMapNum() { return mMapNum; }
	inline void				SetMapNum( unsigned short mapNum ) { mMapNum = mapNum; }
	inline unsigned char	GetMapMode() { return mMapMode; }
	inline void				SetMapMode( unsigned char mapMode ) { mMapMode = mapMode; }


	inline unsigned long	GetChannelNum() { return mChannelNum; }

	inline unsigned long	GetAreaNameIdx() { return mAreaNameIdx; }
	void					SetAreaNameIdx( unsigned long idx );

	/// hero는 max값을 받아 처리할 필요 없다.
	virtual void			HPChange( unsigned long objectHP, unsigned long objectMaxHP );
	virtual void			HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType );
	virtual void			MPChange( unsigned long objectMP, unsigned long objectMaxMP );

	/// hero에서 아무 처리 하지 않는다.
	virtual void PlayerAttack( unsigned long, unsigned long, cBaseObject*, float ) {}
	virtual void PlayerUseSkill( unsigned long /*uIdx*/, unsigned long /*skillIdx*/, unsigned char /*skillStep*/, cBaseObject* /*pTarget*/ ) {}
	virtual void PlayerUseSkill( unsigned long /*uIdx*/, unsigned long /*skillIdx*/, unsigned char /*skillStep*/, NiPoint3 /*fieldPos*/ ) {}
	virtual void PlayerMapChangeSkill( unsigned long /*uIdx*/, unsigned long /*skillIdx*/ ) {}
	virtual void PlayerVehicleSkill( unsigned long /*uIdx*/, unsigned long /*skillIdx*/ ) {}
	virtual void PlayerBlinkSkill( unsigned long /*uIdx*/, unsigned long /*skillIdx*/, unsigned char /*skillStep*/, NiPoint3 /*blinkPos*/ ) {}

	inline void SetWaitMoveFlag( unsigned int flag ) { mWaitMoveFlag = flag; }

	virtual bool EquipWeaponItem( sPlayerWeaponInfo* info );
	virtual void EquipWeaponInfo( eWEAPON_STATE state, sPlayerWeaponInfo* pInfo );

	virtual void ChangeWeaponState( eWEAPON_STATE state );


	/// 후처리
	void AfterAttack( cBaseObject* pTarget );
	void AfterItemPick( unsigned long idx, float destX, float destY );
	void AfterUseSkill( cBaseObject* pTarget );
	void AfterUseSkill( NiPoint3 fieldPos );

	void AfterNpcTalk( unsigned long npcIdx );
	void AfterNpcTalk( cNpc* pNpc );

	void AfterUseTarotStore( cPlayer* pPlayer );
	void AfterMapChange( cMapPortalSceneNode* portalNode );
	void AfterUseStall( cPlayer* pPlayer );
	void AfterGatheringPick( unsigned long idx, unsigned long classIdx, float destX, float destY );

	inline sAfterAction* GetCurrentAfterAction() { return &mAfterAction; }
	bool IsProcessAfterAction();

	void SetLevel( unsigned char level );

	inline void CalcStatus() { mIsCalcStatus = true; }
	void		NowCalcStatus();
	inline void UpdateHeroInfo() { mIsUdateHeroInfo = true; }

	/// 주의 :  target지정 - 공격후 스킬 발동
	///			target지정 안함 - 스킬 장착
	void					WaitWeaponSwitch() { mWaitWeaponSwitch = true; }

	bool					IsWeaponChanging() { return mSwitchWeaponFlag; }
	void					SetWeaponChangeFlag( bool val );

	inline bool				CheckSwichDelay() { return (mSwitchWeaponTime != 0); }

	virtual bool IsPossibleSitDown();

	virtual void AddBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelBuff( unsigned long uniqueIdx );

	virtual void AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelDeBuff( unsigned long uniqueIdx );

	virtual void BlinkBuffIcon( unsigned long uniqueIdx, unsigned long influenceIdx );
	virtual void BlinkDeBuffIcon( unsigned long uniqueIdx, unsigned long influenceIdx );

	inline unsigned long GetMoney() { return mMoney; }
	void SetMoney( unsigned long money ) { mMoney = money; }

	inline unsigned long GetDeposit() { return mDeposit; }
	void SetDeposit( unsigned long money ) { mDeposit = money; }

	inline unsigned long GetTarotPoint() { return mHeroInfo.TarotPoint; }
	void SetTarotPoint( unsigned long point ) { mHeroInfo.TarotPoint = point; }

	unsigned long GetPVPPoint( unsigned char forceType );
	void SetPVPPoint( unsigned long point, unsigned char forceType );

	unsigned long		GetMaxWeight()				{ return FloatToInt( mStatus2.mMaxWeight ); }

	/// 착용 아이템 세트 갱신
	void ResetArmorSets();
	unsigned short  GetArmorSets() { return mArmorSets; }

	virtual void ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker = 0 );

	unsigned long		GetNpcIdx()	{ return mNpcIdx; }
	
	void				OpenNpcTalk( unsigned long npcIndex, bool setUI = true );
	void				CloseNpcTalk( bool sendMsg = true );
	bool				IsOpenNpcTalk();
	
	void				SetNpcFunc( eNpcFunc npcType ) { mNpcFuncType = npcType; }
	inline eNpcFunc		GetNpcFunc() { return mNpcFuncType; }

	void				CloseBySummonItem( char nCategory, char nProtocal );

	virtual void	SetFixedObjectSizeScale( unsigned short size, bool init );

	void ActiveQuestText( LPCTSTR msg, unsigned long color );
	void ActivePosition( float x, float y );
	bool ReActivePosition();
	void DeActivePosition();

	/// 찾아가기 기능
	bool			IsActiveFindPositon() { return mIsFindPosition; }
	void			StopFindPos();
	
	/// 결투 신청/포기
	void			DuelRequest( unsigned long playerIdx = 0 );
	void			DuelGiveUp();

	/// 결투 수락/거부 창 출력
	void			ViewMsgDuelReq( unsigned long duelReqPlayerIdx );
	/// 수락거절 메세지 처리
	void			DuelNoStartMsg( int errorCode );		/// 결투불가 메세지 처리

	void			SetCantMove( bool cantMove )	{ mCantMove = cantMove; }
	void			SetCantSkill( bool cantSkill, unsigned char attributeType );
	bool			IsCantSkill( unsigned long skillIdx );//	{ return mCantSkill == true || mIsSleep == true || mIsStun == true; }
	bool			IsCantMove()	{ return mCantMove == true || mIsSleep == true || mIsStun == true; }
	bool		    IsAvoidMonLook(){ return mIsAvoidMonLook == true; }

	void			SetSleep( bool sleep );
	void			SetStun( bool stun );
	void			SetAvoidMonLook( bool avoidMonLook );

	void			SetStaticFaceReq( unsigned long faceIdx );

	void			CheckTabTargetObject();
	//void			CheckTabPartyObject();
	void			ClearTabTargetList();

	/// 알파
	void SetAlphaBlended( float alpha );

	virtual bool TransformMonster( unsigned long monsterClassIdx );
	virtual void RestoreMonster();

	void			SetForceType( unsigned char forceType );
	void			SetFirePoint( unsigned long firePoint );
	void			SetFireFriendly( unsigned long fireFriendly );
	void			SetWaterPoint( unsigned long waterPoint );
	void			SetWaterFriendly( unsigned long waterFriendly );
	void			SetWindPoint( unsigned long windPoint );
	void			SetWindFriendly( unsigned long windFriendly );
	void			SetEarthPoint( unsigned long earthPoint );
	void			SetEarthFriendly( unsigned long earthFriendly );

	unsigned long	GetFirePoint() { return mHeroInfo.mFirePoint; }
	unsigned long	GetWaterPoint() { return mHeroInfo.mWaterPoint; }
	unsigned long	GetWindPoint() { return mHeroInfo.mWindPoint; }
	unsigned long	GetEarthPoint() { return mHeroInfo.mEarthPoint; }

	void SetFieldEffRadius( float r );
	void UpdateFieldTargetEffect( NiPoint3 center );

	/// 전직 
	void SetJob( ePLAYER_JOB job );

	/// 파티 세팅
	void SetPartyIndex( unsigned long partyIndex );

	/// 파티 룸 인덱스
	void SetPartyRoomIndex( unsigned short roomIndex ) { mPartyRoomIndex = roomIndex; }
	unsigned short GetPartyRoomIndex() { return mPartyRoomIndex; }

	/// 길드 세팅
	void SetGuild( unsigned long guildIndex, unsigned char guildposition, LPCTSTR guildName, tm markRegistDate, bool useMark );

	void UseMapChangeSkill( unsigned long skillIdx, unsigned short slotIdx );
	void UseCashMapChangeSkill( unsigned long skillIdx, unsigned short slotIdx, unsigned short mapIndex, float mapX, float mapY );
	void UseVehicleSkill( unsigned long skillIdx, unsigned short slotIdx );
	

	bool GatheringStart( unsigned long gaderIdx, unsigned long gatherClassIdx, NiPoint3 targetPos );
	void GatheringEnd();

	bool GatheringCancel();

	bool CheckGatheringEnable( unsigned long gaderClassIdx, unsigned char teamType );

	/// 
	bool ItemMixCancel();
	bool ItemEnhanceCancel();
	
	/// 상황에 따른 npc 대화상태 취소
	bool NpcSpeechCancel();

	/// 탈것 처리
	virtual void RideVehicle( unsigned long vehicleIdx );
	virtual void AlightVehicle();

	/// 오늘의 한마디 미리보기
	void PreviewTodayWord( LPCTSTR word, unsigned long color );
	
	/// 길드마크 미리보기
	void PreviewGuildMark( NiTexture* tex );

	/// 길드마크 기존정보로 갱신
	void UpdateGuildMark();

	/// 조합 직업 체크
	bool CheckHeroMakeSkill( unsigned long makeSkillIdx );

	/// 조합습득 스킬 저장
	void SetMixSkillFirst( unsigned char skill1, unsigned char step );
	void SetMixSkillSecond( unsigned char skill2, unsigned char step );

	void UpdateMixSkillExp( unsigned char skillIndex, unsigned long exp );

	void PartyIn_Init() {}
	void PartyOut_Init() {}

	/// Keyboard Move
	inline bool IsKeyMoving() { return mIsKeyMove; }
	inline bool IsGoContinue() { return mGoCountinue; }

	void UpdateKeyState();
	void CheckSendMoveMsg( unsigned long delta, bool force );

	void GoCountinueMove();
	void StopCountinueMove();

	/// jump
	virtual bool Jump();

	unsigned long GetFollowPlayerIndex(){ return mFollowPlayerIndex; }
	bool RequestFollowPlayer( unsigned long idx );
	void ReleaseFollowPlayer();
	bool IsFollowing(){ return mIsFollowing; }

	void ClearMoveFlag();

	// 맵체인지로 인한 상태 취소 요청
	void CancelByMapchange();

	void SetMapPortalSkipTime( unsigned long time );

	bool IsEnemy( cPlayer* player );

	void UseBlinkSkill( unsigned long skillIdx, unsigned char skillStep, bool isForward );

	void SetPos_ResetCam( NiPoint3 pos );

	/// 슬롯 제한 인덱스 가져오기
	unsigned short GetBagEndSlot()       { return mHeroInfo.BagEnd;  }
	unsigned short GetWearCardEndSlot()  { return mHeroInfo.WearEnd; }
	unsigned short GetWareHouseEndSlot() { return mHeroInfo.WareHouseEnd; }
	unsigned short GetMakeSkillEnd()     { return mHeroInfo.MakeSkillEnd; }

	void SetTitle( unsigned long titleIdx );

	// 살펴보기 요청
	void RequestLookPlayer( unsigned long playerIndex );

	virtual void SetSkillRush( float posX, float posY, sObject target, unsigned long nextSkill, unsigned char nextStep, bool blink );	
	virtual bool RushUpdate( unsigned long deltaTime );
	virtual void RushSkillStop()	{ mRushSkill = 0; mRushStep = 0; mRushBlink = false; }

protected:
	/// 애니메이션 관계 처리
	virtual void Interpret( unsigned long time );
	virtual void InterpretVehicle( unsigned long time );
	virtual void InterpretEvent( unsigned long id, float checkTime, cActorManagerForPartition* pActor );
	virtual void InterpretEvent_Vehicle( unsigned long id, float checkTime, cActorManagerForPartition* pActor );

	/// 이동
	virtual bool	Move( float x, float y );
	virtual	bool	ActionMove( float x, float y, float range );

	void StopAfterAction();

	/// 실제 기본 공격 시작 (네트워크 패킷 발송)
	bool			StartAttack( unsigned long attackIdx, cBaseObject* pTarget );

	/// 실제 스킬 시작 (네트워크 패킷 발송)
	void			UseTargetSkill( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget );
	void			UseFieldSkill( unsigned long skillIdx, unsigned char skillStep, NiPoint3 fieldPos );
	void			UseRushTargetSkill();

	void			AutoActionClear();

	void			ProcessAfterAction( NiPoint3& heroPos );

	/// player에서만 사용하고 혹시 모르는 hero에서의 사용을 막는다.
	virtual void	SetActionMove( float, float, float ) {}

	virtual cDynamicSceneNode* CreateSceneNode( cPlayerSceneNodeParam* param );

	//virtual void ClearAllBuff();
	//virtual void ClearAllDeBuff();
	/// 090302 PKH 죽었을때 삭제되지 않는 효과 처리 포함
	virtual void ClearAllBuff( bool delDieInf );
	virtual void ClearAllDeBuff( bool delDieInf );

	void ProcessDelayFlag( unsigned long deltaTime );


	/// monster transform process
	virtual void Update_TransMon( unsigned long deltaTime, unsigned long accumTime );
	virtual void Interpret_TransMon( unsigned long time );
	void InterpretEvent_TransMon( unsigned long id, float checkTime, cActorManagerForPartition* pActor );

	virtual void ApplyDamageDrama_TransMon( int damage, unsigned int damageType, cBaseObject* pAttacker );
	bool StartAttack_Mon( unsigned long attackIdx, cBaseObject* pTarget );
	void UseTargetSkill_TransMon( unsigned long skillIdx, cBaseObject* pTarget );
	void UseFieldSkill_TransMon( unsigned long skillIdx, NiPoint3 fieldPos );

	/// key move
	void KeyMoveUpdate( unsigned long deltaTime );
	bool SetDesiredDir( NiPoint3 dir );

	bool IsEnableKeyMove();

	bool EnableGateMovePos( NiPoint2 startPos, float& x, float& y );    
protected:
	float			mDefaultCameraOffset;
	float			mCameraOffset;

	sHeroInfo		mHeroInfo;
	tArray<unsigned short>	mJobUsedSkillPoint;

	/// hero info window update 필요여부.
	bool			mIsUdateHeroInfo;

	/// status 재계산 필요여부.
	bool			mIsCalcStatus;

	/// 소속 map번호..
	unsigned short	mMapNum;
	unsigned char	mMapMode;
	

	/// 
	unsigned long	mChannelNum;

	///
	unsigned long	mAreaNameIdx;

	/// 스텟1 기본값
	sStatus1		mStatus1;
	/// 스텟1 +값
	sStatus1		mStatus1Plus;
	sStatus1		mStatus1Per;
	/// 스텟2 최종값
	sStatus2		mStatus2;

	/// 스텟외 스텟
	sStatusEtc		mStatusEtc;

	/// 데미지 계산용 순간효과 변수
	sDamageCalc		mDamageCalc;

	/// mp % 소모시 계산될 수치
	unsigned long	mOriMaxMP;

	/// 070508 PKH 스텟 증감(색상) 타입
	unsigned char	mStatusBaseInfoType[ePLAYER_STATUS_LEVEL];
	unsigned char	mStatusExtInfoType[ePLAYER_STATUS_EXT_MAX];

	/// 착용한 아이템 세트
	unsigned short  mArmorSets;

	/// 돈 정보
	unsigned long	mMoney;

	/// 창고 보관액 정보
	unsigned long	mDeposit;

	/// 이동 후 처리
	sAfterAction	mAfterAction;

	/// 무기 교체 대기
	bool			mWaitWeaponSwitch;

	/// 대기 스킬
	sSkillInfo		mWaitSkill;

	/// 공격중 이동 대기 - 주의 : Attack상태에서만 유효하다
	unsigned int	mWaitMoveFlag;
	NiPoint2		mWaitMovePos;

	/// 직업군이 magic/shaman계열인지 체크
	bool			mMageJobType;

	unsigned long	mRushSkill;
	unsigned char	mRushStep;
	bool			mRushBlink;

	/// 대화중인 npc 인덱스
	unsigned long	mNpcIdx;

	/// 이용중인 npc 타입
	eNpcFunc mNpcFuncType;

	/// 이동 불가
	bool			mCantMove;
	/// 스킬 사용 불가
	bool			mCantSkillPhy;
	/// 스킬 사용 불가
	bool			mCantSkillMag;
	/// 수면
	bool			mIsSleep;
	/// 스턴
	bool			mIsStun;
	/// 몬스터 선공 인식 회피
	bool		    mIsAvoidMonLook;

	unsigned long	mSumKeyMoveDeltaTime;

	/// 길찾기
	static const unsigned int MAX_PATH_COUNT = 2048;
	static const unsigned int MAX_PATH_DIST = 30000;
	NiPoint2 mPathArray[MAX_PATH_COUNT];
	unsigned int mPathIndex;
	unsigned int mPathCount;

	//unsigned long	mDuelReqPlayerIdx;

	/// 평화 toggle
	unsigned long	mPeaceToggleTime;
	bool			mPeaceToggleFlag;

	unsigned long	mSwitchWeaponTime;
	bool			mSwitchWeaponFlag;

//	typedef tPointerList<void*> cMoveTargetEffect;
//	cMoveTargetEffect mMoveTargetList;
	unsigned long mMoveTargetEffectIdx;
	cEffectSceneNode* mMoveTargetEffect;

	/// 몬스터 탭타겟을 위한 설정
	NiTPointerList<sObject*> mTabTargetList;

	/// Keyboard Move
	enum
	{
		MOVE_NONE	= 0,
		MOVE_W		= (1 <<  0),
		MOVE_S		= (1 <<  1),
		MOVE_A		= (1 <<  2),
		MOVE_D		= (1 <<  3),
	};
	bool	mIsKeyMove;
	bool	mGoCountinue;

	//bool	mSendKeyMoveMsg;
	unsigned long mCheckSendTime;

	/// 찾아가기 기능
	bool	mIsFindPosition;
	unsigned char	mFindPosCnt;
	NiPoint2		mFindPos;

	/// 따라가기 플레그
	bool			mIsFollowing;
	unsigned long   mFollowUpdateTime;
	unsigned long   mFollowPlayerIndex;


	/// 맵포탈 체크
	unsigned long	mMapPortalSkipTime;

	bool			mAutoAttackFlag;

	// 파티 룸 인덱스
	unsigned short	mPartyRoomIndex;
};

inline
bool cHero::IsProcessAfterAction()
{
	return (mAfterAction.mActionType != eAfterAction_None);
}
