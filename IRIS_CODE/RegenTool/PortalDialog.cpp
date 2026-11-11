#include "stdafx.h"
#include "PortalDialog.h"

#include "RegenToolView.h"

#include "PortalSceneNode.h"

IMPLEMENT_DYNAMIC(cPortalDialog, CDialog)

BEGIN_MESSAGE_MAP(cPortalDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PORTAL_SAVE, OnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_PORTAL_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_PORTAL_DELETE, OnClickedDelete)
	ON_LBN_SELCHANGE(IDC_LIST_PORTAL, OnSelchangeNode)
	ON_CBN_SELCHANGE(IDC_COMBO_PORTAL_TARGET_INDEX, OnSelchangeTargetIndex)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
	ON_BN_CLICKED(IDC_CHECK_PORTAL_CREATED, OnCheckedButton)
END_MESSAGE_MAP()

cPortalDialog::cPortalDialog()
: mSelInfo( 0 )
{
}

cPortalDialog::~cPortalDialog()
{
}

void cPortalDialog::SelectNode( cPortalSceneNode* node )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PORTAL);
	list->SetCurSel( -1 );

	if( node == 0 )
	{
		SetEnabled( false );
		mSelInfo = 0;
		return;
	}
	else
	{
		SetEnabled( true );
	}

	/// 리스트
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		//if( (cPortalSceneNode*)list->GetItemData( i ) == node )
		cPortalSceneNode* n = (cPortalSceneNode*)list->GetItemData( i );
		if( n == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 
	unsigned int index = node->GetUserData0();
	cPortalInfo* info = VIEW->GetPortalInfo( index );
	assert( info );

	mSelInfo = info;

	///
	cString str;
	str.Format( "%d", index );
	GetDlgItem(IDC_EDIT_PORTAL_INDEX)->SetWindowText( str.Cstr() );

	/// 맵 인덱스
	str.Format( "%d", info->mMapIndex );
	GetDlgItem(IDC_EDIT_PORTAL_MAP_INDEX)->SetWindowText( str.Cstr() );

	/// 범위
	mRangeNum.SetValue( (float)info->mRange );

	/// 대상 포탈 인덱스
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_PORTAL_TARGET_INDEX);

	for( unsigned int i = 0, iend = combo->GetCount(); i < iend; ++i )
	{
		if( (unsigned int)combo->GetItemData( i ) == info->mTargetIndex )
		{
			combo->SetCurSel( i );
			break;
		}
	}

	/// 회전
	mRzNum.SetEnabled( true );

	/// 이름 높이 값
	mNameHeightNum.SetEnabled( true );
	mNameHeightNum.SetValue( info->mNameHeight );

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PORTAL_CREATED );
	check->SetCheck( (info->mIsCreateView)? 1:0 );

	/// 회전, 이동
	UpdateNodeTransform( node );
}

void cPortalDialog::UpdateList( const tMap<unsigned int, cPortalInfo*>& infoMap )
{
	{
		CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PORTAL);
		list->ResetContent();
		cString str;

		typedef tMap<unsigned int, cPortalInfo*> cInfoMap;
		cInfoMap::cConstIterator i = infoMap.Begin();
		cInfoMap::cConstIterator iend = infoMap.End();

		for( unsigned int c = 0; i != iend; ++i, ++c )
		{
			unsigned int portalIndex = i->mFirst;
			cPortalInfo* info = i->mSecond;

			str.Format( "%d - map%d", portalIndex, info->mMapIndex );

			list->AddString( str.Cstr() );
			list->SetItemData( c, (DWORD)info->mNode );
		}
	}
	{
		CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_PORTAL_TARGET_INDEX);
		combo->ResetContent();
		combo->AddString( "0" );
		combo->SetItemData( 0, 0 );
		cString str;

		typedef tMap<unsigned int, cPortalInfo*> cInfoMap;
		cInfoMap::cConstIterator i = infoMap.Begin();
		cInfoMap::cConstIterator iend = infoMap.End();

		for( unsigned int c = 1; i != iend; ++i, ++c )
		{
			unsigned int portalIndex = i->mFirst;
			cPortalInfo* info = i->mSecond;

			str.Format( "%d - map%d", portalIndex, info->mMapIndex );

			combo->AddString( str.Cstr() );
			combo->SetItemData( c, (DWORD)portalIndex );
		}

		combo->SetCurSel( 0 );
	}
}

void cPortalDialog::UpdateNodeTransform( cPortalSceneNode* node )
{
	assert( node );

	/// 이동
	const NiPoint3& t = node->GetTranslate();
	mTxNum.SetValue( t.x / 100.0f );
	mTyNum.SetValue( t.y / 100.0f );

	/// 회전
	const NiMatrix3& r = node->GetRotate();
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );
	mRzNum.SetValue( D3DXToDegree( zangle ) );
}

void cPortalDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_PORTAL_SAVE, mSaveButton );
	DDX_Control( dx, IDC_BUTTON_PORTAL_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_PORTAL_DELETE, mDeleteButton );
}

BOOL cPortalDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//
	mRangeNum.Create( 10.0f, 1000.0f, 100.0f, 10.0f, this, IDC_NUM_PORTAL_RANGE );
	mIndexNum.Create( 1.0f, 100000.0f, 1.0f, 1.0f, this, IDC_NUM_PORTAL_INDEX );
	mTxNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_PORTAL_TX );
	mTyNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_PORTAL_TY );
	mRzNum.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_NUM_PORTAL_RZ );
	mNameHeightNum.Create( 0, 100000.0f, 0.0f, 1.0f, this, IDC_NUM_PORTAL_NAMEHEIGHT );

	SetEnabled( false );
	return TRUE;
}

void cPortalDialog::OnClickedSave()
{
	VIEW->SavePortalRegen();
}

void cPortalDialog::OnClickedCreate()
{
	unsigned int index = (unsigned int)mIndexNum.GetValue();

	/// potal file dialog
	CFileDialog dlg(
		TRUE, "Load Potal", "*.nif",
		OFN_HIDEREADONLY,
		"Potal Files (*.nif)| *.nif| All Files (*.*)| *.*|" );

	cString potalName;
	if( dlg.DoModal() == IDOK )
	{
		potalName = (LPCTSTR)dlg.GetFileName();
	}

	///
	int ret = VIEW->CreatePortal( index, potalName.Cstr(), 0.0f, 0.0f, 100, 0, 0, 0, true, true, true );

	switch( ret )
	{
	case -1:
		MessageBox( "Same portal exists.", "Portal" );
		break;
	case -2:
		MessageBox( "Failed to create portal scene node.", "Portal" );
		break;
	}
}

void cPortalDialog::OnClickedDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PORTAL );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPortalSceneNode* node = (cPortalSceneNode*)list->GetItemData( i );
	if( node == 0 )
		return;

	int ret = VIEW->DeletePortal( node );

	switch( ret )
	{
	case -1:
		MessageBox( "Select portal.", "Portal" );
		break;
	}
}

void cPortalDialog::OnSelchangeNode()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PORTAL );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPortalSceneNode* node = (cPortalSceneNode*)list->GetItemData( i );
	if( node == 0 )
	{
		///
		SetEnabled( false );

		///
		CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_PORTAL_TARGET_INDEX);
		unsigned int index = (unsigned int)combo->GetItemData( i + 1 );
		cPortalInfo* info = VIEW->GetPortalInfo( index );
		assert( info );

		mSelInfo = info;

		/// 인덱스
		cString str;
		str.Format( "%d", index );
		GetDlgItem(IDC_EDIT_PORTAL_INDEX)->SetWindowText( str.Cstr() );

		/// 맵 인덱스
		str.Format( "%d", info->mMapIndex );
		GetDlgItem(IDC_EDIT_PORTAL_MAP_INDEX)->SetWindowText( str.Cstr() );

		/// 범위
		mRangeNum.SetValue( (float)info->mRange );

		/// 대상 포탈 인덱스
		for( unsigned int i = 0, iend = combo->GetCount(); i < iend; ++i )
		{
			if( (unsigned int)combo->GetItemData( i ) == info->mTargetIndex )
			{
				combo->SetCurSel( i );
				break;
			}
		}

		/// 이동
		mTxNum.SetValue( info->mX / 100.0f );
		mTyNum.SetValue( info->mY / 100.0f );

		/// 회전
		mRzNum.SetValue( (float)info->mZDegree );
		mRzNum.SetEnabled( false );

		/// 이름높이값
		mNameHeightNum.SetValue( info->mNameHeight );
		mNameHeightNum.SetEnabled( false );
		return;
	}

	VIEW->SelectPortal( node );
}

void cPortalDialog::OnSelchangeTargetIndex()
{
	if( mSelInfo == 0 )
		return;

	CComboBox* combo = (CComboBox*)GetDlgItem( IDC_COMBO_PORTAL_TARGET_INDEX );

	int i = combo->GetCurSel();
	if( i == LB_ERR )
		return;

	unsigned int targetIndex = (unsigned int)combo->GetItemData( i );
	mSelInfo->mTargetIndex = targetIndex;

	cPortalSceneNode* node = mSelInfo->mNode;

	if( node )
		node->SetUserData1( targetIndex );

	//if( node && targetIndex == 0 )
	//{
	//	mRzNum.SetEnabled( true );
	//}
	//else
	//{
	//	mRzNum.SetEnabled( false );
	//}

	///
	VIEW->Update();
}

LRESULT cPortalDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_NUM_PORTAL_RANGE:
		{
			unsigned int range = (unsigned int)mRangeNum.GetValue();

			if( mSelInfo )
				mSelInfo->mRange = range;
			break;
		}
	case IDC_NUM_PORTAL_TX:
	case IDC_NUM_PORTAL_TY:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
			break;
		}
	case IDC_NUM_PORTAL_RZ:
		{
			float zangle = D3DXToRadian( mRzNum.GetValue() );
			VIEW->SetSelRotate( zangle, false );
		}
		break;
	case IDC_NUM_PORTAL_NAMEHEIGHT:
		{
			float nameheight = mNameHeightNum.GetValue();
			
			if( mSelInfo )
				mSelInfo->mNameHeight = nameheight;

			VIEW->SetSelPotalNameHeight( nameheight );
		}
		break;
	}
	return 0;
}

void cPortalDialog::OnCheckedButton()
{
	if( mSelInfo == 0 )
		return;

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PORTAL_CREATED );
	mSelInfo->mIsCreateView = check->GetCheck() != 0;

	VIEW->Update();
}