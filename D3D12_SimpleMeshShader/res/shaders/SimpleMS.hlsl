//-----------------------------------------------------------------------------
// File : SimpleMeshShader.hlsl
// Desc : Simple Mesh Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VertexInput structure
///////////////////////////////////////////////////////////////////////////////
struct VertexInput
{
    float3 Position;    // 頂点座標.
    float4 Color;       // 頂点カラー.
};

///////////////////////////////////////////////////////////////////////////////
// VertexOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// PrimitiveOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PrimitiveOutput
{
    uint PrimitiveId : INDEX0;
};

//-----------------------------------------------------------------------------
// Resources and Samplers.
//-----------------------------------------------------------------------------
StructuredBuffer<VertexInput>   Vertices    : register(t0);
StructuredBuffer<uint3>         Indices     : register(t1);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(32, 1, 1)]
[outputtopology("triangle")]    // 新しいアトリビュート "line" または "triangle" が指定できる.
void main
(
    uint groupIndex : SV_GroupIndex,
    out vertices VertexOutput verts[3],         // verticesは必須属性.
    out indices uint3 tris[1],                  // indicesも必須属性.
    out primitives PrimitiveOutput prims[1]     // primitivesはオプション属性.
)
{
    // スレッドグループの頂点とプリミティブの数を設定. この関数の呼び出しは必須.
    SetMeshOutputCounts(3, 1);

    if (groupIndex < 1)
    {
        tris[groupIndex] = Indices[groupIndex];         // 頂点インデックスを設定.
        prims[groupIndex].PrimitiveId = groupIndex;     // プリミティブインデックスを設定.
    }

    if (groupIndex < 3)
    {
        VertexOutput vout;
        vout.Position   = float4(Vertices[groupIndex].Position, 1.0f);
        vout.Color      = Vertices[groupIndex].Color;

        verts[groupIndex] = vout;   // 頂点を出力.
    }
}
