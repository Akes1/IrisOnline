#include "stdafx.h"
#include "NaviMeshBuilding.h"

#include "../Engine/NaviMesh.h"
#include "../Engine/NaviMeshNode.h"

cNaviMeshNodeBuilding::cNaviMeshNodeBuilding( const cNaviMeshNodeBuildingInfo& info )
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

cNaviMeshNodeBuilding::~cNaviMeshNodeBuilding()
{
	delete [] mUndoHeightArray;
	delete [] mRedoHeightArray;
}

void cNaviMeshNodeBuilding::Undo()
{
	if( mNode )
		mNode->Undo( *this );
}

void cNaviMeshNodeBuilding::Redo()
{
	if( mNode )
		mNode->Redo( *this );
}

cNaviMeshBuilding::cNaviMeshBuilding( const cNaviMeshBuildingInfo& info )
: cDoing( eDOING_NAVIMESH_BUILD )
{
	mNodeArray.Reserve( info.mNodeArray.GetSize() );

	for( unsigned int i = 0, iend = info.mNodeArray.GetSize(); i < iend; ++i )
	{
		const cNaviMeshNodeBuildingInfo& nodeInfo = info.mNodeArray[i];

		if( nodeInfo.mNode )
		{
			cNaviMeshNodeBuilding* n = new cNaviMeshNodeBuilding( nodeInfo );
			mNodeArray.PushBack( n );
		}
	}
}

cNaviMeshBuilding::~cNaviMeshBuilding()
{
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		delete mNodeArray[i];
	}
}

void cNaviMeshBuilding::Undo()
{
	if( NAVIMESH == 0 )
	{
		assert( 0 && "null navimesh" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Undo();
	}
}

void cNaviMeshBuilding::Redo()
{
	if( NAVIMESH == 0 )
	{
		assert( 0 && "null navimesh" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Redo();
	}
}
