//-----------------------------------------------------------------------------
// File : GTAO_PS.hlsl
// Desc : Grand Truth Based Ambient Occlusion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef SLICE_COUNT
#define SLICE_COUNT     (4)    // スライス数.
#endif//SLICE_COUNT

#ifndef STEP_COUNT
#define STEP_COUNT      (4)     // ステップ数.
#endif//STEP_COUNT

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// GTAOParam structure
///////////////////////////////////////////////////////////////////////////////
struct GTAOParam
{
    float2  InvSize;        // レンダーターゲットサイズの逆数.
    float   RadiusSS;       // スクリーン空間の半径.
    float   RadiusVS;       // ビュー空間の半径.
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4 View;
    float4x4 Proj;
    float4x4 InvView;
    float4x4 InvProj;
    float    NearClip;
    float    FarClip;
    float    FieldOfView;
    float    AspectRatio;
};

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float PI = 3.1415926535897932384626433832795f;
static const float HALF_PI = 1.5707963267948966192313216916398f;
static const float FLT_MAX = 3.402823466e+38f; // 浮動小数の最大値.

//-----------------------------------------------------------------------------
// Textures and Samplers
//-----------------------------------------------------------------------------
Texture2D<float>    DepthMap        : register(t0);
Texture2D<float2>   NormalMap       : register(t1);
SamplerState        PointSampler    : register(s0);
SamplerState        LinearSampler   : register(s1);

//-----------------------------------------------------------------------------
// Constant Buffers.
//-----------------------------------------------------------------------------
ConstantBuffer<SceneParam> Scene : register(b1);
ConstantBuffer<GTAOParam>  Param : register(b2);

//-----------------------------------------------------------------------------
//      精度浮動小数の最大値未満に飽和させます.
//-----------------------------------------------------------------------------
float SaturateFloat(float value)
{ return clamp(value, 0.0f, FLT_MAX); }

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
float3 UnpackNormal(float2 packed)
{
    // Octahedron normal vector encoding.
    // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
    float2 encoded = packed * 2.0f - 1.0f;
    float3 n = float3(encoded.x, encoded.y, 1.0f - abs(encoded.x) - abs(encoded.y));
    float  t = saturate(-n.z);
    n.xy += (n.xy >= 0.0f) ? -t : t;
    return normalize(n);
}

//-----------------------------------------------------------------------------
//      ハードウェアから出力されたReverse-Zをビュー空間深度に変換します.
//-----------------------------------------------------------------------------
float ToViewDepthFromReverseZ(float hardwareDepth, float nearClip)
{
    return -nearClip / hardwareDepth;
}

//-----------------------------------------------------------------------------
//      ビュー空間位置を求めます.
//-----------------------------------------------------------------------------
float3 ToViewPos(float2 uv)
{
    float hardwareZ = DepthMap.SampleLevel(LinearSampler, uv, 0.0f);
    float viewZ = ToViewDepthFromReverseZ(hardwareZ, Scene.NearClip);
    float2 p = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    p *= -viewZ * float2(1.0f / Scene.Proj._11, 1.0f / Scene.Proj._22);
    return float3(p, viewZ);
}

//-----------------------------------------------------------------------------
//      R1数列.
//-----------------------------------------------------------------------------
float R1Sequence(float number)
{
    // Martin Roberts, "The Unreasonable Effectiveness of Quasirandom Sequences", 2018
    // http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

    // g  = 1.6180339887498948482;
    // a1 = 1 / g = 0.6180339887498948482
    return frac(0.5f + number * 0.6180339887498948482);
}

//-----------------------------------------------------------------------------
//      R2数列.
//-----------------------------------------------------------------------------
float2 R2Sequence(float number)
{
    // g = 1.32471795724474602596
    // a1 = 1 / g       = 0.75487766624669276005
    // a2 = 1 / (g * g) = 0.5698402909980532659114;
    return frac(0.5f.xx + number * float2(0.75487766624669276005, 0.5698402909980532659114));
}

//-------------------------------------------------------------------------
float Pow2(float x)
{ return x * x; }

//-----------------------------------------------------------------------------
//      エントリーポイントです.
//-----------------------------------------------------------------------------
float main(const VSOutput input) : SV_TARGET0
{
    float2 uv = input.TexCoord;

    // 中心の位置座標.
    float3 p0 = ToViewPos(uv);

    // ピクセル半径.
    float radiusPixels = abs(Param.RadiusSS / p0.z);

    // 1 ピクセル未満の場合は処理しない.
    if (radiusPixels < 1.0f)
    {
        return 1.0f;
    }

    // 中心の法線ベクトル.
    float3 n0 = UnpackNormal(NormalMap.SampleLevel(LinearSampler, uv, 0.0f));
    n0 = mul((float3x3)Scene.View, n0);
    n0 = normalize(n0);

    float3 m = float3(0.06711056f, 0.0233486, 52.9829189f);
    float2 noise = R2Sequence(m.z * dot(input.Position.xy, m.xy));

    float3 view = normalize(-p0);
    float visibility = 0.0f;

    const float pixelTooCloseThreshold  = 1.3f;
    const float minS = pixelTooCloseThreshold / radiusPixels;
    const float invRadius = 1.0f / Pow2(Param.RadiusVS);

    [unroll]
    for(float slice = 0; slice < SLICE_COUNT; ++slice)
    {
        float sliceK = (slice + noise.x) / float(SLICE_COUNT);
        float phi = sliceK * PI;

        float cosPhi, sinPhi;
        sincos(phi, sinPhi, cosPhi);
        float2 omega = float2(cosPhi, -sinPhi) * radiusPixels;

        const float3 dir         = float3(cosPhi, sinPhi, 0);
        const float3 orthoDir    = dir - dot(dir, view) * view;
        const float3 axisV       = normalize(cross(orthoDir, view));
        const float3 projN       = n0 - axisV * dot(n0, axisV);
        const float  projNLength = length(projN);

        // gammaを求める.
        float sgnG = (float)sign(dot(orthoDir, projN));
        float cosG = saturate(dot(projN, view) / projNLength);
        float g    = sgnG * acos(cosG);
        float sinG = sin(g);

        float horizonCos0 = -1.0f;
        float horizonCos1 = -1.0f;

        [unroll]
        for(float step = 0; step < STEP_COUNT; ++step)
        {
            float stepNoise = R1Sequence(slice + step * STEP_COUNT);
            stepNoise = frac(stepNoise + noise.y);
            float s = (step + stepNoise) / float(STEP_COUNT);
            s = Pow2(s);
            s += minS;

            // hat_t(phi) * r.
            float2 offset = round(s * omega) * Param.InvSize;

            // hat_s
            float2 uv0 = uv - offset;
            float2 uv1 = uv + offset;

            // ビュー空間位置を求める.
            float3 pos0 = ToViewPos(uv0);
            float3 pos1 = ToViewPos(uv1);

            float3 diff0 = pos0 - p0;
            float3 diff1 = pos1 - p0;

            // 距離.
            float dist0 = dot(diff0, diff0);
            float dist1 = dot(diff1, diff1);

            // ベクトル正規化.
            float3 horizonV0 = diff0 * rsqrt(dist0);
            float3 horizonV1 = diff1 * rsqrt(dist1);

            // 水平角をサンプル.
            float cos0 = dot(horizonV0, view);
            float cos1 = dot(horizonV1, view);

            // 距離で減衰させる
            float falloff0 = dist0 * invRadius;
            float falloff1 = dist1 * invRadius;

            horizonCos0 = max(horizonCos0, cos0 - falloff0);
            horizonCos1 = max(horizonCos1, cos1 - falloff1);
        }

        // Equation(10).
        float t0 = -acos(horizonCos0);
        float t1 =  acos(horizonCos1);

        // 半球上に制限.
        t0 = g + clamp(t0 - g, -HALF_PI, HALF_PI);
        t1 = g + clamp(t1 - g, -HALF_PI, HALF_PI);

        // Equation(7).
        float hat_a = (cosG + 2 * t0 * sinG - cos(2 * t0 - g)) * 0.25f
                    + (cosG + 2 * t1 * sinG - cos(2 * t1 - g)) * 0.25f;

        // Equation(9).
        visibility += projNLength * hat_a;
    }

    // 正規化.
    visibility /= float(SLICE_COUNT);

    return visibility;
};