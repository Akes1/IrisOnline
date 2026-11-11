#include "StdAfx.h"
#include "WorldManager.h"
#include "PathFinder.h"

#include "ObjectManager.h"
#include "Hero.h"
#include "SceneManager.h"
#include "AreaSceneNode.h"
#include "GameResourceManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "LightAgent.h"
#include "FogAgent.h"
#include "Terrain.h"
#include "NaviMesh.h"
#include "NaviField.h"
#include "MaterialData.h"

#include "BaseObject_Common.h"
#include "Application.h"

cWorldManager* cWorldManager::mSingleton = 0;

cWorldManager::cWorldManager()
: mSkyHeight( 0.f )
, mSkyNeedUpdate( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	/// 길찾기
	mPathFinder = new cPathFinder( 4096 );

	/// 지형을 생성
	mTerrain = new cTerrain;

	/// 네비메시를 생성
	mNaviMesh = new cNaviMesh;

	mpCurrentNaviField = 0;
	mNaviFieldLoadEnd  = false;

	mCurrentMapIndex = 0;//UINT_MAX;
	mSkyDome = 0;

	/// 하늘 주변광을 생성
	//mpSkyLight = NiNew NiAmbientLight;
	//mpSkyLight->SetAmbientColor( NiColor::BLACK );

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
	mFogDepth = 0.5f;

	mTargetFogColor = NiColor::WHITE;
	mTargetFogDepth = 0.5f;

//	mCollidableArray.Reserve( 64 );

	mArea1 = 0;
	mArea2 = 0;

}

cWorldManager::~cWorldManager()
{
	cNaviFieldMap::cIterator pos;
	cNaviFieldMap::cIterator end;

	pos = mNaviFieldMap.Begin();
	end = mNaviFieldMap.End();

	for( ; pos!=end; ++pos )
	{
		cNaviField* pNaviField = (cNaviField*)(*pos).mSecond;
		if( pNaviField )
		{
			pNaviField->Clear();
			SAFE_DELETE( pNaviField );
		}
	}
	mNaviFieldMap.Clear();
	mNaviFieldLoadEnd = false;

	SAFE_DELETE( mPathFinder );
	SAFE_DELETE( mNaviMesh );
	SAFE_DELETE( mTerrain );
	mSingleton = 0;
}

bool cWorldManager::Init()
{
	return true;
}

void cWorldManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mTerrain )
		mTerrain->Process();

	if( mSkyDome && mSkyNeedUpdate )
	{
		//NiMatrix3 matRot;

		/// 스카이 박스의 회전 속도도 지정 가능하도록 한다.
		//matRot.MakeZRotation( 0.00015f );
		//mpSkydome->SetRotate( mpSkydome->GetRotate() * matRot );

		mSkyNeedUpdate = false;
		mSkyDome->Update( (float)accumTime * 0.001f );
	}

	///
	cHero* hero = OBJECTMAN->GetHero();
	if( hero )
	{
		mCollidableArray.Clear();

		if( SCENEMAN->PickAreas( &mCollidableArray, hero->GetBoundSphere() ) )
		{
			if( mCollidableArray.GetSize() == 1 )
			{
				/// 충돌 영역과 전체 영역을 블렌딩
				cAreaSceneNode* n = (cAreaSceneNode*)mCollidableArray[0];
				if( mArea1 != n )
				{
					SetArea( *n );
					mArea1 = n;
					mArea2 = 0;
				}
			}
			else
			{
				/// 2개 이상 충돌하는 경우에도 2개의 영역만 블렌딩
				cAreaSceneNode* n0 = (cAreaSceneNode*)mCollidableArray[0];
				cAreaSceneNode* n1 = (cAreaSceneNode*)mCollidableArray[1];
				if( mArea1 != n0 && mArea2 != n1 )
				{
					float dist0 = (n0->GetCenter() - hero->GetCenter()).Length();
					float dist1 = (n1->GetCenter() - hero->GetCenter()).Length();
					float r = dist0 / (dist0 + dist1);
					BlendArea( *n0, *n1, r );

					mArea1 = n0;
					mArea2 = n1;
				}
			}
		}
		else
		{
			if( mArea1 != SCENEMAN->GetGlobalArea() )
			{
				SetArea( *SCENEMAN->GetGlobalArea() );
				mArea1 = SCENEMAN->GetGlobalArea();
			}
		}
	}
	else
	{
		assert(0);
		return;
	}

	/// 주변광
	ProcessAmbient( deltaTime );

	/// 안개
	ProcessFog( deltaTime );
}

void cWorldManager::ProcessAmbient( unsigned long  dt )
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

		LIGHTAGENT->SetSkyAmbient( mSkyAmbient );
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

		LIGHTAGENT->SetSkyDimmer( mSkyDimmer );
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

		LIGHTAGENT->SetTerrainAmbient( mTerrainAmbient );
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

		LIGHTAGENT->SetTerrainDimmer( mTerrainDimmer );
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

		LIGHTAGENT->SetSceneAmbient( 0, mStaticObjectAmbient );
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

		LIGHTAGENT->SetSceneDimmer( 0, mStaticObjectDimmer );
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

		LIGHTAGENT->SetSceneAmbient( 1, mDynamicObjectAmbient );
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

		LIGHTAGENT->SetSceneDimmer( 1, mDynamicObjectDimmer );
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

		FOGAGENT->SetFogColor( mFogColor );
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

		FOGAGENT->SetFogDepth( mFogDepth );
	}
}

void cWorldManager::Render()
{
	mTerrain->Render();
}

void cWorldManager::RenderSkyDome()
{
	NiRenderer* renderer = NiRenderer::GetRenderer();

	/// 하늘을 렌더링
	if( mSkyDome )
	{
		cRenderObjList::cIterator i = mSkyGeomList.Begin();
		cRenderObjList::cIterator end = mSkyGeomList.End();

		for( ; i != end; ++i )
		{
			((NiRenderObject*)(*i))->RenderImmediate( renderer );
		}
	}
}

void cWorldManager::Exit()
{
}

bool cWorldManager::Open()
{
	if( SCENEMAN )
	{
		SetArea( *SCENEMAN->GetGlobalArea() );

		mArea1 = SCENEMAN->GetGlobalArea();
		mArea2 = 0;
	}

	return true;
}

void cWorldManager::ClearSkyDome()
{
	if( mSkyDome )
		mSkyDome = 0;
}

void cWorldManager::Close()
{
	/// 스카이돔 제거
	ClearSkyDome();

	/// 맵 해제..
	if( mTerrain )
		mTerrain->Clear();

	if( mNaviMesh )
		mNaviMesh->Clear();

	if( mPathFinder	)
		mPathFinder->Clear();

	mSkyMatPropList.Clear();
	mSkyGeomList.Clear();

	mCollidableArray.Clear();
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

	NiColorA color;
	color = NiColorA(mTargetFogColor.r, mTargetFogColor.g, mTargetFogColor.b, 0.0f );

	RENDERSYS->SetClearColor( color );
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

bool cWorldManager::LoadSkydome( const cString& pathName, float scale )
{
	RESOURCEMAN->LoadMapNIF( pathName );
	mSkyDome = RESOURCEMAN->GetObjectByName( pathName );
	if( mSkyDome == 0 )
		return false;

	mSkyDome->SetScale( scale );
	mSkyPathName = pathName;

	/// 기하 노드들을 수집
	mSkyGeomList.Clear();
	mSkyMatPropList.Clear();
	CollectGeometries( &mSkyGeomList, mSkyDome );
	CollectProperties( &mSkyMatPropList, mSkyDome );

	/// 재질을 설정
	cMaterialData matData;
	matData.mAmbient = NiColor::WHITE;
	matData.mDiffuse = NiColor::WHITE;
	matData.mSpecular = NiColor::BLACK;
	matData.mEmissive = NiColor::BLACK;
	SetMaterial( mSkyMatPropList, matData );
	DisableZBuffer( mSkyDome );

	/// 하늘 주변광을 부착
	LIGHTAGENT->AttachSkyLight( mSkyDome );

	/// 하늘 안개를 적용
	mSkyDome->AttachProperty( FOGAGENT->GetFogProperty() );
/*
	NiFogProperty* temp = NiNew NiFogProperty;
	temp->SetFog( false );

	NiAVObject* obj = mSkyDome->GetObjectByName( "skybox01" );
	if( obj )
		obj->AttachProperty( temp );
	obj = mSkyDome->GetObjectByName( "skybox02" );
	if( obj )
		obj->AttachProperty( temp );
*/

	/// 갱신
	mSkyDome->SetSelectiveUpdate( true );
	mSkyDome->SetSelectiveUpdateTransforms( true );
	mSkyDome->SetSelectiveUpdatePropertyControllers( true );
	mSkyDome->SetSelectiveUpdateRigid( false );

	mSkyDome->UpdateProperties();
	mSkyDome->UpdateEffects();

	NiMeshUpdateProcess kUpdateProcess;
	mSkyDome->Update( kUpdateProcess );
	mSkyDome->UpdateNodeBound();
	return true;
}

void cWorldManager::CollectGeometries( tList<void*>* geomList, NiAVObject* pobj )
{
	assert( geomList && "null geom list" );

	if( pobj == 0 )
		return;

	if( NiIsKindOf( NiMesh, pobj ) )
	{
		geomList->PushBack( (NiRenderObject*)pobj );
	}
	else if( NiIsKindOf( NiNode, pobj ) )
	{
		NiNode* node = (NiNode*)pobj;
		NiAVObject* child = 0;
		for( int i = 0, end = node->GetArrayCount(); i < end; ++i )
		{
			child = node->GetAt( i );
			if( child )
				CollectGeometries( geomList, child );
		}
	}
}

void cWorldManager::CollectProperties( tList<void*>* propList, NiAVObject* obj )
{
	assert( propList && "null property list" );
	assert( obj && "null object" );

	if( NiIsKindOf( NiMesh, obj ) )
	{
		NiMaterialProperty* matProp = (NiMaterialProperty*)obj->GetProperty(NiProperty::MATERIAL);
		if( matProp )
			propList->PushBack( matProp );
	}
	else if( NiIsKindOf( NiNode, obj ) )
	{
		NiNode* node = (NiNode*)obj;
		NiAVObject* child = 0;
		for( unsigned int i = 0, end = node->GetArrayCount(); i < end; ++i )
		{
			child = node->GetAt( i );
			if( child )
				CollectProperties( propList, child );				
		}
	}
}

void cWorldManager::SetMaterial( tList<void*>& propList, const cMaterialData& material )
{
	cMatPropList::cIterator i = propList.Begin();
	cMatPropList::cIterator end = propList.End();

	for( ; i != end; ++i )
	{
		NiMaterialProperty* matProp = (NiMaterialProperty*)(*i);
		if( matProp == 0 )
			continue;

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
	if( obj == 0 )
		return;

	NiZBufferProperty* prop = (NiZBufferProperty*)obj->GetProperty( NiProperty::ZBUFFER );
	if( prop )
	{
		prop->SetZBufferTest(false);
		prop->SetZBufferWrite(false);
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
/*
void cWorldManager::RecursivePrepack( NiAVObject* pobj )
{
	if( pobj->GetAppCulled() )
	{
		return;
	}

	NiRenderer* pRenderer = NiRenderer::GetRenderer();
	/// 솔직히 여기 걸린다는건 그냥 죽는게..
	assert(pRenderer);

	bool isDynamic = false;
	if( !(pRenderer->GetFlags() & NiRenderer::CAPS_HARDWARESKINNING) )
	{
		isDynamic = true;
	}

	if( NiIsKindOf(NiGeometry, pobj) )
	{
		NiGeometry* pGeom = (NiGeometry*)pobj;
		if( NiIsKindOf(NiParticles, pobj) )
			isDynamic = true;

		/// Search for morpher controllers
		NiTimeController* pController;
		for( pController = pobj->GetControllers(); pController!=0; pController = pController->GetNext() )
		{
			if( NiIsKindOf(NiGeomMorpherController, pController) )
				isDynamic = true;
		}

		NiGeometryData::Consistency flags = isDynamic ? NiGeometryData::VOLATILE : NiGeometryData::STATIC;
		pGeom->SetConsistency( flags );
		pGeom->GetModelData()->SetCompressFlags( NiGeometryData::COMPRESS_ALL );
		pGeom->GetModelData()->SetKeepFlags( NiGeometryData::KEEP_XYZ |
									NiGeometryData::KEEP_NORM | NiGeometryData::KEEP_INDICES );

		pRenderer->PrecacheMesh( pGeom, 0, 0 );
//		pRenderer->PrecacheGeometry( pGeom, 0, 0 );
	}
	else if( NiIsKindOf(NiNode, pobj) )
	{
		NiAVObject* pChild = 0;
		NiNode* pNode = (NiNode*)pobj;
		for( unsigned int i = 0; i<pNode->GetArrayCount(); ++i )
		{
			pChild = pNode->GetAt( i );
			if( pChild )
				RecursivePrepack( pChild );
		}
	}
}
*/
bool cWorldManager::Pick( NiPoint3* pos, int mouseX, int mouseY )
{
	assert( pos );
	assert( mNaviMesh );

	return mNaviMesh->Pick( pos, mouseX, mouseY );
}

bool cWorldManager::CalcHeight( float* height, float posX, float posY ) 
{ 
	assert( height );
	assert( mNaviMesh );

	return mNaviMesh->CalcHeight( height, posX, posY );
}

bool cWorldManager::EnableMove( int mouseX, int mouseY )
{
	if( !mpCurrentNaviField )
	{
		assert(0);
		return false;
	}

	NiPoint3 pos;
	if( Pick( &pos, mouseX, mouseY ) == false )
		return false;

	unsigned int x = (unsigned int)((pos.x + 50.0f) / 100.0f);
	unsigned int y = (unsigned int)((pos.y + 50.0f) / 100.0f);

	return (mpCurrentNaviField->GetValue( x, y ) == 0);
}

bool cWorldManager::EnableRayGet( NiPoint3 start, NiPoint3 &goal )
{
	NiPoint2 s,g;
	s = NiPoint2(start.x, start.y);
	g = NiPoint2(goal.x, goal.y);

	bool check = mPathFinder->GetPossibleGoal( s, g );

	goal.x = g.x;
	goal.y = g.y;

	return check;
}

bool cWorldManager::EnableRayCheck( NiPoint2 start, NiPoint2 goal )
{
	return mPathFinder->IsPossible( start, goal );
}

void cWorldManager::MoveSkyDome( const NiPoint3& pos )
{
	if( mSkyDome )
	{
		mSkyDome->SetTranslate( pos.x, pos.y, pos.z );//0.0f );
		mSkyNeedUpdate = true;
	}
}

bool cWorldManager::LoadTerrainHeader( unsigned int& loadCount, unsigned int mapIdx, unsigned char mapMode )
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );

	/// 파일명 생성 루틴 추가!!
	cString mapfile;
	mapfile.Format( "./data/map/%03d.terrain", folderIdx );

	if( mTerrain->LoadHeader( loadCount, mapfile.Cstr() ) == false )
	{
		assert( 0 && "failed to load terrain" );
		return false;
	}
	return true;
}

int cWorldManager::LoadTerrainNodes( unsigned int count )
{
	return mTerrain->LoadNodes( count );
}

bool cWorldManager::LoadNaviMeshHeader( unsigned int& loadCount, unsigned int mapIdx, unsigned char mapMode )
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );

	/// 파일명 생성 루틴 추가!!
	cString mapfile;
	mapfile.Format( "./data/map/%03d.navimesh", folderIdx );

	if( mNaviMesh->LoadHeader( loadCount, mapfile.Cstr() ) == false )
	{
		assert( 0 && "failed to load navimesh" );
		return false;
	}
	return true;
}

int cWorldManager::LoadNaviMeshNodes( unsigned int count )
{
	return mNaviMesh->LoadNodes( count );
}

bool cWorldManager::LoadNaviField( unsigned int mapIdx, unsigned char mapMode )
{
	if( !mNaviFieldLoadEnd )
	{
		typedef tPointerHashMap<unsigned long, void*> cArrayHashMap;
		cArrayHashMap* mapArr = GAMERESOURCEMAN->GetMapInfo();

		cArrayHashMap::cConstIterator pos;
		cArrayHashMap::cConstIterator end;
		if( mapArr )
		{
			pos = mapArr->Begin();
			end = mapArr->End();
		}
		else
			pos = end;

		/// 난이도에 따른 중복된 맵을 읽지 않도록 체크
		tHashSet<unsigned long> loadMapFolderSet;

		for( ; pos != end; ++pos )
		{
			cArrayHashMap* mapModeArr = (cArrayHashMap*)(*pos).mSecond;
			unsigned long mapMode = 0;

			if( mapModeArr )
			{
				cArrayHashMap::cConstIterator pos2 = mapModeArr->Begin();

				sMapInfo* pMapInfo = (sMapInfo*)(*pos2).mSecond;
				if( pMapInfo )
				{
					/// 중복된 맵 체크
					if( loadMapFolderSet.Find( pMapInfo->folderIdx ) != loadMapFolderSet.End() )
						continue;

					loadMapFolderSet.Insert( pMapInfo->folderIdx );
					mapMode = pMapInfo->mapMode;

					/// 네비필드를 생성
					cNaviField* pNaviField = new cNaviField;

					if( !pNaviField )
					{
						assert(0);

						if( pNaviField )
							pNaviField->Clear();

						SAFE_DELETE( pNaviField );

						return false;
					}

					/// 파일 정보 생성 루틴
					cString naviMapFile;
					naviMapFile.Format( "./data/map/%03d.navifield", pMapInfo->folderIdx );

					if( pNaviField->Load( naviMapFile.Cstr() ) == false )
					{
						assert( 0 && "failed to load navifield" );
						return false;
					}

					/// 네비필드 정보 삽입
					if( mNaviFieldMap.Insert( pMapInfo->folderIdx, pNaviField ) == false )
						return false;
				}
				else
					assert(0);
			}
			else
				assert(0);
		}

		mNaviFieldLoadEnd = true;
	}

	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );

	cNaviFieldMap::cIterator pos = mNaviFieldMap.Find( folderIdx );

	if( pos == mNaviFieldMap.End() )
	{
		/// 미리 로드했는데 못 찾았으면 잘못된 것임.
		assert(0);

		/// 파일명 생성 루틴 추가!!
		cString mapfile;
		mapfile.Format( "./data/map/%03d.navifield", folderIdx );

		cNaviField* pNaviField = 0;
		pNaviField = new cNaviField;

		if( pNaviField->Load( mapfile.Cstr() ) == false )
		{
			assert( 0 && "failed to load navifield" );
			SAFE_DELETE( pNaviField );
			return false;
		}

		/// 길찾기
		mPathFinder->Init( pNaviField );

		if( mNaviFieldMap.Insert( folderIdx, pNaviField ) == false )
		{
			assert(0);
			pNaviField->Clear();
			SAFE_DELETE( pNaviField );
		}
		else
			mpCurrentNaviField = pNaviField;
	}
	else
	{
		mpCurrentNaviField = (cNaviField*)(*pos).mSecond;

		/// 길찾기
		mPathFinder->Init( mpCurrentNaviField );
	}
	return true;
}

cNaviField* cWorldManager::GetNaviField( int mapIndex, unsigned char mapMode ) const
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIndex, mapMode );

	cNaviFieldMap::cConstIterator pos = mNaviFieldMap.Find( folderIdx );

	if( pos != mNaviFieldMap.End() )
	{
		return (cNaviField*)(*pos).mSecond;	
	}

	return 0;
}