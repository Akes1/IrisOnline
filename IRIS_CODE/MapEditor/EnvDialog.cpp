#include "stdafx.h"
#include "EnvDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "EnvCameraDialog.h"
#include "EnvLevelDistanceDialog.h"
#include "EnvSkyDialog.h"
#include "EnvDirLightDialog.h"

IMPLEMENT_DYNAMIC(cEnvDialog, CDialog)

BEGIN_MESSAGE_MAP(cEnvDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cEnvDialog::cEnvDialog()
: mCameraDialog( 0 )
, mLevelDistanceDialog( 0 )
, mSkyDialog( 0 )
, mDirLightDialog( 0 )
{
}

cEnvDialog::~cEnvDialog()
{
}

int cEnvDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우를 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 카메라 페이지를 추가
	mCameraDialog = new cEnvCameraDialog;
	mCameraDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_ENV_CAMERA), &mRollupWnd );
	mRollupWnd.InsertPage( "Global Camera", mCameraDialog );

	/// 레벨 거리 페이지를 추가
	mLevelDistanceDialog = new cEnvLevelDistanceDialog;
	mLevelDistanceDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_ENV_LEVEL_DISTANCE), &mRollupWnd );
	mRollupWnd.InsertPage( "Global Visible Level Distance", mLevelDistanceDialog );

	/// 하늘 페이지를 추가
	mSkyDialog = new cEnvSkyDialog;
	mSkyDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_ENV_SKY), &mRollupWnd );
	mRollupWnd.InsertPage( "Global Sky", mSkyDialog );

	/// 방향광 페이지를 추가
	mDirLightDialog = new cEnvDirLightDialog;
	mDirLightDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_ENV_DIRLIGHT), &mRollupWnd );
	mRollupWnd.InsertPage( "Global Dir Light", mDirLightDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cEnvDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_ENV, false );
	CDialog::OnClose();
}

void cEnvDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
