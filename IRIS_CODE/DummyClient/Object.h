#pragma once

#include "BaseObject_Common.h"
#include "Player_Common.h"
#include "Monster_Common.h"

class cObjectManager;

struct sMonsterData;
struct sMonsterScript;

//////////////////////////////////////////////////////////////////////////
class cObject
{
public:
	virtual ~cObject();

	inline unsigned char GetType() { return mType; }
	virtual unsigned long GetObjectID() { return mObjectId; }

	virtual void Process( unsigned long /*deltaTime*/, unsigned long /*accumTime*/ ) {}


	NiPoint3 GetPos() { return mTrans.m_Translate; }

	cObject* GetTargetObject();
	sObject* GetTargetInfo() { return &mTarget; }
	void SetTargetObject( unsigned char type, unsigned long idx ) { mTarget.type = type; mTarget.index=idx; }

	unsigned int GetState() { return mState; }
	virtual void SetState( unsigned char state ) { mState = (eOBJECTSTATE)state; }

	virtual float GetFixedObjectSize() { return 0.0f;}

protected:
	cObject( unsigned char type, cObjectManager* manager );

protected:

	cObjectManager* mManager;

	unsigned char	mType;
	unsigned long	mObjectId;

	sObject mTarget;

	eOBJECTSTATE	mState;

	NiTransform		mTrans;
};


//////////////////////////////////////////////////////////////////////////
class cPlayer : public cObject
{
public:
	cPlayer( cObjectManager* manager );
	virtual ~cPlayer();

	bool Init( sPlayerInfo* pBaseInfo, sPlayerWearInfo* pWearInfo, sPlayerWeaponInfo* pWeaponInfo, sPlayerExrInfo* pExrInfo,
		NiPoint3 sPos, NiMatrix3 sRot = NiMatrix3::IDENTITY, float fScale = 1.0f );

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );


	virtual float GetFixedObjectSize();
	void SetFixedObjectSizeScale( unsigned short scalePer, bool init );

protected:
	/// 객체의 정보 구조체
	sPlayerInfo				mPlayerInfo;
	sPlayerExrInfo			mPlayerExrInfo;
	sPlayerWearInfo			mPlayerWearInfo;

	/// 무기 정보
	sPlayerWeaponInfo		mPlayerWeaponInfo;

	/// 착용 무기에 대한 상태 정보
	eWEAPON_STATE			mPlayerWeaponState;

	float				mFixedObjectSize;

};


//////////////////////////////////////////////////////////////////////////
class cMonster : public cObject
{
public:
	cMonster( cObjectManager* manager );
	virtual ~cMonster();

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	bool Init( sMonsterData* baseInfo, NiPoint3 pos, NiMatrix3 rotate );


	void SetPathArray( NiPoint2* moveArray, unsigned int count );

	void HPChange( unsigned long objectHP, unsigned long objectMaxHP );
	void HPChangeDie( unsigned long objectMaxHP );

	void SetMoveSpeed( unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType = eMOVESPEED_STOP );
	void MoveStop( float x, float y );

	virtual void SetState( unsigned char state );

	unsigned long GetHP() { return mHP; }
	unsigned long GetMaxHP() { return mMaxHP; }

	unsigned char GetLevel() { return mpMonsterInfo->mLevel; }

	virtual float GetFixedObjectSize();
	void SetFixedObjectSizeScale( unsigned short scalePer, bool init );

protected:
	bool Move( float x, float y );

protected:

	/// 몬스터 정보
	sMonsterScript*		mpMonsterInfo;

	/// 이동속도
	unsigned int		mMoveSpeed;

	/// HP/MP 관련
	unsigned long		mHP;
	unsigned long		mMP;
	unsigned long		mMaxHP;
	unsigned long		mMaxMP;

	/// 이동시 목적지 좌표
	float				mGotoX;
	float				mGotoY;
	/// 이동시 최종위치
	float				mLastGotoX;
	float				mLastGotoY;
	/// 이동시 방향 및 
	NiPoint3			mDesiredDir;
	/// 이동좌표 최종 목적지까기 가기전 정지의 범위
	float				mMoveStopRange;

	unsigned long		mMoveOverTime;		/// 경로 목적지까지 이동중 프로세스 간격에의해 넘어간 시간


	///몬스터 객체 삭제 시간
	unsigned long		mMonsterDieTime;

	eMOVE_SPEED_TYPE	mMoveSpeedType;

	/// 길찾기
	static const unsigned int MAX_PATH_COUNT = 2048;
	NiPoint2			mPathArray[MAX_PATH_COUNT];
	unsigned int		mPathIndex;
	unsigned int		mPathCount;

	unsigned short		mFixedObjectSizePer;
	float				mFixedObjectSize;

};


//////////////////////////////////////////////////////////////////////////
class cNpc : public cObject
{
public:
	cNpc( cObjectManager* manager );
	virtual ~cNpc();

	bool Init( unsigned long objId, unsigned long npcClassIdx, NiPoint3 sPos, NiMatrix3 sRot );

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

protected:
	unsigned long mClassId;
};

class cPortal : public cObject
{
public:
	cPortal( cObjectManager* manager );
	virtual ~cPortal();

	bool Init( unsigned long posScriptIdx, NiPoint3 pos, NiMatrix3 rot );

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	unsigned long GetPosScriptIdx() { return mPosScriptIdx; }
	unsigned long GetTargetScriptIdx() { return mTargetScriptIdx; }
	float GetRange() { return mRange; }

protected:
	unsigned long mPortalIdx;
	unsigned long mPosScriptIdx;
	unsigned long mTargetScriptIdx;

	float mRange;
};


//////////////////////////////////////////////////////////////////////////
class cGather : public cObject
{
public:
	cGather( cObjectManager* manager );
	virtual ~cGather();

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );
};


//////////////////////////////////////////////////////////////////////////
class cTotem : public cObject
{
public:
	cTotem( cObjectManager* manager );
	virtual ~cTotem();

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );
};

//////////////////////////////////////////////////////////////////////////
class cDropItem : public cObject
{
public:
	cDropItem( cObjectManager* manager );
	virtual ~cDropItem();

	bool Init( unsigned long idx, NiPoint3 pos, unsigned long* owner, unsigned long ownTime, unsigned long* party, unsigned long partyTime, bool rare );

	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	bool IsOwner( unsigned long idx );

protected:
	unsigned long mOwner[MAX_ITEM_OWNER];
	unsigned long mTimeToOwn;

	unsigned long	mParty[ MAX_ITEM_PARTY ];
	unsigned long	mTimeToParty;

	bool		mIsRareItem;

	bool		mAllOwn;

};