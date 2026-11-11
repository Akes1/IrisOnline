#include "stdafx.h"
#include "gamesrv.h"
#include "BaseDeathMatch.h"
#include "PVPManager.h"
#include "Protocol.h"
#include "LevelScript.h"
#include "PvPScript.h"
#include "Totem.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "StageScript.h"
#include "GridManager.h"
#include "NpcScript.h"
#include "Obstruction_Common.h"
#include "TeleportPortal_Common.h"
#include "AIManager.h"

tPool<cPvPDMMemberInfo> cPvPDMMemberInfo::mMemberInfoPool(2048, 2048);

cPvPDMMemberInfo::cPvPDMMemberInfo()
{
	mPlayerIdx = 0;
	mForceType = 0;

	mLeaderKillCnt = 0;
	mKillCnt = 0;
	mDeathCnt = 0;

	mScore = 0;
}

cPvPDMMemberInfo::~cPvPDMMemberInfo()
{
}


void* cPvPDMMemberInfo::operator new( size_t /*n*/ )
{
	return mMemberInfoPool.Alloc();
}

void cPvPDMMemberInfo::operator delete( void* p )
{
	mMemberInfoPool.Free( (cPvPDMMemberInfo*)p );
}

void cPvPDMMemberInfo::CalcScore()
{
	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo == 0 )
	{
		assert(0);
		return;
	}

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mPlayerIdx );
	if( pPlayer == 0 || pPlayer->GetGameIn() == 0 )
	{
		assert(0);
		return;
	}

	sDMGiveInfo* pGive = pDMInfo->GetGiveInfo( pPlayer->GetLevel() );
	if( pGive == 0 )
	{
		assert(0);
		return;
	}

	mScore = (mLeaderKillCnt*pGive->mLeaderKillPoint) + (mKillCnt*pGive->mKillPoint);
}

//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
cPvPDMTeamInfo::cPvPDMTeamInfo()
{
	mMemberCnt = 0;
	mPoint = 0;

	mTotalKillCnt = 0;

	mPartyUnionIndex = 0;

	/// 세력별 동맹팀
	for( unsigned int i=0;i<eFORCETYPE_MAX;i++ )
		mForce[eFORCETYPE_MAX] = false;
}

cPvPDMTeamInfo::~cPvPDMTeamInfo()
{

}

bool cPvPDMTeamInfo::AddMember( unsigned long idx, unsigned char forceType )
{
	cPvPDMMemberInfo* p = new cPvPDMMemberInfo;
	p->mPlayerIdx = idx;
	p->mForceType = forceType;

	if( mMemberMap.Insert(idx, p) == false )
		return false;

	mMemberCnt++;

	return true;
}

bool cPvPDMTeamInfo::DelMember( unsigned long idx )
{
	cPvPDMMemberInfo* p = (cPvPDMMemberInfo*)mMemberMap.GetAt(idx);
	if( p == 0 )
		return false;

	delete p;
	mMemberMap.Erase( idx );
	mMemberCnt--;

	return true;
}

unsigned long cPvPDMTeamInfo::CalcScore( sPvPDMTeamScore* arr )
{
	unsigned long cnt = 0;

	/// calc character score 
	tPointerHashMap<unsigned long, void*>::cIterator b = mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo )
		{
			pInfo->CalcScore();

			arr[cnt].mCharacterIdx = pInfo->mPlayerIdx;
			arr[cnt].mScore = pInfo->mScore;
			cnt++;
		}
	}

	return cnt;
}

bool cPvPDMTeamInfo::AddReport( unsigned long objectIndex, unsigned long targetIndex )
{
	cPvPDMMemberInfo* targetInfo = (cPvPDMMemberInfo*)mMemberMap.GetAt( targetIndex );
	if( targetInfo == NULL )
		return false;

	return targetInfo->mReportSet.Insert( objectIndex );
}

int cPvPDMTeamInfo::GetReportCount( unsigned long targetIndex )
{
	cPvPDMMemberInfo* targetInfo = (cPvPDMMemberInfo*)mMemberMap.GetAt( targetIndex );
	if( targetInfo )
	{
		return targetInfo->mReportSet.GetSize();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
cBaseDeathMatch::cBaseDeathMatch( ePVPDM_OBJECTTYPE type ) : mObjectType( type )
{
	mpDMBlockInfo = 0;
	mFightStartTime = 0;

	mScoreEnd = false;

	mWaitEndTime = NETWORK2->GetAccumTime() + PVP_DM_READY_TIME - PVP_DM_READY_PRINT1_TIME;
	mFightStartTime = NETWORK2->GetAccumTime() + PVP_DM_READY_TIME;
}

cBaseDeathMatch::~cBaseDeathMatch()
{
	/// 텔레포트 포탈 제거
	mPvPTeleportSet.Clear();

	/// 인던안에 있는 Npc를 모두 제거한다
	if( mNpcAry.GetSize() > 0 )
	{
		for( unsigned long i = 0 ; i < mNpcAry.GetSize() ; ++i )
		{
			unsigned long npcIdx = mNpcAry[i];
			if( npcIdx == 0 )
				continue;

			cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIdx );
			if( pNpc == NULL )
				continue;

			if( GRIDMANAGER->RemoveNpc( pNpc ) == false )
				NETWORK2->PostServerEvent("cBaseDeathMatch::Ready() GRIDMANAGER->RemoveNpc( %d ) == false", npcIdx );

			if( OBJECTMANAGER->RemoveNpc( npcIdx ) == false )
				NETWORK2->PostServerEvent("cBaseDeathMatch::Ready() OBJECTMANAGER->RemoveNpc( %d ) == false", npcIdx );
		}
		mNpcAry.Clear();
	}

	if( mDMTotemArr.GetSize() > 0 )
	{
		for( unsigned int i=0; i< mDMTotemArr.GetSize(); ++i )
		{
			OBJECTMANAGER->InsertDeleteTotem( mDMTotemArr[i] );
		}
		mDMTotemArr.Clear();
	}
}

bool cBaseDeathMatch::Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo )
{
	if( pLevelForceInfo == NULL )
		return false;

	///
	mDMIdx = DMIdx;
	mpDMBlockInfo = pLevelForceInfo->mpBlockInfo;

	mProcessEndTime = 0;
	mProcess = ePVPDM_PROCESS_WAIT;
	mScoreEnd = false;

	mFirstKillTeam = ePVPDM_TEAMTYPE_MAX;
	memset( mTeamInfo, 0 , sizeof( mTeamInfo ) );

	mNpcAry.Clear();

	mWaitEndTime = NETWORK2->GetAccumTime() + PVP_DM_READY_TIME - PVP_DM_READY_PRINT1_TIME;
	mFightStartTime = NETWORK2->GetAccumTime() + PVP_DM_READY_TIME;

	/// 팀별 세력정보
	for( int i = eFORCETYPE_FIRE; i<eFORCETYPE_MAX; ++i )
	{
		if( pLevelForceInfo->mATeam[i] == true )
		{
			mTeamInfo[ePVPDM_TEAMTYPE_A].mForce[i] = true;
			mTeamInfo[ePVPDM_TEAMTYPE_B].mForce[i] = false;
		}
		else
		{
			mTeamInfo[ePVPDM_TEAMTYPE_A].mForce[i] = false;
			mTeamInfo[ePVPDM_TEAMTYPE_B].mForce[i] = true;
		}
	}

	PVPOBJECTPOOL->PvPAttachPool( PVPMANAGER->GetPvPObjectRoot(), this );

	/// npc 관리
	typedef const tPointerHashMap<unsigned long, sNPCRegen*> cNpcRegenMap;
	cNpcRegenMap* pNpcRegenMap = NPCSCRIPT->GetNPCRegenMap( 0 );
	
	if( pNpcRegenMap != NULL )
	{
		cNpcRegenMap::cConstIterator begin = pNpcRegenMap->Begin( );
		cNpcRegenMap::cConstIterator end   = pNpcRegenMap->End( );

		cNpc* pNpc = NULL;
		sNPCRegen* pNpcRegen = NULL;
		while( begin != end )
		{
			pNpcRegen = (*begin).mSecond;
			if( pNpcRegen == NULL )
			{
				NETWORK2->PostServerEvent("cObjectManager::LoadNpcInfo pNpcRegenScript[%d] == NULL", (*begin).mFirst );
				continue;
			}

			unsigned long npcIdx = PVPMANAGER->GenNpcIndex();
			pNpc = OBJECTMANAGER->AddNpc( npcIdx, pNpcRegen->mNpcClassIdx, DMIdx, 
				pNpcRegen->mPosX, pNpcRegen->mPosY, pNpcRegen->mDirection );	

			if( pNpc )
			{
				mNpcAry.PushBack( npcIdx );
				GRIDMANAGER->AddNpc( pNpc );
			}
			else
			{
				assert(0);
			}

			++begin;
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cBaseDeathMatch::Init pNpcRegenMap != NULL");
	}

	unsigned short mapNum = NETWORK2->GetInDunMapNum();

	/// 장애물(문) 스크립트 데이터 로드
	cPArray* pObstruction = STAGESCRIPT->GetManagedDoorArr( mapNum );
	if( pObstruction != NULL )
	{
		for( unsigned long i = 0 ; i < pObstruction->GetSize() ; ++i )
		{
			sDoorInfo* pDoor = (sDoorInfo*)(*pObstruction)[i];
			if( pDoor == NULL )
				continue;

			mObstructionMap.Insert( pDoor->mIndex, pDoor );
		}
	}

	return true;
}

void cBaseDeathMatch::SendReadyLeftTime( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	/// 입장후 전투 시작까지 남은시간 발송
	if( mProcess <= ePVPDM_PROCESS_READY )
	{		
		/// 남은시간 계산
		unsigned long leftTime = 0;
		if( mFightStartTime > NETWORK2->GetAccumTime() )
			leftTime = mFightStartTime - NETWORK2->GetAccumTime();

		/// 전투시작까지 남은시간 발송
		HANDLE handle  = NULL;
		MSG_SYN_PVP_JOINTIME* sendMsg = (MSG_SYN_PVP_JOINTIME*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), 
			NM_PVP, NM_PVP_DM_JOINTIME_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mFightLeftTime = leftTime;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_JOINTIME) );
		}		

		return;
	}
	/// 입장후 이미 전투가 시작했으면 시작을 알림
	else if( IsFight() == true )
	{
		SendFight( pPlayer );
		return;
	}

	/// 전투진행 불가를 알림
	NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_DM_NOTREADY_SYN );
}

bool cBaseDeathMatch::IsFight()
{
	if( mProcess == ePVPDM_PROCESS_FIGHT_MINIMUM )
		return true;

	if( mProcess == ePVPDM_PROCESS_FIGHT )
		return true;

	return false;
}

int cBaseDeathMatch::JoinTeam( cPlayer* player )
{
	if( player == 0 )
		return 1;

	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo == 0 )
		return 2;

	/// 프로세스 상태 확인
	if( mProcess == ePVPDM_PROCESS_READY_OUT )
		return 3;

	if( mProcess >= ePVPDM_PROCESS_FIGHTEND )
		return 35;

	/// level check
	unsigned short level = player->GetLevel();
	if( level < mpDMBlockInfo->mLevelMin || level > mpDMBlockInfo->mLevelMax )
		return 4;

	/// force check
	unsigned char forceType = player->GetForceType();
	if( forceType == eFORCETYPE_NONE || forceType >= eFORCETYPE_MAX )
		return 5;

	/// max team count check
	unsigned char teamType = ePVPDM_TEAMTYPE_MAX;
	if( mTeamInfo[ePVPDM_TEAMTYPE_A].mForce[forceType] == true )
	{
		teamType = ePVPDM_TEAMTYPE_A;
	}
	else if( mTeamInfo[ePVPDM_TEAMTYPE_B].mForce[forceType] == true )
	{
		teamType = ePVPDM_TEAMTYPE_B;
	}

	if( teamType == ePVPDM_TEAMTYPE_MAX )
		return 6;

	/// 최대 인원 체크
	if( mTeamInfo[teamType].mMemberCnt >= pDMInfo->mTeamPlayerMax )
		return 7;


	/// add player
	unsigned long idx = player->GetObjectID();
	if( mTeamInfo[teamType].AddMember( idx, forceType ) == false )
		return 8;

	player->SetPvPDMTeam( teamType );

	/// msg
	MSG_SYN_PVP_DM_MEMBERIN syncMsg;
	syncMsg.Category = NM_PVP;
	syncMsg.Protocol = NM_PVP_DM_MEMBERIN_SYN;
	syncMsg.mMemberInfo.mCharacterIdx = idx;
	syncMsg.mMemberInfo.mForceType = forceType;
	::wcsncpy( syncMsg.mMemberInfo.mName, player->GetName(), MAX_NAME_SIZE ); 
	TeamQuickSendExcept( (ePVPDM_TEAM_TYPE)teamType, idx, (char*)&syncMsg, sizeof(syncMsg) );

	return 0;
}

void cBaseDeathMatch::SendTeamList( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	HANDLE handle  = NULL;
	MSG_RES_PVP_DM_MEMBERIN* sendMsg = (MSG_RES_PVP_DM_MEMBERIN*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), NM_PVP, NM_PVP_DM_MEMBERIN_RES );
	if( sendMsg == 0 )
	{
		assert(0);
		NETWORK2->PostServerEvent(" cBaseDeathMatch::SendTeamList sendMsg == 0", pPlayer->GetObjectID() );
		return;
	}

	ePVPDM_TEAM_TYPE teamType = (ePVPDM_TEAM_TYPE)pPlayer->GetPvPDMTeam();
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		NETWORK2->PostServerEvent(" cBaseDeathMatch::SendTeamList teamType >= ePVPDM_TEAMTYPE_MAX ", pPlayer->GetObjectID() );
		return;
	}

	/// 팀별 정보 설정
	sendMsg->mFireForce = mTeamInfo[teamType].mForce[eFORCETYPE_FIRE];
	sendMsg->mWaterForce = mTeamInfo[teamType].mForce[eFORCETYPE_WATER];
	sendMsg->mWindForce = mTeamInfo[teamType].mForce[eFORCETYPE_WIND];
	sendMsg->mEarthForce = mTeamInfo[teamType].mForce[eFORCETYPE_EARTH];

	sendMsg->mMemberCnt = 0;
	tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[teamType].mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[teamType].mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo )
		{
			sendMsg->mMemberInfo[sendMsg->mMemberCnt].mCharacterIdx = pInfo->mPlayerIdx;
			sendMsg->mMemberInfo[sendMsg->mMemberCnt].mForceType = pInfo->mForceType;

			cPlayer* p = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
			if( p )
				::wcsncpy( sendMsg->mMemberInfo[sendMsg->mMemberCnt].mName, p->GetName(), MAX_NAME_SIZE ); 
			else
				::wcsncpy( sendMsg->mMemberInfo[sendMsg->mMemberCnt].mName, L"Error!", MAX_NAME_SIZE ); 

			sendMsg->mMemberCnt++;
		}
	}
	NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
}

void cBaseDeathMatch::OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		NETWORK2->PostServerEvent(" cBaseDeathMatch::OutMember teamType >= ePVPDM_TEAMTYPE_MAX ", playerIdx );
		return;
	}

	if( mTeamInfo[teamType].DelMember( playerIdx ) == false )
	{
		assert(0);
		NETWORK2->PostServerEvent(" cBaseDeathMatch::OutMember delete fail [%d] ", playerIdx );
		return;
	}

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer )
	{
		pPlayer->PvPResurrection( false );	/// 죽어 있으면 부활
		pPlayer->PvPEndDebuffClear();

		/// pvp 나감 처리
		pPlayer->PvPOut();

//		/// pvp 다시 진입 불가 효과 생성
//		sObject object = { pPlayer->GetObjectType(), pPlayer->GetObjectID() };
//		SKILLMANAGER->AddInfluence( object, object, PVP_DM_CANTPVP_INFLUENCE, 0, true );
	}

	/// 팀원들에게 나감 알림
	MSG_SYN_PVP_DM_MEMBEROUT synMsg;
	synMsg.Category = NM_PVP;
	synMsg.Protocol = NM_PVP_DM_MEMBEROUT_SYN;
	synMsg.mCharacterIdx = playerIdx;
	TeamQuickSend( teamType, (char*)&synMsg, sizeof( synMsg ) );
}


void cBaseDeathMatch::Ready()
{
	bool isReady = true;
	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo == 0 )
	{
		assert(0);
		NETWORK2->PostServerEvent( "cBaseDeathMatch::Ready() pDMInfo == 0" );

		isReady = false;
	}
	else
	{
		for( int i = 0 ; i < ePVPDM_TEAMTYPE_MAX ; ++i )
		{
			/// 최소 인원 체크
			if( mTeamInfo[i].mMemberCnt < pDMInfo->mTeamPlayerMin )
			{
				isReady = false;
				break;
			}
		}
	}

	unsigned long accumTime = NETWORK2->GetAccumTime();
	if( isReady == true )
	{
		MSGROOT msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM_READY_SYN;
		DMQuickSend( (char*)&msg, sizeof( msg ) );

		mProcess = ePVPDM_PROCESS_READY;
		mProcessEndTime = accumTime + PVP_DM_READY_PRINT1_TIME;
		return;
	}
	else
	{
		EndNpcTalk();

		MSGROOT msg;
		msg.Category = NM_PVP;
		msg.Protocol = NM_PVP_DM_NOTREADY_SYN;
		DMQuickSend( (char*)&msg, sizeof( msg ) );

		mProcess = ePVPDM_PROCESS_READY_OUT;
		mProcessEndTime = accumTime + PVP_DM_READY_OUT_TIME;
		return;
	}
}


void cBaseDeathMatch::SendFight( unsigned long fightTime )
{
	MSG_SYN_PVP_FIGHT msg;
	msg.Category = NM_PVP;
	msg.Protocol = NM_PVP_DM_FIGHT_SYN;
	msg.mFightTime = fightTime;
	DMQuickSend( (char*)&msg, sizeof( msg ) );

	mFightStartTime = NETWORK2->GetAccumTime();

	mObstructionMap.Clear();
}


bool cBaseDeathMatch::SendFight( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return false;

	if( mProcess < ePVPDM_PROCESS_FIGHT_MINIMUM )
		return false;

	sDMInfo* pInfo = PVPMANAGER->GetDMInfo();
	if( pInfo == NULL )
		return false;

	/// 전투시작 알림
	HANDLE handle  = NULL;
	MSG_SYN_PVP_FIGHT* sendMsg = (MSG_SYN_PVP_FIGHT*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), 
		NM_PVP, NM_PVP_DM_FIGHT_SYN );
	if ( sendMsg != NULL )
	{
		if( mFightStartTime != 0 && NETWORK2->GetAccumTime() >= mFightStartTime )
		{
			if( pInfo->mTotalDMTime * MINUTE  > NETWORK2->GetAccumTime() - mFightStartTime )
				sendMsg->mFightTime = pInfo->mTotalDMTime * MINUTE  - ( NETWORK2->GetAccumTime() - mFightStartTime );
			else
				sendMsg->mFightTime = 0;
		}
		else
			sendMsg->mFightTime = pInfo->mTotalDMTime * MINUTE;
		
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_FIGHT) );
	}

	return true;
}


void cBaseDeathMatch::EndNpcTalk()
{
	/// npc랑 대화중이던 플레이어 상태를 풀어준다.
	for( int i=0;i<ePVPDM_TEAMTYPE_MAX;i++ )
	{
		tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[i].mMemberMap.Begin();
		tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[i].mMemberMap.End();
		for( ; b!=end; ++b )
		{
			cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
			if( pInfo )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
				if( pPlayer && pPlayer->GetState() == eOBJECT_STATE_STOP && pPlayer->GetStateStop() == eSTOP_NPCSPEECH )
				{
					pPlayer->SetStateStop( eSTOP_NONE );
					pPlayer->ChangeState( eOBJECT_STATE_IDLE );
				}
			}
		}
	}

	/// 인던안에 있는 Npc를 모두 제거한다
	if( mNpcAry.GetSize() > 0 )
	{
		for( unsigned long i = 0 ; i < mNpcAry.GetSize() ; ++i )
		{
			unsigned long npcIdx = mNpcAry[i];
			if( npcIdx == 0 )
				continue;

			cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIdx );
			if( pNpc == NULL )
				continue;

			if( GRIDMANAGER->RemoveNpc( pNpc ) == false )
				NETWORK2->PostServerEvent("cBaseDeathMatch::Ready() GRIDMANAGER->RemoveNpc( %d ) == false", npcIdx );

			if( OBJECTMANAGER->RemoveNpc( npcIdx ) == false )
				NETWORK2->PostServerEvent("cBaseDeathMatch::Ready() OBJECTMANAGER->RemoveNpc( %d ) == false", npcIdx );
		}
	}
}

void cBaseDeathMatch::AddTeleportPortal()
{ 
	cPArray* pAry = STAGESCRIPT->GetMapChangeStarterArr( NETWORK2->GetInDunMapNum() );
	if( pAry == NULL )
		return;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		sStageChangePos* pChangePos = (sStageChangePos*)(*pAry)[i];
		if( pChangePos == NULL )
			continue;

		if( mPvPTeleportSet.Insert( pChangePos->mPosIdx ) == false )
		{
			NETWORK2->PostServerEvent("cThemeObject::AddTeleportPortal duplicate %d", pChangePos->mPosIdx );
			continue;
		}

		MSG_SYN_TELEPORTPORTAL_CREATE synMsg;
		synMsg.Category = NM_TELEPORTPORTAL;
		synMsg.Protocol = NM_TELEPORTPORTAL_CREATE_SYN;
		synMsg.mMapChangePosIdx = pChangePos->mPosIdx;
		DMQuickSend( (char*)&synMsg, sizeof( MSG_SYN_TELEPORTPORTAL_CREATE ) );
	}
}

void cBaseDeathMatch::SendResult( ePVPDM_TEAM_TYPE winTeam, ePVPDM_END_TYPE endType )
{
	if( winTeam >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		winTeam = (ePVPDM_TEAM_TYPE)(::rand()%((int)ePVPDM_TEAMTYPE_MAX));
	}

	cPvPDMTeamInfo* pWinTeam = 0;
	cPvPDMTeamInfo* pLoseTeam = 0;
	if( winTeam == ePVPDM_TEAMTYPE_A )
	{
		pWinTeam = &mTeamInfo[ePVPDM_TEAMTYPE_A];
		pLoseTeam = &mTeamInfo[ePVPDM_TEAMTYPE_B];
	}
	else
	{
		pWinTeam = &mTeamInfo[ePVPDM_TEAMTYPE_B];
		pLoseTeam = &mTeamInfo[ePVPDM_TEAMTYPE_A];
	}

	SendTeamResult( pWinTeam, endType, NM_PVP_DM_WIN_SYN );
	SendTeamResult( pLoseTeam, endType, NM_PVP_DM_LOSE_SYN );
}

void cBaseDeathMatch::SendTeamResult( cPvPDMTeamInfo* pTeam, ePVPDM_END_TYPE endType, char protocol )
{
	if( pTeam == 0 )
	{
		assert(0);
		NETWORK2->PostServerEvent( "cBaseDeathMatch::SendTeamResult() pTeam == 0" );
		return;
	}
	sDMInfo* pDMInfo = PVPMANAGER->GetDMInfo();
	if( pDMInfo == 0 )
	{
		assert(0);
		NETWORK2->PostServerEvent( "cBaseDeathMatch::SendTeamResult() pDMInfo == 0" );
		return;
	}

	/// 승/패 메세지 발송
	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	MSG_SYN_PVP_DM_FIGHTEND* pSyn = (MSG_SYN_PVP_DM_FIGHTEND*)perIoContext->buffer;
	pSyn->Category = NM_PVP;
	pSyn->Protocol = protocol;
	pSyn->mEndType = (unsigned char)endType;

	/// 개인 점수 계산
	sPvPDMTeamScore member[PVP_DM_TEAM_CNT] = {0,};
	unsigned long count = pTeam->CalcScore( member );

	pSyn->mTeamCnt = count;
	memcpy( pSyn->mTeam, member, sizeof(sPvPDMTeamScore) * count );

	tPointerHashMap<unsigned long, void*>::cIterator b = pTeam->mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = pTeam->mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo == 0 )
			continue;
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
		if( pPlayer == 0 )
			continue;

		pPlayer->PvPResurrection( false );

		sDMGiveInfo* pGive = pDMInfo->GetGiveInfo( pPlayer->GetLevel() );
		if( pGive == 0 )
		{
			assert(0);
			continue;
		}

		/// 승패에 따른 계산 점수 취합
		unsigned long teamFriendly = 0;
		unsigned long teamPoint = 0;
		unsigned long myFriendly = 0;
		unsigned long myPoint = 0;
		unsigned long money = 0;
		unsigned long addExp = 0;
		unsigned long addSxp = 0;
		unsigned long infIdx = 0;
		sExpTable* expTable = LEVELSCRIPT->GetExpTable( pPlayer->GetLevel() );
		sSxpTable* sxpTable = LEVELSCRIPT->GetSxpTable( pPlayer->GetSkillLevel() );;
		if( protocol == NM_PVP_DM_WIN_SYN )
		{
			infIdx = PVP_DM_WIN_INFLUENCE;

			money = pGive->mWinMoney;
			if( expTable )
				addExp = (unsigned long)(expTable->mExp * pGive->mWinExpPer * 0.01f);
			if( sxpTable )
				addSxp = (unsigned long)(sxpTable->mSxp * pGive->mWinSxpPer * 0.01f); 

			myFriendly = pGive->mWinMyForceFriendly;
			myPoint =  pGive->mWinMyForcePoint;

			teamFriendly = pGive->mWinFamilyForceFriendly;
			teamPoint = pGive->mWinFamilyForcePoint;
		}
		else
		{
			infIdx = PVP_DM_LOSE_INFLUENCE;

			money = pGive->mWinMoney;
			if( expTable )
				addExp = (unsigned long)(expTable->mExp * pGive->mLoseExpPer * 0.01f);
			if( sxpTable )
				addSxp = (unsigned long)(sxpTable->mSxp * pGive->mLoseSxpPer * 0.01f); 

			if( mpDMBlockInfo->mLoseFriendlyBasePoint < pTeam->mPoint )
				myFriendly = pGive->mLoseMyForceFriendly;
			if( mpDMBlockInfo->mLoseDMBasePoint < pTeam->mPoint )
				myPoint = pGive->mLoseMyForcePoint;

			teamFriendly = 0;
			teamPoint = 0;
		}

		/// 경험치 / 돈 / 효과
		pSyn->mExp = addExp;
		pSyn->mSxp = addSxp;
		pSyn->mMoney = money;
		if( addExp != 0 )
			pPlayer->AddExp( addExp, false, false );
		if( addSxp != 0 )
			pPlayer->AddSxp( addSxp, false );
		if( money != 0 )
			pPlayer->AddMoney( pPlayer->GetObject(), money );

		SKILLMANAGER->AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), infIdx, 0, true );

		/// 불세력 점수 취합
		unsigned long addPoint = 0;
		unsigned long addFriendly = 0;

		unsigned long memberPoint = pTeam->mPoint;
		//unsigned long memberPoint = pTeam->mSumTeamPoint / pTeam->mMemberCnt;
		//if( pTeam->mSumTeamPoint != memberPoint * pTeam->mMemberCnt )
		//	memberPoint = memberPoint + 1;

		for( unsigned int i=eFORCETYPE_FIRE; i<eFORCETYPE_MAX; ++i )
		{
			if( pTeam->mForce[i] == true )
			{
				float PvPAddPer = pPlayer->GetStatusEtc()->mPvPPointAddPer;

				if( pPlayer->GetForceType() == i )
				{
					if( PvPAddPer < 0.0f )
					{
						NETWORK2->PostServerEvent( "cBaseDeathMatch::SendTeamResult PvPAddPer < 0.0f[%f]", PvPAddPer );
						addPoint = myPoint + memberPoint;
						addFriendly = myFriendly;
					}
					else
					{
						addPoint = FloatToInt( myPoint + ((float)(myPoint) * (PvPAddPer/100.0f)) + memberPoint);
						addFriendly = myFriendly;
					}
				}
				else
				{
					addPoint = FloatToInt( teamPoint + ((float)(teamPoint) * (PvPAddPer/100.0f)) );
					addFriendly = teamFriendly;
				}
			}
			else
			{
				addPoint = 0;
				addFriendly = 0;
			}

			switch( i )
			{
			case eFORCETYPE_FIRE:
				{
					pSyn->mFireFriendly = addFriendly;
					pSyn->mFirePoint = addPoint;
					pSyn->mFireFriendlyTotal = pPlayer->FireFriendlyPlus( addFriendly, false );
					pSyn->mFirePointTotal = pPlayer->FirePointPlus( addPoint, false );
				}
				break;
			case eFORCETYPE_WATER:
				{
					pSyn->mWaterFriendly = addFriendly;
					pSyn->mWaterPoint = addPoint;
					pSyn->mWaterFriendlyTotal = pPlayer->WaterFriendlyPlus( addFriendly, false );
					pSyn->mWaterPointTotal = pPlayer->WaterPointPlus( addPoint, false );

				}
				break;
			case eFORCETYPE_WIND:
				{
					pSyn->mWindFriendly = addFriendly;
					pSyn->mWindPoint = addPoint;
					pSyn->mWindFriendlyTotal = pPlayer->WindFriendlyPlus( addFriendly, false );
					pSyn->mWindPointTotal = pPlayer->WindPointPlus( addPoint, false );

				}
				break;
			case eFORCETYPE_EARTH:
				{
					pSyn->mEarthFriendly = addFriendly;
					pSyn->mEarthPoint = addPoint;
					pSyn->mEarthFriendlyTotal = pPlayer->EarthFriendlyPlus( addFriendly, false );
					pSyn->mEarthPointTotal = pPlayer->EarthPointPlus( addPoint, false );
				}
				break;
			}
		}

		/// db updata
		pPlayer->ForcePointDBUpdate();

		/// msg send
		NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), (char*)pSyn, pSyn->GetMsgLength() );

		// 퀘스트 체크
		pPlayer->UpdateDutyPVP( (unsigned char)mObjectType, (protocol == NM_PVP_DM_WIN_SYN) );
	}

	perIoContext->offset = pSyn->GetMsgLength();
	NETWORK2->ReleaseIoContext( perIoContext, "cDeathMatch2Object::SendTeamResult:MSG_SYN_PVP_DM_FIGHTEND" );	
}


void cBaseDeathMatch::EndDM()
{
	/// 남아있는 인원 전체 추방
	for( unsigned int i=0; i<ePVPDM_TEAMTYPE_MAX; ++i )
	{
		tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[i].mMemberMap.Begin();
		tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[i].mMemberMap.End();
		for( ; b!=end; ++b )
		{
			cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
			if( pInfo == 0 )
				continue;
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
			if( pPlayer )
			{
				pPlayer->PvPEndDebuffClear();
				pPlayer->AutoPvPOut();
			}
			delete pInfo;
		}
		mTeamInfo[i].mMemberMap.Clear();
	}

	/// 인던 생성시 만들어졌던 토템제거
	for( unsigned int i=0; i< mDMTotemArr.GetSize(); ++i )
	{
		OBJECTMANAGER->InsertDeleteTotem( mDMTotemArr[i] );
	}
	mDMTotemArr.Clear();
}

bool cBaseDeathMatch::OrderEndDM()
{
	/// 남아있는 인원 전체 추방
	for( unsigned int i=0; i<ePVPDM_TEAMTYPE_MAX; ++i )
	{
		tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[i].mMemberMap.Begin();
		tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[i].mMemberMap.End();
		for( ; b!=end; ++b )
		{
			cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
			if( pInfo == 0 )
				continue;
			unsigned long playerIdx = (*b).mFirst;

			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
			if( pPlayer )
			{
				pPlayer->PvPEndDebuffClear();
				pPlayer->AutoPvPOut();
			}
			delete pInfo;

			mTeamInfo[i].mMemberMap.Erase( playerIdx );

			mAutoOutCheckTime = NETWORK2->GetAccumTime() + 1000;
			return false;
		}
	}

	return true;
}

void cBaseDeathMatch::EndDMReq( unsigned long playerIdx, unsigned long teamType )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		NETWORK2->PostServerEvent( "cBaseDeathMatch::EndDMReq teamType >= ePVPDM_TEAMTYPE_MAX" );
		return;
	}

	/// 남아있는 인원 전체 추방
	cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)mTeamInfo[teamType].mMemberMap.GetAt( playerIdx );
	if( pInfo == 0 )
		return;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
	if( pPlayer )
		pPlayer->AutoPvPOut();

	mTeamInfo[teamType].DelMember( playerIdx );
}

void cBaseDeathMatch::DMQuickSend( char* msg, unsigned long length )
{
	TeamQuickSend( ePVPDM_TEAMTYPE_A, msg, length );
	TeamQuickSend( ePVPDM_TEAMTYPE_B, msg, length );
}


void cBaseDeathMatch::TeamQuickSend( ePVPDM_TEAM_TYPE teamType, char* msg, unsigned long length )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		NETWORK2->PostServerEvent(" cBaseDeathMatch::TeamQuickSend teamType >= ePVPDM_TEAMTYPE_MAX" );
		return;
	}

	tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[teamType].mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[teamType].mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
			if( player == 0 || player->GetGameIn() == false )
				continue;

			NETWORK2->SendToUser( player->GetConnectionIdx(), msg, length );
		}
	}
}


void cBaseDeathMatch::TeamQuickSendExcept( ePVPDM_TEAM_TYPE teamType, unsigned long exceptIdx, char* msg, unsigned long length )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		NETWORK2->PostServerEvent(" cBaseDeathMatch::TeamQuickSend teamType >= ePVPDM_TEAMTYPE_MAX" );
		return;
	}

	tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[teamType].mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[teamType].mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo && pInfo->mPlayerIdx != exceptIdx )
		{
			cPlayer* player = OBJECTMANAGER->GetPlayer( pInfo->mPlayerIdx );
			if( player == 0 || player->GetGameIn() == false )
				continue;

			NETWORK2->SendToUser( player->GetConnectionIdx(), msg, length );
		}
	}
}


void cBaseDeathMatch::TeamChatQuickSend( cPlayer* pPlayer, ePVPDM_TEAM_TYPE teamType, char* msg, unsigned long length )
{
	if( pPlayer == NULL )
		return;

	mTempChatAry.Clear();
	GRIDMANAGER->FindPlayers( pPlayer, &mTempChatAry );
	for( unsigned long i = 0 ; i < mTempChatAry.GetSize() ; ++i )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mTempChatAry[i] );
		if( pPlayer == NULL )
			continue;

		if( pPlayer->GetPvPDMTeam() != ePVPDM_TEAMTYPE_MAX && teamType != ePVPDM_TEAMTYPE_MAX )
		{
			if( pPlayer->GetStateDie() == true || pPlayer->GetPvPDMTeam() != (unsigned long)teamType )
				continue;
		}

		NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), msg, length );
	}
}


// 파티연합 정보 세팅
void cBaseDeathMatch::SetPartyUnionIndex( ePVPDM_TEAM_TYPE teamType, unsigned long partyUnionIndex )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return;
	}
	mTeamInfo[teamType].mPartyUnionIndex = partyUnionIndex;
}

unsigned long cBaseDeathMatch::GetPartyUnionIndex( ePVPDM_TEAM_TYPE teamType )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		assert(0);
		return 0;
	}
	return mTeamInfo[teamType].mPartyUnionIndex;
}

unsigned long cBaseDeathMatch::TeamKillTotal( ePVPDM_TEAM_TYPE teamType )
{
	if( teamType >= ePVPDM_TEAMTYPE_MAX )
	{
		NETWORK2->PostServerEvent(" cBaseDeathMatch::TeamKillTotal teamType >= ePVPDM_TEAMTYPE_MAX" );
		return 0;
	}

	unsigned long killTotal = 0;
	tPointerHashMap<unsigned long, void*>::cIterator b = mTeamInfo[teamType].mMemberMap.Begin();
	tPointerHashMap<unsigned long, void*>::cIterator end = mTeamInfo[teamType].mMemberMap.End();
	for( ; b!=end; ++b )
	{
		cPvPDMMemberInfo* pInfo = (cPvPDMMemberInfo*)(*b).mSecond;
		if( pInfo )
		{
			killTotal += pInfo->mKillCnt;
			killTotal += pInfo->mLeaderKillCnt;
		}
	}
	return killTotal;
}

void cBaseDeathMatch::Result()
{
	/// Kill 총합 비교
	unsigned long killTotalA = TeamKillTotal( ePVPDM_TEAMTYPE_A);
	unsigned long killTotalB = TeamKillTotal( ePVPDM_TEAMTYPE_B);
	if( killTotalA > killTotalB )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_KILLTOTAL );
		return;
	}
	if( killTotalA < killTotalB )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_KILLTOTAL );
		return;
	}

	/// 최초 킬러 보유
	if( mFirstKillTeam == ePVPDM_TEAMTYPE_A )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_FIRSTKILL );
		return;
	}
	if( mFirstKillTeam == ePVPDM_TEAMTYPE_B )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_FIRSTKILL );
		return;
	}

	/// 랜덤 선택
	unsigned int r = ::rand()%((unsigned int)ePVPDM_TEAMTYPE_MAX);
	if( r == ePVPDM_TEAMTYPE_A )
	{
		SendResult( ePVPDM_TEAMTYPE_A, ePVPDM_ENDTYPE_RANDOM );
		return;
	}
	else if( r == ePVPDM_TEAMTYPE_B )
	{
		SendResult( ePVPDM_TEAMTYPE_B, ePVPDM_ENDTYPE_RANDOM );
		return;
	}

}


bool cBaseDeathMatch::IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos )
{
	cPHashMap::cIterator b = mObstructionMap.Begin();
	cPHashMap::cIterator e = mObstructionMap.End();
	for( ; b != e ; ++b )
	{
		sDoorInfo* pObstruction = (sDoorInfo*)(*b).mSecond;
		if( pObstruction == NULL )
			continue;

		/// 
		if( STAGESCRIPT->LineIntersectRect( pObstruction->mRectPos, startPos, goalPos ) == true )
			return false;
	}
	return true;
}


void cBaseDeathMatch::SendObstruction( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	cPHashMap::cIterator b = mObstructionMap.Begin();
	cPHashMap::cIterator e = mObstructionMap.End();

	HANDLE handle = NULL;
	MSG_SYN_OBSTRUCTION_LIST* pMsg = (MSG_SYN_OBSTRUCTION_LIST*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
		NM_OBSTRUCTION, NM_OBSTRUCTION_LIST_SYN );
	if ( pMsg != NULL )
	{
		unsigned char i = 0;
		for( ; b != e ; ++b )
		{
			unsigned long idx = (*b).mFirst;
			if( idx == 0 )
				continue;

			pMsg->mObstructionIdx[i] = idx;
			++i;
		}

		pMsg->mCount = i;

		NETWORK2->SendMsgRoot( handle, pMsg->GetMsgLength() );
	}
}


int cBaseDeathMatch::UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx )
{
	if( pPlayer == NULL )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	/// 2차상태 체크
	if( pPlayer->IsRequestRejection() == true )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	/// 맵이동 가능인지 현재 상태 체크
	if( pPlayer->IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	cHashSet::cIterator b = mPvPTeleportSet.Begin();
	cHashSet::cIterator e = mPvPTeleportSet.End();
	if( b == e )
	{
		NETWORK2->PostServerEvent("cBaseDeathMatch::UseTeleportPortal b == e %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_PORTAL;
	}

	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( mapChangeIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cBaseDeathMatch::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cBaseDeathMatch::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
	}

	if( scriptPos->mMapNumber != NETWORK2->GetInDunMapNum() )
		return ERROR_TELEPORTPORTAL_USE_MAP;

	NiPoint2 destPos = pPlayer->GetPos();
	float objectSize = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, 0 );
	NiPoint2 portalPos;
	portalPos.x = scriptPos->mPosX;
	portalPos.y = scriptPos->mPosY;

	mRangeCheck.SetRadius( scriptPos->mRange + objectSize + SYNC_MOVE_RANGE );
	if( mRangeCheck.IsNotRange( portalPos, pPlayer->GetPos() ) == true )
		return ERROR_TELEPORTPORTAL_USE_RANGE;

	sStageChangePos* pTargetInfo = scriptPos->mTargetInfo;
	if( pTargetInfo == NULL )
	{
		NETWORK2->PostServerEvent("cBaseDeathMatch::UseTeleportPortal pTargetInfo == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
	}

	bool isPos = false;
	/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
	if( pTargetInfo->mRange > MAP_RANGE_CHECK_RING )
	{
		for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
		{
			/// 최대 범위에서 20%씩 줄여가면서 체크
			int range = (int)(pTargetInfo->mRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
			if( range <= 0 ) 
				continue;

			destPos.x = pTargetInfo->mPosX + ( rand() % range ) - ( rand() % range );
			destPos.y = pTargetInfo->mPosY + ( rand() % range ) - ( rand() % range );

			if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), destPos.x, destPos.y, pPlayer->GetObject() ) == false )
				continue;

			isPos = true;
			break;
		}
	}

	if( isPos == false )
	{
		/// 위에 랜덤 범위 좌표중 모두다 못가는 경우 스크립트상 정해진 고정위치로 이동시킨다.
		destPos.x = pTargetInfo->mPosX;
		destPos.y = pTargetInfo->mPosY;
	}

	if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), destPos.x, destPos.y, pPlayer->GetObject() ) == false )
		return ERROR_TELEPORTPORTAL_USE_SCRIPT;

	pPlayer->SetPos( destPos.x, destPos.y );
	pPlayer->MoveStop();

	return ERROR_RES_MAPCHANGE_SUCCESS;
}

int cBaseDeathMatch::AddReport( cPlayer* hero, unsigned long targetIndex )
{
	cPlayer* target = OBJECTMANAGER->GetPlayer( targetIndex );
	if( hero == NULL || target == NULL )
		return ERROR_PVP_REPORT_FAIL;

	// 다른 진영
	if( hero->GetPvPDMTeam() != target->GetPvPDMTeam() )
		return ERROR_PVP_REPORT_OTHERTEAM;

	unsigned char team = (unsigned char)target->GetPvPDMTeam();
	if( team >= ePVPDM_TEAMTYPE_MAX )
		return ERROR_PVP_REPORT_FAIL;

	// 전장 상태 체크
	if( mProcess >= ePVPDM_PROCESS_FIGHTEND )
		return ERROR_PVP_REPORT_FAIL;

	// 삽입 : 중복 신고
	if( mTeamInfo[team].AddReport( hero->GetObjectID(), targetIndex ) == false )
		return ERROR_PVP_REPORT_EXIST;

	//
	NETWORK2->SendMsgError( hero->GetConnectionIdx(), NM_PVP, NM_PVP_REPORT_RES, ERROR_PVP_REPORT_SUCCESS );

	// 대상자에게 신고 알림
	HANDLE				handle  = NULL;
	MSG_SYN_PVP_REPORT* sendMsg = (MSG_SYN_PVP_REPORT*)NETWORK2->GetMsgRoot( &handle, target->GetConnectionIdx(), NM_PVP, NM_PVP_REPORT_SYN );
	if( sendMsg != NULL )
	{
		wcscpy( sendMsg->mName, hero->GetName() );
		NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_REPORT) );
	}

	// 누적점수 검사
	int cnt = mTeamInfo[team].GetReportCount( targetIndex );
	if( cnt >= PVP_REPORT_TOTAL )
	{
		// 강제퇴장
		target->PvPEndDebuffClear();
		target->AutoPvPOut();
		
		mTeamInfo[team].DelMember( targetIndex );

		// 강제퇴장 알림(대상자)
		HANDLE				  handle  = NULL;
		MSG_SYN_PVP_FORCEOUT* sendMsg = (MSG_SYN_PVP_FORCEOUT*)NETWORK2->GetMsgRoot( &handle, target->GetConnectionIdx(), NM_PVP, NM_PVP_FORCEOUT_SYN );
		if( sendMsg != NULL )
		{
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_FORCEOUT) );
		}

		// 강제퇴장 알림(팀원)
		MSG_SYN_PVP_REPORT_OUT synMsg;
		synMsg.Category = NM_PVP;
		synMsg.Protocol = NM_PVP_REPORT_OUT_SYN;
		wcscpy( synMsg.mName, target->GetName() );
		TeamQuickSendExcept( (ePVPDM_TEAM_TYPE)team, targetIndex, (char*)&synMsg, sizeof( synMsg ) );
	}

	return ERROR_PVP_REPORT_SUCCESS;
}
