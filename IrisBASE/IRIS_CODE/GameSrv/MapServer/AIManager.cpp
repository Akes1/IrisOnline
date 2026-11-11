#include "StdAfx.h"
#include "AiManager.h"

#include "Monster_Common.h"

#include "GameSrv.h"

#include "NaviField.h"
#include "Pathfinder.h"
#include "ObjectManager.h"
#include "MonsterScript.h"
#include "Monster.h"
#include "Gathering.h"
#include "StageScript.h"
#include "GatheringScript.h"
#include "GridManager.h"
#include "ThemeObject.h"
#include "ThemeManager.h"
#include "PVPManager.h"
#include "DeathMatchObject.h"


cAIManager* cAIManager::mpAIManager = NULL;


cAIManager::cAIManager()
{
	if( !mpAIManager )
	{
		mpAIManager = this;
	}

	mpRegenGroupMap = new cPointHashMap( 999 );
	mpRegenGatherGroupMap = new cPointHashMap( 999 );
	mpRegenSwitchMap = new cPointHashMap( 999 );
	mpFamilyMap = new cPointHashMap( 100 );
}

cAIManager::~cAIManager()
{
	cPointHashMap::cIterator iter;

	for( iter = mMapPathMap.Begin() ; iter != mMapPathMap.End() ; ++iter )
	{
		sMapPathInfo* pPath = (sMapPathInfo*)(*iter).mSecond;
		if( pPath != NULL )
		{
			SAFE_DELETE( pPath->mNaviField );
			SAFE_DELETE( pPath->mPathFinder );
		}
		SAFE_DELETE( pPath );
	}
	mMapPathMap.Clear();	

	SAFE_DELETE( mpFamilyMap );
	SAFE_DELETE( mpRegenSwitchMap );
	SAFE_DELETE( mpRegenGroupMap );
	SAFE_DELETE( mpRegenGatherGroupMap );
}

bool cAIManager::Init( int serverType )
{
	unsigned short mapType = MAP_NONE;
	switch( serverType )
	{
	case _E_ST_ID_PVP_:
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		mapType = NETWORK2->GetInDunMapNum();
		break;
	case _E_ST_NORMAL_MAP_:
		mapType = MAP_NONE;
		break;
	}

	if( mapType == MAP_NONE )
	{
		for( unsigned short mapNum = MAP_MIN ; mapNum <= MAP_MAX ; ++mapNum )
		{
			if( LoadMapPathData( mapNum, DEFAULT_MAPMODE ) == false )
				return false;
		}
	}
	else
	{
		/// 한서버에 맵데이터는 모두 동일하므로 한개만 로드하면 된다.
		for( unsigned char i = eINDUNMODE_MIN ; i <= eINDUNMODE_MAX ; ++i )
		{
			sMapInfo* pMapInfo = STAGESCRIPT->GetMapFolderIdx( mapType, i );				
			if( pMapInfo == NULL )
				continue;

			if( LoadMapPathData( mapType, i ) == false )
				return false;

			break;
		}
	}

	mNextCheckTime = 0;
	mServerType = serverType;
	mServerOnRegen = true;

	/// 일반 서버인경우 리젠 정보 설정
	/// ※ 테마 서버 일경우 리젠 정보는 인던방 자체가 목록을 가지고 관리한다.
	if( serverType == _E_ST_NORMAL_MAP_ )
	{
		unsigned char levelMode = 0;

		cPointHashMap* pRegenGroupMap = new cPointHashMap;
		cPointHashMap* pFamilyMap = new cPointHashMap;
		cPointHashMap* pRegenSwitchMap = new cPointHashMap;
			
		InitMonsterRegen( levelMode, pRegenGroupMap, pFamilyMap, pRegenSwitchMap );

		if( mpRegenGroupMap->Insert( levelMode, pRegenGroupMap ) == false )
			return false;
		if( mpFamilyMap->Insert( levelMode, pFamilyMap ) == false )
			return false;
		if( mpRegenSwitchMap->Insert( levelMode, pRegenSwitchMap ) == false )
			return false;

		//////////////////////////////////////////////////////////////////////////
		/// 스크립트로 로드된 채집, 리젠위치 그룹 정보를 보관
		/// 목록을 보유한 상태로 objectmanager의 채집 객체만 지웠다 추가했다가 한다.
		cPointHashMap* pRegenGatheringMap = GATHERINGSCRIPT->GetRegenGatheringMap( 0 );
		if( pRegenGatheringMap == NULL )
		{
			NETWORK2->PostServerEvent( "pRegenGatheringMap == NULL" );
			return false;
		}

		cPointHashMap* pGatherGroupMap = new cPointHashMap;

		sRegenGather* pRegen = 0;
		cPointHashMap::cIterator iter;
		for( iter = pRegenGatheringMap->Begin() ; iter != pRegenGatheringMap->End() ; ++iter )
		{
			/// 그룹 정보 추가
			pRegen = new sRegenGather;		/// 서버 생성 시점에만 생성(new)함
			pRegen->mRegenIdx = (*iter).mFirst;
			sGatheringRegenHeader* pHeader = (sGatheringRegenHeader*)(*iter).mSecond;
			if( pHeader == NULL )
				continue;

			unsigned long randTime = pHeader->mRandTime;
			if( randTime != 0 )
				randTime = rand() % pHeader->mRandTime;
			pRegen->mRegenTime = NETWORK2->GetAccumTime() + pHeader->mRegenWaitTime + randTime;
			pRegen->mRegenReady = true;

			if( pGatherGroupMap->Insert( pRegen->mRegenIdx, pRegen ) == false )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("mpRegenGatherGroupMap Insert duplicate[%d]", pRegen->mRegenIdx );
				return false;
			}
		}

		if( mpRegenGatherGroupMap->Insert( levelMode, pGatherGroupMap ) == false )
			return false;
	}

	return true;
}


bool cAIManager::LoadMapPathData( unsigned short mapNum, unsigned char modeNum )
{
	sMapInfo* pMapInfo = STAGESCRIPT->GetMapFolderIdx( mapNum, modeNum );				
	if( pMapInfo == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::LoadMapPathData pMapInfo == NULL [%d,%d]", mapNum, modeNum );
		return true;
	}

	unsigned short folderIdx = (unsigned short)pMapInfo->folderIdx;
	if( folderIdx == 0 )
	{
		NETWORK2->PostServerEvent("cAIManager::LoadMapPathData folderIdx == 0 [%d,%d]", mapNum, modeNum );
		return false;
	}

	sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( folderIdx );
	if( pPathInfo != NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::LoadMapPathData pPathInfo != NULL [%d,%d,%d]", mapNum, modeNum, folderIdx );
		return false;
	}

	pPathInfo = new sMapPathInfo;
	mMapPathMap.Insert( folderIdx, pPathInfo );			

	/// 네비필드를 생성
	cNaviField* naviField = pPathInfo->mNaviField = new cNaviField;
	/// 길찾기를 생성
	cPathFinder* pathFinder = pPathInfo->mPathFinder = new cPathFinder( 4096 );

	cString fileName;
	fileName.Format( "./Script/Resource/Map/%03d.navifield", folderIdx );

	if( naviField->Load( fileName.Cstr() ) == false )
	{
		NETWORK2->PostServerEvent("cAIManager::LoadMapPathData naviField->Load == false [%d,%d,%d]", mapNum, modeNum, folderIdx );
		return false;
	}

	pathFinder->Init( naviField );

	/// x좌표 타일갯수, y좌표 타일갯수, 타일1칸사이즈							
	pPathInfo->mMapSize = naviField->GetCellCount() * 100.0f;

	return true;
}


bool cAIManager::InitMonsterRegen( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, 
								  tPointerHashMap<unsigned long, void*>* pFamilyMap,
								  tPointerHashMap<unsigned long, void*>* pRegenSwitchMap )
{
	cPointHashMap::cIterator iter;

	if( pRegenGroupMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InitMonsterRegen pRegenGroupMap == NULL" );
		return false;
	}
	if( pFamilyMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InitMonsterRegen pFamilyMap == NULL" );
		return false;
	}
	if( pRegenSwitchMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InitMonsterRegen pRegenSwitchMap == NULL" );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 스크립트로 로드된 몬스터, 몬스터 그룹 정보를 보관
	/// 목록을 보유한 상태로 objectmanager의 몬스터 객체만 지웠다 추가했다가 한다.
	cPointHashMap* pRegenGroupMapScript = MONSTERSCRIPT->GetRegenGroupMap( levelMode );
	if( pRegenGroupMapScript == NULL )
		return true;

	sRegenGroup* pRegenGroup = NULL;
	for( iter = pRegenGroupMapScript->Begin() ; iter != pRegenGroupMapScript->End() ; ++iter )
	{
		/// 그룹 정보 추가
		pRegenGroup = new sRegenGroup;		/// 서버 생성 시점에만 생성(new)함
		pRegenGroup->mpRegenGroup = (sRegenGroupScript*)(*iter).mSecond;
		pRegenGroup->mGroupIdx = (*iter).mFirst;
		pRegenGroup->mIsRegen = false;
		pRegenGroup->mRegenTime = 0;
		pRegenGroup->mMonsterAliveCnt = 0;

		/// 자신의 리젠그룹번호가 설정되있거나 서버를 키자마자 리젠하는 그룹이면 리젠프로세스 목록에 넣어줌
		pRegenGroup->mUse = false;
		if( pRegenGroup->mpRegenGroup->mServerOnRegen == true )
			pRegenGroup->mUse = true;
		else
		{
			tArray<unsigned long>* pAry = &(pRegenGroup->mpRegenGroup->mRegenGroupAry);
			if( pAry == NULL )
				continue;

			unsigned int count = pAry->GetSize();
			for( unsigned int i=0; i<count; ++i )
			{
				if( (*pAry)[i] == pRegenGroup->mpRegenGroup->mGroupIdx )
				{
					pRegenGroup->mUse = true;
					break;
				}
			}
		}

		if( pRegenGroupMap->Insert( pRegenGroup->mpRegenGroup->mGroupIdx, pRegenGroup ) == false )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("mpRegenGroupMap Insert duplicate[%d]", pRegenGroup->mpRegenGroup->mGroupIdx );
			return false;
		}

		/// 그룹 안에 몬스터 목록도 넣는다.
		for( unsigned long i = 0 ; i < pRegenGroup->mpRegenGroup->mMonsterAry.GetSize() ; ++i )
		{
			sRegenMonster* pRM = new sRegenMonster;

			/// 몬스터 객체 regen 하지 않은 상태로 플래그 설정
			pRM->mObjectIdx = 0;
			pRM->mRegenTime = 0;
			pRM->mDie = true;
			pRM->mRegenIdx = pRegenGroup->mpRegenGroup->mMonsterAry[i];

			pRegenGroup->mMonsterList.PushBack( pRM );

			sRegenScript* pRegenScript = MONSTERSCRIPT->GetRegenMonsterInfo( levelMode, pRM->mRegenIdx );
			if( pRegenScript == NULL )
				continue;

			pRM->mRegenTime = pRegenScript->mRegenTime;

			/// 동족인식 목록 작성
			if( pRegenScript->mFamilyNum != 0 )
			{
				cPointHashMap::cIterator fI = pFamilyMap->Find( pRegenScript->mFamilyNum );
				cPointHashMap::cIterator eI = pFamilyMap->End();

				cHashSet* pFamily;
				if( fI == eI )
				{
					pFamily = new cHashSet;
					pFamilyMap->Insert( pRegenScript->mFamilyNum, pFamily );
				}
				else
				{
					pFamily = (cHashSet*)(*fI).mSecond;
				}
				pFamily->Insert( pRM->mRegenIdx );

				/// 동일 행동 그룹
				sMonsterPattern* pPattern = MONSTERSCRIPT->GetMonsterPattern( levelMode, pRegenScript->mPatternIdx );
				if( pPattern == NULL )
					continue;
			}
		}
	}

	/// 스위치 형태로 리젠하는 리젠그룹들 정보 셋팅
	cPointHashMap* pMap = MONSTERSCRIPT->GetSwitchGroup( levelMode );
	if( pMap != NULL )
	{
		cPointHashMap::cIterator begin = pMap->Begin();
		cPointHashMap::cIterator end = pMap->End();

		for( ; begin != end ; ++begin )
		{
			/// 스크립트에 기록된 스위치 그룹을 ai에 등록
			unsigned long swichGroupIdx = (*begin).mFirst;
			cArray* pAry = (cArray*)(*begin).mSecond;

			/// 스크립트 정보 ai에 기록
			sSwichGroup* pSwitch = new sSwichGroup;
			pSwitch->mSwitchGroup = swichGroupIdx;
			pSwitch->mpGroupIdxAry = pAry;
			pSwitch->mGroupIdx = 0;
			pSwitch->mRegenTime = 0;
			pSwitch->mRegenStop = false;
			pSwitch->mRegenSyncWait = false;
			pSwitch->mCheckTime = 0;

			/// [리젠그룹] 랜덤 선택
			if( pAry != NULL && pAry->GetSize() != 0 )
			{
				unsigned long randSelect = rand() % pAry->GetSize();
				unsigned long selectGroupIdx = (*pAry)[randSelect];

				sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( selectGroupIdx );
				if( pRegenGroup == NULL )
					continue;

				/// 선택된 [리젠그룹]의 정보를 기본정보로 셋팅
				pSwitch->mGroupIdx = selectGroupIdx;
				pSwitch->mRegenTime = pRegenGroup->mpRegenGroup->mRegenTime + ( rand() % SWITCH_RANDTIME ) + NETWORK2->GetAccumTime();
			}			

			/// 정보 기록
			if( pRegenSwitchMap->Insert( pSwitch->mSwitchGroup, pSwitch ) == false )
			{
				NETWORK2->PostServerEvent("AIManager::Init mpRegenSwitchMap->Insert( %d ) == false", pSwitch->mSwitchGroup );
				return false;
			}
		}
	}

	return true;
}

void cAIManager::Release()
{
	//SAFE_DELETE( mPathFinder );
	cPointHashMap::cIterator iter;

	for( iter = mpFamilyMap->Begin() ; iter != mpFamilyMap->End() ; ++iter )
	{
		cPointHashMap* p = (cPointHashMap*)(*iter).mSecond;
		if( p != NULL )
		{
			cPointHashMap::cIterator iter2;
			for( iter2 = p->Begin() ; iter2 != p->End() ; ++iter2 )
			{
				cHashSet* pSet = (cHashSet*)(*iter2).mSecond;
				SAFE_DELETE( pSet );
			}
			p->Clear();
			SAFE_DELETE( p );
		}
	}
	mpFamilyMap->Clear();
	SAFE_DELETE( mpFamilyMap );


	for( iter = mpRegenSwitchMap->Begin() ; iter != mpRegenSwitchMap->End() ; ++iter )
	{
		cPointHashMap* p = (cPointHashMap*)(*iter).mSecond;
		if( p != NULL )
		{
			cPointHashMap::cIterator iter2;
			for( iter2 = p->Begin() ; iter2 != p->End() ; ++iter2 )
			{
				sSwichGroup* pSwitch = (sSwichGroup*)(*iter2).mSecond;
				SAFE_DELETE( pSwitch );
			}
			p->Clear();
			SAFE_DELETE( p );
		}
	}
	mpRegenSwitchMap->Clear();
	SAFE_DELETE( mpRegenSwitchMap );


	for( iter = mpRegenGatherGroupMap->Begin() ; iter != mpRegenGatherGroupMap->End() ; ++iter )
	{
		cPointHashMap* p = (cPointHashMap*)(*iter).mSecond;
		if( p != NULL )
		{
			cPointHashMap::cIterator iter2;
			for( iter2 = p->Begin() ; iter2 != p->End() ; ++iter2 )
			{
				sRegenGather* pGroup = (sRegenGather*)(*iter2).mSecond;
				SAFE_DELETE( pGroup );
			}
			p->Clear();
			SAFE_DELETE( p );
		}
	}
	mpRegenGatherGroupMap->Clear();
	SAFE_DELETE( mpRegenGatherGroupMap );


	for( iter = mpRegenGroupMap->Begin() ; iter != mpRegenGroupMap->End() ; ++iter )
	{
		cPointHashMap* p = (cPointHashMap*)(*iter).mSecond;
		if( p != NULL )
		{
			cPointHashMap::cIterator iter2;
			for( iter2 = p->Begin() ; iter2 != p->End() ; ++iter2 )
			{
				sRegenGroup* pGroup = (sRegenGroup*)(*iter2).mSecond;

				for( unsigned long i = 0 ; i < pGroup->mMonsterList.GetSize() ; ++i )
				{
					sRegenMonster* pMonster = (sRegenMonster*)pGroup->mMonsterList[i];
					SAFE_DELETE( pMonster );
				}
				SAFE_DELETE( pGroup );
			}
			p->Clear();
			SAFE_DELETE( p );
		}
	}
	mpRegenGroupMap->Clear();
	SAFE_DELETE( mpRegenGroupMap );
}



void cAIManager::Process( unsigned long elapsedTime, unsigned long accumTime )
{

	if( mNextCheckTime >= accumTime )
		return;

	mNextCheckTime = AI_REGEN_PROCESS_DELAY + accumTime;

	/// 몬스터
	unsigned char levelMode = 0;
	cPointHashMap* pRegenGroupMap = (cPointHashMap*)mpRegenGroupMap->GetAt( levelMode );
	cPointHashMap* pRegenSwitchMap = (cPointHashMap*)mpRegenSwitchMap->GetAt( levelMode );
	cPointHashMap* pFamilyMap = (cPointHashMap*)mpFamilyMap->GetAt( levelMode );

	if( pRegenGroupMap != NULL )
	{
		ProcessMonRegen( levelMode, pRegenGroupMap, pRegenSwitchMap, pFamilyMap, &mRegenSearch, elapsedTime, accumTime, 0, &mServerOnRegen );
		ProcessMonPatternMove( pFamilyMap, &mRegenSearch, elapsedTime, accumTime );
	}


	cPointHashMap* pRegenGatherGroupMap = (cPointHashMap*)mpRegenGatherGroupMap->GetAt( levelMode );

	if( pRegenGatherGroupMap != NULL )
	{
		/// 채집
		ProcessGatRegen( levelMode, elapsedTime, accumTime );
	}
}


void cAIManager::ProcessMonRegen( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, tPointerHashMap<unsigned long, void*>* pRegenSwitchMap,
								 tPointerHashMap<unsigned long, void*>* pFamilyMap, tHashMap<unsigned long, unsigned long>* pRegenSearch,
								 unsigned long /*elapsedTime*/, unsigned long accumTime, unsigned short mapNumber, bool* pServerOnRegen )
{
	if( pRegenGroupMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonRegen pRegenGroupMap == NULL");
		return;
	}
	if( pRegenSwitchMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonRegen pRegenSwitchMap == NULL");
		return;
	}
	if( pFamilyMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonRegen pFamilyMap == NULL");
		return;
	}
	if( pRegenSearch == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonRegen pRegenSearch == NULL");
		return;
	}

	/// 서버 켜지는 시점에 몬스터 리젠 체크
	bool serverOnRegen = false;
	if( pServerOnRegen != NULL )
		serverOnRegen = *pServerOnRegen;

	cPointHashMap::cIterator iter;
	for( iter = pRegenGroupMap->Begin() ; iter != pRegenGroupMap->End() ; ++iter )
	{
		unsigned long regenGroupIdx = (*iter).mFirst;
		sRegenGroup* pRegenGroup = (sRegenGroup*)(*iter).mSecond;
		if( !pRegenGroup )
		{
			NETWORK2->PostServerEvent("cAIManager::pGroupList == NULL" );
			continue;
		}

		/// 리젠 대상이 아니면 통과
		if( pRegenGroup->mUse == false )
			continue;

		/// 선택 리젠 통과
		if( pRegenGroup->mpRegenGroup->mMapGroupIdx != 0 )
			continue;

		/// 리젠 그룹의 몬스터 목록 확인
		if( pRegenGroup->mMonsterList.GetSize() == 0 )
			continue;

		/// 스크립트 정보 확인
		sRegenGroupScript* pRegenGroupScript = pRegenGroup->mpRegenGroup;
		if( pRegenGroupScript == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::pRegenGroupScript == NULL" );
			continue;
		}

		/// 개별 몬스터 리젠
		for( unsigned long i = 0 ; pRegenGroup->mMonsterList.GetSize() > i ; ++i )
		{
			sRegenMonster* pRM = (sRegenMonster*)pRegenGroup->mMonsterList[i];
			if( pRM == NULL )
				continue;

			if( pRM->mDie == true && pRM->mObjectIdx == 0 && pRM->mRegenTime != 0 && pRM->mRegenTime < accumTime )
				RegenMonster( levelMode, pRegenGroupMap, pFamilyMap, pRegenSearch, pRegenGroup->mGroupIdx, pRM, mapNumber );
		}

		/// 0 나누기 예외처리
		if( pRegenGroupScript->mMonsterAry.GetSize() == 0 )
		{
			/// 리젠을 안하고 리젠 함으로 설정
			assert(NULL);
			pRegenGroup->mIsRegen = true;
			pRegenGroup->mRegenTime = accumTime + pRegenGroupScript->mRegenTime;
			continue;
		}

		/// 그룹 리젠 조건체크
		///	살아있는수					* 100	/ 전체 마리수								> 리젠 퍼센트 
		if( pRegenGroup->mMonsterAliveCnt * 100 / pRegenGroupScript->mMonsterAry.GetSize() > pRegenGroupScript->mRegenPer ) 
		{
			/// 리젠안함으로 설정
			pRegenGroup->mIsRegen = false; 
			continue;
		}

		/// 조건을 만족하는데 리젠 안함으로 된경우 - 처음으로 들어온경우
		if( pRegenGroup->mIsRegen == false )
		{
			/// 리젠 함으로 바꾸고
			pRegenGroup->mIsRegen = true;
			/// 리젠 수행 시간 설정
			pRegenGroup->mRegenTime = accumTime + pRegenGroupScript->mRegenTime;
		}

		/// 서버가 켜지자마자 리젠되는 몬스터들 체크
		if( serverOnRegen == false || pRegenGroupScript->mServerOnRegen == false )
		{
			/// 리젠 시간 확인
			if( pRegenGroup->mRegenTime > accumTime )
				continue;
		}

		/// isRegen 초기화
		pRegenGroup->mIsRegen = false;

		/// 모든 조건을 만족하는 경우 리젠
		cVoidAry* pMonsterAry = &pRegenGroup->mMonsterList;
		if( pMonsterAry == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::pMonsterAry == NULL mRegenGroupIdx[%d]", regenGroupIdx );
			continue;
		}

		/// 그룹에 해당하는 모든 몬스터에게 개별 리젠
		for( unsigned int i = 0 ; pMonsterAry->GetSize() > i ; ++i )
		{
			sRegenMonster* pRM = (sRegenMonster*)(*pMonsterAry)[i];
			if( pRM != NULL && pRM->mDie == true && pRM->mObjectIdx == 0 )
				RegenMonster( levelMode, pRegenGroupMap, pFamilyMap, pRegenSearch, pRegenGroup->mGroupIdx, pRM, mapNumber );
		}
	}

	/// 스위치 그룹 처리
	cPointHashMap::cIterator begin = pRegenSwitchMap->Begin();
	cPointHashMap::cIterator end = pRegenSwitchMap->End();
	for( ; begin != end ; ++begin )
	{
		unsigned long switchGroupIdx = (*begin).mFirst;
		sSwichGroup* pSwitch = (sSwichGroup*)(*begin).mSecond;
		if( pSwitch != NULL )
		{
			/// 현재 리젠중인 [리젠그룹] 끝 체크
			if( pSwitch->mGroupIdx != 0 )
			{
				sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( pSwitch->mGroupIdx );
				if( pRegenGroup == NULL )
					continue;

				if( pRegenGroup->mpRegenGroup->mMonsterAry.GetSize() == 0 )
				{
					assert(NULL);
					continue;
				}

				/// 새로 리젠을 해야하는지 현재 [리젠그룹] 리젠 조건 확인
				if( pRegenGroup->mMonsterAliveCnt * 100 / pRegenGroup->mpRegenGroup->mMonsterAry.GetSize() > pRegenGroup->mpRegenGroup->mRegenPer ) 
				{
					if( pSwitch->mCheckTime < accumTime )
					{
						pSwitch->mCheckTime = (unsigned long)(accumTime + pRegenGroup->mpRegenGroup->mRegenTime * SWITCH_SYNCTIME_RATE);
						/// 같은 서버군에 같은 스위치그룹군에 리젠설정을 다시해줌
						if( pRegenGroup->mpRegenGroup->mChannelCheck == true )
							NETWORK2->PostMonRegenSync( levelMode, switchGroupIdx );
					}

					continue;
				}

				/// 다음 리젠 시간 설정
				pSwitch->mGroupIdx = 0;
				/// 메세지 발송 시간차를 고려해 랜덤시간을 준다.
				pSwitch->mRegenTime = pRegenGroup->mpRegenGroup->mRegenTime + ( rand() % SWITCH_RANDTIME ) + accumTime;
				pSwitch->mRegenStop = false;
				///// 같은 서버군에 같은 스위치그룹군에 리젠설정을 다시해줌
				if( pRegenGroup->mpRegenGroup->mChannelCheck == true )
					NETWORK2->PostMonRegenSync( levelMode, switchGroupIdx );
			}
			else	/// [리젠그룹]이 선택 처리
			{
				/// 리젠 가능시간 체크 && 다른곳에서 리젠된것 있으면 안됨
				if( pSwitch->mRegenTime < accumTime && pSwitch->mRegenStop == false )
				{
					/// 스위치 그룹의 [리젠그룹]목록 로드
					cArray* pAry = (cArray*)pSwitch->mpGroupIdxAry;
					if( pAry != NULL && pAry->GetSize() > 0 )
					{
						/// [리젠그룹] 임의 선택
						unsigned long selectRand = rand() % pAry->GetSize();
						unsigned long groupIdx = (*pAry)[selectRand];

						/// 선택된 [리젠그룹] 데이타 로드
						sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( groupIdx );
						if( pRegenGroup == NULL )
							continue;

						if( pSwitch->mRegenSyncWait == false )
						{
							pSwitch->mRegenSyncWait = true;
							/// 같은 서버군에 같은 스위치그룹군에 리젠을 하지 말라고알림
							if( pRegenGroup->mpRegenGroup->mChannelCheck == true )
								NETWORK2->PostMonRegenSync( levelMode, switchGroupIdx );
						}

						if( pRegenGroup->mpRegenGroup->mChannelCheck == true )
						{
							if( pSwitch->mRegenTime + SWITCH_CHANNEL_CHECKTIME >= accumTime )
								continue;
						}

						/// [리젠그룹]의 몬스터 정보 로드
						cVoidAry* pMonsterAry = &pRegenGroup->mMonsterList;
						if( pMonsterAry == NULL )
							continue;

						/// 그룹에 해당하는 모든 몬스터에게 개별 리젠
						for( unsigned int i = 0 ; pMonsterAry->GetSize() > i ; ++i )
						{
							/// 몬스터 개별 리젠
							sRegenMonster* pRM = (sRegenMonster*)(*pMonsterAry)[i];
							if( pRM != NULL && pRM->mDie == true && pRM->mObjectIdx == 0 )
								RegenMonster( levelMode, pRegenGroupMap, pFamilyMap, pRegenSearch, groupIdx, pRM, mapNumber );
						}

						/// 선택한 [리젠그룹] 기록
						pSwitch->mGroupIdx = groupIdx;
						pSwitch->mRegenStop = true;
						/// 같은 서버군에 같은 스위치그룹군에 리젠을 하지 말라고알림
						if( pRegenGroup->mpRegenGroup->mChannelCheck == true )
							NETWORK2->PostMonRegenSync( levelMode, switchGroupIdx );
					}
				}
			}
		}
	}

	*pServerOnRegen = false;
}


void cAIManager::ProcessMonPatternMove( tPointerHashMap<unsigned long, void*>* pFamilyMap, tHashMap<unsigned long, unsigned long>* pRegenSearch,
									   unsigned long /*elapsedTime*/, unsigned long /*accumTime*/ )
{
	if( pFamilyMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonPatternMove pFamilyMap == NULL" );
		return;
	}
	if( pRegenSearch == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::ProcessMonPatternMove pRegenSearch == NULL" );
		return;
	}

	cPointHashMap::cIterator i = pFamilyMap->Begin();
	cPointHashMap::cIterator e = pFamilyMap->End();

	for( ; i != e ; ++i )
	{
		cHashSet* pSet = (cHashSet*)(*i).mSecond;
		if( pSet == NULL )
			continue;

		cHashSet::cIterator sI = pSet->Begin();
		cHashSet::cIterator sE = pSet->End();

		bool allReady = true;
		for( ; sI != sE ; ++sI )
		{
			unsigned long monsterRegenIdx = *sI;
			unsigned long monsterIdx = 0;
			pRegenSearch->GetAt( &monsterIdx, monsterRegenIdx );

			cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
			if( pMonster == NULL )
				continue;

			if( pMonster->GetMovePattern() == NULL )
				continue;

			if( pMonster->GetGroupMoveReady() != 2 )
				allReady = false;

			break;
		}

		if( allReady == false )
			continue;

		for( sI = pSet->Begin() ; sI != sE ; ++sI )
		{
			unsigned long monsterRegenIdx = *sI;
			unsigned long monsterIdx = 0;
			pRegenSearch->GetAt( &monsterIdx, monsterRegenIdx );

			cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
			if( pMonster == NULL )
				continue;

			if( pMonster->GetMovePattern() == NULL )
				continue;

			pMonster->SetGroupMoveReady( 0 );
		}
	}
}


void cAIManager::RegenMonster( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, tPointerHashMap<unsigned long, void*>* pFamilyMap,
							  tHashMap<unsigned long, unsigned long>* pRegenSearch, unsigned long groupIdx, sRegenMonster* pRM, unsigned short mapNumber )
{
	if( pRegenGroupMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::RegenMonster pRegenGroupMap == NULL");
		return;
	}
	if( pFamilyMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::RegenMonster pFamilyMap == NULL");
		return;
	}
	if( pRegenSearch == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::RegenMonster pRegenSearch == NULL");
		return;
	}

	if( pRM == NULL )
		return;

	unsigned long objectIdx = mMonsterIdxGen.GeneratIdx();
	if( objectIdx == 0 )
	{
		NETWORK2->PostServerEvent("mMonsterIdxGen == 0");
		return;
	}

	/// 몬스터가 만들어져 있을수가 없다.
	if( OBJECTMANAGER->GetMonster( objectIdx ) )
	{
		NETWORK2->PostServerEvent("cAIManager::Process monsterIdx[%d] duplicate", objectIdx );
		return;
	}

	sRegenScript* pRegen = MONSTERSCRIPT->GetRegenMonsterInfo( levelMode, pRM->mRegenIdx );
	if( pRegen == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::Process pRegen[%d,%d] == NULL", objectIdx, pRM->mRegenIdx );
		return;
	}

//^^
	/*
	/// 행동그룹 확인
	sMonsterPattern* pPattern = MONSTERSCRIPT->GetMonsterPattern( pRegen->mPatternIdx );
	if( pPattern != NULL && pPattern->mGroupIdx != 0 )
	{
		cHashSet* pSet = (cHashSet*)mpMonActionGroup->GetAt( pPattern->mGroupIdx );
		if( pSet != NULL )
		{
			cHashSet::cIterator sI = pSet->Begin();
			cHashSet::cIterator sE = pSet->End();

			for( ; sI != sE ; ++sI )
			{
				unsigned long regenIdx = *sI;
				unsigned long monsterIdx = 0;
				mRegenSearch.GetAt( &monsterIdx, regenIdx );
				cMonster* pGroupMon = OBJECTMANAGER->GetMonster( monsterIdx );
				if( pGroupMon == NULL )
					continue;

				if( pGroupMon->GetGroupMoveReady() == 0 )
					return;
			}
		}
	}
	*/

	/// 행동그룹 확인
	sMonsterPattern* pPattern = MONSTERSCRIPT->GetMonsterPattern( levelMode, pRegen->mPatternIdx );
	if( pPattern != NULL )
	{
		cHashSet* pSet = (cHashSet*)pFamilyMap->GetAt( pRegen->mFamilyNum );
		if( pSet != NULL )
		{
			cHashSet::cIterator sI = pSet->Begin();
			cHashSet::cIterator sE = pSet->End();

			for( ; sI != sE ; ++sI )
			{
				unsigned long regenIdx = *sI;
				unsigned long monsterIdx = 0;
				pRegenSearch->GetAt( &monsterIdx, regenIdx );
				cMonster* pGroupMon = OBJECTMANAGER->GetMonster( monsterIdx );
				if( pGroupMon == NULL || pRegen->mPatternIdx == 0 )
				{
					continue;
				}

				if( pGroupMon->GetGroupMoveReady() == 0 )
				{
					return;
				}

				break;
			}
		}
	}

	/// 몬스터 추가
	cMonster* pMonster = OBJECTMANAGER->AddMonster( objectIdx, pRegen->mMonsterClassIdx, pRM->mRegenIdx, mapNumber, levelMode );
	if( pMonster == NULL )
	{
		//NETWORK2->PostServerEvent("cAIManager::RegenMonster OBJECTMANAGER->AddMonster( %d, %d, %d )  == false", objectIdx, pRegen->mMonsterClassIdx, pRM->mRegenIdx );
		return;
	}

	/// 그리드 추가한다.
	if( GRIDMANAGER->AddMonster( pMonster ) == false )
	{
		OBJECTMANAGER->RemoveMonster( pMonster->GetObjectID() );
		//NETWORK2->PostServerEvent("cAIManager::RegenMonster GRIDMANAGER->AddMonster( pMonster ) == false" );
		return;
	}

	pMonster->SendRegen();

	if( SetPathFinder( pMonster, pMonster->GetMapNumber(), levelMode ) == false )
		return;

	pRM->mDie = false;
	pRM->mObjectIdx = objectIdx;
	pRM->mRegenTime = 0;

	sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( groupIdx );
	if( pRegenGroup == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::RegenMonster groupIdx[%d] == pRegenGroup", groupIdx );
		return;
	}

	/// 그룹 리젠 몬스터 수 증가
	if( pRegenGroup->mMonsterAliveCnt <= pRegenGroup->mpRegenGroup->mMonsterAry.GetSize() )
		++pRegenGroup->mMonsterAliveCnt;
	else
		NETWORK2->PostServerEvent("cAIManager::RegenMonster pRegenGroup->mMonsterAliveCnt[%d] == mMonsterAry.GetSize()", pRegenGroup->mMonsterAliveCnt );

	cHashMap::cIterator rI = pRegenSearch->Find( pRM->mRegenIdx );
	cHashMap::cIterator eI = pRegenSearch->End();

	if( rI != eI )
		(*rI).mSecond = objectIdx;
	else
		pRegenSearch->Insert( pRM->mRegenIdx, objectIdx );
}

void cAIManager::ProcessGetRegen( unsigned long accumTime, unsigned char levelMode, unsigned short mapNumber, tPointerHashMap<unsigned long, void*>* pRegenGroupMap,
								 tHashSet<unsigned long>* pGatheringSet )
{
	if( pRegenGroupMap == NULL )
		return;

	if( pGatheringSet == NULL )
		return;

	cPointHashMap* regenMap = (cPointHashMap*)pRegenGroupMap->GetAt( levelMode );
	if( regenMap == NULL )
		return;

	cPointHashMap::cIterator i = regenMap->Begin();
	cPointHashMap::cIterator end = regenMap->End();
	for( ; i != end; ++i )
	{
		sRegenGather* gather = (sRegenGather*)(i->mSecond);
		if( gather == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::gather == NULL" );
			continue;
		}

		if( gather->mRegenReady == false || gather->mRegenTime > accumTime )
			continue;

		const sGatheringRegenHeader* header = GATHERINGSCRIPT->GetGatheringRegen( levelMode, gather->mRegenIdx );
		if( header == NULL )
			continue;

		/// 좌표정보중 임의의 위치 선택	
		unsigned long detailSize = header->mRegenDetail.GetSize();
		if( detailSize == 0 )
			continue;

		unsigned long selectPos = rand() % detailSize;

		/// 리젠 좌표 세부 정보
		sGatheringRegen* pRegenDetail = (sGatheringRegen*)header->mRegenDetail[selectPos];
		if( pRegenDetail == NULL )
			continue;

		/// 객체 리젠
		cGathering* pGathering = OBJECTMANAGER->AddGathering( gather->mRegenIdx, pRegenDetail->mGatheringIdx, mapNumber,
															  pRegenDetail->mPosX, pRegenDetail->mPosY, pRegenDetail->mDirection, pRegenDetail->mHeight );
		if( pGathering == NULL )
			continue;

		if( GRIDMANAGER->AddGathering( pGathering ) == false )
		{
			OBJECTMANAGER->RemoveGathering( pGathering->GetObjectID() );
			continue;
		}

		pGatheringSet->Insert( pGathering->GetObjectID() );

		gather->mRegenReady = false;
		gather->mRegenTime = 0;
		pGathering->SendRegenSync();
	}
}

void cAIManager::ProcessGatRegen( unsigned char levelMode, unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( mpRegenGatherGroupMap == NULL )
		return;

	cPointHashMap::cIterator iter;
	
	cPointHashMap* pMap = (cPointHashMap*)mpRegenGatherGroupMap->GetAt( levelMode );
	if( pMap == NULL )
		return;

	for( iter = pMap->Begin() ; iter != pMap->End() ; ++iter )
	{
		sRegenGather* pGroup = (sRegenGather*)(*iter).mSecond;
		if( !pGroup )
		{
			NETWORK2->PostServerEvent("cAIManager::pGroupHeader == NULL" );
			continue;
		}

		if( pGroup->mRegenReady == false || pGroup->mRegenTime > accumTime )
			continue;

		/// 리젠 그룹 정보
		const sGatheringRegenHeader* pScript = GATHERINGSCRIPT->GetGatheringRegen( levelMode, pGroup->mRegenIdx );
		if( pScript == NULL )
			continue;

		/// 좌표정보중 임의의 위치 선택	
		unsigned long detailSize = pScript->mRegenDetail.GetSize();
		if( detailSize == 0 )
			continue;
		unsigned long selectPos = rand() % detailSize;

		/// 리젠 좌표 세부 정보
		sGatheringRegen* pRegenDetail = (sGatheringRegen*)pScript->mRegenDetail[selectPos];
		if( pRegenDetail == NULL )
			continue;

		/// 객체 리젠
		cGathering* pGathering = OBJECTMANAGER->AddGathering( pGroup->mRegenIdx, pRegenDetail->mGatheringIdx, pRegenDetail->mMapNumber,
			pRegenDetail->mPosX, pRegenDetail->mPosY, pRegenDetail->mDirection, pRegenDetail->mHeight );
		if( pGathering == NULL )
			continue;

		if( GRIDMANAGER->AddGathering( pGathering ) == false )
		{
			OBJECTMANAGER->RemoveGathering( pGathering->GetObjectID() );
            continue;
		}

		pGroup->mRegenReady = false;
		pGroup->mRegenTime = 0;
		pGathering->SendRegenSync();
	}
}


void cAIManager::InsertReadyMonster( unsigned long monsterIdx, unsigned long regenIdx, unsigned long groupIdx )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )
	{
		assert(NULL);
		return;
	}

	unsigned short mapNumber = pMonster->GetMapNumber();

	unsigned char levelMode = DEFAULT_MAPMODE;

	cPointHashMap* pRegenGroupMap = NULL;
	cHashMap* pRegenSearch = NULL;
	switch( mServerType )
	{
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
			if( pTheme == NULL )
				return;

			pRegenGroupMap = pTheme->GetRegenGroupMap();
			pRegenSearch = pTheme->GetRegenSearchMap();
			levelMode = pTheme->GetMode();
		}
		break;
	case _E_ST_ID_PVP_:
		{
			return ;
		}
		break;
	default:
		{
			cPointHashMap* pModeMap = (cPointHashMap*)mpRegenGroupMap->GetAt( levelMode );
			if( pModeMap == NULL )
				return;

			pRegenGroupMap = pModeMap;
			pRegenSearch = &mRegenSearch;
		}
		break;
	}
	if( pRegenGroupMap == NULL || pRegenSearch == NULL )
		return;

	sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( groupIdx );
	if( pRegenGroup == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InsertReadyMonster pMonsterAry[%d] == NULL", groupIdx );
		return;
	}

	/// 그룹안에 몬스터 생존 카운트
	if( pRegenGroup->mMonsterAliveCnt != 0 )
		--pRegenGroup->mMonsterAliveCnt;
	else
		NETWORK2->PostServerEvent("cAIManager::GroupRegenAliveDown pRegenGroup->mMonsterAliveCnt[%d] == 0", pRegenGroup->mMonsterAliveCnt );

	mMonsterIdxGen.DelIdx( monsterIdx );

	cVoidAry* pAry = &pRegenGroup->mMonsterList;
	if( pAry == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InsertReadyMonster pAry == NULL[%d]", groupIdx );
		return;
	}

	unsigned long arryPos = ULONG_MAX;

	unsigned long size = pAry->GetSize();
	for( unsigned long i = 0 ; i < size ; ++i )
	{
		sRegenMonster* pRM = (sRegenMonster*)(*pAry)[i];
		if( pRM && pRM->mRegenIdx == regenIdx && pRM->mObjectIdx == monsterIdx )
		{
			sRegenScript* pRegen = MONSTERSCRIPT->GetRegenMonsterInfo( levelMode, regenIdx );
			if( pRegen != NULL )
			{
				pRM->mDie = true;
				pRM->mObjectIdx = 0;
				if( pRegen->mRegenTime != 0 )	
					pRM->mRegenTime = NETWORK2->GetAccumTime() + pRegen->mRegenTime;
				else
					arryPos = i;	/// 한번만 리젠되는 몬스터는 목록에서 아주빼버린다.
					
				break;
			}
		}
	}
	
	cHashMap::cIterator rI = pRegenSearch->Find( regenIdx );
	cHashMap::cIterator eI = pRegenSearch->End();

	if( rI != eI )
		(*rI).mSecond = 0;

	/// 한번만 리젠되는 몬스터는 목록에서 아주빼버린다.
	if( arryPos != ULONG_MAX && pAry->GetSize() > arryPos )
	{
		sRegenMonster* pRM = (sRegenMonster*)(*pAry)[arryPos];
		SAFE_DELETE( pRM );

		pAry->PopAt( arryPos );
	}

	/// 스크립트 정보 확인
	sRegenGroupScript* pRegenGroupScript = pRegenGroup->mpRegenGroup;
	if( pRegenGroupScript == NULL )
		return;

	if( pRegenGroupScript->mMonsterAry.GetSize() == 0 )
	{
		assert(NULL);
		return;
	}

	/// 그룹 리젠 조건체크
	///	살아있는수					* 100	/ 전체 마리수								> 리젠 퍼센트 
	if( pRegenGroup->mMonsterAliveCnt * 100 / pRegenGroupScript->mMonsterAry.GetSize() <= pRegenGroupScript->mRegenPer ) 
	{
		tArray<unsigned long>* pAry = &(pRegenGroupScript->mRegenGroupAry);
		if( pAry )
		{
			unsigned int count = pAry->GetSize();
			for( unsigned int i=0; i<count; ++i )
			{
				sRegenGroup* pNextRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( (*pAry)[i] );
				if( pNextRegenGroup != NULL )
					pNextRegenGroup->mUse = true;
			}
		}
	}
}


void cAIManager::SwitchRegen( unsigned short levelMode, unsigned short chennelNum, unsigned long switchGroupIdx )
{
	cPointHashMap* pRegenSwitchMap = (cPointHashMap*)mpRegenSwitchMap->GetAt( levelMode );
	if( pRegenSwitchMap == NULL )
		return;

	sSwichGroup* pSwitchGroup = (sSwichGroup*)pRegenSwitchMap->GetAt( switchGroupIdx );
	if( pSwitchGroup == NULL )
		return;

	pSwitchGroup->mRegenStop = false;
	//if( pSwitchGroup->mGroupIdx == 0 )
	//{
		bool regenCheck = false;
		if( pSwitchGroup->mRegenTime < NETWORK2->GetAccumTime() )
		{
			/// 동시 리젠시 채널번호 순위로 선택
			if( chennelNum > NETWORK2->GetChannelNum() )
			{
				NETWORK2->PostMonRegenSync( levelMode, switchGroupIdx );
				regenCheck = true;
			}
		}

		if( regenCheck == false )
		{
			/// 스위치 그룹의 [리젠그룹]목록 로드
			cArray* pAry = (cArray*)pSwitchGroup->mpGroupIdxAry;
			if( pAry != NULL && pAry->GetSize() > 0 )
			{
				/// [리젠그룹] 임의 선택
				unsigned long selectRand = rand() % pAry->GetSize();
				unsigned long groupIdx = (*pAry)[selectRand];

				pSwitchGroup->mGroupIdx = groupIdx;			

				cPointHashMap* pRegenGroupMap = (cPointHashMap*)mpRegenGroupMap->GetAt( levelMode );
				if( pRegenGroupMap != NULL )
				{
					sRegenGroup* pRegenGroup = (sRegenGroup*)pRegenGroupMap->GetAt( pSwitchGroup->mGroupIdx );
					if( pRegenGroup != NULL )	/// 메세지 발송 처리 시간차에 따른 랜덤 시간값 설정
					{
						pSwitchGroup->mGroupIdx = 0;
						pSwitchGroup->mRegenTime = pRegenGroup->mpRegenGroup->mRegenTime + ( rand() % SWITCH_RANDTIME ) + NETWORK2->GetAccumTime();
						pSwitchGroup->mRegenSyncWait = false;
					}
				}
			}
		}
	//}
}


float cAIManager::GetPathMaxSideSize( unsigned int mapNum )
{
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNum );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::GetPathMaxSideSize pPathInfo == NULL %d", mapNum );
			return 0.0f;
		}

		return pPathInfo->mMapSize;
	}
	else
	{
		unsigned short inDunMapNum = NETWORK2->GetInDunMapNum();

		/// 맵폴더 인덱스는 모두 같으므로 맞는정보 하나만 읽어온다.
		sMapInfo* pMapInfo = NULL;
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			pMapInfo = STAGESCRIPT->GetMapFolderIdx( inDunMapNum, i );
			if( pMapInfo != NULL )
				break;
		}

		if( pMapInfo == NULL )
			return 0.0f;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::GetPathMaxSideSize pPathInfo == NULL %d", inDunMapNum );
			return 0.0f;
		}

		return pPathInfo->mMapSize;
	}
}


bool cAIManager::CalcHeight( unsigned short mapNumber, float* height, float x, float y ) const
{
	bool retValue = false;

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNumber );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcHeight pPathInfo == NULL %d", mapNumber );
			return false;
		}

		retValue = pPathInfo->mNaviField->CalcHeight( height, x, y );
		if( retValue == false )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcHeight Error [%d,%f,%f,%f]", mapNumber, x, y, *height);
			return false;
		}
	}
	else
	{
		unsigned short inDunMapNum = NETWORK2->GetInDunMapNum();

		/// 맵폴더 인덱스는 모두 같으므로 맞는정보 하나만 읽어온다.
		sMapInfo* pMapInfo = NULL;
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			pMapInfo = STAGESCRIPT->GetMapFolderIdx( inDunMapNum, i );
			if( pMapInfo != NULL )
				break;
		}

		if( pMapInfo == NULL )
			return false;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcHeight pPathInfo == NULL %d", inDunMapNum );
			return false;
		}

		retValue = pPathInfo->mNaviField->CalcHeight( height, x, y );
		if( retValue == false )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcHeight Error [%d,%f,%f,%f]", inDunMapNum, x, y, *height);
			return false;
		}
	}

	return retValue;
}


/// 이동 가능지역 체크
bool cAIManager::IsPossible( unsigned short mapNumber, float x, float y, sObject object ) const
{
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNumber );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::IsPossible pPathInfo1 == NULL %d", mapNumber );
			return false;
		}

		float mapMax = pPathInfo->mMapSize;

		if( mapMax < x || mapMax < y || x < 0.0f || y < 0.0f )
		{
			Verbose->WriteLog("cAIManager::IsPossible PosError [%d,%f,%f][%d,%d]", mapNumber, x, y, object.type, object.index );
			return false;
		}

		return pPathInfo->mPathFinder->IsPossible( x, y );
	}
	else
	{
		unsigned short inDunMapNum = NETWORK2->GetInDunMapNum();
		
		/// 맵폴더 인덱스는 모두 같으므로 맞는정보 하나만 읽어온다.
		sMapInfo* pMapInfo = NULL;
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			pMapInfo = STAGESCRIPT->GetMapFolderIdx( inDunMapNum, i );
			if( pMapInfo != NULL )
				break;
		}

		if( pMapInfo == NULL )
			return false;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::IsPossible pPathInfo1 == NULL %d", inDunMapNum );
			return false;
		}

		float mapMax = pPathInfo->mMapSize;

		if( mapMax < x || mapMax < y || x < 0.0f || y < 0.0f )
		{
			Verbose->WriteLog("cAIManager::IsPossible PosError [%d,%f,%f][%d,%d]", inDunMapNum, x, y, object.type, object.index );
			return false;
		}

		return pPathInfo->mPathFinder->IsPossible( x, y );
	}
}


bool cAIManager::IsPossible( unsigned short mapNumber, NiPoint2 start, NiPoint2 goal, sObject object ) const
{
	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNumber );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::IsPossible pPathInfo2 == NULL %d", mapNumber );
			return false;
		}

		float mapMax = pPathInfo->mMapSize;

		if( mapMax < start.x || mapMax < start.y || start.x < 0.0f || start.y < 0.0f ||
			mapMax < goal.x || mapMax < goal.y || goal.x < 0.0f || goal.y < 0.0f )
		{
			Verbose->WriteLog("cAIManager::IsPossible PosError [%d,%f,%f][%f,%f][%d,%d]", mapNumber, start.x, start.y, goal.x, goal.y, object.type, object.index );
			return false;
		}

		return pPathInfo->mPathFinder->IsPossible( start, goal );
	}
	else
	{
		unsigned short inDunMapNum = NETWORK2->GetInDunMapNum();

		/// 맵폴더 인덱스는 모두 같으므로 맞는정보 하나만 읽어온다.
		sMapInfo* pMapInfo = NULL;
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			pMapInfo = STAGESCRIPT->GetMapFolderIdx( inDunMapNum, i );
			if( pMapInfo != NULL )
				break;
		}

		if( pMapInfo == NULL )
			return false;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::IsPossible pPathInfo2 == NULL %d", inDunMapNum );
			return false;
		}

		float mapMax = pPathInfo->mMapSize;

		if( mapMax < start.x || mapMax < start.y || start.x < 0.0f || start.y < 0.0f ||
			mapMax < goal.x || mapMax < goal.y || goal.x < 0.0f || goal.y < 0.0f )
		{
			Verbose->WriteLog("cAIManager::IsPossible PosError [%d,%f,%f][%f,%f][%d,%d]", mapNumber, start.x, start.y, goal.x, goal.y, object.type, object.index );
			return false;
		}

		return pPathInfo->mPathFinder->IsPossible( start, goal );
	}
}


unsigned long cAIManager::CalcPathCnt( unsigned short mapNumber, NiPoint2 start, NiPoint2 goal )
{
	unsigned int pathCnt = 0;

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNumber );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcPathCnt pPathInfo == NULL %d", mapNumber );
			return false;
		}

		pPathInfo->mPathFinder->FindPath( mTempPathAry, &pathCnt, MAX_PATH_COUNT, start, goal );

		return pathCnt;
	}
	else
	{
		unsigned short inDunMapNum = NETWORK2->GetInDunMapNum();

		/// 맵폴더 인덱스는 모두 같으므로 맞는정보 하나만 읽어온다.
		sMapInfo* pMapInfo = NULL;
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			pMapInfo = STAGESCRIPT->GetMapFolderIdx( inDunMapNum, i );
			if( pMapInfo != NULL )
				break;
		}

		if( pMapInfo == NULL )
			return false;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcPathCnt pPathInfo2 == NULL %d", inDunMapNum );
			return false;
		}

		pPathInfo->mPathFinder->FindPath( mTempPathAry, &pathCnt, MAX_PATH_COUNT, start, goal );

		return pathCnt;
	}
}


bool cAIManager::SetPathFinder( cMonster* pMonster, unsigned short mapNum, unsigned char levelMode )
{
	if( pMonster == NULL )
		return false;

	if( NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
	{
		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( mapNum );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::SetPathFinder _E_ST_NORMAL_MAP_ pPathInfo == NULL %d", mapNum );
			return false;
		}

		pMonster->SetPathFinder( pPathInfo->mPathFinder );
	}
	else
	{
		sMapInfo* pMapInfo = STAGESCRIPT->GetMapFolderIdx( NETWORK2->GetInDunMapNum(), levelMode );
		if( pMapInfo == NULL )
			return false;

		sMapPathInfo* pPathInfo = (sMapPathInfo*)mMapPathMap.GetAt( pMapInfo->folderIdx );
		if( pPathInfo == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::CalcPathCnt !_E_ST_NORMAL_MAP_ pPathInfo == NULL %d", pMapInfo->folderIdx );
			return false;
		}

		pMonster->SetPathFinder( pPathInfo->mPathFinder );
	}

	return true;
}


void cAIManager::FamilyAttack( unsigned long mainMonIdx, unsigned long groupIdx, sObject attacker )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( mainMonIdx );
	if( pMonster == NULL )
		return;

	unsigned long mapNumber = pMonster->GetMapNumber();	

	cHashMap* pRegenSearch = NULL;
	cPointHashMap* pFamilyMap = NULL;
	switch( mServerType )
	{
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
			if( pTheme == NULL )
				return;

			pRegenSearch = pTheme->GetRegenSearchMap();
			pFamilyMap = pTheme->GetFamilyMap();
		}
		break;
	default:
		{
			pRegenSearch = &mRegenSearch;
			cPointHashMap* pModeMap = (cPointHashMap*)mpFamilyMap->GetAt( DEFAULT_MAPMODE );
			if( pModeMap == NULL )
				return;

			pFamilyMap = pModeMap;
		}
		break;
	}

	if( pRegenSearch == NULL )
		return;
	if( pFamilyMap == NULL )
		return;

	cHashSet* pSet = (cHashSet*)pFamilyMap->GetAt( groupIdx );
	if( pSet == NULL )
		return;

	cHashSet::cIterator i = pSet->Begin();
	cHashSet::cIterator e = pSet->End();

	for( ; i != e ; ++i )
	{
		unsigned long monsterRegenIdx = *i;
		unsigned long monsterIdx = 0;
		pRegenSearch->GetAt( &monsterIdx, monsterRegenIdx );

		if( monsterIdx == mainMonIdx )
			continue;

		cMonster* pMon = OBJECTMANAGER->GetMonster( monsterIdx );
		if( pMon != NULL )
			pMon->AddTakeDamage( attacker, 0, 0, eTAKEDAMAGETYPE_ADDPLAYER ) ;
	}
}


bool cAIManager::FamilyNextPattern( unsigned short mapNum, unsigned long groupIdx, long* pPatternPos, char* pPatternDir )
{
	if( pPatternPos == NULL || pPatternDir == NULL )
		return false;

	cPointHashMap* pModeMap = (cPointHashMap*)mpFamilyMap->GetAt( mapNum );
	if( pModeMap == NULL )
		return false;

	cHashSet* pSet = (cHashSet*)pModeMap->GetAt( groupIdx );
	if( pSet != NULL )
	{
		cHashSet::cIterator sI = pSet->Begin();
		cHashSet::cIterator sE = pSet->End();

		for( ; sI != sE ; ++sI )
		{
			unsigned long monsterRegenIdx = *sI;
			unsigned long monsterIdx = 0;
			
			cHashMap* pRegenSearch = NULL;
			switch( mServerType )
			{
			case _E_ST_ID_THEME_:
			case _E_ST_ID_TUTORIAL_:
				{
					cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
					if( pMonster != NULL )
					{
						unsigned long mapNumber = pMonster->GetMapNumber();
						cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
						if( pTheme == NULL )
							return false;

						pRegenSearch = pTheme->GetRegenSearchMap();
					}
				}
				break;
			default:
				{
					pRegenSearch = &mRegenSearch;
				}
			}
			if( pRegenSearch == NULL )
				return false;

			pRegenSearch->GetAt( &monsterIdx, monsterRegenIdx );

			cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
			if( pMonster == NULL )
				continue;

			pMonster->GetNextPattern( pPatternPos, pPatternDir );
			return true;
		}
	}

	return false;
}


bool cAIManager::FamilyFar( unsigned short mapNum, unsigned long familyNum, NiPoint2 pos )
{
	cPointHashMap* pModeMap = (cPointHashMap*)mpFamilyMap->GetAt( mapNum );
	if( pModeMap == NULL )
		return NULL;

	cPointHashMap::cIterator i = pModeMap->Find( familyNum );
	cPointHashMap::cIterator e = pModeMap->End();

	if( i == e )
		return false;

	cHashSet* pSet = (cHashSet*)(*i).mSecond;
	if( pSet == NULL )
		return false;;

	cHashSet::cIterator sI = pSet->Begin();
	cHashSet::cIterator sE = pSet->End();

	for( ; sI != sE ; ++sI )
	{
		unsigned long monsterRegenIdx = *sI;
		unsigned long monsterIdx = 0;

		cHashMap* pRegenSearch = NULL;
		switch( mServerType )
		{
		case _E_ST_ID_THEME_:
		case _E_ST_ID_TUTORIAL_:
			{
				cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
				if( pMonster != NULL )
				{
					unsigned long mapNumber = pMonster->GetMapNumber();
					cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
					if( pTheme == NULL )
						return false;

					pRegenSearch = pTheme->GetRegenSearchMap();
				}
			}
			break;
		default:
			{
				pRegenSearch = &mRegenSearch;
			}
			break;
		}
		if( pRegenSearch == NULL )
			return false;

		pRegenSearch->GetAt( &monsterIdx, monsterRegenIdx );

		cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
		if( pMonster == NULL )
			continue;

		if( pMonster->GetMovePattern() == NULL )
			continue;

		cRangeCheck check( 500 );
		if( check.IsRange( pos, pMonster->GetPos() ) == true )
			return false;
		else
			return true;
	}

	return false;
}

void cAIManager::BossSummonMonRegen( cMonster* pBoss, unsigned short mapNumber, float x , float y, unsigned long genRangeMin, unsigned long genRangeMax, 
									unsigned long genCnt, unsigned long monsterClassIdx, bool isRand )
{
	if( pBoss == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::BossSummonMonRegen pBoss == NULL" );
		return;
	}

	for( unsigned long i = 0 ; i < genCnt ; ++i )
	{
		unsigned long monsterIdx = mMonsterIdxGen.GeneratIdx();
		if( monsterIdx == 0 )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cAIManager::BossSummonMonRegen mMonsterIdxGen == 0");
			return;
		}

		/// 몬스터가 만들어져 있을수가 없다.
		if( OBJECTMANAGER->GetMonster( monsterIdx ) )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cAIManager::BossSummonMonRegen monsterIdx[%d] duplicate", monsterIdx );
			return;
		}

		float mapMaxSize = GetPathMaxSideSize( mapNumber );
		if( mapMaxSize < x || mapMaxSize < y )
			return;

		/// 몬스터 추가
		cMonster* pMonster = OBJECTMANAGER->AddMonster( monsterIdx, monsterClassIdx, 0, mapNumber , 0 );
		if( pMonster == NULL )
			return;

		/// 소환 몬스터 설정		
		pMonster->SetSummonInfo( pBoss, mapNumber, x, y, genRangeMin, genRangeMax, 0, isRand );

		/// 소환된 몬스터를 보스에 등록
		pBoss->AddSummonMonster( monsterIdx );

		pMonster->SetPathFinder( pBoss->GetPathFinder() );	
	}
}


void cAIManager::CheatSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long genCnt )
{

	for( unsigned long i = 0 ; i < genCnt ; ++i )
	{
		unsigned long monsterIdx = mMonsterIdxGen.GeneratIdx();
		if( monsterIdx == 0 )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cAIManager::CheatAddMonster mMonsterIdxGen == 0");
			return;
		}

		/// 몬스터가 만들어져 있을수가 없다.
		if( OBJECTMANAGER->GetMonster( monsterIdx ) )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cAIManager::CheatAddMonster monsterIdx[%d] duplicate", monsterIdx );
			return;
		}

		float mapMaxSize = GetPathMaxSideSize( mapNumber );
		if( mapMaxSize < x || mapMaxSize < y )
			return;

		/// 몬스터 추가
		cMonster* pMonster = OBJECTMANAGER->AddMonster( monsterIdx, monsterClassIdx, 0, mapNumber, 0 );
		if( pMonster == NULL )
			return;

		/// 소환 몬스터 설정		
		pMonster->SetSummonInfo( 0, mapNumber, x, y, 0, 700, 0 );

		/// 길찾기 설정
		switch( mServerType )
		{
		case _E_ST_NORMAL_MAP_:	SetPathFinder( pMonster, mapNumber, DEFAULT_MAPMODE ); break;
		case _E_ST_ID_THEME_:
		case _E_ST_ID_TUTORIAL_:
			{
				cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
				if( pTheme != NULL )
					SetPathFinder( pMonster, NETWORK2->GetInDunMapNum(), pTheme->GetMode() );
			}
			break;
		case _E_ST_ID_PVP_:	SetPathFinder( pMonster, NETWORK2->GetInDunMapNum(), DEFAULT_MAPMODE ); break;
		}
	}
}

/// 퀘스트로 인한 몬스터 add
unsigned long cAIManager::QuestSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long destroyTime )
{
	unsigned long monsterIdx = mMonsterIdxGen.GeneratIdx();
	if( monsterIdx == 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::QuestAddMonster mMonsterIdxGen == 0");
		return 0;
	}

	/// 몬스터가 만들어져 있을수가 없다.
	if( OBJECTMANAGER->GetMonster( monsterIdx ) )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::QuestAddMonster monsterIdx[%d] duplicate", monsterIdx );
		return 0;
	}

	float mapMaxSize = GetPathMaxSideSize( mapNumber );
	if( mapMaxSize < x || mapMaxSize < y )
		return 0;

	/// 몬스터 추가
	cMonster* pMonster = OBJECTMANAGER->AddMonster( monsterIdx, monsterClassIdx, 0, mapNumber, 0 );
	if( !pMonster )
	{
		//NETWORK2->PostServerEvent("cAIManager::QuestAddMonster pMonster == NULL" );
		return 0;
	}	
	/// 소환 몬스터 설정
	pMonster->SetSummonInfo( 0, mapNumber, x, y, 0, 700, destroyTime );

	///// 길찾기 설정
	//if( mServerType == _E_ST_NORMAL_MAP_ )
	//	pMonster->SetPathFinder( mPathFinder[pMonster->GetMapNumber()-MAP_MIN] );	
	//else	/// 081106 PKH 인던
	//	pMonster->SetPathFinder( mInstDgPathFinder );	

	/// 길찾기 설정
	switch( mServerType )
	{
	case _E_ST_NORMAL_MAP_:	SetPathFinder( pMonster, mapNumber, DEFAULT_MAPMODE ); break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		{
			cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
			if( pTheme != NULL )
				SetPathFinder( pMonster, NETWORK2->GetInDunMapNum(), pTheme->GetMode() );
		}
		break;
	case _E_ST_ID_PVP_:	SetPathFinder( pMonster, NETWORK2->GetInDunMapNum(), DEFAULT_MAPMODE ); break;
	}

	return monsterIdx;
}

/// 퀘스트로 인한 몬스터 add
cMonster* cAIManager::StoneSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long teamType )
{
	unsigned long monsterIdx = mMonsterIdxGen.GeneratIdx();
	if( monsterIdx == 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::StoneSummonMonRegen mMonsterIdxGen == 0");
		return NULL;
	}

	/// 몬스터가 만들어져 있을수가 없다.
	if( OBJECTMANAGER->GetMonster( monsterIdx ) )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::StoneSummonMonRegen monsterIdx[%d] duplicate", monsterIdx );
		return NULL;
	}

	float mapMaxSize = GetPathMaxSideSize( NETWORK2->GetInDunMapNum() );
	if( mapMaxSize < x || mapMaxSize < y )
		return NULL;

	/// 길찾기 설정
	if( NETWORK2->GetInDunMapNum() != MAP_DEATHMATCH )
		return NULL;

	/// 몬스터 추가
	cMonster* pMonster = OBJECTMANAGER->AddMonster( monsterIdx, monsterClassIdx, 0, mapNumber, 0 );
	if( !pMonster )
	{
		return NULL;
	}	
	pMonster->SetDMTeamType( teamType );

	/// 소환 몬스터 설정
	pMonster->SetSummonInfo( 0, mapNumber, x, y, 0, 1, 0 );

	SetPathFinder( pMonster, NETWORK2->GetInDunMapNum(), DEFAULT_MAPMODE );

	return pMonster;
}

bool cAIManager::AutoDetectSummonMonRegen( unsigned short mapNumber, float x, float y )
{
	if( mServerType != _E_ST_NORMAL_MAP_ )
		return false;		

	// 마을 소환 막기
	//if( mapNumber == 1 )
	//	return false;

	// 몬스터 리스트로부터 랜덤한 몬스터 선택.
	unsigned long monsterClassIdx = MONSTERSCRIPT->GetRandomDummy();
	unsigned long monsterIdx = mMonsterIdxGen.GeneratIdx();
	if( monsterIdx == 0 )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::AutoDetectSummonMonReget mMonsterIdxGen == 0");
		return false;
	}

	/// 몬스터가 만들어져 있을수가 없다.
	if( OBJECTMANAGER->GetMonster( monsterIdx ) )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cAIManager::AutoDetectSummonMonReget monsterIdx[%d] duplicate", monsterIdx );
		return false;
	}

	float mapMaxSize = GetPathMaxSideSize( mapNumber );
	if( mapMaxSize < x || mapMaxSize < y )
		return false;

	/// 몬스터 추가
	cMonster* pMonster = OBJECTMANAGER->AddMonster( monsterIdx, monsterClassIdx, 0, mapNumber, 0 );
	if( pMonster == NULL )
		return false;

	/// 소환 몬스터 설정
	pMonster->SetSummonInfo( 0, mapNumber, x, y, 0, 0, 60000, true, true );

	/// 길찾기 설정
	SetPathFinder( pMonster, mapNumber, DEFAULT_MAPMODE );
	return true;
}

void cAIManager::InsertReadyGathering( unsigned long gatheringIdx, unsigned long mapNumber, unsigned long regenIdx )
{
	unsigned char mapMode = DEFAULT_MAPMODE;

	//
	cPointHashMap* regenGroupMap = mpRegenGatherGroupMap;	
	if( mServerType == _E_ST_ID_TUTORIAL_ || mServerType == _E_ST_ID_THEME_ )
	{
		cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( mapNumber );
		if( pTheme == NULL )
		{
			NETWORK2->PostServerEvent("cAIManager::InsertReadyGathering == NULL[%d,%d]", mapMode, mapNumber );
			return;
		}

		regenGroupMap = pTheme->GetGatheringGroupMap();
		mapMode = pTheme->GetMode();
		pTheme->DeleteGathering( gatheringIdx );
	}

	if( regenIdx == 0 )
		return;

	if( regenGroupMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::regenGroupMap == NULL[%d,%d]", mapMode, mapNumber );
		return;
	}

	cPointHashMap* pMap = (cPointHashMap*)regenGroupMap->GetAt( mapMode );
	if( pMap == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InsertReadyMonster pMap == NULL[%d]", mapMode );
		return;
	}

	sRegenGather* pGroup = (sRegenGather*)pMap->GetAt( regenIdx );
	if( pGroup == NULL )
	{
		NETWORK2->PostServerEvent("cAIManager::InsertReadyMonster pMonsterAry[%d,%d] == NULL", mapNumber, regenIdx );
		return;
	}

	/// 초기화
	const sGatheringRegenHeader* pScript = GATHERINGSCRIPT->GetGatheringRegen( mapMode, regenIdx );
	if( pScript == NULL )
	{
		pGroup->mRegenTime = 0;
		return;
	}

	pGroup->mRegenReady = true;

	/// 다음 리젠 설정
	unsigned long randTime = pScript->mRandTime;
	if( randTime != 0 )
		randTime = rand() % pScript->mRandTime;

	pGroup->mRegenTime = NETWORK2->GetAccumTime() + pScript->mRegenWaitTime + randTime;

}
