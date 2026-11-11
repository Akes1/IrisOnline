// Include
#include "gamesrv.h"
#include "playernamepool.h"

// Local definitions

// Global data
cPlayerNamePool* cPlayerNamePool::mpPlayerNamePool = NULL;

// PlayerNameCmpWChar Prototype
int WINAPI PlayerNameCmpWChar(void* arg1, void* arg2)
{
	return wcscmp( ((PerPlayerName*)arg1)->name, ((PerPlayerName*)arg2)->name );
}

// PlayerNameCmpValue Prototype
int WINAPI PlayerNameCmpValue(void* arg1, void* arg2)
{
	return wcscmp( ((PerPlayerName*)arg1)->name, (wchar_t*)arg2 );
}

// cPlayerNamePool Constructor
cPlayerNamePool::cPlayerNamePool( )
{
	// Global Variable
	mpPlayerNamePool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;

	mPlayerNameRoot.pool = NULL;
	mPlayerNameRoot.root = NULL;
}

// ~cPlayerNamePool Destructor.
cPlayerNamePool::~cPlayerNamePool(void)
{
	// cPlayerNamePool를 해제.
	Shutdown();

	// Global Variable
	mpPlayerNamePool = NULL;
}

// Shutdown Method
void cPlayerNamePool::Shutdown()
{
	PerPlayerName* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerPlayerName*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// Search Method
PerPlayerName* cPlayerNamePool::Search(wchar_t* name)
{
	return (PerPlayerName*)MEMORY_POOL->SearchNode( &mPlayerNameRoot, name, &PlayerNameCmpValue );
}

// GetPlayerName Method - 선형리스트의 HEAD를 넘겨준다.
// ReleasePlayerName와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerPlayerName* cPlayerNamePool::GetPlayerName(ULONG connectionIdx, ULONG characterIdx, wchar_t* name)
{
	PerPlayerName* perPlayerName = (PerPlayerName*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerPlayerName) );
	if ( perPlayerName )
	{
		perPlayerName->connectionIdx = connectionIdx;
		perPlayerName->characterIdx  = characterIdx;
		wcscpy( perPlayerName->name, name );

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &mPlayerNameRoot.root, perPlayerName, PlayerNameCmpWChar ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mPlayerNameRoot.pool, perPlayerName );
			return perPlayerName;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perPlayerName );
			return NULL;		// 연결실패.
		}
	}
	return perPlayerName;
}

// ReleasePlayerName Method - 선형리스트의 HEAD로 회수한다.
// GetPlayerName와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cPlayerNamePool::ReleasePlayerName(PerPlayerName* perPlayerName)
{
	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&mPlayerNameRoot.root, (PerNode*)perPlayerName );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&mPlayerNameRoot.pool, (PerNode*)perPlayerName );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perPlayerName );
}
