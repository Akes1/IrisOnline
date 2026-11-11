#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
// #include "CameraManager.h"
// #include "StageScript.h"
// 
// #include "Terrain.h"
#include "AreaGroup.h"
#include "AreaSceneNode.h"
#include "LightSceneNode.h"
#include "SoundSceneNode.h"
#include "PlayerSceneNode.h"
// #include "TransMonSceneNode.h"
// #include "VehicleSceneNode.h"
#include "NpcSceneNode.h"
#include "EffectSceneNode.h"
#include "DropItemSceneNode.h"
#include "GatheringSceneNode.h"
#include "MapPortalSceneNode.h"
// #include "CardSceneNode.h"
 #include "DoorSceneNode.h"
// #include "Signboard.h"
// 
// #include "WorldManager.h"
// #include "NaviMesh.h"
// #include "ShadowGeometry.h"
// 
// #include "ObjectManager.h"
// #include "Hero.h"
// 
#include "soundsystem.h"
// #include "Application.h"
// 
// #if (defined(_DEVSYS) || defined(_DEBUG))
// #include "DevSystem.h"
// #endif

///// 장면 노드 비교 함수자
///// 픽킹 거리로 비교한다.
//bool cDynamicSceneNodeCompareByCamDistance( const unsigned long left, const unsigned long right )
//{
//	/// 
//	cDynamicSceneNode* node1 = SCENEMAN->GetDynamicSceneNode( left );
//	cDynamicSceneNode* node2 = SCENEMAN->GetDynamicSceneNode( right );
//
//	if( node1 == 0 )
//	{
//		assert(0);
//	}
//	if( node2 == 0 )
//	{
//		assert(0);
//	}
//
//	return ( node1->mVisibleCamLength < node2->mVisibleCamLength );
//};

cSceneManager* cSceneManager::mSingleton = 0;

cSceneManager::cSceneManager()
: mVersion( 0 )
, mAlphaArray( 1024, 256 )
, mAlphaTestArray( 1024, 256 )
, mSolidArray( 1024, 256 )
, mDamageArray( 256, 64 )
// , mCuller( &mAlphaArray, &mAlphaTestArray, &mSolidArray )
// , mDamageCuller( &mDamageArray, &mAlphaTestArray, &mSolidArray )
, mAlphaArraySorter( &mAlphaArray, &mAlphaTestArray )
, mDamageSorter( &mDamageArray, 0 )
, mSolidArraySorter( &mSolidArray )
, mTextureMap( 22 )
, mScreenShotMode( false )
, mShowDepthPlane( true )
, mEnableLoading( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mHeroPos = NiPoint3::ZERO;

	/// 전체 영역 노드를 생성
	cAreaSceneNodeParam param;
	mGlobalArea = new cAreaSceneNode;

	if( mGlobalArea->Init( param ) == false )
	{
		delete mGlobalArea;
		mGlobalArea = 0;

		assert( 0 && "failed to init global area scene node" );
	}

	///
	mPickedArray.Reserve( 1024 );

	/// 가시 레벨 거리
	mSqaredLevelDistance0 = 5000.0f * 5000.0f;
	mSqaredLevelDistance1 = 10000.0f * 10000.0f;
	mSqaredLevelDistance2 = 20000.0f * 20000.0f;
}

cSceneManager::~cSceneManager()
{
	SAFE_DELETE(mGlobalArea);
	Clear();

	mSingleton = 0;
}

bool cSceneManager::Init()
{
	///
	mPlaneArray.Reserve( 2048 );
	mVisibleDynamicNodeArray.Reserve( 2048 );

	/// 그림자 가시 배열
	mShadowGeomArray.Reserve( 2048 );
	mTrailGeomArray.Reserve( 100 );
	return true;
}

void cSceneManager::Exit()
{
	Clear();
}

void cSceneManager::Clear()
{

	/// 영역 그룹들을 제거
	cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
	cAreaGroupMap::cIterator iend = mAreaGroupMap.End();

	for( ; i != iend; ++i )
	{
		delete (cAreaGroup*)(i->mSecond);
	}
	mAreaGroupMap.Clear();
	cAreaGroup::mCurrentGroup = 0;

	DestroyAll();

	mVisibleDynamicNodeArray.Clear();

	///
	mAlphaArray.RemoveAll();
	mAlphaTestArray.RemoveAll();
	mSolidArray.RemoveAll();
//	mDamageArray.RemoveAll();

	mPickedArray.Clear();

	///
	mShadowGeomArray.Clear();
	mTrailGeomArray.Clear();

	mTextureMap.Clear();

	mShowDepthPlane = true;
}

bool cSceneManager::Init( const NiPoint3& center, float minRadius, float maxRadius )
{
	/// 지움
	Clear();

	/// 트리를 재생성
	mNodeTree.Init( center, minRadius, maxRadius );
	return true;
}

// void cSceneManager::Process( unsigned long deltaTime, unsigned long accumTime )
// {
// 	cCamera* cam = CAMERAMAN->GetCurrent();
// 	if( cam == 0 )
// 	{
// 		assert( 0 );
// 		return;
// 	}
// 
// 	///
// 	mPlaneArray.Clear();
// 	mVisibleDynamicNodeArray.Clear();
// 	mShadowGeomArray.Clear();
// 	mTrailGeomArray.Clear();
// 
// 	mSolidArray.RemoveAll();
// 	mAlphaArray.RemoveAll();
// 	mAlphaTestArray.RemoveAll();
// 	mDamageArray.RemoveAll();
// 
// 	/// 삭제 등록된 노드 처리
// 	DelNodeProcess();
// 
// 	if( mNodeTree.IsEmpty() )
// 		return;
// 
// 	if( mEnableLoading == true )
// 		return;
// 
// #ifdef _GMTOOL
// 	if( CAMERAMAN->GetCurrentIdx() == 2 )
// 	{
// 		mHeroPos = cam->GetWorldTranslate();
// 	}
// 	else
// 	{
// #endif
// 	mHeroPos = cam->GetLookAt();
// 
// 	/// 카메라 충돌 검사
// 	if( cam->GetNeedUpdate() )
// 	{
// 		float lookAtDist = cam->GetLookAtDistance();
// 		NiPoint3 invDir = -cam->GetWorldDirection();
// 		invDir.Unitize();
// 
// 		/// 차폐물과 충돌 검사
// 		bool collided = false;
// 		NiPoint3 pos;
// 		float dist = lookAtDist + 120.0f;
// 		mPickedArray.Clear();
// 
// 		if( PickStatics( &mPickedArray, cRay(mHeroPos, invDir), true, lookAtDist + 120.0f, true ) )
// 		{
// 			if( mPickedArray.IsEmpty() == false )
// 			{
// 				cStaticSceneNode* n = (cStaticSceneNode*)mPickedArray[0];
// //				if( n->GetOccludeLevel() == 2 )
// //				{
// 					collided = true;
// 					dist = n->GetPickDistance() - 120.0f;
// 
// 					if( dist < lookAtDist )
// 						lookAtDist = dist;
// //				}
// 			}
// 		}
// 
// 		/// 네비메시와 충돌 검사
// 		dist = lookAtDist + 120.0f;
// 
// 		if( NAVIMESH->Pick( &pos, &dist, cRay(mHeroPos, invDir), lookAtDist + 120.0f ) )
// 		{
// 			collided = true;
// 			lookAtDist = dist - 120.0f;
// 		}
// 
// 		/// 카메라를 갱신
// 		if( collided )
// 		{
// 			cam->SetLookAtDistance( lookAtDist );
// 			cam->SetTranslate( mHeroPos + invDir * lookAtDist );
// 			cam->Update( accumTime );
// 		}
// 
// 		/// 시점과의 거리가 너무 짧으면 투명 처리
// 		if( HERO )
// 		{
// 			const float ALPHA_LOOKAT_DIST = 160.0f;
// 
// 			if( lookAtDist <= ALPHA_LOOKAT_DIST )
// 				HERO->SetAlphaBlended( 0.0f );
// 			else
// 				HERO->SetAlphaBlended( 1.0f );
// 		}
// 	}
// #ifdef _GMTOOL
// 	}
// #endif
// 
// 	/// 사운드 노드들을 처리
// 	cSphere testSphere;
// 	{
// 		testSphere.Set( mHeroPos, 100.0f );
// 		mPickedArray.Clear();
// 
// 		/// 충돌 체크
// 		Pick_Type( &mPickedArray, testSphere, cSceneNode::eSOUND );
// 
// 		/// 처리
// 		cSoundMap::cIterator i, iend;
// 		i = mSoundArray.Begin();
// 		iend = mSoundArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cSoundSceneNode* n = (cSoundSceneNode*)(*i).mSecond;
// 			if( n->IsRemoved() == false )
// 				n->OnProcess( deltaTime, accumTime );
// 		}
// 	}
// /*
// 	/// 조명 노드들을 처리
// 	{
// 		testSphere.Set( mHeroPos, 10000.0f );
// 		mPickedArray.Clear();
// 
// 		if( Pick_Type( &mPickedArray, testSphere, cSceneNode::eLIGHT ) )
// 		{
// 			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
// 			{
// 				((cLightSceneNode*)mPickedArray[i])->OnProcess( deltaTime, accumTime );
// 			}
// 		}
// 	}
// //*/
// 
// 	/// 정적 노드들을 처리
// 	{
// 		cNodeMap::cIterator i, iend;
// 		i = mStaticObjectArray.Begin();
// 		iend = mStaticObjectArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cStaticSceneNode* n = (cStaticSceneNode*)(*i).mSecond;
// //			if( n->IsViewNode() )
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 	}
// 
// 	///
// 	{
// 		cNodeMap::cIterator i, iend;
// 		i = mDynamicObjectArray.Begin();
// 		iend = mDynamicObjectArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cDynamicSceneNode* n = (cDynamicSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 
// 		i = mMapPortalArray.Begin();
// 		iend = mMapPortalArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cMapPortalSceneNode* n = (cMapPortalSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 
// 		i = mDropItemArray.Begin();
// 		iend = mDropItemArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cDropItemSceneNode* n = (cDropItemSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 
// 		i=  mGatheringArray.Begin();
// 		iend = mGatheringArray.End();
// 		for(; i != iend; ++i )
// 		{
// 			cGatheringSceneNode* n = (cGatheringSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 
// 		i = mDoorArray.Begin();
// 		iend = mDoorArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cDoorSceneNode* n = (cDoorSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 		}
// 	}
// 
// 	/// 영역 그룹들을 처리
// 	testSphere.Set( mHeroPos, 10.0f );
// 	{
// 		cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
// 		cAreaGroupMap::cIterator iend = mAreaGroupMap.End();
// 
// 		for( ; i != iend; ++i )
// 		{
// 			((cAreaGroup*)i->mSecond)->OnProcess( deltaTime, accumTime, testSphere );
// 		}
// 	}
// 
// 	/*{
// 		if( mPlaneArray.IsEmpty() == false )
// 		{
// 			for( unsigned int i = 0, end = mPlaneArray.GetSize(); i < end; ++i )
// 			{
// 				cPlane* n = (cPlane*)mPlaneArray[i];
// 				n->();
// 			}
// 		}
// 	}*/
// 
// 	{
// 		cNodeMap::cIterator i, iend;
// 		i = mEffectNodeArray.Begin();
// 		iend = mEffectNodeArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cEffectSceneNode* n = (cEffectSceneNode*)(*i).mSecond;
// 			n->OnProcess( deltaTime, accumTime );
// 
// 			if( n->IsRemoved() || n->IsViewNode() == false )
// 				continue;
// 
// 			if( n->IsEnableFrustumFlag() )
// 			{
// 				if( n->IsDamageEffect() )
// 					mDamageCuller.Cull( *cam, n );
// 				else
// 					mCuller.Cull( *cam, n );
// 			}
// 		}
// 	}
// 
// 	DelNodeProcess();
// 
// 	/// 컬링
// 	mNodeTree.Cull( &mSolidArray, &mAlphaArray, &mAlphaTestArray, cam, cSceneNode::eRENDERING );
// 
// 	/// mVisibleDynamicNodeArray sorting & plane등록
// 	if( IsScreenShotMode() == false && mVisibleDynamicNodeArray.IsEmpty() == false )
// 	{
// 		if( mVisibleDynamicNodeArray.GetSize() > 1 )
// 			::Sort( mVisibleDynamicNodeArray.Begin(), mVisibleDynamicNodeArray.End(), cDynamicSceneNodeCompareByCamDistance );
// 
// 		for( int i = mVisibleDynamicNodeArray.GetSize()-1; i >= 0; --i )
// 		{
// 			cDynamicSceneNode* n = GetDynamicSceneNode( mVisibleDynamicNodeArray[i] );
// 			if( n == 0 )
// 			{
// 				assert(0);
// 				continue;
// 			}
// 
// 			if( mShowDepthPlane == true )
// 				n->AddDepthPlaneObject();
// 		}
// 	}
// 
//    	/// 정렬
// 	mSolidArraySorter.Sort( *cam );
// 	mAlphaArraySorter.Sort( *cam );
// 	mDamageSorter.Sort( *cam );
// 
// 	if( mShadowGeomArray.GetSize() > 1 )
// 		::Sort( mShadowGeomArray.Begin(), mShadowGeomArray.End(), cShadowGeomCompareForRendering() );
// 
// 
// #if (defined(_DEVSYS) || defined(_DEBUG))
// 	if( DEVSYSTEM )
// 	{
// 		DEVSYSTEM->mSolidRenderCount = mSolidArray.GetCount();
// 		DEVSYSTEM->mAlphaRenderCount = mAlphaArray.GetCount();
// 		DEVSYSTEM->mAlphaTestRenderCount = mAlphaTestArray.GetCount();
// 		DEVSYSTEM->mDamageRenderCount = mDamageArray.GetCount();
// 	}
// #endif
// }
// 
// void cSceneManager::Render()
// {
// 	if( mEnableLoading == true )
// 		return;
// 
// 	/// 솔리드 개체
// 	mSolidArraySorter.Render();
// 
// 	/// 그림자
// 	RenderShadow();
// 
// 	/// 알파 개체
// 	mAlphaArraySorter.Render();
// 
// 	mSolidArray.RemoveAll();
// 	mAlphaArray.RemoveAll();
// 	mAlphaTestArray.RemoveAll();
// 
// 	RenderTrail();
// }
// 
// void cSceneManager::RenderZFalseDamage()
// {
// 	if( mEnableLoading == true )
// 		return;
// 
// 	/// Damage Geom
// 	mDamageSorter.Render();
// 
// 	mDamageArray.RemoveAll();
// }
// 
// void cSceneManager::RenderShadow()
// {
// 	if( mShadowGeomArray.IsEmpty() == true )
// 		return;
// 
// 	cRenderer* renderer = RENDERSYS->GetRenderer();
// 	if( renderer == 0 )
// 	{
// 		assert( 0 );
// 		return;
// 	}
// 
// 	for( unsigned int i = 0,iend = mShadowGeomArray.GetSize(); i< iend; ++i )
// 	{
// 		cShadowGeometry* geom = mShadowGeomArray[i];
// 		if( geom )
// 			geom->Draw( renderer );
// 	}
// 
// 	mShadowGeomArray.Clear();
// }
// 
// void cSceneManager::RenderTrail()
// {
// 	if( mTrailGeomArray.IsEmpty() == true )
// 		return;
// 
// 	cRenderer* renderer = RENDERSYS->GetRenderer();
// 	if( renderer == 0 )
// 	{
// 		assert( 0 );
// 		return;
// 	}
// 
// 	for( unsigned int i = 0,iend = mTrailGeomArray.GetSize(); i< iend; ++i )
// 	{
// 		cRibbonGeometry* geom = mTrailGeomArray[i];
// 		if( geom )
// 			geom->RenderImmediate( renderer );
// 	}
// 
// 	mTrailGeomArray.Clear();
// }
// 
// 
// void cSceneManager::Render2D()
// {
// 	if( mEnableLoading == true )
// 		return;
// 
// 	/// 영역 그룹
// 	cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
// 	cAreaGroupMap::cIterator iend = mAreaGroupMap.End();
// 
// 	for( ; i != iend; ++i )
// 	{
// 		((cAreaGroup*)i->mSecond)->OnRender();
// 	}
// 
// 	/// 말풍선 객체 랜더링
// 	if( mPlaneArray.IsEmpty() == false )
// 	{
// 		for( unsigned int i = 0, end = mPlaneArray.GetSize(); i < end; ++i )
// 		{
// 			cPlane* n = (cPlane*)mPlaneArray[i];
// 			n->Draw();
// 		}
// 	}
// }
// 
// bool cSceneManager::Pick_Kind( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type )
// {
// 	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, type );
// }
// 
// bool cSceneManager::Pick_Type( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type )
// {
// 	return mNodeTree.CollideSphere_Type( pickedArray, sphere, type );
// }
// 
// bool cSceneManager::PickAreas( tArray<void*>* pickedArray, const cSphere& sphere )
// {
// 	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eAREA );
// }
// 
// bool cSceneManager::PickLights( tArray<void*>* pickedArray, const cSphere& sphere )
// {
// 	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eLIGHT );
// }
// 
// bool cSceneManager::PickSounds( tArray<void*>* pickedArray, const cSphere& sphere )
// {
// 	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eSOUND );
// }
// 
// bool cSceneManager::PickStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
// {
// 	NiPoint3 origin, dir;
// 	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) )
// 		return false;
// 
// 	return PickStatics( pickedArray, cRay(origin, dir), false, NI_INFINITY, sortByDistance );
// }
// 
// bool cSceneManager::PickStatics( tArray<void*>* pickedArray, const cRay& ray, bool camRay, float maxDistance, bool sortByDistance )
// {
// 	return mNodeTree.CollideRay_Kind( pickedArray, ray, camRay, maxDistance, sortByDistance, cSceneNode::eSTATIC );
// }
// 
// bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
// {
// 	NiPoint3 origin, dir;
// 	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) == false )
// 		return false;
// 
// 	return PickDynamics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
// }
// 
// bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
// {
// 	return mNodeTree.CollideRay_Kind( pickedArray, ray, false, maxDistance, sortByDistance, cSceneNode::eDYNAMIC );
// }
// 
// bool cSceneManager::PickManagedStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
// {
// 	NiPoint3 origin, dir;
// 	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) == false )
// 		return false;
// 
// 	return PickManagedStatics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
// }
// 
// bool cSceneManager::PickManagedStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
// {
// 	return mNodeTree.CollideRay_Kind( pickedArray, ray, false, maxDistance, sortByDistance, cSceneNode::eMANAGEDSTATIC );
// }
// 
// bool cSceneManager::PickMapportal( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
// {
// 	return mNodeTree.CollideRay_Kind( pickedArray, ray, false, maxDistance, sortByDistance, cSceneNode::eMAPPORTAL );
// }
// 
// 
// cBaseObject* cSceneManager::PickPlane( int mouseX, int mouseY )
// {
// 	if( mVisibleDynamicNodeArray.IsEmpty() == true )
// 		return 0;
// 
// 	NiPoint3 origin, dir;
// 	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) == false )
// 		return 0;
// 
// 	unsigned int count = mVisibleDynamicNodeArray.GetSize();
// 	for( unsigned int i=0; i<count; ++i )
// 	{
// 		cDynamicSceneNode* n = GetDynamicSceneNode( mVisibleDynamicNodeArray[i] );
// 		if( n == 0 )
// 			continue;
// 		if( n->GetType() != cSceneNode::ePLAYER	&& 	n->GetType() != cSceneNode::eNPC )
// 			continue;
//  		if( n->PickManagedPlane( (float)mouseX, (float)mouseY ) == false )
// 			continue;
// 
// 		return n->GetGameObject();
// 	}
// 	return 0;
// }
 
bool cSceneManager::AddAreaNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eAREA );

	unsigned long idx = mAreaNodeIndexGen.GenerateIndex();
	if( mAreaArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddAreaNode(GenerateIndex)[%d]", idx );

		idx = mAreaNodeIndexGen.GenerateIndex();
	}
	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();

	if( mAreaArray.Insert( node->mIndexByManager, node ) == false )
	{
		assert(0);
		return false;
	}

	mNodeTree.Push( node );
	return true;
}

// bool cSceneManager::AddLightNode( cSceneNode* node )
// {
// 	assert( node );
// 	assert( node->GetType()==cSceneNode::eLIGHT );
// 
// 	unsigned long idx = mLightNodeIndexGen.GenerateIndex();
// 	if( mLightArray.GetAt(idx) != 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "cSceneManager::AddLightNode(GenerateIndex)[%d]", idx );
// 
// 		idx = mLightNodeIndexGen.GenerateIndex();
// 	}
// 	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
// 	if( mLightArray.Insert( node->mIndexByManager, node ) == false )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 
// 	mNodeTree.Push( node );
// 	return true;
// }
// 
bool cSceneManager::AddSoundNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eSOUND );

	unsigned long idx = mSoundIndexGen.GenerateIndex();
	if( mSoundArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddSoundNode(GenerateIndex)[%d]", idx );

		idx = mSoundIndexGen.GenerateIndex();
	}
	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
	if( mSoundArray.Insert( node->mIndexByManager, node ) == false )
	{
		assert(0);
		return false;
	}
	mNodeTree.Push( node );
	return true;
}

bool cSceneManager::AddStaticNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eSTATIC );

	unsigned long idx = mStaticNodeIndexGen.GenerateIndex();
	if( mStaticObjectArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddStaticNode(GenerateIndex)[%d]", idx );

		idx = mStaticNodeIndexGen.GenerateIndex();
	}
	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
	if( mStaticObjectArray.Insert( node->mIndexByManager, node ) == false )
	{
		assert(0);
		return false;
	}
	mNodeTree.Push( node );
	return true;
}

// bool cSceneManager::AddDropItemNode( cSceneNode* node )
// {
// 	assert( node );
// 	assert( node->GetType()==cSceneNode::eDROPITEM );
// 
// 	unsigned long idx = mDropItemNodeIndexGen.GenerateIndex();
// 	if( mDropItemArray.GetAt(idx) != 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "cSceneManager::AddDropItemNode(GenerateIndex)[%d]", idx );
// 
// 		idx = mDropItemNodeIndexGen.GenerateIndex();
// 	}
// 	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
// 	if( mDropItemArray.Insert( node->mIndexByManager, node ) == false )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 	mNodeTree.Push( node );
// 	return true;
// }
// 
// bool cSceneManager::AddGatheringNode( cSceneNode* node )
// {
// 	assert( node );
// 	assert( node->GetType()==cSceneNode::eGATHERING );
// 
// 	unsigned long idx = mGatheringNodeIndexGen.GenerateIndex();
// 	if( mGatheringArray.GetAt(idx) != 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "cSceneManager::AddGatheringNode(GenerateIndex)[%d]", idx );
// 
// 		idx = mGatheringNodeIndexGen.GenerateIndex();
// 	}
// 	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
// 	if( mGatheringArray.Insert( node->mIndexByManager, node ) == false )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 	mNodeTree.Push( node );
// 	return true;
// }
 
bool cSceneManager::AddMapPotalNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eMAPPORTAL );

	unsigned long idx = mMapPortalNodeIndexGen.GenerateIndex();
	if( mMapPortalArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddMapPotalNode(GenerateIndex)[%d]", idx );
		idx = mMapPortalNodeIndexGen.GenerateIndex();
	}

	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
	if( mMapPortalArray.Insert( node->mIndexByManager, node ) == false )
	{
		assert(0);
		return false;
	}
	mNodeTree.Push( node );
	return true;
}
 
bool cSceneManager::AddDynamicNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()&cSceneNode::eDYNAMIC );

	unsigned long idx = mDynamicNodeIndexGen.GenerateIndex();
	if( mDynamicObjectArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddDynamicNode(GenerateIndex)[%d]", idx );
		idx = mDynamicNodeIndexGen.GenerateIndex();
	}

	node->mIndexByManager = idx;//mDynamicNodeIndexGen.GenerateIndex();
	if( mDynamicObjectArray.Insert( node->mIndexByManager, (cDynamicSceneNode*)node ) == false )
	{
		assert(0);
		return false;
	}

	mNodeTree.Push( node );
	return true;
}
 
bool cSceneManager::AddEffectNode( cEffectSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eEFFECT );

	unsigned long idx = mEffectIndexGen.GenerateIndex();
	if( mEffectNodeArray.GetAt(idx) != 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddEffectNode(GenerateIndex)[%d]", idx );
		idx = mEffectIndexGen.GenerateIndex();
	}

	node->mIndexByManager = idx;//mNodeIndexGen.GenerateIndex();
	if( idx == (unsigned long)-1 )
	{
		assert(0);
	}

	if( mEffectNodeArray.Insert( node->mIndexByManager, node ) == false )
	{
		assert(0);
		return false;
	}

	return true;
}

// bool cSceneManager::AddDoorNode( unsigned long index, cSceneNode* node )
// {
// 	assert( node );
// 	assert( node->GetType()==cSceneNode::eDOOR );
// 
// 	if( mDoorArray.GetAt(index) != 0 )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 
// 	node->mIndexByManager = index;
// 	if( mDoorArray.Insert( node->mIndexByManager, node ) == false )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 
// 	mNodeTree.Push( node );
// 	return true;
// }
// 
void cSceneManager::AddDeleteEffectNode( cEffectSceneNode* node )
{
	if( node == 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddDeleteEffectNode : node == 0" );
		return;
	}

	if( IsValidEffectNode( node->GetIndexByManger(), node ) == false )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddDeleteEffectNode : IsValidEffectNode Faile(%d)", node->GetIndexByManger() );
		return;
	}

	mDelEffectList.PushBack( node );
}

void cSceneManager::AddDeleteGatheringNode( cGatheringSceneNode* node )
{
	if( node == 0 )
	{
		assert(0);
		GameErrorLog( "cSceneManager::AddDeleteGatheringNode : node == 0" );
		return;
	}

	mDelGatheringList.PushBack( node );
}

void cSceneManager::AddDeleteSoundNode( cSoundSceneNode* node )
{
	if( node == 0 )
		return;
	if( node->IsRemoved() )
		return;

	node->SetRemoveFlag( true );

	mDelSoundList.PushBack( node );
}

void cSceneManager::AddDeleteDoorNode( cDoorSceneNode* node )
{
	if( node == 0 )
		return;

	mDelDoorList.PushBack( node );
}
// 
// void cSceneManager::AddPlane( cPlane* pPlane )
// {
// 	mPlaneArray.PushBack( pPlane );
// }
// 
// void cSceneManager::AddVisibleDynamicNode( cDynamicSceneNode* node )
// {
// 	if( node == 0 )
// 	{
// 		assert(0);
// 		return;
// 	}
// 
// 	if( ((cDynamicSceneNode*)node)->mSetVisibleArray == true )
// 	{
// //		assert(0);
// 		return;
// 	}
// 
// 	((cDynamicSceneNode*)node)->mSetVisibleArray = true;
// 	mVisibleDynamicNodeArray.PushBack( node->GetIndexByManger() );
// }
 
bool cSceneManager::DestroyDynamic( cSceneNode* node )
{
	assert( node );

	if( (node->GetType() & cSceneNode::eDYNAMIC) == 0 )
	{
		assert(0);
		GameErrorLog( "DestroyDynamic exeption wrong type" );
		return false;
	}

	unsigned long idx = node->mIndexByManager;
	cDynamicSceneNode* n = (cDynamicSceneNode*)mDynamicObjectArray.GetAt( idx );
	if( n == 0 )
	{
		assert(0);
		GameErrorLog( "DestroyDynamic exeption n == 0" );
		return false;
	}
	if( n != node )
	{
		assert(0);
		GameErrorLog( "DestroyDynamic exeption n != node" );
		return false;
	}

	if( n->mContainer )
	{
		n->mContainer->Remove(n);
		n->mContainer = 0;
	}
	else
	{
		assert(0);
		GameErrorLog( "DestroyDynamic exeption (n->mContainer == 0)" );
	}

//	if( n->mSetVisibleArray == true )
//	{
//		assert(0);
//	}

	delete n;

	mDynamicObjectArray.Erase( idx );
	n = (cDynamicSceneNode*)mDynamicObjectArray.GetAt( idx );
	if( n )
	{
		GameErrorLog( "DestroyDynamic exeption Erase Faile" );
		assert(0);
	}

	mDynamicNodeIndexGen.RestoreIndex( idx );

	return true;
}

// bool cSceneManager::DestroyDropItemNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eDROPITEM )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDropItemNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cDropItemSceneNode* n = (cDropItemSceneNode*)mDropItemArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDropItemNode exeption n == 0" );
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDropItemNode exeption n != node" );
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mDropItemArray.Erase( idx );
// 	n = (cDropItemSceneNode*)mDropItemArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestroyDropItemNode exeption Erase Faile" );
// 		assert(0);
// 	}
// 
// 	mDropItemNodeIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
// bool cSceneManager::DestroyGathringNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eGATHERING )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyGathringNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cGatheringSceneNode* n = (cGatheringSceneNode*)mGatheringArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyGathringNode exeption n == 0" );
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyGathringNode exeption n != node" );
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mGatheringArray.Erase( idx );
// 	n = (cGatheringSceneNode*)mGatheringArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestroyGathringNode exeption Erase Faile" );
// 		assert(0);
// 	}
// 
// 	mGatheringNodeIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
// 
// bool cSceneManager::DestroyMapPortalNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eMAPPORTAL )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyMapPortalNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cMapPortalSceneNode* n = (cMapPortalSceneNode*)mMapPortalArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyMapPortalNode exeption n == 0" );
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyMapPortalNode exeption n != node" );
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mMapPortalArray.Erase( idx );
// 	n = (cMapPortalSceneNode*)mMapPortalArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestroyMapPortalNode exeption Erase Faile" );
// 		assert(0);
// 	}
// 
// 	mMapPortalNodeIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
// bool cSceneManager::DestroyDoorNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eDOOR )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDoorNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cDoorSceneNode* n = (cDoorSceneNode*)mDoorArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDoorNode exeption n == 0" );
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyDoorNode exeption n != node" );
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mDoorArray.Erase( idx );
// 	n = (cDoorSceneNode*)mDoorArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestroyDoorNode exeption Erase Fail" );
// 		assert(0);
// 	}
// 	return true;
// }
// 
// bool cSceneManager::DestroyEffectNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eEFFECT )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestoryEffectNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cEffectSceneNode* n = (cEffectSceneNode*)mEffectNodeArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		GameErrorLog( "DestoryEffectNode exeption n == 0" );
// 		assert(0);
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		GameErrorLog( "DestoryEffectNode exeption n != node" );
// 		assert(0);
// 		return false;
// 	}
// 
// 	if( n->IsRemoved() == false )
// 	{
// 		GameErrorLog( "DestoryEffectNode exeption IsRemoved == false" );
// 		assert(0);
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		assert(0);
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mEffectNodeArray.Erase( idx );
// 	n = (cEffectSceneNode*)mEffectNodeArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestoryEffectNode exeption Erase Faile" );
// 		assert(0);
// 	}
// 
// 	mEffectIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
// bool cSceneManager::DestroySoundNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eSOUND )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroySoundNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cSoundSceneNode* n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroySoundNode exeption n == 0" );
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroySoundNode exeption n != node" );
// 		return false;
// 	}
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mSoundArray.Erase( idx );
// 	n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
// 	if( n )
// 	{
// 		GameErrorLog( "DestroySoundNode exeption Erase Faile" );
// 		assert(0);
// 	}
// 
// 	mSoundIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
// bool cSceneManager::DestroyStaticNode( cSceneNode* node )
// {
// 	assert( node );
// 
// 	if( node->GetType() != cSceneNode::eSTATIC )
// 	{
// 		assert(0);
// 		GameErrorLog( "DestroyStaticNode exeption wrong type" );
// 		return false;
// 	}
// 
// 	unsigned long idx = node->mIndexByManager;
// 	cStaticSceneNode* n = (cStaticSceneNode*)mStaticObjectArray.GetAt( idx );
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 	if( n != node )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 
// 
// 	if( n->mContainer )
// 	{
// 		n->mContainer->Remove(n);
// 		n->mContainer = 0;
// 	}
// 
// 	delete n;
// 
// 	mStaticObjectArray.Erase( idx );
// 	mStaticNodeIndexGen.RestoreIndex( idx );
// 
// 	return true;
// }
// 
void cSceneManager::DestroyAll()
{
	mPlaneArray.Clear();

	{
		cAreaMap::cIterator i, iend;
		i = mAreaArray.Begin();
		iend = mAreaArray.End();
		for( ; i != iend; ++i )
		{
			cAreaSceneNode* p = (cAreaSceneNode*)(*i).mSecond;
			delete p;
		}
		mAreaArray.Clear();
	}
	{
		cLightMap::cIterator i, iend;
		i = mLightArray.Begin();
		iend = mLightArray.End();
		for( ; i != iend; ++i )
		{
			cLightSceneNode* p = (cLightSceneNode*)(*i).mSecond;
			delete p;
		}
		mLightArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mDropItemArray.Begin();
		iend = mDropItemArray.End();
		for( ; i != iend; ++i )
		{
			cDropItemSceneNode* p = (cDropItemSceneNode*)(*i).mSecond;
			delete p;
		}
		mDropItemArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mGatheringArray.Begin();
		iend = mGatheringArray.End();
		for( ; i != iend; ++i )
		{
			cGatheringSceneNode* p = (cGatheringSceneNode*)(*i).mSecond;
			delete p;
		}
		mGatheringArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mDynamicObjectArray.Begin();
		iend = mDynamicObjectArray.End();
		for( ; i != iend; ++i )
		{
			cDynamicSceneNode* p = (cDynamicSceneNode*)(*i).mSecond;
			delete p;
		}
		mDynamicObjectArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mMapPortalArray.Begin();
		iend = mMapPortalArray.End();
		for( ; i != iend; ++i )
		{
			cMapPortalSceneNode* p = (cMapPortalSceneNode*)(*i).mSecond;
			delete p;
		}
		mMapPortalArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mDoorArray.Begin();
		iend = mDoorArray.End();
		for( ; i != iend; ++i )
		{
			cDoorSceneNode* p = (cDoorSceneNode*)(*i).mSecond;
			delete p;
		}
		mDoorArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mStaticObjectArray.Begin();
		iend = mStaticObjectArray.End();
		for( ; i != iend; ++i )
		{
			cStaticSceneNode* p = (cStaticSceneNode*)(*i).mSecond;
			delete p;
		}
		mStaticObjectArray.Clear();
	}
	{
		cNodeMap::cIterator i, iend;
		i = mEffectNodeArray.Begin();
		iend = mEffectNodeArray.End();
		for( ; i != iend; ++i )
		{
			cEffectSceneNode* p = (cEffectSceneNode*)(*i).mSecond;
			delete p;
		}
		mEffectNodeArray.Clear();
	}
	{
		cSoundMap::cIterator i, iend;
		i = mSoundArray.Begin();
		iend = mSoundArray.End();
		for( ; i != iend; ++i )
		{
			cSoundSceneNode* p = (cSoundSceneNode*)(*i).mSecond;
			delete p;
		}
		mSoundArray.Clear();
	}

	mDelEffectList.Clear();
	mDelGatheringList.Clear();
	mDelSoundList.Clear();
	mDelDoorList.Clear();

	///
	mNodeTree.Clear();
	mAreaNodeIndexGen.RestoreAll();
	mLightNodeIndexGen.RestoreAll();
	mSoundIndexGen.RestoreAll();
	mStaticNodeIndexGen.RestoreAll();
	mDropItemNodeIndexGen.RestoreAll();
	mGatheringNodeIndexGen.RestoreAll();
	mMapPortalNodeIndexGen.RestoreAll();
	mDynamicNodeIndexGen.RestoreAll();
	mEffectIndexGen.RestoreAll();
}

cAreaSceneNode* cSceneManager::CreateArea( cAreaSceneNodeParam& param )
{
	static int i = 0;

	/// 노드를 생성
	cAreaSceneNode* node = new cAreaSceneNode;

	/// 노드를 초기화
	if( param.mPathName.IsEmpty() )
		param.mPathName.Format( "area%d", i++ );

	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init area scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddAreaNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

// cLightSceneNode* cSceneManager::CreateLight( cLightSceneNodeParam& param )
// {
// 	static int i = 0;
// 
// 	/// 노드를 생성
// 	cLightSceneNode* node = new cLightSceneNode;
// 
// 	/// 노드를 초기화
// 	if( param.mPathName.IsEmpty() )
// 		param.mPathName.Format( "light%d", i++ );
// 
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init light scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddLightNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
cSoundSceneNode* cSceneManager::CreateMapSound( cSoundSceneNodeParam& param )
{
	if( SOUNDSYS->IsUsableSoundSystem() == false )
		return false;

	static int i = 0;

	/// 노드를 생성
	cSoundSceneNode* node = new cSoundSceneNode;

	/// 노드를 초기화
	if( param.mPathName.IsEmpty() )
		param.mPathName.Format( "sound%d", i++ );

	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init sound scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddSoundNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

cSoundSceneNode* cSceneManager::CreateEffectSound( cSoundSceneNodeParam& param )
{
	if( SOUNDSYS->IsUsableSoundSystem() == false )
		return false;

	/// 노드를 생성
	cSoundSceneNode* node = new cSoundSceneNode;

	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init sound scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddSoundNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}
 
cStaticSceneNode* cSceneManager::CreateStatic( cStaticSceneNodeParam& param )
{
	/// 노드를 생성
	cStaticSceneNode* node = new cStaticSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init static scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddStaticNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

// cCardSceneNode* cSceneManager::CreateCard( cCardSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cCardSceneNode* node = new cCardSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init card scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddStaticNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// cDropItemSceneNode* cSceneManager::CreateDropItem( cDropItemSceneNodeParam& param )
// {
// 	cDropItemSceneNode* node = new cDropItemSceneNode;
// 	param.mPathName = "./Data/Effect/DropBox.nif";
// 
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init DropItem scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDropItemNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// cGatheringSceneNode* cSceneManager::CreateGathering( cGatheringSceneNodeParam& param )
// {
// 	cGatheringSceneNode* node = new cGatheringSceneNode;
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init Gathering scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddGatheringNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
 
cMapPortalSceneNode* cSceneManager::CreateMapPortal( unsigned long scriptIdx, NiPoint3 pos, NiMatrix3 rot )
{
	cMapPortalSceneNodeParam param;
	param.mTranslate = pos;
	param.mRotate = rot;
	param.mPosScriptIdx = scriptIdx;
	param.mPathName = "./Data/Effect/";

	cMapPortalSceneNode* node = new cMapPortalSceneNode;
	if( node->Init( param) == false )
	{
		delete node;
		assert( 0 && "failed to init MapPortal scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddMapPotalNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

// cUserPortalSceneNode* cSceneManager::CreateUserPortal( unsigned long portalIdx, unsigned long ownerIdx, unsigned long scriptIdx, NiPoint3 pos )
// {
// 	cUserPortalSceneNodeParam param;
// 	param.mTranslate = pos;
// 	param.mPortalIdx = portalIdx;
// 	param.mOwnerIdx = ownerIdx;
// 	param.mPosScriptIdx = scriptIdx;
// 	param.mPathName = "./Data/Effect/map_portal_c.nif";
// 
// 	cUserPortalSceneNode* node = new cUserPortalSceneNode;
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init UserPortal scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddMapPotalNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 	return node;
// }
// 
// cPlayerSceneNode* cSceneManager::CreatePlayer( cPlayerSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cPlayerSceneNode* node = new cPlayerSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init player scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// cMonsterSceneNode* cSceneManager::CreateMonster( cMonsterSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cMonsterSceneNode* node = new cMonsterSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init Monster scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// cTransMonSceneNode* cSceneManager::CreateTransMonster( cMonsterSceneNodeParam& param, bool IsHero )
// {
// 	/// 노드를 생성
// 	cTransMonSceneNode* node = new cTransMonSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param, IsHero ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init Transform Monster scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// 
// cNpcSceneNode* cSceneManager::CreateNPC( cNpcSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cNpcSceneNode* node = new cNpcSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init Npc scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
 
cEffectSceneNode* cSceneManager::CreateEffect( cEffectSceneNodeParam& param )
{
	cEffectSceneNode* node = new cEffectSceneNode;
	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init Effect scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddEffectNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

cEffectSceneNode* cSceneManager::CreateDamageEffect( cEffectSceneNodeParam& param, bool IsMiss, bool zFalse )
{
	cEffectSceneNode* node = new cEffectSceneNode;
	if( node->InitDamageEffect( param, IsMiss, zFalse ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	/// 노드를 추가
	if( AddEffectNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}
 
cEffectSceneNode* cSceneManager::CreateSelfEffect( cString pathName, bool distCheck, NiTransform& trans, bool bLoop )
{
	cEffectSceneNode* node = new cEffectSceneNode;
	if( node->InitSelfNode( pathName, distCheck, trans, bLoop ) == false )
	{
		assert( 0 && "failed to init Effect scene node" );
		delete node;
		return 0;
	}

	/// 노드를 추가
	if( AddEffectNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

// cHeroSceneNode* cSceneManager::CreateHero( cPlayerSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cHeroSceneNode* node = new cHeroSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init player scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// cVehicleSceneNode* cSceneManager::CreateVehicle( cVehicleSceneNodeParam& param )
// {
// 	/// 노드를 생성
// 	cVehicleSceneNode* node = new cVehicleSceneNode;
// 
// 	/// 노드를 초기화
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init vehicle scene node" );
// 		return 0;
// 	}
// 
// 	/// 노드를 추가
// 	if( AddDynamicNode( node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
cRefereeSceneNode* cSceneManager::CreateReferee( cSceneNodeParam& param )
{
	/// 노드를 생성
	cRefereeSceneNode* node = new cRefereeSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init referee scene node" );
		return 0;
	}

	/// 노드를 추가
	if( AddDynamicNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	return node;
}

// cDoorSceneNode* cSceneManager::CreateDoor( unsigned long index ) //cDoorSceneNodeParam& param )
// {
// 	sDoorInfo* pInfo = STAGESCRIPT->GetManagedDoorInfo( index );
// 	if( pInfo == 0 )
// 		return 0;
// 
// 
// 	cDoorSceneNodeParam param;
// 	param.mPathName.Format( "./data/effect/%s", pInfo->mFileName.Cstr() );
// 	param.mOccludeLevel = 2;
// 	param.mVisibleLevel = 3;
// 
// 	if( pInfo->mMapNumber >= 200 )
// 		param.mDisableAnimation = true;
// 	else
// 		param.mDisableAnimation = false;
// 
// 	param.mDoorIdx = index;
// 
// 	NiPoint3 pos = NiPoint3(pInfo->mPosX, pInfo->mPosY, 0.0f );
// 
// 	if( WORLDMAN->CalcHeight( &(pos.z), pos.x, pos.y ) == false )
// 		assert(0);
// 
// 	param.mTranslate = pos;
// 	param.mRotate.FromEulerAnglesXYZ( 0, 0, pInfo->mRotAngle);
// 	param.mScale = 1.0f;
// 
// 	cDoorSceneNode* node = new cDoorSceneNode;
// 
// 	if( node->Init( param ) == false )
// 	{
// 		delete node;
// 		assert( 0 && "failed to init post scene node" );
// 		return 0;
// 	}
// 
// 	/// 
// 	if( AddDoorNode( param.mDoorIdx, node ) == false )
// 	{
// 		assert(0);
// 		delete node;
// 		return 0;
// 	}
// 
// 	return node;
// }
// 
// void cSceneManager::ActiveSoundNodeAll( bool active )
// {
// 	{
// 		cSoundMap::cIterator i, iend;
// 		i = mSoundArray.Begin();
// 		iend = mSoundArray.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cSoundSceneNode* n = (cSoundSceneNode*)(*i).mSecond;
// 			n->ActiveSound( active );
// 		}
// 	}
// 
// 	{
// 		cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
// 		cAreaGroupMap::cIterator iend = mAreaGroupMap.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cAreaGroup* group = (cAreaGroup*)(i->mSecond);
// 			group->SetActive( active );
// 		}
// 	}
// }
// 
// unsigned int cSceneManager::CalcVisibleLevel( cStaticSceneNode* n )
// {
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		return 0;
// 	}
// 
// #ifdef _GMTOOL
// 	if( CAMERAMAN->GetCurrentIdx() == 2 )
// 		return 0;
// #endif
// 
// 	float sqrDist = (mHeroPos - n->GetCenter()).SqrLength();
// 
// 	if( sqrDist > mSqaredLevelDistance2 )
// 		return 3;
// 	if( sqrDist > mSqaredLevelDistance1 )
// 		return 2;
// 	if( sqrDist > mSqaredLevelDistance0 )
// 		return 1;
// 	return 0;
// }
// 
// bool cSceneManager::CalcDynamicVisibleLevel( cSceneNode* n )
// {
// 	if( n == 0 )
// 	{
// 		assert(0);
// 		return false;
// 	}
// 
// 	float sqrDist = (mHeroPos - n->GetWorldTranslate()).SqrLength();
// 	if( sqrDist > 16000000.0f ) // 4000 * 4000
// 		return false;
// 	else
// 		return true;
// }
// 
// void cSceneManager::UpdateSoundGain()
// {
// 	cSoundMap::cIterator i, iend;
// 	i = mSoundArray.Begin();
// 	iend = mSoundArray.End();
// 	for( ; i != iend; ++i )
// 	{
// 		cSoundSceneNode* p = (cSoundSceneNode*)(*i).mSecond;
// 		p->UpdatVolumeRatio();
// 	}
// }
 
bool cSceneManager::IsValidEffectNode( cEffectSceneNode* node )
{
	assert(node->GetType() == cSceneNode::eEFFECT);

	cNodeMap::cIterator i, iend;
	i = mEffectNodeArray.Begin();
	iend = mEffectNodeArray.End();

	for( ; i != iend; ++i )
	{
		cEffectSceneNode* p = (cEffectSceneNode*)(*i).mSecond;
		if( p == node )
			return true;
	}

	return false;
}

bool cSceneManager::IsValidEffectNode( unsigned long idx, cEffectSceneNode* node )
{
	assert(node->GetType() == cSceneNode::eEFFECT);

	cEffectSceneNode* n = (cEffectSceneNode*)mEffectNodeArray.GetAt( idx );
	if( n == 0 )
	{
//		OutputDebugString(_T("\nn==0"));
		return false;
	}
	if( n != node )
	{
//		OutputDebugString(_T("\nn!=node"));
		return false;
	}

	return true;
}

// bool cSceneManager::IsValidSoundNode( cSoundSceneNode* node )
// {
// 	cNodeMap::cIterator i, iend;
// 	i = mSoundArray.Begin();
// 	iend = mSoundArray.End();
// 
// 	for( ; i != iend; ++i )
// 	{
// 		cSoundSceneNode* p = (cSoundSceneNode*)(*i).mSecond;
// 		if( p == node )
// 			return true;
// 	}
// 
// 	return false;
// }
// 
// 
// bool cSceneManager::IsValidSoundNode( unsigned long idx, cSoundSceneNode* node )
// {
// 	cSoundSceneNode* n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
// 	if( n == 0 )
// 		return false;
// 	if( n != node )
// 		return false;
// 
// 	return true;
// }
// 
// 
// bool cSceneManager::IsValidDynamicNode( cDynamicSceneNode* node )
// {
// 	cNodeMap::cIterator i, iend;
// 	i = mDynamicObjectArray.Begin();
// 	iend = mDynamicObjectArray.End();
// 	for( ; i != iend; ++i )
// 	{
// 		cDynamicSceneNode* p = (cDynamicSceneNode*)(*i).mSecond;
// 		if( p == node )
// 			return true;
// 	}
// 
// 	return false;
// }
// 
// bool cSceneManager::IsValidDynamicNode( unsigned long idx, cDynamicSceneNode* node )
// {
// 	cDynamicSceneNode* n = (cDynamicSceneNode*)mDynamicObjectArray.GetAt( idx );
// 	if( n == 0 )
// 		return false;
// 	if( n != node )
// 		return false;
// 
// 	return true;
// }
// 
// cManagedStaticSceneNode* cSceneManager::GetManagedStaticNode( unsigned int type, unsigned long nodeIdx )
// {
// 	switch( type )
// 	{
// 	case cSceneNode::eDROPITEM:
// 		{
// 			return (cManagedStaticSceneNode*)mDropItemArray.GetAt( nodeIdx );
// 		}
// 		break;
// 	case cSceneNode::eGATHERING: 
// 		{
// 			return (cManagedStaticSceneNode*)mGatheringArray.GetAt( nodeIdx );
// 		}
// 		break;
// 	case cSceneNode::eMAPPORTAL: 
// 		{
// 			return (cManagedStaticSceneNode*)mMapPortalArray.GetAt( nodeIdx );
// 		}
// 		break;
// 	case cSceneNode::eDOOR:
// 		{
// 			return (cManagedStaticSceneNode*)mDoorArray.GetAt( nodeIdx );
// 		}
// 		break;
// 	default:
// 		break;
// 	}
// 	return 0;
// }
// 
// void cSceneManager::OnVisibleModifierPocess( cSceneNode* node )
// {
// 	mModifierProcess.Cull( CAMERAMAN->GetCurrentNi(), node );
// }
// 
// void cSceneManager::DelNodeProcess()
// {
// 	cDelNodeList::cIterator i, iend;
// 	if( mDelEffectList.IsEmpty() == false )
// 	{
// 		i = mDelEffectList.Begin();
// 		iend = mDelEffectList.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cEffectSceneNode* n = (cEffectSceneNode*)*i;
// 			DestroyEffectNode(n);
// 		}
// 		mDelEffectList.Clear();
// 	}
// 	if( mDelGatheringList.IsEmpty() == false )
// 	{
// 		i = mDelGatheringList.Begin();
// 		iend = mDelGatheringList.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cGatheringSceneNode* n = (cGatheringSceneNode*)*i;
// 			DestroyGathringNode(n);
// 		}
// 		mDelGatheringList.Clear();
// 	}
// 	if( mDelSoundList.IsEmpty() == false )
// 	{
// 		i = mDelSoundList.Begin();
// 		iend = mDelSoundList.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cSoundSceneNode* n = (cSoundSceneNode*)*i;
// 			DestroySoundNode(n);
// 		}
// 		mDelSoundList.Clear();
// 	}
// 
// 	if( mDelDoorList.IsEmpty() == false )
// 	{
// 		i = mDelDoorList.Begin();
// 		iend = mDelDoorList.End();
// 		for( ; i != iend; ++i )
// 		{
// 			cDoorSceneNode* n = (cDoorSceneNode*)*i;
// 			DestroyDoorNode(n);
// 		}
// 		mDelDoorList.Clear();
// 	}
// }
// 
// sDoorInfo* cSceneManager::DisableDoorMove( NiPoint2 startPos, NiPoint2 goalPos )
// {
// 	cNodeMap::cIterator i = mDoorArray.Begin();
// 	cNodeMap::cIterator iend = mDoorArray.End();
// 	for( ; i != iend; ++i )
// 	{
// 		cDoorSceneNode* n = (cDoorSceneNode*)(*i).mSecond;
// 		if( n == 0 )
// 			continue;
// 
// 		sDoorInfo* pInfo = STAGESCRIPT->CrashDoor( n->GetDoorIdx(), startPos, goalPos );
// 		if( pInfo )
// 			return pInfo;
// 	}
// 	return 0;
// }
// 
// void cSceneManager::RemoveAllDoorNode()
// {
// 	cNodeMap::cIterator i = mDoorArray.Begin();
// 	cNodeMap::cIterator iend = mDoorArray.End();
// 	for( ; i != iend; ++i )
// 	{
// 		cDoorSceneNode* n = (cDoorSceneNode*)(*i).mSecond;
// 		if( n == 0 )
// 			continue;
// 
// 		n->Remove( THEAPP->GetWorldAccumTime() );
// 	}
// }
