/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.05.14
* 내    용 : 렌더러
* 주의사항 : 
*===========================================================================*/
#pragma once

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
		SwapEffect eSwapEffect = SWAPEFFECT_DEFAULT,
		unsigned int uiFBMode = FBMODE_DEFAULT,
		unsigned int uiBackBufferCount = 1, 
		unsigned int uiRefreshRate = REFRESHRATE_DEFAULT);

	/// 초기화
	bool Init();

	/// 버퍼
	LPDIRECT3DVERTEXBUFFER9 CreateVertexBuffer( unsigned int vertCount, unsigned int vertStride, const void* vertArray );
	LPDIRECT3DINDEXBUFFER9 CreateIndexBuffer( unsigned int indexCount, const unsigned short* indexArray );

	/// 이펙트
	LPD3DXEFFECT GetVColorEffect() const;
	LPD3DXEFFECT GetTerrainEffect() const;
	LPD3DXEFFECT GetNaviMeshEffect() const;
	LPD3DXEFFECT GetNaviFieldEffect() const;
	LPD3DXEFFECT GetTrailEffect() const;

	///
	D3DXMATRIX GetD3DViewProj() const;

public:
	void OnLostDevice();
	void OnResetDevice();

protected:
	bool CreateEffect( const cString& pathName, LPD3DXEFFECT* effect );

protected:
	/// 이펙트
	LPD3DXEFFECT mVColorEffect;
	LPD3DXEFFECT mTerrainEffect;
	LPD3DXEFFECT mNaviMeshEffect;
	LPD3DXEFFECT mNaviFieldEffect;
	LPD3DXEFFECT mTrailEffect;
};

inline
LPD3DXEFFECT cRenderer::GetVColorEffect() const
{
	return mVColorEffect;
}

inline
LPD3DXEFFECT cRenderer::GetTerrainEffect() const
{
	return mTerrainEffect;
}

inline
LPD3DXEFFECT cRenderer::GetNaviMeshEffect() const
{
	return mNaviMeshEffect;
}

inline
LPD3DXEFFECT cRenderer::GetNaviFieldEffect() const
{
	return mNaviFieldEffect;
}

inline
LPD3DXEFFECT cRenderer::GetTrailEffect() const
{
	return mTrailEffect;
}

inline
D3DXMATRIX cRenderer::GetD3DViewProj() const
{
	D3DXMATRIX view( GetD3DView() );
	D3DXMATRIX proj( GetD3DProj() );
	return view * proj;
}

typedef NiPointer<cRenderer> cRendererPtr;
