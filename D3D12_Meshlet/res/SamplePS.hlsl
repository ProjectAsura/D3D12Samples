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
ConstantBuffer<LightingParam>   CbLighting : register(b3);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const MSOutput input) : SV_TARGET0
{
    float3 V = float3(0.0f, 0.0f, 1.0f);
    float3 N = normalize(input.Normal);
    float3 L = normalize(CbLighting.LightDir);
    float3 R = normalize(-reflect(V, N));

    float3 color = float3(0.25f, 1.0f, 0.25f);
    float  shininess = 2000.0f;

    // Lambert.
    float3 lighting = 1.0f / F_PI;

    // Phong.
    lighting += pow(saturate(dot(L, R)), shininess) * ((shininess + 2.0f) / (2.0f * F_PI));

    // レンダリング方程式の余弦項.
    lighting *= saturate(dot(N, L));

    return float4(lighting * color, 1.0f);
}