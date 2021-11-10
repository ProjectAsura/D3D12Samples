//-----------------------------------------------------------------------------
// File : ScalableAO_PS.hlsl
// Desc : Scalable Ambient Obscurance.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT        (16)    // サンプル数.
#endif//SAMPLE_COUNT

#ifndef ENABLE_VOGEL_DISK_SAMPLE
#define ENABLE_VOGEL_DISK_SAMPLE    (1) // フォーゲルディスクサンプル.
#endif//ENABLE_VOGEL_DISK_SAMPLE

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// ScalableAOParam structure
///////////////////////////////////////////////////////////////////////////////
struct ScalableAOParam
{
    float2  InvSize;    // レンダーターゲットサイズの逆数.
    float   RadiusSS;   // スクリーン空間の半径.
    float   InvRadius2; // -1.0 / (Radius * Radius).
    float   Bias;       // バイアス.
    float   Intensity;  // 強度.
    float   Sigma;      // シグマ.
    float   MaxMipLevels;
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
static const float TAU[] = {
    1,   1,  2,  3,  2,  5,  2,  3,  2,  3,  3,  5,  5,
    3,   4,  7,  5,  5,  7,  9,  8,  5,  5,  7,  7,  7, 8, 5, 8, 11, 12, 7,
    10, 13,  8, 11,  8,  7, 14, 11, 11, 13, 12, 13, 19, 17, 13,
    11, 17, 19, 18, 25, 18, 19, 19, 29, 21, 19, 27, 31, 29, 21,
    18, 17, 29, 31, 31, 23, 18, 25, 26, 25, 23, 19, 34, 19, 27,
    21, 25, 39, 29, 17, 27
};

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
ConstantBuffer<SceneParam>          Scene : register(b1);
ConstantBuffer<ScalableAOParam>     Param : register(b2);

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
float3 ToViewPos(float2 uv, float ssR)
{
    // ssR = alpha * radiusPixels
    const float log_q = 3; // log2(2^3) = 3.
    float mipLevel = clamp(firstbithigh(ssR) - log_q, 0, Param.MaxMipLevels);

    float hardwareZ = DepthMap.SampleLevel(LinearSampler, uv, mipLevel);
    float viewZ = ToViewDepthFromReverseZ(hardwareZ, Scene.NearClip);
    float2 p = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    p *= -viewZ * float2(1.0f / Scene.Proj._11, 1.0f / Scene.Proj._22);
    return float3(p, viewZ);
}

//-----------------------------------------------------------------------------
//      AOを評価します.
//-----------------------------------------------------------------------------
float EvalAO(float3 p0, float3 p, float3 n)
{
    // Morgan McGuire, Brian Osman, Michael Bukowski, Padraic Hennessy,
    // "The Alchemy Screen-Space Ambient Obscurance Algorithm", HPG11
    // Equation(10).
    float3 v = p - p0;
    float  vv  = dot(v, v);
    float  vn  = dot(v, n);
    return max(0, (vn + Param.Bias) / (vv + 1e-6f));
}

//-----------------------------------------------------------------------------
//      エントリーポイントです.
//-----------------------------------------------------------------------------
float main(const VSOutput input) : SV_TARGET0
{
    float2 uv = input.TexCoord;

    // 中心の位置座標.
    float3 p0 = ToViewPos(uv, 0);

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

#if ENABLE_VOGEL_DISK_SAMPLE
    // Interleaved Gradient Noise.
    float3 m = float3(0.06711056f, 0.0233486, 52.9829189f);
    float  phi = frac(m.z * frac(dot(input.Position.xy, m.xy)));
#else
    int2 ssP = int2(input.Position.xy);
    float phi = (3 * ssP.x ^ ssP.y + ssP.x * ssP.y) * 10;
#endif

    // アンビエントオクルージョン.
    float occlusion = 0;

    [unroll]
    for(int i = 0; i < SAMPLE_COUNT; ++i)
    {
#if ENABLE_VOGEL_DISK_SAMPLE
        // Vogel Disk Sampling.
        float theta = 2.4f * i + phi;
        float r = sqrt((i + 0.5f) / SAMPLE_COUNT);
        float2 dir = float2(cos(theta), sin(theta)) * r;
#else
        float alpha = (i + 0.5f) / (float)SAMPLE_COUNT;
        float theta = 2.0f * PI * alpha * TAU[SAMPLE_COUNT] + phi;
        float r = alpha;
        float2 dir = float2(cos(theta), sin(theta)) * r;
#endif

        // レイマーチしたテクスチャ座標を求める.
        float2 st = uv + radiusPixels * dir * Param.InvSize;

        float ssR = r * radiusPixels;

        // テクスチャ座標から位置座標を復元.
        float3 p = ToViewPos(st, ssR);

        // AOの寄与を求める.
        occlusion += EvalAO(p0, p, n0);
    }

    occlusion = max(0.0f, 1.0f - 2.0f * Param.Sigma / SAMPLE_COUNT * occlusion);
    occlusion = pow(occlusion, Param.Intensity);

    // Bilateral Reconstruction Filter.
    const float threshold = 0.02f;
    if (abs(ddx_fine(p0.z)) < threshold)
    { occlusion -= ddx_fine(occlusion) * (frac(uv.x) - 0.5f); }
    if (abs(ddy_fine(p0.z)) < threshold)
    { occlusion -= ddy_fine(occlusion) * (frac(uv.y) - 0.5f); }

    return max(0.0f, occlusion);
};