#include "stdafx.h"
#include "ItemSplitWindow.h"

#include "ItemManager.h"
#include "ItemDefine.h"

enum
{
	UIID_ITEM_SPLIT_NULL = 0,
	UIID_ITEM_SPLIT_CAPTION = 10000,
	UIID_ITEM_SPLIT_NUMBER,
	UIID_ITEM_SPLIT_UP,
	UIID_ITEM_SPLIT_DOWN,
	UIID_ITEM_SPLIT_MAX,
};

cItemSplitWindow::cItemSplitWindow()
: mMaxValue( 1 )
, mValue( 1 )
{
}

cItemSplitWindow::~cItemSplitWindow()
{
}

void cItemSplitWindow::ShowModal( unsigned int i0, unsigned int i1 )
{
	const cItem& item0 = ITEMMAN->GetItem( i0 );
	const cItem& item1 = ITEMMAN->GetItem( i1 );

	if( item0.GetCount() <= 1 )
		return;

	if( item1.GetCount() && item0.GetIndex() != item1.GetIndex() )
		return;

	mSlotIndex0 = i0;
	mSlotIndex1 = i1;

	mMaxValue = item0.GetCount();
	mValue = 1;
	mNumEdit.SetValue( 1 );

	cUIWindow::ShowModal();
}

bool cItemSplitWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
	{
		return false;
	}

	/// 자식 컨트롤들을 생성
	if( mCaptionLabel.Create( "ItemSplit_Caption", this, UIID_ITEM_SPLIT_CAPTION ) == false )
	{
		assert( 0 && "failed to create item split caption" );
		return false;
	}
	if( mNumEdit.Create( "ItemSplit_Number", this, UIID_ITEM_SPLIT_NUMBER ) == false )
	{
		assert( 0 && "failed to create item split number edit" );
		return false;
	}
	if( mCloseButton.Create( "ItemSplit_Close", this, eUIID_CANCEL ) == false )
	{
		assert( 0 && "failed to create item split caption" );
		return false;
	}
	if( mUpButton.Create( "ItemSplit_Up", this, UIID_ITEM_SPLIT_UP ) == false )
	{
		return false;
	}
	if( mDownButton.Create( "ItemSplit_Down", this, UIID_ITEM_SPLIT_DOWN ) == false )
	{
		return false;
	}
	if( mMaxButton.Create( "ItemSplit_Max", this, UIID_ITEM_SPLIT_MAX ) == false )
	{
		return false;
	}
	if( mOkButton.Create( "ItemSplit_Ok", this, eUIID_OK ) == false )
	{
		return false;
	}
	if( mCancelButton.Create( "ItemSplit_Cancel", this, eUIID_CANCEL ) == false )
	{
		return false;
	}
	return true;
}

void cItemSplitWindow::OnProcess( float accumTime )
{
	cUIWindow::OnProcess( accumTime );
}

void cItemSplitWindow::OnOk()
{
	ITEMMAN->TrySplitItem( mSlotIndex0, mSlotIndex1, (unsigned short)mValue );

	cUIWindow::OnOk();
}

void cItemSplitWindow::OnCancel()
{
	ITEMMAN->SetItemColor( mSlotIndex0, NiColorA::WHITE );

	cUIWindow::OnCancel();
}

void cItemSplitWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	switch( id )
	{
	case UIID_ITEM_SPLIT_UP:
		if( ++mValue > mMaxValue )
			mValue = mMaxValue;

		mNumEdit.SetValue( mValue );
		break;
	case UIID_ITEM_SPLIT_DOWN:
		if( --mValue < 1 )
			mValue = 1;

		mNumEdit.SetValue( mValue );
		break;
	case UIID_ITEM_SPLIT_MAX:
		mValue = mMaxValue;
		mNumEdit.SetValue( mValue );
		break;
	}
}
