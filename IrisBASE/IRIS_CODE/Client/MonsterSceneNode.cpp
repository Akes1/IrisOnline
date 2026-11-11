#include "StdAfx.h"
#include ".\monsterscenenode.h"
#include "ResourceManager.h"

#include "ObjectNameCard.h"
#include "SceneManager.h"
#include "cameraManager.h"
#include "OptionManager.h"
#include "RenderSystem.h"

#include "ChatBubble.h"

#include "ObjectManager.h"
#include "Hero.h"

#include "application.h"
#include "NifAnimationInfo.h"
#include "Signboard.h"

cMonsterSceneNode::cMonsterSceneNode( eType type )
: cDynamicSceneNode( type )
{
	mpTutorialBoard = 0;
}

cMonsterSceneNode::~cMonsterSceneNode()
{
	SAFE_DELETE( mpTutorialBoard );

	ClearCollectInfo();
}

bool cMonsterSceneNode::Init( const cMonsterSceneNodeParam& param )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHeadDist = param.mHeadHeight;

	/// 말풍선용 객체 생성
	mpChatBubble = new cFuncChatBubble;

	mpTutorialBoard = new cTutorialSignboard( eOBJECTTYPE_MONSTER, param.mMonClassIndex, param.mpObject->GetObjectID() );
	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	InitLinkInfo()
* 목    적 :	링크정보를 저장할 메모리를 초기화 한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cMonsterSceneNode::InitLinkInfo()
{
	mpLinkInfo = NiNew NiTPrimitiveArray<sLinkInfo*>(10, 2);
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
void cMonsterSceneNode::InitializeRandomIdle()
{
	mpRandomIdle = NiNew NiTPrimitiveArray<cActorManagerForPartition::EventCode>(1);
	mpRandomIdle->SetAt(0, 0);
}

/// 061016 PKH 
/* ------------------------------------------------------------------
* 함수이름 :	GetObjectCenter()
* 목    적 :	객체의 중심좌표를 제공한다.
* 주의사항 :	플레이어는 Spine더미의 좌표를 중심좌표로 사용한다.
* ------------------------------------------------------------------ */
const NiPoint3& cMonsterSceneNode::GetObjectCenter()
{
	NiAVObject* spine = GetDummyObject( eLINK_BODY );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cMonsterSceneNode::GetObjectFoot()
{
	NiAVObject* spine = GetDummyObject( eLINK_FOOT );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

const NiPoint3& cMonsterSceneNode::GetObjectHead()
{
	NiAVObject* spine = GetDummyObject( eLINK_HEAD );

	if( spine )	
		return spine->GetWorldTranslate();
	else
		return NiPoint3::ZERO;
}

/// 061013 PKH 
unsigned int cMonsterSceneNode::LinkObject( unsigned int dummyIdx, const char* fileName )
{
	NiNode* pNode = RESOURCEMAN->GetObjectByName( fileName );
	if( !pNode ) 
	{
		assert(0);
		return UINT_MAX;
	}

	return cDynamicSceneNode::LinkObject( dummyIdx, pNode );
}

cEffectSceneNode* cMonsterSceneNode::LinkEffect( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool bLoop, bool bFollow )
{
	if( dummyIdx == eLINK_RHAND || dummyIdx == eLINK_LHAND )
	{
		if( GetDummyObject( dummyIdx ) == 0 )
			return 0;
	}
	return cDynamicSceneNode::LinkEffect( dummyIdx, fileName, trans, bLoop, bFollow );
}

cEffectSceneNode* cMonsterSceneNode::LinkDamage( unsigned int dummyIdx, const char* fileName, NiTransform* trans, bool IsMiss, bool zFalse )
{
	if( dummyIdx == eLINK_RHAND || dummyIdx == eLINK_LHAND )
	{
		if( GetDummyObject( dummyIdx ) == 0 )
			return 0;
	}
	return cDynamicSceneNode::LinkDamage( dummyIdx, fileName, trans, IsMiss, zFalse );
}

///
cObjectNameCard* cMonsterSceneNode::CreateNameCard( cBaseObject* pObj )
{
	if( !pObj )
	{
		assert(0);
		return 0;
	}
	return new cMonsterNameCard( pObj );
}

///
void cMonsterSceneNode::AddDepthPlaneObject()
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

	if( mpNameCard && (OPTIONMAN->IsShowMonsterNameCard() || OPTIONMAN->IsShowGauge()) )
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
			mpNameCard->SetEnableGauge( false );

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

void cMonsterSceneNode::UpdateTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->UpdateImage();
}

void cMonsterSceneNode::ClearTutorialBoard()
{
	if( mpTutorialBoard )
		mpTutorialBoard->ClearImage();
}