#include "stdafx.h"
#include "ColorBoxGroup.h"

#include "ColorBox.h"

cColorBoxGroup::cColorBoxGroup()
: mCheckedBox( 0 )
{
	mBoxArray.Reserve( 15 );
}

cColorBoxGroup::~cColorBoxGroup()
{
	for( unsigned int i = 0, iend = mBoxArray.GetSize(); i < iend; ++i )
	{
		mBoxArray[i]->mGroup = 0;
		mBoxArray[i]->mIndex = 0;
	}
}

void cColorBoxGroup::Add( cColorBox* box )
{
	unsigned int i = mBoxArray.GetSize();
	mBoxArray.PushBack( box );
	box->mIndex = i;
}

bool cColorBoxGroup::Remove( unsigned int i )
{
	bool orderChanged = false;
	if( mBoxArray.PopAt( &orderChanged, i ) == false )
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 상자의 인덱스를 재설정
		mBoxArray[i]->mIndex = i;
	}
	return true;
}

void cColorBoxGroup::SetCheckedBox( int id )
{
	if( id == 0 )
	{
		SetCheckedBox( (cColorBox*)0 );
	}
	else
	{
		for( unsigned int i = 0, iend = mBoxArray.GetSize(); i < iend; ++i )
		{
			cColorBox* b = mBoxArray[i];

			if( b->GetDlgCtrlID() == id )
			{
				SetCheckedBox( b );
				break;
			}
		}
	}
}

void cColorBoxGroup::SetCheckedBox( cColorBox* box )
{
	if( box == mCheckedBox )
		return;

	if( mCheckedBox )
	{
		cColorBox* b = mCheckedBox;
		mCheckedBox = 0;
		b->SetChecked( false );
	}
	if( box )
	{
		mCheckedBox = box;
		mCheckedBox->SetChecked( true );
	}
	else
	{
		mCheckedBox = 0;
	}
}

int cColorBoxGroup::GetCheckedBoxIndex() const
{
	if( mCheckedBox )
		return mCheckedBox->mIndex;
	else
		return -1;
}
