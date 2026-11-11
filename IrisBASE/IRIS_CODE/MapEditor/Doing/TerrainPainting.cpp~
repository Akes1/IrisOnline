#include "stdafx.h"
#include "TerrainPainting.h"

#include "../Engine/Terrain.h"
#include "../Engine/TerrainNode.h"

cTerrainNodePainting::cTerrainNodePainting( const cTerrainNodePaintingInfo& info )
{
	mNode = info.mNode;
	assert( mNode );
	mXBegin = info.mXBegin;
	mYBegin = info.mYBegin;
	mXEnd = info.mXEnd;
	mYEnd = info.mYEnd;
	mUndoColorArray = info.mUndoColorArray;
	mRedoColorArray = info.mRedoColorArray;
}

cTerrainNodePainting::~cTerrainNodePainting()
{
	NiDelete [] mUndoColorArray;
	NiDelete [] mRedoColorArray;
}

void cTerrainNodePainting::Undo()
{
	if( mNode )
		mNode->Undo( *this );
}

void cTerrainNodePainting::Redo()
{
	if( mNode )
		mNode->Redo( *this );
}

cTerrainPainting::cTerrainPainting( const cTerrainPaintingInfo& info )
: cDoing( eDOING_TERRAIN_PAINT )
{
	mNodeArray.Reserve( info.mNodeArray.GetSize() );

	for( unsigned int i = 0, iend = info.mNodeArray.GetSize(); i < iend; ++i )
	{
		const cTerrainNodePaintingInfo& nodeInfo = info.mNodeArray[i];

		if( nodeInfo.mNode )
		{
			cTerrainNodePainting* n = new cTerrainNodePainting( nodeInfo );
			mNodeArray.PushBack( n );
		}
	}
}

cTerrainPainting::~cTerrainPainting()
{
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		delete mNodeArray[i];
	}
}

void cTerrainPainting::Undo()
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

void cTerrainPainting::Redo()
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
