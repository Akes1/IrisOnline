/* ====================================================================
 * 파    일 :	RenderSystem.h
 * 목    적 :	게임브리오 그래픽 Wrapping이면서 모든 랜더링 관리자
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 
 * 주의사항 :	
 * =================================================================== */

#pragma once

#include "Renderer.h"

class cAppWindow;
class cFontAgent;
class cLightAgent;
class cFogAgent;
class cSceneManager;

class cBrightShader;

const unsigned int DEFAULT_WIDTH = 1024;
const unsigned int DEFAULT_HEIGHT = 768;
const unsigned int DEFAULT_BPP = 32;


class cPostProcess
{
public:
	cPostProcess();
	~cPostProcess();

	bool Init( cRenderer* r );
	void Recreate( cRenderer* r, unsigned int width, unsigned int height, unsigned char bpp );

	void Render( cRenderer* r );

	inline NiRenderTargetGroup* GetRenderTarget() { return mpRenderTargetGroup; }

protected:
	void RenderImmediate( cRenderer* r );

protected:

	LPDIRECT3DVERTEXDECLARATION9 mVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9 mPosCoordBuffer;
	LPDIRECT3DVERTEXBUFFER9 mTexCoordBuffer;

	NiRenderedTexturePtr	mpRenderedTexture;
	NiRenderTargetGroupPtr	mpRenderTargetGroup;

public:
	cBrightShader* mShader;
};

/// 렌더링 시스템
class cRenderSystem
{
	static cRenderSystem* mpSingleton;

public:
	/// 단일체를 리턴
	static cRenderSystem* GetSingleton();

public:
	cRenderSystem( cAppWindow* pwindow );
	~cRenderSystem();

	/// 랜더링 옵션
	struct sRenderOption
	{
		/// Window Width Height
		unsigned int						Width;
		unsigned int						Height;
		unsigned char						Bpp;

		/// Gamebryo Option
		unsigned int						Flags;

		/// 그래픽 카드
		unsigned int						Adapter;

		/// 디바이스 옵션
		NiDX9Renderer::DeviceDesc			DevDesc;

		/// 버퍼 포맷
		NiDX9Renderer::FrameBufferFormat	FBformat;

		/// Depth Stencil 포맷
		NiDX9Renderer::DepthStencilFormat	DSformat;

		/// 
		NiDX9Renderer::PresentationInterval Interval;

		///
		NiDX9Renderer::SwapEffect			Swapeffect;

		/// 버퍼 모드
		unsigned int						FBmode;

		/// 
		unsigned int						Backbuffercount;

		/// 
		unsigned int						Refreshrate;

		sRenderOption()
		{
			Width				= 1024;	//= 1280;
			Height				= 768;	//= 1024;
			Bpp					= 32;

			Flags				= NiDX9Renderer::USE_FULLSCREEN | NiDX9Renderer::USE_STENCIL | NiDX9Renderer::USE_MULTITHREADED;

			Adapter				= D3DADAPTER_DEFAULT;
			DevDesc				= NiDX9Renderer::DEVDESC_PURE;
			FBformat			= NiDX9Renderer::FBFMT_X8R8G8B8;
			DSformat			= NiDX9Renderer::DSFMT_D24S8;
			Interval			= NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;
			Swapeffect			= NiDX9Renderer::SWAPEFFECT_DISCARD;
			FBmode				= NiDX9Renderer::FBMODE_LOCKABLE;
//			FBmode				= NiDX9Renderer::FBMODE_MULTISAMPLES_4;
			Backbuffercount		= 1;
			Refreshrate			= NiDX9Renderer::REFRESHRATE_DEFAULT;
		}
	};

	/// 초기화
	bool	Init();

	/// 종료
	void	Exit();

	bool RenderOnce( NiTexture* tex );

	/// 장면 표현
	void	Render();

	/// 옵션 설정의 변경 및 장치의 재생성
	bool	Reset( unsigned int width, unsigned int height, unsigned char bpp, bool winmode );

	/// 장치 소실에 대한 처리 루틴
	void Invalidate();
	void Restore();

	inline void	SetClearColor( NiColorA color ) { mClearColor = color; }

	/// 스크린 모드의 변경 랜더 옵션의 변경
	void			ToggleFullScreenMode();

	/// 렌더러
	cRenderer* GetRenderer() const;

	/// 옵션 참조 함수
	inline cAppWindow*				GetWindowRef() { return mpWindowRef; }
	inline sRenderOption*			GetOption()			{ return &mRenderOption; }
	inline unsigned int				GetScreenWidth()	{ return mRenderOption.Width; }
	inline unsigned int				GetScreenHeight()	{ return mRenderOption.Height; }
	inline unsigned char			GetScreenBpp()	{ return mRenderOption.Bpp; }

	inline tArray<void*>*			GetResolutionArr() { return &mResolutionArr; };
	inline NiDX9AdapterDesc::ModeDesc* GetCurrentMode() { return mCurrentMode; }
	inline NiDX9AdapterDesc*	GetCurrentAdapter() { return mCurrentAdapter; }

	/// 스크린 모드값 체크
	inline const bool				IsFullScreenMode()	{ return (mRenderOption.Flags & NiDX9Renderer::USE_FULLSCREEN)? true:false; }

	/// 윈도우 핸들을 획득한다.
	HWND GetHWND();

	bool mReCreateContinue;
	unsigned long mReCreateTime;

	void SetWorldRendering( bool set ) { mWorldRendering = set; }

	void SetBrightness( float br );

	inline unsigned int				GetMonitorWidth() { return mMonWidth; }
	inline unsigned int				GetMonitorHeight() { return mMonHeight; }
	void SetMonitorDisplay( unsigned int width, unsigned int height, unsigned int bpp );

	void ToggleVideoCapture();
	void SaveScreenShot();
	bool IsVideoCapturing();

	/// post process
protected:

	cPostProcess*				mPostProcess;

private:
	/// 게임브리오 랜더러를 생성한다.
	bool CreateRenderer();

	/// 장치에 대한 나열
	bool EnumAdapters();
	void EnumResolutions( unsigned int curWidth, unsigned int curheight, unsigned int curBpp, bool windowed );
	void EnumMultisamples();

private:

	/// monitor resolution
	unsigned int	mMonWidth;
	unsigned int	mMonHeight;
	unsigned int	mMonBpp;

	/// 랜더링 옵션
	sRenderOption mRenderOption;

	/// 렌더러
	cRendererPtr mRenderer;

	/// 윈도우 참조 포인터
	cAppWindow* mpWindowRef;

	/// Font 관리
	cFontAgent* mpFontAgent;

	/// 라이트 관리
	cLightAgent* mpLightAgent;

	/// 안개 관리
	cFogAgent* mpFogAgent;

	NiColorA		mClearColor;


	/// 장치에 대한 나열
	typedef tArray<void*>	cArray;

	/// adapter and device
	cArray	mAdapterArr;
	NiDX9AdapterDesc*	mCurrentAdapter;
	NiDX9DeviceDesc*	mCurrentDevice;

	/// mode
	cArray	mResolutionArr;
	NiDX9AdapterDesc::ModeDesc*	mCurrentMode;

	/// multi sample
	cArray	mMultisampleArr;

	bool mWorldRendering;

	TCHAR mVideoPathName[MAX_PATH];
	unsigned long mVideoCaptureTime;
};

inline
cRenderer* cRenderSystem::GetRenderer() const
{
	return mRenderer;
}


inline cRenderSystem* cRenderSystem::GetSingleton()
{
	return mpSingleton;
}

/// 장면 노드 비교 함수자
/// 픽킹 거리로 비교한다.
class cResolutionCompare
{
public:
	bool operator () ( const void* left, const void* right ) const
	{
		if( ((NiDX9AdapterDesc::ModeDesc*)left)->m_uiWidth  > ((NiDX9AdapterDesc::ModeDesc*)right)->m_uiWidth )
		{
			return false;
		}
		else if( ((NiDX9AdapterDesc::ModeDesc*)left)->m_uiWidth == ((NiDX9AdapterDesc::ModeDesc*)right)->m_uiWidth )
		{
			 if( ((NiDX9AdapterDesc::ModeDesc*)left)->m_uiHeight > ((NiDX9AdapterDesc::ModeDesc*)right)->m_uiHeight )
			 {
				 return false;
			 }
			 else if( ((NiDX9AdapterDesc::ModeDesc*)left)->m_uiHeight == ((NiDX9AdapterDesc::ModeDesc*)right)->m_uiHeight )
			 {
				 if( ((NiDX9AdapterDesc::ModeDesc*)left)->m_uiBPP >= ((NiDX9AdapterDesc::ModeDesc*)right)->m_uiBPP )
					 return false;
				 else
					 return true;
			 }
			 else
				 return true;
		}
		else
			return true;
	};
};

#define RENDERSYS cRenderSystem::GetSingleton()
