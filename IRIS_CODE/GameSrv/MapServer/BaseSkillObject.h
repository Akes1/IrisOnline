/* ==========================================================================
* 파    일 : SkillObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-06-07
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Skill_Common.h"
#include "RangeCheck.h"
#include "SkillTargetPool.h"

enum eSkillCode
{
	NONUSESKILL,
	ACTIVESKILL,
	PASSIVESKILL,
};

enum eSKILLPROCESS
{
	eSKILLPROCESS_CASTING,			/// 캐스팅
    eSKILLPROCESS_ACTIVITY,			/// 발동
	eSKILLPROCESS_PROJECTILE,		/// 발사체
	eSKILLPROCESS_DAMAGE,			/// 데미지
	eSKILLPROCESS_HP,				/// HP
	eSKILLPROCESS_DESTRUCTION,		/// 소멸
};

enum eSKILLCREATE_TYPE
{
	eSKILLCREATETYPE_NORMAL,		 /// 일반 스킬
	eSKILLCREATETYPE_MAPCHANGE,		 /// 맵이동 스킬
	eSKILLCREATETYPE_VEHICLE,		 /// 탈것 생성 스킬
	eSKILLCREATETYPE_BLINK,			 /// 순간이동
	eSKILLCREATETYPE_DELTARGET,		 /// 타겟 해제 스킬
	eSKILLCREATETYPE_CASHMAPCHANGE,	 /// 캐시 맵이동 스킬
	eSKILLCREATETYPE_CLEAR_COOLTIME, /// 쿨타임 클리어
};

class cBaseObject;
struct sDramaAction;
class sDramaState;

enum eOBJECTTYPE;

class cBaseSkillObject
{

public:

	cBaseSkillObject();
	virtual ~cBaseSkillObject();

	unsigned long	GetAttackerIdx()	{ return mAttacker.index; }
	unsigned char	GetAttackerType()	{ return mAttacker.type; }
	eSKILLPROCESS	GetSkillState()		{ return mSkillState; }
	unsigned long	GetUniqueIdx()		{ return mUniqueIdx; }
	bool			IsProjectile()		{ return mIsProjectile; }
	bool			IsDetailEnd()		{ return mDetailCnt <= mDetailPos + 1; }
	unsigned long	SkillEndTime();
	bool			IsChgMonsSkill()	{ return mIsChgMonsSkill; }
	eATTRIBUTETYPE	GetAttributeType()	{ return mAttributeType; }
	unsigned long	GetSkillIdx()		{ return mSkillIdx; }
	void			AddSendTarget( unsigned long playerIdx );

	sSkillTargetRoot*	GetTargetRoot()	{ return &mTargetRoot; }

	void			Process( unsigned long elapsedTime, unsigned long accumTime );
	bool			IsChanneling()		{ return mIsChanneling; }

protected:

	/// 캐스팅시 공격자 / 타겟 객체 체크
	virtual bool	CastingObjectCheck( cBaseObject* pAttacker ) = 0;

	/// 스킬 발동시 소모항목 소모
	virtual bool	RequireValue( cBaseObject* pAttacker ) = 0;

	/// 발사체 목적지 설정
	virtual bool	SetCenterPos() = 0;

	/// 발사체 이동
	bool			IsProjectileEndProcess( unsigned long elapsedTime );

	/// 타겟 선택
	virtual void	TargetSelect( cBaseObject* pAttacker ) = 0;

	/// 데미지 적용
	virtual void	ApplyDamage( cBaseObject* pAttacker ) = 0;
	/// 스킬타입별 처리
	void			SendDamage( cBaseObject* pAttacker, 
		unsigned long shotType, eATTRIBUTETYPE attribType, eRANGETYPE rangeType,
		unsigned short applyType1, unsigned short applyType2, unsigned long applyValue1, unsigned long applyValue2,
		unsigned long influenceClassIdx, short accuracyValue, unsigned short criticalValue, bool applyHP );

	void			SendHeal( cBaseObject* pAttacker, unsigned long shotType,
		unsigned short applyType1, unsigned short applyType2, unsigned long applyValue1, unsigned long applyValue2,
		unsigned long influenceClassIdx );

	void			SendApply( cBaseObject* pAttacker, unsigned long shotType, eATTRIBUTETYPE attribType, short accuracyValue,
		unsigned short applyType1, unsigned short applyType2, 
		unsigned long applyValue1, unsigned long applyValue2, unsigned long influenceClassIdx );

	void			SendDelTarget( cBaseObject* pAttacker, short accuracyValue );

	void			ApplyBlink( cBaseObject* pAttacker );

	void			AddTotem( cBaseObject* pAttacker, unsigned long partyIdx, unsigned long partyUnionIdx, eAPPLYTYPE applyType, 
		eATTRIBUTETYPE attributeType, unsigned long influenceClassIdx, unsigned char applyCnt );

	void			AddPortal( cBaseObject* pAttacker, unsigned long mapChangePosIdx );

	void			AttackerNullDamage();

	/// HP 적용 ( true : 적용항목 남음, false : 적용항목 없음 )
	virtual bool	ApplyHP( cBaseObject* pAttacker );

	void			SelectPlayerTarget( cBaseObject* pAttacker, NiPoint3 centerPos, float rangeX, float rangeY, unsigned char* pTargetCnt, 
		unsigned long maxCnt, bool applyHP, bool enemy, eBOUNDTYPE boundType );
	void			SelectMonsterTarget( cBaseObject* pAttacker, NiPoint3 centerPos, float rangeX, float rangeY, unsigned char* pTargetCnt, 
		unsigned long maxCnt, bool applyHP, eBOUNDTYPE boundType );

protected:

	typedef tHashSet<unsigned long> cHashSet;
	typedef tArray<sObject> cObjectAry;
	typedef tArray<unsigned long> cLongAry;

	/// 고유 인덱스
	unsigned long	mUniqueIdx;

	/// 스킬 상태
	eSKILLPROCESS	mSkillState;

	///	공격자(사용자)의 정보
	sObject			mAttacker;
	/// 타겟 기준 스킬 기준 오브젝트
	sObject			mCenterTarget;

	/// 적용시점(=데미지) 기준위치
	NiPoint3		mCenterPos;
	/// 맵넘버
	unsigned long	mMapNumber;

	/// 스킬 생성 시점(절대 시간)
	unsigned long	mCreationTime;

	/// 발동 시간
	unsigned long	mActivityTime;
	/// 캐스팅 시간
	unsigned long	mCastingTime;

	/// 발사체 유무
	bool			mIsProjectile;
	/// 발사체 속도
	float			mProjectileSpeed;
	/// 발사체 효과위치
	NiPoint2		mProjectilePos;

	/// 분할 적용 데미지 정보를 읽어올 연출정보 포인트
	unsigned long	mDetailCnt;
	sDramaAction*	mpDetailBegin;
	sDramaAction*	mpDetailEnd;

	unsigned long	mDetailPos;

	unsigned long	mDetailStartTime;
	unsigned long	mDetailEndTime;

	/// 일반공격 속도 변경(기준:1.0)
	float			mSpeedFactor;

	unsigned long	mDramaKey;
	///스크립트 상의 스킬인덱스
	unsigned int	mSkillIdx;

	cHashSet		mSendAttackerSet;
	cHashSet		mSendTargetSet;
	cHashSet		mSendAroundSet;

	unsigned char	mAttackerDie;		/// 0 : 죽지않음, 1 : 죽음(객체만지움), 2 : 죽음(데미지miss)

	/// 발사체가 추적하던 타겟이 사라져 데미지 미스처리
	bool			mNoTargetNoDamage;

	/// 플레이어가 몬스터로 변신후 사용한 스킬인지 기록
	bool			mIsChgMonsSkill;

	/// 범위 방식
	eBOUNDTYPE		boundType;
	/// 적용 타겟
	eAPPLYTYPE		applyType;
	/// 적용 거리
	float			range;

	/// 물리/마법 속성
	eATTRIBUTETYPE	mAttributeType;

	eSKILLCREATE_TYPE mCreateType;
	
	unsigned long	mUseItemIdx;

	long			mDistressPoint;

	/// 타겟을 해제하는 스킬
	bool			mDelTarget;

	/// 타겟 목록
	sSkillTargetRoot	mTargetRoot;	

	/// 타겟 선정시 그리드에서 받아올 객체 고유번호
	cLongAry		mObjectIdxAry;

	unsigned short	mPushPull;
	unsigned long	mPushRange;

	unsigned short	mCashMapNum;
	float			mCashX;
	float			mCashY;

	cRangeCheck		mRangeCheck;

	bool			mIsChanneling;
	sDramaState*	mpChannelDramaState;
	unsigned long	mChannelingTime;
};
