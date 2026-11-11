#include "stdafx.h"
#include "Application.h"

#include "DXVersion.h"
#include "AppWindow.h"
#include "AppTimer.h"

#include "FilePackSystem.h"
#include "RenderSystem.h"
#include "SoundSystem.h"
#include "InputSystem.h"

#include "GameResourceManager.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "SkillManager.h"
#include "DramaturgyManager.h"

cApplication* cApplication::mSingleton = 0;
cString cApplication::mLangaugeFolder = "";


cApplication::cApplication()
: mFileSystem( 0 )
, mRenderSystem( 0 )
, mSoundSystem( 0 )
, mInputSystem( 0 )
, mResourceManager( 0 )
, mGameResourceManager( 0 )
, mCameraManager( 0 )
, mSceneManager( 0 )
, mSkillManager(0)
, mDramaManager(0)
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	/// 전역 타이머 생성
	mTimer = new cAppTimer;
	mTimer->Reset();
	mAccumTime = 0;
	mDeltaTime = 0;

	mActive = true;
}

cApplication::~cApplication()
{
	delete mDramaManager;
	delete mSkillManager;
	delete mSceneManager;
	delete mCameraManager;
	if( mGameResourceManager )
	{
		mGameResourceManager->Exit();
		delete mGameResourceManager;
		mGameResourceManager = 0;
	}
	SAFE_DELETE(mResourceManager);
	SAFE_DELETE(mInputSystem);
	SAFE_DELETE(mRenderSystem);
	SAFE_DELETE(mFileSystem);
	SAFE_DELETE(mSoundSystem);
	SAFE_DELETE(mWindow);
	SAFE_DELETE(mTimer);

	mSingleton = 0;
}

void cApplication::Clear()
{
	if( mSceneManager )
		mSceneManager->Clear();

	if( mResourceManager )
		mResourceManager->Close();
}

bool cApplication::Init( HWND wnd )
{
	/// 전역 윈도우를 생성
	mWindow = new cAppWindow( wnd );

	/// 파일 시스템을 생성
	mFileSystem = new cFilePackSystem;
	if( !mFileSystem || !mFileSystem->Init() )
	{
		MessageBoxA(NULL, "failed to init file system", "Error", MB_OK );
		return false;
	}

	/// 렌더링 시스템을 생성
	mRenderSystem = new cRenderSystem( mWindow );
	if( mRenderSystem->Init() == false )
	{
		MessageBoxA(NULL, "failed to init render system", "Error", MB_OK );
		return false;
	}

	/// 사운드 시스템을 생성
	mSoundSystem = new cSoundSystem;
	if( mSoundSystem->Init() == false )
	{
		MessageBoxA(NULL, "failed to init sound system", "Error", MB_OK );
		return false;
	}

	/// 입력 시스템을 생성
	mInputSystem = new cInputSystem;

	/// 자원 관리자를 생성
	mResourceManager = new cResourceManager;
	if( mResourceManager->Init() == false )
	{
		MessageBoxA(NULL, "failed to init resource manager", "Error", MB_OK );
		return false;
	}

	//mpResourceManager->SetBaseDir( baseDir );

	mDramaManager = new cDramaturgyManager;
	if( mDramaManager == 0 )
		return false;
//	if( mDramaManager->Init() == false )
//	{
//		assert( 0 && "failed to init Drama manager" );
//		return false;
//	}

	mGameResourceManager = new cGameResourceManager;
	if( mGameResourceManager->Init() == false )
	{
		MessageBoxA(NULL, "failed to init game resource manager", "Error", MB_OK );
		return false;
	}

	/// 카메라 관리자를 생성

	mCameraManager = new cCameraManager;
	mCameraManager->CreateFree( true );

	/// 장면 관리자를 생성
	mSceneManager = new cSceneManager;
	if( !mSceneManager || !mSceneManager->Init( NiPoint3::ZERO, 100.0f, 100000.0f ) )
	{
		MessageBoxA(NULL, "failed to init scene manager", "Error", MB_OK );

		return false;
	}


	mSkillManager = new cSkillManager;
	if( mSkillManager->Init() == false )
	{
		MessageBoxA(NULL, "failed to skill manager", "Error", MB_OK );
		return false;
	}

	/// 타이머 시작
	mTimer->Start();

	return true;
}

void cApplication::Exit()
{

	/// 타이머 중지
	mTimer->Stop();
}

void cApplication::Process()
{
//	NiSleep(1);

	/// 경과 시간 얻음
	mDeltaTime = mTimer->GetDeltaTime();
	mAccumTime += mDeltaTime;

	/// 입력 관리자를 처리
	mInputSystem->Process();

	/// 카메라 관리자를 처리
	mCameraManager->Process( mDeltaTime );

	mSkillManager->Process( mDeltaTime, mAccumTime );

	mSoundSystem->Process( mDeltaTime*1000, mAccumTime*1000 );

	/// 장면 관리자를 처리
	mSceneManager->Process( mDeltaTime, mAccumTime );
}

void cApplication::Render()
{
	mRenderSystem->Render();
}
