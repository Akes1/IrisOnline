#include "stdafx.h"
#include "gamesrv.h"
#include "Gathering.h"
#include "Gathering_Common.h"
#include "GatheringScript.h"
#include "Drop.h"
#include "ObjectManager.h"
#include "AIManager.h"
#include "GridManager.h"
#include "PVPManager.h"


cGathering::cGathering( void )
: cBaseObject( eOBJECTTYPE_GATHERING )
, mRegenIndex(0)
, mDMIdx(0)
, mFlagTeamType( ePVPDM_TEAMTYPE_MAX )
{
}


cGathering::~cGathering( void )
{
}			


void* cGathering::operator new( size_t n )
{
	if( n != sizeof(cGathering) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocGathering();
}


void cGathering::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cGathering) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreeGathering( static_cast<cGathering*>(ptr) );

	return;
}


bool cGathering::GatheringInit( unsigned long objectIdx, unsigned long regenIdx, unsigned long gatheringIdx, unsigned short mapNumber,
							   float posX, float posY, float direction, float height )
{
	mObject.index = objectIdx;
	mRegenIndex = regenIdx;

	mGatheringIdx = gatheringIdx;
	mMapNumber = mapNumber;
	mPosX = posX;
	mPosY = posY;
	mDirection = direction;
	mHeight = height;

	mObjectPos.x = mPosX;
	mObjectPos.y = mPosY;

	/// 채집 리스트 정보
	mpGatheringInfo = (sGatheringList*)GATHERINGSCRIPT->GetGatheringInfo( mGatheringIdx );
	if( mpGatheringInfo == NULL )
		return false;

	/// 리젠 지역 체크
	float mapMaxSize = AIMANAGER->GetPathMaxSideSize( mMapNumber );
	if( mObjectPos.x < 0 || mObjectPos.x > mapMaxSize || mObjectPos.y < 0 || mObjectPos.y > mapMaxSize )
		return false;

	//if( AIMANAGER->IsPossible( mMapNumber, mObjectPos.x, mObjectPos.y, GetObject() ) == false )
	//	return false;

	mRangeCheck.SetRadius( 100 + SYNC_MOVE_RANGE );

	return true;
}


void cGathering::Update( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	cHashMap::cIterator iter = mGatherCharactorMap.Begin();
	for( ; iter != mGatherCharactorMap.End() ; ++iter )
	{
		if( (*iter).mSecond <= accumTime )
		{
			if( mDMIdx )
			{
				GatheringDie_DM( (*iter).mFirst );
			}
			else
			{
				GatheringDie( (*iter).mFirst );
			}
			break;
		}
	}
}


unsigned long cGathering::GetClassIdx()
{
	return mGatheringIdx;
}


void cGathering::SendRegenSync()
{
	/// 메세지 발송
	MSG_GATHERING_INFO msg;
	msg.Category = NM_GATHERING;
	msg.Protocol = NM_GATHERING_REGEN_SYN;
	msg.mObjectIdx = mObject.index;
	msg.mPosX = mObjectPos.x;
	msg.mPosY = mObjectPos.y;
	msg.mGatheringIdx = GetClassIdx();
	msg.mDirection = GetDirection();
	msg.mAppendZ   = GetAppendZ();
	NETWORK2->QuickSend( this, (char*)&msg, sizeof(msg) );
}

void cGathering::SendRegenSync( unsigned char teamType )
{
	/// 메세지 발송
	MSG_GATHERING_HOIST_INFO msg;
	msg.Category = NM_GATHERING;
	msg.Protocol = NM_GATHERING_REGEN_HOIST_SYN;
	msg.mObjectIdx = mObject.index;
	msg.mPosX = mObjectPos.x;
	msg.mPosY = mObjectPos.y;
	msg.mGatheringIdx = GetClassIdx();
	msg.mDirection = GetDirection();
	msg.mAppendZ   = GetAppendZ();
	msg.mTeamType  = teamType;
	NETWORK2->QuickSend( this, (char*)&msg, sizeof(msg) );
}


void cGathering::AddGatherPlayer( cPlayer* pPlayer )
{
	unsigned long gatheringTime = 0;

	try
	{
		if( pPlayer == NULL )
			throw ERROR_GATHERING_FAIL;

		pPlayer->MoveStop();

		sGatheringList* pInfo = GATHERINGSCRIPT->GetGatheringInfo( mGatheringIdx );
		if( pInfo == NULL )
		{
			NETWORK2->PostServerEvent("cGathering::AddGatherPlayer pInfo == NULL %d,%d,%d", NETWORK2->GetServerType(),
				pPlayer->GetObjectID(), mGatheringIdx );
			throw ERROR_GATHERING_FAIL;
		}

		/// 거리 체크
		if( mRangeCheck.IsRange( pPlayer->GetPos(), mObjectPos ) == false )
			throw ERROR_GATHERING_RANGE;

		/// 플레이어 상태 체크 ( 스턴, 슬립 )
		if( pPlayer->GetStateOddity( eODDITYTYPE_SLEEP ) != 0 )
		{
			NETWORK2->PostServerEvent("cGathering::AddGatherPlayer eODDITYTYPE_SLEEP %d,%d,%d", NETWORK2->GetServerType(),
				pPlayer->GetObjectID(), mGatheringIdx );
			throw ERROR_GATHERING_FAIL;
		}

		if( pPlayer->GetStateOddity( eODDITYTYPE_STUN ) != 0 )
		{
			NETWORK2->PostServerEvent("cGathering::AddGatherPlayer eODDITYTYPE_STUN %d,%d,%d", NETWORK2->GetServerType(),
				pPlayer->GetObjectID(), mGatheringIdx );
			throw ERROR_GATHERING_FAIL;
		}

		/// 버프 체크
		if( pInfo->mNeedBuffIdx != 0 )
		{
			if( pPlayer->IsInfluenceHave( pInfo->mNeedBuffIdx ) == false )
			{
				throw ERROR_GATHERING_BUFF;
			}
		}

		/// 레벨 체크
		if( pPlayer->GetLevel() < pInfo->mNeedLevel )
			throw ERROR_GATHERING_LEVEL;

		/// 장비 체크
		if( pInfo->mNeedItemIdx != 0 && pPlayer->IsItem( pInfo->mNeedItemIdx ) == false )
			throw ERROR_GATHERING_ITEM;
				
		/// 퀘스트 체크
        if( pInfo->mNeedQuestIdx != 0 && pPlayer->IsCompleteGathering( pInfo->mNeedQuestIdx ) == true )
			throw ERROR_GATHERING_QUEST;
        
		/// 전문기술 체크
		if( pInfo->mNeedMakeIdx != 0 && pPlayer->IsMakeSkill( pInfo->mNeedMakeIdx ) == false )
			throw ERROR_GATHERING_MAKESKILL;

		/// 깃발 뺏기시 깃발 채집이 가능한지 체크
		if( NETWORK2->GetInDunMapNum() == MAP_FLAG )
		{
			unsigned long pvpIdx = pPlayer->GetPvPDMIdx();
			if( pvpIdx != 0 )
			{
				cDeathMatch2Object* pPvP = (cDeathMatch2Object*)PVPMANAGER->GetPvPDMObject( pvpIdx );
				if( pPvP != NULL )
				{
					if( pPvP->IsGathering( pPlayer->GetObjectID(), mObject.index ) == false )
						throw ERROR_GATHERING_PVP;
				}
			}
		}

		/// 플레이어 상태 변경
		if( pPlayer->ChangeState( ePLAYER_STATE_GATHERING ) == false )
		{
			NETWORK2->PostServerEvent("cGathering::AddGatherPlayer pPlayer->ChangeState %d,%d,%d,%d,%d", NETWORK2->GetServerType(),
				pPlayer->GetObjectID(), mGatheringIdx, pPlayer->GetState(), pPlayer->GetStateStop() );
			throw ERROR_GATHERING_FAIL;
		}

		gatheringTime = pInfo->mGatheringTime;

		throw ERROR_GATHERING_SUCCESS;
	}
	catch (int e)
	{
		/// 성공/실패 메세지 발송
		NETWORK2->SendMsgError( pPlayer->GetConnectionIdx(), NM_GATHERING, NM_GATHERING_START_RES, e );

		/// 성공시 부가 처리
		if( e == ERROR_GATHERING_SUCCESS )
		{
			pPlayer->SetGatheringIdx( mObject.index );

			if( mGatherCharactorMap.Insert( pPlayer->GetObjectID(), NETWORK2->GetAccumTime() + gatheringTime ) == false )
				NETWORK2->PostServerEvent("cGathering::AddGatherPlayer duplicate(%d)", pPlayer->GetObjectID() );

			MSG_SYN_GATHERING_START msg;
			msg.Category = NM_GATHERING;
			msg.Protocol = NM_GATHERING_START_SYN;
			msg.mObjectIdx = mObject.index;
			msg.mClassIdx = mpGatheringInfo->mGatheringIdx;
			msg.mCharacterIdx = pPlayer->GetObjectID();
			NETWORK2->QuickSendExcept( pPlayer, (char*)&msg, sizeof(msg) );
		}
	}
}


void cGathering::ReqEraseGatherPlayer( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	/// 플레이어 상태 변경
	if( pPlayer->ChangeState( eOBJECT_STATE_IDLE ) == false )
		return;

	/// 플레이어와 채집중인 인덱스 변경
	pPlayer->SetGatheringIdx( 0 );

	mGatherCharactorMap.Erase( pPlayer->GetObjectID() );

	/// 성공
	NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx(), NM_GATHERING, NM_GATHERING_CANCEL_SUCCESS_RES );

	MSG_SYN_GATHERING_CANCEL msg;
	msg.Category = NM_GATHERING;
	msg.Protocol = NM_GATHERING_CANCEL_SYN;
	msg.mCharacterIdx = pPlayer->GetObjectID();
	NETWORK2->QuickSendExcept( this, (char*)&msg, sizeof(msg) );
}

void cGathering::CancelGatherByOddity( cPlayer* pPlayer )
{
	if( pPlayer != NULL )
	{
		if( pPlayer->GetState() == ePLAYER_STATE_GATHERING )
		{
			// 채집중일때는 반드시 채집 인덱스가 존재해야 한다
			unsigned long gatherIdx = pPlayer->GetGatheringIdx();
			if( gatherIdx == 0 )
			{
				NETWORK2->PostServerEvent("cGathering::CancelGatherByOddity : error state - eOBJECTTYPE_GATHERING(%d)", gatherIdx );
				return;
			}

			// 취소 처리
			if( pPlayer->ChangeState( eOBJECT_STATE_IDLE ) == false )
				return;

			pPlayer->SetGatheringIdx( 0 );

			mGatherCharactorMap.Erase( pPlayer->GetObjectID() );

			// 전체 쏘기
			MSG_SYN_GATHERING_CANCEL msg;
			msg.Category = NM_GATHERING;
			msg.Protocol = NM_GATHERING_CANCEL_SYN;
			msg.mCharacterIdx = pPlayer->GetObjectID();
			NETWORK2->QuickSend( this, (char*)&msg, sizeof(msg) );
		}
	}
	else
	{
		NETWORK2->PostServerEvent("cGathering::CancelGatherByOddity : not exist user");
	}
}

void cGathering::EraseGatherPlayer( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	mGatherCharactorMap.Erase( pPlayer->GetObjectID() );
}


void cGathering::GatheringDie( unsigned long playerIdx )
{
	/// 기존 유저들 상태 복구
	cHashMap::cIterator iter = mGatherCharactorMap.Begin();
	for( ; iter != mGatherCharactorMap.End() ; ++iter )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( (*iter).mFirst );
		if( pPlayer != NULL )
		{
			if( pPlayer->GetState() == ePLAYER_STATE_GATHERING )
				pPlayer->ChangeState( eOBJECT_STATE_IDLE );

			MSG_SYN_GATHERING_END msg;
			msg.Category = NM_GATHERING;
			if( playerIdx == (*iter).mFirst )
				msg.Protocol = NM_GATHERING_END_SUCCESS_SYN;
			else
				msg.Protocol = NM_GATHERING_END_FAILE_SYN;
			msg.mObjectIdx = mObject.index;
			msg.mCharacterIdx = (*iter).mFirst;

			/// 메세지 발송
			NETWORK2->QuickSend( pPlayer, (char*)&msg, sizeof(msg) );
		}
	}

	/// 드랍 처리
	if( playerIdx != 0 )
		DROP->GatherDrop( this, playerIdx );

	/// 객체 삭제 메세지...
	MSG_GATHERINGIDX delMsg;
	delMsg.Category = NM_GATHERING;
	delMsg.Protocol = NM_GATHERING_DEL_SYN;
	delMsg.mObjectIdx = mObject.index;
	NETWORK2->QuickSend( this, (char*)&delMsg, sizeof(delMsg) );

	/// 객체 삭제 대기열에 등록
	GRIDMANAGER->RemoveGathering( this );
	OBJECTMANAGER->InsertDeleteGathering( mObject.index );
	AIMANAGER->InsertReadyGathering( mObject.index, mMapNumber, mRegenIndex );
}

void cGathering::GatheringDie_DM( unsigned long playerIdx )
{
	/// 기존 유저들 상태 복구
	cHashMap::cIterator iter = mGatherCharactorMap.Begin();
	for( ; iter != mGatherCharactorMap.End() ; ++iter )
	{
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( (*iter).mFirst );
		if( pPlayer != NULL )
		{
			if( pPlayer->GetState() == ePLAYER_STATE_GATHERING )
				pPlayer->ChangeState( eOBJECT_STATE_IDLE );

			MSG_SYN_GATHERING_END msg;
			msg.Category = NM_GATHERING;
			if( playerIdx == (*iter).mFirst )
				msg.Protocol = NM_GATHERING_END_SUCCESS_SYN;
			else
				msg.Protocol = NM_GATHERING_END_FAILE_SYN;
			msg.mObjectIdx = mObject.index;
			msg.mCharacterIdx = (*iter).mFirst;

			/// 메세지 발송
			NETWORK2->QuickSend( pPlayer, (char*)&msg, sizeof(msg) );
		}
	}

	/// 객체 삭제 메세지...
	MSG_GATHERINGIDX delMsg;
	delMsg.Category = NM_GATHERING;
	delMsg.Protocol = NM_GATHERING_DEL_SYN;
	delMsg.mObjectIdx = mObject.index;
	NETWORK2->QuickSend( this, (char*)&delMsg, sizeof(delMsg) );

	/// 객체 삭제 대기열에 등록
	GRIDMANAGER->RemoveGathering( this );
	OBJECTMANAGER->InsertDeleteGathering( mObject.index );

	/// 
	if( mDMIdx == 0 )
	{
		assert(0);
	}

	cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( mDMIdx );
	if( pDM && pDM->GetType() == ePVPDM_OBJECTTYPE_2 )
	{
		cDeathMatch2Object* p = (cDeathMatch2Object*)pDM;

		p->ChangeGatheringState( GetObjectID(), playerIdx );
	}
}

unsigned long cGathering::GetInfluence()
{
	if( mpGatheringInfo == NULL )
		return 0;

	return mpGatheringInfo->mInfluenceIdx;
}

// SendSightIn Method
bool cGathering::SendSightIn(char category, char protocol, unsigned long connectionIdx)
{
	if( mFlagTeamType == ePVPDM_TEAMTYPE_MAX )
	{
		HANDLE          handle  = NULL;
		MSG_GATHERING_INFO* sendMsg = (MSG_GATHERING_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
		if ( sendMsg != NULL )
		{
			sendMsg->mObjectIdx         = mObject.index;
			sendMsg->mPosX              = mObjectPos.x;
			sendMsg->mPosY              = mObjectPos.y;
			sendMsg->mGatheringIdx      = GetClassIdx();
			sendMsg->mDirection			= GetDirection();
			sendMsg->mAppendZ			= GetAppendZ();

			return NETWORK2->SendMsgRoot( handle, sizeof(MSG_GATHERING_INFO) );
		}
	}
	else
	{
		HANDLE          handle  = NULL;
		MSG_GATHERING_HOIST_INFO* sendMsg = (MSG_GATHERING_HOIST_INFO*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, NM_GATHERING_HOIST_SIGHT_IN_RES );
		if ( sendMsg != NULL )
		{
			sendMsg->mObjectIdx         = mObject.index;
			sendMsg->mPosX              = mObjectPos.x;
			sendMsg->mPosY              = mObjectPos.y;
			sendMsg->mGatheringIdx      = GetClassIdx();
			sendMsg->mDirection			= GetDirection();
			sendMsg->mAppendZ			= GetAppendZ();
			sendMsg->mTeamType			= mFlagTeamType;

			return NETWORK2->SendMsgRoot( handle, sizeof(MSG_GATHERING_HOIST_INFO) );
		}
	}
	return false;
}


// SendSightOut Method
bool cGathering::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE            handle  = NULL;
	MSG_GATHERINGIDX* sendMsg = (MSG_GATHERINGIDX*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mObjectIdx = mObject.index;
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_GATHERINGIDX) );
	}
	return false;
}
