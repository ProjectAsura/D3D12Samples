//-------------------------------------------------------------------------------------------------
// File : Basic.hlsli
// Desc : Basic Data Defintions.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position   : POSITION;
    float3 Normal     : NORMAL;
    float2 TexCoord   : TEXCOORD;
    uint2  BoneIndex  : BONE_INDEX;
    float2 BoneWeight : BONE_WEIGHT;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4 Color : SV_TARGET0;
};


cbuffer Transform : register( b0 )
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float4x4 Bones[256];
};

cbuffer Material : register( b1 )
{
    float3  Diffuse  : packoffset( c0 );
    float   Alpha    : packoffset( c0.w );
    float   Power    : packoffset( c1 );
    float3  Specular : packoffset( c1.y );
    float3  Emissive : packoffset( c2 );
};

Texture2D       ColorMap : register( t0 );
SamplerState    ColorSmp : register( s0 );

