// Include
#include "gamesrv.h"
#include "themeuserpool.h"

// Local definitions

// Global data
cThemeUserPool* cThemeUserPool::mpThemeUserPool = NULL;

// ThemeUserCmpNode Prototype
int WINAPI ThemeUserCmpNode(void* arg1, void* arg2)
{
	return ((PerThemeUser*)arg1)->cid - ((PerThemeUser*)arg2)->cid;
}

// ThemeUserCmpCID Prototype
int WINAPI ThemeUserCmpCID(void* arg1, void* arg2)
{
	return ((PerThemeUser*)arg1)->cid - (*(long*)arg2);
}

// cThemeUserPool Constructor
cThemeUserPool::cThemeUserPool( )
{
	// Global Variable
	mpThemeUserPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

// ~cThemeUserPool Destructor.
cThemeUserPool::~cThemeUserPool(void)
{
	// cThemeUserPool를 해제.
	Shutdown();

	// Global Variable
	mpThemeUserPool = NULL;
}

// Shutdown Method
void cThemeUserPool::Shutdown()
{
	PerThemeUser* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerThemeUser*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// SearchThemeUser Method
PerThemeUser* cThemeUserPool::SearchThemeUser(ThemeUserRoot* themeUserRoot, long cid)
{
	PerThemeUser* perThemeUser = (PerThemeUser*)MEMORY_POOL->SearchNode( themeUserRoot, &cid, &ThemeUserCmpCID );
	if ( perThemeUser != NULL )
	{
		time_t ltime;
		time( &ltime );

		if ( difftime( perThemeUser->validThru, ltime ) <= 0 )
		{
			ReleaseThemeUser( themeUserRoot, perThemeUser );
			return NULL;
		}
	}
	return perThemeUser;
}

// IsThemeUser Method
bool cThemeUserPool::IsThemeUser(ThemeUserRoot* themeUserRoot, long cid)
{
	return SearchThemeUser( themeUserRoot, cid ) != NULL ? true : false;
}

// GetThemeUser Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseThemeUser와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerThemeUser* cThemeUserPool::GetThemeUser(ThemeUserRoot* themeUserRoot, long cid)
{
	PerThemeUser* perThemeUser = (PerThemeUser*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerThemeUser) );
	if ( perThemeUser )
	{
		perThemeUser->cid = cid; // Key(CID)

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &themeUserRoot->root, perThemeUser, ThemeUserCmpNode ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&themeUserRoot->pool, perThemeUser );
			return perThemeUser;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perThemeUser );
			return NULL;		// 연결실패.
		}
	}
	return perThemeUser;
}

// ReleaseThemeUser Method - 선형리스트의 HEAD로 회수한다.
// GetThemeUser와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cThemeUserPool::ReleaseThemeUser(ThemeUserRoot* themeUserRoot, PerThemeUser* perThemeUser)
{
	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&themeUserRoot->root, (PerNode*)perThemeUser );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&themeUserRoot->pool, (PerNode*)perThemeUser );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perThemeUser );
}
