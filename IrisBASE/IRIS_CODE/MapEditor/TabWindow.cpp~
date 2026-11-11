#include "stdafx.h"
#include "TabWindow.h"

#include "TerrainDialog.h"
#include "ObjectDialog.h"
#include "LightDialog.h"
#include "SoundDialog.h"
#include "AreaDialog.h"
#include "EnvDialog.h"
#include "NaviMeshDialog.h"
#include "NaviFieldDialog.h"
#include "SettingDialog.h"

#include "MapEditorView.h"

#include "Engine/FreeCamera.h"

IMPLEMENT_DYNCREATE(cTabWindow, CFormView)

BEGIN_MESSAGE_MAP(cTabWindow, CFormView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_PERSP, OnClickedZoomPersp)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_ORTHO, OnClickedZoomOrtho)
	ON_BN_CLICKED(IDC_BUTTON_NEW_CAMERA, OnClickedNewCamera)
	ON_BN_CLICKED(IDC_BUTTON_DEL_CAMERA, OnClickedDeleteCamera)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CAMERA, OnClickedLoadCamera)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CAMERA, OnClickedSaveCamera)
	ON_LBN_SELCHANGE(IDC_LIST_CAMERA, OnSelchangeCamera)
	ON_REGISTERED_MESSAGE(NEN_CHANGED, OnChangedFarDist)
END_MESSAGE_MAP()

cTabWindow* cTabWindow::mSingleton = 0;

cTabWindow::cTabWindow()
: CFormView( IDD_FORMVIEW_TAB )
, mTerrainBuildingDialog( 0 )
, mTerrainPaintingDialog( 0 )
, mObjectDialog( 0 )
, mLightDialog( 0 )
, mSoundDialog( 0 )
, mAreaDialog( 0 )
, mEnvDialog( 0 )
, mNaviMeshDialog( 0 )
, mNaviFieldDialog( 0 )
, mSettingDialog( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cTabWindow::~cTabWindow()
{
	/// 오브젝트 다이얼로그를 제거
	delete mTerrainBuildingDialog;
	delete mTerrainPaintingDialog;
	delete mObjectDialog;
	delete mLightDialog;
	delete mSoundDialog;
	delete mAreaDialog;
	delete mEnvDialog;
	delete mNaviMeshDialog;
	delete mNaviFieldDialog;
	delete mSettingDialog;

	mSingleton = 0;
}

void cTabWindow::LoadCamera( const cString& pathName )
{
	unsigned int numCams = VIEW->LoadCamera( pathName );

	if( numCams )
	{
		CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );
		list->ResetContent();
		cString camName;

		for( unsigned int i = 0; i < numCams; ++i )
		{
			camName.Format( "Cam %02d",  + i );

			list->AddString( camName.Cstr() );
		}
	}
}

void cTabWindow::SelectCamera( unsigned int i )
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );
	list->SetCurSel( i );

	OnSelchangeCamera();
}

void cTabWindow::SetDefaultCameraFarDist( float farDist )
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );

	int i = list->GetCurSel();
	if( i == 0 )
	{
		mFarDist.SetValue( farDist / 100.0f );
	}
}

void cTabWindow::SetButtonChecked( int id, bool checked )
{
	switch( id )
	{
	case IDC_BUTTON_TAB_TERRAIN_BUILDING:
		mTerrainBuildingButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_TERRAIN_PAINTING:
		mTerrainPaintingButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_OBJECT:
		mObjectButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_LIGHT:
		mLightButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_SOUND:
		mSoundButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_AREA:
		mAreaButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_ENV:
		mEnvButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_NAVIMESH:
		mNaviMeshButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_NAVIFIELD:
		mNaviFieldButton.SetChecked( checked );
		break;
	case IDC_BUTTON_TAB_SETTING:
		mSettingButton.SetChecked( checked );
		break;
	default:
		assert( 0 && "invalid button id" );
		break;
	}
}

void cTabWindow::DoDataExchange(CDataExchange* dx)
{
	CFormView::DoDataExchange(dx);

	DDX_Control( dx, IDC_BUTTON_TAB_TERRAIN_BUILDING, mTerrainBuildingButton );
	DDX_Control( dx, IDC_BUTTON_TAB_TERRAIN_PAINTING, mTerrainPaintingButton );
	DDX_Control( dx, IDC_BUTTON_TAB_OBJECT, mObjectButton );
	DDX_Control( dx, IDC_BUTTON_TAB_LIGHT, mLightButton );
	DDX_Control( dx, IDC_BUTTON_TAB_SOUND, mSoundButton );
	DDX_Control( dx, IDC_BUTTON_TAB_AREA, mAreaButton );
	DDX_Control( dx, IDC_BUTTON_TAB_ENV, mEnvButton );
	DDX_Control( dx, IDC_BUTTON_TAB_NAVIMESH, mNaviMeshButton );
	DDX_Control( dx, IDC_BUTTON_TAB_NAVIFIELD, mNaviFieldButton );
	DDX_Control( dx, IDC_BUTTON_TAB_SETTING, mSettingButton );
	DDX_Control( dx, IDC_BUTTON_ZOOM_PERSP, mZoomPerspButton );
	DDX_Control( dx, IDC_BUTTON_ZOOM_ORTHO, mZoomOrthoButton );
	DDX_Control( dx, IDC_BUTTON_NEW_CAMERA, mNewCameraButton );
	DDX_Control( dx, IDC_BUTTON_DEL_CAMERA, mDelCameraButton );
	DDX_Control( dx, IDC_BUTTON_LOAD_CAMERA, mLoadCameraButton );
	DDX_Control( dx, IDC_BUTTON_SAVE_CAMERA, mSaveCameraButton );
}

int cTabWindow::OnCreate( LPCREATESTRUCT cs )
{
	if( CFormView::OnCreate( cs ) == -1 )
		return -1;

	/// 지형 다이얼로그를 띄움
	mTerrainBuildingDialog = new cTerrainBuildingDialog;
	mTerrainBuildingDialog->Create( IDD_DIALOG_TERRAIN_BUILDING, this );

	mTerrainPaintingDialog = new cTerrainPaintingDialog;
	mTerrainPaintingDialog->Create( IDD_DIALOG_TERRAIN_PAINTING, this );

	/// 오브젝트 다이얼로그를 띄움
	mObjectDialog = new cObjectDialog;
	mObjectDialog->Create( IDD_DIALOG_OBJECT, this );

	/// 조명 다이얼로그를 띄움
	mLightDialog = new cLightDialog;
	mLightDialog->Create( IDD_DIALOG_LIGHT, this );

	/// 사운드 다이얼로그를 띄움
	mSoundDialog = new cSoundDialog;
	mSoundDialog->Create( IDD_DIALOG_SOUND, this );

	/// 영역 다이얼로그를 띄움
	mAreaDialog = new cAreaDialog;
	mAreaDialog->Create( IDD_DIALOG_AREA, this );

	/// 환경 다이얼로그를 띄움
	mEnvDialog = new cEnvDialog;
	mEnvDialog->Create( IDD_DIALOG_ENV, this );

	/// 네비메시 다이얼로그를 띄움
	mNaviMeshDialog = new cNaviMeshDialog;
	mNaviMeshDialog->Create( IDD_DIALOG_NAVIMESH, this );

	/// 네비필드 다이얼로그를 띄움
	mNaviFieldDialog = new cNaviFieldDialog;
	mNaviFieldDialog->Create( IDD_DIALOG_NAVIFIELD, this );

	/// 설정 다이얼로그를 띄움
	mSettingDialog = new cSettingDialog;
	mSettingDialog->Create( IDD_DIALOG_SETTING, this );

	/// 탭 버튼들에 윈도우를 연결
	mTerrainBuildingButton.SetWindow( mTerrainBuildingDialog );
	mTerrainPaintingButton.SetWindow( mTerrainPaintingDialog );
	mObjectButton.SetWindow( mObjectDialog );
	mLightButton.SetWindow( mLightDialog );
	mSoundButton.SetWindow( mSoundDialog );
	mAreaButton.SetWindow( mAreaDialog );
	mEnvButton.SetWindow( mEnvDialog );
	mNaviMeshButton.SetWindow( mNaviMeshDialog );
	mNaviFieldButton.SetWindow( mNaviFieldDialog );
	mSettingButton.SetWindow( mSettingDialog );
	return 0;
}

void cTabWindow::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	static bool first = true;

	if( first )
	{
		first = false;

		CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );
		list->AddString( "Cam 00 (Default)" );
		list->SetCurSel( 0 );

		mZoomOrthoButton.EnableWindow( 0 );

		mFarDist.Create( 10.0f, 2000.0f, 500.0f, 1.0f, this, IDC_EDIT_CAMERA_FARDIST );
		mFarDist.SetEnabled( false );
	}
}

void cTabWindow::OnDestroy()
{
	/// 오브젝트 다이얼로그 제거
	mTerrainBuildingDialog->DestroyWindow();
	mTerrainPaintingDialog->DestroyWindow();
	mObjectDialog->DestroyWindow();
	mLightDialog->DestroyWindow();
	mSoundDialog->DestroyWindow();
	mAreaDialog->DestroyWindow();
	mEnvDialog->DestroyWindow();
	mNaviMeshDialog->DestroyWindow();
	mNaviFieldDialog->DestroyWindow();
	mSettingDialog->DestroyWindow();
}

void cTabWindow::OnEscape()
{
	mTerrainBuildingButton.SetChecked( false );
	mTerrainPaintingButton.SetChecked( false );
	mObjectButton.SetChecked( false );
	mLightButton.SetChecked( false );
	mSoundButton.SetChecked( false );
	mAreaButton.SetChecked( false );
	mEnvButton.SetChecked( false );
	mNaviMeshButton.SetChecked( false );
	mNaviFieldButton.SetChecked( false );
	mSettingButton.SetChecked( false );
}

void cTabWindow::OnSelchangeCamera()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );

	int i = list->GetCurSel();
	if( i == LB_ERR )
		i = 0;

	if( i == 0 )
	{
		mZoomOrthoButton.EnableWindow( 0 );
		mFarDist.SetEnabled( false );
	}
	else
	{
		mZoomOrthoButton.EnableWindow( 1 );
		mFarDist.SetEnabled( true );
	}

	VIEW->SelectCamera( i );
	mFarDist.SetValue( VIEW->GetFreeCamera()->GetFarDistance() / 100.0f );
}

void cTabWindow::OnClickedZoomPersp()
{
	VIEW->ZoomPersp();
}

void cTabWindow::OnClickedZoomOrtho()
{
	VIEW->ZoomOrtho();
	mFarDist.SetValue( VIEW->GetFreeCamera()->GetFarDistance() / 100.0f );
}

void cTabWindow::OnClickedNewCamera()
{
	unsigned int numCams = VIEW->NewCamera();
	cString camName;
	camName.Format( "Cam %02d", numCams - 1 );

	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );
	list->AddString( camName.Cstr() );
}

void cTabWindow::OnClickedDeleteCamera()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );
	
	if( list->GetCount() <= 1 )
		return;

	if( list->GetCurSel() == list->GetCount() - 1 )
		SelectCamera( 0 );

	VIEW->DeleteCamera();
	list->DeleteString( list->GetCount() - 1 );
}

void cTabWindow::OnClickedLoadCamera()
{
	CFileDialog dlg(
		TRUE, "Load Camera", "*.cam",
		OFN_HIDEREADONLY,
		"Camera Files (*.cam)| *.cam| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		LoadCamera( (LPCTSTR)dlg.GetPathName() );
	}
}

void cTabWindow::OnClickedSaveCamera()
{
	CListBox* list = (CListBox*)GetDlgItem( IDC_LIST_CAMERA );

	if( list->GetCount() <= 1 )
		return;

	CFileDialog dlg(
		FALSE, "Save Camera", "*.cam",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Camera Files (*.cam)| *.cam| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( VIEW->SaveCameraAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Camera", "Camera" );
		}
	}
}

LRESULT cTabWindow::OnChangedFarDist( WPARAM /*id*/, LPARAM )
{
	float farDist = mFarDist.GetValue() * 100.0f;
	VIEW->GetFreeCamera()->SetFarDistance( farDist );
	VIEW->Update();
	return 0;
}
