//-------------------------------------------------------------------------------------------------
// File : SimplePS.hlsl
// Desc : Simple Pixel Shader
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "SimpleDef.hlsli"



SamplerState    ColorSmp     : register( s0 );

//-------------------------------------------------------------------------------------------------
//      �s�N�Z���V�F�[�_�̃��C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
PSOutput PSFunc(const VSOutput input)
{
    PSOutput output = (PSOutput)0;

    Texture2D<float4> colorTexture = ResourceDescriptorHeap[1];

    output.Color = input.Color * colorTexture.Sample( ColorSmp, input.TexCoord );

    return output;
}

