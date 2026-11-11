#include "stdafx.h"
#include "MainFrame.h"

#include "Resource.h"
#include "RegenToolApp.h"
#include "RegenToolView.h"
#include "EditWindow.h"
//
//#include "TerrainDialog.h"
//#include "SettingDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(cMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(cMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//ON_COMMAND(ID_FILE_SAVE_ALL, OnSaveAll)
	//ON_COMMAND(ID_TERRAIN_OPEN, OnTerrainOpen)
	//ON_COMMAND(ID_ESCAPE, OnEscape)
END_MESSAGE_MAP()

#ifdef _DEBUG
void cMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void cMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

cMainFrame* cMainFrame::mSingleton = 0;

cMainFrame::cMainFrame()
//: mView( 0 )
{
	NiInit();

	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cMainFrame::~cMainFrame()
{
	/// 엔진 종료
	NiShutdown();

	mSingleton = 0;
}

/*
void cMainFrame::LoadTerrain( cString pathName )
{
	/// 지형이 변경된 경우 저장할 지를 결정
	if( mTerrainModified )
	{
		int mb = MessageBox(
			"The Terrain has been modified.\n"
			"Do you want to save your changes?",
			"Terrain",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnTerrainSave();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load Terrain", "*.terrain",
			OFN_HIDEREADONLY,
			"Terrain Files (*.terrain)| *.terrain| All Files (*.*)| *.*|" );

		if( dlg.DoModal() == IDOK )
		{
			pathName = (LPCTSTR)dlg.GetPathName();
		}
	}

	if( pathName.IsEmpty() )
	{
		return;
	}
	else
	{
		TABWIN->SelectCamera( 0 );

		if( TERRAINEDIT->Load( pathName ) == false )
		{
			MessageBox( "Failed to Load Terrain.\n""It will be reinitialized.", "Terrain" );
			TERRAINEDIT->Init( 512 );
			NAVIMESHEDIT->Init( 512 );
			NAVIFIELDEDIT->Init( 512 );
		}
	}

	/// 지형과 네비메시의 크기가 다르면 네비메시와 네비필드를 로딩
	if( TERRAINEDIT->GetResolution() != NAVIMESHEDIT->GetResolution() ||
		TERRAINEDIT->GetMetersPerVertex() != NAVIMESHEDIT->GetMetersPerVertex() )
	{
		pathName.Replace( ".terrain", ".navimesh" );
		LoadNaviMesh( pathName.Cstr() );

		pathName.Replace( ".navimesh", ".navifield" );
		LoadNaviField( pathName.Cstr() );
	}
}
*/

BOOL cMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

BOOL cMainFrame::OnCreateClient( LPCREATESTRUCT /*cs*/, CCreateContext* context )
{
	mSplitter.CreateStatic( this, 1, 2 );

	/// 왼쪽
	mSplitter.CreateView( 0, 0, RUNTIME_CLASS(cEditWindow), CSize(240, 600), context );

	/// 오른쪽
	mSplitter.CreateView( 0, 1, RUNTIME_CLASS(cRegenToolView), CSize(800, 600), context );

	/// 탭 윈도우와 뷰를 얻어옴
	cEditWindow* edit = (cEditWindow*)mSplitter.GetPane( 0, 0 );
	cRegenToolView* view = (cRegenToolView*)mSplitter.GetPane( 0, 1 );

	if( view )
	{
		/// 뷰에 필요한 다이얼로그들을 설정
		view->SetMapDialog( edit->GetMapDialog() );
		view->SetNpcDialog( edit->GetNpcDialog() );
		view->SetMonsterDialog( edit->GetMonsterDialog() );
		view->SetPortalDialog( edit->GetPortalDialog() );
		view->SetGatheringDialog( edit->GetGatheringDialog() );
		view->SetPatternDialog(  edit->GetPatternDialog() );
		view->SetDoorDialog( edit->GetDoorDialog() );

		/// 뷰를 초기화
		view->Init();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int cMainFrame::OnCreate( LPCREATESTRUCT cs )
{
	if( CFrameWnd::OnCreate(cs) == -1 )
		return -1;

	//mTerrainName = "?.terrain";
	//mTerrainModified = false;

	//SetWindowText( "Iris RegenTool - ?.terrain, ?.scene, ?.navimesh, ?.navifield" );
	return 0;
}

void cMainFrame::OnClose()
{
	/// 지형이 변경됐는지를 검사
	//if( mTerrainModified )
	//{
	//	int mb = MessageBox(
	//		"The Terrain has been modified.\n"
	//		"Do you want to save your changes?",
	//		"Terrain",
	//		MB_ICONQUESTION | MB_YESNOCANCEL );

	//	if( mb == IDYES )
	//		OnTerrainSave();
	//	else if( mb == IDCANCEL )
	//		return;
	//}

	CFrameWnd::OnClose();
}

void cMainFrame::OnSaveAll()
{
	//OnTerrainSave();
}
//
//void cMainFrame::OnEscape()
//{
//	if( VIEW )
//		VIEW->SetEditMode( eEDIT_NULL );
//	if( TABWIN )
//		TABWIN->OnEscape();
//}
