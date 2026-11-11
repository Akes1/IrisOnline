#include "stdafx.h"
#include "GatheringDialog.h"

#include "RegenToolView.h"
#include "GatheringSceneNode.h"


IMPLEMENT_DYNAMIC(cGatheringDialog, CDialog)

BEGIN_MESSAGE_MAP(cGatheringDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GATHERING_SAVE, OnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_GATHERINGGROUP_CREATE, OnClickedGroupCreate)
	ON_BN_CLICKED(IDC_BUTTON_GATHERINGGROUP_DELETE, OnClickedGroupDelete)
	ON_LBN_SELCHANGE(IDC_LIST_GATHERINGGROUP, OnSelchangeGroup)
	ON_LBN_SELCHANGE(IDC_COMBO_GATHERINGGROUP_REGEN_INDEX, OnSelchangeGroupRegenIndex)
	ON_BN_CLICKED(IDC_BUTTON_GATHERING_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_GATHERING_CHANGE, OnClickedChange)
	ON_BN_CLICKED(IDC_BUTTON_GATHERING_DELETE, OnClickedDelete)
	ON_LBN_SELCHANGE(IDC_LIST_GATHERING, OnSelchangeNode)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
END_MESSAGE_MAP()


cGatheringDialog::cGatheringDialog()
: mSelGatheringGroup( 0 )
, mSelGathering( 0 )
{
}

cGatheringDialog::~cGatheringDialog()
{
}

void cGatheringDialog::Clear()
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERINGGROUP);
	list->ResetContent();

	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERINGGROUP_REGEN_INDEX);
	combo->ResetContent();

	list = (CListBox*)GetDlgItem(IDC_LIST_GATHERING);
	list->ResetContent();
}

void cGatheringDialog::SelectGroup( cGatheringGroup* group )
{
	mSelGatheringGroup = group;
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERINGGROUP);
	list->SetCurSel( -1 );

	if( group == 0 )
	{
		SetGroupEnabled( false );
		SelectNode( 0 );
		return;
	}
	else
	{
		SetGroupEnabled( true );
	}

	/// 리스트
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		if( (cGatheringGroup*)list->GetItemData( i ) == group )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 그룹 데이타를 갱신
	{
		/// 리젠 인덱스
		CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERINGGROUP_REGEN_INDEX);
		unsigned int i = 0, iend = combo->GetCount();

		for( ; i < iend; ++i )
		{
			if( (unsigned int)combo->GetItemData(i) == group->mRegenGroupIndex )
			{
				combo->SetCurSel( i );
				break;
			}
		}
		if( i == iend )
		{
			combo->SetCurSel( -1 );
			group->mRegenGroupIndex = 0;
		}

		/// 리젠 대기 시간
		mGroupRegenWaitTimeNum.SetValue( (float)group->mRegenWaitTime );

		/// 리젠 추가 대기 시간
		mGroupRegenRandTimeNum.SetValue( (float)group->mRegenRandTime );
	}

	/// 채집 객체 리스트를 갱신
	UpdateList( group->mNodeList );
}

void cGatheringDialog::SelectNode( cGatheringSceneNode* node )
{
	mSelGathering = node;
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERING);
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

	/// 리스트
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		if( (cGatheringSceneNode*)list->GetItemData( i ) == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 인덱스
	unsigned int index = node->GetUserData0();
	const cGatheringInfo* info = VIEW->GetGatheringInfo( index );
	assert( info );

	cString str;
	str.Format( "%d", index );
	GetDlgItem(IDC_EDIT_GATHERING_INDEX)->SetWindowText( str.Cstr() );

	/// 이름
	GetDlgItem(IDC_EDIT_GATHERING_NAME)->SetWindowText( VIEW->GetGatheringNameByNameIndex(info->mNameIndex).Cstr() );

	/// 모델
	GetDlgItem(IDC_EDIT_GATHERING_MODEL)->SetWindowText( info->mFileName.Cstr() );

	/// 이동, 회전
	UpdateNodeTransform( node );
}

void cGatheringDialog::UpdateCombo( const tMap<unsigned int, cGatheringInfo*>& infoMap )
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERING_INDEX);
	combo->ResetContent();
	cString str;

	typedef tMap<unsigned int, cGatheringInfo*> cInfoMap;
	cInfoMap::cConstIterator ii = infoMap.Begin();
	cInfoMap::cConstIterator iiend = infoMap.End();

	for( unsigned int c = 0; ii != iiend; ++ii, ++c )
	{
		unsigned int gatheringIndex = ii->mFirst;
		cGatheringInfo* info = ii->mSecond;

		str.Format( "%d %s", gatheringIndex, VIEW->GetGatheringNameByNameIndex(info->mNameIndex).Cstr() );

		combo->AddString( str.Cstr() );
		combo->SetItemData( c, (DWORD)gatheringIndex );
	}

	combo->SetCurSel( 0 );
}

void cGatheringDialog::UpdateGroupList( const tMap<unsigned int, cGatheringGroup*>& groupMap )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERINGGROUP);
	list->ResetContent();
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERINGGROUP_REGEN_INDEX);
	combo->ResetContent();
	cString str;

	typedef tMap<unsigned int, cGatheringGroup*> cGroupMap;
	cGroupMap::cConstIterator ii = groupMap.Begin();
	cGroupMap::cConstIterator iiend = groupMap.End();

	for( unsigned int c = 0; ii != iiend; ++ii, ++c )
	{
		unsigned int groupIndex = ii->mFirst;
		cGatheringGroup* group = ii->mSecond;

		str.Format( "%d {...}", groupIndex );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)group );

		combo->AddString( str.Cstr() );
		combo->SetItemData( c, (DWORD)groupIndex );
	}
}

void cGatheringDialog::UpdateList( const tList<cGatheringSceneNode*>& nodeList )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERING);
	list->ResetContent();
	cString str;

	typedef tList<cGatheringSceneNode*> cGatheringList;
	cGatheringList::cConstIterator i = nodeList.Begin();
	cGatheringList::cConstIterator iend = nodeList.End();

	for( unsigned int c = 0; i != iend; ++i, ++c )
	{
		cGatheringSceneNode* n = *i;
		unsigned int gatheringIndex = n->GetUserData0();

		str.Format( "%d %s", gatheringIndex, VIEW->GetGatheringNameByGatherIndex(gatheringIndex).Cstr() );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)n );
	}
}

void cGatheringDialog::UpdateNodeTransform( cGatheringSceneNode* node )
{
	assert( node );

	/// 이동
	const NiPoint3& t = node->GetTranslate();
	mTxNum.SetValue( t.x / 100.0f );
	mTyNum.SetValue( t.y / 100.0f );
	mAzNum.SetValue( node->GetAppendZ() / 100.0f );

	/// 회전
	const NiMatrix3& r = node->GetRotate();
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );
	mRzNum.SetValue( D3DXToDegree( zangle ) );
}

void cGatheringDialog::DoDataExchange(CDataExchange* dx)
{
	CDialog::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_GATHERING_SAVE, mSaveButton );

	DDX_Control( dx, IDC_BUTTON_GATHERINGGROUP_CREATE, mGroupCreateButton );
	DDX_Control( dx, IDC_BUTTON_GATHERINGGROUP_DELETE, mGroupDeleteButton );

	DDX_Control( dx, IDC_BUTTON_GATHERING_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_GATHERING_CHANGE, mChangeButton );
	DDX_Control( dx, IDC_BUTTON_GATHERING_DELETE, mDeleteButton );
}

BOOL cGatheringDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mGroupIndexNum.Create( 1.0f, 1000.0f, 1.0f, 1.0f, this, IDC_NUM_GATHERINGGROUP_INDEX );
	mGroupRegenWaitTimeNum.Create( 100.0f, 1000000.0f, 10000.0f, 1000.0f, this, IDC_NUM_GATHERINGGROUP_REGEN_WAIT_TIME );
	mGroupRegenRandTimeNum.Create( 100.0f, 1000000.0f, 10000.0f, 1000.0f, this, IDC_NUM_GATHERINGGROUP_REGEN_RAND_TIME );
	mTxNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_GATHERING_TX );
	mTyNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_GATHERING_TY );
	mAzNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_GATHERING_AZ );
	mRzNum.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_NUM_GATHERING_RZ );

	SetGroupEnabled( false );
	SetEnabled( false );
	return TRUE;
}

void cGatheringDialog::OnClickedSave()
{
	VIEW->SaveGatheringRegen();
}

void cGatheringDialog::OnClickedGroupCreate()
{
	unsigned int index = (unsigned int)mGroupIndexNum.GetValue();
	int ret = VIEW->CreateGatheringGroup( index );

	switch( ret )
	{
	case 0:
		MessageBox( "Failed to create gathering group.", "Gathering Group" );
		break;
	case -1:
		MessageBox( "Same group exists.", "Gathering Group" );
		break;
	}
}

void cGatheringDialog::OnClickedGroupDelete()
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_GATHERING);
	list->ResetContent();

	///
	list = (CListBox*)GetDlgItem(IDC_LIST_GATHERINGGROUP);
	int i = list->GetCurSel();

	if( i == LB_ERR )
		return;

	cGatheringGroup* group = (cGatheringGroup*)list->GetItemData( i );
	assert( group );

	VIEW->DeleteGatheringGroup( group->mIndex );
}

void cGatheringDialog::OnSelchangeGroup()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_GATHERINGGROUP );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cGatheringGroup* group = (cGatheringGroup*)list->GetItemData( i );
	VIEW->SelectGatheringGroup( group );
}

void cGatheringDialog::OnSelchangeGroupRegenIndex()
{
	if( mSelGatheringGroup == 0 )
		return;

	CComboBox* combo = (CComboBox*)GetDlgItem( IDC_COMBO_GATHERINGGROUP_REGEN_INDEX );

	int i = combo->GetCurSel();
	if( i == LB_ERR )
		return;

	unsigned int groupIndex = (unsigned int)combo->GetItemData( i );
	mSelGatheringGroup->mRegenGroupIndex = groupIndex;
}

void cGatheringDialog::OnClickedCreate()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERING_INDEX);
	unsigned int gatheringIndex = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	int ret = VIEW->CreateGathering( gatheringIndex, 0, 0, 0, 0.0f, true, true );

	switch( ret )
	{
	case -1:
		MessageBox( "Failed to create gathering scene node.", "Gathering" );
		break;
	}
}

void cGatheringDialog::OnClickedChange()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_GATHERING_INDEX);
	unsigned int gatheringIndex = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	int ret = VIEW->ChangeGathering( gatheringIndex );

	switch( ret )
	{
	case -1:
		MessageBox( "Select gathering.", "Gathering" );
		break;
	case -2:
		MessageBox( "Same gathering exists.", "Gathering" );
		break;
	}
}

void cGatheringDialog::OnClickedDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_GATHERING );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cGatheringSceneNode* node = (cGatheringSceneNode*)list->GetItemData( i );
	int ret = VIEW->DeleteGathering( node );

	switch( ret )
	{
	case -1:
		MessageBox( "Delete gathering.", "Gathering" );
		break;
	}
}

void cGatheringDialog::OnSelchangeNode()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_GATHERING );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cGatheringSceneNode* node = (cGatheringSceneNode*)list->GetItemData( i );
	VIEW->SelectGathering( node );
}

LRESULT cGatheringDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	if( mSelGatheringGroup == 0 )
		return 0;

	switch( id )
	{
	case IDC_NUM_GATHERINGGROUP_REGEN_WAIT_TIME:
		{
			mSelGatheringGroup->mRegenWaitTime = (unsigned int)mGroupRegenWaitTimeNum.GetValue();
			break;
		}
	case IDC_NUM_GATHERINGGROUP_REGEN_RAND_TIME:
		{
			mSelGatheringGroup->mRegenRandTime = (unsigned int)mGroupRegenRandTimeNum.GetValue();
			break;
		}
	case IDC_NUM_GATHERING_TX:
	case IDC_NUM_GATHERING_TY:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
			break;
		}
	case IDC_NUM_GATHERING_AZ:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			float appendZ = mAzNum.GetValue() * 100;

			VIEW->SetSelAppendZ( appendZ, true );
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
			break;
		}
	case IDC_NUM_GATHERING_RZ:
		{
			float zangle = D3DXToRadian( mRzNum.GetValue() );
			VIEW->SetSelRotate( zangle, false );
			break;
		}
	}
	return 0;
}
