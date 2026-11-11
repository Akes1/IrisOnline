#include "stdafx.h"
#include "TerrainEditor.h"

#include "MainFrame.h"
#include "MapEditorApp.h"
#include "MapEditorView.h"
#include "TerrainBuildDialog.h"
#include "TerrainPaintDialog.h"
#include "TerrainOptionDialog.h"
#include "TerrainInfoDialog.h"
#include "TerrainImportDialog.h"
#include "ObjectEditor.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/Box.h"
#include "Engine/Ray.h"
#include "Engine/Terrain.h"
#include "Engine/TerrainNode.h"
#include "Engine/RenderSystem.h"
#include "Engine/CameraManager.h"
#include "Engine/FreeCamera.h"
#include "Engine/SceneManager.h"
#include "Engine/SceneNode.h"

#include "Doing/DoingManager.h"

cTileGroup::cTileGroup()
{
	mBoxLines = NiNew NiMesh;
	mBoxLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mBoxLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 18, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::WHITE );
	mMatProp->SetDiffuseColor( NiColor::WHITE );
	mMatProp->SetSpecularColor( NiColor::WHITE );
	mMatProp->SetEmittance( NiColor( 1.0f, 0.0f, 0.0f ) );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mBoxLines->AttachProperty( mMatProp );
	mBoxLines->AttachProperty( mWireProp );
	mBoxLines->UpdateProperties();
	mBoxLines->Update( 0.0f );
}

cTileGroup::~cTileGroup()
{
}

void cTileGroup::Clear()
{
	mSelectedSet.Clear();
}

void cTileGroup::Render()
{
	if( mBoxLines == 0 )
		return;

	NiRenderer* renderer = NiRenderer::GetRenderer();

	NiDataStreamElementLock kPositionLock( mBoxLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		cTerrainLeafNode* n = *i;

		const NiPoint3& min = n->GetBoundBox().GetMin();
		const NiPoint3& max = n->GetBoundBox().GetMax();
		float minz = min.z - 100.0f;
		float maxz = max.z + 100.0f;

		NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

		kPositionIter[0] = NiPoint3( min.x, min.y, maxz );
		kPositionIter[1] = NiPoint3( max.x, min.y, maxz );
		kPositionIter[2] = NiPoint3( max.x, max.y, maxz );
		kPositionIter[3] = NiPoint3( min.x, max.y, maxz );
		kPositionIter[4] = NiPoint3( min.x, min.y, maxz );
		kPositionIter[5] = NiPoint3( min.x, min.y, minz );
		kPositionIter[6] = NiPoint3( max.x, min.y, minz );
		kPositionIter[7] = NiPoint3( max.x, max.y, minz );
		kPositionIter[8] = NiPoint3( min.x, max.y, minz );
		kPositionIter[9] = NiPoint3( min.x, min.y, minz );
		kPositionIter[10] = NiPoint3( min.x, min.y, maxz );
		kPositionIter[11] = NiPoint3( min.x, min.y, minz );
		kPositionIter[12] = NiPoint3( max.x, min.y, maxz );
		kPositionIter[13] = NiPoint3( max.x, min.y, minz );
		kPositionIter[14] = NiPoint3( max.x, max.y, maxz );
		kPositionIter[15] = NiPoint3( max.x, max.y, minz );
		kPositionIter[16] = NiPoint3( min.x, max.y, maxz );
		kPositionIter[17] = NiPoint3( min.x, max.y, minz );

		mBoxLines->RenderImmediate( renderer );
	}
	kPositionLock.Unlock();
}

bool cTileGroup::Select( cTerrainLeafNode* node )
{
	assert( node );

	cSelectedSet::cIterator i = mSelectedSet.Find( node );

	if( i != mSelectedSet.End() && mSelectedSet.GetSize() == 1 )
		return false;

	mSelectedSet.Clear();
	mSelectedSet.Insert( node );
	return true;
}

bool cTileGroup::Add( cTerrainLeafNode* node )
{
	assert( node );

	cSelectedSet::cIterator i = mSelectedSet.Find( node );

	if( i != mSelectedSet.End() )
		return false;

	mSelectedSet.Insert( node );
	return true;
}

bool cTileGroup::Remove( cTerrainLeafNode* node )
{
	assert( node );

	cSelectedSet::cIterator i = mSelectedSet.Find( node );

	if( i == mSelectedSet.End() )
		return false;

	mSelectedSet.Erase( i );
	return true;
}

/// 가시 여부
void cTileGroup::SetVisible( bool visible )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator iend = mSelectedSet.End();

	for( ; i != iend; ++i )
	{
		(*i)->SetVisible( visible );
	}
}

void cTileGroup::SetDetailTextures( unsigned int index0, unsigned int index1, unsigned int index2 )
{
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator iend = mSelectedSet.End();

	for( ; i != iend; ++i )
	{
		TERRAIN->SetDetailTextures( *i, index0, index1, index2 );
	}
}

int cTileGroup::GetTextureIndex0() const
{
	if( mSelectedSet.GetSize() )
		return (*mSelectedSet.Begin())->GetTextureIndex0();
	else
		return -1;
}

int cTileGroup::GetTextureIndex1() const
{
	if( mSelectedSet.GetSize() )
		return (*mSelectedSet.Begin())->GetTextureIndex1();
	else
		return -1;
}

int cTileGroup::GetTextureIndex2() const
{
	if( mSelectedSet.GetSize() )
		return (*mSelectedSet.Begin())->GetTextureIndex2();
	else
		return -1;
}

cBox cTileGroup::GetBoundBox() const
{
	switch( mSelectedSet.GetSize() )
	{
	case 0:
		return cBox( NiPoint3::ZERO, NiPoint3::ZERO );
	case 1:
		return (*mSelectedSet.Begin())->GetBoundBox();
	}

	NiPoint3 min = NiPoint3( +NI_INFINITY, +NI_INFINITY, +NI_INFINITY );
	NiPoint3 max = NiPoint3( -NI_INFINITY, -NI_INFINITY, -NI_INFINITY );

	cSelectedSet::cConstIterator i = mSelectedSet.Begin();
	cSelectedSet::cConstIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		cTerrainLeafNode* n = *i;

		const NiPoint3& mn = n->GetBoundBox().GetMin();
		const NiPoint3& mx = n->GetBoundBox().GetMax();

		if( mn.x < min.x )
			min.x = mn.x;
		if( mx.x > max.x )
			max.x = mx.x;
		if( mn.y < min.y )
			min.y = mn.y;
		if( mx.y > max.y )
			max.y = mx.y;
		if( mn.z < min.z )
			min.z = mn.z;
		if( mx.z > max.z )
			max.z = mx.z;
	}
	return cBox( min, max );
}

cTerrainEditor* cTerrainEditor::mSingleton = 0;

cTerrainEditor::cTerrainEditor()
: mBuildDialog( 0 )
, mPaintDialog( 0 )
, mOptionDialog( 0 )
, mInfoDialog( 0 )
, mDrawing( false )
, mPickPos( NiPoint3::ZERO )
, mPickOldPos( NiPoint3::ZERO )
, mPickPosChanged( false )
, mInnerLines( NiColorA(1.0f, 0.0f, 0.0f), NiColorA(1.0f, 1.0f, 0.0f) )
, mOuterLines( NiColorA(1.0f, 0.0f, 0.0f), NiColorA(1.0f, 1.0f, 0.0f) )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mPickedArray.Reserve( 1024 );
	mGroup = new cTileGroup;
}

cTerrainEditor::~cTerrainEditor()
{
	delete mGroup;

	mSingleton = 0;
}

void cTerrainEditor::Clear()
{
	mGroup->Clear();
	mDrawing = false;
	mPickOldPos = mPickPos = NiPoint3::ZERO;
	mPickPosChanged = false;
	mPathName.Clear();
}

void cTerrainEditor::Process()
{
	if( mPickPos != NiPoint3::ZERO )
	{
		if( mPickPosChanged )
		{
			mPickPosChanged = false;
			float ir = 0.0f;
			float or = 0.0f;

			if( mBuildDialog->GetCheckedButton() )
			{
				ir = mBuildDialog->GetInnerRadius() * TERRAIN->GetUnitsPerMeter();
				or = mBuildDialog->GetOuterRadius() * TERRAIN->GetUnitsPerMeter();
			}
			else if( mPaintDialog->GetCheckedButton() )
			{
				ir = mPaintDialog->GetInnerRadius() * TERRAIN->GetUnitsPerMeter();
				or = mPaintDialog->GetOuterRadius() * TERRAIN->GetUnitsPerMeter();
			}

			mInnerLines.Process( mPickPos, ir, TERRAIN );
			mOuterLines.Process( mPickPos, or, TERRAIN );
		}
	}
}

void cTerrainEditor::Render( bool group )
{
	if( group )
		mGroup->Render();

	mInnerLines.Render();
	mOuterLines.Render();
}

void cTerrainEditor::Init( unsigned int resolution, float metersPerVertex, unsigned int unitsPerMeter )
{
	/// 현재 경로를 설정
	cString path = theApp.GetBaseDir();
	SetCurrentDirectory( path.Cstr() );

	/// 지움
	Clear();

	/// 실행 기록을 지움
	DOINGMAN->ClearTerrain();

	/// 초기화
	TERRAIN->Init( resolution, metersPerVertex, unitsPerMeter );

	/// 뷰 모드를 적용
	if( mOptionDialog )
		SetViewMode( mOptionDialog->GetViewMode() );
	else
		SetViewMode( eTERRAIN_VIEW_TEXTURED );

	/// 줌인
	ZoomPersp( VIEW->GetFreeCamera() );

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	MAIN->SetTerrainModified( false );

	/// 장면 정보 다이얼로그를 갱신
	OBJECTEDIT->UpdateInfoDialog();

	/// 색칠 다이얼로그를 갱신
	if( mPaintDialog )
		mPaintDialog->UpdateTextures();

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->Update( resolution, TERRAIN->GetLeafCellCount(), metersPerVertex, unitsPerMeter );
}

bool cTerrainEditor::Load( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	::GetFilePath( &path, pathName );
	::SetCurrentDirectory( path.Cstr() );

	/// 지움
	Clear();

	/// 실행 기록을 지움
	DOINGMAN->ClearTerrain();

	/// 지형을 로딩
	cString str;

	switch( TERRAIN->Load( pathName ) )
	{
	case TERRAIN_LOAD_ERROR_OPEN:
		str = "Failed to open terrain file.";
		break;
	case TERRAIN_LOAD_ERROR_FILE_HEADER:
		str = "Failed to load terrain file header.";
		break;
	case TERRAIN_LOAD_ERROR_FILE_TYPE:
		str = "Invalid terrain file type.";
		break;
	case TERRAIN_LOAD_ERROR_FILE_VERSION:
		str = "Invalid terrain file version.";
		break;
	case TERRAIN_LOAD_ERROR_GRID_SIZE:
		str = "Invalid terrain grid size.";
		break;
	case TERRAIN_LOAD_ERROR_TEXTURE_NAME:
		str = "Failed to load terrain texture name.";
		break;
	case TERRAIN_LOAD_ERROR_TEXTURE:
		str = "Failed to load terrain texture.";
		break;
	case TERRAIN_LOAD_ERROR_NODE:
		str = "Failed to load terrain node.";
		break;
	}

	if( str.IsEmpty() == false )
	{
		AfxMessageBox( str.Cstr() );
		return false;
	}

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();

	/// 뷰 모드를 적용
	SetViewMode( mOptionDialog->GetViewMode() );

	/// 줌인
	ZoomPersp( VIEW->GetFreeCamera() );

	/// 주인공의 위치를 초기화
	//HERO->Reset();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetTerrainModified( name, false );

	/// 장면 정보 다이얼로그를 갱신
	OBJECTEDIT->UpdateInfoDialog();

	/// 정보 다이얼로그를 갱신
	mInfoDialog->Update( TERRAIN->GetCellCount(), TERRAIN->GetLeafCellCount(), TERRAIN->GetMetersPerVertex(), TERRAIN->GetUnitsPerMeter() );
	return true;
}

bool cTerrainEditor::Save()
{
	if( mPathName.IsEmpty() )
		return false;
	else
		return SaveAs( mPathName );
}

bool cTerrainEditor::SaveAs( const cString& pathName )
{
	bool ret = TERRAIN->Save( pathName );
	ret = TERRAIN->SavePaintAlpha( pathName );

	/// 미니맵을 저장
	cString tempName = pathName;
	tempName.Replace( ".terrain", ".tga" );

	VIEW->SelectCamera( 0, true );
	cFreeCamera* cam = VIEW->GetDefaultCamera();
	float farDist = cam->GetFarDistance();
	ZoomOrtho( cam );
	VIEW->SaveScreenShot( tempName, 1024, 1024 );
	
	/// 지형 이미지를 저장
	tempName.Replace( ".tga", "_terrain.tga" );
	VIEW->SaveScreenShot( tempName, TERRAIN->GetCellCount(), TERRAIN->GetCellCount(), true );

	///
	ZoomPersp( cam );
	cam->SetFarDistance( farDist );

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetTerrainModified( name, false );
	return ret;
}

void cTerrainEditor::ZoomPersp( cFreeCamera* cam )
{
	if( cam == 0 )
		return;
	if( TERRAIN->IsInited() == false )
		return;

	NiPoint3 min = TERRAIN->GetBoundBox().GetMin();
	NiPoint3 max = TERRAIN->GetBoundBox().GetMax();
	NiPoint3 center = (min + max) * 0.5f;
	float radius = center.x - min.x;
	float fovyRadian = 25.0f * NI_PI / 180.0f;
	float dist = (radius / cam->GetAspectRatio()) / ::tanf(fovyRadian);
	NiPoint3 pos = center;
	pos.y -= dist;
	pos.z += (dist * 0.5f);

	cam->SetOrtho( false );
	cam->SetFarDistance( cam->GetFarDistance() );
	cam->SetTranslate( pos );
	cam->LookAt( center );
}

void cTerrainEditor::ZoomOrtho( cFreeCamera* cam )
{
	if( cam == 0 )
		return;
	if( TERRAIN->IsInited() == false )
		return;

	NiPoint3 min = TERRAIN->GetBoundBox().GetMin();
	NiPoint3 max = TERRAIN->GetBoundBox().GetMax();
	NiPoint3 center = (min + max) * 0.5f;
	NiPoint3 pos = center;
	pos.z += 100000.0f;
	float radius = center.x - min.x;

	cam->SetOrtho( true );
	cam->SetFarDistance( 120000.0f );
	cam->SetViewFrustum( -radius, radius, radius, -radius, cam->GetNearDistance(), cam->GetFarDistance() );
	cam->SetTranslate( pos );
	cam->LookAt( center );
}

void cTerrainEditor::ClearHeightMap()
{
	TERRAIN->ClearHeightMap();
}

bool cTerrainEditor::ImportHeightMap( const cString& pathName )
{
	cString ext;
	::GetFileExtension( &ext, pathName );

	if( ext == "tga" )
	{
		cTerrainImportDialog dlg;

		if( dlg.DoModal() != IDOK )
			return false;

		if( TERRAIN->ImportHeightMap( pathName, dlg.GetScale() ) == false )
			return false;
	}
	else if( ext == "hmap" )
	{
		if( TERRAIN->ImportHeightMap( pathName ) == false )
			return false;
	}
	else
	{
		return false;
	}

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
	return true;
}

bool cTerrainEditor::ExportHeightMap( const cString& pathName )
{
	return TERRAIN->ExportHeightMap( pathName );
}

void cTerrainEditor::ClearAlphaMap()
{
	TERRAIN->ClearAlphaMap();
}

bool cTerrainEditor::ImportAlphaMap( const cString& pathName )
{
	if( TERRAIN->ImportAlphaMap( pathName ) == false )
		return false;

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
	return true;
}

bool cTerrainEditor::ExportAlphaMap( const cString& pathName )
{
	return TERRAIN->ExportAlphaMap( pathName );
}

void cTerrainEditor::ClearColorMap()
{
	TERRAIN->ClearColorMap();
}

bool cTerrainEditor::ImportColorMap( const cString& pathName )
{
	if( TERRAIN->ImportColorMap( pathName ) == false )
		return false;

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
	return true;
}

bool cTerrainEditor::ExportColorMap( const cString& pathName )
{
	return TERRAIN->ExportColorMap( pathName );
}

void cTerrainEditor::AdjustHeight( float scale )
{
	/// 실행 기록을 지움
	DOINGMAN->ClearTerrain();

	/// 스케일을 조절
	TERRAIN->AdjustHeight( scale );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

void cTerrainEditor::SyncAllToNaviMesh()
{
	/// 실행 기록을 지움
	DOINGMAN->ClearTerrain();

	/// 네비메시와 동기화
	TERRAIN->SyncAllToNaviMesh();

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

void cTerrainEditor::SetVisible( bool visible )
{
	if( mGroup->IsEmpty() )
		return;

	/// 선택된 리프 노드들에 가시 여부를 적용
	mGroup->SetVisible( visible );

	// 뷰를 갱신
	VIEW->Update();

	// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

bool cTerrainEditor::LoadTexture( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	if( TERRAIN->LoadTexture( pathName ) == false )
		return false;

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();
	return true;
}

void cTerrainEditor::UnLoadTexture( unsigned int index )
{
	TERRAIN->UnLoadTexture( index );

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();

	// 뷰를 갱신
	VIEW->Update();

	// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

void cTerrainEditor::SetDetailTextures( unsigned int index0, unsigned int index1, unsigned int index2 )
{
	if( mGroup->IsEmpty() )
		return;

	/// 선택된 리프 노드들에 디테일 텍스처를 적용
	mGroup->SetDetailTextures( index0, index1, index2 );

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();

	// 뷰를 갱신
	VIEW->Update();

	// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

void cTerrainEditor::SetDetailTexturesToAll( unsigned int index0, unsigned int index1, unsigned int index2 )
{
	/// 모든 리프 노드들에 디테일 텍스처를 적용
	TERRAIN->SetDetailTexturesToAll( index0, index1, index2 );

	/// 색칠 다이얼로그를 갱신
	mPaintDialog->UpdateTextures();

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( true );
}

void cTerrainEditor::SetViewMode( eTerrainViewMode viewMode )
{
	TERRAIN->SetViewMode( viewMode );
}

void cTerrainEditor::SetLodEnabled( bool enabled )
{
	TERRAIN->SetLodEnabled( enabled );
}

void cTerrainEditor::SetAmbientLightAmbient( const NiColor& color )
{
	TERRAIN->SetAmbientLightAmbient( color );
}

void cTerrainEditor::SetAmbientLightDiffuse( const NiColor& color )
{
	TERRAIN->SetAmbientLightDiffuse( color );
}

void cTerrainEditor::SetAmbientLightDimmer( float dimmer )
{
	TERRAIN->SetAmbientLightDimmer( dimmer );
}

void cTerrainEditor::SetFog( bool enabled, const NiColor& color, float depth )
{
	TERRAIN->SetFog( enabled, color, depth );
}

void cTerrainEditor::SetFogColor( const NiColor& color )
{
	TERRAIN->SetFogColor( color );
}

void cTerrainEditor::SetFogDepth( float depth )
{
	TERRAIN->SetFogDepth( depth );
}

NiAmbientLight* cTerrainEditor::GetAmbientLight() const
{
	return TERRAIN->GetAmbientLight();
}

unsigned int cTerrainEditor::GetResolution() const
{
	return TERRAIN->GetCellCount();
}

float cTerrainEditor::GetMetersPerVertex() const
{
	return TERRAIN->GetMetersPerVertex();
}

unsigned int cTerrainEditor::GetUnitsPerMeter() const
{
	return TERRAIN->GetUnitsPerMeter();
}

void cTerrainEditor::OnTimer_Build()
{
	if( mPickPos != mPickOldPos )
	{
		mPickPosChanged = true;
		mBuildDialog->SetPickPos( mPickPos );
	}
	else if( mDrawing )
	{
		mPickPosChanged = true;
	}

	if( mDrawing && mPickPos != NiPoint3::ZERO )
	{
		switch( mBuildDialog->GetCheckedButton() )
		{
		case 0:
			return;
		case IDC_BUTTON_TBUILD_RAISE:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float strength = mBuildDialog->GetStrength() / 100.0f;

				TERRAIN->Raise( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_LOWER:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float strength = mBuildDialog->GetStrength() / 100.0f;

				TERRAIN->Lower( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_FLATTEN:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float strength = mBuildDialog->GetStrength() / 100.0f;

				TERRAIN->Flatten( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_SMOOTH:
			{
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float ratio = mBuildDialog->GetSmoothRatio() / 100.0f;

				TERRAIN->Smooth( mPickPos, outerRadius, ratio );
			}
			break;
		case IDC_BUTTON_TBUILD_SYNC:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;

				TERRAIN->SyncToNaviMesh( mPickPos, innerRadius, outerRadius );
			}
			break;
		case IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;

				TERRAIN->SyncToPickHeight( mPickPos, innerRadius, outerRadius, mPickedArray[0]->GetPickPos().z );
				mDrawing = false;
			}
			break;
		default:
			assert( 0 && "invalid terrain build type" );
			break;
		}
	}

	/// 변경 여부를 설정
	MAIN->SetTerrainModified( TERRAIN->IsModified() );
}

void cTerrainEditor::OnTimer_Paint()
{
	if( mPickPos != mPickOldPos )
	{
		mPickPosChanged = true;
		mBuildDialog->SetPickPos( mPickPos );
	}
	else if( mDrawing )
	{
		mPickPosChanged = true;
	}

	if( mDrawing && mPickPos != NiPoint3::ZERO )
	{
		switch( mPaintDialog->GetCheckedButton() )
		{
		case 0:
			return;
		case IDC_BUTTON_TPAINT_PAINT_ALPHA:
			{
				float innerRadius = mPaintDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				NiPoint3 alpha;
				alpha.x = mPaintDialog->GetAlpha0() / 100.0f;
				alpha.y = mPaintDialog->GetAlpha1() / 100.0f;
				alpha.z = mPaintDialog->GetAlpha2() / 100.0f;
				bool applyDetailTex = mPaintDialog->GetApplyDetailTex();
				unsigned int texIndex0 = mPaintDialog->GetDetailTexIndex0();
				unsigned int texIndex1 = mPaintDialog->GetDetailTexIndex1();
				unsigned int texIndex2 = mPaintDialog->GetDetailTexIndex2();

				TERRAIN->PaintAlpha( mPickPos, innerRadius, outerRadius, alpha, applyDetailTex, texIndex0, texIndex1, texIndex2 );
			}
			break;
		case IDC_BUTTON_TPAINT_BLUR_ALPHA:
			{
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				float ratio = mPaintDialog->GetBlurRatio() / 100.0f;

				TERRAIN->BlurAlpha( mPickPos, outerRadius, ratio );
			}
			break;
		case IDC_BUTTON_TPAINT_SHARPEN_ALPHA:
			{
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				float ratio = mPaintDialog->GetBlurRatio() / 100.0f;

				TERRAIN->SharpenAlpha( mPickPos, outerRadius, ratio );
			}
			break;
		case IDC_BUTTON_TPAINT_COLOR:
			{
				float innerRadius = mPaintDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				COLORREF color = mPaintDialog->GetColor();
				float red = GetRValue( color ) / 255.0f;
				float green = GetGValue( color ) / 255.0f;
				float blue = GetBValue( color ) / 255.0f;

				TERRAIN->Color( mPickPos, innerRadius, outerRadius, NiColor(red, green, blue) );
			}
			break;
		case IDC_BUTTON_TPAINT_BRUSH:
			{
				float innerRadius = mPaintDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				COLORREF color = mPaintDialog->GetColor();
				float red = GetRValue( color ) / 255.0f;
				float green = GetGValue( color ) / 255.0f;
				float blue = GetBValue( color ) / 255.0f;
				float opacity = mPaintDialog->GetOpacity() / 100.0f;

				TERRAIN->Brush( mPickPos, innerRadius, outerRadius, NiColor(red, green, blue), opacity );
			}
			break;
		case IDC_BUTTON_TPAINT_BLUR_COLOR:
			{
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
				float ratio = mPaintDialog->GetBlurRatio() / 100.0f;

				TERRAIN->BlurColor( mPickPos, outerRadius, ratio );
			}
			break;
		case IDC_BUTTON_TPAINT_LIGHTER:
			{
				float innerRadius = mPaintDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;

				TERRAIN->Lighter( mPickPos, innerRadius, outerRadius );
			}
			break;
		case IDC_BUTTON_TPAINT_DARKER:
			{
				float innerRadius = mPaintDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;

				TERRAIN->Darker( mPickPos, innerRadius, outerRadius );
			}
			break;
		case IDC_BUTTON_TPAINT_PICK:
			{
				if( MOUSE->IsLButtonDown() )
				{
					if( KEY->IsDown(KEY_CONTROL) )
					{
						cTerrainLeafNode* n = 0;
						float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
						float minx = mPickPos.x - outerRadius;
						float maxx = mPickPos.x + outerRadius;
						float miny = mPickPos.y - outerRadius;
						float maxy = mPickPos.y + outerRadius;

						for( float y = miny; y <= maxy; y += 100.0f )
						{
							for( float x = minx; x <= maxx; x += 100.0f )
							{
								n = TERRAIN->GetLeafNode( x, y );
								if( n )
									mGroup->Add( n );
							}
						}

						if( n )
							mPaintDialog->UpdateTileGroup( mGroup );
					}
					else if( KEY->IsDown(KEY_SHIFT) )
					{
						cTerrainLeafNode* n = 0;
						float outerRadius = mPaintDialog->GetOuterRadius() * 100.0f;
						float minx = mPickPos.x - outerRadius;
						float maxx = mPickPos.x + outerRadius;
						float miny = mPickPos.y - outerRadius;
						float maxy = mPickPos.y + outerRadius;

						for( float y = miny; y <= maxy; y += 100.0f )
						{
							for( float x = minx; x <= maxx; x += 100.0f )
							{
								n = TERRAIN->GetLeafNode( x, y );
								if( n )
									mGroup->Remove( n );
							}
						}

						if( n )
							mPaintDialog->UpdateTileGroup( mGroup );
					}
				}
			}
			break;
		default:
			assert( 0 && "invalid terrain paint type" );
			break;
		}

		/// 변경 여부를 설정
		MAIN->SetTerrainModified( TERRAIN->IsModified() );
	}
}

void cTerrainEditor::OnMouseMove_Build( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_I) )
		{
			float r = mBuildDialog->GetInnerRadius();
			r += float((point.x - mMousePos.x) - (point.y - mMousePos.y)) / 10.0f;

			if( r < 0.5f )
				r = 0.5f;
			else if( r > 300.0f )
				r = 300.0f;

			mPickPosChanged = true;
			mMousePos = point;
			mBuildDialog->UpdateInnerRadius( r );
			VIEW->Update();
			return;
		}
		if( KEY->IsDown(KEY_O) )
		{
			float r = mBuildDialog->GetOuterRadius();
			r += float((point.x - mMousePos.x) - (point.y - mMousePos.y)) / 10.0f;

			if( r < 0.5f )
				r = 0.5f;
			else if( r > 300.0f )
				r = 300.0f;

			mPickPosChanged = true;
			mMousePos = point;
			mBuildDialog->UpdateOuterRadius( r );
			VIEW->Update();
			return;
		}
	}

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ))
	{
		mPickOldPos = mPickPos;
		mPickPos = pos;
	}
	else
	{
		mDrawing = false;
		mPickOldPos = mPickPos = NiPoint3::ZERO;
	}

	mMousePos = point;
}

void cTerrainEditor::OnMouseMove_Paint( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_I) )
		{
			float r = mPaintDialog->GetInnerRadius();
			r += float((point.x - mMousePos.x) - (point.y - mMousePos.y)) / 10.0f;

			if( r < 0.5f )
				r = 0.5f;
			else if( r > 300.0f )
				r = 300.0f;

			mPickPosChanged = true;
			mMousePos = point;
			mPaintDialog->UpdateInnerRadius( r );
			VIEW->Update();
			return;
		}
		if( KEY->IsDown(KEY_O) )
		{
			float r = mPaintDialog->GetOuterRadius();
			r += float((point.x - mMousePos.x) - (point.y - mMousePos.y)) / 10.0f;

			if( r < 0.5f )
				r = 0.5f;
			else if( r > 300.0f )
				r = 300.0f;

			mPickPosChanged = true;
			mMousePos = point;
			mPaintDialog->UpdateOuterRadius( r );
			VIEW->Update();
			return;
		}
	}

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ))
	{
		mPickOldPos = mPickPos;
		mPickPos = pos;
	}
	else
	{
		mDrawing = false;
		mPickOldPos = mPickPos = NiPoint3::ZERO;
	}

	mMousePos = point;
}

void cTerrainEditor::OnLButtonDown_Build( CPoint point )
{
	if( KEY->IsDown(KEY_I) || KEY->IsDown(KEY_O) )
	{
		mMousePos = point;
		return;
	}

	switch( mBuildDialog->GetCheckedButton() )
	{
	case 0:
		return;
	case IDC_BUTTON_TBUILD_RAISE:
	case IDC_BUTTON_TBUILD_LOWER:
	case IDC_BUTTON_TBUILD_FLATTEN:
	case IDC_BUTTON_TBUILD_SMOOTH:
	case IDC_BUTTON_TBUILD_SYNC:
		{
			NiPoint3 pos;

			if( TERRAIN->Pick( &pos, point.x, point.y ) )
			{
				mDrawing = true;
				mPickOldPos = mPickPos;
				mPickPos = pos;
				return;
			}
		}
		break;
	case IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS:
		mPickedArray.Clear();

		if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
		{
			cSceneNode* n = mPickedArray[0];
			NiPoint3 origin = n->GetPickPos();
			origin.z = 100000.0f;
			NiPoint3 pos;

			if( TERRAIN->Pick( &pos, cRay(origin, -NiPoint3::UNIT_Z) ) )
			{
				mDrawing = true;
				mPickOldPos = mPickPos;
				mPickPos = pos;
				return;
			}
		}
		break;
	default:
		assert( 0 && "invalid terrain build type" );
		break;
	}
	mDrawing = false;
	mPickPos = NiPoint3::ZERO;
}

void cTerrainEditor::OnLButtonDown_Paint( CPoint point )
{
	if( KEY->IsDown(KEY_I) || KEY->IsDown(KEY_O) )
	{
		mMousePos = point;
		return;
	}

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ) )
	{
		mDrawing = true;
		mPickOldPos = mPickPos;
		mPickPos = pos;

		switch( mPaintDialog->GetCheckedButton() )
		{
		case IDC_BUTTON_TPAINT_PICK:
			{
				if( KEY->IsDown(KEY_SHIFT) || KEY->IsDown(KEY_CONTROL) )
					break;

				cTerrainLeafNode* n = TERRAIN->GetLeafNode( pos.x, pos.y );
				if( mGroup->Select( n ) )
					mPaintDialog->UpdateTileGroup( mGroup );

				/// 해당 지점의 지형 색을 픽킹
				NiColor color;

				if( TERRAIN->GetColor( &color, pos ) )
				{
					unsigned char red = (unsigned char)(color.r * 255.0f);
					unsigned char green = (unsigned char)(color.g * 255.0f);
					unsigned char blue = (unsigned char)(color.b * 255.0f);

					mPaintDialog->UpdateColor( red, green, blue );
				}
			}
			break;
		}
	}
	else
	{
		mDrawing = false;
		mPickPos = NiPoint3::ZERO;
	}
}

void cTerrainEditor::OnLButtonUp()
{
	mDrawing = false;
}

void cTerrainEditor::OnKeyDown( UINT c )
{
	switch( c )
	{
	case KEY_F1:
		{
			mOptionDialog->ToggleWireTextured();
			break;
		}
	case KEY_F5:
		{
			mBuildDialog->SetCheckedButton( IDC_BUTTON_TBUILD_RAISE );
			VIEW->SetEditMode( eEDIT_TERRAIN_BUILD );
			break;
		}
	case KEY_F6:
		{
			mBuildDialog->SetCheckedButton( IDC_BUTTON_TBUILD_LOWER );
			VIEW->SetEditMode( eEDIT_TERRAIN_BUILD );
			break;
		}
	case KEY_F7:
		{
			mBuildDialog->SetCheckedButton( IDC_BUTTON_TBUILD_FLATTEN );
			VIEW->SetEditMode( eEDIT_TERRAIN_BUILD );
			break;
		}
	case KEY_F8:
		{
			mBuildDialog->SetCheckedButton( IDC_BUTTON_TBUILD_SMOOTH );
			VIEW->SetEditMode( eEDIT_TERRAIN_BUILD );
			break;
		}
	case KEY_1:
		{
			mPaintDialog->IncreaseAlpha0();
			break;
		}
	case KEY_2:
		{
			mPaintDialog->IncreaseAlpha1();
			break;
		}
	case KEY_3:
		{
			mPaintDialog->IncreaseAlpha2();
			break;
		}
	case KEY_V:
		{
			mPaintDialog->SetCheckedButton( IDC_BUTTON_TPAINT_PICK );
			VIEW->SetEditMode( eEDIT_TERRAIN_PAINT );
		}
	case KEY_N:
		{
			mPaintDialog->SetCheckedButton( IDC_BUTTON_TPAINT_PAINT_ALPHA );
			VIEW->SetEditMode( eEDIT_TERRAIN_PAINT );
			break;
		}
	case KEY_B:
		{
			mPaintDialog->SetCheckedButton( IDC_BUTTON_TPAINT_BLUR_ALPHA );
			VIEW->SetEditMode( eEDIT_TERRAIN_PAINT );
			break;
		}
	case KEY_M:
		{
			mPaintDialog->SetCheckedButton( IDC_BUTTON_TPAINT_SHARPEN_ALPHA );
			VIEW->SetEditMode( eEDIT_TERRAIN_PAINT );
			break;
		}
	}
}
