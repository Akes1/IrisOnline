#include "stdafx.h"
#include "EditWindow.h"

#include "Resource.h"
#include "CameraDialog.h"
#include "MapDialog.h"
#include "NpcDialog.h"
#include "MonsterDialog.h"
#include "PortalDialog.h"
#include "GatheringDialog.h"
#include "PatternDialog.h"
#include "DoorDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(cEditWindow, CFormView)

BEGIN_MESSAGE_MAP(cEditWindow, CFormView)
	//{{AFX_MSG_MAP(cEditWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

cEditWindow::cEditWindow()
: CFormView( IDD_FORMVIEW_EDIT )
{
	mCameraDialog = 0;
	mMapDialog = 0;
	mNpcDialog = 0;
	mMonsterDialog = 0;
	mPortalDialog = 0;
	mGatheringDialog = 0;
	mPatternDialog = 0;
	mDoorDialog = 0;
}

cEditWindow::~cEditWindow()
{
}

BOOL cEditWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

void cEditWindow::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
}

void cEditWindow::OnDraw(CDC* /*dc*/)
{
}

#ifdef _DEBUG
void cEditWindow::AssertValid() const
{
	CFormView::AssertValid();
}

void cEditWindow::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


int cEditWindow::OnCreate(LPCREATESTRUCT cs)
{
	if( CFormView::OnCreate(cs) == -1 )
		return -1;

	/// 滴风付府 扩档快甫 积己
	mRollupWnd.Create( WS_VISIBLE | WS_CHILD, CRect(4, 4, 220, 600), this, 1 );
	mRollupWnd.ShowWindow(1);

	/// Camera
	mCameraDialog = new cCameraDialog;
	mCameraDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_CAMERA), &mRollupWnd );
	mRollupWnd.InsertPage( "Camera", mCameraDialog );

	/// Map
	mMapDialog = new cMapDialog;
	mMapDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_MAP), &mRollupWnd );
	mRollupWnd.InsertPage( "Map", mMapDialog );

	/// Npc
	mNpcDialog = new cNpcDialog;
	mNpcDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_NPC), &mRollupWnd );
	mRollupWnd.InsertPage( "Npc", mNpcDialog );

	/// Monster
	mMonsterDialog = new cMonsterDialog;
	mMonsterDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_MONSTER), &mRollupWnd );
	mRollupWnd.InsertPage( "Monster", mMonsterDialog );

	/// Portal
	mPortalDialog = new cPortalDialog;
	mPortalDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_PORTAL), &mRollupWnd );
	mRollupWnd.InsertPage( "Portal", mPortalDialog );

	/// Gathering
	mGatheringDialog = new cGatheringDialog;
	mGatheringDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_GATHERING), &mRollupWnd );
	mRollupWnd.InsertPage( "Gathering", mGatheringDialog );

	/// Pattern
	mPatternDialog = new cPatternDialog;
	mPatternDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_PATTERN), &mRollupWnd );
	mRollupWnd.InsertPage( "Pattern", mPatternDialog );


	mDoorDialog = new cDoorDialog;
	mDoorDialog->Create( MAKEINTRESOURCE(IDD_DIALOG_DOOR), &mRollupWnd );
	mRollupWnd.InsertPage( "Door", mDoorDialog );

	///
	mRollupWnd.ExpandPage( 0 );
	mRollupWnd.ExpandPage( 1 );
	return 0;
}

void cEditWindow::OnSize( UINT type, int cx, int cy )
{
	CFormView::OnSize( type, cx, cy );

	mRollupWnd.Resize( type, cx, cy );
}
