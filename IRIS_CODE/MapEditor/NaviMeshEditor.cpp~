#include "stdafx.h"
#include "NaviMeshEditor.h"

#include "resource.h"
#include "MainFrame.h"
#include "MapEditorView.h"
#include "NaviMeshBuildDialog.h"
#include "TerrainImportDialog.h"
#include "Hero.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/Ray.h"
#include "Engine/NaviMesh.h"
#include "Engine/NaviMeshNode.h"
#include "Engine/SceneManager.h"
#include "Engine/SceneNode.h"

#include "Doing/DoingManager.h"

cNaviMeshEditor* cNaviMeshEditor::mSingleton = 0;

cNaviMeshEditor::cNaviMeshEditor()
: mBuildDialog( 0 )
, mDrawing( false )
, mPickPos( NiPoint3::ZERO )
, mPickOldPos( NiPoint3::ZERO )
, mPickPosChanged( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	/// 네비메시를 생성
	mNaviMesh = new cNaviMesh;

	mPickedArray.Reserve( 1024 );

	mCircleLines = NiNew NiMesh;
	mCircleLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mCircleLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 40, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::WHITE );
	mMatProp->SetDiffuseColor( NiColor::WHITE );
	mMatProp->SetSpecularColor( NiColor::WHITE );
	mMatProp->SetEmittance( NiColor(1.0f, 0.0f, 0.0f) );

	mVertColorProp = NiNew NiVertexColorProperty;
	mVertColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
	mVertColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mCircleLines->AttachProperty( mMatProp );
	mCircleLines->AttachProperty( mVertColorProp );
	mCircleLines->AttachProperty( mWireProp );
	mCircleLines->UpdateProperties();
	mCircleLines->Update( 0.0f );
}

cNaviMeshEditor::~cNaviMeshEditor()
{
	delete mNaviMesh;

	mSingleton = 0;
}

void cNaviMeshEditor::Clear()
{
	mDrawing = false;
	mPickOldPos = mPickPos = NiPoint3::ZERO;
	mPickPosChanged = false;
	mPathName.Clear();
}

void cNaviMeshEditor::Process()
{
	mNaviMesh->Process();
}

void cNaviMeshEditor::Render()
{
	mNaviMesh->Render();

	if( mPickPos != NiPoint3::ZERO )
	{
		if( mPickPosChanged )
		{
			mPickPosChanged = false;
			float ir = float(mBuildDialog->GetInnerRadius() * mNaviMesh->GetUnitsPerMeter());
			float or = float(mBuildDialog->GetOuterRadius() * mNaviMesh->GetUnitsPerMeter());

			float x, y;
			NiPoint3 org;
			NiPoint3 pos;

			NiDataStreamElementLock kPositionLock( mCircleLines, NiCommonSemantics::POSITION(), 0,
				NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
			assert( kPositionLock.IsLocked() );

			NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

			for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 36 )
			{
				x = NiCos( ang * NI_PI / 180.0f );
				y = NiSin( ang * NI_PI / 180.0f );

				unsigned int s = (2*i)+1;
				unsigned int e = (2*(i+1))%20;


				/// 안쪽 원
				pos.x = org.x = mPickPos.x + x * ir;
				pos.y = org.y = mPickPos.y + y * ir;
				org.z = 1000000.0f;
				pos.z = 0.0f;

				mNaviMesh->Pick( &pos, cRay(org, -NiPoint3::UNIT_Z) );

				kPositionIter[s].x = kPositionIter[e].x = pos.x;
				kPositionIter[s].y = kPositionIter[e].y = pos.y;
				kPositionIter[s].z = kPositionIter[e].z = pos.z + 10.0f;

				/// 바깥쪽 원
				pos.x = org.x = mPickPos.x + x * or;
				pos.y = org.y = mPickPos.y + y * or;
				org.z = 1000000.0f;
				pos.z = 0.0f;

				mNaviMesh->Pick( &pos, cRay(org, -NiPoint3::UNIT_Z) );

				kPositionIter[s+20].x = kPositionIter[e+20].x = pos.x;
				kPositionIter[s+20].y = kPositionIter[e+20].y = pos.y;
				kPositionIter[s+20].z = kPositionIter[e+20].z = pos.z + 10.0f;
			}

			kPositionLock.Unlock();
		}

		mCircleLines->RenderImmediate( NiRenderer::GetRenderer() );
	}
}

void cNaviMeshEditor::Init( unsigned int resolution, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();
	mNaviMesh->Init( resolution, metersPerVertex, unitsPerMeter );

	/// 실행 기록을 지움
	DOINGMAN->ClearNaviMesh();

	/// 주인공의 위치를 초기화
	HERO->Reset();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	MAIN->SetNaviMeshModified( false );
}

bool cNaviMeshEditor::Load( const cString& pathName )
{
	Clear();

	/// 실행 기록을 지움
	DOINGMAN->ClearNaviMesh();

	if( mNaviMesh == 0 )
	{
		assert( 0 && "null navimesh" );
		return false;
	}
	
	/// 네비메시를 로딩
	cString str;

	switch( mNaviMesh->Load( pathName ) )
	{
	case NAVIMESH_LOAD_ERROR_OPEN:
		str = "Failed to open navimesh file.";
		break;
	case NAVIMESH_LOAD_ERROR_FILE_HEADER:
		str = "Failed to load navimesh file header.";
		break;
	case NAVIMESH_LOAD_ERROR_FILE_TYPE:
		str = "Invalid navimesh file type.";
		break;
	case NAVIMESH_LOAD_ERROR_FILE_VERSION:
		str = "Invalid navimesh file version.";
		break;
	case NAVIMESH_LOAD_ERROR_GRID_SIZE:
		str = "Invalid navimesh grid size.";
		break;
	case NAVIMESH_LOAD_ERROR_TEXTURE_NAME:
		str = "Failed to load navimesh texture name.";
		break;
	case NAVIMESH_LOAD_ERROR_TEXTURE:
		str = "Failed to load navimesh texture.";
		break;
	case NAVIMESH_LOAD_ERROR_NODE:
		str = "Failed to load navimesh node.";
		break;
	}

	if( str.IsEmpty() == false )
	{
		AfxMessageBox( str.Cstr() );
		return false;
	}

	/// 주인공의 위치를 초기화
	HERO->Reset();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetNaviMeshModified( name, false );
	return true;
}

bool cNaviMeshEditor::Save()
{
	if( mPathName.IsEmpty() )
		return false;
	else
		return SaveAs( mPathName );
}

bool cNaviMeshEditor::SaveAs( const cString& pathName )
{
	if( mNaviMesh->Save( pathName ) )
	{
		/// 경로 이름을 설정
		mPathName = pathName;

		cString name;
		::GetFileName( &name, pathName );
		MAIN->SetNaviMeshModified( name, false );
		return true;
	}
	return false;
}

void cNaviMeshEditor::SyncAllToTerrain()
{
	/// 실행 기록을 지움
	DOINGMAN->ClearNaviMesh();

	/// 지형과 동기화
	mNaviMesh->SyncAllToTerrain();

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetNaviMeshModified( true );
}

unsigned int cNaviMeshEditor::GetResolution() const
{
	return mNaviMesh->GetCellCount();
}

float cNaviMeshEditor::GetMetersPerVertex() const
{
	return mNaviMesh->GetMetersPerVertex();
}

void cNaviMeshEditor::OnTimer_Build()
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

				mNaviMesh->Raise( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_LOWER:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float strength = mBuildDialog->GetStrength() / 100.0f;

				mNaviMesh->Lower( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_FLATTEN:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float strength = mBuildDialog->GetStrength() / 100.0f;

				mNaviMesh->Flatten( mPickPos, innerRadius, outerRadius, strength );
			}
			break;
		case IDC_BUTTON_TBUILD_SMOOTH:
			{
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;
				float ratio = mBuildDialog->GetSmoothRatio() / 100.0f;

				mNaviMesh->Smooth( mPickPos, outerRadius, ratio );
			}
			break;
		case IDC_BUTTON_TBUILD_SYNC:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;

				mNaviMesh->SyncToTerrain( mPickPos, innerRadius, outerRadius );
			}
			break;
		case IDC_BUTTON_TBUILD_SYNC_TO_OBJECT:
			{
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;

				mNaviMesh->SyncToObject( mPickPos, outerRadius );
				mDrawing = false;
			}
			break;
		case IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS:
			{
				float innerRadius = mBuildDialog->GetInnerRadius() * 100.0f;
				float outerRadius = mBuildDialog->GetOuterRadius() * 100.0f;

				mNaviMesh->SyncToPickHeight( mPickPos, innerRadius, outerRadius, mPickedArray[0]->GetPickPos().z );
				mDrawing = false;
			}
			break;
		default:
			assert( 0 && "invalid navimesh build type" );
			break;
		}
		
		/// 변경 여부를 설정
		MAIN->SetNaviMeshModified( mNaviMesh->IsModified() );
	}
}

void cNaviMeshEditor::OnMouseMove( CPoint point )
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

	if( mNaviMesh->Pick( &pos, point.x, point.y ))
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

void cNaviMeshEditor::OnLButtonDown_Build( CPoint point )
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

			if( mNaviMesh->Pick( &pos, point.x, point.y ) )
			{
				mDrawing = true;
				mPickOldPos = mPickPos;
				mPickPos = pos;
				return;
			}
		}
		break;
	case IDC_BUTTON_TBUILD_SYNC_TO_OBJECT:
	case IDC_BUTTON_TBUILD_SYNC_TO_PICKPOS:
		mPickedArray.Clear();

		if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
		{
			cSceneNode* n = mPickedArray[0];
			NiPoint3 origin = n->GetPickPos();
			origin.z = 100000.0f;
			NiPoint3 pos;

			if( mNaviMesh->Pick( &pos, cRay(origin, -NiPoint3::UNIT_Z) ) )
			{
				mDrawing = true;
				mPickOldPos = mPickPos;
				mPickPos = pos;
				return;
			}
		}
		break;
	default:
		assert( 0 && "invalid navimesh build type" );
		break;
	}
	mDrawing = false;
	mPickPos = NiPoint3::ZERO;
}

void cNaviMeshEditor::OnLButtonUp()
{
	mDrawing = false;
}
