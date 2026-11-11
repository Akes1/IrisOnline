#include "stdafx.h"
#include "ObjectEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "ObjectPropertyDialog.h"

#include "Engine/StaticSceneNode.h"

void cObjectGroup::SetAmbient( COLORREF rgb )
{
	NiColor color;
	COLORREF r = rgb;
	COLORREF g = rgb;
	COLORREF b = rgb;
	color.r = GetRValue( r ) / 255.0f;
	color.g = GetGValue( g ) / 255.0f;
	color.b = GetBValue( b ) / 255.0f;

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetAmbient( color );
	}
}

void cObjectGroup::SetDiffuse( COLORREF rgb )
{
	NiColor color;
	//color.r = GetRValue( rgb ) / 255.0f;
	//color.g = GetGValue( rgb ) / 255.0f;
	//color.b = GetBValue( rgb ) / 255.0f;
	COLORREF r = rgb;
	COLORREF g = rgb;
	COLORREF b = rgb;
	color.r = GetRValue( r ) / 255.0f;
	color.g = GetGValue( g ) / 255.0f;
	color.b = GetBValue( b ) / 255.0f;

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetDiffuse( color );
	}
}

void cObjectGroup::SetEmissive( COLORREF rgb )
{
	NiColor color;
	//color.r = GetRValue( rgb ) / 255.0f;
	//color.g = GetGValue( rgb ) / 255.0f;
	//color.b = GetBValue( rgb ) / 255.0f;
	COLORREF r = rgb;
	COLORREF g = rgb;
	COLORREF b = rgb;
	color.r = GetRValue( r ) / 255.0f;
	color.g = GetGValue( g ) / 255.0f;
	color.b = GetBValue( b ) / 255.0f;

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetEmissive( color );
	}
}

void cObjectGroup::SetVisibleLevel( unsigned int level )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetVisibleLevel( level );
	}
}

void cObjectGroup::SetOccludeLevel( unsigned int level )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetOccludeLevel( level );
	}
}

void cObjectGroup::SetFogApplied( bool fogApplied )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetFogApplied( fogApplied );
	}
}

void cObjectGroup::SetMaterialApplied( bool matApplied )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetMaterialApplied( matApplied );
	}
}

COLORREF cObjectGroup::GetAmbient() const
{
	if( mSelectedSet.GetSize() == 1 )
	{
		cStaticSceneNode* n = *mSelectedSet.Begin();
		unsigned char r = (unsigned char)(n->GetAmbient().r * 255.0f);
		unsigned char g = (unsigned char)(n->GetAmbient().g * 255.0f);
		unsigned char b = (unsigned char)(n->GetAmbient().b * 255.0f);
		return RGB( r, g, b );
	}
	else
	{
		return 0;
	}
}

COLORREF cObjectGroup::GetDiffuse() const
{
	if( mSelectedSet.GetSize() == 1 )
	{
		cStaticSceneNode* n = *mSelectedSet.Begin();
		unsigned char r = (unsigned char)(n->GetDiffuse().r * 255.0f);
		unsigned char g = (unsigned char)(n->GetDiffuse().g * 255.0f);
		unsigned char b = (unsigned char)(n->GetDiffuse().b * 255.0f);
		return RGB( r, g, b );
	}
	else
	{
		return 0;
	}
}

COLORREF cObjectGroup::GetEmissive() const
{
	if( mSelectedSet.GetSize() == 1 )
	{
		cStaticSceneNode* n = *mSelectedSet.Begin();
		unsigned char r = (unsigned char)(n->GetEmissive().r * 255.0f);
		unsigned char g = (unsigned char)(n->GetEmissive().g * 255.0f);
		unsigned char b = (unsigned char)(n->GetEmissive().b * 255.0f);
		return RGB( r, g, b );
	}
	else
	{
		return 0;
	}
}

const char* cObjectGroup::GetFileName() const
{
	switch( mSelectedSet.GetSize() )
	{
	case 0:
		return "!";
	case 1:
		return (*mSelectedSet.Begin())->GetFileName().Cstr();
	}
	return "?";
}

unsigned int cObjectGroup::GetVisibleLevel() const
{
	if( mSelectedSet.GetSize() == 1 )
		return (*mSelectedSet.Begin())->GetVisibleLevel();
	else
		return 2;
}

unsigned int cObjectGroup::GetOccludeLevel() const
{
	if( mSelectedSet.GetSize() == 1 )
		return (*mSelectedSet.Begin())->GetOccludeLevel();
	else
		return false;
}

bool cObjectGroup::GetFogApplied() const
{
	if( mSelectedSet.GetSize() == 1 )
		return (*mSelectedSet.Begin())->GetFogApplied();
	else
		return false;
}

bool cObjectGroup::GetMaterialApplied() const
{
	if( mSelectedSet.GetSize() == 1 )
		return (*mSelectedSet.Begin())->GetMaterialApplied();
	else
		return false;
}

void cObjectEditor::SetGroupAmbient( COLORREF rgb )
{
	//if( mObjectGroup->IsEmpty() )
	//	return;

	mObjectGroup->SetAmbient( rgb );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupDiffuse( COLORREF rgb )
{
	//if( mObjectGroup->IsEmpty() )
	//	return;

	mObjectGroup->SetDiffuse( rgb );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupEmissive( COLORREF rgb )
{
	//if( mObjectGroup->IsEmpty() )
	//	return;

	mObjectGroup->SetEmissive( rgb );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupVisibleLevel( unsigned int level )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetVisibleLevel( level );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupOccludeLevel( unsigned int level )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetOccludeLevel( level );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupFogApplied( bool fogApplied )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetFogApplied( fogApplied );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupMaterialApplied( bool matApplied )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetMaterialApplied( matApplied );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}
