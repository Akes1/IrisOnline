#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "CameraManager.h"

#include "Ray.h"
#include "SoundSceneNode.h"
#include "StaticSceneNode.h"
#include "DynamicSceneNode.h"
#include "PlayerSceneNode.h"
#include "MonsterSceneNode.h"
#include "NpcSceneNode.h"
#include "EffectSceneNode.h"

#include "ObjectManager.h"

#include "GameResourceManager.h"

#include "characterForm.h"
#include "modelApp.h"

/// 오브젝트 상태 값
enum eMONSTER_ANIMATIONTYPE
{ 
	M_ANITYPE_IDLE1 = 1,	/// 대기
	M_ANITYPE_IDLE2,		/// 대기보조
	M_ANITYPE_WORK,			/// 걷기
	M_ANITYPE_RUN,			/// 달리기
	M_ANITYPE_ATTACK1,		/// 공격1
	M_ANITYPE_ATTACK2,		/// 공격2
	M_ANITYPE_ATTACK3,		/// 공격3
	M_ANITYPE_CASTING,		/// 캐스팅
	M_ANITYPE_SKILL1,		/// 스킬1
	M_ANITYPE_SKILL2,		/// 스킬2
	M_ANITYPE_DAMAGE1,		/// 데미지1
	M_ANITYPE_DAMAGE2,		/// 데미지2
	M_ANITYPE_DIE1,			/// 죽음1
	M_ANITYPE_DIE2,			/// 죽음2

	M_ANITYPE_MODEANI = 21,
};

/// 장면 노드 비교 함수자
/// 픽킹 거리로 비교한다.
bool cDynamicSceneNodeCompareByCamDistance( const unsigned long left, const unsigned long right )
{
	/// 
	cDynamicSceneNode* node1 = SCENEMAN->GetDynamicSceneNode( left );
	cDynamicSceneNode* node2 = SCENEMAN->GetDynamicSceneNode( right );

	if( node1 == 0 )
	{
		assert(0);
	}
	if( node2 == 0 )
	{
		assert(0);
	}

	return ( node1->mVisibleCamLength < node2->mVisibleCamLength );
};

cSceneCuller::cSceneCuller( const cCamera* cam, const NiPoint3& heroPos )
: NiMeshCullingProcess(0, NULL, false)
{
	assert( cam && "null camera" );

	m_pkCamera = (NiCamera*)cam;
	mCamera = cam;

	mHeroPos = heroPos;
}



cSceneManager* cSceneManager::mSingleton = 0;

cSceneManager::cSceneManager()
: mInited( false )
, mVersion( 0 )
, mAlphaArray( 1024, 256 )
, mAlphaTestArray( 1024, 256 )
, mSolidArray( 1024, 256 )
, mDamageArray( 256, 64 )
, mCuller( &mAlphaArray, &mAlphaTestArray, &mSolidArray )
, mDamageCuller( &mDamageArray, &mAlphaTestArray, &mSolidArray )
, mAlphaArraySorter( &mAlphaArray, &mAlphaTestArray )
, mDamageSorter( &mDamageArray, 0 )
, mSolidArraySorter( &mSolidArray )
, mTextureMap( 22 )
, mScreenShotMode( false )
, mEnableLoading( false )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	///
	mPickedArray.Reserve( 1024 );

	/// 가시 레벨 거리
	mSqaredLevelDistance0 = 5000.0f * 5000.0f;
	mSqaredLevelDistance1 = 10000.0f * 10000.0f;
	mSqaredLevelDistance2 = 20000.0f * 20000.0f;

	mAmbientLightArray.SetSize( 20 );
	mAmbientLightArray.SetGrowBy( 10 );
	mDirLightArray.SetSize( 20 );
	mDirLightArray.SetGrowBy( 10 );

	mCurrentLightIndex = 0;

	mpCurrentSceneNode = 0;
	mpDummyNode = 0;

	mDramaProcess = false;

	mHeroPos = NiPoint3::ZERO;
}

cSceneManager::~cSceneManager()
{
	Clear();

	mSingleton = 0;

}

bool cSceneManager::Init()
{
	mTrailGeomArray.Reserve( 100 );

	return true;
}

void cSceneManager::Exit()
{
	Clear();
}

void cSceneManager::Clear()
{
	mInited = false;

	DestroyAll();

	mDirLightArray.RemoveAll();
	mAmbientLightArray.RemoveAll();

	///
	mAlphaArray.RemoveAll();
	mAlphaTestArray.RemoveAll();
	mSolidArray.RemoveAll();
	mDamageArray.RemoveAll();

	mTextureMap.Clear();

	mTrailGeomArray.Clear();
}

bool cSceneManager::Init( const NiPoint3& center, float minRadius, float maxRadius )
{
	/// 지움
	Clear();

	/// 트리를 재생성
	mNodeTree.Init( center, minRadius, maxRadius );

	mInited = true;

	/// scene info load (character light)
	TCHAR dir[MAX_PATH] = {0,};
	cString str = theApp.GetBaseDir();
	::sprintf_s( dir, _T("%sdata\\map\\*.scene"), str.Cstr() );
	ScanDir(dir);

	AddAmbientLight( NiColor::WHITE, NiColor::WHITE, 1.0f );

	cStaticSceneNode* node = new cStaticSceneNode;
	cStaticSceneNodeParam param;
	param.mPathName = "./Data/Effect/plane.nif";
	if( node->Init( param ) == true )
		AddStaticNode( node );
	else
		NiMessageBox( "Check plane object", "Error" );

	return true;
}

void cSceneManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mInited == false )
		return;

	cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 );
		return;
	}

	mTrailGeomArray.Clear();

	///
	mSolidArray.RemoveAll();
	mAlphaArray.RemoveAll();
	mAlphaTestArray.RemoveAll();
	mDamageArray.RemoveAll();

	/// 제거될 이펙트들을 처리
	if( mDelEffectList.IsEmpty() == false )
	{
		cDelEffectList::cIterator i = mDelEffectList.Begin();
		cDelEffectList::cIterator iend = mDelEffectList.End();
		for( ; i != iend; ++i )
		{
			cEffectSceneNode* n = (cEffectSceneNode*)*i;
			DestroyEffectNode(n);
		}
		mDelEffectList.Clear();
	}

	/// 제거될 사운드들을 처리
	if( mDelSoundList.IsEmpty() == false )
	{
		cDelSoundList::cIterator i = mDelSoundList.Begin();
		cDelSoundList::cIterator iend = mDelSoundList.End();
		for( ; i != iend; ++i )
		{
			cSoundSceneNode* n = (cSoundSceneNode*)*i;
			DestroySoundNode(n);
		}
		mDelSoundList.Clear();
	}

	if( mNodeTree.IsEmpty() )
		return;

	if( mEnableLoading == true )
		return;

	/// 사운드 노드들을 처리
	cSphere testSphere;
	{
		testSphere.Set( mHeroPos, 100.0f );
		mPickedArray.Clear();

		Pick_Type( &mPickedArray, testSphere, cSceneNode::eSOUND );
/*
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				if( ((cSoundSceneNode*)mPickedArray[i])->IsRemoved() )
					continue;
				((cSoundSceneNode*)mPickedArray[i])->OnListenerEntered( testSphere );
			}
		}
*/
		{
			cSoundMap::cIterator i, iend;
			i = mSoundArray.Begin();
			iend = mSoundArray.End();
			for( ; i != iend; ++i )
			{
				cSoundSceneNode* n = (cSoundSceneNode*)(*i).mSecond;
				if( n->IsRemoved() == false )
					n->OnProcess( deltaTime, accumTime );
			}
		}
	}
	/*
	/// 조명 노드들을 처리
	{
	testSphere.Set( heroPos, 10000.0f );
	mPickedArray.Clear();

	if( Pick_Type( &mPickedArray, testSphere, cSceneNode::eLIGHT ) )
	{
	for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
	{
	((cLightSceneNode*)mPickedArray[i])->OnProcess();
	}
	}
	}
	*/

	/// 정적 노드들을 처리
	{
		cNodeMap::cIterator i, iend;
		i = mStaticObjectArray.Begin();
		iend = mStaticObjectArray.End();
		for( ; i != iend; ++i )
		{
			cStaticSceneNode* n = (cStaticSceneNode*)(*i).mSecond;
			if( n->IsViewNode() )
				n->OnProcess( deltaTime, accumTime );
		}
	}

	///
	{
		cNodeMap::cIterator i, iend;
		i = mDynamicObjectArray.Begin();
		iend = mDynamicObjectArray.End();
		for( ; i != iend; ++i )
		{
			cDynamicSceneNode* n = (cDynamicSceneNode*)(*i).mSecond;
			n->OnProcess( deltaTime, accumTime );
		}
	}

	{
		cNodeMap::cIterator i, iend;
		i = mEffectNodeArray.Begin();
		iend = mEffectNodeArray.End();
		for( ; i != iend; ++i )
		{
			cEffectSceneNode* n = (cEffectSceneNode*)(*i).mSecond;
			if( n->IsRemoved() )
				continue;

			n->OnProcess( deltaTime, accumTime );

			if( n->IsRemoved() )
				continue;

			if( n->IsViewNode() == false )
				continue;

			if( n->IsEnableFrustumFlag() )
			{
				if( n->IsDamageEffect() )
					mDamageCuller.Cull( *cam, n );
				else
					mCuller.Cull( *cam, n );
			}
		}
	}

	/// 컬링
	mNodeTree.Cull( &mSolidArray, &mAlphaArray, &mAlphaTestArray, cam, cSceneNode::eRENDERING );

	/// 정렬
	mSolidArraySorter.Sort( *cam );
	mAlphaArraySorter.Sort( *cam );
	mDamageSorter.Sort( *cam );

	if( mpCurrentSceneNode )
	{
		if( mpCurrentSceneNode->GetType() == cSceneNode::ePLAYER )
			InterpretPlayer( accumTime );
		else if( mpCurrentSceneNode->GetType() == cSceneNode::eMONSTER )
			InterpretMonster( accumTime );
		else
			InterpretNpc( accumTime );
	}
}

void cSceneManager::Render()
{
	if( mInited == false )
		return;

	/// 솔리드 개체
	mSolidArraySorter.Render();

	/// 알파 개체
	mAlphaArraySorter.Render();

	RenderTrail();
}

void cSceneManager::RenderZFalseDamage()
{
	/// Damage Geom
	mDamageSorter.Render();
}

void cSceneManager::RenderTrail()
{
	if( mTrailGeomArray.IsEmpty() == true )
		return;

	cRenderer* renderer = RENDERSYS->GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 );
		return;
	}

	for( unsigned int i = 0,iend = mTrailGeomArray.GetSize(); i< iend; ++i )
	{
		cRibbonGeometry* geom = mTrailGeomArray[i];
		if( geom )
			geom->RenderImmediate( renderer );
	}

	mTrailGeomArray.Clear();
}


bool cSceneManager::Pick_Kind( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type )
{
	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, type );
}

bool cSceneManager::Pick_Type( tArray<void*>* pickedArray, const cSphere& sphere, cSceneNode::eType type )
{
	return mNodeTree.CollideSphere_Type( pickedArray, sphere, type );
}

bool cSceneManager::PickAreas( tArray<void*>* pickedArray, const cSphere& sphere )
{
	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eAREA );
}

bool cSceneManager::PickLights( tArray<void*>* pickedArray, const cSphere& sphere )
{
	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eLIGHT );
}

bool cSceneManager::PickSounds( tArray<void*>* pickedArray, const cSphere& sphere )
{
	return mNodeTree.CollideSphere_Kind( pickedArray, sphere, cSceneNode::eSOUND );
}

bool cSceneManager::PickStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
{
	NiPoint3 origin, dir;
	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) )
		return false;

	return PickStatics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
}

bool cSceneManager::PickStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
{
	return mNodeTree.CollideRay_Kind( pickedArray, ray, maxDistance, sortByDistance, cSceneNode::eSTATIC );
}

bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
{
	NiPoint3 origin, dir;
	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) == false )
		return false;

	return PickDynamics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
}

bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
{
	return mNodeTree.CollideRay_Kind( pickedArray, ray, maxDistance, sortByDistance, cSceneNode::eDYNAMIC );
}

bool cSceneManager::PickManagedStatics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
{
	NiPoint3 origin, dir;
	if( CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY ) == false )
		return false;

	return PickManagedStatics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
}

bool cSceneManager::PickManagedStatics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
{
	return mNodeTree.CollideRay_Kind( pickedArray, ray, maxDistance, sortByDistance, cSceneNode::eMANAGEDSTATIC );
}

bool cSceneManager::AddSoundNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()==cSceneNode::eSOUND );

	unsigned long idx = mSoundIndexGen.GenerateIndex();
	if( mSoundArray.GetAt(idx) != 0 )
	{
		assert(0);
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

bool cSceneManager::AddDynamicNode( cSceneNode* node )
{
	assert( node );
	assert( node->GetType()&cSceneNode::eDYNAMIC );

	unsigned long idx = mDynamicNodeIndexGen.GenerateIndex();
	if( mDynamicObjectArray.GetAt(idx) != 0 )
	{
		assert(0);
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

void cSceneManager::AddDeleteEffectNode( cEffectSceneNode* node )
{
	if( node == 0 )
	{
		assert(0);
		return;
	}

	if( IsValidEffectNode( node->GetIndexByManger(), node ) == false )
	{
		assert(0);
		return;
	}

	mDelEffectList.PushBack( node );
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

bool cSceneManager::DestroyDynamic( cSceneNode* node )
{
	assert( node );

	if( (node->GetType() & cSceneNode::eDYNAMIC) == 0 )
	{
		assert(0);
		return false;
	}

	unsigned long idx = node->mIndexByManager;
	cDynamicSceneNode* n = (cDynamicSceneNode*)mDynamicObjectArray.GetAt( idx );
	if( n == 0 )
	{
		assert(0);
		return false;
	}
	if( n != node )
	{
		assert(0);
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
		assert(0);
	}

	mDynamicNodeIndexGen.RestoreIndex( idx );

	return true;
}

bool cSceneManager::DestroyEffectNode( cSceneNode* node )
{
	assert( node );

	if( node->GetType() != cSceneNode::eEFFECT )
	{
		assert(0);
		return false;
	}

	unsigned long idx = node->mIndexByManager;
	cEffectSceneNode* n = (cEffectSceneNode*)mEffectNodeArray.GetAt( idx );
	if( n == 0 )
	{
		assert(0);
		return false;
	}
	if( n != node )
	{
		assert(0);
		return false;
	}

	if( n->IsRemoved() == false )
	{
		assert(0);
		return false;
	}

	if( n->mContainer )
	{
		assert(0);
		n->mContainer->Remove(n);
		n->mContainer = 0;
	}

	delete n;

	mEffectNodeArray.Erase( idx );
	n = (cEffectSceneNode*)mEffectNodeArray.GetAt( idx );
	if( n )
	{
		assert(0);
	}

	mEffectIndexGen.RestoreIndex( idx );

	return true;
}

bool cSceneManager::DestroySoundNode( cSceneNode* node )
{
	assert( node );

	if( node->GetType() != cSceneNode::eSOUND )
	{
		assert(0);
		return false;
	}

	unsigned long idx = node->mIndexByManager;
	cSoundSceneNode* n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
	if( n == 0 )
	{
		assert(0);
		return false;
	}
	if( n != node )
	{
		assert(0);
		return false;
	}

	if( n->mContainer )
	{
		n->mContainer->Remove(n);
		n->mContainer = 0;
	}

	delete n;

	mSoundArray.Erase( idx );
	n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
	if( n )
	{
		assert(0);
	}

	mSoundIndexGen.RestoreIndex( idx );

	return true;
}

bool cSceneManager::DestroyStaticNode( cSceneNode* node )
{
	assert( node );

	if( node->GetType() != cSceneNode::eSTATIC )
	{
		assert(0);
		return false;
	}

	unsigned long idx = node->mIndexByManager;
	cStaticSceneNode* n = (cStaticSceneNode*)mStaticObjectArray.GetAt( idx );
	if( n == 0 )
	{
		assert(0);
		return false;
	}
	if( n != node )
	{
		assert(0);
		return false;
	}


	if( n->mContainer )
	{
		n->mContainer->Remove(n);
		n->mContainer = 0;
	}

	delete n;

	mStaticObjectArray.Erase( idx );
	mStaticNodeIndexGen.RestoreIndex( idx );

	return true;
}

void cSceneManager::DestroyAll()
{
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
	mDelSoundList.Clear();

	///
	mNodeTree.Clear();
	mStaticNodeIndexGen.RestoreAll();
	mDynamicNodeIndexGen.RestoreAll();
	mEffectIndexGen.RestoreAll();
}

cSoundSceneNode* cSceneManager::CreateEffectSound( cSoundSceneNodeParam& param )
{
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

cPlayerSceneNode* cSceneManager::CreatePlayer( unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
	{
		NiMessageBox("Find not ModelFileName","Error");
		return 0;
	}

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Character/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	/// 
	cPlayerSceneNodeParam param;
	param.mPathName = pathName;

	cPlayerSceneNode* node = new cPlayerSceneNode;
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init player scene node: %s"), name );
		NiMessageBox( msg.Cstr(), "Error" );
	}

	/// 노드를 추가
	if( AddDynamicNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}
	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );

	return node;
}

void cSceneManager::CreateDummy( NiPoint3 pos )
{
	if( mpDummyNode )
	{
		DestroyStaticNode( mpDummyNode );
		mpDummyNode = 0;
	}

	NiNode* dummy = NiNew NiNode;
	pos.y -= 200.0f;
	dummy->SetTranslate(pos);

	cStaticSceneNode* dNode = new cStaticSceneNode;
	dNode->Init( dummy );

	if( AddStaticNode( dNode ) == false )
	{
		delete dNode;
		mpDummyNode = 0;

		assert(0);
		return;
	}

	mpDummyNode = dNode;
}

cMonsterSceneNode* cSceneManager::CreateMonster( unsigned long monIdx, unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
		return 0;

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Monster/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	/// 노드를 생성
	cMonsterSceneNode* node = new cMonsterSceneNode;

	cMonsterSceneNodeParam param;
	param.mPathName = pathName;
	param.mMonsterIndex = monIdx;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init monster scene node: %s"), param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	if( AddDynamicNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}

	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );

	return node;
}

cNpcSceneNode* cSceneManager::CreateNPC( unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
		return 0;

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Npc/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	cNpcSceneNodeParam param;
	param.mPathName = pathName;

	/// 노드를 생성
	cNpcSceneNode* node = new cNpcSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init npc scene node: %s"), param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	if( AddDynamicNode( node ) == false )
	{
		assert(0);
		delete node;
		return 0;
	}
	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );

	return node;
}

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

unsigned int cSceneManager::CalcVisibleLevel( cStaticSceneNode* n )
{
	if( n == 0 )
	{
		assert(0);
		return 0;
	}

#ifdef _GMTOOL
	if( CAMERAMAN->GetCurrentIdx() == 2 )
		return 0;
#endif

	float sqrDist = (mHeroPos - n->GetCenter()).SqrLength();

	if( sqrDist > mSqaredLevelDistance2 )
		return 3;
	if( sqrDist > mSqaredLevelDistance1 )
		return 2;
	if( sqrDist > mSqaredLevelDistance0 )
		return 1;
	return 0;
}

bool cSceneManager::CalcDynamicVisibleLevel( cSceneNode* n )
{
	if( n == 0 )
	{
		assert(0);
		return false;
	}

	float sqrDist = (mHeroPos - n->GetCenter()).SqrLength();
	if( sqrDist > 16000000.0f ) // 4000 * 4000
		return false;
	else
		return true;
}

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

bool cSceneManager::IsValidSoundNode( cSoundSceneNode* node )
{
	cNodeMap::cIterator i, iend;
	i = mSoundArray.Begin();
	iend = mSoundArray.End();

	for( ; i != iend; ++i )
	{
		cSoundSceneNode* p = (cSoundSceneNode*)(*i).mSecond;
		if( p == node )
			return true;
	}

	return false;
}


bool cSceneManager::IsValidSoundNode( unsigned long idx, cSoundSceneNode* node )
{
	cSoundSceneNode* n = (cSoundSceneNode*)mSoundArray.GetAt( idx );
	if( n == 0 )
		return false;
	if( n != node )
		return false;

	return true;
}


bool cSceneManager::IsValidDynamicNode( cDynamicSceneNode* node )
{
	cNodeMap::cIterator i, iend;
	i = mDynamicObjectArray.Begin();
	iend = mDynamicObjectArray.End();
	for( ; i != iend; ++i )
	{
		cDynamicSceneNode* p = (cDynamicSceneNode*)(*i).mSecond;
		if( p == node )
			return true;
	}

	return false;
}

bool cSceneManager::IsValidDynamicNode( unsigned long idx, cDynamicSceneNode* node )
{
	cDynamicSceneNode* n = (cDynamicSceneNode*)mDynamicObjectArray.GetAt( idx );
	if( n == 0 )
		return false;
	if( n != node )
		return false;

	return true;
}

void cSceneManager::InterpretPlayer( unsigned long /*accumtime*/ )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float checkTime = (float)mpCurrentSceneNode->GetScaleAccumTime() * 0.001f;
//	checkTime += (accumtime - mpCurrentSceneNode->GetLastTime()) * 0.001f;

	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( fEndTime == NI_INFINITY )
		return;

	if( fEndTime == cActorManagerForPartition::INVALID_TIME )
	{
		fEndTime = mpCurrentSceneNode->GetTargetAnimationScaleEndTime() * 0.001f;
	}

	if( mDramaProcess )
	{
		if( fEndTime > 0 && fEndTime < checkTime )
		{
			if( id >= ANITYPE_CASTING1 && id <=ANITYPE_CASTING3 )
			{
				return;
			}
			else if( id >= ANITYPE_IDLE && id < ANITYPE_IDLE2 )
			{
				return;
			}
			else
			{
				unsigned char state = CHARACTERFORM->GetCurrentWeaponState();
				mpCurrentSceneNode->SetTargetAnimation( state );
			}
		}
	}
	else
	{
		if( fEndTime > 0 && fEndTime < checkTime )
		{
			mpCurrentSceneNode->SetTargetAnimation( mpCurrentSceneNode->GetTargetAnimation() );
		}
	}
}

void cSceneManager::InterpretMonster( unsigned long /*accumtime*/ )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float checkTime = (float)mpCurrentSceneNode->GetScaleAccumTime() * 0.001f;
	//checkTime += (accumtime - mpCurrentSceneNode->GetLastTime()) * 0.001f;

	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( fEndTime == NI_INFINITY )
		return;

	if( fEndTime == cActorManagerForPartition::INVALID_TIME )
	{
		fEndTime = mpCurrentSceneNode->GetTargetAnimationScaleEndTime() * 0.001f;
	}

	if( mDramaProcess )
	{

		if( fEndTime > 0 && fEndTime <= checkTime )
		{
			if( id != M_ANITYPE_CASTING )
				mpCurrentSceneNode->UpdateTargetAnimation( M_ANITYPE_IDLE1 );
		}
	}
	else
	{
		if( fEndTime > 0 && fEndTime <= checkTime )
		{
			mpCurrentSceneNode->UpdateTargetAnimation( mpCurrentSceneNode->GetTargetAnimation() );
		}
	}
}

void cSceneManager::InterpretNpc( unsigned long /*accumtime*/ )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float checkTime = (float)mpCurrentSceneNode->GetScaleAccumTime() * 0.001f;
	//checkTime += (accumtime - mpCurrentSceneNode->GetLastTime()) * 0.001f;

	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );
	if( fEndTime == NI_INFINITY )
		return;

	if( fEndTime == cActorManagerForPartition::INVALID_TIME )
	{
		fEndTime = mpCurrentSceneNode->GetTargetAnimationScaleEndTime() * 0.001f;
	}

	if( mDramaProcess )
	{
		if( fEndTime > 0 && fEndTime <= checkTime )
			mpCurrentSceneNode->UpdateTargetAnimation( 0 );
	}
	else
	{
		if( fEndTime > 0 && fEndTime <= checkTime )
		{
			mpCurrentSceneNode->UpdateTargetAnimation( mpCurrentSceneNode->GetTargetAnimation() );
		}
	}
}

void cSceneManager::AddAmbientLight( NiColor ambientColor, NiColor diffuseColor, float dimmer )
{
	NiAmbientLight* p = NiNew NiAmbientLight;
	p->SetAmbientColor( ambientColor );
	p->SetDiffuseColor( diffuseColor );
	p->SetSpecularColor( NiColor::BLACK );
	p->SetDimmer( dimmer );

	mAmbientLightArray.AddFirstEmpty( p );
}

void cSceneManager::AddDirLight( NiColor ambientColor, NiColor diffuseColor, float dimmer, NiMatrix3 rot )
{
	NiDirectionalLight* p = NiNew NiDirectionalLight;
	p->SetAmbientColor( ambientColor);
	p->SetDiffuseColor( diffuseColor );
	p->SetSpecularColor( NiColor::BLACK );
	p->SetDimmer( dimmer );
	p->SetRotate( rot );
	p->Update( 0.0f );

	mDirLightArray.AddFirstEmpty( p );
}

void cSceneManager::ChangeLight( unsigned int index )
{
	if( index >= mAmbientLightArray.GetSize() )
	{
		assert(0);
		return;
	}

	if( mpCurrentSceneNode )
	{
		NiNode* node = mpCurrentSceneNode->GetNiNode();

//		node->DetachEffect( mAmbientLightArray.GetAt( mCurrentLightIndex ) );
//		node->DetachEffect( mDirLightArray.GetAt( mCurrentLightIndex ) );

//		node->AttachEffect( mAmbientLightArray.GetAt( index ) );
//		node->AttachEffect( mDirLightArray.GetAt( index ) );

		node->UpdateEffects();
	}

	mCurrentLightIndex = index;
}

void cSceneManager::LoadMapLightSetting( const char* fullPath )
{
	cFileLoader loader;
	if( loader.Open( fullPath, true ) == false )
	{
		assert( 0 && "failed to open file to load scene" );
		return;
	}

	cSceneFileHeader header;

	if( loader.Read( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		assert( 0 && "failed to load scene file header" );
		goto _ERR;
	}

	/// 헤더 검사
	if( ::memcmp( header.mCode, "IrisScene", 10 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		goto _ERR;
	}

	switch( header.mVersion )
	{
	case 6:
		if( LoadEnv6( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea6( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 7:
		if( LoadEnv7( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea7( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}

		break;
	case 8:
		if( LoadEnv8( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea8( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 9:
		if( LoadEnv9( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea9( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 10:
	case 11:
		if( LoadEnv10( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea10( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	default:
		assert( 0 && "invalid file version" );
		goto _ERR;
	}

_ERR:
	loader.Close();
}


void cSceneManager::ScanDir( LPCTSTR pathname )
{
	WIN32_FIND_DATA info;
	HANDLE h_dir_info = ::FindFirstFile(pathname, &info);
	if( h_dir_info == INVALID_HANDLE_VALUE ) 
		return ;

	NiFilename path = NiFilename(pathname);
	do 
	{
		if( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		char sceneFullPath[MAX_PATH] = {0,};
		::sprintf_s( sceneFullPath, "%s%s%s", path.GetDrive(), path.GetDir(), info.cFileName );
		LoadMapLightSetting( sceneFullPath );
	} 
	while( ::FindNextFile(h_dir_info, &info) );

	::FindClose( h_dir_info );
}

bool cSceneManager::LoadEnv6( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyHeight = 0.f;
	loader.ReadFloat( &skyHeight );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea6( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv7( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea7( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv8( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea8( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv9( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea9( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv10( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea10( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

void cSceneManager::OnVisibleModifierPocess( cSceneNode* node )
{
	mModifierProcess.Cull( *CAMERAMAN->GetCurrent(), node );
}


/*
#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "GameResourceManager.h"
#include "CameraManager.h"
#include "LookAtCamera.h"

#include "Player_Common.h"

#include "Ray.h"
#include "DynamicSceneNode.h"
#include "StaticSceneNode.h"
#include "NpcSceneNode.h"
#include "MonsterSceneNode.h"
#include "PlayerSceneNode.h"
#include "EffectSceneNode.h"
#include "SoundSceneNode.h"


#include "CharacterForm.h"
#include "ModelApp.h"

#include "Application.h"

enum eMONSTER_ANIMATIONTYPE
{ 
	M_ANITYPE_IDLE1 = 1,	/// 대기
	M_ANITYPE_IDLE2,		/// 대기보조
	M_ANITYPE_WORK,			/// 걷기
	M_ANITYPE_RUN,			/// 달리기
	M_ANITYPE_ATTACK1,		/// 공격1
	M_ANITYPE_ATTACK2,		/// 공격2
	M_ANITYPE_ATTACK3,		/// 공격3
	M_ANITYPE_CASTING,		/// 캐스팅
	M_ANITYPE_SKILL1,		/// 스킬1
	M_ANITYPE_SKILL2,		/// 스킬2
	M_ANITYPE_DAMAGE1,		/// 데미지1
	M_ANITYPE_DAMAGE2,		/// 데미지2
	M_ANITYPE_DIE1,			/// 죽음1
	M_ANITYPE_DIE2,			/// 죽음2

	M_ANITYPE_MODEANI = 21,
};


cSceneCuller::cSceneCuller( const cCamera* cam, const NiPoint3& heroPos )
: NiCullingProcess( 0, false )
{
	assert( cam && "null camera" );

	m_pkCamera = (NiCamera*)cam;
	mCamera = cam;

	mHeroPos = heroPos;
}

cSceneManager* cSceneManager::mSingleton = 0;

cSceneManager::cSceneManager()
: mInited( false )
, mTextureMap( 12 )
, mAlphaArray( 2048, 1024 )
, mSolidArray( 2048, 1024 )
, mSkinedArray( 2048, 1024 )
, mCuller( &mAlphaArray, &mSolidArray )
, mAlphaArraySorter( &mAlphaArray )
, mSolidArraySorter( &mSolidArray )
, mSkinedArraySorter( &mSkinedArray )
, mCenter( NiPoint3::ZERO )
, mMinRadius( 0.0f )
, mMaxRadius( 0.0f )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mpCurrentSceneNode = 0;
	mDynamicArray.Reserve( 2048 );
	mStaticObjectArray.Reserve( 2048 );

	mEffectNodeArray.Reserve( 5000 );
	mSoundArray.Reserve( 128 );

	mAmbientLightArray.SetSize( 20 );
	mAmbientLightArray.SetGrowBy( 10 );
	mDirLightArray.SetSize( 20 );
	mDirLightArray.SetGrowBy( 10 );

	mCurrentLightIndex = 0;

	mDramaProcess = false;

	mpDummy = 0;
	mDummyIdx = UINT_MAX;

	///// 안개 속성을 생성
	//mFogProp = NiNew NiFogProperty;
	//mFogProp->SetFog( false );
	//mFogProp->SetDepth( 0.1f );

	///// 주변광을 생성
	//for( unsigned int i = 0; i < 2; ++i )
	//{
	//	NiAmbientLight* p = mAmbientLight[i] = NiNew NiAmbientLight;
	//	p->SetAmbientColor( NiColor::WHITE );
	//	p->SetDiffuseColor( NiColor::WHITE );
	//	p->SetSpecularColor( NiColor::BLACK );
	//}

	///// 방향광을 생성
	//for( unsigned int i = 0; i < 2; ++i )
	//{
	//	NiDirectionalLight* p = mDirLight[i] = NiNew NiDirectionalLight;
	//	p->SetAmbientColor( NiColor::WHITE );
	//	p->SetDiffuseColor( NiColor::WHITE );
	//	p->SetSpecularColor( NiColor::BLACK );
	//}

	///
	mPickedArray.Reserve( 1024 );
}

cSceneManager::~cSceneManager()
{
	Clear();
	mSingleton = 0;
}

void cSceneManager::Clear()
{
	mInited = false;
	mTextureMap.Clear();

	mDirLightArray.RemoveAll();
	mAmbientLightArray.RemoveAll();

	mAlphaArray.RemoveAll();
	mSolidArray.RemoveAll();
	mSkinedArray.RemoveAll();

	DestroyAll();
	RESOURCEMAN->Clear();
}

bool cSceneManager::Init( const NiPoint3& center, float minRadius, float maxRadius )
{
	/// 지움
	Clear();

	mDynamicTree.Init( center, minRadius, maxRadius );
	mStaticObjectTree.Init( center, minRadius, maxRadius );
	mSoundTree.Init( center, minRadius, maxRadius );
	mInited = true;

	/// scene info load (character light)
	TCHAR dir[MAX_PATH] = {0,};
	cString str = theApp.GetBaseDir();
	::sprintf_s( dir, _T("%s\\data\\map\\*"), str.Cstr() );
	ScanDir(dir);

	AddAmbientLight( NiColor::WHITE, NiColor::WHITE, 1.0f );

	cStaticSceneNode* node = new cStaticSceneNode;
	cStaticSceneNodeParam param;
	param.mPathName = "./Data/Effect/plane.nif";
	node->Init( param );

	AddStatic( node );
	return true;
}

bool cSceneManager::InitTree( const NiPoint3& center, float minRadius, float maxRadius )
{
	mCenter = center;
	mMinRadius = minRadius;
	mMaxRadius = maxRadius;

	/// 트리를 재생성
	mDynamicTree.Init( center, minRadius, maxRadius );
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		mDynamicTree.Push( (cDynamicSceneNode*)mDynamicArray[i] );
	}

	mStaticObjectTree.Init( center, minRadius, maxRadius );
	for( unsigned int i = 0, iend = mStaticObjectArray.GetSize(); i < iend; ++i )
	{
		mDynamicTree.Push( (cStaticSceneNode*)mStaticObjectArray[i] );
	}

	mSoundTree.Init( center, minRadius, maxRadius );
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		mSoundTree.Push( (cSoundSceneNode*)mSoundArray[i] );
	}

	return true;
}

void cSceneManager::Process( float deltaTime, float accumTime )
{
	if( mInited == false )
		return;

	const cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 && "can't get current camera" );
		return;
	}

	/// 제거될 이펙트들을 처리
	if( mDelEffectList.IsEmpty() == false )
	{
		cDelEffectList::cIterator i = mDelEffectList.Begin();
		cDelEffectList::cIterator end = mDelEffectList.End();
		for( ; i != end; ++i )
		{
			cSceneNode* n = (cSceneNode*)*i;
			DestroyEffectNode(n);
		}
		mDelEffectList.Clear();
	}

	/// 제거될 사운드들을 처리
	if( mDelSoundList.IsEmpty() == false )
	{
		cDelSoundList::cIterator i = mDelSoundList.Begin();
		cDelSoundList::cIterator end = mDelSoundList.End();
		for( ; i != end; ++i )
		{
			cSceneNode* n = (cSceneNode*)*i;
			DestroySoundNode(n);
		}
		mDelSoundList.Clear();
	}

	if( mpCurrentSceneNode )
	{
		if( mpCurrentSceneNode->GetType() == cSceneNode::ePLAYER )
			InterpretPlayer( accumTime );
		else if( mpCurrentSceneNode->GetType() == cSceneNode::eMONSTER )
			InterpretMonster( accumTime );
		else
			InterpretNpc( accumTime );

	}

	cSceneCuller culler( cam, NiPoint3::ZERO );
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		((cDynamicSceneNode*)mDynamicArray[i])->OnProcess( deltaTime, accumTime );
	}

	for( unsigned int i = 0, iend = mStaticObjectArray.GetSize(); i < iend; ++i )
	{
		((cStaticSceneNode*)mStaticObjectArray[i])->OnProcess( deltaTime,accumTime );
	}

	/// 사운드 노드들을 처리
	cSphere testSphere( NiPoint3(0.0f, 0.0f, 0.0f), 100.f );
	mPickedArray.Clear();

	if( PickSounds( &mPickedArray, testSphere ) )
	{
		for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
		{
			cSoundSceneNode* n = (cSoundSceneNode*)mPickedArray[i];

			n->OnListenerEntered( testSphere );
		}
	}
	{
		for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
		{
			((cSoundSceneNode*)mSoundArray[i])->OnProcess( deltaTime, accumTime );
		}
	}

	/// 컬링
	NiFrustumPlanes planes( *((NiCamera*)(*cam)) );

	mAlphaArray.RemoveAll();
	mSolidArray.RemoveAll();
	mSkinedArray.RemoveAll();

	mStaticObjectTree.Cull( &mSolidArray, &mSkinedArray, &mAlphaArray, planes, culler );
	mDynamicTree.Cull( &mSolidArray, &mSkinedArray, &mAlphaArray, planes, culler );

	for( unsigned int i = 0, iend = mEffectNodeArray.GetSize(); i < iend; ++i )
	{
		cEffectSceneNode* n = (cEffectSceneNode*)mEffectNodeArray[i];

		n->OnProcess( deltaTime, accumTime );
		mCuller.Process( *cam, n );
	}

	/// 정렬
	mSolidArraySorter.Sort( *cam );
	mSkinedArraySorter.Sort( *cam );
	mAlphaArraySorter.Sort( *cam );
}

void cSceneManager::Render()
{
	if( mInited == false )
		return;

	/// 그림자 배치 렌더링
	cRenderer* renderer = RENDERSYS->GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 && "null renderer" );
		return;
	}

	///
	mSolidArraySorter.Render();
	mSkinedArraySorter.Render();

	/// 알파 객체 랜더링
	mAlphaArraySorter.Render();
}
bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, int mouseX, int mouseY, bool sortByDistance )
{
	NiPoint3 origin, dir;
	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );

	return PickDynamics( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance );
}

bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance )
{
	return mDynamicTree.CollideRay( pickedArray, ray, maxDistance, sortByDistance );
}

bool cSceneManager::PickDynamics( tArray<void*>* pickedArray, const cSphere& sphere )
{
	return mDynamicTree.CollideSphere( pickedArray, sphere );
}
bool cSceneManager::PickSounds( tArray<void*>* pickedArray, const cSphere& sphere )
{
	return mSoundTree.CollideSphere( pickedArray, sphere );
}

unsigned int cSceneManager::AddSound( cSoundSceneNode* node )
{
	assert( node && "null node" );

	/// 트리에 추가
	mSoundTree.Push( node );

	/// 사운드 배열에 추가
	unsigned int i = mSoundArray.GetSize();
	mSoundArray.PushBack( node );
	return i;
}

unsigned int cSceneManager::AddDynamic( cDynamicSceneNode* node )
{
	assert( node );

	/// 트리에 추가
	mDynamicTree.Push( node );

	/// 노드 배열에 추가
	unsigned int i = mDynamicArray.GetSize();
	mDynamicArray.PushBack( node );
	return i;
}

unsigned int cSceneManager::AddStatic( cSceneNode* node )
{
	assert( node && "null node" );

	/// 트리에 추가
	mStaticObjectTree.Push( node );

	/// 노드 배열에 추가
	unsigned int i = mStaticObjectArray.GetSize();
	mStaticObjectArray.PushBack( node );
	return i;
}

unsigned int cSceneManager::AddEffectNode( cSceneNode* node )
{
	assert( node && "null node" );

	/// 노드 배열에 추가
	unsigned int i = mEffectNodeArray.GetSize();
	mEffectNodeArray.PushBack( node );
	return i;
}

void cSceneManager::AddDeleteEffectNode( cEffectSceneNode* node )
{
	if( node == 0 )
		return;
	if( node->IsRemoved() )
		return;

	node->SetRemoveFlag( true );

	mDelEffectList.Insert( mDelEffectList.End(), node );
}

void cSceneManager::AddDeleteSoundNode( cSoundSceneNode* node )
{
	if( node == 0 )
		return;
	if( node->IsRemoved() )
		return;

	node->SetRemoveFlag( true );

	mDelSoundList.Insert( mDelSoundList.End(), node );
}

void cSceneManager::DestroyAll()
{
	///
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		delete (cDynamicSceneNode*)mDynamicArray[i];
	}
	mDynamicArray.Clear();
	mDynamicTree.Clear();

	///
	for( unsigned int i = 0, end = mStaticObjectArray.GetSize(); i < end; ++i )
	{
		delete (cStaticSceneNode*)mStaticObjectArray[i];
	}
	mStaticObjectArray.Clear();
	mStaticObjectTree.Clear();

	for( unsigned int i = 0, end = mSoundArray.GetSize(); i < end; ++i )
	{
		delete (cSoundSceneNode*)mSoundArray[i];
	}
	mSoundArray.Clear();
	mSoundTree.Clear();

	///
	for( unsigned int i = 0, end = mEffectNodeArray.GetSize(); i < end; ++i )
	{
		delete (cSceneNode*)mEffectNodeArray[i];
	}
	mEffectNodeArray.Clear();

	mDelEffectList.Clear();
	mDelSoundList.Clear();
}

bool cSceneManager::DestroyDynamic( cSceneNode* node )
{
	assert( node );

	return DestroyDynamic( node->mIndexByManager );
}

bool cSceneManager::DestroyDynamic( unsigned int idx )
{
	//assert( node && "null node" );

	if( idx >= mDynamicArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	/// 트리에서 삭제
	cSceneNode* n = (cSceneNode*)mDynamicArray[idx];
	if( n->mContainer )
		n->mContainer->Remove( n );

	/// 제거
	delete n;

	/// 노드 배열에서 삭제
	bool orderChanged = false;
	if( mDynamicArray.PopAt( &orderChanged, idx ) == false)
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
		((cSceneNode*)mDynamicArray[idx])->mIndexByManager = idx;
	}
	return true;
}

bool cSceneManager::DestroyEffectNode( cSceneNode* node, bool DelListRemove )
{
	assert( node && "null node" );

	unsigned int idx = node->mIndexByManager;

	if( idx >= mEffectNodeArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	/// 트리에서 삭제
	cSceneNode* n = (cSceneNode*)mEffectNodeArray[idx];

	if( DelListRemove )
		mDelEffectList.Remove( node );

	/// 제거
	delete n;

	/// 노드 배열에서 삭제
	bool orderChanged = false;
	if( mEffectNodeArray.PopAt( &orderChanged, idx ) == false)
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
		((cSceneNode*)mEffectNodeArray[idx])->mIndexByManager = idx;
	}
	return true;
}

bool cSceneManager::DestroySoundNode( cSceneNode* node, bool DelListRemove )
{
	assert( node && "null node" );

	unsigned int idx = node->mIndexByManager;

	if( idx >= mSoundArray.GetSize() )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	/// 트리에서 삭제
	cSceneNode* n = (cSceneNode*)mSoundArray[idx];
	if( n->mContainer )
		n->mContainer->Remove( n );

	if( DelListRemove )
		mDelSoundList.Remove( node );

	/// 제거
	delete n;

	/// 노드 배열에서 삭제
	bool orderChanged = false;
	if( mSoundArray.PopAt( &orderChanged, idx ) == false)
		return false;

	if( orderChanged )
	{
		/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
		((cSceneNode*)mSoundArray[idx])->mIndexByManager = idx;
	}
	return true;
}

void cSceneManager::CreateDummy( NiPoint3 pos )
{
	if( mDummyIdx != UINT_MAX )
	{
		cStaticSceneNode* n = (cStaticSceneNode*)mStaticObjectArray[mDummyIdx];
		if( n->mContainer )
			n->mContainer->Remove( n );

		/// 제거
		delete n;

		/// 노드 배열에서 삭제
		bool orderChanged = false;
		if( mStaticObjectArray.PopAt( &orderChanged, mDummyIdx ) == false)
			return;

		if( orderChanged )
		{
			/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
			((cStaticSceneNode*)mStaticObjectArray[mDummyIdx])->mIndexByManager = mDummyIdx;
		}
	}

	NiNode* dummy = NiNew NiNode;
	pos.y -= 200.0f;
	dummy->SetTranslate(pos);

	cStaticSceneNode* dNode = new cStaticSceneNode;
	dNode->Init( dummy );
	mDummyIdx = AddStatic( dNode );
	mpDummy = NiDynamicCast(NiNode, dNode->GetNiObject() );
}

cPlayerSceneNode* cSceneManager::CreatePlayer( unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
	{
		NiMessageBox("Find not ModelFileName","Error");
		return 0;
	}

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Character/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	/// 
	cPlayerSceneNodeParam param;
	param.mPathName = pathName;

	cPlayerSceneNode* node = new cPlayerSceneNode;
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init player scene node: %s"), name );
		NiMessageBox( msg.Cstr(), "Error" );
	}

	/// 노드를 추가
	node->mIndexByManager = AddDynamic( node );
	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );


	return node;
}

cNpcSceneNode* cSceneManager::CreateNPC( unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
		return 0;

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Npc/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	cNpcSceneNodeParam param;
	param.mPathName = pathName;

	/// 노드를 생성
	cNpcSceneNode* node = new cNpcSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init npc scene node: %s"), param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	node->mIndexByManager = AddDynamic( node );
	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );

	return node;
}

cMonsterSceneNode* cSceneManager::CreateMonster( unsigned long modelIndex )
{
	const char* name = GAMERESOURCEMAN->GetModelFileName( modelIndex );
	if( name == 0 )
		return 0;

	char pathName[256] = {0,};
	::sprintf_s( pathName, "Data/Monster/%s", name );

	if( mpCurrentSceneNode !=0 )
		DestroyDynamic( mpCurrentSceneNode );

	/// 노드를 생성
	cMonsterSceneNode* node = new cMonsterSceneNode;

	cMonsterSceneNodeParam param;
	param.mPathName = pathName;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cStringT msg;
		msg.Format( _T("Failed to init monster scene node: %s"), param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	node->mIndexByManager = AddDynamic( node );
	mpCurrentSceneNode = node;

	///
	CreateDummy( node->GetObjectCenter() );

	return node;
}

cSoundSceneNode* cSceneManager::CreateEffectSound( const cSoundSceneNodeParam& param )
{
	/// 노드를 생성
	cSoundSceneNode* node = new cSoundSceneNode;

	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init sound scene node" );
		return 0;
	}

	/// 노드를 추가
	node->mIndexByManager = AddSound( node );

	return node;
}

cEffectSceneNode* cSceneManager::CreateEffect( const cEffectSceneNodeParam& param )
{
	cEffectSceneNode* node = new cEffectSceneNode;
	if( node->Init( param ) == false )
	{
		delete node;
		//		assert( 0 && "failed to init Effect scene node" );
		return 0;
	}

	node->mIndexByManager = AddEffectNode( node );
	return node;
}

cEffectSceneNode* cSceneManager::CreateSelfEffect( cString pathName, NiTransform& trans, bool bLoop )
{
	cEffectSceneNode* node = new cEffectSceneNode;
	if( node->InitSelfNode( pathName, trans, bLoop ) == false )
	{
		delete node;
		assert( 0 && "failed to init Effect scene node" );
		return 0;
	}

	node->mIndexByManager = AddEffectNode( node );
	return node;
}

void cSceneManager::ActiveSoundNodeAll( bool active )
{
	for( unsigned int i = 0, end = mSoundArray.GetSize(); i < end; ++i )
	{
		((cSoundSceneNode*)mSoundArray[i])->ActiveSound( active );
	}
}

//void cSceneManager::SetAmbientLightAmbient( unsigned int i, const NiColor& color )
//{
//	assert( i < 2 );
//
//	mAmbientLight[i]->SetAmbientColor( color );
//}
//
//void cSceneManager::SetAmbientLightDiffuse( unsigned int i, const NiColor& color )
//{
//	assert( i < 2 );
//
//	mAmbientLight[i]->SetDiffuseColor( color );
//}
//
//void cSceneManager::SetAmbientLightDimmer( unsigned int i, float dimmer )
//{
//	assert( i < 2 );
//
//	mAmbientLight[i]->SetDimmer( dimmer );
//}
//
//void cSceneManager::SetDirLightAmbient( unsigned int i, const NiColor& color )
//{
//	assert( i < 2 );
//
//	mDirLight[i]->SetAmbientColor( color );
//}
//
//void cSceneManager::SetDirLightDiffuse( unsigned int i, const NiColor& color )
//{
//	assert( i < 2 );
//
//	mDirLight[i]->SetDiffuseColor( color );
//}
//
//void cSceneManager::SetDirLightDimmer( unsigned int i, float dimmer )
//{
//	assert( i < 2 );
//
//	mDirLight[i]->SetDimmer( dimmer );
//}
//
//void cSceneManager::SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle )
//{
//	assert( i < 2 );
//
//	NiMatrix3 r;
//	r.FromEulerAnglesXYZ( xangle, yangle, zangle );
//
//	mDirLight[i]->SetRotate( r );
//	mDirLight[i]->Update( 0.0f );
//}
//
//void cSceneManager::SetFog( bool enabled, const NiColor& color, float depth )
//{
//	mFogProp->SetFog( enabled );
//	mFogProp->SetFogColor( color );
//	mFogProp->SetDepth( depth );
//}
//
//void cSceneManager::SetFogColor( const NiColor& color )
//{
//	mFogProp->SetFogColor( color );
//}
//
//void cSceneManager::SetFogDepth( float depth )
//{
//	mFogProp->SetDepth( depth );
//}

void cSceneManager::LoadMapLightSetting( const char* fullPath )
{
	cFileLoader loader;
	if( loader.Open( fullPath, true ) == false )
	{
		assert( 0 && "failed to open file to load scene" );
		return;
	}


	cSceneFileHeader header;

	if( loader.Read( &header, sizeof(cSceneFileHeader) ) != sizeof(cSceneFileHeader) )
	{
		assert( 0 && "failed to load scene file header" );
		goto _ERR;
	}

	/// 헤더 검사
	if( ::memcmp( header.mCode, "IrisScene", 10 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		goto _ERR;
	}

	switch( header.mVersion )
	{
	case 6:
		if( LoadEnv6( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea6( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 7:
		if( LoadEnv7( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea7( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}

		break;
	case 8:
		if( LoadEnv8( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea8( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 9:
		if( LoadEnv9( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea9( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	case 10:
		if( LoadEnv10( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		if( LoadGlobalArea10( loader ) == false )
		{
			assert( 0 );
			goto _ERR;
		}
		break;
	default:
		assert( 0 && "invalid file version" );
		goto _ERR;
	}

_ERR:
	loader.Close();
}

void cSceneManager::ScanDir( LPCTSTR pathname, bool filescan )
{
	WIN32_FIND_DATA info;
	HANDLE h_dir_info = ::FindFirstFile(pathname, &info);
	if( h_dir_info == INVALID_HANDLE_VALUE ) 
		return ;

	TCHAR sub_path[MAX_PATH] = {0,};
	do {
		if( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{     
			// 검색되어진 정보가 폴더라면...
			if( info.cFileName[0] != '.' )
			{     
				// 폴더명 맨 첫글자가 '.' 아니라면(상위폴더가 아니라면)
				cString str = theApp.GetBaseDir();

				cStringT check = info.cFileName;
				check = check.SubStr(0, 3);
				if( check.Compare( _T("map") ) != 0  && check.Compare( _T("Map") ) != 0 )
					continue;

				sprintf_s( sub_path, _T("%sMap\\%s\\*"), str.Cstr(), info.cFileName );

				// 해당 하위폴더를 검색한다.
				ScanDir( sub_path, true );
			}
		} 
		else
		{   
			if( filescan == false )
				continue;

			// 검색되어진 정보가 파일이라면...
			NiFilename name = NiFilename(info.cFileName);
			if( ::strcmp( name.GetExt(), ".scene" ) == 0 )
			{
				NiFilename path = NiFilename(pathname);

				char sceneFullPath[MAX_PATH] = {0,};
				::sprintf_s( sceneFullPath, "%s%s", path.GetDir(), info.cFileName );
				LoadMapLightSetting( sceneFullPath );
			}
			// 원하는 작업을 행하게 한다.
		}
	} while( ::FindNextFile(h_dir_info, &info) );

	::FindClose( h_dir_info );
}

bool cSceneManager::LoadEnv6( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyHeight = 0.f;
	loader.ReadFloat( &skyHeight );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea6( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv7( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea7( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv8( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea8( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv9( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea9( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

bool cSceneManager::LoadEnv10( cFileLoader& loader )
{
	/// 카메라
	float camFarDist;
	loader.ReadFloat( &camFarDist );

	/// 가시 레벨 거리
	float dist0 = 0.0f, dist1 = 0.0f, dist2 = 0.0f;
	loader.ReadFloat( &dist0 );
	loader.ReadFloat( &dist1 );
	loader.ReadFloat( &dist2 );

	/// 하늘
	char name[64];
	loader.Read( name, 64 );

	unsigned int skyEnabled = 0;
	loader.ReadUnsignedInt( &skyEnabled );

	float skyScale = 0.f;
	loader.ReadFloat( &skyScale );

	/// 방향광
	NiColor ambient;
	NiColor diffuse;
	float dimmer;
	NiMatrix3 r;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.Read( &diffuse, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );
	loader.Read( &r, sizeof(NiMatrix3) );

//	AddDirLight( ambient, diffuse, dimmer, r );

	return true;
}

bool cSceneManager::LoadGlobalArea10( cFileLoader& loader )
{
	/// 전체 영역의 기본 속성들을 로딩
	unsigned int type;
	char path[64];
	char name[64];
	NiPoint3 translate;
	NiMatrix3 rotate;
	float scale;

	loader.ReadUnsignedInt( &type );
	loader.Read( path, 64 );
	loader.Read( name, 64 );
	loader.Read( &translate, sizeof(NiPoint3) );
	loader.Read( &rotate, sizeof(NiMatrix3) );
	loader.ReadFloat( &scale );

	/// 전체 영역의 반지름을 로딩
	float radius;
	loader.ReadFloat( &radius );

	/// 전체 영역의 주변광을 로딩
	NiColor ambient;
	float dimmer;

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

	loader.Read( &ambient, sizeof(NiColor) );
	loader.ReadFloat( &dimmer );

//	AddAmbientLight( ambient, NiColor::WHITE, dimmer );

	return true;
}

void cSceneManager::AddAmbientLight( NiColor ambientColor, NiColor diffuseColor, float dimmer )
{
	NiAmbientLight* p = NiNew NiAmbientLight;
	p->SetAmbientColor( ambientColor );
	p->SetDiffuseColor( diffuseColor );
	p->SetSpecularColor( NiColor::BLACK );
	p->SetDimmer( dimmer );

	mAmbientLightArray.AddFirstEmpty( p );
}

void cSceneManager::AddDirLight( NiColor ambientColor, NiColor diffuseColor, float dimmer, NiMatrix3 rot )
{
	NiDirectionalLight* p = NiNew NiDirectionalLight;
	p->SetAmbientColor( ambientColor);
	p->SetDiffuseColor( diffuseColor );
	p->SetSpecularColor( NiColor::BLACK );
	p->SetDimmer( dimmer );
	p->SetRotate( rot );
	p->Update( 0.0f );

	mDirLightArray.AddFirstEmpty( p );
}

void cSceneManager::ChangeLight( unsigned int index )
{
	if( index >= mAmbientLightArray.GetSize() )
	{
		assert(0);
		return;
	}

	if( mpCurrentSceneNode )
	{
		NiNode* obj = NiDynamicCast( NiNode,  mpCurrentSceneNode->GetNiObject());

//		obj->DetachEffect( mAmbientLightArray.GetAt( mCurrentLightIndex ) );
//		obj->DetachEffect( mDirLightArray.GetAt( mCurrentLightIndex ) );

//		obj->AttachEffect( mAmbientLightArray.GetAt( index ) );
//		obj->AttachEffect( mDirLightArray.GetAt( index ) );

		obj->UpdateEffects();
	}

	mCurrentLightIndex = index;
}

void cSceneManager::InterpretPlayer( float accumtime )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );

	if( mDramaProcess )
	{
		if( fEndTime > 0 && fEndTime <= accumtime )
		{
			if( id >= ANITYPE_CASTING1 && id <=ANITYPE_CASTING3 )
			{

			}
			else
			{
				unsigned char state = CHARACTERFORM->GetCurrentWeaponState();
				mpCurrentSceneNode->UpdateTargetAnimation( state );
			}
		}
	}
	else
	{
		if( fEndTime == -FLT_MAX )
			mpCurrentSceneNode->UpdateTargetAnimation( id );
	}
}

void cSceneManager::InterpretMonster( float accumtime )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );

	if( mDramaProcess )
	{

		if( fEndTime > 0 && fEndTime <= accumtime )
		{
			if( id == M_ANITYPE_CASTING )
			{

			}
			else
				mpCurrentSceneNode->UpdateTargetAnimation( mpCurrentSceneNode->GetBaseAnimationIndex() );
		}
	}
	else
	{
		if( fEndTime == -FLT_MAX )
		{
			mpCurrentSceneNode->UpdateTargetAnimation( id );
		}
	}
}

void cSceneManager::InterpretNpc( float accumtime )
{
	cActorManagerForPartition*	pActor = mpCurrentSceneNode->GetActorManager();
	if( !pActor ) return;

	unsigned int id = mpCurrentSceneNode->GetTargetAnimation();
	float fEndTime = pActor->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );

	if( mDramaProcess )
	{
		if( fEndTime > 0 && fEndTime <= accumtime )
			mpCurrentSceneNode->UpdateTargetAnimation( 0 );
	}
	else
	{
		if( fEndTime == -FLT_MAX )
			mpCurrentSceneNode->UpdateTargetAnimation( id );
	}
}
*/