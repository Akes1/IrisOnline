#include "stdafx.h"
#include "RenderSystem.h"

#include "AppWindow.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "WorldManager.h"
#include "SceneManager.h"

#include <NiMeshMaterialBinding.h>

cRenderSystem* cRenderSystem::mSingleton = 0;

cRenderSystem::cRenderSystem( cAppWindow* window )
: mWindow( window )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	// It can filter flat images into mipmap pyramids, and can convert 
	// from standard Gamebryo pixel formats (PAL8, PALA8, RGB24, RGBA32, etc)
	// to non-standard formats, and from non-standard formats to a few standard formats
	NiImageConverter::SetImageConverter( NiNew NiDevImageConverter );
	NiTexture::SetMipmapByDefault( true );
}

cRenderSystem::~cRenderSystem()
{
//	/// 이미지 변환자를 설정
//	NiImageConverter::SetImageConverter( NiNew NiDevImageConverter );

	mSingleton = 0;
}

bool cRenderSystem::Init()
{
	NiMeshMaterialBinding::SetCreateMissingStreams(true);

	/// 렌더러를 생성
	if( CreateRenderer() == false )
	{
		assert( 0 && "failed to create renderer" );
		return false;
	}
	if( mRenderer->Init() == false )
	{
		assert( 0 );
		return false;
	}

	return true;
}

void cRenderSystem::Render()
{
	assert( 0 && "do not use this func in map editor" );
}

void cRenderSystem::BeginFrame( bool /*playing*/ )
{
	if( mRenderer->IsDeviceLost() )
	{
		Reset();
	}

	mRenderer->BeginFrame();
	{
//		mRenderer->BeginUsingDefaultRenderTargetGroup( playing ? NiRenderer::CLEAR_ZBUFFER : NiRenderer::CLEAR_ALL );
		mRenderer->BeginUsingDefaultRenderTargetGroup( NiRenderer::CLEAR_ALL );
		{
			/// 카메라 설정
			mRenderer->SetCameraData( (NiCamera*)(*CAMERAMAN->GetCurrent()) );
		}
	}
}

void cRenderSystem::EndFrame()
{
	{
		mRenderer->EndUsingRenderTargetGroup();
	}
	mRenderer->EndFrame();
}

void cRenderSystem::DisplayFrame()
{
	mRenderer->DisplayFrame();
}

void cRenderSystem::Reset()
{
	mRenderer->OnLostDevice();

	/// 어플리케이션 윈도우 핸들을 얻음
	NiWindowRef handle = mWindow->GetHandle();
	mWindow->ToggleWindow( IsFullScreen() );

	/// 엔진 렌더러를 재생성
	NiDX9Renderer::RecreateStatus ret = NiDX9Renderer::GetRenderer()->Recreate(
		mRenderOption.mWidth,
		mRenderOption.mHeight,
		mRenderOption.mFlags,
		handle,
		mRenderOption.mFrameBufferFormat, 
		mRenderOption.mDepthStencilFormat, 
		mRenderOption.mPresentationInterval, 
		mRenderOption.mSwapEffect, 
		mRenderOption.mFrameBufferMode, 
		mRenderOption.mBackBufferCount, 
		mRenderOption.mRefreshRate );

	if( ret == NiDX9Renderer::RECREATESTATUS_FAILED || ret == NiDX9Renderer::RECREATESTATUS_RESTORED )
	{
		OutputDebugString("Error: failed to recreate renderer");

		mRenderOption.mFlags = NiDX9Renderer::USE_STENCIL | NiDX9Renderer::USE_MULTITHREADED;
		if( IsFullScreen() )
			mRenderOption.mFlags |= NiDX9Renderer::USE_FULLSCREEN;

		ret = NiDX9Renderer::GetRenderer()->Recreate(
			mRenderOption.mWidth,
			mRenderOption.mHeight,
			mRenderOption.mFlags,
			handle, 
			mRenderOption.mFrameBufferFormat, 
			mRenderOption.mDepthStencilFormat, 
			mRenderOption.mPresentationInterval, 
			mRenderOption.mSwapEffect, 
			mRenderOption.mFrameBufferMode, 
			mRenderOption.mBackBufferCount, 
			mRenderOption.mRefreshRate );

		if( ret == NiDX9Renderer::RECREATESTATUS_FAILED || ret == NiDX9Renderer::RECREATESTATUS_RESTORED )
		{
			NiMessageBox( "Failed to Recreate DX9 Renderer.", "Error!" );
			::SendMessage( mWindow->GetHandle(), WM_CLOSE, 0, 0 );
		}
	}

	mRenderer->OnResetDevice();
}

bool cRenderSystem::CreateRenderer()
{
	const NiDX9SystemDesc* systemDesc = NiDX9Renderer::GetSystemDesc();
	unsigned int numAdapters = systemDesc->GetAdapterCount();

	for( unsigned int i = 0; i < numAdapters; ++i )
	{
		const NiDX9AdapterDesc* adapterDesc = systemDesc->GetAdapter( i );
		const char* adapterName = adapterDesc->GetStringDescription();

		if( adapterName && ::strstr( adapterName, "NVPerfHUD" ) )
		{
			mRenderOption.mAdapter = i;
			mRenderOption.mDeviceDesc = NiDX9Renderer::DEVDESC_REF_HWVERTEX;
		}
	}

	// 엔진 렌더러를 생성
	NiWindowRef handle = mWindow->GetHandle();

	mRenderer = cRenderer::Create(
		mRenderOption.mWidth,
		mRenderOption.mHeight,
		(NiDX9Renderer::FlagType)mRenderOption.mFlags,
		handle,
		handle,
		mRenderOption.mAdapter,
		mRenderOption.mDeviceDesc, 
		mRenderOption.mFrameBufferFormat, 
		mRenderOption.mDepthStencilFormat,
		mRenderOption.mPresentationInterval,
		mRenderOption.mSwapEffect,
		mRenderOption.mFrameBufferMode,
		mRenderOption.mBackBufferCount,
		mRenderOption.mRefreshRate
		);

	if( mRenderer == 0 )
	{
		NiMessageBox( "Failed to Create DX9 Renderer.", "Error!" );
		return false;
	}

	/// 배경색을 설정
	mRenderer->SetBackgroundColor( NiColor::WHITE );

	/// 윈도우 처리
	mWindow->ToggleWindow( IsFullScreen() );
	return true;
}

void cRenderSystem::ToggleScreenMode()
{
	if( IsFullScreen() )
		mRenderOption.mFlags &= ~(NiDX9Renderer::USE_FULLSCREEN);
	else
		mRenderOption.mFlags |= NiDX9Renderer::USE_FULLSCREEN;
}
