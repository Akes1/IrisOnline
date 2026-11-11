#include "stdafx.h"
#include "TerrainBuilding.h"

#include "../Engine/Terrain.h"
#include "../Engine/TerrainNode.h"

cTerrainNodeBuilding::cTerrainNodeBuilding( const cTerrainNodeBuildingInfo& info )
{
	mNode = info.mNode;
	assert( mNode );
	mXBegin = info.mXBegin;
	mYBegin = info.mYBegin;
	mXEnd = info.mXEnd;
	mYEnd = info.mYEnd;
	mUndoHeightArray = info.mUndoHeightArray;
	mRedoHeightArray = info.mRedoHeightArray;
}

cTerrainNodeBuilding::~cTerrainNodeBuilding()
{
	delete [] mUndoHeightArray;
	delete [] mRedoHeightArray;
}

void cTerrainNodeBuilding::Undo()
{
	if( mNode )
		mNode->Undo( *this );
}

void cTerrainNodeBuilding::Redo()
{
	if( mNode )
		mNode->Redo( *this );
}

cTerrainBuilding::cTerrainBuilding( const cTerrainBuildingInfo& info )
: cDoing( eDOING_TERRAIN_BUILD )
{
	mNodeArray.Reserve( info.mNodeArray.GetSize() );

	for( unsigned int i = 0, iend = info.mNodeArray.GetSize(); i < iend; ++i )
	{
		const cTerrainNodeBuildingInfo& nodeInfo = info.mNodeArray[i];

		if( nodeInfo.mNode )
		{
			cTerrainNodeBuilding* n = new cTerrainNodeBuilding( nodeInfo );
			mNodeArray.PushBack( n );
		}
	}
}

cTerrainBuilding::~cTerrainBuilding()
{
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		delete mNodeArray[i];
	}
}

void cTerrainBuilding::Undo()
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

void cTerrainBuilding::Redo()
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
