#include "stdafx.h"
#include "NaviFieldDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "NaviFieldBuildDialog.h"
#include "NaviFieldPaintDialog.h"
#include "NaviFieldInfoDialog.h"

IMPLEMENT_DYNAMIC(cNaviFieldDialog, CDialog)

BEGIN_MESSAGE_MAP(cNaviFieldDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cNaviFieldDialog::cNaviFieldDialog()
: mBuildDialog( 0 )
, mPaintDialog( 0 )
, mInfoDialog( 0 )
{
}

cNaviFieldDialog::~cNaviFieldDialog()
{
}

int cNaviFieldDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 네비필드 건축 페이지 추가
	mBuildDialog = new cNaviFieldBuildDialog;
	mBuildDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_NAVIFIELD_BUILD), &mRollupWnd );
	mRollupWnd.InsertPage( "Build", mBuildDialog );

	/// 네비필드 색칠 페이지 추가
	mPaintDialog = new cNaviFieldPaintDialog;
	mPaintDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_NAVIFIELD_PAINT), &mRollupWnd );
	mRollupWnd.InsertPage( "Paint", mPaintDialog );

	/// 네비필드 정보 페이지 추가
	mInfoDialog = new cNaviFieldInfoDialog;
	mInfoDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_INFO), &mRollupWnd );
	mRollupWnd.InsertPage( "Info", mInfoDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cNaviFieldDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_NAVIFIELD, false );
	CDialog::OnClose();
}

void cNaviFieldDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
