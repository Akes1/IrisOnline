#include "stdafx.h"
#include "DynamicSceneNode.h"

#include "SceneTree.h"
#include "SceneManager.h"

#include "ResourceManager.h"
#include "RenderSystem.h"
#include "CameraManager.h"
#include "SoundSystem.h"

#include "EffectSceneNode.h"
#include "SoundSceneNode.h"
#include "TrailGeometry.h"

#include "ObjectManager.h"

#include "Application.h"


cDynamicSceneNode::cDynamicSceneNode( eType type ) 
: cSceneNode( type )
, mTargetAnimation( UINT_MAX )
, mTargetAnimationEndTime(0)
, mTargetAnimationScaleEndTime(0)
, mpRandomIdle( 0 )
, mpModel(0)
, mpLinkInfo(0)
, mpActorManager(0)
, mNeedUpdateShadow( false )
{
	mpHead = 0;
	mpFoot = 0;
	mHeadDist = 5.0f;

	//	mpMorpherController = 0;
	//	mStopMorpherProcess = false;

	mScaleAccumTime = 0;
	mLastTime = ULONG_MAX;
	mScaleFactor = 1.0f;

	mVisibleCamLength = 0.0f;

	mUpdateGeomInfo = false;

	mHideEar = false;

	mSetVisibleArray = false;

	mDisplayObj = 0;
}

cDynamicSceneNode::~cDynamicSceneNode()
{
	ClearCollectInfo();

	/// 등록된 이펙트가 있다면 모두 제거 한다.
	ClearEffect();
	ClearSound();

	NiTListIterator pos = mRibbonList.GetHeadPos();
	while( pos )
	{
		sTrailInfo* p = mRibbonList.GetNext(pos);
		SAFE_NIDELETE( p );
	}
	mRibbonList.RemoveAll();

	if( mpLinkInfo )
	{
		sLinkInfo* info = 0;
		for( unsigned int i=0; i<mpLinkInfo->GetSize(); ++i )
		{
			info = mpLinkInfo->GetAt( i );
			if( info )
			{
				NiDelete info;
			}
		}
		mpLinkInfo->RemoveAll();
	}

	SAFE_NIDELETE( mpLinkInfo );
	SAFE_NIDELETE( mpRandomIdle );

	SAFE_NIDELETE(mpActorManager);
}

void cDynamicSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	mSetVisibleArray = false;

	if( mUpdateGeomInfo )
	{
		/// Clear collect info
		ClearCollectInfo();
		RecursiveNodeInfo( GetNiObj() );

		NiMesh::CompleteSceneModifiers( GetNiObj() );

		mUpdateGeomInfo = false;
	}

	/// 임의로 뼈대정보를 변경하는 경우에 사용한다.
	UpdateBipedRootTransforms( accumTime );

	/// Alpha Fade In/Out 처리
	ProcessAlpha( deltaTime );

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	/// 경과 시간 * 속도 조절자
	unsigned long delta = accumTime - mLastTime;
	delta = (unsigned long)(delta * mScaleFactor);

	/// actormanager 전용 시간 계산
	if( mLastTime == ULONG_MAX )
		mScaleAccumTime = accumTime;
	else
		mScaleAccumTime += delta;

	float fScaleTime = mScaleAccumTime*0.001f;

	/// 객체의 Update
	mpActorManager->Update( fScaleTime );

	GetNiNode()->Update( fScaleTime );

	/// 경계구를 설정
	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 장면 트리를 갱신
	if( mContainer )
		mContainer->Update( this );

	///
	if( mNeedUpdateTransform )
	{
		mNeedUpdateTransform = false;
		mNeedUpdateShadow = true;
	}

	/// 누적 시간을 설정
	mLastTime = accumTime;

	ProcessTrail( deltaTime, accumTime );
}

bool cDynamicSceneNode::OnVisible()
{
	if( SCENEMAN->IsScreenShotMode() == true )
		return false;

	if( mAlpha < 0.00001f )
		return false;

	return cSceneNode::OnVisible();
}

bool cDynamicSceneNode::Init( const cDynamicSceneNodeParam& param )
{
	/// 리소스 관리자로 부터 모델정보 획득..
	mpModel = RESOURCEMAN->LoadKFM( param.mPathName.Cstr() );
	if( mpModel == 0 )
	{
		assert(0 && "Model file not found");
		return false;
	}

	/// 링크를 위한 메모리를 잡는다.
	if( InitLinkInfo() == false )
		return false;

	/// ActorManager 생성..
	mpActorManager = cActorManagerForPartition::Create( mpModel );
	if( mpActorManager == 0 )
	{
		assert( 0 && "null actor manager" );
		return false;
	}

	/// 모핑 정보가 있다면 셋팅한다.
	//	mpMorpherController = mpActorManager->GetMorpherController();
	//	if( mpMorpherController )
	//	{
	//		mpMorpherController->SetActive( false );
	//		mpMorpherController->SetAlwaysUpdate( false );
	//		mpMorpherController->Stop();
	//
	//		StartMorpher();
	//	}

	/// 갱신
	mpActorManager->Update(0.0f);

	/// IDLE 상태 애니메이션 셋팅.. (여러가지 형태의 IDLE이 존재 하는 경우)
	InitializeRandomIdle();

	/// 엔진 노드를 설정
	//	NiNode* n = GetNiObj();
	mSceneNiNode = mpActorManager->GetNifRoot();
	if( GetNiObj() == 0 )
	{
		assert( 0 && "null node" );
		return false;
	}

	{
		/// 변환 행렬을 설정
		GetNiNode()->SetTranslate( param.mTranslate );
		GetNiNode()->SetRotate( param.mRotate );
		GetNiNode()->SetScale( param.mScale );


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
	}

	/// 알파 속성이 없으면 생성
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)GetNiObj()->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		alphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		GetNiNode()->AttachProperty( alphaProp );
	}

//	NiAmbientLight* amb = FindAmbientLight( GetNiObj() );
//	if( amb )
//		GetNiNode()->DetachChild( amb );

	/// 정보 추출
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	mPick.SetQueryType( NiPick::QUERY_FIRST );
	mPick.SetIntersectType( NiPick::INTERSECT_TRIANGLE );
	mPick.SetSortType( NiPick::SORT_OFF );
	mPick.SetFrontOnly( false );

	/// 조명을 부착
//	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneAmbientLight( 1 ) );
//	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneDirLight( 1 ) );

	/// 안개 속성을 부착
//	GetNiNode()->AttachProperty( FOGAGENT->GetFogProperty() );

	/// 갱신
	bool selectiveUpdate = true;
	bool rigid = false;
	GetNiNode()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	GetNiNode()->UpdateProperties();
	GetNiNode()->UpdateEffects();

	GetNiNode()->Update( 0.0f );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	GetNiNode()->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	mAlpha = 1.0f;
	SetAlphaBlended( 1.0f );

	mpHead = GetDummyObject( eLINK_HEAD );
	if( !mpHead )
	{
		assert(0);
		return false;
	}
	mpFoot = GetDummyObject( eLINK_FOOT );
	if( mpFoot == 0 )
	{
		assert(0);
		return false;
	}

	mLastTime = param.mWoldAccumTime;

	return true;
}

void cDynamicSceneNode::SetTargetAnimation( unsigned int code )
{

//	if( mTargetAnimation != code )
	{
		NiSequenceData* seq = mpActorManager->SetTargetAnimation( code );
		if( seq )
		{
			mTargetAnimation = code;
			mTargetAnimationEndTime = THEAPP->GetAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = GetScaleAccumTime() + (unsigned long)(seq->GetDuration()*1000);
		}
	}
}

bool cDynamicSceneNode::UpdateTargetAnimation( unsigned int code )
{
/*
	if( mTargetAnimation != code )
	{
		NiSequenceData* seq = mpActorManager->SetTargetAnimation( code );
		if( seq )
		{
			mTargetAnimation = code;
			mTargetAnimationEndTime = THEAPP->GetAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = GetScaleAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			return true;
		}
	}
	else
*/
	{
		NiSequenceData* seq = mpActorManager->UpdateTargetAnimation( code );
		if( seq )
		{
			mTargetAnimation = code;
			mTargetAnimationEndTime = THEAPP->GetAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = GetScaleAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			return true;
		}
	}
	return false;
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeParts( unsigned int index, NiNode* pObj )
* 목    적 :	파트를 교체하는데 사용
* 주의사항 :	본LOD가 완전히 테스트된 상태는 아니다. (좀더 수정요!!)
*				스킨과 본 정보 갱신에 특히 유의 해야한다.
* ------------------------------------------------------------------ */
bool cDynamicSceneNode::ChangeParts( unsigned int partsIdx, NiNode* pNode )
{
	if( partsIdx == ePART_FACE )
	{
		assert(0);
		return false;
	}

	/// 기하 정보를 갱신
	mUpdateGeomInfo = true;


	/// 1. 기존 파트에 대한 정보를 수집한다.
	NiAVObject* pSrcNode = mpActorManager->GetParts( partsIdx );
	if( !pSrcNode )
	{
		assert(0);
		return false;
	}
	NiMesh* pSrcMesh = NULL;
	if( NiIsKindOf(NiMesh, pSrcNode) )
	{
		pSrcMesh = (NiMesh*)pSrcNode;
	}
	else
	{
		pSrcMesh = GetGeom_Parts(pSrcNode);
	}
	if( !pSrcMesh )
	{
		assert(0);
		return false;
	}
	NiSkinningMeshModifier* pSrcSkinningModifier = NiGetModifier(NiSkinningMeshModifier, pSrcMesh);
	if( pSrcSkinningModifier == 0 )
	{
		assert(0);
		return false;
	}

	/// 2. 교체 되어질 파트에 대한 정보 수집
	const char* partsName =RESOURCEMAN->GetManagedPartName( partsIdx );
	if( partsName == 0 || strlen(partsName) == 0 )
	{
		assert(0);
		return false;
	}
	NiMesh* pDestMesh = GetGeom_Parts( pNode );
	if( !pDestMesh )
	{
		assert(0);
		return false;
	}
	NiSkinningMeshModifier* pDestSkinningModifier = NiGetModifier(NiSkinningMeshModifier, pDestMesh);
	if( pDestSkinningModifier == 0 )
	{
		assert(0);
		return false;
	}
	NiAVObject** ppBones = pDestSkinningModifier->GetBones();
	if( ppBones == 0 )
	{
		assert(0);
		return false;
	}
	NiTransform* pSkinToBoneTransforms = pDestSkinningModifier->GetSkinToBoneTransforms();
	if( pSkinToBoneTransforms == 0 )
	{
		assert(0);
		return false;
	}
	NiBound* pBoneBounds = pDestSkinningModifier->GetBoneBounds();
	if( pBoneBounds == 0 )
	{
		assert(0);
		return false;
	}

	/// 3. 새로운 정보를 생성
	NiSkinningMeshModifier* pkModifier = NiNew NiSkinningMeshModifier( pDestSkinningModifier->GetBoneCount() );
	pkModifier->SetSoftwareSkinned( pDestSkinningModifier->GetSoftwareSkinned() );

	// Get the destinations for the converted skinning data
	NiAVObject** apkDestBones = pkModifier->GetBones();
	NIASSERT(apkDestBones);

	NiTransform* akDestSkinToBoneTransforms = pkModifier->GetSkinToBoneTransforms();
	NIASSERT(akDestSkinToBoneTransforms);

	pkModifier->EnableDynamicBounds();
	NiBound* akDestBoneBounds = pkModifier->GetBoneBounds();
	NIASSERT(akDestBoneBounds);

	// Set the root bone and its BoneToBindSpace transform
	pkModifier->SetRootBoneParent( pSrcSkinningModifier->GetRootBoneParent() );
	pkModifier->SetRootBoneParentToSkinTransform( pSrcSkinningModifier->GetRootBoneParentToSkinTransform() );

	// Set each bone and is bind to bone space transforms
	for (unsigned int i = 0; i < pDestSkinningModifier->GetBoneCount(); i++ )
	{
		NiAVObject* pBone = ppBones[i];
		if( pBone == 0 )
		{
			assert(0);
			continue;
		}
		NiAVObject* pNewBone = GetNiObj()->GetObjectByName( pBone->GetName() );
		if( pNewBone == 0 )
		{
			assert(0);
			continue;
		}

		// Add bones, transforms, and bone bounds
		apkDestBones[i] = pNewBone;
		akDestSkinToBoneTransforms[i] =  pSkinToBoneTransforms[i];
		akDestBoneBounds[i] = pBoneBounds[i];
	}

	/// 새로운 객체로 바꾼다.
	mpActorManager->ChangePartsObject( partsIdx, pNode );

	/// Clone한 Object에 있던 Bone Root정보를 삭제한다.
	if( pNode->DetachChild( pNode->GetObjectByName("foot_dummy") ) == 0 )
	{
		if( pNode->DetachChild( pNode->GetObjectByName("Bip01") ) == 0 )
		{
			NiMessageBox( "Warning : this object [foot_dummy] check!!", "Error Dlg" );
			assert(0);
		}
	}

	pDestMesh->RemoveModifier( pDestSkinningModifier );
	pDestMesh->AddModifier( pkModifier );
	pDestMesh->ResetModifiers();

	return true;
}

bool cDynamicSceneNode::ChangePartsTexture( unsigned int partsIdx, NiTexture* pTex )
{
	/// 1. 기존 파트에 대한 정보를 수집한다.
	NiAVObject* pNode = mpActorManager->GetParts( partsIdx );
	if( pNode && pTex )
	{
		NiMesh* pMesh = GetGeom( pNode );
		if( pMesh )
		{
			NiTexturingProperty* pProperty = (NiTexturingProperty*)pMesh->GetProperty( NiProperty::TEXTURING );
			if( pProperty )
			{
				pProperty->SetBaseTexture( pTex );
				pMesh->UpdateProperties();
				return true;
			}
		}
	}
	return false;
}

/* ------------------------------------------------------------------
* 함수이름 :	LinkObject( unsigned int index, NiNode* pObj )
* 목    적 :	무기를 특정더미에 링크 시킨다.
* 주의사항 :	리턴되는 인덱스를 활용해서 제거되므로 꼭 받아서 보관할것..
* ------------------------------------------------------------------ */
unsigned int cDynamicSceneNode::LinkObject( unsigned int dummyIdx, NiNode* pNode )
{
	if( pNode == 0 )
	{
		assert(0);
		return UINT_MAX;
	}

	/// Parts Name점검.. 잘못된 인덱스 체크
	const char* dummyName = RESOURCEMAN->GetManagedDummyName( dummyIdx );
	if( dummyName == 0 || strlen(dummyName) == 0 )
	{
		assert(0);
		return UINT_MAX;
	}

	/// 링크시킬 위치 객체 획득..
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiObj()->GetObjectByName( dummyName ) );
	assert(pDummy);
	if( pDummy == NULL )
	{
		assert(0);
		return UINT_MAX;
	}

	/// 원본객체 복사본 생성..
	NiCloningProcess cloning;
	cloning.m_eCopyType = NiObjectNET::COPY_EXACT;
	NiNode* pNewObj = NiDynamicCast( NiNode, pNode->Clone( cloning ) );
	assert(pNewObj);
	if( pNewObj == NULL )
	{
		assert(0);
		return UINT_MAX;
	}

	//	LinkDataInit( pNewObj );

	/// 실제 링크..
	pDummy->AttachChild( pNewObj );

	/// 갱신
	pNewObj->Update(0.0f);
	pNewObj->UpdateProperties();
	pNewObj->UpdateEffects();

	GetNiObj()->UpdateNodeBound();

	/// 링크 정보 입력..
	sLinkInfo* pkInfo = NiNew sLinkInfo;
	pkInfo->DummyIndex = dummyIdx;
	pkInfo->pObject = pNewObj;

	unsigned int linkIdx = mpLinkInfo->Add( pkInfo );

	/// 기하 정보를 갱신
	mUpdateGeomInfo = true;

	return linkIdx;
}

void cDynamicSceneNode::UnLinkObject( unsigned int linkIdx )
{
	if( linkIdx == UINT_MAX )
		return;

	sLinkInfo* pInfo = mpLinkInfo->GetAt( linkIdx );
	if( pInfo == NULL )
	{
		assert(0);
		return;
	}

	const char* dummyName = RESOURCEMAN->GetManagedDummyName( pInfo->DummyIndex );
	if( dummyName == 0 || strlen(dummyName) == 0 )
	{
		assert(0);
		return;
	}

	/// 링크된 더미를 찾는다.
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiObj()->GetObjectByName(dummyName) );
	if( pDummy == NULL )
	{
		assert(0);
		return;
	}

	/// 더미
	assert( pInfo->pObject );
	if( pInfo->pObject )
	{
		NiNode* parent = pInfo->pObject->GetParent();
		assert( parent );

		if( parent )
		{
			assert( pDummy == parent );
			if( parent->DetachChild( pInfo->pObject ) == 0 )
			{
				assert(0);
			}
		}
	}

	//	if( pDummy->DetachChild( pInfo->pObject ) == 0 )
	//		assert(0);

	mpLinkInfo->RemoveAt( linkIdx );
	SAFE_NIDELETE(pInfo);

	pDummy->Update(0.0f, false);

	GetNiObj()->UpdateNodeBound();

	/// 기하 정보를 갱신
	mUpdateGeomInfo = true;
}

NiAVObject*	cDynamicSceneNode::GetLinkObject( unsigned int linkId )
{
	if( linkId == UINT_MAX ) return 0;

	sLinkInfo* pInfo = mpLinkInfo->GetAt( linkId );
	if( pInfo )
	{
		return pInfo->pObject;
	}
	return 0;
}

cEffectSceneNode* cDynamicSceneNode::LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow )
{
	cEffectSceneNodeParam param;
	param.mpFollowSceneNode = this;
	param.mDummyIdx = dummyIdx;
	param.mPathName = fileName;	

	if( trans != 0 )
	{
		param.mTranslate = trans->m_Translate;
		param.mRotate = trans->m_Rotate;
		param.mScale = trans->m_fScale;
	}
	param.mFollowing = bFollow;
	param.mLooping = bLoop;

	cEffectSceneNode* pnode = SCENEMAN->CreateEffect( param );
	if( pnode && bFollow )
	{
		mEffectList.PushBack( pnode->GetIndexByManger() );
	}
	return pnode;
}

cEffectSceneNode* cDynamicSceneNode::LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse )
{
	cEffectSceneNodeParam param;
	param.mpFollowSceneNode = this;
	param.mDummyIdx = dummyIdx;
	param.mPathName = fileName;	

	if( trans != 0 )
	{
		param.mTranslate = trans->m_Translate;
		param.mRotate = trans->m_Rotate;
		param.mScale = trans->m_fScale;
	}
	param.mFollowing = false;
	param.mLooping = false;

	cEffectSceneNode* pnode = SCENEMAN->CreateDamageEffect( param, IsMiss, zFalse );

	return pnode;
}

/// 주의사항 :
void cDynamicSceneNode::UnLinkEffect( cEffectSceneNode* pnode )
{
	assert(pnode);
	if( pnode == 0 )
		return;

	mEffectList.Remove( pnode->GetIndexByManger() );

	NiNode* parent = pnode->GetNiObj()->GetParent();
	if( parent )
	{
		parent->DetachChild( pnode->GetNiObj() );
	}
	else
	{
		assert(0);
	}
}

cSoundSceneNode* cDynamicSceneNode::LinkSound( unsigned long soundIdx, bool bLoop )
{
	/// 참조할 위치 객체 획득..
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiObj()->GetObjectByName("body_dummy") );
	if( pDummy == NULL )
	{
		return NULL;
	}

	sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( soundIdx );
	if( pInfo == 0 )
		return 0;

	cSoundSceneNodeParam param;
	if( pInfo->mRadius <= 0.0f )
	{
		param.mRadius = 100.0f;
	}
	else
	{
		param.mRadius = pInfo->mRadius;
	}
	param.mLoopCount = bLoop? 0:1;
	param.mVolumeRatio = pInfo->mVolume;

	/// 기본 속성 인자들을 설정
	param.mPathName = pInfo->mFileName;
	param.mTranslate = pDummy->GetWorldTranslate();
	param.mManaged = true;
	param.mpOwnerSceneNode = this;
	param.mEffectSound = true;

	cSoundSceneNode* pnode = SCENEMAN->CreateEffectSound( param );
	if( pnode )
	{
		mSoundList.PushBack( pnode->GetIndexByManger() );
		//		mSoundList.AddFirstEmpty( pnode );
	}

	return pnode;
}

void cDynamicSceneNode::UnLinkSound( cSoundSceneNode* pnode )
{
	assert(pnode);
	if( pnode == 0 )
		return;

	mSoundList.Remove( pnode->GetIndexByManger() );
}
/*
///
void cDynamicSceneNode::StartMorpher()
{
mStopMorpherProcess = false;
}

///
void cDynamicSceneNode::StopMorpher()
{
mStopMorpherProcess = true;
}
*/
void cDynamicSceneNode::AddToVisibleArray()
{
	if( IsViewNode() == false )
		return;

	cSceneNode::AddToVisibleArray();

	NiTListIterator pos = mRibbonList.GetHeadPos();
	while( pos )
	{
		sTrailInfo* p = mRibbonList.GetNext(pos);
		cRibbonGeometry* geom = p->mRibbonGeom;

		if( geom && geom->GetActiveVerts() >= 4 )
			SCENEMAN->AddVisibleTrailGeom( geom );
	}
}

NiAmbientLight* cDynamicSceneNode::FindAmbientLight( NiAVObject* obj )
{
	if( NiIsKindOf( NiAmbientLight, obj ) )
	{
		return (NiAmbientLight*)obj;
	}
	else if( NiIsKindOf( NiNode, obj ) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;

		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt( i );
			if( child )
			{
				NiAmbientLight* amb = FindAmbientLight( child );
				if( amb )
					return amb;
			}
		}
	}
	return 0;
}

/*
void cDynamicSceneNode::CollectPickData( NiAVObject* pobj )
{
if( pobj->GetName() == "PickObj" )
{
mPickDataList.PushBack( pobj );
return;
}
else if( NiIsKindOf(NiNode, pobj) )
{
NiAVObject* child = 0;
NiNode* node = (NiNode*)pobj;
for( unsigned int i = 0, end = node->GetArrayCount(); i < end; ++i )
{
child = node->GetAt(i);
if( child )
CollectPickData( child );
}
}
}
*/

bool cDynamicSceneNode::Pick( const cRay& ray )
{
	//if( mPickDataList.IsEmpty() )
	//{
	//	return cSceneNode::Pick( ray );
	//}

	if( mBoundSphere.IntersectRay( ray ) )
	{
		if( mPick.PickObjects( ray.GetOrigin(), ray.GetDirection() ) )
		{
			NiPick::Record* record = mPick.GetResults().GetAt(0);
			if( record )
			{
				mPickPos = record->GetIntersection();
				mPickDistance = record->GetDistance();
			}
			return true;
		}
	}
	return false;
}

/// 주의 : scalefactor 값으로 
void cDynamicSceneNode::UpdateAniScaleFactor( float factor )
{
	if( mScaleFactor == factor )
		return;

	if( factor <= 0.0f )
		factor = 0.1f;
	if( factor > 2.0f )
		factor = 2.0f;

	mScaleFactor = factor;
}

NiAVObject* cDynamicSceneNode::GetPartObject( unsigned int partsIdx )
{
	const char* partsName = RESOURCEMAN->GetManagedPartName( partsIdx );
	if( strlen(partsName) == 0 )
		return 0;

	/// 링크시킬 위치 객체 획득..
	return mSceneNiNode->GetObjectByName( partsName );
}

NiNode* cDynamicSceneNode::GetDummyObject( unsigned int dummyIdx )
{
	const char* dummyName = RESOURCEMAN->GetManagedDummyName( dummyIdx );
	if( strlen(dummyName) == 0 )
		return 0;

	/// 링크시킬 위치 객체 획득..
	return NiDynamicCast( NiNode, mSceneNiNode->GetObjectByName( dummyName ) );
}

void cDynamicSceneNode::LinkDataInit( NiAVObject* pobj )
{
	if( pobj == 0 )
		return;

	if( pobj->GetName() == "EffectSceneNode" )
		return;
	if( pobj->GetName() == "PickObj" )
		return;
	if( pobj->GetName() == "weapon_dummy" )
		return;

	NiTransform trans;
	trans.MakeIdentity();

	if( NiIsKindOf( NiMesh, pobj ) == true )
	{
		pobj->SetWorldTranslate( NiPoint3::ZERO );
		pobj->SetWorldRotate( NiMatrix3::IDENTITY );
		pobj->SetWorldScale( trans.m_fScale  );
		pobj->SetLocalTransform( trans );
	}
	else if( NiIsKindOf(NiNode, pobj) == true )
	{
		NiNode* node = (NiNode*)pobj;

		node->SetWorldTranslate( NiPoint3::ZERO );
		node->SetWorldRotate( NiMatrix3::IDENTITY );
		node->SetWorldScale( trans.m_fScale  );
		node->SetLocalTransform( trans );

		NiAVObject* child = 0;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt(i);
			if( child )
				LinkDataInit( child );
		}
	}
}

void cDynamicSceneNode::GotoLastFrame()
{
	NiControllerSequence* p = mpActorManager->GetActiveSequence(mpActorManager->GetTargetAnimation(), true, true );
	if( p )
	{
		float time = 0.0f;
		time = p->GetKeyTimeAt( NiAnimationConstants::GetEndTextKey() );

		if( time != NiControllerSequence::INVALID_TIME )
			mpActorManager->Update( -time );
	}
	/*
	NiControllerSequence* p = mpActorManager->GetSequence( mpActorManager->GetTargetAnimation() );
	if( p )
	{
	float time = 0.0f;
	time = p->GetEndKeyTime();

	if( time != NiControllerSequence::INVALID_TIME )
	mpActorManager->Update( -time );
	}
	*/
}

void cDynamicSceneNode::AnimationReset()
{
	mpActorManager->Reset();
	mpActorManager->Update(0.0f);
}

sTrailInfo*	 cDynamicSceneNode::ActiveTrailEffect(cString str, unsigned long lifeTime, unsigned int link1, unsigned int link2 )
{
	NiTexture* tex = RESOURCEMAN->LoadTexture( str );
	if( tex == 0 )
	{
		assert(0);
		return 0;
	}

	cRibbonGeometry* geom = NiNew cRibbonGeometry;
	if( geom->Init( THEAPP->GetAccumTime(), tex, 500, 10 ) == false)
	{
		NiDelete geom;
		return 0;
	}

	sTrailInfo* info = new sTrailInfo;
	info->mRibbonGeom = geom;
	info->mLifeTime = THEAPP->GetAccumTime() + lifeTime;
	info->mStartLink = link1;
	info->mEndLink = link2;
	info->mLoop = (lifeTime == 0);

	mRibbonList.AddTail( info );

	return info;
}

void cDynamicSceneNode::DeActiveTrailEffect( sTrailInfo* pInfo )
{
	if( pInfo == 0 )
		return;

	NiTListIterator pos = mRibbonList.FindPos( pInfo );
	if( pos )
	{
		cRibbonGeometry* p = pInfo->mRibbonGeom;
		if( p )
			p->SetPushEnabled( false );
		pInfo->mLifeTime = 0;
	}
}

void cDynamicSceneNode::ClearAllEffect()
{

	cLinkIndexList::cIterator i = mEffectList.Begin();
	cLinkIndexList::cIterator end = mEffectList.End();
	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)*i;

		cEffectSceneNode* n = SCENEMAN->GetEffectSceneNode( idx );
		assert(n);

		if( n )
		{
			if( n->IsRemoved() == false )
			{
				NiNode* parent = n->GetNiObj()->GetParent();
				if( parent )
				{
					parent->DetachChild( n->GetNiObj() );
				}
				else
				{
					assert(0);
				}
				n->ParentNodeRemove();
			}
		}
	}
	mEffectList.Clear();

	NiTListIterator pos = mRibbonList.GetHeadPos();
	while( pos )
	{
		sTrailInfo* p = mRibbonList.GetNext(pos);
		SAFE_NIDELETE( p );
	}
	mRibbonList.RemoveAll();
}


void cDynamicSceneNode::ClearEffect()
{
	if( mEffectList.IsEmpty() )
		return;

	cLinkIndexList::cIterator i = mEffectList.Begin();
	cLinkIndexList::cIterator end = mEffectList.End();
	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)*i;

		cEffectSceneNode* n = SCENEMAN->GetEffectSceneNode( idx );
		assert(n);

		if( n )
		{
			if( n->IsRemoved() == false )
			{
				NiNode* parent = n->GetNiObj()->GetParent();
				if( parent )
				{
					parent->DetachChild( n->GetNiObj() );
				}
				else
				{
					assert(0);
				}
				n->ParentNodeRemove();
			}
		}
	}
	mEffectList.Clear();
}

void cDynamicSceneNode::ClearSound()
{
	if( mSoundList.IsEmpty() )
		return;

	cLinkIndexList::cIterator i = mSoundList.Begin();
	cLinkIndexList::cIterator end = mSoundList.End();
	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)*i;
		cSoundSceneNode* n = SCENEMAN->GetSoundSceneNode( idx );
		assert(n);

		if( n )
		{
			if( n->IsRemoved() == false )
			{
				n->ParentNodeRemove();
			}
		}
	}
	mSoundList.Clear();
}

void cDynamicSceneNode::CollectNodeInfo(  NiAVObject* obj, bool& collectGeom, bool& collectProperty  )
{
	cSceneNode::CollectNodeInfo( obj, collectGeom, collectProperty );

	///// 픽킹박스 추출
	//if( obj->GetName() == "PickObj" )
	//{
	//	if( NiIsKindOf( NiMesh, obj ) == true)
	//	{
	//		mPickDataList.PushBack( (NiMesh*)obj );
	//	}
	//	else
	//	{
	//		assert(0);
	//	}
	//}
}

NiMesh* cDynamicSceneNode::GetGeom_Parts( NiAVObject* obj )
{
	if( obj == 0 )
	{
		assert( 0 && "null node" );
		return 0;
	}

	if( obj->GetName() == "eff_dummy")
		return 0;

	if( NiIsKindOf(NiMesh, obj) )
	{
		if( NiGetModifier(NiSkinningMeshModifier, (NiMesh*)obj) != 0 )
			return (NiMesh*)obj;
	}

	if( NiIsKindOf(NiNode, obj) )
	{
		NiAVObject* child = 0;
		NiNode* node = (NiNode*)obj;
		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			child = node->GetAt( i );
			if( child )
			{
				NiMesh* next = GetGeom_Parts( child );
				if( next && NiGetModifier(NiSkinningMeshModifier, next) )
					return next;
			}
		}
	}
	return 0;
}
