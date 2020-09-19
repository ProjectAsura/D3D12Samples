//-----------------------------------------------------------------------------
// File : SamplePS.hlsl
// Desc : Sample Pixel Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

///////////////////////////////////////////////////////////////////////////////
// MSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct MSOutput
{
    float4  Position    : SV_POSITION;
    float2  TexCoord    : TEXCOORD;
    float3  Normal      : NORMAL;
    float3  Tangent     : TANGENT;
};

///////////////////////////////////////////////////////////////////////////////
// LightingParam structure
///////////////////////////////////////////////////////////////////////////////
struct LightingParam
{
    float3  LightDir;
    float   LightIntensity;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
ConstantBuffer<LightingParam>   LightingBuffer : register(b0);


float4 main(const MSOutput input) : SV_TARGET0
{
    float3 color = float3(0.5f, 0.5f, 0.5f);
    float3 lighting = color * dot(input.Normal, LightingBuffer.LightDir) / F_PI;
    return float4(lighting, 1.0f);
}