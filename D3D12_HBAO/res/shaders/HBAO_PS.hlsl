//-----------------------------------------------------------------------------
// File : HBAO_PS.hlsl
// Desc : Horizon Based Ambient Occlusion.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------


#ifndef DIR_COUNT
#define DIR_COUNT   (4)    // �T���v�����O����.
#endif//DIR_COUNT

#ifndef STEP_COUNT
#define STEP_COUNT  (8)    // 1����������̃��C�}�[�`��.
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
    float2  InvSize;    // �����_�[�^�[�Q�b�g�T�C�Y�̋t��.
    float   RadiusSS;   // �X�N���[����Ԃ̔��a.
    float   InvRadius2; // -1.0 / (Radius * Radius).
    float   Bias;       // �o�C�A�X.
    float   Intensity;  // ���x.
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
//      �n�[�h�E�F�A����o�͂��ꂽ�[�x���r���[��Ԑ[�x�ɕϊ����܂�.
//-----------------------------------------------------------------------------
float ToViewDepth(float hardwareDepth, float nearClip, float farClip)
{ return -nearClip * farClip / (hardwareDepth * (nearClip - farClip) + farClip); }

//-----------------------------------------------------------------------------
//      �r���[��Ԉʒu�����߂܂�.
//-----------------------------------------------------------------------------
float3 ToViewPos(float2 uv)
{
    float viewZ = ToViewDepth(DepthMap.SampleLevel(LinearSampler, uv, 0.0f), Scene.NearClip, Scene.FarClip);
    float2 p = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    p *= -viewZ * float2(1.0f / Scene.Proj._11, 1.0f / Scene.Proj._22);
    return float3(p, viewZ);
}

//-----------------------------------------------------------------------------
//      AO��]�����܂�.
//-----------------------------------------------------------------------------
float EvalAO(float3 p0, float3 p, float3 n)
{
    float3 v = p - p0;
    float  vv  = dot(v, v);
    float  vn  = dot(v, n) * rsqrt(vv);
    return saturate(vn - Param.Bias) * saturate(vv * Param.InvRadius2 + 1.0f);
}

//-----------------------------------------------------------------------------
//      �Q�����x�N�g������]�����܂�.
//-----------------------------------------------------------------------------
float2 Rotate(float2 dir, float2 cos_sin)
{
    return float2(
        dir.x * cos_sin.x - dir.y * cos_sin.y,
        dir.x * cos_sin.y + dir.y * cos_sin.x);
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

    int2  ssP = input.Position.xy;
        
    // �n�b�V���֐��ɂ���]�p�����߂�.
    // HPG12 "Scalable Ambinent Obscurance" (Equation.8) �Q��. 
    float angle = (3 * ssP.x ^ ssP.y + ssP.x * ssP.y) * 10;

    // ��]�s��.
    float sinA;
    float cosA;
    sincos(angle, sinA, cosA);

    // �K���ȗ���.
    float4 rand = float4(cosA, sinA, 1 - sinA * cosA, 1);

    // ���C�}�[�`�̃X�e�b�v�T�C�Y.
    const float StepSize = radiusPixels / (STEP_COUNT + 1);

    // �P����������̉�]�p.
    const float Alpha = 2.0 * PI / DIR_COUNT;

    // �A���r�G���g�I�N���[�W����.
    float occlusion = 0;

    [unroll]
    for(int d = 0; d < DIR_COUNT; ++d)
    {
        // ��]�p�����߂�.
        float  theta = Alpha * d;

        // ��]�p�����ƂɃ��C�̕��������߂�.
        float2 dir = Rotate(float2(cos(theta), sin(theta)), rand.xy);

        // ���C
        float ray = (rand.z * StepSize + 1.0f);

        [unroll]
        for(int s = 0; s < STEP_COUNT; ++s)
        {
            // ���C�}�[�`�����e�N�X�`�����W�����߂�.
            float2 st  = round(ray * dir) * Param.InvSize + uv;

            // �e�N�X�`�����W����ʒu���W�𕜌�.
            float3 p = ToViewPos(st);

            // AO�̊�^�����߂�.
            occlusion += EvalAO(p0, p, n0);

            // ���C��i�߂�.
            ray += StepSize;
        }
    }

    occlusion /= (DIR_COUNT * STEP_COUNT);
    occlusion = saturate(1.0f - occlusion);
    occlusion = pow(occlusion, Param.Intensity);

    return occlusion;
};