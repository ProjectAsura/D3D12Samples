//-------------------------------------------------------------------------------------------------
// File : SimplePS.hlsl
// Desc : Simple Pixel Shader
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "SimpleDef.hlsli"


//-------------------------------------------------------------------------------------------------
//      �s�N�Z���V�F�[�_�̃��C���G���g���[�|�C���g�ł�.
//-------------------------------------------------------------------------------------------------
PSOutput PSFunc(const VSOutput input)
{
    PSOutput output = (PSOutput)0;

    output.Color = input.Color * ColorTexture.Sample( ColorSmp, input.TexCoord );

    return output;
}
