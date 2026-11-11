#include "stdafx.h"
#include "ObjectTransforming.h"

#include "../ObjectEditor.h"

cObjectTranslating::cObjectTranslating( const NiPoint3& undo, const NiPoint3& redo )
: cDoing( eDOING_OBJECT_TRANSLATE )
{
	mUndo = undo;
	mRedo = redo;
}

void cObjectTranslating::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectTranslating::Redo()
{
	OBJECTEDIT->Redo( *this );
}

cObjectRotating::cObjectRotating( const NiMatrix3& undo, const NiMatrix3& redo )
: cDoing( eDOING_OBJECT_ROTATE )
{
	mUndo = undo;
	mRedo = redo;
}

void cObjectRotating::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectRotating::Redo()
{
	OBJECTEDIT->Redo( *this );
}

cObjectScaling::cObjectScaling( float undo, float redo )
: cDoing( eDOING_OBJECT_SCALE )
{
	mUndo = undo;
	mRedo = redo;
}

void cObjectScaling::Undo()
{
	OBJECTEDIT->Undo( *this );
}

void cObjectScaling::Redo()
{
	OBJECTEDIT->Redo( *this );
}
