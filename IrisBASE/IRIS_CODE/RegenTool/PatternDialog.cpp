#include "stdafx.h"
#include "PatternDialog.h"

#include "RegenToolView.h"
#include "Pattern.h"
#include "../mapeditor/PatternSceneNode.h"

IMPLEMENT_DYNAMIC(cPatternDialog, CDialog)

BEGIN_MESSAGE_MAP(cPatternDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_SAVE, OnClickedSave)
	ON_LBN_SELCHANGE(IDC_LIST_PATTERN, OnSelchangePattern)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_CREATE, OnClickedPatternCreate)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_DELETE, OnClickedPatternDelete)
	ON_BN_CLICKED(IDC_CHECK_PATTERN_CIRCULATE, OnCheckedPatternCirculate)
	ON_LBN_SELCHANGE(IDC_LIST_PATTERN_POSITION, OnSelchangePatternPosition)
	ON_BN_CLICKED(IDC_BUTTON_PATTERN_POSITION_DELETE, OnClickedPatternPositionDelete)
	ON_BN_CLICKED(IDC_CHECK_PATTERN_POSITION_RESTINGPLACE, OnCheckedPatternPositionRestingplace)
	ON_BN_CLICKED(IDC_CHECK_PATTERN_POSITION_NEW, OnCheckedPatternPosNew)
	ON_BN_CLICKED(IDC_CHECK_PATTERN_POSITION_UPPER, OnCheckedPatternPosUpper)
	ON_BN_CLICKED(IDC_CHECK_PATTERN_POSITION_MODIFY, OnCheckedPatternPosModify)

	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedNumber)
END_MESSAGE_MAP()

cPatternDialog::cPatternDialog()
{
}

cPatternDialog::~cPatternDialog()
{
}

void cPatternDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );
	DDX_Control( dx, IDC_BUTTON_PATTERN_SAVE, mSaveButton );
	DDX_Control( dx, IDC_BUTTON_PATTERN_CREATE, mPatternCreate );
	DDX_Control( dx, IDC_BUTTON_PATTERN_DELETE, mPatternDelete );
	DDX_Control( dx, IDC_BUTTON_PATTERN_POSITION_DELETE, mPositionDelete );
}

BOOL cPatternDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mPatternIndex.Create( 1.0f, 10000.0f, 1.0f, 1.0f, this, IDC_NUM_PATTERN_INDEX );
	mPositionRange.Create( 0.0f, 10000.0f, 1.0f, 1.0f, this, IDC_NUM_PATTERN_POSITION_RANGE );
	mPositionWaitTime.Create( 0.0f, 1000000000.0f, 1.0f, 1.0f, this, IDC_NUM_PATTERN_POSITION_WAITTIME );

	SetPatternEnabled( false );
	SetPositionEnabled( false );
	return TRUE;
}

void cPatternDialog::Clear()
{

}

void cPatternDialog::UpdatePatternList( tPointerHashMap<unsigned int, cPattern*>* pPatternMap )
{
	if( pPatternMap == NULL )
		return;
	
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PATTERN);
	list->ResetContent();
	cString str;

	typedef tPointerHashMap<unsigned int, cPattern*> cPatternMap;
	cPatternMap::cIterator i = pPatternMap->Begin();
	cPatternMap::cIterator e = pPatternMap->End();

	for( unsigned int c = 0; i != e; ++i, ++c )
	{
		unsigned int patternIdx = i->mFirst;
		cPattern* pPattern = i->mSecond;

		str.Format( "%d {...}", patternIdx );

		list->AddString( str.Cstr() );
		list->SetItemData( c, (DWORD)pPattern );
	}
}

void cPatternDialog::SelectPattern( cPattern* pPattern )
{
	mSelPattern = pPattern;

	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PATTERN);
	list->SetCurSel( -1 );

	if( pPattern == 0 )
	{
		SetPatternEnabled( false );
		//SelectNode( 0 );
		return;
	}
	else
	{
		SetPatternEnabled( true );
	}

	/// 리스트
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		if( (cPattern*)list->GetItemData( i ) == pPattern )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 순환여부
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_CIRCULATE );
	check->SetCheck( pPattern->mType ? 1 : 0 );


	/// 위치좌표를 갱신
	UpdatePosList( &pPattern->mSceneNodeList );
}

void cPatternDialog::UpdatePosList( tPointerList<cPatternSceneNode*>* nodelist )
{
	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PATTERN_POSITION);
	list->ResetContent();
	
	if( nodelist )
	{
		cString str;
		typedef tPointerList<cPatternSceneNode*> cNodeList;
		cNodeList::cIterator i = nodelist->Begin();
		cNodeList::cIterator e = nodelist->End();

		for( unsigned int c = 0; i != e; ++i, ++c )
		{
			cPatternSceneNode* n = (cPatternSceneNode*)(*i);
			sPatternPos* pos = (sPatternPos*)n->GetUserData();
			if( pos )
			{
				str.Format( "%d %d", pos->mX, pos->mY );
				list->AddString( str.Cstr() );
				list->SetItemData( c, (DWORD)n );
			}
		}
	}

	SetPositionEnabled( false );
}

void cPatternDialog::SelectPatternPos( cPatternSceneNode* node )
{
	sPatternPos* pos = (sPatternPos*)node->GetUserData();
	mSelPatternPosNode = node;

	CListBox* list = (CListBox*)GetDlgItem(IDC_LIST_PATTERN_POSITION);
	list->SetCurSel( -1 );

	if( node == 0 )
	{
		SetPositionEnabled( false );
		return;
	}
	else
	{
		SetPositionEnabled( true );
	}

	/// 리스트
	for( unsigned int i = 0, iend = list->GetCount(); i < iend; ++i )
	{
		cPatternSceneNode* n = (cPatternSceneNode*)list->GetItemData( i );
		if( n == node )
		{
			list->SetCurSel( i );
			break;
		}
	}

	/// 범위
	mPositionRange.SetValue( (float)pos->mRange );

	/// 경유지
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_RESTINGPLACE );
	check->SetCheck( pos->mRelax ? 1 : 0 );

	/// 대기시간
	mPositionWaitTime.SetValue( (float)pos->mWaitTime );
}

void cPatternDialog::OnSelchangePattern()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PATTERN );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPattern* pPattern = (cPattern*)list->GetItemData( i );
	VIEW->SelectPattern( pPattern->mPatternIdx );
}

void cPatternDialog::OnClickedSave()
{
	VIEW->SavePattern();
}

void cPatternDialog::OnClickedPatternCreate()
{
	unsigned int index = (unsigned int)mPatternIndex.GetValue();
	int ret = VIEW->CreatePattern( index );

	switch( ret )
	{
	case 0:
		MessageBox( "Failed to create Pattern.", "Pattern" );
		break;
	case -1:
		MessageBox( "Same Pattern exists.", "Pattern" );
		break;
	}

	VIEW->UpdatePatternCombo();
}

void cPatternDialog::OnClickedPatternDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PATTERN );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPattern* pattern = (cPattern*)list->GetItemData( i );
	assert( pattern );
	int ret = VIEW->DeletePattern( pattern->mPatternIdx );

	switch( ret )
	{
	case 0:
		MessageBox( "Failed to Delete Pattern.", "Pattern" );
		break;
	}

	VIEW->UpdatePatternCombo();
}

void cPatternDialog::OnCheckedPatternCirculate()
{
	if( mSelPattern == NULL )
		return;

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_CIRCULATE );

	mSelPattern->mType = check->GetCheck() != 0;
}

void cPatternDialog::OnSelchangePatternPosition()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PATTERN_POSITION );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPatternSceneNode* sceneNode = (cPatternSceneNode*)list->GetItemData( i );
	assert( sceneNode );
	VIEW->SelectPatternPos( sceneNode );
}

void cPatternDialog::OnClickedPatternPositionDelete()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PATTERN_POSITION );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		return;

	cPatternSceneNode* sceneNode = (cPatternSceneNode*)list->GetItemData( i );
	assert( sceneNode );
	int ret = VIEW->DeletePatternPos( sceneNode );
	
	switch( ret )
	{
	case 0:
		MessageBox( "Select PatternPosition.", "Pattern" );
		break;
	}
}

cPatternSceneNode* cPatternDialog::GetSelectedPos()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_PATTERN_POSITION );
	int i = list->GetCurSel();
	if( i == LB_ERR )
		return 0;

	cPatternSceneNode* sceneNode = (cPatternSceneNode*)list->GetItemData( i );
	return sceneNode;
}

void cPatternDialog::OnCheckedPatternPositionRestingplace()
{
	if( mSelPatternPosNode == NULL )
		return;

	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_RESTINGPLACE );

	sPatternPos* pos = (sPatternPos*)mSelPatternPosNode->GetUserData();
	if( pos )
	{
		pos->mRelax = check->GetCheck() != 0;
		mSelPatternPosNode->SetWireframe( pos->mRelax );
	}
}

LRESULT cPatternDialog::OnChangedNumber( WPARAM id, LPARAM )
{
	if( mSelPattern == 0 )
		return 0;

	switch( id )
	{
	case IDC_NUM_PATTERN_POSITION_RANGE:
		{
			if( mSelPatternPosNode != NULL )
			{
				sPatternPos* pos = (sPatternPos*)mSelPatternPosNode->GetUserData();
				if( pos )
				{
					pos->mRange = (unsigned long)mPositionRange.GetValue();
				}
			}
		}
		break;
	case IDC_NUM_PATTERN_POSITION_WAITTIME:
		{
			if( mSelPatternPosNode != NULL )
			{
				sPatternPos* pos = (sPatternPos*)mSelPatternPosNode->GetUserData();
				if( pos )
					pos->mWaitTime = (unsigned long)mPositionWaitTime.GetValue();
			}
		}
		break;
	}
	return 0;
}

bool cPatternDialog::IsCheckedUpper()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_UPPER );
	return ( check->GetCheck() != 0 );
}

bool cPatternDialog::IsCheckedNew()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_NEW );
	return ( check->GetCheck() != 0 );
}

bool cPatternDialog::IsCheckedModify()
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_MODIFY );
	return ( check->GetCheck() != 0 );
}

void cPatternDialog::OnCheckedPatternPosNew()
{
	/*CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_MODIFY );
	check->SetCheck( false );
	check->EnableWindow( !IsCheckedNew() );*/
}

void cPatternDialog::OnCheckedPatternPosUpper()
{
	//CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_MODIFY );
	//check->SetCheck( false );
	//check->EnableWindow( !IsCheckedUpper() );
}

void cPatternDialog::OnCheckedPatternPosModify()
{
	//CButton* check1 = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_NEW );
	//check1->SetCheck( false );
	//check1->EnableWindow( !IsCheckedModify() );

	//CButton* check2 = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_UPPER );
	//check2->SetCheck( false );
	//check2->EnableWindow( !IsCheckedModify() );
}

void cPatternDialog::SetCheckNew( bool set )
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_NEW );
	check->SetCheck( set );

	OnCheckedPatternPosNew();
}

void cPatternDialog::SetCheckModify( bool set )
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_PATTERN_POSITION_MODIFY );
	check->SetCheck( set );

	OnCheckedPatternPosModify();
}