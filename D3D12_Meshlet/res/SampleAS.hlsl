//-----------------------------------------------------------------------------
// File : SampleAS.hlsl
// Desc : Sample Amplification Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

// �f�o�b�O�`�F�b�N�p.
#define DEBUG_CULLING   (1)


///////////////////////////////////////////////////////////////////////////////
// MeshParam structure
///////////////////////////////////////////////////////////////////////////////
struct MeshParam
{
    float4x4    World;
    float       Scale;
    float3      Padding0;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4    View;
    float4x4    Proj;
    float4      Planes[6];
    float3      CameraPos;
    float3      DebugCameraPos;
};

///////////////////////////////////////////////////////////////////////////////
// CullInfo structure
///////////////////////////////////////////////////////////////////////////////
struct CullInfo
{
    float4 BoundingSphere;
    uint   NormalCone;
};

///////////////////////////////////////////////////////////////////////////////
// MeshletInfo structure
///////////////////////////////////////////////////////////////////////////////
struct MeshletInfo
{
    uint   MeshletCount;
};

///////////////////////////////////////////////////////////////////////////////
// PayloadParam structure
///////////////////////////////////////////////////////////////////////////////
struct PayloadParam
{
    uint MeshletIndices[32];
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
groupshared PayloadParam   s_Payload;
ConstantBuffer<SceneParam>  CbScene         : register(b0);
ConstantBuffer<MeshParam>   CbMesh          : register(b1);
ConstantBuffer<MeshletInfo> CbMeshletInfo   : register(b2);
StructuredBuffer<CullInfo>  CullInfos       : register(t0);

//-----------------------------------------------------------------------------
//      �������`�F�b�N���܂�.
//-----------------------------------------------------------------------------
bool IsVisible(CullInfo cullData, float3 cameraPos, float4x4 world, float scale)
{
    // [-1, 1]�ɓW�J.
    float4 normalCone = UnpackSnorm4(cullData.NormalCone);

    // ���[���h��Ԃɕϊ�.
    float3 center = mul(world, float4(cullData.BoundingSphere.xyz, 1.0f)).xyz;
    float3 axis = normalize(mul(world, float4(normalCone.xyz, 0.0f)).xyz);

    float radius = cullData.BoundingSphere.w * scale;

    // ������J�����O.
    if (IsCull(CbScene.Planes, float4(center.xyz, radius)))
    { return false; }

    // �k�ރ`�F�b�N.
    if (IsConeDegenerate(cullData.NormalCone))
    { return true; }

    // �����x�N�g�������߂�.
    float3 viewDir = normalize(cameraPos - center);

    // �@���J�����O.
    if (IsNormalConeCull(float4(axis, normalCone.w), viewDir))
    { return false; }

    return true;
}

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
[numthreads(32, 1, 1)]
void main(uint dispatchId : SV_DispatchThreadID)
{
    bool visible = false;

    // ���b�V�����b�g�J�����O.
    if (dispatchId < CbMeshletInfo.MeshletCount)
    {
        float3 cameraPos = CbScene.CameraPos;

    #ifdef DEBUG_CULLING
        cameraPos = CbScene.DebugCameraPos;
    #endif

        visible = IsVisible(
            CullInfos[dispatchId],
            cameraPos,
            CbMesh.World,
            CbMesh.Scale);
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dispatchId;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}