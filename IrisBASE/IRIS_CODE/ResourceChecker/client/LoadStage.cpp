#include "stdafx.h"
#include "LoadStage.h"
// #include "LoadUIManager.h"
// #include "StageManager.h"
// #include "ObjectManager.h"
// #include "SoundSystem.h"
// 
// #include "Application.h"
// 
// #include "OptionManager.h"
#include "GameResourceManager.h"
#include "WorldManager.h"
#include "SceneManager.h"
#include "DramaturgyManager.h"
// #include "GameUIManager.h"
// 
// #include "ItemManager.h"
// 
#include "ResourceManager.h"
// #include "CoolTimeManager.h"
// 
#include "./common/Npc_Common.h"
// 
#include "./common/NpcScript.h"
#include "./common/MonsterScript.h"
#include "./common/SkillScript.h"
// #include "SkillManager.h"
#include "StageScript.h"
// 
// #include "hero.h"
// 
// #include "QuickManager.h"
// #include "QuestManager.h"
// #include "PVPManager.h"
// #include "TitleManager.h"
// #include "GuildManager.h"
// #include "ItemMixManager.h"
// 
// #include "rendersystem.h"
// 
// #include "UIMsgboxEvent.h"

#include "./common/Monster_Common.h"

cLoadStage::cLoadStage()
{
	mMonsterClassCount = 0;
	mNpcClassCount = 0;

	mSkillCount = 0;
	mInfluenceCount = 0;
	mGuardInfluenceCount = 0;
	
	mLoadState = eLOADSTATE_NONE;
	mCheckTime = 0;

	mLoadFlag.loadInfo = 0;

	mpLoadUIManager = 0;

	mGoPercent  = 0.0f;

	::memset( mMonsterClassIdx, 0, sizeof( mMonsterClassIdx ) );
	::memset( mNpcClassIdx,     0, sizeof( mNpcClassIdx ) );
	::memset( mSkill,           0, sizeof( mSkill ) );
	::memset( mInfluence,		0, sizeof( mInfluence ) );
	::memset( mGuardInfluence,	0, sizeof( mGuardInfluence ) );
}

cLoadStage::~cLoadStage()
{
}

void cLoadStage::Init( cStageManager* pStageManager )
{
	mpStageManager = pStageManager;
}

void cLoadStage::InitStage()
{
	if( mInit )	return;

// 	mpLoadUIManager = new cLoadUIManager;
// 	mpLoadUIManager->Init();

	mInit = true;
}

void cLoadStage::Exit()
{
//	SAFE_EXIT_DELETE( mpLoadUIManager );

	mpStageManager = NULL;
}

void cLoadStage::Open()
{
	SCENEMAN->Clear();

//	RENDERSYS->SetWorldRendering( false );

	mMonsterClassCount = 0;
	mNpcClassCount = 0;
	mSkillCount = 0;
	mInfluenceCount = 0;
	mGuardInfluenceCount = 0;

	mLoadState = eLOADSTATE_START;
	mCheckTime = 0;

//	SCENEMAN->SetLoadingFlag( true );
	mLoadFlag.loadInfo = 0;

// 	/// ui
// 	mpLoadUIManager->Open( STAGEMAN->GetMapIdx(), STAGEMAN->GetMapMode() );
// 	PVPMAN->SetEnableBattle( false );
// 
// 	GUILDMAN->GuildInfoReset();
// 
// 	/// 로딩이 완료 되었다면....
// 	if( NETWORK->IsConnected() )
// 	{
// 		MSGROOT msg;
// 		msg.Category = NM_USER;
// 		msg.Protocol = NM_USER_LOADDATA_REQ;
// 		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
// 	}

	///
	mGoPercent = 0.0f;
	mCheckTime = 0;
}

void cLoadStage::Close()
{
	/// UI
//	mpLoadUIManager->Close();
}

void cLoadStage::ChangeLoadState( eLOADSTATE state )
{
	mLoadState = state;
	mCheckTime = 0;
	mGageStateCount = 0;

	ChangeGaugeState( state );
}

void cLoadStage::ChangeGaugeState( unsigned int state )
{
	mCheckTime = 0;

	if( mLoadState == eLOADSTATE_GAMEINFO )
	{
		mGoPercent = 100.0f;
	}
	else if( mLoadState != eLOADSTATE_END )
	{
		mGoPercent = ((float)state/(float)eLOADSTATE_END)*100.0f;
	}
}


void cLoadStage::Process( unsigned long deltaTime, unsigned long accumTime )
{
// 	if( SOUNDSYS->IsBGMFading() )
// 		return;
// 
// 	mCheckTime += deltaTime;
// 	if( mCheckTime >= 90000 )
// 	{
// 		LoadTimeFaile( mLoadState );
// 		return;
// 	}

	switch( mLoadState )
	{
	case eLOADSTATE_NONE: 
		return;
	case eLOADSTATE_START:
		{
// 			SOUNDSYS->StopAll();
 			WORLDMAN->Close();

//			if( GAMERESOURCEMAN->IsMapUseTile( STAGEMAN->GetMapIdx(), STAGEMAN->GetMapMode() ) == true )
			if( GAMERESOURCEMAN->IsMapUseTile(  (unsigned int)deltaTime, (unsigned char)accumTime ) == true )
				ChangeLoadState( eLOADSTATE_TERRAIN_HEADER );
			else
				ChangeLoadState( eLOADSTATE_NAVIMESH_HEADER );
		}
		break;
	case eLOADSTATE_TERRAIN_HEADER:
		{
			mCount = 0;
//			if( WORLDMAN->LoadTerrainHeader( mCount, STAGEMAN->GetMapIdx(), STAGEMAN->GetMapMode() ) == false )
			if( WORLDMAN->LoadTerrainHeader( mCount, (unsigned int)deltaTime, (unsigned char)accumTime ) == false )
			{
				LoadFaile( eLOADSTATE_TERRAIN_HEADER );
				return;
			}
			ChangeLoadState( eLOADSTATE_TERRAIN_NODE );
		}
		break;
	case eLOADSTATE_TERRAIN_NODE:
		{
			int ret = WORLDMAN->LoadTerrainNodes( 30 );
			if( ret == -1 )
			{
				LoadFaile( eLOADSTATE_TERRAIN_NODE );
				return;
			}
			else if( ret == -2 )
			{
				ChangeLoadState( eLOADSTATE_NAVIMESH_HEADER );
			}

			if( mLoadState + mGageStateCount < eLOADSTATE_NAVIMESH_HEADER )
			{
				mGageStateCount++;
				ChangeGaugeState( mLoadState+mGageStateCount );
			}
		}
		break;
	case eLOADSTATE_NAVIMESH_HEADER:
		{
			mCount = 0;
			if( WORLDMAN->LoadNaviMeshHeader( mCount, (unsigned int)deltaTime, (unsigned char)accumTime ) == false )
			{
				LoadFaile( eLOADSTATE_NAVIMESH_HEADER );
				return;
			}
			ChangeLoadState( eLOADSTATE_NAVIMESH_NODE );
		}
		break;
	case eLOADSTATE_NAVIMESH_NODE:
		{
			int ret = WORLDMAN->LoadNaviMeshNodes( 30 );
			if( ret == -1 )
			{
				LoadFaile( eLOADSTATE_NAVIMESH_NODE );
				return;
			}
			else if( ret == -2 )
			{
				ChangeLoadState( eLOADSTATE_NAVIFIELD );
			}

			if( mLoadState + mGageStateCount < eLOADSTATE_NAVIFIELD )
			{
				mGageStateCount++;
				ChangeGaugeState( mLoadState+mGageStateCount );
			}
		}
		break;
	case eLOADSTATE_NAVIFIELD:
		{
			/// navifield loading
			if( WORLDMAN->LoadNaviField( (unsigned int)deltaTime, (unsigned char)accumTime ) == false )
			{
				LoadFaile( eLOADSTATE_NAVIFIELD );
				return;
			}

			ChangeLoadState( eLOADSTATE_SCENE_HEADER );
		}
		break;
	case eLOADSTATE_SCENE_HEADER:
		{
			mCount = 0;
			if( BeginSceneLoad( (unsigned int)deltaTime, (unsigned char)accumTime ) == false )
			{
				LoadFaile( eLOADSTATE_SCENE_HEADER );
				return;
			}

			/// 영역 읽기
			if( SCENEMAN->LoadSceneHeader( mFileLoader, mCount ) == false )
			{
				LoadFaile( eLOADSTATE_SCENE_HEADER );
				return;
			}

			if( mCount )
				ChangeLoadState( eLOADSTATE_SCENE_NODE );
			else
				ChangeLoadState( eLOADSTATE_SOUND_HEADER );
		}
		break;
	case eLOADSTATE_SCENE_NODE:
		{
			/// 씬로딩..
			int rVal = SCENEMAN->LoadSceneNode( mFileLoader, 10 );
			if( rVal == -1 )
			{
				LoadFaile( eLOADSTATE_SCENE_NODE );
				return;
			}
			else if( rVal == -2 )
			{
				ChangeLoadState( eLOADSTATE_SOUND_HEADER );
				SCENEMAN->LoadAreaGroup( (unsigned int)deltaTime, (unsigned char)accumTime );
			}

			if( mLoadState + mGageStateCount < eLOADSTATE_SOUND_HEADER )
			{
				mGageStateCount++;
				ChangeGaugeState( mLoadState+mGageStateCount );
			}
		}
		break;
	case eLOADSTATE_SOUND_HEADER:
		{
			mFileLoader.Close();
			mCount = 0;

			if( BeginSoundSceneLoad( (unsigned int)deltaTime, (unsigned char)accumTime ) == false )
			{
				ChangeLoadState( eLOADSTATE_MAP_PORTAL );
				return;
			}

			/// 영역 읽기
			if( SCENEMAN->LoadSoundSceneHeader( mFileLoader, mCount ) == false )
			{
				LoadFaile( eLOADSTATE_SOUND_HEADER );
				return;
			}

			if( mCount )
				ChangeLoadState( eLOADSTATE_SOUND_NODE );
			else
				ChangeLoadState( eLOADSTATE_MAP_PORTAL );
		}
		break;
	case eLOADSTATE_SOUND_NODE:
		{
			int rVal = SCENEMAN->LoadSoundSceneNode( mFileLoader, 10 );
			if( rVal == -1 )
			{
				LoadFaile( eLOADSTATE_SOUND_NODE );
				return;
			}
			else if( rVal == -2 )
			{
				ChangeLoadState( eLOADSTATE_MAP_PORTAL );
			}

			if( mLoadState + mGageStateCount < eLOADSTATE_MAP_PORTAL )
			{
				mGageStateCount++;
				ChangeGaugeState( mLoadState+mGageStateCount );
			}
		}
		break;
	case eLOADSTATE_MAP_PORTAL:
		{
			mFileLoader.Close();

			tPointerArray<void*>* pMapArray = STAGESCRIPT->GetMapChangeStarterArr( (unsigned int)deltaTime );
			if( pMapArray->IsEmpty() == false )
			{
				for( unsigned int i=0, end = pMapArray->GetSize(); i<end; ++i )
				{
					sStageChangePos* p = (sStageChangePos*)(*pMapArray)[i];

					if( p->mFirstRegen == false )
						continue;

					NiPoint3 pos = NiPoint3( p->mPosX, p->mPosY,0.0f );
					if( WORLDMAN->CalcHeight( &pos.z, pos.x, pos.y ) == false )
					{
						assert(0);
						continue;
					}

					NiMatrix3 rot;
					rot.FromEulerAnglesXYZ( 0, 0, p->mRotAngle );
					SCENEMAN->CreateMapPortal( p->mPosIdx, pos, rot );
				}
			}

			ChangeLoadState( eLOADSTATE_MONSTERDATA );
		}
		break;
	case eLOADSTATE_MONSTERDATA:
		{
// 			if( mLoadFlag.flag.monsterModelOn == false )
// 				break;
// 
// 			if( mMonsterClassCount != 0 )
// 			{
// 				unsigned long idx = mMonsterClassIdx[--mMonsterClassCount];
// 				sMonsterScript* info = MONSTERSCRIPT->GetMonsterListInfo( idx );
// 				if( info )
// 				{
// 					const char* name = GAMERESOURCEMAN->GetModelFileName( info->mKfmIdx );
// 					if( name )
// 					{
// 						cString pathName;
// 						pathName.Format( "./data/monster/%s", name );
// 
// 						RESOURCEMAN->LoadKFM( pathName );
// 					}
// 					else
// 					{
// 						assert(0);
// 					}
// 
//					/// 몬스터 스킬에 대한 연출 객체 Load
// 					DRAMATURGYMAN->DirectLoadMonsterDramaObj( idx );
// 				}
// 				else
// 				{
// 					assert(0);
// 				}
// 			}
// 			else
// 			{
 				ChangeLoadState( eLOADSTATE_NPCDATA );
// 			}
// 
// 			if( mLoadState + mGageStateCount < eLOADSTATE_NPCDATA )
// 			{
// 				mGageStateCount++;
// 				ChangeGaugeState( mLoadState+mGageStateCount );
// 			}
		}
		break;
	case eLOADSTATE_NPCDATA:
		{
// 			if( mLoadFlag.flag.npcModelOn == false )
// 				break;
// 
// 			if( mNpcClassCount != 0 )
// 			{
// 				unsigned long idx = mNpcClassIdx[--mNpcClassCount];
// 				sNPCList* info = NPCSCRIPT->GetNPCList( idx );
// 				if( info )
// 				{
// 					const char* name = GAMERESOURCEMAN->GetModelFileName( info->mModelIndex );
// 					if( name )
// 					{
// 						cString pathName;
// 						pathName.Format( "./Data/Npc/%s", name );
// 
// 						RESOURCEMAN->LoadKFM( pathName );
// 					}
// 					else
// 					{
// 						assert(0);
// 					}
// 				}
// 				else
// 				{
// 					assert(0);
// 				}
// 			}
// 			else
// 			{
 				ChangeLoadState( eLOADSTATE_HEROITEM );
// 			}
// 
// 			if( mLoadState + mGageStateCount < eLOADSTATE_HEROITEM )
// 			{
// 				mGageStateCount++;
// 				ChangeGaugeState( mLoadState+mGageStateCount );
// 			}
		}
		break;
	case eLOADSTATE_HEROITEM:
		{
// 			if( mLoadFlag.flag.itemInfoOn == false )
// 				break;

// 			for( unsigned short i=MIN_INVENTORY; i<=MAX_INVENTORY; ++i )
// 			{
// 				cItem* item = ITEMMAN->GetItem( i );
// 				cItemDefine* p = item->GetDefine();
// 				if( p )
// 				{
// 					const char* name1 = p->GetFileName0( HERO->GetRace(), HERO->GetGender() );
// 					const char* name2 = p->GetFileName1( HERO->GetRace(), HERO->GetGender() );
// 					cString pathName;
// 
// 					if( name1 )
// 					{
// 						pathName.Format( "./data/character/%s", name1 );
// 						RESOURCEMAN->LoadNIF( pathName );
// 					}
// 					if( name2 )
// 					{
// 						pathName.Format( "./data/character/%s", name2 );
// 						RESOURCEMAN->LoadNIF( pathName );
// 					}
// 				}
// 			}

			ChangeLoadState( eLOADSTATE_BASESKILL );
		}
		break;
	case eLOADSTATE_BASESKILL:
		{
// 			/// 기본공격 로딩
// 			for( unsigned long int ui= 0; ui< eRACE_MAX; ui++ )
// 				for( unsigned int uj = 0; uj<eWEAPON_STATE_MAX;uj++ )
// 					DRAMATURGYMAN->DirectLoadPlayerDramaObj( ui*10 + uj + 1 );

			ChangeLoadState( eLOADSTATE_HEROSKILL );
		}
		break;
	case eLOADSTATE_HEROSKILL:
		{
//			/// effect data loading
// 			if( mLoadFlag.flag.skillInfoOn == false )
// 				break;
// 
// 			for( unsigned int i=0;i<mSkillCount;i++ )
// 			{
// 				sPlayerSkillBaseInfo* p = SKILLSCRIPT->GetPlayerSkillInfo( mSkill[i].mSkillIdx );
// 				if( p == 0 )
// 					continue;
// 
// 				if( p->mType != eSKILLTYPE_PASSIVE )
// 					DRAMATURGYMAN->DirectLoadPlayerDramaObj( mSkill[i].mSkillIdx );
// 			}
			ChangeLoadState( eLOADSTATE_GAMEINFO );
		}
		break;
	case eLOADSTATE_GAMEINFO:
		{
// 			if( mLoadFlag.flag.heroBaseInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.itemCoolTimeInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.shortcutInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.titleInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.questInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.optionInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.makeSkillInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.InfluenceInfoOn == false )
// 				break;
// 			if( mLoadFlag.flag.guardInfoOn == false )
// 				break;

			ChangeLoadState( eLOADSTATE_END );
		}
		break;
	case eLOADSTATE_END:
		{
		}
		break;
	default:
		{
			assert(0);
		}
	}

	if( mGoPercent == 100.0f )
		ChangeLoadState( eLOADSTATE_NONE );

// 	bool end = mpLoadUIManager->UpdateLoadingGauge( mGoPercent );
// 	if( mGoPercent == 100.0f && end == true )
// 	{
// 		ChangeLoadState( eLOADSTATE_NONE );
// 
// 		/// 게임 진입 패킷을 준다..
// 		MSGROOT msg;
// 		msg.Category = NM_USER;
// 		msg.Protocol = NM_USER_GAMEIN_REQ;
// 		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );
// 	}
//
//	/// UI
//	mpLoadUIManager->Process();
	
}

// void cLoadStage::NetworkMsgParser( unsigned long /*connectionindex*/, char* pMsg, unsigned long /*length*/ )
// {
// 	MSGROOT* pTemMsg = (MSGROOT*)pMsg;
// 	switch( pTemMsg->Category )
// 	{
// 	case NM_USER:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 			case NM_USER_GAMEIN_RES:
// 				{
// 					MSG_RES_GAMEIN* msg = (MSG_RES_GAMEIN*)pMsg;
// 					if( msg->ErrorCode == 0 )
// 					{
// 						STAGEMAN->SetChannelIdx( msg->channelNum );
// 
// 						Msg_CharacterGameinAck( pTemMsg );
// 					}		
// 					else
// 					{
// 						/// 실패 처리
// 						assert(0);
// 						return;
// 					}
// 				}
// 				break;
// 			}
// 
// 		}break;
// 	case NM_MONSTER:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 			case NM_MONSTER_MODELNUMBER_RES:
// 				{
// 					MSG_MODELNUMBER* pmsg = (MSG_MODELNUMBER*)pMsg;
// 
// 					mMonsterClassCount = pmsg->mCount;
// 					assert( mMonsterClassCount <= 50 );
// 					::memcpy( mMonsterClassIdx, pmsg->mClassIdx, sizeof(unsigned long) * mMonsterClassCount );
// 
// 					mLoadFlag.flag.monsterModelOn = true;
// 				}
// 			}
// 		}break;
// 	case NM_NPC:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 			case NM_NPC_MODELNUMBER_RES:
// 				MSG_MODELNUMBER* pmsg = (MSG_MODELNUMBER*)pMsg;
// 
// 				mNpcClassCount = pmsg->mCount;
// 				assert( mNpcClassCount <= 50 );
// 				::memcpy( mNpcClassIdx, pmsg->mClassIdx, sizeof(unsigned long) * mNpcClassCount );
// 
// 				mLoadFlag.flag.npcModelOn = true;
// 			}
// 		}break;
// 	case NM_SKILL:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 				/// 070828 PKH 스킬로드
// 			case NM_SKILL_LIST_RES:
// 				{
// 					MSG_RES_SKILL_LIST* pmsg = (MSG_RES_SKILL_LIST*)pMsg;
// 
// 					mSkillCount = pmsg->mRowCount;
// 					assert( mSkillCount <= 200 );
// 					::memcpy( mSkill, pmsg->mTable, sizeof(sHaveSkill) * mSkillCount );
// 
// 					/// 스킬 생성
// 					sHaveSkill* inf = mSkill;
// 					for( unsigned int i = 0; i < mSkillCount; ++i, ++inf )
// 					{
// 						SKILLMAN->InitKeepSkill( inf );
// 					}
// 
// 					mLoadFlag.flag.skillInfoOn = true;
// 				}
// 				break;
// 			case NM_SKILL_INFLUENCE_LIST_RES:
// 				{
// 					MSG_RES_INFLUENCE_LIST* pmsg = (MSG_RES_INFLUENCE_LIST*)pMsg;
// 
// 					mInfluenceCount = pmsg->mRowCount;
// 					assert( mInfluenceCount <= 50 );
// 					::memcpy( mInfluence, pmsg->mTable, sizeof(sInfluenceClient) * mInfluenceCount );
// 
// 					mLoadFlag.flag.InfluenceInfoOn = true;
// 				}
// 				break;
// 			case NM_SKILL_INFLUENCE_GUARDLIST_RES:
// 				{
// 					MSG_SYN_SKILL_INFLUENCE_GUARDLIST* pmsg = (MSG_SYN_SKILL_INFLUENCE_GUARDLIST*)pMsg;
// 
// 					mGuardInfluenceCount = pmsg->mRowCount;
// 					assert( mGuardInfluenceCount <= 20 );
// 					::memcpy( mGuardInfluence, pmsg->mGuardList, sizeof(sInfGuardList) * mGuardInfluenceCount );
// 
// 					mLoadFlag.flag.guardInfoOn = true;
// 				}
// 				break;
// 			}
// 		}break;
// 	case NM_ITEM:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 			case NM_ITEM_INVENTORY_RES:
// 				{
// 					MSG_RES_INVENTORY* msgData = (MSG_RES_INVENTORY*)pMsg;
// 					sInventory*        inv = msgData->Inventory;
// 
// 					/// ui 는 나중에 업데이트
// 					for( int i = 0, iend = msgData->RowCount; i < iend; ++i, inv++ )
// 					{
// 						ITEMMAN->NetSetItem( inv, false );
// 					}
// 
// 					/// 정보 저장
// 					ITEMMAN->SetActiveWeapon( msgData->activeWeapon );
// 
// 					mLoadFlag.flag.itemInfoOn = true;
// 				}
// 				break;
// 			case NM_ITEM_INVENTORY_COOLTIME_RES:
// 				{
// 					MSG_RES_INVENTORY_COOLTIME* msgData = (MSG_RES_INVENTORY_COOLTIME*)pMsg;
// 
// 					for( int i = 0; i < msgData->RowCount; ++i )
// 					{
// 						sInventoryCooltime& cool = msgData->InventoryCooltime[i];
// 						COOLMAN->NetSetCool( cool );
// 					}
// 
// 					mLoadFlag.flag.itemCoolTimeInfoOn = true;
// 				}
// 				break;
// 			}
// 		}
// 		break;
// 	case NM_PLAYER:
// 		{
// 			switch( pTemMsg->Protocol )
// 			{
// 			case NM_PLAYER_HEROINFO_RES:
// 				{
// 					if( HERO )
// 					{
// 						HERO->SetHeroInfoNet( (MSGROOT*)pMsg );
// 					}
// 					else
// 					{
// 						/// error check
// 						assert(0);
// 						LoadFaile( eLOADTATE_HEROBASEINFO );
// 						return;
// 					}
// 					mLoadFlag.flag.heroBaseInfoOn = true;
// 				}
// 				break;
// 			case NM_PLAYER_OPTION_VALUE_RES:
// 				{
// 					MSG_RES_PLAYER_OPTION_VALUE* msgData = (MSG_RES_PLAYER_OPTION_VALUE*)pMsg;
// 					OPTIONMAN->SetOptionValue( msgData->option1 );
// 
// 					mLoadFlag.flag.optionInfoOn = true;
// 				}
// 				break;
// 			case NM_PLAYER_SHORTCUT_LIST_RES:
// 				{
// 					MSG_RES_PLAYER_SHORTCUT_LIST* msgData = (MSG_RES_PLAYER_SHORTCUT_LIST*)pMsg;
// 
// 					for( int i = 0; i < MAX_SHORTCUT_SIZE; ++i )
// 					{
// 						sShortcut& shorCut = msgData->mShortcut[i];
// 						QUICKMAN->NetSetData( i, shorCut );
// 					}
// 
// 					mLoadFlag.flag.shortcutInfoOn = true;
// 				}
// 				break;
// 			case NM_PLAYER_MONEY_RES:
// 				{
// 					MSG_RES_PLAYER_MONEY* msg = (MSG_RES_PLAYER_MONEY*)pMsg;
// 					HERO->SetMoney( msg->mMoney );
// 				}
// 				break;
// 			case NM_PLAYER_DEPOSIT_RES:
// 				{
// 					MSG_RES_PLAYER_DEPOSIT* msg = (MSG_RES_PLAYER_DEPOSIT*)pMsg;
// 					HERO->SetDeposit( msg->mDeposit );
// 				}
// 				break;
// 			case NM_PLAYER_TITLE_LIST_RES:
// 				{
// 					MSG_RES_PLAYER_TITLELIST* pmsg = (MSG_RES_PLAYER_TITLELIST*)pMsg;
// 
// 					TITLEMAN->ClearList();
// 
// 					for( int i = 0; i < pmsg->rowCount; ++i )
// 						TITLEMAN->AddHaveTitle( pmsg->mTitleIndex[i] );
// 
// 					mLoadFlag.flag.titleInfoOn = true;
// 				}
// 				break;
// 			}
// 		}
// 		break;
// 	case NM_QUEST:
// 		{
// 			QUESTMAN->NetworkMsgParserLoad( pMsg );
// 
// 			mLoadFlag.flag.questInfoOn = true;
// 		}
// 		break;
// 	case NM_MAKESKILL:
// 		{
// 			ITEMMIXMAN->NetworkMsgParserLoad( pMsg );
// 
// 			mLoadFlag.flag.makeSkillInfoOn = true;
// 		}
// 		break;
// 	}
//  }
// 
// void cLoadStage::Msg_CharacterGameinAck( MSGROOT* /*pMsg*/ )
// {
// 	mpStageManager->ChangeStage( eStage_Game );
// 
// 	HERO->GameIn( STAGEMAN->GetChannelIdx() );
// 
// 	///
// 	GAMEUI->UpdateMinimap();
// 
// 	/// 스킬 생성 세팅
// 	SKILLMAN->InitSkillUI();
// 
// 	/// 버프 및 디버프 윈도우 초기화
// 	for( unsigned int i=0; i<mInfluenceCount; i++ )
// 	{
// 		sInfluenceClient* inf = &mInfluence[i];
// 		cInfluenceObject* p = SKILLMAN->CreateInfluenceObject( inf->mUniqueIdx, inf->mInfluenceClassIdx, inf->mAuraSkillClassIdx, HERO, inf->mDieDelete );
// 		if( p )
// 			p->StartProcess( inf->mRestTime, true );
// 	}
// 
// 	/// Guard Influence Init
// 	for( unsigned int i=0; i<mGuardInfluenceCount; i++ )
// 	{
// 		sInfGuardList* inf = &mGuardInfluence[i];
// 		cInfluenceObject* p = SKILLMAN->CreateInfluenceObject( inf->mInfInfo.mUniqueIdx, inf->mInfInfo.mInfluenceClassIdx, inf->mInfInfo.mAuraSkillClassIdx, HERO, inf->mInfInfo.mDieDelete );
// 
// 		if( p )
// 		{
// 			p->SetGuardInfo( inf->mGuardCnt, inf->mGuardValue );
// 			p->StartProcess( inf->mInfInfo.mRestTime, true );
// 		}
// 	}
// 
// 	/// 1. 퀵슬롯
// 	QUICKMAN->Open();
// 
// 	/// 2. 아이템 갱신
// 	ITEMMAN->UpdateItem();
// 
// 	/// 퀘스트 매니저
// 	QUESTMAN->OpenLoad();
// 
// 	/// 저장된 숏컷 정보 셋팅하기
// 	GAMEUI->ApplyWinodwOption_CharacterShortcut();
// 
// 	SCENEMAN->SetLoadingFlag( false );
// }

bool cLoadStage::BeginSceneLoad( unsigned int mapIdx, unsigned char mapMode )
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );

	cString mapfile;
	mapfile.Format( "./data/map/%03d.scene", folderIdx );

	/// 파일 열기
	if( mFileLoader.Open( mapfile.Cstr(), true ) == false )
	{
		assert( 0 && "failed to open file to load scene" );
		return false;
	}

	return true;
}

bool cLoadStage::BeginSoundSceneLoad( unsigned int mapIdx, unsigned char mapMode )
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );

	cString mapfile;
	mapfile.Format( "./data/map/%03d.sound", folderIdx );

	/// 파일 열기
	if( mFileLoader.Open( mapfile.Cstr(), true ) == false )
	{
		return false;
	}

	return true;
}

void cLoadStage::LoadFaile( unsigned long error )
{
	mLoadState = eLOADSTATE_NONE;
	mFileLoader.Close();

	cString msg;
	msg.Format( "State[%d] Data Load Error!!" , error );
	MessageBoxA( NULL, msg.Cstr(), "LoadStage Error", MB_OK | MB_ICONERROR );

// 	/// error message 출력후 종료
// 	TCHAR errormsg[255] = {0,};
// 	_stprintf_s( errormsg, _T( "State[%d] Data Load Error!!" ), error );
// 	
// 	UIMAN->CreateMsgBox( eStage_Load, eMSGBOX_MODAL, eMBEVENT_LOAD_FAIL, eSKIN_OK, 
// 						errormsg,
// 						GAMERESOURCEMAN->GetGameText( 2 ) );
}

// void cLoadStage::LoadTimeFaile( unsigned long error )
// {
// 	mLoadState = eLOADSTATE_NONE;
// 	mFileLoader.Close();
// 
// 	/// error message 출력후 종료
// 	TCHAR errormsg[255] = {0,};
// 	_stprintf_s( errormsg, _T( "State[%d] Load time over!!" ), error );
// 
// 	UIMAN->CreateMsgBox( eStage_Load, eMSGBOX_MODAL, eMBEVENT_LOAD_FAIL, eSKIN_OK, 
// 		errormsg,
// 		GAMERESOURCEMAN->GetGameText( 2 ) );
// }
//
// void cLoadStage::MsgBoxEventParser( unsigned int event, unsigned int result, void* exData )
// {
// 	if( event == eMBEVENT_LOAD_FAIL )
// 		::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
// }
