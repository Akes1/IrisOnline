#pragma once

#include "memorypool.h"

struct PerTakeDamage : PerNode
{
	unsigned long		playerIdx;
	unsigned long		takeDamage;
	unsigned long		distressPoint;

	/// 변화가 없이 지속되는 경우 체크
	bool				changeFlag;
	unsigned long		noChangeTime;

	PerTakeDamage()
	{
		changeFlag = false;
		noChangeTime = 0;
	}
};

struct TakeDamageRoot : NodeRoot
{
	unsigned long count;

	TakeDamageRoot( )
	{
		pool  = NULL;
		root  = NULL;
		count = 0;
	}
};

class cTakeDamagePool
{
private:
	PerTakeDamage*  mNonPagedPoolUsage;        // 페이지 안된 풀.

	int				CompareIndex( PerTakeDamage* perTakeDamage1, PerTakeDamage* perTakeDamage2 );
	int				CompareIndex( unsigned long index1, unsigned long index2 );

	PerTakeDamage*	GetPool( TakeDamageRoot* takeDamageRoot, unsigned long index );
	void			ReleasePool( TakeDamageRoot* takeDamageRoot, PerTakeDamage* perTakeDamage );

public:
	cTakeDamagePool( );
	virtual ~cTakeDamagePool();

	void            Shutdown( );

	PerTakeDamage*  GetTakeDamage( TakeDamageRoot* takeDamageRoot, unsigned long index );
	void            ReleaseTakeDamage( TakeDamageRoot* takeDamageRoot, unsigned long index );

	PerTakeDamage*  AddTakeDamage( TakeDamageRoot* takeDamageRoot, unsigned long index, unsigned long damage, long distress );

public:
	static cTakeDamagePool* mpTakeDamagePool;
};

#define TAKEDAMAGEPOOL cTakeDamagePool::mpTakeDamagePool
