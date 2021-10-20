//-----------------------------------------------------------------------------
// File : FullScreenVS.hlsl
// Desc : Vertex Shader For Post Process.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position;
    float2 TexCoord;
};

static const VSOutput vertices[] = {
    { float4(-1.0f,  1.0f, 0.0f, 1.0f), float2(0.0f, 0.0f) },
    { float4( 3.0f,  1.0f, 0.0f, 1.0f), float2(2.0f, 0.0f) },
    { float4(-1.0f, -3.0f, 0.0f, 1.0f), float2(0.0f, 2.0f) },
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main( uint vertexId : SV_VertexId )
{
    return vertices[vertexId];
}
