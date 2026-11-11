// Include
#include "gamesrv.h"
#include "partyjoinpool.h"

// Local definitions
#define PARTY_JOIN_DELAY		15000

// Global data
cPartyJoinPool* cPartyJoinPool::mpPartyJoinPool = NULL;

// PartyJoinNameCmpWchar Prototype
int WINAPI PartyJoinNameCmpWchar(void* arg1, void* arg2)
{
	return wcscmp( ((PartyJoin*)arg1)->name, ((PartyJoin*)arg2)->name );
}

// PartyJoinNameCmpValue Prototype
int WINAPI PartyJoinNameCmpValue(void* arg1, void* arg2)
{
	return wcscmp( ((PartyJoin*)arg1)->name, (wchar_t*)arg2 );
}

// cPartyJoinPool Constructor
cPartyJoinPool::cPartyJoinPool( )
{
	// Global Variable
	mpPartyJoinPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;

	mPartyJoinRoot.pool = NULL;
	mPartyJoinRoot.root = NULL;
}

// ~cPartyJoinPool Destructor.
cPartyJoinPool::~cPartyJoinPool(void)
{
	// cPartyJoinPool를 해제.
	Shutdown( );

	// Global Variable
	mpPartyJoinPool = NULL;
}

// Shutdown Method
void cPartyJoinPool::Shutdown()
{
	PartyJoin* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PartyJoin*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// Search Method
PartyJoin* cPartyJoinPool::Search(wchar_t* name)
{
	return (PartyJoin*)MEMORY_POOL->SearchNode( &mPartyJoinRoot, name, &PartyJoinNameCmpValue );
}

// Get Method - 선형리스트의 HEAD를 넘겨준다.
// Release와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PartyJoin* cPartyJoinPool::Get(wchar_t* name)
{
	PartyJoin* join = (PartyJoin*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PartyJoin) );
	if ( join != NULL )
	{
		wcscpy( join->name, name );
		join->delay = GetTickCount( ) + PARTY_JOIN_DELAY;

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &mPartyJoinRoot.root, join, PartyJoinNameCmpWchar ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mPartyJoinRoot.pool, join );
			return join;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, join );
			return NULL;		// 연결실패.
		}
	}
	return join;
}

// Release Method - 선형리스트의 HEAD로 회수한다.
// Get와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cPartyJoinPool::Release(PartyJoin* join)
{
	memset( &join->name, 0, sizeof(TB_INVENTORY) );
	join->partyIdx = 0;
	memset( &join->playerDat, 0, sizeof(TB_INVENTORY) );

	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&mPartyJoinRoot.root, (PerNode*)join );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&mPartyJoinRoot.pool, (PerNode*)join );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)join );
}
