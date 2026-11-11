#include "stdafx.h"
#include "StaticSceneNode.h"

#include "SceneManager.h"
#include "ResourceManager.h"

#include "NifAnimationInfo.h"

cStaticSceneNode::cStaticSceneNode( eSceneNodeType type )
: cSceneNode( type )
, mNumVerts( 0 )
, mVisibleLevel( 0 )
, mOccludeLevel( 1 )
, mFogApplied( false )
, mMaterialApplied( false )
, mPickedByCamera( false )
{
	mNifAniInfo = new cNifAnimationInfo;
}

cStaticSceneNode::~cStaticSceneNode()
{
	if( mNifAniInfo )
	{
		delete mNifAniInfo;
		mNifAniInfo = 0;
	}
}

void cStaticSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	ProcessVisibleLevel( deltaTime );
	if( mCheckFrustum == false )
		return;

	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}

#ifdef REGEN_TOOL
	SetAlphaBlended( 0.4f );
#endif

	cSceneNode::Process( deltaTime, accumTime );
}

void cStaticSceneNode::ProcessVisibleLevel( unsigned long deltaTime )
{
	/// 가시 레벨을 처리
#ifdef MAP_EDITOR
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
			if( mOccludeLevel ==1 && mPickedByCamera )
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

		/// 알파를 처리
		ProcessAlpha( deltaTime );
	}
#else
	ProcessAlpha( deltaTime );
#endif
}

bool cStaticSceneNode::Save( cFileSaver& saver )
{
	/// 상위 정보를 저장
	if( cSceneNode::Save( saver ) == false ) 
		return false;

	/// 안개 적용 여부를 저장
	saver.WriteUnsignedInt( mFogApplied ? 1 : 0 );

	/// 재질 적용 여부를 저장
	saver.WriteUnsignedInt( mMaterialApplied ? 1 : 0 );

	/// 재질 속성을 저장
	saver.Write( &mMaterial, sizeof(cMaterialData_Ver9) );

	/// 기타 속성을 저장
	saver.WriteUnsignedInt( mVisibleLevel );
	saver.WriteUnsignedInt( mOccludeLevel );

	return true;
}

bool cStaticSceneNode::Init( cStaticSceneNodeParam& param )
{
	/// 파일을 로딩
	if( RESOURCEMAN->LoadNIF( param.mPathName ) == false )
		return false;

	/// 리소스 관리자로부터 엔진 노드를 복제
	NiNode* n = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	NiTransform world = n->GetWorldTransform();
	if( param.mTranslate == NiPoint3::ZERO && world.m_Translate == NiPoint3::ZERO )
		param.mTranslate = n->GetTranslate();
	if( param.mRotate == NiMatrix3::IDENTITY && world.m_Rotate == NiMatrix3::IDENTITY )
		param.mRotate = n->GetRotate();
	if( param.mScale == 1.0f && world.m_fScale == 1.0f )
		param.mScale = n->GetScale();

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
#ifdef MAP_EDITOR

		mWireProp = NiNew NiWireframeProperty;
		mWireProp->SetWireframe( false );

		n->AttachProperty( mWireProp );
#endif

		/// 알파 속성을 생성
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
	mMaterialApplied = param.mMaterialApplied;
	if( mMaterialApplied )
		SetMaterial( param.mMaterial );

	/// 안개 속성을 부착
	mFogApplied = param.mFogApplied;
	if( mFogApplied )
	{
		n->AttachProperty( SCENEMAN->GetFogProperty() );
	}

	/// 조명을 부착
	n->AttachEffect( SCENEMAN->GetAmbientLight( 0 ) );
	n->AttachEffect( SCENEMAN->GetDirLight( 0 ) );

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );

	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( true );

		NiTimeController::StartAnimations(n, 0.0f);
	}
	else
	{
		/// 애니 정보가 없다면 해제
		delete mNifAniInfo;
		mNifAniInfo = 0;
	}

	/// 갱신
	bool selectiveUpdate = true;
	bool rigid = false;
	n->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	/// 엔진 노드를 처리
	n->UpdateProperties();
	n->UpdateEffects();
	n->Update( 0.0f );
	n->UpdateNodeBound();

	/// 삼각형 수를 계산
	cRenderObjList::cIterator i = mRenderObjList.Begin();
	cRenderObjList::cIterator iend = mRenderObjList.End();

	for( ; i != iend; ++i )
	{
		NiMesh* mesh = NiDynamicCast( NiMesh, i->mFirst );
		if( mesh )
		{
			for( unsigned int ct = 0; ct< mesh->GetSubmeshCount(); ct++ )
				mNumVerts += mesh->GetCount( ct, NiCommonSemantics::POSITION() );
		}
	}
	return true;
}

void cStaticSceneNode::SetMaterial( const cMaterialData& material )
{
	mMaterial = material;

	if( mMaterialApplied )
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();

		for( ; i != iend; ++i )
		{
			cMaterialData* data = (cMaterialData*)*i;
			NiMaterialProperty* matProp = data->mProp;

			matProp->SetAmbientColor( material.mAmbient );
			matProp->SetDiffuseColor( material.mDiffuse );
			matProp->SetSpecularColor( material.mSpecular );
			matProp->SetEmittance( material.mEmissive );
			matProp->SetShineness( material.mShininess );
			matProp->SetAlpha( material.mAlpha );
		}
	}
}

void cStaticSceneNode::SetAmbient( const NiColor& color )
{
	mMaterial.mAmbient = color;

	if( mMaterialApplied )
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();

		for( ; i != iend; ++i )
		{
			cMaterialData* data = (cMaterialData*)*i;
			NiMaterialProperty* matProp = data->mProp;
			matProp->SetAmbientColor( color );
		}
	}
}

void cStaticSceneNode::SetDiffuse( const NiColor& color )
{
	mMaterial.mDiffuse = color;

	if( mMaterialApplied )
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();

		for( ; i != iend; ++i )
		{
			cMaterialData* data = (cMaterialData*)*i;
			NiMaterialProperty* matProp = data->mProp;
			matProp->SetDiffuseColor( color );
		}
	}
}

void cStaticSceneNode::SetEmissive( const NiColor& color )
{
	mMaterial.mEmissive = color;

	if( mMaterialApplied )
	{
		cMaterialDataList::cIterator i = mMatDataList.Begin();
		cMaterialDataList::cIterator iend = mMatDataList.End();

		for( ; i != iend; ++i )
		{
			cMaterialData* data = (cMaterialData*)*i;
			NiMaterialProperty* matProp = data->mProp;
			matProp->SetEmittance( color );
		}
	}
}

void cStaticSceneNode::SetMaterialApplied( bool matApplied )
{
//	if( mMaterialApplied == matApplied )
//	{
//		NiMessageBox( _T("Bug Check!!"), _T("Error") );
//	}

	mMaterialApplied = matApplied;

	if( mMaterialApplied )
		SetMaterial( mMaterial );
	else
		RestoreMaterial();
}

void cStaticSceneNode::SetVisibleLevel( unsigned int level )
{
	mVisibleLevel = level;
}

void cStaticSceneNode::SetOccludeLevel( unsigned int level )
{
	mOccludeLevel = level;
}

void cStaticSceneNode::SetFogApplied( bool fogApplied )
{
	if( mFogApplied == fogApplied )
		return;

	if( fogApplied )
	{
		mFogApplied = true;
		GetNiNode()->AttachProperty( SCENEMAN->GetFogProperty() );
	}
	else
	{
		mFogApplied = false;
		GetNiNode()->DetachProperty( SCENEMAN->GetFogProperty() );
	}
	GetNiNode()->UpdateProperties();
}

void cStaticSceneNode::RestoreMaterial()
{
	cMaterialDataList::cIterator i = mMatDataList.Begin();
	cMaterialDataList::cIterator iend = mMatDataList.End();

	for( ; i != iend; ++i )	
	{
		cMaterialData* data = (cMaterialData*)*i;
		NiMaterialProperty* matProp = data->mProp;
		if( matProp )
		{
			matProp->SetAmbientColor( data->mAmbient );
			matProp->SetDiffuseColor( data->mDiffuse );
			matProp->SetSpecularColor( data->mSpecular );
			matProp->SetEmittance( data->mEmissive );
			matProp->SetShineness( data->mShininess );
			matProp->SetAlpha( data->mAlpha );
		}
	}
}

bool cStaticSceneNode::IsAnimated() const
{
	if( mNifAniInfo )
		return mNifAniInfo->GetNumberOfControllers() > 0;

	return false;
}

/*
unsigned int cStaticSceneNode::CalcVisibleLevel()
{
	float sqrDist = (mHeroPos - GetCenter()).SqrLength();

	if( sqrDist > mSqaredLevelDistance2 )
		return 3;
	if( sqrDist > mSqaredLevelDistance1 )
		return 2;
	if( sqrDist > mSqaredLevelDistance0 )
		return 1;
	return 0;
}
*/