//-------------------------------------------------------------------------------------------------
// File : asdxDeviceContext.cpp
// Desc : Device Context.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxDeviceContext.h>
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceContext class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
DeviceContext::DeviceContext()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
DeviceContext::~DeviceContext()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
bool DeviceContext::Init(const DeviceContextDesc& desc)
{
    if (desc.EnableDebug)
    {
        RefPtr<ID3D12Debug> pDebug;
        auto hr = D3D12GetDebugInterface( IID_PPV_ARGS(pDebug.GetAddress()) );
        if (SUCCEEDED(hr))
        { pDebug->EnableDebugLayer(); }

        pDebug.Reset();
    }

    uint32_t flags = 0;
    if (desc.EnableDebug)
    { flags |= DXGI_CREATE_FACTORY_DEBUG; }

    auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(m_pFactory.GetAddress()) );
    if (FAILED(hr))
    { return false; }

    RefPtr<IDXGIAdapter1> pAdapter;
    hr = m_pFactory->EnumAdapters1(0, pAdapter.GetAddress());
    if (FAILED(hr))
    { return false; }

    hr = pAdapter->QueryInterface( IID_PPV_ARGS(m_pAdapter.GetAddress()) );
    pAdapter.Reset();
    if (FAILED(hr))
    { return false; }

    RefPtr<IDXGIOutput> pOutput;
    hr = m_pAdapter->EnumOutputs(0, pOutput.GetAddress());
    if (FAILED(hr))
    { return false; }

    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.GetAddress()) );
    if (FAILED(hr))
    { return false; }

   {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountRes;
        heap_desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice, &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountSmp;
        heap_desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice, &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountRTV;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice, &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountDSV;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice, &heap_desc ) )
        { return false; }
    }

    if (!m_Queue[0].Init(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_DIRECT, desc.MaxSubmitCountGraphics))
    { return false; }

    if (!m_Queue[1].Init(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COMPUTE, desc.MaxSubmitCountCompute))
    { return false; }

    if (!m_Queue[2].Init(m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COPY, desc.MaxSubmitCountCopy))
    { return false; }

    m_Desc = desc;

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void DeviceContext::Term()
{
    for(auto i=0; i<3; ++i)
    { m_Queue[i].Term(); }

    for(auto i=0; i<4; ++i)
    { m_DescriptorHeap[i].Term(); }

    m_pDevice.Reset();
    m_pOutput.Reset();
    m_pAdapter.Reset();
    m_pFactory.Reset();
    m_Desc = {};
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
DeviceContextDesc DeviceContext::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
ID3D12Device* DeviceContext::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
IDXGIFactory5* DeviceContext::GetDXGIFactory() const
{ return m_pFactory.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
IDXGIAdapter4* DeviceContext::GetDXGIAdapter() const
{ return m_pAdapter.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
IDXGIOutput6* DeviceContext::GetDXGIOutput() const
{ return m_pOutput.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetGraphicsQueue()
{ return &m_Queue[0]; }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetComputeQueue()
{ return &m_Queue[1]; }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetCopyQueue()
{ return &m_Queue[2]; }

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
DescriptorHeap* DeviceContext::GetDescriptorHeap(uint32_t index)
{
    assert(index < 4);
    return &m_DescriptorHeap[index];
}

} // namespace asdx
