#include "gamesrv.h"

#include "stdafx.h"
#include "SkillManager.h"
#include "ObjectManager.h"

#include "../Common/Protocol.h"
#include "Player_Common.h"
#include "Monster_Common.h"
#include "Status_Common.h"

#include "BaseObject.h"
#include "Monster.h"
#include "Npc.h"
#include "Item.h"
#include "Totem.h"
#include "AIManager.h"
#include "StatusCalc_Server.h"
#include "DamageCalc.h"
#include "StatusScript.h"
#include "MonsterScript.h"
#include "NpcScript.h"
#include "TotemScript.h"
#include "PvPScript.h"
#include "VehicleScript.h"
#include "GatheringScript.h"
#include "Gathering.h"
#include "VehicleScript.h"
#include "ShortcutScript.h"
#include "LevelScript.h"
#include "DuelManager.h"
#include "ItemManager.h"
#include "GridManager.h"
#include "ThemeManager.h"
#include "UserPortal.h"
#include "TutorialScript.h"
#include "StageScript.h"
#include "TutorialSkillScript.h"
#include "Pvpmanager.h"

#include "AppTimer.h"

cObjectManager* cObjectManager::mpObjectManager = NULL;

cObjectManager::cObjectManager(void) 
{
	mpObjectManager = this;

	mpPlayerNamePool = NULL;

	mpPlayerPool  = NULL;
	mpMonsterPool = NULL;
	mpNpcPool     = NULL;
	mpItemPool    = NULL;
	mpGatheringPool = NULL;

	mpPlayerMap  = NULL;
	mpMonsterMap = NULL;
	mpNpcMap     = NULL;
	mpItemMap    = NULL;
	mpGatheringScript    = NULL;
	mpGatheringMap = NULL;
	mpUserPortalMap = NULL;

	mpShortCutScript = NULL;

	mpDeathMonsterMap = NULL;
	mpTutorialScript = NULL;
	mpTutorialSkillScript = NULL;
}

cObjectManager::~cObjectManager(void)
{
	Release( );

	mpObjectManager = NULL;
}

// Init Method
bool cObjectManager::Init( int serverType )
{
	// Memory Pool - cPlayerNamePool class.
	mpPlayerNamePool = new cPlayerNamePool( );

	// memorypool
	mpPlayerPool    = new cPlayerPool( 1000 );
	mpMonsterPool   = new cMonsterPool( 1000, 1000 );
	mpNpcPool       = new cNpcPool( 100 );
	mpItemPool      = new cItemPool( 1000 );
	mpTotemPool     = new cTotemPool( 1000 );
	mpGatheringPool = new cGatheringPool( 1000 );
	mpUserPortalPool		= new cUserPortalPool( 100 );

	// hashtable
	mpPlayerMap    = new cPlayerMap( 1000 );
	mpMonsterMap   = new cMonsterMap( 1000 );	
	mpNpcMap       = new cNpcMap( 100 );
	mpItemMap      = new cItemMap( 1000 );
	mpTotemMap     = new cTotemMap( 1000 );
	mpGatheringMap = new cGatheringMap( 1000 );
	mpUserPortalMap = new cPointHashMap( 100 );

	mpDeathMonsterMap   = new cMonsterMap( 100 );	

	unsigned short mapType = MAP_NONE;
	switch( serverType )
	{
	case _E_ST_ID_PVP_:
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		mapType = NETWORK2->GetInDunMapNum();
		break;
	}

	/// 070115 PKH 스텟 계산기
	mpStatusScript = new cStatusScript;
	if ( !mpStatusScript->Init( ) )
	{
		return false;
	}
	mpStatusCalc = new cStatusCalc_Server;
	mpDamageCalc = new cDamageCalc;

	/// 070130 PKH 몬스터 스크립트
	mpMonsterScript = new cMonsterScript;
	if ( !mpMonsterScript->Init() )
	{
		return false;
	}

	/// 070515 PKH 레벨 관리 
	mpLevelScript = new cLevelScript;
	if( mpLevelScript->Init() == false )
	{
		return false;
	}

	/// 070626 PKH NPC정보 로드
	mpNpcScript = new cNPCScript;
	if ( !mpNpcScript->Init() )
	{
		return false;
	}

	// cGatheringScript 클래스 생성 및 초기화
	mpGatheringScript = new cGatheringScript( );
	if( !mpGatheringScript->Init() )
		return false;

	/// 081023 PKH 토템 스크립트
	mpTotemScript = new cTotemScript;
	if ( !mpTotemScript->Init( ) )
	{
		return false;
	}

	/// 090106 PKH 탈것
	mpVehicleScript = new cVehicleScript;
	if ( !mpVehicleScript->Init( ) )
	{
		return false;
	}

	/// 
	mpShortCutScript = new cShortCutScript;
	if( !mpShortCutScript->Init() )
	{
		return false;
	}

	mpTutorialScript = new cTutorialScript;
	if( !mpTutorialScript->Init() )
	{
		return false;
	}

	mpTutorialSkillScript = new cTutorialSkillScript;
	if( !mpTutorialSkillScript->Init() )
	{
		return false;
	}

	StageScriptInit( mapType );

	switch( serverType )
	{
	case _E_ST_NORMAL_MAP_:
		LoadNpcInfo();
		break;
	case _E_ST_ID_PVP_:
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		break;
	default:
		return false;
	}

	return true;
}

void cObjectManager::StageScriptInit( unsigned short mapType )
{
	if( mapType == MAP_NONE )
	{
		mpMonsterScript->StageInit( mapType, DEFAULT_MAPMODE );
		mpNpcScript->StageInit( mapType, DEFAULT_MAPMODE );
		mpGatheringScript->StageInit( mapType, DEFAULT_MAPMODE );
	}
	else
	{
		for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
		{
			if( STAGESCRIPT->GetMapFolderIdx( mapType, i ) != NULL )
			{
				mpMonsterScript->StageInit( mapType, i );
				mpNpcScript->StageInit( mapType, i );
				mpGatheringScript->StageInit( mapType, i );
			}
		}
	}
}

// Release Method
void cObjectManager::Release( )
{
	SAFE_RELEASE_DELETE( mpTutorialSkillScript );

	SAFE_DELETE( mpTutorialScript );

	SAFE_DELETE( mpShortCutScript );

	SAFE_DELETE( mpVehicleScript );

	/// 토템 리소스 제거
	SAFE_RELEASE_DELETE( mpTotemScript );

	SAFE_RELEASE_DELETE( mpGatheringScript );

	SAFE_RELEASE_DELETE( mpNpcScript );

	/// 레벨스크립트 제거
	SAFE_RELEASE_DELETE( mpLevelScript );

	/// 몬스터 리소스 제거
	SAFE_RELEASE_DELETE( mpMonsterScript );

	/// 070117 PKH 스텟 계산기
	SAFE_DELETE( mpDamageCalc );
	SAFE_DELETE( mpStatusCalc );
	SAFE_RELEASE_DELETE( mpStatusScript );

	
	if ( mpDeathMonsterMap )
	{
		cMonsterMap::cIterator begin = mpDeathMonsterMap->Begin( );
		cMonsterMap::cIterator end   = mpDeathMonsterMap->End( );
		cMonster*              monster;
		while ( begin != end )
		{
			monster = (cMonster*)(*begin++).mSecond;
			RecoverDeathMonster( monster );
		}
	}
	SAFE_DELETE( mpDeathMonsterMap );

	if ( mpUserPortalMap )
	{
		cPointHashMap::cIterator begin = mpUserPortalMap->Begin( );
		cPointHashMap::cIterator end   = mpUserPortalMap->End( );
		cUserPortal* pUserPortal;
		while ( begin != end )
		{
			pUserPortal = (cUserPortal*)(*begin++).mSecond;
			delete pUserPortal;
		}
	}
	SAFE_DELETE( mpUserPortalMap );
	SAFE_DELETE( mpUserPortalPool );

	if ( mpGatheringMap )
	{
		cGatheringMap::cIterator begin = mpGatheringMap->Begin( );
		cGatheringMap::cIterator end   = mpGatheringMap->End( );
		cGathering*              gathering;
		while ( begin != end )
		{
			gathering = (cGathering*)(*begin++).mSecond;
			delete gathering;
		}
	}
	SAFE_DELETE( mpGatheringMap );
	SAFE_DELETE( mpGatheringPool );

	if ( mpTotemMap )
	{
		cTotemMap::cIterator begin = mpTotemMap->Begin( );
		cTotemMap::cIterator end   = mpTotemMap->End( );
		cTotem*              totem;
		while ( begin != end )
		{
			totem = (*begin++).mSecond;
			delete totem;
		}
	}
	SAFE_DELETE( mpTotemMap );
	SAFE_DELETE( mpTotemPool );

	/// 071105 KCJ 맵에 들어 있는 내용물 소멸자 호출
	if ( mpItemMap )
	{
		cItemMap::cIterator begin = mpItemMap->Begin( );
		cItemMap::cIterator end   = mpItemMap->End( );
		cItem*              item;
		while ( begin != end )
		{
			item = (*begin++).mSecond;
			delete item;
		}
	}
	SAFE_DELETE( mpItemMap );
	SAFE_DELETE( mpItemPool );

	/// 061201 PKH 맵에 들어 있는 내용물 소멸자 호출
	if ( mpNpcMap )
	{
		cNpcMap::cIterator begin = mpNpcMap->Begin( );
		cNpcMap::cIterator end   = mpNpcMap->End( );
		cNpc*              npc;
		while ( begin != end )
		{
			npc = (*begin++).mSecond;
			delete npc;
		}
	}
	SAFE_DELETE( mpNpcMap );
	SAFE_DELETE( mpNpcPool );

	/// 061201 PKH 맵에 들어 있는 내용물 소멸자 호출
	if ( mpMonsterMap )
	{
		cMonsterMap::cIterator begin = mpMonsterMap->Begin( );
		cMonsterMap::cIterator end   = mpMonsterMap->End( );
		cMonster*              monster;
		while ( begin != end )
		{
			monster = (cMonster*)(*begin++).mSecond;
			delete monster;
		}
	}
	SAFE_DELETE( mpMonsterMap );
	SAFE_DELETE( mpMonsterPool );

	/// 070221 PKH 맵에 들어 있는 내용물 소멸자 호출
	if ( mpPlayerMap )
	{
		cPlayerMap::cIterator begin = mpPlayerMap->Begin( );
		cPlayerMap::cIterator end   = mpPlayerMap->End( );
		cPlayer*              player;
		while ( begin != end )
		{
			player = (*begin++).mSecond;
			delete player;	
		}
	}
	SAFE_DELETE( mpPlayerMap );
	SAFE_DELETE( mpPlayerPool );

	// Memory Pool - cPlayerNamePool class.
	SAFE_DELETE( mpPlayerNamePool );
}

// Release Method
void cObjectManager::ThemeDataClear( )
{
	if ( mpDeathMonsterMap )
	{
		cMonsterMap::cIterator begin = mpDeathMonsterMap->Begin( );
		cMonsterMap::cIterator end   = mpDeathMonsterMap->End( );
		cMonster*              monster;
		while ( begin != end )
		{
			monster = (cMonster*)(*begin++).mSecond;
			RecoverDeathMonster( monster );
		}

		mpDeathMonsterMap->Clear();
	}

	if ( mpUserPortalMap )
	{
		cPointHashMap::cIterator begin = mpUserPortalMap->Begin( );
		cPointHashMap::cIterator end   = mpUserPortalMap->End( );
		cUserPortal* pUserPortal;
		while ( begin != end )
		{
			pUserPortal = (cUserPortal*)(*begin++).mSecond;
			GRIDMANAGER->RemoveUserPortal( pUserPortal );
			delete pUserPortal;			
		}

		mpUserPortalMap->Clear();
	}
	if ( mpTotemMap )
	{
		cTotemMap::cIterator begin = mpTotemMap->Begin( );
		cTotemMap::cIterator end   = mpTotemMap->End( );
		cTotem*              totem;
		while ( begin != end )
		{
			totem = (*begin++).mSecond;
			GRIDMANAGER->RemoveTotem( totem );
			delete totem;
		}

		mpTotemMap->Clear();
	}

	/// 071105 KCJ 맵에 들어 있는 내용물 소멸자 호출
	if ( mpItemMap )
	{
		cItemMap::cIterator begin = mpItemMap->Begin( );
		cItemMap::cIterator end   = mpItemMap->End( );
		cItem*              item;
		while ( begin != end )
		{
			item = (*begin++).mSecond;
			GRIDMANAGER->RemoveItem( item );
			delete item;
		}

		mpItemMap->Clear();
	}

	/// 061201 PKH 맵에 들어 있는 내용물 소멸자 호출
	if ( mpMonsterMap )
	{
		cMonsterMap::cIterator begin = mpMonsterMap->Begin( );
		cMonsterMap::cIterator end   = mpMonsterMap->End( );
		cMonster*              monster;
		while ( begin != end )
		{
			monster = (cMonster*)(*begin++).mSecond;
			if( monster != NULL )
			{
				monster->Release( false );
				GRIDMANAGER->RemoveMonster( monster );
			}
			delete monster;
		}

		mpMonsterMap->Clear();
	}

	if ( mpGatheringMap )
	{
		cGatheringMap::cIterator begin = mpGatheringMap->Begin( );
		cGatheringMap::cIterator end   = mpGatheringMap->End( );
		cGathering*              gathering;
		while ( begin != end )
		{
			gathering = (cGathering*)(*begin++).mSecond;
			delete gathering;
		}
		mpGatheringMap->Clear();
	}
}

// AllocPlayer Method
cPlayer* cObjectManager::AllocPlayer()
{
	return mpPlayerPool->Alloc();
}

// FreePlayer Method
void cObjectManager::FreePlayer( cPlayer* ptr )
{
	mpPlayerPool->Free( ptr );
}

// AllocMonster Method
cMonster* cObjectManager::AllocMonster( )
{
	return mpMonsterPool->Alloc();
}

// FreeMonster Method
void cObjectManager::FreeMonster( cMonster* ptr )
{
	mpMonsterPool->Free( ptr );
}

// AllocNpc Method
cNpc* cObjectManager::AllocNpc()
{
	return mpNpcPool->Alloc();
}

// FreeNpc Method
void cObjectManager::FreeNpc( cNpc* ptr )
{
	mpNpcPool->Free( ptr );
}

// AllocItem Method
cItem* cObjectManager::AllocItem( )
{
	return mpItemPool->Alloc();
}

// FreeItem Method
void cObjectManager::FreeItem(cItem* ptr)
{
	mpItemPool->Free( ptr );
}

// AllocTotem Method
cTotem* cObjectManager::AllocTotem( )
{
	return mpTotemPool->Alloc();
}

// FreeTotem Method
void cObjectManager::FreeTotem(cTotem* ptr)
{
	mpTotemPool->Free( ptr );
}

// AllocGathering Method
cGathering* cObjectManager::AllocGathering( )
{
	return mpGatheringPool->Alloc();
}

// FreeGathering Method
void cObjectManager::FreeGathering(cGathering* ptr)
{
	mpGatheringPool->Free( ptr );
}

// AllocUserPortal Method
cUserPortal* cObjectManager::AllocUserPortal( )
{
	return mpUserPortalPool->Alloc();
}

// FreeUserPortal Method
void cObjectManager::FreeUserPortal(cUserPortal* ptr)
{
	mpUserPortalPool->Free( ptr );
}

// AddPlayer Method
cPlayer* cObjectManager::AddPlayer(unsigned long connectionIdx, unsigned long playerIdx, wchar_t* playerName, unsigned long money, unsigned long deposit)
{
	cPlayer* player = NULL;
	if ( mpPlayerMap->GetAt( playerIdx ) == NULL )
	{
		player = new cPlayer( );
		if( player == 0 )
			return 0;

		if( player->Init( connectionIdx, playerIdx, money, deposit ) == false )
		{
			SAFE_DELETE( player );
			return 0;
		}

		if( mpPlayerMap->Insert( playerIdx, player ) == false )
		{
			SAFE_DELETE( player );
			return 0;
		}

		PerPlayerName* name = mpPlayerNamePool->Search( player->GetName( ) );
		if ( name != NULL )
			mpPlayerNamePool->ReleasePlayerName( name );
		mpPlayerNamePool->GetPlayerName( connectionIdx, playerIdx, playerName );
	}
	return player;
}

// RemovePlayer Method
bool cObjectManager::RemovePlayer(unsigned long playerIdx)
{
	cPlayer* player = (cPlayer*)mpPlayerMap->GetAt( playerIdx );
	if ( player != NULL )
	{
		PerPlayerName* name = mpPlayerNamePool->Search( player->GetName( ) );
		if ( name != NULL )
			mpPlayerNamePool->ReleasePlayerName( name );

		//player->ReleaseSkillInfluenceSet();
		mpPlayerMap->Erase( playerIdx );
		SAFE_DELETE( player );
		return true;
	}
	return false;
}

// AddMonster Method - 몬스터 생성은 풀에서 할당 받는 형태로 오버 라이드됨
cMonster* cObjectManager::AddMonster( unsigned long monsterIdx, unsigned long classIdx, unsigned long regenIdx, unsigned short mapNumber, unsigned char levelMode )
{
	if( NETWORK2->GetServerType() == _E_ST_ID_THEME_ )
	{
		if( THEMEMANAGER->MonsterCntTest( mapNumber, monsterIdx, classIdx, regenIdx ) == false )
			return NULL;
	}

	cMonster* monster = NULL;
	// 061126 PKH 맵에 이미 만들어져 있는 몬스터가 있는지 확인
	if ( mpMonsterMap->GetAt( monsterIdx ) == NULL )
	{
		// 061126 PKH 몬스터 객체생성(monster new override)
		monster = new cMonster( );

		// 061126 PKH 몬스터 초기화 && 생성된 몬스터를 맵에 추가
		if ( !monster )
		{
			SAFE_DELETE( monster );
		}
		else if( !monster->Init( monsterIdx, classIdx, regenIdx, mapNumber, levelMode ) )
		{
			SAFE_DELETE( monster );
		}
		else if ( !mpMonsterMap->Insert( monsterIdx, monster ) )
		{
			SAFE_DELETE( monster );
		}
	}
	return monster;
}

// RemoveMonster Method
bool cObjectManager::RemoveMonster(unsigned long monsterIdx)
{
	cMonster* monster = (cMonster*)mpMonsterMap->GetAt( monsterIdx );
	if ( monster != NULL )
	{
		mpMonsterMap->Erase( monsterIdx );
		SAFE_DELETE( monster );
		return true;
	}
	return false;
}

void cObjectManager::InsertDeleteMonster( unsigned long monsterIdx )
{
	mDelMonster.PushBack( monsterIdx );
}

void cObjectManager::MonJobProcess( cMonster* pMonster, eACTION_ID actionid, eACTION_CHANGE actionstate )
{
	if( pMonster == NULL )
		return;

	/// eACTCH_CONTINUE일경우 액션상태 안바낌
	if(actionstate!=eACTCH_CONTINUE)
	{
		/// 플레이어 비정상위치 도망
		if ( actionstate == eACTCH_ESCAPE )
		{
			pMonster->ActionChange( eACT_ESCAPE ); 
			return;
		}

		/// 현재 액션다음 행동에 해당하는 액션으로 교체
		switch( actionid )
		{
			/// 이동상태
		case eACT_MOVE:
			{
				/// 이동이 끝난경우 추적
				if ( actionstate == eACTCH_MOVE_END ) { pMonster->ActionChange( eACT_FOLLOW ); }
			}
			break;

			/// 추적 상태 인경우
		case eACT_FOLLOW:	
			{
				/// 추적 포기 이동
				if ( actionstate == eACTCH_FOLLOW_END ) { pMonster->ActionChange( eACT_MOVE ); }
				/// 추적중 전투 재시작
				if ( actionstate == eACTCH_FOLLOW_ATTACK ) { pMonster->ActionChange( eACT_ATTACK ); }
			}
			break;

			/// 전투 끝 처리
		case eACT_ATTACK:	
			{
				/// 전투끝 몬스터 승리
				if ( actionstate == eATTCH_ATTACK_END ) { pMonster->ActionChange( eACT_MOVE ); }
				/// 플레이어 도망
				if ( actionstate == eATTCH_ATTACK_FOLLOW ) { pMonster->ActionChange( eACT_FOLLOW ); }
			}
			break;

			/// 도망이 끝난경우 주위이동
		case eACT_ESCAPE:
			{
				/// 도망 끝(타겟 죽음) 자유이동
				if ( actionstate == eACTCH_ESCAPE_END ) { pMonster->ActionChange( eACT_MOVE ); }
				/// 목적지까지 도망 다시 전투 시작
				if ( actionstate == eACTCH_ESCAPE_FOLLOW ) { pMonster->ActionChange( eACT_FOLLOW ); }
			}
			break;

			/// 죽는 경우 대기상태 처리
		case eACT_DIE:
			{
				if ( actionstate == eACTCH_DIE_END )	{ pMonster->ActionChange( eACT_IDLE ); }
			}
			break;

			/// 대기 시간이 끝난경우 전투 추적 상태
		case eACT_IDLE:
			{
				if ( actionstate == eACTCH_MODE_END ) { pMonster->ActionChange( eACT_ATTACK ); }
			}
			break;

			/// 강제로 밀려 나가다가 끝난 경우 다시 전투시작
		case eACT_PUSHPULL:
			{
				/// 밀림 끝 공격 시작
				if ( actionstate == eACTCH_PUSHPULL_END ) { pMonster->ActionChange( eACT_ATTACK ); }
				/// 모드 대기상태 인경우 나머지 시간 진행
				if ( actionstate == eACTCH_PUSHPULL_IDLE )	{ pMonster->ActionChange( eACT_IDLE ); }
			}
			break;

		default: break;
		} 
	} 
}


void cObjectManager::AddDeathMonster( cMonster* pMonster )
{
	if( pMonster == NULL )
	{
		NETWORK2->PostServerEvent("cObjectManager::AddDeathMonster pMonster == NULL" );
		return;
	}

	mDelMonsterMapOnly.PushBack( pMonster->GetObjectID() );		/// 몬스터맵에 있는 몬스터를 지운다. - pool유지

	/// 다른 맵에 담아둔다.
	if( mpDeathMonsterMap->Insert( pMonster->GetObjectID(), pMonster ) == false )
		NETWORK2->PostServerEvent("cObjectManager::AddDeathMonster mpDeathMonsterMap->Insert( %d ) == false", pMonster->GetObjectID() );
}

void cObjectManager::RecoverDeathMonster( cMonster* pMonster )
{
	if( GRIDMANAGER->GetMonster( pMonster->GetObjectID() ) == false )
	{
		mpMonsterMap->Insert( pMonster->GetObjectID(), pMonster );
		GRIDMANAGER->AddMonster( pMonster );
	}
}


// AddNpc Method
cNpc* cObjectManager::AddNpc( int uniqueIdx, unsigned long classIdx, unsigned short mapNumber, float x, float y, float dir )
{
	cNpc* npc = NULL;
	// 070627 PKH 맵에 이미 만들어져 있는 NPC가 있는지 확인
	if ( mpNpcMap->GetAt( uniqueIdx ) == NULL )
	{
		// 070627 PKH NPC 객체생성(monster new override)
		npc = new cNpc( );

		// 070627 PKH NPC 초기화 && 생성된 NPC를 맵에 추가
		if ( !npc )
		{
			SAFE_DELETE( npc );
		}
		else if ( !npc->Init( uniqueIdx, classIdx, mapNumber, x, y, dir ) )
		{
			SAFE_DELETE( npc );
		}
		else if ( !mpNpcMap->Insert( uniqueIdx, npc ) )
		{
			SAFE_DELETE( npc );
		}
	}
	return npc;
}

// RemoveNpc Method
bool cObjectManager::RemoveNpc(unsigned long npcIdx)
{
	cNpc* npc = mpNpcMap->GetAt( npcIdx );
	if ( npc != NULL )
	{
		mpNpcMap->Erase( npcIdx );
		SAFE_DELETE( npc );
		return true;
	}
	return false;
}

// AddItem Method
cItem* cObjectManager::AddItem(unsigned long itemIdx, cBaseObject* baseObject, bool getAuto)
{
	cItem* item = NULL;
	if ( mpItemMap->GetAt( itemIdx ) == NULL )
	{
		item = new cItem( );
		if ( !item )
		{
			SAFE_DELETE( item );
		}
		else if ( !item->Init( itemIdx, baseObject, getAuto ) )
		{
			SAFE_DELETE( item );
		}
		else if ( !mpItemMap->Insert( itemIdx, item ) )
		{
			SAFE_DELETE( item );
		}
	}
	return item;
}

// RemoveItem Method
bool cObjectManager::RemoveItem(unsigned long itemIdx)
{
	cItem* item = mpItemMap->GetAt( itemIdx );
	if ( item != NULL )
	{
		mpItemMap->Erase( itemIdx );
		SAFE_DELETE( item );
		return true;
	}
	return false;
}

// AddTotem Method
cTotem* cObjectManager::AddTotem( unsigned long calssIdx, unsigned long partyIdx, unsigned long partyUnionIdx, NiPoint2 pos, sObject attacker, 
								 eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType, unsigned long skillIdx, float direction, unsigned char applyCnt )
{
	unsigned long objectIdx = mTotemIdxGen.GeneratIdx();

	cTotem* pTotem = NULL;
	if ( mpTotemMap->GetAt( objectIdx ) == NULL )
	{
		pTotem = new cTotem( );

		if ( !pTotem )
		{
			SAFE_DELETE( pTotem );
		}
		else if ( !pTotem->TotemInit( attacker, objectIdx, calssIdx, partyIdx, partyUnionIdx, pos, applyType, attributeType, skillIdx, direction, applyCnt ) )
		{
			SAFE_DELETE( pTotem );
		}
		else if ( !mpTotemMap->Insert( objectIdx, pTotem ) )
		{
			SAFE_DELETE( pTotem );
		}
	}
	return pTotem;
}

// AddTotem Method
cTotem* cObjectManager::PvPAddTotem( unsigned char teamType, unsigned long calssIdx, unsigned short pvpIdx, NiPoint2 pos, 
									eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType )
{
	unsigned long objectIdx = mTotemIdxGen.GeneratIdx();

	cTotem* pTotem = NULL;
	if ( mpTotemMap->GetAt( objectIdx ) == NULL )
	{
		pTotem = new cTotem( );

		if ( pTotem == NULL )
		{
			SAFE_DELETE( pTotem );
			return NULL;
		}

		if ( pTotem->PvPTotemInit( (ePVPDM_TEAM_TYPE)teamType, objectIdx, calssIdx, pvpIdx, pos, applyType, attributeType ) == false )
		{
			SAFE_DELETE( pTotem );
			return NULL;
		}

		if ( mpTotemMap->Insert( objectIdx, pTotem ) == NULL )
		{
			SAFE_DELETE( pTotem );
			return NULL;
		}
	}
	return pTotem;
}

// RemoveTotem Method
bool cObjectManager::RemoveTotem( unsigned long objectIdx )
{
	cTotem* pTotem = mpTotemMap->GetAt( objectIdx );
	if ( pTotem != NULL )
	{
		mpTotemMap->Erase( objectIdx );
		SAFE_DELETE( pTotem );
		return true;
	}
	return false;
}

void cObjectManager::InsertDeleteTotem( unsigned long totemIdx )
{
	mDelTotem.PushBack( totemIdx );
}

// AddGathering Method
cGathering* cObjectManager::AddGathering( unsigned long regenIdx, unsigned long gatheringIdx, unsigned short mapNumber,
										 float posX, float posY, float direction, float height )
{
	cGathering* pGathering = NULL;
	unsigned long objectIdx = mGatheringIdxGen.GeneratIdx();

	if ( mpGatheringMap->GetAt( objectIdx ) == NULL )
	{
		pGathering = new cGathering( );
		if( pGathering == 0 )
			return 0;

		if( pGathering->GatheringInit( objectIdx, regenIdx, gatheringIdx, mapNumber, posX, posY, direction, height ) == false )
		{
			SAFE_DELETE( pGathering );
			mGatheringIdxGen.DelIdx( objectIdx );
			return 0;
		}

		if( mpGatheringMap->Insert( objectIdx, pGathering ) == false )
		{
			SAFE_DELETE( pGathering );
			mGatheringIdxGen.DelIdx( objectIdx );
			return 0;
		}
	}
	return pGathering;
}

// RemoveGathering Method
bool cObjectManager::RemoveGathering( unsigned long objectIdx )
{
	cGathering* pGathering = (cGathering*)mpGatheringMap->GetAt( objectIdx );
	if ( pGathering != NULL )
	{
		mGatheringIdxGen.DelIdx( objectIdx );
		mpGatheringMap->Erase( objectIdx );
		SAFE_DELETE( pGathering );
		return true;
	}
	return false;
}

void cObjectManager::InsertDeleteGathering( unsigned long objectIdx )
{
	mDelGathering.PushBack( objectIdx );
}

// AddUserPortal Method
cUserPortal* cObjectManager::AddUserPortal( unsigned long mapChangePos, unsigned long creatorIdx, unsigned short mapNumber, float posX, float posY )
{
	unsigned long objectIdx = mUserPortalIdxGen.GeneratIdx();

	cUserPortal* pUserPortal = NULL;

	if ( mpUserPortalMap->GetAt( objectIdx ) == NULL )
	{
		pUserPortal = new cUserPortal( );
		if( pUserPortal == 0 )
			return 0;

		if( pUserPortal->InitUserPortal( objectIdx, creatorIdx, mapChangePos, mapNumber, posX, posY ) == false )
		{
			SAFE_DELETE( pUserPortal );
			return 0;
		}

		if( mpUserPortalMap->Insert( objectIdx, pUserPortal ) == false )
		{
			SAFE_DELETE( pUserPortal );
			return 0;
		}
	}
	return pUserPortal;
}

// RemoveUserPortal Method
bool cObjectManager::RemoveUserPortal( unsigned long objectIdx )
{
	cUserPortal* pUserPortal = (cUserPortal*)mpUserPortalMap->GetAt( objectIdx );
	if ( pUserPortal != NULL )
	{
		mpUserPortalMap->Erase( objectIdx );
		SAFE_DELETE( pUserPortal );
		return true;
	}
	return false;
}

void cObjectManager::InsertDeleteUserPortal( unsigned long objectIdx )
{
	mDelUserPortal.PushBack( objectIdx );
}

// SendMsgMonsterModelNumberToPlayer Method
void cObjectManager::SendMsgMonsterModelNumberToPlayer( ULONG_PTR socketContext, unsigned long mapNum )
{
	if ( !mpMonsterMap ) assert(NULL);

	tHashSet<unsigned long>* pHashSet = NULL;
	unsigned long            classIdx;

	/// 070622 PKH 맵번호에 해당하는 몬스터 모델 넘버set을 읽어옴
	pHashSet = MONSTERSCRIPT->GetMonsterModelNumber( mapNum );

	HANDLE           handle  = NULL;
	MSG_MODELNUMBER* sendMsg = (MSG_MODELNUMBER*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_MONSTER, NM_MONSTER_MODELNUMBER_RES );
	unsigned long    length  = 0;

	sendMsg->mCount = 0;

	if ( pHashSet != NULL )
	{
		tHashSet<unsigned long>::cIterator start = pHashSet->Begin( );
		tHashSet<unsigned long>::cIterator end   = pHashSet->End( );

		while ( start != end )
		{
			classIdx = (*start);

			if ( classIdx )
			{
				/// 070622 PKH 모델정보 로드에 필요한 idx를 담는다.
				sendMsg->mClassIdx[sendMsg->mCount] = classIdx;
				++(sendMsg->mCount);
			}
			else
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cObjectManager::SendMsgMonsterModelNumberToPlayer modelNumber[%d]==0", (*start) );
				return;
			}

			start++;
		}
	}

	length  = sizeof(MSG_MODELNUMBER)-sizeof(sendMsg->mClassIdx);
	length += sendMsg->mCount * sizeof(sendMsg->mClassIdx);

	NETWORK2->SendMsgRoot( handle, length );
}

// SendMsgNpcModelNumberToPlayer Method
void cObjectManager::SendMsgNpcModelNumberToPlayer( ULONG_PTR socketContext, unsigned long mapNum )
{
	if ( !mpNpcMap ) assert(NULL);

	tHashSet<unsigned long>* pHashSet = NULL;
	unsigned long            classIdx;

	/// 070622 PKH 맵번호에 해당하는 Npc 모델 넘버set을 읽어옴
	pHashSet = NPCSCRIPT->GetNpcModelNumber( mapNum );

	HANDLE           handle  = NULL;
	MSG_MODELNUMBER* sendMsg = (MSG_MODELNUMBER*)NETWORK2->GetMsgRoot( &handle, (PerSocketContext*)socketContext, NM_NPC, NM_NPC_MODELNUMBER_RES );
	unsigned long    length  = 0;

	sendMsg->mCount = 0;

	if ( pHashSet != NULL )
	{
		tHashSet<unsigned long>::cIterator start = pHashSet->Begin( );
		tHashSet<unsigned long>::cIterator end   = pHashSet->End( );

		while ( start != end )
		{
			classIdx = (*start);

			if ( classIdx )
			{
				/// 070622 PKH 모델정보 로드에 필요한 idx를 넣는다.
				sendMsg->mClassIdx[sendMsg->mCount] = classIdx;
				sendMsg->mCount++;
			}
			else
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cObjectManager::SendMsgNpcModelNumberToPlayer modelNumber[%d]==0", (*start) );
				return;
			}

			start++;
		}
	}

	length  = sizeof(MSG_MODELNUMBER)-sizeof(sendMsg->mClassIdx);
	length += sendMsg->mCount * sizeof(sendMsg->mClassIdx);

	NETWORK2->SendMsgRoot( handle, length );
}

// GetObject Method
cBaseObject* cObjectManager::GetObject(unsigned char objectType, unsigned long idx)
{
	switch( objectType )
	{
	case eOBJECTTYPE_PLAYER:
	case eOBJECTTYPE_HERO:
		return GetPlayer( idx );

	case eOBJECTTYPE_MONSTER:
		return GetMonster( idx );

	case eOBJECTTYPE_NPC:
		return GetNpc( idx );

	case eOBJECTTYPE_TOTEM:
		return GetTotem( idx );

	case eOBJECTTYPE_GATHERING:
		return GetGathering( idx );


	default:
		return NULL;
	}
}

// Process Method
void cObjectManager::Process(unsigned long elapsedTime, unsigned long accumTime)
{
	for( unsigned long i = 0 ; i < mDelMonsterMapOnly.GetSize() ; ++i )
	{
		mpMonsterMap->Erase( mDelMonsterMapOnly[i] );
	}
	mDelMonsterMapOnly.Clear();

	/*-- PKH mpDeathMonsterMap Object 처리. --*/
	try {

		cMonsterMap::cIterator iter = mpDeathMonsterMap->Begin();
		cMonsterMap::cIterator end = mpDeathMonsterMap->End();
		while( iter != end )
		{
			cMonster* pMonster = (cMonster*)(*iter++).mSecond;

			if( !pMonster )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cObjectManager::Process mpDeathMonsterMap pMonster == NULL" );
				continue;
			}

			/// 죽어있는 몬스터 프로세스
			if( pMonster->DeathUpdate() == true )
				mpDeathMonsterMap->Erase( pMonster->GetObjectID() );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cMonster::Update Method. Throwing 'cObjectManager::Process' mpDeathMonsterMap exception." );
		throw;
	}

	/*-- PKH Monster Object 처리. --*/
	try {
		for( cMonsterMap::cIterator iter = mpMonsterMap->Begin(); iter != mpMonsterMap->End() ; ++iter )
		{
			cMonster* pMonster = (cMonster*)(*iter).mSecond;

			if( !pMonster )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("cObjectManager::Process mpMonsterMap pMonster == NULL" );
				continue;
			}
	
			// 몬스터 상태 업데이트
			eACTION_CHANGE actionstate = pMonster->Update( elapsedTime, accumTime  );
			eACTION_ID actionid = pMonster->GetActionID();

			MonJobProcess( pMonster, actionid, actionstate );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cMonster::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}


	/*-- 061208 PKH 클라이언트&서버 플레이어 이동속도 동기화 --*/
	try {
		cPlayerMap::cIterator playerBegin = mpPlayerMap->Begin();
		cPlayerMap::cIterator playerEnd   = mpPlayerMap->End();
		cPlayer*              player;

		// 플레이어 정보 처리
		while ( playerBegin != playerEnd )
		{
			player = (*playerBegin++).mSecond;
			player->Update( elapsedTime, accumTime );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cPlayer::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}

	/*-- Begin Item Object 처리. --*/
	try {
		cItemMap::cIterator itemBegin = mpItemMap->Begin( );
		cItemMap::cIterator itemEnd   = mpItemMap->End( );
		cItem*              item;
		unsigned long       itemIdx;

		// 아이템 관리.
		while ( itemBegin != itemEnd )
		{
			item    = (cItem*)(*itemBegin++).mSecond;
			itemIdx = item->GetObjectID( );

			if ( !item->Update( elapsedTime, accumTime ) )
			{
				if ( item->IsGetAuto( ) != true )
				{
					GRIDMANAGER->RemoveItem( item );
				}
				RemoveItem( itemIdx );
			}
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cItem::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}

	/*-- PKH Totem Object 처리. --*/
	try {
		cTotemMap::cIterator totemBegin = mpTotemMap->Begin();
		cTotemMap::cIterator totemEnd = mpTotemMap->End();
		cTotem* pTotem;

		// 토템 정보 처리
		while ( totemBegin != totemEnd )
		{
			pTotem = (*totemBegin++).mSecond;
			pTotem->ProcessTotem( elapsedTime, accumTime );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cTotem::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}

	/*-- PKH Gathering Object 처리. --*/
	try {
		for( cPointHashMap::cIterator iter = mpGatheringMap->Begin(); iter != mpGatheringMap->End() ; ++iter )
		{
			cGathering* pGathering = (cGathering*)(*iter).mSecond;

			if( !pGathering )
			{
				NETWORK2->PostServerEvent("cObjectManager::Process pGathering  == NULL" );
				continue;
			}

			// 채집 상태 업데이트
			pGathering->Update( elapsedTime, accumTime  );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cGathering::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}

	/*-- PKH UserPortal Object 처리. --*/
	try {
		cPointHashMap::cIterator userPortalBegin = mpUserPortalMap->Begin();
		cPointHashMap::cIterator userPortalEnd = mpUserPortalMap->End();
		cUserPortal* pUserPortal;

		// 토템 정보 처리
		while ( userPortalBegin != userPortalEnd )
		{
			pUserPortal = (cUserPortal*)(*userPortalBegin++).mSecond;
			pUserPortal->ProcessUserPortal( elapsedTime, accumTime );
		}
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In cUserPortal::Update Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}

	/*-- Remove Objects. --*/

	try {
		for( unsigned long i = 0 ; mDelMonster.GetSize() > i ; ++i )
		{
			RemoveMonster( mDelMonster[i] );
		}
		mDelMonster.Clear();
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In RemoveMonster Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}
	try {
		for( unsigned long i = 0 ; mDelTotem.GetSize() > i ; ++i )
		{
			cTotem* pTotem = GetTotem( mDelTotem[i] );
			if( pTotem != NULL )
			{
				GRIDMANAGER->RemoveTotem( pTotem );
			}
			RemoveTotem( mDelTotem[i] );
		}
		mDelTotem.Clear();
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In RemoveTotem Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}
	try {
		for( unsigned long i = 0 ; mDelGathering.GetSize() > i ; ++i )
		{
			RemoveGathering( mDelGathering[i] );
		}
		mDelGathering.Clear();
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In RemoveGathering Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}
	try {
		for( unsigned long i = 0 ; mDelUserPortal.GetSize() > i ; ++i )
		{
			cUserPortal* pUserPortal = GetUserPortal( mDelUserPortal[i] );
			if( pUserPortal != NULL )
			{
				GRIDMANAGER->RemoveUserPortal( pUserPortal );
			}
			RemoveUserPortal( mDelUserPortal[i] );
		}
		mDelUserPortal.Clear();
	} catch ( ... ) {
		NETWORK2->PostServerEvent( "In RemoveUserPortal Method. Throwing 'cObjectManager::Process' exception." );
		throw;
	}
}

// LoadNpcInfo Method
void cObjectManager::LoadNpcInfo( )
{
	typedef const tPointerHashMap<unsigned long, sNPCRegen*> cNpcRegenMap;
	cNpcRegenMap* pNpcRegenMap = NPCSCRIPT->GetNPCRegenMap( 0 );

	cNpcRegenMap::cConstIterator begin = pNpcRegenMap->Begin( );
	cNpcRegenMap::cConstIterator end   = pNpcRegenMap->End( );

	cNpc* pNpc = NULL;
	sNPCRegen* pNpcRegen = NULL;
	unsigned long npcIdx;
	while( begin != end )
	{
		npcIdx = (*begin).mFirst;
		pNpcRegen = (*begin).mSecond;
		if( pNpcRegen == NULL )
		{
			NETWORK2->PostServerEvent("cObjectManager::LoadNpcInfo pNpcRegenScript[%d] == NULL", npcIdx );
			return;
		}

		pNpc = AddNpc( npcIdx, pNpcRegen->mNpcClassIdx, pNpcRegen->mMapNumber, pNpcRegen->mPosX, pNpcRegen->mPosY, pNpcRegen->mDirection );	
		GRIDMANAGER->AddNpc( pNpc );

		++begin;
	}
}

// PlayerInCenterPoint Method
//void cObjectManager::PlayerInCenterPoint( unsigned short centerMapNum, NiPoint3 centerPoint, float range, tArray<sObject>* targetAry, bool /*applyHP*/, unsigned char maxTargetCnt )
//{
//	cRangeCheck rangeCheck;
//	NiPoint3    targetPoint;
//
//	cPlayerMap::cIterator playerStart = mpPlayerMap->Begin( );
//	cPlayerMap::cIterator playerEnd   = mpPlayerMap->End( );
//	cPlayer*              pPlayer     = NULL;
//
//	sObject target;
//	unsigned char targetCnt = 0;
//
//	while ( playerStart != playerEnd )
//	{
//		pPlayer     = (*playerStart++).mSecond;
//		if( pPlayer == NULL || pPlayer->GetStateDie() == true || centerMapNum != pPlayer->GetMapNumber() )
//			continue;
//
//		targetPoint.x = pPlayer->GetXPos( );
//		targetPoint.y = pPlayer->GetYPos( );
//		targetPoint.z = pPlayer->Height( );
//
//		float tempRange = ObjectSizeRange( NULL, pPlayer, range );
//
//		rangeCheck.SetRadius( tempRange );
//
//		/// 플레이어가 범위 안에 들어왔는지 체크
//		if ( rangeCheck.IsRange( centerPoint, targetPoint ) )
//		{
//			target.index = pPlayer->GetObjectID( );
//			target.type = eOBJECTTYPE_PLAYER;
//			targetAry->PushBack( target );
//			++targetCnt;
//
//			if( targetCnt >= maxTargetCnt )
//				return;
//		}
//	}
//}


//
//void cObjectManager::PlayerInCenterPoint( unsigned short centerMapNum, NiPoint3		centerPoint,
//										 float			range,
//										 tArray<sMultiTarget>* targetAry,
//										 unsigned long	attackerIdx,
//										 bool /*applyHP*/,
//										 unsigned char maxTargetCnt )
//{
//	cRangeCheck rangeCheck;
//	NiPoint3    targetPoint;
//
//	cPlayerMap::cIterator playerStart = mpPlayerMap->Begin( );
//	cPlayerMap::cIterator playerEnd   = mpPlayerMap->End( );
//	cPlayer*              pPlayer     = NULL;
//
//	sObject target;
//	unsigned char targetCnt = 0;
//
//	while ( playerStart != playerEnd )
//	{
//		pPlayer     = (*playerStart++).mSecond;
//		if( pPlayer == NULL || pPlayer->GetStateDie() == true || centerMapNum != pPlayer->GetMapNumber() )
//			continue;
//
//		/// 자신 제외
//		if( attackerIdx == pPlayer->GetObjectID() )
//			continue;
//
//		/// 결투 대상 확인
//		if( pPlayer->IsAttack( attackerIdx ) == false )
//			continue;
//
//		targetPoint.x = pPlayer->GetXPos( );
//		targetPoint.y = pPlayer->GetYPos( );
//		targetPoint.z = pPlayer->Height( );
//
//		float tempRange = ObjectSizeRange( NULL, pPlayer, range );
//
//		rangeCheck.SetRadius( tempRange );
//
//		/// 플레이어가 범위 안에 들어왔는지 체크
//		if ( rangeCheck.IsRange( centerPoint, targetPoint ) )
//		{
//			target.index = pPlayer->GetObjectID( );
//			target.type = eOBJECTTYPE_PLAYER;
//			sMultiTarget multiTarget;
//			multiTarget.mTarget = target;
//			//multiTarget.mApplyHP = applyHP;
//			targetAry->PushBack( multiTarget );
//			++targetCnt;
//
//			if( targetCnt >= maxTargetCnt )
//				return;
//		}
//	}
//}



//void cObjectManager::MonsterInCenterPoint( unsigned short centerMapNum, NiPoint3 centerPoint, float range, tArray<sObject>* targetAry, bool /*applyHP*/, unsigned char maxTargetCnt )
//{
//	cRangeCheck rangeCheck;
//	NiPoint3    targetPoint;
//
//	sObject target;
//	unsigned char targetCnt = 0;
//
//	cMonsterMap::cIterator monsterStart = mpMonsterMap->Begin( );
//	cMonsterMap::cIterator monsterEnd   = mpMonsterMap->End( );
//	cMonster*              pMonster     = NULL;
//
//	while ( monsterStart != monsterEnd )
//	{
//		pMonster    = (cMonster*)(*monsterStart++).mSecond;
//		if( pMonster == NULL || pMonster->GetStateDie() == true || centerMapNum != pMonster->GetMapNumber() )
//			continue;
//
//		targetPoint.x = pMonster->GetXPos( );
//		targetPoint.y = pMonster->GetYPos( );
//		targetPoint.z = pMonster->Height( );
//
//		float tempRange = ObjectSizeRange( NULL, pMonster, range );
//
//		rangeCheck.SetRadius( tempRange );
//
//		if ( rangeCheck.IsRange( centerPoint, targetPoint ) )
//		{
//			target.index = pMonster->GetObjectID( );
//			target.type = eOBJECTTYPE_MONSTER;
//			targetAry->PushBack( target );
//			++targetCnt;
//
//			if( targetCnt >= maxTargetCnt )
//				return;
//		}
//	}
//}

//void cObjectManager::MonsterInBox( unsigned short centerMapNum, float direction, NiPoint3 centerPos3, 
//								  float rangeX, float rangeY,  tArray<sMultiTarget>* targetAry, bool applyHP, unsigned char maxTargetCnt )
//{
//	NiPoint2    targetPoint;
//
//	sObject target;
//	unsigned char targetCnt = 0;
//
//	NiPoint2 centerPos;
//	centerPos.x = centerPos3.x;
//	centerPos.y = centerPos3.y;
//
//	cMonsterMap::cIterator monsterStart = mpMonsterMap->Begin( );
//	cMonsterMap::cIterator monsterEnd   = mpMonsterMap->End( );
//	cMonster*              pMonster     = NULL;
//
//	float cos = cosf( direction );
//	float sin = sinf( direction );
//
//	NiPoint2 mRectPos[4];
//
//	mRectPos[0].x = ( ( -rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
//	mRectPos[0].y = ( ( +rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
//	mRectPos[1].x = ( ( -rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
//	mRectPos[1].y = ( ( -rangeY * 0.5f ) * cos - ( -rangeX * 0.5f ) * sin );
//	mRectPos[2].x = ( ( +rangeX * 0.5f ) * cos + ( -rangeY * 0.5f ) * sin );
//	mRectPos[2].y = ( ( -rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
//	mRectPos[3].x = ( ( +rangeX * 0.5f ) * cos + ( +rangeY * 0.5f ) * sin );
//	mRectPos[3].y = ( ( +rangeY * 0.5f ) * cos - ( +rangeX * 0.5f ) * sin );
//
//	mRectPos[0] = mRectPos[0] + centerPos;
//	mRectPos[1] = mRectPos[1] + centerPos;
//	mRectPos[2] = mRectPos[2] + centerPos;
//	mRectPos[3] = mRectPos[3] + centerPos;
//
//	while ( monsterStart != monsterEnd )
//	{
//		pMonster    = (cMonster*)(*monsterStart++).mSecond;
//		if( pMonster == NULL || pMonster->GetStateDie() == true || centerMapNum != pMonster->GetMapNumber() )
//			continue;
//
//		targetPoint.x = pMonster->GetXPos( );
//		targetPoint.y = pMonster->GetYPos( );
//
//		if( STAGESCRIPT->InsidePolygon( mRectPos, 4, targetPoint ) == 1 )
//			continue;
//
//		target.index = pMonster->GetObjectID( );
//		target.type = eOBJECTTYPE_MONSTER;
//		sMultiTarget multiTarget;
//		multiTarget.mTarget = target;
//		multiTarget.mApplyHP = applyHP;
//		targetAry->PushBack( multiTarget );
//
//		++targetCnt;
//		if( targetCnt >= maxTargetCnt )
//			return;
//	}
//}


float cObjectManager::ObjectSizeRange( cBaseObject* object1, cBaseObject* object2, float range )
{
	float object1size = 0.0f;
	float object2size = 0.0f;

	/// 오브젝트 사이즈 만큼의 수정된 거리
	if( object1 != NULL )
		object1size = object1->GetFixedObjectSize();

	if( object2 != NULL )
		object2size = object2->GetFixedObjectSize();

	return range + object1size + object2size;
}


NiPoint2 cObjectManager::CalcFollowPos( unsigned char followPos, NiPoint2 centerPos, float range )
{
	if( followPos >= FOLLOW_POS_MAX )
		return centerPos;

	NiPoint2 dir = FOLLOW_POS_VALUE[followPos];
	dir.Unitize();
	
	if( range > 141.f )
		range = 141.f;

	NiPoint2 followPosTemp = centerPos + dir * range;
	return followPosTemp;
}



unsigned long cObjectManager::IsNpcTalk( unsigned long npcIdx, cPlayer* pPlayer, eNPC_TALK_TYPE npcTalkType, unsigned long value, ULONG_PTR socketContext )
{
	if( pPlayer == NULL )
	{
		NETWORK2->PostServerEvent( "cObjectManager::IsNpcTalk pPlayer == NULL" );
		return ERROR_NPC_OPEN_PLAYERSTATE;
	}

	pPlayer->MoveStop();

	// npc 정보확인
	cNpc* pNpc = GRIDMANAGER->GetNpc( npcIdx );
	if( pNpc == NULL )
	{
		NETWORK2->PostServerEvent("cObjectManager::IsNpcTalk pNpc == NULL[%d,%d,%d,%d,%d]", 
			npcIdx, pPlayer->GetNpcIdx(), pPlayer->GetObjectID(), pPlayer->GetState(), pPlayer->GetStateStop() );
		return ERROR_NPC_OPEN_NPC;
	}

	unsigned long npcClassIdx = pNpc->GetRaceGender();

	if( pPlayer->GetMapNumber() != pNpc->GetMapNumber() )
		return ERROR_NPC_OPEN_RANGE;

	/// 거리 체크
	float talkRange = NPC_VALID_DISTANCE + SYNC_MOVE_RANGE;
	float tempRange = OBJECTMANAGER->ObjectSizeRange( pPlayer, pNpc, talkRange );
	cRangeCheck rangeCheck( tempRange );
	if( rangeCheck.IsNotRange( pPlayer->GetPos(), pNpc->GetPos() ) == true )
		return ERROR_NPC_OPEN_RANGE;


	/// 대화 타입별 조건 체크(공통)
	if( npcTalkType != eNPCTALK_OPEN )
	{
		/// 대화 상대가 틀림
		if( pPlayer->GetNpcIdx() != npcIdx )
		{
			NETWORK2->PostServerEvent("cObjectManager::IsNpcTalk pPlayer->GetNpcIdx() != npcIdx && npcTalkType != eNPCTALK_OPEN[%d,%d,%d,%d,%d]", 
				npcIdx, pPlayer->GetNpcIdx(), pPlayer->GetObjectID(), pPlayer->GetState(), pPlayer->GetStateStop() );
			return ERROR_NPC_OPEN_NPC;
		}

		/// 플레이어가 npc대화 상태가 아님
		if( pPlayer->GetStateStop() != eSTOP_NPCSPEECH )
			return ERROR_NPC_OPEN_PLAYERSTATE;
	}

    if ( pPlayer->IsAutoTrial() == true )
        return ERROR_NPC_OPEN_PLAYERSTATE;

	/// 대화 타입별 체크 & 설정(개별)
	switch( npcTalkType )
	{
		/// 대화 시작
	case eNPCTALK_OPEN:
		{
			if( pPlayer->GetNpcIdx() == npcIdx )
				return ERROR_NPC_OPEN_ALREADY;

			if( pPlayer->GetNpcIdx() != 0 )
			{
				NETWORK2->PostServerEvent("cObjectManager::IsNpcTalk pPlayer->GetNpcIdx() != npcIdx && && npcTalkType == eNPCTALK_OPEN[%d,%d,%d,%d,%d]", 
					npcIdx, pPlayer->GetNpcIdx(), pPlayer->GetObjectID(), pPlayer->GetState(), pPlayer->GetStateStop() );
				return ERROR_NPC_OPEN_NPC;
			}

			/// 2차상태 체크
			if( pPlayer->IsRequestRejection() == true )
			{
				NETWORK2->PostServerEvent("cObjectManager::IsNpcTalk pPlayer->IsRequestRejection() == true[%d,%d,%d,%d,%d,%d]", 
					npcIdx, pPlayer->GetNpcIdx(), pPlayer->GetObjectID(), pPlayer->GetState(), pPlayer->GetStateStop(), pPlayer->GetRequestRejection() );
				return ERROR_NPC_OPEN_PLAYERSTATE;
			}

			/// 플레이어 상태 체크
			if( pPlayer->ChangeState( eOBJECT_STATE_STOP ) == false )
			{
				NETWORK2->PostServerEvent("cObjectManager::IsNpcTalk pPlayer->ChangeState( eOBJECT_STATE_STOP ) == false[%d,%d,%d,%d,%d]", 
					npcIdx, pPlayer->GetNpcIdx(), pPlayer->GetObjectID(), pPlayer->GetState(), pPlayer->GetStateStop() );
				return ERROR_NPC_OPEN_PLAYERSTATE;
			}

			/// 정지 상태 중 npc 대화 상태로 변경
			pPlayer->SetStateStop( eSTOP_NPCSPEECH );

			/// 대화중인 npc인덱스를 설정
			pPlayer->SetNpcIdx( npcIdx );

			sNPCList* info = NPCSCRIPT->GetNPCList( npcClassIdx );
			if( info )
			{
				// 우편함
				if( info->mType == eNPCTYPE_MAIL )
				{
					if( pPlayer->PostOpen( socketContext ) == false )
						NETWORK2->PostServerEvent( "GameIn : failed to send PostOpen()" );
				}
			}
			else
			{
				NETWORK2->PostServerEvent( "GetNPCList : failed to find npc info(%d)", npcClassIdx );
				assert(0);
			}
		}
		break;
		/// 아이템 구매
	case eNPCTALK_ITEMBUY:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_STORE ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_STORE );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
		/// 아이템 판매
	case eNPCTALK_ITEMSELL:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_STORE ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_STORE );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
		/// 창고 이용
	case eNPCTALK_INVENTORY:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_WAREHOUSE ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_WAREHOUSE );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
		/// 퀘스트 추가
	case eNPCTALK_QUESTADD:
		{
			/// 퀘스트번호 유효성 검사
			cQuestDefine* define = QUESTMAN->GetQuestDefine( value );
			if( !define )
				return ERROR_NPC_OPEN_DEALLIST;

			/// 해당 퀘스트와 퀘스트 발생 npc 유효성 검사
			if( define->mGiveNpcIndex > 0 && define->mGiveNpcIndex != npcClassIdx )
				return ERROR_NPC_OPEN_DEALLIST;
		}
		break;
		/// 퀘스트 보상
	case eNPCTALK_QUESTREWARD:
		{
			/// 퀘스트번호 유효성 검사
			cQuestDefine* define = QUESTMAN->GetQuestDefine( value );
			if( !define )
				return ERROR_NPC_OPEN_DEALLIST;

			/// 보상요청한 npc 유효성 검사
			if( define->mTakeNpcIndex != npcClassIdx )
				return ERROR_NPC_OPEN_DEALLIST;
		}
		break;
		/// 길드 생성
	case eNPCTALK_GUILDCREATE:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_GUILD ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_GUILD );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
		/// NPC 점집
	case eNPCTALK_FORTUNETAROT:
		{
			sNpcFuncData* data = NPCSCRIPT->GetNpcFuncData( npcClassIdx );
			if( !data )
				return ERROR_NPC_TAROT_JOIN_FAIL;

			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_FORTUNETAROT ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_FORTUNETAROT );
				return ERROR_NPC_TAROT_JOIN_FAIL;
			}
		}
		break;
	case eNPCTALK_TAROT:
		{
			// 튜토리얼형태 점집
			sNpcFuncData* data = NPCSCRIPT->GetNpcFuncData( npcClassIdx );
			if( !data )
				return ERROR_TAROT_TUTORIAL_SEEKER_OPEN_NPC;

			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_TAROT ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_TAROT );
				return ERROR_TAROT_TUTORIAL_SEEKER_OPEN_NPC;
			}
		}
		break;
	case eNPCTALK_DEAL:
		{
			/// npc에 맞는 기능인지 체크
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_DEAL ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_DEAL );
				return ERROR_NPC_OPEN_DEALLIST;
			}
		}
		break;
	case eNPCTALK_THEME:
		{
			/// npc에 맞는 기능인지 체크
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_THEME ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_THEME );
				return ERROR_NPC_OPEN_DEALLIST;
			}
		}
		break;
	case eNPCTALK_ITEMDISJOINT:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_ITEMDISJOINT ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_ITEMDISJOINT );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
//	case eNPCTALK_ENHANCE_EXCHANGE:
//		{
//			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_ENHANCE_EXCHANGE ) == false )
//			{
//				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_ENHANCE_EXCHANGE );
//				return ERROR_NPC_OPEN_DEALLIST; 
//			}
//		}
//		break;

	case eNPCTALK_EVENT:
		{
			if( NPCSCRIPT->IsHaveNpcFunc( npcClassIdx, NPC_FUNC_EVENT ) == false )
			{
				NETWORK2->PostServerEvent( "IsHaveNpcFunc error(%d,%d)", npcClassIdx, NPC_FUNC_EVENT );
				return ERROR_NPC_OPEN_DEALLIST; 
			}
		}
		break;
	default:
		assert(NULL);
		NETWORK2->PostServerEvent( "IsNpcTalk eNPC_TALK_TYPE error[%d]", npcTalkType );
		break;
	}

	return ERROR_NPC_OPEN_SUCCESS;
}

cPlayer* cObjectManager::GetPlayer( wchar_t* charName )
{
	PerPlayerName* name = mpPlayerNamePool->Search( charName );
	if ( name != NULL )
	{
		return GetPlayer( name->characterIdx );
	}
	return NULL;
}

void cObjectManager::SendNotice( char* msg, unsigned long size )
{
	cPlayerMap::cIterator start = mpPlayerMap->Begin();
	cPlayerMap::cIterator end = mpPlayerMap->End();

	cPlayer* pPlayer;

	while( start != end )
	{
		pPlayer = (*start++).mSecond;
		if( pPlayer == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cObjectManager::SendNotice player == NULL");
			continue;
		}

		if( pPlayer->GetGameIn() == false )
			continue;

		NETWORK2->SendToUser( pPlayer->GetConnectionIdx(), msg, size );
	}
}

/// 치트
void cObjectManager::CheatRemoveItemRange( cBaseObject* obj, float radius )
{
	obj;radius;
}

void cObjectManager::CheatRemoveMonsterRange( cBaseObject* pCenter, float size, bool isCheatKill )
{
	if( pCenter == NULL )
		return;

	cRangeCheck rangeCheck( size );

	cMonster* pMonster = GRIDMANAGER->FindFirstMonster( pCenter );
	/// 각각의 몬스터를 검색함
	while( pMonster != NULL )
	{
		if( pMonster->GetStateDie() == true ) 
		{
			pMonster = GRIDMANAGER->FindNextMonster();
			continue;
		}

		if( pMonster->GetMapNumber() != pCenter->GetMapNumber() )
		{
			pMonster = GRIDMANAGER->FindNextMonster();
			continue;
		}

		if( rangeCheck.IsRange( pCenter->GetPos(), pMonster->GetPos() ) == true )
		{
			if( isCheatKill == true )
				CheatKillMonster( pMonster->GetObjectID(), pCenter->GetObjectID() );
			else
				CheatRemoveMonster( pMonster->GetObjectID() );		
		}

		pMonster = GRIDMANAGER->FindNextMonster();
	}
}


void cObjectManager::CheatRemoveMonster( unsigned long monsterIdx )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster != NULL )
	{
		pMonster->OutsideDestroyTrue();
		pMonster->ActionChange( eACT_DIE );

		MSG_SYN_MONSTER_HP synMsg;
		synMsg.Category			= NM_MONSTER;
		synMsg.Protocol			= NM_MONSTER_HP_SYN;
		synMsg.mMonsterIdx	    = monsterIdx;
		synMsg.mHP				= 0;
		synMsg.mMaxHP			= pMonster->GetMaxHP();
		synMsg.mDie				= true;

		NETWORK2->QuickSendExcept( pMonster, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_HP) );
	}
}

void cObjectManager::CheatKillMonster( unsigned long monsterIdx, unsigned long playerIdx )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster != NULL )
	{
		sObject player = { eOBJECTTYPE_PLAYER, playerIdx };
		if( pMonster->GetfirstAttack() == 0 )
		{
			pMonster->SetFirstAttack( playerIdx );
			pMonster->SetLastAttack( playerIdx );
			pMonster->AddTakeDamage( player, pMonster->GetMaxHP(), 0, eTAKEDAMAGETYPE_NONETARGET );
		}		

		pMonster->ActionChange( eACT_DIE );

		MSG_SYN_MONSTER_HP synMsg;
		synMsg.Category			= NM_MONSTER;
		synMsg.Protocol			= NM_MONSTER_HP_SYN;
		synMsg.mMonsterIdx	    = monsterIdx;
		synMsg.mHP				= 0;
		synMsg.mMaxHP			= pMonster->GetMaxHP();
		synMsg.mDie				= true;

		NETWORK2->QuickSendExcept( pMonster, (char*)&synMsg, sizeof(MSG_SYN_MONSTER_HP) );
	}
}


void cObjectManager::QuestRemoveMonster( unsigned long monsterIdx )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster != NULL )
		pMonster->OutsideDestroy();
}


void cObjectManager::SendPVPNotice( unsigned char leftMinute, unsigned short PvPType, unsigned long aryPos )
{
	sSecionInfo* pInfo = PVPMANAGER->GetSectionInfo( aryPos );
	if( pInfo == NULL )
		return;

	cPlayerMap::cIterator start = mpPlayerMap->Begin();
	cPlayerMap::cIterator end = mpPlayerMap->End();

	cPlayer* pPlayer;

	while( start != end )
	{
		pPlayer = (*start++).mSecond;
		if( pPlayer == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cObjectManager::SendNotice player == NULL");
			continue;
		}

		if( pPlayer->GetGameIn() == false )
			continue;

		if( pPlayer->GetLevel() < pInfo->mMinLevel || pInfo->mMaxLevel < pPlayer->GetLevel () )
			continue;

		HANDLE               handle  = NULL;
		MSG_SYN_PVP_DM_NOTICE* sendMsg = NULL;
		sendMsg = (MSG_SYN_PVP_DM_NOTICE*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_DM_NOTICE_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mPvPType = PvPType;
			sendMsg->mLeftTime = leftMinute;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_DM_NOTICE) );
		}
	}
}


// SendMsgSynPVPJoin Method.
void cObjectManager::SendMsgSynPVPReadyQuestion( unsigned short pvpType, unsigned long aryPos )
{
	sSecionInfo* pInfo = PVPMANAGER->GetSectionInfo( aryPos );
	if( pInfo == NULL )
		return;

	cPlayerMap::cIterator begin = mpPlayerMap->Begin();
	cPlayerMap::cIterator end   = mpPlayerMap->End();
	while ( begin != end )
	{
		cPlayer* pPlayer = (*begin++).mSecond;
		if ( pPlayer == NULL )
			continue;

		if ( pPlayer->GetGameIn( ) == false )
			continue;

		if( pPlayer->CheckPvPJoin() == false )
			continue;

		if( pPlayer->GetForceType() == eFORCETYPE_NONE )
			continue;

		sGameOption* toOption1 = pPlayer->GetOptionData();
		if( toOption1->option1.rejectionPvP == true )
			continue;

		sDMInfo* pDMInfo = PVPSCRIPT->GetDmInfo( pvpType );
		if( pDMInfo == NULL )
			continue;
		sDMBlockInfo* pBlockInfo = pDMInfo->GetBlockInfo( pPlayer->GetLevel() );
		if( pBlockInfo == NULL )
			continue;

		if( pPlayer->GetPvPJoinAryPos() != ULONG_MAX )
			continue;

		if( pPlayer->GetLevel() < pInfo->mMinLevel || pInfo->mMaxLevel < pPlayer->GetLevel () )
			continue;

		HANDLE               handle  = NULL;
		MSG_SYN_PVP_READYQUESTION* sendMsg = NULL;
		sendMsg = (MSG_SYN_PVP_READYQUESTION*)NETWORK2->GetMsgRoot( &handle, pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_READYQUESTION_SYN );
		if ( sendMsg != NULL )
		{
			sendMsg->mPvPType		= pvpType;
			sendMsg->mMinLevel		= pBlockInfo->mLevelMin;
			sendMsg->mMaxLevel		= pBlockInfo->mLevelMax;
			sendMsg->mLeftTime      = PVP_DM_JOIN_TIME;
			//sendMsg->mPvPType		= mapNumber;
			NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_PVP_READYQUESTION) );
		}
	}
}


//void cObjectManager::SendMsgSynPVPReadyClose( unsigned long aryPos )
//{
//	sSecionInfo* pInfo = PVPMANAGER->GetSectionInfo( aryPos );
//	if( pInfo == NULL )
//		return;
//
//	cPlayerMap::cIterator begin = mpPlayerMap->Begin();
//	cPlayerMap::cIterator end   = mpPlayerMap->End();
//	while ( begin != end )
//	{
//		cPlayer* pPlayer = (*begin++).mSecond;
//		if ( pPlayer == NULL )
//			continue;
//
//		if ( pPlayer->GetGameIn( ) == false )
//			continue;
//
//		if( pPlayer->GetLevel() < pInfo->mMinLevel || pInfo->mMaxLevel < pPlayer->GetLevel () )
//			continue;
//
//		if( pPlayer->GetPvPJoinAryPos() == ULONG_MAX )
//			continue;
//
//		NETWORK2->SendMsgRoot( pPlayer->GetConnectionIdx( ), NM_PVP, NM_PVP_READYCLOSE_SYN );
//	}
//}


void cObjectManager::SendTimeEventQuest()
{
	cPlayerMap::cIterator start = mpPlayerMap->Begin();
	cPlayerMap::cIterator end = mpPlayerMap->End();

	cPlayer* player = NULL;

	while( start != end )
	{
		player = (*start++).mSecond;
		if( player == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cObjectManager::SendTimeEventQuest player == NULL");
			continue;
		}

		if( player->GetGameIn() == true )
		{
			player->SendEventQuestList( eQUEST_TIMEEVENT );
		}
	}
}

void cObjectManager::AddGMEvent( sGMEventInfo eventInfo )
{
	cPlayerMap::cIterator start = mpPlayerMap->Begin();
	cPlayerMap::cIterator end = mpPlayerMap->End();

	cPlayer* player = NULL;

	while( start != end )
	{
		player = (*start++).mSecond;
		if( player == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cObjectManager::UpdateGMEvent player == NULL");
			continue;
		}

		player->AddGMEvent( eventInfo );
	}
}

void cObjectManager::DelGMEvent( unsigned long DBIndex, unsigned long influenceIdx )
{
	cPlayerMap::cIterator start = mpPlayerMap->Begin();
	cPlayerMap::cIterator end = mpPlayerMap->End();

	cPlayer* player = NULL;

	while( start != end )
	{
		player = (*start++).mSecond;
		if( player == NULL )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("cObjectManager::DelGMEvent player == NULL");
			continue;
		}

		player->DelGMEvent( DBIndex, influenceIdx );
	}
}
