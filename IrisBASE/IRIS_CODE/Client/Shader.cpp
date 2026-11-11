#include "stdafx.h"
#include "Shader.h"

#include "Camera.h"
#include "SceneManager.h"
#include "LightSceneNode.h"

#include "RenderSystem.h"

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
//////////////////////////////////////////////////////////////////////////
cBrightShader::cBrightShader()
: mEffect(0),
mHTechnique(0),
mHWorldViewProj(0),
mHBright(0)
{

}

void cBrightShader::Init( LPD3DXEFFECT effect )
{
	assert( effect );

	mEffect = effect;
	mHTechnique = effect->GetTechniqueByName( "tech0" );
	mHWorldViewProj = effect->GetParameterBySemantic( 0, "WorldViewProjection" );
	mHBright = effect->GetParameterByName( 0, "gBrightness" );
}

bool cBrightShader::Begin( const D3DXMATRIX& worldViewProj )
{
	/// 이펙트를 준비
	mEffect->SetTechnique( mHTechnique );
	mEffect->SetMatrix( mHWorldViewProj, &worldViewProj );

	/// 이펙트를 시작!
	if( mEffect->Begin( 0, 0 ) != D3D_OK )
		return false;

	mEffect->BeginPass( 0 );
	return true;
}

void cBrightShader::End()
{
	mEffect->EndPass();
	mEffect->End();
}

void cBrightShader::SetBright( float br )
{
	mEffect->SetFloat( mHBright, 0.6f*br - 0.3f );
}

//////////////////////////////////////////////////////////////////////////
bool cBinaryShader::Initialize( const cString& path )
{
	if (!m_bInitialized)
	{
		if (!NiD3DShader::Initialize())
			return false;

		// We are going to cycle through the passes and load the shader
		// programs. That should be all we need to do at this point!
		NiD3DPass* pkPass;
		unsigned int uiCount = m_kPasses.GetSize();
		for (unsigned int ui = 0; ui < uiCount; ui++)
		{
			pkPass = m_kPasses.GetAt(ui);
			if (pkPass)
			{
				if (!LoadVertexShaderProgram(*pkPass, path))
				{
					// Implement a failure response.
				}
				if (!LoadPixelShaderProgram(*pkPass, path))
				{
					// Implement a failure response.
				}
			}
		}
		m_bInitialized = true;
	}
	return m_bInitialized;
}

bool cBinaryShader::LoadVertexShaderProgram( NiD3DPass& pass, const cString& path )
{
	const char* pcProgramFileName = pass.GetVertexShaderProgramFileName();

	// If there is no program to load, it's not a failure.
	// Just return true!
	if (!pcProgramFileName || (strcmp(pcProgramFileName, "") == 0))
		return true;

	char acTrueFileName[_MAX_PATH];
	if (!ResolveVertexShaderFileName(pcProgramFileName, acTrueFileName,
		_MAX_PATH))
	{
		return false;
	}

	// Construct a shader name from the shader file and entry point
	const char* pcShaderEntryPoint = 
		pass.GetVertexShaderProgramEntryPoint();
	char acShaderName[_MAX_PATH];
	if (pcShaderEntryPoint != NULL && *pcShaderEntryPoint != '\0')
	{
		NIASSERT(strlen(pcProgramFileName) + strlen(pcShaderEntryPoint) + 2 
			< _MAX_PATH);
		NiSprintf(acShaderName, _MAX_PATH, "%s##%s", pcProgramFileName, 
			pcShaderEntryPoint);
	}
	else
	{
		NiStrcpy(acShaderName, _MAX_PATH, pcProgramFileName);
	}

	// We need to utilize the usage flag here...
	cString pathName;
	pathName.Format( "%s%s", path.Cstr(), acTrueFileName );

	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 );
		return false;
	}

	NiD3DVertexShader* vs = NiD3DShaderProgramFactory::CreateVertexShaderFromBuffer(
		loader.GetBufferPtr(), loader.GetSize(), pathName.Cstr(), 
		acShaderName, pcShaderEntryPoint, pass.GetVertexShaderProgramShaderTarget(), 
		NULL, 0 );

	if( vs == 0 )
	{
		assert( 0 );
		return false;
	}

	pass.SetVertexShader(vs);

	return true;
}

bool cBinaryShader::LoadPixelShaderProgram( NiD3DPass& pass, const cString& path )
{
	const char* pcProgramFileName = pass.GetPixelShaderProgramFileName();

	// If there is no program to load, it's not a failure.
	// Just return true!
	if (!pcProgramFileName || (strcmp(pcProgramFileName, "") == 0))
		return true;

	char acTrueFileName[_MAX_PATH];
	if (!ResolvePixelShaderFileName(pcProgramFileName, acTrueFileName, 
		_MAX_PATH))
	{
		return false;
	}

	// Construct a shader name from the shader file and entry point
	const char* pcShaderEntryPoint = 
		pass.GetPixelShaderProgramEntryPoint();
	char acShaderName[_MAX_PATH];
	if (pcShaderEntryPoint != NULL && *pcShaderEntryPoint != '\0')
	{
		NIASSERT(strlen(pcProgramFileName) + strlen(pcShaderEntryPoint) + 2 
			< _MAX_PATH);
		NiSprintf(acShaderName, _MAX_PATH, "%s##%s", pcProgramFileName, 
			pcShaderEntryPoint);
	}
	else
	{
		NiStrcpy(acShaderName, _MAX_PATH, pcProgramFileName);
	}

	///
	cString pathName;
	pathName.Format( "%s%s", path.Cstr(), acTrueFileName );

	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 );
		return false;
	}

	NiD3DPixelShader* ps = 
		NiD3DShaderProgramFactory::CreatePixelShaderFromBuffer(
		loader.GetBufferPtr(), loader.GetSize(), pathName.Cstr(),
		acShaderName, pcShaderEntryPoint, 
		pass.GetPixelShaderProgramShaderTarget() );

	if( ps == 0 )
	{
		assert( 0 );
		return false;
	}

	pass.SetPixelShader(ps);

	return true;
}

#include <NSBPass.h>

NiBinaryShader* cNSBShader::GetBinaryShader( NiD3DRenderer* renderer, const cString& path, unsigned int implementation )
{
	NSBImplementation* pkImplementation = 0;

	VersionInfo kVersionInfo;
	SetupVersionInfo(kVersionInfo);

	bool bBestImplementation = false;

	if (implementation != NiShader::DEFAULT_IMPLEMENTATION &&
		implementation < m_kImplementationArray.GetSize())
	{
		// Grab the proper implementation
		pkImplementation = m_kImplementationArray.GetAt(implementation);
		if (!IsImplementationValid(pkImplementation, kVersionInfo))
		{
			NiD3DRenderer::Warning("NSBShader::GetD3DShader - %s - "
				"Requested implementation (%d) invalid on operating "
				"hardware.\n", GetName(), implementation);
			pkImplementation = 0;
		}
		else
		{
			if( pkImplementation == GetBestImplementation() )
			{
				bBestImplementation = true;
			}
		}
	}

	if (!pkImplementation)
	{
		// Get the best for the hardware
		pkImplementation = GetBestImplementation();
		if (!pkImplementation)
		{
			NiD3DRenderer::Warning("NSBShader::GetD3DShader - %s - Unable "
				"to find valid implementation for hardware.\n",
				GetName());
			return 0;
		}
		bBestImplementation = true;
	}

	// This will ALWAYS be set - at least to the default.
	//NiBinaryShader* pkShader = ms_pfnCreateNiBinaryShader(
	//	pkImplementation->GetClassName());
	//if (!pkShader)
	//{
	//	pkShader = NiNew NiBinaryShader();
	//}
	cBinaryShader* pkShader = NiNew cBinaryShader();

	// We better have a shader by now!
	assert(pkShader);

	pkShader->SetD3DRenderer(renderer);
	pkShader->SetName(GetName());
	pkShader->SetImplementation(implementation);
	pkShader->SetIsBestImplementation(bBestImplementation);

	pkShader->SetUserDefinedDataSet(m_spUserDefinedDataSet);
	pkShader->SetImplementationUserDefinedDataSet(
		pkImplementation->GetUserDefinedDataSet());
	NSBPass* pkPass;
	for (unsigned int ui = 0; ui < pkImplementation->GetPassCount(); ui++)
	{
		pkPass = pkImplementation->GetPass(ui, false);
		if (pkPass)
		{
			NSBUserDefinedDataSet* pkUDDSet = 
				pkPass->GetUserDefinedDataSet();
			pkShader->SetPassUserDefinedDataSet(ui, pkUDDSet);
		}
	}

	if (!RegisterTextureStageGlobals(pkImplementation))
	{
		// Determine how to handle the error case
		NiRenderer::Warning("%s - %s - Failed "
			"to register global texture stage variables.\n",
			__FUNCTION__, GetName());
	}

	if (!pkImplementation->SetupNiBinaryShader(*pkShader, m_spShaderDesc))
	{
		NiRenderer::Error("%s - %s - Failed to setup the binary shader.\n",
			__FUNCTION__, GetName());
		NiDelete pkShader;
		return 0;
	}

	// If the implementation has both a valid semantic adapter table (SAT)
	// and a packing defintion, then the SAT wins, otherwise, the packing
	// def is used to construct a NiShaderDeclaration, which is in turn used
	// to create a SAT.
	if ( pkShader->GetSemanticAdapterTable().GetNumFilledTableEntries() > 0 )
	{
		// Already found a SAT, do we have a PackingDef too?
		if (pkImplementation->GetPackingDef())
		{
			NiRenderer::Warning("%s - %s - Both a SemanticAdapterTable and "
				"a PackingDef are present in the '%s' implementation. "
				"The PackingDef will be ignored.\n",
				__FUNCTION__, pkImplementation->GetName(), GetName());
		}
	}
	else if (pkImplementation->GetPackingDef())
	{
		NSBPackingDef* pkPackingDef = GetPackingDef(
			pkImplementation->GetPackingDef(), false);
		if (!pkPackingDef)
		{
			NiRenderer::Error("%s - %s - Failed "
				"to find packing definition for implementation.\n",
				__FUNCTION__, GetName());
			NIASSERT(!"Failed to find packing def!");
			NiDelete pkShader;
			return 0;
		}
		else
		{
			// Create the shader declaration from the packing def
			NiShaderDeclarationPtr spShaderDecl = pkPackingDef->
				GetShaderDeclaration(NiShader::NISHADER_AGNOSTIC);
			if (!spShaderDecl)
			{
				NiRenderer::Error("%s - %s - "
					"Failed to convert packing definition to "
					"NiShaderDeclaration.\n", __FUNCTION__, GetName());
				NIASSERT(!"Failed to convert packing def!");
				NiDelete pkShader;
				return 0;
			}

			// Create the semantic adapter table from the shader
			// declaration
			pkShader->SetSemanticAdapterTableFromShaderDeclaration(spShaderDecl);

			// Determine if the shader is requesting BlendIndices as colors.
			// If this is the case, then set the flag on the NiD3DShader
			// to enable this conversion.
			bool bConvert = false;
			unsigned int uiStreamCount = spShaderDecl->GetStreamCount();
			for (unsigned int uiS = 0; uiS < uiStreamCount; uiS++)
			{
				unsigned int uiEntryCount = spShaderDecl->GetEntryCount(uiS);
				for (unsigned int uiE = 0; uiE < uiEntryCount; uiE++)
				{
					const NiShaderDeclaration::ShaderRegisterEntry* pkEntry =
						spShaderDecl->GetEntry(uiE, uiS);

					if (pkEntry->m_eType == 
						NiShaderDeclaration::SPTYPE_UBYTECOLOR && 
						NiShaderDeclaration::StringToUsage(
						pkEntry->m_kUsage) ==
						NiShaderDeclaration::SPUSAGE_BLENDINDICES)
					{
						bConvert = true;
						break;
					}
				}
			}

			pkShader->SetConvertBlendIndicesToD3DColor(bConvert);
		}
	}
	else
	{
		NiRenderer::Warning("%s - %s - Neither a PackingDef or a "
			"SemanticAdapterTable was present, constructing a default "
			"SemanticAdapterTable containing only POSITION0.\n",
			__FUNCTION__, GetName());

		// There is no decl, so we rely on the default behavior and
		// create a semantic adapter table that needs only position
		pkShader->SetSemanticAdapterTableFromShaderDeclaration(NULL);
	}

	if (!ReleaseTextureStageGlobals( pkImplementation))
	{
		// Determine how to handle the error case
		NiRenderer::Warning("%s - %s - Failed to release global texture "
			"stage variables.\n",
			__FUNCTION__, GetName());
	}

	if( !pkShader->Initialize( path ) )
	{
		NiRenderer::Warning("%s - %s -  Failed to initialize shader.\n",
			__FUNCTION__, GetName());
		NiDelete pkShader;
		return NULL;
	}

	return pkShader;
}
