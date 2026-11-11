#include "stdafx.h"
#include "CameraManager.h"

#include "FreeCamera.h"
#include "LookAtCamera.h"
#include "RenderSystem.h"

cCameraManager* cCameraManager::mSingleton = 0;

cCameraManager::cCameraManager()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mCameraArray.Reserve( 8 );
	mCurrentCamera = 0;
}

cCameraManager::~cCameraManager()
{
	DestroyAll();

	mSingleton = 0;
}

void cCameraManager::Process( unsigned long deltaTime )
{
	float delta = (float)deltaTime * 0.001f;

	if( mCurrentCamera )
		mCurrentCamera->OnProcess( delta );
}

void cCameraManager::DestroyAll()
{
	for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
	{
		delete mCameraArray[i];
	}

	mCameraArray.Clear();
}

void cCameraManager::Destroy( cCamera* cam )
{
	if( mCameraArray.GetSize() <= 1 )
		return;

	for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
	{
		if( cam == mCameraArray[i] )
		{
			if( cam == mCurrentCamera )
				mCurrentCamera = mCameraArray[0];

			delete cam;
			mCameraArray.PopAt( i );
			break;
		}
	}
}

void cCameraManager::SetCurrent( cCamera* cam )
{
	assert( cam && "null camera" );

	mCurrentCamera = cam;
}

void cCameraManager::SetCameraTranslateSpeed( float unitsPerSec )
{
	for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
	{
		mCameraArray[i]->SetTranslateSpeed( unitsPerSec );
	}
}

void cCameraManager::SetCameraRotateSpeed( float anglesPerSec )
{
	for( unsigned int i = 0, iend = mCameraArray.GetSize(); i < iend; ++i )
	{
		mCameraArray[i]->SetRotateSpeed( anglesPerSec );
	}
}

cFreeCamera* cCameraManager::CreateFree( bool makeCurrent )
{
	/// 카메라를 생성
	cFreeCamera* cam = new cFreeCamera;

	/// 카메라를 초기화
	if( cam->Init( RENDERSYS->GetScreenWidth(), RENDERSYS->GetScreenHeight() ) == false )
	{
		delete cam;
		assert( 0 && "failed to init free camera" );
		return 0;
	}

	/// 카메라를 추가
	mCameraArray.PushBack( cam );

	if( makeCurrent )
		SetCurrent( cam );

	return cam;
}

cLookAtCamera* cCameraManager::CreateLookAt( bool makeCurrent )
{
	/// 카메라를 생성
	cLookAtCamera* cam = new cLookAtCamera;

	/// 카메라를 초기화
	if( cam->Init( RENDERSYS->GetScreenWidth(), RENDERSYS->GetScreenHeight() ) == false )
	{
		delete cam;
		assert( 0 && "failed to init lookat camera" );
		return 0;
	}

	/// 카메라를 추가
	mCameraArray.PushBack( cam );

	if( makeCurrent )
		SetCurrent( cam );

	return cam;
}
