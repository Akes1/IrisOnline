#include "stdafx.h"
#include "NaviFieldPainting.h"

#include "../Engine/NaviField.h"
#include "../Engine/NaviFieldNode.h"

cNaviFieldNodePainting::cNaviFieldNodePainting( const cNaviFieldNodePaintingInfo& info )
{
	mNode = info.mNode;
	assert( mNode );
	mXBegin = info.mXBegin;
	mYBegin = info.mYBegin;
	mXEnd = info.mXEnd;
	mYEnd = info.mYEnd;
	mUndoColorArray = info.mUndoColorArray;
	mUndoValueArray = info.mUndoValueArray;
	mRedoColorArray = info.mRedoColorArray;
	mRedoValueArray = info.mRedoValueArray;
}

cNaviFieldNodePainting::~cNaviFieldNodePainting()
{
	NiDelete [] mUndoColorArray;
	NiDelete [] mUndoValueArray;
	NiDelete [] mRedoColorArray;
	NiDelete [] mRedoValueArray;
}

void cNaviFieldNodePainting::Undo()
{
	if( mNode )
		mNode->Undo( *this );
}

void cNaviFieldNodePainting::Redo()
{
	if( mNode )
		mNode->Redo( *this );
}

cNaviFieldPainting::cNaviFieldPainting( const cNaviFieldPaintingInfo& info )
: cDoing( eDOING_NAVIFIELD_PAINT )
{
	mNodeArray.Reserve( info.mNodeArray.GetSize() );

	for( unsigned int i = 0, iend = info.mNodeArray.GetSize(); i < iend; ++i )
	{
		const cNaviFieldNodePaintingInfo& nodeInfo = info.mNodeArray[i];

		if( nodeInfo.mNode )
		{
			cNaviFieldNodePainting* n = new cNaviFieldNodePainting( nodeInfo );
			mNodeArray.PushBack( n );
		}
	}
}

cNaviFieldPainting::~cNaviFieldPainting()
{
	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		delete mNodeArray[i];
	}
}

void cNaviFieldPainting::Undo()
{
	if( NAVIFIELD == 0 )
	{
		assert( 0 && "null navimesh" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Undo();
	}
}

void cNaviFieldPainting::Redo()
{
	if( NAVIFIELD == 0 )
	{
		assert( 0 && "null navimesh" );
		return;
	}

	for( unsigned int i = 0, iend = mNodeArray.GetSize(); i < iend; ++i )
	{
		mNodeArray[i]->Redo();
	}
}
