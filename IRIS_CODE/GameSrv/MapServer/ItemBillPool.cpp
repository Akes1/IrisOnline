// Include
#include "gamesrv.h"
#include "itembillpool.h"

// Local definitions

// Global data
cItemBillPool* cItemBillPool::mpItemBillPool = NULL;

// ItemBillCmpIndex Prototype
int WINAPI ItemBillCmpIndex(void* arg1, void* arg2)
{
	return ((PerItemBill*)arg1)->inventoryIdx - ((PerItemBill*)arg2)->inventoryIdx;
}

// ItemBillCmpValue Prototype
int WINAPI ItemBillCmpValue(void* arg1, void* arg2)
{
	return ((PerItemBill*)arg1)->inventoryIdx - (*(long*)arg2);
}

// cItemBillPool Constructor
cItemBillPool::cItemBillPool( )
{
	// Global Variable
	mpItemBillPool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

// ~cItemBillPool Destructor.
cItemBillPool::~cItemBillPool(void)
{
	// cItemBillPool를 해제.
	Shutdown();

	// Global Variable
	mpItemBillPool = NULL;
}

// Shutdown Method
void cItemBillPool::Shutdown()
{
	PerItemBill* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerItemBill*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// SearchItemBill Method
PerItemBill* cItemBillPool::SearchItemBill(ItemBillRoot* root, long inventoryIdx)
{
	return (PerItemBill*)MEMORY_POOL->SearchNode( root, &inventoryIdx, &ItemBillCmpValue );
}

// GetItemBill Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseItemBill와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerItemBill* cItemBillPool::GetItemBill(ItemBillRoot* root, long inventoryIdx)
{
	PerItemBill* itemBill = (PerItemBill*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerItemBill) );
	if ( itemBill )
	{
		itemBill->inventoryIdx = inventoryIdx;

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &root->root, itemBill, ItemBillCmpIndex ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&root->pool, itemBill );
			return itemBill;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, itemBill );
			return NULL;		// 연결실패.
		}
	}
	return itemBill;
}

// ReleaseItemBill Method - 선형리스트의 HEAD로 회수한다.
// GetItemBill와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cItemBillPool::ReleaseItemBill(ItemBillRoot* root, PerItemBill* itemBill)
{
	itemBill->idx          = 0;
	itemBill->type         = 0;
	itemBill->validDate    = 0;
	itemBill->validTime    = 0;
	itemBill->inventoryIdx = 0;
	itemBill->timeBegin    = 0;
	itemBill->timeEnd      = 0;
	itemBill->apply        = 0;

	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&root->root, (PerNode*)itemBill );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&root->pool, (PerNode*)itemBill );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)itemBill );
}
