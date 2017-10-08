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
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceContext::DeviceContext()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
DeviceContext::~DeviceContext()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
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
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice.GetPtr(), &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountSmp;
        heap_desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice.GetPtr(), &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountRTV;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice.GetPtr(), &heap_desc ) )
        { return false; }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = desc.MaxCountDSV;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[heap_desc.Type].Init(m_pDevice.GetPtr(), &heap_desc ) )
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
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void DeviceContext::Term()
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    for(auto i=0; i<3; ++i)
    { m_Queue[i].Term(); }

    for(auto i=0; i<4; ++i)
    { m_DescriptorHeap[i].Term(); }

    auto itr = m_Disposer.begin();
    while(itr != m_Disposer.end())
    {
        if (itr->pObject != nullptr)
        {
            itr->pObject->Release();
            itr->pObject = nullptr;
        }

        itr++;
    }
    m_Disposer.clear();

    m_pDevice.Reset();
    m_pOutput.Reset();
    m_pAdapter.Reset();
    m_pFactory.Reset();
    m_Desc = {};
}

//-------------------------------------------------------------------------------------------------
//      破棄リストに追加します.
//-------------------------------------------------------------------------------------------------
void DeviceContext::AddToDisposer(ID3D12Object* pObject, uint32_t life)
{
    // 念のためにスレッドを止める.
    std::lock_guard<std::mutex> locker(m_Mutex);

    DisposeItem item;
    item.pObject   = pObject;
    item.LifeCount = life;

    m_Disposer.push_back(item);
}

//-------------------------------------------------------------------------------------------------
//      次のフレームに進みます.
//-------------------------------------------------------------------------------------------------
void DeviceContext::NextFrame()
{
    // 別スレッドでAddToDisposer() が呼ばれていると困るので，念のためにスレッドを止める.
    std::lock_guard<std::mutex> locker(m_Mutex);

    auto itr = m_Disposer.begin();
    while(itr != m_Disposer.end())
    {
        itr->LifeCount--;
        if (itr->LifeCount == 0)
        {
            itr->pObject->Release();
            itr->pObject = nullptr;

            itr = m_Disposer.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      構成設定を取得します.
//-------------------------------------------------------------------------------------------------
DeviceContextDesc DeviceContext::GetDesc() const
{ return m_Desc; }

//-------------------------------------------------------------------------------------------------
//      デバイスを取得します.
//-------------------------------------------------------------------------------------------------
ID3D12Device* DeviceContext::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      DXGIファクトリーを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIFactory5* DeviceContext::GetDXGIFactory() const
{ return m_pFactory.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      DXGIアダプターを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIAdapter3* DeviceContext::GetDXGIAdapter() const
{ return m_pAdapter.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      DXGIアウトプットを取得します.
//-------------------------------------------------------------------------------------------------
IDXGIOutput5* DeviceContext::GetDXGIOutput() const
{ return m_pOutput.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetGraphicsQueue()
{ return &m_Queue[0]; }

//-------------------------------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetComputeQueue()
{ return &m_Queue[1]; }

//-------------------------------------------------------------------------------------------------
//      コピーキューを取得します.
//-------------------------------------------------------------------------------------------------
CommandQueue* DeviceContext::GetCopyQueue()
{ return &m_Queue[2]; }

//-------------------------------------------------------------------------------------------------
//      ディスクリプタヒープを取得します.
//-------------------------------------------------------------------------------------------------
DescriptorHeap* DeviceContext::GetDescriptorHeap(uint32_t index)
{
    assert(index < 4);
    return &m_DescriptorHeap[index];
}

} // namespace asdx
