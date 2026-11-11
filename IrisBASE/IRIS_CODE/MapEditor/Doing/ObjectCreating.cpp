#include "stdafx.h"
#include "ObjectCreating.h"

#include "../ObjectEditor.h"

cObjectCreating::cObjectCreating( const cObjectCreatingInfo& undo )
: cDoing( eDOING_OBJECT_CREATE )
{
	mUndo = undo;
}

cObjectCreating::~cObjectCreating()
{
}

void cObjectCreating::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectCreating::Redo()
{
	OBJECTEDIT->Redo( *this );
}
