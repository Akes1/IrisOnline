#include "stdafx.h"

#include "CameraDialog.h"
#include "Resource.h"
#include "RegenToolView.h"

IMPLEMENT_DYNAMIC(cCameraDialog, CDialog)

BEGIN_MESSAGE_MAP(cCameraDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_PERSP, OnClickedZoomPersp)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_ORTHO, OnClickedZoomOrtho)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedFarDist)
END_MESSAGE_MAP()

cCameraDialog::cCameraDialog()
{
}

cCameraDialog::~cCameraDialog()
{
}

void cCameraDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

	DDX_Control( dx, IDC_BUTTON_ZOOM_PERSP, mZoomPerspButton );
	DDX_Control( dx, IDC_BUTTON_ZOOM_ORTHO, mZoomOrthoButton );
}

BOOL cCameraDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	mFarDist.Create( 10.0f, 2000.0f, 500.0f, 1.0f, this, IDC_EDIT_CAMERA_FARDIST );

	return TRUE;
}

void cCameraDialog::OnClickedZoomPersp()
{
	VIEW->ZoomPersp();
}

void cCameraDialog::OnClickedZoomOrtho()
{
	VIEW->ZoomOrtho();
	mFarDist.SetValue( VIEW->GetCamFarDistance() / 100.0f );
}

LRESULT cCameraDialog::OnChangedFarDist( WPARAM /*id*/, LPARAM )
{
	float farDist = mFarDist.GetValue() * 100.0f;
	VIEW->SetCamFarDistance( farDist );
	return 0;
}
