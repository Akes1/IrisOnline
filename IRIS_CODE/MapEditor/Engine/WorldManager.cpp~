#include "stdafx.h"
#include "WorldManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Terrain.h"
#include "MaterialData.h"
#include "AreaSceneNode.h"
//
#include "../Hero.h"

cWorldManager* cWorldManager::mSingleton = 0;

cWorldManager::cWorldManager()
: mSkyDome( 0 )
, mSkyEnabled( false )
, mSkyName( "Sky.nif" )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	/// 지형을 생성
	mTerrain = new cTerrain;

	/// 하늘 주변광을 생성
	mSkyLight = NiNew NiAmbientLight;
	mSkyLight->SetAmbientColor( NiColor::BLACK );
	mSkyLight->SetDiffuseColor( NiColor::WHITE );
	mSkyLight->SetDimmer( 1.0f );

	/// 주변광
	mSkyAmbient = NiColor::WHITE;
	mSkyDimmer = 1.0f;
	mTerrainAmbient = NiColor::WHITE;
	mTerrainDimmer = 1.0f;
	mStaticObjectAmbient = NiColor::WHITE;
	mStaticObjectDimmer = 1.0f;
	mDynamicObjectAmbient = NiColor::WHITE;
	mDynamicObjectDimmer = 1.0f;

	mTargetSkyAmbient = NiColor::WHITE;
	mTargetSkyDimmer = 1.0f;
	mTargetTerrainAmbient = NiColor::WHITE;
	mTargetTerrainDimmer = 1.0f;
	mTargetStaticObjectAmbient = NiColor::WHITE;
	mTargetStaticObjectDimmer = 1.0f;
	mTargetDynamicObjectAmbient = NiColor::WHITE;
	mTargetDynamicObjectDimmer = 1.0f;

	/// 안개
	mFogColor = NiColor::WHITE;
	mFogDepth = 0.1f;

	mTargetFogColor = NiColor::WHITE;
	mTargetFogDepth = 0.1f;

	///
	mPickedArray.Reserve( 64 );
}

cWorldManager::~cWorldManager()
{
	mSingleton = 0;

	delete mTerrain;
}

void cWorldManager::Clear()
{
	mPickedArray.Clear();
	mTerrain->Clear();
}

void cWorldManager::Init()
{
	SetArea( *SCENEMAN->GetGlobalArea() );
}

void cWorldManager::Process( unsigned long deltaTime, unsigned long accumTime, bool playing )
{
	mTerrain->Process( playing );

#ifdef MAP_EDITOR
	/// 하늘을 회전
	if( mSkyDome )
	{
		//NiMatrix3 rot;
		//rot.MakeZRotation( 0.0001f );
		//mpSkydome->SetRotate( mSkydome->GetRotate() * rot );

		mSkyDome->SetTranslate( CAMERAMAN->GetCurrent()->GetWorldTranslate() );
		mSkyDome->Update( accumTime * 0.001f );
	}

	/// 테스트 모드
	if( playing )
	{
		mPickedArray.Clear();

		if( SCENEMAN->Pick( &mPickedArray, HERO->GetBoundSphere(), SCENENODE_AREA ) )
		{
			if( mPickedArray.GetSize() == 1 )
			{
				/// 충돌 영역과 전체 영역을 블렌딩
				cAreaSceneNode* n = (cAreaSceneNode*)mPickedArray[0];
				SetArea( *n );
			}
			else
			{
				/// 2개 이상 충돌하는 경우에도 2개의 영역만 블렌딩
				cAreaSceneNode* n0 = (cAreaSceneNode*)mPickedArray[0];
				cAreaSceneNode* n1 = (cAreaSceneNode*)mPickedArray[1];
				float dist0 = (n0->GetCenter() - HERO->GetCenter()).Length();
				float dist1 = (n1->GetCenter() - HERO->GetCenter()).Length();
				float r = dist0 / (dist0 + dist1);
				BlendArea( *n0, *n1, r );
			}
		}
		else
		{
			SetArea( *SCENEMAN->GetGlobalArea() );
		}
	}

	/// 주변광
	ProcessAmbient( deltaTime );

	/// 안개
	ProcessFog( deltaTime );
#else
	deltaTime;
	accumTime;
#endif
}

void cWorldManager::ProcessAmbient( unsigned long dt )
{
	float d = dt * 0.0002f;

	/// 하늘 주변광
	if( mSkyAmbient != mTargetSkyAmbient )
	{
		if( mSkyAmbient.r < mTargetSkyAmbient.r )
		{
			mSkyAmbient.r += d;

			if( mSkyAmbient.r > mTargetSkyAmbient.r )
				mSkyAmbient.r = mTargetSkyAmbient.r;
		}
		else
		{
			mSkyAmbient.r -= d;

			if( mSkyAmbient.r < mTargetSkyAmbient.r )
				mSkyAmbient.r = mTargetSkyAmbient.r;
		}

		if( mSkyAmbient.g < mTargetSkyAmbient.g )
		{
			mSkyAmbient.g += d;

			if( mSkyAmbient.g > mTargetSkyAmbient.g )
				mSkyAmbient.g = mTargetSkyAmbient.g;
		}
		else
		{
			mSkyAmbient.g -= d;

			if( mSkyAmbient.g < mTargetSkyAmbient.g )
				mSkyAmbient.g = mTargetSkyAmbient.g;
		}

		if( mSkyAmbient.b < mTargetSkyAmbient.b )
		{
			mSkyAmbient.b += d;

			if( mSkyAmbient.b > mTargetSkyAmbient.b )
				mSkyAmbient.b = mTargetSkyAmbient.b;
		}
		else
		{
			mSkyAmbient.b -= d;

			if( mSkyAmbient.b < mTargetSkyAmbient.b )
				mSkyAmbient.b = mTargetSkyAmbient.b;
		}

		SetSkyAmbient( mSkyAmbient );
	}

	if( mSkyDimmer != mTargetSkyDimmer )
	{
		if( mSkyDimmer < mTargetSkyDimmer )
		{
			mSkyDimmer += d;

			if( mSkyDimmer > mTargetSkyDimmer )
				mSkyDimmer = mTargetSkyDimmer;
		}
		else
		{
			mSkyDimmer -= d;

			if( mSkyDimmer < mTargetSkyDimmer )
				mSkyDimmer = mTargetSkyDimmer;
		}

		SetSkyDimmer( mSkyDimmer );
	}

	/// 지형 주변광
	if( mTerrainAmbient != mTargetTerrainAmbient )
	{
		if( mTerrainAmbient.r < mTargetTerrainAmbient.r )
		{
			mTerrainAmbient.r += d;

			if( mTerrainAmbient.r > mTargetTerrainAmbient.r )
				mTerrainAmbient.r = mTargetTerrainAmbient.r;
		}
		else
		{
			mTerrainAmbient.r -= d;

			if( mTerrainAmbient.r < mTargetTerrainAmbient.r )
				mTerrainAmbient.r = mTargetTerrainAmbient.r;
		}

		if( mTerrainAmbient.g < mTargetTerrainAmbient.g )
		{
			mTerrainAmbient.g += d;

			if( mTerrainAmbient.g > mTargetTerrainAmbient.g )
				mTerrainAmbient.g = mTargetTerrainAmbient.g;
		}
		else
		{
			mTerrainAmbient.g -= d;

			if( mTerrainAmbient.g < mTargetTerrainAmbient.g )
				mTerrainAmbient.g = mTargetTerrainAmbient.g;
		}

		if( mTerrainAmbient.b < mTargetTerrainAmbient.b )
		{
			mTerrainAmbient.b += d;

			if( mTerrainAmbient.b > mTargetTerrainAmbient.b )
				mTerrainAmbient.b = mTargetTerrainAmbient.b;
		}
		else
		{
			mTerrainAmbient.b -= d;

			if( mTerrainAmbient.b < mTargetTerrainAmbient.b )
				mTerrainAmbient.b = mTargetTerrainAmbient.b;
		}

		TERRAIN->SetAmbientLightAmbient( mTerrainAmbient );
	}

	if( mTerrainDimmer != mTargetTerrainDimmer )
	{
		if( mTerrainDimmer < mTargetTerrainDimmer )
		{
			mTerrainDimmer += d;

			if( mTerrainDimmer > mTargetTerrainDimmer )
				mTerrainDimmer = mTargetTerrainDimmer;
		}
		else
		{
			mTerrainDimmer -= d;

			if( mTerrainDimmer < mTargetTerrainDimmer )
				mTerrainDimmer = mTargetTerrainDimmer;
		}

		TERRAIN->SetAmbientLightDimmer( mTerrainDimmer );
	}

	/// 정적 오브젝트 주변광
	if( mStaticObjectAmbient != mTargetStaticObjectAmbient )
	{
		if( mStaticObjectAmbient.r < mTargetStaticObjectAmbient.r )
		{
			mStaticObjectAmbient.r += d;

			if( mStaticObjectAmbient.r > mTargetStaticObjectAmbient.r )
				mStaticObjectAmbient.r = mTargetStaticObjectAmbient.r;
		}
		else
		{
			mStaticObjectAmbient.r -= d;

			if( mStaticObjectAmbient.r < mTargetStaticObjectAmbient.r )
				mStaticObjectAmbient.r = mTargetStaticObjectAmbient.r;
		}

		if( mStaticObjectAmbient.g < mTargetStaticObjectAmbient.g )
		{
			mStaticObjectAmbient.g += d;

			if( mStaticObjectAmbient.g > mTargetStaticObjectAmbient.g )
				mStaticObjectAmbient.g = mTargetStaticObjectAmbient.g;
		}
		else
		{
			mStaticObjectAmbient.g -= d;

			if( mStaticObjectAmbient.g < mTargetStaticObjectAmbient.g )
				mStaticObjectAmbient.g = mTargetStaticObjectAmbient.g;
		}

		if( mStaticObjectAmbient.b < mTargetStaticObjectAmbient.b )
		{
			mStaticObjectAmbient.b += d;

			if( mStaticObjectAmbient.b > mTargetStaticObjectAmbient.b )
				mStaticObjectAmbient.b = mTargetStaticObjectAmbient.b;
		}
		else
		{
			mStaticObjectAmbient.b -= d;

			if( mStaticObjectAmbient.b < mTargetStaticObjectAmbient.b )
				mStaticObjectAmbient.b = mTargetStaticObjectAmbient.b;
		}

		SCENEMAN->SetAmbientLightAmbient( 0, mStaticObjectAmbient );
	}

	if( mStaticObjectDimmer != mTargetStaticObjectDimmer )
	{
		if( mStaticObjectDimmer < mTargetStaticObjectDimmer )
		{
			mStaticObjectDimmer += d;

			if( mStaticObjectDimmer > mTargetStaticObjectDimmer )
				mStaticObjectDimmer = mTargetStaticObjectDimmer;
		}
		else
		{
			mStaticObjectDimmer -= d;

			if( mStaticObjectDimmer < mTargetStaticObjectDimmer )
				mStaticObjectDimmer = mTargetStaticObjectDimmer;
		}

		SCENEMAN->SetAmbientLightDimmer( 0, mStaticObjectDimmer );
	}

	/// 동적 오브젝트 주변광
	if( mDynamicObjectAmbient != mTargetDynamicObjectAmbient )
	{
		if( mDynamicObjectAmbient.r < mTargetDynamicObjectAmbient.r )
		{
			mDynamicObjectAmbient.r += d;

			if( mDynamicObjectAmbient.r > mTargetDynamicObjectAmbient.r )
				mDynamicObjectAmbient.r = mTargetDynamicObjectAmbient.r;
		}
		else
		{
			mDynamicObjectAmbient.r -= d;

			if( mDynamicObjectAmbient.r < mTargetDynamicObjectAmbient.r )
				mDynamicObjectAmbient.r = mTargetDynamicObjectAmbient.r;
		}

		if( mDynamicObjectAmbient.g < mTargetDynamicObjectAmbient.g )
		{
			mDynamicObjectAmbient.g += d;

			if( mDynamicObjectAmbient.g > mTargetDynamicObjectAmbient.g )
				mDynamicObjectAmbient.g = mTargetDynamicObjectAmbient.g;
		}
		else
		{
			mDynamicObjectAmbient.g -= d;

			if( mDynamicObjectAmbient.g < mTargetDynamicObjectAmbient.g )
				mDynamicObjectAmbient.g = mTargetDynamicObjectAmbient.g;
		}

		if( mDynamicObjectAmbient.b < mTargetDynamicObjectAmbient.b )
		{
			mDynamicObjectAmbient.b += d;

			if( mDynamicObjectAmbient.b > mTargetDynamicObjectAmbient.b )
				mDynamicObjectAmbient.b = mTargetDynamicObjectAmbient.b;
		}
		else
		{
			mDynamicObjectAmbient.b -= d;

			if( mDynamicObjectAmbient.b < mTargetDynamicObjectAmbient.b )
				mDynamicObjectAmbient.b = mTargetDynamicObjectAmbient.b;
		}

		SCENEMAN->SetAmbientLightAmbient( 1, mDynamicObjectAmbient );
	}

	if( mDynamicObjectDimmer != mTargetDynamicObjectDimmer )
	{
		if( mDynamicObjectDimmer < mTargetDynamicObjectDimmer )
		{
			mDynamicObjectDimmer += d;

			if( mDynamicObjectDimmer > mTargetDynamicObjectDimmer )
				mDynamicObjectDimmer = mTargetDynamicObjectDimmer;
		}
		else
		{
			mDynamicObjectDimmer -= d;

			if( mDynamicObjectDimmer < mTargetDynamicObjectDimmer )
				mDynamicObjectDimmer = mTargetDynamicObjectDimmer;
		}

		SCENEMAN->SetAmbientLightDimmer( 1, mDynamicObjectDimmer );
	}
}

void cWorldManager::ProcessFog( unsigned long dt )
{
	float d = dt * 0.0002f;

	if( mFogColor != mTargetFogColor )
	{
		if( mFogColor.r < mTargetFogColor.r )
		{
			mFogColor.r += d;

			if( mFogColor.r > mTargetFogColor.r )
				mFogColor.r = mTargetFogColor.r;
		}
		else
		{
			mFogColor.r -= d;

			if( mFogColor.r < mTargetFogColor.r )
				mFogColor.r = mTargetFogColor.r;
		}
		if( mFogColor.g < mTargetFogColor.g )
		{
			mFogColor.g += d;

			if( mFogColor.g > mTargetFogColor.g )
				mFogColor.g = mTargetFogColor.g;
		}
		else
		{
			mFogColor.g -= d;

			if( mFogColor.g < mTargetFogColor.g )
				mFogColor.g = mTargetFogColor.g;
		}
		if( mFogColor.b < mTargetFogColor.b )
		{
			mFogColor.b += d;

			if( mFogColor.b > mTargetFogColor.b )
				mFogColor.b = mTargetFogColor.b;
		}
		else
		{
			mFogColor.b -= d;

			if( mFogColor.b < mTargetFogColor.b )
				mFogColor.b = mTargetFogColor.b;
		}

		TERRAIN->SetFogColor( mFogColor );
		SCENEMAN->SetFogColor( mFogColor );

		NiRenderer* renderer = NiRenderer::GetRenderer();
		if( renderer )
			renderer->SetBackgroundColor( mFogColor );
	}

	if( mFogDepth != mTargetFogDepth )
	{
		if( mFogDepth < mTargetFogDepth )
		{
			mFogDepth += d;

			if( mFogDepth > mTargetFogDepth )
				mFogDepth = mTargetFogDepth;
		}
		else
		{
			mFogDepth -= d;

			if( mFogDepth < mTargetFogDepth )
				mFogDepth = mTargetFogDepth;
		}

		TERRAIN->SetFogDepth( mFogDepth );
		SCENEMAN->SetFogDepth( mFogDepth );
	}
}

void cWorldManager::RenderSkyDome()
{
	NiRenderer* renderer = NiRenderer::GetRenderer();

	/// 하늘을 렌더링
	if( mSkyEnabled && mSkyDome )
	{
		cRenderObjList::cIterator i = mSkyGeomList.Begin();
		cRenderObjList::cIterator end = mSkyGeomList.End();

		for( ; i != end; ++i )
		{
			((NiRenderObject*)(*i))->RenderImmediate( renderer );
		}
	}
}

void cWorldManager::Render()
{
	/// 지형을 렌더링
	mTerrain->Render();
}

void cWorldManager::SetArea( const cAreaSceneNode& n )
{
	/// 주변광
	mTargetSkyAmbient = n.GetSkyAmbient();
	mTargetSkyDimmer = n.GetSkyDimmer();
	mTargetTerrainAmbient = n.GetTerrainAmbient();
	mTargetTerrainDimmer = n.GetTerrainDimmer();
	mTargetStaticObjectAmbient = n.GetStaticObjectAmbient();
	mTargetStaticObjectDimmer = n.GetStaticObjectDimmer();
	mTargetDynamicObjectAmbient = n.GetDynamicObjectAmbient();
	mTargetDynamicObjectDimmer = n.GetDynamicObjectDimmer();

	/// 안개
	mTargetFogColor = n.GetFogColor();
	mTargetFogDepth = n.GetFogDepth();
}

void cWorldManager::BlendArea( const cAreaSceneNode& n0, const cAreaSceneNode& n1, float r )
{
	/// 주변광
	mTargetSkyAmbient = n0.GetSkyAmbient() + (n1.GetSkyAmbient() - n0.GetSkyAmbient()) * r;
	mTargetSkyDimmer = n0.GetSkyDimmer() + (n1.GetSkyDimmer() - n0.GetSkyDimmer()) * r;
	mTargetTerrainAmbient = n0.GetTerrainAmbient() + (n1.GetTerrainAmbient() - n0.GetTerrainAmbient()) * r;
	mTargetTerrainDimmer = n0.GetTerrainDimmer() + (n1.GetTerrainDimmer() - n0.GetTerrainDimmer()) * r;
	mTargetStaticObjectAmbient = n0.GetStaticObjectAmbient() + (n1.GetStaticObjectAmbient() - n0.GetStaticObjectAmbient()) * r;
	mTargetStaticObjectDimmer = n0.GetStaticObjectDimmer() + (n1.GetStaticObjectDimmer() - n0.GetStaticObjectDimmer()) * r;
	mTargetDynamicObjectAmbient = n0.GetDynamicObjectAmbient() + (n1.GetDynamicObjectAmbient() - n0.GetDynamicObjectAmbient()) * r;
	mTargetDynamicObjectDimmer = n0.GetDynamicObjectDimmer() + (n1.GetDynamicObjectDimmer() - n0.GetDynamicObjectDimmer()) * r;

	/// 안개
	mTargetFogColor = n0.GetFogColor() + (n1.GetFogColor() - n0.GetFogColor()) * r;
	mTargetFogDepth = n0.GetFogDepth() + (n1.GetFogDepth() - n0.GetFogDepth()) * r;
}

bool cWorldManager::LoadSkyDome( const cString& pathName )
{
	return LoadSkyDome( pathName, GetSkyScale(), mSkyEnabled );
}

bool cWorldManager::LoadSkyDome( const cString& pathName, float scale, bool enabled )
{
	NiStream stream;

	if( stream.Load( pathName.Cstr() ) == false )
	{
		return false;
	}

	mSkyDome = NiDynamicCast(NiNode, stream.GetObjectAt(0));
	if( mSkyDome == 0 )
		return false;

	stream.RemoveAllObjects();

	///
	mSkyDome->SetScale( scale );
	mSkyEnabled = enabled;

	/// 기하 노드들을 수집
	mSkyGeomList.Clear();
	mSkyMatPropList.Clear();
	CollectGeometries( &mSkyGeomList, mSkyDome );
	CollectProperties( &mSkyMatPropList, mSkyDome );

	/// 재질을 설정
//	cMaterialData matData;
//	matData.mAmbient = NiColor::WHITE;
//	matData.mDiffuse = NiColor::WHITE;
//	matData.mSpecular = NiColor::BLACK;
//	matData.mEmissive = NiColor::BLACK;
//	SetMaterial( mSkyMatPropList, matData );
	DisableZBuffer( mSkyDome );

	/// 하늘 주변광을 부착
	mSkyDome->AttachEffect( mSkyLight );

	///
//	mSkyDome->AttachProperty( TERRAIN->GetFogProperty() );

	/// 이름을 설정
	::GetFileName( &mSkyName, pathName );

	/// 갱신
	mSkyDome->SetSelectiveUpdate( true );
	mSkyDome->SetSelectiveUpdateTransforms( true );
	mSkyDome->SetSelectiveUpdatePropertyControllers( true );
	mSkyDome->SetSelectiveUpdateRigid( false );

	mSkyDome->UpdateProperties();
	mSkyDome->UpdateEffects();

	mSkyDome->Update( 0.0f );
	mSkyDome->UpdateNodeBound();
	return true;
}

void cWorldManager::CollectGeometries( tList<NiRenderObject*>* geomList, NiAVObject* obj )
{
	assert( geomList && "null geom list" );
	assert( obj && "null object" );

	if( NiIsKindOf( NiMesh, obj ) )
	{
		geomList->PushBack( (NiRenderObject*)obj );
	}
	else if( NiIsKindOf( NiNode, obj ) )
	{
		NiNode* node = (NiNode*)obj;

		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			NiAVObject* child = node->GetAt( i );
			if( child )
				CollectGeometries( geomList, child );				
		}
	}
}

void cWorldManager::CollectProperties( tList<NiMaterialProperty*>* propList, NiAVObject* obj )
{
	assert( propList && "null property list" );
	assert( obj && "null object" );

	if( NiIsKindOf( NiMesh, obj ) )
	{
		NiMaterialProperty* matProp = (NiMaterialProperty*)obj->GetProperty( NiProperty::MATERIAL );

		if( matProp )
		{
			propList->PushBack( matProp );
		}
	}
	else if( NiIsKindOf( NiNode, obj ) )
	{
		NiNode* node = (NiNode*)obj;

		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			NiAVObject* child = node->GetAt( i );
			if( child )
				CollectProperties( propList, child );				
		}
	}
}

void cWorldManager::SetMaterial( tList<NiMaterialProperty*>& propList, const cMaterialData& material )
{
	cMatPropList::cIterator i = propList.Begin();
	cMatPropList::cIterator end = propList.End();

	for( ; i != end; ++i )
	{
		NiMaterialProperty* matProp = *i;

		matProp->SetAmbientColor( material.mAmbient );
		matProp->SetDiffuseColor( material.mDiffuse );
		matProp->SetSpecularColor( material.mSpecular );
		matProp->SetEmittance( material.mEmissive );
		matProp->SetShineness( material.mShininess );
		matProp->SetAlpha( material.mAlpha );
	}
}

void cWorldManager::DisableZBuffer( NiAVObject* obj )
{
	NiProperty* prop = obj->GetProperty( NiZBufferProperty::GetType() );
	if( prop )
	{
		((NiZBufferProperty*)prop)->SetZBufferTest(false);
		((NiZBufferProperty*)prop)->SetZBufferWrite(false);
	}

	if( NiIsKindOf(NiNode, obj) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;

		for( unsigned int i = 0; i < node->GetArrayCount(); ++i )
		{
			child = node->GetAt( i );
			if( child )
				DisableZBuffer( child );
		}
	}
}
