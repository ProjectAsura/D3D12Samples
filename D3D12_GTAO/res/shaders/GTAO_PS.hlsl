//-----------------------------------------------------------------------------
// File : GTAO_PS.hlsl
// Desc : Grand Truth Based Ambient Occlusion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT        (16)    // サンプル数.
#endif//SAMPLE_COUNT

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

// From https://www.shadertoy.com/view/3tB3z3 - except we're using R2 here
#define XE_HILBERT_LEVEL    6U
#define XE_HILBERT_WIDTH    ( (1U << XE_HILBERT_LEVEL) )
#define XE_HILBERT_AREA     ( XE_HILBERT_WIDTH * XE_HILBERT_WIDTH )
inline uint HilbertIndex( uint posX, uint posY )
{   
    uint index = 0U;
    for( uint curLevel = XE_HILBERT_WIDTH/2U; curLevel > 0U; curLevel /= 2U )
    {
        uint regionX = ( posX & curLevel ) > 0U;
        uint regionY = ( posY & curLevel ) > 0U;
        index += curLevel * curLevel * ( (3U * regionX) ^ regionY);
        if( regionY == 0U )
        {
            if( regionX == 1U )
            {
                posX = uint( (XE_HILBERT_WIDTH - 1U) ) - posX;
                posY = uint( (XE_HILBERT_WIDTH - 1U) ) - posY;
            }

            uint temp = posX;
            posX = posY;
            posY = temp;
        }
    }
    return index;
}

// Engine-specific screen & temporal noise loader
float2 SpatioTemporalNoise( uint2 pixCoord, uint temporalIndex )    // without TAA, temporalIndex is always 0
{
    float2 noise;
    uint index = HilbertIndex( pixCoord.x, pixCoord.y );
    index += 288*(temporalIndex%64); // why 288? tried out a few and that's the best so far (with XE_HILBERT_LEVEL 6U) - but there's probably better :)
    // R2 sequence - see http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
    return float2( frac( 0.5 + index * float2(0.75487766624669276005, 0.5698402909980532659114) ) );
}


#define GTAO_DEFAULT_RADIUS_MULTIPLIER               (1.457f  )  // allows us to use different value as compared to ground truth radius to counter inherent screen space biases
#define GTAO_DEFAULT_FALLOFF_RANGE                   (0.615f  )  // distant samples contribute less
#define GTAO_DEFAULT_SAMPLE_DISTRIBUTION_POWER       (2.0f    )  // small crevices more important than big surfaces
#define GTAO_DEFAULT_THIN_OCCLUDER_COMPENSATION      (0.0f    )  // the new 'thickness heuristic' approach
#define GTAO_DEFAULT_FINAL_VALUE_POWER               (2.2f    )  // modifies the final ambient occlusion value using power function - this allows some of the above heuristics to do different things
#define GTAO_DEFAULT_DEPTH_MIP_SAMPLING_OFFSET       (3.30f   )  // main trade-off between performance (memory bandwidth) and quality (temporal stability is the first affected, thin objects next)


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

    const float effectRadius = Param.RadiusSS * GTAO_DEFAULT_RADIUS_MULTIPLIER;
    const float falloffRange = GTAO_DEFAULT_FALLOFF_RANGE * effectRadius;
    const float falloffFrom  = effectRadius * (1.0f - GTAO_DEFAULT_FALLOFF_RANGE);

    // fadeout precompute optimisation
    const float falloffMul        = -1.0 / falloffRange;
    const float falloffAdd        = falloffFrom / falloffRange + 1.0;

    // 中心の法線ベクトル.
    float3 n0 = UnpackNormal(NormalMap.SampleLevel(LinearSampler, uv, 0.0f));
    n0 = mul((float3x3)Scene.View, n0);
    n0 = normalize(n0);

    //// Interleaved Gradient Noise.
    //float3 m = float3(0.06711056f, 0.0233486, 52.9829189f);
    //float  angle = frac(m.z * frac(dot(input.Position.xy, m.xy)));
    float2 noise = SpatioTemporalNoise((uint2)input.Position.xy, 0);

    float3 viewV = normalize(-p0);
    float visibility = 0.0f;

    const int SliceCount = 4;
    const int StepCount = 4;

    const float pixelTooCloseThreshold  = 1.3f;
    const float minS = pixelTooCloseThreshold / radiusPixels;

    [unroll]
    for(float slice = 0; slice < SliceCount; ++slice)
    {
        float sliceK = (slice + noise.x) / float(SliceCount);
        float phi = sliceK * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float2 omega = float2(cosPhi, -sinPhi);
        omega *= radiusPixels;

        const float3 directionVec = float3(cosPhi, sinPhi, 0);
        const float3 orthoDirectionVec = directionVec - (dot(directionVec, viewV) * viewV);
        const float3 axisVec = normalize(cross(orthoDirectionVec, viewV));
        float3 projectedNormalVec = n0 - axisVec * dot(n0, axisVec);
        float signNorm = (float)sign(dot(orthoDirectionVec, projectedNormalVec));

        float projectedNormalVecLength = length(projectedNormalVec);
        float cosNorm = saturate(dot(projectedNormalVec, viewV) / projectedNormalVecLength);

        float n = signNorm * acos(cosNorm);

        const float lowHorizonCos0 = cos(n + HALF_PI);
        const float lowHorizonCos1 = cos(n - HALF_PI);

        float horizonCos0 = lowHorizonCos0;
        float horizonCos1 = lowHorizonCos1;

        [unroll]
        for(float step = 0; step < StepCount; ++step)
        {
            // R1 sequence (http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/)
            const float stepBaseNoise = (slice + step * StepCount) * 0.6180339887498948482;
            float stepNoise = frac(noise.y + stepBaseNoise);

            float s = (step + stepNoise) / StepCount;
            s += minS;

            float2 sampleOffset = s * omega;
            sampleOffset = round(sampleOffset) * Param.InvSize;

            float2 uv0 = uv + sampleOffset;;
            float2 uv1 = uv - sampleOffset;

            float3 pos0 = ToViewPos(uv0);
            float3 pos1 = ToViewPos(uv1);

            float3 delta0 = pos0 - p0;
            float3 delta1 = pos1 - p0;

            float dist0 = length(delta0);
            float dist1 = length(delta1);

            float3 horizonV0 = normalize(pos0 - p0);
            float3 horizonV1 = normalize(pos1 - p0);

            float weight0 = saturate(dist0 * falloffMul + falloffAdd);
            float weight1 = saturate(dist1 * falloffMul + falloffAdd);

            float shc0 = dot(horizonV0, viewV);
            float shc1 = dot(horizonV1, viewV);

            shc0 = lerp(lowHorizonCos0, shc0, weight0);
            shc1 = lerp(lowHorizonCos1, shc1, weight1);

            horizonCos0 = max(horizonCos0, shc0);
            horizonCos1 = max(horizonCos1, shc1);
        }

        float h0 = -acos(horizonCos1);
        float h1 =  acos(horizonCos0);

        h0 = n + clamp(h0 - n, -HALF_PI, HALF_PI);
        h1 = n + clamp(h1 - n, -HALF_PI, HALF_PI);

        float arc0 = (cosNorm + 2.0f * h0 * sin(n) - cos(2.0f * h0 - n)) / 4;
        float arc1 = (cosNorm + 2.0f * h1 * sin(n) - cos(2.0f * h1 - n)) / 4;
        float localVisibility = projectedNormalVecLength * (arc0 + arc1);
        visibility += localVisibility;
    }

    visibility /= (float)SliceCount;
    visibility = pow(visibility, Param.Intensity);

    return visibility;
};