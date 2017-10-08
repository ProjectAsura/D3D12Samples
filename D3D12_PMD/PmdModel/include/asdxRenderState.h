//-------------------------------------------------------------------------------------------------
// File : asdxRenderState.h
// Desc : Render State Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTypedef.h>
#include <d3d12.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// BlendState enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class BlendState : u32
{
    Opaque = 0,             //!< 不透明.
    AlphaBlend,             //!< アルファブレンド.
    Addtive,                //!< 加算.
    NonPremultiplied,       //!< 非乗算アルファブレンド.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthState enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class DepthState : u32
{
    None = 0,           //!< 深度テストなし・深度書き込みなし
    Default,            //!< 深度テストあり・深度書き込みあり
    ReadOnly,           //!< 深度テストあり・深度書き込みなし
    WriteOnly,          //!< 深度テストなし・深度書き込みあり
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// RasterizerState enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class RasterizerState : u32
{
    CullNone = 0,           //!< カリングなし.
    CullClockWise,          //!< 時計回りをカリング
    CullCounterClockWise,   //!< 反時計回りをカリング
    WireFrame,              //!< ワイヤーフレーム.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// SamplerState enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class SamplerState : u32
{
    PointWrap = 0,          //!< ポイントサンプリング・繰り返し.
    PointClamp,             //!< ポイントサンプリング・クランプ.
    PointMirror,            //!< ポイントサンプリング・ミラー.
    LinearWrap,             //!< バイリニア補間・繰り返し.
    LinearClamp,            //!< バイリニア補間・クランプ.
    LinearMirror,           //!< バイリニア補間・ミラー.
    AnisotropicWrap,        //!< 異方性補間・繰り返し.
    AnisotropicClamp,       //!< 異方性補間・クランプ.
    AnisotropicMirror,      //!< 異方性補間・ミラー.
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// SamplerComparisonState enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class SamplerComparisonState : u32
{
    PointLess,                  //!< ポイントサンプリング, src <  dst
    PointLessEqual,             //!< ポイントサンプリング, src <= dst
    PointGreater,               //!< ポイントサンプリング, src >  dst
    PointGreaterEqual,          //!< ポイントサンプリング, src >= dst
    LinearLess,                 //!< バイリニア補間, src <  dst
    LinearLessEqual,            //!< バイリニア補間, src <= dst
    LinearGreater,              //!< バイリニア補間, src >  dst
    LinearGreaterEqual,         //!< バイリニア補間, src >= dst
    AnisotropicLess,            //!< 異方性補間, src <  dst
    AnisotropicLessEqual,       //!< 異方性補間, src <= dst
    AnisotropicGreater,         //!< 異方性補間, src >  dst
    AnisotropicGreaterEqual,    //!< 異方性補間, src >= dst
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// RenderState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class RenderState
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public varaibles.
    //=============================================================================================
    static const D3D12_BLEND_DESC                   DefaultBlendDesc;                   //!< デフォルトブレンドステート設定です.
    static const D3D12_DEPTH_STENCILOP_DESC         DefaultDepthStencilOpDesc;          //!< デフォルト深度ステンシル操作設定です.
    static const D3D12_DEPTH_STENCIL_DESC           DefaultDepthStencilDesc;            //!< デフォルト深度ステンシルステート設定です.
    static const D3D12_RASTERIZER_DESC              DefaultRasterizerDesc;              //!< デフォルトラスタライザーステート設定です.
    static const D3D12_SAMPLER_DESC                 DefaultSamplerDesc;                 //!< デフォルトサンプラーステート設定です.
    static const D3D12_SAMPLER_DESC                 DefaultSamplerComparisonDesc;       //!< デフォルトサンプラー比較ステート設定です.
    static const D3D12_STATIC_SAMPLER_DESC          DefaultStaticSamplerDesc;           //!< デフォルト静的サンプラーステート設定です.
    static const D3D12_STATIC_SAMPLER_DESC          DefaultStaticSamplerComparisonDesc; //!< デフォルト静的サンプラー比較ステート設定です.
    static const D3D12_RENDER_TARGET_BLEND_DESC     DefaultRenderTargetBlendDesc;       //!< デフォルトレンダーターゲットブレンド設定です.
    static const D3D12_GRAPHICS_PIPELINE_STATE_DESC DefaultPipelineStateDesc;           //!< デフォルトパイプラインステート設定です.

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      ブレンド設定を生成します.
    //!
    //! @param[in]      state       ブレンドステート.
    //! @return     ブレンド設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_BLEND_DESC CreateBlendDesc( const BlendState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      深度ステンシル設定を生成します.
    //!
    //! @param[in]      state       深度ステンシルステート.
    //! @return     深度ステンシル設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc( const DepthState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      ラスタライザー設定を生成します.
    //!
    //! @param[in]      state       ラスタライザーステート.
    //! @return     ラスタライザー設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_RASTERIZER_DESC CreateRasterizerDesc( const RasterizerState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラー設定を生成します.
    //!
    //! @param[in]      state       サンプラーステート.
    //! @return     サンプラー設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_SAMPLER_DESC CreateSamplerDesc( const SamplerState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      サンプラー設定を生成します.
    //!
    //! @param[in]      state       サンプラー比較ステート.
    //! @return     サンプラー設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_SAMPLER_DESC CreateSamplerDesc( const SamplerComparisonState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      静的サンプラー設定を生成します.
    //!
    //! @param[in]      state       サンプラーステート.
    //! @return     静的サンプラー設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_STATIC_SAMPLER_DESC CreateStaticSamplerDesc( const SamplerState state );

    //---------------------------------------------------------------------------------------------
    //! @brief      静的サンプラー設定を生成します.
    //!
    //! @param[in]      state       サンプラー比較ステート.
    //! @return     静的サンプラー設定を返却します.
    //---------------------------------------------------------------------------------------------
    static D3D12_STATIC_SAMPLER_DESC CreateStaticSamplerDesc( const SamplerComparisonState state );
};


} // namespace asdx

