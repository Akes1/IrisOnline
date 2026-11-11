#include "stdafx.h"
#include "PVPManager.h"
#include "Protocol.h"
#include "ChatManager.h"

#include "StageManager.h"
#include "ObjectManager.h"
#include "hero.h"
#include "GameUIManager.h"
#include "DMScoreWindow.h"
#include "Npc_Common.h"

#include "SceneManager.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "PartyUnionManager.h"
#include "GameResourceManager.h"
#include "PvPScript.h"
#include "DMMsgWindow.h"

#include "InputSystem.h"
#include "ShortcutManager.h"

cPVPManager* cPVPManager::mpSingleton = 0;


cPVPManager::cPVPManager()
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	mPVPType = MAP_NONE;
	mPVPState = ePVPState_None;

	mLeaderIdx = 0;
	mMinLevel = 0;
	mMaxLevel = 0;

	mWaitLeftTime = 0;
	mFightLeftTime = 0;
	mReadyPrintTime = ULONG_MAX;

	mTeamAScore = 0;
	mTeamBScore = 0;

	mGoalPoint = 0;

	mUpdateRankInfo = false;

	mRequestLock = false;
}

cPVPManager::~cPVPManager()
{
	if( mTeamInfoMap.GetSize() != 0 )
	{
		cMyTeamInfoMap::cIterator i = mTeamInfoMap.Begin();
		cMyTeamInfoMap::cIterator end = mTeamInfoMap.End();
		for( ; i != end; ++i )
		{
			sMyTeamInfo* info = (sMyTeamInfo*)(*i).mSecond;
			SAFE_DELETE( info );
		}
		mTeamInfoMap.Clear();
	}
}

int PvPRankSort( const void *arg1, const void *arg2 )
{
	sMyTeamInfo* score1 = (sMyTeamInfo*)arg1;
	sMyTeamInfo* score2 = (sMyTeamInfo*)arg2;

	assert(score1);
	assert(score2);

	if( score2->mKillCnt > score1->mKillCnt )
	{
		return 1;
	}
	else if( score2->mKillCnt < score1->mKillCnt )
	{
		return -1;
	}
	else
	{
		///
		return score1->mDeathCnt - score2->mDeathCnt;
	}
}


int PvPResultSort( const void *arg1, const void *arg2 )
{
	sMyTeamInfo* score1 = (sMyTeamInfo*)arg1;
	sMyTeamInfo* score2 = (sMyTeamInfo*)arg2;

	assert(score1);
	assert(score2);

	return score2->mScoreResult - score1->mScoreResult;
}

void cPVPManager::Open()
{
	if( IsEnableBattle() == false )
		return;

	/// 점수판 보여주기
	cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
	if( win )
	{
		win->Clear();

		if( mPVPType != MAP_NONE )
		{
			sDMInfo* dmInfo = PVPSCRIPT->GetDmInfo( mPVPType );
			if( dmInfo )
			{
				sDMBlockInfo* dmBlockInfo = dmInfo->GetBlockInfo( HERO->GetLevel() );
				if( dmBlockInfo )
				{
					mGoalPoint = dmBlockInfo->mGoalPoint;
				}
			}
		}

		/// 남쪽/북쪽인지 체크 & 데스매치인지 체크해서 석상 이미지 셋팅하기
		if( HERO && mPVPType == MAP_DEATHMATCH )
			win->SetStoneImage( HERO->GetPVPTeam() );

		win->Show();
	}

	cDMRankingListWindow* rankWin = GAMEUI->GetDMRankingListWindow();
	if( rankWin )
		rankWin->Clear();

	mUpdateRankInfo = true;

	CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 821 ) );
}

void cPVPManager::Close()
{
	ChangeState( ePVPState_None );

	if( mTeamInfoMap.GetSize() != 0 )
	{
		cMyTeamInfoMap::cIterator i = mTeamInfoMap.Begin();
		cMyTeamInfoMap::cIterator end = mTeamInfoMap.End();
		for( ; i != end; ++i )
		{
			sMyTeamInfo* info = (sMyTeamInfo*)(*i).mSecond;
			SAFE_DELETE( info );
		}
		mTeamInfoMap.Clear();
	}

	mRequestLock = false;
}

void cPVPManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mPVPState == ePVPState_None )
		return;

	if( mPVPState >= ePVPState_Enable && mPVPState < ePVPState_Result )
	{
		cKeyboardAgent* keyAgent = INPUTSYS->GetKeyboard();
		if( keyAgent->GetKeyPressed( KEY_SCORE_BOARD ) == true )
		{
			OpenRank( mUpdateRankInfo );
			mUpdateRankInfo = false;
		}
		else
		{
			cDMRankingListWindow* win = GAMEUI->GetDMRankingListWindow();
			if( win )
				win->Hide( false );
		}
	}

	switch( mPVPState )
	{
	case ePVPState_Notice:
		{
			// 남은시간 갱신
			if( mWaitLeftTime > deltaTime )
			{
				mWaitLeftTime -= deltaTime;
			}
			else
			{
				mWaitLeftTime = 0;

				ChangeState( ePVPState_None );
			}
		}
		break;
	case ePVPState_Count:
		{
			// 남은시간 갱신
			if( mWaitLeftTime > deltaTime )
			{
				mWaitLeftTime -= deltaTime;
			}
			else
			{
				mWaitLeftTime = 0;

				ChangeState( ePVPState_Wait );
			}
		}
		break;
	case ePVPState_Ready:
		{
			if( mFightLeftTime == 0 )
				return;

			if( mFightLeftTime < deltaTime )
			{
				mFightLeftTime = 0;
				return;
			}
			else
				mFightLeftTime -= deltaTime;

			if( mFightLeftTime <= mReadyPrintTime )
			{
				/// chat message
				cStringT Message;
				Message.Format( GAMERESOURCEMAN->GetGameText( 835 ), mReadyPrintTime/1000 );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, Message.Cstr() );

				if( mReadyPrintTime == 30000 )
					mReadyPrintTime = 10000;
				else if( mReadyPrintTime <= 10000 )
					mReadyPrintTime -= 1000;
			}
		}
		break;
	}
}

void cPVPManager::ChangeState( ePVPState state )
{
	mPVPState = state;

	switch( mPVPState )
	{
	case ePVPState_None:
		{
			// 초기화
			mPVPType = MAP_NONE;
			mLeaderIdx = 0;
			mMinLevel = 0;
			mMaxLevel = 0;
			mFightLeftTime = 0;
			mWaitLeftTime = 0;
			mReadyPrintTime = ULONG_MAX;
			mUpdateRankInfo = false;
			mTeamAScore = 0;
			mTeamBScore = 0;
			mGoalPoint = 0;

			cDMMsgWindow* win = GAMEUI->GetDMMsgWindow();
			if( win )
			{
				win->Hide();
				win->Clear();
			}
		}
		break;
	case ePVPState_Notice:	// 입장알림
		{
			// 전장 입장 알림
			cDMMsgWindow* win = GAMEUI->GetDMMsgWindow();
			if( win )
				win->Show();
		}
		break;	
	case ePVPState_Count:	// 입장 대기
		{
			// 윈도우 갱신
			cDMMsgWindow* win = GAMEUI->GetDMMsgWindow();
			if( win )
				win->Show();
		}
		break;
	case ePVPState_Wait:
		{
			cDMMsgWindow* win = GAMEUI->GetDMMsgWindow();
			if( win )
			{
				win->Hide();
				win->Clear();
			}

			// 맵이동 대기 띄우기
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_PVP_READY_MAPCHANGE, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 843 ),
				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
				);
		}
		break;
	case ePVPState_MapChangeWait:	// 맵 체인지 대기
		{
			cDMMsgWindow* win = GAMEUI->GetDMMsgWindow();
			if( win )
			{
				win->Hide();
				win->Clear();
			}

			// 입장 대기 마감
			HERO->CancelByMapchange();

			/// 맵이동 대기
			if( HERO->GetState() == eOBJECT_STATE_ATTACK || HERO->GetState() == eOBJECT_STATE_MOVE )
			{
				HERO->SetState( eOBJECT_STATE_IDLE );

				/// 따라가기 종료
				if( HERO->IsFollowing() )
					HERO->ReleaseFollowPlayer();
			}

			// 맵이동 대기 띄우기
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_PVP_READY_MAPCHANGE, eSKIN_NONE,
				GAMERESOURCEMAN->GetGameText( 843 ),
				GAMERESOURCEMAN->GetGameText( 2 ) // 알림
				);
		}
		break;
	case ePVPState_Enable:	// 전투 가능(맵이동후)
		{

		}
		break;
	case ePVPState_Ready:	// 전투 대기
		{
			mFightLeftTime = PVP_DM_READY_PRINT1_TIME;
			mReadyPrintTime = PVP_DM_READY_PRINT2_TIME;

			/// chat message
			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 835 ), mFightLeftTime/1000 );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, Message.Cstr() );
		}
		break;
	case ePVPState_Fight:	// 전투
		{
			/// 전투 알림 시작
			cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
			if( win )
			{
				win->SetTime( mReadyPrintTime );
				win->ActiveTime();
			}

			/// npc 삭제
			OBJECTMAN->FadeOutNpcAll();

			SCENEMAN->RemoveAllDoorNode();

			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			if( mini )
				mini->ClearNpc();

			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( wide )
				wide->ClearNpc();

			unsigned int textIdx = 0;
			switch( mPVPType )
			{
			case MAP_DEATHMATCH:		textIdx = 859;			break;
			case MAP_FLAG:				textIdx = 860;			break;
			default:					assert(0);				return;
			}

			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 822 ), GAMERESOURCEMAN->GetGameText( textIdx ) );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)Message.Cstr() );

		}
		break;
	case ePVPState_Result:	// 결과
		{
			/// 시간 정지
			cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
			if( win )
				win->DeActiveTime();

			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				mini->ClearFlag();
				mini->ClearFlagPoint();
				mini->ClearStone();

				wide->ClearFlag();
				wide->ClearFlagPoint();
				wide->ClearStone();
			}
		}
		break;
	}
}

void cPVPManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_PVP_DM_NOTICE_SYN:
		{
			MSG_SYN_PVP_DM_NOTICE* pmsg = (MSG_SYN_PVP_DM_NOTICE*)tempMsg;

			unsigned int textIdx = 0;
			switch( pmsg->mPvPType )
			{
			case MAP_DEATHMATCH:		textIdx = 859;			break;
			case MAP_FLAG:				textIdx = 860;			break;
			default:					assert(0);				return;
			}

			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 838 ), GAMERESOURCEMAN->GetGameText( textIdx ), pmsg->mLeftTime );
			CHATMANAGER->AddSystemMsg( eSYSTEM_PVP, Message.Cstr() );
		}
		break;
	case NM_PVP_READYQUESTION_SYN:
		{
			MSG_SYN_PVP_READYQUESTION* pmsg = (MSG_SYN_PVP_READYQUESTION*)tempMsg;

			mPVPType = pmsg->mPvPType;
			mMinLevel = pmsg->mMinLevel;
			mMaxLevel = pmsg->mMaxLevel;
			mWaitLeftTime = pmsg->mLeftTime;

			// 입장 알림 메세지
			ChangeState( ePVPState_Notice );
		}
		break;
	case NM_PVP_READY_RES:
		{
			MSG_ERROR* pmsg = (MSG_ERROR*)tempMsg;

			if( pmsg->ErrorCode == 0 )
			{
				// 입장 카운트 대기
				ChangeState( ePVPState_Count );
			}
			else
			{
				// 입장대기 실패
				ChangeState( ePVPState_None );

				CHATMANAGER->AddSystemMsg( eSYSTEM_PVP, GAMERESOURCEMAN->GetGameText( 845 ) );
			}
		}
		break;
	case NM_PVP_READYCANCEL_RES:
		{
			// 입장 대기 취소
			MSG_ERROR* pmsg = (MSG_ERROR*)tempMsg;

			if( pmsg->ErrorCode == 0 )
			{	
				ChangeState( ePVPState_None );

				// 취소 
				CHATMANAGER->AddSystemMsg( eSYSTEM_PVP, GAMERESOURCEMAN->GetGameText( 847 ) );
			}
			else
			{
				// 취소 불가
				CHATMANAGER->AddSystemMsg( eSYSTEM_PVP, GAMERESOURCEMAN->GetGameText( 848 ) );
			}
		}
		break;
	case NM_PVP_READYCLOSE_SYN:
		{
			// 맵체인지 대기
			ChangeState( ePVPState_MapChangeWait );
		}
		break;
	case NM_PVP_JOIN_SYN:
		{
			MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)tempMsg;

			UIMAN->DestroyMsgBox( eMBEVENT_PVP_READY_MAPCHANGE );

			if( pmsg->ErrorCode == 0 )
			{
				// 맵이동
				STAGEMAN->SetMapIdx( pmsg->MapNum );
				STAGEMAN->SetMapMode( pmsg->MapMode );
				STAGEMAN->ChangeStage( eStage_MapChange );

				// 전장 타입 저장
				mPVPType = pmsg->MapNum;

				// 스테이지 변경후 PVP 전장 세팅
				ChangeState( ePVPState_Enable );

				/// 스테이지 변경후 HERO객체 생성
				OBJECTMANAGER->AddHeroFromServerMsg( pmsg );
			}
			else
			{
				ChangeState( ePVPState_None );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 839 ),
					GAMERESOURCEMAN->GetGameText( 2 )
					);
			}
		}
		break;
	case NM_PVP_DM_MEMBERIN_RES:
		{
			/// 동맹세력 분류
			MSG_RES_PVP_DM_MEMBERIN* pmsg = (MSG_RES_PVP_DM_MEMBERIN*)tempMsg;

			cDMScoreWindow* scoreWin = (cDMScoreWindow*)GAMEUI->GetDMScoreWindow();
			if( scoreWin )
			{
				scoreWin->SetTeamImage( pmsg->mFireForce, pmsg->mWaterForce, pmsg->mWindForce, pmsg->mEarthForce );
			}

			/// set member info
			for( unsigned int i=0; i<pmsg->mMemberCnt; i++ )
			{
				AddTeamPlayerInfo( &(pmsg->mMemberInfo[i]) );
			}
		}
		break;
	case NM_PVP_DM_MEMBERIN_SYN:
		{
			MSG_SYN_PVP_DM_MEMBERIN* pmsg = (MSG_SYN_PVP_DM_MEMBERIN*)tempMsg;

			AddTeamPlayerInfo( &(pmsg->mMemberInfo) );
		}
		break;
	case NM_PVP_DM_MEMBEROUT_SYN:
		{
			MSG_SYN_PVP_DM_MEMBEROUT* pmsg = (MSG_SYN_PVP_DM_MEMBEROUT*)tempMsg;

			DelTeamPlayerInfo( pmsg->mCharacterIdx );
		}
		break;
	case NM_PVP_DM_READY_SYN:
		{
			/// 1분 카운트 시작
			ChangeState( ePVPState_Ready );
		}
		break;
	case NM_PVP_DM_NOTREADY_SYN:
		{
			ChangeState( ePVPState_Enable );

			/// 게임 진행 불가
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_PVP_DM_NOTREADY, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 839 ),	// pvp out
				GAMERESOURCEMAN->GetGameText( 301 ),	// 알림
				0, 0, PVP_DM_READY_OUT_TIME, false, true );
		}
		break;
	case NM_PVP_DM_FIGHT_SYN:
		{
			MSG_SYN_PVP_FIGHT* pmsg = (MSG_SYN_PVP_FIGHT*)msg;

			mReadyPrintTime = pmsg->mFightTime;			

			//
			ChangeState( ePVPState_Fight );
		}
		break;
	case NM_PVP_DM_JOINTIME_SYN:
		{
			MSG_SYN_PVP_JOINTIME* pmsg = (MSG_SYN_PVP_JOINTIME*)msg;

			cStringT Message;
			Message.Format( GAMERESOURCEMAN->GetGameText( 835 ), pmsg->mFightLeftTime / SECOND );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, Message.Cstr() );
		}
		break;
 	case NM_PVP_DM_SCORE_SYN:
 		{
 			/// 점수 변동
 			MSG_SYN_PVP_DM_SCORE* pmsg = (MSG_SYN_PVP_DM_SCORE*)msg;
 
 			if( pmsg->mTeamType == ePVPDM_TEAMTYPE_A )
 			{
 				mTeamAScore = pmsg->mTotalScore;
 			}
 			else
 			{
 				mTeamBScore = pmsg->mTotalScore;
 			}
 
 			cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
 			if( win )
 				win->UpdatePoint( mTeamAScore, mTeamBScore );
 
 			///
 			if( HERO->GetPVPTeam() == pmsg->mTeamType )
 				UpdateTeamPlayerInfo( pmsg->mPlayerIdx, pmsg->mPlayerScore, true );
 			else
 				UpdateTeamPlayerInfo( pmsg->mPlayerIdx, pmsg->mPlayerScore, false );
 		}
 		break;
	case NM_PVP_DM2_SCORE_SYN:
		{
			/// 점수를 뺐긴쪽에 오는 메시지
			/// 점수 변동
			MSG_SYN_PVP_DM2_SCORE* pmsg = (MSG_SYN_PVP_DM2_SCORE*)msg;

			if( pmsg->mTeamType == ePVPDM_TEAMTYPE_A )
			{
				mTeamAScore = pmsg->mTotalScore;
			}
			else
			{
				mTeamBScore = pmsg->mTotalScore;
			}

			cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
			if( win )
				win->UpdatePoint( mTeamAScore, mTeamBScore );

			// 새 깃발 리젠 시간 알림
			CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 1101 ), COLOR_YELLOW );

			// 깃발 안보이기
			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				mini->DeleteFlag( pmsg->mDelFlagIdx );
				wide->DeleteFlag( pmsg->mDelFlagIdx );

				mini->DeleteFlagPoint( pmsg->mDelHoistIdx );
				wide->DeleteFlagPoint( pmsg->mDelHoistIdx );
			}

			PARTYUNIONMAN->ClearFlagPlayer();
		}
		break;
	case NM_PVP_DM2_FLAGCOMPLETE_SYN:
		{
			/// 깃발을 꽂은 쪽에 오는 메시지
			/// 점수 변동
			MSG_SYN_PVP_FLAGCOMPLETE_SCORE* pmsg = (MSG_SYN_PVP_FLAGCOMPLETE_SCORE*)msg;

			// 아군 게양 알림
			if( HERO->GetPVPTeam() == pmsg->mTeamType )
			{
				LPCTSTR characterName = NULL;
				characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mPlayerIdx );

				cStringT msg;
				msg.Format( GAMERESOURCEMAN->GetGameText( 1104 ), characterName );

				if( characterName )
					CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
				else
					CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 1106 ), COLOR_YELLOW );
			}

			if( pmsg->mTeamType == ePVPDM_TEAMTYPE_A )
			{
				mTeamAScore = pmsg->mTotalScore;
			}
			else
			{
				mTeamBScore = pmsg->mTotalScore;
			}

			cDMScoreWindow* win = GAMEUI->GetDMScoreWindow();
			if( win )
				win->UpdatePoint( mTeamAScore, mTeamBScore );

			// 깃발 안보이기
			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				mini->DeleteFlag( pmsg->mDelFlagIdx );
				wide->DeleteFlag( pmsg->mDelFlagIdx );

				mini->DeleteFlagPoint( pmsg->mDelHoistIdx );
				wide->DeleteFlagPoint( pmsg->mDelHoistIdx );
			}

			PARTYUNIONMAN->ClearFlagPlayer();
		}
		break;
	case NM_PVP_DM2_FLAGDROP_SYN:
		{
			MSG_SYN_PVP_DM2_FLAGDROP* pmsg = (MSG_SYN_PVP_DM2_FLAGDROP*)msg;
			/// 아군 깃발 드롭 메시지 출력
			LPCTSTR characterName = NULL;
			characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mPlayerIdx );

			cStringT msg;
			msg.Format( GAMERESOURCEMAN->GetGameText( 1103 ), characterName );

			if( characterName )
				CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
			else
				CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 1105 ), COLOR_YELLOW );
		}		
		break;
	case NM_PVP_DM2_PLAYER_SCORE_SYN:
		{
			/// 점수 변동
			MSG_SYN_PVP_DM2_PLAYER_SCORE* pmsg = (MSG_SYN_PVP_DM2_PLAYER_SCORE*)msg;

			///
			if( HERO->GetPVPTeam() == pmsg->mTeamType )
				UpdateTeamPlayerInfo( pmsg->mPlayerIdx, pmsg->mPlayerScore, true );
			else
				UpdateTeamPlayerInfo( pmsg->mPlayerIdx, pmsg->mPlayerScore, false );
		}
		break;
	case NM_PVP_DM2_INFO_SYN:
		{
			MSG_SYN_PVP_DM2_INFO* pmsg = (MSG_SYN_PVP_DM2_INFO*)msg;

			// 신규 젠에 대한 메제시 출력여부
			if( pmsg->mPrintNewGenMsg == true )
			{
				CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 1100 ), COLOR_YELLOW );
			}

			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				float xpos = 0;
				float ypos = 0;
				cPlayer* player = OBJECTMAN->GetPlayer( pmsg->mFlagOwnerIdx );
				if( player )
				{
					xpos = player->GetXPos();
					ypos = player->GetYPos();
				}

				// 깃발 세팅 ( add or update )
				mini->AddFlag( pmsg->mGatherIdx, pmsg->mFlagOwnerIdx, xpos, ypos );
				wide->AddFlag( pmsg->mGatherIdx, pmsg->mFlagOwnerIdx, xpos, ypos );

				// 게양포인트 세팅
				//mini->AddFlagPoint( pmsg->mFlagPointPos.x, pmsg->mFlagPointPos.y );
				//wide->AddFlagPoint( pmsg->mFlagPointPos.x, pmsg->mFlagPointPos.y );

				/// 아군 깃발 습득 메시지 출력
				LPCTSTR characterName = NULL;
				characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mFlagOwnerIdx );

				cStringT msg;
				msg.Format( GAMERESOURCEMAN->GetGameText( 1102 ), characterName );

				if( characterName )
					CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
			}

			/// 플래그 매치에서 깃발 가진 유저 설정
			if( pmsg->mFlagOwnerIdx != 0 )
			{
				/// 플래그 매치에서 깃발 가졌을 경우 파티창, 파티연합창 ui 표시
				PARTYUNIONMAN->SetFlagPlayer( pmsg->mFlagOwnerIdx );
			}
			else
			{
				/// 플래그 매치에서 깃발 든 유저 체크
				PARTYUNIONMAN->ClearFlagPlayer();
			}
		}
		break;
	case NM_PVP_DM2_FLAGPOS_SYN:
		{
			MSG_SYN_PVP_DM2_FLAGPOS* pmsg = (MSG_SYN_PVP_DM2_FLAGPOS*)msg;

			// 상대편일 경우에만 온다
			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				// 깃발 세팅 ( add or update )
				mini->AddFlag( pmsg->mGatherIdx, 0, pmsg->mFlagPos.x, pmsg->mFlagPos.y );
				wide->AddFlag( pmsg->mGatherIdx, 0, pmsg->mFlagPos.x, pmsg->mFlagPos.y );
			}
		}
		break;
	case NM_PVP_DM2_HOIST_SYN:
		{
			// 깃발 게양포인트 생성
			MSG_SYN_PVP_DM2_HOIST*	pmsg = (MSG_SYN_PVP_DM2_HOIST*)msg;

			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				mini->AddFlagPoint( pmsg->mGatherIdx, pmsg->mPos.x, pmsg->mPos.y );
				wide->AddFlagPoint( pmsg->mGatherIdx, pmsg->mPos.x, pmsg->mPos.y );
			}
		}
		break;
	case NM_PVP_DM_LEADERSELECT_SYN:
		{
			MSG_SYN_PVP_DM_MEMBEROUT* pmsg = (MSG_SYN_PVP_DM_MEMBEROUT*)msg;
			mLeaderIdx = pmsg->mCharacterIdx;

			/// map ui setting
			cSmallMiniMapWindow* minimap = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* pWide = GAMEUI->GetMinimapWindow();

			if( minimap )
				minimap->SetPvpLeader( mLeaderIdx );
			if( pWide )
				pWide->SetPvpLeader( mLeaderIdx );

			PARTYUNIONMAN->SetPvpLeader( mLeaderIdx );

		}
		break;
	case NM_PVP_DM_LEADERDIE_SYN:
		{
			MSG_SYN_DM_LEADERDIE* pmsg = (MSG_SYN_DM_LEADERDIE*)msg;
			LPCTSTR characterName = NULL;
			characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mPlayerIdx );
			
			if( characterName )
			{
				cStringT msg;
				msg.Format( GAMERESOURCEMAN->GetGameText( 836 ), characterName );
				CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
			}
			else
				CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 877 ), COLOR_YELLOW );

			mLeaderIdx = 0;
		}
		break;
	case NM_PVP_DM_ENEMYLEADERDIE_SYN:
		{
			MSG_SYN_DM_ENEMYLEADERDIE* pmsg = (MSG_SYN_DM_ENEMYLEADERDIE*)msg;
			LPCTSTR characterName = NULL;
			characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mPlayerIdx );

			if( characterName )
			{
				cStringT msg;
				msg.Format( GAMERESOURCEMAN->GetGameText( 837 ), characterName );
				CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
			}
			else
				CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( 878 ), COLOR_YELLOW );


		}
		break;
	case NM_PVP_DM_STONE_DIE_SYN:
		{
			// 석상 파괴
			MSG_SYN_PVP_DM_STONE_DIE* pmsg = (MSG_SYN_PVP_DM_STONE_DIE*)msg;

			// 점수 세팅
			if( pmsg->mTeamType == ePVPDM_TEAMTYPE_A )
				mTeamAScore = pmsg->mTotalScore;
			else
				mTeamBScore = pmsg->mTotalScore;

			cDMScoreWindow* dmScoreWin = GAMEUI->GetDMScoreWindow();
			if( dmScoreWin )
				dmScoreWin->UpdatePoint( mTeamAScore, mTeamBScore );

			// 석상 삭제
			unsigned char loseTeam = (unsigned char)(( pmsg->mTeamType == ePVPDM_TEAMTYPE_A ) ? ePVPDM_TEAMTYPE_B : ePVPDM_TEAMTYPE_A);
			
			cSmallMiniMapWindow* miniMap = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wideMap = GAMEUI->GetMinimapWindow();
			if( miniMap && wideMap )
			{
				miniMap->DeleteStone( loseTeam );
				wideMap->DeleteStone( loseTeam );
			}

			cStringT msg;
			unsigned long nextGenTime = PVP_DM_STONE_REGEN_TIME / MINUTE;

			// 알림 메세지
			if( HERO->GetPVPTeam() == loseTeam )
			{
				// 루즈팀
				msg.Format( GAMERESOURCEMAN->GetGameText( 872 ), nextGenTime );
				CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
				if( dmScoreWin )
					dmScoreWin->SetStoneHP(0);
			}
			else
			{
				// 킬러팀
				LPCTSTR characterName =	PARTYUNIONMAN->GetPlayerName( pmsg->mKillerIndex );

				if( characterName != 0 )
					msg.Format( GAMERESOURCEMAN->GetGameText( 873 ), characterName, nextGenTime );
				else
					msg.Format( GAMERESOURCEMAN->GetGameText( 876 ), characterName, nextGenTime );

				CHATMANAGER->AddPvPNoticeMsg( msg.Cstr(), COLOR_YELLOW );
			}
		}
		break;
	case NM_PVP_DM_STONE_CREATE_SYN:
		{
			MSG_SYN_PVP_DM_STONE_CREATE* pmsg = (MSG_SYN_PVP_DM_STONE_CREATE*)msg;
			
			/// 해당 윈도우 최대 게이지 셋팅 & 게이지 최대치로 셋팅
			cDMScoreWindow* scoreWin = GAMEUI->GetDMScoreWindow();
			if( scoreWin )
			{
				if( HERO->GetPVPTeam() == pmsg->mTeamType )
				{
					scoreWin->SetStoneMaxHP( pmsg->mMaxHP );
					scoreWin->SetStoneHP( pmsg->mMaxHP );
				}

				/// 맵에 석상 표시
				cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
				cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();

				/// 어느팀의 석상이 생성됐는지 체크
				unsigned long msgIndex = 875;
 				if( HERO->GetPVPTeam() == pmsg->mTeamType )
					msgIndex = 874;

				/// 생성 위치 받기
				if( mini && wide )
				{
					mini->AddStone( pmsg->mPos.x, pmsg->mPos.y, pmsg->mTeamType );
					wide->AddStone( pmsg->mPos.x, pmsg->mPos.y, pmsg->mTeamType );
				}

				CHATMANAGER->AddPvPNoticeMsg( GAMERESOURCEMAN->GetGameText( msgIndex ), COLOR_YELLOW );
			}
			else
			{
				assert(0);
				return;
			}
		}
		break;
	case NM_PVP_DM_STONE_HP_SYN:
		{
			MSG_SYN_PVP_DM_STONE_HP* pmsg = (MSG_SYN_PVP_DM_STONE_HP*)msg;

			/// 해당 윈도우 게이지 셋팅
			cDMScoreWindow* scoreWin = GAMEUI->GetDMScoreWindow();
			if( scoreWin )
			{
				scoreWin->SetStoneHP( pmsg->mHP );
			}
			else
			{
				assert(0);
				return;
			}
		}
		break;
	case NM_PVP_DM_WIN_SYN:
	case NM_PVP_DM_LOSE_SYN:
		{
			MSG_SYN_PVP_DM_FIGHTEND* pmsg = (MSG_SYN_PVP_DM_FIGHTEND*)msg;

			ChangeState( ePVPState_Result );

			HERO->SetFirePoint( pmsg->mFirePointTotal );
			HERO->SetFireFriendly( pmsg->mFireFriendlyTotal );
			HERO->SetWaterPoint( pmsg->mWaterPointTotal );
			HERO->SetWaterFriendly( pmsg->mWaterFriendlyTotal );
			HERO->SetWindPoint( pmsg->mWindPointTotal );
			HERO->SetWindFriendly( pmsg->mWindFriendlyTotal );
			HERO->SetEarthPoint( pmsg->mEarthPointTotal );
			HERO->SetEarthFriendly( pmsg->mEarthFriendlyTotal );

			/// Result window show
			OpenResult( pmsg, (tempMsg->Protocol == NM_PVP_DM_WIN_SYN) );

			/// 결과 출력
			if( tempMsg->Protocol == NM_PVP_DM_WIN_SYN )
			{
				/// 결투종료 - 승리
				switch( pmsg->mEndType )
				{
				case ePVPDM_ENDTYPE_POINT:				/// 포인트 달성
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 823 ) );
					break;
				case ePVPDM_ENDTYPE_TIME:				/// 시간 초과(포인트량)
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 825 ) );
					break;
				case ePVPDM_ENDTYPE_MEMBER:				/// 인원수
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 827 ) );
					break;
				case ePVPDM_ENDTYPE_LEADERDIE:			/// 수장 사망 횟수
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 829 ) );
					break;
				case ePVPDM_ENDTYPE_FIRSTKILL:			/// 최초 킬러 보유
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 831 ) );
					break;	
				case ePVPDM_ENDTYPE_KILLTOTAL:			/// Kill 총합
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 850 ) );
					break;	
				case ePVPDM_ENDTYPE_RANDOM:			/// random
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 852 ) );
					break;	
				case ePVPDM_ENDTYPE_FIRSTFLAG:			/// 최초 게양팀
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 854 ) );
					break;	
				case ePVPDM_ENDTYPE_LASTFLAG:			/// 마지막 깃발 보유팀
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 856 ) );
					break;	
				}
			}
			else
			{
				/// 결투종료 - 패배
				switch( pmsg->mEndType )
				{
				case ePVPDM_ENDTYPE_POINT:				/// 포인트 달성
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 824 ) );
					break;
				case ePVPDM_ENDTYPE_TIME:				/// 시간 초과(포인트량)
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 826 ) );
					break;
				case ePVPDM_ENDTYPE_MEMBER:				/// 인원수
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 828 ) );
					break;
				case ePVPDM_ENDTYPE_LEADERDIE:			/// 수장 사망 횟수
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 830 ) );
					break;
				case ePVPDM_ENDTYPE_FIRSTKILL:			/// 최초 킬러 보유
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 832 ) );
					break;	
				case ePVPDM_ENDTYPE_KILLTOTAL:			/// Kill 총합
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 851 ) );
					break;	
				case ePVPDM_ENDTYPE_RANDOM:			/// random
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 853 ) );
					break;	
				case ePVPDM_ENDTYPE_FIRSTFLAG:			/// 최초 게양팀
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 855 ) );
					break;	
				case ePVPDM_ENDTYPE_LASTFLAG:			/// 마지막 깃발 보유팀
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 857 ) );
					break;	
				}
			}
		}
		break;
	case NM_PVP_DM_END_SYN:
		{
			MSGROOT msg;
			msg.Category = NM_PVP;
			msg.Protocol = NM_PVP_DM_ENDACCEPT_REQ;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			cSmallMiniMapWindow* mini = GAMEUI->GetSmallMinimapWindow();
			cMiniMapWindow* wide = GAMEUI->GetMinimapWindow();
			if( mini && wide )
			{
				mini->ClearFlag();
				mini->ClearFlagPoint();
				mini->ClearStone();

				wide->ClearFlag();
				wide->ClearFlagPoint();
				wide->ClearStone();
			}
		}
		break;
	case NM_PVP_REPORT_RES:
		{
			// 전장 자리비움 신고 결과
			MSG_RES_PVP_REPORT* pmsg = (MSG_RES_PVP_REPORT*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_PVP_REPORT_SUCCESS:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 863 ) );
				break;
			case ERROR_PVP_REPORT_FAIL:
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 864 ) );
				break;
			case ERROR_PVP_REPORT_OTHERTEAM:	// 다른 진영
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 865 ) );
				break;
			case ERROR_PVP_REPORT_EXIST:		// 중복신고
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 866 ) );
				break;
			}

			mRequestLock = false;
		}
		break;
	case NM_PVP_REPORT_SYN:
		{
			// 신고 당함 알림
			MSG_SYN_PVP_REPORT* pmsg = (MSG_SYN_PVP_REPORT*)msg;

			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 867 ), pmsg->mName );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_PVP_REPORT_OUT_SYN:
		{
			// 다른 유저 강제 추방 알림
			MSG_SYN_PVP_REPORT_OUT* pmsg = (MSG_SYN_PVP_REPORT_OUT*)msg;

			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 869 ), pmsg->mName );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	case NM_PVP_FORCEOUT_SYN:
		{
			// 추방 당함
			MSG_SYN_PVP_FORCEOUT* pmsg = (MSG_SYN_PVP_FORCEOUT*)msg;
			pmsg;

			// 추방 처리

			// 메세지 출력
			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 868 ), PVP_REPORT_TOTAL );
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
		}
		break;
	default:
		assert(0);
		break;
	}
}


bool cPVPManager::IsMyTeam( cPlayer* player )
{
	if( mPVPState == ePVPState_None )
		return true;

	if( player == 0 )
	{
		assert(0);
		return false;
	}

	if( HERO->GetPVPTeam() == player->GetPVPTeam() )
		return true;

	return false;
}

bool cPVPManager::IsEnableAttack( cPlayer* player )
{
	if( mPVPState != ePVPState_Fight )
		return false;

	if( player == 0 )
	{
		assert(0);
		return false;
	}

	/// 2009/11/09 GM툴 입장시 공격불가 
	if( HERO->GetPVPTeam() == ePVPDM_TEAMTYPE_MAX )
		return false;
	if(	player->GetPVPTeam() == ePVPDM_TEAMTYPE_MAX )
		return false;

	if( HERO->GetPVPTeam() != player->GetPVPTeam() )
		return true;

	return false;
}

void cPVPManager::LinkPvpForceEffect( cPlayer* player, unsigned char forceType )
{
	if( player == 0 )
		return;

	if( IsEnableBattle() == true )
	{
		switch( forceType )
		{
		case eFORCETYPE_FIRE :	player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_PVP_1.nif", 0, true, true ); break;
		case eFORCETYPE_WATER :	player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_PVP_2.nif", 0, true, true ); break;
		case eFORCETYPE_WIND :	player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_PVP_4.nif", 0, true, true ); break;
		case eFORCETYPE_EARTH :	player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_PVP_3.nif", 0, true, true ); break;
		default:
			break;
		}
	}
}

void cPVPManager::AddTeamPlayerInfo( sMsgMemberInfo* memberInfo )
{
	if( memberInfo == 0 )
	{
		assert(0);
		return;
	}

	sMyTeamInfo* info = new sMyTeamInfo;
	info->mPlayerIdx = memberInfo->mCharacterIdx;
	::Sstrncpy( info->mName, MAX_NAME_BUFFER_SIZE, memberInfo->mName, MAX_NAME_SIZE );
	info->mForceType = memberInfo->mForceType;
	info->mKillCnt = 0;
	info->mDeathCnt = 0;
	info->mScoreResult = 0;

	if( mTeamInfoMap.Insert( memberInfo->mCharacterIdx, info ) == false )
	{
		assert(0);
		delete info;
		return;
	}

	mUpdateRankInfo = true;
}

void cPVPManager::DelTeamPlayerInfo( unsigned long playerIdx )
{
	sMyTeamInfo* info = (sMyTeamInfo*)mTeamInfoMap.GetAt( playerIdx );
	if( info == 0 )
	{
		assert(0);
		return;
	}

	delete info;
	mTeamInfoMap.Erase( playerIdx );

	mUpdateRankInfo = true;
}


void cPVPManager::UpdateTeamPlayerInfo( unsigned long playerIdx, unsigned long value, bool kill )
{
	sMyTeamInfo* info = (sMyTeamInfo*)mTeamInfoMap.GetAt(playerIdx);
	if( info == 0 )
	{
		assert(0);
		return;
	}

	if( kill )
		info->mKillCnt = value;
	else
		info->mDeathCnt = value;

	mUpdateRankInfo = true;
}

void cPVPManager::OpenRank( bool update )
{
	cDMRankingListWindow* win = GAMEUI->GetDMRankingListWindow();
	if( win == 0 )
		return;

	if( update == true )
	{
		sMyTeamInfo teamResult[PVP_DM_TEAM_CNT];
		unsigned int resultCnt = 0;

		cMyTeamInfoMap::cIterator itr = mTeamInfoMap.Begin();
		cMyTeamInfoMap::cIterator itrEnd = mTeamInfoMap.End();
		for( ; itr != itrEnd; ++itr )
		{
			sMyTeamInfo* info = (sMyTeamInfo*)(*itr).mSecond;
			if( info )
			{
				teamResult[resultCnt].mPlayerIdx = info->mPlayerIdx;
				::Sstrncpy( teamResult[resultCnt].mName, MAX_NAME_BUFFER_SIZE, info->mName, MAX_NAME_SIZE );
				teamResult[resultCnt].mForceType = info->mForceType;
				teamResult[resultCnt].mKillCnt = info->mKillCnt;
				teamResult[resultCnt].mDeathCnt = info->mDeathCnt;

				resultCnt++;

				if( resultCnt > PVP_DM_TEAM_CNT )
				{
					assert(0);
					break;
				}
			}
		}

		if( resultCnt > PVP_DM_TEAM_CNT )
		{
			assert(0);
			resultCnt = PVP_DM_TEAM_CNT;
		}

		if( resultCnt > 1 )
			::qsort( (void*)teamResult, resultCnt, sizeof(sMyTeamInfo), PvPRankSort );

		win->Clear();
		for( unsigned long i = 0; i < resultCnt; ++i )
		{
			win->AddPlayerRankInfo( &(teamResult[i]), (teamResult[i].mPlayerIdx == mLeaderIdx) );
		}
	}

	if( win->IsVisible() == false )
		win->Show();
}


void cPVPManager::OpenResult( MSG_SYN_PVP_DM_FIGHTEND* pmsg, bool win )
{
	cDMResultWindow* resultWindow = GAMEUI->GetDMResultWindow();
	if( resultWindow == 0 )
		return;

	if( pmsg->mTeamCnt > PVP_DM_TEAM_CNT )
	{
		assert(0);
		pmsg->mTeamCnt = PVP_DM_TEAM_CNT;
	}

	/// 
	sMyTeamInfo teamResult[PVP_DM_TEAM_CNT];
	unsigned int resultCnt = 0;
	for( unsigned long i=0; i<pmsg->mTeamCnt; ++i )
	{
		sPvPDMTeamScore& data = pmsg->mTeam[i];
		sMyTeamInfo* info = (sMyTeamInfo*)mTeamInfoMap.GetAt( data.mCharacterIdx );
		if( info )
		{
			teamResult[resultCnt].mPlayerIdx = info->mPlayerIdx;
			::Sstrncpy( teamResult[resultCnt].mName, MAX_NAME_BUFFER_SIZE, info->mName, MAX_NAME_SIZE );
			teamResult[resultCnt].mForceType = info->mForceType;
			teamResult[resultCnt].mKillCnt = info->mKillCnt;
			teamResult[resultCnt].mDeathCnt = info->mDeathCnt;
			teamResult[resultCnt].mScoreResult = data.mScore;

			resultCnt++;
		}
	}

	if( resultCnt > PVP_DM_TEAM_CNT )
	{
		assert(0);
		resultCnt = PVP_DM_TEAM_CNT;
	}

	if( resultCnt > 1 )
		::qsort( (void*)teamResult, resultCnt, sizeof(sMyTeamInfo), PvPResultSort );

	/// 1. 순위 출력
	resultWindow->Clear();
	for( unsigned long i = 0; i < resultCnt; ++i )
	{
		resultWindow->InsertRank( &teamResult[i], i );
	}

	/// 2. 결과 출력
	resultWindow->SetResult( win );

	/// 3. 보상 세팅
	resultWindow->SetRewardForce( pmsg->mFireFriendly, pmsg->mFirePoint );
	resultWindow->SetRewardForce( pmsg->mWaterFriendly, pmsg->mWaterPoint );
	resultWindow->SetRewardForce( pmsg->mWindFriendly, pmsg->mWindPoint );
	resultWindow->SetRewardForce( pmsg->mEarthFriendly, pmsg->mEarthPoint );

	resultWindow->SetReward( pmsg->mMoney, pmsg->mExp, pmsg->mSxp );
	resultWindow->Show();

}

// 자리비움 신고
void cPVPManager::RequestReport( cStringT name )
{
	if( mRequestLock == true )
		return;

	if( IsEnableBattle() == false )
		return;

	if( ::_tcscmp( (LPCTSTR)name.Cstr(), HERO->GetName() ) == 0 )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 7000 ) );
		return;
	}

	if( name.IsEmpty() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 871 ) );
		return;
	}

	MSG_REQ_PVP_REPORT msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_REPORT_REQ;
	::Sstrncpy( msg.mName, MAX_NAME_BUFFER_SIZE, (LPCTSTR)name.Cstr(), MAX_NAME_SIZE );
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	mRequestLock = true;
}
