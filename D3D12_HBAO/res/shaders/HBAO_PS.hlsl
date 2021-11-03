//-----------------------------------------------------------------------------
// File : HBAO_PS.hlsl
// Desc : Horizon Based Ambient Occlusion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------


#ifndef DIR_COUNT
#define DIR_COUNT   (4)    // サンプリング方向.
#endif//DIR_COUNT

#ifndef STEP_COUNT
#define STEP_COUNT  (8)    // 1方向あたりのレイマーチ数.
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
    float2  Reserved;
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
//      ハードウェアから出力された深度をビュー空間深度に変換します.
//-----------------------------------------------------------------------------
float ToViewDepth(float hardwareDepth, float nearClip, float farClip)
{ return -nearClip * farClip / (hardwareDepth * (nearClip - farClip) + farClip); }

//-----------------------------------------------------------------------------
//      ビュー空間位置を求めます.
//-----------------------------------------------------------------------------
float3 ToViewPos(float2 uv)
{
    float viewZ = ToViewDepth(DepthMap.SampleLevel(LinearSampler, uv, 0.0f), Scene.NearClip, Scene.FarClip);
    float2 p = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    p *= -viewZ * float2(1.0f / Scene.Proj._11, 1.0f / Scene.Proj._22);
    return float3(p, viewZ);
}

//-----------------------------------------------------------------------------
//      AOを評価します.
//-----------------------------------------------------------------------------
float EvalAO(float3 p0, float3 p, float3 n)
{
    float3 v = p - p0;
    float  vv  = dot(v, v);
    float  vn  = dot(v, n) * rsqrt(vv);
    return saturate(vn - Param.Bias) * saturate(vv * Param.InvRadius2 + 1.0f);
}

//-----------------------------------------------------------------------------
//      ２次元ベクトルを回転させます.
//-----------------------------------------------------------------------------
float2 Rotate(float2 dir, float2 cos_sin)
{
    return float2(
        dir.x * cos_sin.x - dir.y * cos_sin.y,
        dir.x * cos_sin.y + dir.y * cos_sin.x);
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

    int2  ssP = input.Position.xy;
        
    // ハッシュ関数により回転角を求める.
    // HPG12 "Scalable Ambinent Obscurance" (Equation.8) 参照. 
    float angle = (3 * ssP.x ^ ssP.y + ssP.x * ssP.y) * 10;

    // 回転行列.
    float sinA;
    float cosA;
    sincos(angle, sinA, cosA);

    // 適当な乱数.
    float4 rand = float4(cosA, sinA, 1 - sinA * cosA, 1);

    // レイマーチのステップサイズ.
    const float StepSize = radiusPixels / (STEP_COUNT + 1);

    // １方向あたりの回転角.
    const float Alpha = 2.0 * PI / DIR_COUNT;

    // アンビエントオクルージョン.
    float occlusion = 0;

    [unroll]
    for(int d = 0; d < DIR_COUNT; ++d)
    {
        // 回転角を求める.
        float  theta = Alpha * d;

        // 回転角をもとにレイの方向を求める.
        float2 dir = Rotate(float2(cos(theta), sin(theta)), rand.xy);

        // レイ
        float ray = (rand.z * StepSize + 1.0f);

        [unroll]
        for(int s = 0; s < STEP_COUNT; ++s)
        {
            // レイマーチしたテクスチャ座標を求める.
            float2 st  = round(ray * dir) * Param.InvSize + uv;

            // テクスチャ座標から位置座標を復元.
            float3 p = ToViewPos(st);

            // AOの寄与を求める.
            occlusion += EvalAO(p0, p, n0);

            // レイを進める.
            ray += StepSize;
        }
    }

    occlusion /= (DIR_COUNT * STEP_COUNT);
    occlusion = saturate(1.0f - occlusion);
    occlusion = pow(occlusion, Param.Intensity);

    return occlusion;
};