#include "StdAfx.h"
#include ".\mapportalscenenode.h"

#include "ResourceManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "LightAgent.h"
#include "FogAgent.h"
#include "NifAnimationInfo.h"

#include "ObjectNameCard.h"
#include "StageScript.h"
#include "GameResourceManager.h"
#include "RenderSystem.h"
#include "NaviMesh.h"

#include "ObjectManager.h"
#include "hero.h"
#include "PartyManager.h"

#include "Application.h"

cMapPortalSceneNode::cMapPortalSceneNode( eType type )
: cManagedStaticSceneNode(type)
, mpNameCard(0)
{
	mPortalIdx = 0;
	mPosScriptIdx = 0;
	mNameHeight = 0;
}

cMapPortalSceneNode::~cMapPortalSceneNode()
{
	ClearCollectInfo();

	SAFE_DELETE(mpNameCard);
}

void cMapPortalSceneNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cManagedStaticSceneNode::OnProcess( deltaTime, accumTime );

	mPickedByCamera = false;

	if( mAlpha < 0.000001f )
		mCheckFrustum = false;
	else
		mCheckFrustum = true;
}

bool cMapPortalSceneNode::Init( cMapPortalSceneNodeParam& param )
{
	mPosScriptIdx = param.mPosScriptIdx;

	sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( mPosScriptIdx );
	if( pInfo == 0 )
		return false;

	mNameHeight = pInfo->mNameHeight;

	/// 파일을 로딩
	param.mPathName += pInfo->mFileName;
	if( RESOURCEMAN->LoadNIF( param.mPathName.Cstr() ) == false )
	{
		assert( 0 && "failed to load nif file" );
		return false;
	}

	/// 리소스 관리자로부터 엔진 노드를 복제
	NiNode* n = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	/// 상위 초기화
	mSceneNiNode = n;

	NiFogProperty* pFog = NiNew NiFogProperty;
	pFog->SetFog( false );
	n->AttachProperty( pFog );

	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	/// create name card
	if( pInfo->mTargetInfo )
	{
		mpNameCard = new cNameCard;
		mpNameCard->SetName( (LPTSTR)GAMERESOURCEMAN->GetMapName( pInfo->mTargetInfo->mMapNumber ), 0xFF19F0A0 );
	}

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( true );

		//mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

void cMapPortalSceneNode::AddToVisibleArray()
{
	if( IsViewNode() == false )
		return;

	cSceneNode::AddToVisibleArray();

	if( mpNameCard )
	{
		NiPoint3 pos = this->GetWorldTranslate();
		pos.z += mNameHeight;

		bool showName = false;

		if( mIsMouseOver == false )
		{
			NiPoint3 camPos = CAMERAMAN->GetCurrentNi()->GetWorldTranslate();
			float distCam = (pos - camPos).Length();
			float distHero = distCam;
			if( HERO )
				distHero = (pos - HERO->GetPos()).Length();

			if( distHero < 1500.0f )
				distHero = 1500.0f;
			if( distHero > 10000.0f )
				distHero = 10000.0f;

			if( distHero <= 3000.0f )
				showName = true;
		}
		else
		{
			showName = true;
		}

		if( showName )
		{
			float x,y;
			cPlane::WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y, false );
			int ScreenPosX = (int)( x * RENDERSYS->GetScreenWidth() );
			int ScreenPosY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );

			mpNameCard->Set2DPos( ScreenPosX, ScreenPosY, 0, 0 );
			SCENEMAN->AddPlane( mpNameCard );
		}
	}
}



//////////////////////////////////////////////////////////////////////////
bool cUserPortalSceneNode::Init( cUserPortalSceneNodeParam& param )
{
	mPosScriptIdx = param.mPosScriptIdx;
	mPortalIdx = param.mPortalIdx;
	mOwnerIdx = param.mOwnerIdx;

	if( mPortalIdx == 0 )
		return false;

	sStageChangePos* pInfo = STAGESCRIPT->GetPosScriptInfo( mPosScriptIdx );
	if( pInfo == 0 )
		return false;

	mNameHeight = pInfo->mNameHeight;

	if( RESOURCEMAN->LoadNIF( param.mPathName.Cstr() ) == false )
	{
		assert( 0 && "failed to load nif file" );
		return false;
	}

	/// 리소스 관리자로부터 엔진 노드를 복제
	NiNode* n = RESOURCEMAN->CloneObjectByName( param.mPathName.Cstr() );
	if( n == 0 )
	{
		assert( 0 && "failed to clone node from resource manager" );
		return false;
	}

	/// 상위 초기화
	mSceneNiNode = n;

	NiFogProperty* pFog = NiNew NiFogProperty;
	pFog->SetFog( false );
	n->AttachProperty( pFog );

	float height = 0.0f;
	NAVIMESH->CalcHeight( &height, param.mTranslate.x, param.mTranslate.y );
	param.mTranslate.z = height;

	if( cSceneNode::Init( param ) == false )
	{
		return false;
	}

	mpNameCard = new cNameCard;
	mpNameCard->SetName( (LPTSTR)GAMERESOURCEMAN->GetMapName( pInfo->mMapNumber ), 0xFF19F0A0 );

	/// 애니메이션 컨트롤러 정보 수집
	mNifAniInfo->CollectData( n );
	if( mNifAniInfo->GetNumberOfControllers() )
	{
		mNifAniInfo->SetAnimType( NiTimeController::APP_INIT );
		mNifAniInfo->SetLooping( true );

		//mNifAniInfo->Start( THEAPP->GetWorldAccumTime() * 0.001f );
	}
	else
	{
		/// 애니 정보가 없다면 메모리 해제
		SAFE_DELETE(mNifAniInfo);
	}

	return true;
}

bool cUserPortalSceneNode::Pick( const cRay& ray )
{
	/// party or owner check
	if( PARTYMAN->IsHeroParty( mOwnerIdx ) == false )
	{
		if( HERO->GetObjectID() != mOwnerIdx )
			return false;
	}
	return cMapPortalSceneNode::Pick( ray );
}
