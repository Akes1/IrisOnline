#include "stdafx.h"
#include "TerrainImportDialog.h"

#include "resource.h"

IMPLEMENT_DYNAMIC(cTerrainImportDialog, CDialog)

BEGIN_MESSAGE_MAP(cTerrainImportDialog, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

cTerrainImportDialog::cTerrainImportDialog( CWnd* parent )
: CDialog(IDD_DIALOG_TERRAIN_IMPORT, parent)
, mScale( 0.0f )
{
}

cTerrainImportDialog::~cTerrainImportDialog()
{
}

void cTerrainImportDialog::DoDataExchange( CDataExchange*  dx )
{
	CDialog::DoDataExchange( dx );
}


BOOL cTerrainImportDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 크기 배율
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TIMPORT_SCALE );
	slider->SetRange( 1, 100 );
	slider->SetPos( 10 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_TIMPORT_SCALE ), FALSE );
	SetDlgItemInt( IDC_EDIT_TIMPORT_SCALE, slider->GetPos() );

	/// 최대 높이
	GetDlgItem( IDC_EDIT_TIMPORT_MAXHEIGHT )->SetWindowText( "25.50 meters" );

	mScale = 0.1f;
	return TRUE;
}

void cTerrainImportDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	int scale = slider->GetPos();
	SetDlgItemInt( IDC_EDIT_TIMPORT_SCALE, scale );

	cString s;
	s.Format( "%3.2f meters", float(scale * 255) / 100.0f );
	SetDlgItemText( IDC_EDIT_TIMPORT_MAXHEIGHT, s.Cstr() );

	mScale = (float)scale / 100.0f;
}
