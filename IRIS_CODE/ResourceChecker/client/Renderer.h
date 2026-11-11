/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.05.14
* 내    용 : 렌더러
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Shader.h"


class cFxShader;

/// 렌더러
class cRenderer : public NiDX9Renderer
{
public:
	cRenderer();
	virtual ~cRenderer();

	/// 생성
	static cRenderer* Create(unsigned int uiWidth, unsigned int uiHeight,
		unsigned int uiUseFlags, NiWindowRef kWndDevice, 
		NiWindowRef kWndFocus, unsigned int uiAdapter = D3DADAPTER_DEFAULT, 
		DeviceDesc eDesc = DEVDESC_PURE,
		FrameBufferFormat eFBFormat = FBFMT_UNKNOWN, 
		DepthStencilFormat eDSFormat = DSFMT_UNKNOWN, 
		PresentationInterval ePresentationInterval = PRESENT_INTERVAL_ONE,
		SwapEffect eSwapEffect = SWAPEFFECT_DISCARD,
		unsigned int uiFBMode = FBMODE_LOCKABLE,
		unsigned int uiBackBufferCount = 1, 
		unsigned int uiRefreshRate = REFRESHRATE_DEFAULT);

	/// 초기화
	bool Init();

	/// 버퍼
	LPDIRECT3DVERTEXBUFFER9 CreateVertexBuffer( unsigned int vertexCount, unsigned int vertexStride, const void* data );
	LPDIRECT3DINDEXBUFFER9 CreateIndexBuffer( unsigned int indexCount, const unsigned short* indexArray );

//	/// 셰이더
//	NiShader* GetShadowShader();

	/// 이펙트
	LPD3DXEFFECT GetTerrainEffect() const;
	LPD3DXEFFECT GetTrailEffect() const;
	LPD3DXEFFECT GetBrightEffect() const;

	///
	D3DXMATRIX GetD3DViewProj() const;

public:
	void OnLostDevice();
	void OnResetDevice();

protected:
	bool CreateEffect( const cString& pathName, LPD3DXEFFECT* effect );

protected:
	/// 셰이더
	NiTPointerList<cNSBShaderPtr> mNSBShaderList;
//	NiShaderPtr mShadowShader;

	/// 이펙트
	LPD3DXEFFECT mTerrainEffect;
	LPD3DXEFFECT mTrailEffect;
	LPD3DXEFFECT mBrightEffect;
};

//inline
//NiShader* cRenderer::GetShadowShader()
//{
//	return mShadowShader;
//}

inline
LPD3DXEFFECT cRenderer::GetTerrainEffect() const
{
	return mTerrainEffect;
}

inline
LPD3DXEFFECT cRenderer::GetTrailEffect() const
{
	return mTrailEffect;
}

inline
LPD3DXEFFECT cRenderer::GetBrightEffect() const
{
	return mBrightEffect;
}


inline
D3DXMATRIX cRenderer::GetD3DViewProj() const
{
	D3DXMATRIX view( GetD3DView() );
	D3DXMATRIX proj( GetD3DProj() );
	return view * proj;
}

typedef NiPointer<cRenderer> cRendererPtr;
