#include "stdafx.h"
#include "AreaEditor.h"

#include "resource.h"
#include "MainFrame.h"
#include "TabWindow.h"
#include "MapEditorView.h"
#include "AreaTransformDialog.h"
#include "AreaLightDialog.h"
#include "AreaFogDialog.h"
#include "AreaEtcDialog.h"
#include "TerrainEditor.h"
#include "ObjectEditor.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/CameraManager.h"
#include "Engine/FreeCamera.h"
#include "Engine/WorldManager.h"
#include "Engine/SceneManager.h"
#include "Engine/AreaSceneNode.h"
#include "Engine/Terrain.h"

cAreaEditor* cAreaEditor::mSingleton = 0;

cAreaEditor::cAreaEditor()
: mTransformDialog( 0 )
, mLightDialog( 0 )
, mFogDialog( 0 )
, mEtcDialog( 0 )
, mCreatedSet( 64 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mSelectedNode = SCENEMAN->GetGlobalArea();

	mPickedArray.Reserve( 16 );

	NiDataStream::GetFactory()->SetCallback( NiDataStreamFactory::ForceCPUReadAccessCallback );

	/// 경계 구를 로딩
	NiStream stream;

	if( stream.Load( "MapData/Sphere.nif" ) )
	{
		NiNode* node = NiDynamicCast(NiNode, stream.GetObjectAt(0));
		assert( node );

		NiMesh* geom = (NiMesh*)node->GetAt( 0 );
		assert( geom );

		NiMaterialProperty* m = (NiMaterialProperty*)geom->GetProperty( NiProperty::MATERIAL );
		assert( m && "null material property" );
		mMatProp = m;
		m->SetAmbientColor( NiColor::WHITE );
		m->SetDiffuseColor( NiColor::WHITE );
		m->SetSpecularColor( NiColor::WHITE );
		m->SetEmittance( NiColor::BLACK );

		NiVertexColorProperty* vp = (NiVertexColorProperty*)geom->GetProperty( NiProperty::VERTEX_COLOR );
		assert( vp && "null vertex color property" );
		vp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
		vp->SetLightingMode( NiVertexColorProperty::LIGHTING_E );

		NiWireframeProperty* wp = (NiWireframeProperty*)geom->GetProperty( NiProperty::WIREFRAME );
		assert( wp && "null wireframe property" );
		wp->SetWireframe( true );

		geom->UpdateProperties();
		geom->Update( 0.0f );

		mSphereGeom = geom;
		stream.RemoveAllObjects();
	}
}

cAreaEditor::~cAreaEditor()
{
	mSingleton = 0;
}

void cAreaEditor::Clear()
{
	mCreatedSet.Clear();
	mPickedArray.Clear();

	if( mTransformDialog )
		mTransformDialog->SetEnabled( false );
}

void cAreaEditor::Render()
{
	if( mSphereGeom == 0 )
		return;

	NiRenderer* renderer = NiRenderer::GetRenderer();

	cCamera* cam = CAMERAMAN->GetCurrent();
	NiPoint3 right = cam->GetWorldRightVector();
	NiPoint3 up = cam->GetWorldUpVector();

	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cAreaSceneNode* n = *i;

		if( n == mSelectedNode )
		{
			mMatProp->SetEmittance( NiColor(1.0f, 0.0f, 0.0f) );
			mSphereGeom->UpdateProperties();
		}
		else if( mMatProp->GetEmittance() != NiColor::BLACK )
		{
			mMatProp->SetEmittance( NiColor::BLACK );
			mSphereGeom->UpdateProperties();
		}

		mSphereGeom->SetTranslate( n->GetCenter() );
		mSphereGeom->SetScale( n->GetRadius() / 100.0f );
		mSphereGeom->Update( 0.0f, false );

		mSphereGeom->RenderImmediate( renderer );
	}
}

void cAreaEditor::Init()
{
	/// 지움
	Clear();

	/// 초기화
	//SelectArea( 0, true );
}

bool cAreaEditor::Save( cFileSaver& saver )
{
	/// 장면 노드들을 저장
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		if( (*i)->Save( saver ) == false )
		{
			assert( 0 && "failed to save area nodes" );
			return false;
		}
	}
	return true;
}

bool cAreaEditor::CreateArea()
{
	cAreaSceneNodeParam param;
	cAreaSceneNode* n = SCENEMAN->CreateArea( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );

		/// 생성된 오브젝트를 선택
		SelectArea( n );

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
	return n != 0;
}

void cAreaEditor::CopyArea()
{
	if( mSelectedNode == 0 )
		return;

	/// 노드를 복사
	cAreaSceneNode* n = Clone( mSelectedNode );
	if( n == 0 )
	{
		assert( 0 );
		return;
	}

	/// 원본 그룹 위치 이동
	SetAreaTranslate( mSelectedNode->GetTranslate() + NiPoint3(0.0f, 0.0f, 300.0f), true );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

bool cAreaEditor::Pick( CPoint point )
{
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_AREA ) )
	{
		SelectArea( (cAreaSceneNode*)mPickedArray[0] );
		return true;
	}
	else
	{
		DeselectAll();
		return false;
	}
}

void cAreaEditor::DeselectAll()
{
	if( mCreatedSet.IsEmpty() == false && mSelectedNode != SCENEMAN->GetGlobalArea() )
	{
		SelectArea( 0 );
	}
}

void cAreaEditor::SelectArea( cAreaSceneNode* n, bool force )
{
	if( n == 0 )
		n = SCENEMAN->GetGlobalArea();

	if( force == false && mSelectedNode == n )
		return;
	else
		mSelectedNode = n;

	///
	WORLDMAN->SetArea( *n );

	/// 다이얼로그를 갱신
	if( mTransformDialog )
	{
		if( n == SCENEMAN->GetGlobalArea() )
		{
			mTransformDialog->SetEnabled( false );
		}
		else
		{
			mTransformDialog->UpdateArea( n->GetWorldTranslate(), n->GetRadius() );
		}
	}

	if( mLightDialog )
	{
		mLightDialog->UpdateSky( n->GetSkyAmbient(), n->GetSkyDimmer() );
		mLightDialog->UpdateTerrain( n->GetTerrainAmbient(), n->GetTerrainDimmer() );
		mLightDialog->UpdateStaticObject( n->GetStaticObjectAmbient(), n->GetStaticObjectDimmer() );
		mLightDialog->UpdateDynamicObject( n->GetDynamicObjectAmbient(), n->GetDynamicObjectDimmer() );
	}

	if( mFogDialog )
		mFogDialog->Update( n->IsFogEnabled(), n->GetFogColor(), n->GetFogDepth() );
	if( mEtcDialog )
		mEtcDialog->Update( n->GetGroupIndex() );

	/// 주변광을 적용
	WORLDMAN->SetSkyAmbient( n->GetSkyAmbient() );
	WORLDMAN->SetSkyDimmer( n->GetSkyDimmer() );
	TERRAINEDIT->SetAmbientLightAmbient( n->GetTerrainAmbient() );
	TERRAINEDIT->SetAmbientLightDimmer( n->GetTerrainDimmer() );
	OBJECTEDIT->SetAmbientLightAmbient( 0, n->GetStaticObjectAmbient() );
	OBJECTEDIT->SetAmbientLightDimmer( 0, n->GetStaticObjectDimmer() );
	OBJECTEDIT->SetAmbientLightAmbient( 1, n->GetDynamicObjectAmbient() );
	OBJECTEDIT->SetAmbientLightDimmer( 1, n->GetDynamicObjectDimmer() );

	/// 안개를 적용
	TERRAINEDIT->SetFog( n->IsFogEnabled(), n->GetFogColor(), n->GetFogDepth() );
	OBJECTEDIT->SetFog( n->IsFogEnabled(), n->GetFogColor(), n->GetFogDepth() );

	/// 뷰를 갱신
	VIEW->Update();
}

void cAreaEditor::AttachAreaToTerrain( CPoint point )
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ) )
	{
		SetAreaTranslate( pos, true );
	}
}

void cAreaEditor::AttachAreaToObject( CPoint point )
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
	{
		cSceneNode* n = mPickedArray[0];
		SetAreaTranslate( n->GetPickPos(), true );
	}
}

void cAreaEditor::SetAreaTranslate( const NiPoint3& trans, bool updateDialog )
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	mSelectedNode->SetTranslate( trans );
	mSelectedNode->Update();

	if( updateDialog )
		mTransformDialog->UpdateArea( mSelectedNode->GetWorldTranslate(), mSelectedNode->GetRadius() );

	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetAreaRadius( float radius, bool updateDialog )
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

	if( radius < 600.0f )
		radius = 600.0f;
	else if( radius > 20000.0f )
		radius = 20000.0f;
	else if( radius > maxr )
		radius = maxr;

	mSelectedNode->SetRadius( radius );
	mSelectedNode->Update();

	if( updateDialog )
		mTransformDialog->UpdateArea( mSelectedNode->GetWorldTranslate(), mSelectedNode->GetRadius() );

	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetAreaGroupIndex( unsigned int groupIndex )
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	mSelectedNode->SetGroupIndex( groupIndex );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

bool cAreaEditor::Kill( cAreaSceneNode* node )
{
	/// 생성 셑에서 제거
	mCreatedSet.Erase( node );

	/// 장면 관리자로부터 제거
	SCENEMAN->DestroyNode( node );
	//SCENEMAN->DestroyArea( node );
	return true;
}

cAreaSceneNode* cAreaEditor::Clone( cAreaSceneNode* node )
{
	cAreaSceneNodeParam param;
	param.mTranslate = node->GetWorldTranslate();
	param.mRotate = node->GetWorldRotate();
	param.mScale = node->GetWorldScale();
	param.mRadius = node->mRadius;
	param.mSkyAmbient = node->mSkyAmbient;
	param.mSkyDimmer = node->mSkyDimmer;
	param.mTerrainAmbient = node->mTerrainAmbient;
	param.mTerrainDimmer = node->mTerrainDimmer;
	param.mStaticObjectAmbient = node->mStaticObjectAmbient;
	param.mStaticObjectDimmer = node->mStaticObjectDimmer;
	param.mDynamicObjectAmbient = node->mDynamicObjectAmbient;
	param.mDynamicObjectDimmer = node->mDynamicObjectDimmer;
	param.mFogEnabled = node->mFogEnabled;
	param.mFogColor = node->mFogColor;
	param.mFogDepth = node->mFogDepth;
	param.mGroupIndex = node->mGroupIndex;

	cAreaSceneNode* n = SCENEMAN->CreateArea( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );
	}
	return n;
}

void cAreaEditor::DeleteArea()
{
	if( mSelectedNode == 0 || mSelectedNode == SCENEMAN->GetGlobalArea() )
		return;

	Kill( mSelectedNode );
	SelectArea( 0 );

	mTransformDialog->SetEnabled( false );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetSkyAmbient( const NiColor& color )
{
	mSelectedNode->SetSkyAmbient( color );
	WORLDMAN->SetSkyAmbient( color );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetSkyDimmer( float dimmer )
{
	mSelectedNode->SetSkyDimmer( dimmer );
	WORLDMAN->SetSkyDimmer( dimmer );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetTerrainAmbient( const NiColor& color )
{
	mSelectedNode->SetTerrainAmbient( color );
	TERRAINEDIT->SetAmbientLightAmbient( color );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetTerrainDimmer( float dimmer )
{
	mSelectedNode->SetTerrainDimmer( dimmer );
	TERRAINEDIT->SetAmbientLightDimmer( dimmer );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetStaticObjectAmbient( const NiColor& color )
{
	mSelectedNode->SetStaticObjectAmbient( color );
	OBJECTEDIT->SetAmbientLightAmbient( 0, color );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetStaticObjectDimmer( float dimmer )
{
	mSelectedNode->SetStaticObjectDimmer( dimmer );
	OBJECTEDIT->SetAmbientLightDimmer( 0, dimmer );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetDynamicObjectAmbient( const NiColor& color )
{
	mSelectedNode->SetDynamicObjectAmbient( color );
	OBJECTEDIT->SetAmbientLightAmbient( 1, color );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetDynamicObjectDimmer( float dimmer )
{
	mSelectedNode->SetDynamicObjectDimmer( dimmer );
	OBJECTEDIT->SetAmbientLightDimmer( 1, dimmer );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetFog( bool enabled, const NiColor& color, float depth )
{
	cAreaSceneNode* n = mSelectedNode;
	n->SetFogEnabled( enabled );
	n->SetFogColor( color );
	n->SetFogDepth( depth );
	TERRAINEDIT->SetFog( enabled, color, n->GetFogDepth() );
	OBJECTEDIT->SetFog( enabled, color, n->GetFogDepth() );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetFogColor( const NiColor& color )
{
	mSelectedNode->SetFogColor( color );
	SelectArea( mSelectedNode, true );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::SetFogDepth( float depth )
{
	cAreaSceneNode* n = mSelectedNode;
	n->SetFogDepth( depth );
	TERRAINEDIT->SetFogDepth( depth );
	OBJECTEDIT->SetFogDepth( depth );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cAreaEditor::OnMouseMove( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_X) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.x += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetAreaTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.y += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetAreaTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Z) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.z -= (point.y - mMousePos.y) * 10.0f;
			SetAreaTranslate( t, true );
		}
		if( KEY->IsDown(KEY_R) )
		{
			float r = mTransformDialog->GetRadius();
			r += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;

			SetAreaRadius( r, true );
		}
	}

	mMousePos = point;
}

void cAreaEditor::OnLButtonDown_Transform( CPoint point )
{
	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) || KEY->IsDown(KEY_R) )
	{
		mMousePos = point;
		return;
	}

	switch( mTransformDialog->GetCheckedButton() )
	{
	case IDC_BUTTON_LFORM_PICK:
		{
			Pick( point );
		}
		break;
	case IDC_BUTTON_LFORM_TO_TERRAIN:
		{
			AttachAreaToTerrain( point );
		}
		break;
	case IDC_BUTTON_LFORM_TO_OBJECT:
		{
			AttachAreaToObject( point );
		}
		break;
	}
}

void cAreaEditor::OnRButtonDown_Transform( CPoint point, bool ctrl )
{
	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) )
	{
		mMousePos = point;
		return;
	}

	if( ctrl )
	{
		cCamera* cam = CAMERAMAN->GetCamera(0);
		NiPoint3 camPos = cam->GetWorldTranslate();
		NiPoint3 pos;
		mPickedArray.Clear();

		bool picked = TERRAIN->Pick( &pos, point.x, point.y );

		if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
		{
			cSceneNode* n = (cSceneNode*)mPickedArray[0];
			const NiPoint3& pickPos = n->GetPickPos();

			if( picked )
			{
				if( (pickPos-camPos).Length() < (pos-camPos).Length() )
					pos = pickPos;
			}
			else
			{
				picked = true;
				pos = pickPos;
			}
		}

		if( picked )
		{
			SetAreaTranslate( pos, true );
		}
	}
}

void cAreaEditor::OnKeyDown( UINT c )
{
	switch( c )
	{
	case KEY_DELETE:
		{
			if( mSelectedNode == 0 )
				return;

			int mb = MessageBox( 0, "Do you want to delete selected area?", "Area", MB_YESNO );

			if( mb == IDYES )
			{
				DeleteArea();
			}
		}
		break;
	}
}
