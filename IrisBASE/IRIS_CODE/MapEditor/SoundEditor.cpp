#include "stdafx.h"
#include "SoundEditor.h"

#include "resource.h"
#include "MainFrame.h"
#include "MapEditorView.h"
#include "SoundTransformDialog.h"
#include "SoundPropertyDialog.h"
#include "SoundInfoDialog.h"
#include "TerrainEditor.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/CameraManager.h"
#include "Engine/SceneManager.h"
#include "Engine/SoundSceneNode.h"
#include "Engine/Terrain.h"

cSoundEditor* cSoundEditor::mSingleton = 0;

cSoundEditor::cSoundEditor()
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
	mMatProp->SetEmittance( NiColor(0.0f, 0.0f, 1.0f) );

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

cSoundEditor::~cSoundEditor()
{
	mSingleton = 0;
}

void cSoundEditor::Clear()
{
	mCreatedSet.Clear();
	mPickedArray.Clear();
	mPathName.Clear();
	mSelectedNode = 0;

	if( mTransformDialog )
		mTransformDialog->SetEnabled( false );
	if( mPropertyDialog )
		mPropertyDialog->SetEnabled( false );
}

void cSoundEditor::Render()
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
		cSoundSceneNode* n = *i;

		const NiPoint3& c = n->GetCenter();
		float r = n->GetRadius();
		float a = n->GetAttenDistance();
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
			kPositionIter[s] = kPositionIter[e] = c + dir * a;

			/// 바깥쪽 원
			kPositionIter[s+20] = kPositionIter[e+20] = c + dir * r;
		}

		if( n == mSelectedNode )
		{
			mMatProp->SetEmittance( NiColor(0.0f, 1.0f, 1.0f) );
			mCircleLines->UpdateProperties();
		}
		else if( mMatProp->GetEmittance() != NiColor(0.0f, 0.0f, 1.0f) )
		{
			mMatProp->SetEmittance( NiColor(0.0f, 0.0f, 1.0f) );
			mCircleLines->UpdateProperties();
		}
		mCircleLines->RenderImmediate( NiRenderer::GetRenderer() );
	}

	kPositionLock.Unlock();

	mPathName.Clear();
}

void cSoundEditor::Init()
{
	Clear();

	MAIN->SetSoundSceneModified( false );

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
}

bool cSoundEditor::Load( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 지움
	Clear();

	/// 로딩
	if( SCENEMAN->LoadSound( pathName, mCreatedSet ) == false )
		return false;

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	mPathName = pathName;

	/// 변경 여부를 설정
	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetSoundSceneModified( name, false );

	/// 정보 다이얼로그를 갱신
	UpdateInfoDialog();

	return true;
}

static const char* gSceneFileCode = "IrisScene";
static const int gSceneFileVersion = 11;

bool cSoundEditor::Save()
{
	if( mPathName.IsEmpty() )
		return false;
	else
		return SaveAs( mPathName );
}

bool cSoundEditor::SaveAs( const cString& pathName )
{
	unsigned int numSceneNodes = 0;
	numSceneNodes += SOUNDEDIT->GetNumSounds();

	if( numSceneNodes == 0 )
		return false;

	/// 파일을 열기
	cFileSaver saver;
	if( ::FileExist( pathName ) )
	{
		cString newPathName = pathName;
		newPathName += ".bak";
		::FileRemove( newPathName );
		::FileRename( pathName, newPathName );
	}

	if( saver.Open( pathName ) == false )
	{
		assert( 0 && "failed to open file to save sound scene" );
		return false;
	}

	/// 파일 헤더를 저장
	cSoundSceneFileHeader header;
	memcpy( header.mCode, gSceneFileCode, 10 );
	header.mVersion = gSceneFileVersion;
	header.mNumSceneNodes = numSceneNodes;

	if( saver.Write( &header, sizeof(cSoundSceneFileHeader) ) != sizeof(cSoundSceneFileHeader) )
	{
		assert( 0 && "failed to save sound scene file header" );
		return false;
	}

	/// 사운드 노드들을 저장
	if( SOUNDEDIT->Save( saver ) == false )
		return false;

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetSoundSceneModified( name, false );

	return true;
}

bool cSoundEditor::Save( cFileSaver& saver )
{
	/// 장면 노드들을 저장
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		if( (*i)->Save( saver ) == false )
		{
			assert( 0 && "failed to save sound nodes" );
			return false;
		}
	}
	return true;
}

bool cSoundEditor::CreateSound( const char* pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 장면 노드를 생성
	cSoundSceneNodeParam param;
	param.mPathName = pathName;
	cSoundSceneNode* n = SCENEMAN->CreateSound( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );

		/// 생성된 오브젝트를 선택
		mSelectedNode = n;
		mTransformDialog->UpdateSound( n->GetWorldTranslate(), n->GetRadius(), n->GetAttenDistance() );
		mPropertyDialog->UpdateSound( n->GetFileName().Cstr(), n->GetLoopCount(), n->GetLoopInterval(), n->GetVolumeRatio() );

		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSoundSceneModified( true );

		/// 정보 다이얼로그를 갱신
		mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
	}
	return n != 0;
}

bool cSoundEditor::Pick( CPoint point )
{
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_SOUND ) )
	{
		cSoundSceneNode* n = mSelectedNode = (cSoundSceneNode*)mPickedArray[0];

		mTransformDialog->UpdateSound( n->GetWorldTranslate(), n->GetRadius(), n->GetAttenDistance() );
		mPropertyDialog->UpdateSound( n->GetFileName().Cstr(), n->GetLoopCount(), n->GetLoopInterval(), n->GetVolumeRatio() );
		return true;
	}
	else
	{
		return false;
	}
}

void cSoundEditor::DeselectAll()
{
	if( mCreatedSet.IsEmpty() == false )
	{
		mSelectedNode = 0;
		mTransformDialog->SetEnabled( false );
		mPropertyDialog->SetEnabled( false );
	}
}

void cSoundEditor::AttachSoundToTerrain( CPoint point )
{
	if( mSelectedNode == 0 )
		return;

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ) )
	{
		SetSoundTranslate( pos, true );
	}
}

void cSoundEditor::AttachSoundToObject( CPoint point )
{
	if( mSelectedNode == 0 )
		return;

	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
	{
		cSceneNode* n = mPickedArray[0];
		SetSoundTranslate( n->GetPickPos(), true );
	}
}

bool cSoundEditor::Kill( cSoundSceneNode* node )
{
	/// 생성 셑에서 제거
	mCreatedSet.Erase( node );

	/// 장면 관리자로부터 제거
	SCENEMAN->DestroyNode( node );
	//SCENEMAN->DestroySound( node );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
	return true;
}

cSoundSceneNode* cSoundEditor::Clone( cSoundSceneNode* node )
{
	cSoundSceneNodeParam param;
	param.mPathName = node->GetFileName();
	param.mTranslate = node->GetWorldTranslate();
	param.mRotate = node->GetWorldRotate();
	param.mScale = node->GetWorldScale();
	param.mOuterRadius = node->GetRadius();

	cSoundSceneNode* n = SCENEMAN->CreateSound( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );
	}
	return n;
	return node;
}

void cSoundEditor::CopySound()
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
	MAIN->SetSoundSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
}

void cSoundEditor::DeleteSound()
{
	assert( mSelectedNode && "null sound" );

	Kill( mSelectedNode );
	mSelectedNode = 0;
	mTransformDialog->SetEnabled( false );
	mPropertyDialog->SetEnabled( false );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
}

void cSoundEditor::UpdateInfoDialog()
{
	if( mInfoDialog )
		mInfoDialog->UpdateNumSounds( mCreatedSet.GetSize() );
}

void cSoundEditor::SetSoundTranslate( const NiPoint3& trans, bool updateDialog )
{
	if( mSelectedNode == 0 )
		return;

	mSelectedNode->SetTranslate( trans );
	mSelectedNode->Update();

	if( updateDialog )
		mTransformDialog->UpdateSound( mSelectedNode->GetWorldTranslate(), mSelectedNode->GetRadius(), mSelectedNode->GetAttenDistance() );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::SetSoundRadius( float radius, bool updateDialog )
{
	if( mSelectedNode == 0 )
		return;

	float a = mSelectedNode->GetAttenDistance();
	float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

	if( radius < 500.0f )
		radius = 500.0f;
	else if( radius < a + 100.0f )
		radius = a + 100.0f;
	else if( radius > 5000.0f )
		radius = 5000.0f;
	else if( radius > maxr )
		radius = maxr;

	mSelectedNode->SetRadius( radius );
	mSelectedNode->Update();

	if( updateDialog )
	{
		mTransformDialog->UpdateSound( mSelectedNode->GetWorldTranslate(),
			mSelectedNode->GetRadius(),
			mSelectedNode->GetAttenDistance() );
	}

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::SetSoundAttenDistance( float dist, bool updateDialog )
{
	if( mSelectedNode == 0 )
		return;

	float r = mSelectedNode->GetRadius();
	float maxr = float(TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex()) * 25.0f;

	if( dist < 100.0f )
		dist = 100.0f;
	else if( dist > 5000.0f )
		dist = 5000.0f;
	else if( dist > r - 100.0f )
		dist = r - 100.0f;
	else if( dist > maxr )
		dist = maxr;

	mSelectedNode->SetAttenDistance( dist );

	if( updateDialog )
	{
		mTransformDialog->UpdateSound( mSelectedNode->GetWorldTranslate(),
			mSelectedNode->GetRadius(),
			mSelectedNode->GetAttenDistance() );
	}

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::SetSoundLoopCount( int count )
{
	assert( mSelectedNode && "null sound" );

	mSelectedNode->SetLoopCount( count );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::SetSoundLoopInterval( float interval )
{
	assert( mSelectedNode && "null sound" );

	mSelectedNode->SetLoopInterval( interval );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::SetSoundVolumeRatio( float ratio )
{
	assert( mSelectedNode && "null sound" );

	mSelectedNode->SetVolumeRatio( ratio );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSoundSceneModified( true );
}

void cSoundEditor::OnMouseMove( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_X) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.x += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetSoundTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.y += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetSoundTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Z) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.z -= (point.y - mMousePos.y) * 10.0f;
			SetSoundTranslate( t, true );
		}
		if( KEY->IsDown(KEY_I) )
		{
			float a = mTransformDialog->GetAttenDistance();
			a += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;

			SetSoundAttenDistance( a, true );
		}
		if( KEY->IsDown(KEY_O) )
		{
			float r = mTransformDialog->GetRadius();
			r += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;

			SetSoundRadius( r, true );
		}
	}

	mMousePos = point;
}

void cSoundEditor::OnLButtonDown_Transform( CPoint point )
{
	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) || KEY->IsDown(KEY_I) || KEY->IsDown(KEY_O) )
	{
		mMousePos = point;
		return;
	}

	switch( mTransformDialog->GetCheckedButton() )
	{
	case IDC_BUTTON_SFORM_PICK:
		{
			Pick( point );
		}
		break;
	case IDC_BUTTON_SFORM_TO_TERRAIN:
		{
			AttachSoundToTerrain( point );
		}
		break;
	case IDC_BUTTON_SFORM_TO_OBJECT:
		{
			AttachSoundToObject( point );
		}
		break;
	}
}

void cSoundEditor::OnRButtonDown_Transform( CPoint point, bool ctrl )
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
			SetSoundTranslate( pos, true );
		}
	}
}

void cSoundEditor::OnKeyDown( UINT c )
{
	switch( c )
	{
	case KEY_DELETE:
		{
			if( mSelectedNode == 0 )
				return;

			int mb = MessageBox( 0, "Do you want to delete selected sound?", "Sound", MB_YESNO );
			
			if( mb == IDYES )
			{
				DeleteSound();
			}
		}
		break;
	}
}
