//-------------------------------------------------------------------------------------------------
// File : asdxDevice.cpp
// Desc : Device Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxDevice.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Device::Device()
: m_Device      ()
, m_Factory     ()
, m_OffsetCount ()
, m_HeapBuffer  ()
, m_HeapSmp     ()
, m_HeapRTV     ()
, m_HeapDSV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Device::~Device()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool Device::Init( DEVICE_DESC* pDesc )
{
    if ( pDesc == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    HRESULT hr = S_OK;

    #if ASDX_IS_DEBUG
    {
        RefPtr<ID3D12Debug> debug;
        hr = D3D12GetDebugInterface( IID_PPV_ARGS( debug.GetAddress()));
        if ( SUCCEEDED(hr) )
        { debug->EnableDebugLayer(); }
    }
    #endif

    hr = CreateDXGIFactory1( IID_PPV_ARGS( m_Factory.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : CreateDXGIFactory1() Failed." );
        return false;
    }

    hr = D3D12CreateDevice( nullptr, pDesc->FeatureLevel, IID_PPV_ARGS( m_Device.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : D3D12CreateDevice() Failed." );
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = pDesc->CountDesc.Buffer;
    heapDesc.NodeMask       = 0;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if ( !m_HeapBuffer.Init( m_Device.GetPtr(), &heapDesc ) )
    {
        ELOG( "Error : DescHeap::Init() Failed." );
        return false;
    }

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapDesc.NumDescriptors = pDesc->CountDesc.Sampler;
    if ( !m_HeapSmp.Init( m_Device.GetPtr(), &heapDesc ) )
    {
        ELOG( "Error : DescHeap::Init() Failed." );
        return false;
    }

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = pDesc->CountDesc.RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if ( !m_HeapRTV.Init( m_Device.GetPtr(), &heapDesc ) ) 
    {
        ELOG( "Error : DescHeap::Init() Failed." );
        return false;
    }

    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.NumDescriptors = pDesc->CountDesc.DSV;
    if ( !m_HeapDSV.Init( m_Device.GetPtr(), &heapDesc ) ) 
    {
        ELOG( "Error : DescHeap::Init() Failed." );
        return false;
    }

    m_OffsetCount.Buffer  = 0;
    m_OffsetCount.Sampler = 0;
    m_OffsetCount.RTV     = 0;
    m_OffsetCount.DSV     = 0;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void Device::Term()
{
    m_HeapBuffer.Term();
    m_HeapSmp.Term();
    m_HeapRTV.Term();
    m_HeapDSV.Term();
    m_Device.Reset();
    m_Factory.Reset();

    m_OffsetCount.Buffer  = 0;
    m_OffsetCount.Sampler = 0;
    m_OffsetCount.RTV     = 0;
    m_OffsetCount.DSV     = 0;
}

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Device* Device::GetDevice() const
{ return m_Device.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      ファクトリを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory4* Device::GetFactory() const
{ return m_Factory.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      スワップチェインを生成します.
//-------------------------------------------------------------------------------------------------
bool Device::CreateSwapChain
(
    ID3D12CommandQueue*     pQueue,
    DXGI_SWAP_CHAIN_DESC*   pDesc,
    IDXGISwapChain3**       ppSwapChain
)
{
    if ( pQueue == nullptr || pDesc == nullptr || ppSwapChain == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    HRESULT hr = S_OK;

    RefPtr<IDXGISwapChain> pSwapChain;
    hr = m_Factory->CreateSwapChain( pQueue, pDesc, pSwapChain.GetAddress() );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGIFactory::CreateSwapChain() Failed." );
        return false;
    }

    hr = pSwapChain->QueryInterface( IID_PPV_ARGS( ppSwapChain ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::QueryInterface() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      定数バッファビューを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateCBV( D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc )
{
    DescHandle handle(
        m_HeapBuffer.GetHandleCPU( m_OffsetCount.Buffer ),
        m_HeapBuffer.GetHandleGPU( m_OffsetCount.Buffer ) );
    m_OffsetCount.Buffer++;
    m_Device->CreateConstantBufferView( pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateSRV( ID3D12Resource* pResource, D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc )
{
    DescHandle handle(
        m_HeapBuffer.GetHandleCPU( m_OffsetCount.Buffer ),
        m_HeapBuffer.GetHandleGPU( m_OffsetCount.Buffer ) );
    m_OffsetCount.Buffer++;
    m_Device->CreateShaderResourceView( pResource, pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      アンオーダードアクセスビューを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateUAV
(
    ID3D12Resource* pResource,
    ID3D12Resource* pCounterResource,
    D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc
)
{
    DescHandle handle(
        m_HeapBuffer.GetHandleCPU( m_OffsetCount.Buffer ),
        m_HeapBuffer.GetHandleGPU( m_OffsetCount.Buffer ) );
    m_OffsetCount.Buffer++;
    m_Device->CreateUnorderedAccessView( pResource, pCounterResource, pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      サンプラーを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateSmp( D3D12_SAMPLER_DESC* pDesc )
{
    DescHandle handle(
        m_HeapSmp.GetHandleCPU( m_OffsetCount.Sampler ),
        m_HeapSmp.GetHandleGPU( m_OffsetCount.Sampler ) );
    m_OffsetCount.Sampler++;
    m_Device->CreateSampler( pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      深度ステンシルビューを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateDSV( ID3D12Resource* pResource, D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc )
{
    DescHandle handle( m_HeapDSV.GetHandleCPU( m_OffsetCount.DSV ) );
    m_OffsetCount.DSV++;
    m_Device->CreateDepthStencilView( pResource, pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      レンダーターゲットビューを生成します.
//-------------------------------------------------------------------------------------------------
DescHandle Device::CreateRTV( ID3D12Resource* pResource, D3D12_RENDER_TARGET_VIEW_DESC* pDesc )
{
    DescHandle handle( m_HeapRTV.GetHandleCPU( m_OffsetCount.RTV ) );
    m_OffsetCount.RTV++;
    m_Device->CreateRenderTargetView( pResource, pDesc, handle.GetHandleCpu() );
    return handle;
}

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D12Device* Device::operator -> () const
{ return m_Device.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープ設定コマンドを生成します.
//-------------------------------------------------------------------------------------------------
void Device::MakeSetDescHeapCmd( ID3D12GraphicsCommandList* pCmd )
{
    ID3D12DescriptorHeap* heaps[2] = { m_HeapBuffer.GetPtr(), m_HeapSmp.GetPtr() };
    pCmd->SetDescriptorHeaps( 2, heaps );
}


} // namespace asdx
