#include "stdafx.h"
#include "SceneManager.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "LookAtCamera.h"

#include "Ray.h"
#include "NaviMesh.h"
#include "Terrain.h"
#include "AreaGroup.h"
#include "AreaSceneNode.h"
#include "LightSceneNode.h"
#include "SoundSceneNode.h"
#include "StaticSceneNode.h"
#include "DynamicSceneNode.h"
#include "NpcSceneNode.h"
#include "MonsterSceneNode.h"
#include "PortalSceneNode.h"
#include "GatheringSceneNode.h"
#include "ShadowGeometry.h"
#include "../mapeditor/PatternSceneNode.h"

#include "../Hero.h"

cSceneCuller::cSceneCuller( const cCamera* cam, const NiPoint3& heroPos, bool testing )
: NiMeshCullingProcess( 0, false )
#ifdef MAP_EDITOR
, mSqaredLevelDistance0( 0.0f )
, mSqaredLevelDistance1( 0.0f )
, mSqaredLevelDistance2( 0.0f )
#endif
{
	assert( cam && "null camera" );

	m_pkCamera = (NiCamera*)cam;
	mCamera = cam;

	mHeroPos = heroPos;
	mPlaying = testing;
}

#ifdef MAP_EDITOR
unsigned int cSceneCuller::GetVisibleLevel( const NiPoint3& pos )
{
	float sqrDist = (mHeroPos - pos).SqrLength();

	if( sqrDist > mSqaredLevelDistance2 )
		return 3;
	if( sqrDist > mSqaredLevelDistance1 )
		return 2;
	if( sqrDist > mSqaredLevelDistance0 )
		return 1;
	return 0;
}
#endif

void cSceneManager::OnVisibleModifierPocess( cSceneNode* node )
{
	mModifierProcess.Cull( *CAMERAMAN->GetCurrent(), node );
}

//////////////////////////////////////////////////////////////////////////

cModifierProcess::cModifierProcess()
: NiMeshCullingProcess(NULL, NULL, true)
{
}

cModifierProcess::~cModifierProcess()
{

}

void cModifierProcess::Cull( const NiCamera* pcamera, cSceneNode* pnode )
{
	if( pcamera == 0 || pnode == 0 )
	{
		assert( 0 && "null camera or null node" );
		return;
	}

	if( pnode->GetNiObj() == 0 )
		return;

	NiVisibleArray temp;
	NiMeshCullingProcess::Cull( pcamera, pnode->GetNiObj(), &temp );
}

void cModifierProcess::Process( NiAVObject* pobj )
{
	pobj->OnVisible(*this);
}

void cModifierProcess::Process(const NiCamera* pkCamera, NiAVObject* pkScene, NiVisibleArray* pkVisibleSet)
{
	NIASSERT(pkCamera && pkScene);
	if (!pkCamera || !pkScene)
		return;

	m_pkCamera = pkCamera;
	SetFrustum(m_pkCamera->GetViewFrustum());

	m_pkVisibleSet = pkVisibleSet;

	if( pkScene )
		pkScene->Cull(*this);

	m_pkCamera = 0;
}

cSceneManager* cSceneManager::mSingleton = 0;

cSceneManager::cSceneManager()
: mInited( false )
, mTextureMap( 12 )
, mAlphaArray( 1024, 256 )
, mAlphaTestArray( 1024, 256 )
, mSolidArray( 1024, 256 )
, mAlphaArraySorter( &mAlphaArray, &mAlphaTestArray )
, mSolidArraySorter( &mSolidArray )
, mCenter( NiPoint3::ZERO )
, mMinRadius( 0.0f )
, mMaxRadius( 0.0f )
#ifdef MAP_EDITOR
, mStaticObjectWireframe(false)
#endif
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mAreaArray.Reserve( 64 );
	mLightArray.Reserve( 64 );
	mStaticArray.Reserve( 2048 );
	mDynamicArray.Reserve( 2048 );
#ifdef MAP_EDITOR
	mSoundArray.Reserve( 128 );
#endif

	/// 안개 속성을 생성
	mFogProp = NiNew NiFogProperty;
	mFogProp->SetFog( false );
	mFogProp->SetDepth( 0.1f );

	/// 주변광을 생성
	for( unsigned int i = 0; i < 2; ++i )
	{
		NiAmbientLight* p = mAmbientLight[i] = NiNew NiAmbientLight;
		p->SetAmbientColor( NiColor::WHITE );
		p->SetDiffuseColor( NiColor::WHITE );
		p->SetSpecularColor( NiColor::BLACK );
	}

	/// 방향광을 생성
	NiMatrix3 rot;
	rot.MakeIdentity();
	rot.SetCol( 0, 0.0f, 0.0f, -1.0f );

	for( unsigned int i = 0; i < 2; ++i )
	{
		NiDirectionalLight* p = mDirLight[i] = NiNew NiDirectionalLight;
		p->SetDimmer( 0.5f );
		p->SetAmbientColor( NiColor::WHITE );
		p->SetDiffuseColor( NiColor::WHITE );
		p->SetSpecularColor( NiColor::BLACK );
		p->SetRotate( rot );
		p->Update( 0.0f );
	}

	/// 전체 영역 노드를 생성
	cAreaSceneNodeParam param;
	param.mPathName = "global";

	cAreaSceneNode* n = mGlobalArea = new cAreaSceneNode;

	if( n->Init( param ) == false )
	{
		delete n;
		assert( 0 && "failed to init global area scene node" );
	}

	///
	mPickedArray.Reserve( 2048 );

	/// 가시 레벨 거리
	mLevelDistance0 = 5000.0f;
	mLevelDistance1 = 10000.0f;
	mLevelDistance2 = 20000.0f;
	mSqaredLevelDistance0 = mLevelDistance0 * mLevelDistance0;
	mSqaredLevelDistance1 = mLevelDistance1 * mLevelDistance1;
	mSqaredLevelDistance2 = mLevelDistance2 * mLevelDistance2;

	/*// 그림자 렌더링 속성을 생성
	mShadowZBuffProp = NiNew NiZBufferProperty;
	mShadowZBuffProp->SetZBufferTest( true );
	mShadowZBuffProp->SetZBufferWrite( false );

	mShadowAlphaProp = NiNew NiAlphaProperty;
	mShadowAlphaProp->SetAlphaBlending( true );
	mShadowAlphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_ZERO );
	mShadowAlphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_SRCCOLOR );

	mShadowTexProp = NiNew NiTexturingProperty;
	mShadowTexProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );

	NiTexture* tex = RESOURCEMAN->GetShadowTexture();
	assert(tex);
	NiTexturingProperty::ShaderMap* shaderMap = NiNew NiTexturingProperty::ShaderMap( tex, 0, NiTexturingProperty::CLAMP_S_CLAMP_T, NiTexturingProperty::FILTER_BILERP );
	mShadowTexProp->SetShaderMap( 0, shaderMap );
	*/

	/// 그림자 가시 배열
	mShadowGeomArray.Reserve( 1024 );
}

cSceneManager::~cSceneManager()
{
	delete mGlobalArea;
	Clear();
	mSingleton = 0;
}

void cSceneManager::Clear()
{
	mInited = false;
	mTextureMap.Clear();

	mAlphaTestArray.RemoveAll();
	mAlphaArray.RemoveAll();
	mSolidArray.RemoveAll();

	DestroyAll();
	RESOURCEMAN->Clear();
}

bool cSceneManager::Init( const cTerrain& terrain, float minRadius )
{
	/// 지형으로부터 필요한 정보를 계산
	const cBox& box = terrain.GetBoundBox();
	NiPoint3 center = (box.GetMin() + box.GetMax()) * 0.5f;
	center.z = 0.0f;
	float maxRadius = (box.GetMax() - center).Length() * 1.1f;

	if( minRadius * 2 >= maxRadius )
	{
		assert( 0 && "min radius is too big!" );
		return false;
	}

	/// 트리를 초기화
	Init( center, minRadius, maxRadius );
	return true;
}

bool cSceneManager::Init( const NiPoint3& center, float minRadius, float maxRadius )
{
	/// 지움
	Clear();

	/// 트리를 재생성
	mNodeTree.Init( center, minRadius, maxRadius );
	mInited = true;
	return true;
}

bool cSceneManager::InitTree( const cTerrain& terrain, float minRadius )
{
	/// 지형으로부터 필요한 정보를 계산
	const cBox& box = terrain.GetBoundBox();
	NiPoint3 center = (box.GetMin() + box.GetMax()) * 0.5f;
	center.z = 0.0f;
	float maxRadius = (box.GetMax() - center).Length() * 1.1f;

	if( minRadius * 2 >= maxRadius )
	{
		assert( 0 && "min radius is too big!" );
		return false;
	}

	InitTree( center, minRadius, maxRadius );
	return true;
}

bool cSceneManager::InitTree( const NiPoint3& center, float minRadius, float maxRadius )
{
	mCenter = center;
	mMinRadius = minRadius;
	mMaxRadius = maxRadius;

	/// 트리를 재생성
	mNodeTree.Init( center, minRadius, maxRadius );

	/// 트리에 노드들을 추가
	for( unsigned int i = 0, iend = mAreaArray.GetSize(); i < iend; ++i )
	{
		mNodeTree.Push( mAreaArray[i] );
	}
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		mNodeTree.Push( mLightArray[i] );
	}
	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		mNodeTree.Push( mStaticArray[i] );
	}
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		mNodeTree.Push( mDynamicArray[i] );
	}
#ifdef MAP_EDITOR
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		mNodeTree.Push( mSoundArray[i] );
	}
#endif
	return true;
}

void cSceneManager::Process( unsigned long deltaTime, unsigned long accumTime, bool playing )
{
	if( mInited == false )
		return;

	const cCamera* cam = CAMERAMAN->GetCurrent();
	if( cam == 0 )
	{
		assert( 0 && "can't get current camera" );
		return;
	}

	///
	cLookAtCamera* lookatCam = (cLookAtCamera*)cam;
#ifdef MAP_EDITOR
	if( playing )
	{
		mHeroPos = lookatCam->GetLookAt();
	}
	else
	{
		mHeroPos = cam->GetWorldTranslate();
	}

	cSceneCuller culler( cam, mHeroPos, playing );
	culler.mSqaredLevelDistance0 = mSqaredLevelDistance0;
	culler.mSqaredLevelDistance1 = mSqaredLevelDistance1;
	culler.mSqaredLevelDistance2 = mSqaredLevelDistance2;

#else
	mHeroPos = NiPoint3::ZERO;
//	const NiPoint3& heroPos = NiPoint3::ZERO;
	cSceneCuller culler( cam, mHeroPos, playing );
#endif

	///
	mShadowGeomArray.Clear();

	/// 카메라 충돌 검사
	if( playing && lookatCam->GetNeedUpdate() )
	{
		float lookAtDist = lookatCam->GetLookAtDistance();
		NiPoint3 invDir = -lookatCam->GetWorldDirection();
		invDir.Unitize();

		/// 차폐물과 충돌 검사
		bool collided = false;
		NiPoint3 pos;
		float dist = lookAtDist + 120.0f;
		mPickedArray.Clear();

		if( Pick( &mPickedArray, cRay( mHeroPos, invDir ), lookAtDist + 120.0f, true, SCENENODE_STATIC ) )
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				cStaticSceneNode* n = (cStaticSceneNode*)mPickedArray[i];
				n->mPickedByCamera = true;

				if( n->GetOccludeLevel() == 2 )
				{
					collided = true;
					dist = n->GetPickDistance() - 120.0f;

					if( dist < lookAtDist )
						lookAtDist = dist;
				}
			}
		}

		/// 네비메시와 충돌 검사
		dist = lookAtDist + 120.0f;

		if( collided == false && NAVIMESH->Pick( &pos, &dist, cRay(mHeroPos, invDir), lookAtDist + 120.0f ) )
		{
			collided = true;
			lookAtDist = dist - 120.0f;
		}
/*
		/// 갱신
		if( collided )
		{
			lookatCam->SetLookAtDistance( lookAtDist );
			lookatCam->SetTranslate( heroPos + invDir * lookAtDist );
			lookatCam->Update( accumTime );
		}
*/
		/// 시점과의 거리가 너무 짧으면 목표물을 투명 처리
	}

	/// 조명 노드들을 처리
	cSphere testSphere;
	{
		testSphere.Set( mHeroPos, 10000.0f );
		mPickedArray.Clear();

		if( Pick( &mPickedArray, testSphere, SCENENODE_LIGHT ) )
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				((cLightSceneNode*)mPickedArray[i])->Process( deltaTime, accumTime );
			}
		}
	}

/*
	/// 정적 노드들을 처리
	{
		testSphere.Set( heroPos, 10000.0f );
		mPickedArray.Clear();

		if( Pick( &mPickedArray, testSphere, SCENENODE_STATIC ) )
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				((cStaticSceneNode*)mPickedArray[i])->ProcessVisibleLevel( deltaTime, culler );
			}
		}
	}
*/

	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		cStaticSceneNode* n = mStaticArray[i];

#ifdef MAP_EDITOR
		n->SetWirframe( mStaticObjectWireframe );
#endif

//		if( n->mNeedUpdateTransform || n->mNifAniInfo != 0 )
		n->Process( deltaTime, accumTime );
	}

	/// 동적 노드들을 처리
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		mDynamicArray[i]->Process( deltaTime, accumTime );
	}

	/// 사운드 노드들을 처리
#ifdef MAP_EDITOR
	{
		testSphere.Set( mHeroPos, 100.f );
		mPickedArray.Clear();

		if( Pick( &mPickedArray, testSphere, SCENENODE_SOUND ) )
		{
			for( unsigned int i = 0, iend = mPickedArray.GetSize(); i < iend; ++i )
			{
				cSoundSceneNode* n = (cSoundSceneNode*)mPickedArray[i];

				n->SetListenerStatus( cSoundSceneNode::LISTENER_IN, testSphere );
			}
		}
		for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
		{
			mSoundArray[i]->Process( deltaTime, accumTime );
		}
	}

	/// 영역 그룹들을 처리
	{
		cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
		cAreaGroupMap::cIterator iend = mAreaGroupMap.End();

		for( ; i != iend; ++i )
		{
			i->mSecond->Process( deltaTime, testSphere );
		}
	}
#else
	deltaTime = 0;
#endif

	//*// 컬링
	NiFrustumPlanes planes( *((NiCamera*)(*cam)) );

	mAlphaTestArray.RemoveAll();
	mAlphaArray.RemoveAll();
	mSolidArray.RemoveAll();

	mNodeTree.Cull( &mSolidArray, &mAlphaArray, &mAlphaTestArray, planes, culler, SCENENODE_RENDERING );
	//*/
#ifdef MAP_EDITOR
	if( playing )
	{
		HERO->Process( deltaTime );
		HERO->GetSceneNode()->Process( deltaTime, accumTime );
		HERO->GetSceneNode()->OnVisible( culler );
		HERO->GetSceneNode()->AddToVisibleArray( &mSolidArray, &mAlphaArray, &mAlphaTestArray );
	}
#endif

	//*// 정렬
	mSolidArraySorter.Sort( *cam );
	mAlphaArraySorter.Sort( *cam );

	if( mShadowGeomArray.GetSize() > 2 )
		::Sort( mShadowGeomArray.Begin(), mShadowGeomArray.End(), cShadowGeomCompareForRendering() );
	//*/
}

void cSceneManager::Render()
{
	if( mInited == false )
		return;

	/// 솔리드 개체 렌더링
	mSolidArraySorter.Render();

	/// 그림자 배치 렌더링
	RenderShadow();

	/// 알파 객체 랜더링
	mAlphaArraySorter.Render();
/*
#ifdef MAP_EDITOR
	/// 영역 그룹들을 렌더링
	cAreaGroupMap::cIterator i = mAreaGroupMap.Begin();
	cAreaGroupMap::cIterator iend = mAreaGroupMap.End();

	for( ; i != iend; ++i )
	{
		i->mSecond->OnRender();
	}
#endif
*/
}

void cSceneManager::RenderShadow()
{
	cRenderer* renderer = RENDERSYS->GetRenderer();
	if( renderer == 0 )
	{
		assert( 0 && "null renderer" );
		return;
	}

	if( mShadowGeomArray.IsEmpty() == false )
	{
		for( unsigned int i = 0,iend = mShadowGeomArray.GetSize(); i< iend; ++i )
		{
			cShadowGeometry* geom = mShadowGeomArray[i];
			if( geom )
				geom->Draw( renderer );
		}
	}
}

void cSceneManager::AdjustAllPos( float scale )
{
	if( scale == 1.0f )
		return;

	for( unsigned int i = 0, iend = mAreaArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mAreaArray[i];
		n->SetTranslate( n->GetWorldTranslate() * scale );
		n->UpdateTransform();
	}
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mLightArray[i];
		n->SetTranslate( n->GetWorldTranslate() * scale );
		n->UpdateTransform();
	}
	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mStaticArray[i];
		n->SetTranslate( n->GetWorldTranslate() * scale );
		n->UpdateTransform();
	}
#ifdef MAP_EDITOR
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mSoundArray[i];
		n->SetTranslate( n->GetWorldTranslate() * scale );
		n->UpdateTransform();
	}
#endif

	/// 장면 트리를 초기화
	NiPoint3 center = mCenter * scale;
	float minRadius = mMinRadius * scale;
	float maxRadius = mMaxRadius * scale;

	InitTree( center, minRadius, maxRadius );
}

void cSceneManager::AdjustAllSize( float scale )
{
	if( scale == 1.0f )
		return;

	for( unsigned int i = 0, iend = mAreaArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mAreaArray[i];
		n->SetScale( n->GetWorldScale() * scale );
		n->UpdateTransform();
	}
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mLightArray[i];
		n->SetScale( n->GetWorldScale() * scale );
		n->UpdateTransform();
	}
	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mStaticArray[i];
		n->SetScale( n->GetWorldScale() * scale );
		n->UpdateTransform();
	}
#ifdef MAP_EDITOR
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mSoundArray[i];
		n->SetScale( n->GetWorldScale() * scale );
		n->UpdateTransform();
	}
#endif

	/// 장면 트리를 초기화
	float maxDist = -NI_INFINITY;

	for( unsigned int i = 0, iend = mAreaArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mAreaArray[i];
		float dist = (n->GetWorldTranslate() - mCenter).Length() + n->GetRadius();

		if( dist > maxDist )
			maxDist = dist;
	}
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mLightArray[i];
		float dist = (n->GetWorldTranslate() - mCenter).Length() + n->GetRadius();

		if( dist > maxDist )
			maxDist = dist;
	}
	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mStaticArray[i];
		float dist = (n->GetWorldTranslate() - mCenter).Length() + n->GetRadius();

		if( dist > maxDist )
			maxDist = dist;
	}
#ifdef MAP_EDITOR
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		cSceneNode* n = mSoundArray[i];
		float dist = (n->GetWorldTranslate() - mCenter).Length() + n->GetRadius();

		if( dist > maxDist )
			maxDist = dist;
	}
#endif

	InitTree( mCenter, mMinRadius, maxDist * 1.1f );
}

bool cSceneManager::Pick( tArray<cSceneNode*>* pickedArray, int mouseX, int mouseY, bool sortByDistance, eSceneNodeType type )
{
	NiPoint3 origin, dir;
	CAMERAMAN->GetRayFromWindowPoint( &origin, &dir, mouseX, mouseY );

	return Pick( pickedArray, cRay(origin, dir), NI_INFINITY, sortByDistance, type );
}

bool cSceneManager::Pick( tArray<cSceneNode*>* pickedArray, const cRay& ray, float maxDistance, bool sortByDistance, eSceneNodeType type )
{
	return mNodeTree.CollideRay( pickedArray, ray, maxDistance, sortByDistance, type );
}

bool cSceneManager::Pick( tArray<cSceneNode*>* pickedArray, const cSphere& sphere, eSceneNodeType type )
{
	return mNodeTree.CollideSphere( pickedArray, sphere, type );
}

void cSceneManager::AddNode( cSceneNode* node )
{
	assert( node );

	/// 트리에 추가
	mNodeTree.Push( node );

	/// 배열에 추가
	switch( node->GetType() )
	{
	case SCENENODE_AREA:
		node->mIndexByManager = mAreaArray.GetSize();
		mAreaArray.PushBack( (cAreaSceneNode*)node );
		break;
	case SCENENODE_LIGHT:
		node->mIndexByManager = mLightArray.GetSize();
		mLightArray.PushBack( (cLightSceneNode*)node );
		break;
#ifdef MAP_EDITOR
	case SCENENODE_SOUND:
		node->mIndexByManager = mSoundArray.GetSize();
		mSoundArray.PushBack( (cSoundSceneNode*)node );
		break;
#endif
	case SCENENODE_STATIC:
	case SCENENODE_PORTAL:
	case SCENENODE_GATHERING:
	case SCENENODE_PATTERN:
	case SCENENODE_DOOR:
		node->mIndexByManager = mStaticArray.GetSize();
		mStaticArray.PushBack( (cStaticSceneNode*)node );
		break;
	case SCENENODE_DYNAMIC:
	case SCENENODE_NPC:
	case SCENENODE_MONSTER:
		node->mIndexByManager = mDynamicArray.GetSize();
		mDynamicArray.PushBack( (cDynamicSceneNode*)node );
		break;
	}
}

void cSceneManager::DestroyAll()
{
	for( unsigned int i = 0, iend = mAreaArray.GetSize(); i < iend; ++i )
	{
		delete mAreaArray[i];
	}
	mAreaArray.Clear();

	///
	for( unsigned int i = 0, iend = mLightArray.GetSize(); i < iend; ++i )
	{
		delete mLightArray[i];
	}
	mLightArray.Clear();

	///
	for( unsigned int i = 0, iend = mStaticArray.GetSize(); i < iend; ++i )
	{
		delete mStaticArray[i];
	}
	mStaticArray.Clear();

	///
	for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
	{
		delete mDynamicArray[i];
	}
	mDynamicArray.Clear();

	///
#ifdef MAP_EDITOR
	for( unsigned int i = 0, iend = mSoundArray.GetSize(); i < iend; ++i )
	{
		delete mSoundArray[i];
	}
	mSoundArray.Clear();
#endif

	///
	mNodeTree.Clear();
}

bool cSceneManager::DestroyNode( cSceneNode* node )
{
	assert( node );

	return DestroyNode( node->mType, node->mIndexByManager );
}

bool cSceneManager::DestroyNode( eSceneNodeType type, unsigned int i )
{
	switch( type )
	{
	case SCENENODE_AREA:
		{
			if( i >= mAreaArray.GetSize() )
			{
				assert( 0 && "index out of range" );
				return false;
			}

			/// 제거
			cAreaSceneNode* n = mAreaArray[i];
			delete n;

			/// 노드 배열에서 삭제
			bool orderChanged = false;
			if( mAreaArray.PopAt( &orderChanged, i ) == false)
				return false;

			if( orderChanged )
			{
				/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
				mAreaArray[i]->mIndexByManager = i;
			}
			break;
		}
	case SCENENODE_LIGHT:
		{
			if( i >= mLightArray.GetSize() )
			{
				assert( 0 && "index out of range" );
				return false;
			}

			/// 제거
			cLightSceneNode* n = mLightArray[i];
			delete n;

			/// 노드 배열에서 삭제
			bool orderChanged = false;
			if( mLightArray.PopAt( &orderChanged, i ) == false)
				return false;

			if( orderChanged )
			{
				/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
				mLightArray[i]->mIndexByManager = i;
			}
			break;
		}
#ifdef MAP_EDITOR
	case SCENENODE_SOUND:
		{
			if( i >= mSoundArray.GetSize() )
			{
				assert( 0 && "index out of range" );
				return false;
			}

			/// 제거
			cSoundSceneNode* n = mSoundArray[i];
			delete n;

			/// 노드 배열에서 삭제
			bool orderChanged = false;
			if( mSoundArray.PopAt( &orderChanged, i ) == false)
				return false;

			if( orderChanged )
			{
				/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
				mSoundArray[i]->mIndexByManager = i;
			}
			break;
		}
#endif
	case SCENENODE_GATHERING:
	case SCENENODE_PORTAL:
	case SCENENODE_STATIC:
	case SCENENODE_PATTERN:
	case SCENENODE_DOOR:
		{
			if( i >= mStaticArray.GetSize() )
			{
				assert( 0 && "index out of range" );
				return false;
			}

			/// 제거
			cStaticSceneNode* n = mStaticArray[i];
			delete n;

			/// 노드 배열에서 삭제
			bool orderChanged = false;
			if( mStaticArray.PopAt( &orderChanged, i ) == false)
				return false;

			if( orderChanged )
			{
				/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
				mStaticArray[i]->mIndexByManager = i;
			}
			break;
		}
	case SCENENODE_NPC:
	case SCENENODE_MONSTER:
	case SCENENODE_DYNAMIC:
		{
			if( i >= mDynamicArray.GetSize() )
			{
				assert( 0 && "index out of range" );
				return false;
			}

			/// 제거
			cDynamicSceneNode* n = mDynamicArray[i];
			delete n;

			/// 노드 배열에서 삭제
			bool orderChanged = false;
			if( mDynamicArray.PopAt( &orderChanged, i ) == false)
				return false;

			if( orderChanged )
			{
				/// 해당 위치에 새로 들어온 노드의 인덱스를 재설정
				mDynamicArray[i]->mIndexByManager = i;
			}
			break;
		}
	default:
		assert( 0 );
		break;
	}
	return true;
}

cAreaSceneNode* cSceneManager::CreateArea( const cAreaSceneNodeParam& param )
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
	AddNode( node );
	return node;
}

cLightSceneNode* cSceneManager::CreateLight( const cLightSceneNodeParam& param )
{
	static int i = 0;

	/// 노드를 생성
	cLightSceneNode* node = new cLightSceneNode;

	/// 노드를 초기화
	if( param.mPathName.IsEmpty() )
		param.mPathName.Format( "light%d", i++ );

	if( node->Init( param ) == false )
	{
		delete node;
		assert( 0 && "failed to init light scene node" );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
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

		cString msg;
		msg.Format( "Failed to init static scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cNpcSceneNode* cSceneManager::CreateNPC( const cNpcSceneNodeParam& param )
{
	/// 노드를 생성
	cNpcSceneNode* node = new cNpcSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init npc scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cMonsterSceneNode* cSceneManager::CreateMonster( const cMonsterSceneNodeParam& param )
{
	/// 노드를 생성
	cMonsterSceneNode* node = new cMonsterSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init monster scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cGatheringSceneNode* cSceneManager::CreateGathering( cGatheringSceneNodeParam& param )
{
	/// 노드를 생성
	cGatheringSceneNode* node = new cGatheringSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init gathering scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cPortalSceneNode* cSceneManager::CreatePortal( cPortalSceneNodeParam& param )
{
	/// 노드를 생성
	cPortalSceneNode* node = new cPortalSceneNode;

	/// 노드를 초기화
	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init portal scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cPatternSceneNode* cSceneManager::CreatePattern( cPatternSceneNodeParam& param )
{
	cPatternSceneNode* node = new cPatternSceneNode;

	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init pattern scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}

cDoorSceneNode* cSceneManager::CreateDoor( cDoorSceneNodeParam& param )
{
	cDoorSceneNode* node = new cDoorSceneNode;

	if( node->Init( param ) == false )
	{
		delete node;
		cString msg;
		msg.Format( "Failed to init door scene node: %s", param.mPathName.Cstr() );
		AfxMessageBox( msg.Cstr() );
		return 0;
	}

	/// 노드를 추가
	AddNode( node );
	return node;
}


#ifdef MAP_EDITOR
cSoundSceneNode* cSceneManager::CreateSound( const cSoundSceneNodeParam& param )
{
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
	AddNode( node );
	return node;
}
#endif

void cSceneManager::SetAmbientLightAmbient( unsigned int i, const NiColor& color )
{
	assert( i < 2 );

	mAmbientLight[i]->SetAmbientColor( color );

	if( i == 1 )
	{
#ifdef MAP_EDITOR
		HERO->GetSceneNode()->SetAmbientLightAmbient( color );
#endif

		for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
		{
			cDynamicSceneNode* n = mDynamicArray[i];

			n->SetAmbientLightAmbient( color );
		}
	}
}

void cSceneManager::SetAmbientLightDiffuse( unsigned int i, const NiColor& color )
{
	assert( i < 2 );

	mAmbientLight[i]->SetDiffuseColor( color );

	if( i == 1 )
	{
#ifdef MAP_EDITOR
		HERO->GetSceneNode()->SetAmbientLightDiffuse( color );
#endif

		for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
		{
			cDynamicSceneNode* n = mDynamicArray[i];

			n->SetAmbientLightDiffuse( color );
		}
	}
}

void cSceneManager::SetAmbientLightDimmer( unsigned int i, float dimmer )
{
	assert( i < 2 );

	mAmbientLight[i]->SetDimmer( dimmer );

	if( i == 1 )
	{
#ifdef MAP_EDITOR
		HERO->GetSceneNode()->SetAmbientLightDimmer( dimmer );
#endif

		for( unsigned int i = 0, iend = mDynamicArray.GetSize(); i < iend; ++i )
		{
			cDynamicSceneNode* n = mDynamicArray[i];

			n->SetAmbientLightDimmer( dimmer );
		}
	}
}

void cSceneManager::SetDirLightAmbient( unsigned int i, const NiColor& color )
{
	assert( i < 2 );

	mDirLight[i]->SetAmbientColor( color );
}

void cSceneManager::SetDirLightDiffuse( unsigned int i, const NiColor& color )
{
	assert( i < 2 );

	mDirLight[i]->SetDiffuseColor( color );
}

void cSceneManager::SetDirLightDimmer( unsigned int i, float dimmer )
{
	assert( i < 2 );

	mDirLight[i]->SetDimmer( dimmer );
}

void cSceneManager::SetDirLightRotate( unsigned int i, float xangle, float yangle, float zangle )
{
	assert( i < 2 );

	NiMatrix3 r;
	r.FromEulerAnglesXYZ( xangle, yangle, zangle );

	mDirLight[i]->SetRotate( r );
	mDirLight[i]->Update( 0.0f );
}

void cSceneManager::SetFog( bool enabled, const NiColor& color, float depth )
{
	mFogProp->SetFog( enabled );
	mFogProp->SetFogColor( color );
	mFogProp->SetDepth( depth );
}

void cSceneManager::SetFogColor( const NiColor& color )
{
	mFogProp->SetFogColor( color );
}

void cSceneManager::SetFogDepth( float depth )
{
	mFogProp->SetDepth( depth );
}

unsigned int cSceneManager::CalcVisibleLevel( cStaticSceneNode* n )
{
	if( n == 0 )
	{
		assert(0);
		return 0;
	}

	float sqrDist = (mHeroPos - n->GetCenter()).SqrLength();

	if( sqrDist > mSqaredLevelDistance2 )
		return 3;
	if( sqrDist > mSqaredLevelDistance1 )
		return 2;
	if( sqrDist > mSqaredLevelDistance0 )
		return 1;
	return 0;
}

