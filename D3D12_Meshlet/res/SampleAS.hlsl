//-----------------------------------------------------------------------------
// File : SampleAS.hlsl
// Desc : Sample Amplification Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

// デバッグチェック用.
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
//      可視性をチェックします.
//-----------------------------------------------------------------------------
bool IsVisible(CullInfo cullData, float3 cameraPos, float4x4 world, float scale)
{
    // [-1, 1]に展開.
    float4 normalCone = UnpackSnorm4(cullData.NormalCone);

    // ワールド空間に変換.
    float3 center = mul(world, float4(cullData.BoundingSphere.xyz, 1.0f)).xyz;
    float3 axis = normalize(mul(world, float4(normalCone.xyz, 0.0f)).xyz);

    float radius = cullData.BoundingSphere.w * scale;

    // 視錐台カリング.
    if (IsCull(CbScene.Planes, float4(center.xyz, radius)))
    { return false; }

    // 縮退チェック.
    if (IsConeDegenerate(cullData.NormalCone))
    { return true; }

    // 視線ベクトルを求める.
    float3 viewDir = normalize(cameraPos - center);

    // 法錐カリング.
    if (IsNormalConeCull(float4(axis, normalCone.w), viewDir))
    { return false; }

    return true;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(32, 1, 1)]
void main(uint dispatchId : SV_DispatchThreadID)
{
    bool visible = false;

    // メッシュレットカリング.
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