/* ==========================================================================
* 파    일 : PlayerMonSkillObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-06-17
* 주의사항 : 플레이어변신 몬스터 스킬은 쿨타임 체크를 하지 않고 연출시간 체크만 한다!
*===========================================================================*/
#pragma once

#include "Skill_Common.h"
#include "BaseSkillObject.h"

class cBaseObject;

enum eOBJECTTYPE;

class cPlayerMonSkillObject : public cBaseSkillObject
{

public:

	cPlayerMonSkillObject();
	virtual ~cPlayerMonSkillObject();

	/// Skillmanager의 메모리풀에서 스킬 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 스킬을 제거해줌
	void operator delete( void* ptr, size_t n );

	bool			InitPlayerMonAttack( unsigned long playerIdx, unsigned long monsterClassIdx,	eMONSTERATTACK_TYPE attackType, 
		sObject target,	unsigned long uniqueIdx, bool casting, float speedFactor, unsigned long mMapNumber  );
	bool			InitPlayerMonPosition( unsigned long playerIdx, unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType,
		float targetXPos, float targetYPos, unsigned long uniqueIdx, bool casting, unsigned long mapNumber );

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

	/// 몬스터 종류인덱스
	unsigned long	mMonsterClassIdx;

	cObjectAry		mTempDelTargetAry;
};
