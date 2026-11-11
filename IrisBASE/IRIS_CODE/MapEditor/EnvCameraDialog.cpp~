#include "stdafx.h"
#include "EnvCameraDialog.h"

#include "resource.h"
#include "MapEditorView.h"
#include "EnvEditor.h"
#include "AreaFogDialog.h"

IMPLEMENT_DYNAMIC(cEnvCameraDialog, CDialog)

BEGIN_MESSAGE_MAP(cEnvCameraDialog, CDialog)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedFarDist)
END_MESSAGE_MAP()

cEnvCameraDialog::cEnvCameraDialog()
{
}

cEnvCameraDialog::~cEnvCameraDialog()
{
}

void cEnvCameraDialog::Update( float farDist )
{
	mFarDist.SetValue( farDist / 100.0f );
}

void cEnvCameraDialog::DoDataExchange( CDataExchange* dx )
{
	CDialog::DoDataExchange( dx );

}

BOOL cEnvCameraDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	/// 카메라 원거리
	mFarDist.Create( 10.0f, GAME_CAM_FAR_DIST_METER, GAME_CAM_FAR_DIST_METER, 1.0f, this, IDC_EDIT_ACAM_FARDIST );

	return TRUE;
}

LRESULT cEnvCameraDialog::OnChangedFarDist( WPARAM /*id*/, LPARAM )
{
	float farDist = mFarDist.GetValue() * 100.0f;

	ENVEDIT->SetCamFarDistance( farDist );
	return 0;
}
