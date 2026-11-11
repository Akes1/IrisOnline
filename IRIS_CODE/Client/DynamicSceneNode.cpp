#include "stdafx.h"
#include "DynamicSceneNode.h"

#include "SceneTree.h"
#include "SceneManager.h"
#include "OptionManager.h"

#include "ResourceManager.h"
#include "RenderSystem.h"
#include "CameraManager.h"
#include "LightAgent.h"
#include "FogAgent.h"
#include "SoundSystem.h"

#include "EffectSceneNode.h"
#include "SoundSceneNode.h"
#include "ObjectNameCard.h"
#include "ShadowGeometry.h"

#include "ChatBubble.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "Application.h"


cDynamicSceneNode::cDynamicSceneNode( eType type ) 
: cSceneNode( type )
, mTargetAnimation( UINT_MAX )
, mTargetAnimationEndTime(0)
, mTargetAnimationScaleEndTime(0.0f)
, mpRandomIdle( 0 )
, mpModel(0)
, mpNameCard(0)
, mpChatBubble(0)
, mShadowGeom( 0 )
, mpLinkInfo(0)
, mpActorManager(0)
, mNeedUpdateShadow( false )
, mShowNameGauge( false )
{
	mpHead = 0;
	mpFoot = 0;
	mHeadDist = 5.0f;

	mHeadScreenPosX = 0;
	mHeadScreenPosY = 0;

	mFootScreenPosX = 0;
	mFootScreenPosY = 0;

//	mpMorpherController = 0;
//	mStopMorpherProcess = false;

	mScaleAccumTime = 0;
	mLastUpdateTime = 0;
	mScaleFactor = 1.0f;

	mVisibleCamLength = 0.0f;

	mUpdateGeomInfo = false;

//	mpGameObject = 0;
	mGameObject.type = eOBJECTTYPE_NONE;
	mGameObject.index = 0;

	mSetVisibleArray = false;

	mDisplayObj = 0;

	mTargetScale = 1.0f;
	mScaleCheckTime = 0;
}

cDynamicSceneNode::~cDynamicSceneNode()
{
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

	/// 말풍선 제거
	SAFE_DELETE(mpChatBubble);

	/// 이름 객체 제거
	SAFE_DELETE(mpNameCard);

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
	SAFE_DELETE( mShadowGeom );

	SAFE_NIDELETE(mpActorManager);

	mGameObject.type = eOBJECTTYPE_NONE;
	mGameObject.index = 0;
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

	if( mGameObject.type != eOBJECTTYPE_HERO )//mpGameObject != HERO )
	{
		cBaseObject* gameObj = GetGameObject();
		if( gameObj && gameObj->IsDie() == false )
		{
			if( SCENEMAN->CalcDynamicVisibleLevel( this ) == true )
			{
				if( mTargetAlpha == 0.0f )
					SetAlphaBlended( 1.0f );
			}
			else
			{
				if( mTargetAlpha > 0.01f )
					SetAlphaBlended( 0.0f );
			}
		}
	}

	/// Alpha Fade In/Out 처리
	ProcessScale( deltaTime );
	ProcessAlpha( deltaTime );

	if( mAlpha < 0.01f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	/// 경과 시간 * 속도 조절자
	unsigned long delta = accumTime - mLastUpdateTime;
	delta = (unsigned long)(delta * mScaleFactor);

	mScaleAccumTime += delta;

	float fScaleTime = mScaleAccumTime*0.001f;

	/// 객체의 Update
	mpActorManager->Update( fScaleTime );

	if( mUpdateSelectLIght )
	{
		if( mAttachSelectLight )
			GetNiNode()->AttachEffect( LIGHTAGENT->GetSelectAmbientLight() );
		else
			GetNiNode()->DetachEffect( LIGHTAGENT->GetSelectAmbientLight() );

		mUpdateSelectLIght = false;
		GetNiNode()->UpdateEffects();
	}

	GetNiNode()->Update( fScaleTime );

	/// 경계구를 설정
	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 장면 트리를 갱신
	if( mContainer )
		mContainer->Update( this );

	///
	if( mpChatBubble )
		mpChatBubble->Update( accumTime );

	///
	if( mNeedUpdateTransform )
	{
		mNeedUpdateTransform = false;
		mNeedUpdateShadow = true;
	}

	/// 누적 시간을 설정
	mLastUpdateTime = accumTime;

	ProcessTrail( deltaTime, accumTime );
}

bool cDynamicSceneNode::OnVisible()
{
	if( SCENEMAN->IsScreenShotMode() == true )
		return false;

	if( mAlpha <= 0.01f )
		return false;

	/// 그림자
	cBaseObject* gameObj = GetGameObject();
	if( gameObj )
	{
		if( gameObj->IsDie() == false && mShadowGeom )
		{
			if( mAlpha > 0.01f )
				mShadowGeom->Process( mNeedUpdateShadow, GetWorldTranslate(), mAlpha );

			mNeedUpdateShadow = false;
		}
	}
	
	return cSceneNode::OnVisible();
}

bool cDynamicSceneNode::Init( const cDynamicSceneNodeParam& param )
{
	if( param.mpObject == 0 )
	{
		assert(0);
		return false;
	}

	mGameObject.type = param.mpObject->GetObjectType();
	mGameObject.index = param.mpObject->GetObjectID();

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

	/// 갱신
	mpActorManager->Update( 0.0f );

	/// IDLE 상태 애니메이션 셋팅.. (여러가지 형태의 IDLE이 존재 하는 경우)
	InitializeRandomIdle();

	/// 엔진 노드를 설정
	mSceneNiNode = mpActorManager->GetNifRoot();
	if( GetNiNode() == 0 )
	{
		assert( 0 && "null node" );
		return false;
	}

	{
		/// 변환 행렬을 설정
		GetNiNode()->SetTranslate( param.mTranslate );
		GetNiNode()->SetRotate( param.mRotate );
		GetNiNode()->SetScale( param.mScale );

		/// 갱신 여부를 설정
		mNeedUpdateTransform = true;

		/// 픽킹 방식을 설정
		mPick.SetQueryType( NiPick::QUERY_FIRST );
		mPick.SetSortType( NiPick::SORT_OFF );
		mPick.SetIntersectType( NiPick::INTERSECT_TRIANGLE );
		mPick.SetCoordinateType( param.mPickCoordinate );
		mPick.SetFrontOnly( false );
		mPick.SetReturnTexture( param.mPickReturnTexture );
		mPick.SetReturnNormal( param.mPickReturnNormal );
		mPick.SetReturnSmoothNormal( false );
		mPick.SetReturnColor( param.mPickReturnColor );
		mPick.SetTarget( GetNiObj() );
		mPick.SetPickObjectPolicy(NiNew SimpleBoxPickObjectPolicy(true));
	}

	/// 알파 속성이 없으면 생성
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)GetNiNode()->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		alphaProp = NiNew NiAlphaProperty;
		alphaProp->SetAlphaBlending( false );
		alphaProp->SetAlphaTesting( false );
		alphaProp->SetTestMode(NiAlphaProperty::TEST_GREATEREQUAL);
		GetNiNode()->AttachProperty( alphaProp );
	}

	NiAmbientLight* amb = FindAmbientLight( GetNiObj() );
	if( amb )
		GetNiNode()->DetachChild( amb );

	/// 정보 추출
	ClearCollectInfo();
	RecursiveNodeInfo( GetNiObj() );

	/// 조명을 부착
	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneAmbientLight( 1 ) );
	GetNiNode()->AttachEffect( LIGHTAGENT->GetSceneDirLight( 1 ) );

	/// 안개 속성을 부착
	GetNiNode()->AttachProperty( FOGAGENT->GetFogProperty() );

	/// 갱신
	bool selectiveUpdate = true;
	bool rigid = false;
	GetNiNode()->SetSelectiveUpdateFlags( selectiveUpdate, true, rigid );

	GetNiNode()->UpdateProperties();
	GetNiNode()->UpdateEffects();

	NiMeshUpdateProcess kUpdateProcess;
//	kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
	GetNiNode()->Update( kUpdateProcess );
	NiMesh::CompleteSceneModifiers( GetNiObj() );
	GetNiNode()->UpdateNodeBound();

	mTargetScale = GetNiNode()->GetWorldScale();
	mScaleDt[0] = mScaleDt[1] = mTargetScale;

	mBoundSphere.Set( GetCenter(), GetRadius() );

	/// 정보표시를 위한 객체 생성
	if( param.mCreateNameCard )
	{
		mpNameCard = CreateNameCard( GetGameObject() ); //mpGameObject );
		assert(mpNameCard);
	}

	if( param.mUseShadow )
	{
		/// 그림자 기하를 생성
		assert( mShadowGeom == 0 );
		mShadowGeom = new cShadowGeometry();
		if( mShadowGeom->Init( RESOURCEMAN->GetShadowTexture(), GetRadius() * 0.6f ) == false )
		{
			assert( 0 );
			return false;
		}
		mNeedUpdateShadow = true;
	}

	NiPoint3 camPos = CAMERAMAN->GetCurrentCameraPos();
	if( (camPos - GetWorldTranslate()).Length() > 4000.0f )
	{
		mAlpha = 0.0f;
		SetAlphaBlended( 0.0f );
	}
	else
	{
		mAlpha = 1.0f;
		SetAlphaBlended( 1.0f );
	}

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

	mLastUpdateTime = THEAPP->GetWorldAccumTime();

	return true;
}

void cDynamicSceneNode::SetTargetAnimation( unsigned int code )
{
	if( mTargetAnimation != code )
	{
		NiSequenceData* seq = mpActorManager->SetTargetAnimation( code );
		if( seq )
		{
			mTargetAnimation = code;
			mTargetAnimationEndTime = THEAPP->GetWorldAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = (float)GetScaleAccumTime()*0.001f + seq->GetDuration();
		}
	}
}

bool cDynamicSceneNode::UpdateTargetAnimation( unsigned int code )
{
	if( mTargetAnimation != code )
	{
		NiSequenceData* seq = mpActorManager->SetTargetAnimation( code );
		if( seq )
		{
			mTargetAnimation = code;
			mTargetAnimationEndTime = THEAPP->GetWorldAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = (float)GetScaleAccumTime()*0.001f + seq->GetDuration();
			return true;
		}
	}
	else
	{
		NiSequenceData* seq = mpActorManager->UpdateTargetAnimation( code );
		if( seq )
		{
			mTargetAnimationEndTime = THEAPP->GetWorldAccumTime() + (unsigned long)(seq->GetDuration()*1000);
			mTargetAnimationScaleEndTime = (float)GetScaleAccumTime()*0.001f + seq->GetDuration();
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

	/// 새로운 객체로 바꾼다.
	mpActorManager->ChangePartsObject( partsIdx, pNode );

	/// Clone한 Object에 있던 Bone Root정보를 삭제한다.
	if( pNode->DetachChild( pNode->GetObjectByName("foot_dummy") ) == 0 )
	{
		if( pNode->DetachChild( pNode->GetObjectByName("Bip01") ) == 0 )
		{
			assert(0);
		}
	}

	pDestMesh->RemoveModifier( pDestSkinningModifier );
	pDestMesh->AddModifier( pkModifier );
	pDestMesh->ResetModifiers();


/*
	/// 1. 기존 파트에 대한 정보를 수집한다.
	NiAVObject* pOldNode = mpActorManager->GetParts( partsIdx );
	if( !pOldNode )
	{
		assert(0);
		return false;
	}

	/// Geom객체를 획득한다.
	NiTriBasedGeom* pOldGeom = NULL;
	if( NiIsKindOf(NiTriBasedGeom, pOldNode) )
	{
		pOldGeom = (NiTriBasedGeom*)pOldNode;
	}
	else
	{
		pOldGeom = NiDynamicCast( NiTriBasedGeom, GetGeom(pOldNode) );
	}

	if( !pOldGeom )
	{
		assert(0);
		return false;
	}

	/// 스킨정보 획득
	NiSkinInstance* pOldSkinInst = pOldGeom->GetSkinInstance();
	assert(pOldSkinInst);
	if( !pOldSkinInst )
	{
		return false;
	}
	NiSkinData* pOldSkinData = pOldSkinInst->GetSkinData();
	assert(pOldSkinData);
	if( !pOldSkinData )
	{
		return false;
	}

	/// 2. 교체 되어질 파트에 대한 정보 수집
	const char* partsName =RESOURCEMAN->GetManagedPartName( partsIdx );
	if( partsName == 0 || strlen(partsName) == 0 )
	{
		assert(0);
		return false;
	}

//	/// 교체 객체의 Geom객체 정보
	NiTriBasedGeom* pGeom = NiDynamicCast(NiTriBasedGeom, GetGeom( pNode ) );
	assert(pGeom);
	if( !pGeom )
	{
		return false;
	}

	/// 교체 객체의 스킨정보
	NiSkinInstance* pSkinInst = pGeom->GetSkinInstance();
	assert(pSkinInst);
	if( !pSkinInst )
	{
		return false;
	}
	NiSkinData* pSkinData = pSkinInst->GetSkinData();
	assert(pSkinData);
	if( !pSkinData )
	{
		return false;
	}

	/// 교체되어질 Bone정보
	NiAVObject*const* ppBones = pSkinInst->GetBones();
	assert(ppBones);
	if( !ppBones )
	{
		return false;
	}
	NiSkinData::BoneData* pBoneData = pSkinData->GetBoneData();
	assert(pBoneData);
	if( !pBoneData )
	{
		return false;
	}

	/// 새로운 모델의 Bone정보를 현재 Bone Hierachy의 Bone Node들로 교체한다.
	/// 하나의 본셋에서 분할한 것이므로 본은 기존것을 공유한다.
	/// 분할 객체 자체도 독립객체로 출력이 가능하도록 본 정보를 갖는다.
	/// 주의!! 이 본정보는 모두 같지만, 참조 위치를 기존 포인터로 변경해야 한다.
	/// (스킨정보가 참조하는 메모리 주소로의 변경이다.)
	unsigned int i;
	NiAVObject *pBone = NULL;
	NiAVObject *pNewBone = NULL;
	NiTPrimitiveSet<NiAVObject*> newBones;
	NiTPrimitiveSet<sBoneDataObj*> newBoneDataObjs;
	sBoneDataObj* pBoneDataObj = NULL;

//	NiAVObject* pRoot= GetNiObj();
	for( i=0; i<pSkinData->GetBoneCount(); ++i )
	{
		pBone = ppBones[i];
		if( !pBone )
		{
			assert(0);
			continue;
		}

		pNewBone = GetNiObj()->GetObjectByName( pBone->GetName() );
		if( !pNewBone )
		{
			assert(0);
			continue;
		}

		newBones.Add( pNewBone );
		pBoneDataObj = NiNew sBoneDataObj;
		pBoneDataObj->pBoneData = &pBoneData[i];
		for( unsigned int k = 0; k < pBoneData[i].m_usVerts; ++k )
		{
			pBoneDataObj->BoneVertData.Add( &pBoneData[i].m_pkBoneVertData[k] );
		}
		newBoneDataObjs.Add( pBoneDataObj );
	}

	/// Create the new arrays.
	unsigned int numNewBones = newBones.GetSize();
	NiAVObject** ppNewBones = NiAlloc(NiAVObject*, numNewBones);
	NiSkinData::BoneData* pNewBoneData = NiNew NiSkinData::BoneData[numNewBones];

	/// Fill the new arrays.
	for( i = 0; i<numNewBones; ++i )
	{
		ppNewBones[i] = newBones.GetAt( i );

		pBoneDataObj = newBoneDataObjs.GetAt( i );
		pNewBoneData[i].m_kBound = pBoneDataObj->pBoneData->m_kBound;
		pNewBoneData[i].m_kSkinToBone = pBoneDataObj->pBoneData->m_kSkinToBone;

		/// 구동상의 문제는 아니지만, 별도의 체크를 요한다!!
		assert( pBoneDataObj->BoneVertData.GetSize() < (unsigned int) (unsigned short) ~0 );
		pNewBoneData[i].m_usVerts = (unsigned short)pBoneDataObj->BoneVertData.GetSize();
		pNewBoneData[i].m_pkBoneVertData = NiNew NiSkinData::BoneVertData[pNewBoneData[i].m_usVerts];

		for ( unsigned short k = 0; k<pNewBoneData[i].m_usVerts; ++k )
		{
			pNewBoneData[i].m_pkBoneVertData[k].m_fWeight = pBoneDataObj->BoneVertData.GetAt( k )->m_fWeight;
			pNewBoneData[i].m_pkBoneVertData[k].m_usVert = pBoneDataObj->BoneVertData.GetAt( k )->m_usVert;
		}
	}

	/// Create the new NiSkinData and NiSkinInstance objects.
	NiSkinData* pNewSkinData = NiNew NiSkinData( numNewBones, pNewBoneData, 
		pSkinData->GetRootParentToSkin(), pGeom->GetVertices() );
	pNewSkinData->SortAndMergeBoneData();
	NiSkinInstance* pNewSkinInst = NiNew NiSkinInstance( pNewSkinData,
		pOldSkinInst->GetRootParent(), ppNewBones );

	/// Delete allocated memory.
	for( i = 0; i< newBoneDataObjs.GetSize(); ++i )
	{
		NiDelete newBoneDataObjs.GetAt( i );
	}

	/// Clone한 Object에 있던 Bone Root정보를 삭제한다.
	if( pNode->DetachChild( pNode->GetObjectByName("foot_dummy") ) == 0)
	{
		assert(0);
	}

	/// Skin Instance 갱신
	pGeom->SetSkinInstance( pNewSkinInst );

	/// 새로운 객체로 바꾼다.
	mpActorManager->ChangePartsObject( partsIdx, pNode );

	/// Bone LOD 정보의 갱신.
	NiSkinningLODController* pOldCtrl = NULL;
	pOldCtrl = mpActorManager->GetSkinningLODController();
	if( pOldCtrl )
	{
		int curLOD = pOldCtrl->GetBoneLOD();

		mpActorManager->RefreshControllerManager();

		NiSkinningLODController* pNewCtrl = mpActorManager->GetSkinningLODController();
		pNewCtrl->SetBoneLOD( curLOD );

	}

	NiBoneLODController* pOldCtrl = NULL;
	pOldCtrl = mpActorManager->GetBoneLODController();
	if( pOldCtrl )
	{
		int curLOD = pOldCtrl->GetBoneLOD();
		pOldCtrl->ProcessScene( GetNiNode() );
		pOldCtrl->ReplaceSkin( pOldGeom, pGeom );

		mpActorManager->RefreshControllerManager();

		NiBoneLODController* pNewCtrl = mpActorManager->GetBoneLODController();
		pNewCtrl->SetBoneLOD( curLOD );
	}
*/
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

//				mUpdateGeomInfo = true;

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
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiNode()->GetObjectByName( dummyName ) );
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

	/// 실제 링크..
	pDummy->AttachChild( pNewObj );

	/// 갱신
	pNewObj->Update(0.0f);
	pNewObj->UpdateProperties();
	pNewObj->UpdateEffects();

	GetNiNode()->UpdateNodeBound();

	/// 경계구를 설정
	mBoundSphere.Set( GetCenter(), GetRadius() );

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
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiNode()->GetObjectByName(dummyName) );
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

	GetNiNode()->UpdateNodeBound();
	mBoundSphere.Set(GetCenter(), GetRadius());

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
		if( parent->DetachChild( pnode->GetNiObj() ) == 0 )
		{
			GameErrorLog( "cDynamicSceneNode::UnLinkEffect(DetachChild)[%d,%d] : %s", mGameObject.index, pnode->GetIndexByManger(), parent->GetName() );
		}

//		char tempmsg[255] = {0,};
//		sprintf( tempmsg, "\ncDynamicSceneNode::UnLinkEffect[%d, %d] : %s", GetIndexByManger(), pnode->GetIndexByManger(), parent->GetName() );
//		OutputDebugStringA( tempmsg );
	}
	else
	{
		assert(0);
	}
}

cSoundSceneNode* cDynamicSceneNode::LinkSound( unsigned char type, unsigned long soundIdx, bool bLoop )
{
	/// 참조할 위치 객체 획득..
	NiNode* pDummy = NiDynamicCast( NiNode, GetNiNode()->GetObjectByName("body_dummy") );
	if( pDummy == NULL )
	{
		return NULL;
	}

	sSoundInfo* pInfo = SOUNDSYS->GetSoundInfo( soundIdx );
	if( pInfo == 0 )
		return 0;

	if( type != eSOUNDTYPE_EFFECT && type != eSOUNDTYPE_NOFADE_EFFECT )
	{
		assert(0);
		return 0;
	}

	cSoundSceneNodeParam param;
	if( pInfo->mRadius <= 0.0f )
	{
		param.mRadius = 1000.0f;
	}
	else
	{
		param.mRadius = pInfo->mRadius;
	}
	param.mSoundType = (eSOUNDTYPE)type;
	param.mLoopCount = bLoop? 0:1;
	param.mVolumeRatio = OPTIONMAN->GetEffVolume() * pInfo->mVolume;

	/// 기본 속성 인자들을 설정
	param.mPathName = pInfo->mFileName;
	param.mTranslate = pDummy->GetWorldTranslate();
	param.mpOwnerSceneNode = this;

	cSoundSceneNode* pnode = SCENEMAN->CreateEffectSound( param );
	if( pnode )
	{
		mSoundList.PushBack( pnode->GetIndexByManger() );
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
	if( mUpdateGeomInfo == true )
		return;

	cSceneNode::AddToVisibleArray();

	{
		/// 카메라와의 거리 등록
		NiPoint3 vec = GetWorldTranslate() - CAMERAMAN->GetCurrentCameraPos();
		mVisibleCamLength = vec.Length();

		/// visible dynamic array에 등록
		SCENEMAN->AddVisibleDynamicNode( this );
	}

	///
	CalcScreenPos();

	NiTListIterator pos = mRibbonList.GetHeadPos();
	while( pos )
	{
		sTrailInfo* p = mRibbonList.GetNext(pos);
		cRibbonGeometry* geom = p->mRibbonGeom;

		if( geom && geom->GetActiveVerts() >= 4 )
			SCENEMAN->AddVisibleTrailGeom( geom );
	}
}

///
void cDynamicSceneNode::AddDepthPlaneObject()
{
	if( mpHead == 0 )
	{
		assert( 0 );
		return;
	}

	NiPoint3 pos = this->GetWorldTranslate();
	pos.z = mpHead->GetWorldTranslate().z;
	pos.z += mHeadDist;

	NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
	float distCam = (pos - camPos).Length();
	float distHero = distCam;
	if( HERO )
		distHero = (GetWorldTranslate() - HERO->GetPos()).Length();

	unsigned int optionHeight = 0;
	if( OPTIONMAN->IsShowBubble() )
	{
		if( mpChatBubble && mpChatBubble->IsActive() )
		{
			/// 말풍선 등록
			if( distCam <= 3000.0f)
			{
				SCENEMAN->AddPlane( mpChatBubble );
				mpChatBubble->SetPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5) );

				optionHeight = mpChatBubble->GetBubbleHeight()+5;
			}
		}
	}

	if( mpNameCard )
	{
		cBaseObject* gameObj = GetGameObject();
		if( distHero <= 1500 )
		{
			if( mShowNameGauge )
			{
				mpNameCard->SetEnableGauge( true );
				if( distHero > 500.0f )
				{
					if( HERO->GetTargetObject() != gameObj )
						mpNameCard->SetEnableGauge( false );
				}
			}
			else
			{
				mpNameCard->SetEnableGauge( false );
			}

			if( gameObj == 0 || gameObj->IsDie() )
			{
				mpNameCard->SetEnableGauge( false );
			}
			else
			{
				mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
				SCENEMAN->AddPlane( mpNameCard );
			}
		}
		else
		{
			/// over인 경우
			cBaseObject* o = OBJECTMAN->GetOverObject();
			if( o == gameObj && distHero <= 5000 )
			{
				mpNameCard->SetEnableGauge( false );

				mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
				SCENEMAN->AddPlane( mpNameCard );
			}
		}
	}
}

bool cDynamicSceneNode::PickManagedPlane( float /*mouseX*/, float /*mouseY*/ )
{
	return false;
}

///
void cDynamicSceneNode::ActiveChatBubble( LPTSTR msg, unsigned long color )
{
	if( OPTIONMAN->IsShowBubble() == false )
		return;

	if( mpChatBubble )
		mpChatBubble->Active( msg, color );
}

void cDynamicSceneNode::ActiveChatBubble( sTextItem* text )
{
	if( OPTIONMAN->IsShowBubble() == false )
		return;

	if( mpChatBubble )
		mpChatBubble->Active( text );
}

void cDynamicSceneNode::UpdateNameCardTextValue()
{
	if( mpNameCard )
		mpNameCard->ChangeTextValue();
}

void cDynamicSceneNode::UpdateNameCardGaugeValue()
{
	if( mpNameCard )
		mpNameCard->ChangeGaugeValue();
}

void cDynamicSceneNode::UpdateGuildName()
{
	if( mpNameCard )
		mpNameCard->ChangeGuildName();
}

void cDynamicSceneNode::UpdateGuildMark()
{
	if( mpNameCard )
		mpNameCard->ChangeGuildMark();
}

void cDynamicSceneNode::PreviewGuildMark( NiTexture* tex )
{
	if( mpNameCard )
		mpNameCard->PreviewGuildMark( tex );
}

void cDynamicSceneNode::PreviewTodayWord( LPCTSTR word, unsigned long color )
{
	if( mpNameCard )
		mpNameCard->SetTodayWord( (LPTSTR)word, color );
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
	if( factor >= 2.0f )
		factor = 1.9f;

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

void cDynamicSceneNode::GotoLastFrame()
{
	NiSequenceData* p = mpActorManager->GetSequenceData( mpActorManager->GetTargetAnimation() );
	if( p )
	{
		float time = 0.0f;
		time = p->GetKeyTimeAt( NiAnimationConstants::GetEndTextKey() );

		if( time != NiControllerSequence::INVALID_TIME )
			mpActorManager->Update( -time );
	}
}

void cDynamicSceneNode::AnimationReset()
{
	mpActorManager->Reset();
	mpActorManager->Update(0.0f);
}

sTrailInfo*	 cDynamicSceneNode::ActiveTrailEffect( cString str, unsigned long lifeTime, unsigned int link1, unsigned int link2, float lenPer, float factor )
{
	NiTexture* tex = RESOURCEMAN->LoadTexture( str );
	if( tex == 0 )
	{
		assert(0);
		return 0;
	}

	cRibbonGeometry* geom = NiNew cRibbonGeometry;
	if( geom->Init( THEAPP->GetWorldAccumTime(), tex, 500, 10 ) == false)
	{
		NiDelete geom;
		return 0;
	}

	sTrailInfo* info = new sTrailInfo;
	info->mRibbonGeom = geom;
	info->mLifeTime = THEAPP->GetWorldAccumTime() + lifeTime;
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
					if( parent->DetachChild( n->GetNiObj() ) == 0 )
					{
						GameErrorLog( "cDynamicSceneNode::ClearEffect(DetachChild)[%d,%d]\n", GetIndexByManger(), idx );
					}
				}
				else
				{
					assert(0);
					GameErrorLog( "cDynamicSceneNode::ClearEffect(No Parent)[%d,%d]\n", GetIndexByManger(), idx );
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

void cDynamicSceneNode::CalcScreenPos()
{
	/// 
	if( mpHead == 0 || mpFoot == 0 )
	{
		assert(0);
		return;
	}

	float x,y;
	NiPoint3 posF = mpFoot->GetWorldTranslate();
	cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), posF, x, y, false );
	mFootScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
	mFootScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );

	///
	NiPoint3 posH = mpHead->GetWorldTranslate();

	cBaseObject* gameObj = GetGameObject();
	if( gameObj == 0 || gameObj->IsDie() )
	{
		posF = posH;
		posF.z = posH.z + mHeadDist;
	}
	else
	{
		posF.z = posH.z + mHeadDist;
	}

	cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), posF, x, y, false );
	mHeadScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
	mHeadScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );
}

cBaseObject* cDynamicSceneNode::GetGameObject()
{
	if( mDisplayObj != 0 )
		return mDisplayObj;

	cBaseObject* obj = OBJECTMAN->GetObject( mGameObject.type, mGameObject.index );
	assert(obj);

	return obj;
}

void cDynamicSceneNode::SetTargetScale( float scale, bool init ) 
{ 
	if( init )
	{
		mTargetScale = scale;
		mScaleDt[0] = mScaleDt[1] = scale;
		SetScale( scale );
	}
	else
	{
		mTargetScale = scale; 
		mScaleDt[0] = mScaleDt[1] = GetWorldScale();
	}

	mCurDtIdx = 0;
	mScaleCheckTime = 0;
}

void cDynamicSceneNode::ProcessScale( unsigned long deltatime )
{
	NiNode* node = GetNiNode();
	if( node == 0 )
		return;

	if( mScaleDt[0] == mTargetScale )
		return;

	// 폭1
	float dt = 0.6f * 0.001f * deltatime;
	if( mScaleDt[0] > mTargetScale )
	{
		mScaleDt[0] -= dt;
		if( mScaleDt[0] < mTargetScale )
			mScaleDt[0] = mTargetScale;
	}
	else if( mScaleDt[0] < mTargetScale )
	{
		mScaleDt[0] += dt;
		if( mScaleDt[0] > mTargetScale )
			mScaleDt[0] = mTargetScale;
	}

	// 폭2
	dt = 0.8f * 0.001f * deltatime;
	if( mScaleDt[1] > mTargetScale )
	{
		mScaleDt[1] -= dt;
		if( mScaleDt[1] < mTargetScale )
			mScaleDt[1] = mTargetScale;
	}
	else if( mScaleDt[1] < mTargetScale )
	{
		mScaleDt[1] += dt;
		if( mScaleDt[1] > mTargetScale )
			mScaleDt[1] = mTargetScale;
	}

	SetScale( mScaleDt[mCurDtIdx] );


	if( mScaleCheckTime <= deltatime )
	{
		mScaleCheckTime = 55;

		mCurDtIdx++;
		mCurDtIdx = mCurDtIdx%2;
	}
	else
	{
		mScaleCheckTime -= deltatime;
	}
}

void cDynamicSceneNode::ResetHead2()
{
	/// update head2 dummy position
	NiNode* pHead1 = GetDummyObject( eLINK_HEAD );
	if( pHead1 == 0 )
		return;

	NiNode* pHead2 = GetDummyObject( eLINK_HEAD2 );
	if( pHead2 == 0 )
		return;

	NiNode* pFoot = GetDummyObject( eLINK_FOOT );
	if( pFoot == 0 )
		return;

	NiPoint3 trans = pFoot->GetTranslate();
	trans.z = pHead1->GetTranslate().z;

	pHead2->SetTranslate( trans );
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
