#include "stdafx.h"
#include "LightEditor.h"

#include "resource.h"
#include "MainFrame.h"
#include "MapEditorView.h"
#include "LightTransformDialog.h"
#include "LightPropertyDialog.h"
#include "LightInfoDialog.h"
#include "TerrainEditor.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/CameraManager.h"
#include "Engine/SceneManager.h"
#include "Engine/LightSceneNode.h"
#include "Engine/Terrain.h"

cLightEditor* cLightEditor::mSingleton = 0;

cLightEditor::cLightEditor()
: mTransformDialog( 0 )
, mPropertyDialog( 0 )
, mInfoDialog( 0 )
, mCreatedSet( 2048 )
, mSelectedNode( 0 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mPickedArray.Reserve( 512 );

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
	mMatProp->SetEmittance( NiColor(1.0f, 1.0f, 0.0f) );

	mVertColorProp = NiNew NiVertexColorProperty;
	mVertColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
	mVertColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mCircleLines->AttachProperty( mMatProp );
	mCircleLines->AttachProperty( mVertColorProp );
	mCircleLines->AttachProperty( mWireProp );
	mCircleLines->UpdateProperties();
	mCircleLines->Update( 0.0f );
}

cLightEditor::~cLightEditor()
{
	mSingleton = 0;
}

void cLightEditor::Clear()
{
	mCreatedSet.Clear();
	mPickedArray.Clear();
	mSelectedNode = 0;

	if( mTransformDialog )
		mTransformDialog->SetEnabled( false );
	if( mPropertyDialog )
		mPropertyDialog->SetEnabled( false );
}

void cLightEditor::Render()
{
	if( mCircleLines == 0 )
		return;

	cCamera* cam = CAMERAMAN->GetCurrent();
	NiPoint3 right = cam->GetWorldRightVector();
	NiPoint3 up = cam->GetWorldUpVector();

	NiDataStreamElementLock kPositionLock( mCircleLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();
	for( ; i != end; ++i )
	{
		cLightSceneNode* n = *i;

		const NiPoint3& c = n->GetCenter();
		float r = n->GetRadius();
		float x, y;
		NiPoint3 dir;

		NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

		for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 36 )
		{
			x = NiCos( ang * NI_PI / 180.0f );
			y = NiSin( ang * NI_PI / 180.0f );

			dir = right * x + up * y;

			unsigned int s = (2*i)+1;
			unsigned int e = (2*(i+1))%20;

			/// 안쪽 원
			kPositionIter[s] = c + dir * 50;
			kPositionIter[e] = c + dir * 50;

			/// 바깥쪽 원
			kPositionIter[s+20] = c + dir * r;
			kPositionIter[e+20] = c + dir * r;
		}

		if( n == mSelectedNode )
		{
			mMatProp->SetEmittance( NiColor(1.0f, 0.0f, 0.0f) );
			mCircleLines->UpdateProperties();
		}
		else if( mMatProp->GetEmittance() != NiColor(1.0f, 1.0f, 0.0f) )
		{
			mMatProp->SetEmittance( NiColor(1.0f, 1.0f, 0.0f) );
			mCircleLines->UpdateProperties();
		}
		mCircleLines->RenderImmediate( NiRenderer::GetRenderer() );
	}

	kPositionLock.Unlock();
}

void cLightEditor::Init()
{
	Clear();

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
}

bool cLightEditor::Save( cFileSaver& saver )
{
	/// 장면 노드들을 저장
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		if( (*i)->Save( saver ) == false )
		{
			assert( 0 && "failed to save light nodes" );
			return false;
		}
	}
	return true;
}

bool cLightEditor::CreateLight()
{
	cLightSceneNodeParam param;
	param.mRadius = mTransformDialog->GetRadius();
	cLightSceneNode* n = SCENEMAN->CreateLight( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );

		/// 생성된 오브젝트를 선택
		mSelectedNode = n;
		mTransformDialog->UpdateLight( n->GetWorldTranslate(), n->GetRadius() );
		mPropertyDialog->UpdateLight( n->GetAmbient(),
			n->GetDiffuse0(), n->GetDiffuse1(), n->GetDiffuseAnimTime(),
			n->GetConstantAtten0(), n->GetConstantAtten1(), n->GetConstantAttenAnimTime(),
			n->GetLinearAtten(), n->GetQuadricAtten() );

		/// 조명의 영향을 받는 노드들을 검색
		n->DetachAll();
		mPickedArray.Clear();

		if( SCENEMAN->Pick( &mPickedArray, n->GetBoundSphere(), SCENENODE_STATIC ) )
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				mPickedArray[i]->SetNeedUpdateTransform( true );
			}
		}

		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );

		/// 정보 다이얼로그를 갱신
		mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
	}
	return n != 0;
}

bool cLightEditor::Pick( CPoint point )
{
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_LIGHT ) )
	{
		cLightSceneNode* n = mSelectedNode = (cLightSceneNode*)mPickedArray[0];

		mTransformDialog->UpdateLight( n->GetWorldTranslate(), n->GetRadius() );
		mPropertyDialog->UpdateLight( n->GetAmbient(),
			n->GetDiffuse0(), n->GetDiffuse1(), n->GetDiffuseAnimTime(),
			n->GetConstantAtten0(), n->GetConstantAtten1(), n->GetConstantAttenAnimTime(),
			n->GetLinearAtten(), n->GetQuadricAtten() );
		return true;
	}
	else
	{
		return false;
	}
}

void cLightEditor::DeselectAll()
{
	if( mCreatedSet.IsEmpty() == false )
	{
		mSelectedNode = 0;
		mTransformDialog->SetEnabled( false );
		mPropertyDialog->SetEnabled( false );
	}
}

void cLightEditor::AttachLightToTerrain( CPoint point )
{
	if( mSelectedNode == 0 )
		return;

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ) )
	{
		SetLightTranslate( pos, true );
	}
}

void cLightEditor::AttachLightToObject( CPoint point )
{
	if( mSelectedNode == 0 )
		return;

	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
	{
		cSceneNode* n = mPickedArray[0];

		SetLightTranslate( n->GetPickPos(), true );
	}
}

void cLightEditor::SetLightTranslate( const NiPoint3& trans, bool updateDialog )
{
	if( mSelectedNode == 0 )
		return;

	mSelectedNode->SetTranslate( trans );
	mSelectedNode->Update();

	if( updateDialog )
		mTransformDialog->UpdateLight( mSelectedNode->GetWorldTranslate(), mSelectedNode->GetRadius() );

	/// 조명의 영향을 받는 노드들을 검색
	mSelectedNode->DetachAll();
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, mSelectedNode->GetBoundSphere(), SCENENODE_STATIC ) )
	{
		for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
		{
			mPickedArray[i]->SetNeedUpdateTransform( true );
		}
	}

	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightRadius( float radius, bool updateDialog )
{
	if( mSelectedNode == 0 )
		return;

	float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

	if( radius < 100.0f )
		radius = 100.0f;
	else if( radius > 6000.0f )
		radius = 6000.0f;
	else if( radius > maxr )
		radius = maxr;

	mSelectedNode->SetRadius( radius );
	mSelectedNode->Update();

	if( updateDialog )
		mTransformDialog->UpdateLight( mSelectedNode->GetWorldTranslate(), mSelectedNode->GetRadius() );
	
	/// 조명의 영향을 받는 노드들을 검색
	mSelectedNode->DetachAll();
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, mSelectedNode->GetBoundSphere(), SCENENODE_STATIC ) )
	{
		for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
		{
			mPickedArray[i]->SetNeedUpdateTransform( true );
		}
	}

	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

bool cLightEditor::Kill( cLightSceneNode* node )
{
	/// 생성 셑에서 제거
	mCreatedSet.Erase( node );

	/// 조명의 영향을 받는 노드들을 모두 탈착
	node->DetachAll();

	/// 장면 관리자로부터 제거
	SCENEMAN->DestroyNode( node );
	//SCENEMAN->DestroyLight( node );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
	return true;
}

cLightSceneNode* cLightEditor::Clone( cLightSceneNode* node )
{
	cLightSceneNodeParam param;
	param.mPathName = node->GetFileName();
	param.mTranslate = node->GetWorldTranslate();
	param.mRotate = node->GetWorldRotate();
	param.mScale = node->GetWorldScale();
	param.mRadius = node->GetRadius();
	param.mAmbient = node->GetAmbient();
	param.mDiffuse0 = node->GetDiffuse0();
	param.mDiffuse1 = node->GetDiffuse1();
	param.mDiffuseAnimTime = node->GetDiffuseAnimTime();
	param.mSpecular = node->GetSpecular();
	param.mConstantAtten0 = node->GetConstantAtten0();
	param.mConstantAtten1 = node->GetConstantAtten1();
	param.mConstantAttenAnimTime = node->GetConstantAttenAnimTime();
	param.mLinearAtten = node->GetLinearAtten();
	param.mQuadricAtten = node->GetQuadricAtten();

	cLightSceneNode* n = SCENEMAN->CreateLight( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );
	}
	return n;
}

void cLightEditor::CopyLight()
{
	if( mSelectedNode == 0 )
		return;

	cSceneNode* n = Clone( mSelectedNode );

	if( n == 0 )
	{
		assert( !"failed to clone object" );
		return;
	}

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
}

void cLightEditor::DeleteLight()
{
	assert( mSelectedNode && "null light" );

	Kill( mSelectedNode );
	mSelectedNode = 0;
	mTransformDialog->SetEnabled( false );
	mPropertyDialog->SetEnabled( false );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
}

void cLightEditor::UpdateInfoDialog()
{
	if( mInfoDialog )
		mInfoDialog->UpdateNumLights( mCreatedSet.GetSize() );
}

void cLightEditor::SetLightAmbient( COLORREF rgb )
{
	assert( mSelectedNode && "null light" );

	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	mSelectedNode->SetAmbient( color );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightDiffuse0( COLORREF rgb )
{
	assert( mSelectedNode && "null light" );

	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	mSelectedNode->SetDiffuse0( color );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightDiffuse1( COLORREF rgb )
{
	assert( mSelectedNode && "null light" );

	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	mSelectedNode->SetDiffuse1( color );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightDiffuseAnimTime( float time )
{
	assert( mSelectedNode && "null light" );

	mSelectedNode->SetDiffuseAnimTime( time );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightConstantAtten0( float value )
{
	mSelectedNode->SetConstantAtten0( value );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightConstantAtten1( float value )
{
	mSelectedNode->SetConstantAtten1( value );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightConstantAttenAnimTime( float value )
{
	mSelectedNode->SetConstantAttenAnimTime( value );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightLinearAtten( float value )
{
	mSelectedNode->SetLinearAtten( value );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::SetLightQuadricAtten( float value )
{
	mSelectedNode->SetQuadricAtten( value );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cLightEditor::OnMouseMove( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_X) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.x += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetLightTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.y += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetLightTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Z) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.z -= (point.y - mMousePos.y) * 10.0f;
			SetLightTranslate( t, true );
		}
		if( KEY->IsDown(KEY_R) )
		{
			float r = mTransformDialog->GetRadius();
			r += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;

			SetLightRadius( r, true );
		}
	}

	mMousePos = point;
}

void cLightEditor::OnLButtonDown_Transform( CPoint point )
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
			AttachLightToTerrain( point );
		}
		break;
	case IDC_BUTTON_LFORM_TO_OBJECT:
		{
			AttachLightToObject( point );
		}
		break;
	}
}

void cLightEditor::OnRButtonDown_Transform( CPoint point, bool ctrl )
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
			SetLightTranslate( pos, true );
		}
	}
}

void cLightEditor::OnKeyDown( UINT c )
{
	switch( c )
	{
	case KEY_DELETE:
		{
			if( mSelectedNode == 0 )
				return;

			int mb = MessageBox( 0, "Do you want to delete selected light?", "Light", MB_YESNO );
			
			if( mb == IDYES )
			{
				DeleteLight();
			}
		}
		break;
	}
}
