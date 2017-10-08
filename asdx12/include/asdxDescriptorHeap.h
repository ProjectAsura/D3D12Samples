//-------------------------------------------------------------------------------------------------
// File : asdxDescriptorHeap.h
// Desc : Descriptor Heap.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <d3d12.h>
#include <atomic>
#include <asdxRefPtr.h>
#include <asdxPoolContainer.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Descriptor class
///////////////////////////////////////////////////////////////////////////////////////////////////
class Descriptor : public IReference
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    friend class DescriptorHeap;
    friend class PoolContainer<Descriptor>;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    void        AddRef  () override;
    void        Release () override;
    uint32_t    GetCount() const override;
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU() const;

private:
    //=============================================================================================
    // private variables.
    //============================================================================================= 
    DescriptorHeap*             m_pHeap;        //!< ヒープへのポインタです.
    D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCPU;    //!< CPUディスクリプタハンドルです.
    D3D12_GPU_DESCRIPTOR_HANDLE m_HandleGPU;    //!< GPUディスクリプタハンドルです.
    std::atomic<uint32_t>       m_RefCount;     //!< 参照カウンタです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    Descriptor();
    ~Descriptor();

    Descriptor      (const Descriptor&) = delete;
    void operator = (const Descriptor&) = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// DescriptorHeap class
///////////////////////////////////////////////////////////////////////////////////////////////////
class DescriptorHeap
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    friend class Descriptor;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    DescriptorHeap();
    ~DescriptorHeap();
    bool Init(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc);
    void Term();
    Descriptor* CreateDescriptor();
    uint32_t GetAvailableHandleCount() const;
    uint32_t GetAllocatedHandleCount() const;
    uint32_t GetHandleCount() const;
    ID3D12DescriptorHeap* GetHeap() const;

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    RefPtr<ID3D12DescriptorHeap>    m_pHeap;
    PoolContainer<Descriptor>       m_Pool;
    uint32_t                        m_IncrementSize;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    void DisposeDescriptor(Descriptor* pValue);

    DescriptorHeap  (const DescriptorHeap&) = delete;
    void operator = (const DescriptorHeap&) = delete;
};

} // namespace asdx