//-------------------------------------------------------------------------------------------------
// File : asdxPipelineState.cpp
// Desc : Pipeline State.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxPipelineState.h>
#include <cassert>
#include <d3d11.h>


namespace {

//-------------------------------------------------------------------------------------------------
//      ブレンドステートを設定します.
//-------------------------------------------------------------------------------------------------
void SetBS(asdx::BlendType type, D3D12_BLEND_DESC& desc)
{
    struct ArgBS
    {
        D3D12_BLEND Src;
        D3D12_BLEND Dst;
    };

    static const ArgBS args[] = {
        { D3D12_BLEND_ONE,       D3D12_BLEND_ZERO           }, // Opaque
        { D3D12_BLEND_ONE,       D3D12_BLEND_INV_SRC_ALPHA  }, // AlphaBlend,
        { D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE            }, // Additive
        { D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA  }  // NonPremultiplied
    };

    desc.RenderTarget[0].BlendEnable = (args[type].Src != D3D12_BLEND_ONE ) || (args[type].Dst != D3D12_BLEND_ZERO);
    desc.RenderTarget[0].SrcBlend    = desc.RenderTarget[0].SrcBlendAlpha  = args[type].Src;
    desc.RenderTarget[0].DestBlend   = desc.RenderTarget[0].DestBlendAlpha = args[type].Dst;
    desc.RenderTarget[0].BlendOp     = desc.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask  = D3D12_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].LogicOpEnable          = false;
    desc.RenderTarget[0].LogicOp                = D3D12_LOGIC_OP_NOOP;
    desc.AlphaToCoverageEnable                  = false;
    desc.IndependentBlendEnable                 = false;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートを設定します.
//-------------------------------------------------------------------------------------------------
void SetDSS(asdx::DepthType type, D3D12_DEPTH_STENCIL_DESC& desc)
{
    struct ArgDSS
    {
        bool enable_test;
        bool enable_write;
    };

    static const ArgDSS args[] = {
        { false, false }, // None
        { true,  true  }, // Default
        { true,  false }, // ReadOnly
        { false, true  }, // WriteOnly
    };

    desc.DepthEnable    = (!args[type].enable_test && args[type].enable_write) ? TRUE : args[type].enable_test;
    desc.DepthWriteMask = ( args[type].enable_write ) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc      = (!args[type].enable_test && args[type].enable_write) ? D3D12_COMPARISON_FUNC_ALWAYS : D3D12_COMPARISON_FUNC_LESS_EQUAL;
    desc.StencilEnable                  = FALSE;
    desc.StencilReadMask                = D3D12_DEFAULT_STENCIL_READ_MASK;
    desc.StencilWriteMask               = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    desc.FrontFace.StencilFunc          = D3D12_COMPARISON_FUNC_ALWAYS;
    desc.FrontFace.StencilPassOp        = D3D12_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFailOp        = D3D12_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp   = D3D12_STENCIL_OP_KEEP;
    desc.BackFace                       = desc.FrontFace;
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザーステートを設定します.
//-------------------------------------------------------------------------------------------------
void SetRS(asdx::RasterizerType type, D3D12_RASTERIZER_DESC& desc)
{
    struct ArgRS
    {
        D3D12_CULL_MODE cull;
        D3D12_FILL_MODE fill;
    };

    static const ArgRS args[] = {
        { D3D12_CULL_MODE_NONE,     D3D12_FILL_MODE_SOLID     }, // CullNone
        { D3D12_CULL_MODE_FRONT,    D3D12_FILL_MODE_SOLID     }, // CullClockWise
        { D3D12_CULL_MODE_BACK,     D3D12_FILL_MODE_SOLID     }, // CullCounterClockWize
        { D3D12_CULL_MODE_NONE,     D3D12_FILL_MODE_WIREFRAME }, // WiretFrame 
    };

    desc.CullMode               = args[type].cull;
    desc.FillMode               = args[type].fill;
    desc.FrontCounterClockwise  = FALSE;
    desc.DepthBias              = 0;
    desc.DepthBiasClamp         = 0.0f;
    desc.SlopeScaledDepthBias   = 0.0f;
    desc.DepthClipEnable        = TRUE;
    desc.MultisampleEnable      = FALSE;
    desc.AntialiasedLineEnable  = FALSE;
    desc.ForcedSampleCount      = 0;
    desc.ConservativeRaster     = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// GraphicsPipelineStateDesc class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc::GraphicsPipelineStateDesc()
{
    pRootSignature = nullptr;
    VS = {};
    PS = {};
    DS = {};
    HS = {};
    GS = {};
    StreamOutput = {};
    SetBS(BlendType::Opaque, BlendState);
    SampleMask =  0;
    SetRS(RasterizerType::CullNone, RasterizerState);
    SetDSS(DepthType::Default, DepthStencilState);
    InputLayout             = {};
    IBStripCutValue         = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    PrimitiveTopologyType   = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    NumRenderTargets        = 0;
    for(auto i=0; i<8; ++i)
    { RTVFormats[i] = DXGI_FORMAT_UNKNOWN; }
    DSVFormat   = DXGI_FORMAT_UNKNOWN;
    SampleDesc  = {};
    CachedPSO   = {};
    Flags       = D3D12_PIPELINE_STATE_FLAG_NONE;
    NodeMask    = 0;
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc::~GraphicsPipelineStateDesc()
{
    m_pVS.Reset();
    m_pPS.Reset();
    m_pDS.Reset();
    m_pHS.Reset();
    m_pGS.Reset();
}

//-------------------------------------------------------------------------------------------------
//      頂点シェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool GraphicsPipelineStateDesc::LoadVS(const wchar_t* filename)
{
    m_pVS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pVS.GetAddress());
    if (FAILED(hr))
    { return false; }

    VS.pShaderBytecode = m_pVS->GetBufferPointer();
    VS.BytecodeLength  = m_pVS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ピクセルシェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool GraphicsPipelineStateDesc::LoadPS(const wchar_t* filename)
{
    m_pPS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pPS.GetAddress());
    if (FAILED(hr))
    { return false; }

    PS.pShaderBytecode = m_pPS->GetBufferPointer();
    PS.BytecodeLength  = m_pPS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ドメインシェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool GraphicsPipelineStateDesc::LoadDS(const wchar_t* filename)
{
    m_pDS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pDS.GetAddress());
    if (FAILED(hr))
    { return false; }

    GS.pShaderBytecode = m_pDS->GetBufferPointer();
    GS.BytecodeLength  = m_pDS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ハルシェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool GraphicsPipelineStateDesc::LoadHS(const wchar_t* filename)
{
    m_pHS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pHS.GetAddress());
    if (FAILED(hr))
    { return false; }

    HS.pShaderBytecode = m_pHS->GetBufferPointer();
    HS.BytecodeLength  = m_pHS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ジオメトリシェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool GraphicsPipelineStateDesc::LoadGS(const wchar_t* filename)
{
    m_pGS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pGS.GetAddress());
    if (FAILED(hr))
    { return false; }

    GS.pShaderBytecode = m_pGS->GetBufferPointer();
    GS.BytecodeLength  = m_pGS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      頂点シェーダを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetVS(const void* binary, size_t size)
{
    VS.pShaderBytecode = binary;
    VS.BytecodeLength  = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ピクセルシェーダを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetPS(const void* binary, size_t size)
{
    PS.pShaderBytecode = binary;
    PS.BytecodeLength  = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ドメインシェーダを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetDS(const void* binary, size_t size)
{
    DS.pShaderBytecode = binary;
    DS.BytecodeLength  = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ハルシェーダを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetHS(const void* binary, size_t size)
{
    HS.pShaderBytecode = binary;
    HS.BytecodeLength  = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ジオメトリシェーダを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetGS(const void* binary, size_t size)
{
    GS.pShaderBytecode = binary;
    GS.BytecodeLength = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ストリーム出力を設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetStreamOutput(D3D12_STREAM_OUTPUT_DESC value)
{
    StreamOutput = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ブレンドステートを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetBlendState(BlendType type)
{
    SetBS(type, BlendState);
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルステートを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetDepthState(DepthType type)
{
    SetDSS(type, DepthStencilState);
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ラスタライザーステートを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetRasterizerState(RasterizerType type)
{
    SetRS(type, RasterizerState);
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ルートシグニチャを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetRootSignature(ID3D12RootSignature* value)
{
    pRootSignature = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      レンダーターゲット数を設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetRenderTargetCount(uint32_t value)
{
    NumRenderTargets = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューフォーマットを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetRTVFormat(uint32_t index, DXGI_FORMAT format)
{
    assert(index < 8);
    RTVFormats[index] = format;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューフォーマットを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetDSVFormat(DXGI_FORMAT format)
{
    DSVFormat = format;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      入力レイアウトを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pValue, uint32_t count )
{
    InputLayout.pInputElementDescs = pValue;
    InputLayout.NumElements        = count;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      インデックスバッファストリップカット値を設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetIBStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)
{
    IBStripCutValue = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      プリミティブトポロジーを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE value)
{
    PrimitiveTopologyType = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      サンプリング構成を設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetSampleDesc(uint32_t count, uint32_t quality)
{
    SampleDesc.Count = count;
    SampleDesc.Quality = quality;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      キャッシュ済みパイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetCachedPSO(const void* binary, size_t size)
{
    CachedPSO.pCachedBlob = binary;
    CachedPSO.CachedBlobSizeInBytes = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      フラグを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetFlag(D3D12_PIPELINE_STATE_FLAGS value)
{
    Flags = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ノードマスクを設定します.
//-------------------------------------------------------------------------------------------------
GraphicsPipelineStateDesc& GraphicsPipelineStateDesc::SetNodeMask(uint32_t value)
{
    NodeMask = value;
    return *this;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ComputePipelineStateDesc class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc::ComputePipelineStateDesc()
{
    pRootSignature  = nullptr;
    CS              = {};
    CachedPSO       = {};
    Flags           = D3D12_PIPELINE_STATE_FLAG_NONE;
    NodeMask        = 0;
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc::~ComputePipelineStateDesc()
{
    m_pCS.Reset();
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダをロードします.
//-------------------------------------------------------------------------------------------------
bool ComputePipelineStateDesc::LoadCS(const wchar_t* filename)
{
    m_pCS.Reset();
    auto hr = D3DReadFileToBlob(filename, m_pCS.GetAddress());
    if (FAILED(hr))
    { return false; }

    CS.pShaderBytecode = m_pCS->GetBufferPointer();
    CS.BytecodeLength  = m_pCS->GetBufferSize();
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ルートシグニチャを設定します.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc& ComputePipelineStateDesc::SetRootSignature(ID3D12RootSignature* pValue)
{
    pRootSignature = pValue;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートシェーダを設定します.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc& ComputePipelineStateDesc::SetCS(const void* binary, size_t size)
{
    CS.pShaderBytecode = binary;
    CS.BytecodeLength  = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      キャッシュ済みパイプラインステートを設定します.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc& ComputePipelineStateDesc::SetCachedPSO(const void* binary, size_t size)
{
    CachedPSO.pCachedBlob = binary;
    CachedPSO.CachedBlobSizeInBytes = size;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      フラグを設定します.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc& ComputePipelineStateDesc::SetFlag(D3D12_PIPELINE_STATE_FLAGS value)
{
    Flags = value;
    return *this;
}

//-------------------------------------------------------------------------------------------------
//      ノードマスクを設定します.
//-------------------------------------------------------------------------------------------------
ComputePipelineStateDesc& ComputePipelineStateDesc::SetNodeMask(uint32_t value)
{
    NodeMask = value;
    return *this;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// PipelineState class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
PipelineState::PipelineState()
: m_IsGraphics(true)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
PipelineState::~PipelineState()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsGraphics(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    auto hr = pDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
    if (FAILED(hr))
    { return false; }

    m_IsGraphics = true;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートとして初期化します.
//-------------------------------------------------------------------------------------------------
bool PipelineState::InitAsCompute(ID3D12Device* pDevice, const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
    if (pDevice == nullptr || pDesc == nullptr)
    { return false; }

    auto hr = pDevice->CreateComputePipelineState(pDesc, IID_PPV_ARGS(m_pPSO.GetAddress()));
    if (FAILED(hr))
    { return false; }

    m_IsGraphics = false;
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void PipelineState::Term()
{
    m_pPSO.Reset();
    m_IsGraphics = true;
}

//-------------------------------------------------------------------------------------------------
//      グラフィックスパイプラインステートかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool PipelineState::IsGraphics() const
{ return m_IsGraphics; }

//-------------------------------------------------------------------------------------------------
//      コンピュートパイプラインステートかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool PipelineState::IsCompute() const
{ return m_IsGraphics == false; }

//-------------------------------------------------------------------------------------------------
//      パイプラインステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12PipelineState* PipelineState::GetPSO() const
{ return m_pPSO.GetPtr(); }

} // namespace asdx
