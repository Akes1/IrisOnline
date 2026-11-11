#include "stdafx.h"
#include "ObjectEditor.h"

#include "resource.h"
#include "MainFrame.h"
#include "MapEditorApp.h"
#include "MapEditorView.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"
#include "ObjectInfoDialog.h"
#include "LightEditor.h"
#include "SoundEditor.h"
#include "AreaEditor.h"
#include "EnvEditor.h"

#include "Engine/MouseAgent.h"
#include "Engine/KeyAgent.h"
#include "Engine/CameraManager.h"
#include "Engine/SceneManager.h"
#include "Engine/SceneTree.h"
#include "Engine/AreaSceneNode.h"
#include "Engine/StaticSceneNode.h"
#include "Engine/Terrain.h"

#include "Doing/DoingManager.h"

cObjectGroup::cObjectGroup()
{
	mNiNode = NiNew NiNode;

	mCircleLines = NiNew NiMesh;
	mCircleLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mCircleLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 20, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::WHITE );
	mMatProp->SetDiffuseColor( NiColor::WHITE );
	mMatProp->SetSpecularColor( NiColor::WHITE );
	mMatProp->SetEmittance( NiColor(0.0f, 1.0f, 0.0f) );

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

cObjectGroup::~cObjectGroup()
{
}

void cObjectGroup::Clear()
{
	DetachAll();
	mSelectedSet.Clear();
}

void cObjectGroup::Render()
{
	if( mCircleLines == 0 )
		return;

	cCamera* cam = CAMERAMAN->GetCurrent();
	NiPoint3 right = cam->GetWorldRightVector();
	NiPoint3 up = cam->GetWorldUpVector();

	NiDataStreamElementLock kPositionLock( mCircleLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

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

			kPositionIter[(2*i)+1] = c + dir * r;
			kPositionIter[(2*(i+1))%20] = c + dir * r;

		}

		mCircleLines->RenderImmediate( NiRenderer::GetRenderer() );
	}

	kPositionLock.Unlock();
}

void cObjectGroup::DetachAll()
{
	unsigned int numNodes = mSelectedSet.GetSize();

	switch( numNodes )
	{
	case 0:
		break;
	case 1:
		{
			cStaticSceneNode* n = *mSelectedSet.Begin();

			mNiNode->DetachChild( n->GetNiObj() );
			n->SetTranslate( mNiNode->GetWorldTranslate() );
			n->SetRotate( mNiNode->GetWorldRotate() );
			n->SetScale( mNiNode->GetWorldScale() );
		}
		break;
	case 2:
	default:
		{
			NiPoint3 t;
			NiMatrix3 r;
			float s;
			cSelectedSet::cIterator i = mSelectedSet.Begin();
			cSelectedSet::cIterator end = mSelectedSet.End();

			for( ; i != end; ++i )
			{
				cStaticSceneNode* n = *i;

				t = n->GetWorldTranslate();
				r = n->GetWorldRotate();
				s = n->GetWorldScale();
				mNiNode->DetachChild( n->GetNiObj() );

				n->SetTranslate( t );
				n->SetRotate( r );
				n->SetScale( s );
			}
		}
		break;
	}

	mNiNode->SetTranslate( 0.0f, 0.0f, 0.0f );
	mNiNode->SetRotate( NiMatrix3::IDENTITY );
	mNiNode->SetScale( 1.0f );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::AttachAll()
{
	unsigned int numNodes = mSelectedSet.GetSize();

	switch( numNodes )
	{
	case 0:
		mNiNode->SetTranslate( 0.0f, 0.0f, 0.0f );
		mNiNode->SetRotate( NiMatrix3::IDENTITY );
		mNiNode->SetScale( 1.0f );
		mNiNode->Update( 0.0f );
		break;
	case 1:
		{
			cStaticSceneNode* n = *mSelectedSet.Begin();

			mNiNode->SetTranslate( n->GetWorldTranslate() );
			mNiNode->SetRotate( n->GetWorldRotate() );
			mNiNode->SetScale( n->GetWorldScale() );
			mNiNode->AttachChild( n->GetNiObj() );

			n->SetTranslate( NiPoint3::ZERO );
			n->SetRotate( NiMatrix3::IDENTITY );
			n->SetScale( 1.0f );
			mNiNode->Update( 0.0f );
		}
		break;
	case 2:
	default:
		{
			/// 노드들의 중점 계산
			NiPoint3 pos( NiPoint3::ZERO );
			cSelectedSet::cIterator i = mSelectedSet.Begin();
			cSelectedSet::cIterator end = mSelectedSet.End();

			for( ; i != end; ++i )
			{
				pos += (*i)->GetWorldTranslate();
			}

			/// 변환 설정
			mNiNode->SetTranslate( pos / (float)numNodes );
			mNiNode->SetRotate( NiMatrix3::IDENTITY );
			mNiNode->SetScale( 1.0f );
			mNiNode->Update( 0.0f );

			/// 노드들을 부착
			NiPoint3 t;
			NiMatrix3 r;
			float s;
			i = mSelectedSet.Begin();
			end = mSelectedSet.End();

			for( ; i != end; ++i )
			{
				cStaticSceneNode* n = *i;

				t = n->GetWorldTranslate() - mNiNode->GetWorldTranslate();
				r = n->GetWorldRotate();
				s = n->GetWorldScale();
				mNiNode->AttachChild( n->GetNiObj() );

				n->SetTranslate( t );
				n->SetRotate( r );
				n->SetScale( s );
			}

			mNiNode->Update( 0.0f );
		}
		break;
	}
}

cObjectEditor* cObjectEditor::mSingleton = 0;

cObjectEditor::cObjectEditor()
: mTransformDialog( 0 )
, mPropertyDialog( 0 )
, mInfoDialog( 0 )
, mCreatedSet( 2048 )
, mDestroyedSet( 2048 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mPickedArray.Reserve( 1024 );
	mObjectGroup = new cObjectGroup;
}

cObjectEditor::~cObjectEditor()
{
	/// 삭제 대기 상태의 장면 노드들을 제거
	cDestroyedSet::cIterator i = mDestroyedSet.Begin();
	cDestroyedSet::cIterator end = mDestroyedSet.End();

	for( ; i != end; ++i )
	{
		delete *i;
	}

	delete mObjectGroup;
}

void cObjectEditor::Clear()
{
	LIGHTEDIT->Clear();
	SOUNDEDIT->Clear();
	AREAEDIT->Clear();

	/// 삭제 대기 상태의 장면 노드들을 제거
	cDestroyedSet::cIterator i = mDestroyedSet.Begin();
	cDestroyedSet::cIterator end = mDestroyedSet.End();

	for( ; i != end; ++i )
	{
		delete *i;
	}

	mDestroyedSet.Clear();
	mCreatedSet.Clear();
	mPickedArray.Clear();
	mObjectGroup->Clear();
	mPathName.Clear();

	if( mTransformDialog )
		mTransformDialog->SetEnabled( false );
	if( mPropertyDialog )
		mPropertyDialog->SetEnabled( false );
}

void cObjectEditor::ClearGarbage()
{
	/// 삭제 대기 상태의 장면 노드들을 제거
	cDestroyedSet::cIterator i = mDestroyedSet.Begin();
	cDestroyedSet::cIterator end = mDestroyedSet.End();

	for( ; i != end; ++i )
	{
		delete *i;
	}

	mDestroyedSet.Clear();
	mPickedArray.Clear();
}

void cObjectEditor::Render( bool group )
{
	SCENEMAN->Render();

	if( group )
		mObjectGroup->Render();
}

void cObjectEditor::Init()
{
	Clear();

	/// 현재 경로를 설정
	cString path = theApp.GetBaseDir();
	path += "MapData";
	SetCurrentDirectory( path.Cstr() );

	/// 하늘을 로딩
	ENVEDIT->LoadSky( "Sky.nif" );

	if( TERRAIN )
	{
		SCENEMAN->Init( *TERRAIN );
		LIGHTEDIT->Init();
		SOUNDEDIT->Init();
		AREAEDIT->Init();
	}

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	MAIN->SetSceneModified( false );

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->UpdateScene( SCENEMAN->GetCenter(), SCENEMAN->GetMinRadius(), SCENEMAN->GetMaxRadius(), mCreatedSet.GetSize() );
	LIGHTEDIT->UpdateInfoDialog();
	SOUNDEDIT->UpdateInfoDialog();
}

void cObjectEditor::Init( unsigned int gridSize, float metersPerVertex, unsigned int unitsPerMeter )
{
	Clear();

	/// 현재 경로를 설정
	cString path = theApp.GetBaseDir();
	path += "MapData";
	SetCurrentDirectory( path.Cstr() );

	/// 하늘을 로딩
	//ENVEDIT->LoadSky( "Sky.nif" );

	/// 장면 관리자 및 각종 편집자 초기화
	float minRadius = 1000.0f;
	float maxRadius = float(gridSize * unitsPerMeter) * metersPerVertex * 0.5f;
	NiPoint3 center = NiPoint3( maxRadius, maxRadius, 0.0f );

	SCENEMAN->Init( center, minRadius, maxRadius * 1.4142135623731f * 1.1f );
	//WORLDMAN->Init();
	LIGHTEDIT->Init();
	SOUNDEDIT->Init();
	AREAEDIT->Init();
	/*

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	MAIN->SetSceneModified( false );

	/// 정보 다이얼로그를 갱신
	if( mInfoDialog )
		mInfoDialog->UpdateScene( center, minRadius, maxRadius, mCreatedSet.GetSize() );
	LIGHTEDIT->UpdateInfoDialog();
	SOUNDEDIT->UpdateInfoDialog();
	*/
	gridSize;metersPerVertex;unitsPerMeter;
}

void cObjectEditor::InitTree()
{
	SCENEMAN->InitTree( *TERRAIN );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateScene( SCENEMAN->GetCenter(), SCENEMAN->GetMinRadius(), SCENEMAN->GetMaxRadius(), mCreatedSet.GetSize() );
}

bool cObjectEditor::Load( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 지움
	Clear();

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 로딩
	if( SCENEMAN->Load( pathName, mCreatedSet, LIGHTEDIT->mCreatedSet, SOUNDEDIT->mCreatedSet, AREAEDIT->mCreatedSet ) == false )
		return false;

	/// 카메라 거리를 제한
	float farDist = CAMERAMAN->GetCamera(0)->GetFarDistance();
	if( farDist > GAME_CAM_FAR_DIST_UNIT )
	{
		CAMERAMAN->GetCamera(0)->SetFarDistance( GAME_CAM_FAR_DIST_UNIT );
	}

	/// 전체 영역을 선택
	AREAEDIT->SelectArea( SCENEMAN->GetGlobalArea(), true );

	/// 환경 정보를 설정
	ENVEDIT->Update();

	/// 뷰를 갱신
	VIEW->Update();

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	MAIN->SetSceneModified( name, false );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateScene( SCENEMAN->GetCenter(), SCENEMAN->GetMinRadius(), SCENEMAN->GetMaxRadius(), mCreatedSet.GetSize() );
	LIGHTEDIT->UpdateInfoDialog();
	SOUNDEDIT->UpdateInfoDialog();
	return true;
}

bool cObjectEditor::Add( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 지움
	ClearGarbage();

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 로딩
	if( SCENEMAN->Load( pathName, mCreatedSet, LIGHTEDIT->mCreatedSet, SOUNDEDIT->mCreatedSet, AREAEDIT->mCreatedSet, true ) == false )
		return false;

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
	LIGHTEDIT->UpdateInfoDialog();
	SOUNDEDIT->UpdateInfoDialog();
	return true;
}

bool cObjectEditor::LoadAreaGroup( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 로딩
	if( SCENEMAN->LoadAreaGroup( pathName ) == false )
		return false;

	return true;
}

bool cObjectEditor::LoadObjectTransformScript( const cString& pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 지움
	ClearGarbage();

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 로딩
	if( SCENEMAN->LoadObjectTransformScript( pathName, mCreatedSet ) == false )
		return false;

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
	return true;
}

bool cObjectEditor::Save()
{
	if( mPathName.IsEmpty() )
		return false;
	else
		return SaveAs( mPathName );
}

static const char* gSceneFileCode = "IrisScene";
static const int gSceneFileVersion = 11;

bool cObjectEditor::SaveGlobalArea( cFileSaver& saver )
{
	/// 전체 영역 정보를 저장
	return SCENEMAN->GetGlobalArea()->Save( saver );
}

bool cObjectEditor::SaveAs( const cString& pathName, bool objectAdded, bool lightAdded, bool soundAdded, bool areaAdded )
{
	unsigned int numSceneNodes = 0;
	if( objectAdded )
		numSceneNodes += mCreatedSet.GetSize();
	if( lightAdded )
		numSceneNodes += LIGHTEDIT->GetNumLights();
//	if( soundAdded )
//		numSceneNodes += SOUNDEDIT->GetNumSounds();
	if( areaAdded )
		numSceneNodes += AREAEDIT->GetNumAreas();
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
		assert( 0 && "failed to open file to save scene" );
		return false;
	}

	/// 파일 헤더를 저장
	cSceneFileHeader header;
	memcpy( header.mCode, gSceneFileCode, 10 );
	header.mVersion = gSceneFileVersion;
	header.mCenter = SCENEMAN->GetCenter();
	header.mMinRadius = SCENEMAN->GetMinRadius();
	header.mMaxRadius = SCENEMAN->GetMaxRadius();
	header.mNumSceneNodes = numSceneNodes;

	if( saver.Write( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		assert( 0 && "failed to save scene file header" );
		return false;
	}

	/// 환경 정보를 저장
	if( ENVEDIT->Save( saver ) == false )
		return false;

	/// 전역 정보를 저장
	if( SaveGlobalArea( saver ) == false )
	{
		assert( 0 && "failed to save global area" );
		return false;
	}

	/// 장면 노드들을 저장
	if( objectAdded )
	{
		cCreatedSet::cIterator i = mCreatedSet.Begin();
		cCreatedSet::cIterator end = mCreatedSet.End();

		for( ; i != end; ++i )
		{
			if( (*i)->Save( saver ) == false )
			{
				assert( 0 && "failed to save scene nodes" );
				return false;
			}
		}
	}

	/// 조명 노드들을 저장
	if( lightAdded && LIGHTEDIT->Save( saver ) == false )
		return false;

//	/// 사운드 노드들을 저장
//	if( soundAdded && SOUNDEDIT->Save( saver ) == false )
//		return false;

	/// 영역 노드들을 저장
	if( areaAdded && AREAEDIT->Save( saver ) == false )
		return false;

	/// 경로 이름을 설정
	mPathName = pathName;

	cString name;
	::GetFileName( &name, pathName );
	
	if( objectAdded && lightAdded && soundAdded && areaAdded )
		MAIN->SetSceneModified( name, false );
	return true;
}

bool cObjectEditor::SaveAreaAs( const cString& pathName )
{
	SaveAs( pathName, false, false, false, true );
	return true;
}

bool cObjectEditor::SaveObjectGroupAs( const cString& pathName )
{
	cCreatedSet& selectedSet = mObjectGroup->mSelectedSet;
	if( selectedSet.IsEmpty() )
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
		assert( 0 && "failed to open file to save scene" );
		return false;
	}

	/// 파일 헤더를 저장
	cSceneFileHeader header;
	memcpy( header.mCode, gSceneFileCode, 10 );
	header.mVersion = gSceneFileVersion;
	header.mCenter = SCENEMAN->GetCenter();
	header.mMinRadius = SCENEMAN->GetMinRadius();
	header.mMaxRadius = SCENEMAN->GetMaxRadius();
	header.mNumSceneNodes = selectedSet.GetSize();

	if( saver.Write( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		assert( 0 && "failed to save scene file header" );
		return false;
	}

	/// 환경 정보를 저장
	if( ENVEDIT->Save( saver ) == false )
		return false;

	/// 전역 정보를 저장
	if( SaveGlobalArea( saver ) == false )
	{
		assert( 0 && "failed to save global area" );
		return false;
	}

	/// 장면 노드들을 저장
	cCreatedSet::cIterator i = selectedSet.Begin();
	cCreatedSet::cIterator iend = selectedSet.End();

	for( ; i != iend; ++i )
	{
		if( (*i)->Save( saver ) == false )
		{
			assert( 0 && "failed to save scene nodes" );
			return false;
		}
	}
	return true;
}

void cObjectEditor::AdjustAllPos( float scale )
{
	if( mCreatedSet.IsEmpty() )
		return;

	DeselectAll();

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 스케일을 조절
	SCENEMAN->AdjustAllPos( scale );

	/// 뷰를 갱신
	VIEW->Update();

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateScene( SCENEMAN->GetCenter(), SCENEMAN->GetMinRadius(), SCENEMAN->GetMaxRadius(), mCreatedSet.GetSize() );

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::AdjustAllSize( float scale )
{
	if( mCreatedSet.IsEmpty() )
		return;

	DeselectAll();

	/// 실행 기록을 지움
	DOINGMAN->ClearObject();

	/// 스케일을 조절
	SCENEMAN->AdjustAllSize( scale );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

bool cObjectEditor::Find( cStaticSceneNode* node )
{
	return mCreatedSet.Find( node ) != mCreatedSet.End();
}

bool cObjectEditor::Kill( cStaticSceneNode* node )
{
	/// 생성 셑에서 제거
	mCreatedSet.Erase( node );

	/// 삭제 셑에 추가
	mDestroyedSet.Insert( node );

	/// 장면 관리자로부터 제거
	cSceneManager::cStaticArray& nodeArray = SCENEMAN->mStaticArray;
	unsigned int i = node->mIndexByManager;

	if( i >= nodeArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	/// 트리에서 삭제
	cSceneNode* n = nodeArray[i];
	if( n->mContainer )
		n->mContainer->Remove( n );

	/// 노드 배열에서 삭제
	bool orderChanged = false;
	if( nodeArray.PopAt( &orderChanged, i ) == false )
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
		nodeArray[i]->mIndexByManager = i;
	}

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
	return true;
}

void cObjectEditor::Rebirth( cStaticSceneNode* node )
{
	/// 삭제 셑에서 제거
	mDestroyedSet.Erase( node );

	/// 생성 셑에 추가
	mCreatedSet.Insert( node );

	/// 장면 관리자에 추가
	SCENEMAN->AddNode( node );
	//unsigned int i = SCENEMAN->AddStatic( node );
	//node->mIndexByManager = i;

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
}

cStaticSceneNode* cObjectEditor::Clone( cStaticSceneNode* node )
{
	cStaticSceneNodeParam param;
	param.mPathName = node->GetFileName();
	param.mTranslate = node->GetWorldTranslate();
	param.mRotate = node->GetWorldRotate();
	param.mScale = node->GetWorldScale();
	param.mFogApplied = node->GetFogApplied();
	param.mMaterialApplied = node->GetMaterialApplied();
	param.mMaterial = node->GetMaterial();
	param.mVisibleLevel = node->GetVisibleLevel();
	param.mOccludeLevel = node->GetOccludeLevel();

	cStaticSceneNode* n = SCENEMAN->CreateStatic( param );

	if( n )
	{
		/// 생성 셑에 추가
		mCreatedSet.Insert( n );
	}
	return n;
}

void cObjectEditor::UpdateInfoDialog()
{
	if( mInfoDialog )
		mInfoDialog->UpdateScene( SCENEMAN->GetCenter(), SCENEMAN->GetMinRadius(), SCENEMAN->GetMaxRadius(), mCreatedSet.GetSize() );
}

void cObjectEditor::SetVisibleLevelToSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	unsigned int level = mObjectGroup->GetVisibleLevel();

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetVisibleLevel( level );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetOccludeLevelToSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	unsigned int level = mObjectGroup->GetOccludeLevel();

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetOccludeLevel( level );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetFogToSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	bool fogApplied = mObjectGroup->GetFogApplied();

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetFogApplied( fogApplied );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetMaterialToSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	bool matApplied = mObjectGroup->GetMaterialApplied();

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetMaterialApplied( matApplied );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetAmbientSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	COLORREF rgb = mObjectGroup->GetAmbient();
	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetAmbient( color );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetDiffuseSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	COLORREF rgb = mObjectGroup->GetDiffuse();
	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetDiffuse( color );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetEmissiveSiblings()
{
	if( mObjectGroup->GetSize() != 1 )
		return;

	cString name = mObjectGroup->GetFileName();
	COLORREF rgb = mObjectGroup->GetEmissive();
	NiColor color;
	color.r = GetRValue( rgb ) / 255.0f;
	color.g = GetGValue( rgb ) / 255.0f;
	color.b = GetBValue( rgb ) / 255.0f;

	bool found = false;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;

		if( name == n->GetFileName() )
		{
			n->SetEmissive( color );
			found = true;
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetAmbientLightAmbient( unsigned int i, const NiColor& color )
{
	SCENEMAN->SetAmbientLightAmbient( i, color );
}

void cObjectEditor::SetAmbientLightDiffuse( unsigned int i, const NiColor& color )
{
	SCENEMAN->SetAmbientLightDiffuse( i, color );
}

void cObjectEditor::SetAmbientLightDimmer( unsigned int i, float dimmer )
{
	SCENEMAN->SetAmbientLightDimmer( i, dimmer );
}

void cObjectEditor::SetFog( bool enabled, const NiColor& color, float depth )
{
	SCENEMAN->SetFog( enabled, color, depth );
}

void cObjectEditor::SetFogColor( const NiColor& color )
{
	SCENEMAN->SetFogColor( color );
}

void cObjectEditor::SetFogDepth( float depth )
{
	SCENEMAN->SetFogDepth( depth );
}

NiAmbientLight* cObjectEditor::GetAmbientLight( unsigned int i ) const
{
	return SCENEMAN->GetAmbientLight( i );
}

void cObjectEditor::OnMouseMove( CPoint point )
{
	if( MOUSE->IsLButtonDown() )
	{
		if( KEY->IsDown(KEY_X) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.x += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetGroupTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.y += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
			SetGroupTranslate( t, true );
		}
		if( KEY->IsDown(KEY_Z) )
		{
			NiPoint3 t = mTransformDialog->GetTranslate();
			t.z -= (point.y - mMousePos.y) * 10.0f;
			SetGroupTranslate( t, true );
		}
	}
	else if( MOUSE->IsRButtonDown() )
	{
		if( KEY->IsDown(KEY_X) )
		{
			float xangle, yangle, zangle;
			mTransformDialog->GetRotate( &xangle, &yangle, &zangle );
			xangle += D3DXToRadian((point.x - mMousePos.x) - (point.y - mMousePos.y));
			SetGroupRotate( xangle, yangle, zangle, true );
		}
		if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
		{
			float xangle, yangle, zangle;
			mTransformDialog->GetRotate( &xangle, &yangle, &zangle );
			yangle += D3DXToRadian((point.x - mMousePos.x) - (point.y - mMousePos.y));
			SetGroupRotate( xangle, yangle, zangle, true );
		}
		if( KEY->IsDown(KEY_Z) )
		{
			float xangle, yangle, zangle;
			mTransformDialog->GetRotate( &xangle, &yangle, &zangle );
			zangle += D3DXToRadian(mMousePos.x - point.x);
			SetGroupRotate( xangle, yangle, zangle, true );
		}
	}
	mMousePos = point;
}

void cObjectEditor::OnLButtonDown_Transform( CPoint point, bool ctrl )
{
	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) )
	{
		mMousePos = point;
		return;
	}

	switch( mTransformDialog->GetCheckedButton() )
	{
	case 0:
	case IDC_BUTTON_OFORM_PICK:
		{
			Pick( point, ctrl );
		}
		break;
	case IDC_BUTTON_OFORM_TO_TERRAIN:
		{
			AttachGroupToTerrain( point );
		}
		break;
	case IDC_BUTTON_OFORM_TO_OBJECT:
		{
			AttachGroupToObject( point );
		}
		break;
	}
}

void cObjectEditor::OnRButtonDown_Transform( CPoint point, bool ctrl )
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
			SetGroupTranslate( pos, true );
		}
	}
}

void cObjectEditor::OnKeyDown( UINT c )
{
	switch( c )
	{
	case KEY_DELETE:
		{
			if( mObjectGroup->GetSize() > 0 )
			{
				int mb = MessageBox( 0, "Do you want to delete selected objects?", "Object", MB_YESNO );

				if( mb == IDYES )
				{
					DeleteGroup();
				}
			}
			break;
		}
	case KEY_COMMA:
		{
			float s = mTransformDialog->GetScale();
			s -= 0.1f;
			if( s < 0.1f )
				s = 0.1f;
			SetGroupScale( s, true );
			break;
		}
	case KEY_PERIOD:
		{
			float s = mTransformDialog->GetScale();
			s += 0.1f;
			if( s > 10.0f )
				s = 10.0f;
			SetGroupScale( s, true );
			break;
		}
	}
}