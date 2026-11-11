#include "stdafx.h"
#include "Shader.h"

#include "Camera.h"
#include "SceneManager.h"
//#include "LightSceneNode.h"

#include "RenderSystem.h"
#include "Renderer.h"

cShader::cShader()
: mCamera( 0 )
, mEffect( 0 )
, mHTechnique( 0 )
, mHWorldViewProj( 0 )
, mHAmbientLight( 0 )
, mHFogColor( 0 )
, mHFogEnd( 0 )
, mHFogRange( 0 )
//, mHTexture( 0 )
{
}

void cShader::Init( LPD3DXEFFECT effect )
{
	assert( effect );

	mEffect = effect;
	mHTechnique = effect->GetTechniqueByName( "tech0" );
	mHWorldViewProj = effect->GetParameterBySemantic( 0, "WorldViewProjection" );
	mHAmbientLight = effect->GetParameterByName( 0, "gAmbientLight" );
	mHFogColor = effect->GetParameterByName( 0, "gFogColor" );
	mHFogEnd = effect->GetParameterByName( 0, "gFogEnd" );
	mHFogRange = effect->GetParameterByName( 0, "gFogRange" );
	//mHTexture = effect->GetParameterByName( 0, "gTex" );
}

bool cShader::Begin( cCamera* cam, const D3DXMATRIX& worldViewProj )
{
	SetEffect();

	assert( cam );
	mCamera = cam;

	/// 이펙트를 준비
	mEffect->SetTechnique( mHTechnique );
	mEffect->SetMatrix( mHWorldViewProj, &worldViewProj );

	/// 이펙트를 시작!
	if( mEffect->Begin( 0, 0 ) != D3D_OK )
		return false;

	mEffect->BeginPass( 0 );
	return true;
}

void cShader::End()
{
	mEffect->EndPass();
	mEffect->End();
}

void cShader::SetEffect()
{
	cRenderer* r = RENDERSYS->GetRenderer();
	mEffect = r->GetTrailEffect();

	mHTechnique = mEffect->GetTechniqueByName( "tech0" );
	mHWorldViewProj = mEffect->GetParameterBySemantic( 0, "WorldViewProjection" );
	mHAmbientLight = mEffect->GetParameterByName( 0, "gAmbientLight" );
	mHFogColor = mEffect->GetParameterByName( 0, "gFogColor" );
	mHFogEnd = mEffect->GetParameterByName( 0, "gFogEnd" );
	mHFogRange = mEffect->GetParameterByName( 0, "gFogRange" );
}

void cShader::SetAmbientLight( NiAmbientLight* ambientLight )
{
	if( mHAmbientLight )
	{
		if( ambientLight )
		{
			NiColor ambientColor = ambientLight->GetAmbientColor();
			ambientColor *= ambientLight->GetDimmer();
			mEffect->SetFloatArray( mHAmbientLight, (const float*)&ambientColor, 3 );
		}
		else
		{
			mEffect->SetFloatArray( mHAmbientLight, (const float*)&NiColor::WHITE, 3 );
		}
	}
}

void cShader::SetFog( NiFogProperty* fogProp )
{
	if( mHFogEnd && mHFogRange )
	{
		if( fogProp )
		{
			float nearDist = mCamera->GetNearDistance();
			float farDist = mCamera->GetFarDistance();
			float fogStart = nearDist + (farDist - 10.0f - nearDist) * (1.0f - fogProp->GetDepth() );
			float fogEnd = farDist;
			float fogRange = fogEnd - fogStart;

			mEffect->SetFloat( mHFogEnd, fogEnd );
			mEffect->SetFloat( mHFogRange, fogRange );
		}
		else
		{
			float nearDist = mCamera->GetNearDistance();
			float farDist = mCamera->GetFarDistance();
			float fogStart = nearDist + (farDist - 10.0f - nearDist) * 0.5f;
			float fogEnd = farDist;
			float fogRange = fogEnd - fogStart;

			mEffect->SetFloat( mHFogEnd, fogEnd );
			mEffect->SetFloat( mHFogRange, fogRange );
		}
	}

	if( mHFogColor )
	{
		if( fogProp )
		{
			mEffect->SetFloatArray( mHFogColor, (const float*)&fogProp->GetFogColor(), 3 );
		}
		else
		{
			mEffect->SetFloatArray( mHFogColor, (const float*)&NiColor::WHITE, 3 );
		}
	}
}

//void cShader::SetTexture( LPDIRECT3DBASETEXTURE9 tex )
//{
//	if( mHTexture )
//	{
//		mEffect->SetTexture( mHTexture, tex );
//	}
//}

cTerrainShader::cTerrainShader()
: mHOrgX( 0 )
, mHOrgY( 0 )
, mHPointLightPos( 0 )
, mHPointLightDiffuse( 0 )
, mHPointLightAtten( 0 )
{
}

void cTerrainShader::Init( LPD3DXEFFECT effect )
{
	assert( effect );

	cShader::Init( effect );

	mHOrgX = effect->GetParameterByName( 0, "gOrgX" );
	mHOrgY = effect->GetParameterByName( 0, "gOrgY" );
	mHPointLightPos = effect->GetParameterByName( 0, "gPointLightPos" );
	mHPointLightDiffuse = effect->GetParameterByName( 0, "gPointLightDiffuse" );
	mHPointLightAtten = effect->GetParameterByName( 0, "gPointLightAtten" );
}

void cTerrainShader::SetOrigin( float x, float y )
{
	if( mHOrgX && mHOrgY )
	{
		mEffect->SetFloat( mHOrgX, x );
		mEffect->SetFloat( mHOrgY, y );
	}
}
/*
void cTerrainShader::SetPointLight( cLightSceneNode* light )
{
	if( mHPointLightPos )
	{
		if( light )
		{
			mEffect->SetFloatArray( mHPointLightPos, (const float*)&light->GetWorldTranslate(), 3 );
		}
		else
		{
			mEffect->SetFloatArray( mHPointLightPos, (const float*)&NiPoint3::ZERO, 3 );
		}
	}
	if( mHPointLightDiffuse )
	{
		if( light )
		{
			mEffect->SetFloatArray( mHPointLightDiffuse, (const float*)&light->GetDiffuse(), 3 );
		}
		else
		{
			mEffect->SetFloatArray( mHPointLightDiffuse, (const float*)&NiColor::WHITE, 3 );
		}
	}
	if( mHPointLightAtten )
	{
		if( light )
		{
			float atten[3] = { light->GetConstantAtten(), light->GetLinearAtten(), light->GetQuadricAtten() };
			mEffect->SetFloatArray( mHPointLightAtten, (const float*)atten, 3 );
		}
	}
}
*/

void cTerrainShader::SetEffect()
{
	cRenderer* r = RENDERSYS->GetRenderer();
	mEffect = r->GetTerrainEffect();

	mHTechnique = mEffect->GetTechniqueByName( "tech0" );
	mHWorldViewProj = mEffect->GetParameterBySemantic( 0, "WorldViewProjection" );
	mHAmbientLight = mEffect->GetParameterByName( 0, "gAmbientLight" );
	mHFogColor = mEffect->GetParameterByName( 0, "gFogColor" );
	mHFogEnd = mEffect->GetParameterByName( 0, "gFogEnd" );
	mHFogRange = mEffect->GetParameterByName( 0, "gFogRange" );

	mHOrgX = mEffect->GetParameterByName( 0, "gOrgX" );
	mHOrgY = mEffect->GetParameterByName( 0, "gOrgY" );
	mHPointLightPos = mEffect->GetParameterByName( 0, "gPointLightPos" );
	mHPointLightDiffuse = mEffect->GetParameterByName( 0, "gPointLightDiffuse" );
	mHPointLightAtten = mEffect->GetParameterByName( 0, "gPointLightAtten" );
}
