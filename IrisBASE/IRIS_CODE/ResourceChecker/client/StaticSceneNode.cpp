#include "stdafx.h"
#include "StaticSceneNode.h"

#include "ResourceManager.h"
// #include "CameraManager.h"
#include "SceneManager.h"
// #include "LightAgent.h"
// #include "FogAgent.h"

#include "NifAnimationInfo.h"

#include "Application.h"

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
	ProcessVisibleLevel( deltaTime );

	if( mNifAniInfo )
		mNeedUpdateTransform = true;
	else
		return;


	/// 반복 동작을 가진 경우 엔진 노드를 처리
	cSceneNode::OnProcess( deltaTime, accumTime );

	mNifAniInfo->Update( deltaTime, accumTime );
}

void cStaticSceneNode::ProcessVisibleLevel( unsigned long deltaTime )
{
// 	if( mVisibleLevel < 3 )
// 	{
// 		/// 정해진 거리만큼 거르는 체크
// 		if( mVisibleLevel <  SCENEMAN->CalcVisibleLevel( this ) )
// 		{
// 			SetAlphaBlended( 0.0f );
// 			if( mAlpha == mTargetAlpha )
// 				/// no frustum check
// 				mCheckFrustum = false;
// 			else
// 				/// frustum check
// 				mCheckFrustum = true;
// 		}
// 		else
// 		{
// 			if( mOccludeLevel == 1 && mPickedByCamera )
// 			{
// 				mPickedByCamera = false;
// 				SetAlphaBlended( 0.4f );
// 			}
// 			else
// 			{
// 				SetAlphaBlended( 1.0f );
// 			}
// 
// 			/// frustum check
// 			mCheckFrustum = true;
// 		}
// 	}
// 
// 	/// 알파를 처리
// 	ProcessAlpha( deltaTime );
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

	/// 변환 행렬을 설정
	GetNiObj()->SetTranslate( param.mTranslate );
	GetNiObj()->SetRotate( param.mRotate );
	GetNiObj()->SetScale( param.mScale );

	/// 갱신 여부를 설정
	mNeedUpdateTransform = true;

	/// 픽킹 방식을 설정
	mPick.SetQueryType( param.mQueryType );
	mPick.SetSortType( param.mPickSort );
	mPick.SetIntersectType( param.mPickIntersect );
	mPick.SetCoordinateType( param.mPickCoordinate );
	mPick.SetFrontOnly( param.mPickFrontOnly );
	mPick.SetReturnTexture( param.mPickReturnTexture );
	mPick.SetReturnNormal( param.mPickReturnNormal );
	mPick.SetReturnSmoothNormal( false );
	mPick.SetReturnColor( param.mPickReturnColor );
	mPick.SetTarget( GetNiObj() );
	mPick.SetPickObjectPolicy(NiNew SimpleBoxPickObjectPolicy(false));

	/// 알파 속성을 설정
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)n->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		n->AttachProperty( alphaProp );
	}

// 	/// 안개 속성을 부착
// 	if( param.mFogApplied )
// 		n->AttachProperty( FOGAGENT->GetFogProperty() );

// 	if( param.mNoAttachLight == false )
// 	{
// 		/// 조명을 부착
// 		n->AttachEffect( LIGHTAGENT->GetSceneAmbientLight( 0 ) );
// 		n->AttachEffect( LIGHTAGENT->GetSceneDirLight( 0 ) );
// 	}

	/// 기하 오브젝트들을 수집
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	/// 재질을 설정
	if( param.mMaterialApplied )
	{
		SetMaterial( param.mMaterial );
	}

	/// 엔진 노드를 설정
	bool selectiveUpdate = true;
	bool rigid = false;
	n->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	n->UpdateProperties();
	n->UpdateEffects();

	/// 엔진 노드를 처리
	NiMeshUpdateProcess kUpdateProcess;
	n->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	n->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	///// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( true );

//		mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
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

bool cStaticSceneNode::UpdateCameraPickInfo()
{
	mPickedByCamera = true;

	if( mOccludeLevel == 2 )
		return true;
	else
		return false;
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
