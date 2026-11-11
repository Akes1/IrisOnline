#include "stdafx.h"
#include "gamesrv.h"
#include "CircusObject.h"
#include "ThemeManager.h"
#include "Protocol.h"
#include "NpcScript.h"
#include "ObjectManager.h"
#include "StageScript.h"
#include "GridManager.h"

cTutorialObject::cTutorialObject()
: mLastAccumTime(0)
{

}

cTutorialObject::~cTutorialObject()
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
	}
	mNpcArr.Clear();
}

void* cTutorialObject::operator new( size_t n )
{
	if( n != sizeof(cTutorialObject) ) 
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator new %d != sizeof(cTutorialObject)", n );
		return NULL;
	}

	return THEMEMANAGER->AllocThemeObject();
}


void cTutorialObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator delete ptr == 0" );
		return;
	}

	if( n != sizeof(cTutorialObject) ) 
	{
		NETWORK2->PostServerEvent("cTutorialObject::operator delete n != sizeof(cTutorialObject)", n );
		return;
	}

	THEMEMANAGER->FreeThemeObject( static_cast<cTutorialObject*>(ptr) );
	return;
}

bool cTutorialObject::InitTheme( unsigned short themeIdx, unsigned char mode )
{
	if( cThemeObject::InitTheme( themeIdx, mode ) == false )
		return false;

	// NPC 리젠
	typedef const tPointerHashMap<unsigned long, sNPCRegen*> cNpcRegenMap;
	cNpcRegenMap* pNpcMap = NPCSCRIPT->GetNPCRegenMap( 0 );
	if( pNpcMap != NULL )
	{
		cNpcRegenMap::cConstIterator i = pNpcMap->Begin();
		cNpcRegenMap::cConstIterator end = pNpcMap->End();
		for( ; i != end; ++i )
		{
			unsigned long npcIndex = (unsigned long)(i->mFirst);
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

	return true;
}

void cTutorialObject::ProcessTheme( unsigned long elapsedTime, unsigned long accumTime )
{
	cThemeObject::ProcessTheme( elapsedTime, accumTime );

	//
	if( mLastAccumTime == 0 )
		return;

	// 일정시간이 지나면 다시 리젠
	if( accumTime - mLastAccumTime >= DOOR_REGENTIME )
	{
		RegenDoor();

		mLastAccumTime = 0;
	}
}

void cTutorialObject::DeleteObstruction( unsigned long obstructionIdx )
{
	cThemeObject::DeleteObstruction( obstructionIdx );

	// 마지막 삭제 시간 저장
	mLastAccumTime = NETWORK2->GetAccumTime();
}

void cTutorialObject::RegenDoor()
{
	unsigned short mapDataNum = NETWORK2->GetInDunMapNum();
	cPArray* pObstruction = STAGESCRIPT->GetManagedDoorArr( mapDataNum );
	if( pObstruction == NULL )
		return;

	for( unsigned long i = 0 ; i < pObstruction->GetSize() ; ++i )
	{
		sDoorInfo* pDoor = (sDoorInfo*)(*pObstruction)[i];
		if( pDoor == NULL )
			continue;

		mObstructionMap.Insert( pDoor->mIndex, pDoor );
	}

	// 보내주기
	cHashSet::cIterator b = mMemberSet.Begin();
	cHashSet::cIterator end = mMemberSet.End();
	for( ; b != end; ++b )
	{
		unsigned long playerIndex = (unsigned long)(*b);

		cPlayer* player = OBJECTMANAGER->GetPlayer( playerIndex );
		if( player == NULL )
		{
			NETWORK2->PostServerEvent("cThemeObject::RegenDoor player not exist(%d,%d)", playerIndex, NETWORK2->GetInDunMapNum() );
			continue;
		}

		SendObstruction( player );
	}
}














//cThemeObject::cThemeObject()
//: mMemberCnt( 0 )
//, mDependency( false )
//{
//}
//
//
//cThemeObject::~cThemeObject()
//{
//	cPHashMap::cIterator iter;
//
//	for( iter = mpFamilyMap->Begin() ; iter != mpFamilyMap->End() ; ++iter )
//	{
//		cHashSet* pSet = (cHashSet*)(*iter).mSecond;
//		SAFE_DELETE( pSet );
//	}
//	mpFamilyMap->Clear();
//
//	for( iter = mpRegenSwitchMap->Begin() ; iter != mpRegenSwitchMap->End() ; ++iter )
//	{
//		sSwichGroup* pSwitch = (sSwichGroup*)(*iter).mSecond;
//		SAFE_DELETE( pSwitch );
//	}
//	mpRegenSwitchMap->Clear();
//
//	for( iter = mpRegenGroupMap->Begin() ; iter != mpRegenGroupMap->End() ; ++iter )
//	{
//		sRegenGroup* pGroup = (sRegenGroup*)(*iter).mSecond;
//
//		for( unsigned long i = 0 ; i < pGroup->mMonsterList.GetSize() ; ++i )
//		{
//			sRegenMonster* pMonster = (sRegenMonster*)pGroup->mMonsterList[i];
//			if( pMonster != NULL )
//			{
//				cMonster* pMonObject = OBJECTMANAGER->GetMonster( pMonster->mObjectIdx );
//				if( pMonObject != NULL )
//					GRIDMANAGER->RemoveMonster( pMonObject );
//				OBJECTMANAGER->RemoveMonster( pMonster->mObjectIdx );				
//			}
//			SAFE_DELETE( pMonster );
//		}
//		SAFE_DELETE( pGroup );
//	}
//	mpRegenGroupMap->Clear();
//
//	SAFE_DELETE( mpFamilyMap );
//	SAFE_DELETE( mpRegenSwitchMap );
//	SAFE_DELETE( mpRegenGroupMap );
//
//	mRegenSearch.Clear();
//	mObstructionMap.Clear();
//
//	mTeleportSet.Clear();
//}
//
//
//void* cThemeObject::operator new( size_t n )
//{
//	if( n != sizeof(cThemeObject) ) 
//	{
//		NETWORK2->PostServerEvent("cThemeObject::operator new %d != sizeof(cThemeObject)", n );
//		return NULL;
//	}
//
//	return THEMEMANAGER->AllocThemeObject();
//}
//
//
//void cThemeObject::operator delete( void* ptr, size_t n )
//{
//	/// NULL 포인터 검사
//	if( ptr == 0 )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::operator delete ptr == 0" );
//		return;
//	}
//
//	if( n != sizeof(cThemeObject) ) 
//	{
//		NETWORK2->PostServerEvent("cThemeObject::operator delete n != sizeof(cThemeObject", n );
//		return;
//	}
//
//	THEMEMANAGER->FreeThemeObject( static_cast<cThemeObject*>(ptr) );
//
//	return;
//}
//
//
//bool cThemeObject::InitTheme( unsigned short circusIdx, unsigned char mode )
//{
//	mThemeIdx = circusIdx;
//	mMode = mode;
//
//	mServerOnRegen = true;
//	mMemberCnt = 0;
//	memset( mMember, 0, sizeof(mMember) );
//
//	mpRegenGroupMap = new cPHashMap( 999 );
//	mpRegenSwitchMap = new cPHashMap( 999 );
//	mpFamilyMap = new cPHashMap( 100 );
//
//	if( AIMANAGER->InitMonsterRegen( mode, mpRegenGroupMap, mpFamilyMap, mpRegenSwitchMap ) == false )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::InitTheme AIMANAGER->InitMonsterRegen(%d) == false",
//			circusIdx );
//		return false;
//	}
//
//	/// 장애물(문) 스크립트 데이터 로드
//	cPArray* pObstruction = STAGESCRIPT->GetManagedDoorArr( NETWORK2->GetInDunMapNum() );
//	if( pObstruction != NULL )
//	{
//		for( unsigned long i = 0 ; i < pObstruction->GetSize() ; ++i )
//		{
//			sDoorInfo* pDoor = (sDoorInfo*)(*pObstruction)[i];
//			if( pDoor == NULL )
//				continue;
//
//			mObstructionMap.Insert( pDoor->mIndex, pDoor );
//		}
//	}
//
//	return true;
//}
//
//
//bool cThemeObject::ThemeAddPlayer( unsigned long playerIdx )
//{
//	unsigned long* pMember = NULL;
//	for( int i = 0 ; i < CIRCUS_MEMBER_COUNT_MAX ; ++i )
//	{
//		pMember = &mMember[i];
//		if( pMember == NULL )
//		{
//			NETWORK2->PostServerEvent("cThemeObject::ThemeAddPlayer pMember == NULL[%d,%d]", playerIdx, mThemeIdx );
//			return false;
//		}
//
//		/// 이미 등록된 내아이디가 있는지 체크
//		if( *pMember == playerIdx )
//		{
//			break;
//		}
//		else if( *pMember == 0 )
//		{
//			/// 빈공간이 있는지 체크
//			*pMember = playerIdx;
//			if( mMemberCnt <= CIRCUS_MEMBER_COUNT_MAX )
//				++mMemberCnt;
//			else
//				NETWORK2->PostServerEvent("cThemeObject::ThemeAddPlayer mMemberCnt[%d,%d,%d] >= CIRCUS_MEMBER_COUNT_MAX", 
//				playerIdx, mThemeIdx, mMemberCnt );
//				
//			break;
//		}
//		else
//		{
//			pMember = NULL;
//		}
//	}
//
//	/// 빈공간도 없고 내아이디 등록된것도 없으면 실패 리턴
//	if( pMember == NULL )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::ThemeAddPlayer Full pMember == NULL[%d,%d]", playerIdx, mThemeIdx );
//		return false;
//	}
//
//	return true;
//}
//
//
//void cThemeObject::ProcessTheme( unsigned long elapsedTime, unsigned long accumTime )
//{
//	AIMANAGER->ProcessMonRegen( mMode, mpRegenGroupMap, mpRegenSwitchMap, mpFamilyMap, &mRegenSearch, elapsedTime, accumTime, mThemeIdx, &mServerOnRegen );
//	AIMANAGER->ProcessMonPatternMove( mpFamilyMap, &mRegenSearch, elapsedTime, accumTime );
//}
//
//
//bool cThemeObject::IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos )
//{
//	cPHashMap::cIterator b = mObstructionMap.Begin();
//	cPHashMap::cIterator e = mObstructionMap.End();
//	for( ; b != e ; ++b )
//	{
//		sDoorInfo* pObstruction = (sDoorInfo*)(*b).mSecond;
//		if( pObstruction == NULL )
//			continue;
//
//		/// 
//		if( STAGESCRIPT->LineIntersectRect( pObstruction->mRectPos, startPos, goalPos ) == true )
//			return false;
//	}
//	return true;
//}
//
//
//void cThemeObject::DeleteObstruction( unsigned long obstructionIdx )
//{
//	HANDLE         handle = NULL;
//	for( int i = 0 ; i < CIRCUS_MEMBER_COUNT_MAX ; ++i )
//	{
//		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mMember[i] );
//		if( pPlayer == NULL )
//			continue;
//
//		MSG_SYN_OBSTRUCTION_DELETE* pMsg = (MSG_SYN_OBSTRUCTION_DELETE*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
//			NM_OBSTRUCTION, NM_OBSTRUCTION_DELETE_SYN );
//		if ( pMsg != NULL )
//		{
//			pMsg->mObstructionIdx = obstructionIdx;
//			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_OBSTRUCTION_DELETE) );
//		}
//
//	}
//
//	mObstructionMap.Erase( obstructionIdx );
//}
//
//
//void cThemeObject::SendObstruction( cPlayer* pPlayer )
//{
//	if( pPlayer == NULL )
//		return;
//
//	cPHashMap::cIterator b = mObstructionMap.Begin();
//	cPHashMap::cIterator e = mObstructionMap.End();
//
//	HANDLE handle = NULL;
//	MSG_SYN_OBSTRUCTION_LIST* pMsg = (MSG_SYN_OBSTRUCTION_LIST*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
//		NM_OBSTRUCTION, NM_OBSTRUCTION_LIST_SYN );
//	if ( pMsg != NULL )
//	{
//		unsigned char i = 0;
//		for( ; b != e ; ++b )
//		{
//			unsigned long idx = (*b).mFirst;
//			if( idx == 0 )
//				continue;
//
//			pMsg->mObstructionIdx[i] = idx;
//			++i;
//		}
//
//		pMsg->mCount = i;
//
//		NETWORK2->SendMsgRoot( handle, pMsg->GetMsgLength() );
//	}
//}
//
//
//void cThemeObject::AddTeleportPortal( unsigned long mapChangeIdx )
//{ 
//	if( mTeleportSet.Insert( mapChangeIdx ) == false )
//		NETWORK2->PostServerEvent("cThemeObject::AddTeleportPortal %d", mapChangeIdx );
//
//	HANDLE         handle = NULL;
//	for( int i = 0 ; i < CIRCUS_MEMBER_COUNT_MAX ; ++i )
//	{
//		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mMember[i] );
//		if( pPlayer == NULL )
//			continue;
//
//		MSG_SYN_TELEPORTPORTAL_CREATE* pMsg = (MSG_SYN_TELEPORTPORTAL_CREATE*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
//			NM_TELEPORTPORTAL, NM_TELEPORTPORTAL_CREATE_SYN );
//		if ( pMsg != NULL )
//		{
//			pMsg->mMapChangePosIdx = mapChangeIdx;
//			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TELEPORTPORTAL_CREATE) );
//		}
//	}
//}
//
//int cThemeObject::UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx )
//{
//	if( pPlayer == NULL )
//		return ERROR_TELEPORTPORTAL_USE_STATE;
//
//	/// 2차상태 체크
//	if( pPlayer->IsRequestRejection() == true )
//		return ERROR_TELEPORTPORTAL_USE_STATE;
//
//	/// 맵이동 가능인지 현재 상태 체크
//	if( pPlayer->IsChangeState( eOBJECT_STATE_MOVE ) == false )
//		return ERROR_TELEPORTPORTAL_USE_STATE;
//
//	cHashSet::cIterator b = mTeleportSet.Begin();
//	cHashSet::cIterator e = mTeleportSet.End();
//	if( b == e )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal b == e %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
//		return ERROR_TELEPORTPORTAL_USE_PORTAL;
//	}
//	
//	sStageChangePos* scriptPos = STAGESCRIPT->GetPosScriptInfo( mapChangeIdx );
//	if( scriptPos == NULL )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
//		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
//	}
//
//	/// 이동 좌표 오류
//	if( scriptPos->mMapNumber == 0 )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal scriptPos == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
//		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
//	}
//
//	if( scriptPos->mMapNumber != NETWORK2->GetInDunMapNum() )
//		return ERROR_TELEPORTPORTAL_USE_MAP;
//
//	NiPoint2 destPos = pPlayer->GetPos();
//	float objectSize = OBJECTMANAGER->ObjectSizeRange( pPlayer, NULL, 0 );
//	NiPoint2 portalPos;
//	portalPos.x = scriptPos->mPosX;
//	portalPos.y = scriptPos->mPosY;
//	
//	mRangeCheck.SetRadius( scriptPos->mRange + objectSize + SYNC_MOVE_RANGE );
//	if( mRangeCheck.IsNotRange( portalPos, pPlayer->GetPos() ) == true )
//		return ERROR_TELEPORTPORTAL_USE_RANGE;
//
//	sStageChangePos* pTargetInfo = scriptPos->mTargetInfo;
//	if( pTargetInfo == NULL )
//	{
//		NETWORK2->PostServerEvent("cThemeObject::UseTeleportPortal pTargetInfo == NULL %d, %d", pPlayer->GetObjectID(), mapChangeIdx );
//		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
//	}
//
//	bool isPos = false;
//	/// 이동좌표가 못가는 지역일 경우 범위를 쫍혀가면서 이동위치를 잡는다.
//	if( pTargetInfo->mRange > MAP_RANGE_CHECK_RING )
//	{
//		for( int i = MAP_RANGE_CHECK_RING ; i > 0 ; --i )
//		{
//			/// 최대 범위에서 20%씩 줄여가면서 체크
//			int range = (int)(pTargetInfo->mRange * i * ( 1.0f / MAP_RANGE_CHECK_RING ));
//			if( range <= 0 ) 
//				continue;
//
//			destPos.x = pTargetInfo->mPosX + ( rand() % range ) - ( rand() % range );
//			destPos.y = pTargetInfo->mPosY + ( rand() % range ) - ( rand() % range );
//
//			if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), destPos.x, destPos.y, pPlayer->GetObject() ) == false )
//				continue;
//
//			isPos = true;
//			break;
//		}
//	}
//
//	if( isPos == false )
//	{
//		/// 위에 랜덤 범위 좌표중 모두다 못가는 경우 스크립트상 정해진 고정위치로 이동시킨다.
//		destPos.x = pTargetInfo->mPosX;
//		destPos.y = pTargetInfo->mPosY;
//	}
//
//	if( AIMANAGER->IsPossible( pPlayer->GetMapNumber(), destPos.x, destPos.y, pPlayer->GetObject() ) == false )
//		return ERROR_TELEPORTPORTAL_USE_SCRIPT;
//
//	pPlayer->SetPos( destPos.x, destPos.y );
//	pPlayer->MoveStop();
//
//	return ERROR_RES_MAPCHANGE_SUCCESS;
//}
//
//void cThemeObject::SendTeleportPortal( cPlayer* pPlayer )
//{
//	if( pPlayer == NULL )
//		return;
//
//	cHashSet::cIterator b = mTeleportSet.Begin();
//	cHashSet::cIterator e = mTeleportSet.End();
//
//	HANDLE handle = NULL;
//	MSG_SYN_TELEPORTPORTAL_LIST* pMsg = (MSG_SYN_TELEPORTPORTAL_LIST*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx(), 
//		NM_TELEPORTPORTAL, NM_TELEPORTPORTAL_LIST_SYN );
//	if ( pMsg != NULL )
//	{
//		unsigned char i = 0;
//		for( ; b != e ; ++b )
//		{
//			unsigned long idx = *b;
//			if( idx == 0 )
//				continue;
//
//			pMsg->mMapChangePosIdx[i] = idx;
//			++i;
//		}
//
//		pMsg->mCount = i;
//
//		NETWORK2->SendMsgRoot( handle, pMsg->GetMsgLength() );
//	}
//}
//
//
//void cThemeObject::ThemeMemberOut( unsigned long playerIdx )
//{
//	if( mMemberCnt != 0 )
//	{
//		for( unsigned long i = 0 ; mMemberCnt > i ; ++i )
//		{
//			if( mMember[i] == 0 )
//				return;
//
//			if( mMember[i] != playerIdx )
//				continue;
//
//			/// 마지막위치 유저를 나가는맴버 자리로 이동
//			mMember[i] = mMember[mMemberCnt-1];
//
//			/// 마지막에 있던 유저 값을 제거
//			mMember[mMemberCnt-1] = 0;
//			--mMemberCnt;
//
//			break;
//		}
//	}
//	else
//	{
//		NETWORK2->PostServerEvent("cThemeObject::ThemeMemberOut mMemberCnt != 0");
//	}
//
//	if( mMemberCnt == 0 && mDependency == false )
//	{
//		THEMEMANAGER->InsertDelTheme( mThemeIdx );
//	}
//}
//
//
//sStageChangePos* cThemeObject::ThemeOutPos()
//{
//	cPArray* pAry = STAGESCRIPT->GetMapChangeStarterArr( NETWORK2->GetInDunMapNum() );
//	if( pAry == NULL )
//		return NULL;
//
//	if( pAry->GetSize() == 0 )
//		return NULL;
//
//	return (sStageChangePos*)(*pAry)[0];
//	
//}
//
//
//void cThemeObject::ThemeAllOut()
//{
//	for( unsigned long i = 0 ; mMemberCnt > i ; ++i )
//	{
//		if( mMember[i] == 0 )
//			continue;
//
//		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( mMember[i] );
//		if( pPlayer == NULL )
//			continue;
//
//		/// 인던 종료 플래그 설정
//		pPlayer->SetInDunJoin( false );
//		pPlayer->SetThemeRoomIdx( 0 );
//
//		/// 게임서버로 이동하라고 알림
//		NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx(), NM_THEME, NM_THEME_END_SYN );
//	}
//}
