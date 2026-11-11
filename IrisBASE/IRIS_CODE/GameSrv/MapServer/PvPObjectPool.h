#pragma once

#include "memorypool.h"

class cBaseDeathMatch;
class cDeathMatchObject;
class cDeathMatch2Object;

struct sPvPObjectRoot : NodeRoot
{
	unsigned long count;

	sPvPObjectRoot( )
	{
		pool  = NULL;
		root  = NULL;
		count = 0;
	}
};

class cPvPObjectPool
{
private:
	cBaseDeathMatch*	mNonPagedPoolUsage;        // 페이지 안된 풀.

//	int					CompareIndex( cBaseDeathMatch* pPvPObject1, cBaseDeathMatch* pPvPObject2 );
//	int					CompareIndex( unsigned long index1, unsigned long index2 );

//	cBaseDeathMatch*	GetPool( sPvPObjectRoot* pPvPObjectRoot, unsigned long index );
//	void				ReleasePool( sPvPObjectRoot* pPvPObjectRoot, cBaseDeathMatch* pPvPObject );

public:
	cPvPObjectPool( );
	virtual ~cPvPObjectPool();

	void				Shutdown();

	cBaseDeathMatch*	GetPvPObject( sPvPObjectRoot* pPvPObjectRoot, unsigned long index );
	void				ReleasePvPObject( sPvPObjectRoot* pPvPObjectRoot, unsigned long index );

	cDeathMatchObject*	NewDMObject();
	cDeathMatch2Object*	NewDM2Object();
	bool				PvPAttachPool( sPvPObjectRoot* pPvPObjectRoot, cBaseDeathMatch* pPvPObject );

public:
	static cPvPObjectPool* mpPvPObjectPool;
};

#define PVPOBJECTPOOL cPvPObjectPool::mpPvPObjectPool
