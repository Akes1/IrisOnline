#include "gamesrv.h"

#include "stdafx.h"
#include "InfluenceObject.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "StatusCalc_Server.h"
#include "ObjectManager.h"
#include "Protocol.h"
#include "Skill_Common.h"
#include "Player.h"
#include "Totem.h"
#include "DamageCalc.h"
#include "DuelManager.h"
#include "Party.h"
#include "PartyManager.h"
#include "GridManager.h"
#include "PartyUnion.h"
#include "PartyUnionManager.h"
#include "AppTimer.h"


cInfluenceObject::cInfluenceObject()
{
	mTime1 = 0;
	mTime2 = 0;

	mSkillIdx = 0;
	mSwitchType = eSWITCHTYPE_NORMAL;
	mParentIdx = 0;
	mCopyRange = 0;
	mCopyApplyType = eAPPLYTYPE_SELF;
	mAttriButeType = eATTRIBUTETYPE_PHYSICAL;
	mCopyCheckTime = 0;

	mInfluenceValue1 = 0;
	mInfluenceValue2 = 0;

	mProcessStart = true;
	mSendDelMsg = true;
	mDelStatusCalc = true;
	mGuardType = eINFGUARDTYPE_NONE;
	mLeftGuardCnt = 0;
	mLeftGuardValue = 0;

//	mIsTotem = false;

	mIsDelReady = false;
	mDistressPoint = 0;

//	mDieDelete = true;

	//mCalcTempValue1 = 0;
	//mCalcTempValue2 = 0;
	//mSkillRangeType = eRANGETYPE_CLOSE;
	mDotTotalTime = 0;

	mInfGameRestTime = 0;
	mInfRealEndTime = TIMER->GetRealTime();
	mDotTotalTime = 0;

	mParentIdx = 0;

	mAttackerLv = 0;
	mAttackerDamageMin = 0;
	mAttackerDamageMax = 0;
	mHeal = 0;

	mDotValueAcuumTime = 0;

	mGuardType = eINFGUARDTYPE_NONE;
}



cInfluenceObject::~cInfluenceObject()
{
	SKILLMANAGER->DelInfluenceUniqueIdx( mUniqueIdx );
}



void* cInfluenceObject::operator new( size_t n )
{
	if( n != sizeof(cInfluenceObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocInfluenceObject();
}



void cInfluenceObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cInfluenceObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreeInfluenceObject( static_cast<cInfluenceObject*>(ptr) );

	return;
}


bool cInfluenceObject::Init( sObject attacker, sObject target, unsigned long uniqueIdx, unsigned long infClassIdx, unsigned long skillClassIdx, 
							eSWITCH_TYPE infType, unsigned long parentIdx, unsigned long dotTotalTime, unsigned long* pPlusTime )
{

	/// 효과 스크립트 정보 읽어옴
	sInfluenceScript* pInfluence = SKILLSCRIPT->GetInfluenceInfo( infClassIdx );
	if( !pInfluence )	
	{ 
		NETWORK2->PostServerEvent( "influence init - GetInfluenceInfo(%d)", infClassIdx );
		return false; 
	}

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent( "cInfluenceObject::Init - pTarget == NULL", infClassIdx );
		return false;
	}

	mUniqueIdx = uniqueIdx;
	mDBIdx = 0;
	mInfluenceClassIdx = infClassIdx;
	mTarget = target;
	mAttacker = attacker;
	mInfluenceCreationTime = NETWORK2->GetAccumTime();
	mSkillIdx = skillClassIdx;
	mIsSleep = false;
	mGuardType = eINFGUARDTYPE_NONE;
	mLeftGuardCnt = 0;
	mLeftGuardValue = 0;
	mIsRealTime = pInfluence->mTimeType == eINFTIMETYPE_REAL;
	mInfRealEndTime = TIMER->GetRealTime();
	mDotTotalTime = 0;
	mDotValueAcuumTime = NETWORK2->GetAccumTime() + AURAMP_CHECK_TIME;;

	if( pInfluence->mStatusPlusIdx1 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;
	if( pInfluence->mStatusPlusIdx2 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;
	if( pInfluence->mStatusPlusIdx3 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;

	bool timeDelay = mIsSleep;
	if( pInfluence->mStatusPlusIdx1 == eSTATUSPLUS_STUN )
		timeDelay = true;
	if( pInfluence->mStatusPlusIdx2 == eSTATUSPLUS_STUN )
		timeDelay = true;
	if( pInfluence->mStatusPlusIdx3 == eSTATUSPLUS_STUN )
		timeDelay = true;

	/// 스크립트에서 중복으로 설정했는지 체크
	mGuardType = eINFGUARDTYPE_NONE;
	mChangeMPType = eINFCHANGEMPTYPE_NONE;
	mChangeMPValue = 100;
	unsigned short statusIdx[3] = { pInfluence->mStatusPlusIdx1, pInfluence->mStatusPlusIdx2, pInfluence->mStatusPlusIdx3 };
	unsigned long statusValue[3] = { pInfluence->mValue1, pInfluence->mValue2, pInfluence->mValue3 };

	for( int i = 0 ; i < 3 ; ++i )
	{
		switch( statusIdx[i] )
		{
		case eSTATUSPLUS_GUARDCNT_PHY:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_PHY )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR1 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_PHY;
				mLeftGuardCnt = mLeftGuardCnt + (unsigned char)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_PHY:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_PHY )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR2 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_PHY;
				mLeftGuardValue = mLeftGuardValue + (unsigned short)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDCNT_MAG:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_MAG )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR3 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_MAG;
				mLeftGuardCnt = mLeftGuardCnt + (unsigned char)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_MAG:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_MAG )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR4 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_MAG;
				mLeftGuardValue = mLeftGuardValue + (unsigned short)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDCNT_ALL:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_ALL )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR5 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_ALL;
				mLeftGuardCnt = mLeftGuardCnt + (unsigned char)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_ALL:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_ALL )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType ERROR6 %d", infClassIdx );
					return false;
				}
				mGuardType = eINFGUARDTYPE_ALL;
				mLeftGuardValue = mLeftGuardValue + (unsigned short)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDMP_PHY:
			{
				mChangeMPType = eINFCHANGEMPTYPE_PHY;
				mChangeMPValue = (unsigned char)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDMP_MAG:
			{
				mChangeMPType = eINFCHANGEMPTYPE_MAG;
				mChangeMPValue = (unsigned char)statusValue[i];
			}
			break;
		case eSTATUSPLUS_GUARDMP_ALL:
			{
				mChangeMPType = eINFCHANGEMPTYPE_ALL;
				mChangeMPValue = (unsigned char)statusValue[i];
			}
			break;
		}
	}

	for( int i = 0 ; i < 3 ; ++i )
	{
		switch( statusIdx[i] )
		{
		case eSTATUSPLUS_CASHEXPGUARDCNT:
			{
				if( mGuardType != eINFGUARDTYPE_NONE || mChangeMPType != eINFCHANGEMPTYPE_NONE )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init eSTATUSPLUS_CASHEXPGUARDCNT %d", infClassIdx );
					return false;
				}

				mGuardType = eINFGUARDTYPE_CASHEXP;
				mLeftGuardCnt = (unsigned char)statusValue[i];
			}
		}
	}

	switch( mGuardType )
	{
	case eINFGUARDTYPE_CASHEXP:
		if( mLeftGuardCnt == 0 )
		{
			NETWORK2->PostServerEvent("cInfluenceObject::Init %d != eINFGUARDTYPE_NONE && ( %d == 0 || %d == 0 ) %d", mGuardType, mLeftGuardCnt, mLeftGuardValue, infClassIdx );
			return false;
		}
		break;
	case eINFGUARDTYPE_NONE:
		break;
	default:
		if( mLeftGuardCnt == 0 || mLeftGuardValue == 0 )
		{
			NETWORK2->PostServerEvent("cInfluenceObject::Init %d != eINFGUARDTYPE_NONE && ( %d == 0 || %d == 0 ) %d", mGuardType, mLeftGuardCnt, mLeftGuardValue, infClassIdx );
			return false;
		}
		break;			
	}

	mSwitchType = infType;

	switch( mSwitchType )
	{
	case eSWITCHTYPE_NORMAL:
		{
			/// 효과 시간이 -1이면 오라효과다
			if( pInfluence->mContinuanceTime == -1 )
			{
				mSwitchType = eSWITCHTYPE_PARENT;
			}
			else	/// 일반 효과 설정
			{
				if( mIsRealTime == true )
				{
					mInfRealEndTime = TIMER->GetRealPlusTime( pInfluence->mContinuanceTime );
				}
				else
				{
					mInfRealEndTime = TIMER->GetRealTime();

					/// plusTime을 적용시키고 적용된 값을리턴해 그값만큼 클라이언트에게 알려 지연시킨다.
					long plusTime = 0;
					if( pPlusTime != NULL )
					{
						if( timeDelay == true )
							plusTime = *pPlusTime;
						else
							*pPlusTime = 0;
					}

					mInfGameRestTime = pInfluence->mContinuanceTime + plusTime;
				}

				mDotTotalTime = pInfluence->mContinuanceTime;
			}
		}
		break;
	case eSWITCHTYPE_PARENT:
		{
			/// 스킬 스크립트에서 필요한 수치를 얻어 기록한다.
			sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
			if( pSkillInfo != NULL )	
			{
				cHaveSkillObject* pPlayerHaveSkill = SKILLMANAGER->GetPlayerHaveSkill( mAttacker.index, mSkillIdx );
				if( pPlayerHaveSkill )	
				{ 
					unsigned char skillStep = pPlayerHaveSkill->GetSkillStep();
					if( skillStep < pSkillInfo->mStepCount )
					{
						sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];
						mCopyRange = pStepInfo->mBoundDist1;
						mCopyApplyType = pSkillInfo->mApplyType;
					}
				}
				else
				{
					sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
					mCopyRange = pStepInfo->mBoundDist1;
					mCopyApplyType = pSkillInfo->mApplyType;
				}
			}
		}
		break;
	case eSWITCHTYPE_CHILD:
		{
			mParentIdx = parentIdx;

			/// 스킬 스크립트에서 필요한 수치를 얻어 기록한다.
			sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
			if( pSkillInfo != NULL )	
			{
				mCopyApplyType = pSkillInfo->mApplyType;
			}
		}
		break;
	case eSWITCHTYPE_TOTEM:
		{
			mDotTotalTime = dotTotalTime;
			mParentIdx = parentIdx;
		}
		break;
	}

	bool isCalcDamage = false;
	bool isCalcHeal = false;
	if( pInfluence->mTime1 != 0 )	
	{
		/// 지속 적으로 효과적용(데미지) 인경우 다음 적용 시간 계산
		mTime1 = mInfluenceCreationTime + pInfluence->mTime1;

		/// 공격자의 데미지/힐 계산이 필요한 기본항목을 저장해둘지 체크한다.
		mInfluenceValue1 = pInfluence->mValue1;
		/// 계산식을 사용하는 항목만 mInfluenceValue1값을 다시 설정 해준다.
		switch( pInfluence->mStatusPlusIdx1 )
		{
		case eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS:			/// 지속 HP 힐/데미지 (계산식사용)
			{
				/// 오라스킬 사용 불가
				if( mDotTotalTime == -1 )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType mDotTotalTime1 == -1 %d", infClassIdx );
					return false;
				}

				if( pInfluence->mValue1 > 0 )
				{
					isCalcHeal = true;
					//mCalcTempValue1 = DAMAGECALC->HealDotCalc( attacker, target, pInfluence->mValue1, true, mDotTotalTime / pInfluence->mValue1);
				}
				else if( pInfluence->mValue1 < 0 )
				{
					isCalcDamage = true;
					/*mCalcTempValue1 = DAMAGECALC->DamageDotCalc( attacker, target, pInfluence->mValue1 * -1, skillClassIdx, 
						mAttriButeType, mSkillRangeType, mDotTotalTime / pInfluence->mValue1 );
					mCalcTempValue1 = mCalcTempValue1 * -1;*/
				}
			}
			break;
		case eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS:			/// 지속 MP 힐/데미지 (계산식사용)
			{
				/// 오라스킬 사용 불가
				if( mDotTotalTime == -1 )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType mDotTotalTime2 == -1 %d", infClassIdx );
					return false;
				}

				if( pInfluence->mValue1 > 0 )
				{
					isCalcHeal = true;
					//mCalcTempValue1 = DAMAGECALC->HealDotCalc( attacker, target, pInfluence->mValue1, false, mDotTotalTime / pInfluence->mValue1 );
				}
				else if( pInfluence->mValue1 < 0 )
				{
					isCalcDamage = true;
					//mCalcTempValue1 = DAMAGECALC->DamageDotCalc( attacker, target, pInfluence->mValue1 * -1, skillClassIdx, 
					//	mAttriButeType, mSkillRangeType, mDotTotalTime / pInfluence->mValue1 );
					//mCalcTempValue1 = mCalcTempValue1 * -1;
				}
			}
			break;
		case eSTATUSPLUS_DOT_HPCHANGE_PER:
			mInfluenceValue1 = FloatToInt( (float)pTarget->GetMaxHP() * pInfluence->mValue1 * 0.01f );
			break;
		case eSTATUSPLUS_DOT_MPCHANGE_PER:
			mInfluenceValue1 = FloatToInt( (float)pTarget->GetMaxMP() * pInfluence->mValue1 * 0.01f );
			break;
		}
	}

	/// 지속 적으로 효과적용(데미지) 인경우 다음 적용 시간 계산
	if( pInfluence->mTime2 != 0 )	
	{
		mTime2 = mInfluenceCreationTime + pInfluence->mTime2;

		/// 효과 번호에 힐데미지 계산 공식이 있으면 미리 계산해서 결과만 가지고 있는다
		/// ※ 적용당시 공격자 객체가 사라져 계산이 불가능하게 되는 경우를 막음
		mInfluenceValue2 = pInfluence->mValue2;
		/// 계산식을 사용하는 항목만 mInfluenceValue2값을 다시 설정 해준다.
		switch( pInfluence->mStatusPlusIdx2 )
		{
		case eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS:			/// 지속 HP 힐/데미지 (계산식사용)
			{
				/// 오라스킬 사용 불가
				if( mDotTotalTime == -1 )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType mDotTotalTime3 == -1 %d", infClassIdx );
					return false;
				}

				if( pInfluence->mValue2 > 0 )
				{
					isCalcHeal = true;
					//mCalcTempValue2 = DAMAGECALC->HealDotCalc( attacker, target, pInfluence->mValue2, true, mDotTotalTime / pInfluence->mValue2 );
				}
				else if( pInfluence->mValue2 < 0 )
				{
					isCalcDamage = true;
					//mCalcTempValue2 = DAMAGECALC->DamageDotCalc( attacker, target, pInfluence->mValue2 * -1, skillClassIdx, 
					//	mAttriButeType, mSkillRangeType, mDotTotalTime / pInfluence->mValue2 );
					//mCalcTempValue2 = mCalcTempValue2 * -1;
				}
			}
			break;
		case eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS:			/// 지속 MP 힐/데미지 (계산식사용)
			{
				/// 오라스킬 사용 불가
				if( mDotTotalTime == -1 )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init mGuardType mDotTotalTime4 == -1 %d", infClassIdx );
					return false;
				}

				if( pInfluence->mValue2 > 0 )
				{
					isCalcHeal = true;
					//mCalcTempValue2 = DAMAGECALC->HealDotCalc( attacker, target, pInfluence->mValue2, false, mDotTotalTime / pInfluence->mValue2 );
				}
				else if( pInfluence->mValue2 < 0 )
				{
					isCalcDamage = true;
					//mCalcTempValue2 = DAMAGECALC->DamageDotCalc( attacker, target, pInfluence->mValue2 * -1, skillClassIdx, 
					//	mAttriButeType, mSkillRangeType, mDotTotalTime / pInfluence->mValue2 );
					//mCalcTempValue2 = mCalcTempValue2 * -1;
				}
			}
			break;
		case eSTATUSPLUS_DOT_HPCHANGE_PER:
			mInfluenceValue2 = FloatToInt( (float)pTarget->GetMaxHP() * pInfluence->mValue2 * 0.01f );
			break;
		case eSTATUSPLUS_DOT_MPCHANGE_PER:
			mInfluenceValue2 = FloatToInt( (float)pTarget->GetMaxMP() * pInfluence->mValue2 * 0.01f );
			break;
		}
	}

	/// 공격자의 공격력을 계산해서 저장해둔다.
	if( isCalcDamage == true )
	{
		switch( attacker.type )
		{
		case eOBJECTTYPE_PLAYER:
			{
				/// 스킬 스크립트에서 필요한 수치를 얻어 기록한다.
				sPlayerSkillBaseInfo* pSkillInfo = SKILLSCRIPT->GetPlayerSkillInfo( mSkillIdx );
				if( pSkillInfo != NULL )	
				{
					cHaveSkillObject* pPlayerHaveSkill = SKILLMANAGER->GetPlayerHaveSkill( mAttacker.index, mSkillIdx );
					if( pPlayerHaveSkill )	
					{ 
						unsigned char skillStep = pPlayerHaveSkill->GetSkillStep();
						if( skillStep < pSkillInfo->mStepCount )
						{
							sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[skillStep];

							/// 스킬 어그로 수치
							mDistressPoint = pStepInfo->mDistressValue;
							//mAttriButeType = pSkillInfo->mAttributeType;
							//skillRangeType = pSkillInfo->mRangeType;
						}
					}
					else
					{
						sPlayerSkillStepInfo* pStepInfo = &pSkillInfo->mpSetpInfoArray[0];
						if( pStepInfo == NULL )
						{
							assert(0);
							NETWORK2->PostServerEvent("cInfluenceObject::Init pStepInfo == NULL [%d,%d]", mAttacker.index, mSkillIdx );
							mDistressPoint = 0;
						}
						else
						{
							/// 스킬 어그로 수치
							mDistressPoint = pStepInfo->mDistressValue;
							//mAttriButeType = pSkillInfo->mAttributeType;
							//skillRangeType = pSkillInfo->mRangeType;
						}
					}
				}
				
				cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attacker.index );
				if( pAttacker == NULL )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init eOBJECTTYPE_PLAYER pAttacker == NULL [%d,%d,%d]",
						infClassIdx, skillClassIdx, infType );
					return false;
				}

				mAttackerLv = pAttacker->GetLevel();

				switch( pInfluence->mDotType )
				{
				case eDOTTYPE_PHYSICALCLOSE:
					{
						mAttackerDamageMin = FloatToInt( pAttacker->GetStatus2()->mPhysicMinAttack );
						mAttackerDamageMax = FloatToInt( pAttacker->GetStatus2()->mPhysicMaxAttack );
					}
					break;
				case eDOTTYPE_PHYSICALRANGE:
					{
						mAttackerDamageMin = FloatToInt( pAttacker->GetStatus2()->mPhysicMinRangeAttack );
						mAttackerDamageMax = FloatToInt( pAttacker->GetStatus2()->mPhysicMaxRangeAttack );
					}
					break;
				case eDOTTYPE_MAGIC:
					{
						mAttackerDamageMin = FloatToInt( pAttacker->GetStatus2()->mMagicMinAttack );
						mAttackerDamageMax = FloatToInt( pAttacker->GetStatus2()->mMagicMaxAttack );
					}
					break;
				default:
					NETWORK2->PostServerEvent("cInfluenceObject::Init eOBJECTTYPE_PLAYER pInfluence->mDotType ERROR [%d,%d,%d,%d]",
						infClassIdx, skillClassIdx, infType, pInfluence->mDotType );
					return false;
				}
			}
			break;
		case eOBJECTTYPE_MONSTER:
			{
				cMonster* pAttacker = OBJECTMANAGER->GetMonster( attacker.index );
				if( pAttacker == NULL )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init eOBJECTTYPE_MONSTER pAttacker == NULL [%d,%d,%d]",
						infClassIdx, skillClassIdx, infType );
					return false;
				}

				mAttackerLv = pAttacker->GetLevel();

				sMonsterSkillScript* pSkillInfo = SKILLSCRIPT->GetMonsterSkillInfo( pAttacker->GetRaceGender(), (eMONSTERATTACK_TYPE)skillClassIdx );
				if( pSkillInfo == NULL )
				{
					NETWORK2->PostServerEvent("cInfluenceObject::Init eOBJECTTYPE_MONSTER pSkillInfo == NULL [%d,%d,%d]",
						infClassIdx, skillClassIdx, infType );
					return false;
				}


				switch( pInfluence->mDotType )
				{
				case eDOTTYPE_PHYSICALCLOSE:
					{
						mAttackerDamageMin = pAttacker->CalcPhysicAttack( pSkillInfo->mMinAttackValue );
						mAttackerDamageMax = pAttacker->CalcPhysicAttack( pSkillInfo->mMaxAttackValue );
					}
					break;
				case eDOTTYPE_PHYSICALRANGE:
					{
						mAttackerDamageMin = pAttacker->CalcPhysicRangeAttack( pSkillInfo->mMinAttackValue );
						mAttackerDamageMax = pAttacker->CalcPhysicRangeAttack( pSkillInfo->mMaxAttackValue );
					}
					break;
				case eDOTTYPE_MAGIC:
					{
						mAttackerDamageMin = pAttacker->CalcMagicAttack( pSkillInfo->mMinAttackValue );
						mAttackerDamageMax = pAttacker->CalcMagicAttack( pSkillInfo->mMaxAttackValue );
					}
					break;
				default:
					NETWORK2->PostServerEvent("cInfluenceObject::Init eOBJECTTYPE_MONSTER pInfluence->mDotType ERROR [%d,%d,%d,%d]",
						infClassIdx, skillClassIdx, infType, pInfluence->mDotType );
					return false;
				}
			}
			break;
		default:
			NETWORK2->PostServerEvent("cInfluenceObject::Init isCalcDamage == true attacker.type ERROR [%d,%d,%d,%d]",
				attacker.type, infClassIdx, skillClassIdx, infType );
			return false;
		}
	}

	/// 공격자의 힐수치를 계산해서 저장해둔다.
	if( isCalcHeal == true )
	{
		if( attacker.type == eOBJECTTYPE_PLAYER && target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attacker.index );
			if( pAttacker == NULL )
			{
				NETWORK2->PostServerEvent("cInfluenceObject::Init isCalcHeal == true eOBJECTTYPE_PLAYER pAttacker == NULL [%d,%d,%d]",
					infClassIdx, skillClassIdx, infType );
				return false;
			}

			mAttackerLv = pAttacker->GetLevel();
			mHeal = FloatToInt( pAttacker->GetStatus2()->mHeal );
		}
		else
		{
			NETWORK2->PostServerEvent("cInfluenceObject::Init isCalcHeal == true attacker.type ERROR [%d,%d,%d,%d]",
				attacker.type, infClassIdx, skillClassIdx, infType );
			return false;
		}
	}

	return true;
}


bool cInfluenceObject::RestorePlayerInfluence( unsigned long playerIdx, unsigned long uniqueidx, sInfluence* pInfluence )
{
	mUniqueIdx = uniqueidx;
	mDBIdx = pInfluence->mDbIdx;

	mInfluenceClassIdx = pInfluence->mInfluenceClassIdx;

	/// 오라는 DB에 저장 되어있지 않음
	mSkillIdx = 0;

	sInfluenceScript* pPlayerInfluence = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( !pPlayerInfluence )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent("cInfluenceObject::RestorePlayerInfluence - pPlayer[%d]Influence[%d] == NULL", playerIdx, mInfluenceClassIdx );
		return false; 
	}

	mTarget.index = playerIdx;
	mTarget.type = eOBJECTTYPE_PLAYER;

	mAttacker = pInfluence->mAttacker;

	/// DB에서 읽어온 효과는 클라이언트 gamein 메세지가 오는 시점이 효과 생성 시점이다.
	mInfluenceCreationTime = NETWORK2->GetAccumTime();

	mIsRealTime = pInfluence->mIsRealTime;
	mInfRealEndTime = pInfluence->mEndDate;

	mTime1 = 0; 
	mTime2 = 0; 

	if( pPlayerInfluence->mContinuanceTime == -1 )
	{
		mInfGameRestTime = 0;
		mSwitchType = eSWITCHTYPE_PARENT;
	}
	else
	{
		mInfGameRestTime = pInfluence->mRestTime;// * SECOND_THOUSAND;
	}

	/// 지속 적으로 효과적용(데미지) 인경우 다음 적용 시간 계산
	if( pInfluence->mTime1 != 0 )	
	{
		mTime1 = mInfluenceCreationTime + pInfluence->mTime1;
		mInfluenceValue1 = pInfluence->mInfluenceValue1;
	}

	/// 지속 적으로 효과적용(데미지) 인경우 다음 적용 시간 계산
	if( pInfluence->mTime2 != 0 )	
	{
		mTime2 = mInfluenceCreationTime + pInfluence->mTime2;
		mInfluenceValue2 = pInfluence->mInfluenceValue2;
	}

	mDotTotalTime = pPlayerInfluence->mContinuanceTime;

	mParentIdx = 0;
	mCopyRange = 0;
	mCopyCheckTime = 0;
	mCopyApplyType = eAPPLYTYPE_SELF;
	mSkillIdx = 0;

	mIsSleep = false;
	if( pPlayerInfluence->mStatusPlusIdx1 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;
	if( pPlayerInfluence->mStatusPlusIdx2 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;
	if( pPlayerInfluence->mStatusPlusIdx3 == eSTATUSPLUS_SLEEP )
		mIsSleep = true;

	mAttackerLv = pInfluence->mAttackerLv;
	mAttackerDamageMin = pInfluence->mAttackerDamageMin;
	mAttackerDamageMax = pInfluence->mAttackerDamageMax;
	mHeal = pInfluence->mHeal;

	mLeftGuardCnt = pInfluence->mLeftGuardCnt;
	mLeftGuardValue = pInfluence->mLeftGuardValue;

	/// 스크립트에서 중복으로 설정했는지 체크
	mGuardType = eINFGUARDTYPE_NONE;
	mChangeMPType = eINFCHANGEMPTYPE_NONE;
	unsigned short statusIdx[3] = { pPlayerInfluence->mStatusPlusIdx1, pPlayerInfluence->mStatusPlusIdx2, pPlayerInfluence->mStatusPlusIdx3 };

	for( int i = 0 ; i < 3 ; ++i )
	{
		switch( statusIdx[i] )
		{
		case eSTATUSPLUS_GUARDCNT_PHY:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_PHY )
					return false;
				mGuardType = eINFGUARDTYPE_PHY;
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_PHY:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_PHY )
					return false;
				mGuardType = eINFGUARDTYPE_PHY;
			}
			break;
		case eSTATUSPLUS_GUARDCNT_MAG:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_MAG )
					return false;
				mGuardType = eINFGUARDTYPE_MAG;
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_MAG:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_MAG )
					return false;
				mGuardType = eINFGUARDTYPE_MAG;
			}
			break;
		case eSTATUSPLUS_GUARDCNT_ALL:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_ALL )
					return false;
				mGuardType = eINFGUARDTYPE_ALL;
			}
			break;
		case eSTATUSPLUS_GUARDVALUE_ALL:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_ALL )
					return false;
				mGuardType = eINFGUARDTYPE_ALL;
			}
			break;
		case eSTATUSPLUS_CASHEXPGUARDCNT:
			{
				if( mGuardType != eINFGUARDTYPE_NONE && mGuardType != eINFGUARDTYPE_ALL )
					return false;
				mGuardType = eINFGUARDTYPE_CASHEXP;
			}
			break;
		case eSTATUSPLUS_GUARDMP_PHY:
			{
				mChangeMPType = eINFCHANGEMPTYPE_PHY;
			}
			break;
		case eSTATUSPLUS_GUARDMP_MAG:
			{
				mChangeMPType = eINFCHANGEMPTYPE_MAG;
			}
			break;
		case eSTATUSPLUS_GUARDMP_ALL:
			{
				mChangeMPType = eINFCHANGEMPTYPE_ALL;
			}
			break;
		}
	}

	switch( mGuardType )
	{
	case eINFGUARDTYPE_CASHEXP:
		if( mLeftGuardCnt == 0 )
			return false;
		break;
	case eINFGUARDTYPE_NONE:
		break;
	default:
		if( mLeftGuardCnt == 0 || mLeftGuardValue == 0 )
			return false;
		break;			
	}

	/// db에서 읽어들인 자료는 gamein 메세지가 오기전까지 프로세스를 하지 않는다.
	ProcessStop();

	return true;
}


void cInfluenceObject::ProcessStart()
{ 
	mInfluenceCreationTime = NETWORK2->GetAccumTime();
	mProcessStart = true; 

	sInfluenceScript* pInfluence = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( pInfluence != NULL )
	{
		if( pInfluence->mTime1 != 0 )
			mTime1 = mInfluenceCreationTime + pInfluence->mTime1;

		if( pInfluence->mTime2 != 0 )
			mTime2 = mInfluenceCreationTime + pInfluence->mTime2;
	}
}


void cInfluenceObject::ProcessStop()
{ 
	if( mSwitchType == eSWITCHTYPE_NORMAL )
	{
		unsigned long accum = NETWORK2->GetAccumTime();

		if( mIsRealTime == false )
		{
			if( accum - mInfluenceCreationTime >= mInfGameRestTime )
				mInfGameRestTime = 0;
			else
				mInfGameRestTime = mInfGameRestTime - (accum - mInfluenceCreationTime);
		}
	}

	mProcessStart = false; 
}


bool cInfluenceObject::SetAllocDbIdx( unsigned long dbIdx )
{
	if( mDBIdx != 0 )
		return false;

	mDBIdx = dbIdx;
	return true;
}

unsigned char cInfluenceObject::GetDeleteType()
{ 
	sInfluenceScript* pInf = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( pInf == NULL )	
	{ 
		NETWORK2->PostServerEvent( "cInfluenceObject::GetDieDelete pInf[%d] == NULL", mInfluenceClassIdx );
		return true; 
	}

	return pInf->mDeleteType; 
}


bool cInfluenceObject::IsSavePlayerInfluence( bool onlyCash )
{
	/// pc방 버프 저장안함
	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( pScript == NULL )
		return false;

	if( pScript->mType == eINFLUENCETYPE_PC )
		return false;

	if( onlyCash == true )
	{
		if( pScript->mType == eINFLUENCETYPE_CASH && mDBIdx != 0 )
			return true;
		else
			return false;
	}

	/// 수장효과
	if( PVP_DM_LEADER_INFLUENCE == mInfluenceClassIdx )
		return false;		
	if( PVP_DM_LEADER_INFLUENCE2 == mInfluenceClassIdx )
		return false;		
	if( PVP_DM2_FLAG_INFLUENCE == mInfluenceClassIdx )
		return false;
	if( PVP_DM_NODAMAGE_INFLUENCE == mInfluenceClassIdx )
		return false;
	if( PVP_DM_WAIT_NODAMAGE_INFLUENCE == mInfluenceClassIdx )
		return false;

	/// 오라효과, 오라에 복사된 효과 - 캐시버프는 저장
	if( mSwitchType != eSWITCHTYPE_NORMAL && pScript->mType != eINFLUENCETYPE_CASH )
		return false;

	if( pScript->mType == eINFLUENCETYPE_CASH )
		return true;

	if( mDBIdx != 0 )
		return true;

	/// 삭제 대기상태 효과
	if( mIsDelReady == true )
		return false;

	return true;
}

void cInfluenceObject::SavePlayerInfluence( sInfluence* pInfluence )
{
	sInfluenceScript* pPlayerInfluence = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( !pPlayerInfluence )	
	{ 
		assert(NULL); 
		NETWORK2->PostServerEvent( "db Influence save - GetInfluenceInfo( %d )", mInfluenceClassIdx );
		return; 
	}

	unsigned long currenttime = NETWORK2->GetAccumTime();

	pInfluence->mDbIdx = mDBIdx;
	pInfluence->mUniqueIdx = mUniqueIdx;
	pInfluence->mInfluenceClassIdx = mInfluenceClassIdx;
	pInfluence->mAttacker = mAttacker;
	pInfluence->mIsRealTime = mIsRealTime;

	if( mProcessStart == false )
	{
		pInfluence->mRestTime = mInfGameRestTime;
	}
	else
	{

		if( pPlayerInfluence->mType == eINFLUENCETYPE_CASH && mSwitchType != eSWITCHTYPE_NORMAL )
		{
			pInfluence->mRestTime = 0;

			if( pPlayerInfluence->mContinuanceTime > 0 )
			{
				if( currenttime - mInfluenceCreationTime >= mInfGameRestTime )
				{
					pInfluence->mRestTime = 0;

					if( currenttime >= mInfluenceCreationTime + mInfGameRestTime )
					{
					}
					else
					{
						pInfluence->mRestTime = mInfluenceCreationTime + mInfGameRestTime - currenttime;
					}
				}
				else
					pInfluence->mRestTime =  mInfGameRestTime - (currenttime-mInfluenceCreationTime);
			}
		}
		else if( currenttime - mInfluenceCreationTime >= mInfGameRestTime )
		{
			pInfluence->mRestTime = 0;

			if( currenttime >= mInfluenceCreationTime + mInfGameRestTime )
			{
			}
			else
			{
				pInfluence->mRestTime = mInfluenceCreationTime + mInfGameRestTime - currenttime;
			}
		}
		else
			pInfluence->mRestTime =  mInfGameRestTime - (currenttime-mInfluenceCreationTime);
	}

	pInfluence->mEndDate = mInfRealEndTime;

	if( pPlayerInfluence->mTime1 != 0.0f )
		pInfluence->mTime1 = mTime1 - currenttime; 
	else					
		pInfluence->mTime1 = 0; 

	if( pPlayerInfluence->mTime2 != 0.0f )
		pInfluence->mTime2 = mTime2 - currenttime; 
	else					
		pInfluence->mTime2 = 0; 

	pInfluence->mInfluenceValue1 = mInfluenceValue1;
	pInfluence->mInfluenceValue2 = mInfluenceValue2;

	pInfluence->mAttriButeType = (unsigned char)mAttriButeType;
	pInfluence->mAttackerLv = mAttackerLv;
	pInfluence->mAttackerDamageMin = mAttackerDamageMin;
	pInfluence->mAttackerDamageMax = mAttackerDamageMax;
	pInfluence->mHeal = mHeal;
	pInfluence->mLeftGuardCnt = mLeftGuardCnt;
	pInfluence->mLeftGuardValue = mLeftGuardValue;
}



bool cInfluenceObject::Process( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( mIsDelReady	== true )
		return true;

	if( !mProcessStart ) 
		return true; 

	sInfluenceScript* pInfluence = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( pInfluence == NULL )
		return false;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
	if( pTarget == NULL || ( pTarget->GetStateDie() == true && pInfluence->mDeleteType != eDELETETYPE_NODELETE_PLAYER_DIE ) )
		return false;

	unsigned long scriptTime1 = pInfluence->mTime1;
	unsigned long scriptTime2 = pInfluence->mTime2;

	/// 자신이 건오라가 자신에게는 적용 안되는 경우 체크
	if( ( mSwitchType == eSWITCHTYPE_PARENT && mCopyApplyType == eAPPLYTYPE_ENEMY ) == false )
	{
		if( mTime1 != 0 && mTime1 <= accumTime )
		{
			mTime1 = mTime1 + scriptTime1;

			/// 지속 효과 처리
			DotInfluence( (eSTATUS_PLUS)pInfluence->mStatusPlusIdx1, mInfluenceValue1, mDotTotalTime, scriptTime1 );
		}

		if( mTime2 != 0 && mTime2 <= accumTime )
		{
			mTime2 = mTime2 + scriptTime2;

			/// 지속 효과 처리
			DotInfluence( (eSTATUS_PLUS)pInfluence->mStatusPlusIdx2, mInfluenceValue2, mDotTotalTime, scriptTime2 );
		}
	}

	/// 효과 생존시간 끝난 경우
	if( mIsRealTime == false && mSwitchType == eSWITCHTYPE_NORMAL && mInfluenceCreationTime + mInfGameRestTime <= accumTime  )
		return false;
	if( mIsRealTime == true && mSwitchType == eSWITCHTYPE_NORMAL && TIMER->DiffRealTime( mInfRealEndTime ) <= 0 )
		return false;

	/// 효과가 오라 인경우 범위안에 들어온 타겟에게 효과 복사
	if( mSwitchType == eSWITCHTYPE_PARENT )
	{
		if( mCopyCheckTime <= accumTime )
		{
			CopyAuraInfluence();
			mCopyCheckTime = accumTime + AURA_CHECK_TIME;
		}

		// 일정 시간마다 체크
		if( mDotValueAcuumTime <= accumTime )
		{
			switch( pInfluence->mDotType )
			{
			case eDOTTYPE_MPPLUS:			/// 지속 MP소모(+)
				{
					pTarget->MPUse( pInfluence->mDotValue, true );
					if( pTarget->GetMP() <= 0 )
						return false;
				}
				break;
			case eDOTTYPE_MPPER:			/// 지속 MP소모(%)
				{
					long dotValue = (long)(pTarget->GetOriMPMax() * pInfluence->mDotValue / 100);

					pTarget->MPUse( dotValue, true );
					if( pTarget->GetMP() <= 0 )
						return false;
				}
				break;
			case eDOTTYPE_HPPLUS:			/// 지속 HP소모(+)
				{
					pTarget->HPUse( pInfluence->mDotValue, true );
					if( pTarget->GetHP() <= 0 )
						return false;
				}
				break;
			case eDOTTYPE_HPPER:			/// 지속 HP소모(%)
				{
					long dotValue = (long)(pTarget->GetOriHPMax() * pInfluence->mDotValue / 100);

					pTarget->HPUse( dotValue, true );
					if( pTarget->GetHP() <= 0 )
						return false;
				}
				break;
			}
			
			mDotValueAcuumTime = accumTime + AURAMP_CHECK_TIME;
		}
	}
	/// 자식 효과 인경우 효과 제거 체크
	else if ( mSwitchType == eSWITCHTYPE_TOTEM )
	{
		/// 부모효과유무와 거리 체크
		if( mCopyCheckTime <= accumTime )
		{
			cBaseObject* pTarget = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
			if( pTarget == NULL )
			{
				CheckTotemInfluence();
				return false;
			}

			if( CheckTotemInfluence() == true )
				return false;

			mCopyCheckTime = accumTime + AURA_CHECK_TIME;
		}
	}
	else if ( mSwitchType == eSWITCHTYPE_CHILD )
	{
		/// 부모효과유무와 거리 체크
		if( mCopyCheckTime <= accumTime )
		{
			cBaseObject* pTarget = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
			if( pTarget == NULL || pTarget->GetStateDie() == true )
			{
				/// 효과 제거
				DeleteChildAuraInfluence();
				return false;
			}

			if( DeleteChildAuraInfluence() == true )
				return false;

			mCopyCheckTime = accumTime + AURA_CHECK_TIME;
		}
	}

	return true;
}


void cInfluenceObject::AddModeType( unsigned char applyType, unsigned short range )
{ 
	mCopyApplyType = (eAPPLYTYPE)applyType; 
	mCopyRange = range;
}


void cInfluenceObject::DotInfluence( eSTATUS_PLUS statusPlusIdx, long infValue, unsigned long totalTime, unsigned long dotTime )
{
	long influenceValue = infValue;

	cBaseObject* pTargetObject = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
	if( pTargetObject == NULL || pTargetObject->GetStateDie() == true )
		return;

	sInfluenceScript* pInf = SKILLSCRIPT->GetInfluenceInfo( mInfluenceClassIdx );
	if( pInf == NULL )	
	{ 
		NETWORK2->PostServerEvent("cInfluenceObject::DotInfluence pInf[%d] == NULL", mInfluenceClassIdx );
		return; 
	}

	bool isDamage = true;		/// 데미지(true), 힐(false) 구분
	if( influenceValue > 0 )
		isDamage = false;

	bool isHP = true;			/// hp적용(true), mp적용(false) 구분
	switch( statusPlusIdx )
	{
	case eSTATUSPLUS_DOT_HPCHANGE_CALC_PLUS:	/// 지속 HP 힐/데미지(계산식사용)
		{
			isHP = true;
			
			influenceValue = 0;
			if( infValue > 0 )
			{
				influenceValue = pTargetObject->DotHealCalc( mTarget, infValue, 0, mHeal, totalTime, dotTime );
			}
			else if( infValue < 0 )
			{
				influenceValue = pTargetObject->DotDamageCalc( mAttacker.type, mAttackerLv, mAttackerDamageMin, mAttackerDamageMax,
					pInf->mDotType, -infValue, 0, totalTime, dotTime );
				influenceValue = influenceValue * -1;
			}
			//if( influenceValue == 0 )
			//	influenceValue = mCalcTempValue1;
		}
		break;
	case eSTATUSPLUS_DOT_HPCHANGE_CALC_PER:
		{
			isHP = true;

			influenceValue = 0;
			if( infValue > 0 )
			{
				influenceValue = pTargetObject->DotHealCalc( mTarget, 0, infValue, mHeal, totalTime, dotTime );
			}
			else if( infValue < 0 )
			{
				influenceValue = pTargetObject->DotDamageCalc( mAttacker.type, mAttackerLv, mAttackerDamageMin, mAttackerDamageMax,
					pInf->mDotType, 0, -infValue, totalTime, dotTime );
				influenceValue = influenceValue * -1;
			}
			//if( influenceValue == 0 )
			//	influenceValue = mCalcTempValue1;
		}
		break;
	case eSTATUSPLUS_DOT_HPCHANGE_PLUS:			///	(효과) 지속 HP +
	case eSTATUSPLUS_DOT_HPCHANGE_PER:			/// 지속 HP %
		isHP = true;
		break;
	case eSTATUSPLUS_DOT_MPCHANGE_CALC_PLUS:	/// 지속 MP 힐(계산식사용)
		{
			isHP = false;
			
			influenceValue = 0;
			if( infValue > 0 )
			{
				influenceValue = pTargetObject->DotHealCalc( mTarget, infValue, 0, mHeal, totalTime, dotTime );
			}
			else if( infValue < 0 )
			{
				influenceValue = pTargetObject->DotDamageCalc( mAttacker.type, mAttackerLv, mAttackerDamageMin, mAttackerDamageMax,
					pInf->mDotType, -infValue, 0, totalTime, dotTime );
				influenceValue = influenceValue * -1;
			}
			//if( influenceValue == 0 )
			//	influenceValue = mCalcTempValue1;
		}
		break;
	case eSTATUSPLUS_DOT_MPCHANGE_CALC_PER:
		{
			isHP = false;

			influenceValue = 0;
			if( infValue > 0 )
			{
				influenceValue = pTargetObject->DotHealCalc( mTarget, 0, infValue, mHeal, totalTime, dotTime );
			}
			else if( infValue < 0 )
			{
				influenceValue = pTargetObject->DotDamageCalc( mAttacker.type, mAttackerLv, mAttackerDamageMin, mAttackerDamageMax,
					pInf->mDotType, 0, -infValue, totalTime, dotTime );
				influenceValue = influenceValue * -1;
			}
			//if( influenceValue == 0 )
			//	influenceValue = mCalcTempValue1;
		}
		break;
	case eSTATUSPLUS_DOT_MPCHANGE_PLUS:			/// 지속 MP +
	case eSTATUSPLUS_DOT_MPCHANGE_PER:			/// 지속 MP %
		isHP = false;
		break;
	}

	bool die = false;
	if( isDamage == false )	/// 힐처리
	{
		if( isHP == true )		/// hp 힐처리
		{
			pTargetObject->HPHeal( influenceValue, false, mAttacker, mDistressPoint, eTAKEDAMAGETYPE_HEAL );

			MSG_SYN_SKILL_INFLUENCE_HPCHANGE msg; 
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_HPCHAGNE_SYN;
			msg.mInfluenceIdx = mUniqueIdx;
			msg.mTarget = mTarget;
			msg.mChangeValue = (short)influenceValue;
			
			MSG_SYN_SKILL_INFLUENCE_HPCHANGE aroundMsg; 
			aroundMsg.Category = NM_SKILL;
			aroundMsg.Protocol = NM_SKILL_INFLUENCE_HPCHAGNE_AROUND_SYN;
			aroundMsg.mInfluenceIdx = mUniqueIdx;
			aroundMsg.mTarget = mTarget;
			aroundMsg.mChangeValue = (short)influenceValue;

			SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );
		}
		else
		{
			pTargetObject->MPHeal( influenceValue, false );

			MSG_SYN_SKILL_INFLUENCE_MPCHANGE msg; 
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_MPCHAGNE_SYN;
			msg.mInfluenceIdx = mUniqueIdx;
			msg.mTarget = mTarget;
			msg.mChangeValue = (short)influenceValue;

			MSG_SYN_SKILL_INFLUENCE_MPCHANGE aroundMsg; 
			aroundMsg.Category = NM_SKILL;
			aroundMsg.Protocol = NM_SKILL_INFLUENCE_MPCHAGNE_AROUND_SYN;
			aroundMsg.mInfluenceIdx = mUniqueIdx;
			aroundMsg.mTarget = mTarget;
			aroundMsg.mChangeValue = (short)influenceValue;

			SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );
		}

		SKILLMANAGER->InsertChangeSet( pTargetObject, mAttacker, eDAMAGE_NORMAL );
	}
	else	/// 데미지 처리
	{
		unsigned long baseDamage = abs(influenceValue);			/// 데미지 수치 양수화

		/// 데미지 흡수 - 데미지의 값을 감소시킨다.
		long absorbPhyPer = pTargetObject->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_PHY );
		long absorbMagPer = pTargetObject->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_MAG );
		long absorbPhyRangePer = pTargetObject->GetStateOddity( eODDITYTYPE_DAMAGE_ABSORB_PHY_RANGE );

		if( absorbPhyPer != 0 && ( pInf->mDotType == eDOTTYPE_PHYSICALCLOSE || pInf->mDotType == eDOTTYPE_PHYSICALRANGE ) )
		{
			if( absorbPhyRangePer != 0 && pInf->mDotType == eDOTTYPE_PHYSICALRANGE )
				absorbPhyRangePer += absorbPhyPer;

			if( absorbPhyPer < 0 )		absorbPhyPer = 0;
			if( absorbPhyPer > 100 )	absorbPhyPer = 100;
			baseDamage = FloatToInt( baseDamage * ( 100 - absorbPhyPer ) * 0.01f );
		}
		else if( absorbMagPer != 0 && pInf->mDotType == eDOTTYPE_MAGIC )
		{
			if( absorbMagPer < 0 )		absorbMagPer = 0;
			if( absorbMagPer > 100 )	absorbMagPer = 100;
			baseDamage = FloatToInt( baseDamage * ( 100 - absorbMagPer ) * 0.01f );
		}
		if( baseDamage == 0 )
			baseDamage = 1;

		/// 무적 처리 - 데미지를 없에 버린다.
		if( ( pTargetObject->GetStateOddity( eODDITYTYPE_INVINCIBILITY_PHY ) != 0 && ( pInf->mDotType == eDOTTYPE_PHYSICALCLOSE || pInf->mDotType == eDOTTYPE_PHYSICALRANGE ) ) )
			baseDamage = 0;
		else if( pTargetObject->GetStateOddity( eODDITYTYPE_INVINCIBILITY_MAG ) != 0 && pInf->mDotType == eDOTTYPE_MAGIC )
			baseDamage = 0;

		/// 데미지 반사 - 데미지량을 줄이고 공격자에게 돌려준다.
		long reflectionPhyPer = pTargetObject->GetStateOddity( eODDITYTYPE_DAMAGE_REFLECTION_PHY );
		long reflectionMagPer = pTargetObject->GetStateOddity( eODDITYTYPE_DAMAGE_REFLECTION_MAG );
		long reflectionDamage = 0;
		if( reflectionPhyPer != 0 && ( pInf->mDotType == eDOTTYPE_PHYSICALCLOSE || pInf->mDotType == eDOTTYPE_PHYSICALRANGE ) )
		{
			if( reflectionPhyPer < 0 )		reflectionPhyPer = 0;
			if( reflectionPhyPer > 100 )	reflectionPhyPer = 100;
			reflectionDamage = FloatToInt( baseDamage * reflectionPhyPer * 0.01f );
			baseDamage = baseDamage - reflectionDamage;
		}
		else if( reflectionMagPer != 0 && pInf->mDotType == eDOTTYPE_MAGIC )
		{
			if( reflectionMagPer < 0 )		reflectionMagPer = 0;
			if( reflectionMagPer > 100 )	reflectionMagPer = 100;
			reflectionDamage = FloatToInt( baseDamage * reflectionMagPer * 0.01f );
			baseDamage = baseDamage - reflectionDamage;
		}

		/// 실제로 적용된 데미지
		unsigned long applyDamage = 0;
		/// 타겟 결과 처리
		if( isHP == true )		
		{
			/// hp 데미지 처리부
			if( DUELMANAGER->DuelLastAttack( mAttacker, mTarget, baseDamage, false, &applyDamage ) == false )
			{
				applyDamage = pTargetObject->HPDamage( mAttacker, baseDamage, &die, false );
			}				
			/// 받은 데미지 처리
			pTargetObject->AddTakeDamage( mAttacker, applyDamage, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );		

			/// 타겟이 받은 데미지 발송
			MSG_SYN_SKILL_INFLUENCE_HPCHANGE msg; 
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_HPCHAGNE_SYN;
			msg.mInfluenceIdx = mUniqueIdx;
			msg.mTarget = mTarget;
			msg.mChangeValue = (short)(applyDamage * -1);

			/// 타겟이 받은 데미지 발송
			MSG_SYN_SKILL_INFLUENCE_HPCHANGE aroundMsg; 
			aroundMsg.Category = NM_SKILL;
			aroundMsg.Protocol = NM_SKILL_INFLUENCE_HPCHAGNE_AROUND_SYN;
			aroundMsg.mInfluenceIdx = mUniqueIdx;
			aroundMsg.mTarget = mTarget;
			aroundMsg.mChangeValue = (short)(applyDamage * -1);

			SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );
		}
		else
		{
			applyDamage = baseDamage;
			/// mp 데미지 처리부
			pTargetObject->MPDamage( baseDamage, false );
			/// 받은 데미지 처리
			pTargetObject->AddTakeDamage( mAttacker, applyDamage, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );		

			/// 타겟이 받은 데미지 발송
			MSG_SYN_SKILL_INFLUENCE_MPCHANGE msg; 
			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_MPCHAGNE_SYN;
			msg.mInfluenceIdx = mUniqueIdx;
			msg.mTarget = mTarget;
			msg.mChangeValue = (short)(applyDamage * -1);

			MSG_SYN_SKILL_INFLUENCE_MPCHANGE aroundMsg; 
			aroundMsg.Category = NM_SKILL;
			aroundMsg.Protocol = NM_SKILL_INFLUENCE_MPCHAGNE_AROUND_SYN;
			aroundMsg.mInfluenceIdx = mUniqueIdx;
			aroundMsg.mTarget = mTarget;
			aroundMsg.mChangeValue = (short)(applyDamage * -1);

			SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );
		}

		SKILLMANAGER->InsertChangeSet( pTargetObject, mAttacker, eDAMAGE_NORMAL );

		/// 공격자가 데미지 흡수 && 반사된 데미지 처리
		cBaseObject* pAttacker = OBJECTMANAGER->GetObject( mAttacker.type, mAttacker.index );
		if( pAttacker != NULL && pAttacker->GetStateDie() == false )
		{
			//////////////////////////////////////////////////////////////////////////
			/// 공격자가 hp흡수
			long attackerBaseHeal = 0;
			if( isHP == true )		/// 데미지 반사된 량이 hp이면 흡수값에 합산
				attackerBaseHeal -= reflectionDamage;
			/// 공격자 HP흡수 값이 있는 경우 계산
			unsigned long attkAbsorbHPPer = pAttacker->GetStateOddity( eODDITYTYPE_HP_ABSORB );	
			if( attkAbsorbHPPer != 0 )
			{
				/// 공격자 데미지 hp흡수
				if( attkAbsorbHPPer < 0 )	attkAbsorbHPPer = 0;
				if( attkAbsorbHPPer > 100 )	attkAbsorbHPPer = 100;
				unsigned long attkAbsorbHP = FloatToInt( applyDamage * attkAbsorbHPPer * 0.01f );
				/// 흡수량과 반사되 돌아온 데미지 합산
				attackerBaseHeal = attkAbsorbHP + attackerBaseHeal;
			}

			/// 실제로 적용된 데미지
			long attackerApplyDamage = 0;
			/// 타겟 결과 처리
			if( attackerBaseHeal > 0 )	/// 합산된 공격자 데미지가 양수면 흡수량이 더많음 - heal
			{
				/// 힐 처리
				attackerApplyDamage = attackerBaseHeal;
				pAttacker->HPHeal( attackerApplyDamage, false, pAttacker->GetObject(), 0, eTAKEDAMAGETYPE_NONE );
			}
			else if( attackerBaseHeal < 0 )		/// 합산된 공격자 데미지가 음수면 흡수량보다 데미지량이 더 많음 - 데미지
			{
				unsigned long tempDamage = 0;
				/// hp 데미지 처리부
				if( DUELMANAGER->DuelLastAttack( mTarget, mAttacker, attackerBaseHeal * -1, eDAMAGE_NORMAL, &tempDamage ) == false )
					tempDamage = pAttacker->HPDamage( mTarget, attackerBaseHeal* -1, &die, false, eDAMAGE_NORMAL );
				/// 받은 데미지 처리
				pAttacker->AddTakeDamage( mTarget, tempDamage, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );

				/// 데미지는 음수값으로 변환
				attackerApplyDamage = tempDamage * -1;
			}

			/// 흡수&데미지 반사 내용이 있는 경우 발송
			if( attackerApplyDamage != 0 )
			{
				/// 공격자 hp 데미지/힐 적용 메세지 발송
				MSG_SYN_SKILL_NONSKILL_HPCHANGE msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_SYN;
				msg.mInfluenceClassIdx = 0;
				msg.mTarget = mAttacker;
				msg.mChangeValue = (short)attackerApplyDamage;

				MSG_SYN_SKILL_NONSKILL_HPCHANGE aroundMsg;
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_HPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = 0;
				aroundMsg.mTarget = mAttacker;
				aroundMsg.mChangeValue = (short)attackerApplyDamage;

				SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				SKILLMANAGER->InsertChangeSet( pAttacker, mTarget, eDAMAGE_NORMAL );
			}

			//////////////////////////////////////////////////////////////////////////
			/// 공격자가 mp흡수
			attackerBaseHeal = 0;
			if( isHP == false )		/// 데미지 반사된 량이 mp이면 흡수값에 합산
				attackerBaseHeal -= reflectionDamage;
			/// 공격자 데미지 mp흡수
			unsigned long attkAbsorbMPPer = pAttacker->GetStateOddity( eODDITYTYPE_MP_ABSORB ); /// 공격자 MP흡수 비율
			if( attkAbsorbMPPer != 0 )
			{
				if( attkAbsorbMPPer < 0 )	attkAbsorbMPPer = 0;
				if( attkAbsorbMPPer > 100 )	attkAbsorbMPPer = 100;
				unsigned long attkAbsorbMP = FloatToInt( applyDamage * attkAbsorbMPPer * 0.01f );
				/// 흡수량과 반사되 돌아온 데미지 합산
				attackerBaseHeal = attkAbsorbMP + attackerBaseHeal;
			}

			/// 실제로 적용된 데미지
			attackerApplyDamage = 0;
			/// 타겟 결과 처리
			if( attackerBaseHeal > 0 )	/// 합산된 공격자 데미지가 양수면 흡수량이 더많음 - heal
			{
				/// 힐 처리
				attackerApplyDamage = attackerBaseHeal;
				pAttacker->MPHeal( attackerApplyDamage, false );
			}
			else if( attackerBaseHeal < 0 )		/// 합산된 공격자 데미지가 음수면 흡수량보다 데미지량이 더 많음 - 데미지
			{
				attackerApplyDamage = attackerBaseHeal;
				pAttacker->MPDamage( attackerApplyDamage * -1, false );
				pAttacker->AddTakeDamage( mTarget, attackerApplyDamage * -1, mDistressPoint, eTAKEDAMAGETYPE_DAMAGE );

				/// 데미지는 음수값으로 변환
				attackerApplyDamage = attackerApplyDamage * -1;
			}

			/// 흡수&데미지 반사 내용이 있는 경우 발송
			if( attackerApplyDamage != 0 )
			{

				MSG_SYN_SKILL_NONSKILL_MPCHANGE msg;
				msg.Category = NM_SKILL;
				msg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_SYN;
				msg.mInfluenceClassIdx = 0;
				msg.mTarget = mAttacker;
				msg.mChangeValue = (short)attackerApplyDamage;

				MSG_SYN_SKILL_NONSKILL_MPCHANGE aroundMsg;
				aroundMsg.Category = NM_SKILL;
				aroundMsg.Protocol = NM_SKILL_NONSKILL_MPCHANGE_AROUND_SYN;
				aroundMsg.mInfluenceClassIdx = 0;
				aroundMsg.mTarget = mAttacker;
				aroundMsg.mChangeValue = (short)attackerApplyDamage;

				SKILLMANAGER->SendHPMPInf( pTargetObject, mAttacker, (char*)&msg, sizeof(msg), (char*)&aroundMsg, sizeof(aroundMsg) );

				SKILLMANAGER->InsertChangeSet( pAttacker, mTarget, eDAMAGE_NORMAL );
			}
		}
	}
}


/// 오라인 효과에서 범위 안에 해당되는 object에게 오라를 복사해줌
void cInfluenceObject::CopyAuraInfluence()
{
	/// 오라의 효과가 범위거리가 없으면 안된다.
	if( mCopyRange == 0.0f )
		return;

	if( mTarget.type == eOBJECTTYPE_PLAYER )
	{
		/// 오라 효과의 부모객체
		cPlayer* pParentObejct = OBJECTMANAGER->GetPlayer( mTarget.index );
		if( pParentObejct == NULL )
			return;

		/// 부모객체의 위치와 타겟의 위치
		NiPoint3 parentPos( pParentObejct->GetXPos(), pParentObejct->GetYPos(), pParentObejct->Height() );
		mRangeCheck.SetRadius( mCopyRange );

		switch( mCopyApplyType )
		{
		case eAPPLYTYPE_PARTY:
			{
				/// 파티 정보를 읽어옴
				cParty* pParty = PARTYMAN->GetParty( ((cPlayer*)pParentObejct)->GetPartyIndex() );
				if( pParty != NULL )
				{
					unsigned long* userArr = pParty->GetUserArr();
					unsigned int partyCount = pParty->GetCount();

					for( unsigned int i = 0 ; i < partyCount ; ++i )
					{
						/// 자신은 통과
						if( userArr[i] == pParentObejct->GetObjectID() )
							continue;

						/// 파티원을 읽어옴
						cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
						if( pChild == NULL )
							continue;

						if( pChild->GetStateDie() == true )
							continue;

						if( pChild->GetGameIn() == false )
							continue;

						/// 거리안에 들어오는 경우
						NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
						if( mRangeCheck.IsRange( childPos, parentPos ) )
						{
							/// 같은 오라 효과가 복사되는것 방지
							if( pChild->IsParentEqual( mUniqueIdx ) == true )
								continue;	

							SKILLMANAGER->AddInfChildAura( mTarget, pChild->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
						}
					}
				}
				else
				{
					/// 파티연합으로부터 정보 읽음
					cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( ((cPlayer*)pParentObejct)->GetPartyUnionIndex() );
					if( partyUnion != NULL )
					{
						unsigned long* userArr = partyUnion->GetGroupArr( pParentObejct->GetObjectID() );

						for( unsigned int i = 0; i < MAX_PARTYUSER; ++i )
						{
							if( userArr[i] == pParentObejct->GetObjectID() )
								continue;

							/// 파티원을 읽어옴
							cPlayer* pChild = OBJECTMANAGER->GetPlayer( userArr[i] );
							if( pChild == NULL )
								continue;

							if( pChild->GetStateDie() == true )
								continue;

							if( pChild->GetGameIn() == false )
								continue;

							/// 거리안에 들어오는 경우
							NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
							if( mRangeCheck.IsRange( childPos, parentPos ) )
							{
								/// 같은 오라 효과가 복사되는것 방지
								if( pChild->IsParentEqual( mUniqueIdx ) == true )
									continue;	

								SKILLMANAGER->AddInfChildAura( mTarget, pChild->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
							}
						}
					}
				}
			}
			break;
		case eAPPLYTYPE_BUDDY:
			{
				cPlayer* pChild = GRIDMANAGER->FindFirstPlayer( pParentObejct );
				while( pChild != NULL )
				{

					/// 자신은 통과
					if( pChild->GetObjectID() == mTarget.index && pChild->GetObjectType() == mTarget.type )
					{
						pChild = GRIDMANAGER->FindNextPlayer();
						continue;
					}

					if( pChild->GetStateDie() == true || pChild->IsAttack( mTarget.index ) == true )
					{
						pChild = GRIDMANAGER->FindNextPlayer();
						continue;
					}

					NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );
					if( mRangeCheck.IsRange( childPos, parentPos ) )
					{
						/// 같은 오라 효과가 복사되는것 방지
						if( pChild->IsParentEqual( mUniqueIdx ) == true )
						{
							pChild = GRIDMANAGER->FindNextPlayer();
							continue;					
						}

						SKILLMANAGER->AddInfChildAura( mTarget, pChild->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
					}	
					pChild = GRIDMANAGER->FindNextPlayer();
				}
			}
			break;
		case eAPPLYTYPE_SELF:
			break;
		case eAPPLYTYPE_ENEMY:
			{
				/// 플레이어
				cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pParentObejct );
				while( pPlayer != NULL )
				{
					/// 자신은 통과
					if( pPlayer->GetObjectID() == mTarget.index && pPlayer->GetObjectType() == mTarget.type )
					{
						pPlayer = GRIDMANAGER->FindNextPlayer();
						continue;
					}

					if( pPlayer->GetStateDie() == true || pPlayer->IsAttack( mTarget.index ) == true )
					{
						pPlayer = GRIDMANAGER->FindNextPlayer();
						continue;
					}

					NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					if( mRangeCheck.IsRange( childPos, parentPos ) )
					{
						/// 같은 오라 효과가 복사되는것 방지
						if( pPlayer->IsParentEqual( mUniqueIdx ) == true )
						{
							pPlayer = GRIDMANAGER->FindNextPlayer();
							continue;					
						}

						SKILLMANAGER->AddInfChildAura( mTarget, pPlayer->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
					}	
					pPlayer = GRIDMANAGER->FindNextPlayer();
				}

				/// 몬스터
				cMonster* pMonster = GRIDMANAGER->FindFirstMonster( pParentObejct );
				while( pMonster != NULL )
				{
					if( pMonster->GetStateDie() == true )
					{
						pMonster = GRIDMANAGER->FindNextMonster();
						continue;
					}

					NiPoint3 childPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
					if( mRangeCheck.IsRange( childPos, parentPos ) )
					{
						/// 같은 오라 효과가 복사되는것 방지
						if( pMonster->IsParentEqual( mUniqueIdx ) == true )
						{
							pMonster = GRIDMANAGER->FindNextMonster();
							continue;					
						}

						SKILLMANAGER->AddInfChildAura( mTarget, pMonster->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
					}	
					pMonster = GRIDMANAGER->FindNextMonster();
				}
			}
			break;
		default:
			NETWORK2->PostServerEvent("cSkillManager::CopyAuraInfluence mAuraCopyType[%d,%d,%d,%d] ERROR ", 
				mCopyApplyType, pParentObejct->GetObjectID(), pParentObejct->GetState(), pParentObejct->GetStateStop() );
		}
	}
	else if( mTarget.type == eOBJECTTYPE_MONSTER )
	{
		/// 오라 효과의 부모객체
		cMonster* pParentObejct = OBJECTMANAGER->GetMonster( mTarget.index );
		if( pParentObejct == NULL )
			return;

		/// 부모객체의 위치와 타겟의 위치
		NiPoint3 parentPos( pParentObejct->GetXPos(), pParentObejct->GetYPos(), pParentObejct->Height() );
		mRangeCheck.SetRadius( mCopyRange );

		switch( mCopyApplyType )
		{
		case eAPPLYTYPE_PARTY:
			{
				NETWORK2->PostServerEvent("cInfluenceObject::CopyAuraInfluence() eOBJECTTYPE_MONSTER eAPPLYTYPE_PARTY [%d,%d]", 
					pParentObejct->GetRaceGender(), mInfluenceClassIdx );
			}
			break;
		case eAPPLYTYPE_SELF:
			{
				NETWORK2->PostServerEvent("cInfluenceObject::CopyAuraInfluence() eOBJECTTYPE_MONSTER eAPPLYTYPE_SELF [%d,%d]", 
					pParentObejct->GetRaceGender(), mInfluenceClassIdx );
			}
			break;
		case eAPPLYTYPE_BUDDY:
			{
				/// 몬스터
				cMonster* pMonster = GRIDMANAGER->FindFirstMonster( pParentObejct );
				while( pMonster != NULL )
				{
					/// 자신은 통과
					if( pMonster->GetObjectID() == mTarget.index && pMonster->GetObjectType() == mTarget.type )
					{
						pMonster = GRIDMANAGER->FindNextMonster();
						continue;
					}

					if( pMonster->GetStateDie() == true )
					{
						pMonster = GRIDMANAGER->FindNextMonster();
						continue;
					}

					NiPoint3 childPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
					if( mRangeCheck.IsRange( childPos, parentPos ) )
					{
						/// 같은 오라 효과가 복사되는것 방지
						if( pMonster->IsParentEqual( mUniqueIdx ) == true )
						{
							pMonster = GRIDMANAGER->FindNextMonster();
							continue;					
						}

						SKILLMANAGER->AddInfChildAura( mTarget, pMonster->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
					}	
					pMonster = GRIDMANAGER->FindNextMonster();
				}
			}
			break;
		case eAPPLYTYPE_ENEMY:
			{
				/// 플레이어
				cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( pParentObejct );
				while( pPlayer != NULL )
				{
					if( pPlayer->GetStateDie() == true )
					{
						pPlayer = GRIDMANAGER->FindNextPlayer();
						continue;
					}

					NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					if( mRangeCheck.IsRange( childPos, parentPos ) )
					{
						/// 같은 오라 효과가 복사되는것 방지
						if( pPlayer->IsParentEqual( mUniqueIdx ) == true )
						{
							pPlayer = GRIDMANAGER->FindNextPlayer();
							continue;					
						}

						SKILLMANAGER->AddInfChildAura( mTarget, pPlayer->GetObject(), mInfluenceClassIdx, mSkillIdx, mUniqueIdx, true );
					}	
					pPlayer = GRIDMANAGER->FindNextPlayer();
				}
			}
			break;
		default:
			NETWORK2->PostServerEvent("cSkillManager::CopyAuraInfluence eOBJECTTYPE_MONSTER default[%d,%d] ", 
				mCopyApplyType, pParentObejct->GetObjectID() );
		}
	}
}


bool cInfluenceObject::DeleteChildAuraInfluence()
{

	cInfluenceObject* pInfluenceObject = SKILLMANAGER->GetInfluence( mParentIdx );
	if( pInfluenceObject == NULL )
		return true;

	cBaseObject* pParent = OBJECTMANAGER->GetObject( mAttacker.type, mAttacker.index );
	if( pParent == NULL || pParent->GetStateDie() == true )
		return true;

	cBaseObject* pChild = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
	if( pChild == NULL || pChild->GetStateDie() == true )
		return true;

	if( pParent->GetMapNumber() != pChild->GetMapNumber() )
		return true;

	/// 부모가 파티원들에게 복사해주는 효과라면 파티 존재여부를 체크한다.
	if( mCopyApplyType == eAPPLYTYPE_PARTY )
	{
		if( pParent->GetObjectType() == eOBJECTTYPE_PLAYER && pChild->GetObjectType() == eOBJECTTYPE_PLAYER )
		{
			if( ((cPlayer*)pParent)->GetPartyIndex() == 0 && ((cPlayer*)pParent)->GetPartyUnionIndex() == 0 )
				return true;

			if( ((cPlayer*)pParent)->GetPartyIndex() > 0 )
			{
				if( ((cPlayer*)pParent)->GetPartyIndex() != ((cPlayer*)pChild)->GetPartyIndex() )
					return true;
			}
			else if( ((cPlayer*)pParent)->GetPartyUnionIndex() > 0 )
			{
				unsigned long partyUnionIndex = ((cPlayer*)pParent)->GetPartyUnionIndex();
				cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( partyUnionIndex );
				if( partyUnion != NULL )
				{
					// 파티연합에서 동일 파티군이 아니면 효과 빼주기
					if( partyUnion->IsUser( pParent->GetObjectID(), pChild->GetObjectID() ) == false )
						return true;
				}
				else
				{
					assert(0);
				}
			}
		}
	}

	NiPoint3 parentPos( pParent->GetXPos(), pParent->GetYPos(), pParent->Height() );
	NiPoint3 childPos( pChild->GetXPos(), pChild->GetYPos(), pChild->Height() );

	cRangeCheck rangeCheck( pInfluenceObject->GetRange() );
	return rangeCheck.IsNotRange( parentPos, childPos );
}


bool cInfluenceObject::CheckTotemInfluence()
{
	/// 부모 토템 객체 확인
	cTotem* pTotemObject = OBJECTMANAGER->GetTotem( mParentIdx );
	if( pTotemObject == NULL )
		return true;

	cBaseObject* pTarget = OBJECTMANAGER->GetObject( mTarget.type, mTarget.index );
	if( pTarget == NULL )
	{
		pTotemObject->EraseTarget( mTarget );	/// 부모 토템에서 타겟 목록 제거
		return true;
	}

	/// 부모 와 효과(타겟) 거리 계산
	NiPoint3 parentPos( pTotemObject->GetXPos(), pTotemObject->GetYPos(), pTotemObject->Height() );
	NiPoint3 childPos( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height() );

	cRangeCheck rangeCheck( pTotemObject->GetRange() );
	if( rangeCheck.IsNotRange( parentPos, childPos ) == true )
	{
		pTotemObject->EraseTarget( mTarget );	/// 부모 토템에서 타겟 목록 제거
		return true;
	}

	return false;
}


bool cInfluenceObject::CalcLeftGuard( unsigned char attributeType, unsigned long* damage )
{
	if( damage == NULL )
		return false;

	if( mGuardType == eINFGUARDTYPE_NONE || mGuardType == eINFGUARDTYPE_CASHEXP )
		return false;

	/// 속성확인
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		if( mGuardType == eINFGUARDTYPE_PHY || mGuardType == eINFGUARDTYPE_ALL )
		{
			/// 카운트 감소
			--mLeftGuardCnt; 
			/// 데미지 양만큼 감소
			if( mLeftGuardValue > *damage )
			{
				mLeftGuardValue = mLeftGuardValue - (unsigned short)*damage;
				*damage = 0;
			}
			else
			{
				*damage = *damage - mLeftGuardValue;
				mLeftGuardValue = 0;
			}
			return true;
		}
		else
			return false;
	}
	else if( attributeType == eATTRIBUTETYPE_MAGIC )
	{
		if( mGuardType == eINFGUARDTYPE_MAG || mGuardType == eINFGUARDTYPE_ALL )
		{
			--mLeftGuardCnt; 
			if( mLeftGuardValue > *damage )
			{
				mLeftGuardValue = mLeftGuardValue - (unsigned short)*damage;
				*damage = 0;
			}
			else
			{
				*damage = *damage - mLeftGuardValue;
				mLeftGuardValue = 0;
			}
			return true;
		}
		else
			return false;
	}
	else
		return false;
}


bool cInfluenceObject::CalcCashExpGuard()
{
	if( mGuardType == eINFGUARDTYPE_CASHEXP )
	{
		/// 카운트 감소
		--mLeftGuardCnt; 
		return true;
	}
	else
		return false;
}


void cInfluenceObject::TotemSetEraseTarget()
{
	/// 부모 토템 객체 확인
	cTotem* pTotemObject = OBJECTMANAGER->GetTotem( mParentIdx );
	if( pTotemObject != NULL )
		pTotemObject->EraseTarget( mTarget );
}
