#include "stdafx.h"
#include "gamesrv.h"
#include "ThemeObject.h"
#include "ThemeManager.h"
#include "Protocol.h"
#include "AIManager.h"
#include "ObjectManager.h"
#include "Obstruction_Common.h"
#include "StageScript.h"
#include "GridManager.h"
#include "TeleportPortal_Common.h"
#include "GatheringScript.h"
#include "NpcScript.h"
#include "Monster.h"

cThemeObject::cThemeObject()
: mDependency( false )
{
	mMonsterCntTest = 0;
}

cThemeObject::~cThemeObject()
{
	RelaseObject();
}


void* cThemeObject::operator new( size_t n )
{
	if( n != sizeof(cThemeObject) ) 
	{
		NETWORK2->PostServerEvent("cThemeObject::operator new %d != sizeof(cThemeObject)", n );
		return NULL;
	}

	return THEMEMANAGER->AllocThemeObject();
}


void cThemeObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		NETWORK2->PostServerEvent("cThemeObject::operator delete ptr == 0" );
		return;
	}

	if( n != sizeof(cThemeObject) ) 
	{
		NETWORK2->PostServerEvent("cThemeObject::operator delete n != sizeof(cThemeObject", n );
		return;
	}

	THEMEMANAGER->FreeThemeObject( static_cast<cThemeObject*>(ptr) );

	return;
}


bool cThemeObject::InitTheme( unsigned short themeIndex, unsigned char mode )
{
	mThemeRoomIndex = themeIndex;
	mMode = mode;
	mServerOnRegen = true;
	mMonsterCntTest = 0;

	unsigned short mapDataNum = NETWORK2->GetInDunMapNum();
	sMapInfo* info = STAGESCRIPT->GetMapFolderIdx( mapDataNum, mMode );
	if( info == NULL )
	{
		NETWORK2->PostServerEvent("cThemeObject::Init - not exist sMapInfo (%d) == false", mapDataNum );
		return false;
	}

	mMaxCount = info->maxPlayer;

	if( AIMANAGER->InitMonsterRegen( mode, &mRegenGroupMap, &mFamilyMap, &mRegenSwitchMap ) == false )
	{
		NETWORK2->PostServerEvent("cThemeObject::Init AIMANAGER->InitMonsterRegen(%d) == false", themeIndex );
		return false;
	}

	/// 장애물(문) 스크립트 데이터 로드
	cPArray* pObstruction = STAGESCRIPT->GetManagedDoorArr( NETWORK2->GetInDunMapNum() );
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

	// NPC 리젠
	typedef const tPointerHashMap<unsigned long, sNPCRegen*> cNpcRegenMap;
	cNpcRegenMap* pNpcMap = NPCSCRIPT->GetNPCRegenMap( mMode );
	if( pNpcMap != NULL )
	{
		cNpcRegenMap::cConstIterator i = pNpcMap->Begin();
		cNpcRegenMap::cConstIterator end = pNpcMap->End();
		for( ; i != end; ++i )
		{
			//unsigned long npcIndex = (unsigned long)(i->mFirst);
			unsigned long npcIndex = THEMEMANAGER->GetNpcGenIdx();
			sNPCRegen*	 regenData = (sNPCRegen*)(i->mSecond);
			if( regenData == NULL )
			{
				NETWORK2->PostServerEvent("cTutorialObject::InitTheme regenData[%d] == NULL", npcIndex );
				continue;
			}

			//
			cNpc* pNpc = OBJECTMANAGER->AddNpc( npcIndex, regenData->mNpcClassIdx, mThemeRoomIndex, 
				regenData->mPosX, regenData->mPosY, regenData->mDirection );

			if( pNpc == NULL )
			{
				NETWORK2->PostServerEvent("cTutorialObject::InitTheme pNpc == NULL[%d]", npcIndex );
				continue;
			}

			//
			if( GRIDMANAGER->AddNpc( pNpc ) == false )
			{
				NETWORK2->PostServerEvent("cTutorialObject::InitTheme GRIDMANAGER->AddNpc == NULL[%d]", npcIndex );
				continue;
			}

			mNpcArr.PushBack( npcIndex );
		}
	}

	// 채집물 리젠
	typedef tPointerHashMap<unsigned long, void*> cHashMap;
	cHashMap* gatherMap = GATHERINGSCRIPT->GetRegenGatheringMap( mode );
	if( gatherMap == NULL )
	{
		NETWORK2->PostServerEvent( "gatherMap == NULL" );
		return false;
	}

	cHashMap* pGroupMap = new cHashMap;

	cHashMap::cIterator i = gatherMap->Begin();
	cHashMap::cIterator end = gatherMap->End();
	for( ; i != end; ++i )
	{
		unsigned long regenIdx		  = (unsigned long)(i->mFirst);
		sGatheringRegenHeader* header = (sGatheringRegenHeader*)(i->mSecond);
		if( header == NULL )
		{
			assert(0);
			continue;
		}

		sRegenGather* regen = new sRegenGather;
		regen->mRegenIdx = regenIdx;

		unsigned long randTime = header->mRandTime;
		if( randTime != 0 )
		{
			randTime = rand() % header->mRandTime;
		}

		regen->mRegenTime = NETWORK2->GetAccumTime() + header->mRegenWaitTime + randTime;
		regen->mRegenReady = true;

		if( pGroupMap->Insert( regenIdx, regen ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("InitTheme::mpRegenGatherGroupMap Insert duplicate[%d]", regenIdx );
			return false;
		}
	}

	if( mGatherRegenGroupMap.Insert( mode, pGroupMap ) == false )
		return false;

	mGatheringSet.Clear();

	return true;
}

void cThemeObject::RelaseObject()
{
	{
		// 채집물 제거
		cPHashMap::cIterator i = mGatherRegenGroupMap.Begin();
		cPHashMap::cIterator end = mGatherRegenGroupMap.End();
		for( ; i != end; ++i )
		{
			cPHashMap* p = (cPHashMap*)(i->mSecond);
			if( p != NULL )
			{
				cPHashMap::cIterator pi = p->Begin();
				cPHashMap::cIterator pend = p->End();
				for( ; pi != pend; ++pi )
				{
					sRegenGather* regen = (sRegenGather*)(pi->mSecond);
					SAFE_DELETE( regen );
				}
				p->Clear();
			}
			SAFE_DELETE( p );
		}
		mGatherRegenGroupMap.Clear();
	}

	{
		cHashSet::cIterator i = mGatheringSet.Begin();
		cHashSet::cIterator e = mGatheringSet.End();
		for( ; i != e ; ++i )
		{
			unsigned long objectIdx = (unsigned long)(*i);
			cGathering* pGathering = OBJECTMANAGER->GetGathering( objectIdx );
			if( pGathering != NULL )
				GRIDMANAGER->RemoveGathering( pGathering );
			OBJECTMANAGER->RemoveGathering( objectIdx );
		}
		mGatheringSet.Clear();
	}

	{
		// npc 정보 제거
		for( unsigned int i = 0; i < mNpcArr.GetSize(); ++i )
		{
			unsigned long npcIndex = mNpcArr[i];

			cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIndex );
			if( pNpc == NULL )
				continue;

			if( GRIDMANAGER->RemoveNpc( pNpc ) == false )
			{
				NETWORK2->PostServerEvent("cTutorialObject::~cTutorialObject() GRIDMANAGER->RemoveNpc( %d ) == false", npcIndex );
			}

			if( OBJECTMANAGER->RemoveNpc( npcIndex ) == false )
			{
				NETWORK2->PostServerEvent("cTutorialObject::~cTutorialObject() OBJECTMANAGER->RemoveNpc( %d ) == false", npcIndex );
			}

			THEMEMANAGER->DelNpcGenIdx( npcIndex );
		}
		mNpcArr.Clear();
	}

	cPHashMap::cIterator iter;

	{
		for( iter = mFamilyMap.Begin() ; iter != mFamilyMap.End() ; ++iter )
		{
			cHashSet* pSet = (cHashSet*)(*iter).mSecond;
			SAFE_DELETE( pSet );
		}
		mFamilyMap.Clear();
	}

	{
		for( iter = mRegenSwitchMap.Begin() ; iter != mRegenSwitchMap.End() ; ++iter )
		{
			sSwichGroup* pSwitch = (sSwichGroup*)(*iter).mSecond;
			SAFE_DELETE( pSwitch );
		}
		mRegenSwitchMap.Clear();
	}

	{
		for( iter = mRegenGroupMap.Begin() ; iter != mRegenGroupMap.End() ; ++iter )
		{
			sRegenGroup* pGroup = (sRegenGroup*)(*iter).mSecond;

			for( unsigned long i = 0 ; i < pGroup->mMonsterList.GetSize() ; ++i )
			{
				sRegenMonster* pRegenMonster = (sRegenMonster*)pGroup->mMonsterList[i];
				if( pRegenMonster != NULL )
				{
					cMonster* pMonObject = OBJECTMANAGER->GetMonster( pRegenMonster->mObjectIdx );
					if( pMonObject != NULL )
					{
						pMonObject->Release();
						GRIDMANAGER->RemoveMonster( pMonObject );
					}
					OBJECTMANAGER->RemoveMonster( pRegenMonster->mObjectIdx );				
				}
				SAFE_DELETE( pRegenMonster );
			}
			SAFE_DELETE( pGroup );
		}
		mRegenGroupMap.Clear();
	}

	mRegenSearch.Clear();
	mObstructionMap.Clear();
	mTeleportSet.Clear();
	mMemberSet.Clear();
}

void cThemeObject::ProcessTheme( unsigned long elapsedTime, unsigned long accumTime )
{
	if( mMemberSet.GetSize() > 0 )
	{
		AIMANAGER->ProcessMonRegen( mMode, &mRegenGroupMap, &mRegenSwitchMap, &mFamilyMap, &mRegenSearch, elapsedTime, accumTime, mThemeRoomIndex, &mServerOnRegen );
		AIMANAGER->ProcessMonPatternMove( &mFamilyMap, &mRegenSearch, elapsedTime, accumTime );

		// 채집물 처리
		AIMANAGER->ProcessGetRegen( accumTime, mMode, mThemeRoomIndex, &mGatherRegenGroupMap, &mGatheringSet );
	}
}

bool cThemeObject::ThemeAddPlayer( unsigned long playerIdx )
{
	unsigned int count = mMemberSet.GetSize();
	if( count < mMaxCount )
	{
		cHashSet::cIterator i = mMemberSet.Find( playerIdx );
		if( i != mMemberSet.End() )
			return true;

		// 등록
		mMemberSet.Insert( playerIdx );
	}
	else
	{
		// 최대 인원수 넘어감
		NETWORK2->PostServerEvent("cThemeObject::ThemeAddPlayer mMemberCnt[%d,%d,%d] >= mMaxCount(%d)", 
									playerIdx, mThemeRoomIndex, count, mMaxCount );
		return false;
	}
	return true;
}

void cThemeObject::ThemeOutPlayer( unsigned long playerIdx )
{
	unsigned int count = mMemberSet.GetSize();
	if( count == 0 )
	{
		NETWORK2->PostServerEvent("cThemeObject::ThemeOutPlayer mMemberCnt == 0");
		return;
	}

	// 삭제
	mMemberSet.Erase( playerIdx );

	if( mMemberSet.GetSize() == 0 )
	{
		mMonsterCntTest = 0;

		if( mDependency == true )
		{
			cPHashMap::cIterator pS = mRegenGroupMap.Begin();
			cPHashMap::cIterator pE = mRegenGroupMap.End();
			for( ; pS != pE ; ++pS )
			{
				sRegenGroup* pGroup = (sRegenGroup*)(*pS).mSecond;
				if( pGroup == NULL )
					continue;

				for( unsigned long i = 0 ; i < pGroup->mMonsterList.GetSize() ; ++i )
				{
					sRegenMonster* pRegenMonster = (sRegenMonster*)pGroup->mMonsterList[i];
					if( pRegenMonster == NULL )
						continue;

					if( pRegenMonster->mObjectIdx == 0 )
						continue;

					cMonster* pMonster = OBJECTMANAGER->GetMonster( pRegenMonster->mObjectIdx );
					if( pMonster == NULL )
						continue;

					MSG_SYN_MONSTER_DESTROY synMsg;
					synMsg.Category			= NM_MONSTER;
					synMsg.Protocol			= NM_MONSTER_DESTROY_SYN;
					synMsg.mMonsterIdx	    = pRegenMonster->mObjectIdx;
					NETWORK2->QuickSend( pMonster, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_DESTROY) );

					cMonster* pMonObject = OBJECTMANAGER->GetMonster( pRegenMonster->mObjectIdx );
					if( pMonObject != NULL )
					{
						GRIDMANAGER->RemoveMonster( pMonObject );
					}
					OBJECTMANAGER->RemoveMonster( pRegenMonster->mObjectIdx );

					pRegenMonster->mObjectIdx = 0;
					pRegenMonster->mDie = true;
				}

				pGroup->mMonsterAliveCnt = 0;
			}	

			/// 채집물 제거
			{
				cHashSet::cIterator i = mGatheringSet.Begin();
				cHashSet::cIterator e = mGatheringSet.End();
				for( ; i != e ; ++i )
				{
					unsigned long objectIdx = (unsigned long)(*i);
					cGathering* pGathering = OBJECTMANAGER->GetGathering( objectIdx );
					if( pGathering != NULL )
					{
						/// 객체 삭제 메세지...
						MSG_GATHERINGIDX delMsg;
						delMsg.Category = NM_GATHERING;
						delMsg.Protocol = NM_GATHERING_DEL_SYN;
						delMsg.mObjectIdx = objectIdx;
						NETWORK2->QuickSendExcept( (cBaseObject*)pGathering, (char*)&delMsg, sizeof(delMsg) );

						GRIDMANAGER->RemoveGathering( pGathering );
					}
					OBJECTMANAGER->InsertDeleteGathering( objectIdx );
				}
				mGatheringSet.Clear();
			}

			typedef tPointerHashMap<unsigned long, void*> cHashMap;

			{
				cPHashMap::cIterator i = mGatherRegenGroupMap.Begin();
				cPHashMap::cIterator end = mGatherRegenGroupMap.End();
				for( ; i != end; ++i )
				{
					cHashMap* p = (cHashMap*)(i->mSecond);
					if( p != NULL )
					{
						cHashMap::cIterator pi = p->Begin();
						cHashMap::cIterator pend = p->End();
						for( ; pi != pend; ++pi )
						{
							/// 채집물 리젠 안함으로 설정
							sRegenGather* regen = (sRegenGather*)(pi->mSecond);
							if( regen != NULL )
							{
								regen->mRegenReady = true;
								regen->mRegenTime = 0;
							}
						}
					}
				}
			}
		}
		else
		{
			// 현재 테마에 종속되어있는 유저가 없으면 삭제
			THEMEMANAGER->InsertDelTheme( mThemeRoomIndex );
		}
	}
}

void cThemeObject::ThemeAllOut()
{
	cHashSet::cIterator i = mMemberSet.Begin();
	cHashSet::cIterator end = mMemberSet.End();
	for( ; i != end; ++i )
	{
		unsigned long playerIndex = (unsigned long)(*i);

		cPlayer* player = OBJECTMANAGER->GetPlayer( playerIndex );
		if( player == NULL )
		{
			NETWORK2->PostServerEvent("cThemeObject::ThemeAllOut player not exist(%d,%d)", playerIndex, NETWORK2->GetInDunMapNum() );
			continue;
		}

		// 인던 종료 플래그 세움
		player->SetInDunJoin( false );
		player->SetThemeRoomIdx( 0 );

		/// 게임서버로 이동하라고 알림
		NETWORK2->SendMsgRoot( player->GetConnectionIdx(), NM_THEME, NM_THEME_END_SYN );
	}
}

bool cThemeObject::IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos )
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

void cThemeObject::SendObstruction( cPlayer* pPlayer )
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

void cThemeObject::DeleteObstruction( unsigned long obstructionIdx )
{
	// 장애물 삭제 알림
	cHashSet::cIterator i = mMemberSet.Begin();
	cHashSet::cIterator end = mMemberSet.End();
	for( ; i != end; ++i )
	{
		unsigned long playerIndex = (unsigned long)(*i);

		cPlayer* player = OBJECTMANAGER->GetPlayer( playerIndex );
		if( player == NULL )
		{
			NETWORK2->PostServerEvent("cThemeObject::DeleteObstruction player not exist(%d,%d)", playerIndex, NETWORK2->GetInDunMapNum() );
			continue;
		}

		HANDLE						handle = NULL;
		MSG_SYN_OBSTRUCTION_DELETE* sendMsg = (MSG_SYN_OBSTRUCTION_DELETE*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), 
																								NM_OBSTRUCTION, NM_OBSTRUCTION_DELETE_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mObstructionIdx = obstructionIdx;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_OBSTRUCTION_DELETE) );
		}
	}

	// 장애물삭제
	mObstructionMap.Erase( obstructionIdx );
}

void cThemeObject::AddTeleportPortal( unsigned long mapChangeIdx )
{ 
	if( mTeleportSet.Insert( mapChangeIdx ) == false )
		NETWORK2->PostServerEvent("cThemeObject::AddTeleportPortal %d", mapChangeIdx );

	cHashSet::cIterator i = mMemberSet.Begin();
	cHashSet::cIterator end = mMemberSet.End();
	for( ; i != end; ++i )
	{
		unsigned long playerIndex = (unsigned long)(*i);

		cPlayer* player = OBJECTMANAGER->GetPlayer( playerIndex );
		if( player == NULL )
		{
			NETWORK2->PostServerEvent("cThemeObject::AddTeleportPortal player not exist(%d,%d,%d)", playerIndex, mapChangeIdx, NETWORK2->GetInDunMapNum() );
			continue;
		}

		HANDLE						handle = NULL;
		MSG_SYN_TELEPORTPORTAL_CREATE* pMsg = (MSG_SYN_TELEPORTPORTAL_CREATE*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx(), 
																									NM_TELEPORTPORTAL, NM_TELEPORTPORTAL_CREATE_SYN );
		if ( pMsg != NULL )
		{
			pMsg->mMapChangePosIdx = mapChangeIdx;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TELEPORTPORTAL_CREATE) );
		}
	}
}

int cThemeObject::UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx )
{
	if( pPlayer == NULL )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	/// 2차상태 체크
	if( pPlayer->IsRequestRejection() == true )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	/// 맵이동 가능인지 현재 상태 체크
	if( pPlayer->IsChangeState( eOBJECT_STATE_MOVE ) == false )
		return ERROR_TELEPORTPORTAL_USE_STATE;

	cHashSet::cIterator b = mTeleportSet.Begin();
	cHashSet::cIterator e = mTeleportSet.End();
	if( b == e )
	{
		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal b == e %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_PORTAL;
	}

	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( mapChangeIdx );
	if( scriptPos == NULL )
	{
		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
	}

	/// 이동 좌표 오류
	if( scriptPos->mMapNumber == 0 )
	{
		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
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
		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal pTargetInfo == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
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

void cThemeObject::SendTeleportPortal( cPlayer* pPlayer )
{
	if( pPlayer == NULL )
		return;

	cHashSet::cIterator b = mTeleportSet.Begin();
	cHashSet::cIterator e = mTeleportSet.End();

	HANDLE handle = NULL;
	MSG_SYN_TELEPORTPORTAL_LIST* pMsg = (MSG_SYN_TELEPORTPORTAL_LIST*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
		NM_TELEPORTPORTAL, NM_TELEPORTPORTAL_LIST_SYN );
	if ( pMsg != NULL )
	{
		unsigned char i = 0;
		for( ; b != e ; ++b )
		{
			unsigned long idx = *b;
			if( idx == 0 )
				continue;

			pMsg->mMapChangePosIdx[i] = idx;
			++i;
		}

		pMsg->mCount = i;

		NETWORK2->SendMsgRoot( handle, pMsg->GetMsgLength() );
	}
}

sStageChangePos* cThemeObject::GetThemeOutPos()
{
	cPArray* pAry = STAGESCRIPT->GetMapChangeStarterArr( NETWORK2->GetInDunMapNum() );
	if( pAry == NULL )
		return NULL;

	if( pAry->GetSize() == 0 )
		return NULL;

	return (sStageChangePos*)(*pAry)[0];

}

bool cThemeObject::MonsterCntTestObject( unsigned short mapNum, unsigned long monsterIdx, unsigned long classIdx, unsigned long regenIdx )
{
	if( mMonsterCntTest < 100000000 )
		++mMonsterCntTest;

	if( mMonsterCntTest == 300 || mMonsterCntTest == 301 || mMonsterCntTest == 302 )
	{
		NETWORK2->PostServerEvent("cThemeObject::MonsterCntTestObject %d[%d,%d,%d,%d]%d", 
			mMonsterCntTest, mapNum, monsterIdx, classIdx, regenIdx, mMemberSet.GetSize() );
	}

	if( mMonsterCntTest > 302 )
		return false;

	return true;
}

