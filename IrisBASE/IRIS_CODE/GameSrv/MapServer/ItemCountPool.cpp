// Include
#include "gamesrv.h"
#include "ItemCountPool.h"

// Local definitions

// Global data
cItemCountPool* cItemCountPool::mpItemCountPool = NULL;

// ItemCountCmpIndex Prototype
int WINAPI ItemCountCmpIndex(void* arg1, void* arg2)
{
	return ((PerItemCount*)arg1)->index - ((PerItemCount*)arg2)->index;
}

// ItemCountCmpValue Prototype
int WINAPI ItemCountCmpValue(void* arg1, void* arg2)
{
	return ((PerItemCount*)arg1)->index - (*(long*)arg2);
}

// cItemCountPool Constructor
cItemCountPool::cItemCountPool( )
{
	// Global Variable
	mpItemCountPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

// ~cItemCountPool Destructor.
cItemCountPool::~cItemCountPool(void)
{
	// cItemCountPool를 해제.
	Shutdown();

	// Global Variable
	mpItemCountPool = NULL;
}

// Shutdown Method
void cItemCountPool::Shutdown()
{
	PerItemCount* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerItemCount*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// SearchItemCount Method
PerItemCount* cItemCountPool::SearchItemCount(ItemCountRoot* root, long index)
{
	return (PerItemCount*)MEMORY_POOL->SearchNode( root, &index, &ItemCountCmpValue );
}

// UpdateItemCount Method
PerItemCount* cItemCountPool::UpdateItemCount(ItemCountRoot* root, long index, short number, short count)
{
	PerItemCount* perItemCount = (PerItemCount*)MEMORY_POOL->SearchNode( root, &index, &ItemCountCmpValue );
	if ( perItemCount != NULL )
	{
		if ( number >= INVENTORY_EQUIP_BEGIN && number <= INVENTORY_EQUIP_END )
		{
			perItemCount->equip += count;
		}
		else if ( number >= INVENTORY_BAG_BEGIN && number <= INVENTORY_BAG_END_QUEST )
		{
			perItemCount->bag += count;
		}
		else
		{
			perItemCount->etc += count;
		}

		perItemCount->sum += count;

		if ( perItemCount->sum > 0 )
		{
			return perItemCount;
		}
		else
		{
			if ( perItemCount->sum < 0 )
			{
				NETWORK2->PostServerEvent( "ERROR cItemCountPool::UpdateItemCount:Index(=%d), Bag(=%d), Sum(=%d)"
										  ,perItemCount->index
										  ,perItemCount->bag
										  ,perItemCount->sum );
			}
			ReleaseItemCount( root, perItemCount );
			return NULL;
		}
	}
	else
	{
		perItemCount = GetItemCount( root, index, number, count );
		return perItemCount;
	}
}

// GetItemCount Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseItemCount와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerItemCount* cItemCountPool::GetItemCount(ItemCountRoot* root, long index, short number, short count)
{
	PerItemCount* perItemCount = (PerItemCount*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerItemCount) );
	if ( perItemCount )
	{
		perItemCount->index = index; // ItemCount Key(Index)

		if ( number >= INVENTORY_EQUIP_BEGIN && number <= INVENTORY_EQUIP_END )
		{
			perItemCount->equip += count;
		}
		else if ( number >= INVENTORY_BAG_BEGIN && number <= INVENTORY_BAG_END_QUEST )
		{
			perItemCount->bag += count;
		}
		else
		{
			perItemCount->etc += count;
		}

		perItemCount->sum = count;

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &root->root, perItemCount, ItemCountCmpIndex ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&root->pool, perItemCount );
			return perItemCount;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perItemCount );
			return NULL;		// 연결실패.
		}
	}
	return perItemCount;
}

// ReleaseItemCount Method - 선형리스트의 HEAD로 회수한다.
// GetItemCount와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cItemCountPool::ReleaseItemCount(ItemCountRoot* root, PerItemCount* perItemCount)
{
	perItemCount->index = 0;
	perItemCount->equip = 0;
	perItemCount->bag   = 0;
	perItemCount->etc   = 0;
	perItemCount->sum   = 0;

	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&root->root, (PerNode*)perItemCount );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&root->pool, (PerNode*)perItemCount );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perItemCount );
}
