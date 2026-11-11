#include "stdafx.h"
#include "ObjectManager.h"

#include "DummyClient.h"
#include "object.h"

#include "WorldManager.h"
#include "GameResourceManager.h"

#include "Monster_Common.h"
#include "Npc_Common.h"

cObjectManager::cObjectManager( cDummyClient* dummy )
{
	mHero = dummy;

	mFolderIdx = 0;

	mPortalGen = 0;
}

cObjectManager::~cObjectManager()
{
	Clear();
}

void cObjectManager::Clear()
{
	cPointerHashMap::cIterator i, end;

	i = mDropItemMap.Begin();
	end = mDropItemMap.End();
	for( ; i != end; ++i )
	{
		cDropItem* item = (cDropItem*)(*i).mSecond;
		SAFE_DELETE(item);
	}
	mDropItemMap.Clear();

	/// npc
	i = mNpcMap.Begin();
	end = mNpcMap.End();
	for( ; i != end; ++i )
	{
		cNpc* npc = (cNpc*)(*i).mSecond;
		SAFE_DELETE(npc);
	}
	mNpcMap.Clear();

	/// monster
	i = mMonsterMap.Begin();
	end = mMonsterMap.End();
	for( ; i != end; ++i )
	{
		cMonster* monster = (cMonster*)(*i).mSecond;
		SAFE_DELETE(monster);
	}
	mMonsterMap.Clear();

	/// totem
	i = mTotemMap.Begin();
	end = mTotemMap.End();
	for( ; i != end; ++i )
	{
		cTotem* totem = (cTotem*)(*i).mSecond;
		SAFE_DELETE(totem);
	}
	mTotemMap.Clear();

	/// gather
	i = mGatherMap.Begin();
	end = mGatherMap.End();
	for( ; i != end; ++i )
	{
		cGather* gather = (cGather*)(*i).mSecond;
		SAFE_DELETE(gather);
	}
	mGatherMap.Clear();

	/// portal
	i = mPortalMap.Begin();
	end = mPortalMap.End();
	for( ; i != end; ++i )
	{
		cPortal* portal = (cPortal*)(*i).mSecond;
		SAFE_DELETE(portal);
	}
	mPortalMap.Clear();

	/// player
	i = mPlayerMap.Begin();
	end = mPlayerMap.End();
	for( ; i != end; ++i )
	{
		cPlayer* player = (cPlayer*)(*i).mSecond;
		SAFE_DELETE(player);
	}
	mPlayerMap.Clear();
}


void cObjectManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	cPointerHashMap::cIterator i, end;

	/// npc
	i = mNpcMap.Begin();
	end = mNpcMap.End();
	for( ; i != end; ++i )
	{
		cNpc* npc = (cNpc*)(*i).mSecond;
		if( npc )
			npc->Process( deltaTime, accumTime );
	}

	/// monster
	i = mMonsterMap.Begin();
	end = mMonsterMap.End();
	for( ; i != end; ++i )
	{
		cMonster* monster = (cMonster*)(*i).mSecond;
		if( monster )
			monster->Process( deltaTime, accumTime );
	}

	/// totem
	i = mTotemMap.Begin();
	end = mTotemMap.End();
	for( ; i != end; ++i )
	{
		cTotem* totem = (cTotem*)(*i).mSecond;
		if( totem )
			totem->Process( deltaTime, accumTime );
	}

	/// gather
	i = mGatherMap.Begin();
	end = mGatherMap.End();
	for( ; i != end; ++i )
	{
		cGather* gather = (cGather*)(*i).mSecond;
		if( gather )
			gather->Process( deltaTime, accumTime );
	}

	/// portal
	i = mPortalMap.Begin();
	end = mPortalMap.End();
	for( ; i != end; ++i )
	{
		cPortal* portal = (cPortal*)(*i).mSecond;
		if( portal )
			portal->Process( deltaTime, accumTime );
	}

	/// player
	i = mPlayerMap.Begin();
	end = mPlayerMap.End();
	for( ; i != end; ++i )
	{
		cPlayer* player = (cPlayer*)(*i).mSecond;
		if( player )
			player->Process( deltaTime, accumTime );
	}
}

bool cObjectManager::AddPlayer( MSGROOT* pMsg )
{
	MSG_PLAYERINFO* pmsg = (MSG_PLAYERINFO*)pMsg;

	cPlayer* player = new cPlayer(this);
	if( mPlayerMap.Insert( pmsg->PlayerInfo.CharacterIdx, player ) == false )
	{
		delete player;
		return false;
	}

	sPlayerInfo baseInfo = pmsg->PlayerInfo;
	sPlayerExrInfo exrInfo = pmsg->Exrinfo;


	NiPoint3 pos = NiPoint3( pmsg->XPos, pmsg->YPos, 0.0f );
	if( WORLDMAN->CalcHeight( mFolderIdx, &pos.z, pos.x, pos.y ) == false )
		assert(0);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0.0f, 0.0f, pmsg->RotAngle );

	if( player->Init( &baseInfo, &pmsg->WearInfo, &pmsg->WeaponInfo, &exrInfo, pos, rot ) == false )
	{
		assert(0);
		DelPlayer( pmsg->PlayerInfo.CharacterIdx );
		return false;
	}

	return true;
}

bool cObjectManager::AddMonster( sMonsterData* data )
{
	cMonster* monster = new cMonster(this);


	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3((float)data->mPosX, (float)data->mPosY, 0.0f);
	if( WORLDMAN->CalcHeight( mFolderIdx, &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, data->mDirection );

	if( monster->Init( data, pos, rot ) == false )
	{
		delete monster;
		return false;
	}

	if( mMonsterMap.Insert( data->mMonsterIdx, monster ) == false )
	{
		delete monster;
		return false;
	}

	return true;
}

bool cObjectManager::AddNpc( sNpcData* data )
{
	cNpc* npc = new cNpc(this);

	/// 위치 정보 생성..
	NiPoint3 pos = NiPoint3( data->mPosX, data->mPosY, 0.0f );
	if( WORLDMAN->CalcHeight( mFolderIdx, &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	/// 회전 정보 생성..
	NiMatrix3 rot;
	rot.FromEulerAnglesXYZ( 0, 0, data->mDirection );

	if( npc->Init( data->mNpcIdx, data->mNpcClassIdx, pos, rot ) == false )
	{
		delete npc;
		return false;
	}

	if( mNpcMap.Insert( data->mNpcIdx, npc ) == false )
	{
		delete npc;
		return false;
	}

	return true;
}

bool cObjectManager::AddPortal( unsigned long idx, unsigned long posIdx, NiPoint3 pos, NiMatrix3 rot )
{
	if( idx == 0 )
		mPortalGen++;

	cPortal* portal = new cPortal(this);
	if( portal->Init( posIdx, pos, rot ) == false )
	{
		delete portal;
		return false;
	}

	if( mPortalMap.Insert( mPortalGen, portal ) == false )
	{
		delete portal;
		return false;
	}

	return true;
}

bool cObjectManager::AddGather( unsigned long idx )
{
	cGather* gather = new cGather(this);

	if( mGatherMap.Insert( idx, gather ) == false )
	{
		delete gather;
		return false;
	}

	return true;
}

bool cObjectManager::AddTotem( unsigned long idx )
{
	cTotem* totem = new cTotem(this);

	if( mTotemMap.Insert( idx, totem ) == false )
	{
		delete totem;
		return false;
	}

	return true;
}

bool cObjectManager::AddDropItem( MSGROOT* pMsg )
{
	MSG_ITEM_INFO* msg = (MSG_ITEM_INFO*)pMsg;

	NiPoint3 pos = NiPoint3( msg->itemDrop.xPos, msg->itemDrop.yPos, 0.0f );
	if( WORLDMAN->CalcHeight( mFolderIdx, &pos.z, pos.x, pos.y) == false )
		assert(NULL);

	cDropItem* item = new cDropItem(this);
	if( item->Init( msg->itemDrop.idx, pos, msg->owner, msg->timeToOwner, 
		msg->party, msg->timeToParty, (msg->rare >= eTOOLTIP_RARE)? true:false ) == false )
		return false;

	if( mDropItemMap.Insert(msg->itemDrop.idx, item) == false )
	{
		delete item;
		return false;
	}

	return true;
}


void cObjectManager::DelPlayer( unsigned long idx )
{
	cPlayer* player = (cPlayer*)mPlayerMap.GetAt(idx);
	SAFE_DELETE(player);

	mPlayerMap.Erase( idx );
}

void cObjectManager::DelMonster( unsigned long idx )
{
	cMonster* monster = (cMonster*)mMonsterMap.GetAt(idx);
	SAFE_DELETE(monster);

	mMonsterMap.Erase( idx );
}

void cObjectManager::DelNpc( unsigned long idx )
{
	cNpc* npc = (cNpc*)mNpcMap.GetAt(idx);
	SAFE_DELETE(npc);

	mNpcMap.Erase( idx );
}

void cObjectManager::DelPortal( unsigned long idx )
{
	cPortal* portal = (cPortal*)mPortalMap.GetAt(idx);
	SAFE_DELETE(portal);

	mPortalMap.Erase( idx );
}

void cObjectManager::DelGather( unsigned long idx )
{
	cGather* gather = (cGather*)mGatherMap.GetAt(idx);
	SAFE_DELETE(gather);

	mGatherMap.Erase( idx );
}

void cObjectManager::DelTotem( unsigned long idx )
{
	cTotem* totem = (cTotem*)mTotemMap.GetAt(idx);
	SAFE_DELETE(totem);

	mTotemMap.Erase( idx );
}

void cObjectManager::DelDropItem( unsigned long idx )
{
	cDropItem* item = (cDropItem*)mDropItemMap.GetAt(idx);
	SAFE_DELETE(item);

	mDropItemMap.Erase(idx);
}


cPortal* cObjectManager::GetMapChangePortal( unsigned short mapIdx, NiPoint3 pos )
{
	cPortal* ret = 0;

	cPointerHashMap::cIterator i, end;

	i = mPortalMap.Begin();
	end = mPortalMap.End();
	for( ; i != end; ++i )
	{
		cPortal* portal = (cPortal*)(*i).mSecond;
		if( portal )
		{
			unsigned short mapNum = GAMERESOURCEMAN->GetStageChangeMap( portal->GetTargetScriptIdx() );
			if( mapNum == 0 || mapIdx == mapNum )
				continue;

			if( ret == 0 )
				ret = portal;
			else
			{
				float len1 = (ret->GetPos() - pos).Length();
				float len2 = (portal->GetPos() - pos).Length();

				if( len1 > len2 )
					ret = portal;
			}

		}
	}
	return ret;
}

cMonster* cObjectManager::GetNearMonster( unsigned char level, NiPoint3 pos )
{
	::srand( ::rand()%500000 );

	if( mMonsterMap.IsEmpty() )
		return 0;

	cPointerHashMap::cIterator i = mMonsterMap.Begin();
	cPointerHashMap::cIterator end = mMonsterMap.End();

	cMonster* monster = 0;
	for(; i !=end; ++i )
	{
		cMonster* mon = (cMonster*)(*i).mSecond;

		sObject* pInfo = mon->GetTargetInfo();
		if( pInfo->index == mHero->GetObjectID() )
			return mon;

		if( (mon->GetLevel() + 2 < level) || mon->GetLevel()  > level+ 3 )
			continue;
		if( mon->GetHP() < mon->GetMaxHP()*0.95f )
			continue;

		monster = mon;
		if( ::rand()%100 < 50 )
			break;
	}

	return monster;
}

cDropItem* cObjectManager::GetNearDropItem( unsigned long idx, NiPoint3 pos )
{
	cPointerHashMap::cIterator i, end;
	i = mDropItemMap.Begin();
	end = mDropItemMap.End();

	cDropItem* ret = 0;
	for( ; i != end; ++i )
	{
		cDropItem* item = (cDropItem*)(*i).mSecond;
		if( item == 0 )
			continue;
		if( item->IsOwner(idx) == false )
			continue;

		if( ret == 0 )
			ret = item;
		else
		{
			float len1 = (ret->GetPos() - pos).Length();
			float len2 = (item->GetPos() - pos).Length();

			if( len1 > len2 )
				ret = item;
		}
	}
	return ret;
}


cObject* cObjectManager::GetObject( unsigned char type, unsigned long idx )
{
	switch(type)
	{
	case eOBJECTTYPE_PLAYER: return GetPlayer(idx);
	case eOBJECTTYPE_MONSTER: return GetMonster(idx);
	case eOBJECTTYPE_NPC: return GetNpc(idx);
	case eOBJECTTYPE_PORTAL: return GetPortal(idx);
	case eOBJECTTYPE_GATHERING: return GetGather(idx);
	case eOBJECTTYPE_TOTEM: return GetTotem(idx);
	}
	return 0;
}

cPlayer* cObjectManager::GetPlayer( unsigned long idx )
{
	return (cPlayer*)mPlayerMap.GetAt(idx);
}

cMonster* cObjectManager::GetMonster( unsigned long idx )
{
	return (cMonster*)mMonsterMap.GetAt(idx);
}

cNpc* cObjectManager::GetNpc( unsigned long idx )
{
	return (cNpc*)mNpcMap.GetAt(idx);
}

cPortal* cObjectManager::GetPortal( unsigned long idx )
{
	return (cPortal*)mPortalMap.GetAt(idx);
}

cGather* cObjectManager::GetGather( unsigned long idx )
{
	return (cGather*)mGatherMap.GetAt(idx);
}

cTotem* cObjectManager::GetTotem( unsigned long idx )
{
	return (cTotem*)mTotemMap.GetAt(idx);

}