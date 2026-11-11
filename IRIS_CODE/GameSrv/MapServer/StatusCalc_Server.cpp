#include "stdafx.h"
#include "statuscalc_Server.h"
#include "GameSrv.h"
#include "Player.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "StatusScript.h"
#include "Skill_Common.h"
#include "HaveSkillObject.h"
#include "InfluenceObject.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "MonsterScript.h"

cStatusCalc_Server* cStatusCalc_Server::mpStatusCalc = NULL;


cStatusCalc_Server::cStatusCalc_Server(void)
{
	if( !mpStatusCalc )
	{
		mpStatusCalc = this;
	}
}



cStatusCalc_Server::~cStatusCalc_Server(void)
{
}



void cStatusCalc_Server::CalcPlayerStatusItemSkillAddDetail( cPlayer* pPlayer, eSTATUS_PLUS statusType, float value )
{
	unsigned long arrayNum = 0;

	if( eSTATUSPLUS_STR_PLUS <= statusType && statusType < eSTATUSPLUS_BASE_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_STR_PLUS;
		mBaseStatusPlus[arrayNum] = mBaseStatusPlus[arrayNum] + value;
	}
	else if( eSTATUSPLUS_STR_PERCENT <= statusType && statusType < eSTATUSPLUS_BASE_PERCENT_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_STR_PERCENT;
		mBaseStatusPer[arrayNum] = mBaseStatusPer[arrayNum] + value;
	}
	else if( eSTATUSPLUS_ATK_PLUS <= statusType && statusType <= eSTATUSPLUS_EXT_PLUS_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_ATK_PLUS;
		mExtStatusPlus[arrayNum] = mExtStatusPlus[arrayNum] + value;
	}
	else if( eSTATUSPLUS_ATK_PER <= statusType && statusType <= eSTATUSPLUS_EXT_PER_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_ATK_PER;
		mExtStatusPer[arrayNum] = mExtStatusPer[arrayNum] + value;

		if( mExtStatusPer[arrayNum] <= -100 )
			mExtStatusPer[arrayNum] = -99;
	}
	else if( eSTATUSPLUS_EXPADDPER <= statusType && statusType < eSTATUSPLUS_BUFF_ETC_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_EXPADDPER;
		mExtAddStatus[arrayNum] = mExtAddStatus[arrayNum] + value;

		if( statusType == eSTATUSPLUS_EXPADDPER || statusType == eSTATUSPLUS_SXPADDPER || 
			statusType == eSTATUSPLUS_MONEYPER || statusType == eSTATUSPLUS_PVPPOINTPER )
		{
			if( mExtAddStatus[arrayNum] <= -100 )
				mExtAddStatus[arrayNum] = -99;
		}
	}
	else if( eSTATUSPLUS_CHAR_SIZE == statusType )
	{
		mSizePer = mSizePer + (short)FloatToInt(value); 
	}
	/// 기본스텟 5개 전체를 변경
	else if( eSTATUSPLUS_BASEALL_PLUS == statusType )
	{
		for( int i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			mBaseStatusPlus[i] = mBaseStatusPlus[i] + value;
		}
	}
	else if( eSTATUSPLUS_BASEALL_PERCENT == statusType )
	{
		for( int i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			mBaseStatusPer[i] = mBaseStatusPer[i] + value;
		}
	}
	/// 공격력 전체를 변경+
	else if( eSTATUSPLUS_ATKTOTAL_PLUS == statusType )
	{
		mExtStatusPlus[ePLAYER_STATUS_EXT_ATK] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_RATK] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_MATK] += value;			
	}
	/// 방어력 전체를 변경+
	else if( eSTATUSPLUS_DEFTOTAL_PLUS == statusType )
	{
		mExtStatusPlus[ePLAYER_STATUS_EXT_DEF] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_MDEF] += value;
	}
	/// 공격력 전체를 변경%
	else if( eSTATUSPLUS_ATKTOTAL_PER == statusType )
	{
		mExtStatusPer[ePLAYER_STATUS_EXT_ATK] += value;
		mExtStatusPer[ePLAYER_STATUS_EXT_RATK] += value;
		mExtStatusPer[ePLAYER_STATUS_EXT_MATK] += value;	

		if( mExtStatusPer[ePLAYER_STATUS_EXT_ATK] <= -100 )
			mExtStatusPer[ePLAYER_STATUS_EXT_ATK] = -99;
		if( mExtStatusPer[ePLAYER_STATUS_EXT_RATK] <= -100 )
			mExtStatusPer[ePLAYER_STATUS_EXT_RATK] = -99;
		if( mExtStatusPer[ePLAYER_STATUS_EXT_MATK] <= -100 )
			mExtStatusPer[ePLAYER_STATUS_EXT_MATK] = -99;
	}
	/// 방어력 전체를 변경%
	else if( eSTATUSPLUS_DEFTOTAL_PER == statusType )
	{
		mExtStatusPer[ePLAYER_STATUS_EXT_DEF] += value;
		mExtStatusPer[ePLAYER_STATUS_EXT_MDEF] += value;

		if( mExtStatusPer[ePLAYER_STATUS_EXT_DEF] <= -100 )
			mExtStatusPer[ePLAYER_STATUS_EXT_DEF] = -99;
		if( mExtStatusPer[ePLAYER_STATUS_EXT_MDEF] <= -100 )
			mExtStatusPer[ePLAYER_STATUS_EXT_MDEF] = -99;
	}
	else
	{
		if( pPlayer == NULL )
		{
			NETWORK2->PostServerEvent( "cStatusCalc_Server::CalcPlayerStatusItemSkillAddDetail pPlayer == NULL" );
			return;
		}

		switch( statusType )
		{
		case eSTATUSPLUS_STOPMOVE:					mOddity[eODDITYTYPE_CANTMOVE] += 1; break;
		case eSTATUSPLUS_STOPSKILL_PHY:				mOddity[eODDITYTYPE_CANTSKILL_PHY] += 1; break;
		case eSTATUSPLUS_STOPSKILL_MAG:				mOddity[eODDITYTYPE_CANTSKILL_MAG] += 1; break;
		case eSTATUSPLUS_SLEEP:						mOddity[eODDITYTYPE_SLEEP] += 1; break;
		case eSTATUSPLUS_STUN:						mOddity[eODDITYTYPE_STUN] += 1; break;
		case eSTATUSPLUS_INVINCIBILITY_PHY:			mOddity[eODDITYTYPE_INVINCIBILITY_PHY] += 1; break;
		case eSTATUSPLUS_INVINCIBILITY_MAG:			mOddity[eODDITYTYPE_INVINCIBILITY_MAG] += 1; break;
		case eSTATUSPLUS_HP_ABSORB_LONG:			mOddity[eODDITYTYPE_HP_ABSORB] += (long)value; break;
		case eSTATUSPLUS_MP_ABSORB_LONG:			mOddity[eODDITYTYPE_MP_ABSORB] += (long)value; break;
		case eSTATUSPLUS_STOPITEM_KIND:				mOddity[eODDITYTYPE_STOPITEM_KIND] += (long)value; break;
		case eSTATUSPLUS_STOPITEM_IDX:				mOddity[eODDITYTYPE_STOPITEM_IDX] += (long)value; break;
		case eSTATUSPLUS_DAMAGE_ABSORB_PHY:			mOddity[eODDITYTYPE_DAMAGE_ABSORB_PHY] += (long)value; break;
		case eSTATUSPLUS_DAMAGE_ABSORB_PHY_RANGE:	mOddity[eODDITYTYPE_DAMAGE_ABSORB_PHY_RANGE] += (long)value; break;
		case eSTATUSPLUS_DAMAGE_ABSORB_MAG:			mOddity[eODDITYTYPE_DAMAGE_ABSORB_MAG] += (long)value; break;
		case eSTATUSPLUS_DAMAGE_REFLECT_PHY:		mOddity[eODDITYTYPE_DAMAGE_REFLECTION_PHY] += (long)value; break;
		case eSTATUSPLUS_DAMAGE_REFLECT_MAG:		mOddity[eODDITYTYPE_DAMAGE_REFLECTION_MAG] += (long)value; break;
		case eSTATUSPLUS_IGNORE_DEFENCE_PHY:		mOddity[eODDITYTYPE_IGNORE_DEFENCE_PHY] += 1; break;
		case eSTATUSPLUS_IGNORE_DEFENCE_MAG:		mOddity[eODDITYTYPE_IGNORE_DEFENCE_MAG] += 1; break;
		case eSTATUSPLUS_AVOID_MON_LOOK:			mOddity[eODDITYTYPE_AVOID_MON_LOOK] += 1; break;
		case eSTATUSPLUS_ADD_APPLYCNT:				mOddity[eODDITYTYPE_ADD_APPLY_CNT] += (long)value; break;
		case eSTATUSPLUS_STOPSKILL:
			{
				mOddity[eODDITYTYPE_CANTSKILL_PHY] += 1;
				mOddity[eODDITYTYPE_CANTSKILL_MAG] += 1;
			}
			break;
		case eSTATUSPLUS_IMMUNE_KIND:				pPlayer->AddImmuneKind( (unsigned long)value );	break;
		case eSTATUSPLUS_IMMUNEDEBUFF_APPLYTYPE:	pPlayer->AddImmuneApplyType( eINFLUENCETYPE_DEBUF, (unsigned short)value );	break;
		case eSTATUSPLUS_IMMUNEBUFF_APPLYTYPE:		pPlayer->AddImmuneApplyType( eINFLUENCETYPE_BUF, (unsigned short)value );	break;
		}
	}
}



void cStatusCalc_Server::CalcPlayerBaseOnly( cPlayer* pPlayer )
{

	if( pPlayer == NULL )
		return;

	/// 상태이상 관련
	memset( mOddity, 0, sizeof(mOddity) );

	/// 기본 스텟 초기화
	for( unsigned char i = ePLAYER_STATUS_STR ; ePLAYER_STATUS_LEVEL > i ; ++i )
		mBaseStatus[i] = 0.0f;

	/// 스크립트에서 읽어들인 순수한 기본 스텟값 계산
	CalcPlayerStatus( pPlayer );

	/// 기본 스텟 결과를 플레이어에 기록
	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		pPlayer->SetStatus1( (ePLAYER_STATUS)i, mBaseStatus[i] );
}



void cStatusCalc_Server::CalcPlayerExtensionOnly( cPlayer* pPlayer )
{

	if( pPlayer == NULL )
		return;

	/// 스텟1 + 초기화
	memset( mBaseStatusPlus, 0, sizeof(mBaseStatusPlus) );

	memset( mBaseStatusPer, 0, sizeof(mBaseStatusPer) );

	/// 스텟2 + 초기화
	memset( mExtStatusPlus, 0, sizeof(mExtStatusPlus) );

	/// 스텟2 % 초기화
	memset( mExtStatusPer, 0, sizeof(mExtStatusPer) );

	/// 경험치 관련
	memset( mExtAddStatus, 0, sizeof(mExtAddStatus) );

	/// 상태이상 관련
	memset( mOddity, 0, sizeof(mOddity) );

	/// 스킬효과, 스킬패시브, 아이템 데이타를 읽어와 스텟 수치 적용
	CalcPlayerStatusSkillInfluenceAdd( pPlayer );
	CalcPlayerStatusSkillPassiveAdd( pPlayer );
	CalcPlayerStatusItemAdd( pPlayer );

	/// 확장 결과를 플레이어에 기록
	memcpy( pPlayer->GetPointerStatus2Plus(), mExtStatusPlus, sizeof(pPlayer->GetPointerStatus2Plus()) * ePLAYER_STATUS_EXT_MAX );
	memcpy( pPlayer->GetPointerStatus2Per(), mExtStatusPer, sizeof(pPlayer->GetPointerStatus2Per()) * ePLAYER_STATUS_EXT_MAX );

	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
	{
		pPlayer->SetStatus1Plus( (ePLAYER_STATUS)i, mBaseStatusPlus[i] );
		pPlayer->SetStatus1Per( (ePLAYER_STATUS)i, mBaseStatusPer[i] );
	}

	for( unsigned char i = ePLAYER_STATUS_EXT_ADD_EXPADDPER ; i < ePLAYER_STATUS_EXT_ADD_MAX ; ++i )
		pPlayer->SetStatusEtc( (ePLAYER_STATUS_EXT_ADD)i, mExtAddStatus[i] );
}



unsigned long cStatusCalc_Server::GetParantJob( unsigned long job, unsigned int jobPos )
{

	unsigned long jobCode;

	/// 현재 직업의 부모 직업들을 배열로 받아온다.
	tArray<unsigned long>* pParentJob = NULL;

	/// 상위 직업 코드를 읽어옮
	pParentJob = STATUSSCRIPT->GetParentJob( job );

	/// 데이터가 없다 - 현재 직업이 기본 직업 이다.
	if( pParentJob == NULL )
		return NULL;

	/// 사이즈보다 크다 - 현재 직업이 최종 직업이다.
	if( jobPos > pParentJob->GetSize() - 1 )
		return NULL;

	/// 전직 차수에 해당하는 직업의 코드드를 읽어간다.
	pParentJob->GetAt( &jobCode, jobPos );

	return jobCode;
}



void cStatusCalc_Server::CalcPlayerStatusSkillInfluenceAdd( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	cHashSet::cIterator iter;

	/// 서버에서의 효과 증가치 계산
	cHashSet* influenceSet = pPlayer->GetInfluenceSet();
	if( influenceSet == NULL )
		return;

	pPlayer->ClearImmune();

	cInfluenceObject* pInfluence = NULL;
	sInfluenceScript* pInfluenceScript = NULL;
	unsigned long infIdx = 0;
	for( iter = influenceSet->Begin() ; iter != influenceSet->End() ; ++iter )
	{
		infIdx = *iter;
		pInfluence = SKILLMANAGER->GetInfluence( infIdx );
		if( pInfluence == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatusSkillInfluenceAdd pInfluence[%d]==NULL", *iter);
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}		

		pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pInfluence->GetInfluenceClassIdx() );
		if( pInfluenceScript == NULL )
		{
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatusSkillInfluenceAdd pInfluenceScript[%d] == NULL", 
				pInfluence->GetInfluenceClassIdx());
			continue;
		}

		if( pInfluence->IsTargetNoApply() == true )
			continue;

		/// 지속(데미지) 적인 효과가 아닌경우만
		if( pInfluenceScript->mTime1 == 0 )
		{
			if( pInfluenceScript->mStatusPlusIdx1 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( pPlayer, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx1),
					(float)(pInfluenceScript->mValue1) );
			}
		}
		if( pInfluenceScript->mTime2 == 0 )
		{
			if( pInfluenceScript->mStatusPlusIdx2 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( pPlayer, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx2),
					(float)(pInfluenceScript->mValue2) );
			}
		}
		if( pInfluenceScript->mStatusPlusIdx3 != 0 )
		{
			CalcPlayerStatusItemSkillAddDetail( pPlayer, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx3),
				(float)(pInfluenceScript->mValue3) );
		}
	}
}



void cStatusCalc_Server::CalcPlayerStatusSkillPassiveAdd( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	/// 서버에서의 효과 증가치 계산
	cPointerHashMap::cIterator iter;

	cPointerHashMap* pHaveMap = (cPointerHashMap*)SKILLMANAGER->GetPlayerHaveSkill( pPlayer->GetObjectID() );
	if( pHaveMap == NULL )
		return;

	cHaveSkillObject* pSkillObject = NULL;
	sPlayerSkillBaseInfo* pSkillInfo = NULL;
	sPlayerSkillStepInfo* pSkillStep = NULL;
	for( iter = pHaveMap->Begin() ; iter != pHaveMap->End() ; ++iter )
	{

		pSkillObject = (cHaveSkillObject*)((*iter).mSecond);
		if( pSkillObject == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatusSkillPassiveAdd pSkillObject == NULL [%d]", pPlayer->GetObjectID());
			continue;
		}

		pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( pSkillObject->GetSkillIdx() );
		if( pSkillInfo == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatusSkillPassiveAdd pSkillInfo == NULL [%d,%d]", pPlayer->GetObjectID(), pSkillObject->GetSkillIdx() );
			continue;
		}

		if( pSkillInfo->mType != eSKILLTYPE_PASSIVE )
			continue;

		pSkillStep = &pSkillInfo->mpSetpInfoArray[pSkillObject->GetSkillStep()];
		if( pSkillStep == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatusSkillPassiveAdd pSkillStep == NULL[%d,%d]", pPlayer->GetObjectID(), pSkillObject->GetSkillIdx() );
			continue;
		}


		/// 내 장비가 맞는지 체크
		bool IsEquipment = true;
		if( pSkillInfo->mUseEquipment )
		{
			eWEAPON_STATE weaponState = pPlayer->GetWeaponState();

			/// 착용 장비 체크
			switch( pSkillInfo->mUseEquipment )
			{
			case eEQUIPTYPE_SWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment = false;
				break;
			case eEQUIPTYPE_LONGSWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD:
				if( weaponState != eWEAPON_STATE_SHORTSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_GUN:
				if( weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_STAFF:
				if( weaponState != eWEAPON_STATE_STAFF )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHIELD:
				if( weaponState != eWEAPON_STATE_SHEILD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
				if( weaponState != eWEAPON_STATE_SWORD &&
					weaponState != eWEAPON_STATE_SWORD_SHEILD &&
					weaponState != eWEAPON_STATE_LONGSWORD &&
					weaponState != eWEAPON_STATE_DOUBLESWORD )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_SHORTSWORD_GUN:
				if( weaponState != eWEAPON_STATE_SHORTSWORD &&
					weaponState != eWEAPON_STATE_GUN )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_CLOTH:
				if( pPlayer->GetArmorSets() != ARMOR_SETS_ROBES )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_LIGHTARMOR:
				if( pPlayer->GetArmorSets() != ARMOR_SETS_LIGHT )
					IsEquipment =  false;
				break;
			case eEQUIPTYPE_HEAVYARMOR:
				if( pPlayer->GetArmorSets() != ARMOR_SETS_HEAVY )
					IsEquipment =  false;
				break;
			}
		}

		/// 장비가 맞는 경우만 계산
		if( IsEquipment == true )
		{
			if( pSkillStep->mApplyValueType1 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( pPlayer, (eSTATUS_PLUS)(pSkillStep->mApplyValueType1),
					(float)(pSkillStep->mApplyValue1) );
			}
			if( pSkillStep->mApplyValueType2 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( pPlayer, (eSTATUS_PLUS)(pSkillStep->mApplyValueType2),
					(float)(pSkillStep->mApplyValue2) );
			}
		}
	}
}



void cStatusCalc_Server::CalcPlayerStatusItemAdd( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	sEquipAbility* pEquipAbility = pPlayer->GetEquipAbility();
	if( pEquipAbility == NULL )
		return;

	/// 스텟 계산 마지막 시점에 계산되는 항목은 제외
	if( pEquipAbility->mOptStr != 0 )	/// 증가:힘(+)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_STR_PLUS, (float)pEquipAbility->mOptStr );
	}
	if( pEquipAbility->mOptDex != 0 )	/// 증가:민첩(+)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DEX_PLUS, (float)pEquipAbility->mOptDex );
	}
	if( pEquipAbility->mOptCon != 0 )	/// 증가:체력(+)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_CON_PLUS, (float)pEquipAbility->mOptCon );
	}
	if( pEquipAbility->mOptInt != 0 )	/// 증가:지능(+)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_INT_PLUS, (float)pEquipAbility->mOptInt );
	}
	if( pEquipAbility->mOptWis != 0 )	/// 증가:지혜(+)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_WIS_PLUS, (float)pEquipAbility->mOptWis );
	}

/*  // 아이템 옵션이 아직 적용 안되는 시점임. [7/13/2010 JO_CLIENT]
	//
	if( pEquipAbility->mOptPercentStr != 0 )	/// 증가:힘(%)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_STR_PERCENT, (float)pEquipAbility->mOptPercentStr );
	}
	if( pEquipAbility->mOptPercentDex != 0 )	/// 증가:민첩(%)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DEX_PERCENT, (float)pEquipAbility->mOptPercentDex );
	}
	if( pEquipAbility->mOptPercentCon != 0 )	/// 증가:체력(%)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_CON_PERCENT, (float)pEquipAbility->mOptPercentCon );
	}
	if( pEquipAbility->mOptPercentInt != 0 )	/// 증가:지능(%)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_INT_PERCENT, (float)pEquipAbility->mOptPercentInt );
	}
	if( pEquipAbility->mOptPercentWis != 0 )	/// 증가:지혜(%)
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_WIS_PERCENT, (float)pEquipAbility->mOptPercentWis );
	}
*/
	//if( pEquipAbility->mOptPlusWei != 0 )	/// 증가:무게
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_WEIGHT_PLUS, (float)pEquipAbility->mOptPlusWei );
	//}
	//if( pEquipAbility->mOptPlusMov != 0 )	/// 증가:이동속도
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_SPD_PLUS, (float)pEquipAbility->mOptPlusMov );
	//}
	//if( pEquipAbility->mOptPlusMaxHp != 0 )	/// 증가:최대HP 
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_HP_PLUS, (float)pEquipAbility->mOptPlusMaxHp );
	//}
	//if( pEquipAbility->mOptPlusMaxMp != 0 )	/// 증가:최대MP 
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_MP_PLUS, (float)pEquipAbility->mOptPlusMaxMp );
	//}

	if( pEquipAbility->mOptPercentDrainHp != 0 )	// HP 흡수
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_HP_ABSORB_LONG, (float)pEquipAbility->mOptPercentDrainHp );
	}
	if( pEquipAbility->mOptPercentDrainMp != 0 )	// MP 흡수
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_MP_ABSORB_LONG, (float)pEquipAbility->mOptPercentDrainMp );
	}
	if( pEquipAbility->mOptPercentDamageDec != 0 )	// 데미지 흡수/감소
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DAMAGE_ABSORB_PHY, (float)pEquipAbility->mOptPercentDamageDec );
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DAMAGE_ABSORB_MAG, (float)pEquipAbility->mOptPercentDamageDec );
	}
	if( pEquipAbility->mOptPercentDamageRef != 0 )	// 데미지 반사
	{
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DAMAGE_REFLECT_PHY, (float)pEquipAbility->mOptPercentDamageRef );
		CalcPlayerStatusItemSkillAddDetail( pPlayer, eSTATUSPLUS_DAMAGE_REFLECT_MAG, (float)pEquipAbility->mOptPercentDamageRef );
	}
}



void cStatusCalc_Server::CalcStatusPlusDetailMonster( unsigned long monsterIdx, eSTATUS_PLUS statusType, float value )
{
	/// 몬스터 정보를 가져옴
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL ) 
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent("cStatusCalc_Server::CalcStatusPlusDetailMonster pMonster == NULL");
		return; 
	}

	switch( statusType )
	{
		/// plus
	case eSTATUSPLUS_ATK_PLUS:	{ pMonster->GetStatusPlus()->mPhysicAttack += value; }break;
	case eSTATUSPLUS_RATK_PLUS:	{ pMonster->GetStatusPlus()->mPhysicRangeAttack += value; }break;
	case eSTATUSPLUS_DEF_PLUS:	{ pMonster->GetStatusPlus()->mPhysicDefense += value; }break;
	case eSTATUSPLUS_MATK_PLUS:	{ pMonster->GetStatusPlus()->mMagicAttack += value; }break;
	case eSTATUSPLUS_MDEF_PLUS:	{ pMonster->GetStatusPlus()->mMagicDefense += value; }break;
	case eSTATUSPLUS_CRI_PLUS:	{ pMonster->GetStatusPlus()->mPhysicCritical += value; }break;
	case eSTATUSPLUS_MCRI_PLUS:	{ pMonster->GetStatusPlus()->mMagicCritical += value; }break;
	case eSTATUSPLUS_HP_PLUS:	{ pMonster->GetStatusPlus()->mMaxHP += value; } break;
	case eSTATUSPLUS_MP_PLUS:	{ pMonster->GetStatusPlus()->mMaxMP += value; } break;
	case eSTATUSPLUS_HIT_PLUS:	{ pMonster->GetStatusPlus()->mPhysicAttackRate += value; }break;
	case eSTATUSPLUS_MHIT_PLUS:	{ pMonster->GetStatusPlus()->mMagicAttackRate += value; }break;
	case eSTATUSPLUS_AVOID_PLUS:	{ pMonster->GetStatusPlus()->mPhysicAvoid += value; }break;
	case eSTATUSPLUS_ASPD_PLUS:	{ pMonster->GetStatusPlus()->mAttackSpeed += value; }break;
	case eSTATUSPLUS_COOLTIME_PLUS:	{ pMonster->GetStatusPlus()->mCooltime += value; }break;
	case eSTATUSPLUS_RANGE_PLUS:	{ pMonster->GetStatusPlus()->mInRange += value; }break;
	case eSTATUSPLUS_RRANGE_PLUS:	{ pMonster->GetStatusPlus()->mOutRange += value; }break;
	case eSTATUSPLUS_SPD_PLUS:	{ pMonster->GetStatusPlus()->mMoveSpeed += value; }break;
	case eSTATUSPLUS_MAVOID_PLUS:	{ pMonster->GetStatusPlus()->mMagicAvoid += value; }break;
	case eSTATUSPLUS_CRIDEF_PLUS:	{ pMonster->GetStatusPlus()->mPhysicCriticalDef += value; }break;
	case eSTATUSPLUS_MCRIDEF_PLUS:	{ pMonster->GetStatusPlus()->mMagicCriticalDef += value; }break;
	case eSTATUSPLUS_ATKTOTAL_PLUS:
		{
			pMonster->GetStatusPlus()->mPhysicAttack += value;
			pMonster->GetStatusPlus()->mPhysicRangeAttack += value;
			pMonster->GetStatusPlus()->mMagicAttack += value;
		}
		break;
	case eSTATUSPLUS_DEFTOTAL_PLUS:
		{
			pMonster->GetStatusPlus()->mPhysicDefense += value;
			pMonster->GetStatusPlus()->mMagicDefense += value;
		}
		break;

		/// percent
	case eSTATUSPLUS_ATK_PER:	{ pMonster->GetStatusPer()->mPhysicAttack += value; }break;
	case eSTATUSPLUS_RATK_PER:	{ pMonster->GetStatusPer()->mPhysicRangeAttack += value; }break;
	case eSTATUSPLUS_DEF_PER:	{ pMonster->GetStatusPer()->mPhysicDefense += value; }break;
	case eSTATUSPLUS_MATK_PER:	{ pMonster->GetStatusPer()->mMagicAttack += value; }break;
	case eSTATUSPLUS_MDEF_PER:	{ pMonster->GetStatusPer()->mMagicDefense += value; }break;
	case eSTATUSPLUS_CRI_PER:	{ pMonster->GetStatusPer()->mPhysicCritical += value; }break;
	case eSTATUSPLUS_MCRI_PER:	{ pMonster->GetStatusPer()->mMagicCritical += value; }break;
	case eSTATUSPLUS_HP_PER:	{ pMonster->GetStatusPer()->mMaxHP += value; } break;
	case eSTATUSPLUS_MP_PER:	{ pMonster->GetStatusPer()->mMaxMP += value; } break;
	case eSTATUSPLUS_HIT_PER:	{ pMonster->GetStatusPer()->mPhysicAttackRate += value; }break;
	case eSTATUSPLUS_MHIT_PER:	{ pMonster->GetStatusPer()->mMagicAttackRate += value; }break;
	case eSTATUSPLUS_AVOID_PER:	{ pMonster->GetStatusPer()->mPhysicAvoid += value; }break;
	case eSTATUSPLUS_ASPD_PER:	{ pMonster->GetStatusPer()->mAttackSpeed += value; }break;
	case eSTATUSPLUS_COOLTIME_PER:	{ pMonster->GetStatusPer()->mCooltime += value; }break;
	case eSTATUSPLUS_RANGE_PER:	{ pMonster->GetStatusPer()->mInRange += value; }break;
	case eSTATUSPLUS_RRANGE_PER:	{ pMonster->GetStatusPer()->mOutRange += value; }break;
	case eSTATUSPLUS_SPD_PER:	{ pMonster->GetStatusPer()->mMoveSpeed += value; }break;
	case eSTATUSPLUS_MAVOID_PER:	{ pMonster->GetStatusPer()->mMagicAvoid += value; }break;
	case eSTATUSPLUS_CRIDEF_PER:	{ pMonster->GetStatusPer()->mPhysicCriticalDef += value; }break;
	case eSTATUSPLUS_MCRIDEF_PER:	{ pMonster->GetStatusPer()->mMagicCriticalDef += value; }break;
	case eSTATUSPLUS_ATKTOTAL_PER:
		{
			pMonster->GetStatusPer()->mPhysicAttack += value;
			pMonster->GetStatusPer()->mPhysicRangeAttack += value;
			pMonster->GetStatusPer()->mMagicAttack += value;
		}
		break;
	case eSTATUSPLUS_DEFTOTAL_PER:
		{
			pMonster->GetStatusPer()->mPhysicDefense += value;
			pMonster->GetStatusPer()->mMagicDefense += value;
		}
		break;

		/// 상태 이상
	case eSTATUSPLUS_STOPMOVE:	mOddity[eODDITYTYPE_CANTMOVE] += 1; break;
	case eSTATUSPLUS_STOPSKILL_PHY:	mOddity[eODDITYTYPE_CANTSKILL_PHY] += 1; break;
	case eSTATUSPLUS_STOPSKILL_MAG:	mOddity[eODDITYTYPE_CANTSKILL_MAG] += 1; break;
	case eSTATUSPLUS_SLEEP:		mOddity[eODDITYTYPE_SLEEP] += 1; break;
	case eSTATUSPLUS_STUN:		mOddity[eODDITYTYPE_STUN] += 1; break;
	case eSTATUSPLUS_INVINCIBILITY_PHY:	mOddity[eODDITYTYPE_INVINCIBILITY_PHY] += 1; break;
	case eSTATUSPLUS_INVINCIBILITY_MAG:	mOddity[eODDITYTYPE_INVINCIBILITY_MAG] += 1; break;
	case eSTATUSPLUS_HP_ABSORB_LONG:	mOddity[eODDITYTYPE_HP_ABSORB] += (long)value; break;
	case eSTATUSPLUS_MP_ABSORB_LONG:	mOddity[eODDITYTYPE_MP_ABSORB] += (long)value; break;
	case eSTATUSPLUS_STOPITEM_KIND:		mOddity[eODDITYTYPE_STOPITEM_KIND] += (long)value; break;
	case eSTATUSPLUS_STOPITEM_IDX:		mOddity[eODDITYTYPE_STOPITEM_IDX] += (long)value; break;
	case eSTATUSPLUS_DAMAGE_ABSORB_PHY:	mOddity[eODDITYTYPE_DAMAGE_ABSORB_PHY] += (long)value; break;
	case eSTATUSPLUS_DAMAGE_ABSORB_MAG:	mOddity[eODDITYTYPE_DAMAGE_ABSORB_MAG] += (long)value; break;
    case eSTATUSPLUS_DAMAGE_ABSORB_PHY_RANGE:	mOddity[eODDITYTYPE_DAMAGE_ABSORB_PHY_RANGE] += (long)value; break;
	case eSTATUSPLUS_DAMAGE_REFLECT_PHY:		mOddity[eODDITYTYPE_DAMAGE_REFLECTION_PHY] += (long)value; break;
	case eSTATUSPLUS_DAMAGE_REFLECT_MAG:		mOddity[eODDITYTYPE_DAMAGE_REFLECTION_MAG] += (long)value; break;
	case eSTATUSPLUS_IGNORE_DEFENCE_PHY: mOddity[eODDITYTYPE_IGNORE_DEFENCE_PHY] += 1; break;
	case eSTATUSPLUS_IGNORE_DEFENCE_MAG: mOddity[eODDITYTYPE_IGNORE_DEFENCE_MAG] += 1; break;
	case eSTATUSPLUS_AVOID_MON_LOOK:	 mOddity[eODDITYTYPE_AVOID_MON_LOOK] += 1; break;
	case eSTATUSPLUS_ADD_APPLYCNT:		 mOddity[eODDITYTYPE_ADD_APPLY_CNT] += (long)value; break;
	case eSTATUSPLUS_STOPSKILL:
		{
			mOddity[eODDITYTYPE_CANTSKILL_PHY] += 1;
			mOddity[eODDITYTYPE_CANTSKILL_MAG] += 1;
		}
		break;
	case eSTATUSPLUS_IMMUNE_KIND:				pMonster->AddImmuneKind( (unsigned long)value );	break;
	case eSTATUSPLUS_IMMUNEDEBUFF_APPLYTYPE:	pMonster->AddImmuneApplyType( eINFLUENCETYPE_DEBUF, (unsigned short)value );	break;
	case eSTATUSPLUS_IMMUNEBUFF_APPLYTYPE:		pMonster->AddImmuneApplyType( eINFLUENCETYPE_BUF, (unsigned short)value );	break;
	case eSTATUSPLUS_CHAR_SIZE:
		{
			mSizePer = mSizePer + (short)FloatToInt(value); 
		}
		break;
	}
}



void cStatusCalc_Server::CalcMonster( unsigned long monsterIdx )
{

	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )	
	{ 
		NETWORK2->PostServerEvent("cStatusCalc_Server::CalcMonster pMonster[%d] == NULL", monsterIdx );
		return; 
	}

	mMoveSpeed = pMonster->GetMoveSpeed();

	/// 몬스터에 걸린 효과 정보를 읽어옴
	cHashSet* influenceSet = pMonster->GetInfluenceSet();
	if( influenceSet == NULL )
		return;

	sMonsterStatus* pStatusPlus = pMonster->GetStatusPlus();
	if( pStatusPlus == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cStatusCalc_Server::CalcMonster pStatusPlus == NULL" );
		return;
	}

	sMonsterStatus* pStatusPer = pMonster->GetStatusPer();
	if( pStatusPer == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cStatusCalc_Server::CalcMonster pStatusPer == NULL" );
		return;
	}

	memset( pStatusPlus, 0, sizeof(sMonsterStatus) );
	memset( pStatusPer, 0, sizeof(sMonsterStatus) );
	memset( mOddity, 0, sizeof(mOddity) );

	pMonster->ClearImmune();

	cInfluenceObject* pInfluence = NULL;
	sInfluenceScript* pInfluenceScript = NULL;

	cHashSet::cIterator iter = influenceSet->Begin();
	cHashSet::cIterator end = influenceSet->End();

	mSizePer = 0;		/// 캐릭터 크기변화
	mFixedObjectSize = pMonster->GetFixedObjectSize();

	while( iter != end )
	{
		pInfluence = SKILLMANAGER->GetInfluence( *iter++ );
		if( pInfluence == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcMonster pInfluence == NULL" );
			//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );	/// 객체 삭제 대기열에 넣는다.
			continue;
		}		

		pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pInfluence->GetInfluenceClassIdx() );
		/// 지속(데미지) 적인 효과가 아닌경우만
		if( pInfluenceScript->mTime1 == 0 )
		{
			if( pInfluenceScript->mStatusPlusIdx1 != 0 )
			{
				CalcStatusPlusDetailMonster( monsterIdx, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx1),
					(float)(pInfluenceScript->mValue1) );
			}
		}
		if( pInfluenceScript->mStatusPlusIdx2 != 0 )
		{
			CalcStatusPlusDetailMonster( monsterIdx, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx2),
				(float)(pInfluenceScript->mValue2) );
		}
		if( pInfluenceScript->mStatusPlusIdx3 != 0 )
		{
			CalcStatusPlusDetailMonster( monsterIdx, (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx3),
				(float)(pInfluenceScript->mValue3) );
		}
	}

	pMonster->SetStateOddity( mOddity );

	/// 이동 속도가 변한경우 클라이언트에 전송
	if( pMonster->GetMoveSpeed() != mMoveSpeed )
	{
		pMonster->SendMoveSpeed();
	}

	/// 크기 계산후 변했으면 클라이언트에도 알림
	pMonster->SetFixedObjectSizePer( (short)mSizePer );
	if( mFixedObjectSize != pMonster->GetFixedObjectSize() )
		pMonster->SendMonsterSize();
}


void cStatusCalc_Server::CalcPlayerStatus( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	unsigned char level = pPlayer->GetLevel();
	ePLAYER_JOB job = pPlayer->GetJob();

	/// 전직클래스1 기준 레벨 
	unsigned char class1 = 30;
	/// 전직클래스2 기준 레벨
	unsigned char class2 = 70;

	sStatusBaseScript* pStatusBase = NULL;

	/// 자기 자신에 해당하는 직업의 statusbase 정보를 가져옴
	pStatusBase = STATUSSCRIPT->GetStatusBaseInfo( job );

	/// 자신에 해당하는 직업이 없으면 상위 직업을 읽어와서 처리한다.
	if( pStatusBase == NULL )
	{
		/// 상위 직업 배열을 읽어온다.
		tArray<unsigned long>* playerJobParent = STATUSSCRIPT->GetParentJob( job );
		if( playerJobParent == NULL )
		{
			/// 자신의 직업도 없는데 부모의 직업도 없으면 오류
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatus parantJob[%d] == NULL", job );
			return; 
		}
		else
		{

			/// 상위 직업이 있으면 
			for( int i = playerJobParent->GetSize() - 1 ; 0 <= i  ; --i )
			{
				/// 상위 직업코드로 스크립트 값을 읽어옴
				pStatusBase = STATUSSCRIPT->GetStatusBaseInfo( (*playerJobParent)[i] );
				if( pStatusBase != NULL )
				{
					break;
				}
			}

			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatus playerJobParentAry == NULL" );
		}
	}

	/// 입력 값 기본 스텟 수치로 적용 
	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
	{
		mBaseStatus[i] = pStatusBase->mBaseStatus[i];
	}

	/// 전직 차수와 레벨에 따른 실제 적용 받아야할 직업코드
	unsigned long fixjob = job;

	sStatusMultiplScript* pStatusMultipl;

	/// 현재 레벨에서 1레벨까지 스텟 계산 ( 0레벨은 기본값 1레벨부터 증가치 )
	//for( unsigned char j = 1 ; j <= level ; ++j )
	for( unsigned char j = 1 ; j < level ; ++j )
	{

		/// 1레벨에서 1차전직레벨(30)전까지 기본직업 코드로 스텟 계산
		if( ( 1 <= j ) && ( j < class1 ) )
		{
			/// 상위 직업 리스트에서 1차전직 직업을 가져옴
			fixjob = GetParantJob( job, 0 );
			/// 없는 경우 현재 직업이 기본 직업이라 부모가 없는 경우임
			if( fixjob == NULL )
			{
				/// 현재 직업을 스텟 계산 기준 코드로 등록
				fixjob = job;
			}
		}

		/// 1차전직레벨(30)에서 2차전직레벨(70)까지 1차전직
		else if( ( class1 <= j ) && ( j < class2 ) )
		{
			/// 상위 직업 리스트에서 1차전직 직업을 가져옴
			fixjob = GetParantJob( job, 1 );
			/// 없는 경우 현재 직업이 1차 직업임
			if( fixjob == NULL )
			{
				fixjob = job;
			}
		}

		/// 2차전직레벨(30)에서 레벨 한계점 까지
		else if( class2 <= j )
		{
			/// 상위 직업 리스트에서 2차전직 직업을 가져옴
			fixjob = GetParantJob( job, 2 );
			/// 없는 경우 현재 직업이 2차 직업임
			if( fixjob == NULL )
			{
				fixjob = job;
			}
		}

		/// 현재 레벨에 맞는 직업 코드로 스텟 증가치를 얻어옴
		pStatusMultipl = STATUSSCRIPT->GetStatusMultiplInfo( fixjob );
		if( pStatusMultipl == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cStatusCalc_Server::CalcPlayerStatus pStatusMultipl[%d] == NULL", fixjob);
			return;
		}

		for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			/// 클래스기준에 따른 증가치 만큼 증가
			mBaseStatus[i] = mBaseStatus[i] + pStatusMultipl->mBaseStatus[i];
		}
	}
}



void cStatusCalc_Server::CalcPlayerExtStatus( cPlayer* pPlayer, ePLAYER_STATUS_EXT statusExt )
{
	if( pPlayer == NULL )
		return;

	ePLAYER_JOB job = pPlayer->GetJob();

	/// 플레이어의 스텟1 최종값을 읽어옴 + 레벨값까지
	float baseStatus[ePLAYER_STATUS_LEVEL_MAX];
	unsigned char i = ePLAYER_STATUS_STR;
	for( ; i < ePLAYER_STATUS_LEVEL ; ++i )
	{
		baseStatus[i] = (mBaseStatus[i] + mBaseStatusPlus[i]);

		if( mBaseStatusPer[i] != 0 )
			baseStatus[i] += (mBaseStatus[i] + mBaseStatusPlus[i]) * (mBaseStatusPer[i] * 0.01f);
		
		if( baseStatus[i] < 0.0f )
			baseStatus[i] = 0.0f;
	}
	baseStatus[i] = pPlayer->GetLevel(); 

	/// 스텟의 계산 결과 값
	float			statusValue = 0.0f;

	/// 자연회복HP/MP 기본값은 [캐릭터스탯회복수치]에 더하지 않고 자연회복계산시 사용한다.
	if( statusExt == ePLAYER_STATUS_EXT_RECOVHP || statusExt == ePLAYER_STATUS_EXT_RECOVMP )
	{
		statusValue = 0.0f;
	}
	else
	{
		/// 자신의 직업과 스텟 구분이랑 같은 것이 있는지 체크
		statusValue = STATUSSCRIPT->GetStatusExtBaseInfo( statusExt, job );
		//if( FloatToInt(statusValue) == 0 )
		//{
		//	if( playerJobParent != 0 )
		//	{
		//		/// 상위 직업이 있으면 
		//		for( int i = playerJobParent->GetSize() - 1 ; 0 <= i  ; --i )
		//		{
		//			/// 상위 직업코드로 스크립트 값을 읽어옴
		//			statusValue = STATUSSCRIPT->GetStatusExtBaseInfo( statusExt, (*playerJobParent)[i] );

		//			if( FloatToInt(statusValue) != 0 )
		//			{
		//				break;
		//			}
		//		}
		//	}
		//	else
		//	{
		//		statusValue = 0.0f;
		//	}
		//}
	}


	/// 플레이어의 최상위(1차) 직업 기록
	tArray<unsigned long>* playerJobParent = STATUSSCRIPT->GetParentJob( job );
	unsigned long firstJob = job;
	if( playerJobParent != NULL )
		firstJob = (*playerJobParent)[0];

	/// 1차 직업에 따라 공격력 스텟 비중치 읽어올 항목값 설정
	unsigned long plusStatus = 0;
	switch( firstJob )
	{		
	case ePLAYER_FIGHTER:	/// 파이터
	case ePLAYER_ARMSMAN:	/// 워리어
		plusStatus = 0;
		break;
	case ePLAYER_WANDERER:	/// 레인져
	case ePLAYER_RANGER:		/// 로그
		plusStatus = 1;
		break;
	case ePLAYER_MAGE:		/// 메이지
	case ePLAYER_WARLOCK:	/// 샤먼
		plusStatus = 2;
		break;
	}

	float oriMaxHP = statusValue;
	float oriMaxMP = statusValue;

	float importance = 0.0f;
	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL_MAX ; ++i )
	{
        eEXT_MULTIPL chgStatusExt = eEXT_MULTIPL_MAX;
		switch( statusExt )
		{
		case ePLAYER_STATUS_EXT_ATK:
            chgStatusExt = (eEXT_MULTIPL)( eEXT_MULTIPL_ATK_FIGHTER + plusStatus );	/// 근거리, 원거리, 마법 직업에 따른 공격력 항목 설정
			break;
		case ePLAYER_STATUS_EXT_RATK:
			chgStatusExt = (eEXT_MULTIPL)( eEXT_MULTIPL_RATK_FIGHTER + plusStatus );
			break;
		case ePLAYER_STATUS_EXT_MATK:
			chgStatusExt = (eEXT_MULTIPL)( eEXT_MULTIPL_MATK_FIGHTER + plusStatus );
			break;
		case ePLAYER_STATUS_EXT_DEF:		chgStatusExt = eEXT_MULTIPL_DEF;		break;	/// 물리 방어력
		case ePLAYER_STATUS_EXT_MDEF:		chgStatusExt = eEXT_MULTIPL_MDEF;		break;	/// 마법 방어력
		case ePLAYER_STATUS_EXT_CRI:		chgStatusExt = eEXT_MULTIPL_CRI;		break;	/// 물리 크리티컬 수치
		case ePLAYER_STATUS_EXT_MCRI:		chgStatusExt = eEXT_MULTIPL_MCRI;		break;	/// 마법 크리티컬 수치
		case ePLAYER_STATUS_EXT_MAXHP:		/// HP 
			{
				chgStatusExt = eEXT_MULTIPL_MAXHP;		

				/// 베이스 스텟을 비중과 곱한다.
				importance = STATUSSCRIPT->GetStatusExtMultiplInfo( chgStatusExt, (ePLAYER_STATUS)i );
				oriMaxHP += mBaseStatus[i] * importance;
			}
			break;	
		case ePLAYER_STATUS_EXT_MAXMP:		/// MP
			{
				chgStatusExt = eEXT_MULTIPL_MAXMP;
				/// 베이스 스텟을 비중과 곱한다.
				importance = STATUSSCRIPT->GetStatusExtMultiplInfo( chgStatusExt, (ePLAYER_STATUS)i );
				oriMaxMP += mBaseStatus[i] * importance;
			}
			break;	
		case ePLAYER_STATUS_EXT_RECOVHP:	chgStatusExt = eEXT_MULTIPL_RECOVHP;	break;	/// HP 회복 속도
		case ePLAYER_STATUS_EXT_RECOVMP:	chgStatusExt = eEXT_MULTIPL_RECOVMP;	break;	/// MP 회복 속도
		case ePLAYER_STATUS_EXT_HIT:		chgStatusExt = eEXT_MULTIPL_HIT;		break;	/// 명중 수치
		case ePLAYER_STATUS_EXT_MHIT:		chgStatusExt = eEXT_MULTIPL_MHIT;		break;	/// 마법 명중
		case ePLAYER_STATUS_EXT_AVOID:		chgStatusExt = eEXT_MULTIPL_AVOID;		break;	/// 회피 수치
		case ePLAYER_STATUS_EXT_WEIGHT:		chgStatusExt = eEXT_MULTIPL_WEIGHT;		break;	/// 무게 
		case ePLAYER_STATUS_EXT_RANGE:		chgStatusExt = eEXT_MULTIPL_RANGE;		break;	/// 사정거리
		case ePLAYER_STATUS_EXT_RRANGE:		chgStatusExt = eEXT_MULTIPL_RRANGE;		break;	/// 방출혈사정거리
		case ePLAYER_STATUS_EXT_COOLTIME:	chgStatusExt = eEXT_MULTIPL_COOLTIME;	break;	/// 쿨타임
		case ePLAYER_STATUS_EXT_SPD:		chgStatusExt = eEXT_MULTIPL_SPD;		break;	/// 이동속도
		case ePLAYER_STATUS_EXT_ASPD:		chgStatusExt = eEXT_MULTIPL_ASPD;		break;	/// 공격 속도	
		case ePLAYER_STATUS_EXT_MAVOID:		chgStatusExt = eEXT_MULTIPL_MAVOID;		break;	/// 마법회피(저항력)
		case ePLAYER_STATUS_EXT_HEAL:		chgStatusExt = eEXT_MULTIPL_HEAL;		break;	/// 힐회복량
		case ePLAYER_STATUS_EXT_CRIDEF:		chgStatusExt = eEXT_MULTIPL_CRIDEF;		break;	/// 물리 크리티컬 감소(방호)
		case ePLAYER_STATUS_EXT_MCRIDEF:	chgStatusExt = eEXT_MULTIPL_MCRIDEF;	break;	/// 마법 크리티컬 감소(방호)
		}

		/// 베이스 스텟을 비중과 곱한다.
		importance = STATUSSCRIPT->GetStatusExtMultiplInfo( chgStatusExt, (ePLAYER_STATUS)i );
		statusValue += baseStatus[i] * importance;
	}

	if( statusExt == ePLAYER_STATUS_EXT_MAXHP )
		pPlayer->SetOriHPMax( FloatToInt( oriMaxMP ) );
	if( statusExt == ePLAYER_STATUS_EXT_MAXMP )
		pPlayer->SetOriMPMax( FloatToInt( oriMaxMP ) );

	/// 기준 이동 속도가 이미 설정돼있는 경우(몬스터변신) 처리
	if( statusExt == ePLAYER_STATUS_EXT_SPD && pPlayer->GetMoveSpeed() != 0 )
    {
		mExtStatus[statusExt] = (float)pPlayer->GetMoveSpeed();
    }
    else if ( statusExt == ePLAYER_STATUS_EXT_WEIGHT )
    {
        mExtStatus[statusExt] = mExtStatus[statusExt] + statusValue;

        switch ( pPlayer->GetBagEnd() )
        {
        case INVENTORY_BAG1_END : mExtStatus[statusExt] += ADD_BAG1_WEIGHT;  break;
        case INVENTORY_BAG2_END : mExtStatus[statusExt] += ADD_BAG2_WEIGHT;  break;
        case INVENTORY_BAG3_END : mExtStatus[statusExt] += ADD_BAG3_WEIGHT;  break;
        }
    }
	else	/// 결과 값을 기록
        mExtStatus[statusExt] = mExtStatus[statusExt] + statusValue;

	if( mExtStatus[statusExt] < 0.0f ) 
		mExtStatus[statusExt] = 0.0f;
}



void cStatusCalc_Server::CalcPlayerInit( sObject object )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( object.index );
	if( pPlayer != NULL )
	{
		mMoveSpeed = pPlayer->GetMoveSpeed();
		mAttackSpeed = pPlayer->GetStatus2()->mAttackSpeed;
		mFixedObjectSize = pPlayer->GetFixedObjectSize();
		mSizePer = 0;
		mMaxHP = pPlayer->GetMaxHP();
		mMaxMP = pPlayer->GetMaxMP();

		pPlayer->InitStatus2();

		CalcPlayerBaseOnly( pPlayer );
		CalcPlayerExtensionOnly( pPlayer );
		CalcPlayerGlobal( pPlayer );
	}
}



void cStatusCalc_Server::CalcPlayerBaseGlobal( sObject object )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( object.index );
	if( pPlayer != NULL )
	{
		mMoveSpeed = pPlayer->GetMoveSpeed();
		mAttackSpeed = pPlayer->GetStatus2()->mAttackSpeed;
		mFixedObjectSize = 0;
		mSizePer = 0;
		mMaxHP = pPlayer->GetMaxHP();
		mMaxMP = pPlayer->GetMaxMP();

		pPlayer->InitStatus2();

		mBaseStatusPlus[ePLAYER_STATUS_STR] = (float)pPlayer->GetStatus1Plus()->mStr;
		mBaseStatusPlus[ePLAYER_STATUS_DEX] = (float)pPlayer->GetStatus1Plus()->mDex;
		mBaseStatusPlus[ePLAYER_STATUS_CON] = (float)pPlayer->GetStatus1Plus()->mCon;
		mBaseStatusPlus[ePLAYER_STATUS_INT] = (float)pPlayer->GetStatus1Plus()->mInt;
		mBaseStatusPlus[ePLAYER_STATUS_WIS] = (float)pPlayer->GetStatus1Plus()->mWis;

		mBaseStatusPer[ePLAYER_STATUS_STR] = (float)pPlayer->GetStatus1Per()->mStr;
		mBaseStatusPer[ePLAYER_STATUS_DEX] = (float)pPlayer->GetStatus1Per()->mDex;
		mBaseStatusPer[ePLAYER_STATUS_CON] = (float)pPlayer->GetStatus1Per()->mCon;
		mBaseStatusPer[ePLAYER_STATUS_INT] = (float)pPlayer->GetStatus1Per()->mInt;
		mBaseStatusPer[ePLAYER_STATUS_WIS] = (float)pPlayer->GetStatus1Per()->mWis;

		memcpy( mExtStatusPlus, pPlayer->GetPointerStatus2Plus(), sizeof(mExtStatusPlus) );
		memcpy( mExtStatusPer, pPlayer->GetPointerStatus2Per(), sizeof(mExtStatusPer) );

		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_EXPADDPER] = (float)pPlayer->GetStatusEtc()->mExpAddPer;
		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_EXPADDPLUS] = (float)pPlayer->GetStatusEtc()->mExpAddPlus; 
		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_SXPADDPER] = (float)pPlayer->GetStatusEtc()->mSxpAddPer;
		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_SXPADDPLUS] = (float)pPlayer->GetStatusEtc()->mSxpAddPlus;
		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_MONEYPER] = (float)pPlayer->GetStatusEtc()->mMoneyAddPer;
		mExtAddStatus[ePLAYER_STATUS_EXT_ADD_PVPPOINTPER] = (float)pPlayer->GetStatusEtc()->mPvPPointAddPer;

		CalcPlayerBaseOnly( pPlayer );
		CalcPlayerGlobal( pPlayer );
	}
}



void cStatusCalc_Server::CalcPlayerExtensionGlobal( sObject object )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( object.index );
	if( pPlayer != NULL )
	{
		mMoveSpeed = pPlayer->GetMoveSpeed();
		mAttackSpeed = pPlayer->GetStatus2()->mAttackSpeed;
		mFixedObjectSize = pPlayer->GetFixedObjectSize();
		mSizePer = 0;
		mMaxHP = pPlayer->GetMaxHP();
		mMaxMP = pPlayer->GetMaxMP();
		
		pPlayer->InitStatus2();

		mBaseStatus[ePLAYER_STATUS_STR] = (float)pPlayer->GetStatus1()->mStr;
		mBaseStatus[ePLAYER_STATUS_DEX] = (float)pPlayer->GetStatus1()->mDex;
		mBaseStatus[ePLAYER_STATUS_CON] = (float)pPlayer->GetStatus1()->mCon;
		mBaseStatus[ePLAYER_STATUS_INT] = (float)pPlayer->GetStatus1()->mInt;
		mBaseStatus[ePLAYER_STATUS_WIS] = (float)pPlayer->GetStatus1()->mWis;

		CalcPlayerExtensionOnly( pPlayer );
		CalcPlayerGlobal( pPlayer );
	}
}



void  cStatusCalc_Server::CalcPlayerGlobal( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	/// 스텟2 베이스 초기화
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatus[i] = 0;
	}

	/// 스텟2 최종값
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatusLast[i] = 0;
	}

	sEquipAbility* pItem = pPlayer->GetEquipAbility();

	for( unsigned char i = ePLAYER_STATUS_EXT_ATK ; i < ePLAYER_STATUS_EXT_MAX ; ++i )
	{
		/// 스크립트에서 읽어들인 순수한 스텟2 값 계산
		CalcPlayerExtStatus( pPlayer , (ePLAYER_STATUS_EXT)i );

		/// 최종 스텟 계산전 가감산 할 항목 적용
		switch( (ePLAYER_STATUS_EXT)i )
		{
			/// 근거리 물리 공격
		case ePLAYER_STATUS_EXT_ATK:
			{
				float num301 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 301 );

				// 공격력(근거리)
				// A =	캐릭터 스탯 근거리 공격력
				// B =	아이템 물리 공격력 + ( [캐릭터] Lv * {num301} )
				// C =	((100 + 버프성 근거리 공격력[%]값 + 아이템 물리 공격력[%]값) / 100)
				// D =	((A + B ) * C) + 버프성 근거리 공격력[+-]값 + 아이템 물리 공격력[+-]값
				float valueA = mExtStatus[i];
				float valueB = pItem->mPhysicMinAttack + ( pPlayer->GetLevel() * num301 );
				float valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_ATKMin, mExtStatusLast[i] );

				valueB = pItem->mPhysicMaxAttack + ( pPlayer->GetLevel() * num301 );
				valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;
					
				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_ATKMax, mExtStatusLast[i] );

				mExtStatusLast[i] = 0.0f;
			}
			break;
			/// 원거리 물리 공격
		case ePLAYER_STATUS_EXT_RATK:
			{
				float num331 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 331 );

				// 공격력(원거리)
				// A =	캐릭터 스탯 원거리 공격력
				// B =	아이템 물리 공격력 + ( [캐릭터] Lv * {num331} )
				// C =	((100 + 버프성 원거리 공격력[%]값 + 아이템 물리 공격력[%]값) / 100)
				// D =	(A + B ) * C + 버프성 원거리 공격력[+-]값 + 아이템 물리 공격력[+-]값
				float valueA = mExtStatus[i];
				float valueB = pItem->mPhysicMinAttack + ( pPlayer->GetLevel() * num331 );
				float valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_RATKMin, mExtStatusLast[i] );

				valueB = pItem->mPhysicMaxAttack + ( pPlayer->GetLevel() * num331 );
				valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_RATKMax, mExtStatusLast[i] );

				mExtStatusLast[i] = 0.0f;
			}
			break;
			/// 물리 방어력
		case ePLAYER_STATUS_EXT_DEF:
			{
				float num401 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 401 );
				float num402 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 402 );

				// 방어력(물리)
				// A =	캐릭터 스탯물리 방어력
				// B =	아이템 물리 방어력
				// C =	((100 + 버프성 물리 방어력[%]값 + 아이템 물리 방어력[%]값) / 100)
				// D =	((A / {num401} + B / {num402} ) * C) + 버프성 물리 방어력[+-]값 + 아이템 물리 방어력[+-]값
				float valueA = mExtStatus[i];
				float valueB = pItem->mPhysicDefense;
				float valueC = (( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentPhysicDefense ) ) / 100.0f );
				mExtStatusLast[i] = (( valueA / num401 + valueB / num402 ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicDefense;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 마법 공격
		case ePLAYER_STATUS_EXT_MATK:
			{
				float num361 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 361 );
				float num362 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 362 );

				// 공격력(마법)
				// A =	캐릭터 스탯 마법 공격력 / {num361}
				// B =	아이템 마법 공격력 + ( [캐릭터] Lv * {num362} )
				// C =	((100 + 버프성 마법 공격력[%]값 + 아이템 마법 공격력[%]값) / 100)
				// D =	(A + B ) * C + 버프성 마법 공격력[+-]값 + 아이템 마법 공격력[+-]값
				float valueA = mExtStatus[i] / num361;
				float valueB = pItem->mMagicMinAttack + ( pPlayer->GetLevel() * num362 );
				float valueC = (( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMagicAttack ) ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusMagicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_MATKMin, mExtStatusLast[i] );

				valueB = pItem->mMagicMaxAttack + ( pPlayer->GetLevel() * num362 );
				valueC = (( 100 + ( mExtStatusPer[i] + pItem->mOptPercentMagicAttack ) ) / 100 );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusMagicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_MATKMax, mExtStatusLast[i] );

				mExtStatusLast[i] = 0.0f;
			}
			break;
			/// 마법 방어력
		case ePLAYER_STATUS_EXT_MDEF:
			{
				float num431 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 431 );
				float num432 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 432 );

				// 방어력(마법)
				// A =	캐릭터 스탯마법 방어력
				// B =	아이템 마법 방어력
				// C =	((100 + 버프성 마법 방어력[%]값 + 아이템 마법 방어력[%]값) / 100)
				// D =	((A /{num431} + B / {num432} ) * C) + 버프성 물리 방어력[+-]값 + 아이템 마법 방어력[+-]값
				float valueA = mExtStatus[i];
				float valueB = pItem->mMagicDefense;
				float valueC = (( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMagicDefense ) ) / 100.0f );
				mExtStatusLast[i] = (( valueA / num431 + valueB / num432 ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusMagicDefense;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 물리 공격 크리
		case ePLAYER_STATUS_EXT_CRI:
			{
				/// 방어자 항목과 같이 계산하는 부분때문에 서버에선 A 항목까지만 계산해둠!!!
				mExtStatusLast[i] = mExtStatus[i];
				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				//float num201 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 201 );
				//float num202 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 202 );

				//// 크리티컬 발생 확률(물리)
				////A =	[공격자] 캐릭터 스탯 물리 크리티컬 수치 + [공격자]아이템 물리 크리티컬 수치[+-]값	
				////[B] =	(A * {num201})  * ((100 + [공격자]버프성 물리 크리티컬 [%]값) + [공격자]아이템 물리 크리티컬[%]값)  / 100) + ( [공격자]버프성 물리 크리티컬 수치[+-]값 * {num202} )	
				//float valueA = mExtStatus[i] + pItem->mOptPlusPhysicCritical;
				//float valueB = ( valueA * num201 ) * ( ( 100 + mExtStatusPer[i]) / 100 ) + ( mExtStatusPlus[i] * num202 );

				//mExtStatusLast[i] = valueB;

				//if( mExtStatusLast[i] < 0.0f )
				//	mExtStatusLast[i] = 0.0f;

				//pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

			}break;
			///	마법 공격 크리
		case ePLAYER_STATUS_EXT_MCRI:
			{
				/// 방어자 항목과 같이 계산하는 부분때문에 서버에선 A 항목까지만 계산해둠!!!
				mExtStatusLast[i] = mExtStatus[i];
				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				//float num231 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 231 );
				//float num232 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 232 );

				//// 크리티컬 발생 확률(마법)
				////A =	[공격자] 캐릭터 스탯 마법 크리티컬 수치 + [공격자]아이템 마법 크리티컬 수치[+-]값	
				////B =	(A * {num231})  * ((100 + [공격자]버프성 마법 크리티컬 [%]값) + [공격자]아이템 마법 크리티컬[%]값)  / 100) + ( [공격자]버프성 마법 크리티컬 수치[+-]값 * {num232} )	
				//float valueA = mExtStatus[i] + pItem->mOptPlusMagicCritical;
				//float valueB = ( valueA * num231 ) * ( ( 100 + mExtStatusPer[i]) / 100 ) + ( mExtStatusPlus[i] * num232 );

				//mExtStatusLast[i] = valueB;

				//if( mExtStatusLast[i] < 0.0f )
				//	mExtStatusLast[i] = 0.0f;

				//pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

			}break;
			/// 최대 HP
		case ePLAYER_STATUS_EXT_MAXHP:
			{
				float num20 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 20 );

				/// 기본 hp량 + ( 레벨 * num1 )
				mExtStatus[i] = mExtStatus[i] + ( pPlayer->GetLevel() * num20 );                
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMaxHp ) ) / 100.0f )
					+ mExtStatusPlus[i] + pItem->mOptPlusMaxHp;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 최대 MP
		case ePLAYER_STATUS_EXT_MAXMP:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMaxMp ) ) / 100.0f ) + 
					mExtStatusPlus[i] + pItem->mOptPlusMaxMp;

				if( mExtStatusLast[i] < 1.0f )
					mExtStatusLast[i] = 1.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// HP 자연 회복량
		case ePLAYER_STATUS_EXT_RECOVHP:
			{
				float num1001 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 1001 );

				float defaultValueHP = STATUSSCRIPT->GetStatusExtBaseInfo( (ePLAYER_STATUS_EXT)i, pPlayer->GetJob() );

				// 캐릭터 회복 - HP 회복
				// A =	클래스 기본값
				// B =	( 캐릭터 스탯 회복 수치 * {num1001} ) * ((100 + 아이템 HP회복 [%]값+ 버프성 HP회복 [%]값) / 100) + 아이템 HP회복 [+-]값
				// A + B + 버프성 HP회복 [+-]값
				float valueA = defaultValueHP;
				float valueB = ( mExtStatus[i] * num1001 ) * (( 100.0f + pItem->mOptPercentHp + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusHp;
				mExtStatusLast[i] = valueA + valueB + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// MP 자연 회복량
		case ePLAYER_STATUS_EXT_RECOVMP:
			{
				float num1101 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 1101 );

				float defaultValueMP = STATUSSCRIPT->GetStatusExtBaseInfo( (ePLAYER_STATUS_EXT)i, pPlayer->GetJob() );

				// 캐릭터 회복 - MP 회복
				// A =	클래스 기본값
				// B =	( 캐릭터 스탯 회복 수치 * {num1101} ) * ((100 + 아이템 MP회복 [%]값 + 버프성 MP회복 [%]값) / 100)  + 아이템 MP회복 [+-]값
				// A + B + 버프성 MP회복 [+-]값
				float valueA = defaultValueMP;
				float valueB = ( mExtStatus[i] * num1101 ) * ( ( 100.0f + pItem->mOptPercentMp + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusMp;
				mExtStatusLast[i] = valueA + valueB + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 물리 공격 성공
		case ePLAYER_STATUS_EXT_HIT:
			{
				float num101 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 101 );
				float num102 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 102 );

				// 공격 성공 확률(물리)
				// A =	{num101} + (( [공격자] 캐릭터 스탯 명중 수치) * ((100 + [공격자]아이템 명중 수치[%]값 + [공격자]버프성 명중 수치[%]값) / 100) + [공격자]아이템 명중 수치[+-]값 + [공격자]버프성 명중 수치[+-]값 ) * {num102}
				float valueA = num101 + (( mExtStatus[i] ) * (( 100.0f + pItem->mOptPercentPhysicAttackRate + mExtStatusPer[i]) / 100.0f ) + pItem->mOptPlusPhysicAttackRate + mExtStatusPlus[i] ) * num102;
				
				mExtStatusLast[i] = valueA;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 마법 공격 성공
		case ePLAYER_STATUS_EXT_MHIT:
			{
				float num131 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 131 );
				float num132 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 132 );

				// 공격 성공 확률(마법)
				// A =	{num131} + (( [공격자] 캐릭터 스탯 마법 명중 수치) * ((100  + [공격자]아이템 마법 명중 수치[%]값 + [공격자]버프성 마법 명중 수치[%]값) / 100)  + [공격자]아이템 마법 명중 수치[+-]값 + [공격자]버프성 마법 명중 수치[+-]값 ) * {num132}
				float valueA = num131 + (( mExtStatus[i] ) * (( 100.0f + pItem->mOptPercentMagicAttackRate + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusMagicAttackRate + mExtStatusPlus[i] ) * num132;

				mExtStatusLast[i] = valueA;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 회피율
		case  ePLAYER_STATUS_EXT_AVOID:
			{
				float num103 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 102 );

				// C =	(( [방어자] 캐릭터 스탯 회피 수치) * ((100 + [방어자]아이템 회피 수치[%]값  + [방어자]버프성 회피 수치[%]값 ) / 100) + [방어자]아이템 회피 수치[+-]값 + [방어자]버프성 회피 수치[+-]값 ) * {num103}
				float valueC = (( mExtStatus[i] ) * (( 100.0f + pItem->mOptPercentDod + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusDod + mExtStatusPlus[i] ) * num103;

				mExtStatusLast[i] = valueC;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 무게
		case ePLAYER_STATUS_EXT_WEIGHT:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentWei ) ) / 100.0f ) + 
					mExtStatusPlus[i] + pItem->mOptPlusWei;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 일반공격 속도
		case ePLAYER_STATUS_EXT_ASPD:
			{
				mExtStatusLast[i] = ( 100.0f + pItem->mAttackSpeed + mExtStatusPer[i] + pItem->mOptPercentAttackSpeed ) / 100.0f;

				if( mExtStatusLast[i] <= 0.0f )
					mExtStatusLast[i] = 0.1f;

				if( mExtStatusLast[i] >= 2.0f )
					mExtStatusLast[i] = 1.9f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );                
			}break;
			/// 이동 속도 계산
		case ePLAYER_STATUS_EXT_SPD:
			{
				/// 몬스터 변신중일 경우 이동속도 기본값을 몬스터에서 읽어온다.
				if( pPlayer->GetChgMonsterIdx() != 0 )
				{
					sMonsterScript* pMScript = MONSTERSCRIPT->GetMonsterListInfo( pPlayer->GetChgMonsterIdx() );
					if( pMScript != NULL )
						mExtStatus[i] = (float)pMScript->mActionRunSpd;

					mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] + pItem->mOptPercentMov ) / 100 ) + 
						mExtStatusPlus[i] + pItem->mOptPlusMov;

					if( mExtStatusLast[i] < 0.0f )
						mExtStatusLast[i] = 0.0f;

					pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
				}
				else
				{
					mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] + pItem->mOptPercentMov  ) / 100 ) + 
						mExtStatusPlus[i] + pItem->mOptPlusMov;

					if( mExtStatusLast[i] < 0.0f )
						mExtStatusLast[i] = 0.0f;

					pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
				}
			}
			break;
			/// 힐 회복량
		case ePLAYER_STATUS_EXT_HEAL:
			{
				// 힐 회복량
				// A =	((캐릭터 스탯 힐회복량 + 아이템 기본 힐 회복량) * ((100 + 버프성 힐 회복량 [%]값 + 아이템 힐 회복량 [%]값) / 100) 
				//		+ 아이템 힐 회복량[+-]값 + 버프성 힐 회복량 [+-]값) 
				float valueA = (( mExtStatus[ePLAYER_STATUS_EXT_HEAL] + pItem->mHealValue ) * (( 100 + mExtStatusPer[ePLAYER_STATUS_EXT_HEAL] + pItem->mOptPercentHeal ) / 100 ) +
					pItem->mOptPlusHeal + mExtStatusPlus[ePLAYER_STATUS_EXT_HEAL] );
				mExtStatusLast[i] = valueA;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( ePLAYER_STATUS_EXT_HEAL, mExtStatusLast[i] );
			}
			break;
			/// 마법 회피율(저항력)
		case ePLAYER_STATUS_EXT_MAVOID:
			{
				float num133 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 133 );

				// C =  (( [방어자] 캐릭터 스탯 저항력 수치 ) * ((100 + [방어자]아이템 저항력 수치[%]값 + [방어자]버프성 저항력 수치[%]값) / 100) + [방어자]아이템 저항력 수치[+-]값 + [방어자]버프성 저항력 수치[+-]값 ) * {num133}
				float valueC = (( mExtStatus[i] ) * (( 100.0f + pItem->mOptPercentResistance + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusResistance + mExtStatusPlus[i] ) * num133;

				mExtStatusLast[i] = valueC;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 물리 공격 크리 감소
		case ePLAYER_STATUS_EXT_CRIDEF:
			{
				float num203 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 203 );
				float num204 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 204 );

				//D =	[방어자] 캐릭터 스탯 물리 방호 수치 * {num203}	
				//E =	D * ((100 +  [방어자]아이템 물리 방호[%]값 + [방어자]버프성 물리 방호 [%]값)  / 100) 
				//		+ ( ([방어자]아이템 물리 방호 수치[+-]값 + [방어자]버프성 물리 방호 수치[+-]값) * {num204} )	
				float valueD = mExtStatus[i] * num203;
				float valueE = valueD * (( 100.0f + pItem->mOptPercentPhysicCriticalDec + mExtStatusPer[i] ) / 100 ) + 
					( ( pItem->mOptPlusPhysicCriticalDec + mExtStatusPlus[i] ) * num204 );

				mExtStatusLast[i] = valueE;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 마법 공격 크리 감소
		case ePLAYER_STATUS_EXT_MCRIDEF:
			{
				float num233 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 233 );
				float num234 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 234 );

				//D =	[방어자] 캐릭터 스탯 마법 방호 수치 * {num233}	
				//E =	D * ((100 +  [방어자]아이템 마법 방호[%]값 + [방어자]버프성 마법 방호 [%]값)  / 100) 
				//		+ ( ([방어자]아이템 마법 방호 수치[+-]값 + [방어자]버프성 마법 방호 수치[+-]값) * {num234} )		
				float valueD = mExtStatus[i] * num233;
				float valueE = valueD * (( 100.0f + pItem->mOptPercentMagicCriticalDec + mExtStatusPer[i] ) / 100 ) + 
					( ( pItem->mOptPlusMagicCriticalDec + mExtStatusPlus[i] ) * num234 );

				mExtStatusLast[i] = valueE;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
		case ePLAYER_STATUS_EXT_DISTRESS:	 /// 어그로 추가/감소
		case ePLAYER_STATUS_EXT_USE_MP:      /// MP소모량 추가/감소
		case ePLAYER_STATUS_EXT_CASTING:     /// 캐스팅 시간 추가/감소
		case ePLAYER_STATUS_EXT_HUMAN_ATK:   /// 인간형 공격력
		case ePLAYER_STATUS_EXT_ANIMAL_ATK:	 /// 동물형 공격력
		case ePLAYER_STATUS_EXT_PLANT_ATK:	 /// 식물형 공격력
		case ePLAYER_STATUS_EXT_BUG_ATK:	 /// 곤충형 공격력
		case ePLAYER_STATUS_EXT_SOUL_ATK:	 /// 정령형 공격력
		case ePLAYER_STATUS_EXT_UNDEAD_ATK:	 /// 언데드형 공격력
		case ePLAYER_STATUS_EXT_GIANT_ATK:	 /// 거인형 공격력
		case ePLAYER_STATUS_EXT_DRAGON_ATK:	 /// 용형 공격력
		case ePLAYER_STATUS_EXT_NORMAL_ATK:  /// 일반 몬스터 공격력
		case ePLAYER_STATUS_EXT_VETERAN_ATK: /// 정예 몬스터 공격력
		case ePLAYER_STATUS_EXT_ELITE_ATK:   /// 엘리트 몬스터 공격력
		case ePLAYER_STATUS_EXT_BOSS_ATK:    /// 보스 몬스터 공격력
		case ePLAYER_STATUS_EXT_CHARACTER_ATK:
			break;
		default:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] ) / 100.0f ) + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				pPlayer->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
		}
	}

	if( pPlayer->GetGameIn() == true )
	{
		pPlayer->SetStateOddity( mOddity );

		/// hp수치를 hp최대치로 넘어가지 않게 셋팅
		if( pPlayer->GetHP() > pPlayer->GetMaxHP() )
			pPlayer->HPHeal( 0, true, pPlayer->GetObject(), 0, eTAKEDAMAGETYPE_NONE );
		
		/// mp수치를 mp최대치로 넘어가지 않게 셋팅
		if( pPlayer->GetMP() > pPlayer->GetMaxMP() )
			pPlayer->MPHeal( 0, true );

		if( pPlayer->GetStateDie() != true )
		{
			if( mMaxHP != pPlayer->GetMaxHP() )
				pPlayer->SendHPHeal();

			if( mMaxMP != pPlayer->GetMaxMP() )
				pPlayer->SendMPHeal();
		}
		else
		{
			if( mMaxHP != pPlayer->GetMaxHP() )
				pPlayer->SendMaxHP();

			if( mMaxMP != pPlayer->GetMaxMP() )
				pPlayer->SendMaxMP();
		}

		/// 치트 이동속도 증가
		pPlayer->CalcCheatSpeedUp();

		/// 공격속도 변경시 발송
		if( pPlayer->GetStatus2()->mAttackSpeed != mAttackSpeed )
			pPlayer->SendAttackSpeed();

		/// 이동 속도가 변한경우 클라이언트에 전송
		if( pPlayer->GetMoveSpeed() != mMoveSpeed )
			pPlayer->SendMoveSpeed();

		/// 크기 계산후 변했으면 클라이언트에도 알림
		if( mFixedObjectSize != 0 )
		{
			pPlayer->CalcFixedObjectSize( (short)mSizePer );
			if( mFixedObjectSize != pPlayer->GetFixedObjectSize() )
				pPlayer->SendPlayerSize();
		}
	}
}
