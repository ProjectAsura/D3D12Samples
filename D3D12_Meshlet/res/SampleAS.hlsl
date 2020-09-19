//-----------------------------------------------------------------------------
// File : SampleAS.hlsl
// Desc : Sample Amplification Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"


///////////////////////////////////////////////////////////////////////////////
// MeshParam structure
///////////////////////////////////////////////////////////////////////////////
struct MeshParam
{
    float4x4    World;
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
// PayloadParam structure
///////////////////////////////////////////////////////////////////////////////
struct PayloadParam
{
    uint MeshletIndices[64];
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
groupshared PayloadParam   s_Payload;
ConstantBuffer<SceneParam>  SceneBuffer : register(b0);
ConstantBuffer<MeshParam>   MeshBuffer  : register(b1);
StructuredBuffer<CullInfo>  CullBuffer  : register(t0);

//-----------------------------------------------------------------------------
//      可視性をチェックします.
//-----------------------------------------------------------------------------
bool IsVisible(CullInfo cullData)
{
    // 視錐台カリング.
    if (IsCull(SceneBuffer.Planes, cullData.BoundingSphere))
    { return false; }

    // 縮退チェック.
    if (IsConeDegenerate(cullData.NormalCone))
    { return true; }

    // ワールド空間に変換.
    float3 center = mul(float4(cullData.BoundingSphere.xyz, 1.0f), MeshBuffer.World).xyz;

    // 視線ベクトルを求める.
    float3 viewDir = normalize(SceneBuffer.CameraPos - center);

    // 法錐カリング.
    if (IsNormalConeCull(cullData.NormalCone, viewDir))
    { return false; }

    return true;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(64, 1, 1)]
void main(uint dispatchId : SV_DispatchThreadID)
{
    bool visible = false;

    // メッシュレットカリング.
    visible = IsVisible(CullBuffer[dispatchId]);

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dispatchId;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}