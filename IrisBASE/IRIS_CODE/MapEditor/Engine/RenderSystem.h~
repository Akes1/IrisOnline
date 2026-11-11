/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 렌더링 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Renderer.h"

class cAppWindow;

/// 렌더링 옵션
class cRenderOption
{
public:
	cRenderOption();

public:
	/// 크기
	unsigned int mWidth;
	unsigned int mHeight;

	/// 플래그값
	unsigned int mFlags;

	/// 어댑터
	unsigned int mAdapter;

	/// 백버퍼 수
	unsigned int mBackBufferCount;

	/// 갱신율
	unsigned int mRefreshRate;

	/// 렌더러 정보
	NiDX9Renderer::DeviceDesc mDeviceDesc;
	NiDX9Renderer::FrameBufferFormat mFrameBufferFormat;
	NiDX9Renderer::DepthStencilFormat mDepthStencilFormat;
	NiDX9Renderer::PresentationInterval mPresentationInterval;
	NiDX9Renderer::SwapEffect mSwapEffect;
	NiDX9Renderer::FramebufferMode mFrameBufferMode;
};

inline
cRenderOption::cRenderOption()
{
	mWidth = 800;
	mHeight = 600;
	mFlags = NiDX9Renderer::USE_MULTITHREADED;//NiDX9Renderer::USE_STENCIL;///USE_MANUALDEPTHSTENCIL;//USE_STENCIL;// | NiDX9Renderer::USE_MULTITHREADED;
	mAdapter = D3DADAPTER_DEFAULT;
	mBackBufferCount = 1;
	mRefreshRate = NiDX9Renderer::REFRESHRATE_DEFAULT;
	mDeviceDesc = NiDX9Renderer::DEVDESC_PURE;
	mFrameBufferFormat = NiDX9Renderer::FBFMT_X8R8G8B8;//FBFMT_UNKNOWN;
	mDepthStencilFormat = NiDX9Renderer::DSFMT_UNKNOWN;//NiDX9Renderer::DSFMT_D24S8;//DSFMT_D32;//DSFMT_D24S8;//DSFMT_UNKNOWN;
	mPresentationInterval = NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;
	mSwapEffect = NiDX9Renderer::SWAPEFFECT_DEFAULT;// NiDX9Renderer::SWAPEFFECT_DISCARD;
	mFrameBufferMode = NiDX9Renderer::FBMODE_DEFAULT;//NiDX9Renderer::FBMODE_MULTISAMPLES_2;
}

/// 렌더링 시스템
class cRenderSystem
{
	static cRenderSystem* mSingleton;

public:
	cRenderSystem( cAppWindow* window );
	~cRenderSystem();

	/// 초기화
	bool Init();

	/// 렌더링
	void Render();
	void BeginFrame( bool playing );
	void EndFrame();
	void DisplayFrame();

	/// 재설정
	void Reset();

	/// 화면 크기
	void SetScreenSize( unsigned int width, unsigned int height );
	unsigned int GetScreenWidth() const;
	unsigned int GetScreenHeight() const;

	/// 화면 모드 변경
	void ToggleScreenMode();

	/// 전체 화면 여부
	bool IsFullScreen() const;

	/// 렌더링 옵션
	const cRenderOption* GetOption() const;

	/// 렌더러
	cRenderer* GetRenderer();

private:
	/// 렌더러를 생성
	bool CreateRenderer();

public:
	/// 단일체를 리턴
	static cRenderSystem* GetSingleton();

private:
	/// 렌더링 옵션
	cRenderOption mRenderOption;

	/// 렌더러
	cRendererPtr mRenderer;

	/// 어플리케이션 윈도우
	cAppWindow* mWindow;
};

inline
void cRenderSystem::SetScreenSize( unsigned int w, unsigned int h )
{
	mRenderOption.mWidth = w;
	mRenderOption.mHeight = h;
}

inline
unsigned int cRenderSystem::GetScreenWidth() const
{
	return mRenderOption.mWidth;
}

inline
unsigned int cRenderSystem::GetScreenHeight() const
{
	return mRenderOption.mHeight;
}

inline
bool cRenderSystem::IsFullScreen() const
{
	return (mRenderOption.mFlags & NiDX9Renderer::USE_FULLSCREEN) ? true : false;
}

inline
const cRenderOption* cRenderSystem::GetOption() const
{
	return &mRenderOption;
}

inline
cRenderer* cRenderSystem::GetRenderer()
{
	return mRenderer;
}

inline
cRenderSystem* cRenderSystem::GetSingleton()
{
	return mSingleton;
}

#define RENDERSYS cRenderSystem::GetSingleton()
