//-----------------------------------------------------------------------------
// File : TemporalAA_PS.hlsl
// Desc : Temporal Anti-Aliasing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float2 kOffsets[8] = {
    float2(-1.0f, -1.0f),
    float2(-1.0f,  0.0f),
    float2(-1.0f,  1.0f),
    float2( 0.0f,  1.0f),
    float2( 1.0f,  1.0f),
    float2( 1.0f,  0.0f),
    float2( 1.0f, -1.0f),
    float2( 0.0f, -1.0f)
};
static const float kVarianceIntersectionMaxT  = 100.0f;
static const float kFrameVelocityInPixelsDiff = 256.0f; // 1920 x 1080.


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4 Color    : SV_TARGET0;
    float4 History  : SV_TARGET1;
};

///////////////////////////////////////////////////////////////////////////////
// CbTemporalAA constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTemporalAA : register(b0)
{
    float2 InvSize;             // カラーバッファのサイズの逆数.
    float2 Jitter;              // TAAジッタリング.
    float  LerpMul;             //
    float  LerpPow;             //
    float  VarClipGammaMin;     //
    float  VarClipGammaMax;     //
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D<float3>   ColorMap        : register(t0);
Texture2D<float4>   HistoryMap      : register(t1);
Texture2D<float2>   VelocityMap     : register(t2);
Texture2D<float>    DepthMap        : register(t3);
SamplerState        PointSampler    : register(s0);
SamplerState        LinearSampler   : register(s1);

//-----------------------------------------------------------------------------
//      RGBからYCoCgに変換します.
//-----------------------------------------------------------------------------
float3 ToYCoCg(float3 rgb)
{
    const float y  = dot(rgb, float3( 0.25f, 0.5f,  0.25f));
    const float co = dot(rgb, float3(  0.5f, 0.0f,  -0.5f));
    const float cg = dot(rgb, float3(-0.25f, 0.5f, -0.25f));
    return float3(y, co, cg);
}

//-----------------------------------------------------------------------------
//      YCoCgからRGBに変換します.
//-----------------------------------------------------------------------------
float3 ToRGB(float3 ycocg)
{
    const float r = dot(ycocg, float3(1.0f,  1.0f, -1.0f));
    const float g = dot(ycocg, float3(1.0f,  0.0f,  1.0f));
    const float b = dot(ycocg, float3(1.0f, -1.0f, -1.0f));
    return float3(r, g, b);
}

//-----------------------------------------------------------------------------
//      バイキュービックフィルタリング.
//-----------------------------------------------------------------------------
float4 BicubicSample(float2 uv)
{
    // 手動でフィルタリングしてタップ数を稼ぐ.
    const float2 fractional = frac(uv);
    const float2 st = (floor(uv) + float(0.5f).xx) * InvSize;

    const float2 t1 = fractional;
    const float2 t2 = fractional * fractional;
    const float2 t3 = fractional * t2;

    const float s = 0.5f;
    const float2 w0 = -s * t3 + 2.0f * s * t2 - s * t1;
    const float2 w1 = (2.0f - s) * t3 + (s - 3.0f) * t2 + 1.0f;
    const float2 w2 = (s - 2.0f) * t3 + (3.0f - 2.0f * s) * t2 + s * t1;
    const float2 w3 = s * t3 - s * t2;

    const float2 s0 = w1 + w2;
    const float2 f0 = w2 / (w1 + w2);
    const float2 m0 = uv + f0 * InvSize;

    const float2 tc0 = uv - 1.0f * InvSize;
    const float2 tc3 = uv + 2.0f * InvSize;

    const float4 A = HistoryMap.SampleLevel(LinearSampler, float2( m0.x, tc0.y), 0.0f);
    const float4 B = HistoryMap.SampleLevel(LinearSampler, float2(tc0.x,  m0.y), 0.0f);
    const float4 C = HistoryMap.SampleLevel(LinearSampler, float2( m0.x,  m0.y), 0.0f);
    const float4 D = HistoryMap.SampleLevel(LinearSampler, float2(tc3.x,  m0.y), 0.0f);
    const float4 E = HistoryMap.SampleLevel(LinearSampler, float2( m0.x, tc3.y), 0.0f);

    return (0.5f * (A + B) * w0.x + A * s0.x + 0.5f * (A + B) * w3.x) * w0.y
         + (0.5f * (B + E) * w0.x + E * s0.x + 0.5f * (D + E) * w3.x) * w3.y
         + (B * w0.x + C * s0.x + D * w3.x) * s0.y;
}

//-----------------------------------------------------------------------------
//      現在フレームのカラーを取得します.
//-----------------------------------------------------------------------------
float3 GetCurrentColor(float2 uv)
{
    return ColorMap.SampleLevel(PointSampler, uv, 0.0f);
}

//-----------------------------------------------------------------------------
//      ヒストリーカラーを取得します.
//-----------------------------------------------------------------------------
float4 GetHistoryColor(float2 uv)
{
    return max(BicubicSample(uv), float(0.0f).xxxx);
}

//-----------------------------------------------------------------------------
//      速度ベクトルを取得します.
//-----------------------------------------------------------------------------
float2 GetVelocity(float2 uv)
{
    float2 result = VelocityMap.SampleLevel(PointSampler, uv, 0.0f);
    float currLengthSq = dot(result, result);

    // 最も長い速度ベクトルを取得.
    [unroll] for(uint i=0; i<8; ++i)
    {
        const float2 velocity = VelocityMap.SampleLevel(PointSampler, uv + kOffsets[i] * InvSize, 0.0f);
        const float  lengthSq = dot(velocity, velocity);
        if (lengthSq > currLengthSq)
        {
            result = velocity;
            currLengthSq = lengthSq;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      現在フレームの深度を取得します.
//-----------------------------------------------------------------------------
float GetCurrentDepth(float2 uv)
{
    const float4 depths = DepthMap.GatherRed(PointSampler, uv);
    return min( min(depths.x, depths.y), min(depths.z, depths.w) );
}

//-----------------------------------------------------------------------------
//      前フレームの深度を取得します.
//-----------------------------------------------------------------------------
float GetPreviousDepth(float2 uv)
{
    const float4 depths = DepthMap.GatherRed(PointSampler, uv);
    return max( max(depths.x, depths.y), max(depths.z, depths.w) ) + 0.001f;
}

//-----------------------------------------------------------------------------
//      カラーをAABBでクリップします.
//-----------------------------------------------------------------------------
float3 ClipToAABB(float3 currentColor, float3 historyColor, float3 center, float3 extents)
{
    const float3 dir = currentColor - historyColor;
    const float3 intersection = ((center - sign(dir) * extents) - historyColor) / dir;
    const float3 possibleT = (intersection >= float(0.0).xxx) ? intersection : kVarianceIntersectionMaxT + 1.0f;
    const float t = min(kVarianceIntersectionMaxT, min(possibleT, min(possibleT.y, possibleT.z)));
    return (t < kVarianceIntersectionMaxT) ? historyColor + dir * t : historyColor;
}

//-----------------------------------------------------------------------------
//      ヒストリーカラーをクリップします.
//-----------------------------------------------------------------------------
float3 ClipHistoryColor(float2 uv, float3 currentColor, float3 historyColor, float gamma)
{
    const float3 currColorYCoCg = ToYCoCg(currentColor);

    // 平均と分散を求める.
    float3 ave = currColorYCoCg;
    float3 var = currColorYCoCg * currColorYCoCg;

    [unroll]
    for(uint i=0; i<8; ++i)
    {
        const float2 st = uv + kOffsets[i] * InvSize;
        const float3 newColor = ToYCoCg( GetCurrentColor(st) );
        ave += newColor;
        var += newColor * newColor;
    }

    const float invSamples = 1.0f / 9.0f;
    ave *= invSamples;
    var = sqrt( max(float3(0.0000003, 0.00001, 0.00001), var * invSamples - ave * ave )) * gamma;

    // 分散クリッピングした結果を返却.
    return ToRGB( ClipToAABB( ToYCoCg(historyColor), currColorYCoCg, ave, var) );
}

//-----------------------------------------------------------------------------
//      補間係数を求めます.
//-----------------------------------------------------------------------------
float CalcLerpFactor(float weight)
{
    return saturate( pow( weight * LerpMul, LerpPow ) );
}

//-----------------------------------------------------------------------------
//      現在カラーとヒストリーからのブレンド結果を求めます.
//-----------------------------------------------------------------------------
float4 GetFinalColor(float2 uv, float3 currentColor, float3 historyColor, float weight)
{
    float newWeight = saturate( 0.5f - weight );
    return float4( lerp(currentColor, historyColor, CalcLerpFactor(weight)), newWeight );
}

//-----------------------------------------------------------------------------
//      隣接ピクセルを考慮した現在カラーを求めます.
//-----------------------------------------------------------------------------
float3 GetCurrentNeighborColor(float2 uv, float3 currentColor)
{
    const float2 offsets[4] = {
        float2(-1.0f, -1.0f),
        float2(-1.0f,  1.0f),
        float2( 1.0f,  1.0f),
        float2( 1.0f, -1.0f)
    };

    const float centerWeight = 4.0f;
    float3 accColor = currentColor * centerWeight;
    [unroll]
    for(uint i=0; i<4; ++i)
    {
        const float2 st = uv + offsets[i] * InvSize;
        accColor += GetCurrentColor(st);
    }
    const float invWeight = 1.0f / (4.0f + centerWeight);
    accColor *= invWeight;
    return accColor;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float GetDepthConfidenceFactor(float2 uv, float2 velocity, float currentDepth)
{
    const float prevDepth = GetPreviousDepth( uv + (velocity + Jitter) * InvSize );
    const float currDepth = currentDepth;// + velocity.z;
    return step(currDepth, prevDepth);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    float2 currUV = input.TexCoord;

    // 現在フレームの色.
    float3 currentColor = GetCurrentColor(currUV);

    // 速度ベクトルを取得.
    float2 velocity = GetVelocity(currUV);
    float  velocityDelta = saturate(1.0f - length(velocity) / kFrameVelocityInPixelsDiff);

    // 現在深度を取得.
    float depth = GetCurrentDepth(currUV);
    float depthDelta = GetDepthConfidenceFactor(currUV, velocity, depth);

    // 前フレームのテクスチャ座標を計算.
    float2 prevUV = currUV + velocity.xy * InvSize;

    // テクスチャ座標の重みを求める(画面範囲外ならゼロ).
    const float uvWeight = ( all(prevUV >= float(0.0).xx) && all(prevUV < float(1.0f).xx) ) ? 1.0f : 0.0f;

    // ヒストリーが有効かどうか? (速度の差分・深度の差分・画面範囲の少なくともの一つがゼロなら無効と判断）
    const bool isValidHistory = (velocityDelta * depthDelta * uvWeight) > 0.0f;

    // 最終カラー.
    float4 finalColor;

    // ヒストリーバッファが有効な場合.
    if (isValidHistory)
    {
        // ヒストリーカラーを取得.
        float4 rawHistoryColor = GetHistoryColor(prevUV);

        // 分散γ.
        float varianceGamma = lerp(VarClipGammaMin, VarClipGammaMax, velocityDelta * velocityDelta);

        // ヒストリーカラーをクリップする.
        float3 historyColor = ClipHistoryColor(prevUV, currentColor, rawHistoryColor.rgb, varianceGamma);

        // 重みを求める.
        float weight = rawHistoryColor.a * velocityDelta * depthDelta;

        // 現在カラーとヒストリーカラーをブレンドして最終カラーを求める.
        finalColor = GetFinalColor(currUV, currentColor, historyColor, weight);
    }
    else
    {
        // 隣接ピクセルを考慮して最終カラーを求める.
        const float3 neighborColor = GetCurrentNeighborColor(currUV, currentColor);
        finalColor = float4(neighborColor, 0.5f);
    }

    PSOutput output;
    output.Color   = finalColor;
    output.History = finalColor;

    // 結果を出力.
    return output;
}