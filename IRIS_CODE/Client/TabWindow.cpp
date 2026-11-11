#include "StdAfx.h"
#include "TabWindow.h"

#include "UIContainer.h"
#include "UINodeProperty.h"
#include "TabButton.h"

cTabWindow::cTabWindow( eUINodeType type )
: cUIWindow( type )
, mCurrentButton( 0 )
, mCurrentSheet( 0 )
{
}

cTabWindow::~cTabWindow()
{
}

void cTabWindow::Close()
{
	cUIWindow::Close();

	mCurrentSheet = 0;
	mCurrentButton = 0;
}

void cTabWindow::AddSheet( cUIWindow* sheet )
{
	assert( sheet && "null tab sheet" );

	sheet->Hide( false );
	mTabSheetArray.PushBack( sheet );
}

void cTabWindow::AddTab( cTabButton* tab )
{
	assert( tab && "null tab button" );

	mTabButtonArray.PushBack( tab );
}

cUIWindow* cTabWindow::GetSheetByID( int nID )
{
	for( unsigned int i = 0, iend = mTabSheetArray.GetSize(); i < iend; ++i )
	{
		cUIWindow* sheet = (cUIWindow*)mTabSheetArray[i];
		if( sheet )
		{
			if( sheet->GetID() == nID )
			{
				return sheet;
			}
		}
	}
	return NULL;
}

void cTabWindow::SetCurrent( unsigned int index )
{
	if( index >= mTabButtonArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return;
	}
	if( index >= mTabSheetArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return;
	}

	/// 우선 눌림 초기화
	for( unsigned int i = 0, iend = mTabButtonArray.GetSize(); i < iend; ++i )
	{
		cTabButton* button = (cTabButton*)mTabButtonArray[i];
		if( button )
		{
			button->SetPress( false );
		}
		else
		{
			assert(0);
		}
	}

	/// 쉬트 초기화
	for( unsigned int i = 0, iend = mTabSheetArray.GetSize(); i < iend; ++i )
	{
		cUIWindow* sheet = (cUIWindow*)mTabSheetArray[i];
		if( sheet )
		{
			sheet->Hide();
		}
		else
		{
			assert(0);
		}
	}

	/// 선택된 쉬트 보이기
	cUIWindow* sheet = (cUIWindow*)mTabSheetArray[index];
	if( sheet )
	{
		sheet->Show( false );
		mCurrentSheet = sheet;
	}
	else
	{
		assert(0);
	}

	/// 선택된 버튼 눌림
	cTabButton* button = (cTabButton*)mTabButtonArray[index];
	if( button )
	{
		button->SetPress( true );
		mCurrentButton = button;
	}
	else
	{
		assert(0);
	}

	if( mCurrentButton )
	{
		UIMAN->GotoFrontNode( mCurrentButton );
	}
}

bool cTabWindow::SetSkin( const cUINodeSkin* skin )
{
	if( skin->IsKindof( eUINODE_TABWINDOW ) == false )
	{
		assert( 0 && "not tabwindow skin type" );
		return false;
	}

	if( cUIWindow::SetSkin( skin ) == false )
	{
		return false;
	}
	return true;
}

bool cTabWindow::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return mpParent->HandleEvent( event );
	}

	switch( event.mType )
	{
	case eUIEVENT_TABBUTTON_PRESSED:
		OnTabButtonPressed( event.mpCaller, event.mID );
		break;

	default:
		return cUIWindow::HandleEvent( event );
	}
	return true;
}

bool cTabWindow::OnCreate( cUINodeProperty* prop )
{
	if( cUIWindow::OnCreate( prop ) == false )
		return false;

	/// 생성된 탭버튼을 추가
	cChildList::cConstIterator i = mChildList.Begin();
	cChildList::cConstIterator iend = mChildList.End();

	for( ; i != iend; ++i )
	{
		cUINode* child = (cUINode*)(*i);

		if( child->IsKindof( eUINODE_TABBUTTON ) == true )
		{
			mTabButtonArray.PushBack( child );
		}

		if( child->IsKindof( eUINODE_TABSHEET ) == true )
		{
			mTabSheetArray.PushBack( child );
		}
	}
	return true;
}

void cTabWindow::OnTabButtonPressed( cUINode* caller, unsigned int )
{
	if( caller->IsKindof( eUINODE_TABBUTTON ) == false )
	{
		assert(0);
		return;
	}
	if( mCurrentButton == caller )
	{
		return;
	}

	for( unsigned int i = 0, iend = mTabButtonArray.GetSize(); i < iend; ++i )
	{
		cTabButton* button = (cTabButton*)mTabButtonArray[i];
		//cUIWindow* sheet = (cUIWindow*)mTabSheetArray[i];
		if( button == caller )
		{
			SetCurrent( i );
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
cTabWindowSkin::cTabWindowSkin( eUINodeType type )
: cUIWindowSkin( type )
, mPressed( false )
{

}

cTabWindowSkin::~cTabWindowSkin()
{

}
