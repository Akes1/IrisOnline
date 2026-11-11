#include "stdafx.h"
#include "ObjectSelecting.h"

#include "../ObjectEditor.h"

cObjectSelecting::cObjectSelecting( const cObjectSelectingInfo& undo, const cObjectSelectingInfo& redo )
: cDoing( eDOING_OBJECT_SELECT )
{
	mUndo = undo;
	mRedo = redo;
}

cObjectSelecting::~cObjectSelecting()
{
	mUndo.Clear();
	mRedo.Clear();
}

void cObjectSelecting::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectSelecting::Redo()
{
	OBJECTEDIT->Redo( *this );
}
