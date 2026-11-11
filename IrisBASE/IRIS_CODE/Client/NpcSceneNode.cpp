#include "StdAfx.h"
#include ".\npcscenenode.h"

#include "ObjectNameCard.h"

#include "ObjectManager.h"
#include "Hero.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "cameraManager.h"
#include "OptionManager.h"
#include "RenderSystem.h"

#include "ChatBubble.h"
#include "BaseObject.h"
#include "ShadowGeometry.h"

#include "LightAgent.h"
#include "FogAgent.h"
#include "Signboard.h"

#include "application.h"
#include "NifAnimationInfo.h"
#include "Npc.h"

cNpcSceneNode::cNpcSceneNode( eType type )
: cDynamicSceneNode( type )
{
	mpSignBoard = 0;
	mpTutorialBoard = 0;
	mIsFadeOutProcess = false;
}

cNpcSceneNode::~cNpcSceneNode()
{
	ClearCollectInfo();

	SAFE_DELETE( mpTutorialBoard );
	SAFE_DELETE( mpSignBoard );
}

bool cNpcSceneNode::Init( const cNpcSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mpSignBoard = new cNpcSignboard( param.mNpcClassIndex );
	mpTutorialBoard = new cTutorialSignboard( eOBJECTTYPE_NPC, param.mNpcClassIndex, param.mpObject->GetObjectID() );
	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cNpcSceneNode::InitLinkInfo()
{
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(2, 1);
	assert(mpLinkInfo);
	if( mpLinkInfo == NULL ) 
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
void cNpcSceneNode::InitializeRandomIdle()
{
	mpRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mpRandomIdle->SetAt(0, 0);
}

///
cObjectNameCard* cNpcSceneNode::CreateNameCard( cBaseObject* pObj )
{
	if( !pObj )
	{
		assert(0);
		return 0;
	}
	return new cNpcNameCard( pObj );
}

const NiPoint3& cNpcSceneNode::GetObjectHead()
{
	NiAVObject* spine = GetDummyObject( eLINK_HEAD );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cNpcSceneNode::GetObjectFoot()
{
	NiAVObject* spine = GetDummyObject( eLINK_FOOT );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cNpcSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetDummyObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

void cNpcSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mSetVisibleArray == false )
	{
		cBaseObject* gameObj = GetGameObject();
		if( gameObj && gameObj->GetObjectType() == eOBJECTTYPE_NPC )
		{
			cNpc* npc = (cNpc*)gameObj;
			npc->HideTalkWindow();
		}
	}
	
	cDynamicSceneNode::OnProcess( deltaTime, accumTime );

	if( mpTutorialBoard )
		mpTutorialBoard->Update( accumTime );
}

void cNpcSceneNode::AddDepthPlaneObject()
{
	if( mpHead == 0 )
	{
		assert( 0 );
		return;
	}

	if( mIsFadeOutProcess == true )
		return;

	NiPoint3 pos = this->GetWorldTranslate();
	pos.z = mpHead->GetWorldTranslate().z;
	pos.z += mHeadDist;

	NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
	float distCam = (pos - camPos).Length();
	float distHero = distCam;
	if( HERO )
		distHero = (GetWorldTranslate() - HERO->GetPos()).Length();

	int optionHeight = 0;
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

	bool showTalkButton = false;

	if( mpNameCard && OPTIONMAN->IsShowNPCNameCard() )
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

				// 
				if( distHero <= 800.0f )
				{
					int x = mHeadScreenPosX;
					int y = mHeadScreenPosY - (optionHeight+35);
					if( (x >= 0 && x < (int)RENDERSYS->GetScreenWidth()) && (y >= 0 && y < (int)RENDERSYS->GetScreenHeight()) )
					{
						showTalkButton = true;
					}
				}

				if( mpTutorialBoard && mpTutorialBoard->IsActive() )
				{
					if( gameObj && gameObj == HERO->GetTargetObject() )
					{
						SCENEMAN->AddPlane( mpTutorialBoard );
						mpTutorialBoard->SetPos( mHeadScreenPosX + mpTutorialBoard->GetPosX(), mHeadScreenPosY + mpTutorialBoard->GetPosY() );
					}
					else
					{
						if( distHero <= 400.0f )
						{
							SCENEMAN->AddPlane( mpTutorialBoard );
							mpTutorialBoard->SetPos( mHeadScreenPosX + mpTutorialBoard->GetPosX(), mHeadScreenPosY + mpTutorialBoard->GetPosY() );
						}
					}
				}
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

	cBaseObject* gameObj = GetGameObject();
	if( gameObj && gameObj->GetObjectType() == eOBJECTTYPE_NPC )
	{
		cNpc* npc = (cNpc*)gameObj;

		if( showTalkButton == true )
			npc->ShowTalkWindow( mHeadScreenPosX,  mHeadScreenPosY - (optionHeight+35) );
		else
			npc->HideTalkWindow();
	}
}

bool cNpcSceneNode::Pick( const cRay& ray )
{
	if( mIsFadeOutProcess == true )
		return false;

	return cDynamicSceneNode::Pick( ray );
}


void cNpcSceneNode::SetAlphaBlended( float targetAlpha )
{
	if( mIsFadeOutProcess == true )
		return;

	cDynamicSceneNode::SetAlphaBlended( targetAlpha );
}

void cNpcSceneNode::FadeOut()
{
	SetAlphaBlended( 0.0f );
	mIsFadeOutProcess = true;
}

bool cNpcSceneNode::IsFadeOutEnd()
{
	if( mIsFadeOutProcess == false )
		return false;

	if( mAlpha == 0.0f )
		return true;

	return false;
}

void cNpcSceneNode::UpdateTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->UpdateImage();
}

void cNpcSceneNode::ClearTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->ClearImage();
}

//////////////////////////////////////////////////////////////////////////
cRefereeSceneNode::cRefereeSceneNode( eType type )
: cDynamicSceneNode( type )
{
}

cRefereeSceneNode::~cRefereeSceneNode()
{
}

bool cRefereeSceneNode::Init( const cSceneNodeParam& param )
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

	/// 갱신
	mpActorManager->Update(0.0f);

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
		GetNiObj()->SetTranslate( param.mTranslate );
		GetNiObj()->SetRotate( param.mRotate );
		GetNiObj()->SetScale( param.mScale );

		///// 경계 구를 설정
		//NiMeshUpdateProcess kUpdateProcess;
		////kUpdateProcess.SetTime( THEAPP->GetWorldAccumTime()* 0.001f );
		//GetNiNode()->Update( kUpdateProcess );

		/// 갱신 여부를 설정
		mNeedUpdateTransform = true;
	}

	/// 알파 속성이 없으면 생성
	NiAlphaProperty* alphaProp = (NiAlphaProperty*)GetNiNode()->GetProperty( NiProperty::ALPHA );
	if( alphaProp == 0 )
	{
		/// 알파 속성이 없으면 생성
		if( alphaProp == 0 )
		{
			alphaProp = NiNew NiAlphaProperty;
			alphaProp->SetAlphaBlending( false );
			alphaProp->SetAlphaTesting( false );
			GetNiNode()->AttachProperty( alphaProp );
		}
	}


	NiAmbientLight* amb = FindAmbientLight( GetNiObj() );
	if( amb )
		GetNiNode()->DetachChild( amb );

	///
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

	mBoundSphere.Set( GetCenter(), GetRadius() );

	NiPoint3 camPos = CAMERAMAN->GetCurrentCameraPos();
	if( (camPos - GetWorldTranslate()).Length() > 5000.0f )
	{
		mTargetAlpha = 0.0f;
		mAlpha = 0.0f;
	}

	mLastUpdateTime = THEAPP->GetWorldAccumTime();

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cRefereeSceneNode::InitLinkInfo()
{
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(2, 1);
	assert(mpLinkInfo);
	if( mpLinkInfo == NULL ) 
	{
		return false;
	}

	for( unsigned int i=0; i<mpLinkInfo->GetAllocatedSize(); ++i )
	{
		mpLinkInfo->SetAt( i,0 );
	}

	return true;
}

void cRefereeSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	mSetVisibleArray = false;

	Interpret( accumTime );

	/// Alpha Fade In/Out 처리
	ProcessAlpha( deltaTime );

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;

	/// 경과 시간 * 속도 조절자
	unsigned long delta = accumTime - mLastUpdateTime;
	delta = (unsigned long)(delta * mScaleFactor);

	/// actormanager 전용 시간 계산
	mScaleAccumTime += delta;

	float fScaleTime = mScaleAccumTime*0.001f;

	//mNifAniInfo->Update( deltaTime, accumTime );

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
	mLastUpdateTime = accumTime;
}

void cRefereeSceneNode::Interpret( unsigned long accumTime )
{
	if( mpActorManager == 0 )
		return;

	unsigned int id = GetTargetAnimation();
	float endTime = mpActorManager->GetNextEventTime( cActorManagerForPartition::TEXT_KEY_EVENT, id, "end" );

	if( endTime == NI_INFINITY )
		return;
	if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
		endTime = GetTargetAnimationScaleEndTime();
	}

	float checkTime = (float)GetScaleAccumTime() * 0.001f;
	if( endTime > 0 && endTime <= checkTime )
	{
		if( id == 1 )
		{
			SetTargetAnimation( 2 );
		}
		else if( id == 3 )
		{
			SetAlphaBlended( 0.0f );
			LinkEffect( 1, "./Data/Effect/eff_duel_02.nif", NULL, false, true );
		}
	}
	else if( endTime == cActorManagerForPartition::INVALID_TIME )
	{
	}
}

void cRefereeSceneNode::AddToVisibleArray()
{
	if( IsViewNode() == false )
		return;

	cSceneNode::AddToVisibleArray();

	{
		/// 카메라와의 거리 등록
		NiPoint3 vec = GetWorldTranslate() - CAMERAMAN->GetCurrentCameraPos();
		mVisibleCamLength = vec.Length();

		/// visible dynamic array에 등록
		SCENEMAN->AddVisibleDynamicNode( this );
	}
}
