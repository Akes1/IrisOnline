#include "stdafx.h"
#include "TabButtonGroup.h"

#include "TabButton.h"

cTabButtonGroup::cTabButtonGroup()
: mCheckedButton( 0 )
{
	mButtonArray.Reserve( 12 );
}

cTabButtonGroup::~cTabButtonGroup()
{
	for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
	{
		mButtonArray[i]->mGroup = 0;
		mButtonArray[i]->mIndex = 0;
	}
}

void cTabButtonGroup::Add( cTabButton* button )
{
	unsigned int i = mButtonArray.GetSize();
	mButtonArray.PushBack( button );
	button->mIndex = i;
}

bool cTabButtonGroup::Remove( unsigned int i )
{
	bool orderChanged = false;
	if( mButtonArray.PopAt( &orderChanged, i ) == false )
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 버튼의 인덱스를 재설정
		mButtonArray[i]->mIndex = i;
	}
	return true;
}

void cTabButtonGroup::SetCheckedButton( int id )
{
	if( id == 0 )
	{
		SetCheckedButton( (cTabButton*)0 );
	}
	else
	{
		for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
		{
			cTabButton* b = mButtonArray[i];

			if( b->GetDlgCtrlID() == id )
			{
				SetCheckedButton( b );
				break;
			}
		}
	}
}

void cTabButtonGroup::SetCheckedButton( cTabButton* button )
{
	if( button == mCheckedButton )
		return;

	if( mCheckedButton )
	{
		cTabButton* b = mCheckedButton;
		mCheckedButton = 0;
		b->SetChecked( false );
	}
	if( button )
	{
		mCheckedButton = button;
		mCheckedButton->SetChecked( true );
	}
	else
	{
		mCheckedButton = 0;
	}
}
