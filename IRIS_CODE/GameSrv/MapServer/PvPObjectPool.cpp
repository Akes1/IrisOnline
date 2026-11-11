#include "GameSrv.h"
#include "PvPObjectPool.h"
#include "BaseDeathMatch.h"
#include "PVPManager.h"

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }

// Global data
cPvPObjectPool* cPvPObjectPool::mpPvPObjectPool = NULL;

// PvPObjectCmpIndex Method
int WINAPI PvPObjectCmpIndex(void* arg1, void* arg2)
{
	cBaseDeathMatch* pPvPObject1 = (cBaseDeathMatch*)arg1;
	cBaseDeathMatch* pPvPObject2 = (cBaseDeathMatch*)arg2;
	return (pPvPObject1->GetDMIdx() - pPvPObject2->GetDMIdx());
}

// PvPObjectCmpValue Method
int WINAPI PvPObjectCmpValue(void* arg1, void* arg2)
{
	return ((cBaseDeathMatch*)arg1)->GetDMIdx() - (*(long*)arg2);
}

cPvPObjectPool::cPvPObjectPool()
{
	// Global Variable
	mpPvPObjectPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

cPvPObjectPool::~cPvPObjectPool()
{
	// Pool를 해제.
	Shutdown();

	// Global Variable
	mpPvPObjectPool = NULL;
}

// Shutdown Method
void cPvPObjectPool::Shutdown()
{
	cBaseDeathMatch* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (cBaseDeathMatch*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp );
	}
}

// GetPvPObject Method
cBaseDeathMatch* cPvPObjectPool::GetPvPObject(sPvPObjectRoot* pPvPObjectRoot, unsigned long index)
{
	return (cBaseDeathMatch*)MEMORY_POOL->SearchNode( pPvPObjectRoot, &index, &PvPObjectCmpValue );
}

// ReleasePvPObject Method
void cPvPObjectPool::ReleasePvPObject(sPvPObjectRoot* pPvPObjectRoot, unsigned long index)
{
	cBaseDeathMatch* pPvPObject = (cBaseDeathMatch*)MEMORY_POOL->SearchNode( pPvPObjectRoot, &index, PvPObjectCmpValue );
	if ( pPvPObject )
	{
		// BST - 연결종료.
		MEMORY_POOL->DetachBst( (PerNode**)&pPvPObjectRoot->root, (PerNode*)pPvPObject );

		// 페이지 된 풀 사용량을 차감.
		MEMORY_POOL->DetachPool( (PerNode**)&pPvPObjectRoot->pool, (PerNode*)pPvPObject );

		MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)pPvPObject );

		pPvPObjectRoot->count--;

		SAFE_DELETE( pPvPObject );
	}
	else
	{
		assert(0);
	}
}

// NewDMObject Method
cDeathMatchObject* cPvPObjectPool::NewDMObject()
{
	return (cDeathMatchObject*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(cDeathMatchObject) );
}

// NewDM2Object Method
cDeathMatch2Object* cPvPObjectPool::NewDM2Object()
{
	return (cDeathMatch2Object*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(cDeathMatch2Object) );
}

// PvPAttachPool Method
bool cPvPObjectPool::PvPAttachPool( sPvPObjectRoot* pPvPObjectRoot, cBaseDeathMatch* pPvPObject )
{
	if( pPvPObject == NULL )
		return false;

	// BST-연결시도.
	if ( MEMORY_POOL->AttachBst( &pPvPObjectRoot->root, pPvPObject, &PvPObjectCmpIndex ) )
	{
		MEMORY_POOL->AttachPool( &pPvPObjectRoot->pool, pPvPObject );

		pPvPObjectRoot->count++;

		return true;	// 연결성공.
	}
	else
	{
		MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)pPvPObject );
		assert(0);
		return false;
	}
}
