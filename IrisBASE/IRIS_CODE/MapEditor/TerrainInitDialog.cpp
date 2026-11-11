#include "stdafx.h"
#include "TerrainInitDialog.h"

#include "Resource.h"

IMPLEMENT_DYNAMIC( cTerrainInitDialog, CDialog )

BEGIN_MESSAGE_MAP( cTerrainInitDialog, CDialog )
	ON_LBN_SELCHANGE(IDC_LIST_TINIT_RESOLUTION, OnSelchangeResolution)
END_MESSAGE_MAP()

cTerrainInitDialog::cTerrainInitDialog( CWnd* parent /*=0*/ )
: CDialog( IDD_DIALOG_TERRAIN_INIT, parent )
, mResolution( 0 )
, mMetersPerVertex( 0 )
{
}

cTerrainInitDialog::~cTerrainInitDialog()
{
}

void cTerrainInitDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );
}

BOOL cTerrainInitDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TINIT_RESOLUTION );
	list->AddString( "(8) 512" );
	list->AddString( "(8) 1024" );
	list->SetCurSel( 0 );

	list = (CListBox*)GetDlgItem( IDC_LIST_TINIT_METERSPERVERTEX );
	list->AddString( "1" );
	list->SetCurSel( 0 );

	SetDlgItemText( IDC_EDIT_TINIT_SIZE, "512 meters" );
	mResolution = 512;
	mMetersPerVertex = 1;
	return TRUE;
}

void cTerrainInitDialog::OnSelchangeResolution()
{
	ChangeTerrainSize();
}

void cTerrainInitDialog::ChangeTerrainSize()
{
	static int resolution[2] = { 512, 1024 };

	/// 지형 해상도를 얻음
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TINIT_RESOLUTION );
	int i = list->GetCurSel();
	if( i == LB_ERR )
		i = 0;

	/// 지형 크기를 설정
	cString s;
	s.Format( "%d meters", resolution[i] );
	SetDlgItemText( IDC_EDIT_TINIT_SIZE, s.Cstr() );
	mResolution = resolution[i];
}
