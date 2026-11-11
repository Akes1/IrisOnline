#include "stdafx.h"
#include "CheckButtonGroup.h"

#include "CheckButton.h"

cCheckButtonGroup::cCheckButtonGroup()
: mCheckedButton( 0 )
{
	mButtonArray.Reserve( 12 );
}

cCheckButtonGroup::~cCheckButtonGroup()
{
	for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
	{
		mButtonArray[i]->mGroup = 0;
		mButtonArray[i]->mIndex = 0;
	}
}

void cCheckButtonGroup::Add( cCheckButton* button )
{
	unsigned int i = mButtonArray.GetSize();
	mButtonArray.PushBack( button );
	button->mIndex = i;
}

bool cCheckButtonGroup::Remove( unsigned int i )
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

void cCheckButtonGroup::SetCheckedButton( int id )
{
	if( id == 0 )
	{
		SetCheckedButton( (cCheckButton*)0 );
	}
	else
	{
		for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
		{
			cCheckButton* b = mButtonArray[i];

			if( b->GetDlgCtrlID() == id )
			{
				SetCheckedButton( b );
				break;
			}
		}
	}
}

void cCheckButtonGroup::SetCheckedButton( cCheckButton* button )
{
	if( button == mCheckedButton )
		return;

	if( mCheckedButton )
	{
		cCheckButton* b = mCheckedButton;
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
