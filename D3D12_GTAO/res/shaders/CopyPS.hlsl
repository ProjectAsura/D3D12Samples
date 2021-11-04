//-----------------------------------------------------------------------------
// File : CopyPS.hlsl
// Desc : Copy Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------
Texture2D       InputMap        : register(t0);
SamplerState    PointSampler    : register(s0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    return InputMap.SampleLevel(PointSampler, input.TexCoord, 0.0f).xxxx;
}
