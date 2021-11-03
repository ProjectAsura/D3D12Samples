//-----------------------------------------------------------------------------
// File : CrossBilateralFilterPS.hlsl
// Desc : Cross-Bilateral Filter for SSAO.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define KARNEL_RADIUS   (5)

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// BlurParam structure
///////////////////////////////////////////////////////////////////////////////
struct BlurParam
{
    float2 SampleDir;
    float  BlurSharpness;
    float  Reserved;
};

//-----------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------
ConstantBuffer<BlurParam>   Param           : register(b3);
Texture2D<float>            DepthMap        : register(t0);
Texture2D<float>            AoMap           : register(t1);
SamplerState                PointSampler    : register(s0);
SamplerState                LinearSampler   : register(s1);

//-----------------------------------------------------------------------------
//      クロスバイラテラルフィルタの重みを求めます.
//-----------------------------------------------------------------------------
float CrossBilateralWeight(float r, float d, float d0)
{
    // Louis Bavoil, Johan Andersson, 
    // "Stable SSAO in Battlefield 3 with Selective Tempoaral Filtering"
    // GDC 2012, Bonus Slides.
    const float BlurSigma = (float)(KARNEL_RADIUS + 1.0) * 0.5f;
    const float BlurFallOff = 1.0f / (2.0f * BlurSigma * BlurSigma);

    float dz = d0 - d * Param.BlurSharpness;
    return exp2(-r * r * BlurFallOff - dz * dz);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float main(const VSOutput input) : SV_TARGET0
{
    float2 uv0 = input.TexCoord;

    float ao0 = AoMap.Sample(PointSampler, uv0);
    float z0  = DepthMap.Sample(PointSampler, uv0);

    float totalAO = ao0;
    float totalW  = 1.0f;
    float r;

    //=================
    //　プラス方向.
    //=================
    // Inner half of the kernel: step size = 1 and POINT filtering
    r = 1.0f;
    for(; r<=KARNEL_RADIUS/2; r+=1)
    {
        float2 uv = uv0 + r * Param.SampleDir;
        float ao = AoMap.Sample(PointSampler, uv);
        float z  = DepthMap.Sample(PointSampler, uv);

        float w = CrossBilateralWeight(r, z, z0);

        totalAO += w * ao;
        totalW  += w;
    }
    // Outer half of the kernel: step size = 2 and LINEAR filtering
    for(; r<=KARNEL_RADIUS; r+=2)
    {
        float2 uv = uv0 + (r + 0.5f) * Param.SampleDir;
        float ao = AoMap.Sample(LinearSampler, uv);
        float z  = DepthMap.Sample(LinearSampler, uv);

        float w = CrossBilateralWeight(r, z, z0);

        totalAO += w * ao;
        totalW  += w;
    }

    //=================
    //　マイナス方向.
    //=================
    // Inner half of the kernel: step size = 1 and POINT filtering
    r = 1.0f;
    for(; r<=KARNEL_RADIUS/2; r+=1)
    {
        float2 uv = uv0 - r * Param.SampleDir;
        float ao = AoMap.Sample(PointSampler, uv);
        float z  = DepthMap.Sample(PointSampler, uv);

        float w = CrossBilateralWeight(r, z, z0);

        totalAO += w * ao;
        totalW  += w;
    }
    // Outer half of the kernel: step size = 2 and LINEAR filtering
    for(; r<=KARNEL_RADIUS; r+=2)
    {
        float2 uv = uv0 - (r + 0.5f) * Param.SampleDir;
        float ao = AoMap.Sample(LinearSampler, uv);
        float z  = DepthMap.Sample(LinearSampler, uv);

        float w = CrossBilateralWeight(r, z, z0);

        totalAO += w * ao;
        totalW  += w;
    }

    return saturate(totalAO / totalW);
}