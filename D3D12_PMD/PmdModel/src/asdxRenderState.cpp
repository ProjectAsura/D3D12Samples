//-------------------------------------------------------------------------------------------------
// File : asdxRenderState.cpp
// Desc : Render State Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxRenderState.h>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      ブレンドステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_BLEND_DESC CreateBS( D3D12_BLEND src, D3D12_BLEND dst )
{
    D3D12_BLEND_DESC desc = asdx::RenderState::DefaultBlendDesc;

    desc.RenderTarget[0].BlendEnable = ( src != D3D12_BLEND_ONE ) || ( dst != D3D12_BLEND_ZERO );
    desc.RenderTarget[0].SrcBlend  = desc.RenderTarget[0].SrcBlendAlpha  = src;
    desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = dst;
    desc.RenderTarget[0].BlendOp   = desc.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_DESC CreateDSS( bool enableTest, bool enableWrite )
{
    D3D12_DEPTH_STENCIL_DESC desc = asdx::RenderState::DefaultDepthStencilDesc;

    desc.DepthEnable    = ( !enableTest && enableWrite ) ? TRUE : enableTest;
    desc.DepthWriteMask = ( enableWrite ) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc      = ( !enableTest && enableWrite ) ? D3D12_COMPARISON_FUNC_ALWAYS : D3D12_COMPARISON_FUNC_LESS_EQUAL;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザーステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_RASTERIZER_DESC CreateRS( D3D12_CULL_MODE cullMode, D3D12_FILL_MODE fillMode )
{
    D3D12_RASTERIZER_DESC desc = asdx::RenderState::DefaultRasterizerDesc;

    desc.CullMode = cullMode;
    desc.FillMode = fillMode;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_SAMPLER_DESC CreateSmp( D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode )
{
    D3D12_SAMPLER_DESC desc = asdx::RenderState::DefaultSamplerDesc;

    desc.Filter   = filter;
    desc.AddressU = addressMode;
    desc.AddressV = addressMode;
    desc.AddressW = addressMode;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_SAMPLER_DESC CreateSmpCmp( D3D12_FILTER filter, D3D12_COMPARISON_FUNC compare )
{
    D3D12_SAMPLER_DESC desc = asdx::RenderState::DefaultSamplerComparisonDesc;

    desc.Filter         = filter;
    desc.ComparisonFunc = compare;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      静的サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_STATIC_SAMPLER_DESC CreateStaticSmp( D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode )
{
    D3D12_STATIC_SAMPLER_DESC desc = asdx::RenderState::DefaultStaticSamplerDesc;

    desc.Filter   = filter;
    desc.AddressU = addressMode;
    desc.AddressV = addressMode;
    desc.AddressW = addressMode;

    return desc;
}

//-------------------------------------------------------------------------------------------------
//      静的サンプラーステートを生成します.
//-------------------------------------------------------------------------------------------------
D3D12_STATIC_SAMPLER_DESC CreateStaticSmpCmp( D3D12_FILTER filter, D3D12_COMPARISON_FUNC compare )
{
    D3D12_STATIC_SAMPLER_DESC desc = asdx::RenderState::DefaultStaticSamplerComparisonDesc;

    desc.Filter         = filter;
    desc.ComparisonFunc = compare;

    return desc;
}

} // namespace /* anonymous */


namespace asdx {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
const D3D12_RENDER_TARGET_BLEND_DESC RenderState::DefaultRenderTargetBlendDesc = {
    FALSE,
    FALSE,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
};

const D3D12_BLEND_DESC RenderState::DefaultBlendDesc = {
    FALSE,
    FALSE,
    {
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
        RenderState::DefaultRenderTargetBlendDesc,
    }
};

const D3D12_DEPTH_STENCILOP_DESC RenderState::DefaultDepthStencilOpDesc = {
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_STENCIL_OP_KEEP,
    D3D12_COMPARISON_FUNC_NEVER
};

const D3D12_DEPTH_STENCIL_DESC RenderState::DefaultDepthStencilDesc = {
    TRUE,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    FALSE,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    RenderState::DefaultDepthStencilOpDesc,
    RenderState::DefaultDepthStencilOpDesc,
};

const D3D12_RASTERIZER_DESC RenderState::DefaultRasterizerDesc = {
    D3D12_FILL_MODE_SOLID,
    D3D12_CULL_MODE_NONE,
    FALSE,
    D3D12_DEFAULT_DEPTH_BIAS,
    D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
    D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
    TRUE,
    FALSE,
    FALSE,
    0,
    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
};

const D3D12_SAMPLER_DESC RenderState::DefaultSamplerDesc = {
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    D3D12_DEFAULT_MAX_ANISOTROPY,
    D3D12_COMPARISON_FUNC_NEVER,
    { 0.0f, 0.0f, 0.0f, 0.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_SAMPLER_DESC RenderState::DefaultSamplerComparisonDesc = {
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    D3D12_DEFAULT_MAX_ANISOTROPY,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    0.0f,
    D3D12_FLOAT32_MAX
};

const D3D12_GRAPHICS_PIPELINE_STATE_DESC RenderState::DefaultPipelineStateDesc = {
    nullptr,                            // pRootSignature
    { nullptr, 0 },                     // VS
    { nullptr, 0 },                     // PS
    { nullptr, 0 },                     // DS
    { nullptr, 0 },                     // HS
    { nullptr, 0 },                     // GS
    { nullptr, 0, nullptr, 0, 0 },      // StreamOutput
    RenderState::DefaultBlendDesc,      // BlendState
    0,                                  // SampleMask
    RenderState::DefaultRasterizerDesc, // RasterizerState
    RenderState::DefaultDepthStencilDesc,   // DepthStencilState
    { nullptr, 0 },                     // InputLayout
    D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,    // IBStribCutValue
    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,         // PrimitiveTopologyType
    0,                              // NumRenderTargets
    { 
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[0]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[1]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[2]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[3]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[4]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[5]
        DXGI_FORMAT_UNKNOWN,        // RTVFormats[6]
        DXGI_FORMAT_UNKNOWN         // RTVFormats[7]
    },
    DXGI_FORMAT_UNKNOWN,                // DSVFormat
    { 0, 0 },                           // SampleDesc
    0,                                  // NodeMask
    { nullptr, 0 },                     // CachedPSO
    D3D12_PIPELINE_STATE_FLAG_NONE,     // Flags
};

const D3D12_STATIC_SAMPLER_DESC RenderState::DefaultStaticSamplerDesc = {
    D3D12_FILTER_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    D3D12_DEFAULT_MAX_ANISOTROPY,
    D3D12_COMPARISON_FUNC_NEVER,
    D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
    0.0f,
    D3D12_FLOAT32_MAX,
    0,
    0,
    D3D12_SHADER_VISIBILITY_ALL
};

const D3D12_STATIC_SAMPLER_DESC RenderState::DefaultStaticSamplerComparisonDesc = {
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_DEFAULT_MIP_LOD_BIAS,
    D3D12_DEFAULT_MAX_ANISOTROPY,
    D3D12_COMPARISON_FUNC_NEVER,
    D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
    0.0f,
    D3D12_FLOAT32_MAX,
    0,
    0,
    D3D12_SHADER_VISIBILITY_ALL
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderState class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      ブレンド設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_BLEND_DESC RenderState::CreateBlendDesc( const BlendState state )
{
    switch( state )
    {
    case BlendState::Opaque:
        return CreateBS( D3D12_BLEND_ONE, D3D12_BLEND_ZERO );

    case BlendState::AlphaBlend:
        return CreateBS( D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA );

    case BlendState::Addtive:
        return CreateBS( D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE );

    case BlendState::NonPremultiplied:
        return CreateBS( D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA );

    default:
        return DefaultBlendDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシル設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_DESC RenderState::CreateDepthStencilDesc( const DepthState state )
{
    switch( state )
    {
    case DepthState::None:
        return CreateDSS( false, false );

    case DepthState::Default:
        return CreateDSS( true, true );

    case DepthState::ReadOnly:
        return CreateDSS( true, false );

    case DepthState::WriteOnly:
        return CreateDSS( false, true );

    default:
        return DefaultDepthStencilDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザー設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_RASTERIZER_DESC RenderState::CreateRasterizerDesc( const RasterizerState state )
{
    switch( state )
    {
    case RasterizerState::CullNone:
        return CreateRS( D3D12_CULL_MODE_NONE, D3D12_FILL_MODE_SOLID );

    case RasterizerState::CullClockWise:
        return CreateRS( D3D12_CULL_MODE_FRONT, D3D12_FILL_MODE_SOLID );

    case RasterizerState::CullCounterClockWise:
        return CreateRS( D3D12_CULL_MODE_BACK, D3D12_FILL_MODE_SOLID );

    case RasterizerState::WireFrame:
        return CreateRS( D3D12_CULL_MODE_NONE, D3D12_FILL_MODE_WIREFRAME );

    default:
        return DefaultRasterizerDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      サンプラー設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_SAMPLER_DESC RenderState::CreateSamplerDesc( const SamplerState state )
{
    switch( state )
    {
    case SamplerState::PointWrap:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::PointClamp:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::PointMirror:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    case SamplerState::LinearWrap:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::LinearClamp:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::LinearMirror:
        return CreateSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    case SamplerState::AnisotropicWrap:
        return CreateSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::AnisotropicClamp:
        return CreateSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::AnisotropicMirror:
        return CreateSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    default:
        return DefaultSamplerDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      サンプラー設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_SAMPLER_DESC RenderState::CreateSamplerDesc( const SamplerComparisonState state )
{
    switch( state )
    {
    case SamplerComparisonState::PointLess:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::PointLessEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::PointGreater:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::PointGreaterEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    case SamplerComparisonState::LinearLess:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::LinearLessEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::LinearGreater:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::LinearGreaterEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    case SamplerComparisonState::AnisotropicLess:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::AnisotropicLessEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::AnisotropicGreater:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::AnisotropicGreaterEqual:
        return CreateSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    default:
        return DefaultSamplerComparisonDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      静的サンプラー設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_STATIC_SAMPLER_DESC RenderState::CreateStaticSamplerDesc( const SamplerState state )
{
    switch( state )
    {
    case SamplerState::PointWrap:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::PointClamp:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::PointMirror:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    case SamplerState::LinearWrap:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::LinearClamp:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::LinearMirror:
        return CreateStaticSmp( D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    case SamplerState::AnisotropicWrap:
        return CreateStaticSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP );

    case SamplerState::AnisotropicClamp:
        return CreateStaticSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

    case SamplerState::AnisotropicMirror:
        return CreateStaticSmp( D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_MIRROR );

    default:
        return DefaultStaticSamplerDesc;
    }
}

//-------------------------------------------------------------------------------------------------
//      静的サンプラー設定を生成します.
//-------------------------------------------------------------------------------------------------
D3D12_STATIC_SAMPLER_DESC RenderState::CreateStaticSamplerDesc( const SamplerComparisonState state )
{
    switch( state )
    {
    case SamplerComparisonState::PointLess:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::PointLessEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::PointGreater:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::PointGreaterEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    case SamplerComparisonState::LinearLess:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::LinearLessEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::LinearGreater:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::LinearGreaterEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    case SamplerComparisonState::AnisotropicLess:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_LESS );

    case SamplerComparisonState::AnisotropicLessEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_LESS_EQUAL );

    case SamplerComparisonState::AnisotropicGreater:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_GREATER );

    case SamplerComparisonState::AnisotropicGreaterEqual:
        return CreateStaticSmpCmp( D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_COMPARISON_FUNC_GREATER_EQUAL );

    default:
        return DefaultStaticSamplerComparisonDesc;
    }
}

} // namespace asdx
