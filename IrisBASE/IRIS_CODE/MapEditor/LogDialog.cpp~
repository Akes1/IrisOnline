#include "stdafx.h"
#include "LogDialog.h"

#include "Resource.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC( cLogDialog, CDialog )

BEGIN_MESSAGE_MAP( cLogDialog, CDialog )
	ON_LBN_SELCHANGE(IDC_LIST_LOG_DEVISION_RESOLUTION, OnSelchangeResolution)
END_MESSAGE_MAP()

cLogDialog::cLogDialog( CWnd* parent )
: CDialog( IDD_DIALOG_LOG, parent )
, mResolution( 0 )
{
}

cLogDialog::~cLogDialog()
{
}

void cLogDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );
}

BOOL cLogDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_LOG_DEVISION_RESOLUTION );
	list->AddString( "4" );
	list->AddString( "8" );
	list->SetCurSel( 1 );

	mResolution = 4;
	ChangeDevisionSize();
	return TRUE;
}

void cLogDialog::OnSelchangeResolution()
{
	ChangeDevisionSize();
}

void cLogDialog::ChangeDevisionSize()
{
	static int res[2] = { 4, 8 };

	/// 분할 해상도를 얻음
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_LOG_DEVISION_RESOLUTION );
	int i = list->GetCurSel();
	if( i == LB_ERR )
		i = 0;

	/// 분할 크기를 설정
	cString s;
	s.Format( "%d meters", TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex() / res[i] );
	SetDlgItemText( IDC_EDIT_LOG_DEVISION_SIZE, s.Cstr() );
	mResolution = res[i];
}
