//-----------------------------------------------------------------------------
// File : SimpleVS.hlsl
// Desc : Vertex Shader For Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4 View;
    float4x4 Proj;
    float4x4 InvView;
    float4x4 InvProj;
    float    NearClip;
    float    FarClip;
    float    FieldOfView;
    float    AspectRatio;
};

///////////////////////////////////////////////////////////////////////////////
// ModelParam structure
///////////////////////////////////////////////////////////////////////////////
struct ModelParam
{
    float4x4 World;
};

//-----------------------------------------------------------------------------
// Constant Buffers
//-----------------------------------------------------------------------------
ConstantBuffer<ModelParam> Model : register(b0);
ConstantBuffer<SceneParam> Scene : register(b1);

//-----------------------------------------------------------------------------
//      エントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4 localPos = float4( input.Position, 1.0f );
    float4 worldPos = mul( Model.World, localPos );
    float4 viewPos  = mul( Scene.View,  worldPos );
    float4 projPos  = mul( Scene.Proj,  viewPos );

    float3 worldNormal  = mul((float3x3)Model.World, input.Normal);
    float3 worldTangent = mul((float3x3)Model.World, input.Tangent);

    output.Position = projPos;
    output.Normal   = normalize(worldNormal);
    output.Tangent  = normalize(worldTangent);
    output.TexCoord = input.TexCoord;

    return output;
}