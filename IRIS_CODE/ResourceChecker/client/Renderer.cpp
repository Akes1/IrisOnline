#include "stdafx.h"
#include "Renderer.h"

#include <NiDX9PersistentSrcTextureRendererData.h>

#if defined(_DX9)
#pragma comment(lib, "NiBinaryShaderLib.lib")
#pragma comment(lib, "NSBShaderLib.lib")
#pragma comment(lib, "NiD3DXEffectShaderLib.lib")
#endif

cRenderer::cRenderer()
: mTerrainEffect(0)
, mTrailEffect(0)
, mBrightEffect(0)
//, mShadowShader(0)
{
}

cRenderer::~cRenderer()
{
	/// 이펙트를 제거
	if( mTrailEffect )
		mTrailEffect->Release();

	if( mTerrainEffect )
		mTerrainEffect->Release();

	if( mBrightEffect )
		mBrightEffect->Release();
}

cRenderer* cRenderer::Create( unsigned int uiWidth, 
							 unsigned int uiHeight, unsigned int uiUseFlags, NiWindowRef kWndDevice, 
							 NiWindowRef kWndFocus, unsigned int uiAdapter, DeviceDesc eDesc, 
							 FrameBufferFormat eFBFormat, DepthStencilFormat eDSFormat, 
							 PresentationInterval ePresentationInterval, SwapEffect eSwapEffect, 
							 unsigned int uiFBMode, unsigned int uiBackBufferCount, 
							 unsigned int uiRefreshRate )
{
	// Create the Direct3D9 interface
	if( CreateDirect3D9() == -1)
	{
		SetLastErrorString("Creation failed: Could not initialize DirectX9");
		Error("cRenderer::Create> Invalid device ID specified "
			"- FAILING\n");
		return 0;
	}

	// Create an 'empty' instance of the renderer
	cRenderer* r = NiNew cRenderer;
	assert(r);

	ms_kD3DCriticalSection.Lock();

	bool bSuccess = r->Initialize( uiWidth, uiHeight, uiUseFlags, kWndDevice, 
		kWndFocus, uiAdapter, eDesc, eFBFormat, eDSFormat, 
		ePresentationInterval, eSwapEffect, uiFBMode, uiBackBufferCount, 
		uiRefreshRate );

	ms_kD3DCriticalSection.Unlock();

	if( bSuccess == false )
	{
		NiDelete r;
		r = 0;
	}
	else
	{
		// Registration of the DX9-specific
		// NiDX9PersistentSrcTextureRendererData::LoadBinary() that overrides
		// NiPersistentSrcTextureRendererData::LoadBinary();
		NiStream::UnregisterLoader( "NiPersistentSrcTextureRendererData" );
		NiStream::RegisterLoader( "NiPersistentSrcTextureRendererData", NiDX9PersistentSrcTextureRendererData::CreateObject );
	}

	return r;
}

bool cRenderer::Init()
{
	///
	//cFileLoader loader;
	//if( loader.Open( "./Script/Resource/ShadowShader.nsb", true ) == false )
	//{
	//	assert( 0 );
	//	return false;
	//}

	//cNSBShader* nsbShader = NiNew cNSBShader;
	//NiMemStream stream( loader.GetBufferPtr(), loader.GetSize() );

	//if( nsbShader->LoadBinary( stream ) == false )
	//{
	//	assert( 0 );
	//	return false;
	//}

	//mNSBShaderList.AddTail( nsbShader );
//	mShadowShader = (NiShader*)(nsbShader->GetBinaryShader( this, "./Script/Resource/" ));

	/// 이펙트를 생성
	if( CreateEffect( "./Script/Resource/Brightness.fx", &mBrightEffect ) == false || mBrightEffect == 0 )
	{
		assert( 0 );
		return false;
	}
	if( CreateEffect( "./Script/Resource/TerrainShader.fx", &mTerrainEffect ) == false || mTerrainEffect == 0 )
	{
		assert( 0 );
		return false;
	}
	if( CreateEffect( "./Script/Resource/TrailShader.fx", &mTrailEffect ) == false || mTrailEffect == 0 )
	{
		assert( 0 );
		return false;
	}

	return true;
}

bool cRenderer::CreateEffect( const cString& pathName, LPD3DXEFFECT* effect )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
		return false;

	/// D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
	/// 위 플래그는 ps_1_x를 더이상 지원하지 않는 dx버젼을 위한 재생 플래그
	LPD3DXBUFFER error = 0;
#ifdef D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
	HRESULT hr = ::D3DXCreateEffect( m_pkD3DDevice9, loader.GetBufferPtr(), loader.GetSize(), 0, 0, D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, effect, &error );
#else
	HRESULT hr = ::D3DXCreateEffect( m_pkD3DDevice9, loader.GetBufferPtr(), loader.GetSize(), 0, 0, 0, 0, effect, &error );
#endif

	if( FAILED(hr) )
	{
		const char* c = (const char*)error->GetBufferPointer();
		c;
		assert( 0 );
	}
	if( error )
		error->Release();

	if( FAILED(hr) )
		return false;
	else
		return true;
}

LPDIRECT3DVERTEXBUFFER9 cRenderer::CreateVertexBuffer( unsigned int vertexCount, unsigned int vertexStride, const void* data )
{
	LPDIRECT3DVERTEXBUFFER9 vb = 0;
	unsigned int length = vertexCount * vertexStride;

	HRESULT ret = m_pkD3DDevice9->CreateVertexBuffer( length, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &vb, 0 );

	if( FAILED(ret) || vb == 0 )
		return 0;

	/// 생성된 버퍼에 내용 복사
	if( data )
	{
		void* p = 0;
		HRESULT ret = vb->Lock( 0, length, &p, 0 );

		if( SUCCEEDED(ret) )
		{
			NiMemcpy( p, (const void*)data, length );
		}
		vb->Unlock();
	}
	return vb;
}

LPDIRECT3DINDEXBUFFER9 cRenderer::CreateIndexBuffer( unsigned int indexCount, const unsigned short* indexArray )
{
	assert( m_pkD3DDevice9 );

	LPDIRECT3DINDEXBUFFER9 ib = 0;
	unsigned int length = indexCount * sizeof(unsigned short);

	HRESULT ret = m_pkD3DDevice9->CreateIndexBuffer( length, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, 0 );

	if( FAILED(ret) || ib == 0 )
		return 0;

	/// 생성된 버퍼에 내용 복사
	if( indexArray )
	{
		void* p = 0;
		HRESULT ret = ib->Lock( 0, length, &p, 0 );

		if( SUCCEEDED(ret) )
		{
			NiMemcpy( p, (const void*)indexArray, length );
		}
		ib->Unlock();
	}
	return ib;
}

void cRenderer::OnLostDevice()
{
	if( mTerrainEffect )
		mTerrainEffect->OnLostDevice();
	if( mTrailEffect )
		mTrailEffect->OnLostDevice();
	if( mBrightEffect )
		mBrightEffect->OnLostDevice();
}

void cRenderer::OnResetDevice()
{
	if( mTerrainEffect )
		mTerrainEffect->OnResetDevice();
	if( mTrailEffect )
		mTrailEffect->OnResetDevice();
	if( mBrightEffect )
		mBrightEffect->OnResetDevice();
}
