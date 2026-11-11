#include "stdafx.h"
#include "objectmanager.h"

#include "Protocol.h"

#include "WorldManager.h"
#include "DynamicSceneNode.h"
#include "GatheringSceneNode.h"
#include "DropItemSceneNode.h"
#include "SceneManager.h"
#include "StageManager.h"
#include "GameResourceManager.h"

#include "Signboard.h"

#include "hero.h"
#include "monster.h"
#include "npc.h"
#include "totem.h"

#include "UICursor.h"
#include "GameUIManager.h"
#include "UIManager.h"
#include "RButtonMenuWindow.h"

#include "OptionManager.h"

/// 070105 PKH cheat사용
#include "Lexer.h"
#include "Token.h"
#include "ChatManager.h"

#include "MapPortalSceneNode.h"

#include "Player_Common.h"
///
#include "GatheringScript.h"
#include "Gathering_Common.h"

#include "DuelManager.h"
#include "PVPManager.h"

#include "UIMsgboxEvent.h"

#if (defined(_DEVSYS) || defined(_DEBUG))
	#include "DevSystem.h"
#endif


cObjectManager* cObjectManager::mpObjectManager = NULL;


cObjectManager::cObjectManager()
: mpPlayerMap(0)
, mpMonsterMap(0)
, mpNpcMap(0)
, mpTotemMap(0)
, mpGatheringMap(0)
, mpFPortalMap(0)
{
	mpObjectManager = this;

	mpHero = NULL;

	///
	mPickObject.type = eOBJECTTYPE_NONE;
	mPickObject.index = 0;

	///
	mOverObject.type = eOBJECTTYPE_NONE;
	mOverObject.index = 0;

	mRbuttonObject = 0;

	///
	mOverNode.type = cSceneNode::eNULL;
	mOverNode.indexByManager = 0;

//	mCollidableArray.Reserve(100);

	//mMapSize = 0.0f;
}

cObjectManager::~cObjectManager()
{
	SAFE_DELETE(mpHero);

	Close();
	Exit();
}

bool cObjectManager::Init()
{
	mpPlayerMap = new cPlayerHashMap(1000);
	mpMonsterMap = new cMonsterHashMap(1000);
	mpNpcMap = new cNpcHashMap( 20 );

	mpTotemMap = new cTotemHashMap(500);
	mpGatheringMap = new cGatheringMap(1000);
	mpFPortalMap = new cFPortalHashMap(100);

	return true;
}

void cObjectManager::Open()
{
	mOverNode.type = cSceneNode::eNULL;
	mOverNode.indexByManager = 0;

	mRbuttonObject = 0;
}

void cObjectManager::Close()
{
	mOverNode.type = cSceneNode::eNULL;
	mOverNode.indexByManager = 0;

	if( mpFPortalMap )
		mpFPortalMap->Clear();

	if( mpGatheringMap )
		mpGatheringMap->Clear();

	/// Totem 모두 제거
	if( mpTotemMap )
	{
		cTotemHashMap::cIterator i, end;
		cTotem* totem = NULL;

		i = mpTotemMap->Begin();
		end = mpTotemMap->End();
		for( ; i != end; ++i )
		{
			totem = (cTotem*)(*i).mSecond;
			SAFE_DELETE( totem );
		}
		mpTotemMap->Clear();
	}

	/// Npc 모두 제거
	if( mpNpcMap )
	{
		cNpcHashMap::cIterator i, end;
		cNpc* npc = NULL;

		i = mpNpcMap->Begin();
		end = mpNpcMap->End();
		for( ; i != end; ++i )
		{
			npc = (cNpc*)(*i).mSecond;
			SAFE_DELETE( npc );
		}
		mpNpcMap->Clear();
	}

	/// Monster 모두 제거
	if( mpMonsterMap )
	{
		cMonsterHashMap::cIterator i, end;
		cMonster* monster = NULL;

		i = mpMonsterMap->Begin();
		end = mpMonsterMap->End();
		for( ; i != end; ++i )
		{
			monster = (cMonster*)(*i).mSecond;
			SAFE_DELETE( monster );
		}
		mpMonsterMap->Clear();

	}

	/// Player 모두 제거
	if( mpPlayerMap )
	{
		cPlayerHashMap::cIterator i, end;
		cPlayer* player = NULL;

		i = mpPlayerMap->Begin();
		end = mpPlayerMap->End();
		for( ; i != end; ++i )
		{
			player = (cPlayer*)(*i).mSecond;
			SAFE_DELETE( player );
		}
		mpPlayerMap->Clear();
	}

	mMonsterDieArray.Clear();
	mNpcRemoveArray.Clear();
//	mPortalRemoveArray.Cle
}

void cObjectManager::ClearHeroData()
{ 
	SAFE_DELETE(mpHero); 
}

void cObjectManager::Exit()
{
	SAFE_DELETE(mpFPortalMap);
	SAFE_DELETE(mpGatheringMap);
	SAFE_DELETE(mpTotemMap);
	SAFE_DELETE(mpNpcMap);
	SAFE_DELETE(mpMonsterMap);
	SAFE_DELETE(mpPlayerMap);
}

void cObjectManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpNpcMap->IsEmpty() == false )
	{
		cNpcHashMap::cIterator beginNpc = mpNpcMap->Begin();
		cNpcHashMap::cIterator endNpc = mpNpcMap->End();
		cNpc* npc = NULL;

		for( ; beginNpc != endNpc; ++beginNpc )
		{
			npc = (cNpc*)(*beginNpc).mSecond;
			npc->Update( deltaTime, accumTime );
		}
	}

	if( mNpcRemoveArray.IsEmpty() == false )
	{
		for( unsigned int i = 0 ; mNpcRemoveArray.GetSize() > i ; ++i )
		{
			RemoveNpc( mNpcRemoveArray[i] );
		}
		mNpcRemoveArray.Clear();
	}

	if( mpMonsterMap->IsEmpty() == false )
	{
		cMonsterHashMap::cIterator beginMonster = mpMonsterMap->Begin();
		cMonsterHashMap::cIterator endMonster = mpMonsterMap->End();
		cMonster* monster = NULL;

		for( ; beginMonster != endMonster; ++beginMonster )
		{
			monster = (cMonster*)(*beginMonster).mSecond;
			monster->Update( deltaTime, accumTime );
		}
	}

	/// 061207 PKH 죽은몬스터 삭제 처리
	if( mMonsterDieArray.IsEmpty() == false )
	{
		for( unsigned int i = 0 ; mMonsterDieArray.GetSize() > i ; ++i )
		{
			RemoveMonster( mMonsterDieArray[i] );
		}
		mMonsterDieArray.Clear();
	}

	if( mpPlayerMap->IsEmpty() == false )
	{
		cPlayerHashMap::cIterator beginPlayer = mpPlayerMap->Begin();
		cPlayerHashMap::cIterator endPlayer = mpPlayerMap->End();
		cPlayer* player = NULL;

		for( ; beginPlayer != endPlayer; ++beginPlayer )
		{
			player = (cPlayer*)(*beginPlayer).mSecond;
			player->Update( deltaTime, accumTime );
		}
	}

	if( mpHero )
		mpHero->Update( deltaTime, accumTime );
}

void cObjectManager::ClearPickObject()
{
	mPickObject.type = eOBJECTTYPE_NONE;
	mPickObject.index = 0;
}

void cObjectManager::HeroGoto( int mouseX, int mouseY )
{
	if( HERO->GetState() == eOBJECT_STATE_DIE )
		return;

	if( HERO->GetState() == eOBJECT_STATE_PUSHPULL || HERO->GetState() == ePLAYER_STATE_RUSH )
		return;

	if( HERO->GetState() == eOBJECT_STATE_STOP )
	{
		unsigned int stopflag = HERO->GetStopFlag();
		if( !(stopflag == eSTOP_ENHANCED || stopflag == eSTOP_ITEMMIX || stopflag == eSTOP_NPCSPEECH) )
			return;
	}

	if( HERO->IsJumping() == true )
		return;

	NiPoint3 pos;
	if( WORLDMAN->Pick( &pos, mouseX, mouseY ) == false )
		return;

	if( pos.x < 0 || pos.y < 0 )
	{
		assert(0);
		return;
	}

	/// 이동..
	if( mpHero )
	{
		mpHero->ClearMoveFlag();
		mpHero->StopCountinueMove();
		mpHero->ResetMovePath( NiPoint2(mpHero->GetXPos(), mpHero->GetYPos() ), NiPoint2(pos.x, pos.y) );
	}
}

void cObjectManager::MovePlayer( unsigned long objectIdx, float x, float y, long moveSpeed )
{
	cPlayer* player = (cPlayer*)mpPlayerMap->GetAt( objectIdx );
	if( !player )	
	{
		assert(0);
		return;
	}
	if( player == HERO )
	{
		assert(0);
		return;
	}

	if( player->GetState() == eOBJECT_STATE_DIE )
		return;

	/// 설정 변경
	player->ClearActionMoveRange();
	player->SetTargetObject( eOBJECTTYPE_NONE, 0 );

	player->SetMoveSpeed( moveSpeed );
	player->Move( x, y );
}

void cObjectManager::ActionMovePlayer( MSGROOT* pMsg )
{
	assert(pMsg);
	MSG_SYN_ACTIONMOVE* pmsg = (MSG_SYN_ACTIONMOVE*)pMsg;

	cPlayer* player = (cPlayer*)mpPlayerMap->GetAt( pmsg->characterIdx );
	if( player == 0 )
	{
		assert(0);
		return;
	}
	if( player == HERO )
	{
		assert(0);
		return;
	}

	/// 범위 검사를 위한 변수 셋팅.
	player->SetActionMove( pmsg->targetX, pmsg->targetY, pmsg->range );
	player->SetTargetObject( pmsg->targetInfo.type, pmsg->targetInfo.index );

	/// 이동.
	player->SetMoveSpeed( pmsg->moveSpeed );
	player->Move( pmsg->destX, pmsg->destY );
}

void cObjectManager::RemovePlayer( unsigned long objectIdx )
{
	cPlayer* player = (cPlayer*)mpPlayerMap->GetAt( objectIdx );
	if( !player )	
	{
		assert(0);
		return;
	}

	if( mpHero->GetTargetObject() == player )
	{
		mpHero->SetTargetObject( eOBJECTTYPE_NONE, 0 );
	}

	mpPlayerMap->Erase( objectIdx );
	/// 주의 : 소멸자에서 관리자에 등록된 그래픽 데이터 삭제 한다.
	SAFE_DELETE(player);
}

void cObjectManager::AddNpc( sNpcData* pNpcInfo )
{
	assert( pNpcInfo);

	cNpc* pNpc = new cNpc;
	if( mpNpcMap->Insert( pNpcInfo->mNpcIdx, pNpc )  == false )
	{
		assert(0);
		return;
	}

	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3( pNpcInfo->mPosX, pNpcInfo->mPosY, 0.0f );
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, pNpcInfo->mDirection );

	if( pNpc->Create( pNpcInfo->mNpcIdx, pNpcInfo->mNpcClassIdx, pos, rot ) == false )
	{
		assert(NULL);
		return;
	}
}

void cObjectManager::RemoveNpc( unsigned long objectIdx )
{
	cNpc* pNpc = (cNpc*)mpNpcMap->GetAt( objectIdx );
	if( !pNpc )	
	{
		assert(0);
		return;
	}

	if( mpHero->GetTargetObject() == pNpc )
	{
		mpHero->SetTargetObject( eOBJECTTYPE_NONE, 0 );
	}

	mpNpcMap->Erase( objectIdx );
	SAFE_DELETE(pNpc);
}

void cObjectManager::FadeOutNpcAll()
{
	/// npc fade out
	if( mpNpcMap )
	{
		cNpcHashMap::cIterator i, end;
		cNpc* npc = NULL;

		i = mpNpcMap->Begin();
		end = mpNpcMap->End();
		for( ; i != end; ++i )
		{
			npc = (cNpc*)(*i).mSecond;
			npc->FadeOut();
		}
	}

	/// hero 상태 체크 및 타겟 체크
	if( mpHero )
	{
		const sObject* info = mpHero->GetTargetInfo();
		if( info && info->type == eOBJECTTYPE_NPC )
			mpHero->SetTargetObject( eOBJECTTYPE_NONE, 0 );

		if( mpHero->GetState() == eOBJECT_STATE_STOP )
		{
			if( mpHero->GetStopFlag() == eSTOP_NPCSPEECH )
			{
				/// npc 대화 상태인 경우 해제..
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_PVP_REMOVE_NPC, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 844 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
		}
	}
}

bool cObjectManager::AddMonster( sMonsterData* pMonstInfo, bool isRegen )
{
	assert( pMonstInfo );

	///// 061026 PKH 몬스터 인덱스
	unsigned long objectIdx =  pMonstInfo->mMonsterIdx;

	cMonster* pMonster = new cMonster;
	if( mpMonsterMap->GetAt( objectIdx ) != NULL )
	{
		OBJECTMANAGER->RemoveMonster( objectIdx );
	}

	if( mpMonsterMap->Insert( objectIdx, pMonster ) == false )
	{
		assert(0);
		return false;
	}

	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3((float)pMonstInfo->mPosX, (float)pMonstInfo->mPosY, 0.0f);
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, pMonstInfo->mDirection );

	if( pMonster->Create( pMonstInfo, pos, rot ) == false )
	{
		assert(NULL);
		return false;
	}

	if( isRegen == true )
		pMonster->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_Mon_Regen_01.nif", 0 );

	return true;
}



void cObjectManager::RemoveMonster( unsigned long objectIdx )
{
	cMonster* pMonster = (cMonster*)mpMonsterMap->GetAt( objectIdx );
	if( !pMonster )	
	{
		assert(0);
		return;
	}

	if( mpHero->GetTargetObject() == pMonster )
	{
		mpHero->SetTargetObject( eOBJECTTYPE_NONE, 0 );
	}

	mpMonsterMap->Erase( objectIdx );
	SAFE_DELETE(pMonster);
}

void cObjectManager::AddTotem( unsigned long uIdx, unsigned long totemInfoIdx, NiPoint3 totemPos, float dir )
{
	///
	if( mpTotemMap->GetAt( uIdx ) != NULL )
	{
		assert(0);
		return;
	}

	NiPoint3 pos = NiPoint3((float)totemPos.x, (float)totemPos.y, 0.0f);
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
	{
		assert(0);
	}

	cTotem* pTotem = new cTotem;
	if( pTotem->Init( totemInfoIdx, pos, dir ) == false )
	{
		delete pTotem;
		return;
	}

	if( mpTotemMap->Insert( uIdx, pTotem ) == false )
	{
		assert(0);
		delete pTotem;
		return;
	}
}

void cObjectManager::RemoveTotem( unsigned long uIdx )
{
	///
	cTotem* pTotem = (cTotem*)mpTotemMap->GetAt( uIdx );
	if( !pTotem )	
	{
		assert(0);
		return;
	}

	mpTotemMap->Erase( uIdx );
	SAFE_DELETE(pTotem);
}

void cObjectManager::AddGathering( unsigned long uIdx, unsigned long gatheringIdx, NiPoint3 genPos, float dir, unsigned char teamType )
{
	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( gatheringIdx );
	if( info == 0 )
	{
		assert(0);
		return;
	}

	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, dir );

	cGatheringSceneNodeParam param;
	param.mObjectIdx = uIdx;
	param.mClassIdx = gatheringIdx;
	param.mTranslate = genPos;
	param.mRotate = rot;
	param.mPathName.Format( "./data/monster/%s", info->mFileName.Cstr() );
	param.mTeamType = teamType;

	cSceneNode* node = SCENEMAN->CreateGathering( param );
	if( node == 0 )
	{
		assert(0);
		return;
	}

	if( mpGatheringMap->Insert( uIdx, node->GetIndexByManger() ) == false )
	{
		assert(0);
		return;
	}
}

NiPoint3 cObjectManager::GetGatherPos( unsigned long idx )
{
	unsigned long nodeIdx = mpGatheringMap->GetAt( idx );
	cGatheringSceneNode* node = SCENEMAN->GetGatheringSceneNode( nodeIdx );
	if( node )
		return node->GetWorldTranslate();

	return NiPoint3::ZERO;
}

cSceneNode*	cObjectManager::GetGatherSceneNode( unsigned long idx )
{
	unsigned long nodeIdx = mpGatheringMap->GetAt( idx );
	return SCENEMAN->GetGatheringSceneNode( nodeIdx );
}

void cObjectManager::RemoveGathering( unsigned long idx )
{
	unsigned long nodeIdx = mpGatheringMap->GetAt( idx );
	cGatheringSceneNode* node = SCENEMAN->GetGatheringSceneNode( nodeIdx );
	if( node )
		node->Remove();

	mpGatheringMap->Erase( idx );
}

void cObjectManager::AddUserPortal( unsigned long uIdx, unsigned long ownerIdx, unsigned long posScriptIdx, NiPoint3 genPos )
{
	unsigned long nodeIdx = mpFPortalMap->GetAt( uIdx );
	if( nodeIdx != 0 )
		return;

	cSceneNode* node = SCENEMAN->CreateUserPortal( uIdx, ownerIdx, posScriptIdx, genPos );
	if( node == 0 )
	{
		assert(0);
		return;
	}

	if( mpFPortalMap->Insert( uIdx, node->GetIndexByManger() ) == false )
	{
		assert(0);
		return;
	}
}

void cObjectManager::RemoveUserPortal( unsigned long uIdx )
{
	unsigned long nodeIdx = mpFPortalMap->GetAt( uIdx );
	cSceneNode* node = SCENEMAN->GetManagedStaticNode( cSceneNode::eMAPPORTAL, nodeIdx );
	if( node )
		SCENEMAN->DestroyMapPortalNode( node );

	mpFPortalMap->Erase( uIdx );
}

void cObjectManager::MonsterGoto( int idx, NiPoint2* moveArray, unsigned long count,
								 unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType,
								 float range, sObject* pTarget )
{
	cMonster* monster = GetMonster(idx);
	if( !monster )
	{
//		assert(NULL);
		return;
	}

	if( monster->GetState() == eOBJECT_STATE_DIE )
		return;


	//cPathNodeSet* pathNodeSet = monster->GetPathNodeSet();
	//if( !pathNodeSet )
	//{
	//	assert(NULL);
	//	return;
	//}

	monster->SetMoveSpeed( moveSpeed, moveSpeedType );
	//pathNodeSet->SetPathArray( movearry, count );
	monster->SetPathArray( moveArray, count );
	monster->SetMoveStopRange( range );

	/*// 노드 포인트를 가지고 실제 좌표를 얻어낸다.
	NiPoint2 position = pathNodeSet->GetNextPath();
	monster->SetMoveStopRange( range );

	/// 길찾기 최종 경로를 기록해둔다.
	monster->SetLastGoto( lastXPos, lastYPos );
	if( pathNodeSet->IsFinalPath() )
	{
		monster->Move( lastXPos, lastYPos );
	}
	else
	{
		/// 길찾기 경로대로 움직인다.
		monster->Move( position.x , position.y );
	}
	*/

	/// 타겟 설정
	if( pTarget == NULL )	
	{ 
		monster->SetTargetObject( eOBJECTTYPE_NONE, 0 ); 
	}
	else
	{ 
		monster->SetTargetObject( pTarget->type, pTarget->index ); 
	}
}

unsigned int cObjectManager::ObjectPicking( int mouseX, int mouseY, cBaseObject** out )
{
	if( mPickObject.type != eOBJECTTYPE_NONE )
	{
		cBaseObject* obj = GetObject( &mPickObject);
		if( obj )
		{
			*out = obj;
			return mPickObject.type;
		}
	}

	///
	mCollidableArray.Clear();
	if( SCENEMAN->PickDynamics(&mCollidableArray, mouseX, mouseY, true) == false )
	{
		mPickObject.index = 0;
		mPickObject.type = eOBJECTTYPE_NONE;
		*out = 0;
	}
	else
	{
		cDynamicSceneNode* n = (cDynamicSceneNode*)mCollidableArray[0];
		cBaseObject* pObject =  0;
		if( n && n->IsEnableVisibleArray() )
		{
			pObject = n->GetGameObject();
		}

		if( pObject )
		{
			if( pObject->GetObjectType() == eOBJECTTYPE_MONSTER && pObject->IsDie() == true  )
			{
				mPickObject.index = 0;
				mPickObject.type = eOBJECTTYPE_NONE;
				*out = 0;
			}
			else
			{
				*out = pObject;

				/// over object
				mPickObject.index = pObject->GetObjectID();
				mPickObject.type = pObject->GetObjectType();

				return mPickObject.type;
			}
		}
		else
		{
			mPickObject.index = 0;
			mPickObject.type = eOBJECTTYPE_NONE;
			*out = 0;
		}
	}

	return eOBJECTTYPE_NONE;
}

cManagedStaticSceneNode* cObjectManager::ManagedStaticPicking( int mouseX, int mouseY )
{
	mCollidableArray.Clear();
	if( SCENEMAN->PickManagedStatics( &mCollidableArray, mouseX, mouseY, true ) == true )
	{
		cManagedStaticSceneNode* n = (cManagedStaticSceneNode*)mCollidableArray[0];
		return n;
	}
	return 0;
}

cMapPortalSceneNode* cObjectManager::MapPotalPicking()
{
	if( mpHero == 0 )
		return 0;

	NiPoint3 origin = mpHero->GetPos();
	origin.z += 300.0f;
	NiPoint3 dir = mpHero->GetDirection();

	mCollidableArray.Clear();
	if( SCENEMAN->PickMapportal( &mCollidableArray, cRay(origin, dir), 200.0f, true ) == true )
	{
		cMapPortalSceneNode* n = (cMapPortalSceneNode*)mCollidableArray[0];
		if( n )
			return n;
	}
	return 0;
}

void cObjectManager::AddHeroFromServerMsg( MSGROOT* pMsg )
{
	SAFE_DELETE( mpHero );

	mpHero = new cHero;
	if( mpHero == 0 )
	{
		assert(0);
		return;
	}


	MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;
	NiPoint3 pos = NiPoint3( (float)pmsg->XPos, (float)pmsg->YPos, 0.0f );

	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0.0f, 0.0f, pmsg->RotAngle );

	mpHero->Create( &pmsg->PlayerInfo, &pmsg->WearInfo, &pmsg->WeaponInfo, &pmsg->Exrinfo, pos, rot );
	mpHero->SetMapNum( pmsg->MapNum );
	mpHero->SetMapMode( pmsg->MapMode );
}

void cObjectManager::AddPlayerFromServerMsg( MSGROOT* pMsg, bool showEffect )
{
	MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;

	cPlayer* player = new cPlayer;
	if( mpPlayerMap->Insert( pmsg->PlayerInfo.CharacterIdx, player ) == false )
	{
		assert(0);
		return;
	}

	sPlayerInfo baseInfo = pmsg->PlayerInfo;
	sPlayerExrInfo exrInfo = pmsg->Exrinfo;

	NiPoint3 pos = NiPoint3( pmsg->XPos, pmsg->YPos, 0.0f );

	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0.0f, 0.0f, pmsg->RotAngle );

	if( player->Create( &baseInfo, &pmsg->WearInfo, &pmsg->WeaponInfo, &exrInfo, pos, rot ) == false )
	{
		assert(0);
		RemovePlayer( pmsg->PlayerInfo.CharacterIdx );
		return;
	}

	/// 진입 이펙트
	if( showEffect == true )
	{
		player->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_PC_LogIn_01.nif", 0 );
	}
}

cBaseObject* cObjectManager::GetObject( sObject* pInfo )
{
	if( pInfo == 0 )
		return 0;
	return GetObject( pInfo->type, pInfo->index );
}

cBaseObject* cObjectManager::GetObject( unsigned char type, unsigned long idx )
{
	switch( type )
	{
	case eOBJECTTYPE_HERO://		return mpHero;
	case eOBJECTTYPE_PLAYER:	return GetPlayer( idx );
	case eOBJECTTYPE_MONSTER:	return GetMonster( idx );
	case eOBJECTTYPE_NPC:		return GetNpc( idx );
	}
	return 0;
}

cPlayer* cObjectManager::GetPlayer( unsigned long idx )
{
	/// 플레이어와 히어로가 다른 자료구조에 있어서 따로 따로 검사
	if( mpHero && mpHero->GetObjectID() == idx )
	{
		return mpHero;
	}

	cPlayer*  pPlayer = (cPlayer*)mpPlayerMap->GetAt( idx ); 
	return pPlayer;
}

cPlayer* cObjectManager::GetPlayerByName( LPCTSTR name )
{
	if( mpPlayerMap == 0 )
		return 0;

	cPlayerHashMap::cIterator i, end;
	cPlayer* player = NULL;

	i = mpPlayerMap->Begin();
	end = mpPlayerMap->End();
	for( ; i != end; ++i )
	{
		player = (cPlayer*)(*i).mSecond;
		if( player )
		{
			if( ::_tcscmp( player->GetName(), name ) == 0 )
				return player;
		}
	}
	return 0;
}

cMonster* cObjectManager::GetMonster( unsigned long idx )
{
	cMonster*  pMonster = (cMonster*)mpMonsterMap->GetAt( idx ); 
	return pMonster;
}

cNpc* cObjectManager::GetNpc( unsigned long idx )
{
	cNpc*  pNpc = (cNpc*)mpNpcMap->GetAt( idx ); 
	return pNpc;
}

bool cObjectManager::ProcessLButtonClick( int mouseX, int mouseY )
{
	/// 점집/노점 간판 클릭체크
	{
		cBaseObject* pObj = SCENEMAN->PickPlane( mouseX, mouseY );
		if( pObj )
		{
			switch( pObj->GetObjectType() )
			{
			case eOBJECTTYPE_PLAYER:
				{
					/// 플레이어 검사 
					cPlayer* pPlayer = (cPlayer*)pObj;
					if( pPlayer->GetStopFlag() == eSTOP_OPENTAROT )
					{
						if( mpHero->GetState() != eOBJECT_STATE_STOP )
							return true;
					}
					else if( pPlayer->GetStopFlag() == eSTOP_OPENSTALL )
					{
						if( mpHero->GetState() != eOBJECT_STATE_STOP )
							return true;
					}
				}
				break;
			case eOBJECTTYPE_NPC:
				{
					if( STAGEMAN->GetCurrentStage() == eStage_Game )
					{
						if( mpHero->GetState() != eOBJECT_STATE_STOP )
							return true;
					}
				}
				break;
			}
		}
	}

	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)ObjectPicking( mouseX, mouseY, &pObj );
	if( pObj == 0 )
	{
		cManagedStaticSceneNode* node = ManagedStaticPicking( mouseX, mouseY );
		if( node )
		{
			if( node->GetType() == cSceneNode::eGATHERING )
			{
				if( ((cGatheringSceneNode*)node)->IsEnableGathering() == false )
					return false;
			}
			return true;
		}
		return false;
	}

	switch( type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_MONSTER:
	case eOBJECTTYPE_NPC:
	case eOBJECTTYPE_PLAYER:
		/// 공통 처리..
		mpHero->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );
		break;
	case eOBJECTTYPE_DISPLAY:
		{
		}
		break;
	default:
		return false;
	}

	return true;
}

bool cObjectManager::ProcessLButtonDClick( int mouseX, int mouseY )
{
	if( mpHero )
	{
		unsigned int state = mpHero->GetState();
		if( state == eOBJECT_STATE_DIE )
			return false;
	}

	/// 점집/노점 간판 클릭체크
	{
		cBaseObject* pObj = SCENEMAN->PickPlane( mouseX, mouseY );
		if( pObj )
		{
			switch( pObj->GetObjectType() )
			{
			case eOBJECTTYPE_PLAYER:
				{
					/// 플레이어 검사 
					cPlayer* pPlayer = (cPlayer*)pObj;
					if( pPlayer->GetStopFlag() == eSTOP_OPENTAROT )
					{
						if( mpHero->GetState() != eOBJECT_STATE_STOP )
						{
							mpHero->AfterUseTarotStore( pPlayer );
							return true;
						}
					}
					else if( pPlayer->GetStopFlag() == eSTOP_OPENSTALL )
					{
						if( mpHero->GetState() != eOBJECT_STATE_STOP )
						{
							mpHero->AfterUseStall( pPlayer );
							return true;
						}
					}
				}
				break;
			case eOBJECTTYPE_NPC:
				{
					cNpc* pNpc = (cNpc*)pObj;
					if( pNpc )
					{
						/// 패킷 발송할 부분..
						if( STAGEMAN->GetCurrentStage() == eStage_Game )
						{
							if( mpHero->GetState() != eOBJECT_STATE_STOP )
							{
								mpHero->AfterNpcTalk( pNpc );
								return true;
							}
						}
					}
				}
				break;
			}
		}
	}

	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)ObjectPicking( mouseX, mouseY, &pObj );
	if( pObj == 0 )
	{
		///
		cManagedStaticSceneNode* node = ManagedStaticPicking( mouseX, mouseY );
		if( node )
		{
			switch( node->GetType() )
			{
			case cSceneNode::eDROPITEM:
				{
					cDropItemSceneNode* n = (cDropItemSceneNode*)node;
					if( n->IsHeroOwner() )
					{
						unsigned long idx = n->GetDropIndex();
						HERO->AfterItemPick( idx, n->GetWorldTranslate().x, n->GetWorldTranslate().y );
					}
					else
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 211 ) );
					}
				}
				break;
			case cSceneNode::eMAPPORTAL:
				{
					cMapPortalSceneNode* portal = (cMapPortalSceneNode*)node;
					if( mpHero->GetState() == eOBJECT_STATE_STOP )
					{
						if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
						{
							///
							return false;
						}
					}
					mpHero->AfterMapChange( portal );
				}
				break;
			case cSceneNode::eGATHERING:
				{
					cGatheringSceneNode* n = (cGatheringSceneNode*)node;
					if( n->IsRemove() )
					{
						/// chat msg
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 601 ) );
					}
					else
					{
						if( n->IsEnableGathering() == false )
							return false;

						unsigned long gatherIdx = n->GetGatheringIndex();
						if( gatherIdx != 0 )
						{
							if( mpHero->GetState() == eOBJECT_STATE_STOP )
							{
								if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
									return false;
							}

							mpHero->AfterGatheringPick( gatherIdx, n->GetGatheringClassIdx(), n->GetWorldTranslate().x, n->GetWorldTranslate().y );
						}
					}
				}
				break;
			}
			return true;
		}
		return false;
	}

	switch( type )
	{
	case eOBJECTTYPE_NPC:
		{
			mpHero->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );

			/// 패킷 발송할 부분..
			if( STAGEMAN->GetCurrentStage() == eStage_Game )
			{
				if( mpHero->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
						return false;
				}

				mpHero->AfterNpcTalk( (cNpc*)pObj );
			}
		}
		break;
	case eOBJECTTYPE_MONSTER:
	case eOBJECTTYPE_HERO:
		{
			mpHero->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );
		}
		break;
	case eOBJECTTYPE_PLAYER:
		{
			mpHero->SetTargetObject( (unsigned char)type, pObj->GetObjectID() );

			/// 플레이어 검사 
			cPlayer* pPlayer = (cPlayer*)pObj;
			if( pPlayer->GetStopFlag() == eSTOP_OPENTAROT )
			{
				if( mpHero->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
						return false;
				}

				mpHero->AfterUseTarotStore( pPlayer );
			}
			else if( pPlayer->GetStopFlag() == eSTOP_OPENSTALL )
			{
				if( mpHero->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->GetStopFlag() != eSTOP_NPCSPEECH )
						return false;
				}

				mpHero->AfterUseStall( pPlayer );
			}
		}
		break;
	case eOBJECTTYPE_DISPLAY:
		{

		}
		break;
	default:
		return false;
	}

	return true;
}

/// 객체에 Mouse가 올라왔을때 처리..
bool cObjectManager::ProcessMouseOver( int mouseX, int mouseY )
{
	/// 점집/노점 간판 클릭체크
	{
		cBaseObject* pObj = SCENEMAN->PickPlane( mouseX, mouseY );
		if( pObj )
		{
			switch( pObj->GetObjectType() )
			{
			case eOBJECTTYPE_PLAYER:
				{
					/// 플레이어 검사 
					cPlayer* pPlayer = (cPlayer*)pObj;
					if( pPlayer->GetStopFlag() == eSTOP_OPENTAROT )
					{
						CURSOR->SetCursor( eCURSOR_TAROT );
						return true;
					}
					else if( pPlayer->GetStopFlag() == eSTOP_OPENSTALL )
					{
						CURSOR->SetCursor( eCURSOR_USER_SELL );
						return true;
					}
				}
				break;
			case eOBJECTTYPE_NPC:
				{
					if( STAGEMAN->GetCurrentStage() == eStage_Game )
					{
						CURSOR->SetCursor( eCURSOR_TALK );
						return true;
					}
				}
				break;
			}
		}
	}

	cBaseObject* pOldObj = GetObject( &mOverObject );

	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)ObjectPicking( mouseX, mouseY, &pObj );
	if( pObj )
	{
		mOverObject.index = pObj->GetObjectID();
		mOverObject.type = pObj->GetObjectType();

		if( mOverObject.type == eOBJECTTYPE_MONSTER )
		{
			if( PVPMAN && PVPMAN->IsEnableBattle() )
			{
				if( ((cMonster*)pObj)->GetPvPTeamType() != ePVPDM_TEAMTYPE_MAX &&  ((cMonster*)pObj)->GetPvPTeamType() != HERO->GetPVPTeam() )
					CURSOR->SetCursor( eCURSOR_ATTACK );
			}
			else
				CURSOR->SetCursor( eCURSOR_ATTACK );
		}
		else if( mOverObject.type == eOBJECTTYPE_NPC )
		{
			CURSOR->SetCursor( eCURSOR_TALK );
		}
		else if( mOverObject.type == eOBJECTTYPE_PLAYER )
		{
			if( DUELMAN->IsDuelEnemy( (cPlayer*)pObj ) == true ||
				PVPMAN->IsEnableAttack( (cPlayer*)pObj ) == true )
			{
				CURSOR->SetCursor( eCURSOR_ATTACK );
			}
			else
			{
				CURSOR->SetCursor( eCURSOR_USER );
			}
		}
		else
		{
			CURSOR->SetCursor( eCURSOR_DEFAULT );
		}
	}
	else
	{
		mOverObject.index = 0;
		mOverObject.type = eOBJECTTYPE_NONE;
	}

	if( pOldObj != pObj )
	{
		if( pOldObj )
		{
			if( pOldObj != HERO->GetTargetObject() )
				pOldObj->SetOverFlag( false );
		}

		if( pObj )
			pObj->SetOverFlag( true );
	}

	bool check = true;
	switch( type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_MONSTER:
	case eOBJECTTYPE_NPC:
	case eOBJECTTYPE_DISPLAY:
		break;
	case eOBJECTTYPE_PLAYER:
		{
			if( pObj == NULL )
				check = false;

// 			if( DUELMAN->IsDuelEnemy( (cPlayer*)pObj ) != true &&
// 				PVPMAN->IsEnableAttack( (cPlayer*)pObj ) != true )
// 				check = false;
		}
		break;
	default:
		check = false;
		break;
	}

	cManagedStaticSceneNode* oldNode = SCENEMAN->GetManagedStaticNode( mOverNode.type, mOverNode.indexByManager );
	if( oldNode )
		oldNode->SetMouseOver( false );

	/// managed static
	if( check == false )
	{
		cManagedStaticSceneNode* node = ManagedStaticPicking( mouseX, mouseY );
		if( node )
		{
			check = true;
			switch( node->GetType() )
			{
			case cSceneNode::eDROPITEM:
				{
					CURSOR->SetCursor( eCURSOR_DROPITEM );
				}
				break;
			case cSceneNode::eMAPPORTAL:
				{
					CURSOR->SetCursor( eCURSOR_MOVEMAP );
				}
				break;
			case cSceneNode::eGATHERING:
				{
					cGatheringSceneNode* n = (cGatheringSceneNode*)node;
					if( n->IsRemove() || n->IsEnableGathering() == false )
						check = false;
					else
					{
						CURSOR->SetCursor( eCURSOR_GATHERING );
					}
				}
				break;
			default: 
				check = false; 
				break;
			}

			if( check == true )
			{
				mOverNode.type = node->GetType();
				mOverNode.indexByManager = node->GetIndexByManger();
				node->SetMouseOver( true );
			}
			else
			{
				mOverNode.type = cSceneNode::eNULL;
				mOverNode.indexByManager = 0;
			}
		}
	}
	else
	{
		mOverNode.type = cSceneNode::eNULL;
		mOverNode.indexByManager = 0;
	}
	return check;
}

bool cObjectManager::ProcessRButtonDown( int mouseX, int mouseY )
{
	mRbuttonObject = 0;

	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)ObjectPicking( mouseX, mouseY, &pObj );

	switch( type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_MONSTER:
	case eOBJECTTYPE_NPC:
		return false;		
	case eOBJECTTYPE_PLAYER:
		break;
	default:
		return false;
	}

	mRbuttonObject = pObj;

	return false;
}

bool cObjectManager::ProcessRButtonUp( int mouseX, int mouseY )
{
	cBaseObject* pObj = 0;
	eOBJECTTYPE type = (eOBJECTTYPE)ObjectPicking( mouseX, mouseY, &pObj );

	switch( type )
	{
	case eOBJECTTYPE_HERO:
	case eOBJECTTYPE_MONSTER:
	case eOBJECTTYPE_NPC:
		return false;
	case eOBJECTTYPE_PLAYER:
		{
			if( mRbuttonObject == pObj )
			{
				/// 
				cRButtonMenuWindow* win = GAMEUI->GetRButtonMenuWindow();
				if( win )
					win->PopUp( mouseX, mouseY, pObj->GetObjectID(), pObj->GetName(), ePopupPos_Character );

				mRbuttonObject = 0;
			}
		}
		break;
	default:
		return false;
	}

	return true;
}

/// 주의 : 아래 함수들?/// no connect server를 위한 함수들 시작
void cObjectManager::AddHeroInfo( unsigned long objectIdx, LPTSTR heroname, char race, char gender )
{
	if( mpHero )
	{
		assert(0);
		return;
	}

	/// 임시!!
	gender = eGENDER_MALE;
	race = eRACE_HUMAN;

	objectIdx;

	mpHero = new cHero;
	mpHero->SetName( heroname );
	mpHero->SetGender( gender );
	mpHero->SetRace( race );
	mpHero->SetJob( ePLAYER_FIGHTER );

	/// 061102 wonju
	mpHero->SetPathFinder( WORLDMAN->GetPathFinder() );
}

void cObjectManager::AddHero()
{
	if( !mpHero )	return;

	sPlayerExrInfo exrInfo;

	sPlayerWearInfo WearInfo;
	sPlayerWeaponInfo WeaponInfo;

	WeaponInfo.WeaponIdx[eHAND_LEFT] = 1600000;
	WeaponInfo.WeaponIdx[eHAND_RIGHT] = 1000000;

	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3( 17600.71f, 9000.33f, 0.0f );
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	mpHero->Create( HERO->GetPlayerInfo(), &WearInfo, &WeaponInfo, &exrInfo, pos );
}

void cObjectManager::AddPlayer( unsigned long objectIdx, char race, char gender, int x, int y )
{
	cPlayer* player = new cPlayer;
	if( mpPlayerMap->Insert( objectIdx, player ) == false )
	{
		assert(0);
		return;
	}

	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3(float(x), (float)y, 0.0f);
	if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	//	player->SetPos((float)x,(float)y);

	/// 임시!!
	gender = eGENDER_MALE;
	race = eRACE_HUMAN;	

	sPlayerInfo sBaseInfo; 
	sBaseInfo.CharacterIdx = objectIdx;
	sBaseInfo.Gender = gender;
	sBaseInfo.Race = race;

	sPlayerExrInfo exrInfo;

	sPlayerWearInfo WearInfo;
	sPlayerWeaponInfo WeaponInfo;
	WeaponInfo.WeaponIdx[eHAND_LEFT] = 1600000;
	WeaponInfo.WeaponIdx[eHAND_RIGHT] = 1000000;

	player->Create( &sBaseInfo, &WearInfo, &WeaponInfo, &exrInfo, pos );
}

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
/// no connect server를 위한 함수들 끝..
///

void cObjectManager::ChangeGuildMarkRegistDate( unsigned long guildIndex, tm markRegistDate )
{
	if( !mpPlayerMap->IsEmpty() )
	{
		cPlayerHashMap::cIterator i = mpPlayerMap->Begin();
		cPlayerHashMap::cIterator end = mpPlayerMap->End();
		for( ; i != end; ++i )
		{
			cPlayer* player = (cPlayer*)(*i).mSecond;
			if( player && player->GetGuildIndex() == guildIndex )
				player->ChangeGuildMarkRegistDate( markRegistDate, true );
		}
	}
}

// 선공/비선공 이름 컬러 갱신
void cObjectManager::UpdateMonsterNameColor()
{
	if( mpMonsterMap->IsEmpty() == false )
	{
		cMonsterHashMap::cIterator i = mpMonsterMap->Begin();
		cMonsterHashMap::cIterator end = mpMonsterMap->End();
		for( ; i != end; ++i )
		{
			cMonster* monster = (cMonster*)(i->mSecond);
			if( monster )
			{
				monster->UpdateNameCardTextValue();
			}
		}
	}
}

void cObjectManager::ClearTutorialImage()
{
	cMonsterHashMap::cIterator mi = mpMonsterMap->Begin();
	cMonsterHashMap::cIterator mend = mpMonsterMap->End();
	for( ; mi != mend; ++mi )
	{
		cMonster* monster = (cMonster*)(mi->mSecond);
		if( monster )
		{
			monster->ClearTutorialBoard();
		}
	}

	cNpcHashMap::cIterator ni = mpNpcMap->Begin();
	cNpcHashMap::cIterator nend = mpNpcMap->End();
	for( ; ni != nend; ++ni )
	{
		cNpc* npc = (cNpc*)(ni->mSecond);
		if( npc )
		{
			npc->ClearTutorialBoard();
		}
	}
}

void cObjectManager::UpdateTutorialImage( eOBJECTTYPE type )
{
	if( type == eOBJECTTYPE_MONSTER )
	{
		cMonsterHashMap::cIterator i = mpMonsterMap->Begin();
		cMonsterHashMap::cIterator end = mpMonsterMap->End();
		for( ; i != end; ++i )
		{
			cMonster* monster = (cMonster*)(i->mSecond);
			if( monster )
			{
				monster->UpdateTutorialBoard();
			}
		}
	}
	else if( type == eOBJECTTYPE_NPC )
	{
		cNpcHashMap::cIterator i = mpNpcMap->Begin();
		cNpcHashMap::cIterator end = mpNpcMap->End();
		for( ; i != end; ++i )
		{
			cNpc* npc = (cNpc*)(i->mSecond);
			if( npc )
			{
				npc->UpdateTutorialBoard();
			}
		}
	}
}

void cObjectManager::UpdateHelmetState()
{
	bool show = OPTIONMAN->IsShowHelmet();

	if( mpHero )
		mpHero->UpdateHelmetState( show );

	if( mpPlayerMap->IsEmpty() == true )
		return;

	cPlayerHashMap::cIterator i = mpPlayerMap->Begin();
	cPlayerHashMap::cIterator end = mpPlayerMap->End();
	for( ; i != end; ++i )
	{
		cPlayer* player = (cPlayer*)(*i).mSecond;
		if( player )
			player->UpdateHelmetState( show );
	}
}
