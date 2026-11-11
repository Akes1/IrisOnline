#include "StdAfx.h"
#include "PlayerSceneNode.h"

#include "ResourceManager.h"

//#include "ObjectNameCard.h"
#include "SceneManager.h"
//#include "cameraManager.h"
//#include "OptionManager.h"
#include "RenderSystem.h"

// #include "ChatBubble.h"
// #include "CenterMsgPlane.h"
// #include "Signboard.h"
// #include "PartyManager.h"
// #include "PartyUnionManager.h"
// 
// #include "ObjectManager.h"
// #include "Hero.h"

#include "ShadowGeometry.h"
//#include "StageManager.h"

#include "EffectSceneNode.h"

// #include "LightAgent.h"
// #include "FogAgent.h"
// 
// #if (defined(_DEVSYS) || defined(_DEBUG))
// #include "DevSystem.h"
// #endif


cPlayerSceneNode::cPlayerSceneNode( eType type )
: cDynamicSceneNode( type )
{
	mpTarotBoard = 0;
	mpUserSellBoard = 0;
}

cPlayerSceneNode::~cPlayerSceneNode()
{
// 	SAFE_DELETE( mpUserSellBoard );
// 	SAFE_DELETE( mpTarotBoard );
}

bool cPlayerSceneNode::Init( const cPlayerSceneNodeParam& param )
{
	if( param.mDisplayObj == true )
		mDisplayObj = param.mpObject;

	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// parts 정보 등록
	const char* partName = 0;
	for( unsigned int i=0; i<ePART_MAX; i++ )
	{
		partName = RESOURCEMAN->GetManagedPartName( i );
		if( strlen(partName) == 0 )
		{
			assert(0);
			continue;
		}
		mpActorManager->AddPartsObject( i, partName );
	}

	/// 말풍선용 객체 생성
// 	mpChatBubble = new cChatBubble;
// 	mpTarotBoard = new cTarotSignboard;
// 	mpUserSellBoard = new cUseSellSignboard;

	return true;
}

bool cPlayerSceneNode::InitLinkInfo()
{
	// 이부분은 적당한 메모리 할당을 한다. (나중에 좀더 정확하게 계산하여 추가하거나 줄일것..)
	// 대략 계산 : 양손 무기 2개 + 방패 1개
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(15, 5);
	assert(mpLinkInfo);
	if( mpLinkInfo == 0 ) 
	{
		return false;
	}

	for( unsigned int i=0; i<mpLinkInfo->GetAllocatedSize(); ++i )
	{
		mpLinkInfo->SetAt( i,0 );
	}

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitializeRandomIdle()
* 목    적 :	동작의 다양성을 제공하기 위해서
* 주의사항 :	
* ------------------------------------------------------------------ */
void cPlayerSceneNode::InitializeRandomIdle()
{
	mpRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);

	mpRandomIdle->SetAt( 0, 0 );
}

void cPlayerSceneNode::ProcessTrail( unsigned long deltatime, unsigned long accumtime )
{
	///
	NiTListIterator pos = mRibbonList.GetHeadPos();
	while( pos )
	{
		NiTListIterator kOldPos = pos;
		sTrailInfo* p = mRibbonList.GetNext(pos);
		cRibbonGeometry* geom = p->mRibbonGeom;
		if( p->mLoop == false )
		{
			if( p->mLifeTime < accumtime )
			{
				geom->SetPushEnabled( false );
				if( geom->IsDelete() == true )
				{
					/// delete
					SAFE_DELETE(p);
					mRibbonList.RemovePos( kOldPos );
					continue;
				}
			}
		}
  
		if( geom != NULL )
		{
			/// push point
			NiAVObject* obj1 = GetDummyObject( p->mStartLink );
			NiAVObject* obj2 = GetDummyObject( p->mEndLink );

			if( obj1 && obj2 )
			{
				const NiPoint3& pos1 = obj1->GetWorldTranslate();
				const NiPoint3& pos2 = obj2->GetWorldTranslate();

				geom->PushTrailPoint( accumtime, pos1, pos2 );
			}
			else
			{
				assert(0);
			}

			geom->Process( deltatime );
		}
	}
}

const NiPoint3& cPlayerSceneNode::GetObjectHead()
{
	NiNode* spine = GetDummyObject( eLINK_HEAD );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cPlayerSceneNode::GetObjectFoot()
{
	NiNode* spine = GetDummyObject( eLINK_FOOT );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cPlayerSceneNode::GetObjectCenter()
{
	NiNode* spine = GetDummyObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

/* ------------------------------------------------------------------
* 함수이름 :	ChangeParts( unsigned int index, const cString& pathName )
* 목    적 :	파트 교체용 인터페이스 제공
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cPlayerSceneNode::ChangeParts( unsigned int partIdx, const char* fileName )
{
	if( fileName == 0 )
		return false;

	///
	cString pathName;
	pathName.Format( "./Data/Character/%s", fileName );
	RESOURCEMAN->LoadNIF( pathName );

	///
	NiNode* pNode = RESOURCEMAN->CloneObjectByName( fileName );
	assert(pNode);
	if( !pNode ) 
	{
		return false;
	}

	if( partIdx == ePART_FACE )
		return cPlayerSceneNode::ChangeParts( partIdx, pNode );
	else
		return cDynamicSceneNode::ChangeParts( partIdx, pNode );
}

bool cPlayerSceneNode::ChangeParts( unsigned int partsIdx, NiNode* pNode )
{
	if( partsIdx != ePART_FACE )
		return cDynamicSceneNode::ChangeParts( partsIdx, pNode );

	/// 기하 정보를 갱신
	mUpdateGeomInfo = true;

	//////////////////////////////////////////////////////////////////////////

	/// face 와 ear에 대한 정보 획득
	NiAVObject* pSrcFace = GetPartObject( partsIdx );
	if( pSrcFace == 0 )
	{
		assert(0);
		return false;
	}
	NiAVObject* pSrcEar = GetPartObject( ePART_EAR );

	//////////////////////////////////////////////////////////////////////////
	/// Face 처리
	{
		/// 1. 기존 Face에 대한 Mesh와 Modifier정보를 획득한다.
		NiMesh* pSrcMesh = NULL;
		if( NiIsKindOf(NiMesh, pSrcFace) )
		{
			pSrcMesh = (NiMesh*)pSrcFace;
		}
		else
		{
			pSrcMesh = GetGeom(pSrcFace);
		}
		if( pSrcMesh == 0 )
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

		/// 2. 신규 Face에 대한  Mesh와 Modifier정보를 획득한다.
		const char* partsName =RESOURCEMAN->GetManagedPartName( partsIdx );
		if( partsName == 0 || strlen(partsName) == 0 )
		{
			assert(0);
			return false;
		}
		NiAVObject* pDestFace = pNode->GetObjectByName( partsName );
		if( pDestFace == 0 )
		{
			assert(0);
			return false;
		}
		NiMesh* pDestMesh = GetGeom( pDestFace );
		if( pDestMesh == 0 )
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

		/// 3. 신규 정보로 갱신한다.
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

		pDestMesh->RemoveModifier( pDestSkinningModifier );
		pDestMesh->AddModifier( pkModifier );
		pDestMesh->ResetModifiers();
	}

	//////////////////////////////////////////////////////////////////////////
	/// Ear 처리
	{
		const char* partsName =RESOURCEMAN->GetManagedPartName( ePART_EAR );
		if( partsName == 0 || strlen(partsName) == 0 )
		{
			assert(0);
			return false;
		}
		NiAVObject* pDestEar = pNode->GetObjectByName( partsName );
		if( pDestEar )
		{
			/// 1. 기존 Ear에 대한 Mesh와 Modifier정보를 획득한다.
			NiMesh* pSrcMesh = NULL;
			if( pSrcEar )
			{
				if( NiIsKindOf(NiMesh, pSrcEar) )
					pSrcMesh = (NiMesh*)pSrcEar;
				else
					pSrcMesh = GetGeom(pSrcEar);
			}

			if( pSrcMesh == 0 )
			{
				if( NiIsKindOf(NiMesh, pSrcFace) )
					pSrcMesh = (NiMesh*)pSrcFace;
				else
					pSrcMesh = GetGeom(pSrcFace);

				if( pSrcMesh == 0 )
				{
					assert(0);
					return false;
				}
			}

			NiSkinningMeshModifier* pSrcSkinningModifier = NiGetModifier(NiSkinningMeshModifier, pSrcMesh);
			if( pSrcSkinningModifier == 0 )
			{
				assert(0);
				return false;
			}

			/// 2. 신규 Ear에 대한 Mesh와 Modifier정보를 획득한다.
			NiMesh* pDestMesh = GetGeom( pDestEar );
			if( pDestMesh == 0 )
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

			/// 3. 신규 정보로 갱신한다.
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

			pDestMesh->RemoveModifier( pDestSkinningModifier );
			pDestMesh->AddModifier( pkModifier );
			pDestMesh->ResetModifiers();
		}
	}

	/// 새로운 객체로 바꾼다.
	mpActorManager->ChangePartsObject( partsIdx, pNode );

	return true;
}


bool cPlayerSceneNode::ChangePartsTexture( unsigned int partsIdx, const char* fileName )
{
	cString pathName;
	pathName.Format( "./Data/Character/%s", fileName );
	NiTexture* pTexture = RESOURCEMAN->LoadTexture( pathName, false );
	if( pTexture == 0 )
	{
		return false;
	}
	
	return cDynamicSceneNode::ChangePartsTexture( partsIdx, pTexture );
}


/* ------------------------------------------------------------------
* 함수이름 :	LinkToDummy( unsigned int index, const cString& pathName )
* 목    적 :	객체 링크용 인터페이스
* 주의사항 :	
* ------------------------------------------------------------------ */
unsigned int cPlayerSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
{
	if( fileName == 0 )
		return UINT_MAX;

	///
	cString pathName;
	pathName.Format( "./Data/Character/%s", fileName );
	if( RESOURCEMAN->LoadNIF( pathName ) == false )
		return UINT_MAX;

	///
	NiNode* n = RESOURCEMAN->GetObjectByName( fileName );
	if( n == 0 ) 
	{
		assert(0);
		return UINT_MAX;
	}

	n->UpdateProperties();
	n->UpdateEffects();
	n->Update(0.0f);


	return cDynamicSceneNode::LinkObject( dummyIdx, n );
}

NiAVObject* cPlayerSceneNode::GetWeaponDummy( unsigned int objectIdx, bool left )
{
	NiAVObject* pObj = GetLinkObject( objectIdx );
	if( pObj == 0 )
		return 0;

	unsigned int dummyIdx = (left == true)? eLINK_LWEAPON:eLINK_RWEAPON;
	const char* dummyName = RESOURCEMAN->GetManagedDummyName( dummyIdx );
	if( strlen( dummyName ) == 0 )
		return 0;

	return pObj->GetObjectByName( dummyName );
}

cObjectNameCard* cPlayerSceneNode::CreateNameCard( cBaseObject* pObj )
{
// 	if( !pObj )
// 	{
// 		assert(0);
// 		return 0;
// 	}
// 	return new cPlayerNameCard( pObj );
	return 0;
}


void cPlayerSceneNode::AddDepthPlaneObject()
{
// 	bool showName = true;
// 	unsigned int optionHeight = 0;
// 
// 	/// 타롯 & 점집 간판 추가
// 	if( mpTarotBoard && mpTarotBoard->IsActive() )
// 	{
// 		SCENEMAN->AddPlane( mpTarotBoard );
// 
// 		mpTarotBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );
// 		optionHeight += mpTarotBoard->GetBoardHeight();
// 
// 		showName = false;
// 	}
// 	else if( mpUserSellBoard && mpUserSellBoard->IsActive() )
// 	{
// 		SCENEMAN->AddPlane( mpUserSellBoard );
// 		mpUserSellBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );
// 
// 		optionHeight += mpUserSellBoard->GetBoardHeight();
// 		showName = false;
// 	}
// 
// 	if( mpHead == 0 )
// 	{
// 		assert( 0 );
// 		return;
// 	}
// 
// 	NiPoint3 pos = this->GetWorldTranslate();
// 	pos.z = mpHead->GetWorldTranslate().z;
// 	pos.z += mHeadDist;
// 
// 	NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
// 
// 	float distCam = (pos - camPos).Length();
// 	float distHero = distCam;
// 	if( HERO )
// 	{
// 		distHero = (GetWorldTranslate() - HERO->GetPos()).Length();
// 	}
// 
// 	/// 말풍선 보이기 여부
// 	if( OPTIONMAN->IsShowBubble() )
// 	{
// 		if( mpChatBubble && mpChatBubble->IsActive() )
// 		{
// 			if( distCam <= 3000.0f)
// 			{
// 				/// 말풍선 등록
// 				SCENEMAN->AddPlane( mpChatBubble );
// 				mpChatBubble->SetPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5) );
// 
// 				optionHeight += mpChatBubble->GetBubbleHeight();
// 			}
// 		}
// 	}
// 
// 	/// NameCard
// 	if( mpNameCard && showName == true )
// 	{
// 		cBaseObject* gameObj = GetGameObject();
// 		if( distHero <= 1500 )
// 		{
// 			if( PARTYMAN->IsHeroParty( mGameObject.index ) == true || 
// 				PARTYUNIONMAN->IsHeroParty( mGameObject.index ) == true )
// 			{
// 				mpNameCard->SetEnableGauge( true );
// 			}
// 			else
// 			{
// 				mpNameCard->SetEnableGauge( false );
// 			}
// 			
// 
// //			if( mShowNameGauge )
// //			{
// //				if( PARTYMAN->IsHeroParty( mGameObject.index ) == true )
// //				{
// //					mpNameCard->SetEnableGauge( true );
// ////					if( distHero > 500.0f )
// ////					{
// ////						if( HERO->GetTargetObject() != gameObj )
// ////							mpNameCard->SetEnableGauge( false );
// ////					}
// //				}
// //				else
// //				{
// //					mpNameCard->SetEnableGauge( false );
// //				}
// //			}
// //			else
// //			{
// //				if( PARTYMAN->IsHeroParty( mGameObject.index ) == true )
// //					mpNameCard->SetEnableGauge( true );
// //				else
// //					mpNameCard->SetEnableGauge( false );
// //			}
// 
// 			if( gameObj == 0 || gameObj->IsDie() )
// 				mpNameCard->SetEnableGauge( false );
// 
// 			mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
// 			SCENEMAN->AddPlane( mpNameCard );
// 		}
// 		else
// 		{
// 			mpNameCard->SetEnableGauge( false );
// 
// 			/// over인 경우
// 			cBaseObject* o = OBJECTMAN->GetOverObject();
// 			if( o == gameObj && distHero <= 5000 )
// 			{
// 				mpNameCard->SetEnableGauge( false );
// 
// 				mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
// 				SCENEMAN->AddPlane( mpNameCard );
// 			}
// 		}
// 	}
}

bool cPlayerSceneNode::PickManagedPlane( float mouseX, float mouseY )
{
// 	if( mpTarotBoard && mpTarotBoard->PickPoint( mouseX, mouseY ) == true )
// 		return true;
// 
// 	if( mpUserSellBoard && mpUserSellBoard->PickPoint( mouseX, mouseY ) == true )
// 		return true;

	return false;
}


void cPlayerSceneNode::ActiveTarotBubble( LPTSTR title, unsigned long color )
{
// 	cBaseObject* gameObj = GetGameObject();
// 	if( gameObj == 0 )
// 	{
// 		if( mpTarotBoard )
// 			mpTarotBoard->Active( _T("Error!!"), title, color, true );
// 	}
// 	else
// 	{
// 		if( mpTarotBoard )
// 			mpTarotBoard->Active( gameObj->GetName(), title, color, true );
// 	}
}

void cPlayerSceneNode::DeActiveTarotBubble()
{
// 	if( mpTarotBoard )
// 		mpTarotBoard->DeActive();
}

void cPlayerSceneNode::ActiveUserSellBubble( LPTSTR title, unsigned long color )
{
// 	cBaseObject* gameObj = GetGameObject();
// 	if( gameObj == 0 )
// 	{
// 		if( mpUserSellBoard )
// 			mpUserSellBoard->Active( _T("Error!!"), title, color, true );
// 	}
// 	else
// 	{
// 		if( mpUserSellBoard )
// 			mpUserSellBoard->Active( gameObj->GetName(), title, color, true );
// 	}
}

void cPlayerSceneNode::DeActiveUserSellBubble()
{
// 	if( mpUserSellBoard )
// 		mpUserSellBoard->DeActive();
}

void cPlayerSceneNode::UpdateLinkEffect()
{
	if( mEffectList.IsEmpty() )
		return;

	cLinkIndexList::cIterator i = mEffectList.Begin();
	cLinkIndexList::cIterator end = mEffectList.End();
	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)*i;
		cEffectSceneNode* n = SCENEMAN->GetEffectSceneNode( idx );
		if( n )
		{
			unsigned int dummyIdx = n->GetDummyIdx();

			if( dummyIdx != eLINK_RWEAPON && dummyIdx != eLINK_LWEAPON )
				continue;

			NiNode* parent = n->GetNiObj()->GetParent();
			NiNode* check = GetDummyObject( dummyIdx );
			if( check )
			{
				if( check != parent )
				{
					/// new dummy
					if( parent )
						parent->DetachChild( n->GetNiObj() );
					else
						n->GetNiObj()->DetachParent();

					check->AttachChild( n->GetNiObj() );
				}
			}
			else
			{
				/// hand dummy check
				if( dummyIdx == eLINK_RWEAPON )
					check = GetDummyObject( eLINK_RHAND );
				else
					check = GetDummyObject( eLINK_LHAND );

				assert(check);
				if( check )
				{
					if( check != parent )
					{
						if( parent )
							parent->DetachChild( n->GetNiObj() );
						else
							n->GetNiObj()->DetachParent();

						check->AttachChild( n->GetNiObj() );
					}
				}
			}
		}
	}

}

bool cPlayerSceneNode::OnVisibleVehicle( bool update, NiPoint3 pos )
{
// 	if( SCENEMAN->IsScreenShotMode() == true )
// 		return false;
// 
// 	if( mAlpha < 0.00001f )
// 		return false;
// 
// 	/// 그림자
// 	cBaseObject* gameObj = GetGameObject();
// 	if( gameObj )
// 	{
// 		if( gameObj->IsDie() == false && mShadowGeom )
// 		{
// 			if( mAlpha > 0.00001f )
// 				mShadowGeom->Process( update, pos, mAlpha );
// 
// 			mNeedUpdateShadow = false;
// 		}
// 	}
	return cSceneNode::OnVisible();
}

void cPlayerSceneNode::HideEar( bool hide )
{
	NiAVObject* obj = GetPartObject( ePART_EAR );
	if( obj )
		obj->SetAppCulled( hide );
}

//////////////////////////////////////////////////////////////////////////
/// Hero SceneNode


cHeroSceneNode::cHeroSceneNode( eType type )
: cPlayerSceneNode( type )
, mpQuestPlane(0)
, mpFieldTargetEffect(0)
{
	mAlphaSpeed = 4.0f;
	mFieldEffectOn = false;
	mAlpha = 1.0f;
}

cHeroSceneNode::~cHeroSceneNode()
{
	ClearCollectInfo();

	SAFE_DELETE( mpFieldTargetEffect );
//	SAFE_DELETE( mpQuestPlane );
}

bool cHeroSceneNode::Init( const cPlayerSceneNodeParam& param )
{
	if( cPlayerSceneNode::Init( param ) == false )
		return false;

// 	mpQuestPlane = new cCenterMsgPlane;
// 	if( mpQuestPlane == 0 )
// 	{
// 		assert(mpQuestPlane);
// 		return false;
// 	}

	mpFieldTargetEffect = new cShadowGeometry();
	if( mpFieldTargetEffect->Init( RESOURCEMAN->GetFieldTargetTexture(), 1000.0f, 2.0f, false ) == false )
	{
		assert(mpFieldTargetEffect);
		return false;
	}

	mAlpha = 1.0f;
	mTargetAlpha = 1.0f;

	return true;
}


void cHeroSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
// 	cPlayerSceneNode::OnProcess( deltaTime, accumTime );
// 
// 	///
// 	if( mpQuestPlane )
// 		mpQuestPlane->Update( deltaTime );
}

void cHeroSceneNode::ActiveQuestText( LPCTSTR msg, unsigned long color )
{
// 	if( mpQuestPlane == 0 )
// 		return;
// 
// 	if( mpHead == 0 )
// 	{
// 		assert(0);
// 		return;
// 	}
// 
// 	cBaseObject* gameObj = GetGameObject();
// 	if( gameObj == 0 )
// 	{
// 		assert(0);
// 		return;
// 	}
// 
// 	NiPoint3 pos = mpFoot->GetWorldTranslate();
// 	pos.z += gameObj->GetStatureValue() + mHeadDist;
// 
// 	mpQuestPlane->ActiveMsg( msg, pos, color );
}

void cHeroSceneNode::AddToVisibleArray()
{
// 	if( SCENEMAN->IsScreenShotMode() == true )
// 		return;

	cDynamicSceneNode::AddToVisibleArray();
}

void cHeroSceneNode::AddDepthPlaneObject()
{
// 	if( mpQuestPlane && mpQuestPlane->IsActive() )
// 	{
// 		SCENEMAN->AddPlane( mpQuestPlane );
// 	}
// 
// 	///
// 	bool showName = true;
// 	unsigned int optionHeight = 0;
// 
// 	/// 타롯 & 노점 간판 추가
// 	if( mpTarotBoard && mpTarotBoard->IsActive() )
// 	{
// 		SCENEMAN->AddPlane( mpTarotBoard );
// 
// 		mpTarotBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );
// 		optionHeight += mpTarotBoard->GetBoardHeight()+5;
// 
// 		showName = false;
// 	}
// 	else if( mpUserSellBoard && mpUserSellBoard->IsActive() )
// 	{
// 		SCENEMAN->AddPlane( mpUserSellBoard );
// 		mpUserSellBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );
// 
// 		optionHeight += mpUserSellBoard->GetBoardHeight()+5;
// 		showName = false;
// 	}
// 
// 	NiPoint3 pos = this->GetWorldTranslate();
// 	pos.z = mpHead->GetWorldTranslate().z;
// 	pos.z += mHeadDist;
// 
// 	//NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
// 	//float distCam = (pos - camPos).Length();
// 
// 	/// 말풍선 보이기 여부
// 	if( OPTIONMAN->IsShowBubble() )
// 	{
// 		if( mpChatBubble && mpChatBubble->IsActive() )
// 		{
// 			/// 말풍선 등록
// 			SCENEMAN->AddPlane( mpChatBubble );
// 			mpChatBubble->SetPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5) );
// 
// 			optionHeight += mpChatBubble->GetBubbleHeight()+5;
// 		}
// 	}
// 
// 	cBaseObject* gameObj = GetGameObject();
// 	if( STAGEMAN->GetCurrentStage() == eStage_Select )
// 	{
// 		if( mpNameCard && showName == true )
// 		{
// 			if( mShowNameGauge )
// 				mpNameCard->SetEnableGauge( true );
// 			else
// 				mpNameCard->SetEnableGauge( false );
// 
// 			if( gameObj == 0 || gameObj->IsDie() )
// 				mpNameCard->SetEnableGauge( false );
// 
// 			mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
// 			SCENEMAN->AddPlane( mpNameCard );
// 		}
// 	}
// 	else
// 	{
// 		if( OPTIONMAN->IsShowHeroNameCard() || OPTIONMAN->IsShowGauge() )
// 		{
// 			if( mpNameCard && showName == true )
// 			{
// 				if( mShowNameGauge )
// 					mpNameCard->SetEnableGauge( true );
// 				else
// 					mpNameCard->SetEnableGauge( false );
// 
// 				if( gameObj == 0 || gameObj->IsDie() )
// 					mpNameCard->SetEnableGauge( false );
// 
// 				mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
// 				SCENEMAN->AddPlane( mpNameCard );
// 			}
// 		}
// 	}
}

void cHeroSceneNode::SetFieldEffRadius( float r )
{
	if( mpFieldTargetEffect == 0 )
		return;
	mpFieldTargetEffect->SetRadius( r );
}

void cHeroSceneNode::UpdateFieldTargetEffect( NiPoint3 center )
{
	if( mpFieldTargetEffect == 0 )
		return;

	mFieldEffectOn = true;
	mFieldEffectCenter = center;
}

bool cHeroSceneNode::OnVisible()
{
	bool check = cPlayerSceneNode::OnVisible();

	if( check && mFieldEffectOn )
	{
		mFieldEffectOn = false;
		mpFieldTargetEffect->Process( true, mFieldEffectCenter, 0.9f );
	}
	return check;
}