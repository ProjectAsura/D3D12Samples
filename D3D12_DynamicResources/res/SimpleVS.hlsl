//-------------------------------------------------------------------------------------------------
// File : SimpleVS.hlsl
// Desc : Simple Vertex Shader.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "SimpleDef.hlsli"


struct TransformBuffer
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
};

//-------------------------------------------------------------------------------------------------
//      頂点シェーダのメインエントリーポイントです.
//-------------------------------------------------------------------------------------------------
VSOutput VSFunc(const VSInput input)
{
    VSOutput output = (VSOutput)0;

    ConstantBuffer<TransformBuffer> cb = ResourceDescriptorHeap[0];

    float4 localPos = float4(input.Position, 1.0f);
    float4 worldPos = mul(cb.World, localPos);
    float4 viewPos  = mul(cb.View,  worldPos);
    float4 projPos  = mul(cb.Proj,  viewPos);

    output.Position = projPos;
    output.Normal   = input.Normal;
    output.TexCoord = input.TexCoord;
    output.Color    = input.Color;

    return output;
}

