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
// HBAOParam structure
///////////////////////////////////////////////////////////////////////////////
struct HBAOParam
{
    float2  InvSize;    // レンダーターゲットサイズの逆数.
    float   RadiusSS;   // スクリーン空間の半径.
    float   InvRadius2; // -1.0 / (Radius * Radius).
    float   Bias;       // バイアス.
    float   Intensity;  // 強度.
    float   Sigma;      // シグマ.
    float   Reserved;
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
ConstantBuffer<HBAOParam>  Param : register(b2);

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

float R1Sequence(float number)
{
    return frac(number * 0.6180339887498948482);
}

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
    float  noise = frac(m.z * frac(dot(input.Position.xy, m.xy)));

    float3 view = normalize(-p0);
    float visibility = 0.0f;

    const float pixelTooCloseThreshold  = 1.3f;
    const float minS = pixelTooCloseThreshold / radiusPixels;

    [unroll]
    for(float slice = 0; slice < SLICE_COUNT; ++slice)
    {
        float sliceK = (slice + noise) / float(SLICE_COUNT);
        float phi = sliceK * PI;

        float cosPhi, sinPhi;
        sincos(phi, sinPhi, cosPhi);
        float2 omega = float2(cosPhi, -sinPhi) * radiusPixels;

        const float3 dir         = float3(cosPhi, sinPhi, 0);
        const float3 orthoDir    = dir - (dot(dir, view) * view);
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
            float s = (step + stepNoise) / float(STEP_COUNT) + minS;

            // hat_t(phi) * r.
            float2 offset = round(s * omega) * Param.InvSize;

            // hat_s
            float2 uv0 = uv - offset;
            float2 uv1 = uv + offset;

            float3 pos0 = ToViewPos(uv0);
            float3 pos1 = ToViewPos(uv1);

            float3 horizonV0 = normalize(pos0 - p0);
            float3 horizonV1 = normalize(pos1 - p0);

            horizonCos0 = max(horizonCos0, dot(horizonV0, view));
            horizonCos1 = max(horizonCos1, dot(horizonV1, view));
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