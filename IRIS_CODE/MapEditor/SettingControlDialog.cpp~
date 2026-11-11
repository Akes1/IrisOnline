#include "stdafx.h"
#include "SettingControlDialog.h"

#include "resource.h"

#include "Engine/CameraManager.h"

IMPLEMENT_DYNAMIC(cSettingControlDialog, CDialog)

BEGIN_MESSAGE_MAP(cSettingControlDialog, CDialog)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

cSettingControlDialog::cSettingControlDialog()
{
}

cSettingControlDialog::~cSettingControlDialog()
{
}

int cSettingControlDialog::GetMouseWheel() const
{
	return ((CSliderCtrl*)GetDlgItem( IDC_SLIDER_SETCTRL_MOUSE_WHEEL ))->GetPos();
}

void cSettingControlDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange(dx);
}

BOOL cSettingControlDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 카메라 속도
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_SETCTRL_CAMERA_TSPEED );
	slider->SetRange( 10, 100 );
	slider->SetPos( 70 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_SETCTRL_CAMERA_TSPEED ), FALSE );
	SetDlgItemInt( IDC_EDIT_SETCTRL_CAMERA_TSPEED, slider->GetPos() );

	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_SETCTRL_CAMERA_RSPEED );
	slider->SetRange( 5, 100 );
	slider->SetPos( 15 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_SETCTRL_CAMERA_RSPEED ), FALSE );
	SetDlgItemInt( IDC_EDIT_SETCTRL_CAMERA_RSPEED, slider->GetPos() );

	/// 마우스 휠
	slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_SETCTRL_MOUSE_WHEEL );
	slider->SetRange( 1, 100 );
	slider->SetPos( 30 );
	slider->SetBuddy( GetDlgItem( IDC_EDIT_SETCTRL_MOUSE_WHEEL ), FALSE );
	SetDlgItemInt( IDC_EDIT_SETCTRL_MOUSE_WHEEL, slider->GetPos() );
	return TRUE;
}

void cSettingControlDialog::OnHScroll( UINT /*code*/, UINT /*pos*/, CScrollBar* bar )
{
	CSliderCtrl* slider = (CSliderCtrl*)bar;

	switch( bar->GetDlgCtrlID() )
	{
	case IDC_SLIDER_SETCTRL_CAMERA_TSPEED:
		{
			unsigned int speed = slider->GetPos();
			SetDlgItemInt( IDC_EDIT_SETCTRL_CAMERA_TSPEED, speed );
			CAMERAMAN->SetCameraTranslateSpeed( float(speed * 100) );
		}
		break;
	case IDC_SLIDER_SETCTRL_CAMERA_RSPEED:
		{
			unsigned int speed = slider->GetPos();
			SetDlgItemInt( IDC_EDIT_SETCTRL_CAMERA_RSPEED, speed );
			CAMERAMAN->SetCameraRotateSpeed( float(speed) );
		}
		break;
	case IDC_SLIDER_SETCTRL_MOUSE_WHEEL:
		SetDlgItemInt( IDC_EDIT_SETCTRL_MOUSE_WHEEL, slider->GetPos() );
		break;
	}
}
