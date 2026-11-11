#include "stdafx.h"
#include "MapEditorView.h"

#include "Resource.h"
#include "MapEditorApp.h"
#include "MainFrame.h"
#include "TabWindow.h"
#include "TerrainBuildDialog.h"
#include "TerrainPaintDialog.h"
#include "TerrainOptionDialog.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"
#include "LightTransformDialog.h"
#include "LightPropertyDialog.h"
#include "SoundTransformDialog.h"
#include "SoundPropertyDialog.h"
#include "AreaTransformDialog.h"
#include "NaviMeshBuildDialog.h"
#include "NaviFieldPaintDialog.h"
#include "SettingViewDialog.h"
#include "SettingControlDialog.h"

#include "TerrainEditor.h"
#include "ObjectEditor.h"
#include "LightEditor.h"
#include "SoundEditor.h"
#include "AreaEditor.h"
#include "EnvEditor.h"
#include "NaviMeshEditor.h"
#include "NaviFieldEditor.h"
#include "Hero.h"

#include "Doing/DoingManager.h"

#include "Engine/Application.h"
#include "Engine/AppTimer.h"
#include "Engine/RenderSystem.h"
#include "Engine/InputSystem.h"
#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/ResourceManager.h"
#include "Engine/WorldManager.h"
#include "Engine/CameraManager.h"
#include "Engine/FreeCamera.h"
#include "Engine/LookAtCamera.h"
#include "Engine/NaviMesh.h"

#include "Engine/StaticSceneNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(cMapEditorView, CView)

BEGIN_MESSAGE_MAP(cMapEditorView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

#ifdef _DEBUG
void cMapEditorView::AssertValid() const
{
	CView::AssertValid();
}

void cMapEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

cMapEditorView* cMapEditorView::mSingleton = 0;

cMapEditorView::cMapEditorView()
: mApplication( 0 )
, mDoingManager( 0 )
, mTerrainEditor( 0 )
, mObjectEditor( 0 )
, mLightEditor( 0 )
, mSoundEditor( 0 )
, mAreaEditor( 0 )
, mEnvEditor( 0 )
, mNaviMeshEditor( 0 )
, mNaviFieldEditor( 0 )
, mTerrainBuildDialog( 0 )
, mTerrainPaintDialog( 0 )
, mObjectTransformDialog( 0 )
, mObjectPropertyDialog( 0 )
, mLightTransformDialog( 0 )
, mLightPropertyDialog( 0 )
, mSoundTransformDialog( 0 )
, mSoundPropertyDialog( 0 )
, mAreaTransformDialog( 0 )
, mNaviMeshBuildDialog( 0 )
, mNaviFieldPaintDialog( 0 )
, mSettingViewDialog( 0 )
, mSettingControlDialog( 0 )
, mEditMode( eEDIT_NULL )
, mPlaying( false )
, mDefaultCamera( 0 )
, mFreeCamera( 0 )
, mLookAtCamera( 0 )
, mHero( 0 )
, mHeroLines( true )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	memset( mEditDialogs, 0, sizeof(CDialog*) * eEDIT_MAX_MODES );
	mCameraArray.Reserve( 12 );
}

cMapEditorView::~cMapEditorView()
{
	/// 어플리케이션을 제거
	delete mApplication;

	/// 엔진 종료
	NiShutdown();

	mSingleton = 0;
}

void cMapEditorView::Init()
{
	mObjectEditor->Init( 512 );
	mSoundEditor->Init();
	mTerrainEditor->Init( 512 );
	mNaviMeshEditor->Init( 512 );
	mNaviFieldEditor->Init( 512 );
}

void cMapEditorView::Update()
{
	/// 카메라 거리를 설정
	if( mLookAtCamera && mDefaultCamera )
		mLookAtCamera->SetFarDistance( mDefaultCamera->GetFarDistance() );

	Invalidate( FALSE );
	UpdateWindow();
}

bool cMapEditorView::SaveScreenShot( const cString& pathName, unsigned int width, unsigned int height, bool onlyTerrain )
{
	NiDX9Renderer* renderer = NiDX9Renderer::GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 && "null renderer" );
		return false;
	}

	/// 화면 크기 설정
	unsigned int w = RENDERSYS->GetScreenWidth();
	unsigned int h = RENDERSYS->GetScreenHeight();
	Resize( width, height );

	/// 처리
	if( mSettingViewDialog->IsLodChecked() )
		mTerrainEditor->SetLodEnabled( true );
	else
		mTerrainEditor->SetLodEnabled( false );

	mApplication->Process( mPlaying );
	mTerrainEditor->Process();

	/// 렌더링
	renderer->BeginFrame();
	renderer->BeginUsingDefaultRenderTargetGroup( NiRenderer::CLEAR_ALL );
	{
		cCamera* cam = CAMERAMAN->GetCurrent();
		renderer->SetCameraData( *cam );

		WORLDMAN->Render();

		if( mSettingViewDialog->IsTerrainChecked() )
			mTerrainEditor->Render( mSettingViewDialog->IsTerrainSelGroupChecked() );
		if( onlyTerrain == false && mSettingViewDialog->IsSceneChecked() )
			mObjectEditor->Render( mSettingViewDialog->IsSceneSelGroupChecked() );
		//if( mSettingViewDialog->IsLightChecked() )
		//	mLightEditor->Render();
		//if( mSettingViewDialog->IsSoundChecked() )
		//	mSoundEditor->Render();
		//if( mSettingViewDialog->IsAreaChecked() )
		//	mAreaEditor->Render();
	}
	renderer->EndUsingRenderTargetGroup();
	renderer->EndFrame();

	/// 백버퍼를 파일로 저장
	LPDIRECT3DDEVICE9 device = renderer->GetD3DDevice();
	assert( device );

	IDirect3DSurface9* buffer;
	if( device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &buffer ) != D3D_OK )
	{
		return false;
	}

	HRESULT ret = D3DXSaveSurfaceToFile( pathName.Cstr(), D3DXIFF_TGA, buffer, 0, 0 );
	buffer->Release();
	renderer->DisplayFrame();

	/// 화면 복구
	Resize( w, h );
	return ret == D3D_OK;
}

void cMapEditorView::Resize( unsigned int width, unsigned int height )
{
	if( RENDERSYS )
	{
		RENDERSYS->SetScreenSize( width, height );		
		RENDERSYS->Reset();

		/// 카메라 설정
		for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
		{
			cFreeCamera* cam = mCameraArray[i];
			cam->SetViewFrustum( width, height );
			cam->Update( 0.0f );
		}

		mLookAtCamera->SetViewFrustum( width, height );
		mLookAtCamera->Update( 0.0f );
	}
}

void cMapEditorView::SetTerrainBuildDialog( cTerrainBuildDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_TERRAIN_BUILD] = dlg;
	mTerrainBuildDialog = dlg;
	mTerrainEditor->SetBuildDialog( dlg );
}

void cMapEditorView::SetTerrainPaintDialog( cTerrainPaintDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_TERRAIN_PAINT] = dlg;
	mTerrainPaintDialog = dlg;
	mTerrainEditor->SetPaintDialog( dlg );
}

void cMapEditorView::SetTerrainOptionDialog( cTerrainOptionDialog* dlg )
{
	assert( dlg );

	mTerrainEditor->SetOptionDialog( dlg );
}

void cMapEditorView::SetTerrainInfoDialog( cTerrainInfoDialog* dlg )
{
	assert( dlg );

	mTerrainEditor->SetInfoDialog( dlg );
}

void cMapEditorView::SetObjectTransformDialog( cObjectTransformDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_OBJECT_TRANSFORM] = dlg;
	mObjectTransformDialog = dlg;
	mObjectEditor->SetTransformDialog( dlg );
}

void cMapEditorView::SetObjectPropertyDialog( cObjectPropertyDialog* dlg )
{
	assert( dlg );

	mObjectEditor->SetPropertyDialog( dlg );
}

void cMapEditorView::SetObjectInfoDialog( cObjectInfoDialog* dlg )
{
	assert( dlg );

	mObjectEditor->SetInfoDialog( dlg );
}

void cMapEditorView::SetLightTransformDialog( cLightTransformDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_LIGHT_TRANSFORM] = dlg;
	mLightTransformDialog = dlg;
	mLightEditor->SetTransformDialog( dlg );
}

void cMapEditorView::SetLightPropertyDialog( cLightPropertyDialog* dlg )
{
	assert( dlg );

	mLightEditor->SetPropertyDialog( dlg );
}

void cMapEditorView::SetLightInfoDialog( cLightInfoDialog* dlg )
{
	assert( dlg );

	mLightEditor->SetInfoDialog( dlg );
}

void cMapEditorView::SetSoundTransformDialog( cSoundTransformDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_SOUND_TRANSFORM] = dlg;
	mSoundTransformDialog = dlg;
	mSoundEditor->SetTransformDialog( dlg );
}

void cMapEditorView::SetSoundPropertyDialog( cSoundPropertyDialog* dlg )
{
	assert( dlg );

	mSoundEditor->SetPropertyDialog( dlg );
}

void cMapEditorView::SetSoundInfoDialog( cSoundInfoDialog* dlg )
{
	assert( dlg );

	mSoundEditor->SetInfoDialog( dlg );
}

void cMapEditorView::SetAreaTransformDialog( cAreaTransformDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_AREA_TRANSFORM] = dlg;
	mAreaTransformDialog = dlg;
	mAreaEditor->SetTransformDialog( dlg );
}

void cMapEditorView::SetAreaLightDialog( cAreaLightDialog* dlg )
{
	assert( dlg );

	mAreaEditor->SetLightDialog( dlg );
}

void cMapEditorView::SetAreaFogDialog( cAreaFogDialog* dlg )
{
	assert( dlg );

	mAreaEditor->SetFogDialog( dlg );
}

void cMapEditorView::SetAreaEtcDialog( cAreaEtcDialog* dlg )
{
	assert( dlg );

	mAreaEditor->SetEtcDialog( dlg );
}

void cMapEditorView::SetEnvCameraDialog( cEnvCameraDialog* dlg )
{
	assert( dlg );

	mEnvEditor->SetCameraDialog( dlg );
}

void cMapEditorView::SetEnvLevelDistanceDialog( cEnvLevelDistanceDialog* dlg )
{
	assert( dlg );

	mEnvEditor->SetLevelDistanceDialog( dlg );
}

void cMapEditorView::SetEnvSkyDialog( cEnvSkyDialog* dlg )
{
	assert( dlg );

	mEnvEditor->SetSkyDialog( dlg );
}

void cMapEditorView::SetEnvDirLightDialog( cEnvDirLightDialog* dlg )
{
	assert( dlg );

	mEnvEditor->SetDirLightDialog( dlg );
}

void cMapEditorView::SetNaviMeshBuildDialog( cNaviMeshBuildDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_NAVIMESH_BUILD] = dlg;
	mNaviMeshBuildDialog = dlg;
	mNaviMeshEditor->SetBuildDialog( dlg );
}

void cMapEditorView::SetNaviFieldPaintDialog( cNaviFieldPaintDialog* dlg )
{
	assert( dlg );

	mEditDialogs[eEDIT_NAVIFIELD_PAINT] = dlg;
	mNaviFieldPaintDialog = dlg;
	mNaviFieldEditor->SetPaintDialog( dlg );
}

void cMapEditorView::SetNaviFieldInfoDialog( cNaviFieldInfoDialog* dlg )
{
	assert( dlg );

	mNaviFieldEditor->SetInfoDialog( dlg );
}

void cMapEditorView::SetSettingViewDialog( cSettingViewDialog* dlg )
{
	assert( dlg );

	mSettingViewDialog = dlg;
}

void cMapEditorView::SetSettingControlDialog( cSettingControlDialog* dlg )
{
	assert( dlg );

	mSettingControlDialog = dlg;
}

void cMapEditorView::SetEditMode( eEditMode mode )
{
	if( mode != mEditMode )
	{
		for( int i = 0; i < eEDIT_MAX_MODES; ++i )
		{
			cDialog* dlg = mEditDialogs[i];

			if( i != mode )
			{
				dlg->SetCheckedButton( 0 );
			}
			else
			{
				if( dlg->GetCheckedButton() == 0 )
				{
					switch( mode )
					{
					case eEDIT_OBJECT_TRANSFORM:
						dlg->SetCheckedButton( IDC_BUTTON_OFORM_PICK );
						break;
					case eEDIT_LIGHT_TRANSFORM:
						dlg->SetCheckedButton( IDC_BUTTON_LFORM_PICK );
						break;
					case eEDIT_SOUND_TRANSFORM:
						dlg->SetCheckedButton( IDC_BUTTON_SFORM_PICK );
						break;
					}
				}
			}
		}
		mEditMode = mode;
		Update();
	}
}

void cMapEditorView::SelectCamera( unsigned int i, bool makeCurrent )
{
	if( i >= mCameraArray.GetSize() )
		return;

	mFreeCamera = mCameraArray[i];

	if( makeCurrent || mPlaying == false )
	{
		CAMERAMAN->SetCurrent( mFreeCamera );
		Update();
	}
}

void cMapEditorView::ZoomPersp()
{
	mTerrainEditor->ZoomPersp( mFreeCamera );
	Update();
}

void cMapEditorView::ZoomOrtho()
{
	mTerrainEditor->ZoomOrtho( mFreeCamera );
	Update();
}

unsigned int cMapEditorView::NewCamera()
{
	cFreeCamera* cam = CAMERAMAN->CreateFree( false );
	
	mTerrainEditor->ZoomPersp( cam );
	mCameraArray.PushBack( cam );
	return mCameraArray.GetSize();
}

void cMapEditorView::DeleteCamera()
{
	if( mCameraArray.GetSize() <= 1 )
		return;

	CAMERAMAN->Destroy( mCameraArray.Back() );
	mCameraArray.PopBack();
}

unsigned int cMapEditorView::LoadCamera( const cString& pathName )
{
	/// 파일을 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to open file to load camera" );
		return 0;
	}

	/// 카메라 수
	unsigned int numCams = 0;
	loader.ReadUnsignedInt( &numCams );

	if( numCams == 0 )
	{
		return 0;
	}

	for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
	{
		CAMERAMAN->Destroy( mCameraArray[i] );
	}
	mCameraArray.Clear();

	/// 카메라 배열 로딩
	for( unsigned int i = 0; i < numCams; ++i )
	{
		cFreeCamera* cam = CAMERAMAN->CreateFree( false );

		/// 이동
		NiPoint3 pos = NiPoint3::ZERO;
		loader.Read( &pos, sizeof(NiPoint3) );

		/// 회전
		NiMatrix3 rot = NiMatrix3::IDENTITY;
		loader.Read( &rot, sizeof(NiMatrix3) );

		/// 직교모드 여부
		unsigned int ortho = 0;
		loader.ReadUnsignedInt( &ortho );

		/// 원거리
		float farDist = 0.0f;
		loader.ReadFloat( &farDist );

		if( ortho == 0 )
		{
			mTerrainEditor->ZoomPersp( cam );

			cam->SetOrtho( false );
			cam->SetFarDistance( farDist );
			cam->SetTranslate( pos.z, pos.x, pos.y );
			cam->SetRotate( rot );
			cam->Update( 0.0f );
		}
		else
		{
			mTerrainEditor->ZoomOrtho( cam );
		}

		mCameraArray.PushBack( cam );
	}

	mDefaultCamera = mCameraArray[0];
	SelectCamera( 0, true );
	return numCams;
}

bool cMapEditorView::SaveCameraAs( const cString& pathName )
{
	if( mCameraArray.GetSize() <= 1 )
		return false;

	/// 파일을 열기
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		assert( 0 && "failed to open file to save camera" );
		return false;
	}

	/// 카메라 수
	unsigned int numCams = mCameraArray.GetSize();
	saver.WriteUnsignedInt( numCams );

	/// 카메라 배열 저장
	for( unsigned int i = 0; i < numCams; ++i )
	{
		mCameraArray[i]->Save( saver );
	}

	///
	cString fileName;
	GetFileName( &fileName, pathName );
	MAIN->SetCameraFileName( fileName );
	return true;
}

void cMapEditorView::ChangeToFreeCamera()
{
	mPlaying = false;
	CAMERAMAN->SetCurrent( mFreeCamera );
	mAreaEditor->SelectArea( 0, true );
	Update();
}

void cMapEditorView::ChangeToLookAtCamera()
{
	mPlaying = true;
	CAMERAMAN->SetCurrent( mLookAtCamera );
	Update();
}

void cMapEditorView::MoveFreeCameraToSelectObject( bool /*next*/ )
{
	NiPoint3 pos = NiPoint3::ZERO;
	cObjectGroup* group = OBJECTEDIT->GetObjectGroup();
	if( group == 0 )
		return;

	const cObjectGroup::cSelectedSet& selectedSet = group->GetSelectedSet();
	if( selectedSet.GetSize() == 0 )
		return;

	cObjectGroup::cSelectedSet::cConstIterator i = selectedSet.Begin();
	cObjectGroup::cSelectedSet::cConstIterator end = selectedSet.End();
	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;
		pos = n->GetCenter();
	}

	NiPoint3 curDir = mFreeCamera->GetWorldDirection() ;
	NiPoint3 viewDir = NiPoint3(10.0f, 0.0f, 10.0f);
	viewDir.Unitize();

	//vA와 vB의 사이각을 구한다.
	float fRadian = acosf( (curDir.x * viewDir.x) + (curDir.y * viewDir.y) + (curDir.z * viewDir.z) );

	//완전히 정반대벡터이면 외적 축을 구할수 없기때문에 보정한다.
	NiPoint3 viewDir2 = viewDir;
	if(D3DX_PI - fRadian < 0.0005f)
	{
		viewDir2.x += 0.0005f;
		viewDir2.y += 0.0005f;
		viewDir2.z += 0.0005f;
	}

	//vA와 vB에 직교하는 벡터를 구한다.(cross)
	curDir.UnitCross( viewDir2 );

	mFreeCamera->SetTranslate( pos + 5000.0f * -curDir );
/*
	if( next )
	{

	}
	else
	{

	}
*/
}

void cMapEditorView::MoveFreeCameraToHero()
{
	const NiPoint3& pos = HERO->GetCenter();

	mFreeCamera->SetTranslate( pos.x, pos.y, pos.z + 180.0f );
}

void cMapEditorView::SetDefaultCameraFarDistance( float dist )
{
	mDefaultCamera->SetFarDistance( dist );
}

void cMapEditorView::SetLookAtCameraFarDistance( float dist )
{
	mLookAtCamera->SetFarDistance( dist );
}

BOOL cMapEditorView::PreCreateWindow( CREATESTRUCT& cs ) 
{
	if( !CView::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(0, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), 0);

	return TRUE;
}

void cMapEditorView::OnDraw( CDC* )
{
	/// 처리
	if( mSettingViewDialog->IsLodChecked() )
		mTerrainEditor->SetLodEnabled( true );
	else
		mTerrainEditor->SetLodEnabled( false );

	mApplication->Process( mPlaying );
	mTerrainEditor->Process();
	mNaviMeshEditor->Process();
	mNaviFieldEditor->Process();

	/// 렌더링
	if( RENDERSYS )
	{
		RENDERSYS->BeginFrame( mPlaying );
		{
			//*
			WORLDMAN->RenderSkyDome();

			if( mSettingViewDialog->IsTerrainChecked() )
			{
				WORLDMAN->Render();
				mTerrainEditor->Render( mSettingViewDialog->IsTerrainSelGroupChecked() );
			}
			if( mSettingViewDialog->IsSceneChecked() )
				mObjectEditor->Render( mSettingViewDialog->IsSceneSelGroupChecked() );
			if( mSettingViewDialog->IsLightChecked() )
				mLightEditor->Render();
			if( mSettingViewDialog->IsSoundChecked() )
				mSoundEditor->Render();
			if( mSettingViewDialog->IsAreaChecked() )
				mAreaEditor->Render();
			if( mSettingViewDialog->IsNaviMeshChecked() )
				mNaviMeshEditor->Render();
			if( mSettingViewDialog->IsNaviFieldChecked() )
				mNaviFieldEditor->Render();
			//*/
			if( mPlaying && mHeroLines )
				HERO->RenderLines();
//			if( mSettingViewDialog->IsHUDChecked() )
//				DEVSYS->Render();
		}
		RENDERSYS->EndFrame();
		RENDERSYS->DisplayFrame();
	}
}

int cMapEditorView::OnCreate( LPCREATESTRUCT cs )
{
	if( CView::OnCreate(cs) == -1 )
		return -1;

	/// 엔진 초기화
	NiInit();

	/// 어플리케이션을 생성
	mApplication = new cApplication;

	/// 어플리케이션을 초기화
	if( mApplication->Init( GetSafeHwnd() ) == false )
	{
		assert( 0 && "failed to init application" );
		return -1;
	}

	/// 타이머 설정
	SetTimer( 1, 20, 0 );

	/// 실행 관리자를 생성
	mDoingManager = new cDoingManager;

	/// 편집자를 생성
	mObjectEditor = new cObjectEditor;
	mTerrainEditor = new cTerrainEditor;
	mLightEditor = new cLightEditor;
	mSoundEditor = new cSoundEditor;
	mAreaEditor = new cAreaEditor;
	mEnvEditor = new cEnvEditor;
	mNaviMeshEditor = new cNaviMeshEditor;
	mNaviFieldEditor = new cNaviFieldEditor;

	/// 카메라를 생성
	TABWIN->SetDefaultCameraFarDist( GAME_CAM_FAR_DIST_UNIT );
	mDefaultCamera = mFreeCamera = CAMERAMAN->CreateFree( true );
	mDefaultCamera->SetFarDistance( GAME_CAM_FAR_DIST_UNIT );
	mCameraArray.PushBack( mFreeCamera );

	mLookAtCamera = CAMERAMAN->CreateLookAt();
	mLookAtCamera->SetFarDistance( GAME_CAM_FAR_DIST_UNIT );

	/// 주인공을 생성
	mHero = new cHero( mLookAtCamera );
	
	if( mHero->Init() == false )
	{
		assert( 0 && "failed to init hero" );
		//return -1;
	}
	return 0;
}

void cMapEditorView::OnDestroy()
{
	delete mHero;
	delete mNaviFieldEditor;
	delete mNaviMeshEditor;
	delete mEnvEditor;
	delete mAreaEditor;
	delete mSoundEditor;
	delete mLightEditor;
	delete mTerrainEditor;
	delete mObjectEditor;
	delete mDoingManager;

	/// 어플리케이션 종료
	mApplication->Exit();

	CView::OnDestroy();
}

BOOL cMapEditorView::OnEraseBkgnd( CDC* ) 
{
	return FALSE;
}

void cMapEditorView::OnSize( UINT type, int cx, int cy )
{
	CView::OnSize(type, cx, cy);
	Resize( cx, cy );
}

void cMapEditorView::OnTimer( UINT_PTR /*nIDEvent*/ )
{
	if( mPlaying == false )
	{
		switch( mEditMode )
		{
		case eEDIT_TERRAIN_BUILD:
			mTerrainEditor->OnTimer_Build();
			break;
		case eEDIT_TERRAIN_PAINT:
			mTerrainEditor->OnTimer_Paint();
			break;
		case eEDIT_NAVIMESH_BUILD:
			mNaviMeshEditor->OnTimer_Build();
			break;
		case eEDIT_NAVIFIELD_PAINT:
			mNaviFieldEditor->OnTimer_Paint();
			break;
		}
	}
}

void cMapEditorView::OnEditCopy()
{
	switch( mEditMode )
	{
	case eEDIT_OBJECT_TRANSFORM:
		mObjectEditor->CopyGroup();
		break;
	case eEDIT_LIGHT_TRANSFORM:
		mLightEditor->CopyLight();
		break;
	case eEDIT_AREA_TRANSFORM:
		mAreaEditor->CopyArea();
		break;
	}
}

void cMapEditorView::OnEditDeselectAll()
{
	switch( mEditMode )
	{
	case eEDIT_OBJECT_TRANSFORM:
		mObjectEditor->DeselectAll();
		break;
	case eEDIT_LIGHT_TRANSFORM:
		mLightEditor->DeselectAll();
		break;
	case eEDIT_SOUND_TRANSFORM:
		mSoundEditor->DeselectAll();
		break;
	case eEDIT_AREA_TRANSFORM:
		mAreaEditor->DeselectAll();
		break;
	}
}

void cMapEditorView::OnEditChangeBackgroundColor()
{
	static NiColor color[6] =
	{
		NiColor::WHITE,
			NiColor( 1.0f, 0.0f, 0.0f ),
			NiColor( 0.0f, 1.0f, 0.0f ),
			NiColor( 0.0f, 0.0f, 1.0f ),
			NiColor( 1.0f, 1.0f, 0.0f ),
			NiColor::BLACK
	};
	static int i = 0;
	NiRenderer* renderer = NiRenderer::GetRenderer();

	if( ++i > 5 )
		i = 0;
	if( renderer )
	{
		renderer->SetBackgroundColor( color[i] );
		Update();
	}
}

void cMapEditorView::OnMouseMove( UINT /*flags*/, CPoint point )
{
	MOUSE->SetPosition( point.x, point.y );

	if( mPlaying == false )
	{
		switch( mEditMode )
		{
		case eEDIT_TERRAIN_BUILD:
			mTerrainEditor->OnMouseMove_Build( point );
			break;
		case eEDIT_TERRAIN_PAINT:
			mTerrainEditor->OnMouseMove_Paint( point );
			break;
		case eEDIT_OBJECT_TRANSFORM:
			mObjectEditor->OnMouseMove( point );
			break;
		case eEDIT_LIGHT_TRANSFORM:
			mLightEditor->OnMouseMove( point );
			break;
		case eEDIT_SOUND_TRANSFORM:
			mSoundEditor->OnMouseMove( point );
			break;
		case eEDIT_AREA_TRANSFORM:
			mAreaEditor->OnMouseMove( point );
			break;
		case eEDIT_NAVIMESH_BUILD:
			mNaviMeshEditor->OnMouseMove( point );
			break;
		case eEDIT_NAVIFIELD_PAINT:
			mNaviFieldEditor->OnMouseMove( point );
			break;
		}
	}
}

BOOL cMapEditorView::OnMouseWheel( UINT /*flags*/, short zdelta, CPoint /*point*/ )
{
	MOUSE->SetDeltaWheel( zdelta * (float)mSettingControlDialog->GetMouseWheel() );
	return TRUE;
}

void cMapEditorView::OnLButtonDown( UINT flags, CPoint point )
{
	MOUSE->SetLButtonDown( point.x, point.y );

	if( mPlaying == false )
	{
		bool ctrl = (flags & MK_CONTROL) != 0;

		switch( mEditMode )
		{
		case eEDIT_NULL:
			if( mObjectEditor->Pick( point, ctrl ) )
				SetEditMode( eEDIT_OBJECT_TRANSFORM );
			else if( mLightEditor->Pick( point ) )
				SetEditMode( eEDIT_LIGHT_TRANSFORM );
			else if( mSoundEditor->Pick( point ) )
				SetEditMode( eEDIT_SOUND_TRANSFORM );
			break;
		case eEDIT_TERRAIN_BUILD:
			mTerrainEditor->OnLButtonDown_Build( point );
			break;
		case eEDIT_TERRAIN_PAINT:
			mTerrainEditor->OnLButtonDown_Paint( point );
			break;
		case eEDIT_OBJECT_TRANSFORM:
			mObjectEditor->OnLButtonDown_Transform( point, ctrl );
			break;
		case eEDIT_LIGHT_TRANSFORM:
			mLightEditor->OnLButtonDown_Transform( point );
			break;
		case eEDIT_SOUND_TRANSFORM:
			mSoundEditor->OnLButtonDown_Transform( point );
			break;
		case eEDIT_AREA_TRANSFORM:
			mAreaEditor->OnLButtonDown_Transform( point );
			break;
		case eEDIT_NAVIMESH_BUILD:
			mNaviMeshEditor->OnLButtonDown_Build( point );
			break;
		case eEDIT_NAVIFIELD_PAINT:
			mNaviFieldEditor->OnLButtonDown_Paint( point );
			break;
		}
		OnTimer( 1 );
	}
	else
	{
		NiPoint3 pos;

		if( NAVIMESH->Pick( &pos, point.x, point.y ) )
		{
			if( KEY->IsDown( KEY_CONTROL ) )
				mHero->SetTranslate( pos );
			else
				mHero->SetTargetPos( pos );
		}
	}
}

void cMapEditorView::OnLButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetLButtonUp( point.x, point.y );
	mTerrainEditor->OnLButtonUp();
	mNaviMeshEditor->OnLButtonUp();
	mNaviFieldEditor->OnLButtonUp();
}

void cMapEditorView::OnRButtonDown( UINT flags, CPoint point )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}
	MOUSE->SetRButtonDown( point.x, point.y );

	if( mPlaying == false )
	{
		bool ctrl = (flags & MK_CONTROL) != 0;

		switch( mEditMode )
		{
		case eEDIT_OBJECT_TRANSFORM:
			mObjectEditor->OnRButtonDown_Transform( point, ctrl );
			break;
		case eEDIT_LIGHT_TRANSFORM:
			mLightEditor->OnRButtonDown_Transform( point, ctrl );
			break;
		case eEDIT_SOUND_TRANSFORM:
			mSoundEditor->OnRButtonDown_Transform( point, ctrl );
			break;
		case eEDIT_AREA_TRANSFORM:
			mAreaEditor->OnRButtonDown_Transform( point, ctrl );
			break;
		}
	}
}

void cMapEditorView::OnRButtonUp( UINT /*flags*/, CPoint point )
{
	if( GetCapture() == this )
	{
		ReleaseCapture();
	}
	MOUSE->SetRButtonUp( point.x, point.y );
}

void cMapEditorView::OnMButtonDown( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonDown( point.x, point.y );
}

void cMapEditorView::OnMButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonUp( point.x, point.y );
}

void cMapEditorView::OnKeyDown( UINT c, UINT /*repeats*/, UINT /*flags*/ )
{
	KEY->SetDown( (eKeyCode)c );

	switch( c )
	{
	case KEY_SPACE:
		if( mPlaying )
			ChangeToFreeCamera();
		else
			ChangeToLookAtCamera();
		return;
	case 0xDE:
		if( mPlaying == false )
			MoveFreeCameraToSelectObject( false );
		return;
/*
	case KEY_COMMA:
		if( mPlaying == false )
			MoveFreeCameraToSelectObject( false );
		return;
	case KEY_PERIOD:
		if( mPlaying == false )
			MoveFreeCameraToSelectObject( true );
		return;
*/
	case KEY_RETURN:
		if( mPlaying == false )
			MoveFreeCameraToHero();
		return;
	case KEY_F2:
		mSettingViewDialog->ToggleScene();
		return;
	case KEY_F3:
		mSettingViewDialog->ToggleTerrainSelGroup();
		return;
	case KEY_F1:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
		mTerrainEditor->OnKeyDown( c );
		return;
	case KEY_F11:
		mHeroLines = !mHeroLines;
		return;
	}

	if( mPlaying == false )
	{
		switch( mEditMode )
		{
		case eEDIT_TERRAIN_PAINT:
			mTerrainEditor->OnKeyDown( c );
			break;
		case eEDIT_OBJECT_TRANSFORM:
			mObjectEditor->OnKeyDown( c );
			break;
		case eEDIT_LIGHT_TRANSFORM:
			mLightEditor->OnKeyDown( c );
			break;
		case eEDIT_SOUND_TRANSFORM:
			mSoundEditor->OnKeyDown( c );
			break;
		case eEDIT_AREA_TRANSFORM:
			mAreaEditor->OnKeyDown( c );
			break;
		}
	}
}

void cMapEditorView::OnKeyUp( UINT c, UINT /*repeats*/, UINT /*flags*/ )
{
	KEY->SetUp( (eKeyCode)c );
}
