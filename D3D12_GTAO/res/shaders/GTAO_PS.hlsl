//-----------------------------------------------------------------------------
// File : GTAO_PS.hlsl
// Desc : Grand Truth Based Ambient Occlusion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT        (16)    // �T���v����.
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
    float2  InvSize;    // �����_�[�^�[�Q�b�g�T�C�Y�̋t��.
    float   RadiusSS;   // �X�N���[����Ԃ̔��a.
    float   InvRadius2; // -1.0 / (Radius * Radius).
    float   Bias;       // �o�C�A�X.
    float   Intensity;  // ���x.
    float   Sigma;      // �V�O�}.
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
//      �@���x�N�g�����A���p�b�L���O���܂�.
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
//      �n�[�h�E�F�A����o�͂��ꂽReverse-Z���r���[��Ԑ[�x�ɕϊ����܂�.
//-----------------------------------------------------------------------------
float ToViewDepthFromReverseZ(float hardwareDepth, float nearClip)
{
    return -nearClip / hardwareDepth;
}

//-----------------------------------------------------------------------------
//      �r���[��Ԉʒu�����߂܂�.
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
//      AO��]�����܂�.
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
//      �G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float main(const VSOutput input) : SV_TARGET0
{
    float2 uv = input.TexCoord;

    // ���S�̈ʒu���W.
    float3 p0 = ToViewPos(uv);

    // �s�N�Z�����a.
    float radiusPixels = abs(Param.RadiusSS / p0.z);

    // 1 �s�N�Z�������̏ꍇ�͏������Ȃ�.
    if (radiusPixels < 1.0f)
    {
        return 1.0f;
    }

    // ���S�̖@���x�N�g��.
    float3 n0 = UnpackNormal(NormalMap.SampleLevel(LinearSampler, uv, 0.0f));
    n0 = mul((float3x3)Scene.View, n0);
    n0 = normalize(n0);

    // Interleaved Gradient Noise.
    float3 m = float3(0.06711056f, 0.0233486, 52.9829189f);
    float  phi = frac(m.z * frac(dot(input.Position.xy, m.xy)));

    // �A���r�G���g�I�N���[�W����.
    float occlusion = 0;

    [unroll]
    for(int i = 0; i < SAMPLE_COUNT; ++i)
    {
        // Vogel Disk Sampling.
        float theta = 2.4f * i + phi;
        float r = sqrt((i + 0.5f) / SAMPLE_COUNT);
        float2 dir = float2(cos(theta), sin(theta)) * r;

        // ���C�}�[�`�����e�N�X�`�����W�����߂�.
        float2 st = uv + radiusPixels * dir * Param.InvSize;

        // �e�N�X�`�����W����ʒu���W�𕜌�.
        float3 p = ToViewPos(st);

        // AO�̊�^�����߂�.
        occlusion += EvalAO(p0, p, n0);
    }

    occlusion = max(0.0f, 1.0f - 2.0f * Param.Sigma / SAMPLE_COUNT * occlusion);
    occlusion = pow(occlusion, Param.Intensity);

    return occlusion;
};