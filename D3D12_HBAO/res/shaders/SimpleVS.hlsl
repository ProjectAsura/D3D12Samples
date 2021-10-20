//-----------------------------------------------------------------------------
// File : SimpleVS.hlsl
// Desc : Vertex Shader For Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct SceneParam
{
    float4x4 View;
    float4x4 Proj;
    float4x4 InvView;
    float4x4 InvProj;
};

struct ModelParam
{
    float4x4 World;
};

ConstantBuffer<ModelParam> Model : register(b0);
ConstantBuffer<Transform> Scene : register(b1);

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

    float3 worldNormal  = mul((float3x3)World, input.Normal);
    float3 worldTangent = mul((float3x3)World, input.Tangent);

    output.Position = projPos;
    output.Normal   = normalize(worldNormal);
    output.Tangent  = normalize(worldTangent);
    output.TexCoord = input.TexCoord;

    return output;
}