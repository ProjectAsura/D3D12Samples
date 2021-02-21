//-----------------------------------------------------------------------------
// File : SampleRayTracing.hlsl
// Desc : Sample Ray Tracer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

#define GLOBAL space0
#define LOCAL  space1

#define VERTEX_STRIDE   (9 * 4)     // 1���_������̃T�C�Y.
#define INDEX_STRIDE    (3 * 4)     // 1�|���S��������̃C���f�b�N�X�T�C�Y.
#define TEXCOORD_OFFSET (3 * 4)     // 1���_�f�[�^�̐擪����e�N�X�`�����W�f�[�^�܂ł̃I�t�Z�b�g.
#define COLOR_OFFSET    (2 * 4)     // 1���_�f�[�^�̐擪����J���[�f�[�^�܂ł̃I�t�Z�b�g.

//-----------------------------------------------------------------------------
// Type Definitions
//-----------------------------------------------------------------------------
typedef BuiltInTriangleIntersectionAttributes HitArgs;


///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4    InvView;
    float4x4    InvViewProj;
};

///////////////////////////////////////////////////////////////////////////////
// RayPayload structure
///////////////////////////////////////////////////////////////////////////////
struct RayPayload
{
    float4 Color;
    float3 RayDir;
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    float3 Position;    // �ʒu���W.
    float2 TexCoord;    // �e�N�X�`�����W.
    float4 Color;       // ���_�J���[.
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
RWTexture2D<float4>             RenderTarget  : register(u0);
RaytracingAccelerationStructure Scene         : register(t0);
Texture2D<float4>               BackGround    : register(t1);
ByteAddressBuffer               Vertices      : register(t2);
ByteAddressBuffer               Indices       : register(t3);
ConstantBuffer<SceneParam>      SceneParam    : register(b0);
SamplerState                    LinearWrap    : register(s0);

Texture2D BaseColor : register(t4);


//-----------------------------------------------------------------------------
//      �v���~�e�B�u�ԍ����擾���܂�.
//-----------------------------------------------------------------------------
uint3 GetIndices(uint triangleIndex)
{
    uint address = triangleIndex * INDEX_STRIDE;
    return Indices.Load3(address);
}

//-----------------------------------------------------------------------------
//      �d�S���W�ŕ�Ԃ������_�f�[�^���擾���܂�.
//-----------------------------------------------------------------------------
Vertex GetVertex(uint triangleIndex, float3 barycentrices)
{
    uint3 indices = GetIndices(triangleIndex);
    Vertex v = (Vertex)0;

    [unroll]
    for(uint i=0; i<3; ++i)
    {
        uint address = indices[i] * VERTEX_STRIDE;
        v.Position += asfloat(Vertices.Load3(address)) * barycentrices[i];

        address += TEXCOORD_OFFSET;
        v.TexCoord += asfloat(Vertices.Load2(address)) * barycentrices[i];

        address += COLOR_OFFSET;
        v.Color += asfloat(Vertices.Load4(address)) * barycentrices[i];
    }

    return v;
}

//-----------------------------------------------------------------------------
//      ���C�����߂܂�.
//-----------------------------------------------------------------------------
void CalcRay(float2 index, out float3 pos, out float3 dir)
{
    float4 orig   = float4(0.0f, 0.0f, 0.0f, 1.0f);           // �J�����̈ʒu.
    float4 screen = float4(-2.0f * index + 1.0f, 0.0f, 1.0f); // �X�N���[���̈ʒu.

    orig   = mul(SceneParam.InvView,     orig);
    screen = mul(SceneParam.InvViewProj, screen);

    // w = 1 �Ɏˉe.
    screen.xyz /= screen.w;

    // ���C�̈ʒu�ƕ�����ݒ�.
    pos = orig.xyz;
    dir = normalize(screen.xyz - orig.xyz);
}

//-----------------------------------------------------------------------------
//      ���C�𐶐����܂�.
//-----------------------------------------------------------------------------
[shader("raygeneration")]
void OnGenerateRay()
{
    float2 index = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

    // ���C�𐶐�.
    float3 rayOrig;
    float3 rayDir;
    CalcRay(index, rayOrig, rayDir);

    // �y�C���[�h������.
    RayPayload payload;
    payload.RayDir = rayDir;
    payload.Color  = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // ���C��ݒ�.
    RayDesc ray;
    ray.Origin    = rayOrig;
    ray.Direction = rayDir;
    ray.TMin      = 1e-3f;
    ray.TMax      = 10000.0f;

    // ���C��ǐ�.
    TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    // �����_�[�^�[�Q�b�g�Ɋi�[.
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

//-----------------------------------------------------------------------------
//      ������̏����ł�.
//-----------------------------------------------------------------------------
[shader("closesthit")]
void OnClosestHit1(inout RayPayload payload, in HitArgs args)
{
    // �d�S���W�����߂�.
    float3 barycentrics = float3(
        1.0f - args.barycentrics.x - args.barycentrics.y,
        args.barycentrics.x,
        args.barycentrics.y);

    // �v���~�e�B�u�ԍ��擾.
    uint triangleIndex = PrimitiveIndex();

    // ���_�f�[�^�擾.
    Vertex v = GetVertex(triangleIndex, barycentrics);

    // ���_�J���[��Z���āC�F�����߂�.
    payload.Color = BaseColor.SampleLevel(LinearWrap, v.TexCoord, 0.0f) * v.Color;
}

//-----------------------------------------------------------------------------
//      ������̏����ł�.
//-----------------------------------------------------------------------------
[shader("closesthit")]
void OnClosestHit2(inout RayPayload payload, in HitArgs args)
{
    // �d�S���W�����߂�.
    float3 barycentrics = float3(
        1.0f - args.barycentrics.x - args.barycentrics.y,
        args.barycentrics.x,
        args.barycentrics.y);

    // �v���~�e�B�u�ԍ��擾.
    uint triangleIndex = PrimitiveIndex();

    // ���_�f�[�^�擾.
    Vertex v = GetVertex(triangleIndex, barycentrics);

    // �e�N�X�`���J���[�̂�.
    payload.Color = BaseColor.SampleLevel(LinearWrap, v.TexCoord, 0.0f);
}

//-----------------------------------------------------------------------------
//      �������̏����ł�.
//-----------------------------------------------------------------------------
[shader("miss")]
void OnMiss(inout RayPayload payload)
{
    // �X�t�B�A�}�b�v�̃e�N�X�`�����W���Z�o.
    float2 uv = ToSphereMapCoord(payload.RayDir);

    // �X�t�B�A�}�b�v���T���v��.
    float4 color = BackGround.SampleLevel(LinearWrap, uv, 0.0f);

    // �F��ݒ�.
    payload.Color = color;
}
