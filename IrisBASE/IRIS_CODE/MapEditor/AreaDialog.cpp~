#include "stdafx.h"
#include "AreaDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "AreaTransformDialog.h"
#include "AreaLightDialog.h"
#include "AreaFogDialog.h"
#include "AreaEtcDialog.h"

IMPLEMENT_DYNAMIC(cAreaDialog, CDialog)

BEGIN_MESSAGE_MAP(cAreaDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cAreaDialog::cAreaDialog()
: mTransformDialog( 0 )
, mLightDialog( 0 )
, mFogDialog( 0 )
, mEtcDialog( 0 )
{
}

cAreaDialog::~cAreaDialog()
{
}

int cAreaDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우를 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 영역 배치 페이지를 추가
	mTransformDialog = new cAreaTransformDialog;
	mTransformDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_LIGHT_TRANSFORM), &mRollupWnd );
	mRollupWnd.InsertPage( "Transform", mTransformDialog );

	/// 주변광 페이지를 추가
	mLightDialog = new cAreaLightDialog;
	mLightDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_AREA_LIGHT), &mRollupWnd );
	mRollupWnd.InsertPage( "Ambient Light", mLightDialog );

	/// 안개 페이지를 추가
	mFogDialog = new cAreaFogDialog;
	mFogDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_AREA_FOG), &mRollupWnd );
	mRollupWnd.InsertPage( "Fog", mFogDialog );

	/// 기타 속성 페이지를 추가
	mEtcDialog = new cAreaEtcDialog;
	mEtcDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_AREA_ETC), &mRollupWnd );
	mRollupWnd.InsertPage( "Etc", mEtcDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cAreaDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_AREA, false );
	CDialog::OnClose();
}

void cAreaDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
