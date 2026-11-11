#include "stdafx.h"
#include "MainFrame.h"

#include "Resource.h"
#include "MapEditorApp.h"
#include "MapEditorView.h"
#include "TabWindow.h"
#include "ExplorerWindow.h"

#include "TerrainDialog.h"
#include "TerrainInitDialog.h"
#include "TerrainScaleZDialog.h"
#include "TerrainPaintDialog.h"
#include "ObjectDialog.h"
#include "ObjectScaleDialog.h"
#include "LightDialog.h"
#include "SoundDialog.h"
#include "NaviMeshDialog.h"
#include "NaviFieldDialog.h"
#include "AreaDialog.h"
#include "AreaFogDialog.h"
#include "EnvDialog.h"
#include "EnvCameraDialog.h"
#include "SettingDialog.h"
#include "LogDialog.h"

#include "TerrainEditor.h"
#include "ObjectEditor.h"
#include "NaviMeshEditor.h"
#include "NaviFieldEditor.h"
#include "SoundEditor.h"

#include "Doing/DoingManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(cMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(cMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_LOAD_PROJECT, OnLoadProject)
	ON_COMMAND(ID_FILE_SAVE_PROJECT, OnSaveProject)
	ON_COMMAND(ID_FILE_SAVE_SCREEN_SHOT, OnSaveScreenShot)
	ON_COMMAND(ID_FILE_SAVE_LOG, OnSaveLog)
	ON_COMMAND(ID_TERRAIN_NEW, OnTerrainNew)
	ON_COMMAND(ID_TERRAIN_OPEN, OnTerrainOpen)
	ON_COMMAND(ID_TERRAIN_SAVE, OnTerrainSave)
	ON_COMMAND(ID_TERRAIN_SAVEAS, OnTerrainSaveAs)
	ON_COMMAND(ID_TERRAIN_CLEAR_HEIGHTMAP, OnTerrainClearHeightMap)
	ON_COMMAND(ID_TERRAIN_IMPORT_HEIGHTMAP, OnTerrainImportHeightMap)
	ON_COMMAND(ID_TERRAIN_EXPORT_HEIGHTMAP, OnTerrainExportHeightMap)
	ON_COMMAND(ID_TERRAIN_CLEAR_ALPHAMAP, OnTerrainClearAlphaMap)
	ON_COMMAND(ID_TERRAIN_IMPORT_ALPHAMAP, OnTerrainImportAlphaMap)
	ON_COMMAND(ID_TERRAIN_EXPORT_ALPHAMAP, OnTerrainExportAlphaMap)
	ON_COMMAND(ID_TERRAIN_CLEAR_COLORMAP, OnTerrainClearColorMap)
	ON_COMMAND(ID_TERRAIN_IMPORT_COLORMAP, OnTerrainImportColorMap)
	ON_COMMAND(ID_TERRAIN_EXPORT_COLORMAP, OnTerrainExportColorMap)
	ON_COMMAND(ID_TERRAIN_ADJUST_HEIGHT, OnTerrainAdjustHeight)
	ON_COMMAND(ID_SCENE_NEW, OnSceneNew)
	ON_COMMAND(ID_SCENE_OPEN, OnSceneOpen)
	ON_COMMAND(ID_SCENE_ADD, OnSceneAdd)
	ON_COMMAND(ID_SCENE_SAVE, OnSceneSave)
	ON_COMMAND(ID_SCENE_SAVE_AS, OnSceneSaveAs)
	ON_COMMAND(ID_SOUND_NEW, OnSoundNew)
	ON_COMMAND(ID_SOUND_OPEN, OnSoundOpen)
	ON_COMMAND(ID_SOUND_SAVE, OnSoundSave)
	ON_COMMAND(ID_SOUND_SAVEAS, OnSoundSaveAs)
	ON_COMMAND(ID_SCENE_SAVE_AREA_AS, OnSceneSaveAreaAs)
	ON_COMMAND(ID_SCENE_SAVE_OBJECT_GROUP_AS, OnSceneSaveObjectGroupAs)
	ON_COMMAND(ID_SCENE_LOAD_AREA_GROUP, OnSceneLoadAreaGroup)
	ON_COMMAND(ID_SCENE_LOAD_OBJECT_TRANSFORM_SCRIPT, OnSceneLoadObjectTransformScript)
	ON_COMMAND(ID_SCENE_ADJUST_ALL_POS, OnSceneAdjustAllPos)
	ON_COMMAND(ID_SCENE_ADJUST_ALL_SIZE, OnSceneAdjustAllSize)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_DESELECT_ALL, OnEditDeselectAll)
	ON_COMMAND(ID_EDIT_CHANGE_BGCOLOR, OnEditChangeBackgroundColor)
	ON_COMMAND(ID_ESCAPE, OnEscape)
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
: mView( 0 )
, mTerrainModified( false )
, mSceneModified( false )
, mSoundModified( false )
, mNaviMeshModified( false )
, mNaviFieldModified( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cMainFrame::~cMainFrame()
{
	mSingleton = 0;
}

void cMainFrame::LoadProject( cString pathName )
{
	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load Project", "*.mapproj",
			OFN_HIDEREADONLY,
			"Project Files (*.mapproj)| *.mapproj| All Files (*.*)| *.*|" );

		if( dlg.DoModal() == IDOK )
		{
			pathName = (LPCTSTR)dlg.GetPathName();
		}
	}
	if( pathName.IsEmpty() )
	{
		return;
	}

	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		AfxMessageBox( "failed to open project file" );
		return;
	}

	/// 경로 및 파일 이름을 추출
	cString path, fileName;
	::GetFilePath( &path, pathName );
	::GetFileName( &fileName, pathName );

	/// 파싱
	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n\"", pathName.Cstr() );
	cString str;

	/// 식별 문자열
	tok.GetNext( &str );

	if( str != "IrisMapProject" )
	{
		AfxMessageBox( "invalid project file" );
		return;
	}

	/// 버전
	tok.GetNext( &str );
	tok.GetNext( &str );
	
	int version = str.ToInt();
	switch( version )
	{
	case 1:
		break;
	case 2:
		break;
	default:
		AfxMessageBox( "invalid version" );
		return;
	}

	/// 지형
	tok.GetNext( &str );

	if( str != "terrain" )
		return;
	{
		tok.GetNext( &str );
		pathName = path;
		pathName += str;
		LoadTerrain( pathName, false );
	}

	/// 네비 메시
	tok.GetNext( &str );

	if( str != "navimesh" )
		return;
	{
		tok.GetNext( &str );
		pathName = path;
		pathName += str;
		LoadNaviMesh( pathName );
	}

	/// 네비 필드
	tok.GetNext( &str );

	if( str != "navifield" )
		return;
	{
		tok.GetNext( &str );
		pathName = path;
		pathName += str;
		LoadNaviField( pathName );
	}

	if( version == 2 )
	{
		/// 사운드
		tok.GetNext( &str );
		if( str != "sound" )
			return;
		{
			tok.GetNext( &str );
			pathName = path;
			pathName += str;
			LoadSound( pathName );
		}
	}

	/// 장면
	tok.GetNext( &str );

	if( str != "scene" )
		return;
	{
		tok.GetNext( &str );
		pathName = path;
		pathName += str;
		LoadScene( pathName );
	}

	/// 

	/// 카메라
	{
		str = fileName;
		str.Replace( "mapproj", "cam" );
		pathName = path;
		pathName += str;
		TABWIN->LoadCamera( pathName );
	}

	/// 팔레트
	{
		str = fileName;
		str.Replace( "mapproj", "pal" );
		pathName = path;
		pathName += str;
		TABWIN->GetTerrainPaintingDialog()->GetPaintDialog()->LoadPalette( pathName );
	}
}

void cMainFrame::SaveProject( cString pathName )
{
	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			FALSE, "Save Project", "*.mapproj",
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"Project Files (*.mapproj)| *.mapproj| All Files (*.*)| *.*|" );

		if( dlg.DoModal() == IDOK )
		{
			pathName = (LPCTSTR)dlg.GetPathName();
		}
	}
	if( pathName.IsEmpty() )
	{
		return;
	}

	cString path;
	cString fileName;
	::GetFilePath( &path, pathName );
	::GetFileName( &fileName, pathName );
	fileName.Erase( fileName.FindReverse( '.' ) );

	///
	cFileSaver saver;
	if( saver.Open( pathName ) == false )
		return;

	cString str;
	str = "IrisMapProject\r\n";
	saver.Write( str.Cstr(), str.GetSize() );

	str = "version 2\n\n";
	saver.Write( str.Cstr(), str.GetSize() );

	/// 지형을 저장
	{
		cString pathName = TERRAINEDIT->GetPathName();

		if( pathName.IsEmpty() )
		{
			pathName.Format( "%s%s.terrain", path.Cstr(), fileName.Cstr() );

			if( TERRAINEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save Terrain", "Terrain" );
			}
		}
		else
		{
			if( TERRAINEDIT->Save() == false )
			{
				MessageBox( "Failed to Save Terrain", "Terrain" );
			}
		}

		::GetFileName( &pathName, pathName );
		str.Format( "terrain \"%s\"\r\n", pathName.Cstr() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	/// 네비메시를 저장
	{
		cString pathName = NAVIMESHEDIT->GetPathName();

		if( pathName.IsEmpty() )
		{
			pathName.Format( "%s%s.navimesh", path.Cstr(), fileName.Cstr() );

			if( NAVIMESHEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
			}
		}
		else
		{
			if( NAVIMESHEDIT->Save() == false )
			{
				MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
			}
		}

		::GetFileName( &pathName, pathName );
		str.Format( "navimesh \"%s\"\r\n", pathName.Cstr() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	/// 네비필드를 저장
	{
		cString pathName = NAVIFIELDEDIT->GetPathName();

		if( pathName.IsEmpty() )
		{
			pathName.Format( "%s%s.navifield", path.Cstr(), fileName.Cstr() );

			if( NAVIFIELDEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save NaviField", "NaviField" );
			}
		}
		else
		{
			if( NAVIFIELDEDIT->Save() == false )
			{
				MessageBox( "Failed to Save NaviField", "NaviField" );
			}
		}

		::GetFileName( &pathName, pathName );
		str.Format( "navifield \"%s\"\r\n", pathName.Cstr() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	/// 사운드를 저장
	{
		cString pathName = SOUNDEDIT->GetPathName();

		if( pathName.IsEmpty() )
		{
			pathName.Format( "%s%s.sound", path.Cstr(), fileName.Cstr() );

			if( SOUNDEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save Sound Scene", "Sound" );
			}
		}
		else
		{
			if( SOUNDEDIT->Save() == false )
			{
				MessageBox( "Failed to Sound Save Scene", "Sound" );
			}
		}

		::GetFileName( &pathName, pathName );
		str.Format( "sound \"%s\"\r\n", pathName.Cstr() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	/// 장면을 저장
	{
		cString pathName = OBJECTEDIT->GetPathName();

		if( pathName.IsEmpty() )
		{
			pathName.Format( "%s%s.scene", path.Cstr(), fileName.Cstr() );

			if( OBJECTEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save Scene", "Scene" );
			}
		}
		else
		{
			if( OBJECTEDIT->Save() == false )
			{
				MessageBox( "Failed to Save Scene", "Scene" );
			}
		}

		::GetFileName( &pathName, pathName );
		str.Format( "scene \"%s\"\r\n", pathName.Cstr() );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	/// 카메라를 저장
	{
		cString pathName;
		pathName.Format( "%s%s.cam", path.Cstr(), fileName.Cstr() );

		VIEW->SaveCameraAs( pathName );
	}

	/// 팔레트를 저장
	{
		cString pathName;
		pathName.Format( "%s%s.pal", path.Cstr(), fileName.Cstr() );

		TABWIN->GetTerrainPaintingDialog()->GetPaintDialog()->SavePalette( pathName );
	}
}

void cMainFrame::LoadTerrain( cString pathName, bool loadNavi )
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

	if( loadNavi )
	{
		/// 지형과 네비메시의 크기가 다르면 네비메시와 네비필드를 로딩
		if( TERRAINEDIT->GetResolution() != NAVIMESHEDIT->GetResolution() ||
			TERRAINEDIT->GetMetersPerVertex() != NAVIMESHEDIT->GetMetersPerVertex() )
		{
			pathName.Replace( ".terrain", ".navimesh" );
			LoadNaviMesh( pathName );

			pathName.Replace( ".navimesh", ".navifield" );
			LoadNaviField( pathName );
		}
	}
}

void cMainFrame::LoadScene( cString pathName )
{
	/// 장면이 변경된 경우 저장할 지를 결정
	if( mSceneModified )
	{
		int mb = MessageBox(
			"The Scene has been modified.\n"
			"Do you want to save your changes?",
			"Scene",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSceneSave();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load Scene", "*.scene",
			OFN_HIDEREADONLY,
			"Scene Files (*.scene)| *.scene| All Files (*.*)| *.*|" );

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

		if( OBJECTEDIT->Load( pathName ) == false )
		{
			MessageBox( "Failed to Load Scene", "Scene" );
		}
	}
}

void cMainFrame::AddScene( cString pathName )
{
	/// 장면이 변경된 경우 저장할 지를 결정
	if( mSceneModified )
	{
		int mb = MessageBox(
			"The Scene has been modified.\n"
			"Do you want to save your changes?",
			"Scene",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSceneSave();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Add Scene", "*.scene",
			OFN_HIDEREADONLY,
			"Scene Files (*.scene)| *.scene| All Files (*.*)| *.*|" );

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
		if( OBJECTEDIT->Add( pathName ) == false )
		{
			MessageBox( "Failed to Add Scene", "Scene" );
		}
	}
}

void cMainFrame::LoadSound( cString pathName )
{
	if( mSoundModified )
	{
		int mb = MessageBox(
			"The Sound Scene has been modified.\n"
			"Do you want to save your changes?",
			"Sound",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSoundSaveAs();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load Sound Scene", "*.sound",
			OFN_HIDEREADONLY,
			"Sound Scene Files (*.sound)| *.sound| All Files (*.*)| *.*|" );

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
		if( SOUNDEDIT->Load( pathName ) == false )
		{
			MessageBox( "Failed to Load Sound Scene", "sound" );
		}
	}
}

void cMainFrame::LoadNaviMesh( cString pathName )
{
	/// 네비메시가 변경된 경우 저장할 지를 결정
	if( mNaviMeshModified )
	{
		int mb = MessageBox(
			"The NaviMesh has been modified.\n"
			"Do you want to save your changes?",
			"NaviMesh",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviMeshSave();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load NaviMesh", "*.navimesh",
			OFN_HIDEREADONLY,
			"NaviMesh Files (*.navimesh)| *.navimesh| All Files (*.*)| *.*|" );

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
		if( NAVIMESHEDIT->Load( pathName ) == false )
		{
			MessageBox( "Failed to Load NaviMesh.\n""It will be reinitialized.", "NaviMesh" );
			NAVIMESHEDIT->Init( TERRAINEDIT->GetResolution(), TERRAINEDIT->GetMetersPerVertex() );
		}
	}

	/// 지형과 네비메시의 크기가 다르면 네비메시를 초기화
	if( TERRAINEDIT->GetResolution() != NAVIMESHEDIT->GetResolution() ||
		TERRAINEDIT->GetMetersPerVertex() != NAVIMESHEDIT->GetMetersPerVertex() )
	{
		MessageBox( "NaviMesh does not match with Terrain.\n""It will be reinitialized.", "NaviMesh" );
		NAVIMESHEDIT->Init( TERRAINEDIT->GetResolution(), TERRAINEDIT->GetMetersPerVertex() );
	}
}

void cMainFrame::LoadNaviField( cString pathName )
{
	/// 네비필드가 변경된 경우 저장할 지를 결정
	if( mNaviFieldModified )
	{
		int mb = MessageBox(
			"The NaviField has been modified.\n"
			"Do you want to save your changes?",
			"NaviField",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviFieldSave();
		else if( mb == IDCANCEL )
			return;
	}

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			TRUE, "Load NaviField", "*.navifield",
			OFN_HIDEREADONLY,
			"NaviField Files (*.navifield)| *.navifield| All Files (*.*)| *.*|" );

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
		if( NAVIFIELDEDIT->Load( pathName ) == false )
		{
			MessageBox( "Failed to Load NaviField.\n""It will be reinitialized.", "NaviField" );
			NAVIFIELDEDIT->Init( NAVIMESHEDIT->GetResolution(), NAVIMESHEDIT->GetMetersPerVertex() );
		}
	}

	/// 네비메시와 네비필드의 크기가 다르면 네비필드를 초기화
	if( NAVIMESHEDIT->GetResolution() != NAVIFIELDEDIT->GetResolution() )
	{
		MessageBox( "NaviField does not match with NaviMesh.\n""It will be reinitialized.", "NaviField" );
		NAVIFIELDEDIT->Init( NAVIMESHEDIT->GetResolution(), NAVIMESHEDIT->GetMetersPerVertex() );
	}
}

void cMainFrame::LoadAreaGroup( const cString& pathName )
{
	if( OBJECTEDIT->LoadAreaGroup( pathName ) == false )
	{
		MessageBox( "Failed to Load Area Group", "Scene" );
	}
}

void cMainFrame::LoadOscr( const cString& pathName )
{
	if( OBJECTEDIT->LoadObjectTransformScript( pathName ) == false )
	{
		MessageBox( "Failed to Load Object Transform Script", "Scene" );
	}
}

void cMainFrame::SetTerrainModified( const cString& pathName, bool modified )
{
	if( pathName.IsEmpty() )
		mTerrainName = "?.terrain";
	else
		mTerrainName = pathName;

	SetTerrainModified( modified );
}

void cMainFrame::SetTerrainModified( bool modified )
{
	if( modified != mTerrainModified )
	{
		mTerrainModified = modified;

		if( modified )
			mTerrainName.PushBack( '*' );
		else
			mTerrainName.TrimRight( '*' );
	}

	cString title;
	title.Format( "Iris MapEditor - %s, %s, %s, %s, %s", mTerrainName.Cstr(), mSceneName.Cstr(), mSoundSceneName.Cstr(), mNaviMeshName.Cstr(), mNaviFieldName.Cstr() );
	SetWindowText( title.Cstr() );
}

void cMainFrame::SetSceneModified( const cString& pathName, bool modified )
{
	if( pathName.IsEmpty() )
		mSceneName = "?.scene";
	else
		mSceneName = pathName;

	SetSceneModified( modified );
}

void cMainFrame::SetSceneModified( bool modified )
{
	if( modified != mSceneModified )
	{
		mSceneModified = modified;

		if( modified )
			mSceneName.PushBack( '*' );
		else
			mSceneName.TrimRight( '*' );
	}

	cString title;
	title.Format( "Iris MapEditor - %s, %s, %s, %s, %s", mTerrainName.Cstr(), mSceneName.Cstr(), mSoundSceneName.Cstr(), mNaviMeshName.Cstr(), mNaviFieldName.Cstr() );
	SetWindowText( title.Cstr() );
}

void cMainFrame::SetSoundSceneModified( const cString& pathName, bool modified )
{
	if( pathName.IsEmpty() )
		mSoundSceneName= "?.sound";
	else
		mSoundSceneName = pathName;

	SetSoundSceneModified( modified );
}

void cMainFrame::SetSoundSceneModified( bool modified )
{
	if( modified != mSoundModified )
	{
		mSoundModified = modified;

		if( modified )
			mSoundSceneName.PushBack( '*' );
		else
			mSoundSceneName.TrimRight( '*' );
	}

	cString title;
	title.Format( "Iris MapEditor - %s, %s, %s, %s, %s", mTerrainName.Cstr(), mSceneName.Cstr(), mSoundSceneName.Cstr(), mNaviMeshName.Cstr(), mNaviFieldName.Cstr() );
	SetWindowText( title.Cstr() );
}

void cMainFrame::SetNaviMeshModified( const cString& pathName, bool modified )
{
	if( pathName.IsEmpty() )
		mNaviMeshName = "?.navimesh";
	else
		mNaviMeshName = pathName;

	SetNaviMeshModified( modified );
}

void cMainFrame::SetNaviMeshModified( bool modified )
{
	if( modified != mNaviMeshModified )
	{
		mNaviMeshModified = modified;

		if( modified )
			mNaviMeshName.PushBack( '*' );
		else
			mNaviMeshName.TrimRight( '*' );
	}

	cString title;
	title.Format( "Iris MapEditor - %s, %s, %s, %s, %s", mTerrainName.Cstr(), mSceneName.Cstr(), mSoundSceneName.Cstr(), mNaviMeshName.Cstr(), mNaviFieldName.Cstr() );
	SetWindowText( title.Cstr() );
}

void cMainFrame::SetNaviFieldModified( const cString& pathName, bool modified )
{
	if( pathName.IsEmpty() )
		mNaviMeshName = "?.navifield";
	else
		mNaviFieldName = pathName;

	SetNaviFieldModified( modified );
}

void cMainFrame::SetNaviFieldModified( bool modified )
{
	if( modified != mNaviFieldModified )
	{
		mNaviFieldModified = modified;

		if( modified )
			mNaviFieldName.PushBack( '*' );
		else
			mNaviFieldName.TrimRight( '*' );
	}

	cString title;
	title.Format( "Iris MapEditor - %s, %s, %s, %s, %s", mTerrainName.Cstr(), mSceneName.Cstr(), mSoundSceneName.Cstr(), mNaviMeshName.Cstr(), mNaviFieldName.Cstr() );
	SetWindowText( title.Cstr() );
}

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
	int id = mSplitter.IdFromRowCol( 0, 0 );
	mLeftSplitter.CreateStatic( &mSplitter, 2, 1, WS_CHILD | WS_VISIBLE, id );
	mLeftSplitter.CreateView( 0, 0, RUNTIME_CLASS(cTabWindow), CSize(0, 315), context );
	mLeftSplitter.CreateView( 1, 0, RUNTIME_CLASS(cExplorerWindow), CSize(0, 200), context );

	/// 오른쪽
	mSplitter.CreateView( 0, 1, RUNTIME_CLASS(cMapEditorView), CSize(800, 600), context );

	mSplitter.SetColumnInfo( 0, 210, 0 );

	/// 탭 윈도우와 뷰를 얻어옴
	mTabWindow = (cTabWindow*)mLeftSplitter.GetPane( 0, 0 );
	mView = (cMapEditorView*)mSplitter.GetPane( 0, 1 );
	assert( mTabWindow );
	assert( mView );


	if( mView )
	{
		/// 뷰에 필요한 다이얼로그들을 설정
		{
			cTerrainBuildingDialog* dlg = mTabWindow->GetTerrainBuildingDialog();
			mView->SetTerrainBuildDialog( dlg->GetBuildDialog() );
			mView->SetTerrainOptionDialog( dlg->GetOptionDialog() );
			mView->SetTerrainInfoDialog( dlg->GetInfoDialog() );
		}
		{
			cTerrainPaintingDialog* dlg = mTabWindow->GetTerrainPaintingDialog();
			mView->SetTerrainPaintDialog( dlg->GetPaintDialog() );
		}
		{
			cObjectDialog* dlg = mTabWindow->GetObjectDialog();
			mView->SetObjectTransformDialog( dlg->GetTransformDialog() );
			mView->SetObjectPropertyDialog( dlg->GetPropertyDialog() );
			mView->SetObjectInfoDialog( dlg->GetInfoDialog() );
		}
		{
			cLightDialog* dlg = mTabWindow->GetLightDialog();
			mView->SetLightTransformDialog( dlg->GetTransformDialog() );
			mView->SetLightPropertyDialog( dlg->GetPropertyDialog() );
			mView->SetLightInfoDialog( dlg->GetInfoDialog() );
		}
		{
			cSoundDialog* dlg = mTabWindow->GetSoundDialog();
			mView->SetSoundTransformDialog( dlg->GetTransformDialog() );
			mView->SetSoundPropertyDialog( dlg->GetPropertyDialog() );
			mView->SetSoundInfoDialog( dlg->GetInfoDialog() );
		}
		{
			cNaviMeshDialog* dlg = mTabWindow->GetNaviMeshDialog();
			mView->SetNaviMeshBuildDialog( dlg->GetBuildDialog() );
		}
		{
			cNaviFieldDialog* dlg = mTabWindow->GetNaviFieldDialog();
			mView->SetNaviFieldPaintDialog( dlg->GetPaintDialog() );
			mView->SetNaviFieldInfoDialog( dlg->GetInfoDialog() );
		}
		{
			cAreaDialog* dlg = mTabWindow->GetAreaDialog();
			mView->SetAreaTransformDialog( dlg->GetTransformDialog() );
			mView->SetAreaLightDialog( dlg->GetLightDialog() );
			mView->SetAreaFogDialog( dlg->GetFogDialog() );
			mView->SetAreaEtcDialog( dlg->GetEtcDialog() );
		}
		{
			cEnvDialog* dlg = mTabWindow->GetEnvDialog();
			mView->SetEnvCameraDialog( dlg->GetCameraDialog() );
			mView->SetEnvLevelDistanceDialog( dlg->GetLevelDistanceDialog() );
			mView->SetEnvSkyDialog( dlg->GetSkyDialog() );
			mView->SetEnvDirLightDialog( dlg->GetDirLightDialog() );
		}
		{
			cSettingDialog* dlg = mTabWindow->GetSettingDialog();
			mView->SetSettingViewDialog( dlg->GetViewDialog() );
			mView->SetSettingControlDialog( dlg->GetControlDialog() );
		}

		///
		mView->Init();
		return TRUE;
	}
	return FALSE;
}

int cMainFrame::OnCreate( LPCREATESTRUCT cs )
{
	if( CFrameWnd::OnCreate(cs) == -1 )
		return -1;

	mTerrainName = "?.terrain";
	mSceneName = "?.scene";
	mSoundSceneName = "?.sound";
	mNaviMeshName = "?.navimesh";
	mNaviFieldName = "?.navifield";

	mTerrainModified = false;
	mSceneModified = false;
	mSoundModified = false;
	mNaviMeshModified = false;
	mNaviFieldModified = false;

	SetWindowText( "Iris MapEditor - ?.terrain, ?.scene, ?.sound, ?.navimesh, ?.navifield" );
	return 0;
}

void cMainFrame::OnClose()
{
	/// 지형이 변경됐는지를 검사
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

	/// 장면이 변경됐는지를 검사
	if( mSceneModified )
	{
		int mb = MessageBox(
			"The Scene has been modified.\n"
			"Do you want to save your changes?",
			"Scene",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSceneSave();
		else if( mb == IDCANCEL )
			return;
	}

	/// 사운드 변경됐는지를 검사
	if( mSoundModified )
	{
		int mb = MessageBox(
			"The Sound Scene has been modified.\n"
			"Do you want to save your changes?",
			"Sound",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSoundSave();
		else if( mb == IDCANCEL )
			return;
	}

	/// 네비메시가 변경됐는지를 검사
	if( mNaviMeshModified )
	{
		int mb = MessageBox(
			"The NaviMesh has been modified.\n"
			"Do you want to save your changes?",
			"NaviMesh",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviMeshSave();
		else if( mb == IDCANCEL )
			return;
	}

	/// 네비필드가 변경됐는지를 검사
	if( mNaviFieldModified )
	{
		int mb = MessageBox(
			"The NaviField has been modified.\n"
			"Do you want to save your changes?",
			"NaviField",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviFieldSave();
		else if( mb == IDCANCEL )
			return;
	}

	CFrameWnd::OnClose();
}

void cMainFrame::OnLoadProject()
{
	LoadProject( "" );
}

void cMainFrame::OnSaveProject()
{
	SaveProject( "" );
}

void cMainFrame::OnSaveScreenShot()
{
	/// 높이맵을 저장
	CFileDialog dlg(
		FALSE, "Save Screen Shot", "*.tga",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Image Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName = (LPCTSTR)dlg.GetPathName();

		if( VIEW->SaveScreenShot( pathName, 1024, 1024 ) == false )
		{
			MessageBox( "Failed to Save Screen Shot", "View" );
		}
	}
}

void cMainFrame::OnSaveLog()
{
	/// 로그를 저장
	cLogDialog dlg;
	int ret = dlg.DoModal();

	if( ret != IDOK )
		return;

	CFileDialog fdlg(
		FALSE, "Save Log", "*.csv",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Log Files (*.csv)| *.csv|"
		"All Files (*.*)| *.*|" );

	if( fdlg.DoModal() != IDOK )
		return;

	cString pathName = (LPCTSTR)fdlg.GetPathName();
	::GetFileName( &pathName, pathName );

	if( VIEW->SaveLog( pathName, dlg.GetResolution() ) == false )
	{
		MessageBox( "Failed to Save Log", "View" );
	}
}

void cMainFrame::OnTerrainNew()
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

	/// 네비메시가 변경된 경우 저장할 지를 결정
	if( mNaviMeshModified )
	{
		int mb = MessageBox(
			"The NaviMesh has been modified.\n"
			"Do you want to save your changes?",
			"NaviMesh",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviMeshSave();
		else if( mb == IDCANCEL )
			return;
	}

	/// 네비필드가 변경된 경우 저장할 지를 결정
	if( mNaviMeshModified )
	{
		int mb = MessageBox(
			"The NaviField has been modified.\n"
			"Do you want to save your changes?",
			"NaviField",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnNaviFieldSave();
		else if( mb == IDCANCEL )
			return;
	}

	cTerrainInitDialog dlg;
	int ret = dlg.DoModal();

	if( ret == IDOK )
	{
		int resolution = dlg.GetResolution();
		int metersPerVertex = dlg.GetMetersPerVertex();

		TERRAINEDIT->Init( resolution, (float)metersPerVertex, 100 );
		NAVIMESHEDIT->Init( resolution, (float)metersPerVertex, 100 );
		NAVIFIELDEDIT->Init( resolution, (float)metersPerVertex, 100 );
	}
}

void cMainFrame::OnTerrainOpen()
{
	LoadTerrain( "" );
}

void cMainFrame::OnTerrainSave()
{
	/// 지형을 저장
	cString pathName = TERRAINEDIT->GetPathName();

	if( pathName.IsEmpty() )
	{
		CFileDialog dlg(
			FALSE, "Save Terrain", "*.terrain",
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"Terrain Files (*.terrain)| *.terrain|"
			"All Files (*.*)| *.*|" );

		if( dlg.DoModal() == IDOK )
		{
			pathName = (LPCTSTR)dlg.GetPathName();

			if( TERRAINEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save Terrain", "Terrain" );
			}
		}
	}
	else
	{
		if( TERRAINEDIT->Save() == false )
		{
			MessageBox( "Failed to Save Terrain", "Terrain" );
		}
	}

	/// 네비메시를 저장
	if( NAVIMESHEDIT->GetPathName().IsEmpty() )
	{
		if( pathName.IsEmpty() )
		{
			CFileDialog dlg(
				FALSE, "Save NaviMesh", "*.navimesh",
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				"NaviMesh Files (*.navimesh)| *.navimesh|"
				"All Files (*.*)| *.*|" );

			if( dlg.DoModal() == IDOK )
			{
				pathName = (LPCTSTR)dlg.GetPathName();
			}
		}
		else
		{
			pathName.Erase( pathName.FindReverse( '.' ) );
			pathName += ".navimesh";
		}

		if( pathName.IsEmpty() == false )
		{
			if( NAVIMESHEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
			}
		}
	}
	else
	{
		if( NAVIMESHEDIT->Save() == false )
		{
			MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
		}
	}

	/// 네비필드를 저장
	if( NAVIFIELDEDIT->GetPathName().IsEmpty() )
	{
		if( pathName.IsEmpty() )
		{
			CFileDialog dlg(
				FALSE, "Save NaviField", "*.navifield",
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				"NaviField Files (*.navifield)| *.navifield|"
				"All Files (*.*)| *.*|" );

			if( dlg.DoModal() == IDOK )
			{
				pathName = (LPCTSTR)dlg.GetPathName();
			}
		}
		else
		{
			pathName.Erase( pathName.FindReverse( '.' ) );
			pathName += ".navifield";
		}

		if( pathName.IsEmpty() == false )
		{
			if( NAVIFIELDEDIT->SaveAs( pathName ) == false )
			{
				MessageBox( "Failed to Save NaviField", "NaviField" );
			}
		}
	}
	else
	{
		if( NAVIFIELDEDIT->Save() == false )
		{
			MessageBox( "Failed to Save NaviField", "NaviField" );
		}
	}
}

void cMainFrame::OnTerrainSaveAs()
{
	/// 지형을 저장
	CFileDialog dlg(
		FALSE, "Save Terrain", "*.terrain",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Terrain Files (*.terrain)| *.terrain|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDCANCEL )
		return;

	cString pathName = (LPCTSTR)dlg.GetPathName();

	if( TERRAINEDIT->SaveAs( pathName ) == false )
	{
		MessageBox( "Failed to Save Terrain", "Terrain" );
	}

	/// 네비메시를 저장
	pathName.Replace( ".terrain", ".navimesh" );

	if( NAVIMESHEDIT->SaveAs( pathName ) == false )
	{
		MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
	}

	/// 네비필드를 저장
	pathName.Replace( ".navimesh", ".navifield" );

	if( NAVIFIELDEDIT->SaveAs( pathName ) == false )
	{
		MessageBox( "Failed to Save NaviField", "NaviField" );
	}
}

void cMainFrame::OnTerrainClearHeightMap()
{
	int mb = MessageBox(
		"Do you really want to clear height map?",
		"Terrain",
		MB_ICONQUESTION | MB_YESNO );

	if( mb == IDYES )
	{
		TERRAINEDIT->ClearHeightMap();
	}
}

void cMainFrame::OnTerrainImportHeightMap()
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

	/// 높이맵을 로딩
	CFileDialog dlg(
		TRUE, "Import HeightMap", "*.hmap",
		OFN_HIDEREADONLY,
		"HeightMap Files (*.hmap)| *.hmap|"
		"HeightMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName;
		pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ImportHeightMap( pathName ) == false )
		{
			MessageBox( "Failed to Import HeightMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainExportHeightMap()
{
	/// 높이맵을 저장
	CFileDialog dlg(
		FALSE, "Export HeightMap", "*.hmap",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"HeightMap Files (*.hmap)| *.hmap|"
		"HeightMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ExportHeightMap( pathName ) == false )
		{
			MessageBox( "Failed to Export HeightMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainClearAlphaMap()
{
	int mb = MessageBox(
		"Do you really want to clear alpha map?",
		"Terrain",
		MB_ICONQUESTION | MB_YESNO );

	if( mb == IDYES )
	{
		TERRAINEDIT->ClearAlphaMap();
	}
}

void cMainFrame::OnTerrainImportAlphaMap()
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

	/// 알파맵을 로딩
	CFileDialog dlg(
		TRUE, "Import AlphaMap", "*.tga",
		OFN_HIDEREADONLY,
		"AlphaMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName;
		pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ImportAlphaMap( pathName ) == false )
		{
			MessageBox( "Failed to Import AlphaMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainExportAlphaMap()
{
	/// 알파맵을 저장
	CFileDialog dlg(
		FALSE, "Export AlphaMap", "*.tga",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"AlphaMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ExportAlphaMap( pathName ) == false )
		{
			MessageBox( "Failed to Export AlphaMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainClearColorMap()
{
	int mb = MessageBox(
		"Do you really want to clear color map?",
		"Terrain",
		MB_ICONQUESTION | MB_YESNO );

	if( mb == IDYES )
	{
		TERRAINEDIT->ClearColorMap();
	}
}

void cMainFrame::OnTerrainImportColorMap()
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

	/// 색상맵을 로딩
	CFileDialog dlg(
		TRUE, "Import ColorMap", "*.tga",
		OFN_HIDEREADONLY,
		"ColorMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName;
		pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ImportColorMap( pathName ) == false )
		{
			MessageBox( "Failed to Import ColorMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainExportColorMap()
{
	/// 색상맵을 저장
	CFileDialog dlg(
		FALSE, "Export ColorMap", "*.tga",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"ColorMap Files (*.tga)| *.tga|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		cString pathName = (LPCTSTR)dlg.GetPathName();

		if( TERRAINEDIT->ExportColorMap( pathName ) == false )
		{
			MessageBox( "Failed to Export ColorMap", "Terrain" );
		}
	}
}

void cMainFrame::OnTerrainAdjustHeight()
{
	cTerrainScaleZDialog dlg;

	if( dlg.DoModal() == IDOK )
	{
		TERRAINEDIT->AdjustHeight( dlg.GetScale() );
	}
}

void cMainFrame::OnSceneNew()
{
	/// 오브젝트들이 변경된 경우 저장할 지를 결정
	if( mSceneModified )
	{
		int mb = MessageBox(
			"The Scene has been modified.\n"
			"Do you want to save your changes?",
			"Scene",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSceneSaveAs();
		else if( mb == IDCANCEL )
			return;
	}
    
	OBJECTEDIT->Init();
}

void cMainFrame::OnSceneOpen()
{
	LoadScene( "" );
}

void cMainFrame::OnSceneAdd()
{
	AddScene( "" );
}

void cMainFrame::OnSceneSave()
{
	if( OBJECTEDIT->GetPathName().IsEmpty() )
	{
		OnSceneSaveAs();
	}
	else
	{
		if( OBJECTEDIT->Save() == false )
		{
			MessageBox( "Failed to Save Scene", "Scene" );
		}
	}
}

void cMainFrame::OnSceneSaveAs()
{
	CFileDialog dlg(
		FALSE, "Save Scene", "*.scene",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Scene Files (*.scene)| *.scene|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( OBJECTEDIT->SaveAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Scene", "Scene" );
		}
	}
}

void cMainFrame::OnSceneSaveAreaAs()
{
	CFileDialog dlg(
		FALSE, "Save Scene", "*.scene",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Scene Files (*.scene)| *.scene|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( OBJECTEDIT->SaveAreaAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Scene", "Scene" );
		}
	}
}

void cMainFrame::OnSceneSaveObjectGroupAs()
{
	CFileDialog dlg(
		FALSE, "Save Object Group", "*.scene",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Scene Files (*.scene)| *.scene|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( OBJECTEDIT->SaveObjectGroupAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Object Group", "Scene" );
		}
	}
}

void cMainFrame::OnSceneLoadAreaGroup()
{
	CFileDialog dlg(
		TRUE, "Load Area Group", "*.txt",
		OFN_HIDEREADONLY,
		"Area Group Files (*.txt)| *.txt| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		LoadAreaGroup( (LPCTSTR)dlg.GetPathName() );
	}
}

void cMainFrame::OnSceneLoadObjectTransformScript()
{
	CFileDialog dlg(
		TRUE, "Load Object Transform Script", "*.oscr",
		OFN_HIDEREADONLY,
		"Object Transform Script (*.oscr)| *.oscr| All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		LoadOscr( (LPCTSTR)dlg.GetPathName() );
	}
}

void cMainFrame::OnSceneAdjustAllPos()
{
	cObjectScaleDialog dlg;

	if( dlg.DoModal() == IDOK )
	{
		OBJECTEDIT->AdjustAllPos( dlg.GetScale() );
	}
}

void cMainFrame::OnSceneAdjustAllSize()
{
	cObjectScaleDialog dlg;

	if( dlg.DoModal() == IDOK )
	{
		OBJECTEDIT->AdjustAllSize( dlg.GetScale() );
	}
}

void cMainFrame::OnSoundNew()
{
	/// 사운드 객체들이 변경된 경우 저장할 지를 결정
	if( mSoundModified )
	{
		int mb = MessageBox(
			"The Sound Scene has been modified.\n"
			"Do you want to save your changes?",
			"Sound",
			MB_ICONQUESTION | MB_YESNOCANCEL );

		if( mb == IDYES )
			OnSoundSaveAs();
		else if( mb == IDCANCEL )
			return;
	}

	SOUNDEDIT->Init();
}

void cMainFrame::OnSoundOpen()
{
	LoadSound( "" );
}

void cMainFrame::OnSoundSave()
{
	if( SOUNDEDIT->GetPathName().IsEmpty() )
	{
		OnSoundSaveAs();
	}
	else
	{
		if( SOUNDEDIT->Save() == false )
		{
			MessageBox( "Failed to Save Sound Scene", "Sound" );
		}
	}
}

void cMainFrame::OnSoundSaveAs()
{
	CFileDialog dlg(
		FALSE, "Save Sound Scene", "*.sound",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Sound Scene Files (*.sound)| *.sound|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( SOUNDEDIT->SaveAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save Sound Scene", "Sound" );
		}
	}
}

void cMainFrame::OnNaviMeshSave()
{
	if( NAVIMESHEDIT->GetPathName().IsEmpty() )
	{
		OnNaviMeshSaveAs();
	}
	else
	{
		if( NAVIMESHEDIT->Save() == false )
		{
			MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
		}
	}
}

void cMainFrame::OnNaviMeshSaveAs()
{
	CFileDialog dlg(
		FALSE, "Save NaviMesh", "*.navimesh",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"NaviMesh Files (*.navimesh)| *.navimesh|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( NAVIMESHEDIT->SaveAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save NaviMesh", "NaviMesh" );
		}
	}
}

void cMainFrame::OnNaviFieldSave()
{
	if( NAVIFIELDEDIT->GetPathName().IsEmpty() )
	{
		OnNaviFieldSaveAs();
	}
	else
	{
		if( NAVIFIELDEDIT->Save() == false )
		{
			MessageBox( "Failed to Save NaviField", "NaviField" );
		}
	}
}

void cMainFrame::OnNaviFieldSaveAs()
{
	CFileDialog dlg(
		FALSE, "Save NaviField", "*.navifield",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"NaviField Files (*.navifield)| *.navifield|"
		"All Files (*.*)| *.*|" );

	if( dlg.DoModal() == IDOK )
	{
		if( NAVIFIELDEDIT->SaveAs( (LPCTSTR)dlg.GetPathName() ) == false )
		{
			MessageBox( "Failed to Save NaviField", "NaviField" );
		}
	}
}

void cMainFrame::OnEditCopy()
{
	if( VIEW )
		VIEW->OnEditCopy();
}

void cMainFrame::OnEditSelectAll()
{
	if( OBJECTEDIT )
		OBJECTEDIT->SelectAll();
}

void cMainFrame::OnEditDeselectAll()
{
	if( VIEW )
		VIEW->OnEditDeselectAll();
}

void cMainFrame::OnEditChangeBackgroundColor()
{
	if( VIEW )
		VIEW->OnEditChangeBackgroundColor();
}

void cMainFrame::OnEditUndo()
{
	if( DOINGMAN )
		DOINGMAN->Undo();
}

void cMainFrame::OnEditRedo()
{
	if( DOINGMAN )
		DOINGMAN->Redo();
}

void cMainFrame::OnEscape()
{
	if( VIEW )
		VIEW->SetEditMode( eEDIT_NULL );
	if( TABWIN )
		TABWIN->OnEscape();
}
