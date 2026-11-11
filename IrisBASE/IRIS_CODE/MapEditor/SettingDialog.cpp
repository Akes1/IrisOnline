#include "stdafx.h"
#include "SettingDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "SettingViewDialog.h"
#include "SettingControlDialog.h"

IMPLEMENT_DYNAMIC(cSettingDialog, CDialog)

BEGIN_MESSAGE_MAP(cSettingDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cSettingDialog::cSettingDialog()
: mViewDialog( 0 )
, mControlDialog( 0 )
{
}

cSettingDialog::~cSettingDialog()
{
}

int cSettingDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 뷰 페이지 추가
	mViewDialog = new cSettingViewDialog;
	mViewDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_SETTING_VIEW), &mRollupWnd );
	mRollupWnd.InsertPage( "View", mViewDialog );

	/// 컨트롤 페이지 추가
	mControlDialog = new cSettingControlDialog;
	mControlDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_SETTING_CONTROL), &mRollupWnd );
	mRollupWnd.InsertPage( "Control", mControlDialog );

	/// 옵션 속성 페이지 추가

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cSettingDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_SETTING, false );
	CDialog::OnClose();
}

void cSettingDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
