#include "StdAfx.h"
#include "DoorSceneNode.h"

#include "SceneManager.h"
#include "NifAnimationInfo.h"

cDoorSceneNode::cDoorSceneNode( eType type )
: cManagedStaticSceneNode( type )
{
	mDoorIdx = 0;
	mRemove = false;

}

cDoorSceneNode::~cDoorSceneNode()
{
}

bool cDoorSceneNode::Init( const cDoorSceneNodeParam& param )
{
	mDoorIdx = param.mDoorIdx;
	mRemove = false;

	if( cManagedStaticSceneNode::Init( param ) == false )
		return false;

	if( param.mDisableAnimation == true )
	{
		if( mNifAniInfo )
			mNifAniInfo->SetLooping( false );
	}
	return true;
}

void cDoorSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	ProcessVisibleLevel( deltaTime );

	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		cSceneNode::OnProcess( deltaTime, accumTime );

		if( mNifAniInfo->Update( deltaTime, accumTime ) == cNifAnimationInfo::eEvent_End )
			SetAlphaBlended( 0.0f );
	}

	if( mAlpha == 0.0f && mRemove == true )
		SCENEMAN->AddDeleteDoorNode( this );
}

void cDoorSceneNode::Remove( unsigned long accumTime )
{
	if( mRemove == true )
		return;

	mRemove = true;

	if( mNifAniInfo && mNifAniInfo->IsEnable() == false )
		mNifAniInfo->Start( accumTime*0.001f, true );
	else
		SetAlphaBlended( 0.0f );
}
