#include "stdafx.h"
#include "StatusCalc_Client.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "SkillScript.h"
#include "StatusScript.h"
#include "SkillManager.h"
#include "ItemManager.h"
#include "MonsterScript.h"

cStatusCalc_Client* cStatusCalc_Client::mpStatusCalc = NULL;


cStatusCalc_Client::cStatusCalc_Client(void)
{
	if( !mpStatusCalc )
	{
		mpStatusCalc = this;
	}

	::memset( mBaseStatus,	   0, sizeof( mBaseStatus ) );
	::memset( mBaseStatusPlus, 0, sizeof( mBaseStatusPlus ) );
	::memset( mBaseStatusPer,  0, sizeof( mBaseStatusPer ) );
	::memset( mExtStatus,      0, sizeof( mExtStatus ) );
	::memset( mExtStatusPlus,  0, sizeof( mExtStatusPlus ) );
	::memset( mExtStatusPer,   0, sizeof( mExtStatusPer ) );
	::memset( mExtStatusLast,  0, sizeof( mExtStatusLast ) );
	::memset( mExtAddStatus,   0, sizeof( mExtAddStatus ) );

}



cStatusCalc_Client::~cStatusCalc_Client(void)
{
}



void cStatusCalc_Client::CalcPlayerStatusItemSkillAddDetail( eSTATUS_PLUS statusType, float value, bool passive )
{
	unsigned long arrayNum = 0;

	if( eSTATUSPLUS_STR_PLUS <= statusType && statusType < eSTATUSPLUS_BASE_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_STR_PLUS;
		mBaseStatusPlus[arrayNum] = mBaseStatusPlus[arrayNum] + value;
		/// 항목별 색값을 지정한다. 
		SetBaseStatusInfoType( (ePLAYER_STATUS)arrayNum, value, mBaseStatusPlus[arrayNum] );
	}
	else if( eSTATUSPLUS_STR_PERCENT <= statusType && statusType < eSTATUSPLUS_BASE_PERCENT_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_STR_PERCENT;
		mBaseStatusPer[arrayNum] = mBaseStatusPer[arrayNum] + value;

		SetBaseStatusInfoType( (ePLAYER_STATUS)arrayNum, value, mBaseStatusPer[arrayNum] );
	}
	else if( eSTATUSPLUS_ATK_PLUS <= statusType && statusType <= eSTATUSPLUS_EXT_PLUS_MAX )
	{
		arrayNum = statusType - eSTATUSPLUS_ATK_PLUS;
		mExtStatusPlus[arrayNum] = mExtStatusPlus[arrayNum] + value;
		if( passive == false )
			SetExtStatusInfoType( (ePLAYER_STATUS_EXT)arrayNum, value, mExtStatusPlus[arrayNum] );
	}
	/// 기본스텟 5개 전체를 변경
	else if( eSTATUSPLUS_BASEALL_PLUS == statusType )
	{
		for( int i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			mBaseStatusPlus[i] = mBaseStatusPlus[i] + value;
			/// 항목별 색값을 지정한다. 
			SetBaseStatusInfoType( (ePLAYER_STATUS)i, value, mBaseStatusPlus[i] );
		}
	}
	else if( eSTATUSPLUS_BASEALL_PERCENT == statusType )
	{
		for( int i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			mBaseStatusPer[i] = mBaseStatusPer[i] + value;

			SetBaseStatusInfoType( (ePLAYER_STATUS)i, value, mBaseStatusPer[i] );
		}
	}
	/// 공격력 전체를 변경+
	else if( eSTATUSPLUS_ATKTOTAL_PLUS == statusType )
	{
		mExtStatusPlus[ePLAYER_STATUS_EXT_ATK] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_RATK] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_MATK] += value;			
		if( passive == false )
		{
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_ATK, value, mExtStatusPlus[ePLAYER_STATUS_EXT_ATK] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_RATK, value, mExtStatusPlus[ePLAYER_STATUS_EXT_RATK] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_MATK, value, mExtStatusPlus[ePLAYER_STATUS_EXT_MATK] );
		}
	}

	/// 방어력 전체를 변경+
	else if( eSTATUSPLUS_DEFTOTAL_PLUS == statusType )
	{
		mExtStatusPlus[ePLAYER_STATUS_EXT_DEF] += value;
		mExtStatusPlus[ePLAYER_STATUS_EXT_MDEF] += value;
		if( passive == false )
		{
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_DEF, value, mExtStatusPlus[ePLAYER_STATUS_EXT_DEF] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_MDEF, value, mExtStatusPlus[ePLAYER_STATUS_EXT_MDEF] );
		}
	}

	else if( eSTATUSPLUS_ATK_PER <= statusType && statusType <= eSTATUSPLUS_EXT_PER_MAX )
	{

		arrayNum = statusType - eSTATUSPLUS_ATK_PER;
		mExtStatusPer[arrayNum] = mExtStatusPer[arrayNum] + value;

		if( mExtStatusPer[arrayNum] <= -100 )
			mExtStatusPer[arrayNum] = -99;

		/// 패시브는 색변환 안함
		if( passive == false )
		{
			/// 항목별 색값을 지정한다. 
			SetExtStatusInfoType( (ePLAYER_STATUS_EXT)arrayNum, value, mExtStatusPer[arrayNum] );
		}

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

		if( passive == false )
		{
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_ATK, value, mExtStatusPer[ePLAYER_STATUS_EXT_ATK] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_RATK, value, mExtStatusPer[ePLAYER_STATUS_EXT_RATK] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_MATK, value, mExtStatusPer[ePLAYER_STATUS_EXT_MATK] );
		}
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

		if( passive == false )
		{
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_DEF, value, mExtStatusPer[ePLAYER_STATUS_EXT_DEF] );
			SetExtStatusInfoType( ePLAYER_STATUS_EXT_MDEF, value, mExtStatusPer[ePLAYER_STATUS_EXT_MDEF] );
		}
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

}



void cStatusCalc_Client::SetBaseStatusInfoType( ePLAYER_STATUS pos, float value, float totalValue )
{

	if( !( ePLAYER_STATUS_STR <= pos && pos < ePLAYER_STATUS_LEVEL ) )
	{
		assert(NULL);
		return;
	}

	/// 플레이어 정보 읽어옴
	cHero* pHero = OBJECTMANAGER->GetHero();

	/// 현재 컬러 타입을 가져옴
	eStateInfo_Type colorType;
	colorType = pHero->GetBaseStatusInfoType( pos );

	/// 읽어온 수치값이 0 일경우 리턴
	if( value == 0 ) { return; }

	/// 기존 컬러타입이 없는 상태에 +값이 들어 온경우
	else if( colorType == eSTATE_NORMAL && value > 0 )
	{
		/// 컬러타입을 +색으로 설정
		pHero->SetBaseStatusInfoType( pos, eSTATE_PLUS );
	}
	/// 기존 컬러타입이 없는 상태에 -값이 들어 온경우
	else if( colorType == eSTATE_NORMAL && value < 0 )
	{
		/// 컬러타입을 -색으로 설정
		pHero->SetBaseStatusInfoType( pos, eSTATE_MINUS );
	}
	/// 기존 컬러타입이 +상태에서 -값이 들어 온경우 또는 기존 컬러타입이 -상태에서 +값이 들어 온경우
	else if( ( colorType == eSTATE_PLUS && value < 0 ) || ( colorType == eSTATE_MINUS && value > 0 ))
	{
		if( totalValue > 0 )
		{
			/// 컬러타입을 +색으로 설정
			pHero->SetBaseStatusInfoType( pos, eSTATE_OVERLAPPLUS );
		}
		else
		{
			/// 컬러타입을 +색으로 설정
			pHero->SetBaseStatusInfoType( pos, eSTATE_OVERLAPMINUS );
		}
	}
}



void cStatusCalc_Client::SetExtStatusInfoType( ePLAYER_STATUS_EXT pos, float value, float totalValue )
{

	if( !( ePLAYER_STATUS_EXT_ATK <= pos && pos <= ePLAYER_STATUS_EXT_MAX ) )
	{
		assert(NULL);
		return;
	}

	/// 현재 컬러 타입을 가져옴
	eStateInfo_Type colorType;
	colorType = HERO->GetExtStatusInfoType( pos );

	/// 읽어온 수치값이 0 일경우 리턴
	if( value == 0 ) { return; }

	/// 기존 컬러타입이 없는 상태에 +값이 들어 온경우
	else if( colorType == eSTATE_NORMAL && value > 0 )
	{
		/// 컬러타입을 +색으로 설정
		HERO->SetExtStatusInfoType( pos, eSTATE_PLUS );
	}
	/// 기존 컬러타입이 없는 상태에 -값이 들어 온경우
	else if( colorType == eSTATE_NORMAL && value < 0 )
	{
		/// 컬러타입을 -색으로 설정
		HERO->SetExtStatusInfoType( pos, eSTATE_MINUS );
	}
	/// 기존 컬러타입이 +상태에서 -값이 들어 온경우 또는 기존 컬러타입이 -상태에서 +값이 들어 온경우
	else if( ( colorType == eSTATE_PLUS && value < 0 ) || ( colorType == eSTATE_MINUS && value > 0 ))
	{
		if( totalValue > 0 )
		{
			/// 컬러타입을 +색으로 설정
			HERO->SetExtStatusInfoType( pos, eSTATE_OVERLAPPLUS );
		}
		else
		{
			/// 컬러타입을 +색으로 설정
			HERO->SetExtStatusInfoType( pos, eSTATE_OVERLAPMINUS );
		}
	}
}


void cStatusCalc_Client::CalcPlayerBaseOnly()
{
	//HERO->InitStatusColor();

	/// 기본 스텟 초기화
	for( unsigned char i = ePLAYER_STATUS_STR ; ePLAYER_STATUS_LEVEL > i ; ++i )
	{
		mBaseStatus[i] = 0.0f;
	}

	/// 스크립트에서 읽어들인 순수한 기본 스텟값 계산
	CalcPlayerStatus();
}



void cStatusCalc_Client::CalcPlayerExtensionOnly()
{
	HERO->InitStatusInfoType();

	/// 스텟1 + 초기화
	for( unsigned char i = ePLAYER_STATUS_STR ; ePLAYER_STATUS_LEVEL > i ; ++i )
	{
		mBaseStatusPlus[i] = 0.0f;
	}

	/// 스텟1 % 초기화
	for( unsigned char i = ePLAYER_STATUS_STR ; ePLAYER_STATUS_LEVEL > i ; ++i )
	{
		mBaseStatusPer[i] = 0.0f;
	}

	/// 스텟2 + 초기화
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatusPlus[i] = 0;
	}

	/// 스텟2 % 초기화
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatusPer[i] = 0;
	}

	/// 스텟외 스텟
	for( int i = ePLAYER_STATUS_EXT_ADD_EXPADDPER ; ePLAYER_STATUS_EXT_ADD_MAX > i ; ++i )
	{
		mExtAddStatus[i] = 0;
	}

	/// 스킬효과, 스킬패시브, 아이템 데이타를 읽어와 스텟 수치 적용
	CalcPlayerStatusSkillInfluenceAdd();
	CalcPlayerStatusSkillPassiveAdd();
	CalcPlayerStatusItemAdd();
}



unsigned long cStatusCalc_Client::GetParantJob( unsigned long job, unsigned int jobPos )
{

	unsigned long jobCode;

	/// 현재 직업의 부모 직업들을 배열로 받아온다.
	tArray<unsigned long>* pParentJob;

	/// 상위 직업 코드를 읽어옮
	pParentJob = GAMERESOURCEMAN->GetParentJob( job );

	/// 데이터가 없다 - 현재 직업이 기본 직업 이다.
	if( pParentJob == NULL )
	{
		return NULL;
	}

	/// 사이즈보다 크다 - 현재 직업이 최종 직업이다.
	if( jobPos > pParentJob->GetSize() - 1 )
	{
		return NULL;
	}

	/// 전직 차수에 해당하는 직업의 코드드를 읽어간다.
	pParentJob->GetAt( &jobCode, jobPos );

	return jobCode;
}



void cStatusCalc_Client::CalcPlayerStatusSkillInfluenceAdd()
{
	cPointerHashMap::cIterator iter;

	cInfluenceObject* pInfluenceObject = NULL;
	sInfluenceScript* pInfluenceScript = NULL;
	cPointerHashMap* influenceMap = HERO->GetBufMap();
	if( influenceMap != NULL )
	{
		iter = influenceMap->Begin();
		while( iter != influenceMap->End() )
		{
			pInfluenceObject = (cInfluenceObject*)((*iter).mSecond);
			++iter;

			pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pInfluenceObject->GetInfluenceIdx() );
			/// 지속적인 효과가 아닌경우만(데미지)
			if( pInfluenceScript->mTime1 == 0 )
			{
				if( pInfluenceScript->mStatusPlusIdx1 != 0 )
				{
					CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx1),
						(float)(pInfluenceScript->mValue1), false );
				}
			}
			/// 지속적인 효과가 아닌경우만(데미지)
			if( pInfluenceScript->mTime2 == 0 )
			{
				if( pInfluenceScript->mStatusPlusIdx2 != 0 )
				{
					CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx2),
						(float)(pInfluenceScript->mValue2), false );
				}
			}
			if( pInfluenceScript->mStatusPlusIdx3 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx3),
					(float)(pInfluenceScript->mValue3), false );
			}
		}
	}

	influenceMap = HERO->GetDeBufMap();
	if( influenceMap != NULL )
	{
		iter = influenceMap->Begin();
		while( iter != influenceMap->End() )
		{
			pInfluenceObject = (cInfluenceObject*)((*iter).mSecond);
			++iter;

			pInfluenceScript = SKILLSCRIPT->GetInfluenceInfo( pInfluenceObject->GetInfluenceIdx() );
			/// 지속적인 효과가 아닌경우만(데미지)
			if( pInfluenceScript->mTime1 == 0 )
			{
				if( pInfluenceScript->mStatusPlusIdx1 != 0 )
				{
					CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx1),
						(float)(pInfluenceScript->mValue1), false );
				}
			}
			if( pInfluenceScript->mStatusPlusIdx2 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx2),
					(float)(pInfluenceScript->mValue2), false );
			}
			if( pInfluenceScript->mStatusPlusIdx3 != 0 )
			{
				CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pInfluenceScript->mStatusPlusIdx3),
					(float)(pInfluenceScript->mValue3), false );
			}
		}
	}
}



void cStatusCalc_Client::CalcPlayerStatusSkillPassiveAdd()
{

	cPointerHashMap::cIterator iter;

	unsigned long skillIdx = 0;
	sKeepSkill* pKeepSkill = NULL;

	sPlayerSkillBaseInfo* pSkillInfo = NULL;
	sPlayerSkillStepInfo* pSkillStep = NULL;

	cPointerHashMap* keepSkillMap = SKILLMAN->GetKeepSkillMap();
	if( keepSkillMap != NULL )
	{
		iter = keepSkillMap->Begin();
		while( iter != keepSkillMap->End() )
		{
			skillIdx = (*iter).mFirst;
			pKeepSkill = (sKeepSkill*)((*iter).mSecond);
			if( pKeepSkill == NULL )
			{
				assert(NULL);
				return;
			}
			++iter;

			pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );

			/// 패시브만 효과값 적용
			if( pSkillInfo->mType != eSKILLTYPE_PASSIVE )
			{
				continue;
			}

			if( pSkillInfo->mStepCount <= pKeepSkill->mStep )
			{
				assert(0);
			}

			pSkillStep = &pSkillInfo->mpSetpInfoArray[ pKeepSkill->mStep ];


			/// 내 장비가 맞는지 체크
			bool IsEquipment = true;
			if( pSkillInfo->mUseEquipment )
			{
				cItem* rightWeapon = ITEMMAN->GetHeroRightWeapon();
				if( !rightWeapon )
				{
					assert(0);
					return;
				}
				
				cItem* leftWeapon = ITEMMAN->GetHeroLeftWeapon();
				if( !leftWeapon )
				{
					assert(0);
					return;
				}

				int rType = rightWeapon->GetDefine()? rightWeapon->GetDefine()->GetSubType() : -1;
				int lType = leftWeapon->GetDefine()? leftWeapon->GetDefine()->GetSubType() : -1;

				/// 착용 장비 체크
				switch( pSkillInfo->mUseEquipment )
				{
				case eEQUIPTYPE_SWORD:
					if( rType != ITEM_WEAPON_SWORD )
						IsEquipment = false;
					break;
				case eEQUIPTYPE_LONGSWORD:
					if( rType != ITEM_WEAPON_BLADE )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_DOUBLESWORD:
					if( rType != ITEM_WEAPON_DUAL )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SHORTSWORD:
					if( rType != ITEM_WEAPON_CUTTER )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_GUN:
					if( rType != ITEM_WEAPON_GUN )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_STAFF:
					if( rType != ITEM_WEAPON_STAFF )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SHIELD:
					if( lType != ITEM_WEAPON_SHIELD )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SWORD_LONGSWORD:
					if( rType != ITEM_WEAPON_SWORD &&
						rType != ITEM_WEAPON_BLADE )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SWORD_DOUBLESWORD:
					if( rType != ITEM_WEAPON_SWORD &&
						rType != ITEM_WEAPON_DUAL )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
					if( rType != ITEM_WEAPON_BLADE &&
						rType != ITEM_WEAPON_DUAL )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
					if( rType != ITEM_WEAPON_SWORD &&
						rType != ITEM_WEAPON_BLADE &&
						rType != ITEM_WEAPON_DUAL )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_SHORTSWORD_GUN:
					if( rType != ITEM_WEAPON_CUTTER &&
						rType != ITEM_WEAPON_GUN )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_CLOTH:
					if( HERO->GetArmorSets() != ARMOR_SETS_ROBES )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_LIGHTARMOR:
					if( HERO->GetArmorSets() != ARMOR_SETS_LIGHT )
						IsEquipment =  false;
					break;
				case eEQUIPTYPE_HEAVYARMOR:
					if( HERO->GetArmorSets() != ARMOR_SETS_HEAVY )
						IsEquipment =  false;
					break;
				}
			}

			if( IsEquipment == true )
			{
				if( pSkillStep->mApplyValueType1 != 0 )
				{
					CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pSkillStep->mApplyValueType1),
						(float)(pSkillStep->mApplyValue1), true );
				}
				if( pSkillStep->mApplyValueType2 != 0 )
				{
					CalcPlayerStatusItemSkillAddDetail( (eSTATUS_PLUS)(pSkillStep->mApplyValueType2),
						(float)(pSkillStep->mApplyValue2), true );
				}
			}
		}
	}
}



void cStatusCalc_Client::CalcPlayerStatusItemAdd()
{
	sEquipAbility* pEquipAbility = ITEMMAN->GetBasicAbil();
	if( pEquipAbility == NULL )
		return;

	pEquipAbility = ITEMMAN->GetBasicAbil();

	if( pEquipAbility->mOptStr != 0 )	/// 증가:힘
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_STR_PLUS, (float)pEquipAbility->mOptStr, false );
	}
	if( pEquipAbility->mOptDex != 0 )	/// 증가:민첩
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_DEX_PLUS, (float)pEquipAbility->mOptDex, false );
	}
	if( pEquipAbility->mOptCon != 0 )	/// 증가:체력
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_CON_PLUS, (float)pEquipAbility->mOptCon, false );
	}
	if( pEquipAbility->mOptInt != 0 )	/// 증가:지능
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_INT_PLUS, (float)pEquipAbility->mOptInt, false );
	}
	if( pEquipAbility->mOptWis != 0 )	/// 증가:지혜
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_WIS_PLUS, (float)pEquipAbility->mOptWis, false );
	}

/*	// 아직 아이템 옵션 적용안되는 시점임 [7/13/2010 JO_CLIENT]
	//
	if( pEquipAbility->mOptPercentStr != 0 )	/// 증가:힘(%)
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_STR_PERCENT, (float)pEquipAbility->mOptPercentStr, false );
	}
	if( pEquipAbility->mOptPercentDex != 0 )	/// 증가:민첩(%)
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_DEX_PERCENT, (float)pEquipAbility->mOptPercentDex, false );
	}
	if( pEquipAbility->mOptPercentCon != 0 )	/// 증가:체력(%)
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_CON_PERCENT, (float)pEquipAbility->mOptPercentCon, false );
	}
	if( pEquipAbility->mOptPercentInt != 0 )	/// 증가:지능(%)
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_INT_PERCENT, (float)pEquipAbility->mOptPercentInt, false );
	}
	if( pEquipAbility->mOptPercentWis != 0 )	/// 증가:지혜(%)
	{
		CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_WIS_PERCENT, (float)pEquipAbility->mOptPercentWis, false );
	}
*/
	//if( pEquipAbility->mOptPlusWei != 0 )	/// 증가:무게
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_WEIGHT_PLUS, (float)pEquipAbility->mOptPlusWei, colorChange );
	//}
	//if( pEquipAbility->mOptPlusMov != 0 )	/// 증가:이동속도
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_SPD_PLUS, (float)pEquipAbility->mOptPlusMov, colorChange );
	//}
	//if( pEquipAbility->mOptPlusMaxHp != 0 )	/// 증가:최대HP 
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_HP_PLUS, (float)pEquipAbility->mOptPlusMaxHp, colorChange );
	//}
	//if( pEquipAbility->mOptPlusMaxMp != 0 )	/// 증가:최대MP 
	//{
	//	CalcPlayerStatusItemSkillAddDetail( eSTATUSPLUS_MP_PLUS, (float)pEquipAbility->mOptPlusMaxMp, colorChange );
	//}
}



void cStatusCalc_Client::CalcPlayerStatus()
{
	unsigned char level = HERO->GetLevel();
	ePLAYER_JOB job = HERO->GetJob();

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
		tArray<unsigned long>* playerJobParent = GAMERESOURCEMAN->GetParentJob( job );
		if( playerJobParent == NULL )
		{
			/// 자신의 직업도 없는데 부모의 직업도 없으면 오류
			assert(NULL);
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
			return;
		}

		for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
		{
			/// 클래스기준에 따른 증가치 만큼 증가
			mBaseStatus[i] = mBaseStatus[i] + pStatusMultipl->mBaseStatus[i];
		}
	}
}



void cStatusCalc_Client::CalcPlayerExtStatus( ePLAYER_STATUS_EXT statusExt )
{
	ePLAYER_JOB job = HERO->GetJob();

	/// 플레이어의 스텟1 최종값을 읽어옴 + 레벨값까지
	float baseStatus[ePLAYER_STATUS_LEVEL_MAX];
	unsigned char i = ePLAYER_STATUS_STR;
	for( ; i < ePLAYER_STATUS_LEVEL ; ++i )
	{
		baseStatus[i] = (mBaseStatus[i] + mBaseStatusPlus[i]);

		if( mBaseStatusPer[i] != 0 )
			baseStatus[i] += (mBaseStatus[i] + mBaseStatusPlus[i]) * (mBaseStatusPer[i]*0.01f);
		
		if( baseStatus[i] < 0.0f )
			baseStatus[i] = 0.0f;
	}
	baseStatus[i] = HERO->GetLevel(); 

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
		statusValue = (float)STATUSSCRIPT->GetStatusExtBaseInfo( statusExt, job );
		//if( FloatToInt(statusValue) == 0 )
		//{
		//	if( playerJobParent != 0 )
		//	{
		//		/// 상위 직업이 있으면 
		//		for( int i = playerJobParent->GetSize() - 1 ; 0 <= i  ; --i )
		//		{
		//			/// 상위 직업코드로 스크립트 값을 읽어옴
		//			statusValue = (float)STATUSSCRIPT->GetStatusExtBaseInfo( statusExt, (*playerJobParent)[i] );

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
	tArray<unsigned long>* playerJobParent = GAMERESOURCEMAN->GetParentJob( job );
	unsigned long firstJob = HERO->GetJob();
	if( playerJobParent != NULL )
		firstJob = (*playerJobParent)[0];

	/// 1차 직업에 따라 공격력 스텟 비중치 읽어올 항목값 설정
	unsigned long plusStatus = 0;
	switch( firstJob )
	{		
	case ePLAYER_FIGHTER:	/// 파이터
	case ePLAYER_ARMSMAN:	/// 암즈맨
		plusStatus = 0;
		break;
	case ePLAYER_WANDERER:	/// 원더러
	case ePLAYER_RANGER:	/// 레인저
		plusStatus = 1;
		break;
	case ePLAYER_MAGE:		/// 메이지
	case ePLAYER_WARLOCK:	/// 워록
		plusStatus = 2;
		break;
	}

	float oriMaxMP   = statusValue; 
	float importance = 0.0f;
	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL_MAX ; ++i )
	{
		eEXT_MULTIPL chgStatusExt = eEXT_MULTIPL_MAX;
		switch( statusExt )
		{
		case ePLAYER_STATUS_EXT_ATK:
			chgStatusExt = (eEXT_MULTIPL)( eEXT_MULTIPL_ATK_FIGHTER + plusStatus );		/// 근거리, 원거리, 마법 직업에 따른 공격력 항목 설정
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
		case ePLAYER_STATUS_EXT_MAXHP:		chgStatusExt = eEXT_MULTIPL_MAXHP;		break;	/// HP 
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

	if( statusExt == ePLAYER_STATUS_EXT_MAXMP )
		HERO->SetOriMpMax( FloatToInt( oriMaxMP ) );

	/// 결과 값을 기록
	mExtStatus[statusExt] = mExtStatus[statusExt] + statusValue;

	if( statusExt == ePLAYER_STATUS_EXT_WEIGHT )
	{
		sHeroInfo* pHeroInfo = HERO->GetHeroInfo();
		if( pHeroInfo )
		{
			switch( pHeroInfo->BagEnd )
			{
			case INVENTORY_BAG1_END: mExtStatus[statusExt] += ADD_BAG1_WEIGHT; break;
			case INVENTORY_BAG2_END: mExtStatus[statusExt] += ADD_BAG2_WEIGHT; break;
			case INVENTORY_BAG3_END: mExtStatus[statusExt] += ADD_BAG3_WEIGHT; break;
			}					
		}
		else
		{
			assert(0);
		}
	}

	if( mExtStatus[statusExt] < 0.0f ) 
		mExtStatus[statusExt] = 0.0f;
}



void cStatusCalc_Client::CalcPlayerInit()
{
	CalcPlayerBaseOnly();
	CalcPlayerExtensionOnly();
	CalcPlayerGlobal();
}



void cStatusCalc_Client::CalcPlayerBaseGlobal()
{
	CalcPlayerBaseOnly();
	CalcPlayerGlobal();
}



void cStatusCalc_Client::CalcPlayerExtensionGlobal()
{
	CalcPlayerExtensionOnly();
	CalcPlayerGlobal();
}



void  cStatusCalc_Client::CalcPlayerGlobal()
{

	/// 확장 스텟 베이스 초기화
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatus[i] = 0;
	}

	/// 확장 스텟 최종값
	for( int i = ePLAYER_STATUS_EXT_ATK ; ePLAYER_STATUS_EXT_MAX > i ; ++i )
	{
		mExtStatusLast[i] = 0;
	}

	sEquipAbility* pItem = ITEMMAN->GetBasicAbil();

	for( unsigned char i = ePLAYER_STATUS_EXT_ATK ; i < ePLAYER_STATUS_EXT_MAX ; ++i )
	{
		/// 스크립트에서 읽어들인 순수한 확장 스텟값 계산
		CalcPlayerExtStatus( (ePLAYER_STATUS_EXT)i );

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
				float valueB = pItem->mPhysicMinAttack + ( HERO->GetLevel() * num301 );
				float valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_ATKMin, mExtStatusLast[i] );

				valueB = pItem->mPhysicMaxAttack + ( HERO->GetLevel() * num301 );
				valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_ATKMax, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicAttack, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicAttack, mExtStatusLast[i] );

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
				float valueB = pItem->mPhysicMinAttack + ( HERO->GetLevel() * num331 );
				float valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_RATKMin, mExtStatusLast[i] );

				valueB = pItem->mPhysicMaxAttack + ( HERO->GetLevel() * num331 );
				valueC = (( 100.0f + mExtStatusPer[i] + pItem->mOptPercentPhysicAttack ) / 100.0f );
				mExtStatusLast[i] = (( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_RATKMax, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicAttack, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicAttack, mExtStatusLast[i] );

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
				float valueC = ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentPhysicDefense ) ) / 100.0f );
				mExtStatusLast[i] = ( ( valueA / num401 + valueB / num402 ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusPhysicDefense;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicDefense != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicDefense, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicDefense != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicDefense, mExtStatusLast[i] );
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
				float valueB = pItem->mMagicMinAttack + ( HERO->GetLevel() * num362 );
				float valueC = ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMagicAttack ) ) / 100.0f );
				mExtStatusLast[i] = ( ( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusMagicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_MATKMin, mExtStatusLast[i] );

				valueB = pItem->mMagicMaxAttack + ( HERO->GetLevel() * num362 );
				valueC = ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMagicAttack ) ) / 100.0f );
				mExtStatusLast[i] = ( ( valueA + valueB ) * valueC ) + mExtStatusPlus[i] + pItem->mOptPlusMagicAttack;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( ePLAYER_STATUS_EXT_MATKMax, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMagicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMagicAttack, mExtStatusLast[i] );
				if( pItem->mOptPlusMagicAttack != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMagicAttack, mExtStatusLast[i] );

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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMagicDefense != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMagicDefense, mExtStatusLast[i] );
				if( pItem->mOptPlusMagicDefense != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMagicDefense, mExtStatusLast[i] );

			}
			break;
			/// 물리 공격 크리
		case ePLAYER_STATUS_EXT_CRI:
			{
				float num201 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 201 );
				float num202 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 202 );

				// 크리티컬 발생 확률(물리)
				//A =	[공격자] 캐릭터 스탯 물리 크리티컬 수치 * {num201}	
				//B =	A * ((100 +  [공격자]아이템 물리 크리티컬[%]값 + [공격자]버프성 물리 크리티컬 [%]값)  / 100) + ( ([공격자]아이템 물리 크리티컬 수치[+-]값 + [공격자]버프성 물리 크리티컬 수치[+-]값) * {num202} )	
				float valueA = mExtStatus[i] * num201;
				float valueB = valueA * (( 100.0f + pItem->mOptPercentPhysicCritical + mExtStatusPer[i] ) / 100.0f ) + (( pItem->mOptPlusPhysicCritical + mExtStatusPlus[i] ) * num202 );

				mExtStatusLast[i] = valueB;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicCritical != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicCritical, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicCritical != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicCritical, mExtStatusLast[i] );
			}
			break;
			///	마법 공격 크리
		case ePLAYER_STATUS_EXT_MCRI:
			{
				float num231 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 231 );
				float num232 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 232 );

				// 크리티컬 발생 확률(마법)
				//A =	[공격자] 캐릭터 스탯 마법 크리티컬 수치 * {num231}
				//B =	A  * ((100 + [공격자]아이템 마법 크리티컬[%]값 + [공격자]버프성 마법 크리티컬 [%]값)  / 100) + (([공격자]아이템 마법 크리티컬 수치[+-]값 + [공격자]버프성 마법 크리티컬 수치[+-]값) * {num232} )	
				float valueA = mExtStatus[i] * num231;
				float valueB = valueA * (( 100.0f + pItem->mOptPercentMagicCritical + mExtStatusPer[i] ) / 100.0f ) + (( pItem->mOptPlusMagicCritical + mExtStatusPlus[i] ) * num232 );

				mExtStatusLast[i] = valueB;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMagicCritical != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMagicCritical, mExtStatusLast[i] );
				if( pItem->mOptPlusMagicCritical != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMagicCritical, mExtStatusLast[i] );

			}break;
			/// 최대 HP
		case ePLAYER_STATUS_EXT_MAXHP:
			{
				float num20 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 20 );

				/// 기본 hp량 + ( 레벨 * num1 )
				mExtStatus[i] = mExtStatus[i] + ( HERO->GetLevel() * num20 );    
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMaxHp ) ) / 100.0f )
					+ mExtStatusPlus[i] + pItem->mOptPlusMaxHp;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// 최대 MP
		case ePLAYER_STATUS_EXT_MAXMP:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentMaxMp ) ) / 100.0f ) + 
					mExtStatusPlus[i] + pItem->mOptPlusMaxMp;

				if( mExtStatusLast[i] < 1.0f )
					mExtStatusLast[i] = 1.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
			/// HP 자연 회복량
		case ePLAYER_STATUS_EXT_RECOVHP:
			{
				float num1001 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 1001 );

				float defaultValueHP = (float)STATUSSCRIPT->GetStatusExtBaseInfo( (ePLAYER_STATUS_EXT)i, HERO->GetJob() );

				// 캐릭터 회복 - HP 회복
				// A =	클래스 기본값
				// B =	( 캐릭터 스탯 회복 수치 * {num1001} ) * ((100 + 아이템 HP회복 [%]값+ 버프성 HP회복 [%]값) / 100) + 아이템 HP회복 [+-]값
				// A + B + 버프성 HP회복 [+-]값
				float valueA = defaultValueHP;
				float valueB = ( mExtStatus[i] * num1001 ) * (( 100.0f + pItem->mOptPercentHp + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusHp;
				mExtStatusLast[i] = valueA + valueB + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentHp != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentHp, mExtStatusLast[i] );
				if( pItem->mOptPlusHp != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusHp, mExtStatusLast[i] );
			}
			break;
			/// MP 자연 회복량
		case ePLAYER_STATUS_EXT_RECOVMP:
			{
				float num1101 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 1101 );

				float defaultValueMP = (float)STATUSSCRIPT->GetStatusExtBaseInfo( (ePLAYER_STATUS_EXT)i, HERO->GetJob() );

				// 캐릭터 회복 - MP 회복
				// A =	클래스 기본값
				// B =	( 캐릭터 스탯 회복 수치 * {num1101} ) * ((100 + 아이템 MP회복 [%]값 + 버프성 MP회복 [%]값) / 100)  + 아이템 MP회복 [+-]값
				// A + B + 버프성 MP회복 [+-]값
				float valueA = defaultValueMP;
				float valueB = ( mExtStatus[i] * num1101 ) * ( ( 100.0f + pItem->mOptPercentMp + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusMp;
				mExtStatusLast[i] = valueA + valueB + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMp != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMp, mExtStatusLast[i] );
				if( pItem->mOptPlusMp != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMp, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicAttackRate != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicAttackRate, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicAttackRate != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicAttackRate, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMagicAttackRate != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMagicAttackRate, mExtStatusLast[i] );
				if( pItem->mOptPlusMagicAttackRate != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMagicAttackRate, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentDod != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentDod, mExtStatusLast[i] );
				if( pItem->mOptPlusDod != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusDod, mExtStatusLast[i] );
			}
			break;
			/// 무게
		case ePLAYER_STATUS_EXT_WEIGHT:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + ( mExtStatusPer[i] + pItem->mOptPercentWei ) ) / 100.0f ) + 
					mExtStatusPlus[i] + pItem->mOptPlusWei;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );                  
			}break;
			/// 이동 속도 계산
		case ePLAYER_STATUS_EXT_SPD:
			{
				/// 몬스터 변신중일 경우 이동속도 기본값을 몬스터에서 읽어온다.
				if( HERO->GetTransMonsterClassIdx() != 0 )
				{
					sMonsterScript* pMScript = MONSTERSCRIPT->GetMonsterListInfo( HERO->GetTransMonsterClassIdx() );
					if( pMScript != NULL )
						mExtStatus[i] = (float)pMScript->mActionRunSpd;

					mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] + pItem->mOptPercentMov ) / 100.0f ) + 
						mExtStatusPlus[i] + pItem->mOptPlusMov;

					if( mExtStatusLast[i] < 0.0f )
						mExtStatusLast[i] = 0.0f;

					HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
				}
				else
				{
					mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] + pItem->mOptPercentMov  ) / 100.0f ) + 
						mExtStatusPlus[i] + pItem->mOptPlusMov;

					if( mExtStatusLast[i] < 0.0f )
						mExtStatusLast[i] = 0.0f;

					HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
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

				HERO->SetStatus2( ePLAYER_STATUS_EXT_HEAL, mExtStatusLast[i] );
			}
			break;
			/// 마법 회피율(저항)
		case  ePLAYER_STATUS_EXT_MAVOID:
			{
				float num133 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 133 );

				// C =  (( [방어자] 캐릭터 스탯 저항력 수치 ) * ((100 + [방어자]아이템 저항력 수치[%]값 + [방어자]버프성 저항력 수치[%]값) / 100) + [방어자]아이템 저항력 수치[+-]값 + [방어자]버프성 저항력 수치[+-]값 ) * {num133}
				float valueC = (( mExtStatus[i] ) * (( 100.0f + pItem->mOptPercentResistance + mExtStatusPer[i] ) / 100.0f ) + pItem->mOptPlusResistance + mExtStatusPlus[i] ) * num133;

				mExtStatusLast[i] = valueC;

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentDod != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentResistance, mExtStatusLast[i] );
				if( pItem->mOptPlusDod != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusResistance, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentPhysicCriticalDec != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentPhysicCriticalDec, mExtStatusLast[i] );
				if( pItem->mOptPlusPhysicCriticalDec != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusPhysicCriticalDec, mExtStatusLast[i] );
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

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );

				/// 확장항목에 수치가 있는경우 색변환
				if( pItem->mOptPercentMagicCriticalDec != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPercentMagicCriticalDec, mExtStatusLast[i] );
				if( pItem->mOptPlusMagicCriticalDec != 0 )
					SetExtStatusInfoType( (ePLAYER_STATUS_EXT)i, pItem->mOptPlusMagicCriticalDec, mExtStatusLast[i] );
			}
			break;
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
			break;
		default:
			{
				mExtStatusLast[i] = mExtStatus[i] * ( ( 100.0f + mExtStatusPer[i] ) / 100.0f ) + mExtStatusPlus[i];

				if( mExtStatusLast[i] < 0.0f )
					mExtStatusLast[i] = 0.0f;

				HERO->SetStatus2( (ePLAYER_STATUS_EXT)i, mExtStatusLast[i] );
			}
			break;
		}
	}

	/// 최종 결과를 플레이어에 기록
	for( unsigned char i = ePLAYER_STATUS_STR ; i < ePLAYER_STATUS_LEVEL ; ++i )
	{
		HERO->SetStatus1( (ePLAYER_STATUS)i, mBaseStatus[i] );
		HERO->SetStatus1Plus( (ePLAYER_STATUS)i, mBaseStatusPlus[i] );
		HERO->SetStatus1Per( (ePLAYER_STATUS)i, mBaseStatusPer[i] );
	}

	for( unsigned char i = ePLAYER_STATUS_EXT_ADD_EXPADDPER ; i < ePLAYER_STATUS_EXT_ADD_MAX ; ++i )
	{
		HERO->SetStatusEtc( (ePLAYER_STATUS_EXT_ADD)i, mExtAddStatus[i] );
	}

}



float cStatusCalc_Client::GetStatus2Plus( ePLAYER_STATUS_EXT status2 )
{ 
	if( status2 >= ePLAYER_STATUS_EXT_MAX )
	{
		assert(NULL);
		return 0.0f;
	}

	return mExtStatusPlus[status2];
}


float cStatusCalc_Client::GetStatus2Per( ePLAYER_STATUS_EXT status2 )
{ 
	if( status2 >= ePLAYER_STATUS_EXT_MAX )
	{
		assert(NULL);
		return 0.0f;
	}

	return mExtStatusPer[status2];
}
