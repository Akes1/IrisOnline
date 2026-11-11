#include "stdafx.h"
#include "DoorDialog.h"

#include "RegenToolView.h"

#include "PortalSceneNode.h"

IMPLEMENT_DYNAMIC(cDoorDialog, CDialog)

BEGIN_MESSAGE_MAP(cDoorDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DOOR_SAVE, OnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_DOOR_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_DOOR_DELETE, OnClickedDelete)
	ON_LBN_SELCHANGE(IDC_LIST_DOOR, OnSelchangeNode)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
END_MESSAGE_MAP()


cDoorDialog::cDoorDialog()
{

}

cDoorDialog::~cDoorDialog()
{
}

void cDoorDialog::UpdateList( const tMap<unsigned int, cDoorInfo*>& infoMap )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_DOOR);
	list->ResetContent();
	cString str;

	unsigned long curMapIdx = VIEW->GetCurrentMapIndex();

	typedef tMap<unsigned int, cDoorInfo*> cInfoMap;
	cInfoMap::cConstIterator i = infoMap.Begin();
	cInfoMap::cConstIterator iend = infoMap.End();

	for( unsigned int c = 0; i != iend; ++i )
	{
		unsigned int postIndex = i->mFirst;
		cDoorInfo* info = i->mSecond;

		if( info->mMapIndex != curMapIdx )
			continue;

		str.Format( "%d - map%d", postIndex, info->mMapIndex );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)info->mNode );
		c++;
	}
}

void cDoorDialog::SelectNode( cDoorSceneNode* node )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_DOOR);
	list->SetCurSel( -1 );

	if( node == 0 )
	{
		SetEnabled( false );
		return;
	}
	else
	{
		SetEnabled( true );
	}

	/// ¸®½ºÆ®
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		cDoorSceneNode* n = (cDoorSceneNode*)list->GetItemData( i );
		if( n == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 
	unsigned int index = node->GetUserData0();

	cDoorInfo* info = VIEW->GetDoorInfo( index );
	assert( info );

	///
	cString str;
	str.Format( "%d", index );
	GetDlgItem(IDC_EDIT_DOOR_INDEX)->SetWindowText( str.Cstr() );

	/// ¸Ê ÀÎµ¦½º
	str.Format( "%d", info->mMapIndex );
	GetDlgItem(IDC_EDIT_DOOR_MAPINDEX)->SetWindowText( str.Cstr() );

	/// È¸Àü, ÀÌµ¿
	UpdateNodeTransform( node );
}

void cDoorDialog::UpdateNodeTransform( cDoorSceneNode* node )
{
	assert( node );

	/// ÀÌµ¿
	const NiPoint3& t = node->GetTranslate();
	mTxNum.SetValue( t.x / 100.0f );
	mTyNum.SetValue( t.y / 100.0f );
	mAzNum.SetValue( node->GetAppendZ() / 100.0f );

	float tWidth, bWidth, lHeight, rHeight;
	node->GetRectInfo( tWidth, bWidth, lHeight, rHeight );

	mRectT_WidthNum.SetValue( tWidth );
	mRectB_WidthNum.SetValue( bWidth );
	mRectL_HeightNum.SetValue( lHeight );
	mRectR_HeightNum.SetValue( rHeight );

	mRectAppendNum.SetValue( node->GetRectAppendY() / 100.0f );

	/// È¸Àü
	const NiMatrix3& r = node->GetRotate();
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );
	mRzNum.SetValue( D3DXToDegree( zangle ) );
}

void cDoorDialog::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_DOOR_SAVE, mSaveButton );
	DDX_Control( dx, IDC_BUTTON_DOOR_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_DOOR_DELETE, mDeleteButton );
}

BOOL cDoorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mIndexNum.Create( 1.0f, 100000.0f, 1.0f, 1.0f, this, IDC_NUM_DOOR_INDEX );

	mRectT_WidthNum.Create( 1.0f, 100000.0f, 10.0f, 1.0f, this, IDC_NUM_DOOR_RECT_TWIDTH );
	mRectB_WidthNum.Create( 1.0f, 100000.0f, 10.0f, 1.0f, this, IDC_NUM_DOOR_RECT_BWIDTH );
	mRectL_HeightNum.Create( 1.0f, 100000.0f, 10.0f, 1.0f, this, IDC_NUM_DOOR_RECT_LHEIGHT );
	mRectR_HeightNum.Create( 1.0f, 100000.0f, 10.0f, 1.0f, this, IDC_NUM_DOOR_RECT_RHEIGHT );

	mRectAppendNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_DOOR_RECT_APPENDY );

	mTxNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_DOOR_TX );
	mTyNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_DOOR_TY );
	mAzNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_DOOR_AZ );

	mRzNum.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_NUM_DOOR_RZ );

	SetEnabled( false );

	return TRUE;
}

void cDoorDialog::OnClickedSave()
{
	VIEW->SaveDoorRegen();
}

void cDoorDialog::OnClickedCreate()
{
	unsigned int index = (unsigned int)mIndexNum.GetValue();

	/// potal file dialog
	CFileDialog dlg(
		TRUE, "Load Door", "*.nif",
		OFN_HIDEREADONLY,
		"Door Files (*.nif)| *.nif| All Files (*.*)| *.*|" );

	cString doorName;
	if( dlg.DoModal() == IDOK )
	{
		doorName = (LPCTSTR)dlg.GetFileName();
	}

	float width[2];
	float height[2];

	width[0] = width[1] = 10.0f;
	height[0] = height[1] = 10.0f;

	///
	int ret = VIEW->CreateDoor( index, doorName.Cstr(), NiPoint2::ZERO, 0.0f, 0, width, height, 0.0f, true, true );

	switch( ret )
	{
	case -1:
		MessageBox( "Same door exists.", "Door" );
		break;
	case -2:
		MessageBox( "Failed to create door scene node.", "Door" );
		break;
	}
}

void cDoorDialog::OnClickedDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_DOOR );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cDoorSceneNode* node = (cDoorSceneNode*)list->GetItemData( i );
	if( node == 0 )
		return;

	int ret = VIEW->DeleteDoor( node );

	switch( ret )
	{
	case -1:
		MessageBox( "Select door.", "Door" );
		break;
	}
}

void cDoorDialog::OnSelchangeNode()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_DOOR );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cDoorSceneNode* node = (cDoorSceneNode*)list->GetItemData( i );
	if( node == 0 )
	{
		///
		SetEnabled( false );

		unsigned int index = node->GetUserData0();

		///
		cDoorInfo* info = VIEW->GetDoorInfo( index );
		assert( info );

		/// ÀÎµ¦½º
		cString str;
		str.Format( "%d", index );
		GetDlgItem(IDC_EDIT_DOOR_INDEX)->SetWindowText( str.Cstr() );

		/// ¸Ê ÀÎµ¦½º
		str.Format( "%d", info->mMapIndex );
		GetDlgItem(IDC_EDIT_DOOR_MAPINDEX)->SetWindowText( str.Cstr() );

		/// ÀÌµ¿
		NiPoint3 pos = node->GetTranslate();
		mTxNum.SetValue( pos.x / 100.0f );
		mTyNum.SetValue( pos.y / 100.0f );

		mAzNum.SetValue( node->GetAppendZ() / 100.0f );

		/// rect info
		float tWidth, bWidth, lHeight, rHeight;
		node->GetRectInfo( tWidth, bWidth, lHeight, rHeight );

		mRectT_WidthNum.SetValue( tWidth );
		mRectB_WidthNum.SetValue( bWidth );
		mRectL_HeightNum.SetValue( lHeight );
		mRectR_HeightNum.SetValue( rHeight );

		mRectAppendNum.SetValue( node->GetRectAppendY() / 100.0f );

		/// È¸Àü
		const NiMatrix3& r = node->GetWorldRotate();
		float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
		r.ToEulerAnglesXYZ( xangle, yangle, zangle );
		int zdegree = (int)D3DXToDegree( zangle );

		mRzNum.SetValue( (float)zdegree );
		mRzNum.SetEnabled( false );
		return;
	}

	VIEW->SelectDoor( node );
}

LRESULT cDoorDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_NUM_DOOR_TX:
	case IDC_NUM_DOOR_TY:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
			break;
		}
	case IDC_NUM_DOOR_AZ:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			float appendZ = mAzNum.GetValue() * 100;

			VIEW->SetSelAppendZ( appendZ, true );
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
		}
		break;
	case IDC_NUM_DOOR_RZ:
		{
			float zangle = D3DXToRadian( mRzNum.GetValue() );
			VIEW->SetSelRotate( zangle, false );
		}
		break;
	case IDC_NUM_DOOR_RECT_TWIDTH:
	case IDC_NUM_DOOR_RECT_BWIDTH:
	case IDC_NUM_DOOR_RECT_LHEIGHT:
	case IDC_NUM_DOOR_RECT_RHEIGHT:
		{
			float tWidth = mRectT_WidthNum.GetValue();
			float bWidth = mRectB_WidthNum.GetValue();
			float lHeight = mRectL_HeightNum.GetValue();
			float rHeight = mRectR_HeightNum.GetValue();

			VIEW->SetSelDoorInfo( tWidth, bWidth, lHeight, rHeight );
		}
		break;
	case IDC_NUM_DOOR_RECT_APPENDY:
		{
			float appendY = mRectAppendNum.GetValue() * 100;

			VIEW->SetSelDoorAppendY( appendY );
		}
		break;
	}
	return 0;
}