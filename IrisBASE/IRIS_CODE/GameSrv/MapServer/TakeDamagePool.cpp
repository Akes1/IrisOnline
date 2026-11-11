#include "GameSrv.h"
#include "TakeDamagePool.h"

// Local definitions
#define SetLeft(P,L)    { (P)->left  = L; if(L) (L)->parent = P; }
#define SetRight(P,R)   { (P)->right = R; if(R) (R)->parent = P; }

// Global data
cTakeDamagePool* cTakeDamagePool::mpTakeDamagePool = NULL;

// TakeDamageCmpIndex Method
int WINAPI TakeDamageCmpIndex(void* arg1, void* arg2)
{
	PerTakeDamage* perTakeDamage1 = (PerTakeDamage*)arg1;
	PerTakeDamage* perTakeDamage2 = (PerTakeDamage*)arg2;
	return (perTakeDamage1->playerIdx - perTakeDamage2->playerIdx);
}

// TakeDamageCmpValue Method
int WINAPI TakeDamageCmpValue(void* arg1, void* arg2)
{
	return ((PerTakeDamage*)arg1)->playerIdx - (*(long*)arg2);
}

cTakeDamagePool::cTakeDamagePool()
{
	// Global Variable
	mpTakeDamagePool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

cTakeDamagePool::~cTakeDamagePool()
{
	// cTakeDamagePool를 해제.
	Shutdown();

	// Global Variable
	mpTakeDamagePool = NULL;
}

// Shutdown Method
void cTakeDamagePool::Shutdown()
{
	PerTakeDamage* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerTakeDamage*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp );
	}
}

// GetTakeDamage Method
PerTakeDamage* cTakeDamagePool::GetTakeDamage(TakeDamageRoot* takeDamageRoot, unsigned long index)
{
	return (PerTakeDamage*)MEMORY_POOL->SearchNode( takeDamageRoot, &index, &TakeDamageCmpValue );
}

// ReleaseTakeDamage Method
void cTakeDamagePool::ReleaseTakeDamage(TakeDamageRoot* takeDamageRoot, unsigned long index)
{
	PerTakeDamage* perTakeDamage = (PerTakeDamage*)MEMORY_POOL->SearchNode( takeDamageRoot, &index, TakeDamageCmpValue );
	if ( perTakeDamage )
	{
		// BST - 연결종료.
		MEMORY_POOL->DetachBst( (PerNode**)&takeDamageRoot->root, (PerNode*)perTakeDamage );

		// 페이지 된 풀 사용량을 차감.
		MEMORY_POOL->DetachPool( (PerNode**)&takeDamageRoot->pool, (PerNode*)perTakeDamage );

		// 페이지 안된 풀 사용량을 증감.
		MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perTakeDamage );

		if( takeDamageRoot->count != 0 )
			takeDamageRoot->count--;
	}
	else
	{
		assert(0);
	}
}

// AddTakeDamage Method
PerTakeDamage* cTakeDamagePool::AddTakeDamage(TakeDamageRoot* takeDamageRoot, unsigned long index, unsigned long damage, long distress)
{
	PerTakeDamage* perTakeDamage = (PerTakeDamage*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerTakeDamage) );
	if ( perTakeDamage )
	{
		perTakeDamage->playerIdx = index; // Index - Key

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &takeDamageRoot->root, (PerNode*)perTakeDamage, &TakeDamageCmpIndex ) )
		{
			MEMORY_POOL->AttachPool( &takeDamageRoot->pool, perTakeDamage );

			perTakeDamage->takeDamage = damage;

			if ( distress < 0 )
				perTakeDamage->distressPoint = 0; 
			else
				perTakeDamage->distressPoint = distress; 

			takeDamageRoot->count++;

			return perTakeDamage;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perTakeDamage );
			assert(0);
			return NULL;		// 연결실패.
		}
	}
	return perTakeDamage;
}
