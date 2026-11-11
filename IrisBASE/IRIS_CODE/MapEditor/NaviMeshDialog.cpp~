#include "stdafx.h"
#include "NaviMeshDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "NaviMeshBuildDialog.h"

IMPLEMENT_DYNAMIC(cNaviMeshDialog, CDialog)

BEGIN_MESSAGE_MAP(cNaviMeshDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cNaviMeshDialog::cNaviMeshDialog()
: mBuildDialog( 0 )
{
}

cNaviMeshDialog::~cNaviMeshDialog()
{
}

int cNaviMeshDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 네비메시 건축 페이지 추가
	mBuildDialog = new cNaviMeshBuildDialog;
	mBuildDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_BUILD), &mRollupWnd );
	mRollupWnd.InsertPage( "Build", mBuildDialog );

	mRollupWnd.ExpandPage( 0 );
	return 0;
}

void cNaviMeshDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_NAVIMESH, false );
	CDialog::OnClose();
}

void cNaviMeshDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
