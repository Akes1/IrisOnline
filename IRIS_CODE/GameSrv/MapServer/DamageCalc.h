/* ==========================================================================
* 파    일 : DamageCalc.h
* 목    적 : 데미지를 산출한다.
* 작 성 자 : 박경희
* 작 성 일 : 2007-01-16
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Status_Common.h"
#include "Skill_Common.h"

class cBaseObject;
class cPlayer;
class cMonster;

enum eOBJECTTYPE;

struct sObject;

const float CRITICAL_RATE = 1.5f;
const unsigned long DEFENSE_RANDOM_PER = 80;
const unsigned long HEAL_RANDOM_PER = 90;

class cDamageCalc
{

public:

	static cDamageCalc* mpDamageCalc;

public:
	cDamageCalc(void);
	~cDamageCalc(void);

public:

	/// 공격 성공 확률
	unsigned long		PhyAttackSuccess100( unsigned long attackerStatusRate, unsigned long targetStatusAvoid, long skillRate, 
											 unsigned char attackerType, unsigned char attackerLv, unsigned char targetLv,
											 unsigned long instantSuccess, unsigned long instantSuccessPer );
	
	unsigned long		MagAttackSuccess130( unsigned long attackerStatusRate, unsigned long targetStatusAvoid, long skillRate, 
											 unsigned char attackerType, unsigned char attackerLv, unsigned char targetLv,
											 unsigned long instantSuccess, unsigned long instantSuccessPer );

	/// 크리티컬 발생 확율
	unsigned long		PhyCritical200( unsigned long attackerStatusCri, long skillCri, unsigned char attackerLv, unsigned char targetLv,
		long attackerPer, long attackerItemPer, long attackerPlus, long attackerItemPlus, float targetCriDef,
		unsigned long instantCri, unsigned long instantCriPer );
	unsigned long		MagCritical230( unsigned long attackerStatusCri, long skillCri, unsigned char attackerLv, unsigned char targetLv,
		long attackerPer, long attackerItemPer, long attackerPlus, long attackerItemPlus, float targetCriDef,
		unsigned long instantCri, unsigned long instantCriPer );
	unsigned long		MonsterCritical260( long skillCri, unsigned char attackerLv, unsigned char targetLv,
		long attackerPer, long attackerPlus, float targetCriDef );

	/// 최종 데미지
	unsigned long		PhyDamage500( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
		unsigned long attackerStatusPhyAttack, long skillAttackPlus, long skillAttackPer, unsigned long targetStatusPhyDef, float addTotalDamage, float addPerTotalDamage );

	unsigned long		MagDamage530( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
		unsigned long attackerStatusMagAttack, long skillAttackPlus, long skillAttackPer, unsigned long targetStatusMagDef, float addTotalDamage, float addPerTotalDamage );

	/// 도트데미지
	unsigned long		PhyDotDamage600( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
		unsigned long attackerStatusPhyAttack, long infAttackPlus, long infAttackPer, unsigned long targetStatusPhyDef, unsigned long dotTotalTime, unsigned long dotTime );
	unsigned long		MagDotDamage630( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
		unsigned long attackerStatusMagAttack, long infAttackPlus, long infAttackPer, unsigned long targetStatusMagDef, unsigned long dotTotalTime, unsigned long dotTime );

	/// 상태 성공 확률
	unsigned long		PhyConditionOdd900( long skillRate, unsigned char attackerLv, unsigned char targetLv );
	unsigned long		MagConditionOdd930( long skillRate, unsigned char attackerLv, unsigned char targetLv );

	/// 회복(힐)
	unsigned long		HealHP1200( long statusHeal, long skillHealPlus, long skillHealPer, long targetHealPlus, long targetHealPer, long targetItemPlus,
		long targetItemPer );

	/// 회복 도트
	unsigned long		DotHealHP1230( long statusHeal, long infHealPlus, long infHealPer, unsigned long dotTotalTime, unsigned long dotTime );
};

#define DAMAGECALC cDamageCalc::mpDamageCalc
