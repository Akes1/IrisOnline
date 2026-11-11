#include "StdAfx.h"
#include "transmonscenenode.h"

#include "ResourceManager.h"
#include "SceneManager.h"
#include "cameraManager.h"
#include "OptionManager.h"

#include "BaseObject.h"

#include "ObjectNameCard.h"
#include "ChatBubble.h"
#include "CenterMsgPlane.h"
#include "Signboard.h"

#include "ShadowGeometry.h"



cTransMonSceneNode::cTransMonSceneNode( eType type )
: cMonsterSceneNode( type )
, mpTarotBoard(0)
, mpUserSellBoard(0)
, mpQuestPlane(0)
, mpFieldTargetEffect(0)
{
	mFieldEffectOn = false;
	mIsHero = false;
	mFieldEffectCenter = NiPoint3::ZERO;
}

cTransMonSceneNode::~cTransMonSceneNode()
{
	ClearCollectInfo();

	SAFE_DELETE( mpFieldTargetEffect );
	SAFE_DELETE( mpQuestPlane );
	SAFE_DELETE( mpUserSellBoard );
	SAFE_DELETE( mpTarotBoard );
}

bool cTransMonSceneNode::Init( const cMonsterSceneNodeParam& param, bool hero )
{
	if( cDynamicSceneNode::Init( param ) == false )
	{
		return false;
	}

	mHeadDist = param.mHeadHeight;

	/// 말풍선용 객체 생성
	mpTarotBoard = new cTarotSignboard;
	if( mpTarotBoard == 0 )
	{
		assert(0);
		return false;
	}

	mpUserSellBoard = new cUseSellSignboard;
	if( mpUserSellBoard == 0 )
	{
		assert(0);
		return false;
	}

	mpChatBubble = new cChatBubble;
	if( mpChatBubble == 0 )
	{
		assert(0);
		return false;
	}

	mIsHero = hero;

	if( mIsHero )
	{
		mpQuestPlane = new cCenterMsgPlane;
		if( mpQuestPlane == 0 )
		{
			assert(0);
			return false;
		}

		mpFieldTargetEffect = new cShadowGeometry();
		if( mpFieldTargetEffect->Init( RESOURCEMAN->GetFieldTargetTexture(), 1000.0f, 2.0f, false ) == false )
		{
			assert(mpFieldTargetEffect);
			return false;
		}
	}

	return true;
}

void cTransMonSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cMonsterSceneNode::OnProcess( deltaTime, accumTime );

	if( mpQuestPlane )
		mpQuestPlane->Update( deltaTime );
}

void cTransMonSceneNode::AddToVisibleArray()
{
	if( SCENEMAN->IsScreenShotMode() == true )
		return;

	cDynamicSceneNode::AddToVisibleArray();
}

void cTransMonSceneNode::AddDepthPlaneObject()
{
	if( mpQuestPlane && mpQuestPlane->IsActive() )
	{
		SCENEMAN->AddPlane( mpQuestPlane );
	}

	bool showName = true;
	unsigned int optionHeight = 0;

	/// 타롯 & 노점 간판 추가
	if( mpTarotBoard && mpTarotBoard->IsActive() )
	{
		SCENEMAN->AddPlane( mpTarotBoard );

		mpTarotBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );
		optionHeight += mpTarotBoard->GetBoardHeight()+5;

		showName = false;
	}
	else if( mpUserSellBoard && mpUserSellBoard->IsActive() )
	{
		SCENEMAN->AddPlane( mpUserSellBoard );

		mpUserSellBoard->SetPos( mHeadScreenPosX, mHeadScreenPosY );

		optionHeight += mpUserSellBoard->GetBoardHeight()+5;
		showName = false;
	}

	NiPoint3 pos = this->GetWorldTranslate();
	pos.z = mpHead->GetWorldTranslate().z;
	pos.z += mHeadDist;

	NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
	float distCam = (pos - camPos).Length();

	/// 말풍선 보이기 여부
	if( OPTIONMAN->IsShowBubble() )
	{
		if( mpChatBubble && mpChatBubble->IsActive() )
		{
			/// 말풍선 등록
			SCENEMAN->AddPlane( mpChatBubble );
			mpChatBubble->SetPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5) );

			optionHeight += mpChatBubble->GetBubbleHeight()+5;
		}
	}

	if( mpNameCard && showName == true )
	{
		if( distCam < 1500.0f )
			distCam = 1500.0f;
		if( distCam > 10000.0f )
			distCam = 10000.0f;

		if( mShowNameGauge )
			mpNameCard->SetEnableGauge( true );
		else
			mpNameCard->SetEnableGauge( false );

		mpNameCard->Set2DPos( mHeadScreenPosX, mHeadScreenPosY - (optionHeight+5), mFootScreenPosX, mFootScreenPosY+20 );
		SCENEMAN->AddPlane( mpNameCard );
	}
}

bool cTransMonSceneNode::PickManagedPlane( float mouseX, float mouseY )
{
	if( mIsHero == true )
		return false;

	if( mpTarotBoard && mpTarotBoard->PickPoint( mouseX, mouseY ) == true )
		return true;

	if( mpUserSellBoard && mpUserSellBoard->PickPoint( mouseX, mouseY ) == true )
		return true;

	return false;
}


void cTransMonSceneNode::ActiveQuestText( LPCTSTR msg, unsigned long color )
{
	if( mpQuestPlane == 0 )
		return;

	if( mpHead == 0 || mpFoot == 0 )
	{
		assert(0);
		return;
	}

	cBaseObject* gameObj = GetGameObject();
	if( gameObj == 0 )
	{
		assert(0);
		return;
	}

	NiPoint3 pos = mpFoot->GetWorldTranslate();
	pos.z += gameObj->GetStatureValue() + mHeadDist;

	mpQuestPlane->ActiveMsg( msg, pos, color );
}

void cTransMonSceneNode::ActiveTarotBubble( LPTSTR title, unsigned long color )
{
	cBaseObject* gameObj = GetGameObject();
	if( gameObj == 0 )
	{
		if( mpTarotBoard )
			mpTarotBoard->Active( _T("Error!!"), title, color, true );
	}
	else
	{
		if( mpTarotBoard )
			mpTarotBoard->Active( gameObj->GetName(), title, color, true );
	}
}

void cTransMonSceneNode::DeActiveTarotBubble()
{
	if( mpTarotBoard )
		mpTarotBoard->DeActive();
}

void cTransMonSceneNode::ActiveUserSellBubble( LPTSTR title, unsigned long color )
{
	cBaseObject* gameObj = GetGameObject();
	if( gameObj == 0 )
	{
		if( mpUserSellBoard )
			mpUserSellBoard->Active( _T("Error!!"), title, color, true );
	}
	else
	{
		if( mpUserSellBoard )
			mpUserSellBoard->Active( gameObj->GetName(), title, color, true );
	}
}

void cTransMonSceneNode::DeActiveUserSellBubble()
{
	if( mpUserSellBoard )
		mpUserSellBoard->DeActive();
}

cObjectNameCard* cTransMonSceneNode::CreateNameCard( cBaseObject* pObj )
{
	if( !pObj )
	{
		assert(0);
		return 0;
	}
	return new cPlayerNameCard( pObj );
}

void cTransMonSceneNode::SetFieldEffRadius( float r )
{
	if( mpFieldTargetEffect == 0 )
		return;
	mpFieldTargetEffect->SetRadius( r );
}

void cTransMonSceneNode::UpdateFieldTargetEffect( NiPoint3 center )
{
	if( mpFieldTargetEffect == 0 )
		return;

	mFieldEffectOn = true;
	mFieldEffectCenter = center;
}

bool cTransMonSceneNode::OnVisible()
{
	bool check = cMonsterSceneNode::OnVisible();

	if( check && mFieldEffectOn )
	{
		mFieldEffectOn = false;
		mpFieldTargetEffect->Process( true, mFieldEffectCenter, 0.9f );
	}
	return check;
}
