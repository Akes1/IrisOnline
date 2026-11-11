#include "StdAfx.h"
#include "effectscenenode.h"
#include "NifAnimationInfo.h"

#include "ResourceManager.h"
#include "DynamicSceneNode.h"
#include "SceneManager.h"
#include "CameraManager.h"

#include "Application.h"

cEffectSceneNode::cEffectSceneNode( eType type )
: cSceneNode( type )
, mpFollowSceneNode(0)
, mRemoved(false)
{
	mFollowSceneNodeIdx = (unsigned long)-1;
	mDummyIdx = (unsigned int)-1;

	mNifAniInfo = new cNifAnimationInfo;
	assert(mNifAniInfo);

	mIsDamage = false;
	mIsDistCheck = true;
}

cEffectSceneNode::~cEffectSceneNode()
{
	ClearCollectInfo();

	if( mSceneNiNode )
	{
		NiNode* parent = mSceneNiNode->GetParent();
		if( parent != 0 )
		{
			parent->DetachChild( mSceneNiNode );
		}
	}

	try {
		RemoveSelf();
		SAFE_DELETE(mNifAniInfo);
	} catch ( ... ) {
		GameErrorLog( "cEffectSceneNode::~cEffectSceneNode(Delete NifAniInfo)" );
		throw;
	}
}

bool cEffectSceneNode::InitSelfNode( const cString& pathName, bool distCheck, NiTransform& trans, bool bLoop )
{
	if( RESOURCEMAN->LoadNIF( pathName ) == false )
	{
		assert( 0 && "failed to load nif file" );
		return false;
	}


	NiNode* n = RESOURCEMAN->CloneObjectByName( pathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	mIsDistCheck = distCheck;

	NiTransform kWorld = trans * n->GetLocalTransform();
	n->SetTranslate( kWorld.m_Translate );
	n->SetRotate( kWorld.m_Rotate );
	n->SetScale( kWorld.m_fScale );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	mSceneNiNode = n;
	n->SetName( "EffectSceneNode" );

	/// 
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	NiFogProperty* pFog = NiNew NiFogProperty;
	pFog->SetFog( false );
	n->AttachProperty( pFog );

	/// 엔진 노드를 설정
	bool selectiveUpdate = true;
	bool rigid = false;
	n->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	n->UpdateProperties();
	n->UpdateEffects();

	/// 엔진 노드를 처리
	NiMeshUpdateProcess kUpdateProcess;
	kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	n->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	n->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( bLoop );

		mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

/// param을 생성하는 경우 dummy를 포함해야 한다.
bool cEffectSceneNode::Init( const cEffectSceneNodeParam& param )
{
	/// 파일을 로딩
	if( RESOURCEMAN->LoadNIF( param.mPathName ) == false )
	{
		assert( 0 && "failed to load nif file" );
		return false;
	}

	if( param.mpFollowSceneNode == 0 )
	{
		assert(0);
		return false;
	}

	/// 리소스 관리자로부터 엔진 노드를 복제
	NiNode* n = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	n->SetTranslate( param.mTranslate );
	n->SetRotate( param.mRotate );
	n->SetScale( param.mScale );

	mIsDistCheck = param.mIsDistCheck;

	NiNode* pDummy = param.mpFollowSceneNode->GetDummyObject(param.mDummyIdx);
	if( pDummy == 0 )
	{
		if( param.mDummyIdx == eLINK_RWEAPON ) /// right weapon dummy
		{
			/// right hand link
			pDummy = param.mpFollowSceneNode->GetDummyObject( eLINK_RHAND );
		}
		else if( param.mDummyIdx == eLINK_LWEAPON ) /// left weapon dummy
		{
			/// left hand link
			pDummy = param.mpFollowSceneNode->GetDummyObject( eLINK_LHAND );
		}
		else
		{
			assert(0);
			return false;
		}
	}

	if( param.mFollowing == false )
	{
		if( pDummy )
		{
			NiTransform kWorld =  pDummy->GetWorldTransform() * n->GetLocalTransform();
			n->SetScale( kWorld.m_fScale );

			NiPoint3 trans = pDummy->GetWorldTranslate();
			n->SetTranslate( n->GetTranslate() + trans );
		}
	}
	else
	{
		mpFollowSceneNode = param.mpFollowSceneNode;
		mFollowSceneNodeIdx = mpFollowSceneNode->GetIndexByManger();
		mDummyIdx = param.mDummyIdx;
	}

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	mSceneNiNode = n;
	n->SetName("EffectSceneNode");

	/// 
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	NiFogProperty* pFog = NiNew NiFogProperty;
	pFog->SetFog( false );
	n->AttachProperty( pFog );

	if( param.mFollowing == true )
	{
		if( pDummy )
		{
			pDummy->AttachChild( n );
			n->SetScale( param.mScale );
		}
		else
		{
			assert(0);
			return false;
		}
	}
    
	/// 엔진 노드를 설정
	bool selectiveUpdate = true;
	bool rigid = false;
	n->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	n->UpdateProperties();
	n->UpdateEffects();

	/// 엔진 노드를 처리
	NiMeshUpdateProcess kUpdateProcess;
	kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	n->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	n->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( param.mLooping );

		mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

bool cEffectSceneNode::InitDamageEffect( const cEffectSceneNodeParam& param, bool IsMiss, bool zFalse )
{
	/// 파일을 로딩
	if( RESOURCEMAN->LoadNIF( param.mPathName ) == false )
	{
		assert( 0 );
		return false;
	}

	if( param.mpFollowSceneNode == 0 )
	{
		assert(0);
		return false;
	}
	NiNode* pDummy = param.mpFollowSceneNode->GetDummyObject(param.mDummyIdx);
	if( pDummy == 0 )
	{
		assert(0);
		return false;
	}

	///
	NiNodePtr des = 0;
	if( IsMiss == true )
	{
		/// 리소스 관리자로부터 엔진 노드를 복제
		des = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
		if( des == 0 )
		{
			assert( 0 && "failed to clone node from resource manager" );
			return false;
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// 공유 GeomData를 버리고 새로 갱신한다.

		des = RESOURCEMAN->DeepCopyObjectByName( param.mPathName.Cstr() );
		if( des == 0 )
		{
			assert(0);
			return false;
		}

		//NiNode* src = RESOURCEMAN->GetObjectByName( param.mPathName.Cstr() );
		//if( src == 0 )
		//	return false;

		//des = CopyDamageMesh(src );
		//if( des == 0 )
		//{
		//	assert(0);
		//	return false ;
		//}
		//////////////////////////////////////////////////////////////////////////
	}

	des->SetTranslate( param.mTranslate );
	des->SetRotate( param.mRotate );
	des->SetScale( param.mScale );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	mIsDistCheck = param.mIsDistCheck;

	if( param.mFollowing == false )
	{
		NiPoint3 trans = pDummy->GetWorldTranslate();
		des->SetTranslate( des->GetTranslate() + trans );
	}
	else
	{
		mpFollowSceneNode = param.mpFollowSceneNode;
		mFollowSceneNodeIdx = mpFollowSceneNode->GetIndexByManger();
		mDummyIdx = param.mDummyIdx;
	}

	mSceneNiNode = des;
	des->SetName("EffectSceneNode");

	/// 
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );


	NiFogProperty* pFog = NiNew NiFogProperty;
	pFog->SetFog( false );
	des->AttachProperty( pFog );

	if( zFalse )
	{
		NiZBufferProperty* zProp = (NiZBufferProperty*)des->GetProperty( NiProperty::ZBUFFER );
		if( zProp == 0 )
		{
			/// 알파 속성이 없으면 생성
			zProp = NiNew NiZBufferProperty;
			des->AttachProperty( zProp );
		}

		DisableZBuffer( des );
		mIsDamage = true;
	}

	if( param.mFollowing == true && pDummy )
		pDummy->AttachChild( des );

	/// 엔진 노드를 설정
	bool selectiveUpdate = true;
	bool rigid = false;
	des->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	des->UpdateProperties();
	des->UpdateEffects();

	/// 엔진 노드를 처리
	NiMeshUpdateProcess kUpdateProcess;
	kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	des->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	des->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( des );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( param.mLooping );

		mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

void cEffectSceneNode::CollectNodeInfo( NiAVObject* obj, bool& collectGeom, bool& collectProperty )
{
	if( obj == 0 )
	{
		assert( obj );
		return;
	}

	/// billboard Info
	{
		if( collectGeom )
		{
			if( NiIsKindOf(NiBillboardNode, obj) )
			{
				mBillboardList.PushBack( (NiBillboardNode*)obj );
			}
		}
	}

	/// property
	{
		NiFogProperty* fog = (NiFogProperty*)obj->GetProperty( NiProperty::FOG );
		if( fog )
			obj->DetachProperty( fog );

		if( collectProperty == true )
		{
			NiMaterialProperty* matProp = (NiMaterialProperty*)obj->GetProperty( NiProperty::MATERIAL );
			if( matProp )
			{
				mMatPropList.PushBack( matProp );
			}

			NiVertexColorProperty* vertProp = (NiVertexColorProperty*)obj->GetProperty( NiProperty::VERTEX_COLOR );
			if( vertProp )
			{
				NiVertexColorProperty::SourceVertexMode sourceMode = vertProp->GetSourceMode();
				NiVertexColorProperty::LightingMode lightMode = vertProp->GetLightingMode();

				mVertexColorDataList.PushBack( new cVertexColorData( sourceMode, lightMode, vertProp ) );
			}

			NiAlphaProperty* alphaProp = (NiAlphaProperty*)obj->GetProperty( NiProperty::ALPHA );
			if( alphaProp )
			{
				bool blendEnabled = alphaProp->GetAlphaBlending();
				bool testEnabled = alphaProp->GetAlphaTesting();
				NiAlphaProperty::TestFunction testFunc = alphaProp->GetTestMode();
				unsigned char testRef = alphaProp->GetTestRef();

				mAlphaDataList.PushBack( new cAlphaData( blendEnabled, testEnabled, testFunc, testRef, alphaProp ) );
			}
		}
	}
}


void cEffectSceneNode::RemoveSelf()
{
	assert( mType == cSceneNode::eEFFECT );

	if( mRemoved == true )
		return;

	if( mIndexByManager == (unsigned int)-1 )
		return;

	if( mpFollowSceneNode )
	{
		mpFollowSceneNode->UnLinkEffect( this );
		mFollowSceneNodeIdx = (unsigned long)-1;
		mpFollowSceneNode = 0;
	}

	if( mFollowSceneNodeIdx != (unsigned long)-1 )
	{
		cDynamicSceneNode* node = SCENEMAN->GetDynamicSceneNode( mFollowSceneNodeIdx );
		if( node )
		{
			assert( node == mpFollowSceneNode );
			if( node != mpFollowSceneNode )
				GameErrorLog( "cEffectSceneNode::RemoveSelf : node == mpFollowSceneNode" );

			node->UnLinkEffect( this );
		}
	}

	mRemoved = true;
	mpFollowSceneNode = 0;
	mFollowSceneNodeIdx = (unsigned long)-1;
	mDummyIdx = (unsigned int)-1;

	SCENEMAN->AddDeleteEffectNode( this );

	if( mNifAniInfo )
		mNifAniInfo->Stop();
}

void cEffectSceneNode::Remove()
{
	assert( mType == cSceneNode::eEFFECT );

	if( mRemoved == true )
		return;

	if( mpFollowSceneNode )
	{
		mpFollowSceneNode->UnLinkEffect( this );
		mFollowSceneNodeIdx = (unsigned long)-1;
		mpFollowSceneNode = 0;
	}

	if( mFollowSceneNodeIdx != (unsigned long)-1 )
	{
		cDynamicSceneNode* node = SCENEMAN->GetDynamicSceneNode( mFollowSceneNodeIdx );
		if( node )
		{
			assert( node == mpFollowSceneNode );
			if( node != mpFollowSceneNode )
				GameErrorLog( "cEffectSceneNode::Remove : node == mpFollowSceneNode" );

			node->UnLinkEffect( this );
		}
	}

	mRemoved = true;
	mpFollowSceneNode = 0;
	mFollowSceneNodeIdx = (unsigned long)-1;
	mDummyIdx = (unsigned int)-1;

	SCENEMAN->AddDeleteEffectNode( this );

	if( mNifAniInfo )
		mNifAniInfo->Stop();
}

void cEffectSceneNode::ParentNodeRemove()
{
	assert( mType == cSceneNode::eEFFECT );

	if( mRemoved == true )
		return;

	mRemoved = true;
	mpFollowSceneNode = 0;
	mFollowSceneNodeIdx = (unsigned long)-1;
	mDummyIdx = (unsigned int)-1;

	SCENEMAN->AddDeleteEffectNode( this );

	if( mNifAniInfo )
		mNifAniInfo->Stop();
}

void cEffectSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mRemoved == true )
		return;

	if( mDummyIdx == (unsigned int)-1 )
	{
		if( GetNiNode() )
			GetNiNode()->Update( accumTime * 0.001f );
	}

	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		unsigned char state = mNifAniInfo->Update( deltaTime, accumTime );
		if( state == cNifAnimationInfo::eEvent_End )
		{
			RemoveSelf();
			return;
		}
	}

	/// Alpha Fade In/Out 처리
	ProcessAlpha( deltaTime );

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	if( mpFollowSceneNode )
	{
		mCheckFrustum = mpFollowSceneNode->IsEnableFrustumFlag();
	}
	else
	{
		if( SCENEMAN->CalcDynamicVisibleLevel( this ) == true )
			SetAlphaBlended( 1.0f );
		else
			SetAlphaBlended( 0.0f );
	}
}

void cEffectSceneNode::SetLoopFlag( bool flag )
{
	if( mNifAniInfo )
	{
		if( mNifAniInfo->IsEnable() )
			mNifAniInfo->SetLooping( flag );
		else
			RemoveSelf();
	}
	else
	{
		RemoveSelf();
	}
}

bool cEffectSceneNode::GetLoopFlag()
{
	if( mNifAniInfo )
		return mNifAniInfo->IsLooping();

	return false;
}

bool cEffectSceneNode::IsViewNode()
{
	if( mIsDistCheck )
	{
		NiPoint3 camPos = CAMERAMAN->GetCurrentCameraPos();
		if( (camPos - GetWorldTranslate()).Length() > 5000.0f )
			return false;
	}
	
	return cSceneNode::IsViewNode();
}

void cEffectSceneNode::DisableZBuffer( NiAVObject* obj )
{
	NiZBufferProperty* prop = (NiZBufferProperty*)obj->GetProperty( NiProperty::ZBUFFER );
	if( prop )
	{
		prop->SetZBufferTest( false );
		prop->SetZBufferWrite( false );
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
