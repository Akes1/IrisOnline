#include "gamesrv.h"

#include "stdafx.h"
#include "Monster.h"
#include "Totem.h"

#include "../Common/Protocol.h"
#include "Status_Common.h"

#include "ObjectManager.h"
#include "SkillManager.h"
#include "AIManager.h"
#include "MonsterScript.h"
#include "DamageCalc.h"
#include "Player.h"
#include "StatusCalc_Server.h"
#include "drop.h"
#include "PathFinder.h"
#include "mode.h"
#include "Party.h"
#include "PartyManager.h"
#include "GridManager.h"
#include "ThemeObject.h"
#include "ThemeManager.h"
#include "RandomTable.h"
#include "Gathering.h"
#include "BaseDeathMatch.h"
#include "DeathMatchObject.h"
#include "PVPManager.h"

extern bool InfLeftTimeSort( unsigned long arg1, unsigned long arg2 );
extern bool InfChangeMPMinSort( unsigned long arg1, unsigned long arg2 );

int DistressCompare( const void *arg1, const void *arg2 )
{
	PerTakeDamage* per1 = (PerTakeDamage*)arg1;
	PerTakeDamage* per2 = (PerTakeDamage*)arg2;

	// 내림차순 정렬
	return per1->distressPoint > per2->distressPoint;
}

cMonster::cMonster( void )
: cBaseObject( eOBJECTTYPE_MONSTER )
, mPathFinder( 0 )
, mPathCount( 0 )
, mPathIndex( 0 )
, mpAction( NULL )
, mMoveStopRange( 0 )
, mGroupRegen( false )
, mMoveOverTime( 0 )
, mAttackType( eMONSTERATTACK_MAX )
, mTalkDealyEndTime( 0 )
, mTalkState( eMONSTERTALK_MAX )
, mDestroyTime( 0 )
, mFollowImportance( 0 )
, mPatternPos( 0 )
, mPatternDir( 1 )
, mPatternWait( 0 )
, mServerPosTime( 0 )
, mAutoDummy( 0 )
{
	mModeSkillIdx = eMONSTERATTACK_MAX;
	mModeInitSkillIdx = eMONSTERATTACK_MAX;
	mTFixedPlayerIdx = 0;
	mTFixedInfIdx = 0;
	mReservationComeback = false;
	mIdleEndTime = 0;
	mLastUsedSkillIdx = eMONSTERATTACK_MAX;
	mDeathMonster = false;
	mDMTeamType = ePVPDM_TEAMTYPE_MAX;
	mGroupMoveReady = 1;
}

cMonster::~cMonster( void )
{
	mpAction = NULL;
}			



void* cMonster::operator new( size_t n )
{
	if( n != sizeof(cMonster) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocMonster();
}



void cMonster::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cMonster) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreeMonster( static_cast<cMonster*>(ptr) );

	return;
}



bool cMonster::Init( unsigned long uniqueidx, unsigned long classIdx, unsigned long regenIdx, unsigned short mapNumber, unsigned char modeNum )
{
	/// 초기화
	memset( &mStatusPlus, 0 , sizeof(mStatusPlus) );
	memset( &mStatusPer, 0 , sizeof(mStatusPer) );
	memset( mODDITY, 0, sizeof(mODDITY) );
	mTakeDamageRoot.pool = NULL;
	mTakeDamageRoot.root = NULL;
	mIsParentLink = false;
	mParentMonsterIdx = 0;
	mNextTargetChangeTime = 0;
	mFollowEndMove = false;
	mFirstAttackCharacter = 0;
	mLastAttacker = 0;
	mFirstAttackParty = 0;
	mIsFirstMove = true;
	mCriDie = false;
	mOutsideDestroy = false;
	mSkillWaitEndTime = 0;
	mPathCount = 0;
	mPathIndex = 0;
	SetTarget( eOBJECTTYPE_NONE, 0 );
	SetMoveStopRange( 0 );
	mPlayerFollowPos = FOLLOW_POS_MAX;
	mpRegenMonsterInfo = NULL;
	mpMovePattern = NULL;
	ActionChange( eACT_MOVE );
	mOutsideAddMonster = false;
	mDestroyTime = 0;
	mAutoDummy = 0;

	/// 모드
	mIsCombat = false;
	mTotalTime = 0;
	mModeTime = 0;
	mModeAgent = MONSTERSCRIPT->GetMonsterModeAgent( classIdx );
	mCurrentMode = 0;
	mIdleEndTime = 0;
	mReservationComeback = false;
	mFixedObjectSizePer = 0;

	mLastUsedSkillIdx = eMONSTERATTACK_MAX;

	mGuardAry.Clear();
	mChangeMPDamageAry.Clear();
	mDeleteTypeDamagedAry.Clear();

	mDeathMonster = false;

	mGroupMoveReady = 1;

	/// 
	ClearImmune();

	mSkillCoolTimeMap.Clear();
	mEliteCoolTimeMap.Clear();
	mGenTotemCoolTimeMap.Clear();
	mGenMonCoolTimeMap.Clear();
	mGenGatheringCoolTimeMap.Clear();

	if( mSummonListMap.IsEmpty() == false )
	{
		cPHashMap::cIterator b = mSummonListMap.Begin();
		cPHashMap::cIterator end = mSummonListMap.End();
		for( ; b!=end; b++ )
		{
			cHashSet* pSet = (cHashSet*)(*b).mSecond;
			SAFE_DELETE( pSet );
		}
		mSummonListMap.Clear();
	}

	mSummonGatherAry.Clear();

	/// 설정값 셋팅
	mObject.index = uniqueidx;


	mSkillLastAttack = mObject;
	mSkillLastKind = eDAMAGE_MISS;

	/// 몬스터 스크립트
	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( classIdx );
	if( !pMonsterScript )
	{
		NETWORK2->PostServerEvent("Monster::Init pMonsterScript[%d]", classIdx );
		return false;
	}
	mpMonsterInfo = pMonsterScript;
	mMoveSpeed = (float)mpMonsterInfo->mActionWalkSpd;
	mHP = mpMonsterInfo->mMaxHp;
	mMP = mpMonsterInfo->mMaxMp;
	mFollowImportance =  mpMonsterInfo->mImportance;
	SetFixedObjectSizePer( mFixedObjectSizePer );

	if( mpMonsterInfo->mMonsterScale <= 0.0f )
	{
		NETWORK2->PostServerEvent("cMonsterScript MonsterScale ERROR - %d", mpMonsterInfo->mMonsterScale );
		return false;
	}
	mFixedObjectSize = mpMonsterInfo->mMonsterFixSize * mpMonsterInfo->mMonsterScale; 

	cArray* pMonsterSkillAry = (cArray*)SKILLSCRIPT->GetMonsterBaseSkillArr( mpMonsterInfo->mMonsterClassIdx );
	if( pMonsterSkillAry == NULL )
	{
		NETWORK2->PostServerEvent("Monster::Init pMonsterSkillAry[%d]", mpMonsterInfo->mMonsterClassIdx );
		return false;
	}

	sMonsterSkillScript* pMonsterSkill;
	for( unsigned long i = 0 ; i< pMonsterSkillAry->GetSize() ; ++i )
	{
		pMonsterSkill = (sMonsterSkillScript*)(*pMonsterSkillAry)[i];
		if( pMonsterSkill == NULL )
		{
			NETWORK2->PostServerEvent("Monster::Init pMonsterSkill == NULL" );
			continue;
		}

		mSkillCoolTimeMap.Insert( pMonsterSkill->mAttackType, 0 );
	}


	/// 리젠 정보 - 있는 경우만 셋팅 - 없는 경우 소환 몬스터
	sRegenScript* pRegenScript = MONSTERSCRIPT->GetRegenMonsterInfo( modeNum, regenIdx );
	if( pRegenScript )
	{
		mpRegenMonsterInfo = pRegenScript;

		SetMapNumber( mpRegenMonsterInfo->mMapNumber );
		mDestroyTime = pRegenScript->mRegenLifeTime;

		sRegenGroupScript* pRegenGroupScript = MONSTERSCRIPT->GetRegenGroupInfo( modeNum, pRegenScript->mGroupNumber );
		if( pRegenGroupScript == NULL )
		{
			NETWORK2->PostServerEvent("cActionRegen::Action pRegenGroupScript == NULL");
			return false;
		}

		float regenX = mpRegenMonsterInfo->mRegenX;
		float regenY = mpRegenMonsterInfo->mRegenY;
		unsigned long regenRange = pRegenGroupScript->mRegenRange;

		/// 동족이 있는 몬스터는 비중을 무시한다.
		if( pRegenScript->mFamilyNum != 0 )
			mFollowImportance = 0;

		/// 이동 패턴 데이터 참조
		mPatternPos = 0;
		mPatternDir = 1;
		mpMovePattern = MONSTERSCRIPT->GetMonsterPattern( modeNum, pRegenScript->mPatternIdx );
		if( mpMovePattern != NULL )
		{
			AIMANAGER->FamilyNextPattern( mapNumber, pRegenScript->mFamilyNum, &mPatternPos, &mPatternDir );
			mGroupMoveReady = 2;

			if( (long)mpMovePattern->mPosAry.GetSize() > mPatternPos )
			{
				sMonsterPatternPos* pPos = (sMonsterPatternPos*)mpMovePattern->mPosAry[mPatternPos];
				if( pPos != NULL )
				{
					regenX = pPos->mPosX;
					regenY = pPos->mPosY;
					regenRange = pPos->mRange;
				}
			}
		}

		bool isRandPos = false;
		/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
		if( regenRange > MAP_RANGE_CHECK_RING )
		{
			for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
			{
				NiPoint2 pos;

				/// 최대 범위에서 20%씩 줄여가면서 체크
				int range = (int)(regenRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
				if( range <= 0 ) 
					continue;

				pos.x = regenX + ( rand() % range ) - ( rand() % range );
				pos.y = regenY + ( rand() % range ) - ( rand() % range );

				if( AIMANAGER->IsPossible( mMapNumber, pos.x, pos.y, mObject ) == true )
				{
					regenX = pos.x;
					regenY = pos.y;
					isRandPos = true;
					break;
				}
			}
		}

		if( isRandPos == false )
		{
			if( AIMANAGER->IsPossible( mMapNumber,  mpRegenMonsterInfo->mRegenX, mpRegenMonsterInfo->mRegenY, mObject ) == false )
			{
				return false;
				Verbose->WriteLog("MonsterInit PositionError[ Map:%d Group:%d X:%.0f Y:%.0f ]",
					mMapNumber, pRegenScript->mGroupNumber, pRegenScript->mRegenX, pRegenScript->mRegenY );
				//return false;
			}

			regenX = mpRegenMonsterInfo->mRegenX;
			regenY = mpRegenMonsterInfo->mRegenY;
		}

		mRegenPos.x = regenX;
		mRegenPos.y = regenY;
		mObjectPos = mRegenPos;

		mDirection = pRegenScript->mDirection;		/// 방향값
		if( pRegenScript->mDirection >= 1000.0f )		/// 1000 인경우 랜덤 방향값
			mDirection = S_ToRadian( ( rand() % 359 ) - 179 );		/// -179 ~ +179 사이 값을 랜덤하게 결정
	}

	mLastPos = mRegenPos;
	if( mapNumber != 0 )
		SetMapNumber( mapNumber );


	return true;
}


void cMonster::SetSummonInfo( cMonster* pParent, unsigned short mapNumber, float regenX, float regenY, 
							 unsigned long regenMin, unsigned long regenMax, unsigned long destroyTime, bool isRand, bool isAutuDummy )
{
	SetMapNumber( mapNumber );
	mObjectPos.x = regenX;
	mObjectPos.y = regenY;

	if( isRand == true )
		mObjectPos = CalcMinMaxRandPos( regenX, regenY, regenMin, regenMax );

	mRegenPos.x = mObjectPos.x;
	mRegenPos.y = mObjectPos.y;

	mDirection = S_ToRadian( ( rand() % 358 ) - 179 );		/// 방향값 -179 ~ +179 사이 값을 랜덤하게 결정

	mOutsideAddMonster = true;

	mDestroyTime = destroyTime;

	mAutoDummy = isAutuDummy;
	if( mAutoDummy == 1 )
	{
		// 오토 더미인경우
		mHP = GetMaxHP();
	}

	/// 그리드 추가한다.
	if( GRIDMANAGER->AddMonster( this ) == false )
	{
		OBJECTMANAGER->RemoveMonster( GetObjectID() );
		NETWORK2->PostServerEvent("cAIManager::BossSummonMonRegen GRIDMANAGER->AddMonster( pMonster ) == false" );
		return;
	}

	SendRegen();

	if( pParent != NULL )
	{
		mIsParentLink = true;
		mParentMonsterIdx = pParent->GetObjectID();
		mFollowImportance = 0;

		/// 보스를 공격하던 공격자 목록을 자식에 넣음
		TakeDamageRoot* pRoot = pParent->GetTakeDamageRoot();
		if( pRoot != NULL )
		{
			unsigned long randSelect = 0;
			if( pRoot->count != 0 )
				randSelect = rand() % pRoot->count;

			unsigned long cnt = 0;
			PerTakeDamage* per = (PerTakeDamage*)pRoot->pool;
			while( per )
			{
				sObject player = { eOBJECTTYPE_PLAYER, per->playerIdx };
				if( cnt == randSelect )
					AddTakeDamage( player, 0, 2000, eTAKEDAMAGETYPE_ADDPLAYER );
				else
					AddTakeDamage( player, 0, 0, eTAKEDAMAGETYPE_ADDPLAYER );
				per = (PerTakeDamage*)per->next;
				++cnt;
			}
		}

		ActionChange( eACT_FOLLOW );
	}
}



void cMonster::Release( bool parentLinkRelase )
{
	if( parentLinkRelase == true )
	{
		if( mIsParentLink == true )
		{
			cMonster* pParent = OBJECTMANAGER->GetMonster( mParentMonsterIdx );
			if( pParent != NULL )
				pParent->DieSummonMonster( mObject.index, mObjectPos );

			pParent = OBJECTMANAGER->GetDeathMonster( mParentMonsterIdx );
			if( pParent != NULL )
				pParent->DieSummonMonster( mObject.index, mObjectPos );
		}
	}

	SetTarget( eOBJECTTYPE_NONE, 0 );
	ClearTakeDamage();

	ReleaseInfluenceSet( false );

	ClearImmune();

	mGenTotemCoolTimeMap.Clear();
	mGenMonCoolTimeMap.Clear();
	mGenGatheringCoolTimeMap.Clear();

	if( mSummonListMap.IsEmpty() == false )
	{
		cPHashMap::cIterator b = mSummonListMap.Begin();
		cPHashMap::cIterator end = mSummonListMap.End();
		for( ; b!=end; b++ )
		{
			cHashSet* pSet = (cHashSet*)(*b).mSecond;
			SAFE_DELETE( pSet );
		}
		mSummonListMap.Clear();
	}

	for( unsigned long i = 0 ; i < mSummonGatherAry.GetSize() ; ++i )
	{
		cGathering* pGather = OBJECTMANAGER->GetGathering( mSummonGatherAry[i] );
		if( pGather != NULL )
			pGather->GatheringDie( 0 );
	}
	mSummonGatherAry.Clear();

	PerNode* node = mSummonPatternRoot.pool;
	while ( node != NULL )
	{
		sSummonPatternInfo* pInfo = (sSummonPatternInfo*)node;

		node = node->next;
		SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
	}
}


cBaseObject* cMonster::GetTarget( )
{
	/// 타겟 종류에 따라
	switch( mTarget.type )
	{
	case eOBJECTTYPE_PLAYER:
		/// 플레이어 포인터 반환
		return (cBaseObject*)NearGridFindPlayer( mTarget.index, true );
	case eOBJECTTYPE_MONSTER:
		return GRIDMANAGER->GetMonster( mTarget.index );
	default:
		/// 없으면 NULL포인터 반환
		return NULL;
	}
}


bool cMonster::SetTarget( unsigned char kind, unsigned long idx )
{
	if( kind == mTarget.type && idx == mTarget.index )
		return true;

	/// 추적 퍼짐 위치 저장
	unsigned char oldPlayerFollowPos = mPlayerFollowPos;
	unsigned char newPlayerFollowPos = FOLLOW_POS_MAX;

	bool isNewFight = false;
	bool isFightEnd = false;

	if( kind == eOBJECTTYPE_PLAYER )
	{
		//pPlayer = NearGridFindPlayer( idx );
		/// ※ 객체가 있지만 그리드 범위 밖으로 나갔을 수 있으니 objectmanager에서 찾아온다.
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( idx );
		if( pPlayer == NULL || pPlayer->GetStateDie() == true )
			return false;

		/// 새로운 타겟에 몬스터 비중 계산해 가능하다면 설정
		if( pPlayer->MonsterImportancePlus( mFollowImportance ) == false )
			return false;		/// 기존 타겟 유지

		/// 신규 타겟에 맞는 추적 퍼짐 위치 선정
		newPlayerFollowPos = pPlayer->SelectFollowPos( oldPlayerFollowPos );

		/// 추적 시작 위치, 시간 설정
		//		mFollowStartPos = mObjectPos;
		mFollowEndTime  = NETWORK2->GetAccumTime() + mpMonsterInfo->mFollowTime;

		/// 타겟이 없다가 생김
		if( GetTarget() == NULL )
			isNewFight = true;
	}

	mPlayerFollowPos = newPlayerFollowPos;

	if( mTarget.type == eOBJECTTYPE_PLAYER )
	{
		/// 기존타겟에 몬스터 비중을 감소
		/// ※ 객체가 있지만 그리드 범위 밖으로 나갔을 수 있으니 objectmanager에서 찾아온다.
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mTarget.index );
		if( pPlayer != NULL && pPlayer->GetStateDie() == false )
		{
			pPlayer->MonsterImportanceMinus( mFollowImportance );

			/// 기존 타겟의 추적 퍼짐 위치 반환
			pPlayer->UnSelectFollowPos( oldPlayerFollowPos );
			newPlayerFollowPos = FOLLOW_POS_MAX;
		}

		/// 타겟이 있는데 해제함
		if( kind == eOBJECTTYPE_NONE )
		{
			mAttackType = eMONSTERATTACK_MAX;
			/// 죽지 않은 경우만 추적 포기 대사 처리
			if( pPlayer != NULL && pPlayer->GetStateDie() == false )
				isFightEnd = true;
		}
	}

	/// 내가 죽는 경우를 제외하고는 추적을 포기하는 경우다
	/// ※타겟 해제하기전에 대사 발송
	if( isFightEnd == true && mHP != 0 )
		SendSpeech( eMONSTERTALK_GIVEUP );

	cBaseObject::SetTarget( kind, idx );

	/// 타겟 설정 후에 대사 발송
	if( isNewFight == true )
		SendSpeech( eMONSTERTALK_FIGHT );	

	return true;
}


void cMonster::Die()
{
	unsigned long dropPlayerIdx = GetfirstAttack();

	if( NETWORK2->GetInDunMapNum() == MAP_DEATHMATCH )
	{
		cDeathMatchObject* pDM = (cDeathMatchObject*)PVPMANAGER->GetPvPDMObject( mMapNumber );
		if( pDM != NULL )
			pDM->DeleteStone( mDMTeamType, mLastAttacker );
	}
	else
	{
		if( mOutsideDestroy == false && mAutoDummy == 0 )
		{
			unsigned long partyIdx = 0;	
			cPlayer* player = NearGridFindPlayer( GetfirstAttack(), false );
			if( player )
			{
				partyIdx = player->GetPartyIndex();
				if( partyIdx > 0 )
				{
					/// 파티 전체 갱신
					PARTYMAN->SendToPartyHunt( this, player, GetRaceGender() );
				}
				else
				{
					/// 자기자신만 갱신
					player->UpdateDutyHunt( GetRaceGender(), player->GetObjectID() );
				}
			}

			/// 드랍 소유권 계산 - 선공 유저가 몬스터HP의 20%를 깎으면 선공유저 소유
			unsigned long damage = GetTakeDamage( dropPlayerIdx );
			if( (float)damage < GetMaxHP() * 0.2f )
			{
				/// 선공 유저가 20%를 깎지못하면 가장 데미지가 큰유저가 소유
				TakeDamageRoot* pTakeDamageRoot = GetTakeDamageRoot();
				if( pTakeDamageRoot != NULL )
				{
					PerTakeDamage* per = (PerTakeDamage*)pTakeDamageRoot->pool;
					while( per )
					{
						if( damage < per->takeDamage )
						{
							damage = per->takeDamage;
							dropPlayerIdx = per->playerIdx;
						}

						per = (PerTakeDamage*)per->next;
					}
				}
				else
					NETWORK2->PostServerEvent("cMonster::Drop pTakeDamageMap == NULL");
			}

			/// 드랍 처리
			DROP->MonsterDrop( GetObjectID(), dropPlayerIdx, partyIdx );
			DROP->MonsterQuestDrop( GetObjectID(), dropPlayerIdx, partyIdx );

			short type = -1;

			switch( GetRaceGender() )
			{
			case 10006: type = EVENT_RESERVED_THEME_01; break;
			case 15006: type = EVENT_RESERVED_THEME_01; break;
			case 10013: type = EVENT_RESERVED_THEME_02; break;
			case 15013: type = EVENT_RESERVED_THEME_02; break;
			case 10018: type = EVENT_RESERVED_THEME_03; break;
			}

			/// 인던 최종 보스죽인내용 로그에기록 - 예외처리
			if( type != -1 )
			{
				/// 테마서버에서 치트몬스터가 아닌 정상몬스터 잡은 경우
				if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ && IsCheatTempMonster() == false )
				{
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( dropPlayerIdx );
					if( pPlayer != NULL )
					{
						if( pPlayer->GetPartyIndex() == 0 )
						{
							NETWORK2->PostReservedEvent( type, pPlayer->GetObjectID(), pPlayer->GetConnectionIdx() );
						}
						else
						{
							cParty* pParty = PARTYMAN->GetParty( pPlayer->GetPartyIndex() );
							if( pParty != NULL )
							{
								unsigned long* partyPlayerIdx = pParty->GetUserArr();
								if( partyPlayerIdx != NULL )
								{
									for( unsigned long j = 0 ; j < pParty->GetCount() ; ++j )
									{
										cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( partyPlayerIdx[j] );
										if ( pPlayer != NULL )
											NETWORK2->PostReservedEvent( type, pPlayer->GetObjectID(), pPlayer->GetConnectionIdx() );
									}
								}
							}
						}
					}
				}
			}

			LinkObstruction();
			LinkTeleportPortal();
		}
	}

	Release();

	/// 리젠 대기 몬스터 목록에 넣음
	const sRegenScript* pScript = GetRegenMonsterInfo();
	if( pScript != NULL && IsCheatTempMonster() == false )
		AIMANAGER->InsertReadyMonster( GetObjectID(), pScript->mRegenIdx, pScript->mGroupNumber );
}

void cMonster::SendRegen()
{
	if( mDMTeamType == ePVPDM_TEAMTYPE_MAX )
	{
		/// 메세지 발송
		MSG_MONSTER_INFO Msg;
		Msg.Category = NM_MONSTER;
		Msg.Protocol = NM_MONSTER_REGEN_SYN;
		Msg.mMonsterInfo.mMonsterIdx = GetObjectID();
		Msg.mMonsterInfo.mMonsterClassIdx = GetRaceGender();
		Msg.mMonsterInfo.mPosX = GetXPos();
		Msg.mMonsterInfo.mPosY = GetYPos();
		Msg.mMonsterInfo.mDirection = GetDirection();
		Msg.mMonsterInfo.mHP = GetHP();
		Msg.mMonsterInfo.mMaxHP = GetMaxHP();
		Msg.mMonsterInfo.mMP = GetMP();
		Msg.mMonsterInfo.mMaxMP = GetMaxMP();
		Msg.mMonsterInfo.mScale = GetFixedObjectSizePer();
		Msg.mMonsterInfo.mAutoDummy = mAutoDummy;
		Msg.mCount = 0;
		NETWORK2->QuickSend( this, (char*)&Msg, Msg.GetMsgLength() );
	}
	else
	{
		/// 메세지 발송
		MSG_MONSTERTEAM_INFO Msg;
		Msg.Category = NM_MONSTER;
		Msg.Protocol = NM_MONSTER_TEAMREGEN_SYN;
		Msg.mMonsterInfo.mMonsterIdx = GetObjectID();
		Msg.mMonsterInfo.mMonsterClassIdx = GetRaceGender();
		Msg.mMonsterInfo.mPosX = GetXPos();
		Msg.mMonsterInfo.mPosY = GetYPos();
		Msg.mMonsterInfo.mDirection = GetDirection();
		Msg.mMonsterInfo.mHP = GetHP();
		Msg.mMonsterInfo.mMaxHP = GetMaxHP();
		Msg.mMonsterInfo.mMP = GetMP();
		Msg.mMonsterInfo.mMaxMP = GetMaxMP();
		Msg.mMonsterInfo.mScale = GetFixedObjectSizePer();
		Msg.mMonsterInfo.mAutoDummy = mAutoDummy;
		Msg.mCount = 0;
		Msg.mTeamType = mDMTeamType;
		NETWORK2->QuickSend( this, (char*)&Msg, Msg.GetMsgLength() );
	}
}


NiPoint2 cMonster::CalcMinMaxRandPos( float centerX, float centerY, unsigned long rangeMin, unsigned long rangeMax )
{
	NiPoint2 center;
	center.x = centerX;
	center.y = centerY;

	NiPoint2 dest;
	dest.x = centerX;
	dest.y = centerY;

	if( rangeMin >= rangeMax )
		return dest;

	unsigned long randRange = rand() % ( rangeMax - rangeMin );

	float isPositiveNumber = 1;
	if( rand() % 2 == 0 )
		isPositiveNumber = -1;

	float randDir = (float)(rand() % 180);
	randDir = randDir * isPositiveNumber;

	float radian = S_ToRadian( randDir );
	NiPoint2 dir = NiPoint2( -sinf(radian), -cosf(radian) );
	dir.Unitize();

	unsigned long divideRange = ( rangeMin + randRange ) / 10;
	for( unsigned long i = 0 ; i < 10 ; ++i )
	{
		dest = center + dir * (float)( ( rangeMin + randRange ) - ( i * divideRange ) );

		if( AIMANAGER->IsPossible( mMapNumber, dest.x, dest.y, mObject ) == true )
			return dest;
	}

	return center;
}


eACTION_CHANGE cMonster::Update( unsigned long elapsedTime, unsigned long accumTime )
{
	/// 일정시간 공격상태로 가지 않는경우 제거처리
	if( mIsParentLink == true )
	{
		/// 부모가 없는경우
		cMonster* pParent = OBJECTMANAGER->GetMonster( mParentMonsterIdx );
		if( pParent == 0 )
		{
			pParent = OBJECTMANAGER->GetDeathMonster( mParentMonsterIdx );
			if( pParent == 0 )
			{
				mIsParentLink = false;
				mParentMonsterIdx = 0;

				/// 삭제시간 설정
				mDestroyTime = 5000;
			}
		}
		else
		{
			if( pParent->GetDeathMonster() == false && ( pParent->GetStateDie() == true || pParent->IsCombat() == false ) )
			{
				/// 부모와의 연결정보 초기화
				mIsParentLink = false;
				mParentMonsterIdx = 0;

				/// 삭제시간 설정
				mDestroyTime = 5000;
			}
		}
	}

	mTotalTime += elapsedTime;
	if( mIsCombat == false )
	{
		if( mDestroyTime != 0 )
		{
			if( mDestroyTime <= elapsedTime )
			{
				mDestroyTime = 0;

				OutsideDestroy();
			}
			else
			{
				mDestroyTime -= elapsedTime;
			}
		}
	}
	else
	{
		/// Mode Process
		if( mModeAgent != 0 && mCurrentMode != 0 )
		{
			/// 속도 체크후 - 단위시간당 체크를 넣어보자!!
			UpdateMode( elapsedTime, accumTime );
		}
	}

	///
	SpeechProcess();

	ProcessTakeDamage( elapsedTime, accumTime );


	/// 소환물 패턴 처리
	//for( unsigned long i = mSummonPatternInfoAry.GetSize() ; i > 0 ; --i )
	//{
	//	sSummonPatternInfo* pInfo = (sSummonPatternInfo*)mSummonPatternInfoAry[i-1];
	//	if( pInfo == NULL )
	//	{
	//		delete( pInfo );
	//		mSummonPatternInfoAry.PopAt( i-1 );
	//		continue;
	//	}

	//	if( pInfo->mNextPatternTime > accumTime )
	//		continue;

	//	cArray* pPatterAry = MONSTERSCRIPT->GetSummonPattern( pInfo->mSummonPatternIdx );
	//	if( pPatterAry == NULL )
	//	{
	//		delete( pInfo );
	//		mSummonPatternInfoAry.PopAt( i-1 );
	//		continue;
	//	}

	//	sSummonPatternScript* pScript = (sSummonPatternScript*)(*pPatterAry)[pInfo->mNowPatternPos];
	//	if( pScript == NULL )
	//	{
	//		delete( pInfo );
	//		mSummonPatternInfoAry.PopAt( i-1 );
	//		continue;
	//	}
	//	
	//	for( unsigned long j = 0 ; j < pInfo->mCount ; ++j )
	//	{
	//		NiPoint2 centerPos;
	//		centerPos.x = pInfo->mCenterPos.x;
	//		centerPos.y = pInfo->mCenterPos.y;

	//		unsigned long genRangeMin = pInfo->mRangeMin;
	//		unsigned long genRangeMax = pInfo->mRangeMax;

	//		float radian = pInfo->mCenterDir;
	//		NiPoint2 dir = NiPoint2( -sinf(radian), -cosf(radian) );
	//		dir.Unitize();

	//		float randRange = (float)( genRangeMax - genRangeMin );
	//		if( randRange > 0 )
	//			randRange = (float)( ( rand() % (unsigned long)randRange ) + genRangeMin );
	//		NiPoint2 pos = centerPos + dir * randRange;

	//		/// 스크립트 방향으로 회전
	//		float direction = pScript->mDir;
	//		float cos = cosf( direction );
	//		float sin = sinf( direction );

	//		NiPoint2 changePos;
	//		changePos.x = ( ( pos.x - centerPos.x ) * cos + ( pos.y - centerPos.y ) * sin );
	//		changePos.y = ( ( pos.y - centerPos.y ) * cos - ( pos.x - centerPos.x ) * sin );

	//		changePos = changePos + centerPos;

	//		switch( pInfo->mObject.type )
	//		{
	//		case eOBJECTTYPE_MONSTER:
	//			{
	//				/// 몬스터 소환 & 다음 리젠시간 기록
	//				AIMANAGER->BossSummonMonRegen( this, mMapNumber, changePos.x, changePos.y, genRangeMin, genRangeMax,
	//					1, pInfo->mObject.index, false );
	//			}
	//			break;
	//		case eOBJECTTYPE_TOTEM:
	//			{
	//				cTotem* pTotem = OBJECTMANAGER->AddTotem( pInfo->mObject.index, 0, 0, changePos, mObject, eAPPLYTYPE_ENEMY, eATTRIBUTETYPE_PHYSICAL, 0, mDirection );
	//				if( pTotem != NULL )
	//					GRIDMANAGER->AddTotem( pTotem );
	//			}
	//			break;
	//		case eOBJECTTYPE_GATHERING:
	//			{
	//				cGathering* pGathering = OBJECTMANAGER->AddGathering( 0, pInfo->mObject.index, mMapNumber, changePos.x, changePos.y, mDirection, 0 );
	//				if( pGathering != NULL )
	//				{
	//					GRIDMANAGER->AddGathering( pGathering );
	//					pGathering->SendRegenSync();
	//					break;
	//				}
	//			}
	//			break;
	//		}
	//	}

	//	pInfo->mNowPatternPos = pInfo->mNowPatternPos + 1;
	//	if( pPatterAry->GetSize() <= pInfo->mNowPatternPos )
	//	{
	//		delete( pInfo );
	//		mSummonPatternInfoAry.PopAt( i-1 );
	//	}
	//	else
	//	{
	//		sSummonPatternScript* pNextScript = (sSummonPatternScript*)(*pPatterAry)[pInfo->mNowPatternPos];
	//		if( pNextScript != NULL )
	//			pInfo->mNextPatternTime = accumTime + pNextScript->mDelayTime;
	//		else
	//		{
	//			delete( pInfo );
	//			mSummonPatternInfoAry.PopAt( i-1 );
	//		}
	//	}
	//}



	PerNode* node = mSummonPatternRoot.pool;
	while ( node != NULL )
	{
		sSummonPatternInfo* pInfo = (sSummonPatternInfo*)node;
		if( pInfo == NULL )
		{
			node = node->next;
			SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
			continue;
		}

		if( pInfo->mNextPatternTime > accumTime )
		{
			node = node->next;
			continue;
		}

		cArray* pPatterAry = MONSTERSCRIPT->GetSummonPattern( pInfo->mSummonPatternIdx );
		if( pPatterAry == NULL )
		{
			node = node->next;
			SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
			continue;
		}

		sSummonPatternScript* pScript = (sSummonPatternScript*)(*pPatterAry)[pInfo->mNowPatternPos];
		if( pScript == NULL )
		{
			node = node->next;
			SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
			continue;
		}

		for( unsigned long j = 0 ; j < pInfo->mCount ; ++j )
		{
			NiPoint2 centerPos;
			centerPos.x = pInfo->mCenterPos.x;
			centerPos.y = pInfo->mCenterPos.y;

			unsigned long genRangeMin = pInfo->mRangeMin;
			unsigned long genRangeMax = pInfo->mRangeMax;

			float radian = pInfo->mCenterDir;
			NiPoint2 dir = NiPoint2( -sinf(radian), -cosf(radian) );
			dir.Unitize();

			float randRange = (float)( genRangeMax - genRangeMin );
			if( randRange > 0 )
				randRange = (float)( ( rand() % (unsigned long)randRange ) + genRangeMin );
			NiPoint2 pos = centerPos + dir * randRange;

			/// 스크립트 방향으로 회전
			float direction = pScript->mDir;
			float cos = cosf( direction );
			float sin = sinf( direction );

			NiPoint2 changePos;
			changePos.x = ( ( pos.x - centerPos.x ) * cos + ( pos.y - centerPos.y ) * sin );
			changePos.y = ( ( pos.y - centerPos.y ) * cos - ( pos.x - centerPos.x ) * sin );

			changePos = changePos + centerPos;

			switch( pInfo->mObject.type )
			{
			case eOBJECTTYPE_MONSTER:
				{
					/// 몬스터 소환 & 다음 리젠시간 기록
					AIMANAGER->BossSummonMonRegen( this, mMapNumber, changePos.x, changePos.y, genRangeMin, genRangeMax,
						1, pInfo->mObject.index, false );
				}
				break;
			case eOBJECTTYPE_TOTEM:
				{
					cTotem* pTotem = OBJECTMANAGER->AddTotem( pInfo->mObject.index, 0, 0, changePos, mObject, 
						(eAPPLYTYPE)pInfo->mApplyType, eATTRIBUTETYPE_PHYSICAL, 0, mDirection, 0 );
					if( pTotem != NULL )
					{
						GRIDMANAGER->AddTotem( pTotem );
					}
				}
				break;
			case eOBJECTTYPE_GATHERING:
				{
					cGathering* pGathering = OBJECTMANAGER->AddGathering( 0, pInfo->mObject.index, mMapNumber, changePos.x, changePos.y, mDirection, 0 );
					if( pGathering != NULL )
					{
						GRIDMANAGER->AddGathering( pGathering );
						pGathering->SendRegenSync();
						mSummonGatherAry.PushBack( pGathering->GetObjectID() );
					}
				}
				break;
			}
		}

		pInfo->mNowPatternPos = pInfo->mNowPatternPos + 1;
		if( pPatterAry->GetSize() <= pInfo->mNowPatternPos )
		{
			node = node->next;
			SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
			continue;
		}
		else
		{
			sSummonPatternScript* pNextScript = (sSummonPatternScript*)(*pPatterAry)[pInfo->mNowPatternPos];
			if( pNextScript != NULL )
				pInfo->mNextPatternTime = accumTime + pNextScript->mDelayTime;
			else
			{
				node = node->next;
				SUMMONPATTERNPOOL->ReleaseSummonPattern( &mSummonPatternRoot, pInfo );
				continue;
			}
		}
		node = node->next;
	}

	return mpAction->Action( this, elapsedTime, accumTime );
}

void cMonster::UpdateMode( unsigned long elapsedTime, unsigned long /*accumTime*/ )
{
	/// 모드 시간 증가
	mModeTime += elapsedTime;

	if( GetStateDie() == true )
		return;

	if( IsSkillWaitEnd() == false )
		return;

	if( mCurrentMode == NULL )
		return;

	/// 모드 전환 시점 체크
	cModeInfoArr* conArr = mCurrentMode->GetConditionModeInfoArr();
	if( conArr != NULL )
	{
		for( unsigned int i=0; i<conArr->GetSize(); i++ )
		{
			sModeInfoBase* p = (sModeInfoBase*)((*conArr)[i]);
			if( p == NULL )
				continue;

			if( p->type != eMODEINFO_MODECHANGE )
			{
				/// error
				assert(0);
				continue;
			}

			sModeChange* pInfo = (sModeChange*)p;
			bool modeChange = true;
			for( unsigned int i=0; i<2; i++ )
			{
				if( ConditionValueCheck( pInfo->mCondition[i].conType, pInfo->mCondition[i].conValue ) == false )
				{
					modeChange = false;
					break;
				}
			}

			if( modeChange == true )
			{
				ChangeCurrentMode( pInfo->mModeIdx );
				return;
			}
		}
	}

	/// 모드 지속처리 체크
	cModeInfoArr* processArr = mCurrentMode->GetProcessModeInfoArr();
	if( processArr != NULL )
	{
		for( unsigned int i=0; i<processArr->GetSize(); i++ )
		{
			/// 보유 소환물에 대한 시간 처리 문제 체크
			sModeInfoBase* p = (sModeInfoBase*)((*processArr)[i]);
			if( p == NULL )
				continue;

			switch( p->type )
			{
			case eMODEINFO_GENMONSTER:
				{
					sModeGenMonster* pMonMode = (sModeGenMonster*)p;
					if( pMonMode == NULL )
						return;

					/// 반복 여부 체크 - 반복 하는 것만 쿨타임이 들어 있음
					cHashMap::cIterator find = mGenMonCoolTimeMap.Find( pMonMode->mOrderNum );
					if( find != mGenMonCoolTimeMap.End() )
					{
						unsigned long cooltime = (*find).mSecond;
						if( cooltime < NETWORK2->GetAccumTime() )
						{
							(*find).mSecond = NETWORK2->GetAccumTime() + pMonMode->mDelayTime;

							/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
							bool pattern = false;
							if( pMonMode->mGenPatternIdx != 0 )
							{
								cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pMonMode->mGenPatternIdx );
								if( pAryScript != NULL && pAryScript->GetSize() > 0 )
								{
									sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
									if( pScript != NULL )
									{
										pattern = true;
										//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
										//pPattern->mObject.index = pMonMode->mGenMonsterIdx;
										//pPattern->mObject.type = eOBJECTTYPE_MONSTER;
										//pPattern->mCount = pMonMode->mCount;
										//pPattern->mSummonPatternIdx = pMonMode->mGenPatternIdx;
										//pPattern->mCenterPos = mObjectPos;
										//pPattern->mCenterDir = mDirection;
										//pPattern->mRangeMin = pMonMode->mGenRangeMin;
										//pPattern->mRangeMax = pMonMode->mGenRangeMax;
										//pPattern->mNowPatternPos = 0;
										//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

										//mSummonPatternInfoAry.PushBack( pPattern );

										sObject object = { eOBJECTTYPE_MONSTER, pMonMode->mGenMonsterIdx };
										SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pMonMode->mGenPatternIdx, pMonMode->mCount,
											mObjectPos, mDirection, pMonMode->mGenRangeMin, pMonMode->mGenRangeMax, 0, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );
									}
									else
										NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
								}
								else
									NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
							}

							/// 패턴이 없는경우 랜덤위치로 소환
							if( pattern == false )
							{
								/// 몬스터 소환 & 다음 리젠시간 기록
								AIMANAGER->BossSummonMonRegen( this, mMapNumber, mObjectPos.x, mObjectPos.y, pMonMode->mGenRangeMin, pMonMode->mGenRangeMax,
									pMonMode->mCount, pMonMode->mGenMonsterIdx );
							}							
						}
					}
				}
				break;
			case eMODEINFO_GENTOTEM:
				{
					sModeGenTotem* pTotemMode = (sModeGenTotem*)p;
					if( pTotemMode == NULL )
						return;

					cHashMap::cIterator find = mGenTotemCoolTimeMap.Find( pTotemMode->mOrderNum );
					if( find != mGenTotemCoolTimeMap.End() )
					{
						unsigned long coolTime = (*find).mSecond;
						if( coolTime < NETWORK2->GetAccumTime() )
						{
							(*find).mSecond = NETWORK2->GetAccumTime() + pTotemMode->mDelayTime;

							/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
							bool pattern = false;
							if( pTotemMode->mGenPatternIdx != 0 )
							{
								cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pTotemMode->mGenPatternIdx );
								if( pAryScript != NULL && pAryScript->GetSize() > 0 )
								{
									sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
									if( pScript != NULL )
									{
										pattern = true;
										//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
										//pPattern->mObject.index = pTotemMode->mGenTotemIdx;
										//pPattern->mObject.type = eOBJECTTYPE_TOTEM;
										//pPattern->mCount = pTotemMode->mCount;
										//pPattern->mSummonPatternIdx = pTotemMode->mGenPatternIdx;
										//pPattern->mCenterPos = mObjectPos;
										//pPattern->mCenterDir = mDirection;
										//pPattern->mRangeMin = pTotemMode->mGenRangeMin;
										//pPattern->mRangeMax = pTotemMode->mGenRangeMax;
										//pPattern->mNowPatternPos = 0;
										//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

										//mSummonPatternInfoAry.PushBack( pPattern );

										sObject object = { eOBJECTTYPE_TOTEM, pTotemMode->mGenTotemIdx };
										SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pTotemMode->mGenPatternIdx, pTotemMode->mCount,
											mObjectPos, mDirection, pTotemMode->mGenRangeMin, pTotemMode->mGenRangeMax, pTotemMode->mApplyType, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );

									}
									else
										NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
								}
								else
									NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
							}

							/// 패턴이 없는경우 랜덤위치로 소환
							if( pattern == false )
							{
								unsigned long genRangeMin = pTotemMode->mGenRangeMin;
								unsigned long genRangeMax = pTotemMode->mGenRangeMax;

								for( unsigned char i = 0 ; i < pTotemMode->mCount ; ++i )
								{
									NiPoint2 pos = CalcMinMaxRandPos( mObjectPos.x, mObjectPos.y, genRangeMin, genRangeMax );

									cTotem* pTotem = OBJECTMANAGER->AddTotem( pTotemMode->mGenTotemIdx, 0, 0, pos, mObject, 
										(eAPPLYTYPE)pTotemMode->mApplyType, eATTRIBUTETYPE_PHYSICAL, 0, mDirection, 0 );
									if( pTotem != NULL )
										GRIDMANAGER->AddTotem( pTotem );
								}
							}				

						}
					}
				}
				break;
			case eMODEINFO_GENGATHERING:
				{
					sModeGenGathering* pGatheringMode = (sModeGenGathering*)p;
					if( pGatheringMode == NULL )
						return;

					cHashMap::cIterator find = mGenGatheringCoolTimeMap.Find( pGatheringMode->mOrderNum );
					if( find != mGenGatheringCoolTimeMap.End() )
					{
						unsigned long coolTime = (*find).mSecond;
						if( coolTime < NETWORK2->GetAccumTime() )
						{
							(*find).mSecond = NETWORK2->GetAccumTime() + pGatheringMode->mDelayTime;

							/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
							bool pattern = false;
							if( pGatheringMode->mGenPatternIdx != 0 )
							{
								cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pGatheringMode->mGenPatternIdx );
								if( pAryScript != NULL && pAryScript->GetSize() > 0 )
								{
									sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
									if( pScript != NULL )
									{
										pattern = true;
										//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
										//pPattern->mObject.index = pGatheringMode->mGenGatheringIdx;
										//pPattern->mObject.type = eOBJECTTYPE_GATHERING;
										//pPattern->mCount = pGatheringMode->mCount;
										//pPattern->mSummonPatternIdx = pGatheringMode->mGenPatternIdx;
										//pPattern->mCenterPos = mObjectPos;
										//pPattern->mCenterDir = mDirection;
										//pPattern->mRangeMin = pGatheringMode->mGenRangeMin;
										//pPattern->mRangeMax = pGatheringMode->mGenRangeMax;
										//pPattern->mNowPatternPos = 0;
										//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

										//mSummonPatternInfoAry.PushBack( pPattern );

										sObject object = { eOBJECTTYPE_GATHERING, pGatheringMode->mGenGatheringIdx };
										SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pGatheringMode->mGenPatternIdx, pGatheringMode->mCount,
											mObjectPos, mDirection, pGatheringMode->mGenRangeMin, pGatheringMode->mGenRangeMax, 0, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );

									}
									else
										NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
								}
								else
									NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
							}

							/// 패턴이 없는경우 랜덤위치로 소환
							if( pattern == false )
							{
								unsigned long genRangeMin = pGatheringMode->mGenRangeMin;
								unsigned long genRangeMax = pGatheringMode->mGenRangeMax;

								for( unsigned char i = 0 ; i < pGatheringMode->mCount ; ++i )
								{
									NiPoint2 pos = CalcMinMaxRandPos( mObjectPos.x, mObjectPos.y, genRangeMin, genRangeMax );

									cGathering* pGathering = OBJECTMANAGER->AddGathering( 0, pGatheringMode->mGenGatheringIdx, mMapNumber, pos.x, pos.y, mDirection, 0 );
									if( pGathering != NULL )
									{
										GRIDMANAGER->AddGathering( pGathering );
										pGathering->SendRegenSync();
										mSummonGatherAry.PushBack( pGathering->GetObjectID() );
									}
								}
							}			
						}
					}
				}
				break;
			}
		}
	}

	/// 모드 사용스킬 체크
	if( GetActionID() == eACT_FOLLOW || GetActionID() == eACT_ATTACK )
	{
		if( mModeInitSkillIdx == eMONSTERATTACK_MAX && mModeSkillIdx == eMONSTERATTACK_MAX )
		{
			mTempSelectSkill.Clear();
			unsigned long totalRate = 0;

			cModeInfoArr* skillArr = mCurrentMode->GetCheckSkillModeInfoArr();
			for( unsigned int i=0; i<skillArr->GetSize(); i++ )
			{
				sModeInfoBase* p = (sModeInfoBase*)((*skillArr)[i]);
				if( p == NULL )
					continue;

				if( p->type != eMODEINFO_USESKILL )
				{
					/// error
					assert(0);
					continue;
				}

				sModeSkill* pInfo = (sModeSkill*)p;

				//if( pInfo->mModeDelayTime < mModeTime )
				//	continue;

				bool useSkill = true;
				for( unsigned int i=0; i<2; i++ )
				{
					if( ConditionValueCheck( pInfo->mCondition[i].conType, pInfo->mCondition[i].conValue ) == false )
					{
						useSkill = false;
						break;
					}
				}

				if( useSkill == false )
					continue;

				sMonsterSkillScript* pScript = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, (eMONSTERATTACK_TYPE)pInfo->mSkillIdx );
				if( pScript == NULL )
					continue;

				if( mMP < pScript->mUseMP )
					continue;

				if( IsCantSkill( pScript->mAttributeType ) == true )
					continue;

				cCoolTime::cIterator begin = mEliteCoolTimeMap.Find( pInfo->mSkillIdx );
				cCoolTime::cIterator end = mEliteCoolTimeMap.End();

				if( begin != end )
				{
					if( (*begin).mSecond > NETWORK2->GetAccumTime() )
						continue;
				}

				cBaseObject* pTarget = GetTarget();
				if( pTarget == NULL )
					continue;

				long useRate = CalcSkillRate( pTarget, (eMONSTERATTACK_TYPE)pInfo->mSkillIdx, (long)pInfo->mActionPer );
				totalRate += useRate;

				/// 조건에 만족하는 공격타입을 담는다.
				mTempSelectSkill.Insert( (eMONSTERATTACK_TYPE)pInfo->mSkillIdx, useRate );
			}

			if( totalRate != 0 )
			{
				/// 선택된 공격타입의 비율 전체 합계중 한지점을 정한다.
				unsigned long randSelect = rand() % totalRate + 1;

				/// 선택된 한지점의 공격타입값을 찾아 리턴한다.
				cHashMap::cIterator sI = mTempSelectSkill.Begin();
				cHashMap::cIterator sE = mTempSelectSkill.End();
				for( ; sI != sE ; ++sI )
				{
					unsigned long attackType = (*sI).mFirst;
					unsigned long rate = (*sI).mSecond;

					if( rate >= randSelect )
					{
						mModeSkillIdx = (eMONSTERATTACK_TYPE)attackType;
						break;
					}

					randSelect = randSelect - rate;
				}
			}
		}
	}
}


bool cMonster::DeathUpdate()
{
	bool end = true;
	if( mSummonListMap.GetSize() != 0 )
	{
		cPHashMap::cIterator b = mSummonListMap.Find( 0 );
		cPHashMap::cIterator e = mSummonListMap.End();

		if( b != e )
		{
			cHashSet* pIdxSet = (cHashSet*)(*b).mSecond;
			if( pIdxSet != NULL && pIdxSet->GetSize() != 0 )
				end = false;
		}
	}

	if( end == true )
	{
		OBJECTMANAGER->RecoverDeathMonster( this );
		Die();

		/// 그리드/오브젝트 메니저에서 제거
		GRIDMANAGER->RemoveMonster( this );
		OBJECTMANAGER->InsertDeleteMonster( mObject.index );
	}

	return end;
}


unsigned long cMonster::DamageCalc( sObject target, unsigned long skillIdx, unsigned long attributeType, unsigned long rangeType,
								   unsigned long skillDamage, unsigned long skillDamagePer,bool isIgnoreDefence, float addTotalDamage, float addPerTotalDamage )
{
	//return DAMAGECALC->MonsterDamageCalc( mObject.index, target, (eMONSTERATTACK_TYPE)skillIdx, (eATTRIBUTETYPE)attributeType, (eRANGETYPE)rangeType );

	unsigned long damage = 0;

	sMonsterSkillScript* pMonsterSkill = SKILLSCRIPT->GetMonsterSkillInfo( GetRaceGender(), (eMONSTERATTACK_TYPE)skillIdx );
	if( pMonsterSkill == NULL )
	{
		NETWORK2->PostServerEvent("cMonster::DamageCalc pMonsterSkill[%d,%d,%d,%d,%d] == NULL", 
			GetRaceGender(), mObject.index, skillIdx, attributeType, rangeType );
		return 0;
	}

	//// 오토더미 공격력
	//if( IsAutoDummy() == true )
	//	return 1;

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		unsigned long minAttack = 0;
		unsigned long maxAttack = 0;

		if( rangeType == eRANGETYPE_CLOSE )
		{
			minAttack = CalcPhysicAttack( pMonsterSkill->mMinAttackValue );
			maxAttack = CalcPhysicAttack( pMonsterSkill->mMaxAttackValue );
		}
		else if( rangeType == eRANGETYPE_LONG )
		{
			minAttack = CalcPhysicRangeAttack( pMonsterSkill->mMinAttackValue );
			maxAttack = CalcPhysicRangeAttack( pMonsterSkill->mMaxAttackValue );
		}
		else
		{
			NETWORK2->PostServerEvent("cMonster::DamageCalc rangeType[%d,%d,%d,%d,%d] ERROR", 
				GetRaceGender(), mObject.index, skillIdx, attributeType, rangeType );
		}

		/// 랜덤 공격력 결정
		unsigned long attackerStatusAttack = minAttack;
		if( minAttack < maxAttack )
		{
			attackerStatusAttack = maxAttack - minAttack;
			if( attackerStatusAttack > 0 )
				attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
			attackerStatusAttack = minAttack + attackerStatusAttack;
		}

		unsigned long targetStatusDef = 0;
		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::DamageCalc eATTRIBUTETYPE_PHYSICAL pPlayer == NULL" );
				return 0;
			}

			targetStatusDef = FloatToInt( pPlayer->GetStatus2()->mPhysicDefense );
			targetLv = pPlayer->GetLevel();
		}
		else
		{
			NETWORK2->PostServerEvent("cMonster::DamageCalc eATTRIBUTETYPE_PHYSICAL target.type[%d,%d,%d,%d,%d] ERROR", 
				GetRaceGender(), mObject.index, skillIdx, attributeType, rangeType );
			return 0;
		}

		/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
		unsigned long randTargetDef = 0;
		if( isIgnoreDefence == false )
			randTargetDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

		///// 랜덤 범위 제거 - 테스트용
		//attackerStatusAttack = ( maxAttack + minAttack ) / 2;
		//randTargetDef = targetStatusDef;

		damage = DAMAGECALC->PhyDamage500( eOBJECTTYPE_MONSTER, attackerLv, target.type, targetLv, attackerStatusAttack, 
			skillDamage, skillDamagePer, randTargetDef, addTotalDamage, addPerTotalDamage );
	}
	/// 마법 스킬 성공확률 계산 처리
	else if( attributeType == eATTRIBUTETYPE_MAGIC )
	{
		unsigned long minAttack = CalcMagicAttack( pMonsterSkill->mMinAttackValue );
		unsigned long maxAttack = CalcMagicAttack( pMonsterSkill->mMaxAttackValue );

		/// 랜덤 공격력 결정
		unsigned long attackerStatusAttack = minAttack;
		if( minAttack < maxAttack )
		{
			attackerStatusAttack = maxAttack - minAttack;
			if( attackerStatusAttack > 0 )
				attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
			attackerStatusAttack = minAttack + attackerStatusAttack;
		}

		unsigned long targetStatusDef = 0;
		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::DamageCalc eATTRIBUTETYPE_MAGIC pPlayer == NULL" );
				return 0;
			}

			targetStatusDef = FloatToInt( pPlayer->GetStatus2()->mMagicDefense );
			targetLv = pPlayer->GetLevel();
		}
		else
		{
			NETWORK2->PostServerEvent("cMonster::DamageCalc eATTRIBUTETYPE_MAGIC target.type[%d,%d,%d,%d,%d] ERROR", 
				GetRaceGender(), mObject.index, skillIdx, attributeType, rangeType );
			return 0;
		}

		/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
		unsigned long randTargetDef = 0;
		if( isIgnoreDefence == false )
			randTargetDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

		///// 랜덤 범위 제거 - 테스트용
		//attackerStatusAttack = ( maxAttack + minAttack ) / 2;
		//randTargetDef = targetStatusDef;

		damage = DAMAGECALC->MagDamage530( eOBJECTTYPE_MONSTER, attackerLv, target.type, targetLv, attackerStatusAttack, 
			skillDamage, skillDamagePer, randTargetDef, addTotalDamage, addPerTotalDamage );
	}
	else
	{
		NETWORK2->PostServerEvent("cDamageCalc::PlayerDamageCalc attributeType[%d,%d,%d,%d,%d,%d] ERROR", attributeType,
			GetRaceGender(), mObject.index, skillIdx, attributeType, rangeType );
		return 0;
	}

	return damage;
}


unsigned long cMonster::DotDamageCalc( unsigned char attackerType, unsigned char attackerLv, unsigned long attackMin, 
									  unsigned long attackMax, unsigned char dotType, unsigned long skillDamage, 
									  unsigned long skillDamagePer, unsigned long totalTime, unsigned long dotTime )
{
	/// 기본정보
	unsigned long damage = 0;
	unsigned char targetLv = GetLevel();


	switch( dotType )
	{
	case eDOTTYPE_PHYSICALCLOSE:
	case eDOTTYPE_PHYSICALRANGE:
		{
			/// 공격력 범위안에서 선정
			unsigned long attackerStatusAttack = attackMin;
			if( attackMin < attackMax )
			{
				attackerStatusAttack = attackMax - attackMin;
				if( attackerStatusAttack > 0 )
					attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
				attackerStatusAttack = attackMin + attackerStatusAttack;
			}

			///  방어력 산출
			unsigned long targetStatusDef = GetPhysicDefense();

			/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
			targetStatusDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

			damage = DAMAGECALC->PhyDotDamage600( attackerType, attackerLv, mObject.type, targetLv,
				attackerStatusAttack, skillDamage, skillDamagePer, targetStatusDef, totalTime, dotTime );
		}
		break;
	case eDOTTYPE_MAGIC:
		{
			/// 공격력 범위안에서 선정
			unsigned long attackerStatusAttack = attackMin;
			if( attackMin < attackMax )
			{
				/// 마법 공격력을 최대값에서 최소값 사이에 랜덤 값으로 설정
				attackerStatusAttack = attackMax - attackMin;
				if( attackerStatusAttack > 0 )
					attackerStatusAttack = rand() % ( attackerStatusAttack + 1 );
				attackerStatusAttack = attackMin + attackerStatusAttack;
			}

			/// 타겟의 방어력 산출
			targetLv = GetLevel();
			unsigned long targetStatusDef = GetMagicDefense();

			/// 방어력 DEFENSE_RANDOM_PER 값만큼 랜덤 선택
			targetStatusDef = targetStatusDef * ( DEFENSE_RANDOM_PER + ( rand() % ( 100 -  DEFENSE_RANDOM_PER + 1 ) ) ) / 100;

			damage = DAMAGECALC->MagDotDamage630( attackerType, attackerLv, mObject.type, targetLv,
				attackerStatusAttack, skillDamage, skillDamagePer, targetStatusDef, totalTime, dotTime );
		}
		break;
	default:
		NETWORK2->PostServerEvent("cDamageCalc::DotDamageCalc dotType[%d,%d,%d] ERROR", dotType,
			mObject.index, GetRaceGender() );
		return 0;
	}

	return damage;
}


unsigned long cMonster::ConditionOdd( sObject target, long influenceRate, unsigned long skillClassIdx )
{
	cBaseObject* pTarget = OBJECTMANAGER->GetObject( target.type, target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent( "cMonster::ConditionOdd pTarget == NULL[%d,%d,%d,%d]", target.type, target.index, influenceRate, skillClassIdx );
		return 0;
	}

	/// 몬스터인 경우 몬스터 스크립트 로드
	sMonsterSkillScript* pMonsterSkill = (sMonsterSkillScript*)SKILLSCRIPT->GetMonsterSkillInfo( GetRaceGender(), (eMONSTERATTACK_TYPE)skillClassIdx );
	if( pMonsterSkill == NULL )
	{
		NETWORK2->PostServerEvent( "cMonster::ConditionOdd pMonsterSkill == NULL[%d,%d,%d,%d]", target.type, target.index, influenceRate, skillClassIdx );
		return 0;
	}

	/// 공격자 레벨, 타겟 레벨
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = pTarget->GetLevel();

	unsigned long checkPer = 0;

	if( pMonsterSkill->mAttributeType == eATTRIBUTETYPE_PHYSICAL )
		checkPer = DAMAGECALC->PhyConditionOdd900( influenceRate, attackerLv, targetLv );
	else if( pMonsterSkill->mAttributeType == eATTRIBUTETYPE_MAGIC )
		checkPer = DAMAGECALC->MagConditionOdd930( influenceRate, attackerLv, targetLv );
	else
	{
		NETWORK2->PostServerEvent("cMonster::ConditionOdd pPlayerSkill->mAttributeType[%d][%d,%d,%d,%d] ERROR",
			pMonsterSkill->mAttributeType, target.type, target.index, influenceRate, skillClassIdx );
	}

	return checkPer;
}


bool cMonster::AttackSuccess( sObject target, unsigned long attributeType, short accuracyValue, unsigned long instantSuccess, unsigned long instantSuccessPer )
{
	// return DAMAGECALC->MonsterAttackSuccess( mObject.index, target, (eMONSTERATTACK_TYPE)skillIdx, (eATTRIBUTETYPE)attributeType);

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		/// 공격자 스텟성공수치
		unsigned long attackerStatusRate = GetPhysicAttackRate();

		/// 타겟 회피 수치
		unsigned long targetStatusAvoid = 0;

		/// 타겟(플레이어)
		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pTargetPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pTargetPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::AttackSuccess pTargetPlayer[%d] == NULL", target.index );
				return false;
			}

			targetLv = pTargetPlayer->GetLevel();
			targetStatusAvoid = FloatToInt( pTargetPlayer->GetStatus2()->mPhysicAvoid );
		}
		else
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::AttackSuccess pMonster[%d] == NULL", target.index );
				return false;
			}

			targetLv = pMonster->GetLevel();
			targetStatusAvoid = pMonster->GetPhysicAvoid();
		}

		/// 성공율 퍼센트 수치 환산
		unsigned long resultSuccess = DAMAGECALC->PhyAttackSuccess100( attackerStatusRate, targetStatusAvoid, accuracyValue, 
			eOBJECTTYPE_PLAYER, attackerLv, targetLv, instantSuccess, instantSuccessPer );

		/// 확률체크
		unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100);
		if( randvalue > resultSuccess ) 
			return false;
	}
	else if( attributeType == eATTRIBUTETYPE_MAGIC )	/// 마법 스킬 성공확률 계산 처리
	{
		/// 공격자 스텟성공수치
		unsigned long attackerStatusRate = GetMagicAttackRate();

		/// 타겟 회피 수치
		unsigned long targetStatusAvoid = 0;

		/// 타겟(플레이어)
		if( target.type == eOBJECTTYPE_PLAYER )
		{
			cPlayer* pTargetPlayer = OBJECTMANAGER->GetPlayer( target.index );
			if( pTargetPlayer == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::AttackSuccess pTargetPlayer[%d] == NULL", target.index );
				return false;
			}

			targetLv = pTargetPlayer->GetLevel();
			targetStatusAvoid = FloatToInt( pTargetPlayer->GetStatus2()->mMagicAvoid );
		}
		else
		{
			cMonster* pMonster = OBJECTMANAGER->GetMonster( target.index );
			if( pMonster == NULL )
			{
				NETWORK2->PostServerEvent("cMonster::AttackSuccess pMonster[%d] == NULL", target.index );
				return false;
			}

			targetLv = pMonster->GetLevel();
			targetStatusAvoid = pMonster->GetMagicAvoid();
		}

		/// 성공율 퍼센트 수치 환산
		unsigned long resultSuccess = DAMAGECALC->MagAttackSuccess130( attackerStatusRate, targetStatusAvoid, accuracyValue, 
			eOBJECTTYPE_PLAYER, attackerLv, targetLv, instantSuccess, instantSuccessPer );

		/// 확률체크
		unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100);
		if( randvalue > resultSuccess ) 
			return false;
	}
	else
	{
		NETWORK2->PostServerEvent("cMonster::AttackSuccess attributeType[%d] ERROR", attributeType );
		return false;	
	}

	return true;
}


bool cMonster::CriticalSuccess( sObject target,  unsigned long attributeType, unsigned short criticalValue, unsigned long /*instantCri*/, unsigned long /*instantCriPer*/ )
{
	// return DAMAGECALC->MonsterCritical( mObject.index, target, (eMONSTERATTACK_TYPE)skillIdx, (eATTRIBUTETYPE)attributeType );
	if( target.type != eOBJECTTYPE_PLAYER )
	{
		assert(NULL);
		return false;
	}

	cPlayer* pTarget = OBJECTMANAGER->GetPlayer( target.index );
	if( pTarget == NULL )
	{
		NETWORK2->PostServerEvent("cMonster::CriticalSuccess pTarget[%d,%d] == NULL", target.type, target.index );
		return false;
	}

	sEquipAbility* pTargetItem = pTarget->GetEquipAbility();
	if( pTargetItem == NULL )
	{
		NETWORK2->PostServerEvent("cMonster::CriticalSuccess pTargetItem[%d,%d] == NULL", target.type, target.index );
		return false;
	}

	/// 기본정보
	unsigned char attackerLv = GetLevel();
	unsigned char targetLv = pTarget->GetLevel();

	unsigned long criSuccess = 0;

	/// 물리 스킬 성공확률 계산 처리
	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{		
		long attackerPer = (long)GetStatusPer()->mPhysicCriticalDef;
		long attackerPlus = (long)GetStatusPlus()->mPhysicCriticalDef;

		float targetCriDef = pTarget->GetStatus2()->mPhyCriDef;

		criSuccess = DAMAGECALC->MonsterCritical260( criticalValue, attackerLv, targetLv,
			attackerPer, attackerPlus, targetCriDef );
	}
	else if( attributeType == eATTRIBUTETYPE_MAGIC )
	{
		long attackerPer = (long)GetStatusPer()->mMagicCriticalDef;
		long attackerPlus = (long)GetStatusPlus()->mMagicCriticalDef;

		float targetCriDef = pTarget->GetStatus2()->mMagCriDef;

		criSuccess = DAMAGECALC->MonsterCritical260( criticalValue, attackerLv, targetLv,
			attackerPer, attackerPlus, targetCriDef );
	}
	else
	{
		NETWORK2->PostServerEvent("cMonster::CriticalSuccess attributeType[%d] ERROR", attributeType );
		return false;
	}

	/// 확률체크
	unsigned long randvalue = (unsigned long)(RANDOMTABLE->Get( ) * 100 + 1);
	if( randvalue > criSuccess ) 
		return false;

	return true;
}

void cMonster::StatusCalc()
{ 
	STATUSCALC->CalcMonster( mObject.index );
}

void cMonster::SkillEscape( unsigned long type )
{
	if( IsModeState() == false )
		return;

	cPArray* pImmune = GetBuffImmune();
	if( pImmune == NULL )
		return;

	for( unsigned int i=0; i<pImmune->GetSize(); i++ )
	{
		sModeInfoBase* p = (sModeInfoBase*)((*pImmune)[i]);
		if( p == NULL )
			return;

		if( p->type != eMODEINFO_IMMUNE )
			continue;

		sModeImmune* pInfo = (sModeImmune*)p;
		if( pInfo->mSkillApplyType == 0 )
			return;

		if( pInfo->mSkillApplyType == (unsigned long)type )
			return;
	}

	SetEscape();
}


void cMonster::SetEscape()
{
	switch( GetActionID() )
	{
	case eACT_ESCAPE:
	case eACT_DIE:
		return;
	}

	ActionChange( eACT_ESCAPE );
}


void cMonster::Comeback()
{
	/// 타겟 제거
	SetTarget( eOBJECTTYPE_NONE, 0 );

	/// 효과 제거
	ReleaseInfluenceSet( false );

	/// 원래 지역으로 이동중 - 선공몬스터 공격 안하게 함
	SetFollowEndMove( true );

	/// 선공 드랍 유저 클리어
	mFirstAttackCharacter = 0;
	mFirstAttackParty = 0;
	mLastAttacker = 0;

	/// 자유이동 범위 밖에 있는 경우만 RUN
	unsigned long freeRange = mpMonsterInfo->mFreeMoveRange;
	long moveSpeed = 0;
	mRangeCheck.SetRadius( (float)freeRange );
	if( mRangeCheck.IsRange( mObjectPos, mRegenPos ) == false )
		moveSpeed = SetMoveSpeed( mpMonsterInfo->mActionRunSpd, eMOVESPEED_RUN );	/// 몬스터 회귀 속도
	else
		moveSpeed = SetMoveSpeed( mpMonsterInfo->mActionWalkSpd, eMOVESPEED_WALK );

	/// 이동 안하는 몬스터 인경우
	if( moveSpeed != 0 )
	{
		float targetPosX = mRegenPos.x;
		float targetPosY = mRegenPos.y;

		/// 패턴 이동중이면 기존목적지를 타겟으로 잡는다.
		if( mpMovePattern != NULL && mpRegenMonsterInfo->mPatternIdx != 0)
		{
			cArray* pAry = &mpMovePattern->mPosAry;
			sMonsterPatternPos* pPos = (sMonsterPatternPos*)(*pAry)[mPatternPos];
			if( pPos != NULL)
			{
				freeRange = pPos->mRange;
				targetPosX = pPos->mPosX;
				targetPosY = pPos->mPosY;
			}
		}

		/// 범위안에서 랜덤 좌표의 리젠 위치 
		if( freeRange == 0 )
			freeRange = 1;
		targetPosX = targetPosX + ( rand() % freeRange ) - ( rand() % freeRange );
		targetPosY = targetPosY + ( rand() % freeRange ) - ( rand() % freeRange );

		/// 갈수 있는지 역인지 체크
		if( AIMANAGER->IsPossible( mMapNumber, targetPosX, targetPosY, mObject ) == true )
		{
			/// 이동 초기화
			MoveStop( true );
			SetMoveStopRange( 0 );

			/// 몬스터가 이동할 경로 셋팅
			SetPath( targetPosX, targetPosY );
		}
	}

	/// hp/mp 회복 설정 체크
	if( mpMonsterInfo->mIsRecovery == true )
	{
		if( mHP != GetMaxHP() )
			HPHeal( GetMaxHP(), true, mObject, 0, eTAKEDAMAGETYPE_NONE );

		if( mMP != GetMaxMP() )
			MPHeal( GetMaxMP(), false );
	}

	/// 자유 이동으로 변경
	ActionChange( eACT_MOVE );

	SetCombatFlag( false );

	/// 
	ClearImmune();

	mGenTotemCoolTimeMap.Clear();
	mGenMonCoolTimeMap.Clear();
	mGenGatheringCoolTimeMap.Clear();

	if( mSummonListMap.IsEmpty() == false )
	{
		cPHashMap::cIterator b = mSummonListMap.Begin();
		cPHashMap::cIterator end = mSummonListMap.End();
		for( ; b!=end; b++ )
		{
			cHashSet* pSet = (cHashSet*)(*b).mSecond;
			if( pSet )
				pSet->Clear();
		}
	}

	for( unsigned long i = 0 ; i < mSummonGatherAry.GetSize() ; ++i )
	{
		cGathering* pGather = OBJECTMANAGER->GetGathering( mSummonGatherAry[i] );
		if( pGather != NULL )
			pGather->GatheringDie( 0 );
	}
	mSummonGatherAry.Clear();
}


void cMonster::ReservationComeback()
{ 
	mReservationComeback = true;

	mTFixedPlayerIdx = 0;

	/// 자유 이동으로 변경
	ActionChange( eACT_MOVE );

	/// 
	ClearImmune();

	mGenTotemCoolTimeMap.Clear();
	mGenMonCoolTimeMap.Clear();
	mGenGatheringCoolTimeMap.Clear();

	if( mSummonListMap.IsEmpty() == false )
	{
		cPHashMap::cIterator b = mSummonListMap.Begin();
		cPHashMap::cIterator end = mSummonListMap.End();
		for( ; b!=end; b++ )
		{
			cHashSet* pSet = (cHashSet*)(*b).mSecond;
			if( pSet )
				pSet->Clear();
		}
	}

	for( unsigned long i = 0 ; i < mSummonGatherAry.GetSize() ; ++i )
	{
		cGathering* pGather = OBJECTMANAGER->GetGathering( mSummonGatherAry[i] );
		if( pGather != NULL )
			pGather->GatheringDie( 0 );
	}
	mSummonGatherAry.Clear();
}


void cMonster::CheckComback()
{ 
	if( mReservationComeback == true ) 
	{
		Comeback(); 
		mReservationComeback = false;
	}

}


bool cMonster::MoveUpdate( unsigned long deltaTime )
{
	if( mpMonsterInfo->mActionRunSpd == 0 && mpMonsterInfo->mActionWalkSpd == 0 )
		return false;

	/// 이동불가 상태 이상인 경우
	if( IsCantMove() == true )
		return false;

	if( mPathCount == 0 )
		return false;

	/// 경로가 설정되지 않았다
	if( mNextPos.x < 0.0f || mNextPos.y < 0.0f )
		return false;

	float moveDeltaTime = (float)(deltaTime) / SECOND_THOUSAND;

	/// 몬스터의 이동속도
	NiPoint2 pos = mObjectPos;
	float speed = static_cast<float>(GetMoveSpeed());

	/// 방향 설정
	NiPoint2 dir1;
	dir1.x = mNextPos.x - pos.x;
	dir1.y = mNextPos.y - pos.y;
	dir1.Unitize();

	const float limitOverTime = 0.01f;
	float plusTime;

	/// 목적지점 넘어서 저장된 시간을 이동시 +
	if( mMoveOverTime > limitOverTime )
		plusTime = limitOverTime;
	else
		plusTime = mMoveOverTime;

	mMoveOverTime = mMoveOverTime - plusTime;

	/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
	pos.x += (speed * ( moveDeltaTime + plusTime ) * dir1.x);
	pos.y += (speed * ( moveDeltaTime + plusTime ) * dir1.y);

	/// 목적지 좌표와 예상 이동 위치를 비교
	NiPoint2 dir2;
	dir2.x = mNextPos.x - pos.x;
	dir2.y = mNextPos.y - pos.y;
	dir2.Unitize();

	bool keepMoving = true;

	/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달로 판정
	if( dir2.Dot(dir1) <= 0.0f )
	{
		NiPoint2 overPoint( pos.x, pos.y );

		/// 길찾기 경로 목적 지점을 넘어선 경우 넘은 거리만큼 시간 보관
		float overLength = ( overPoint - mNextPos ).Length();
		mMoveOverTime = mMoveOverTime + ( overLength / speed );

		//CONSOLE->WriteLog("OverLength, MoveOverTime %f %f", overLength, mMoveOverTime );

		pos.x = mNextPos.x;
		pos.y = mNextPos.y;
		if( mPathIndex < mPathCount-1 )
		{
			mNextPos = mPathArray[++mPathIndex];

			float viewPosX, viewPosY;
			viewPosX = mObjectPos.x - mNextPos.x;
			viewPosY = mObjectPos.y - mNextPos.y;

			mDirection = atan2( viewPosX, viewPosY );
		}
		else
		{
			mPathCount = 0;
			mPathIndex = 0;
			mMoveOverTime = 0.0f;
			keepMoving = false;
		}
	}

	/// 몬스터가 목적지에서 근처(일정범위)까지 도달하면 정지하는 모듈
	if( mMoveStopRange != 0.0f )
	{
		/// 타겟 오브젝트의 최종 목적지
		NiPoint3 targetPos;
		NiPoint3 monsterGuessPos( pos.x, pos.y, 0.0f );
		AIMANAGER->CalcHeight( mMapNumber, &monsterGuessPos.z, monsterGuessPos.x, monsterGuessPos.y );
		NiPoint3 monsterNowPos( GetXPos(), GetYPos(), Height() );

		/// 실제 타겟의 현재위치
		cBaseObject* pBaseObject = GetTarget();
		if( pBaseObject )
		{
			/// 타겟 오브젝트가 있는경우 오브젝트의 실제 위치를 가져옴
			targetPos.x = pBaseObject->GetXPos();
			targetPos.y = pBaseObject->GetYPos();
			targetPos.z = pBaseObject->Height();
		}
		else
		{
			/// 타겟 오브젝트가 없는경우 기록되있던 타겟 오브젝트의 위치를 가져옴
			targetPos.x = mLastPos.x;
			targetPos.y = mLastPos.y;
			AIMANAGER->CalcHeight( mMapNumber, &targetPos.z, targetPos.x, targetPos.y );
		}

		float moveStopRangeFix = OBJECTMANAGER->ObjectSizeRange( this, pBaseObject, mMoveStopRange );

		mRangeCheck.SetRadius( moveStopRangeFix );


		/// 이동 예상 위치와 실제 타겟 오브젝트의 충돌체크
		if( mRangeCheck.IsRange( monsterGuessPos, targetPos ) )
		{
			/// 좌표 보정
			NiPoint3 p;

			NiPoint3 d = monsterNowPos - targetPos;
			d.Unitize();

			p.x = targetPos.x + ((moveStopRangeFix-1.0f)*d.x);
			p.y = targetPos.y + ((moveStopRangeFix-1.0f)*d.y);

			SetPos( p.x, p.y );
			SetMoveStopRange( 0.0f );
			mLastPos.x = p.x;
			mLastPos.y = p.y;

			return false;
		}
	}

	SetPos( pos.x, pos.y );

#ifdef _DEBUG
	/// 테스트코드
	static bool check = false;
	static unsigned long delayTime = 100;
	static bool sync = false;
	if( check == true && mServerPosTime < NETWORK2->GetAccumTime() )
	{
		mServerPosTime += delayTime;

		MSG_SERVERPOS_TEST syncMsg;
		syncMsg.Category = NM_PLAYER;
		syncMsg.Protocol = NM_PLAYER_TEST_SERVERPOS;
		syncMsg.mCharacterIdx = mObject.index;
		syncMsg.mServerPosX = mObjectPos.x;
		syncMsg.mServerPosY = mObjectPos.y;
		NETWORK2->QuickSend( this, (char*)&syncMsg, sizeof( MSG_SERVERPOS_TEST ) );
	}
#endif

	return keepMoving;
}


bool cMonster::PushPullUpdate( unsigned long deltaTime )
{
	if( mpMonsterInfo->mActionRunSpd == 0 && mpMonsterInfo->mActionWalkSpd == 0 )
		return false;

	/// 경로가 설정되지 않았다
	if( mNextPos.x < 0.0f || mNextPos.y < 0.0f )
		return false;

	float moveDeltaTime = (float)(deltaTime) / SECOND_THOUSAND;

	/// 몬스터의 이동속도
	NiPoint2 pos = mObjectPos;
	float speed = SKILL_PUSHPULL_SPEED;

	/// 방향 설정
	NiPoint2 dir1;
	dir1.x = mNextPos.x - pos.x;
	dir1.y = mNextPos.y - pos.y;
	dir1.Unitize();

	const float limitOverTime = 0.01f;
	float plusTime;

	/// 목적지점 넘어서 저장된 시간을 이동시 +
	if( mMoveOverTime > limitOverTime )
		plusTime = limitOverTime;
	else
		plusTime = mMoveOverTime;

	mMoveOverTime = mMoveOverTime - plusTime;

	/// 속도 * 경과시간 만큼의 예상 이동 위치 계산
	pos.x += (speed * ( moveDeltaTime + plusTime ) * dir1.x);
	pos.y += (speed * ( moveDeltaTime + plusTime ) * dir1.y);

	/// 목적지 좌표와 예상 이동 위치를 비교
	NiPoint2 dir2;
	dir2.x = mNextPos.x - pos.x;
	dir2.y = mNextPos.y - pos.y;
	dir2.Unitize();

	/// 예상 이동위치가 목적지 좌표를 넘어가면 목적지 도달로 판정
	if( dir2.Dot(dir1) <= 0.0f )
	{
		pos.x = mNextPos.x;
		pos.y = mNextPos.y;

		return false;
	}

	SetPos( pos.x, pos.y );

#ifdef _DEBUG
	/// 테스트코드
	static bool check = false;
	static unsigned long delayTime = 100;
	static bool sync = false;
	if( check == true && mServerPosTime < NETWORK2->GetAccumTime() )
	{
		mServerPosTime += delayTime;

		MSG_SERVERPOS_TEST syncMsg;
		syncMsg.Category = NM_PLAYER;
		syncMsg.Protocol = NM_PLAYER_TEST_SERVERPOS;
		syncMsg.mCharacterIdx = mObject.index;
		syncMsg.mServerPosX = mObjectPos.x;
		syncMsg.mServerPosY = mObjectPos.y;
		NETWORK2->QuickSend( this, (char*)&syncMsg, sizeof( MSG_SERVERPOS_TEST ) );
	}
#endif

	return true;
}


bool cMonster::SelectNextPath( float* pPosX, float* pPosY )
{
	if( pPosX == NULL || pPosY == NULL )
		return false;

	/// 이동 패턴 설정이 없으면 자유이동
	float targetPosX = GetRegenPos().x;
	float targetPosY = GetRegenPos().y;
	unsigned long freeRange = mpMonsterInfo->mFreeMoveRange;

	/// 이동 패턴이 설정된경우 목적지를 찾아낸다.
	if( mpRegenMonsterInfo != NULL && mpRegenMonsterInfo->mPatternIdx != 0 )
	{
		try
		{
			if( mpMovePattern == NULL )
				throw 1;

			cArray* pAry = &mpMovePattern->mPosAry;
			if( pAry == NULL )
				throw 2;

			if( pAry->GetSize() <= 1 )
				throw 3;

			if( (long)pAry->GetSize() <= mPatternPos )
				throw 4;

			long pos = mPatternPos;
			char dir = mPatternDir;

			if( mpMovePattern->mType == true )
			{
				/// 순환
				pos = pos + mPatternDir;

				if( (long)pAry->GetSize() <= pos )
					pos = 0;
			}
			else
			{
				/// 역주행
				if( pos + dir >= (long)pAry->GetSize() )
					dir = -1;
				else if( pos + dir < 0 )
					dir = 1;

				pos = pos + dir;
			}

			sMonsterPatternPos* pPos = (sMonsterPatternPos*)(*pAry)[pos];
			if( pPos == NULL )
				throw 5;

			mPatternPos = pos;
			mPatternDir = dir;
			targetPosX = pPos->mPosX;
			targetPosY = pPos->mPosY;
			freeRange = pPos->mRange;

			if( pPos->mRelaxPoint == true )
				mPatternWait = pPos->mWaitTime;
		}
		catch ( int error )
		{
			mPatternPos = 0;
			mPatternDir = 1;
			NETWORK2->PostServerEvent( "cMonster::SelectNextPath [%d][%d,%d,%d,%d]", error, mObject.index, GetRaceGender(),
				mpRegenMonsterInfo->mPatternIdx, mPatternPos );
		}
	}

	/// 범위안에서 랜덤 좌표 설정
	if( freeRange == 0 )
		freeRange = 1;

	targetPosX = targetPosX + ( rand() % (int)freeRange ) - ( rand() % (int)freeRange );
	targetPosY = targetPosY + ( rand() % (int)freeRange ) - ( rand() % (int)freeRange );
	if( targetPosX < 0 )
		targetPosX = 0;
	if( targetPosY < 0 )
		targetPosY = 0;

	*pPosX = targetPosX;
	*pPosY = targetPosY;

	return true;
}

void cMonster::GetNextPattern( long* pPatternPos, char* pPatternDir )
{
	if( pPatternPos == NULL || pPatternDir == NULL )
		return;

	*pPatternPos = mPatternPos;
	*pPatternDir = mPatternDir;
}


unsigned int cMonster::SetPath( float targetX, float targetY )
{
	/// 이동불가 상태 이상인 경우
	if( IsCantMove() == true )
		return 0;

	/// 타겟 좌표와 몬스터 좌표가 같으면 경로 설정안함
	if( GetXPos() == targetX && GetYPos() == targetY )
	{
		/// 경로 없음
		return 0;
	}

	/// 이동 할 최종 포인트를 기록해둠
	mLastPos.x = targetX;
	mLastPos.y = targetY;

	unsigned long timetick = GetTickCount();

	/// 길찾기 실행
	if( FindPath( NiPoint2( targetX, targetY ) ) == false )
	{
		return 0;
	}

	unsigned int pathCount = GetPathCount();
	assert( pathCount );

	unsigned long now = GetTickCount();

	if( now - timetick > 1000 )
	{
		NETWORK2->PostServerEvent("PathDelay[%d] cnt[%d] %.0f,%.0f -> %.0f,%.0f", now - timetick, pathCount, GetXPos(), GetYPos(), targetX, targetY );
		Verbose->WriteLog("PathDelay[%d] cnt[%d] %.0f,%.0f -> %.0f,%.0f", now - timetick, pathCount, GetXPos(), GetYPos(), targetX, targetY );
	}

	/// 이동 목적지(좌표)까지 이동하는 형태 
	SetMoveStopRange( 0 );

	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );

	/// 이동 메세지 발송
	MSG_SYN_MOVE_MONSTER* msg = (MSG_SYN_MOVE_MONSTER*)perIoContext->buffer;

	msg->Category = NM_MONSTER;
	msg->Protocol = NM_MONSTER_MOVE_SYN;

	msg->mMonsterIdx = GetObjectID();

	for( unsigned int i = 0 ; i < pathCount ; ++i )
		msg->mPathArray[i] = GetPathArray()[i];

	msg->mMoveSpeed = (unsigned short)GetMoveSpeed();
	msg->mMoveSpeedType = (unsigned char)mMoveSpeedType;
	msg->mCount = (unsigned short)pathCount;

	NETWORK2->QuickSend( this, (char*)msg, msg->GetMsgLength() );

	perIoContext->offset = msg->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext, "cMonster::SetPath:MSG_SYN_MOVE_MONSTER" );

	return pathCount;	
}



unsigned int cMonster::SetPath( float targetX, float targetY, float range, sObject target )
{
	/// 이동불가 상태 이상인 경우
	if( IsCantMove() == true )
		return 0;

	/// 타겟 좌표와 몬스터 좌표가 같으면 경로 설정안함
	if( GetXPos() == targetX && GetYPos() == targetY )
	{
		/// 경로 없음
		return 0;
	}

	/// 이동 할 최종 포인트를 기록해둠
	mLastPos.x = targetX;
	mLastPos.y = targetY;

	/// 길찾기 실행
	if( FindPath( NiPoint2(targetX, targetY) ) == false )
	{
		return 0;
	}

	unsigned int pathCount = GetPathCount();
	assert( pathCount );

	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );

	/// 이동끝까지 가기전에 range범위 안에 들어오면 이동을 멈추는 이동형태
	SetMoveStopRange( range );

	/// 에이전트 -> 클라이언트에 전송
	MSG_SYN_ACTIONMOVE_MONSTER*	msg = (MSG_SYN_ACTIONMOVE_MONSTER*)perIoContext->buffer;

	msg->Category = NM_MONSTER;
	msg->Protocol = NM_MONSTER_ACTIONMOVE_SYN;

	for( unsigned int i = 0 ; i < pathCount ; ++i )
		msg->mPathArray[i] = GetPathArray()[i];

	msg->mMonsterIdx = GetObjectID();
	msg->mMoveSpeed = (unsigned short)GetMoveSpeed();
	msg->mCount = (unsigned short)pathCount;
	msg->mTarget = target;
	msg->mRange = range;

	NETWORK2->QuickSend( this, (char*)msg, msg->GetMsgLength() );

	perIoContext->offset = msg->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext, "cMonster::SetPath:MSG_SYN_ACTIONMOVE_MONSTER" );

	return pathCount;
}



void cMonster::ActionChange( eACTION_ID state )
{
	switch( state )
	{
	case eACT_MOVE:			SetAction( mActionMove ); SetCombatFlag( false ); break;
	case eACT_DIE:			SetAction( mActionDie ); SetCombatFlag( false ); break;
	case eACT_ATTACK:		SetAction( mActionAttack ); SetCombatFlag( true ); break;
	case eACT_ESCAPE:		SetAction( mActionEscape ); break;
	case eACT_FOLLOW:		SetAction( mActionFollow ); SetCombatFlag( true ); break;
	case eACT_IDLE:			SetAction( mActionIdle ); break;
	case eACT_PUSHPULL:		SetAction( mActionPushPull ); break;
	}
}



void cMonster::SetAction( cAction& pNewAtion )
{
	if( mpAction != NULL && mpAction->GetActionID() == eACT_DIE && pNewAtion.GetActionID() != eACT_IDLE )
	{
		return;
	}

	/// 이미 사용중인 상태면 굳이 바꿀 필요는 없다.
	if( mpAction == &pNewAtion ) return;
	mpAction = NULL;
	mpAction = &pNewAtion;
	mpAction->ActionInit( this, NETWORK2->GetAccumTime() );
}


//bool cMonster::AddInfluence( unsigned long uniqueIdx, unsigned long influenceClassIdx )
bool cMonster::AddInfluence( cInfluenceObject* pInf )
{ 
	if( pInf == NULL )
		return false;

	if( mInfluenceSet.Insert( pInf->GetUniqueIdx() ) == false )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster[%d]::AddSkillInfluence mInfluenceSet.Insert( %d, %d ) == false", 
			mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx() );
		return false;
	}

	sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
	if( pScript != NULL )
	{
		switch( pInf->GetChangeMPType() )
		{
		case eINFCHANGEMPTYPE_NONE:
			break;
		case eINFCHANGEMPTYPE_PHY:
		case eINFCHANGEMPTYPE_MAG:
		case eINFCHANGEMPTYPE_ALL:
			{
				mChangeMPDamageAry.PushBack( pInf->GetUniqueIdx() );
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			}
			break;
		default:
			NETWORK2->PostServerEvent("cMonster::AddSkillInfluence pInf->GetChangeMPType[%d,%d](%d,%d,%d) == false", 
				mObject.index, GetRaceGender(), pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx(), pInf->GetChangeMPType() );
		}

		switch( pInf->GetGuardType() )
		{
		case eINFGUARDTYPE_NONE:
			break;
		case eINFGUARDTYPE_PHY:
		case eINFGUARDTYPE_MAG:
		case eINFGUARDTYPE_ALL:
			{
				mGuardAry.PushBack( pInf->GetUniqueIdx() );
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			}
			break;
		default:
			NETWORK2->PostServerEvent("cMonster[%d]::AddSkillInfluence pInf->GetGuardType(%d,%d,%d,%d) == false", 
				mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx(), pInf->GetChangeMPType() );
		}

		/// 데미지를 받으면 삭제할 건지 체크하여 삽입
		switch( pInf->GetDeleteType() )
		{
		case eDELETETYPE_PLAYER_DIE:
		case eDELETETYPE_NODELETE_PLAYER_DIE:
			break;
		case eDELETETYPE_PLAYER_DAMAGED:
			{
				mDeleteTypeDamagedAry.PushBack( pInf->GetUniqueIdx() );
			}
			break;
		default:
			NETWORK2->PostServerEvent("cMonster::AddSkillInfluence pInf->GetDeleteType(%d,%d,%d,%d) == false", 
				mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx(), pInf->GetDeleteType() );
			break;
		}

		/// 현재 지속 효과 번호가 3개 밖에 없어서 3번만 체크
		for( int checkCount = 0; checkCount < 3; ++checkCount )
		{
			unsigned short checkStatusPlusIdx = eSTATUSPLUS_NONE;
			switch(checkCount)
			{
			case 0: checkStatusPlusIdx = pScript->mStatusPlusIdx1; break;
			case 1: checkStatusPlusIdx = pScript->mStatusPlusIdx2; break;
			case 2: checkStatusPlusIdx = pScript->mStatusPlusIdx3; break;
			}

			if( checkStatusPlusIdx != eSTATUSPLUS_NONE )
			{
				if( pScript->mType == eINFLUENCETYPE_BUF )
				{
					cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( checkStatusPlusIdx );
					if(  findPos != mBuffmap.End() )
					{
						/// 이미 들어가 있는 경우
						cHashSet& influenceSet = (*findPos).mSecond;
						influenceSet.Insert( pInf->GetUniqueIdx() );
					}
					else
					{
						/// 새로 추가한 경우
						cHashSet influenceSet;
						influenceSet.Insert( pInf->GetUniqueIdx() );
						mBuffmap.Insert( checkStatusPlusIdx, influenceSet );
					}
				}
				else if( pScript->mType == eINFLUENCETYPE_DEBUF )
				{
					cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( checkStatusPlusIdx );
					if(  findPos != mDeBuffmap.End() )
					{
						/// 이미 들어가 있는 경우
						cHashSet& influenceSet = (*findPos).mSecond;
						influenceSet.Insert( pInf->GetUniqueIdx() );
					}
					else
					{
						/// 새로 추가한 경우
						cHashSet influenceSet;
						influenceSet.Insert( pInf->GetUniqueIdx() );
						mDeBuffmap.Insert( checkStatusPlusIdx, influenceSet );
					}
				}
			}
			else
			{
				/// 효과는 순차적으로 집어 넣기 때문에 하나라도 빈 인덱스가 나오면 나머지는 체크할 필요가 없다.
				break;
			}
		}
	}

	return true;
}


void cMonster::DeleteInfluence( unsigned long influenceIdx )
{
	for( unsigned long i = 0 ; i < mGuardAry.GetSize() ; ++i )
	{
		if( influenceIdx == mGuardAry[i] )
			mGuardAry.PopAt( i );
	}

	for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
	{
		if( influenceIdx == mChangeMPDamageAry[i] )
			mChangeMPDamageAry.PopAt( i );
	}

	for( unsigned long i = 0 ; i < mDeleteTypeDamagedAry.GetSize() ; ++i )
	{
		if( influenceIdx == mDeleteTypeDamagedAry[i] )
			mDeleteTypeDamagedAry.PopAt( i );
	}

	/// 버프/디버프 관리 map 에서 빼기
	cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( influenceIdx );
	if( pInf != NULL )
	{
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
		if( pScript != NULL )
		{
			/// 현재 지속 효과 번호가 3개 밖에 없어서 3번만 체크
			for( int checkCount = 0; checkCount < 3; ++checkCount )
			{
				unsigned short checkStatusPlusIdx = eSTATUSPLUS_NONE;
				switch(checkCount)
				{
				case 0: checkStatusPlusIdx = pScript->mStatusPlusIdx1; break;
				case 1: checkStatusPlusIdx = pScript->mStatusPlusIdx2; break;
				case 2: checkStatusPlusIdx = pScript->mStatusPlusIdx3; break;
				}

				if( checkStatusPlusIdx != eSTATUSPLUS_NONE )
				{
					if( pScript->mType == eINFLUENCETYPE_BUF )
					{
						cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( checkStatusPlusIdx );
						if(  findPos != mBuffmap.End() )
						{
							/// 찾은 경우 해당 set에서 삭제
							cHashSet& influenceSet = (*findPos).mSecond;
							influenceSet.Erase( pInf->GetUniqueIdx() );
						}
						else
						{
							/// 효과 맵에는 있지만 버프 맵에 없는 경우 mInfluenceSet 과 mBuffmap 이 동기화 안맞았음!
							NETWORK2->PostServerEvent("cMonster::DeleteInfluence mBuffmap.Find(%d)==mBuffmap.End (%d,%d,%d)", 
								checkStatusPlusIdx, mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx()  );
						}
					}
					else if( pScript->mType == eINFLUENCETYPE_DEBUF )
					{
						cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( checkStatusPlusIdx );
						if(  findPos != mDeBuffmap.End() )
						{
							/// 찾은 경우 해당 set에서 삭제
							cHashSet& influenceSet = (*findPos).mSecond;
							influenceSet.Erase( pInf->GetUniqueIdx() );
						}
						else
						{
							/// 효과 맵에는 있지만 버프 맵에 없는 경우 mInfluenceSet 과 mBuffmap 이 동기화 안맞았음!
							NETWORK2->PostServerEvent("cMonster::DeleteInfluence mDeBuffmap.Find(%d)==mDeBuffmap.End (%d,%d,%d)", 
								checkStatusPlusIdx, mObject.index, pInf->GetUniqueIdx(), pInf->GetInfluenceClassIdx()  );
						}
					}
				}
				else
				{
					/// 효과는 순차적으로 집어 넣기 때문에 하나라도 빈 인덱스가 나오면 나머지는 체크할 필요가 없다.
					break;
				}
			}				
		}
	}

	if( mInfluenceSet.Erase( influenceIdx ) == false )
		NETWORK2->PostServerEvent("cMonster::EraseSkillInfluence[%d][%d]", mObject.index, influenceIdx );

//	cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( influenceIdx );
	if( pInf != NULL )
	{
		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( pInf->GetInfluenceClassIdx() );
		if( pScript != NULL )
		{
			if( pScript->mStatusPlusIdx1 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx1 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx2 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx2 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx3 >= eSTATUSPLUS_GUARDMP_PHY && pScript->mStatusPlusIdx3 <= eSTATUSPLUS_GUARDMP_ALL )
				std::sort( mChangeMPDamageAry.Begin(), mChangeMPDamageAry.End(), InfChangeMPMinSort );
			else if( pScript->mStatusPlusIdx1 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx1 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			else if( pScript->mStatusPlusIdx2 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx2 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
			else if( pScript->mStatusPlusIdx3 >= eSTATUSPLUS_GUARDCNT_PHY && pScript->mStatusPlusIdx3 <= eSTATUSPLUS_GUARDCNT_ALL )
				std::sort( mGuardAry.Begin(), mGuardAry.End(), InfLeftTimeSort );
		}
	}
}



void cMonster::ReleaseInfluenceSet( bool logOut ) 
{ 
	cSkillInfluenceSet::cIterator start = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

	unsigned long infIdx = 0;
	while( start != end )
	{
		infIdx = *start++;
		if( logOut == true )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( infIdx );
			if( pInf != NULL )
			{
				pInf->NoSendDelMsg();
				pInf->NoStatusCalc();
			}
		}

		SKILLMANAGER->DeleteInfluenceList( infIdx );
	}
}


bool cMonster::IsParentEqual( unsigned long parentUniqueIdx )
{
	cSkillInfluenceSet::cIterator beginAura = mInfluenceSet.Begin();
	cSkillInfluenceSet::cIterator endAura = mInfluenceSet.End();

	cInfluenceObject* pInf = NULL;
	while( beginAura != endAura )
	{
		/// 보유 효과 오브젝트
		pInf = SKILLMANAGER->GetInfluence( *beginAura++ );
		if( !pInf )	
		{ 
			assert(NULL); 
			NETWORK2->PostServerEvent("cSkillManager::IsParentEqual pHaveInfluenceObject==NULL Aura" );
			//-/-//InsertDeleteInfluenceObject( pHaveInfluenceObject );	/// 객체 삭제 대기열에 넣는다.
			continue; 
		}

		if( pInf->GetSwitchType() == eSWITCHTYPE_PARENT && pInf->GetRarentIdx() == parentUniqueIdx )
			return true;
	}

	return false;
}

void cMonster::AttackDummy( sObject attacker )
{
	if( attacker.type != eOBJECTTYPE_PLAYER )
		return;

	if( IsAutoDummy() == false )
		return;

	cPlayer* player = OBJECTMANAGER->GetPlayer( attacker.index );
	if( player != NULL )
	{
		// 오토더미인 경우, 경고메세지 발송
		HANDLE                  handle  = NULL;
		MSG_SYN_CHAT_AUTODUMMY* sendMsg = (MSG_SYN_CHAT_AUTODUMMY*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), 
																						 NM_CHAT, NM_CHAT_AUTODUMMY_SYN );
		if ( sendMsg != NULL )
		{
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_CHAT_AUTODUMMY) );
		}

		// 오토 공격 정보
		cHashMap::cIterator i = mAutoAttackerMap.Find( attacker.index );
		if( i != mAutoAttackerMap.End() )
		{	
			unsigned long& count = (unsigned long)(*i).mSecond;
			count++;

			if( count >= MAX_DUMMY_DAMAGE )
			{
				// 오토판정
				player->AutoLimit();

				// 정보 삭제
				mAutoAttackerMap.Erase( attacker.index );
			}
		}
		else
		{
			mAutoAttackerMap.Insert( attacker.index, 1 );
		}
	}
}

void cMonster::AddTakeDamage( sObject attacker, unsigned long damage, long distressPoint, eTAKEDAMAGE_TYPE type )
{
	unsigned long playerIdx = attacker.index;
	if( attacker.type == eOBJECTTYPE_TOTEM )
	{
		cTotem* totem = OBJECTMANAGER->GetTotem( attacker.index );
		if( totem == 0 )
			return;

		sObject o = totem->GetAttacker();
		if( o.type != eOBJECTTYPE_PLAYER )
			return;

		playerIdx = o.index;
	}
	else if( attacker.type == eOBJECTTYPE_PLAYER )
	{
		cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( attacker.index );
		if( pAttacker == NULL )
			return;
	}
	else
	{
		return;
	}

	float distressPer  = 0.0f;
	float distressPlus = 0.0f;

	cPlayer* pAttacker = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pAttacker )
	{
		distressPer  = pAttacker->GetStatus2Per( ePLAYER_STATUS_EXT_DISTRESS );
		distressPlus = pAttacker->GetStatus2Plus( ePLAYER_STATUS_EXT_DISTRESS );
	}
		
	/// 추적 시작시간 갱신, 추적 시작위치 갱신
	mFollowEndTime = NETWORK2->GetAccumTime() + mpMonsterInfo->mFollowTime;
	mFollowStartPos = mObjectPos;

	/// 등록값 계산..
	long calcPoint = 0;
	unsigned long calcDamage = 0;
	switch( type )
	{
	case eTAKEDAMAGETYPE_DAMAGE:
		{
			/// 최초 공격자에 대한 정보 등록..
			if( mFirstAttackCharacter == 0 )
			{
				mFirstAttackCharacter = playerIdx;
				cPlayer* pPlayer = NearGridFindPlayer( playerIdx, false );
				if( pPlayer != NULL )
					mFirstAttackParty = pPlayer->GetPartyIndex();
			}

			calcPoint = (long)(damage * AGGRO_DAMAGE_RATE) + distressPoint;
			calcDamage = damage;

			if( distressPlus != 0.0f || distressPer != 0.0f )
			{
				calcPoint = (long)(calcPoint + ( (calcPoint * distressPer) / 100) + distressPlus);
			}
		}
		break;
	case eTAKEDAMAGETYPE_DAMAGE_SETTARGET:
		{
			calcPoint = (long)(damage * AGGRO_DAMAGE_RATE) + distressPoint;
			calcDamage = damage;

		}
		break;
	case eTAKEDAMAGETYPE_HEAL:
		{
			calcPoint = (long)(damage * AGGRO_HEAL_RATE) + distressPoint;

			if( distressPlus != 0.0f || distressPer != 0.0f )
			{
				calcPoint = (long)(calcPoint + ( (calcPoint * distressPer) / 100) + distressPlus);
			}
		}
		break;
	case eTAKEDAMAGETYPE_POTION:
		{
			calcPoint = (long)(damage * AGGRO_POTION_RATE) + distressPoint;
		}
		break;
	case eTAKEDAMAGETYPE_ADDPLAYER:
		{
			calcPoint = distressPoint;
		}
		break;
	default:
		return;
	}



	/// damage List에 등록 또는 갱신
	PerTakeDamage* perTakeDamage = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, playerIdx );
	if( perTakeDamage  == 0 )
	{
		perTakeDamage = TAKEDAMAGEPOOL->AddTakeDamage( &mTakeDamageRoot, playerIdx, calcDamage, calcPoint );
		if( perTakeDamage )
		{
			/// player에 등록한다.
			cPlayer* pPlayer = GRIDMANAGER->GetPlayer( playerIdx );
			if( pPlayer )
				pPlayer->AddTargetingMonster( GetObjectID() );
		}
		else
		{
			assert(0);
		}

		/// party 정보 등록
		cPlayer* pPlayer = GRIDMANAGER->GetPlayer( playerIdx );
		if( pPlayer != NULL )
		{
			unsigned long partyIdx = pPlayer->GetPartyIndex();
			cParty* partyInfo = PARTYMAN->GetParty( partyIdx );
			if( partyInfo )
			{
				unsigned long* userArr = partyInfo->GetUserArr();
				for( unsigned int i = 0; i < partyInfo->GetCount(); ++i, ++userArr )
				{
					if( playerIdx == (*userArr) )
						continue;

					cPlayer* pPartyPlayer = GRIDMANAGER->GetPlayer( (*userArr) );
					if( pPartyPlayer == 0 )
						continue;

					if( TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, (*userArr) ) == 0 )
					{
						unsigned long partyDistress = 0;
						if( eTAKEDAMAGETYPE_DAMAGE == type )
							partyDistress = 1;

						if( TAKEDAMAGEPOOL->AddTakeDamage( &mTakeDamageRoot, (*userArr), 0, partyDistress ) )
						{
							/// player에 등록한다.
							pPartyPlayer->AddTargetingMonster( GetObjectID() );
						}
						else
						{
							assert(0);
						}
					}
				}
			}
		}
	}
	else
	{
		perTakeDamage->takeDamage += calcDamage;

		if( calcPoint < 0 )
		{
			if( perTakeDamage->distressPoint < (unsigned long)(calcPoint*(-1)) )
				perTakeDamage->distressPoint = 0;
			else
				perTakeDamage->distressPoint += calcPoint;
		}
		else
			perTakeDamage->distressPoint += calcPoint;

		perTakeDamage->noChangeTime = 0;
		perTakeDamage->changeFlag = true;
	}

	if( GetStateDie() == true )
		return;

	if( type == eTAKEDAMAGETYPE_DAMAGE_SETTARGET )
	{
		if( SetTarget( eOBJECTTYPE_PLAYER, playerIdx ) == true )
			ActionChange( eACT_ATTACK );

		return;
	}
	else if( type == eTAKEDAMAGETYPE_NONETARGET )
		return;

	/// 타겟 갱신..
	cBaseObject* pCurTarget = GetTarget();
	if( pCurTarget == 0 || calcPoint < 0 )
	{
		/// 신규 갱신..
		cPlayer* pNewTarget = 0;
		unsigned long maxPoint = 0;

		if( mTFixedPlayerIdx != 0 )
		{
			cPlayer* perTarget = NearGridFindPlayer( mTFixedPlayerIdx, true );
			if( perTarget )
				SetTarget( eOBJECTTYPE_PLAYER, mTFixedPlayerIdx );
		}

		PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
		while( per )
		{
			cPlayer* perTarget = NearGridFindPlayer( per->playerIdx, true );
			if( perTarget )
			{
				if( maxPoint < per->distressPoint )
					pNewTarget = perTarget;
			}
			per = (PerTakeDamage*)per->next;
		}

		if( pNewTarget )
		{
			/// 타겟 갱신
			if( SetTarget( pNewTarget->GetObjectType(), pNewTarget->GetObjectID() ) == false )
			{
				/// 기존 타겟도 없고 신규타겟도 잡을수 없는 경우 회귀 호출
				ClearTakeDamage();
				ReservationComeback();
				return;
			}
		}
		else
		{
			/// 타겟을 새로 잡을 수 없는 경우
			if( SetTarget( eOBJECTTYPE_PLAYER, playerIdx ) == false )
			{
				/// 기존 타겟도 없고 신규타겟도 잡을수 없는 경우 회귀 호출
				ClearTakeDamage();
				ReservationComeback();
				return;
			}
		}
		ActionChange( eACT_ATTACK );
	}
	else
	{
		PerTakeDamage* per = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, pCurTarget->GetObjectID() );
		if( per == 0 )
		{
			NETWORK2->PostServerEvent("cMonster::AddTakeDamage per == 0[%d]", pCurTarget->GetObjectID() );

			cPlayer* pCheck = NearGridFindPlayer( playerIdx, true );
			if( pCheck )
			{
				/// 현재 공격자 타겟팅..
				SetTarget( eOBJECTTYPE_PLAYER, playerIdx );
			}

			return;
		}

		if( mTFixedPlayerIdx == 0 )
		{
			if( perTakeDamage == NULL )
			{
				assert(NULL);
				return;
			}

			unsigned long maxPoint = (unsigned long)((float)per->distressPoint * 1.1f);
			if( maxPoint <= perTakeDamage->distressPoint )
			{
				cPlayer* pCheck = NearGridFindPlayer( playerIdx, true );
				if( pCheck )
				{
					/// 타겟 변경
					if( SetTarget( eOBJECTTYPE_PLAYER, playerIdx ) == false )
						return;
				}
			}
		}
	}
}

void cMonster::DelTakeDamagePlayer( unsigned long playerIdx )
{
	TAKEDAMAGEPOOL->ReleaseTakeDamage( &mTakeDamageRoot, playerIdx );
}

void cMonster::ClearDistressPointPlayer( unsigned long playerIdx )
{
	PerTakeDamage* per = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, playerIdx );
	if( per )
		per->distressPoint = 0;
	else
		assert(0);

	if( mTarget.index == playerIdx )
		ChangeActionTarget();
}

void cMonster::ClearDistressPoint()
{
	/// 신규 갱신..
	PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
	while( per )
	{
		per->distressPoint = 0;

		per = (PerTakeDamage*)per->next;
	}
}

void cMonster::ClearTakeDamage()
{
	/// 신규 갱신..
	PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
	while( per )
	{
		unsigned long playerIdx = per->playerIdx;

		cPlayer* perPlayer = NearGridFindPlayer( per->playerIdx, false );
		if( perPlayer )
			perPlayer->DelTargetingMonster( GetObjectID() );
		else
		{
			perPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
			if( perPlayer )
				perPlayer->DelTargetingMonster( GetObjectID() );
		}

		per = (PerTakeDamage*)per->next;
		TAKEDAMAGEPOOL->ReleaseTakeDamage( &mTakeDamageRoot,  playerIdx );
	}

	TAKEDAMAGEPOOL->Shutdown();
}

//void cMonster::AddTakeDamage( unsigned long playerIdx, unsigned long takeDamage )
//{
//	/// 처음 공격자 우선권 설정
//	if( mFirstAttackCharacter == 0 )
//	{
//		mFirstAttackCharacter = playerIdx;
//		cPlayer* pPlayer = NearGridFindPlayer( playerIdx );
//		if( pPlayer != NULL )
//			mFirstAttackParty = pPlayer->GetPartyIndex();
//	}
//
//	/// 공격 성공이면 추적 시간 재설정
//	mFollowEndTime = NETWORK2->GetAccumTime() + mpMonsterInfo->mFollowTime;
//
//	cTakeDamageMap::cIterator find = mTakeDamageMap.Find( playerIdx );
//	cTakeDamageMap::cIterator end = mTakeDamageMap.End();
//
//	if( find == end )
//	{
//		if( mTakeDamageMap.Insert( playerIdx, takeDamage ) == NULL )
//			assert(NULL);		
//	}
//	else
//		(*find).mSecond = (*find).mSecond + takeDamage;
//
//	/// 추적이동 끝 - 원래 위로 복귀 이동인지 체크
//	if( mFollowEndMove == true )
//		SetFollowEndMove( false );	/// 복귀중이 아님으로 변경
//
//	/// 선정된 타겟이 없으면 타겟 변경
//	if( GetTarget() == NULL )
//	{
//		SetTarget( eOBJECTTYPE_PLAYER, playerIdx );
//	}
//
//	if( GetStateDie() != true )
//		SendSpeech( eMONSTERTALK_DAMAGE );
//}



unsigned long cMonster::GetTakeDamage( unsigned long playerIdx )
{
	PerTakeDamage* per = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, playerIdx );
	if( per )
		return per->takeDamage;

	return 0;
}


eMONSTERATTACK_TYPE cMonster::SelectSkill()
{
	///// 현재 선정돼있으면 리턴
	//if( mAttackType != eMONSTERATTACK_MAX )
	//	return mAttackType;

	/// 타겟 객체 확인
	cPlayer* pTarget = (cPlayer*)GetTarget();
	if( pTarget == NULL || pTarget->GetStateDie() == true )
	{
		mAttackType = eMONSTERATTACK_MAX;
		return mAttackType;
	}

	// 오토더미는 일반스킬만 사용
	if( IsAutoDummy() == true )
		return eMONSTERATTACK_NORMAL1;

	if( mModeInitSkillIdx != eMONSTERATTACK_MAX )
	{
		sMonsterSkillScript* pMonsterSkillScript = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, mModeInitSkillIdx );
		if( pMonsterSkillScript == NULL )
		{
			assert(NULL);
			return eMONSTERATTACK_MAX;
		}

		if( pMonsterSkillScript->mApplyType == eAPPLYTYPE_ENEMY && mTarget.type != eOBJECTTYPE_PLAYER )
			ChangeActionTarget();

		return SelectApplyTargetSkill( mpMonsterInfo->mMonsterClassIdx, mModeInitSkillIdx );
	}

	/// 모드스킬 선정된것이 있고 대상이 이동 상태가 아니면 모드스킬 사용
	if( mModeSkillIdx != eMONSTERATTACK_MAX && pTarget->GetState() != eOBJECT_STATE_MOVE )
	{
		sMonsterSkillScript* pMonsterSkillScript = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, mModeSkillIdx );
		if( pMonsterSkillScript == NULL )
		{
			assert(NULL);
			return eMONSTERATTACK_MAX;
		}

		if( pMonsterSkillScript->mApplyType == eAPPLYTYPE_ENEMY && mTarget.type == eOBJECTTYPE_MONSTER )
			ChangeActionTarget();

		mAttackType = mModeSkillIdx;

		return SelectApplyTargetSkill( mpMonsterInfo->mMonsterClassIdx, mModeSkillIdx );
	}

	cCoolTime::cIterator iter;

	mTempSelectSkill.Clear();

	if( mODDITY[eODDITYTYPE_SLEEP] != 0 )
	{
		mAttackType = eMONSTERATTACK_MAX;
		return mAttackType;
	}
	if( mODDITY[eODDITYTYPE_STUN] != 0 )
	{
		mAttackType = eMONSTERATTACK_MAX;
		return mAttackType;
	}

	unsigned long useTotalPer = 0;
	eMONSTERATTACK_TYPE attackType;
	for( iter = mSkillCoolTimeMap.Begin() ; iter != mSkillCoolTimeMap.End() ; ++iter )
	{
		attackType = (eMONSTERATTACK_TYPE)(*iter).mFirst;

		/// 쿨타임 시간 안됨
		if( (*iter).mSecond > NETWORK2->GetAccumTime() )
		{
			continue;
		}

		sMonsterSkillScript* pMonsterSkillScript = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, attackType );
		if( pMonsterSkillScript == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("Monster::SelectSkill pMonsterSkillScript[%d,%d] == NULL", mpMonsterInfo->mMonsterClassIdx, attackType);
			continue;
		}

		if( IsCantSkill( pMonsterSkillScript->mAttributeType, attackType ) == true )
			continue;

		switch( pMonsterSkillScript->mUseHPCheck )
		{
		case eMONSTERHPCHECK_NONE:
			break;
		case eMONSTERHPCHECK_LOWHP40:
			{
				if( GetMaxHP() == 0 )
				{
					NETWORK2->PostServerEvent("0 divide 3");
					continue;
				}

				if( mHP * 100 / GetMaxHP() >= SKILL_USE_HP40 )
					continue;
			}
			break;
		case eMONSTERHPCHECK_LOWHP80:
			{
				if( GetMaxHP() == 0 )
				{
					NETWORK2->PostServerEvent("0 divide 4");
					continue;
				}

				if( mHP * 100 / GetMaxHP() >= SKILL_USE_HP80 )
					continue;
			}
			break;
		}

		if( mMP < pMonsterSkillScript->mUseMP )
		{
			continue;
		}

		if( pMonsterSkillScript->mSkillUsePer == 0 )
			continue;

		/// 스킬 선정에 필요한 항목 비율 계산
		unsigned long skillRate = CalcSkillRate( pTarget, attackType, pMonsterSkillScript->mSkillUsePer );
		useTotalPer = useTotalPer + skillRate;

		/// 조건에 만족하는 공격타입을 담는다.
		mTempSelectSkill.Insert( attackType, skillRate );
	}

	if( useTotalPer == 0 )
		return eMONSTERATTACK_MAX;

	/// 선택된 공격타입의 비율 전체 합계중 한지점을 정한다.
	unsigned long randSelect = rand() % useTotalPer + 1;

	/// 선택된 한지점의 공격타입값을 찾아 리턴한다.
	cHashMap::cIterator sI = mTempSelectSkill.Begin();
	cHashMap::cIterator sE = mTempSelectSkill.End();
	for( ; sI != sE ; ++sI )
	{
		unsigned long attackType = (*sI).mFirst;
		unsigned long rate = (*sI).mSecond;

		if( rate >= randSelect )
		{
			//mAttackType = (eMONSTERATTACK_TYPE)attackType;
			//return mAttackType;

			mAttackType = (eMONSTERATTACK_TYPE)attackType;
			break;
		}

		randSelect = randSelect - rate;
	}

	return SelectApplyTargetSkill( mpMonsterInfo->mMonsterClassIdx, mAttackType );
}


eMONSTERATTACK_TYPE cMonster::SelectApplyTargetSkill( unsigned long monsClassIdx, eMONSTERATTACK_TYPE attackType )
{
	sMonsterSkillScript* pMonsterSkillScript = SKILLSCRIPT->GetMonsterSkillInfo( monsClassIdx, attackType );
	if( pMonsterSkillScript == NULL )
		return eMONSTERATTACK_MAX;

	cRangeCheck rangeCheck;

	switch( pMonsterSkillScript->mApplyType )
	{
	case eAPPLYTYPE_SELF:	SetTarget( mObject.type, mObject.index );	return attackType;
	case eAPPLYTYPE_ENEMY:
		{
			if( mTarget.type == eOBJECTTYPE_MONSTER )
				ChangeActionTarget();

			return attackType;
		}
		break;
	case eAPPLYTYPE_BUDDY:
		{
			cLongAry buddyAry;
			buddyAry.Clear();

			cMonster* pMonster = GRIDMANAGER->FindFirstMonster( this );
			while( pMonster != NULL )
			{
				if( pMonster->GetStateDie() == true )
				{
					pMonster = GRIDMANAGER->FindNextMonster();
					continue;
				}

				if( pMonster->GetTakeDamageRoot()->count == 0 )
				{
					pMonster = GRIDMANAGER->FindNextMonster();
					continue;
				}

				rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
				if( rangeCheck.IsNotRange( mObjectPos, pMonster->GetPos() ) )
				{
					pMonster = GRIDMANAGER->FindNextMonster();
					continue;
				}

				buddyAry.PushBack( pMonster->GetObjectID() );

				pMonster = GRIDMANAGER->FindNextMonster();
			}

			if( buddyAry.GetSize() != 0 )
			{
				unsigned long selectRand = rand() % buddyAry.GetSize();
				SetTarget( eOBJECTTYPE_MONSTER, buddyAry[selectRand] );
				return attackType;
			}
		}
		break;
	case eAPPLYTYPE_FAR:
		{
			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			unsigned long selectPlayer = 0;
			float topValue = 0;
			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					/// 시야 범위 안에 잇는 플레이어중 가장 먼 케릭터
					float value = ( mObjectPos - pPlayer->GetPos() ).Length();
					if( mpMonsterInfo->mSeekRange >= value )
					{
						if( value > topValue )
						{
							topValue = value;
							selectPlayer = per->playerIdx;
						}
					}
				}
				per = (PerTakeDamage*)per->next;
			}

			SetTarget( eOBJECTTYPE_PLAYER, selectPlayer );
			return attackType;
		}
		break;
	case eAPPLYTYPE_RANDOM:
		{
			cLongAry randAry;
			randAry.Clear();

			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
						randAry.PushBack( per->playerIdx );
				}
				per = (PerTakeDamage*)per->next;
			}

			if( randAry.GetSize() != 0 )
			{
				unsigned long selectRand = rand() % randAry.GetSize();
				SetTarget( eOBJECTTYPE_PLAYER, randAry[selectRand] );
				return attackType;
			}
		}
		break;
	case eAPPLYTYPE_DISTRESS2:
		{
			cPArray sortAry;
			sortAry.Clear();

			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
						sortAry.PushBack( per );
				}
				per = (PerTakeDamage*)per->next;
			}

			std::sort( sortAry.Begin(), sortAry.End(), DistressCompare );

			for( unsigned long i = 2 ; i > 0 ; --i )
			{
				if( i <= sortAry.GetSize() )
				{
					if( i > 0 )
					{
						PerTakeDamage* per2 = (PerTakeDamage*)sortAry[i-1];
						if( per2 != NULL )
						{
							SetTarget( eOBJECTTYPE_PLAYER, per2->playerIdx );
							return attackType;
						}
					}
				}
			}
		}
		break;
	case eAPPLYTYPE_DISTRESS3:
		{
			cPArray sortAry;
			sortAry.Clear();

			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
						sortAry.PushBack( per );
				}
				per = (PerTakeDamage*)per->next;
			}

			std::sort( sortAry.Begin(), sortAry.End(), DistressCompare );

			for( unsigned long i = 3 ; i > 0 ; --i )
			{
				if( i <= sortAry.GetSize() )
				{
					if( i > 0 )
					{
						PerTakeDamage* per2 = (PerTakeDamage*)sortAry[i-1];
						if( per2 != NULL )
						{
							SetTarget( eOBJECTTYPE_PLAYER, per2->playerIdx );
							return attackType;
						}
					}
				}
			}
		}
		break;
	case eAPPLYTYPE_DISTRESSMIN:
		{
			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			unsigned long selectPlayer = 0;
			unsigned long topValue = ULONG_MAX;
			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					{
						unsigned long value = per->distressPoint;
						if( value < topValue )
						{
							topValue = value;
							selectPlayer = per->playerIdx;
						}
					}
				}
				per = (PerTakeDamage*)per->next;
			}

			if( selectPlayer != 0 )
			{
				SetTarget( eOBJECTTYPE_PLAYER, selectPlayer );
				return attackType;
			}
		}
		break;
	case eAPPLYTYPE_HPMAX:
		{
			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			unsigned long selectPlayer = 0;
			unsigned long topValue = 0;
			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					{
						unsigned long value = pPlayer->GetMaxHP();
						if( value > topValue )
						{
							topValue = value;
							selectPlayer = per->playerIdx;
						}
					}
				}
				per = (PerTakeDamage*)per->next;
			}

			if( selectPlayer == 0 )
			{
				SetTarget( eOBJECTTYPE_PLAYER, selectPlayer );
				return attackType;
			}
		}
		break;
	case eAPPLYTYPE_HPMIN:
		{
			/// 보스를 공격하던 공격자 목록을 자식에 넣음
			if( mTakeDamageRoot.count == 0 )
				return eMONSTERATTACK_MAX;

			unsigned long selectPlayer = 0;
			unsigned long topValue = ULONG_MAX;
			PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
			while( per )
			{
				cPlayer* pPlayer = GRIDMANAGER->GetPlayer( per->playerIdx );
				if( pPlayer != NULL )
				{
					rangeCheck.SetRadius( (float)mpMonsterInfo->mSeekRange );
					if( rangeCheck.IsRange( mObjectPos, pPlayer->GetPos() ) == true )
					{
						unsigned long value = pPlayer->GetMaxHP();
						if( value < topValue )
						{
							topValue = value;
							selectPlayer = per->playerIdx;
						}
					}
				}
				per = (PerTakeDamage*)per->next;
			}

			if( selectPlayer == 0 )
			{
				SetTarget( eOBJECTTYPE_PLAYER, selectPlayer );
				return attackType;
			}
		}
		break;
	}

	return eMONSTERATTACK_MAX;
}


void cMonster::UpdateSkillCoolTime( eMONSTERATTACK_TYPE attackType )
{
	if( eMONSTERATTACK_MAX > attackType )
	{
		cCoolTime::cIterator iter = mSkillCoolTimeMap.Find( attackType );
		cCoolTime::cIterator end = mSkillCoolTimeMap.End();

		sMonsterSkillScript* pSkill = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, attackType );
		if( pSkill == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("monster coolTime error - script MonClassIdx[%d], attackType[%d]",mpMonsterInfo->mMonsterClassIdx, attackType );
			return;
		}

		if( iter != end )
		{
			(*iter).mSecond = NETWORK2->GetAccumTime() + pSkill->mCoolTime;
		}
	}
	else
	{
		cCoolTime::cIterator iter = mEliteCoolTimeMap.Find( attackType );
		cCoolTime::cIterator end = mEliteCoolTimeMap.End();

		sMonsterSkillScript* pSkill = SKILLSCRIPT->GetMonsterSkillInfo( mpMonsterInfo->mMonsterClassIdx, attackType );
		if( pSkill == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("monster coolTime error - script MonClassIdx[%d], attackType[%d]",mpMonsterInfo->mMonsterClassIdx, attackType );
			return;
		}

		if( iter != end )
		{
			(*iter).mSecond = NETWORK2->GetAccumTime() + pSkill->mCoolTime;
		}
		else
		{
			mEliteCoolTimeMap.Insert( attackType, NETWORK2->GetAccumTime() + pSkill->mCoolTime );
		}

	}
}


void cMonster::CalcSkillWaitTime( unsigned long skillEndTime )
{ 
	mSkillWaitEndTime = skillEndTime + SKILL_WAIT_TIME; 
}


bool cMonster::IsSkillWaitEnd()
{ 
	return mSkillWaitEndTime <= NETWORK2->GetAccumTime(); 
}


inline bool cMonster::GetStateDie() 
{
	switch( GetActionID() )
	{
	case eACT_DIE:
		return true;
	}		
	return false;
}


void cMonster::OutsideDestroy()
{
	mOutsideDestroy = true; 		
	ActionChange( eACT_DIE );

	MSG_SYN_MONSTER_DESTROY synMsg;
	synMsg.Category			= NM_MONSTER;
	synMsg.Protocol			= NM_MONSTER_DESTROY_SYN;
	synMsg.mMonsterIdx	    = mObject.index;
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_DESTROY) );
}


void cMonster::SetStateOddity( long* pOddity )
{
	for( int i = 0 ; i < eODDITYTYPE_MAX ; ++i )
	{
		if( mODDITY[i] != pOddity[i] )
		{
			mODDITY[i] = pOddity[i];

			if( pOddity[i] != 0 )
			{
				/// 상태 이상중 클라이언트에 메세지를 발송 해줘야 하는 부분 처리
				switch(i)
				{
				case eODDITYTYPE_CANTSKILL_PHY:
					{
						if( GetActionID() == eACT_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STOPSKILL_PHY ) == false*/ )
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_PHYSICAL );
					}
					break;
				case eODDITYTYPE_CANTSKILL_MAG:
					{
						if( GetActionID() == eACT_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STOPSKILL_MAG ) == false*/ )
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_MAGIC );
					}
					break;
				case eODDITYTYPE_CANTMOVE:
					{
						MoveStop( true );
					}
					break;
				case eODDITYTYPE_SLEEP:
					{
						MoveStop( true );

						if( GetActionID() == eACT_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_SLEEP ) == false */)
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
					}
					break;
				case eODDITYTYPE_STUN:
					{
						MoveStop( true );

						if( GetActionID() == eACT_ATTACK /*&& IsImmuneApplyType( eSTATUSPLUS_STUN ) == false*/ )
							SKILLMANAGER->CastSkillCancel( mObject, true, eATTRIBUTETYPE_NONE );
					}
					break;
				}
			}
		}
	}
}


bool cMonster::IsCantSkill( unsigned long attributeType )
{ 
	if( mODDITY[eODDITYTYPE_SLEEP] != 0 )
		return true; 
	if( mODDITY[eODDITYTYPE_STUN] != 0 )
		return true;
	if( eMONSTERATTACK_NORMAL1 == mAttackType || eMONSTERATTACK_NORMAL2 == mAttackType || eMONSTERATTACK_MAX == mAttackType )
		return false;

	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		if( mODDITY[eODDITYTYPE_CANTSKILL_PHY] == 0 )
			return false;
		else
			return true;
	}
	else
	{
		if( mODDITY[eODDITYTYPE_CANTSKILL_MAG] == 0 )
			return false;
		else
			return true;
	}
}

bool cMonster::IsCantSkill( unsigned long attributeType, eMONSTERATTACK_TYPE attackType )
{
	if( mODDITY[eODDITYTYPE_SLEEP] != 0 )
		return true; 
	if( mODDITY[eODDITYTYPE_STUN] != 0 )
		return true;
	if( eMONSTERATTACK_NORMAL1 == attackType || eMONSTERATTACK_NORMAL2 == attackType || eMONSTERATTACK_MAX == attackType )
		return false;

	if( attributeType == eATTRIBUTETYPE_PHYSICAL )
	{
		if( mODDITY[eODDITYTYPE_CANTSKILL_PHY] == 0 )
			return false;
		else
			return true;
	}
	else
	{
		if( mODDITY[eODDITYTYPE_CANTSKILL_MAG] == 0 )
			return false;
		else
			return true;
	}
}

void cMonster::TargetChange( sObject attacker, long distressPoint )
{
	/// 어그로 수치 증가
	AddTakeDamage( attacker, 0, distressPoint, eTAKEDAMAGETYPE_DAMAGE_SETTARGET );

	/// 추적 상태 설정
	if( GetStateDie() == false )
		ActionChange( eACT_ATTACK );
}

void cMonster::SetSkillPushPull( unsigned short type, cBaseObject* pAttacker, unsigned long range )
{
	if( pAttacker == NULL )
		return;

	if( GetActionID() == eACT_DIE && GetActionID() == eACT_PUSHPULL )
		return;

	NiPoint2 attackerPos = pAttacker->GetPos();

	if( type == 0 )
		return;


	/// 공격자에서 타겟(현재오브젝트)의 방향산출
	NiPoint2 dir;
	dir.x = mObjectPos.x - attackerPos.x;
	dir.y = mObjectPos.y - attackerPos.y;
	dir.Unitize();

	float fixRange = 0.0f;
	NiPoint2 targetPos = mObjectPos;
	if( type == eSTATUSPLUS_AROUND_PUSH )
	{
		fixRange = OBJECTMANAGER->ObjectSizeRange( this, pAttacker, (float)range );

		if( (mObjectPos - attackerPos).Length() > fixRange )
			return;

		targetPos = attackerPos + dir * fixRange;
	}
	else if( type == eSTATUSPLUS_AROUND_PULL )
	{
		fixRange = OBJECTMANAGER->ObjectSizeRange( this, pAttacker, 100.0f );

		if( (mObjectPos - attackerPos).Length() < fixRange )
			return;

		targetPos = attackerPos + dir * fixRange;
	}
	else if( type == eSTATUSPLUS_TARGET_PUSH )
	{
		targetPos = mObjectPos + dir * (float)range;
	}

	NiPoint2 destPos = mObjectPos;

	/// 목적지까지의 방향정보
	NiPoint2 divideDir = targetPos - mObjectPos;
	float moveRange = divideDir.Unitize();

	/// 분할 숫자
	unsigned long divideCnt = 10;

	/// 이동거리를 분할
	float divideRange = moveRange / (float)divideCnt;
	for( unsigned long i = 0 ; i < divideCnt ; ++i )	/// 먼저 선출된 끝점부터 공격자에 가까운 위치로 체크
	{
		/// 목적지에서 분할한 거리만큼 빼가면서 이동가능한지 체크후 가능하면 그위치로 이동한다.
		NiPoint2 pos = mObjectPos + ( divideDir * ( moveRange - ( divideRange * i ) ) );
		if( IsPushPullPossible( mObjectPos, pos ) == true )
		{
			destPos = pos;
			break;
		}
	}

	MSG_SYN_SKLL_PUSHPULL sync;
	sync.Category = NM_SKILL;
	sync.Protocol = NM_SKILL_PUSHPULL_SYN;
	sync.mTarget = mObject;
	sync.mPosX = destPos.x;
	sync.mPosY = destPos.y;

	NETWORK2->QuickSend( this, (char*)&sync, sizeof(sync) );

	mNextPos = destPos;
	ActionChange( eACT_PUSHPULL );
}

bool cMonster::IsPushPullPossible( NiPoint2 startPos, NiPoint2 goalPos )
{
	switch( NETWORK2->GetServerType() )
	{
	case _E_ST_ID_PVP_:
		{
			cBaseDeathMatch* pDMObject = PVPMANAGER->GetPvPDMObject( mMapNumber );
			if( pDMObject == NULL )
				return false;

			if( pDMObject->IsMovePossible( startPos, goalPos ) == false )
				return false;
		}
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			cThemeObject* ptheme = THEMEMANAGER->GetThemeObject( mMapNumber );
			if( ptheme == NULL )
				return false;

			if( ptheme->IsMovePossible( startPos, goalPos ) == false )
				return false;
		}
		break;
	}

	/// 직선 코스 이동 가능 인지 확인
	if( AIMANAGER->IsPossible( mMapNumber, startPos, goalPos, mObject ) == false )
		return false;

	return true;
}

unsigned long cMonster::CalcPhysicAttack( unsigned long skillAttack )
{
	float num391 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 391 );

	// 공격력(몬스터)
	// A =	몬스터 공격력
	// B =	[몬스터] Lv * {num391}
	// C =	(( 100 + 버프성 공격력[%]값) / 100 )
	// D =	((A + B ) * C) + 버프성 공격력[+-]값
	float valueA = (float)skillAttack;
	float valueB = mpMonsterInfo->mLevel * num391;
	float valueC = (( 100 + mStatusPer.mPhysicAttack ) / 100 );
	float attack = (( valueA + valueB ) * valueC ) + mStatusPlus.mPhysicAttack;

	if( attack < 0.0f )
		return 0;

	return FloatToInt( attack ); 
}


unsigned long cMonster::CalcPhysicRangeAttack( unsigned long skillAttack )
{
	float num391 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 391 );

	// 공격력(몬스터)
	// A =	몬스터 공격력
	// B =	[몬스터] Lv * {num391}
	// C =	(( 100 + 버프성 공격력[%]값) / 100 )
	// D =	((A + B ) * C) + 버프성 공격력[+-]값
	float valueA = (float)skillAttack;
	float valueB = mpMonsterInfo->mLevel * num391;
	float valueC = (( 100 + mStatusPer.mPhysicRangeAttack ) / 100 );
	float attack = (( valueA + valueB ) * valueC ) + mStatusPlus.mPhysicRangeAttack;

	if( attack < 0.0f )
		return 0;

	return FloatToInt( attack );  
}


unsigned long cMonster::CalcMagicAttack( unsigned long skillAttack )
{
	float num391 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 391 );

	// 공격력(몬스터)
	// A =	몬스터 공격력
	// B =	[몬스터] Lv * {num391}
	// C =	(( 100 + 버프성 공격력[%]값) / 100 )
	// D =	((A + B ) * C) + 버프성 공격력[+-]값
	float valueA = (float)skillAttack;
	float valueB = mpMonsterInfo->mLevel * num391;
	float valueC = (( 100 + mStatusPer.mMagicAttack ) / 100 );
	float attack = (( valueA + valueB ) * valueC ) + mStatusPlus.mMagicAttack;

	if( attack < 0.0f )
		return 0;

	return FloatToInt( attack );  
}


unsigned long cMonster::GetPhysicDefense()
{
	float num401 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 401 );

	// 방어력(물리)
	// A =	캐릭터 스탯물리 방어력
	// B =	아이템 물리 방어력
	// C =	((100 + 버프성 물리 방어력[%]값 + 아이템 물리 방어력[%]값) / 100)
	// D =	((A / {num401} + B / {num402} ) * C) + 버프성 물리 방어력[+-]값 + 아이템 물리 방어력[+-]값

	// 몬스터의 경우: 스탯 물리 방어력 = 몬스터 물리 방어력
	// ※ 몬스터는 아이템이 없다.
	float valueA = mpMonsterInfo->mDef;
	float valueC = (( 100 + mStatusPer.mPhysicDefense ) / 100 );
	float def = (( valueA / num401 ) * valueC ) + mStatusPlus.mPhysicDefense;

	if( def < 0.0f )
		return 0;

	return FloatToInt( def ); 
}


unsigned long cMonster::GetMagicDefense()
{
	float num431 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 431 );

	// 방어력(마법)
	// A =	캐릭터 스탯마법 방어력
	// B =	아이템 마법 방어력
	// C =	((100 + 버프성 마법 방어력[%]값 + 아이템 마법 방어력[%]값) / 100)
	// D =	((A /{num431} + B / {num432} ) * C) + 버프성 물리 방어력[+-]값 + 아이템 마법 방어력[+-]값

	// 몬스터의 경우: 스탯 마법 방어력 = 몬스터 마법 방어력
	// ※ 몬스터는 아이템이 없다.

	float valueA = mpMonsterInfo->mMdef;
	float valueC = (( 100 + mStatusPer.mMagicDefense ) / 100 );
	float def = (( valueA / num431 ) * valueC ) + mStatusPlus.mMagicDefense;

	if( def < 0.0f )
		return 0;

	return FloatToInt( def );  
}


unsigned long cMonster::GetPhysicAvoid()
{
	float num102 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 102 );

	/// 몬스터의 경우, [( [방어자] 캐릭터 스탯 회피(저항력) 수치 + 아이템 회피(저항력) 수치 ) = 회피 수치]로 계산
	float avoid = mpMonsterInfo->mAvoid;
	// C =	(( [방어자] 캐릭터 스탯 회피 수치 + 아이템 회피 수치 ) * ((100 + 버프성 회피 수치[%]값) / 100) + 버프성 회피 수치[+-]값 )) * {num102}
	avoid = (avoid * ( ( 100 + mStatusPer.mPhysicAvoid ) / 100 ) + mStatusPlus.mPhysicAvoid ) * num102; 

	if( avoid < 0.0f )
		return 0;

	return FloatToInt( avoid );
}


unsigned long cMonster::GetMagicAvoid()
{
	float num132 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 132 );

	/// 몬스터의 경우, [( [방어자] 캐릭터 스탯 회피(저항력) 수치 + 아이템 회피(저항력) 수치 ) = 회피 수치]로 계산
	float avoid = mpMonsterInfo->mAvoid;
	// C =	(( [방어자] 캐릭터 스탯 회피 수치 + 아이템 회피 수치 ) * ((100 + 버프성 회피 수치[%]값) / 100) + 버프성 회피 수치[+-]값 )) * {num132}
	avoid = (avoid * ( ( 100 + mStatusPer.mMagicAvoid ) / 100 ) + mStatusPlus.mMagicAvoid ) * num132; 

	if( avoid < 0.0f )
		return 0;

	return FloatToInt( avoid );
}


unsigned long cMonster::GetPhysicAttackRate()
{
	float num101 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 101 );
	float num102 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 102 );

	// 몬스터의 경우, [( [공격자] 캐릭터 스탯 명중(마법 명중) 수치 + 아이템 명중(마법 명중) 수치 ) = 명중 수치]로 계산
	// A =	{num101} + (( [공격자] 캐릭터 스탯 명중 수치) * ((100 + [공격자]아이템 명중 수치[%]값 + [공격자]버프성 명중 수치[%]값) / 100) + [공격자]아이템 명중 수치[+-]값 + [공격자]버프성 명중 수치[+-]값 ) * {num102}
	float attackRate = (float)mpMonsterInfo->mHit;
	attackRate = num101 + (( attackRate * ( ( 100 + mStatusPer.mPhysicAttackRate ) / 100 ) + mStatusPlus.mPhysicAttackRate ) * num102 );

	if( attackRate < 0.0f )
		return 0;

	return FloatToInt( attackRate );  
}


unsigned long cMonster::GetMagicAttackRate()
{
	float num131 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 131 );
	float num132 = STATUSSCRIPT->GetDamageCalcNumericalInfo( 132 );

	// ※ 몬스터 기본 마법 명중율은 스크립트에 있는 명중율을 같이 쓴다. - 값이 틀리지 않아 따로 기입할 필요 없다고함
	// 몬스터의 경우, [( [공격자] 캐릭터 스탯 명중(마법 명중) 수치 + 아이템 명중(마법 명중) 수치 ) = 명중 수치]로 계산
	// A =	{num131} + (( [공격자] 캐릭터 스탯 마법 명중 수치) * ((100  + [공격자]아이템 마법 명중 수치[%]값 + [공격자]버프성 마법 명중 수치[%]값) / 100)  + [공격자]아이템 마법 명중 수치[+-]값 + [공격자]버프성 마법 명중 수치[+-]값 ) * {num132}
	float attackRate = (float)mpMonsterInfo->mHit;
	attackRate = num131 + (( attackRate * ( ( 100 + mStatusPer.mMagicAttackRate ) / 100 ) + mStatusPlus.mMagicAttackRate ) * num132 );

	if( attackRate < 0.0f )
		return 0;

	return FloatToInt( attackRate ); 
}


//unsigned long cMonster::CalcPhysicCritical( unsigned long physicCritical )
//{
//	long tempPhysicCritical = (long)( physicCritical
//		+ ( physicCritical * mStatusPer.mPhysicCritical / 100 ) 
//		+ mStatusPlus.mPhysicCritical ); 
//
//	return tempPhysicCritical < 0 ? 0 : tempPhysicCritical; 
//}
//
//
//unsigned long cMonster::CalcMagicCritical( unsigned long magicCritical )
//{
//	long tempMagicCritical = (long)( magicCritical
//		+ ( magicCritical * mStatusPer.mMagicCritical / 100 ) 
//		+ mStatusPlus.mMagicCritical ); 
//
//	return tempMagicCritical < 0 ? 0 : tempMagicCritical; 
//}


float cMonster::GetAttackSpeed()
{
	if( mStatusPer.mAttackSpeed == 0.0f )
		return 1.0f;

	float attackSpeed = ( 100 + mStatusPer.mAttackSpeed ) / 100;

	if( attackSpeed <= 0.0f )
		attackSpeed = 0.1f;

	if( attackSpeed >= 2.0f )
		attackSpeed = 1.9f;

	return attackSpeed;
}


float cMonster::CalcStatusSkillRange( float skillRange, eRANGETYPE mRangeType )
{
	/// 스킬 사거리
	float range = skillRange;

	/// 효과 사거리
	float statusRangePlus = 0;
	float statusRangePer = 0;

	if( mRangeType == eRANGETYPE_CLOSE )
	{
		statusRangePlus = mStatusPlus.mInRange;
		statusRangePer = mStatusPer.mInRange;
	}
	else if( mRangeType == eRANGETYPE_LONG )
	{
		statusRangePlus = mStatusPlus.mOutRange;
		statusRangePer = mStatusPer.mOutRange;
	}
	else
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster::CalcStatusSkillRange mRangeType[%d] Err", mRangeType );
	}

	/// 사거리 계산
	range = range * ( ( 100 + statusRangePer ) / 100 ) + statusRangePlus;

	return range;
}


unsigned short cMonster::GetMoveSpeed()
{
	float moveSpeed = ( mStatusBase.mMoveSpeed
		+ ( mStatusBase.mMoveSpeed * mStatusPer.mMoveSpeed / 100 ) 
		+ mStatusPlus.mMoveSpeed ); 

	return moveSpeed < 0.0f ? 0 : (unsigned short)moveSpeed; 
}


unsigned short cMonster::SetMoveSpeed( unsigned long moveSpeed, eMOVE_SPEED_TYPE moveSpeedType )
{ 
	mMoveSpeedType = moveSpeedType;
	mStatusBase.mMoveSpeed = (float)moveSpeed; 
	return GetMoveSpeed(); 
}


void cMonster::SendMoveSpeed()
{
	MSG_SYN_MONSTER_MOVESPEED synMsg;
	synMsg.Category     = NM_MONSTER;
	synMsg.Protocol     = NM_MONSTER_MOVESPEED_SYN;
	synMsg.mMonsterIdx  = mObject.index;
	synMsg.mMoveSpeed   = (unsigned short)GetMoveSpeed();
	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(synMsg) );
}


unsigned long cMonster::HPDamage( sObject attacker, unsigned long damage, bool* die, bool msgSend, bool cri )
{
	if( GetMaxHP() == 0 )
	{
		NETWORK2->PostServerEvent("0 divide 5");
		return 0; //수정함 realDamage;
	}

	/// 도망 조건 체크
	unsigned long beforeHPPer = mHP * 100 / GetMaxHP();	/// 이전 hp %

	/// 실제 적용된 데미지
	unsigned long realDamage = 0;
	*die = false;

	if( IsAutoDummy() == true )
		damage = 1;

	if( GetStateDie() == true )
	{  
		*die = true;
		return realDamage;
	}

	if( mpRegenMonsterInfo && mpRegenMonsterInfo->mFamilyNum != 0 )
		AIMANAGER->FamilyAttack( mObject.index, mpRegenMonsterInfo->mFamilyNum, attacker );

	/// 현재 HP가 공격 받은 데미지 보다 작은경우
	if( mHP <= damage )
	{
		if( attacker.type == eOBJECTTYPE_PLAYER )
			mLastAttacker = attacker.index;

		/// 몬스터HP 0으로 바꾸고 남아있던 HP만큼만 리턴
		realDamage = mHP;
		mHP = 0;
		*die = true;
		ActionChange( eACT_DIE );

		/// 죽을 때 대사 구분
		mCriDie = cri;
	}
	else 
	{ 
		/// 잠들어 있는 상태라면 풀어 준다.
		if( mODDITY[eODDITYTYPE_SLEEP] != 0 )
		{
			cSkillInfluenceSet::cIterator begin = mInfluenceSet.Begin();
			cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

			cInfluenceObject* pInf = NULL;
			unsigned long influenceIdx = 0;
			while( begin != end )
			{
				influenceIdx = (*begin++);
				pInf = SKILLMANAGER->GetInfluence( influenceIdx );
				if( pInf == NULL )
					continue;

				if( pInf->IsSleep() == true )
					SKILLMANAGER->DeleteInfluenceList( influenceIdx );
			}
		}

		/// 데미지를 받으면 풀리는 효과들 삭제하기
		if( mDeleteTypeDamagedAry.GetSize() > 0 )
		{
			cLongAry::cIterator begin = mDeleteTypeDamagedAry.Begin();
			cLongAry::cIterator end = mDeleteTypeDamagedAry.End();

			cInfluenceObject* pInf = NULL;
			unsigned long influenceIdx = 0;
			while( begin != end )
			{
				influenceIdx = (*begin++);
				pInf = SKILLMANAGER->GetInfluence( influenceIdx );
				if( pInf == NULL )
					continue;

				if( pInf->GetDeleteType() == eDELETETYPE_PLAYER_DAMAGED )
					SKILLMANAGER->DeleteInfluenceList( influenceIdx );
			}
		}

		/// 데미지 만큼 감소시키고 데미지량 리턴
		realDamage = damage;
		mHP -= damage;
	}

	/// 공격 성공이면 추적 시간 재설정
	mFollowEndTime = NETWORK2->GetAccumTime() + mpMonsterInfo->mFollowTime;

	/// 도망 조건 체크
	unsigned long afterHPPer = mHP * 100 / GetMaxHP();	/// 현재 hp %

	switch( mpMonsterInfo->mEscapeType )
	{
	case eESCAPETYPE_HP50LOW:
		{
			if( beforeHPPer > ESCAPE_HP50 && afterHPPer < ESCAPE_HP50 )
			{
				unsigned long randSelect = rand() % 100 + 1;
				if( mpMonsterInfo->mEscapePer >= randSelect )
					SetEscape();
			}
		}
		break;
	case eESCAPETYPE_HP30LOW:
		{
			if( beforeHPPer > ESCAPE_HP30 && afterHPPer < ESCAPE_HP30 )
			{
				unsigned long randSelect = rand() % 100 + 1;
				if( mpMonsterInfo->mEscapePer >= randSelect )
					SetEscape();
			}
		}
		break;
	}

	if( msgSend == true )
	{
		MSG_SYN_MONSTER_HP synMsg;
		synMsg.Category			= NM_MONSTER;
		synMsg.Protocol			= NM_MONSTER_HP_SYN;
		synMsg.mMonsterIdx	    = GetObjectID();
		synMsg.mHP				= mHP;
		synMsg.mMaxHP			= GetMaxHP();
		synMsg.mDie				= GetStateDie();

		NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_HP) );
	}

	/// 석상 몬스터 인경우 전장 팀원들에게 메세지 발송
	if( mDMTeamType != ePVPDM_TEAMTYPE_MAX )
	{
		if( NETWORK2->GetInDunMapNum() == MAP_DEATHMATCH )
		{
			cDeathMatchObject* pObject = (cDeathMatchObject*)PVPMANAGER->GetPvPDMObject( mMapNumber );
			if( pObject != NULL )
				pObject->SyncStoneHP( mDMTeamType, mHP );
		}
	}

	return realDamage;
}

void cMonster::HPUse( unsigned long useValue, bool msgSend )
{
	/// 현재 MP가 공격 받은 데미지 보다 작은경우
	if( mHP <= useValue )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster::HPUse [%d,%d,%d]", mObject.index, useValue, mHP );
		mHP = 1;
	}
	else 
		mHP -= useValue;	

	if( msgSend == true )
	{
		MSG_SYN_MONSTER_HP synMsg;
		synMsg.Category			= NM_MONSTER;
		synMsg.Protocol			= NM_MONSTER_HP_SYN;
		synMsg.mMonsterIdx	    = GetObjectID();
		synMsg.mHP				= mHP;
		synMsg.mMaxHP			= GetMaxHP();
		synMsg.mDie				= GetStateDie();

		NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_HP) );
	}
}

void cMonster::HPHeal( unsigned long hp, bool /*msgSend*/, sObject /*healer*/, long /*distressPoint*/, eTAKEDAMAGE_TYPE /*type*/ )
{
	/// 힐을 받은 HP양이 최대치를 넘어 서는 경우
	if( GetMaxHP() < mHP + hp )
	{
		/// hp값을 최대치로 변경
		mHP = GetMaxHP();
	}
	else
	{ 
		/// 힐을 받은 양을 더해줌
		mHP += hp;
	}

	MSG_SYN_MONSTER_HP synMsg;
	synMsg.Category			= NM_MONSTER;
	synMsg.Protocol			= NM_MONSTER_HP_SYN;
	synMsg.mMonsterIdx	    = GetObjectID();
	synMsg.mHP				= mHP;
	synMsg.mMaxHP			= GetMaxHP();
	synMsg.mDie				= GetStateDie();

	NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_HP) );

}



void cMonster::MPDamage( unsigned long damage, bool msgSend )
{
	/// 공격을 받아 MP를 소모하는 경우에 대한 추가 자리

	/// 데미지를 받으면 풀리는 효과들 삭제하기
	if( mDeleteTypeDamagedAry.GetSize() > 0 )
	{
		cLongAry::cIterator begin = mDeleteTypeDamagedAry.Begin();
		cLongAry::cIterator end = mDeleteTypeDamagedAry.End();

		cInfluenceObject* pInf = NULL;
		unsigned long influenceIdx = 0;
		while( begin != end )
		{
			influenceIdx = (*begin++);
			pInf = SKILLMANAGER->GetInfluence( influenceIdx );
			if( pInf == NULL )
				continue;

			if( pInf->GetDeleteType() == eDELETETYPE_PLAYER_DAMAGED )
				SKILLMANAGER->DeleteInfluenceList( influenceIdx );
		}
	}

	MPUse( damage, msgSend );
}

void cMonster::MPUse( unsigned long useValue, bool msgSend )
{
	/// 현재 MP가 공격 받은 데미지 보다 작은경우
	if( mMP <= useValue )
		mMP = 0;
	else 
		mMP -= useValue;	

	if( msgSend == true )
	{
		MSG_SYN_MONSTER_MP synMsg;
		synMsg.Category			= NM_MONSTER;
		synMsg.Protocol			= NM_MONSTER_MP_SYN;
		synMsg.mMonsterIdx	    = GetObjectID();
		synMsg.mMP				= mMP;
		synMsg.mMaxMP			= GetMaxMP();

		NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_MP) );
	}
}


void cMonster::MPHeal( unsigned long heal, bool /*msgSend*/ )
{
	/// 힐을 받은 MP양이 최대치를 넘어 서는 경우
	if( GetMaxMP() < mMP + heal )
	{
		/// mp값을 최대치로 변경
		mMP = GetMaxMP();
	}
	else
	{ 
		/// 힐을 받은 양을 더해줌
		mMP += heal;
	}

	//MSG_SYN_MONSTER_MP synMsg;
	//synMsg.Category			= NM_MONSTER;
	//synMsg.Protocol			= NM_MONSTER_MP_SYN;
	//synMsg.mMonsterIdx	    = GetObjectID();
	//synMsg.mMP				= mMP;
	//synMsg.mMaxMP			= GetMaxMP();

	//NETWORK2->QuickSendExcept( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_MP) );
}



void cMonster::MoveStop( bool sendMsg )
{
	mPathCount = 0;
	mPathIndex = 0;
	mNextPos = mObjectPos;
	mLastPos = mObjectPos;

	if( sendMsg == true )
	{
		/// 이동중이던 클라이언트는 정지 시킨다.
		MSG_SYN_MONSTER_MOVESTOP synMsg;
		synMsg.Category     = NM_MONSTER;
		synMsg.Protocol     = NM_MONSTER_MOVESTOP_SYN;
		synMsg.mMonsterIdx  = mObject.index;
		synMsg.mMonsterPos.x = mObjectPos.x;
		synMsg.mMonsterPos.y = mObjectPos.y;

		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );
	}
}


void cMonster::CalcDirection( NiPoint2 targetPos )
{
	float viewPosX, viewPosY;
	viewPosX = mObjectPos.x - targetPos.x;
	viewPosY = mObjectPos.y - targetPos.y;

	mDirection = atan2( viewPosX, viewPosY );
}


bool cMonster::FindPath( const NiPoint2& goal )
{
	if( mPathFinder == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster::FindPath == NULL");
		return false;
	}

	if( mPathFinder->FindPath( mPathArray, &mPathCount, MAX_PATH_COUNT, NiPoint2(mObjectPos.x, mObjectPos.y), goal ) == false )
		return false;

	mPathIndex = 0;
	mLastPos.x = goal.x;
	mLastPos.y = goal.y;
	mNextPos.x = mPathArray[0].x;
	mNextPos.y = mPathArray[0].y;

	float viewPosX, viewPosY;
	viewPosX = mObjectPos.x - mNextPos.x;
	viewPosY = mObjectPos.y - mNextPos.y;

	mDirection = atan2( viewPosX, viewPosY );

	return true;
}


void cMonster::SetFixedObjectSizePer( short sizePer )
{
	mFixedObjectSizePer = 100 + sizePer;
	if( 100 + sizePer < 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster::GetFixedObjectSize() sizePer < -100");
		sizePer = -100;
	}

	float objectSize = mpMonsterInfo->mMonsterFixSize * mpMonsterInfo->mMonsterScale * ( (float)( 100 + sizePer ) / 100 );
	if( objectSize <= 0.0f )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cMonster::GetFixedObjectSize() <= 0");
		objectSize = 1.0f;
	}

	mFixedObjectSize = objectSize;
}


void cMonster::SendMonsterSize()
{
	MSG_SYN_MONSTER_SCALE synMsg;
	synMsg.Category = NM_MONSTER;
	synMsg.Protocol = NM_MONSTER_SCALE_SYN;
	synMsg.mMonsterIdx = mObject.index;
	synMsg.mScale = GetFixedObjectSizePer();
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(synMsg) );

}


void cMonster::SendSpeech( unsigned long talkState )
{
	if( talkState == eMONSTERTALK_MAX )
	{
		mTalkState = (unsigned char)talkState;
		return;
	}

	sMonsterTalk* pMonsterTalk = MONSTERSCRIPT->GetMonsterTalkInfo( mpMonsterInfo->mMonsterClassIdx, (unsigned char)talkState );
	if( pMonsterTalk == NULL )
		return;

	/// 프로세싱 설정
	switch( talkState )
	{
	case eMONSTERTALK_ATTACK1:	/// 발동하면 기존 대사 프로세싱 하던것을 종료		
	case eMONSTERTALK_ATTACK2:
	case eMONSTERTALK_SKILL1:
	case eMONSTERTALK_SKILL2:
	case eMONSTERTALK_SKILL3:
	case eMONSTERTALK_GIVEUP:	
		mTalkState = eMONSTERTALK_MAX;
		break;
	case eMONSTERTALK_STAND:	/// 프로세싱 해야하는 항목
	case eMONSTERTALK_WORK:
	case eMONSTERTALK_RUN:
	case eMONSTERTALK_FOLLOW:
	case eMONSTERTALK_ESCAPE:
		{
			mTalkState = (unsigned char)talkState;
			/// 다음 체크 시간 설정
			mTalkProcessCheckTime = NETWORK2->GetAccumTime() + pMonsterTalk->mCheckTime;
		}
		break;
	}

	/// TALK_DELAY초간 다른 대화 발송 불가
	if( mTalkDealyEndTime > NETWORK2->GetAccumTime() )
		return;

	if( pMonsterTalk->mViewPercent == 0 )
		return;

	/// 대사 표시 확률 체크
	unsigned long randSelect = rand() % 100 + 1;
	if( pMonsterTalk->mViewPercent < randSelect )
		return;

	/// 선택된 대사 체크
	unsigned char randType = (unsigned char)( rand() % MONSTER_TALK_SIZE );
	unsigned long talkIdx = pMonsterTalk->mTalkIdx[randType];
	if( talkIdx == 0 )
		return;

	/// 메세지 발송
	MSG_SYN_MONSTER_SPEECH synMsg;
	synMsg.Category = NM_MONSTER;
	synMsg.Protocol = NM_MONSTER_SPEECH_SYN;
	synMsg.mMonsterIdx = GetObjectID();
	synMsg.mViewType = pMonsterTalk->mViewType;
	synMsg.mTalkIdx	= talkIdx;
	synMsg.mCharacterIdx = 0;

	/// 확인코드 - 이 경우 외의 타겟 정보는 타겟 선정 방법을 결정한 후에 넣자!
	switch( talkState )
	{
	case eMONSTERTALK_FIGHT:
	case eMONSTERTALK_ATTACK1:
	case eMONSTERTALK_ATTACK2:
	case eMONSTERTALK_SKILL1:
	case eMONSTERTALK_SKILL2:
	case eMONSTERTALK_SKILL3:
	case eMONSTERTALK_DAMAGE:		/// 타겟과 공격자가 틀리더라도 타겟 정보로 발송 한다.
	case eMONSTERTALK_FOLLOW:
	case eMONSTERTALK_ESCAPE:
	case eMONSTERTALK_GIVEUP:
	case eMONSTERTALK_DIE1:
	case eMONSTERTALK_DIE2:
		synMsg.mCharacterIdx = mTarget.index;
		break;
	}

	/// 외치기 설정에 따른 발송 범위 선택
	if( pMonsterTalk->mViewType == eM_TALKVIEW_SHOUT )
		NETWORK2->QuickSendMap( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_SPEECH) );
	else
		NETWORK2->QuickSend( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_SPEECH) );

	/// TALK_DELAY 초간 발송 제약 시간 설정
	mTalkDealyEndTime = NETWORK2->GetAccumTime() + TALK_DELAY;
}


void cMonster::SpeechProcess()
{
	if( mTalkProcessCheckTime > NETWORK2->GetAccumTime() )
		return;

	SendSpeech( mTalkState );
}


void cMonster::AddSummonMonster( unsigned long monsterIdx )
{
	unsigned int modeIdx = 0;
	cHashSet* pSet = 0;

	if( mDeathMonster == true )
	{
		/// 잠수 상태
		pSet = (cHashSet*)mSummonListMap.GetAt( modeIdx );
	}
	else
	{
		/// 일반 제거
		if( mCurrentMode == NULL )
			return;

		modeIdx = mCurrentMode->GetModeIndex();
		pSet = (cHashSet*)mSummonListMap.GetAt( modeIdx );
	}

	if( pSet == 0 )
	{
		pSet = new cHashSet;
		mSummonListMap.Insert( modeIdx, pSet );
	}

	if( pSet == 0 || pSet->Insert( monsterIdx ) == false )
		NETWORK2->PostServerEvent("cMonster::AddSummonMonster[%d] mDeathMonster mSummonListSet.Insert( %d, %d ) == false", mObject.index, monsterIdx, modeIdx );
}

void cMonster::DieSummonMonster( unsigned long monsterIdx, NiPoint2 diePos )
{ 
	unsigned int modeIdx = 0;

	if( mDeathMonster == true )
	{
		/// 잠수 상태
		mObjectPos = diePos;
	}
	else
	{
		/// 일반 제거
		if( mCurrentMode == NULL )
			return;

		modeIdx = mCurrentMode->GetModeIndex();
	}

	cHashSet* pSet = (cHashSet*)mSummonListMap.GetAt( modeIdx );
	if( pSet == NULL )
		return;		

	pSet->Erase( monsterIdx );
}

void cMonster::ModeSkillEnd()
{
	if( mModeInitSkillIdx != eMONSTERATTACK_MAX && mModeInitSkillIdx == mAttackType )
	{
		mModeInitSkillIdx = eMONSTERATTACK_MAX;
	}

	if( mModeSkillIdx != eMONSTERATTACK_MAX && mModeSkillIdx == mAttackType )
	{
		mModeSkillIdx = eMONSTERATTACK_MAX;
	}
}

void cMonster::TargetFixed( unsigned long playerIdx, unsigned long influenceIdx )
{
	mTFixedPlayerIdx = playerIdx;
	mTFixedInfIdx = influenceIdx;

	sObject object = { eOBJECTTYPE_PLAYER, playerIdx };
	TargetChange( object, 0 );
}

void cMonster::TargetFixedEnd()
{
	mTFixedPlayerIdx = 0;
	mTFixedInfIdx = 0;
}

void cMonster::LinkObstruction()
{
	if( NETWORK2->GetServerType() != _E_ST_ID_THEME_ && NETWORK2->GetServerType() != _E_ST_ID_TUTORIAL_ )
		return;

	unsigned long mapNumber = GetMapNumber();
	cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
	if( pTheme == NULL )
		return;

	/// 신규 모드 Init정보 처리
	cModeAgent* pAgent = MONSTERSCRIPT->GetMonsterModeAgent( mpMonsterInfo->mMonsterClassIdx );
	if( pAgent == NULL )
		return;

	cLongAry* pAry = pAgent->GetDeathDoor();
	if( pAry == NULL )
		return;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		unsigned long deathDoorIdx = (*pAry)[i];
		pTheme->DeleteObstruction( deathDoorIdx );
	}
}

void cMonster::DeathGenMonster()
{
	/// 신규 모드 Init정보 처리
	cModeAgent* pAgent = MONSTERSCRIPT->GetMonsterModeAgent( mpMonsterInfo->mMonsterClassIdx );
	if( pAgent == NULL )
		return;

	cPArray* pAry = pAgent->GetDeathMonster();
	if( pAry == NULL )
		return;

	if( pAry->GetSize() == 0 )
		return;

	mDeathMonster = true;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		sModeDeathMonster* pDeathMonster = (sModeDeathMonster*)((*pAry)[i]);
		if( pDeathMonster == NULL )
			continue;

		/// 소환 몬스터 추가
		AIMANAGER->BossSummonMonRegen( this, mMapNumber, mObjectPos.x, mObjectPos.y, pDeathMonster->mGenRangeMin, pDeathMonster->mGenRangeMax,
			pDeathMonster->mCount, pDeathMonster->mGenMonsterIdx );
	}
}

void cMonster::LinkTeleportPortal()
{
	if( NETWORK2->GetServerType() != _E_ST_ID_THEME_ && NETWORK2->GetServerType() != _E_ST_ID_TUTORIAL_ )
		return;

	unsigned long mapNumber = GetMapNumber();
	cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
	if( pTheme == NULL )
		return;

	/// 신규 모드 Init정보 처리
	cModeAgent* pAgent = MONSTERSCRIPT->GetMonsterModeAgent( mpMonsterInfo->mMonsterClassIdx );
	if( pAgent == NULL )
		return;

	cLongAry* pAry = pAgent->GetTeleportPortal();
	if( pAry == NULL )
		return;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		unsigned long mapChangePosIdx = (*pAry)[i];
		pTheme->AddTeleportPortal( mapChangePosIdx );
	}
}

bool cMonster::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	if( GetStateDie() == true )
		return true;

	if( mDMTeamType == ePVPDM_TEAMTYPE_MAX )
	{
		HANDLE            handle  = NULL;
		MSG_MONSTER_INFO* sendMsg = (MSG_MONSTER_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
		if ( sendMsg != NULL )
		{
			sMonsterData*      monsterData       = &sendMsg->mMonsterInfo;
			sMonsterInfluence* pMonsterInfluence = sendMsg->mInfluence;

			monsterData->mMonsterIdx      = mObject.index;
			monsterData->mMonsterClassIdx = mpMonsterInfo->mMonsterClassIdx;
			monsterData->mPosX            = mObjectPos.x;
			monsterData->mPosY            = mObjectPos.y;
			monsterData->mDirection       = mDirection;
			monsterData->mHP			  = mHP;
			monsterData->mMaxHP			  = GetMaxHP();
			monsterData->mMP			  = mMP;
			monsterData->mMaxMP			  = GetMaxMP();
			monsterData->mScale           = mFixedObjectSizePer;
			monsterData->mAutoDummy		  = mAutoDummy;

			sendMsg->mCount = 0;

			cSkillInfluenceSet::cIterator iter = mInfluenceSet.Begin();
			cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

			cInfluenceObject* pInfluence = NULL;

			while( iter != end )
			{
				pInfluence = SKILLMANAGER->GetInfluence( *iter++ );
				if( pInfluence == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("cMonster::SendSightIn pInfluence == NULL" );
					//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );
					continue;
				}

				pMonsterInfluence->mUniqueIdx         = pInfluence->GetUniqueIdx();
				pMonsterInfluence->mInfluenceClassIdx = pInfluence->GetInfluenceClassIdx();
				++pMonsterInfluence;
				++sendMsg->mCount;
			}

			if ( NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() ) )
			{
				//-- TO 캐릭터 FROM 그리드(그리드 이동정보).
				if( GetActionID() == eACT_PUSHPULL )
				{
					HANDLE        handle  = NULL;
					MSG_SYN_SKLL_PUSHPULL* sendMsg = (MSG_SYN_SKLL_PUSHPULL*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_SKILL, NM_SKILL_PUSHPULL_SYN );
					if ( sendMsg != NULL )
					{
						sendMsg->mTarget = mObject;
						sendMsg->mPosX = mNextPos.x;
						sendMsg->mPosY = mNextPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKLL_PUSHPULL) );
					}
				}
				return SendMonsterMove( connectionIdx );
			}
			else
				return false;
		}
	}
	else
	{
		HANDLE            handle  = NULL;
		MSG_MONSTERTEAM_INFO* sendMsg = (MSG_MONSTERTEAM_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, NM_MONSTER_TEAMSIGHT_IN_RES );
		if ( sendMsg != NULL )
		{
			sMonsterData*      monsterData       = &sendMsg->mMonsterInfo;
			sMonsterInfluence* pMonsterInfluence = sendMsg->mInfluence;

			monsterData->mMonsterIdx      = mObject.index;
			monsterData->mMonsterClassIdx = mpMonsterInfo->mMonsterClassIdx;
			monsterData->mPosX            = mObjectPos.x;
			monsterData->mPosY            = mObjectPos.y;
			monsterData->mDirection       = mDirection;
			monsterData->mHP			  = mHP;
			monsterData->mMaxHP			  = GetMaxHP();
			monsterData->mMP			  = mMP;
			monsterData->mMaxMP			  = GetMaxMP();
			monsterData->mScale           = mFixedObjectSizePer;
			monsterData->mAutoDummy		  = mAutoDummy;
			sendMsg->mTeamType            = mDMTeamType;

			sendMsg->mCount = 0;

			cSkillInfluenceSet::cIterator iter = mInfluenceSet.Begin();
			cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

			cInfluenceObject* pInfluence = NULL;

			while( iter != end )
			{
				pInfluence = SKILLMANAGER->GetInfluence( *iter++ );
				if( pInfluence == NULL )
				{
					assert(NULL);
					NETWORK2->PostServerEvent("cMonster::SendSightIn pInfluence == NULL" );
					//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );
					continue;
				}

				pMonsterInfluence->mUniqueIdx         = pInfluence->GetUniqueIdx();
				pMonsterInfluence->mInfluenceClassIdx = pInfluence->GetInfluenceClassIdx();
				++pMonsterInfluence;
				++sendMsg->mCount;
			}

			if ( NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() ) )
			{
				//-- TO 캐릭터 FROM 그리드(그리드 이동정보).
				if( GetActionID() == eACT_PUSHPULL )
				{
					HANDLE        handle  = NULL;
					MSG_SYN_SKLL_PUSHPULL* sendMsg = (MSG_SYN_SKLL_PUSHPULL*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_SKILL, NM_SKILL_PUSHPULL_SYN );
					if ( sendMsg != NULL )
					{
						sendMsg->mTarget = mObject;
						sendMsg->mPosX = mNextPos.x;
						sendMsg->mPosY = mNextPos.y;
						NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_SKLL_PUSHPULL) );
					}
				}
				return SendMonsterMove( connectionIdx );
			}
			else
				return false;
		}
	}

	return false;
}

bool cMonster::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	if( GetStateDie() == true )
		return true;

	HANDLE                handle  = NULL;
	MSG_MONSTER_SIGHTOUT* sendMsg = (MSG_MONSTER_SIGHTOUT*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		//unsigned long* pInfluenceIdx = sendMsg->mInfluenceUniqueIdx;

		sendMsg->mMonsterIdx = mObject.index;
		//sendMsg->mCount      = 0;

		//cSkillInfluenceSet::cIterator iter = mInfluenceSet.Begin();
		//cSkillInfluenceSet::cIterator end = mInfluenceSet.End();

		//cInfluenceObject* pInfluence = NULL;

		//while( iter != end )
		//{
		//	pInfluence = SKILLMANAGER->GetInfluence( *iter++ );

		//	if( pInfluence == NULL )
		//	{
		//		assert(NULL);
		//		NETWORK2->PostServerEvent("cMonster::SendSightOut pInfluence == NULL" );
		//		//-/-//SKILLMANAGER->InsertDeleteInfluenceObject( pInfluence );
		//		continue;
		//	}

		//	(*pInfluenceIdx) = pInfluence->GetUniqueIdx();
		//	++pInfluenceIdx;
		//	++sendMsg->mCount;
		//}

		return NETWORK2->SendMsgRoot( handle, sizeof( MSG_MONSTER_SIGHTOUT ) );
	}
	return false;
}

bool cMonster::SendMonsterMove(unsigned long connectionIdx)
{
	// 061122 PKH 죽지 않은 몬스터만 보낸다.
	if ( GetStateDie() == true )
		return true;

	// 길찾기 경로 개수
	unsigned int pathCount = GetPathCount();

	// 길찾기가 만들어져 있지 않은 경우 다음 몬스터 검색
	if ( pathCount == 0 )
		return true;

	// 이동 경로가 최종목적지 포인트까지 이동이면
	if ( GetMoveStopRange() == 0 )
	{
		HANDLE handle = NULL;
		MSG_SYN_MOVE_MONSTER* msg = (MSG_SYN_MOVE_MONSTER*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_MONSTER, NM_MONSTER_MOVE_SYN );
		if ( msg != NULL )
		{
			msg->mMonsterIdx = GetObjectID();

			for( unsigned int i = 0 ; i < pathCount ; ++i )
				msg->mPathArray[i] = GetPathArray()[i];

			msg->mMoveSpeed = (unsigned short)GetMoveSpeed();
			msg->mMoveSpeedType = (unsigned char)mMoveSpeedType;
			msg->mCount = (unsigned short)pathCount;

			return NETWORK2->SendMsgRoot( handle, msg->GetMsgLength() );
		}
	}
	else	// 이동 경로가 최종목적지 포인트전 RANGE범위까지 인경우
	{
		cBaseObject* player = GetTarget();
		if ( player != NULL )
		{
			sObject target;

			target.index = player->GetObjectID();
			target.type  = player->GetObjectType();

			HANDLE handle = NULL;
			MSG_SYN_ACTIONMOVE_MONSTER* msg = (MSG_SYN_ACTIONMOVE_MONSTER*)NETWORK2->GetMsgRoot( &handle, connectionIdx, NM_MONSTER, NM_MONSTER_ACTIONMOVE_SYN );
			if ( msg != NULL )
			{
				msg->mMonsterIdx = GetObjectID();
				msg->mTarget     = target;
				msg->mRange      = GetMoveStopRange();

				for( unsigned int i = 0 ; i < pathCount ; ++i )
					msg->mPathArray[i] = GetPathArray()[i];

				msg->mMoveSpeed = (unsigned short)GetMoveSpeed();
				msg->mCount = (unsigned short)pathCount;

				return NETWORK2->SendMsgRoot( handle, msg->GetMsgLength() );
			}
		}
	}

	return false;
}


cPlayer* cMonster::NearGridFindPlayer( unsigned long playerIdx, bool diePass )
{
	/// 그리드 좌표를 넣고 검색하는 부분으로 바꿔보자!!
	cPlayer* pPlayer = GRIDMANAGER->FindFirstPlayer( this );
	/// 각각의 플레이어를 검색함
	while( pPlayer != NULL )
	{
		if( diePass == true && pPlayer->GetStateDie() == true )
		{
			pPlayer = GRIDMANAGER->FindNextPlayer();
			continue;
		}

		if( pPlayer->GetObjectID() == playerIdx )
			return pPlayer;

		pPlayer = GRIDMANAGER->FindNextPlayer();
	}

	return NULL;
}


long cMonster::CalcSkillRate( cBaseObject* pTarget, eMONSTERATTACK_TYPE attackType, long scriptRate )
{
	if( pTarget == NULL )
		return 0;

	sMonsterSkillScript* pScript = SKILLSCRIPT->GetMonsterSkillInfo( GetRaceGender(), attackType );
	if( pScript == NULL )
		return 0;

	if( pScript->mAttackSpd == 0 )
		return 0;

	/// 선택 확률 계산
	/// ( (공격력(소) + 공격력(대) ) / 2 ) / ( 공격속도/1000 ) - (절대값)( (대상과의 거리 - 사거리) / 10 ) + 사용확률
	float averageAttack = (float)( pScript->mMinAttackValue + pScript->mMaxAttackValue) / 2;
	float speedAttack = averageAttack / ( (float)pScript->mAttackSpd / 1000 );
	float range = (mObjectPos - pTarget->GetPos()).Length();
	float rangeRate = ( range - pScript->mTargetDist ) / 10;
	float rangeAttack = speedAttack - abs(rangeRate);
	float useRate = rangeAttack + scriptRate;

	if( useRate < 0.0f )
		return 0;

	return (unsigned long)useRate;    
}


tPointerArray<void*>* cMonster::GetBuffImmune()
{
	if( mCurrentMode != NULL )
		return mCurrentMode->GetImmuneModeInfoArr();

	return false;
}


void cMonster::ChangeActionTarget()
{
	if( mTFixedPlayerIdx != 0 )
	{
		cPlayer* pTarget = NearGridFindPlayer( mTFixedPlayerIdx, true );
		if( pTarget == NULL )
			SKILLMANAGER->DeleteInfluence( mObject, mTFixedInfIdx );
		else
		{
			SetTarget( eOBJECTTYPE_PLAYER, mTFixedPlayerIdx );
			return;
		}
	}

	eOBJECTTYPE objectType = eOBJECTTYPE_NONE;
	cBaseObject* target = GetTarget();
	if( target != NULL )
		objectType = (eOBJECTTYPE)target->GetObjectType();
	
	/// Take Damage List를 검사한다.
	unsigned long maxPoint = 0;
	cPlayer* newTarget = 0;
	PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
	while( per )
	{
		cPlayer* player = NearGridFindPlayer( per->playerIdx, true );
		if( player && player != GetTarget() && player->GetStateDie() == false )
		{
			if( objectType == eOBJECTTYPE_MONSTER )
			{
				if( maxPoint <= per->distressPoint )
				{
					maxPoint = per->distressPoint;
					newTarget = player;
				}
			}
			else
			{
				if( maxPoint < per->distressPoint )
				{
					maxPoint = per->distressPoint;
					newTarget = player;
				}
			}
		}
		per = (PerTakeDamage*)per->next;
	}

	if( newTarget )
	{
		/// 타겟 설정
		if( SetTarget( newTarget->GetObjectType(), newTarget->GetObjectID() ) == false )
			return;

		/// 추적 상태 설정
		ActionChange( eACT_ATTACK );
	}
	else
	{
		/// 회기 상태 설정
		ClearTakeDamage();
		ReservationComeback();
	}
}


void cMonster::ChangeTargetDeleteSkill()
{
	if( mTakeDamageRoot.count == 1 )
		return;

	ChangeActionTarget();
}


void cMonster::AddImmuneApplyType( unsigned char InfluenceType, unsigned short applyType )
{ 
	SKILLMANAGER->ClearImmuneApplyType( mObject, InfluenceType, applyType );
	
	if( InfluenceType == eINFLUENCETYPE_BUF ) 
	{
		mImmuneBuffSet.Insert( applyType );
	}
	else if( InfluenceType == eINFLUENCETYPE_DEBUF ) 
	{
		mImmuneDebuffSet.Insert( applyType ); 
	}
	else
		assert(0);
}


void cMonster::AddImmuneKind( unsigned long infKind )
{ 
	SKILLMANAGER->ClearImmuneKind( mObject, infKind );
	mImmuneKindSet.Insert( infKind );	
}


bool cMonster::IsImmuneApplyType( unsigned char InfluenceType, unsigned long applyType )
{
	if( applyType == 0 )
		return false;

	if( IsModeState() == true )
	{
		cPArray* pImmune = GetBuffImmune();
		if( pImmune != NULL )
		{
			for( unsigned int i=0; i<pImmune->GetSize(); i++ )
			{
				sModeInfoBase* p = (sModeInfoBase*)((*pImmune)[i]);
				if( p == NULL )
					continue;

				if( p->type != eMODEINFO_IMMUNE )
					continue;

				sModeImmune* pInfo = (sModeImmune*)p;
				if( pInfo->mSkillApplyType == (unsigned long)applyType )
					return true;

				if( pInfo->mSkillApplyType == 0 )
				{
					switch( applyType )
					{
					case eSTATUSPLUS_STOPMOVE:
					case eSTATUSPLUS_STOPSKILL:
					case eSTATUSPLUS_STOPSKILL_PHY:
					case eSTATUSPLUS_STOPSKILL_MAG:
					case eSTATUSPLUS_ATTACKERTARGETING:
					case eSTATUSPLUS_SLEEP:
					case eSTATUSPLUS_STUN:
						return true;
					}
				}
			}
		}
	}

	/// 면역 목록에서 찾아봐서 있으면 면역
	if( InfluenceType == eINFLUENCETYPE_BUF )
	{
		cHashSet::cIterator i = mImmuneBuffSet.Find( applyType );
		if( i != mImmuneBuffSet.End() )
			return true;
	}
	else if( InfluenceType == eINFLUENCETYPE_DEBUF )
	{
		cHashSet::cIterator i = mImmuneDebuffSet.Find( applyType );
		if( i != mImmuneDebuffSet.End() )
			return true;
	}
	return false;
}

bool cMonster::IsImmuneApplyAll( unsigned long applyType )
{
	if( applyType == 0 )
		return false;

	if( IsModeState() == true )
	{
		cPArray* pImmune = GetBuffImmune();
		if( pImmune != NULL )
		{
			for( unsigned int i=0; i<pImmune->GetSize(); i++ )
			{
				sModeInfoBase* p = (sModeInfoBase*)((*pImmune)[i]);
				if( p == NULL )
					continue;

				if( p->type != eMODEINFO_IMMUNE )
					continue;

				sModeImmune* pInfo = (sModeImmune*)p;
				if( pInfo->mSkillApplyType == (unsigned long)applyType )
					return true;

				if( pInfo->mSkillApplyType == 0 )
				{
					switch( applyType )
					{
					case eSTATUSPLUS_STOPMOVE:
					case eSTATUSPLUS_STOPSKILL:
					case eSTATUSPLUS_STOPSKILL_PHY:
					case eSTATUSPLUS_STOPSKILL_MAG:
					case eSTATUSPLUS_ATTACKERTARGETING:
					case eSTATUSPLUS_SLEEP:
					case eSTATUSPLUS_STUN:
						return true;
					}
				}
			}
		}
	}

	/// 면역 목록에서 찾아봐서 있으면 면역
	cHashSet::cIterator i;
	i = mImmuneBuffSet.Find( applyType );
	if( i != mImmuneBuffSet.End() )
		return true;

	i = mImmuneDebuffSet.Find( applyType );
	if( i != mImmuneDebuffSet.End() )
		return true;
	return false;
}

bool cMonster::IsImmuneKind( unsigned long infKind )
{
	if( infKind == 0 )
		return false;

	/// 면역 목록에서 찾아봐서 있으면 면역
	cHashSet::cIterator b = mImmuneKindSet.Find( infKind );
	cHashSet::cIterator e = mImmuneKindSet.End();

	if( b != e )
		return true;

	return false;

}


unsigned long cMonster::CalcChangeMPDamage( unsigned char attributeType, unsigned long damage, bool applyHP )
{
	unsigned long leftDamage = damage;
	unsigned long calcMP = GetMP();

	if( applyHP == true )
	{
		for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mChangeMPDamageAry[i] );
			if( pInf == NULL || pInf->IsDelReady() == true )
				continue;

			/// 속성확인
			if( ( attributeType == eATTRIBUTETYPE_PHYSICAL && ( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_PHY || pInf->GetChangeMPType() == eINFCHANGEMPTYPE_ALL ) ) ||
				( attributeType == eATTRIBUTETYPE_MAGIC && ( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_MAG || pInf->GetChangeMPType() == eINFCHANGEMPTYPE_ALL ) ) )
			{
				/// 효과에서 데미지mp변환 수치 계산
				unsigned long cutMP = (unsigned long)( GetMaxMP() * pInf->GetChangeMPValue() * 0.01f );
				cutMP = GetMaxMP() - cutMP;

				/// 계산된 현재mp가 효과제한값보다 큰경우만 계산
				if( calcMP > cutMP )
				{
					/// 데미지 적용값이 효과 커트라인 넘는지 체크
					if( cutMP + leftDamage < calcMP  )
					{
						/// 넘지 않는경우 데미지 만큼 계산된현재mp에 적용
						calcMP = calcMP - leftDamage;
						leftDamage = 0;
					}
					else
					{
						/// 넘는 경우 효과데미지mp변환 값까지만 적용
						leftDamage = leftDamage - calcMP - cutMP;
						calcMP = cutMP;

						/// 해당 효과는 지운다.
						SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
					}
				}
				else
				{
					/// 이미 넘는 경우이므로 효과는 지운다.
					SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
				}
			}
		}
	}
	else	/// mp데미지 이므로 데미지는 전부다 적용하고 허용치 밑으로내려간 효과만 전부 지워버린다.
	{
		calcMP = GetMP() - damage;
		leftDamage = damage;

		for( unsigned long i = 0 ; i < mChangeMPDamageAry.GetSize() ; ++i )
		{
			cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mChangeMPDamageAry[i] );
			if( pInf == NULL || pInf->IsDelReady() == true )
				continue;

			/// 속성확인
			if( pInf->GetChangeMPType() == eINFCHANGEMPTYPE_NONE )
				continue;

			/// 효과에서 데미지mp변환 수치 계산
			unsigned long cutMP = (unsigned long)( GetMaxMP() * pInf->GetChangeMPValue() * 0.01f );
			cutMP = GetMaxMP() - cutMP;

			if( calcMP <= cutMP )
				SKILLMANAGER->DeleteInfluenceList( mChangeMPDamageAry[i] );
		}
	}

	return leftDamage;
}


unsigned long cMonster::CalcGuard( unsigned char attributeType, unsigned long damage )
{
	unsigned long tempDamage = damage;
	for( unsigned long i = 0 ; i < mGuardAry.GetSize() ; ++i )
	{
		cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( mGuardAry[i] );
		if( pInf == NULL || pInf->IsDelReady() == true )
			continue;

		if( pInf->CalcLeftGuard( attributeType, &tempDamage ) == false )
			continue;

		if( pInf->GetLeftGuardValue() == 0 || pInf->GetLeftGuardCnt() == 0 )
			SKILLMANAGER->DeleteInfluenceList( mGuardAry[i] );

		if( tempDamage != 0 )
			continue;

		return true;
	}

	return tempDamage;
}


void cMonster::SetCombatFlag( bool set )
{
	if( mIsCombat == set )
		return;

	mIsCombat = set;

	if( mIsCombat == true )
		CombatStart();
	else
		CombatEnd();
}

void cMonster::CombatStart()
{
	/// 전투 시작 시점
	mTotalTime = 0;

	ChangeCurrentMode( 0 );
}

void cMonster::CombatEnd()
{
	/// 전투 종료 시점
	mTotalTime = 0;
	if( mpRegenMonsterInfo != NULL )
		mDestroyTime = mpRegenMonsterInfo->mRegenLifeTime;

	ChangeCurrentMode( -1 );
}

void cMonster::ChangeCurrentMode( long modeIdx )
{
	mIdleEndTime = 0;

	if( mModeAgent == 0 )
		return;

	if( GetStateDie() == true )
		return;

	/// 모드 변수 처리
	if( modeIdx < 0 )
	{
		mCurrentMode = NULL;
	}
	else
	{
		/// 모드 변경전 기존 모드 설정 항목 제거
		if( mCurrentMode )
		{
			cModeInfoArr* arr = mCurrentMode->GetInitModeInfoArr();
			for( unsigned int i=0; i<arr->GetSize(); i++ )
			{
				sModeInfoBase* p = (sModeInfoBase*)((*arr)[i]);
				if( p == NULL )
					continue;

				switch( p->type )
				{
					/// 버프 제거
				case eMODEINFO_APPLYBUFF:
					{
						sModeBuff* pBuffMode = (sModeBuff*)p;
						if( pBuffMode == NULL )
							return;

						if( pBuffMode->mModeDel == true && pBuffMode->mApplyBuffIdx != 0 )
							SKILLMANAGER->DeleteInfluenceClassIdx( mObject, pBuffMode->mApplyBuffIdx );
					}
				case eMODEINFO_APPLYAURA:
					{
						sModeAura* pAuraMode = (sModeAura*)p;
						if( pAuraMode == NULL )
							return;

						if( pAuraMode->mModeDel == true && pAuraMode->mApplyBuffIdx != 0 )
							SKILLMANAGER->DeleteInfluenceClassIdx( mObject, pAuraMode->mApplyBuffIdx );
					}
				}
			}
		}


		cMode* mode = mModeAgent->GetMode( modeIdx );
		//if( mCurrentMode == mode )
		//{
		//	assert(0);
		//	return;
		//}
		mCurrentMode = mode;
	}

	mModeTime = 0;
	mModeSkillIdx = eMONSTERATTACK_MAX;
	mModeInitSkillIdx = eMONSTERATTACK_MAX;
	SetAttackType( eMONSTERATTACK_MAX );

	/// Net Msg처리
	MSG_SYN_MONSTER_MODECHANGE synMsg;
	synMsg.Category = NM_MONSTER;
	synMsg.Protocol = NM_MONSTER_MODECHANGE_SYN;
	synMsg.mMonsterIdx = mObject.index;
	synMsg.mModeIdx = (unsigned char)modeIdx;
	NETWORK2->QuickSend( this, (char*)&synMsg, sizeof( synMsg ) );

	/// 모드 시작 설정 처리
	if( mCurrentMode )
	{
		cModeInfoArr* arr = mCurrentMode->GetInitModeInfoArr();
		for( unsigned int i=0; i<arr->GetSize(); i++ )
		{
			sModeInfoBase* p = (sModeInfoBase*)((*arr)[i]);
			if( p == NULL )
				continue;

			switch( p->type )
			{
			case eMODEINFO_APPLYBUFF:
				{
					sModeBuff* pBuffMode = (sModeBuff*)p;
					if( pBuffMode == NULL )
						return;

					if( pBuffMode->mApplyBuffIdx != 0 )
						SKILLMANAGER->AddInfluence( mObject, mObject, pBuffMode->mApplyBuffIdx, 0, true );
				}
				break;
			case eMODEINFO_APPLYAURA:
				{
					sModeAura* pAuraMode = (sModeAura*)p;
					if( pAuraMode == NULL )
						return;

					if( pAuraMode->mApplyBuffIdx != 0 )
					{
						SKILLMANAGER->AddInfMode( mObject, mObject, pAuraMode->mApplyBuffIdx, pAuraMode->mEnemyApply,
							pAuraMode->mRange, true );
					}
				}
				break;
			case eMODEINFO_GENMONSTER:
				{
					sModeGenMonster* pMonMode = (sModeGenMonster*)p;
					if( pMonMode == NULL )
						return;

					/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
					bool pattern = false;
					if( pMonMode->mGenPatternIdx != 0 )
					{
						cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pMonMode->mGenPatternIdx );
						if( pAryScript != NULL && pAryScript->GetSize() > 0 )
						{
							sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
							if( pScript != NULL )
							{
								pattern = true;
								//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
								//pPattern->mObject.index = pMonMode->mGenMonsterIdx;
								//pPattern->mObject.type = eOBJECTTYPE_MONSTER;
								//pPattern->mCount = pMonMode->mCount;
								//pPattern->mSummonPatternIdx = pMonMode->mGenPatternIdx;
								//pPattern->mCenterPos = mObjectPos;
								//pPattern->mCenterDir = mDirection;
								//pPattern->mRangeMin = pMonMode->mGenRangeMin;
								//pPattern->mRangeMax = pMonMode->mGenRangeMax;
								//pPattern->mNowPatternPos = 0;
								//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

								//mSummonPatternInfoAry.PushBack( pPattern );

								sObject object = { eOBJECTTYPE_MONSTER, pMonMode->mGenMonsterIdx };
								SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pMonMode->mGenPatternIdx, pMonMode->mCount,
									mObjectPos, mDirection, pMonMode->mGenRangeMin, pMonMode->mGenRangeMax, 0, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );
							}
							else
								NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
						}
						else
							NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
					}

					/// 패턴이 없는경우 랜덤위치로 소환
					if( pattern == false )
					{
						/// 소환 몬스터 추가
						AIMANAGER->BossSummonMonRegen( this, mMapNumber, mObjectPos.x, mObjectPos.y, pMonMode->mGenRangeMin, pMonMode->mGenRangeMax,
							pMonMode->mCount, pMonMode->mGenMonsterIdx );
					}

					/// 루프를 도는 것만 쿨타임 정보를 기록해서 몬스터process시 추가 리젠한다.
					if( pMonMode->mLoop == true )
					{
						unsigned long nextTime = NETWORK2->GetAccumTime() + pMonMode->mDelayTime;

						/// 등록된 정보를 찾아 추가/변경
						cHashMap::cIterator find = mGenMonCoolTimeMap.Find( pMonMode->mOrderNum );
						if( find == mGenMonCoolTimeMap.End() )
							mGenMonCoolTimeMap.Insert( pMonMode->mOrderNum, nextTime );							
						else
							(*find).mSecond = nextTime;
					}							
				}
				break;
			case eMODEINFO_GENTOTEM:
				{
					//bool			mLoop;
					//unsigned long	mDelayTime;

					sModeGenTotem* pTotemMode = (sModeGenTotem*)p;
					if( pTotemMode == NULL )
						return;

					/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
					bool pattern = false;
					if( pTotemMode->mGenPatternIdx != 0 )
					{
						cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pTotemMode->mGenPatternIdx );
						if( pAryScript != NULL && pAryScript->GetSize() > 0 )
						{
							sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
							if( pScript != NULL )
							{
								pattern = true;
								//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
								//pPattern->mObject.index = pTotemMode->mGenTotemIdx;
								//pPattern->mObject.type = eOBJECTTYPE_TOTEM;
								//pPattern->mCount = pTotemMode->mCount;
								//pPattern->mSummonPatternIdx = pTotemMode->mGenPatternIdx;
								//pPattern->mCenterPos = mObjectPos;
								//pPattern->mCenterDir = mDirection;
								//pPattern->mRangeMin = pTotemMode->mGenRangeMin;
								//pPattern->mRangeMax = pTotemMode->mGenRangeMax;
								//pPattern->mNowPatternPos = 0;
								//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

								//mSummonPatternInfoAry.PushBack( pPattern );

								sObject object = { eOBJECTTYPE_TOTEM, pTotemMode->mGenTotemIdx };
								SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pTotemMode->mGenPatternIdx, pTotemMode->mCount,
									mObjectPos, mDirection, pTotemMode->mGenRangeMin, pTotemMode->mGenRangeMax, pTotemMode->mApplyType, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );

							}
							else
								NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
						}
						else
							NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
					}

					/// 패턴이 없는경우 랜덤위치로 소환
					if( pattern == false )
					{
						unsigned long genRangeMin = pTotemMode->mGenRangeMin;
						unsigned long genRangeMax = pTotemMode->mGenRangeMax;

						for( unsigned char i = 0 ; i < pTotemMode->mCount ; ++i )
						{
							NiPoint2 pos = CalcMinMaxRandPos( mObjectPos.x, mObjectPos.y, genRangeMin, genRangeMax );

							cTotem* pTotem = OBJECTMANAGER->AddTotem( pTotemMode->mGenTotemIdx, 0, 0, pos, mObject, 
								(eAPPLYTYPE)pTotemMode->mApplyType, eATTRIBUTETYPE_PHYSICAL, 0, mDirection, 0 );
							if( pTotem != NULL )
								GRIDMANAGER->AddTotem( pTotem );
						}
					}

					/// 루프를 도는 것만 쿨타임 정보를 기록해서 몬스터process시 추가 리젠한다.
					if( pTotemMode->mLoop == true )
					{
						unsigned long nextTime = NETWORK2->GetAccumTime() + pTotemMode->mDelayTime;

						/// 등록된 정보를 찾아 추가/변경
						cHashMap::cIterator find = mGenTotemCoolTimeMap.Find( pTotemMode->mOrderNum );
						if( find == mGenTotemCoolTimeMap.End() )
							mGenTotemCoolTimeMap.Insert( pTotemMode->mOrderNum, nextTime );							
						else
							(*find).mSecond = nextTime;
					}	
				}
				break;
			case eMODEINFO_GENGATHERING:
				{
					//bool			mLoop;
					//unsigned long	mDelayTime;

					sModeGenGathering* pGatheringMode = (sModeGenGathering*)p;
					if( pGatheringMode == NULL )
						return;

					/// 소환 패턴이 있는경우 패턴대로 생성 시켜 준다.
					bool pattern = false;
					if( pGatheringMode->mGenPatternIdx != 0 )
					{
						cArray* pAryScript = MONSTERSCRIPT->GetSummonPattern( pGatheringMode->mGenPatternIdx );
						if( pAryScript != NULL && pAryScript->GetSize() > 0 )
						{
							sSummonPatternScript* pScript = (sSummonPatternScript*)(*pAryScript)[0];
							if( pScript != NULL )
							{
								pattern = true;
								//sSummonPatternInfo* pPattern = new sSummonPatternInfo;
								//pPattern->mObject.index = pGatheringMode->mGenGatheringIdx;
								//pPattern->mObject.type = eOBJECTTYPE_GATHERING;
								//pPattern->mCount = pGatheringMode->mCount;
								//pPattern->mSummonPatternIdx = pGatheringMode->mGenPatternIdx;
								//pPattern->mCenterPos = mObjectPos;
								//pPattern->mCenterDir = mDirection;
								//pPattern->mRangeMin = pGatheringMode->mGenRangeMin;
								//pPattern->mRangeMax = pGatheringMode->mGenRangeMax;
								//pPattern->mNowPatternPos = 0;
								//pPattern->mNextPatternTime = pScript->mDelayTime + NETWORK2->GetAccumTime();

								//mSummonPatternInfoAry.PushBack( pPattern );

								sObject object = { eOBJECTTYPE_GATHERING, pGatheringMode->mGenGatheringIdx };
								SUMMONPATTERNPOOL->AddSummonPattern( &mSummonPatternRoot, object, pGatheringMode->mGenPatternIdx, pGatheringMode->mCount,
									mObjectPos, mDirection, pGatheringMode->mGenRangeMin, pGatheringMode->mGenRangeMax, 0, 0, pScript->mDelayTime + NETWORK2->GetAccumTime() );

							}
							else
								NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pScript != NULL");
						}
						else
							NETWORK2->PostServerEvent("cMonster::ChangeCurrentMode pAryScript != NULL && pAryScript->GetSize > 0");
					}

					/// 패턴이 없는경우 랜덤위치로 소환
					if( pattern == false )
					{
						unsigned long genRangeMin = pGatheringMode->mGenRangeMin;
						unsigned long genRangeMax = pGatheringMode->mGenRangeMax;

						for( unsigned char i = 0 ; i < pGatheringMode->mCount ; ++i )
						{
							NiPoint2 pos = CalcMinMaxRandPos( mObjectPos.x, mObjectPos.y, genRangeMin, genRangeMax );

							cGathering* pGathering = OBJECTMANAGER->AddGathering( 0, pGatheringMode->mGenGatheringIdx, mMapNumber, pos.x, pos.y, mDirection, 0 );
							if( pGathering != NULL )
							{
								GRIDMANAGER->AddGathering( pGathering );
								pGathering->SendRegenSync();
								mSummonGatherAry.PushBack( pGathering->GetObjectID() );
							}
						}
					}

					/// 루프를 도는 것만 쿨타임 정보를 기록해서 몬스터process시 추가 리젠한다.
					if( pGatheringMode->mLoop == true )
					{
						unsigned long nextTime = NETWORK2->GetAccumTime() + pGatheringMode->mDelayTime;

						/// 등록된 정보를 찾아 추가/변경
						cHashMap::cIterator find = mGenTotemCoolTimeMap.Find( pGatheringMode->mOrderNum );
						if( find == mGenTotemCoolTimeMap.End() )
							mGenTotemCoolTimeMap.Insert( pGatheringMode->mOrderNum, nextTime );							
						else
							(*find).mSecond = nextTime;
					}	
				}
				break;
				//case eMODEINFO_SPEECH:
				//	{
				//		//sModeSpeech* pSpeechMode = (sModeSpeech*)p;
				//		//if( pSpeechMode == NULL )
				//		//	return;

				//		///// 메세지 발송
				//		//MSG_SYN_MOSTER_MODESPEECH synMsg;
				//		//synMsg.Category = NM_MONSTER;
				//		//synMsg.Protocol = NM_MONSTER_MODESPEECH_SYN;
				//		//synMsg.mMonsterIdx = GetObjectID();
				//		//synMsg.mTalkIdx	= pSpeechMode->mSpeechIdx;
				//		//synMsg.mCharacterIdx = mTarget.index;

				//		//NETWORK2->QuickSendMap( this, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_SPEECH) );
				//	}
				break;
			case eMODEINFO_MODESTATE:
				{
					sModeState* pIdleMode = (sModeState*)p;
					if( pIdleMode == NULL )
						return;

					mIdleEndTime = NETWORK2->GetAccumTime() + pIdleMode->mMaintainTime;
					ActionChange( eACT_IDLE );
				}
				break;
			case eMODEINFO_AGGRO:
				{
					/// 자신의 어그로 초기화
					ClearDistressPoint();
				}
				break;
			case eMODEINFO_MODEINITSKILL:
				{
					cModeInfoArr* skillArr = mCurrentMode->GetCheckSkillModeInfoArr();
					for( unsigned int i=0; i<skillArr->GetSize(); i++ )
					{
						sModeInfoBase* p = (sModeInfoBase*)((*skillArr)[i]);
						if( p == NULL )
							continue;

						if( p->type != eMODEINFO_USESKILL )
						{
							/// error
							assert(0);
							continue;
						}

						sModeSkill* pInfo = (sModeSkill*)p;
						mModeInitSkillIdx = (eMONSTERATTACK_TYPE)pInfo->mSkillIdx;
					}

					/*
					/// 모드 진입시 조건가능하면 무조건 선정
					cHashMap successSkill;
					unsigned long sumRate = 0;

					cBaseObject* pTarget = GetTarget();
					if( pTarget == NULL )
					break;

					/// 선택 가능 스킬목록 작성
					cModeInfoArr* skillArr = mCurrentMode->GetCheckSkillModeInfoArr();
					for( unsigned int i=0; i<skillArr->GetSize(); i++ )
					{
					sModeInfoBase* p = (sModeInfoBase*)((*skillArr)[i]);
					if( p->type != eMODEINFO_USESKILL )
					{
					/// error
					assert(0);
					continue;
					}

					sModeSkill* pInfo = (sModeSkill*)p;

					/// 쿨타임 체크
					cCoolTime::cIterator begin = mEliteCoolTimeMap.Find( pInfo->mSkillIdx );
					cCoolTime::cIterator end = mEliteCoolTimeMap.End();
					if( begin != end )
					{
					if( (*begin).mSecond > NETWORK2->GetAccumTime() )
					continue;
					}

					/// 선택 비율값 계산
					long useRate = CalcSkillRate( pTarget, (eMONSTERATTACK_TYPE)pInfo->mSkillIdx, (long)pInfo->mActionPer );

					/// 가능한 스킬 목록에 저장
					successSkill.Insert( pInfo->mSkillIdx, useRate );
					sumRate = sumRate + useRate;
					}

					/// 가능한 스킬이 없으면 통과
					if( sumRate == 0 )
					break;

					/// 가능한 스킬중 선택
					unsigned long selectRand = rand() % sumRate;
					cHashMap::cIterator sI = successSkill.Begin();                                      
					cHashMap::cIterator sE = successSkill.End();
					for( ; sI != sE ; ++sI )
					{
					unsigned long rate = (*sI).mSecond;
					if( selectRand < rate )
					{
					mModeSkillIdx = (eMONSTERATTACK_TYPE)(*sI).mFirst;
					break;
					}

					selectRand = selectRand - rate;
					}
					*/
				}
				break;	
			default:
				assert(0);
				break;
			}
		}
	}
}

bool cMonster::ConditionValueCheck( unsigned char type, unsigned long value )
{
	switch( type )
	{
	case 0: 
		return true;
	case 1: // total time
		{
			if( value <= mTotalTime )
				return true;
		}
		break;
	case 2: // mode time
		{
			if( value <= mModeTime )
				return true;
		}
		break;
	case 3: // monster hp
		{
			if( GetMaxHP() == 0 )
			{
				NETWORK2->PostServerEvent("0 divide 6");
				return false;
			}

			unsigned long per = GetHP() * 100 / GetMaxHP();
			if( value >= per )
				return true;
		}
		break;
	case 4: // regenMonster die
		{
			/// 자신의모드에서 소환된 몬스터가 모두 죽었으면 모드 변경
			if( mCurrentMode != NULL )
			{
				unsigned long modeIdx = mCurrentMode->GetModeIndex();
				cHashSet* pSet = (cHashSet*)mSummonListMap.GetAt( modeIdx );
				if( pSet == NULL )
					return true;

				if( pSet->IsEmpty() == true )
					return true;
			}

		}
		break;
	case 5:	// mode skill use
		{
			/// 사용한 스킬을 체크해서 변환조건과 같으면 모드 변경
			if( mLastUsedSkillIdx == (eMONSTERATTACK_TYPE)value )
			{
				mLastUsedSkillIdx = eMONSTERATTACK_MAX;
				return true;
			}
		}
		break;
	default:
		assert(0);
		break;
	}
	return false;
}

eMONSTERATTACK_TYPE	cMonster::GetModeSkill()
{
	if( mModeInitSkillIdx != eMONSTERATTACK_MAX )
		return mModeInitSkillIdx;

	return mModeSkillIdx;
}

bool cMonster::IsFollowContinue()
{
	if( mpMonsterInfo == 0 )
	{
		NETWORK2->PostServerEvent("cMonster::CombatContinue mpMonsterInfo == NULL");
		return false;
	}

	cRangeCheck rangechk;
	rangechk.SetRadius( (float)mpMonsterInfo->mFollowRange );

	cPlayer* newTarget = 0;
	PerTakeDamage* per = (PerTakeDamage*)mTakeDamageRoot.pool;
	while( per )
	{
		cPlayer* player = NearGridFindPlayer( per->playerIdx, true );
		if( player && player != GetTarget() && player->GetStateDie() == false )
		{
			if( rangechk.IsRange( GetFollowStartPos(), player->GetPos() ) )
			{
				newTarget = player;
				break;
			}
		}
		per = (PerTakeDamage*)per->next;
	}

	if( newTarget )
	{
		NiPoint2 dir = GetPos() - GetFollowStartPos();
		dir.Unitize();
		mFollowStartPos = mFollowStartPos + dir*5.0f;

		SetTarget( newTarget->GetObjectType(), newTarget->GetObjectID() );
		return true;
	}

	return false;
}

void cMonster::ProcessTakeDamage( unsigned long elapsedTime, unsigned long /*accumTime*/ )
{
	/// 어그로 감소 루틴..
	TakeDamageRoot* pTakeDamageRoot = GetTakeDamageRoot();
	if( pTakeDamageRoot == 0 )
		return;
	if( pTakeDamageRoot->count == 0 )
		return;

	PerTakeDamage* per = (PerTakeDamage*)pTakeDamageRoot->pool;
	while( per )
	{
		if( per->changeFlag == false )
			per->noChangeTime += elapsedTime;

		per->changeFlag = false;

		if( per->distressPoint > 1 )
		{
			if( per->noChangeTime >= 8000 )
			{
				///
				unsigned long decrease = (unsigned long)(elapsedTime * 0.05f);
				if( decrease < 1 )
					decrease = 1;

				if( per->distressPoint <= decrease )
					per->distressPoint = 1;
				else
					per->distressPoint -= decrease;
			}
		}
		per = (PerTakeDamage*)per->next;
	}
}

unsigned long cMonster::GetMaxHP()
{ 
	if( IsAutoDummy() == true )
		return 9999;
	return (unsigned long)( mpMonsterInfo->mMaxHp + ( mpMonsterInfo->mMaxHp * mStatusPer.mMaxHP ) + mStatusPlus.mMaxHP );
}

bool cMonster::IsHaveApplyBuffType( unsigned short applyValueType )
{
	cApplyValueTypeMap::cIterator findPos = mBuffmap.Find( applyValueType );
	if(  findPos != mBuffmap.End() )
	{
		/// 이미 들어가 있는 경우
		cHashSet& influenceSet = (*findPos).mSecond;
		if( influenceSet.IsEmpty() == false )
		{	
			/// 해당 스킬이 유효한지 검사는 생략 중
			return true;
		}
	}

	return false;
}

bool cMonster::IsHaveApplyDeBuffType( unsigned short applyValueType )
{
	cApplyValueTypeMap::cIterator findPos = mDeBuffmap.Find( applyValueType );
	if(  findPos != mDeBuffmap.End() )
	{
		/// 이미 들어가 있는 경우
		cHashSet& influenceSet = (*findPos).mSecond;
		if( influenceSet.IsEmpty() == false )
		{	
			/// 해당 스킬이 유효한지 검사는 생략 중
			return true;
		}
	}

	return false;
}

void cMonster::AddDistressPer( sObject attacker, long distressPointPer, eTAKEDAMAGE_TYPE type )
{
	unsigned long playerIdx = attacker.index;
	
	/// damage List에 등록 또는 갱신
	PerTakeDamage* perTakeDamage = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, playerIdx );
	if( perTakeDamage  == 0 )
	{
		assert(0);
		AddTakeDamage( attacker, 0, 0, type );
	}
	else
	{
		long distressPoint = perTakeDamage->distressPoint;
		distressPoint = long( ( distressPoint * distressPointPer ) / 100 );

		AddTakeDamage( attacker, 0, distressPoint, type );
	}
}

void cMonster::SwapDistressPoint( unsigned long attackerIdx, unsigned long targetIdx )
{
 	long attakerDistressPoint = -1;
 	long targetDistressPoint  = -1;

	/// damage List에 검색
	PerTakeDamage* perTakeDamage = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, attackerIdx );
	if( perTakeDamage != 0 )
	{
		/// 공격자에 대한 어그로가 있다면 값을 백업 시킨 후 초기화 시킨다.
		attakerDistressPoint = perTakeDamage->distressPoint;
		// = 0 인 경우 ChangeActionTarget() 에서 새로운 타겟팅을 할때 기존 유저를 무시한다. = 1인경우 ChangeActionTarget()에서 원래 유저를 타겟팅 시도한다.
		perTakeDamage->distressPoint = 1; 
	}

	perTakeDamage = TAKEDAMAGEPOOL->GetTakeDamage( &mTakeDamageRoot, targetIdx );
	if( perTakeDamage != 0 )
	{
		/// 타겟에 대한 어그로가 있다면 값을 백업 시킨 후 초기화 시킨다.
		targetDistressPoint = perTakeDamage->distressPoint;
		// = 0 인 경우 ChangeActionTarget() 에서 새로운 타겟팅을 할때 기존 유저를 무시한다. = 1인경우 ChangeActionTarget()에서 원래 유저를 타겟팅 시도한다.
		perTakeDamage->distressPoint = 1;
	}

	if( attakerDistressPoint < 0 && targetDistressPoint < 0 )
	{	
		/// 두 플레이어 모두 공격자 명단에 들어가있지 않다면 교체해야 할 대상이 잘못된것이다.
		assert(0);
	}
	else if( attakerDistressPoint >= 0 && targetDistressPoint < 0 )
	{
		/// 공격자의 어그로 정보만 있는 경우, 타겟에 공격자의 어그로 정보를 입력한다.
		sObject target = { eOBJECTTYPE_PLAYER, targetIdx };
		AddTakeDamage( target, 0, attakerDistressPoint, eTAKEDAMAGETYPE_ADDPLAYER );
	}
	else if( attakerDistressPoint < 0 && targetDistressPoint >= 0 )
	{
		/// 타겟의 어그로 정보만 있는 경우, 공격자에 타겟의 어그로 정보를 입력한다.
		sObject attacker = { eOBJECTTYPE_PLAYER, attackerIdx };
		AddTakeDamage( attacker, 0, targetDistressPoint, eTAKEDAMAGETYPE_ADDPLAYER );
	}
	else if( attakerDistressPoint >= 0 && targetDistressPoint >= 0 )
	{
		/// 두 플레이어 모두 어그로 정보가 있는 경우, 서로에게 어그로 정보를 입력한다.
		sObject attacker = { eOBJECTTYPE_PLAYER, attackerIdx };
		sObject target = { eOBJECTTYPE_PLAYER, targetIdx };
		AddTakeDamage( attacker, 0, targetDistressPoint, eTAKEDAMAGETYPE_ADDPLAYER );
		AddTakeDamage( target, 0, attakerDistressPoint, eTAKEDAMAGETYPE_ADDPLAYER );
	}
	else
		assert(0);
}
