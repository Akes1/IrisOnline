#include "stdafx.h"
#include "TerrainDialog.h"

#include "Resource.h"
#include "TabWindow.h"
#include "TerrainBuildDialog.h"
#include "TerrainPaintDialog.h"
#include "TerrainOptionDialog.h"
#include "TerrainInfoDialog.h"

IMPLEMENT_DYNAMIC(cTerrainBuildingDialog, CDialog)

BEGIN_MESSAGE_MAP(cTerrainBuildingDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cTerrainBuildingDialog::cTerrainBuildingDialog()
: mBuildDialog( 0 )
, mOptionDialog( 0 )
, mInfoDialog( 0 )
{
}

cTerrainBuildingDialog::~cTerrainBuildingDialog()
{
}

int cTerrainBuildingDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 지형 건축 페이지 추가
	mBuildDialog = new cTerrainBuildDialog;
	mBuildDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_BUILD), &mRollupWnd );
	mRollupWnd.InsertPage( "Build", mBuildDialog );

	/// 지형 옵션 페이지 추가
	mOptionDialog = new cTerrainOptionDialog;
	mOptionDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_OPTION), &mRollupWnd );
	mRollupWnd.InsertPage( "Option", mOptionDialog );

	/// 지형 정보 페이지 추가
	mInfoDialog = new cTerrainInfoDialog;
	mInfoDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_INFO), &mRollupWnd );
	mRollupWnd.InsertPage( "Info", mInfoDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cTerrainBuildingDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_TERRAIN_BUILDING, false );
	CDialog::OnClose();
}

void cTerrainBuildingDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}

IMPLEMENT_DYNAMIC(cTerrainPaintingDialog, CDialog)

BEGIN_MESSAGE_MAP(cTerrainPaintingDialog, CDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

cTerrainPaintingDialog::cTerrainPaintingDialog()
: mPaintDialog( 0 )
{
}

cTerrainPaintingDialog::~cTerrainPaintingDialog()
{
}

int cTerrainPaintingDialog::OnCreate( LPCREATESTRUCT cs )
{
	if( CDialog::OnCreate(cs) == -1 )
		return -1;

	/// 두루마리 윈도우 생성
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 400), this, 1 );

	/// 지형 색칠 페이지 추가
	mPaintDialog = new cTerrainPaintDialog;
	mPaintDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_TERRAIN_PAINT), &mRollupWnd );
	mRollupWnd.InsertPage( "Paint", mPaintDialog );

	mRollupWnd.ExpandAllPages();
	return 0;
}

void cTerrainPaintingDialog::OnClose()
{
	TABWIN->SetButtonChecked( IDC_BUTTON_TAB_TERRAIN_PAINTING, false );
	CDialog::OnClose();
}

void cTerrainPaintingDialog::OnSize( UINT type, int cx, int cy )
{
	CDialog::OnSize( type, cx, cy );
	mRollupWnd.Resize( type, cx, cy );
}
