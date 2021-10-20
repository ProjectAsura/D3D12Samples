//-----------------------------------------------------------------------------
// File : SimplePS.hlsl
// Desc : Pixel Shader For Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct PSOutput
{
    float2 Normal : SV_TARGET0;
};

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
float2 OctWrap(float2 v)
{ return (1.0f - abs(v.yx)) * (v.xy >= 0.0f ? 1.0f : -1.0f); }

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
float2 PackNormal(float3 normal)
{
    // Octahedron normal vector encoding.
    // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
    float3 n = normal / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    n.xy = (n.z >= 0.0f) ? n.xy : OctWrap(n.xy);
    return n.xy * 0.5f + 0.5f;
}

//-----------------------------------------------------------------------------
//      エントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    PSOutput output = (PSOutput)0;

    output.Normal = PackNormal(input.Normal);

    return output;
}