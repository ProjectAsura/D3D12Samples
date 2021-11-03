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
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    return InputMap.SampleLevel(PointSampler, input.TexCoord, 0.0f).xxxx;
}
