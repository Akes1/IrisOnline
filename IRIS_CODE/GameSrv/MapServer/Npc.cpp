#include "gamesrv.h"

#include "stdafx.h"
#include "Npc.h"
#include "ObjectManager.h"
#include "NpcScript.h"

#include "player.h"

cNpc::cNpc(void) : cBaseObject( eOBJECTTYPE_NPC )
{
}

cNpc::~cNpc(void)
{
}

void* cNpc::operator new( size_t n )
{
	if( n != sizeof(cNpc) ) 
	{
		assert(0);
		return NULL;
	}

	return OBJECTMANAGER->AllocNpc();
}

void cNpc::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0 )
	{
		assert(0);
		return;
	}

	if( n != sizeof(cNpc) ) 
	{
		assert(0);
		return;
	}

	OBJECTMANAGER->FreeNpc( static_cast<cNpc*>(ptr) );

	return;
}

bool cNpc::Init( int uniqueIdx, unsigned long classIdx, unsigned short mapNumber, float x, float y, float dir )
{
	/// 고유번호 등록
	mObject.index = uniqueIdx;
	mNpcClassIdx = classIdx;

	SetMapNumber( mapNumber );

	/// 몬스터 리스트 정보
	sNPCList* pNpcListScript = NPCSCRIPT->GetNPCList( classIdx );
	if ( !pNpcListScript )	
	{ 
		NETWORK2->PostServerEvent( "cNpc::Init GetNPCList( %d ) = NULL", classIdx );
		return false; 
	}

	mpNpcList = pNpcListScript;

	SetPos( x, y );
	mDirection = dir;
	return true;
}

// GetNpcData Method
void cNpc::GetNpcData( cPlayer* player, sNpcData* npcData )
{
	if( player == NULL || npcData == NULL )
		return;

	npcData->mNpcIdx      = mObject.index;
	npcData->mNpcClassIdx = mpNpcList->mNpcClassIdx;
	npcData->mPosX        = mObjectPos.x;
	npcData->mPosY        = mObjectPos.y;
	npcData->mDirection   = mDirection;
}

// SendSightIn Method
bool cNpc::SendSightIn(char category, char protocol, cPlayer* player )
{
	HANDLE        handle  = NULL;
	MSG_NPC_INFO* sendMsg = (MSG_NPC_INFO*)NETWORK2->GetMsgRoot( &handle, player->GetConnectionIdx( ), category, protocol );
	if ( sendMsg != NULL )
	{
		sNpcData* npcData = sendMsg->mNpcInfo;

		sendMsg->mCount = 1;
		GetNpcData( player, npcData );

		return NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
	}
	return false;
}

// SendSightOut Method
bool cNpc::SendSightOut(char category, char protocol, unsigned long connectionIdx)
{
	HANDLE      handle  = NULL;
	MSG_NPCIDX* sendMsg = (MSG_NPCIDX*)NETWORK2->GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->mNpcIdx = mObject.index; // GetObjectID Method
		return NETWORK2->SendMsgRoot( handle, sizeof(MSG_NPCIDX) );
	}
	return false;
}
