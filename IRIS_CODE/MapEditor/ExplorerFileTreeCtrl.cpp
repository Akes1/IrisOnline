#include "stdafx.h"
#include "ExplorerFileTreeCtrl.h"

#include "Resource.h"
#include "MainFrame.h"
#include "MapEditorView.h"
#include "TerrainEditor.h"
#include "ObjectEditor.h"
#include "SoundEditor.h"
#include "NaviMeshEditor.h"

BEGIN_MESSAGE_MAP(cExplorerFileTreeCtrl, cFileTreeCtrl)
	ON_COMMAND( ID_FILE_PROPERTY, OnProperties )
	ON_UPDATE_COMMAND_UI( ID_FILE_PROPERTY, OnUpdateProperties )
	ON_COMMAND( ID_LOAD_PROJECT, OnLoadProject )
	ON_COMMAND( ID_CREATE_OBJECT, OnCreateObject )
	ON_COMMAND( ID_SELECT_OBJECT, OnSelectObject )
	ON_COMMAND( ID_CREATE_SOUND, OnCreateSound )
	ON_COMMAND( ID_OPEN_TERRAIN, OnOpenTerrain )
	ON_COMMAND( ID_OPEN_SCENE, OnOpenScene )
	ON_COMMAND( ID_ADD_SCENE, OnAddScene )
	ON_COMMAND( ID_OPEN_SOUND, OnOpenSound )
	ON_COMMAND( ID_OPEN_NAVIMESH, OnOpenNaviMesh )
	ON_COMMAND( ID_OPEN_NAVIFIELD, OnOpenNaviField )
	ON_COMMAND( ID_LOAD_AREA_GROUP, OnLoadAreaGroup )
	ON_COMMAND( ID_LOAD_OSCR, OnLoadOscr )
	ON_COMMAND( ID_LOAD_TERRAIN_TEXTURE_EMPTY_SLOT, OnLoadTerrainTexture )
	ON_COMMAND( ID_IMPORT_HEIGHTMAP, OnImportHeightMap )
	ON_COMMAND( ID_IMPORT_ALPHAMAP, OnImportAlphaMap )
	ON_COMMAND( ID_IMPORT_COLORMAP, OnImportColorMap )
END_MESSAGE_MAP()

cExplorerFileTreeCtrl::cExplorerFileTreeCtrl()
{
}

cExplorerFileTreeCtrl::~cExplorerFileTreeCtrl()
{
}

void cExplorerFileTreeCtrl::PopMenu( CPoint point )
{
	CMenu menu;
	cString pathName = GetSelectedPath();
	cString ext;
	GetFileExtension( &ext, pathName );

	if( ext == "mapproj" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_PROJECT_FILE));
	}
	else if( ext == "terrain" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_TERRAIN_FILE));
	}
	else if( ext == "scene" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_SCENE_FILE));
	}
	else if( ext == "sound" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_SOUND_SCENE_FILE));
	}
	else if( ext == "navimesh" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_NAVIMESH_FILE));
	}
	else if( ext == "navifield" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_NAVIFIELD_FILE));
	}
	else if( ext == "nif" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_OBJECT_FILE));
	}
	else if( ext == "mp3" || ext == "wav" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_SOUND_FILE));
	}
	else if( ext == "tga" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_TARGA_FILE));
	}
	else if( ext == "txt" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_TEXT_FILE));
	}
	else if( ext == "oscr" )
	{
		VERIFY(menu.LoadMenu(IDR_MENU_OSCR_FILE));
	}
	else
	{
		VERIFY(menu.LoadMenu(IDR_MENU_FILE));
	}

	CMenu* popup = menu.GetSubMenu(0);
	assert( popup != 0 );
	popup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
}

void cExplorerFileTreeCtrl::OnLoadProject()
{
	MAIN->LoadProject( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnCreateObject()
{
	OBJECTEDIT->CreateObject( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnSelectObject()
{
	OBJECTEDIT->SelectObject( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnCreateSound()
{
	SOUNDEDIT->CreateSound( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnOpenTerrain()
{
	MAIN->LoadTerrain( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnOpenScene()
{
	MAIN->LoadScene( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnOpenSound()
{
	MAIN->LoadSound( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnAddScene()
{
	MAIN->AddScene( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnOpenNaviMesh()
{
	MAIN->LoadNaviMesh( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnOpenNaviField()
{
	MAIN->LoadNaviField( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnLoadAreaGroup()
{
	MAIN->LoadAreaGroup( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnLoadOscr()
{
	MAIN->LoadOscr( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnLoadTerrainTexture()
{
	TERRAINEDIT->LoadTexture( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnImportHeightMap()
{
	TERRAINEDIT->ImportHeightMap( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnImportAlphaMap()
{
	TERRAINEDIT->ImportAlphaMap( (LPCTSTR)GetSelectedPath() );
}

void cExplorerFileTreeCtrl::OnImportColorMap()
{
	TERRAINEDIT->ImportColorMap( (LPCTSTR)GetSelectedPath() );
}
