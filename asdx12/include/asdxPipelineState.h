//-------------------------------------------------------------------------------------------------
// File : asdxPipelineState.h
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <cstdint>
#include <asdxRefPtr.h>
#include <d3dcompiler.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// BlendType enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum BlendType
{
    Opaque = 0,             //!< 不透明
    AlphaBlend,             //!< アルファブレンド.
    Additive,               //!< 加算.
    NonPremultiplied,       //!< 非事前乗算済みアルファブレンド.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DepthType enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum DepthType
{
    None = 0,       //!< 深度テストなし, 深度書き込みなし.
    Default,        //!< 深度テストあり, 深度書き込みあり.
    Readonly,       //!< 深度テストあり, 深度書き込みなし.
    WriteOnly,      //!< 深度テストなし, 深度書き込みあり.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RasterizerType enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum RasterizerType
{
    CullNone = 0,               //!< カリングなし. マルチサンプル無効.
    CullClockWise,              //!< 時計回りをカリング. マルチサンプル無効.
    CullCounterClockWise,       //!< 反時計周りをカリング. マルチサンプル無効.
    WireFrame,                  //!< ワイヤーフレーム. マルチサンプル無効.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class GraphicsPipelineStateDesc : public D3D12_GRAPHICS_PIPELINE_STATE_DESC
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    GraphicsPipelineStateDesc();
    ~GraphicsPipelineStateDesc();

    bool LoadVS(const wchar_t* filename);
    bool LoadPS(const wchar_t* filename);
    bool LoadDS(const wchar_t* filename);
    bool LoadHS(const wchar_t* filename);
    bool LoadGS(const wchar_t* filename);

    GraphicsPipelineStateDesc& SetVS(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetPS(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetDS(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetHS(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetGS(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetStreamOutput(D3D12_STREAM_OUTPUT_DESC value);
    GraphicsPipelineStateDesc& SetBlendState(BlendType type);
    GraphicsPipelineStateDesc& SetDepthState(DepthType type);
    GraphicsPipelineStateDesc& SetRasterizerState(RasterizerType type);
    GraphicsPipelineStateDesc& SetRootSignature(ID3D12RootSignature* pValue);
    GraphicsPipelineStateDesc& SetRenderTargetCount(uint32_t value);
    GraphicsPipelineStateDesc& SetRTVFormat(uint32_t index, DXGI_FORMAT format);
    GraphicsPipelineStateDesc& SetDSVFormat(DXGI_FORMAT format);
    GraphicsPipelineStateDesc& SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pValue, uint32_t count);
    GraphicsPipelineStateDesc& SetIBStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value);
    GraphicsPipelineStateDesc& SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE value);
    GraphicsPipelineStateDesc& SetSampleDesc(uint32_t count, uint32_t quality);
    GraphicsPipelineStateDesc& SetCachedPSO(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetFlag(D3D12_PIPELINE_STATE_FLAGS value);
    GraphicsPipelineStateDesc& SetNodeMask(uint32_t value);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3DBlob>    m_pVS;
    RefPtr<ID3DBlob>    m_pPS;
    RefPtr<ID3DBlob>    m_pDS;
    RefPtr<ID3DBlob>    m_pHS;
    RefPtr<ID3DBlob>    m_pGS;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ComputePipelineStateDesc class
///////////////////////////////////////////////////////////////////////////////////////////////////
class ComputePipelineStateDesc : public D3D12_COMPUTE_PIPELINE_STATE_DESC
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    ComputePipelineStateDesc();
    ~ComputePipelineStateDesc();

    bool LoadCS(const wchar_t* filename);

    ComputePipelineStateDesc& SetRootSignature(ID3D12RootSignature* pValue);
    ComputePipelineStateDesc& SetCS(const void* binary, size_t size);
    ComputePipelineStateDesc& SetCachedPSO(const void* binary, size_t size);
    ComputePipelineStateDesc& SetFlag(D3D12_PIPELINE_STATE_FLAGS value);
    ComputePipelineStateDesc& SetNodeMask(uint32_t value);

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3DBlob>    m_pCS;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineState
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    PipelineState();
    ~PipelineState();

    bool InitAsGraphics(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc);
    bool InitAsCompute (ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc);
    void Term();

    bool IsGraphics() const;
    bool IsCompute() const;

    ID3D12PipelineState* GetPSO() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D12PipelineState>    m_pPSO;
    bool                           m_IsGraphics;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

} // namespace asdx
