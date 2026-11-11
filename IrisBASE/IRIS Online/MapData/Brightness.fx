float4x4 gWorldViewProj : WorldViewProjection;
float gBrightness;

texture gTex0 <string NTM = "shader";>;

sampler gTexture0 = sampler_state
{
    Texture = (gTex0);
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
    MipFilter = Linear;
    MinFilter = Linear;
    MagFilter = Linear;
};

//---------------------------------------------------------------------------
// ¡§¡° ºŒ¿Ã¥ı
//---------------------------------------------------------------------------
struct VShaderOutput
{
    float4 pos : POSITION0;
    float2 tex : TEXCOORD0;
};

VShaderOutput vmain( float3 pos : POSITION0, float2 tex : TEXCOORD0 )
{
    VShaderOutput o;

    o.pos = mul( float4(pos, 1.0f), gWorldViewProj );
    o.tex = tex;

    return o;
}

//---------------------------------------------------------------------------
// «»ºø ºŒ¿Ã¥ı
//---------------------------------------------------------------------------
float4 pmain( float2 tex : TEXCOORD ) : COLOR
{
  float4 result = tex2D(gTexture0, tex);

  result = (result + clamp(gBrightness, -0.3, 0.3));

  return result;
}

technique tech0
{
    pass pass0
    {
	Lighting = False;
	AlphaBlendEnable = False;
	ZEnable = True;
	ZFunc = LESSEQUAL;
	ZWriteEnable = False;
	CullMode = NONE;

	VertexShader = compile vs_1_0 vmain();
	PixelShader  = compile ps_1_4 pmain();
    }
}
