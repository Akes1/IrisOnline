/* ==========================================================================
* 파    일 : SkillObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-06-07
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Skill_Common.h"
#include "BaseSkillObject.h"
#include "RangeCheck.h"

class cBaseObject;

enum eOBJECTTYPE;

class cPlayerSkillObject : public cBaseSkillObject
{

public:

	cPlayerSkillObject();
	virtual ~cPlayerSkillObject();

	/// Skillmanager의 메모리풀에서 스킬 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 스킬을 제거해줌
	void operator delete( void* ptr, size_t n );

	unsigned char	InitPlayerNormalAttack( unsigned long playerIdx, sObject target, unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx );
	bool			InitPlayerSkill( unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx, cPlayer* pAttacker, cBaseObject* pTarget, unsigned long castingTime );
	bool			InitPlayerSkillPosition( unsigned long playerIdx, float targetXPos, float targetYPos, unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx, bool casting );
	bool			InitPlayerMapChange( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long useItemDBIdx, unsigned long castingTime );
	bool			InitPlayerVehicle( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long useItemIdx, unsigned long castingTime );
	bool			InitPlayerBlink( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker,
		NiPoint2 targetPos );
	bool			InitCashMapChange( unsigned long skillIdx, unsigned long uniqueIdx, cPlayer* pAttacker, unsigned long castingTime, 
		unsigned short mapNum, float x, float y, unsigned long useItemIdx );
	bool			InitRushAfterSkill( unsigned long skillIdx, unsigned char skillStep, unsigned long uniqueIdx, cPlayer* pAttacker, cBaseObject* pTarget, unsigned long castingTime );

	unsigned long	GetSkillIdx()		{ return mSkillIdx; }
	unsigned char	GetSkillSetp()		{ return mSkillStep; }

private:

	/// 캐스팅시 공격자 / 타겟 객체 체크
	bool			CastingObjectCheck( cBaseObject* pAttacker );

	/// 스킬 발동시 소모항목 소모
	bool			RequireValue( cBaseObject* pAttacker );

	/// 발사체 목적지 설정
	bool			SetCenterPos();

	/// 타겟 선택
	void			TargetSelect( cBaseObject* pAttacker );

	/// 데미지 적용
	void			ApplyDamage( cBaseObject* pAttacker );

protected:

	///스크립트 상의 스킬단계
	unsigned char	mSkillStep;

	cRangeCheck		mRangeCheck;
	cObjectAry		mTempDelTargetAry;
};
