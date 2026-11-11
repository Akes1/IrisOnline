#include "stdafx.h"
#include "TerrainDetailing.h"

#include "../Engine/Terrain.h"
#include "../Engine/TerrainNode.h"

cTerrainNodeDetailing::cTerrainNodeDetailing( const cTerrainNodeDetailingInfo& info )
{
	mNode = info.mNode;
	assert( mNode );
	mXBegin = info.mXBegin;
	mYBegin = info.mYBegin;
	mXEnd = info.mXEnd;
	mYEnd = info.mYEnd;
	mUndoAlphaArray = info.mUndoAlphaArray;
	mRedoAlphaArray = info.mRedoAlphaArray;
}

cTerrainNodeDetailing::~cTerrainNodeDetailing()
{
	NiDelete [] mUndoAlphaArray;
	NiDelete [] mRedoAlphaArray;
}

void cTerrainNodeDetailing::Undo()
{
	if( mNode )
		mNode->Undo( *this );
}

void cTerrainNodeDetailing::Redo()
{
	if( mNode )
		mNode->Redo( *this );
}

cTerrainDetailing::cTerrainDetailing( const cTerrainDetailingInfo& info )
: cDoing( eDOING_TERRAIN_DETAIL )
{
	mNodeArray.Reserve( info.mNodeArray.GetSize() );

	for( unsigned int i = 0, iend = info.mNodeArray.GetSize(); i < iend; ++i )
	{
		const cTerrainNodeDetailingInfo& nodeInfo = info.mNodeArray[i];

		if( nodeInfo.mNode )
		{
			cTerrainNodeDetailing* n = new cTerrainNodeDetailing( nodeInfo );
			mNodeArray.PushBack( n );
		}
	}
}

cTerrainDetailing::~cTerrainDetailing()
{
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		delete mNodeArray[i];
	}
}

void cTerrainDetailing::Undo()
{
	if( TERRAIN == 0 )
	{
		assert( 0 && "null terrain" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Undo();
	}
}

void cTerrainDetailing::Redo()
{
	if( TERRAIN == 0 )
	{
		assert( 0 && "null terrain" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Redo();
	}
}
