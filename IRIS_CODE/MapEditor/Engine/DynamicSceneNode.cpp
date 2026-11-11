#include "stdafx.h"
#include "DynamicSceneNode.h"

#include "SceneTree.h"
#include "SceneManager.h"

#include "ResourceManager.h"
#include "RenderSystem.h"
#include "CameraManager.h"
//#include "LightAgent.h"
//#include "FogAgent.h"
//#include "SoundSystem.h"

//#include "EffectSceneNode.h"
//#include "SoundSceneNode.h"
//#include "ObjectNameCard.h"
#include "TrailGeometry.h"
#include "ShadowGeometry.h"
//
//#include "ChatBubble.h"


cDynamicSceneNode::cDynamicSceneNode( eSceneNodeType type ) 
: cSceneNode( type )
, mTargetAnimation( UINT_MAX )
, mRandomIdle( 0 )
, mModel(0)
//, mpNameCard(0)
//, mpChatBubble(0)
, mLinkInfo(0)
, mActorManager(0)
, mAmbientLight( 0 )
#ifdef MAP_EDITOR
, mRibbonGeom( 0 )
, mShadowGeom( 0 )
, mNeedUpdateShadow( false )
#endif
{
	mHead = 0;
	mHeadDist = 20.0f;

	mScaleAccumTime = 0;
	mLastTime = ULONG_MAX;
	mScaleFactor = 1.0f;

	mRadius = 0.0f;
}

cDynamicSceneNode::~cDynamicSceneNode()
{
	/// 등록된 이펙트가 있다면 모두 제거 한다.
	//NiTListIterator pos = mEffectList.GetHeadPos();
	//while( pos )
	//{
	//	cEffectSceneNode* pnode = mEffectList.GetNext(pos);
	//	SCENEMAN->DestroyEffectNode( pnode, true );
	//}
	//mEffectList.RemoveAll();

	//pos = mSoundList.GetHeadPos();
	//while( pos )
	//{
	//	cSoundSceneNode* pnode = mSoundList.GetNext(pos);
	//	SCENEMAN->DestroySoundNode( pnode, true );
	//}
	//mSoundList.RemoveAll();


	/// 말풍선 제거
	//SAFE_DELETE(mpChatBubble);

	/// 이름 객체 제거
	//SAFE_DELETE(mpNameCard);

	if( mLinkInfo )
	{
		sLinkInfo* info = 0;
		for( unsigned int i=0; i<mLinkInfo->GetSize(); ++i )
		{
			info = mLinkInfo->GetAt( i );
			if( info )
			{
				NiDelete info;
			}
		}
		mLinkInfo->RemoveAll();
	}

	SAFE_NIDELETE( mLinkInfo );
	SAFE_NIDELETE( mRandomIdle );
#ifdef MAP_EDITOR
	SAFE_NIDELETE( mRibbonGeom );
	SAFE_DELETE( mShadowGeom );
#endif

	mPartsNameMap.RemoveAll();

	SAFE_NIDELETE( mActorManager );

	mModel = 0;
}

void cDynamicSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
#ifdef MAP_EDITOR
	/// 궤적
	if( mRibbonGeom )
		mRibbonGeom->Process( deltaTime );
#endif

	/// 임의로 뼈대정보를 변경하는 경우에 사용한다.
	UpdateBipedRootTransforms( accumTime );

	/// Alpha Fade In/Out 처리
	ProcessAlpha( deltaTime );

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	unsigned long delta = accumTime - mLastTime;
	delta = (unsigned long)(delta * mScaleFactor);

	/// actormanager 전용 시간 계산
	if( mLastTime == ULONG_MAX )
		mScaleAccumTime = accumTime;
	else
		mScaleAccumTime += delta;

	float fScaleTime = mScaleAccumTime*0.001f;
	mLastTime = accumTime;

	/// 객체의 Update
	mActorManager->Update( fScaleTime );

	GetNiNode()->Update( fScaleTime );

	/// 경계구를 설정
	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 장면 트리를 갱신
	if( mContainer )
		mContainer->Update( this );

	//if( mpChatBubble )
	//{
	//	if( mHead == 0 )
	//	{
	//		assert( 0 );
	//		return;
	//	}

	//	NiPoint3 headPos = mHead->GetWorldTranslate();
	//	headPos.z += mHeadDist;

	//	mpChatBubble->Update( headPos, accumTime );
	//}

	///
	if( mNeedUpdateTransform )
	{
		mNeedUpdateTransform = false;
#ifdef MAP_EDITOR
		mNeedUpdateShadow = true;
#endif
	}
}

//bool cDynamicSceneNode::OnVisible( cSceneCuller& culler )
//{
//	bool ret = cSceneNode::OnVisible( culler );
//
//	if( ret )
//	{
//		assert( mShadowGeom );
//		mShadowGeom->Update( mNeedUpdateShadow, mAlpha );
//
//		mNeedUpdateShadow = false;
//	}
//
//	return ret;
//}

bool cDynamicSceneNode::OnVisible( cSceneCuller& culler )
{
#ifdef REGEN_TOOL
	if( mCulled )
		return false;
#endif

#ifdef MAP_EDITOR
	/// 그림자
	if( mShadowGeom && mAlpha > 0.00001f )
		mShadowGeom->Process( mNeedUpdateShadow, GetWorldTranslate(), mAlpha );

	mNeedUpdateShadow = false;
#endif

	return cSceneNode::OnVisible( culler );
}

bool cDynamicSceneNode::Init( const cDynamicSceneNodeParam& param )
{
	//mpGameObject = param.mpObject;
	//assert( mpGameObject && "UnUsed Base Object" );

	/// 리소스 관리자로 부터 모델정보 획득..
	//mModel = RESOURCEMAN->GetModelByName( param.mPathName.Cstr() );
	mModel = RESOURCEMAN->LoadKFM( param.mPathName.Cstr() );
	if( mModel == 0 )
	{
		assert(0 && "Model file not found");
		return false;
	}

	/// 링크를 위한 메모리를 잡는다.
	if( InitLinkInfo() == false )
		return false;

	/// 기본 파트의 네이밍 초기화
	if( InitPartsName() == false )
	{
		return false;
	}

	/// ActorManager 생성..
	mActorManager = cActorManagerForPartition::Create( mModel );
	if( mActorManager == 0 )
	{
		assert( 0 );
		return false;
	}


	/// 갱신
	mActorManager->Update(0.0f);

	/// IDLE 상태 애니메이션 셋팅.. (여러가지 형태의 IDLE이 존재 하는 경우)
	InitializeRandomIdle();

	/// 엔진 노드를 설정
	mSceneNiNode = mActorManager->GetNifRoot();
	if( GetNiObj() == 0 )
	{
		assert( 0 && "null node" );
		return false;
	}

	{
		/// 변환 행렬을 설정
		GetNiObj()->SetTranslate( param.mTranslate );
		GetNiObj()->SetRotate( param.mRotate );
		GetNiObj()->SetScale( param.mScale );


		/// 픽킹 방식을 설정
		mPick.SetQueryType( NiPick::QUERY_FIRST );
		mPick.SetSortType( NiPick::SORT_OFF );
		mPick.SetIntersectType( NiPick::INTERSECT_COLLISION_DATA );
		mPick.SetCoordinateType( param.mPickCoordinate );
		mPick.SetFrontOnly( param.mPickFrontOnly );
		mPick.SetReturnTexture( param.mPickReturnTexture );
		mPick.SetReturnNormal( param.mPickReturnNormal );
		mPick.SetReturnSmoothNormal( false );
		mPick.SetReturnColor( param.mPickReturnColor );
		mPick.SetTarget( GetNiObj() );
//		mPick.SetPickObjectPolicy(NiNew SimpleSkinPickObjectPolicy());

		/// 경계 구를 설정
		GetNiNode()->Update( 0.0f );

		/// 갱신 여부를 설정
		mNeedUpdateTransform = true;
	}

	/// 알파 속성이 없으면 생성
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)GetNiObj()->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		alphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		GetNiObj()->AttachProperty( alphaProp );
	}

	NiAmbientLight* amb = FindAmbientLight( GetNiObj() );
	if( amb )
		GetNiNode()->DetachChild( amb );

	/// 정보 추출
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	/// 조명을 부착
	GetNiNode()->AttachEffect( SCENEMAN->GetAmbientLight( 1 ) );
	GetNiNode()->AttachEffect( SCENEMAN->GetDirLight( 1 ) );

	/// 안개 속성을 부착
	//GetNiNode()->AttachProperty( SCENEMAN->GetFogProperty() );

	/// 갱신
	bool selectiveUpdate = true;
	bool rigid = false;
	GetNiObj()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	GetNiObj()->UpdateProperties();
	GetNiObj()->UpdateEffects();
	GetNiObj()->Update( 0.0f );
	GetNiObj()->UpdateNodeBound();

	mBoundSphere.Set( GetCenter(), GetRadius() );

	///// 정보표시를 위한 객체 생성
	//if( param.mCreateNameCard )
	//{
	//	mpNameCard = CreateNameCard( mpGameObject );
	//	assert(mpNameCard);
	//}

#ifdef MAP_EDITOR
	/// 그림자 기하를 생성
	if( param.mUseShadow )
	{
		assert( mShadowGeom == 0 );
		mShadowGeom = new cShadowGeometry();
		if( mShadowGeom->Init( RESOURCEMAN->GetShadowTexture(), 140.0f ) == false )
		{
			assert( 0 );
			return false;
		}
	}
#endif
	return true;
}

void cDynamicSceneNode::SetTargetAnimation( unsigned int code )
{
	if( mTargetAnimation != code )
	{
		if( mActorManager->SetTargetAnimation( code ) == false )
			return;

		mTargetAnimation = code;
	}
}

bool cDynamicSceneNode::UpdateTargetAnimation( unsigned int code )
{
	if( mTargetAnimation != code )
	{
		if( mActorManager->SetTargetAnimation( code ) == 0 )
			return false;

		mTargetAnimation = code;
		return true;;
	}

	return (mActorManager->UpdateTargetAnimation( code ) != 0);
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeParts( unsigned int index, NiNode* pObj )
* 목    적 :	파트를 교체하는데 사용
* 주의사항 :	본LOD가 완전히 테스트된 상태는 아니다. (좀더 수정요!!)
*				스킨과 본 정보 갱신에 특히 유의 해야한다.
* ------------------------------------------------------------------ */
bool cDynamicSceneNode::ChangeParts( unsigned int partsIdx, NiNode* pNode )
{
	/// 1. 기존 파트에 대한 정보를 수집한다.
	NiAVObject* pSrcNode = mActorManager->GetParts( partsIdx );
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
		pSrcMesh = GetGeom(pSrcNode);
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
	const char* partsName = RESOURCEMAN->GetManagedPartName( partsIdx );
	if( partsName == 0 || strlen(partsName) == 0 )
	{
		assert(0);
		return false;
	}
	NiMesh* pDestMesh = GetGeom( pNode );
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
		NiAVObject* pNewBone = GetNiNode()->GetObjectByName( pBone->GetName() );
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

	/// Clone한 Object에 있던 Bone Root정보를 삭제한다.
	if( pNode->DetachChild( pNode->GetObjectByName("foot_dummy") ) == 0 )
	{
		assert(0);
	}

	pDestMesh->RemoveModifier( pDestSkinningModifier );
	pDestMesh->AddModifier( pkModifier );

	/// 새로운 객체로 바꾼다.
	mActorManager->ChangePartsObject( partsIdx, pNode );

	return true;
}

bool cDynamicSceneNode::ChangePartsTexture( unsigned int partsIdx, NiTexture* pTex )
{
	/// 1. 기존 파트에 대한 정보를 수집한다.
	NiAVObject* pNode = mActorManager->GetParts( partsIdx );
	if( pNode && pTex )
	{
		//NiAVObject* pGeom = GetGeom( pNode );
		NiAVObject* pGeom = GetGeom( (NiNode*)pNode );

		if( pGeom )
		{
			NiTexturingProperty* pProperty = (NiTexturingProperty*)pGeom->GetProperty( NiProperty::TEXTURING );
			if( pProperty )
			{
				pProperty->SetBaseTexture( pTex );
				pGeom->UpdateProperties();
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
	/// Parts Name점검.. 잘못된 인덱스 체크
	const char* partsName = 0;
	if( mPartsNameMap.GetAt( dummyIdx, partsName ) == false )
	{
		assert(0);
		return UINT_MAX;
	}

	/// 링크 대상 원본객체 획득..
//	NiGeometry* pGeom = (NiGeometry*)GetGeom( pNode );
//	assert(pGeom);
//	if( pGeom == 0 )
//	{
//		assert(0);
//		return UINT_MAX;
//	}
	NiNode* pobj = NiDynamicCast(NiNode, CollectTestData( pNode ));
	if( pobj == 0 )
	{
		assert(0);
		return UINT_MAX;
	}

	/// 링크시킬 위치 객체 획득..
	NiNode* pDummy = (NiNode*)mActorManager->GetNifRoot()->GetObjectByName( partsName );
	assert(pDummy);
	if( pDummy == 0 )
	{
		assert(0);
		return UINT_MAX;
	}

	/// 원본객체 복사본 생성..
	NiCloningProcess cloning;
	cloning.m_eCopyType = NiObjectNET::COPY_EXACT;
//	NiGeometry* pNewObj = NiDynamicCast( NiGeometry, pGeom->Clone( cloning ) );
	NiNode* pNewObj = NiDynamicCast( NiNode, pobj->Clone( cloning ) );
	assert(pNewObj);
	if( pNewObj == 0 )
	{
		assert(0);
		return UINT_MAX;
	}

	pNewObj->SetTranslate( 0.0f, 0.0f, 0.0f );

	/// 실제 링크..
	pDummy->AttachChild( pNewObj );

	/// 갱신
	pNewObj->Update(0.0f);
	pNewObj->UpdateProperties();
	pNewObj->UpdateEffects();

	GetNiNode()->UpdateNodeBound();

	/// 링크 정보 입력..
	sLinkInfo* pkInfo = NiNew sLinkInfo;
	pkInfo->DummyIndex = dummyIdx;
	pkInfo->pObject = pNewObj;

	unsigned int linkIdx = mLinkInfo->Add( pkInfo );

	/// 기하 정보를 갱신
//	RecollectGeoms();

	return linkIdx;
}

/* ------------------------------------------------------------------
* 함수이름 :	UnLinkObject( unsigned int index )
* 목    적 :	현재 링크된 객체를 제거한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
void cDynamicSceneNode::UnLinkObject( unsigned int linkIdx )
{
	if( linkIdx == UINT_MAX )
	{
		return;
	}

	sLinkInfo* pInfo = mLinkInfo->GetAt( linkIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}

	const char* partsName = 0;
	if( mPartsNameMap.GetAt( pInfo->DummyIndex, partsName ) == false )
	{
		assert(0);
		return;
	}

	/// 링크된 더미를 찾는다.
	NiNode* pDummy = (NiNode*)mActorManager->GetNifRoot()->GetObjectByName( partsName );
	if( pDummy == 0 )
	{
		assert(0);
		return;
	}

	/// 더미
//	pDummy->GetParent()->DetachChild( pInfo->pObject );
	pDummy->DetachChild( pInfo->pObject );
	mLinkInfo->RemoveAt( linkIdx );
	SAFE_NIDELETE(pInfo);

	pDummy->Update(0.0f, false);
	GetNiNode()->UpdateNodeBound();

	/// 기하 정보를 갱신
//	RecollectGeoms();
}

NiAVObject*	cDynamicSceneNode::GetLinkObject( unsigned int linkId )
{
	if( linkId == UINT_MAX ) return 0;

	sLinkInfo* pInfo = mLinkInfo->GetAt( linkId );
	if( pInfo )
	{
		return pInfo->pObject;
	}
	return 0;
}

//cEffectSceneNode* cDynamicSceneNode::LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow )
//{
//	/// Parts Name점검.. 잘못된 인덱스 체크
//	const char* partsName = 0;
//	if( mPartsNameMap.GetAt( dummyIdx, partsName ) == false )
//	{
//		return 0;
//	}
//
//	/// 링크시킬 위치 객체 획득..
//	NiNode* pDummy = (NiNode*)mActorManager->GetNifRoot()->GetObjectByName( partsName );
//	if( pDummy == 0 )
//	{
//		return 0;
//	}
//
//	cEffectSceneNodeParam param;
//	param.mpFollowSceneNode = this;
//	param.mpDummy = pDummy;
//	param.mPathName = fileName;	
//
//	if( trans != 0 )
//	{
//		param.mTranslate = trans->m_Translate;
//		param.mRotate = trans->m_Rotate;
//		param.mScale = trans->m_fScale;
//	}
//	param.mFollowing = bFollow;
//	param.mLooping = bLoop;
//
//	cEffectSceneNode* pnode = SCENEMAN->CreateEffect( param );
//	if( pnode )
//		mEffectList.AddTail(pnode);
//
//	NiNode* pRoot = (NiNode*)(NiAVObject*)mNiObject;
//	pRoot->UpdateNodeBound();
//
//	return pnode;
//}
//
//cEffectSceneNode* cDynamicSceneNode::LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool /*bFollow*/ )
//{
//	/// Parts Name점검.. 잘못된 인덱스 체크
//	const char* partsName = 0;
//	if( mPartsNameMap.GetAt( dummyIdx, partsName ) == false )
//	{
//		return 0;
//	}
//
//	/// 링크시킬 위치 객체 획득..
//	NiNode* pDummy = (NiNode*)mActorManager->GetNifRoot()->GetObjectByName( partsName );
//	if( pDummy == 0 )
//	{
//		return 0;
//	}
//
//	cEffectSceneNodeParam param;
//	param.mpFollowSceneNode = this;
//	param.mpDummy = pDummy;
//	param.mPathName = fileName;	
//
//	if( trans != 0 )
//	{
//		param.mTranslate = trans->m_Translate;
//		param.mRotate = trans->m_Rotate;
//		param.mScale = trans->m_fScale;
//	}
//	param.mFollowing = false;//bFollow;
//	param.mLooping = bLoop;
//
//	cEffectSceneNode* pnode = SCENEMAN->CreateDamageEffect( param );
//	if( pnode )
//		mEffectList.AddTail(pnode);
//
//	NiNode* pRoot = (NiNode*)(NiAVObject*)mNiObject;
//	pRoot->UpdateNodeBound();
//
//	return pnode;
//}
//
///// 주의사항 :
//void cDynamicSceneNode::UnLinkEffect( cEffectSceneNode* pnode )
//{
//	assert(pnode);
//	if( pnode == 0 )
//		return;
//
//	SCENEMAN->AddDeleteEffectNode( pnode );
//	mEffectList.Remove( pnode );
//}
//
//bool cDynamicSceneNode::IsLinkedEffect( cEffectSceneNode* pnode )
//{
//	return (!mEffectList.FindPos( pnode ))? false:true;
//}
//
//cSoundSceneNode* cDynamicSceneNode::LinkSound( unsigned long soundIdx, bool bLoop )
//{
//	/// 참조할 위치 객체 획득..
//	NiNode* pDummy = (NiNode*)mActorManager->GetNifRoot()->GetObjectByName( "body_dummy");
//	if( pDummy == 0 )
//	{
//		return 0;
//	}
//
//	sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( soundIdx );
//	if( pInfo == 0 )
//		return 0;
//
//	cSoundSceneNodeParam param;
//	if( pInfo->mRadius <= 0.0f )
//	{
//		param.mRadius = 100.0f;
//	}
//	else
//	{
//		param.mRadius = pInfo->mRadius;
//	}
//	param.mLoopCount = bLoop? 0:1;
//	param.mVolumeRatio = pInfo->mVolume;
//
//	/// 기본 속성 인자들을 설정
//	param.mPathName = pInfo->mFileName;
//	param.mTranslate = pDummy->GetWorldTranslate();
//	param.mManaged = true;
//	param.mpOwnerSceneNode = this;
//
//	cSoundSceneNode* pnode = SCENEMAN->CreateEffectSound( param );
//	if( pnode )
//		mSoundList.AddTail(pnode);
//
//	return pnode;
//}
//
//void cDynamicSceneNode::UnLinkSound( cSoundSceneNode* pnode )
//{
//	assert(pnode);
//	if( pnode == 0 )
//		return;
//
//	SCENEMAN->AddDeleteSoundNode( pnode );
//	mSoundList.Remove( pnode );
//}
//
//bool cDynamicSceneNode::IsLinkedSound( cSoundSceneNode* pnode )
//{
//	return (!mSoundList.FindPos( pnode ))? false:true;
//}

///
//void cDynamicSceneNode::CreateShadow()
//{
//}

///
//void cDynamicSceneNode::AddToVisibleArray( NiVisibleArray* psolidArray, NiVisibleArray* pskinedArray, NiVisibleArray* palphaArray ) const
//{
//	cSceneNode::AddToVisibleArray( psolidArray, pskinedArray, palphaArray );
//
//	/// 가시 이름표로 등록
//	//AddDepthPlaneObject();
//
//	/// 그림자에 대한 갱신 및 랜더링을 위한 객체 추가
//}
//
/////
//void cDynamicSceneNode::AddDepthPlaneObject() const
//{
//	/// 말풍선 보이기 여부
//	if( mpChatBubble && mpChatBubble->IsActive() )
//	{
//		/// 말풍선 등록
//		SCENEMAN->AddPlane( mpChatBubble );
//		return;
//	}
//
//	if( mHead == 0 )
//	{
////		assert( 0 );
//		return;
//	}
//
//	NiPoint3 headPos = mHead->GetWorldTranslate();
//	headPos.z += mHeadDist;
//
//	if( mpNameCard )
//	{
//		mpNameCard->OnUpdate( CAMERAMAN->GetCurrentNi(), headPos );
//
//		SCENEMAN->AddDepthPlane( mpNameCard );
//	}
//}
//
/////
//void cDynamicSceneNode::ActiveChatBubble( LPTSTR msg, unsigned long color )
//{
//	if( mpChatBubble )
//		mpChatBubble->Active( msg, color );
//}
//
//void cDynamicSceneNode::UpdateNameCardTextValue()
//{
//	if( mpNameCard )
//		mpNameCard->ChangeTextValue();
//}
//
//void cDynamicSceneNode::UpdateNameCardGaugeValue()
//{
//	if( mpNameCard )
//		mpNameCard->ChangeGaugeValue();
//}

void cDynamicSceneNode::AddToVisibleArray( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray )
{
	cSceneNode::AddToVisibleArray( solidArray, alphaArray, alphaTestArray );

//#ifdef MAP_EDITOR
//	if( mRibbonGeom && mRibbonGeom->GetNumVerts() >= 4 )
//	{
//		mRibbonGeom->SetCenter( GetCenter() );
//		alphaArray->Add( *mRibbonGeom );
//	}
//#endif
}

void cDynamicSceneNode::FindLight( NiAVObject* obj )
{
	if( NiIsKindOf( NiAmbientLight, obj ) )
	{
		mAmbientLight = (NiAmbientLight*)obj;
		SetAmbientLightAmbient( SCENEMAN->GetAmbientLight(1)->GetAmbientColor() );
		SetAmbientLightDiffuse( SCENEMAN->GetAmbientLight(1)->GetDiffuseColor() );
		SetAmbientLightDimmer( SCENEMAN->GetAmbientLight(1)->GetDimmer() );
	}
	else if( NiIsKindOf( NiNode, obj ) )
	{
		NiNode* node = (NiNode*)obj;

		for( unsigned int i = 0, iend = node->GetArrayCount(); i < iend; ++i )
		{
			NiAVObject* child = node->GetAt( i );
			if( child )
				FindLight( child );
		}
	}
}

void cDynamicSceneNode::RecollectGeoms()
{
//	mRenderObjList.Clear();
//	CollectGeoms( GetNiObj() );
}

void cDynamicSceneNode::CollectPickData( NiAVObject* pobj )
{
	if( pobj->GetName() == "PickObj" )
	{
		mPickDataList.PushBack( pobj );
		return;
	}

	if( NiIsKindOf(NiNode, pobj) )
	{
		NiNode* node = (NiNode*)pobj;
		for( unsigned int i = 0, end = node->GetArrayCount(); i < end; ++i )
		{
			NiAVObject* child = node->GetAt(i);
			if( child )
				CollectPickData( child );
		}
	}
}

bool cDynamicSceneNode::Pick( const cRay& ray )
{
	if( mPickDataList.IsEmpty() )
	{
		return cSceneNode::Pick( ray );
	}

	if( mBoundSphere.IntersectRay( ray ) )
	{
		NiAVObject* obj = 0;
		cPickDataList::cConstIterator i = mPickDataList.Begin();
		cPickDataList::cConstIterator end = mPickDataList.End();

		for( ; i != end; ++i )
		{
			obj = (NiAVObject*)*i;
			mPick.SetTarget( obj );

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
	}
	return false;
}

/// 주의 : scalefactor 값으로 
void cDynamicSceneNode::UpdateAniScaleFactor( float factor )
{
	if( factor <= 0.0f )
		factor = 0.1f;
	if( factor > 2.0f )
		factor = 2.0f;
	mScaleFactor = factor;
}

NiAVObject* cDynamicSceneNode::GetPartObject( unsigned int index ) const
{
	const char* partsName = 0;
	if( mPartsNameMap.GetAt( index, partsName ) == false )
	{
		return 0;
	}

	/// 링크시킬 위치 객체 획득..
	return mActorManager->GetNifRoot()->GetObjectByName( partsName );
}

NiAVObject* cDynamicSceneNode::CollectTestData( NiNode* pobj )
{
	if( pobj->GetChildCount() == 1 )
	{
		if( NiIsKindOf( NiGeometry, pobj->GetAt(0) ) )
		{
			pobj->GetAt(0)->SetTranslate( 0.0f, 0.0f, 0.0f );
			return pobj;
		}

		NiAVObject* p = pobj->GetAt(0);
		if( NiIsKindOf(NiNode, p) )
		{
			return CollectTestData( (NiNode*)p );
		}
	}

	return pobj;
}

void cDynamicSceneNode::GotoLastFrame()
{
	NiControllerSequence* p = mActorManager->GetActiveSequence(mActorManager->GetTargetAnimation(), true, true );
	if( p )
	{
		float time = 0.0f;
		time = p->GetKeyTimeAt( NiAnimationConstants::GetEndTextKey() );

		if( time != NiControllerSequence::INVALID_TIME )
			mActorManager->Update( -time );
	}
}
//
//float cDynamicSceneNode::GetRadius() const
//{
//	return mRadius;
//}

void cDynamicSceneNode::SetAmbientLightAmbient( const NiColor& color )
{
	if( mAmbientLight )
		mAmbientLight->SetAmbientColor( color );
}

void cDynamicSceneNode::SetAmbientLightDiffuse( const NiColor& color )
{
	if( mAmbientLight )
		mAmbientLight->SetDiffuseColor( color );
}

void cDynamicSceneNode::SetAmbientLightDimmer( float dimmer )
{
	if( mAmbientLight )
		mAmbientLight->SetDimmer( dimmer );
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