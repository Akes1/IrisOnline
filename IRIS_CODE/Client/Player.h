/* ====================================================================
* 파    일 :	Player.h
* 목    적 :	플레이어 관리
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
* 주의사항 :	
* =================================================================== */
#pragma once
#include "BaseObject.h"
#include "Player_Common.h"
#include "StatusCalc_Client.h"

enum eVEHICLE_ANIMATIONTYPE
{ 
	V_ANITYPE_IDLE1 = 1,	/// 대기
	V_ANITYPE_IDLE2 = 2,	/// 대기보조
	V_ANITYPE_RUN = 3,		/// 달리기
	V_ANITYPE_JUMP = 4,		/// 점프
};

enum eREADY_TRANSSTATE
{
	eReady_None = 0,
	eReady_TransformMonster,
	eReady_RestoreMonster, 
};

struct sReadyTransform
{
	unsigned char		readState;
	unsigned long		chgMonIdx;

	sReadyTransform()
	{
		readState = eReady_None;
		chgMonIdx = 0;
	}
};

class cPathFinder;
class cPlayerSceneNodeParam;
class cActorManagerForPartition;

enum eGATHERING_TYPE;

class cPlayer : public cBaseObject
{
public:
	cPlayer( unsigned char type = eOBJECTTYPE_PLAYER );
	virtual ~cPlayer();

	inline virtual unsigned long	GetObjectID() { return mPlayerInfo.CharacterIdx; }

	/// 생성부
	virtual bool Create( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, sPlayerExrInfo* pExrInfo,
		NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );

	virtual void ReCreate( char race, char gender );

	/// 객체의 그래픽 데이터에 대한 제거
	virtual void RemoveObjectSceneNode();

	/// 갱신부
	virtual void Update( unsigned long deltaTime, unsigned long accumTime );

	/// 상태갱신부
	virtual void	IdleUpdate( unsigned long deltaTime );
	virtual void	DieUpdate() {}
	virtual void	MoveUpdate( unsigned long deltaTime );
	virtual void	PushPullUpdate( unsigned long deltaTime );
	virtual bool	RushUpdate( unsigned long deltaTime );
	virtual void	RushSkillStop() {}

	/// 객체 이동
	virtual bool	Move( float x, float y );

	virtual void	SetActionMove( float tPosX, float tPosY, float range );
	virtual void	ClearActionMoveRange()	{ mActionMoveRange = 0; }

	virtual bool	GetDummyPos( unsigned int dummyId, NiPoint3& pos );

	virtual inline LPTSTR	GetName() { return mPlayerInfo.strName; }
	virtual inline void		SetName( LPTSTR name );
	virtual inline char		GetGender() { return mPlayerInfo.Gender; }
	virtual inline char		GetRace() { return mPlayerInfo.Race; }
	virtual inline void		SetGender( char gender ) { mPlayerInfo.Gender = gender; }
	virtual inline void		SetRace( char race ) { mPlayerInfo.Race = race; }

	inline NiPoint3 GetDesireDir() { return mDesiredDir; }
	
	inline sPlayerInfo*		GetPlayerInfo() { return &mPlayerInfo; }
	inline sPlayerExrInfo*		GetPlayerExrInfo() { return &mPlayerExrInfo; }

	virtual void EquipWeaponInfo( eWEAPON_STATE state, sPlayerWeaponInfo* pInfo );
	virtual void EquipWearInfo( eWEAR_TYPE type, unsigned long itemIdx );
	virtual void EquipCostumeInfo( unsigned char type, unsigned long itemIdx );
	virtual void EquipCostumeAcceInfo( unsigned char type, unsigned long itemIdx );

	inline eWEAPON_STATE	GetWeaponState() { return mPlayerWeaponState; }
	inline unsigned long	GetLeftWeaponIdx() { return mPlayerWeaponInfo.WeaponIdx[eHAND_LEFT]; }
	inline unsigned long	GetRightWeaponIdx() { return mPlayerWeaponInfo.WeaponIdx[eHAND_RIGHT]; }

	float			GetAttackSpeed() { return mPlayerExrInfo.mAttackSpeed; }
	virtual void	SetAttackSpeed( float speed )	{ mPlayerExrInfo.mAttackSpeed = speed; }

	/// 061123 wonju 
	void				SetPathFinder( cPathFinder* pathFinder );

	bool				SetDesiredDir(NiPoint3 CurrentPos, NiPoint3 TargetPos, unsigned long delay = 0 );
	inline unsigned int	GetRaceGender(){ return (mPlayerInfo.Race*eGENDER_MAX + mPlayerInfo.Gender); }

	//////////////////////////////////////////////////////////////////////////
	/// Player의 상태 정보를 변화
	virtual unsigned int	GetState() { return mPlayerExrInfo.mState; }
	virtual void			SetState( unsigned char state , bool bShowAni = true );

	/// 부활시 행동
	virtual void Rebirth();

	/// 앉기/서기
	virtual	void SitDown();
	virtual void StandUp();

	virtual bool IsPossibleSitDown() { return (mSitDownFlag==false); }

	virtual void	SetIdleFlag( unsigned char flag );
	eSTOPFLAG		GetStopFlag() { return (eSTOPFLAG)mPlayerExrInfo.mStateStop; }
	virtual void	SetStopFlag( unsigned char flag );

	void ShowWeapon( bool show );
	void ShowNameGauge( bool show );
	virtual void PartyIn_Init();
	virtual void PartyOut_Init();

	//////////////////////////////////////////////////////////////////////////

	/// 070115 PKH 직업
	ePLAYER_JOB			GetJob() { return mPlayerInfo.Job; }
	virtual void		SetJob( ePLAYER_JOB job ) { mPlayerInfo.Job = job; }

	/// 070115 PKH 플레이어 레벨
	unsigned char		GetLevel() { return mPlayerInfo.Level; }
	virtual void		SetLevel( unsigned char level )	{ mPlayerInfo.Level = level; }


	/// HP, MP 셋팅
	unsigned long	GetHP()						{ return mPlayerExrInfo.RestHP; }
	unsigned long	GetMP()						{ return mPlayerExrInfo.RestMP; }

	void SetMaxHP( unsigned long maxHP )	{ mPlayerExrInfo.MaxHP = maxHP; }
	void SetMaxMP( unsigned long maxMP )	{ mPlayerExrInfo.MaxMP = maxMP; }

	virtual unsigned long		GetMaxHP()					{ return mPlayerExrInfo.MaxHP; }
	virtual unsigned long		GetMaxMP()					{ return mPlayerExrInfo.MaxMP; }

	virtual void	HPChange( unsigned long objectHP, unsigned long objectMaxHP );
	virtual void	HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType );
	virtual void	MPChange( unsigned long objectMP, unsigned long objectMaxMP );

	virtual void AddBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelBuff( unsigned long uniqueIdx );
	virtual void AddDeBuff( unsigned long uniqueIdx, cInfluenceObject* p );
	virtual void DelDeBuff( unsigned long uniqueIdx );

	virtual long GetMoveSpeed() { return mPlayerExrInfo.mMoveSpeed; }
	virtual void SetMoveSpeed( long moveSpeed );

	virtual unsigned long GetTitleIndex() { return mPlayerExrInfo.mTitleIndex; }

	void SetRemoveDropItem( unsigned int idx );

	bool ChangeBaseWear( unsigned int part );

	void ResetBaseFace();
	void SetEmotionFace( unsigned long faceIdx );
	void SetStaticFace( unsigned long faceIdx );

	/// 아이템 해체
	bool UnEquipWearItem( unsigned long itemIdx );
	void UnEquipWeaponItem( eHAND_TYPE handType );

	void ActiveChatBubble( LPTSTR msg, unsigned long color );
	void ActiveTarotBubble( LPTSTR title, unsigned long color );
	void ActiveUserSellBubble( LPTSTR title, unsigned long color );
	void DeActiveTarotBubble();
	void DeActiveUserSellBubble();

	virtual cEffectSceneNode*	SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop=false, bool bFollow=true );
	virtual cEffectSceneNode*	SetLinkdDamage( unsigned int num, const char* nif, NiTransform* trans, bool IsMiss, bool zFalse );
  
	virtual void PlayerAttack( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pTarget );
	virtual void PlayerUseSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget );
	virtual void PlayerUseSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, NiPoint3 fieldPos );
	virtual void PlayerMapChangeSkill( unsigned long uIdx, unsigned long skillIdx );
	virtual void PlayerVehicleSkill( unsigned long uIdx, unsigned long skillIdx );
	virtual void PlayerBlinkSkill( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, NiPoint3 blinkPos );

	virtual void ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker = 0 );

	/// SetState() skill->processActive() 외 사용 하지 마시오!! 
	virtual void ChangeAnimation( ePLAYER_ANIMATIONTYPE type );
	virtual void ChangeAnimation_TransMon( unsigned int type );

	///
	virtual void UpdateSkillAnimation( unsigned int aniIdx, bool checkWeapon = false );

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float			GetFixedObjectSize();
	virtual void	SetFixedObjectSizeScale( unsigned short size, bool init );

	/// 이동정지
	virtual void SetCantMove( bool /*cantMove*/ ) {}
	virtual void SetSleep( bool /*sleep*/ )	{}
	virtual void SetStun( bool /*stun*/ )	{}


	void SetDuelEffect();
	void ClearDuelEffect();

	void SetItemMixEffect();
	void ClearItemMixEffect();

	void SetItemEnhanceEffect();
	void ClearItemEnhanceEffect();

	void SetItemGatheringEffect( eGATHERING_TYPE gatheringType );
	void ClearItemGatheringEffect();

	void SetItemDisjointSound();
	void ClearItemDisjointSound();

	/// 몬스터 외형 변신
	virtual bool TransformMonster( unsigned long monsterClassIdx );
	virtual void RestoreMonster();

	bool IsTransformMonster() { return (mPlayerExrInfo.mChgMonsterIdx != 0); }
	unsigned long GetTransMonsterClassIdx() { return mPlayerExrInfo.mChgMonsterIdx; }
	void ReadyTransform( unsigned char readyState, unsigned long chgMonIdx );

	/// 탈것 처리
	virtual void RideVehicle( unsigned long vehicleIdx );
	virtual void AlightVehicle();

	bool IsRideVehicle();

	virtual bool IsDie() { return (GetState()==eOBJECT_STATE_DIE); }
	virtual void FadeOutObject() {}

	/// 세력
	virtual void SetForceType( unsigned char forceType ) { mPlayerExrInfo.mForceType = forceType; }
	unsigned char GetForceType() { return mPlayerExrInfo.mForceType; }

	inline unsigned char GetPVPTeam() { return mPlayerExrInfo.mPvPDMTeamType; }
	
	virtual inline void		SetTitle( unsigned long titleIdx );
	virtual inline void		SetPartyIndex( unsigned long partyIndex );
	unsigned long			GetPartyIndex() { return mPlayerExrInfo.mPartyIndex; }

	virtual inline void		SetPartyUnionIndex( unsigned long unionIndex );
	unsigned long			GetPartyUnionIndex() { return mPlayerExrInfo.mPartyUnionIndex; }

	virtual inline void		SetGuild( unsigned long guildIndex, unsigned char guildposition, LPCTSTR guildName, tm markRegistDate, bool useMark );
	virtual inline void		SetGuildPosition( unsigned char guildPosition );
	virtual inline void		ChangeGuildMarkRegistDate( tm markRegistDate, bool useMark );
	virtual inline void		ClearGuild();
	
	unsigned long			GetGuildIndex() { return mPlayerExrInfo.mGuildIndex; }
	unsigned char			GetGuildPosition() { return mPlayerExrInfo.mGuildPosition; }
	LPCTSTR					GetGuildName() { return mPlayerExrInfo.mGuildName; }
	
	void					SetTodayWord( LPCTSTR word, unsigned long color );

	///
	virtual bool GatheringStart( unsigned long gaderIdx, unsigned long gatherClassIdx, NiPoint3 targetPos );
	virtual void GatheringEnd();

	void UpdateHelmetState( bool show );

	/// jump
	virtual bool Jump();

	void SetJumpFlag( bool flag );
	bool IsJumping() { return mJumping; }
	bool IsJumpProcess() { return mJumpProcess; } 

	void UpdateNameCardTextValue();

	virtual void SetSkillPushPull( float posX, float posY );
	virtual void SetSkillRush( float posX, float posY, sObject target );

protected:

	/// 기본으로 제공되는 머리와 얼굴에 대한 교체
	bool SetBaseFace();

	virtual bool ChangeBaseHair( unsigned int hairIdx, unsigned int hairColorIdx );
	virtual bool ChangeFaceTexture( unsigned int faceIdx, bool base = false );

	/// 공격 종료
	virtual void EndAttack() {}

	/// 애니메이션 관계 처리
	virtual void Interpret( unsigned long time );
	virtual void InterpretEvent( unsigned long id, float checkTime, cActorManagerForPartition* pActor );
	virtual void InterpretVehicle( unsigned long time );
	virtual void InterpretEvent_Vehicle( unsigned long id, float checkTime, cActorManagerForPartition* pActor );

	/// 아이템 장착
	bool EquipWearItem( unsigned long itemIndex );
	bool EquipAccessoryItem( unsigned char costumPos, unsigned long itemIndex );
	virtual bool EquipWeaponItem( sPlayerWeaponInfo* info );

	virtual void ChangeWeaponState( eWEAPON_STATE state );

	virtual cDynamicSceneNode* CreateSceneNode( cPlayerSceneNodeParam* param );

	void ResetState();

	void DesiredDirProcess( unsigned long delta );
	void FaceProcess( unsigned long delta );

	/// monster transform process
	virtual void Update_TransMon( unsigned long deltaTime, unsigned long accumTime );
	virtual void SetState_TransMon( unsigned char state , bool bShowAni );
	virtual void Interpret_TransMon( unsigned long time );
	virtual void InterpretEvent_TransMon( unsigned long id, float checkTime, cActorManagerForPartition* pActor );
	virtual void ApplyDamageDrama_TransMon( int damage, unsigned int damageType, cBaseObject* pAttacker );

protected:
	/// 이동시 목적지 좌표
	float		mGotoX;
	float		mGotoY;

	/// 이동시 방향 및 
	NiPoint3	mDesiredDir;

	/// 이동시 거리 체크..
	float		mActionMoveRange;
	NiPoint3	mTargetPos; /// 목적지 좌표

	/// 객체의 정보 구조체
	sPlayerInfo				mPlayerInfo;
	sPlayerExrInfo			mPlayerExrInfo;
	sPlayerWearInfo			mPlayerWearInfo;

	cStringT				mTarotTitle;
	cStringT				mSellTitle;

	/// 링크되는 코스튬 인덱스 (Object Data Manager 제거용)
	unsigned long			mCostumeObjectIndex[eCOSTUME_MAX];

	/// 무기 정보
	sPlayerWeaponInfo		mPlayerWeaponInfo;

	unsigned long			mGatheringToolObj;

	/// 착용 무기에 대한 상태 정보
	eWEAPON_STATE			mPlayerWeaponState;

	/// 무기 그래픽 인덱스 (ObjectDataManager 제거용)
	unsigned long			mWeaponObjectIndex[eHAND_ALL];
	unsigned long			mWeaponObjectEffect[eHAND_ALL];

	/// 길찾기
	cPathFinder* mPathFinder;
	//cPathNodeSet	mPathNodeSet;

	ePLAYER_ANIMATIONTYPE	mAnimationType;
	unsigned int			mDamageAfterAnim;

	/// 앉(서)는중 = true
	bool					mSitDownFlag;

	/// 애니 동작 종료시에 드랍용 이미지를 제거 하기 위한 변수
	unsigned int	mRemoveDropItemIdx;

	bool				mShowWeapon;

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				mFixedObjectSize;

	/// 
	unsigned long		mDelay;
	float				mAngle;
	float				mAccumAngle;
	NiPoint3			mAxis;

	/// 3초유지용 표정
	unsigned long		mEmotionCommandIdx;
	unsigned long		mEmotionRemainTime;

	/// 눈깜박임 관련
	unsigned long		mWinkFaceIdx;
	unsigned long		mWinkTime;
	unsigned long		mWinkRemainTime;

	unsigned long		mDuelEffectIdx;
	cEffectSceneNode*	mDuelEffect;

	unsigned long		mSitDownEffectIdx;
	cEffectSceneNode*	mSitDownEffect;

	unsigned long		mItemMixEffectIdx;
	cEffectSceneNode*	mItemMixEffect;
	unsigned long		mItemMixSoundIdx;
	cSoundSceneNode*	mItemMixSoundNode;

	unsigned long		mItemEnhanceEffectIdx;
	cEffectSceneNode*	mItemEnhanceEffect;
	unsigned long		mItemEnhanceSoundIdx;
	cSoundSceneNode*	mItemEnhanceSoundNode;

	unsigned long		mItemGatheringEffectIdx;
	cEffectSceneNode*	mItemGatheringEffect;

	unsigned long		mItemDisjointSoundIdx;
	cSoundSceneNode*	mItemDisjointSoundNode;

	sReadyTransform		mReadyTransformInfo;

	bool				mJumping;
	bool				mJumpProcess;
	/// check jump time
	unsigned long		mJumpRemainTime;

	NiPoint2			mPushPullPos;
	/// 세력 정보
	//unsigned char		mForceType;		/// eFORCE_TYPE
};

inline void cPlayer::SetName( LPTSTR name ) 
{ 
	Sstrncpy( mPlayerInfo.strName, MAX_NAME_BUFFER_SIZE, name, MAX_NAME_SIZE ); 
	//	::_tcsncpy( mPlayerInfo.strName, name, MAX_NAME_SIZE ); 
	//	mPlayerInfo.strName[MAX_NAME_SIZE] = 0;
}

