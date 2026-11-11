// Include
#include "gamesrv.h"
#include "PvPPlayerListPool.h"

cPvPPlayerListPool* cPvPPlayerListPool::mpPvPPlayerListPool = NULL;

int WINAPI PvPPlayerCmpIndex(void* arg1, void* arg2)
{
	return ((PerPvPPlayer*)arg1)->mPlayerIdx - ((PerPvPPlayer*)arg2)->mPlayerIdx;
}

int WINAPI PvPPlayerCmpValue(void* arg1, void* arg2)
{
	return ((PerPvPPlayer*)arg1)->mPlayerIdx - (*(long*)arg2);
}

cPvPPlayerListPool::cPvPPlayerListPool( )
{
	// Global Variable
	mpPvPPlayerListPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

cPvPPlayerListPool::~cPvPPlayerListPool(void)
{
	// cItemCountPool를 해제.
	Shutdown();

	// Global Variable
	mpPvPPlayerListPool = NULL;
}

void cPvPPlayerListPool::Shutdown()
{
	PerPvPPlayer* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerPvPPlayer*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

PerPvPPlayer* cPvPPlayerListPool::AddPlayer( PvPPlayerRoot* pPvPPlayerRoot, short channel, unsigned long playerIdx, unsigned char level, 
											unsigned char forceType, bool selection )
{
	PerPvPPlayer* perPvPPlayer = (PerPvPPlayer*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerPvPPlayer) );
	if ( perPvPPlayer )
	{
		perPvPPlayer->mPlayerIdx = playerIdx; // Index - Key

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &pPvPPlayerRoot->root, (PerNode*)perPvPPlayer, &PvPPlayerCmpIndex ) )
		{
			MEMORY_POOL->AttachPool( &pPvPPlayerRoot->pool, perPvPPlayer );

			perPvPPlayer->mCharacterChannel = channel;
			perPvPPlayer->mLevel = level;
			perPvPPlayer->mForceType = forceType;
			perPvPPlayer->mSelection = selection;

			pPvPPlayerRoot->count++;

			return perPvPPlayer;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perPvPPlayer );
			assert(0);
			return NULL;		// 연결실패.
		}
	}
	return perPvPPlayer;
}

PerPvPPlayer* cPvPPlayerListPool::GetPlayer(PvPPlayerRoot* root, long index)
{
	return (PerPvPPlayer*)MEMORY_POOL->SearchNode( root, &index, &PvPPlayerCmpValue );
}

void cPvPPlayerListPool::ReleasePlayer( PvPPlayerRoot* root, PerPvPPlayer* perPlayer )
{
	perPlayer->mCharacterChannel = 0;
	perPlayer->mPlayerIdx = 0;
	perPlayer->mLevel = 0;
	perPlayer->mForceType = 0;
	perPlayer->mSelection = 0;

	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&root->root, (PerNode*)perPlayer );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&root->pool, (PerNode*)perPlayer );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perPlayer );
	--root->count;
}
