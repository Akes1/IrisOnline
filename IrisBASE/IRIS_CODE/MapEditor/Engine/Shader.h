/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.02.03
* 내    용 : 지형 셰이더
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

protected:
	cCamera* mCamera;
	LPD3DXEFFECT mEffect;
	D3DXHANDLE mHTechnique;
	D3DXHANDLE mHWorldViewProj;
	D3DXHANDLE mHAmbientLight;
	D3DXHANDLE mHFogColor;
	D3DXHANDLE mHFogEnd;
	D3DXHANDLE mHFogRange;
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
	D3DXHANDLE mHOrgX;
	D3DXHANDLE mHOrgY;
	D3DXHANDLE mHPointLightPos;
	D3DXHANDLE mHPointLightDiffuse;
	D3DXHANDLE mHPointLightAtten;
};
