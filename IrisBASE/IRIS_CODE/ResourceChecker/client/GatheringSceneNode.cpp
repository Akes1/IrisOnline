#include "StdAfx.h"
#include "gatheringscenenode.h"

#include "GatheringScript.h"
#include "Gathering_common.h"

//#include "ObjectNameCard.h"
// #include "ObjectManager.h"
// #include "Hero.h"
// #include "ItemManager.h"
// #include "QuestManager.h"
// #include "CameraManager.h"
#include "RenderSystem.h"
// #include "PVPManager.h"

#include "GameResourceManager.h"
#include "SceneManager.h"
#include "NaviMesh.h"
#include "NifAnimationInfo.h"

#include "EffectSceneNode.h"

#include "ResourceManager.h"
#include "ShadowGeometry.h"

const unsigned int EFFECT_CHECK_TIME = 1000;

cGatheringSceneNode::cGatheringSceneNode( eType type )
: cManagedStaticSceneNode( type )
, mObjectIdx(0)
, mClassIdx(0)
, mQuestIdx(0)
, mRemoveFlag(false)
, mpNameCard(0)
, mShadowGeom(0)
, mNeedUpdateShadow(false)
{
	mNameHeight = 200.0f;

	mEnableGatheringEffect = 0;
	mEffectIdxByManager = (unsigned long)-1;

	mEffectCheckTimer = 0;
}

cGatheringSceneNode::~cGatheringSceneNode()
{
	ClearCollectInfo();

	ClearEffect();

//	SAFE_DELETE(mpNameCard);

	SAFE_DELETE( mShadowGeom );
}

bool cGatheringSceneNode::Init( cGatheringSceneNodeParam& param )
{
	if( param.mObjectIdx == 0 )
	{
		assert(0);
		return false;
	}

	/// 아이템 정보들을 받는다.
	mObjectIdx = param.mObjectIdx;
	mClassIdx = param.mClassIdx;

	float height = 0.0f;
	NAVIMESH->CalcHeight( &height, param.mTranslate.x, param.mTranslate.y );
	param.mTranslate.z += height;

	if( cManagedStaticSceneNode::Init( param ) == false )
		return false;

	/// create name card
	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( mClassIdx );
	assert(info);

	if( info )
	{
// 		mpNameCard = new cNameCard;
// 		mpNameCard->SetName( (LPTSTR)GAMERESOURCEMAN->GetGatheringName( info->mNameIdx ), 0xFFFFFFFF );

		mNameHeight = (float)info->mNameHeight;

		mQuestIdx = info->mNeedQuestIdx;
	}

	mRemoveFlag = false;

	mAlpha = 0.0f;
	SetAlphaBlended( 1.0f );

// 	if( HERO && HERO->CheckGatheringEnable( mClassIdx ) == true )
// 		AddEffect();

	mShadowGeom = new cShadowGeometry();
	if( mShadowGeom->Init( RESOURCEMAN->GetShadowTexture(), GetRadius() * 0.8f ) == false )
	{
		assert( 0 );
		return false;
	}

	mNeedUpdateShadow = true;

	return true;
}

void cGatheringSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
// 	/// gathering 객체의 채집 가능여부 체크 (1초단위)
// 	if( mEffectCheckTimer >= EFFECT_CHECK_TIME)
// 	{
// 		if( mRemoveFlag == false )
// 		{
// 			if( HERO && HERO->CheckGatheringEnable( mClassIdx ) == true )
// 				AddEffect();
// 			else
// 				ClearEffect();
// 		}
// 
// 		mEffectCheckTimer = 0;
// 	}
// 	else
// 	{
// 		mEffectCheckTimer += deltaTime;
// 	}
// 
// 	ProcessAlpha( deltaTime );
// 
// 	/// 반복 동작을 가진 경우 엔진 노드를 처리
// 	if( mNifAniInfo )
// 	{
// 		mNeedUpdateTransform = true;
// 		cSceneNode::OnProcess( deltaTime, accumTime );
// 
// 		mNifAniInfo->Update( deltaTime, accumTime );
// 	}
// //	else
// //	{
// //		mNeedUpdateTransform = true;
// //		cSceneNode::OnProcess( deltaTime, accumTime );
// //	}
// 
// 	if( mAlpha < 0.000001f )
// 	{
// 		mCheckFrustum = false;
// 
// 		if( mRemoveFlag == true )
// 			SCENEMAN->AddDeleteGatheringNode( this );
// 	}
// 	else
// 		mCheckFrustum = true;
}

void cGatheringSceneNode::AddToVisibleArray()
{
// 	if( IsViewNode() == false )
// 		return;
// 
// 	cSceneNode::AddToVisibleArray();
// 
// 	if( mpNameCard )
// 	{
// 		NiPoint3 pos = this->GetWorldTranslate();
// 		pos.z += mNameHeight;
// 
// 		if( mIsMouseOver )
// 		{
// 			unsigned long color = 0xFFFFFFFF;
// 			if( mEnableGatheringEffect == 0 )
// 				color = 0xFFF03719;
// 
// 			mpNameCard->SetNameColor( color );
// 
// 			float x,y;
// 			cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y, false );
// 			int ScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
// 			int ScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );
// 
// 			mpNameCard->Set2DPos( ScreenPosX, ScreenPosY, 0, 0 );
// 			SCENEMAN->AddPlane( mpNameCard );
// 		}
// 	}
}

bool cGatheringSceneNode::IsEnableQuest()
{
// 	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( mClassIdx );
// 	if( info == 0 )
// 		return false;
// 
// 	if( mQuestIdx )
// 	{
// 		if( QUESTMAN->IsCompleteGathering( mQuestIdx ) == true )
// 			return false;
// 	}
	return true;
}

void cGatheringSceneNode::AddEffect()
{
	if( mEnableGatheringEffect != 0 )
		return;

	sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( mClassIdx );
	if( info == 0 )
	{
		assert(0);
		return;
	}

	/// create effect node
	NiTransform trans;
	trans.MakeIdentity();
	trans.m_Translate = GetWorldTranslate();

	cString str;
	str.Format("./data/effect/%s", info->mEffectFileName.Cstr() );

	mEnableGatheringEffect = SCENEMAN->CreateSelfEffect( str.Cstr(), true, trans, true );
	if( mEnableGatheringEffect )
		mEffectIdxByManager = mEnableGatheringEffect->GetIndexByManger();
}

void cGatheringSceneNode::ClearEffect()
{
	if( mEffectIdxByManager != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdxByManager );
		if( node )
		{
			assert( node == mEnableGatheringEffect );
			node->Remove();
		}
		mEnableGatheringEffect = 0;
		mEffectIdxByManager = (unsigned long)-1;
	}
}

void cGatheringSceneNode::Remove()
{ 
	mRemoveFlag = true; 
	SetAlphaBlended(0.0f); 

	ClearEffect();
}

bool cGatheringSceneNode::OnVisible()
{
// 	if( SCENEMAN->IsScreenShotMode() == true )
// 		return false;
// 
// 	if( mAlpha < 0.00001f )
// 		return false;
// 
// 	/// 그림자
// 	if( mShadowGeom )
// 	{
// 		if( mAlpha > 0.00001f )
// 			mShadowGeom->Process( mNeedUpdateShadow, GetWorldTranslate(), mAlpha );
// 
// 		mNeedUpdateShadow = false;
// 	}
// 
 	return cManagedStaticSceneNode::OnVisible();
}

bool cGatheringSceneNode::Pick( const cRay& ray )
{
// 	if( PVPMAN && PVPMAN->IsEnableBattle() == true )
// 	{
// 		sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( mClassIdx );
// 		if( info && info->mNeedBuffIdx != 0 )
// 		{
// 			if( HERO && HERO->IsBuffHave( info->mNeedBuffIdx ) == false )
// 				return false;
// 		}
// 	}

	return cManagedStaticSceneNode::Pick( ray );
}
