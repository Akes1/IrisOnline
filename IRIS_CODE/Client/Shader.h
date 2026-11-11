/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.02.03
* 내    용 : 셰이더
* 주의사항 : 
*===========================================================================*/
#pragma once

class cCamera;
class cLightSceneNode;

/// 셰이더
class cShader
{
public:
	cShader();

	/// 초기화
	virtual void Init( LPD3DXEFFECT effect );

	///
	bool Begin( cCamera* cam, const D3DXMATRIX& worldViewProj );
	void End();

	///
	void CommitChanges();
	void SetAmbientLight( NiAmbientLight* ambientLight );
	void SetFog( NiFogProperty* fogProp );

	//void SetTexture( LPDIRECT3DBASETEXTURE9 tex );

protected:
	virtual void SetEffect();

protected:
	cCamera* mCamera;
	LPD3DXEFFECT mEffect;
	D3DXHANDLE mHTechnique;
	D3DXHANDLE mHWorldViewProj;
	D3DXHANDLE mHAmbientLight;
	D3DXHANDLE mHFogColor;
	D3DXHANDLE mHFogEnd;
	D3DXHANDLE mHFogRange;
	//D3DXHANDLE mHTexture;
};

inline
void cShader::CommitChanges()
{
	mEffect->CommitChanges();
}

/// 지형 셰이더
class cTerrainShader : public cShader
{
public:
	cTerrainShader();

	/// 초기화
	void Init( LPD3DXEFFECT effect );

	///
	void SetOrigin( float x, float y );
	void SetPointLight( cLightSceneNode* light );

protected:
	virtual void SetEffect();

protected:
	D3DXHANDLE mHOrgX;
	D3DXHANDLE mHOrgY;
	D3DXHANDLE mHPointLightPos;
	D3DXHANDLE mHPointLightDiffuse;
	D3DXHANDLE mHPointLightAtten;
};

class cBrightShader 
{
public:
	cBrightShader();

	void Init( LPD3DXEFFECT effect );

	bool Begin( const D3DXMATRIX& worldViewProj );
	void End();

	///
	void CommitChanges();

	void SetBright( float br );

	///
protected:
	LPD3DXEFFECT mEffect;
	D3DXHANDLE mHTechnique;
	D3DXHANDLE mHWorldViewProj;
	D3DXHANDLE mHBright;
};

inline
void cBrightShader::CommitChanges()
{
	mEffect->CommitChanges();
}


/// 바이너리 셰이더
#include <NiBinaryShader.h>
class cBinaryShader : public NiBinaryShader
{
public:
	bool Initialize( const cString& path );

protected:
	bool LoadVertexShaderProgram( NiD3DPass& pass, const cString& path );
	bool LoadPixelShaderProgram( NiD3DPass& pass, const cString& path );

protected:
	cString mPath;
};

/// NSB 셰이더
#include <NSBShader.h>

class cNSBShader : public NSBShader
{
public:
	// Get the BINARY shader
	NiBinaryShader* GetBinaryShader( NiD3DRenderer* renderer, const cString& path, unsigned int implementation = NiShader::DEFAULT_IMPLEMENTATION );
};

typedef NiPointer<cNSBShader> cNSBShaderPtr;
