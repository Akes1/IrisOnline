/* ==========================================================================
* 파    일 : Totem.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-10-21
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Totem_Common.h"
#include "BaseObject.h"
#include "PVP_Common.h"
#include "RangeCheck.h"

class cBaseObject;

class cTotem : public cBaseObject
{

public:
	/// 생성소멸
	cTotem( void );
	~cTotem( void );

	/// 메모리 할당 제거 pool 사용
	void* operator new ( size_t n ); 
	void operator delete ( void* ptr, size_t n );

	bool			TotemInit( sObject attacker, unsigned long objectIdx, unsigned long classIdx, unsigned long partyIdx,
							   unsigned long partyUnionIdx, NiPoint2 pos, eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType,
							   unsigned long skillIdx, float direction, unsigned char applyCnt );
	bool			PvPTotemInit( ePVPDM_TEAM_TYPE teamType, unsigned long objectIdx, unsigned long classIdx,
		unsigned short pvpIdx, NiPoint2 pos, eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType );

	void			ProcessTotem( unsigned long elapsedTime, unsigned long accumTime );

	unsigned long	GetClassIdx()	{ return mpTotemScript->mTotemIdx; }

	sObject			GetAttacker()	{ return mAttacker; }
	unsigned short	GetRange()	{ return mpTotemScript->mApplyRange1; }

	bool			GetStateDie( ) { return false; }

	/// 오라 타겟 보유목록제거
	void			EraseTarget( sObject taraget );
	float			GetDirection()	{ return mDirection; }

private:

	bool			CheckTotem( unsigned long accumTime );

	void			PlayerSelectTarget();		/// 플레이어공격자의 타겟 선정
	void			MonsterSelectTarget();		/// 몬스터공격자의 타겟 선정

	bool			RangeCheck( cBaseObject* pTarget );

	/// 기준팀의 동료와 적 구분을해 버프/디버프를 걸어준다.
	void			PvPAddInf( ePVPDM_TEAM_TYPE mainTeamType, eAPPLYTYPE applyType );

	bool			CopyTotemInfluence( cBaseObject* pTarget, sInfluenceCreate* pInf = NULL );		/// 효과 만들기

private:

	typedef tHashSet<unsigned long> cHashSet;
	typedef tArray<unsigned long> cArray;

	cHashSet		mPlayerTargetSet;
	cHashSet		mMonsterTargetSet;

	/// 스크립트 정보
	const sTotemScript* mpTotemScript;
	const sInfluenceScript* mpInfScript;

	sObject			mAttacker;				/// 공격자
	unsigned long	mPartyIdx;				/// 공격자의 파티
	unsigned long	mPartyUnionIdx;			/// 공격자의 파티연합 인덱스

	unsigned long	mNextCheckTime;			/// 다음 검색 시간
	unsigned long	mEndTime;				/// 객제 삭제 시간

	cRangeCheck		mRangeCheck;
	eAPPLYTYPE		mApplytype;
	eATTRIBUTETYPE	mAttriButeType;

	float			mDirection;				/// 방향

	ePVPDM_TEAM_TYPE mTeamType;				/// pvp용 시스템 토템설정
	unsigned long	mSkillIdx;

	/// 한프레임 최대 타겟 선정 카운트
	unsigned char	mTargetCnt;	
	unsigned char	mTargetCntMax;

	cArray			mObjectIdxAry;

public:
	bool           SendSightIn        ( char category, char protocol, unsigned long connectionIdx );
	bool           SendSightOut       ( char category, char protocol, unsigned long connectionIdx );
};

