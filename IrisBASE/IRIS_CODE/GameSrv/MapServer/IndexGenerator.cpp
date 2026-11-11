#include "Stdafx.h"
#include "Indexgenerator.h"



cIndexGenerator::cIndexGenerator(void)
: mEmptyIdxQueue( 25000 )
, mIdxMax( 0 )
, mUseCount( ULONG_MAX )
{
}



cIndexGenerator::~cIndexGenerator(void)
{
	mEmptyIdxQueue.Clear();
}



unsigned long cIndexGenerator::GeneratIdx()
{
	if( mUseCount != ULONG_MAX )
	{
		if( mEmptyIdxQueue.GetSize() > mUseCount )
		{
			unsigned long frontData = mEmptyIdxQueue.Front();
			mEmptyIdxQueue.PopFront();
			return frontData;
		}
	}

	return ++mIdxMax;
}



void cIndexGenerator::DelIdx( unsigned long idx )
{
	if( idx == ULONG_MAX )
	{
		idx = 0;
	}

	if( mUseCount != ULONG_MAX )
		mEmptyIdxQueue.PushBack( idx );
}
