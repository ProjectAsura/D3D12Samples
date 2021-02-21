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

#define VERTEX_STRIDE   (9 * 4)     // 1頂点あたりのサイズ.
#define INDEX_STRIDE    (3 * 4)     // 1ポリゴンあたりのインデックスサイズ.
#define TEXCOORD_OFFSET (3 * 4)     // 1頂点データの先頭からテクスチャ座標データまでのオフセット.
#define COLOR_OFFSET    (2 * 4)     // 1頂点データの先頭からカラーデータまでのオフセット.

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
    float3 Position;    // 位置座標.
    float2 TexCoord;    // テクスチャ座標.
    float4 Color;       // 頂点カラー.
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
//      プリミティブ番号を取得します.
//-----------------------------------------------------------------------------
uint3 GetIndices(uint triangleIndex)
{
    uint address = triangleIndex * INDEX_STRIDE;
    return Indices.Load3(address);
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
//      レイを求めます.
//-----------------------------------------------------------------------------
void CalcRay(float2 index, out float3 pos, out float3 dir)
{
    float4 orig   = float4(0.0f, 0.0f, 0.0f, 1.0f);           // カメラの位置.
    float4 screen = float4(-2.0f * index + 1.0f, 0.0f, 1.0f); // スクリーンの位置.

    orig   = mul(SceneParam.InvView,     orig);
    screen = mul(SceneParam.InvViewProj, screen);

    // w = 1 に射影.
    screen.xyz /= screen.w;

    // レイの位置と方向を設定.
    pos = orig.xyz;
    dir = normalize(screen.xyz - orig.xyz);
}

//-----------------------------------------------------------------------------
//      レイを生成します.
//-----------------------------------------------------------------------------
[shader("raygeneration")]
void OnGenerateRay()
{
    float2 index = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

    // レイを生成.
    float3 rayOrig;
    float3 rayDir;
    CalcRay(index, rayOrig, rayDir);

    // ペイロード初期化.
    RayPayload payload;
    payload.RayDir = rayDir;
    payload.Color  = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // レイを設定.
    RayDesc ray;
    ray.Origin    = rayOrig;
    ray.Direction = rayDir;
    ray.TMin      = 1e-3f;
    ray.TMax      = 10000.0f;

    // レイを追跡.
    TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    // レンダーターゲットに格納.
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

//-----------------------------------------------------------------------------
//      交差後の処理です.
//-----------------------------------------------------------------------------
[shader("closesthit")]
void OnClosestHit1(inout RayPayload payload, in HitArgs args)
{
    // 重心座標を求める.
    float3 barycentrics = float3(
        1.0f - args.barycentrics.x - args.barycentrics.y,
        args.barycentrics.x,
        args.barycentrics.y);

    // プリミティブ番号取得.
    uint triangleIndex = PrimitiveIndex();

    // 頂点データ取得.
    Vertex v = GetVertex(triangleIndex, barycentrics);

    // 頂点カラー乗算して，色を求める.
    payload.Color = BaseColor.SampleLevel(LinearWrap, v.TexCoord, 0.0f) * v.Color;
}

//-----------------------------------------------------------------------------
//      交差後の処理です.
//-----------------------------------------------------------------------------
[shader("closesthit")]
void OnClosestHit2(inout RayPayload payload, in HitArgs args)
{
    // 重心座標を求める.
    float3 barycentrics = float3(
        1.0f - args.barycentrics.x - args.barycentrics.y,
        args.barycentrics.x,
        args.barycentrics.y);

    // プリミティブ番号取得.
    uint triangleIndex = PrimitiveIndex();

    // 頂点データ取得.
    Vertex v = GetVertex(triangleIndex, barycentrics);

    // テクスチャカラーのみ.
    payload.Color = BaseColor.SampleLevel(LinearWrap, v.TexCoord, 0.0f);
}

//-----------------------------------------------------------------------------
//      非交差後の処理です.
//-----------------------------------------------------------------------------
[shader("miss")]
void OnMiss(inout RayPayload payload)
{
    // スフィアマップのテクスチャ座標を算出.
    float2 uv = ToSphereMapCoord(payload.RayDir);

    // スフィアマップをサンプル.
    float4 color = BackGround.SampleLevel(LinearWrap, uv, 0.0f);

    // 色を設定.
    payload.Color = color;
}
