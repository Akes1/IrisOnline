#include "StdAfx.h"
#include "CameraManager.h"

#include "FreeCamera.h"
#include "LookAtCamera.h"

#include "UIManager.h"
#include "RenderSystem.h"

cCameraManager* cCameraManager::mpSingleton = NULL;

cCameraManager::cCameraManager()
: mpCurrentCamera(NULL)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

//	mCameraArray.Reserve( 5 );

	mCurrentCameraIdx = 0;
}

cCameraManager::~cCameraManager()
{
	cCamera* pCamera = NULL;
	for( unsigned int i=0; i<mCameraArray.GetSize(); ++i )
	{
		pCamera = (cCamera*)mCameraArray[i];
		SAFE_DELETE( pCamera );
	}
	mCameraArray.Clear();

	mpSingleton = 0;
}

bool cCameraManager::Init()
{
	/// LookAt Camera생성 : 게임 Hero에 종속된다.
	cLookAtCamera* pcam = new cLookAtCamera;
	pcam->Init( (float)RENDERSYS->GetScreenWidth(), (float)RENDERSYS->GetScreenHeight() );
	mCameraArray.PushBack( pcam );

	/// Free Camera생성
	cFreeCamera* pcam2 = new cFreeCamera;
	pcam2->Init( (float)RENDERSYS->GetScreenWidth(), (float)RENDERSYS->GetScreenHeight() );
	mCameraArray.PushBack( pcam2 );


#ifdef _GMTOOL
	cFreeCamera* pcam3 = new cFreeCamera;
	pcam3->Init( (float)RENDERSYS->GetScreenWidth(), (float)RENDERSYS->GetScreenHeight() );
	mCameraArray.PushBack( pcam3 );
#endif

	/// 기본 카메라 셋팅
	mpCurrentCamera = pcam2;
	mCurrentCameraIdx = 1;

	return true;
}

void cCameraManager::Open()
{
	mpCurrentCamera->Reset();
}

void cCameraManager::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mpCurrentCamera )
	{
		mpCurrentCamera->OnProcess( deltaTime, accumTime );
	}
}

void cCameraManager::ChangeCamera( unsigned int cameraIdx )
{
	cCamera* pCamera = (cCamera*)mCameraArray[cameraIdx];
	if( !pCamera || mpCurrentCamera == pCamera )
	{
		return;
	}

	mCurrentCameraIdx = cameraIdx;
	mpCurrentCamera = pCamera;
	mpCurrentCamera->Reset();
}

NiPoint3 cCameraManager::GetCurrentCameraPos()
{
	NiCamera* pCamera = GetCurrentNi();

	if( pCamera )
	{
		return pCamera->GetWorldTranslate();
	}

	return NiPoint3::ZERO;
}

float cCameraManager::GetCurrentLookAtDistance()
{
	cCamera* pCamera = GetCurrent();

	if( pCamera )
	{
		return pCamera->GetLookAtDistance();
	}

	return 0.0f;
}

void cCameraManager::Reset( unsigned int screenWidth, unsigned int screenHeight )
{
	cCamera* pCamera = NULL;
	for( unsigned int i=0; i<mCameraArray.GetSize(); ++i )
	{
		pCamera = (cCamera*)mCameraArray[i];
		if( pCamera )
		{
			pCamera->SetViewFrustum( (float)screenWidth, (float)screenHeight );
		}
	}
}

//bool cCameraManager::ProcessKeyUpDown( float time )
//{
//	bool used = false;
//	if( mpCurrentCamera )
//	{
//		used = mpCurrentCamera->ProcessKeyUpDown( time );
//	}
//	return used;
//}

//bool cCameraManager::ProcessKeyPressed( float time )
//{
//	bool used = false;
//	if( mpCurrentCamera )
//	{
//		used = mpCurrentCamera->ProcessKeyPressed( time );
//	}
//	return used;
//}

void cCameraManager::ProcessMouse( unsigned long time )
{
	if( UIMAN->IsCameraEnableContainUI() == false )
		return;

	if( mpCurrentCamera )
	{
		mpCurrentCamera->OnProcessMouse( time );
	}
}

/*
bool cCameraManager::ProcessInputKey( float time )
{
	bool used = false;
	if( mpCurrentCamera )
	{
		used = mpCurrentCamera->ProcessInputKey( time );
	}
	return used;
}

void cCameraManager::ProcessInputMouse( float time )
{
	if( mpCurrentCamera )
	{
		mpCurrentCamera->ProcessInputMouse( time );
	}
}
*/

void cCameraManager::Shake()
{
	if( mpCurrentCamera )
		mpCurrentCamera->Shake();
}

bool cCameraManager::GetRayFromWindowPoint( NiPoint3* porigin, NiPoint3* pdir, int mouseX, int mouseY )
{
	bool check = false;
	if( mpCurrentCamera )
	{
		check = mpCurrentCamera->GetRayFromWindowPoint( mouseX, mouseY, *porigin, *pdir );
		pdir->Unitize();
	}
	else
	{
		assert( 0 && "null current camera" );
	}

	return check;
}