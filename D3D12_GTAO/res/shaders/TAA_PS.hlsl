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
    float2 InvSize;             // �J���[�o�b�t�@�̃T�C�Y�̋t��.
    float2 Jitter;              // TAA�W�b�^�����O.
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
//      RGB����YCoCg�ɕϊ����܂�.
//-----------------------------------------------------------------------------
float3 ToYCoCg(float3 rgb)
{
    const float y  = dot(rgb, float3( 0.25f, 0.5f,  0.25f));
    const float co = dot(rgb, float3(  0.5f, 0.0f,  -0.5f));
    const float cg = dot(rgb, float3(-0.25f, 0.5f, -0.25f));
    return float3(y, co, cg);
}

//-----------------------------------------------------------------------------
//      YCoCg����RGB�ɕϊ����܂�.
//-----------------------------------------------------------------------------
float3 ToRGB(float3 ycocg)
{
    const float r = dot(ycocg, float3(1.0f,  1.0f, -1.0f));
    const float g = dot(ycocg, float3(1.0f,  0.0f,  1.0f));
    const float b = dot(ycocg, float3(1.0f, -1.0f, -1.0f));
    return float3(r, g, b);
}

//-----------------------------------------------------------------------------
//      �o�C�L���[�r�b�N�t�B���^�����O.
//-----------------------------------------------------------------------------
float4 BicubicSample(float2 uv)
{
    // �蓮�Ńt�B���^�����O���ă^�b�v�����҂�.
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
//      ���݃t���[���̃J���[���擾���܂�.
//-----------------------------------------------------------------------------
float3 GetCurrentColor(float2 uv)
{
    return ColorMap.SampleLevel(PointSampler, uv, 0.0f);
}

//-----------------------------------------------------------------------------
//      �q�X�g���[�J���[���擾���܂�.
//-----------------------------------------------------------------------------
float4 GetHistoryColor(float2 uv)
{
    return max(BicubicSample(uv), float(0.0f).xxxx);
}

//-----------------------------------------------------------------------------
//      ���x�x�N�g�����擾���܂�.
//-----------------------------------------------------------------------------
float2 GetVelocity(float2 uv)
{
    float2 result = VelocityMap.SampleLevel(PointSampler, uv, 0.0f);
    float currLengthSq = dot(result, result);

    // �ł��������x�x�N�g�����擾.
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
//      ���݃t���[���̐[�x���擾���܂�.
//-----------------------------------------------------------------------------
float GetCurrentDepth(float2 uv)
{
    const float4 depths = DepthMap.GatherRed(PointSampler, uv);
    return min( min(depths.x, depths.y), min(depths.z, depths.w) );
}

//-----------------------------------------------------------------------------
//      �O�t���[���̐[�x���擾���܂�.
//-----------------------------------------------------------------------------
float GetPreviousDepth(float2 uv)
{
    const float4 depths = DepthMap.GatherRed(PointSampler, uv);
    return max( max(depths.x, depths.y), max(depths.z, depths.w) ) + 0.001f;
}

//-----------------------------------------------------------------------------
//      �J���[��AABB�ŃN���b�v���܂�.
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
//      �q�X�g���[�J���[���N���b�v���܂�.
//-----------------------------------------------------------------------------
float3 ClipHistoryColor(float2 uv, float3 currentColor, float3 historyColor, float gamma)
{
    const float3 currColorYCoCg = ToYCoCg(currentColor);

    // ���ςƕ��U�����߂�.
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

    // ���U�N���b�s���O�������ʂ�ԋp.
    return ToRGB( ClipToAABB( ToYCoCg(historyColor), currColorYCoCg, ave, var) );
}

//-----------------------------------------------------------------------------
//      ��ԌW�������߂܂�.
//-----------------------------------------------------------------------------
float CalcLerpFactor(float weight)
{
    return saturate( pow( weight * LerpMul, LerpPow ) );
}

//-----------------------------------------------------------------------------
//      ���݃J���[�ƃq�X�g���[����̃u�����h���ʂ����߂܂�.
//-----------------------------------------------------------------------------
float4 GetFinalColor(float2 uv, float3 currentColor, float3 historyColor, float weight)
{
    float newWeight = saturate( 0.5f - weight );
    return float4( lerp(currentColor, historyColor, CalcLerpFactor(weight)), newWeight );
}

//-----------------------------------------------------------------------------
//      �אڃs�N�Z�����l���������݃J���[�����߂܂�.
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
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    float2 currUV = input.TexCoord;

    // ���݃t���[���̐F.
    float3 currentColor = GetCurrentColor(currUV);

    // ���x�x�N�g�����擾.
    float2 velocity = GetVelocity(currUV);
    float  velocityDelta = saturate(1.0f - length(velocity) / kFrameVelocityInPixelsDiff);

    // ���ݐ[�x���擾.
    float depth = GetCurrentDepth(currUV);
    float depthDelta = GetDepthConfidenceFactor(currUV, velocity, depth);

    // �O�t���[���̃e�N�X�`�����W���v�Z.
    float2 prevUV = currUV + velocity.xy * InvSize;

    // �e�N�X�`�����W�̏d�݂����߂�(��ʔ͈͊O�Ȃ�[��).
    const float uvWeight = ( all(prevUV >= float(0.0).xx) && all(prevUV < float(1.0f).xx) ) ? 1.0f : 0.0f;

    // �q�X�g���[���L�����ǂ���? (���x�̍����E�[�x�̍����E��ʔ͈͂̏��Ȃ��Ƃ��̈���[���Ȃ疳���Ɣ��f�j
    const bool isValidHistory = (velocityDelta * depthDelta * uvWeight) > 0.0f;

    // �ŏI�J���[.
    float4 finalColor;

    // �q�X�g���[�o�b�t�@���L���ȏꍇ.
    if (isValidHistory)
    {
        // �q�X�g���[�J���[���擾.
        float4 rawHistoryColor = GetHistoryColor(prevUV);

        // ���U��.
        float varianceGamma = lerp(VarClipGammaMin, VarClipGammaMax, velocityDelta * velocityDelta);

        // �q�X�g���[�J���[���N���b�v����.
        float3 historyColor = ClipHistoryColor(prevUV, currentColor, rawHistoryColor.rgb, varianceGamma);

        // �d�݂����߂�.
        float weight = rawHistoryColor.a * velocityDelta * depthDelta;

        // ���݃J���[�ƃq�X�g���[�J���[���u�����h���čŏI�J���[�����߂�.
        finalColor = GetFinalColor(currUV, currentColor, historyColor, weight);
    }
    else
    {
        // �אڃs�N�Z�����l�����čŏI�J���[�����߂�.
        const float3 neighborColor = GetCurrentNeighborColor(currUV, currentColor);
        finalColor = float4(neighborColor, 0.5f);
    }

    PSOutput output;
    output.Color   = finalColor;
    output.History = finalColor;

    // ���ʂ��o��.
    return output;
}