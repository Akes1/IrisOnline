#include "StdAfx.h"
#include ".\cardscenenode.h"

#include "SceneManager.h"
#include "NifAnimationInfo.h"
#include "EffectSceneNode.h"

#include "DisplayObject.h"

cCardSceneNode::cCardSceneNode( eType type )
: cStaticSceneNode( type )
, mDisplayObj(0)
{
	mEffect = 0;
	mEffectIdx = (unsigned long)-1;

	mCurrentPosIdx = 0;
	mMoveState = eCard_Stop;
	mNifTimeManager = 0;

	mDummy = 0;

	mCardUpdate = false;
	mCardGotoTime = 0;
	mGotoDeltaTime = 0;

	Stop();
}

cCardSceneNode::~cCardSceneNode()
{
	mDisplayObj = 0;

	ClearCollectInfo();

	DestroyEffect();
	SAFE_DELETE( mNifTimeManager )
}

void cCardSceneNode::DestroyEffect()
{
	if( mEffectIdx != (unsigned long)-1 )
	{
		cEffectSceneNode* node = SCENEMAN->GetEffectSceneNode( mEffectIdx );
		if( node )
		{
			assert( node == mEffect );
			node->Remove();
		}
		mEffect = 0;
		mEffectIdx = (unsigned long)-1;
	}
}

void cCardSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	mPickedByCamera = false;
	if( mNifAniInfo == 0 )
	{
		cSceneNode::OnProcess( deltaTime, accumTime );
		return;
	}

	mNifAniInfo->Update( deltaTime, accumTime );

	unsigned long time = accumTime;
	unsigned long lastDelta = deltaTime;

	if( mMoveState == eCard_Rew )
		time = (unsigned long)(mNifTimeManager->GetRewTime((float)lastDelta * 0.001f) * 1000 );
	else
		time = (unsigned long)(mNifTimeManager->GetFrameTime((float)lastDelta * 0.001f) * 1000 );

	if( mCardUpdate )
	{
		if( mGotoDeltaTime <= deltaTime )
		{
			time = mCardGotoTime;
			mNifTimeManager->SetFrameTime( (float)time*0.001f );
			mGotoDeltaTime = 0;
			Stop();
		}
		else
			mGotoDeltaTime -= deltaTime;
	}

	mNeedUpdateTransform = true;
	cSceneNode::OnProcess( deltaTime, time );


	if( mEffect )
	{
		mEffect->SetRotate( GetDummyRot() );
		mEffect->SetTranslate( GetDummyTrans() );
	}

	if( mDisplayObj )
	{
		mDisplayObj->SetRot( GetDummyRot() );
		mDisplayObj->SetPos( GetDummyTrans() );
	}
}

void cCardSceneNode::UpdateCard( float time )
{
	GetNiNode()->Update( time );
}

bool cCardSceneNode::Init( const cCardSceneNodeParam& param )
{
	if( cStaticSceneNode::Init( param ) == false )
		return false;

	if( mNifAniInfo )
	{
		mNifAniInfo->SetStopFlag( false );
		mNifAniInfo->SetAnimType( NiTimeController::APP_TIME );

		mNifTimeManager = new cNifTimeManager;
		mNifTimeManager->SetStartTime( mNifAniInfo->GetMinTime() );
		mNifTimeManager->SetEndTime( mNifAniInfo->GetMaxTime() );
	}

	mCurrentPosIdx = param.mCardIndex;

	mDummy = GetNiNode()->GetObjectByName("character_Dummy01");
	assert(mDummy);
	
	NiTransform trans;
	trans.MakeIdentity();
	mEffect = SCENEMAN->CreateSelfEffect( "./data/map/character_select_cardeffect.nif", false, trans, true );

	if( mEffect )
	{
		mEffect->SetViewNode( false );
		mEffectIdx = mEffect->GetIndexByManger();
	}

	return true;
}

void cCardSceneNode::SetFrameTime( float time )
{
	if( mNifTimeManager == 0 )
		return;

	mNifTimeManager->SetFrameTime( time );
	mNifTimeManager->Disable();
}

void cCardSceneNode::SetViewNode( bool view )
{
	cStaticSceneNode::SetViewNode( view );

	if( mEffect )
	{
		mEffect->SetViewNode( view );
/*
		cEffectSceneNode* eff = SCENEMAN->GetEffectNode( mEffectIdx );
		if( eff != mEffect )
		{
			assert(0);
		}
		else
		{
			mEffect->SetViewNode( view );
		}
*/
	}
}

NiPoint3 cCardSceneNode::GetDummyTrans()
{
	if( mDummy == 0 )
		return NiPoint3::ZERO;

	return mDummy->GetWorldTranslate();
}

NiMatrix3 cCardSceneNode::GetDummyRot()
{
	if( mDummy == 0 )
		return NiMatrix3::IDENTITY;

	return mDummy->GetWorldRotate();
}

void cCardSceneNode::Play( unsigned long time )
{
	mCardUpdate = true;

	if( mNifAniInfo == 0 )
		return;

	switch(mCurrentPosIdx)
	{
	case 0: mCardGotoTime = 1000; mGotoDeltaTime = 1000; break;
	case 1: mCardGotoTime = unsigned long((50.0f/30.0f)*1000); mGotoDeltaTime = 666; break;
	case 2: mCardGotoTime = unsigned long((70.0f/30.0f)*1000); mGotoDeltaTime = 667; break;
	case 3: mCardGotoTime = 3000; mGotoDeltaTime = 667; break;
	case 4: mCardGotoTime = 0; mGotoDeltaTime = 1333; break;
	default: assert(0); return;
	}

	mCurrentPosIdx++;
	if( mCurrentPosIdx > 4 )
		mCurrentPosIdx = 0;

	mMoveState = eCard_Play;

	mNifAniInfo->Start( (float)time * 0.001f );
	mNifTimeManager->Enable();
}

void cCardSceneNode::Rew( unsigned long time )
{
	mCardUpdate = true;

	if( mNifAniInfo == 0 )
		return;

	switch(mCurrentPosIdx)
	{
	case 0: mCardGotoTime = 3000; mGotoDeltaTime = 1333; break;
	case 1: mCardGotoTime = 0; mGotoDeltaTime = 1000; break;
	case 2: mCardGotoTime = 1000; mGotoDeltaTime = 666; break;
	case 3: mCardGotoTime = unsigned long((50.0f/30.0f)*1000); mGotoDeltaTime = 667; break;
	case 4: mCardGotoTime = unsigned long((70.0f/30.0f)*1000); mGotoDeltaTime = 667; break;
	default: assert(0); return;
	}

	mCurrentPosIdx--;
	if( mCurrentPosIdx < 0  )
		mCurrentPosIdx = 4;

	mMoveState = eCard_Rew;

	mNifAniInfo->Start( (float)time * 0.001f );
	mNifTimeManager->Enable();
}

void cCardSceneNode::Stop()
{
	mCardUpdate = false;

	if( mNifAniInfo == 0 )
		return;

	mMoveState = eCard_Stop;

	mNifAniInfo->Stop();

	if( mNifTimeManager )
		mNifTimeManager->Disable();
}
