#include "stdafx.h"
#include "LightDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "LightTransformDialog.h"
#include "LightPropertyDialog.h"
#include "LightInfoDialog.h"

IMPLEMENT_DYNAMIC(cLightDialog, CDialog)

BEGIN_MESSAGE_MAP(cLightDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cLightDialog::cLightDialog()
: mTransformDialog( 0 )
, mPropertyDialog( 0 )
, mInfoDialog( 0 )
{
}

cLightDialog::~cLightDialog()
{
}

int cLightDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 조명 배치 페이지 추가
	mTransformDialog = new cLightTransformDialog;
	mTransformDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_LIGHT_TRANSFORM), &mRollupWnd );
	mRollupWnd.InsertPage( "Transform", mTransformDialog );

	/// 조명 속성 페이지 추가
	mPropertyDialog = new cLightPropertyDialog;
	mPropertyDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_LIGHT_PROPERTY), &mRollupWnd );
	mRollupWnd.InsertPage( "Property", mPropertyDialog );

	/// 장면 정보 페이지 추가
	mInfoDialog = new cLightInfoDialog;
	mInfoDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_LIGHT_INFO), &mRollupWnd );
	mRollupWnd.InsertPage( "Info", mInfoDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cLightDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_LIGHT, false );
	CDialog::OnClose();
}

void cLightDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
