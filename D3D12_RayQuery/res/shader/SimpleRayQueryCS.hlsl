//-----------------------------------------------------------------------------
// File : SimpleRayQueryCS.hlsl
// Desc : Ray Query Sample.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"
#include "RayQuery.hlsli"


///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4    View;
    float4x4    Proj;
    float4x4    InvView;
    float4x4    InvViewProj;
    uint2       TargetSize;
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    float3 Position;
    float4 Color;
};

ConstantBuffer<SceneParam>  SceneBuffer   : register(b0);
Tlas                        SceneTlas     : register(t0);
Texture2D<float4>           Background    : register(t1);
ByteAddressBuffer           VertexBuffer  : register(t2);
ByteAddressBuffer           IndexBuffer   : register(t3);
RWTexture2D<float4>         Canvas        : register(u0);
SamplerState                LinearSampler : register(s0);

//-----------------------------------------------------------------------------
//      プリミティブ番号を取得します.
//-----------------------------------------------------------------------------
uint3 GetIndices(uint triangleIndex)
{
    uint address = triangleIndex * sizeof(uint3);
    return IndexBuffer.Load3(address);
}

//-----------------------------------------------------------------------------
//      重心座標で補間した頂点データを取得します.
//-----------------------------------------------------------------------------
Vertex GetVertex(uint triangleIndex, float3 barycentrices)
{
    uint3 indices = GetIndices(triangleIndex);
    Vertex v = (Vertex)0;

    [unroll]
    for(uint i=0; i<3; ++i)
    {
        uint address = indices[i] * sizeof(Vertex);
        v.Position += asfloat(VertexBuffer.Load3(address)) * barycentrices[i];

        address += sizeof(float3);
        v.Color += asfloat(VertexBuffer.Load4(address)) * barycentrices[i];
    }

    return v;
}

//-----------------------------------------------------------------------------
//      レイを求めます.
//-----------------------------------------------------------------------------
void CalcRay(float2 index, out float3 pos, out float3 dir)
{
    float4 orig   = float4(0.0f, 0.0f, 0.0f, 1.0f);           // カメラの位置.
    float4 screen = float4(-2.0f * index + 1.0f, 0.0f, 1.0f); // スクリーンの位置.

    orig   = mul(SceneBuffer.InvView,     orig);
    screen = mul(SceneBuffer.InvViewProj, screen);

    // w = 1 に射影.
    screen.xyz /= screen.w;

    // レイの位置と方向を設定.
    pos = orig.xyz;
    dir = normalize(screen.xyz - orig.xyz);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main
(
    uint3 dispatchId : SV_DispatchThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    uint2 remappedId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(remappedId >= SceneBuffer.TargetSize)) 
    { return; }

    float2 index = (float2)remappedId / (float2)SceneBuffer.TargetSize;

    float3 rayOrigin;
    float3 rayDirection;
    HitRecord hit = Intersect(
        SceneTlas,
        RAY_FLAG_NONE,
        ~0,
        rayOrigin,
        rayDirection,
        1e-3f,
        10000.0f);

    float4 color;
    if (hit.IsHit())
    {
        Vertex v = GetVertex(hit.PrimitiveIndex, hit.GetBaryCentrics());
        color = v.Color;
    }
    else
    {
        float2 uv = ToSphereMapCoord(rayDirection);
        color = Background.SampleLevel(LinearSampler, uv, 0.0f);
    }

    Canvas[remappedId] = color;
}