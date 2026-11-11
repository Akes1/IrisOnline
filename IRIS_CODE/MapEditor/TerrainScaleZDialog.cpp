#include "stdafx.h"
#include "TerrainScaleZDialog.h"

#include "resource.h"
#include "TerrainEditor.h"

IMPLEMENT_DYNAMIC(cTerrainScaleZDialog, CDialog)

BEGIN_MESSAGE_MAP(cTerrainScaleZDialog, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_TSCALE_Z, OnSelchangeScale)
END_MESSAGE_MAP()

cTerrainScaleZDialog::cTerrainScaleZDialog( CWnd* parent )
: CDialog(IDD_DIALOG_TERRAIN_SCALE_Z, parent)
, mScale( 0.0f )
{
}

cTerrainScaleZDialog::~cTerrainScaleZDialog()
{
}

void cTerrainScaleZDialog::DoDataExchange( CDataExchange*  dx )
{
	CDialog::DoDataExchange( dx );
}

BOOL cTerrainScaleZDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 크기 배율
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TSCALE_Z );
	list->AddString( "0.0" );
	list->AddString( "0.2" );
	list->AddString( "0.4" );
	list->AddString( "0.5" );
	list->AddString( "0.6" );
	list->AddString( "0.8" );
	list->AddString( "1.0" );
	list->AddString( "1.2" );
	list->AddString( "1.4" );
	list->AddString( "1.5" );
	list->AddString( "1.6" );
	list->AddString( "1.8" );
	list->AddString( "2.0" );
	list->AddString( "5.0" );
	list->AddString( "10.0" );
	list->SetCurSel( 6 );

	OnSelchangeScale();
	return TRUE;
}

void cTerrainScaleZDialog::OnSelchangeScale()
{
	static float scale[15] = { 0.0f, 0.2f, 0.4f, 0.5f, 0.6f, 0.8f, 1.0f, 1.2f, 1.4f, 1.5f, 1.6f, 1.8f, 2.0f, 5.0f, 10.0f };

	/// 배율을 얻음
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_TSCALE_Z );
	int i = list->GetCurSel();
	if( i == LB_ERR )
		i = 0;

	mScale = scale[i];
}
