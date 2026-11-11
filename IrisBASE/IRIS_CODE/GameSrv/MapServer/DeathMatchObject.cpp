#include "stdafx.h"
#include "gamesrv.h"
#include "DeathMatchObject.h"
#include "PVPManager.h"
#include "Protocol.h"
#include "LevelScript.h"
#include "PvPScript.h"
#include "Totem.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "StageScript.h"
#include "GatheringScript.h"
#include "Gathering.h"
#include "GridManager.h"
#include "MonsterScript.h"
#include "AIManager.h"
#include "Monster.h"


cDeathMatchObject::cDeathMatchObject() : cBaseDeathMatch( ePVPDM_OBJECTTYPE_1 )
{
	mLeaderTimePass = 0;
	mNextLogTime = 0;
}


cDeathMatchObject::~cDeathMatchObject()
{
	for( unsigned long i = 0 ; i < ePVPDM_TEAMTYPE_MAX ; ++i )
	{
		if( mStoneInfo[i].mStoneIdx != 0 )
		{
			cMonster* pMonObject = OBJECTMANAGER->GetMonster( mStoneInfo[i].mStoneIdx );
			if( pMonObject != NULL )
			{
				pMonObject->Release();
				GRIDMANAGER->RemoveMonster( pMonObject );
			}
			OBJECTMANAGER->RemoveMonster( mStoneInfo[i].mStoneIdx );	
		}
	}
}

void* cDeathMatchObject::operator new( size_t n )
{
	if( n != sizeof(cDeathMatchObject) ) 
	{
		NETWORK2->PostServerEvent("cDeathMatchObject::operator new %d != sizeof(cDeathMatchObject)", n );
		return NULL;
	}

	return PVPOBJECTPOOL->NewDMObject();
}


void cDeathMatchObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사



	if( ptr == 0 )
	{
		NETWORK2->PostServerEvent("cDeathMatchObject::operator delete ptr == 0" );
		return;
	}

	if( n != sizeof(cDeathMatchObject) ) 
	{
		NETWORK2->PostServerEvent("cDeathMatchObject::operator delete n != sizeof(cDeathMatchObject", n );
		return;
	}

	//PVPOBJECTPOOL->FreeDMObject( static_cast<cDeathMatchObject*>(ptr) );

	return;
}

bool cDeathMatchObject::Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo )
{
	memset( mLeaderInfo, 0, sizeof( mLeaderInfo ) );
	memset( mStoneInfo, 0, sizeof( mStoneInfo ) ); 

	return cBaseDeathMatch::Init( DMIdx, pLevelForceInfo );
}


void cDeathMatchObject::Process( unsigned long elapsedTime, unsigned long accumTime )
{
	/// 방에 아무도 없으면 객체 소멸
	/// 마지막 유저가 나가는 시점에 삭제 불가능 했던 이유 체크해보자!!
	if( mTeamInfo[ePVPDM_TEAMTYPE_A].mMemberCnt + mTeamInfo[ePVPDM_TEAMTYPE_B].mMemberCnt == 0 && ePVPDM_PROCESS_READY < mProcess )
		mProcess = ePVPDM_PROCESS_END;

	if( mScoreEnd == true )
	{
		mScoreEnd = false;
		Result();
	}

	switch( mProcess )
	{
	case ePVPDM_PROCESS_WAIT:		/// Ready() 호출 기다림
		{
			if( mWaitEndTime > accumTime )
				return;

			Ready();
		}
		break;
	case ePVPDM_PROCESS_READY:
		{
			/// PvP 대기시간


			if( mProcessEndTime > accumTime )
				return;

			EndNpcTalk();

			unsigned long fightTime = 0;
			sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
			if( pInfo != NULL )
			{
				/// ePVPDM_PROCESS_FIGHT_MINIMUM 에서 나머지 잔여시간을 다시 설정
				fightTime = pInfo->mTotalDMTime * MINUTE;		/// 총전투시간

				/// 총전투시간보다 인원부족체크 안함 시간이 큰경우


				if( PVP_DM_TEAM_MINCNT_TIME > fightTime )	
					mProcessEndTime = accumTime + fightTime;	 /// 다음프로세스 수행시간을 스크립트시간으로
				else	/// 총전투시간보다 인원부족 시간이 작은경우
					mProcessEndTime = accumTime + PVP_DM_TEAM_MINCNT_TIME;	/// 다음프로세스 수행시간을 최소인원 체크안함시간으로
			}

			/// 클라이언트에는 총전투시간을 알려줌


			SendFight( fightTime );

			/// pvp 전투 시작 프로세스로 이동
			mProcess = ePVPDM_PROCESS_FIGHT_MINIMUM;

			///
			cTotem* pTotem = NULL;
			NiPoint2 centerPos;
			for( unsigned char i=0;i<ePVPDM_TEAMTYPE_MAX; ++i )
			{
				unsigned long posIdx = PVPMANAGER->GetTeamGenPosIdx( (ePVPDM_TEAM_TYPE)i );
				if( posIdx != 0 )
				{
					sStageChangePos* pStagePos = STAGESCRIPT->GetPosScriptInfo( posIdx );
					if( pStagePos != 0 )
					{
						centerPos.x = pStagePos->mPosX;
						centerPos.y = pStagePos->mPosY;
						pTotem = OBJECTMANAGER->PvPAddTotem( i, PVP_DM_ADD_TOTEMIDX_BUFF, mDMIdx, centerPos, eAPPLYTYPE_BUDDY, eATTRIBUTETYPE_PHYSICAL );
						if( pTotem != NULL )
						{
							GRIDMANAGER->AddTotem( pTotem );
							mDMTotemArr.PushBack(pTotem->GetObjectID());
						}
						pTotem = OBJECTMANAGER->PvPAddTotem( i, PVP_DM_ADD_TOTEMIDX_DEBUFF, mDMIdx, centerPos, eAPPLYTYPE_ENEMY, eATTRIBUTETYPE_PHYSICAL );
						if( pTotem != NULL )
						{
							GRIDMANAGER->AddTotem( pTotem );
							mDMTotemArr.PushBack(pTotem->GetObjectID());
						}
					}
				}
			}

			AddTeleportPortal();
		}
		break;
	case ePVPDM_PROCESS_READY_OUT:
		{
			/// pvp out 요청 받는 시간
			if( mProcessEndTime > accumTime )
				return;

			/// 오브젝트 제거
			mProcess = ePVPDM_PROCESS_END;
		}
		break;
	case ePVPDM_PROCESS_FIGHT_MINIMUM:
		{
			LeaderProcess( elapsedTime, accumTime );
			StoneProcess( elapsedTime, accumTime );

			/// 일정 시간동안 인원부족 결투 종료 체크 안함
			if( mProcessEndTime <= accumTime )
			{
				mProcess = ePVPDM_PROCESS_FIGHT;

				mProcessEndTime = accumTime + 0;
				sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
				if( pInfo != NULL )
				{
					/// 인원체크 안함 시간보다 총전투시간이 큰경우 나머지 잔여시간을 설정해줌
					if( PVP_DM_TEAM_MINCNT_TIME < pInfo->mTotalDMTime * MINUTE )
						mProcessEndTime = accumTime + ( pInfo->mTotalDMTime * MINUTE - PVP_DM_TEAM_MINCNT_TIME );

					for( int i = ePVPDM_TEAMTYPE_A ; i < ePVPDM_TEAMTYPE_MAX ; ++i )
					{
						if( mTeamInfo[i].mMemberCnt < pInfo->mTeamPlayerMin )
						{
							Result();
							return;
						}
					}
				}
			}
		}
		break;
	case ePVPDM_PROCESS_FIGHT:
		{
			LeaderProcess( elapsedTime, accumTime );
			StoneProcess( elapsedTime, accumTime );

			/// 시간 종료
			if( mProcessEndTime <= accumTime )
				Result();
		}
		break;
	case ePVPDM_PROCESS_FIGHTEND:
		{
			/// pvp out 요청 받는 시간
			if( mProcessEndTime <= accumTime )
				mProcess = ePVPDM_PROCESS_END;
		}
		break;
	case ePVPDM_PROCESS_END:
		{
			/// 전체 유저 추방
			if( mAutoOutCheckTime < accumTime )
			{
				if( OrderEndDM() == true )
				{
					EndDM();
					PVPMANAGER->DelAryInsertPVP( mDMIdx );
				}
			}
		}
		break;
	}
}


void cDeathMatchObject::LeaderProcess( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( mLeaderTimePass < accumTime )
		mLeaderTimePass = accumTime + 200;

	/// 리더 선정
	for( int teamType = ePVPDM_TEAMTYPE_A ; teamType < ePVPDM_TEAMTYPE_MAX ; ++teamType )
	{
		/// 시간 체크
		if( mLeaderInfo[teamType].mLeaderChangeTime >= accumTime )
			continue;

		unsigned long memberIdx[PVP_DM_TEAM_CNT];
		unsigned long cnt = 0;
		tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[teamType].mMemberMap.Begin();
		tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[teamType].mMemberMap.End();
		for( ; b!=end; ++b )
		{
			cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
			if( pInfo )
			{
				cPlayer* player = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
				if( player == 0 || player->GetStateDie() == true )
					continue;

				memberIdx[cnt] = pInfo->mPlayerIdx;
				++cnt;
			}
		}
		if( cnt == 0 )
			continue;

		/// 기존 수장 권한 반납
		sObject leader = { eOBJECTTYPE_PLAYER, mLeaderInfo[teamType].mLeaderIdx };
		SKILLMANAGER->DeleteInfluenceClassIdx( leader, PVP_DM_LEADER_INFLUENCE );
		SKILLMANAGER->DeleteInfluenceClassIdx( leader, PVP_DM_LEADER_INFLUENCE2 );

		/// 선정된 목록중 랜덤 선택
		unsigned long selectCnt = rand() % cnt;

		sObject object = { eOBJECTTYPE_PLAYER, memberIdx[selectCnt] };
		if( SKILLMANAGER->AddInfSwitch( object, object, PVP_DM_LEADER_INFLUENCE, PVP_DM_LEADER_SKILL, true ) == 0 )
			return;
		SKILLMANAGER->AddInfSwitch( object, object, PVP_DM_LEADER_INFLUENCE2, 0, true );

		mLeaderInfo[teamType].mLeaderIdx = memberIdx[selectCnt];
		/// 다음 변경 시간 설정
		mLeaderInfo[teamType].mLeaderChangeTime = accumTime + PVP_DM_LEADER_CHANGETIME;

		/// 수장 설정 메세지 발송
		MSG_SYN_PVP_DM_LEADERCHANGE synMsg;
		synMsg.Category = NM_PVP;
		synMsg.Protocol = NM_PVP_DM_LEADERSELECT_SYN;
		synMsg.mCharacterIdx = mLeaderInfo[teamType].mLeaderIdx;
		TeamQuickSend( (ePVPDM_TEAM_TYPE)teamType, (char*)&synMsg, sizeof(synMsg) );
	}
}


void cDeathMatchObject::StoneProcess( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( mpDMBlockInfo == NULL )
	{
		assert(NULL);
		if( mNextLogTime < accumTime )
		{
			mNextLogTime = accumTime + 1 * MINUTE;
			NETWORK2->PostServerEvent("cDeathMatchObject::StoneProcess mpDMBlockInfo == NULL");
		}
		return;
	}

	/// 리더 선정
	for( int teamType = ePVPDM_TEAMTYPE_A ; teamType < ePVPDM_TEAMTYPE_MAX ; ++teamType )
	{
		if( mStoneInfo[teamType].mStoneIdx != 0 )
			continue;

		if( mStoneInfo[teamType].mStoneRegenTime != 0 )
		{
			/// 시간 체크
			if( mStoneInfo[teamType].mStoneRegenTime >= accumTime )
				continue;
		}

		mStoneInfo[teamType].mStoneRegenTime = ULONG_MAX;
		
		//unsigned long posIdx = PVPMANAGER->GetTeamGenPosIdx( (ePVPDM_TEAM_TYPE)teamType );
		//if( posIdx == 0 )
		//{
		//	if( mNextLogTime < accumTime )
		//	{
		//		mNextLogTime = accumTime + 1 * MINUTE;
		//		NETWORK2->PostServerEvent("cDeathMatchObject::StoneProcess posIdx[%d] == 0", posIdx);
		//	}

		//	assert(NULL);
		//	continue;
		//}
		//else
		//{
		//	sStageChangePos* pStagePos = STAGESCRIPT->GetPosScriptInfo( posIdx );
		//	if( pStagePos != 0 )
		//	{
		//		x = pStagePos->mPosX;
		//		y = pStagePos->mPosY;
		//	}
		//	else
		//	{
		//		if( mNextLogTime < accumTime )
		//		{
		//			mNextLogTime = accumTime + 1 * MINUTE;
		//			NETWORK2->PostServerEvent("cDeathMatchObject::StoneProcess pStagePos[%d] != 0", posIdx);
		//		}

		//		assert(NULL);
		//		continue;
		//	}
		//}

		/// 다음 변경 시간 설정
		//mStoneInfo[teamType].mStoneRegenTime = accumTime + PVP_DM_STONE_REGEN_TIME;

		sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
		if( pDMInfo == NULL )
		{
			if( mNextLogTime < accumTime )
			{
				mNextLogTime = accumTime + 1 * MINUTE;
				NETWORK2->PostServerEvent("cDeathMatchObject::StoneProcess pDMInfo == NULL");
			}
			continue;
		}

		if( pDMInfo->mStone == false )
			continue;

		NiPoint2 pos;
		if( teamType == ePVPDM_TEAMTYPE_A )
			pos = pDMInfo->mStoneAPos;
		else if( teamType == ePVPDM_TEAMTYPE_B )
			pos = pDMInfo->mStoneBPos;
		else
			assert(NULL);

		unsigned long stoneIdx = mpDMBlockInfo->mStoneAIdx;
		if( teamType == ePVPDM_TEAMTYPE_B )
			stoneIdx = mpDMBlockInfo->mStoneBIdx;

		cMonster* pMonster = AIMANAGER->StoneSummonMonRegen( mDMIdx, pos.x, pos.y, stoneIdx, (ePVPDM_TEAM_TYPE)teamType );
		if( pMonster == NULL )
		{
			if( mNextLogTime < accumTime )
			{
				mNextLogTime = accumTime + 1 * MINUTE;
				NETWORK2->PostServerEvent("cDeathMatchObject::StoneProcess pMonster == NULL[%d,%f,%f,%d,%f]", 
					mDMIdx, pos.x, pos.y, stoneIdx, (ePVPDM_TEAM_TYPE)teamType);
			}

			assert(NULL);
			continue;
		}

		/// 고유번호
		mStoneInfo[teamType].mStoneIdx = pMonster->GetObjectID();

		/// 석상생성
		MSG_SYN_PVP_DM_STONE_CREATE synMsg;
		synMsg.Category = NM_PVP;
		synMsg.Protocol = NM_PVP_DM_STONE_CREATE_SYN;
		synMsg.mTeamType = (unsigned char)teamType;
		synMsg.mMaxHP = pMonster->GetMaxHP();
		synMsg.mPos = pMonster->GetPos();
		DMQuickSend( (char*)&synMsg, sizeof(synMsg) );
	}
}


void cDeathMatchObject::ScoreUpdate( unsigned long killIdx, ePVPDM_TEAM_TYPE killerTeam, 
									unsigned long deadIdx, ePVPDM_TEAM_TYPE deadTeam )
{
	if( mProcess < ePVPDM_PROCESS_FIGHT_MINIMUM || ePVPDM_PROCESS_FIGHT < mProcess )
		return;

	if( killerTeam >= ePVPDM_TEAMTYPE_MAX || deadTeam >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return;
	}

	/// 최초 킬포인트 획득팀 셋팅
	if( mFirstKillTeam == ePVPDM_TEAMTYPE_MAX )
		mFirstKillTeam = killerTeam;

	/// 수장 죽음 체크
	bool isLeader = false;

	unsigned long plusPoint = PVP_DM_MEMBER_DIE_POINT;
	/// 죽은 수장 처리
	if( mLeaderInfo[deadTeam].mLeaderIdx == deadIdx )
	{
		isLeader = true;

		/// 수장 없음 설정
		++mLeaderInfo[deadTeam].mLeaderDieCnt;
		mLeaderInfo[deadTeam].mLeaderIdx = 0;
		mLeaderInfo[deadTeam].mLeaderChangeTime = NETWORK2->GetAccumTime() + PVP_DM_LEADER_DIECHANGETIME;

		/// 수장 죽음 메세지 발송
		MSG_SYN_DM_LEADERDIE synMsg;
		synMsg.Category = NM_PVP;
		synMsg.Protocol = NM_PVP_DM_LEADERDIE_SYN;
		synMsg.mPlayerIdx = deadIdx;
		TeamQuickSend( deadTeam, (char*)&synMsg, sizeof( synMsg ) );

		MSG_SYN_DM_ENEMYLEADERDIE synMsgEnemy;
		synMsgEnemy.Category = NM_PVP;
		synMsgEnemy.Protocol = NM_PVP_DM_ENEMYLEADERDIE_SYN;
		synMsgEnemy.mPlayerIdx = killIdx;
		TeamQuickSend( killerTeam, (char*)&synMsgEnemy, sizeof( synMsgEnemy ) );

		/// 죽은 자가 수장이면 5포인트를 줌

		plusPoint = PVP_DM_LEADER_DIE_POINT;
	}

	/// 죽인자 팀에 점수 증가
	mTeamInfo[killerTeam].mPoint += plusPoint;

	/// 죽인자 kill 증가
	cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)mTeamInfo[killerTeam].mMemberMap.GetAt( killIdx );
	if( pInfo )
	{
		if( isLeader == true )
			pInfo->mLeaderKillCnt++;
		else
			pInfo->mKillCnt++;

		/// 점수 메세지 발송
		MSG_SYN_PVP_DM_SCORE msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM_SCORE_SYN;
		msg.mTeamType = (unsigned char)killerTeam;
		msg.mPlusScore = plusPoint;
		msg.mTotalScore = mTeamInfo[killerTeam].mPoint;
		msg.mPlayerIdx = killIdx;
		msg.mPlayerScore = pInfo->mKillCnt + pInfo->mLeaderKillCnt;
		TeamQuickSend( killerTeam, (char*)&msg, sizeof( MSG_SYN_PVP_DM_SCORE ) );
	}
	else
	{
		assert(0);
	}

	pInfo = (cPvPDMMemberInfo*)mTeamInfo[deadTeam].mMemberMap.GetAt( deadIdx );
	if( pInfo )
	{
		pInfo->mDeathCnt++;

		/// 점수 메세지 발송
		MSG_SYN_PVP_DM_SCORE msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM_SCORE_SYN;
		msg.mTeamType = (unsigned char)killerTeam;
		msg.mPlusScore = plusPoint;
		msg.mTotalScore = mTeamInfo[killerTeam].mPoint;
		msg.mPlayerIdx = deadIdx;
		msg.mPlayerScore = pInfo->mDeathCnt;
		TeamQuickSend( deadTeam, (char*)&msg, sizeof( MSG_SYN_PVP_DM_SCORE ) );
	}
	else
	{
		assert(0);
	}

	/// 점수 승리조건 체크
	if( mTeamInfo[killerTeam].mPoint >= mpDMBlockInfo->mGoalPoint )
		mScoreEnd = true; 
}

void cDeathMatchObject::StoneScoreUpdate( ePVPDM_TEAM_TYPE killerTeam, unsigned long killerIdx )
{
	if( mProcess < ePVPDM_PROCESS_FIGHT_MINIMUM || ePVPDM_PROCESS_FIGHT < mProcess )
		return;

	if( killerTeam >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return;
	}

	/// 최초 킬포인트 획득팀 셋팅
	if( mFirstKillTeam == ePVPDM_TEAMTYPE_MAX )
		mFirstKillTeam = killerTeam;

	/// 죽인자 팀에 점수 증가
	mTeamInfo[killerTeam].mPoint += PVP_DM_STONE_DIE_POINT;

	/// 죽인자 kill 증가
	unsigned long killerScore = 0;
	cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)mTeamInfo[killerTeam].mMemberMap.GetAt( killerIdx );
	if( pInfo )
		killerScore = pInfo->mKillCnt + pInfo->mLeaderKillCnt;

	/// 점수 메세지 발송
	MSG_SYN_PVP_DM_STONE_DIE msg;
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_DM_STONE_DIE_SYN;
	msg.mTeamType = (unsigned char)killerTeam;
	msg.mKillerIndex = killerIdx;
	msg.mTotalScore = mTeamInfo[killerTeam].mPoint;

	DMQuickSend( (char*)&msg, sizeof(MSG_SYN_PVP_DM_STONE_DIE) );

	//MSG_SYN_PVP_DM_STONE_ENEMYDIE msgEnemyDie;
	//msgEnemyDie.Category = NM_PVP;
	//msgEnemyDie.Protocol = NM_PVP_DM_STONE_ENEMYDIE_SYN;

	//msgEnemyDie.mTeamType = (unsigned char)killerTeam;
	//msgEnemyDie.mTotalScore = mTeamInfo[killerTeam].mPoint;
	//msgEnemyDie.mPlayerIdx = killerIdx;

	////msgEnemyDie.mPlusScore = PVP_DM_STONE_DIE_POINT;
	////msgEnemyDie.mPlayerScore = killerScore;
	////msgEnemyDie.mPlayerIdx = killerIdx;
	//TeamQuickSend( killerTeam, (char*)&msgEnemyDie, sizeof( MSG_SYN_PVP_DM_STONE_ENEMYDIE ) );

	///// 점수 메세지 발송
	//MSG_SYN_PVP_DM_STONE_DIE msg;
	//msg.Category = NM_PVP;
	//msg.Protocol = NM_PVP_DM_STONE_DIE_SYN;

	//msg.mTeamType = (unsigned char)killerTeam;
	//msg.mTotalScore = mTeamInfo[killerTeam].mPoint;
	//msg.mPlayerIdx = 0;

	//msg.mPlusScore = PVP_DM_STONE_DIE_POINT;
	//msg.mPlayerScore = 0;
	//TeamQuickSend( deadTeam, (char*)&msg, sizeof( MSG_SYN_PVP_DM_STONE_DIE ) );

	/// 점수 승리조건 체크
	if( mTeamInfo[killerTeam].mPoint >= mpDMBlockInfo->mGoalPoint )
		mScoreEnd = true; 
}

void cDeathMatchObject::Result()
{
	mProcess = ePVPDM_PROCESS_FIGHTEND;
	mProcessEndTime = NETWORK2->GetAccumTime() + PVP_DM_OUTWAIT_TIME;

	if( mpDMBlockInfo )
	{
		///포인트 달성 체크
		if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint >= mpDMBlockInfo->mGoalPoint )
		{
			SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_POINT );
			return;
		}
		if( mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint >= mpDMBlockInfo->mGoalPoint )
		{
			SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_POINT );
			return;
		}
	}
	else
	{
		assert(0);
	}

	/// 포인트 우위 체크
	if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint > mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_TIME );
		return;
	}
	else if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint < mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_TIME );
		return;
	}

	/// 수장 사망 횟수 체크
	if( mLeaderInfo[ePVPDM_TEAMTYPE_A].mLeaderDieCnt > mLeaderInfo[ePVPDM_TEAMTYPE_B].mLeaderDieCnt )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_LEADERDIE );
		return;
	}
	if( mLeaderInfo[ePVPDM_TEAMTYPE_A].mLeaderDieCnt < mLeaderInfo[ePVPDM_TEAMTYPE_B].mLeaderDieCnt )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_LEADERDIE );
		return;
	}

	cBaseDeathMatch::Result();
}

void cDeathMatchObject::OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType )
{
	cBaseDeathMatch::OutMember( playerIdx, teamType );

	if( mLeaderInfo[teamType].mLeaderIdx == playerIdx )
	{
		mLeaderInfo[teamType].mLeaderIdx = 0;
		mLeaderInfo[teamType].mLeaderChangeTime = NETWORK2->GetAccumTime() + PVP_DM_LEADER_DIECHANGETIME;
	}

	///
	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo )
	{
		if( ( ePVPDM_PROCESS_FIGHT_MINIMUM == mProcess && mTeamInfo[teamType].mMemberCnt == 0 ) ||
			( ePVPDM_PROCESS_FIGHT == mProcess && mTeamInfo[teamType].mMemberCnt < pDMInfo->mTeamPlayerMin ) )
		{
			mProcess = ePVPDM_PROCESS_FIGHTEND;
			mProcessEndTime = NETWORK2->GetAccumTime() + PVP_DM_OUTWAIT_TIME;

			if( teamType == ePVPDM_TEAMTYPE_A )
				SendResult( ePVPDM_TEAMTYPE_B,ePVPDM_ENDTYPE_MEMBER );
			if( teamType == ePVPDM_TEAMTYPE_B )
				SendResult( ePVPDM_TEAMTYPE_A,ePVPDM_ENDTYPE_MEMBER );
		}
	}
	else
	{
		assert(0);
	}
}

void cDeathMatchObject::SyncStoneHP( unsigned long teamType, unsigned long hp )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
		return;

	MSG_SYN_PVP_DM_STONE_HP syncHP;
	syncHP.Category = NM_PVP;
	syncHP.Protocol = NM_PVP_DM_STONE_HP_SYN;
	syncHP.mHP = hp;
	TeamQuickSend( (ePVPDM_TEAM_TYPE)teamType, (char*)&syncHP, sizeof( syncHP ) );
}

void cDeathMatchObject::DeleteStone( unsigned long teamType, unsigned long lastAttacker )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
		return;

	if( mProcess != ePVPDM_PROCESS_FIGHT_MINIMUM && mProcess != ePVPDM_PROCESS_FIGHT )
		return;

	mStoneInfo[teamType].mStoneIdx = 0;
	mStoneInfo[teamType].mStoneRegenTime = NETWORK2->GetAccumTime() + PVP_DM_STONE_REGEN_TIME;

	if( teamType == ePVPDM_TEAMTYPE_A )
		StoneScoreUpdate( ePVPDM_TEAMTYPE_B, lastAttacker );
	else
		StoneScoreUpdate( ePVPDM_TEAMTYPE_A, lastAttacker );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

cDeathMatch2Object::cDeathMatch2Object() : cBaseDeathMatch( ePVPDM_OBJECTTYPE_2 )
{
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
			continue;

		pF->mFlagIndex = 0;
		pF->mFlagPointIndex = 0;

		pF->mFlagOwnerIndex = 0;
		pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;

		pF->mNewFlagGen = false;
		pF->mNewFlagWaitTime = 0;
		pF->mSendFlagPosTime = ULONG_MAX;
	}

	mFirstFlagTeam = ePVPDM_TEAMTYPE_MAX;
}

cDeathMatch2Object::~cDeathMatch2Object()
{
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
			continue;

		if( pF->mFlagIndex != 0 )
		{
			cGathering* p = OBJECTMANAGER->GetGathering( pF->mFlagIndex );
			if( p )
			{
				if( GRIDMANAGER->RemoveGathering( p ) == false )
					NETWORK2->PostServerEvent("~cDeathMatch2Object() GRIDMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, i );

				if( OBJECTMANAGER->RemoveGathering( pF->mFlagIndex ) == false )
					NETWORK2->PostServerEvent("~cDeathMatch2Object() OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, i );
			}

			pF->mFlagIndex = 0;
		}

		if( pF->mFlagPointIndex != 0 )
		{
			cGathering* p = OBJECTMANAGER->GetGathering( pF->mFlagPointIndex );
			if( p )
			{
				if( GRIDMANAGER->RemoveGathering( p ) == false )
					NETWORK2->PostServerEvent("~cDeathMatch2Object() GRIDMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagPointIndex, i );

				if( OBJECTMANAGER->RemoveGathering( pF->mFlagIndex ) == false )
					NETWORK2->PostServerEvent("~cDeathMatch2Object() OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagPointIndex, i );
			}
			pF->mFlagPointIndex = 0;
		}
	}

	for( unsigned char i = 0 ; i < mAHoistCnt ; ++i )
	{
		sHoistInfo* pH = &mAteamHoist[i];
		if( pH == NULL )
			continue;

		cGathering* p = OBJECTMANAGER->GetGathering( pH->mObjectIdx );
		if( p )
		{
			if( GRIDMANAGER->RemoveGathering( p ) == false )
				NETWORK2->PostServerEvent("~cDeathMatch2Object() mAteamHoist GRIDMANAGER->RemoveGathering( %d, %d ) == false", pH->mObjectIdx, i );

			if( OBJECTMANAGER->RemoveGathering( pH->mObjectIdx ) == false )
				NETWORK2->PostServerEvent("~cDeathMatch2Object() mAteamHoist OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pH->mObjectIdx, i );
		}
	}

	for( unsigned char i = 0 ; i < mBHoistCnt ; ++i )
	{
		sHoistInfo* pH = &mBteamHoist[i];
		if( pH == NULL )
			continue;

		cGathering* p = OBJECTMANAGER->GetGathering( pH->mObjectIdx );
		if( p )
		{
			if( GRIDMANAGER->RemoveGathering( p ) == false )
				NETWORK2->PostServerEvent("~cDeathMatch2Object() mBteamHoist GRIDMANAGER->RemoveGathering( %d, %d ) == false", pH->mObjectIdx, i );

			if( OBJECTMANAGER->RemoveGathering( pH->mObjectIdx ) == false )
				NETWORK2->PostServerEvent("~cDeathMatch2Object() mBteamHoist OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pH->mObjectIdx, i );
		}
	}
}

void* cDeathMatch2Object::operator new( size_t n )
{
	if( n != sizeof(cDeathMatch2Object) ) 
	{
		NETWORK2->PostServerEvent("cDeathMatchObject::operator new %d != sizeof(cDeathMatch2Object)", n );
		return NULL;
	}

	return PVPOBJECTPOOL->NewDM2Object();
}


void cDeathMatch2Object::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사



	if( ptr == 0 )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::operator delete ptr == 0" );
		return;
	}

	if( n != sizeof(cDeathMatch2Object) ) 
	{
		NETWORK2->PostServerEvent("cDeathMatchObject::operator delete n != sizeof(cDeathMatch2Object", n );
		return;
	}

	//PVPOBJECTPOOL->FreeDMObject( static_cast<cDeathMatch2Object*>(ptr) );

	return;
}

bool cDeathMatch2Object::Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo )
{
	if( pLevelForceInfo == NULL )
		return false;

	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
			continue;

		pF->mFlagIndex = 0;
		pF->mFlagPointIndex = 0;

		pF->mFlagOwnerIndex = 0;
		pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;

		pF->mNewFlagGen = false;
		pF->mNewFlagWaitTime = 0;
		pF->mSendFlagPosTime = ULONG_MAX;
	}

	mFlagCnt = 0;

	mAHoistCnt = 0;
	memset( mAteamHoist, 0, sizeof(mAteamHoist) );
	mBHoistCnt = 0;
	memset( mBteamHoist, 0, sizeof(mBteamHoist) );

	mFirstFlagTeam = ePVPDM_TEAMTYPE_MAX;

	return cBaseDeathMatch::Init( DMIdx, pLevelForceInfo );
}


void cDeathMatch2Object::Process( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	/// 방에 아무도 없으면 객체 소멸
	/// 마지막 유저가 나가는 시점에 삭제 불가능 했던 이유 체크해보자!!
	if( mTeamInfo[ePVPDM_TEAMTYPE_A].mMemberCnt + mTeamInfo[ePVPDM_TEAMTYPE_B].mMemberCnt == 0 && ePVPDM_PROCESS_READY < mProcess )
		mProcess = ePVPDM_PROCESS_END;

	if( mScoreEnd == true )
	{
		mScoreEnd = false;
		Result();
	}

	switch( mProcess )
	{
	case ePVPDM_PROCESS_WAIT:
		{
			if( mWaitEndTime > accumTime )
				return;

			Ready();
		}
		break;
	case ePVPDM_PROCESS_READY:
		{
			/// PvP 대기시간

			if( mProcessEndTime > accumTime )
				return;

			EndNpcTalk();

			unsigned long fightTime = 0;
			sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
			if( pInfo != NULL )
			{
				/// ePVPDM_PROCESS_FIGHT_MINIMUM 에서 나머지 잔여시간을 다시 설정
				fightTime = pInfo->mTotalDMTime * MINUTE;		/// 총전투시간

				/// 총전투시간보다 인원부족체크 안함 시간이 큰경우


				if( PVP_DM_TEAM_MINCNT_TIME > fightTime )	
					mProcessEndTime = accumTime + fightTime;	 /// 다음프로세스 수행시간을 스크립트시간으로
				else	/// 총전투시간보다 인원부족 시간이 작은경우
					mProcessEndTime = accumTime + PVP_DM_TEAM_MINCNT_TIME;	/// 다음프로세스 수행시간을 최소인원 체크안함시간으로
			}

			/// 클라이언트에는 총전투시간을 알려줌


			SendFight( fightTime );

			/// pvp 전투 시작 프로세스로 이동
			mProcess = ePVPDM_PROCESS_FIGHT_MINIMUM;

			///
			cTotem* pTotem = NULL;
			NiPoint2 centerPos;
			for( unsigned char i=0;i<ePVPDM_TEAMTYPE_MAX; ++i )
			{
				unsigned long posIdx = PVPMANAGER->GetTeamGenPosIdx( (ePVPDM_TEAM_TYPE)i );
				if( posIdx != 0 )
				{
					sStageChangePos* pStagePos = STAGESCRIPT->GetPosScriptInfo( posIdx );
					if( pStagePos != 0 )
					{
						centerPos.x = pStagePos->mPosX;
						centerPos.y = pStagePos->mPosY;
						pTotem = OBJECTMANAGER->PvPAddTotem( i, PVP_DM_ADD_TOTEMIDX_BUFF, mDMIdx, centerPos, eAPPLYTYPE_BUDDY, eATTRIBUTETYPE_PHYSICAL );
						if( pTotem != NULL )
						{
							GRIDMANAGER->AddTotem( pTotem );
							mDMTotemArr.PushBack(pTotem->GetObjectID());
						}
						pTotem = OBJECTMANAGER->PvPAddTotem( i, PVP_DM_ADD_TOTEMIDX_DEBUFF, mDMIdx, centerPos, eAPPLYTYPE_ENEMY, eATTRIBUTETYPE_PHYSICAL );
						if( pTotem != NULL )
						{
							GRIDMANAGER->AddTotem( pTotem );
							mDMTotemArr.PushBack(pTotem->GetObjectID());
						}
					}
				}
			}

			AddTeleportPortal();

			RegenHoist( true );
			EnableNewGen();
		}
		break;
	case ePVPDM_PROCESS_READY_OUT:
		{
			/// pvp out 요청 받는 시간
			if( mProcessEndTime > accumTime )
				return;

			/// 오브젝트 제거
			mProcess = ePVPDM_PROCESS_END;
		}
		break;
	case ePVPDM_PROCESS_FIGHT_MINIMUM:
		{
			/// 일정 시간동안 인원부족 결투 종료 체크 안함
			if( mProcessEndTime <= accumTime )
			{
				mProcess = ePVPDM_PROCESS_FIGHT;

				mProcessEndTime = accumTime + 0;
				sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
				if( pInfo != NULL )
				{
					/// 인원체크 안함 시간보다 총전투시간이 큰경우 나머지 잔여시간을 설정해줌
					if( PVP_DM_TEAM_MINCNT_TIME < pInfo->mTotalDMTime * MINUTE )
						mProcessEndTime = accumTime + ( pInfo->mTotalDMTime * MINUTE - PVP_DM_TEAM_MINCNT_TIME );

					for( int i = ePVPDM_TEAMTYPE_A ; i < ePVPDM_TEAMTYPE_MAX ; ++i )
					{
						if( mTeamInfo[i].mMemberCnt < pInfo->mTeamPlayerMin )
						{
							Result();
							return;
						}
					}
				}
			}

			FlagProcess( accumTime );
		}
		break;
	case ePVPDM_PROCESS_FIGHT:
		{
			/// 시간 종료
			if( mProcessEndTime <= accumTime )
				Result();
			else
				FlagProcess( accumTime );
		}
		break;
	case ePVPDM_PROCESS_FIGHTEND:
		{
			/// pvp out 요청 받는 시간
			if( mProcessEndTime <= accumTime )
				mProcess = ePVPDM_PROCESS_END;
		}
		break;
	case ePVPDM_PROCESS_END:
		{
			/// 전체 유저 추방
			if( mAutoOutCheckTime < accumTime )
			{
				if( OrderEndDM() == true )
				{
					EndDM();
					PVPMANAGER->DelAryInsertPVP( mDMIdx );
				}
			}
		}
		break;
	}
}

void cDeathMatch2Object::FlagProcess( unsigned long accumTime )
{
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
			continue;

		if( pF->mNewFlagGen && pF->mNewFlagWaitTime <= accumTime )
		{
			RegenNewFlag( i );

			pF->mNewFlagGen = false;
			pF->mNewFlagWaitTime = 0;
		}

		if( pF->mSendFlagPosTime <= accumTime )
			SendFlagPos( i );
	}

	RegenHoist( false );
}


void cDeathMatch2Object::SendFight( unsigned long fightTime )
{
	/// 전투 시작 발송
	cBaseDeathMatch::SendFight( fightTime );
}

void cDeathMatch2Object::RegenNewFlag( unsigned char aryPos )
{
	if( mFlagCnt <= aryPos )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::RegenNewFlag mFlagCnt <= aryPos[%d,%d]", mFlagCnt, aryPos );
		return;
	}

	sFlagInfo* pF = &mFlagInfo[aryPos];
	if( pF == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::RegenNewFlag pF == NULL[%d]", aryPos );
		return;
	}

	/// 기존 리젠 제거
	if( pF->mFlagIndex != 0 )
	{
		cGathering* p = OBJECTMANAGER->GetGathering( pF->mFlagIndex );
		if( p )
		{
			if( GRIDMANAGER->RemoveGathering( p ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() GRIDMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, aryPos );

			if( OBJECTMANAGER->RemoveGathering( pF->mFlagIndex ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, aryPos );
		}
		pF->mFlagIndex = 0;
	}
	if( pF->mFlagPointIndex != 0 )
	{
		cGathering* p = OBJECTMANAGER->GetGathering( pF->mFlagPointIndex );
		if( p )
		{
			if( GRIDMANAGER->RemoveGathering( p ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() GRIDMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagPointIndex, aryPos );

			if( OBJECTMANAGER->RemoveGathering( pF->mFlagIndex ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagPointIndex, aryPos );
		}
		pF->mFlagPointIndex = 0;
	}

	/// 깃발 보유 버프 변경



	if( pF->mFlagOwnerIndex != 0 )
	{
		pF->mFlagOwnerIndex = 0;
		pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;

		/// 버프 제거
		sObject object = { eOBJECTTYPE_PLAYER, pF->mFlagOwnerIndex };
		SKILLMANAGER->DeleteInfluenceClassIdx( object, PVP_DM2_FLAG_INFLUENCE );
	}

	unsigned long mapNum = PVPMANAGER->GetMapIndex();

	/// 깃발 생성
	{
		unsigned long flagIdx = mapNum*MAPNUMBER_UNIT + 3;
		const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( 0, flagIdx );
		if( header )
		{
			unsigned int posCnt = header->mRegenDetail.GetSize();
			if( posCnt )
			{
				sGatheringRegen* flagInfo = (sGatheringRegen*)header->mRegenDetail[aryPos];

				pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;
				cGathering* pFlag = OBJECTMANAGER->AddGathering( flagIdx, flagInfo->mGatheringIdx, mDMIdx, 
					flagInfo->mPosX, flagInfo->mPosY, flagInfo->mDirection, flagInfo->mHeight );
				if( pFlag )
				{
					pFlag->SetDMIndex( mDMIdx );

					if( GRIDMANAGER->AddGathering( pFlag ) == false )
					{
						assert(0);
						OBJECTMANAGER->RemoveGathering( pFlag->GetObjectID() );
						pF->mFlagIndex = 0;
						pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;
					}
					else
					{
						pFlag->SendRegenSync();
						pF->mFlagIndex = pFlag->GetObjectID();
					}
				}
				else
				{
					assert(0);
					NETWORK2->PostServerEvent( "RegenNewFlag() : flag create fail" );
				}
			}
			else
			{
				assert(0);
				NETWORK2->PostServerEvent( "RegenNewFlag() : flag regen pos count 0" );
			}
		}
		else
		{
			assert(0);
			NETWORK2->PostServerEvent( "RegenNewFlag() : flag info NULL" );
		}
	}

	///// 계양 포인트 생성
	//{
	//	unsigned long flagPointIdx = mapNum*MAPNUMBER_UNIT + 2;
	//	const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( 0, flagPointIdx );
	//	if( header )
	//	{
	//		unsigned int posCnt = header->mRegenDetail.GetSize();
	//		if( posCnt )
	//		{
	//			unsigned int i = rand() % posCnt;
	//			sGatheringRegen* flagPointInfo = (sGatheringRegen*)header->mRegenDetail[i];

	//			cGathering* pFlagPoint = OBJECTMANAGER->AddGathering( flagPointIdx, flagPointInfo->mGatheringIdx, mDMIdx, 
	//				flagPointInfo->mPosX, flagPointInfo->mPosY, flagPointInfo->mDirection, flagPointInfo->mHeight );
	//			if( pFlagPoint )
	//			{
	//				pFlagPoint->SetDMIndex( mDMIdx );

	//				if( GRIDMANAGER->AddGathering( pFlagPoint ) == false )
	//				{
	//					assert(0);
	//					OBJECTMANAGER->RemoveGathering( pFlagPoint->GetObjectID() );
	//					mFlagPointIndex = 0;
	//				}
	//				else
	//				{
	//					pFlagPoint->SendRegenSync();
	//					mFlagPointIndex = pFlagPoint->GetObjectID();
	//				}
	//			}
	//			else
	//			{
	//				assert(0);
	//				NETWORK2->PostServerEvent( "RegenNewFlag() : flag point create fail" );
	//			}
	//		}
	//		else
	//		{
	//			assert(0);
	//			NETWORK2->PostServerEvent( "RegenNewFlag() : flag point regen pos count 0" );
	//		}

	//	}
	//	else
	//	{
	//		assert(0);
	//		NETWORK2->PostServerEvent( "RegenNewFlag() : flag point info NULL" );
	//	}
	//}

	/// 정보 갱신
	SendFlagInfo( aryPos, true );
}

void cDeathMatch2Object::RegenHoist( bool first )
{
	for( int ii = 1 ; ii <= 2 ; ++ii )
	{
		sHoistInfo* pTeamHoist = NULL;
		unsigned char* pHoistCnt = NULL;
		unsigned long basePos = 0;
		unsigned char teamType = ePVPDM_TEAMTYPE_MAX;
		if( ii == 1 )
		{
			pHoistCnt = &mAHoistCnt;
			pTeamHoist = mAteamHoist;
			basePos = 0;
			teamType = ePVPDM_TEAMTYPE_A;
		}
		else if( ii == 2 )
		{
			pHoistCnt = &mBHoistCnt;
			pTeamHoist = mBteamHoist;
			basePos = FLAG_MAX;
			teamType = ePVPDM_TEAMTYPE_B;
		}

		if( pTeamHoist == NULL )
			return;

		/// 계양 포인트 생성
		unsigned long flagPointIdx = PVPMANAGER->GetMapIndex() * MAPNUMBER_UNIT + ii;
		const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( 0, flagPointIdx );
		if( header )
		{
			unsigned int posCnt = header->mRegenDetail.GetSize();
			if( first == true )
				*pHoistCnt = (unsigned char)posCnt;

			if( posCnt )
			{
				for( unsigned char i = 0 ; i < *pHoistCnt ; ++i )
				{
					if( ( pTeamHoist[i].mObjectIdx == 0 && pTeamHoist[i].mNextRegenTime <= NETWORK2->GetAccumTime() ) || first == true  )
					{
						sGatheringRegen* flagPointInfo = (sGatheringRegen*)header->mRegenDetail[i];

						if( first == true )
							pTeamHoist->mRegenIdx = flagPointInfo->mGatheringIdx;

						cGathering* pFlagPoint = OBJECTMANAGER->AddGathering( flagPointIdx, flagPointInfo->mGatheringIdx, mDMIdx, 
							flagPointInfo->mPosX, flagPointInfo->mPosY, flagPointInfo->mDirection, flagPointInfo->mHeight );
						if( pFlagPoint )
						{
							pFlagPoint->SetDMIndex( mDMIdx );

							if( GRIDMANAGER->AddGathering( pFlagPoint ) == false )
							{
								assert(0);
								OBJECTMANAGER->RemoveGathering( pFlagPoint->GetObjectID() );
								pTeamHoist[i].mObjectIdx = 0;
							}
							else
							{
								pFlagPoint->SetFlagTeamType( teamType );
								pFlagPoint->SendRegenSync( teamType );
								pTeamHoist[i].mObjectIdx = pFlagPoint->GetObjectID();

								unsigned long hoistPos = basePos + i + 1;
								/// 메세지 발송
								MSG_SYN_PVP_DM2_HOIST msg;
								msg.Category = NM_PVP;
								msg.Protocol = NM_PVP_DM2_HOIST_SYN;
								msg.mGatherIdx = hoistPos;
								msg.mPos.x = flagPointInfo->mPosX;
								msg.mPos.y = flagPointInfo->mPosY;
								DMQuickSend( (char*)&msg, sizeof(msg) );
							}
						}
						else
						{
							assert(0);
							NETWORK2->PostServerEvent( "cDeathMatch2Object::UseHoist() A flag point create fail" );
						}
					}
				}
			}
			else
			{
				assert(0);
				NETWORK2->PostServerEvent( "cDeathMatch2Object::UseHoist() A flag point regen pos count 0" );
			}
		}
		else
		{
			assert(0);
			NETWORK2->PostServerEvent( "cDeathMatch2Object::UseHoist() A flag point info NULL" );
		}
	}
}

void cDeathMatch2Object::RegenFlag( unsigned char aryPos, float x, float y )
{
	if( mFlagCnt <= aryPos )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::RegenFlag mFlagCnt <= aryPos[%d,%d]", mFlagCnt, aryPos );
		return;
	}

	sFlagInfo* pF = &mFlagInfo[aryPos];
	if( pF == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::RegenFlag pF == NULL[%d]", aryPos );
		return;
	}

	if( pF->mFlagOwnerTeam < ePVPDM_TEAMTYPE_MAX )
	{
		MSG_SYN_PVP_DM2_FLAGDROP msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM2_FLAGDROP_SYN;
		msg.mPlayerIdx = pF->mFlagOwnerIndex;
		TeamQuickSend( pF->mFlagOwnerTeam, (char*)&msg, sizeof(msg) );
	}

	if( pF->mFlagIndex != 0 )
	{
		assert(0);
		cGathering* p = OBJECTMANAGER->GetGathering( pF->mFlagIndex );
		if( p )
		{
			if( GRIDMANAGER->RemoveGathering( p ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() GRIDMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, aryPos );

			if( OBJECTMANAGER->RemoveGathering( pF->mFlagIndex ) == false )
				NETWORK2->PostServerEvent("RegenNewFlag() OBJECTMANAGER->RemoveGathering( %d, %d ) == false", pF->mFlagIndex, aryPos );
		}
		pF->mFlagIndex = 0;
		pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;
	}

	/// 깃발 보유 버프 변경

	if( pF->mFlagOwnerIndex != 0 )
	{
		/// 버프 제거
		sObject object = { eOBJECTTYPE_PLAYER, pF->mFlagOwnerIndex };
		SKILLMANAGER->DeleteInfluenceClassIdx( object, PVP_DM2_FLAG_INFLUENCE );

		pF->mFlagOwnerIndex = 0;
	}

	unsigned long mapNum = PVPMANAGER->GetMapIndex();

	/// 운반자 사망시 깃발 리젠
	unsigned long flagIdx = mapNum*MAPNUMBER_UNIT + 3;
	const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( 0, flagIdx );
	if( header )
	{
		unsigned int posCnt = header->mRegenDetail.GetSize();
		if( posCnt )
		{
			sGatheringRegen* flagInfo = (sGatheringRegen*)header->mRegenDetail[aryPos];

			pF->mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;
			cGathering* pFlag = OBJECTMANAGER->AddGathering( flagIdx, flagInfo->mGatheringIdx, mDMIdx, 
				x, y, flagInfo->mDirection, flagInfo->mHeight );
			if( pFlag )
			{
				pFlag->SetDMIndex( mDMIdx );

				if( GRIDMANAGER->AddGathering( pFlag ) == false )
				{
					assert(0);
					OBJECTMANAGER->RemoveGathering( pFlag->GetObjectID() );
					pF->mFlagIndex = 0;
				}
				else
				{
					pFlag->SendRegenSync();
					pF->mFlagIndex = pFlag->GetObjectID();
				}
			}
			else
			{
				assert(0);
				NETWORK2->PostServerEvent( "RegenFlag() : flag create fail" );
			}
		}
		else
		{
			assert(0);
			NETWORK2->PostServerEvent( "RegenFlag() : flag regen pos count 0" );
		}
	}
	else
	{
		NETWORK2->PostServerEvent( "RegenFlag() : flag info NULL" );
	}

	/// 정보 갱신
	SendFlagInfo( aryPos, false );
}

bool cDeathMatch2Object::DelInfRegenFlag( unsigned long playerIdx, float x, float y )
{ 
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
		{
			NETWORK2->PostServerEvent("cDeathMatch2Object::GetFlagOwnerIndex pF == NULL[%d,%d]", i, playerIdx );
			continue;
		}

		if( pF->mFlagOwnerIndex == playerIdx )
			RegenFlag( i, x, y );
	}

	return false;
}

bool cDeathMatch2Object::IsGathering( unsigned long playerIdx, unsigned long gatheringIdx )
{
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return false;

	bool haveFlag = pPlayer->IsInfluenceHave( PVP_DM2_FLAG_INFLUENCE );
	if( haveFlag == true )
	{
		if( pPlayer->GetPvPDMTeam() == ePVPDM_TEAMTYPE_A )
		{
			for( int i = 0 ; i < mAHoistCnt ; ++i )
			{
				if( mAteamHoist[i].mObjectIdx == gatheringIdx )
					return true;
			}
		}
		else if( pPlayer->GetPvPDMTeam() == ePVPDM_TEAMTYPE_B )
		{
			for( int i = 0 ; i < mBHoistCnt ; ++i )
			{
				if( mBteamHoist[i].mObjectIdx == gatheringIdx )
					return true;
			}
		}
		else
			return false;
	}
	else
	{
		for( int i = 0 ; i < mFlagCnt ; ++i )
		{
			if( mFlagInfo[i].mFlagIndex == gatheringIdx )
				return true;
		}
	}

	return false;
}

void cDeathMatch2Object::ScoreUpdate( unsigned long killIdx, ePVPDM_TEAM_TYPE killerTeam, 
									 unsigned long deadIdx, ePVPDM_TEAM_TYPE deadTeam )
{
	if( mProcess < ePVPDM_PROCESS_FIGHT_MINIMUM || ePVPDM_PROCESS_FIGHT < mProcess )
		return;

	if( killerTeam >= ePVPDM_TEAMTYPE_MAX || deadTeam >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return;
	}

	/// 최초 킬포인트 획득팀 셋팅
	if( mFirstKillTeam == ePVPDM_TEAMTYPE_MAX )
		mFirstKillTeam = killerTeam;

	/// 깃발 보유자 사망시 처리
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		if( mFlagInfo[i].mFlagOwnerIndex == deadIdx )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( mFlagInfo[i].mFlagOwnerIndex );
			if( player )
			{
				RegenFlag( i, player->GetXPos(), player->GetYPos() );
			}
			else
			{
				assert(0);
				EnableNewGen( i );
			}
			mFlagInfo[i].mFlagOwnerIndex = 0;
			mFlagInfo[i].mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;

			break;
		}
	}

	/// 죽인자 kill 증가
	cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)mTeamInfo[killerTeam].mMemberMap.GetAt( killIdx );
	if( pInfo )
	{
		pInfo->mKillCnt++;

		/// 점수 메세지 발송
		MSG_SYN_PVP_DM2_PLAYER_SCORE msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM2_PLAYER_SCORE_SYN;
		msg.mTeamType = (unsigned char)killerTeam;
		msg.mPlayerIdx = killIdx;
		msg.mPlayerScore = pInfo->mKillCnt;
		TeamQuickSend( killerTeam, (char*)&msg, sizeof( MSG_SYN_PVP_DM2_PLAYER_SCORE ) );
	}
	else
	{
		assert(0);
	}

	/// 죽은자 death 증가
	pInfo = (cPvPDMMemberInfo*)mTeamInfo[deadTeam].mMemberMap.GetAt( deadIdx );
	if( pInfo )
	{
		pInfo->mDeathCnt++;

		/// 점수 메세지 발송
		MSG_SYN_PVP_DM2_PLAYER_SCORE msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM2_PLAYER_SCORE_SYN;
		msg.mTeamType = (unsigned char)killerTeam;
		msg.mPlayerIdx = deadIdx;
		msg.mPlayerScore = pInfo->mDeathCnt;
		TeamQuickSend( deadTeam, (char*)&msg, sizeof( MSG_SYN_PVP_DM2_PLAYER_SCORE ) );
	}
	else
	{
		assert(0);
	}
}

void cDeathMatch2Object::Result()
{
	mProcess = ePVPDM_PROCESS_FIGHTEND;
	mProcessEndTime = NETWORK2->GetAccumTime() + PVP_DM_OUTWAIT_TIME;

	///포인트 달성 체크
	if( mpDMBlockInfo )
	{
		if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint >= mpDMBlockInfo->mGoalPoint )
		{
			SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_POINT );
			return;
		}
		if( mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint >= mpDMBlockInfo->mGoalPoint )
		{
			SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_POINT );
			return;
		}
	}
	else
	{
		assert(0);
	}

	/// 포인트 우위 체크
	if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint > mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_TIME );
		return;
	}
	else if( mTeamInfo[ePVPDM_TEAMTYPE_A].mPoint < mTeamInfo[ePVPDM_TEAMTYPE_B].mPoint )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_TIME );
		return;
	}

	/// 최초 게양팀
	if( mFirstFlagTeam == ePVPDM_TEAMTYPE_A )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_FIRSTFLAG );
		return;
	}
	if( mFirstFlagTeam == ePVPDM_TEAMTYPE_B )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_FIRSTFLAG );
		return;
	}

	///// 마지막 깃발 보유
	//if( mFlagOwnerTeam == ePVPDM_TEAMTYPE_A )
	//{
	//	SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_LASTFLAG );
	//	return;
	//}
	//if( mFlagOwnerTeam == ePVPDM_TEAMTYPE_B )
	//{
	//	SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_LASTFLAG );
	//	return;
	//}

	cBaseDeathMatch::Result();
}

void cDeathMatch2Object::OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType )
{
	cBaseDeathMatch::OutMember( playerIdx, teamType );

	/// 깃발 보유자이면


	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		if( mFlagInfo[i].mFlagOwnerIndex == playerIdx )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( mFlagInfo[i].mFlagOwnerIndex );
			if( player )
			{
				RegenFlag( i, player->GetXPos(), player->GetYPos() );
			}
			else
			{
				assert(0);
				EnableNewGen( i );

				mFlagInfo[i].mFlagOwnerIndex = 0;
				mFlagInfo[i].mFlagOwnerTeam = ePVPDM_TEAMTYPE_MAX;

				SendFlagInfo( i, false );
			}
		}
	}

	/// 인원 패배 조건에 해당하는지 확인
	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo )
	{
		if( ( ePVPDM_PROCESS_FIGHT_MINIMUM == mProcess && mTeamInfo[teamType].mMemberCnt == 0 ) ||
			( ePVPDM_PROCESS_FIGHT == mProcess && mTeamInfo[teamType].mMemberCnt < pDMInfo->mTeamPlayerMin ) )
		{
			mProcess = ePVPDM_PROCESS_FIGHTEND;
			mProcessEndTime = NETWORK2->GetAccumTime() + PVP_DM_OUTWAIT_TIME;

			if( teamType == ePVPDM_TEAMTYPE_A )
				SendResult( ePVPDM_TEAMTYPE_B,ePVPDM_ENDTYPE_MEMBER );
			if( teamType == ePVPDM_TEAMTYPE_B )
				SendResult( ePVPDM_TEAMTYPE_A,ePVPDM_ENDTYPE_MEMBER );
		}
	}
	else
	{
		assert(0);
	}
}

void cDeathMatch2Object::ChangeGatheringState( unsigned long gIdx, unsigned long playerIdx )
{
	if( mProcess > ePVPDM_PROCESS_FIGHT )
		return;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == 0 )
	{
		assert(0);
		return;
	}

	/// 깃발을 뽑은것인지 체크
	sFlagInfo* pF = NULL;
	sFlagInfo* pOwn = NULL;
	unsigned char aryPos = 0;
	unsigned char ownPos = 0;
	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		/// 뽑혔으면 정보를 만든다.
		if( mFlagInfo[i].mFlagIndex == gIdx )
		{
			pF = &mFlagInfo[i];
			aryPos = i;
		}

		/// 꼽는 경우에 필요한 정보 기록
		if( mFlagInfo[i].mFlagOwnerIndex == playerIdx )
		{
			pOwn = &mFlagInfo[i];
			ownPos = i;
		}
	}

	if( pF != NULL )
	{
		/// 깃발이 뽑혔다.
		pF->mFlagIndex = 0;
		pF->mFlagOwnerIndex = playerIdx;
		pF->mFlagOwnerTeam = (ePVPDM_TEAM_TYPE)pPlayer->GetPvPDMTeam();

		/// 버프 부여



		if( pF->mFlagOwnerIndex != 0 )
		{
			/// 버프 부여



			sObject object = { eOBJECTTYPE_PLAYER, pF->mFlagOwnerIndex };
			SKILLMANAGER->AddInfSwitch( object, object, PVP_DM2_FLAG_INFLUENCE, 0, true );
		}

		SendFlagInfo( aryPos, false );

		return;
	}

	if( pOwn == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::ChangeGatheringState pOwn == NULL");
		return;
	}

	unsigned long hoistPos = 0;
	for( unsigned char i = 0 ; i < mAHoistCnt ; ++i )
	{
		if( mAteamHoist[i].mObjectIdx == gIdx )
		{
			hoistPos = i + 1;
			mAteamHoist[i].mNextRegenTime = NETWORK2->GetAccumTime() + 10 * SECOND;
			mAteamHoist[i].mObjectIdx = 0;
		}
	}

	for( unsigned char i = 0 ; i < mBHoistCnt ; ++i )
	{
		if( mBteamHoist[i].mObjectIdx == gIdx )
		{
			hoistPos = FLAG_MAX + i + 1;
			mBteamHoist[i].mNextRegenTime = NETWORK2->GetAccumTime() + 10 * SECOND;
			mBteamHoist[i].mObjectIdx = 0;
		}
	}

	unsigned long curOwnerIdx = pOwn->mFlagOwnerIndex;
	if( pOwn->mFlagOwnerIndex != 0 )
	{
		/// 버프 제거
		sObject object = { eOBJECTTYPE_PLAYER, pOwn->mFlagOwnerIndex };
		SKILLMANAGER->DeleteInfluenceClassIdx( object, PVP_DM2_FLAG_INFLUENCE );

		pOwn->mFlagOwnerIndex = 0;
	}

	/// Error시 재시작!!
	if( curOwnerIdx != playerIdx )
	{
		assert(0);
		EnableNewGen( ownPos );
		return;
	}
	if( pOwn->mFlagOwnerTeam != (ePVPDM_TEAM_TYPE)pPlayer->GetPvPDMTeam() )
	{
		assert(0);
		EnableNewGen( ownPos );
		return;
	}
	if( pOwn->mFlagOwnerTeam >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		EnableNewGen( ownPos );
		return;
	}

	/// 점수 갱신
	mTeamInfo[pOwn->mFlagOwnerTeam].mPoint++;
	if( mFirstFlagTeam == ePVPDM_TEAMTYPE_MAX )
		mFirstFlagTeam = pOwn->mFlagOwnerTeam;


	ePVPDM_TEAM_TYPE ownerTeam = pOwn->mFlagOwnerTeam;
	ePVPDM_TEAM_TYPE viewTeam = pOwn->mFlagOwnerTeam;
	if( ownerTeam == ePVPDM_TEAMTYPE_A )
		viewTeam = ePVPDM_TEAMTYPE_B;
	else
		viewTeam = ePVPDM_TEAMTYPE_A;

	/// 점수 메세지 발송
	MSG_SYN_PVP_DM2_SCORE msg;
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_DM2_SCORE_SYN;
	msg.mTeamType = (unsigned char)pOwn->mFlagOwnerTeam;
	msg.mTotalScore = mTeamInfo[pOwn->mFlagOwnerTeam].mPoint;
	msg.mDelFlagIdx = ownPos;
	msg.mDelHoistIdx = hoistPos;
	TeamQuickSend( viewTeam, (char*)&msg, sizeof( msg ) );

	/// 점수 메세지 발송
	MSG_SYN_PVP_FLAGCOMPLETE_SCORE msgComplete;
	msgComplete.Category = NM_PVP;
	msgComplete.Protocol = NM_PVP_DM2_FLAGCOMPLETE_SYN;
	msgComplete.mTeamType = (unsigned char)pOwn->mFlagOwnerTeam;
	msgComplete.mTotalScore = mTeamInfo[pOwn->mFlagOwnerTeam].mPoint;
	msgComplete.mDelFlagIdx = ownPos;
	msgComplete.mDelHoistIdx = hoistPos;
	msgComplete.mPlayerIdx = playerIdx;
	TeamQuickSend( ownerTeam, (char*)&msgComplete, sizeof( msgComplete ) );

	/// 게임 종료 체크

	/// 점수 승리조건 체크
	if( mTeamInfo[pOwn->mFlagOwnerTeam].mPoint >= mpDMBlockInfo->mGoalPoint )
		mScoreEnd = true; 
	else
		EnableNewGen( ownPos );

	return;
}

void cDeathMatch2Object::EnableNewGen()
{
	unsigned long flagIdx = PVPMANAGER->GetMapIndex() * MAPNUMBER_UNIT + 3;
	const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( 0, flagIdx );
	if( header )
		mFlagCnt = (unsigned char)header->mRegenDetail.GetSize();

	for( unsigned char i = 0 ; i < mFlagCnt ; ++i )
	{
		sFlagInfo* pF = &mFlagInfo[i];
		if( pF == NULL )
			continue;

		pF->mNewFlagGen = true;
		pF->mNewFlagWaitTime = NETWORK2->GetAccumTime() + ( ( 60 * SECOND ) * ( i ) );
	}
}

void cDeathMatch2Object::EnableNewGen( unsigned char aryPos )
{
	if( mFlagCnt <= aryPos )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::EnableNewGen mFlagCnt <= aryPos[%d,%d]", mFlagCnt, aryPos );
		return;
	}

	sFlagInfo* pF = &mFlagInfo[aryPos];
	if( pF == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::EnableNewGen pF == NULL[%d]", aryPos );
		return;
	}

	pF->mNewFlagGen = true;
	pF->mNewFlagWaitTime = NETWORK2->GetAccumTime() + ( 10 * SECOND );
}

void cDeathMatch2Object::SendFlagInfo( unsigned char aryPos, bool newGen )
{
	if( mFlagCnt <= aryPos )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::SendFlagInfo mFlagCnt <= aryPos[%d,%d]", mFlagCnt, aryPos );
		return;
	}

	sFlagInfo* pF = &mFlagInfo[aryPos];
	if( pF == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::SendFlagInf pF == NULL[%d,%d]", aryPos, newGen );
		return;
	}

	MSG_SYN_PVP_DM2_INFO msg;
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_DM2_INFO_SYN;
	msg.mFlagOwnerIdx = pF->mFlagOwnerIndex;
	msg.mGatherIdx = aryPos;

	cGathering* pGather = OBJECTMANAGER->GetGathering( pF->mFlagPointIndex );
	if( pGather == 0 )
		msg.mFlagPointPos = NiPoint2::ZERO;
	else
		msg.mFlagPointPos = pGather->GetPos();

	msg.mPrintNewGenMsg = newGen;
	DMQuickSend( (char*)&msg, sizeof(MSG_SYN_PVP_DM2_INFO) );

	SendFlagPos( aryPos );
}

void cDeathMatch2Object::SendFlagPos( unsigned char aryPos )
{
	if( mFlagCnt <= aryPos )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::SendFlagPos mFlagCnt <= aryPos[%d,%d]", mFlagCnt, aryPos );
		return;
	}

	sFlagInfo* pF = &mFlagInfo[aryPos];
	if( pF == NULL )
	{
		NETWORK2->PostServerEvent("cDeathMatch2Object::SendFlagPos pF == NULL[%d]", aryPos );
		return;
	}

	pF->mSendFlagPosTime = NETWORK2->GetAccumTime() + 1000;

	MSG_SYN_PVP_DM2_FLAGPOS msg;
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_DM2_FLAGPOS_SYN;
	msg.mGatherIdx = aryPos;

	msg.mFlagPos = NiPoint2::ZERO;
	if( pF->mFlagOwnerIndex != 0 )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pF->mFlagOwnerIndex );
		if( pPlayer )
			msg.mFlagPos = pPlayer->GetPos();
	}
	else if( pF->mFlagIndex != 0 )
	{
		cGathering* pGather = OBJECTMANAGER->GetGathering( pF->mFlagIndex );
		if( pGather )
			msg.mFlagPos = pGather->GetPos();
	}
	else
	{
		return;
	}

	if( pF->mFlagOwnerTeam != ePVPDM_TEAMTYPE_A )
		TeamQuickSend( ePVPDM_TEAMTYPE_A, (char*)&msg, sizeof(MSG_SYN_PVP_DM2_FLAGPOS) );

	if( pF->mFlagOwnerTeam != ePVPDM_TEAMTYPE_B )
		TeamQuickSend( ePVPDM_TEAMTYPE_B, (char*)&msg, sizeof(MSG_SYN_PVP_DM2_FLAGPOS) );
}
