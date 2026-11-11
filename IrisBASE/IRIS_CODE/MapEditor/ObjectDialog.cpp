#include "stdafx.h"
#include "ObjectDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"
#include "ObjectInfoDialog.h"

IMPLEMENT_DYNAMIC(cObjectDialog, CDialog)

BEGIN_MESSAGE_MAP(cObjectDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cObjectDialog::cObjectDialog()
: mTransformDialog( 0 )
, mPropertyDialog( 0 )
, mInfoDialog( 0 )
{
}

cObjectDialog::~cObjectDialog()
{
}

int cObjectDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 오브젝트 배치 페이지 추가
	mTransformDialog = new cObjectTransformDialog;
	mTransformDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_OBJECT_TRANSFORM), &mRollupWnd );
	mRollupWnd.InsertPage( "Transform", mTransformDialog );

	/// 오브젝트 속성 페이지 추가
	mPropertyDialog = new cObjectPropertyDialog;
	mPropertyDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_OBJECT_PROPERTY), &mRollupWnd );
	mRollupWnd.InsertPage( "Property", mPropertyDialog );

	/// 장면 정보 페이지 추가
	mInfoDialog = new cObjectInfoDialog;
	mInfoDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_OBJECT_INFO), &mRollupWnd );
	mRollupWnd.InsertPage( "Info", mInfoDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cObjectDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_OBJECT, false );
	CDialog::OnClose();
}

void cObjectDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}