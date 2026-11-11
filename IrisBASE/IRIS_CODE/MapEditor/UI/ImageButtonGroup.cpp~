#include "stdafx.h"
#include "ImageButtonGroup.h"

#include "ImageButton.h"

cImageButtonGroup::cImageButtonGroup()
: mCheckedButton( 0 )
{
	mButtonArray.Reserve( 15 );
}

cImageButtonGroup::~cImageButtonGroup()
{
	for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
	{
		mButtonArray[i]->mGroup = 0;
		mButtonArray[i]->mIndex = 0;
	}
}

void cImageButtonGroup::Add( cImageButton* button )
{
	unsigned int i = mButtonArray.GetSize();
	mButtonArray.PushBack( button );
	button->mIndex = i;
}

bool cImageButtonGroup::Remove( unsigned int i )
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

void cImageButtonGroup::SetCheckedButton( int id )
{
	if( id == 0 )
	{
		SetCheckedButton( (cImageButton*)0 );
	}
	else
	{
		for( unsigned int i = 0, iend = mButtonArray.GetSize(); i < iend; ++i )
		{
			cImageButton* b = mButtonArray[i];

			if( b->GetDlgCtrlID() == id )
			{
				SetCheckedButton( b );
				break;
			}
		}
	}
}

void cImageButtonGroup::SetCheckedButton( cImageButton* button )
{
	if( button == mCheckedButton )
		return;

	if( mCheckedButton )
	{
		cImageButton* b = mCheckedButton;
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

int cImageButtonGroup::GetCheckedButtonIndex() const
{
	if( mCheckedButton )
		return mCheckedButton->mIndex;
	else
		return -1;
}
