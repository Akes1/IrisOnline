// ============================================================================
// 작 성 자 : 이순규
// 작 성 일 : 2008.02.15
// 내    용 : 지형 셰이더
// 주의사항 : 
// ============================================================================

float4x4 gWorldViewProj : WorldViewProjection;
float3 gAmbientLight;
float3 gPointLightPos;
float3 gPointLightDiffuse;
float3 gPointLightAtten;
float3 gFogColor;
float gFogEnd;
float gFogRange;
float gOrgX;
float gOrgY;

texture gTex0;
texture gTex1;
texture gTex2;

sampler gDetailMap0 = sampler_state
{
    Texture = (gTex0);
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

sampler gDetailMap1 = sampler_state
{
    Texture = (gTex1);
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

sampler gDetailMap2 = sampler_state
{
    Texture = (gTex2);
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU = Wrap;
    AddressV = Wrap;
};

//---------------------------------------------------------------------------
// 정점 셰이더
//---------------------------------------------------------------------------
struct VShaderOutput
{
    float4 pos : POSITION0;
    float3 color: COLOR0;
    float3 alpha : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    float fog : TEXCOORD2;
};

float3 ProcessLighting( float3 pos, float3 normal )
{
	float d = distance( pos, gPointLightPos );
	float atten = 1.0f / (gPointLightAtten.x + gPointLightAtten.y * d + gPointLightAtten.z * d * d);
	float3 lightDir = normalize(gPointLightPos - pos);
	
	return gPointLightDiffuse * (atten * max(dot(normal, lightDir), 0.0f));
}

VShaderOutput vmain(
    float3 pos : POSITION0,
    float3 normal : NORMAL0,
    float3 color : COLOR0,
    float3 alpha : TEXCOORD0
)
{
    VShaderOutput o;
    
    o.pos = mul( float4(pos, 1.0f), gWorldViewProj );

	if( gPointLightPos.x >= 1.0f )
	{
		o.color = gAmbientLight * color + ProcessLighting( pos, normal );
	}
	else
	{
		o.color = gAmbientLight * color;
	}
    o.alpha = alpha;
    o.texCoord.x = ((pos.x - gOrgX) / 100.0f) * 0.25f;
    o.texCoord.y = ((pos.y - gOrgY) / 100.0f) * 0.25f;
    o.fog = saturate( (gFogEnd - o.pos.z) / gFogRange );
    
    return o;
}

//---------------------------------------------------------------------------
// 픽셀 셰이더
//---------------------------------------------------------------------------
struct PShaderOutput
{
    float4 color : COLOR0;
};

PShaderOutput pmain(
    float3 color : COLOR0,
    float3 alpha : TEXCOORD0,
    float2 texCoord : TEXCOORD1,
    float fog : TEXCOORD2
)
{
    PShaderOutput o;

    float3 color0 = tex2D(gDetailMap0, texCoord).rgb * alpha.r + tex2D(gDetailMap1, texCoord).rgb * alpha.g + tex2D(gDetailMap2, texCoord).rgb * alpha.b;
    float3 color1 = color0 * color;
    float3 color2 = lerp( gFogColor, color1, fog );
    
    o.color = float4(color2, 1.0f);

    return o;
}

//-----------------------------------------------------------------------------
// 이펙트
//-----------------------------------------------------------------------------
technique tech0
{
    pass pass0
    {
		Lighting = False;
		AlphaBlendEnable = False;
		AlphaTestEnable = False;
		StencilEnable = False;
		ZEnable = True;
		ZFunc = LESSEQUAL;
		ZWriteEnable = True;
		CullMode = CCW;
		FogEnable = False;
		SpecularEnable = False;

		VertexShader = compile vs_1_1 vmain();
		PixelShader  = compile ps_1_4 pmain();
    }
}
