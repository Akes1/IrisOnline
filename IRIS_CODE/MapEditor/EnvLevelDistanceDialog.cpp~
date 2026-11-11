#include "stdafx.h"
#include "EnvLevelDistanceDialog.h"

#include "resource.h"

#include "EnvEditor.h"

IMPLEMENT_DYNAMIC(cEnvLevelDistanceDialog, CDialog)

BEGIN_MESSAGE_MAP(cEnvLevelDistanceDialog, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

cEnvLevelDistanceDialog::cEnvLevelDistanceDialog()
{
}

cEnvLevelDistanceDialog::~cEnvLevelDistanceDialog()
{
}

void cEnvLevelDistanceDialog::Update( unsigned int dist0, unsigned int dist1, unsigned int dist2 )
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL0_DIST );
	slider->SetPos( dist0 );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL0_DIST, slider->GetPos() );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL1_DIST );
	slider->SetPos( dist1 );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL1_DIST, slider->GetPos() );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL2_DIST );
	slider->SetPos( dist2 );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL2_DIST, slider->GetPos() );
}

void cEnvLevelDistanceDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}

BOOL cEnvLevelDistanceDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 레벨 거리
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL0_DIST );
	slider->SetRange( 10, 300 );
	slider->SetPos( 50 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_ENV_LEVEL0_DIST ), FALSE );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL0_DIST, slider->GetPos() );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL1_DIST );
	slider->SetRange( 10, 300 );
	slider->SetPos( 100 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_ENV_LEVEL1_DIST ), FALSE );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL1_DIST, slider->GetPos() );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL2_DIST );
	slider->SetRange( 10, 300 );
	slider->SetPos( 200 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_ENV_LEVEL2_DIST ), FALSE );
	SetDlgItemInt( IDC_EDIT_ENV_LEVEL2_DIST, slider->GetPos() );
	return TRUE;
}

void cEnvLevelDistanceDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider0 = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL0_DIST );
	unsigned int dist0 = slider0->GetPos();

	CSliderCtrl* slider1 = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL1_DIST );
	unsigned int dist1 = slider1->GetPos();

	CSliderCtrl* slider2 = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_ENV_LEVEL2_DIST );
	unsigned int dist2 = slider2->GetPos();

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_ENV_LEVEL0_DIST:
		{
			if( dist0 >= dist1 )
				dist0 = dist1 - 1;

			slider0->SetPos( dist0 );
			SetDlgItemInt( IDC_EDIT_ENV_LEVEL0_DIST, dist0 );
			ENVEDIT->SetLevelDistance( dist0, dist1, dist2 );
		}
		break;
	case IDC_SLIDER_ENV_LEVEL1_DIST:
		{
			if( dist1 <= dist0 )
				dist1 = dist0 + 1;
			else if( dist1 >= dist2 )
				dist1 = dist2 -1;

			slider1->SetPos( dist1 );
			SetDlgItemInt( IDC_EDIT_ENV_LEVEL1_DIST, dist1 );
			ENVEDIT->SetLevelDistance( dist0, dist1, dist2 );
		}
		break;
	case IDC_SLIDER_ENV_LEVEL2_DIST:
		{
			if( dist2 <= dist1 )
				dist2 = dist1 + 1;

			slider2->SetPos( dist2 );
			SetDlgItemInt( IDC_EDIT_ENV_LEVEL2_DIST, dist2 );
			ENVEDIT->SetLevelDistance( dist0, dist1, dist2 );
		}
		break;
	}
}
