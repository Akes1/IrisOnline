// ============================================================================
// 작 성 자 : 이순규
// 작 성 일 : 2008.03.13
// 내    용 : 궤적 셰이더
// 주의사항 : 
// ============================================================================

float4x4 gWorldViewProj : WorldViewProjection;
float3 gFogColor;
float gFogEnd;
float gFogRange;

texture gTex;

sampler gSampler = sampler_state
{
    Texture = (gTex);
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU = Clamp;
    AddressV = Clamp;
};

//---------------------------------------------------------------------------
// 정점 셰이더
//---------------------------------------------------------------------------
struct VShaderOutput
{
    float4 pos : POSITION0;
    float2 tex : TEXCOORD0;
    float alpha : TEXCOORD1;
    float fog : TEXCOORD2;
};

VShaderOutput vmain(
    float3 pos : POSITION0,
    float2 tex : TEXCOORD0,
    float alpha : COLOR0
)
{
    VShaderOutput o;

    o.pos = mul( float4(pos, 1.0f), gWorldViewProj );
    o.tex = tex;
    o.alpha = alpha;
    o.fog = clamp( (gFogEnd - o.pos.z) / gFogRange, 0, 1 );

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
	float2 tex : TEXCOORD0,
    float alpha : TEXCOORD1,
    float fog : TEXCOORD2
)
{
    PShaderOutput o;

	float4 color0 = tex2D(gSampler, tex).rgba;
    float3 color1 = lerp( gFogColor, color0.rgb, fog );
    
    o.color = float4(color1, alpha * color0.a);

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
		LightEnable[0] = False;
		AlphaBlendEnable = True;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		AlphaTestEnable = False;
		StencilEnable = False;
		ZEnable = True;
		ZFunc = LESSEQUAL;
		ZWriteEnable = False;
		CullMode = NONE;
		FogEnable = False;
		SpecularEnable = False;

		VertexShader = compile vs_1_0 vmain();
		PixelShader  = compile ps_1_0 pmain();
    }
}
