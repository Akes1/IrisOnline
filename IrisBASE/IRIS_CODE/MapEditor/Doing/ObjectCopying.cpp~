#include "stdafx.h"
#include "ObjectCopying.h"

#include "../ObjectEditor.h"

cObjectCopying::cObjectCopying( const cObjectCopyingInfo& undo )
: cDoing( eDOING_OBJECT_COPY )
{
	mNumObjects = undo.mNumObjects;
	mNodes = undo.mNodes;
}

cObjectCopying::~cObjectCopying()
{
	delete [] mNodes;
}

void cObjectCopying::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectCopying::Redo()
{
	OBJECTEDIT->Redo( *this );
}
