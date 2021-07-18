//-------------------------------------------------------------------------------------------------
// File : SimpleDef.hlsli
// Desc : Data Layout.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3  Position : POSITION;
    float3  Normal   : NORMAL;
    float2  TexCoord : TEXCOORD;
    float4  Color    : VTX_COLOR;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position : SV_POSITION;
    float3  Normal   : NORMAL;
    float2  TexCoord : TEXCOORD;
    float4  Color    : VTX_COLOR;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4  Color   : SV_TARGET0;
};
