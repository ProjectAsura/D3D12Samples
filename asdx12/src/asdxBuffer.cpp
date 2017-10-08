//-------------------------------------------------------------------------------------------------
// File : asdxBuffer.cpp
// Desc : Buffer.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxBuffer.h>


namespace asdx{

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです
//-------------------------------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer()
: m_Index(0)
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool ConstantBuffer::Init(ID3D12Device* pDevice, DescriptorHeap* pHeap, size_t size, uint32_t count)
{
    if (pDevice == nullptr || pHeap == nullptr || size == 0 || count == 0)
    { return false; }

    size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    UINT64 sizeAligned  = (size + (align - 1)) & ~(align - 1); // alignに切り上げる.

    // ヒーププロパティ.
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask       = 1;
    prop.VisibleNodeMask        = 1;

    // リソースの設定.
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = sizeAligned;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    m_Instance.resize(count);

    for(auto i=0u; i<count; ++i)
    {
        // リソースを生成.
        auto hr = pDevice->CreateCommittedResource(
            &prop, 
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_Instance[i].pResource.GetAddress()));
        if (FAILED(hr))
        { return false; }

        // メモリマッピングしておきます.
        hr = m_Instance[i].pResource->Map(0, nullptr, &m_Instance[i].pMappedPtr);
        if (FAILED(hr))
        { return false; }

        m_Instance[i].pDescriptor = pHeap->CreateDescriptor();
        if (m_Instance[i].pDescriptor == nullptr)
        { return false; }

        D3D12_CONSTANT_BUFFER_VIEW_DESC view_desc = {};
        view_desc.BufferLocation = m_Instance[i].pResource->GetGPUVirtualAddress();
        view_desc.SizeInBytes    = UINT(sizeAligned);
        
        pDevice->CreateConstantBufferView(&view_desc, m_Instance[i].pDescriptor->GetHandleCPU());
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void ConstantBuffer::Term()
{
    for(size_t i=0; i<m_Instance.size(); ++i)
    {
        m_Instance[i].pResource.Reset();
        m_Instance[i].pDescriptor.Reset();
        m_Instance[i].pMappedPtr = nullptr;
    }

    m_Instance.clear();
    m_Index = 0;
}

//-------------------------------------------------------------------------------------------------
//      次のバッファに進みます.
//-------------------------------------------------------------------------------------------------
void ConstantBuffer::Next()
{ m_Index = (m_Index + 1) % m_Instance.size(); }

//-------------------------------------------------------------------------------------------------
//      内部バッファ数を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ConstantBuffer::GetCount() const
{ return uint32_t(m_Instance.size()); }

//-------------------------------------------------------------------------------------------------
//      バッファ番号を取得します.
//-------------------------------------------------------------------------------------------------
uint32_t ConstantBuffer::GetIndex() const
{ return m_Index; }

//-------------------------------------------------------------------------------------------------
//      CPUハンドルを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleCPU() const
{ return GetHandleCPU(m_Index); }

//-------------------------------------------------------------------------------------------------
//      GPUハンドルを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleGPU() const
{ return GetHandleGPU(m_Index); }

//-------------------------------------------------------------------------------------------------
//      マップ済みポインタを取得します.
//-------------------------------------------------------------------------------------------------
void* ConstantBuffer::GetPtr() const
{ return GetPtr(m_Index); }

//-------------------------------------------------------------------------------------------------
//      インデックスを指定してCPUハンドルを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleCPU(uint32_t index) const
{
    if (index >= uint32_t(m_Instance.size()))
    { return D3D12_CPU_DESCRIPTOR_HANDLE(); }

    if (m_Instance[index].pDescriptor == nullptr)
    { return D3D12_CPU_DESCRIPTOR_HANDLE(); }

    return m_Instance[index].pDescriptor->GetHandleCPU();
}

//-------------------------------------------------------------------------------------------------
//      インデックスを指定してGPUハンドルを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetHandleGPU(uint32_t index) const
{
    if (index >= uint32_t(m_Instance.size()))
    { return D3D12_GPU_DESCRIPTOR_HANDLE(); }

    if (m_Instance[index].pDescriptor == nullptr)
    { return D3D12_GPU_DESCRIPTOR_HANDLE(); }

    return m_Instance[index].pDescriptor->GetHandleGPU();
}

//-------------------------------------------------------------------------------------------------
//      インデックスを指定してマップ済みポインタを取得します.
//-------------------------------------------------------------------------------------------------
void* ConstantBuffer::GetPtr(uint32_t index) const
{
    if (index >= uint32_t(m_Instance.size()))
    { return nullptr; }

    return m_Instance[index].pMappedPtr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
: m_pResource(nullptr)
{ memset(&m_View, 0, sizeof(m_View)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool VertexBuffer::Init(ID3D12Device* pDevice, size_t size, size_t stride)
{
    // 引数チェック.
    if (pDevice == nullptr || size == 0 || stride == 0)
    { return false; }

    // ヒーププロパティ.
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask       = 1;
    prop.VisibleNodeMask        = 1;

    // リソースの設定.
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = UINT64(size);
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    // リソースを生成.
    auto hr = pDevice->CreateCommittedResource(
        &prop, 
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_pResource.GetAddress()));
    if (FAILED(hr))
    { return false; }

    // 頂点バッファビューの設定.
    m_View.BufferLocation = m_pResource->GetGPUVirtualAddress();
    m_View.StrideInBytes  = UINT(stride);
    m_View.SizeInBytes    = UINT(size);

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void VertexBuffer::Term()
{
    m_pResource.Reset();
    memset(&m_View, 0, sizeof(m_View));
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを行います.
//-------------------------------------------------------------------------------------------------
void* VertexBuffer::Map() const
{
    void* ptr;
    auto hr = m_pResource->Map(0, nullptr, &ptr);
    if (FAILED(hr))
    { return nullptr; }

    return ptr;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void VertexBuffer::Unmap()
{ m_pResource->Unmap(0, nullptr); }

//-------------------------------------------------------------------------------------------------
//      頂点バッファビューを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetView() const
{ return m_View; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
: m_pResource(nullptr)
{ memset(&m_View, 0, sizeof(m_View)); }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool IndexBuffer::Init(ID3D12Device* pDevice, size_t count)
{
    // ヒーププロパティ.
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask       = 1;
    prop.VisibleNodeMask        = 1;

    // リソースの設定.
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = UINT64(count * sizeof(uint32_t));
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    // リソースを生成.
    auto hr = pDevice->CreateCommittedResource(
        &prop, 
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_pResource.GetAddress()));
    if (FAILED(hr))
    { return false; }

    // インデックスバッファビューの設定.
    m_View.BufferLocation   = m_pResource->GetGPUVirtualAddress();
    m_View.Format           = DXGI_FORMAT_R32_UINT;
    m_View.SizeInBytes      = UINT(desc.Width);

    m_Count = count;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void IndexBuffer::Term()
{
    m_pResource.Reset();
    memset(&m_View, 0, sizeof(m_View));
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを行います.
//-------------------------------------------------------------------------------------------------
uint32_t* IndexBuffer::Map()
{
    uint32_t* ptr;
    auto hr = m_pResource->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    { return nullptr; }

    return ptr;
}

//-------------------------------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-------------------------------------------------------------------------------------------------
void IndexBuffer::Unmap()
{ m_pResource->Unmap(0, nullptr); }

//-------------------------------------------------------------------------------------------------
//      インデックスバッファビューを取得します.
//-------------------------------------------------------------------------------------------------
D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetView() const
{ return m_View; }

//-------------------------------------------------------------------------------------------------
//      インデックス数を取得します.
//-------------------------------------------------------------------------------------------------
size_t IndexBuffer::GetCount() const
{ return m_Count; }

} // namespace asdx
