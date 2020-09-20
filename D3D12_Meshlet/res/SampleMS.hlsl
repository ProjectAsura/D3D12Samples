//-----------------------------------------------------------------------------
// File : SampleMS.hlsl
// Desc : Sample Mesh Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"


///////////////////////////////////////////////////////////////////////////////
// MSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct MSOutput
{
    float4  Position    : SV_POSITION;
    float2  TexCoord    : TEXCOORD;
    float3  Normal      : NORMAL;
    float3  Tangent     : TANGENT;
};

///////////////////////////////////////////////////////////////////////////////
// Meshlet structure
///////////////////////////////////////////////////////////////////////////////
struct Meshlet
{
    uint VertexOffset;      // 頂点番号オフセット.
    uint VertexCount;       // 出力頂点数.
    uint PrimitiveOffset;   // プリミティブ番号オフセット.
    uint PrimitiveCount;    // 出力プリミティブ数.
};

///////////////////////////////////////////////////////////////////////////////
// MeshParam structure
///////////////////////////////////////////////////////////////////////////////
struct MeshParam
{
    float4x4 World;
    float    Scale;
    float3   Padding0;
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
    float       Padding0;
    float3      DebugCameraPos;
    float       Padding1;
    float4      DebugPlanes[6];
};

///////////////////////////////////////////////////////////////////////////////
// PayloadParam structure
///////////////////////////////////////////////////////////////////////////////
struct PayloadParam
{
    uint MeshletIndices[32];
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
StructuredBuffer<float3>    Positions       : register(t0);
StructuredBuffer<uint>      TangentSpaces   : register(t1);
StructuredBuffer<uint>      TexCoords       : register(t2);
StructuredBuffer<uint>      Indices         : register(t3);
StructuredBuffer<uint>      Primitives      : register(t4);
StructuredBuffer<Meshlet>   Meshlets        : register(t5);
ConstantBuffer<SceneParam>  CbScene         : register(b0);
ConstantBuffer<MeshParam>   CbMesh          : register(b1);

//-----------------------------------------------------------------------------
//      メインエントリーポイント.
//-----------------------------------------------------------------------------
[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main
(
    uint groupThreadId  : SV_GroupThreadID,
    uint groupId        : SV_GroupId,
    in   payload  PayloadParam payloadParam,
    out  vertices MSOutput verts[64],
    out  indices  uint3    polys[126]
)
{
    uint meshletIndex = payloadParam.MeshletIndices[groupId];
    Meshlet m = Meshlets[meshletIndex];

    SetMeshOutputCounts(m.VertexCount, m.PrimitiveCount);

    if (groupThreadId < m.PrimitiveCount)
    {
        uint packedIndex = Primitives[m.PrimitiveOffset + groupThreadId];
        polys[groupThreadId] = UnpackPrimitiveIndex(packedIndex);
    }
    
    if (groupThreadId < m.VertexCount)
    {
        uint index = Indices[m.VertexOffset + groupThreadId];
        float4 localPos = float4(Positions[index], 1.0f);
        float4 worldPos = mul(CbMesh.World, localPos);
        float4 viewPos  = mul(CbScene.View, worldPos);
        float4 projPos  = mul(CbScene.Proj, viewPos);

        float3 T, N;
        uint encodedTBN = TangentSpaces[index];
        UnpackTN(encodedTBN, T, N);

        float3 worldT = normalize(mul((float3x3)CbMesh.World, T));
        float3 worldN = normalize(mul((float3x3)CbMesh.World, N));

        MSOutput output = (MSOutput)0;
        output.Position     = projPos;
        output.TexCoord     = UnpackHalf2(TexCoords[index]);
        output.Normal       = worldN;
        output.Tangent      = worldT;

        verts[groupThreadId] = output;
    }
}
