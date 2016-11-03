//-------------------------------------------------------------------------------------------------
// File : BasicVS.hlsl
// Desc : Basic Vertex Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "Basic.hlsli"


//-------------------------------------------------------------------------------------------------
//      頂点シェーダメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
VSOutput VSFunc(VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4 localPos = float4(input.Position, 1.0f);

    float4x4 skinning = (float4x4)0;
    skinning += Bones[input.BoneIndex.x] * input.BoneWeight.x;
    skinning += Bones[input.BoneIndex.y] * input.BoneWeight.y;

    float4 transPos = mul(skinning, localPos);
    float4 worldPos = mul(World, transPos);
    float4 viewPos  = mul(View, worldPos);
    float4 projPos  = mul(Proj, viewPos);

    float3 transNormal = mul((float3x3)skinning, input.Normal);
    float3 worldNormal = mul((float3x3)World, transNormal);
    worldNormal = normalize(worldNormal);

    output.Position = projPos;
    output.TexCoord = input.TexCoord;
    output.Normal   = worldNormal;

    return output;
}