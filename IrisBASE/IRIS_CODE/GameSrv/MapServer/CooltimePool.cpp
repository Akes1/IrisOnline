// Include
#include "gamesrv.h"
#include "ItemManager.h"
#include "CooltimePool.h"

// Local definitions

// Global data
cCooltimePool* cCooltimePool::mpCooltimePool = NULL;

// CooltimeCmpIndex Prototype
int WINAPI CooltimeCmpIndex(void* arg1, void* arg2)
{
	return ((PerCooltime*)arg1)->index - ((PerCooltime*)arg2)->index;
}

// CooltimeCmpValue Prototype
int WINAPI CooltimeCmpValue(void* arg1, void* arg2)
{
	return ((PerCooltime*)arg1)->index - (*(long*)arg2);
}

// cCooltimePool Constructor
cCooltimePool::cCooltimePool( )
{
	// Global Variable
	mpCooltimePool = this;

	// Pool Usage Pointer
	mNonPagedPoolUsage = NULL;
}

// ~cCooltimePool Destructor.
cCooltimePool::~cCooltimePool(void)
{
	// cCooltimePool를 해제.
	Shutdown();

	// Global Variable
	mpCooltimePool = NULL;
}

// Shutdown Method
void cCooltimePool::Shutdown()
{
	PerCooltime* temp;
	// Pool Usage Shutdown - NonPagedPoolUsage
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = (PerCooltime*)mNonPagedPoolUsage->next;
		MEMORY_POOL->FreeNode( (void**)&temp  );
	}
}

// IsCooltime Method
bool cCooltimePool::IsCooltime(CooltimeRoot* cooltimeRoot, long index)
{
	PerCooltime* perCooltime = (PerCooltime*)MEMORY_POOL->SearchNode( cooltimeRoot, &index, &CooltimeCmpValue );
	if ( perCooltime != NULL )
	{
		time_t ltime;
		time( &ltime );

		if ( difftime( perCooltime->validThru, ltime ) <= 0 )
			ReleaseCooltime( cooltimeRoot, perCooltime );
		else
			return true;
	}
	return false;
}

// ApplyCooltime Method
void cCooltimePool::ApplyCooltime(CooltimeRoot* cooltimeRoot, TB_INVENTORY* inventory)
{
	TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( inventory->itemDefineIdx );
	if ( itemAbility == NULL )
		return;

	TB_ITEM_COOLTIME2* itemCooltime2 = ITEMMANAGER->GetItemCooltime2( inventory->itemDefineIdx );
	PerCooltime*       perCooltime   = NULL;
	time_t             ltime;
	long               elapsedtime;

	time( &ltime );

	if ( itemCooltime2 != NULL )
	{
		// 그룹 쿨타임
		TB_ITEM_COOLTIME1* itemCooltime1 = itemCooltime2->cooltime1;
		while ( itemCooltime1 != NULL )
		{
			perCooltime = (PerCooltime*)MEMORY_POOL->SearchNode( cooltimeRoot, &itemCooltime1->itemDefineIndex, &CooltimeCmpValue );
			if ( perCooltime != NULL )
			{
				elapsedtime = (long)difftime( perCooltime->validThru, ltime );
				if ( elapsedtime > (itemAbility->cool_time1/1000) )
				{
					// SKIP
				}
				else
				{
					UpdateCooltime( perCooltime
								   ,(itemCooltime1->cooltime > itemAbility->cool_time1) ? itemAbility->cool_time1 : itemCooltime1->cooltime
								   ,itemCooltime2->idx
								   ,&ltime );
				}
			}
			else
			{
				perCooltime = GetCooltime( cooltimeRoot, itemCooltime1->itemDefineIndex );
				if ( perCooltime != NULL )
				{
					UpdateCooltime( perCooltime
								   ,(itemCooltime1->cooltime > itemAbility->cool_time1) ? itemAbility->cool_time1 : itemCooltime1->cooltime
								   ,itemCooltime2->idx
								   ,&ltime );
				}
			}
			itemCooltime1 = itemCooltime1->next;
		}
	}
	else
	{
		// 싱글 쿨타임
		perCooltime = GetCooltime( cooltimeRoot, inventory->itemDefineIndex );
		if ( perCooltime != NULL )
			UpdateCooltime( perCooltime, itemAbility->cool_time1, itemAbility->cool_time2, &ltime );
	}
}

// UpdateCooltime Method
bool cCooltimePool::UpdateCooltime(PerCooltime* perCooltime, long cooltime1, long cooltime2, time_t* ltime)
{
	struct tm validThru;
	time_t    result;

	validThru = *localtime( ltime );
	validThru.tm_sec += (cooltime1/1000);

	result = mktime( &validThru );
	if ( result != (time_t)-1 )
	{
		perCooltime->cooltime1 = cooltime1;
		perCooltime->cooltime2 = cooltime2;
		perCooltime->validThru = result;
		return true;
	}
	else
		return false;
}

// GetCooltime Method - 선형리스트의 HEAD를 넘겨준다.
// ReleaseCooltime와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerCooltime* cCooltimePool::GetCooltime(CooltimeRoot* cooltimeRoot, long index)
{
	PerCooltime* perCooltime = (PerCooltime*)MEMORY_POOL->GetPool( (PerNode**)&mNonPagedPoolUsage, sizeof(PerCooltime) );
	if ( perCooltime )
	{
		perCooltime->index     = index; // Cooltime Key(Index)
		perCooltime->cooltime1 = 0;     // Cooltime Value
		perCooltime->cooltime2 = 0;     // Cooltime Group

		// BST-연결시도.
		if ( MEMORY_POOL->AttachBst( &cooltimeRoot->root, perCooltime, CooltimeCmpIndex ) )
		{
			MEMORY_POOL->AttachPool( (PerNode**)&cooltimeRoot->pool, perCooltime );
			return perCooltime;	// 연결성공.
		}
		else
		{
			MEMORY_POOL->AttachPool( (PerNode**)&mNonPagedPoolUsage, perCooltime );
			return NULL;		// 연결실패.
		}
	}
	return perCooltime;
}

// ReleaseCooltime Method - 선형리스트의 HEAD로 회수한다.
// GetCooltime와 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cCooltimePool::ReleaseCooltime(CooltimeRoot* cooltimeRoot, PerCooltime* perCooltime)
{
	// BST - 연결종료.
	MEMORY_POOL->DetachBst( (PerNode**)&cooltimeRoot->root, (PerNode*)perCooltime );

	// 페이지 된 풀 사용량을 차감.
	MEMORY_POOL->DetachPool( (PerNode**)&cooltimeRoot->pool, (PerNode*)perCooltime );

	// 페이지 안된 풀 사용량을 증감.
	MEMORY_POOL->ReleasePool( (PerNode**)&mNonPagedPoolUsage, (PerNode*)perCooltime );
}
