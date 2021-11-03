//-----------------------------------------------------------------------------
// File : DescriptorHeap.cpp
// Desc : Descriptor Heap Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <DescriptorHeap.h>
#include <asdxList.h>


///////////////////////////////////////////////////////////////////////////////
// Descriptor class
///////////////////////////////////////////////////////////////////////////////
class Descriptor : public asdx::List<Descriptor>::Node, public IDescriptor
{
    friend class DescriptorHeap;

public:
    void Release() override;

    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const override { return m_HandleCPU; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const override { return m_HandleGPU; }

private:
    DescriptorHeap*             m_Heap      = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCPU = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_HandleGPU = {};
};


///////////////////////////////////////////////////////////////////////////////
// DescriptorHeap class
///////////////////////////////////////////////////////////////////////////////
class DescriptorHeap : public IDescriptorHeap
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    void Release() override
    {
        delete this;
    }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = type;
        desc.NumDescriptors = count;

        auto enableGPU = false;
        if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
            type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        {
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            enableGPU = true;
        }

        auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));
        if (FAILED(hr))
        {
            return false;
        }

        auto increment = device->GetDescriptorHandleIncrementSize(type);

        D3D12_CPU_DESCRIPTOR_HANDLE headCPU = m_Heap->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE headGPU = {};

        if (enableGPU)
        { headGPU = m_Heap->GetGPUDescriptorHandleForHeapStart(); }

        m_Descriptors = new Descriptor[count];

        for(size_t i=0; i<count; ++i)
        {
            m_Descriptors[i].m_HandleCPU.ptr = headCPU.ptr + increment * i;
            if (enableGPU)
            { m_Descriptors[i].m_HandleGPU.ptr = headGPU.ptr + increment * i; }

            m_FreeList.PushBack(&m_Descriptors[i]);
        }

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        m_UsedList.Clear();
        m_FreeList.Clear();

        if (m_Descriptors != nullptr)
        {
            delete[] m_Descriptors;
            m_Descriptors = nullptr;
        }

        if (m_Heap != nullptr)
        {
            m_Heap->Release();
            m_Heap = nullptr;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタを確保.
    //-------------------------------------------------------------------------
    IDescriptor* Alloc() override
    {
        auto item = m_FreeList.PopFront();
        m_UsedList.PushBack(item);
        return item;
    }

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタを解放.
    //-------------------------------------------------------------------------
    void Free(IDescriptor* descriptor)
    {
        auto item = reinterpret_cast<Descriptor*>(descriptor);

        if (item == nullptr)
        { return; }

        if (m_UsedList.Contains(item))
        {
            m_UsedList.Remove(item);
            m_FreeList.PushBack(item);
        }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ID3D12DescriptorHeap*   m_Heap          = nullptr;
    Descriptor*             m_Descriptors   = nullptr;
    asdx::List<Descriptor>  m_UsedList;
    asdx::List<Descriptor>  m_FreeList;

    //=========================================================================
    // private methods.
    //=========================================================================
    ~DescriptorHeap()
    { Term(); }
};

void Descriptor::Release()
{
    if (m_Heap != nullptr)
    { m_Heap->Free(this); }

    m_HandleCPU.ptr = 0;
    m_HandleGPU.ptr = 0;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool CreateDescriptorHeap
(
    ID3D12Device*               device,
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    UINT                        count,
    IDescriptorHeap**           heap
)
{
    if (heap == nullptr)
    { return false; }

    auto instance = new DescriptorHeap();
    if (!instance->Init(device, type, count))
    {
        instance->Release();
        return false;
    }

    *heap = instance;
    return true;
}
