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
public:
    GraphicsPipelineStateDesc();
    ~GraphicsPipelineStateDesc();

    bool LoadVS(const char* filename);
    bool LoadPS(const char* filename);
    bool LoadDS(const char* filename);
    bool LoadHS(const char* filename);
    bool LoadGS(const char* filename);

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
    GraphicsPipelineStateDesc& SetRenderTargetFormat(uint32_t index, DXGI_FORMAT format);
    GraphicsPipelineStateDesc& SetDepthStencilFormat(DXGI_FORMAT format);
    GraphicsPipelineStateDesc& SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pValue, uint32_t count);
    GraphicsPipelineStateDesc& SetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value);
    GraphicsPipelineStateDesc& SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE value);
    GraphicsPipelineStateDesc& SetSampleDesc(uint32_t count, uint32_t quality);
    GraphicsPipelineStateDesc& SetCachedPipelineState(const void* binary, size_t size);
    GraphicsPipelineStateDesc& SetFlag(D3D12_PIPELINE_STATE_FLAGS value);

private:
    RefPtr<ID3DBlob>    m_pVS;
    RefPtr<ID3DBlob>    m_pPS;
    RefPtr<ID3DBlob>    m_pDS;
    RefPtr<ID3DBlob>    m_pHS;
    RefPtr<ID3DBlob>    m_pGS;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// ComputePipelineStateDesc class
///////////////////////////////////////////////////////////////////////////////////////////////////
class ComputePipelineStateDesc : public D3D12_COMPUTE_PIPELINE_STATE_DESC
{
public:
    ComputePipelineStateDesc();
    ~ComputePipelineStateDesc();

    bool LoadCS(const char* filename);

    ComputePipelineStateDesc& SetRootSignature(ID3D12RootSignature* pValue);
    ComputePipelineStateDesc& SetCS(const void* binary, size_t size);
    ComputePipelineStateDesc& SetCachedPipelineState(const void* binary, size_t size);
    ComputePipelineStateDesc& SetFlag(D3D12_PIPELINE_STATE_FLAGS value);

private:
    RefPtr<ID3DBlob>    m_pCS;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////
class PipelineState
{
public:
    PipelineState();
    ~PipelineState();

    bool InitAsGraphics(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc);
    bool InitAsCompute(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc);

    bool IsGraphics() const;
    bool IsCompute() const;

    ID3D12PipelineState* GetPipelineState() const;

private:
    RefPtr<ID3D12PipelineState*>    m_pPipelineState;
};

} // namespace asdx