#include "stdafx.h"
#include "StaticSceneNode.h"

#include "ResourceManager.h"
#include "CameraManager.h"
#include "SceneManager.h"

#include "NifAnimationInfo.h"

cStaticSceneNode::cStaticSceneNode( eType type )
: cSceneNode( type )
, mVisibleLevel( 0 )
, mOccludeLevel( 1 )
, mPickedByCamera( false )
{
	mNifAniInfo = new cNifAnimationInfo;
}

cStaticSceneNode::~cStaticSceneNode()
{
	ClearCollectInfo();

	SAFE_DELETE( mNifAniInfo );
}

void cStaticSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
#ifndef _MODELVIEW
	ProcessVisibleLevel( deltaTime );
#endif
	if( mCheckFrustum == false )
		return;

	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}
	else
		return;

	cSceneNode::OnProcess( deltaTime, accumTime );
}

void cStaticSceneNode::ProcessVisibleLevel( unsigned long deltaTime )
{
	if( mVisibleLevel < 3 )
	{
		/// 정해진 거리만큼 거르는 체크
		if( mVisibleLevel <  SCENEMAN->CalcVisibleLevel( this ) )
		{
			SetAlphaBlended( 0.0f );
			if( mAlpha == mTargetAlpha )
				/// no frustum check
				mCheckFrustum = false;
			else
				/// frustum check
				mCheckFrustum = true;
		}
		else
		{
			if( mOccludeLevel == 1 && mPickedByCamera )
			{
				mPickedByCamera = false;
				SetAlphaBlended( 0.4f );
			}
			else
			{
				SetAlphaBlended( 1.0f );
			}

			/// frustum check
			mCheckFrustum = true;
		}
	}

	/// 알파를 처리
	ProcessAlpha( deltaTime );
}

bool cStaticSceneNode::Init( const cStaticSceneNodeParam& param )
{
	/// 파일을 로딩
	if( RESOURCEMAN->LoadMapNIF( param.mPathName.Cstr() ) == false )
	{
		assert( 0 && "failed to load nif file" );
		return false;
	}

	/// 리소스 관리자로부터 엔진 노드를 복제
	NiNode* n = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	/// 상위 초기화
	mSceneNiNode = n;

	/// 가시 레벨을 설정
	mVisibleLevel = param.mVisibleLevel;

	/// 차폐 레벨을 설정
	mOccludeLevel = param.mOccludeLevel;

	/// 알파 속성을 설정
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)n->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		n->AttachProperty( alphaProp );
	}

	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// 재질을 설정
	if( param.mMaterialApplied )
		SetMaterial( param.mMaterial );

	/// 안개 속성을 부착
//	if( param.mFogApplied )
//		n->AttachProperty( FOGAGENT->GetFogProperty() );

	if( param.mNoAttachLight == false )
	{
		n->AttachEffect( SCENEMAN->GetAmbientLight() );
//		/// 조명을 부착
//		n->AttachEffect( LIGHTAGENT->GetSceneAmbientLight( 0 ) );
//		n->AttachEffect( LIGHTAGENT->GetSceneDirLight( 0 ) );
	}

	/// 엔진 노드를 설정
	bool selectiveUpdate = true;
	bool rigid = false;
	n->SetSelectiveUpdateFlags( selectiveUpdate, mNifAniInfo!=0, rigid );

	/// 엔진 노드를 처리
	n->UpdateProperties();
	n->UpdateEffects();
	n->Update( 0.0f );
	n->UpdateNodeBound();

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );

	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( true );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	if( mNifAniInfo && param.mDisableAnimation == true )
		mNifAniInfo->Stop();

	return true;
}

bool cStaticSceneNode::Init( NiNode* node )
{

	/// 조명을 부착
	//	node->AttachEffect( SCENEMAN->GetAmbientLight() );

	/// 엔진 노드를 설정
	node->SetSelectiveUpdate( true );
	node->SetSelectiveUpdateTransforms( false );
	node->SetSelectiveUpdatePropertyControllers( true );
	node->SetSelectiveUpdateRigid( true );

	/// 엔진 노드를 처리
	node->UpdateProperties();
	node->UpdateEffects();
	node->Update( 0.f );
	node->UpdateNodeBound();

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( node );

	mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
	mNifAniInfo->SetLooping( true );

	/// 상위 초기화
	mSceneNiNode = node;

	mBoundSphere.Set( GetCenter(), GetRadius() );

	return true;
}

void cStaticSceneNode::SetMaterial( const cMaterialData& material )
{
	cMatPropList::cIterator i = mMatPropList.Begin();
	cMatPropList::cIterator end = mMatPropList.End();

	for( ; i != end; ++i )
	{
		NiMaterialProperty* matProp = (NiMaterialProperty*)*i;

		matProp->SetAmbientColor( material.mAmbient );
		matProp->SetDiffuseColor( material.mDiffuse );
		matProp->SetSpecularColor( material.mSpecular );
		matProp->SetEmittance( material.mEmissive );
		matProp->SetShineness( material.mShininess );
		matProp->SetAlpha( material.mAlpha );
	}

	mMaterial = material;
}

//////////////////////////////////////////////////////////////////////////

cManagedStaticSceneNode::cManagedStaticSceneNode( eType type )
: cStaticSceneNode( type )
{
	mIsMouseOver = false;
}

cManagedStaticSceneNode::~cManagedStaticSceneNode()
{

}