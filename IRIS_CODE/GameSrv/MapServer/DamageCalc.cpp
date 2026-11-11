#include "StdAfx.h"
#include "damagecalc.h"
#include "StatusScript.h"
#include "BaseObject.h"
#include "ObjectManager.h"
#include "Player.h"
#include "Monster.h"
#include "Skill_Common.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "MonsterScript.h"
#include "GameSrv.h"
#include "RandomTable.h"



cDamageCalc* cDamageCalc::mpDamageCalc = NULL;



cDamageCalc::cDamageCalc(void)
{
	/// 싱글톤 - 2번이상 객체를 생성 하려고 할경우 콘솔에 에러 찍어줌
	if( mpDamageCalc )
	{
		//CONSOLE->WriteLog( "mpDamageCalc new error");
	}
	else
	{
		mpDamageCalc = this;
	}
}



cDamageCalc::~cDamageCalc(void)
{
}



unsigned long cDamageCalc::PhyAttackSuccess100( unsigned long attackerStatusRate, unsigned long targetStatusAvoid, long skillRate, 
												unsigned char attackerType, unsigned char attackerLv, unsigned char targetLv,
												unsigned long instantSuccess, unsigned long instantSuccessPer )
{
	float num104 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 104 );
	float num105 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 105 );
	float num106 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 106 );
	float num107 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 107 );
	float num108 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 108 );
	float num109 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 109 );
	float num110 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 110 );
	float num111 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 111 );
	float num112 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 112 );

	// 공격 성공 확률(물리)
	// A =	{num101} + (( [공격자] 캐릭터 스탯 명중 수치) * ((100 + [공격자]아이템 명중 수치[%]값 + [공격자]버프성 명중 수치[%]값) / 100) + [공격자]아이템 명중 수치[+-]값 + [공격자]버프성 명중 수치[+-]값 ) * {num102}
	// B =	[공격자]스킬 성공확률값
	// C =	(( [방어자] 캐릭터 스탯 회피 수치) * ((100 + [방어자]아이템 회피 수치[%]값  + [방어자]버프성 회피 수치[%]값 ) / 100) + [방어자]아이템 회피 수치[+-]값 + [방어자]버프성 회피 수치[+-]값 ) * {num103}
	//[공격자]캐릭터 [공격자] Lv < [방어자] Lv   D =  - ((( [방어자] Lv - [공격자] Lv ) * {num104} ) ^ {num105} )) 	최소 {num106}
	//[공격자]몬스터 [공격자] Lv < [방어자] Lv   D =  - ((( [방어자] Lv - [공격자] Lv ) * {num107} ) ^ {num108} )) 	최소 {num109}
	//[공격자]공통 [공격자] Lv >= [방어자] Lv    D =  (( [공격자] Lv - [방어자] Lv ) * {num110} )	최대 {num111}
	// E =	( ( ( A + B ) - C ) * ( 1 + [공격자]공격 성공 확률 순간효과[%]값 / 100 )  + [공격자]공격 성공 확률 순간효과[+-]값 * {num112} ) + D	
	float valueA = (float)attackerStatusRate;		/// [A]는 스텟에서 이미 계산된 값이다
	float valueB = (float)skillRate;
	float valueC = (float)targetStatusAvoid;		/// [C]는 스텟에서 이미 계산된 값이다
	float valueD = 0.0f;
	float valueE = 0.0f;
	
	/// 정의 되지 않은 공격자 타입 
	if( attackerType != eOBJECTTYPE_PLAYER && attackerType != eOBJECTTYPE_MONSTER )
	{
		NETWORK2->PostServerEvent("cDamageCalc::PhyAttackSuccess100 attackerType[%d] ERROR", attackerType );
		return 0;
	}
	
	/// 공격자 레벨이 낮은 경우 공격자별 계산을 다르게 처리한다.
	if( attackerLv < targetLv && attackerType == eOBJECTTYPE_PLAYER )
	{
		// [공격자]캐릭터 [공격자] Lv < [방어자] Lv
		// D =  - ((( [방어자] Lv - [공격자] Lv ) * {num104} ) ^ {num105} )) 	최소 {num106}
		valueD = -( pow((((float)targetLv - attackerLv) * num104 ), num105));
		if( valueD < num106 )
			valueD = num106;
	}
	/// 공격자 레벨이 낮은 경우 공격자별 계산을 다르게 처리한다.
	else if( attackerLv < targetLv && attackerType == eOBJECTTYPE_MONSTER )
	{
		// [공격자]몬스터 [공격자] Lv < [방어자] Lv
		// D =  -((( [방어자] Lv - [공격자] Lv ) * {num107} ) ^ {num108} )) 	최소 {num109}
		valueD = -( pow((((float)targetLv - attackerLv) * num107 ), num108 ));
		if( valueD < num109 )
			valueD = num109;
	}
	else
	{
		// [공격자]공통 [공격자] Lv >= [방어자] Lv
		// D =  (( [공격자] Lv - [방어자] Lv ) * {num110} )	최대 {num111}
		valueD = (((float)attackerLv - targetLv) * num110 );
		if( valueD > num111 )
			valueD = num111;
	}

	// E =	( ( ( A + B ) - C )	 * ( 1 + [공격자]공격 성공 확률 순간효과[%]값 / 100 )  + [공격자]공격 성공 확률 순간효과[+-]값 * {num112} ) + D	
	valueE = ((( valueA + valueB ) - valueC ) * ( 1.0f + (float)instantSuccessPer / 100.0f ) + instantSuccess * num112 ) + valueD;

	if( g_eventPrint == true )
		Verbose->WriteLog("PhyAttackSuccess attackerLv[%d] E[%.2f]", attackerLv, valueE );

	/// 공격 성공 확률의 최대값은 100%, 최소값은 35%이다.
	if( valueE < 35.0f )
		valueE = 35.0f;

	if( valueE > 100.0f )
		valueE = 100.0f;	/// 적용항목에 따라 100보다 큰수치가 나올 수 있으나 확률체크라 100보다 커도 100이랑 다를바가 없다.

	return (unsigned long)valueE;
}



unsigned long cDamageCalc::MagAttackSuccess130( unsigned long attackerStatusRate, unsigned long targetStatusAvoid, long skillRate, 
											    unsigned char attackerType, unsigned char attackerLv, unsigned char targetLv,
												unsigned long instantSuccess, unsigned long instantSuccessPer )
{
	float num134 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 134 );
	float num135 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 135 );
	float num136 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 136 );
	float num137 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 137 );
	float num138 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 138 );
	float num139 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 139 );
	float num140 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 140 );
	float num141 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 141 );
	float num142 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 142 );

	// 공격 성공 확률(마법)
	// A =	{num131} + (( [공격자] 캐릭터 스탯 마법 명중 수치) * ((100  + [공격자]아이템 마법 명중 수치[%]값 + [공격자]버프성 마법 명중 수치[%]값) / 100)  + [공격자]아이템 마법 명중 수치[+-]값 + [공격자]버프성 마법 명중 수치[+-]값 ) * {num132}
	// B =	[공격자]스킬 성공확률값
	// C =  (( [방어자] 캐릭터 스탯 저항력 수치 ) * ((100 + [방어자]아이템 저항력 수치[%]값 + [방어자]버프성 저항력 수치[%]값) / 100) + [방어자]아이템 저항력 수치[+-]값 + [방어자]버프성 저항력 수치[+-]값 ) * {num133}
	//[공격자]캐릭터 [공격자] Lv < [방어자] Lv   D =  - ((( [방어자] Lv - [공격자] Lv ) * {num134} ) ^ {num135} )) 	최소 {num136}
	//[공격자]몬스터 [공격자] Lv < [방어자] Lv   D =  - ((( [방어자] Lv - [공격자] Lv ) * {num137} ) ^ {num138} )) 	최소 {num139}
	//[공격자]공통 [공격자] Lv >= [방어자] Lv    D =  (( [공격자] Lv - [방어자] Lv ) * {num140} )	최대 {num141}
	// E =	( ( ( A + B ) - C ) * ( 1 + [공격자]공격 성공 확률 순간효과[%]값 / 100)  + [공격자]공격 성공 확률 순간효과[+-]값 * {num142} ) + D
	float valueA = (float)attackerStatusRate;		/// [A]는 스텟에서 이미 계산된 값이다
	float valueB = (float)skillRate;
	float valueC = (float)targetStatusAvoid;		/// [C]는 스텟에서 이미 계산된 값이다
	float valueD = 0.0f;
	float valueE = 0.0f;

	/// 정의 되지 않은 공격자 타입 
	if( attackerType != eOBJECTTYPE_PLAYER && attackerType != eOBJECTTYPE_MONSTER )
	{
		NETWORK2->PostServerEvent("cDamageCalc::MagAttackSuccess130 attackerType[%d] ERROR", attackerType );
		return 0;
	}

	/// 공격자 레벨이 낮은 경우 공격자별 계산을 다르게 처리한다.
	if( attackerLv < targetLv && attackerType == eOBJECTTYPE_PLAYER )
	{
		// [공격자]캐릭터 [공격자] Lv < [방어자] Lv   
		// D =  - ((( [방어자] Lv - [공격자] Lv ) * {num134} ) ^ {num135} )) 	최소 {num136}
		valueD = -( pow((((float)targetLv - attackerLv) * num134 ), num135)) / 100.0f;
		if( valueD < num136 )
			valueD = num136;
	}
	/// 공격자 레벨이 낮은 경우 공격자별 계산을 다르게 처리한다.
	else if( attackerLv < targetLv && attackerType == eOBJECTTYPE_MONSTER )
	{
		// [공격자]몬스터 [공격자] Lv < [방어자] Lv   
		// D =  - ((( [방어자] Lv - [공격자] Lv ) * {num137} ) ^ {num138} )) 	최소 {num139}
		valueD = -( pow((((float)targetLv - attackerLv) * num137 ), num138 )) / 100.0f;
		if( valueD < num139 )
			valueD = num139;
	}
	else
	{
		// [공격자]공통 [공격자] Lv >= [방어자] Lv    
		// D =  (( [공격자] Lv - [방어자] Lv ) * {num140} )	최대 {num141}
		valueD = (((float)attackerLv - targetLv) * num140 );
		if( valueD > num141 )
			valueD = num141;
	}

	// E =	( ( ( A + B ) - C ) * ( 1 + [공격자]공격 성공 확률 순간효과[%]값 / 100)  + [공격자]공격 성공 확률 순간효과[+-]값 * {num142} ) + D
	valueE = ((( valueA + valueB ) - valueC ) * ( 1.0f + instantSuccessPer / 100.0f ) + instantSuccess * num142 ) + valueD;

	if( g_eventPrint == true )
		Verbose->WriteLog("MagAttackSuccess attackerLv[%d] E[%.2f]", attackerLv, valueE );

	/// 공격 성공 확률의 최대값은 100%, 최소값은 35%이다.
	if( valueE < 35.0f )
		valueE = 35.0f;

	if( valueE > 100.0f )
		valueE = 100.0f;	/// 적용항목에 따라 100보다 큰수치가 나올 수 있으나 확률체크라 100보다 커도 100이랑 다를바가 없다.

	return (unsigned long)valueE;
}


unsigned long cDamageCalc::PhyCritical200( unsigned long attackerStatusCri, long skillCri, unsigned char attackerLv, unsigned char targetLv,
										  long attackerPer, long attackerItemPer, long attackerPlus, long attackerItemPlus, float targetCriDef,
										  unsigned long instantCri, unsigned long instantCriPer )
{
	float num201 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 201 );
	float num202 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 202 );
	float num205 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 205 );
	float num206 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 206 );
	float num207 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 207 );
	float num208 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 208 );
	float num209 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 209 );
	float num210 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 210 );
	float num211 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 211 );

	// 크리티컬 발생 확률(물리)
	//A =	[공격자] 캐릭터 스탯 물리 크리티컬 수치 * {num201}	
	//B =	A * ((100 +  [공격자]아이템 물리 크리티컬[%]값 + [공격자]버프성 물리 크리티컬 [%]값)  / 100) + ( ([공격자]아이템 물리 크리티컬 수치[+-]값 + [공격자]버프성 물리 크리티컬 수치[+-]값) * {num202} )	
	//C =	스킬 기본값
	//몬스터의 경우				
	//D =	A
	//캐릭터의 경우				
	//D =	[방어자] 캐릭터 스탯 물리 방호 수치 * {num203}
	//[E] =	D * ((100 +  [방어자]아이템 물리 방호[%]값 + [방어자]버프성 물리 방호 [%]값)  / 100) + ( ([방어자]아이템 물리 방호 수치[+-]값 + [방어자]버프성 물리 방호 수치[+-]값) * {num204} )	
	//[방어자]Lv>=[공격자]Lv		
	//F =	(100 - (([방어자]Lv - [공격자]Lv) * {num205}) ^ {num206}) /100	최소 {num207}
	//[방어자]Lv<[공격자]Lv		
	//F =	({num208} + ([공격자] Lv - [방어자] Lv) / {num209})	최대 {num210}
	//G =	(B + C - E) * F  * (1 + [공격자]크리티컬 순간효과[%]값 / 100)  + ([공격자]크리티컬 순간효과[+-]값 * {num211})

	//A =	[공격자] 캐릭터 스탯 물리 크리티컬 수치 * {num201}	
	float valueA = (float)attackerStatusCri * num201;
	//B =	A * ((100 +  [공격자]아이템 물리 크리티컬[%]값 + [공격자]버프성 물리 크리티컬 [%]값)  / 100) + ( ([공격자]아이템 물리 크리티컬 수치[+-]값 + [공격자]버프성 물리 크리티컬 수치[+-]값) * {num202} )	
	float valueB = valueA * (( 100.0f + attackerItemPer + attackerPer ) / 100.0f ) + (( attackerItemPlus + attackerPlus ) * num202 );
	//C =	스킬 기본값	
	float valueC = (float)skillCri;
	//D =	[방어자] 캐릭터 스탯 물리 방호 수치 * {num203}	
	//[E] =	D * ((100 +  [방어자]아이템 물리 방호[%]값 + [방어자]버프성 물리 방호 [%]값)  / 100) + ( ([방어자]아이템 물리 방호 수치[+-]값 + [방어자]버프성 물리 방호 수치[+-]값) * {num204} )	
	float valueE = targetCriDef;
	float valueF = 0;
	if( targetLv >= attackerLv )
	{
		//[방어자]Lv>=[공격자]Lv		
		//F =	(100 - (([방어자]Lv - [공격자]Lv) * {num205}) ^ {num206}) /100	최소 {num207}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num205 ), num206 )) / 100.0f;
		if( valueF < num207 )
		{
			valueF = num207;
		}
	}
	else
	{
		//[방어자]Lv<[공격자]Lv		
		//F =	({num208} + ([공격자] Lv - [방어자] Lv) / {num209})	최대 {num210}
		valueF = ( num208 + (float)( attackerLv - targetLv ) / num209 );
		if( valueF > num210 )
		{
			valueF = num210;
		}		
	}
	//G =	(B + C - E) * F  * (1 + [공격자]크리티컬 순간효과[%]값 / 100)  + ([공격자]크리티컬 순간효과[+-]값 * {num211})
	float valueG = ( valueB + valueC - valueE ) * valueF * ( 1.0f + instantCriPer / 100.0f ) + ( instantCri * num211 ) ;

	if( g_eventPrint == true )
		Verbose->WriteLog("PhyCritical attackerLv[%d] G[%.2f]", attackerLv, valueG );

	if( valueG < 0.0f )
		valueG = 0.0f;

	return FloatToInt( valueG );
}


unsigned long cDamageCalc::MagCritical230( unsigned long attackerStatusCri, long skillCri, unsigned char attackerLv, unsigned char targetLv,
										  long attackerPer, long attackerItemPer, long attackerPlus, long attackerItemPlus, float targetCriDef,
										  unsigned long instantCri, unsigned long instantCriPer )
{
	float num231 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 231 );
	float num232 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 232 );
	float num235 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 235 );
	float num236 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 236 );
	float num237 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 237 );
	float num238 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 238 );
	float num239 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 239 );
	float num240 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 240 );
	float num241 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 241 );

	// 크리티컬 발생 확률(마법)
	//A =	[공격자] 캐릭터 스탯 마법 크리티컬 수치 * {num231}
	//B =	A  * ((100 + [공격자]아이템 마법 크리티컬[%]값 + [공격자]버프성 마법 크리티컬 [%]값)  / 100) + (([공격자]아이템 마법 크리티컬 수치[+-]값 + [공격자]버프성 마법 크리티컬 수치[+-]값) * {num232} )	
	//C =	스킬 기본값	
	//몬스터의 경우				
	//D =	A
	//캐릭터의 경우				
	//D =	[방어자] 캐릭터 스탯 마법 방호 수치 * {num233}	
	//[E] =	D * ((100 +  [방어자]아이템 마법 방호[%]값 + [방어자]버프성 마법 방호 [%]값)  / 100) + ( ([방어자]아이템 마법 방호 수치[+-]값 + [방어자]버프성 마법 방호 수치[+-]값) * {num234} )		
	//[방어자]Lv>=[공격자]Lv		
	//F =	(100 - (([방어자]Lv - [공격자]Lv) * {num235}) ^ {num236}) /100	최소 {num237}
	//[방어자]Lv<[공격자]Lv		
	//F =	({num238} + ([공격자] Lv - [방어자] Lv) / {num239})	최대 {num240}
	//G =	(B + C - E) * F  * (1 + [공격자]크리티컬 순간효과[%]값 / 100)  + ([공격자]크리티컬 순간효과[+-]값 * {num241})

	//A =	[공격자] 캐릭터 스탯 마법 크리티컬 수치 * {num231}
	float valueA = (float)attackerStatusCri * num231;
	//B =	A  * ((100 + [공격자]아이템 마법 크리티컬[%]값 + [공격자]버프성 마법 크리티컬 [%]값)  / 100) + (([공격자]아이템 마법 크리티컬 수치[+-]값 + [공격자]버프성 마법 크리티컬 수치[+-]값) * {num232} )	
	float valueB = valueA * (( 100.0f + attackerItemPer + attackerPer ) / 100.0f ) + ((float)( attackerItemPlus + attackerPlus ) * num232 );
	//C =	스킬 기본값		
	float valueC = (float)skillCri;
	//D =	[방어자] 캐릭터 스탯 마법 방호 수치 * {num233}	
	//[E] =	D * ((100 +  [방어자]아이템 마법 방호[%]값 + [방어자]버프성 마법 방호 [%]값)  / 100) + ( ([방어자]아이템 마법 방호 수치[+-]값 + [방어자]버프성 마법 방호 수치[+-]값) * {num234} )		
	float valueE = targetCriDef;
	float valueF = 0;
	if( targetLv >= attackerLv )
	{
		//[방어자]Lv>=[공격자]Lv		
		//F =	(100 - (([방어자]Lv - [공격자]Lv) * {num235}) ^ {num236}) /100	최소 {num237}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num235 ), num236 )) / 100.0f;
		if( valueF < num237 )
		{
			valueF = num237;
		}
	}
	else
	{
		//[방어자]Lv<[공격자]Lv		
		//F =	({num238} + ([공격자] Lv - [방어자] Lv) / {num239})	최대 {num240}
		valueF = ( num238 + (float)( attackerLv - targetLv ) / num239 );
		if( valueF < num240 )
		{
			valueF = num240;
		}		
	}
	//G =	(B + C - E) * F  * (1 + [공격자]크리티컬 순간효과[%]값 / 100)  + ([공격자]크리티컬 순간효과[+-]값 * {num241})
	float valueG = ( valueB + valueC - valueE ) * valueF * ( 1.0f + instantCriPer / 100.0f ) + ( instantCri * num241 ) ;

	if( g_eventPrint == true )
		Verbose->WriteLog("MagCritical attackerLv[%d] G[%.2f]", attackerLv, valueG );

	if( valueG < 0.0f )
		valueG = 0.0f;

	return FloatToInt( valueG );
}


unsigned long cDamageCalc::MonsterCritical260( long skillCri, unsigned char attackerLv, unsigned char targetLv,
											  long attackerPer, long attackerPlus, float targetCriDef )
{
	float num261 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 261 );
	float num264 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 264 );
	float num265 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 265 );
	float num266 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 266 );
	float num267 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 267 );
	float num268 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 268 );
	float num269 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 269 );

	// 크리티컬 발생 확률(몬스터)
	//A =	스킬 기본값						
	//B =	A  * ((100 + [공격자]버프성 크리티컬 [%]값) / 100) + ( [공격자]버프성 크리티컬 수치[+-]값 * {num261} )						
	//C =	[방어자] 캐릭터 스탯 방호 수치 * {num262}	
	//D =	C * ((100 +  [방어자]아이템 방호[%]값 + [방어자]버프성 방호 [%]값)  / 100) + ( ([방어자]아이템 방호 수치[+-]값 + [방어자]버프성 방호 수치[+-]값) * {num263} )
	//[방어자]Lv>=[공격자]Lv							
	//E =	(100 - (([방어자]Lv - [공격자]Lv) * {num264}) ^ {num265}) /100		최소 {num266}
	//[방어자]Lv<[공격자]Lv							
	//E =	({num267} + ([공격자] Lv - [방어자] Lv) / {num268})		최대 {num269}
	//F =	(B - D) * E	

	//A =	스킬 기본값
	float valueA = (float)skillCri;
	//B =	A  * ((100 + [공격자]버프성 크리티컬 [%]값) / 100) + ( [공격자]버프성 크리티컬 수치[+-]값 * {num261} )						
	float valueB = valueA * (( 100.0f + attackerPer ) / 100.0f ) + ( attackerPlus * num261 );
	//C =	[방어자] 캐릭터 스탯 방호 수치 * {num262}	
	//D =	C * ((100 +  [방어자]아이템 방호[%]값 + [방어자]버프성 방호 [%]값)  / 100) + ( ([방어자]아이템 방호 수치[+-]값 + [방어자]버프성 방호 수치[+-]값) * {num263} )
	float valueD = targetCriDef;
	float valueE = 0;
	if( targetLv >= attackerLv )
	{
		//[방어자]Lv>=[공격자]Lv							
		//E =	(100 - (([방어자]Lv - [공격자]Lv) * {num264}) ^ {num265}) /100		최소 {num266}
		valueE = ( 100.0f - pow((( targetLv - attackerLv ) * num264 ), num265 )) / 100.0f;
		if( valueE < num266 )
		{
			valueE = num266;
		}
	}
	else
	{
		//[방어자]Lv<[공격자]Lv							
		//E =	({num267} + ([공격자] Lv - [방어자] Lv) / {num268})		최대 {num269}
		valueE = ( num267 + (float)( attackerLv - targetLv ) / num268 );
		if( valueE < num269 )
		{
			valueE = num269;
		}		
	}

	//F =	(B - D) * E	
	float valueF = ( valueB - valueD ) * valueE;

	if( g_eventPrint == true )
		Verbose->WriteLog("MonsterCritical attackerLv[%d] F[%.2f]", attackerLv, valueF );

	if( valueF < 0.0f )
		valueF = 0.0f;

	return FloatToInt( valueF );
}


unsigned long cDamageCalc::PhyDamage500( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv, unsigned long attackerStatusPhyAttack,
										long skillAttackPlus, long skillAttackPer, unsigned long targetStatusPhyDef, float addTotalDamage, float addPerTotalDamage )
{
	float num501 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 501 );
	float num502 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 502 );
	float num503 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 503 );
	float num504 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 504 );
	float num505 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 505 );
	float num506 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 506 );
	float num507 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 507 );
	float num508 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 508 );
	float num509 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 509 );
	float num510 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 510 );
	float num511 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 511 );
	float num512 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 512 );
	float num513 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 513 );
	float num514 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 514 );
	float num515 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 515 );
	
	// 최종대미지(물리)
	// A =	[공격자] 물리 공격력	
	// B =  [방어자] 물리 방어력 * {num501}
	// C =	{num502} - (( {num503} * [방어자] 물리 방어력) / ({num504} * ([방어자] 물리 방어력 ^ {num505}) + {num506} + ( {num507} * [공격자] Lv) - ( {num508} * [방어자] Lv)))
	// D =	[공격자] 스킬 공격력(+)		
	// E =	(100 + [공격자] 스킬 공격력(%)) / 100
	//[공격자] Lv(유저) < [방어자] Lv(유저)
	// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num509}) ^ {num510}) / 100	최소 {num511}
	//[공격자] Lv(유저) < [방어자] Lv(몬스터)
	// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num512}) ^ {num513}) / 100	최소 {num514}
	//그 외의 모든 경우
	// F =  {num515}
	// G =  (((A * E) + D) * C) * F
	// H =  몬스터 종류에 따른 추가대미지 [%]값 + 몬스터 분류에 따른 추가 대미지[%]값
	// I =  몬스터 종류에 따른 추가대미지 [+-]값 + 몬스터 분류에 따른 추가 대미지[+-]값
	// J =  G * H + I

	// A =	[공격자] 물리 공격력	
	float valueA = (float)attackerStatusPhyAttack;
	// B =  [방어자] 물리 방어력 * {num501}
	float valueB = (float)targetStatusPhyDef * num501;
	// C =	{num502} - (( {num503} * [방어자] 물리 방어력) / ({num504} * ([방어자] 물리 방어력 ^ {num505}) + {num506} + ( {num507} * [공격자] Lv) - ( {num508} * [방어자] Lv)))
	float valueC = num502 - ( ( num503 * valueB ) / ( num504 * ( pow( valueB, num505) ) + num506 + ( num507 * attackerLv ) - ( num508 * targetLv )));
	// D =	[공격자] 스킬 공격력(+)		
	float valueD = (float)skillAttackPlus;
	// E =	(100 + [공격자] 스킬 공격력(%)) / 100
	float valueE = (float)( 100.0f + skillAttackPer ) / 100.0f;
	//그 외의 모든 경우
	// F =  {num515}
	float valueF = num515;
	if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_PLAYER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(유저)
		// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num509}) ^ {num510}) / 100	최소 {num511}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num509 ), num510 )) / 100.0f;
		if( valueF < num511 )
		{
			valueF = num511;
		}
	}
	else if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_MONSTER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(몬스터)
		// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num512}) ^ {num513}) / 100	최소 {num514}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num512 ), num513 )) / 100.0f;
		if( valueF < num514 )
		{
			valueF = num514;
		}
	}

	// G =  (((A * E) + D) * C) * F	
	float valueG = ((( valueA * valueE ) + valueD ) * valueC ) * valueF;
	
	float valueH = addPerTotalDamage;
	float valueI = addTotalDamage;
	float valueJ = valueG * valueH + valueI;

	if( g_eventPrint == true )
		Verbose->WriteLog("PhyDamage attackerLv[%d] G[%.2f]", attackerLv, valueG );

	/// 데미지 값은 1보다 작을수 없다.
	return valueG > 1.0f ? FloatToInt( valueJ ) : 1;
}


unsigned long cDamageCalc::MagDamage530( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv, unsigned long attackerStatusMagAttack, 
										long skillAttackPlus, long skillAttackPer, unsigned long targetStatusMagDef, float addTotalDamage, float addPerTotalDamage )
{

	float num531 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 531 );
	float num532 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 532 );
	float num533 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 533 );
	float num534 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 534 );
	float num535 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 535 );
	float num536 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 536 );
	float num537 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 537 );
	float num538 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 538 );
	float num539 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 539 );
	float num540 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 540 );
	float num541 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 541 );
	float num542 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 542 );
	float num543 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 543 );
	float num544 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 544 );
	float num545 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 545 );

	// 최종대미지(마법)
	// A =	[공격자] 마법 공격력	
	// B =  [방어자] 마법 방어력 * {num531}
	// C =	{num532} - (( {num533} * [방어자] 마법 방어력) / ({num534} * ([방어자] 마법 방어력 ^ {num535}) + {num536} + ( {num537} * [공격자] Lv) - ( {num538} * [방어자] Lv)))
	// D =	[공격자] 스킬 공격력(+)		
	// E =	(100 + [공격자] 스킬 공격력(%)) / 100
	//[공격자] Lv(유저) < [방어자] Lv(유저)
	// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num539}) ^ {num540}) / 100	최소 {num541}
	//[공격자] Lv(유저) < [방어자] Lv(몬스터)
	// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num542}) ^ {num543}) / 100	최소 {num544}
	//그 외의 모든 경우
	// F =  {num545}
	// G =  (((A * E) + D) * C) * F
	// H =  몬스터 종류에 따른 추가대미지 [%]값 + 몬스터 분류에 따른 추가 대미지[%]값
	// I =  몬스터 종류에 따른 추가대미지 [+-]값 + 몬스터 분류에 따른 추가 대미지[+-]값
	// J =  G * H + I

	// A =	[공격자] 마법 공격력	
	float valueA = (float)attackerStatusMagAttack;
	// B =  [방어자] 마법 방어력 * {num531}
	float valueB = (float)targetStatusMagDef * num531;
	// C =	{num532} - (( {num533} * [방어자] 마법 방어력) / ({num534} * ([방어자] 마법 방어력 ^ {num535}) + {num536} + ( {num537} * [공격자] Lv) - ( {num538} * [방어자] Lv)))
	float valueC = num532 - ( ( num533 * valueB ) / ( num534 * ( pow( valueB, num535) ) + num536 + ( num537 * attackerLv ) - ( num538 * targetLv )));
	// D =	[공격자] 스킬 공격력(+)			
	float valueD = (float)skillAttackPlus;
	// E =	(100 + [공격자] 스킬 공격력(%)) / 100
	float valueE = (float)( 100.0f + skillAttackPer ) / 100.0f;
	//그 외의 모든 경우
	// F =  {num545}
	float valueF = num545;
	if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_PLAYER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(유저)
		// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num539}) ^ {num540}) / 100	최소 {num541}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num539 ), num540 )) / 100.0f;
		if( valueF < num541 )
		{
			valueF = num541;
		}
	}
	else if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_MONSTER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(몬스터)
		// F =  (100 - (([방어자] Lv - [공격자] Lv) * {num542}) ^ {num543}) / 100	최소 {num544}
		valueF = ( 100.0f - pow((( targetLv - attackerLv ) * num542 ), num543 )) / 100.0f;
		if( valueF < num544 )
		{
			valueF = num544;
		}
	}

	// G =  (((A * E) + D) * C) * F	
	float valueG = ((( valueA * valueE ) + valueD ) * valueC ) * valueF;

	float valueH = addPerTotalDamage;
	float valueI = addTotalDamage;
	float valueJ = valueG * valueH + valueI;

	if( g_eventPrint == true )
		Verbose->WriteLog("MagDamage attackerLv[%d] G[%.2f]", attackerLv, valueG );

	/// 데미지 값은 1보다 작을수 없다.
	return valueG > 1.0f ? FloatToInt( valueJ ) : 1;
}


unsigned long cDamageCalc::PhyDotDamage600( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
										unsigned long attackerStatusPhyAttack, long infAttackPlus, long infAttackPer, unsigned long targetStatusPhyDef,
										unsigned long dotTotalTime, unsigned long dotTime )
{
	float num601 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 601 );
	float num602 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 602 );
	float num603 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 603 );
	float num604 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 604 );
	float num605 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 605 );
	float num606 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 606 );
	float num607 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 607 );
	float num608 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 608 );
	float num609 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 609 );
	float num610 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 610 );
	float num611 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 611 );
	float num612 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 612 );
	float num613 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 613 );
	float num614 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 614 );
	float num615 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 615 );

	// 도트 대미지(물리)
	// A =	[공격자] 물리 공격력	
	// B =  [방어자] 물리 방어력 * {num601}
	// C =	{num602} - (( {num603} * [방어자] 물리 방어력) / ({num604} * ([방어자] 물리 방어력 ^ {num605}) + {num606} + ( {num607} * [공격자] Lv) - ( {num608} * [방어자] Lv)))
	//[공격자] Lv(유저) < [방어자] Lv(유저)
	// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num609}) ^ {num610}) / 100	최소 {num611}
	//[공격자] Lv(유저) < [방어자] Lv(몬스터)
	// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num612}) ^ {num613}) / 100	최소 {num614}
	//그 외의 모든 경우
	// D =  {num615}
	// E =	[공격자] 효과 스킬 공격력(+)		
	// F =	(100 + [공격자] 효과 스킬 공격력(%)) / 100
	// G =  스킬 지속시간 / 스킬 틱시간
	// H =  ((A * F) * C / G + E) * D

	// A =	[공격자] 물리 공격력		
	float valueA = (float)attackerStatusPhyAttack;
	// B =  [방어자] 물리 방어력 * {num601}
	float valueB = (float)targetStatusPhyDef * num601;
	// C =	{num602} - (( {num603} * [방어자] 물리 방어력) / ({num604} * ([방어자] 물리 방어력 ^ {num605}) + {num606} + ( {num607} * [공격자] Lv) - ( {num608} * [방어자] Lv)))
	float valueC = num602 - ( ( num603 * valueB ) / ( num604 * ( pow( valueB, num605) ) + num606 + ( num607 * attackerLv ) - ( num608 * targetLv )));
	//그 외의 모든 경우
	// D =  {num615}
	float valueD = num615;
	if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_PLAYER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(유저)
		// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num609}) ^ {num610}) / 100	최소 {num611}
		valueD = ( 100.0f - pow((( targetLv - attackerLv ) * num609 ), num610 )) / 100.0f;
		if( valueD < num611 )
		{
			valueD = num611;
		}
	}
	else if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_MONSTER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(몬스터)
		// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num612}) ^ {num613}) / 100	최소 {num614}
		valueD = ( 100.0f - pow((( targetLv - attackerLv ) * num612 ), num613 )) / 100.0f;
		if( valueD < num614 )
		{
			valueD = num614;
		}
	}
	// E =	[공격자] 효과 스킬 공격력(+)		
	float valueE = (float)infAttackPlus;
	// F =	(100 + [공격자] 효과 스킬 공격력(%)) / 100
	float valueF = (float)( 100.0f + infAttackPer ) / 100.0f;
	// G =  효과 지속시간 / 효과 틱시간
	float valueG = 1;
	if( dotTime == 0 || dotTotalTime / dotTime == 0 )
	{
		NETWORK2->PostServerEvent("cDamageCalc::PhyDotDamage600 valueG == 0 [%d,%d][%d,%d,%d,%d,%d,%d,%d,%d]", dotTotalTime, dotTime,
			attackerType, attackerLv, targetType, targetLv, attackerStatusPhyAttack, infAttackPlus, infAttackPer, targetStatusPhyDef );
	}
	else
	{
		valueG = (float)dotTotalTime / dotTime;
	}

	// H =  ((A * F) * C / G + E) * D
	float valueH = (( valueA * valueF ) * valueC / valueG + valueE ) * valueD;

	if( g_eventPrint == true )
		Verbose->WriteLog("PhyDotDamage attackerLv[%d] H[%.2f]", attackerLv, valueH );

	/// 데미지 값은 1보다 작을수 없다.
	return valueH > 1.0f ? FloatToInt( valueH ) : 1;
}


unsigned long cDamageCalc::MagDotDamage630( unsigned char attackerType, unsigned char attackerLv, unsigned char targetType, unsigned char targetLv,
										   unsigned long attackerStatusMagAttack, long infAttackPlus, long infAttackPer, unsigned long targetStatusMagDef,
										   unsigned long dotTotalTime, unsigned long dotTime )
{
	float num631 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 631 );
	float num632 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 632 );
	float num633 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 633 );
	float num634 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 634 );
	float num635 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 635 );
	float num636 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 636 );
	float num637 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 637 );
	float num638 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 638 );
	float num639 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 639 );
	float num640 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 640 );
	float num641 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 641 );
	float num642 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 642 );
	float num643 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 643 );
	float num644 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 644 );
	float num645 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 645 );

	// 도트 대미지(마법)
	// A =	[공격자] 마법 공격력	
	// B =  [방어자] 마법 방어력 * {num631}
	// C =	{num632} - (({num633} * [방어자] 마법 방어력) / ({num634} * ([방어자] 마법 방어력 ^ {num635}) + {num636} + ( {num637} * [공격자] Lv) - ( {num638} * [방어자] Lv)))
	//[공격자] Lv(유저) < [방어자] Lv(유저)
	// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num639}) ^ {num640}) / 100	최소 {num641}
	//[공격자] Lv(유저) < [방어자] Lv(몬스터)
	// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num642}) ^ {num643}) / 100	최소 {num644}
	//그 외의 모든 경우
	// D =  {num645}
	// E =	[공격자] 효과 스킬 공격력(+)		
	// F =	(100 + [공격자] 효과 스킬 공격력(%)) / 100
	// G =  스킬 지속시간 / 스킬 틱시간
	// H =  ((A * F) * C / G + E) * D

	// A =	[공격자] 마법 공격력		
	float valueA = (float)attackerStatusMagAttack;
	// B =  [방어자] 마법 방어력 * {num631}
	float valueB = (float)targetStatusMagDef * num631;
	// C =	{num632} - (({num633} * [방어자] 마법 방어력) / ({num634} * ([방어자] 마법 방어력 ^ {num635}) + {num636} + ( {num637} * [공격자] Lv) - ( {num638} * [방어자] Lv)))
	float valueC = num632 - ( ( num633 * valueB ) / ( num634 * ( pow( valueB, num635) ) + num636 + ( num637 * attackerLv ) - ( num638 * targetLv )));
	//그 외의 모든 경우
	// D =  {num645}
	float valueD = num645;
	if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_PLAYER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(유저)
		// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num639}) ^ {num640}) / 100	최소 {num641}
		valueD = ( 100.0f - pow((( targetLv - attackerLv ) * num639 ), num640 )) / 100.0f;
		if( valueD < num641 )
		{
			valueD = num641;
		}
	}
	else if( attackerType == eOBJECTTYPE_PLAYER && targetType == eOBJECTTYPE_MONSTER && attackerLv < targetLv )
	{
		//[공격자] Lv(유저) < [방어자] Lv(몬스터)
		// D =  (100 - (([방어자] Lv - [공격자] Lv) * {num642}) ^ {num643}) / 100	최소 {num644}
		valueD = ( 100.0f - pow((( targetLv - attackerLv ) * num642 ), num643 )) / 100.0f;
		if( valueD < num644 )
		{
			valueD = num644;
		}
	}
	// E =	[공격자] 효과 스킬 공격력(+)	
	float valueE = (float)infAttackPlus;
	// F =	(100 + [공격자] 효과 스킬 공격력(%)) / 100
	float valueF = (float)( 100.0f + infAttackPer ) / 100.0f;
	// G =  효과 지속시간 / 효과 틱시간
	float valueG = 1;
	if( dotTime == 0 || dotTotalTime / dotTime == 0 )
	{
		NETWORK2->PostServerEvent("cDamageCalc::MagDotDamage630 valueG == 0 [%d,%d][%d,%d,%d,%d,%d,%d,%d,%d]", dotTotalTime, dotTime,
			attackerType, attackerLv, targetType, targetLv, attackerStatusMagAttack, infAttackPlus, infAttackPer, targetStatusMagDef );
	}
	else
	{
		valueG = (float)dotTotalTime / dotTime;
	}

	// H =  ((A * F) * C / G + E) * D
	float valueH = (( valueA * valueF ) * valueC / valueG + valueE ) * valueD;

	if( g_eventPrint == true )
		Verbose->WriteLog("MagDotDamage attackerLv[%d] H[%.2f]", attackerLv, valueH );

	/// 데미지 값은 1보다 작을수 없다.
	return valueH > 1.0f ? FloatToInt( valueH ) : 1;
}


unsigned long cDamageCalc::PhyConditionOdd900( long skillRate, unsigned char attackerLv, unsigned char targetLv )
{
	float num901 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 901 );
	float num902 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 902 );
	float num903 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 903 );
	float num904 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 904 );
	float num905 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 905 );

	// 상태이상(물리) 
	// A =	스킬 성공 기본값
	// [공격자] Lv < [방어자] Lv   B =  ( 100 - ((( [방어자] Lv - [공격자] Lv ) * {num901} ) ^ {num902} )) / 100	최소 {num903}
	// [공격자] Lv > [방어자] Lv   B =  1 + (( [공격자] Lv - [방어자] Lv ) / {num904} )	최대 {num905}
	// A * B
	float valueA = (float)skillRate;
	float valueB = 0;

	if( attackerLv <= targetLv )
	{
		valueB = ( 100.0f - pow(((targetLv - attackerLv) * num901), num902)) / 100.0f;
		if( valueB < num903 )
		{
			valueB = num903;
		}
	}
	else
	{
		valueB = 1.0f + ((float)(attackerLv - targetLv) / num904);
		if( valueB > num905 )
		{
			valueB = num905;
		}
	}

	float valueC = valueA * valueB;

	if( g_eventPrint == true )
		Verbose->WriteLog("PhyConditionOdd attackerLv[%d] C[%.2f]", attackerLv, valueC );

	if( valueC < 0.0f )
	{
		NETWORK2->PostServerEvent("cDamageCalc::PhyConditionOdd900 valueC < 0.0f[%d,%d,%d,%d]", valueC, skillRate, attackerLv, targetLv );
		valueC = 0.0f;
	}
	return (unsigned long)valueC;
}



unsigned long cDamageCalc::MagConditionOdd930( long skillRate, unsigned char attackerLv, unsigned char targetLv )
{
	float num931 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 931 );
	float num932 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 932 );
	float num933 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 933 );
	float num934 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 934 );
	float num935 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 935 );

	// 상태이상(마법)
	// A =	스킬 성공 기본값
	//[공격자] Lv =< [방어자] Lv   B =  ( 100 - ((( [방어자] Lv - [공격자] Lv ) * {num931} ) ^ {num932} )) / 100	최소 {num933}
	//[공격자] Lv > [방어자] Lv   B =  1 + (( [공격자] Lv - [방어자] Lv ) / {num934} )	최대 {num935}
	// A * B
	float valueA = (float)skillRate;
	float valueB = 0;

	if( attackerLv <= targetLv )
	{
		valueB = ( 100.0f - pow(((targetLv - attackerLv) * num931), num932) ) / 100.0f;
		if( valueB < num933 )
		{
			valueB = num933;
		}
	}
	else
	{
		valueB = 1.0f + ((float)(attackerLv - targetLv) / num934);
		if( valueB > num935 )
		{
			valueB = num935;
		}
	}

	float valueC = valueA * valueB;

	if( g_eventPrint == true )
		Verbose->WriteLog("MagConditionOdd attackerLv[%d] C[%.2f]", attackerLv, valueC );

	if( valueC < 0.0f )
	{
		NETWORK2->PostServerEvent("cDamageCalc::MagConditionOdd930 valueC < 0.0f[%d,%d,%d,%d]", valueC, skillRate, attackerLv, targetLv );
		valueC = 0.0f;
	}
	return (unsigned long)valueC;
}


unsigned long cDamageCalc::HealHP1200( long statusHeal, long skillHealPlus, long skillHealPer, long targetHealPlus, long targetHealPer, long targetItemPlus,
									  long targetItemPer )
{
	// 회복(힐) 
	// [A] = 힐 회복량
	// B   = 스킬 기본 회복량 [+-]값
	// C   = (A * ((100 + 스킬 기본 회복량 [%]값) / 100) + B) * ((100 + 아이템 받는 회복량 증가 [%] 값 + 버프성 받는 회복량 증가 [%] 값) /
	//       100) + 아이템 받는 회복력 증가 [+-]값 + 버프성 받는 회복량 증가 [+-]값 
	float valueA = (float)statusHeal;
	float valueB = (float)skillHealPlus;
	float valueC = ( valueA * (( 100.0f + skillHealPer ) / 100.0f ) + valueB ) * (( 100.0f + targetItemPer + targetHealPer ) /
		100.0f ) + targetItemPlus + targetHealPlus;
	
	if( g_eventPrint == true )
		Verbose->WriteLog("HealHP C[%.2f]", valueC );

	if( valueC < 0.0f )
	{
		NETWORK2->PostServerEvent("cDamageCalc::HealHP1200 valueC < 0.0f[%f][%d,%d,%d,%d,%d,%d,%d]", valueC, statusHeal, 
			skillHealPlus, skillHealPer, targetHealPlus, targetHealPer, targetItemPlus, targetItemPer );
		valueC = 0.0f;
	}

	/// 결과값 리턴
	return (unsigned long)valueC;
}


unsigned long cDamageCalc::DotHealHP1230( long statusHeal, long infHealPlus, long infHealPer, unsigned long dotTotalTime, unsigned long dotTime )
{
	// 도트 힐 회복 계산
	// [A] = 힐 회복량
	// B   =	스킬 도트 회복량[+-]값
	// C   =  스킬 지속시간 / 스킬 틱시간
	// D   =  (A * ((100 + 스킬 도트회복량[%]값) / 100 ) / C) + B 
	float valueA = (float)statusHeal;
	float valueB = (float)infHealPlus;	
	float valueC = 1.0f;
	if( dotTotalTime == 0 || dotTotalTime / dotTime == 0 )
	{
		NETWORK2->PostServerEvent("cDamageCalc::DotHealHP1230 dotTotalTime == 0[%d,%d,%d,%d,%d]", statusHeal, infHealPlus, 
			infHealPer,	dotTotalTime, dotTime );
	}
	else
		valueC = (float)dotTotalTime / dotTime;
	float valueD = (float) ( valueA * (( 100.0f + infHealPer ) / 100.0f ) / valueC ) + valueB;

	if( g_eventPrint == true )
		Verbose->WriteLog("DotHealHP D[%.2f] ", valueD );

	if( valueD < 0.0f )
	{
		NETWORK2->PostServerEvent("cDamageCalc::DotHealHP1230 valueD < 0.0f[%f,%d,%d,%d,%d,%d]", valueD, statusHeal, infHealPlus,
			infHealPer, dotTotalTime, dotTime );
		valueD = 0.0f;
	}

	/// 결과값 리턴
	return (unsigned long)valueD;
}
