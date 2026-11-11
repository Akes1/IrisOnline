#include "stdafx.h"
#include "MonsterDialog.h"

#include "RegenToolView.h"
#include "MonsterSceneNode.h"

IMPLEMENT_DYNAMIC(cMonsterDialog, CDialog)

BEGIN_MESSAGE_MAP(cMonsterDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_SAVE, OnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_MONGROUP_CREATE, OnClickedGroupCreate)
	ON_BN_CLICKED(IDC_BUTTON_MONGROUP_DELETE, OnClickedGroupDelete)
	ON_LBN_SELCHANGE(IDC_LIST_MONGROUP, OnSelchangeGroup)
	ON_BN_CLICKED(IDC_CHECK_MONGROUP_FIRST_REGEN, OnCheckedGroupFirstRegen)
	ON_BN_CLICKED(IDC_CHECK_MONGROUP_CHANNEL_CHECK, OnCheckedChannelCheck)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_CREATE, OnClickedCreate)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_CHANGE, OnClickedChange)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_DELETE, OnClickedDelete)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_FAMILY_NUM_NEW, OnClickedFamilyNumNew)
	ON_BN_CLICKED(IDC_BUTTON_MONSTER_FAMILY_NUM_CHANGE, OnClickedFamilyNumChange)
	ON_LBN_SELCHANGE(IDC_COMBO_MONSTER_PATTERN_IDX, OnSelchangeMonsterPatternIndex)
	ON_LBN_SELCHANGE(IDC_LIST_MONSTER, OnSelchangeNode)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MONGROUP_REGEN_INDEX, OnItemChanged)
END_MESSAGE_MAP()

cMonsterDialog::cMonsterDialog()
: mSelMonsterGroup( 0 )
, mSelMonster( 0 )
{
	mInitedNextRegenGroupList = false;
}

cMonsterDialog::~cMonsterDialog()
{
}

void cMonsterDialog::Clear()
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONGROUP);
	list->ResetContent();

	mNextRegenGroupList.DeleteAllItems();

	list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER);
	list->ResetContent();
}

void cMonsterDialog::SelectGroup( cMonsterGroup* group )
{
	mInitedNextRegenGroupList = false;

	mSelMonsterGroup = group;
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONGROUP);
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
		if( (cMonsterGroup*)list->GetItemData( i ) == group )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 그룹 데이타를 갱신
	{
		/// 최초 리젠 여부
		CButton* check = (CButton*)GetDlgItem( IDC_CHECK_MONGROUP_FIRST_REGEN );
		check->SetCheck( group->mFirstRegen ? 1 : 0 );

		/// 채널간 공유 체크
		check = (CButton*)GetDlgItem( IDC_CHECK_MONGROUP_CHANNEL_CHECK );
		check->SetCheck( group->mChannelCheck ? 1 : 0 );

		/// 리젠 인덱스
		unsigned int i=0, iend = mNextRegenGroupList.GetItemCount();
		for( ; i< iend; ++i )
		{
			unsigned int idx = mNextRegenGroupList.GetItemData(i);
			if( group->mNextRegenGroupMap.Find( idx ) != group->mNextRegenGroupMap.End() )
				mNextRegenGroupList.SetCheck( i, TRUE );
			else
				mNextRegenGroupList.SetCheck( i, FALSE );
		}

		mInitedNextRegenGroupList = true;

		/// 맵 그룹 인덱스
		mMapGroupIndex.SetValue( (float)group->mMapGroupIndex );

		/// 리젠 퍼센트
		mGroupRegenPercentNum.SetValue( (float)group->mRegenPercent );

		/// 리젠 범위
		mGroupRegenRangeNum.SetValue( (float)group->mRegenRange );

		/// 리젠 대기 시간
		mGroupRegenWaitTimeNum.SetValue( (float)group->mRegenWaitTime );
	}

	/// 몬스터 리스트를 갱신
	UpdateList( group->mNodeList );
}

void cMonsterDialog::SelectNode( cMonsterSceneNode* node )
{
	mSelMonster = node;
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER);
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
		if( (cMonsterSceneNode*)list->GetItemData( i ) == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 인덱스
	unsigned int index = node->GetUserData0();
	const cMonsterInfo* info = VIEW->GetMonsterInfo( index );
	assert( info );

	cString str;
	str.Format( "%d", index );
	GetDlgItem(IDC_EDIT_MONSTER_INDEX)->SetWindowText( str.Cstr() );

	/// 이름
	GetDlgItem(IDC_EDIT_MONSTER_NAME)->SetWindowText( VIEW->GetMonsterNameByNameIndex(info->mNameIndex).Cstr() );

	/// 모델
	GetDlgItem(IDC_EDIT_MONSTER_MODEL)->SetWindowText( VIEW->GetModelFile(info->mModelIndex).Cstr() );

	/// 리젠 대기 시간
	mRegenWaitTimeNum.SetValue( (float)node->GetUserData1() );

	/// 리젠 유지 시간
	mRegenLifeTimeNum.SetValue( (float)node->GetUserData3() );

	/// 동족그룹 번호
	str.Format( "%d", node->GetUserData4() );
	GetDlgItem(IDC_EDIT_MONSTER_FAMILY_NUM)->SetWindowText( str.Cstr() );

	/// 패턴 인덱스
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MONSTER_PATTERN_IDX);
	unsigned int j = 0, jend = combo->GetCount();

	for( ; j < jend; ++j )
	{
		unsigned int comboData = (unsigned int)combo->GetItemData(j);
		if( comboData == node->GetUserData5() )
		{
			combo->SetCurSel( j );
			break;
		}
	}
	if( j == jend )
	{
		combo->SetCurSel( -1 );
		node->SetUserData5( 0 );
	}

	/// 이동, 회전
	UpdateNodeTransform( node );
}

void cMonsterDialog::UpdateCombo( const tMap<unsigned int, cMonsterInfo*>& infoMap )
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MONSTER_INDEX);
	combo->ResetContent();
	cString str;

	typedef tMap<unsigned int, cMonsterInfo*> cInfoMap;
	cInfoMap::cConstIterator ii = infoMap.Begin();
	cInfoMap::cConstIterator iiend = infoMap.End();

	for( unsigned int c = 0; ii != iiend; ++ii, ++c )
	{
		unsigned int monIndex = ii->mFirst;
		cMonsterInfo* info = ii->mSecond;

		str.Format( "%d %s", monIndex, VIEW->GetMonsterNameByNameIndex(info->mNameIndex).Cstr() );

		combo->AddString( str.Cstr() );
		combo->SetItemData( c, (DWORD)monIndex );
	}

	combo->SetCurSel( 0 );
}

void cMonsterDialog::UpdateGroupList( const tMap<unsigned int, cMonsterGroup*>& groupMap )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONGROUP);
	list->ResetContent();

	mNextRegenGroupList.DeleteAllItems();

	cString str;
	typedef tMap<unsigned int, cMonsterGroup*> cGroupMap;
	cGroupMap::cConstIterator ii = groupMap.Begin();
	cGroupMap::cConstIterator iiend = groupMap.End();
	for( unsigned int c = 0; ii != iiend; ++ii, ++c )
	{
		unsigned int groupIndex = ii->mFirst;
		cMonsterGroup* group = ii->mSecond;

		str.Format( "%d {...}", groupIndex );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)group );

		mNextRegenGroupList.InsertItem(c, str.Cstr());
		mNextRegenGroupList.SetItemData( c, (DWORD)groupIndex );
	}

	mInitedNextRegenGroupList = false;
}

void cMonsterDialog::UpdateList( const tList<cMonsterSceneNode*>& nodeList )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER);
	list->ResetContent();
	cString str;

	typedef tList<cMonsterSceneNode*> cMonsterList;
	cMonsterList::cConstIterator i = nodeList.Begin();
	cMonsterList::cConstIterator iend = nodeList.End();

	for( unsigned int c = 0; i != iend; ++i, ++c )
	{
		cMonsterSceneNode* n = *i;
		unsigned int monIndex = n->GetUserData0();

		str.Format( "%d %s", monIndex, VIEW->GetMonsterNameByMonIndex(monIndex).Cstr() );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)n );
	}
}

void cMonsterDialog::UpdateNodeTransform( cMonsterSceneNode* node )
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

void cMonsterDialog::UpdateFamilyList( tMap<unsigned int, void*>* pFamilyMap )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER_FAMILY);
	list->ResetContent();
	cString str;

	typedef tHashSet<unsigned long> cHashMap;
	typedef tMap<unsigned int, void*> cFamilyMap;	
	cFamilyMap::cIterator i = pFamilyMap->Begin();
	cFamilyMap::cIterator e = pFamilyMap->End();

	for( unsigned int c = 0; i != e; ++i, ++c )
	{
		unsigned int familyNum = (*i).mFirst;
		cHashMap* n = (cHashMap*)(*i).mSecond;
		
		str.Format( "%d", familyNum );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)n );
	}
}

void cMonsterDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_MONSTER_SAVE, mSaveButton );
	DDX_Control( dx, IDC_BUTTON_MONGROUP_CREATE, mGroupCreateButton );
	DDX_Control( dx, IDC_BUTTON_MONGROUP_DELETE, mGroupDeleteButton );
	DDX_Control( dx, IDC_BUTTON_MONSTER_CREATE, mCreateButton );
	DDX_Control( dx, IDC_BUTTON_MONSTER_CHANGE, mChangeButton );
	DDX_Control( dx, IDC_BUTTON_MONSTER_DELETE, mDeleteButton );
	DDX_Control( dx, IDC_BUTTON_MONSTER_FAMILY_NUM_NEW, mFamilyNewButton );
	DDX_Control( dx, IDC_BUTTON_MONSTER_FAMILY_NUM_CHANGE, mFamilyChangeButton );
	DDX_Control( dx, IDC_LIST_MONGROUP_REGEN_INDEX, mNextRegenGroupList);
}

BOOL cMonsterDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mGroupIndexNum.Create( 1.0f, 1000.0f, 1.0f, 1.0f, this, IDC_NUM_MONGROUP_INDEX );
	mMapGroupIndex.Create( 0.0f, 1000.0f, 0.0f, 1.0f, this, IDC_NUM_MONGROUP_MAPGROUP_INDEX );
	mGroupRegenPercentNum.Create( 0.0f, 100.0f, 50.0f, 5.0f, this, IDC_NUM_MONGROUP_REGEN_PERCENT );
	mGroupRegenRangeNum.Create( 10.0f, 1000.0f, 100.0f, 10.0f, this, IDC_NUM_MONGROUP_REGEN_RANGE );
	mGroupRegenWaitTimeNum.Create( 100.0f, 1000000.0f, 10000.0f, 1000.0f, this, IDC_NUM_MONGROUP_REGEN_WAIT_TIME );
	mTxNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_MONSTER_TX );
	mTyNum.Create( -100000.0f, 100000.0f, 0.0f, 0.1f, this, IDC_NUM_MONSTER_TY );
	mRzNum.Create( -179.0f, 179.0f, 0.0f, 1.0f, this, IDC_NUM_MONSTER_RZ );
	mRegenWaitTimeNum.Create( 100.0f, 1000000.0f, 100000.0f, 1000.0f, this, IDC_NUM_MONSTER_REGEN_WAIT_TIME );
	mRegenLifeTimeNum.Create( 0.0f, 1000000.0f, 0.0f, 500.0f, this, IDC_NUM_MONSTER_REGEN_LIFE_TIME );
	mFamilyNum.Create( 0.0f, 1000000.0f, 0.0f, 1.0f, this, IDC_NUM_MONSTER_FAMILY_NUM );

	CRect rect;
	mNextRegenGroupList.GetClientRect(rect);
	mNextRegenGroupList.InsertColumn(0, _T(""), LVCFMT_LEFT, 80);

	mNextRegenGroupList.SetExtendedStyle( mNextRegenGroupList.GetExtendedStyle() | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT );
	mNextRegenGroupList.Init();

	SetGroupEnabled( false );
	SetEnabled( false );
	return TRUE;
}

void cMonsterDialog::OnClickedSave()
{
	VIEW->SaveMonsterRegen();
}

void cMonsterDialog::OnClickedGroupCreate()
{
	unsigned int index = (unsigned int)mGroupIndexNum.GetValue();
	int ret = VIEW->CreateMonsterGroup( index );

	switch( ret )
	{
	case 0:
		MessageBox( "Failed to create monster group.", "Monster Group" );
		break;
	case -1:
		MessageBox( "Same group exists.", "Monster Group" );
		break;
	}
}

void cMonsterDialog::OnClickedGroupDelete()
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER);
	list->ResetContent();

	///
	list = (CListBox*)GetDlgItem(IDC_LIST_MONGROUP);
	int i = list->GetCurSel();

	if( i == LB_ERR )
		return;

	cMonsterGroup* group = (cMonsterGroup*)list->GetItemData( i );
	assert( group );

	VIEW->DeleteMonsterGroup( group->mIndex );
}

void cMonsterDialog::OnSelchangeGroup()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_MONGROUP );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cMonsterGroup* group = (cMonsterGroup*)list->GetItemData( i );

    VIEW->SelectMonsterGroup( group );
}

void cMonsterDialog::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if( mSelMonsterGroup == 0 )
		return;

	NMLISTVIEW* pNMLV = (NMLISTVIEW*)pNMHDR;
	*pResult = 0;

	if( mInitedNextRegenGroupList == false )
		return ;

	if( pNMLV->uChanged & LVIF_STATE )
	{
		if( (pNMLV->uOldState & LVIS_STATEIMAGEMASK) != (pNMLV->uNewState & LVIS_STATEIMAGEMASK) )
		{
			unsigned int idx = mNextRegenGroupList.GetItemData( pNMLV->iItem );

			if( (pNMLV->uOldState & 0x1000) && (pNMLV->uNewState & 0x2000) )
			{
				mSelMonsterGroup->mNextRegenGroupMap.Insert( idx, idx );
			}
			else if( (pNMLV->uOldState & 0x2000) && (pNMLV->uNewState & 0x1000) )
			{
				mSelMonsterGroup->mNextRegenGroupMap.Erase( idx );
			}
		}
	}
}

void cMonsterDialog::OnCheckedGroupFirstRegen()
{
	if( mSelMonsterGroup == 0 )
		return;

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_MONGROUP_FIRST_REGEN );

	mSelMonsterGroup->mFirstRegen = check->GetCheck() != 0;
}

void cMonsterDialog::OnCheckedChannelCheck()
{
	if( mSelMonsterGroup == 0 )
		return;

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_MONGROUP_CHANNEL_CHECK );

	mSelMonsterGroup->mChannelCheck = check->GetCheck() != 0;
}


void cMonsterDialog::OnClickedCreate()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MONSTER_INDEX);
	unsigned int monIndex = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	unsigned int regenWaitTime = (unsigned int)mRegenWaitTimeNum.GetValue();
	unsigned int regenLifeTime = (unsigned int)mRegenLifeTimeNum.GetValue();
	int ret = VIEW->CreateMonster( monIndex, 0, 0, 0, regenWaitTime, regenLifeTime, true, true, 0, 0 );

	switch( ret )
	{
	case -1:
		MessageBox( "Failed to create monster scene node.", "Monster" );
		break;
	}
}

void cMonsterDialog::OnClickedChange()
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MONSTER_INDEX);
	unsigned int monIndex = (unsigned int)combo->GetItemData( combo->GetCurSel() );

	///
	int ret = VIEW->ChangeMonster( monIndex );

	switch( ret )
	{
	case -1:
		MessageBox( "Select monster.", "Monster" );
		break;
	case -2:
		MessageBox( "Same monster exists.", "Monster" );
		break;
	}
}

void cMonsterDialog::OnClickedDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_MONSTER );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cMonsterSceneNode* node = (cMonsterSceneNode*)list->GetItemData( i );
	int ret = VIEW->DeleteMonster( node );

	switch( ret )
	{
	case -1:
		MessageBox( "Select monster.", "Monster" );
		break;
	}
}

void cMonsterDialog::OnClickedFamilyNumNew()
{
	/// 신규로 생성할 그룹idx
	unsigned long newfamilyNum = (unsigned long)mFamilyNum.GetValue();

	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER_FAMILY);
	list->SetCurSel( -1 );
	unsigned int c = 0;
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i, ++c )
	{
		CString string;
		list->GetText( i, string );

		unsigned long familyIdx = _ttoi(string);
		if( familyIdx == newfamilyNum )
			return;
	}

	if( mSelMonster != NULL )
		mSelMonster->SetUserData4( newfamilyNum );

	typedef tHashSet<unsigned long> cHashSet;
	cHashSet* pSet = new cHashSet;

	cString str;
	str.Format( "%d", newfamilyNum );
	list->AddString( str.Cstr() );
	list->SetItemData( c, (DWORD)pSet );
	list->SetCurSel( c );

	VIEW->AddFamilyGroup( newfamilyNum, pSet );

	OnClickedFamilyNumChange();
}

void cMonsterDialog::OnClickedFamilyNumChange()
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_MONSTER_FAMILY);
	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	CString string;
	list->GetText( i, string );

	/// 그룹번호
	GetDlgItem(IDC_EDIT_MONSTER_FAMILY_NUM)->SetWindowText( string );

	int familyNum = _ttoi(string);
	if( mSelMonster != NULL )
	mSelMonster->SetUserData4( familyNum );
}

void cMonsterDialog::OnSelchangeMonsterPatternIndex()
{
	if( mSelMonster == 0 )
		return;

	CComboBox* combo = (CComboBox*)GetDlgItem( IDC_COMBO_MONSTER_PATTERN_IDX );

	int i = combo->GetCurSel();
	if( i == LB_ERR )
		return;

	unsigned int patternIdx = (unsigned int)combo->GetItemData( i );
	mSelMonster->SetUserData5( patternIdx );

	VIEW->SelectPattern( patternIdx );
}

void cMonsterDialog::OnSelchangeNode()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_MONSTER );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cMonsterSceneNode* node = (cMonsterSceneNode*)list->GetItemData( i );
	VIEW->SelectMonster( node );
}

LRESULT cMonsterDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	if( mSelMonsterGroup == 0 )
		return 0;

	switch( id )
	{
	case IDC_NUM_MONGROUP_MAPGROUP_INDEX:
		{
			mSelMonsterGroup->mMapGroupIndex = (unsigned int)mMapGroupIndex.GetValue();
			break;
		}
	case IDC_NUM_MONGROUP_REGEN_PERCENT:
		{
			mSelMonsterGroup->mRegenPercent = (unsigned int)mGroupRegenPercentNum.GetValue();
			break;
		}
	case IDC_NUM_MONGROUP_REGEN_RANGE:
		{
			mSelMonsterGroup->mRegenRange = (unsigned int)mGroupRegenRangeNum.GetValue();
			break;
		}
	case IDC_NUM_MONGROUP_REGEN_WAIT_TIME:
		{
			mSelMonsterGroup->mRegenWaitTime = (unsigned int)mGroupRegenWaitTimeNum.GetValue();
			break;
		}
	case IDC_NUM_MONSTER_TX:
	case IDC_NUM_MONSTER_TY:
		{
			float x = mTxNum.GetValue() * 100;
			float y = mTyNum.GetValue() * 100;
			VIEW->SetSelTranslate( NiPoint3(x, y, 0.0f), false );
			break;
		}
	case IDC_NUM_MONSTER_RZ:
		{
			float zangle = D3DXToRadian( mRzNum.GetValue() );
			VIEW->SetSelRotate( zangle, false );
			break;
		}
	case IDC_NUM_MONSTER_REGEN_WAIT_TIME:
		{
			if( mSelMonster )
				mSelMonster->SetUserData1( (unsigned int)mRegenWaitTimeNum.GetValue() );
			break;
		}
	case IDC_NUM_MONSTER_REGEN_LIFE_TIME:
		{
			if( mSelMonster )
				mSelMonster->SetUserData3( (unsigned int)mRegenLifeTimeNum.GetValue() );
			break;
		}
	}
	return 0;
}


void cMonsterDialog::UpdatePatternCombo( tPointerHashMap<unsigned int, cPattern*>* infoMap )
{
	CComboBox* combo = (CComboBox*)GetDlgItem(IDC_COMBO_MONSTER_PATTERN_IDX);
	combo->ResetContent();
	cString str;

	typedef tPointerHashMap<unsigned int, cPattern*> cInfoMap;
	cInfoMap::cConstIterator ii = infoMap->Begin();
	cInfoMap::cConstIterator iiend = infoMap->End();

	str.Format( "%d", 0 );

	combo->AddString( str.Cstr() );
	combo->SetItemData( 0, (DWORD)0 );

	for( unsigned int c = 1; ii != iiend; ++ii, ++c )
	{
		unsigned int patternIdx = ii->mFirst;

		str.Format( "%d", patternIdx );

		combo->AddString( str.Cstr() );
		combo->SetItemData( c, (DWORD)patternIdx );
	}

	combo->SetCurSel( 0 );
}
