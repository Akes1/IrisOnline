#include "gamesrv.h"
#include "stdafx.h"
#include "Totem.h"
#include "Totem_Common.h"
#include "TotemScript.h"
#include "ObjectManager.h"
#include "AIManager.h"
#include "DuelManager.h"
#include "SkillManager.h"
#include "DeathMatchObject.h"
#include "PVPManager.h"
#include "Monster.h"
#include "Party.h"
#include "PartyManager.h"
#include "GridManager.h"
#include "PartyUnion.h"
#include "PartyUnionManager.h"
#include "StageScript.h"

cTotem::cTotem( void ) : cBaseObject( eOBJECTTYPE_TOTEM )
{
	/// 스크립트 정보
	mPlayerTargetSet.Clear();
	mMonsterTargetSet.Clear();
	mpTotemScript = NULL;
	mpInfScript = NULL;
	mAttacker.index = 0;
	mAttacker.type = eOBJECTTYPE_NONE;
	mPartyIdx = 0;
	mPartyUnionIdx = 0;
	mNextCheckTime = 0;
	mEndTime = 0;
	mApplytype = eAPPLYTYPE_SELF;
	mAttriButeType = eATTRIBUTETYPE_NONE;
	mDirection = 0;
	mTeamType = ePVPDM_TEAMTYPE_MAX;
	mSkillIdx = 0;
	mTargetCnt = 0;

	mTargetCntMax = 255;
}


cTotem::~cTotem( void )
{
}			


void* cTotem::operator new( size_t n )
{
	if( n != sizeof(cTotem) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocTotem();
}


void cTotem::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cTotem) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreeTotem( static_cast<cTotem*>(ptr) );

	return;
}


bool cTotem::TotemInit( sObject attacker, unsigned long objectIdx, unsigned long classIdx, unsigned long partyIdx, unsigned long partyUnionIdx, 
					   NiPoint2 pos, eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType, unsigned long skillIdx, float direction, unsigned char applyCnt )
{
	/// 토템 스크립트
	mpTotemScript = TOTEMSCRIPT->GetTotemInfo( classIdx );
	if( mpTotemScript == NULL )
		return false;

	/// 적용 효과 스크립트
	mpInfScript = SKILLSCRIPT->GetInfluenceInfo( mpTotemScript->mInfluenceIdx );
	if( mpInfScript == NULL )
	{
		NETWORK2->PostServerEvent("cTotem::TotemInit mpInfScript[%d] != NULL", mpTotemScript->mInfluenceIdx );
		return false;
	}

	cBaseObject* pObject = OBJECTMANAGER->GetObject( attacker.type, attacker.index );
	if( pObject == NULL )
		return false;

	mMapNumber = pObject->GetMapNumber();
	mTeamType = ePVPDM_TEAMTYPE_MAX;
	mObjectPos = pos;
	mObject.index = objectIdx;
	mAttacker = attacker;
	mApplytype = applyType;
	mAttriButeType = attributeType;
	mNextCheckTime = NETWORK2->GetAccumTime() + mpTotemScript->mApplyTime;
	mEndTime = NETWORK2->GetAccumTime() + mpTotemScript->mContinuanceTime;
	mPartyIdx = partyIdx;
	mPartyUnionIdx = partyUnionIdx;
	mSkillIdx = skillIdx;
	mDirection = direction;
	mTargetCntMax = applyCnt;
	if( applyCnt == 0 )
		mTargetCntMax = 255;
	if( applyCnt > 255 )
		mTargetCntMax = 255;

	if( AIMANAGER->IsPossible( GetMapNumber(), pos.x, pos.y, GetObject() ) == false )
		return false;

	return true;
}


bool cTotem::PvPTotemInit( ePVPDM_TEAM_TYPE teamType, unsigned long objectIdx, unsigned long classIdx,
						  unsigned short pvpIdx, NiPoint2 pos, eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType )
{
	/// 토템 스크립트
	mpTotemScript = TOTEMSCRIPT->GetTotemInfo( classIdx );
	if( mpTotemScript == NULL )
		return false;

	/// 적용 효과 스크립트
	mpInfScript = SKILLSCRIPT->GetInfluenceInfo( mpTotemScript->mInfluenceIdx );
	if( mpInfScript == NULL )
	{
		NETWORK2->PostServerEvent("cTotem::PvPTotemInit mpInfScript[%d] != NULL", mpTotemScript->mInfluenceIdx );
		return false;
	}

	mMapNumber = pvpIdx;		/// pvp 방번호
	mTeamType = teamType;
	mObjectPos = pos;
	mObject.index = objectIdx;
	mAttacker = mObject;
	mApplytype = applyType;
	mAttriButeType = attributeType;
	mNextCheckTime = NETWORK2->GetAccumTime() + mpTotemScript->mApplyTime;
	mEndTime = NETWORK2->GetAccumTime();
	sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
	if( pInfo != NULL )
		mEndTime = NETWORK2->GetAccumTime() + ( pInfo->mTotalDMTime * MINUTE );
	mPartyIdx = 0;
	mPartyUnionIdx = 0;

	return true;
}


void cTotem::ProcessTotem( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	/// 소멸시점과 적용시점이 일치하면 적용까지 하고 끝낸다.
	if( CheckTotem( accumTime ) == false && mNextCheckTime < accumTime )
		return;

	/// 적용 주기 체크
	if( mNextCheckTime >= accumTime )
		return;

	/// 적용 주기 셋팅
	if( mpTotemScript->mApplyTime < 100 )
		mNextCheckTime = mNextCheckTime + 100;		/// 적용 주기를 너무 빈번하게 하지 않게함
	else
		mNextCheckTime = mNextCheckTime + mpTotemScript->mApplyTime;

	/// 적용...
	mTargetCnt = 0; /// 한프레임 타겟 선정 갯수 초기화

	switch( mTeamType )
	{
	case ePVPDM_TEAMTYPE_A:
	case ePVPDM_TEAMTYPE_B:
		PvPAddInf( mTeamType, mApplytype );
		break;
	case ePVPDM_TEAMTYPE_MAX:	/// 플레이어/몬스터가 사용한 일반 토템
		{
			/// 범위안에 들어온 타겟을 목록에 추가
			if( mAttacker.type == eOBJECTTYPE_PLAYER )
				PlayerSelectTarget();
			else if( mAttacker.type == eOBJECTTYPE_MONSTER )
				MonsterSelectTarget();
		}
		break;
	}

}


void cTotem::PlayerSelectTarget()
{
	/// 오라 타입 체크
	bool addInfAura = false;
	if( mpInfScript != NULL && mpInfScript->mContinuanceTime == -1 )
		addInfAura = true;

	mTargetCnt = 0;

	/// 공통 처리
	switch( mApplytype )
	{
	case eAPPLYTYPE_SELF:
	case eAPPLYTYPE_BUDDY:
	case eAPPLYTYPE_PARTY:
		{
			/// 공격자가 보유 목록에 이미 들어 있는지 체크
			if( mPlayerTargetSet.Find( mAttacker.index ) != mPlayerTargetSet.End() )
				break;

			if( addInfAura == true )
			{
				cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttacker.index );
				if( pAttacker != NULL )
					CopyTotemInfluence( pAttacker );	/// 오라 타입 효과 생성
			}
			else
			{
				cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( mAttacker.index );
				if( pAttacker != NULL )
				{
					///// 거리 체크
					//NiPoint3 childPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pAttacker->GetPos() ) == true )
					//	SKILLMANAGER->AddInfluenceTotem( mAttacker, mAttacker, mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
					//	mpTotemScript->mContinuanceTime );

					if( RangeCheck( pAttacker ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, mAttacker, mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}

				}
			}
		}
		break;
	}

	/// 개별 처리
	switch( mApplytype )
	{
	case eAPPLYTYPE_ENEMY:
		{
			/// 몬스터 타겟
			mObjectIdxAry.Clear();
			GRIDMANAGER->FindMonsters( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cMonster* pMonster = OBJECTMANAGER->GetMonster( mObjectIdxAry[i] );
				if( pMonster == NULL || pMonster->GetStateDie() == true )
					continue;

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mMonsterTargetSet.Find( pMonster->GetObjectID() ) != mMonsterTargetSet.End() )
					continue;

				if( addInfAura == true )
				{
					/// 오라 타입 효과 생성
					if( CopyTotemInfluence( pMonster ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pMonster->GetPos() ) == true )
					//{
					//	/// 일반 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pMonster->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
					//		mpTotemScript->mContinuanceTime ) == true )
					//		++mTargetCnt;
					//}

					if( RangeCheck( pMonster ) == true )
					{
						pMonster->AddTakeDamage( mAttacker, 0, 0, eTAKEDAMAGETYPE_DAMAGE );
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pMonster->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 리턴
				if( mTargetCnt == mTargetCntMax )
					return;
			}


			/// 플레이어 타겟
			mObjectIdxAry.Clear();
			GRIDMANAGER->FindPlayers( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
				if( pPlayer == NULL || pPlayer->GetStateDie() == true )
					continue;

				if( pPlayer->GetObjectID() == mAttacker.index )
					continue;		/// 자신 통과

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mPlayerTargetSet.Find( pPlayer->GetObjectID() ) != mPlayerTargetSet.End() )
					continue;

				/// 1:1 공격대상 체크
				if( pPlayer->IsAttack( mAttacker.index ) == false )
					continue;

				if( addInfAura == true )
				{
					/// 오라 타입 효과 생성
					if( CopyTotemInfluence( pPlayer ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					//{
					//	/// 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
					//		mpTotemScript->mContinuanceTime ) == true )
					//		++mTargetCnt;
					//}
					if( RangeCheck( pPlayer ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 리턴
				if( mTargetCnt == mTargetCntMax )
					return;
			}

			mObjectIdxAry.Clear();
		}
		break;
	case eAPPLYTYPE_BUDDY:
		{
			/// 플레이어 타겟
			mObjectIdxAry.Clear();
			GRIDMANAGER->FindPlayers( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
				if( pPlayer == NULL || pPlayer->GetStateDie() == true )
					continue;

				if( pPlayer->GetObjectID() == mAttacker.index )
					continue;		/// 자신 통과

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mPlayerTargetSet.Find( pPlayer->GetObjectID() ) != mPlayerTargetSet.End() )
					continue;

				/// 1:1 공격대상 체크
				if( pPlayer->IsAttack( mAttacker.index ) == true )
					continue;

				if( addInfAura == true )
				{
					/// 오라 타입 효과 생성
					if( CopyTotemInfluence( pPlayer ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					//{
					//	/// 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
					//		mpTotemScript->mContinuanceTime ) != 0 )
					//		++mTargetCnt;
					//}
					if( RangeCheck( pPlayer ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,	mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 리턴
				if( mTargetCnt == mTargetCntMax )
					return;
			}

			mObjectIdxAry.Clear();
		}
		break;
	case eAPPLYTYPE_DIEBUDDY:
		{
			/// 플레이어 타겟
			mObjectIdxAry.Clear();
			GRIDMANAGER->FindPlayers( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
				if( pPlayer == NULL || pPlayer->GetStateDie() == true )
					continue;

				if( pPlayer->GetObjectID() == mAttacker.index )
					continue;		/// 자신 통과

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mPlayerTargetSet.Find( pPlayer->GetObjectID() ) != mPlayerTargetSet.End() )
					continue;

				/// 죽지 않은 유저 제외
				if( pPlayer->GetStateDie() == false )
					continue;

				/// 1:1 공격대상 체크
				if( pPlayer->IsAttack( mAttacker.index ) == true )
					continue;

				if( addInfAura == true )
				{
					/// 오라 타입 효과 생성
					if( CopyTotemInfluence( pPlayer ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					//{
					//	/// 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
					//		mpTotemScript->mContinuanceTime ) != 0 )
					//		++mTargetCnt;
					//}

					if( RangeCheck( pPlayer ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,	mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 리턴
				if( mTargetCnt == mTargetCntMax )
					return;
			}

			mObjectIdxAry.Clear();
		}
		break;
	case eAPPLYTYPE_PARTY:
		{
			/// 파티가 없으면 공격자에게만 적용
			cParty* pParty = PARTYMAN->GetParty( mPartyIdx );
			if( pParty != NULL )
			{
				/// 파티 목록 비교
				unsigned long* userArr = pParty->GetUserArr();
				unsigned int partyCount = pParty->GetCount();

				for( unsigned int i = 0 ; i < partyCount ; ++i )
				{
					cPlayer* pPlayerTarget = GRIDMANAGER->GetPlayer( userArr[i] );
					if( pPlayerTarget != NULL )
					{
						if( pPlayerTarget->GetObjectID() == mAttacker.index || pPlayerTarget->GetStateDie() == true )
							continue;		/// 자신 통과

						/// 타겟 보유 목록에 이미 들어 있는지 체크
						if( mPlayerTargetSet.Find( pPlayerTarget->GetObjectID() ) != mPlayerTargetSet.End() )
							continue;

						if( addInfAura == true )
						{
							/// 오라 타입 효과 생성
							if( CopyTotemInfluence( pPlayerTarget ) == true )
								++mTargetCnt;
						}
						else
						{
							/// 거리 체크
							NiPoint3 childPos( pPlayerTarget->GetXPos(), pPlayerTarget->GetYPos(), pPlayerTarget->Height() );
							NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
							mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
							if( mRangeCheck.IsRange( mObjectPos, pPlayerTarget->GetPos() ) == true )
							{
								///// 효과 생성
								//if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayerTarget->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
								//	mpTotemScript->mContinuanceTime ) != 0 )
								//	++mTargetCnt;

								if( RangeCheck( pPlayerTarget ) == true )
								{
									SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayerTarget->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
									++mTargetCnt;
								}
							}
						}
					}
				}
			}
			else
			{
				// 파티 연합 체크
				cPartyUnion* partyUnion = PARTYUNIONMAN->GetUnion( mPartyUnionIdx );
				if( partyUnion != NULL )
				{
					unsigned long* userArr = partyUnion->GetUserArr();

					for( unsigned int i = 0 ; i < MAX_PARTYUSER ; ++i )
					{
						cPlayer* pPlayerTarget = GRIDMANAGER->GetPlayer( userArr[i] );
						if( pPlayerTarget != NULL )
						{
							if( pPlayerTarget->GetObjectID() == mAttacker.index || pPlayerTarget->GetStateDie() == true )
								continue;		/// 자신 통과

							/// 타겟 보유 목록에 이미 들어 있는지 체크
							if( mPlayerTargetSet.Find( pPlayerTarget->GetObjectID() ) != mPlayerTargetSet.End() )
								continue;

							if( addInfAura == true )
							{
								/// 오라 타입 효과 생성
								if( CopyTotemInfluence( pPlayerTarget ) == true )
									++mTargetCnt;
							}
							else
							{
								///// 거리 체크
								//NiPoint3 childPos( pPlayerTarget->GetXPos(), pPlayerTarget->GetYPos(), pPlayerTarget->Height() );
								//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
								//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
								//if( mRangeCheck.IsRange( mObjectPos, pPlayerTarget->GetPos() ) == true )
								//{
								//	/// 효과 생성
								//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayerTarget->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true,
								//		mpTotemScript->mContinuanceTime ) != 0 )
								//		++mTargetCnt;
								//}

								if( RangeCheck( pPlayerTarget ) == true )
								{
									SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayerTarget->GetObject(), mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
									++mTargetCnt;
								}
							}
						}
					}
				}
			}
		}
		break;
	}
}


void cTotem::MonsterSelectTarget()
{
	/// 오라 타입 체크
	bool addInfAura = false;
	if( mpInfScript != NULL && mpInfScript->mContinuanceTime == -1 )
		addInfAura = true;

	mTargetCnt = 0;

	/// 공통 처리
	switch( mApplytype )
	{
	case eAPPLYTYPE_SELF:
	case eAPPLYTYPE_BUDDY:
	case eAPPLYTYPE_PARTY:
		{
			/// 공격자가 보유 목록에 이미 들어 있는지 체크
			if( mMonsterTargetSet.Find( mAttacker.index ) != mMonsterTargetSet.End() )
				break;

			if( addInfAura == true )
			{
				cMonster* pAttacker = OBJECTMANAGER->GetMonster( mAttacker.index );
				if( pAttacker != NULL )
					CopyTotemInfluence( pAttacker );	/// 오라 타입 효과 생성
			}
			else
			{
				cMonster* pAttacker = OBJECTMANAGER->GetMonster( mAttacker.index );
				if( pAttacker != NULL )
				{
					///// 거리 체크
					//NiPoint3 childPos( pAttacker->GetXPos(), pAttacker->GetYPos(), pAttacker->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pAttacker->GetPos() ) == true )
					//	SKILLMANAGER->AddInfluenceTotem( mAttacker, mAttacker, mpTotemScript->mInfluenceIdx, 0, 0, true,
					//	mpTotemScript->mContinuanceTime );

					if( RangeCheck( pAttacker ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, mAttacker, mpTotemScript->mInfluenceIdx, mSkillIdx, 0, true, mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}
			}
		}
		break;
	}

	/// 개별 처리
	switch( mApplytype )
	{
	case eAPPLYTYPE_BUDDY:
		{
			/// 몬스터 타겟
			mObjectIdxAry.Clear();
			GRIDMANAGER->FindMonsters( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cMonster* pMonster = OBJECTMANAGER->GetMonster( mObjectIdxAry[i] );
				if( pMonster == NULL || pMonster->GetStateDie() == true )
					continue;

				if( pMonster->GetObjectID() == mAttacker.index )
					continue;		/// 자신 통과

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mMonsterTargetSet.Find( pMonster->GetObjectID() ) != mMonsterTargetSet.End() )
					continue;

				if( addInfAura == true )
				{
					/// 오라 타입 효과 생성
					if( CopyTotemInfluence( pMonster ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pMonster->GetXPos(), pMonster->GetYPos(), pMonster->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pMonster->GetPos() ) == true )
					//{
					//	/// 일반 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pMonster->GetObject(), mpTotemScript->mInfluenceIdx, 0, 0, true, mpTotemScript->mContinuanceTime ) == true )
					//		++mTargetCnt;
					//}

					if( RangeCheck( pMonster ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pMonster->GetObject(), mpTotemScript->mInfluenceIdx, 0, 0, true, mpTotemScript->mContinuanceTime );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 리턴
				if( mTargetCnt == mTargetCntMax )
					return;
			}

			mObjectIdxAry.Clear();
		}
		break;
	case eAPPLYTYPE_ENEMY:
		{
			/// 풀에서 메모리 할당
			PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
			if( perIoContext == NULL )
			{
				NETWORK2->PostServerEvent("cTotem::MonsterSelectTarget perIoContext == NULL" );
				return;
			}

			MSG_SYN_INFLUENCEARY_CREATE* pInfMsg = (MSG_SYN_INFLUENCEARY_CREATE*)perIoContext->buffer;
			pInfMsg->Category = NM_SKILL;
			pInfMsg->Protocol = NM_SKILL_INFLUENCE_CREATELIST_SYN;
			pInfMsg->mTargetCnt = 0;
			sInfluenceCreate* pInf = pInfMsg->mTable;

			mObjectIdxAry.Clear();
			GRIDMANAGER->FindPlayers( this, &mObjectIdxAry );
			for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
				if( pPlayer == NULL || pPlayer->GetStateDie() == true )
					continue;

				/// 타겟 보유 목록에 이미 들어 있는지 체크
				if( mPlayerTargetSet.Find( pPlayer->GetObjectType() ) != mPlayerTargetSet.End() )
					continue;

				if( addInfAura == true )
				{
					/// 효과 생성
					if( CopyTotemInfluence( pPlayer, &pInf[mTargetCnt] ) == true )
						++mTargetCnt;
				}
				else
				{
					///// 거리 체크
					//NiPoint3 childPos( pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->Height() );
					//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
					//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
					//if( mRangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					//{
					//	/// 효과 생성
					//	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, 0, 0, true,
					//		mpTotemScript->mContinuanceTime, pInf ) == true )
					//		++mTargetCnt;
					//}

					if( RangeCheck( pPlayer ) == true )
					{
						SKILLMANAGER->AddInfluenceTotem( mAttacker, pPlayer->GetObject(), mpTotemScript->mInfluenceIdx, 0, 0, true, mpTotemScript->mContinuanceTime, pInf );
						++mTargetCnt;
					}
				}

				/// 타겟 수가 꽉차면 더 선정하지 않음
				if( mTargetCnt == mTargetCntMax )
					break;
			}

			pInfMsg->mTargetCnt = mTargetCnt;

			/// 패킷 검사
			if( pInfMsg->mTargetCnt > 50 || pInfMsg->GetMsgLength() > 14000 )
			{
				NETWORK2->PostServerEvent("cTotem::MonsterSelectTarget pInfMsg->mTargetCnt(%d) > 50, pInfMsg->GetMsgLength(%d) > 14000", 
					pInfMsg->mTargetCnt, pInfMsg->GetMsgLength() );

				perIoContext->offset = pInfMsg->GetMsgLength();
				NETWORK2->ReleaseIoContext( perIoContext );

				return;
			}

			/// 메세지 발송명단 플레이어들에게 발송
			if( pInfMsg->mTargetCnt > 0 )
			{
				NETWORK2->QuickSend( this, (char*)pInfMsg, pInfMsg->GetMsgLength() );
			}

			perIoContext->offset = pInfMsg->GetMsgLength();

			NETWORK2->ReleaseIoContext( perIoContext );

			mObjectIdxAry.Clear();
		}
		break;
	}
}


bool cTotem::RangeCheck( cBaseObject* pTarget )
{
	if( mpTotemScript == NULL )
		return false;

	if( pTarget == NULL )
		return false;

	if( mpTotemScript->mApplyRange2 == 0 )
	{
		/// 거리 체크
		NiPoint3 childPos( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height() );
		NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
		mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );

		return mRangeCheck.IsRange( parentPos, childPos );
	}
	else
	{
		NiPoint2 centerPos;
		centerPos.x = mObjectPos.x;
		centerPos.y = mObjectPos.y;

		float cos = cosf( mDirection );
		float sin = sinf( mDirection );

		float rangeX = mpTotemScript->mApplyRange1;
		float rangeY = mpTotemScript->mApplyRange2;

		/// 사각범위 위치 계산
		NiPoint2 rectPos[4];
		rectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
		rectPos[0].y = ( ( +rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
		rectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
		rectPos[1].y = ( ( -rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
		rectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
		rectPos[2].y = ( ( -rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
		rectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
		rectPos[3].y = ( ( +rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );

		rectPos[0] = rectPos[0] + centerPos;
		rectPos[1] = rectPos[1] + centerPos;
		rectPos[2] = rectPos[2] + centerPos;
		rectPos[3] = rectPos[3] + centerPos;

		NiPoint2 targetPoint;
		targetPoint.x = pTarget->GetXPos();
		targetPoint.y = pTarget->GetYPos();

		if( STAGESCRIPT->InsidePolygon( rectPos, 4, targetPoint ) == 0 )
			return true;
	}

	return false;
}


void cTotem::PvPAddInf( ePVPDM_TEAM_TYPE mainTeamType, eAPPLYTYPE applyType )
{
	mTargetCnt = 0;

	ePVPDM_TEAM_TYPE applyTeamType = mainTeamType;
	if( mainTeamType == ePVPDM_TEAMTYPE_A )
	{
		if( applyType == eAPPLYTYPE_BUDDY )
			applyTeamType = ePVPDM_TEAMTYPE_A;
		else if( applyType == eAPPLYTYPE_ENEMY )
			applyTeamType = ePVPDM_TEAMTYPE_B;
		else
			return;
	}
	else if( mainTeamType == ePVPDM_TEAMTYPE_B )
	{
		if( applyType == eAPPLYTYPE_BUDDY )
			applyTeamType = ePVPDM_TEAMTYPE_B;
		else if( applyType == eAPPLYTYPE_ENEMY )
			applyTeamType = ePVPDM_TEAMTYPE_A;
		else
			return;
	}

	cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( mMapNumber );
	if( pDM == NULL )
		return;

	/// 풀에서 메모리 할당
	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	if( perIoContext == NULL )
	{
		NETWORK2->PostServerEvent("cTotem::ATeamSelectTarget perIoContext == NULL" );
		return;
	}

	MSG_SYN_INFLUENCEARY_CREATE* pInfMsg = (MSG_SYN_INFLUENCEARY_CREATE*)perIoContext->buffer;
	pInfMsg->Category = NM_SKILL;
	pInfMsg->Protocol = NM_SKILL_INFLUENCE_CREATELIST_SYN;
	pInfMsg->mTargetCnt = 0;
	sInfluenceCreate* pInf = pInfMsg->mTable;

	mObjectIdxAry.Clear();
	GRIDMANAGER->FindPlayers( this, &mObjectIdxAry );
	for( unsigned long i = 0 ; i < mObjectIdxAry.GetSize() ; ++i )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mObjectIdxAry[i] );
		if( pPlayer == NULL || pPlayer->GetStateDie() == true )
			continue;

		/// 팀없으면 GM 효과 안받음
		if( pPlayer->GetPvPDMTeam() == ePVPDM_TEAMTYPE_MAX )
			continue;

		if( pPlayer->GetStateDie() == true || pPlayer->GetPvPDMTeam() != (unsigned long)applyTeamType )
			continue;

		/// 타겟 보유 목록에 이미 들어 있는지 체크
		if( mPlayerTargetSet.Find( pPlayer->GetObjectID() ) != mPlayerTargetSet.End() )
			continue;

		/// 오라 타입 효과 생성
		if( CopyTotemInfluence( pPlayer, &pInf[mTargetCnt] ) == true )
			++mTargetCnt;

		/// 타겟 수가 꽉차면 리턴
		if( mTargetCnt == mTargetCntMax )
			break;
	}

	pInfMsg->mTargetCnt = mTargetCnt;

	/// 패킷 검사
	if( pInfMsg->mTargetCnt > 50 || pInfMsg->GetMsgLength() > 14000 )
	{
		NETWORK2->PostServerEvent("cTotem::PvPAddInf[%d,%d] pInfMsg->mTargetCnt(%d) > 50, pInfMsg->GetMsgLength(%d) > 14000", 
			mainTeamType,applyType, pInfMsg->mTargetCnt, pInfMsg->GetMsgLength() );

		perIoContext->offset = pInfMsg->GetMsgLength();
		NETWORK2->ReleaseIoContext( perIoContext );

		return;
	}

	/// 메세지 발송명단 플레이어들에게 발송
	if( pInfMsg->mTargetCnt > 0 )
	{
		NETWORK2->QuickSend( this, (char*)pInfMsg, pInfMsg->GetMsgLength() );
	}

	perIoContext->offset = pInfMsg->GetMsgLength();

	NETWORK2->ReleaseIoContext( perIoContext );

	mObjectIdxAry.Clear();
}


bool cTotem::CheckTotem( unsigned long accumTime )
{
	/// 시간 확인
	if( mEndTime <= accumTime )
	{
		OBJECTMANAGER->InsertDeleteTotem( mObject.index );
		return false;
	}

	/// 공격자와 종속 관계 처리
	cBaseObject* pAttacker = NULL;
	if( mpTotemScript->mDependent == true )
	{
		/// 종속 적일 경우 공격자가 없으면 제거
		pAttacker = OBJECTMANAGER->GetObject( mAttacker.type, mAttacker.index );
		if( pAttacker == NULL || pAttacker->GetStateDie() == true )
		{
			OBJECTMANAGER->InsertDeleteTotem( mObject.index );
			return false;
		}

		/// 거리 밖으로 나간 경우 제거
		mRangeCheck.SetRadius( mpTotemScript->mAttackerRange );
		if( mRangeCheck.IsNotRange( pAttacker->GetPos(), mObjectPos ) == true )
		{
			OBJECTMANAGER->InsertDeleteTotem( mObject.index );
			return false;
		}
	}

	return true;
}


// SendSightIn Method
bool cTotem::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE          handle  = NULL;
	MSG_TOTEM_INFO* sendMsg = (MSG_TOTEM_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mObjectIdx         = mObject.index;
		sendMsg->mPosX              = mObjectPos.x;
		sendMsg->mPosY              = mObjectPos.y;
		sendMsg->mDirection			= mDirection;
		sendMsg->mCreateInfClassIdx = GetClassIdx();

		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_TOTEM_INFO) );
	}
	return false;
}

// SendSightOut Method
bool cTotem::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE        handle  = NULL;
	MSG_TOTEMIDX* sendMsg = (MSG_TOTEMIDX*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mObjectIdx = mObject.index;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_TOTEMIDX) );
	}
	return false;
}


bool cTotem::CopyTotemInfluence( cBaseObject* pTarget, sInfluenceCreate* pInf )
{
	if( mpTotemScript == NULL )
		return false;

	/// 오라의 효과가 범위거리가 없으면 안된다.
	if( mpTotemScript->mApplyRange1 == 0.0f )
	{
		NETWORK2->PostServerEvent("cSkillManager::CopyAuraInfluence auraRange == 0.0f" );
		return false;
	}

	if( pTarget == NULL )
		return false;

	if( RangeCheck( pTarget ) == false )
		return false;

	///// 거리 체크
	//NiPoint3 childPos( pTarget->GetXPos(), pTarget->GetYPos(), pTarget->Height() );
	//NiPoint3 parentPos( mObjectPos.x, mObjectPos.y, Height() );
	//mRangeCheck.SetRadius( mpTotemScript->mApplyRange1 );
	//if( mRangeCheck.IsNotRange( mObjectPos, pTarget->GetPos() ) == true )
	//	return false;

	/// 효과 생성
	unsigned long skillIdx = 0;
	if( mAttacker.type == eOBJECTTYPE_PLAYER )
		skillIdx = mSkillIdx;

	if( SKILLMANAGER->AddInfluenceTotem( mAttacker, pTarget->GetObject(), mpTotemScript->mInfluenceIdx, skillIdx, mObject.index, true, 
		mpTotemScript->mContinuanceTime, pInf ) == true )
	{
		if( pTarget->GetObjectType() == eOBJECTTYPE_PLAYER )
			mPlayerTargetSet.Insert( pTarget->GetObjectID() );
		else if( pTarget->GetObjectType() == eOBJECTTYPE_MONSTER )
		{
			if( mAttacker.type != eOBJECTTYPE_MONSTER )
				pTarget->AddTakeDamage( mAttacker, 0, 0, eTAKEDAMAGETYPE_DAMAGE );
			mMonsterTargetSet.Insert( pTarget->GetObjectID() );
		}

		return true;
	}

	return false;
}


void cTotem::EraseTarget( sObject target )
{
	if( target.type == eOBJECTTYPE_MONSTER )
		mMonsterTargetSet.Erase( target.index );
	else if( target.type == eOBJECTTYPE_PLAYER )
		mPlayerTargetSet.Erase( target.index );
}
