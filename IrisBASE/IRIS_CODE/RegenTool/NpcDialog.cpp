#include "stdafx.h"
#include "NpcDialog.h"

#include "RegenToolView.h"

#include "NpcSceneNode.h"

IMPLEMENT_DYNAMIC(cNpcDialog, CDialog)

BEGIN_MESSAGE_MAP(cNpcDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_NPC_SAVE, OnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_NPC_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_NPC_CHANGE, OnClickedChange)
	ON_BN_CLICKED(IDC_BUTTON_NPC_DELETE, OnClickedDelete)
	ON_LBN_SELCHANGE(IDC_LIST_NPC, OnSelchangeNode)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
END_MESSAGE_MAP()

cNpcDialog::cNpcDialog()
{
}

cNpcDialog::~cNpcDialog()
{
}

void cNpcDialog::SelectNode( cNpcSceneNode* node )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_NPC);
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
		if( (cNpcSceneNode*)list->GetItemData( i ) == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 인덱스
	unsigned int index = node->GetUserData0();
	const cNpcInfo* info = VIEW->GetNpcInfo( index );
	assert( info );

	cString str;
	str.Format( "%d", index );
	GetDlgItem(IDC_EDIT_NPC_INDEX)->SetWindowText( str.Cstr() );

	/// 이름
	GetDlgItem(IDC_EDIT_NPC_NAME)->SetWindowText( VIEW->GetNpcNameByNameIndex(info->mNameIndex).Cstr() );

	/// 모델
	GetDlgItem(IDC_EDIT_NPC_MODEL)->SetWindowText( VIEW->GetModelFile(info->mModelIndex).Cstr() );

	/// 이동, 회전
	UpdateNodeTransform( node );
}

void cNpcDialog::UpdateCombo( const tMap<unsigned int, cNpcInfo*>& infoMap )
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_NPC_INDEX);
	combo->ResetContent();
	cString str;

	typedef tMap<unsigned int, cNpcInfo*> cInfoMap;
	cInfoMap::cConstIterator ii = infoMap.Begin();
	cInfoMap::cConstIterator iiend = infoMap.End();

	for( unsigned int c = 0; ii != iiend; ++ii, ++c )
	{
		unsigned int npcIndex = ii->mFirst;
		cNpcInfo* info = ii->mSecond;

		str.Format( "%d %s", npcIndex, VIEW->GetNpcNameByNameIndex(info->mNameIndex).Cstr() );

		combo->AddString( str.Cstr() );
		combo->SetItemData( c, (DWORD)npcIndex );
	}

	combo->SetCurSel( 0 );
}

void cNpcDialog::UpdateList( const tMap<unsigned int, cNpcSceneNode*>& nodeMap )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_NPC);
	list->ResetContent();
	cString str;

	typedef tMap<unsigned int, cNpcSceneNode*> cNpcMap;
	cNpcMap::cConstIterator i = nodeMap.Begin();
	cNpcMap::cConstIterator iend = nodeMap.End();

	for( unsigned int c = 0; i != iend; ++i, ++c )
	{
		unsigned int idx = i->mFirst;
		cNpcSceneNode* n = i->mSecond;
		unsigned int classIndex = n->GetUserData0();

		str.Format( "%d %s", idx, VIEW->GetNpcNameByNpcIndex(classIndex).Cstr() );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)n );
	}
}

void cNpcDialog::UpdateNodeTransform( cNpcSceneNode* node )
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

void cNpcDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_NPC_SAVE, mSaveButton );
	DDX_Control( dx, IDC_BUTTON_NPC_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_NPC_CHANGE, mChangeButton );
	DDX_Control( dx, IDC_BUTTON_NPC_DELETE, mDeleteButton );
}

BOOL cNpcDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//
	mTxNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_NPC_TX );
	mTyNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_NPC_TY );
	mRzNum.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_NUM_NPC_RZ );

	SetEnabled( false );
	return TRUE;
}

void cNpcDialog::OnClickedSave()
{
	VIEW->SaveNpcRegen();
}

void cNpcDialog::OnClickedCreate()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_NPC_INDEX);
	unsigned int index = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	int ret = VIEW->CreateNpc( index, 0.0f, 0.0f, 0.0f, true, true );

	switch( ret )
	{
	case -1:
		MessageBox( "Same npc exists.", "Npc" );
		break;
	case -2:
		MessageBox( "Failed to create npc scene node.", "Npc" );
		break;
	}
}

void cNpcDialog::OnClickedChange()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_NPC_INDEX);
	unsigned int index = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	int ret = VIEW->ChangeNpc( index );

	switch( ret )
	{
	case -1:
		MessageBox( "Select npc first.", "Npc" );
		break;
	case -2:
		MessageBox( "Same npc exists.", "Npc" );
		break;
	}
}

void cNpcDialog::OnClickedDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_NPC );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cNpcSceneNode* node = (cNpcSceneNode*)list->GetItemData( i );
	int ret = VIEW->DeleteNpc( node );

	switch( ret )
	{
	case -1:
		MessageBox( "Select npc.", "Npc" );
		break;
	}
}

void cNpcDialog::OnSelchangeNode()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_NPC );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cNpcSceneNode* node = (cNpcSceneNode*)list->GetItemData( i );
	VIEW->SelectNpc( node );
}

LRESULT cNpcDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	switch( id )
	{
	case IDC_NUM_NPC_TX:
	case IDC_NUM_NPC_TY:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
		}
		break;
	case IDC_NUM_NPC_RZ:
		{
			float zangle = D3DXToRadian( mRzNum.GetValue() );
			VIEW->SetSelRotate( zangle, false );
		}
		break;
	}
	return 0;
}
