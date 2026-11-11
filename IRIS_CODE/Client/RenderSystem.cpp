#include "StdAfx.h"
#include "RenderSystem.h"

#include "AppWindow.h"
#include "Application.h"
#include "CameraManager.h"
#include "FontAgent.h"
#include "LightAgent.h"
#include "FogAgent.h"
#include "SceneManager.h"
#include "WorldManager.h"
#include "UIManager.h"
#include "OptionManager.h"

#include "stageManager.h"
#include "GameResourceManager.h"


#include "Shader.h"
#include "bandicap.h"

#if (defined(_DEVSYS) || defined(_DEBUG))
#include "DevSystem.h"
#endif

cRenderSystem* cRenderSystem::mpSingleton = 0;


// ..
CBandiCaptureLibrary	 mBandiCaptureLib;

bool g_Restore( bool bBeforeReset, void* pvData )
{
	cRenderSystem* render = (cRenderSystem*)pvData;
	assert(render);

	if( bBeforeReset )
	{
		THEAPP->Invalidate();

		render->GetWindowRef()->AdjustWindowForChange( render->IsFullScreenMode() );
	}
	else
	{
		THEAPP->Restore();

		if( render->IsFullScreenMode() == false )
		{
			if( render->GetScreenWidth() > (unsigned int)GetSystemMetrics(SM_CXSCREEN) || 
				render->GetScreenHeight() > (unsigned int)GetSystemMetrics(SM_CYSCREEN) )
			{
				render->mReCreateContinue = true;
			}
		}

		render->GetWindowRef()->AdjustWindowRect( render->GetScreenWidth(), render->GetScreenHeight() );

		CAMERAMAN->Reset( render->GetScreenWidth(), render->GetScreenHeight() );

		/// 옵션 내용은 윈도우 모드일때 GetClientRect에 의해 변할 수 잇다.
		UIMAN->UpdateScreenSize( render->GetScreenWidth(), render->GetScreenHeight() );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
cPostProcess::cPostProcess()
: mpRenderedTexture(0)
, mpRenderTargetGroup(0)
, mPosCoordBuffer( 0 )
, mTexCoordBuffer( 0 )
, mVertexDeclaration( 0 )
{
	mShader = new cBrightShader;
}

cPostProcess::~cPostProcess()
{
	if( mVertexDeclaration )
	{
		mVertexDeclaration->Release();
		mVertexDeclaration = 0;
	}
	if( mPosCoordBuffer )
	{
		mPosCoordBuffer->Release();
		mPosCoordBuffer = 0;
	}
	if( mTexCoordBuffer )
	{
		mTexCoordBuffer->Release();
		mTexCoordBuffer = 0;
	}

	SAFE_DELETE(mShader);

	mpRenderedTexture = 0;
	mpRenderTargetGroup = 0;
}

bool cPostProcess::Init( cRenderer* r )
{
	///
	NiTexture::FormatPrefs prefs;
	const NiRenderTargetGroup* pkRTGroup = r->GetDefaultRenderTargetGroup();
	const NiPixelFormat* pkPixelFormat = pkRTGroup->GetPixelFormat(0);
	if (pkPixelFormat->GetBitsPerPixel() == 16)
		prefs.m_ePixelLayout = NiTexture::FormatPrefs::HIGH_COLOR_16;
	else
		prefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;

	prefs.m_eAlphaFmt = NiTexture::FormatPrefs::NONE;
	prefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

	mpRenderedTexture = NiRenderedTexture::Create( pkRTGroup->GetWidth(0), pkRTGroup->GetHeight(0), r, prefs );
	if( mpRenderedTexture == 0 )
		return false;

	mpRenderTargetGroup = NiRenderTargetGroup::Create( mpRenderedTexture->GetBuffer(), r, true, false );

	/// create Quad point
	mShader->Init( r->GetBrightEffect() );

	/// 정점 선언을 생성
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	LPDIRECT3DDEVICE9 device = r->GetD3DDevice();
	device->CreateVertexDeclaration( decl, &mVertexDeclaration );

	/// 정점 버퍼를 생성

	NiPoint3 vBuffer[4] = { NiPoint3(0.0f, 0.0f, 1.0f),NiPoint3(1.0f, 0.0f, 1.0f), NiPoint3(0.0f, 1.0f, 1.0f), NiPoint3(1.0f, 1.0f, 1.0f) };
	mPosCoordBuffer = r->CreateVertexBuffer( 4, sizeof(NiPoint3), vBuffer );
	if( mPosCoordBuffer == 0 )
		return false;

	NiPoint2 tBuffer[4] = { NiPoint2(0.0f, 0.0f), NiPoint2(1.0f, 0.0f), NiPoint2(0.0f, 1.0f), NiPoint2(1.0f, 1.0f) };
	mTexCoordBuffer = r->CreateVertexBuffer( 4, sizeof(NiPoint2), tBuffer );
	if( mTexCoordBuffer == 0 )
		return false;

	return true;
}

void cPostProcess::Recreate( cRenderer* r, unsigned int width, unsigned int height, unsigned char bpp )
{
	mpRenderedTexture = 0;
	mpRenderTargetGroup = 0;

	///
	NiTexture::FormatPrefs prefs;
	if (bpp == 16)
		prefs.m_ePixelLayout = NiTexture::FormatPrefs::HIGH_COLOR_16;
	else
		prefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;

	prefs.m_eAlphaFmt = NiTexture::FormatPrefs::NONE;
	prefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

	mpRenderedTexture = NiRenderedTexture::Create( width, height, r, prefs );
	mpRenderTargetGroup = NiRenderTargetGroup::Create( mpRenderedTexture->GetBuffer(), r, true, false );
}

void cPostProcess::RenderImmediate( cRenderer* r )
{
//	if( r->IsDeviceLost() )
//		return;

	LPDIRECT3DDEVICE9 device = r->GetD3DDevice();
	IDirect3DVertexDeclaration9* oldVertDecl = 0;
	device->GetVertexDeclaration( &oldVertDecl );

	/// 셰이더를 시작
	if( mShader->Begin( r->GetD3DViewProj() ) == true )
	{
		mShader->CommitChanges();

		bool bS0Mipmap, bNonPow2, bChanged; 
		D3DBaseTexturePtr pkD3DTexture = r->GetTextureManager()->PrepareTextureForRendering( mpRenderedTexture, bChanged, bS0Mipmap, bNonPow2 );

		///
		device->SetTexture( 0, pkD3DTexture );
		device->SetVertexDeclaration( mVertexDeclaration );
		device->SetStreamSource( 0, mPosCoordBuffer, 0, sizeof(NiPoint3) );
		device->SetStreamSource( 1, mTexCoordBuffer, 0, sizeof(NiPoint2) );

		device->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

		mShader->End();
	}

	/// 장치 설정을 복원
	if( oldVertDecl )
		device->SetVertexDeclaration( oldVertDecl );
}

void cPostProcess::Render( cRenderer* r )
{
	if( r->BeginUsingDefaultRenderTargetGroup( NiRenderer::CLEAR_ALL ) == true )
	{
		r->SetScreenSpaceCameraData();

		RenderImmediate( r );

		UIMAN->Render();

#if (defined(_DEVSYS) || defined(_DEBUG))
		DEVSYSTEM->DrawDevInfo();
#endif
		r->EndUsingRenderTargetGroup();
	}
}


//////////////////////////////////////////////////////////////////////////
cRenderSystem::cRenderSystem( cAppWindow* pwindow )
: mpWindowRef(pwindow)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;


	/// 중개자들을 생성
	mpFontAgent = new cFontAgent;
	mpLightAgent = new cLightAgent;
	mpFogAgent = new cFogAgent;

	mPostProcess = new cPostProcess;

	mClearColor = NiColorA(0.2f,0.1f,0.1f,0.0f);

	/// It can filter flat images into mipmap pyramids, and can convert 
	/// from standard Gamebryo pixel formats (PAL8, PALA8, RGB24, RGBA32, etc)
	/// to non-standard formats, and from non-standard formats to a few standard formats
	NiImageConverter::SetImageConverter( NiNew NiDevImageConverter );
	NiTexture::SetMipmapByDefault( true );

	/// 장치 설정
	mCurrentAdapter = 0;
	mCurrentDevice = 0;
	mCurrentMode = 0;

	mReCreateContinue = false;
	mReCreateTime = 0;
	mWorldRendering = true;
}

cRenderSystem::~cRenderSystem()
{
	Exit();

	mBandiCaptureLib.Destroy();
	mVideoCaptureTime = 0;

	/// 중개자들을 제거
	SAFE_DELETE( mPostProcess );
	SAFE_DELETE( mpFogAgent );
	SAFE_DELETE( mpLightAgent );
	SAFE_DELETE( mpFontAgent );

	mRenderer = 0;

	mpSingleton = 0;
}

HWND cRenderSystem::GetHWND()
{ 
	return (mpWindowRef)? mpWindowRef->GetHWND():NULL; 
}

bool cRenderSystem::Init()
{
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm );

	mMonWidth = dm.dmPelsWidth;
	mMonHeight = dm.dmPelsHeight;
	mMonBpp = dm.dmBitsPerPel;

//	mMonWidth = ::GetSystemMetrics(SM_CXSCREEN);
//	mMonHeight = ::GetSystemMetrics(SM_CYSCREEN);

	/// DirectX 인스턴스 생성..
	/// Rendering을 주관해줄 Renderer 생성..
	/// 엔진에서 제공하는 NiRenderer를 활용
	if( CreateRenderer() == false )
	{
		return false;
	}

	if( mRenderer->Init() == false )
	{
		assert( 0 );
		return false;
	}

//	unsigned int verVS = mRenderer->GetShaderLibraryVersion()->GetSystemVertexShaderVersion();
//	unsigned int verPS = mRenderer->GetShaderLibraryVersion()->GetSystemPixelShaderVersion();

	if( mPostProcess->Init( mRenderer ) == false )
	{
		assert(0);
		return false;
	}

	/// FontAgent 초기화
	if( mpFontAgent->Init() == false )
	{
		return false;
	}

	/// LIghtAgent 초기화
	if( mpLightAgent->Init() == false )
	{
		return false;
	}

	mRenderer->AddResetNotificationFunc( g_Restore, this );

	SetBrightness( OPTIONMAN->GetGammaPer() );

	return true;
}

void cRenderSystem::Exit()
{
	/// 1. 종료시에 현재 적용된 최종 Render 옵션을 저장한다.
	if( mpFontAgent )
		mpFontAgent->Exit();
}

/// 주의 사항 : 불필요한 코드의 삽입을 금지한다.
void cRenderSystem::Render()
{
	if( mReCreateContinue == true )
	{
		mReCreateTime += THEAPP->GetWorldDeltaTime();
		if( mReCreateTime >= 30000 )
		{
			NiMessageBox("Window reset fail. Game exit.", "Error");
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
		}

		mReCreateContinue = false;
		THEAPP->UpdateScreenSize( mRenderOption.Width, mRenderOption.Height, mRenderOption.Bpp, OPTIONMAN->IsWindowed() );
		return;
	}

	if( mRenderer->BeginFrame() == false )
	{
		assert(0);
		return;
	}

	{
		/// 버퍼 클리어 컬러 지정
		mRenderer->SetBackgroundColor( mClearColor );

		///
		SCENEMAN->PreRender2D();
		UIMAN->PreRender();

		if( mRenderer->BeginUsingRenderTargetGroup( mPostProcess->GetRenderTarget(), NiRenderer::CLEAR_ALL ) == true )
		{
			if( mWorldRendering )
			{
				/// D3DTS_VIEW & D3DTS_PROJECTION 셋팅
				mRenderer->SetCameraData( CAMERAMAN->GetCurrentNi() );

				WORLDMAN->RenderSkyDome();

				WORLDMAN->Render();

				SCENEMAN->Render();
			}

			mRenderer->SetScreenSpaceCameraData();
			SCENEMAN->Render2D();

			mRenderer->SetCameraData( CAMERAMAN->GetCurrentNi() );
			SCENEMAN->RenderZFalseDamage();

			mRenderer->EndUsingRenderTargetGroup();

			mPostProcess->Render( mRenderer );
		}
//		mRenderer->EndFrame();
	}

	if( mBandiCaptureLib.IsCapturing() )
	{

		LPDIRECT3DDEVICE9 device = mRenderer->GetD3DDevice();
		mBandiCaptureLib.Work( (LONG_PTR)device );

		/// 1/1000 sec
		mVideoCaptureTime = mBandiCaptureLib.GetCaptureTime();

		unsigned int restTime = (unsigned int)(mVideoCaptureTime * 0.001f);
		unsigned int h = restTime / 3600;
		unsigned int m = restTime / 60;
		unsigned int s = restTime % 60 + 1;

		float captureSize = (float)mBandiCaptureLib.GetCaptureFileSize();
		captureSize = captureSize / (1024.0f*1024.0f);

		POINT pos;
		pos.x = 1;
		pos.y = 1;

		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText(10002), h, m, s, captureSize );

//		if( mRenderer->BeginFrame() == true )
//		{
			FONTAGENT->DrawText( cFontAgent::eFont_System, (LPTSTR)str.Cstr(), str.GetSize(), &pos, 0xFFFFFC31 );

//		}
	}
	mRenderer->EndFrame();


	mRenderer->DisplayFrame();
}

void cRenderSystem::Invalidate()
{
	mpFontAgent->Invalidate();
	mRenderer->OnLostDevice();
}

void cRenderSystem::Restore()
{
	mRenderer->OnResetDevice();
	mpFontAgent->Restore();
}

bool cRenderSystem::Reset( unsigned int width, unsigned int height, unsigned char bpp, bool winmode )
{
	NiDX9Renderer* pkD3drenderer = NiDynamicCast(NiDX9Renderer, NiDX9Renderer::GetRenderer());
	if( pkD3drenderer == 0 )
	{
		assert(0);
		return false;
	}

	NiWindowRef handle = mpWindowRef->GetHWND();
	if( handle == 0 )
	{
		assert(0);
		return false;
	}

	NiWindowRef kWndDevice, kWndFocus;
	pkD3drenderer->GetCreationParameters( mRenderOption.Width, mRenderOption.Height, mRenderOption.Flags, 
		kWndDevice, kWndFocus, mRenderOption.Adapter, mRenderOption.DevDesc, mRenderOption.FBformat, mRenderOption.DSformat, 
		mRenderOption.Interval, mRenderOption.Swapeffect, mRenderOption.FBmode, mRenderOption.Backbuffercount, mRenderOption.Refreshrate );

	mRenderOption.Width = width;
	mRenderOption.Height = height;
	mRenderOption.Bpp = bpp;

	///
	if( winmode == false )
	{
		mRenderOption.Flags = mRenderOption.Flags | NiDX9Renderer::USE_FULLSCREEN;
		mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_ONE;
	}
	else
	{
		if( mMonWidth < mRenderOption.Width || mMonHeight < mRenderOption.Height )
		{
			OPTIONMAN->SetWindowedNoReset( false );

			mRenderOption.Flags = mRenderOption.Flags | NiDX9Renderer::USE_FULLSCREEN;
			mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_ONE;
		}
		else
		{
			mRenderOption.Flags = mRenderOption.Flags & ~(NiDX9Renderer::USE_FULLSCREEN);
			mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;
		}
	}

	EnumResolutions( width, height, bpp, winmode );
	if( mCurrentMode == 0 )
	{
		assert(0);
		return false;
	}

	mRenderOption.FBformat = NiDX9Renderer::GetNiFBFormat( mCurrentMode->m_eD3DFormat );
	mRenderOption.DSformat = NiDX9Renderer::DSFMT_UNKNOWN;
	mRenderOption.FBmode = NiDX9Renderer::FBMODE_LOCKABLE;
//	mRenderOption.Refreshrate = NiDX9Renderer::REFRESHRATE_DEFAULT;
	mRenderOption.Backbuffercount = 1;

	NiDX9Renderer::RecreateStatus eStatus = pkD3drenderer->Recreate( mRenderOption.Width, mRenderOption.Height, mRenderOption.Flags, 
		handle, mRenderOption.FBformat, mRenderOption.DSformat, mRenderOption.Interval, 
		mRenderOption.Swapeffect, mRenderOption.FBmode, mRenderOption.Backbuffercount, mRenderOption.Refreshrate );

	if( eStatus == NiDX9Renderer::RECREATESTATUS_FAILED )
	{
		mReCreateContinue = true;
		return false;
	}

	mPostProcess->Recreate( mRenderer,  mRenderOption.Width, mRenderOption.Height, bpp );

	mReCreateTime = 0;
	return true;
}

bool cRenderSystem::CreateRenderer()
{
	/// 랜더러 초기화를 위한 옵션정보..
	/// 다이렉트 엑스 초기화
	/// 게임브리오 랜더러 초기화

	NiWindowRef handle = mpWindowRef->GetHWND();

	mRenderOption.Width = OPTIONMAN->GetWindowWidth();
	mRenderOption.Height = OPTIONMAN->GetWindowHeight();
	mRenderOption.Bpp = OPTIONMAN->GetWindowBpp();

	/// 생성 옵션 적용
	if( OPTIONMAN->IsWindowed() == false )
	{
		mRenderOption.Flags = mRenderOption.Flags | NiDX9Renderer::USE_FULLSCREEN;
		mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_ONE;
	}
	else
	{
		if( mMonWidth < mRenderOption.Width || mMonHeight < mRenderOption.Height )
		{
			OPTIONMAN->SetWindowedNoReset( false );

			mRenderOption.Flags = mRenderOption.Flags | NiDX9Renderer::USE_FULLSCREEN;
			mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_ONE;
		}
		else
		{
			mRenderOption.Flags = mRenderOption.Flags & ~(NiDX9Renderer::USE_FULLSCREEN);
			mRenderOption.Interval = NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE;
		}
	}

	/// main window update
	mpWindowRef->AdjustWindowForChange( IsFullScreenMode() );

	EnumAdapters();
	EnumResolutions( mRenderOption.Width, mRenderOption.Height, mRenderOption.Bpp, OPTIONMAN->IsWindowed() );

	if( mCurrentAdapter == 0  || mCurrentDevice == 0 || mCurrentMode == 0 )
	{
		/// message box 띄우자!! Default값으로 생성한다!!
		NiMessageBox( "Failed Device create!! Default option create!!", "Error Info" );

		/// 기본 해상도로 생성해 본다.
		mRenderOption.Width = 1024;
		mRenderOption.Height = 768;
		mRenderOption.Bpp = 32;

		/// main window update
		EnumAdapters();
		EnumResolutions( mRenderOption.Width, mRenderOption.Height, mRenderOption.Bpp, OPTIONMAN->IsWindowed() );
		if( mCurrentAdapter == 0 || mCurrentDevice == 0  || mCurrentMode == 0 )
		{
			NiMessageBox( "Failed Device create!! ShotDown Application.", "Error Info" );
			return false;
		}
	}

	NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;
	if( mCurrentDevice->GetDeviceType() == D3DDEVTYPE_REF )
		eDesc = NiDX9Renderer::DEVDESC_REF;

	/// Renderer Create
	mRenderer = cRenderer::Create( 1024, 768,
		(NiDX9Renderer::FlagType)mRenderOption.Flags &~NiDX9Renderer::USE_FULLSCREEN,
		handle,
		handle,
		mCurrentAdapter->GetAdapterIndex(),
		eDesc,
		NiDX9Renderer::GetNiFBFormat( mCurrentMode->m_eD3DFormat ),
		NiDX9Renderer::DSFMT_UNKNOWN,
		NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE,
		NiDX9Renderer::SWAPEFFECT_DISCARD,
		mRenderOption.FBmode );

	// dialog 앞으로 보내기
	LPDIRECT3DDEVICE9 device = mRenderer->GetD3DDevice();
	device->SetDialogBoxMode( true );
/*
	if( IsFullScreenMode() )
	{
		if( mCurrentAdapter && mCurrentDevice && mCurrentMode )
		{
			NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;
			if( mCurrentDevice->GetDeviceType() == D3DDEVTYPE_REF )
				eDesc = NiDX9Renderer::DEVDESC_REF;

			/// Renderer Create
			mRenderer = cRenderer::Create( mRenderOption.Width, mRenderOption.Height,
				(NiDX9Renderer::FlagType)mRenderOption.Flags,
				handle,
				handle,
				mCurrentAdapter->GetAdapterIndex(),
				eDesc,
				NiDX9Renderer::GetNiFBFormat( mCurrentMode->m_eD3DFormat ),
				NiDX9Renderer::DSFMT_UNKNOWN,
				NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE,
				NiDX9Renderer::SWAPEFFECT_DISCARD,
				mRenderOption.FBmode );
		}
		else
		{
			/// message box 띄우자!! Default값으로 생성한다!!
			NiMessageBox( "Failed Device create!! Default option create!!", "Error Info" );

			/// 기본 해상도로 생성해 본다.
			mRenderOption.Width = 1024;
			mRenderOption.Height = 768;
			mRenderOption.Bpp = 32;

			/// main window update
			EnumAdapters();
			EnumResolutions( mRenderOption.Width, mRenderOption.Height, mRenderOption.Bpp, OPTIONMAN->IsWindowed() );
			if( mCurrentAdapter == 0 || mCurrentDevice == 0  || mCurrentMode == 0 )
			{
				NiMessageBox( "Failed Device create!! ShotDown Application.", "Error Info" );
				return false;
			}

			NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;
			if( mCurrentDevice->GetDeviceType() == D3DDEVTYPE_REF )
				eDesc = NiDX9Renderer::DEVDESC_REF;

			/// Renderer Create
			mRenderer = cRenderer::Create( mRenderOption.Width, mRenderOption.Height,
				(NiDX9Renderer::FlagType)mRenderOption.Flags,
				handle,
				handle,
				mCurrentAdapter->GetAdapterIndex(),
				eDesc,
				NiDX9Renderer::GetNiFBFormat( mCurrentMode->m_eD3DFormat ),
				NiDX9Renderer::DSFMT_UNKNOWN,
				NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE,
				NiDX9Renderer::SWAPEFFECT_DISCARD,
				mRenderOption.FBmode );
		}
	}
	else
	{
		NiDX9Renderer::DeviceDesc eDesc = NiDX9Renderer::DEVDESC_PURE;
		if( mCurrentDevice->GetDeviceType() == D3DDEVTYPE_REF )
			eDesc = NiDX9Renderer::DEVDESC_REF;

		// create and return the renderer
		mRenderer = cRenderer::Create( mRenderOption.Width, mRenderOption.Height,
			(NiDX9Renderer::FlagType)mRenderOption.Flags,
			handle,
			handle,
			mCurrentAdapter->GetAdapterIndex(),
			eDesc,
			NiDX9Renderer::GetNiFBFormat( mCurrentMode->m_eD3DFormat ), 
			NiDX9Renderer::DSFMT_UNKNOWN,
			NiDX9Renderer::PRESENT_INTERVAL_IMMEDIATE,
			NiDX9Renderer::SWAPEFFECT_DISCARD,
			mRenderOption.FBmode );
	}
*/
	if( mRenderer == NULL )
	{
		NiMessageBox( "DX9 Renderer Creation Failed", "Error Info" );
		return false;
	}

//	mpWindowRef->AdjustWindowForChange( IsFullScreenMode() );
//	mpWindowRef->AdjustWindowRect( mRenderOption.Width, mRenderOption.Height, true );

	/// Setting dx background color
	mRenderer->SetBackgroundColor( mClearColor );

	return true;
}

void cRenderSystem::ToggleFullScreenMode()
{
	if( IsFullScreenMode() )
		mRenderOption.Flags = mRenderOption.Flags & ~(NiDX9Renderer::USE_FULLSCREEN);
	else
		mRenderOption.Flags = mRenderOption.Flags | NiDX9Renderer::USE_FULLSCREEN;
}

//////////////////////////////////////////////////////////////////////////
bool cRenderSystem::EnumAdapters()
{
	mCurrentAdapter = 0;
	mCurrentDevice = 0;

	const NiDX9SystemDesc* pSystem = NiDX9Renderer::GetSystemDesc();
	unsigned int adapterCount = pSystem->GetAdapterCount();

	bool checkAdapter = false;
	for ( unsigned int i = 0; i < adapterCount; i++ )
	{
		const NiDX9AdapterDesc* pAdapter = pSystem->GetAdapter(i);
		assert(pAdapter);

		const NiDX9DeviceDesc* pDevice = pAdapter->GetDevice( D3DDEVTYPE_HAL );
		if( !pDevice )
			continue;

		/// 윈도우 모드 지원 체크.
		if( pAdapter->CanDeviceRenderWindowed( D3DDEVTYPE_HAL ) == false )
			continue;

		/// 배열에 담아 두자
		mAdapterArr.PushBack( (void*)pAdapter );

		checkAdapter = true;
	}

	if( checkAdapter )
	{
		mCurrentAdapter = (NiDX9AdapterDesc*)mAdapterArr[0];
		if( mCurrentAdapter == 0 )
			return false;

		mCurrentDevice = (NiDX9DeviceDesc*)mCurrentAdapter->GetDevice( D3DDEVTYPE_HAL );
		assert( mCurrentDevice );
	}

	return checkAdapter;
}

void cRenderSystem::EnumResolutions( unsigned int curWidth, unsigned int curheight, unsigned int curBpp, bool windowed )
{
	if( mCurrentAdapter == 0 )
		return;

	mCurrentMode = 0;
	mResolutionArr.Clear();

	unsigned int modeCount = mCurrentAdapter->GetModeCount();

	for( unsigned int i=0; i<modeCount; i++ )
	{
		const NiDX9AdapterDesc::ModeDesc* pMode = mCurrentAdapter->GetMode( i );

		if( pMode == 0 )
			continue;

		/// 추가로 불필요한 모드는 제외시킨다.
		if( pMode->m_uiWidth <= 800 )
			continue;
		if( pMode->m_uiHeight <= 600 )
			continue;

		/// 배열에 담아 두자
		mResolutionArr.PushBack( (void*)pMode );

		/// 현재 모드 체크
		if( pMode->m_uiWidth == curWidth && pMode->m_uiHeight == curheight )
		{
			if( pMode->m_uiBPP == curBpp )
				mCurrentMode = (NiDX9AdapterDesc::ModeDesc*)pMode;
		}
	}

	if( mResolutionArr.GetSize() > 1)
		::Sort( mResolutionArr.Begin(), mResolutionArr.End(), cResolutionCompare() );
}

void cRenderSystem::EnumMultisamples()
{
/*
	if( mCurrentDevice == 0 )
		return;

	D3DFORMAT eformat;
	if( mCurrentMode )
	{
		NiDX9AdapterDesc::ModeDesc* pMode = (NiDX9AdapterDesc::ModeDesc*)mCurrentMode;
		eformat = pMode->m_eD3DFormat;
	}
	else
	{
		unsigned int bitdepth = 

	}
*/
}

#include "PlaneObject.h"
bool cRenderSystem::RenderOnce( NiTexture* tex )
{
	cPlaneObject* pPlane = new cPlaneObject;
	if( pPlane->Create( tex, 0, 0, (unsigned short)GetScreenWidth(), (unsigned short)GetScreenHeight(),
		0, 0, (unsigned short)tex->GetWidth(), (unsigned short)tex->GetHeight() ) == false )
	{
		return true;
	}

	if( mRenderer == 0 || mRenderer->BeginFrame() == false )
	{
		SAFE_DELETE(pPlane);
		return false;
	}

	mRenderer->SetBackgroundColor( mClearColor );
	if( mRenderer->BeginUsingDefaultRenderTargetGroup( NiRenderer::CLEAR_ALL ) == true )
	{
		mRenderer->SetScreenSpaceCameraData();
		pPlane->Draw();

		mRenderer->EndUsingRenderTargetGroup();
	}
	mRenderer->EndFrame();

	if( mRenderer->DisplayFrame() == false )
	{
		SAFE_DELETE(pPlane);
		return false;
	}

	SAFE_DELETE(pPlane);
	return true;
}

void cRenderSystem::SetBrightness( float br )
{
	if( mPostProcess )
		mPostProcess->mShader->SetBright( br );
}

void cRenderSystem::SetMonitorDisplay( unsigned int width, unsigned int height, unsigned int bpp )
{
	if( mMonWidth == width && mMonHeight == height && mMonBpp == bpp )
		return;

	if( IsFullScreenMode() == false )
		mReCreateContinue = true;
	else
		return;

	mMonWidth = width; 
	mMonHeight = height;
	mMonBpp = bpp;
}


void cRenderSystem::ToggleVideoCapture()
{
	if( mRenderer == 0 )
		return;

	LPDIRECT3DDEVICE9 device = mRenderer->GetD3DDevice();
	if( device == 0 )
		return;

	// 캡처를 시작합니다.
	if( mBandiCaptureLib.IsCapturing() == FALSE )
	{
		// 최초 실행시 DLL 을 로드하고 내부 인스턴스를 생성합니다. 
		if( mBandiCaptureLib.IsCreated() == FALSE )
		{
			// 실패시 DLL과 .H파일의 버전이 동일한지 여부등을 확인해 보시기 바랍니다.
			if( FAILED(mBandiCaptureLib.Create(BANDICAP_RELEASE_DLL_FILE_NAME)) )
			{
				assert(0);
				return;
			}

//			// 인증을 받지 않으면 인코딩된 동영상 상단에 로고가 표시됩니다. 
//			// 정식구매후 발급받은 키를 사용하거나, 홈페이지에서 데모용으로 공개된 키를 사용해서 인증을 받습니다. 
			if( FAILED(mBandiCaptureLib.Verify( "EYA_IRIS_20091228", "74f9454b" )) )
			{
				assert(0);
			}
		}

		if( mBandiCaptureLib.IsCreated() )
		{
			BCAP_CONFIG cfg;

			/// option mode check...
			int videoMode = OPTIONMAN->GetVideoMode();
			switch( videoMode )
			{
			case BCAP_PRESET_320x240:
			case BCAP_PRESET_640x480:
			case BCAP_PRESET_800x600:
			case BCAP_PRESET_1024x768:
			case BCAP_PRESET_1280x720:
			case BCAP_PRESET_GAME:
				break;
			default:
				videoMode = BCAP_PRESET_GAME;
			}

			// 프리셋을 통해서 손쉽게 설정이 가능하며, 직접 파라미터 설정을 바꿀 수도 있습니다.
			BCapConfigPreset(&cfg, (BCAP_PRESET)videoMode ); 
			if( videoMode == BCAP_PRESET_GAME )
			{
				cfg.VideoSizeW = mRenderOption.Width;
				cfg.VideoSizeH = mRenderOption.Height;
			}

			mBandiCaptureLib.CheckConfig(&cfg);       // 잘못된 설정 바로 잡기
			mBandiCaptureLib.SetConfig(&cfg);         // 설정 적용

			mBandiCaptureLib.SetMinMaxFPS(30, 60);    // 최소, 최대 프레임 설정

			if( FileExist("VideoCapture") == false )
			{
				::CreateDirectory( L"VideoCapture", 0 );
			}

			// 현재시간을 이용해서 파일명을 만듭니다. 
			mBandiCaptureLib.MakePathnameByDate( _T(".\\VideoCapture"), _T("IrisOnline"), _T("avi"), mVideoPathName, MAX_PATH );

			// 캡처를 시작합니다.
			HRESULT hr = mBandiCaptureLib.Start(mVideoPathName, NULL, BCAP_MODE_D3D9_SCALE, (LONG_PTR)device);
			if( FAILED(hr) )
			{
				ASSERT(0);
				return;
			}

			mVideoCaptureTime = 0;
		}
	}
	else
	{
		mBandiCaptureLib.Stop();

		if( STAGEMAN &&  mVideoCaptureTime != 0 )
			STAGEMAN->AddVideoCaptureMsg( mVideoPathName );

		mVideoCaptureTime = 0;
	}
}

bool cRenderSystem::IsVideoCapturing()
{
	if( mBandiCaptureLib.IsCreated() && mBandiCaptureLib.IsCapturing() )
		return true;
	else
		return false;
}

void cRenderSystem::SaveScreenShot()
{
	if( mRenderer == 0 )
		return;

	LPDIRECT3DDEVICE9 device = mRenderer->GetD3DDevice();
	if( device == 0 )
		return;

	if( mBandiCaptureLib.IsCreated() == FALSE )
	{
		// 실패시 DLL과 .H파일의 버전이 동일한지 여부등을 확인해 보시기 바랍니다.
		if( FAILED(mBandiCaptureLib.Create(BANDICAP_RELEASE_DLL_FILE_NAME)) )
		{
			assert(0);
			return;
		}

//		// 인증을 받지 않으면 인코딩된 동영상 상단에 로고가 표시됩니다. 
//		// 정식구매후 발급받은 키를 사용하거나, 홈페이지에서 데모용으로 공개된 키를 사용해서 인증을 받습니다. 
		if( FAILED(mBandiCaptureLib.Verify( "EYA_IRIS_20091228", "74f9454b" )) )
			assert(0);
	}

	///
	if( ::FileExist( "ScreenShot" ) == false )
	{
		::CreateDirectory( L"ScreenShot", 0 );
	}

	// get (target directory + filename by date)
	TCHAR szCaptureFilePathName[MAX_PATH*2];
	mBandiCaptureLib.MakePathnameByDate(_T(".\\ScreenShot"), _T("Capture"), _T("jpg"), szCaptureFilePathName, MAX_PATH*2);

	INT nQuality = 100;
	if( FAILED(mBandiCaptureLib.CaptureImage(szCaptureFilePathName, BCAP_IMAGE_JPG, nQuality, BCAP_MODE_D3D9, FALSE, (LONG_PTR)device)) )
		return;

	STAGEMAN->AddScreenShotMsg( szCaptureFilePathName );
}
