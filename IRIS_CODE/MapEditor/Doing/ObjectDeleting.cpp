#include "stdafx.h"
#include "ObjectDeleting.h"

#include "../ObjectEditor.h"
#include "../Engine/SceneNode.h"

void cObjectDeletingInfo::Clear()
{
	delete [] mObjectInfos;
}

cObjectDeleting::cObjectDeleting( const cObjectDeletingInfo& undo, const cObjectDeletingInfo& redo )
: cDoing( eDOING_OBJECT_DELETE )
{
	mUndo = undo;
	mRedo = redo;
}

cObjectDeleting::~cObjectDeleting()
{
	mUndo.Clear();
	mRedo.Clear();
}

void cObjectDeleting::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectDeleting::Redo()
{
	OBJECTEDIT->Redo( *this );
}
