#include "stdafx.h"
#include "SkillExecuter.h"

#include "SkillManager.h"
#include "ObjectManager.h"
#include "WorldManager.h"
#include "Hero.h"
#include "monster.h"
#include "protocol.h"

#include "skillscript.h"
#include "TotemScript.h"
#include "GameUIManager.h"
#include "SkillWindow.h"

#include "ItemManager.h"
#include "chatmanager.h"
#include "GameResourceManager.h"
#include "QuickManager.h"
#include "UICursor.h"
#include "CastingBar.h"

#include "DuelManager.h"
#include "PVPManager.h"
#include "PartyManager.h"
#include "PartyUnionManager.h"
#include "rangecheck.h"
#include "TutorialManager.h"
#include "CheatManager.h"

//////////////////////////////////////////////////////////////////////////
unsigned int cAttackPattern::GetAttackFlag()
{
	bool newPattern = false;
	if( mPatternIdx < 0 || mPatternIdx > 2 )
		newPattern = true;
	if( mProgressIdx > 5 )
		newPattern = true;

	if( newPattern == true )
	{
		/// 새로 생성
		unsigned int r = ::rand()%1000;
		if( r < 330 )		mPatternIdx = 0;
		else if( r < 660 )	mPatternIdx = 1;
		else				mPatternIdx = 2;

		mProgressIdx = 0;
	}
	else
	{
		if( mPattern[mPatternIdx][mProgressIdx] == -1 )
		{
			/// 새로 생성
			unsigned int r = ::rand()%1000;
			if( r < 330 )		mPatternIdx = 0;
			else if( r < 660 )	mPatternIdx = 1;
			else				mPatternIdx = 2;

			mProgressIdx = 0;
		}
	}

	int curIdx = mPattern[mPatternIdx][mProgressIdx];
	if( mOldAttackIdx == curIdx )
	{
		mProgressIdx++;

		if( mProgressIdx > 5 )
		{
			assert(0);
		}
	}

	mOldAttackIdx = mPattern[mPatternIdx][mProgressIdx];
	mProgressIdx++;

	return (unsigned int)mOldAttackIdx;
}

//////////////////////////////////////////////////////////////////////////
cSkillExecuter* cSkillExecuter::mpExec = NULL;

cSkillExecuter::cSkillExecuter()
{
	mpExec = this;

	mChargeSkill.mChargeType = eCharge_None;
	mChargeSkill.mSkillIdx = 0;
	mChargeSkill.mSkillStep = 0;
	mChargeSkill.mSkillRange = FLT_MAX;

	mAttackTarget.index = 0;
	mAttackTarget.type = eOBJECTTYPE_NONE;

	mpCurrentHeroSkillObject = 0;
}

cSkillExecuter::~cSkillExecuter()
{
}

void cSkillExecuter::Close()
{
	mChargeSkill.mChargeType = eCharge_None;
	mChargeSkill.mSkillIdx = 0;
	mChargeSkill.mSkillStep = 0;
	mChargeSkill.mSkillRange = FLT_MAX;

	mAttackTarget.index = 0;
	mAttackTarget.type = eOBJECTTYPE_NONE;

	mpCurrentHeroSkillObject = 0;
}

bool cSkillExecuter::AutoAttack( cBaseObject* pTarget )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return true;

	if( pTarget == 0 )
		return false;

	if( HERO->IsTransformMonster() )
	{
		unsigned long skillIdx = GenerateTransMonsAttackIdx();
		if( skillIdx > eMONSTERATTACK_NORMAL2 )
			return true;

		return AutoAttack_TransMon( skillIdx, pTarget );
	}

	/// 스킬 사용 불가
	if( HERO->IsCantSkill( NORMAL_ATTACK_SKILL ) == true )
		return false;

	/// target의 상태 체크..
	unsigned char type = pTarget->GetObjectType();
	switch( type )
	{
	case eOBJECTTYPE_PLAYER:
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetState() == eOBJECT_STATE_DIE )
				return false;

			/// 적인지 여부 판단
			if( DUELMAN->IsDuelEnemy( pPlayer ) != true && 
				PVPMAN->IsEnableAttack( pPlayer ) != true )
				return false;
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMon = (cMonster*)pTarget;
			if( pMon->GetState() == eOBJECT_STATE_DIE )
				return false;

			if( PVPMAN && PVPMAN->IsEnableBattle() )
			{
				if( ((cMonster*)pTarget)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pTarget)->GetPvPTeamType() == HERO->GetPVPTeam() )
					return false;
			}
		}
		break;
	default:
		return false;
	}

	/// 기본 공격 인덱스 산정 - 무기, 공격 인덱스
	ChargeOn( NORMAL_ATTACK_SKILL, 0 );

	/// 공격 거리 체크 - 후처리기
	HERO->AfterAttack( pTarget );

	return true;
}

bool cSkillExecuter::ExecuteAttack( cBaseObject* pTarget  )
{
	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	/// 080508 PKH 정지 상태일때 일반공격 하면 에니메이션이 바껴서 체크
	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	if( pTarget == 0 )
		return false;

	if( HERO->IsTransformMonster() )
	{
		unsigned long skillIdx = GenerateTransMonsAttackIdx();
		if( skillIdx > eMONSTERATTACK_NORMAL2 )
			return false;

		return ExecuteAttack_TransMon( skillIdx, pTarget );
	}

	/// 현재 보유 Target과 같은 Target을 공격하는 경우
	cBaseObject* pBeforeTarget = OBJECTMAN->GetObject( &mAttackTarget );
	if( pTarget == pBeforeTarget )
	{
		/// cooltime check
		if( SKILLMAN->IsUsedKeepSkill( NORMAL_ATTACK_SKILL, 0 ) == false )
			return false;
	}
	else
	{
		/// target window 및 hero 타겟 설정
		HERO->SetTargetObject( pTarget->GetObjectType(), pTarget->GetObjectID() );
	}

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_Attack &&
		p->mTarget.index == pTarget->GetObjectID() )
		return false;

	/// target의 상태 체크..
	unsigned char type = pTarget->GetObjectType();

	switch( type )
	{
	case eOBJECTTYPE_PLAYER:
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetState() == eOBJECT_STATE_DIE )
				return false;

			/// 적인지 여부 판단
			if( DUELMAN->IsDuelEnemy( pPlayer ) != true &&
				PVPMAN->IsEnableAttack( pPlayer ) != true )
				return false;
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMon = (cMonster*)pTarget;
			if( pMon->GetState() == eOBJECT_STATE_DIE )
				return false;
			
			if( PVPMAN && PVPMAN->IsEnableBattle() )
			{
				if( ((cMonster*)pTarget)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pTarget)->GetPvPTeamType() == HERO->GetPVPTeam() )
					return false;
			}
		}
		break;
	default:
		return false;
	}

	/// 기본 공격 인덱스 산정 - 무기, 공격 인덱스
	ChargeOn( NORMAL_ATTACK_SKILL, 0 );

	/// 공격 거리 체크 - 후처리기
	HERO->AfterAttack( pTarget );

	ResetAttackFlag();

	mAttackTarget.index = pTarget->GetObjectID();
	mAttackTarget.type = pTarget->GetObjectType();
	return true;
}

/// 타겟을 기준으로 한 스킬에 사용
void cSkillExecuter::ExecuteSkill( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pTarget )
{
	if( HERO->IsTransformMonster() )
	{
		ExecuteSkill_TransMon( skillIdx, pTarget );
		return;
	}

	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return;
#endif

	/// 080508 PKH 정지 상태일때 일반공격 하면 에니메이션이 바껴서 체크
	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return;
	}

	/// 스킬 정보를 획득한다.
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}

	if( pInfo->mShotType == eSHOTTYPE_AURA )
	{
		cSkillWindow* pSkillWindow = GAMEUI->GetSkillWindow();
		if( pSkillWindow == 0 )
		{
			assert(0);
			return;
		}

		unsigned long uniqueIdx = pSkillWindow->GetAuraUniqueIdx( skillIdx );
		if( uniqueIdx )
		{
			MSG_REQ_INFLUENCE_AURA_DELETE msg;

			msg.Category = NM_SKILL;
			msg.Protocol = NM_SKILL_INFLUENCE_AURA_DELETE_REQ;
			msg.mUniqueIdx = uniqueIdx;
			msg.mSkillClassIdx = skillIdx;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
			return;
		}
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return;

	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		assert(0);
		return;
	}

	/// 사용 가능 여부 체크
	if( IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	/// 캐스팅바 보이기
	if( pInfo->mShotType == eSHOTTYPE_CASTING )
	{
		cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
		if( pWin && pWin->IsVisible() )
		{
			if( HERO->IsReadyUseSkill() )
				return;
		}
	}

	/// 
	bool targetCheck = true;
	if( pTarget == 0 )
		pTarget = HERO->GetTargetObject();

	if( pInfo->mBoundType == eBOUNDTYPE_FIELD )
	{
		FieldSkillOn( skillIdx, skillStep );
		return;
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_SELF_NOTARGET )
	{
		/// active
		targetCheck = false;
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_AUTOFIELD )
	{
		ExecuteAutoPos( skillIdx, skillStep );
		return;
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_NONE )
	{
		if( pInfo->mApplyType == eAPPLYTYPE_SELF	  || 
			pInfo->mApplyType == eAPPLYTYPE_BUFF_SELF || 
			pInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF )
			pTarget = HERO;
		else if( pInfo->mApplyType == eAPPLYTYPE_BUDDY	|| 
				 pInfo->mApplyType == eAPPLYTYPE_PARTY	||
				 pInfo->mApplyType == eAPPLYTYPE_BUFF_PARTY ||
				 pInfo->mApplyType == eAPPLYTYPE_DEBUFF_PARTY )
		{
			if( pTarget == 0 )
				pTarget = HERO;
		}

		targetCheck = true;
	}

	/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
	/// (쓸수 있는지를 체크하기 위해 자기 자신에게 쓰는 경우만 체크, 타겟이 필요한 경우에는 chargeOn을 위해 skip )
	if( pInfo->mApplyValueType != eSTATUSPLUS_NONE )
	{
		switch( pInfo->mApplyType )
		{
		case eAPPLYTYPE_BUFF_SELF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyBuffType( pInfo->mApplyValueType ) == false )
					return;
			}
			break;
		case  eAPPLYTYPE_DEBUFF_SELF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyDeBuffType( pInfo->mApplyValueType ) == false )
					return;
			}
			break;
		}
	}

	if( targetCheck )
	{
		/// 적용 타겟이 아닌 경우 charge
		if( IsPossibleUseSkillTarget( skillIdx, pTarget ) == false )
		{
			ChargeOn( skillIdx, skillStep );
			return;
		}

		/// 
		cBaseObject* pBeforeTarget = OBJECTMAN->GetObject( &mAttackTarget );
		if( pTarget == pBeforeTarget )
		{
			/// cooltime check
			if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == false )
				return;
		}

		sAfterAction* p = HERO->GetCurrentAfterAction();
		if( p->mActionType == eAfterAction_UseTargetSkill &&
			p->mExData1 == skillIdx &&
			p->mExData2 == skillStep &&
			p->mTarget.index == pTarget->GetObjectID() )
			return;
	}

	/// 스킬 발동
	ChargeOn( skillIdx, skillStep );
	HERO->AfterUseSkill( pTarget );

	if( pTarget )
		mAttackTarget = *(pTarget->GetTargetInfo());
}

/// BOUNDTYPE = eBOUNDTYPE_FIELD 사용
void cSkillExecuter::ExecuteSkill( unsigned long skillIdx, unsigned char skillStep, NiPoint3 applyPos )
{
	if( HERO->IsTransformMonster() )
	{
		ExecuteSkill_TransMon( skillIdx, applyPos );
		return;
	}

	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return;
#endif

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return;

	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		assert(0);
		return;
	}

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}

	if( pInfo->mShotType == eSHOTTYPE_CASTING )
	{
		cCastingBar* pWin = (cCastingBar*)UIMAN->FindNodeByID( eUIID_GAME_CASTINGBAR );
		if( pWin && pWin->IsVisible() )
		{
			if( HERO->IsReadyUseSkill() )
				return;
		}
	}

	/// 사용 가능 여부 체크
	if( IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	if( pInfo->mBoundType != eBOUNDTYPE_FIELD )
		return;

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_UseFieldSkill &&
		p->mExData1 == skillIdx &&
		p->mExData2 == skillStep )
		return;

	ChargeOn( skillIdx, skillStep );
	HERO->AfterUseSkill( applyPos );
}

void cSkillExecuter::ExecuteAutoPos( unsigned long skillIdx, unsigned char skillStep )
{
#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return;
#endif

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	if( pInfo->mBoundType != eBOUNDTYPE_AUTOFIELD )
	{
		assert(0);
		return;
	}

	if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == false )
		return;

	if( pInfo->mStepCount <= skillStep )
	{
		assert(0);
		return;
	}

	float range = pInfo->mpSetpInfoArray[skillStep].mTargetDist;
	mChargeSkill.mSkillRange = HERO->CalcStatusSkillRange( range, pInfo->mRangeType, skillIdx < NORMAL_ATTACK_SKILL_MAX );
	NiPoint3 heroPos = HERO->GetPos();

	NiPoint3 applyPos = heroPos + HERO->GetDesireDir() * mChargeSkill.mSkillRange;

	/// 좌표가 이동 불가 지역이면 재계산
	unsigned long divideCnt = 10;
	float divideRange = mChargeSkill.mSkillRange / (float)divideCnt;
	cRangeCheck rangecheck( mChargeSkill.mSkillRange );
	for( unsigned long i = 0; i <= divideCnt ; ++i )
	{
		NiPoint3 divPos = heroPos + ( HERO->GetDesireDir() * ( mChargeSkill.mSkillRange - ( divideRange * i ) ) );
		WORLDMAN->CalcHeight( &divPos.z, divPos.x, divPos.y );

		if( rangecheck.IsRange( heroPos, divPos ) == true )
		{
			applyPos = divPos;
			break;
		}
	}

	WORLDMAN->EnableRayGet( heroPos, applyPos );
	WORLDMAN->CalcHeight( &applyPos.z, applyPos.x, applyPos.y );

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_UseFieldSkill && p->mExData1 == skillIdx && p->mExData2 == skillStep )
		return;

	ChargeOn( skillIdx, skillStep );
	HERO->AfterUseSkill( applyPos );
}


bool cSkillExecuter::IsPossibleUseSkill( unsigned long skillIdx, unsigned char skillStep )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->IsTransformMonster() )
		return IsPossibleUseSkill_TransMon( skillIdx );

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	if( HERO->IsJumping() == true )
		return false;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	/// TODO : 보유 스킬인지 체크 및 쿨타임 체크
	if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == false )
		return false;

	if( pInfo->mStepCount <= skillStep )
	{
		assert(0);
		return false;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[skillStep];

	/// 소모 아이템 보유 여부 체크
	if( pInfo->mUseItem )
	{
		/// 인벤 체크
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
			return false;
	}

	if( TUTORIALMAN->IsTutorialMode() == false )
	{
		if( HERO->GetLevel() < pStepInfo->mPlayerLevel )
			return false;
	}
	
	/// 내 장비가 맞는지 체크
	if( pInfo->mUseEquipment )
	{
		eWEAPON_STATE weaponState = HERO->GetWeaponState();

		/// 착용 장비 체크
		switch( pInfo->mUseEquipment )
		{
		case eEQUIPTYPE_SWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD:
			if( weaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD:
			if( weaponState != eWEAPON_STATE_SHORTSWORD )
				return false;
			break;
		case eEQUIPTYPE_GUN:
			if( weaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		case eEQUIPTYPE_STAFF:
			if( weaponState != eWEAPON_STATE_STAFF )
				return false;
			break;
		case eEQUIPTYPE_SHIELD:
			if( weaponState != eWEAPON_STATE_SHEILD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_LONGSWORD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_LONGSWORD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD_GUN:
			if( weaponState != eWEAPON_STATE_SHORTSWORD &&
				weaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		case eEQUIPTYPE_CLOTH:
			if( HERO->GetArmorSets() != ARMOR_SETS_ROBES )
				return false;
			break;
		case eEQUIPTYPE_LIGHTARMOR:
			if( HERO->GetArmorSets() != ARMOR_SETS_LIGHT )
				return false;
			break;
		case eEQUIPTYPE_HEAVYARMOR:
			if( HERO->GetArmorSets() != ARMOR_SETS_HEAVY )
				return false;
			break;
		}
	}

	/// 사용 상태가 맞는지 체크
	if( pInfo->mUseState )
	{
		switch( pInfo->mUseState )
		{
		case eSKILL_USE_STATE_HP_PER_UP: // hp up
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_HP_PER_DOWN: // hp down
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_UP: // mp up
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_DOWN: // mp down
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_BUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_DEBUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		default:
			{
				assert(0);
				return false;
			}
		}
	}

	/// 소모 HP, MP 체크
	if( HERO->GetHP() < pStepInfo->mUseHP )
	{
		return false;
	}
	unsigned long totalUseMP = HERO->CalcUseMPExt( pStepInfo->mUseMP );
	if( HERO->GetMP() < totalUseMP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6001 ) );
		return false;
	}
	if( pStepInfo->mUseMPPer > 0 )
	{
		unsigned long currentUseMP = HERO->GetOriMpMax();
		currentUseMP = (unsigned long)((currentUseMP * pStepInfo->mUseMPPer) / 100);
		unsigned long totalUseMP = HERO->CalcUseMPExt( currentUseMP );
		if( HERO->GetMP() < totalUseMP )
			return false;
	}

//	if( pInfo->mBoundType == eBOUNDTYPE_FIELD )
//		return true;

	return true;
}

bool cSkillExecuter::IsPossibleUseSkillTarget( unsigned long skillIdx, cBaseObject* pTarget )
{
	if( HERO->IsTransformMonster() )
		return IsPossibleUseSkillTarget_TransMon( skillIdx, pTarget );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}
	
	/// 스킬 사용 조건 체크
	if( IsPossibleApplyValueType( skillIdx, pTarget ) == false )
		return false;

	/// 타겟이 없어도 발동하는 스킬
	/// 1. 타겟이 자기 자신인 경우
	/// 2. 타겟이 없는 범위 스킬인 경우
	/// 그 외는 모두 발동 불가
	if( pInfo->mApplyType != eAPPLYTYPE_SELF        &&
		pInfo->mApplyType != eAPPLYTYPE_BUFF_SELF   &&
		pInfo->mApplyType != eAPPLYTYPE_DEBUFF_SELF &&
		pInfo->mBoundType != eBOUNDTYPE_SELF_NOTARGET )
	{
		if( pTarget == 0 )
			return false;

		if( pInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
		{
			if( pTarget->GetState() == eOBJECT_STATE_DIE )
			{
				if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					/// 적인지 아군인지 구분
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return false;

					return true;
				}
			}
			return false;
		}
		else
		{
			if( pTarget->GetState() == eOBJECT_STATE_DIE )
			{
				return false;
			}
		}

		unsigned char type = pTarget->GetObjectType();
		switch( type )
		{
		case eOBJECTTYPE_HERO:
			{
				if( pInfo->mApplyType == eAPPLYTYPE_ENEMY	   || 
					pInfo->mApplyType == eAPPLYTYPE_BUFF_ENEMY || 
					pInfo->mApplyType == eAPPLYTYPE_DEBUFF_ENEMY )
					return false;
			}
			break;
		case eOBJECTTYPE_MONSTER:
			{
				if( pInfo->mApplyType != eAPPLYTYPE_ENEMY	   && 
					pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY && 
					pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
					return false;

				if( PVPMAN && PVPMAN->IsEnableBattle() )
				{
					if( ((cMonster*)pTarget)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pTarget)->GetPvPTeamType() == HERO->GetPVPTeam() )
						return false;
				}
			}
			break;
		case eOBJECTTYPE_PLAYER:
			{
				if( pInfo->mApplyType == eAPPLYTYPE_PARTY      || 
					pInfo->mApplyType == eAPPLYTYPE_BUFF_PARTY || 
					pInfo->mApplyType == eAPPLYTYPE_DEBUFF_PARTY )
				{
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return false;

					if( PARTYMAN->IsHeroParty( pTarget->GetObjectID()) || 
						PARTYUNIONMAN->IsHeroParty( pTarget->GetObjectID() ))
						return true;

					return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_BUDDY )
				{
					/// 적군이냐?
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_ENEMY		||
						 pInfo->mApplyType == eAPPLYTYPE_BUFF_ENEMY || 
						 pInfo->mApplyType == eAPPLYTYPE_DEBUFF_ENEMY )
				{
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return true;

					return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
				{
					if( PVPMAN->IsEnableBattle() )
						return false;
				}
			}
			break;
		default: 
			return false;
		}
	}

	return true;
}

bool cSkillExecuter::IsRushUseSkill( unsigned long skillIdx, unsigned char skillStep )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->IsTransformMonster() )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	if( HERO->IsJumping() == true )
		return false;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	if( pInfo->mStepCount <= skillStep )
	{
		assert(0);
		return false;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[skillStep];

	/// 소모 아이템 보유 여부 체크
	if( pInfo->mUseItem )
	{
		/// 인벤 체크
		sItemCount* p = ITEMMAN->GetItemCount( pInfo->mUseItem );
		unsigned int count = ( p ) ? p->bag : 0;
		if( count < pStepInfo->mUseItemCount )
			return false;
	}

	if( TUTORIALMAN->IsTutorialMode() == false )
	{
		if( HERO->GetLevel() < pStepInfo->mPlayerLevel )
			return false;
	}

	/// 내 장비가 맞는지 체크
	if( pInfo->mUseEquipment )
	{
		eWEAPON_STATE weaponState = HERO->GetWeaponState();

		/// 착용 장비 체크
		switch( pInfo->mUseEquipment )
		{
		case eEQUIPTYPE_SWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD:
			if( weaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD:
			if( weaponState != eWEAPON_STATE_SHORTSWORD )
				return false;
			break;
		case eEQUIPTYPE_GUN:
			if( weaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		case eEQUIPTYPE_STAFF:
			if( weaponState != eWEAPON_STATE_STAFF )
				return false;
			break;
		case eEQUIPTYPE_SHIELD:
			if( weaponState != eWEAPON_STATE_SHEILD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_LONGSWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_LONGSWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_LONGSWORD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SWORD_LONGSWORD_DOUBLESWORD:
			if( weaponState != eWEAPON_STATE_SWORD &&
				weaponState != eWEAPON_STATE_SWORD_SHEILD &&
				weaponState != eWEAPON_STATE_LONGSWORD &&
				weaponState != eWEAPON_STATE_DOUBLESWORD )
				return false;
			break;
		case eEQUIPTYPE_SHORTSWORD_GUN:
			if( weaponState != eWEAPON_STATE_SHORTSWORD &&
				weaponState != eWEAPON_STATE_GUN )
				return false;
			break;
		case eEQUIPTYPE_CLOTH:
			if( HERO->GetArmorSets() != ARMOR_SETS_ROBES )
				return false;
			break;
		case eEQUIPTYPE_LIGHTARMOR:
			if( HERO->GetArmorSets() != ARMOR_SETS_LIGHT )
				return false;
			break;
		case eEQUIPTYPE_HEAVYARMOR:
			if( HERO->GetArmorSets() != ARMOR_SETS_HEAVY )
				return false;
			break;
		}
	}

	/// 사용 상태가 맞는지 체크
	if( pInfo->mUseState )
	{
		switch( pInfo->mUseState )
		{
		case eSKILL_USE_STATE_HP_PER_UP: // hp up
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_HP_PER_DOWN: // hp down
			{
				unsigned long hp = HERO->GetHP();
				unsigned long maxHP = HERO->GetMaxHP();
				if( maxHP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (hp * 100 / maxHP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_UP: // mp up
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue > (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_MP_PER_DOWN: // mp down
			{
				unsigned long mp = HERO->GetMP();
				unsigned long maxMP = HERO->GetMaxMP();
				if( maxMP == 0 )
				{
					assert(0);
					return false;
				}

				if( pStepInfo->mUseStateValue < (mp * 100 / maxMP) )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_BUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		case eSKILL_USE_STATE_SELF_DEBUFF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				if( HERO->IsHaveApplyDeBuffType( pStepInfo->mUseStateValue ) == false )
					return false;
			}
			break;
		default:
			{
				assert(0);
				return false;
			}
		}
	}

	/// 소모 HP, MP 체크
	if( HERO->GetHP() < pStepInfo->mUseHP )
	{
		return false;
	}
	unsigned long totalUseMP = HERO->CalcUseMPExt( pStepInfo->mUseMP );
	if( HERO->GetMP() < totalUseMP )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6001 ) );
		return false;
	}
	if( pStepInfo->mUseMPPer > 0 )
	{
		unsigned long currentUseMP = HERO->GetOriMpMax();
		currentUseMP = (unsigned long)((currentUseMP * pStepInfo->mUseMPPer) / 100);
		unsigned long totalUseMP = HERO->CalcUseMPExt( currentUseMP );
		if( HERO->GetMP() < totalUseMP )
			return false;
	}

	//	if( pInfo->mBoundType == eBOUNDTYPE_FIELD )
	//		return true;

	return true;
}

void cSkillExecuter::ChargeOn( unsigned long skillIdx, unsigned char skillstep )
{
	if( IsSkillCharge() )
	{
		ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );
		return;
	}

	/// 
	if( HERO->IsTransformMonster() )
	{
		ChargeOn_TransMon( skillIdx );
		return;
	}

	if( skillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		unsigned int state = (unsigned int)HERO->GetWeaponState();
		if( state > eWEAPON_STATE_MAX )
			state -= eWEAPON_STATE_SHEILD;

		unsigned int flag = 0;//mAttackPattern.GetAttackFlag();
		mChargeSkill.mSkillIdx = flag*10 + state + 1;
		mChargeSkill.mSkillStep = 0;
	}
	else
	{
		mChargeSkill.mSkillIdx = skillIdx;
		mChargeSkill.mSkillStep = skillstep;
	}
	
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mChargeSkill.mSkillIdx );
	assert(pInfo);

	if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillstep ) == false )
	{
		return;
	}

	/// error check
	if( pInfo->mStepCount <= skillstep )
	{
		assert(0);
		return;
	}

	/// skill step은 획득한다. 추가해야 한다.
	float range = pInfo->mpSetpInfoArray[skillstep].mTargetDist;

	/// 080225 PKH 스킬+아이템+효과 사거리
	mChargeSkill.mSkillRange = HERO->CalcStatusSkillRange( range, pInfo->mRangeType, mChargeSkill.mSkillIdx < NORMAL_ATTACK_SKILL_MAX );

	/// 퀵슬롯 스킬에 차지 세팅
	QUICKMAN->SetCharge( skillIdx, true );
	CURSOR->SetCursor( eCURSOR_SKILL );

	mChargeSkill.mChargeType = eCharge_Target;
}

void cSkillExecuter::ChargeOff( unsigned long skillIdx, unsigned char skillStep )
{
	/// 
	if( HERO->IsTransformMonster() )
	{
		ChargeOff_TransMon( skillIdx );
		return;
	}

	if( mChargeSkill.mSkillIdx != skillIdx && mChargeSkill.mSkillStep != skillStep )
		return;

	mChargeSkill.mChargeType = eCharge_None;
	mChargeSkill.mSkillIdx = 0;
	mChargeSkill.mSkillStep = 0;

	/// 퀵슬롯 스킬에 차지 세팅
	QUICKMAN->SetCharge( skillIdx, false );
	CURSOR->SetCursor( eCURSOR_DEFAULT );
}

bool cSkillExecuter::IsSkillCharge()
{
	if( mChargeSkill.mChargeType == eCharge_None )
		return false;

	if( HERO->IsTransformMonster() )
	{
		if( mChargeSkill.mSkillIdx < eMONSTERATTACK_SKILL1 )
			return false;
	}
	else
	{
		if( mChargeSkill.mSkillIdx < NORMAL_ATTACK_SKILL_MAX )
			return false;
	}
	return true;
}

sChargeInfo cSkillExecuter::GetChargeSkill()
{
	/// 
	if( HERO->IsTransformMonster() )
		return mChargeSkill;

	if( mChargeSkill.mSkillIdx < NORMAL_ATTACK_SKILL_MAX )
	{
		unsigned int state = (unsigned int)HERO->GetWeaponState();
		if( state > eWEAPON_STATE_MAX )
			state -= eWEAPON_STATE_SHEILD;

		unsigned int flag = mAttackPattern.GetAttackFlag();
		unsigned long skillIdx = flag*10 + state + 1;

		if( skillIdx != mChargeSkill.mSkillIdx )
		{
			/// 무기가 교체된 경우는 사정거리 재계산
			mChargeSkill.mSkillIdx = skillIdx;
			mChargeSkill.mSkillStep = 0;

			sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( mChargeSkill.mSkillIdx );
			assert(pInfo);

			if( pInfo->mStepCount == 0 )
			{
				assert(0);
			}

			float range = pInfo->mpSetpInfoArray[0].mTargetDist;

			/// 080225 PKH 스킬+아이템+효과 사거리
			mChargeSkill.mSkillRange = HERO->CalcStatusSkillRange( range, pInfo->mRangeType, true );
		}
	}
	return mChargeSkill;
}

///

unsigned long cSkillExecuter::GenerateTransMonsAttackIdx()
{
	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();

	/// monster skill
	unsigned long rd = ::rand() % mTotalMonsterAttackPer;

	sMonsterSkillScript* info = NULL;
	/// 기본 공격중에 조건을 만족하는 것을 리턴한다.
	for( unsigned int i=0;i<eMONSTERATTACK_SKILL1;i++ )
	{
		if( mMonsterAttackInfo[i].mUsePer == 0 )
			continue;

		if( mMonsterAttackInfo[i].mUsePer < rd )
			rd -= mMonsterAttackInfo[i].mUsePer;
		else
		{
			info = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)i );
			if( info == 0 )
				continue;

			unsigned long totalUseMP = HERO->CalcUseMPExt( info->mUseMP );
			if( totalUseMP > HERO->GetMP() )
				continue;

			sKeepSkill* keep = SKILLMAN->GetCoolTimeMon( i );
			if( keep && keep->mRestTime != 0 && keep->mRestTime != ULONG_MAX )
			{
				rd = 0;
				continue;
			}
			return i;
		}
	}

	/// 아무것도 선택되지 않은경우 기본공격1에 대한 체크로 보정
	info = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, eMONSTERATTACK_NORMAL1 );
	if( info )
	{
		unsigned long totalUseMP = HERO->CalcUseMPExt( info->mUseMP );
		if( totalUseMP < HERO->GetMP() )
		{
			sKeepSkill* keep = SKILLMAN->GetCoolTimeMon( eMONSTERATTACK_NORMAL1 );
			if( keep && (keep->mRestTime == 0 || keep->mRestTime == ULONG_MAX) )
			{
				return eMONSTERATTACK_NORMAL1;
			}
		}
	}

	return eMONSTERATTACK_MAX;
}

void cSkillExecuter::ChargeOn_TransMon( unsigned long skillIdx )
{
	if( skillIdx >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return;
	}

	mChargeSkill.mChargeType = eCharge_Target;
	mChargeSkill.mSkillIdx = skillIdx;
	mChargeSkill.mSkillRange = mMonsterAttackInfo[skillIdx].mRange;

	CURSOR->SetCursor( eCURSOR_SKILL );
	QUICKMAN->SetMonCharge( skillIdx, true );
}

void cSkillExecuter::ChargeOff_TransMon( unsigned long skillIdx )
{
	if( mChargeSkill.mChargeType == eCharge_None )
		return;

	if( skillIdx >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return;
	}

	if( mChargeSkill.mSkillIdx != skillIdx )
		return;

	mChargeSkill.mSkillIdx = 0;
	mChargeSkill.mSkillStep = 0;
	mChargeSkill.mChargeType = eCharge_None;

	CURSOR->SetCursor( eCURSOR_DEFAULT );
	QUICKMAN->SetMonCharge( skillIdx, false );
}

void cSkillExecuter::SetMonsterSkillInfo( unsigned long monsterClassIdx )
{
	mTotalMonsterAttackPer = 0;

	sMonsterSkillScript* info;
	for( unsigned int i=0;i<eMONSTERATTACK_MAX;i++ )
	{
		mMonsterAttackInfo[i].mUsePer = 0;

		info = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)i );
		if( info == 0 )
			continue;

		mMonsterAttackInfo[i].mUsePer = info->mSkillUsePer;
		mMonsterAttackInfo[i].mRange = info->mTargetDist;

		if( i < eMONSTERATTACK_SKILL1 )
			mTotalMonsterAttackPer += mMonsterAttackInfo[i].mUsePer;
	}
}

bool cSkillExecuter::IsPossibleUseSkill_TransMon( unsigned long skillIdx )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();
	if( monsterClassIdx == 0 )
		return false;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return false;

	/// TODO : 보유 스킬인지 체크 및 쿨타임 체크
	if( SKILLMAN->IsUsedMonsterSkill( skillIdx ) == false )
		return false;

	/// 소모 MP 체크
	unsigned long totalUseMP = HERO->CalcUseMPExt( pInfo->mUseMP  );
	if( HERO->GetMP() < totalUseMP)
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 6001 ) );
		return false;
	}
//	if( pInfo->mBoundType == eBOUNDTYPE_FIELD )
//		return true;

	return true;
}

bool cSkillExecuter::IsPossibleUseSkillTarget_TransMon( unsigned long skillIdx, cBaseObject* pTarget )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();
	if( monsterClassIdx == 0 )
		return false;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return false;

	/// 타겟이 없어도 발동하는 스킬
	/// 1. 타겟이 자기 자신인 경우
	/// 2. 타겟이 없는 범위 스킬인 경우
	/// 그 외는 모두 발동 불가
	if( pInfo->mApplyType != eAPPLYTYPE_SELF		&&
		pInfo->mApplyType != eAPPLYTYPE_BUFF_SELF   &&
		pInfo->mApplyType != eAPPLYTYPE_DEBUFF_SELF &&
		pInfo->mBoundType != eBOUNDTYPE_SELF_NOTARGET )
	{
		if( pTarget == 0 )
			return false;

		if( pInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
		{
			if( pTarget->GetState() == eOBJECT_STATE_DIE )
			{
				if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
				{
					/// 적인지 아군인지 구분
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) != true &&
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) != true )
						return false;

					return true;
				}
			}
			return false;
		}
		else
		{
			if( pTarget->GetState() == eOBJECT_STATE_DIE )
			{
				return false;
			}
		}

		unsigned char type = pTarget->GetObjectType();
		switch( type )
		{
		case eOBJECTTYPE_HERO:
			{
				if( pInfo->mApplyType == eAPPLYTYPE_ENEMY      ||
					pInfo->mApplyType == eAPPLYTYPE_BUFF_ENEMY ||
					pInfo->mApplyType == eAPPLYTYPE_DEBUFF_ENEMY )
					return false;
			}
			break;
		case eOBJECTTYPE_MONSTER:
			{
				if( pInfo->mApplyType != eAPPLYTYPE_ENEMY      &&
					pInfo->mApplyType != eAPPLYTYPE_BUFF_ENEMY &&
					pInfo->mApplyType != eAPPLYTYPE_DEBUFF_ENEMY )
					return false;

				if( PVPMAN && PVPMAN->IsEnableBattle() )
				{
					if( ((cMonster*)pTarget)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pTarget)->GetPvPTeamType() == HERO->GetPVPTeam() )
						return false;
				}
			}
			break;
		case eOBJECTTYPE_PLAYER:
			{
				if( pInfo->mApplyType == eAPPLYTYPE_PARTY      || 
					pInfo->mApplyType == eAPPLYTYPE_BUFF_PARTY || 
					pInfo->mApplyType == eAPPLYTYPE_DEBUFF_PARTY )
				{
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return false;

					if( PARTYMAN->IsHeroParty( pTarget->GetObjectID()) ||
						PARTYUNIONMAN->IsHeroParty( pTarget->GetObjectID() ) )
						return true;

					return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_BUDDY )
				{
					/// 적군이냐?
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_ENEMY      ||
						 pInfo->mApplyType == eAPPLYTYPE_BUFF_ENEMY ||
						 pInfo->mApplyType == eAPPLYTYPE_DEBUFF_ENEMY )
				{
					if( DUELMAN->IsDuelEnemy( (cPlayer*)pTarget ) ||
						PVPMAN->IsEnableAttack( (cPlayer*)pTarget ) )
						return true;

					return false;
				}
				else if( pInfo->mApplyType == eAPPLYTYPE_DIEBUDDY )
				{
					if( PVPMAN->IsEnableBattle() )
						return false;
				}
			}
			break;
		default: 
			return false;
		}
	}

	return true;
}

bool cSkillExecuter::AutoAttack_TransMon( unsigned long skillIdx, cBaseObject* pTarget )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	/// 스킬 사용 불가
	if( HERO->IsCantSkill( skillIdx ) == true )
		return false;

	/// target의 상태 체크..
	unsigned char type = pTarget->GetObjectType();
	switch( type )
	{
	case eOBJECTTYPE_PLAYER:
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetState() == eOBJECT_STATE_DIE )
				return false;

			/// 적인지 여부 판단
			if( DUELMAN->IsDuelEnemy( pPlayer ) != true &&
				PVPMAN->IsEnableAttack( pPlayer ) != true )
				return false;
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMon = (cMonster*)pTarget;
			if( pMon->GetState() == eOBJECT_STATE_DIE )
				return false;

			if( PVPMAN && PVPMAN->IsEnableBattle() )
			{
				if( ((cMonster*)pTarget)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pTarget)->GetPvPTeamType() == HERO->GetPVPTeam() )
					return false;
			}
		}
		break;
	default:
		return false;
	}

	/// 기본 공격 인덱스 산정 - 무기, 공격 인덱스
	ChargeOn( skillIdx, 0 );

	/// 공격 거리 체크 - 후처리기
	HERO->AfterAttack( pTarget );

	return true;
}

bool cSkillExecuter::ExecuteAttack_TransMon( unsigned long skillIdx, cBaseObject* pTarget )
{

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->IsCantSkill( skillIdx ) == true )
		return false;

	/// target window 및 hero 타겟 설정
	HERO->SetTargetObject( pTarget->GetObjectType(), pTarget->GetObjectID() );

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_Attack &&
		p->mTarget.index == pTarget->GetObjectID() )
		return false;

	/// target의 상태 체크..
	unsigned char type = pTarget->GetObjectType();

	switch( type )
	{
	case eOBJECTTYPE_PLAYER:
		{
			cPlayer* pPlayer = (cPlayer*)pTarget;
			if( pPlayer->GetState() == eOBJECT_STATE_DIE )
				return false;

			/// 적인지 여부 판단
			if( DUELMAN->IsDuelEnemy( pPlayer ) != true &&
				PVPMAN->IsEnableAttack( pPlayer ) != true )
				return false;
		}
		break;
	case eOBJECTTYPE_MONSTER:
		{
			cMonster* pMon = (cMonster*)pTarget;
			if( pMon->GetState() == eOBJECT_STATE_DIE )
				return false;
		}
		break;
	default:
		return false;
	}

	/// 기본 공격 인덱스 산정 - 무기, 공격 인덱스
	ChargeOn( skillIdx, 0 );

	/// 공격 거리 체크 - 후처리기
	HERO->AfterAttack( pTarget );

	ResetAttackFlag();

	mAttackTarget.index = pTarget->GetObjectID();
	mAttackTarget.type = pTarget->GetObjectType();
	return true;
}

bool cSkillExecuter::ExecuteSkill_TransMon( unsigned long skillIdx, cBaseObject* pTarget )
{
	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();
	if( monsterClassIdx == 0 )
		return false;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return false;

	/// 사용 가능 여부 체크
	if( IsPossibleUseSkill( skillIdx, 0 ) == false )
		return false;

	/// 
	bool targetCheck = true;
	if( pTarget == 0 )
		pTarget = HERO->GetTargetObject();

	if( pInfo->mBoundType == eBOUNDTYPE_FIELD )
	{
		/// field base skill
		FieldSkillOn_TransMon( monsterClassIdx, skillIdx );
		return true;
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_AUTOFIELD )
	{
		return ExecuteAutoPos_TransMon( skillIdx );
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_SELF_NOTARGET )
	{
		/// active
		targetCheck = false;
	}
	else if( pInfo->mBoundType == eBOUNDTYPE_NONE )
	{
		if( pInfo->mApplyType == eAPPLYTYPE_SELF      ||
			pInfo->mApplyType == eAPPLYTYPE_BUFF_SELF ||
			pInfo->mApplyType == eAPPLYTYPE_DEBUFF_SELF )
			pTarget = HERO;

		targetCheck = true;
	}

	if( targetCheck )
	{
		/// 적용 타겟이 아닌 경우 charge
		if( IsPossibleUseSkillTarget( skillIdx, pTarget) == false )
		{		
			ChargeOn( skillIdx, 0 );
			return false;
		}

		sAfterAction* p = HERO->GetCurrentAfterAction();
		if( p->mActionType == eAfterAction_UseTargetSkill &&
			p->mExData1 == skillIdx &&
			p->mTarget.index == pTarget->GetObjectID() )
			return false;
	}

	/// 스킬 발동
	ChargeOn( skillIdx, 0 );
	HERO->AfterUseSkill( pTarget );

	if( pTarget )
		mAttackTarget = *(pTarget->GetTargetInfo());

	return true;
}

bool cSkillExecuter::ExecuteSkill_TransMon( unsigned long skillIdx, NiPoint3 applyPos )
{
	HERO->SetWaitMoveFlag( eWAITMOVE_NONE );

	ChargeOff( mChargeSkill.mSkillIdx, mChargeSkill.mSkillStep );

#ifdef _GMTOOL
	if( CHEATMAN->GetHideMode() == true )
		return false;
#endif

	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL )
		return false;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
			return false;
	}

	/// 서버로부터 무기교체완료 메세지 받기전까지 스킬 사용 불가
	if( HERO->IsWeaponChanging() == true )
		return false;

	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();
	if( monsterClassIdx == 0 )
		return false;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return false;

	/// 사용 가능 여부 체크
	if( IsPossibleUseSkill( skillIdx, 0 ) == false )
		return false;

	if( pInfo->mBoundType != eBOUNDTYPE_FIELD )
		return false;

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_UseFieldSkill &&
		p->mExData1 == skillIdx )
		return false;

	ChargeOn( skillIdx, 0 );
	HERO->AfterUseSkill( applyPos );

	return true;
}

bool cSkillExecuter::ExecuteAutoPos_TransMon( unsigned long skillIdx )
{
	unsigned long monsterClassIdx = HERO->GetTransMonsterClassIdx();
	if( monsterClassIdx == 0 )
		return false;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return false;

	if( pInfo->mBoundType != eBOUNDTYPE_AUTOFIELD )
		return false;

	mChargeSkill.mSkillRange = HERO->CalcStatusSkillRange( pInfo->mTargetDist, pInfo->mRangeType, skillIdx < eMONSTERATTACK_SKILL1 );
	NiPoint3 heroPos = HERO->GetPos();

	NiPoint3 applyPos = heroPos + HERO->GetDesireDir() * mChargeSkill.mSkillRange;

	/// 좌표가 이동 불가 지역이면 재계산
	unsigned long divideCnt = 10;
	float divideRange = mChargeSkill.mSkillRange / (float)divideCnt;
	cRangeCheck rangecheck( mChargeSkill.mSkillRange );
	for( unsigned long i = 0; i <= divideCnt ; ++i )
	{
		NiPoint3 divPos = heroPos + ( HERO->GetDesireDir() * ( mChargeSkill.mSkillRange - ( divideRange * i ) ) );
		WORLDMAN->CalcHeight( &divPos.z, divPos.x, divPos.y );

		if( rangecheck.IsRange( heroPos, divPos ) == true )
		{
			applyPos = divPos;
			break;
		}
	}

	WORLDMAN->EnableRayGet( heroPos, applyPos );
	WORLDMAN->CalcHeight( &applyPos.z, applyPos.x, applyPos.y );

	sAfterAction* p = HERO->GetCurrentAfterAction();
	if( p->mActionType == eAfterAction_UseFieldSkill && p->mExData1 == skillIdx )
		return false;

	ChargeOn( skillIdx, 0 );
	HERO->AfterUseSkill( applyPos );

	return true;
}


void cSkillExecuter::FieldSkillOn( unsigned long skillIdx, unsigned char skillStep )
{
	if( HERO->IsTransformMonster() == true )
		return;

	if( IsPossibleUseSkill( skillIdx, skillStep ) == false )
		return;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	if( SKILLMAN->IsUsedKeepSkill( skillIdx, skillStep ) == false )
		return;

	if( pInfo->mStepCount <= skillStep )
	{
		assert(0);
		return;
	}
	sPlayerSkillStepInfo* pStepInfo = &pInfo->mpSetpInfoArray[skillStep];

	if( pInfo->mType == eSKILLTYPE_TOTEM )
	{
		sTotemScript* totemInfo = TOTEMSCRIPT->GetTotemInfo( pStepInfo->mInfulenceIdx );
		if( totemInfo == 0 )
		{
			assert(0);
			HERO->SetFieldEffRadius( (float)pStepInfo->mBoundDist1 );
		}
		else
		{
			HERO->SetFieldEffRadius( (float)totemInfo->mApplyRange1 );
		}
	}
	else
		HERO->SetFieldEffRadius( (float)pStepInfo->mBoundDist1 );

	ChargeOn( skillIdx, skillStep );
	mChargeSkill.mChargeType = eCharge_Field;

	::SetCursor(NULL);
}

void cSkillExecuter::FieldSkillOn_TransMon( unsigned long monsterClassIdx, unsigned long skillIdx )
{
	if( HERO->IsTransformMonster() == false )
		return;

	if( IsPossibleUseSkill( skillIdx, 0 ) == false )
		return;

	sMonsterSkillScript* pInfo = SKILLSCRIPT->GetMonsterSkillInfo( monsterClassIdx, (eMONSTERATTACK_TYPE)skillIdx );
	if( pInfo == 0 )
		return;

	HERO->SetFieldEffRadius( (float)pInfo->mBoundDist1 );

	ChargeOn( skillIdx, 0 );
	mChargeSkill.mChargeType = eCharge_Field;
}

bool cSkillExecuter::IsPossibleApplyValueType( unsigned long skillIdx, cBaseObject* pTarget )
{
	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return false;
	}

	/// 특정 효과가 걸려있는 대상으로 스킬을 사용했는지 체크
	if( pInfo->mApplyValueType != eSTATUSPLUS_NONE )
	{
		switch( pInfo->mApplyType )
		{
		case eAPPLYTYPE_BUFF_SELF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				return HERO->IsHaveApplyBuffType( pInfo->mApplyValueType );
			}
			break;
		case  eAPPLYTYPE_DEBUFF_SELF:
			{
				/// 보유중인 효과에서 해당 효과가 있는지 체크
				return HERO->IsHaveApplyDeBuffType( pInfo->mApplyValueType );
			}
			break;
		case  eAPPLYTYPE_BUFF_ENEMY:
			{
				/// 타겟 조건 체크
				if( pTarget )
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					return pTarget->IsHaveApplyBuffType( pInfo->mApplyValueType );
				}
				else
					return false;
			}
			break;
		case eAPPLYTYPE_DEBUFF_ENEMY:
			{
				/// 타겟 조건 체크
				if( pTarget )
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					return pTarget->IsHaveApplyDeBuffType( pInfo->mApplyValueType );
				}
				else
					return false;
			}
			break;
		case eAPPLYTYPE_BUFF_PARTY:
			{
				if( pTarget )
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					return pTarget->IsHaveApplyBuffType( pInfo->mApplyValueType );
				}
				else
					return false;
			}
			break;
		case eAPPLYTYPE_DEBUFF_PARTY:
			{
				if( pTarget )
				{
					/// 보유중인 효과에서 해당 효과가 있는지 체크
					return pTarget->IsHaveApplyDeBuffType( pInfo->mApplyValueType );
				}
				else
					return false;
			}
			break;
		default:
			assert(0);
			return false;

		}//end switch( pSkillInfo->mApplyType )
	}
	
	return true;
}
